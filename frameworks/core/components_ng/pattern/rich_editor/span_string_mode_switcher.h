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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_RICH_EDITOR_SPAN_STRING_MODE_SWITCHER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_RICH_EDITOR_SPAN_STRING_MODE_SWITCHER_H

#include <string>
#include "core/components_ng/pattern/text/span/span_object.h"

#include "base/memory/ace_type.h"
#include "core/common/ime/text_input_filter.h"
#include "core/common/ime/text_input_type.h"
#include "core/components_ng/pattern/text/span/mutable_span_string.h"

namespace OHOS::Ace {

/**
 * @brief Helper for switching styledString_ between PlainTextSpanString and MutableSpanString.
 */
class SpanStringModeSwitcher {
public:
    SpanStringModeSwitcher() = default;
    ~SpanStringModeSwitcher() = default;

    void SetLastPlainMode(bool val) { lastPlainMode_ = val; }
    bool ShouldSwitch(bool currentPlain) const { return currentPlain != lastPlainMode_; }

    static constexpr bool IsPlainTextInputType(TextInputType inputType)
    {
        return inputType != TextInputType::UNSPECIFIED && inputType != TextInputType::MULTILINE;
    }

    static RefPtr<SpanString> FilterSpanString(const RefPtr<SpanString>& src, TextInputType keyboard)
    {
        CHECK_NULL_RETURN(src, src);
        auto filtered = src->GetU16string();
        TextInputFilter::FilterByInputType(keyboard, filtered);
        // Always create a new plain SpanString to strip any styling from the source,
        // ensuring pasted/dragged content conforms to the single-style plain text mode.
        return AceType::MakeRefPtr<SpanString>(filtered);
    }

    void SwitchToPlainText(RefPtr<MutableSpanString>& styledString);
    void SwitchToMutable(RefPtr<MutableSpanString>& styledString);

private:
    bool lastPlainMode_ = false;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_RICH_EDITOR_SPAN_STRING_MODE_SWITCHER_H
