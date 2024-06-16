/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/list/list_pattern.h"

#include <string>

#include "base/geometry/axis.h"
#include "base/geometry/rect.h"
#include "base/log/dump_log.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/animation/bilateral_spring_node.h"
#include "core/animation/spring_model.h"
#include "core/common/container.h"
#include "core/components/common/layout/constants.h"
#include "core/components/list/list_theme.h"
#include "core/components/scroll/scroll_bar_theme.h"
#include "core/components_ng/base/inspector_filter.h"
#include "core/components_ng/pattern/list/list_height_offset_calculator.h"
#include "core/components_ng/pattern/list/list_item_group_pattern.h"
#include "core/components_ng/pattern/list/list_item_pattern.h"
#include "core/components_ng/pattern/list/list_lanes_layout_algorithm.h"
#include "core/components_ng/pattern/list/list_layout_algorithm.h"
#include "core/components_ng/pattern/list/list_layout_property.h"
#include "core/components_ng/pattern/scroll/effect/scroll_fade_effect.h"
#include "core/components_ng/pattern/scroll/scroll_spring_effect.h"
#include "core/components_ng/pattern/scrollable/scrollable.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
namespace {
constexpr Dimension CHAIN_INTERVAL_DEFAULT = 20.0_vp;
constexpr double CHAIN_SPRING_MASS = 1.0;
constexpr double CHAIN_SPRING_DAMPING = 30.0;
constexpr double CHAIN_SPRING_STIFFNESS = 228;
constexpr float DEFAULT_MIN_SPACE_SCALE = 0.75f;
constexpr float DEFAULT_MAX_SPACE_SCALE = 2.0f;
constexpr float LIST_FADINGEDGE_DEFAULT = 32.0f;
constexpr float LIST_START_MAIN_POS = 0.0f;
constexpr float LIST_FADE_ERROR_RANGE = 1.0f;
} // namespace

void ListPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto listLayoutProperty = host->GetLayoutProperty<ListLayoutProperty>();
    CHECK_NULL_VOID(listLayoutProperty);
    auto axis = listLayoutProperty->GetListDirection().value_or(Axis::VERTICAL);
    if (axis != GetAxis()) {
        needReEstimateOffset_ = true;
        SetAxis(axis);
        ChangeAxis(GetHost());
    }
    if (!GetScrollableEvent()) {
        AddScrollEvent();
        auto scrollableEvent = GetScrollableEvent();
        CHECK_NULL_VOID(scrollableEvent);
        scrollable_ = scrollableEvent->GetScrollable();
    }

    SetEdgeEffect();

    auto paintProperty = GetPaintProperty<ScrollablePaintProperty>();
    CHECK_NULL_VOID(paintProperty);
    if (paintProperty->GetScrollBarProperty()) {
        SetScrollBar(paintProperty->GetScrollBarProperty());
    }

    SetChainAnimation();
    if (multiSelectable_ && !isMouseEventInit_) {
        InitMouseEvent();
    }
    if (!multiSelectable_ && isMouseEventInit_) {
        UninitMouseEvent();
    }
    auto focusHub = host->GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    InitOnKeyEvent(focusHub);
    Register2DragDropManager();
    SetAccessibilityAction();
    if (IsNeedInitClickEventRecorder()) {
        Pattern::InitClickEventRecorder();
    }
    ReadThemeToFadingEdge();
}

void ListPattern::ReadThemeToFadingEdge()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto listLayoutProperty = host->GetLayoutProperty<ListLayoutProperty>();
    CHECK_NULL_VOID(listLayoutProperty);
    auto conlist = PipelineBase::GetCurrentContextSafely();
    CHECK_NULL_VOID(conlist);
    auto listTheme = conlist->GetTheme<ListTheme>();
    CHECK_NULL_VOID(listTheme);
    auto listThemeFadingEdge = listTheme->GetFadingEdge();
    if (listLayoutProperty->GetFadingEdge().has_value()) {
        isFadingEdge_ = listLayoutProperty->GetFadingEdge().value_or(listThemeFadingEdge);
    } else if (GetAxis() == Axis::HORIZONTAL) {
        isFadingEdge_ = false;
    } else {
        isFadingEdge_ = listThemeFadingEdge;
    }
    auto overlayNode = host->GetOverlayNode();
    if (!overlayNode && isFadingEdge_) {
        CreateAnalyzerOverlay(host);
    }
}

void ListPattern::ChangeAxis(RefPtr<UINode> node)
{
    CHECK_NULL_VOID(node);
    auto children = node->GetChildren();
    for (const auto& child : children) {
        if (AceType::InstanceOf<FrameNode>(child)) {
            auto frameNode = AceType::DynamicCast<FrameNode>(child);
            CHECK_NULL_VOID(frameNode);
            auto listItemPattern = frameNode->GetPattern<ListItemPattern>();
            if (listItemPattern) {
                listItemPattern->ChangeAxis(GetAxis());
                return;
            }
            auto listItemGroupPattern = frameNode->GetPattern<ListItemGroupPattern>();
            if (listItemGroupPattern) {
                frameNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
                ChangeAxis(child);
            }
        } else {
            ChangeAxis(child);
        }
    }
}

bool ListPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto listLayoutAlgorithm = DynamicCast<ListLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(listLayoutAlgorithm, false);
    itemPosition_ = listLayoutAlgorithm->GetItemPosition();
    maxListItemIndex_ = listLayoutAlgorithm->GetMaxListItemIndex();
    spaceWidth_ = listLayoutAlgorithm->GetSpaceWidth();
    float relativeOffset = listLayoutAlgorithm->GetCurrentOffset();
    auto predictSnapOffset = listLayoutAlgorithm->GetPredictSnapOffset();
    auto predictSnapEndPos = listLayoutAlgorithm->GetPredictSnapEndPosition();
    bool isJump = listLayoutAlgorithm->NeedEstimateOffset();
    if (childrenSize_) {
        listTotalHeight_ = posMap_->GetTotalHeight();
        currentOffset_ = itemPosition_.empty() ? 0.0f :
            posMap_->GetPos(itemPosition_.begin()->first, itemPosition_.begin()->second.startPos);
    } else {
        if (isJump || needReEstimateOffset_) {
            lanes_ = listLayoutAlgorithm->GetLanes();
            auto calculate = ListHeightOffsetCalculator(itemPosition_, spaceWidth_, lanes_, GetAxis());
            calculate.GetEstimateHeightAndOffset(GetHost());
            currentOffset_ = calculate.GetEstimateOffset();
            relativeOffset = 0.0f;
            needReEstimateOffset_ = false;
            posMap_->ClearPosMap();
        }
        CalculateCurrentOffset(relativeOffset);
    }
    if (targetIndex_) {
        AnimateToTarget(targetIndex_.value(), targetIndexInGroup_, scrollAlign_);
        targetIndex_.reset();
        targetIndexInGroup_.reset();
    }
    if (predictSnapOffset.has_value()) {
        if (scrollable_ && !NearZero(predictSnapOffset.value()) && !AnimateRunning()) {
            scrollable_->StartScrollSnapMotion(predictSnapOffset.value(), scrollSnapVelocity_);
            if (snapTrigOnScrollStart_) {
                FireOnScrollStart();
            }
        }
        scrollSnapVelocity_ = 0.0f;
        predictSnapOffset_.reset();
        snapTrigOnScrollStart_ = false;
        if (predictSnapEndPos.has_value()) {
            predictSnapEndPos_ = predictSnapEndPos;
        } else {
            predictSnapEndPos_.reset();
        }
    }
    if (predictSnapEndPos.has_value() && predictSnapEndPos_.has_value() &&
        !NearEqual(predictSnapEndPos.value(), predictSnapEndPos_.value())) {
        if (scrollable_) {
            scrollable_->UpdateScrollSnapEndWithOffset(
                predictSnapEndPos.value() - predictSnapEndPos_.value());
        }
        predictSnapEndPos_.reset();
    }

    if (isScrollEnd_) {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, false);
        host->OnAccessibilityEvent(AccessibilityEventType::SCROLL_END);
        // AccessibilityEventType::SCROLL_END
        isScrollEnd_ = false;
    }
    currentDelta_ = 0.0f;
    isNeedCheckOffset_ = false;
    float prevStartOffset = startMainPos_;
    float prevEndOffset = endMainPos_ - contentMainSize_ + contentEndOffset_;
    float prevContentSize = contentMainSize_ - contentStartOffset_ - contentEndOffset_;
    float prevTotalSize = endMainPos_ - startMainPos_;
    contentMainSize_ = listLayoutAlgorithm->GetContentMainSize();
    contentStartOffset_ = listLayoutAlgorithm->GetContentStartOffset();
    contentEndOffset_ = listLayoutAlgorithm->GetContentEndOffset();
    startMainPos_ = listLayoutAlgorithm->GetStartPosition();
    endMainPos_ = listLayoutAlgorithm->GetEndPosition();
    crossMatchChild_ = listLayoutAlgorithm->IsCrossMatchChild();
    bool sizeDiminished =
        LessNotEqual(endMainPos_ - startMainPos_, contentMainSize_ - contentStartOffset_ - contentEndOffset_) &&
        GreatOrEqual(prevTotalSize, prevContentSize) && LessNotEqual(endMainPos_ - startMainPos_, prevTotalSize);
    auto lanesLayoutAlgorithm = DynamicCast<ListLanesLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    if (lanesLayoutAlgorithm) {
        lanesLayoutAlgorithm->SwapLanesItemRange(lanesItemRange_);
        if (lanesLayoutAlgorithm->GetLanes() != lanes_) {
            auto item = swiperItem_.Upgrade();
            if (item) {
                item->SwiperReset();
            }
        }
        lanes_ = lanesLayoutAlgorithm->GetLanes();
        laneGutter_ = lanesLayoutAlgorithm->GetLaneGutter();
    }
    CheckScrollable();

    bool indexChanged = false;
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)) {
        indexChanged = (startIndex_ != listLayoutAlgorithm->GetStartIndex()) ||
                       (endIndex_ != listLayoutAlgorithm->GetEndIndex()) ||
                       (centerIndex_ != listLayoutAlgorithm->GetMidIndex(AceType::RawPtr(dirty)));
    } else {
        indexChanged =
            (startIndex_ != listLayoutAlgorithm->GetStartIndex()) || (endIndex_ != listLayoutAlgorithm->GetEndIndex());
    }
    endIndexChanged_ = endIndex_ != listLayoutAlgorithm->GetEndIndex();
    if (indexChanged) {
        startIndex_ = listLayoutAlgorithm->GetStartIndex();
        endIndex_ = listLayoutAlgorithm->GetEndIndex();
        centerIndex_ = listLayoutAlgorithm->GetMidIndex(AceType::RawPtr(dirty));
    }
    ProcessEvent(indexChanged, relativeOffset, isJump, prevStartOffset, prevEndOffset);
    HandleScrollBarOutBoundary();
    UpdateScrollBarOffset();
    if (config.frameSizeChange) {
        if (GetScrollBar() != nullptr) {
            GetScrollBar()->ScheduleDisappearDelayTask();
        }
    }
    CheckRestartSpring(sizeDiminished);

    DrivenRender(dirty);

    SetScrollSource(SCROLL_FROM_NONE);
    isInitialized_ = true;
    MarkSelectedItems();
    UpdateListDirectionInCardStyle();
    return true;
}

void ListPattern::UpdateListDirectionInCardStyle()
{
    if (isNeedToUpdateListDirection_) {
        auto layoutProperty = GetLayoutProperty<ListLayoutProperty>();
        layoutProperty->UpdateListDirection(Axis::VERTICAL);
        isNeedToUpdateListDirection_ = false;
    }
}

ScrollAlign ListPattern::GetScrollAlignByScrollSnapAlign() const
{
    auto scrollAlign = ScrollAlign::START;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, scrollAlign);
    auto listProperty = host->GetLayoutProperty<ListLayoutProperty>();
    CHECK_NULL_RETURN(listProperty, scrollAlign);
    auto scrollSnapAlign = listProperty->GetScrollSnapAlign().value_or(V2::ScrollSnapAlign::NONE);
    if (scrollSnapAlign == V2::ScrollSnapAlign::CENTER) {
        scrollAlign = ScrollAlign::CENTER;
    }
    return scrollAlign;
}

float ListPattern::CalculateTargetPos(float startPos, float endPos)
{
    float topOffset = startPos - contentStartOffset_;
    float bottomOffset = endPos - contentMainSize_ + contentEndOffset_;
    if (GreatOrEqual(topOffset, 0.0f) && LessOrEqual(bottomOffset, 0.0f)) {
        return 0.0f;
    }
    if ((NearEqual(topOffset, 0.0f) && GreatNotEqual(bottomOffset, 0.0f)) ||
        (LessNotEqual(topOffset, 0.0f) && NearEqual(bottomOffset, 0.0f))) {
        return 0.0f;
    }
    if (LessNotEqual(topOffset, 0.0f) && GreatNotEqual(bottomOffset, 0.0f)) {
        if (GreatOrEqual(std::abs(topOffset), std::abs(bottomOffset))) {
            return bottomOffset;
        } else {
            return topOffset;
        }
    }
    if (GreatNotEqual(std::abs(topOffset), std::abs(bottomOffset))) {
        return bottomOffset;
    } else if (LessNotEqual(std::abs(topOffset), std::abs(bottomOffset))) {
        return topOffset;
    } else {
        if (LessNotEqual(topOffset, 0.0f)) {
            return topOffset;
        } else {
            return bottomOffset;
        }
    }
    return 0.0f;
}

void ListPattern::CreateAnalyzerOverlay(const RefPtr<FrameNode> listNode)
{
    auto builderFunc = []() -> RefPtr<UINode> {
        auto uiNode = FrameNode::GetOrCreateFrameNode(V2::STACK_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() {
                return AceType::MakeRefPtr<LinearLayoutPattern>(true);
            });
        return uiNode;
    };
    auto overlayNode = AceType::DynamicCast<FrameNode>(builderFunc());
    CHECK_NULL_VOID(overlayNode);
    listNode->SetOverlayNode(overlayNode);
    overlayNode->SetParent(AceType::WeakClaim(AceType::RawPtr(listNode)));
    overlayNode->SetActive(true);
    overlayNode->SetHitTestMode(HitTestMode::HTMTRANSPARENT);
    auto overlayProperty = AceType::DynamicCast<LayoutProperty>(overlayNode->GetLayoutProperty());
    CHECK_NULL_VOID(overlayProperty);
    overlayProperty->SetIsOverlayNode(true);
    overlayProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);
    overlayProperty->UpdateAlignment(Alignment::CENTER);
    auto overlayOffsetX = std::make_optional<Dimension>(Dimension::FromString("0px"));
    auto overlayOffsetY = std::make_optional<Dimension>(Dimension::FromString("0px"));
    overlayProperty->SetOverlayOffset(overlayOffsetX, overlayOffsetY);
    auto focusHub = overlayNode->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->SetFocusable(false);
    overlayNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

RefPtr<NodePaintMethod> ListPattern::CreateNodePaintMethod()
{
    auto listLayoutProperty = GetLayoutProperty<ListLayoutProperty>();
    V2::ItemDivider divider;
    if (!chainAnimation_ && listLayoutProperty->HasDivider()) {
        divider = listLayoutProperty->GetDivider().value();
    }
    auto axis = listLayoutProperty->GetListDirection().value_or(Axis::VERTICAL);
    auto drawVertical = (axis == Axis::HORIZONTAL);
    auto paint = MakeRefPtr<ListPaintMethod>(divider, drawVertical, lanes_, spaceWidth_);
    paint->SetScrollBar(GetScrollBar());
    CreateScrollBarOverlayModifier();
    paint->SetScrollBarOverlayModifier(GetScrollBarOverlayModifier());
    paint->SetTotalItemCount(maxListItemIndex_ + 1);
    auto scrollEffect = GetScrollEdgeEffect();
    if (scrollEffect && scrollEffect->IsFadeEffect()) {
        paint->SetEdgeEffect(scrollEffect);
    }
    if (!listContentModifier_) {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, paint);
        auto renderContext = host->GetRenderContext();
        CHECK_NULL_RETURN(renderContext, paint);
        const auto& geometryNode = host->GetGeometryNode();
        auto size = renderContext->GetPaintRectWithoutTransform().GetSize();
        auto& padding = geometryNode->GetPadding();
        if (padding) {
            size.MinusPadding(*padding->left, *padding->right, *padding->top, *padding->bottom);
        }
        OffsetF offset = geometryNode->GetPaddingOffset() - geometryNode->GetFrameOffset();
        listContentModifier_ = AceType::MakeRefPtr<ListContentModifier>(offset, size);
    }
    paint->SetLaneGutter(laneGutter_);
    paint->SetItemsPosition(itemPosition_, pressedItem_);
    paint->SetContentModifier(listContentModifier_);

    UpdateFadingEdge(paint);
    return paint;
}

void ListPattern::UpdateFadingEdge(const RefPtr<ListPaintMethod> paint)
{
    if (LIST_START_MAIN_POS == contentMainSize_) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto overlayNode = host->GetOverlayNode();
    CHECK_NULL_VOID(overlayNode);
    auto overlayRenderContext = overlayNode->GetRenderContext();
    CHECK_NULL_VOID(overlayRenderContext);
    if (!isFadingEdge_) {
        paint->SetOverlayRenderContext(overlayRenderContext);
        paint->SetFadingInfo(false, false);
        isTopEdgeFading_ = false;
        isLowerEdgeFading_ = false;
        return;
    }
    auto isFadingTop = LessNotEqual(startMainPos_, LIST_START_MAIN_POS - LIST_FADE_ERROR_RANGE);
    auto isFadingBottom = GreatNotEqual(endMainPos_, contentMainSize_ + LIST_FADE_ERROR_RANGE);
    if (isFadingTop || isFadingBottom) {
        auto isTopEdgeFadingUpdate = isTopEdgeFading_ != isFadingTop;
        auto isLowerEdgeFadingUpdate = isLowerEdgeFading_ != isFadingBottom;
        if (isTopEdgeFadingUpdate || isLowerEdgeFadingUpdate || (fadingAxis_ != GetAxis())) {
            paint->SetOverlayRenderContext(overlayRenderContext);
            UpdateFadeInfo(isFadingTop, isFadingBottom, paint);
            fadingAxis_ = GetAxis();
        }
    } else if (isTopEdgeFading_ || isLowerEdgeFading_) {
        paint->SetOverlayRenderContext(overlayRenderContext);
        UpdateFadeInfo(isFadingTop, isFadingBottom, paint);
    }
    isTopEdgeFading_ = isFadingTop;
    isLowerEdgeFading_ = isFadingBottom;
}

void ListPattern::UpdateFadeInfo(bool isFadingTop, bool isFadingBottom, const RefPtr<ListPaintMethod> paint)
{
    if (startIndex_ > 0) {
        isFadingTop = true;
    }
    if (endIndex_ < maxListItemIndex_) {
        isFadingBottom = true;
    }
    auto percentFading = CalcDimension(LIST_FADINGEDGE_DEFAULT, DimensionUnit::VP).ConvertToPx() /
        std::abs(contentMainSize_ - LIST_START_MAIN_POS);
    paint->SetFadingInfo(isFadingTop, isFadingBottom, percentFading);
}

bool ListPattern::UpdateStartListItemIndex()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto startWrapper = host->GetOrCreateChildByIndex(startIndex_);
    int32_t startArea = -1;
    int32_t startItemIndexInGroup = -1;
    bool startFlagChanged = false;
    bool startIsGroup = startWrapper && startWrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
    if (startIsGroup) {
        auto startPattern = startWrapper->GetHostNode()->GetPattern<ListItemGroupPattern>();
        VisibleContentInfo startGroupInfo = startPattern->GetStartListItemIndex();
        startFlagChanged = (startInfo_.index != startIndex_) || (startInfo_.area != startGroupInfo.area) ||
                           (startInfo_.indexInGroup != startGroupInfo.indexInGroup);
        startArea = startGroupInfo.area;
        startItemIndexInGroup = startGroupInfo.indexInGroup;
    }
    startInfo_ = { startIndex_, startArea, startItemIndexInGroup };
    return startFlagChanged;
}

bool ListPattern::UpdateEndListItemIndex()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto endWrapper = host->GetOrCreateChildByIndex(endIndex_);
    int32_t endArea = -1;
    int32_t endItemIndexInGroup = -1;
    bool endFlagChanged = false;
    bool endIsGroup = endWrapper && endWrapper->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG;
    if (endIsGroup) {
        auto endPattern = endWrapper->GetHostNode()->GetPattern<ListItemGroupPattern>();
        VisibleContentInfo endGroupInfo = endPattern->GetEndListItemIndex();
        endFlagChanged = (endInfo_.index != endIndex_) || (endInfo_.area != endGroupInfo.area) ||
                         (endInfo_.indexInGroup != endGroupInfo.indexInGroup);
        endArea = endGroupInfo.area;
        endItemIndexInGroup = endGroupInfo.indexInGroup;
    }
    endInfo_ = { endIndex_, endArea, endItemIndexInGroup };
    return endFlagChanged;
}

void ListPattern::ProcessEvent(
    bool indexChanged, float finalOffset, bool isJump, float prevStartOffset, float prevEndOffset)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto listEventHub = host->GetEventHub<ListEventHub>();
    CHECK_NULL_VOID(listEventHub);

    paintStateFlag_ = !NearZero(finalOffset) && !isJump;
    isFramePaintStateValid_ = true;
    auto onScroll = listEventHub->GetOnScroll();
    PrintOffsetLog(AceLogTag::ACE_LIST, host->GetId(), finalOffset);
    if (onScroll) {
        if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)) {
            FireOnScroll(finalOffset, onScroll);
        } else {
            if (!NearZero(finalOffset)) {
                auto offsetPX = Dimension(finalOffset);
                auto offsetVP = Dimension(offsetPX.ConvertToVp(), DimensionUnit::VP);
                auto source = GetScrollSource();
                if (source == SCROLL_FROM_AXIS || source == SCROLL_FROM_BAR ||
                    source == SCROLL_FROM_ANIMATION_CONTROLLER) {
                    source = SCROLL_FROM_NONE;
                }
                onScroll(offsetVP, GetScrollState(source));
            }
        }
    }

    auto onDidScroll = listEventHub->GetOnDidScroll();
    if (onDidScroll) {
        FireOnScroll(finalOffset, onDidScroll);
    }

    if (indexChanged) {
        auto onScrollIndex = listEventHub->GetOnScrollIndex();
        if (onScrollIndex) {
            int32_t startIndex = startIndex_ == -1 ? 0 : startIndex_;
            int32_t endIndex = endIndex_ == -1 ? 0 : endIndex_;
            onScrollIndex(startIndex, endIndex, centerIndex_);
        }
    }

    auto onScrollVisibleContentChange = listEventHub->GetOnScrollVisibleContentChange();
    if (onScrollVisibleContentChange) {
        bool startChanged = UpdateStartListItemIndex();
        bool endChanged = UpdateEndListItemIndex();
        if (indexChanged || startChanged || endChanged) {
            onScrollVisibleContentChange(startInfo_, endInfo_);
        }
    }

    auto onReachStart = listEventHub->GetOnReachStart();
    if (onReachStart && (startIndex_ == 0)) {
        bool scrollUpToStart = GreatNotEqual(prevStartOffset, contentStartOffset_) &&
            LessOrEqual(startMainPos_, contentStartOffset_);
        bool scrollDownToStart = (LessNotEqual(prevStartOffset, contentStartOffset_) || !isInitialized_) &&
            GreatOrEqual(startMainPos_, contentStartOffset_);
        if (scrollUpToStart || scrollDownToStart) {
            onReachStart();
        }
    }
    auto onReachEnd = listEventHub->GetOnReachEnd();
    if (onReachEnd && (endIndex_ == maxListItemIndex_)) {
        float endOffset = endMainPos_ - contentMainSize_ + contentEndOffset_;
        // deltaOffset passes through multiple items also needs to fire reachEnd
        bool scrollUpToEnd =
            (endIndexChanged_ || (Positive(prevEndOffset) || !isInitialized_)) && NonPositive(endOffset);
        bool scrollDownToEnd = Negative(prevEndOffset) && NonNegative(endOffset);
        if (scrollUpToEnd || (scrollDownToEnd && GetScrollSource() != SCROLL_FROM_NONE)) {
            onReachEnd();
        }
    }

    OnScrollStop(listEventHub->GetOnScrollStop());
}

void ListPattern::DrivenRender(const RefPtr<LayoutWrapper>& layoutWrapper)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto listLayoutProperty = host->GetLayoutProperty<ListLayoutProperty>();
    auto listPaintProperty = host->GetPaintProperty<ScrollablePaintProperty>();
    auto axis = listLayoutProperty->GetListDirection().value_or(Axis::VERTICAL);
    auto stickyStyle = listLayoutProperty->GetStickyStyle().value_or(V2::StickyStyle::NONE);
    bool barNeedPaint = GetScrollBar() ? GetScrollBar()->NeedPaint() : false;
    auto chainAnimation = listLayoutProperty->GetChainAnimation().value_or(false);
    bool drivenRender = !(axis != Axis::VERTICAL || stickyStyle != V2::StickyStyle::NONE || barNeedPaint ||
                          chainAnimation || !isScrollable_);

    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->MarkDrivenRender(drivenRender);
    if (drivenRender && isFramePaintStateValid_) {
        // Mark items
        int32_t indexStep = 0;
        int32_t startIndex = itemPosition_.empty() ? 0 : itemPosition_.begin()->first;
        for (auto& pos : itemPosition_) {
            auto wrapper = layoutWrapper->GetOrCreateChildByIndex(pos.first);
            CHECK_NULL_VOID(wrapper);
            auto itemHost = wrapper->GetHostNode();
            CHECK_NULL_VOID(itemHost);
            auto itemRenderContext = itemHost->GetRenderContext();
            CHECK_NULL_VOID(itemRenderContext);
            itemRenderContext->MarkDrivenRenderItemIndex(startIndex + indexStep);
            indexStep++;
        }
        renderContext->MarkDrivenRenderFramePaintState(paintStateFlag_);
        isFramePaintStateValid_ = false;
    }
}

void ListPattern::CheckScrollable()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(hub);
    auto gestureHub = hub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    auto listProperty = GetLayoutProperty<ListLayoutProperty>();
    CHECK_NULL_VOID(listProperty);
    if (itemPosition_.empty()) {
        isScrollable_ = false;
    } else {
        if ((itemPosition_.begin()->first == 0) && (itemPosition_.rbegin()->first == maxListItemIndex_) &&
            !IsScrollSnapAlignCenter()) {
            isScrollable_ = GetAlwaysEnabled() || GreatNotEqual(endMainPos_ - startMainPos_,
                contentMainSize_ - contentStartOffset_ - contentEndOffset_);
        } else {
            isScrollable_ = true;
        }
    }

    SetScrollEnabled(isScrollable_);

    if (!listProperty->GetScrollEnabled().value_or(isScrollable_)) {
        SetScrollEnabled(false);
    }
}

RefPtr<LayoutAlgorithm> ListPattern::CreateLayoutAlgorithm()
{
    auto listLayoutProperty = GetLayoutProperty<ListLayoutProperty>();
    CHECK_NULL_RETURN(listLayoutProperty, nullptr);
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm;
    if (listLayoutProperty->HasLanes() || listLayoutProperty->HasLaneMinLength() ||
        listLayoutProperty->HasLaneMaxLength()) {
        auto lanesLayoutAlgorithm = MakeRefPtr<ListLanesLayoutAlgorithm>();
        RefreshLanesItemRange();
        lanesLayoutAlgorithm->SwapLanesItemRange(lanesItemRange_);
        lanesLayoutAlgorithm->SetLanes(lanes_);
        listLayoutAlgorithm.Swap(lanesLayoutAlgorithm);
    } else {
        listLayoutAlgorithm.Swap(MakeRefPtr<ListLayoutAlgorithm>());
    }
    if (!posMap_) {
        posMap_ = MakeRefPtr<ListPositionMap>();
    }
    if (childrenSize_) {
        listLayoutAlgorithm->SetListChildrenMainSize(childrenSize_);
        listLayoutAlgorithm->SetListPositionMap(posMap_);
    }
    if (!isInitialized_) {
        jumpIndex_ = listLayoutProperty->GetInitialIndex().value_or(0);
        if (NeedScrollSnapAlignEffect()) {
            scrollAlign_ = GetScrollAlignByScrollSnapAlign();
        }
    }
    if (jumpIndex_) {
        listLayoutAlgorithm->SetIndex(jumpIndex_.value());
        listLayoutAlgorithm->SetIndexAlignment(scrollAlign_);
        jumpIndex_.reset();
    }
    if (targetIndex_) {
        listLayoutAlgorithm->SetTargetIndex(targetIndex_.value());
        listLayoutAlgorithm->SetIndexAlignment(scrollAlign_);
    }
    if (jumpIndexInGroup_) {
        listLayoutAlgorithm->SetIndexInGroup(jumpIndexInGroup_.value());
        jumpIndexInGroup_.reset();
    }
    if (predictSnapOffset_.has_value()) {
        listLayoutAlgorithm->SetPredictSnapOffset(predictSnapOffset_.value());
    }
    listLayoutAlgorithm->SetTotalOffset(GetTotalOffset());
    listLayoutAlgorithm->SetCurrentDelta(currentDelta_);
    listLayoutAlgorithm->SetIsNeedCheckOffset(isNeedCheckOffset_);
    listLayoutAlgorithm->SetItemsPosition(itemPosition_);
    listLayoutAlgorithm->SetPrevContentMainSize(contentMainSize_);
    listLayoutAlgorithm->SetPrevContentStartOffset(contentStartOffset_);
    listLayoutAlgorithm->SetPrevContentEndOffset(contentEndOffset_);
    if (IsOutOfBoundary(false) && GetScrollSource() != SCROLL_FROM_AXIS) {
        listLayoutAlgorithm->SetOverScrollFeature();
    }
    listLayoutAlgorithm->SetIsSpringEffect(IsScrollableSpringEffect());
    listLayoutAlgorithm->SetCanOverScroll(CanOverScroll(GetScrollSource()));
    if (chainAnimation_) {
        SetChainAnimationLayoutAlgorithm(listLayoutAlgorithm, listLayoutProperty);
        SetChainAnimationToPosMap();
    }
    if (predictSnapEndPos_.has_value()) {
        listLayoutAlgorithm->SetPredictSnapEndPosition(predictSnapEndPos_.value());
    }
    return listLayoutAlgorithm;
}

void ListPattern::SetChainAnimationToPosMap()
{
    CHECK_NULL_VOID(posMap_);
    posMap_->SetChainOffsetCallback([weak = AceType::WeakClaim(this)](int32_t index) {
        auto list = weak.Upgrade();
        CHECK_NULL_RETURN(list, 0.0f);
        return list->GetChainDelta(index);
    });
}

void ListPattern::SetChainAnimationLayoutAlgorithm(
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm, RefPtr<ListLayoutProperty> listLayoutProperty)
{
    CHECK_NULL_VOID(listLayoutAlgorithm);
    CHECK_NULL_VOID(listLayoutProperty);
    listLayoutAlgorithm->SetChainOffsetCallback([weak = AceType::WeakClaim(this)](int32_t index) {
        auto list = weak.Upgrade();
        CHECK_NULL_RETURN(list, 0.0f);
        return list->GetChainDelta(index);
    });
    if (!listLayoutProperty->GetSpace().has_value() && chainAnimation_) {
        listLayoutAlgorithm->SetChainInterval(CHAIN_INTERVAL_DEFAULT.ConvertToPx());
    }
}

bool ListPattern::IsScrollSnapAlignCenter() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto listProperty = host->GetLayoutProperty<ListLayoutProperty>();
    CHECK_NULL_RETURN(listProperty, false);
    auto scrollSnapAlign = listProperty->GetScrollSnapAlign().value_or(V2::ScrollSnapAlign::NONE);
    if (scrollSnapAlign == V2::ScrollSnapAlign::CENTER) {
        return true;
    }

    return false;
}

void ListPattern::UpdateScrollSnap()
{
    if (!AnimateStoped()) {
        return;
    }
    predictSnapOffset_ = 0.0f;
}

bool ListPattern::NeedScrollSnapAlignEffect() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto listProperty = host->GetLayoutProperty<ListLayoutProperty>();
    CHECK_NULL_RETURN(listProperty, false);
    auto scrollSnapAlign = listProperty->GetScrollSnapAlign().value_or(V2::ScrollSnapAlign::NONE);
    if (scrollSnapAlign == V2::ScrollSnapAlign::NONE) {
        return false;
    }

    return true;
}

bool ListPattern::IsAtTop() const
{
    bool groupAtStart = true;
    bool groupAtEnd = true;
    GetListItemGroupEdge(groupAtStart, groupAtEnd);
    return (startIndex_ == 0 && groupAtStart) &&
           NonNegative(startMainPos_ - currentDelta_ + GetChainDelta(0) - contentStartOffset_);
}

bool ListPattern::IsAtBottom() const
{
    bool groupAtStart = true;
    bool groupAtEnd = true;
    GetListItemGroupEdge(groupAtStart, groupAtEnd);
    return (endIndex_ == maxListItemIndex_ && groupAtEnd) &&
           LessOrEqual(endMainPos_ - currentDelta_ + GetChainDelta(endIndex_), contentMainSize_ - contentEndOffset_);
}

bool ListPattern::OutBoundaryCallback()
{
    bool outBoundary = IsOutOfBoundary();
    if (!dragFromSpring_ && outBoundary && chainAnimation_) {
        chainAnimation_->SetOverDrag(false);
        auto delta = chainAnimation_->SetControlIndex(IsAtTop() ? 0 : maxListItemIndex_);
        currentDelta_ -= delta;
        dragFromSpring_ = true;
    }
    return outBoundary;
}

void ListPattern::GetListItemGroupEdge(bool& groupAtStart, bool& groupAtEnd) const
{
    if (itemPosition_.empty()) {
        return;
    }
    bool firstIsGroup = startIndex_ == 0 && itemPosition_.begin()->second.isGroup;
    bool lastIsGroup = endIndex_ == maxListItemIndex_ && itemPosition_.rbegin()->second.isGroup;
    if (!firstIsGroup && !lastIsGroup) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    std::list<RefPtr<FrameNode>> childrens;
    host->GenerateOneDepthVisibleFrame(childrens);
    if (childrens.empty()) {
        return;
    }
    if (firstIsGroup) {
        auto itemGroup = (*childrens.begin())->GetPattern<ListItemGroupPattern>();
        if (itemGroup) {
            groupAtStart = itemGroup->GetDisplayStartIndexInGroup() == 0;
        }
    }
    if (lastIsGroup) {
        auto itemGroup = (*childrens.rbegin())->GetPattern<ListItemGroupPattern>();
        if (itemGroup) {
            groupAtEnd = itemGroup->GetDisplayEndIndexInGroup() == itemGroup->GetEndIndexInGroup() ||
                         LessOrEqual(itemPosition_.rbegin()->second.endPos, contentMainSize_);
        }
    }
}

float ListPattern::GetOffsetWithLimit(float offset) const
{
    auto currentOffset = GetTotalOffset() + contentStartOffset_;
    if (Positive(offset)) {
        return std::min(currentOffset, offset);
    } else if (Negative(offset)) {
        auto remainHeight = GetTotalHeight() - currentOffset;
        return std::max(offset, -remainHeight);
    }
    return 0;
}

OverScrollOffset ListPattern::GetOverScrollOffset(double delta) const
{
    OverScrollOffset offset = { 0, 0 };
    bool groupAtStart = true;
    bool groupAtEnd = true;
    GetListItemGroupEdge(groupAtStart, groupAtEnd);
    if (startIndex_ == 0 && groupAtStart) {
        offset.start = GetStartOverScrollOffset(delta);
    }
    if (endIndex_ == maxListItemIndex_ && groupAtEnd) {
        offset.end = GetEndOverScrollOffset(delta);
    }
    return offset;
}

float ListPattern::GetStartOverScrollOffset(float offset) const
{
    float startOffset = 0.0f;
    auto startPos = startMainPos_ + GetChainDelta(0);
    auto newStartPos = startPos + offset;
    if (startPos > contentStartOffset_ && newStartPos > contentStartOffset_) {
        startOffset = offset;
    }
    if (startPos > contentStartOffset_ && newStartPos <= contentStartOffset_) {
        startOffset = contentStartOffset_ - startPos;
    }
    if (startPos <= contentStartOffset_ && newStartPos > contentStartOffset_) {
        startOffset = newStartPos - contentStartOffset_;
    }
    if (IsScrollSnapAlignCenter() && !itemPosition_.empty()) {
        float startItemHeight = itemPosition_.begin()->second.endPos - itemPosition_.begin()->second.startPos;
        if (newStartPos > (contentMainSize_ / 2.0f - startItemHeight / 2.0f - spaceWidth_ / 2.0f)) {
            startOffset = newStartPos - (contentMainSize_ / 2.0f - startItemHeight / 2.0f - spaceWidth_ / 2.0f);
        }
    }
    return startOffset;
}

float ListPattern::GetEndOverScrollOffset(float offset) const
{
    float endOffset = 0.0f;
    auto endPos = endMainPos_ + GetChainDelta(endIndex_);
    auto contentEndPos = contentMainSize_ - contentEndOffset_;
    if (GreatNotEqual(contentEndPos, endMainPos_ - startMainPos_) && !IsScrollSnapAlignCenter()) {
        endPos = startMainPos_ + contentEndPos;
    }
    auto newEndPos = endPos + offset;
    if (endPos < contentEndPos && newEndPos < contentEndPos) {
        endOffset = offset;
    }
    if (endPos < contentEndPos && newEndPos >= contentEndPos) {
        endOffset = contentEndPos - endPos;
    }
    if (endPos >= contentEndPos && newEndPos < contentEndPos) {
        endOffset = newEndPos - contentEndPos;
    }
    if (IsScrollSnapAlignCenter() && !itemPosition_.empty()) {
        float endItemHeight = itemPosition_.begin()->second.endPos - itemPosition_.begin()->second.startPos;
        if (newEndPos < (contentMainSize_ / 2.0f + endItemHeight / 2.0f + spaceWidth_ / 2.0f)) {
            endOffset = newEndPos - (contentMainSize_ / 2.0f + endItemHeight / 2.0f + spaceWidth_ / 2.0f);
        }
    }
    return endOffset;
}

bool ListPattern::UpdateCurrentOffset(float offset, int32_t source)
{
    CHECK_NULL_RETURN(!itemPosition_.empty(), false);

    // check edgeEffect is not springEffect
    if (!jumpIndex_.has_value() && !targetIndex_.has_value() && !HandleEdgeEffect(offset, source, GetContentSize())) {
        if (IsOutOfBoundary(false)) {
            MarkDirtyNodeSelf();
        }
        return false;
    }
    UpdateFrameSizeToWeb();
    SetScrollSource(source);
    FireAndCleanScrollingListener();
    auto lastDelta = currentDelta_;
    currentDelta_ = currentDelta_ - offset;
    if (source == SCROLL_FROM_BAR || source == SCROLL_FROM_BAR_FLING) {
        isNeedCheckOffset_ = true;
    }
    if (!NearZero(offset)) {
        MarkDirtyNodeSelf();
    }
    if (!IsOutOfBoundary() || !isScrollable_) {
        auto userOffset = FireOnWillScroll(currentDelta_ - lastDelta);
        currentDelta_ = lastDelta + userOffset;
        return true;
    }

    // over scroll in drag update from normal to over scroll.
    float overScroll = 0.0f;
    // over scroll in drag update during over scroll.
    auto startPos = startMainPos_ - currentDelta_;
    if ((itemPosition_.begin()->first == 0) && GreatNotEqual(startPos, contentStartOffset_)) {
        overScroll = startPos - contentStartOffset_;
    } else {
        overScroll = contentMainSize_ - contentEndOffset_ - (endMainPos_ - currentDelta_);
    }
    if (IsScrollSnapAlignCenter()) {
        auto itemHeight = itemPosition_.begin()->second.endPos - itemPosition_.begin()->second.startPos;
        auto endPos = endMainPos_ - currentDelta_;
        if (startIndex_ == 0 && Positive(startPos + itemHeight / 2.0f - contentMainSize_ / 2.0f)) {
            overScroll = startPos + itemHeight / 2.0f - contentMainSize_ / 2.0f;
        } else if ((endIndex_ == maxListItemIndex_) &&
                   LessNotEqual(endPos - itemHeight / 2.0f, contentMainSize_ / 2.0f)) {
            overScroll = endPos - itemHeight / 2.0f - contentMainSize_ / 2.0f;
        }
    }

    if (GetScrollSource() == SCROLL_FROM_UPDATE) {
        // adjust offset.
        auto friction = ScrollablePattern::CalculateFriction(std::abs(overScroll) / contentMainSize_);
        currentDelta_ = currentDelta_ * friction;
    }

    auto userOffset = FireOnWillScroll(currentDelta_ - lastDelta);
    currentDelta_ = lastDelta + userOffset;
    return true;
}

void ListPattern::MarkDirtyNodeSelf()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (!crossMatchChild_) {
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    } else {
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT);
    }
}

void ListPattern::OnScrollEndCallback()
{
    scrollStop_ = true;
    MarkDirtyNodeSelf();
}

SizeF ListPattern::GetContentSize() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, SizeF());
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, SizeF());
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, SizeF());
    auto size = renderContext->GetPaintRectWithoutTransform().GetSize();
    auto& padding = geometryNode->GetPadding();
    if (padding) {
        size.MinusPadding(*padding->left, *padding->right, *padding->top, *padding->bottom);
    }
    return size;
}

bool ListPattern::IsOutOfBoundary(bool useCurrentDelta)
{
    if (itemPosition_.empty()) {
        return false;
    }
    auto startPos = useCurrentDelta ? startMainPos_ - currentDelta_ : startMainPos_;
    auto endPos = useCurrentDelta ? endMainPos_ - currentDelta_ : endMainPos_;
    if (startIndex_ == 0) {
        startPos += GetChainDelta(0);
    }
    if (endIndex_ == maxListItemIndex_) {
        endPos += GetChainDelta(endIndex_);
    }
    auto contentEndPos = contentMainSize_ - contentEndOffset_;
    bool outOfStart = (startIndex_ == 0) && GreatNotEqual(startPos, contentStartOffset_) &&
        GreatNotEqual(endPos, contentEndPos);
    if (GetAlwaysEnabled()) {
        outOfStart = (startIndex_ == 0) && GreatNotEqual(startPos, contentStartOffset_);
    }

    bool outOfEnd = (endIndex_ == maxListItemIndex_) && LessNotEqual(endPos, contentEndPos) &&
        LessNotEqual(startPos, contentStartOffset_);
    return outOfStart || outOfEnd;
}

bool ListPattern::OnScrollCallback(float offset, int32_t source)
{
    if (source == SCROLL_FROM_START) {
        ProcessDragStart(offset);
        auto item = swiperItem_.Upgrade();
        if (item) {
            item->SwiperReset();
        }
        FireOnScrollStart();
        return true;
    }
    ProcessDragUpdate(offset, source);
    return UpdateCurrentOffset(offset, source);
}

bool ListPattern::OnScrollSnapCallback(double targetOffset, double velocity)
{
    auto listProperty = GetLayoutProperty<ListLayoutProperty>();
    CHECK_NULL_RETURN(listProperty, false);
    auto scrollSnapAlign = listProperty->GetScrollSnapAlign().value_or(V2::ScrollSnapAlign::NONE);
    if (scrollSnapAlign == V2::ScrollSnapAlign::NONE) {
        return false;
    }
    if (AnimateRunning()) {
        return false;
    }
    if (!GetIsDragging()) {
        snapTrigOnScrollStart_ = true;
    }
    predictSnapOffset_ = targetOffset;
    scrollSnapVelocity_ = velocity;
    MarkDirtyNodeSelf();
    return true;
}

void ListPattern::SetEdgeEffectCallback(const RefPtr<ScrollEdgeEffect>& scrollEffect)
{
    scrollEffect->SetCurrentPositionCallback([weak = AceType::WeakClaim(this)]() -> double {
        auto list = weak.Upgrade();
        CHECK_NULL_RETURN(list, 0.0);
        return list->startMainPos_ + list->GetChainDelta(list->startIndex_) - list->currentDelta_;
    });
    scrollEffect->SetLeadingCallback([weak = AceType::WeakClaim(this)]() -> double {
        auto list = weak.Upgrade();
        auto endPos = list->endMainPos_ + list->GetChainDelta(list->endIndex_);
        auto startPos = list->startMainPos_ + list->GetChainDelta(list->startIndex_);
        if (list->IsScrollSnapAlignCenter() && !list->itemPosition_.empty()) {
            float endItemHeight =
                list->itemPosition_.rbegin()->second.endPos - list->itemPosition_.rbegin()->second.startPos;
            return list->contentMainSize_ / 2.0f + endItemHeight / 2.0f - (endPos - startPos);
        }
        float leading = list->contentMainSize_ - (endPos - startPos) - list->contentEndOffset_;
        return (list->startIndex_ == 0) ? std::min(leading, list->contentStartOffset_) : leading;
    });
    scrollEffect->SetTrailingCallback([weak = AceType::WeakClaim(this)]() -> double {
        auto list = weak.Upgrade();
        CHECK_NULL_RETURN(list, 0.0);
        if (list->IsScrollSnapAlignCenter() && !list->itemPosition_.empty()) {
            float startItemHeight =
                list->itemPosition_.begin()->second.endPos - list->itemPosition_.begin()->second.startPos;
            return list->contentMainSize_ / 2.0f - startItemHeight / 2.0f;
        }

        return list->contentStartOffset_;
    });
    scrollEffect->SetInitLeadingCallback([weak = AceType::WeakClaim(this)]() -> double {
        auto list = weak.Upgrade();
        auto endPos = list->endMainPos_ + list->GetChainDelta(list->endIndex_);
        auto startPos = list->startMainPos_ + list->GetChainDelta(list->startIndex_);
        if (list->IsScrollSnapAlignCenter() && !list->itemPosition_.empty()) {
            float endItemHeight =
                list->itemPosition_.rbegin()->second.endPos - list->itemPosition_.rbegin()->second.startPos;
            return list->contentMainSize_ / 2.0f + endItemHeight / 2.0f - (endPos - startPos);
        }
        float leading = list->contentMainSize_ - (endPos - startPos) - list->contentEndOffset_;
        return (list->startIndex_ == 0) ? std::min(leading, list->contentStartOffset_) : leading;
    });
    scrollEffect->SetInitTrailingCallback([weak = AceType::WeakClaim(this)]() -> double {
        auto list = weak.Upgrade();
        CHECK_NULL_RETURN(list, 0.0);
        if (list->IsScrollSnapAlignCenter() && !list->itemPosition_.empty()) {
            float startItemHeight =
                list->itemPosition_.begin()->second.endPos - list->itemPosition_.begin()->second.startPos;
            return list->contentMainSize_ / 2.0f - startItemHeight / 2.0f;
        }

        return list->contentStartOffset_;
    });
}

void ListPattern::InitOnKeyEvent(const RefPtr<FocusHub>& focusHub)
{
    auto onKeyEvent = [wp = WeakClaim(this)](const KeyEvent& event) -> bool {
        auto pattern = wp.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->OnKeyEvent(event);
    };
    focusHub->SetOnKeyEventInternal(std::move(onKeyEvent));
}

bool ListPattern::OnKeyEvent(const KeyEvent& event)
{
    if (event.action != KeyAction::DOWN) {
        return false;
    }
    if (event.code == KeyCode::KEY_PAGE_DOWN) {
        ScrollPage(false);
        return true;
    }
    if (event.code == KeyCode::KEY_PAGE_UP) {
        ScrollPage(true);
        return true;
    }
    return HandleDirectionKey(event);
}

bool ListPattern::HandleDirectionKey(const KeyEvent& event)
{
    return false;
}

WeakPtr<FocusHub> ListPattern::GetNextFocusNode(FocusStep step, const WeakPtr<FocusHub>& currentFocusNode)
{
    auto curFocus = currentFocusNode.Upgrade();
    CHECK_NULL_RETURN(curFocus, nullptr);
    auto curFrame = curFocus->GetFrameNode();
    CHECK_NULL_RETURN(curFrame, nullptr);
    auto curPattern = curFrame->GetPattern();
    CHECK_NULL_RETURN(curPattern, nullptr);
    auto curItemPattern = AceType::DynamicCast<ListItemPattern>(curPattern);
    CHECK_NULL_RETURN(curItemPattern, nullptr);
    auto listProperty = GetLayoutProperty<ListLayoutProperty>();
    CHECK_NULL_RETURN(listProperty, nullptr);

    auto isVertical = listProperty->GetListDirection().value_or(Axis::VERTICAL) == Axis::VERTICAL;
    auto curIndex = curItemPattern->GetIndexInList();
    auto curIndexInGroup = curItemPattern->GetIndexInListItemGroup();
    auto curListItemGroupPara = GetListItemGroupParameter(curFrame);
    if (curIndex < 0 || curIndex > maxListItemIndex_) {
        return nullptr;
    }

    auto moveStep = 0;
    auto nextIndex = curIndex;
    auto nextIndexInGroup = curIndexInGroup;
    if (lanes_ <= 1) {
        if ((isVertical && step == FocusStep::UP_END) || (!isVertical && step == FocusStep::LEFT_END)) {
            moveStep = 1;
            nextIndex = 0;
            nextIndexInGroup = -1;
        } else if ((isVertical && step == FocusStep::DOWN_END) || (!isVertical && step == FocusStep::RIGHT_END)) {
            moveStep = -1;
            nextIndex = maxListItemIndex_;
            nextIndexInGroup = -1;
        } else if ((isVertical && (step == FocusStep::DOWN)) || (!isVertical && step == FocusStep::RIGHT) ||
                   (step == FocusStep::TAB)) {
            moveStep = 1;
            if ((curIndexInGroup == -1) || (curIndexInGroup >= curListItemGroupPara.itemEndIndex)) {
                nextIndex = curIndex + moveStep;
                nextIndexInGroup = -1;
            } else {
                nextIndexInGroup = curIndexInGroup + moveStep;
            }
        } else if ((isVertical && step == FocusStep::UP) || (!isVertical && step == FocusStep::LEFT) ||
                   (step == FocusStep::SHIFT_TAB)) {
            moveStep = -1;
            if ((curIndexInGroup == -1) || (curIndexInGroup <= 0)) {
                nextIndex = curIndex + moveStep;
                nextIndexInGroup = -1;
            } else {
                nextIndexInGroup = curIndexInGroup + moveStep;
            }
        }
    } else {
        if ((step == FocusStep::UP_END) || (step == FocusStep::LEFT_END)) {
            moveStep = 1;
            nextIndex = 0;
            nextIndexInGroup = -1;
        } else if ((step == FocusStep::DOWN_END) || (step == FocusStep::RIGHT_END)) {
            moveStep = -1;
            nextIndex = maxListItemIndex_;
            nextIndexInGroup = -1;
        } else if ((isVertical && (step == FocusStep::DOWN)) || (!isVertical && step == FocusStep::RIGHT)) {
            if (curIndexInGroup == -1) {
                moveStep = lanes_;
                nextIndex = curIndex + moveStep;
                nextIndexInGroup = -1;
            } else {
                moveStep = curListItemGroupPara.lanes;
                nextIndexInGroup = curIndexInGroup + moveStep;
            }
        } else if ((isVertical && step == FocusStep::UP) || (!isVertical && step == FocusStep::LEFT)) {
            if (curIndexInGroup == -1) {
                moveStep = -lanes_;
                nextIndex = curIndex + moveStep;
                nextIndexInGroup = -1;
            } else {
                moveStep = -curListItemGroupPara.lanes;
                nextIndexInGroup = curIndexInGroup + moveStep;
            }
        } else if ((isVertical && (step == FocusStep::RIGHT)) || (!isVertical && step == FocusStep::DOWN)) {
            moveStep = 1;
            if (((curIndexInGroup == -1) && ((curIndex - (lanes_ - 1)) % lanes_ != 0)) ||
                ((curIndexInGroup != -1) &&
                    ((curIndexInGroup - (curListItemGroupPara.lanes - 1)) % curListItemGroupPara.lanes == 0))) {
                nextIndex = curIndex + moveStep;
                nextIndexInGroup = -1;
            } else if ((curIndexInGroup != -1) &&
                       ((curIndexInGroup - (curListItemGroupPara.lanes - 1)) % curListItemGroupPara.lanes != 0)) {
                nextIndexInGroup = curIndexInGroup + moveStep;
            }
        } else if ((isVertical && step == FocusStep::LEFT) || (!isVertical && step == FocusStep::UP)) {
            moveStep = -1;
            if (((curIndexInGroup == -1) && (curIndex % lanes_ != 0)) ||
                ((curIndexInGroup != -1) && (curIndexInGroup % curListItemGroupPara.lanes == 0))) {
                nextIndex = curIndex + moveStep;
                nextIndexInGroup = -1;
            } else if ((curIndexInGroup != -1) && (curIndexInGroup % curListItemGroupPara.lanes != 0)) {
                nextIndexInGroup = curIndexInGroup + moveStep;
            }
        } else if (step == FocusStep::TAB) {
            moveStep = 1;
            if ((curIndexInGroup == -1) || (curIndexInGroup >= curListItemGroupPara.itemEndIndex)) {
                nextIndex = curIndex + moveStep;
                nextIndexInGroup = -1;
            } else {
                nextIndexInGroup = curIndexInGroup + moveStep;
            }
        } else if (step == FocusStep::SHIFT_TAB) {
            moveStep = -1;
            if ((curIndexInGroup == -1) || (curIndexInGroup <= 0)) {
                nextIndex = curIndex + moveStep;
                nextIndexInGroup = -1;
            } else {
                nextIndexInGroup = curIndexInGroup + moveStep;
            }
        }
    }
    while (nextIndex >= 0 && nextIndex <= maxListItemIndex_) {
        if ((nextIndex == curIndex) && (curIndexInGroup == nextIndexInGroup)) {
            return nullptr;
        }
        auto nextFocusNode =
            ScrollAndFindFocusNode(nextIndex, curIndex, nextIndexInGroup, curIndexInGroup, moveStep, step);
        if (nextFocusNode.Upgrade()) {
            return nextFocusNode;
        }
        if (nextIndexInGroup > -1) {
            nextIndexInGroup += moveStep;
        } else {
            nextIndex += moveStep;
        }
    }
    return nullptr;
}

WeakPtr<FocusHub> ListPattern::GetChildFocusNodeByIndex(int32_t tarMainIndex, int32_t tarGroupIndex)
{
    auto listFrame = GetHost();
    CHECK_NULL_RETURN(listFrame, nullptr);
    auto listFocus = listFrame->GetFocusHub();
    CHECK_NULL_RETURN(listFocus, nullptr);
    auto childFocusList = listFocus->GetChildren();
    for (const auto& childFocus : childFocusList) {
        if (!childFocus->IsFocusable()) {
            continue;
        }
        auto childFrame = childFocus->GetFrameNode();
        if (!childFrame) {
            continue;
        }
        auto childPattern = childFrame->GetPattern();
        if (!childPattern) {
            continue;
        }
        auto childItemPattern = AceType::DynamicCast<ListItemPattern>(childPattern);
        if (!childItemPattern) {
            continue;
        }
        auto curIndex = childItemPattern->GetIndexInList();
        auto curIndexInGroup = childItemPattern->GetIndexInListItemGroup();
        if (curIndex == tarMainIndex && curIndexInGroup == tarGroupIndex) {
            return AceType::WeakClaim(AceType::RawPtr(childFocus));
        }
    }
    return nullptr;
}

bool ListPattern::ScrollToNode(const RefPtr<FrameNode>& focusFrameNode)
{
    CHECK_NULL_RETURN(focusFrameNode, false);
    auto focusPattern = focusFrameNode->GetPattern<ListItemPattern>();
    CHECK_NULL_RETURN(focusPattern, false);
    auto curIndex = focusPattern->GetIndexInList();
    ScrollToIndex(curIndex, smooth_, ScrollAlign::AUTO);
    auto pipeline = PipelineContext::GetCurrentContext();
    if (pipeline) {
        pipeline->FlushUITasks();
    }
    return true;
}

std::pair<std::function<bool(float)>, Axis> ListPattern::GetScrollOffsetAbility()
{
    return { [wp = WeakClaim(this)](float moveOffset) -> bool {
                auto pattern = wp.Upgrade();
                CHECK_NULL_RETURN(pattern, false);
                pattern->ScrollBy(-moveOffset);
                return true;
            },
        GetAxis() };
}

std::function<bool(int32_t)> ListPattern::GetScrollIndexAbility()
{
    return [wp = WeakClaim(this)](int32_t index) -> bool {
        auto pattern = wp.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        if (index == FocusHub::SCROLL_TO_HEAD) {
            pattern->ScrollToIndex(0, false, ScrollAlign::START);
        } else if (index == FocusHub::SCROLL_TO_TAIL) {
            pattern->ScrollToIndex(ListLayoutAlgorithm::LAST_ITEM, false, ScrollAlign::END);
        } else {
            pattern->ScrollToIndex(index, false, ScrollAlign::AUTO);
        }
        return true;
    };
}

WeakPtr<FocusHub> ListPattern::ScrollAndFindFocusNode(int32_t nextIndex, int32_t curIndex, int32_t& nextIndexInGroup,
    int32_t curIndexInGroup, int32_t moveStep, FocusStep step)
{
    auto isScrollIndex = ScrollListForFocus(nextIndex, curIndex, nextIndexInGroup);
    auto groupIndexInGroup =
        ScrollListItemGroupForFocus(nextIndex, nextIndexInGroup, curIndexInGroup, moveStep, step, isScrollIndex);

    return groupIndexInGroup ? GetChildFocusNodeByIndex(nextIndex, nextIndexInGroup) : nullptr;
}

bool ListPattern::ScrollListForFocus(int32_t nextIndex, int32_t curIndex, int32_t nextIndexInGroup)
{
    auto isScrollIndex = false;
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, isScrollIndex);
    if (nextIndex < startIndex_) {
        if (nextIndexInGroup == -1) {
            isScrollIndex = true;
            ScrollToIndex(nextIndex, smooth_, ScrollAlign::START);
            pipeline->FlushUITasks();
        } else {
            ScrollToIndex(nextIndex, nextIndexInGroup, ScrollAlign::START);
            pipeline->FlushUITasks();
        }
    } else if (nextIndex > endIndex_) {
        if (nextIndexInGroup == -1) {
            isScrollIndex = true;
            ScrollToIndex(nextIndex, smooth_, ScrollAlign::END);
            pipeline->FlushUITasks();
        } else {
            ScrollToIndex(nextIndex, nextIndexInGroup, ScrollAlign::END);
            pipeline->FlushUITasks();
        }
    }
    return isScrollIndex;
}

bool ListPattern::ScrollListItemGroupForFocus(int32_t nextIndex, int32_t& nextIndexInGroup, int32_t curIndexInGroup,
    int32_t moveStep, FocusStep step, bool isScrollIndex)
{
    auto groupIndexInGroup = true;
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, groupIndexInGroup);
    RefPtr<FrameNode> nextIndexNode;
    auto isNextInGroup = IsListItemGroup(nextIndex, nextIndexNode);
    CHECK_NULL_RETURN(nextIndexNode, groupIndexInGroup);
    if (!isNextInGroup) {
        nextIndexInGroup = -1;
        return groupIndexInGroup;
    }
    auto nextListItemGroupPara = GetListItemGroupParameter(nextIndexNode);
    if (nextIndexInGroup == -1) {
        auto scrollAlign = ScrollAlign::END;
        nextIndexInGroup = moveStep < 0 ? nextListItemGroupPara.itemEndIndex : 0;
        if ((step == FocusStep::UP_END) || (step == FocusStep::LEFT_END) || (step == FocusStep::DOWN_END) ||
            (step == FocusStep::RIGHT_END)) {
            scrollAlign = moveStep < 0 ? ScrollAlign::END : ScrollAlign::START;
        } else {
            scrollAlign = moveStep < 0 ? ScrollAlign::START : ScrollAlign::END;
        }
        if ((nextIndexInGroup < nextListItemGroupPara.displayStartIndex) ||
            (nextIndexInGroup > nextListItemGroupPara.displayEndIndex) || (isScrollIndex)) {
            ScrollToIndex(nextIndex, nextIndexInGroup, scrollAlign);
            pipeline->FlushUITasks();
        }
    } else if (nextIndexInGroup > nextListItemGroupPara.itemEndIndex) {
        nextIndexInGroup = -1;
        groupIndexInGroup = false;
    } else {
        if ((nextIndexInGroup < curIndexInGroup) && (nextIndexInGroup < nextListItemGroupPara.displayStartIndex)) {
            ScrollToIndex(nextIndex, nextIndexInGroup, ScrollAlign::START);
            pipeline->FlushUITasks();
        } else if ((nextIndexInGroup > curIndexInGroup) && (nextIndexInGroup > nextListItemGroupPara.displayEndIndex)) {
            ScrollToIndex(nextIndex, nextIndexInGroup, ScrollAlign::END);
            pipeline->FlushUITasks();
        }
    }
    return groupIndexInGroup;
}

void ListPattern::OnAnimateStop()
{
    scrollStop_ = true;
    MarkDirtyNodeSelf();
    isScrollEnd_ = true;
}

void ListPattern::ScrollTo(float position)
{
    StopAnimate();
    jumpIndex_.reset();
    targetIndex_.reset();
    currentDelta_ = 0.0f;
    UpdateCurrentOffset(GetTotalOffset() - position, SCROLL_FROM_JUMP);
    MarkDirtyNodeSelf();
    isScrollEnd_ = true;
}

void ListPattern::ScrollToIndex(int32_t index, bool smooth, ScrollAlign align, std::optional<float> extraOffset)
{
    SetScrollSource(SCROLL_FROM_JUMP);
    if (!smooth) {
        StopAnimate();
    }
    if (index >= 0 || index == ListLayoutAlgorithm::LAST_ITEM) {
        currentDelta_ = 0.0f;
        smooth_ = smooth;
        if (smooth_) {
            SetExtraOffset(extraOffset);
            if (!AnimateToTarget(index, std::nullopt, align)) {
                targetIndex_ = index;
                scrollAlign_ = align;
            }
        } else {
            if (extraOffset.has_value()) {
                currentDelta_ = extraOffset.value();
            }
            jumpIndex_ = index;
            scrollAlign_ = align;
        }
        MarkDirtyNodeSelf();
    }
    isScrollEnd_ = true;
    FireAndCleanScrollingListener();
}

bool ListPattern::CheckTargetValid(int32_t index, int32_t indexInGroup)
{
    auto host = GetHost();
    auto totalItemCount = host->GetTotalChildCount();
    if ((index < 0) || (index >= totalItemCount)) {
        return false;
    }
    auto groupWrapper = host->GetOrCreateChildByIndex(index);
    CHECK_NULL_RETURN(groupWrapper, false);
    if (groupWrapper->GetHostTag() != V2::LIST_ITEM_GROUP_ETS_TAG) {
        return false;
    }
    auto groupNode = groupWrapper->GetHostNode();
    CHECK_NULL_RETURN(groupNode, false);
    auto groupPattern = groupNode->GetPattern<ListItemGroupPattern>();
    CHECK_NULL_RETURN(groupPattern, false);
    auto groupItemCount = groupWrapper->GetTotalChildCount() - groupPattern->GetItemStartIndex();
    if ((indexInGroup < 0) || (indexInGroup >= groupItemCount)) {
        return false;
    }
    return true;
}

void ListPattern::ScrollToItemInGroup(int32_t index, int32_t indexInGroup, bool smooth, ScrollAlign align)
{
    SetScrollSource(SCROLL_FROM_JUMP);
    StopAnimate();
    if (index >= 0 || index == ListLayoutAlgorithm::LAST_ITEM) {
        currentDelta_ = 0.0f;
        smooth_ = smooth;
        if (smooth_) {
            if (!AnimateToTarget(index, indexInGroup, align)) {
                if (CheckTargetValid(index, indexInGroup)) {
                    targetIndex_ = index;
                    currentDelta_ = 0;
                    targetIndexInGroup_ = indexInGroup;
                    scrollAlign_ = align;
                }
            }
        } else {
            jumpIndex_ = index;
            jumpIndexInGroup_ = indexInGroup;
            scrollAlign_ = align;
        }
        MarkDirtyNodeSelf();
    }
    isScrollEnd_ = true;
    FireAndCleanScrollingListener();
}

bool ListPattern::GetListItemAnimatePos(float startPos, float endPos, ScrollAlign align, float& targetPos)
{
    switch (align) {
        case ScrollAlign::START:
        case ScrollAlign::NONE:
            targetPos = startPos;
            if (!IsScrollSnapAlignCenter() || childrenSize_) {
                targetPos -= contentStartOffset_;
            }
            break;
        case ScrollAlign::CENTER:
            targetPos = (endPos + startPos) / 2.0f - contentMainSize_ / 2.0f;
            break;
        case ScrollAlign::END:
            targetPos = endPos - contentMainSize_;
            if (!IsScrollSnapAlignCenter() || childrenSize_) {
                targetPos += contentEndOffset_;
            }
            break;
        case ScrollAlign::AUTO:
            targetPos = CalculateTargetPos(startPos, endPos);
            break;
    }
    return true;
}

bool ListPattern::GetListItemGroupAnimatePosWithoutIndexInGroup(
    int32_t index, float startPos, float endPos, ScrollAlign align, float& targetPos)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto groupWrapper = host->GetChildByIndex(index);
    CHECK_NULL_RETURN(groupWrapper, false);
    auto groupNode = groupWrapper->GetHostNode();
    CHECK_NULL_RETURN(groupNode, false);
    auto groupPattern = groupNode->GetPattern<ListItemGroupPattern>();
    CHECK_NULL_RETURN(groupPattern, false);
    auto groupLayoutProperty = groupNode->GetLayoutProperty<ListItemGroupLayoutProperty>();
    CHECK_NULL_RETURN(groupLayoutProperty, false);
    auto visible = groupLayoutProperty->GetVisibility().value_or(VisibleType::VISIBLE);

    switch (align) {
        case ScrollAlign::START:
        case ScrollAlign::NONE:
            if (visible != VisibleType::GONE && !groupPattern->IsDisplayStart()) {
                return false;
            }
            targetPos = startPos;
            if (!IsScrollSnapAlignCenter() || childrenSize_) {
                targetPos -= contentStartOffset_;
            }
            break;
        case ScrollAlign::CENTER:
            if (visible != VisibleType::GONE && (!groupPattern->IsDisplayStart() || !groupPattern->IsDisplayEnd())) {
                return false;
            }
            targetPos = (endPos + startPos) / 2.0f - contentMainSize_ / 2.0f;
            break;
        case ScrollAlign::END:
            if (visible != VisibleType::GONE && !groupPattern->IsDisplayEnd()) {
                return false;
            }
            targetPos = endPos - contentMainSize_;
            if (!IsScrollSnapAlignCenter() || childrenSize_) {
                targetPos += contentEndOffset_;
            }
            break;
        case ScrollAlign::AUTO:
            if (targetIndex_.has_value()) {
                targetPos = CalculateTargetPos(startPos, endPos);
                return true;
            }
            return false;
    }

    return true;
}

bool ListPattern::GetListItemGroupAnimatePosWithIndexInGroup(
    int32_t index, int32_t indexInGroup, float startPos, ScrollAlign align, float& targetPos)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto groupWrapper = host->GetChildByIndex(index);
    CHECK_NULL_RETURN(groupWrapper, false);
    auto groupNode = groupWrapper->GetHostNode();
    CHECK_NULL_RETURN(groupNode, false);
    auto groupPattern = groupNode->GetPattern<ListItemGroupPattern>();
    CHECK_NULL_RETURN(groupPattern, false);
    auto listLayoutProperty = host->GetLayoutProperty<ListLayoutProperty>();
    CHECK_NULL_RETURN(listLayoutProperty, false);
    auto stickyStyle = listLayoutProperty->GetStickyStyle().value_or(V2::StickyStyle::NONE);
    auto itemsPosInGroup = groupPattern->GetItemPosition();
    auto it = itemsPosInGroup.find(indexInGroup);
    if (it == itemsPosInGroup.end()) {
        return false;
    }
    auto padding = groupWrapper->GetGeometryNode()->GetPadding()->top;
    float paddingBeforeContent = padding ? padding.value() : 0.0f;
    if (align == ScrollAlign::CENTER) {
        targetPos = paddingBeforeContent + startPos + (it->second.startPos + it->second.endPos) / 2.0f -
                    contentMainSize_ / 2.0f;
    } else {
        float itemStartPos = paddingBeforeContent + startPos + it->second.startPos;
        float itemEndPos = paddingBeforeContent + startPos + it->second.endPos;
        if (stickyStyle == V2::StickyStyle::HEADER || stickyStyle == V2::StickyStyle::BOTH) {
            itemStartPos -= groupPattern->GetHeaderMainSize();
        }
        if (stickyStyle == V2::StickyStyle::FOOTER || stickyStyle == V2::StickyStyle::BOTH) {
            itemEndPos += groupPattern->GetFooterMainSize();
        }
        if (!IsScrollSnapAlignCenter() || childrenSize_) {
            itemStartPos -= contentStartOffset_;
            itemEndPos += contentEndOffset_;
        }
        if (align == ScrollAlign::AUTO) {
            targetPos = CalculateTargetPos(itemStartPos, itemEndPos);
        } else {
            targetPos = align == ScrollAlign::END ? itemEndPos - contentMainSize_ : itemStartPos;
        }
    }
    return true;
}

bool ListPattern::AnimateToTarget(int32_t index, std::optional<int32_t> indexInGroup, ScrollAlign align)
{
    auto iter = itemPosition_.find(index);
    if (iter == itemPosition_.end()) {
        return false;
    }
    float targetPos = 0.0f;
    if (iter->second.isGroup) {
        if (indexInGroup.has_value()) {
            if (!GetListItemGroupAnimatePosWithIndexInGroup(index, indexInGroup.value(), iter->second.startPos,
                align, targetPos)) {
                return false;
            }
        } else {
            if (!GetListItemGroupAnimatePosWithoutIndexInGroup(index, iter->second.startPos, iter->second.endPos,
                align, targetPos)) {
                return false;
            }
        }
    } else {
        if (indexInGroup.has_value()) {
            return false;
        }
        GetListItemAnimatePos(iter->second.startPos, iter->second.endPos, align, targetPos);
    }
    auto extraOffset = GetExtraOffset();
    if (extraOffset.has_value()) {
        SetUseTotalOffset(false);
        targetPos += extraOffset.value();
        ResetExtraOffset();
    }
    if (!NearZero(targetPos)) {
        AnimateTo(targetPos + currentOffset_, -1, nullptr, true);
        if (predictSnapOffset_.has_value() && AnimateRunning()) {
            scrollSnapVelocity_ = 0.0f;
            predictSnapOffset_.reset();
            snapTrigOnScrollStart_ = false;
        }
    }
    return true;
}

void ListPattern::ScrollPage(bool reverse, bool smooth)
{
    float distance = reverse ? contentMainSize_ : -contentMainSize_;
    if (smooth) {
        float position = -GetCurrentOffset().GetY() + distance;
        AnimateTo(-position, -1, nullptr, true);
    } else {
        StopAnimate();
        UpdateCurrentOffset(distance, SCROLL_FROM_JUMP);
        isScrollEnd_ = true;
    }
}

void ListPattern::ScrollBy(float offset)
{
    StopAnimate();
    UpdateCurrentOffset(-offset, SCROLL_FROM_JUMP);
    isScrollEnd_ = true;
}

Offset ListPattern::GetCurrentOffset() const
{
    if (GetAxis() == Axis::HORIZONTAL) {
        return { GetTotalOffset(), 0.0 };
    }
    return { 0.0, GetTotalOffset() };
}

void ListPattern::HandleScrollBarOutBoundary()
{
    if (itemPosition_.empty()) {
        return;
    }
    if (!GetScrollBar() && !GetScrollBarProxy()) {
        return;
    }
    if (!IsOutOfBoundary(false) || !isScrollable_) {
        ScrollablePattern::HandleScrollBarOutBoundary(0);
        return;
    }
    float overScroll = 0.0f;
    if (!IsScrollSnapAlignCenter()) {
        if ((itemPosition_.begin()->first == 0) && GreatNotEqual(startMainPos_, contentStartOffset_)) {
            overScroll = startMainPos_ - contentStartOffset_;
        } else {
            overScroll = contentMainSize_ - contentEndOffset_ - endMainPos_;
        }
    } else {
        float itemHeight = itemPosition_[centerIndex_].endPos - itemPosition_[centerIndex_].startPos;
        if (startIndex_ == 0 && Positive(startMainPos_ + itemHeight / 2.0f - contentMainSize_ / 2.0f)) {
            overScroll = startMainPos_ + itemHeight / 2.0f - contentMainSize_ / 2.0f;
        } else if ((endIndex_ == maxListItemIndex_) &&
                   LessNotEqual(endMainPos_ - itemHeight / 2.0f, contentMainSize_ / 2.0f)) {
            overScroll = endMainPos_ - itemHeight / 2.0f - contentMainSize_ / 2.0f;
        }
    }
    ScrollablePattern::HandleScrollBarOutBoundary(overScroll);
}

Rect ListPattern::GetItemRect(int32_t index) const
{
    if (index < 0 || index < startIndex_ || index > endIndex_) {
        return Rect();
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, Rect());
    auto item = host->GetChildByIndex(index);
    CHECK_NULL_RETURN(item, Rect());
    auto itemGeometry = item->GetGeometryNode();
    CHECK_NULL_RETURN(itemGeometry, Rect());
    return Rect(itemGeometry->GetFrameRect().GetX(), itemGeometry->GetFrameRect().GetY(),
        itemGeometry->GetFrameRect().Width(), itemGeometry->GetFrameRect().Height());
}

Rect ListPattern::GetItemRectInGroup(int32_t index, int32_t indexInGroup) const
{
    if (index < 0 || indexInGroup < 0 || index < startIndex_ || index > endIndex_) {
        return Rect();
    }
    auto host = GetHost();
    CHECK_NULL_RETURN(host, Rect());
    auto itemGroupWrapper = host->GetChildByIndex(index);
    CHECK_NULL_RETURN(itemGroupWrapper, Rect());
    auto itemGroup = itemGroupWrapper->GetHostNode();
    CHECK_NULL_RETURN(itemGroup, Rect());
    if (!(itemGroup->GetHostTag() == V2::LIST_ITEM_GROUP_ETS_TAG)) {
        return Rect();
    }
    auto itemGroupGeometry = itemGroup->GetGeometryNode();
    CHECK_NULL_RETURN(itemGroupGeometry, Rect());
    auto groupPattern = itemGroup->GetPattern<ListItemGroupPattern>();
    CHECK_NULL_RETURN(groupPattern, Rect());
    if (indexInGroup < groupPattern->GetDisplayStartIndexInGroup() ||
        indexInGroup > groupPattern->GetDisplayEndIndexInGroup()) {
        return Rect();
    }
    auto groupItem = itemGroup->GetChildByIndex(indexInGroup + groupPattern->GetItemStartIndex());
    CHECK_NULL_RETURN(groupItem, Rect());
    auto groupItemGeometry = groupItem->GetGeometryNode();
    CHECK_NULL_RETURN(groupItemGeometry, Rect());
    return Rect(itemGroupGeometry->GetFrameRect().GetX() + groupItemGeometry->GetFrameRect().GetX(),
        itemGroupGeometry->GetFrameRect().GetY() + groupItemGeometry->GetFrameRect().GetY(),
        groupItemGeometry->GetFrameRect().Width(), groupItemGeometry->GetFrameRect().Height());
}

void ListPattern::CalculateCurrentOffset(float delta)
{
    if (itemPosition_.empty()) {
        return;
    }
    posMap_->UpdatePosMapStart(
        delta, currentOffset_, spaceWidth_, itemPosition_.begin()->first, itemPosition_.begin()->second.startPos);
    for (auto& [index, pos] : itemPosition_) {
        float height = pos.endPos - pos.startPos;
        posMap_->UpdatePos(index, { currentOffset_ + pos.startPos, height });
    }
    posMap_->UpdatePosMapEnd(itemPosition_.rbegin()->first, spaceWidth_);
}

void ListPattern::UpdateScrollBarOffset()
{
    if (itemPosition_.empty()) {
        return;
    }
    if (!GetScrollBar() && !GetScrollBarProxy()) {
        return;
    }
    float currentOffset = 0.0f;
    float estimatedHeight = 0.0f;
    if (childrenSize_) {
        currentOffset = currentOffset_;
        estimatedHeight = listTotalHeight_;
    } else {
        auto calculate = ListHeightOffsetCalculator(itemPosition_, spaceWidth_, lanes_, GetAxis());
        calculate.GetEstimateHeightAndOffset(GetHost());
        currentOffset = calculate.GetEstimateOffset();
        estimatedHeight = calculate.GetEstimateHeight();
    }
    if (GetAlwaysEnabled()) {
        estimatedHeight = estimatedHeight - spaceWidth_;
    }
    if (!IsScrollSnapAlignCenter() || childrenSize_) {
        currentOffset += contentStartOffset_;
        estimatedHeight += contentStartOffset_ + contentEndOffset_;
    }

    // calculate padding offset of list
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutPriority = host->GetLayoutProperty();
    CHECK_NULL_VOID(layoutPriority);
    auto padding = layoutPriority->CreatePaddingAndBorder();
    auto paddingMain = GetAxis() == Axis::VERTICAL ? padding.Height() : padding.Width();
    const auto& geometryNode = host->GetGeometryNode();
    auto frameSize = geometryNode->GetFrameSize();
    Size size(frameSize.Width(), frameSize.Height());
    UpdateScrollBarRegion(currentOffset, estimatedHeight + paddingMain, size, Offset(0.0f, 0.0f));
}

float ListPattern::GetTotalHeight() const
{
    auto currentOffset = GetTotalOffset();
    if (endIndex_ >= maxListItemIndex_) {
        return currentOffset + endMainPos_ + contentEndOffset_;
    }
    if (itemPosition_.empty()) {
        return 0.0f;
    }
    int32_t remainCount = maxListItemIndex_ - endIndex_;
    float itemsSize = itemPosition_.rbegin()->second.endPos - itemPosition_.begin()->second.startPos + spaceWidth_;
    float remainOffset = itemsSize / itemPosition_.size() * remainCount - spaceWidth_;
    return currentOffset + endMainPos_ + remainOffset + contentEndOffset_;
}

void ListPattern::TriggerModifyDone()
{
    OnModifyDone();
}

void ListPattern::SetChainAnimation()
{
    auto listLayoutProperty = GetLayoutProperty<ListLayoutProperty>();
    CHECK_NULL_VOID(listLayoutProperty);
    auto edgeEffect = GetEdgeEffect();
    int32_t lanes = std::max(listLayoutProperty->GetLanes().value_or(1), 1);
    bool autoLanes = listLayoutProperty->HasLaneMinLength() || listLayoutProperty->HasLaneMaxLength();
    bool animation = listLayoutProperty->GetChainAnimation().value_or(false);
    bool enable = edgeEffect == EdgeEffect::SPRING && lanes == 1 && !autoLanes && animation;
    if (!enable) {
        chainAnimation_.Reset();
        return;
    }
    auto space = listLayoutProperty->GetSpace().value_or(CHAIN_INTERVAL_DEFAULT).ConvertToPx();
    if (Negative(space)) {
        space = CHAIN_INTERVAL_DEFAULT.ConvertToPx();
    }
    if (!chainAnimation_ || (chainAnimation_ && space != chainAnimation_->GetSpace())) {
        springProperty_ =
            AceType::MakeRefPtr<SpringProperty>(CHAIN_SPRING_MASS, CHAIN_SPRING_STIFFNESS, CHAIN_SPRING_DAMPING);
        if (chainAnimationOptions_.has_value()) {
            float maxSpace = chainAnimationOptions_.value().maxSpace.ConvertToPx();
            float minSpace = chainAnimationOptions_.value().minSpace.ConvertToPx();
            minSpace = Negative(minSpace) ? 0.0f : minSpace;
            minSpace = GreatNotEqual(minSpace, space) ? space : minSpace;
            maxSpace = LessNotEqual(maxSpace, space) ? space : maxSpace;
            springProperty_->SetStiffness(chainAnimationOptions_.value().stiffness);
            springProperty_->SetDamping(chainAnimationOptions_.value().damping);
            chainAnimation_ = AceType::MakeRefPtr<ChainAnimation>(space, maxSpace, minSpace, springProperty_);
            auto conductivity = chainAnimationOptions_.value().conductivity;
            if (LessNotEqual(conductivity, 0) || GreatNotEqual(conductivity, 1)) {
                conductivity = ChainAnimation::DEFAULT_CONDUCTIVITY;
            }
            chainAnimation_->SetConductivity(conductivity);
            auto intensity = chainAnimationOptions_.value().intensity;
            if (LessNotEqual(intensity, 0) || GreatNotEqual(intensity, 1)) {
                intensity = ChainAnimation::DEFAULT_INTENSITY;
            }
            chainAnimation_->SetIntensity(intensity);
            auto effect = chainAnimationOptions_.value().edgeEffect;
            chainAnimation_->SetEdgeEffect(effect == 1 ? ChainEdgeEffect::STRETCH : ChainEdgeEffect::DEFAULT);
        } else {
            auto minSpace = space * DEFAULT_MIN_SPACE_SCALE;
            auto maxSpace = space * DEFAULT_MAX_SPACE_SCALE;
            chainAnimation_ = AceType::MakeRefPtr<ChainAnimation>(space, maxSpace, minSpace, springProperty_);
        }
        chainAnimation_->SetAnimationCallback([weak = AceType::WeakClaim(this)]() {
            auto list = weak.Upgrade();
            CHECK_NULL_VOID(list);
            list->MarkDirtyNodeSelf();
        });
    }
}

void ListPattern::SetChainAnimationOptions(const ChainAnimationOptions& options)
{
    chainAnimationOptions_ = options;
    if (chainAnimation_) {
        auto listLayoutProperty = GetLayoutProperty<ListLayoutProperty>();
        CHECK_NULL_VOID(listLayoutProperty);
        auto space = listLayoutProperty->GetSpace().value_or(CHAIN_INTERVAL_DEFAULT).ConvertToPx();
        if (Negative(space)) {
            space = CHAIN_INTERVAL_DEFAULT.ConvertToPx();
        }
        float maxSpace = options.maxSpace.ConvertToPx();
        float minSpace = options.minSpace.ConvertToPx();
        minSpace = Negative(minSpace) ? 0.0f : minSpace;
        minSpace = GreatNotEqual(minSpace, space) ? space : minSpace;
        maxSpace = LessNotEqual(maxSpace, space) ? space : maxSpace;
        chainAnimation_->SetSpace(space, maxSpace, minSpace);
        auto conductivity = chainAnimationOptions_.value().conductivity;
        if (LessNotEqual(conductivity, 0) || GreatNotEqual(conductivity, 1)) {
            conductivity = ChainAnimation::DEFAULT_CONDUCTIVITY;
        }
        chainAnimation_->SetConductivity(conductivity);
        auto intensity = chainAnimationOptions_.value().intensity;
        if (LessNotEqual(intensity, 0) || GreatNotEqual(intensity, 1)) {
            intensity = ChainAnimation::DEFAULT_INTENSITY;
        }
        chainAnimation_->SetIntensity(intensity);
        auto effect = options.edgeEffect;
        chainAnimation_->SetEdgeEffect(effect == 1 ? ChainEdgeEffect::STRETCH : ChainEdgeEffect::DEFAULT);
    }
    if (springProperty_) {
        springProperty_->SetStiffness(chainAnimationOptions_.value().stiffness);
        springProperty_->SetDamping(chainAnimationOptions_.value().damping);
    }
}

void ListPattern::ProcessDragStart(float startPosition)
{
    CHECK_NULL_VOID(chainAnimation_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto globalOffset = host->GetTransformRelativeOffset();
    int32_t index = -1;
    auto offset = startPosition - GetMainAxisOffset(globalOffset, GetAxis());
    auto it = std::find_if(
        itemPosition_.begin(), itemPosition_.end(), [offset](auto pos) { return offset <= pos.second.endPos; });
    if (it != itemPosition_.end()) {
        index = it->first;
    } else if (!itemPosition_.empty()) {
        index = itemPosition_.rbegin()->first + 1;
    }
    dragFromSpring_ = false;
    chainAnimation_->SetControlIndex(index);
    chainAnimation_->SetMaxIndex(maxListItemIndex_);
}

void ListPattern::ProcessDragUpdate(float dragOffset, int32_t source)
{
    CHECK_NULL_VOID(chainAnimation_);
    if (NearZero(dragOffset) || source == SCROLL_FROM_BAR || source == SCROLL_FROM_AXIS ||
        source == SCROLL_FROM_BAR_FLING) {
        return;
    }
    if (NeedScrollSnapAlignEffect()) {
        auto delta = 0.0f;
        if (chainAnimation_->GetControlIndex() < startIndex_ - 1) {
            delta = chainAnimation_->SetControlIndex(std::max(startIndex_ - 1, 0));
        }
        if (chainAnimation_->GetControlIndex() > endIndex_ + 1) {
            delta = chainAnimation_->SetControlIndex(std::min(endIndex_ + 1, maxListItemIndex_));
        }
        if (!NearZero(delta)) {
            auto scrollableEvent = GetScrollableEvent();
            CHECK_NULL_VOID(scrollableEvent);
            auto scrollable = scrollableEvent->GetScrollable();
            CHECK_NULL_VOID(scrollable);
            scrollable->UpdateScrollSnapStartOffset(delta);
            currentDelta_ -= delta;
        }
    }
    bool overDrag = (source == SCROLL_FROM_UPDATE) && (IsAtTop() || IsAtBottom());
    chainAnimation_->SetDelta(-dragOffset, overDrag);
}

float ListPattern::GetChainDelta(int32_t index) const
{
    CHECK_NULL_RETURN(chainAnimation_, 0.0f);
    return chainAnimation_->GetValue(index);
}

void ListPattern::MultiSelectWithoutKeyboard(const RectF& selectedZone)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    std::list<RefPtr<FrameNode>> childrens;
    host->GenerateOneDepthVisibleFrame(childrens);
    for (const auto& item : childrens) {
        if (item->GetTag() == V2::LIST_ITEM_GROUP_ETS_TAG) {
            auto itemGroupPattern = item->GetPattern<ListItemGroupPattern>();
            CHECK_NULL_VOID(itemGroupPattern);
            auto itemGroupGeometry = item->GetGeometryNode();
            CHECK_NULL_VOID(itemGroupGeometry);
            auto itemGroupRect = itemGroupGeometry->GetFrameRect();
            if (!selectedZone.IsIntersectWith(itemGroupRect)) {
                continue;
            }
            HandleCardModeSelectedEvent(selectedZone, item, itemGroupRect.Top());
            continue;
        }
        auto itemPattern = item->GetPattern<ListItemPattern>();
        CHECK_NULL_VOID(itemPattern);
        if (!itemPattern->Selectable()) {
            continue;
        }

        auto itemGeometry = item->GetGeometryNode();
        CHECK_NULL_VOID(itemGeometry);

        auto itemRect = itemGeometry->GetFrameRect();
        if (!selectedZone.IsIntersectWith(itemRect)) {
            itemPattern->MarkIsSelected(false);
        } else {
            itemPattern->MarkIsSelected(true);
        }
    }

    DrawSelectedZone(selectedZone);
}

void ListPattern::HandleCardModeSelectedEvent(
    const RectF& selectedZone, const RefPtr<FrameNode>& itemGroupNode, float itemGroupTop)
{
    CHECK_NULL_VOID(itemGroupNode);
    std::list<RefPtr<FrameNode>> childrens;
    itemGroupNode->GenerateOneDepthVisibleFrame(childrens);
    for (const auto& item : childrens) {
        auto itemPattern = item->GetPattern<ListItemPattern>();
        if (!itemPattern) {
            continue;
        }
        if (!itemPattern->Selectable()) {
            continue;
        }
        auto itemGeometry = item->GetGeometryNode();
        CHECK_NULL_VOID(itemGeometry);
        auto context = item->GetRenderContext();
        CHECK_NULL_VOID(context);
        auto itemRect = itemGeometry->GetFrameRect();
        RectF itemRectInGroup(itemRect.GetX(), itemRect.GetY() + itemGroupTop, itemRect.Width(), itemRect.Height());
        if (!selectedZone.IsIntersectWith(itemRectInGroup)) {
            itemPattern->MarkIsSelected(false);
        } else {
            itemPattern->MarkIsSelected(true);
        }
    }
}

void ListPattern::ClearMultiSelect()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    std::list<RefPtr<FrameNode>> children;
    host->GenerateOneDepthAllFrame(children);
    for (const auto& child : children) {
        if (!child) {
            continue;
        }
        auto itemPattern = child->GetPattern<ListItemPattern>();
        if (itemPattern) {
            itemPattern->MarkIsSelected(false);
            continue;
        }
        auto itemGroupPattern = child->GetPattern<ListItemGroupPattern>();
        if (itemGroupPattern) {
            std::list<RefPtr<FrameNode>> itemChildren;
            child->GenerateOneDepthAllFrame(itemChildren);
            for (const auto& item : itemChildren) {
                if (!item) {
                    continue;
                }
                itemPattern = item->GetPattern<ListItemPattern>();
                if (itemPattern) {
                    itemPattern->MarkIsSelected(false);
                }
            }
        }
    }

    ClearSelectedZone();
}

bool ListPattern::IsItemSelected(const GestureEvent& info)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto node = host->FindChildByPosition(info.GetGlobalLocation().GetX(), info.GetGlobalLocation().GetY());
    CHECK_NULL_RETURN(node, false);
    auto itemPattern = node->GetPattern<ListItemPattern>();
    if (itemPattern) {
        return itemPattern->IsSelected();
    }
    auto itemGroupPattern = node->GetPattern<ListItemGroupPattern>();
    if (itemGroupPattern) {
        auto itemNode = node->FindChildByPosition(info.GetGlobalLocation().GetX(), info.GetGlobalLocation().GetY());
        CHECK_NULL_RETURN(itemNode, false);
        itemPattern = itemNode->GetPattern<ListItemPattern>();
        CHECK_NULL_RETURN(itemPattern, false);
        return itemPattern->IsSelected();
    }
    return false;
}

void ListPattern::SetSwiperItem(WeakPtr<ListItemPattern> swiperItem)
{
    // swiper item only can be replaced when no other items be dragged
    if (canReplaceSwiperItem_) {
        if (swiperItem != swiperItem_) {
            auto item = swiperItem_.Upgrade();
            if (item) {
                item->SwiperReset();
            }
            swiperItem_ = std::move(swiperItem);
        }
        canReplaceSwiperItem_ = false;
    }
    FireAndCleanScrollingListener();
}

int32_t ListPattern::GetItemIndexByPosition(float xOffset, float yOffset)
{
    auto host = GetHost();
    auto globalOffset = host->GetTransformRelativeOffset();
    float relativeX = xOffset - globalOffset.GetX();
    float relativeY = yOffset - globalOffset.GetY();
    float mainOffset = GetAxis() == Axis::VERTICAL ? relativeY : relativeX;
    float crossOffset = GetAxis() == Axis::VERTICAL ? relativeX : relativeY;
    float crossSize = GetCrossAxisSize(GetContentSize(), GetAxis());
    int32_t lanesOffset = 0;
    if (lanes_ > 1) {
        lanesOffset = static_cast<int32_t>(crossOffset / (crossSize / lanes_));
    }
    for (auto& pos : itemPosition_) {
        if (mainOffset <= pos.second.endPos + spaceWidth_ / 2) { /* 2:half */
            return std::min(pos.first + lanesOffset, maxListItemIndex_ + 1);
        }
    }
    if (!itemPosition_.empty()) {
        return itemPosition_.rbegin()->first + 1;
    }
    return 0;
}

void ListPattern::ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const
{
    ScrollablePattern::ToJsonValue(json, filter);
    json->PutExtAttr("multiSelectable", multiSelectable_, filter);
    json->PutExtAttr("startIndex", startIndex_, filter);
    if (!itemPosition_.empty()) {
        json->PutExtAttr("itemStartPos", itemPosition_.begin()->second.startPos, filter);
    }
}

void ListPattern::FromJson(const std::unique_ptr<JsonValue>& json)
{
    ScrollToIndex(json->GetInt("startIndex"));
    if (json->Contains("itemStartPos")) {
        ScrollBy(-json->GetDouble("itemStartPos"));
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->UpdateClipEdge(true);
    ScrollablePattern::FromJson(json);
}

void ListPattern::SetAccessibilityAction()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto accessibilityProperty = host->GetAccessibilityProperty<AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    accessibilityProperty->SetActionScrollForward([weakPtr = WeakClaim(this)]() {
        const auto& pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (!pattern->IsScrollable()) {
            return;
        }
        pattern->ScrollPage(false);
    });

    accessibilityProperty->SetActionScrollBackward([weakPtr = WeakClaim(this)]() {
        const auto& pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (!pattern->IsScrollable()) {
            return;
        }
        pattern->ScrollPage(true);
    });
}

ListItemGroupPara ListPattern::GetListItemGroupParameter(const RefPtr<FrameNode>& node)
{
    ListItemGroupPara listItemGroupPara = { -1, -1, -1, -1 };
    auto curFrameParent = node->GetParent();
    auto curFrameParentNode = AceType::DynamicCast<FrameNode>(curFrameParent);
    while (curFrameParent && (!curFrameParentNode)) {
        curFrameParent = curFrameParent->GetParent();
        curFrameParentNode = AceType::DynamicCast<FrameNode>(curFrameParent);
    }
    CHECK_NULL_RETURN(curFrameParentNode, listItemGroupPara);
    if (curFrameParent->GetTag() == V2::LIST_ITEM_GROUP_ETS_TAG) {
        auto itemGroupPattern = curFrameParentNode->GetPattern<ListItemGroupPattern>();
        CHECK_NULL_RETURN(itemGroupPattern, listItemGroupPara);
        listItemGroupPara.displayEndIndex = itemGroupPattern->GetDisplayEndIndexInGroup();
        listItemGroupPara.displayStartIndex = itemGroupPattern->GetDisplayStartIndexInGroup();
        listItemGroupPara.lanes = itemGroupPattern->GetLanesInGroup();
        listItemGroupPara.itemEndIndex = itemGroupPattern->GetEndIndexInGroup();
    }
    return listItemGroupPara;
}

bool ListPattern::IsListItemGroup(int32_t listIndex, RefPtr<FrameNode>& node)
{
    auto listFrame = GetHost();
    CHECK_NULL_RETURN(listFrame, false);
    auto listFocus = listFrame->GetFocusHub();
    CHECK_NULL_RETURN(listFocus, false);
    for (const auto& childFocus : listFocus->GetChildren()) {
        if (!childFocus->IsFocusable()) {
            continue;
        }
        if (auto childFrame = childFocus->GetFrameNode()) {
            if (auto childPattern = AceType::DynamicCast<ListItemPattern>(childFrame->GetPattern())) {
                auto curIndex = childPattern->GetIndexInList();
                auto curIndexInGroup = childPattern->GetIndexInListItemGroup();
                if (curIndex == listIndex) {
                    node = childFrame;
                    return curIndexInGroup > -1;
                }
            }
        }
    }
    return false;
}

void ListPattern::RefreshLanesItemRange()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto updatePos = host->GetChildrenUpdated();
    if (updatePos == -1) {
        return;
    }
    if (updatePos == 0) {
        lanesItemRange_.clear();
        return;
    }
    for (auto it = lanesItemRange_.begin(); it != lanesItemRange_.end();) {
        if (it->second < updatePos) {
            it++;
        } else if (it->first >= updatePos) {
            lanesItemRange_.erase(it++);
        } else {
            it->second = updatePos - 1;
            it++;
        }
    }
}

std::string ListPattern::ProvideRestoreInfo()
{
    return std::to_string(startIndex_);
}

void ListPattern::CloseAllSwipeActions(OnFinishFunc&& onFinishCallback)
{
    auto item = swiperItem_.Upgrade();
    if (item) {
        return item->CloseSwipeAction(std::move(onFinishCallback));
    }
}

void ListPattern::OnRestoreInfo(const std::string& restoreInfo)
{
    jumpIndex_ = StringUtils::StringToInt(restoreInfo);
}

void ListPattern::DumpAdvanceInfo()
{
    DumpLog::GetInstance().AddDesc("maxListItemIndex:" + std::to_string(maxListItemIndex_));
    DumpLog::GetInstance().AddDesc("startIndex:" + std::to_string(startIndex_));
    DumpLog::GetInstance().AddDesc("endIndex:" + std::to_string(endIndex_));
    DumpLog::GetInstance().AddDesc("centerIndex:" + std::to_string(centerIndex_));
    DumpLog::GetInstance().AddDesc("startMainPos:" + std::to_string(startMainPos_));
    DumpLog::GetInstance().AddDesc("endMainPos:" + std::to_string(endMainPos_));
    DumpLog::GetInstance().AddDesc("currentOffset:" + std::to_string(currentOffset_));
    DumpLog::GetInstance().AddDesc("contentMainSize:" + std::to_string(contentMainSize_));
    DumpLog::GetInstance().AddDesc("contentStartOffset:" + std::to_string(contentStartOffset_));
    DumpLog::GetInstance().AddDesc("contentEndOffset:" + std::to_string(contentEndOffset_));
    DumpLog::GetInstance().AddDesc("currentDelta:" + std::to_string(currentDelta_));
    crossMatchChild_ ? DumpLog::GetInstance().AddDesc("crossMatchChild:true")
                     : DumpLog::GetInstance().AddDesc("crossMatchChild:false");
    smooth_ ? DumpLog::GetInstance().AddDesc("smooth:true") : DumpLog::GetInstance().AddDesc("smooth:false");
    if (jumpIndex_.has_value()) {
        DumpLog::GetInstance().AddDesc("jumpIndex:" + std::to_string(jumpIndex_.value()));
    } else {
        DumpLog::GetInstance().AddDesc("jumpIndex:null");
    }
    if (jumpIndexInGroup_.has_value()) {
        DumpLog::GetInstance().AddDesc("jumpIndexInGroup:" + std::to_string(jumpIndexInGroup_.value()));
    } else {
        DumpLog::GetInstance().AddDesc("jumpIndexInGroup:null");
    }
    if (targetIndex_.has_value()) {
        DumpLog::GetInstance().AddDesc("targetIndex:" + std::to_string(targetIndex_.value()));
    } else {
        DumpLog::GetInstance().AddDesc("targetIndex:null");
    }
    if (predictSnapOffset_.has_value()) {
        DumpLog::GetInstance().AddDesc("predictSnapOffset:" + std::to_string(predictSnapOffset_.value()));
    } else {
        DumpLog::GetInstance().AddDesc("predictSnapOffset:null");
    }
    if (predictSnapEndPos_.has_value()) {
        DumpLog::GetInstance().AddDesc("predictSnapEndPos:" + std::to_string(predictSnapEndPos_.value()));
    } else {
        DumpLog::GetInstance().AddDesc("predictSnapEndPos:null");
    }
    // DumpLog::GetInstance().AddDesc("scrollAlign:%{public}d", scrollAlign_);
    isScrollable_ ? DumpLog::GetInstance().AddDesc("isScrollable:true")
                  : DumpLog::GetInstance().AddDesc("isScrollable:false");
    paintStateFlag_ ? DumpLog::GetInstance().AddDesc("paintStateFlag:true")
                    : DumpLog::GetInstance().AddDesc("paintStateFlag:false");
    isFramePaintStateValid_ ? DumpLog::GetInstance().AddDesc("isFramePaintStateValid:true")
                            : DumpLog::GetInstance().AddDesc("isFramePaintStateValid:false");
    for (auto item : itemPosition_) {
        DumpLog::GetInstance().AddDesc("------------------------------------------");
        DumpLog::GetInstance().AddDesc("itemPosition.first:" + std::to_string(item.first));
        DumpLog::GetInstance().AddDesc("startPos:" + std::to_string(item.second.startPos));
        DumpLog::GetInstance().AddDesc("endPos:" + std::to_string(item.second.endPos));
        DumpLog::GetInstance().AddDesc("isGroup:" + std::to_string(item.second.isGroup));
    }
    DumpLog::GetInstance().AddDesc("------------------------------------------");
    scrollStop_ ? DumpLog::GetInstance().AddDesc("scrollStop:true")
                : DumpLog::GetInstance().AddDesc("scrollStop:false");
    for (auto item : lanesItemRange_) {
        DumpLog::GetInstance().AddDesc("------------------------------------------");
        DumpLog::GetInstance().AddDesc("lanesItemRange.first:" + std::to_string(item.first));
        DumpLog::GetInstance().AddDesc("lanesItemRange.second:" + std::to_string(item.second));
    }
    DumpLog::GetInstance().AddDesc("------------------------------------------");
    DumpLog::GetInstance().AddDesc("lanes:" + std::to_string(lanes_));
    DumpLog::GetInstance().AddDesc("laneGutter:" + std::to_string(laneGutter_));
    dragFromSpring_ ? DumpLog::GetInstance().AddDesc("dragFromSpring:true")
                    : DumpLog::GetInstance().AddDesc("dragFromSpring:false");
    isScrollEnd_ ? DumpLog::GetInstance().AddDesc("isScrollEnd:true")
                 : DumpLog::GetInstance().AddDesc("isScrollEnd:false");
    IsAtTop() ? DumpLog::GetInstance().AddDesc("IsAtTop:true") : DumpLog::GetInstance().AddDesc("IsAtTop:false");
    IsAtBottom() ? DumpLog::GetInstance().AddDesc("IsAtBottom:true")
                 : DumpLog::GetInstance().AddDesc("IsAtBottom:false");
}

DisplayMode ListPattern::GetDefaultScrollBarDisplayMode() const
{
    auto defaultDisplayMode = DisplayMode::OFF;
    if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TEN)) {
        defaultDisplayMode = DisplayMode::AUTO;
    }
    return defaultDisplayMode;
}

std::vector<RefPtr<FrameNode>> ListPattern::GetVisibleSelectedItems()
{
    std::vector<RefPtr<FrameNode>> children;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, children);
    for (int32_t index = startIndex_; index <= endIndex_; ++index) {
        auto item = host->GetChildByIndex(index);
        if (!AceType::InstanceOf<FrameNode>(item)) {
            continue;
        }
        auto itemFrameNode = AceType::DynamicCast<FrameNode>(item);
        auto itemPattern = itemFrameNode->GetPattern<ListItemPattern>();
        if (!itemPattern) {
            continue;
        }
        if (!itemPattern->IsSelected()) {
            continue;
        }
        children.emplace_back(itemFrameNode);
    }
    return children;
}

void ListPattern::registerSlideUpdateListener(const std::shared_ptr<ISlideUpdateCallback>& listener)
{
    listenerVector_.emplace_back(listener);
}

void ListPattern::UpdateFrameSizeToWeb()
{
    for (auto listenerItem : listenerVector_) {
        if (listenerItem) {
            listenerItem->OnSlideUpdate();
        }
    }
}

RefPtr<ListChildrenMainSize> ListPattern::GetOrCreateListChildrenMainSize()
{
    if (childrenSize_) {
        return childrenSize_;
    }
    childrenSize_ = AceType::MakeRefPtr<ListChildrenMainSize>();
    auto callback = [weakPattern = WeakClaim(this)](std::tuple<int32_t, int32_t, int32_t> change, ListChangeFlag flag) {
        auto pattern = weakPattern.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        context->AddBuildFinishCallBack([weakPattern, change, flag]() {
            auto pattern = weakPattern.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->OnChildrenSizeChanged(change, flag);
        });
        context->RequestFrame();
    };
    childrenSize_->SetOnDataChange(callback);
    return childrenSize_;
}

void ListPattern::OnChildrenSizeChanged(std::tuple<int32_t, int32_t, int32_t> change, ListChangeFlag flag)
{
    if (!posMap_) {
        posMap_ = MakeRefPtr<ListPositionMap>();
    }
    posMap_->MarkDirty(flag);
    MarkDirtyNodeSelf();
}

void ListPattern::SetListChildrenMainSize(float defaultSize, const std::vector<float>& mainSize)
{
    childrenSize_ = AceType::MakeRefPtr<ListChildrenMainSize>(mainSize, defaultSize);
    OnChildrenSizeChanged({ -1, -1, -1 }, LIST_UPDATE_CHILD_SIZE);
}

void ListPattern::ResetChildrenSize()
{
    if (childrenSize_) {
        childrenSize_ = nullptr;
        MarkDirtyNodeSelf();
        OnChildrenSizeChanged({ -1, -1, -1 }, LIST_UPDATE_CHILD_SIZE);
    }
}
} // namespace OHOS::Ace::NG
