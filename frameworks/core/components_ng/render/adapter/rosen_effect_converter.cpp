/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "core/components_ng/render/adapter/rosen_effect_converter.h"

#include "render_service_client/core/ui_effect/property/include/rs_ui_filter_base.h"
#include "render_service_client/core/ui_effect/property/include/rs_ui_shader_base.h"
#include "ui/properties/ui_material_structs.h"

#include "core/components/common/properties/ui_material.h"

namespace OHOS::Ace::NG {
namespace {
#if !defined(PREVIEW)
constexpr uint32_t maskBlurParams = 0b0001u;
constexpr uint32_t maskRefactorParams = 0b0001u;
constexpr uint32_t maskSdParams = 0b0111u;
constexpr uint32_t maskEnvLightParams = 0b0111u;
constexpr uint32_t maskEdLightParams = 0b0011u;
const Rosen::Vector2f EMPTY_ROSEN_VECTOR2F { 0.0f, 0.0f };
constexpr FrostedGlassParam::Array2 TINTED_GLASS_BG_RATES { 0.0f, 0.0f };
constexpr FrostedGlassParam::Array3 TINTED_GLASS_BG_KBS { 1.0f, 0.0f, 1.0f };
constexpr FrostedGlassParam::Array3 TINTED_GLASS_BG_POS { 1.0f, 1.0f, 1.0f };
constexpr FrostedGlassParam::Array3 TINTED_GLASS_BG_NEG { 1.0f, 1.0f, 1.0f };
const Rosen::Vector3f TINTED_GLASS_LUMA_PARAMS { 0.6628f, 0.3137f, 0.1538f };
constexpr float TINTED_GLASS_MATERIAL_COLOR_FRACTION = 0.0f;

inline float InterpolateFloat(float a, float b, float fraction)
{
    return a * fraction + b * (1.0f - fraction);
}

Rosen::Vector2f InterpolateToVec2f(
    const FrostedGlassParam::Array2& a, const FrostedGlassParam::Array2& b, float alpha)
{
    return { InterpolateFloat(a[0], b[0], alpha), InterpolateFloat(a[1], b[1], alpha) };
}

Rosen::Vector3f InterpolateToVec3f(
    const FrostedGlassParam::Array3& a, const FrostedGlassParam::Array3& b, float alpha)
{
    return { InterpolateFloat(a[0], b[0], alpha), InterpolateFloat(a[1], b[1], alpha),
             InterpolateFloat(a[2], b[2], alpha) };
}
#endif
} // namespace
std::shared_ptr<Rosen::RSNGFilterBase> RosenEffectConverter::ConvertToFrostedGlassFilter(
    const FrostedGlassParam& param, float oneVpValue, const ExtraGlassParam* extraParam)
{
#if !defined(PREVIEW)
    auto filter = std::make_shared<Rosen::RSNGFrostedGlassFilter>();
    filter->Setter<Rosen::FrostedGlassBlurParamsTag>(ConvertToVector2f(param.blurParams, oneVpValue, maskBlurParams));
    filter->Setter<Rosen::FrostedGlassWeightsEmbossTag>(ConvertToVector2f(param.weightsEmboss));
    filter->Setter<Rosen::FrostedGlassWeightsEdlTag>(ConvertToVector2f(param.weightsEdl));
    filter->Setter<Rosen::FrostedGlassBgAlphaTag>(param.bgAlpha);
    filter->Setter<Rosen::FrostedGlassRefractParamsTag>(
        ConvertToVector3f(param.refractParams, oneVpValue, maskRefactorParams));
    filter->Setter<Rosen::FrostedGlassSdParamsTag>(ConvertToVector3f(param.sdParams, oneVpValue, maskSdParams));
    filter->Setter<Rosen::FrostedGlassSdRatesTag>(ConvertToVector2f(param.sdRates));
    filter->Setter<Rosen::FrostedGlassSdKBSTag>(ConvertToVector3f(param.sdKBS));
    filter->Setter<Rosen::FrostedGlassSdPosTag>(ConvertToVector3f(param.sdPos));
    filter->Setter<Rosen::FrostedGlassSdNegTag>(ConvertToVector3f(param.sdNeg));
    filter->Setter<Rosen::FrostedGlassEnvLightParamsTag>(
        ConvertToVector3f(param.envLightParams, oneVpValue, maskEnvLightParams));
    filter->Setter<Rosen::FrostedGlassEnvLightRatesTag>(ConvertToVector2f(param.envLightRates));
    filter->Setter<Rosen::FrostedGlassEnvLightKBSTag>(ConvertToVector3f(param.envLightKBS));
    filter->Setter<Rosen::FrostedGlassEnvLightPosTag>(ConvertToVector3f(param.envLightPos));
    filter->Setter<Rosen::FrostedGlassEnvLightNegTag>(ConvertToVector3f(param.envLightNeg));
    filter->Setter<Rosen::FrostedGlassEdLightParamsTag>(
        ConvertToVector2f(param.edLightParams, oneVpValue, maskEdLightParams));
    filter->Setter<Rosen::FrostedGlassEdLightAnglesTag>(ConvertToVector2f(param.edLightAngles));
    filter->Setter<Rosen::FrostedGlassEdLightDirTag>(ConvertToVector2f(param.edLightDir));
    filter->Setter<Rosen::FrostedGlassEdLightRatesTag>(ConvertToVector2f(param.edLightRates));
    filter->Setter<Rosen::FrostedGlassEdLightKBSTag>(ConvertToVector3f(param.edLightKBS));
    filter->Setter<Rosen::FrostedGlassEdLightPosTag>(ConvertToVector3f(param.edLightPos));
    filter->Setter<Rosen::FrostedGlassEdLightNegTag>(ConvertToVector3f(param.edLightNeg));
    filter->Setter<Rosen::FrostedGlassBaseVibrancyEnabledTag>(param.baseVibrancyEnabled);
    filter->Setter<Rosen::FrostedGlassSamplingScaleTag>(param.samplingScale);
    filter->Setter<Rosen::FrostedGlassDarkModeBlurParamsTag>(
        ConvertToVector2f(param.darkModeBlurParams, oneVpValue, maskBlurParams));
    filter->Setter<Rosen::FrostedGlassDarkModeWeightsEmbossTag>(ConvertToVector2f(param.darkModeWeightsEmboss));
    // no FrostedGlassDarkModeEdLightAnglesTag in env.
    // no FrostedGlassDarkModeEdLightKBSTag in env.
    if (extraParam) {
        float alpha = extraParam->alpha;
        filter->Setter<Rosen::FrostedGlassColorBlendModeTag>(static_cast<int>(MaterialColorBlendMode::TINTED_GLASS));
        filter->Setter<Rosen::FrostedGlassVibrancyStrengthTag>(1.0f - alpha);
        filter->Setter<Rosen::FrostedGlassLumaParamsTag>(TINTED_GLASS_LUMA_PARAMS);
        filter->Setter<Rosen::FrostedGlassMaterialColorFractionTag>(TINTED_GLASS_MATERIAL_COLOR_FRACTION);
        filter->Setter<Rosen::FrostedGlassBgRatesTag>(InterpolateToVec2f(TINTED_GLASS_BG_RATES, param.bgRates, alpha));
        filter->Setter<Rosen::FrostedGlassBgKBSTag>(InterpolateToVec3f(TINTED_GLASS_BG_KBS, param.bgKBS, alpha));
        filter->Setter<Rosen::FrostedGlassBgPosTag>(InterpolateToVec3f(TINTED_GLASS_BG_POS, param.bgPos, alpha));
        filter->Setter<Rosen::FrostedGlassBgNegTag>(InterpolateToVec3f(TINTED_GLASS_BG_NEG, param.bgNeg, alpha));
        filter->Setter<Rosen::FrostedGlassDarkModeBgRatesTag>(
            InterpolateToVec2f(TINTED_GLASS_BG_RATES, param.darkModeBgRates, alpha));
        filter->Setter<Rosen::FrostedGlassDarkModeBgKBSTag>(
            InterpolateToVec3f(TINTED_GLASS_BG_KBS, param.darkModeBgKBS, alpha));
        filter->Setter<Rosen::FrostedGlassDarkModeBgPosTag>(
            InterpolateToVec3f(TINTED_GLASS_BG_POS, param.darkModeBgPos, alpha));
        filter->Setter<Rosen::FrostedGlassDarkModeBgNegTag>(
            InterpolateToVec3f(TINTED_GLASS_BG_NEG, param.darkModeBgNeg, alpha));
    } else {
        filter->Setter<Rosen::FrostedGlassBgRatesTag>(ConvertToVector2f(param.bgRates));
        filter->Setter<Rosen::FrostedGlassBgKBSTag>(ConvertToVector3f(param.bgKBS));
        filter->Setter<Rosen::FrostedGlassBgPosTag>(ConvertToVector3f(param.bgPos));
        filter->Setter<Rosen::FrostedGlassBgNegTag>(ConvertToVector3f(param.bgNeg));
        filter->Setter<Rosen::FrostedGlassDarkModeBgRatesTag>(ConvertToVector2f(param.darkModeBgRates));
        filter->Setter<Rosen::FrostedGlassDarkModeBgKBSTag>(ConvertToVector3f(param.darkModeBgKBS));
        filter->Setter<Rosen::FrostedGlassDarkModeBgPosTag>(ConvertToVector3f(param.darkModeBgPos));
        filter->Setter<Rosen::FrostedGlassDarkModeBgNegTag>(ConvertToVector3f(param.darkModeBgNeg));
    }
    return filter;
#else
    return nullptr;
#endif
}

std::shared_ptr<Rosen::RSNGFilterBase> RosenEffectConverter::ConvertToFrostedGlassFilterEC(
    const FrostedGlassParam& param, float oneVpValue)
{
#if !defined(PREVIEW) && !defined(CROSS_PLATFORM)
    auto filter = std::make_shared<Rosen::RSNGFrostedGlassBlurFilter>();
    auto blurParams = ConvertToVector2f(param.blurParams, oneVpValue, maskBlurParams);
    auto envLightParams = ConvertToVector3f(param.envLightParams, oneVpValue, maskEnvLightParams);
    filter->Setter<Rosen::FrostedGlassBlurRadiusTag>(blurParams[0]);
    filter->Setter<Rosen::FrostedGlassBlurRadiusScaleKTag>(blurParams[1]);
    filter->Setter<Rosen::FrostedGlassBlurRefractOutPxTag>(envLightParams[0]);

    return filter;
#else
    return nullptr;
#endif
}

std::shared_ptr<Rosen::RSNGShaderBase> RosenEffectConverter::ConvertToRSNGFrostedGlassEffectECSub(
    const FrostedGlassParam& param, float oneVpValue, bool needSplitOverlayShader, const ExtraGlassParam* extraParam)
{
#if !defined(PREVIEW) && !defined(CROSS_PLATFORM)
    auto filter = std::make_shared<Rosen::RSNGFrostedGlassEffect>();
    auto envLightParams = ConvertToVector3f(param.envLightParams, oneVpValue, maskEnvLightParams);
    if (needSplitOverlayShader) {
        filter->Setter<Rosen::FrostedGlassEffectWeightsEmbossTag>(EMPTY_ROSEN_VECTOR2F);
        filter->Setter<Rosen::FrostedGlassEffectWeightsEdlTag>(EMPTY_ROSEN_VECTOR2F);
        // no FrostedGlassEffectDarkModeWeightsEmbossTag in env.
    } else {
        filter->Setter<Rosen::FrostedGlassEffectWeightsEmbossTag>(ConvertToVector2f(param.weightsEmboss));
        filter->Setter<Rosen::FrostedGlassEffectWeightsEdlTag>(ConvertToVector2f(param.weightsEdl));
        // no FrostedGlassEffectDarkModeWeightsEmbossTag in env.
    }
    filter->Setter<Rosen::FrostedGlassEffectBgAlphaTag>(param.bgAlpha);
    filter->Setter<Rosen::FrostedGlassEffectRefractParamsTag>(
        ConvertToVector3f(param.refractParams, oneVpValue, maskRefactorParams));
    filter->Setter<Rosen::FrostedGlassEffectSdParamsTag>(ConvertToVector3f(param.sdParams, oneVpValue, maskSdParams));
    filter->Setter<Rosen::FrostedGlassEffectSdRatesTag>(ConvertToVector2f(param.sdRates));
    filter->Setter<Rosen::FrostedGlassEffectSdKBSTag>(ConvertToVector3f(param.sdKBS));
    filter->Setter<Rosen::FrostedGlassEffectSdPosTag>(ConvertToVector3f(param.sdPos));
    filter->Setter<Rosen::FrostedGlassEffectSdNegTag>(ConvertToVector3f(param.sdNeg));
    filter->Setter<Rosen::FrostedGlassEffectEnvLightParamsTag>(
        Rosen::Vector2f { envLightParams[1], envLightParams[2] });
    filter->Setter<Rosen::FrostedGlassEffectEnvLightRatesTag>(ConvertToVector2f(param.envLightRates));
    filter->Setter<Rosen::FrostedGlassEffectEnvLightKBSTag>(ConvertToVector3f(param.envLightKBS));
    filter->Setter<Rosen::FrostedGlassEffectEnvLightPosTag>(ConvertToVector3f(param.envLightPos));
    filter->Setter<Rosen::FrostedGlassEffectEnvLightNegTag>(ConvertToVector3f(param.envLightNeg));
    filter->Setter<Rosen::FrostedGlassEffectEdLightParamsTag>(
        ConvertToVector2f(param.edLightParams, oneVpValue, maskEdLightParams));
    filter->Setter<Rosen::FrostedGlassEffectEdLightAnglesTag>(ConvertToVector2f(param.edLightAngles));
    filter->Setter<Rosen::FrostedGlassEffectEdLightDirTag>(ConvertToVector2f(param.edLightDir));
    filter->Setter<Rosen::FrostedGlassEffectEdLightRatesTag>(ConvertToVector2f(param.edLightRates));
    filter->Setter<Rosen::FrostedGlassEffectEdLightKBSTag>(ConvertToVector3f(param.edLightKBS));
    filter->Setter<Rosen::FrostedGlassEffectEdLightPosTag>(ConvertToVector3f(param.edLightPos));
    filter->Setter<Rosen::FrostedGlassEffectEdLightNegTag>(ConvertToVector3f(param.edLightNeg));
    // no FrostedGlassEffectDarkModeEdLightAnglesTag in blue env.
    // no FrostedGlassEffectDarkModeEdLightKBSTag in blue env.

    if (extraParam) {
        float alpha = extraParam->alpha;
        filter->Setter<Rosen::FrostedGlassEffectColorBlendModeTag>(
            static_cast<int>(MaterialColorBlendMode::TINTED_GLASS));
        filter->Setter<Rosen::FrostedGlassEffectVibrancyStrengthTag>(1.0f - alpha);
        filter->Setter<Rosen::FrostedGlassEffectLumaParamsTag>(TINTED_GLASS_LUMA_PARAMS);
        filter->Setter<Rosen::FrostedGlassEffectMaterialColorFractionTag>(TINTED_GLASS_MATERIAL_COLOR_FRACTION);
        filter->Setter<Rosen::FrostedGlassEffectBgRatesTag>(
            InterpolateToVec2f(TINTED_GLASS_BG_RATES, param.bgRates, alpha));
        filter->Setter<Rosen::FrostedGlassEffectBgKBSTag>(InterpolateToVec3f(TINTED_GLASS_BG_KBS, param.bgKBS, alpha));
        filter->Setter<Rosen::FrostedGlassEffectBgPosTag>(InterpolateToVec3f(TINTED_GLASS_BG_POS, param.bgPos, alpha));
        filter->Setter<Rosen::FrostedGlassEffectBgNegTag>(InterpolateToVec3f(TINTED_GLASS_BG_NEG, param.bgNeg, alpha));
        // no FrostedGlassEffectDarkModeBgRatesTag in env.
        // no FrostedGlassEffectDarkModeBgKBSTag in env.
        // no FrostedGlassEffectDarkModeBgPosTag in env.
        // no FrostedGlassEffectDarkModeBgNegTag in env.
    } else {
        filter->Setter<Rosen::FrostedGlassEffectBgRatesTag>(ConvertToVector2f(param.bgRates));
        filter->Setter<Rosen::FrostedGlassEffectBgKBSTag>(ConvertToVector3f(param.bgKBS));
        filter->Setter<Rosen::FrostedGlassEffectBgPosTag>(ConvertToVector3f(param.bgPos));
        filter->Setter<Rosen::FrostedGlassEffectBgNegTag>(ConvertToVector3f(param.bgNeg));
        // no FrostedGlassEffectDarkModeBgRatesTag in env.
        // no FrostedGlassEffectDarkModeBgKBSTag in env.
        // no FrostedGlassEffectDarkModeBgPosTag in env.
        // no FrostedGlassEffectDarkModeBgNegTag in env.
    }
    return filter;
#else
    return nullptr;
#endif
}

std::shared_ptr<Rosen::RSNGShaderBase> RosenEffectConverter::ConvertToRSNGFrostedGlassEffectECSubOverlay(
    const FrostedGlassParam& param, float oneVpValue, const ExtraGlassParam* extraParam)
{
#if !defined(PREVIEW) && !defined(CROSS_PLATFORM)
    auto filter = std::make_shared<Rosen::RSNGFrostedGlassEffect>();
    auto envLightParams = ConvertToVector3f(param.envLightParams, oneVpValue, maskEnvLightParams);
    filter->Setter<Rosen::FrostedGlassEffectWeightsEmbossTag>(ConvertToVector2f(param.weightsEmboss));
    filter->Setter<Rosen::FrostedGlassEffectWeightsEdlTag>(ConvertToVector2f(param.weightsEdl));
    filter->Setter<Rosen::FrostedGlassEffectBgAlphaTag>(0.0f);
    filter->Setter<Rosen::FrostedGlassEffectRefractParamsTag>(
        ConvertToVector3f(param.refractParams, oneVpValue, maskRefactorParams));
    filter->Setter<Rosen::FrostedGlassEffectSdParamsTag>(ConvertToVector3f(param.sdParams, oneVpValue, maskSdParams));
    filter->Setter<Rosen::FrostedGlassEffectSdRatesTag>(ConvertToVector2f(param.sdRates));
    filter->Setter<Rosen::FrostedGlassEffectSdKBSTag>(ConvertToVector3f(param.sdKBS));
    filter->Setter<Rosen::FrostedGlassEffectSdPosTag>(ConvertToVector3f(param.sdPos));
    filter->Setter<Rosen::FrostedGlassEffectSdNegTag>(ConvertToVector3f(param.sdNeg));
    filter->Setter<Rosen::FrostedGlassEffectEnvLightParamsTag>(
        Rosen::Vector2f { envLightParams[1], envLightParams[2] });
    filter->Setter<Rosen::FrostedGlassEffectEnvLightRatesTag>(ConvertToVector2f(param.envLightRates));
    filter->Setter<Rosen::FrostedGlassEffectEnvLightKBSTag>(ConvertToVector3f(param.envLightKBS));
    filter->Setter<Rosen::FrostedGlassEffectEnvLightPosTag>(ConvertToVector3f(param.envLightPos));
    filter->Setter<Rosen::FrostedGlassEffectEnvLightNegTag>(ConvertToVector3f(param.envLightNeg));
    filter->Setter<Rosen::FrostedGlassEffectEdLightParamsTag>(
        ConvertToVector2f(param.edLightParams, oneVpValue, maskEdLightParams));
    filter->Setter<Rosen::FrostedGlassEffectEdLightAnglesTag>(ConvertToVector2f(param.edLightAngles));
    filter->Setter<Rosen::FrostedGlassEffectEdLightDirTag>(ConvertToVector2f(param.edLightDir));
    filter->Setter<Rosen::FrostedGlassEffectEdLightRatesTag>(ConvertToVector2f(param.edLightRates));
    filter->Setter<Rosen::FrostedGlassEffectEdLightKBSTag>(ConvertToVector3f(param.edLightKBS));
    filter->Setter<Rosen::FrostedGlassEffectEdLightPosTag>(ConvertToVector3f(param.edLightPos));
    filter->Setter<Rosen::FrostedGlassEffectEdLightNegTag>(ConvertToVector3f(param.edLightNeg));
    // no FrostedGlassEffectDarkModeWeightsEmbossTag in blue env.
    // no FrostedGlassEffectDarkModeEdLightAnglesTag in blue env.
    // no FrostedGlassEffectDarkModeEdLightKBSTag in blue env.

    if (extraParam) {
        float alpha = extraParam->alpha;
        filter->Setter<Rosen::FrostedGlassEffectColorBlendModeTag>(
            static_cast<int>(MaterialColorBlendMode::TINTED_GLASS));
        filter->Setter<Rosen::FrostedGlassEffectVibrancyStrengthTag>(1.0f - alpha);
        filter->Setter<Rosen::FrostedGlassEffectLumaParamsTag>(TINTED_GLASS_LUMA_PARAMS);
        filter->Setter<Rosen::FrostedGlassEffectMaterialColorFractionTag>(TINTED_GLASS_MATERIAL_COLOR_FRACTION);
        filter->Setter<Rosen::FrostedGlassEffectBgRatesTag>(
            InterpolateToVec2f(TINTED_GLASS_BG_RATES, param.bgRates, alpha));
        filter->Setter<Rosen::FrostedGlassEffectBgKBSTag>(InterpolateToVec3f(TINTED_GLASS_BG_KBS, param.bgKBS, alpha));
        filter->Setter<Rosen::FrostedGlassEffectBgPosTag>(InterpolateToVec3f(TINTED_GLASS_BG_POS, param.bgPos, alpha));
        filter->Setter<Rosen::FrostedGlassEffectBgNegTag>(InterpolateToVec3f(TINTED_GLASS_BG_NEG, param.bgNeg, alpha));
        // no FrostedGlassEffectDarkModeBgRatesTag in env.
        // no FrostedGlassEffectDarkModeBgKBSTag in env.
        // no FrostedGlassEffectDarkModeBgPosTag in env.
        // no FrostedGlassEffectDarkModeBgNegTag in env.
    } else {
        filter->Setter<Rosen::FrostedGlassEffectBgRatesTag>(ConvertToVector2f(param.bgRates));
        filter->Setter<Rosen::FrostedGlassEffectBgKBSTag>(ConvertToVector3f(param.bgKBS));
        filter->Setter<Rosen::FrostedGlassEffectBgPosTag>(ConvertToVector3f(param.bgPos));
        filter->Setter<Rosen::FrostedGlassEffectBgNegTag>(ConvertToVector3f(param.bgNeg));
        // no FrostedGlassEffectDarkModeBgRatesTag in env.
        // no FrostedGlassEffectDarkModeBgKBSTag in env.
        // no FrostedGlassEffectDarkModeBgPosTag in env.
        // no FrostedGlassEffectDarkModeBgNegTag in env.
    }
    return filter;
#else
    return nullptr;
#endif
}

Rosen::Vector2f RosenEffectConverter::ConvertToVector2f(const std::array<float, 2>& arr)
{
    return Rosen::Vector2f { arr[0], arr[1] };
}

Rosen::Vector3f RosenEffectConverter::ConvertToVector3f(const std::array<float, 3>& arr)
{
    return Rosen::Vector3f { arr[0], arr[1], arr[2] };
}

Rosen::Vector2f RosenEffectConverter::ConvertToVector2f(
    const std::array<float, 2>& arr, float oneVpValue, uint32_t mask)
{
    float x = (mask & 0b0001u) ? (arr[0] * oneVpValue) : arr[0];
    float y = (mask & 0b0010u) ? (arr[1] * oneVpValue) : arr[1];
    return Rosen::Vector2f { x, y };
}

Rosen::Vector3f RosenEffectConverter::ConvertToVector3f(
    const std::array<float, 3>& arr, float oneVpValue, uint32_t mask)
{
    float x = (mask & 0b0001u) ? (arr[0] * oneVpValue) : arr[0];
    float y = (mask & 0b0010u) ? (arr[1] * oneVpValue) : arr[1];
    float z = (mask & 0b0100u) ? (arr[2] * oneVpValue) : arr[2];
    return Rosen::Vector3f { x, y, z };
}
} // namespace OHOS::Ace::NG
