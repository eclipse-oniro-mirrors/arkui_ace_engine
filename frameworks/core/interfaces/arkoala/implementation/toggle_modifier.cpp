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
#include "core/components_ng/pattern/toggle/toggle_model_ng.h"
#include "core/interfaces/arkoala/generated/interface/node_api.h"
#include "core/interfaces/arkoala/utility/converter.h"
#include "core/interfaces/arkoala/utility/converter2.h"
#include "core/interfaces/arkoala/utility/reverse_converter.h"
#include "core/interfaces/arkoala/utility/validators.h"

namespace OHOS::Ace::NG::Converter {
struct ToggleOptions {
    std::optional<ToggleType> type;
    std::optional<bool> isOn;
};

template<>
ToggleOptions Convert(const Ark_ToggleOptions& src)
{
    return {
        .type = Converter::OptConvert<ToggleType>(src.type),
        .isOn = Converter::OptConvert<bool>(src.isOn)
    };
}

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
namespace ToggleInterfaceModifier {
void SetToggleOptionsImpl(Ark_NativePointer node,
                          const Ark_ToggleOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(options);
    auto convValue = Converter::Convert<Converter::ToggleOptions>(*options);
    if (convValue.isOn.has_value()) {
        ToggleModelNG::SetToggleState(frameNode, convValue.isOn.value());
    }
    LOGE("ToggleModifier::SetToggleOptionsImpl. Set ToggleType is not supported!");
}
} // ToggleInterfaceModifier
namespace ToggleAttributeModifier {
void OnChangeImpl(Ark_NativePointer node,
                  Ark_Function callback)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto onChange = [frameNode](bool isOn) {
        auto arkValue = Converter::ArkValue<Ark_Boolean>(isOn);
        GetFullAPI()->getEventsAPI()->getToggleEventsReceiver()->onChange(frameNode->GetId(), arkValue);
    };
    ToggleModelNG::OnChange(frameNode, std::move(onChange));
}
void ContentModifierImpl(Ark_NativePointer node,
                         const Ark_CustomObject* modifier)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(modifier);
    LOGE("ToggleModifier::ContentModifierImpl is not implemented, Ark_CustomObject is not supported!");
}
void SelectedColorImpl(Ark_NativePointer node,
                       const Ark_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto convValue = Converter::OptConvert<Color>(*value);
    ToggleModelNG::SetSelectedColor(frameNode, convValue);
}
void SwitchPointColorImpl(Ark_NativePointer node,
                          const Ark_ResourceColor* color)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(color);
    auto convValue = Converter::OptConvert<Color>(*color);
    ToggleModelNG::SetSwitchPointColor(frameNode, convValue);
}
void SwitchStyleImpl(Ark_NativePointer node,
                     const Ark_SwitchStyle* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto convValue = Converter::Convert<Converter::SwitchStyle>(*value);
    Validator::ValidateNonNegative(convValue.pointRadius);
    Validator::ValidateNonPercent(convValue.pointRadius);
    ToggleModelNG::SetPointRadius(frameNode, convValue.pointRadius);
    ToggleModelNG::SetUnselectedColor(frameNode, convValue.unselectedColor);
    ToggleModelNG::SetSwitchPointColor(frameNode, convValue.pointColor);
    Validator::ValidateNonNegative(convValue.trackBorderRadius);
    Validator::ValidateNonPercent(convValue.trackBorderRadius);
    ToggleModelNG::SetTrackBorderRadius(frameNode, convValue.trackBorderRadius);
}
} // ToggleAttributeModifier
const GENERATED_ArkUIToggleModifier* GetToggleModifier()
{
    static const GENERATED_ArkUIToggleModifier ArkUIToggleModifierImpl {
        ToggleInterfaceModifier::SetToggleOptionsImpl,
        ToggleAttributeModifier::OnChangeImpl,
        ToggleAttributeModifier::ContentModifierImpl,
        ToggleAttributeModifier::SelectedColorImpl,
        ToggleAttributeModifier::SwitchPointColorImpl,
        ToggleAttributeModifier::SwitchStyleImpl,
    };
    return &ArkUIToggleModifierImpl;
}

}