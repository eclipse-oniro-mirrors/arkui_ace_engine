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
namespace PluginComponentInterfaceModifier {
void SetPluginComponentOptionsImpl(Ark_NativePointer node,
                                   const Ark_Literal_PluginComponentTemplate_template_Any_data* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //PluginComponentModelNG::SetSetPluginComponentOptions(frameNode, convValue);
}
} // PluginComponentInterfaceModifier
namespace PluginComponentAttributeModifier {
void OnCompleteImpl(Ark_NativePointer node,
                    Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //PluginComponentModelNG::SetOnComplete(frameNode, convValue);
}
void OnErrorImpl(Ark_NativePointer node,
                 Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //PluginComponentModelNG::SetOnError(frameNode, convValue);
}
} // PluginComponentAttributeModifier
const GENERATED_ArkUIPluginComponentModifier* GetPluginComponentModifier()
{
    static const GENERATED_ArkUIPluginComponentModifier ArkUIPluginComponentModifierImpl {
        PluginComponentInterfaceModifier::SetPluginComponentOptionsImpl,
        PluginComponentAttributeModifier::OnCompleteImpl,
        PluginComponentAttributeModifier::OnErrorImpl,
    };
    return &ArkUIPluginComponentModifierImpl;
}

}
