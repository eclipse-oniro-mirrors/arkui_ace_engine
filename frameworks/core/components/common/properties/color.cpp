/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "core/components/common/properties/color.h"

#include <cstdlib>
#include <regex>

#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/common/resource/resource_manager.h"

namespace OHOS::Ace {
namespace {

constexpr uint32_t COLOR_ALPHA_OFFSET = 24;
constexpr uint32_t COLOR_STRING_SIZE_STANDARD = 8;
constexpr uint32_t COLOR_STRING_BASE = 16;
constexpr uint32_t RGB_SUB_MATCH_SIZE = 4;
constexpr uint32_t RGBA_SUB_MATCH_SIZE = 5;

const std::regex COLOR_WITH_MAGIC("#[0-9A-Fa-f]{6,8}");
const std::regex COLOR_WITH_MAGIC_MINI("#[0-9A-Fa-f]{3,4}");
const std::regex COLOR_WITH_RGB(R"(rgb\(([0-9]{1,3})\,([0-9]{1,3})\,([0-9]{1,3})\))", std::regex::icase);
const std::regex COLOR_WITH_RGBA(R"(rgba\(([0-9]{1,3})\,([0-9]{1,3})\,([0-9]{1,3})\,(\d+\.?\d*)\))", std::regex::icase);
constexpr double GAMMA_FACTOR = 2.2;
constexpr float MAX_ALPHA = 255.0f;
constexpr char HEX[] = "0123456789ABCDEF";
constexpr uint8_t BIT_LENGTH_INT32 = 8;
constexpr uint8_t MIN_RGB_VALUE = 0;
constexpr uint8_t MAX_RGB_VALUE = 255;
constexpr double MIN_RGBA_OPACITY = 0.0;
constexpr double MAX_RGBA_OPACITY = 1.0;
const std::vector<size_t> EXPECT_MAGIC_COLOR_LENGTHS = {7, 9};
const std::vector<size_t> EXPECT_MAGIC_MINI_COLOR_LENGTHS = {4, 5};
} // namespace

const Color Color::TRANSPARENT = Color(0x00000000);
const Color Color::WHITE = Color(0xffffffff);
const Color Color::BLACK = Color(0xff000000);
const Color Color::RED = Color(0xffff0000);
const Color Color::GREEN = Color(0xff00ff00);
const Color Color::BLUE = Color(0xff0000ff);
const Color Color::GRAY = Color(0xffc0c0c0);
const Color Color::FOREGROUND = Color(0x00000001); // foreground color and foreground color strategy identification

const LinearColor LinearColor::TRANSPARENT = LinearColor(0x00000000);
const LinearColor LinearColor::WHITE = LinearColor(0xffffffff);
const LinearColor LinearColor::BLACK = LinearColor(0xff000000);
const LinearColor LinearColor::RED = LinearColor(0xffff0000);
const LinearColor LinearColor::GREEN = LinearColor(0xff00ff00);
const LinearColor LinearColor::BLUE = LinearColor(0xff0000ff);
const LinearColor LinearColor::GRAY = LinearColor(0xffc0c0c0);

unsigned long int HandleIncorrectColor(const std::string& newColorStr)
{
    errno = 0;
    char* end = nullptr;
    unsigned long int value = strtoul(newColorStr.c_str(), &end, COLOR_STRING_BASE);
    if (errno == ERANGE) {
        LOGE("%{public}s is out of range.", newColorStr.c_str());
    }
    if (value == 0 && end == newColorStr.c_str()) {
        LOGW("input %{public}s can not be converted to number, use default color：0x00000000.", newColorStr.c_str());
    }
    return value;
}

Color Color::FromString(std::string colorStr, uint32_t maskAlpha, Color defaultColor)
{
    if (colorStr.empty()) {
        // empty string, return transparent
        return Color::TRANSPARENT;
    }

    // Remove all " ".
    colorStr.erase(std::remove(colorStr.begin(), colorStr.end(), ' '), colorStr.end());

    std::smatch matches;
    // Regex match for #909090 or #90909090.
    if (std::regex_match(colorStr, matches, COLOR_WITH_MAGIC)) {
        colorStr.erase(0, 1);
        unsigned long int value = HandleIncorrectColor(colorStr);
        if (colorStr.length() < COLOR_STRING_SIZE_STANDARD) {
            // no alpha specified, set alpha to 0xff
            value |= maskAlpha;
        }
        return Color(value);
    }
    // Regex match for #rgb or #rgba.
    if (std::regex_match(colorStr, matches, COLOR_WITH_MAGIC_MINI)) {
        colorStr.erase(0, 1);
        std::string newColorStr;
        // translate #rgb or #rgba to #rrggbb or #rrggbbaa
        for (auto& c : colorStr) {
            newColorStr += c;
            newColorStr += c;
        }
        unsigned long int value = HandleIncorrectColor(newColorStr);
        if (newColorStr.length() < COLOR_STRING_SIZE_STANDARD) {
            // no alpha specified, set alpha to 0xff
            value |= maskAlpha;
        }
        return Color(value);
    }
    // Regex match for rgb(90,254,180).
    if (std::regex_match(colorStr, matches, COLOR_WITH_RGB)) {
        if (matches.size() == RGB_SUB_MATCH_SIZE) {
            auto red = static_cast<uint8_t>(StringUtils::StringToInt(matches[1]));   // red value.
            auto green = static_cast<uint8_t>(StringUtils::StringToInt(matches[2])); // green value.
            auto blue = static_cast<uint8_t>(StringUtils::StringToInt(matches[3]));  // blue value.
            return FromRGB(red, green, blue);
        }
    }
    // Regex match for rgba(90,254,180,0.5).
    if (std::regex_match(colorStr, matches, COLOR_WITH_RGBA)) {
        if (matches.size() == RGBA_SUB_MATCH_SIZE) {
            auto red = static_cast<uint8_t>(StringUtils::StringToInt(matches[1]));
            auto green = static_cast<uint8_t>(StringUtils::StringToInt(matches[2]));
            auto blue = static_cast<uint8_t>(StringUtils::StringToInt(matches[3]));
            auto opacity = StringUtils::StringToDouble(matches[4]);
            return FromRGBO(red, green, blue, opacity);
        }
    }
    // match for special string
    static const LinearMapNode<Color> colorTable[] = {
        { "black", Color(0xff000000) },
        { "blue", Color(0xff0000ff) },
        { "gray", Color(0xffc0c0c0) },
        { "green", Color(0xff00ff00) },
        { "red", Color(0xffff0000) },
        { "white", Color(0xffffffff) },
    };
    int64_t colorIndex = BinarySearchFindIndex(colorTable, ArraySize(colorTable), colorStr.c_str());
    if (colorIndex != -1) {
        return colorTable[colorIndex].value;
    }

    // parse uint32_t color string.
    auto uint32Color = StringUtils::StringToUint(colorStr);
    if (uint32Color > 0) {
        Color value;
        if ((uint32Color >> COLOR_ALPHA_OFFSET) == 0) {
            value = Color(uint32Color).ChangeAlpha(MAX_ALPHA);
        } else {
            value = Color(uint32Color);
        }
        return value;
    }

    // Default color.
    return defaultColor;
}

bool Color::ParseColorString(const std::string& colorStr, Color& color, const Color& defaultColor, uint32_t maskAlpha)
{
    if (colorStr.empty()) {
        return false;
    }

    // Remove all " ".
    std::string tmpColorStr = colorStr;
    tmpColorStr.erase(std::remove(tmpColorStr.begin(), tmpColorStr.end(), ' '), tmpColorStr.end());

    return (MatchColorWithMagic(tmpColorStr, maskAlpha, color) ||
        MatchColorWithMagicMini(tmpColorStr, maskAlpha, color) ||
        MatchColorWithRGB(tmpColorStr, color) ||
        MatchColorWithRGBA(tmpColorStr, color) ||
        MatchColorSpecialString(tmpColorStr, color) ||
        ParseUintColorString(tmpColorStr, color, defaultColor));
}

bool Color::ParseColorString(std::string colorStr, Color& color, uint32_t maskAlpha)
{
    if (colorStr.empty()) {
        return false;
    }

    // Remove all " ".
    colorStr.erase(std::remove(colorStr.begin(), colorStr.end(), ' '), colorStr.end());

    return (MatchColorWithMagic(colorStr, maskAlpha, color) || MatchColorWithMagicMini(colorStr, maskAlpha, color) ||
            MatchColorWithRGB(colorStr, color) || MatchColorWithRGBA(colorStr, color) ||
            MatchColorSpecialString(colorStr, color) || ParseUintColorString(colorStr, color));
}

std::string Color::ColorToString() const
{
    std::string colorStr;
    int count = 0;
    uint32_t value = GetValue();
    while (count++ < BIT_LENGTH_INT32) {
        colorStr = HEX[(value & 0xf)] + colorStr;
        value >>= 4;
    }
    colorStr = "#" + colorStr;
    return colorStr;
}

// for example str = #FFFFFFFF
Color Color::ColorFromString(const std::string& str)
{
    static const int32_t colorStrLen = 9;
    static const int32_t offset = 4;

    if (str.length() != colorStrLen || str.find('#') != 0) {
        LOGE("UITree |ERROR| invalid %{public}s", str.c_str());
        return Color::BLACK;
    }

    std::string colorStr = str.substr(1, colorStrLen - 1);
    uint32_t value = 0;
    for (const auto& it : colorStr) {
        value <<= offset;
        value += it < 'A' ? it - '0' : it - '7';
    }

    return Color(value);
}

std::string Color::ToString() const
{
    return ColorToString();
}

std::string Color::ToSvgFillColorKey() const
{
    std::ostringstream oss;
    oss << ColorToString() << "_cs" << static_cast<int32_t>(colorSpace_);
    return oss.str();
}

Color Color::FromARGB(uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue)
{
    ColorParam colorValue {
#if BIG_ENDIANNESS
        .argb = { .alpha = alpha, .red = red, .green = green, .blue = blue }
#else
        .argb = { .blue = blue, .green = green, .red = red, .alpha = alpha }
#endif
    };
    return Color(colorValue);
}

Color Color::FromRGBO(uint8_t red, uint8_t green, uint8_t blue, double opacity)
{
    return FromARGB(static_cast<uint8_t>(round(opacity * 0xff)) & 0xff, red, green, blue);
}

Color Color::FromRGB(uint8_t red, uint8_t green, uint8_t blue)
{
    return FromARGB(0xff, red, green, blue);
}

Color Color::BlendColor(const Color& overlayColor) const
{
    if (GetValue() == Color::TRANSPARENT.GetValue()) {
        return overlayColor;
    }
    if (GetAlpha() < static_cast<uint8_t>(MAX_ALPHA)) {
        return BlendColorWithAlpha(overlayColor);
    }
    auto alphaRate = static_cast<float>(overlayColor.GetAlpha()) / MAX_ALPHA;
    auto newRed = static_cast<uint8_t>(GetRed() * (1.0f - alphaRate) + overlayColor.GetRed() * alphaRate);
    auto newGreen = static_cast<uint8_t>(GetGreen() * (1.0f - alphaRate) + overlayColor.GetGreen() * alphaRate);
    auto newBlue = static_cast<uint8_t>(GetBlue() * (1.0f - alphaRate) + overlayColor.GetBlue() * alphaRate);
    auto newColor = Color::FromRGB(newRed, newGreen, newBlue);
    newColor.SetColorSpace(GetColorSpace());
    return newColor;
}

float Color::CalculateBlend(float alphaLeft, float alphaRight, float valueLeft, float valueRight) const
{
    return (valueLeft * alphaLeft * (1.0 - alphaRight) + valueRight * alphaRight) /
           (alphaLeft + alphaRight - alphaLeft * alphaRight);
}

Color Color::BlendColorWithAlpha(const Color& overlayColor) const
{
    float alphaA = GetAlpha() / 255.0;
    float alphaB = overlayColor.GetAlpha() / 255.0;
    float blendAlpha = alphaA + alphaB - alphaA * alphaB;
    float blendRed = CalculateBlend(alphaA, alphaB, GetRed() / 255.0, overlayColor.GetRed() / 255.0);
    float blendGreen = CalculateBlend(alphaA, alphaB, GetGreen() / 255.0, overlayColor.GetGreen() / 255.0);
    float blendBlue = CalculateBlend(alphaA, alphaB, GetBlue() / 255.0, overlayColor.GetBlue() / 255.0);

    return Color::FromARGB(blendAlpha * 255, blendRed * 255, blendGreen * 255, blendBlue * 255);
}

const Color Color::LineColorTransition(const Color& startColor, const Color& endColor, double percent)
{
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    uint8_t alpha = 0;

    red = static_cast<uint8_t>((endColor.GetRed()- startColor.GetRed()) * percent) + startColor.GetRed();
    green = static_cast<uint8_t>((endColor.GetGreen() - startColor.GetGreen()) * percent) + startColor.GetGreen();
    blue = static_cast<uint8_t>((endColor.GetBlue() - startColor.GetBlue()) * percent) + startColor.GetBlue();
    alpha = static_cast<uint8_t>((endColor.GetAlpha() - startColor.GetAlpha()) * percent) + startColor.GetAlpha();

    return Color::FromARGB(alpha, red, green, blue);
}

Color Color::BlendOpacity(double opacityRatio) const
{
    int32_t alpha = static_cast<int32_t>(GetAlpha() * opacityRatio);
    alpha = std::clamp(alpha, 0, UINT8_MAX);
    return Color::FromARGB(alpha, GetRed(), GetGreen(), GetBlue());
}

Color Color::ChangeOpacity(double opacity) const
{
    return Color::FromRGBO(GetRed(), GetGreen(), GetBlue(), opacity);
}

Color Color::ChangeAlpha(uint8_t alpha) const
{
    return Color::FromARGB(alpha, GetRed(), GetGreen(), GetBlue());
}

Color Color::operator+(const Color& color) const
{
    // convert first color from ARGB to linear
    double firstLinearRed = 0.0;
    double firstLinearGreen = 0.0;
    double firstLinearBlue = 0.0;
    ConvertGammaToLinear(*this, firstLinearRed, firstLinearGreen, firstLinearBlue);

    // convert second color from ARGB to linear
    double secondLinearRed = 0.0;
    double secondLinearGreen = 0.0;
    double secondLinearBlue = 0.0;
    ConvertGammaToLinear(color, secondLinearRed, secondLinearGreen, secondLinearBlue);

    // get linear result and convert to gamma
    return ConvertLinearToGamma(GetAlpha() + color.GetAlpha(), firstLinearRed + secondLinearRed,
        firstLinearGreen + secondLinearGreen, firstLinearBlue + secondLinearBlue);
}

Color Color::operator-(const Color& color) const
{
    // convert first color from ARGB to linear
    double firstLinearRed = 0.0;
    double firstLinearGreen = 0.0;
    double firstLinearBlue = 0.0;
    ConvertGammaToLinear(*this, firstLinearRed, firstLinearGreen, firstLinearBlue);

    // convert second color from ARGB to linear
    double secondLinearRed = 0.0;
    double secondLinearGreen = 0.0;
    double secondLinearBlue = 0.0;
    ConvertGammaToLinear(color, secondLinearRed, secondLinearGreen, secondLinearBlue);

    // get linear result and convert to gamma
    return ConvertLinearToGamma(GetAlpha() - color.GetAlpha(), firstLinearRed - secondLinearRed,
        firstLinearGreen - secondLinearGreen, firstLinearBlue - secondLinearBlue);
}

Color Color::operator*(double value) const
{
    // convert color from ARGB to linear
    double linearRed = 0.0;
    double linearGreen = 0.0;
    double linearBlue = 0.0;
    ConvertGammaToLinear(*this, linearRed, linearGreen, linearBlue);

    // get linear result and convert to gamma
    return ConvertLinearToGamma(GetAlpha() * value, linearRed * value, linearGreen * value, linearBlue * value);
}

Color Color::operator/(double value) const
{
    if (NearZero(value)) {
        return *this;
    }
    // convert color from ARGB to linear
    double linearRed = 0.0;
    double linearGreen = 0.0;
    double LinearBlue = 0.0;
    ConvertGammaToLinear(*this, linearRed, linearGreen, LinearBlue);

    // get linear result and convert to gamma
    return ConvertLinearToGamma(GetAlpha() / value, linearRed / value, linearGreen / value, LinearBlue / value);
}

double Color::ConvertGammaToLinear(uint8_t value)
{
    return std::pow(value, GAMMA_FACTOR);
}

uint8_t Color::ConvertLinearToGamma(double value)
{
    return std::clamp(static_cast<int32_t>(round(std::pow(value, 1.0 / GAMMA_FACTOR))), 0, UINT8_MAX);
}

void Color::ConvertGammaToLinear(const Color& gammaColor, double& linearRed, double& linearGreen, double& linearBlue)
{
    linearRed = ConvertGammaToLinear(gammaColor.GetRed());
    linearGreen = ConvertGammaToLinear(gammaColor.GetGreen());
    linearBlue = ConvertGammaToLinear(gammaColor.GetBlue());
}

Color Color::ConvertLinearToGamma(double alpha, double linearRed, double linearGreen, double linearBlue)
{
    uint8_t gammaRed = ConvertLinearToGamma(linearRed);
    uint8_t gammaGreen = ConvertLinearToGamma(linearGreen);
    uint8_t gammaBlue = ConvertLinearToGamma(linearBlue);
    uint8_t gammaAlpha = std::clamp(static_cast<int32_t>(round(alpha)), 0, UINT8_MAX);

    return FromARGB(gammaAlpha, gammaRed, gammaGreen, gammaBlue);
}

bool Color::IsHexNumber(std::string& colorStr)
{
    for (size_t i = 1; i < colorStr.size(); i++) {
        if (colorStr[i] >= '0' && colorStr[i] <= '9') {
            continue;
        }
        if (colorStr[i] >= 'A' && colorStr[i] <= 'F') {
            continue;
        }
        if (colorStr[i] >= 'a' && colorStr[i] <= 'f') {
            continue;
        }
        return false;
    }
    return true;
}

bool Color::FastCheckColorType(const std::string& colorStr, const std::string& expectPrefix,
    const std::vector<size_t>&  expectLengths)
{
    if (colorStr.rfind(expectPrefix, 0) != 0) {
        return false;
    }
    if (expectLengths.size() == 0) {
        return true;
    }
    for (size_t expectLength : expectLengths) {
        if (expectLength == colorStr.size()) {
            return true;
        }
    }
    return false;
}

bool Color::MatchColorWithMagic(std::string& colorStr, uint32_t maskAlpha, Color& color)
{
    // Regex match for #909090 or #90909090
    if (!FastCheckColorType(colorStr, "#", EXPECT_MAGIC_COLOR_LENGTHS)) {
        return false;
    }
    if (!IsHexNumber(colorStr)) {
        return false;
    }
    colorStr.erase(0, 1);
    unsigned long int value = HandleIncorrectColor(colorStr);
    if (colorStr.length() < COLOR_STRING_SIZE_STANDARD) {
        // no alpha specified, set alpha to 0xff
        value |= maskAlpha;
    }
    color = Color(value);
    return true;
}

bool Color::MatchColorWithMagicMini(std::string& colorStr, uint32_t maskAlpha, Color& color)
{
    if (!FastCheckColorType(colorStr, "#", EXPECT_MAGIC_MINI_COLOR_LENGTHS)) {
        return false;
    }
    if (!IsHexNumber(colorStr)) {
        return false;
    }
    colorStr.erase(0, 1);
    std::string newColorStr;
    // translate #rgb or #rgba to #rrggbb or #rrggbbaa
    for (auto& c : colorStr) {
        newColorStr += c;
        newColorStr += c;
    }
    unsigned long int value = HandleIncorrectColor(newColorStr);
    
    if (newColorStr.length() < COLOR_STRING_SIZE_STANDARD) {
        // no alpha specified, set alpha to 0xff
        value |= maskAlpha;
    }
    color = Color(value);
    return true;
}

bool Color::MatchColorHexString(const std::string& colorStr)
{
    if (colorStr.empty()) {
        return false;
    }
    std::smatch matches;
    if (std::regex_match(colorStr, matches, COLOR_WITH_MAGIC) ||
        std::regex_match(colorStr, matches, COLOR_WITH_RGBA) ||
        std::regex_match(colorStr, matches, COLOR_WITH_RGB) ||
        std::regex_match(colorStr, matches, COLOR_WITH_MAGIC_MINI)) {
        return true;
    }
    return false;
}

bool Color::MatchColorWithRGB(const std::string& colorStr, Color& color)
{
    if (!FastCheckColorType(colorStr, "rgb(", {})) {
        return false;
    }
    std::smatch matches;
    if (std::regex_match(colorStr, matches, COLOR_WITH_RGB)) {
        if (matches.size() == RGB_SUB_MATCH_SIZE) {
            auto redInt = StringUtils::StringToInt(matches[1]);
            auto greenInt = StringUtils::StringToInt(matches[2]);
            auto blueInt = StringUtils::StringToInt(matches[3]);
            if (!IsRGBValid(redInt) || !IsRGBValid(greenInt) || !IsRGBValid(blueInt)) {
                return false;
            }

            auto red = static_cast<uint8_t>(redInt);
            auto green = static_cast<uint8_t>(greenInt);
            auto blue = static_cast<uint8_t>(blueInt);
            color = FromRGB(red, green, blue);
            return true;
        }
    }
    return false;
}

bool Color::MatchColorWithRGBA(const std::string& colorStr, Color& color)
{
    if (!FastCheckColorType(colorStr, "rgba(", {})) {
        return false;
    }
    std::smatch matches;
    if (std::regex_match(colorStr, matches, COLOR_WITH_RGBA)) {
        if (matches.size() == RGBA_SUB_MATCH_SIZE) {
            auto redInt = StringUtils::StringToInt(matches[1]);
            auto greenInt = StringUtils::StringToInt(matches[2]);
            auto blueInt = StringUtils::StringToInt(matches[3]);
            auto opacityDouble = StringUtils::StringToDouble(matches[4]);
            if (!IsRGBValid(redInt) || !IsRGBValid(greenInt) || !IsRGBValid(blueInt) ||
                !IsOpacityValid(opacityDouble)) {
                return false;
            }

            auto red = static_cast<uint8_t>(redInt);
            auto green = static_cast<uint8_t>(greenInt);
            auto blue = static_cast<uint8_t>(blueInt);
            auto opacity = static_cast<double>(opacityDouble);

            color = FromRGBO(red, green, blue, opacity);
            return true;
        }
    }

    return false;
}

bool Color::MatchColorSpecialString(const std::string& colorStr, Color& color)
{
    static const LinearMapNode<Color> colorTable[] = {
        { "black", Color(0xff000000) },
        { "blue", Color(0xff0000ff) },
        { "gray", Color(0xffc0c0c0) },
        { "green", Color(0xff00ff00) },
        { "red", Color(0xffff0000) },
        { "transparent", Color(0x00000000) },
        { "white", Color(0xffffffff) },
    };

    int64_t colorIndex = BinarySearchFindIndex(colorTable, ArraySize(colorTable), colorStr.c_str());
    if (colorIndex != -1) {
        color = colorTable[colorIndex].value;
        return true;
    }

    return false;
}

bool Color::ParseUintColorString(const std::string& colorStr, Color& color, const Color& defaultColor)
{
    auto uint32Color = StringUtils::StringToUintCheck(colorStr, defaultColor.GetValue());
    if (uint32Color > 0) {
        if ((uint32Color >> COLOR_ALPHA_OFFSET) == 0) {
            color = Color(uint32Color).ChangeAlpha(MAX_ALPHA);
        } else {
            color = Color(uint32Color);
        }
        return true;
    }

    return false;
}

bool Color::ParseUintColorString(const std::string& colorStr, Color& color)
{
    auto uint32Color = StringUtils::StringToUint(colorStr);
    if (uint32Color > 0) {
        if ((uint32Color >> COLOR_ALPHA_OFFSET) == 0) {
            color = Color(uint32Color).ChangeAlpha(MAX_ALPHA);
        } else {
            color = Color(uint32Color);
        }
        return true;
    }

    return false;
}

bool Color::IsRGBValid(int value)
{
    return value >= MIN_RGB_VALUE && value <= MAX_RGB_VALUE;
}

bool Color::IsOpacityValid(double value)
{
    return value >= MIN_RGBA_OPACITY && value <= MAX_RGBA_OPACITY;
}

void Color::UpdateColorByResourceId()
{
#ifndef ACE_UNITTEST
    CHECK_NULL_VOID(resourceId_ != 0);
    auto resourceAdapter = ResourceManager::GetInstance().GetResourceAdapter(Container::CurrentIdSafely());
    CHECK_NULL_VOID(resourceAdapter);
    auto newColor = resourceAdapter->GetColor(resourceId_);
    SetValue(newColor.GetValue());
#endif
}

} // namespace OHOS::Ace
