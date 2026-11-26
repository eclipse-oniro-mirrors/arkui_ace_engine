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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_CUSTOM_MEASURE_LAYOUT_NODE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_CUSTOM_MEASURE_LAYOUT_NODE_H

#include <functional>
#include <string>

#include "base/utils/macros.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/custom/custom_measure_layout_param.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/pattern/custom/custom_node_base.h"
#include "core/components_ng/pattern/custom/custom_node_pattern.h"

namespace OHOS::Ace::NG {

// CustomMeasureLayoutNode is the frame node of @Component struct.
class ACE_EXPORT CustomMeasureLayoutNode : public FrameNode, public CustomNodeBase {
    DECLARE_ACE_TYPE(CustomMeasureLayoutNode, FrameNode, CustomNodeBase);

public:
    static RefPtr<CustomMeasureLayoutNode> CreateCustomMeasureLayoutNode(int32_t nodeId, const std::string& viewKey);

    CustomMeasureLayoutNode(int32_t nodeId, const std::string& viewKey);
    ~CustomMeasureLayoutNode() override = default;

    void SetRenderFunction(const RenderFunction& renderFunction) override
    {
        auto pattern = DynamicCast<CustomNodePattern>(GetPattern());
        if (pattern) {
            pattern->SetRenderFunction(renderFunction);
        }
    }

    bool FireOnMeasure(LayoutWrapper* layoutWrapper);

    bool FireOnLayout(LayoutWrapper* layoutWrapper);

    void SetLayoutFunction(std::function<void(LayoutWrapper* layoutWrapper)>&& layoutFunc)
    {
        layoutFunc_ = std::move(layoutFunc);
    }

    void SetMeasureFunction(std::function<void(LayoutWrapper* layoutWrapper)>&& measureFunc)
    {
        measureFunc_ = std::move(measureFunc);
    }
    void SetCompleteReloadFunc(RenderFunction&& func) override {}

    RefPtr<MeasureLayoutParam> GetMeasureLayoutParam() const
    {
        return measureLayoutParam_;
    }
    void SetMeasureLayoutParam(RefPtr<MeasureLayoutParam> param)
    {
        measureLayoutParam_ = param;
    }
    void SetUpdateParamFunc(std::function<void(LayoutWrapper* layoutWrapper)>&& updateParamFunc)
    {
        updateParamFunc_ = updateParamFunc;
    }
    bool FireOnUpdateParam(NG::LayoutWrapper* layoutWrapper);
    bool UpdateSize(int32_t index, SizeF size)
    {
        CHECK_NULL_RETURN(measureLayoutParam_, false);
        CHECK_NULL_RETURN(measureLayoutParam_->ChildIndexInRange(index), false);
        auto childParam = measureLayoutParam_->Get(index);
        childParam.UpdateSize(size);
        return true;
    }

    bool Render(int64_t deadline = 0);
    bool RenderCustomChild(int64_t deadline) override;

    // --- Asynchronous loading (enableAsyncLoad); host for nodes WITH measure
    // callbacks; the deferral hook sits at the layout-algorithm Measure entry
    // While deferred neither content nor JS callbacks are dispatched.
    // Convenience query: the full gate (including this host's unbuilt marker
    // below) lives in the shared base, evaluated inside TryStartAsyncLoad.
    bool ShouldStartAsyncLoad() const;
    RefPtr<UINode> GetAsyncLoadHost() override;
    // "Not built yet" signal. Content children presence is the usual marker,
    // but a legitimately empty component stays childless after building, so a
    // completed build also marks the node explicitly — "built" is either
    // marker (children present, or a finished build). This keeps pre-built
    // nodes (lazy caches, preRender — including empty ones) mounting with
    // their content shown directly instead of a deferral that would re-run
    // the build. The placeholder never pollutes this marker: it exists only
    // in non-NONE states (mounted after the gate engages, detached at DONE),
    // and the gate is evaluated solely from NONE.
    bool HasUnbuiltContent() const override
    {
        return GetChildren().empty() && !contentBuilt_;
    }
    // Marks a completed content build; the synchronous Measure path and the
    // asynchronous slice driver both call it at build completion.
    void MarkContentBuilt()
    {
        contentBuilt_ = true;
    }
    void FlushAsyncLoadCompletion(bool wasForced) override;

    bool GetJsActive() const
    {
        return jsActive_;
    }
    // Transparent to the native active walk (tracks jsActive_ for the
    // async-load scheduler, then descends to children like the base
    // implementation; does not fire this host's own JS callback).
    void SetJSViewActive(bool active, bool isLazyForEachNode = false,
        bool isReuse = false, bool suppressActiveLifecycle = false) override;

    void OnAttachToMainTree(bool recursive) override;
    void OnDetachFromMainTree(bool recursive, PipelineContext* context) override;

private:
    UINode* GetCustomNodeOwner() const override
    {
        return const_cast<CustomMeasureLayoutNode*>(this);
    }

    // Slice-retained asynchronous build state (the prebuild-slicing retention
    // protocol): a timed-out slice parks its partially built subtree — the
    // collector below plus anything pushed but not yet popped — on the retained
    // processor, so the resumed slice continues in place instead of discarding
    // children built before the interrupt. Released at the terminal settle.
    std::unique_ptr<ViewStackProcessor> asyncBuildProcessor_;
    RefPtr<FrameNode> asyncBuildCollector_;

    // True once any build path has completed this node's content (see
    // HasUnbuiltContent): a legitimately empty component has no children to
    // show for it. Monotonic — the load state machine never returns to NONE,
    // so it never needs to clear.
    bool contentBuilt_ = false;

    // Terminal cleanup: drop the parked build state (no-op after a natural
    // completion, which clears it via the scoped processor's choreography).
    void ReleaseAsyncBuildState() override
    {
        asyncBuildProcessor_ = nullptr;
        asyncBuildCollector_ = nullptr;
    }

    // Drives the render function outside the layout pass; false = budget ran out.
    bool ExecuteAsyncBuild(int64_t deadline) override;
    // DONE settle: placeholder swap, dirty + frame request.

    void BuildChildren(const RefPtr<FrameNode>& child);
    std::function<void(LayoutWrapper* layoutWrapper)> layoutFunc_;
    std::function<void(LayoutWrapper* layoutWrapper)> measureFunc_;
    std::function<void(LayoutWrapper* layoutWrapper)> updateParamFunc_;
    std::string viewKey_;
    RefPtr<MeasureLayoutParam> measureLayoutParam_;
    bool jsActive_ = true;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_CUSTOM_NODE_H
