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

#include "core/common/ime/text_input_keyboard_utils.h"

#include "base/utils/utils.h"

#if defined(ENABLE_STANDARD_INPUT)
#include "input_method_controller.h"
#endif

namespace OHOS::Ace {

void NotifyInputMethodConfigChange(TextInputAction action, TextInputType keyboard)
{
#if defined(ENABLE_STANDARD_INPUT)
    auto inputMethod = MiscServices::InputMethodController::GetInstance();
    CHECK_NULL_VOID(inputMethod);
    MiscServices::Configuration config;
    config.SetEnterKeyType(static_cast<MiscServices::EnterKeyType>(action));
    config.SetTextInputType(static_cast<MiscServices::TextInputType>(keyboard));
    inputMethod->OnConfigurationChange(config);
#endif
}

} // namespace OHOS::Ace