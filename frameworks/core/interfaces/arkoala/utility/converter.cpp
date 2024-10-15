/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "converter.h"
#include "reverse_converter.h"
#include "core/interfaces/arkoala/utility/validators.h"

#include "core/interfaces/native/node/node_api.h"
#include "frameworks/bridge/common/utils/utils.h"

namespace OHOS::Ace::NG::Converter {
void AssignArkValue(Ark_Resource& dst, const Ark_Length& src)
{
    dst.id = ArkValue<Ark_Number>(src.resource);
    dst.type = ArkValue<Ark_Number>(static_cast<Ark_Int32>(NodeModifier::ResourceType::FLOAT));
    dst.params = ArkValue<Opt_Array_String>();
}

void AssignArkValue(Ark_TouchObject& touch, const OHOS::Ace::TouchLocationInfo& info)
{
    Offset globalOffset = info.GetGlobalLocation();
    Offset localOffset = info.GetLocalLocation();
    Offset screenOffset = info.GetScreenLocation();

    touch.displayX.tag = Ark_Tag::ARK_TAG_FLOAT32;
    touch.displayX.f32 = static_cast<float>(
        PipelineBase::Px2VpWithCurrentDensity(screenOffset.GetX()));
    touch.displayY.tag = Ark_Tag::ARK_TAG_FLOAT32;
    touch.displayY.f32 = static_cast<float>(
        PipelineBase::Px2VpWithCurrentDensity(screenOffset.GetY()));

    touch.id.tag = Ark_Tag::ARK_TAG_INT32;
    touch.id.i32 = static_cast<int32_t>(info.GetTouchDeviceId());

    touch.screenX.tag = Ark_Tag::ARK_TAG_FLOAT32;
    touch.screenX.f32 = static_cast<float>(
        PipelineBase::Px2VpWithCurrentDensity(globalOffset.GetX()));
    touch.screenY.tag = Ark_Tag::ARK_TAG_FLOAT32;
    touch.screenY.f32 = static_cast<float>(
        PipelineBase::Px2VpWithCurrentDensity(globalOffset.GetY()));

    touch.type = static_cast<Ark_TouchType>(info.GetTouchType());

    touch.windowX.tag = Ark_Tag::ARK_TAG_FLOAT32;
    touch.windowX.f32 = static_cast<float>(
        PipelineBase::Px2VpWithCurrentDensity(globalOffset.GetX()));
    touch.windowY.tag = Ark_Tag::ARK_TAG_FLOAT32;
    touch.windowY.f32 = static_cast<float>(
        PipelineBase::Px2VpWithCurrentDensity(globalOffset.GetY()));

    touch.x.tag = Ark_Tag::ARK_TAG_FLOAT32;
    touch.x.f32 = static_cast<float>(
        PipelineBase::Px2VpWithCurrentDensity(localOffset.GetX()));
    touch.y.tag = Ark_Tag::ARK_TAG_FLOAT32;
    touch.y.f32 = static_cast<float>(
        PipelineBase::Px2VpWithCurrentDensity(localOffset.GetY()));
}

void AssignArkValue(Ark_ClickEvent& onClick, const OHOS::Ace::GestureEvent& info)
{
    Offset globalOffset = info.GetGlobalLocation();
    Offset localOffset = info.GetLocalLocation();
    Offset screenOffset = info.GetScreenLocation();

    onClick.axisHorizontal = ArkValue<Opt_Number>();
    onClick.axisVertical = ArkValue<Opt_Number>();
    onClick.displayX = ArkValue<Ark_Number>(PipelineBase::Px2VpWithCurrentDensity(screenOffset.GetX()));
    onClick.displayY = ArkValue<Ark_Number>(PipelineBase::Px2VpWithCurrentDensity(screenOffset.GetY()));

    onClick.pressure = ArkValue<Ark_Number>(0.0f);
    onClick.preventDefault.id = 0;

    onClick.screenX = ArkValue<Ark_Number>(PipelineBase::Px2VpWithCurrentDensity(globalOffset.GetX()));
    onClick.screenY = ArkValue<Ark_Number>(PipelineBase::Px2VpWithCurrentDensity(globalOffset.GetY()));

    onClick.source = static_cast<Ark_SourceType>(info.GetSourceDevice());

    onClick.sourceTool = static_cast<Ark_SourceTool>(0);
    onClick.deviceId = ArkValue<Opt_Number>();
    onClick.target.area.globalPosition.x.tag = Ark_Tag::ARK_TAG_UNDEFINED;
    onClick.target.area.globalPosition.y.tag = Ark_Tag::ARK_TAG_UNDEFINED;
    onClick.target.area.position.x.tag = Ark_Tag::ARK_TAG_UNDEFINED;
    onClick.target.area.position.y.tag = Ark_Tag::ARK_TAG_UNDEFINED;
    onClick.target.area.height = ArkValue<Ark_Length>(0);
    onClick.target.area.width = ArkValue<Ark_Length>(0);

    onClick.tiltX = ArkValue<Ark_Number>(0);
    onClick.tiltY = ArkValue<Ark_Number>(0);

    onClick.timestamp = ArkValue<Ark_Number>(
        static_cast<float>(info.GetTimeStamp().time_since_epoch().count()));

    onClick.windowX = ArkValue<Ark_Number>(PipelineBase::Px2VpWithCurrentDensity(globalOffset.GetX()));
    onClick.windowY = ArkValue<Ark_Number>(PipelineBase::Px2VpWithCurrentDensity(globalOffset.GetY()));

    onClick.x = ArkValue<Ark_Number>(PipelineBase::Px2VpWithCurrentDensity(localOffset.GetX()));
    onClick.y = ArkValue<Ark_Number>(PipelineBase::Px2VpWithCurrentDensity(localOffset.GetY()));
}

uint32_t ColorAlphaAdapt(uint32_t origin)
{
    constexpr uint32_t COLOR_ALPHA_OFFSET = 24;
    constexpr uint32_t COLOR_ALPHA_VALUE = 0xFF000000;

    uint32_t result = origin;
    if ((origin >> COLOR_ALPHA_OFFSET) == 0) {
        result = origin | COLOR_ALPHA_VALUE;
    }
    return result;
}

ResourceConverter::ResourceConverter(const Ark_Resource& resource)
{
    if (resource.id.tag == ARK_TAG_INT32 && resource.type.tag == ARK_TAG_INT32) {
        id_ = resource.id.i32;
        type_ = static_cast<NodeModifier::ResourceType>(resource.type.i32);
        bundleName_ = Convert<std::string>(resource.bundleName);
        moduleName_ = Convert<std::string>(resource.moduleName);
        if (resource.params.tag != ARK_TAG_UNDEFINED) {
            for (int i = 0; i < resource.params.value.length; i++) {
                params_.emplace_back(resource.params.value.array[i].chars);
            }
        }

        themeConstants_ = NodeModifier::GetThemeConstants(nullptr, bundleName_.c_str(), moduleName_.c_str());
    } else {
        LOGE("ResourceConverter illegal id/type tag: id.tag = %{public}d, type.tag = %{public}d",
             resource.id.tag, resource.type.tag);
    }
}

std::optional<std::string> ResourceConverter::ToString()
{
    std::optional<std::string> result;
    CHECK_NULL_RETURN(themeConstants_, result);
    switch (type_) {
        case NodeModifier::ResourceType::STRING:
            if (id_ != -1) {
                result = themeConstants_->GetString(id_);
            } else if (!params_.empty()) {
                result = themeConstants_->GetStringByName(params_.front());
            } else {
                LOGE("Unknown resource value OHOS::Ace::NG::Converter::ResourceConverter");
            }
            break;

        case NodeModifier::ResourceType::RAWFILE:
            if (params_.size() > 0) {
                result = themeConstants_->GetRawfile(params_.front());
            }
            break;

        case NodeModifier::ResourceType::MEDIA:
            if (id_ != -1) {
                result = themeConstants_->GetMediaPath(id_);
            } else if (params_.size() > 0) {
                result = themeConstants_->GetMediaPathByName(params_.front());
            }
            break;

        default:
            break;
    }
    return result;
}

std::optional<StringArray> ResourceConverter::ToStringArray()
{
    CHECK_NULL_RETURN(themeConstants_, std::nullopt);
    if (type_ == NodeModifier::ResourceType::STRARRAY) {
        if (id_ != -1) {
            return themeConstants_->GetStringArray(id_);
        } else if (params_.size() > 0) {
            return themeConstants_->GetStringArrayByName(params_.front());
        } else {
            LOGE("Unknown STRARRAY value OHOS::Ace::NG::Converter::ResourceConverter");
        }
    }
    return std::nullopt;
}

std::optional<StringArray> ResourceConverter::ToFontFamilies()
{
    CHECK_NULL_RETURN(themeConstants_, std::nullopt);
    if (type_ == NodeModifier::ResourceType::STRING) {
        std::optional<std::string> str;
        if (id_ != -1) {
            str = themeConstants_->GetString(id_);
        } else if (!params_.empty()) {
            str = themeConstants_->GetStringByName(params_.front());
        } else {
            LOGE("ResourceConverter::ToFontFamilies Unknown resource value");
        }
        if (str.has_value()) {
            return Framework::ConvertStrToFontFamilies(str.value());
        }
    } else if (type_ == NodeModifier::ResourceType::STRARRAY) {
        LOGE("ResourceConverter::ToFontFamilies Support of ResourceType::STRARRAY type is not implemented");
    } else {
        LOGE("ResourceConverter::ToFontFamilies Resource type is not supported");
    }
    return std::nullopt;
}

std::optional<Dimension> ResourceConverter::ToDimension()
{
    CHECK_NULL_RETURN(themeConstants_, std::nullopt);
    if (type_ == NodeModifier::ResourceType::FLOAT) {
        if (id_ == -1 && params_.size() > 0) {
            return themeConstants_->GetDimensionByName(params_.front());
        } else {
            return themeConstants_->GetDimension(id_);
        }
    }
    return std::nullopt;
}

std::optional<float> ResourceConverter::ToFloat()
{
    std::optional<float> optFloat = std::nullopt;
    CHECK_NULL_RETURN(themeConstants_, std::nullopt);
    if (type_ == NodeModifier::ResourceType::FLOAT) {
        if (id_ == -1 && params_.size() > 0) {
            optFloat = static_cast<float>(themeConstants_->GetDoubleByName(params_[0]));
        } else {
            optFloat = static_cast<float>(themeConstants_->GetDouble(id_));
        }
    }
    return optFloat;
}

std::optional<int32_t> ResourceConverter::ToInt()
{
    CHECK_NULL_RETURN(themeConstants_, std::nullopt);
    if (type_ == NodeModifier::ResourceType::INTEGER) {
        return themeConstants_->GetInt(id_);
    }
    return std::nullopt;
}

std::optional<Color> ResourceConverter::ToColor()
{
    std::optional<Color> result;
    CHECK_NULL_RETURN(themeConstants_, result);
    if (id_ == -1 && params_.size() > 0) {
        result = themeConstants_->GetColorByName(params_[0]);
        return result;
    }

    switch (type_) {
        case NodeModifier::ResourceType::STRING: {
            Color color;
            if (Color::ParseColorString(themeConstants_->GetString(id_), color)) {
                result = color;
            }
            break;
        }

        case NodeModifier::ResourceType::INTEGER:
            result = Color(ColorAlphaAdapt(themeConstants_->GetInt(id_)));
            break;

        case NodeModifier::ResourceType::COLOR:
            result = themeConstants_->GetColor(id_);
            break;

        default:
            break;
    }

    return result;
}

template<>
Shadow Convert(const Ark_ShadowOptions& src)
{
    Shadow shadow;

    auto radius = Converter::OptConvert<float>(src.radius);
    if (radius) {
        shadow.SetBlurRadius(radius.value());
    }

    auto shadowType = Converter::OptConvert<ShadowType>(src.type);
    if (shadowType) {
        shadow.SetShadowType(shadowType.value());
    }

    auto color = Converter::OptConvert<Color>(src.color);
    if (color) {
        shadow.SetColor(color.value());
    }

    auto offsetX = Converter::OptConvert<float>(src.offsetX);
    if (offsetX) {
        shadow.SetOffsetX(offsetX.value());
    }

    auto offsetY = Converter::OptConvert<float>(src.offsetY);
    if (offsetY) {
        shadow.SetOffsetY(offsetY.value());
    }

    auto fill = Converter::OptConvert<bool>(src.fill);
    if (fill) {
        shadow.SetIsFilled(fill.value());
    }

    return shadow;
}

template<>
std::vector<Shadow> Convert(const Ark_ShadowOptions& src)
{
    return { Convert<Shadow>(src) };
}

template<>
StringArray Convert(const Ark_String& src)
{
    auto familiesStr = Convert<std::string>(src);
    return Framework::ConvertStrToFontFamilies(familiesStr);
}

template<>
Font Convert(const Ark_Font& src)
{
    Font font;
    auto fontFamilies = OptConvert<std::vector<std::string>>(src.family);
    if (fontFamilies) {
        font.fontFamilies = fontFamilies.value();
    }
    auto fontSize = OptConvert<Dimension>(src.size);
    if (fontSize) {
        Validator::ValidatePositive(fontSize);
        Validator::ValidateNonPercent(fontSize);
        font.fontSize = fontSize;
    }
    auto weight = OptConvert<FontWeight>(src.weight);
    if (weight) {
        font.fontWeight = weight;
    }
    font.fontStyle = OptConvert<OHOS::Ace::FontStyle>(src.style);
    return font;
}

template<>
CaretStyle Convert(const Ark_CaretStyle& src)
{
    CaretStyle caretStyle;
    caretStyle.color = OptConvert<Color> (src.color);
    caretStyle.width = OptConvert<Dimension> (src.width);
    return caretStyle;
}

template<>
TextDecorationOptions Convert(const Ark_TextDecorationOptions& src)
{
    TextDecorationOptions options;
    options.textDecoration = OptConvert<TextDecoration>(src.type);
    options.color = OptConvert<Color>(src.color);
    options.textDecorationStyle = OptConvert<TextDecorationStyle>(src.style);
    return options;
}

template<>
ListItemIndex Convert(const Ark_VisibleListContentInfo& src)
{
    auto itemIndex = ListItemIndex{.index = Convert<int32_t>(src.index)}; // a struct is initialized by default
    auto itemGroupArea = OptConvert<ListItemGroupArea>(src.itemGroupArea);
    if (itemGroupArea.has_value()) {
        itemIndex.area = static_cast<int32_t>(itemGroupArea.value());
    }
    auto indexInGroup = OptConvert<int32_t>(src.itemIndexInGroup);
    if (indexInGroup.has_value()) {
        itemIndex.indexInGroup = indexInGroup.value();
    }
    return itemIndex;
}

template<>
std::pair<Dimension, Dimension> Convert(const Ark_LengthConstrain& src)
{
    auto minLength = Convert<Dimension>(src.minLength);
    auto maxLength = Convert<Dimension>(src.maxLength);
    return {minLength, maxLength};
}

template<>
ItemDragInfo Convert(const Ark_ItemDragInfo& src)
{
    ItemDragInfo itemDragInfo;
    itemDragInfo.SetX(Convert<float>(src.x));
    itemDragInfo.SetY(Convert<float>(src.y));
    return itemDragInfo;
}

template<>
void AssignCast(std::optional<FontWeight>& dst, const Ark_Number& src)
{
    auto intVal = src.tag == Ark_Tag::ARK_TAG_INT32 ? src.i32 : static_cast<int32_t>(src.f32);
    if (intVal >= 0) {
        auto strVal = std::to_string(intVal);
        if (auto [parseOk, val] = StringUtils::ParseFontWeight(strVal); parseOk) {
            dst = val;
        }
    }
}

template<>
void AssignCast(std::optional<FontWeight>& dst, const Ark_String& src)
{
    if (auto [parseOk, val] = StringUtils::ParseFontWeight(src.chars); parseOk) {
        dst = val;
    }
}

template<>
RefPtr<Curve> Convert(const Ark_String& src)
{
    return Framework::CreateCurve(Converter::Convert<std::string>(src), false);
}

template<>
RefPtr<Curve> Convert(const Ark_Curve& src)
{
    return Framework::CreateCurve(src, false);
}

template<>
RefPtr<Curve> Convert(const Ark_ICurve& src)
{
    LOGE("Convert [Ark_ICurve] to [RefPtr<Curve>] is not supported");
    return nullptr;
}

template<>
void AssignCast(std::optional<float>& dst, const Ark_String& src)
{
    auto value = Convert<std::string>(src);
    double result;
    if (StringUtils::StringToDouble(value, result)) {
        dst = result;
    }
}

template<>
Dimension Convert(const Ark_Length& src)
{
    if (src.type == Ark_Tag::ARK_TAG_RESOURCE) {
        auto resource = ArkValue<Ark_Resource>(src);
        ResourceConverter converter(resource);
        return converter.ToDimension().value_or(Dimension());
    } else {
        return Dimension(src.value, static_cast<DimensionUnit>(src.unit));
    }
}

template<>
PaddingProperty Convert(const Ark_Padding& src)
{
    PaddingProperty padding;
    padding.left = Converter::OptConvert<CalcLength>(src.left);
    padding.top = Converter::OptConvert<CalcLength>(src.top);
    padding.right = Converter::OptConvert<CalcLength>(src.right);
    padding.bottom = Converter::OptConvert<CalcLength>(src.bottom);
    return padding;
}

template<>
CalcLength Convert(const Ark_Length& src)
{
    if (src.type == Ark_Tag::ARK_TAG_RESOURCE) {
        auto resource = ArkValue<Ark_Resource>(src);
        ResourceConverter converter(resource);
        Dimension value = converter.ToDimension().value_or(Dimension());
        return CalcLength(value.Value(), value.Unit());
    }
    return CalcLength(src.value, static_cast<OHOS::Ace::DimensionUnit>(src.unit));
}

template<>
void AssignCast(std::optional<Color>& dst, const Ark_String& src)
{
    Color result;
    auto color = Convert<std::string>(src);
    if (Color::ParseColorString(color, result)) {
        dst = result;
    }
}
} // namespace OHOS::Ace::NG::Converter
