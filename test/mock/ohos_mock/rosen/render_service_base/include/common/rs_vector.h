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
#ifndef FOUNDATION_ACE_TEST_MOCK_OHOS_MOCK_RENDER_SERVICE_BASE_INCLUDE_COMMON_RS_VECTOR_H
#define FOUNDATION_ACE_TEST_MOCK_OHOS_MOCK_RENDER_SERVICE_BASE_INCLUDE_COMMON_RS_VECTOR_H
#include <algorithm>
#include <array>
#include <cmath>

#include "core/components_ng/render/drawing.h"
#include "utils/rect.h"
namespace OHOS::Rosen {
template<typename T>
class Vector2 {
public:
    T x_ {};
    T y_ {};
    Vector2() = default;
    Vector2(T x, T y) : x_(x), y_(y) {}
    explicit Vector2(T v) : x_(v), y_(v) {}
    T& operator[](size_t i)
    {
        return i == 0 ? x_ : y_;
    }
    T operator[](size_t i) const
    {
        return i == 0 ? x_ : y_;
    }
    bool operator==(const Vector2& o) const
    {
        return x_ == o.x_ && y_ == o.y_;
    }
    bool operator!=(const Vector2& o) const
    {
        return !(*this == o);
    }
};
using Vector2f = Vector2<float>;
template<typename T>
class Vector3 {
public:
    T x_ {};
    T y_ {};
    T z_ {};
    Vector3() = default;
    Vector3(T x, T y, T z) : x_(x), y_(y), z_(z) {}
    T& operator[](size_t i)
    {
        return i == 0 ? x_ : (i == 1 ? y_ : z_);
    }
    T operator[](size_t i) const
    {
        return i == 0 ? x_ : (i == 1 ? y_ : z_);
    }
    bool operator==(const Vector3& other) const
    {
        return x_ == other.x_ && y_ == other.y_ && z_ == other.z_;
    }
    bool operator!=(const Vector3& other) const
    {
        return !(*this == other);
    }
};
using Vector3f = Vector3<float>;
template<typename T>
class Vector4 {
public:
    T x_ {};
    T y_ {};
    T z_ {};
    T w_ {};
    void SetValues(T x, T y, T z, T w)
    {
        x_ = x;
        y_ = y;
        z_ = z;
        w_ = w;
    }
    Vector4() = default;
    explicit Vector4(T v) : x_(v), y_(v), z_(v), w_(v) {}
    Vector4(T x, T y, T z, T w) : x_(x), y_(y), z_(z), w_(w) {}
    T& operator[](size_t i)
    {
        return i == 0 ? x_ : (i == 1 ? y_ : (i == 2 ? z_ : w_));
    }
    T operator[](size_t i) const
    {
        return i == 0 ? x_ : (i == 1 ? y_ : (i == 2 ? z_ : w_));
    }
    bool operator==(const Vector4& o) const
    {
        return x_ == o.x_ && y_ == o.y_ && z_ == o.z_ && w_ == o.w_;
    }
    bool operator!=(const Vector4& o) const
    {
        return !(*this == o);
    }
};
using Vector4f = Vector4<float>;
} // namespace OHOS::Rosen

namespace OHOS::Ace {
using RSSize = Size;
}

namespace OHOS::Rosen {
class Quaternion : public Vector4f {
public:
    using Vector4f::Vector4f;
    bool IsIdentity() const
    {
        return x_ == 0 && y_ == 0 && z_ == 0 && std::abs(w_ - 1.0f) < 1e-6f;
    }
};
} // namespace OHOS::Rosen

#endif // FOUNDATION_ACE_TEST_MOCK_OHOS_MOCK_RENDER_SERVICE_BASE_INCLUDE_COMMON_RS_VECTOR_H
