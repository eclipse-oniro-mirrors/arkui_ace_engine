/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "base/geometry/axis.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/animation/bilateral_spring_node.h"
#include "core/animation/spring_model.h"
#include "core/components/common/layout/constants.h"
#include "core/components/scroll/scroll_bar_theme.h"
#include "core/components/scroll/scrollable.h"
#include "core/components_ng/pattern/list/list_item_pattern.h"
#include "core/components_ng/pattern/list/list_lanes_layout_algorithm.h"
#include "core/components_ng/pattern/list/list_layout_algorithm.h"
#include "core/components_ng/pattern/list/list_layout_property.h"
#include "core/components_ng/pattern/scroll/effect/scroll_fade_effect.h"
#include "core/components_ng/pattern/scroll/scroll_spring_effect.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/property/property.h"

namespace OHOS::Ace::NG {
namespace {
constexpr Dimension CHAIN_INTERVAL_DEFAULT = 20.0_vp;
constexpr double CHAIN_SPRING_MASS = 1.0;
constexpr double CHAIN_SPRING_DAMPING = 30.0;
constexpr double CHAIN_SPRING_STIFFNESS = 228;
constexpr Color SELECT_FILL_COLOR = Color(0x1A000000);
constexpr Color SELECT_STROKE_COLOR = Color(0x33FFFFFF);
constexpr float DEFAULT_MIN_SPACE_SCALE = 0.75f;
constexpr float DEFAULT_MAX_SPACE_SCALE = 2.0f;
} // namespace

void ListPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToBounds(true);
}

void ListPattern::OnModifyDone()
{
    if (!isInitialized_) {
        jumpIndex_ = GetLayoutProperty<ListLayoutProperty>()->GetInitialIndex().value_or(0);
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto listLayoutProperty = host->GetLayoutProperty<ListLayoutProperty>();
    CHECK_NULL_VOID(listLayoutProperty);
    auto axis = listLayoutProperty->GetListDirection().value_or(Axis::VERTICAL);
    SetAxis(axis);
    if (!GetScrollableEvent()) {
        InitScrollableEvent();
    }
    auto edgeEffect = listLayoutProperty->GetEdgeEffect().value_or(EdgeEffect::SPRING);
    SetEdgeEffect(edgeEffect);

    auto defaultDisplayMode = DisplayMode::OFF;
    const static int32_t PLATFORM_VERSION_TEN = 10;
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    if (pipeline->GetMinPlatformVersion() >= PLATFORM_VERSION_TEN) {
        defaultDisplayMode = DisplayMode::AUTO;
    }
    auto listPaintProperty = host->GetPaintProperty<ListPaintProperty>();
    SetScrollBar(listPaintProperty->GetBarDisplayMode().value_or(defaultDisplayMode));
    SetChainAnimation();
    if (multiSelectable_ && !isMouseEventInit_) {
        InitMouseEvent();
    }
    if (!multiSelectable_ && isMouseEventInit_) {
        UninitMouseEvent();
    }
    auto focusHub = host->GetFocusHub();
    CHECK_NULL_VOID_NOLOG(focusHub);
    InitOnKeyEvent(focusHub);
    SetAccessibilityAction();
}

bool ListPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }
    bool isJump = false;
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto listLayoutAlgorithm = DynamicCast<ListLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(listLayoutAlgorithm, false);
    itemPosition_ = listLayoutAlgorithm->GetItemPosition();
    maxListItemIndex_ = listLayoutAlgorithm->GetMaxListItemIndex();
    spaceWidth_ = listLayoutAlgorithm->GetSpaceWidth();
    float relativeOffset = listLayoutAlgorithm->GetCurrentOffset();
    if (jumpIndex_) {
        float absoluteOffset = listLayoutAlgorithm->GetEstimateOffset();
        relativeOffset += absoluteOffset - currentOffset_;
        isJump = true;
        jumpIndex_.reset();
    }
    if (listLayoutAlgorithm->GetStartIndex() == 0) {
        currentOffset_ = -listLayoutAlgorithm->GetStartPosition();
    } else {
        currentOffset_ = currentOffset_ + relativeOffset;
    }
    if (isScrollEnd_) {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, false);
        host->OnAccessibilityEvent(AccessibilityEventType::SCROLL_END);
        isScrollEnd_ = false;
    }
    currentDelta_ = 0.0f;
    float prevStartOffset = startMainPos_;
    float prevEndOffset = endMainPos_ - contentMainSize_;
    contentMainSize_ = listLayoutAlgorithm->GetContentMainSize();
    startMainPos_ = listLayoutAlgorithm->GetStartPosition();
    endMainPos_ = listLayoutAlgorithm->GetEndPosition();
    crossMatchChild_ = listLayoutAlgorithm->IsCrossMatchChild();
    itemGroupList_.swap(listLayoutAlgorithm->GetItemGroupList());
    auto lanesLayoutAlgorithm = DynamicCast<ListLanesLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    if (lanesLayoutAlgorithm) {
        lanesLayoutAlgorithm->SwapLanesItemRange(lanesItemRange_);
        lanes_ = lanesLayoutAlgorithm->GetLanes();
    }
    CheckScrollable();

    bool indexChanged =
        (startIndex_ != listLayoutAlgorithm->GetStartIndex()) || (endIndex_ != listLayoutAlgorithm->GetEndIndex());
    startIndex_ = listLayoutAlgorithm->GetStartIndex();
    endIndex_ = listLayoutAlgorithm->GetEndIndex();
    ProcessEvent(indexChanged, relativeOffset, isJump, prevStartOffset, prevEndOffset);
    UpdateScrollBarOffset();
    CheckRestartSpring();

    DrivenRender(dirty);

    SetScrollState(SCROLL_FROM_NONE);
    isInitialized_ = true;
    return true;
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
    paint->SetScrollBar(AceType::WeakClaim(AceType::RawPtr(GetScrollBar())));
    paint->SetTotalItemCount(maxListItemIndex_ + 1);
    auto scrollEffect = GetScrollEdgeEffect();
    if (scrollEffect && scrollEffect->IsFadeEffect()) {
        paint->SetEdgeEffect(scrollEffect);
    }
    if (!listContentModifier_) {
        listContentModifier_ = AceType::MakeRefPtr<ListContentModifier>();
    }
    listContentModifier_->SetItemsPosition(itemPosition_);
    paint->SetContentModifier(listContentModifier_);
    return paint;
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
    if (onScroll && !NearZero(finalOffset)) {
        auto source = GetScrollState();
        auto offsetPX = Dimension(finalOffset);
        auto offsetVP = Dimension(offsetPX.ConvertToVp(), DimensionUnit::VP);
        if (source == SCROLL_FROM_UPDATE) {
            onScroll(offsetVP, ScrollState::SCROLL);
        } else if (source == SCROLL_FROM_ANIMATION || source == SCROLL_FROM_ANIMATION_SPRING) {
            onScroll(offsetVP, ScrollState::FLING);
        } else {
            onScroll(offsetVP, ScrollState::IDLE);
        }
    }

    if (indexChanged) {
        auto onScrollIndex = listEventHub->GetOnScrollIndex();
        if (onScrollIndex) {
            onScrollIndex(startIndex_, endIndex_);
        }
    }

    auto onReachStart = listEventHub->GetOnReachStart();
    if (onReachStart && (startIndex_ == 0)) {
        bool scrollUpToStart = Positive(prevStartOffset) && NonPositive(startMainPos_);
        bool scrollDownToStart = (Negative(prevStartOffset) || !isInitialized_) && NonNegative(startMainPos_);
        if (scrollUpToStart || scrollDownToStart) {
            onReachStart();
        }
    }
    auto onReachEnd = listEventHub->GetOnReachEnd();
    if (onReachEnd && (endIndex_ == maxListItemIndex_)) {
        float endOffset = endMainPos_ - contentMainSize_;
        bool scrollUpToEnd = (Positive(prevEndOffset) || !isInitialized_) && NonPositive(endOffset);
        bool scrollDownToEnd = Negative(prevEndOffset) && NonNegative(endOffset);
        if (scrollUpToEnd || (scrollDownToEnd && GetScrollState() != SCROLL_FROM_NONE)) {
            onReachEnd();
        }
    }

    if (scrollStop_) {
        auto onScrollStop = listEventHub->GetOnScrollStop();
        if (!scrollAbort_ && onScrollStop) {
            onScrollStop();
        }
        scrollStop_ = false;
        scrollAbort_ = false;
    }

    if (isScrollEnd_) {
        isScrollEnd_ = false;
    }
}

void ListPattern::DrivenRender(const RefPtr<LayoutWrapper>& layoutWrapper)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto listLayoutProperty = host->GetLayoutProperty<ListLayoutProperty>();
    auto listPaintProperty = host->GetPaintProperty<ListPaintProperty>();
    auto axis = listLayoutProperty->GetListDirection().value_or(Axis::VERTICAL);
    auto stickyStyle = listLayoutProperty->GetStickyStyle().value_or(V2::StickyStyle::NONE);
    bool barNeedPaint = GetScrollBar() ? GetScrollBar()->NeedPaint() : false;
    auto chainAnimation = listLayoutProperty->GetChainAnimation().value_or(false);
    bool drivenRender = !(axis != Axis::VERTICAL || stickyStyle != V2::StickyStyle::NONE ||
        barNeedPaint || chainAnimation || !scrollable_);

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

    if (itemPosition_.empty()) {
        scrollable_ = false;
    } else {
        if ((itemPosition_.begin()->first == 0) && (itemPosition_.rbegin()->first == maxListItemIndex_)) {
            scrollable_ = GreatNotEqual((endMainPos_ - startMainPos_), contentMainSize_);
        } else {
            scrollable_ = true;
        }
    }

    SetScrollEnable(scrollable_);
}

RefPtr<LayoutAlgorithm> ListPattern::CreateLayoutAlgorithm()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto listLayoutProperty = host->GetLayoutProperty<ListLayoutProperty>();
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm;
    if (listLayoutProperty->HasLanes() || listLayoutProperty->HasLaneMinLength() ||
        listLayoutProperty->HasLaneMaxLength()) {
        auto lanesLayoutAlgorithm = MakeRefPtr<ListLanesLayoutAlgorithm>();
        if ((listLayoutProperty->GetPropertyChangeFlag() & PROPERTY_UPDATE_MEASURE_SELF_AND_PARENT) == 0) {
            lanesLayoutAlgorithm->SwapLanesItemRange(lanesItemRange_);
        }
        lanesLayoutAlgorithm->SetLanes(lanes_);
        listLayoutAlgorithm.Swap(lanesLayoutAlgorithm);
    } else {
        listLayoutAlgorithm.Swap(MakeRefPtr<ListLayoutAlgorithm>());
    }
    if (jumpIndex_) {
        listLayoutAlgorithm->SetIndex(jumpIndex_.value());
        listLayoutAlgorithm->SetIndexAlignment(scrollIndexAlignment_);
    }
    if (jumpIndexInGroup_) {
        listLayoutAlgorithm->SetIndexInGroup(jumpIndexInGroup_.value());
        jumpIndexInGroup_.reset();
    }
    listLayoutAlgorithm->SetCurrentDelta(currentDelta_);
    listLayoutAlgorithm->SetItemsPosition(itemPosition_);
    listLayoutAlgorithm->SetPrevContentMainSize(contentMainSize_);
    if (IsOutOfBoundary(false) && scrollState_ != SCROLL_FROM_AXIS) {
        listLayoutAlgorithm->SetOverScrollFeature();
    }
    auto effect = listLayoutProperty->GetEdgeEffect().value_or(EdgeEffect::SPRING);
    bool canOverScroll = (effect == EdgeEffect::SPRING) && !ScrollableIdle() &&
        scrollState_ != SCROLL_FROM_BAR && scrollState_ != SCROLL_FROM_AXIS;
    listLayoutAlgorithm->SetCanOverScroll(canOverScroll);
    if (chainAnimation_) {
        listLayoutAlgorithm->SetChainOffsetCallback([weak = AceType::WeakClaim(this)](int32_t index) {
            auto list = weak.Upgrade();
            CHECK_NULL_RETURN(list, 0.0f);
            return list->GetChainDelta(index);
        });
        if (!listLayoutProperty->GetSpace().has_value() && chainAnimation_) {
            listLayoutAlgorithm->SetChainInterval(CHAIN_INTERVAL_DEFAULT.ConvertToPx());
        }
    }
    return listLayoutAlgorithm;
}

bool ListPattern::IsAtTop() const
{
    return (startIndex_ == 0) && NonNegative(startMainPos_ + GetChainDelta(0));
}

bool ListPattern::IsAtBottom() const
{
    return endIndex_ == maxListItemIndex_ && LessOrEqual(endMainPos_ + GetChainDelta(endIndex_), contentMainSize_);
}

bool ListPattern::OutBoundaryCallback()
{
    bool outBoundary = IsAtTop() || IsAtBottom();
    if (!dragFromSpring_ && outBoundary && chainAnimation_) {
        chainAnimation_->SetOverDrag(false);
        auto delta = chainAnimation_->SetControlIndex(IsAtTop() ? 0 : maxListItemIndex_);
        currentDelta_ -= delta;
        dragFromSpring_ = true;
    }
    return outBoundary;
}

bool ListPattern::UpdateCurrentOffset(float offset, int32_t source)
{
    // check edgeEffect is not springEffect
    if (!jumpIndex_.has_value() && !HandleEdgeEffect(offset, source, GetContentSize())) {
        return false;
    }
    SetScrollState(source);
    currentDelta_ = currentDelta_ - offset;
    MarkDirtyNodeSelf();
    if (!IsOutOfBoundary() || !scrollable_) {
        return true;
    }

    // over scroll in drag update from normal to over scroll.
    float overScroll = 0.0f;
    // over scroll in drag update during over scroll.
    auto startPos = startMainPos_ - currentDelta_;
    if ((itemPosition_.begin()->first == 0) && Positive(startPos)) {
        overScroll = startPos;
    } else {
        overScroll = contentMainSize_ - (endMainPos_ - currentDelta_);
    }

    if (scrollState_ == SCROLL_FROM_UPDATE) {
        // adjust offset.
        auto friction = CalculateFriction(std::abs(overScroll) / contentMainSize_);
        currentDelta_ = currentDelta_ * friction;
    }
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
    for (const auto& weak : itemGroupList_) {
        auto itemGroup = weak.Upgrade();
        if (!itemGroup) {
            continue;
        }
        auto layoutProperty = itemGroup->GetLayoutProperty();
        if (layoutProperty) {
            layoutProperty->UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE_SELF);
        }
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
    return geometryNode->GetPaddingSize();
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
    bool outOfStart = (startIndex_ == 0) && Positive(startPos) && GreatNotEqual(endPos, contentMainSize_);
    bool outOfEnd = (endIndex_ == maxListItemIndex_) && LessNotEqual(endPos, contentMainSize_) && Negative(startPos);
    return outOfStart || outOfEnd;
}

void ListPattern::FireOnScrollStart()
{
    if (scrollAbort_) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<ListEventHub>();
    CHECK_NULL_VOID_NOLOG(hub);
    auto onScrollStart = hub->GetOnScrollStart();
    CHECK_NULL_VOID_NOLOG(onScrollStart);
    onScrollStart();
}

bool ListPattern::OnScrollCallback(float offset, int32_t source)
{
    if (source == SCROLL_FROM_START) {
        ProcessDragStart(offset);
        auto item = swiperItem_.Upgrade();
        if (item) {
            item->SwiperReset();
        }
        if (animator_ && !animator_->IsStopped()) {
            scrollAbort_ = true;
            animator_->Stop();
        }
        FireOnScrollStart();
        return true;
    }
    if (animator_ && !animator_->IsStopped()) {
        return false;
    }
    auto scrollBar = GetScrollBar();
    if (scrollBar && scrollBar->IsDriving()) {
        offset = scrollBar->CalcPatternOffset(offset);
        source = SCROLL_FROM_BAR;
    } else {
        ProcessDragUpdate(offset, source);
    }
    return UpdateCurrentOffset(offset, source);
}

void ListPattern::InitScrollableEvent()
{
    AddScrollEvent();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto listEventHub = host->GetEventHub<ListEventHub>();
    auto onScrollBegin = listEventHub->GetOnScrollBegin();
    auto onScrollFrameBegin = listEventHub->GetOnScrollFrameBegin();
    auto scrollableEvent = GetScrollableEvent();
    CHECK_NULL_VOID(scrollableEvent);
    if (onScrollBegin) {
        scrollableEvent->SetScrollBeginCallback(std::move(onScrollBegin));
    }
    if (onScrollFrameBegin) {
        scrollableEvent->SetScrollFrameBeginCallback(std::move(onScrollFrameBegin));
    }
}

void ListPattern::SetEdgeEffectCallback(const RefPtr<ScrollEdgeEffect>& scrollEffect)
{
    scrollEffect->SetCurrentPositionCallback([weak = AceType::WeakClaim(this)]() -> double {
        auto list = weak.Upgrade();
        CHECK_NULL_RETURN_NOLOG(list, 0.0);
        return list->startMainPos_ + list->GetChainDelta(list->startIndex_) - list->currentDelta_;
    });
    scrollEffect->SetLeadingCallback([weak = AceType::WeakClaim(this)]() -> double {
        auto list = weak.Upgrade();
        auto endPos = list->endMainPos_ + list->GetChainDelta(list->endIndex_);
        auto startPos = list->startMainPos_ + list->GetChainDelta(list->startIndex_);
        return list->contentMainSize_ - (endPos - startPos);
    });
    scrollEffect->SetTrailingCallback([]() -> double { return 0.0; });
    scrollEffect->SetInitLeadingCallback([weak = AceType::WeakClaim(this)]() -> double {
        auto list = weak.Upgrade();
        auto endPos = list->endMainPos_ + list->GetChainDelta(list->endIndex_);
        auto startPos = list->startMainPos_ + list->GetChainDelta(list->startIndex_);
        return list->contentMainSize_ - (endPos - startPos);
    });
    scrollEffect->SetInitTrailingCallback([]() -> double { return 0.0; });
}

void ListPattern::CheckRestartSpring()
{
    if (!ScrollableIdle() || !IsOutOfBoundary()) {
        return;
    }
    auto edgeEffect = GetScrollEdgeEffect();
    if (!edgeEffect || !edgeEffect->IsSpringEffect()) {
        return;
    }
    if (animator_ && animator_->IsRunning()) {
        return;
    }
    FireOnScrollStart();
    edgeEffect->ProcessScrollOver(0);
}

void ListPattern::InitOnKeyEvent(const RefPtr<FocusHub>& focusHub)
{
    auto onKeyEvent = [wp = WeakClaim(this)](const KeyEvent& event) -> bool {
        auto pattern = wp.Upgrade();
        CHECK_NULL_RETURN_NOLOG(pattern, false);
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
        LOGD("Keycode is PgDn. Scroll to next page");
        ScrollPage(false);
        return true;
    }
    if (event.code == KeyCode::KEY_PAGE_UP) {
        LOGD("Keycode is PgDn. Scroll to next page");
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
    if (curIndex < 0 || curIndex > maxListItemIndex_) {
        LOGE("can't find focused child.");
        return nullptr;
    }

    auto moveStep = 0;
    auto nextIndex = curIndex;
    if ((isVertical && step == FocusStep::UP_END) || (!isVertical && step == FocusStep::LEFT_END)) {
        moveStep = 1;
        nextIndex = 0;
    } else if ((isVertical && step == FocusStep::DOWN_END) || (!isVertical && step == FocusStep::RIGHT_END)) {
        moveStep = -1;
        nextIndex = maxListItemIndex_;
    } else if ((isVertical && (step == FocusStep::DOWN)) || (!isVertical && step == FocusStep::RIGHT)) {
        moveStep = 1;
        nextIndex = curIndex + moveStep;
    } else if ((isVertical && step == FocusStep::UP) || (!isVertical && step == FocusStep::LEFT)) {
        moveStep = -1;
        nextIndex = curIndex + moveStep;
    }

    while (nextIndex >= 0 && nextIndex <= maxListItemIndex_) {
        // Need update here. ListItemGroup.
        if (nextIndex == curIndex) {
            return nullptr;
        }

        if (nextIndex < curIndex && nextIndex < startIndex_) {
            ScrollToIndex(nextIndex, ScrollIndexAlignment::ALIGN_TOP);
            auto pipeline = PipelineContext::GetCurrentContext();
            if (pipeline) {
                pipeline->FlushUITasks();
            }
        } else if (nextIndex > curIndex && nextIndex > endIndex_) {
            ScrollToIndex(nextIndex, ScrollIndexAlignment::ALIGN_BOTTOM);
            auto pipeline = PipelineContext::GetCurrentContext();
            if (pipeline) {
                pipeline->FlushUITasks();
            }
        }

        auto nextFocusNode = GetChildFocusNodeByIndex(nextIndex, curIndexInGroup);
        if (nextFocusNode.Upgrade()) {
            return nextFocusNode;
        }
        nextIndex += moveStep;
    }
    return nullptr;
}

WeakPtr<FocusHub> ListPattern::GetChildFocusNodeByIndex(int32_t tarMainIndex, int32_t tarGroupIndex)
{
    LOGD("Get target item location is (%{public}d,%{public}d)", tarMainIndex, tarGroupIndex);
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
    LOGD("The target item at location(%{public}d,%{public}d) can not found.", tarMainIndex, tarGroupIndex);
    return nullptr;
}

void ListPattern::AnimateTo(float position, float duration, const RefPtr<Curve>& curve)
{
    LOGI("AnimateTo:%f, duration:%f", position, duration);
    if (!IsScrollableStopped()) {
        scrollAbort_ = true;
        StopScrollable();
    }
    if (!animator_) {
        animator_ = CREATE_ANIMATOR(PipelineBase::GetCurrentContext());
        animator_->AddStopListener([weak = AceType::WeakClaim(this)]() {
            auto list = weak.Upgrade();
            CHECK_NULL_VOID_NOLOG(list);
            list->scrollStop_ = true;
            list->MarkDirtyNodeSelf();
            list->isScrollEnd_ = true;
        });
    } else if (!animator_->IsStopped()) {
        scrollAbort_ = true;
        animator_->Stop();
    }
    animator_->ClearInterpolators();
    auto animation = AceType::MakeRefPtr<CurveAnimation<float>>(GetTotalOffset(), position, curve);
    animation->AddListener([weakScroll = AceType::WeakClaim(this)](float value) {
        auto list = weakScroll.Upgrade();
        CHECK_NULL_VOID_NOLOG(list);
        list->UpdateCurrentOffset(list->GetTotalOffset() - value, SCROLL_FROM_JUMP);
    });
    animator_->AddInterpolator(animation);
    animator_->SetDuration(static_cast<int32_t>(duration));
    animator_->Play();
    FireOnScrollStart();
}

void ListPattern::StopAnimate()
{
    if (!IsScrollableStopped()) {
        StopScrollable();
    }
    if (animator_ && !animator_->IsStopped()) {
        animator_->Stop();
    }
}

void ListPattern::ScrollTo(float position)
{
    LOGI("ScrollTo:%{public}f", position);
    StopAnimate();
    UpdateCurrentOffset(GetTotalOffset() - position, SCROLL_FROM_JUMP);
    isScrollEnd_ = true;
}

void ListPattern::ScrollToIndex(int32_t index, ScrollIndexAlignment align)
{
    LOGI("ScrollToIndex:%{public}d", index);
    StopAnimate();
    if (index >= 0 || index == ListLayoutAlgorithm::LAST_ITEM) {
        currentDelta_ = 0;
        jumpIndex_ = index;
        scrollIndexAlignment_ = align;
        MarkDirtyNodeSelf();
    }
    isScrollEnd_ = true;
}

void ListPattern::ScrollToIndex(int32_t index, int32_t indexInGroup, ScrollIndexAlignment align)
{
    LOGI("ScrollToIndex:%{public}d, %{public}d", index, indexInGroup);
    StopAnimate();
    if (index >= 0 || index == ListLayoutAlgorithm::LAST_ITEM) {
        currentDelta_ = 0;
        jumpIndex_ = index;
        jumpIndexInGroup_ = indexInGroup;
        scrollIndexAlignment_ = align;
        MarkDirtyNodeSelf();
    }
    isScrollEnd_ = true;
}

void ListPattern::ScrollToEdge(ScrollEdgeType scrollEdgeType)
{
    LOGI("ScrollToEdge:%{public}zu", scrollEdgeType);
    if (scrollEdgeType == ScrollEdgeType::SCROLL_TOP) {
        ScrollToIndex(0, ScrollIndexAlignment::ALIGN_TOP);
    } else if (scrollEdgeType == ScrollEdgeType::SCROLL_BOTTOM) {
        ScrollToIndex(ListLayoutAlgorithm::LAST_ITEM, ScrollIndexAlignment::ALIGN_BOTTOM);
    }
}

bool ListPattern::ScrollPage(bool reverse)
{
    LOGI("ScrollPage:%{public}d", reverse);
    StopAnimate();
    float distance = reverse ? contentMainSize_ : -contentMainSize_;
    UpdateCurrentOffset(distance, SCROLL_FROM_JUMP);
    isScrollEnd_ = true;
    return true;
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

void ListPattern::UpdateScrollBarOffset()
{
    if (itemPosition_.empty()) {
        return;
    }
    if (!GetScrollBar() && !GetScrollBarProxy()) {
        return;
    }
    SizeF ContentSize = GetContentSize();
    Size size(ContentSize.Width(), ContentSize.Height());
    float itemsSize = itemPosition_.rbegin()->second.endPos - itemPosition_.begin()->second.startPos + spaceWidth_;
    float currentOffset = itemsSize / itemPosition_.size() * itemPosition_.begin()->first - startMainPos_;
    Offset scrollOffset = { currentOffset, currentOffset }; // fit for w/h switched.
    auto estimatedHeight = itemsSize / itemPosition_.size() * (maxListItemIndex_ + 1);

    // calculate padding offset of list
    auto host = GetHost();
    CHECK_NULL_VOID_NOLOG(host);
    auto layoutPriority = host->GetLayoutProperty();
    CHECK_NULL_VOID_NOLOG(layoutPriority);
    auto paddingOffset = layoutPriority->CreatePaddingAndBorder().Offset();
    Offset viewOffset = { paddingOffset.GetX(), paddingOffset.GetY() };
    UpdateScrollBarRegion(currentOffset, estimatedHeight, size, viewOffset);
}

void ListPattern::SetChainAnimation()
{
    auto listLayoutProperty = GetLayoutProperty<ListLayoutProperty>();
    CHECK_NULL_VOID(listLayoutProperty);
    auto edgeEffect = listLayoutProperty->GetEdgeEffect().value_or(EdgeEffect::SPRING);
    int32_t lanes = listLayoutProperty->GetLanes().value_or(1);
    bool autoLanes = listLayoutProperty->HasLaneMinLength() || listLayoutProperty->HasLaneMaxLength();
    bool animation = listLayoutProperty->GetChainAnimation().value_or(false);
    bool enable = edgeEffect == EdgeEffect::SPRING && lanes == 1 && !autoLanes && animation;
    if (!enable) {
        chainAnimation_.Reset();
        return;
    }
    if (!chainAnimation_) {
        auto space = listLayoutProperty->GetSpace().value_or(CHAIN_INTERVAL_DEFAULT).ConvertToPx();
        springProperty_ =
            AceType::MakeRefPtr<SpringProperty>(CHAIN_SPRING_MASS, CHAIN_SPRING_STIFFNESS, CHAIN_SPRING_DAMPING);
        if (chainAnimationOptions_.has_value()) {
            float maxSpace = chainAnimationOptions_.value().maxSpace.ConvertToPx();
            float minSpace = chainAnimationOptions_.value().minSpace.ConvertToPx();
            if (GreatNotEqual(minSpace, maxSpace)) {
                minSpace = space;
                maxSpace = space;
            }
            chainAnimation_ =
                AceType::MakeRefPtr<ChainAnimation>(space, maxSpace, minSpace, springProperty_);
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
        float maxSpace = options.maxSpace.ConvertToPx();
        float minSpace = options.minSpace.ConvertToPx();
        if (GreatNotEqual(minSpace, maxSpace)) {
            minSpace = space;
            maxSpace = space;
        }
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
}

void ListPattern::ProcessDragStart(float startPosition)
{
    CHECK_NULL_VOID_NOLOG(chainAnimation_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto globalOffset = host->GetTransformRelativeOffset();
    int32_t index = -1;
    auto offset = startPosition - GetMainAxisOffset(globalOffset, GetAxis());
    auto it = std::find_if(itemPosition_.begin(), itemPosition_.end(), [offset](auto pos) {
        return offset <= pos.second.endPos;
    });
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
    CHECK_NULL_VOID_NOLOG(chainAnimation_);
    if (NearZero(dragOffset)) {
        return;
    }
    bool overDrag = (source == SCROLL_FROM_UPDATE) && (IsAtTop() || IsAtBottom());
    chainAnimation_->SetDelta(-dragOffset, overDrag);
}

float ListPattern::GetChainDelta(int32_t index) const
{
    CHECK_NULL_RETURN_NOLOG(chainAnimation_, 0.0f);
    return chainAnimation_->GetValue(index);
}

void ListPattern::UninitMouseEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto mouseEventHub = host->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(mouseEventHub);
    mouseEventHub->SetMouseEvent(nullptr);
    ClearMultiSelect();
    isMouseEventInit_ = false;
}

void ListPattern::InitMouseEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto mouseEventHub = host->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(mouseEventHub);
    mouseEventHub->SetMouseEvent([weak = WeakClaim(this)](MouseInfo& info) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleMouseEventWithoutKeyboard(info);
        }
    });
    isMouseEventInit_ = true;
}

void ListPattern::HandleMouseEventWithoutKeyboard(const MouseInfo& info)
{
    if (info.GetButton() != MouseButton::LEFT_BUTTON) {
        return;
    }

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto manager = pipeline->GetDragDropManager();
    CHECK_NULL_VOID(manager);
    if (manager->IsDragged()) {
        return;
    }

    auto mouseOffsetX = static_cast<float>(info.GetLocalLocation().GetX());
    auto mouseOffsetY = static_cast<float>(info.GetLocalLocation().GetY());
    if (info.GetAction() == MouseAction::PRESS) {
        ClearMultiSelect();
        mouseStartOffset_ = OffsetF(mouseOffsetX, mouseOffsetY);
        mouseEndOffset_ = OffsetF(mouseOffsetX, mouseOffsetY);
        mousePressOffset_ = OffsetF(mouseOffsetX, mouseOffsetY);
        mousePressed_ = true;
        // do not select when click
    } else if (info.GetAction() == MouseAction::MOVE) {
        if (!mousePressed_) {
            return;
        }
        const static double FRAME_SELECTION_DISTANCE =
            pipeline->NormalizeToPx(Dimension(DEFAULT_PAN_DISTANCE, DimensionUnit::VP));
        auto delta = OffsetF(mouseOffsetX, mouseOffsetY) - mousePressOffset_;
        if (Offset(delta.GetX(), delta.GetY()).GetDistance() > FRAME_SELECTION_DISTANCE) {
            mouseEndOffset_ = OffsetF(mouseOffsetX, mouseOffsetY);
            auto selectedZone = ComputeSelectedZone(mouseStartOffset_, mouseEndOffset_);
            MultiSelectWithoutKeyboard(selectedZone);
        }
    } else if (info.GetAction() == MouseAction::RELEASE) {
        mouseStartOffset_.Reset();
        mouseEndOffset_.Reset();
        mousePressed_ = false;
        ClearSelectedZone();
    }
}

void ListPattern::MultiSelectWithoutKeyboard(const RectF& selectedZone)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    std::list<RefPtr<FrameNode>> childrens;
    host->GenerateOneDepthVisibleFrame(childrens);
    for (const auto& item : childrens) {
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

    auto hostContext = host->GetRenderContext();
    CHECK_NULL_VOID(hostContext);
    hostContext->UpdateMouseSelectWithRect(selectedZone, SELECT_FILL_COLOR, SELECT_STROKE_COLOR);
}

void ListPattern::ClearMultiSelect()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    std::list<RefPtr<FrameNode>> children;
    host->GenerateOneDepthAllFrame(children);
    for (const auto& item : children) {
        if (!AceType::InstanceOf<FrameNode>(item)) {
            continue;
        }

        auto itemFrameNode = AceType::DynamicCast<FrameNode>(item);
        auto itemPattern = itemFrameNode->GetPattern<ListItemPattern>();
        CHECK_NULL_VOID(itemPattern);
        itemPattern->MarkIsSelected(false);
    }

    ClearSelectedZone();
}

void ListPattern::ClearSelectedZone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hostContext = host->GetRenderContext();
    CHECK_NULL_VOID(hostContext);
    hostContext->UpdateMouseSelectWithRect(RectF(), SELECT_FILL_COLOR, SELECT_STROKE_COLOR);
}

RectF ListPattern::ComputeSelectedZone(const OffsetF& startOffset, const OffsetF& endOffset)
{
    RectF selectedZone;
    if (startOffset.GetX() <= endOffset.GetX()) {
        if (startOffset.GetY() <= endOffset.GetY()) {
            // bottom right
            selectedZone = RectF(startOffset.GetX(), startOffset.GetY(), endOffset.GetX() - startOffset.GetX(),
                endOffset.GetY() - startOffset.GetY());
        } else {
            // top right
            selectedZone = RectF(startOffset.GetX(), endOffset.GetY(), endOffset.GetX() - startOffset.GetX(),
                startOffset.GetY() - endOffset.GetY());
        }
    } else {
        if (startOffset.GetY() <= endOffset.GetY()) {
            // bottom left
            selectedZone = RectF(endOffset.GetX(), startOffset.GetY(), startOffset.GetX() - endOffset.GetX(),
                endOffset.GetY() - startOffset.GetY());
        } else {
            // top left
            selectedZone = RectF(endOffset.GetX(), endOffset.GetY(), startOffset.GetX() - endOffset.GetX(),
                startOffset.GetY() - endOffset.GetY());
        }
    }

    return selectedZone;
}

void ListPattern::SetSwiperItem(WeakPtr<ListItemPattern> swiperItem)
{
    if (swiperItem_ != swiperItem) {
        auto item = swiperItem_.Upgrade();
        if (item) {
            item->SwiperReset();
        }
        swiperItem_ = std::move(swiperItem);
    }
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
    for (auto & pos : itemPosition_) {
        if (mainOffset <= pos.second.endPos + spaceWidth_ / 2) { /* 2:half */
            return std::min(pos.first + lanesOffset, maxListItemIndex_ + 1);
        }
    }
    if (!itemPosition_.empty()) {
        return itemPosition_.rbegin()->first + 1;
    }
    return 0;
}

void ListPattern::ToJsonValue(std::unique_ptr<JsonValue>& json) const
{
    json->Put("multiSelectable", multiSelectable_);
    json->Put("startIndex", startIndex_);
    json->Put("startMainPos", startMainPos_);
}

void ListPattern::FromJson(const std::unique_ptr<JsonValue>& json)
{
    ScrollToIndex(json->GetInt("startIndex"));
    ScrollBy(-json->GetDouble("startMainPos"));
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
} // namespace OHOS::Ace::NG
