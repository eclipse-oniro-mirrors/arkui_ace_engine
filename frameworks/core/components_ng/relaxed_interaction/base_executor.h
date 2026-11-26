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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_BASE_EXECUTOR_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_BASE_EXECUTOR_H

#include <string>
#include <utility>

#include "base/memory/referenced.h"
#include "core/components_ng/relaxed_interaction/frame_node_finder.h"
#include "core/components_ng/relaxed_interaction/relaxed_interaction_types.h"

namespace OHOS::Ace::NG {

class PipelineContext;
class FrameNode;

class BaseExecutor {
public:
    explicit BaseExecutor(WeakPtr<PipelineContext> context) : context_(context) {}
    virtual ~BaseExecutor() = default;

    virtual ExecutorResult ExecuteStep() = 0;

    virtual void Cancel() {}

    virtual std::string GetType() const = 0;

    virtual bool IsSingleStep() const = 0;

    // Longest in-executor hold/wait (ms) that legitimately extends the workflow timeout window.
    virtual int32_t GetHoldMs() const
    {
        return 0;
    }

    // Input already delivered by this executor (partial multi-click): used to stop AUTO
    // fallback from replaying the full count after a partial success.
    virtual bool HasProducedInput() const
    {
        return false;
    }

    // P2 security seam: stamp deny callback and identity before execution.
    virtual void SetDenyCallback(RelaxedDenyCallback callback, const RelaxedCommandIdentity& identity)
    {
        denyCallback_ = std::move(callback);
        identity_ = identity;
    }

    bool IsDenied(const PointF& coordinates) const
    {
        return denyCallback_ && denyCallback_(coordinates, identity_);
    }

    virtual std::string GetDescription() const
    {
        return "{executorType='" + GetType() + "', isSingleStep=" + (IsSingleStep() ? "true" : "false") + "}";
    }

    ExecutionState GetFailureState() const
    {
        return failureState_;
    }

protected:
    ExecutorResult SetFailureState(ExecutionState state = ExecutionState::FAILED)
    {
        // A security refusal is final for this executor; later "not found" style
        // failures must not mask the security denied classification.
        if (failureState_ != ExecutionState::SECURITY_DENIED) {
            failureState_ = state;
        }
        return ExecutorResult::FAILED;
    }

    FrameNodeMatch FindFrameNode(double x, double y, GestureRecognizerPred pred);
    bool IsTargetDeniedAt(double x, double y);

    // Handles a hit on a special-component placeholder node (UIExtension/DynamicComponent):
    // the security gate precedes any hit information (DENY nodes are audited and reported as
    // SECURITY_DENIED without a payload); otherwise stashes nodeId + component-local offset
    // (converted with the same NGGestureRecognizer::Transform used by real-input forwarding),
    // records TARGET_NOT_FOUND failure state and returns true.
    bool HandleSpecialComponentHit(const RefPtr<FrameNode>& node, double x, double y);

    WeakPtr<PipelineContext> context_;
    ExecutionState failureState_ = ExecutionState::FAILED;
    RelaxedDenyCallback denyCallback_;
    RelaxedCommandIdentity identity_;

public:
    // Detects a hit on a special-component placeholder node (UIExtension/DynamicComponent)
    // at the injection point; returns true when the caller should fail with the
    // SEC_UEC_FAILED contract.
    bool DetectSpecialComponentHit(double x, double y);

    // Special-component hit payload consumed by the result serialization (SEC_UEC_FAILED).
    static void ConsumeSpecialComponentHit(SpecialComponentHitInfo& info);
    static void ClearSpecialComponentHit();

private:
    static thread_local SpecialComponentHitInfo specialComponentHit_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RELAXED_INTERACTION_BASE_EXECUTOR_H
