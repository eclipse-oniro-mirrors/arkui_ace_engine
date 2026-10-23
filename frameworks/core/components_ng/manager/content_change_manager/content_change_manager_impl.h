/**
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_CONTENT_CHANGE_MANAGER_IMPL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_CONTENT_CHANGE_MANAGER_IMPL_H

#include <atomic>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "content_change_manager.h"

namespace OHOS::Ace::NG {
// Internal C API state; include only from implementation files and white-box tests.
class ContentChangeManager::CapiImpl final {
public:
    uint64_t RegisterCapiContentChangeEvent(bool withStart, uint32_t eventMask, uint32_t ignoreMask, void* cookie,
        CapiSink sink, void (*release)(void*));
    bool UnregisterCapiContentChangeEvent(uint64_t subscriptionId);
    void InvalidateCapiContentChangeSubscriptions();
    void ReportCapiPageStartEvent(const RefPtr<FrameNode>& node);
    void ReportCapiPageEndEvent(const RefPtr<FrameNode>& keyNode, int32_t transitionId);
    void ReportCapiSwiperStartEvent(const RefPtr<FrameNode>& node, bool hasTabsAncestor);
    void CompleteCapiSwiperChangeWithoutMove(const RefPtr<FrameNode>& node, bool hasTabsAncestor);
    void CompleteCapiSwiperChangeAfterAutoPlay(const RefPtr<FrameNode>& node, bool hasTabsAncestor, bool canceled);
    void ReportCapiOverlayEvent(const RefPtr<FrameNode>& target, bool isShow, const char* overlayType,
        const RefPtr<FrameNode>& trigger = nullptr);
    void ReportCapiScrollCommand(const RefPtr<FrameNode>& node, uint32_t source);
    void MarkCapiScrollByEventSource(int32_t nodeId);
    void MarkCapiScrollToEventSource(int32_t nodeId);
    void ClearCapiScrollEventSource(int32_t nodeId);
    bool HasCapiContentChangeSubscriptions() const
    {
        return !capiSubscriptions_.empty();
    }
    void OnContentChangeStart(const RefPtr<FrameNode>& node, ChangeType type);
    void OnScrollRemoved(int32_t nodeId);
    void OnScrollChangeStart(const RefPtr<FrameNode>& node);

private:
    friend class ContentChangeManager;

    void DispatchCapiContentChangeEvent(uint32_t type, const RefPtr<FrameNode>& target, uint32_t source = 0,
        const char* overlayType = nullptr, const RefPtr<FrameNode>& trigger = nullptr,
        bool pairedOnly = false, const std::set<uint64_t>* excluded = nullptr, int32_t transitionId = -1);
    void DrainCapiDeliveries();
    bool HasCapiContentChangeStart(int32_t nodeId, uint32_t type) const;
    bool SuppressCapiSwiperAutoPlay(const RefPtr<FrameNode>& node, bool hasTabsAncestor);
    void QueueCapiCachedEndEvent(int32_t nodeId, uint32_t startType);
    void QueueCapiSwiperEndEvent(const RefPtr<FrameNode>& node, bool hasTabsAncestor, bool pairedOnly = false);
    void ReportCapiSwiperCancelEvent(
        const RefPtr<FrameNode>& node, bool hasTabsAncestor, bool preserveNormalEnd = false);
    void FlushCapiSwiperEndEvents();
    void ReportCapiScrollEndEvent(const RefPtr<FrameNode>& node, uint32_t source, bool pairedOnly = false);
    void CompleteCapiContentChangeEventsOnNodeDestroyed(int32_t nodeId);

    struct CapiSubscription {
        bool active = true;
        bool withStart = false;
        uint32_t eventMask = 0;
        uint32_t ignoreMask = 0;
        void* cookie = nullptr;
        CapiSink sink = nullptr;
        // Accepted activity for both listener modes, independent of START delivery; never retains the target node.
        std::map<std::pair<int32_t, uint32_t>, std::shared_ptr<const std::string>> starts;
        void (*release)(void*) = nullptr;
        ~CapiSubscription()
        {
            if (release) {
                release(cookie);
            }
        }
    };

    struct CapiDelivery {
        uint32_t type;
        uint64_t timestamp;
        struct Receiver {
            std::shared_ptr<CapiSubscription> subscription;
            std::shared_ptr<const std::string> json;
        };
        std::vector<Receiver> receivers;
    };
    std::vector<CapiDelivery> capiDeliveries_;
    bool capiDispatching_ = false;
    std::map<uint64_t, std::shared_ptr<CapiSubscription>> capiSubscriptions_;
    std::set<int32_t> capiScrollingNodes_;
    static std::atomic<uint64_t> nextCapiSubscriptionId_;
    struct CapiActiveSwiper {
        WeakPtr<FrameNode> target;
        int32_t logicalNodeId = -1;
    };
    std::map<std::pair<int32_t, bool>, CapiActiveSwiper> capiActiveSwipers_;
    struct CapiPendingSwiper {
        WeakPtr<FrameNode> node;
        WeakPtr<FrameNode> target;
        int32_t logicalNodeId = -1;
        bool pairedOnly = false;
        std::set<uint64_t> completedSubscriptions;
    };
    std::map<std::pair<int32_t, bool>, std::shared_ptr<CapiPendingSwiper>> capiPendingSwipers_;
    std::map<int32_t, uint32_t> capiScrollSources_;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_CONTENT_CHANGE_MANAGER_IMPL_H
