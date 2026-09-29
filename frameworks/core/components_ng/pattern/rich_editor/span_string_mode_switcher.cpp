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

#include "core/components_ng/pattern/text/span_node.h"
#include "span_string_mode_switcher.h"

namespace OHOS::Ace {

void SpanStringModeSwitcher::SwitchToPlainText(RefPtr<MutableSpanString>& styledString)
{
    CHECK_NULL_VOID(styledString);
    auto plainStr = AceType::MakeRefPtr<PlainTextSpanString>(AceType::DynamicCast<SpanString>(styledString));
    styledString = AceType::DynamicCast<MutableSpanString>(plainStr);
}

void SpanStringModeSwitcher::SwitchToMutable(RefPtr<MutableSpanString>& styledString)
{
    CHECK_NULL_VOID(styledString);
    auto rich = AceType::MakeRefPtr<MutableSpanString>(u"");
    rich->SetString(styledString->GetU16string());
    auto spanItems = styledString->GetSpanItems();
    rich->SetSpanItems(std::move(spanItems));
    auto spansMap = styledString->GetSpansMap();
    rich->SetSpanMap(std::move(spansMap));
    styledString = rich;
}

} // namespace OHOS::Ace
