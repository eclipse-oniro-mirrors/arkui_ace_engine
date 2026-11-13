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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_INTERACTION_OBSERVER_UI_INFO_COLLECTION_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_INTERACTION_OBSERVER_UI_INFO_COLLECTION_MANAGER_H

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/memory/referenced.h"
#include "core/components_ng/event/event_constants.h"
#include "core/event/touch_event.h"
#include "ui/gestures/gesture_event.h"

// Forward declaration: the JSON wrapper type is opaque to the framework side
// and only pointer types are used below. The full bridge contract lives in
// core/interfaces/native/node/interaction_observer_api.h and is included by
// the implementing .cpp files only, keeping its reference count low.
struct OH_ArkUI_NativeModule_UIJsonWrapper;

namespace OHOS::Ace::NG {

class FrameNode;

// Event bits mirroring the public
// OH_ArkUI_NativeModule_UIInfoCollection_InteractionEventType values.
enum InteractionObserverEvent : uint32_t {
    INTERACTION_EVENT_TAP = 1 << 0,
    INTERACTION_EVENT_CLICK = 1 << 1,
    INTERACTION_EVENT_LONG_PRESS = 1 << 2,
    INTERACTION_EVENT_PAN = 1 << 3,
    INTERACTION_EVENT_PINCH = 1 << 4,
    INTERACTION_EVENT_ROTATION = 1 << 5,
    INTERACTION_EVENT_SWIPE = 1 << 6,
    INTERACTION_EVENT_DRAG = 1 << 7,
    INTERACTION_EVENT_TOUCH = 1 << 8,
    INTERACTION_EVENT_ALL = (1 << 9) - 1,
};

// Result codes mirroring the public ArkUI_ErrorCode values used by the
// interaction observer entry points.
constexpr int32_t INTERACTION_RESULT_SUCCESS = 0;
constexpr int32_t INTERACTION_RESULT_PARAM_INVALID = 401;
constexpr int32_t INTERACTION_RESULT_UI_CONTEXT_INVALID = 190001;

// Registry and dispatch hub for NDK interaction observers. Observers are
// registered through the ArkUIInteractionObserverAPI bridge from the NDK
// layer; framework hook sites (gesture recognizers, EventManager,
// DragDropManager) call the Notify* methods on the UI thread. The manager is
// read-only with respect to gesture/hit-testing behavior: it only observes
// the data already produced at those dispatch points.
class UIInfoCollectionManager {
public:
    using InteractionCallback = void (*)(const OH_ArkUI_NativeModule_UIJsonWrapper* json, void* userData);
    using CreateJsonWrapperFunc = OH_ArkUI_NativeModule_UIJsonWrapper* (*)(const char* data, uint32_t size);
    using DestroyJsonWrapperFunc = void (*)(const OH_ArkUI_NativeModule_UIJsonWrapper* wrapper);

    // Registration request supplied by the NDK bridge entry point. eventMask
    // uses the InteractionObserverEvent bits; unsupported bits are ignored.
    struct RegisterParam {
        int32_t instanceId = -1;
        uint32_t eventMask = 0;
        InteractionCallback callback = nullptr;
        void* userData = nullptr;
        CreateJsonWrapperFunc createWrapper = nullptr;
        DestroyJsonWrapperFunc destroyWrapper = nullptr;
    };

    static UIInfoCollectionManager& GetInstance()
    {
        static UIInfoCollectionManager instance;
        return instance;
    }

    // Bridge entry points, callable from any thread. On failure *registerID
    // is set to 0.
    int32_t RegisterObserver(const RegisterParam& param, uint32_t* registerID);
    int32_t UnregisterObserver(uint32_t registerID);

    // Lock-free fast path for hook sites: false means no observer is
    // registered anywhere and all serialization work can be skipped.
    bool HasAnyObserver() const
    {
        return hasAnyObserver_.load(std::memory_order_acquire);
    }

    // Gesture hook sites. All must be called on the UI thread; node may be
    // null (the notification is dropped).
    void NotifyTapOrClick(const RefPtr<FrameNode>& node, bool isClick, int32_t count, const GestureEvent& info);
    void NotifyLongPress(const RefPtr<FrameNode>& node, const GestureEvent& info, int64_t durationNanoseconds);
    void NotifyPan(const RefPtr<FrameNode>& node, const GestureEvent& info, uint32_t direction,
        GestureCallbackType type);
    void NotifyPinch(const RefPtr<FrameNode>& node, const GestureEvent& info, GestureCallbackType type);
    void NotifyRotation(const RefPtr<FrameNode>& node, const GestureEvent& info, GestureCallbackType type);
    void NotifySwipe(const RefPtr<FrameNode>& node, const GestureEvent& info, uint32_t direction,
        double configuredSpeedPx, const std::map<int32_t, TouchEvent>& downEvents);

    // Raw touch hook site (EventManager), UI thread. hitNodeId is the first
    // hit frame node id for a down event, or -1 when unknown; up events never
    // carry a node id.
    void NotifyTouch(int32_t instanceId, bool isDown, const TouchEvent& event, int32_t hitNodeId);

    // Touch-cancel hook site (EventManager), UI thread. Cancel never reaches
    // observers (the public TOUCH surface is down and up only); it only ends
    // the drag-start timing record maintained by NotifyTouch.
    void NotifyTouchCancelled(int32_t instanceId, int32_t pointerId);

    // Drag hook sites (DragDropManager), UI thread. Screen coordinates are
    // physical pixels. For the end notification targetNodeId is the drop
    // target id and is only reported when dropSuccess is true.
    void NotifyDragStart(const RefPtr<FrameNode>& node, double screenX, double screenY);
    void NotifyDragEnd(int32_t instanceId, double screenX, double screenY, bool dropSuccess, int32_t targetNodeId);

private:
    UIInfoCollectionManager() = default;
    ~UIInfoCollectionManager() = default;
    UIInfoCollectionManager(const UIInfoCollectionManager&) = delete;
    UIInfoCollectionManager& operator=(const UIInfoCollectionManager&) = delete;

    struct Observer {
        uint32_t eventMask = 0;
        InteractionCallback callback = nullptr;
        void* userData = nullptr;

        // Lifecycle coordination with UnregisterObserver. The public contract
        // requires every entry point to be called on the instance's UI main
        // thread, so unregistering and dispatching are serialized on one
        // thread and no cross-thread race exists. What remains is the same
        // dispatch batch: a callback may unregister an observer (another one
        // or itself) whose entry was already copied into the dispatch
        // snapshot. The flag flips to false before UnregisterObserver
        // returns, and DispatchEvent re-checks it before every invocation,
        // so a removed observer is never invoked. An atomic is kept purely
        // as defense in depth against contract violations.
        std::atomic<bool> active { true };
    };
    using ObserverPtr = std::shared_ptr<Observer>;

    bool HasObserver(int32_t instanceId, uint32_t eventBit);
    void DispatchEvent(int32_t instanceId, uint32_t eventBit, const std::string& json);
    std::string GetHostName(int32_t instanceId);
    int32_t GetNodeInstanceId(const RefPtr<FrameNode>& node);
    int64_t GetDragStartDurationMs(int32_t instanceId);
    // Maintains the per-instance touch-down record used by the drag-start
    // duration field. Recording is gated on the instance observing DRAG (the
    // record must not depend on TOUCH being observed) and happens under one
    // lock acquisition together with the subscription check. A down refreshes
    // the record; an up or cancel of the recording pointer erases it, bounding
    // the record to the press that wrote it.
    void MaintainTouchDownRecord(int32_t instanceId, bool isDown, int32_t pointerId);

    // Touch-down timestamp source for the drag-start duration field.
    struct TouchDownRecord {
        int64_t timestampNanos = 0;
        int32_t pointerId = -1;
    };

    // Guarded by mutex_.
    std::unordered_map<int32_t, std::unordered_map<uint32_t, ObserverPtr>> observers_;
    std::unordered_map<int32_t, TouchDownRecord> lastTouchDown_;
    uint32_t nextRegisterID_ = 1;
    CreateJsonWrapperFunc createWrapper_ = nullptr;
    DestroyJsonWrapperFunc destroyWrapper_ = nullptr;
    std::mutex mutex_;
    std::atomic<bool> hasAnyObserver_ { false };
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_INTERACTION_OBSERVER_UI_INFO_COLLECTION_MANAGER_H
