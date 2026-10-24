/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LAZY_LAYOUT_LAZY_LAYOUT_OFFSET_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LAZY_LAYOUT_LAZY_LAYOUT_OFFSET_H

#include <memory>

namespace OHOS::Ace::NG {
struct PreparedLayoutGeometry;
struct AdjustOffset {
    float start = 0.0f; // Positive: Top edge adjust up, Negative: Top edge adjust down.
    float end = 0.0f;   // Positive: Bottom edge adjust down, Negative: Bottom edge adjust up.
};

struct LazyPremeasureResult {
    // The requested boundary range is measured, or every item was exhausted without a measurement failure.
    bool complete = false;
    // True only when the opposite edge and all traversed nested sections were fully measured.
    bool reachedOppositeEdge = false;
    float marginMainSize = 0.0f;
    // Relative to the previously displayed anchor, not the synthetic measurement window.
    AdjustOffset adjust;
    // Terminal failure after measurement began; size and adjustment still describe committed partial results.
    bool unsupported = false;
    // Insets are relative to published geometry; the remaining start adjustment is incremental.
    float startInsetAdjustment = 0.0f;
    std::shared_ptr<PreparedLayoutGeometry> geometry;
};
}

#endif
