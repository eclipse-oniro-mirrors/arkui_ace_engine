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

#include "core/components_ng/pattern/patternlock/patternlock_layout_algorithm.h"
#include "core/components_ng/pattern/patternlock/patternlock_layout_property.h"

namespace OHOS::Ace::NG {

std::optional<SizeF> PatternLockLayoutAlgorithm::MeasureContent(
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    auto patternLockLayoutProperty = DynamicCast<PatternLockLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_RETURN(patternLockLayoutProperty, std::nullopt);
    sideLength_ = patternLockLayoutProperty->GetSideLength().value_or(sideLength_);
    auto length = static_cast<float>(sideLength_.ConvertToPx());
    float maxWidth = std::max(contentConstraint.maxSize.Width(), contentConstraint.minSize.Width());
    float maxHeight = std::max(contentConstraint.maxSize.Height(), contentConstraint.minSize.Height());
    float maxLength = std::min(maxWidth, maxHeight);
    length = std::min(maxLength, length);
    return SizeF(length, length);
}

} // namespace OHOS::Ace::NG
