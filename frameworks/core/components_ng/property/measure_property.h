/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PROPERTIES_MEASURE_PROPERTIES_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PROPERTIES_MEASURE_PROPERTIES_H

#include <array>
#include <optional>

#include "core/components_ng/property/calc_length.h"

namespace OHOS::Ace::NG {

enum class MeasureType {
    MATCH_PARENT,
    WRAP_CONTENT,
};

class CalcSize {
public:
    CalcSize() = default;
    ~CalcSize() = default;
    CalcSize(const CalcLength& width, const CalcLength& height) : width_(width), height_(height) {}

    void Reset()
    {
        width_.Reset();
        height_.Reset();
    }

    const CalcLength& Width() const
    {
        return width_;
    }

    const CalcLength& Height() const
    {
        return height_;
    }

    void SetWidth(const CalcLength& width)
    {
        width_ = width;
    }

    void SetHeight(const CalcLength& height)
    {
        height_ = height;
    }

    void SetSizeT(const CalcSize& Size)
    {
        width_ = Size.Width();
        height_ = Size.Height();
    }

    bool operator==(const CalcSize& Size) const
    {
        return (width_ == Size.width_) && (height_ == Size.height_);
    }

    bool operator!=(const CalcSize& Size) const
    {
        return !operator==(Size);
    }

    bool UpdateSizeWithCheck(const CalcSize& Size)
    {
        if ((width_ == Size.width_) && ((height_ == Size.height_))) {
            return false;
        }
        if (Size.width_.IsValid()) {
            width_ = Size.width_;
        }
        if (Size.height_.IsValid()) {
            height_ = Size.height_;
        }
        return true;
    }

private:
    CalcLength width_ { -1 };
    CalcLength height_ { -1 };
};

struct MeasureProperty {
    std::optional<CalcSize> maxSize;
    std::optional<CalcSize> minSize;
    std::optional<CalcSize> selfIdealSize;

    void Reset()
    {
        minSize.reset();
        maxSize.reset();
        selfIdealSize.reset();
    }

    bool operator==(const MeasureProperty& measureProperty) const
    {
        return (minSize == measureProperty.minSize) && (maxSize == measureProperty.maxSize) &&
               (selfIdealSize == measureProperty.selfIdealSize);
    }

    bool UpdateSelfIdealSizeWithCheck(const CalcSize& size)
    {
        if (selfIdealSize == size) {
            return false;
        }
        if (selfIdealSize.has_value()) {
            return selfIdealSize->UpdateSizeWithCheck(size);
        }
        selfIdealSize = size;
        return true;
    }

    bool UpdateMaxSizeWithCheck(const CalcSize& size)
    {
        if (maxSize == size) {
            return false;
        }
        if (maxSize.has_value()) {
            return maxSize->UpdateSizeWithCheck(size);
        }
        maxSize = size;
        return true;
    }

    bool UpdateMinSizeWithCheck(const CalcSize& size)
    {
        if (minSize == size) {
            return false;
        }
        if (minSize.has_value()) {
            return minSize->UpdateSizeWithCheck(size);
        }
        minSize = size;
        return true;
    }
};

template<typename T>
struct PaddingPropertyT {
    std::optional<T> left;
    std::optional<T> right;
    std::optional<T> top;
    std::optional<T> bottom;

    void SetEdges(const T& padding)
    {
        left = padding;
        right = padding;
        top = padding;
        bottom = padding;
    }

    bool operator==(const PaddingPropertyT& value) const
    {
        return (left == value.left) && (right == value.right) && (top == value.top) && (bottom == value.bottom);
    }

    bool UpdateWithCheck(const PaddingPropertyT& value)
    {
        bool isModified = false;
        if (value.left.has_value() && (left != value.left)) {
            left = value.left;
            isModified = true;
        }
        if (value.right.has_value() && (right != value.right)) {
            right = value.right;
            isModified = true;
        }
        if (value.top.has_value() && (top != value.top)) {
            top = value.top;
            isModified = true;
        }
        if (value.bottom.has_value() && (bottom != value.bottom)) {
            bottom = value.bottom;
            isModified = true;
        }
        return isModified;
    }
};

using PaddingProperty = PaddingPropertyT<CalcLength>;
using MarginProperty = PaddingProperty;
using PaddingPropertyF = PaddingPropertyT<float>;
using MarginPropertyF = PaddingPropertyT<float>;

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PROPERTIES_MEASURE_PROPERTIES_H