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
namespace GridItemModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    return {};
}
} // GridItemModifier
namespace GridItemInterfaceModifier {
void SetGridItemOptionsImpl(Ark_NativePointer node,
                            const Opt_GridItemOptions* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //GridItemModelNG::SetSetGridItemOptions(frameNode, convValue);
}
} // GridItemInterfaceModifier
namespace GridItemAttributeModifier {
void RowStartImpl(Ark_NativePointer node,
                  const Ark_Number* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //GridItemModelNG::SetRowStart(frameNode, convValue);
}
void RowEndImpl(Ark_NativePointer node,
                const Ark_Number* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //GridItemModelNG::SetRowEnd(frameNode, convValue);
}
void ColumnStartImpl(Ark_NativePointer node,
                     const Ark_Number* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //GridItemModelNG::SetColumnStart(frameNode, convValue);
}
void ColumnEndImpl(Ark_NativePointer node,
                   const Ark_Number* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //GridItemModelNG::SetColumnEnd(frameNode, convValue);
}
void ForceRebuildImpl(Ark_NativePointer node,
                      Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::Convert<bool>(value);
    //GridItemModelNG::SetForceRebuild(frameNode, convValue);
}
void SelectableImpl(Ark_NativePointer node,
                    Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::Convert<bool>(value);
    //GridItemModelNG::SetSelectable(frameNode, convValue);
}
void SelectedImpl(Ark_NativePointer node,
                  Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::Convert<bool>(value);
    //GridItemModelNG::SetSelected(frameNode, convValue);
}
void OnSelectImpl(Ark_NativePointer node,
                  const Callback_Boolean_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //GridItemModelNG::SetOnSelect(frameNode, convValue);
}
void _onChangeEvent_selectedImpl(Ark_NativePointer node,
                                 const Callback_Boolean_Void* callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(callback);
    //auto convValue = Converter::OptConvert<type_name>(*callback);
    //GridItemModelNG::Set_onChangeEvent_selected(frameNode, convValue);
}
} // GridItemAttributeModifier
const GENERATED_ArkUIGridItemModifier* GetGridItemModifier()
{
    static const GENERATED_ArkUIGridItemModifier ArkUIGridItemModifierImpl {
        GridItemModifier::ConstructImpl,
        GridItemInterfaceModifier::SetGridItemOptionsImpl,
        GridItemAttributeModifier::RowStartImpl,
        GridItemAttributeModifier::RowEndImpl,
        GridItemAttributeModifier::ColumnStartImpl,
        GridItemAttributeModifier::ColumnEndImpl,
        GridItemAttributeModifier::ForceRebuildImpl,
        GridItemAttributeModifier::SelectableImpl,
        GridItemAttributeModifier::SelectedImpl,
        GridItemAttributeModifier::OnSelectImpl,
        GridItemAttributeModifier::_onChangeEvent_selectedImpl,
    };
    return &ArkUIGridItemModifierImpl;
}

}
