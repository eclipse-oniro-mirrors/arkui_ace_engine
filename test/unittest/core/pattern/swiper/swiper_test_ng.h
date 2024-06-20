/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_TEST_UNITTEST_CORE_PATTERN_SWIPER_SWIPER_TEST_NG_H
#define FOUNDATION_ACE_TEST_UNITTEST_CORE_PATTERN_SWIPER_SWIPER_TEST_NG_H

#include <cstddef>

#include "gtest/gtest.h"

#define private public
#define protected public
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/animation/animator.h"
#include "core/animation/spring_curve.h"
#include "core/components/common/layout/constants.h"
#include "core/components/button/button_theme.h"
#include "core/components/swiper/swiper_component.h"
#include "core/components/swiper/swiper_indicator_theme.h"
#include "core/components/theme/resource_adapter.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/touch_event.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/button/button_model_ng.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/pattern/swiper/swiper_event_hub.h"
#include "core/components_ng/pattern/swiper/swiper_model_ng.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/swiper/arc_swiper_pattern.h"
#include "core/components_ng/pattern/swiper_indicator/circle_dot_indicator/circle_dot_indicator_layout_algorithm.h"
#include "core/components_ng/pattern/swiper_indicator/circle_dot_indicator/circle_dot_indicator_paint_method.h"
#include "core/components_ng/pattern/swiper_indicator/circle_dot_indicator/circle_dot_indicator_paint_property.h"
#include "core/components_ng/pattern/swiper_indicator/dot_indicator/dot_indicator_layout_algorithm.h"
#include "core/components_ng/pattern/swiper_indicator/dot_indicator/dot_indicator_paint_method.h"
#include "core/components_ng/pattern/swiper_indicator/dot_indicator/dot_indicator_paint_property.h"
#include "core/components_ng/pattern/swiper_indicator/indicator_common/swiper_arrow_pattern.h"
#include "core/components_ng/pattern/swiper_indicator/indicator_common/swiper_indicator_layout_property.h"
#include "core/components_ng/pattern/swiper_indicator/indicator_common/swiper_indicator_pattern.h"
#include "core/components_ng/pattern/swiper/swiper_utils.h"
#include "core/components_ng/pattern/text/text_model_ng.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "test/mock/core/pattern/mock_nestable_scroll_container.h"
#include "test/mock/core/render/mock_render_context.h"
#include "test/mock/core/rosen/mock_canvas.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/unittest/core/pattern/test_ng.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/base/mock_task_executor.h"
#include "test/mock/core/common/mock_container.h"

namespace OHOS::Ace::NG {
using namespace testing;
using namespace testing::ext;
constexpr float SWIPER_WIDTH = 480.f;
constexpr float SWIPER_HEIGHT = 800.f;
constexpr int32_t ITEM_NUMBER = 4;
constexpr int32_t DEFAULT_INTERVAL = 3000;
constexpr int32_t DEFAULT_DURATION = 400;
constexpr float CAPTURE_MARGIN_SIZE = 15.0f;
const SwiperArrowParameters ARROW_PARAMETERS = {
    true, true, Dimension(24.f), Color::BLACK, Dimension(18.f), Color::FromString("#182431")
};
const Color HOVER_ARROW_COLOR = Color::GRAY;
const Color CLICK_ARROW_COLOR = Color::FromString("#19182431");
constexpr double ARROW_DISABLED_ALPHA = 0.5;
const Color INDICATOR_TEXT_FONT_COLOR = Color::FromString("#ff182431");
const Dimension INDICATOR_TEXT_FONT_SIZE = Dimension(14.f);
const FontWeight INDICATOR_TEXT_FONT_WEIGHT = FontWeight::W800;

class SwiperTestNg : public TestNG {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp() override;
    void TearDown() override;
    void GetInstance();

    void Create(const std::function<void(SwiperModelNG)>& callback = nullptr);
    void CreateWithItem(const std::function<void(SwiperModelNG)>& callback = nullptr, int32_t itemNumber = ITEM_NUMBER);
    static void CreateItem(int32_t itemNumber = ITEM_NUMBER);
    static void CreateItemWithSize(float width, float height);
    void ShowNext();
    void ShowPrevious();
    void ChangeIndex(int32_t index);

    void InitCaptureTest();
    void CreateWithCustomAnimation();

    RefPtr<FrameNode> frameNode_;
    RefPtr<SwiperPattern> pattern_;
    RefPtr<SwiperEventHub> eventHub_;
    RefPtr<SwiperLayoutProperty> layoutProperty_;
    RefPtr<SwiperPaintProperty> paintProperty_;
    RefPtr<SwiperAccessibilityProperty> accessibilityProperty_;
    RefPtr<SwiperController> controller_;

    RefPtr<FrameNode> indicatorNode_;
    RefPtr<FrameNode> leftArrowNode_;
    RefPtr<FrameNode> rightArrowNode_;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_TEST_UNITTEST_CORE_PATTERN_SWIPER_SWIPER_TEST_NG_H