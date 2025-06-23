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

#include <algorithm>

#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/pattern/shape/rect_model_ng.h"
#include "core/components_ng/pattern/shape/rect_model_static.h"
#include "core/components_ng/pattern/shape/rect_paint_property.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/shape/shape_abstract_model_ng.h"
#include "core/interfaces/native/utility/converter.h"

namespace OHOS::Ace::NG {
struct RectRadius {
    std::optional<Dimension> radiusWidth;
    std::optional<Dimension> radiusHeight;
};
struct RectOptions {
    std::optional<Dimension> width;
    std::optional<Dimension> height;
    std::optional<Dimension> radiusWidth;
    std::optional<Dimension> radiusHeight;
    std::vector<RectRadius> cornerRadius;
};
} // OHOS::Ace::NG
namespace {
    constexpr size_t MAX_RADIUS_ITEM_COUNT = 4;
}
namespace OHOS::Ace::NG::Converter {
template<>
RectRadius Convert(const Ark_RadiusItem& src)
{
    RectRadius radiusStruct;
    radiusStruct.radiusWidth = OptConvert<Dimension>(src.value0);
    radiusStruct.radiusHeight = OptConvert<Dimension>(src.value1);
    return radiusStruct;
}
template<>
RectOptions Convert(const Ark_RectOptions& src)
{
    RectOptions rectOptions;
    rectOptions.width = Converter::OptConvert<Dimension>(src.width);
    rectOptions.height = Converter::OptConvert<Dimension>(src.height);
    VisitUnion(src.radius,
        [&rectOptions](const Ark_Length& src) {
            auto radius = OptConvert<Dimension>(src);
            rectOptions.radiusWidth = radius;
            rectOptions.radiusHeight = radius;
        },
        [&rectOptions](const Array_RadiusItem& src) {
            rectOptions.cornerRadius = Convert<std::vector<RectRadius>>(src);
        },
        []() {});
    return rectOptions;
}

template<>
RectOptions Convert(const Ark_RoundedRectOptions& src)
{
    RectOptions rectOptions;
    rectOptions.width = Converter::OptConvert<Dimension>(src.width);
    rectOptions.height = Converter::OptConvert<Dimension>(src.height);
    rectOptions.radiusWidth = Converter::OptConvert<Dimension>(src.radiusWidth);
    rectOptions.radiusHeight = Converter::OptConvert<Dimension>(src.radiusHeight);
    return rectOptions;
}

template<>
RectRadius Convert(const Ark_Number& src)
{
    RectRadius radiusStruct;
    radiusStruct.radiusWidth = Converter::OptConvert<Dimension>(src);
    radiusStruct.radiusHeight = Converter::OptConvert<Dimension>(src);
    return radiusStruct;
}

template<>
RectRadius Convert(const Ark_String& src)
{
    RectRadius radiusStruct;
    radiusStruct.radiusWidth = Converter::OptConvert<Dimension>(src);
    radiusStruct.radiusHeight = Converter::OptConvert<Dimension>(src);
    return radiusStruct;
}

template<>
RectRadius Convert(const Array_Union_Number_String& src)
{
    RectRadius radiusStruct;
    auto vec = Convert<std::vector<std::optional<Dimension>>>(src);
    if (vec.size() > 1) {
        radiusStruct.radiusWidth = vec[0];
        radiusStruct.radiusHeight = vec[1];
    }
    return radiusStruct;
}
} // namespace OHOS::Ace::NG::Converter

namespace OHOS::Ace::NG::GeneratedModifier {
namespace RectModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    auto frameNode = RectModelStatic::CreateFrameNode(id);
    CHECK_NULL_RETURN(frameNode, nullptr);
    frameNode->IncRefCount();
    return AceType::RawPtr(frameNode);
}
} // RectModifier
namespace RectInterfaceModifier {
void SetRectOptionsImpl(Ark_NativePointer node,
                        const Opt_Union_RectOptions_RoundedRectOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(options);
    auto opt = Converter::OptConvert<RectOptions>(*options);
    CHECK_NULL_VOID(opt);
    if (opt->width) {
        ShapeAbstractModelNG::SetWidth(frameNode, opt->width.value());
    }

    if (opt->height) {
        ShapeAbstractModelNG::SetHeight(frameNode, opt->height.value());
    }

    if (!opt->radiusWidth || !opt->radiusHeight) {
        size_t length = std::min(opt->cornerRadius.size(), MAX_RADIUS_ITEM_COUNT);
        for (int32_t i = 0; i < length; ++i) {
            const RectRadius radiusItem = opt->cornerRadius[i];
            const Dimension rx = radiusItem.radiusWidth.value_or(Dimension());
            const Dimension ry = radiusItem.radiusHeight.value_or(Dimension());
            RectModelNG::SetRadiusValue(frameNode, rx, ry, i);
        }
        return;
    }
    const Dimension rx = opt->radiusWidth.value();
    const Dimension ry = opt->radiusHeight.value();
    RectModelNG::SetRadiusWidth(frameNode, rx);
    RectModelNG::SetRadiusHeight(frameNode, ry);
}
} // RectInterfaceModifier
namespace RectAttributeModifier {
void RadiusWidthImpl(Ark_NativePointer node,
                     const Opt_Union_Number_String* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto radiusWidth = Converter::OptConvert<Dimension>(*value);
    CHECK_NULL_VOID(radiusWidth);
    RectModelNG::SetRadiusWidth(frameNode, radiusWidth.value());
}
void RadiusHeightImpl(Ark_NativePointer node,
                      const Opt_Union_Number_String* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto radiusHeight = Converter::OptConvert<Dimension>(*value);
    CHECK_NULL_VOID(radiusHeight);
    RectModelNG::SetRadiusHeight(frameNode, radiusHeight.value());
}
void RadiusImpl(Ark_NativePointer node,
                const Opt_Union_Number_String_Array_Union_Number_String* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto radius = Converter::OptConvert<RectRadius>(*value);
    CHECK_NULL_VOID(radius);
    CHECK_NULL_VOID(radius->radiusHeight);
    CHECK_NULL_VOID(radius->radiusWidth);
    RectModelNG::SetRadiusWidth(frameNode, radius->radiusWidth.value());
    RectModelNG::SetRadiusHeight(frameNode, radius->radiusHeight.value());
}
} // RectAttributeModifier
const GENERATED_ArkUIRectModifier* GetRectModifier()
{
    static const GENERATED_ArkUIRectModifier ArkUIRectModifierImpl {
        RectModifier::ConstructImpl,
        RectInterfaceModifier::SetRectOptionsImpl,
        RectAttributeModifier::RadiusWidthImpl,
        RectAttributeModifier::RadiusHeightImpl,
        RectAttributeModifier::RadiusImpl,
    };
    return &ArkUIRectModifierImpl;
}

}
