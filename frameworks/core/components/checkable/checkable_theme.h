/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_CHECKABLE_CHECKABLE_THEME_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_CHECKABLE_CHECKABLE_THEME_H

#include "base/utils/system_properties.h"
#include "core/common/container.h"
#include "core/components/theme/theme.h"
#include "core/components/theme/theme_constants.h"
#include "core/components/theme/theme_constants_defines.h"

namespace OHOS::Ace {

class CheckableTheme : public virtual Theme {
    DECLARE_ACE_TYPE(CheckableTheme, Theme);

public:
    ~CheckableTheme() override = default;

    const Color& GetPointColor() const
    {
        return pointColor_;
    }
    const Color& GetActiveColor() const
    {
        return activeColor_;
    }
    const Color& GetInactiveColor() const
    {
        return inactiveColor_;
    }
    const Color& GetFocusColor() const
    {
        return focusColor_;
    }
    const Color& GetFocusBoardColor() const
    {
        return focusBoardColor_;
    }
    const Color& GetBorderFocusedColor() const
    {
        return borderFocusedColor_;
    }
    const Color& GetFocusedBGColorUnselected() const
    {
        return focusedBGColorUnselected_;
    }
    const Dimension& GetWidth() const
    {
        return width_;
    }
    const Dimension& GetHeight() const
    {
        return height_;
    }
    const Dimension& GetHotZoneHorizontalPadding() const
    {
        return hotZoneHorizontalPadding_;
    }
    const Dimension& GetHotZoneVerticalPadding() const
    {
        return hotZoneVerticalPadding_;
    }
    double GetAspectRatio() const
    {
        return aspectRatio_;
    }
    const Dimension& GetDefaultWidth() const
    {
        return defaultWidth_;
    }
    const Dimension& GetDefaultHeight() const
    {
        return defaultHeight_;
    }
    double GetRadioInnerSizeRatio() const
    {
        return radioInnerSizeRatio_;
    }
    bool GetNeedFocus() const
    {
        return needFocus_;
    }
    bool IsBackgroundSolid() const
    {
        return backgroundSolid_;
    }
    const Dimension& GetBorderWidth() const
    {
        return borderWidth_;
    }

    void SetHoverColor(const Color& hoverColor)
    {
        hoverColor_ = hoverColor;
    }

    const Color& GetHoverColor() const
    {
        return hoverColor_;
    }

    void SetClickEffectColor(const Color& clickEffectColor)
    {
        clickEffectColor_ = clickEffectColor;
    }

    const Color& GetClickEffectColor() const
    {
        return clickEffectColor_;
    }

    const Color& GetInactivePointColor() const
    {
        return inactivePointColor_;
    }

    const Color& GetShadowColor() const
    {
        return shadowColor_;
    }

    const Dimension& GetShadowWidth() const
    {
        return shadowWidth_;
    }

    const Dimension& GetHoverRadius() const
    {
        return hoverRadius_;
    }

    const Dimension& GetDefaultPaddingSize() const
    {
        return defaultPaddingSize_;
    }

    const Dimension& GetFocusRadius() const
    {
        return focusRadius_;
    }

    const Dimension& GetFocusPaintPadding() const
    {
        return focusPaintPadding_;
    }

    const Dimension& GetFocusBoardSize() const
    {
        return focusBoardSize_;
    }

    double GetHoverDuration() const
    {
        return hoverDuration_;
    }

    double GetHoverToTouchDuration() const
    {
        return hoverToTouchDuration_;
    }

    double GetTouchDuration() const
    {
        return touchDuration_;
    }
    const InternalResource::ResourceId& GetTickResourceId() const
    {
        return tickResourceId_;
    }
    const InternalResource::ResourceId& GetDotResourceId() const
    {
        return dotResourceId_;
    }

    const Color& GetFocusedBgColor() const
    {
        return focusedBgColor_;
    }

    const Dimension& GetSizeFocusBg() const
    {
        return sizeFocusBg_;
    }

    const Color& GetFocusedRingUnchecked() const
    {
        return focusedRingUnchecked_;
    }

    const Color& GetFocusedBgUnchecked() const
    {
        return focusedBgUnchecked_;
    }

protected:
    CheckableTheme() = default;

    Color pointColor_;
    Color activeColor_;
    Color inactiveColor_;
    Color inactivePointColor_;
    Color focusColor_;
    Color hoverColor_;
    Color focusedRingUnchecked_;
    Color focusedBgUnchecked_;
    Color clickEffectColor_;
    Color shadowColor_;
    Color focusBoardColor_;
    Color borderFocusedColor_;
    Color focusedBGColorUnselected_;
    Color focusedBgColor_;
    Dimension sizeFocusBg_;
    Dimension width_;
    Dimension height_;
    Dimension hotZoneHorizontalPadding_;
    Dimension hotZoneVerticalPadding_;
    Dimension defaultWidth_;
    Dimension defaultHeight_;
    Dimension borderWidth_;
    Dimension shadowWidth_;
    Dimension hoverRadius_;
    Dimension defaultPaddingSize_;
    Dimension focusRadius_;
    Dimension focusPaintPadding_;
    Dimension focusBoardSize_;
    double hoverDuration_ = 0.0f;
    double hoverToTouchDuration_ = 0.0f;
    double touchDuration_ = 0.0f;
    double aspectRatio_ = 1.0;
    double radioInnerSizeRatio_ = 0.5;
    bool needFocus_ = true;
    bool backgroundSolid_ = true;
    const float ratio_ = 1.8f;
    InternalResource::ResourceId tickResourceId_ = InternalResource::ResourceId::RADIO_TICK_SVG;
    InternalResource::ResourceId dotResourceId_ = InternalResource::ResourceId::RADIO_DOT_SVG;
};

class CheckboxTheme : public CheckableTheme {
    DECLARE_ACE_TYPE(CheckboxTheme, CheckableTheme);

public:
    class Builder {
    public:
        Builder() = default;
        ~Builder() = default;

        RefPtr<CheckboxTheme> Build(const RefPtr<ThemeConstants>& themeConstants) const
        {
            RefPtr<CheckboxTheme> theme = AceType::Claim(new CheckboxTheme());
            if (!themeConstants) {
                LOGI("Build AppTheme error, themeConstants is null!");
                return theme;
            }
            ParsePattern(themeConstants, theme);
            return theme;
        }

    private:
        void ParsePattern(const RefPtr<ThemeConstants>& themeConstants, const RefPtr<CheckboxTheme>& theme) const
        {
            RefPtr<ThemeStyle> checkboxPattern = themeConstants->GetPatternByName(THEME_PATTERN_CHECKBOX);
            if (!checkboxPattern) {
                LOGE("Pattern of checkbox is null, please check!");
                return;
            }
            theme->width_ = checkboxPattern->GetAttr<Dimension>("checkbox_size", 0.0_vp);
            theme->height_ = theme->width_;
            theme->hotZoneHorizontalPadding_ = checkboxPattern->GetAttr<Dimension>("checkbox_hotzone_padding", 0.0_vp);
            theme->defaultWidth_ = checkboxPattern->GetAttr<Dimension>("checkbox_default_size", 0.0_vp);
            if (Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
                theme->defaultPaddingSize_ =
                    checkboxPattern->GetAttr<Dimension>("checkbox_default_padding_size", 2.0_vp);
                theme->defaultWidth_ = checkboxPattern->GetAttr<Dimension>("checkbox_default_size_twelve", 24.0_vp);
            }
            theme->hotZoneVerticalPadding_ = theme->hotZoneHorizontalPadding_;
            theme->defaultHeight_ = theme->defaultWidth_;
            theme->needFocus_ = static_cast<bool>(checkboxPattern->GetAttr<double>("checkbox_need_focus", 0.0));
            theme->backgroundSolid_ =
                static_cast<bool>(checkboxPattern->GetAttr<double>("checkbox_inactive_background_solid", 0.0));
            theme->borderWidth_ = checkboxPattern->GetAttr<Dimension>("checkbox_border_width", 0.0_vp);
            theme->checkStroke_ = checkboxPattern->GetAttr<Dimension>("checkbox_stroke_width", 0.0_vp);
            theme->shadowColor_ = checkboxPattern->GetAttr<Color>("checkbox_shadow_color", Color());
            theme->shadowWidth_ = checkboxPattern->GetAttr<Dimension>("checkbox_shadow_width", 0.0_vp);
            theme->pointColor_ = checkboxPattern->GetAttr<Color>("fg_color_checked", Color::RED);
            theme->activeColor_ = checkboxPattern->GetAttr<Color>("bg_color_checked", Color::RED);
            theme->inactiveColor_ = checkboxPattern->GetAttr<Color>("bg_border_color_unchecked", Color::RED);
            theme->focusColor_ = checkboxPattern->GetAttr<Color>("focus_border_color", Color::RED);
            theme->focusBoardColor_ = checkboxPattern->GetAttr<Color>("color_focused_bg", Color::RED);
            theme->focusBoardSize_ = checkboxPattern->GetAttr<Dimension>("size_focused_bg", 2.0_vp);
            theme->borderFocusedColor_ = checkboxPattern->GetAttr<Color>("focused_border_color", Color::RED);
            theme->focusedBGColorUnselected_ =
                checkboxPattern->GetAttr<Color>("focused_bg_color_unselected", Color::RED);
            theme->borderRadius_ = checkboxPattern->GetAttr<Dimension>("bg_border_radius", 0.0_vp);
            theme->hoverColor_ = checkboxPattern->GetAttr<Color>("hover_border_color", Color::RED);
            theme->clickEffectColor_ = checkboxPattern->GetAttr<Color>("click_effect_color", Color::RED);
            theme->inactivePointColor_ = checkboxPattern->GetAttr<Color>("bg_color_unchecked", Color::RED);
            theme->hoverRadius_ = checkboxPattern->GetAttr<Dimension>("hover_border_radius", 0.0_vp);
            theme->focusRadius_ = checkboxPattern->GetAttr<Dimension>("focus_border_radius", 0.0_vp);
            theme->focusPaintPadding_ = checkboxPattern->GetAttr<Dimension>("focus_paint_padding", 0.0_vp);
            theme->hoverDuration_ = checkboxPattern->GetAttr<double>("hover_animation_duration", 0.0);
            theme->hoverToTouchDuration_ = checkboxPattern->GetAttr<double>("hover_to_press_animation_duration", 0.0);
            theme->touchDuration_ = checkboxPattern->GetAttr<double>("touch_animation_duration", 0.0);
            theme->colorAnimationDuration_ = checkboxPattern->GetAttr<double>("color_animation_duration", 0.0);

            if (SystemProperties::GetDeviceType() != DeviceType::CAR) {
                return;
            }
            SetCheckboxSize(themeConstants, theme);
        }

        void SetCheckboxSize(const RefPtr<ThemeConstants>& themeConstants, const RefPtr<CheckboxTheme>& theme) const
        {
            RefPtr<ThemeStyle> checkboxPattern = themeConstants->GetPatternByName(THEME_PATTERN_CHECKBOX);
            // width/height/borderRadius not exist in theme
            theme->width_ = checkboxPattern->GetAttr<Dimension>("width", 26.0_vp);
            theme->height_ = theme->width_;
            theme->borderRadius_ = checkboxPattern->GetAttr<Dimension>("hover_border_radius", 4.0_vp);
            theme->hotZoneHorizontalPadding_ =
                checkboxPattern->GetAttr<Dimension>("hotzone_padding_horizontal", 11.0_vp);
            theme->hotZoneVerticalPadding_ = theme->hotZoneHorizontalPadding_;
        }
    };

    const Dimension& GetBorderRadius() const
    {
        return borderRadius_;
    }

    const Dimension& GetCheckStroke() const
    {
        return checkStroke_;
    }

    double GetColorAnimationDuration() const
    {
        return colorAnimationDuration_;
    }

private:
    Dimension borderRadius_;
    Dimension checkStroke_;
    double colorAnimationDuration_ = 0.0;
};

class SwitchTheme : public CheckableTheme {
    DECLARE_ACE_TYPE(SwitchTheme, CheckableTheme);

public:
    class Builder {
    public:
        Builder() = default;
        ~Builder() = default;

        RefPtr<SwitchTheme> Build(const RefPtr<ThemeConstants>& themeConstants) const
        {
            RefPtr<SwitchTheme> theme = AceType::Claim(new SwitchTheme());
            if (!themeConstants) {
                LOGE("Build AppTheme error, themeConstants is null!");
                return theme;
            }
            ParsePattern(themeConstants, theme);
            ParseSubStylePattern(themeConstants, theme);
            return theme;
        }

    private:
        void ParseSubStylePattern(const RefPtr<ThemeConstants>& themeConstants,
            const RefPtr<SwitchTheme>& theme) const;
        void ParsePattern(const RefPtr<ThemeConstants>& themeConstants, const RefPtr<SwitchTheme>& theme) const;
    };

    const Color& GetPointColorUnselectedFocus() const
    {
        return focusedPointColorUnselected_;
    }

    const Dimension& GetFocusBoardWidth() const
    {
        return focusBoardWidth_;
    }

    const Dimension& GetFocusBoardHeight() const
    {
        return focusBoardHeight_;
    }

    const Dimension& GetFocusBoardRadius() const
    {
        return focusBoardRadius_;
    }

    float GetRatio() const
    {
        return ratio_;
    }

    double GetColorAnimationDuration() const
    {
        return colorAnimationDuration_;
    }

    double GetPointAnimationDuration() const
    {
        return pointAnimationDuration_;
    }

private:
    double colorAnimationDuration_ = 0.0;
    double pointAnimationDuration_ = 0.0;
    Color focusedPointColorUnselected_;
    Dimension focusBoardWidth_;
    Dimension focusBoardHeight_;
    Dimension focusBoardRadius_;
};

class RadioTheme : public CheckableTheme {
    DECLARE_ACE_TYPE(RadioTheme, CheckableTheme);

public:
    class Builder {
    public:
        Builder() = default;
        ~Builder() = default;

        RefPtr<RadioTheme> Build(const RefPtr<ThemeConstants>& themeConstants) const
        {
            RefPtr<RadioTheme> theme = AceType::Claim(new RadioTheme());
            if (!themeConstants) {
                return theme;
            }
            ParseNewPattern(themeConstants, theme);
            ParsePattern(themeConstants, theme);
            return theme;
        }

    private:
        void ParsePattern(const RefPtr<ThemeConstants>& themeConstants, const RefPtr<RadioTheme>& theme) const
        {
            RefPtr<ThemeStyle> radioPattern = themeConstants->GetPatternByName(THEME_PATTERN_RADIO);
            if (!radioPattern) {
                LOGW("find pattern of radio fail");
                return;
            }
            theme->width_ = radioPattern->GetAttr<Dimension>("radio_size", 0.0_vp);
            theme->height_ = theme->width_;
            theme->hotZoneHorizontalPadding_ = radioPattern->GetAttr<Dimension>("radio_hotzone_padding", 0.0_vp);
            theme->hotZoneVerticalPadding_ = theme->hotZoneHorizontalPadding_;
            theme->defaultWidth_ = radioPattern->GetAttr<Dimension>("radio_default_size", 0.0_vp);
            theme->defaultHeight_ = theme->defaultWidth_;
            if (Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
                theme->width_ = radioPattern->GetAttr<Dimension>("radio_size_api_twelve", 24.0_vp);
                theme->height_ = theme->width_;
                theme->defaultPaddingSize_ = radioPattern->GetAttr<Dimension>("radio_default_padding_size", 2.0_vp);
                theme->defaultWidth_ = radioPattern->GetAttr<Dimension>("radio_default_size_api_twelve", 24.0_vp);
                theme->defaultHeight_ = theme->defaultWidth_;
            }
            theme->radioInnerSizeRatio_ = radioPattern->GetAttr<double>("radio_inner_size_ratio", 0.0);
            theme->needFocus_ = static_cast<bool>(radioPattern->GetAttr<double>("radio_need_focus", 0.0));
            theme->backgroundSolid_ =
                static_cast<bool>(radioPattern->GetAttr<double>("radio_inactive_background_solid", 0.0));
            theme->borderWidth_ = radioPattern->GetAttr<Dimension>("radio_border_width", 0.0_vp);
            theme->shadowColor_ = radioPattern->GetAttr<Color>("radio_shadow_color", Color());
            theme->shadowWidth_ = radioPattern->GetAttr<Dimension>("radio_shadow_width", 0.0_vp);
            theme->pointColor_ = radioPattern->GetAttr<Color>("fg_color_checked", Color::RED);
            theme->activeColor_ = radioPattern->GetAttr<Color>("bg_color_checked", Color::RED);
            theme->focusedRingUnchecked_ = radioPattern->GetAttr<Color>("focused_ring_unchecked", Color::TRANSPARENT);
            theme->focusedBgUnchecked_ = radioPattern->GetAttr<Color>("focused_bg_unchecked", Color::TRANSPARENT);
            theme->inactiveColor_ = radioPattern->GetAttr<Color>("bg_color_unchecked", Color::RED);
            theme->inactivePointColor_ = radioPattern->GetAttr<Color>("fg_color_unchecked", Color::RED);
            theme->focusColor_ = radioPattern->GetAttr<Color>("bg_focus_outline_color", Color::RED);
            theme->hoverColor_ = radioPattern->GetAttr<Color>("hover_border_color", Color::RED);
            theme->clickEffectColor_ = radioPattern->GetAttr<Color>("click_effect_color", Color::RED);
            theme->focusPaintPadding_ = radioPattern->GetAttr<Dimension>("focus_paint_padding", 0.0_vp);
            theme->hoverDuration_ = radioPattern->GetAttr<double>("hover_animation_duration", 0.0);
            theme->hoverToTouchDuration_ = radioPattern->GetAttr<double>("hover_to_press_animation_duration", 0.0);
            theme->touchDuration_ = radioPattern->GetAttr<double>("touch_animation_duration", 0.0);
            if (SystemProperties::GetDeviceType() != DeviceType::CAR) {
                return;
            }
            theme->width_ = radioPattern->GetAttr<Dimension>(RADIO_WIDTH, 26.0_vp);
            theme->height_ = theme->width_;
            theme->hotZoneHorizontalPadding_ = radioPattern->GetAttr<Dimension>(RADIO_PADDING, 11.0_vp);
            theme->hotZoneVerticalPadding_ = theme->hotZoneHorizontalPadding_;
            SetRadioSize(themeConstants, theme);
        }

        void SetRadioSize(const RefPtr<ThemeConstants>& themeConstants, const RefPtr<RadioTheme>& theme) const
        {
            RefPtr<ThemeStyle> radioPattern = themeConstants->GetPatternByName(THEME_PATTERN_RADIO);
            if (Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
                theme->width_ = radioPattern->GetAttr<Dimension>("radio_size_api_twelve", 24.0_vp);
                theme->height_ = theme->width_;
            }
        }

        void ParseNewPattern(const RefPtr<ThemeConstants>& themeConstants, const RefPtr<RadioTheme>& theme) const
        {
            RefPtr<ThemeStyle> radioPattern = themeConstants->GetPatternByName(THEME_PATTERN_RADIO);
            if (!radioPattern) {
                LOGW("find pattern of radio fail");
                return;
            }
            theme->focusedBgColor_ = radioPattern->GetAttr<Color>("color_focused_bg", Color::RED);
            theme->sizeFocusBg_ = radioPattern->GetAttr<Dimension>("size_focused_bg", 0.0_vp);
        }
    };
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_CHECKABLE_CHECKABLE_THEME_H
