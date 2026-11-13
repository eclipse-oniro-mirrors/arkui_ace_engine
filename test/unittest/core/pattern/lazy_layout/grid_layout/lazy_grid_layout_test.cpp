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

#include "interfaces/inner_api/ui_session/param_config.h"
#include "lazy_grid_layout_test.h"

#include <vector>

#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/unittest/core/pattern/lazy_layout/lazy_layout_test_utils.h"

#include "core/components_ng/pattern/lazy_grid_layout/lazy_grid_layout_model.h"
#include "core/components_ng/pattern/lazy_grid_layout/lazy_grid_layout_property.h"
#include "core/components_ng/pattern/lazy_grid_layout/lazy_grid_layout_algorithm.h"
#include "core/components_ng/pattern/lazy_layout/lazy_layout_utils.h"
#include "core/components_ng/pattern/lazy_layout/prepared_layout_geometry.h"
#include "core/components_ng/pattern/list/list_model_ng.h"
#include "core/components_ng/pattern/list/list_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_model_ng.h"
#include "core/components_ng/pattern/waterflow/water_flow_model_ng.h"
#include "core/components_ng/pattern/waterflow/water_flow_pattern.h"
#include "core/components_ng/pattern/waterflow/water_flow_layout_property.h"
#include "core/components_ng/pattern/stack/stack_model_ng.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"
#include "test/unittest/core/syntax/mock_lazy_for_each_builder.h"
#undef private

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t PREMEASURE_ITEM_COUNT = 10;
constexpr int32_t VISIBLE_CUSTOM_INDEX = 5;
constexpr int32_t REJECTED_HEADER_NODE_ID = 4900;
constexpr int32_t REJECTED_CHILD_NODE_ID = 4901;
constexpr int32_t PREMEASURE_LANES = 2;
constexpr int32_t REJECTED_START_INDEX = 4;
constexpr int32_t REJECTED_END_INDEX = 5;
constexpr int32_t COMMITTED_ITEM_COUNT = 16;
constexpr int32_t COMMITTED_NODE_ID = 5200;
constexpr int32_t COMMITTED_TALL_ROW = 6;
constexpr int32_t COMMITTED_REJECTED_INDEX = 11;
} // namespace


namespace {
using VisibleRange = std::pair<int32_t, int32_t>;

class PremeasureHookPattern final : public Pattern {
    DECLARE_ACE_TYPE(PremeasureHookPattern, Pattern);
public:
    int32_t beforeMeasureCount = 0;

    void BeforeCreateLayoutWrapper() override
    {
        ++beforeMeasureCount;
        GetHost()->GetLayoutProperty()->UpdateUserDefinedIdealSize(
            CalcSize(CalcLength(100.0f), CalcLength(125.0f)));
    }
};

class PremeasureDeferredAlgorithm final : public BoxLayoutAlgorithm {
    DECLARE_ACE_TYPE(PremeasureDeferredAlgorithm, BoxLayoutAlgorithm);
public:
    bool MeasureInNextFrame() const override { return true; }
};

class PremeasureLazyBuilder final : public Framework::MockLazyForEachBuilder {
public:
    RefPtr<CustomNode> visibleItem;

protected:
    int32_t OnGetTotalCount() override { return PREMEASURE_ITEM_COUNT; }

    std::pair<std::string, RefPtr<UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, LazyForEachCacheChild>&) override
    {
        auto custom = CustomNode::CreateCustomNode(-4700 - index, std::to_string(index));
        auto item = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG, -4800 - index, AceType::MakeRefPtr<Pattern>());
        item->GetLayoutProperty()->UpdateUserDefinedIdealSize(CalcSize(CalcLength(100.0f), CalcLength(100.0f)));
        custom->AddChild(item);
        if (index == VISIBLE_CUSTOM_INDEX) {
            visibleItem = custom;
        }
        return { std::to_string(index), custom };
    }
};
}

void LazyGridLayoutTest::SetUpTestSuite()
{
    TestNG::SetUpTestSuite();
    MockPipelineContext::GetCurrent()->SetUseFlushUITasks(true);
    MockPipelineContext::GetCurrentContext()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    MockAnimationManager::Enable(true);
    g_segmentedWaterflow = false;
}

void LazyGridLayoutTest::TearDownTestSuite()
{
    TestNG::TearDownTestSuite();
}

void LazyGridLayoutTest::SetUp() {}

void LazyGridLayoutTest::TearDown()
{
    RemoveFromStageNode();
    frameNode_ = nullptr;
    pattern_ = nullptr;
    layoutProperty_ = nullptr;
    AceApplicationInfo::GetInstance().isRightToLeft_ = false;
    MockAnimationManager::GetInstance().Reset();
}

RefPtr<FrameNode> LazyGridLayoutTest::GetMainFrameNode()
{
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->GetMainElementNode();
    return AceType::DynamicCast<FrameNode>(element);
}

void LazyGridLayoutTest::CreateStack(Axis axis)
{
    StackModelNG stackModel;
    stackModel.Create();
    if (axis == Axis::HORIZONTAL) {
        ViewAbstract::SetHeight(CalcLength(1, DimensionUnit::PERCENT));
        ViewAbstract::SetWidth(CalcLength(ITEM_HEIGHT));
    } else {
        ViewAbstract::SetWidth(CalcLength(1, DimensionUnit::PERCENT));
        ViewAbstract::SetHeight(CalcLength(ITEM_HEIGHT));
    }
}

void LazyGridLayoutTest::CreateLazyGridLayout(Axis axis)
{
    LazyVGridLayoutModel model;
    model.Create();
    if (axis == Axis::HORIZONTAL) {
        ViewAbstract::SetHeight(CalcLength(SCROLL_WIDTH));
    } else {
        ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    }
    model.SetColumnsTemplate("1fr 1fr");
    frameNode_ = GetMainFrameNode();
    pattern_ = frameNode_->GetPattern<LazyGridLayoutPattern>();
    pattern_->axis_ = axis;
    layoutProperty_ = frameNode_->GetLayoutProperty<LazyGridLayoutProperty>();
}

void LazyGridLayoutTest::CreateContent(int32_t count)
{
    for (int32_t i = 0; i < count; i++) {
        CreateStack();
        ViewStackProcessor::GetInstance()->Pop();
    }
}

void LazyGridLayoutTest::CreateWaterFlow(WaterFlowLayoutMode mode)
{
    WaterFlowModelNG model;
    model.Create();
    ViewAbstract::SetHeight(CalcLength(SCROLL_HEIGHT));
    ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    model.SetLayoutMode(mode);
    scrollableFrameNode_ = GetMainFrameNode();
    scrollablePattern_ = scrollableFrameNode_->GetPattern<ScrollablePattern>();
}

void LazyGridLayoutTest::CreateList()
{
    ListModelNG model;
    model.Create();
    ViewAbstract::SetHeight(CalcLength(SCROLL_HEIGHT));
    ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    scrollableFrameNode_ = GetMainFrameNode();
    scrollablePattern_ = scrollableFrameNode_->GetPattern<ScrollablePattern>();
    listFrameNode_ = scrollableFrameNode_;
    listPattern_ = listFrameNode_->GetPattern<ListPattern>();
}

void LazyGridLayoutTest::CreateScroll()
{
    ScrollModelNG model;
    model.Create();
    ViewAbstract::SetHeight(CalcLength(SCROLL_HEIGHT));
    ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    scrollableFrameNode_ = GetMainFrameNode();
    scrollablePattern_ = scrollableFrameNode_->GetPattern<ScrollablePattern>();
}

PaddingProperty LazyGridLayoutTest::CreatePadding(float left, float top, float right, float bottom)
{
    PaddingProperty padding;
    padding.left = CalcLength(left);
    padding.right = CalcLength(right);
    padding.top = CalcLength(top);
    padding.bottom = CalcLength(bottom);
    return padding;
}

void LazyGridLayoutTest::AddChild()
{
    CreateStack(Axis::VERTICAL);
    RefPtr<UINode> currentNode = ViewStackProcessor::GetInstance()->Finish();
    auto currentFrameNode = AceType::DynamicCast<FrameNode>(currentNode);
    currentFrameNode->MountToParent(frameNode_);
}

void LazyGridLayoutTest::FlushIdleTask(const RefPtr<LazyGridLayoutPattern>& pattern)
{
    int32_t tryCount = 10;
    auto needPredict = pattern->layoutInfo_->NeedPredict();
    while (needPredict && tryCount > 0) {
        const int64_t time = GetSysTimestamp();
        auto pipeline = pattern->GetContext();
        pipeline->OnIdle(time + 16 * 1000000); // 16 * 1000000: 16ms
        FlushUITasks();
        needPredict = pattern->layoutInfo_->NeedPredict();
        tryCount--;
    }
}

/**
 * @tc.name: ColumnsTemplateTest001
 * @tc.desc: Test ColumnsTemplate.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, ColumnsTemplateTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     * @tc.expected: layout visible items and predict items
     */
    CreateWaterFlow();
    CreateLazyGridLayout();
    CreateContent(30);
    CreateDone();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1500);
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Width(), 200);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 2);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 13);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 13);

    /**
     * @tc.steps: step2. UpdateColumnsTemplate 1fr
     * @tc.expected: start and end index updated
     */
    layoutProperty_->UpdateColumnsTemplate("1fr");
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 3000);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 1);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 6);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 6);

    /**
     * @tc.steps: step3. UpdateColumnsTemplate 1fr 1fr 1fr
     * @tc.expected: start and end index updated
     */
    layoutProperty_->UpdateColumnsTemplate("1fr 1fr 1fr");
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1000);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 3);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 14);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 20);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 20);
}

/**
 * @tc.name: ColumnsTemplateTest002
 * @tc.desc: Test ColumnsTemplate repeat mode.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, ColumnsTemplateTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     */
    CreateWaterFlow();
    CreateLazyGridLayout();
    CreateContent(30);
    CreateDone();

    /**
     * @tc.steps: step2. UpdateColumnsTemplate 'repeat(auto-fit, x)'
     * @tc.expected: start and end index updated
     */
    layoutProperty_->UpdateColumnsTemplate("repeat(auto-fit, 80)");
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1500);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 2);
    EXPECT_EQ(GetChildWidth(frameNode_, 0), 100);
    EXPECT_EQ(GetChildX(frameNode_, 1), 100);
    layoutProperty_->UpdateColumnsTemplate("repeat(auto-fit, 120)");
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 3000);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 1);
    EXPECT_EQ(GetChildWidth(frameNode_, 0), 200);

    /**
     * @tc.steps: step3. UpdateColumnsTemplate 'repeat(auto-fit, x)'
     * @tc.expected: start and end index updated
     */
    layoutProperty_->UpdateColumnsTemplate("repeat(auto-fill, 80)");
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1500);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 2);
    EXPECT_EQ(GetChildWidth(frameNode_, 0), 80);
    EXPECT_EQ(GetChildX(frameNode_, 1), 80);
    layoutProperty_->UpdateColumnsTemplate("repeat(auto-fill, 120)");
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 3000);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 1);
    EXPECT_EQ(GetChildWidth(frameNode_, 0), 120);

    /**
     * @tc.steps: step4. UpdateColumnsTemplate 'repeat(auto-stretch, x)'
     * @tc.expected: start and end index updated
     */
    layoutProperty_->UpdateColumnsTemplate("repeat(auto-stretch, 80)");
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1500);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 2);
    EXPECT_EQ(GetChildWidth(frameNode_, 0), 80);
    EXPECT_EQ(GetChildX(frameNode_, 1), 120);
    layoutProperty_->UpdateColumnsTemplate("repeat(auto-stretch, 60)");
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1000);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 3);
    EXPECT_EQ(GetChildWidth(frameNode_, 0), 60);
}

/**
 * @tc.name: ColumnsGapTest001
 * @tc.desc: Test ColumnsGap.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, ColumnsGapTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     */
    CreateWaterFlow();
    CreateLazyGridLayout();
    CreateContent(30);
    CreateDone();

    /**
     * @tc.steps: step2. UpdateColumnGap 20
     * @tc.expected: start and end index updated
     */
    layoutProperty_->UpdateColumnGap(Dimension(20));
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1500);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 2);
    EXPECT_EQ(GetChildWidth(frameNode_, 0), 90);
    EXPECT_EQ(GetChildX(frameNode_, 1), 110);
}

/**
 * @tc.name: RowsGapTest001
 * @tc.desc: Test RowsGap.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, RowsGapTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     */
    CreateWaterFlow();
    CreateLazyGridLayout();
    CreateContent(30);
    CreateDone();

    /**
     * @tc.steps: step2. UpdateColumnGap 20
     * @tc.expected: start and end index updated
     */
    layoutProperty_->UpdateRowGap(Dimension(20));
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1780);
    EXPECT_EQ(pattern_->layoutInfo_->lanes_, 2);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 7);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 11);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 11);
}

/**
 * @tc.name: RowsGapTest002
 * @tc.desc: Shrink rowGap repeatedly while scrolled to the bottom; the last row stays bottom-pinned
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, RowsGapTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Scroll > LazyVGrid with 20 items (2 lanes) and rowGap = 30
     * @tc.expected: 10 rows, totalMainSize = 10 * ITEM_HEIGHT + 9 * 30 = 1270
     */
    CreateScroll();
    CreateLazyGridLayout();
    auto rowGap = 30.0f;
    LazyVGridLayoutModel::SetRowGap(AceType::RawPtr(frameNode_), Dimension(rowGap));
    CreateContent(20);
    CreateDone();

    /**
     * @tc.steps: step2. Scroll to the bottom
     * @tc.expected: last row (index 18/19) bottom-aligned, on-screen top y = SCROLL_HEIGHT - ITEM_HEIGHT = 350
     */
    scrollablePattern_->UpdateCurrentOffset(-1000, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 12);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 19);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), SCROLL_HEIGHT - 10 * ITEM_HEIGHT - 9 * rowGap);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 19), 350);

    /**
     * @tc.steps: step3. Shrink rowGap 30 -> 20
     * @tc.expected: last row stays bottom-pinned, on-screen top y stays 350 (no jump)
     */
    rowGap = 20.0f;
    LazyVGridLayoutModel::SetRowGap(AceType::RawPtr(frameNode_), Dimension(rowGap));
    FlushUITasks(scrollableFrameNode_);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 12);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 19);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), SCROLL_HEIGHT - 10 * ITEM_HEIGHT - 9 * rowGap);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 19), 350);

    /**
     * @tc.steps: step4. Shrink rowGap 20 -> 10
     * @tc.expected: last row on-screen top y stays 350
     */
    rowGap = 10.0f;
    LazyVGridLayoutModel::SetRowGap(AceType::RawPtr(frameNode_), Dimension(rowGap));
    FlushUITasks(scrollableFrameNode_);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 10);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 19);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), SCROLL_HEIGHT - 10 * ITEM_HEIGHT - 9 * rowGap);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 19), 350);

    /**
     * @tc.steps: step5. Shrink rowGap 10 -> 0
     * @tc.expected: last row on-screen top y stays 350
     */
    rowGap = 0.0f;
    LazyVGridLayoutModel::SetRowGap(AceType::RawPtr(frameNode_), Dimension(rowGap));
    FlushUITasks(scrollableFrameNode_);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 10);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 19);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), SCROLL_HEIGHT - 10 * ITEM_HEIGHT - 9 * rowGap);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 19), 350);
}

/**
 * @tc.name: PaddingMarginTest001
 * @tc.desc: Test padding and margin.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PaddingMarginTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     */
    CreateWaterFlow();
    CreateLazyGridLayout();
    CreateContent(30);
    CreateDone();

    /**
     * @tc.steps: step2. Update padding top and bottom.
     * @tc.expected: start and end index updated
     */
    PaddingProperty padding = CreatePadding(0, 100, 0, 100);;
    layoutProperty_->UpdatePadding(padding);
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1700);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 7);

    /**
     * @tc.steps: step3. Update margin top and bottom.
     * @tc.expected: start and end index updated
     */
    PaddingProperty margin = CreatePadding(0, 100, 0, 100);;
    layoutProperty_->UpdateMargin(margin);
    FlushUITasks();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1700);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 5);
}

/**
 * @tc.name: RTLTest001
 * @tc.desc: Test RTL layout.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, RTLTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     */
    CreateWaterFlow();
    CreateLazyGridLayout();
    CreateContent(30);
    CreateDone();

    /**
     * @tc.steps: step2. Update padding top and bottom.
     * @tc.expected: start and end index updated
     */
    layoutProperty_->UpdateLayoutDirection(TextDirection::RTL);
    FlushUITasks();
    EXPECT_EQ(GetChildX(frameNode_, 0), 100);
    EXPECT_EQ(GetChildX(frameNode_, 1), 0);
    EXPECT_EQ(GetChildX(frameNode_, 2), 100);
    EXPECT_EQ(GetChildX(frameNode_, 3), 0);
}

/**
 * @tc.name: WidthHeightTest001
 * @tc.desc: Test LazyGrid set width height.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, WidthHeightTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     */
    CreateWaterFlow();
    CreateLazyGridLayout();
    CreateContent(30);
    CreateDone();

    /**
     * @tc.steps: step2. Update width and height.
     * @tc.expected: width and height updated
     */
    auto size = CalcSize(CalcLength(160), CalcLength(750));
    layoutProperty_->UpdateUserDefinedIdealSize(size);
    FlushUITasks();
    EXPECT_EQ(GetChildWidth(scrollableFrameNode_, 0), 160);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 750);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
    EXPECT_EQ(GetChildX(frameNode_, 1), 80);

    /**
     * @tc.steps: step3. Update width and height.
     * @tc.expected: width and height updated
     */
    size = CalcSize(CalcLength(240), CalcLength(150));
    layoutProperty_->UpdateUserDefinedIdealSize(size);
    FlushUITasks();
    EXPECT_EQ(GetChildWidth(scrollableFrameNode_, 0), 240);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 150);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
    EXPECT_EQ(GetChildX(frameNode_, 1), 120);

    /**
     * @tc.steps: step4. Reset user defined ideal size.
     * @tc.expected: width and height updated
     */
    layoutProperty_->ClearUserDefinedIdealSize(true, true);
    FlushUITasks();
    EXPECT_EQ(GetChildWidth(scrollableFrameNode_, 0), 200);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 1500);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
    EXPECT_EQ(GetChildX(frameNode_, 1), 100);
}

/**
 * @tc.name: SizeConstraintTest001
 * @tc.desc: Test LazyGrid set size constraint.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, SizeConstraintTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     */
    CreateWaterFlow();
    CreateLazyGridLayout();
    CreateContent(30);
    CreateDone();

    /**
     * @tc.steps: step2. Update max size.
     * @tc.expected: width and height updated
     */
    auto size = CalcSize(CalcLength(160), CalcLength(750));
    layoutProperty_->UpdateCalcMaxSize(size);
    FlushUITasks();
    EXPECT_EQ(GetChildWidth(scrollableFrameNode_, 0), 160);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 750);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
    EXPECT_EQ(GetChildX(frameNode_, 1), 80);

    /**
     * @tc.steps: step3. Update min size.
     * @tc.expected: width and height updated
     */
    layoutProperty_->ResetCalcMaxSize();
    size = CalcSize(CalcLength(240), CalcLength(1800));
    layoutProperty_->UpdateCalcMinSize(size);
    FlushUITasks();
    EXPECT_EQ(GetChildWidth(scrollableFrameNode_, 0), 240);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 1800);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
    EXPECT_EQ(GetChildX(frameNode_, 1), 120);

    /**
     * @tc.steps: step4. reset min size.
     * @tc.expected: width and height updated
     */
    layoutProperty_->ResetCalcMinSize();
    FlushUITasks();
    EXPECT_EQ(GetChildWidth(scrollableFrameNode_, 0), 200);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 1500);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
    EXPECT_EQ(GetChildX(frameNode_, 1), 100);
}

/**
 * @tc.name: WaterFlowLayoutTest001
 * @tc.desc: Lazy layout in WaterFlow
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, WaterFlowLayoutTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     * @tc.expected: layout visible items and predict items
     */
    CreateWaterFlow();
    CreateLazyGridLayout();
    CreateContent(30);
    CreateDone();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1500);
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Width(), 200);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 13);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 13);

    /**
     * @tc.steps: step2. Update refences position
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-120, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 2);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 11);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 15);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 15);

    /**
     * @tc.steps: step3. Scroll to bottom
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-1500, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 20);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 29);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 16);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 29);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 16);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 29);
}

/**
 * @tc.name: WaterFlowLayoutTest002
 * @tc.desc: Lazy layout in WaterFlow, waterFlow adjust offset.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, WaterFlowLayoutTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     * @tc.expected: layout visible items and predict items
     */
    CreateWaterFlow();
    CreateLazyGridLayout();
    CreateContent(30);
    CreateDone();

    /**
     * @tc.steps: step2. Update refences position
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-420, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 8);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 17);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 2);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 21);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 2);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 21);

    /**
     * @tc.steps: step3. UpdateColumnsTemplate
     * @tc.expected: waterFlow adjust offset
     */
    layoutProperty_->UpdateColumnsTemplate("1fr 1fr 1fr");
    FlushUITasks();
    EXPECT_EQ(scrollablePattern_->GetTotalOffset(), 220);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 6);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 20);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 26);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 26);
}

/**
 * @tc.name: WaterFlowLayoutTest003
 * @tc.desc: Lazy layout in WaterFlow SW mode, update LazyGrid column template above display area.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, WaterFlowLayoutTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     * @tc.expected: layout visible items and predict items
     */
    CreateWaterFlow(WaterFlowLayoutMode::SLIDING_WINDOW);
    CreateLazyGridLayout();
    CreateContent(19);
    ViewStackProcessor::GetInstance()->Pop();
    CreateLazyGridLayout();
    CreateContent(19);
    CreateDone();

    /**
     * @tc.steps: step2. Update refences position
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-420, SCROLL_FROM_UPDATE);
    FlushUITasks();
    auto pattern0 = GetChildPattern<LazyGridLayoutPattern>(scrollableFrameNode_, 0);
    auto pattern1 = GetChildPattern<LazyGridLayoutPattern>(scrollableFrameNode_, 1);
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 8);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 17);
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, -1);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, -1);
    FlushIdleTask(pattern0);
    EXPECT_EQ(pattern0->layoutInfo_->cachedStartIndex_, 2);
    EXPECT_EQ(pattern0->layoutInfo_->cachedEndIndex_, 18);
    EXPECT_EQ(pattern0->layoutInfo_->layoutedStartIndex_, 2);
    EXPECT_EQ(pattern0->layoutInfo_->layoutedEndIndex_, 18);

    /**
     * @tc.steps: step3. Update refences position, LazyGrid0 out of view.
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-600, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, 9);

    /**
     * @tc.steps: step3. Update LazyGrid0 lanes and scroll backward.
     * @tc.expected: start and end index updated
     */
    auto layoutProperty0 = pattern0->GetLayoutProperty<LazyGridLayoutProperty>();
    layoutProperty0->UpdateColumnsTemplate("1fr 1fr 1fr");
    scrollablePattern_->UpdateCurrentOffset(300, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 12);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 18);
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, 3);
}

/**
 * @tc.name: WaterFlowLayoutTest004
 * @tc.desc: Lazy layout in WaterFlow SW mode, update LazyGrid row gap above display area.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, WaterFlowLayoutTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     * @tc.expected: layout visible items and predict items
     */
    CreateWaterFlow(WaterFlowLayoutMode::SLIDING_WINDOW);
    CreateLazyGridLayout();
    CreateContent(19);
    ViewStackProcessor::GetInstance()->Pop();
    CreateLazyGridLayout();
    CreateContent(19);
    CreateDone();

    /**
     * @tc.steps: step2. Update refences position
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-420, SCROLL_FROM_UPDATE);
    FlushUITasks();
    auto pattern0 = GetChildPattern<LazyGridLayoutPattern>(scrollableFrameNode_, 0);
    auto pattern1 = GetChildPattern<LazyGridLayoutPattern>(scrollableFrameNode_, 1);
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 8);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 17);
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, -1);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, -1);
    FlushIdleTask(pattern0);
    EXPECT_EQ(pattern0->layoutInfo_->cachedStartIndex_, 2);
    EXPECT_EQ(pattern0->layoutInfo_->cachedEndIndex_, 18);
    EXPECT_EQ(pattern0->layoutInfo_->layoutedStartIndex_, 2);
    EXPECT_EQ(pattern0->layoutInfo_->layoutedEndIndex_, 18);

    /**
     * @tc.steps: step3. Update refences position, LazyGrid0 out of view.
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-600, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, 9);

    /**
     * @tc.steps: step3. Update LazyGrid0 lanes and scroll backward.
     * @tc.expected: start and end index updated
     */
    auto layoutProperty0 = pattern0->GetLayoutProperty<LazyGridLayoutProperty>();
    layoutProperty0->UpdateRowGap(Dimension(10));
    scrollablePattern_->UpdateCurrentOffset(300, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 14);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 18);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 1), 280);
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, 3);
}

/**
 * @tc.name: WaterFlowLayoutTest005
 * @tc.desc: First LazyGird only display bottom padding, Update column template.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, WaterFlowLayoutTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     */
    CreateWaterFlow(WaterFlowLayoutMode::SLIDING_WINDOW);
    CreateLazyGridLayout();
    PaddingProperty padding = CreatePadding(0, 100, 0, 100);;
    layoutProperty_->UpdatePadding(padding);
    CreateContent(19);
    ViewStackProcessor::GetInstance()->Pop();
    CreateLazyGridLayout();
    CreateContent(19);
    CreateDone();

    /**
     * @tc.steps: step2. Update padding top and bottom.
     * @tc.expected: start and end index updated
     */
    auto pattern0 = GetChildPattern<LazyGridLayoutPattern>(scrollableFrameNode_, 0);
    auto pattern1 = GetChildPattern<LazyGridLayoutPattern>(scrollableFrameNode_, 1);
    FlushUITasks();
    EXPECT_EQ(pattern0->GetHost()->GetGeometryNode()->GetFrameSize().Height(), 1200);
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 7);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), 0);

    /**
     * @tc.steps: step3. Update refences position
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-520, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 8);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 17);
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, -1);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, -1);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), -520);

    /**
     * @tc.steps: step4. Update refences position
     * @tc.expected: LazyGrid0 out of view.
     */
    scrollablePattern_->UpdateCurrentOffset(-600, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 19);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 19);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 1), 80);
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, 7);

    /**
     * @tc.steps: step5. Update columns template
     * @tc.expected: Display content not change.
     */
    auto layoutProperty0 = pattern0->GetLayoutProperty<LazyGridLayoutProperty>();
    layoutProperty0->UpdateColumnsTemplate("1fr 1fr 1fr");
    FlushUITasks();
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 19);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 19);
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, 7);

    /**
     * @tc.steps: step6. Scroll backward.
     * @tc.expected:  start and end index updated.
     */
    scrollablePattern_->UpdateCurrentOffset(300, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 12);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 18);
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, 1);
}

/**
 * @tc.name: WaterFlowLayoutTest006
 * @tc.desc: First LazyGird only display bottom padding, Update rows gap.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, WaterFlowLayoutTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     */
    CreateWaterFlow(WaterFlowLayoutMode::SLIDING_WINDOW);
    CreateLazyGridLayout();
    PaddingProperty padding = CreatePadding(0, 100, 0, 100);;
    layoutProperty_->UpdatePadding(padding);
    CreateContent(19);
    ViewStackProcessor::GetInstance()->Pop();
    CreateLazyGridLayout();
    CreateContent(19);
    CreateDone();

    /**
     * @tc.steps: step2. Update padding top and bottom.
     * @tc.expected: start and end index updated
     */
    auto pattern0 = GetChildPattern<LazyGridLayoutPattern>(scrollableFrameNode_, 0);
    auto pattern1 = GetChildPattern<LazyGridLayoutPattern>(scrollableFrameNode_, 1);
    FlushUITasks();
    EXPECT_EQ(pattern0->GetHost()->GetGeometryNode()->GetFrameSize().Height(), 1200);
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 7);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), 0);

    /**
     * @tc.steps: step3. Update refences position
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-520, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 8);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 17);
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, -1);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, -1);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), -520);

    /**
     * @tc.steps: step4. Update refences position
     * @tc.expected: LazyGrid0 out of view.
     */
    scrollablePattern_->UpdateCurrentOffset(-600, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 19);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 19);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 1), 80);
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, 7);

    /**
     * @tc.steps: step5. Update columns template
     * @tc.expected: Display content not change.
     */
    auto layoutProperty0 = pattern0->GetLayoutProperty<LazyGridLayoutProperty>();
    layoutProperty0->UpdateRowGap(Dimension(10));
    FlushUITasks();
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 1290);
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 19);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 19);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 1), 80);
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, 7);

    /**
     * @tc.steps: step6. Scroll backward.
     * @tc.expected:  start and end index updated.
     */
    scrollablePattern_->UpdateCurrentOffset(300, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 14);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 18);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 1), 380);
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, 1);
}

/**
 * @tc.name: WaterFlowLayoutTest007
 * @tc.desc: Lazy layout in WaterFlow SW mode, update LazyGrid all children height above display area.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, WaterFlowLayoutTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     * @tc.expected: layout visible items and predict items
     */
    CreateWaterFlow(WaterFlowLayoutMode::SLIDING_WINDOW);
    CreateLazyGridLayout();
    CreateContent(19);
    ViewStackProcessor::GetInstance()->Pop();
    CreateLazyGridLayout();
    CreateContent(19);
    CreateDone();
    auto pattern0 = GetChildPattern<LazyGridLayoutPattern>(scrollableFrameNode_, 0);
    auto pattern1 = GetChildPattern<LazyGridLayoutPattern>(scrollableFrameNode_, 1);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 1000);
    /**
     * @tc.steps: step2. Update refences position
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-420, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 8);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 17);
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, -1);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, -1);

    /**
     * @tc.steps: step3. Update refences position, LazyGrid0 out of view.
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-600, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, 9);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 1), -20);

    /**
     * @tc.steps: step3. Update LazyGrid0 lanes and scroll backward.
     * @tc.expected: start and end index updated
     */
    auto frameNode0 = GetChildFrameNode(scrollableFrameNode_, 0);
    for (auto& child : frameNode0->GetChildren()) {
        auto property = AceType::DynamicCast<FrameNode>(child)->GetLayoutProperty();
        property->UpdateUserDefinedIdealSize(CalcSize(CalcLength(SCROLL_WIDTH), CalcLength(200.f)));
    }
    scrollablePattern_->UpdateCurrentOffset(200, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 18);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 18);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 1), 180);
    scrollablePattern_->UpdateCurrentOffset(200, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 16);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 18);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 1), 380);
    scrollablePattern_->UpdateCurrentOffset(200, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 14);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 18);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 1), 580);
}

/**
 * @tc.name: WaterFlowLayoutTest008
 * @tc.desc: Test LazyGrid child different height
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, WaterFlowLayoutTest008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     * @tc.expected: layout visible items and predict items
     */
    CreateWaterFlow(WaterFlowLayoutMode::SLIDING_WINDOW);
    CreateLazyGridLayout();
    layoutProperty_->UpdateColumnsTemplate("1fr 1fr 1fr");
    float itemHeight[3] = {80, 100, 90};
    for (int32_t i = 0; i < 10; i++) {
        for (int32_t j = 0; j < 3; j++) {
            StackModelNG stackModel;
            stackModel.Create();
            ViewAbstract::SetWidth(CalcLength(1, DimensionUnit::PERCENT));
            ViewAbstract::SetHeight(CalcLength(itemHeight[j]));
            ViewStackProcessor::GetInstance()->Pop();
        }
    }
    CreateDone();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1000);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 14);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 20);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 20);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEnd_, 700);

    /**
     * @tc.steps: step2. Update refences position.
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-420, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 12);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 26);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 3);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 29);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 3);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStart_, 100);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 29);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEnd_, 1000);
}

/**
 * @tc.name: WaterFlowLayoutTest009
 * @tc.desc: Test LazyGrid child different height, predict backward.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, WaterFlowLayoutTest009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     * @tc.expected: layout visible items and predict items
     */
    CreateWaterFlow(WaterFlowLayoutMode::SLIDING_WINDOW);
    CreateLazyGridLayout();
    layoutProperty_->UpdateColumnsTemplate("1fr 1fr 1fr");
    float itemHeight[3] = {80, 100, 90};
    for (int32_t i = 0; i < 10; i++) {
        for (int32_t j = 0; j < 3; j++) {
            StackModelNG stackModel;
            stackModel.Create();
            ViewAbstract::SetWidth(CalcLength(1, DimensionUnit::PERCENT));
            ViewAbstract::SetHeight(CalcLength(itemHeight[j]));
            ViewStackProcessor::GetInstance()->Pop();
        }
    }
    ViewStackProcessor::GetInstance()->Pop();
    CreateLazyGridLayout();
    CreateContent(20);
    CreateDone();
    auto pattern0 = GetChildPattern<LazyGridLayoutPattern>(scrollableFrameNode_, 0);
    auto pattern1 = GetChildPattern<LazyGridLayoutPattern>(scrollableFrameNode_, 1);
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 14);

    scrollablePattern_->ScrollToIndex(1);
    FlushUITasks();
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, 9);

    /**
     * @tc.steps: step2. Update refences position.
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(220, SCROLL_FROM_UPDATE);
    FlushUITasks();
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 21);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 29);
    FlushIdleTask(pattern0);
    EXPECT_EQ(pattern0->layoutInfo_->cachedStartIndex_, 15);
    EXPECT_EQ(pattern0->layoutInfo_->cachedEndIndex_, 29);
    EXPECT_EQ(pattern0->layoutInfo_->layoutedStartIndex_, 15);
    EXPECT_EQ(pattern0->layoutInfo_->layoutedStart_, 500);
    EXPECT_EQ(pattern0->layoutInfo_->layoutedEndIndex_, 29);
    EXPECT_EQ(pattern0->layoutInfo_->layoutedEnd_, 1000);
}

/**
 * @tc.name: AddDelChildrenTest001
 * @tc.desc: Test add and del children in LazyVGridLayout
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, AddDelChildrenTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     */
    CreateWaterFlow();
    CreateLazyGridLayout();
    CreateContent(30);
    CreateDone();
    EXPECT_EQ(pattern_->layoutInfo_->totalMainSize_, 1500);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 30);

    /**
     * @tc.steps: step2. Remove 10 children
     * @tc.expected: totalMainSize_ and totalItemCount_ index updated
     */
    for (int32_t i = 0; i < 10; i++) {
        frameNode_->RemoveChildAtIndex(0);
    }
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
    EXPECT_EQ(pattern_->layoutInfo_->totalMainSize_, 1000);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 20);

    /**
     * @tc.steps: step3. Add 6 children
     * @tc.expected: totalMainSize_ and totalItemCount_ index updated
     */
    for (int32_t i = 0; i < 6; i++) {
        AddChild();
    }
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->totalMainSize_, 1300);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 26);
}

/**
 * @tc.name: AddDelChildrenTest002
 * @tc.desc: Test add and del all children in LazyVGridLayout
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, AddDelChildrenTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy grid layout
     */
    CreateWaterFlow();
    CreateLazyGridLayout();
    CreateDone();
    EXPECT_EQ(pattern_->layoutInfo_->totalMainSize_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 0);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 0);

    /**
     * @tc.steps: step2. Add 6 children
     * @tc.expected: totalMainSize_ and totalItemCount_ index updated
     */
    for (int32_t i = 0; i < 6; i++) {
        AddChild();
    }
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->totalMainSize_, 300);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 6);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 300);

    /**
     * @tc.steps: step3. Remove 6 children
     * @tc.expected: totalMainSize_ and totalItemCount_ index updated
     */
    for (int32_t i = 0; i < 6; i++) {
        frameNode_->RemoveChildAtIndex(0);
    }
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->totalMainSize_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 0);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 0);
}

/**
 * @tc.name: AddDelChildrenTest003
 * @tc.desc: Repeatedly delete bottom children while scrolled to the bottom; the last row stays bottom-pinned
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, AddDelChildrenTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Scroll > LazyVGrid with 40 items (2 lanes)
     * @tc.expected: 20 rows, totalMainSize = 20 * ITEM_HEIGHT = 2000, scrollable range = 2000 - SCROLL_HEIGHT = 1550
     */
    CreateScroll();
    CreateLazyGridLayout();
    CreateContent(40);
    CreateDone();

    /**
     * @tc.steps: step2. Scroll to the bottom
     * @tc.expected: visible range 30-39 (last 5 rows), scroll offset = SCROLL_HEIGHT - 20 * ITEM_HEIGHT = -1550,
     *               last row bottom-aligned (on-screen top y = SCROLL_HEIGHT - ITEM_HEIGHT = 350)
     */
    scrollablePattern_->UpdateCurrentOffset(-2000, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 40);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 30);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 39);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), -1550);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 39), 350);

    /**
     * @tc.steps: step3. Delete the last 10 (bottom) children, then flush once
     * @tc.expected: the remaining 30 items (15 rows) become the whole content and the scroll re-clamps to the new
     *               bottom; totalItemCount = 30, offset = SCROLL_HEIGHT - 15 * ITEM_HEIGHT = -1050,
     *               last row on-screen top y stays 350
     */
    for (int32_t i = 0; i < 10; i++) {
        frameNode_->RemoveChild(frameNode_->GetLastChild());
    }
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(scrollableFrameNode_);
    FlushIdleTask(pattern_);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 30);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 20);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 29);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), -1050);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 29), 350);

    /**
     * @tc.steps: step4. Delete the last 10 (bottom) children, then flush once
     * @tc.expected: the remaining 20 items (10 rows) become the whole content and the scroll re-clamps to the new
     *               bottom; totalItemCount = 20, offset = SCROLL_HEIGHT - 10 * ITEM_HEIGHT = -550,
     *               last row on-screen top y stays 350
     */
    for (int32_t i = 0; i < 10; i++) {
        frameNode_->RemoveChild(frameNode_->GetLastChild());
    }
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(scrollableFrameNode_);
    FlushIdleTask(pattern_);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 20);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 10);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 19);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), -550);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 19), 350);

    /**
     * @tc.steps: step5. Delete the last 10 (bottom) children, then flush once
     * @tc.expected: the remaining 10 items (5 rows) become the whole content and the scroll re-clamps to the new
     *               bottom; totalItemCount = 10, offset = SCROLL_HEIGHT - 5 * ITEM_HEIGHT = -50,
     *               last row on-screen top y stays 350
     */
    for (int32_t i = 0; i < 10; i++) {
        frameNode_->RemoveChild(frameNode_->GetLastChild());
    }
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(scrollableFrameNode_);
    FlushIdleTask(pattern_);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 10);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 9);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), -50);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 9), 350);
}

/**
 * @tc.name: OnVisibleIndexesChangeTest001
 * @tc.desc: Verify callback fires with initial visible range.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, OnVisibleIndexesChangeTest001, TestSize.Level1)
{
    CreateWaterFlow();
    LazyVGridLayoutModel model;
    model.Create();
    ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    model.SetColumnsTemplate("1fr 1fr");
    frameNode_ = GetMainFrameNode();
    pattern_ = frameNode_->GetPattern<LazyGridLayoutPattern>();
    pattern_->axis_ = Axis::VERTICAL;
    layoutProperty_ = frameNode_->GetLayoutProperty<LazyGridLayoutProperty>();

    int32_t indexStart = 0;
    int32_t indexEnd = 0;
    auto onVisibleIndexesChangeCallback = [&indexStart, &indexEnd](int32_t start, int32_t end) {
        indexStart = start;
        indexEnd = end;
    };
    model.SetOnVisibleIndexesChange(onVisibleIndexesChangeCallback);

    CreateContent(30);
    CreateDone();

    EXPECT_EQ(indexStart, 0);
    EXPECT_EQ(indexEnd, 9);
}

/**
 * @tc.name: OnVisibleIndexesChangeTest002
 * @tc.desc: Verify callback returns -1,-1 when there are no child items.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, OnVisibleIndexesChangeTest002, TestSize.Level1)
{
    CreateWaterFlow();
    LazyVGridLayoutModel model;
    model.Create();
    ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    model.SetColumnsTemplate("1fr 1fr");

    int indexStart = 0;
    int indexEnd = 0;
    auto onVisibleIndexesChangeCallback = [&indexStart, &indexEnd](int32_t start, int32_t end) {
        indexStart = start;
        indexEnd = end;
    };
    model.SetOnVisibleIndexesChange(onVisibleIndexesChangeCallback);
    CreateDone();

    EXPECT_EQ(indexStart, -1);
    EXPECT_EQ(indexEnd, -1);
}

/**
 * @tc.name: OnVisibleIndexesChangeTest003
 * @tc.desc: Verify callback returns -1,-1 when LazyGrid becomes fully out of viewport above.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, OnVisibleIndexesChangeTest003, TestSize.Level1)
{
    CreateWaterFlow(WaterFlowLayoutMode::SLIDING_WINDOW);
    LazyVGridLayoutModel model;
    model.Create();
    ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    model.SetColumnsTemplate("1fr 1fr");

    int indexStart = 0;
    int indexEnd = 0;
    auto onVisibleIndexesChangeCallback = [&indexStart, &indexEnd](int32_t start, int32_t end) {
        indexStart = start;
        indexEnd = end;
    };
    model.SetOnVisibleIndexesChange(onVisibleIndexesChangeCallback);
    CreateContent(19);
    ViewStackProcessor::GetInstance()->Pop();
    CreateLazyGridLayout();
    CreateContent(19);
    CreateDone();

    scrollablePattern_->UpdateCurrentOffset(-420, SCROLL_FROM_UPDATE);
    FlushUITasks();
    scrollablePattern_->UpdateCurrentOffset(-600, SCROLL_FROM_UPDATE);
    FlushUITasks();

    EXPECT_EQ(indexStart, -1);
    EXPECT_EQ(indexEnd, -1);
}

/**
 * @tc.name: IsVerticalScrollableParent001
 * @tc.desc: Verify IsVerticalScrollableParent returns false for null node
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, IsVerticalScrollableParent001, TestSize.Level1)
{
    CreateWaterFlow();
    CreateLazyGridLayout();
    CreateDone();
    EXPECT_NE(pattern_, nullptr);
    RefPtr<UINode> nullNode = nullptr;
    bool result = LazyLayoutUtils::IsVerticalScrollableParent(nullNode);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsVerticalScrollableParent002
 * @tc.desc: Verify IsVerticalScrollableParent returns true for vertical WaterFlow
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, IsVerticalScrollableParent002, TestSize.Level1)
{
    CreateWaterFlow();
    CreateLazyGridLayout(Axis::VERTICAL);
    CreateDone();
    EXPECT_NE(pattern_, nullptr);
    bool result = LazyLayoutUtils::IsVerticalScrollableParent(scrollableFrameNode_);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsVerticalScrollableParent003
 * @tc.desc: Verify IsVerticalScrollableParent returns true for vertical List
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, IsVerticalScrollableParent003, TestSize.Level1)
{
    ListModelNG listModel;
    listModel.Create();
    ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    ViewAbstract::SetHeight(CalcLength(SCROLL_HEIGHT));
    auto listNode = GetMainFrameNode();
    EXPECT_NE(listNode, nullptr);
    auto listPattern = listNode->GetPattern<ListPattern>();
    EXPECT_NE(listPattern, nullptr);
    EXPECT_EQ(listPattern->GetAxis(), Axis::VERTICAL);
    CreateLazyGridLayout(Axis::VERTICAL);
    EXPECT_NE(pattern_, nullptr);
    CreateDone();

    bool result = LazyLayoutUtils::IsVerticalScrollableParent(listNode);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsVerticalScrollableParent004
 * @tc.desc: Verify IsVerticalScrollableParent returns true for vertical Scroll
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, IsVerticalScrollableParent004, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    ViewAbstract::SetHeight(CalcLength(SCROLL_HEIGHT));
    auto scrollNode = GetMainFrameNode();
    EXPECT_NE(scrollNode, nullptr);
    auto scrollPattern = scrollNode->GetPattern<ScrollablePattern>();
    EXPECT_NE(scrollPattern, nullptr);
    EXPECT_EQ(scrollPattern->GetAxis(), Axis::VERTICAL);
    CreateLazyGridLayout(Axis::VERTICAL);
    EXPECT_NE(pattern_, nullptr);
    CreateDone();

    bool result = LazyLayoutUtils::IsVerticalScrollableParent(scrollNode);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsVerticalScrollableParent005
 * @tc.desc: Verify IsVerticalScrollableParent returns false for non-scrollable container
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, IsVerticalScrollableParent005, TestSize.Level1)
{
    ScrollModelNG scrollModel;
    scrollModel.Create();
    ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    ViewAbstract::SetHeight(CalcLength(SCROLL_HEIGHT));
    auto scrollNode = GetMainFrameNode();
    EXPECT_NE(scrollNode, nullptr);
    auto scrollPattern = scrollNode->GetPattern<ScrollablePattern>();
    EXPECT_NE(scrollPattern, nullptr);
    EXPECT_EQ(scrollPattern->GetAxis(), Axis::VERTICAL);
    CreateLazyGridLayout(Axis::VERTICAL);
    EXPECT_NE(pattern_, nullptr);
    CreateDone();

    StackModelNG stackModel;
    stackModel.Create();
    auto stackNode = GetMainFrameNode();
    EXPECT_NE(stackNode, nullptr);
    CreateDone();

    bool result = LazyLayoutUtils::IsVerticalScrollableParent(stackNode);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: LazyGridInList001
 * @tc.desc: Verify LazyVGrid works in vertical List without abort
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInList001, TestSize.Level1)
{
    CreateList();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 10; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(80));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(listFrameNode_, nullptr);
    EXPECT_GE(listFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridInList002
 * @tc.desc: Verify List with LazyVGrid can scroll and offset updates
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInList002, TestSize.Level1)
{
    CreateList();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 30; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(80));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();

    EXPECT_NE(listPattern_, nullptr);
    EXPECT_NE(scrollablePattern_, nullptr);

    double initialOffset = scrollablePattern_->GetTotalOffset();
    scrollablePattern_->UpdateCurrentOffset(-200.0f, SCROLL_FROM_UPDATE);
    FlushUITasks(listFrameNode_);
    double offsetAfterScroll = scrollablePattern_->GetTotalOffset();

    EXPECT_NE(initialOffset, offsetAfterScroll);
    EXPECT_GT(offsetAfterScroll, initialOffset);
}

/**
 * @tc.name: LazyGridInList003
 * @tc.desc: Verify LazyVGrid with different columns template in List
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInList003, TestSize.Level1)
{
    CreateList();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr 1fr");

    for (int i = 0; i < 15; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(66));
        ViewAbstract::SetHeight(CalcLength(80));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(listFrameNode_, nullptr);
    EXPECT_GE(listFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridInList004
 * @tc.desc: Verify LazyVGrid with gaps in List
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInList004, TestSize.Level1)
{
    CreateList();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");
    gridModel.SetRowGap(Dimension(10.0));
    gridModel.SetColumnGap(Dimension(15.0));

    for (int i = 0; i < 15; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(100));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(listFrameNode_, nullptr);
}

/**
 * @tc.name: LazyGridInList005
 * @tc.desc: Verify LazyVGrid with repeat(auto-fit) in List
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInList005, TestSize.Level1)
{
    CreateList();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("repeat(auto-fit, 80)");

    for (int i = 0; i < 15; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(100));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(listFrameNode_, nullptr);
}

/**
 * @tc.name: LazyGridInList006
 * @tc.desc: Verify empty LazyVGrid works in List
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInList006, TestSize.Level1)
{
    CreateList();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(listFrameNode_, nullptr);
    EXPECT_GE(listFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridInList007
 * @tc.desc: Verify scroll back to top works in List with LazyVGrid
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInList007, TestSize.Level1)
{
    CreateList();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 20; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(100));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollablePattern_, nullptr);

    scrollablePattern_->UpdateCurrentOffset(-300.0f, SCROLL_FROM_UPDATE);
    FlushUITasks(listFrameNode_);
    double offsetDown = scrollablePattern_->GetTotalOffset();

    scrollablePattern_->UpdateCurrentOffset(300.0f, SCROLL_FROM_UPDATE);
    FlushUITasks(listFrameNode_);
    double offsetUp = scrollablePattern_->GetTotalOffset();

    EXPECT_GT(offsetDown, 0.0);
    EXPECT_EQ(offsetUp, 0.0);
}

/**
 * @tc.name: LazyGridInList008
 * @tc.desc: Verify multiple LazyVGrid children in List
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInList008, TestSize.Level1)
{
    CreateList();

    LazyVGridLayoutModel gridModel1;
    gridModel1.Create();
    gridModel1.SetColumnsTemplate("1fr 1fr");
    for (int i = 0; i < 8; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(80));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    LazyVGridLayoutModel gridModel2;
    gridModel2.Create();
    gridModel2.SetColumnsTemplate("1fr 1fr");
    for (int i = 0; i < 10; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(80));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(listFrameNode_, nullptr);
    auto children = listFrameNode_->GetChildren();
    EXPECT_GE(children.size(), 2u);
}

// ==================== LazyVGrid in Scroll Tests ====================

/**
 * @tc.name: LazyGridInScroll001
 * @tc.desc: Verify LazyVGrid works in vertical Scroll without abort
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInScroll001, TestSize.Level1)
{
    CreateScroll();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 10; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(100));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridInScroll002
 * @tc.desc: Verify Scroll with LazyVGrid can scroll and offset updates
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInScroll002, TestSize.Level1)
{
    CreateScroll();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 30; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(100));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollablePattern_, nullptr);

    double initialOffset = scrollablePattern_->GetTotalOffset();
    scrollablePattern_->UpdateCurrentOffset(-200.0f, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    double offsetAfterScroll = scrollablePattern_->GetTotalOffset();

    EXPECT_NE(initialOffset, offsetAfterScroll);
    EXPECT_GT(offsetAfterScroll, initialOffset);
}

/**
 * @tc.name: LazyGridInScroll003
 * @tc.desc: Verify Scroll with LazyVGrid can scroll back to top
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInScroll003, TestSize.Level1)
{
    CreateScroll();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 20; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(100));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollablePattern_, nullptr);

    scrollablePattern_->UpdateCurrentOffset(-250.0f, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    double offsetDown = scrollablePattern_->GetTotalOffset();

    scrollablePattern_->UpdateCurrentOffset(250.0f, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    double offsetUp = scrollablePattern_->GetTotalOffset();

    EXPECT_GT(offsetDown, 0.0);
    EXPECT_EQ(offsetUp, 0.0);
}

/**
 * @tc.name: LazyGridInScroll005
 * @tc.desc: Verify LazyVGrid with 3 columns in Scroll
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInScroll005, TestSize.Level1)
{
    CreateScroll();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr 1fr");

    for (int i = 0; i < 15; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(66));
        ViewAbstract::SetHeight(CalcLength(80));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridInScroll006
 * @tc.desc: Verify LazyVGrid with columnGap in Scroll
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInScroll006, TestSize.Level1)
{
    CreateScroll();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");
    gridModel.SetColumnGap(Dimension(10.0));

    for (int i = 0; i < 12; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(95));
        ViewAbstract::SetHeight(CalcLength(80));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridInScroll007
 * @tc.desc: Verify LazyVGrid with rowGap and columnGap in Scroll
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInScroll007, TestSize.Level1)
{
    CreateScroll();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr 1fr");
    gridModel.SetRowGap(Dimension(8.0));
    gridModel.SetColumnGap(Dimension(12.0));

    for (int i = 0; i < 18; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(60));
        ViewAbstract::SetHeight(CalcLength(70));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
}

/**
 * @tc.name: LazyGridInScroll008
 * @tc.desc: Verify LazyVGrid with large dataset in Scroll
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInScroll008, TestSize.Level1)
{
    CreateScroll();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 50; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(80));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    auto scrollNode = GetMainFrameNode();
    EXPECT_NE(scrollNode, nullptr);
    CreateDone();
    auto scrollPattern = scrollNode->GetPattern<ScrollablePattern>();
    EXPECT_NE(scrollPattern, nullptr);

    scrollPattern->UpdateCurrentOffset(-400.0f, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollNode);
    double offsetAfterScroll = scrollPattern->GetTotalOffset();

    EXPECT_GT(offsetAfterScroll, 0.0);
}

/**
 * @tc.name: LazyGridInScroll009
 * @tc.desc: Verify LazyVGrid with variable item sizes in Scroll
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInScroll009, TestSize.Level1)
{
    CreateScroll();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 20; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        float height = (i % 3 == 0) ? 120.0f : 80.0f;
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(height));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridInScroll010
 * @tc.desc: Verify Scroll axis verification for LazyVGrid container
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInScroll010, TestSize.Level1)
{
    CreateScroll();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 10; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(90));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollablePattern_, nullptr);
    EXPECT_EQ(scrollablePattern_->GetAxis(), Axis::VERTICAL);
}

/**
 * @tc.name: LazyGridColumnsTemplate001
 * @tc.desc: Verify LazyVGrid with single column template
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridColumnsTemplate001, TestSize.Level1)
{
    CreateWaterFlow();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr");

    for (int i = 0; i < 15; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(200));
        ViewAbstract::SetHeight(CalcLength(60));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridColumnsTemplate003
 * @tc.desc: Verify LazyVGrid with mixed fixed and fr columns
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridColumnsTemplate003, TestSize.Level1)
{
    CreateWaterFlow();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("50px 1fr 50px");

    for (int i = 0; i < 12; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(70));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridGaps001
 * @tc.desc: Verify LazyVGrid with only rowGap
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridGaps001, TestSize.Level1)
{
    CreateWaterFlow();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");
    gridModel.SetRowGap(Dimension(16.0));

    for (int i = 0; i < 15; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(80));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    auto frameNode = GetMainFrameNode();
    CreateDone();
    EXPECT_NE(frameNode, nullptr);
    EXPECT_GE(frameNode->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridGaps003
 * @tc.desc: Verify LazyVGrid with VP dimension gaps
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridGaps003, TestSize.Level1)
{
    CreateWaterFlow();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");
    gridModel.SetRowGap(Dimension(12.0, DimensionUnit::VP));
    gridModel.SetColumnGap(Dimension(8.0, DimensionUnit::VP));

    for (int i = 0; i < 12; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(90));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridItemSize002
 * @tc.desc: Verify LazyVGrid with tall items
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridItemSize002, TestSize.Level1)
{
    CreateWaterFlow();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 12; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(150));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridItemSize004
 * @tc.desc: Verify LazyVGrid with small items
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridItemSize004, TestSize.Level1)
{
    CreateWaterFlow();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr 1fr 1fr");

    for (int i = 0; i < 24; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(50));
        ViewAbstract::SetHeight(CalcLength(50));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridInList009
 * @tc.desc: Verify List axis verification for LazyVGrid container
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInList009, TestSize.Level1)
{
    CreateList();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 10; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(90));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(listPattern_, nullptr);
    EXPECT_EQ(scrollablePattern_->GetAxis(), Axis::VERTICAL);
}

/**
 * @tc.name: LazyGridInList010
 * @tc.desc: Verify LazyVGrid with different columnGap in List
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInList010, TestSize.Level1)
{
    CreateList();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr 1fr");
    gridModel.SetColumnGap(Dimension(16.0));

    for (int i = 0; i < 15; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(60));
        ViewAbstract::SetHeight(CalcLength(75));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(listFrameNode_, nullptr);
    EXPECT_GE(listFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridInList011
 * @tc.desc: Verify LazyVGrid with only rowGap in List
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInList011, TestSize.Level1)
{
    CreateList();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");
    gridModel.SetRowGap(Dimension(20.0));

    for (int i = 0; i < 12; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(85));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(listFrameNode_, nullptr);
}

/**
 * @tc.name: LazyGridInList012
 * @tc.desc: Verify LazyVGrid with large items in List
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInList012, TestSize.Level1)
{
    CreateList();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr");

    for (int i = 0; i < 8; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(200));
        ViewAbstract::SetHeight(CalcLength(120));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollablePattern_, nullptr);

    scrollablePattern_->UpdateCurrentOffset(-200.0f, SCROLL_FROM_UPDATE);
    FlushUITasks(listFrameNode_);
    double offset = scrollablePattern_->GetTotalOffset();

    EXPECT_GT(offset, 0.0);
}

/**
 * @tc.name: LazyGridInList013
 * @tc.desc: Verify LazyVGrid with 4 columns in List
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInList013, TestSize.Level1)
{
    CreateList();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr 1fr 1fr");

    for (int i = 0; i < 20; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(50));
        ViewAbstract::SetHeight(CalcLength(60));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(listFrameNode_, nullptr);
    EXPECT_GE(listFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridInList014
 * @tc.desc: Verify LazyVGrid with mixed size items in List
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInList014, TestSize.Level1)
{
    CreateList();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 16; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        float height = (i % 4 == 0) ? 140.0f : 70.0f;
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(height));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(listFrameNode_, nullptr);
}

/**
 * @tc.name: LazyGridInList015
 * @tc.desc: Verify List with LazyVGrid maintains children hierarchy
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridInList015, TestSize.Level1)
{
    CreateList();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 10; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(80));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(listFrameNode_, nullptr);

    auto children = listFrameNode_->GetChildren();
    EXPECT_GT(children.size(), 0u);

    for (auto& child : children) {
        EXPECT_NE(child, nullptr);
    }
}

/**
 * @tc.name: LazyGridEdgeCase001
 * @tc.desc: Verify LazyVGrid with single item
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridEdgeCase001, TestSize.Level1)
{
    CreateWaterFlow();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    StackModelNG stackModel;
    stackModel.Create();
    ViewAbstract::SetWidth(CalcLength(100));
    ViewAbstract::SetHeight(CalcLength(100));
    ViewStackProcessor::GetInstance()->Pop();
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridEdgeCase002
 * @tc.desc: Verify LazyVGrid with two items (exactly one row)
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridEdgeCase002, TestSize.Level1)
{
    CreateWaterFlow();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 2; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(100));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
}

/**
 * @tc.name: LazyGridEdgeCase003
 * @tc.desc: Verify LazyVGrid with three items (one full row + partial)
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridEdgeCase003, TestSize.Level1)
{
    CreateWaterFlow();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 3; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(90));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridEdgeCase004
 * @tc.desc: Verify LazyVGrid with exactly one full row of 3 columns
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridEdgeCase004, TestSize.Level1)
{
    CreateWaterFlow();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr 1fr");

    for (int i = 0; i < 3; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(66));
        ViewAbstract::SetHeight(CalcLength(66));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
}

/**
 * @tc.name: LazyGridEdgeCase005
 * @tc.desc: Verify LazyVGrid with very small item height
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridEdgeCase005, TestSize.Level1)
{
    CreateWaterFlow();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 20; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(20));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LazyGridEdgeCase006
 * @tc.desc: Verify LazyVGrid with very large item height
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridEdgeCase006, TestSize.Level1)
{
    CreateWaterFlow();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr");

    for (int i = 0; i < 6; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(100));
        ViewAbstract::SetHeight(CalcLength(200));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
}

/**
 * @tc.name: LazyGridEdgeCase007
 * @tc.desc: Verify LazyVGrid with 5 columns template
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LazyGridEdgeCase007, TestSize.Level1)
{
    CreateWaterFlow();

    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    gridModel.SetColumnsTemplate("1fr 1fr 1fr 1fr 1fr");

    for (int i = 0; i < 25; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(40));
        ViewAbstract::SetHeight(CalcLength(40));
        ViewStackProcessor::GetInstance()->Pop();
    }
    ViewStackProcessor::GetInstance()->Pop();

    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);
}

/**
 * @tc.name: LayoutPolicyTest001
 * @tc.desc: test the measure result when setting matchParent.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LayoutPolicyTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create LazyVGridLayout
     */
    RefPtr<FrameNode> lazyGrid;
    auto column = CreateColumn([this, &lazyGrid](ColumnModelNG model) {
        ViewAbstract::SetWidth(CalcLength(500));
        ViewAbstract::SetHeight(CalcLength(300));
        LazyVGridLayoutModel lazyGridModel;
        lazyGridModel.Create();
        ViewAbstractModelNG model1;
        model1.UpdateLayoutPolicyProperty(LayoutCalPolicy::MATCH_PARENT, true);
        model1.UpdateLayoutPolicyProperty(LayoutCalPolicy::MATCH_PARENT, false);
        RefPtr<UINode> element = ViewStackProcessor::GetInstance()->GetMainElementNode();
        ViewStackProcessor::GetInstance()->PopContainer();
        lazyGrid = AceType::DynamicCast<FrameNode>(element);
    });
    ViewStackProcessor::GetInstance()->Finish();
    ASSERT_NE(column, nullptr);
    ASSERT_EQ(column->GetChildren().size(), 1);
    CreateLayoutTask(column);

    // Expect LazyVGridLayout's width is 500, height is 300 land offset is [0.0, 0.0].
    auto geometryNode = lazyGrid->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    auto size = geometryNode->GetFrameSize();
    auto offset = geometryNode->GetFrameOffset();
    EXPECT_EQ(size, SizeF(500.0f, 300.0f));
    EXPECT_EQ(offset, OffsetF(0.0f, 0.0f));
}

/**
 * @tc.name: UpdatePosMapStartIncludesSpaceWidth001
 * @tc.desc: Rebase from a non-zero first cached line must include row gap in the estimated line position.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, UpdatePosMapStartIncludesSpaceWidth001, TestSize.Level1)
{
    LazyGridLayoutInfo layoutInfo;
    layoutInfo.SetLanes(2);
    layoutInfo.SetSpace(10.0f);
    layoutInfo.SetTotalItemCount(6);
    layoutInfo.SetPosMap(4, { 0, 200.0f, 300.0f });
    layoutInfo.SetPosMap(5, { 1, 200.0f, 300.0f });

    layoutInfo.UpdatePosMap(0.0f);

    EXPECT_FLOAT_EQ(layoutInfo.posMap_[4].startPos, 220.0f);
    EXPECT_FLOAT_EQ(layoutInfo.posMap_[4].endPos, 320.0f);
    EXPECT_FLOAT_EQ(layoutInfo.posMap_[5].startPos, 220.0f);
    EXPECT_FLOAT_EQ(layoutInfo.posMap_[5].endPos, 320.0f);
    EXPECT_FLOAT_EQ(layoutInfo.totalMainSize_, 320.0f);
}
/**
 * @tc.name: FirstFrameWindowSeedsWithUnknownBody001
 * @tc.desc: First layout with an already-scrolled viewport (body extent unknown): the forward window must fall back
 *           to measuring from item 0 (no past-the-end clamp), and the backward window seeds the last item at body 0.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, FirstFrameWindowSeedsWithUnknownBody001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    info->totalItemCount_ = 10;
    LazyGridLayoutAlgorithm algorithm(info);
    algorithm.totalItemCount_ = 10;
    algorithm.hadMeasuredItems_ = false; // true first layout
    algorithm.prevBodyMainSize_ = 0.0f;
    algorithm.startPos_ = 200.0f; // viewport already scrolled when this child first measures
    algorithm.endPos_ = 650.0f;

    int32_t index = -2;
    float pos = -1.0f;
    algorithm.GetStartIndexInfo(index, pos);
    EXPECT_EQ(index, 0);
    EXPECT_FLOAT_EQ(pos, 0.0f);

    index = -2;
    pos = -1.0f;
    algorithm.GetEndIndexInfo(index, pos);
    EXPECT_EQ(index, 9);
    EXPECT_FLOAT_EQ(pos, 0.0f);
}

/**
 * @tc.name: FirstFrameWindowSkipsPastKnownBody001
 * @tc.desc: A fresh lazy grid with a known parent-supplied extent must not walk every child when fully before view.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, FirstFrameWindowSkipsPastKnownBody001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    info->totalItemCount_ = 3000;
    LazyGridLayoutAlgorithm algorithm(info);
    algorithm.totalItemCount_ = 3000;
    algorithm.lanes_ = 3;
    algorithm.hadMeasuredItems_ = false;
    algorithm.knownMainSize_ = 250000.0f;
    algorithm.prevBodyMainSize_ = 250000.0f;
    algorithm.startPos_ = 250500.0f;
    algorithm.endPos_ = 251000.0f;

    int32_t index = -1;
    float pos = -1.0f;
    algorithm.GetStartIndexInfo(index, pos);
    EXPECT_EQ(index, 3000);
    EXPECT_FLOAT_EQ(pos, 250000.0f);

    index = -1;
    pos = -1.0f;
    algorithm.GetEndIndexInfo(index, pos);
    EXPECT_EQ(index, 3000);
    EXPECT_FLOAT_EQ(pos, 250000.0f);
    EXPECT_TRUE(info->posMap_.empty());
}

/**
 * @tc.name: FirstFrameKnownBodySeedsNearTail001
 * @tc.desc: A fresh grid measures a near-tail viewport backward from the known body end, not from row zero.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, FirstFrameKnownBodySeedsNearTail001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    LazyGridLayoutAlgorithm algorithm(info);
    algorithm.totalItemCount_ = 3000;
    algorithm.spaceWidth_ = 12.0f;
    algorithm.knownMainSize_ = 375000.0f;
    algorithm.prevBodyMainSize_ = 375000.0f;
    algorithm.startPos_ = 374121.0f;
    algorithm.endPos_ = 374722.0f;

    int32_t index = -1;
    float pos = -1.0f;
    algorithm.GetStartIndexInfo(index, pos);
    EXPECT_EQ(index, 3000);
    EXPECT_FLOAT_EQ(pos - algorithm.spaceWidth_, 375000.0f);

    info->posMap_[0] = { 0, 0.0f, 500.0f };
    algorithm.GetStartIndexInfo(index, pos);
    EXPECT_EQ(index, 3000);
    EXPECT_FLOAT_EQ(pos - algorithm.spaceWidth_, 375000.0f);
}

/**
 * @tc.name: FirstFrameKnownBodySeedsNearHead001
 * @tc.desc: A backward layout near the head uses the known origin instead of walking from the far tail.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, FirstFrameKnownBodySeedsNearHead001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    LazyGridLayoutAlgorithm algorithm(info);
    algorithm.totalItemCount_ = 3000;
    algorithm.spaceWidth_ = 12.0f;
    algorithm.knownMainSize_ = 375000.0f;
    algorithm.prevBodyMainSize_ = 375000.0f;
    algorithm.startPos_ = 279.0f;
    algorithm.endPos_ = 879.0f;

    int32_t index = 3000;
    float pos = 375000.0f;
    algorithm.GetEndIndexInfo(index, pos);
    EXPECT_EQ(index, -1);
    EXPECT_FLOAT_EQ(pos + algorithm.spaceWidth_, 0.0f);

    info->posMap_[2997] = { 0, 374500.0f, 375000.0f };
    algorithm.GetEndIndexInfo(index, pos);
    EXPECT_EQ(index, -1);
    EXPECT_FLOAT_EQ(pos + algorithm.spaceWidth_, 0.0f);
}

/**
 * @tc.name: EstimateLayoutWhenDetached001
 * @tc.desc: Test fresh, reused and short detached LazyVGrid hosts select the correct measure mode.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, EstimateLayoutWhenDetached001, TestSize.Level1)
{
    auto layoutInfo = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    auto algorithm = AceType::MakeRefPtr<LazyGridLayoutAlgorithm>(layoutInfo);
    algorithm->SetAxis(Axis::VERTICAL);
    algorithm->totalItemCount_ = 10;
    algorithm->lanes_ = 2;
    algorithm->hadMeasuredItems_ = false;
    auto context = CreateCachedLazyLayoutTestContext(V2::COLUMN_ETS_TAG,
        AceType::MakeRefPtr<LazyGridLayoutPattern>(), algorithm, SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    ASSERT_NE(context.host, nullptr);
    ASSERT_NE(context.wrapper, nullptr);
    EXPECT_FALSE(context.host->IsOnMainTree());
    EXPECT_TRUE(context.host->IsNeedLazyLayout());
    ASSERT_NE(context.host->GetParentFrameNode(), nullptr);
    EXPECT_EQ(context.host->GetParentFrameNode()->GetTag(), V2::WATERFLOW_ETS_TAG);

    std::optional<ViewPosReference> posRef;
    algorithm->UpdateReferencePos(AceType::RawPtr(context.wrapper), posRef);
    EXPECT_TRUE(algorithm->isEstimatePass_);
    EXPECT_FALSE(algorithm->needSkipLayout_);
    EXPECT_TRUE(layoutInfo->posMap_.empty());

    layoutInfo->posMap_[0] = { 0, 0.0f, 40.0f };
    layoutInfo->posMap_[1] = { 1, 0.0f, 60.0f };
    layoutInfo->totalMainSize_ = 500.0f;
    algorithm->hadMeasuredItems_ = true;
    const auto previousFirstItem = layoutInfo->posMap_.at(0);
    context.host->AttachToMainTree();
    context.host->DetachFromMainTree();
    algorithm->UpdateReferencePos(AceType::RawPtr(context.wrapper), posRef);
    EXPECT_FALSE(algorithm->isEstimatePass_);
    EXPECT_TRUE(algorithm->needSkipLayout_);
    EXPECT_FALSE(algorithm->needAllLayout_);
    EXPECT_EQ(layoutInfo->posMap_.at(0).laneIdx, previousFirstItem.laneIdx);
    EXPECT_FLOAT_EQ(layoutInfo->posMap_.at(0).startPos, previousFirstItem.startPos);
    EXPECT_FLOAT_EQ(layoutInfo->posMap_.at(0).endPos, previousFirstItem.endPos);
    EXPECT_FLOAT_EQ(layoutInfo->totalMainSize_, 500.0f);

    algorithm->totalItemCount_ = 2;
    algorithm->UpdateReferencePos(AceType::RawPtr(context.wrapper), posRef);
    EXPECT_FALSE(algorithm->isEstimatePass_);
    EXPECT_FALSE(algorithm->needSkipLayout_);
}

/**
 * @tc.name: EstimateLayoutWithParentReference001
 * @tc.desc: Test LazyVGrid estimates only fresh hosts inside the parent predictive range.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, EstimateLayoutWithParentReference001, TestSize.Level1)
{
    auto layoutInfo = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    auto algorithm = AceType::MakeRefPtr<LazyGridLayoutAlgorithm>(layoutInfo);
    algorithm->SetAxis(Axis::VERTICAL);
    algorithm->totalItemCount_ = 10;
    algorithm->lanes_ = 2;
    algorithm->hadMeasuredItems_ = false;
    auto context = CreateCachedLazyLayoutTestContext(V2::COLUMN_ETS_TAG,
        AceType::MakeRefPtr<LazyGridLayoutPattern>(), algorithm, SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    ASSERT_NE(context.wrapper, nullptr);
    std::optional<ViewPosReference> posRef = ViewPosReference {
        .viewPosStart = 0.0f,
        .viewPosEnd = SCROLL_HEIGHT,
        .referencePos = SCROLL_HEIGHT,
        .referenceEdge = ReferenceEdge::START,
        .axis = Axis::VERTICAL,
        .deadline = 1,
    };

    algorithm->UpdateReferencePos(AceType::RawPtr(context.wrapper), posRef);
    EXPECT_TRUE(algorithm->isEstimatePass_);
    algorithm->hadMeasuredItems_ = true;
    algorithm->UpdateReferencePos(AceType::RawPtr(context.wrapper), posRef);
    EXPECT_FALSE(algorithm->isEstimatePass_);
    EXPECT_TRUE(layoutInfo->deadline_.has_value());

    algorithm->hadMeasuredItems_ = false;
    posRef->referencePos = SCROLL_HEIGHT * 2.0f;
    algorithm->UpdateReferencePos(AceType::RawPtr(context.wrapper), posRef);
    EXPECT_FALSE(algorithm->isEstimatePass_);
}

/**
 * @tc.name: EstimateLinesTotalHeight001
 * @tc.desc: Test LazyVGrid samples two complete lines and extrapolates the complete detached host height.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, EstimateLinesTotalHeight001, TestSize.Level1)
{
    auto layoutInfo = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    LazyGridLayoutAlgorithm algorithm(layoutInfo);
    algorithm.totalItemCount_ = 10;
    algorithm.lanes_ = 2;
    algorithm.spaceWidth_ = 4.0f;
    algorithm.childLayoutConstraints_.resize(2);

    auto host = FrameNode::CreateFrameNode(
        V2::LAZY_V_GRID_LAYOUT_ETS_TAG, -2100, AceType::MakeRefPtr<LazyGridLayoutPattern>());
    auto wrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        host, AceType::MakeRefPtr<GeometryNode>(), host->GetLayoutProperty());
    int32_t measureCount = 0;
    AppendFixedHeightChild(wrapper, -2101, 40.0f, measureCount);
    AppendFixedHeightChild(wrapper, -2102, 60.0f, measureCount);
    AppendFixedHeightChild(wrapper, -2103, 80.0f, measureCount);
    AppendFixedHeightChild(wrapper, -2104, 70.0f, measureCount);
    AppendFixedHeightChild(wrapper, -2105, 200.0f, measureCount);

    algorithm.MeasureEstimateItems(AceType::RawPtr(wrapper));

    EXPECT_EQ(measureCount, 4);
    EXPECT_EQ(layoutInfo->posMap_.size(), 4);
    EXPECT_FLOAT_EQ(layoutInfo->totalMainSize_, 366.0f);
    EXPECT_FLOAT_EQ(layoutInfo->estimateItemSize_, -1.0f);
}

/**
 * @tc.name: SparseMapEstimateUsesMeasuredRows001
 * @tc.desc: Unknown rows between measured head and tail rows do not inflate the average row height.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, SparseMapEstimateUsesMeasuredRows001, TestSize.Level1)
{
    LazyGridLayoutInfo info;
    info.lanes_ = 3;
    info.spaceWidth_ = 10.0f;
    for (int32_t lane = 0; lane < 3; ++lane) {
        info.posMap_[lane] = { lane, 0.0f, 100.0f };
        info.posMap_[2997 + lane] = { lane, 300000.0f, 300500.0f };
    }
    info.EstimateItemSize();
    // Ordinary layout retains its original span-based estimator.
    EXPECT_FLOAT_EQ(info.estimateItemSize_, 150245.0f);
    info.EstimateBoundaryItemSize();
    EXPECT_FLOAT_EQ(info.estimateItemSize_, 300.0f);
}

/**
 * @tc.name: GetStartIndexFallbackUsesExplicitPreparation001
 * @tc.desc: Only a committed boundary measurement or supplied extent may replace the legacy missing-anchor fallback.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, GetStartIndexFallbackUsesExplicitPreparation001, TestSize.Level1)
{
    for (int32_t source = 0; source < 3; ++source) {
        for (int32_t endIndex : { -1, 9 }) {
            SCOPED_TRACE(testing::Message() << "source=" << source << ", endIndex=" << endIndex);
            auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
            info->lanes_ = 2;
            info->totalItemCount_ = 10;
            info->totalMainSize_ = 1000.0f;
            info->endIndex_ = endIndex;
            info->posMap_[8] = { 0, 800.0f, 900.0f };
            info->posMap_[9] = { 1, 800.0f, 900.0f };
            if (source == 1) {
                info->premeasuredMainSize_ = 1000.0f;
            }
            LazyGridLayoutAlgorithm algorithm(info);
            algorithm.lanes_ = 2;
            algorithm.totalItemCount_ = 10;
            algorithm.CaptureFrameBaseline();
            if (source == 2) {
                algorithm.knownMainSize_ = 1000.0f;
            }
            algorithm.startPos_ = 850.0f;
            algorithm.endPos_ = 950.0f;
            int32_t index = -1;
            float position = -1.0f;
            algorithm.GetStartIndexInfo(index, position);
            EXPECT_EQ(index, source == 0 ? 0 : 8);
            EXPECT_FLOAT_EQ(position, source == 0 ? 0.0f : 800.0f);
        }
    }
}

/**
 * @tc.name: SetSpaceWithNonzeroFirstLine001
 * @tc.desc: Changing the gap preserves the estimated prefix before the first cached line.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, SetSpaceWithNonzeroFirstLine001, TestSize.Level1)
{
    for (int32_t lanes : { 1, 2, 3 }) {
        SCOPED_TRACE(lanes);
        LazyGridLayoutInfo layoutInfo;
        layoutInfo.SetLanes(lanes);
        layoutInfo.SetTotalItemCount(6 * lanes);
        layoutInfo.SetSpace(10.0f);
        // Cache only the third and fourth lines, leaving an estimated prefix and suffix.
        for (int32_t index = 2 * lanes; index < 4 * lanes; ++index) {
            float startPos = (index / lanes) * 110.0f;
            layoutInfo.SetPosMap(index, { index % lanes, startPos, startPos + 100.0f });
        }
        layoutInfo.UpdatePosMap(0.0f);

        for (float space : { 20.0f, 0.0f }) {
            SCOPED_TRACE(space);
            layoutInfo.SetSpace(space);
            for (int32_t index = 2 * lanes; index < 4 * lanes; ++index) {
                const auto& pos = layoutInfo.posMap_.at(index);
                EXPECT_FLOAT_EQ(pos.startPos, (index / lanes) * (100.0f + space));
                EXPECT_FLOAT_EQ(pos.endPos - pos.startPos, 100.0f);
            }
        }
    }
}

/**
 * @tc.name: RowsGapAfterLaneChangeAndScroll001
 * @tc.desc: A gap change keeps offscreen cached lines and the total height correct after changing lanes.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, RowsGapAfterLaneChangeAndScroll001, TestSize.Level1)
{
    CreateScroll();
    CreateLazyGridLayout();
    layoutProperty_->UpdateColumnsTemplate("1fr");
    CreateContent(40);
    CreateDone();
    scrollablePattern_->UpdateCurrentOffset(-610, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    layoutProperty_->UpdateColumnsTemplate("1fr 1fr");
    FlushUITasks(scrollableFrameNode_);

    // Leave the first cached line outside the current measurement window.
    scrollablePattern_->UpdateCurrentOffset(-800, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    auto layoutInfo = pattern_->layoutInfo_;
    ASSERT_FALSE(layoutInfo->posMap_.empty());
    const int32_t firstIndex = layoutInfo->posMap_.begin()->first;
    ASSERT_GT(firstIndex, 0);
    ASSERT_GT(layoutInfo->startIndex_, firstIndex + 1);
    ASSERT_FLOAT_EQ(layoutInfo->totalMainSize_, 2000.0f);

    layoutProperty_->UpdateRowGap(Dimension(20));
    FlushUITasks(scrollableFrameNode_);
    EXPECT_FLOAT_EQ(layoutInfo->posMap_.at(firstIndex).startPos, (firstIndex / 2) * 120.0f);
    EXPECT_FLOAT_EQ(layoutInfo->totalMainSize_, 2380.0f); // 20 lines and 19 gaps.
}

/**
 * @tc.name: UpdatePosMapWithCachedLastLine001
 * @tc.desc: Rebasing a cached full or partial last line uses its end position for the total height.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, UpdatePosMapWithCachedLastLine001, TestSize.Level1)
{
    for (int32_t count : { 5, 6 }) {
        SCOPED_TRACE(count);
        LazyGridLayoutInfo layoutInfo;
        layoutInfo.SetLanes(2);
        layoutInfo.SetTotalItemCount(count);
        layoutInfo.SetSpace(10.0f);
        layoutInfo.SetPosMap(0, { 0, 0.0f, 100.0f });
        layoutInfo.SetPosMap(1, { 1, 0.0f, 100.0f });
        layoutInfo.SetPosMap(2, { 0, 110.0f, 260.0f });
        layoutInfo.SetPosMap(3, { 1, 110.0f, 260.0f });
        for (int32_t index = 4; index < count; ++index) {
            layoutInfo.SetPosMap(index, { index % 2, 270.0f, 370.0f });
        }
        layoutInfo.UpdatePosMap(0.0f);
        ASSERT_FLOAT_EQ(layoutInfo.totalMainSize_, 370.0f);

        // Resize the middle line in both directions while the last line stays cached.
        for (float height : { 100.0f, 180.0f }) {
            SCOPED_TRACE(height);
            float previousTotal = layoutInfo.totalMainSize_;
            layoutInfo.SetPosMap(2, { 0, 110.0f, 110.0f + height });
            layoutInfo.SetPosMap(3, { 1, 110.0f, 110.0f + height });
            layoutInfo.UpdatePosMap(previousTotal);
            EXPECT_FLOAT_EQ(layoutInfo.posMap_.rbegin()->second.endPos, height + 220.0f);
            EXPECT_FLOAT_EQ(layoutInfo.totalMainSize_, height + 220.0f);
        }
    }
}

/**
 * @tc.name: ResizeBeforeCachedPartialLastLine001
 * @tc.desc: Resizing a visible line keeps the section height consistent with a cached partial last line.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, ResizeBeforeCachedPartialLastLine001, TestSize.Level1)
{
    CreateScroll();
    CreateLazyGridLayout();
    layoutProperty_->UpdateRowGap(Dimension(10));
    CreateContent(21);
    CreateDone();
    scrollablePattern_->UpdateCurrentOffset(-10000, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    scrollablePattern_->UpdateCurrentOffset(10000, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    auto layoutInfo = pattern_->layoutInfo_;
    ASSERT_FALSE(layoutInfo->posMap_.empty());
    ASSERT_EQ(layoutInfo->posMap_.rbegin()->first, 20);
    ASSERT_FLOAT_EQ(layoutInfo->totalMainSize_, 1200.0f);
    for (int32_t index : { 2, 3 }) {
        auto child = GetChildFrameNode(frameNode_, index);
        ASSERT_NE(child, nullptr);
        child->GetLayoutProperty()->UpdateUserDefinedIdealSize(CalcSize(CalcLength(100), CalcLength(150)));
        child->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    }
    FlushUITasks(scrollableFrameNode_);
    EXPECT_FLOAT_EQ(layoutInfo->posMap_.at(20).endPos, 1250.0f);
    EXPECT_FLOAT_EQ(layoutInfo->totalMainSize_, 1250.0f);
}
/**
 * @tc.name: PremeasureSuffixKeepsWindow001
 * @tc.desc: A fresh cached grid measures complete tail rows without publishing a new visible window.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PremeasureSuffixKeepsWindow001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    auto algorithm = AceType::MakeRefPtr<LazyGridLayoutAlgorithm>(info);
    auto context = CreateCachedLazyLayoutTestContext(V2::LAZY_V_GRID_LAYOUT_ETS_TAG,
        AceType::MakeRefPtr<LazyGridLayoutPattern>(), algorithm, SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    context.host->GetLayoutProperty<LazyGridLayoutProperty>()->UpdateColumnsTemplate("1fr 1fr 1fr");
    int32_t measured = 0;
    for (int32_t index = 0; index < 30; ++index) {
        AppendFixedHeightChild(context.wrapper, -4000 - index,
            index < 18 ? 50.0f : (index % 3 == 2 ? 225.0f : 100.0f), measured);
        auto child = context.wrapper->GetOrCreateChildByIndex(index, false, true);
        child->SetActive(index < 3);
        child->GetGeometryNode()->SetFrameOffset(OffsetF(12.0f, 34.0f));
    }
    info->adjustOffset_ = { 7.0f, 9.0f };
    info->cacheStartPos_ = -100.0f;
    info->cacheEndPos_ = 600.0f;
    algorithm->startPos_ = 73.0f;
    auto constraint = context.parentConstraint;
    constraint.maxSize.SetHeight(Infinity<float>());
    constraint.viewPosRef = ViewPosReference {
        .viewPosStart = -900.0f, .viewPosEnd = 0.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::END, .axis = Axis::VERTICAL,
    };
    auto result = LazyLayoutUtils::Premeasure(context.wrapper, constraint);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->complete);
    EXPECT_FALSE(result->reachedOppositeEdge);
    EXPECT_EQ(measured, 12);
    ASSERT_EQ(info->posMap_.size(), 12);
    EXPECT_EQ(info->posMap_.begin()->first, 18);
    EXPECT_FLOAT_EQ(info->totalMainSize_ - info->posMap_.at(18).startPos, 900.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(18).endPos - info->posMap_.at(18).startPos, 225.0f);
    EXPECT_FLOAT_EQ(result->marginMainSize, info->totalMainSize_);
    EXPECT_EQ(info->startIndex_, -1);
    EXPECT_EQ(info->endIndex_, -1);
    EXPECT_EQ(info->visibleStartIndex_, -1);
    EXPECT_EQ(info->layoutedStartIndex_, -1);
    EXPECT_FLOAT_EQ(info->cacheStartPos_, -100.0f);
    EXPECT_FLOAT_EQ(info->cacheEndPos_, 600.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.start, 7.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.end, 9.0f);
    EXPECT_FLOAT_EQ(algorithm->startPos_, 73.0f);
    for (int32_t index = 0; index < 30; ++index) {
        auto child = context.wrapper->GetOrCreateChildByIndex(index, false, true);
        EXPECT_EQ(child->IsActive(), index < 3);
        EXPECT_EQ(child->GetGeometryNode()->GetFrameOffset(), OffsetF(12.0f, 34.0f));
    }
}

/**
 * @tc.name: PremeasureColdSuffixNormalForwardSkipsMiddle001
 * @tc.desc: A cold grid uses its prepared tail when normal forward layout enters that tail.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PremeasureColdSuffixNormalForwardSkipsMiddle001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    auto pattern = AceType::MakeRefPtr<LazyGridLayoutPattern>();
    pattern->layoutInfo_ = info;
    auto context = CreateCachedLazyLayoutTestContext(V2::LAZY_V_GRID_LAYOUT_ETS_TAG, pattern,
        AceType::MakeRefPtr<LazyGridLayoutAlgorithm>(info), SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    context.host->GetLayoutProperty<LazyGridLayoutProperty>()->UpdateColumnsTemplate("1fr 1fr 1fr");
    std::vector<int32_t> measured(30, 0);
    for (int32_t index = 0; index < 30; ++index) {
        AppendFixedHeightChild(context.wrapper, -5300 - index, index < 18 ? 50.0f : 225.0f, measured[index]);
    }
    auto constraint = context.parentConstraint;
    constraint.maxSize.SetHeight(Infinity<float>());
    constraint.viewPosRef = ViewPosReference {
        .viewPosStart = -900.0f, .viewPosEnd = 0.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::END, .axis = Axis::VERTICAL,
    };
    auto result = LazyLayoutUtils::Premeasure(context.wrapper, constraint);
    ASSERT_TRUE(result && result->complete);
    ASSERT_EQ(info->startIndex_, -1);
    ASSERT_EQ(info->posMap_.begin()->first, 18);
    const float total = result->marginMainSize;
    auto normal = AceType::MakeRefPtr<LazyGridLayoutAlgorithm>(info);
    context.wrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(normal));
    constraint.viewPosRef = ViewPosReference {
        .viewPosStart = total - 500.0f, .viewPosEnd = total - 100.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::START, .axis = Axis::VERTICAL,
    };
    context.wrapper->SetActive(true);
    context.wrapper->Measure(constraint);
    EXPECT_GE(info->visibleStartIndex_, 18);
    for (int32_t index = 3; index < 18; ++index) {
        EXPECT_EQ(measured[index], 0) << "unneeded middle item=" << index;
    }
}

/**
 * @tc.name: PremeasurePrefixAndInvalidReference001
 * @tc.desc: Prefix measurement is bounded, and missing/deadline references perform no measurement.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PremeasurePrefixAndInvalidReference001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    auto algorithm = AceType::MakeRefPtr<LazyGridLayoutAlgorithm>(info);
    auto context = CreateCachedLazyLayoutTestContext(V2::LAZY_V_GRID_LAYOUT_ETS_TAG,
        AceType::MakeRefPtr<LazyGridLayoutPattern>(), algorithm, SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    context.host->GetLayoutProperty<LazyGridLayoutProperty>()->UpdateColumnsTemplate("1fr 1fr");
    int32_t measured = 0;
    for (int32_t index = 0; index < 10; ++index) {
        AppendFixedHeightChild(context.wrapper, -4100 - index, 100.0f, measured);
    }
    auto constraint = context.parentConstraint;
    constraint.maxSize.SetHeight(Infinity<float>());
    EXPECT_FALSE(LazyLayoutUtils::Premeasure(context.wrapper, constraint));
    constraint.viewPosRef = ViewPosReference {
        .viewPosStart = 0.0f, .viewPosEnd = 200.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::START, .axis = Axis::VERTICAL, .deadline = 1,
    };
    EXPECT_FALSE(LazyLayoutUtils::Premeasure(context.wrapper, constraint));
    EXPECT_EQ(measured, 0);
    constraint.viewPosRef->deadline.reset();
    auto result = LazyLayoutUtils::Premeasure(context.wrapper, constraint);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->complete);
    EXPECT_FALSE(result->reachedOppositeEdge);
    EXPECT_EQ(measured, 4);
    ASSERT_EQ(info->posMap_.size(), 4);
    EXPECT_FLOAT_EQ(info->posMap_.at(0).startPos, 0.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(3).endPos, 200.0f);

    context.wrapper->GetOrCreateChildByIndex(0, false, true)->GetLayoutProperty()->UpdateAspectRatio(2.0f);
    context.wrapper->GetOrCreateChildByIndex(1, false, true)->GetLayoutProperty()->UpdateAspectRatio(2.0f);
    constraint.viewPosRef->viewPosEnd = 50.0f;
    result = LazyLayoutUtils::Premeasure(context.wrapper, constraint);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->complete);
    EXPECT_FLOAT_EQ(info->posMap_.at(0).endPos, 50.0f);
}
/**
 * @tc.name: PremeasureContainerMeasureLifecycle001
 * @tc.desc: The real container Measure callback runs without publishing preparation geometry.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PremeasureContainerMeasureLifecycle001, TestSize.Level1)
{
    CreateScroll();
    CreateLazyGridLayout();
    layoutProperty_->UpdateColumnsTemplate("1fr");
    CreateContent(30);
    CreateDone();
    const auto published = frameNode_->GetGeometryNode();
    const auto frameRect = published->GetFrameRect();
    auto callbackCount = std::make_shared<int32_t>(0);
    auto callbackHeight = std::make_shared<float>(0.0f);
    frameNode_->SetMeasureCallback([callbackCount, callbackHeight, weak = WeakPtr<FrameNode>(frameNode_)](
        RefPtr<Kit::FrameNode>) {
        ++*callbackCount;
        auto node = weak.Upgrade();
        if (node) {
            *callbackHeight = node->GetGeometryNode()->GetMarginFrameSize().Height();
        }
    });
    auto constraint = published->GetParentLayoutConstraint().value();
    constraint.viewPosRef = ViewPosReference {
        .viewPosStart = -250.0f, .viewPosEnd = 0.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::END, .axis = Axis::VERTICAL,
    };
    std::vector<WeakPtr<FrameNode>> measuredNodes;
    const auto result = LazyLayoutUtils::Premeasure(frameNode_, constraint, &measuredNodes);
    ASSERT_TRUE(result && result->complete);
    EXPECT_EQ(*callbackCount, 1);
    EXPECT_FLOAT_EQ(*callbackHeight, result->marginMainSize);
    EXPECT_EQ(frameNode_->GetGeometryNode(), published);
    EXPECT_EQ(published->GetFrameRect(), frameRect);
    EXPECT_TRUE(std::any_of(measuredNodes.begin(), measuredNodes.end(), [node = frameNode_](const auto& weak) {
        return weak.Upgrade() == node;
    }));
}

/**
 * @tc.name: PremeasureFrameNodeKeepsActiveRange001
 * @tc.desc: Preparation must preserve the displayed and cached child state.
 * @tc.type: FUNC
 */
class LazyGridHistoryPremeasureTest : public LazyGridLayoutTest, public testing::WithParamInterface<int32_t> {};

/**
 * @tc.name: PremeasureRefreshesHistory001
 * @tc.desc: New lanes, spacing or a shortened partial row use current properties before preparation.
 * @tc.type: FUNC
 */
HWTEST_P(LazyGridHistoryPremeasureTest, PremeasureRefreshesHistory001, TestSize.Level1)
{
    CreateScroll();
    CreateLazyGridLayout();
    CreateContent(30);
    CreateDone();
    auto info = pattern_->layoutInfo_;
    ASSERT_FALSE(info->posMap_.empty());
    const auto published = frameNode_->GetGeometryNode();
    const auto frameRect = published->GetFrameRect();
    const auto visible = VisibleRange(info->visibleStartIndex_, info->visibleEndIndex_);
    if (GetParam() == 0) {
        layoutProperty_->UpdateColumnsTemplate("1fr 1fr 1fr");
    } else if (GetParam() == 1) {
        layoutProperty_->UpdateRowGap(Dimension(10.0f, DimensionUnit::PX));
    } else {
        for (int32_t index = 0; index < 5; ++index) {
            frameNode_->RemoveChild(frameNode_->GetLastChild());
        }
    }
    auto constraint = published->GetParentLayoutConstraint().value();
    constraint.viewPosRef = ViewPosReference {
        .viewPosStart = -250.0f, .viewPosEnd = 0.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::END, .axis = Axis::VERTICAL,
    };
    const auto result = LazyLayoutUtils::Premeasure(frameNode_, constraint);
    ASSERT_TRUE(result && result->complete);
    const int32_t lanes = GetParam() == 0 ? 3 : 2;
    const int32_t count = GetParam() == 2 ? 25 : 30;
    const float gap = GetParam() == 1 ? 10.0f : 0.0f;
    const int32_t rows = (count + lanes - 1) / lanes;
    EXPECT_EQ(info->lanes_, lanes);
    EXPECT_EQ(info->totalItemCount_, count);
    EXPECT_FLOAT_EQ(info->spaceWidth_, gap);
    EXPECT_FLOAT_EQ(result->marginMainSize, rows * ITEM_HEIGHT + (rows - 1) * gap);
    EXPECT_FLOAT_EQ(info->posMap_.at(count - 1).endPos, info->totalMainSize_);
    EXPECT_EQ(info->posMap_.rbegin()->first, count - 1);
    EXPECT_EQ(VisibleRange(info->visibleStartIndex_, info->visibleEndIndex_), visible);
    EXPECT_EQ(frameNode_->GetGeometryNode(), published);
    EXPECT_EQ(published->GetFrameRect(), frameRect);
}

INSTANTIATE_TEST_SUITE_P(CurrentProperties, LazyGridHistoryPremeasureTest, testing::Values(0, 1, 2));

HWTEST_F(LazyGridLayoutTest, PremeasureFrameNodeKeepsActiveRange001, TestSize.Level1)
{
    CreateScroll();
    CreateLazyGridLayout();
    layoutProperty_->UpdateColumnsTemplate("1fr");
    CreateContent(30);
    CreateDone();
    auto info = pattern_->layoutInfo_;
    const auto visible = VisibleRange(info->visibleStartIndex_, info->visibleEndIndex_);
    const auto range = VisibleRange(info->startIndex_, info->endIndex_);
    std::vector<std::pair<bool, OffsetF>> states;
    for (int32_t index = 0; index < 30; ++index) {
        auto child = GetChildFrameNode(frameNode_, index);
        states.emplace_back(child->IsActive(), child->GetGeometryNode()->GetFrameOffset());
    }
    auto constraint = frameNode_->GetGeometryNode()->GetParentLayoutConstraint().value();
    constraint.viewPosRef = ViewPosReference {
        .viewPosStart = -250.0f, .viewPosEnd = 0.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::END, .axis = Axis::VERTICAL,
    };
    auto result = LazyLayoutUtils::Premeasure(frameNode_, constraint);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->complete);
    EXPECT_FALSE(result->reachedOppositeEdge);
    EXPECT_EQ(VisibleRange(info->visibleStartIndex_, info->visibleEndIndex_), visible);
    EXPECT_EQ(VisibleRange(info->startIndex_, info->endIndex_), range);
    EXPECT_FLOAT_EQ(info->totalMainSize_ - info->posMap_.at(27).startPos, 300.0f);
    for (int32_t index = 0; index < 30; ++index) {
        auto child = GetChildFrameNode(frameNode_, index);
        EXPECT_EQ(child->IsActive(), states[index].first);
        EXPECT_EQ(child->GetGeometryNode()->GetFrameOffset(), states[index].second);
    }
}
/**
 * @tc.name: PremeasureIncompleteRowIsNotPublished001
 * @tc.desc: An unmeasurable cell must not produce an exact row or a completed range result.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PremeasureIncompleteRowIsNotPublished001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    auto context = CreateCachedLazyLayoutTestContext(V2::LAZY_V_GRID_LAYOUT_ETS_TAG,
        AceType::MakeRefPtr<LazyGridLayoutPattern>(), AceType::MakeRefPtr<LazyGridLayoutAlgorithm>(info),
        SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    context.host->GetLayoutProperty<LazyGridLayoutProperty>()->UpdateColumnsTemplate("1fr 1fr");
    int32_t measured = 0;
    for (int32_t index = 0; index < 4; ++index) {
        AppendFixedHeightChild(context.wrapper, -4500 - index, 100.0f, measured);
    }
    context.wrapper->GetOrCreateChildByIndex(3, false, true)->GetLayoutAlgorithm()->SetLayoutAlgorithm(nullptr);
    auto constraint = context.parentConstraint;
    constraint.maxSize.SetHeight(Infinity<float>());
    constraint.viewPosRef = ViewPosReference {
        .viewPosStart = -100.0f, .viewPosEnd = 0.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::END, .axis = Axis::VERTICAL,
    };
    auto result = LazyLayoutUtils::Premeasure(context.wrapper, constraint);
    ASSERT_TRUE(result);
    EXPECT_FALSE(result->complete);
    EXPECT_FALSE(result->reachedOppositeEdge);
    EXPECT_TRUE(info->posMap_.empty());
    EXPECT_EQ(measured, 1);
}

/**
 * @tc.name: PremeasureKeepsLazyForEachJSActive001
 * @tc.desc: Sparse LazyForEach indexes cannot alias a footer or freeze the published active item.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PremeasureKeepsLazyForEachJSActive001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<LazyGridLayoutPattern>();
    auto context = CreateCachedLazyLayoutTestContext(V2::LAZY_V_GRID_LAYOUT_ETS_TAG, pattern,
        AceType::MakeRefPtr<LazyGridLayoutAlgorithm>(pattern->layoutInfo_), SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    context.host->GetLayoutProperty<LazyGridLayoutProperty>()->UpdateColumnsTemplate("1fr");
    auto builder = AceType::MakeRefPtr<PremeasureLazyBuilder>();
    auto lazy = LazyForEachNode::CreateLazyForEachNode(-4600, builder);
    context.host->AddChild(lazy);
    auto footer = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG, -4601, AceType::MakeRefPtr<Pattern>());
    footer->GetLayoutProperty()->UpdateUserDefinedIdealSize(CalcSize(CalcLength(100.0f), CalcLength(70.0f)));
    LazyGridLayoutModel::SetFooter(AceType::RawPtr(context.host), footer);
    auto visible = AceType::DynamicCast<FrameNode>(lazy->GetFrameChildByIndex(5, true, false, true));
    ASSERT_NE(visible, nullptr);
    context.host->SetActive(true);
    visible->SetActive(true);
    builder->visibleItem->SetJSViewActive(true);
    auto info = pattern->layoutInfo_;
    info->startIndex_ = info->endIndex_ = 5;
    info->cachedStartIndex_ = info->cachedEndIndex_ = 5;
    info->visibleStartIndex_ = info->visibleEndIndex_ = 5;
    info->posMap_[5] = { 0, 500.0f, 600.0f };
    info->totalItemCount_ = 10;
    info->estimateItemSize_ = 100.0f;
    info->footerMainSize_ = 70.0f;
    info->totalMainSize_ = 1070.0f;
    auto constraint = context.parentConstraint;
    constraint.maxSize.SetHeight(Infinity<float>());
    constraint.viewPosRef = ViewPosReference {
        .viewPosStart = 0.0f, .viewPosEnd = 650.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::START, .axis = Axis::VERTICAL,
    };
    auto result = LazyLayoutUtils::Premeasure(context.host, constraint);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->complete);
    EXPECT_TRUE(visible->IsActive());
    EXPECT_TRUE(builder->visibleItem->GetJsActive());
    EXPECT_EQ(info->visibleStartIndex_, 5);
    EXPECT_EQ(info->visibleEndIndex_, 5);
    EXPECT_EQ(info->posMap_.size(), 7);
    EXPECT_FLOAT_EQ(info->posMap_.at(1).endPos - info->posMap_.at(1).startPos, 100.0f);
    EXPECT_FLOAT_EQ(info->footerMainSize_, 70.0f);
    EXPECT_FLOAT_EQ(info->totalMainSize_, 1070.0f);
}

/**
 * @tc.name: PremeasureUsesChildMeasureLifecycle001
 * @tc.desc: Offscreen measurement refreshes real child content without publishing the container geometry.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PremeasureUsesChildMeasureLifecycle001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<LazyGridLayoutPattern>();
    auto context = CreateCachedLazyLayoutTestContext(V2::LAZY_V_GRID_LAYOUT_ETS_TAG, pattern,
        AceType::MakeRefPtr<LazyGridLayoutAlgorithm>(pattern->layoutInfo_), SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    context.host->GetLayoutProperty<LazyGridLayoutProperty>()->UpdateColumnsTemplate("1fr");
    std::vector<RefPtr<PremeasureHookPattern>> children;
    for (int32_t index = 0; index < 10; ++index) {
        auto childPattern = AceType::MakeRefPtr<PremeasureHookPattern>();
        auto child = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG, -4900 - index, childPattern);
        context.host->AddChild(child);
        children.push_back(childPattern);
    }
    const auto previousSize = context.host->GetGeometryNode()->GetFrameSize();
    auto constraint = context.parentConstraint;
    constraint.maxSize.SetHeight(Infinity<float>());
    constraint.viewPosRef = ViewPosReference {
        .viewPosStart = 0.0f, .viewPosEnd = 200.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::START, .axis = Axis::VERTICAL,
    };
    auto result = LazyLayoutUtils::Premeasure(context.host, constraint);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->complete);
    EXPECT_FALSE(result->reachedOppositeEdge);
    EXPECT_FLOAT_EQ(result->marginMainSize, 1250.0f);
    EXPECT_EQ(context.host->GetGeometryNode()->GetFrameSize(), previousSize);
    EXPECT_EQ(pattern->layoutInfo_->visibleStartIndex_, -1);
    for (int32_t index = 0; index < 10; ++index) {
        EXPECT_EQ(children[index]->beforeMeasureCount, index < 2 ? 1 : 0);
        EXPECT_FALSE(children[index]->GetHost()->IsActive());
    }
}

/**
 * @tc.name: PremeasureRejectsDeferredGeometry001
 * @tc.desc: GONE is measured as zero, but pre-make and deferred layout are not completed measurements.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PremeasureRejectsDeferredGeometry001, TestSize.Level1)
{
    auto node = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG, -4910, AceType::MakeRefPtr<Pattern>());
    node->GetLayoutProperty()->UpdateVisibility(VisibleType::GONE);
    node->Measure(LayoutConstraintF());
    EXPECT_TRUE(LazyLayoutUtils::IsChildMeasureComplete(node));
    node->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
    EXPECT_FALSE(LazyLayoutUtils::IsChildMeasureComplete(node));
    auto deferred = AceType::MakeRefPtr<LayoutWrapperNode>(
        node, AceType::MakeRefPtr<GeometryNode>(), node->GetLayoutProperty());
    deferred->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(
        AceType::MakeRefPtr<PremeasureDeferredAlgorithm>()));
    deferred->Measure(LayoutConstraintF());
    EXPECT_TRUE(deferred->GetLayoutAlgorithm()->GetLayoutAlgorithm()->GetHasMeasured());
    EXPECT_FALSE(LazyLayoutUtils::IsChildMeasureComplete(deferred));
}
/**
 * @tc.name: KnownExtentKeepsRealRows001
 * @tc.desc: Known extent redistributes only unknown rows, including spacing and an incomplete final row.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, KnownExtentKeepsRealRows001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    info->lanes_ = 3;
    info->totalItemCount_ = 7;
    info->spaceWidth_ = 10.0f;
    info->startIndex_ = 6;
    info->totalMainSize_ = 900.0f;
    for (int32_t index = 0; index < 3; ++index) {
        info->posMap_[index] = { index, 0.0f, 120.0f };
    }
    info->posMap_[6] = { 0, 600.0f, 800.0f };
    ASSERT_TRUE(info->ApplyKnownBodySize(900.0f, 620.0f));
    EXPECT_FLOAT_EQ(info->estimateItemSize_, 280.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(0).endPos, 120.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(6).startPos, 420.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(6).endPos, 620.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.start, -180.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.end, -100.0f);
    EXPECT_EQ(info->posMap_.size(), 4);
    for (int32_t index = 3; index < 6; ++index) {
        info->posMap_[index] = { index - 3, 130.0f, 410.0f };
    }
    ASSERT_TRUE(info->ApplyKnownBodySize(620.0f, 620.0f));
    EXPECT_FLOAT_EQ(info->posMap_.at(3).startPos, 130.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(6).startPos, 420.0f);
}

/**
 * @tc.name: KnownExtentRejectsIncompleteMeasurement001
 * @tc.desc: An unfinished row or a total smaller than its real rows cannot certify an endpoint.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, KnownExtentRejectsIncompleteMeasurement001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    info->lanes_ = 2;
    info->totalItemCount_ = 4;
    info->posMap_[0] = { 0, 0.0f, 100.0f };
    EXPECT_FALSE(info->ApplyKnownBodySize(0.0f, 400.0f));
    info->posMap_[1] = { 1, 0.0f, 100.0f };
    EXPECT_FALSE(info->ApplyKnownBodySize(0.0f, 50.0f));
    EXPECT_FLOAT_EQ(info->posMap_.at(0).endPos, 100.0f);
    EXPECT_FALSE(info->ApplyKnownBodySize(0.0f, std::numeric_limits<float>::infinity()));
    EXPECT_TRUE(info->ApplyKnownBodySize(0.0f, 400.0f));
    EXPECT_FLOAT_EQ(info->estimateItemSize_, 300.0f);
}

namespace {
CachedLazyLayoutTestContext CreateUnsupportedGridContext(
    const RefPtr<LazyGridLayoutInfo>& info, int32_t& measured)
{
    auto pattern = AceType::MakeRefPtr<LazyGridLayoutPattern>();
    auto context = CreateCachedLazyLayoutTestContext(V2::LAZY_V_GRID_LAYOUT_ETS_TAG, pattern,
        AceType::MakeRefPtr<LazyGridLayoutAlgorithm>(info), SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    context.host->GetLayoutProperty<LazyGridLayoutProperty>()->UpdateColumnsTemplate("1fr 1fr");
    AppendFixedHeightChild(context.wrapper, -REJECTED_HEADER_NODE_ID, 70.0f, measured);
    pattern->header_ = context.wrapper->GetOrCreateChildByIndex(0, false, true)->GetHostNode();
    for (int32_t index = 0; index < PREMEASURE_ITEM_COUNT; ++index) {
        AppendFixedHeightChild(context.wrapper, -REJECTED_CHILD_NODE_ID - index,
            index < PREMEASURE_LANES ? 300.0f : 100.0f, measured);
        if (index >= REJECTED_START_INDEX) {
            info->posMap_[index] = { index % 2, (index / 2) * 100.0f, (index / 2 + 1) * 100.0f };
        }
    }
    context.wrapper->GetOrCreateChildByIndex(REJECTED_START_INDEX, false, true)->GetHostNode()->SetNeedLazyLayout(true);
    info->lanes_ = PREMEASURE_LANES;
    info->totalItemCount_ = PREMEASURE_ITEM_COUNT;
    info->estimateItemSize_ = 100.0f;
    info->headerMainSize_ = 50.0f;
    info->totalMainSize_ = 550.0f;
    info->startIndex_ = info->layoutedStartIndex_ = info->cachedStartIndex_ = info->visibleStartIndex_ =
        REJECTED_START_INDEX;
    info->endIndex_ = info->layoutedEndIndex_ = info->cachedEndIndex_ = info->visibleEndIndex_ = REJECTED_END_INDEX;
    info->layoutedStart_ = info->cacheStartPos_ = 200.0f;
    info->layoutedEnd_ = info->cacheEndPos_ = 300.0f;
    info->adjustOffset_ = { 7.0f, 9.0f };
    context.wrapper->GetGeometryNode()->SetFrameSize(SizeF(SCROLL_WIDTH, 550.0f));
    context.parentConstraint.maxSize.SetHeight(Infinity<float>());
    context.parentConstraint.viewPosRef = ViewPosReference {
        .viewPosStart = 0.0f, .viewPosEnd = 500.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::START, .axis = Axis::VERTICAL,
    };
    return context;
}
} // namespace

/**
 * @tc.name: PremeasureUnsupportedFinalizesRows001
 * @tc.desc: A rejected later cell finalizes earlier full rows and resized edges without publishing a partial row.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PremeasureUnsupportedFinalizesRows001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    int32_t measured = 0;
    auto context = CreateUnsupportedGridContext(info, measured);
    auto active = context.wrapper->GetOrCreateChildByIndex(5, false, true);
    active->SetActive(true);
    auto result = LazyLayoutUtils::Premeasure(context.wrapper, context.parentConstraint);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->unsupported);
    EXPECT_FALSE(result->complete);
    EXPECT_FALSE(result->reachedOppositeEdge);
    EXPECT_EQ(measured, 4);
    EXPECT_FLOAT_EQ(info->headerMainSize_, 70.0f);
    EXPECT_FLOAT_EQ(info->totalMainSize_, 770.0f);
    EXPECT_FLOAT_EQ(result->marginMainSize, 770.0f);
    EXPECT_FLOAT_EQ(result->adjust.start, 220.0f);
    EXPECT_FLOAT_EQ(result->adjust.end, 0.0f);
    EXPECT_EQ(info->posMap_.size(), 8);
    EXPECT_EQ(info->posMap_.count(2), 0);
    EXPECT_EQ(info->posMap_.count(3), 0);
    EXPECT_FLOAT_EQ(info->posMap_.at(0).endPos, 300.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(4).startPos, 400.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(9).endPos, 700.0f);
    EXPECT_EQ(info->updatedStart_, INT_MAX);
    EXPECT_EQ(info->updatedEnd_, -1);
    EXPECT_FLOAT_EQ(info->layoutedStart_, 400.0f);
    EXPECT_FLOAT_EQ(info->layoutedEnd_, 500.0f);
    EXPECT_FLOAT_EQ(info->cacheStartPos_, 400.0f);
    EXPECT_FLOAT_EQ(info->cacheEndPos_, 500.0f);
    EXPECT_EQ(info->startIndex_, 4);
    EXPECT_EQ(info->endIndex_, 5);
    EXPECT_EQ(info->visibleStartIndex_, 4);
    EXPECT_EQ(info->visibleEndIndex_, 5);
    EXPECT_TRUE(active->IsActive());
    EXPECT_FLOAT_EQ(info->adjustOffset_.start, 7.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.end, 9.0f);
    EXPECT_FLOAT_EQ(context.wrapper->GetGeometryNode()->GetFrameSize().Height(), 550.0f);
}

/**
 * @tc.name: PremeasureUnsupportedThenNormalMeasure001
 * @tc.desc: Normal measurement preserves a finalized prefix and consumes no duplicate anchor adjustment.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PremeasureUnsupportedThenNormalMeasure001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    int32_t measured = 0;
    auto context = CreateUnsupportedGridContext(info, measured);
    auto result = LazyLayoutUtils::Premeasure(context.wrapper, context.parentConstraint);
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->unsupported);
    auto constraint = context.parentConstraint;
    constraint.viewPosRef->viewPosStart = 250.0f;
    constraint.viewPosRef->viewPosEnd = 350.0f;
    constraint.viewPosRef->referencePos -= result->adjust.start;
    context.wrapper->SetActive(true);
    context.wrapper->Measure(constraint);
    ASSERT_EQ(info->posMap_.size(), 10);
    EXPECT_FLOAT_EQ(info->posMap_.at(1).endPos, 300.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(2).startPos, 300.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(3).endPos, 400.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(4).startPos, 400.0f);
    EXPECT_FLOAT_EQ(info->totalMainSize_, 770.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.start, 0.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.end, 0.0f);
}

/**
 * @tc.name: PremeasureRejectedKnownExtentKeepsPartial001
 * @tc.desc: Rejecting an impossible known extent still returns the finalized rows and their adjustment.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PremeasureRejectedKnownExtentKeepsPartial001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    int32_t measured = 0;
    auto context = CreateUnsupportedGridContext(info, measured);
    context.parentConstraint.viewPosRef->viewPosEnd = 270.0f;
    context.parentConstraint.viewPosRef->knownMainSize = 170.0f;
    auto result = LazyLayoutUtils::Premeasure(context.wrapper, context.parentConstraint);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->unsupported);
    EXPECT_FALSE(result->complete);
    EXPECT_FALSE(result->reachedOppositeEdge);
    EXPECT_EQ(measured, 3);
    EXPECT_FLOAT_EQ(result->marginMainSize, 770.0f);
    EXPECT_FLOAT_EQ(result->adjust.start, 220.0f);
    EXPECT_FLOAT_EQ(result->adjust.end, 0.0f);
    EXPECT_FLOAT_EQ(info->totalMainSize_, 770.0f);
    EXPECT_EQ(info->updatedStart_, INT_MAX);
}

namespace {
CachedLazyLayoutTestContext CreateUnpublishedGridContext(
    const RefPtr<LazyGridLayoutInfo>& info, int32_t& measured)
{
    auto pattern = AceType::MakeRefPtr<LazyGridLayoutPattern>();
    pattern->layoutInfo_ = info;
    auto context = CreateCachedLazyLayoutTestContext(V2::LAZY_V_GRID_LAYOUT_ETS_TAG, pattern,
        AceType::MakeRefPtr<LazyGridLayoutAlgorithm>(info), SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    context.host->GetLayoutProperty<LazyGridLayoutProperty>()->UpdateColumnsTemplate("1fr 1fr");
    for (int32_t index = 0; index < COMMITTED_ITEM_COUNT; ++index) {
        AppendFixedHeightChild(context.wrapper, -COMMITTED_NODE_ID - index,
            index / PREMEASURE_LANES == COMMITTED_TALL_ROW ? 400.0f : 100.0f, measured);
        info->posMap_[index] = { index % 2, (index / 2) * 100.0f, (index / 2 + 1) * 100.0f };
    }
    context.wrapper->GetOrCreateChildByIndex(COMMITTED_REJECTED_INDEX, false, true)
        ->GetHostNode()->SetNeedLazyLayout(true);
    info->lanes_ = PREMEASURE_LANES;
    info->totalItemCount_ = COMMITTED_ITEM_COUNT;
    info->estimateItemSize_ = 100.0f;
    info->totalMainSize_ = 800.0f;
    info->startIndex_ = info->visibleStartIndex_ = 0;
    info->endIndex_ = info->visibleEndIndex_ = 1;
    context.wrapper->GetGeometryNode()->SetFrameSize(SizeF(SCROLL_WIDTH, 800.0f));
    context.parentConstraint.maxSize.SetHeight(Infinity<float>());
    context.parentConstraint.viewPosRef = ViewPosReference {
        .viewPosStart = -600.0f, .viewPosEnd = 0.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::END, .axis = Axis::VERTICAL,
    };
    return context;
}
} // namespace

/**
 * @tc.name: PremeasureCrossFrameEndUsesCommittedExtent001
 * @tc.desc: A new algorithm measures and activates the actual tail using the unpublished committed extent.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PremeasureCrossFrameEndUsesCommittedExtent001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    int32_t measured = 0;
    auto context = CreateUnpublishedGridContext(info, measured);
    auto result = LazyLayoutUtils::Premeasure(context.wrapper, context.parentConstraint);
    ASSERT_TRUE(result && result->unsupported);
    EXPECT_FLOAT_EQ(result->marginMainSize, 1100.0f);
    EXPECT_FLOAT_EQ(context.wrapper->GetGeometryNode()->GetFrameSize().Height(), 800.0f);
    ASSERT_TRUE(info->premeasuredMainSize_);
    auto normal = AceType::MakeRefPtr<LazyGridLayoutAlgorithm>(info);
    context.wrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(normal));
    context.parentConstraint.viewPosRef->viewPosStart = -50.0f;
    context.parentConstraint.viewPosRef->viewPosEnd = 50.0f;
    context.wrapper->SetActive(true);
    context.wrapper->Measure(context.parentConstraint);
    context.wrapper->Layout();
    EXPECT_FLOAT_EQ(normal->realMainSize_, 1100.0f);
    EXPECT_FLOAT_EQ(normal->startPos_, 1050.0f);
    EXPECT_FALSE(info->premeasuredMainSize_);
    EXPECT_EQ(info->visibleStartIndex_, 14);
    EXPECT_EQ(info->visibleEndIndex_, 15);
    auto tail = context.wrapper->GetOrCreateChildByIndex(15, false, true);
    EXPECT_TRUE(tail->IsActive());
    EXPECT_FLOAT_EQ(tail->GetGeometryNode()->GetFrameOffset().GetY(), 1000.0f);
    EXPECT_FLOAT_EQ(info->totalMainSize_, 1100.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.start, 0.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.end, 0.0f);
}

/**
 * @tc.name: PremeasureSameFrameEndConsumesAdjustOnce001
 * @tc.desc: End compensation is applied once against the committed extent while preserving the displayed head.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PremeasureSameFrameEndConsumesAdjustOnce001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    int32_t measured = 0;
    auto context = CreateUnpublishedGridContext(info, measured);
    auto result = LazyLayoutUtils::Premeasure(context.wrapper, context.parentConstraint);
    ASSERT_TRUE(result && result->unsupported);
    ASSERT_FLOAT_EQ(result->adjust.end, 300.0f);
    context.parentConstraint.viewPosRef->viewPosStart = 0.0f;
    context.parentConstraint.viewPosRef->viewPosEnd = 100.0f;
    context.parentConstraint.viewPosRef->referencePos = 750.0f + result->adjust.end;
    context.wrapper->SetActive(true);
    context.wrapper->Measure(context.parentConstraint);
    auto normal = AceType::DynamicCast<LazyGridLayoutAlgorithm>(
        context.wrapper->GetLayoutAlgorithm()->GetLayoutAlgorithm());
    ASSERT_NE(normal, nullptr);
    EXPECT_FLOAT_EQ(normal->startPos_, 50.0f);
    EXPECT_FLOAT_EQ(normal->endPos_, 150.0f);
    EXPECT_EQ(info->visibleStartIndex_, 0);
    EXPECT_EQ(info->visibleEndIndex_, 3);
    EXPECT_FALSE(info->premeasuredMainSize_);
    EXPECT_FLOAT_EQ(info->adjustOffset_.start, 0.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.end, 0.0f);
    context.wrapper->Measure(context.parentConstraint);
    EXPECT_FLOAT_EQ(normal->startPos_, 50.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.end, 0.0f);
}

/**
 * @tc.name: PremeasureExtentSurvivesCountChange001
 * @tc.desc: A newly appended row updates the committed baseline without rereading stale source geometry.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PremeasureExtentSurvivesCountChange001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    int32_t measured = 0;
    auto context = CreateUnpublishedGridContext(info, measured);
    ASSERT_TRUE(LazyLayoutUtils::Premeasure(context.wrapper, context.parentConstraint));
    AppendFixedHeightChild(context.wrapper, -5220, 100.0f, measured);
    AppendFixedHeightChild(context.wrapper, -5221, 100.0f, measured);
    context.parentConstraint.viewPosRef->viewPosStart = -50.0f;
    context.parentConstraint.viewPosRef->viewPosEnd = 50.0f;
    context.wrapper->Measure(context.parentConstraint);
    auto normal = AceType::DynamicCast<LazyGridLayoutAlgorithm>(
        context.wrapper->GetLayoutAlgorithm()->GetLayoutAlgorithm());
    ASSERT_NE(normal, nullptr);
    EXPECT_FLOAT_EQ(normal->realMainSize_, 1100.0f);
    EXPECT_FALSE(info->premeasuredMainSize_);
    EXPECT_EQ(info->totalItemCount_, 18);
    EXPECT_EQ(info->visibleEndIndex_, 17);
    EXPECT_FLOAT_EQ(info->totalMainSize_, 1200.0f);
}

/**
 * @tc.name: PremeasureExtentClearsOnlyForPublishedGone001
 * @tc.desc: Only a published GONE frame invalidates the pending extent, not temporary zero-size measurement.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PremeasureExtentClearsOnlyForPublishedGone001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    int32_t measured = 0;
    auto context = CreateUnpublishedGridContext(info, measured);
    ASSERT_TRUE(LazyLayoutUtils::Premeasure(context.wrapper, context.parentConstraint));
    context.host->GetLayoutProperty()->UpdateVisibility(VisibleType::GONE);
    auto gone = LazyLayoutUtils::Premeasure(context.wrapper, context.parentConstraint);
    ASSERT_TRUE(gone && gone->complete);
    EXPECT_FLOAT_EQ(gone->marginMainSize, 0.0f);
    EXPECT_FLOAT_EQ(context.host->GetGeometryNode()->GetFrameSize().Height(), 800.0f);
    EXPECT_TRUE(info->premeasuredMainSize_);
    context.host->Measure(context.parentConstraint);
    EXPECT_FLOAT_EQ(context.host->GetGeometryNode()->GetFrameSize().Height(), 0.0f);
    EXPECT_FALSE(context.host->IsLayoutDirtyMarked());
    DirtySwapConfig config;
    config.skipMeasure = true;
    config.skipLayout = true;
    context.host->GetPattern<LazyGridLayoutPattern>()->OnDirtyLayoutWrapperSwap(context.host, config);
    EXPECT_FALSE(info->premeasuredMainSize_);
}
/**
 * @tc.name: PreparedGeometryRebindsLogicalItemWithoutOwningRequest001
 * @tc.desc: One Pattern consumes each logical geometry once, accepts rebinds, and does not retain an expired request.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PreparedGeometryRebindsLogicalItemWithoutOwningRequest001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<LazyGridLayoutPattern>();
    auto host = FrameNode::CreateFrameNode(V2::LAZY_V_GRID_LAYOUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), pattern);
    EXPECT_EQ(pattern->GetPreparedGeometry(), nullptr);
    auto first = std::make_shared<PreparedLayoutGeometry>();
    first->type = PreparedLayoutType::GRID;
    first->itemCount = 100;
    first->bodyMainSize = 10000.0f;
    first->fromEnd = true;
    first->positions[99] = { -100.0f, 0.0f, 0 };
    pattern->RestorePreparedGeometry(first);
    EXPECT_EQ(pattern->GetPreparedGeometry(), first);
    EXPECT_FLOAT_EQ(pattern->layoutInfo_->posMap_.at(99).startPos, 9900.0f);
    pattern->layoutInfo_->posMap_.at(99).startPos = 9890.0f;
    pattern->RestorePreparedGeometry(first);
    EXPECT_FLOAT_EQ(pattern->layoutInfo_->posMap_.at(99).startPos, 9890.0f);
    auto second = std::make_shared<PreparedLayoutGeometry>(*first);
    second->positions.clear();
    second->positions[98] = { -300.0f, -100.0f, 0 };
    pattern->layoutInfo_->deadline_ = 100;
    pattern->layoutInfo_->referencePos_ = 500.0f;
    pattern->layoutInfo_->visibleStartIndex_ = 99;
    pattern->layoutInfo_->cacheEndPos_ = 10000.0f;
    pattern->RestorePreparedGeometry(second);
    EXPECT_FALSE(pattern->layoutInfo_->deadline_);
    EXPECT_FLOAT_EQ(pattern->layoutInfo_->referencePos_, 0.0f);
    EXPECT_EQ(pattern->layoutInfo_->visibleStartIndex_, -1);
    EXPECT_FLOAT_EQ(pattern->layoutInfo_->cacheEndPos_, 0.0f);
    EXPECT_EQ(pattern->layoutInfo_->posMap_.count(99), 0);
    EXPECT_FLOAT_EQ(pattern->layoutInfo_->posMap_.at(98).startPos, 9700.0f);
    std::weak_ptr<const PreparedLayoutGeometry> weak = second;
    second.reset();
    EXPECT_TRUE(weak.expired());
    EXPECT_EQ(pattern->GetPreparedGeometry(), nullptr);
}

/**
 * @tc.name: PreparedGeometryReverseWindowUsesMeasuredInterior001
 * @tc.desc: A cold reverse window begins at its prepared interior row, not the section's distant estimated tail.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PreparedGeometryReverseWindowUsesMeasuredInterior001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    info->totalItemCount_ = 100;
    info->premeasuredMainSize_ = 10000.0f;
    info->posMap_[89] = { 0, 8900.0f, 9000.0f };
    info->posMap_[90] = { 0, 9000.0f, 9100.0f };
    LazyGridLayoutAlgorithm algorithm(info);
    algorithm.totalItemCount_ = 100;
    algorithm.prevBodyMainSize_ = 10000.0f;
    algorithm.startPos_ = 8850.0f;
    algorithm.endPos_ = 8950.0f;
    int32_t index = -1;
    float position = 0.0f;
    algorithm.GetEndIndexInfo(index, position);
    EXPECT_EQ(index, 89);
    EXPECT_FLOAT_EQ(position, 9000.0f);
}

/**
 * @tc.name: PublicDumpPreservesFlatLayoutFields
 * @tc.desc: Public filtering preserves legacy field values without a layoutInfo wrapper.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, PublicDumpPreservesFlatLayoutFields, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<LazyGridLayoutPattern>();
    ASSERT_NE(pattern->layoutInfo_, nullptr);
    pattern->layoutInfo_->headerMainSize_ = 12.0f;
    pattern->layoutInfo_->footerMainSize_ = 24.0f;
    pattern->layoutInfo_->totalMainSize_ = 240.0f;
    auto legacy = JsonUtil::Create();
    pattern->DumpAdvanceInfo(legacy);
    ASSERT_TRUE(legacy->Contains("itemStartIndex"));
    for (bool visual : { false, true }) {
        for (bool functional : { false, true }) {
            ParamConfig config;
            config.dumpMode = TreeDumpMode::PUBLIC_TREE;
            config.visualProperties = visual;
            config.functionalProperties = functional;
            auto json = JsonUtil::Create();
            pattern->DumpAdvanceInfo(json, config);
            EXPECT_FALSE(json->Contains("layoutInfo"));
            EXPECT_FALSE(json->Contains("itemStartIndex"));
            EXPECT_EQ(json->Contains("headerMainSize"), visual);
            EXPECT_EQ(json->Contains("totalMainSize"), visual);
            EXPECT_EQ(json->Contains("spaceWidth"), visual);
            if (visual) {
                EXPECT_EQ(json->GetDouble("headerMainSize"), legacy->GetDouble("headerMainSize"));
                EXPECT_EQ(json->GetDouble("footerMainSize"), legacy->GetDouble("footerMainSize"));
                EXPECT_DOUBLE_EQ(json->GetDouble("totalMainSize"), 240.0);
            } else {
                EXPECT_EQ(json->ToString(), "{}");
            }
        }
    }
    auto after = JsonUtil::Create();
    pattern->DumpAdvanceInfo(after);
    EXPECT_EQ(after->ToString(), legacy->ToString());
}


/**
 * @tc.name: LegacyDumpMatchesBaselineSnapshot
 * @tc.desc: Preserve the baseline layout diagnostic keys, numeric values and insertion order.
 * @tc.type: FUNC
 */
HWTEST_F(LazyGridLayoutTest, LegacyDumpMatchesBaselineSnapshot, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyGridLayoutInfo>();
    info->startIndex_ = 2;
    info->endIndex_ = 6;
    info->totalItemCount_ = 9;
    info->headerMainSize_ = 12.0f;
    info->footerMainSize_ = 24.0f;
    info->spaceWidth_ = 3.0f;
    info->totalMainSize_ = 240.0f;
    // Frozen from fdf7d63988ed9fa338051a11c19fadf1f86653dc; compare serialized key order and types.
    const std::string expected = R"({"itemStartIndex":2,"itemEndIndex":6,"itemTotalCount":9,)"
        R"("headerMainSize":12,"footerMainSize":24,"spaceWidth":3,"totalMainSize":240})";
    auto json = JsonUtil::Create();
    info->DumpAdvanceInfo(json);
    EXPECT_EQ(json->ToString(), expected);
}

} // namespace OHOS::Ace::NG
