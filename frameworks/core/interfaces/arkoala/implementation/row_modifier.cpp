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

#include "core/interfaces/native/node/node_api.h"
#include "arkoala_api_generated.h"
#include "core/interfaces/arkoala/utility/converter.h"
#include "core/common/container.h"
#include "core/components_ng/pattern/linear_layout/row_model_ng.h"
#include "core/components_ng/base/view_stack_processor.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace RowInterfaceModifier {
void SetRowOptionsImpl(Ark_NativePointer node,
                       const Opt_Type_RowInterface_setRowOptions_Arg0* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    std::tuple<Ark_Float32, Ark_Int32> space = Converter::ConvertOrDefault(
        *value, std::make_tuple(0.0f, (int)DimensionUnit::PX));
    RowModelNG::SetSpace(
        frameNode,
        CalcDimension(std::get<0>(space), (DimensionUnit)std::get<1>(space)));
}
} // RowInterfaceModifier
namespace RowAttributeModifier {
void AlignItemsImpl(Ark_NativePointer node,
                    Ark_Int32 value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    RowModelNG::SetAlignItems(frameNode, static_cast<FlexAlign>(value + 1));
}
void JustifyContentImpl(Ark_NativePointer node,
                        Ark_Int32 value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    RowModelNG::SetJustifyContent(frameNode, static_cast<FlexAlign>(value));
}
void PointLightImpl(Ark_NativePointer node,
                    const Ark_PointLightStyle* value)
{
    LOGW("ARKOALA RowAttribute_PointLightImpl -> Method is not FULLY implemented.");
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    ACE_UPDATE_NODE_RENDER_CONTEXT(
        LightIlluminated, (float)Converter::ConvertOrDefault(value->illuminated, 0),
        frameNode);
    ACE_UPDATE_NODE_RENDER_CONTEXT(
        Bloom, (float)Converter::ConvertOrDefault(value->bloom, 0), frameNode);
}
} // RowAttributeModifier
const GENERATED_ArkUIRowModifier* GetRowModifier()
{
    static const GENERATED_ArkUIRowModifier ArkUIRowModifierImpl {
        RowInterfaceModifier::SetRowOptionsImpl,
        RowAttributeModifier::AlignItemsImpl,
        RowAttributeModifier::JustifyContentImpl,
        RowAttributeModifier::PointLightImpl,
    };
    return &ArkUIRowModifierImpl;
}

}
} // RowInterface

namespace RowAttribute {
void AlignItemsImpl(Ark_NativePointer node, Ark_Int32 value) {
}
void JustifyContentImpl(Ark_NativePointer node, Ark_Int32 value) {
  auto frameNode = reinterpret_cast<FrameNode *>(node);
  RowModelNG::SetJustifyContent(frameNode, static_cast<FlexAlign>(value));
}
void PointLightImpl(Ark_NativePointer node, const Ark_PointLightStyle* value) {
}
} // RowAttribute

const GENERATED_ArkUIRowModifier* GetRowModifier() {
    static const GENERATED_ArkUIRowModifier ArkUIRowModifierImpl {
        RowInterface::_setRowOptionsImpl,
        RowAttribute::AlignItemsImpl,
        RowAttribute::JustifyContentImpl,
        RowAttribute::PointLightImpl,
    };
    return &ArkUIRowModifierImpl;
}

} // namespace OHOS::Ace::NG::GeneratedModifier
