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
namespace TabsInterfaceModifier {
void SetTabsOptionsImpl(Ark_NativePointer node,
                        const Opt_Type_TabsInterface_value* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //TabsModelNG::SetSetTabsOptions(frameNode, convValue);
}
} // TabsInterfaceModifier
namespace TabsAttributeModifier {
void VerticalImpl(Ark_NativePointer node,
                  Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    [[maybe_unused]]
    auto convValue = Converter::Convert<bool>(value);
    //TabsModelNG::SetVertical(frameNode, convValue);
}
void BarPositionImpl(Ark_NativePointer node,
                     Ark_BarPosition value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TabsModelNG::SetBarPosition(frameNode, convValue);
}
void ScrollableImpl(Ark_NativePointer node,
                    Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    [[maybe_unused]]
    auto convValue = Converter::Convert<bool>(value);
    //TabsModelNG::SetScrollable(frameNode, convValue);
}
void BarMode0Impl(Ark_NativePointer node,
                  Ark_BarMode value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TabsModelNG::SetBarMode0(frameNode, convValue);
}
void BarWidthImpl(Ark_NativePointer node,
                  const Ark_Length* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TabsModelNG::SetBarWidth(frameNode, convValue);
}
void BarHeightImpl(Ark_NativePointer node,
                   const Ark_Length* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TabsModelNG::SetBarHeight(frameNode, convValue);
}
void AnimationDurationImpl(Ark_NativePointer node,
                           const Ark_Number* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TabsModelNG::SetAnimationDuration(frameNode, convValue);
}
void AnimationModeImpl(Ark_NativePointer node,
                       const Opt_AnimationMode* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //TabsModelNG::SetAnimationMode(frameNode, convValue);
}
void EdgeEffectImpl(Ark_NativePointer node,
                    const Opt_EdgeEffect* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //TabsModelNG::SetEdgeEffect(frameNode, convValue);
}
void OnChangeImpl(Ark_NativePointer node,
                  Ark_Function value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TabsModelNG::SetOnChange(frameNode, convValue);
}
void OnTabBarClickImpl(Ark_NativePointer node,
                       Ark_Function value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TabsModelNG::SetOnTabBarClick(frameNode, convValue);
}
void OnAnimationStartImpl(Ark_NativePointer node,
                          Ark_Function value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TabsModelNG::SetOnAnimationStart(frameNode, convValue);
}
void OnAnimationEndImpl(Ark_NativePointer node,
                        Ark_Function value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TabsModelNG::SetOnAnimationEnd(frameNode, convValue);
}
void OnGestureSwipeImpl(Ark_NativePointer node,
                        Ark_Function value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TabsModelNG::SetOnGestureSwipe(frameNode, convValue);
}
void FadingEdgeImpl(Ark_NativePointer node,
                    Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    [[maybe_unused]]
    auto convValue = Converter::Convert<bool>(value);
    //TabsModelNG::SetFadingEdge(frameNode, convValue);
}
void DividerImpl(Ark_NativePointer node,
                 const Ark_Union_DividerStyle_Undefined* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TabsModelNG::SetDivider(frameNode, convValue);
}
void BarOverlapImpl(Ark_NativePointer node,
                    Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    [[maybe_unused]]
    auto convValue = Converter::Convert<bool>(value);
    //TabsModelNG::SetBarOverlap(frameNode, convValue);
}
void BarBackgroundColorImpl(Ark_NativePointer node,
                            const Ark_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TabsModelNG::SetBarBackgroundColor(frameNode, convValue);
}
void BarGridAlignImpl(Ark_NativePointer node,
                      const Ark_BarGridColumnOptions* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //TabsModelNG::SetBarGridAlign(frameNode, convValue);
}
void CustomContentTransitionImpl(Ark_NativePointer node,
                                 Ark_Function value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TabsModelNG::SetCustomContentTransition(frameNode, convValue);
}
void BarBackgroundBlurStyleImpl(Ark_NativePointer node,
                                Ark_BlurStyle value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TabsModelNG::SetBarBackgroundBlurStyle(frameNode, convValue);
}
void OnContentWillChangeImpl(Ark_NativePointer node,
                             Ark_Function value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = [frameNode](input values) { code }
    //TabsModelNG::SetOnContentWillChange(frameNode, convValue);
}
void BarMode1Impl(Ark_NativePointer node,
                  Ark_BarMode value,
                  const Ark_ScrollableBarModeOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TabsModelNG::SetBarMode1(frameNode, convValue);
}
void BarMode2Impl(Ark_NativePointer node,
                  Ark_BarMode value,
                  const Opt_ScrollableBarModeOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(value);
    //auto convValue = Converter::OptConvert<type>(value); // for enums
    //TabsModelNG::SetBarMode2(frameNode, convValue);
}
} // TabsAttributeModifier
const GENERATED_ArkUITabsModifier* GetTabsModifier()
{
    static const GENERATED_ArkUITabsModifier ArkUITabsModifierImpl {
        TabsInterfaceModifier::SetTabsOptionsImpl,
        TabsAttributeModifier::VerticalImpl,
        TabsAttributeModifier::BarPositionImpl,
        TabsAttributeModifier::ScrollableImpl,
        TabsAttributeModifier::BarMode0Impl,
        TabsAttributeModifier::BarWidthImpl,
        TabsAttributeModifier::BarHeightImpl,
        TabsAttributeModifier::AnimationDurationImpl,
        TabsAttributeModifier::AnimationModeImpl,
        TabsAttributeModifier::EdgeEffectImpl,
        TabsAttributeModifier::OnChangeImpl,
        TabsAttributeModifier::OnTabBarClickImpl,
        TabsAttributeModifier::OnAnimationStartImpl,
        TabsAttributeModifier::OnAnimationEndImpl,
        TabsAttributeModifier::OnGestureSwipeImpl,
        TabsAttributeModifier::FadingEdgeImpl,
        TabsAttributeModifier::DividerImpl,
        TabsAttributeModifier::BarOverlapImpl,
        TabsAttributeModifier::BarBackgroundColorImpl,
        TabsAttributeModifier::BarGridAlignImpl,
        TabsAttributeModifier::CustomContentTransitionImpl,
        TabsAttributeModifier::BarBackgroundBlurStyleImpl,
        TabsAttributeModifier::OnContentWillChangeImpl,
        TabsAttributeModifier::BarMode1Impl,
        TabsAttributeModifier::BarMode2Impl,
    };
    return &ArkUITabsModifierImpl;
}

}
