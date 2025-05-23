/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/text/span/span_string.h"
#include "core/text/html_utils.h"

namespace OHOS::Ace {

// Currently not completed
std::string HtmlUtils::ToHtml(const SpanString* str)
{
    return "";
}

std::string HtmlUtils::ToHtml(const std::list<RefPtr<NG::SpanItem>>& spanItems)
{
    return "";
}

std::string HtmlUtils::ToHtmlForNormalType(const NG::FontStyle& fontStyle,
    const NG::TextLineStyle& textLineStyle, const std::u16string& contentStr)
{
    return "";
}

} // namespace OHOS::Ace
