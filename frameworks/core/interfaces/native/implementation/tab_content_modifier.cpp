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
namespace TabContentModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    return {};
}
} // TabContentModifier
namespace TabContentInterfaceModifier {
void SetTabContentOptionsImpl(Ark_NativePointer node)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(undefined);
    //auto convValue = Converter::OptConvert<type>(undefined); // for enums
    //TabContentModelNG::SetSetTabContentOptions(frameNode, convValue);
}
} // TabContentInterfaceModifier
namespace TabContentAttributeModifier {
void SetTabBarImpl(Ark_NativePointer node,
                   const Opt_Union_ComponentContent_SubTabBarStyle_BottomTabBarStyle_String_Resource_CustomBuilder_TabBarOptions* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //TabContentModelNG::SetSetTabBar(frameNode, convValue);
}
void SetOnWillShowImpl(Ark_NativePointer node,
                       const Opt_VoidCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //TabContentModelNG::SetSetOnWillShow(frameNode, convValue);
}
void SetOnWillHideImpl(Ark_NativePointer node,
                       const Opt_VoidCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //TabContentModelNG::SetSetOnWillHide(frameNode, convValue);
}
} // TabContentAttributeModifier
const GENERATED_ArkUITabContentModifier* GetTabContentModifier()
{
    static const GENERATED_ArkUITabContentModifier ArkUITabContentModifierImpl {
        TabContentModifier::ConstructImpl,
        TabContentInterfaceModifier::SetTabContentOptionsImpl,
        TabContentAttributeModifier::SetTabBarImpl,
        TabContentAttributeModifier::SetOnWillShowImpl,
        TabContentAttributeModifier::SetOnWillHideImpl,
    };
    return &ArkUITabContentModifierImpl;
}

}
