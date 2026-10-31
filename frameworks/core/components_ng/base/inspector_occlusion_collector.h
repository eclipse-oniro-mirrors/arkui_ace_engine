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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_OCCLUSION_COLLECTOR_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_OCCLUSION_COLLECTOR_H

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "base/geometry/ng/rect_t.h"
#include "base/memory/ace_type.h"

namespace OHOS::Ace::NG {

class FrameNode;

enum class OcclusionDecision {
    UNKNOWN,
    VISIBLE,
    OCCLUDED,
};

// A request-local value copy. The vector containing these entries is ordered
// from the bottom paint operation to the top paint operation in each domain.
struct OcclusionPaintSnapshot {
    int32_t nodeId = -1;
    int32_t domainId = -1;
    RectF candidateEnvelope;
    RectF opaqueEvidence;
    double finalOpacity = 0.0;
    bool candidateTrusted = false;
    bool evidenceTrusted = false;
    bool paintOrderTrusted = false;
};

class InspectorOcclusionCollector final {
public:
    using Decisions = std::unordered_map<int32_t, OcclusionDecision>;

    // Pure numeric evaluator, also used by tests to check the safety rules.
    static Decisions Evaluate(const std::vector<OcclusionPaintSnapshot>& paintOrder, double threshold);

    // Must run on the UI thread. Never rebuilds the render tree or retains UI pointers.
    static Decisions Collect(const RefPtr<FrameNode>& startNode, double threshold, int32_t domainId);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_OCCLUSION_COLLECTOR_H
