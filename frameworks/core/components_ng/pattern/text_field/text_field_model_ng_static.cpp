/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/text_field/text_field_model_ng.h"
#include <cstddef>
#include "core/components_ng/pattern/text_field/text_field_layout_property.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"

namespace OHOS::Ace::NG {
RefPtr<TextFieldControllerBase> TextFieldModelNG::GetOrCreateController(FrameNode* frameNode)
{
    CHECK_NULL_RETURN(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_RETURN(pattern, nullptr);
    if (!pattern->GetTextFieldController()) {
        auto controller = AceType::MakeRefPtr<NG::TextFieldController>();
        pattern->SetTextFieldController(controller);
        controller->SetPattern(AceType::WeakClaim(AceType::RawPtr(pattern)));
    }
    return pattern->GetTextFieldController();
}

RefPtr<TextFieldControllerBase> TextFieldModelNG::GetController(FrameNode* frameNode,
    const std::optional<std::u16string>& placeholder, const std::optional<std::u16string>& value)
{
    CHECK_NULL_RETURN(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_RETURN(pattern, nullptr);

    auto textFieldLayoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    CHECK_NULL_RETURN(textFieldLayoutProperty, nullptr);
    textFieldLayoutProperty->UpdatePlaceholder(placeholder.value_or(u""));

    auto textValue = pattern->GetTextUtf16Value();
    if (value.has_value() && value.value() != textValue) {
        auto changed = pattern->InitValueText(value.value());
        pattern->SetTextChangedAtCreation(changed);
    }

    return pattern->GetTextFieldController();
}

void TextFieldModelNG::SetJSTextEditableController(FrameNode* frameNode, const RefPtr<Referenced>& controller)
{
    CHECK_NULL_VOID(frameNode);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetJSTextEditableController(controller);
}

RefPtr<Referenced> TextFieldModelNG::GetJSTextEditableController(FrameNode* frameNode)
{
    CHECK_NULL_RETURN(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<TextFieldPattern>();
    CHECK_NULL_RETURN(pattern, nullptr);
    return pattern->GetJSTextEditableController();
}

void TextFieldModelNG::SetLineSpacing(FrameNode* frameNode, const std::optional<Dimension>& valueOpt)
{
    if (valueOpt) {
        ACE_UPDATE_NODE_LAYOUT_PROPERTY(TextFieldLayoutProperty, LineSpacing, valueOpt.value(), frameNode);
    } else {
        ACE_RESET_NODE_LAYOUT_PROPERTY(TextFieldLayoutProperty, LineSpacing, frameNode);
    }
}
} // namespace OHOS::Ace::NG
