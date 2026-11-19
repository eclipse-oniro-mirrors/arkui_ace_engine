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

#ifndef RENDER_SERVICE_CLIENT_CORE_ANIMATION_RS_PARTICLE_PARAMS_H
#define RENDER_SERVICE_CLIENT_CORE_ANIMATION_RS_PARTICLE_PARAMS_H

#include <algorithm>
#include <cstdint>
#include <memory>

#include "common/rs_mock_enums.h"
#include "common/rs_vector.h"

namespace OHOS::Rosen {
class RSColor;
class RSImage;
class RSShader;
class RSTransitionEffect;
class RectF {
public:
    RectF() = default;
    bool operator==(const RectF& o) const
    {
        return left_ == o.left_ && top_ == o.top_ && width_ == o.width_ && height_ == o.height_;
    }
    RectF JoinRect(const RectF& other) const
    {
        const auto left = std::min(left_, other.left_);
        const auto top = std::min(top_, other.top_);
        const auto right = std::max(left_ + width_, other.left_ + other.width_);
        const auto bottom = std::max(top_ + height_, other.top_ + other.height_);
        return RectF(left, top, right - left, bottom - top);
    }
    RectF(float left, float top, float width, float height) : left_(left), top_(top), width_(width), height_(height) {}

    float GetLeft() const
    {
        return left_;
    }

    float GetTop() const
    {
        return top_;
    }

    float GetWidth() const
    {
        return width_;
    }

    float GetHeight() const
    {
        return height_;
    }

private:
    float left_ = 0.0f;
    float top_ = 0.0f;
    float width_ = 0.0f;
    float height_ = 0.0f;
};

template<typename T>
class Range {
public:
    Range() = default;
    Range(const T& start, const T& end) : start_(start), end_(end) {}
    T start_ {};
    T end_ {};
};
template<typename T>
class Change {
public:
    template<typename... Args>
    explicit Change(Args&&...)
    {}
};
enum class ParticleUpdator { NONE, RANDOM, CURVE };
enum class DistributionType { UNIFORM, GAUSSIAN };
enum class ParticleType { POINTS, IMAGES };

template<typename T>
class ParticleParaType {
public:
    Range<T> val_;
    ParticleUpdator updator_ = ParticleUpdator::NONE;
    ParticleParaType() = default;
    template<typename... Rest>
    ParticleParaType(const Range<T>& value, ParticleUpdator updater, Rest&&...)
        : val_(value), updator_(updater) {}
};

class ParticleAcceleration {
public:
    ParticleParaType<float> accelerationValue_;
    ParticleParaType<float> accelerationAngle_;
    ParticleAcceleration() = default;
    ParticleAcceleration(const ParticleParaType<float>& value, const ParticleParaType<float>& angle)
        : accelerationValue_(value), accelerationAngle_(angle) {}
};
class ParticleColorParaType {
public:
    ParticleUpdator updator_ = ParticleUpdator::NONE;
    ParticleColorParaType() = default;
    template<typename RangeT, typename... Rest>
    ParticleColorParaType(RangeT&&, DistributionType, ParticleUpdator updater, Rest&&...) : updator_(updater) {}
};
class EmitterConfig {
public:
    template<typename... Args>
    void SetConfigShape(Args&&...) {}
    EmitterConfig() = default;
    template<typename... Args>
    explicit EmitterConfig(Args&&...) {}
};
class ParticleVelocity {
public:
    Range<float> velocityValue_;
    Range<float> velocityAngle_;
    ParticleVelocity() = default;
    ParticleVelocity(const Range<float>& value, const Range<float>& angle)
        : velocityValue_(value), velocityAngle_(angle) {}
};
class ParticleParams {
public:
    ParticleVelocity velocity_;
    ParticleAcceleration acceleration_;
    ParticleColorParaType color_;
    ParticleParaType<float> opacity_;
    ParticleParaType<float> scale_;
    ParticleParaType<float> spin_;
    ParticleParams() = default;
    ParticleParams(const EmitterConfig&, const ParticleVelocity& velocity, const ParticleAcceleration& acceleration,
        const ParticleColorParaType& color, const ParticleParaType<float>& opacity, const ParticleParaType<float>& scale,
        const ParticleParaType<float>& spin)
        : velocity_(velocity), acceleration_(acceleration), color_(color), opacity_(opacity), scale_(scale), spin_(spin) {}
};

} // namespace OHOS::Rosen

#endif // RENDER_SERVICE_CLIENT_CORE_ANIMATION_RS_PARTICLE_PARAMS_H
