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

#include "core/components_ng/base/frame_node.h"

#include "base/geometry/ng/point_t.h"
#include "base/log/ace_trace.h"
#include "base/log/dump_log.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/thread/cancelable_callback.h"
#include "base/thread/task_executor.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/common/ace_application_info.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/paint_wrapper.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {
FrameNode::FrameNode(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern, bool isRoot)
    : UINode(tag, nodeId, isRoot), pattern_(pattern)
{
    renderContext_->InitContext(IsRootNode(), pattern_->GetSurfaceNodeName(), pattern_->UseExternalRSNode());
    paintProperty_ = pattern->CreatePaintProperty();
    layoutProperty_ = pattern->CreateLayoutProperty();
    eventHub_ = pattern->CreateEventHub();
    accessibilityProperty_ = pattern->CreateAccessibilityProperty();
    // first create make layout property dirty.
    layoutProperty_->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
    layoutProperty_->SetHost(WeakClaim(this));
}

FrameNode::~FrameNode()
{
    pattern_->DetachFromFrameNode(this);
    if (IsOnMainTree()) {
        OnDetachFromMainTree();
    }
    auto focusHub = GetFocusHub();
    if (focusHub) {
        focusHub->RemoveSelf();
    }
}

RefPtr<FrameNode> FrameNode::CreateFrameNodeWithTree(
    const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern)
{
    auto newChild = CreateFrameNode(tag, nodeId, pattern, true);
    newChild->SetDepth(1);
    return newChild;
}

RefPtr<FrameNode> FrameNode::GetOrCreateFrameNode(
    const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator)
{
    auto frameNode = GetFrameNode(tag, nodeId);
    if (frameNode) {
        return frameNode;
    }
    auto pattern = patternCreator ? patternCreator() : MakeRefPtr<Pattern>();
    return CreateFrameNode(tag, nodeId, pattern);
}

RefPtr<FrameNode> FrameNode::GetFrameNode(const std::string& tag, int32_t nodeId)
{
    auto frameNode = ElementRegister::GetInstance()->GetSpecificItemById<FrameNode>(nodeId);
    if (!frameNode) {
        return nullptr;
    }
    if (frameNode->GetTag() != tag) {
        LOGE("the tag is changed");
        ElementRegister::GetInstance()->RemoveItemSilently(nodeId);
        auto parent = frameNode->GetParent();
        if (parent) {
            parent->RemoveChild(frameNode);
        }
        return nullptr;
    }
    return frameNode;
}

RefPtr<FrameNode> FrameNode::CreateFrameNode(
    const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern, bool isRoot)
{
    auto frameNode = MakeRefPtr<FrameNode>(tag, nodeId, pattern, isRoot);
    frameNode->InitializePatternAndContext();
    ElementRegister::GetInstance()->AddUINode(frameNode);
    return frameNode;
}

void FrameNode::InitializePatternAndContext()
{
    eventHub_->AttachHost(WeakClaim(this));
    pattern_->AttachToFrameNode(WeakClaim(this));
    renderContext_->SetRequestFrame([weak = WeakClaim(this)] {
        auto frameNode = weak.Upgrade();
        CHECK_NULL_VOID(frameNode);
        if (frameNode->IsOnMainTree()) {
            auto context = frameNode->GetContext();
            CHECK_NULL_VOID(context);
            context->RequestFrame();
            return;
        }
        frameNode->hasPendingRequest_ = true;
    });
    renderContext_->SetHostNode(WeakClaim(this));
    // Initialize FocusHub
    if (pattern_->GetFocusPattern().focusType != FocusType::DISABLE) {
        auto focusHub = GetOrCreateFocusHub();
        CHECK_NULL_VOID(focusHub);
        focusHub->SetScopeFocusAlgorithm(pattern_->GetScopeFocusAlgorithm());
    }
}

void FrameNode::DumpInfo()
{
    DumpLog::GetInstance().AddDesc(std::string("FrameRect: ").append(geometryNode_->GetFrameRect().ToString()));
    DumpLog::GetInstance().AddDesc(
        std::string("BackgroundColor: ").append(renderContext_->GetBackgroundColor()->ColorToString()));
    DumpLog::GetInstance().AddDesc(std::string("ParentLayoutConstraint: ")
                                       .append(geometryNode_->GetParentLayoutConstraint().has_value()
                                                   ? geometryNode_->GetParentLayoutConstraint().value().ToString()
                                                   : "NA"));
    DumpLog::GetInstance().AddDesc(std::string("top: ").append(std::to_string(GetOffsetRelativeToWindow().GetY())));
    DumpLog::GetInstance().AddDesc(std::string("left: ").append(std::to_string(GetOffsetRelativeToWindow().GetX())));
    DumpLog::GetInstance().AddDesc(
        std::string("width: ").append(std::to_string(geometryNode_->GetFrameRect().Width())));
    DumpLog::GetInstance().AddDesc(
        std::string("height: ").append(std::to_string(geometryNode_->GetFrameRect().Height())));
    DumpLog::GetInstance().AddDesc(std::string("compid: ").append(propInspectorId_.value_or("")));
    DumpLog::GetInstance().AddDesc(std::string("ContentConstraint: ")
                                       .append(layoutProperty_->GetContentLayoutConstraint().has_value()
                                                   ? layoutProperty_->GetContentLayoutConstraint().value().ToString()
                                                   : "NA"));
    DumpLog::GetInstance().AddDesc(
        std::string("PaintRect: ").append(renderContext_->GetPaintRectWithTransform().ToString()));
    if (pattern_) {
        pattern_->DumpInfo();
    }
}

void FrameNode::ToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    ACE_PROPERTY_TO_JSON_VALUE(layoutProperty_, LayoutProperty);
    ACE_PROPERTY_TO_JSON_VALUE(paintProperty_, PaintProperty);
    ACE_PROPERTY_TO_JSON_VALUE(pattern_, Pattern);
    if (renderContext_) {
        renderContext_->ToJsonValue(json);
    }
    if (pattern_) {
        pattern_->ToJsonValue(json);
    }
}

void FrameNode::OnAttachToMainTree()
{
    UINode::OnAttachToMainTree();
    eventHub_->FireOnAppear();
    renderContext_->OnNodeAppear();
    if (!hasPendingRequest_) {
        return;
    }
    auto context = GetContext();
    CHECK_NULL_VOID(context);
    context->RequestFrame();
    hasPendingRequest_ = false;

    if (IsResponseRegion()) {
        auto parent = GetParent();
        while (parent) {
            auto frameNode = AceType::DynamicCast<FrameNode>(parent);
            if (frameNode) {
                frameNode->MarkResponseRegion(true);
            }
            parent = parent->GetParent();
        }
    }
}

void FrameNode::OnDetachFromMainTree()
{
    eventHub_->FireOnDisappear();
    renderContext_->OnNodeDisappear(this);
}

void FrameNode::SwapDirtyLayoutWrapperOnMainThread(const RefPtr<LayoutWrapper>& dirty)
{
    ACE_FUNCTION_TRACE();
    LOGD("SwapDirtyLayoutWrapperOnMainThread, %{public}s", GetTag().c_str());
    CHECK_NULL_VOID(dirty);

    // active change flag judge.
    SetActive(dirty->IsActive());
    if (!isActive_) {
        LOGD("current node is inactive, don't need to render");
        return;
    }

    // update layout size.
    bool frameSizeChange = geometryNode_->GetFrameSize() != dirty->GetGeometryNode()->GetFrameSize();
    bool frameOffsetChange = geometryNode_->GetFrameOffset() != dirty->GetGeometryNode()->GetFrameOffset();
    bool contentSizeChange = geometryNode_->GetContentSize() != dirty->GetGeometryNode()->GetContentSize();
    bool contentOffsetChange = geometryNode_->GetContentOffset() != dirty->GetGeometryNode()->GetContentOffset();
    bool parentOriginChange =
        geometryNode_->GetParentGlobalOffset() != dirty->GetGeometryNode()->GetParentGlobalOffset();
    // fire OnAreaChanged event.
    if (eventHub_->HasOnAreaChanged() && (frameSizeChange || frameOffsetChange || parentOriginChange)) {
        eventHub_->FireOnAreaChanged(geometryNode_->GetFrameRect(), geometryNode_->GetParentGlobalOffset(),
            dirty->GetGeometryNode()->GetFrameRect(), dirty->GetGeometryNode()->GetParentGlobalOffset());
    }

    SetGeometryNode(dirty->GetGeometryNode());
    if (frameSizeChange || frameOffsetChange || (pattern_->GetSurfaceNodeName().has_value() && contentSizeChange)) {
        if (pattern_->NeedOverridePaintRect()) {
            renderContext_->SyncGeometryProperties(pattern_->GetOverridePaintRect().value_or(RectF()));
        } else {
            renderContext_->SyncGeometryProperties(RawPtr(dirty->GetGeometryNode()));
        }
    }

    // clean layout flag.
    layoutProperty_->CleanDirty();
    DirtySwapConfig config { frameSizeChange, frameOffsetChange, contentSizeChange, contentOffsetChange };
    // check if need to paint content.
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_VOID(layoutAlgorithmWrapper);
    config.skipMeasure = layoutAlgorithmWrapper->SkipMeasure() || dirty->SkipMeasureContent();
    config.skipLayout = layoutAlgorithmWrapper->SkipLayout();
    auto needRerender = pattern_->OnDirtyLayoutWrapperSwap(dirty, config);
    // TODO: temp use and need to delete.
    needRerender = needRerender || pattern_->OnDirtyLayoutWrapperSwap(dirty, config.skipMeasure, config.skipLayout);
    if (needRerender || CheckNeedRender(paintProperty_->GetPropertyChangeFlag())) {
        MarkDirtyNode(true, true, PROPERTY_UPDATE_RENDER);
    }

    // update border.
    if (layoutProperty_->GetBorderWidthProperty()) {
        if (!renderContext_->HasBorderColor()) {
            BorderColorProperty borderColorProperty;
            borderColorProperty.SetColor(Color::BLACK);
            renderContext_->UpdateBorderColor(borderColorProperty);
        }
        if (!renderContext_->HasBorderStyle()) {
            BorderStyleProperty borderStyleProperty;
            borderStyleProperty.SetBorderStyle(BorderStyle::SOLID);
            renderContext_->UpdateBorderStyle(borderStyleProperty);
        }
        if (layoutProperty_->GetLayoutConstraint().has_value()) {
            renderContext_->UpdateBorderWidthF(ConvertToBorderWidthPropertyF(layoutProperty_->GetBorderWidthProperty(),
                ScaleProperty::CreateScaleProperty(),
                layoutProperty_->GetLayoutConstraint()->percentReference.Width()));
        } else {
            renderContext_->UpdateBorderWidthF(ConvertToBorderWidthPropertyF(layoutProperty_->GetBorderWidthProperty(),
                ScaleProperty::CreateScaleProperty(), PipelineContext::GetCurrentRootWidth()));
        }
    }

    // rebuild child render node.
    RebuildRenderContextTree();
}

void FrameNode::SetActive(bool active)
{
    bool activeChanged = false;
    if (active && !isActive_) {
        pattern_->OnActive();
        isActive_ = true;
        activeChanged = true;
    }
    if (!active && isActive_) {
        pattern_->OnInActive();
        isActive_ = false;
        activeChanged = true;
    }
    if (activeChanged) {
        auto parent = GetAncestorNodeOfFrame();
        if (parent) {
            parent->MarkNeedSyncRenderTree();
        }
    }
}

void FrameNode::SetGeometryNode(const RefPtr<GeometryNode>& node)
{
    geometryNode_ = node;
}

std::optional<UITask> FrameNode::CreateLayoutTask(bool forceUseMainThread)
{
    if (!isLayoutDirtyMarked_) {
        return std::nullopt;
    }
    ACE_SCOPED_TRACE("CreateLayoutTask:PrepareTask");
    RefPtr<LayoutWrapper> layoutWrapper;
    UpdateLayoutPropertyFlag();
    layoutWrapper = CreateLayoutWrapper();
    if (!layoutWrapper) {
        return std::nullopt;
    }
    auto task = [layoutWrapper, layoutConstraint = GetLayoutConstraint(), forceUseMainThread]() {
        layoutWrapper->SetActive();
        layoutWrapper->SetRootMeasureNode();
        {
            ACE_SCOPED_TRACE("LayoutWrapper::Measure");
            layoutWrapper->Measure(layoutConstraint);
        }
        {
            ACE_SCOPED_TRACE("LayoutWrapper::Layout");
            layoutWrapper->Layout();
        }
        {
            ACE_SCOPED_TRACE("LayoutWrapper::MountToHostOnMainThread");
            if (forceUseMainThread || layoutWrapper->CheckShouldRunOnMain()) {
                layoutWrapper->MountToHostOnMainThread();
                return;
            }
            auto host = layoutWrapper->GetHostNode();
            CHECK_NULL_VOID(host);
            host->PostTask([layoutWrapper]() { layoutWrapper->MountToHostOnMainThread(); });
        }
    };
    if (forceUseMainThread || layoutWrapper->CheckShouldRunOnMain()) {
        return UITask(std::move(task), MAIN_TASK);
    }
    return UITask(std::move(task), layoutWrapper->CanRunOnWhichThread());
}

std::optional<UITask> FrameNode::CreateRenderTask(bool forceUseMainThread)
{
    if (!isRenderDirtyMarked_) {
        return std::nullopt;
    }
    ACE_SCOPED_TRACE("CreateRenderTask:PrepareTask");
    auto wrapper = CreatePaintWrapper();
    if (!wrapper) {
        return std::nullopt;
    }
    auto task = [wrapper, paintProperty = paintProperty_]() {
        ACE_SCOPED_TRACE("FrameNode::RenderTask");
        wrapper->FlushRender();
        paintProperty->CleanDirty();
    };
    if (forceUseMainThread || wrapper->CheckShouldRunOnMain()) {
        return UITask(std::move(task), MAIN_TASK);
    }
    return UITask(std::move(task), wrapper->CanRunOnWhichThread());
}

LayoutConstraintF FrameNode::GetLayoutConstraint() const
{
    if (geometryNode_->GetParentLayoutConstraint().has_value()) {
        return geometryNode_->GetParentLayoutConstraint().value();
    }
    LayoutConstraintF layoutConstraint;
    layoutConstraint.scaleProperty = ScaleProperty::CreateScaleProperty();
    auto rootWidth = PipelineContext::GetCurrentRootWidth();
    auto rootHeight = PipelineContext::GetCurrentRootHeight();
    layoutConstraint.percentReference.SetWidth(rootWidth);
    layoutConstraint.percentReference.SetHeight(rootHeight);
    layoutConstraint.maxSize.SetWidth(rootWidth);
    layoutConstraint.maxSize.SetHeight(rootHeight);
    return layoutConstraint;
}

OffsetF FrameNode::GetParentGlobalOffset() const
{
    auto parent = GetAncestorNodeOfFrame();
    if (!parent) {
        return { 0.0f, 0.0f };
    }
    return parent->geometryNode_->GetParentGlobalOffset();
}

void FrameNode::UpdateLayoutPropertyFlag()
{
    auto selfFlag = layoutProperty_->GetPropertyChangeFlag();
    if (!CheckUpdateByChildRequest(selfFlag)) {
        return;
    }
    if (CheckForceParentMeasureFlag(selfFlag)) {
        return;
    }
    auto flag = PROPERTY_UPDATE_NORMAL;
    const auto& children = GetChildren();
    for (const auto& child : children) {
        child->UpdateLayoutPropertyFlag();
        child->AdjustParentLayoutFlag(flag);
        if (CheckForceParentMeasureFlag(selfFlag)) {
            break;
        }
    }
    if (CheckForceParentMeasureFlag(flag)) {
        layoutProperty_->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
    }
}

void FrameNode::AdjustParentLayoutFlag(PropertyChangeFlag& flag)
{
    flag = flag | layoutProperty_->GetPropertyChangeFlag();
}

RefPtr<LayoutWrapper> FrameNode::CreateLayoutWrapper(bool forceMeasure, bool forceLayout)
{
    isLayoutDirtyMarked_ = false;
    if (layoutProperty_->GetVisibility().value_or(VisibleType::VISIBLE) == VisibleType::GONE) {
        auto layoutWrapper =
            MakeRefPtr<LayoutWrapper>(WeakClaim(this), MakeRefPtr<GeometryNode>(), MakeRefPtr<LayoutProperty>());
        layoutWrapper->SetLayoutAlgorithm(MakeRefPtr<LayoutAlgorithmWrapper>(nullptr, true, true));
        return layoutWrapper;
    }

    pattern_->BeforeCreateLayoutWrapper();
    if (!isActive_ || forceMeasure) {
        layoutProperty_->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
    }
    if (forceLayout) {
        layoutProperty_->UpdatePropertyChangeFlag(PROPERTY_UPDATE_LAYOUT);
    }
    auto flag = layoutProperty_->GetPropertyChangeFlag();
    auto layoutWrapper = MakeRefPtr<LayoutWrapper>(WeakClaim(this), geometryNode_->Clone(), layoutProperty_);
    LOGD("%{public}s create layout wrapper: %{public}x, %{public}d, %{public}d", GetTag().c_str(), flag, forceMeasure,
        forceLayout);
    do {
        if (CheckNeedMeasure(flag) || forceMeasure) {
            layoutWrapper->SetLayoutAlgorithm(MakeRefPtr<LayoutAlgorithmWrapper>(pattern_->CreateLayoutAlgorithm()));
            bool forceChildMeasure = CheckMeasureFlag(flag) || forceMeasure;
            UpdateChildrenLayoutWrapper(layoutWrapper, forceChildMeasure, false);
            break;
        }
        if (CheckNeedLayout(flag) || forceLayout) {
            layoutWrapper->SetLayoutAlgorithm(
                MakeRefPtr<LayoutAlgorithmWrapper>(pattern_->CreateLayoutAlgorithm(), true, false));
            UpdateChildrenLayoutWrapper(layoutWrapper, false, false);
            break;
        }
        layoutWrapper->SetLayoutAlgorithm(MakeRefPtr<LayoutAlgorithmWrapper>(nullptr, true, true));
    } while (false);
    // check position flag.
    const auto& gridProperty = layoutWrapper->GetLayoutProperty()->GetGridProperty(Claim(this));
    bool hasGridOffset = gridProperty ? (gridProperty->GetOffset() != UNDEFINED_DIMENSION) : false;
    if (renderContext_->HasPosition() || hasGridOffset) {
        layoutWrapper->SetOutOfLayout(true);
    }
    layoutWrapper->SetActive(isActive_);
    return layoutWrapper;
}

void FrameNode::UpdateChildrenLayoutWrapper(const RefPtr<LayoutWrapper>& self, bool forceMeasure, bool forceLayout)
{
    const auto& children = GetChildren();
    for (const auto& child : children) {
        child->AdjustLayoutWrapperTree(self, forceMeasure, forceLayout);
    }
}

void FrameNode::AdjustLayoutWrapperTree(const RefPtr<LayoutWrapper>& parent, bool forceMeasure, bool forceLayout)
{
    ACE_DCHECK(parent);
    auto layoutWrapper = CreateLayoutWrapper(forceMeasure, forceLayout);
    parent->AppendChild(layoutWrapper);
}

RefPtr<PaintWrapper> FrameNode::CreatePaintWrapper()
{
    pattern_->BeforeCreatePaintWrapper();
    isRenderDirtyMarked_ = false;
    auto paintMethod = pattern_->CreateNodePaintMethod();
    if (paintMethod) {
        auto paintWrapper = MakeRefPtr<PaintWrapper>(renderContext_, geometryNode_->Clone(), paintProperty_);
        paintWrapper->SetNodePaintMethod(paintMethod);
        return paintWrapper;
    }
    if (renderContext_->GetAccessibilityFocus().value_or(false)) {
        auto paintWrapper = MakeRefPtr<PaintWrapper>(renderContext_, geometryNode_->Clone(), paintProperty_);
        paintWrapper->SetNodePaintMethod(MakeRefPtr<NodePaintMethod>());
        return paintWrapper;
    }
    return nullptr;
}

void FrameNode::PostTask(std::function<void()>&& task, TaskExecutor::TaskType taskType)
{
    auto context = GetContext();
    CHECK_NULL_VOID(context);
    context->PostAsyncEvent(std::move(task), taskType);
}

void FrameNode::UpdateLayoutConstraint(const MeasureProperty& calcLayoutConstraint)
{
    layoutProperty_->UpdateCalcLayoutProperty(calcLayoutConstraint);
}

void FrameNode::RebuildRenderContextTree()
{
    if (!needSyncRenderTree_) {
        return;
    }
    frameChildren_.clear();
    std::list<RefPtr<FrameNode>> children;
    GenerateOneDepthVisibleFrame(children);
    frameChildren_ = { children.begin(), children.end() };
    renderContext_->RebuildFrame(this, children);
    pattern_->OnRebuildFrame();
    needSyncRenderTree_ = false;
}

void FrameNode::MarkModifyDone()
{
    pattern_->OnModifyDone();
    if (IsResponseRegion()) {
        auto parent = GetParent();
        while (parent) {
            auto frameNode = AceType::DynamicCast<FrameNode>(parent);
            if (frameNode) {
                frameNode->MarkResponseRegion(true);
            }
            parent = parent->GetParent();
        }
    }
    renderContext_->OnModifyDone();
}

void FrameNode::OnMountToParentDone()
{
    pattern_->OnMountToParentDone();
}

void FrameNode::FlushUpdateAndMarkDirty()
{
    MarkDirtyNode();
}

void FrameNode::MarkDirtyNode(PropertyChangeFlag extraFlag)
{
    MarkDirtyNode(IsMeasureBoundary(), IsRenderBoundary(), extraFlag);
}

RefPtr<FrameNode> FrameNode::GetAncestorNodeOfFrame() const
{
    auto parent = GetParent();
    while (parent) {
        if (InstanceOf<FrameNode>(parent)) {
            return DynamicCast<FrameNode>(parent);
        }
        parent = parent->GetParent();
    }
    return nullptr;
}

void FrameNode::MarkNeedRenderOnly()
{
    MarkNeedRender(IsRenderBoundary());
}

void FrameNode::MarkNeedRender(bool isRenderBoundary)
{
    auto context = GetContext();
    CHECK_NULL_VOID(context);
    // If it has dirtyLayoutBox, need to mark dirty after layout done.
    paintProperty_->UpdatePropertyChangeFlag(PROPERTY_UPDATE_RENDER);
    if (isRenderDirtyMarked_ || isLayoutDirtyMarked_) {
        LOGD("this node has already mark dirty, %{public}s, %{public}d, %{public}d", GetTag().c_str(),
            isRenderDirtyMarked_, isLayoutDirtyMarked_);
        return;
    }
    isRenderDirtyMarked_ = true;
    if (isRenderBoundary) {
        context->AddDirtyRenderNode(Claim(this));
        return;
    }
    auto parent = GetAncestorNodeOfFrame();
    if (parent) {
        parent->MarkDirtyNode(PROPERTY_UPDATE_RENDER_BY_CHILD_REQUEST);
    }
}

void FrameNode::MarkDirtyNode(bool isMeasureBoundary, bool isRenderBoundary, PropertyChangeFlag extraFlag)
{
    if (CheckNeedRender(extraFlag)) {
        paintProperty_->UpdatePropertyChangeFlag(extraFlag);
    }
    layoutProperty_->UpdatePropertyChangeFlag(extraFlag);
    paintProperty_->UpdatePropertyChangeFlag(extraFlag);
    auto layoutFlag = layoutProperty_->GetPropertyChangeFlag();
    auto paintFlag = paintProperty_->GetPropertyChangeFlag();
    if (CheckNoChanged(layoutFlag | paintFlag)) {
        LOGD("MarkDirtyNode: flag not changed, node tag: %{public}s", GetTag().c_str());
        return;
    }
    auto context = GetContext();
    CHECK_NULL_VOID(context);

    if (CheckNeedRequestMeasureAndLayout(layoutFlag)) {
        if (isLayoutDirtyMarked_) {
            LOGD("MarkDirtyNode: isLayoutDirtyMarked is true");
            return;
        }
        isLayoutDirtyMarked_ = true;
        if (!isMeasureBoundary && CheckNeedRequestParentMeasure(layoutFlag)) {
            auto parent = GetAncestorNodeOfFrame();
            if (parent) {
                parent->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
            } else {
                context->AddDirtyLayoutNode(Claim(this));
            }
            return;
        }
        context->AddDirtyLayoutNode(Claim(this));
        return;
    }
    layoutProperty_->CleanDirty();
    MarkNeedRender(isRenderBoundary);
}

void FrameNode::OnGenerateOneDepthVisibleFrame(std::list<RefPtr<FrameNode>>& visibleList)
{
    if (isActive_ && IsVisible()) {
        visibleList.emplace_back(Claim(this));
    }
}

bool FrameNode::IsMeasureBoundary()
{
    return isMeasureBoundary_ || pattern_->IsMeasureBoundary();
}

bool FrameNode::IsRenderBoundary()
{
    return pattern_->IsRenderBoundary();
}

const RefPtr<Pattern>& FrameNode::GetPattern() const
{
    return pattern_;
}

bool FrameNode::IsAtomicNode() const
{
    return pattern_->IsAtomicNode();
}

HitTestMode FrameNode::GetHitTestMode() const
{
    auto gestureHub = eventHub_->GetGestureEventHub();
    return gestureHub ? gestureHub->GetHitTestMode() : HitTestMode::HTMDEFAULT;
}

bool FrameNode::GetTouchable() const
{
    auto gestureHub = eventHub_->GetGestureEventHub();
    return gestureHub ? gestureHub->GetTouchable() : true;
}

bool FrameNode::IsResponseRegion() const
{
    auto gestureHub = eventHub_->GetGestureEventHub();
    return gestureHub ? gestureHub->IsResponseRegion() : false;
}

void FrameNode::MarkResponseRegion(bool isResponseRegion)
{
    auto gestureHub = eventHub_->GetGestureEventHub();
    if (gestureHub) {
        gestureHub->MarkResponseRegion(isResponseRegion);
    }
}

HitTestResult FrameNode::TouchTest(const PointF& globalPoint, const PointF& parentLocalPoint,
    const TouchRestrict& touchRestrict, TouchTestResult& result)
{
    if (!isActive_) {
        LOGE("%{public}s is inActive, need't do touch test", GetTag().c_str());
        return HitTestResult::OUT_OF_REGION;
    }
    auto paintRect = renderContext_->GetPaintRectWithTransform();
    auto responseRegionList = GetResponseRegionList(paintRect);
    if (SystemProperties::GetDebugEnabled()) {
        LOGD("TouchTest: point is %{public}s in %{public}s, depth: %{public}d", parentLocalPoint.ToString().c_str(),
            GetTag().c_str(), GetDepth());
        for (const auto& rect : responseRegionList) {
            LOGD("TouchTest: responseRegionList is %{public}s, point is %{public}s", rect.ToString().c_str(),
                parentLocalPoint.ToString().c_str());
        }
    }
    if ((!InResponseRegionList(parentLocalPoint, responseRegionList) || !GetTouchable()) && !IsResponseRegion()) {
        LOGD("TouchTest: point is out of region in %{public}s", GetTag().c_str());
        return HitTestResult::OUT_OF_REGION;
    }

    HitTestResult testResult = HitTestResult::OUT_OF_REGION;
    bool preventBubbling = false;
    // Child nodes are repackaged into gesture groups (parallel gesture groups, exclusive gesture groups, etc.) based on
    // the gesture attributes set by the current parent node (high and low priority, parallel gestures, etc.), the
    // newComingTargets is the template object to collect child nodes gesture and used by gestureHub to pack gesture
    // group.
    TouchTestResult newComingTargets;
    const auto localPoint = parentLocalPoint - paintRect.GetOffset();
    bool consumed = false;
    for (auto iter = frameChildren_.rbegin(); iter != frameChildren_.rend(); ++iter) {
        if (GetHitTestMode() == HitTestMode::HTMBLOCK) {
            break;
        }

        const auto& child = *iter;
        auto childHitResult = child->TouchTest(globalPoint, localPoint, touchRestrict, newComingTargets);
        if (childHitResult == HitTestResult::STOP_BUBBLING) {
            preventBubbling = true;
            consumed = true;
            if ((child->GetHitTestMode() == HitTestMode::HTMDEFAULT) ||
                (child->GetHitTestMode() == HitTestMode::HTMTRANSPARENT_SELF)) {
                break;
            }
        }

        // In normal process, the node block the brother node.
        if (childHitResult == HitTestResult::BUBBLING &&
            ((child->GetHitTestMode() == HitTestMode::HTMDEFAULT) ||
                (child->GetHitTestMode() == HitTestMode::HTMTRANSPARENT_SELF))) {
            consumed = true;
            break;
        }
    }

    // first update HitTestResult by children status.
    if (consumed) {
        testResult = preventBubbling ? HitTestResult::STOP_BUBBLING : HitTestResult::BUBBLING;
        consumed = false;
    }

    if (!preventBubbling && (GetHitTestMode() != HitTestMode::HTMNONE) &&
        InResponseRegionList(parentLocalPoint, responseRegionList)) {
        consumed = true;
        auto gestureHub = eventHub_->GetGestureEventHub();
        if (gestureHub) {
            TouchTestResult finalResult;
            const auto coordinateOffset = globalPoint - localPoint;
            preventBubbling =
                gestureHub->ProcessTouchTestHit(coordinateOffset, touchRestrict, newComingTargets, finalResult);
            newComingTargets.swap(finalResult);
        }
    }

    // combine into exclusive recognizer group.
    result.splice(result.end(), std::move(newComingTargets));
    auto gestureHub = eventHub_->GetGestureEventHub();
    if (gestureHub) {
        gestureHub->CombineIntoExclusiveRecognizer(globalPoint, localPoint, result);
    }

    // consumed by children and return result.
    if (!consumed) {
        return testResult;
    }

    if (testResult == HitTestResult::OUT_OF_REGION) {
        // consume only by self.
        if (preventBubbling) {
            return HitTestResult::STOP_BUBBLING;
        }
        return (GetHitTestMode() == HitTestMode::HTMTRANSPARENT_SELF) ? HitTestResult::SELF_TRANSPARENT
                                                                      : HitTestResult::BUBBLING;
    }
    // consume by self and children.
    return testResult;
}

std::vector<RectF> FrameNode::GetResponseRegionList(const RectF& rect)
{
    std::vector<RectF> responseRegionList;
    auto gestureHub = eventHub_->GetGestureEventHub();
    if (!gestureHub) {
        responseRegionList.emplace_back(rect);
        return responseRegionList;
    }

    if (gestureHub->GetResponseRegion().empty()) {
        responseRegionList.emplace_back(rect);
        return responseRegionList;
    }

    auto scaleProperty = ScaleProperty::CreateScaleProperty();
    for (const auto& region : gestureHub->GetResponseRegion()) {
        auto x = ConvertToPx(region.GetOffset().GetX(), scaleProperty, rect.Width());
        auto y = ConvertToPx(region.GetOffset().GetY(), scaleProperty, rect.Height());
        auto width = ConvertToPx(region.GetWidth(), scaleProperty, rect.Width());
        auto height = ConvertToPx(region.GetHeight(), scaleProperty, rect.Height());
        RectF responseRegion(
            rect.GetOffset().GetX() + x.value(), rect.GetOffset().GetY() + y.value(), width.value(), height.value());
        responseRegionList.emplace_back(responseRegion);
    }
    return responseRegionList;
}

bool FrameNode::InResponseRegionList(const PointF& parentLocalPoint, const std::vector<RectF>& responseRegionList) const
{
    for (const auto& rect : responseRegionList) {
        if (rect.IsInRegion(parentLocalPoint)) {
            return true;
        }
    }
    return false;
}

HitTestResult FrameNode::MouseTest(const PointF& globalPoint, const PointF& parentLocalPoint,
    MouseTestResult& onMouseResult, MouseTestResult& onHoverResult, RefPtr<FrameNode>& hoverNode)
{
    const auto& rect = renderContext_->GetPaintRectWithTransform();
    LOGD("MouseTest: type is %{public}s, the region is %{public}lf, %{public}lf, %{public}lf, %{public}lf",
        GetTag().c_str(), rect.Left(), rect.Top(), rect.Width(), rect.Height());
    // TODO: disableTouchEvent || disabled_ need handle

    // TODO: Region need change to RectList
    if (!rect.IsInRegion(parentLocalPoint)) {
        return HitTestResult::OUT_OF_REGION;
    }

    bool preventBubbling = false;

    const auto localPoint = parentLocalPoint - rect.GetOffset();
    const auto& children = GetChildren();
    for (auto iter = children.rbegin(); iter != children.rend(); ++iter) {
        const auto& child = *iter;
        auto childHitResult = child->MouseTest(globalPoint, localPoint, onMouseResult, onHoverResult, hoverNode);
        if (childHitResult == HitTestResult::STOP_BUBBLING) {
            preventBubbling = true;
        }
        // In normal process, the node block the brother node.
        if (childHitResult == HitTestResult::BUBBLING) {
            // TODO: add hit test mode judge.
            break;
        }
    }

    MouseTestResult mouseResult;
    MouseTestResult hoverResult;
    bool isPrevent = false;
    auto mouseHub = eventHub_->GetInputEventHub();
    if (mouseHub) {
        const auto coordinateOffset = globalPoint - localPoint;
        isPrevent = mouseHub->ProcessMouseTestHit(coordinateOffset, mouseResult, hoverResult, hoverNode);
        onHoverResult.splice(onHoverResult.end(), std::move(hoverResult));
    }

    if (!preventBubbling) {
        preventBubbling = isPrevent;
        onMouseResult.splice(onMouseResult.end(), std::move(mouseResult));
    }
    if (preventBubbling) {
        return HitTestResult::STOP_BUBBLING;
    }
    return HitTestResult::BUBBLING;
}

HitTestResult FrameNode::AxisTest(
    const PointF& globalPoint, const PointF& parentLocalPoint, AxisTestResult& onAxisResult)
{
    const auto& rect = renderContext_->GetPaintRectWithTransform();
    LOGD("AxisTest: type is %{public}s, the region is %{public}lf, %{public}lf, %{public}lf, %{public}lf",
        GetTag().c_str(), rect.Left(), rect.Top(), rect.Width(), rect.Height());
    // TODO: disableTouchEvent || disabled_ need handle

    // TODO: Region need change to RectList
    if (!rect.IsInRegion(parentLocalPoint)) {
        return HitTestResult::OUT_OF_REGION;
    }

    bool preventBubbling = false;

    const auto localPoint = parentLocalPoint - rect.GetOffset();
    const auto& children = GetChildren();
    for (auto iter = children.rbegin(); iter != children.rend(); ++iter) {
        auto& child = *iter;
        auto childHitResult = child->AxisTest(globalPoint, localPoint, onAxisResult);
        if (childHitResult == HitTestResult::STOP_BUBBLING) {
            preventBubbling = true;
        }
        // In normal process, the node block the brother node.
        if (childHitResult == HitTestResult::BUBBLING) {
            // TODO: add hit test mode judge.
            break;
        }
    }

    AxisTestResult axisResult;
    bool isPrevent = false;
    auto inputHub = eventHub_->GetInputEventHub();
    if (inputHub) {
        const auto coordinateOffset = globalPoint - localPoint;
        isPrevent = inputHub->ProcessAxisTestHit(coordinateOffset, axisResult);
    }

    if (!preventBubbling) {
        preventBubbling = isPrevent;
        onAxisResult.splice(onAxisResult.end(), std::move(axisResult));
    }
    if (preventBubbling) {
        return HitTestResult::STOP_BUBBLING;
    }
    return HitTestResult::BUBBLING;
}

RefPtr<FocusHub> FrameNode::GetOrCreateFocusHub() const
{
    if (!pattern_) {
        return eventHub_->GetOrCreateFocusHub();
    }
    return eventHub_->GetOrCreateFocusHub(pattern_->GetFocusPattern().focusType, pattern_->GetFocusPattern().focusable);
}

void FrameNode::OnWindowShow()
{
    pattern_->OnWindowShow();
}

void FrameNode::OnWindowHide()
{
    pattern_->OnWindowHide();
}

void FrameNode::OnWindowFocused()
{
    pattern_->OnWindowFocused();
}

void FrameNode::OnWindowUnfocused()
{
    pattern_->OnWindowUnfocused();
}

OffsetF FrameNode::GetOffsetRelativeToWindow() const
{
    auto offset = geometryNode_->GetFrameOffset();
    auto parent = GetAncestorNodeOfFrame();
    while (parent) {
        offset += parent->geometryNode_->GetFrameOffset();
        parent = parent->GetAncestorNodeOfFrame();
    }

    return offset;
}

void FrameNode::OnNotifyMemoryLevel(int32_t level)
{
    pattern_->OnNotifyMemoryLevel(level);
}

void FrameNode::OnAccessibilityEvent(AccessibilityEventType eventType) const
{
    if (AceApplicationInfo::GetInstance().IsAccessibilityEnabled()) {
        AccessibilityEvent event;
        event.type = eventType;
        event.nodeId = GetId();
        PipelineContext::GetCurrentContext()->SendEventToAccessibility(event);
    }
}

} // namespace OHOS::Ace::NG
