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

/*
 * WARNING! THIS FILE IS AUTO-GENERATED, DO NOT MAKE CHANGES, THEY WILL BE LOST ON NEXT GENERATION!
 */

#ifndef CONVERTER_GENERATED_H
#define CONVERTER_GENERATED_H

#include <optional>
#include <cstdlib>
#include "arkoala_api_generated.h"
#include "base/log/log_wrapper.h"

#define SELECTOR_ID_0 0
#define SELECTOR_ID_1 1
#define SELECTOR_ID_2 2
#define SELECTOR_ID_3 3
#define SELECTOR_ID_4 4
#define SELECTOR_ID_5 5
#define SELECTOR_ID_6 6
#define SELECTOR_ID_7 7
#define SELECTOR_ID_8 8
#define SELECTOR_ID_9 9
#define SELECTOR_ID_10 10
#define SELECTOR_ID_11 11
#define SELECTOR_ID_12 12
#define SELECTOR_ID_13 13
#define SELECTOR_ID_14 14
#define SELECTOR_ID_15 15

namespace OHOS::Ace::NG::Converter {

template<typename T, typename P>
void AssignTo(std::optional<T>& dst, const P& src);

template<typename T, typename P>
void AssignUnionTo(std::optional<T>& dst, const P& src);

template<typename T, typename P>
void AssignOptionalTo(std::optional<T>& dst, const P& src);

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_Resource& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Color_String_Resource_ColoringStrategy& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Dimension_BorderRadiuses& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_ResourceColor& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_LengthMetrics_BorderRadiuses& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_LengthMetrics_Padding& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_LengthMetrics_Margin& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Dimension_Margin& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_String_Resource& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_FontWeight_Number_String& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_ResourceStr& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_LeadingMarginPlaceholder& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Curve_ICurve& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_FontWeight_String& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_String_Resource& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_String& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ShadowOptions_Array_ShadowOptions& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Length_Number& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_ResourceStr& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Padding_Dimension& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ResourceStr_ComponentContent& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Dimension_LeadingMarginPlaceholder& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_VP& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Length_GridRowSizeOption& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Length_BorderRadiuses& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Resource_String& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Curve_String_ICurve& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ShadowOptions_ShadowStyle& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_BorderStyle_EdgeStyles& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ResourceColor_EdgeColors_LocalizedEdgeColors& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Dimension_EdgeWidths_LocalizedEdgeWidths& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Dimension_BorderRadiuses_LocalizedBorderRadiuses& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_String_PixelMap_Resource& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ResourceColor_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Padding_Dimension_LocalizedPadding& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ResourceStr_TabBarSymbol& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Union_Padding_Dimension_LocalizedPadding& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ResourceStr_Union_ResourceStr_ComponentContent& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_String_Array_Any& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_PX_VP_LPX_Resource& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_TitleHeight_Length& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_String_Number_CanvasGradient_CanvasPattern& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_SheetSize_Length& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Boolean_Callback_DismissPopupAction_Void& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Boolean_Literal_ResourceColor_color& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Color_String_Resource_Number& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_StyledStringValue& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        case SELECTOR_ID_4: AssignTo(dst, src.value4); break;
        case SELECTOR_ID_5: AssignTo(dst, src.value5); break;
        case SELECTOR_ID_6: AssignTo(dst, src.value6); break;
        case SELECTOR_ID_7: AssignTo(dst, src.value7); break;
        case SELECTOR_ID_8: AssignTo(dst, src.value8); break;
        case SELECTOR_ID_9: AssignTo(dst, src.value9); break;
        case SELECTOR_ID_10: AssignTo(dst, src.value10); break;
        case SELECTOR_ID_11: AssignTo(dst, src.value11); break;
        case SELECTOR_ID_12: AssignTo(dst, src.value12); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Margin_Dimension& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_Array_Number& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_String_Array_String& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Type_TextPickerOptions_range& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        case SELECTOR_ID_4: AssignTo(dst, src.value4); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ScrollAnimationOptions_Boolean& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ResourceColor_EdgeColors& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Dimension_EdgeWidths& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_String_WebResourceRequest& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_WebController_WebviewController& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_String_PlaybackSpeed& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Type_SliderBlockStyle_shape& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_DividerOptions_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Dimension_Array_Dimension& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_String_Number& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ResourceStr_CustomBuilder& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_CustomBuilder_SwipeActionItem& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_String_Resource_PixelMap& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_GridRowColumnOption& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Length_GutterOption& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_GridColColumnOption& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ResourceColor_LinearGradient& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ResourceStr_Scene& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_BadgePosition_Position& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_SheetTitleOptions_CustomBuilder& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Boolean_Resource& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Length_BorderRadiuses_LocalizedBorderRadiuses& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_MenuPreviewMode_CustomBuilder& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Boolean_Number& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_DragPreviewMode_Array_DragPreviewMode& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_Literal_Number_offset_span& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_EdgeOutlineStyles_OutlineStyle& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_OutlineRadiuses_Dimension& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_EdgeColors_ResourceColor_LocalizedEdgeColors& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_EdgeOutlineWidths_Dimension& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Length_EdgeWidths_LocalizedEdgeWidths& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_String_Resource_LinearGradient& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_EdgeWidths_LengthMetrics_LocalizedEdgeWidths& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_EdgeStyles_BorderStyle& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_BorderRadiuses_Length_LocalizedBorderRadiuses& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_EdgeWidths_Length_LocalizedEdgeWidths& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_String_ImageAttachment_CustomSpan& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_String_Number_Resource_ArrayBuffer& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Array_Number_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Type_RichEditorController_updateSpanStyle_value& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_PixelMap_ResourceStr& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_NavDestinationContext_NavBar& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_OnMoveHandler_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_String_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ImageBitmap_PixelMap& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Type_AlertDialog_show_value& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_OnLinearIndicatorChangeCallback_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Boolean_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_LinearIndicatorStyle_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_DividerStyle_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_NestedScrollOptions_NestedScrollOptionsExt& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ResourceColor_UnderlineColor_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ResourceStr_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_TextInputStyle_TextContentStyle& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_DateTimeOptions_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_SubTabBarStyle_BottomTabBarStyle& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Type_TabContentAttribute_tabBar_value& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_TabContentAnimatedTransition_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_EdgeEffect_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_AnimationMode_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_String_SwiperAutoFill& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ArrowStyle_Boolean& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_DotIndicator_DigitIndicator_Boolean& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Union_DividerOptions_Undefined_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Dimension_OptionWidthMode& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_CancelButtonOptions_CancelButtonSymbolOptions& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_IconOptions_SymbolGlyphModifier& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Color_Number_String& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ResponseType_RichEditorResponseType& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_RichEditorTextSpanResult_RichEditorImageSpanResult& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Type_RectInterface_value& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_LinearStyleOptions_RingStyleOptions_CapsuleStyleOptions_ProgressStyleOptions& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_CircleStyleOptions_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Dimension_PanelHeight& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Array_ToolbarItem_CustomBuilder& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Type_NavigationAttribute_title_value& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_SystemBarStyle_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_NavigationAnimatedTransition_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Object_CustomBuilder& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Array_NavigationMenuItem_CustomBuilder& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_String_PixelMap_Resource_SymbolGlyphModifier& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Type_NavDestinationAttribute_title_value& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        case SELECTOR_ID_4: AssignTo(dst, src.value4); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ResourceStr_PixelMap_SymbolGlyphModifier& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Boolean_ResourceStr_SymbolGlyphModifier& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_MenuItemOptions_CustomBuilder& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_DividerStyleOptions_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_PixelMap_ResourceStr_DrawableDescriptor_ASTCResource& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Position_Edges_LocalizedEdges& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ListDividerOptions_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Boolean_EditMode& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_LengthConstrain& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ColorFilter_DrawingColorFilter& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ResourceStr_PixelMap& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_PixelMap_ResourceStr_DrawableDescriptor& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_PixelMap_ResourceStr_DrawableDescriptor_ImageContent& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Color_Number_String_Resource& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_GridItemAlignment_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Type_GaugeAttribute_colors_colors& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ColumnSplitDividerStyle_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_CanvasRenderingContext2D_DrawingRenderingContext& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_SpringMotion_FrictionMotion_ScrollMotion& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_String_FunctionKey& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Array_MenuElement_CustomBuilder& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_PopupOptions_CustomPopupOptions& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_BlendMode_Blender& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_String_CustomBuilder_ComponentContent& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_TransitionFinishCallback_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Object_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_CircleShape_EllipseShape_PathShape_RectShape& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Type_CommonMethod_mask_value& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        case SELECTOR_ID_4: AssignTo(dst, src.value4); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Type_CommonMethod_clip_value& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        case SELECTOR_ID_3: AssignTo(dst, src.value3); break;
        case SELECTOR_ID_4: AssignTo(dst, src.value4); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_CustomBuilder_DragItemInfo_String& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Array_UniformDataType_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_CustomBuilder_DragItemInfo& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ClickEffect_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Position_LocalizedPosition& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Number_InvertOptions& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Color_String_Resource& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_TransitionOptions_TransitionEffect& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_ResourceColor_ColoringStrategy& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Dimension_OutlineRadiuses& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Dimension_EdgeOutlineWidths& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_OutlineStyle_EdgeOutlineStyles& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Position_Alignment& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_SizeOptions_ImageSize& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Margin_Length_LocalizedMargin& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Padding_Length_LocalizedPadding& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        case SELECTOR_ID_2: AssignTo(dst, src.value2); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_Array_Rectangle_Rectangle& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T>
void AssignUnionTo(std::optional<T>& dst,
                   const Ark_Union_DrawModifier_Undefined& src)
{
    switch (src.selector) {
        case SELECTOR_ID_0: AssignTo(dst, src.value0); break;
        case SELECTOR_ID_1: AssignTo(dst, src.value1); break;
        default:
        {
            LOGE("Unexpected src->selector: %{public}d\n", src.selector);
            return;
        }
    }
}

template<typename T, typename P>
void AssignLiteralTo(std::optional<T>& dst, const P& src);

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_BorderRadiuses& src)
{
    AssignTo(dst, src.topLeft);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Padding& src)
{
    AssignTo(dst, src.top);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_EdgeStyles& src)
{
    AssignTo(dst, src.top);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_EdgeColors& src)
{
    AssignTo(dst, src.top);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_EdgeWidths& src)
{
    AssignTo(dst, src.top);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_ResourceColor_color& src)
{
    AssignTo(dst, src.color);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Offset& src)
{
    AssignTo(dst, src.dx);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_String_value_Callback_Void_action& src)
{
    AssignTo(dst, src.value);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Number_offset_span& src)
{
    AssignTo(dst, src.span);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_EdgeOutlineStyles& src)
{
    AssignTo(dst, src.top);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_OutlineRadiuses& src)
{
    AssignTo(dst, src.topLeft);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_EdgeOutlineWidths& src)
{
    AssignTo(dst, src.top);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_TransitionEffect_appear_disappear& src)
{
    AssignTo(dst, src.appear);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Type_ButtonStyle_icons& src)
{
    AssignTo(dst, src.shown);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Union_String_Resource_icon_text& src)
{
    AssignTo(dst, src.icon);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_SwiperAutoFill& src)
{
    AssignTo(dst, src.minSize);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Type_RectInterface_value_u1& src)
{
    AssignTo(dst, src.width);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Type_RectInterface_value_u0& src)
{
    AssignTo(dst, src.width);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_LengthConstrain& src)
{
    AssignTo(dst, src.minLength);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Number_day_month_year& src)
{
    AssignTo(dst, src.year);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_String_anchor_VerticalAlign_align& src)
{
    AssignTo(dst, src.anchor);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_String_anchor_HorizontalAlign_align& src)
{
    AssignTo(dst, src.anchor);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Number_surfaceHeight_surfaceWidth& src)
{
    AssignTo(dst, src.surfaceWidth);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Object_object_String_name_Array_String_methodList& src)
{
    AssignTo(dst, src.object);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Union_String_Resource_url_Array_Header_headers& src)
{
    AssignTo(dst, src.url);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_String_baseUrl_data_encoding_historyUrl_mimeType& src)
{
    AssignTo(dst, src.data);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_String_script_Callback_String_Void_callback& src)
{
    AssignTo(dst, src.script);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Boolean_next_Axis_direction& src)
{
    AssignTo(dst, src.next);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Boolean_next& src)
{
    AssignTo(dst, src.next);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Number_distance_fingers_PanDirection_direction& src)
{
    AssignTo(dst, src.fingers);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_TransitionEffects& src)
{
    AssignTo(dst, src.identity);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Number_code_Want_want& src)
{
    AssignTo(dst, src.code);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Empty& src)
{
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Type_XComponentInterface_value& src)
{
    AssignTo(dst, src.id);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_String_plainText& src)
{
    AssignTo(dst, src.plainText);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Function_callback_Object_fileSelector& src)
{
    AssignTo(dst, src.callback);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Object_detail& src)
{
    AssignTo(dst, src.detail);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Function_handler_Object_error& src)
{
    AssignTo(dst, src.handler);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Union_String_WebResourceRequest_data& src)
{
    AssignTo(dst, src.data);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Number_time& src)
{
    AssignTo(dst, src.time);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Number_duration& src)
{
    AssignTo(dst, src.duration);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Boolean_fullscreen& src)
{
    AssignTo(dst, src.fullscreen);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_CancelButtonStyle_style_IconOptions_icon& src)
{
    AssignTo(dst, src.style);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Type_TabsInterface_value& src)
{
    AssignTo(dst, src.barPosition);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Number_index& src)
{
    AssignTo(dst, src.index);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Union_Number_String_height_width_x_y& src)
{
    AssignTo(dst, src.x);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Type_SearchInterface_options& src)
{
    AssignTo(dst, src.value);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_PluginComponentTemplate_template_Any_data& src)
{
    AssignTo(dst, src.template_);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Union_Number_String_height_width_String_commands& src)
{
    AssignTo(dst, src.width);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_String_target_NavigationType_type& src)
{
    AssignTo(dst, src.target);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Type_ImageAttribute_onComplete_callback_event& src)
{
    AssignTo(dst, src.width);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Number_offsetRemain& src)
{
    AssignTo(dst, src.offsetRemain);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Number_errcode_String_msg& src)
{
    AssignTo(dst, src.errcode);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Number_height_width& src)
{
    AssignTo(dst, src.width);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Union_String_Number_height_width& src)
{
    AssignTo(dst, src.width);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Type_CalendarInterface_value& src)
{
    AssignTo(dst, src.date);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Want_want& src)
{
    AssignTo(dst, src.want);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Boolean_isVisible& src)
{
    AssignTo(dst, src.isVisible);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Alignment_align& src)
{
    AssignTo(dst, src.align);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Type_CommonMethod_radialGradient_value& src)
{
    AssignTo(dst, src.center);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Type_CommonMethod_sweepGradient_value& src)
{
    AssignTo(dst, src.center);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Type_CommonMethod_linearGradient_value& src)
{
    AssignTo(dst, src.angle);
}

template<typename T>
void AssignLiteralTo(std::optional<T>& dst,
                     const Ark_Literal_Union_Number_Literal_Number_offset_span_lg_md_sm_xs& src)
{
    AssignTo(dst, src.xs);
}


#define ASSIGN_OPT(name) \
template<typename T> \
void AssignOptionalTo(std::optional<T>& dst, const name& src) { \
    if (src.tag != ARK_TAG_UNDEFINED) { \
        AssignUnionTo(dst, src.value); \
    } \
} \
template<typename T> \
void WithOptional(const name& src, T call) { \
    if (src.tag != ARK_TAG_UNDEFINED) { \
        call(src.value); \
    } \
}
ASSIGN_OPT(Opt_Int32)
ASSIGN_OPT(Opt_Number)
ASSIGN_OPT(Opt_Array_String)
ASSIGN_OPT(Opt_String)
ASSIGN_OPT(Opt_Length)
ASSIGN_OPT(Opt_Boolean)
ASSIGN_OPT(Opt_LengthUnit)
ASSIGN_OPT(Opt_Resource)
ASSIGN_OPT(Opt_ColoringStrategy)
ASSIGN_OPT(Opt_Color)
ASSIGN_OPT(Opt_BorderRadiuses)
ASSIGN_OPT(Opt_Tuple_Dimension_Dimension)
ASSIGN_OPT(Opt_PixelMap)
ASSIGN_OPT(Opt_LengthMetrics)
ASSIGN_OPT(Opt_Padding)
ASSIGN_OPT(Opt_Union_Number_Resource)
ASSIGN_OPT(Opt_Union_Color_String_Resource_ColoringStrategy)
ASSIGN_OPT(Opt_ShadowType)
ASSIGN_OPT(Opt_FontWeight)
ASSIGN_OPT(Opt_Union_Dimension_BorderRadiuses)
ASSIGN_OPT(Opt_ResourceColor)
ASSIGN_OPT(Opt_LeadingMarginPlaceholder)
ASSIGN_OPT(Opt_Union_LengthMetrics_BorderRadiuses)
ASSIGN_OPT(Opt_Union_LengthMetrics_Padding)
ASSIGN_OPT(Opt_Union_LengthMetrics_Margin)
ASSIGN_OPT(Opt_ICurve)
ASSIGN_OPT(Opt_Curve)
ASSIGN_OPT(Opt_Union_Dimension_Margin)
ASSIGN_OPT(Opt_Array_ShadowOptions)
ASSIGN_OPT(Opt_ShadowOptions)
ASSIGN_OPT(Opt_TextDecorationStyle)
ASSIGN_OPT(Opt_TextDecorationType)
ASSIGN_OPT(Opt_BorderStyle)
ASSIGN_OPT(Opt_Position)
ASSIGN_OPT(Opt_FontStyle)
ASSIGN_OPT(Opt_Union_String_Resource)
ASSIGN_OPT(Opt_Union_FontWeight_Number_String)
ASSIGN_OPT(Opt_ResourceStr)
ASSIGN_OPT(Opt_CustomObject)
ASSIGN_OPT(Opt_GridRowSizeOption)
ASSIGN_OPT(Opt_TextBackgroundStyle)
ASSIGN_OPT(Opt_Union_Number_LeadingMarginPlaceholder)
ASSIGN_OPT(Opt_WordBreak)
ASSIGN_OPT(Opt_TextOverflow)
ASSIGN_OPT(Opt_TextAlign)
ASSIGN_OPT(Opt_ImageAttachmentLayoutStyle)
ASSIGN_OPT(Opt_ImageFit)
ASSIGN_OPT(Opt_ImageSpanAlignment)
ASSIGN_OPT(Opt_SizeOptions)
ASSIGN_OPT(Opt_Union_Curve_ICurve)
ASSIGN_OPT(Opt_SymbolRenderingStrategy)
ASSIGN_OPT(Opt_SymbolEffectStrategy)
ASSIGN_OPT(Opt_Union_Number_FontWeight_String)
ASSIGN_OPT(Opt_Array_ResourceColor)
ASSIGN_OPT(Opt_Union_Number_String_Resource)
ASSIGN_OPT(Opt_RichEditorLayoutStyle)
ASSIGN_OPT(Opt_Union_Number_String)
ASSIGN_OPT(Opt_Union_ShadowOptions_Array_ShadowOptions)
ASSIGN_OPT(Opt_DecorationStyleInterface)
ASSIGN_OPT(Opt_Union_Length_Number)
ASSIGN_OPT(Opt_Callback_PopInfo_Void)
ASSIGN_OPT(Opt_ShadowStyle)
ASSIGN_OPT(Opt_EdgeStyles)
ASSIGN_OPT(Opt_LocalizedEdgeColors)
ASSIGN_OPT(Opt_EdgeColors)
ASSIGN_OPT(Opt_LocalizedEdgeWidths)
ASSIGN_OPT(Opt_EdgeWidths)
ASSIGN_OPT(Opt_LocalizedBorderRadiuses)
ASSIGN_OPT(Opt_VoidCallback)
ASSIGN_OPT(Opt_DialogButtonStyle)
ASSIGN_OPT(Opt_Area)
ASSIGN_OPT(Opt_Undefined)
ASSIGN_OPT(Opt_LocalizedPadding)
ASSIGN_OPT(Opt_Font)
ASSIGN_OPT(Opt_TextHeightAdaptivePolicy)
ASSIGN_OPT(Opt_Union_Number_ResourceStr)
ASSIGN_OPT(Opt_TabBarSymbol)
ASSIGN_OPT(Opt_Union_Padding_Dimension)
ASSIGN_OPT(Opt_Union_ResourceStr_ComponentContent)
ASSIGN_OPT(Opt_Tuple_Number_Number)
ASSIGN_OPT(Opt_LineBreakStrategy)
ASSIGN_OPT(Opt_Union_Dimension_LeadingMarginPlaceholder)
ASSIGN_OPT(Opt_DecorationStyleResult)
ASSIGN_OPT(Opt_Array_CustomObject)
ASSIGN_OPT(Opt_VP)
ASSIGN_OPT(Opt_TitleHeight)
ASSIGN_OPT(Opt_Callback_SwipeActionState_Void)
ASSIGN_OPT(Opt_Callback_Void)
ASSIGN_OPT(Opt_Callback_Any)
ASSIGN_OPT(Opt_Union_Length_GridRowSizeOption)
ASSIGN_OPT(Opt_Array_Tuple_ResourceColor_Number)
ASSIGN_OPT(Opt_GradientDirection)
ASSIGN_OPT(Opt_CanvasPattern)
ASSIGN_OPT(Opt_CanvasGradient)
ASSIGN_OPT(Opt_SheetSize)
ASSIGN_OPT(Opt_Callback_DismissPopupAction_Void)
ASSIGN_OPT(Opt_Literal_ResourceColor_color)
ASSIGN_OPT(Opt_OutlineStyle)
ASSIGN_OPT(Opt_Map_String_CustomObject)
ASSIGN_OPT(Opt_BackgroundColorStyle)
ASSIGN_OPT(Opt_UserDataSpan)
ASSIGN_OPT(Opt_CustomSpan)
ASSIGN_OPT(Opt_UrlStyle)
ASSIGN_OPT(Opt_LineHeightStyle)
ASSIGN_OPT(Opt_ParagraphStyle)
ASSIGN_OPT(Opt_ImageAttachment)
ASSIGN_OPT(Opt_GestureStyle)
ASSIGN_OPT(Opt_TextShadowStyle)
ASSIGN_OPT(Opt_LetterSpacingStyle)
ASSIGN_OPT(Opt_BaselineOffsetStyle)
ASSIGN_OPT(Opt_DecorationStyle)
ASSIGN_OPT(Opt_TextStyle)
ASSIGN_OPT(Opt_Union_Length_BorderRadiuses)
ASSIGN_OPT(Opt_Union_Resource_String)
ASSIGN_OPT(Opt_ButtonRole)
ASSIGN_OPT(Opt_ButtonStyleMode)
ASSIGN_OPT(Opt_ButtonType)
ASSIGN_OPT(Opt_Array_Number)
ASSIGN_OPT(Opt_Array_TextCascadePickerRangeContent)
ASSIGN_OPT(Opt_Array_TextPickerRangeContent)
ASSIGN_OPT(Opt_Array_Array_String)
ASSIGN_OPT(Opt_ScrollAnimationOptions)
ASSIGN_OPT(Opt_RichEditorSymbolSpanStyle)
ASSIGN_OPT(Opt_RichEditorImageSpanStyle)
ASSIGN_OPT(Opt_RichEditorTextStyle)
ASSIGN_OPT(Opt_Callback_GestureEvent_Void)
ASSIGN_OPT(Opt_Callback_ClickEvent_Void)
ASSIGN_OPT(Opt_NavPathStack)
ASSIGN_OPT(Opt_NavPathInfo)
ASSIGN_OPT(Opt_ExpectedFrameRateRange)
ASSIGN_OPT(Opt_FinishCallbackType)
ASSIGN_OPT(Opt_PlayMode)
ASSIGN_OPT(Opt_Union_Curve_String_ICurve)
ASSIGN_OPT(Opt_TransitionEffect)
ASSIGN_OPT(Opt_ArrayBuffer)
ASSIGN_OPT(Opt_DialogButtonDirection)
ASSIGN_OPT(Opt_Array_AlertDialogButtonOptions)
ASSIGN_OPT(Opt_HoverModeAreaType)
ASSIGN_OPT(Opt_Union_ShadowOptions_ShadowStyle)
ASSIGN_OPT(Opt_Union_BorderStyle_EdgeStyles)
ASSIGN_OPT(Opt_Union_ResourceColor_EdgeColors_LocalizedEdgeColors)
ASSIGN_OPT(Opt_Union_Dimension_EdgeWidths_LocalizedEdgeWidths)
ASSIGN_OPT(Opt_Union_Dimension_BorderRadiuses_LocalizedBorderRadiuses)
ASSIGN_OPT(Opt_Callback_DismissDialogAction_Void)
ASSIGN_OPT(Opt_BlurStyle)
ASSIGN_OPT(Opt_Rectangle)
ASSIGN_OPT(Opt_Offset)
ASSIGN_OPT(Opt_DialogAlignment)
ASSIGN_OPT(Opt_AlertDialogButtonBaseOptions)
ASSIGN_OPT(Opt_Union_String_PixelMap_Resource)
ASSIGN_OPT(Opt_ImageAnalyzerController)
ASSIGN_OPT(Opt_Array_ImageAnalyzerType)
ASSIGN_OPT(Opt_Array_TouchObject)
ASSIGN_OPT(Opt_TouchType)
ASSIGN_OPT(Opt_SourceTool)
ASSIGN_OPT(Opt_SourceType)
ASSIGN_OPT(Opt_EventTarget)
ASSIGN_OPT(Opt_Map_String_String)
ASSIGN_OPT(Opt_NestedScrollMode)
ASSIGN_OPT(Opt_WebResourceRequest)
ASSIGN_OPT(Opt_WebController)
ASSIGN_OPT(Opt_PlaybackSpeed)
ASSIGN_OPT(Opt_Union_ResourceColor_Undefined)
ASSIGN_OPT(Opt_TabBarIconStyle)
ASSIGN_OPT(Opt_VerticalAlign)
ASSIGN_OPT(Opt_LayoutMode)
ASSIGN_OPT(Opt_Union_Padding_Dimension_LocalizedPadding)
ASSIGN_OPT(Opt_LabelStyle)
ASSIGN_OPT(Opt_Union_ResourceStr_TabBarSymbol)
ASSIGN_OPT(Opt_Union_Union_Padding_Dimension_LocalizedPadding)
ASSIGN_OPT(Opt_BoardStyle)
ASSIGN_OPT(Opt_SelectedMode)
ASSIGN_OPT(Opt_IndicatorStyle)
ASSIGN_OPT(Opt_Union_ResourceStr_Union_ResourceStr_ComponentContent)
ASSIGN_OPT(Opt_Callback_TabContentTransitionProxy_Void)
ASSIGN_OPT(Opt_RectAttribute)
ASSIGN_OPT(Opt_PathAttribute)
ASSIGN_OPT(Opt_EllipseAttribute)
ASSIGN_OPT(Opt_CircleAttribute)
ASSIGN_OPT(Opt_DividerOptions)
ASSIGN_OPT(Opt_CancelButtonStyle)
ASSIGN_OPT(Opt_IconOptions)
ASSIGN_OPT(Opt_Array_Length)
ASSIGN_OPT(Opt_RichEditorImageSpanStyleResult)
ASSIGN_OPT(Opt_RichEditorSpanPosition)
ASSIGN_OPT(Opt_RichEditorParagraphStyle)
ASSIGN_OPT(Opt_RichEditorTextStyleResult)
ASSIGN_OPT(Opt_Union_Number_String_Array_Any)
ASSIGN_OPT(Opt_ProgressStatus)
ASSIGN_OPT(Opt_Union_PX_VP_LPX_Resource)
ASSIGN_OPT(Opt_Union_TitleHeight_Length)
ASSIGN_OPT(Opt_NavDestinationMode)
ASSIGN_OPT(Opt_Callback_NavigationTransitionProxy_Void)
ASSIGN_OPT(Opt_Callback_Boolean_Void)
ASSIGN_OPT(Opt_SwipeActionItem)
ASSIGN_OPT(Opt_BreakpointsReference)
ASSIGN_OPT(Opt_GridRowColumnOption)
ASSIGN_OPT(Opt_GutterOption)
ASSIGN_OPT(Opt_GridColColumnOption)
ASSIGN_OPT(Opt_LinearGradient)
ASSIGN_OPT(Opt_Size)
ASSIGN_OPT(Opt_Union_String_Number_CanvasGradient_CanvasPattern)
ASSIGN_OPT(Opt_Array_CalendarDay)
ASSIGN_OPT(Opt_BadgePosition)
ASSIGN_OPT(Opt_SheetTitleOptions)
ASSIGN_OPT(Opt_Union_SheetSize_Length)
ASSIGN_OPT(Opt_MenuPreviewMode)
ASSIGN_OPT(Opt_Union_Boolean_Callback_DismissPopupAction_Void)
ASSIGN_OPT(Opt_ArrowPointPosition)
ASSIGN_OPT(Opt_Union_Boolean_Literal_ResourceColor_color)
ASSIGN_OPT(Opt_Callback_Literal_Boolean_isVisible_Void)
ASSIGN_OPT(Opt_Union_Color_String_Resource_Number)
ASSIGN_OPT(Opt_Placement)
ASSIGN_OPT(Opt_PopupMessageOptions)
ASSIGN_OPT(Opt_Literal_String_value_Callback_Void_action)
ASSIGN_OPT(Opt_Array_DragPreviewMode)
ASSIGN_OPT(Opt_DragPreviewMode)
ASSIGN_OPT(Opt_ClickEffectLevel)
ASSIGN_OPT(Opt_HorizontalAlign)
ASSIGN_OPT(Opt_Literal_Number_offset_span)
ASSIGN_OPT(Opt_RotateOptions)
ASSIGN_OPT(Opt_ScaleOptions)
ASSIGN_OPT(Opt_TranslateOptions)
ASSIGN_OPT(Opt_TransitionType)
ASSIGN_OPT(Opt_EdgeOutlineStyles)
ASSIGN_OPT(Opt_OutlineRadiuses)
ASSIGN_OPT(Opt_EdgeOutlineWidths)
ASSIGN_OPT(Opt_ThemeColorMode)
ASSIGN_OPT(Opt_Want)
ASSIGN_OPT(Opt_StyledStringValue)
ASSIGN_OPT(Opt_StyledStringKey)
ASSIGN_OPT(Opt_Union_Margin_Dimension)
ASSIGN_OPT(Opt_GetItemMainSizeByIndex)
ASSIGN_OPT(Opt_Array_Header)
ASSIGN_OPT(Opt_Callback_String_Void)
ASSIGN_OPT(Opt_WebCaptureMode)
ASSIGN_OPT(Opt_Callback_TimePickerResult_Void)
ASSIGN_OPT(Opt_PickerTextStyle)
ASSIGN_OPT(Opt_PickerDialogButtonStyle)
ASSIGN_OPT(Opt_TimePickerFormat)
ASSIGN_OPT(Opt_Date)
ASSIGN_OPT(Opt_Union_Number_Array_Number)
ASSIGN_OPT(Opt_Union_String_Array_String)
ASSIGN_OPT(Opt_Callback_TextPickerResult_Void)
ASSIGN_OPT(Opt_Type_TextPickerOptions_range)
ASSIGN_OPT(Opt_TextMenuItemId)
ASSIGN_OPT(Opt_Axis)
ASSIGN_OPT(Opt_Union_ScrollAnimationOptions_Boolean)
ASSIGN_OPT(Opt_StyledString)
ASSIGN_OPT(Opt_TextRange)
ASSIGN_OPT(Opt_OnDidChangeCallback)
ASSIGN_OPT(Opt_Callback_StyledStringChangeValue_Boolean)
ASSIGN_OPT(Opt_RichEditorUpdateSymbolSpanStyleOptions)
ASSIGN_OPT(Opt_RichEditorUpdateImageSpanStyleOptions)
ASSIGN_OPT(Opt_RichEditorUpdateTextSpanStyleOptions)
ASSIGN_OPT(Opt_RichEditorGesture)
ASSIGN_OPT(Opt_Array_FingerInfo)
ASSIGN_OPT(Opt_MenuPolicy)
ASSIGN_OPT(Opt_NavDestinationContext)
ASSIGN_OPT(Opt_InterceptionModeCallback)
ASSIGN_OPT(Opt_InterceptionShowCallback)
ASSIGN_OPT(Opt_LaunchMode)
ASSIGN_OPT(Opt_PanDirection)
ASSIGN_OPT(Opt_Callback_Date_Void)
ASSIGN_OPT(Opt_Callback_DatePickerResult_Void)
ASSIGN_OPT(Opt_LunarSwitchStyle)
ASSIGN_OPT(Opt_KeyboardAvoidMode)
ASSIGN_OPT(Opt_Union_ResourceColor_EdgeColors)
ASSIGN_OPT(Opt_Union_Dimension_EdgeWidths)
ASSIGN_OPT(Opt_AnimateParam)
ASSIGN_OPT(Opt_OnMoveHandler)
ASSIGN_OPT(Opt_Literal_TransitionEffect_appear_disappear)
ASSIGN_OPT(Opt_TransitionEdge)
ASSIGN_OPT(Opt_Uint8ClampedArray)
ASSIGN_OPT(Opt_ImageBitmap)
ASSIGN_OPT(Opt_AlertDialogParamWithOptions)
ASSIGN_OPT(Opt_AlertDialogParamWithButtons)
ASSIGN_OPT(Opt_AlertDialogParamWithConfirm)
ASSIGN_OPT(Opt_DismissReason)
ASSIGN_OPT(Opt_ActionSheetOffset)
ASSIGN_OPT(Opt_Array_SheetInfo)
ASSIGN_OPT(Opt_ActionSheetButtonOptions)
ASSIGN_OPT(Opt_OnLinearIndicatorChangeCallback)
ASSIGN_OPT(Opt_LinearIndicatorStyle)
ASSIGN_OPT(Opt_DpiFollowStrategy)
ASSIGN_OPT(Opt_WaterFlowLayoutMode)
ASSIGN_OPT(Opt_WaterFlowSections)
ASSIGN_OPT(Opt_Scroller)
ASSIGN_OPT(Opt_RRect)
ASSIGN_OPT(Opt_DividerStyle)
ASSIGN_OPT(Opt_Type_ButtonStyle_icons)
ASSIGN_OPT(Opt_ImageAIOptions)
ASSIGN_OPT(Opt_XComponentController)
ASSIGN_OPT(Opt_XComponentType)
ASSIGN_OPT(Opt_WebKeyboardController)
ASSIGN_OPT(Opt_Callback_Literal_String_plainText_Void)
ASSIGN_OPT(Opt_RenderProcessNotRespondingReason)
ASSIGN_OPT(Opt_EventResult)
ASSIGN_OPT(Opt_TouchEvent)
ASSIGN_OPT(Opt_NativeEmbedInfo)
ASSIGN_OPT(Opt_NativeEmbedStatus)
ASSIGN_OPT(Opt_NestedScrollOptionsExt)
ASSIGN_OPT(Opt_NestedScrollOptions)
ASSIGN_OPT(Opt_WebNavigationType)
ASSIGN_OPT(Opt_DataResubmissionHandler)
ASSIGN_OPT(Opt_ControllerHandler)
ASSIGN_OPT(Opt_ClientAuthenticationHandler)
ASSIGN_OPT(Opt_SslError)
ASSIGN_OPT(Opt_SslErrorHandler)
ASSIGN_OPT(Opt_WebContextMenuResult)
ASSIGN_OPT(Opt_WebContextMenuParam)
ASSIGN_OPT(Opt_ScreenCaptureHandler)
ASSIGN_OPT(Opt_PermissionRequest)
ASSIGN_OPT(Opt_HttpAuthHandler)
ASSIGN_OPT(Opt_FullScreenExitHandler)
ASSIGN_OPT(Opt_Function)
ASSIGN_OPT(Opt_FileSelectorParam)
ASSIGN_OPT(Opt_FileSelectorResult)
ASSIGN_OPT(Opt_RenderExitReason)
ASSIGN_OPT(Opt_Union_String_WebResourceRequest)
ASSIGN_OPT(Opt_WebResourceResponse)
ASSIGN_OPT(Opt_WebResourceError)
ASSIGN_OPT(Opt_ConsoleMessage)
ASSIGN_OPT(Opt_JsResult)
ASSIGN_OPT(Opt_JsGeolocation)
ASSIGN_OPT(Opt_Union_WebController_WebviewController)
ASSIGN_OPT(Opt_RenderMode)
ASSIGN_OPT(Opt_VideoController)
ASSIGN_OPT(Opt_Union_Number_String_PlaybackSpeed)
ASSIGN_OPT(Opt_ToggleType)
ASSIGN_OPT(Opt_TextTimerController)
ASSIGN_OPT(Opt_UnderlineColor)
ASSIGN_OPT(Opt_TextContentStyle)
ASSIGN_OPT(Opt_TextInputStyle)
ASSIGN_OPT(Opt_TextInputController)
ASSIGN_OPT(Opt_TextClockController)
ASSIGN_OPT(Opt_TextAreaController)
ASSIGN_OPT(Opt_TextController)
ASSIGN_OPT(Opt_BottomTabBarStyle)
ASSIGN_OPT(Opt_SubTabBarStyle)
ASSIGN_OPT(Opt_Literal_Union_String_Resource_icon_text)
ASSIGN_OPT(Opt_LayoutStyle)
ASSIGN_OPT(Opt_TabContentAnimatedTransition)
ASSIGN_OPT(Opt_EdgeEffect)
ASSIGN_OPT(Opt_AnimationMode)
ASSIGN_OPT(Opt_TabsController)
ASSIGN_OPT(Opt_BarPosition)
ASSIGN_OPT(Opt_SwiperAutoFill)
ASSIGN_OPT(Opt_ArrowStyle)
ASSIGN_OPT(Opt_Callback_SwiperContentTransitionProxy_Void)
ASSIGN_OPT(Opt_DigitIndicator)
ASSIGN_OPT(Opt_DotIndicator)
ASSIGN_OPT(Opt_Alignment)
ASSIGN_OPT(Opt_Type_SliderBlockStyle_shape)
ASSIGN_OPT(Opt_SliderBlockType)
ASSIGN_OPT(Opt_SliderStyle)
ASSIGN_OPT(Opt_Union_DividerOptions_Undefined)
ASSIGN_OPT(Opt_OptionWidthMode)
ASSIGN_OPT(Opt_TextDeleteDirection)
ASSIGN_OPT(Opt_CancelButtonSymbolOptions)
ASSIGN_OPT(Opt_CancelButtonOptions)
ASSIGN_OPT(Opt_SearchController)
ASSIGN_OPT(Opt_BarState)
ASSIGN_OPT(Opt_ScrollBarDirection)
ASSIGN_OPT(Opt_Union_Dimension_Array_Dimension)
ASSIGN_OPT(Opt_ScrollSnapAlign)
ASSIGN_OPT(Opt_SaveDescription)
ASSIGN_OPT(Opt_SaveIconStyle)
ASSIGN_OPT(Opt_Union_String_Number)
ASSIGN_OPT(Opt_MenuType)
ASSIGN_OPT(Opt_MenuOnAppearCallback)
ASSIGN_OPT(Opt_RichEditorResponseType)
ASSIGN_OPT(Opt_ResponseType)
ASSIGN_OPT(Opt_Array_RichEditorTextSpanResult)
ASSIGN_OPT(Opt_Array_RichEditorImageSpanResult)
ASSIGN_OPT(Opt_Array_TextDataDetectorType)
ASSIGN_OPT(Opt_RichEditorImageSpanResult)
ASSIGN_OPT(Opt_RichEditorTextSpanResult)
ASSIGN_OPT(Opt_Array_Union_RichEditorTextSpanResult_RichEditorImageSpanResult)
ASSIGN_OPT(Opt_RichEditorDeleteDirection)
ASSIGN_OPT(Opt_RichEditorStyledStringController)
ASSIGN_OPT(Opt_RichEditorController)
ASSIGN_OPT(Opt_LocalizedBarrierDirection)
ASSIGN_OPT(Opt_BarrierDirection)
ASSIGN_OPT(Opt_GuideLinePosition)
ASSIGN_OPT(Opt_Type_RectInterface_value_u1)
ASSIGN_OPT(Opt_Type_RectInterface_value_u0)
ASSIGN_OPT(Opt_RadioIndicatorType)
ASSIGN_OPT(Opt_ProgressStyleOptions)
ASSIGN_OPT(Opt_CapsuleStyleOptions)
ASSIGN_OPT(Opt_RingStyleOptions)
ASSIGN_OPT(Opt_LinearStyleOptions)
ASSIGN_OPT(Opt_ProgressType)
ASSIGN_OPT(Opt_ProgressStyle)
ASSIGN_OPT(Opt_PluginComponentTemplate)
ASSIGN_OPT(Opt_CircleStyleOptions)
ASSIGN_OPT(Opt_PasteDescription)
ASSIGN_OPT(Opt_PasteIconStyle)
ASSIGN_OPT(Opt_PanelHeight)
ASSIGN_OPT(Opt_NavigationType)
ASSIGN_OPT(Opt_ToolbarItemStatus)
ASSIGN_OPT(Opt_Array_ToolbarItem)
ASSIGN_OPT(Opt_NavigationCustomTitle)
ASSIGN_OPT(Opt_NavigationCommonTitle)
ASSIGN_OPT(Opt_NavContentInfo)
ASSIGN_OPT(Opt_NavigationAnimatedTransition)
ASSIGN_OPT(Opt_Array_NavigationMenuItem)
ASSIGN_OPT(Opt_TextModifier)
ASSIGN_OPT(Opt_BarStyle)
ASSIGN_OPT(Opt_NavDestinationCustomTitle)
ASSIGN_OPT(Opt_NavDestinationCommonTitle)
ASSIGN_OPT(Opt_Union_ResourceStr_CustomBuilder)
ASSIGN_OPT(Opt_MenuItemOptions)
ASSIGN_OPT(Opt_DividerStyleOptions)
ASSIGN_OPT(Opt_ASTCResource)
ASSIGN_OPT(Opt_LocationDescription)
ASSIGN_OPT(Opt_LocationIconStyle)
ASSIGN_OPT(Opt_LocalizedEdges)
ASSIGN_OPT(Opt_Edges)
ASSIGN_OPT(Opt_ListDividerOptions)
ASSIGN_OPT(Opt_ListItemGroupStyle)
ASSIGN_OPT(Opt_Callback_Number_Void)
ASSIGN_OPT(Opt_SwipeEdgeEffect)
ASSIGN_OPT(Opt_Union_CustomBuilder_SwipeActionItem)
ASSIGN_OPT(Opt_EditMode)
ASSIGN_OPT(Opt_ListItemStyle)
ASSIGN_OPT(Opt_LengthConstrain)
ASSIGN_OPT(Opt_ListItemGroupArea)
ASSIGN_OPT(Opt_ChainEdgeEffect)
ASSIGN_OPT(Opt_ColorFilter)
ASSIGN_OPT(Opt_Union_String_Resource_PixelMap)
ASSIGN_OPT(Opt_ImageContent)
ASSIGN_OPT(Opt_GridRowDirection)
ASSIGN_OPT(Opt_BreakPoints)
ASSIGN_OPT(Opt_Union_Number_GridRowColumnOption)
ASSIGN_OPT(Opt_Union_Length_GutterOption)
ASSIGN_OPT(Opt_SizeType)
ASSIGN_OPT(Opt_Union_Number_GridColColumnOption)
ASSIGN_OPT(Opt_GridItemStyle)
ASSIGN_OPT(Opt_GridItemAlignment)
ASSIGN_OPT(Opt_Callback_Number_Tuple_Number_Number_Number_Number)
ASSIGN_OPT(Opt_Callback_Number_Tuple_Number_Number)
ASSIGN_OPT(Opt_Union_ResourceColor_LinearGradient)
ASSIGN_OPT(Opt_Array_Tuple_Union_ResourceColor_LinearGradient_Number)
ASSIGN_OPT(Opt_FormShape)
ASSIGN_OPT(Opt_FormRenderingMode)
ASSIGN_OPT(Opt_FormDimension)
ASSIGN_OPT(Opt_AppRotation)
ASSIGN_OPT(Opt_FoldStatus)
ASSIGN_OPT(Opt_FlexSpaceOptions)
ASSIGN_OPT(Opt_FlexAlign)
ASSIGN_OPT(Opt_ItemAlign)
ASSIGN_OPT(Opt_FlexWrap)
ASSIGN_OPT(Opt_FlexDirection)
ASSIGN_OPT(Opt_Array_Union_ResourceColor_LinearGradient)
ASSIGN_OPT(Opt_DataPanelType)
ASSIGN_OPT(Opt_ModelType)
ASSIGN_OPT(Opt_Union_ResourceStr_Scene)
ASSIGN_OPT(Opt_ColumnSplitDividerStyle)
ASSIGN_OPT(Opt_IlluminatedType)
ASSIGN_OPT(Opt_LightSource)
ASSIGN_OPT(Opt_SelectStatus)
ASSIGN_OPT(Opt_DrawingRenderingContext)
ASSIGN_OPT(Opt_CanvasRenderingContext2D)
ASSIGN_OPT(Opt_CalendarController)
ASSIGN_OPT(Opt_MonthData)
ASSIGN_OPT(Opt_Literal_Number_day_month_year)
ASSIGN_OPT(Opt_ControlSize)
ASSIGN_OPT(Opt_BadgeStyle)
ASSIGN_OPT(Opt_Union_BadgePosition_Position)
ASSIGN_OPT(Opt_ScrollMotion)
ASSIGN_OPT(Opt_FrictionMotion)
ASSIGN_OPT(Opt_SpringMotion)
ASSIGN_OPT(Opt_FunctionKey)
ASSIGN_OPT(Opt_SheetKeyboardAvoidMode)
ASSIGN_OPT(Opt_Callback_SheetType_Void)
ASSIGN_OPT(Opt_ScrollSizeMode)
ASSIGN_OPT(Opt_SheetMode)
ASSIGN_OPT(Opt_Callback_SpringBackAction_Void)
ASSIGN_OPT(Opt_Callback_DismissSheetAction_Void)
ASSIGN_OPT(Opt_Callback_SheetDismiss_Void)
ASSIGN_OPT(Opt_Union_SheetTitleOptions_CustomBuilder)
ASSIGN_OPT(Opt_SheetType)
ASSIGN_OPT(Opt_Union_Boolean_Resource)
ASSIGN_OPT(Opt_Type_SheetOptions_detents)
ASSIGN_OPT(Opt_Callback_DismissContentCoverAction_Void)
ASSIGN_OPT(Opt_ModalTransition)
ASSIGN_OPT(Opt_ContextMenuAnimationOptions)
ASSIGN_OPT(Opt_Union_Length_BorderRadiuses_LocalizedBorderRadiuses)
ASSIGN_OPT(Opt_Union_MenuPreviewMode_CustomBuilder)
ASSIGN_OPT(Opt_Array_MenuElement)
ASSIGN_OPT(Opt_CustomPopupOptions)
ASSIGN_OPT(Opt_PopupOptions)
ASSIGN_OPT(Opt_TransitionHierarchyStrategy)
ASSIGN_OPT(Opt_BlendMode)
ASSIGN_OPT(Opt_OverlayOffset)
ASSIGN_OPT(Opt_Union_Boolean_Number)
ASSIGN_OPT(Opt_Union_DragPreviewMode_Array_DragPreviewMode)
ASSIGN_OPT(Opt_SharedTransitionEffectType)
ASSIGN_OPT(Opt_MotionPathOptions)
ASSIGN_OPT(Opt_Array_FractionStop)
ASSIGN_OPT(Opt_TransitionFinishCallback)
ASSIGN_OPT(Opt_BlurOptions)
ASSIGN_OPT(Opt_AdaptiveColor)
ASSIGN_OPT(Opt_BlurType)
ASSIGN_OPT(Opt_BlurStyleActivePolicy)
ASSIGN_OPT(Opt_GestureControl_GestureType)
ASSIGN_OPT(Opt_ProgressMask)
ASSIGN_OPT(Opt_Tuple_Length_Length)
ASSIGN_OPT(Opt_DragItemInfo)
ASSIGN_OPT(Opt_DragBehavior)
ASSIGN_OPT(Opt_ClickEffect)
ASSIGN_OPT(Opt_Bias)
ASSIGN_OPT(Opt_LocalizedVerticalAlignParam)
ASSIGN_OPT(Opt_LocalizedHorizontalAlignParam)
ASSIGN_OPT(Opt_Literal_String_anchor_VerticalAlign_align)
ASSIGN_OPT(Opt_Literal_String_anchor_HorizontalAlign_align)
ASSIGN_OPT(Opt_Union_Number_Literal_Number_offset_span)
ASSIGN_OPT(Opt_LocalizedPosition)
ASSIGN_OPT(Opt_InvertOptions)
ASSIGN_OPT(Opt_MotionBlurAnchor)
ASSIGN_OPT(Opt_TransitionOptions)
ASSIGN_OPT(Opt_KeySource)
ASSIGN_OPT(Opt_KeyType)
ASSIGN_OPT(Opt_MouseAction)
ASSIGN_OPT(Opt_MouseButton)
ASSIGN_OPT(Opt_AccessibilityHoverType)
ASSIGN_OPT(Opt_Union_EdgeOutlineStyles_OutlineStyle)
ASSIGN_OPT(Opt_Union_OutlineRadiuses_Dimension)
ASSIGN_OPT(Opt_Union_EdgeColors_ResourceColor_LocalizedEdgeColors)
ASSIGN_OPT(Opt_Union_EdgeOutlineWidths_Dimension)
ASSIGN_OPT(Opt_Union_Length_EdgeWidths_LocalizedEdgeWidths)
ASSIGN_OPT(Opt_Union_String_Resource_LinearGradient)
ASSIGN_OPT(Opt_RepeatMode)
ASSIGN_OPT(Opt_Union_EdgeWidths_LengthMetrics_LocalizedEdgeWidths)
ASSIGN_OPT(Opt_Union_EdgeStyles_BorderStyle)
ASSIGN_OPT(Opt_Union_BorderRadiuses_Length_LocalizedBorderRadiuses)
ASSIGN_OPT(Opt_Union_EdgeWidths_Length_LocalizedEdgeWidths)
ASSIGN_OPT(Opt_ImageSize)
ASSIGN_OPT(Opt_PixelRoundCalcPolicy)
ASSIGN_OPT(Opt_TouchTestStrategy)
ASSIGN_OPT(Opt_RectResult)
ASSIGN_OPT(Opt_Array_Rectangle)
ASSIGN_OPT(Opt_DrawModifier)
ASSIGN_OPT(Opt_WithThemeOptions)
ASSIGN_OPT(Opt_WithThemeInterface)
ASSIGN_OPT(Opt_SliderTriggerChangeCallback)
ASSIGN_OPT(Opt_ScrollOnWillScrollCallback)
ASSIGN_OPT(Opt_ScrollOnScrollCallback)
ASSIGN_OPT(Opt_RouteType)
ASSIGN_OPT(Opt_PageTransitionCallback)
ASSIGN_OPT(Opt_ScrollSource)
ASSIGN_OPT(Opt_OnWillScrollCallback)
ASSIGN_OPT(Opt_OnScrollCallback)
ASSIGN_OPT(Opt_IsolatedOptions)
ASSIGN_OPT(Opt_IsolatedComponentInterface)
ASSIGN_OPT(Opt_HoverCallback)
ASSIGN_OPT(Opt_Callback_Any_Number_Void)
ASSIGN_OPT(Opt_Callback_Any_Number_String)
ASSIGN_OPT(Opt_ButtonTriggerClickCallback)
ASSIGN_OPT(Opt_WithThemeAttribute)
ASSIGN_OPT(Opt_Callback_WithThemeAttribute_void)
ASSIGN_OPT(Opt_Callback_WebKeyboardOptions_void)
ASSIGN_OPT(Opt_Callback_Union_TabContentAnimatedTransition_Undefined_void)
ASSIGN_OPT(Opt_Callback_Union_NavigationAnimatedTransition_Undefined_void)
ASSIGN_OPT(Opt_Callback_GestureRecognizer_void)
ASSIGN_OPT(Opt_OffsetResult)
ASSIGN_OPT(Opt_Callback_OffsetResult_void)
ASSIGN_OPT(Opt_ScrollResult)
ASSIGN_OPT(Opt_Callback_ScrollResult_void)
ASSIGN_OPT(Opt_Callback_OnScrollFrameBeginHandlerResult_void)
ASSIGN_OPT(Opt_Callback_Array_String_void)
ASSIGN_OPT(Opt_IsolatedComponentAttribute)
ASSIGN_OPT(Opt_Callback_IsolatedComponentAttribute_void)
ASSIGN_OPT(Opt_Callback_Number_void)
ASSIGN_OPT(Opt_Callback_HitTestMode_void)
ASSIGN_OPT(Opt_Callback_WebResourceResponse_void)
ASSIGN_OPT(Opt_Callback_Tuple_Number_Number_Number_Number_void)
ASSIGN_OPT(Opt_Callback_Tuple_Number_Number_void)
ASSIGN_OPT(Opt_Callback_Literal_Number_offsetRemain_void)
ASSIGN_OPT(Opt_Callback_ComputedBarAttribute_void)
ASSIGN_OPT(Opt_Callback_Callback_Any_void)
ASSIGN_OPT(Opt_Callback_GestureJudgeResult_void)
ASSIGN_OPT(Opt_Callback_Union_CustomBuilder_DragItemInfo_void)
ASSIGN_OPT(Opt_Callback_Boolean_void)
ASSIGN_OPT(Opt_Callback_TouchResult_void)
ASSIGN_OPT(Opt_Callback_String_void)
ASSIGN_OPT(Opt_Callback_CustomObject_void)
ASSIGN_OPT(Opt_Callback_void)
ASSIGN_OPT(Opt_LinearIndicatorStartOptions)
ASSIGN_OPT(Opt_CustomSpanDrawInfo)
ASSIGN_OPT(Opt_CustomSpanMeasureInfo)
ASSIGN_OPT(Opt_SpanStyle)
ASSIGN_OPT(Opt_StyleOptions)
ASSIGN_OPT(Opt_Array_StyleOptions)
ASSIGN_OPT(Opt_Union_String_ImageAttachment_CustomSpan)
ASSIGN_OPT(Opt_SectionOptions)
ASSIGN_OPT(Opt_Array_SectionOptions)
ASSIGN_OPT(Opt_SurfaceRotationOptions)
ASSIGN_OPT(Opt_SurfaceRect)
ASSIGN_OPT(Opt_Literal_Number_surfaceHeight_surfaceWidth)
ASSIGN_OPT(Opt_Literal_Object_object_String_name_Array_String_methodList)
ASSIGN_OPT(Opt_Literal_Union_String_Resource_url_Array_Header_headers)
ASSIGN_OPT(Opt_Literal_String_baseUrl_data_encoding_historyUrl_mimeType)
ASSIGN_OPT(Opt_Literal_String_script_Callback_String_Void_callback)
ASSIGN_OPT(Opt_Header)
ASSIGN_OPT(Opt_Union_String_Number_Resource_ArrayBuffer)
ASSIGN_OPT(Opt_MessageLevel)
ASSIGN_OPT(Opt_ScreenCaptureConfig)
ASSIGN_OPT(Opt_SeekMode)
ASSIGN_OPT(Opt_TimePickerDialogOptions)
ASSIGN_OPT(Opt_TextPickerResult)
ASSIGN_OPT(Opt_TextPickerDialogOptions)
ASSIGN_OPT(Opt_TextMenuItem)
ASSIGN_OPT(Opt_Array_TextMenuItem)
ASSIGN_OPT(Opt_Union_Array_Number_Undefined)
ASSIGN_OPT(Opt_ScrollToIndexOptions)
ASSIGN_OPT(Opt_Literal_Boolean_next_Axis_direction)
ASSIGN_OPT(Opt_Literal_Boolean_next)
ASSIGN_OPT(Opt_ScrollEdgeOptions)
ASSIGN_OPT(Opt_ScrollOptions)
ASSIGN_OPT(Opt_StyledStringChangeValue)
ASSIGN_OPT(Opt_StyledStringChangedListener)
ASSIGN_OPT(Opt_RichEditorParagraphStyleOptions)
ASSIGN_OPT(Opt_Type_RichEditorController_updateSpanStyle_value)
ASSIGN_OPT(Opt_RichEditorSymbolSpanOptions)
ASSIGN_OPT(Opt_RichEditorBuilderSpanOptions)
ASSIGN_OPT(Opt_RichEditorImageSpanOptions)
ASSIGN_OPT(Opt_Union_PixelMap_ResourceStr)
ASSIGN_OPT(Opt_GestureEvent)
ASSIGN_OPT(Opt_RichEditorTextSpanOptions)
ASSIGN_OPT(Opt_SelectionOptions)
ASSIGN_OPT(Opt_PatternLockChallengeResult)
ASSIGN_OPT(Opt_Union_NavDestinationContext_NavBar)
ASSIGN_OPT(Opt_NavigationInterception)
ASSIGN_OPT(Opt_NavigationOptions)
ASSIGN_OPT(Opt_CloseSwipeActionOptions)
ASSIGN_OPT(Opt_ScrollAlign)
ASSIGN_OPT(Opt_Literal_Number_distance_fingers_PanDirection_direction)
ASSIGN_OPT(Opt_DatePickerDialogOptions)
ASSIGN_OPT(Opt_CustomDialogControllerOptions)
ASSIGN_OPT(Opt_IPropertySubscriber)
ASSIGN_OPT(Opt_UIGestureEvent)
ASSIGN_OPT(Opt_Union_OnMoveHandler_Undefined)
ASSIGN_OPT(Opt_TransitionEffects)
ASSIGN_OPT(Opt_RenderingContextSettings)
ASSIGN_OPT(Opt_ImageData)
ASSIGN_OPT(Opt_Union_String_Undefined)
ASSIGN_OPT(Opt_Union_ImageBitmap_PixelMap)
ASSIGN_OPT(Opt_Matrix2D)
ASSIGN_OPT(Opt_Path2D)
ASSIGN_OPT(Opt_CalendarDialogOptions)
ASSIGN_OPT(Opt_AlertDialogButtonOptions)
ASSIGN_OPT(Opt_Type_AlertDialog_show_value)
ASSIGN_OPT(Opt_DismissDialogAction)
ASSIGN_OPT(Opt_SheetInfo)
ASSIGN_OPT(Opt_ActionSheetOptions)
ASSIGN_OPT(Opt_Union_OnLinearIndicatorChangeCallback_Undefined)
ASSIGN_OPT(Opt_Union_Boolean_Undefined)
ASSIGN_OPT(Opt_Union_LinearIndicatorStyle_Undefined)
ASSIGN_OPT(Opt_LinearIndicatorController)
ASSIGN_OPT(Opt_Literal_Number_code_Want_want)
ASSIGN_OPT(Opt_Callback_Literal_Number_code_Want_want_Void)
ASSIGN_OPT(Opt_Literal_Empty)
ASSIGN_OPT(Opt_Callback_Literal_Empty_Void)
ASSIGN_OPT(Opt_UIExtensionProxy)
ASSIGN_OPT(Opt_Callback_UIExtensionProxy_Void)
ASSIGN_OPT(Opt_UIExtensionOptions)
ASSIGN_OPT(Opt_WaterFlowOptions)
ASSIGN_OPT(Opt_WindowAnimationTarget)
ASSIGN_OPT(Opt_Union_DividerStyle_Undefined)
ASSIGN_OPT(Opt_SideBarPosition)
ASSIGN_OPT(Opt_ButtonStyle)
ASSIGN_OPT(Opt_SideBarContainerType)
ASSIGN_OPT(Opt_Callback_Object_Void)
ASSIGN_OPT(Opt_XComponentOptions)
ASSIGN_OPT(Opt_Type_XComponentInterface_value)
ASSIGN_OPT(Opt_WebKeyboardAvoidMode)
ASSIGN_OPT(Opt_AdsBlockedDetails)
ASSIGN_OPT(Opt_OnAdsBlockedCallback)
ASSIGN_OPT(Opt_WebKeyboardOptions)
ASSIGN_OPT(Opt_WebKeyboardCallbackInfo)
ASSIGN_OPT(Opt_WebKeyboardCallback)
ASSIGN_OPT(Opt_ViewportFit)
ASSIGN_OPT(Opt_OnViewportFitChangedCallback)
ASSIGN_OPT(Opt_Literal_String_plainText)
ASSIGN_OPT(Opt_ExpandedMenuItemOptions)
ASSIGN_OPT(Opt_Array_ExpandedMenuItemOptions)
ASSIGN_OPT(Opt_OnRenderProcessRespondingCallback)
ASSIGN_OPT(Opt_RenderProcessNotRespondingData)
ASSIGN_OPT(Opt_OnRenderProcessNotRespondingCallback)
ASSIGN_OPT(Opt_NativeMediaPlayerConfig)
ASSIGN_OPT(Opt_OnOverrideUrlLoadingCallback)
ASSIGN_OPT(Opt_NativeEmbedTouchInfo)
ASSIGN_OPT(Opt_Callback_NativeEmbedTouchInfo_Void)
ASSIGN_OPT(Opt_NativeEmbedVisibilityInfo)
ASSIGN_OPT(Opt_OnNativeEmbedVisibilityChangeCallback)
ASSIGN_OPT(Opt_NativeEmbedDataInfo)
ASSIGN_OPT(Opt_Callback_NativeEmbedDataInfo_Void)
ASSIGN_OPT(Opt_Union_NestedScrollOptions_NestedScrollOptionsExt)
ASSIGN_OPT(Opt_WebLayoutMode)
ASSIGN_OPT(Opt_Array_ScriptItem)
ASSIGN_OPT(Opt_ScriptItem)
ASSIGN_OPT(Opt_IntelligentTrackingPreventionDetails)
ASSIGN_OPT(Opt_OnIntelligentTrackingPreventionCallback)
ASSIGN_OPT(Opt_LoadCommittedDetails)
ASSIGN_OPT(Opt_OnNavigationEntryCommittedCallback)
ASSIGN_OPT(Opt_ThreatType)
ASSIGN_OPT(Opt_OnSafeBrowsingCheckResultCallback)
ASSIGN_OPT(Opt_OnOverScrollEvent)
ASSIGN_OPT(Opt_Callback_OnOverScrollEvent_Void)
ASSIGN_OPT(Opt_OnLoadInterceptEvent)
ASSIGN_OPT(Opt_Callback_OnLoadInterceptEvent_Boolean)
ASSIGN_OPT(Opt_LargestContentfulPaint)
ASSIGN_OPT(Opt_OnLargestContentfulPaintCallback)
ASSIGN_OPT(Opt_FirstMeaningfulPaint)
ASSIGN_OPT(Opt_OnFirstMeaningfulPaintCallback)
ASSIGN_OPT(Opt_OnFirstContentfulPaintEvent)
ASSIGN_OPT(Opt_Callback_OnFirstContentfulPaintEvent_Void)
ASSIGN_OPT(Opt_OnAudioStateChangedEvent)
ASSIGN_OPT(Opt_Callback_OnAudioStateChangedEvent_Void)
ASSIGN_OPT(Opt_OnDataResubmittedEvent)
ASSIGN_OPT(Opt_Callback_OnDataResubmittedEvent_Void)
ASSIGN_OPT(Opt_OnPageVisibleEvent)
ASSIGN_OPT(Opt_Callback_OnPageVisibleEvent_Void)
ASSIGN_OPT(Opt_OnFaviconReceivedEvent)
ASSIGN_OPT(Opt_Callback_OnFaviconReceivedEvent_Void)
ASSIGN_OPT(Opt_OnTouchIconUrlReceivedEvent)
ASSIGN_OPT(Opt_Callback_OnTouchIconUrlReceivedEvent_Void)
ASSIGN_OPT(Opt_OnWindowNewEvent)
ASSIGN_OPT(Opt_Callback_OnWindowNewEvent_Void)
ASSIGN_OPT(Opt_OnClientAuthenticationEvent)
ASSIGN_OPT(Opt_Callback_OnClientAuthenticationEvent_Void)
ASSIGN_OPT(Opt_SslErrorEvent)
ASSIGN_OPT(Opt_OnSslErrorEventCallback)
ASSIGN_OPT(Opt_OnSslErrorEventReceiveEvent)
ASSIGN_OPT(Opt_Callback_OnSslErrorEventReceiveEvent_Void)
ASSIGN_OPT(Opt_OnScrollEvent)
ASSIGN_OPT(Opt_Callback_OnScrollEvent_Void)
ASSIGN_OPT(Opt_OnSearchResultReceiveEvent)
ASSIGN_OPT(Opt_Callback_OnSearchResultReceiveEvent_Void)
ASSIGN_OPT(Opt_OnContextMenuHideCallback)
ASSIGN_OPT(Opt_OnContextMenuShowEvent)
ASSIGN_OPT(Opt_Callback_OnContextMenuShowEvent_Boolean)
ASSIGN_OPT(Opt_OnScreenCaptureRequestEvent)
ASSIGN_OPT(Opt_Callback_OnScreenCaptureRequestEvent_Void)
ASSIGN_OPT(Opt_OnPermissionRequestEvent)
ASSIGN_OPT(Opt_Callback_OnPermissionRequestEvent_Void)
ASSIGN_OPT(Opt_OnInterceptRequestEvent)
ASSIGN_OPT(Opt_Callback_OnInterceptRequestEvent_WebResourceResponse)
ASSIGN_OPT(Opt_OnHttpAuthRequestEvent)
ASSIGN_OPT(Opt_Callback_OnHttpAuthRequestEvent_Boolean)
ASSIGN_OPT(Opt_OnScaleChangeEvent)
ASSIGN_OPT(Opt_Callback_OnScaleChangeEvent_Void)
ASSIGN_OPT(Opt_FullScreenEnterEvent)
ASSIGN_OPT(Opt_OnFullScreenEnterCallback)
ASSIGN_OPT(Opt_OnResourceLoadEvent)
ASSIGN_OPT(Opt_Callback_OnResourceLoadEvent_Void)
ASSIGN_OPT(Opt_Literal_Function_callback_Object_fileSelector)
ASSIGN_OPT(Opt_Callback_Literal_Function_callback_Object_fileSelector_Void)
ASSIGN_OPT(Opt_Literal_Object_detail)
ASSIGN_OPT(Opt_Callback_Literal_Object_detail_Boolean)
ASSIGN_OPT(Opt_OnShowFileSelectorEvent)
ASSIGN_OPT(Opt_Callback_OnShowFileSelectorEvent_Boolean)
ASSIGN_OPT(Opt_OnRenderExitedEvent)
ASSIGN_OPT(Opt_Callback_OnRenderExitedEvent_Void)
ASSIGN_OPT(Opt_Literal_Function_handler_Object_error)
ASSIGN_OPT(Opt_Callback_Literal_Function_handler_Object_error_Void)
ASSIGN_OPT(Opt_Literal_Union_String_WebResourceRequest_data)
ASSIGN_OPT(Opt_Type_WebAttribute_onUrlLoadIntercept_callback)
ASSIGN_OPT(Opt_OnRefreshAccessedHistoryEvent)
ASSIGN_OPT(Opt_Callback_OnRefreshAccessedHistoryEvent_Void)
ASSIGN_OPT(Opt_OnDownloadStartEvent)
ASSIGN_OPT(Opt_Callback_OnDownloadStartEvent_Void)
ASSIGN_OPT(Opt_OnHttpErrorReceiveEvent)
ASSIGN_OPT(Opt_Callback_OnHttpErrorReceiveEvent_Void)
ASSIGN_OPT(Opt_OnErrorReceiveEvent)
ASSIGN_OPT(Opt_Callback_OnErrorReceiveEvent_Void)
ASSIGN_OPT(Opt_OnConsoleEvent)
ASSIGN_OPT(Opt_Callback_OnConsoleEvent_Boolean)
ASSIGN_OPT(Opt_OnPromptEvent)
ASSIGN_OPT(Opt_Callback_OnPromptEvent_Boolean)
ASSIGN_OPT(Opt_OnConfirmEvent)
ASSIGN_OPT(Opt_Callback_OnConfirmEvent_Boolean)
ASSIGN_OPT(Opt_OnBeforeUnloadEvent)
ASSIGN_OPT(Opt_Callback_OnBeforeUnloadEvent_Boolean)
ASSIGN_OPT(Opt_OnAlertEvent)
ASSIGN_OPT(Opt_Callback_OnAlertEvent_Boolean)
ASSIGN_OPT(Opt_OnGeolocationShowEvent)
ASSIGN_OPT(Opt_Callback_OnGeolocationShowEvent_Void)
ASSIGN_OPT(Opt_OnTitleReceiveEvent)
ASSIGN_OPT(Opt_Callback_OnTitleReceiveEvent_Void)
ASSIGN_OPT(Opt_OnProgressChangeEvent)
ASSIGN_OPT(Opt_Callback_OnProgressChangeEvent_Void)
ASSIGN_OPT(Opt_OnPageBeginEvent)
ASSIGN_OPT(Opt_Callback_OnPageBeginEvent_Void)
ASSIGN_OPT(Opt_OnPageEndEvent)
ASSIGN_OPT(Opt_Callback_OnPageEndEvent_Void)
ASSIGN_OPT(Opt_OverScrollMode)
ASSIGN_OPT(Opt_WebMediaOptions)
ASSIGN_OPT(Opt_WebDarkMode)
ASSIGN_OPT(Opt_CacheMode)
ASSIGN_OPT(Opt_JavaScriptProxy)
ASSIGN_OPT(Opt_MixedMode)
ASSIGN_OPT(Opt_WebOptions)
ASSIGN_OPT(Opt_Literal_Number_time)
ASSIGN_OPT(Opt_Callback_Literal_Number_time_Void)
ASSIGN_OPT(Opt_Literal_Number_duration)
ASSIGN_OPT(Opt_Callback_Literal_Number_duration_Void)
ASSIGN_OPT(Opt_Literal_Boolean_fullscreen)
ASSIGN_OPT(Opt_Callback_Literal_Boolean_fullscreen_Void)
ASSIGN_OPT(Opt_VideoOptions)
ASSIGN_OPT(Opt_SwitchStyle)
ASSIGN_OPT(Opt_ToggleOptions)
ASSIGN_OPT(Opt_TimePickerResult)
ASSIGN_OPT(Opt_TimePickerOptions)
ASSIGN_OPT(Opt_TextTimerOptions)
ASSIGN_OPT(Opt_Type_TextPickerAttribute_onChange_callback)
ASSIGN_OPT(Opt_Callback_String_Number_Void)
ASSIGN_OPT(Opt_TextCascadePickerRangeContent)
ASSIGN_OPT(Opt_TextPickerRangeContent)
ASSIGN_OPT(Opt_TextPickerOptions)
ASSIGN_OPT(Opt_Literal_CancelButtonStyle_style_IconOptions_icon)
ASSIGN_OPT(Opt_Union_ResourceColor_UnderlineColor_Undefined)
ASSIGN_OPT(Opt_Union_ResourceStr_Undefined)
ASSIGN_OPT(Opt_PasswordIcon)
ASSIGN_OPT(Opt_Union_TextInputStyle_TextContentStyle)
ASSIGN_OPT(Opt_Callback_EnterKeyType_SubmitEvent_Void)
ASSIGN_OPT(Opt_InputType)
ASSIGN_OPT(Opt_TextInputOptions)
ASSIGN_OPT(Opt_Union_DateTimeOptions_Undefined)
ASSIGN_OPT(Opt_TextClockOptions)
ASSIGN_OPT(Opt_InputCounterOptions)
ASSIGN_OPT(Opt_ContentType)
ASSIGN_OPT(Opt_TextAreaType)
ASSIGN_OPT(Opt_Callback_EnterKeyType_Void)
ASSIGN_OPT(Opt_TextAreaOptions)
ASSIGN_OPT(Opt_TextResponseType)
ASSIGN_OPT(Opt_TextSpanType)
ASSIGN_OPT(Opt_FontSettingOptions)
ASSIGN_OPT(Opt_TextSelectableMode)
ASSIGN_OPT(Opt_EllipsisMode)
ASSIGN_OPT(Opt_TextOverflowOptions)
ASSIGN_OPT(Opt_TextOptions)
ASSIGN_OPT(Opt_Union_SubTabBarStyle_BottomTabBarStyle)
ASSIGN_OPT(Opt_Type_TabContentAttribute_tabBar_value)
ASSIGN_OPT(Opt_ScrollableBarModeOptions)
ASSIGN_OPT(Opt_TabContentTransitionProxy)
ASSIGN_OPT(Opt_Union_TabContentAnimatedTransition_Undefined)
ASSIGN_OPT(Opt_Type_TabsAttribute_customContentTransition_delegate)
ASSIGN_OPT(Opt_BarGridColumnOptions)
ASSIGN_OPT(Opt_Callback_Number_TabsAnimationEvent_Void)
ASSIGN_OPT(Opt_TabsAnimationEvent)
ASSIGN_OPT(Opt_Callback_Number_Number_TabsAnimationEvent_Void)
ASSIGN_OPT(Opt_Union_EdgeEffect_Undefined)
ASSIGN_OPT(Opt_Union_AnimationMode_Undefined)
ASSIGN_OPT(Opt_BarMode)
ASSIGN_OPT(Opt_Type_TabsInterface_value)
ASSIGN_OPT(Opt_SymbolEffect)
ASSIGN_OPT(Opt_Union_Number_String_SwiperAutoFill)
ASSIGN_OPT(Opt_Union_ArrowStyle_Boolean)
ASSIGN_OPT(Opt_ContentDidScrollCallback)
ASSIGN_OPT(Opt_SwiperContentTransitionProxy)
ASSIGN_OPT(Opt_SwiperContentAnimatedTransition)
ASSIGN_OPT(Opt_SwiperNestedScrollMode)
ASSIGN_OPT(Opt_Callback_Number_SwiperAnimationEvent_Void)
ASSIGN_OPT(Opt_SwiperAnimationEvent)
ASSIGN_OPT(Opt_Callback_Number_Number_SwiperAnimationEvent_Void)
ASSIGN_OPT(Opt_SwiperDisplayMode)
ASSIGN_OPT(Opt_Union_DotIndicator_DigitIndicator_Boolean)
ASSIGN_OPT(Opt_SwiperController)
ASSIGN_OPT(Opt_ItemState)
ASSIGN_OPT(Opt_Literal_Number_index)
ASSIGN_OPT(Opt_StackOptions)
ASSIGN_OPT(Opt_TextCase)
ASSIGN_OPT(Opt_SlideRange)
ASSIGN_OPT(Opt_SliderInteraction)
ASSIGN_OPT(Opt_SliderBlockStyle)
ASSIGN_OPT(Opt_SliderChangeMode)
ASSIGN_OPT(Opt_Callback_Number_SliderChangeMode_Void)
ASSIGN_OPT(Opt_SliderOptions)
ASSIGN_OPT(Opt_Literal_Union_Number_String_height_width_x_y)
ASSIGN_OPT(Opt_MenuAlignType)
ASSIGN_OPT(Opt_Union_Union_DividerOptions_Undefined_Undefined)
ASSIGN_OPT(Opt_Union_Dimension_OptionWidthMode)
ASSIGN_OPT(Opt_ArrowPosition)
ASSIGN_OPT(Opt_Callback_Number_String_Void)
ASSIGN_OPT(Opt_SelectOption)
ASSIGN_OPT(Opt_Array_SelectOption)
ASSIGN_OPT(Opt_SearchButtonOptions)
ASSIGN_OPT(Opt_Callback_DeleteValue_Void)
ASSIGN_OPT(Opt_DeleteValue)
ASSIGN_OPT(Opt_Callback_DeleteValue_Boolean)
ASSIGN_OPT(Opt_Callback_InsertValue_Void)
ASSIGN_OPT(Opt_InsertValue)
ASSIGN_OPT(Opt_Callback_InsertValue_Boolean)
ASSIGN_OPT(Opt_SearchType)
ASSIGN_OPT(Opt_TextDecorationOptions)
ASSIGN_OPT(Opt_Callback_String_PasteEvent_Void)
ASSIGN_OPT(Opt_PreviewText)
ASSIGN_OPT(Opt_EditableTextOnChangeCallback)
ASSIGN_OPT(Opt_CaretStyle)
ASSIGN_OPT(Opt_Union_CancelButtonOptions_CancelButtonSymbolOptions)
ASSIGN_OPT(Opt_Union_IconOptions_SymbolGlyphModifier)
ASSIGN_OPT(Opt_Type_SearchInterface_options)
ASSIGN_OPT(Opt_ScrollBarOptions)
ASSIGN_OPT(Opt_OffsetOptions)
ASSIGN_OPT(Opt_ScrollSnapOptions)
ASSIGN_OPT(Opt_OnScrollFrameBeginHandlerResult)
ASSIGN_OPT(Opt_OnScrollFrameBeginCallback)
ASSIGN_OPT(Opt_Union_Color_Number_String)
ASSIGN_OPT(Opt_Edge)
ASSIGN_OPT(Opt_OnScrollEdgeCallback)
ASSIGN_OPT(Opt_ScrollDirection)
ASSIGN_OPT(Opt_SaveButtonOnClickResult)
ASSIGN_OPT(Opt_Callback_ClickEvent_SaveButtonOnClickResult_Void)
ASSIGN_OPT(Opt_SaveButtonOptions)
ASSIGN_OPT(Opt_RowOptions)
ASSIGN_OPT(Opt_RootSceneSession)
ASSIGN_OPT(Opt_PlaceholderStyle)
ASSIGN_OPT(Opt_KeyboardOptions)
ASSIGN_OPT(Opt_SelectionMenuOptions)
ASSIGN_OPT(Opt_Union_ResponseType_RichEditorResponseType)
ASSIGN_OPT(Opt_RichEditorSpanType)
ASSIGN_OPT(Opt_EditMenuOptions)
ASSIGN_OPT(Opt_CopyEvent)
ASSIGN_OPT(Opt_Callback_CopyEvent_Void)
ASSIGN_OPT(Opt_CutEvent)
ASSIGN_OPT(Opt_Callback_CutEvent_Void)
ASSIGN_OPT(Opt_RichEditorChangeValue)
ASSIGN_OPT(Opt_Callback_RichEditorChangeValue_Boolean)
ASSIGN_OPT(Opt_SubmitEvent)
ASSIGN_OPT(Opt_SubmitCallback)
ASSIGN_OPT(Opt_EnterKeyType)
ASSIGN_OPT(Opt_TextDataDetectorType)
ASSIGN_OPT(Opt_TextDataDetectorConfig)
ASSIGN_OPT(Opt_PasteEvent)
ASSIGN_OPT(Opt_PasteEventCallback)
ASSIGN_OPT(Opt_Union_RichEditorTextSpanResult_RichEditorImageSpanResult)
ASSIGN_OPT(Opt_RichEditorDeleteValue)
ASSIGN_OPT(Opt_Callback_RichEditorDeleteValue_Boolean)
ASSIGN_OPT(Opt_Callback_TextRange_Void)
ASSIGN_OPT(Opt_Callback_RichEditorTextSpanResult_Void)
ASSIGN_OPT(Opt_RichEditorInsertValue)
ASSIGN_OPT(Opt_Callback_RichEditorInsertValue_Boolean)
ASSIGN_OPT(Opt_RichEditorRange)
ASSIGN_OPT(Opt_Callback_RichEditorRange_Void)
ASSIGN_OPT(Opt_RichEditorSelection)
ASSIGN_OPT(Opt_Callback_RichEditorSelection_Void)
ASSIGN_OPT(Opt_RichEditorStyledStringOptions)
ASSIGN_OPT(Opt_RichEditorOptions)
ASSIGN_OPT(Opt_LocalizedBarrierStyle)
ASSIGN_OPT(Opt_Array_LocalizedBarrierStyle)
ASSIGN_OPT(Opt_BarrierStyle)
ASSIGN_OPT(Opt_Array_BarrierStyle)
ASSIGN_OPT(Opt_GuideLineStyle)
ASSIGN_OPT(Opt_Array_GuideLineStyle)
ASSIGN_OPT(Opt_Union_Number_Undefined)
ASSIGN_OPT(Opt_RefreshStatus)
ASSIGN_OPT(Opt_Callback_RefreshStatus_Void)
ASSIGN_OPT(Opt_RefreshOptions)
ASSIGN_OPT(Opt_Type_RectInterface_value)
ASSIGN_OPT(Opt_StarStyleOptions)
ASSIGN_OPT(Opt_RatingOptions)
ASSIGN_OPT(Opt_RadioStyle)
ASSIGN_OPT(Opt_RadioOptions)
ASSIGN_OPT(Opt_Union_LinearStyleOptions_RingStyleOptions_CapsuleStyleOptions_ProgressStyleOptions)
ASSIGN_OPT(Opt_ProgressOptions)
ASSIGN_OPT(Opt_Literal_PluginComponentTemplate_template_Any_data)
ASSIGN_OPT(Opt_Union_CircleStyleOptions_Undefined)
ASSIGN_OPT(Opt_Callback_Array_Number_Void)
ASSIGN_OPT(Opt_PatternLockController)
ASSIGN_OPT(Opt_Literal_Union_Number_String_height_width_String_commands)
ASSIGN_OPT(Opt_PasteButtonOnClickResult)
ASSIGN_OPT(Opt_Callback_ClickEvent_PasteButtonOnClickResult_Void)
ASSIGN_OPT(Opt_PasteButtonOptions)
ASSIGN_OPT(Opt_Callback_Number_Number_PanelMode_Void)
ASSIGN_OPT(Opt_Union_Dimension_PanelHeight)
ASSIGN_OPT(Opt_PanelType)
ASSIGN_OPT(Opt_PanelMode)
ASSIGN_OPT(Opt_NodeController)
ASSIGN_OPT(Opt_Literal_String_target_NavigationType_type)
ASSIGN_OPT(Opt_Array_LayoutSafeAreaEdge)
ASSIGN_OPT(Opt_Array_LayoutSafeAreaType)
ASSIGN_OPT(Opt_NavigationToolbarOptions)
ASSIGN_OPT(Opt_ToolbarItem)
ASSIGN_OPT(Opt_Union_Array_ToolbarItem_CustomBuilder)
ASSIGN_OPT(Opt_Type_NavigationAttribute_title_value)
ASSIGN_OPT(Opt_Union_SystemBarStyle_Undefined)
ASSIGN_OPT(Opt_NavigationTransitionProxy)
ASSIGN_OPT(Opt_Union_NavigationAnimatedTransition_Undefined)
ASSIGN_OPT(Opt_NavigationOperation)
ASSIGN_OPT(Opt_Type_NavigationAttribute_customNavContentTransition_delegate)
ASSIGN_OPT(Opt_Callback_String_Unknown_Void)
ASSIGN_OPT(Opt_Callback_NavigationMode_Void)
ASSIGN_OPT(Opt_Callback_NavigationTitleMode_Void)
ASSIGN_OPT(Opt_Union_Object_CustomBuilder)
ASSIGN_OPT(Opt_Union_Array_NavigationMenuItem_CustomBuilder)
ASSIGN_OPT(Opt_NavigationTitleMode)
ASSIGN_OPT(Opt_Union_String_PixelMap_Resource_SymbolGlyphModifier)
ASSIGN_OPT(Opt_NavigationMode)
ASSIGN_OPT(Opt_NavBarPosition)
ASSIGN_OPT(Opt_NavRouteMode)
ASSIGN_OPT(Opt_RouteInfo)
ASSIGN_OPT(Opt_LayoutSafeAreaEdge)
ASSIGN_OPT(Opt_LayoutSafeAreaType)
ASSIGN_OPT(Opt_NavigationTitleOptions)
ASSIGN_OPT(Opt_Type_NavDestinationAttribute_title_value)
ASSIGN_OPT(Opt_NavigationSystemTransitionType)
ASSIGN_OPT(Opt_PopInfo)
ASSIGN_OPT(Opt_Callback_NavDestinationContext_Void)
ASSIGN_OPT(Opt_NavigationMenuItem)
ASSIGN_OPT(Opt_Union_ResourceStr_PixelMap_SymbolGlyphModifier)
ASSIGN_OPT(Opt_Callback_Boolean)
ASSIGN_OPT(Opt_MenuItemGroupOptions)
ASSIGN_OPT(Opt_Union_Boolean_ResourceStr_SymbolGlyphModifier)
ASSIGN_OPT(Opt_Union_MenuItemOptions_CustomBuilder)
ASSIGN_OPT(Opt_SubMenuExpandingMode)
ASSIGN_OPT(Opt_Union_DividerStyleOptions_Undefined)
ASSIGN_OPT(Opt_Union_PixelMap_ResourceStr_DrawableDescriptor_ASTCResource)
ASSIGN_OPT(Opt_MarqueeUpdateStrategy)
ASSIGN_OPT(Opt_MarqueeOptions)
ASSIGN_OPT(Opt_LocationButtonOnClickResult)
ASSIGN_OPT(Opt_Callback_ClickEvent_LocationButtonOnClickResult_Void)
ASSIGN_OPT(Opt_LocationButtonOptions)
ASSIGN_OPT(Opt_Union_Position_Edges_LocalizedEdges)
ASSIGN_OPT(Opt_SecurityComponentLayoutDirection)
ASSIGN_OPT(Opt_Union_ListDividerOptions_Undefined)
ASSIGN_OPT(Opt_ListItemGroupOptions)
ASSIGN_OPT(Opt_SwipeActionState)
ASSIGN_OPT(Opt_SwipeActionOptions)
ASSIGN_OPT(Opt_Union_Boolean_EditMode)
ASSIGN_OPT(Opt_Sticky)
ASSIGN_OPT(Opt_ListItemOptions)
ASSIGN_OPT(Opt_Union_Number_LengthConstrain)
ASSIGN_OPT(Opt_Callback_Number_Number_Boolean)
ASSIGN_OPT(Opt_Callback_Number_Boolean)
ASSIGN_OPT(Opt_VisibleListContentInfo)
ASSIGN_OPT(Opt_OnScrollVisibleContentChangeCallback)
ASSIGN_OPT(Opt_Callback_Number_Number_Number_Void)
ASSIGN_OPT(Opt_ChildrenMainSize)
ASSIGN_OPT(Opt_StickyStyle)
ASSIGN_OPT(Opt_ChainAnimationOptions)
ASSIGN_OPT(Opt_ListItemAlign)
ASSIGN_OPT(Opt_ListOptions)
ASSIGN_OPT(Opt_ImageLoadResult)
ASSIGN_OPT(Opt_ImageCompleteCallback)
ASSIGN_OPT(Opt_Union_ColorFilter_DrawingColorFilter)
ASSIGN_OPT(Opt_Union_ResourceStr_PixelMap)
ASSIGN_OPT(Opt_ImageFrameInfo)
ASSIGN_OPT(Opt_Array_ImageFrameInfo)
ASSIGN_OPT(Opt_ImageAnalyzerConfig)
ASSIGN_OPT(Opt_ImageError)
ASSIGN_OPT(Opt_ImageErrorCallback)
ASSIGN_OPT(Opt_Type_ImageAttribute_onComplete_callback_event)
ASSIGN_OPT(Opt_Callback_Type_ImageAttribute_onComplete_callback_event_Void)
ASSIGN_OPT(Opt_CopyOptions)
ASSIGN_OPT(Opt_ImageSourceSize)
ASSIGN_OPT(Opt_ImageInterpolation)
ASSIGN_OPT(Opt_DynamicRangeMode)
ASSIGN_OPT(Opt_ImageRenderMode)
ASSIGN_OPT(Opt_Union_PixelMap_ResourceStr_DrawableDescriptor)
ASSIGN_OPT(Opt_Union_PixelMap_ResourceStr_DrawableDescriptor_ImageContent)
ASSIGN_OPT(Opt_Union_Color_Number_String_Resource)
ASSIGN_OPT(Opt_GridRowOptions)
ASSIGN_OPT(Opt_GridContainerOptions)
ASSIGN_OPT(Opt_GridColOptions)
ASSIGN_OPT(Opt_GridItemOptions)
ASSIGN_OPT(Opt_Literal_Number_offsetRemain)
ASSIGN_OPT(Opt_Callback_Number_ScrollState_Literal_Number_offsetRemain)
ASSIGN_OPT(Opt_Union_GridItemAlignment_Undefined)
ASSIGN_OPT(Opt_Callback_ItemDragInfo_Number_Number_Boolean_Void)
ASSIGN_OPT(Opt_Callback_ItemDragInfo_Number_Void)
ASSIGN_OPT(Opt_Callback_ItemDragInfo_Number_Number_Void)
ASSIGN_OPT(Opt_Callback_ItemDragInfo_Void)
ASSIGN_OPT(Opt_ItemDragInfo)
ASSIGN_OPT(Opt_Callback_ItemDragInfo_Number_Callback_Any)
ASSIGN_OPT(Opt_GridDirection)
ASSIGN_OPT(Opt_Callback_Number_Number_Void)
ASSIGN_OPT(Opt_ComputedBarAttribute)
ASSIGN_OPT(Opt_Callback_Number_Number_ComputedBarAttribute)
ASSIGN_OPT(Opt_Tuple_Number_Number_Number_Number)
ASSIGN_OPT(Opt_GridLayoutOptions)
ASSIGN_OPT(Opt_FadingEdgeOptions)
ASSIGN_OPT(Opt_EdgeEffectOptions)
ASSIGN_OPT(Opt_ScrollState)
ASSIGN_OPT(Opt_Callback_Number_ScrollState_Void)
ASSIGN_OPT(Opt_GaugeIndicatorOptions)
ASSIGN_OPT(Opt_GaugeShadowOptions)
ASSIGN_OPT(Opt_Tuple_Union_ResourceColor_LinearGradient_Number)
ASSIGN_OPT(Opt_Type_GaugeAttribute_colors_colors)
ASSIGN_OPT(Opt_GaugeOptions)
ASSIGN_OPT(Opt_FormLinkOptions)
ASSIGN_OPT(Opt_Callback_Any_Void)
ASSIGN_OPT(Opt_Literal_Number_errcode_String_msg)
ASSIGN_OPT(Opt_Callback_Literal_Number_errcode_String_msg_Void)
ASSIGN_OPT(Opt_FormCallbackInfo)
ASSIGN_OPT(Opt_Callback_FormCallbackInfo_Void)
ASSIGN_OPT(Opt_Literal_Number_height_width)
ASSIGN_OPT(Opt_FormInfo)
ASSIGN_OPT(Opt_HoverEventParam)
ASSIGN_OPT(Opt_OnHoverStatusChangeCallback)
ASSIGN_OPT(Opt_OnFoldStatusChangeInfo)
ASSIGN_OPT(Opt_OnFoldStatusChangeCallback)
ASSIGN_OPT(Opt_FolderStackOptions)
ASSIGN_OPT(Opt_FlexOptions)
ASSIGN_OPT(Opt_TerminationInfo)
ASSIGN_OPT(Opt_Callback_TerminationInfo_Void)
ASSIGN_OPT(Opt_EmbeddedType)
ASSIGN_OPT(Opt_Literal_Union_String_Number_height_width)
ASSIGN_OPT(Opt_DatePickerResult)
ASSIGN_OPT(Opt_DatePickerOptions)
ASSIGN_OPT(Opt_DataPanelShadowOptions)
ASSIGN_OPT(Opt_DataPanelOptions)
ASSIGN_OPT(Opt_SceneOptions)
ASSIGN_OPT(Opt_Union_ColumnSplitDividerStyle_Undefined)
ASSIGN_OPT(Opt_PointLightStyle)
ASSIGN_OPT(Opt_ColumnOptions)
ASSIGN_OPT(Opt_CircleOptions)
ASSIGN_OPT(Opt_LineJoinStyle)
ASSIGN_OPT(Opt_LineCapStyle)
ASSIGN_OPT(Opt_CheckboxGroupResult)
ASSIGN_OPT(Opt_OnCheckboxGroupChangeCallback)
ASSIGN_OPT(Opt_CheckboxGroupOptions)
ASSIGN_OPT(Opt_OnCheckboxChangeCallback)
ASSIGN_OPT(Opt_MarkStyle)
ASSIGN_OPT(Opt_CheckBoxShape)
ASSIGN_OPT(Opt_CheckboxOptions)
ASSIGN_OPT(Opt_ImageAnalyzerType)
ASSIGN_OPT(Opt_Union_CanvasRenderingContext2D_DrawingRenderingContext)
ASSIGN_OPT(Opt_CalendarAlign)
ASSIGN_OPT(Opt_CalendarOptions)
ASSIGN_OPT(Opt_CalendarRequestedData)
ASSIGN_OPT(Opt_Callback_CalendarRequestedData_Void)
ASSIGN_OPT(Opt_CalendarSelectedDate)
ASSIGN_OPT(Opt_Callback_CalendarSelectedDate_Void)
ASSIGN_OPT(Opt_WorkStateStyle)
ASSIGN_OPT(Opt_WeekStyle)
ASSIGN_OPT(Opt_TodayStyle)
ASSIGN_OPT(Opt_NonCurrentDayStyle)
ASSIGN_OPT(Opt_CurrentDayStyle)
ASSIGN_OPT(Opt_CalendarDay)
ASSIGN_OPT(Opt_Type_CalendarInterface_value)
ASSIGN_OPT(Opt_ButtonOptions)
ASSIGN_OPT(Opt_BadgeParamWithString)
ASSIGN_OPT(Opt_BadgeParamWithNumber)
ASSIGN_OPT(Opt_Union_SpringMotion_FrictionMotion_ScrollMotion)
ASSIGN_OPT(Opt_FillMode)
ASSIGN_OPT(Opt_AnimationStatus)
ASSIGN_OPT(Opt_IndexerAlign)
ASSIGN_OPT(Opt_OnAlphabetIndexerPopupSelectCallback)
ASSIGN_OPT(Opt_OnAlphabetIndexerRequestPopupDataCallback)
ASSIGN_OPT(Opt_OnAlphabetIndexerSelectCallback)
ASSIGN_OPT(Opt_AlphabetIndexerOptions)
ASSIGN_OPT(Opt_Literal_Want_want)
ASSIGN_OPT(Opt_ModifierKey)
ASSIGN_OPT(Opt_Array_ModifierKey)
ASSIGN_OPT(Opt_Union_String_FunctionKey)
ASSIGN_OPT(Opt_VisibleAreaChangeCallback)
ASSIGN_OPT(Opt_SpringBackAction)
ASSIGN_OPT(Opt_DismissSheetAction)
ASSIGN_OPT(Opt_SheetDismiss)
ASSIGN_OPT(Opt_SheetOptions)
ASSIGN_OPT(Opt_DismissContentCoverAction)
ASSIGN_OPT(Opt_ContentCoverOptions)
ASSIGN_OPT(Opt_ContextMenuOptions)
ASSIGN_OPT(Opt_Union_Array_MenuElement_CustomBuilder)
ASSIGN_OPT(Opt_MenuOptions)
ASSIGN_OPT(Opt_MenuElement)
ASSIGN_OPT(Opt_DismissPopupAction)
ASSIGN_OPT(Opt_Literal_Boolean_isVisible)
ASSIGN_OPT(Opt_Union_PopupOptions_CustomPopupOptions)
ASSIGN_OPT(Opt_GeometryTransitionOptions)
ASSIGN_OPT(Opt_Union_BlendMode_Blender)
ASSIGN_OPT(Opt_BlendApplyType)
ASSIGN_OPT(Opt_OverlayOptions)
ASSIGN_OPT(Opt_Union_String_CustomBuilder_ComponentContent)
ASSIGN_OPT(Opt_DragInteractionOptions)
ASSIGN_OPT(Opt_DragPreviewOptions)
ASSIGN_OPT(Opt_ChainStyle)
ASSIGN_OPT(Opt_sharedTransitionOptions)
ASSIGN_OPT(Opt_FractionStop)
ASSIGN_OPT(Opt_LinearGradientBlurOptions)
ASSIGN_OPT(Opt_GestureMask)
ASSIGN_OPT(Opt_Union_TransitionFinishCallback_Undefined)
ASSIGN_OPT(Opt_FocusPriority)
ASSIGN_OPT(Opt_ForegroundBlurStyleOptions)
ASSIGN_OPT(Opt_BackgroundBlurStyleOptions)
ASSIGN_OPT(Opt_ImageRepeat)
ASSIGN_OPT(Opt_Literal_Alignment_align)
ASSIGN_OPT(Opt_SafeAreaEdge)
ASSIGN_OPT(Opt_Array_SafeAreaEdge)
ASSIGN_OPT(Opt_SafeAreaType)
ASSIGN_OPT(Opt_Array_SafeAreaType)
ASSIGN_OPT(Opt_Union_Object_Undefined)
ASSIGN_OPT(Opt_SizeChangeCallback)
ASSIGN_OPT(Opt_Callback_TouchEvent_HitTestMode)
ASSIGN_OPT(Opt_Array_GestureRecognizer)
ASSIGN_OPT(Opt_ShouldBuiltInRecognizerParallelWithCallback)
ASSIGN_OPT(Opt_GestureRecognizer)
ASSIGN_OPT(Opt_GestureRecognizerJudgeBeginCallback)
ASSIGN_OPT(Opt_GestureJudgeResult)
ASSIGN_OPT(Opt_FingerInfo)
ASSIGN_OPT(Opt_BaseGestureEvent)
ASSIGN_OPT(Opt_GestureInfo)
ASSIGN_OPT(Opt_Callback_GestureInfo_BaseGestureEvent_GestureJudgeResult)
ASSIGN_OPT(Opt_BackgroundBrightnessOptions)
ASSIGN_OPT(Opt_GestureModifier)
ASSIGN_OPT(Opt_RenderFit)
ASSIGN_OPT(Opt_ObscuredReasons)
ASSIGN_OPT(Opt_Array_ObscuredReasons)
ASSIGN_OPT(Opt_PixelStretchEffectOptions)
ASSIGN_OPT(Opt_StateStyles)
ASSIGN_OPT(Opt_Union_CircleShape_EllipseShape_PathShape_RectShape)
ASSIGN_OPT(Opt_Type_CommonMethod_mask_value)
ASSIGN_OPT(Opt_Type_CommonMethod_clip_value)
ASSIGN_OPT(Opt_Type_CommonMethod_radialGradient_value)
ASSIGN_OPT(Opt_Type_CommonMethod_sweepGradient_value)
ASSIGN_OPT(Opt_Type_CommonMethod_linearGradient_value)
ASSIGN_OPT(Opt_PreDragStatus)
ASSIGN_OPT(Opt_Callback_PreDragStatus_Void)
ASSIGN_OPT(Opt_Union_CustomBuilder_DragItemInfo_String)
ASSIGN_OPT(Opt_Union_Array_UniformDataType_Undefined)
ASSIGN_OPT(Opt_Callback_DragEvent_String_Void)
ASSIGN_OPT(Opt_Union_CustomBuilder_DragItemInfo)
ASSIGN_OPT(Opt_DragEvent)
ASSIGN_OPT(Opt_Callback_DragEvent_String_Union_CustomBuilder_DragItemInfo)
ASSIGN_OPT(Opt_Union_ClickEffect_Undefined)
ASSIGN_OPT(Opt_LocalizedAlignRuleOptions)
ASSIGN_OPT(Opt_AlignRuleOption)
ASSIGN_OPT(Opt_Literal_Union_Number_Literal_Number_offset_span_lg_md_sm_xs)
ASSIGN_OPT(Opt_Union_Position_LocalizedPosition)
ASSIGN_OPT(Opt_Direction)
ASSIGN_OPT(Opt_Visibility)
ASSIGN_OPT(Opt_Callback_Area_Area_Void)
ASSIGN_OPT(Opt_Union_Number_InvertOptions)
ASSIGN_OPT(Opt_Union_Color_String_Resource)
ASSIGN_OPT(Opt_MotionBlurOptions)
ASSIGN_OPT(Opt_Union_TransitionOptions_TransitionEffect)
ASSIGN_OPT(Opt_FocusBoxStyle)
ASSIGN_OPT(Opt_Callback_KeyEvent_Boolean)
ASSIGN_OPT(Opt_KeyEvent)
ASSIGN_OPT(Opt_Callback_KeyEvent_Void)
ASSIGN_OPT(Opt_TouchObject)
ASSIGN_OPT(Opt_Callback_TouchEvent_Void)
ASSIGN_OPT(Opt_MouseEvent)
ASSIGN_OPT(Opt_Callback_MouseEvent_Void)
ASSIGN_OPT(Opt_HoverEffect)
ASSIGN_OPT(Opt_AccessibilityHoverEvent)
ASSIGN_OPT(Opt_AccessibilityCallback)
ASSIGN_OPT(Opt_HoverEvent)
ASSIGN_OPT(Opt_Callback_Boolean_HoverEvent_Void)
ASSIGN_OPT(Opt_ClickEvent)
ASSIGN_OPT(Opt_Union_ResourceColor_ColoringStrategy)
ASSIGN_OPT(Opt_Union_Dimension_OutlineRadiuses)
ASSIGN_OPT(Opt_Union_Dimension_EdgeOutlineWidths)
ASSIGN_OPT(Opt_Union_OutlineStyle_EdgeOutlineStyles)
ASSIGN_OPT(Opt_OutlineOptions)
ASSIGN_OPT(Opt_Tuple_ResourceColor_Number)
ASSIGN_OPT(Opt_BorderImageOption)
ASSIGN_OPT(Opt_BorderOptions)
ASSIGN_OPT(Opt_ForegroundEffectOptions)
ASSIGN_OPT(Opt_ResizableOptions)
ASSIGN_OPT(Opt_BackgroundEffectOptions)
ASSIGN_OPT(Opt_Union_Position_Alignment)
ASSIGN_OPT(Opt_Union_SizeOptions_ImageSize)
ASSIGN_OPT(Opt_PixelRoundPolicy)
ASSIGN_OPT(Opt_Union_Margin_Length_LocalizedMargin)
ASSIGN_OPT(Opt_Union_Padding_Length_LocalizedPadding)
ASSIGN_OPT(Opt_TouchResult)
ASSIGN_OPT(Opt_TouchTestInfo)
ASSIGN_OPT(Opt_Array_TouchTestInfo)
ASSIGN_OPT(Opt_Callback_Array_TouchTestInfo_TouchResult)
ASSIGN_OPT(Opt_HitTestMode)
ASSIGN_OPT(Opt_ConstraintSizeOptions)
ASSIGN_OPT(Opt_Union_Array_Rectangle_Rectangle)
ASSIGN_OPT(Opt_Union_DrawModifier_Undefined)
#undef ASSIGN_OPT
}

#endif // CONVERTER_GENERATED_H