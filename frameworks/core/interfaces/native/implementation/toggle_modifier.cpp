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
#include "core/components_ng/pattern/toggle/toggle_model_ng.h"
#include "core/components_ng/pattern/toggle/toggle_model_static.h"
#include "core/interfaces/native/utility/callback_helper.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/converter2.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/utility/validators.h"

namespace OHOS::Ace::NG {
namespace {
std::optional<bool> ProcessBindableIsOn(FrameNode* frameNode, const Opt_Union_Boolean_Bindable& value)
{
    std::optional<bool> result;
    Converter::VisitUnion(value,
        [&result](const Ark_Boolean& src) {
            result = Converter::OptConvert<bool>(src);
        },
        [&result, frameNode](const Ark_Bindable_Boolean& src) {
            result = Converter::OptConvert<bool>(src.value);
            WeakPtr<FrameNode> weakNode = AceType::WeakClaim(frameNode);
            auto onEvent = [arkCallback = CallbackHelper(src.onChange), weakNode](bool isOn) {
                PipelineContext::SetCallBackNode(weakNode);
                arkCallback.Invoke(Converter::ArkValue<Ark_Boolean>(isOn));
            };
            ToggleModelStatic::OnChangeEvent(frameNode, std::move(onEvent));
        },
        []{});
    return result;
}
} // namespace
} // namespace OHOS::Ace::NG

namespace OHOS::Ace::NG::Converter {
struct SwitchStyle {
    std::optional<Dimension> pointRadius;
    std::optional<Color> unselectedColor;
    std::optional<Color> pointColor;
    std::optional<Dimension> trackBorderRadius;
};

template<>
SwitchStyle Convert(const Ark_SwitchStyle& src)
{
    return {
        .pointRadius = Converter::OptConvert<Dimension>(src.pointRadius),
        .unselectedColor = Converter::OptConvert<Color>(src.unselectedColor),
        .pointColor = Converter::OptConvert<Color>(src.pointColor),
        .trackBorderRadius = Converter::OptConvert<Dimension>(src.trackBorderRadius)
    };
}
}

namespace OHOS::Ace::NG::GeneratedModifier {
namespace ToggleModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    auto frameNode = ToggleModelNG::CreateFrameNode(id, NG::ToggleType::SWITCH, false);
    CHECK_NULL_RETURN(frameNode, nullptr);
    frameNode->IncRefCount();
    return AceType::RawPtr(frameNode);
}

Ark_NativePointer buttonConstruct(Ark_Int32 id, Ark_Int32 flags)
{
    auto frameNode = ToggleModelNG::CreateFrameNode(id, NG::ToggleType::BUTTON, false);
    CHECK_NULL_RETURN(frameNode, nullptr);
    frameNode->IncRefCount();
    return AceType::RawPtr(frameNode);
}

Ark_NativePointer checkboxConstruct(Ark_Int32 id, Ark_Int32 flags)
{
    auto frameNode = ToggleModelNG::CreateFrameNode(id, NG::ToggleType::CHECKBOX, false);
    CHECK_NULL_RETURN(frameNode, nullptr);
    frameNode->IncRefCount();
    return AceType::RawPtr(frameNode);
}
} // namespace ToggleModifier
namespace ToggleInterfaceModifier {
void SetToggleOptionsImpl(Ark_NativePointer node,
                          const Ark_ToggleOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(options);
    auto type = Converter::OptConvert<ToggleType>(options->type);
    auto isOn = ProcessBindableIsOn(frameNode, options->isOn);
    if (isOn.has_value()) {
        ToggleModelNG::SetToggleState(frameNode, *isOn);
    }
    LOGE("ToggleModifier::SetToggleOptionsImpl. Set ToggleType is not supported!");
}
} // ToggleInterfaceModifier
namespace ToggleAttributeModifier {
void SetOnChangeImpl(Ark_NativePointer node,
                     const Opt_Callback_Boolean_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optValue = Converter::GetOptPtr(value);
    if (!optValue) {
        // Implement Reset value
        return;
    }
    auto onChange = [arkCallback = CallbackHelper(*optValue)](const bool isOn) {
        auto arkValue = Converter::ArkValue<Ark_Boolean>(isOn);
        arkCallback.Invoke(arkValue);
    };
    ToggleModelNG::OnChange(frameNode, std::move(onChange));
}
void SetSelectedColorImpl(Ark_NativePointer node,
                          const Opt_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<Color>(value);
    ToggleModelNG::SetSelectedColor(frameNode, convValue);
}
void SetSwitchPointColorImpl(Ark_NativePointer node,
                             const Opt_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<Color>(value);
    ToggleModelNG::SetSwitchPointColor(frameNode, convValue);
}
void SetSwitchStyleImpl(Ark_NativePointer node,
                        const Opt_SwitchStyle* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvertPtr<Converter::SwitchStyle>(value).value_or(Converter::SwitchStyle{});
    Validator::ValidateNonNegative(convValue.pointRadius);
    Validator::ValidateNonPercent(convValue.pointRadius);
    ToggleModelStatic::SetPointRadius(frameNode, convValue.pointRadius);
    ToggleModelStatic::SetUnselectedColor(frameNode, convValue.unselectedColor);
    ToggleModelNG::SetSwitchPointColor(frameNode, convValue.pointColor);
    Validator::ValidateNonNegative(convValue.trackBorderRadius);
    Validator::ValidateNonPercent(convValue.trackBorderRadius);
    ToggleModelStatic::SetTrackBorderRadius(frameNode, convValue.trackBorderRadius);
}
} // ToggleAttributeModifier
const GENERATED_ArkUIToggleModifier* GetToggleModifier()
{
    static const GENERATED_ArkUIToggleModifier ArkUIToggleModifierImpl {
        ToggleModifier::ConstructImpl,
        ToggleInterfaceModifier::SetToggleOptionsImpl,
        ToggleAttributeModifier::SetOnChangeImpl,
        ToggleAttributeModifier::SetSelectedColorImpl,
        ToggleAttributeModifier::SetSwitchPointColorImpl,
        ToggleAttributeModifier::SetSwitchStyleImpl,
    };
    return &ArkUIToggleModifierImpl;
}

}
