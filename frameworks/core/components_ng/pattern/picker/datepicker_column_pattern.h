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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_DATE_PICKER_DATE_PICKER_COLUMN_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_DATE_PICKER_DATE_PICKER_COLUMN_PATTERN_H

#include <utility>

#include "base/i18n/localization.h"
#include "core/components/picker/picker_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/picker/datepicker_column_accessibility_property.h"
#include "core/components_ng/pattern/picker/datepicker_column_layout_algorithm.h"
#include "core/components_ng/pattern/picker/datepicker_event_hub.h"
#include "core/components_ng/pattern/picker/datepicker_layout_property.h"
#include "core/components_ng/pattern/picker/datepicker_paint_method.h"
#include "core/components_ng/pattern/picker/datepicker_row_layout_property.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/picker/toss_animation_controller.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {

using ColumnChangeCallback = std::function<void(const RefPtr<FrameNode>&, bool, uint32_t, bool)>;
using ColumnFinishCallback = std::function<void(bool)>;
using EventCallback = std::function<void(bool)>;

struct DateTextProperties {
    Dimension upFontSize;
    Dimension fontSize;
    Dimension downFontSize;
    Color upColor;
    Color currentColor;
    Color downColor;
};

class DatePickerColumnPattern : public LinearLayoutPattern {
    DECLARE_ACE_TYPE(DatePickerColumnPattern, LinearLayoutPattern);

public:
    DatePickerColumnPattern() : LinearLayoutPattern(true) {};

    ~DatePickerColumnPattern() override = default;

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        auto layoutAlgorithm = MakeRefPtr<DatePickerColumnLayoutAlgorithm>();
        layoutAlgorithm->SetCurrentOffset(GetCurrentOffset());
        return layoutAlgorithm;
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<DataPickerLayoutProperty>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<DatePickerColumnAccessibilityProperty>();
    }

    void FlushCurrentOptions(bool isDown = false, bool isUpateTextContentOnly = false,
        bool isUpdateAnimationProperties = false);

    bool NotLoopOptions() const;

    void UpdateColumnChildPosition(double offsetY);

    bool CanMove(bool isDown) const;

    bool InnerHandleScroll(bool isDown, bool isUpatePropertiesOnly = false, bool isUpdateAnimationProperties = false);

    uint32_t GetCurrentIndex() const
    {
        // currentIndex_ is year/month/day information, for example month [0, 11] is Equivalent to [1, 12]
        return currentIndex_;
    }

    void SetCurrentIndex(uint32_t value)
    {
        currentIndex_ = value;
    }

    double GetCurrentOffset() const
    {
        return deltaSize_;
    }

    void SetCurrentOffset(double deltaSize)
    {
        deltaSize_ = deltaSize;
    }

    const std::map<RefPtr<FrameNode>, std::vector<std::string>>& GetOptions() const
    {
        return options_;
    }

    void SetOptions(const std::map<RefPtr<FrameNode>, std::vector<std::string>>& value)
    {
        options_ = value;
    }

    uint32_t GetShowCount() const
    {
        return showCount_;
    }

    void SetShowCount(const uint32_t showCount)
    {
        showCount_ = showCount;
    }

    void HandleChangeCallback(bool isAdd, bool needNotify)
    {
        if (changeCallback_) {
            changeCallback_(GetHost(), isAdd, GetCurrentIndex(), needNotify);
        } else {
            LOGE("change callback is null.");
        }
    }

    const ColumnChangeCallback& GetChangeCallback() const
    {
        return changeCallback_;
    }

    void SetChangeCallback(ColumnChangeCallback&& value)
    {
        changeCallback_ = value;
    }

    void HandleEventCallback(bool refresh)
    {
        if (EventCallback_) {
            EventCallback_(refresh);
        } else {
            LOGE("event callback is null.");
        }
    }

    const EventCallback& GetEventCallback() const
    {
        return EventCallback_;
    }

    void SetEventCallback(EventCallback&& value)
    {
        EventCallback_ = value;
    }

    const RefPtr<TossAnimationController>& GetToss() const
    {
        return tossAnimationController_;
    }

    void SetLocalDownDistance(float value)
    {
        localDownDistance_ = value;
    }

    float GetLocalDownDistance() const
    {
        return localDownDistance_;
    }

    void UpdateToss(double offsetY);

    void TossStoped();

    void UpdateScrollDelta(double delta);

private:
    void OnModifyDone() override;
    void OnAttachToFrameNode() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    void SetDividerHeight(uint32_t showOptionCount);
    void OnTouchDown();
    void OnTouchUp();
    void InitMouseAndPressEvent();
    void HandleMouseEvent(bool isHover);
    void SetButtonBackgroundColor(const Color& pressColor);
    void PlayPressAnimation(const Color& pressColor);
    void PlayHoverAnimation(const Color& color);

    void InitPanEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleDragStart(const GestureEvent& event);
    void HandleDragMove(const GestureEvent& event);
    void HandleDragEnd();
    void CreateAnimation();
    RefPtr<CurveAnimation<double>> CreateAnimation(double from, double to);
    void HandleCurveStopped();
    void ScrollOption(double delta, bool isJump = false);
    void UpdatePickerTextProperties(uint32_t index, uint32_t showOptionCount,
        const RefPtr<TextLayoutProperty>& textLayoutProperty,
        const RefPtr<DataPickerRowLayoutProperty>& dataPickerRowLayoutProperty);
    void UpdateDisappearTextProperties(const RefPtr<PickerTheme>& pickerTheme,
        const RefPtr<TextLayoutProperty>& textLayoutProperty,
        const RefPtr<DataPickerRowLayoutProperty>& timePickerLayoutProperty);
    void UpdateCandidateTextProperties(const RefPtr<PickerTheme>& pickerTheme,
        const RefPtr<TextLayoutProperty>& textLayoutProperty,
        const RefPtr<DataPickerRowLayoutProperty>& timePickerLayoutProperty);
    void UpdateSelectedTextProperties(const RefPtr<PickerTheme>& pickerTheme,
        const RefPtr<TextLayoutProperty>& textLayoutProperty,
        const RefPtr<DataPickerRowLayoutProperty>& timePickerLayoutProperty);
    void AddAnimationTextProperties(uint32_t currentIndex, const RefPtr<TextLayoutProperty>& textLayoutProperty);
    void UpdateTextPropertiesLinear(bool isDown, double scale);
    void TextPropertiesLinearAnimation(const RefPtr<TextLayoutProperty>& textLayoutProperty,
        uint32_t index, uint32_t showCount, bool isDown, double scale);
    void FlushAnimationTextProperties(bool isDown);
    Dimension LinearFontSize(const Dimension& startFontSize, const Dimension& endFontSize, double percent);
    void SetAccessibilityAction();
    float localDownDistance_ = 0.0f;
    RefPtr<TouchEventImpl> touchListener_;
    RefPtr<InputEvent> mouseEvent_;
    std::map<RefPtr<FrameNode>, std::vector<std::string>> options_;
    ColumnChangeCallback changeCallback_;
    EventCallback EventCallback_;
    uint32_t currentIndex_ = 0;
    int32_t currentChildIndex_ = 0;
    double yLast_ = 0.0;
    double yOffset_ = 0.0;
    double jumpInterval_;
    uint32_t showCount_ = 0;
    float gradientHeight_;
    float dividerHeight_;
    float dividerSpacingWidth_;
    Color pressColor_;
    Color hoverColor_;

    double deltaSize_ = 0.0;
    RefPtr<PanEvent> panEvent_;
    bool pressed_ = false;
    bool hoverd_ = false;
    double scrollDelta_ = 0.0;
    bool animationCreated_ = false;
    RefPtr<Animator> toController_;
    RefPtr<Animator> fromController_;
    RefPtr<CurveAnimation<double>> fromBottomCurve_;
    RefPtr<CurveAnimation<double>> fromTopCurve_;
    RefPtr<TossAnimationController> tossAnimationController_ = AceType::MakeRefPtr<TossAnimationController>();
    std::vector<DateTextProperties> animationProperties_;
    bool isJump_ = false;

    ACE_DISALLOW_COPY_AND_MOVE(DatePickerColumnPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_DATE_PICKER_DATE_PICKER_COLUMN_PATTERN_H
