/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/scrollable/scrollable_controller.h"

#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"

namespace OHOS::Ace::NG {
void ScrollableController::ScrollToIndex(
    int32_t index, bool smooth, ScrollAlign align, std::optional<float> extraOffset)
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_VOID(pattern);
    CHECK_NULL_VOID(pattern->SupportScrollToIndex());
    if (align == ScrollAlign::NONE) {
        align = pattern->GetDefaultScrollAlign();
    }
    pattern->ScrollToIndex(index, smooth, align, extraOffset);
}

bool ScrollableController::AnimateTo(
    const Dimension& position, float duration, const RefPtr<Curve>& curve, bool smooth, bool canOverScroll)
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_RETURN(pattern, false);
    if (pattern->GetAxis() != Axis::NONE) {
        if (position.Unit() == DimensionUnit::PERCENT) {
            return false;
        }
        if (Positive(duration) || smooth) {
            pattern->AnimateTo(position.ConvertToPx(), duration, curve, smooth, canOverScroll);
        } else {
            pattern->ScrollTo(position.ConvertToPx());
        }
        return true;
    }
    return false;
}

Offset ScrollableController::GetCurrentOffset() const
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_RETURN(pattern, Offset::Zero());
    auto axis = pattern->GetAxis();
    if (axis == Axis::NONE) {
        return Offset::Zero();
    }
    auto pxOffset = pattern->GetTotalOffset();
    auto vpOffset = Dimension(pxOffset, DimensionUnit::PX).ConvertToVp();
    return (axis == Axis::HORIZONTAL) ? Offset(vpOffset, 0) : Offset(0, vpOffset);
}

Axis ScrollableController::GetScrollDirection() const
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_RETURN(pattern, Axis::NONE);
    return pattern->GetAxis();
}

void ScrollableController::ScrollBy(double pixelX, double pixelY, bool /* smooth */)
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_VOID(pattern);
    pattern->StopAnimate();
    auto offset = pattern->GetAxis() == Axis::VERTICAL ? pixelY : pixelX;
    pattern->UpdateCurrentOffset(static_cast<float>(-offset), SCROLL_FROM_JUMP);
}

void ScrollableController::ScrollToEdge(ScrollEdgeType scrollEdgeType, float velocity)
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_VOID(pattern);
    if (scrollEdgeType == ScrollEdgeType::SCROLL_TOP) {
        pattern->ScrollAtFixedVelocity(velocity);
    } else if (scrollEdgeType == ScrollEdgeType::SCROLL_BOTTOM) {
        pattern->ScrollAtFixedVelocity(-velocity);
    }
}

void ScrollableController::ScrollToEdge(ScrollEdgeType scrollEdgeType, bool smooth)
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_VOID(pattern);
    if (pattern->GetAxis() != Axis::NONE) {
        pattern->ScrollToEdge(scrollEdgeType, smooth);
    }
}

void ScrollableController::Fling(double flingVelocity)
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_VOID(pattern);
    pattern->Fling(flingVelocity);
}

void ScrollableController::ScrollPage(bool reverse, bool smooth)
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_VOID(pattern);
    if (pattern->GetAxis() == Axis::NONE) {
        return;
    }
    auto offset = reverse ? pattern->GetMainContentSize() : -pattern->GetMainContentSize();
    if (smooth) {
        auto position = pattern->GetTotalOffset() - offset;
        pattern->AnimateTo(position, -1, nullptr, true);
    } else {
        pattern->StopAnimate();
        pattern->UpdateCurrentOffset(offset, SCROLL_FROM_JUMP);
    }
}

bool ScrollableController::IsAtEnd() const
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_RETURN(pattern, false);
    return pattern->IsAtBottom();
}

Rect ScrollableController::GetItemRect(int32_t index) const
{
    auto pattern = scroll_.Upgrade();
    CHECK_NULL_RETURN(pattern, Rect());
    auto pxRect = pattern->GetItemRect(index);
    auto pxOffset = pxRect.GetOffset();
    return Rect(Dimension(pxOffset.GetX(), DimensionUnit::PX).ConvertToVp(),
                Dimension(pxOffset.GetY(), DimensionUnit::PX).ConvertToVp(),
                Dimension(pxRect.Width(), DimensionUnit::PX).ConvertToVp(),
                Dimension(pxRect.Height(), DimensionUnit::PX).ConvertToVp());
}
} // namespace OHOS::Ace::NG
