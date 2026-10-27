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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SCROLL_PLACEHOLDER_SCROLL_PLACEHOLDER_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SCROLL_PLACEHOLDER_SCROLL_PLACEHOLDER_MANAGER_H

#include <array>
#include <atomic>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

#include "base/memory/ace_type.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_cost_model.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_template_registry.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_types.h"

namespace OHOS::Ace::NG {

class PipelineContext;
class FrameNode;

// Prediction request for one real item about to be created inside a scroll container.
struct ScrollPlaceholderPredictParams {
    ScrollPlaceholderComponentType componentType = ScrollPlaceholderComponentType::LIST;
    // Placeholder template the item falls back to; empty means the default placeholder bucket.
    std::string templateId;
    int32_t hostNodeId = -1;
    int32_t index = -1;
    // True when the build is requested by the cache/preload path instead of the visible layout.
    bool cacheBuild = false;
};

// Prediction outcome, kept alongside the inputs of the decision for trace/diagnostics.
struct ScrollPlaceholderPredictResult {
    ScrollPlaceholderDecision decision = ScrollPlaceholderDecision::BUILD_REAL_NOW;
    int64_t estimateNs = 0;
    int64_t remainingBudgetNs = 0;
    // False when no frame anchor was available yet; the decision then keeps the legacy path
    // and must not be read as a budget-based suggestion.
    bool budgetAvailable = false;
};

// Per pipeline scroll placeholder scheduler core:
// - lazy created by PipelineContext, isolated per container instance;
// - predicts, before the real builder runs, whether the estimated item build cost fits the
//   remaining frame budget anchored to the current vsync period;
// - owns the UI-thread framed real-build queue: priority ordered, budget checked before each
//   item, requeue + request frame once the next estimate exceeds the remaining budget;
// - template registry lifecycle (register/unregister/query + instance LRU) and diagnostics
//   counters are surfaced through this facade.
// Threading: created, called and destroyed on the UI thread only.
// ACE_FORCE_EXPORT: the List/Grid/WaterFlow split .so libraries reference
// NotifyRealBuildStart/End through the observation scope header, so the class
// must keep default visibility to be exportable via build/libace.map.
class ACE_FORCE_EXPORT ScrollPlaceholderManager : public AceType {
    DECLARE_ACE_TYPE(ScrollPlaceholderManager, AceType);

public:
    explicit ScrollPlaceholderManager(int32_t instanceId);
    ~ScrollPlaceholderManager() override = default;

    void SetPipelineContext(const WeakPtr<PipelineContext>& pipeline);
    // Frame anchor pushed from PipelineContext::FlushVsync; period <= 0 falls back to 60Hz.
    void NotifyVsync(int64_t vsyncTimestampNs, int64_t vsyncPeriodNs);

    // Template registry facade.
    uint64_t RegisterTemplate(const std::string& templateId, ScrollPlaceholderBuilder&& builder);
    bool UnregisterTemplate(const std::string& templateId);
    bool IsTemplateRegistered(const std::string& templateId) const;
    // Registration probe: synchronously runs one restricted-protocol subtree
    // clone of the resident source instance on the UI thread (validation is the
    // clone itself — same path production clones take). Returns false when any node
    // of the template is not copyable, with the offending subtree path in
    // subtreePath for diagnostics; on success the probe product is cached into the
    // instance pool. The template itself stays registered either way: the caller
    // (registration bridge) unregisters and reports the rejection.
    bool ProbeTemplateClone(const std::string& templateId, std::string& subtreePath);
    std::optional<ScrollPlaceholderTemplateSnapshot> LookupTemplateForCreate(const std::string& templateId);
    // Resident copy source of the template (built by the builder on the UI thread); never
    // handed out, only traversed and copied by background clone tasks.
    void SetTemplateSourceInstance(const std::string& templateId, const RefPtr<UINode>& instance);
    RefPtr<UINode> GetTemplateSourceInstance(const std::string& templateId) const;
    void CacheTemplateInstance(const std::string& templateId, const RefPtr<UINode>& instance);
    // Always executes the template builder synchronously on the UI thread and returns
    // the product. The instance pool stays maintained (registration seeds it, retirement
    // returns content to it) but is not consumed on acquire: pooled clones carry layout
    // properties only (no paint properties), which visibly degrades the placeholder.
    RefPtr<UINode> AcquireTemplateInstance(const std::string& templateId);
    size_t GetCachedInstanceCount(const std::string& templateId) const;
    size_t GetHotTemplateCount() const;
    // Background clone tasks submitted but not finished yet for the template.
    size_t GetPendingCloneCount(const std::string& templateId) const;

    // Prediction core.
    ScrollPlaceholderPredictResult Predict(const ScrollPlaceholderPredictParams& params, int64_t frameDeadlineNs) const;
    int64_t EstimateRealBuildDuration(const ScrollPlaceholderPredictParams& params) const;
    int64_t GetRemainingFrameBudgetNs(int64_t frameDeadlineNs) const;
    int64_t GetVsyncDerivedFrameDeadlineNs() const;
    void RecordRealBuildDuration(const ScrollPlaceholderPredictParams& params, int64_t durationNs);

    // Framed real-build queue. The task runs on the UI thread and returns whether the real
    // item was built and committed; its wall duration is sampled into the cost model.
    using RealBuildTaskFunc = std::function<bool()>;
    void EnqueueRealBuild(const ScrollPlaceholderPredictParams& params, uint64_t templateGeneration,
        RealBuildTaskFunc&& task, ScrollPlaceholderRealBuildPriority priority);
    void FlushRealBuild(int64_t deadlineNs);
    bool HasPendingRealBuild() const;
    size_t GetPendingRealBuildCount() const;
    void CancelHostRealBuilds(int32_t hostNodeId, ScrollPlaceholderCancelReason reason);

    // Lifecycle.
    void Destroy();
    bool IsDestroyed() const;

    // ---- Acquisition deferral table (the index->wrapper substitution itself
    // executes in FrameProxy::GetFrameNodeByIndex, one choke point above every lazy
    // syntax node). The container pattern stays the decision side: on defer it mounts
    // the shell and records (hostId, index) -> shell here; FrameProxy queries the table
    // on every non-cache index acquisition and returns the shell as the index's current
    // occupant until the real build commits and the entry is cleared. Registered shells
    // never enter FrameProxy's partFrameNodeChildren_: the index slot stays reserved
    // for the real item. Threading: UI thread only.
    // Records one live deferral; replaces any previous shell for the same (hostId, index).
    void RecordDeferral(int32_t hostNodeId, int32_t index, const RefPtr<FrameNode>& shell);
    // Drops the deferral so the index resolves through the normal acquisition again.
    void ClearDeferral(int32_t hostNodeId, int32_t index);
    // Drops every deferral whose recorded shell is the given node (the retirement
    // funnel covers paths that do not know the index, e.g. the orphan sweep).
    void RemoveDeferralsByShell(int32_t hostNodeId, const RefPtr<FrameNode>& shell);
    // Returns the deferred shell for the index, or null when nothing is deferred (a
    // dead weak reference is treated as no deferral and swept).
    RefPtr<FrameNode> QueryDeferredShell(int32_t hostNodeId, int32_t index);
    // Process-wide fast gate for the FrameProxy hot path: false when no deferral was
    // ever recorded and not yet cleared, letting non-placeholder acquisitions skip the
    // context/manager lookup entirely.
    static bool HasAnyDeferrals();

    // Observation hooks for scroll container item builds (no behavior impact: the real item
    // is still built synchronously; these only sample its cost and let the prediction say,
    // per frame, how many items it would have built now and how many it would defer to
    // placeholders once the feature is enabled by the component adapters).
    ScrollPlaceholderPredictResult NotifyRealBuildStart(const ScrollPlaceholderPredictParams& params);
    void NotifyRealBuildEnd(const ScrollPlaceholderPredictParams& params, int64_t buildStartNs);

    // Aggregated observation of the frame closed by the most recent NotifyVsync.
    struct FrameObservationStats {
        uint32_t observedBuilds = 0;
        uint32_t suggestedReal = 0;
        uint32_t suggestedPlaceholder = 0;
        int64_t totalBuildNs = 0;
        int64_t maxBuildNs = 0;
        int64_t lastRemainingBudgetNs = 0;
    };
    FrameObservationStats GetLastFrameObservation(ScrollPlaceholderComponentType componentType) const;

    // Diagnostics counters (relaxed atomics, snapshot copy for dump/trace).
    static constexpr size_t CANCEL_REASON_COUNT = 8;
    static constexpr size_t COMPONENT_TYPE_COUNT = 3;
    struct Diagnostics {
        uint64_t predictBuildRealNow = 0;
        uint64_t predictUsePlaceholder = 0;
        uint64_t realBuildExecuted = 0;
        uint64_t realBuildRequeued = 0;
        uint64_t realBuildDropped = 0;
        uint64_t registerRejected = 0;
        uint64_t lruEvictedTemplates = 0;
        uint64_t flushRounds = 0;
        uint64_t requestFrameCount = 0;
        uint64_t observedRealBuilds = 0;
        std::array<uint64_t, CANCEL_REASON_COUNT> cancelledByReason = {};
    };
    Diagnostics GetDiagnostics() const;

protected:
    // Schedules one clone task through the shared BackgroundTaskExecutor (low priority: on an
    // empty pool the acquire path degrades to synchronous creation, so a clone never blocks
    // user-visible work). Returns false when the executor rejected the task; the caller then
    // releases the pending slot. Virtual so tests can capture and drain tasks deterministically.
    virtual bool SubmitBackgroundCloneTask(std::function<void()> task);

    // Materializes one node of the immutable source subtree during a background traversal
    // clone: creates a same-type node through the node creation interface and copies the
    // source properties over. The template builder itself is a JS function bound to the JS VM
    // and must only run on the UI thread, so the background copy never calls it; the caller
    // wraps the traversal in a thread-safe node scope so created nodes register into the
    // mutex-guarded multi-thread registry. The compiler restricted factory contract (step2)
    // extends this hook for pattern-specific state and JS custom nodes.
    virtual RefPtr<UINode> CreatePlaceholderNodeCopy(const RefPtr<UINode>& node);

private:
    struct RealBuildTask {
        ScrollPlaceholderPredictParams params;
        uint64_t templateGeneration = 0;
        uint64_t sequence = 0;
        ScrollPlaceholderRealBuildPriority priority = ScrollPlaceholderRealBuildPriority::VISIBLE;
        RealBuildTaskFunc task;
    };

    ScrollPlaceholderPredictResult PredictCore(const ScrollPlaceholderPredictParams& params, int64_t frameDeadlineNs) const;
    void CloseObservationFrame(int64_t vsyncTimestampNs);
    static size_t ComponentIndex(ScrollPlaceholderComponentType componentType);

    // Placeholder instance pool policy: submit as many background clone tasks as needed to
    // bring spare copies + in-flight clones back to SCROLL_PLACEHOLDER_INSTANCE_CACHE_CAPACITY - 1.
    // UI-thread only: re-seeds the resident copy source from the template builder when the
    // entry lost it (re-register or LRU demotion).
    void ScheduleBackgroundReplenish(const std::string& templateId);
    // Task body running off the UI thread; never invokes the builder. Every exit path
    // releases its pending slot.
    void RunBackgroundClone(const std::string& templateId, uint64_t generation);
    // Releases the slot booked for one clone of the given generation. Slots are keyed by
    // generation on purpose: after a re-register reset, a late release of the superseded
    // generation finds its own entry gone and becomes a no-op instead of consuming a slot
    // freshly booked for the current generation.
    void ReleasePendingCloneSlot(const std::string& templateId, uint64_t templateGeneration);
    // Re-register/unregister drops the slot bookings of every generation of the template;
    // clones still in flight of the old generation are stale by the generation guard and
    // their late releases become no-ops.
    void ResetPendingCloneSlots(const std::string& templateId);
    // Caller holds cloneMutex_.
    size_t SumPendingClonesLocked(const std::string& templateId) const;

    void RequestFrameIfAlive();
    void PurgeTemplateRealBuilds(const std::string& templateId, ScrollPlaceholderCancelReason reason);

    // Deferral table key: host id and item index packed into one 64-bit key. Host ids
    // and lazy-container indexes are generated positive int32s, so the packing is
    // collision-free for every real pair.
    static uint64_t DeferralKey(int32_t hostNodeId, int32_t index)
    {
        return (static_cast<uint64_t>(static_cast<uint32_t>(hostNodeId)) << 32) |
               static_cast<uint64_t>(static_cast<uint32_t>(index));
    }
    // Erases one table entry and keeps the live count in step; true when an entry
    // was actually removed.
    bool TakeDeferralEntry(uint64_t key);

    int32_t instanceId_ = -1;
    WeakPtr<PipelineContext> pipeline_;
    ScrollPlaceholderCostModel costModel_;
    ScrollPlaceholderTemplateRegistry registry_;
    std::deque<RealBuildTask> pendingRealBuilds_;
    uint64_t nextTaskSequence_ = 0;
    int64_t lastVsyncTimestampNs_ = 0;
    int64_t vsyncPeriodNs_ = 0;
    bool frameAnchored_ = false;
    std::atomic<bool> destroyed_ = false;
    std::array<FrameObservationStats, COMPONENT_TYPE_COUNT> frameStats_;
    std::array<FrameObservationStats, COMPONENT_TYPE_COUNT> lastFrameStats_;
    // Guards pendingCloneCount_, which is read and written from both the UI thread and the
    // background clone tasks.
    mutable std::mutex cloneMutex_;
    // templateId -> (generation -> booked clone slots). The generation key keeps the
    // in-flight count truthful across a re-register: bookings and releases always meet in
    // the same generation entry, so a reset cannot let stale releases eat the slots of the
    // freshly registered generation.
    std::unordered_map<std::string, std::unordered_map<uint64_t, uint32_t>> pendingCloneCount_;

    // (hostId, index) -> mounted shell of a deferred index. Weak so a leaked pattern
    // bookkeeping bug can never keep a retired shell alive through this table.
    std::unordered_map<uint64_t, WeakPtr<FrameNode>> deferrals_;
    // Process-wide live-deferral count behind HasAnyDeferrals(): relaxed atomic, the
    // FrameProxy fast gate reads it from the UI thread while record/clear run on the
    // UI thread as well (the atomic only keeps the read well-defined off-contract).
    inline static std::atomic<int32_t> liveDeferralCount_ = 0;

    mutable std::atomic<uint64_t> predictBuildRealNow_ = 0;
    mutable std::atomic<uint64_t> predictUsePlaceholder_ = 0;
    mutable std::atomic<uint64_t> realBuildExecuted_ = 0;
    mutable std::atomic<uint64_t> realBuildRequeued_ = 0;
    mutable std::atomic<uint64_t> realBuildDropped_ = 0;
    mutable std::atomic<uint64_t> registerRejected_ = 0;
    mutable std::atomic<uint64_t> flushRounds_ = 0;
    mutable std::atomic<uint64_t> requestFrameCount_ = 0;
    mutable std::atomic<uint64_t> observedRealBuilds_ = 0;
    mutable std::array<std::atomic<uint64_t>, CANCEL_REASON_COUNT> cancelledByReason_ = {};
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SCROLL_PLACEHOLDER_SCROLL_PLACEHOLDER_MANAGER_H
