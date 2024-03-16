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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_APP_BAR_THEME_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_APP_BAR_THEME_H

#include <optional>

#include "base/geometry/dimension.h"
#include "base/geometry/ng/rect_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/color.h"
#include "core/components/theme/theme.h"
#include "core/components/theme/theme_attributes.h"
#include "core/components/theme/theme_constants.h"
#include "core/components/theme/theme_style.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

class AppBarTheme : public virtual Theme {
    DECLARE_ACE_TYPE(AppBarTheme, Theme);

public:
    ~AppBarTheme() = default;

    class Builder {
    public:
        Builder() = default;
        ~Builder() = default;
        RefPtr<AppBarTheme> Build(const RefPtr<ThemeConstants>& themeConstants)
        {
            RefPtr<AppBarTheme> theme = AceType::Claim(new AppBarTheme());
            if (!themeConstants) {
                LOGE("Build AppBarTheme error, themeConstants is null!");
                return theme;
            }
            ParsePattern(themeConstants, theme);
            return theme;
        }
    };

    const Dimension& GetAppBarHeight() const
    {
        return appBarHeight_;
    }

    const Dimension& GetIconSize() const
    {
        return iconSize_;
    }

    const Dimension& GetIconCornerRadius() const
    {
        return iconCornerRadius_;
    }

    const Dimension& GetFontSize() const
    {
        return appBarFontSize_;
    }

    const Color& GetTextColor() const
    {
        return textColorPrimary_;
    }

    const Color& GetBgColor() const
    {
        return appBarBgColor_;
    }

    const std::string& GetBundleName() const
    {
        return bundleName_;
    }

    const std::string& GetAbilityName() const
    {
        return abilityName_;
    }

    const std::string& GetStageAbilityName() const
    {
        return stageAbilityName_;
    }

    std::optional<RectF> GetAppBarRect() const
    {
        return std::nullopt;
    }

    const Color& GetIconColor() const
    {
        return iconColor_;
    }

    const Color& GetDividerColor() const
    {
        return dividerColor_;
    }

    const Color& GetBorderColor() const
    {
        return borderColor_;
    }

    const Color& GetHoverColor() const
    {
        return hoverColor_;
    }

    const Color& GetClickEffectColor() const
    {
        return clickEffectColor_;
    }

    const Color& GetFocusedOutlineColor() const
    {
        return focusedOutlineColor_;
    }

    const Dimension& GetBentRadius() const
    {
        return bentRadius_;
    }

    const Dimension& GetRightAngle() const
    {
        return rightAngle_;
    }

    const Dimension& GetMenuBarWidth() const
    {
        return menuBarWidth_;
    }

    const Dimension& GetMenuBarHeight() const
    {
        return menuBarHeight_;
    }

    const Dimension& GetNewIconSize() const
    {
        return newIconSize_;
    }

    const Dimension& GetDividerWidth() const
    {
        return dividerWidth_;
    }

    const Dimension& GetDividerHeight() const
    {
        return dividerHeight_;
    }

    const Dimension& GetBorderWidth() const
    {
        return borderWidth_;
    }

    const Dimension& GetButtonWidth() const
    {
        return buttonWidth_;
    }

    const Dimension& GetButtonHeight() const
    {
        return buttonHeight_;
    }

    const Dimension& GetFocusedOutlineWidth() const
    {
        return focusedOutlineWidth_;
    }

    const Dimension& GetMenuBarTopMargin() const
    {
        return menuBarTopMargin_;
    }

    const Dimension& GetMenuBarLeftMargin() const
    {
        return menuBarLeftMargin_;
    }

    const Dimension& GetMenuBarRightMargin() const
    {
        return menuBarRightMargin_;
    }

    const Dimension& GetIconOutsideMargin() const
    {
        return iconOutsideMargin_;
    }

    const Dimension& GetIconInsideMargin() const
    {
        return iconInsideMargin_;
    }

    const Dimension& GetIconVerticalMargin() const
    {
        return iconVerticalMargin_;
    }

private:
    AppBarTheme() = default;
    static void ParsePattern(const RefPtr<ThemeConstants>& themeConstants, const RefPtr<AppBarTheme>& theme)
    {
        RefPtr<ThemeStyle> appBarPattern = themeConstants->GetPatternByName(THEME_PATTERN_APP_BAR);
        if (!appBarPattern) {
            LOGE("Pattern of app bar is null, please check!");
            return;
        }
        theme->appBarHeight_ = appBarPattern->GetAttr<Dimension>("app_bar_height", 0.0_vp);
        theme->iconSize_ = appBarPattern->GetAttr<Dimension>("icon_size", 0.0_vp);
        theme->iconCornerRadius_ = appBarPattern->GetAttr<Dimension>("icon_corner_radius", 0.0_vp);
        theme->appBarFontSize_ = appBarPattern->GetAttr<Dimension>("app_bar_font_size", 0.0_vp);
        theme->textColorPrimary_ = appBarPattern->GetAttr<Color>("text_color_primary", Color());
        theme->appBarBgColor_ = appBarPattern->GetAttr<Color>("app_bar_bg_color", Color());
        theme->bundleName_ = appBarPattern->GetAttr<std::string>("app_bar_bundle_name", "com.ohos.hag.famanager");
        theme->abilityName_ = appBarPattern->GetAttr<std::string>("app_bar_ability_name", "FaPanelAbility");
        theme->stageAbilityName_ = appBarPattern->GetAttr<std::string>("app_bar_stage_ability_name", "PanelAbility");

        theme->iconColor_ = appBarPattern->GetAttr<Color>("icon_color", Color());
        theme->dividerColor_ = appBarPattern->GetAttr<Color>("divider_color", Color());
        theme->borderColor_ = appBarPattern->GetAttr<Color>("border_color", Color());
        theme->hoverColor_ = appBarPattern->GetAttr<Color>("hover_color", Color());
        theme->clickEffectColor_ = appBarPattern->GetAttr<Color>("click_effect_color", Color());
        theme->focusedOutlineColor_ = appBarPattern->GetAttr<Color>("focused_outline_color", Color());
        theme->bentRadius_ = appBarPattern->GetAttr<Dimension>("bent_radius", 18.0_vp);
        theme->rightAngle_ = appBarPattern->GetAttr<Dimension>("right_angle", 0.0_vp);
        theme->menuBarWidth_ = appBarPattern->GetAttr<Dimension>("menu_bar_width", 80.0_vp);
        theme->menuBarHeight_ = appBarPattern->GetAttr<Dimension>("menu_bar_height", 36.0_vp);
        theme->newIconSize_ = appBarPattern->GetAttr<Dimension>("new_icon_size", 20.0_vp);
        theme->dividerWidth_ = appBarPattern->GetAttr<Dimension>("divider_width", 1.0_px);
        theme->dividerHeight_ = appBarPattern->GetAttr<Dimension>("divider_height", 16.0_vp);
        theme->borderWidth_ = appBarPattern->GetAttr<Dimension>("border_width", 1.0_px);
        theme->buttonWidth_ = appBarPattern->GetAttr<Dimension>("button_width", 40.0_vp);
        theme->buttonHeight_ = appBarPattern->GetAttr<Dimension>("button_height", 36.0_vp);
        theme->focusedOutlineWidth_ = appBarPattern->GetAttr<Dimension>("focused_outline_width", 2.0_vp);
        theme->menuBarTopMargin_ = appBarPattern->GetAttr<Dimension>("menu_bar_top_margin", 10.0_vp);
        theme->menuBarLeftMargin_ = appBarPattern->GetAttr<Dimension>("menu_bar_left_margin", 12.0_vp);
        theme->menuBarRightMargin_ = appBarPattern->GetAttr<Dimension>("menu_bar_right_margin", 8.0_vp);
        theme->iconOutsideMargin_ = appBarPattern->GetAttr<Dimension>("icon_outside_margin", 12.0_vp);
        theme->iconInsideMargin_ = appBarPattern->GetAttr<Dimension>("icon_inside_margin", 8.0_vp);
        theme->iconVerticalMargin_ = appBarPattern->GetAttr<Dimension>("icon_vertical_margin", 8.0_vp);
    }

    Dimension appBarHeight_;
    Dimension iconSize_;
    Dimension iconCornerRadius_;
    Dimension appBarFontSize_;
    Color textColorPrimary_;
    Color appBarBgColor_;
    std::string bundleName_;
    std::string abilityName_;
    std::string stageAbilityName_;

    Color iconColor_;
    Color dividerColor_;
    Color borderColor_;
    Color hoverColor_;
    Color clickEffectColor_;
    Color focusedOutlineColor_;
    Dimension bentRadius_;
    Dimension rightAngle_;
    Dimension menuBarWidth_;
    Dimension menuBarHeight_;
    Dimension newIconSize_;
    Dimension dividerWidth_;
    Dimension dividerHeight_;
    Dimension borderWidth_;
    Dimension buttonWidth_;
    Dimension buttonHeight_;
    Dimension focusedOutlineWidth_;
    Dimension menuBarTopMargin_;
    Dimension menuBarLeftMargin_;
    Dimension menuBarRightMargin_;
    Dimension iconOutsideMargin_;
    Dimension iconInsideMargin_;
    Dimension iconVerticalMargin_;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_APP_BAR_THEME_H