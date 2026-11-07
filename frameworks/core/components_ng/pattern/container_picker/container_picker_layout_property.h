/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CONTAINER_PICKER_CONTAINER_PICKER_LAYOUT_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CONTAINER_PICKER_CONTAINER_PICKER_LAYOUT_PROPERTY_H

#include <string>
#include "base/geometry/dimension.h"
#include "base/json/json_util.h"
#include "base/utils/macros.h"
#include "core/common/resource/resource_object.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/pattern/container_picker/container_picker_theme.h"
#include "core/components_ng/pattern/text/text_styles.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/property/property.h"
#include "core/components_v2/inspector/utils.h"
#include "core/pipeline/pipeline_base.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
class InspectorFilter;

namespace {
static const LinearEnumMapNode<FontWeight, std::string> FONT_WEIGHT_TABLE[] = {
    { FontWeight::W100, "100" },
    { FontWeight::W200, "200" },
    { FontWeight::W300, "300" },
    { FontWeight::W400, "400" },
    { FontWeight::W500, "500" },
    { FontWeight::W600, "600" },
    { FontWeight::W700, "700" },
    { FontWeight::W800, "800" },
    { FontWeight::W900, "900" },
    { FontWeight::BOLD, "FontWeight.Bold" },
    { FontWeight::NORMAL, "FontWeight.Normal" },
    { FontWeight::BOLDER, "FontWeight.Bolder" },
    { FontWeight::LIGHTER, "FontWeight.Lighter" },
    { FontWeight::MEDIUM, "FontWeight.Medium" },
    { FontWeight::REGULAR, "FontWeight.Regular" },
};

static const LinearEnumMapNode<TextOverflow, std::string> TEXT_OVERFLOW_TABLE[] = {
    { TextOverflow::NONE, "TextOverflow.None" },
    { TextOverflow::CLIP, "TextOverflow.Clip" },
    { TextOverflow::ELLIPSIS, "TextOverflow.Ellipsis" },
    { TextOverflow::MARQUEE, "TextOverflow.Marquee" },
};

inline std::string ConvertWrapFontWeightToStirng(FontWeight fontWeight)
{
    auto index = BinarySearchFindIndex(FONT_WEIGHT_TABLE, ArraySize(FONT_WEIGHT_TABLE), fontWeight);
    return index < 0 ? "FontWeight.Normal" : FONT_WEIGHT_TABLE[index].value;
}

inline std::string ConvertWrapTextOverflowToString(TextOverflow textOverflow)
{
    auto index = BinarySearchFindIndex(TEXT_OVERFLOW_TABLE, ArraySize(TEXT_OVERFLOW_TABLE), textOverflow);
    return index < 0 ? "TextOverflow.Clip" : TEXT_OVERFLOW_TABLE[index].value;
}
}

enum class PickerIndicatorType { BACKGROUND = 0, DIVIDER };
static const char* INDICATOR_TYPE_VALUE[] = { "PickerIndicatorType.BACKGROUND", "PickerIndicatorType.DIVIDER" };
struct PickerIndicatorStyle {
    int32_t type = 0;
    std::optional<Dimension> strokeWidth;
    std::optional<Color> dividerColor;
    std::optional<Dimension> startMargin;
    std::optional<Dimension> endMargin;
    std::optional<Color> backgroundColor;
    std::optional<BorderRadiusProperty> borderRadius;
    bool isDefaultDividerWidth = true;
    bool isDefaultDividerColor = true;
    bool isDefaultStartMargin = true;
    bool isDefaultEndMargin = true;
    bool isDefaultBackgroundColor = true;
    bool isDefaultBorderRadius = true;
    RefPtr<ResourceObject> borderRadiusResObj;
};

class ACE_EXPORT ContainerPickerLayoutProperty : public LayoutProperty {
    DECLARE_ACE_TYPE(ContainerPickerLayoutProperty, LayoutProperty);

public:
    ContainerPickerLayoutProperty() = default;
    ~ContainerPickerLayoutProperty() override = default;

    RefPtr<LayoutProperty> Clone() const override
    {
        auto value = MakeRefPtr<ContainerPickerLayoutProperty>();
        value->LayoutProperty::UpdateLayoutProperty(DynamicCast<LayoutProperty>(this));
        value->propCanLoop_ = CloneCanLoop();
        value->propEnableHapticFeedback_ = CloneEnableHapticFeedback();
        value->propDisplayedItemCount_ = CloneDisplayedItemCount();
        value->propItemHeight_ = CloneItemHeight();
        value->propSelectedTextStyle_ = CloneSelectedTextStyle();
        value->propSelectedTextOverflow_ = CloneSelectedTextOverflow();
        return value;
    }

    void Reset() override
    {
        LayoutProperty::Reset();
        ResetCanLoop();
        ResetEnableHapticFeedback();
        ResetDisplayedItemCount();
        ResetItemHeight();
        ResetSelectedTextStyle();
        ResetSelectedTextOverflow();
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const override
    {
        LayoutProperty::ToJsonValue(json, filter);
        json->PutExtAttr("canLoop", V2::ConvertBoolToString(GetCanLoopValue(true)).c_str(), filter);
        json->PutExtAttr(
            "enableHapticFeedback", V2::ConvertBoolToString(GetEnableHapticFeedbackValue(true)).c_str(), filter);
        json->PutExtAttr("displayedItemCount",
            std::to_string(HasDisplayedItemCount() ? GetDisplayedItemCount().value() : 7).c_str(), filter);
        json->PutExtAttr("itemHeight",
            (HasItemHeight() ? GetItemHeight().value() : Dimension(40, DimensionUnit::VP)).ToString().c_str(), filter);

        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto pickerTheme = host->GetTheme<ContainerPickerTheme>(true);
        CHECK_NULL_VOID(pickerTheme);
        Dimension defaultDividerWidth = pickerTheme->GetStrokeWidth();
        Color defaultDividerColor = pickerTheme->GetIndicatorDividerColor();
        Color defaultIndicatorBackgroundColor = pickerTheme->GetIndicatorBackgroundColor();
        Dimension defaultIndicatorBorderRadius = pickerTheme->GetIndicatorBackgroundRadius();

        auto pickerIndicatorStyle = JsonUtil::Create(true);
        int32_t typeIndex = GetIndicatorType().value_or(0);
        typeIndex = (typeIndex < 0 || typeIndex >= static_cast<int32_t>(std::size(INDICATOR_TYPE_VALUE))) ? 0
            : typeIndex;
        pickerIndicatorStyle->Put("type", INDICATOR_TYPE_VALUE[typeIndex]);
        pickerIndicatorStyle->Put(
            "strokeWidth", GetIndicatorDividerWidth().value_or(defaultDividerWidth).ToString().c_str());
        pickerIndicatorStyle->Put(
            "dividerColor", GetIndicatorDividerColor().value_or(defaultDividerColor).ColorToString().c_str());
        pickerIndicatorStyle->Put("startMargin", GetIndicatorStartMargin().value_or(Dimension()).ToString().c_str());
        pickerIndicatorStyle->Put("endMargin", GetIndicatorEndMargin().value_or(Dimension()).ToString().c_str());
        pickerIndicatorStyle->Put("backgroundColor",
            GetIndicatorBackgroundColor().value_or(defaultIndicatorBackgroundColor).ColorToString().c_str());
        pickerIndicatorStyle->Put("borderRadius",
            GetIndicatorBorderRadius().value_or(BorderRadiusProperty(defaultIndicatorBorderRadius)).ToString().c_str());
        json->PutExtAttr("selectionIndicator", pickerIndicatorStyle, filter);

        auto selectedFont = JsonUtil::Create(true);
        selectedFont->Put("size", GetSelectedFontSizeValue(Dimension(0)).ToString().c_str());
        selectedFont->Put("weight", ConvertWrapFontWeightToStirng(
            GetSelectedWeight().value_or(FontWeight::NORMAL)).c_str());
        auto selectedTextStyleJson = JsonUtil::Create(true);
        selectedTextStyleJson->Put("color", GetSelectedColor().value_or(Color::BLACK).ColorToString().c_str());
        selectedTextStyleJson->Put("font", selectedFont);
        selectedTextStyleJson->Put("minFontSize", GetSelectedMinFontSize().value_or(Dimension()).ToString().c_str());
        selectedTextStyleJson->Put("maxFontSize", GetSelectedMaxFontSize().value_or(Dimension()).ToString().c_str());
        selectedTextStyleJson->Put("overflow",
            ConvertWrapTextOverflowToString(GetSelectedTextOverflow().value_or(TextOverflow::CLIP)).c_str());
        json->PutExtAttr("selectedTextStyle", selectedTextStyleJson, filter);
    }

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(SelectedIndex, int32_t, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(CanLoop, bool, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(EnableHapticFeedback, bool, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(DisplayedItemCount, int32_t, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(ItemHeight, Dimension, PROPERTY_UPDATE_MEASURE);

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(IndicatorType, int32_t, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(IndicatorDividerWidth, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(IndicatorDividerColor, Color, PROPERTY_UPDATE_MEASURE_SELF);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(IndicatorStartMargin, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(IndicatorEndMargin, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(IndicatorBackgroundColor, Color, PROPERTY_UPDATE_MEASURE_SELF);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(IndicatorBorderRadius, BorderRadiusProperty, PROPERTY_UPDATE_MEASURE);

    ACE_DEFINE_PROPERTY_GROUP(SelectedTextStyle, FontStyle);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        SelectedTextStyle, FontSize, SelectedFontSize, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        SelectedTextStyle, TextColor, SelectedColor, Color, PROPERTY_UPDATE_MEASURE_SELF);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        SelectedTextStyle, FontWeight, SelectedWeight, FontWeight, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        SelectedTextStyle, FontFamily, SelectedFontFamily, std::vector<std::string>, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        SelectedTextStyle, ItalicFontStyle, SelectedFontStyle, Ace::FontStyle, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        SelectedTextStyle, AdaptMinFontSize, SelectedMinFontSize, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP_ITEM(
        SelectedTextStyle, AdaptMaxFontSize, SelectedMaxFontSize, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(SelectedTextOverflow, TextOverflow, PROPERTY_UPDATE_MEASURE);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CONTAINER_PICKER_CONTAINER_PICKER_LAYOUT_PROPERTY_H
