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

#include "core/interfaces/native/utility/converter.h"
#include "arkoala_api_generated.h"

#include "core/components_ng/pattern/shape/shape_abstract_model_ng.h"
#include "core/interfaces/native/generated/interface/node_api.h"

namespace OHOS::Ace::NG::Converter {
struct CircleOptions {
    std::optional<Dimension> width;
    std::optional<Dimension> height;
};

template<>
inline CircleOptions Convert(const Ark_CircleOptions& src)
{
    CircleOptions circleOptions;
    circleOptions.width = Converter::OptConvert<Dimension>(src.width);
    circleOptions.height = Converter::OptConvert<Dimension>(src.height);
    return circleOptions;
}
} // namespace OHOS::Ace::NG::Converter

namespace OHOS::Ace::NG::GeneratedModifier {
namespace CircleInterfaceModifier {
void SetCircleOptionsImpl(Ark_NativePointer node,
                          const Opt_CircleOptions* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto circleOptions = Converter::OptConvert<Converter::CircleOptions>(*value);
    if (circleOptions && circleOptions.value().width) {
        ShapeAbstractModelNG::SetWidth(frameNode, circleOptions.value().width.value());
    }
    if (circleOptions && circleOptions.value().height) {
        ShapeAbstractModelNG::SetHeight(frameNode, circleOptions.value().height.value());
    }
}
} // CircleInterfaceModifier
const GENERATED_ArkUICircleModifier* GetCircleModifier()
{
    static const GENERATED_ArkUICircleModifier ArkUICircleModifierImpl {
        CircleInterfaceModifier::SetCircleOptionsImpl,
    };
    return &ArkUICircleModifierImpl;
}

}