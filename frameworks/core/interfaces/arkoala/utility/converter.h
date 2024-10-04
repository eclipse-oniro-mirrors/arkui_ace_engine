/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GENERATED_FOUNDATION_ACE_FRAMEWORKS_CORE_UTILITY_CONVERTER_H
#define GENERATED_FOUNDATION_ACE_FRAMEWORKS_CORE_UTILITY_CONVERTER_H

#pragma once

// SORTED_SECTION
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// SORTED_SECTION
#include "base/geometry/axis.h"
#include "base/geometry/calc_dimension.h"
#include "base/geometry/dimension.h"
#include "bridge/common/utils/utils.h"
#include "core/animation/chain_animation.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/paint_state.h"
#include "core/components/common/properties/shadow.h"
#include "core/components/common/properties/text_style.h"
#include "core/components_ng/pattern/list/list_item_group_pattern.h"
#include "core/components_ng/pattern/menu/menu_layout_property.h"
#include "core/components_ng/pattern/navigation/navigation_declaration.h"
#include "core/components_ng/pattern/scrollable/scrollable_properties.h"
#include "core/components_ng/pattern/text_field/text_field_model.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_v2/list/list_properties.h"
#include "core/image/image_source_info.h"

#include "ace_engine_types.h"
#include "core/interfaces/native/node/node_api.h"
#include "core/interfaces/arkoala/utility/generated/converter_generated.h"

namespace OHOS::Ace::NG {
template<typename T>
std::optional<int32_t> EnumToInt(const std::optional<T>& src)
{
    return src ? std::optional(static_cast<int32_t>(src.value())) : std::nullopt;
}

using StringArray = std::vector<std::string>;

namespace Converter {
    template<typename To, typename From>
    To Convert(const From& src);
    template<typename To, typename From>
    std::optional<To> OptConvert(const From& value);

     //Allow conversion for Ark_Xxx type to same Ark_Xxx type
    template<typename T>
    void AssignTo(T& dst, const T& src)
    {
        dst = src;
    }

    template<typename T, typename... Types>
    void AssignTo(std::variant<Types...>& dst, const T& src)
    {
        dst = src;
    }

    template<typename To, typename From, typename = decltype(From().array), typename = decltype(From().length)>
    void AssignTo(std::vector<To>& dst, const From& src)
    {
        dst.clear();
        dst.reserve(src.length);
        for (Ark_Int32 i = 0; i < src.length; i++) {
            dst.push_back(Convert<To>(src.array[i]));
        }
    }

    template<typename To, typename From, typename = decltype(From().array), typename = decltype(From().length)>
    void AssignTo(std::vector<std::optional<To>>& dst, const From& src)
    {
        dst.clear();
        dst.reserve(src.length);
        for (Ark_Int32 i = 0; i < src.length; i++) {
            dst.push_back(OptConvert<To>(src.array[i]));
        }
    }

    template<typename To, typename From>
    To Convert(const From& src)
    {
        To result;
        AssignTo(result, src);
        return result;
    }

    class ResourceConverter {
        public:
            ResourceConverter() = delete;
            ~ResourceConverter() = default;
            ResourceConverter(const ResourceConverter&) = delete;
            ResourceConverter& operator=(const ResourceConverter&) = delete;

            explicit ResourceConverter(const Ark_Resource& resource);

            std::optional<std::string> ToString();
            std::optional<StringArray> ToStringArray();
            std::optional<Dimension> ToDimension();
            std::optional<float> ToFloat();
            std::optional<int32_t> ToInt();
            std::optional<Color> ToColor();

            inline const char* BundleName() { return bundleName_.c_str(); }
            inline const char* ModuleName() { return moduleName_.c_str(); }

        private:
            RefPtr<ThemeConstants> themeConstants_;
            NodeModifier::ResourceType type_;
            std::string bundleName_;
            std::string moduleName_;
            int32_t id_;
            StringArray params_;
    };

    template<typename T, typename P>
    void AssignCast(std::optional<T>& dst, const P& src)
    {
        dst = Convert<T, P>(src);
    }

    template<typename T>
    void AssignCast(std::optional<T>& dst, const Ark_Undefined&)
    {
        dst = std::nullopt;
    }

    template<typename T, typename P>
    void AssignLiteralTo(std::optional<T>& dst, const P& src)
    {
        AssignCast(dst, src);
    }

    template<typename T, typename P>
    void AssignUnionTo(std::optional<T>& dst, const P& src)
    {
        AssignCast(dst, src);
    }

    template<typename T, typename P>
    void AssignOptionalTo(std::optional<T>& dst, const P& src)
    {
        AssignUnionTo(dst, src);
    }

    template<typename T, typename P>
    void AssignTo(std::optional<T>& dst, const P& src)
    {
        AssignOptionalTo(dst, src);
    }

    template<typename To, typename From>
    std::optional<To> OptConvert(const From& value)
    {
        std::optional<To> opt;
        AssignTo(opt, value);
        return std::move(opt);
    }

    template<typename To, typename From>
    To ConvertOrDefault(const From& value, To defaultValue)
    {
        return OptConvert<To, From>(value).value_or(defaultValue);
    }

    // Base converters
    template<>
    inline bool Convert(const Ark_Boolean& src)
    {
        return static_cast<bool>(src);
    }

    template<>
    inline std::string Convert(const Ark_String& src)
    {
        return (src.chars != nullptr) ? src.chars : "";
    }

    template<>
    inline Ark_CharPtr Convert(const Ark_String& src)
    {
        return src.chars;
    }

    template<>
    inline int Convert(const Ark_Number& src)
    {
        return src.tag == ARK_TAG_FLOAT32 ? static_cast<int>(src.f32) : src.i32;
    }

    template<>
    inline float Convert(const Ark_Number& src)
    {
        return src.tag == ARK_TAG_FLOAT32 ? src.f32 : static_cast<float>(src.i32);
    }

    template<>
    inline ImageSourceInfo Convert(const Ark_String& value)
    {
        return ImageSourceInfo(value.chars);
    }

    template<>
    inline ImageSourceInfo Convert(const Ark_CustomObject& value)
    {
        LOGE("Convert [Ark_CustomObject] to [ImageSourceInfo] is not supported");
        return ImageSourceInfo();
    }

    template<>
    inline void AssignCast(std::optional<StringArray>& dst, const Ark_Resource& value)
    {
        ResourceConverter converter(value);
        dst = converter.ToStringArray();
    }

    template<>
    inline void AssignCast(std::optional<std::string>& dst, const Ark_Resource& value)
    {
        ResourceConverter converter(value);
        dst = converter.ToString();
    }

    template<>
    inline void AssignCast(std::optional<ImageSourceInfo>& dst, const Ark_Resource& value)
    {
        ResourceConverter converter(value);
        auto resourceString = converter.ToString();
        if (resourceString) {
            dst = ImageSourceInfo(resourceString.value(), converter.BundleName(), converter.ModuleName());
        } else {
            LOGE("Not a string resource: %{public}s:%{public}s\n", converter.BundleName(), converter.ModuleName());
        }
    }

    template<>
    inline void AssignCast(std::optional<Dimension>& dst, const Ark_Resource& src)
    {
        ResourceConverter converter(src);
        dst = converter.ToDimension();
    }

    template<>
    inline void AssignCast(std::optional<Color>& dst, const Ark_Resource& src)
    {
        ResourceConverter converter(src);
        dst = converter.ToColor();
    }

    template<>
    inline void AssignCast(std::optional<Dimension>& dst, const Ark_CustomObject& src)
    {
        LOGE("ARKOALA Converter Ark_CustomObject -> Dimension is not implemented.");
    }

    template<>
    inline void AssignCast(std::optional<Ark_CharPtr>& dst, const Ark_Resource& src)
    {
        LOGE("ARKOALA Converter -> Resource support (String) is not implemented.");
    }

    template<>
    inline void AssignCast(std::optional<Color>& dst, const Ark_ColoringStrategy& src)
    {
        LOGE("ARKOALA Converter Ark_ColoringStrategy -> Color is not implemented.");
    }

    template<>
    inline void AssignCast(std::optional<float>& dst, const Ark_Resource& src)
    {
        ResourceConverter converter(src);
        dst = converter.ToFloat();
    }

    template<>
    inline void AssignCast(std::optional<int32_t>& dst, const Ark_Resource& src)
    {
        ResourceConverter converter(src);
        dst = converter.ToInt();
    }

    // Converter implementations
    template<>
    inline TextOverflow Convert(const Ark_TextOverflow& src)
    {
        return static_cast<TextOverflow>(src);
    }

    template<>
    inline TextHeightAdaptivePolicy Convert(const Ark_TextHeightAdaptivePolicy& src)
    {
        return static_cast<TextHeightAdaptivePolicy>(src);
    }

    template<>
    inline Dimension Convert(const Ark_String& src)
    {
        return Dimension::FromString(src.chars);
    }

    template<>
    Dimension Convert(const Ark_Length& src);

    template<>
    inline std::pair<Dimension, Dimension> Convert(const Tuple_Length_Length& src)
    {
        return { Converter::Convert<Dimension>(src.value0), Converter::Convert<Dimension>(src.value1) };
    }

    template<>
    inline Dimension Convert(const Ark_Number& src)
    {
        return Dimension(Converter::Convert<float>(src), DimensionUnit::VP);
    }

    template<>
    inline int Convert(const Ark_IlluminatedType& src)
    {
        return static_cast<int>(src);
    }

    template<>
    inline ImageInterpolation Convert(const Ark_ImageInterpolation& src)
    {
        return static_cast<ImageInterpolation>(src);
    }

    template<>
    inline DynamicRangeMode Convert(const Ark_DynamicRangeMode& src)
    {
        return static_cast<DynamicRangeMode>(src);
    }

    template<>
    inline ImageRenderMode Convert(const Ark_ImageRenderMode& src)
    {
        return static_cast<ImageRenderMode>(src);
    }

    template<>
    inline ImageRepeat Convert(const Ark_ImageRepeat& src)
    {
        return static_cast<ImageRepeat>(src);
    }

    template<>
    inline ImageFit Convert(const Ark_ImageFit& src)
    {
        return static_cast<ImageFit>(src);
    }

    template<>
    inline StringArray Convert(const Ark_String& src)
    {
        return (src.chars != nullptr)  ? StringArray(1, src.chars) : StringArray();
    }

    template<>
    inline StringArray Convert(const Ark_CustomObject& src)
    {
        LOGE("Convert [Ark_CustomObject] to [StringArray] is not supported");
        return StringArray();
    }

    template<>
    inline Color Convert(const Ark_Number& src)
    {
        uint32_t value = static_cast<uint32_t>(Convert<int>(src));
        return Color((value <= 0xFFFFFF && value > 0) ? value + 0xFF000000U : value);
    }

    template<>
    inline Color Convert(const Ark_String& src)
    {
        return Color::FromString(src.chars);
    }

    template<> CalcLength Convert(const Ark_Length& src);

    template<>
    inline EdgesParam Convert(const Ark_Edges& src)
    {
        EdgesParam edges;
        edges.left = OptConvert<Dimension>(src.left);
        edges.top = OptConvert<Dimension>(src.top);
        edges.right = OptConvert<Dimension>(src.right);
        edges.bottom = OptConvert<Dimension>(src.bottom);
        return edges;
    }

    template<>
    inline PaddingProperty Convert(const Ark_Length& src)
    {
        auto value = OptConvert<CalcLength>(src);
        return { .left = value, .right = value, .top = value, .bottom = value
        };
    }

    template<> PaddingProperty Convert(const Ark_Padding& src);

    template<>
    inline PaddingProperty Convert(const Ark_LocalizedPadding& src)
    {
        LOGE("Convert [Ark_LocalizedPadding] to [PaddingProperty] is not supported.");
        return PaddingProperty();
    }
    template<>
    inline RadioStyle Convert(const Ark_RadioStyle& src)
    {
        return { .checkedBackgroundColor = Converter::OptConvert<Color>(src.checkedBackgroundColor),
            .uncheckedBorderColor = Converter::OptConvert<Color>(src.uncheckedBorderColor),
            .indicatorColor = Converter::OptConvert<Color>(src.indicatorColor)
        };
    }

    template<>
    inline Dimension Convert(const Ark_CustomObject& src)
    {
        LOGE("Convert [Ark_CustomObject] to [Dimension] is not supported");
        return Dimension();
    }

    template<>
    inline DimensionOffset Convert(const Ark_Offset& src)
    {
        return DimensionOffset(Convert<Dimension>(src.dx), Convert<Dimension>(src.dy));
    }

    template<>
    inline FontMetaData Convert(const Ark_Font& src)
    {
        return { OptConvert<Dimension>(src.size), OptConvert<FontWeight>(src.weight) };
    }

    template<>
    inline Ark_NativePointer Convert(const Ark_Materialized& src)
    {
        return src.ptr;
    }


    template<>
    inline ShadowType Convert(const Ark_ShadowType& src)
    {
        return static_cast<ShadowType>(src);
    }

    // SORTED_SECTION: Converter's specializations. No multiline declarations, please!
    template<> CaretStyle Convert(const Ark_CaretStyle& src);
    template<> Font Convert(const Ark_Font& src);
    template<> ItemDragInfo Convert(const Ark_ItemDragInfo& src);
    template<> ListItemIndex Convert(const Ark_VisibleListContentInfo& src);
    template<> RefPtr<Curve> Convert(const Ark_Curve& src);
    template<> RefPtr<Curve> Convert(const Ark_ICurve& src);
    template<> RefPtr<Curve> Convert(const Ark_String& src);
    template<> Shadow Convert(const Ark_ShadowOptions& src);
    template<> TextDecorationOptions Convert(const Ark_TextDecorationOptions& src);
    template<> std::pair<Dimension, Dimension> Convert(const Ark_LengthConstrain& src);
    template<> std::vector<Shadow> Convert(const Ark_ShadowOptions& src);

    // SORTED_SECTION: Non-enum specializations. No multiline declarations, please!
    template<> void AssignCast(std::optional<FontWeight>& dst, const Ark_Number& src);
    template<> void AssignCast(std::optional<FontWeight>& dst, const Ark_String& src);
    template<> void AssignCast(std::optional<float>& dst, const Ark_String& src);

    // SORTED_SECTION: Enums specializations. No multiline declarations, please!
    template<> void AssignCast(std::optional<Alignment>& dst, const Ark_Alignment& src);
    template<> void AssignCast(std::optional<Axis>& dst, const Ark_Axis& src);
    template<> void AssignCast(std::optional<BarPosition>& dst, const Ark_BarPosition& src);
    template<> void AssignCast(std::optional<BlurStyle>& dst, const Ark_BlurStyle& src);
    template<> void AssignCast(std::optional<ButtonRole>& dst, const Ark_ButtonRole& src);
    template<> void AssignCast(std::optional<ButtonStyleMode>& dst, const Ark_ButtonStyleMode& src);
    template<> void AssignCast(std::optional<ButtonType>& dst, const Ark_ButtonType& src);
    template<> void AssignCast(std::optional<CancelButtonStyle>& dst, const Ark_CancelButtonStyle& src);
    template<> void AssignCast(std::optional<ChainEdgeEffect>& dst, const Ark_ChainEdgeEffect& src);
    template<> void AssignCast(std::optional<Color>& dst, const enum Ark_Color& src);
    template<> void AssignCast(std::optional<ControlSize>& dst, const Ark_ControlSize& src);
    template<> void AssignCast(std::optional<CopyOptions>& dst, const Ark_CopyOptions& src);
    template<> void AssignCast(std::optional<DisplayMode>& dst, const Ark_BarState& src);
    template<> void AssignCast(std::optional<EdgeEffect>& dst, const Ark_EdgeEffect& src);
    template<> void AssignCast(std::optional<FlexDirection>& dst, const Ark_GridDirection& src);
    template<> void AssignCast(std::optional<FontWeight>& dst, const Ark_FontWeight& src);
    template<> void AssignCast(std::optional<ForegroundColorStrategy>& dst, const Ark_ColoringStrategy& src);
    template<> void AssignCast(std::optional<LineCap>& dst, const Ark_LineCapStyle& src);
    template<> void AssignCast(std::optional<LineCapStyle>& dst, const Ark_LineCapStyle& src);
    template<> void AssignCast(std::optional<LineJoinStyle>& dst, const Ark_LineJoinStyle& src);
    template<> void AssignCast(std::optional<ListItemGroupArea>& dst, const Ark_ListItemGroupArea& src);
    template<> void AssignCast(std::optional<NavRouteMode>& dst, const Ark_NavRouteMode& src);
    template<> void AssignCast(std::optional<NestedScrollMode>& dst, const Ark_NestedScrollMode& src);
    template<> void AssignCast(std::optional<OHOS::Ace::FontStyle>& dst, const Ark_FontStyle& src);
    template<> void AssignCast(std::optional<ScrollState>& dst, const Ark_ScrollState& src);
    template<> void AssignCast(std::optional<ShadowColorStrategy>& dst, const Ark_ColoringStrategy& src);
    template<> void AssignCast(std::optional<SubMenuExpandingMode>& dst, const Ark_SubMenuExpandingMode& src);
    template<> void AssignCast(std::optional<TabAnimateMode>& dst, const Ark_AnimationMode& src);
    template<> void AssignCast(std::optional<TabBarMode>& dst, const Ark_BarMode& src);
    template<> void AssignCast(std::optional<TextAlign>& dst, const Ark_TextAlign& src);
    template<> void AssignCast(std::optional<TextCase>& dst, const Ark_TextCase& src);
    template<> void AssignCast(std::optional<TextContentType>& dst, const Ark_ContentType& src);
    template<> void AssignCast(std::optional<TextDecoration>& dst, const Ark_TextDecorationType& src);
    template<> void AssignCast(std::optional<TextDecorationStyle>& dst, const Ark_TextDecorationStyle& src);
    template<> void AssignCast(std::optional<TextDeleteDirection>& dst, const Ark_TextDeleteDirection& src);
    template<> void AssignCast(std::optional<TextInputAction>& dst, const Ark_EnterKeyType& src);
    template<> void AssignCast(std::optional<TextInputType>& dst, const Ark_SearchType& src);
    template<> void AssignCast(std::optional<TextInputType>& dst, const Ark_TextAreaType& src);
    template<> void AssignCast(std::optional<V2::EditMode>& dst, const Ark_EditMode& src);
    template<> void AssignCast(std::optional<V2::ListItemAlign>& dst, const Ark_ListItemAlign& src);
    template<> void AssignCast(std::optional<V2::ListItemGroupStyle>& dst, const Ark_ListItemGroupStyle& src);
    template<> void AssignCast(std::optional<V2::ListItemStyle>& dst, const Ark_ListItemStyle& src);
    template<> void AssignCast(std::optional<V2::ScrollSnapAlign>& dst, const Ark_ScrollSnapAlign& src);
    template<> void AssignCast(std::optional<V2::StickyMode>& dst, const Ark_Sticky& src);
    template<> void AssignCast(std::optional<V2::StickyStyle>& dst, const Ark_StickyStyle& src);
    template<> void AssignCast(std::optional<V2::SwipeEdgeEffect>& dst, const Ark_SwipeEdgeEffect& src);

    template<>
    void AssignCast(std::optional<SharedTransitionEffectType>& dst, const Ark_SharedTransitionEffectType& src);
} // namespace OHOS::Ace::NG::Converter
} // namespace OHOS::Ace::NG

#endif  // GENERATED_FOUNDATION_ACE_FRAMEWORKS_CORE_UTILITY_CONVERTER_H
