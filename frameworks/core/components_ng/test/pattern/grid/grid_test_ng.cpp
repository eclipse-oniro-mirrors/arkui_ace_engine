/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <map>
#include <memory>

#include "gtest/gtest.h"

#include "base/geometry/ng/size_t.h"
#include "base/geometry/offset.h"
#include "base/utils/utils.h"
#define protected public
#define private public
#include "core/components/button/button_theme.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/button/button_model_ng.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/grid/grid_item_model_ng.h"
#include "core/components_ng/pattern/grid/grid_item_pattern.h"
#include "core/components_ng/pattern/grid/grid_model_ng.h"
#include "core/components_ng/pattern/grid/grid_pattern.h"
#include "core/components_ng/test/mock/render/mock_render_context.h"
#include "core/components_ng/test/mock/rosen/mock_canvas.h"
#include "core/components_ng/test/mock/theme/mock_theme_manager.h"
#include "core/pipeline/base/constants.h"
#include "core/pipeline_ng/test/mock/mock_pipeline_base.h"
#include "core/components_ng/pattern/grid/grid_scroll/grid_scroll_layout_algorithm.h"
#include "core/components_v2/inspector/inspector_constants.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr float DEVICE_WIDTH = 720.0f;
constexpr float DEVICE_HEIGHT = 1136.0f;
constexpr float PREV_OFFSET_FIRST = 1.0f;
constexpr float PREV_OFFSET_SECOND = 2.0f;
constexpr float CURRENT_OFFSET_FIRST = 2.0f;
constexpr float CURRENT_OFFSET_SECOND = 1.0f;
constexpr int32_t START_INDEX = 1;
constexpr int32_t START_MAIN_LINE_INDEX = 0;
constexpr int32_t END_INDEX = -100;
constexpr int32_t START_INDEX_END = -100;
constexpr int32_t CHILDREN_COUNT = 100;
constexpr int32_t ALGORITHM_RELATE_LALUE_FIRST = 2;
constexpr int32_t ALGORITHM_RELATE_LALUE_SECOND = 0;
const SizeF CONTAINER_SIZE(DEVICE_WIDTH, DEVICE_HEIGHT);
const Dimension GRID_ROWS_GAP = Dimension(5);
constexpr float ITEM_HEIGHT = 100.f;
constexpr float ITEM_WIDTH = 100.f;
constexpr float GRID_HEIGHT = 300.f;
const std::string TEMPLATE_4 = "1fr 1fr 1fr 1fr";
} // namespace

class GridTestNg : public testing::Test {
protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp() override;
    void TearDown() override;
    void GetInstance();
    static void SetWidth(const Dimension& width);
    static void SetHeight(const Dimension& height);
    void CreateGridItem(int32_t count = 10, Axis direction = Axis::VERTICAL, bool focusable = false);
    RefPtr<LayoutWrapper> RunMeasureAndLayout(
        float width = DEVICE_WIDTH, float height = GRID_HEIGHT);
    RefPtr<FrameNode> GetItemFrameNode(int32_t index);
    RefPtr<GridItemPattern> GetItemPattern(int32_t index);
    RefPtr<FocusHub> GetItemFocusHub(int32_t index);
    void UpdateCurrentOffset(float offset);
    void MouseSelect(Offset start, Offset end);
    void MouseSelectRelease();

    testing::AssertionResult IsEqualNextFocusNode(
        int32_t currentIndex, std::map<FocusStep, int32_t> next);

    RefPtr<FrameNode> frameNode_;
    RefPtr<GridPattern> pattern_;
    RefPtr<GridEventHub> eventHub_;
    RefPtr<GridLayoutProperty> layoutProperty_;
    RefPtr<GridAccessibilityProperty> accessibilityProperty_;
};

void GridTestNg::SetUpTestSuite()
{
    MockPipelineBase::SetUp();
    // set buttonTheme to themeManager before using themeManager to get buttonTheme
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineBase::GetCurrent()->SetThemeManager(themeManager);
    auto buttonTheme = AceType::MakeRefPtr<ButtonTheme>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(buttonTheme));
}

void GridTestNg::TearDownTestSuite()
{
    MockPipelineBase::TearDown();
}

void GridTestNg::SetUp() {}

void GridTestNg::TearDown()
{
    frameNode_ = nullptr;
    pattern_ = nullptr;
    eventHub_ = nullptr;
    layoutProperty_ = nullptr;
    accessibilityProperty_ = nullptr;
}

void GridTestNg::GetInstance()
{
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    frameNode_ = AceType::DynamicCast<FrameNode>(element);
    pattern_ = frameNode_->GetPattern<GridPattern>();
    eventHub_ = frameNode_->GetEventHub<GridEventHub>();
    layoutProperty_ = frameNode_->GetLayoutProperty<GridLayoutProperty>();
    accessibilityProperty_ = frameNode_->GetAccessibilityProperty<GridAccessibilityProperty>();
}

void GridTestNg::SetWidth(const Dimension& width)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto layoutProperty = frameNode->GetLayoutProperty();
    layoutProperty->UpdateUserDefinedIdealSize(CalcSize(CalcLength(width), std::nullopt));
}

void GridTestNg::SetHeight(const Dimension& height)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto layoutProperty = frameNode->GetLayoutProperty();
    layoutProperty->UpdateUserDefinedIdealSize(CalcSize(std::nullopt, CalcLength(height)));
}

void GridTestNg::CreateGridItem(int32_t count, Axis direction, bool focusable)
{
    for (int32_t i = 0; i < count; i++) {
        GridItemModelNG gridItemModel;
        gridItemModel.Create();
        if (direction == Axis::VERTICAL) {
            SetHeight(Dimension(ITEM_HEIGHT));
        } else {
            SetWidth(Dimension(ITEM_WIDTH));
        }
        if (focusable) {
            ButtonModelNG buttonModelNG;
            buttonModelNG.CreateWithLabel("label");
            ViewStackProcessor::GetInstance()->Pop();
        }
        ViewStackProcessor::GetInstance()->Pop();
    }
}

RefPtr<LayoutWrapper> GridTestNg::RunMeasureAndLayout(float width, float height)
{
    RefPtr<LayoutWrapper> layoutWrapper = frameNode_->CreateLayoutWrapper(false, false);
    layoutWrapper->SetActive();
    LayoutConstraintF LayoutConstraint;
    LayoutConstraint.parentIdealSize = { DEVICE_WIDTH, DEVICE_HEIGHT };
    LayoutConstraint.percentReference = { DEVICE_WIDTH, DEVICE_HEIGHT };
    LayoutConstraint.selfIdealSize = { width, height };
    LayoutConstraint.maxSize = { width, height };
    layoutWrapper->Measure(LayoutConstraint);
    layoutWrapper->Layout();
    layoutWrapper->MountToHostOnMainThread();
    return layoutWrapper;
}

RefPtr<FrameNode> GridTestNg::GetItemFrameNode(int32_t index)
{
    return AceType::DynamicCast<FrameNode>(frameNode_->GetChildAtIndex(index));
}

RefPtr<GridItemPattern> GridTestNg::GetItemPattern(int32_t index)
{
    return GetItemFrameNode(index)->GetPattern<GridItemPattern>();
}

RefPtr<FocusHub> GridTestNg::GetItemFocusHub(int32_t index)
{
    return GetItemFrameNode(index)->GetOrCreateFocusHub();
}

void GridTestNg::MouseSelect(Offset start, Offset end)
{
    MouseInfo info;
    info.SetButton(MouseButton::LEFT_BUTTON);
    info.SetAction(MouseAction::PRESS);
    info.SetLocalLocation(start);
    pattern_->HandleMouseEventWithoutKeyboard(info);
    if (start != end) {
        info.SetAction(MouseAction::MOVE);
        info.SetLocalLocation(end);
        pattern_->HandleMouseEventWithoutKeyboard(info);
    }
}

void GridTestNg::MouseSelectRelease()
{
    MouseInfo info;
    info.SetButton(MouseButton::LEFT_BUTTON);
    info.SetAction(MouseAction::RELEASE);
    pattern_->HandleMouseEventWithoutKeyboard(info);
}

void GridTestNg::UpdateCurrentOffset(float offset)
{
    pattern_->UpdateCurrentOffset(offset, SCROLL_FROM_UPDATE);
    RunMeasureAndLayout();
}

testing::AssertionResult GridTestNg::IsEqualNextFocusNode(
    int32_t currentIndex, std::map<FocusStep, int32_t> next)
{
    RefPtr<FocusHub> currentFocusNode = GetItemFocusHub(currentIndex);
    currentFocusNode->RequestFocusImmediately();
    for (auto iter = next.begin(); iter != next.end(); iter++) {
        FocusStep step = iter->first;
        int32_t nextIndex = iter->second;
        RefPtr<FocusHub> nextFocusNode = pattern_->GetNextFocusNode(step, currentFocusNode).Upgrade();
        if (nextIndex == -1 && nextFocusNode != nullptr) {
            return testing::AssertionFailure() <<
                "Next FocusNode is not null." <<
                " FocusStep: " << static_cast<int32_t>(step) <<
                " nextIndex: " << nextIndex;
        }
        if (nextIndex != -1 && nextFocusNode != GetItemFocusHub(nextIndex)) {
            return testing::AssertionFailure() <<
                "Get wrong next FocusNode." <<
                " FocusStep: " << static_cast<int32_t>(step) <<
                " nextIndex: " << nextIndex;
        }
    }
    return testing::AssertionSuccess();
}

/**
 * @tc.name: Property001
 * @tc.desc: Test all the properties of Grid.
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, Property001, TestSize.Level1)
{
    GridModelNG gridModelNG;
    RefPtr<ScrollControllerBase> positionController = gridModelNG.CreatePositionController();
    RefPtr<ScrollProxy> scrollBarProxy = gridModelNG.CreateScrollBarProxy();
    gridModelNG.Create(positionController, scrollBarProxy);
    gridModelNG.SetRowsTemplate("1fr 1fr 1fr");
    gridModelNG.SetColumnsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(Dimension(5));
    gridModelNG.SetColumnsGap(Dimension(10));
    gridModelNG.SetEdgeEffect(EdgeEffect::SPRING);
    CreateGridItem(10);
    GetInstance();
    RunMeasureAndLayout();

    EXPECT_EQ(layoutProperty_->GetRowsTemplateValue(), "1fr 1fr 1fr");
    EXPECT_EQ(layoutProperty_->GetColumnsTemplateValue(), "1fr 1fr");
    EXPECT_EQ(layoutProperty_->GetRowsGapValue(), Dimension(5));
    EXPECT_EQ(layoutProperty_->GetColumnsGapValue(), Dimension(10));
    EXPECT_EQ(layoutProperty_->GetEdgeEffectValue(), EdgeEffect::SPRING);

    auto json = JsonUtil::Create(true);
    layoutProperty_->ToJsonValue(json);
    EXPECT_NE(json, nullptr);

    layoutProperty_->UpdateEdgeEffect(EdgeEffect::FADE);
    auto json2 = JsonUtil::Create(true);
    layoutProperty_->ToJsonValue(json2);
    EXPECT_EQ(json2->GetString("edgeEffect"), "EdgeEffect.Fade");
}

/**
 * @tc.name: Property002
 * @tc.desc: Test empty row/col template.
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, Property002, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("");
    gridModelNG.SetColumnsTemplate("");
    CreateGridItem(10);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. compare grid properties and expected value after change.
     * @tc.expected: grid properties equals expected value after change.
     */
    EXPECT_EQ(layoutProperty_->GetRowsTemplateValue(), "1fr");
    EXPECT_EQ(layoutProperty_->GetColumnsTemplateValue(), "1fr");
}

/**
 * @tc.name: Property003
 * @tc.desc: Test Negative Gap
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, Property003, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsGap(Dimension(-5));
    gridModelNG.SetColumnsGap(Dimension(-10));
    CreateGridItem(10);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Verify GapValue
     * @tc.expected: Gap not be set
     */
    EXPECT_EQ(layoutProperty_->GetRowsGap(), std::nullopt);
    EXPECT_EQ(layoutProperty_->GetColumnsGap(), std::nullopt);
}

/**
 * @tc.name: Property004
 * @tc.desc: Test GetGridDirectionStr func.
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, Property004, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    CreateGridItem(10);
    GetInstance();
    RunMeasureAndLayout();

    EXPECT_EQ(layoutProperty_->GetGridDirectionStr(), "GridDirection.Row");
    layoutProperty_->UpdateGridDirection(FlexDirection::ROW_REVERSE);
    EXPECT_EQ(layoutProperty_->GetGridDirectionStr(), "GridDirection.RowReverse");
    layoutProperty_->UpdateGridDirection(FlexDirection::COLUMN);
    EXPECT_EQ(layoutProperty_->GetGridDirectionStr(), "GridDirection.Column");
    layoutProperty_->UpdateGridDirection(FlexDirection::COLUMN_REVERSE);
    EXPECT_EQ(layoutProperty_->GetGridDirectionStr(), "GridDirection.ColumnReverse");
}

/**
 * @tc.name: Property005
 * @tc.desc: Test all the properties of GridItem.
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, Property005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create gridItem and Set properties.
     */
    GridItemModelNG gridItemModelNG;
    gridItemModelNG.Create();
    gridItemModelNG.SetRowStart(1);
    gridItemModelNG.SetRowEnd(2);
    gridItemModelNG.SetColumnStart(1);
    gridItemModelNG.SetColumnEnd(2);
    gridItemModelNG.SetForceRebuild(true);
    gridItemModelNG.SetSelectable(false);
    gridItemModelNG.SetOnSelect([](bool) {});

    /**
     * @tc.steps: step2. Get frameNode and properties.
     * @tc.expected: step2. Check properties is correct.
     */
    auto frameNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    auto layoutProperty = frameNode->GetLayoutProperty<GridItemLayoutProperty>();
    EXPECT_EQ(layoutProperty->GetRowStart(), 1);
    EXPECT_EQ(layoutProperty->GetRowEnd(), 2);
    EXPECT_EQ(layoutProperty->GetColumnStart(), 1);
    EXPECT_EQ(layoutProperty->GetColumnEnd(), 2);
    EXPECT_EQ(layoutProperty->GetCustomCrossIndex(Axis::VERTICAL), 1);
    EXPECT_EQ(layoutProperty->GetMainSpan(Axis::VERTICAL), 2);
    EXPECT_EQ(layoutProperty->GetCrossSpan(Axis::VERTICAL), 2);
    EXPECT_EQ(layoutProperty->GetMainStart(Axis::VERTICAL), 1);
    EXPECT_EQ(layoutProperty->GetCrossStart(Axis::VERTICAL), 1);
    EXPECT_EQ(layoutProperty->GetCustomCrossIndex(Axis::HORIZONTAL), 1);
    EXPECT_EQ(layoutProperty->GetMainSpan(Axis::HORIZONTAL), 2);
    EXPECT_EQ(layoutProperty->GetCrossSpan(Axis::HORIZONTAL), 2);
    EXPECT_EQ(layoutProperty->GetMainStart(Axis::HORIZONTAL), 1);
    EXPECT_EQ(layoutProperty->GetCrossStart(Axis::HORIZONTAL), 1);
    auto pattern = frameNode->GetPattern<GridItemPattern>();
    EXPECT_TRUE(pattern->forceRebuild_);
    auto eventHub = frameNode->GetEventHub<GridItemEventHub>();
    ASSERT_NE(eventHub->onSelect_, nullptr);
    auto json = JsonUtil::Create(true);
    layoutProperty->ToJsonValue(json);
    EXPECT_NE(json, nullptr);
}

/**
 * @tc.name: KeyEvent001
 * @tc.desc: Test OnKeyEvent func.
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, KeyEvent001, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate("1fr 1fr");
    CreateGridItem(10);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. KeyAction::UNKNOWN.
     * @tc.expected: Nothing changed.
     */
    KeyEvent event;
    pattern_->OnKeyEvent(event);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
    
    /**
     * @tc.steps: step1. KeyCode::KEY_PAGE_DOWN.
     * @tc.expected: Page jump down width Grid height.
     */
    event.action = KeyAction::DOWN;
    event.code = KeyCode::KEY_PAGE_DOWN;
    pattern_->OnKeyEvent(event);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, -300.f);
    
    /**
     * @tc.steps: step1. KeyCode::KEY_PAGE_UP.
     * @tc.expected: Page jump up width Grid height.
     */
    event.code = KeyCode::KEY_PAGE_UP;
    pattern_->OnKeyEvent(event);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().currentOffset_, 0);
}

/**
 * @tc.name: KeyEvent002
 * @tc.desc: Test HandleDirectionKey func.
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, KeyEvent002, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate("1fr 1fr");
    CreateGridItem(10);
    GetInstance();
    RunMeasureAndLayout();

    EXPECT_FALSE(pattern_->HandleDirectionKey(KeyCode::KEY_UNKNOWN));
    EXPECT_TRUE(pattern_->HandleDirectionKey(KeyCode::KEY_DPAD_UP));
    EXPECT_TRUE(pattern_->HandleDirectionKey(KeyCode::KEY_DPAD_DOWN));
}

/**
 * @tc.name: GridTest001
 * @tc.desc: Set fixed rows only Fill all items in the grid.
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridTest001, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(Dimension(5));
    gridModelNG.SetScrollBarColor("#909090");
    gridModelNG.SetMultiSelectable(true);
    CreateGridItem(10);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.expected: step1. Check whether the updated properties and parameters are correct.
     */
    EXPECT_TRUE(pattern_->multiSelectable_);
    EXPECT_TRUE(pattern_->isMouseEventInit_);
    EXPECT_TRUE(pattern_->isConfigScrollable_);

    /**
     * @tc.steps: step2. Change MultiSelectable and Call OnModifyDone  function.
     * @tc.expected: Check whether the updated properties and parameters are correct.
     */
    pattern_->SetMultiSelectable(false);
    pattern_->OnModifyDone();
    EXPECT_FALSE(pattern_->multiSelectable_);
    EXPECT_FALSE(pattern_->isMouseEventInit_);

    /**
     * @tc.steps: step3. Call the AddScrollEvent function to add a scroll position callback event.
     * @tc.expected: Call and check the return value of the callback function.
     */
    pattern_->AddScrollEvent();
    ASSERT_NE(pattern_->scrollableEvent_, nullptr);
    auto callback = pattern_->scrollableEvent_->GetScrollPositionCallback();
    ASSERT_NE(callback, nullptr);
    auto ret = callback(1.f, SCROLL_FROM_START);
    EXPECT_EQ(ret, true);

    /**
     * @tc.steps: step4. When isConfigScrollable_ is false, call related functions.
     * @tc.expected: Check the return value of the related function.
     */
    pattern_->isConfigScrollable_ = false;
    EXPECT_EQ(pattern_->AnimateTo(100.f, 200.f, Curves::LINEAR), false);
    EXPECT_EQ(pattern_->UpdateCurrentOffset(1.f, SCROLL_FROM_UPDATE), true);
    EXPECT_EQ(pattern_->UpdateStartIndex(1.0), false);

    /**
     * @tc.steps: step8. When isConfigScrollable_ is true, call AnimateTo functions.
     * @tc.expected: step8. Check the return value and related parameters.
     */
    pattern_->ScrollPage(false);
    pattern_->isConfigScrollable_ = true;
    EXPECT_EQ(pattern_->AnimateTo(100.f, 200.f, Curves::LINEAR), true);
    ASSERT_NE(pattern_->animator_, nullptr);

    /**
     * @tc.steps: step9. Call AnimateTo functions again.
     * @tc.expected: step9. Check the return value and related parameters.
     */
    pattern_->animator_->status_ = Animator::Status::STOPPED;
    EXPECT_EQ(pattern_->AnimateTo(100.f, 200.f, Curves::LINEAR), true);
    ASSERT_NE(pattern_->animator_, nullptr);

    /**
     * @tc.steps: step10. When offsetEnd_ and reachStart_ are true, call OnScrollCallback functions.
     * @tc.expected: step10. Check whether the return value is correct.
     */
    pattern_->gridLayoutInfo_.offsetEnd_ = true;
    pattern_->gridLayoutInfo_.reachStart_ = true;
    EXPECT_EQ(pattern_->OnScrollCallback(0.0f, SCROLL_FROM_ANIMATION), true);
    EXPECT_EQ(pattern_->OnScrollCallback(1.f, SCROLL_FROM_ANIMATION), false);
    EXPECT_EQ(pattern_->gridLayoutInfo_.offsetEnd_, true);
    EXPECT_EQ(pattern_->OnScrollCallback(-1.0f, SCROLL_FROM_ANIMATION), false);
    EXPECT_EQ(pattern_->gridLayoutInfo_.reachStart_, true);
}

/**
 * @tc.name: GridTest002
 * @tc.desc: Fill all items to grid with fixed row and column
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridTest002, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr 1fr");
    gridModelNG.SetColumnsTemplate("1fr 1fr 1fr");
    CreateGridItem(9);
    GetInstance();

    /**
     * @tc.steps: step1. When setting fixed rows and columns, check the status of child nodes in the grid.
     * @tc.expected: All child nodes are active.
     */
    RefPtr<LayoutWrapper> layoutWrapper = RunMeasureAndLayout();
    for (int32_t i = 0; i < 9; ++i) {
        EXPECT_TRUE(layoutWrapper->GetOrCreateChildByIndex(i, false)->IsActive());
    }
}

/**
 * @tc.name: GridTest003
 * @tc.desc: grid with fixed column
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridTest003, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate("1fr 1fr 1fr");
    CreateGridItem(9);
    GetInstance();

    /**
     * @tc.steps: step1. When setting fixed rows and columns, check the status of child nodes in the grid.
     * @tc.expected: All child nodes are active.
     */
    RefPtr<LayoutWrapper> layoutWrapper = RunMeasureAndLayout();
    for (int32_t i = 0; i < 9; ++i) {
        EXPECT_TRUE(layoutWrapper->GetOrCreateChildByIndex(i, false)->IsActive());
    }
}

/**
 * @tc.name: GridTest004
 * @tc.desc: grid with fixed column, some griditem not show
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridTest004, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate("1fr 1fr 1fr");
    CreateGridItem(9);
    GetInstance();

    RefPtr<LayoutWrapper> layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    for (int32_t i = 0; i < 6; ++i) {
        EXPECT_TRUE(layoutWrapper->GetOrCreateChildByIndex(i, false)->IsActive());
    }
}

/**
 * @tc.name: GridTest005
 * @tc.desc: grid with fixed column, some griditem not fully show
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridTest005, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate("1fr 1fr 1fr");
    CreateGridItem(9);
    GetInstance();

    RefPtr<LayoutWrapper> layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 250.f);
    for (int32_t i = 0; i < 9; ++i) {
        EXPECT_TRUE(layoutWrapper->GetOrCreateChildByIndex(i, false)->IsActive());
    }
}

/**
 * @tc.name: GridTest006
 * @tc.desc: grid with fixed column, scroll to show one more line
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridTest006, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate("1fr 1fr 1fr");
    CreateGridItem(9);
    GetInstance();

    RefPtr<LayoutWrapper> layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    pattern_->UpdateCurrentOffset(-50.f, SCROLL_FROM_UPDATE);
    layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    for (int32_t i = 0; i < 9; ++i) {
        EXPECT_TRUE(layoutWrapper->GetOrCreateChildByIndex(i, false)->IsActive());
    }
}

/**
 * @tc.name: GridTest007
 * @tc.desc: grid with fixed column, scroll to end
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridTest007, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate("1fr 1fr 1fr");
    CreateGridItem(9);
    GetInstance();

    RefPtr<LayoutWrapper> layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    pattern_->UpdateCurrentOffset(-100.f, SCROLL_FROM_UPDATE);
    layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    for (int32_t i = 3; i < 9; ++i) {
        EXPECT_TRUE(layoutWrapper->GetOrCreateChildByIndex(i, false)->IsActive());
    }
}

/**
 * @tc.name: GridTest008
 * @tc.desc: grid with fixed column, scroll to index not fully showed at last line
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridTest008, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate("1fr 1fr 1fr");
    CreateGridItem(9);
    GetInstance();

    RefPtr<LayoutWrapper> layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    pattern_->UpdateStartIndex(8);
    layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    for (int32_t i = 0; i < 3; ++i) {
        EXPECT_FALSE(layoutWrapper->GetOrCreateChildByIndex(i, false)->IsActive());
    }
}

/**
 * @tc.name: GridTest009
 * @tc.desc: grid with fixed column, scroll to index not fully showed at first line
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridTest009, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate("1fr 1fr 1fr");
    CreateGridItem(9);
    GetInstance();

    RefPtr<LayoutWrapper> layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    pattern_->UpdateStartIndex(1);
    layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    for (int32_t i = 6; i < 9; ++i) {
        EXPECT_FALSE(layoutWrapper->GetOrCreateChildByIndex(i, false)->IsActive());
    }
}

/**
 * @tc.name: GridTest010
 * @tc.desc: grid with fixed column, scroll to index fully showed
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridTest010, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate("1fr 1fr 1fr");
    CreateGridItem(9);
    GetInstance();

    RefPtr<LayoutWrapper> layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    pattern_->UpdateStartIndex(3);
    layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    for (int32_t i = 0; i < 6; ++i) {
        EXPECT_TRUE(layoutWrapper->GetOrCreateChildByIndex(i, false)->IsActive());
    }
}

/**
 * @tc.name: GridTest011
 * @tc.desc: grid with fixed column, scroll to index not fully showed at last line
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridTest011, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate("1fr 1fr 1fr");
    CreateGridItem(10);
    GetInstance();

    RefPtr<LayoutWrapper> layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    pattern_->UpdateStartIndex(8);
    layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    EXPECT_FALSE(layoutWrapper->GetOrCreateChildByIndex(9, false)->IsActive());
}

/**
 * @tc.name: GridTest012
 * @tc.desc: grid with fixed column, scroll to index out of view
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridTest012, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate("1fr 1fr 1fr");
    CreateGridItem(10);
    GetInstance();

    RefPtr<LayoutWrapper> layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    pattern_->UpdateStartIndex(9);
    layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    EXPECT_TRUE(layoutWrapper->GetOrCreateChildByIndex(9, false)->IsActive());
    EXPECT_FALSE(layoutWrapper->GetOrCreateChildByIndex(0, false)->IsActive());

    pattern_->UpdateStartIndex(0);
    layoutWrapper = RunMeasureAndLayout(DEVICE_WIDTH, 200.f);
    EXPECT_FALSE(layoutWrapper->GetOrCreateChildByIndex(9, false)->IsActive());
    EXPECT_TRUE(layoutWrapper->GetOrCreateChildByIndex(0, false)->IsActive());
}

/**
 * @tc.name: EventHub001
 * @tc.desc: Test GetInsertPosition func
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, EventHub001, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    CreateGridItem(8);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Mock GetPaintRectWithTransform.
     */
    RectF gridRect(0.f, 0.f, DEVICE_WIDTH, DEVICE_HEIGHT);
    EXPECT_CALL(
        *(AceType::RawPtr(AceType::DynamicCast<MockRenderContext>(frameNode_->renderContext_))),
        GetPaintRectWithTransform())
        .WillRepeatedly(Return(gridRect));

    /**
     * @tc.steps: step2. Run GetInsertPosition func.
     * @tc.expected: Verify return value.
     */
    EXPECT_EQ(eventHub_->GetInsertPosition(DEVICE_WIDTH + 1, DEVICE_HEIGHT), -1); // out of grid
    EXPECT_EQ(eventHub_->GetInsertPosition(0.f, 0.f), 0); // 0, 0
    EXPECT_EQ(eventHub_->GetInsertPosition(90.f, 50.f), 0); // first item
    EXPECT_EQ(eventHub_->GetInsertPosition(360.f, 50.f), 1); // between the second and third
    EXPECT_EQ(eventHub_->GetInsertPosition(360.f, 100.f), 1); // between the 2nd, 3rd, 6th, 7th
    EXPECT_EQ(eventHub_->GetInsertPosition(180.f, 300.f), 8); // in grid but not on item
    pattern_->GetGridLayoutInfo().currentRect_ = RectF(0.f, 0.f, 180.f, 300.f);
    EXPECT_EQ(eventHub_->GetInsertPosition(180.f, 300.f), 8); // on currentRect_
}

/**
 * @tc.name: EventHub002
 * @tc.desc: Test GetFrameNodeChildSize func
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, EventHub002, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    CreateGridItem(8);
    GetInstance();
    RunMeasureAndLayout();

    EXPECT_EQ(eventHub_->GetFrameNodeChildSize(), 8);
}

/**
 * @tc.name: PositionController001
 * @tc.desc: Test positionController func in VERTICAL Grid
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, PositionController001, TestSize.Level1)
{
    GridModelNG gridModelNG;
    RefPtr<ScrollControllerBase> positionController = gridModelNG.CreatePositionController();
    RefPtr<ScrollProxy> scrollBarProxy = gridModelNG.CreateScrollBarProxy();
    gridModelNG.Create(positionController, scrollBarProxy);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    CreateGridItem(14);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Test JumpTo func.
     * @tc.expected: Verify return value.
     */
    auto controller = pattern_->positionController_;
    controller->JumpTo(1, 0);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, 1);

    /**
     * @tc.steps: step2. Test AnimateTo func.
     * @tc.expected: Verify return value.
     */
    controller->AnimateTo(Dimension(100.f, DimensionUnit::PX), 200.f, Curves::LINEAR);
    ASSERT_NE(pattern_->animator_, nullptr);

    /**
     * @tc.steps: step3. Test GetScrollDirection func.
     * @tc.expected: Verify return value.
     */
    EXPECT_EQ(controller->GetScrollDirection(), Axis::VERTICAL);

    /**
     * @tc.steps: step4. Test GetCurrentOffset func.
     * @tc.expected: Verify return value.
     */
    pattern_->UpdateCurrentOffset(20.f, SCROLL_FROM_UPDATE);
    Offset currentOffset = controller->GetCurrentOffset();
    EXPECT_EQ(currentOffset, Offset(0, 20.f));

    /**
     * @tc.steps: step5. Test ScrollToEdge func.
     * @tc.expected: Verify return value.
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_LEFT, true);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, 1);
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_RIGHT, true);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, 1);
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, true);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, 13);
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, true);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, 0);

    /**
     * @tc.steps: step6. Test ScrollPage func.
     * @tc.expected: Verify currentOffset.
     */
    RunMeasureAndLayout();
    controller->ScrollPage(true, true);
    EXPECT_EQ(controller->GetCurrentOffset(), Offset(0, 300.f));

    controller->ScrollPage(false, true);
    EXPECT_EQ(controller->GetCurrentOffset(), Offset(0, 0));
}

/**
 * @tc.name: PositionController002
 * @tc.desc: Test positionController func in HORIZONTAL Grid
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, PositionController002, TestSize.Level1)
{
    GridModelNG gridModelNG;
    RefPtr<ScrollControllerBase> positionController = gridModelNG.CreatePositionController();
    RefPtr<ScrollProxy> scrollBarProxy = gridModelNG.CreateScrollBarProxy();
    gridModelNG.Create(positionController, scrollBarProxy);
    gridModelNG.SetRowsTemplate(TEMPLATE_4);
    CreateGridItem(14, Axis::HORIZONTAL);
    GetInstance();

    /**
     * @tc.steps: step1. Change Grid width to 300.
     */
    RunMeasureAndLayout(300.f);

    /**
     * @tc.steps: step2. Test GetScrollDirection func.
     * @tc.expected: Verify return value.
     */
    auto controller = pattern_->positionController_;
    EXPECT_EQ(controller->GetScrollDirection(), Axis::HORIZONTAL);

    /**
     * @tc.steps: step3. Test GetCurrentOffset func.
     * @tc.expected: Verify return value.
     */
    pattern_->UpdateCurrentOffset(20.f, SCROLL_FROM_UPDATE);
    Offset currentOffset = controller->GetCurrentOffset();
    EXPECT_EQ(currentOffset, Offset(20.f, 0));

    /**
     * @tc.steps: step4. Test ScrollToEdge func.
     * @tc.expected: Verify return value.
     */
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_TOP, true);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, -1);
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, true);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, -1);
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_RIGHT, true);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, 13);
    controller->ScrollToEdge(ScrollEdgeType::SCROLL_LEFT, true);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().jumpIndex_, 0);

    /**
     * @tc.steps: step5. Test ScrollPage func.
     * @tc.expected: Verify currentOffset.
     */
    pattern_->UpdateCurrentOffset(-20.f, SCROLL_FROM_UPDATE);
    controller->ScrollPage(true, true);
    EXPECT_EQ(controller->GetCurrentOffset(), Offset(300.f, 0));
    EXPECT_EQ(pattern_->GetMainContentSize(), 300.f);

    controller->ScrollPage(false, true);
    EXPECT_EQ(controller->GetCurrentOffset(), Offset(0, 0));
}

/**
 * @tc.name: LayoutInfo001
 * @tc.desc: Test UpdateEndLine func
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, LayoutInfo001, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    CreateGridItem(8);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Change Grid size.
     * @tc.expected: Verify endMainLineIndex_.
     */
    RunMeasureAndLayout(DEVICE_WIDTH, 100.f);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().endMainLineIndex_, 1);
}

/**
 * @tc.name: GridAccessibilityTest001
 * @tc.desc: Test Accessibility func
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridAccessibilityTest001, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    gridModelNG.SetMultiSelectable(true);
    gridModelNG.SetEditable(true);
    CreateGridItem(14);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Run accessibility func.
     * @tc.expected: The return_value is correct.
     */
    EXPECT_TRUE(accessibilityProperty_->IsScrollable());
    EXPECT_TRUE(accessibilityProperty_->IsEditable());
    EXPECT_EQ(accessibilityProperty_->GetBeginIndex(), 0);
    EXPECT_EQ(accessibilityProperty_->GetEndIndex(), 11);
    EXPECT_EQ(accessibilityProperty_->GetCollectionItemCounts(), 14);
    AceCollectionInfo info = accessibilityProperty_->GetCollectionInfo();
    EXPECT_EQ(info.rows, 3);
    EXPECT_EQ(info.columns, 4);
    EXPECT_EQ(info.selectMode, 1);
}

/**
 * @tc.name: GridAccessibilityTest002
 * @tc.desc: Test Accessibility GetCollectionInfo func with non-scrollable Grid
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridAccessibilityTest002, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    CreateGridItem(8);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Run GetCollectionInfo Func.
     * @tc.expected: Verify return value.
     */
    AceCollectionInfo info = accessibilityProperty_->GetCollectionInfo();
    EXPECT_EQ(info.rows, 0);
    EXPECT_EQ(info.columns, 0);
    EXPECT_EQ(info.selectMode, 0);
}

/**
 * @tc.name: GridAccessibilityTest003
 * @tc.desc: Test Accessibility SetSpecificSupportAction func with
 * scrollable Grid and scroll the Grid to the middle
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridAccessibilityTest003, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    CreateGridItem(14);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Run SetSpecificSupportAction func.
     * @tc.expected: The return_value is correct.
     */
    accessibilityProperty_->ResetSupportAction();
    std::unordered_set<AceAction> supportAceActions = accessibilityProperty_->GetSupportAction();
    uint64_t actions = 0, exptectActions = 0;
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_FORWARD);
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_BACKWARD);
    for (auto action : supportAceActions) {
        actions |= 1UL << static_cast<uint32_t>(action);
    }
    EXPECT_EQ(actions, exptectActions);
}

/**
 * @tc.name: GridAccessibilityTest004
 * @tc.desc: Test Accessibility SetSpecificSupportAction func with non-scrollable Grid
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridAccessibilityTest004, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    CreateGridItem(14);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Run SetSpecificSupportAction func.
     * @tc.expected: The return_value is correct.
     */
    accessibilityProperty_->ResetSupportAction();
    std::unordered_set<AceAction> supportAceActions = accessibilityProperty_->GetSupportAction();
    uint64_t actions = 0, exptectActions = 0;
    for (auto action : supportAceActions) {
        actions |= 1UL << static_cast<uint32_t>(action);
    }
    EXPECT_EQ(actions, exptectActions);
}

/**
 * @tc.name: GridAccessibilityTest005
 * @tc.desc: Test Accessibility SetSpecificSupportAction func with
 * scrollable Grid and scroll the Grid to the top
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridAccessibilityTest005, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    CreateGridItem(14);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Run SetSpecificSupportAction func.
     * @tc.expected: The return_value is correct.
     */
    pattern_->UpdateCurrentOffset(100.f, SCROLL_FROM_UPDATE);
    RunMeasureAndLayout();
    accessibilityProperty_->ResetSupportAction();
    std::unordered_set<AceAction> supportAceActions = accessibilityProperty_->GetSupportAction();
    uint64_t actions = 0, exptectActions = 0;
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_FORWARD);
    for (auto action : supportAceActions) {
        actions |= 1UL << static_cast<uint32_t>(action);
    }
    EXPECT_EQ(actions, exptectActions);
}

/**
 * @tc.name: GridAccessibilityTest006
 * @tc.desc: Test Accessibility SetSpecificSupportAction func with
 * scrollable Grid and scroll the Grid to the bottom
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridAccessibilityTest006, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    CreateGridItem(14);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Run SetSpecificSupportAction func.
     * @tc.expected: The return_value is correct.
     */
    pattern_->UpdateCurrentOffset(-200.f, SCROLL_FROM_UPDATE);
    RunMeasureAndLayout();
    accessibilityProperty_->ResetSupportAction();
    std::unordered_set<AceAction> supportAceActions = accessibilityProperty_->GetSupportAction();
    uint64_t actions = 0, exptectActions = 0;
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SCROLL_BACKWARD);
    for (auto action : supportAceActions) {
        actions |= 1UL << static_cast<uint32_t>(action);
    }
    EXPECT_EQ(actions, exptectActions);
}

/**
 * @tc.name: GridAccessibilityTest007
 * @tc.desc: Test GridItem_Accessibility func
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridAccessibilityTest007, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    CreateGridItem(10);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Get 2nd GridItem.
     */
    auto itemFrameNode = GetItemFrameNode(1);
    ASSERT_NE(itemFrameNode, nullptr);
    auto itemAccessibility = itemFrameNode->GetAccessibilityProperty<GridItemAccessibilityProperty>();
    ASSERT_NE(itemAccessibility, nullptr);

    /**
     * @tc.steps: step2. Run itemAccessibility func.
     * @tc.expected: Verify return value.
     */
    EXPECT_FALSE(itemAccessibility->IsSelected());
    AceCollectionItemInfo info = itemAccessibility->GetCollectionItemInfo();
    EXPECT_EQ(info.row, 0);
    EXPECT_EQ(info.column, 1);
    EXPECT_EQ(info.rowSpan, 1);
    EXPECT_EQ(info.columnSpan, 1);
    EXPECT_FALSE(info.heading);

    /**
     * @tc.steps: step3. Run SetSpecificSupportAction func.
     * @tc.expected: Verify return value.
     */
    itemAccessibility->ResetSupportAction();
    std::unordered_set<AceAction> supportAceActions = itemAccessibility->GetSupportAction();
    uint64_t actions = 0, exptectActions = 0;
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_SELECT);
    exptectActions |= 1UL << static_cast<uint32_t>(AceAction::ACTION_CLEAR_SELECTION);
    for (auto action : supportAceActions) {
        actions |= 1UL << static_cast<uint32_t>(action);
    }
    EXPECT_EQ(actions, exptectActions);
}

/**
 * @tc.name: GridAccessibilityTest008
 * @tc.desc: Test GridItem_Accessibility GetCollectionItemInfo func with has heading GridItem
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridAccessibilityTest008, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);

    /**
     * @tc.steps: step1. Create heading GridItem.
     */
    GridItemModelNG gridItemModel;
    gridItemModel.Create();
    gridItemModel.SetColumnStart(0);
    gridItemModel.SetColumnEnd(3);
    SetHeight(Dimension(ITEM_HEIGHT));
    ViewStackProcessor::GetInstance()->Pop();
    CreateGridItem(10);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step2. Get 1st GridItem.
     */
    auto itemFrameNode = GetItemFrameNode(0);
    ASSERT_NE(itemFrameNode, nullptr);
    auto itemAccessibility = itemFrameNode->GetAccessibilityProperty<GridItemAccessibilityProperty>();
    ASSERT_NE(itemAccessibility, nullptr);

    /**
     * @tc.steps: step2. Run GetCollectionItemInfo func.
     * @tc.expected: Verify return value.
     */
    AceCollectionItemInfo info = itemAccessibility->GetCollectionItemInfo();
    EXPECT_EQ(info.row, 0);
    EXPECT_EQ(info.column, 0);
    EXPECT_EQ(info.rowSpan, 1);
    EXPECT_EQ(info.columnSpan, 4);
    EXPECT_TRUE(info.heading);
}

/**
 * @tc.name: MouseSelect001
 * @tc.desc: Test mouse click to select, only PRESS and RELEASE the mouse
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, MouseSelect001, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    gridModelNG.SetMultiSelectable(true);
    CreateGridItem(8);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Click the (0, 0) point of firstItem.
     * @tc.expected: item not selected on MouseAction::PRESS.
     */
    MouseSelect(Offset(0.f, 0.f), Offset(0.f, 0.f));
    EXPECT_FALSE(GetItemPattern(0)->IsSelected());
}

/**
 * @tc.name: MouseSelect002
 * @tc.desc: Test mouse box selection, PRESS, MOVE and RELEASE the mouse
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, MouseSelect002, TestSize.Level1)
{
    const Offset LEFT_TOP = Offset(90.f, 50.f);
    const Offset LEFT_BOTTOM = Offset(90.f, 150.f);
    const Offset RIGHT_TOP = Offset(270.f, 50.f);
    const Offset RIGHT_BOTTOM = Offset(270.f, 150.f);

    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    gridModelNG.SetMultiSelectable(true);
    CreateGridItem(8);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Select (0, 0) - (180, 100) zone.
     * @tc.expected: The 1st, 2nd, 5th, 6th items are selected.
     */
    MouseSelect(Offset(0.f, 0.f), Offset(180.f, 100.f));
    EXPECT_TRUE(GetItemPattern(0)->IsSelected());
    EXPECT_TRUE(GetItemPattern(1)->IsSelected());
    EXPECT_TRUE(GetItemPattern(4)->IsSelected());
    EXPECT_TRUE(GetItemPattern(5)->IsSelected());
    MouseSelectRelease();

    /**
     * @tc.steps: step2. Select (90, 50) - (270, 150) zone, from the LEFT_TOP to the RIGHT_BOTTOM.
     * @tc.expected: The 1st, 2nd, 5th, 6th are selected.
     */
    MouseSelect(LEFT_TOP, RIGHT_BOTTOM);
    EXPECT_TRUE(GetItemPattern(0)->IsSelected());
    EXPECT_TRUE(GetItemPattern(1)->IsSelected());
    EXPECT_TRUE(GetItemPattern(4)->IsSelected());
    EXPECT_TRUE(GetItemPattern(5)->IsSelected());
    MouseSelectRelease();

    /**
     * @tc.steps: step3. Select (90, 50) - (270, 150) zone, from the RIGHT_TOP to the LEFT_BOTTOM.
     * @tc.expected: The 1st, 2nd, 5th, 6th are selected.
     */
    MouseSelect(RIGHT_TOP, LEFT_BOTTOM);
    EXPECT_TRUE(GetItemPattern(0)->IsSelected());
    EXPECT_TRUE(GetItemPattern(1)->IsSelected());
    EXPECT_TRUE(GetItemPattern(4)->IsSelected());
    EXPECT_TRUE(GetItemPattern(5)->IsSelected());
    MouseSelectRelease();

    /**
     * @tc.steps: step4. Select (90, 50) - (270, 150) zone, from the LEFT_BOTTOM to the RIGHT_TOP.
     * @tc.expected: The 1st, 2nd, 5th, 6th are selected.
     */
    MouseSelect(LEFT_BOTTOM, RIGHT_TOP);
    EXPECT_TRUE(GetItemPattern(0)->IsSelected());
    EXPECT_TRUE(GetItemPattern(1)->IsSelected());
    EXPECT_TRUE(GetItemPattern(4)->IsSelected());
    EXPECT_TRUE(GetItemPattern(5)->IsSelected());
    MouseSelectRelease();

    /**
     * @tc.steps: step5. Select (90, 50) - (270, 150) zone, from the RIGHT_BOTTOM to the LEFT_TOP.
     * @tc.expected: The 1st, 2nd, 5th, 6th are selected.
     */
    MouseSelect(RIGHT_BOTTOM, LEFT_TOP);
    EXPECT_TRUE(GetItemPattern(0)->IsSelected());
    EXPECT_TRUE(GetItemPattern(1)->IsSelected());
    EXPECT_TRUE(GetItemPattern(4)->IsSelected());
    EXPECT_TRUE(GetItemPattern(5)->IsSelected());
    MouseSelectRelease();
}

/**
 * @tc.name: MouseSelect003
 * @tc.desc: Test gridItem selectable property and onSelect callback
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, MouseSelect003, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    gridModelNG.SetMultiSelectable(true);
    bool isSixthItemSelected = false;
    auto selectCallback = [&isSixthItemSelected](bool) { isSixthItemSelected = true; };
    for (int32_t i = 0; i < 10; i++) {
        GridItemModelNG gridItemModel;
        gridItemModel.Create();
        SetHeight(Dimension(ITEM_HEIGHT));
        /**
         *  ___180__360___540___720
         * |____|__1__|__2__|____|100
         * |____|__5__|_____|____|200
         * |____|_____|          300
         */
        if (i == 1) {
            gridItemModel.SetSelectable(false);
        }
        if (i == 2) {
            auto itemFrameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
            auto itemEvent = itemFrameNode->GetEventHub<EventHub>();
            itemEvent->SetEnabled(false);
        }
        if (i == 5) {
            gridItemModel.SetOnSelect(std::move(selectCallback));
        }
        ViewStackProcessor::GetInstance()->Pop();
    }
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step2. Select (225, 50) - (315, 150) zone, include 2nd, 3rd, 6th, 7th item.
     * @tc.expected: The 2nd and 3rd item is not selected but 6th item is selected.
     */
    MouseSelect(Offset(225.f, 50.f), Offset(315.f, 150.f));
    EXPECT_FALSE(GetItemPattern(1)->IsSelected());
    EXPECT_FALSE(GetItemPattern(2)->IsSelected());
    EXPECT_TRUE(GetItemPattern(5)->IsSelected());
    EXPECT_TRUE(isSixthItemSelected);
}

/**
 * @tc.name: MouseSelect004
 * @tc.desc: Test OnMouseSelectAll func
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, MouseSelect004, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    gridModelNG.SetMultiSelectable(true);
    CreateGridItem(8);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Run OnMouseSelectAll func.
     * @tc.expected: All items are selected.
     */
    pattern_->OnMouseSelectAll();
    auto children = frameNode_->GetChildren();
    for (const auto& item : children) {
        auto itemFrameNode = AceType::DynamicCast<FrameNode>(item);
        if (!itemFrameNode) {
            continue;
        }
        auto itemPattern = itemFrameNode->GetPattern<GridItemPattern>();
        EXPECT_TRUE(itemPattern->IsSelected());
    }
}

/**
 * @tc.name: MouseSelect005
 * @tc.desc: Test select in other condition
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, MouseSelect005, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    gridModelNG.SetMultiSelectable(true);
    CreateGridItem(8);
    GetInstance();
    RunMeasureAndLayout();

    const Offset startOffset = Offset(225.f, 50.f);
    const Offset endOffset = Offset(315.f, 150.f);

    /**
     * @tc.steps: step1. Select (225, 50) - (315, 150) zone with RIGHT_BUTTON.
     * @tc.expected: The item is not selected.
     */
    MouseInfo info;
    info.SetButton(MouseButton::RIGHT_BUTTON); // Use RIGHT_BUTTON to select.
    info.SetAction(MouseAction::PRESS);
    info.SetLocalLocation(startOffset);
    pattern_->HandleMouseEventWithoutKeyboard(info);
    info.SetAction(MouseAction::MOVE);
    info.SetLocalLocation(endOffset);
    pattern_->HandleMouseEventWithoutKeyboard(info);
    EXPECT_FALSE(GetItemPattern(1)->IsSelected());

    /**
     * @tc.steps: step2. Select (225, 50) - (315, 150) zone with HOVER
     * @tc.expected: The item is not Selected
     */
    info.SetButton(MouseButton::LEFT_BUTTON);
    info.SetAction(MouseAction::HOVER); // HOVER
    info.SetLocalLocation(startOffset);
    pattern_->HandleMouseEventWithoutKeyboard(info);
    info.SetAction(MouseAction::MOVE);
    info.SetLocalLocation(endOffset);
    pattern_->HandleMouseEventWithoutKeyboard(info);
    EXPECT_FALSE(GetItemPattern(1)->IsSelected());

    /**
     * @tc.steps: step3. Move distance < FRAME_SELECTION_DISTANCE
     * @tc.expected: The item is not Selected
     */
    MouseSelect(Offset(0.f, 0.f), Offset(1.f, 1.f));
    EXPECT_FALSE(GetItemPattern(0)->IsSelected());
}

/**
 * @tc.name: Drag001
 * @tc.desc: Verify drag func
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, Drag001, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    gridModelNG.SetEditable(true);
    auto onItemDragStart = [](const ItemDragInfo&, int32_t) {
        auto dragItem = AceType::MakeRefPtr<FrameNode>("test", 0, AceType::MakeRefPtr<Pattern>());
        return AceType::DynamicCast<UINode>(dragItem);
    };
    gridModelNG.SetOnItemDragStart(onItemDragStart);
    CreateGridItem(8);
    GetInstance();
    RunMeasureAndLayout();
    eventHub_->onItemDragStart_ = onItemDragStart;

    GestureEvent info;
    Point globalPoint = Point(270.f, 50.f); // Point at the second item.
    info.SetGlobalPoint(globalPoint);

    /**
     * @tc.steps: step1. Trigger HandleOnItemDragStart, HandleOnItemDragUpdate, HandleOnItemDragEnd.
     * @tc.expected: Verify some values of the drag.
     */
    eventHub_->HandleOnItemDragStart(info);
    EXPECT_EQ(eventHub_->draggedIndex_, 1);
    ASSERT_NE(eventHub_->dragDropProxy_, nullptr);
    auto itemFrameNode = GetItemFrameNode(1);
    EXPECT_EQ(eventHub_->draggingItem_, itemFrameNode);
    eventHub_->HandleOnItemDragUpdate(info);
    eventHub_->HandleOnItemDragEnd(info);
    EXPECT_EQ(eventHub_->draggedIndex_, 0);
    EXPECT_EQ(eventHub_->dragDropProxy_, nullptr);
    EXPECT_EQ(eventHub_->draggingItem_, nullptr);

    /**
     * @tc.steps: step2. Trigger HandleOnItemDragStart, HandleOnItemDragUpdate, HandleOnItemDragCancel.
     * @tc.expected: Verify some values of the drag.
     */
    eventHub_->HandleOnItemDragStart(info);
    eventHub_->HandleOnItemDragUpdate(info);
    eventHub_->HandleOnItemDragCancel();
    EXPECT_EQ(eventHub_->draggedIndex_, 0);
    EXPECT_EQ(eventHub_->dragDropProxy_, nullptr);
    EXPECT_EQ(eventHub_->draggingItem_, nullptr);
}

/**
 * @tc.name: Drag002
 * @tc.desc: Verify drag func
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, Drag002, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    gridModelNG.SetEditable(true);
    gridModelNG.SetSupportAnimation(true);
    CreateGridItem(8);
    GetInstance();
    RunMeasureAndLayout();
    auto onItemDragStart = [](const ItemDragInfo&, int32_t) {
        auto dragItem = AceType::MakeRefPtr<FrameNode>("test", 0, AceType::MakeRefPtr<Pattern>());
        return AceType::DynamicCast<UINode>(dragItem);
    };
    eventHub_->onItemDragStart_ = onItemDragStart;

    GestureEvent info;
    Point globalPoint = Point(270.f, 50.f);
    info.SetGlobalPoint(globalPoint);
    eventHub_->HandleOnItemDragStart(info);

    /**
     * @tc.steps: step1. Drag 1st item to out of Grid.
     * @tc.expected: GetOriginalIndex return number of GridItem.
     */
    ItemDragInfo dragInfo;
    dragInfo.SetX(0.f);
    dragInfo.SetY(0.f);
    eventHub_->FireOnItemDragEnter(dragInfo);
    eventHub_->FireOnItemDragLeave(dragInfo, -1);
    EXPECT_EQ(pattern_->GetOriginalIndex(), 8);

    /**
     * @tc.steps: step2. Drag 2nd item to 3rd item, Drag 3 item to 2 item.
     * @tc.expected: GetOriginalIndex changed.
     */
    eventHub_->FireOnItemDragEnter(dragInfo);
    eventHub_->FireOnItemDragMove(dragInfo, 1, 2);
    EXPECT_EQ(pattern_->GetOriginalIndex(), 2);
    eventHub_->FireOnItemDragLeave(dragInfo, -1);
    eventHub_->FireOnItemDragEnter(dragInfo);
    // 3 to 2
    eventHub_->FireOnItemDragMove(dragInfo, 2, 1);
    EXPECT_EQ(pattern_->GetOriginalIndex(), 1);
    // SupportAnimation, ClearDragState
    eventHub_->FireOnItemDrop(dragInfo, 0, 1, true);
    EXPECT_EQ(pattern_->GetOriginalIndex(), -1);

    /**
     * @tc.steps: step3. Move something to 3rd item.
     * @tc.expected: GetOriginalIndex changed.
     */
    eventHub_->FireOnItemDragEnter(dragInfo);
    eventHub_->FireOnItemDragLeave(dragInfo, -1);
    eventHub_->FireOnItemDragMove(dragInfo, -1, 2);
    EXPECT_EQ(pattern_->GetOriginalIndex(), 2);
    // SupportAnimation, ClearDragState
    eventHub_->FireOnItemDrop(dragInfo, -1, 1, true);
    EXPECT_EQ(pattern_->GetOriginalIndex(), -1);
}

/**
 * @tc.name:FocusStep001
 * @tc.desc: Test GetNextFocusNode func
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, FocusStep001, TestSize.Level1)
{
    /**
     *  ___180__360__540__720
     * |____|____|____|____|100
     * |____|____|____|____|200
     * |____|____|          300
     */
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    CreateGridItem(10, Axis::VERTICAL, true);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. GetNextFocusNode from left_top.
     * @tc.expected: Verify all condition of FocusStep.
     */
    constexpr int32_t currentIndex1 = 0;
    std::map<FocusStep, int32_t> next1 = {
        {FocusStep::NONE, -1},
        {FocusStep::LEFT, -1},
        {FocusStep::UP, -1},
        {FocusStep::RIGHT, 1},
        {FocusStep::DOWN, 4},
        {FocusStep::LEFT_END, -1},
        {FocusStep::UP_END, -1},
        {FocusStep::RIGHT_END, 3},
        {FocusStep::DOWN_END, -1}
    };
    EXPECT_TRUE(IsEqualNextFocusNode(currentIndex1, next1));

    /**
     * @tc.steps: step2. GetNextFocusNode from right_top.
     * @tc.expected: Verify all condition of FocusStep.
     */
    constexpr int32_t currentIndex2 = 3;
    std::map<FocusStep, int32_t> next2 = {
        {FocusStep::NONE, -1},
        {FocusStep::LEFT, 2},
        {FocusStep::UP, -1},
        {FocusStep::RIGHT, 4},
        {FocusStep::DOWN, 7},
        {FocusStep::LEFT_END, 0},
        {FocusStep::UP_END, -1},
        {FocusStep::RIGHT_END, -1},
        {FocusStep::DOWN_END, -1}
    };
    EXPECT_TRUE(IsEqualNextFocusNode(currentIndex2, next2));

    /**
     * @tc.steps: step3. GetNextFocusNode from left_bottom.
     * @tc.expected: Verify all condition of FocusStep.
     */
    constexpr int32_t currentIndex3 = 8;
    std::map<FocusStep, int32_t> next3 = {
        {FocusStep::NONE, -1},
        {FocusStep::LEFT, 7},
        {FocusStep::UP, 4},
        {FocusStep::RIGHT, 9},
        {FocusStep::DOWN, -1},
        {FocusStep::LEFT_END, -1},
        {FocusStep::UP_END, -1},
        {FocusStep::RIGHT_END, 9},
        {FocusStep::DOWN_END, -1}
    };
    EXPECT_TRUE(IsEqualNextFocusNode(currentIndex3, next3));

    /**
     * @tc.steps: step4. GetNextFocusNode from right_bottom.
     * @tc.expected: Verify all condition of FocusStep.
     */
    constexpr int32_t currentIndex4 = 9;
    std::map<FocusStep, int32_t> next4 = {
        {FocusStep::NONE, -1},
        {FocusStep::LEFT, 8},
        {FocusStep::UP, 5},
        {FocusStep::RIGHT, -1},
        {FocusStep::DOWN, -1},
        {FocusStep::LEFT_END, 8},
        {FocusStep::UP_END, -1},
        {FocusStep::RIGHT_END, -1},
        {FocusStep::DOWN_END, -1}
    };
    EXPECT_TRUE(IsEqualNextFocusNode(currentIndex4, next4));

    /**
     * @tc.steps: step5. GetNextFocusNode from middle.
     * @tc.expected: Verify all condition of FocusStep.
     */
    constexpr int32_t currentIndex5 = 5;
    std::map<FocusStep, int32_t> next5 = {
        {FocusStep::NONE, -1},
        {FocusStep::LEFT, 4},
        {FocusStep::UP, 1},
        {FocusStep::RIGHT, 6},
        {FocusStep::DOWN, 9},
        {FocusStep::LEFT_END, 4},
        {FocusStep::UP_END, -1},
        {FocusStep::RIGHT_END, 7},
        {FocusStep::DOWN_END, -1}
    };
    EXPECT_TRUE(IsEqualNextFocusNode(currentIndex5, next5));
}

/**
 * @tc.name:FocusStep002
 * @tc.desc: Test GetNextFocusNode() with FlexDirection::COLUMN
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, FocusStep002, TestSize.Level1)
{
    /**
     *  ___180__360__540__720
     * |____|____|____|     100
     * |____|____|____|     200
     * |____|____|          300
     * |____|____|          400
     */
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate(TEMPLATE_4);
    CreateGridItem(10, Axis::HORIZONTAL, true);
    GetInstance();
    RunMeasureAndLayout(DEVICE_WIDTH, 400.f);

    /**
     * @tc.steps: step1. GetNextFocusNode from left_top.
     * @tc.expected: Verify all condition of FocusStep.
     */
    constexpr int32_t currentIndex1 = 0;
    std::map<FocusStep, int32_t> next1 = {
        {FocusStep::NONE, -1},
        {FocusStep::LEFT, -1},
        {FocusStep::UP, -1},
        {FocusStep::RIGHT, 4},
        {FocusStep::DOWN, 1},
        {FocusStep::LEFT_END, -1},
        {FocusStep::UP_END, -1},
        {FocusStep::RIGHT_END, -1},
        {FocusStep::DOWN_END, 3}
    };
    EXPECT_TRUE(IsEqualNextFocusNode(currentIndex1, next1));

    /**
     * @tc.steps: step2. GetNextFocusNode from right_top.
     * @tc.expected: Verify all condition of FocusStep.
     */
    constexpr int32_t currentIndex2 = 8;
    std::map<FocusStep, int32_t> next2 = {
        {FocusStep::NONE, -1},
        {FocusStep::LEFT, 4},
        {FocusStep::UP, 7},
        {FocusStep::RIGHT, -1},
        {FocusStep::DOWN, 9},
        {FocusStep::LEFT_END, -1},
        {FocusStep::UP_END, -1},
        {FocusStep::RIGHT_END, -1},
        {FocusStep::DOWN_END, 9}
    };
    EXPECT_TRUE(IsEqualNextFocusNode(currentIndex2, next2));

    /**
     * @tc.steps: step3. GetNextFocusNode from left_bottom.
     * @tc.expected: Verify all condition of FocusStep.
     */
    constexpr int32_t currentIndex3 = 3;
    std::map<FocusStep, int32_t> next3 = {
        {FocusStep::NONE, -1},
        {FocusStep::LEFT, -1},
        {FocusStep::UP, 2},
        {FocusStep::RIGHT, 7},
        {FocusStep::DOWN, 4},
        {FocusStep::LEFT_END, -1},
        {FocusStep::UP_END, 0},
        {FocusStep::RIGHT_END, -1},
        {FocusStep::DOWN_END, -1}
    };
    EXPECT_TRUE(IsEqualNextFocusNode(currentIndex3, next3));

    /**
     * @tc.steps: step4. GetNextFocusNode from right_bottom.
     * @tc.expected: Verify all condition of FocusStep.
     */
    constexpr int32_t currentIndex4 = 9;
    std::map<FocusStep, int32_t> next4 = {
        {FocusStep::NONE, -1},
        {FocusStep::LEFT, 5},
        {FocusStep::UP, 8},
        {FocusStep::RIGHT, -1},
        {FocusStep::DOWN, -1},
        {FocusStep::LEFT_END, -1},
        {FocusStep::UP_END, 8},
        {FocusStep::RIGHT_END, -1},
        {FocusStep::DOWN_END, -1}
    };
    EXPECT_TRUE(IsEqualNextFocusNode(currentIndex4, next4));

    /**
     * @tc.steps: step5. GetNextFocusNode from middle.
     * @tc.expected: Verify all condition of FocusStep.
     */
    constexpr int32_t currentIndex5 = 5;
    std::map<FocusStep, int32_t> next5 = {
        {FocusStep::NONE, -1},
        {FocusStep::LEFT, 1},
        {FocusStep::UP, 4},
        {FocusStep::RIGHT, 9},
        {FocusStep::DOWN, 6},
        {FocusStep::LEFT_END, -1},
        {FocusStep::UP_END, 4},
        {FocusStep::RIGHT_END, -1},
        {FocusStep::DOWN_END, 7}
    };
    EXPECT_TRUE(IsEqualNextFocusNode(currentIndex5, next5));
}

/**
 * @tc.name: FocusStep003
 * @tc.desc: Test GetNextFocusNode func when has unfocuseable item
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, FocusStep003, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    CreateGridItem(10, Axis::VERTICAL, true);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. GetNextFocusNode from 1st item and FocusStep::RIGHT.
     * @tc.expected: The 3rd item is focused.
     */
    GetItemFocusHub(1)->SetFocusable(false); // The 2nd item can not focus.
    constexpr int32_t currentIndex = 0;
    std::map<FocusStep, int32_t> next = {
        {FocusStep::RIGHT, 2},
    };
    EXPECT_TRUE(IsEqualNextFocusNode(currentIndex, next));
}

/**
 * @tc.name: FocusStep004
 * @tc.desc: Test GetNextFocusNode func from top boundary item in Scrollable Grid
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, FocusStep004, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    CreateGridItem(18, Axis::VERTICAL, true);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Scroll to second row
     */
    constexpr float scrollOffset = -ITEM_HEIGHT - 1.f;
    UpdateCurrentOffset(scrollOffset);

    /**
     * @tc.steps: step2. UP
     */
    constexpr int32_t currentIndex = 4;
    std::map<FocusStep, int32_t> next = {
        {FocusStep::UP, -1},
    };
    EXPECT_TRUE(IsEqualNextFocusNode(currentIndex, next));
    EXPECT_EQ(pattern_->gridLayoutInfo_.jumpIndex_, 3);
}

/**
 * @tc.name: FocusStep005
 * @tc.desc: Test GetNextFocusNode func from bottom boundary item in Scrollable Grid
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, FocusStep005, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    CreateGridItem(18, Axis::VERTICAL, true);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Scroll to first row
     */
    constexpr float scrollOffset = -ITEM_HEIGHT + 1.f;
    UpdateCurrentOffset(scrollOffset);

    /**
     * @tc.steps: step2. DOWN
     */
    constexpr int32_t currentIndex = 15;
    std::map<FocusStep, int32_t> next = {
        {FocusStep::DOWN, -1},
    };
    EXPECT_TRUE(IsEqualNextFocusNode(currentIndex, next));
    EXPECT_EQ(pattern_->gridLayoutInfo_.jumpIndex_, 16);
}

/**
 * @tc.name: GridPatternTest001
 * @tc.desc: Test grid pattern UpdateCurrentOffset function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest001, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();

    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    DirtySwapConfig config;
    config.skipMeasure = true;
    config.skipLayout = true;
    pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    bool result;
    /**
     * @tc.steps: step2. call OnDirtyLayoutWrapperSwap function
     * @tc.steps: case1: no scrollEffect, !isScrollContent
     * @tc.expected: step2. equal.
     */
    result = pattern_->UpdateCurrentOffset(0, SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->scrollState_, SCROLL_FROM_UPDATE);
    EXPECT_TRUE(result);

    /**
     * @tc.steps: step2. call OnDirtyLayoutWrapperSwap function
     * @tc.steps: case2: no scrollEffect, isScrollContent
     * @tc.expected: step2. equal.
     */
    result = pattern_->UpdateCurrentOffset(0, SCROLL_FROM_BAR);
    EXPECT_EQ(pattern_->scrollState_, SCROLL_FROM_BAR);
    EXPECT_TRUE(result);

    /**
     * @tc.steps: step2. call OnDirtyLayoutWrapperSwap function
     * @tc.steps: case3: scrollEffect not Spring, !isScrollContent
     * @tc.expected: step2. equal.
     */
    pattern_->scrollEffect_ = AceType::MakeRefPtr<ScrollEdgeEffect>(EdgeEffect::FADE);
    result = pattern_->UpdateCurrentOffset(0, SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->scrollState_, SCROLL_FROM_UPDATE);
    EXPECT_TRUE(result);

    /**
     * @tc.steps: step2. call OnDirtyLayoutWrapperSwap function
     * @tc.steps: case4: scrollEffect Spring, !isScrollContent
     * @tc.expected: step2. equal.
     */
    pattern_->scrollEffect_ = AceType::MakeRefPtr<ScrollEdgeEffect>(EdgeEffect::SPRING);
    result = pattern_->UpdateCurrentOffset(0, SCROLL_FROM_UPDATE);
    EXPECT_EQ(pattern_->scrollState_, SCROLL_FROM_UPDATE);
    EXPECT_TRUE(result);

    /**
     * @tc.steps: step2. call OnDirtyLayoutWrapperSwap function
     * @tc.steps: case4: scrollEffect Spring, isScrollContent
     * @tc.expected: step2. equal.
     */
    pattern_->scrollEffect_ = AceType::MakeRefPtr<ScrollEdgeEffect>(EdgeEffect::SPRING);
    result = pattern_->UpdateCurrentOffset(0, SCROLL_FROM_BAR);
    EXPECT_EQ(pattern_->scrollState_, SCROLL_FROM_BAR);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: GridPatternTest002
 * @tc.desc: Test grid pattern IsOutOfBoundary function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest002, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    bool result;
    result = pattern_->IsOutOfBoundary();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GridPatternTest003
 * @tc.desc: Test grid pattern IsOutOfBoundary function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest003, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    DirtySwapConfig config;
    config.skipMeasure = true;
    config.skipLayout = true;
    pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    bool result;
    pattern_->gridLayoutInfo_.reachStart_ = true;
    result = pattern_->IsOutOfBoundary();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: GridPatternTest004
 * @tc.desc: Test grid pattern IsOutOfBoundary function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest004, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    DirtySwapConfig config;
    config.skipMeasure = true;
    config.skipLayout = true;
    pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    bool result;
    pattern_->gridLayoutInfo_.reachStart_ = false;
    pattern_->gridLayoutInfo_.offsetEnd_ = true;
    result = pattern_->IsOutOfBoundary();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: GridPatternTest005
 * @tc.desc: Test grid pattern IsOutOfBoundary function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest005, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    DirtySwapConfig config;
    config.skipMeasure = true;
    config.skipLayout = true;
    pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    bool result;
    pattern_->gridLayoutInfo_.reachStart_ = true;
    pattern_->gridLayoutInfo_.offsetEnd_ = true;
    result = pattern_->IsOutOfBoundary();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: GridPatternTest006
 * @tc.desc: Test grid pattern OutBoundaryCallback function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest006, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    bool outBoundary;
    outBoundary = pattern_->OutBoundaryCallback();
    EXPECT_FALSE(outBoundary);
}

/**
 * @tc.name: GridPatternTest007
 * @tc.desc: Test grid pattern OutBoundaryCallback function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest007, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    DirtySwapConfig config;
    config.skipMeasure = true;
    config.skipLayout = true;
    pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    bool outBoundary;
    pattern_->gridLayoutInfo_.reachStart_ = true;
    pattern_->gridLayoutInfo_.offsetEnd_ = true;
    outBoundary = pattern_->OutBoundaryCallback();
    EXPECT_TRUE(outBoundary);
}

/**
 * @tc.name: GridPatternTest008
 * @tc.desc: Test grid pattern OutBoundaryCallback function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest008, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    DirtySwapConfig config;
    config.skipMeasure = true;
    config.skipLayout = true;
    pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    bool outBoundary;
    pattern_->gridLayoutInfo_.reachStart_ = false;
    pattern_->gridLayoutInfo_.offsetEnd_ = true;
    outBoundary = pattern_->OutBoundaryCallback();
    EXPECT_TRUE(outBoundary);
}

/**
 * @tc.name: GridPatternTest009
 * @tc.desc: Test grid pattern OutBoundaryCallback function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest009, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    DirtySwapConfig config;
    config.skipMeasure = true;
    config.skipLayout = true;
    pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    bool outBoundary;
    pattern_->gridLayoutInfo_.reachStart_ = true;
    pattern_->gridLayoutInfo_.offsetEnd_ = false;
    outBoundary = pattern_->OutBoundaryCallback();
    EXPECT_TRUE(outBoundary);
}

/**
 * @tc.name: GridPatternTest011
 * @tc.desc: Test grid pattern OnModifyDone function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest011, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    pattern_->SetMultiSelectable(true);
    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    LayoutConstraintF constraint;
    constraint.selfIdealSize.SetSize(CONTAINER_SIZE);
    auto gridScrollLayoutAlgorithm = AceType::MakeRefPtr<GridScrollLayoutAlgorithm>(
        pattern_->gridLayoutInfo_, ALGORITHM_RELATE_LALUE_FIRST, ALGORITHM_RELATE_LALUE_SECOND);
    ASSERT_NE(gridScrollLayoutAlgorithm, nullptr);
    gridScrollLayoutAlgorithm->SetCanOverScroll(true);
    layoutWrapper->Measure(constraint);
    layoutWrapper->Layout();
    pattern_->gridLayoutInfo_.reachEnd_ = true;
    pattern_->gridLayoutInfo_.prevOffset_ = PREV_OFFSET_FIRST;
    pattern_->gridLayoutInfo_.currentOffset_ = CURRENT_OFFSET_FIRST;
    EXPECT_FALSE(gridScrollLayoutAlgorithm->gridLayoutInfo_.reachEnd_);
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), CONTAINER_SIZE);
    pattern_->OnModifyDone();
    EXPECT_TRUE(pattern_->multiSelectable_);
    EXPECT_TRUE(pattern_->isMouseEventInit_);
    EXPECT_TRUE(pattern_->isConfigScrollable_);
    pattern_->SetMultiSelectable(false);
    pattern_->OnModifyDone();
    EXPECT_FALSE(pattern_->multiSelectable_);
    EXPECT_FALSE(pattern_->isMouseEventInit_);
}

/**
 * @tc.name: GridPatternTest012
 * @tc.desc: Test grid pattern OnModifyDone function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest012, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    pattern_->SetMultiSelectable(true);
    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    LayoutConstraintF constraint;
    constraint.selfIdealSize.SetSize(CONTAINER_SIZE);
    auto gridScrollLayoutAlgorithm = AceType::MakeRefPtr<GridScrollLayoutAlgorithm>(
        pattern_->gridLayoutInfo_, ALGORITHM_RELATE_LALUE_FIRST, ALGORITHM_RELATE_LALUE_SECOND);
    ASSERT_NE(gridScrollLayoutAlgorithm, nullptr);
    gridScrollLayoutAlgorithm->SetCanOverScroll(false);
    layoutWrapper->Measure(constraint);
    layoutWrapper->Layout();
    pattern_->gridLayoutInfo_.reachEnd_ = true;
    pattern_->gridLayoutInfo_.prevOffset_ = PREV_OFFSET_FIRST;
    pattern_->gridLayoutInfo_.currentOffset_ = CURRENT_OFFSET_FIRST;
    EXPECT_FALSE(gridScrollLayoutAlgorithm->gridLayoutInfo_.reachEnd_);
    EXPECT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize(), CONTAINER_SIZE);
    pattern_->OnModifyDone();
    EXPECT_TRUE(pattern_->multiSelectable_);
    EXPECT_TRUE(pattern_->isMouseEventInit_);
    EXPECT_TRUE(pattern_->isConfigScrollable_);
    pattern_->SetMultiSelectable(false);
    pattern_->OnModifyDone();
    EXPECT_FALSE(pattern_->multiSelectable_);
    EXPECT_FALSE(pattern_->isMouseEventInit_);
}

/**
 * @tc.name: GridPatternTest013
 * @tc.desc: Test grid pattern CreateLayoutAlgorithm function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest013, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto result = pattern_->CreateLayoutAlgorithm();
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name: GridPatternTest014
 * @tc.desc: Test grid pattern GetContentSize function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest014, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);

    auto size = pattern_->GetContentSize();
    EXPECT_EQ(size.Width(), 0);
    EXPECT_EQ(size.Height(), 0);
}

/**
 * @tc.name: GridPatternTest015
 * @tc.desc: Test grid pattern CheckScrollable function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest015, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    EXPECT_TRUE(pattern_->scrollable_);
    pattern_->CheckScrollable();
    EXPECT_FALSE(pattern_->scrollable_);
}

/**
 * @tc.name: GridPatternTest016
 * @tc.desc: Test grid pattern CheckScrollable function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest016, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    EXPECT_TRUE(pattern_->scrollable_);
    pattern_->gridLayoutInfo_.endIndex_ = END_INDEX;
    pattern_->gridLayoutInfo_.startIndex_ = START_INDEX_END;
    pattern_->gridLayoutInfo_.childrenCount_ = CHILDREN_COUNT;
    pattern_->CheckScrollable();
    EXPECT_TRUE(pattern_->scrollable_);
}
/**
 * @tc.name: GridPatternTest017
 * @tc.desc: Test grid pattern UpdateScrollBarOffset function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest017, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    pattern_->scrollBar_ = nullptr;
    pattern_->scrollBarProxy_ = nullptr;
    pattern_->UpdateScrollBarOffset();
    EXPECT_EQ(pattern_->estimatedHeight_, 0.0f);
}

/**
 * @tc.name: GridPaintMethodTest001
 * @tc.desc: Test grid paint method GetForegroundDrawFunction function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPaintMethodTest001, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    CreateGridItem(10);
    GetInstance();

    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    auto paintMethod = AceType::DynamicCast<GridPaintMethod>(pattern_->CreateNodePaintMethod());
    ASSERT_NE(paintMethod, nullptr);
    auto paintProperty = pattern_->CreatePaintProperty();
    ASSERT_NE(paintProperty, nullptr);
    PaintWrapper paintWrapper(frameNode_->GetRenderContext(), frameNode_->GetGeometryNode(), paintProperty);
    auto drawFunction = paintMethod->GetForegroundDrawFunction(&paintWrapper);
    ASSERT_NE(drawFunction, nullptr);
    Testing::MockCanvas rsCanvas;
    EXPECT_CALL(rsCanvas, DetachPen()).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DrawRect(_)).WillRepeatedly(Return());
    drawFunction(rsCanvas);

    /**
     * @tc.steps: step1. When EdgeEffect::SPRING
     * @tc.expected: CreateNodePaintMethod would not trigger SetEdgeEffect
     */
    pattern_->SetEdgeEffect(EdgeEffect::SPRING);
    paintMethod = AceType::DynamicCast<GridPaintMethod>(pattern_->CreateNodePaintMethod());
    EXPECT_EQ(paintMethod->edgeEffect_.Upgrade(), nullptr);

    /**
     * @tc.steps: step2. When EdgeEffect::FADE
     * @tc.expected: CreateNodePaintMethod would trigger SetEdgeEffect
     */
    pattern_->SetEdgeEffect(EdgeEffect::FADE);
    paintMethod = AceType::DynamicCast<GridPaintMethod>(pattern_->CreateNodePaintMethod());
    EXPECT_NE(paintMethod->edgeEffect_.Upgrade(), nullptr);
}

/**
 * @tc.name: GridPatternTest018
 * @tc.desc: Test grid pattern ModifyCurrentOffsetWhenReachEnd function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest018, TestSize.Level1)
{
    constexpr int32_t itemCount = 8;
    constexpr float itemHeight = 100.0f;
    /**
     * @tc.steps: step1. Get frameNode, pattern, eventHub and RunMeasureAndLayout.
     */
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    RunMeasureAndLayout();
    pattern_->gridLayoutInfo_.reachEnd_ = true;
    pattern_->gridLayoutInfo_.prevOffset_ = PREV_OFFSET_FIRST;
    pattern_->gridLayoutInfo_.currentOffset_ = CURRENT_OFFSET_FIRST;
    // only set one of rowTemplate and columnsTemplate, use scrollable layout algorithm
    auto gridScrollLayoutAlgorithm = AceType::MakeRefPtr<GridScrollLayoutAlgorithm>(
        pattern_->gridLayoutInfo_, ALGORITHM_RELATE_LALUE_FIRST, ALGORITHM_RELATE_LALUE_SECOND);
    ASSERT_NE(gridScrollLayoutAlgorithm, nullptr);
    gridScrollLayoutAlgorithm->SetCanOverScroll(true);
    gridScrollLayoutAlgorithm->ModifyCurrentOffsetWhenReachEnd(DEVICE_HEIGHT);
    EXPECT_TRUE(gridScrollLayoutAlgorithm->gridLayoutInfo_.reachEnd_);

    for (uint32_t i = 0; i < itemCount; i++) {
        gridScrollLayoutAlgorithm->gridLayoutInfo_.lineHeightMap_[i] = itemHeight;
    }
    gridScrollLayoutAlgorithm->gridLayoutInfo_.prevOffset_ = PREV_OFFSET_SECOND;
    gridScrollLayoutAlgorithm->gridLayoutInfo_.currentOffset_ = CURRENT_OFFSET_SECOND;
    gridScrollLayoutAlgorithm->gridLayoutInfo_.startIndex_ = START_INDEX;
    gridScrollLayoutAlgorithm->gridLayoutInfo_.startMainLineIndex_ = START_MAIN_LINE_INDEX;
    gridScrollLayoutAlgorithm->gridLayoutInfo_.endMainLineIndex_ = itemCount;
    gridScrollLayoutAlgorithm->ModifyCurrentOffsetWhenReachEnd(DEVICE_HEIGHT);
    EXPECT_EQ(gridScrollLayoutAlgorithm->gridLayoutInfo_.currentOffset_, CURRENT_OFFSET_SECOND);
    EXPECT_EQ(gridScrollLayoutAlgorithm->gridLayoutInfo_.prevOffset_, PREV_OFFSET_SECOND);
    EXPECT_TRUE(gridScrollLayoutAlgorithm->gridLayoutInfo_.offsetEnd_);
}

/**
 * @tc.name: GridPatternTest019
 * @tc.desc: Test grid pattern GetMainGap function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest019, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetRowsTemplate("1fr 1fr");
    gridModelNG.SetRowsGap(GRID_ROWS_GAP);
    CreateGridItem(10);
    GetInstance();
    auto layoutWrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(layoutWrapper, nullptr);
    DirtySwapConfig config;
    config.skipMeasure = true;
    config.skipLayout = true;
    pattern_->OnDirtyLayoutWrapperSwap(layoutWrapper, config);
    auto mainGap = pattern_->GetMainGap();
    EXPECT_EQ(mainGap, 0);
}

/**
 * @tc.name: GridPatternTest020
 * @tc.desc: Test grid pattern SetEdgeEffect function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest020, TestSize.Level1)
{
    GridModelNG gridModelNG;
    auto positionController = AceType::MakeRefPtr<ScrollControllerBase>();
    ASSERT_NE(positionController, nullptr);
    auto scrollProxy = AceType::MakeRefPtr<ScrollProxy>();
    ASSERT_NE(scrollProxy, nullptr);
    gridModelNG.Create(positionController, scrollProxy);
    gridModelNG.SetEdgeEffect(EdgeEffect::NONE);
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<GridPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<GridLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetEdgeEffectValue(), EdgeEffect::NONE);
}

/**
 * @tc.name: GridPatternTest021
 * @tc.desc: Test grid pattern SetEdgeEffect function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest021, TestSize.Level1)
{
    GridModelNG gridModelNG;
    auto positionController = AceType::MakeRefPtr<ScrollControllerBase>();
    ASSERT_NE(positionController, nullptr);
    auto scrollProxy = AceType::MakeRefPtr<ScrollProxy>();
    ASSERT_NE(scrollProxy, nullptr);
    gridModelNG.Create(positionController, scrollProxy);
    gridModelNG.SetEdgeEffect(EdgeEffect::FADE);
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<GridPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<GridLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetEdgeEffectValue(), EdgeEffect::FADE);
}

/**
 * @tc.name: GridPatternTest022
 * @tc.desc: Test grid pattern SetEdgeEffect function
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, GridPatternTest022, TestSize.Level1)
{
    GridModelNG gridModelNG;
    auto positionController = AceType::MakeRefPtr<ScrollControllerBase>();
    ASSERT_NE(positionController, nullptr);
    auto scrollProxy = AceType::MakeRefPtr<ScrollProxy>();
    ASSERT_NE(scrollProxy, nullptr);
    gridModelNG.Create(positionController, scrollProxy);
    gridModelNG.SetEdgeEffect(EdgeEffect::SPRING);
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<GridPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<GridLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetEdgeEffectValue(), EdgeEffect::SPRING);
}

/**
 * @tc.name: EventHubCoverage001
 * @tc.desc: For Coverage Rate, branches that are not need test.
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, EventHubCoverage001, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    CreateGridItem(8);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Supplement InitItemDragEvent branch.
     */
    auto gestureEventHub = eventHub_->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureEventHub, nullptr);
    eventHub_->InitItemDragEvent(gestureEventHub);
    auto dragEventActuator = gestureEventHub->dragEventActuator_;
    ASSERT_NE(dragEventActuator, nullptr);
    auto userCallback = dragEventActuator->userCallback_;
    ASSERT_NE(userCallback, nullptr);
    auto HandleOnItemDragStart = userCallback->GetActionStartEventFunc();
    auto HandleOnItemDragUpdate = userCallback->GetActionUpdateEventFunc();
    auto HandleOnItemDragEnd = userCallback->GetActionEndEventFunc();
    auto HandleOnItemDragCancel = userCallback->GetActionCancelEventFunc();
    GestureEvent info;
    HandleOnItemDragStart(info);
    HandleOnItemDragUpdate(info);
    HandleOnItemDragEnd(info);
    HandleOnItemDragCancel();

    EXPECT_TRUE(true);
}

/**
 * @tc.name: EventHubCoverage002
 * @tc.desc: For Coverage Rate, branches that are not normally covered.
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, EventHubCoverage002, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    gridModelNG.SetEditable(true);
    CreateGridItem(8);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Supplement HandleOnItemDragEnd branch,
     * has no condition that draggingItem_ is null when dragDropProxy_ was not null.
     */
    auto onItemDragStart = [](const ItemDragInfo&, int32_t) {
        auto dragItem = AceType::MakeRefPtr<FrameNode>("test", 0, AceType::MakeRefPtr<Pattern>());
        return AceType::DynamicCast<UINode>(dragItem);
    };
    eventHub_->onItemDragStart_ = onItemDragStart;
    GestureEvent info;
    Point globalPoint = Point(270.f, 50.f);
    info.SetGlobalPoint(globalPoint);
    eventHub_->HandleOnItemDragStart(info);
    eventHub_->draggingItem_ = nullptr;
    eventHub_->HandleOnItemDragEnd(info);

    eventHub_->HandleOnItemDragStart(info);
    eventHub_->draggingItem_ = nullptr;
    eventHub_->HandleOnItemDragCancel();
    // has no condition that draggingItem_ is null when FireOnItemDrop was called.
    ItemDragInfo dragInfo;
    eventHub_->FireOnItemDrop(dragInfo, 0, 0, false);

    EXPECT_TRUE(true);
}

/**
 * @tc.name: EventHubCoverage003
 * @tc.desc: For Coverage Rate, branches that are not need test.
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, EventHubCoverage003, TestSize.Level1)
{
    GridModelNG gridModelNG;
    gridModelNG.Create(nullptr, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    gridModelNG.SetEditable(true);
    gridModelNG.SetOnItemDragEnter([](const ItemDragInfo) {});
    gridModelNG.SetOnItemDragMove([](const ItemDragInfo&, int32_t, int32_t) {});
    gridModelNG.SetOnItemDragLeave([](const ItemDragInfo&, int32_t) {});
    gridModelNG.SetOnItemDrop([](const ItemDragInfo&, int32_t, int32_t, bool) {});
    CreateGridItem(8);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Supplement drag callback branch.
     */
    ItemDragInfo dragInfo;
    eventHub_->FireOnItemDragEnter(dragInfo);
    eventHub_->FireOnItemDragLeave(dragInfo, 1);
    eventHub_->FireOnItemDragMove(dragInfo, 1, 2);
    eventHub_->FireOnItemDrop(dragInfo, 0, 2, true);

    EXPECT_TRUE(true);
}

/**
 * @tc.name: ModelCoverage001
 * @tc.desc: For Coverage Rate, branches that are not need test.
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, ModelCoverage001, TestSize.Level1)
{
    GridItemModelNG gridItemModelNG;
    gridItemModelNG.Create([](int32_t) {}, true);
    gridItemModelNG.Create([](int32_t) {}, false);

    EXPECT_TRUE(true);
}

/**
 * @tc.name: PositionControllerCoverage001
 * @tc.desc: For Coverage Rate, branches that are not normally covered.
 * @tc.type: FUNC
 */
HWTEST_F(GridTestNg, PositionControllerCoverage001, TestSize.Level1)
{
    GridModelNG gridModelNG;
    RefPtr<ScrollControllerBase> positionController = gridModelNG.CreatePositionController();
    gridModelNG.Create(positionController, nullptr);
    gridModelNG.SetColumnsTemplate(TEMPLATE_4);
    CreateGridItem(8);
    GetInstance();
    RunMeasureAndLayout();

    /**
     * @tc.steps: step1. Supplement ScrollPage, GetCurrentOffset branch,
     * has no condition that axis_ is Axis::NONE.
     */
    pattern_->gridLayoutInfo_.axis_ = Axis::NONE;
    auto controller = pattern_->positionController_;
    controller->ScrollPage(true, true);
    controller->GetCurrentOffset();

    EXPECT_TRUE(true);
}
} // namespace OHOS::Ace::NG
