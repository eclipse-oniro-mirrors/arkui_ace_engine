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
#include "core/components_ng/pattern/security_component/location_button/location_button_common.h"
#include "core/components_ng/pattern/security_component/location_button/location_button_model_ng.h"
#include "core/components/common/layout/constants.h"
#include "core/interfaces/arkoala/utility/converter.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::Converter {
template<>
void AssignCast(std::optional<LocationButtonIconStyle>& dst, const Ark_LocationIconStyle& src)
{
    switch (src) {
        case ARK_LOCATION_ICON_STYLE_FULL_FILLED: dst = LocationButtonIconStyle::ICON_FULL_FILLED; break;
        case ARK_LOCATION_ICON_STYLE_LINES: dst = LocationButtonIconStyle::ICON_LINE; break;
        default: LOGE("Unexpected enum value in Ark_LocationIconStyle: %{public}d", src);
    }
}
template<>
void AssignCast(std::optional<LocationButtonLocationDescription>& dst, const Ark_LocationDescription& src)
{
    switch (src) {
        case ARK_LOCATION_DESCRIPTION_CURRENT_LOCATION:
            dst = LocationButtonLocationDescription::CURRENT_LOCATION;
            break;
        case ARK_LOCATION_DESCRIPTION_ADD_LOCATION:
            dst = LocationButtonLocationDescription::ADD_LOCATION;
            break;
        case ARK_LOCATION_DESCRIPTION_SELECT_LOCATION:
            dst = LocationButtonLocationDescription::SELECT_LOCATION;
            break;
        case ARK_LOCATION_DESCRIPTION_SHARE_LOCATION:
            dst = LocationButtonLocationDescription::SHARE_LOCATION;
            break;
        case ARK_LOCATION_DESCRIPTION_SEND_LOCATION:
            dst = LocationButtonLocationDescription::SEND_LOCATION;
            break;
        case ARK_LOCATION_DESCRIPTION_LOCATING:
            dst = LocationButtonLocationDescription::LOCATING;
            break;
        case ARK_LOCATION_DESCRIPTION_LOCATION:
            dst = LocationButtonLocationDescription::LOCATION;
            break;
        case ARK_LOCATION_DESCRIPTION_SEND_CURRENT_LOCATION:
            dst = LocationButtonLocationDescription::SEND_CURRENT_LOCATION;
            break;
        case ARK_LOCATION_DESCRIPTION_RELOCATION:
            dst = LocationButtonLocationDescription::RELOCATION;
            break;
        case ARK_LOCATION_DESCRIPTION_PUNCH_IN:
            dst = LocationButtonLocationDescription::PUNCHIN;
            break;
        case ARK_LOCATION_DESCRIPTION_CURRENT_POSITION:
            dst = LocationButtonLocationDescription::CURRENT_POSITION;
            break;
        default:
            LOGE("Unexpected enum value in Ark_LocationDescription: %{public}d", src);
    }
}
template<>
LocationButtonStyle Convert(const Ark_LocationButtonOptions& src)
{
    LocationButtonStyle style;
    style.text = OptConvert<LocationButtonLocationDescription>(src.text);
    style.icon = OptConvert<LocationButtonIconStyle>(src.icon);
    style.backgroundType = OptConvert<ButtonType>(src.buttonType);
    return style;
}
} // namespace OHOS::Ace::NG::Converter
namespace OHOS::Ace::NG::GeneratedModifier {
namespace LocationButtonInterfaceModifier {
void SetLocationButtonOptions0Impl(Ark_NativePointer node)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    LocationButtonModelNG::InitLocationButton(frameNode, LocationButtonStyle(), false);
}
void SetLocationButtonOptions1Impl(Ark_NativePointer node,
                                   const Ark_LocationButtonOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(options);
    auto style = Converter::Convert<LocationButtonStyle>(*options);
    LocationButtonModelNG::InitLocationButton(frameNode, style, false);
}
} // LocationButtonInterfaceModifier
namespace LocationButtonAttributeModifier {
void OnClickImpl(Ark_NativePointer node,
                 Ark_Function event)
{
}
} // LocationButtonAttributeModifier
const GENERATED_ArkUILocationButtonModifier* GetLocationButtonModifier()
{
    static const GENERATED_ArkUILocationButtonModifier ArkUILocationButtonModifierImpl {
        LocationButtonInterfaceModifier::SetLocationButtonOptions0Impl,
        LocationButtonInterfaceModifier::SetLocationButtonOptions1Impl,
        LocationButtonAttributeModifier::OnClickImpl,
    };
    return &ArkUILocationButtonModifierImpl;
}

}