/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "core/common/ime/text_input_obscure_utils.h"

#include <algorithm>
#include <cctype>

#include "base/i18n/localization.h"
#include "core/common/container.h"

namespace OHOS::Ace {

std::u16string TextInputObscureUtils::CreateObscuredText(int32_t len)
{
    return std::u16string(len, GetObscuringCharacter());
}

std::u16string TextInputObscureUtils::CreateDisplayText(
    const std::u16string& content, int32_t nakedCharPosition, bool needObscureText, bool showPasswordDirectly)
{
    if (!content.empty() && needObscureText) {
        auto text = CreateObscuredText(static_cast<int32_t>(content.length()));
        if (nakedCharPosition >= 0 && nakedCharPosition < static_cast<int32_t>(content.length())) {
            if (Container::LessThanAPITargetVersion(PlatformVersion::VERSION_TWELVE) || !showPasswordDirectly) {
                text[nakedCharPosition] = content[nakedCharPosition];
            }
        }
        return text;
    }
    return content;
}

char16_t TextInputObscureUtils::GetObscuringCharacter()
{
    return Localization::GetInstance()->GetLanguage() == "ar" ? OBSCURING_CHARACTER_FOR_AR : OBSCURING_CHARACTER;
}

TextInputObscureUtils::ObscureUpdateResult TextInputObscureUtils::UpdateObscureState(
    TextInputType inputType, const std::u16string& insertValue, bool hasInsert,
    const std::u16string& content, int32_t caretPosition)
{
    ObscureUpdateResult result;
    if (insertValue.length() == 1 &&
        (inputType != TextInputType::NUMBER_PASSWORD || std::isdigit(insertValue[0])) && hasInsert) {
        auto insertIndex = std::clamp(caretPosition - 1, 0, static_cast<int32_t>(content.length()));
        auto strBeforeCaret = content.empty() ? u"" : content.substr(insertIndex, 1);
        bool match = (strBeforeCaret == insertValue);
        result.tickCountDown = match ? static_cast<int32_t>(OBSCURE_SHOW_TICKS) : 0;
        result.nakedCharPosition = match ? insertIndex : -1;
    }
    return result;
}

} // namespace OHOS::Ace
