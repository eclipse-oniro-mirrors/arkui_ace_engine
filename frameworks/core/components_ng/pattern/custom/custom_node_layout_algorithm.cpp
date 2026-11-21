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

#include "core/components_ng/pattern/custom/custom_node_layout_algorithm.h"

#include "core/components_ng/pattern/custom/custom_measure_layout_node.h"
#include "core/components_ng/pattern/custom/custom_node_base.h"
#include "core/components_ng/layout/layout_wrapper_node.h"

namespace OHOS::Ace::NG {

void CustomNodeLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto host = DynamicCast<CustomMeasureLayoutNode>(layoutWrapper->GetHostNode());
    CHECK_NULL_VOID(host);
    // Asynchronous loading for deferred custom components: the Measure entry is the
    // deferral hook for this node kind. The first-build gate is evaluated inside
    // TryStartAsyncLoad (single source of truth); while deferred neither the
    // content build block below nor the JS measure callback runs - the placeholder
    // subtree is measured by the default path.
    host->TryStartAsyncLoad();
    const bool asyncDeferred = host->IsAsyncLoadDeferred();
    if (asyncDeferred) {
        // The pattern keeps the single master copy of the render function; dropping
        // this wrapper's copy avoids a stale double build once the async path has
        // consumed the master.
        renderFunction_ = nullptr;
    }
    if (!asyncDeferred && renderFunction_ && host) {
        {
            ACE_SCOPED_TRACE("CustomNode:OnAppear");
            host->FireOnAppear();
            host->CustomNodeBase::FireTriggerLifecycleFunc(CustomNodeBase::LifeCycleEvent::ON_APPEAR);
        }
        {
            ACE_SCOPED_TRACE("CustomNode:BuildItem");
            RefPtr<UINode> child;
            RefPtr<FrameNode> frameNode;
            {
                // first create child node and wrapper.
                ScopedViewStackProcessor scopedViewStackProcessor;
                auto* stack = ViewStackProcessor::GetInstance();
                auto nodeId = stack->ClaimNodeId();
                ACE_UINODE_TRACE(nodeId);
                frameNode = FrameNode::GetOrCreateFrameNode(
                    "dummyNode", nodeId, []() { return AceType::MakeRefPtr<Pattern>(); });
                stack->Push(frameNode);
                bool isTimeout = false;
                child = renderFunction_(0, isTimeout);
                renderFunction_ = nullptr;
                stack->Pop();
            }
            
            auto layoutWrapperNode = DynamicCast<LayoutWrapperNode>(Claim(layoutWrapper));
            for (const auto& child : frameNode->GetChildren()) {
                if (!child) {
                    continue;
                }
                if (buildItem_ == nullptr) {
                    buildItem_ = child;
                }
                child->MountToParent(host);
                if (layoutWrapperNode) {
                    child->AdjustLayoutWrapperTree(layoutWrapperNode, true, true);
                }
            }
        }
        // The build finished here (children mounted, or legitimately none):
        // an empty component must not look "unbuilt" to the deferral gate on a
        // later mount.
        host->MarkContentBuilt();
        host->CustomNodeBase::FireTriggerLifecycleFunc(CustomNodeBase::LifeCycleEvent::ON_BUILD);
    }
    {
        auto customNode = DynamicCast<CustomNodeBase>(layoutWrapper->GetHostNode());
        if (customNode) {
            customNode->FireRecycleRenderFunc();
        }
    }

    // call js measure
    // While the asynchronous build is deferred the custom measure
    // callback is suppressed (first dispatch happens on the load-complete frame).
    if (asyncDeferred || !host->FireOnMeasure(layoutWrapper)) {
        // use normal measure step.
        auto layoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
        const auto& children = layoutWrapper->GetAllChildrenWithBuild();
        int32_t index = 0;
        host->FireOnUpdateParam(layoutWrapper);
        for (auto&& child : children) {
            child->Measure(layoutConstraint);
            auto size = child->GetGeometryNode()->GetFrameSize();
            host->UpdateSize(index, size);
            index++;
        }
        BoxLayoutAlgorithm::PerformMeasureSelf(layoutWrapper);
    }
}

void CustomNodeLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    auto host = DynamicCast<CustomMeasureLayoutNode>(layoutWrapper->GetHostNode());
    CHECK_NULL_VOID(host);
    // The custom layout callback is suppressed while the asynchronous
    // build is deferred; the default path lays out the placeholder subtree.
    if (host->IsAsyncLoadDeferred() || !host->FireOnLayout(layoutWrapper)) {
        PerformLayout(layoutWrapper);
        for (auto&& child : layoutWrapper->GetAllChildrenWithBuild()) {
            child->Layout();
        }
    }
}

} // namespace OHOS::Ace::NG
