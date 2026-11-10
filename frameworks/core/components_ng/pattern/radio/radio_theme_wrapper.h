/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_RADIO_RADIO_THEME_WRAPPER_H
#define FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_RADIO_RADIO_THEME_WRAPPER_H

#include <memory>

#include "base/memory/ace_type.h"
#include "core/components/checkable/checkable_theme.h"
#include "core/components_ng/token_theme/token_theme.h"

namespace OHOS::Ace::NG {

class RadioThemeWrapper : public RadioTheme {
    DECLARE_ACE_TYPE(RadioThemeWrapper, RadioTheme);

public:
    class WrapperBuilder : public Builder {
    public:
        WrapperBuilder() = default;
        ~WrapperBuilder() = default;

        RefPtr<Theme> BuildWrapper(const RefPtr<ThemeConstants>& themeConstants) const
        {
            auto wrapper = AceType::MakeRefPtr<RadioThemeWrapper>();
            if (!themeConstants) {
                return wrapper;
            }
            ParsePattern(themeConstants, AceType::DynamicCast<RadioTheme>(wrapper));
            return wrapper;
        }
    };

    ~RadioThemeWrapper() override = default;

    void ApplyTokenTheme(const TokenTheme& theme) override
    {
        if (auto colors = theme.Colors(); colors) {
            // CheckedBackgroundColor in RadioModel
            activeColor_ = colors->CompBackgroundEmphasize();
            // UncheckedBorderColor in RadioModel
            inactiveColor_ = colors->IconFourth();
            // IndicatorColor in RadioModel
            pointColor_ = colors->IconOnPrimary();
            // UncheckedBorderColor (Radio-specific)
            uncheckBorderColor_ = colors->IconFourth();
        }
    }

protected:
    RadioThemeWrapper() = default;
};

} // namespace
#endif
