/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/custom/custom_measure_layout_node.h"

#include "base/utils/time_util.h"
#include "core/components_ng/base/async_load_config.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/pipeline_ng/pipeline_context.h"


namespace OHOS::Ace::NG {

RefPtr<CustomMeasureLayoutNode> CustomMeasureLayoutNode::CreateCustomMeasureLayoutNode(
    int32_t nodeId, const std::string& viewKey)
{
    ACE_UINODE_TRACE(nodeId);
    auto node = MakeRefPtr<CustomMeasureLayoutNode>(nodeId, viewKey);
    node->InitializePatternAndContext();
    ElementRegister::GetInstance()->AddUINode(node);
    return node;
}

CustomMeasureLayoutNode::CustomMeasureLayoutNode(int32_t nodeId, const std::string& viewKey)
    : FrameNode(V2::JS_VIEW_ETS_TAG, nodeId, MakeRefPtr<CustomNodePattern>()), viewKey_(viewKey)
{}

bool CustomMeasureLayoutNode::FireOnMeasure(NG::LayoutWrapper* layoutWrapper)
{
    if (measureFunc_) {
        measureFunc_(layoutWrapper);
        return true;
    }
    return false;
}

bool CustomMeasureLayoutNode::FireOnLayout(NG::LayoutWrapper* layoutWrapper)
{
    if (layoutFunc_) {
        layoutFunc_(layoutWrapper);
        return true;
    }
    return false;
}

bool CustomMeasureLayoutNode::FireOnUpdateParam(NG::LayoutWrapper* layoutWrapper)
{
    if (updateParamFunc_) {
        updateParamFunc_(layoutWrapper);
        return true;
    }
    return false;
}

bool CustomMeasureLayoutNode::Render(int64_t deadline)
{
    // External synchronous callers (e.g. FrameNode::FrameProxy flat wrapper building)
    // bypass the layout algorithm, so the first-build gate is engaged here as well —
    // its conditions are evaluated inside TryStartAsyncLoad, the single source of
    // truth, and it returns true whether the deferral engaged just now or on an
    // earlier call (the idle path drives ExecuteAsyncBuild directly, unaffected).
    if (TryStartAsyncLoad()) {
        return false;
    }
    if (deadline > 0 && GetSysTimestamp() > deadline) {
        return false;
    }
    {
        FireRecycleRenderFunc();
    }
    return true;
}

bool CustomMeasureLayoutNode::RenderCustomChild(int64_t deadline)
{
    if (GetSysTimestamp() > deadline) {
        return false;
    }
    if (!Render(deadline)) {
        return false;
    }
    return FrameNode::RenderCustomChild(deadline);
}

void CustomMeasureLayoutNode::SetJSViewActive(bool active, bool isLazyForEachNode,
    bool isReuse, bool suppressActiveLifecycle)
{
    // Track for the async-load scheduler's freeze gate (AsyncLoadNodeActive ->
    // GetJsActive), but stay transparent to the native active walk like the base
    // FrameNode was: unlike CustomNode, this host must NOT become a JS-view
    // boundary. Firing its own JS callback here would double-notify JS children
    // (the callback triggers the JS view tree propagation, and the walk below
    // would fire CustomNode children again, corrupting their active count), so
    // this host's own JS view keeps being notified through the JS view tree by
    // its JS parent — exactly as before this override existed — while the
    // descent keeps native-managed structures below (LazyForEach/Repeat caches,
    // nested walk-reachable subtrees) receiving active propagation.
    jsActive_ = active;
    UINode::SetJSViewActive(active, isLazyForEachNode, isReuse, suppressActiveLifecycle);
}

void CustomMeasureLayoutNode::OnAttachToMainTree(bool recursive)
{
    FrameNode::OnAttachToMainTree(recursive);
    if (IsAsyncLoadPending()) {
        // Remount after an instance switch or re-attach: resume through the current
        // instance's idle set with a fresh timer (remount: fresh timer). In-flight forced
        // tasks drop themselves for detached nodes, so this is the only resume path.
        auto context = GetContext();
        if (context) {
            context->RegisterAsyncLoadIdleEntry(WeakClaim(this));
            TAG_LOGI(AceLogTag::ACE_STATE_MGMT,
                "AsyncLoad resume on remount (measure host): node %{public}d, timer reset", GetId());
        }
    }
}

void CustomMeasureLayoutNode::OnDetachFromMainTree(bool recursive, PipelineContext* context)
{
    FrameNode::OnDetachFromMainTree(recursive, context);
    // Detaching only invalidates the execution vehicle - drop this node's entry
    // from the detaching instance's idle set; subtree and state are kept for resume.
    if (IsAsyncLoadPending() && context) {
        context->RemoveAsyncLoadIdleEntry(WeakClaim(this));
        TAG_LOGI(AceLogTag::ACE_STATE_MGMT,
            "AsyncLoad suspend on detach (measure host): node %{public}d, state kept", GetId());
    }
}

bool CustomMeasureLayoutNode::ShouldStartAsyncLoad() const
{
    // Convenience query forwarding to the shared full gate (state, suppression,
    // config, on-tree and this host's no-children-yet marker included).
    return CustomNodeBase::ShouldStartAsyncLoad(IsOnMainTree());
}

RefPtr<UINode> CustomMeasureLayoutNode::GetAsyncLoadHost()
{
    return Claim(this);
}

void CustomMeasureLayoutNode::FlushAsyncLoadCompletion(bool wasForced)
{
    // The host itself is a FrameNode: a self measure-dirty request is a valid flush,
    // and the render-tree sync for the mounted content propagates naturally (this
    // host does not gate MarkNeedSyncRenderTree).
    MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    auto context = GetContext();
    CHECK_NULL_VOID(context);
    context->RequestFrame();
}

bool CustomMeasureLayoutNode::ExecuteAsyncBuild(int64_t deadline)
{
    if (!CheckFireOnAppear()) {
        ACE_SCOPED_TRACE("CustomMeasureLayoutNode:OnAppear");
        FireOnAppear();
        FireTriggerLifecycleFunc(CustomNodeBase::LifeCycleEvent::ON_APPEAR);
        if (deadline > 0 && GetSysTimestamp() > deadline) {
            return false; // budget exhausted before the build started; resume next slice
        }
    }
    auto pattern = DynamicCast<CustomNodePattern>(GetPattern());
    CHECK_NULL_RETURN(pattern, false);
    {
        ACE_SCOPED_TRACE("CustomMeasureLayoutNode:BuildItem");
        // The scoped processor installs the retained build state when one exists,
        // so the render function resumes on the parked stack (collector and
        // half-built subtree included) instead of a fresh one.
        ScopedViewStackProcessor scopedViewStackProcessor(asyncBuildProcessor_);
        auto* stack = ViewStackProcessor::GetInstance();
        if (!asyncBuildCollector_) {
            auto nodeId = stack->ClaimNodeId();
            ACE_UINODE_TRACE(nodeId);
            asyncBuildCollector_ = FrameNode::GetOrCreateFrameNode(
                "dummyNode", nodeId, []() { return AceType::MakeRefPtr<Pattern>(); });
            stack->Push(asyncBuildCollector_);
        }
        bool isTimeout = false;
        pattern->InvokeRenderFunction(deadline, isTimeout);
        stack->Pop();
        if (isTimeout) {
            // Park the partial build (collector plus everything built so far) on
            // the retained processor; the render function itself is kept by the
            // pattern, so the resumed slice continues right where this one broke.
            scopedViewStackProcessor.SwapViewStackProcessor(asyncBuildProcessor_);
            return false;
        }
        for (const auto& child : asyncBuildCollector_->GetChildren()) {
            if (!child) {
                continue;
            }
            child->MountToParent(Claim(this));
        }
        // The build finished (children mounted, or legitimately none): mark it
        // so a later gate check cannot mistake an empty component for unbuilt.
        contentBuilt_ = true;
    }
    FireTriggerLifecycleFunc(CustomNodeBase::LifeCycleEvent::ON_BUILD);
    FireDidBuild();
    return true;
}


} // namespace OHOS::Ace::NG
