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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_NAVIGATION_NAVIGATION_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_NAVIGATION_NAVIGATION_LAYOUT_ALGORITHM_H

#include <cstdint>

#include "base/memory/referenced.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/navigation/navigation_declaration.h"
#include "core/components_ng/pattern/navigation/navigation_group_node.h"
#include "core/components_ng/pattern/navigation/navigation_layout_property.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT NavigationLayoutAlgorithm : public LayoutAlgorithm {
    DECLARE_ACE_TYPE(NavigationLayoutAlgorithm, LayoutAlgorithm);

public:
    NavigationLayoutAlgorithm() = default;
    ~NavigationLayoutAlgorithm() override = default;
    void Measure(LayoutWrapper* layoutWrapper) override;
    void Layout(LayoutWrapper* layoutWrapper) override;

    NavigationMode GetNavigationMode() const
    {
        return navigationMode_;
    }

    void SetNavigationMode(NavigationMode navigationMode)
    {
        navigationMode_ = navigationMode;
    }

    void SetRealNavBarWidth(float realNavBarWidth)
    {
        realNavBarWidth_ = realNavBarWidth;
    }

    float GetRealNavBarWidth()
    {
        return realNavBarWidth_;
    }

    void SetIfNeedInit(bool ifNeedInit)
    {
        ifNeedInit_ = ifNeedInit;
    }

    float GetRealNavBarHeight() const
    {
        return realNavBarHeight_;
    }

    float GetRealDividerWidth() const
    {
        return realDividerWidth_;
    }

    OffsetF GetNavBarOffset() const
    {
        return navBarOffset_;
    }

private:
    NavigationMode usrNavigationMode_ = NavigationMode::AUTO;
    NavigationMode navigationMode_ = NavigationMode::AUTO;
    ACE_DISALLOW_COPY_AND_MOVE(NavigationLayoutAlgorithm);
    void MeasureNavBar(LayoutWrapper* layoutWrapper, const RefPtr<NavigationGroupNode>& hostNode,
        const RefPtr<NavigationLayoutProperty>& navigationLayoutProperty, const SizeF& navBarSize);
    float realNavBarWidth_ = 0.0f;
    float realDividerWidth_ = 0.0f;
    float realNavBarHeight_ = 0.0f;
    bool ifNeedInit_ = true;
    OffsetF navBarOffset_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_NAVIGATION_NAVIGATION_LAYOUT_ALGORITHM_H
