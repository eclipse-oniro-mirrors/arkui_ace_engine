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
#include "core/components_ng/pattern/symbol/symbol_model_ng.h"
#include "core/interfaces/arkoala/utility/validators.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG{
namespace Converter {
template<>
void AssignCast(std::optional<int32_t>& dst, const Ark_SymbolRenderingStrategy& src)
{
    switch (src) {
        case ARK_SYMBOL_RENDERING_STRATEGY_SINGLE: dst = 0; break;
        case ARK_SYMBOL_RENDERING_STRATEGY_MULTIPLE_COLOR: dst = 1; break;
        case ARK_SYMBOL_RENDERING_STRATEGY_MULTIPLE_OPACITY: dst = 2; break;
        default: LOGE("Unexpected enum value in Ark_SymbolRenderingStrategy: %{public}d", src);
    }
}

template<>
void AssignCast(std::optional<int32_t>& dst, const Ark_SymbolEffectStrategy& src)
{
    switch (src) {
        case ARK_SYMBOL_EFFECT_STRATEGY_NONE: dst = 0; break;
        case ARK_SYMBOL_EFFECT_STRATEGY_SCALE: dst = 1; break;
        case ARK_SYMBOL_EFFECT_STRATEGY_HIERARCHICAL: dst = 2; break;
        default: LOGE("Unexpected enum value in Ark_SymbolEffectStrategy: %{public}d", src);
    }
}

}

namespace GeneratedModifier {
namespace SymbolGlyphInterfaceModifier {
void SetSymbolGlyphOptionsImpl(Ark_NativePointer node,
                               const Opt_Resource* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    std::uint32_t convValue;
    Ark_Resource res = (*value).value;
    if (res.id.i32 != 0) {
        convValue = res.id.f32;
    } else {
        // check attribute for resource name
    }
    SymbolModelNG::InitialSymbol(frameNode, convValue);
}
} // SymbolGlyphInterfaceModifier
namespace SymbolGlyphAttributeModifier {
void FontSizeImpl(Ark_NativePointer node,
                  const Ark_Union_Number_String_Resource* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto convValue = Converter::OptConvert<Dimension>(*value);
    Validator::ValidateNonNegative(convValue);
    Validator::ValidateNonPercent(convValue);
    SymbolModelNG::SetFontSize(frameNode, convValue);
}
void FontColorImpl(Ark_NativePointer node,
                   const Array_ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto optFontColors = Converter::Convert<std::vector<std::optional<Color>>>(*value);
    std::vector<Color> fontColors;
    for (auto color : optFontColors) {
        if (color.has_value())
            fontColors.emplace_back(color.value());
    };
    SymbolModelNG::SetFontColor(frameNode, fontColors);
}
void FontWeightImpl(Ark_NativePointer node,
                    const Ark_Union_Number_FontWeight_String* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto convValue = Converter::OptConvert<Ace::FontWeight>(*value);
    SymbolModelNG::SetFontWeight(frameNode, convValue);
}
void EffectStrategyImpl(Ark_NativePointer node,
                        Ark_SymbolEffectStrategy value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<int32_t>(value);
    SymbolModelNG::SetSymbolEffect(frameNode, convValue);
}
void RenderingStrategyImpl(Ark_NativePointer node,
                           Ark_SymbolRenderingStrategy value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<int32_t>(value);
    SymbolModelNG::SetRenderingStrategy(frameNode, convValue);
}
void SymbolEffect0Impl(Ark_NativePointer node,
                       const Ark_SymbolEffect* symbolEffect,
                       const Opt_Boolean* isActive)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(symbolEffect);
    //SymbolModelNG::SetSymbolEffectOptions(frameNode, convValue);
}
void SymbolEffect1Impl(Ark_NativePointer node,
                       const Ark_SymbolEffect* symbolEffect,
                       const Opt_Number* triggerValue)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(symbolEffect);
    //SymbolModelNG::SetSymbolEffectOptions(frameNode, convValue);
}
} // SymbolGlyphAttributeModifier
const GENERATED_ArkUISymbolGlyphModifier* GetSymbolGlyphModifier()
{
    static const GENERATED_ArkUISymbolGlyphModifier ArkUISymbolGlyphModifierImpl {
        SymbolGlyphInterfaceModifier::SetSymbolGlyphOptionsImpl,
        SymbolGlyphAttributeModifier::FontSizeImpl,
        SymbolGlyphAttributeModifier::FontColorImpl,
        SymbolGlyphAttributeModifier::FontWeightImpl,
        SymbolGlyphAttributeModifier::EffectStrategyImpl,
        SymbolGlyphAttributeModifier::RenderingStrategyImpl,
        SymbolGlyphAttributeModifier::SymbolEffect0Impl,
        SymbolGlyphAttributeModifier::SymbolEffect1Impl,
    };
    return &ArkUISymbolGlyphModifierImpl;
}
}
}
