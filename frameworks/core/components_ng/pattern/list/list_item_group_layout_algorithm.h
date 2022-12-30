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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_ITEM_GROUP_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_ITEM_GROUP_LAYOUT_ALGORITHM_H

#include <optional>
#include "base/geometry/axis.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/list/list_layout_property.h"
#include "core/components_v2/list/list_properties.h"

namespace OHOS::Ace::NG {

// TextLayoutAlgorithm acts as the underlying text layout.
class ACE_EXPORT ListItemGroupLayoutAlgorithm : public LayoutAlgorithm {
    DECLARE_ACE_TYPE(ListItemGroupLayoutAlgorithm, LayoutAlgorithm);
public:
    using PositionMap = std::map<int32_t, std::pair<float, float>>;

    ListItemGroupLayoutAlgorithm(int32_t headerIndex, int32_t footerIndex, int32_t itemStartIndex)
        :headerIndex_(headerIndex), footerIndex_(footerIndex), itemStartIndex_(itemStartIndex) {}

    void Measure(LayoutWrapper* layoutWrapper) override;

    void Layout(LayoutWrapper* layoutWrapper) override;

    const PositionMap& GetItemPosition() const
    {
        return itemPosition_;
    }

    float GetSpaceWidth() const
    {
        return spaceWidth_;
    }

    Axis GetAxis() const
    {
        return axis_;
    }

    int32_t GetLanes() const
    {
        return lanes_;
    }

    void SetListMainSize(float contentMainSize)
    {
        listMainSize_ = contentMainSize;
    }

    void SetListLayoutProperty(RefPtr<ListLayoutProperty> layoutProperty)
    {
        listLayoutProperty_ = std::move(layoutProperty);
    }

private:
    float CalculateLaneCrossOffset(float crossSize, float childCrossSize);
    void UpdateListItemConstraint(const OptionalSizeF& selfIdealSize, LayoutConstraintF& contentConstraint);
    int32_t MeasureALine(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint,
        int32_t currentIndex, float& mainLen);
    float MeasureListItem(LayoutWrapper* layoutWrapper, const LayoutConstraintF& layoutConstraint, float startPos);
    void LayoutListItem(LayoutWrapper* layoutWrapper, const OffsetF& paddingOffset, float crossSize);
    void LayoutHeaderFooter(LayoutWrapper* layoutWrapper, const OffsetF& paddingOffset, float crossSize);
    void LayoutIndex(const RefPtr<LayoutWrapper>& wrapper, const OffsetF& paddingOffset,
        float crossSize, float startPos);
    inline RefPtr<LayoutWrapper> GetListItem(LayoutWrapper* layoutWrapper, int32_t index) const
    {
        return layoutWrapper->GetOrCreateChildByIndex(index + itemStartIndex_);
    }
    void CalculateLanes(const RefPtr<ListLayoutProperty>& layoutProperty,
        const LayoutConstraintF& layoutConstraint, std::optional<float> crossSizeOptional, Axis axis);

    int32_t headerIndex_;
    int32_t footerIndex_;
    int32_t itemStartIndex_;
    RefPtr<ListLayoutProperty> listLayoutProperty_;
    float listMainSize_ = 0.0f;

    PositionMap itemPosition_;
    Axis axis_ = Axis::VERTICAL;
    int32_t lanes_ = 1;
    std::optional<float> minLaneLength_;
    std::optional<float> maxLaneLength_;
    V2::ListItemAlign itemAlign_ = V2::ListItemAlign::START;
    float spaceWidth_ = 0.0f;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_LAYOUT_ALGORITHM_H