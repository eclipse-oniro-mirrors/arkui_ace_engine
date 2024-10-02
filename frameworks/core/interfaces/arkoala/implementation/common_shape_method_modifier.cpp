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
#include "core/components_ng/pattern/shape/shape_model_ng.h"
#include "core/interfaces/arkoala/utility/converter.h"
#include "core/interfaces/arkoala/utility/validators.h"
#include "core/components/common/properties/paint_state.h"

namespace {
    static const double STROKE_MITER_LIMIT_MIN_VALUE = 1.0;
    void validateStrokeMiterLimit(std::optional<float>& limit)
    {
        if (limit && limit.value() < STROKE_MITER_LIMIT_MIN_VALUE) {
            limit = 1.0;
        }
    }
} // namespace

namespace OHOS::Ace::NG::GeneratedModifier {
namespace CommonShapeMethodModifier {
void StrokeImpl(Ark_NativePointer node,
                const ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    ShapeModelNG::SetStroke(frameNode, Converter::OptConvert<Color>(*value));
}
void FillImpl(Ark_NativePointer node,
              const ResourceColor* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    ShapeModelNG::SetFill(frameNode, Converter::OptConvert<Color>(*value));
}
void StrokeDashOffsetImpl(Ark_NativePointer node,
                          const Type_CommonShapeMethod_strokeDashOffset_Arg0* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto strokeDashOffset = Converter::OptConvert<Dimension>(*value);
    Validator::ValidatePositive(strokeDashOffset);
    Validator::ValidateNonPercent(strokeDashOffset);
    ShapeModelNG::SetStrokeDashOffset(frameNode, strokeDashOffset);
}
void StrokeLineCapImpl(Ark_NativePointer node,
                       enum Ark_LineCapStyle value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto enumLineCapStyle = Converter::OptConvert<LineCapStyle>(value);
    auto intLineCapStyle = EnumToInt(enumLineCapStyle);
    ShapeModelNG::SetStrokeLineCap(frameNode, intLineCapStyle);
}
void StrokeLineJoinImpl(Ark_NativePointer node,
                        enum Ark_LineJoinStyle value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto enumLineJoinStyle = Converter::OptConvert<LineJoinStyle>(value);
    auto intLineJoinStyle = EnumToInt(enumLineJoinStyle);
    ShapeModelNG::SetStrokeLineJoin(frameNode, intLineJoinStyle);
}
void StrokeMiterLimitImpl(Ark_NativePointer node,
                          const Type_CommonShapeMethod_strokeMiterLimit_Arg0* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto strokeMiterLimit = Converter::OptConvert<float>(*value);
    validateStrokeMiterLimit(strokeMiterLimit);
    ShapeModelNG::SetStrokeMiterLimit(frameNode, strokeMiterLimit);
}
void StrokeOpacityImpl(Ark_NativePointer node,
                       const Type_CommonShapeMethod_strokeOpacity_Arg0* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto strokeOpacity = Converter::OptConvert<float>(*value);
    Validator::ValidateOpacity(strokeOpacity);
    ShapeModelNG::SetStrokeOpacity(frameNode, strokeOpacity);
}
void FillOpacityImpl(Ark_NativePointer node,
                     const Type_CommonShapeMethod_fillOpacity_Arg0* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto strokeOpacity = Converter::OptConvert<float>(*value);
    Validator::ValidateOpacity(strokeOpacity);
    ShapeModelNG::SetFillOpacity(frameNode, strokeOpacity);
}
void StrokeWidthImpl(Ark_NativePointer node,
                     const Ark_Length* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto strokeWidth = Converter::OptConvert<Dimension>(*value);
    Validator::ValidatePositive(strokeWidth);
    Validator::ValidateNonPercent(strokeWidth);
    ShapeModelNG::SetStrokeWidth(frameNode, strokeWidth);
}
void AntiAliasImpl(Ark_NativePointer node,
                   Ark_Boolean value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ShapeModelNG::SetAntiAlias(frameNode, Converter::Convert<bool>(value));
}
void StrokeDashArrayImpl(Ark_NativePointer node,
                         const Array_CustomObject* value)
{
    LOGE("ARKOALA CommonShapeMethodModifier.StrokeDashArray -> Method is not implemented.");
}
} // CommonShapeMethodModifier
const GENERATED_ArkUICommonShapeMethodModifier* GetCommonShapeMethodModifier()
{
    static const GENERATED_ArkUICommonShapeMethodModifier ArkUICommonShapeMethodModifierImpl {
        CommonShapeMethodModifier::StrokeImpl,
        CommonShapeMethodModifier::FillImpl,
        CommonShapeMethodModifier::StrokeDashOffsetImpl,
        CommonShapeMethodModifier::StrokeLineCapImpl,
        CommonShapeMethodModifier::StrokeLineJoinImpl,
        CommonShapeMethodModifier::StrokeMiterLimitImpl,
        CommonShapeMethodModifier::StrokeOpacityImpl,
        CommonShapeMethodModifier::FillOpacityImpl,
        CommonShapeMethodModifier::StrokeWidthImpl,
        CommonShapeMethodModifier::AntiAliasImpl,
        CommonShapeMethodModifier::StrokeDashArrayImpl,
    };
    return &ArkUICommonShapeMethodModifierImpl;
}

}
