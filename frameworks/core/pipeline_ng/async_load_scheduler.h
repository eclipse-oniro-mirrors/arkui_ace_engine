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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_PIPELINE_NG_ASYNC_LOAD_SCHEDULER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_PIPELINE_NG_ASYNC_LOAD_SCHEDULER_H

#include <functional>
#include <list>

#include "base/memory/ace_type.h"
#include "base/utils/macros.h"

namespace OHOS::Ace::NG {

class PipelineContext;
class UINode;

// Passive home of the custom-component asynchronous-loading scheduling state:
// the idle set consumed by PipelineContext::OnIdle (normal path), the per-frame
// force counter and the expired-entry transfer into the existing asyncLoadTasks_
// queue (timeout path), and the "an asynchronous build is executing" nesting
// scope. It owns no tasks, no timers and no queue of its own — PipelineContext
// drives every entry point from its OnIdle / vsync-flush hooks, so the approved
// OnIdle-carrier + reused-force-queue architecture is unchanged; this is only a
// state-and-algorithm module, not the self-polling manager rejected in review.
class AsyncLoadScheduler {
public:
    static constexpr int32_t FORCE_LIMIT_PER_FRAME = 8;

    struct IdleEntry {
        WeakPtr<UINode> node;
        int64_t enableTimestamp = 0;   // GetSysTimestamp() ticks at (re)registration
        int64_t resolvedTimeoutMs = 0; // resolved: >0 expire, =0 next frame, <0 never
    };

    explicit AsyncLoadScheduler(PipelineContext& owner);

    // One live entry per node; a remount replaces the previous entry and restarts
    // the timeout countdown (remount resume: fresh timer).
    void RegisterIdleEntry(const WeakPtr<UINode>& node);
    void RemoveIdleEntry(const WeakPtr<UINode>& node);

    // Per-frame vsync flush checkpoint: reset the force budget, then transfer
    // expired idle entries into the pipeline's asyncLoadTasks_ force queue.
    void OnVsyncFlush(int64_t now);

    // OnIdle consumer: build deferred components with the remaining frame budget.
    void FlushIdleBuild(int64_t deadline);

    std::function<void()> MakeForceTask(const WeakPtr<UINode>& node);

    // Nesting scope for "an ancestor's asynchronous build is executing on THIS
    // thread": descendants created inside it keep synchronous semantics (creation
    // happens synchronously within the build call, always on the building thread).
    // thread_local on purpose: isolated-thread instances may run UI off the main
    // thread, and a plain static would let one instance's in-flight build suppress
    // another instance's configuration delivery; module-level (not per scheduler)
    // because the scope must be observable while a created node is still
    // unattached (no pipeline context reachable yet).
    // Out-of-line in async_load_scheduler.cpp and exported via libace.map:
    // inline bodies would duplicate the inline-variable storage per .so under
    // -fvisibility=hidden, splitting the scope state across libraries.
    // Unit-test links get functional definitions from the mock instead.
    static void EnterBuildScope();
    static void ExitBuildScope();
    static bool InBuildScope();
    class BuildScopeGuard {
    public:
        BuildScopeGuard()
        {
            EnterBuildScope();
        }
        ~BuildScopeGuard()
        {
            ExitBuildScope();
        }
    };

private:
    PipelineContext& owner_;
    std::list<IdleEntry> idleEntries_;
    int32_t forceCount_ = 0; // forced builds issued in the current frame

    static thread_local int32_t buildScopeDepth_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_PIPELINE_NG_ASYNC_LOAD_SCHEDULER_H
