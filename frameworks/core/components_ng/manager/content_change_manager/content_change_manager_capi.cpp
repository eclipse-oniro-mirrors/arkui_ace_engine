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

#include "content_change_manager_impl.h"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <limits>
#include <vector>

#include "base/log/ace_trace.h"
#include "base/utils/time_util.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/swiper/swiper_paint_property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "interfaces/inner_api/ace_kit/include/json/json_util.h"
#include "interfaces/native/ui_info_collection.h"

namespace OHOS::Ace::NG {
namespace {
constexpr uint32_t PAGE_CATEGORY = OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_PAGE;
constexpr uint32_t SCROLL_CATEGORY = OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_SCROLL;
constexpr uint32_t OVERLAY_CATEGORY = OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_OVERLAY;
constexpr uint32_t IGNORE_SCROLL_BY = OH_ARKUI_NATIVEMODULE_CONTENTCHANGE_IGNORE_SCROLLBY;
constexpr uint32_t IGNORE_SCROLL_TO = OH_ARKUI_NATIVEMODULE_CONTENTCHANGE_IGNORE_SCROLLTO;
constexpr uint32_t IGNORE_TOAST = OH_ARKUI_NATIVEMODULE_CONTENTCHANGE_IGNORE_TOAST;
constexpr uint32_t PAGE_START = OH_ARKUI_NATIVEMODULE_EVENT_PAGE_CHANGE_START;
constexpr uint32_t PAGE_END = OH_ARKUI_NATIVEMODULE_EVENT_PAGE_CHANGE_END;
constexpr uint32_t SCROLL_START = OH_ARKUI_NATIVEMODULE_EVENT_SCROLL_START;
constexpr uint32_t SCROLL_END = OH_ARKUI_NATIVEMODULE_EVENT_SCROLL_END;
constexpr uint32_t OVERLAY_SHOW = OH_ARKUI_NATIVEMODULE_EVENT_OVERLAY_SHOW;
constexpr uint32_t OVERLAY_HIDE = OH_ARKUI_NATIVEMODULE_EVENT_OVERLAY_HIDE;
constexpr uint32_t SWIPER_START = OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_START;
constexpr uint32_t SWIPER_END = OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_END;
constexpr uint32_t SWIPER_CANCEL = OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_CANCEL;
constexpr uint32_t TABS_START = OH_ARKUI_NATIVEMODULE_EVENT_TABS_START;
constexpr uint32_t TABS_END = OH_ARKUI_NATIVEMODULE_EVENT_TABS_END;
constexpr uint32_t TABS_CANCEL = OH_ARKUI_NATIVEMODULE_EVENT_TABS_CANCEL;

constexpr int32_t CONTENT_CHANGE_SCHEMA_VERSION = 1;
constexpr uint32_t NO_SCROLL_EVENT_SOURCE = 0;
constexpr uint32_t END_EVENT_OFFSET = PAGE_END - PAGE_START;
constexpr uint32_t CANCEL_EVENT_OFFSET = SWIPER_CANCEL - SWIPER_START;
static_assert(SCROLL_END - SCROLL_START == END_EVENT_OFFSET &&
    SWIPER_END - SWIPER_START == END_EVENT_OFFSET && TABS_END - TABS_START == END_EVENT_OFFSET);
static_assert(TABS_CANCEL - TABS_START == CANCEL_EVENT_OFFSET);

uint32_t CategoryForType(uint32_t type)
{
    if (type == SCROLL_START || type == SCROLL_END) {
        return SCROLL_CATEGORY;
    }
    if (type == OVERLAY_SHOW || type == OVERLAY_HIDE) {
        return OVERLAY_CATEGORY;
    }
    return PAGE_CATEGORY;
}

bool IsStartType(uint32_t type)
{
    return type == PAGE_START || type == SCROLL_START || type == SWIPER_START || type == TABS_START ||
        type == SWIPER_CANCEL || type == TABS_CANCEL;
}

std::unique_ptr<JsonValue> NodeRef(const RefPtr<FrameNode>& node)
{
    if (!node) {
        return nullptr;
    }
    auto result = JsonUtil::Create(true);
    result->Put("id", std::to_string(node->GetId()).c_str());
    result->Put("type", node->GetTag().c_str());
    return result;
}

std::unique_ptr<JsonValue> SnapshotNode(const RefPtr<UINode>& node);

void AppendSnapshots(const RefPtr<UINode>& node, std::unique_ptr<JsonValue>& children)
{
    if (!node) {
        return;
    }
    auto snapshot = SnapshotNode(node);
    if (snapshot) {
        children->PutRef(std::move(snapshot));
        return;
    }
    for (const auto& child : node->GetChildren(true)) {
        AppendSnapshots(child, children);
    }
    for (const auto& [child, index, branch] : node->GetDisappearingChildren()) {
        AppendSnapshots(child, children);
    }
}

std::unique_ptr<JsonValue> SnapshotNode(const RefPtr<UINode>& node)
{
    auto frameNode = AceType::DynamicCast<FrameNode>(node);
    if (!frameNode) {
        return nullptr;
    }
    auto result = NodeRef(frameNode);
    auto children = JsonUtil::CreateArray(true);
    for (const auto& child : node->GetChildren(true)) {
        AppendSnapshots(child, children);
    }
    for (const auto& [child, index, branch] : node->GetDisappearingChildren()) {
        AppendSnapshots(child, children);
    }
    result->PutRef("children", std::move(children));
    return result;
}

std::string BuildCapiContentChangeEventJson(uint32_t type, const RefPtr<FrameNode>& target,
    const char* overlayType = nullptr, const RefPtr<FrameNode>& trigger = nullptr, bool includeSubtree = true)
{
    auto category = CategoryForType(type);
    if (target) {
        auto json = JsonUtil::Create(true);
        json->Put("schemaVersion", CONTENT_CHANGE_SCHEMA_VERSION);
        json->Put("eventType", static_cast<int32_t>(type));
        json->Put("category", category == PAGE_CATEGORY ? "page" :
            (category == SCROLL_CATEGORY ? "scroll" : "overlay"));
        json->PutRef("targetNode", NodeRef(target));
        if (trigger) {
            json->PutRef("triggerNode", NodeRef(trigger));
        } else {
            json->PutRef("triggerNode", JsonUtil::ParseJsonString("null"));
        }
        if (overlayType) {
            json->Put("overlayType", overlayType);
        } else {
            json->PutRef("overlayType", JsonUtil::ParseJsonString("null"));
        }
        if (includeSubtree && (type == PAGE_END || type == OVERLAY_SHOW)) {
            json->PutRef("subtree", SnapshotNode(target));
        } else {
            json->PutRef("subtree", JsonUtil::ParseJsonString("null"));
        }
        return json->ToString();
    }
    return {};
}
} // namespace

std::atomic<uint64_t> ContentChangeManager::CapiImpl::nextCapiSubscriptionId_ { 1 };

RefPtr<FrameNode> ContentChangeManager::GetSwiperContentChangeNode(const RefPtr<FrameNode>& node, bool hasTabsAncestor)
{
    if (!node || !hasTabsAncestor) {
        return node;
    }
    auto parent = node->GetParent();
    while (parent) {
        auto frame = AceType::DynamicCast<FrameNode>(parent);
        if (frame && frame->GetTag() == V2::TABS_ETS_TAG) {
            return frame;
        }
        parent = parent->GetParent();
    }
    return node;
}

uint64_t ContentChangeManager::CapiImpl::RegisterCapiContentChangeEvent(
    bool withStart, uint32_t eventMask, uint32_t ignoreMask,
    void* cookie, CapiSink sink, void (*release)(void*))
{
    if (!sink) {
        return 0;
    }
    auto record = std::make_shared<CapiSubscription>();
    record->withStart = withStart;
    record->eventMask = eventMask;
    record->ignoreMask = ignoreMask;
    record->cookie = cookie;
    record->sink = sink;
    auto id = nextCapiSubscriptionId_.load(std::memory_order_relaxed);
    while (id && !nextCapiSubscriptionId_.compare_exchange_weak(id,
        id == std::numeric_limits<uint64_t>::max() ? 0 : id + 1, std::memory_order_relaxed)) {}
    if (!id) {
        return 0;
    }
    record->release = release;
    capiSubscriptions_.emplace(id, std::move(record));
    return id;
}

bool ContentChangeManager::CapiImpl::UnregisterCapiContentChangeEvent(uint64_t subscriptionId)
{
    auto subscription = capiSubscriptions_.find(subscriptionId);
    if (subscription == capiSubscriptions_.end()) {
        return false;
    }
    subscription->second->active = false;
    capiSubscriptions_.erase(subscription);
    if (capiSubscriptions_.empty()) {
        capiActiveSwipers_.clear();
        capiPendingSwipers_.clear();
    }
    return true;
}

void ContentChangeManager::CapiImpl::InvalidateCapiContentChangeSubscriptions()
{
    for (const auto& [id, record] : capiSubscriptions_) {
        record->active = false;
    }
    capiSubscriptions_.clear();
    capiScrollingNodes_.clear();
    capiActiveSwipers_.clear();
    capiPendingSwipers_.clear();
    capiScrollSources_.clear();
}

void ContentChangeManager::CapiImpl::DispatchCapiContentChangeEvent(uint32_t type, const RefPtr<FrameNode>& target,
    uint32_t source,
    const char* overlayType, const RefPtr<FrameNode>& trigger, bool pairedOnly,
    const std::set<uint64_t>* excluded, int32_t transitionId)
{
    if (capiSubscriptions_.empty()) {
        return;
    }
    std::vector<std::shared_ptr<CapiSubscription>> batch;
    auto category = CategoryForType(type);
    const bool start = type == PAGE_START || type == SCROLL_START || type == SWIPER_START || type == TABS_START;
    const bool end = type == PAGE_END || type == SCROLL_END || type == SWIPER_END || type == TABS_END;
    const bool cancel = type == SWIPER_CANCEL || type == TABS_CANCEL;
    const bool isToast = overlayType && std::strcmp(overlayType, "toast") == 0;
    const auto startType = start ? type : (cancel ? type - CANCEL_EVENT_OFFSET : type - END_EVENT_OFFSET);
    const auto nodeId = type == PAGE_END && transitionId >= 0 ? transitionId : (target ? target->GetId() : -1);
    const auto key = std::make_pair(nodeId, startType);
    for (const auto& [id, record] : capiSubscriptions_) {
        if (excluded && excluded->count(id)) {
            continue;
        }
        const bool paired = record->starts.count(key) > 0;
        if (end || cancel) {
            record->starts.erase(key);
        }
        if (!(record->eventMask & category) || (IsStartType(type) && !record->withStart) ||
            ((pairedOnly || cancel) && !paired) || (start && paired) ||
            (!paired && (source & IGNORE_SCROLL_BY) && (record->ignoreMask & IGNORE_SCROLL_BY)) ||
            (!paired && (source & IGNORE_SCROLL_TO) && (record->ignoreMask & IGNORE_SCROLL_TO)) ||
            (isToast && (record->ignoreMask & IGNORE_TOAST))) {
            continue;
        }
        batch.emplace_back(record);
    }
    if (batch.empty()) {
        return;
    }
    auto timestamp = static_cast<uint64_t>(GetSysTimestamp());
    auto jsonText = BuildCapiContentChangeEventJson(type, target, overlayType, trigger, !pairedOnly);
    if (jsonText.size() > std::numeric_limits<uint32_t>::max()) {
        return;
    }
    if (start) {
        // Cache metadata only: START must not traverse children or keep a dying node alive.
        auto endJson = std::make_shared<const std::string>(
            BuildCapiContentChangeEventJson(type + END_EVENT_OFFSET, target, nullptr, nullptr, false));
        for (const auto& record : batch) {
            record->starts.emplace(key, endJson);
        }
    }
    CapiDelivery delivery { type, timestamp, {} };
    auto json = std::make_shared<const std::string>(std::move(jsonText));
    for (const auto& record : batch) {
        delivery.receivers.push_back({ record, json });
    }
    capiDeliveries_.emplace_back(std::move(delivery));
    DrainCapiDeliveries();
}

void ContentChangeManager::CapiImpl::DrainCapiDeliveries()
{
    if (capiDispatching_) {
        return;
    }
    capiDispatching_ = true;
    for (size_t index = 0; index < capiDeliveries_.size(); ++index) {
        // Callbacks may enqueue another event. Do not keep a reference into the growing vector.
        auto delivery = std::move(capiDeliveries_[index]);
        auto& receivers = delivery.receivers;
        receivers.erase(std::remove_if(receivers.begin(), receivers.end(),
            [](const auto& receiver) { return !receiver.subscription->active; }), receivers.end());
        for (const auto& receiver : receivers) {
            const auto& record = receiver.subscription;
            if (!record->active) {
                continue;
            }
            record->sink(delivery.type, delivery.timestamp, receiver.json->c_str(),
                static_cast<uint32_t>(receiver.json->size()), record->cookie);
        }
    }
    capiDeliveries_.clear();
    capiDispatching_ = false;
}

bool ContentChangeManager::CapiImpl::HasCapiContentChangeStart(int32_t nodeId, uint32_t type) const
{
    for (const auto& [id, record] : capiSubscriptions_) {
        if (record->starts.count({ nodeId, type })) {
            return true;
        }
    }
    return false;
}

void ContentChangeManager::CapiImpl::QueueCapiCachedEndEvent(int32_t nodeId, uint32_t startType)
{
    CapiDelivery delivery { startType + END_EVENT_OFFSET, static_cast<uint64_t>(GetSysTimestamp()), {} };
    for (const auto& [id, record] : capiSubscriptions_) {
        auto start = record->starts.find({ nodeId, startType });
        if (start == record->starts.end()) {
            continue;
        }
        delivery.receivers.push_back({ record, std::move(start->second) });
        record->starts.erase(start);
    }
    if (!delivery.receivers.empty()) {
        capiDeliveries_.emplace_back(std::move(delivery));
    }
}

bool ContentChangeManager::CapiImpl::SuppressCapiSwiperAutoPlay(const RefPtr<FrameNode>& node, bool hasTabsAncestor)
{
    if (!node) {
        return false;
    }
    auto paint = node->GetPaintProperty<SwiperPaintProperty>();
    if (!paint || !paint->GetAutoPlay().value_or(false)) {
        return false;
    }
    const auto key = std::make_pair(node->GetId(), hasTabsAncestor);
    auto target = GetSwiperContentChangeNode(node, hasTabsAncestor);
    auto active = capiActiveSwipers_.find(key);
    const auto targetId = active != capiActiveSwipers_.end() ? active->second.logicalNodeId : target->GetId();
    for (const auto& [id, record] : capiSubscriptions_) {
        record->starts.erase({ targetId, hasTabsAncestor ? TABS_START : SWIPER_START });
    }
    capiActiveSwipers_.erase(key);
    capiPendingSwipers_.erase(key);
    return true;
}

void ContentChangeManager::CapiImpl::ReportCapiSwiperStartEvent(const RefPtr<FrameNode>& node, bool hasTabsAncestor)
{
    if (!node || capiSubscriptions_.empty() || SuppressCapiSwiperAutoPlay(node, hasTabsAncestor)) {
        return;
    }
    auto target = GetSwiperContentChangeNode(node, hasTabsAncestor);
    auto key = std::make_pair(node->GetId(), hasTabsAncestor);
    for (const auto& [id, record] : capiSubscriptions_) {
        if (record->withStart && (record->eventMask & PAGE_CATEGORY)) {
            capiActiveSwipers_[key] = { target, target->GetId() };
            break;
        }
    }
    DispatchCapiContentChangeEvent(hasTabsAncestor ? TABS_START : SWIPER_START, target);
}

void ContentChangeManager::CapiImpl::QueueCapiSwiperEndEvent(
    const RefPtr<FrameNode>& node, bool hasTabsAncestor, bool pairedOnly)
{
    if (!node || capiSubscriptions_.empty() || SuppressCapiSwiperAutoPlay(node, hasTabsAncestor)) {
        return;
    }
    auto key = std::make_pair(node->GetId(), hasTabsAncestor);
    auto active = capiActiveSwipers_.find(key);
    auto target = GetSwiperContentChangeNode(node, hasTabsAncestor);
    if (active != capiActiveSwipers_.end()) {
        if (auto original = active->second.target.Upgrade()) {
            target = original;
        } else {
            QueueCapiCachedEndEvent(active->second.logicalNodeId, hasTabsAncestor ? TABS_START : SWIPER_START);
        }
    }
    auto pending = capiPendingSwipers_.find(key);
    if (pending != capiPendingSwipers_.end()) {
        // Preserve the cycle and its completed receivers; normal completion may broaden END-only delivery.
        pending->second->pairedOnly = pending->second->pairedOnly && pairedOnly;
        const auto startType = hasTabsAncestor ? TABS_START : SWIPER_START;
        for (const auto& [id, record] : capiSubscriptions_) {
            if (record->starts.count({ pending->second->logicalNodeId, startType })) {
                pending->second->completedSubscriptions.erase(id);
            }
        }
        return;
    }
    capiPendingSwipers_[key] = std::make_shared<CapiPendingSwiper>(
        CapiPendingSwiper { node, target, target->GetId(), pairedOnly, {} });
}

void ContentChangeManager::CapiImpl::ReportCapiSwiperCancelEvent(
    const RefPtr<FrameNode>& node, bool hasTabsAncestor, bool preserveNormalEnd)
{
    if (!node || SuppressCapiSwiperAutoPlay(node, hasTabsAncestor)) {
        return;
    }
    auto key = std::make_pair(node->GetId(), hasTabsAncestor);
    auto target = GetSwiperContentChangeNode(node, hasTabsAncestor);
    auto active = capiActiveSwipers_.find(key);
    if (active != capiActiveSwipers_.end()) {
        if (auto logical = active->second.target.Upgrade()) {
            target = logical;
        } else {
            QueueCapiCachedEndEvent(active->second.logicalNodeId, hasTabsAncestor ? TABS_START : SWIPER_START);
        }
        capiActiveSwipers_.erase(active);
    }
    auto pending = capiPendingSwipers_.find(key);
    if (preserveNormalEnd && pending != capiPendingSwipers_.end() && !pending->second->pairedOnly) {
        for (const auto& [id, record] : capiSubscriptions_) {
            if (record->starts.count({ target->GetId(), hasTabsAncestor ? TABS_START : SWIPER_START })) {
                pending->second->completedSubscriptions.insert(id);
            }
        }
    } else {
        capiPendingSwipers_.erase(key);
    }
    DispatchCapiContentChangeEvent(hasTabsAncestor ? TABS_CANCEL : SWIPER_CANCEL, target, NO_SCROLL_EVENT_SOURCE,
        nullptr, nullptr, true);
    DrainCapiDeliveries();
}

void ContentChangeManager::CapiImpl::FlushCapiSwiperEndEvents()
{
    // Keep entries revocable while callbacks consume this frame's snapshot.
    auto pending = capiPendingSwipers_;
    for (const auto& [key, transaction] : pending) {
        auto current = capiPendingSwipers_.find(key);
        if (current == capiPendingSwipers_.end() || current->second != transaction) {
            continue;
        }
        if (SuppressCapiSwiperAutoPlay(transaction->node.Upgrade(), key.second)) {
            continue;
        }
        capiPendingSwipers_.erase(current);
        auto active = capiActiveSwipers_.find(key);
        if (active != capiActiveSwipers_.end()) {
            bool hasNewStart = false;
            for (const auto id : transaction->completedSubscriptions) {
                auto record = capiSubscriptions_.find(id);
                hasNewStart = hasNewStart || (record != capiSubscriptions_.end() && record->second->starts.count(
                    { active->second.logicalNodeId, key.second ? TABS_START : SWIPER_START }));
            }
            if (!hasNewStart) {
                capiActiveSwipers_.erase(active);
            }
        }
        auto node = transaction->node.Upgrade();
        auto target = transaction->target.Upgrade();
        if (node && !target) {
            QueueCapiCachedEndEvent(transaction->logicalNodeId, key.second ? TABS_START : SWIPER_START);
            target = GetSwiperContentChangeNode(node, key.second);
        }
        if (node && target) {
            DispatchCapiContentChangeEvent(key.second ? TABS_END : SWIPER_END, target, NO_SCROLL_EVENT_SOURCE,
                nullptr, nullptr,
                transaction->pairedOnly, &transaction->completedSubscriptions);
        } else {
            QueueCapiCachedEndEvent(transaction->logicalNodeId, key.second ? TABS_START : SWIPER_START);
        }
    }
    DrainCapiDeliveries();
}

void ContentChangeManager::CapiImpl::ReportCapiScrollEndEvent(
    const RefPtr<FrameNode>& node, uint32_t source, bool pairedOnly)
{
    if (!node) {
        return;
    }
    capiScrollingNodes_.erase(node->GetId());
    auto marked = capiScrollSources_.find(node->GetId());
    if (marked != capiScrollSources_.end()) {
        if (!source) {
            source = marked->second;
        }
        capiScrollSources_.erase(marked);
    }
    DispatchCapiContentChangeEvent(SCROLL_END, node, source, nullptr, nullptr, pairedOnly);
}

void ContentChangeManager::CapiImpl::CompleteCapiContentChangeEventsOnNodeDestroyed(int32_t nodeId)
{
    capiScrollingNodes_.erase(nodeId);
    capiScrollSources_.erase(nodeId);
    if (capiSubscriptions_.empty()) {
        return;
    }
    std::set<std::pair<int32_t, uint32_t>> ends { { nodeId, PAGE_START }, { nodeId, SCROLL_START } };
    for (auto iter = capiActiveSwipers_.begin(); iter != capiActiveSwipers_.end();) {
        const auto targetId = iter->second.logicalNodeId;
        if (iter->first.first != nodeId && targetId != nodeId) {
            ++iter;
            continue;
        }
        const auto startType = iter->first.second ? TABS_START : SWIPER_START;
        auto pending = capiPendingSwipers_.find(iter->first);
        if (pending != capiPendingSwipers_.end()) {
            for (const auto& [id, record] : capiSubscriptions_) {
                if (record->starts.count({ targetId, startType })) {
                    pending->second->completedSubscriptions.insert(id);
                }
            }
        }
        ends.emplace(targetId, startType);
        iter = capiActiveSwipers_.erase(iter);
    }
    // Consume and enqueue every old pair before callbacks can create another cycle.
    for (const auto& [targetId, startType] : ends) {
        QueueCapiCachedEndEvent(targetId, startType);
    }
    DrainCapiDeliveries();
}

void ContentChangeManager::CapiImpl::ReportCapiScrollCommand(const RefPtr<FrameNode>& node, uint32_t source)
{
    if (!node || capiSubscriptions_.empty()) {
        return;
    }
    DispatchCapiContentChangeEvent(SCROLL_START, node, source);
    ReportCapiScrollEndEvent(node, source);
}

void ContentChangeManager::CapiImpl::MarkCapiScrollByEventSource(int32_t nodeId)
{
    capiScrollSources_[nodeId] = IGNORE_SCROLL_BY;
}

void ContentChangeManager::CapiImpl::MarkCapiScrollToEventSource(int32_t nodeId)
{
    // Operation provenance must also be available to subscribers added before completion.
    capiScrollSources_[nodeId] = IGNORE_SCROLL_TO;
}

void ContentChangeManager::CapiImpl::ClearCapiScrollEventSource(int32_t nodeId)
{
    capiScrollSources_.erase(nodeId);
}

void ContentChangeManager::CapiImpl::ReportCapiOverlayEvent(const RefPtr<FrameNode>& target, bool isShow,
    const char* overlayType, const RefPtr<FrameNode>& trigger)
{
    if (!target || !overlayType || capiSubscriptions_.empty()) {
        return;
    }
    DispatchCapiContentChangeEvent(isShow ? OVERLAY_SHOW : OVERLAY_HIDE, target, NO_SCROLL_EVENT_SOURCE,
        overlayType, trigger);
}

void ContentChangeManager::OnTransitionAdded(int32_t nodeId)
{
#ifndef CROSS_PLATFORM
    if (!NeedContentChangeReportOrPageSceneDetect()) {
        return;
    }
    transitioningNodes_.emplace(nodeId);
#endif
}

void ContentChangeManager::OnTransitionRemoved(int32_t nodeId)
{
    if (transitioningNodes_.count(nodeId)) {
        transitioningNodes_.erase(nodeId);
    }
}

void ContentChangeManager::OnSwiperScrollEnd(const RefPtr<FrameNode>& keyNode)
{
    CHECK_NULL_VOID(keyNode);
    ACE_SCOPED_TRACE("[ContentChangeManager] OnSwiperScrollEnd");
    scrollingSwiperNodes_.erase(keyNode->GetId());
}

void ContentChangeManager::OnContentChangeStart(const RefPtr<FrameNode>& node, ChangeType type)
{
    OnContentChangeStart(node, type, true);
}

void ContentChangeManager::OnContentChangeStart(const RefPtr<FrameNode>& node, ChangeType type, bool reportCapi)
{
#ifndef CROSS_PLATFORM
    OnContentChangeStartInner(node, type);
#endif
    if (reportCapi) {
        capiImpl_->OnContentChangeStart(node, type);
    }
}

void ContentChangeManager::CapiImpl::ReportCapiPageStartEvent(const RefPtr<FrameNode>& node)
{
    if (node) {
        DispatchCapiContentChangeEvent(PAGE_START, node);
    }
}

void ContentChangeManager::OnContentChangeInterrupted(const RefPtr<FrameNode>& node, ChangeType type)
{
#ifndef CROSS_PLATFORM
    OnContentChangeInterruptedInner(node, type);
#endif
    if (node && type == ChangeType::PAGE) {
        capiImpl_->DispatchCapiContentChangeEvent(PAGE_END, node, NO_SCROLL_EVENT_SOURCE, nullptr, nullptr, true);
    } else if (node && type == ChangeType::SCROLL) {
        capiImpl_->ReportCapiScrollEndEvent(node, NO_SCROLL_EVENT_SOURCE, true);
    }
}

void ContentChangeManager::OnContentChangeNodeDestroyed(int32_t nodeId)
{
#ifndef CROSS_PLATFORM
    OnContentChangeNodeDestroyedInner(nodeId);
#endif
    capiImpl_->CompleteCapiContentChangeEventsOnNodeDestroyed(nodeId);
}

void ContentChangeManager::OnPageTransitionEnd(const RefPtr<FrameNode>& keyNode)
{
    CHECK_NULL_VOID(keyNode);
    OnPageTransitionEnd(keyNode, keyNode->GetId());
}

void ContentChangeManager::OnPageTransitionEnd(const RefPtr<FrameNode>& keyNode, int32_t transitionId)
{
#ifndef CROSS_PLATFORM
    OnPageTransitionEndInner(keyNode, transitionId);
#endif
    ReportCapiPageEndEvent(keyNode, transitionId);
}

void ContentChangeManager::CapiImpl::ReportCapiPageEndEvent(const RefPtr<FrameNode>& keyNode, int32_t transitionId)
{
    if (keyNode) {
        DispatchCapiContentChangeEvent(PAGE_END, keyNode, NO_SCROLL_EVENT_SOURCE,
            nullptr, nullptr, false, nullptr, transitionId);
    } else {
        for (const auto& [id, record] : capiSubscriptions_) {
            record->starts.erase({ transitionId, PAGE_START });
        }
    }
}

void ContentChangeManager::OnScrollChangeEnd(const RefPtr<FrameNode>& keyNode, uint32_t type)
{
    OnScrollChangeEnd(keyNode, type, true);
}

void ContentChangeManager::OnScrollChangeEnd(const RefPtr<FrameNode>& keyNode, uint32_t type, bool reportCapi)
{
#ifndef CROSS_PLATFORM
    OnScrollChangeEndInner(keyNode, type);
#endif
    if (keyNode && reportCapi) {
        capiImpl_->ReportCapiScrollEndEvent(keyNode,
            type == SCROLL_TO || type == SCROLL_TO_INDEX ? IGNORE_SCROLL_TO : NO_SCROLL_EVENT_SOURCE);
    }
}

void ContentChangeManager::OnSwiperChangeStart(const RefPtr<FrameNode>& keyNode, bool hasTabsAncestor)
{
#ifndef CROSS_PLATFORM
    OnSwiperChangeStartInner(keyNode, hasTabsAncestor);
#endif
    ReportCapiSwiperStartEvent(keyNode, hasTabsAncestor);
}

void ContentChangeManager::OnSwiperChangeEnd(const RefPtr<FrameNode>& keyNode, bool hasTabsAncestor)
{
#ifndef CROSS_PLATFORM
    OnSwiperChangeEndInner(keyNode, hasTabsAncestor);
#endif
    capiImpl_->QueueCapiSwiperEndEvent(keyNode, hasTabsAncestor);
}

void ContentChangeManager::OnSwiperChangeCancel(const RefPtr<FrameNode>& keyNode, bool hasTabsAncestor)
{
#ifndef CROSS_PLATFORM
    OnSwiperChangeCancelInner(keyNode, hasTabsAncestor);
#endif
    capiImpl_->ReportCapiSwiperCancelEvent(keyNode, hasTabsAncestor);
}

void ContentChangeManager::OnDialogChangeEnd(const RefPtr<FrameNode>& keyNode, bool isShow)
{
    OnDialogChangeEnd(keyNode, isShow, true);
}

void ContentChangeManager::OnDialogChangeEnd(const RefPtr<FrameNode>& keyNode, bool isShow, bool reportCapi)
{
#ifndef CROSS_PLATFORM
    OnDialogChangeEndInner(keyNode, isShow);
#endif
    if (reportCapi) {
        ReportCapiOverlayEvent(keyNode, isShow, nullptr);
    }
}

void ContentChangeManager::OnVsyncEnd(const RectF& rootRect)
{
#ifndef CROSS_PLATFORM
    OnVsyncEndInner(rootRect);
#endif
    capiImpl_->FlushCapiSwiperEndEvents();
}

void ContentChangeManager::OnScrollRemoved(int32_t nodeId)
{
#ifndef CROSS_PLATFORM
    OnScrollRemovedInner(nodeId);
#endif
    capiImpl_->OnScrollRemoved(nodeId);
}

#ifdef CROSS_PLATFORM
ContentChangeManager::ContentChangeManager(const RefPtr<TaskExecutor>& taskExecutor)
    : capiImpl_(std::make_unique<CapiImpl>()), taskExecutor_(taskExecutor)
{}
#endif

void ContentChangeManager::OnScrollChangeStart(const RefPtr<FrameNode>& node)
{
#ifndef CROSS_PLATFORM
    OnScrollChangeStartInner(node);
#endif
    capiImpl_->OnScrollChangeStart(node);
}

void ContentChangeManager::OnSwiperScrollStart(const RefPtr<FrameNode>& node, bool hasTabsAncestor)
{
#ifndef CROSS_PLATFORM
    OnSwiperScrollStartInner(node);
#endif
    OnSwiperChangeStart(node, hasTabsAncestor);
}

void ContentChangeManager::OnScrollInterrupted(const RefPtr<FrameNode>& node, bool reportEnd)
{
#ifndef CROSS_PLATFORM
    if (reportEnd) {
        OnContentChangeInterruptedInner(node, ChangeType::SCROLL);
    }
#endif
    capiImpl_->ReportCapiScrollEndEvent(node, NO_SCROLL_EVENT_SOURCE, true);
}

void ContentChangeManager::CapiImpl::CompleteCapiSwiperChangeWithoutMove(
    const RefPtr<FrameNode>& node, bool hasTabsAncestor)
{
    CHECK_NULL_VOID(node);
    if (SuppressCapiSwiperAutoPlay(node, hasTabsAncestor)) {
        return;
    }
    auto target = GetSwiperContentChangeNode(node, hasTabsAncestor);
    if (HasCapiContentChangeStart(target->GetId(), hasTabsAncestor ? TABS_START : SWIPER_START)) {
        ReportCapiSwiperCancelEvent(node, hasTabsAncestor);
    }
}

void ContentChangeManager::CapiImpl::CompleteCapiSwiperChangeAfterAutoPlay(
    const RefPtr<FrameNode>& node, bool hasTabsAncestor, bool /* canceled */)
{
    // Enabling autoPlay discards unfinished manual events without synthesizing a terminal event.
    SuppressCapiSwiperAutoPlay(node, hasTabsAncestor);
}

void ContentChangeManager::OnSwiperChangeInterrupted(const RefPtr<FrameNode>& node, bool hasTabsAncestor)
{
    if (!node) {
        return;
    }
    auto paint = node->GetPaintProperty<SwiperPaintProperty>();
    if (paint && paint->GetAutoPlay().value_or(false)) {
        capiImpl_->QueueCapiSwiperEndEvent(node, hasTabsAncestor, true);
        return;
    }
#ifndef CROSS_PLATFORM
    if (IsStartEventReportEnabled()) {
        OnSwiperChangeEndInner(node, hasTabsAncestor);
    }
#endif
    capiImpl_->QueueCapiSwiperEndEvent(node, hasTabsAncestor, true);
}

ContentChangeManager::~ContentChangeManager() = default;

bool ContentChangeManager::HasCapiContentChangeSubscriptions() const
{
    return capiImpl_->HasCapiContentChangeSubscriptions();
}

void ContentChangeManager::CapiImpl::OnContentChangeStart(const RefPtr<FrameNode>& node, ChangeType type)
{
    if (!node) {
        return;
    }
    if (type == ChangeType::PAGE) {
        DispatchCapiContentChangeEvent(PAGE_START, node);
    } else if (type == ChangeType::SCROLL) {
        auto source = capiScrollSources_.find(node->GetId());
        DispatchCapiContentChangeEvent(SCROLL_START, node,
            source == capiScrollSources_.end() ? NO_SCROLL_EVENT_SOURCE : source->second);
    }
}

void ContentChangeManager::CapiImpl::OnScrollRemoved(int32_t nodeId)
{
    capiScrollingNodes_.erase(nodeId);
    QueueCapiCachedEndEvent(nodeId, SCROLL_START);
    capiScrollSources_.erase(nodeId);
    DrainCapiDeliveries();
}

void ContentChangeManager::CapiImpl::OnScrollChangeStart(const RefPtr<FrameNode>& node)
{
    if (node) {
        capiScrollingNodes_.insert(node->GetId());
    }
}

uint64_t ContentChangeManager::RegisterCapiContentChangeEvent(bool withStart, uint32_t eventMask, uint32_t ignoreMask,
    void* cookie, CapiSink sink, void (*release)(void*))
{
    return capiImpl_->RegisterCapiContentChangeEvent(withStart, eventMask, ignoreMask, cookie, sink, release);
}

bool ContentChangeManager::UnregisterCapiContentChangeEvent(uint64_t subscriptionId)
{
    return capiImpl_->UnregisterCapiContentChangeEvent(subscriptionId);
}

void ContentChangeManager::InvalidateCapiContentChangeSubscriptions()
{
    capiImpl_->InvalidateCapiContentChangeSubscriptions();
}

void ContentChangeManager::ReportCapiPageStartEvent(const RefPtr<FrameNode>& node)
{
    capiImpl_->ReportCapiPageStartEvent(node);
}

void ContentChangeManager::ReportCapiPageEndEvent(const RefPtr<FrameNode>& keyNode, int32_t transitionId)
{
    capiImpl_->ReportCapiPageEndEvent(keyNode, transitionId);
}

void ContentChangeManager::ReportCapiSwiperStartEvent(const RefPtr<FrameNode>& node, bool hasTabsAncestor)
{
    capiImpl_->ReportCapiSwiperStartEvent(node, hasTabsAncestor);
}

void ContentChangeManager::CompleteCapiSwiperChangeWithoutMove(const RefPtr<FrameNode>& node, bool hasTabsAncestor)
{
    capiImpl_->CompleteCapiSwiperChangeWithoutMove(node, hasTabsAncestor);
}

void ContentChangeManager::CompleteCapiSwiperChangeAfterAutoPlay(
    const RefPtr<FrameNode>& node, bool hasTabsAncestor, bool canceled)
{
    capiImpl_->CompleteCapiSwiperChangeAfterAutoPlay(node, hasTabsAncestor, canceled);
}

void ContentChangeManager::ReportCapiOverlayEvent(const RefPtr<FrameNode>& target, bool isShow,
    const char* overlayType, const RefPtr<FrameNode>& trigger)
{
    capiImpl_->ReportCapiOverlayEvent(target, isShow, overlayType, trigger);
}

void ContentChangeManager::ReportCapiScrollCommand(const RefPtr<FrameNode>& node, uint32_t source)
{
    capiImpl_->ReportCapiScrollCommand(node, source);
}

void ContentChangeManager::MarkCapiScrollByEventSource(int32_t nodeId)
{
    capiImpl_->MarkCapiScrollByEventSource(nodeId);
}

void ContentChangeManager::MarkCapiScrollToEventSource(int32_t nodeId)
{
    capiImpl_->MarkCapiScrollToEventSource(nodeId);
}

void ContentChangeManager::ClearCapiScrollEventSource(int32_t nodeId)
{
    capiImpl_->ClearCapiScrollEventSource(nodeId);
}
} // namespace OHOS::Ace::NG
