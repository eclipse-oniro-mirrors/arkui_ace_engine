/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"

#include "base/geometry/offset.h"
#define private public
#define protected public
#include "test/mock/base/mock_task_executor.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/core/rosen/mock_canvas.h"
#include "test/unittest/core/pattern/test_ng.h"

#include "base/geometry/ng/point_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/memory/ace_type.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_abstract_model_ng.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/linear_layout/column_model_ng.h"
#include "core/components_ng/pattern/linear_layout/row_model_ng.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/scroll/effect/scroll_fade_effect.h"
#include "core/components_ng/pattern/scroll/scroll_model_ng.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_spring_effect.h"
#include "core/components_ng/pattern/scroll_bar/scroll_bar_layout_algorithm.h"
#include "core/components_ng/pattern/scroll_bar/scroll_bar_model_ng.h"
#include "core/components_ng/pattern/scroll_bar/scroll_bar_pattern.h"
#include "core/components_ng/pattern/scrollable/scrollable.h"
#include "core/components_ng/pattern/scrollable/scrollable_paint_property.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/stack/stack_model_ng.h"
#include "core/components_ng/pattern/stack/stack_layout_algorithm.h"
#include "core/components_ng/pattern/stack/stack_layout_property.h"
#include "core/components_v2/inspector/inspector_constants.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr float DEVICE_WIDTH = 720.0f;
constexpr float DEVICE_HEIGHT = 1136.0f;
constexpr float SCROLL_WIDTH = 480.f;
constexpr float SCROLL_HEIGHT = 800.f;
constexpr float CONTENT_WIDTH = 600.f;
constexpr float CONTENT_HEIGHT = 1000.f;
constexpr float SCROLL_BAR_CHILD_WIDTH = 20.f;
constexpr float SCROLL_BAR_CHILD_HEIGHT = 160.f;
constexpr float SCROLL_BAR_FLOAT_10 = 10.0f;
constexpr float SCROLL_BAR_FLOAT_100 = 100.0f;
constexpr float SCROLL_BAR_FLOAT_200 = 200.0f;
constexpr float SCROLL_BAR_FLOAT_NEGATIVE_100 = -100.0f;
const SizeF CONTAINER_SIZE(DEVICE_WIDTH, DEVICE_HEIGHT);
const SizeF SCROLL_BAR_CHILD_SIZE(SCROLL_BAR_CHILD_WIDTH, SCROLL_BAR_CHILD_HEIGHT);
const SizeF SCROLL_BAR_SELF_SIZE(30.0f, DEVICE_HEIGHT);
} // namespace

class ScrollBarTestNg : public TestNG {
protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp() override;
    void TearDown() override;
    void GetInstance();

    RefPtr<FrameNode> CreateMainFrameNode();
    void CreateStack(Alignment align = Alignment::CENTER_RIGHT);
    void CreateScroll(float mainSize, Axis axis = Axis::VERTICAL);
    void CreateContent(float mainSize, Axis axis = Axis::VERTICAL);
    void CreateScrollBar(Axis axis = Axis::VERTICAL);

    void CreateScrollBarWithoutChild(bool infoFlag, bool proxyFlag, int directionValue, int stateValue);
    void CreateScrollBar(
        bool infoFlag, bool proxyFlag, int directionValue, int stateValue, const LayoutConstraintF& layoutConstraint);
    void CreateScrollBarWithoutSubComponent(
        bool infoFlag, bool proxyFlag, int directionValue, int stateValue, const LayoutConstraintF& layoutConstraint);

    void HandleDragStart(GestureEvent info);
    void HandleDragUpdate(GestureEvent info);
    void HandleDragEnd(GestureEvent info);

    RefPtr<FrameNode> frameNode_;
    RefPtr<ScrollBarPattern> pattern_;
    RefPtr<ScrollBarLayoutProperty> layoutProperty_;
    RefPtr<ScrollBarAccessibilityProperty> accessibilityProperty_;
    RefPtr<LayoutAlgorithm> layoutAlgorithm_;
    RefPtr<LayoutWrapperNode> layoutWrapper_;
    RefPtr<ScrollablePaintProperty> paintProperty_;
    RefPtr<ScrollBar> scrollBar_;

    RefPtr<FrameNode> stackNode_;
    RefPtr<FrameNode> scrollNode_;
    RefPtr<ScrollPattern> scrollPattern_;
    RefPtr<FrameNode> scrollBarNode_;
    RefPtr<ScrollBarPattern> scrollBarPattern_;
};

void ScrollBarTestNg::SetUpTestSuite()
{
    TestNG::SetUpTestSuite();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto themeConstants = CreateThemeConstants(THEME_PATTERN_SCROLL_BAR);
    auto scrollBarTheme = ScrollBarTheme::Builder().Build(themeConstants);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(scrollBarTheme));
}

void ScrollBarTestNg::TearDownTestSuite()
{
    TestNG::TearDownTestSuite();
}

void ScrollBarTestNg::SetUp() {}

void ScrollBarTestNg::TearDown()
{
    frameNode_ = nullptr;
    pattern_ = nullptr;
    layoutProperty_ = nullptr;
    accessibilityProperty_ = nullptr;
    layoutAlgorithm_ = nullptr;
    layoutWrapper_ = nullptr;
    scrollBar_ = nullptr;
    AceApplicationInfo::GetInstance().isRightToLeft_ = false;
}

void ScrollBarTestNg::GetInstance()
{
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    frameNode_ = AceType::DynamicCast<FrameNode>(element);
    pattern_ = frameNode_->GetPattern<ScrollBarPattern>();
    layoutProperty_ = frameNode_->GetLayoutProperty<ScrollBarLayoutProperty>();
    paintProperty_ = frameNode_->GetPaintProperty<ScrollablePaintProperty>();
    accessibilityProperty_ = frameNode_->GetAccessibilityProperty<ScrollBarAccessibilityProperty>();
    layoutAlgorithm_ = pattern_->CreateLayoutAlgorithm();
}

RefPtr<FrameNode> ScrollBarTestNg::CreateMainFrameNode()
{
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->GetMainElementNode();
    return AceType::DynamicCast<FrameNode>(element);
}

void ScrollBarTestNg::CreateStack(Alignment align)
{
    StackModelNG stackModel;
    stackModel.Create();
    stackModel.SetAlignment(align);
    ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    ViewAbstract::SetHeight(CalcLength(SCROLL_HEIGHT));
    stackNode_ = CreateMainFrameNode();
}

void ScrollBarTestNg::CreateScroll(float mainSize, Axis axis)
{
    ScrollModelNG model;
    model.Create();
    auto scrollBarProxy = model.CreateScrollBarProxy();
    model.SetAxis(axis);
    model.SetDisplayMode(static_cast<int>(DisplayMode::OFF));
    model.SetScrollBarProxy(scrollBarProxy);
    ViewAbstract::SetWidth(CalcLength(FILL_LENGTH));
    ViewAbstract::SetHeight(CalcLength(FILL_LENGTH));
    scrollNode_ = CreateMainFrameNode();
    scrollPattern_ = scrollNode_->GetPattern<ScrollPattern>();
    CreateContent(mainSize, axis);
    ViewStackProcessor::GetInstance()->Pop();
}

void ScrollBarTestNg::CreateContent(float mainSize, Axis axis)
{
    if (axis == Axis::HORIZONTAL) {
        RowModelNG rowModel;
        rowModel.Create(Dimension(0), nullptr, "");
        ViewAbstract::SetWidth(CalcLength(mainSize));
        ViewAbstract::SetHeight(CalcLength(FILL_LENGTH));
    } else {
        ColumnModelNG colModel;
        colModel.Create(Dimension(0), nullptr, "");
        ViewAbstract::SetWidth(CalcLength(FILL_LENGTH));
        ViewAbstract::SetHeight(CalcLength(mainSize));
    }
    ViewStackProcessor::GetInstance()->Pop();
}

void ScrollBarTestNg::CreateScrollBar(Axis axis)
{
    auto scrollBarProxy = scrollPattern_->GetScrollBarProxy();
    ScrollBarModelNG scrollBarModel;
    scrollBarModel.Create(scrollBarProxy, true, true, static_cast<int>(axis), static_cast<int>(DisplayMode::ON));
    scrollBarNode_ = CreateMainFrameNode();
    scrollBarPattern_ = scrollBarNode_->GetPattern<ScrollBarPattern>();
    // Create ScrollBar Child
    ColumnModelNG colModel;
    colModel.Create(Dimension(0), nullptr, "");
    if (axis == Axis::HORIZONTAL) {
        ViewAbstract::SetWidth(CalcLength(SCROLL_BAR_CHILD_HEIGHT));
        ViewAbstract::SetHeight(CalcLength(SCROLL_BAR_CHILD_WIDTH));
    } else {
        ViewAbstract::SetWidth(CalcLength(SCROLL_BAR_CHILD_WIDTH));
        ViewAbstract::SetHeight(CalcLength(SCROLL_BAR_CHILD_HEIGHT));
    }
    ViewStackProcessor::GetInstance()->Pop();
    ViewStackProcessor::GetInstance()->Pop();
}

void ScrollBarTestNg::CreateScrollBarWithoutChild(bool infoFlag, bool proxyFlag, int directionValue, int stateValue)
{
    ScrollBarModelNG model;
    RefPtr<ScrollProxy> scrollProxy;
    auto proxy = model.GetScrollBarProxy(scrollProxy);
    model.Create(proxy, infoFlag, proxyFlag, directionValue, stateValue);
    GetInstance();
}

void ScrollBarTestNg::CreateScrollBar(
    bool infoFlag, bool proxyFlag, int directionValue, int stateValue, const LayoutConstraintF& layoutConstraint)
{
    ScrollBarModelNG model;
    RefPtr<ScrollProxy> scrollProxy;
    auto proxy = model.GetScrollBarProxy(scrollProxy);
    model.Create(proxy, infoFlag, proxyFlag, directionValue, stateValue);
    GetInstance();

    layoutProperty_->UpdateLayoutConstraint(layoutConstraint);
    layoutProperty_->UpdateContentConstraint();
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    layoutWrapper_ = AceType::MakeRefPtr<LayoutWrapperNode>(frameNode_, geometryNode, layoutProperty_);
    layoutWrapper_->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm_));

    auto childFrameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    ViewStackProcessor::GetInstance()->Push(childFrameNode);
    RefPtr<GeometryNode> childGeometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutProperty> childLayoutProperty = childFrameNode->GetLayoutProperty();
    RefPtr<LayoutWrapperNode> childLayoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(childFrameNode, childGeometryNode, childLayoutProperty);
    childLayoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(SCROLL_BAR_CHILD_WIDTH), CalcLength(SCROLL_BAR_CHILD_HEIGHT)));
    auto childLayoutAlgorithm = childFrameNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    childLayoutWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(childLayoutAlgorithm));
    layoutWrapper_->AppendChild(childLayoutWrapper);

    layoutAlgorithm_->Measure(AceType::RawPtr(layoutWrapper_));
    layoutAlgorithm_->Layout(AceType::RawPtr(layoutWrapper_));
}

void ScrollBarTestNg::CreateScrollBarWithoutSubComponent(
    bool infoFlag, bool proxyFlag, int directionValue, int stateValue, const LayoutConstraintF& layoutConstraint)
{
    ScrollBarModelNG model;
    RefPtr<ScrollProxy> scrollProxy;
    auto proxy = model.GetScrollBarProxy(scrollProxy);
    model.Create(proxy, infoFlag, proxyFlag, directionValue, stateValue);
    GetInstance();

    layoutProperty_->UpdateLayoutConstraint(layoutConstraint);
    layoutProperty_->UpdateContentConstraint();
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    layoutWrapper_ = AceType::MakeRefPtr<LayoutWrapperNode>(frameNode_, geometryNode, layoutProperty_);
    layoutWrapper_->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithm_));
    layoutAlgorithm_->Measure(AceType::RawPtr(layoutWrapper_));
}

void ScrollBarTestNg::HandleDragStart(GestureEvent info)
{
    auto handleDragStart = *(scrollBarPattern_->panRecognizer_->onActionStart_);
    handleDragStart(info);
}

void ScrollBarTestNg::HandleDragUpdate(GestureEvent info)
{
    auto handleDragUpdate = *(scrollBarPattern_->panRecognizer_->onActionUpdate_);
    handleDragUpdate(info);
}

void ScrollBarTestNg::HandleDragEnd(GestureEvent info)
{
    auto handleDragEnd = *(scrollBarPattern_->panRecognizer_->onActionEnd_);
    handleDragEnd(info);
}

/**
 * @tc.name: HandleDrag001
 * @tc.desc: Test ScrollBar about HandleDrag
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, HandleDrag001, TestSize.Level1)
{
    CreateStack();
    CreateScroll(CONTENT_HEIGHT);
    CreateScrollBar();
    CreateDone(stackNode_);
    EXPECT_TRUE(IsEqual(GetChildRect(stackNode_, 0), RectF(0, 0, SCROLL_WIDTH, SCROLL_HEIGHT)));
    EXPECT_TRUE(IsEqual(GetChildRect(stackNode_, 1), RectF(460.f, 0, SCROLL_BAR_CHILD_WIDTH, SCROLL_HEIGHT)));
    float controlDistance = scrollBarPattern_->GetControlDistance();
    float scrollableDistance = scrollBarPattern_->GetScrollableDistance();
    EXPECT_EQ(controlDistance, CONTENT_HEIGHT - SCROLL_HEIGHT); // 200.f
    EXPECT_EQ(scrollableDistance, SCROLL_HEIGHT - SCROLL_BAR_CHILD_HEIGHT); // 640.f

    /**
     * @tc.steps: step1. HandleDragStart, drag on scrollBar
     */
    GestureEvent info;
    info.SetGlobalPoint(Point(SCROLL_WIDTH - 1.f, 1.f));
    HandleDragStart(info);

    /**
     * @tc.steps: step2. HandleDragUpdate, drag down
     */
    float delta = SCROLL_BAR_CHILD_HEIGHT;
    info.SetMainDelta(delta);
    HandleDragUpdate(info);
    FlushLayoutTask(stackNode_);
    EXPECT_EQ(scrollBarPattern_->GetCurrentPosition(), delta);
    float expectOffset = delta * controlDistance / scrollableDistance;
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), expectOffset); // 50.f

    /**
     * @tc.steps: step3. HandleDragUpdate, drag down continue
     */
    HandleDragUpdate(info);
    FlushLayoutTask(stackNode_);
    EXPECT_EQ(scrollBarPattern_->GetCurrentPosition(), delta * 2);
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), expectOffset * 2); // 100.f

    /**
     * @tc.steps: step4. HandleDragUpdate, drag up
     */
    info.SetMainDelta(-delta);
    HandleDragUpdate(info);
    FlushLayoutTask(stackNode_);
    EXPECT_EQ(scrollBarPattern_->GetCurrentPosition(), delta);
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), expectOffset); // 50.f

    /**
     * @tc.steps: step5. HandleDragEnd, drag end
     */
    HandleDragEnd(info);
    FlushLayoutTask(stackNode_);
    EXPECT_EQ(scrollBarPattern_->GetCurrentPosition(), delta);
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), expectOffset);
}

/**
 * @tc.name: HandleDrag002
 * @tc.desc: Test ScrollBar about HandleDrag with InputEventType::AXIS
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, HandleDrag002, TestSize.Level1)
{
    CreateStack();
    CreateScroll(CONTENT_HEIGHT);
    CreateScrollBar();
    CreateDone(stackNode_);
    float controlDistance = scrollBarPattern_->GetControlDistance();
    float scrollableDistance = scrollBarPattern_->GetScrollableDistance();

    /**
     * @tc.steps: step1. HandleDragStart, mouse wheel on scrollBar
     */
    GestureEvent info;
    info.SetGlobalPoint(Point(SCROLL_WIDTH - 1.f, 1.f));
    info.SetInputEventType(InputEventType::AXIS);
    HandleDragStart(info);

    /**
     * @tc.steps: step2. HandleDragUpdate, mouse wheel down
     */
    float delta = -SCROLL_BAR_CHILD_HEIGHT;
    info.SetMainDelta(delta);
    HandleDragUpdate(info);
    FlushLayoutTask(stackNode_);
    float expectBarPosition = -delta / controlDistance * scrollableDistance;
    EXPECT_EQ(scrollBarPattern_->GetCurrentPosition(), expectBarPosition); // 512.f
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), -delta);

    /**
     * @tc.steps: step3. HandleDragUpdate, mouse wheel down continue
     */
    HandleDragUpdate(info);
    FlushLayoutTask(stackNode_);
    EXPECT_EQ(scrollBarPattern_->GetCurrentPosition(), scrollableDistance); // 640.f
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), controlDistance);

    /**
     * @tc.steps: step4. HandleDragUpdate, mouse wheel up
     */
    info.SetMainDelta(-delta);
    HandleDragUpdate(info);
    FlushLayoutTask(stackNode_);
    EXPECT_EQ(scrollBarPattern_->GetCurrentPosition(), scrollableDistance - expectBarPosition); // 138
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), controlDistance + delta); // 40.f

    /**
     * @tc.steps: step5. HandleDragEnd, mouse wheel end
     */
    HandleDragEnd(info);
    FlushLayoutTask(stackNode_);
    EXPECT_EQ(scrollBarPattern_->GetCurrentPosition(), scrollableDistance - expectBarPosition); // 138
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), controlDistance + delta); // 40.f
}

/**
 * @tc.name: HandleDrag003
 * @tc.desc: Test ScrollBar about HandleDrag in HORIZONTAL Layout and RTL Layout
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, HandleDrag003, TestSize.Level1)
{
    AceApplicationInfo::GetInstance().isRightToLeft_ = true;
    CreateStack(Alignment::BOTTOM_CENTER);
    CreateScroll(CONTENT_WIDTH, Axis::HORIZONTAL);
    CreateScrollBar(Axis::HORIZONTAL);
    CreateDone(stackNode_);
    EXPECT_TRUE(IsEqual(GetChildRect(stackNode_, 0), RectF(0, 0, SCROLL_WIDTH, SCROLL_HEIGHT)));
    EXPECT_TRUE(IsEqual(GetChildRect(stackNode_, 1), RectF(0.f, 780.f, SCROLL_WIDTH, SCROLL_BAR_CHILD_WIDTH)));
    float controlDistance = scrollBarPattern_->GetControlDistance();
    float scrollableDistance = scrollBarPattern_->GetScrollableDistance();
    EXPECT_EQ(controlDistance, CONTENT_WIDTH - SCROLL_WIDTH); // 120.f
    EXPECT_EQ(scrollableDistance, SCROLL_WIDTH - SCROLL_BAR_CHILD_HEIGHT); // 320.f
    EXPECT_EQ(scrollBarPattern_->GetCurrentPosition(), scrollableDistance);

    /**
     * @tc.steps: step1. HandleDragStart, drag on scrollBar
     */
    GestureEvent info;
    info.SetGlobalPoint(Point(1.f, SCROLL_HEIGHT - 1.f));
    HandleDragStart(info);

    /**
     * @tc.steps: step2. HandleDragUpdate, drag left
     */
    float delta = SCROLL_BAR_CHILD_HEIGHT;
    info.SetMainDelta(delta);
    HandleDragUpdate(info);
    FlushLayoutTask(stackNode_);
    EXPECT_EQ(scrollBarPattern_->GetCurrentPosition(), delta);

    /**
     * @tc.steps: step3. HandleDragUpdate, drag left continue
     */
    HandleDragUpdate(info);
    FlushLayoutTask(stackNode_);
    EXPECT_EQ(scrollBarPattern_->GetCurrentPosition(), 0.f);

    /**
     * @tc.steps: step4. HandleDragUpdate, drag right
     */
    info.SetMainDelta(-delta);
    HandleDragUpdate(info);
    FlushLayoutTask(stackNode_);
    EXPECT_EQ(scrollBarPattern_->GetCurrentPosition(), delta);

    /**
     * @tc.steps: step5. HandleDragEnd, drag end
     */
    HandleDragEnd(info);
    FlushLayoutTask(stackNode_);
    EXPECT_EQ(scrollBarPattern_->GetCurrentPosition(), delta);
}

/**
 * @tc.name: HandleDrag004
 * @tc.desc: Test ScrollBar about HandleDrag with InputEventType::AXIS in unScrollable Scroll, can not scroll
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, HandleDrag004, TestSize.Level1)
{
    CreateStack();
    CreateScroll(SCROLL_HEIGHT);
    CreateScrollBar();
    CreateDone(stackNode_);
    float scrollableDistance = scrollPattern_->GetScrollableDistance();
    EXPECT_EQ(scrollableDistance, 0.f);

    /**
     * @tc.steps: step1. HandleDragStart, mouse wheel scrollBar
     */
    GestureEvent info;
    info.SetGlobalPoint(Point(SCROLL_WIDTH - 1.f, 1.f));
    info.SetInputEventType(InputEventType::AXIS);
    HandleDragStart(info);

    /**
     * @tc.steps: step2. HandleDragUpdate, mouse wheel down
     */
    float delta = -SCROLL_BAR_CHILD_HEIGHT;
    info.SetMainDelta(delta);
    HandleDragUpdate(info);
    FlushLayoutTask(stackNode_);
    EXPECT_EQ(scrollBarPattern_->GetCurrentPosition(), 0.f);
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), 0.f);

    /**
     * @tc.steps: step5. HandleDragEnd, mouse wheel end
     */
    HandleDragEnd(info);
    FlushLayoutTask(stackNode_);
    EXPECT_EQ(scrollBarPattern_->GetCurrentPosition(), 0.f);
    EXPECT_EQ(scrollPattern_->GetTotalOffset(), 0.f);
}

/**
 * @tc.name: HandleDrag005
 * @tc.desc: Test ScrollBar about HandleDrag with velocity
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, HandleDrag005, TestSize.Level1)
{
    CreateStack();
    CreateScroll(CONTENT_HEIGHT);
    CreateScrollBar();
    CreateDone(stackNode_);

    /**
     * @tc.steps: step1. HandleDragEnd with velocity
     */
    GestureEvent info;
    info.SetGlobalPoint(Point(SCROLL_WIDTH - 1.f, 1.f));
    HandleDragStart(info);
    float delta = SCROLL_BAR_CHILD_HEIGHT;
    info.SetMainDelta(delta);
    HandleDragUpdate(info);
    FlushLayoutTask(stackNode_);
    info.SetMainVelocity(1200.f);
    HandleDragEnd(info);
    FlushLayoutTask(stackNode_);
    EXPECT_TRUE(scrollBarPattern_->frictionController_->IsRunning());

    /**
     * @tc.steps: step2. HandleDragEnd with velocity again
     */
    HandleDragStart(info);
    HandleDragUpdate(info);
    FlushLayoutTask(stackNode_);
    HandleDragEnd(info);
    FlushLayoutTask(stackNode_);
    EXPECT_TRUE(scrollBarPattern_->frictionController_->IsRunning());
}

/**
 * @tc.name: ScrollBarTest001
 * @tc.desc: When setting a fixed length and width, and axis is VERTICAL, verify the Measure and Layout functions.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBar(
        true, false, static_cast<int>(Axis::VERTICAL), static_cast<int>(DisplayMode::OFF), layoutConstraint);

    auto childLayout = layoutWrapper_->GetOrCreateChildByIndex(0);
    EXPECT_NE(childLayout, nullptr);
    auto scrollBarSize = layoutWrapper_->GetGeometryNode()->GetFrameSize();
    EXPECT_EQ(scrollBarSize, SCROLL_BAR_SELF_SIZE) << "scrollBarSize: " << scrollBarSize.ToString()
                                                   << " SCROLL_BAR_SELF_SIZE: " << SCROLL_BAR_SELF_SIZE.ToString();
    auto childSize = childLayout->GetGeometryNode()->GetFrameSize();
    auto expectChildSize = SizeF(SCROLL_BAR_CHILD_WIDTH, SCROLL_BAR_CHILD_HEIGHT);
    EXPECT_EQ(childSize, expectChildSize)
        << "childSize: " << childSize.ToString() << " expectChildSize: " << expectChildSize.ToString();

    /**
     * @tc.steps: step2. Verify the OnDirtyLayoutWrapperSwap function of scrollBar.
     * @tc.expected: step2. Compare return value with expected value.
     */
    DirtySwapConfig config;
    config.skipMeasure = true;
    config.skipLayout = false;
    auto ret = pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper_, config);
    EXPECT_EQ(ret, false);
    config.skipMeasure = true;
    config.skipLayout = true;
    ret = pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper_, config);
    EXPECT_EQ(ret, false);
    config.skipMeasure = false;
    config.skipLayout = false;
    ret = pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper_, config);
    EXPECT_EQ(ret, false);
    auto expectDistance = SCROLL_BAR_SELF_SIZE.Height() - SCROLL_BAR_CHILD_HEIGHT; // 1036
    EXPECT_EQ(pattern_->scrollableDistance_, expectDistance);
}

/**
 * @tc.name: ScrollBarTest002
 * @tc.desc: When the fixed length and width are not set, and axis is HORIZONTAL, verify the Measure and Layout, and
 * related functions in the scrollbar pattern. functions.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBar(
        true, false, static_cast<int>(Axis::HORIZONTAL), static_cast<int>(DisplayMode::AUTO), layoutConstraint);

    /**
     * @tc.steps: step2. After the layout is completed, check the framesize of the scrollBar and child nodes.
     * @tc.expected: step2. Compare whether the return value is expected to be the same.
     */
    auto childLayout = layoutWrapper_->GetOrCreateChildByIndex(0);
    EXPECT_NE(childLayout, nullptr);
    auto scrollBarSize = layoutWrapper_->GetGeometryNode()->GetFrameSize();
    // has no idealSize, scrollBarSize.height is child.height
    EXPECT_EQ(scrollBarSize, SizeF(DEVICE_WIDTH, SCROLL_BAR_CHILD_HEIGHT))
        << "scrollBarSize: " << scrollBarSize.ToString()
        << " SizeF(DEVICE_WIDTH, SCROLL_BAR_CHILD_HEIGHT): " << SizeF(DEVICE_WIDTH, SCROLL_BAR_CHILD_HEIGHT).ToString();
    auto childSize = childLayout->GetGeometryNode()->GetFrameSize();
    auto expectChildSize = SizeF(SCROLL_BAR_CHILD_WIDTH, SCROLL_BAR_CHILD_HEIGHT);
    EXPECT_EQ(childSize, expectChildSize)
        << "childSize: " << childSize.ToString() << " expectChildSize: " << expectChildSize.ToString();

    /**
     * @tc.steps: step3. Verify the callback function in scrollBar under different conditions.
     * @tc.expected: step3. Compare return value with expected value.
     */
    pattern_->OnModifyDone();
    EXPECT_EQ(pattern_->axis_, Axis::HORIZONTAL);
    auto callback = pattern_->scrollPositionCallback_;
    EXPECT_NE(callback, nullptr);
    auto ret = callback(0.0, SCROLL_FROM_START);
    EXPECT_EQ(ret, true);
    ret = callback(0.0, SCROLL_FROM_BAR);
    EXPECT_EQ(ret, false);
    ret = callback(1.0, SCROLL_FROM_UPDATE);
    EXPECT_EQ(ret, false);
    ret = callback(-1.0, SCROLL_FROM_ANIMATION);
    EXPECT_EQ(ret, false);

    pattern_->scrollableDistance_ = SCROLL_BAR_FLOAT_100;
    ret = callback(1.0, SCROLL_FROM_BAR);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(pattern_->lastOffset_, 0.0f);
    EXPECT_EQ(pattern_->currentOffset_, 1.0f);
    pattern_->axis_ = Axis::NONE;
    ret = callback(1.0, SCROLL_FROM_BAR);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: ScrollBarTest003
 * @tc.desc: When setting a fixed length and width, and axis is HORIZONTAL, verify the Measure and Layout functions, and
 * related functions in the scrollbar pattern.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBar(
        true, false, static_cast<int>(Axis::HORIZONTAL), static_cast<int>(DisplayMode::ON), layoutConstraint);

    /**
     * @tc.steps: step2. After the layout is completed, check the framesize of the scrollBar and child nodes.
     * @tc.expected: step2. Compare whether the return value is expected to be the same.
     */
    auto childLayout = layoutWrapper_->GetOrCreateChildByIndex(0);
    EXPECT_NE(childLayout, nullptr);
    auto scrollBarSize = layoutWrapper_->GetGeometryNode()->GetFrameSize();
    EXPECT_EQ(scrollBarSize, SCROLL_BAR_SELF_SIZE) << "scrollBarSize: " << scrollBarSize.ToString()
                                                   << " SCROLL_BAR_SELF_SIZE: " << SCROLL_BAR_SELF_SIZE.ToString();
    auto childSize = childLayout->GetGeometryNode()->GetFrameSize();
    auto expectChildSize = SizeF(SCROLL_BAR_CHILD_WIDTH, SCROLL_BAR_CHILD_HEIGHT);
    EXPECT_EQ(childSize, expectChildSize)
        << "childSize: " << childSize.ToString() << " expectChildSize: " << expectChildSize.ToString();

    /**
     * @tc.steps: step3. Verify the callback function in scrollBar under different conditions.
     * @tc.expected: step3. Compare return value with expected value.
     */
    auto scrollBarProxy = AceType::MakeRefPtr<ScrollBarProxy>();
    EXPECT_NE(scrollBarProxy, nullptr);
    pattern_->SetScrollBarProxy(scrollBarProxy);
    EXPECT_NE(pattern_->scrollBarProxy_, nullptr);
    auto callback = pattern_->scrollPositionCallback_;
    EXPECT_NE(callback, nullptr);
    pattern_->scrollableDistance_ = SCROLL_BAR_FLOAT_100;
    auto ret = callback(1.0, SCROLL_FROM_BAR);
    EXPECT_EQ(ret, true);
    pattern_->currentOffset_ = SCROLL_BAR_FLOAT_10;
    ret = callback(1.0, SCROLL_FROM_BAR);
    EXPECT_EQ(ret, true);
    pattern_->currentOffset_ = -1.0f;
    pattern_->scrollableDistance_ = 0.0f;
    ret = callback(1.0, SCROLL_FROM_BAR);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: ScrollBarTest004
 * @tc.desc: When the fixed length and width are not set, and axis is VERTICAL, verify the Measure and Layout functions,
 * and related functions in the scrollbar proxy.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBar(true, false, static_cast<int>(Axis::VERTICAL), -1, layoutConstraint);

    /**
     * @tc.steps: step2. After the layout is completed, check the framesize of the scrollBar and child nodes.
     * @tc.expected: step2. Compare whether the return value is expected to be the same.
     */
    auto childLayout = layoutWrapper_->GetOrCreateChildByIndex(0);
    EXPECT_NE(childLayout, nullptr);
    auto scrollBarSize = layoutWrapper_->GetGeometryNode()->GetFrameSize();
    auto expectScrollBarSize = SizeF(SCROLL_BAR_CHILD_WIDTH, DEVICE_HEIGHT);
    // has no idealSize, scrollBarSize.width is child.width
    EXPECT_EQ(scrollBarSize, expectScrollBarSize)
        << "scrollBarSize: " << scrollBarSize.ToString() << " expectScrollBarSize: " << expectScrollBarSize.ToString();
    auto childSize = childLayout->GetGeometryNode()->GetFrameSize();
    auto expectChildSize = SizeF(SCROLL_BAR_CHILD_WIDTH, SCROLL_BAR_CHILD_HEIGHT);
    EXPECT_EQ(childSize, expectChildSize)
        << "childSize: " << childSize.ToString() << " expectChildSize: " << expectChildSize.ToString();

    /**
     * @tc.steps: step3. Create scrollbar proxy and verify the RegisterScrollableNode function and NotifyScrollableNode
     * function.
     * @tc.expected: step3. Check whether relevant parameters are correct.
     */
    RefPtr<ScrollProxy> ScrollProxy = AceType::MakeRefPtr<ScrollBarProxy>();
    EXPECT_NE(ScrollProxy, nullptr);
    auto distance = -1.0;
    auto source = SCROLL_FROM_START;
    auto scrollFunction = [&distance, &source](double parameter1, int32_t parameter2) {
        distance = parameter1;
        source = parameter2;
        return true;
    };
    auto scrollBarProxy = AceType::DynamicCast<ScrollBarProxy>(ScrollProxy);
    EXPECT_NE(scrollBarProxy, nullptr);
    auto scrollPattern1 = AceType::MakeRefPtr<ScrollPattern>();
    EXPECT_NE(scrollPattern1, nullptr);
    auto* scrollRaw1 = AceType::RawPtr(scrollPattern1);
    EXPECT_NE(scrollRaw1, nullptr);
    scrollBarProxy->RegisterScrollableNode({ AceType::WeakClaim(scrollRaw1), std::move(scrollFunction) });
    EXPECT_EQ(scrollBarProxy->scrollableNodes_.size(), 1);
    auto* scrollBarPatternRaw = AceType::RawPtr(pattern_);
    EXPECT_NE(scrollBarPatternRaw, nullptr);
    scrollBarProxy->NotifyScrollableNode(1.0, SCROLL_FROM_BAR, AceType::WeakClaim(scrollBarPatternRaw));
    EXPECT_EQ(distance, 0.0);
    EXPECT_EQ(source, SCROLL_FROM_BAR);

    /**
     * @tc.steps: step4. When calling RegisterScrollableNode multiple times, verify the relevant parameters.
     * @tc.expected: step4. Check whether relevant parameters are correct.
     */
    auto scrollPattern2 = AceType::MakeRefPtr<ScrollPattern>();
    EXPECT_NE(scrollPattern2, nullptr);
    auto* scrollRaw2 = AceType::RawPtr(scrollPattern2);
    EXPECT_NE(scrollRaw2, nullptr);
    scrollBarProxy->RegisterScrollableNode({ AceType::WeakClaim(scrollRaw1), std::move(scrollFunction) });
    EXPECT_EQ(scrollBarProxy->scrollableNodes_.size(), 1);
    scrollBarProxy->RegisterScrollableNode({ AceType::WeakClaim(scrollRaw2), nullptr });
    EXPECT_EQ(scrollBarProxy->scrollableNodes_.size(), 2);
    scrollBarProxy->RegisterScrollableNode({ nullptr, std::move(scrollFunction) });
    EXPECT_EQ(scrollBarProxy->scrollableNodes_.size(), 3);
    scrollPattern1->estimatedHeight_ = SCROLL_BAR_FLOAT_100;
    pattern_->scrollableDistance_ = SCROLL_BAR_FLOAT_100;
    scrollBarProxy->NotifyScrollableNode(1.0, SCROLL_FROM_BAR, AceType::WeakClaim(scrollBarPatternRaw));
    EXPECT_EQ(distance, 1.0);
    EXPECT_EQ(source, SCROLL_FROM_BAR);

    /**
     * @tc.steps: step5. Repeatedly call UnRegisterScrollableNode to delete the relevant ScrollableNode.
     * @tc.expected: step5. Check the size of the list container.
     */
    scrollBarProxy->UnRegisterScrollableNode(AceType::WeakClaim(scrollRaw2));
    EXPECT_EQ(scrollBarProxy->scrollableNodes_.size(), 2);
    scrollBarProxy->UnRegisterScrollableNode(AceType::WeakClaim(scrollRaw2));
    EXPECT_EQ(scrollBarProxy->scrollableNodes_.size(), 2);
}

/**
 * @tc.name: ScrollBarTest005
 * @tc.desc: When the fixed length and width are not set, and axis is VERTICAL, verify the Measure and Layout functions,
 * and related functions in the scrollbar proxy.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBar(true, false, 3, 3, layoutConstraint);

    /**
     * @tc.steps: step2. Create scrollbar proxy and verify the RegisterScrollBar function and RegisterScrollBar
     * function.
     * @tc.expected: step2. Check whether relevant parameters are correct.
     */
    RefPtr<ScrollProxy> ScrollProxy = AceType::MakeRefPtr<ScrollBarProxy>();
    ASSERT_NE(ScrollProxy, nullptr);
    auto scrollBarProxy = AceType::DynamicCast<ScrollBarProxy>(ScrollProxy);
    ASSERT_NE(scrollBarProxy, nullptr);
    auto scrollPattern = AceType::MakeRefPtr<ScrollPattern>();
    ASSERT_NE(scrollPattern, nullptr);
    auto* scrollRaw = AceType::RawPtr(scrollPattern);
    ASSERT_NE(scrollRaw, nullptr);
    auto* scrollBarPatternRaw1 = AceType::RawPtr(pattern_);
    ASSERT_NE(scrollBarPatternRaw1, nullptr);
    scrollBarProxy->RegisterScrollBar(AceType::WeakClaim(scrollBarPatternRaw1));
    EXPECT_EQ(scrollBarProxy->scrollBars_.size(), 1);
    scrollBarProxy->NotifyScrollBar(nullptr);
    EXPECT_EQ(pattern_->currentOffset_, 0.0f);
    scrollBarProxy->NotifyScrollBar(AceType::WeakClaim(scrollRaw));
    EXPECT_EQ(pattern_->currentOffset_, 0.0f);

    /**
     * @tc.steps: step3. When calling RegisterScrollBar multiple times, verify the relevant parameters.
     * @tc.expected: step3. Check whether relevant parameters are correct.
     */
    auto scrollBarPattern = AceType::MakeRefPtr<ScrollBarPattern>();
    ASSERT_NE(scrollBarPattern, nullptr);
    auto* scrollBarPatternRaw2 = AceType::RawPtr(scrollBarPattern);
    ASSERT_NE(scrollBarPatternRaw2, nullptr);
    scrollBarProxy->RegisterScrollBar(AceType::WeakClaim(scrollBarPatternRaw1));
    EXPECT_EQ(scrollBarProxy->scrollBars_.size(), 1);
    scrollBarProxy->RegisterScrollBar(AceType::WeakClaim(scrollBarPatternRaw2));
    EXPECT_EQ(scrollBarProxy->scrollBars_.size(), 2);
    scrollBarProxy->RegisterScrollBar(nullptr);
    EXPECT_EQ(scrollBarProxy->scrollBars_.size(), 3);
    scrollBarProxy->StartScrollBarAnimator();
    pattern_->displayMode_ = DisplayMode::AUTO;
    scrollBarProxy->StartScrollBarAnimator();
    scrollBarProxy->StopScrollBarAnimator();
    scrollPattern->estimatedHeight_ = SCROLL_BAR_FLOAT_100;
    scrollPattern->barOffset_ = SCROLL_BAR_FLOAT_NEGATIVE_100;
    pattern_->scrollableDistance_ = 1.0f;
    EXPECT_EQ(pattern_->currentOffset_, 0.0f);

    /**
     * @tc.steps: step4. Repeatedly call UnRegisterScrollBar to delete the relevant scrollableBar.
     * @tc.expected: step4. Check the size of the list container.
     */
    scrollBarProxy->UnRegisterScrollBar(AceType::WeakClaim(scrollBarPatternRaw2));
    EXPECT_EQ(scrollBarProxy->scrollBars_.size(), 2);
    scrollBarProxy->UnRegisterScrollBar(AceType::WeakClaim(scrollBarPatternRaw2));
    EXPECT_EQ(scrollBarProxy->scrollBars_.size(), 2);
}

/**
 * @tc.name: PerformActionTest001
 * @tc.desc: ScrollBar Accessibility PerformAction test ScrollForward and ScrollBackward.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, PerformActionTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    CreateScrollBarWithoutChild(true, false, 3, 3);

    /**
     * @tc.steps: step2. Get scrollBar frameNode and pattern, set callback function.
     * @tc.expected: Related function is called.
     */
    pattern_->axis_ = Axis::NONE;
    pattern_->scrollableDistance_ = 0.0;
    pattern_->SetAccessibilityAction();

    /**
     * @tc.steps: step3. When scrollBar Axis is NONE and scrollable distance is 0, call the callback function in
     *                   scrollBarAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    EXPECT_TRUE(accessibilityProperty_->ActActionScrollForward());
    EXPECT_TRUE(accessibilityProperty_->ActActionScrollBackward());

    /**
     * @tc.steps: step4. When scrollBar Axis is NONE and scrollable distance is not 0, call the callback function in
     *                   scrollBarAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    pattern_->scrollableDistance_ = SCROLL_BAR_FLOAT_100;
    EXPECT_TRUE(accessibilityProperty_->ActActionScrollForward());
    EXPECT_TRUE(accessibilityProperty_->ActActionScrollBackward());

    /**
     * @tc.steps: step5. When scrollBar Axis is VERTICAL and scrollable distance is not 0, call the callback function in
     *                   scrollBarAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    pattern_->axis_ = Axis::VERTICAL;
    EXPECT_TRUE(accessibilityProperty_->ActActionScrollForward());
    EXPECT_TRUE(accessibilityProperty_->ActActionScrollBackward());

    /**
     * @tc.steps: step6. When scrollBar Axis is VERTICAL and scrollable distance is 0, call the callback function in
     *                   scrollBarAccessibilityProperty.
     * @tc.expected: Related function is called.
     */
    pattern_->scrollableDistance_ = 0.0;
    EXPECT_TRUE(accessibilityProperty_->ActActionScrollForward());
    EXPECT_TRUE(accessibilityProperty_->ActActionScrollBackward());
}

/**
 * @tc.name: AccessibilityEventTest001
 * @tc.desc: Test the ScrollEndCallback
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, AccessibilityEventTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    CreateScrollBarWithoutChild(true, false, static_cast<int>(Axis::VERTICAL), static_cast<int>(DisplayMode::OFF));

    /**
     * @tc.steps: step2. set callback function.
     */
    pattern_->OnModifyDone();
    ASSERT_NE(pattern_->scrollableEvent_, nullptr);
    auto callback = pattern_->scrollEndCallback_;
    ASSERT_NE(callback, nullptr);
    MockPipelineContext::SetUp();
    auto context = MockPipelineContext::GetCurrent();
    ASSERT_NE(context, nullptr);
    MockPipelineContext::GetCurrentContext()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();

    /**
     * @tc.steps: step3. DisplayMode::ON
     * @tc.expected: opacity_ is UINT8_MAX.
     */
    auto scrollBarProxy = AceType::MakeRefPtr<ScrollBarProxy>();
    EXPECT_NE(scrollBarProxy, nullptr);
    pattern_->SetScrollBarProxy(scrollBarProxy);
    EXPECT_NE(pattern_->scrollBarProxy_, nullptr);
    layoutProperty_->UpdateDisplayMode(DisplayMode::ON);
    pattern_->controlDistance_ = 10.f;
    pattern_->OnModifyDone();
    callback();
    EXPECT_EQ(pattern_->disappearAnimation_, nullptr);
    EXPECT_EQ(pattern_->opacity_, UINT8_MAX);

    /**
     * @tc.steps: step4. DisplayMode::AUTO and call callback function and controlDistance_ is 0.
     * @tc.expected: disappearAnimation_ is nullptr.
     */
    pattern_->displayMode_ = DisplayMode::AUTO;
    pattern_->controlDistance_ = 0.f;
    callback();
    EXPECT_EQ(pattern_->disappearAnimation_, nullptr);

    /**
     * @tc.steps: step5. call callback function and controlDistance_ bigger than 0.
     * @tc.expected: disappearAnimation_ is not nullptr.
     */
    pattern_->controlDistance_ = 10.f;
    pattern_->displayMode_ = DisplayMode::AUTO;
    callback();
    ASSERT_NE(pattern_->disappearAnimation_, nullptr);
}

/**
 * @tc.name: ScrollBarTest006
 * @tc.desc: Test OnModifyDone of ScrollBarPattern.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    CreateScrollBarWithoutChild(true, true, -1, -1);

    /**
     * @tc.steps: step2. Get scrollBar accessibilityProperty to call callback function.
     * @tc.expected: Related function is called.
     */
    pattern_->OnModifyDone();
    EXPECT_EQ(pattern_->GetDisplayMode(), DisplayMode::AUTO);
    EXPECT_EQ(pattern_->GetAxis(), Axis::VERTICAL);

    layoutProperty_->UpdateDisplayMode(DisplayMode::OFF);
    layoutProperty_->UpdateAxis(Axis::HORIZONTAL);
    pattern_->OnModifyDone();
    EXPECT_EQ(pattern_->GetDisplayMode(), DisplayMode::OFF);
    EXPECT_EQ(pattern_->GetAxis(), Axis::HORIZONTAL);

    layoutProperty_->UpdateDisplayMode(DisplayMode::AUTO);
    layoutProperty_->UpdateAxis(Axis::VERTICAL);
    pattern_->OnModifyDone();
    EXPECT_EQ(pattern_->GetDisplayMode(), DisplayMode::AUTO);
    EXPECT_EQ(pattern_->GetAxis(), Axis::VERTICAL);

    layoutProperty_->UpdateDisplayMode(DisplayMode::ON);
    layoutProperty_->UpdateAxis(Axis::HORIZONTAL);
    pattern_->OnModifyDone();
    EXPECT_EQ(pattern_->GetDisplayMode(), DisplayMode::ON);
    EXPECT_EQ(pattern_->GetAxis(), Axis::HORIZONTAL);
}

/**
 * @tc.name: ScrollBarTest007
 * @tc.desc: Test OnDirtyLayoutWrapperSwap of ScrollBarPattern.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBar(
        true, false, static_cast<int>(Axis::VERTICAL), static_cast<int>(DisplayMode::AUTO), layoutConstraint);

    /**
     * @tc.steps: step2. Verify the OnDirtyLayoutWrapperSwap function of scrollBar.
     * @tc.expected: step4. Compare return value with expected value.
     */
    DirtySwapConfig config;
    config.skipMeasure = true;
    config.skipLayout = false;
    pattern_->SetControlDistance(10.0f);
    EXPECT_EQ(pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper_, config), true);

    EXPECT_EQ(pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper_, config), false);

    pattern_->SetControlDistance(-10.0f);
    EXPECT_EQ(pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper_, config), true);

    layoutProperty_->UpdateDisplayMode(DisplayMode::ON);
    pattern_->OnModifyDone();
    pattern_->SetControlDistance(5.0f);
    EXPECT_EQ(pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper_, config), true);
}

/**
 * @tc.name: ScrollBarTest008
 * @tc.desc: Test OnDirtyLayoutWrapperSwap of ScrollBarPattern.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    CreateScrollBarWithoutChild(true, false, static_cast<int>(Axis::VERTICAL), static_cast<int>(DisplayMode::AUTO));

    /**
     * @tc.steps: step2. Verify the scrollPositionCallback_ function of scrollBar.
     * @tc.expected: step3. Init panRecognizer_.
     */
    auto context = PipelineContext::GetCurrentContext();
    ASSERT_NE(pattern_, nullptr);
    context->SetMinPlatformVersion(10);
    pattern_->InitPanRecognizer();
    EXPECT_NE(pattern_->panRecognizer_, nullptr);
    EXPECT_NE(pattern_->panRecognizer_->onActionStart_, nullptr);
    EXPECT_NE(pattern_->panRecognizer_->onActionEnd_, nullptr);
    EXPECT_NE(pattern_->panRecognizer_->onActionUpdate_, nullptr);
}

/**
 * @tc.name: ScrollBarTest009
 * @tc.desc: Test OnDirtyLayoutWrapperSwap of ScrollBarPattern.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBar(
        true, false, static_cast<int>(Axis::VERTICAL), static_cast<int>(DisplayMode::AUTO), layoutConstraint);

    /**
     * @tc.steps: step2. Verify the scrollPositionCallback_ function of scrollBar.
     * @tc.expected: step2. Compare return value with expected value.
     */
    DirtySwapConfig config;
    config.skipMeasure = true;
    config.skipLayout = false;
    pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper_, config);
    auto distance = pattern_->scrollableDistance_;

    EXPECT_EQ(pattern_->scrollPositionCallback_(100, SCROLL_FROM_START), true);
    EXPECT_FLOAT_EQ(pattern_->currentOffset_, 0.f);

    EXPECT_EQ(pattern_->scrollPositionCallback_(distance / 2.0, 1), true);
    EXPECT_FLOAT_EQ(pattern_->currentOffset_, distance / 2.0);

    EXPECT_EQ(pattern_->scrollPositionCallback_(distance - distance / 2.0, 1), true);
    EXPECT_FLOAT_EQ(pattern_->currentOffset_, distance);

    EXPECT_EQ(pattern_->scrollPositionCallback_(100, 1), false);
    EXPECT_FLOAT_EQ(pattern_->currentOffset_, distance);
}

/**
 * @tc.name: ScrollBarTest010
 * @tc.desc: Test SetSpecificSupportAction of ScrollBarAccessibilityProperty.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBar(true, true, -1, -1, layoutConstraint);

    /**
     * @tc.steps: step2. Verify the IsScrollable function of ScrollBarAccessibilityProperty.
     * @tc.expected: step2. Compare the supportActions_ value with expected value.
     */
    auto supportActions = accessibilityProperty_->supportActions_;
    accessibilityProperty_->SetSpecificSupportAction();
    EXPECT_EQ(accessibilityProperty_->supportActions_, supportActions);

    layoutProperty_->UpdateAxis(Axis::NONE);
    pattern_->OnModifyDone();
    accessibilityProperty_->SetSpecificSupportAction();
    EXPECT_EQ(accessibilityProperty_->supportActions_, supportActions);

    layoutProperty_->UpdateAxis(Axis::VERTICAL);
    pattern_->OnModifyDone();
    accessibilityProperty_->SetSpecificSupportAction();
    EXPECT_EQ(accessibilityProperty_->supportActions_, supportActions);

    pattern_->SetControlDistance(10.f);
    accessibilityProperty_->SetSpecificSupportAction();
    EXPECT_EQ(accessibilityProperty_->supportActions_, supportActions);

    /**
     * @tc.steps: step3. Verify the SetSpecificSupportAction function of ScrollBarAccessibilityProperty.
     * @tc.expected: step3. Compare the supportActions_ value with expected value.
     */
    DirtySwapConfig config;
    config.skipMeasure = true;
    config.skipLayout = false;
    pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper_, config);
    auto distance = pattern_->scrollableDistance_;

    accessibilityProperty_->SetSpecificSupportAction();
    EXPECT_EQ(accessibilityProperty_->supportActions_,
        supportActions | (1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_FORWARD)));

    pattern_->scrollPositionCallback_(distance / 2.0, 1);
    accessibilityProperty_->ResetSupportAction();
    EXPECT_EQ(accessibilityProperty_->supportActions_,
        supportActions | (1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_FORWARD)) |
            (1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_BACKWARD)));

    pattern_->scrollPositionCallback_(distance, 1);
    accessibilityProperty_->ResetSupportAction();
    EXPECT_EQ(accessibilityProperty_->supportActions_,
        supportActions | (1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_BACKWARD)));
}

/**
 * @tc.name: ScrollBarTest011
 * @tc.desc: Test Create of ScrollBarModelNG.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar.
     */
    ScrollBarModelNG model;
    RefPtr<ScrollProxy> scrollProxy = AceType::MakeRefPtr<ScrollBarProxy>();
    auto proxy = model.GetScrollBarProxy(scrollProxy);
    model.Create(proxy, false, false, -1, -1);
    GetInstance();

    /**
     * @tc.steps: step2. Create scrollBar and initialize related properties.
     * @tc.expected: step2. Compare value with expected value.
     */
    EXPECT_EQ(pattern_->scrollBarProxy_, nullptr);
    EXPECT_EQ(pattern_->opacity_, UINT8_MAX);
    EXPECT_EQ(layoutProperty_->HasAxis(), false);
    EXPECT_EQ(layoutProperty_->HasDisplayMode(), false);
    EXPECT_EQ(layoutProperty_->HasVisibility(), false);
}

/**
 * @tc.name: ScrollBarTest012
 * @tc.desc: Test OnCollectTouchTarget of ScrollBarPattern.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBar(true, true, -1, -1, layoutConstraint);

    /**
     * @tc.steps: step2. Verify the OnCollectTouchTarget function of ScrollBarPattern.
     * @tc.expected: step2. Compare the value with expected value.
     */
    const RectF& rect = pattern_->childRect_;
    PointF localPoint = PointF(rect.Width() + rect.GetX(), rect.Height() + rect.GetY());
    const SourceType source = SourceType::TOUCH;
    OffsetF coordinateOffset;
    GetEventTargetImpl getEventTargetImpl;
    TouchTestResult result;
    ResponseLinkResult responseLinkResult;
    const int32_t size = result.size();
    EXPECT_EQ(pattern_->scrollableEvent_->InBarRegion(localPoint, source), true);

    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::LIST_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    pattern_->scrollableEvent_->BarCollectTouchTarget(
        coordinateOffset, getEventTargetImpl, result, frameNode, nullptr, responseLinkResult);
    EXPECT_FLOAT_EQ(pattern_->panRecognizer_->GetCoordinateOffset().GetX(), coordinateOffset.GetX());
    EXPECT_FLOAT_EQ(pattern_->panRecognizer_->GetCoordinateOffset().GetY(), coordinateOffset.GetY());
    EXPECT_EQ(result.size(), size + 1);

    localPoint.SetX(localPoint.GetX() + 1);
    localPoint.SetY(localPoint.GetY() + 1);
    EXPECT_EQ(pattern_->scrollableEvent_->InBarRegion(localPoint, source), false);

    pattern_->panRecognizer_ = nullptr;
    pattern_->scrollableEvent_->BarCollectTouchTarget(
        coordinateOffset, getEventTargetImpl, result, frameNode, nullptr, responseLinkResult);
    EXPECT_EQ(result.size(), size + 1);
}

/**
 * @tc.name: ScrollBarTest013
 * @tc.desc: Test StartDisappearAnimator of ScrollBarPattern.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest013, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBar(true, true, -1, 0, layoutConstraint);

    /**
     * @tc.steps: step2. Verify the StartDisappearAnimator function of ScrollBarPattern.
     * @tc.expected: step2. Compare the value with expected value.
     */
    pattern_->SetControlDistance(10.f);
    layoutProperty_->UpdateDisplayMode(DisplayMode::AUTO);
    pattern_->OnModifyDone();
    EXPECT_EQ(pattern_->GetDisplayMode(), DisplayMode::AUTO);
    EXPECT_EQ(pattern_->opacity_, 0);

    layoutProperty_->UpdateDisplayMode(DisplayMode::OFF);
    pattern_->OnModifyDone();
    EXPECT_EQ(pattern_->GetDisplayMode(), DisplayMode::OFF);

    layoutProperty_->UpdateDisplayMode(DisplayMode::AUTO);
    pattern_->OnModifyDone();
    EXPECT_EQ(pattern_->GetDisplayMode(), DisplayMode::AUTO);
    EXPECT_EQ(pattern_->opacity_, 0);
    EXPECT_NE(pattern_->disappearAnimation_, nullptr);

    /**
     * @tc.steps: step2. Verify the StopDisappearAnimator function of ScrollBarPattern.
     * @tc.expected: step2. Compare the value with expected value.
     */
    layoutProperty_->UpdateDisplayMode(DisplayMode::ON);
    pattern_->OnModifyDone();
    EXPECT_EQ(pattern_->GetDisplayMode(), DisplayMode::ON);
    EXPECT_NE(pattern_->disappearAnimation_, nullptr);
    EXPECT_EQ(pattern_->opacity_, UINT8_MAX);
}

/**
 * @tc.name: ScrollBarTest014
 * @tc.desc: Test ProcessFrictionMotion of ScrollBarPattern.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest014, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBar(true, true, -1, -1, layoutConstraint);

    /**
     * @tc.steps: step2. Verify the ProcessFrictionMotion function of ScrollBarPattern.
     * @tc.expected: step2. Compare the value with expected value.
     */
    pattern_->ProcessFrictionMotion(10.0);
    pattern_->ProcessFrictionMotionStop();
    EXPECT_DOUBLE_EQ(pattern_->frictionPosition_, 10.0);

    pattern_->scrollBarProxy_ = nullptr;
    pattern_->scrollPositionCallback_ = nullptr;
    pattern_->ProcessFrictionMotion(20.0);
    pattern_->ProcessFrictionMotionStop();
    EXPECT_DOUBLE_EQ(pattern_->frictionPosition_, 20.0);

    pattern_->scrollEndCallback_ = nullptr;
    pattern_->scrollableEvent_ = nullptr;
    pattern_->ProcessFrictionMotionStop();
    pattern_->OnModifyDone();
    EXPECT_NE(pattern_->scrollEndCallback_, nullptr);
    EXPECT_NE(pattern_->scrollPositionCallback_, nullptr);
    EXPECT_NE(pattern_->scrollableEvent_, nullptr);
}

/**
 * @tc.name: ScrollBarTest015
 * @tc.desc: Test scrollbar opacity
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest015, TestSize.Level1)
{
    /**
     * @tc.steps: case1. controlDistance_ is 0
     * @tc.expected: opacity_ is 0
     */
    CreateScrollBarWithoutChild(true, false, static_cast<int>(Axis::VERTICAL), static_cast<int>(DisplayMode::ON));
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->controlDistance_, 0);
    EXPECT_EQ(pattern_->opacity_, 0);

    /**
     * @tc.steps: case2. controlDistance_ > 0
     * @tc.expected: opacity_ is UINT8_MAX
     */
    pattern_->controlDistanceChanged_ = true;
    pattern_->controlDistance_ = 10;
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushLayoutTask(frameNode_);
    EXPECT_EQ(pattern_->opacity_, UINT8_MAX);
}

/**
 * @tc.name: ScrollBarTest016
 * @tc.desc: Test scrollbar NotifySnapScroll in the scrollbar proxy.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest016, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBar(true, false, static_cast<int>(Axis::VERTICAL), -1, layoutConstraint);

    /**
     * @tc.steps: step2. Create scrollbar proxy and verify the RegisterScrollableNode function and
     * NotifySnapScroll function.
     * @tc.expected: step3. Check whether relevant parameters are correct.
     */
    RefPtr<ScrollProxy> ScrollProxy = AceType::MakeRefPtr<ScrollBarProxy>();
    ASSERT_NE(ScrollProxy, nullptr);
    auto distance = -1.0;
    auto source = SCROLL_FROM_START;
    auto scrollStartCallback = [&distance, &source](double parameter1, int32_t parameter2) {
        distance = parameter1;
        source = parameter2;
        return true;
    };
    auto scrollEndCallback = [&distance, &source]() {
        distance = -2.0;
        source = SCROLL_FROM_BAR_FLING;
        return true;
    };

    // no ScrollableNode
    auto scrollBarProxy = AceType::DynamicCast<ScrollBarProxy>(ScrollProxy);
    EXPECT_NE(scrollBarProxy, nullptr);
    EXPECT_EQ(scrollBarProxy->scrollableNodes_.size(), 0);
    scrollBarProxy->NotifyScrollStart();
    EXPECT_EQ(distance, -1.0);
    EXPECT_EQ(source, SCROLL_FROM_START);
    scrollBarProxy->NotifyScrollStop();
    EXPECT_EQ(distance, -1.0);
    EXPECT_EQ(source, SCROLL_FROM_START);

    // one ScrollableNode
    auto scrollPattern1 = AceType::MakeRefPtr<ScrollPattern>();
    EXPECT_NE(scrollPattern1, nullptr);
    auto* scrollRaw1 = AceType::RawPtr(scrollPattern1);
    EXPECT_NE(scrollRaw1, nullptr);
    scrollBarProxy->RegisterScrollableNode(
        { AceType::WeakClaim(scrollRaw1), nullptr, std::move(scrollStartCallback), std::move(scrollEndCallback) });
    EXPECT_EQ(scrollBarProxy->scrollableNodes_.size(), 1);
    scrollBarProxy->NotifyScrollStart();
    EXPECT_EQ(distance, 0.0);
    EXPECT_EQ(source, SCROLL_FROM_BAR);
    scrollBarProxy->NotifyScrollStop();
    EXPECT_EQ(distance, -2.0);
    EXPECT_EQ(source, SCROLL_FROM_BAR_FLING);
}

/**
 * @tc.name: ScrollBarTest017
 * @tc.desc: Test scrollbar NotifySnapScroll in the scrollbar proxy.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest017, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBar(true, false, static_cast<int>(Axis::VERTICAL), -1, layoutConstraint);

    /**
     * @tc.steps: step2. Create scrollbar proxy and verify the RegisterScrollableNode function and
     * NotifySnapScroll function.
     * @tc.expected: Check whether relevant parameters are correct.
     */
    RefPtr<ScrollProxy> ScrollProxy = AceType::MakeRefPtr<ScrollBarProxy>();
    ASSERT_NE(ScrollProxy, nullptr);
    auto distance = -1.0;
    auto source = SCROLL_FROM_START;
    auto scrollStartCallback = [&distance, &source](double parameter1, int32_t parameter2) {
        distance = parameter1;
        source = parameter2;
        return true;
    };
    auto scrollEndCallback = [&distance, &source]() {
        distance = -2.0;
        source = SCROLL_FROM_BAR_FLING;
        return true;
    };

    // one ScrollableNode
    auto scrollBarProxy = AceType::DynamicCast<ScrollBarProxy>(ScrollProxy);
    auto scrollPattern1 = AceType::MakeRefPtr<ScrollPattern>();
    EXPECT_NE(scrollPattern1, nullptr);
    auto* scrollRaw1 = AceType::RawPtr(scrollPattern1);
    EXPECT_NE(scrollRaw1, nullptr);
    scrollBarProxy->RegisterScrollableNode(
        { AceType::WeakClaim(scrollRaw1), nullptr, std::move(scrollStartCallback), std::move(scrollEndCallback) });

    // two ScrollableNode (the last node callback func is null)
    distance = -1.0;
    source = SCROLL_FROM_START;
    auto scrollPattern2 = AceType::MakeRefPtr<ScrollPattern>();
    EXPECT_NE(scrollPattern2, nullptr);
    auto* scrollRaw2 = AceType::RawPtr(scrollPattern2);
    EXPECT_NE(scrollRaw2, nullptr);
    scrollBarProxy->RegisterScrollableNode({ AceType::WeakClaim(scrollRaw2), nullptr, nullptr, nullptr });
    EXPECT_EQ(scrollBarProxy->scrollableNodes_.size(), 2);
    scrollBarProxy->NotifyScrollStart();
    EXPECT_EQ(distance, 0.0);
    EXPECT_EQ(source, SCROLL_FROM_BAR);
    scrollBarProxy->NotifyScrollStop();
    EXPECT_EQ(distance, -2.0);
    EXPECT_EQ(source, SCROLL_FROM_BAR_FLING);
}

/**
 * @tc.name: ScrollBarTest018
 * @tc.desc: Test scrollbar NotifySnapScroll in the scrollbar proxy.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest018, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBar(true, false, static_cast<int>(Axis::VERTICAL), -1, layoutConstraint);

    /**
     * @tc.steps: step2. Create scrollbar proxy and verify the RegisterScrollableNode function and
     * NotifySnapScroll function.
     * @tc.expected: Check whether relevant parameters are correct.
     */
    RefPtr<ScrollProxy> ScrollProxy = AceType::MakeRefPtr<ScrollBarProxy>();
    ASSERT_NE(ScrollProxy, nullptr);
    float scrollSnapDelta = 0;
    float scrollSnapVelocity = 0;
    auto startScrollSnapMotionCallback = [&scrollSnapDelta, &scrollSnapVelocity](float parameter1, float parameter2) {
        scrollSnapDelta = parameter1;
        scrollSnapVelocity = parameter2;
    };
    CalePredictSnapOffsetCallback calePredictSnapOffsetCallback = [](float delta, float dragDistance, float velocity) {
        return (delta + dragDistance + velocity);
    };
    // no ScrollableNode
    auto scrollBarProxy = AceType::DynamicCast<ScrollBarProxy>(ScrollProxy);
    EXPECT_NE(scrollBarProxy, nullptr);
    EXPECT_EQ(scrollBarProxy->scrollableNodes_.size(), 0);
    scrollBarProxy->NotifySnapScroll(10.0f, 10.0f, 1.0f, 1.0f);
    EXPECT_EQ(scrollSnapDelta, 0);
    EXPECT_EQ(scrollSnapVelocity, 0);

    // one ScrollableNode
    auto scrollPattern1 = AceType::MakeRefPtr<ScrollPattern>();
    EXPECT_NE(scrollPattern1, nullptr);
    auto* scrollRaw1 = AceType::RawPtr(scrollPattern1);
    EXPECT_NE(scrollRaw1, nullptr);
    scrollPattern1->scrollableDistance_ = SCROLL_BAR_FLOAT_100;
    scrollBarProxy->RegisterScrollableNode({ AceType::WeakClaim(scrollRaw1), nullptr, nullptr, nullptr,
        std::move(calePredictSnapOffsetCallback), std::move(startScrollSnapMotionCallback) });
    EXPECT_EQ(scrollBarProxy->scrollableNodes_.size(), 1);
    scrollBarProxy->NotifySnapScroll(10.0f, 10.0f, 1.0f, 1.0f);
    EXPECT_EQ(scrollSnapDelta, -10.0);
    EXPECT_EQ(scrollSnapVelocity, 10.0);
}

/**
 * @tc.name: ScrollBarTest019
 * @tc.desc: Test scrollbar NotifySnapScroll in the scrollbar proxy.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest019, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBar(true, false, static_cast<int>(Axis::VERTICAL), -1, layoutConstraint);

    /**
     * @tc.steps: step2. Create scrollbar proxy and verify the RegisterScrollableNode function and
     * NotifySnapScroll function.
     * @tc.expected: step3. Check whether relevant parameters are correct.
     */
    RefPtr<ScrollProxy> ScrollProxy = AceType::MakeRefPtr<ScrollBarProxy>();
    ASSERT_NE(ScrollProxy, nullptr);
    float scrollSnapDelta = 0;
    float scrollSnapVelocity = 0;
    auto startScrollSnapMotionCallback = [&scrollSnapDelta, &scrollSnapVelocity](float parameter1, float parameter2) {
        scrollSnapDelta = parameter1;
        scrollSnapVelocity = parameter2;
    };
    CalePredictSnapOffsetCallback calePredictSnapOffsetCallback = [](float delta, float dragDistance, float velocity) {
        return (delta + dragDistance + velocity);
    };

    // one ScrollableNode calePredictSnapOffsetCallback/startScrollSnapMotionCallback
    auto scrollBarProxy = AceType::DynamicCast<ScrollBarProxy>(ScrollProxy);
    auto scrollPattern1 = AceType::MakeRefPtr<ScrollPattern>();
    auto* scrollRaw1 = AceType::RawPtr(scrollPattern1);
    scrollBarProxy->RegisterScrollableNode({ AceType::WeakClaim(scrollRaw1), nullptr, nullptr, nullptr,
        std::move(calePredictSnapOffsetCallback), std::move(startScrollSnapMotionCallback) });
    // two ScrollableNode calePredictSnapOffsetCallback/nullptr
    auto scrollPattern2 = AceType::MakeRefPtr<ScrollPattern>();
    auto* scrollRaw2 = AceType::RawPtr(scrollPattern2);
    scrollBarProxy->RegisterScrollableNode({ AceType::WeakClaim(scrollRaw2), nullptr, nullptr, nullptr,
        std::move(calePredictSnapOffsetCallback), nullptr });
    // three ScrollableNode nullptr/startScrollSnapMotionCallback
    scrollSnapDelta = 0;
    scrollSnapVelocity = 0;
    auto scrollPattern3 = AceType::MakeRefPtr<ScrollPattern>();
    auto* scrollRaw3 = AceType::RawPtr(scrollPattern3);
    EXPECT_NE(scrollRaw3, nullptr);
    scrollPattern3->scrollableDistance_ = SCROLL_BAR_FLOAT_100;
    scrollBarProxy->RegisterScrollableNode({ AceType::WeakClaim(scrollRaw3), nullptr, nullptr, nullptr, nullptr,
        std::move(startScrollSnapMotionCallback) });
    EXPECT_EQ(scrollBarProxy->scrollableNodes_.size(), 3);
    scrollBarProxy->NotifySnapScroll(10.0f, 10.0f, 1.0f, 1.0f);
    EXPECT_EQ(scrollSnapDelta, -10.0);
    EXPECT_EQ(scrollSnapVelocity, 10.0);
}

/**
 * @tc.name: HandleClickScroll001
 * @tc.desc: Test scrolling when clicking on the scroll bar
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, HandleClickScroll001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create CreateScrollBar , GestureEvent and ScrollBarProxy.
     * @tc.expected: create CreateScrollBar , GestureEvent and ScrollBarProxy created successfully.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBar(true, false, static_cast<int>(Axis::VERTICAL), static_cast<int>(DisplayMode::ON), layoutConstraint);
    pattern_->scrollBarProxy_ = AceType::MakeRefPtr<ScrollBarProxy>();
    auto scrollBarProxy = pattern_->scrollBarProxy_;
    pattern_->locationInfo_ = Offset(1.0f, 1.0f);
    pattern_->InitClickEvent();
    const RectF& rect = RectF(0.0f, 10.0f, 30.0f, 50.0f);
    const SizeF& size = SizeF(10.0f, 20.0f);
    auto scrollBar = pattern_->GetHost();
    scrollBar->GetGeometryNode()->SetFrameSize(size);
    pattern_->SetChildRect(rect);
    // /**
    //  * @tc.steps: step2. Test HandleClickEvent.
    //  * @tc.expect: reverse is FALSE.
    //  */
    pattern_->isMousePressed_ = true;
    auto reverse = false;
    auto smooth = false;
    auto scrollPageFunction = [&reverse, &smooth](bool parameter1, bool parameter2) {
        reverse = parameter1;
        smooth = parameter2;
    };
    auto scrollPattern1 = AceType::MakeRefPtr<ScrollPattern>();
    EXPECT_NE(scrollPattern1, nullptr);
    auto* scrollRaw1 = AceType::RawPtr(scrollPattern1);
    EXPECT_NE(scrollRaw1, nullptr);
    scrollBarProxy->RegisterScrollableNode({ AceType::WeakClaim(scrollRaw1), nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, std::move(scrollPageFunction) });
    EXPECT_EQ(scrollBarProxy->scrollableNodes_.size(), 1);
    // /**
    //  * @tc.steps: step2. Test HandleClickEvent.
    //  * @tc.expect: reverse is TRUE or FALSE.
    //  */
    pattern_->HandleClickEvent();
    EXPECT_TRUE(reverse);
    pattern_->locationInfo_ = Offset(1.0f, 70.0f);
    pattern_->HandleClickEvent();
    EXPECT_FALSE(reverse);
    pattern_->locationInfo_ = Offset(1.0f, 35.0f);
    pattern_->HandleClickEvent();
    EXPECT_FALSE(reverse);
}

/**
 * @tc.name: HandleLongPressScroll001
 * @tc.desc: Test long press and hold scrolling when clicking on the scroll bar
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, HandleLongPressScroll001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create CreateScrollBar and ScrollBarProxy.
     * @tc.expected: create CreateScrollBar and ScrollBarProxy created successfully.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBar(true, false, static_cast<int>(Axis::VERTICAL), static_cast<int>(DisplayMode::ON), layoutConstraint);
    pattern_->scrollBarProxy_ = AceType::MakeRefPtr<ScrollBarProxy>();
    auto scrollBarProxy = pattern_->scrollBarProxy_;
    pattern_->InitLongPressEvent();
    pattern_->locationInfo_ = Offset(1.0f, 1.0f);
    const RectF& rect = RectF(0.0f, 10.0f, 30.0f, 50.0f);
    const SizeF& size = SizeF(10.0f, 100.0f);
    auto scrollBar = pattern_->GetHost();
    scrollBar->GetGeometryNode()->SetFrameSize(size);
    pattern_->SetChildRect(rect);
    pattern_->isMousePressed_ = true;
    auto reverse = false;
    auto smooth = false;
    auto scrollPageFunction = [this, &reverse, &smooth](bool parameter1, bool parameter2) {
        reverse = parameter1;
        smooth = parameter2;
        pattern_->isMousePressed_ = false;
    };
    auto scrollPattern1 = AceType::MakeRefPtr<ScrollPattern>();
    EXPECT_NE(scrollPattern1, nullptr);
    auto* scrollRaw1 = AceType::RawPtr(scrollPattern1);
    EXPECT_NE(scrollRaw1, nullptr);
    scrollBarProxy->RegisterScrollableNode({ AceType::WeakClaim(scrollRaw1), nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, std::move(scrollPageFunction) });
    EXPECT_EQ(scrollBarProxy->scrollableNodes_.size(), 1);
    // /**
    //  * @tc.steps: step2. Test HandleClickEvent.
    //  * @tc.expect: reverse equal to TRUE or FALSE.
    //  */
    pattern_->HandleLongPress(true);
    EXPECT_TRUE(reverse);
    pattern_->locationInfo_ = Offset(1.0f, 70.0f);
    pattern_->isMousePressed_ = true;
    pattern_->scrollingUp_ = false;
    pattern_->scrollingDown_ = false;
    pattern_->HandleLongPress(true);
    EXPECT_FALSE(reverse);
    pattern_->locationInfo_ = Offset(1.0f, 35.0f);
    pattern_->isMousePressed_ = true;
    pattern_->scrollingUp_ = false;
    pattern_->scrollingDown_ = false;
    pattern_->HandleLongPress(true);
    EXPECT_FALSE(reverse);
}

/**
 * @tc.name: InitMouseEvent001
 * @tc.desc: Test mouse event callback
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, InitMouseEvent001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create CreateScrollBar and MouseInfo.
     * @tc.expected: create CreateScrollBar and MouseInfo created successfully.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBar(true, false, static_cast<int>(Axis::VERTICAL), static_cast<int>(DisplayMode::ON), layoutConstraint);
    // /**
    //  * @tc.steps: step2. Test HandleMouseEvent.
    //  * @tc.expect: isMousePressed_ is FALSE.
    //  */
    pattern_->InitMouseEvent();
    MouseInfo info;
    info.SetAction(MouseAction::PRESS);
    info.SetButton(MouseButton::LEFT_BUTTON);
    auto& inputEvents = pattern_->GetEventHub<EventHub>()->GetInputEventHub()->mouseEventActuator_->inputEvents_;
    EXPECT_EQ(inputEvents.size(), 1);
    for (const auto& callback : inputEvents) {
        if (callback) {
            (*callback)(info);
        }
    }
    EXPECT_TRUE(pattern_->isMousePressed_);
    MouseInfo info1;
    info1.SetAction(MouseAction::RELEASE);
    info1.SetButton(MouseButton::LEFT_BUTTON);
    for (const auto& callback : inputEvents) {
        if (callback) {
            (*callback)(info1);
        }
    }
    EXPECT_FALSE(pattern_->isMousePressed_);
}

/**
 * @tc.name: BarCollectLongPressTarget001
 * @tc.desc: Test OnCollectLongPressTarget of ScrollBarPattern.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, BarCollectLongPressTarget001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create scrollBar and initialize related properties.
     */
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBar(true, true, -1, -1, layoutConstraint);

    /**
     * @tc.steps: step2. Verify the OnCollectLongPressTarget function of ScrollBarPattern.
     * @tc.expected: step2. Compare the value with expected value.
     */
    const RectF& rect = pattern_->childRect_;
    PointF localPoint = PointF(rect.Width() + rect.GetX(), rect.Height() + rect.GetY());
    const SourceType source = SourceType::TOUCH;
    OffsetF coordinateOffset;
    GetEventTargetImpl getEventTargetImpl;
    TouchTestResult result;
    ResponseLinkResult responseLinkResult;
    const int32_t size = result.size();
    EXPECT_EQ(pattern_->scrollableEvent_->InBarRectRegion(localPoint, source), true);
    auto frameNode = AceType::MakeRefPtr<FrameNode>(V2::LIST_ETS_TAG, -1, AceType::MakeRefPtr<Pattern>());
    pattern_->scrollableEvent_->BarCollectLongPressTarget(
        coordinateOffset, getEventTargetImpl, result, frameNode, nullptr, responseLinkResult);
    EXPECT_FLOAT_EQ(pattern_->longPressRecognizer_->GetCoordinateOffset().GetX(), coordinateOffset.GetX());
    EXPECT_FLOAT_EQ(pattern_->longPressRecognizer_->GetCoordinateOffset().GetY(), coordinateOffset.GetY());
    EXPECT_EQ(result.size(), size + 2);

    localPoint.SetX(localPoint.GetX() + 1);
    localPoint.SetY(localPoint.GetY() + 1);
    EXPECT_EQ(pattern_->scrollableEvent_->InBarRectRegion(localPoint, source), true);

    pattern_->longPressRecognizer_ = nullptr;
    pattern_->scrollableEvent_->BarCollectLongPressTarget(
        coordinateOffset, getEventTargetImpl, result, frameNode, nullptr, responseLinkResult);
    EXPECT_EQ(result.size(), size + 3);
}

/**
 * @tc.name: ScrollBarTest020
 * @tc.desc: When axis is VERTICAL, and no subcomponents, verify the Measure and Layout,
 * and related functions in the scrollbar pattern functions.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest020, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBarWithoutSubComponent(
        true, false, static_cast<int>(Axis::VERTICAL), static_cast<int>(DisplayMode::OFF), layoutConstraint);
    auto scrollBarSize = layoutWrapper_->GetGeometryNode()->GetFrameSize();
    EXPECT_EQ(scrollBarSize, SCROLL_BAR_SELF_SIZE) << "scrollBarSize: " << scrollBarSize.ToString()
                                                   << " SCROLL_BAR_SELF_SIZE: " << SCROLL_BAR_SELF_SIZE.ToString();
    DirtySwapConfig config;
    config.skipMeasure = false;
    config.skipLayout = false;
    pattern_->SetChild(false);
    auto ret = pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper_, config);
    EXPECT_EQ(ret, true);
    pattern_->SetChild(true);
    pattern_->preFrameChildState_ = true;
    ret = pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper_, config);
    EXPECT_EQ(ret, false);
    CreateScrollBarWithoutSubComponent(
        true, false, static_cast<int>(Axis::VERTICAL), static_cast<int>(DisplayMode::ON), layoutConstraint);
    ret = pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper_, config);
    EXPECT_EQ(ret, true);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest021
 * @tc.desc: When axis is HORIZONTAL, and no subcomponents, verify the Measure and Layout.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest021, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBarWithoutSubComponent(
        true, false, static_cast<int>(Axis::HORIZONTAL), static_cast<int>(DisplayMode::ON), layoutConstraint);
    auto scrollBarSize = layoutWrapper_->GetGeometryNode()->GetFrameSize();
    EXPECT_EQ(scrollBarSize, SCROLL_BAR_SELF_SIZE) << "scrollBarSize: " << scrollBarSize.ToString()
                                                   << " SCROLL_BAR_SELF_SIZE: " << SCROLL_BAR_SELF_SIZE.ToString();
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest022
 * @tc.desc: Test NotifyScrollBarNode
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest022, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::VERTICAL), -1, layoutConstraint);
    RefPtr<ScrollProxy> ScrollProxy = AceType::MakeRefPtr<ScrollBarProxy>();
    EXPECT_NE(ScrollProxy, nullptr);
    auto distance = -1.0;
    auto source = SCROLL_FROM_START;
    auto scrollFunction = [&distance, &source](double parameter1, int32_t parameter2) {
        distance = parameter1;
        source = parameter2;
        return true;
    };
    auto scrollBarProxy = AceType::DynamicCast<ScrollBarProxy>(ScrollProxy);
    EXPECT_NE(scrollBarProxy, nullptr);
    auto scrollPattern1 = AceType::MakeRefPtr<ScrollPattern>();
    EXPECT_NE(scrollPattern1, nullptr);
    auto* scrollRaw1 = AceType::RawPtr(scrollPattern1);
    EXPECT_NE(scrollRaw1, nullptr);
    scrollBarProxy->RegisterScrollableNode({ AceType::WeakClaim(scrollRaw1), std::move(scrollFunction) });
    EXPECT_EQ(scrollBarProxy->scrollableNodes_.size(), 1);
    scrollBarProxy->NotifyScrollBarNode(1.0, SCROLL_FROM_BAR);
    EXPECT_EQ(distance, 1.0);
    EXPECT_EQ(source, SCROLL_FROM_BAR);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest023
 * @tc.desc: Test scrollbar pattern functions
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest023, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::VERTICAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_100);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    pattern_->UpdateScrollBarOffset();
    EXPECT_EQ(pattern_->scrollBar_->paintOffset_, Offset(0.0f, 0.0f));
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_100);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_100);
    pattern_->scrollBar_->displayMode_ = DisplayMode::OFF;
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_100);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_100);
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_100);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_100);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest024
 * @tc.desc: Test UpdateOverlayModifier
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest024, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::VERTICAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto scrollBar = scrollPaint->scrollBar_.Upgrade();
    EXPECT_TRUE(scrollBar->NeedPaint());
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    scrollBar->SetHoverAnimationType(HoverAnimationType::GROW);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::GROW);
    scrollPaint->UpdateOverlayModifier(&paintWrapper);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest063
 * @tc.desc: Test scrollbar pattern functions
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest063, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::VERTICAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_10);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    pattern_->UpdateScrollBarOffset();
    EXPECT_EQ(pattern_->scrollBar_->paintOffset_, Offset(0.0f, 0.0f));
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_10);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_10);
    pattern_->scrollBar_->displayMode_ = DisplayMode::OFF;
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_10);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_10);
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_10);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_10);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest064
 * @tc.desc: Test scrollbar pattern functions
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest064, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::VERTICAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_200);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    pattern_->UpdateScrollBarOffset();
    EXPECT_EQ(pattern_->scrollBar_->paintOffset_, Offset(0.0f, 0.0f));
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_200);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_200);
    pattern_->scrollBar_->displayMode_ = DisplayMode::OFF;
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_200);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_200);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_200);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest065
 * @tc.desc: Test scrollbar pattern functions
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest065, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::HORIZONTAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_100);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    pattern_->UpdateScrollBarOffset();
    EXPECT_EQ(pattern_->scrollBar_->paintOffset_, Offset(0.0f, 0.0f));
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_100);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_100);
    pattern_->scrollBar_->displayMode_ = DisplayMode::OFF;
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_100);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_100);
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_100);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_100);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest066
 * @tc.desc: Test scrollbar pattern functions
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest066, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::HORIZONTAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_10);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    pattern_->UpdateScrollBarOffset();
    EXPECT_EQ(pattern_->scrollBar_->paintOffset_, Offset(0.0f, 0.0f));
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_10);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_10);
    pattern_->scrollBar_->displayMode_ = DisplayMode::OFF;
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_10);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_10);
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_10);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_10);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest067
 * @tc.desc: Test scrollbar pattern functions
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest067, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::HORIZONTAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_200);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    pattern_->UpdateScrollBarOffset();
    EXPECT_EQ(pattern_->scrollBar_->paintOffset_, Offset(0.0f, 0.0f));
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_200);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_200);
    pattern_->scrollBar_->displayMode_ = DisplayMode::OFF;
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_200);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_200);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_200);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest068
 * @tc.desc: Test scrollbar pattern functions
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest068, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::VERTICAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_100);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    pattern_->UpdateScrollBarOffset();
    EXPECT_EQ(pattern_->scrollBar_->paintOffset_, Offset(0.0f, 0.0f));
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_100);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_100);
    pattern_->scrollBar_->displayMode_ = DisplayMode::AUTO;
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_100);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_100);
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_100);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_100);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest069
 * @tc.desc: Test scrollbar pattern functions
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest069, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::VERTICAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_10);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    pattern_->UpdateScrollBarOffset();
    EXPECT_EQ(pattern_->scrollBar_->paintOffset_, Offset(0.0f, 0.0f));
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_10);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_10);
    pattern_->scrollBar_->displayMode_ = DisplayMode::AUTO;
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_10);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_10);
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_10);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_10);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest070
 * @tc.desc: Test scrollbar pattern functions
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest070, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::VERTICAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_200);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    pattern_->UpdateScrollBarOffset();
    EXPECT_EQ(pattern_->scrollBar_->paintOffset_, Offset(0.0f, 0.0f));
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_200);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_200);
    pattern_->scrollBar_->displayMode_ = DisplayMode::AUTO;
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_200);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_200);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_200);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest071
 * @tc.desc: Test scrollbar pattern functions
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest071, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::HORIZONTAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_100);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    pattern_->UpdateScrollBarOffset();
    EXPECT_EQ(pattern_->scrollBar_->paintOffset_, Offset(0.0f, 0.0f));
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_100);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_100);
    pattern_->scrollBar_->displayMode_ = DisplayMode::AUTO;
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_100);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_100);
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_100);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_100);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest072
 * @tc.desc: Test scrollbar pattern functions
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest072, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::HORIZONTAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_10);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    pattern_->UpdateScrollBarOffset();
    EXPECT_EQ(pattern_->scrollBar_->paintOffset_, Offset(0.0f, 0.0f));
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_10);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_10);
    pattern_->scrollBar_->displayMode_ = DisplayMode::AUTO;
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_10);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_10);
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_10);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_10);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest073
 * @tc.desc: Test scrollbar pattern functions
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest073, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    layoutConstraint.selfIdealSize.SetSize(SCROLL_BAR_SELF_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::HORIZONTAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_200);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    auto host = pattern_->GetHost();
    ASSERT_NE(host, nullptr);
    pattern_->UpdateScrollBarOffset();
    EXPECT_EQ(pattern_->scrollBar_->paintOffset_, Offset(0.0f, 0.0f));
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_200);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_200);
    pattern_->scrollBar_->displayMode_ = DisplayMode::AUTO;
    pattern_->SetControlDistance(SCROLL_BAR_FLOAT_200);
    pattern_->SetCurrentPosition(SCROLL_BAR_FLOAT_200);
    pattern_->HandleScrollBarOutBoundary(SCROLL_BAR_FLOAT_200);
    EXPECT_EQ(pattern_->scrollBar_->outBoundary_, SCROLL_BAR_FLOAT_200);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest074
 * @tc.desc: Test UpdateOverlayModifier
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest074, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::HORIZONTAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto scrollBar = scrollPaint->scrollBar_.Upgrade();
    EXPECT_TRUE(scrollBar->NeedPaint());
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    scrollBar->SetHoverAnimationType(HoverAnimationType::GROW);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::GROW);
    scrollPaint->UpdateOverlayModifier(&paintWrapper);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest075
 * @tc.desc: Test UpdateOverlayModifier
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest075, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::FREE), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto scrollBar = scrollPaint->scrollBar_.Upgrade();
    EXPECT_TRUE(scrollBar->NeedPaint());
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    scrollBar->SetHoverAnimationType(HoverAnimationType::GROW);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::GROW);
    scrollPaint->UpdateOverlayModifier(&paintWrapper);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest076
 * @tc.desc: Test UpdateOverlayModifier
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest076, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::NONE), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto scrollBar = scrollPaint->scrollBar_.Upgrade();
    EXPECT_TRUE(scrollBar->NeedPaint());
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    scrollBar->SetHoverAnimationType(HoverAnimationType::GROW);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::GROW);
    scrollPaint->UpdateOverlayModifier(&paintWrapper);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest077
 * @tc.desc: Test UpdateOverlayModifier
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest077, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::VERTICAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto scrollBar = scrollPaint->scrollBar_.Upgrade();
    EXPECT_TRUE(scrollBar->NeedPaint());
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    scrollBar->SetHoverAnimationType(HoverAnimationType::GROW);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::GROW);
    scrollPaint->UpdateOverlayModifier(&paintWrapper);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest078
 * @tc.desc: Test UpdateOverlayModifier
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest078, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::HORIZONTAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto scrollBar = scrollPaint->scrollBar_.Upgrade();
    EXPECT_TRUE(scrollBar->NeedPaint());
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    scrollBar->SetHoverAnimationType(HoverAnimationType::SHRINK);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::SHRINK);
    scrollPaint->UpdateOverlayModifier(&paintWrapper);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest79
 * @tc.desc: Test UpdateOverlayModifier
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest79, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::FREE), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto scrollBar = scrollPaint->scrollBar_.Upgrade();
    EXPECT_TRUE(scrollBar->NeedPaint());
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    scrollBar->SetHoverAnimationType(HoverAnimationType::SHRINK);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::SHRINK);
    scrollPaint->UpdateOverlayModifier(&paintWrapper);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest80
 * @tc.desc: Test UpdateOverlayModifier
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest80, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::NONE), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto scrollBar = scrollPaint->scrollBar_.Upgrade();
    EXPECT_TRUE(scrollBar->NeedPaint());
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    scrollBar->SetHoverAnimationType(HoverAnimationType::SHRINK);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::SHRINK);
    scrollPaint->UpdateOverlayModifier(&paintWrapper);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest081
 * @tc.desc: Test UpdateOverlayModifier
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest081, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::HORIZONTAL), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto scrollBar = scrollPaint->scrollBar_.Upgrade();
    EXPECT_TRUE(scrollBar->NeedPaint());
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    scrollBar->SetHoverAnimationType(HoverAnimationType::NONE);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    scrollPaint->UpdateOverlayModifier(&paintWrapper);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest82
 * @tc.desc: Test UpdateOverlayModifier
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest82, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::FREE), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto scrollBar = scrollPaint->scrollBar_.Upgrade();
    EXPECT_TRUE(scrollBar->NeedPaint());
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    scrollBar->SetHoverAnimationType(HoverAnimationType::NONE);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    scrollPaint->UpdateOverlayModifier(&paintWrapper);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest083
 * @tc.desc: Test UpdateOverlayModifier
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest083, TestSize.Level1)
{
    int32_t settingApiVersion = 12;
    int32_t backupApiVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(settingApiVersion);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper paintWrapper(nullptr, geometryNode, paintProperty_);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.maxSize = CONTAINER_SIZE;
    layoutConstraint.parentIdealSize.SetSize(CONTAINER_SIZE);
    CreateScrollBarWithoutSubComponent(true, false, static_cast<int>(Axis::NONE), -1, layoutConstraint);
    pattern_->scrollBar_->SetScrollable(true);
    auto paint = pattern_->CreateNodePaintMethod();
    auto scrollPaint = AceType::DynamicCast<ScrollBarPaintMethod>(paint);
    auto scrollBar = scrollPaint->scrollBar_.Upgrade();
    EXPECT_TRUE(scrollBar->NeedPaint());
    auto modifier = scrollPaint->GetOverlayModifier(&paintWrapper);
    auto scrollBarOverlayModifier = AceType::DynamicCast<ScrollBarOverlayModifier>(modifier);
    ASSERT_NE(scrollBarOverlayModifier, nullptr);
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    scrollBar->SetHoverAnimationType(HoverAnimationType::NONE);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    scrollPaint->UpdateOverlayModifier(&paintWrapper);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    Container::Current()->SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: ScrollBarTest084
 * @tc.desc: Test UpdateOverlayModifier
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTestNg, ScrollBarTest084, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create scrollbar without child
     */
    const int32_t apiTargetVersion = Container::Current()->GetApiTargetVersion();
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    int32_t directionValue = static_cast<int32_t>(Axis::VERTICAL);
    int32_t stateValue = static_cast<int32_t>(DisplayMode::ON);
    CreateScrollBarWithoutChild(true, false, directionValue, stateValue);

    auto scrollableEvent = pattern_->scrollableEvent_;
    auto scrollBar = pattern_->scrollBar_;
    scrollBar->SetScrollable(true);
    pattern_->CreateScrollBarOverlayModifier();
    EXPECT_FALSE(scrollableEvent->InBarRegion(PointF(1.f, 1.f), SourceType::MOUSE));
    EXPECT_FALSE(scrollableEvent->InBarRegion(PointF(1.f, 1.f), SourceType::TOUCH));
    EXPECT_EQ(scrollBar->displayMode_, DisplayMode::ON);

    GetEventTargetImpl GetEventTargetImpl;
    TouchTestResult result;
    ResponseLinkResult responseLinkResult;
    scrollableEvent->BarCollectTouchTarget(
        OffsetF(1.f, 1.f), GetEventTargetImpl, result, frameNode_, nullptr, responseLinkResult);
    EXPECT_EQ(result.size(), 1);

    /**
     * @tc.steps: step2. set displayMode DisplayMode::AUTO
     * @tc.expected: opacity is UINT8_MAX.
     */
    pattern_->SetScrollBar(DisplayMode::AUTO);
    EXPECT_EQ(pattern_->scrollBarOverlayModifier_->GetOpacity(), UINT8_MAX);

    /**
     * @tc.steps: step3. set displayMode DisplayMode::OFF
     * @tc.expected: opacity is 0.
     */
    pattern_->SetScrollBar(DisplayMode::OFF);
    EXPECT_EQ(pattern_->scrollBarOverlayModifier_->GetOpacity(), 0);
    Container::Current()->SetApiTargetVersion(apiTargetVersion);
}
} // namespace OHOS::Ace::NG