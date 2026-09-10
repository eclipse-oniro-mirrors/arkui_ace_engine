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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PROPERTY_SAFE_AREA_INSET_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PROPERTY_SAFE_AREA_INSET_PROPERTY_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

#include "core/components_ng/property/calc_length.h"

namespace OHOS::Ace::NG {

enum class SafeAreaInsetEdge : int32_t {
    TOP = 0,
    END,
    BOTTOM,
    START,
};

inline constexpr size_t SAFE_AREA_INSET_EDGE_COUNT = 4;

enum class SafeAreaInsetAlignment : int32_t {
    START = 0,
    CENTER,
    END,
};

enum class SafeAreaInsetPhysicalEdge { TOP, RIGHT, BOTTOM, LEFT };

struct SafeAreaInsetResolvedEdge {
    SafeAreaInsetPhysicalEdge edge = SafeAreaInsetPhysicalEdge::TOP;
    SafeAreaInsetAlignment alignment = SafeAreaInsetAlignment::CENTER;
};

struct SafeAreaInsetEdgeOptions {
    CalcLength spacing;
    SafeAreaInsetAlignment alignment = SafeAreaInsetAlignment::CENTER;

    bool operator==(const SafeAreaInsetEdgeOptions& other) const
    {
        return spacing == other.spacing && alignment == other.alignment;
    }

    bool operator!=(const SafeAreaInsetEdgeOptions& other) const
    {
        return !(*this == other);
    }
};

struct SafeAreaInsetOptions {
    using EdgeOptionsArray =
        std::array<std::optional<SafeAreaInsetEdgeOptions>, SAFE_AREA_INSET_EDGE_COUNT>;

    EdgeOptionsArray edges;

    static constexpr size_t ToIndex(SafeAreaInsetEdge edge)
    {
        return static_cast<size_t>(edge);
    }

    const std::optional<SafeAreaInsetEdgeOptions>& GetEdge(SafeAreaInsetEdge edge) const
    {
        return edges[ToIndex(edge)];
    }

    std::optional<SafeAreaInsetEdgeOptions>& GetEdge(SafeAreaInsetEdge edge)
    {
        return edges[ToIndex(edge)];
    }

    bool Empty() const
    {
        for (const auto& edge : edges) {
            if (edge.has_value()) {
                return false;
            }
        }
        return true;
    }

    bool operator==(const SafeAreaInsetOptions& other) const
    {
        return edges == other.edges;
    }

    bool operator!=(const SafeAreaInsetOptions& other) const
    {
        return !(*this == other);
    }
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PROPERTY_SAFE_AREA_INSET_PROPERTY_H
