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
#include "core/interfaces/native/utility/converter.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace UIExtensionComponentInterfaceModifier {
void SetUIExtensionComponentOptionsImpl(Ark_NativePointer node,
                                        const Ark_Want* want,
                                        const Opt_UIExtensionOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(want);
    //auto convValue = Converter::OptConvert<type>(want); // for enums
    //UIExtensionComponentModelNG::SetSetUIExtensionComponentOptions(frameNode, convValue);
}
} // UIExtensionComponentInterfaceModifier
namespace UIExtensionComponentAttributeModifier {
void OnRemoteReadyImpl(Ark_NativePointer node,
                       const Callback_UIExtensionProxy_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //UIExtensionComponentModelNG::SetOnRemoteReady(frameNode, convValue);
}
void OnReceiveImpl(Ark_NativePointer node,
                   const Callback_Literal_Empty_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //UIExtensionComponentModelNG::SetOnReceive(frameNode, convValue);
}
void OnResultImpl(Ark_NativePointer node,
                  const Callback_Literal_Number_code_Want_want_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //UIExtensionComponentModelNG::SetOnResult(frameNode, convValue);
}
void OnReleaseImpl(Ark_NativePointer node,
                   const Callback_Number_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //UIExtensionComponentModelNG::SetOnRelease(frameNode, convValue);
}
void OnErrorImpl(Ark_NativePointer node,
                 const Ark_CustomObject* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //UIExtensionComponentModelNG::SetOnError(frameNode, convValue);
}
void OnTerminatedImpl(Ark_NativePointer node,
                      const Callback_TerminationInfo_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //UIExtensionComponentModelNG::SetOnTerminated(frameNode, convValue);
}
} // UIExtensionComponentAttributeModifier
const GENERATED_ArkUIUIExtensionComponentModifier* GetUIExtensionComponentModifier()
{
    static const GENERATED_ArkUIUIExtensionComponentModifier ArkUIUIExtensionComponentModifierImpl {
        UIExtensionComponentInterfaceModifier::SetUIExtensionComponentOptionsImpl,
        UIExtensionComponentAttributeModifier::OnRemoteReadyImpl,
        UIExtensionComponentAttributeModifier::OnReceiveImpl,
        UIExtensionComponentAttributeModifier::OnResultImpl,
        UIExtensionComponentAttributeModifier::OnReleaseImpl,
        UIExtensionComponentAttributeModifier::OnErrorImpl,
        UIExtensionComponentAttributeModifier::OnTerminatedImpl,
    };
    return &ArkUIUIExtensionComponentModifierImpl;
}

}