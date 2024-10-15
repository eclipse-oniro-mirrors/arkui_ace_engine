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
namespace CanvasInterfaceModifier {
void SetCanvasOptions0Impl(Ark_NativePointer node,
                           const Opt_Union_CanvasRenderingContext2D_DrawingRenderingContext* context)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = context ? Converter::OptConvert<type>(*context) : std::nullopt;
    //CanvasModelNG::SetSetCanvasOptions0(frameNode, convValue);
}
void SetCanvasOptions1Impl(Ark_NativePointer node,
                           const Ark_Union_CanvasRenderingContext2D_DrawingRenderingContext* context,
                           const Ark_ImageAIOptions* imageAIOptions)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(context);
    //auto convValue = Converter::OptConvert<type>(context); // for enums
    //CanvasModelNG::SetSetCanvasOptions1(frameNode, convValue);
}
} // CanvasInterfaceModifier
namespace CanvasAttributeModifier {
void OnReadyImpl(Ark_NativePointer node,
                 Ark_Function value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //CanvasModelNG::SetOnReady(frameNode, convValue);
}
void EnableAnalyzerImpl(Ark_NativePointer node,
                        Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    [[maybe_unused]]
    auto convValue = Converter::Convert<bool>(value);
    //CanvasModelNG::SetEnableAnalyzer(frameNode, convValue);
}
} // CanvasAttributeModifier
const GENERATED_ArkUICanvasModifier* GetCanvasModifier()
{
    static const GENERATED_ArkUICanvasModifier ArkUICanvasModifierImpl {
        CanvasInterfaceModifier::SetCanvasOptions0Impl,
        CanvasInterfaceModifier::SetCanvasOptions1Impl,
        CanvasAttributeModifier::OnReadyImpl,
        CanvasAttributeModifier::EnableAnalyzerImpl,
    };
    return &ArkUICanvasModifierImpl;
}

}
