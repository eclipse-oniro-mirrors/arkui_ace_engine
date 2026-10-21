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

#include "core/components_ng/pattern/rich_editor/rich_editor_accessibility_property.h"

#include "core/components_ng/pattern/common_text/counter_decorator.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"
#include "core/common/ime/text_input_type.h"

namespace OHOS::Ace::NG {

std::string RichEditorAccessibilityProperty::GetText() const
{
    auto frameNode = host_.Upgrade();
    CHECK_NULL_RETURN(frameNode, "");
    auto richEditorPattern = frameNode->GetPattern<RichEditorPattern>();
    CHECK_NULL_RETURN(richEditorPattern, "");
    if (richEditorPattern->IsPasswordObscured()) {
        auto content = richEditorPattern->GetObscureContent();
        return std::string(content.size(), '*');
    }
    return TextAccessibilityProperty::GetText();
}

bool RichEditorAccessibilityProperty::IsEditable() const
{
    return true;
}

bool RichEditorAccessibilityProperty::IsHint() const
{
    auto frameNode = host_.Upgrade();
    CHECK_NULL_RETURN(frameNode, false);
    auto layoutProperty = frameNode->GetLayoutProperty<RichEditorLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, false);
    auto pattern = frameNode->GetPattern<RichEditorPattern>();
    CHECK_NULL_RETURN(pattern, false);
    return pattern->GetTextContentLength() == 0 && !layoutProperty->GetPlaceholderValue(u"").empty();
}

std::string RichEditorAccessibilityProperty::GetHintText() const
{
    auto frameNode = host_.Upgrade();
    CHECK_NULL_RETURN(frameNode, "");
    auto richEditorPattern = frameNode->GetPattern<RichEditorPattern>();
    CHECK_NULL_RETURN(richEditorPattern, "");
    return richEditorPattern->GetPlaceHolder();
}

bool RichEditorAccessibilityProperty::IsPassword() const
{
    auto frameNode = host_.Upgrade();
    CHECK_NULL_RETURN(frameNode, false);
    auto richEditorPattern = frameNode->GetPattern<RichEditorPattern>();
    CHECK_NULL_RETURN(richEditorPattern, false);
    return richEditorPattern->IsPasswordObscured();
}

AceTextCategory RichEditorAccessibilityProperty::GetTextInputType() const
{
    auto frameNode = host_.Upgrade();
    CHECK_NULL_RETURN(frameNode, AceTextCategory::INPUT_TYPE_DEFAULT);
    auto richEditorPattern = frameNode->GetPattern<RichEditorPattern>();
    CHECK_NULL_RETURN(richEditorPattern, AceTextCategory::INPUT_TYPE_DEFAULT);
    auto textInputType = richEditorPattern->GetTextInputType();

    switch (textInputType) {
        case TextInputType::TEXT:
            return AceTextCategory::INPUT_TYPE_TEXT;
        case TextInputType::NUMBER:
        case TextInputType::NUMBER_DECIMAL:
        case TextInputType::ONE_TIME_CODE:
        case TextInputType::ONE_TIME_CODE_NUMBER:
            return AceTextCategory::INPUT_TYPE_NUMBER;
        case TextInputType::PHONE:
            return AceTextCategory::INPUT_TYPE_PHONENUMBER;
        case TextInputType::DATETIME:
            return AceTextCategory::INPUT_TYPE_DATE;
        case TextInputType::EMAIL_ADDRESS:
            return AceTextCategory::INPUT_TYPE_EMAIL;
        case TextInputType::VISIBLE_PASSWORD:
        case TextInputType::NUMBER_PASSWORD:
        case TextInputType::SCREEN_LOCK_PASSWORD:
            return AceTextCategory::INPUT_TYPE_PASSWORD;
        case TextInputType::USER_NAME:
            return AceTextCategory::INPUT_TYPE_USER_NAME;
        case TextInputType::NEW_PASSWORD:
            return AceTextCategory::INPUT_TYPE_NEW_PASSWORD;
        default:
            return AceTextCategory::INPUT_TYPE_DEFAULT;
    }
}

bool RichEditorAccessibilityProperty::IsShowCount() const
{
    auto frameNode = host_.Upgrade();
    CHECK_NULL_RETURN(frameNode, false);
    auto richEditorPattern = frameNode->GetPattern<RichEditorPattern>();
    CHECK_NULL_RETURN(richEditorPattern, false);
    CHECK_NULL_RETURN(!richEditorPattern->IsInPasswordMode(), false);
    CHECK_NULL_RETURN(richEditorPattern->IsShowCounterEnabled(), false);
    auto counterDecorator = DynamicCast<CounterDecorator>(richEditorPattern->GetCounterDecorator());
    CHECK_NULL_RETURN(counterDecorator, false);
    return counterDecorator->HasContent();
}

const std::list<RefPtr<UINode>>& RichEditorAccessibilityProperty::GetChildren(const RefPtr<FrameNode>& host) const
{
    auto pattern = host->GetPattern<RichEditorPattern>();
    CHECK_NULL_RETURN(pattern, host->GetChildren());
    auto contentHost = pattern->GetContentHost();
    CHECK_NULL_RETURN(contentHost, host->GetChildren());
    return contentHost->GetChildren();
}

} // namespace OHOS::Ace::NG
