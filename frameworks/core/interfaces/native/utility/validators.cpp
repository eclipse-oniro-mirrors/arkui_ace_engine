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

#include <algorithm>

#include "validators.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::NG {
namespace Validator {
namespace {
    constexpr float DEFAULT_OPACITY = 1.0;
    constexpr float MIN_OPACITY = 0.0;
    constexpr float INTENSITY_MIN = 0.0f;
    constexpr float INTENSITY_MAX = 1.0f;
    constexpr float BLOOM_MIN = 0.0f;
    constexpr float BLOOM_MAX = 1.0f;
} // namespace

void ValidateNonNegative(std::optional<Dimension>& opt)
{
    if (opt.has_value() && opt.value().IsNegative()) {
        opt.reset();
    }
}
void ValidateNonPercent(std::optional<Dimension>& opt)
{
    if (opt.has_value() && (opt.value().Unit() == DimensionUnit::PERCENT)) {
        opt.reset();
    }
}
void ValidatePositive(std::optional<Dimension>& opt)
{
    if (opt.has_value() && opt.value().IsNonPositive()) {
        opt.reset();
    }
}
void ValidateOpacity(std::optional<float>& opt)
{
    if (opt.has_value()) {
        opt.value() = std::min(opt.value(), DEFAULT_OPACITY);
        opt.value() = std::max(opt.value(), MIN_OPACITY);
    }
}
void ValidatePositive(std::optional<float>& value)
{
    if (value.has_value() && NonPositive(value.value())) {
        value.reset();
    }
}

void ValidateNonNegative(std::optional<float>& value)
{
    if (value.has_value() && Negative(value.value())) {
        value.reset();
    }
}
void ValidateGreatOrEqual(std::optional<float>& opt, const float& right)
{
    if (opt.has_value() && LessNotEqual(opt.value(), right)) {
        opt.reset();
    }
}
void ValidateGreatOrEqual(std::optional<int32_t>& opt, const int32_t& right)
{
    if (opt.has_value() && opt.value() < right) {
        opt.reset();
    }
}
void ValidateLessOrEqual(std::optional<float>& opt, const float& right)
{
    if (opt.has_value() && GreatNotEqual(opt.value(), right)) {
        opt.reset();
    }
}
void ValidateByRange(std::optional<float>& opt, const float& left, const float& right)
{
    if (opt.has_value()) {
        ValidateGreatOrEqual(opt, left);
        ValidateLessOrEqual(opt, right);
    }
}
void ValidateNonNegative(std::optional<CalcDimension>& opt)
{
    if (opt.has_value() && opt.value().Unit() != DimensionUnit::CALC && opt.value().IsNegative()) {
        opt.reset();
    }
}
void ValidateNonPercent(std::optional<CalcDimension>& opt)
{
    if (opt.has_value() && (opt.value().Unit() == DimensionUnit::PERCENT)) {
        opt.reset();
    }
}
void ValidateNonEmpty(std::optional<CalcDimension>& opt)
{
    if (opt.has_value() && (opt.value().Unit() == DimensionUnit::CALC) && opt.value().CalcValue().empty()) {
        opt.reset();
    }
}
void ValidateNonNegative(std::optional<int>& value)
{
    if (value.has_value() && Negative(value.value())) {
        value.reset();
    }
}

void ValidatePositive(std::optional<int>& value)
{
    if (value.has_value() && NonPositive(value.value())) {
        value.reset();
    }
}

void ValidateNonNegative(std::optional<V2::GridContainerSize>& value)
{
    if (value.has_value()) {
        auto val = value.value();
        bool fail = Negative(val.lg) || Negative(val.md) || Negative(val.sm) || Negative(val.xl)
            || Negative(val.xs) || Negative(val.xxl);
        if (fail) {
            value.reset();
        }
    }
}
void ValidateBloom(std::optional<float>& opt)
{
    ValidateByRange(opt, BLOOM_MIN, BLOOM_MAX);
}
void ValidateIntensity(std::optional<float>& opt)
{
    ValidateByRange(opt, INTENSITY_MIN, INTENSITY_MAX);
}
void ValidateNonNegative(std::optional<CalcLength>& opt)
{
    if (opt.has_value() &&
        opt.value().GetDimension().Unit() != DimensionUnit::CALC && opt.value().GetDimension().IsNegative()) {
        opt.reset();
    }
}
void ValidateNonPercent(std::optional<CalcLength>& opt)
{
    if (opt.has_value() && (opt.value().GetDimension().Unit() == DimensionUnit::PERCENT)) {
        opt.reset();
    }
}
} // namespace OHOS::Ace::NG::Validator
} // namespace OHOS::Ace::NG