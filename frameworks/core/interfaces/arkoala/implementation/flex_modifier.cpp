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
#include "core/interfaces/arkoala/utility/converter.h"
#include "core/interfaces/arkoala/generated/interface/node_api.h"
#include "arkoala_api_generated.h"
#include "core/components_ng/pattern/flex/flex_model_ng.h"

namespace OHOS::Ace::NG {

constexpr int NUM_1 = 1;
constexpr int NUM_2 = 2;

struct FlexOptions {
    std::optional<FlexDirection> direction;
    std::optional<FlexWrap> wrap;
    std::optional<FlexAlign> align;
    std::optional<WrapAlignment> wrapAlignment;
    std::optional<FlexAlign> alignItems;
    std::optional<WrapAlignment> wrapAlignItems;
    std::optional<WrapAlignment> alignContent;
    std::optional<Dimension> crossSpace;
    std::optional<Dimension> mainSpace;
};
} // OHOS::Ace::NG

namespace OHOS::Ace::NG {
namespace Converter {

template<>
void AssignCast(std::optional<FlexDirection>& dst, const Ark_FlexDirection& src)
{
    switch (src) {
        case ARK_FLEX_DIRECTION_ROW: dst = FlexDirection::ROW; break;
        case ARK_FLEX_DIRECTION_COLUMN: dst = FlexDirection::COLUMN; break;
        case ARK_FLEX_DIRECTION_ROW_REVERSE: dst = FlexDirection::ROW_REVERSE; break;
        case ARK_FLEX_DIRECTION_COLUMN_REVERSE: dst = FlexDirection::COLUMN_REVERSE; break;
        default: LOGE("Unexpected enum value in Ark_FlexDirection: %{public}d", src);
    }
}

template<>
void AssignCast(std::optional<FlexWrap>& dst, const Ark_FlexWrap& src)
{
    switch (src) {
        case ARK_FLEX_WRAP_NO_WRAP: dst = FlexWrap::NO_WRAP; break;
        case ARK_FLEX_WRAP_WRAP: dst = FlexWrap::WRAP; break;
        case ARK_FLEX_WRAP_WRAP_REVERSE: dst = FlexWrap::WRAP_REVERSE; break;
        default: LOGE("Unexpected enum value in Ark_FlexWrap: %{public}d", src);
    }
}

template<>
void AssignCast(std::optional<WrapAlignment>& dst, const Ark_FlexAlign& src)
{
    switch (src) {
        case ARK_FLEX_ALIGN_START: dst = WrapAlignment::START; break;
        case ARK_FLEX_ALIGN_CENTER: dst = WrapAlignment::CENTER; break;
        case ARK_FLEX_ALIGN_END: dst = WrapAlignment::END; break;
        case ARK_FLEX_ALIGN_SPACE_BETWEEN: dst = WrapAlignment::SPACE_AROUND; break;
        case ARK_FLEX_ALIGN_SPACE_AROUND: dst = WrapAlignment::SPACE_BETWEEN; break;
        case ARK_FLEX_ALIGN_SPACE_EVENLY: dst = WrapAlignment::STRETCH; break;
        default: LOGE("Unexpected enum value in Ark_FlexAlign: %{public}d", src);
    }
}

template<>
void AssignCast(std::optional<WrapAlignment>& dst, const Ark_ItemAlign& src)
{
    switch (src) {
        case ARK_ITEM_ALIGN_AUTO: dst = WrapAlignment::START; break;
        case ARK_ITEM_ALIGN_START: dst = WrapAlignment::CENTER; break;
        case ARK_ITEM_ALIGN_CENTER: dst = WrapAlignment::END; break;
        case ARK_ITEM_ALIGN_END: dst = WrapAlignment::SPACE_AROUND; break;
        case ARK_ITEM_ALIGN_BASELINE: dst = WrapAlignment::SPACE_BETWEEN; break;
        case ARK_ITEM_ALIGN_STRETCH: dst = WrapAlignment::STRETCH; break;
        default: LOGE("Unexpected enum value in Ark_ItemAlign: %{public}d", src);
    }
}

template<>
void AssignCast(std::optional<FlexAlign>& dst, const Ark_ItemAlign& src)
{
    switch (src) {
        case ARK_ITEM_ALIGN_AUTO: dst = FlexAlign::AUTO; break;
        case ARK_ITEM_ALIGN_START: dst = FlexAlign::FLEX_START; break;
        case ARK_ITEM_ALIGN_CENTER: dst = FlexAlign::CENTER; break;
        case ARK_ITEM_ALIGN_END: dst = FlexAlign::FLEX_END; break;
        case ARK_ITEM_ALIGN_BASELINE: dst = FlexAlign::STRETCH; break;
        case ARK_ITEM_ALIGN_STRETCH: dst = FlexAlign::BASELINE; break;
        default: LOGE("Unexpected enum value in Ark_ItemAlign: %{public}d", src);
    }
}
} // namespace Converter
} // namespace OHOS::Ace::NG

namespace OHOS::Ace::NG::Converter {
template<>
FlexOptions Convert(
    const Ark_FlexOptions& src)
{
    FlexOptions flexOptions;
    flexOptions.direction = Converter::OptConvert<FlexDirection>(src.direction.value);
    flexOptions.wrap = Converter::OptConvert<FlexWrap>(src.wrap.value);
    if (src.wrap.value == Ark_FlexWrap::ARK_FLEX_WRAP_NO_WRAP) {
        flexOptions.align = Converter::OptConvert<FlexAlign>(src.justifyContent.value);
        flexOptions.alignItems = Converter::OptConvert<FlexAlign>(src.alignItems.value);
    } else {
        flexOptions.wrapAlignment = Converter::OptConvert<WrapAlignment>(src.justifyContent.value);
        flexOptions.wrapAlignItems = Converter::OptConvert<WrapAlignment>(src.alignItems.value);
        flexOptions.alignContent = Converter::OptConvert<WrapAlignment>(src.alignContent.value);
    }

    flexOptions.crossSpace = Converter::OptConvert<Dimension>(src.space.value.cross);
    flexOptions.mainSpace = Converter::OptConvert<Dimension>(src.space.value.main);
    return flexOptions;
}
}

namespace OHOS::Ace::NG::GeneratedModifier {
namespace FlexInterfaceModifier {
void SetFlexOptionsImpl(Ark_NativePointer node,
                        const Opt_FlexOptions* value)
{
    CHECK_NULL_VOID(value);
    auto frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto options = Converter::OptConvert<FlexOptions>(*value);
    CHECK_NULL_VOID(options);
    int32_t wrap = static_cast<int32_t>(options->wrap.value());
    int32_t direction = static_cast<int32_t>(options->direction.value());

    if (options->wrap == FlexWrap::NO_WRAP) {
        FlexModelNG::SetFlexRow(frameNode);
        FlexModelNG::SetFlexDirection(frameNode, options->direction);
        FlexModelNG::SetMainAxisAlign(frameNode, options->align);
        FlexModelNG::SetCrossAxisAlign(frameNode, options->alignItems);
    } else if (options->wrap == FlexWrap::WRAP or options->wrap == FlexWrap::WRAP_REVERSE) {
        if (options->direction != std::nullopt) {
            FlexModelNG::SetFlexDirection(frameNode, options->direction);
            // WrapReverse means wrapVal = 2. Wrap means wrapVal = 1.
            direction <= 1 ? direction += NUM_2 * (wrap - NUM_1) : direction -= NUM_2 * (wrap - NUM_1);
            FlexModelNG::SetFlexWrapDirection(frameNode, static_cast<WrapDirection>(direction));
        } else {
            // No direction set case: wrapVal == 2 means FlexWrap.WrapReverse.
            WrapDirection wrapDirection = wrap == NUM_2 ?
             WrapDirection::HORIZONTAL_REVERSE : WrapDirection::HORIZONTAL;
            FlexModelNG::SetFlexWrapDirection(frameNode, wrapDirection);
        }
        FlexModelNG::SetWrapMainAlignment(frameNode, options->wrapAlignment);
        FlexModelNG::SetWrapCrossAlignment(frameNode, options->wrapAlignItems);
        FlexModelNG::SetFlexAlignContent(frameNode, options->alignContent);
        if (options->crossSpace) {
            FlexModelNG::SetCrossSpace(frameNode, options->crossSpace);
        }
    }
    if (options->mainSpace) {
        FlexModelNG::SetMainSpace(frameNode, options->mainSpace);
    }
}
} // FlexInterfaceModifier

namespace FlexAttributeModifier {
void PointLightImpl(Ark_NativePointer node,
                    const Ark_PointLightStyle* value)
{
    LOGE("ARKOALA FlexAttribute_PointLightImpl -> Method is not FULLY implemented.");
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ACE_UPDATE_NODE_RENDER_CONTEXT(LightIlluminated,
        (float)Converter::ConvertOrDefault(value->illuminated, 0), frameNode);
    ACE_UPDATE_NODE_RENDER_CONTEXT(Bloom, (float)Converter::ConvertOrDefault(value->bloom, 0), frameNode);
}
} // FlexAttributeModifier
const GENERATED_ArkUIFlexModifier* GetFlexModifier()
{
    static const GENERATED_ArkUIFlexModifier ArkUIFlexModifierImpl {
        FlexInterfaceModifier::SetFlexOptionsImpl,
        FlexAttributeModifier::PointLightImpl,
    };
    return &ArkUIFlexModifierImpl;
}

}