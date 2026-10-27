/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_CONTENT_CHANGE_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_CONTENT_CHANGE_MANAGER_H

#include "base/geometry/ng/rect_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/thread/task_executor.h"
#include "base/utils/macros.h"
#include "interfaces/inner_api/ui_session/param_config.h"

#include <memory>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

namespace OHOS::Ace::NG {
class FrameNode;
class PageTranslateNode;
#ifndef IS_RELEASE_VERSION
class ContentChangeDumpManager;
#endif

class ACE_FORCE_EXPORT ContentChangeManager final : public AceType {
    DECLARE_ACE_TYPE(ContentChangeManager, AceType);
public:
    enum ContentIgnoreEventType : uint32_t {
        NONE = 0,                 // no event
        SCROLL_TO = 1 << 0,       // SCROLL: scrollTo
        SCROLL_TO_INDEX = 1 << 1, // SCROLL: scrollToIndex
        ARKWEB_ALL = 1 << 2,      // ARKWEB: block all web events
    };

    explicit ContentChangeManager(const RefPtr<TaskExecutor>& taskExecutor = nullptr);
    ~ContentChangeManager() override;
    void StartContentChangeReport(const ContentChangeConfig& config);
    void StopContentChangeReport();
    void AddOnContentChangeNode(WeakPtr<FrameNode> node);
    void RemoveOnContentChangeNode(WeakPtr<FrameNode> node);
    void StartTextTranslateReport();
    void StopTextTranslateReport();
    void StartTextTranslateSnapshotReport();
    void ReportTranslateTextNode(const WeakPtr<PageTranslateNode>& node, const std::string& text);
    void ReportTranslateTextFrameNode(const WeakPtr<FrameNode>& node, bool isContinuous = true);
    bool ApplyTranslateResult(int32_t nodeId, const std::string& result, int64_t version);
    void ResetTranslateTextNode(int32_t nodeId = -1);
    bool IsContentChangeDetectEnable() const
    {
        return currentContentChangeConfig_.has_value();
    }

    bool IsStartEventReportEnabled() const
    {
        return currentContentChangeConfig_.has_value() && currentContentChangeConfig_->reportStartEvent;
    }

    void OnPageTransitionEnd(const RefPtr<FrameNode>& keyNode);
    void OnPageTransitionEnd(const RefPtr<FrameNode>& keyNode, int32_t transitionId, bool reportCapi = true);
    void ReportCapiPageEndEvent(const RefPtr<FrameNode>& keyNode, int32_t transitionId);
    void ReportCapiPageEndEventFromCache(int32_t transitionId);
    std::function<void()> BuildCapiOverlayContentHideCallback(const RefPtr<FrameNode>& target);
    void OnContentChangeStart(const RefPtr<FrameNode>& keyNode, ChangeType type);
    void OnContentChangeStart(const RefPtr<FrameNode>& keyNode, ChangeType type, bool reportCapi);
    void ReportCapiPageStartEvent(const RefPtr<FrameNode>& node);
    void OnContentChangeInterrupted(const RefPtr<FrameNode>& keyNode, ChangeType type, bool reportCapiEnd = true);
    void OnContentChangeNodeDestroyed(int32_t nodeId);
    void OnScrollChangeStart(const RefPtr<FrameNode>& keyNode, bool reportCapi = true);
    void OnScrollChangeEnd(const RefPtr<FrameNode>& keyNode, uint32_t type = NONE);
    void OnScrollChangeEnd(const RefPtr<FrameNode>& keyNode, uint32_t type, bool reportCapi);
    void OnSwiperChangeStart(const RefPtr<FrameNode>& keyNode, bool hasTabsAncestor);
    void ReportCapiSwiperStartEvent(const RefPtr<FrameNode>& node, bool hasTabsAncestor);
    void OnSwiperChangeCancel(const RefPtr<FrameNode>& keyNode, bool hasTabsAncestor);
    void CompleteCapiSwiperChangeWithoutMove(const RefPtr<FrameNode>& node, bool hasTabsAncestor);
    void CompleteCapiSwiperChangeAfterAutoPlay(const RefPtr<FrameNode>& node, bool hasTabsAncestor, bool canceled);
    void OnSwiperChangeEnd(const RefPtr<FrameNode>& keyNode, bool hasTabsAncestor);
    ACE_FORCE_EXPORT void OnDialogChangeEnd(const RefPtr<FrameNode>& keyNode, bool isShow);
    void OnDialogChangeEnd(const RefPtr<FrameNode>& keyNode, bool isShow, bool reportCapi);
    void OnScrollRemoved(int32_t nodeId);
    void OnTransitionAdded(int32_t nodeId);
    void OnTransitionRemoved(int32_t nodeId);
    bool IsScrolling() const;
    bool IsTransitioning() const;
    void OnTextChangeEnd(const RectF& rect, const RectF& rootRect);
    void OnVsyncStart();
    void OnVsyncEnd(const RectF& rootRect);
    bool IsTextAABBCollecting() const;
    void OnImageChangeEnd(const WeakPtr<FrameNode>& keyNode, const std::string& sourceType, const RectF& rootRect);

    uint32_t ConvertEventStringToEnum(const std::string& type) const;
    uint32_t GetIgnoreEventMask(const std::string& ignoreEventType) const;
    bool IsIgnoringEventType(uint32_t type) const;

    bool IsSwiperScrolling() const;
    void OnSwiperScrollStart(const RefPtr<FrameNode>& keyNode, bool hasTabsAncestor = false);
    void OnSwiperScrollEnd(const RefPtr<FrameNode>& keyNode);

    using CapiSink = void (*)(uint32_t, uint64_t, const char*, uint32_t, void*);
    uint64_t RegisterCapiContentChangeEvent(bool withStart, uint32_t eventMask, uint32_t ignoreMask, void* cookie,
        CapiSink sink, void (*release)(void*));
    bool UnregisterCapiContentChangeEvent(uint64_t subscriptionId);
    bool HasCapiContentChangeSubscriptions() const;
    void InvalidateCapiContentChangeSubscriptions();
    void OnScrollInterrupted(const RefPtr<FrameNode>& node, bool reportEnd);
    void OnSwiperChangeInterrupted(const RefPtr<FrameNode>& node, bool hasTabsAncestor);
    void ReportCapiOverlayEvent(const RefPtr<FrameNode>& target, bool isShow, const char* overlayType,
        const RefPtr<FrameNode>& trigger = nullptr);
    void ReportCapiScrollStartEvent(const RefPtr<FrameNode>& node);
    void ReportCapiScrollEndEvent(const RefPtr<FrameNode>& node);
    void ReportCapiScrollCommand(const RefPtr<FrameNode>& node, uint32_t source);
    void MarkCapiScrollByEventSource(int32_t nodeId);
    void MarkCapiScrollToEventSource(int32_t nodeId);
    void ClearCapiScrollEventSource(int32_t nodeId);

#ifndef IS_RELEASE_VERSION
    std::string DumpInfo() const;
#endif

    // PageScene detection entry points. Exposed so route-change hook points
    // (PagePattern::OnShow, NavDestinationEventHub::FireOnShownEvent) can
    // trigger an immediate detection cycle for AC-6.2/AC-6.4 (route-leave
    // false event, A-false-before-B-true ordering). No-op when no search
    // ruleSet is registered (NeedPageSceneDetect fast-returns).
    void NotifyPageSceneContentChanged(bool flushNow);
    void FlushPageSceneNodeChanged();

private:
    class CapiImpl;
    std::unique_ptr<CapiImpl> capiImpl_;

    static RefPtr<FrameNode> GetSwiperContentChangeNode(const RefPtr<FrameNode>& node, bool hasTabsAncestor);
    void OnScrollChangeStartInner(const RefPtr<FrameNode>& node);
    void OnSwiperScrollStartInner(const RefPtr<FrameNode>& node);
    void OnContentChangeNodeDestroyedInner(int32_t nodeId);
    void OnContentChangeStartInner(const RefPtr<FrameNode>& keyNode, ChangeType type);
    void OnContentChangeInterruptedInner(const RefPtr<FrameNode>& keyNode, ChangeType type);
    void OnPageTransitionEndInner(const RefPtr<FrameNode>& keyNode, int32_t transitionId);
    void OnScrollChangeEndInner(const RefPtr<FrameNode>& keyNode, uint32_t type);
    void OnSwiperChangeStartInner(const RefPtr<FrameNode>& keyNode, bool hasTabsAncestor);
    void OnSwiperChangeEndInner(const RefPtr<FrameNode>& keyNode, bool hasTabsAncestor);
    void OnSwiperChangeCancelInner(const RefPtr<FrameNode>& keyNode, bool hasTabsAncestor);
    void OnDialogChangeEndInner(const RefPtr<FrameNode>& keyNode, bool isShow);
    void OnVsyncEndInner(const RectF& rootRect);
    void OnScrollRemovedInner(int32_t nodeId);

    struct ActiveContentChange {
        ChangeType endType = ChangeType::PAGE;
        std::string tag;
        int32_t nodeId = -1;
        int64_t startTimestamp = 0;
        bool normalEndPending = false;
        int32_t pendingReportNodeId = -1;
    };

    struct PendingSwiperChange {
        int32_t logicalNodeId = -1;
        bool canceled = false;
        WeakPtr<FrameNode> reportNode;
    };

    std::optional<ChangeType> GetStartChangeType(ChangeType type) const;
    void CompleteContentChange(int32_t nodeId, ChangeType type);
    void MarkSwiperNormalEndPending(
        const RefPtr<FrameNode>& reportNode, const RefPtr<FrameNode>& logicalNode, ChangeType type);
    bool ConsumeCanceledSwiperEnd(int32_t reportNodeId, ChangeType type);
    void CompleteSwiperContentChange(int32_t reportNodeId, ChangeType type);
    void CancelPendingSwiperEnd(const ActiveContentChange& activeChange);
    void ReportContentChangeEnd(const ActiveContentChange& activeChange, const char* endReason = nullptr);
    void SetContentChangeConfig(const ContentChangeConfig& config);
    void UpdateContentChangeParameters();
    void ResetContentChangeState();
    void RegisterContentChangeNodes(const ContentChangeConfig& config);
    void ProcessSwiperNodes();
    void ReportSwiperEvent(const RefPtr<FrameNode>& node, bool hasTabsAncestor);
    void StartTextAABBCollecting();
    void StopTextAABBCollecting(const RectF& rootRect);
    bool IsContentChanging() const;
    bool NeedPageSceneDetect() const;
    bool NeedContentChangeReportOrPageSceneDetect() const;
    void ReportImageEvent();
    void RemoveImageReportTask();
    void PostImageReportTask(uint32_t delay);
    bool IsInTransitionDelayWindow() const;
    bool IsTextTranslateActive() const;
    int64_t UpdateTranslateVersionIfNeeded(int32_t nodeId, const std::string& text);
    void ReportTranslateTextSnapshotNode(const WeakPtr<PageTranslateNode>& node, const std::string& text);
    void ClearTranslateTextSnapshotCache();
    int64_t NextTranslateTextSnapshotVersion();
    bool HasTranslateResultVersion(const std::unordered_map<int32_t, std::pair<size_t, int64_t>>& versions,
        int32_t nodeId, int64_t version) const;
    bool ApplyTranslateResultToNode(const RefPtr<PageTranslateNode>& node, const std::string& result, int64_t version);
    bool ApplyTranslateResultWithCache(const std::set<WeakPtr<PageTranslateNode>>& nodes,
        const std::unordered_map<int32_t, std::pair<size_t, int64_t>>& versions, int32_t nodeId,
        const std::string& result, int64_t version);
    void ResetTranslateNode(const RefPtr<PageTranslateNode>& node);
    void ResetTranslateNodes(std::set<WeakPtr<PageTranslateNode>>& nodes, int32_t nodeId);

    std::set<WeakPtr<FrameNode>> onContentChangeNodes_;
    std::set<WeakPtr<PageTranslateNode>> translateTextNodes_;
    std::unordered_map<int32_t, std::pair<size_t, int64_t>> translateTextVersions_;
    std::set<WeakPtr<PageTranslateNode>> translateTextSnapshotNodes_;
    std::unordered_map<int32_t, std::pair<size_t, int64_t>> translateTextSnapshotVersions_;
    int64_t translateTextSnapshotVersion_ = 0;
    bool textTranslateActive_ = false;
    std::optional<ContentChangeConfig> currentContentChangeConfig_;

    static constexpr uint64_t NS_PER_MS = 1000000;
    static constexpr float DEFAULT_TEXT_CONTENT_RATIO = 0.15f;
    static constexpr int32_t DEFAULT_COMPONENT_MIN_REPORT_TIME = 100;
    static constexpr int32_t DEFAULT_IMAGE_MIN_WIDTH = 100;
    static constexpr int32_t DEFAULT_IMAGE_MIN_HEIGHT = 100;
    static constexpr int32_t DEFAULT_COMPONENT_REPORT_DELAY_TIME = 600;
    float textContentRatio_ = 0.15f; // default text content ratio is 0.15
    uint64_t componentReportInterval_ = 100 * NS_PER_MS; // minimum component content change interval is 100 ms
    uint32_t ignoreEventMask_ = NONE; // default ignore event mask is no event
    int32_t imageMinWidth_ = 100; // default image minimum width is 100px
    int32_t imageMinHeight_ = 100; // default image minimum height is 100px
    uint64_t componentReportDelayTime_ = 600 * NS_PER_MS; // default component report delay time is 600 ms
    uint64_t lastComponentReportTime_ = 0;  // Timestamp of last component event report
    uint64_t lastTransitionReportTime_ = 0; // Timestamp of last transition event report
    bool textCollecting_ = false;
    std::unordered_map<int32_t, std::unordered_map<ChangeType, ActiveContentChange>> activeContentChanges_;
    std::unordered_map<int32_t, std::unordered_map<ChangeType, PendingSwiperChange>> pendingSwiperChanges_;
    std::set<std::pair<WeakPtr<FrameNode>, bool>> changedSwiperNodes_;
    std::set<int32_t> scrollingNodes_;
    std::set<int32_t> scrollingSwiperNodes_;
    std::set<int32_t> transitioningNodes_;
    RectF textAABB_; // Axis-aligned bounding box(AABB) of Text rects.
    std::vector<std::tuple<int32_t, RectF, std::string>> imageChangeList_;
    RefPtr<TaskExecutor> taskExecutor_;
    std::string imageChangeTaskName_ = "ImageContentChange";
    std::string imageReportTaskName_ = "ImageContentChangeReport";
#ifndef IS_RELEASE_VERSION
    std::shared_ptr<ContentChangeDumpManager> dumpMgr_;
#endif
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_CONTENT_CHANGE_MANAGER_H
