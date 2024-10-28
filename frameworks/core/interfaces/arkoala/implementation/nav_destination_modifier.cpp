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
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/base/frame_node.h"
#include "arkoala_api_generated.h"
#include "core/interfaces/arkoala/utility/converter.h"
#include "core/interfaces/arkoala/generated/interface/node_api.h"
#include "core/components_ng/pattern/navrouter/navdestination_model_ng.h"

namespace OHOS::Ace::NG {
namespace Converter {

template<>
void AssignCast(std::optional<NavDestinationMode>& dst, const Ark_NavDestinationMode& src)
{
    switch (src) {
        case ARK_NAV_DESTINATION_MODE_STANDARD: dst = NavDestinationMode::STANDARD; break;
        case ARK_NAV_DESTINATION_MODE_DIALOG: dst = NavDestinationMode::DIALOG; break;
        default: LOGE("Unexpected enum value in Ark_NavDestinationMode: %{public}d", src);
    }
}
} // namespace Converter
} // namespace OHOS::Ace::NG

namespace OHOS::Ace::NG::GeneratedModifier {
namespace NavDestinationInterfaceModifier {
void SetNavDestinationOptionsImpl(Ark_NativePointer node)
{
    // "No need to implement this method"
}
} // NavDestinationInterfaceModifier
namespace NavDestinationAttributeModifier {
void TitleImpl(Ark_NativePointer node,
               const Ark_Type_NavDestinationAttribute_title_value* value,
               const Opt_NavigationTitleOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    LOGE("ARKOALA NavDestination.TitleImpl -> Method is not implemented. Ark_CustomObject is not supported!");
}
void HideTitleBar0Impl(Ark_NativePointer node,
                       Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavDestinationModelNG::SetHideTitleBar(frameNode, Converter::Convert<bool>(value));
}
void HideTitleBar1Impl(Ark_NativePointer node,
                       Ark_Boolean hide,
                       Ark_Boolean animated)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavDestinationModelNG::SetHideTitleBar(frameNode, Converter::Convert<bool>(hide));
    LOGE("ARKOALA NavDestination.HideTitleBar1Impl -> Method is not fully implemented.");
}
void OnShownImpl(Ark_NativePointer node,
                 Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onShownEvent = [frameNode]() {
        GetFullAPI()->getEventsAPI()->getNavDestinationEventsReceiver()->onShown(frameNode->GetId());
    };
    NavDestinationModelNG::SetOnShown(frameNode, std::move(onShownEvent));
}
void OnHiddenImpl(Ark_NativePointer node,
                  Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onHiddenEvent = [frameNode]() {
        GetFullAPI()->getEventsAPI()->getNavDestinationEventsReceiver()->onHidden(frameNode->GetId());
    };
    NavDestinationModelNG::SetOnHidden(frameNode, std::move(onHiddenEvent));
}
void OnBackPressedImpl(Ark_NativePointer node,
                       Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onBackPressedEvent = [frameNode]() -> bool {
        GetFullAPI()->getEventsAPI()->getNavDestinationEventsReceiver()->onBackPressed(frameNode->GetId());
        return true;
    };
    NavDestinationModelNG::SetOnBackPressed(frameNode, std::move(onBackPressedEvent));
    LOGE("ARKOALA NavDestination.OnBackPressedImpl -> Method is not fully implemented.");
}
void ModeImpl(Ark_NativePointer node,
              Ark_NavDestinationMode value)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto mode = Converter::OptConvert<NavDestinationMode>(value);
    NavDestinationModelNG::SetNavDestinationMode(frameNode, mode.value());
}
void BackButtonIconImpl(Ark_NativePointer node,
                        const Ark_Union_ResourceStr_PixelMap_SymbolGlyphModifier* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    LOGE("ARKOALA NavDestination.BackButtonIconImpl -> Method is not implemented, Ark_CustomObject is not supported!");
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //NavDestinationModelNG::SetBackButtonIcon(frameNode, convValue);
}
void MenusImpl(Ark_NativePointer node,
               const Ark_Union_Array_NavigationMenuItem_CustomBuilder* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    LOGE("ARKOALA NavDestination.MenusImpl -> Method is not implemented, CustomBuilder is not supported!");
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //NavDestinationModelNG::SetMenus(frameNode, convValue);
}
void OnReadyImpl(Ark_NativePointer node,
                 Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    LOGE("ARKOALA NavDestination.OnReadyImpl -> Method is not implemented.");
    //auto convValue = [frameNode](input values) { code }
    //NavDestinationModelNG::SetOnReady(frameNode, convValue);
}
void OnWillAppearImpl(Ark_NativePointer node,
                      Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onWillAppearEvent = [frameNode]() {
        GetFullAPI()->getEventsAPI()->getNavDestinationEventsReceiver()->onWillAppear(frameNode->GetId());
    };
    NavDestinationModelNG::SetOnWillAppear(frameNode, std::move(onWillAppearEvent));
}
void OnWillDisappearImpl(Ark_NativePointer node,
                         Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onWillDisappearEvent = [frameNode]() {
        GetFullAPI()->getEventsAPI()->getNavDestinationEventsReceiver()->onWillDisappear(frameNode->GetId());
    };
    NavDestinationModelNG::SetOnWillDisAppear(frameNode, std::move(onWillDisappearEvent));
}
void OnWillShowImpl(Ark_NativePointer node,
                    Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onWillShowEvent = [frameNode]() {
        GetFullAPI()->getEventsAPI()->getNavDestinationEventsReceiver()->onWillShow(frameNode->GetId());
    };
    NavDestinationModelNG::SetOnWillShow(frameNode, std::move(onWillShowEvent));
}
void OnWillHideImpl(Ark_NativePointer node,
                    Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onWillHideEvent = [frameNode]() {
        GetFullAPI()->getEventsAPI()->getNavDestinationEventsReceiver()->onWillHide(frameNode->GetId());
    };
    NavDestinationModelNG::SetOnWillHide(frameNode, std::move(onWillHideEvent));
}
void IgnoreLayoutSafeAreaImpl(Ark_NativePointer node,
                              const Opt_Array_LayoutSafeAreaType* types,
                              const Opt_Array_LayoutSafeAreaEdge* edges)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);

    NG::SafeAreaExpandOpts opts { .type = NG::SAFE_AREA_TYPE_SYSTEM, .edges = NG::SAFE_AREA_EDGE_ALL };
    auto typesOpt = types ? Converter::OptConvert<Array_LayoutSafeAreaType>(*types) : std::nullopt;
    uint32_t safeAreaType = NG::SAFE_AREA_TYPE_NONE;
    if (typesOpt != std::nullopt) {
        for (int i = 0; i < typesOpt->length; ++i) {
            auto value = typesOpt->array[i];
            if (value == ARK_LAYOUT_SAFE_AREA_TYPE_SYSTEM) {
                safeAreaType = NG::SAFE_AREA_TYPE_SYSTEM;
                break;
            }
        }
        opts.type = safeAreaType;
    }

    auto edgesOpt = types ? Converter::OptConvert<Array_LayoutSafeAreaEdge>(*edges) : std::nullopt;
    uint32_t safeAreaEdge = NG::SAFE_AREA_EDGE_NONE;
    if (edgesOpt != std::nullopt) {
        for (int i = 0; i < edgesOpt->length; ++i) {
            auto value = edgesOpt->array[i];
            if (value == ARK_LAYOUT_SAFE_AREA_EDGE_TOP) {
                safeAreaEdge = NG::SAFE_AREA_EDGE_TOP;
                break;
            } else if (value == ARK_LAYOUT_SAFE_AREA_EDGE_BOTTOM) {
                safeAreaEdge = NG::SAFE_AREA_EDGE_BOTTOM;
                break;
            }
        }
        opts.edges = safeAreaEdge;
    }
    NavDestinationModelNG::SetIgnoreLayoutSafeArea(frameNode, opts);
}
void SystemBarStyleImpl(Ark_NativePointer node,
                        const Opt_CustomObject* style)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = style ? Converter::OptConvert<type>(*style) : std::nullopt;
    //NavDestinationModelNG::SetSystemBarStyle(frameNode, convValue);
    LOGE("ARKOALA NavDestination.SystemBarStyleImpl -> Method is not implemented, Opt_CustomObject is not supported!");
}
void RecoverableImpl(Ark_NativePointer node,
                     const Opt_Boolean* recoverable)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = recoverable ? Converter::OptConvert<bool>(*recoverable) : std::nullopt;
    NavDestinationModelNG::SetRecoverable(frameNode, convValue);
}
void SystemTransitionImpl(Ark_NativePointer node,
                          Ark_NavigationSystemTransitionType type)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(type);
    //auto convValue = Converter::OptConvert<type>(type); // for enums
    //NavDestinationModelNG::SetSystemTransition(frameNode, convValue);
    LOGE("ARKOALA NavDestination.SystemTransitionImpl -> Method is not implemented. "
         "No handlers for Ark_NavigationSystemTransitionType in model");
}
} // NavDestinationAttributeModifier
const GENERATED_ArkUINavDestinationModifier* GetNavDestinationModifier()
{
    static const GENERATED_ArkUINavDestinationModifier ArkUINavDestinationModifierImpl {
        NavDestinationInterfaceModifier::SetNavDestinationOptionsImpl,
        NavDestinationAttributeModifier::TitleImpl,
        NavDestinationAttributeModifier::HideTitleBar0Impl,
        NavDestinationAttributeModifier::HideTitleBar1Impl,
        NavDestinationAttributeModifier::OnShownImpl,
        NavDestinationAttributeModifier::OnHiddenImpl,
        NavDestinationAttributeModifier::OnBackPressedImpl,
        NavDestinationAttributeModifier::ModeImpl,
        NavDestinationAttributeModifier::BackButtonIconImpl,
        NavDestinationAttributeModifier::MenusImpl,
        NavDestinationAttributeModifier::OnReadyImpl,
        NavDestinationAttributeModifier::OnWillAppearImpl,
        NavDestinationAttributeModifier::OnWillDisappearImpl,
        NavDestinationAttributeModifier::OnWillShowImpl,
        NavDestinationAttributeModifier::OnWillHideImpl,
        NavDestinationAttributeModifier::IgnoreLayoutSafeAreaImpl,
        NavDestinationAttributeModifier::SystemBarStyleImpl,
        NavDestinationAttributeModifier::RecoverableImpl,
        NavDestinationAttributeModifier::SystemTransitionImpl,
    };
    return &ArkUINavDestinationModifierImpl;
}

}
