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

#include "core/interfaces/arkoala/utility/converter.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/stack/stack_model_ng.h"
#include "core/components/common/properties/alignment.h"

namespace OHOS::Ace::NG {
namespace {
struct StackOptions {
    std::optional<Alignment> alignContent;
};
}

namespace Converter {
template<>
StackOptions Convert(const Ark_StackOptions& src)
{
    return {
        .alignContent = OptConvert<Alignment>(src.alignContent),
    };
}
} // namespace Converter
} // namespace OHOS::Ace::NG

namespace OHOS::Ace::NG::GeneratedModifier {
namespace StackInterfaceModifier {

void SetStackOptionsImpl(Ark_NativePointer node,
                         const Opt_StackOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    auto opts = Converter::OptConvert<StackOptions>(*options);
    auto align = opts ? opts->alignContent : std::nullopt;
    StackModelNG::SetAlignment(frameNode, align.value_or(Alignment::CENTER));
}
} // StackInterfaceModifier
namespace StackAttributeModifier {
void AlignContentImpl(Ark_NativePointer node,
                      Ark_Alignment value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    StackModelNG::SetAlignment(frameNode, Converter::ConvertOrDefault(value, Alignment::CENTER));
}
void PointLightImpl(Ark_NativePointer node,
                    const Ark_PointLightStyle* value)
{
    LOGE("Arkoala: Stack.PointLightImpl - method not implemented");
}
} // StackAttributeModifier
const GENERATED_ArkUIStackModifier* GetStackModifier()
{
    static const GENERATED_ArkUIStackModifier ArkUIStackModifierImpl {
        StackInterfaceModifier::SetStackOptionsImpl,
        StackAttributeModifier::AlignContentImpl,
        StackAttributeModifier::PointLightImpl,
    };
    return &ArkUIStackModifierImpl;
}

}