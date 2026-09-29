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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_IME_PASSWORD_OBSCURE_HELPER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_IME_PASSWORD_OBSCURE_HELPER_H

#include <cstdint>
#include <string>

#include "core/common/ime/text_input_obscure_utils.h"
#include "core/common/ime/text_input_type.h"

namespace OHOS::Ace {

class PasswordObscureHelper {
public:
    PasswordObscureHelper() = default;

    void UpdateObscure(TextInputType inputType, const std::u16string& insertValue,
        bool hasInsert, const std::u16string& content, int32_t caretPosition)
    {
        auto [tickCountDown, nakedCharPosition] = TextInputObscureUtils::UpdateObscureState(
            inputType, insertValue, hasInsert, content, caretPosition);
        tickCountDown_ = tickCountDown;
        nakedCharPosition_ = nakedCharPosition;
    }

    bool ResetTickCountDown()
    {
        CHECK_NULL_RETURN(tickCountDown_ > 0, false);
        tickCountDown_ = 0;
        return true;
    }

    void ClearState()
    {
        tickCountDown_ = 0;
        nakedCharPosition_ = -1;
    }

    void TickDown()
    {
        if (tickCountDown_ > 0) {
            --tickCountDown_;
        }
    }

    int32_t GetNakedCharPosition(bool hasContent) const
    {
        return hasContent ? nakedCharPosition_ : -1;
    }

    int32_t GetTickCountDown() const { return tickCountDown_; }

private:
    int32_t tickCountDown_ = 0;
    int32_t nakedCharPosition_ = -1;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_IME_PASSWORD_OBSCURE_HELPER_H
