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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SAFE_AREA_INSET_SAFE_AREA_INSET_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SAFE_AREA_INSET_SAFE_AREA_INSET_PATTERN_H

#include <array>

#include "core/components_ng/pattern/pattern.h"

namespace OHOS::Ace {
class JsonValue;
}

namespace OHOS::Ace::NG {

inline constexpr char SAFE_AREA_INSET_CONTAINER_TAG[] = "SafeAreaInsetContainer";

enum class SafeAreaInsetEdge : int32_t;

// Builds the retained per-edge configuration shared by Inspector (owner side)
// and DFX dumps (carrier side); returns null when nothing is retained.
std::unique_ptr<JsonValue> BuildSafeAreaInsetConfig(const FrameNode* owner);

// Builds the safeAreaInset dump info for a node: container/carrier references,
// retained configuration and settled contribution fields; null when not
// applicable. Both dump channels route through this single implementation.
std::unique_ptr<JsonValue> BuildSafeAreaInsetDumpInfo(const FrameNode* node);

class ACE_EXPORT SafeAreaInsetPattern final : public Pattern {
    DECLARE_ACE_TYPE(SafeAreaInsetPattern, Pattern);

public:
    SafeAreaInsetPattern() = default;
    ~SafeAreaInsetPattern() override = default;

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override;

    bool IsAtomicNode() const override
    {
        return false;
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::SCOPE, true };
    }

    bool SetEdgeContent(SafeAreaInsetEdge edge, const RefPtr<UINode>& content);

    RefPtr<UINode> GetEdgeContent(SafeAreaInsetEdge edge) const;

    int32_t GetContentFrameIndex(SafeAreaInsetEdge edge) const;

    static size_t GetEdgeCount();

    bool Empty() const;

private:
    int32_t GetEdgeMountSlot(SafeAreaInsetEdge edge) const;

    // Element count is the SafeAreaInset edge count (TOP, END, BOTTOM, START);
    // kept in sync with SAFE_AREA_INSET_EDGE_COUNT by a static_assert in the cpp.
    std::array<RefPtr<UINode>, 4> contents_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SAFE_AREA_INSET_SAFE_AREA_INSET_PATTERN_H
