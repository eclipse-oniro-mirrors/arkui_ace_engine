/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_TEST_UNITTEST_CORE_PATTERN_TABS_TABS_TEST_NG_H
#define FOUNDATION_ACE_TEST_UNITTEST_CORE_PATTERN_TABS_TABS_TEST_NG_H

#include <cstddef>
#include <utility>

#include "gtest/gtest.h"

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/geometry/ng/size_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"

#define private public
#define protected public
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/core/render/mock_render_context.h"
#include "test/mock/core/rosen/mock_canvas.h"
#include "test/unittest/core/pattern/test_ng.h"

#include "base/memory/ace_type.h"
#include "core/components/tab_bar/tab_theme.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/layout/layout_wrapper_builder.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/pattern/divider/divider_pattern.h"
#include "core/components_ng/pattern/divider/divider_render_property.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/column_model_ng.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "core/components_ng/pattern/scrollable/scrollable.h"
#include "core/components_ng/pattern/stage/page_pattern.h"
#include "core/components_ng/pattern/swiper/swiper_event_hub.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"
#include "core/components_ng/pattern/tabs/tab_content_model_ng.h"
#include "core/components_ng/pattern/tabs/tab_content_pattern.h"
#include "core/components_ng/pattern/tabs/tab_content_transition_proxy.h"
#include "core/components_ng/pattern/tabs/tabs_model_ng.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_model_ng.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
using namespace testing;
using namespace testing::ext;
constexpr float TABS_WIDTH = 720.f;
constexpr float TABS_HEIGHT = 400.f;
constexpr int32_t TABCONTENT_NUMBER = 4;
constexpr float FIRST_ITEM_WIDTH = 800.0f;
constexpr float FIRST_ITEM_HEIGHT = 800.0f;
const SizeF FIRST_ITEM_SIZE(FIRST_ITEM_WIDTH, FIRST_ITEM_HEIGHT);
constexpr float INFINITY_NUM = 1000000.0f;
const SizeF INFINITY_SIZE(INFINITY_NUM, INFINITY_NUM);
constexpr float NEGTIVE_NUM = -100.0f;
const SizeF NEGTIVE_SIZE(NEGTIVE_NUM, NEGTIVE_NUM);
constexpr float TABBAR_WIDTH = 50.0f;
constexpr float SWIPER_WIDTH = 750.0f;
const double DEFAULT_OFFSET = -1.0f;
const int DEFAULT_ITEMCOUNT = 1;
const int DEFAULT_INDEX = -1;
const int BEGIN_INDEX = 0;
const int CURRENT_INDEX = 1;
const int END_INDEX = 0;
const OffsetF CURRENT_OFFSET(1.0f, 1.0f);
constexpr float TEST_MASK_A_RADIUS_RATIO = 0.0f;
constexpr float TEST_MASK_B_RADIUS_RATIO = 1.414f;
constexpr float TEST_MASK_MIDDLE_RADIUS_RATIO = (TEST_MASK_A_RADIUS_RATIO + TEST_MASK_B_RADIUS_RATIO) / 2.0f;
constexpr int32_t TEST_SWIPER_INDEX = 0;
constexpr int32_t TEST_DIVIDER_INDEX = 1;
constexpr int32_t TEST_TAB_BAR_INDEX = 2;
constexpr int32_t TEST_SELECTED_MASK_COUNT = 2;
constexpr int32_t TEST_UNSELECTED_MASK_COUNT = 1;
constexpr int32_t LG_COLUMN_NUM = 12;
constexpr int32_t XS_COLUMN_NUM = 2;
constexpr int32_t PLATFORM_VERSION_10 = 10;
constexpr int32_t PLATFORM_VERSION_11 = 11;
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t TABBAR_DEFAULT_WIDTH = 56.f;
constexpr int32_t TABBAR_DEFAULT_HEIGHT = 56.f;

class TabsTestNg : public TestNG {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp() override;
    void TearDown() override;
    void GetInstance();
    void Create(const std::function<void(TabsModelNG)>& callback,
        BarPosition barPosition = BarPosition::START, int32_t index = 0);
    void CreateWithItem(const std::function<void(TabsModelNG)>& callback,
        BarPosition barPosition = BarPosition::START, int32_t index = 0);

    static void CreateItem(
        int32_t itemNumber, const std::function<void(TabContentModelNG, int32_t)>& callback = nullptr);
    static void CreateSingleItem(const std::function<void(TabContentModelNG)>& callback, int32_t nodeId);
    static void CreateSingleItemWithoutBuilder(
        const std::function<void(TabContentModelNG)>& callback, int32_t nodeId);
    static TabBarBuilderFunc TabBarItemBuilder();
    void SwipeTo(int32_t index);

    RefPtr<TabsNode> frameNode_;
    RefPtr<TabsPattern> pattern_;
    RefPtr<TabsLayoutProperty> layoutProperty_;

    RefPtr<FrameNode> swiperNode_;
    RefPtr<SwiperPattern> swiperPattern_;
    RefPtr<SwiperLayoutProperty> swiperLayoutProperty_;
    RefPtr<SwiperPaintProperty> swiperPaintProperty_;
    RefPtr<SwiperController> swiperController_;

    RefPtr<FrameNode> tabBarNode_;
    RefPtr<TabBarPattern> tabBarPattern_;
    RefPtr<TabBarLayoutProperty> tabBarLayoutProperty_;
    RefPtr<TabBarPaintProperty> tabBarPaintProperty_;
    RefPtr<TabBarAccessibilityProperty> tabBarAccessibilityProperty_;

    RefPtr<FrameNode> dividerNode_;
    RefPtr<DividerRenderProperty> dividerRenderProperty_;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_TEST_UNITTEST_CORE_PATTERN_TABS_TABS_TEST_NG_H