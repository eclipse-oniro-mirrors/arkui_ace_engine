/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "core/common/ime/text_input_action.h"

namespace OHOS::Ace {

TextInputAction CastToTextInputAction(int32_t value)
{
    if (value < static_cast<int32_t>(TextInputAction::BEGIN) || value > static_cast<int32_t>(TextInputAction::END)) {
        return TextInputAction::UNSPECIFIED;
    }
    return static_cast<TextInputAction>(value);
}

AutoCapitalizationMode CastToAutoCapitalizationMode(int32_t value)
{
    if (value< static_cast<int32_t>(AutoCapitalizationMode::NONE) ||
        value > static_cast<int32_t>(AutoCapitalizationMode::ALL_CHARACTERS)) {
        return AutoCapitalizationMode::NONE;
    }
    return static_cast<AutoCapitalizationMode>(value);
}
} // namespace OHOS::Ace
