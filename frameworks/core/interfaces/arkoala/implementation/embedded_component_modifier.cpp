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

#include "core/components_ng/base/frame_node.h"
#include "core/interfaces/arkoala/utility/converter.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace EmbeddedComponentInterfaceModifier {
void SetEmbeddedComponentOptionsImpl(Ark_NativePointer node,
                                     const Ark_CustomObject* loader,
                                     Ark_EmbeddedType type)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(loader);
    //auto convValue = Converter::OptConvert<type>(loader); // for enums
    //EmbeddedComponentModelNG::SetSetEmbeddedComponentOptions(frameNode, convValue);
}
} // EmbeddedComponentInterfaceModifier
namespace EmbeddedComponentAttributeModifier {
void OnTerminatedImpl(Ark_NativePointer node,
                      Ark_Function value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //EmbeddedComponentModelNG::SetOnTerminated(frameNode, convValue);
}
void OnErrorImpl(Ark_NativePointer node,
                 const Ark_CustomObject* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //EmbeddedComponentModelNG::SetOnError(frameNode, convValue);
}
} // EmbeddedComponentAttributeModifier
const GENERATED_ArkUIEmbeddedComponentModifier* GetEmbeddedComponentModifier()
{
    static const GENERATED_ArkUIEmbeddedComponentModifier ArkUIEmbeddedComponentModifierImpl {
        EmbeddedComponentInterfaceModifier::SetEmbeddedComponentOptionsImpl,
        EmbeddedComponentAttributeModifier::OnTerminatedImpl,
        EmbeddedComponentAttributeModifier::OnErrorImpl,
    };
    return &ArkUIEmbeddedComponentModifierImpl;
}

}
