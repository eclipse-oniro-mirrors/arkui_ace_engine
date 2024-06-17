/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/waterflow/water_flow_pattern.h"

#include "base/utils/utils.h"
#include "core/components/scroll/scroll_controller_base.h"
#include "core/components_ng/pattern/waterflow/water_flow_layout_algorithm.h"
#include "core/components_ng/pattern/waterflow/water_flow_paint_method.h"
#include "core/components_ng/pattern/waterflow/water_flow_segmented_layout.h"

namespace OHOS::Ace::NG {
SizeF WaterFlowPattern::GetContentSize() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, SizeF());
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, SizeF());
    return geometryNode->GetPaddingSize();
}

bool WaterFlowPattern::UpdateCurrentOffset(float delta, int32_t source)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);

    // check edgeEffect is not springEffect
    if (!HandleEdgeEffect(delta, source, GetContentSize())) {
        if (IsOutOfBoundary()) {
            host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
        }
        return false;
    }
    SetScrollSource(source);
    FireAndCleanScrollingListener();
    if (GetScrollEdgeEffect()) {
        // over scroll in drag update from normal to over scroll.
        float overScroll = 0.0f;
        if (layoutInfo_.itemStart_) {
            overScroll = layoutInfo_.currentOffset_ + delta;
        }
        if (layoutInfo_.offsetEnd_) {
            overScroll = GetMainContentSize() - (layoutInfo_.GetMaxMainHeight() + layoutInfo_.currentOffset_ - delta);
        }
        if (source == SCROLL_FROM_UPDATE) {
            auto friction = ScrollablePattern::CalculateFriction(std::abs(overScroll) / GetMainContentSize());
            delta *= friction;
        }
    } else {
        if (layoutInfo_.itemStart_ && delta > 0) {
            return false;
        }
        if (layoutInfo_.offsetEnd_ && delta < 0) {
            return false;
        }
        if (GreatNotEqual(delta, 0.0f)) {
            delta = std::min(delta, -layoutInfo_.currentOffset_);
        }
    }
    auto userOffset = FireOnWillScroll(-delta);
    layoutInfo_.prevOffset_ = layoutInfo_.currentOffset_;
    layoutInfo_.currentOffset_ -= userOffset;
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    return true;
};

bool WaterFlowPattern::IsScrollable() const
{
    return !(IsAtTop() && IsAtBottom() && !GetAlwaysEnabled());
}
bool WaterFlowPattern::IsAtTop() const
{
    return layoutInfo_.itemStart_;
};
bool WaterFlowPattern::IsAtBottom() const
{
    return layoutInfo_.offsetEnd_;
};
bool WaterFlowPattern::IsReverse() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto layoutProperty = host->GetLayoutProperty<WaterFlowLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, false);
    return layoutProperty->IsReverse();
}
OverScrollOffset WaterFlowPattern::GetOverScrollOffset(double delta) const
{
    OverScrollOffset offset = { 0, 0 };
    if (layoutInfo_.startIndex_ == 0) {
        auto startPos = layoutInfo_.currentOffset_;
        auto newStartPos = startPos + delta;
        if (startPos > 0 && newStartPos > 0) {
            offset.start = delta;
        }
        if (startPos > 0 && newStartPos <= 0) {
            offset.start = -startPos;
        }
        if (startPos <= 0 && newStartPos > 0) {
            offset.start = newStartPos;
        }
    }
    if (layoutInfo_.itemEnd_) {
        auto endPos = layoutInfo_.currentOffset_ + layoutInfo_.maxHeight_;
        auto newEndPos = endPos + delta;
        if (endPos < layoutInfo_.lastMainSize_ && newEndPos < layoutInfo_.lastMainSize_) {
            offset.end = delta;
        }
        if (endPos < layoutInfo_.lastMainSize_ && newEndPos >= layoutInfo_.lastMainSize_) {
            offset.end = layoutInfo_.lastMainSize_ - endPos;
        }
        if (endPos >= layoutInfo_.lastMainSize_ && newEndPos < layoutInfo_.lastMainSize_) {
            offset.end = newEndPos - layoutInfo_.lastMainSize_;
        }
    }
    return offset;
}

void WaterFlowPattern::UpdateScrollBarOffset()
{
    if (!GetScrollBar() && !GetScrollBarProxy()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto geometryNode = host->GetGeometryNode();
    auto viewSize = geometryNode->GetFrameSize();
    auto overScroll = 0.0f;
    if (Positive(layoutInfo_.currentOffset_)) {
        overScroll = layoutInfo_.currentOffset_;
    } else {
        overScroll = GetMainContentSize() - (layoutInfo_.GetContentHeight() + layoutInfo_.currentOffset_);
        overScroll = Positive(overScroll) ? overScroll : 0.0f;
    }
    HandleScrollBarOutBoundary(overScroll);
    UpdateScrollBarRegion(-layoutInfo_.currentOffset_, layoutInfo_.GetContentHeight(),
        Size(viewSize.Width(), viewSize.Height()), Offset(0.0f, 0.0f));
};

RefPtr<LayoutAlgorithm> WaterFlowPattern::CreateLayoutAlgorithm()
{
    if (targetIndex_.has_value()) {
        layoutInfo_.targetIndex_ = targetIndex_;
    }
    RefPtr<WaterFlowLayoutBase> algorithm;
    if (sections_ || SystemProperties::WaterFlowUseSegmentedLayout()) {
        algorithm = MakeRefPtr<WaterFlowSegmentedLayout>(layoutInfo_);
    } else {
        int32_t footerIndex = -1;
        auto footer = footer_.Upgrade();
        if (footer) {
            int32_t count = footer->FrameCount();
            if (count > 0) {
                footerIndex = 0;
            }
        }
        layoutInfo_.footerIndex_ = footerIndex;
        algorithm = MakeRefPtr<WaterFlowLayoutAlgorithm>(layoutInfo_);
    }
    algorithm->SetCanOverScroll(CanOverScroll(GetScrollSource()));
    return algorithm;
}

RefPtr<NodePaintMethod> WaterFlowPattern::CreateNodePaintMethod()
{
    auto paint = MakeRefPtr<WaterFlowPaintMethod>();
    if (!contentModifier_) {
        contentModifier_ = AceType::MakeRefPtr<WaterFlowContentModifier>();
    }
    paint->SetContentModifier(contentModifier_);

    paint->SetScrollBar(GetScrollBar());
    CreateScrollBarOverlayModifier();
    paint->SetScrollBarOverlayModifier(GetScrollBarOverlayModifier());

    auto scrollEffect = GetScrollEdgeEffect();
    if (scrollEffect && scrollEffect->IsFadeEffect()) {
        paint->SetEdgeEffect(scrollEffect);
    }
    return paint;
}

void WaterFlowPattern::OnModifyDone()
{
    auto layoutProperty = GetLayoutProperty<WaterFlowLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    // SetAxis for scroll event
    SetAxis(layoutProperty->GetAxis());
    if (!GetScrollableEvent()) {
        AddScrollEvent();
    }
    SetEdgeEffect();

    auto paintProperty = GetPaintProperty<ScrollablePaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    if (paintProperty->GetScrollBarProperty()) {
        SetScrollBar(paintProperty->GetScrollBarProperty());
    }
    SetAccessibilityAction();
    Register2DragDropManager();
}

void WaterFlowPattern::TriggerModifyDone()
{
    OnModifyDone();
}

bool WaterFlowPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }
    auto layoutAlgorithmWrapper = dirty->GetLayoutAlgorithm();
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto layoutAlgorithm = DynamicCast<WaterFlowLayoutBase>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithm, false);
    auto layoutInfo = layoutAlgorithm->GetLayoutInfo();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto eventHub = host->GetEventHub<WaterFlowEventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    auto onScroll = eventHub->GetOnScroll();
    PrintOffsetLog(AceLogTag::ACE_WATERFLOW, host->GetId(), prevOffset_ - layoutInfo.currentOffset_);
    if (onScroll) {
        FireOnScroll(prevOffset_ - layoutInfo.currentOffset_, onScroll);
    }
    auto onDidScroll = eventHub->GetOnDidScroll();
    if (onDidScroll) {
        FireOnScroll(prevOffset_ - layoutInfo.currentOffset_, onDidScroll);
    }
    bool indexChanged =
        layoutInfo_.firstIndex_ != layoutInfo.firstIndex_ || layoutInfo_.endIndex_ != layoutInfo.endIndex_;
    if (indexChanged) {
        auto onScrollIndex = eventHub->GetOnScrollIndex();
        if (onScrollIndex) {
            onScrollIndex(layoutInfo.firstIndex_, layoutInfo.endIndex_);
        }
    }
    auto onReachStart = eventHub->GetOnReachStart();
    if (onReachStart && layoutInfo.ReachStart(prevOffset_, !isInitialized_)) {
        onReachStart();
    }
    auto onReachEnd = eventHub->GetOnReachEnd();
    if (onReachEnd && layoutInfo.ReachEnd(prevOffset_)) {
        onReachEnd();
    }
    OnScrollStop(eventHub->GetOnScrollStop());

    layoutInfo_ = std::move(layoutInfo);
    if (targetIndex_.has_value()) {
        ScrollToTargetIndex(targetIndex_.value());
        targetIndex_.reset();
    }
    layoutInfo_.UpdateStartIndex();
    prevOffset_ = layoutInfo_.currentOffset_;
    layoutInfo_.extraOffset_.reset();
    UpdateScrollBarOffset();
    CheckScrollable();

    isInitialized_ = true;

    return NeedRender();
}

bool WaterFlowPattern::ScrollToTargetIndex(int32_t index)
{
    if (index == LAST_ITEM) {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, false);
        auto totalItemCount = host->TotalChildCount();
        index = totalItemCount - 1;
    }
    auto crossIndex = layoutInfo_.GetCrossIndex(index);
    if (crossIndex == -1) {
        return false;
    }
    auto item = layoutInfo_.items_[layoutInfo_.GetSegment(index)].at(crossIndex).at(index);
    float targetPosition = -layoutInfo_.JumpToTargetAlign(item);
    auto extraOffset = GetExtraOffset();
    if (extraOffset.has_value()) {
        targetPosition += extraOffset.value();
        ResetExtraOffset();
    }
    ScrollablePattern::AnimateTo(targetPosition, -1, nullptr, true);
    return true;
}

void WaterFlowPattern::CheckScrollable()
{
    auto layoutProperty = GetLayoutProperty<WaterFlowLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    SetScrollEnabled(IsScrollable());
    if (!layoutProperty->GetScrollEnabled().value_or(IsScrollable())) {
        SetScrollEnabled(false);
    }
}

bool WaterFlowPattern::UpdateStartIndex(int32_t index)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto childCount = host->GetTotalChildCount();
    layoutInfo_.jumpIndex_ = (index == LAST_ITEM ? childCount - 1 : index);
    //if target index is footer, fix align because it will jump after fillViewport.
    if (layoutInfo_.footerIndex_ == 0 && layoutInfo_.jumpIndex_ == childCount - 1) {
        SetScrollAlign(ScrollAlign::END);
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    return true;
}

int32_t WaterFlowPattern::GetRows() const
{
    auto layoutProperty = GetLayoutProperty<WaterFlowLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, 0);

    return layoutProperty->GetAxis() == Axis::VERTICAL ? layoutInfo_.GetMainCount() : layoutInfo_.GetCrossCount();
}

int32_t WaterFlowPattern::GetColumns() const
{
    auto layoutProperty = GetLayoutProperty<WaterFlowLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, 0);

    return layoutProperty->GetAxis() == Axis::VERTICAL ? layoutInfo_.GetCrossCount() : layoutInfo_.GetMainCount();
}

void WaterFlowPattern::SetAccessibilityAction()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto accessibilityProperty = host->GetAccessibilityProperty<AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    accessibilityProperty->SetActionScrollForward([weakPtr = WeakClaim(this)]() {
        const auto& pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        CHECK_NULL_VOID(pattern->IsScrollable());
        pattern->ScrollPage(false);
    });

    accessibilityProperty->SetActionScrollBackward([weakPtr = WeakClaim(this)]() {
        const auto& pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        CHECK_NULL_VOID(pattern->IsScrollable());
        pattern->ScrollPage(true);
    });
}

void WaterFlowPattern::ScrollPage(bool reverse, bool smooth)
{
    CHECK_NULL_VOID(IsScrollable());

    auto layoutProperty = GetLayoutProperty<WaterFlowLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto axis = layoutProperty->GetAxis();

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto mainContentSize = geometryNode->GetPaddingSize().MainSize(axis);
    if (smooth) {
        float distance = reverse ? mainContentSize : -mainContentSize;
        float position = layoutInfo_.currentOffset_ + distance;
        AnimateTo(-position, -1, nullptr, true);
    } else {
        UpdateCurrentOffset(reverse ? mainContentSize : -mainContentSize, SCROLL_FROM_JUMP);
    }
    // AccessibilityEventType::SCROLL_END
}

std::string WaterFlowPattern::ProvideRestoreInfo()
{
    auto jsonObj = JsonUtil::Create(true);
    jsonObj->Put("beginIndex", GetBeginIndex());
    Dimension dimension(GetStoredOffset());
    jsonObj->Put("offset", dimension.ConvertToVp());
    return jsonObj->ToString();
}

void WaterFlowPattern::OnRestoreInfo(const std::string& restoreInfo)
{
    auto info = JsonUtil::ParseJsonString(restoreInfo);
    if (!info->IsValid() || !info->IsObject()) {
        return;
    }
    UpdateStartIndex(info->GetInt("beginIndex"));
    Dimension dimension(info->GetDouble("offset"), DimensionUnit::VP);
    SetRestoreOffset(dimension.ConvertToPx());
    SetScrollAlign(ScrollAlign::START);
}

Rect WaterFlowPattern::GetItemRect(int32_t index) const
{
    if (index < 0 || index < layoutInfo_.startIndex_ || index > layoutInfo_.endIndex_) {
        return Rect();
    }
    index += layoutInfo_.footerIndex_ + 1;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, Rect());
    auto item = host->GetChildByIndex(index);
    CHECK_NULL_RETURN(item, Rect());
    auto itemGeometry = item->GetGeometryNode();
    CHECK_NULL_RETURN(itemGeometry, Rect());
    return Rect(itemGeometry->GetFrameRect().GetX(), itemGeometry->GetFrameRect().GetY(),
        itemGeometry->GetFrameRect().Width(), itemGeometry->GetFrameRect().Height());
}

RefPtr<WaterFlowSections> WaterFlowPattern::GetOrCreateWaterFlowSections()
{
    if (sections_) {
        return sections_;
    }
    sections_ = AceType::MakeRefPtr<WaterFlowSections>();
    auto callback = [weakPattern = WeakClaim(this)](int32_t start) {
        auto pattern = weakPattern.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        context->AddBuildFinishCallBack([weakPattern, start]() {
            auto pattern = weakPattern.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->OnSectionChanged(start);
        });
        context->RequestFrame();
    };
    auto callbackNow = [weakPattern = WeakClaim(this)](int32_t start) {
        auto pattern = weakPattern.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->OnSectionChangedNow(start);
    };
    sections_->SetOnDataChange(callback);
    sections_->SetOnDataChangeNow(callbackNow);
    return sections_;
}

void WaterFlowPattern::OnSectionChanged(int32_t start)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    int32_t childUpdateIdx = host->GetChildrenUpdated();
    if (childUpdateIdx > -1 && layoutInfo_.GetSegment(childUpdateIdx - 1) == start && sections_->IsSpecialUpdate()) {
        // optimize adding or removing children in the last section. Prevent complete reset of that section.
        ++start;
    }
    layoutInfo_.InitSegments(sections_->GetSectionInfo(), start);
    layoutInfo_.margins_.clear();

    MarkDirtyNodeSelf();
}

void WaterFlowPattern::OnSectionChangedNow(int32_t start)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    int32_t childUpdateIdx = host->GetChildrenUpdated();
    if (sections_->IsSpecialUpdateCAPI(childUpdateIdx)) {
        start += sections_->GetSectionInfo().size();
    }
    layoutInfo_.InitSegments(sections_->GetSectionInfo(), start);
    layoutInfo_.margins_.clear();
    MarkDirtyNodeSelf();
}

void WaterFlowPattern::ResetSections()
{
    layoutInfo_.Reset();
    sections_.Reset();
    MarkDirtyNodeSelf();
}

void WaterFlowPattern::ScrollToIndex(int32_t index, bool smooth, ScrollAlign align, std::optional<float> extraOffset)
{
    SetScrollSource(SCROLL_FROM_JUMP);
    SetScrollAlign(align);
    StopAnimate();
    if ((index >= 0) || (index == LAST_ITEM)) {
        if (smooth) {
            SetExtraOffset(extraOffset);
            if (!ScrollToTargetIndex(index)) {
                targetIndex_ = index;
                auto host = GetHost();
                CHECK_NULL_VOID(host);
                host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
            }
        } else {
            UpdateStartIndex(index);
            if (extraOffset.has_value()) {
                layoutInfo_.extraOffset_ = -extraOffset.value();
            }
        }
    }
    FireAndCleanScrollingListener();
}

bool WaterFlowPattern::IsOutOfBoundary(bool useCurrentDelta)
{
    bool outOfStart = layoutInfo_.itemStart_ && Positive(layoutInfo_.currentOffset_);
    bool outOfEnd = layoutInfo_.offsetEnd_ &&
                    LessNotEqual(layoutInfo_.currentOffset_ + layoutInfo_.maxHeight_, layoutInfo_.lastMainSize_);
    return outOfStart || outOfEnd;
}

void WaterFlowPattern::SetEdgeEffectCallback(const RefPtr<ScrollEdgeEffect>& scrollEffect)
{
    scrollEffect->SetCurrentPositionCallback([weak = AceType::WeakClaim(this)]() -> double {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, 0.0);
        return pattern->layoutInfo_.currentOffset_;
    });
    scrollEffect->SetLeadingCallback([weak = AceType::WeakClaim(this)]() -> double {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, 0.0);
        auto leadOffset = pattern->GetMainContentSize() - pattern->layoutInfo_.GetContentHeight();
        if (pattern->GetAlwaysEnabled() && Positive(leadOffset)) {
            return 0.0;
        }
        return Negative(leadOffset) ? leadOffset : 0.0;
    });
    scrollEffect->SetTrailingCallback([]() -> double { return 0.0; });
    scrollEffect->SetInitLeadingCallback([weak = AceType::WeakClaim(this)]() -> double {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, 0.0);
        auto leadOffset = pattern->GetMainContentSize() - pattern->layoutInfo_.GetContentHeight();
        if (pattern->GetAlwaysEnabled() && Positive(leadOffset)) {
            return 0.0;
        }
        return Negative(leadOffset) ? leadOffset : 0.0;
    });
    scrollEffect->SetInitTrailingCallback([]() -> double { return 0.0; });
}

void WaterFlowPattern::MarkDirtyNodeSelf()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void WaterFlowPattern::OnScrollEndCallback()
{
    scrollStop_ = true;
    MarkDirtyNodeSelf();
}

void WaterFlowPattern::OnAnimateStop()
{
    scrollStop_ = true;
    MarkDirtyNodeSelf();
}

bool WaterFlowPattern::NeedRender()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, false);
    auto size = geometryNode->GetPaddingSize();

    auto needRender = lastSize_ != size;
    lastSize_ = size;

    auto property = host->GetLayoutProperty();
    CHECK_NULL_RETURN(host, false);
    needRender = property->GetPaddingProperty() != nullptr || needRender;
    return needRender;
}

void WaterFlowPattern::ResetLayoutInfo()
{
    layoutInfo_.Reset();
    if (sections_) {
        layoutInfo_.InitSegments(sections_->GetSectionInfo(), 0);
    }
}

void WaterFlowPattern::AddFooter(const RefPtr<NG::UINode>& footer)
{
    // assume this is always before other children are modified, because it's called during State update.
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto prevFooter = footer_.Upgrade();
    if (!prevFooter) {
        layoutInfo_.footerIndex_ = 0;
        host->AddChild(footer);
    } else {
        host->ReplaceChild(prevFooter, footer);
    }
    footer_ = footer;
    footer->SetActive(false);
}
} // namespace OHOS::Ace::NG
