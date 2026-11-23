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

#include "core/pipeline_ng/async_load_scheduler.h"

#include <cinttypes>

#include "base/log/log.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/custom/custom_measure_layout_node.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/pattern/custom/custom_node_base.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int64_t ONE_MS_IN_NS = 1000000;

// Asynchronous loading has two host node kinds: CustomNode (components without
// measure callbacks) and CustomMeasureLayoutNode (with onMeasureSize/
// onPlaceChildren). The scheduling below talks to them through these dispatch
// helpers (the state machine lives on CustomNodeBase).
bool AsyncLoadNodePending(const RefPtr<UINode>& node)
{
    auto customNode = AceType::DynamicCast<CustomNode>(node);
    if (customNode) {
        return customNode->IsAsyncLoadPending();
    }
    auto measureNode = AceType::DynamicCast<CustomMeasureLayoutNode>(node);
    return measureNode && measureNode->IsAsyncLoadPending();
}

bool AsyncLoadNodeActive(const RefPtr<UINode>& node)
{
    auto customNode = AceType::DynamicCast<CustomNode>(node);
    if (customNode) {
        return customNode->GetJsActive();
    }
    auto measureNode = AceType::DynamicCast<CustomMeasureLayoutNode>(node);
    return !measureNode || measureNode->GetJsActive();
}

void AsyncLoadNodeRunOnIdle(const RefPtr<UINode>& node, int64_t deadline)
{
    auto customNode = AceType::DynamicCast<CustomNode>(node);
    if (customNode) {
        customNode->RunAsyncLoadOnIdle(deadline);
        return;
    }
    auto measureNode = AceType::DynamicCast<CustomMeasureLayoutNode>(node);
    if (measureNode) {
        measureNode->RunAsyncLoadOnIdle(deadline);
    }
}

void AsyncLoadNodeForce(const RefPtr<UINode>& node)
{
    auto customNode = AceType::DynamicCast<CustomNode>(node);
    if (customNode) {
        customNode->ForceLoad();
        return;
    }
    auto measureNode = AceType::DynamicCast<CustomMeasureLayoutNode>(node);
    if (measureNode) {
        measureNode->ForceLoad();
    }
}
} // namespace

thread_local int32_t AsyncLoadScheduler::buildScopeDepth_ = 0;

void AsyncLoadScheduler::EnterBuildScope()
{
    ++buildScopeDepth_;
}

void AsyncLoadScheduler::ExitBuildScope()
{
    --buildScopeDepth_;
}

bool AsyncLoadScheduler::InBuildScope()
{
    return buildScopeDepth_ > 0;
}

AsyncLoadScheduler::AsyncLoadScheduler(PipelineContext& owner) : owner_(owner) {}

void AsyncLoadScheduler::RegisterIdleEntry(const WeakPtr<UINode>& node)
{
    auto uiNode = node.Upgrade();
    CHECK_NULL_VOID(uiNode);
    auto base = AceType::DynamicCast<CustomNodeBase>(uiNode);
    CHECK_NULL_VOID(base);
    // One live entry per node: a remount replaces the previous entry instead of
    // adding a second one, and the countdown starts at (re)registration — the
    // stamped entry timestamp is the single countdown authority (remount resume:
    // fresh timer; freeze resume refreshes it on every skipped frame).
    RemoveIdleEntry(node);
    IdleEntry entry;
    entry.node = node;
    entry.enableTimestamp = GetSysTimestamp();
    entry.resolvedTimeoutMs = base->GetAsyncLoadResolvedTimeoutMs();
    idleEntries_.emplace_back(std::move(entry));
}

void AsyncLoadScheduler::RemoveIdleEntry(const WeakPtr<UINode>& node)
{
    idleEntries_.remove_if([&node](const IdleEntry& entry) { return entry.node == node; });
}

std::function<void()> AsyncLoadScheduler::MakeForceTask(const WeakPtr<UINode>& node)
{
    return [weakNode = node, self = this]() {
        auto customNode = weakNode.Upgrade();
        CHECK_NULL_VOID(customNode);
        if (!AsyncLoadNodePending(customNode)) {
            // Finished or destroyed between transfer and execution: weak-reference
            // skip, not a lost load.
            return;
        }
        if (!customNode->IsOnMainTree()) {
            // Detached (instance switch): forced work must not run for a pipeline
            // the node left; the remount path re-registers it into the idle set.
            return;
        }
        if (!AsyncLoadNodeActive(customNode)) {
            // Frozen components take no forced builds. Return the entry to the
            // idle set instead of requeueing here: the transfer checkpoint
            // refreshes its timestamp on every skipped frame while frozen
            // (countdown suspended) and re-expires it after activation, so the
            // timeout backstop restarts from the activation point. A plain requeue
            // would self-sustain a frame request per vsync for invisible content.
            self->RegisterIdleEntry(weakNode);
            return;
        }
        if (self->forceCount_ >= FORCE_LIMIT_PER_FRAME) {
            // At most FORCE_LIMIT_PER_FRAME forced builds per frame. Requeue into
            // the (fresh) queue; AddAsyncLoadTask requests the next frame, the
            // ENDORSE_LAYOUT_COUNT count-and-carry-on pattern.
            TAG_LOGI(AceLogTag::ACE_STATE_MGMT,
                "AsyncLoad force limit reached (%{public}d/frame), requeue node %{public}d",
                FORCE_LIMIT_PER_FRAME, customNode->GetId());
            self->owner_.AddAsyncLoadTask(self->MakeForceTask(weakNode));
            return;
        }
        {
            // Descendant custom components created during this build keep
            // synchronous semantics (their enableAsyncLoad requests are suppressed).
            BuildScopeGuard scope;
            self->forceCount_++;
            AsyncLoadNodeForce(customNode);
        }
    };
}

void AsyncLoadScheduler::OnVsyncFlush(int64_t now)
{
    forceCount_ = 0;
    if (idleEntries_.empty()) {
        return;
    }
    for (auto it = idleEntries_.begin(); it != idleEntries_.end();) {
        auto customNode = it->node.Upgrade();
        if (!customNode || !AsyncLoadNodePending(customNode)) {
            it = idleEntries_.erase(it); // weak-reference cleanup
            continue;
        }
        if (!AsyncLoadNodeActive(customNode)) {
            // While frozen the timeout does not advance. Refreshing the start
            // timestamp on every skipped frame suspends the countdown, so the full
            // timeout re-runs from activation ("resume resets the timer").
            it->enableTimestamp = now;
            ++it;
            continue;
        }
        bool expired = false;
        if (it->resolvedTimeoutMs == 0) {
            // Zero is pre-expired; forced on the next scheduling frame
            // after registration.
            expired = true;
        } else if (it->resolvedTimeoutMs > 0) {
            expired = now - it->enableTimestamp >= it->resolvedTimeoutMs * ONE_MS_IN_NS;
        }
        // Negative: never force — idle-only completion.
        if (expired) {
            TAG_LOGI(AceLogTag::ACE_STATE_MGMT,
                "AsyncLoad timeout transfer: node %{public}d, resolvedTimeoutMs %{public}" PRId64,
                customNode->GetId(), it->resolvedTimeoutMs);
            auto task = MakeForceTask(it->node);
            it = idleEntries_.erase(it);
            owner_.AddAsyncLoadTask(std::move(task)); // queue already requests a frame
        } else {
            ++it;
        }
    }
}

void AsyncLoadScheduler::FlushIdleBuild(int64_t deadline)
{
    if (idleEntries_.empty()) {
        return;
    }
    // Unfinished slices must keep frames flowing: OnIdle and the vsync-flush
    // timeout transfer only run while vsync is scheduled, so a quiescent page
    // with pending work would stall until the next user input. Frozen-only sets
    // stay quiet (no frames for invisible content; unfreeze re-arms the pipeline).
    auto hasRunnableWork = [this]() {
        for (const auto& entry : idleEntries_) {
            auto node = entry.node.Upgrade();
            if (node && AsyncLoadNodePending(node) && AsyncLoadNodeActive(node)) {
                return true;
            }
        }
        return false;
    };
    if (GetSysTimestamp() >= deadline) {
        if (hasRunnableWork()) {
            owner_.RequestFrame(); // no budget this frame; retry on the next idle
        }
        return; // no idle budget left this frame
    }
    auto it = idleEntries_.begin();
    while (it != idleEntries_.end()) {
        auto customNode = it->node.Upgrade();
        if (!customNode || !AsyncLoadNodePending(customNode)) {
            // Destroyed or already finished: weak-reference cleanup.
            it = idleEntries_.erase(it);
            continue;
        }
        if (!AsyncLoadNodeActive(customNode)) {
            // Frozen components consume no idle budget.
            ++it;
            continue;
        }
        {
            // Descendant custom components created during this build keep
            // synchronous semantics (their enableAsyncLoad requests are suppressed).
            BuildScopeGuard scope;
            AsyncLoadNodeRunOnIdle(customNode, deadline);
        }
        if (!AsyncLoadNodePending(customNode)) {
            it = idleEntries_.erase(it);
        } else {
            ++it;
        }
        if (GetSysTimestamp() >= deadline) {
            // Budget exhausted: unfinished builds resume on the next idle (idle
            // slicing); the timeout backstop still guarantees progress.
            TAG_LOGD(AceLogTag::ACE_STATE_MGMT, "AsyncLoad idle budget exhausted, remaining %{public}zu entries",
                idleEntries_.size());
            break;
        }
    }
    if (hasRunnableWork()) {
        owner_.RequestFrame(); // keep the slicing loop alive until the set drains
                               // or the timeout backstop transfers the stragglers
    }
}
} // namespace OHOS::Ace::NG
