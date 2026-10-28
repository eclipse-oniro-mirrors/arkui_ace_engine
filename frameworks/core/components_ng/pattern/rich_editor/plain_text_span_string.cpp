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

#include "core/components_ng/pattern/rich_editor/plain_text_span_string.h"

#include "base/utils/utils.h"
#include "core/components_ng/pattern/text/span_node.h"

namespace OHOS::Ace {

PlainTextSpanString::PlainTextSpanString(const RefPtr<SpanString>& src) : MutableSpanString(u"")
{
    CHECK_NULL_VOID(src);
    text_ = src->GetU16string();
    spans_ = src->GetSpanItems();
    spansMap_ = src->GetSpansMap();
    NormalizeSingleSpan();
}

PlainTextSpanString::~PlainTextSpanString() = default;

void PlainTextSpanString::NotifySpanWatcher()
{
    NormalizeSingleSpan();
    MutableSpanString::NotifySpanWatcher();
}

void PlainTextSpanString::ReplaceString(int32_t start, int32_t length, const std::u16string& other)
{
    MutableSpanString::ReplaceString(start, length, other);
    NormalizeSingleSpan();
}

void PlainTextSpanString::ClearAllSpans()
{
    MutableSpanString::ClearAllSpans();
    NormalizeSingleSpan();
}

void PlainTextSpanString::NormalizeSingleSpan()
{
    auto fullText = GetU16string();
    if (spans_.empty()) {
        spans_.push_back(GetDefaultSpanItem(fullText));
    } else {
        auto length = static_cast<int32_t>(fullText.length());
        auto head = spans_.front();
        head->UpdateContent(fullText);
        head->interval = { 0, length };
        auto it = spans_.begin();
        ++it;
        spans_.erase(it, spans_.end());
    }

    UpdateSpansMap();
}

} // namespace OHOS::Ace
