/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef WEB_SUPPORTED
#include "core/components_ng/pattern/web/richtext_model_ng.h"
#endif
#include "core/components_ng/base/frame_node.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "arkoala_api_generated.h"
#include "core/interfaces/native/utility/callback_helper.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace RichTextInterfaceModifier {
void SetRichTextOptionsImpl(Ark_NativePointer node,
                            const Ark_String* content)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(content);
#ifdef WEB_SUPPORTED
    auto convValue = Converter::Convert<std::string>(*content);
    RichTextModelNG::SetRichTextOptions(frameNode, convValue);
#endif
}
} // RichTextInterfaceModifier
namespace RichTextAttributeModifier {
void OnStartImpl(Ark_NativePointer node,
                 const Callback_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
#ifdef WEB_SUPPORTED
    auto onCallback = [arkCallback = CallbackHelper(*value)](const BaseEventInfo* event) {
        arkCallback.Invoke();
    };
    RichTextModelNG::SetOnPageStart(frameNode, std::move(onCallback));
#endif
}
void OnCompleteImpl(Ark_NativePointer node,
                    const Callback_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
#ifdef WEB_SUPPORTED
    auto onCallback = [arkCallback = CallbackHelper(*value)](const BaseEventInfo* event) {
        arkCallback.Invoke();
    };
    RichTextModelNG::SetOnPageFinish(frameNode, std::move(onCallback));
#endif
}
} // RichTextAttributeModifier
const GENERATED_ArkUIRichTextModifier* GetRichTextModifier()
{
    static const GENERATED_ArkUIRichTextModifier ArkUIRichTextModifierImpl {
        RichTextInterfaceModifier::SetRichTextOptionsImpl,
        RichTextAttributeModifier::OnStartImpl,
        RichTextAttributeModifier::OnCompleteImpl,
    };
    return &ArkUIRichTextModifierImpl;
}

}