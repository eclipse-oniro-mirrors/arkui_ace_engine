/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ui_info_collection_manager.h"

#include "base/utils/time_util.h"
#include "core/common/container.h"
#include "core/components_ng/base/frame_node.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "ui_interaction_json_builder.h"

namespace OHOS::Ace::NG {
namespace {

constexpr int64_t NANOSECONDS_PER_MILLISECOND = 1000000;

const char* GetGestureActionString(GestureCallbackType type)
{
    switch (type) {
        case GestureCallbackType::START:
            return "start";
        case GestureCallbackType::END:
            return "end";
        case GestureCallbackType::CANCEL:
            return "cancel";
        default:
            return nullptr;
    }
}

std::vector<ArkUIInteractionPoint> GetFingerScreenPoints(const GestureEvent& info)
{
    std::vector<ArkUIInteractionPoint> points;
    for (const auto& finger : info.GetFingerList()) {
        points.push_back({ finger.screenLocation_.GetX(), finger.screenLocation_.GetY() });
    }
    return points;
}

} // namespace

int32_t UIInfoCollectionManager::RegisterObserver(const RegisterParam& param, uint32_t* registerID)
{
    if (registerID) {
        *registerID = 0;
    }
    if (!registerID || !param.callback || !param.createWrapper || !param.destroyWrapper) {
        return INTERACTION_RESULT_PARAM_INVALID;
    }
    const uint32_t effectiveMask = param.eventMask & INTERACTION_EVENT_ALL;
    if (effectiveMask == 0) {
        return INTERACTION_RESULT_PARAM_INVALID;
    }
    auto container = Container::GetContainer(param.instanceId);
    if (!container) {
        return INTERACTION_RESULT_UI_CONTEXT_INVALID;
    }
    uint32_t id = 0;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        id = nextRegisterID_++;
        if (id == 0) {
            return INTERACTION_RESULT_PARAM_INVALID;
        }
        auto observer = std::make_shared<Observer>();
        observer->eventMask = effectiveMask;
        observer->callback = param.callback;
        observer->userData = param.userData;
        observers_[param.instanceId][id] = observer;
        createWrapper_ = param.createWrapper;
        destroyWrapper_ = param.destroyWrapper;
        hasAnyObserver_.store(true, std::memory_order_release);
    }
    *registerID = id;
    return INTERACTION_RESULT_SUCCESS;
}

int32_t UIInfoCollectionManager::UnregisterObserver(uint32_t registerID)
{
    if (registerID == 0) {
        return INTERACTION_RESULT_PARAM_INVALID;
    }
    ObserverPtr observer;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto instanceIt = observers_.begin(); instanceIt != observers_.end(); ++instanceIt) {
            auto& registry = instanceIt->second;
            auto observerIt = registry.find(registerID);
            if (observerIt != registry.end()) {
                observer = observerIt->second;
                registry.erase(observerIt);
                if (registry.empty()) {
                    // The instance's last observer is gone: drop its
                    // drag-start timing record as well.
                    lastTouchDown_.erase(instanceIt->first);
                    observers_.erase(instanceIt);
                }
                break;
            }
        }
        if (observers_.empty()) {
            hasAnyObserver_.store(false, std::memory_order_release);
        }
    }
    if (!observer) {
        return INTERACTION_RESULT_PARAM_INVALID;
    }
    // The public contract confines every entry point to the instance's UI
    // main thread, so no other thread can be dispatching this observer's
    // callback: flipping the flag is all the synchronization needed. When
    // called from inside the observer's own callback, the running invocation
    // is the caller's own stack frame and simply completes; no new one is
    // started because the entry is out of the registry and the flag is
    // false for dispatchers holding an older snapshot.
    observer->active.store(false, std::memory_order_seq_cst);
    return INTERACTION_RESULT_SUCCESS;
}

bool UIInfoCollectionManager::HasObserver(int32_t instanceId, uint32_t eventBit)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto instanceIt = observers_.find(instanceId);
    if (instanceIt == observers_.end()) {
        return false;
    }
    for (const auto& item : instanceIt->second) {
        const auto& observer = item.second;
        if (observer && observer->callback && (observer->eventMask & eventBit)) {
            return true;
        }
    }
    return false;
}

void UIInfoCollectionManager::DispatchEvent(int32_t instanceId, uint32_t eventBit, const std::string& json)
{
    if (json.empty()) {
        return;
    }
    std::vector<ObserverPtr> targets;
    CreateJsonWrapperFunc createWrapper = nullptr;
    DestroyJsonWrapperFunc destroyWrapper = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        createWrapper = createWrapper_;
        destroyWrapper = destroyWrapper_;
        auto instanceIt = observers_.find(instanceId);
        if (instanceIt != observers_.end()) {
            for (const auto& item : instanceIt->second) {
                const auto& observer = item.second;
                if (observer && observer->callback && (observer->eventMask & eventBit)) {
                    targets.push_back(observer);
                }
            }
        }
    }
    if (targets.empty() || !createWrapper || !destroyWrapper) {
        return;
    }
    // The wrapper is immutable, so one instance is shared by the sequential
    // callback invocations and destroyed right after the last one returns.
    OH_ArkUI_NativeModule_UIJsonWrapper* wrapper = createWrapper(json.c_str(), static_cast<uint32_t>(json.size()));
    if (!wrapper) {
        return;
    }
    for (const auto& observer : targets) {
        // Skip observers removed after the snapshot was taken: a callback in
        // this batch may have unregistered them (or themselves). Under the
        // UI-main-thread contract this re-check is the only race window that
        // can exist, and it closes it.
        if (!observer->active.load(std::memory_order_seq_cst)) {
            continue;
        }
        observer->callback(wrapper, observer->userData);
    }
    destroyWrapper(wrapper);
}

std::string UIInfoCollectionManager::GetHostName(int32_t instanceId)
{
    auto container = Container::GetContainer(instanceId);
    return container ? container->GetBundleName() : std::string();
}

int32_t UIInfoCollectionManager::GetNodeInstanceId(const RefPtr<FrameNode>& node)
{
    if (!node) {
        return -1;
    }
    auto context = node->GetContext();
    return context ? context->GetInstanceId() : -1;
}

void UIInfoCollectionManager::MaintainTouchDownRecord(int32_t instanceId, bool isDown, int32_t pointerId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto instanceIt = observers_.find(instanceId);
    if (instanceIt == observers_.end()) {
        return;
    }
    bool dragObserved = false;
    for (const auto& item : instanceIt->second) {
        const auto& observer = item.second;
        if (observer && observer->callback && (observer->eventMask & INTERACTION_EVENT_DRAG)) {
            dragObserved = true;
            break;
        }
    }
    if (!dragObserved) {
        return;
    }
    if (isDown) {
        lastTouchDown_[instanceId] = { GetSysTimestamp(), pointerId };
        return;
    }
    auto recordIt = lastTouchDown_.find(instanceId);
    if (recordIt != lastTouchDown_.end() && recordIt->second.pointerId == pointerId) {
        lastTouchDown_.erase(recordIt);
    }
}

int64_t UIInfoCollectionManager::GetDragStartDurationMs(int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = lastTouchDown_.find(instanceId);
    if (it == lastTouchDown_.end() || it->second.timestampNanos <= 0) {
        return 0;
    }
    const int64_t now = GetSysTimestamp();
    if (now <= it->second.timestampNanos) {
        return 0;
    }
    return (now - it->second.timestampNanos) / NANOSECONDS_PER_MILLISECOND;
}

void UIInfoCollectionManager::NotifyTapOrClick(
    const RefPtr<FrameNode>& node, bool isClick, int32_t count, const GestureEvent& info)
{
    if (!HasAnyObserver() || !node) {
        return;
    }
    const int32_t instanceId = GetNodeInstanceId(node);
    const uint32_t eventBit = isClick ? INTERACTION_EVENT_CLICK : INTERACTION_EVENT_TAP;
    if (!HasObserver(instanceId, eventBit)) {
        return;
    }
    const auto& screenPoint = info.GetScreenLocation();
    auto json = ArkUIInteractionJsonBuilder::BuildTapOrClickJson(isClick, node->GetId(),
        { screenPoint.GetX(), screenPoint.GetY() }, count,
        static_cast<int32_t>(info.GetFingerList().size()));
    DispatchEvent(instanceId, eventBit, json);
}

void UIInfoCollectionManager::NotifyLongPress(
    const RefPtr<FrameNode>& node, const GestureEvent& info, int64_t durationNanoseconds)
{
    if (!HasAnyObserver() || !node) {
        return;
    }
    const int32_t instanceId = GetNodeInstanceId(node);
    if (!HasObserver(instanceId, INTERACTION_EVENT_LONG_PRESS)) {
        return;
    }
    const auto& screenPoint = info.GetScreenLocation();
    auto json = ArkUIInteractionJsonBuilder::BuildLongPressJson(node->GetId(),
        { screenPoint.GetX(), screenPoint.GetY() }, durationNanoseconds / NANOSECONDS_PER_MILLISECOND);
    DispatchEvent(instanceId, INTERACTION_EVENT_LONG_PRESS, json);
}

void UIInfoCollectionManager::NotifyPan(
    const RefPtr<FrameNode>& node, const GestureEvent& info, uint32_t direction, GestureCallbackType type)
{
    const char* action = GetGestureActionString(type);
    if (action == nullptr || !HasAnyObserver() || !node) {
        return;
    }
    const int32_t instanceId = GetNodeInstanceId(node);
    if (!HasObserver(instanceId, INTERACTION_EVENT_PAN)) {
        return;
    }
    const auto& screenPoint = info.GetScreenLocation();
    auto json = ArkUIInteractionJsonBuilder::BuildPanJson(
        node->GetId(), { screenPoint.GetX(), screenPoint.GetY() }, direction, action);
    DispatchEvent(instanceId, INTERACTION_EVENT_PAN, json);
}

void UIInfoCollectionManager::NotifyPinch(const RefPtr<FrameNode>& node, const GestureEvent& info,
    GestureCallbackType type)
{
    const char* action = GetGestureActionString(type);
    if (action == nullptr || !HasAnyObserver() || !node) {
        return;
    }
    const int32_t instanceId = GetNodeInstanceId(node);
    if (!HasObserver(instanceId, INTERACTION_EVENT_PINCH)) {
        return;
    }
    ArkUIInteractionPinchParam param;
    param.nodeId = node->GetId();
    param.points = GetFingerScreenPoints(info);
    param.fingers = static_cast<int32_t>(info.GetFingerList().size());
    param.action = action;
    if (type == GestureCallbackType::END) {
        param.scale = info.GetScale();
    }
    auto json = ArkUIInteractionJsonBuilder::BuildPinchJson(param);
    DispatchEvent(instanceId, INTERACTION_EVENT_PINCH, json);
}

void UIInfoCollectionManager::NotifyRotation(const RefPtr<FrameNode>& node, const GestureEvent& info,
    GestureCallbackType type)
{
    const char* action = GetGestureActionString(type);
    if (action == nullptr || !HasAnyObserver() || !node) {
        return;
    }
    const int32_t instanceId = GetNodeInstanceId(node);
    if (!HasObserver(instanceId, INTERACTION_EVENT_ROTATION)) {
        return;
    }
    ArkUIInteractionRotationParam param;
    param.nodeId = node->GetId();
    param.points = GetFingerScreenPoints(info);
    param.fingers = static_cast<int32_t>(info.GetFingerList().size());
    param.action = action;
    if (type == GestureCallbackType::END) {
        param.angle = info.GetAngle();
    }
    auto json = ArkUIInteractionJsonBuilder::BuildRotationJson(param);
    DispatchEvent(instanceId, INTERACTION_EVENT_ROTATION, json);
}

void UIInfoCollectionManager::NotifySwipe(const RefPtr<FrameNode>& node, const GestureEvent& info,
    uint32_t direction, double configuredSpeedPx, const std::map<int32_t, TouchEvent>& downEvents)
{
    if (!HasAnyObserver() || !node) {
        return;
    }
    const int32_t instanceId = GetNodeInstanceId(node);
    if (!HasObserver(instanceId, INTERACTION_EVENT_SWIPE)) {
        return;
    }
    std::vector<ArkUIInteractionPoint> downPoints;
    for (const auto& item : downEvents) {
        const auto& screenPoint = item.second.GetScreenOffset();
        downPoints.push_back({ screenPoint.GetX(), screenPoint.GetY() });
    }
    ArkUIInteractionSwipeParam param;
    param.nodeId = node->GetId();
    param.downPoints = std::move(downPoints);
    param.upPoints = GetFingerScreenPoints(info);
    param.direction = direction;
    param.speed = configuredSpeedPx;
    param.actualSpeed = info.GetSpeed();
    auto json = ArkUIInteractionJsonBuilder::BuildSwipeJson(param);
    DispatchEvent(instanceId, INTERACTION_EVENT_SWIPE, json);
}

void UIInfoCollectionManager::NotifyTouch(
    int32_t instanceId, bool isDown, const TouchEvent& event, int32_t hitNodeId)
{
    if (!HasAnyObserver()) {
        return;
    }
    // The drag-start duration field is derived from the touch-down record,
    // so the record must be maintained whenever the instance observes DRAG
    // even when TOUCH itself is not observed: subscription filtering must
    // not change the correctness of another subscribed event's fields. The
    // record is refreshed on every down and erased on the recording pointer's
    // up or cancel, bounding it to the current press.
    MaintainTouchDownRecord(instanceId, isDown, event.id);
    // The TOUCH JSON is still created and dispatched by the TOUCH bit alone.
    if (!HasObserver(instanceId, INTERACTION_EVENT_TOUCH)) {
        return;
    }
    const auto& screenPoint = event.GetScreenOffset();
    auto json = ArkUIInteractionJsonBuilder::BuildTouchJson(isDown, event.id,
        { screenPoint.GetX(), screenPoint.GetY() }, hitNodeId >= 0, hitNodeId);
    DispatchEvent(instanceId, INTERACTION_EVENT_TOUCH, json);
}

void UIInfoCollectionManager::NotifyTouchCancelled(int32_t instanceId, int32_t pointerId)
{
    if (!HasAnyObserver()) {
        return;
    }
    MaintainTouchDownRecord(instanceId, false, pointerId);
}

void UIInfoCollectionManager::NotifyDragStart(const RefPtr<FrameNode>& node, double screenX, double screenY)
{
    if (!HasAnyObserver() || !node) {
        return;
    }
    const int32_t instanceId = GetNodeInstanceId(node);
    if (!HasObserver(instanceId, INTERACTION_EVENT_DRAG)) {
        return;
    }
    auto json = ArkUIInteractionJsonBuilder::BuildDragStartJson(
        node->GetId(), { screenX, screenY }, GetHostName(instanceId), GetDragStartDurationMs(instanceId));
    DispatchEvent(instanceId, INTERACTION_EVENT_DRAG, json);
}

void UIInfoCollectionManager::NotifyDragEnd(
    int32_t instanceId, double screenX, double screenY, bool dropSuccess, int32_t targetNodeId)
{
    if (!HasAnyObserver() || !HasObserver(instanceId, INTERACTION_EVENT_DRAG)) {
        return;
    }
    ArkUIInteractionDragEndParam param;
    param.point = { screenX, screenY };
    param.hostName = GetHostName(instanceId);
    param.dropSuccess = dropSuccess;
    if (targetNodeId >= 0) {
        param.targetId = targetNodeId;
    }
    auto json = ArkUIInteractionJsonBuilder::BuildDragEndJson(param);
    DispatchEvent(instanceId, INTERACTION_EVENT_DRAG, json);
}

} // namespace OHOS::Ace::NG
