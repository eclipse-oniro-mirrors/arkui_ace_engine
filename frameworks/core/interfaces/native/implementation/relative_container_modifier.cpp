/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
namespace RelativeContainerModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    return {};
}
} // RelativeContainerModifier
namespace RelativeContainerInterfaceModifier {
void SetRelativeContainerOptionsImpl(Ark_NativePointer node)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(undefined);
    //auto convValue = Converter::OptConvert<type>(undefined); // for enums
    //RelativeContainerModelNG::SetSetRelativeContainerOptions(frameNode, convValue);
}
} // RelativeContainerInterfaceModifier
namespace RelativeContainerAttributeModifier {
void SetGuideLineImpl(Ark_NativePointer node,
                      const Opt_Array_GuideLineStyle* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //RelativeContainerModelNG::SetSetGuideLine(frameNode, convValue);
}
void SetBarrierImpl(Ark_NativePointer node,
                    const Opt_Array_BarrierStyle* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //RelativeContainerModelNG::SetSetBarrier(frameNode, convValue);
}
} // RelativeContainerAttributeModifier
const GENERATED_ArkUIRelativeContainerModifier* GetRelativeContainerModifier()
{
    static const GENERATED_ArkUIRelativeContainerModifier ArkUIRelativeContainerModifierImpl {
        RelativeContainerModifier::ConstructImpl,
        RelativeContainerInterfaceModifier::SetRelativeContainerOptionsImpl,
        RelativeContainerAttributeModifier::SetGuideLineImpl,
        RelativeContainerAttributeModifier::SetBarrierImpl,
    };
    return &ArkUIRelativeContainerModifierImpl;
}

}
