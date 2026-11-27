/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "grid_test_ng.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"
#include "core/components_ng/syntax/repeat_virtual_scroll_model_ng.h"
#include "core/components_ng/render/paint_wrapper.h"
#include "test/mock/frameworks/core/animation/mock_animation_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#include "test/mock/frameworks/core/rosen/mock_canvas.h"

#include "core/components_ng/pattern/grid/grid_item_pattern.h"
#include "core/components_ng/pattern/grid/grid_item_layout_property.h"
#include "core/components_ng/pattern/grid/grid_constants.h"
#include "core/components_ng/pattern/grid/grid_layout/grid_layout_algorithm.h"
#include "core/components_ng/pattern/grid/grid_paint_method.h"
#define private public
#include "core/components_ng/pattern/grid/grid_scroll/grid_scroll_layout_algorithm.h"
#undef private
#include "core/components_ng/pattern/grid/grid_scroll/grid_scroll_with_options_layout_algorithm.h"
#include "core/components_ng/pattern/scrollable/scrollable_model_ng.h"
#include "core/common/text_field_manager_ng.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t COLD_ITEM_COUNT = 120;
constexpr int32_t FIRST_EXPLICIT_ITEM = 39;
constexpr int32_t SECOND_EXPLICIT_ITEM = 40;
constexpr int32_t REFERENCE_TARGET_INDEX = 90;
constexpr int32_t REFERENCE_STEP_LIMIT = 1000;
constexpr int32_t VARIABLE_HEIGHT_PERIOD = 5;

// Zero-height-first-line scenario geometry, named per the G.CNS.02
// magic-number rule: the normal-height lines following the zero-height first
// line, and the scroll step that moves that first line above the content top.
constexpr int32_t FIXED_HEIGHT_ITEM_NUMBER = 5;
constexpr float SCROLL_PAST_ZERO_HEIGHT_LINE = 10.0f;
} // namespace

class GridScrollLayoutTestNg : public GridTestNg {
public:
    void UpdateLayoutInfo();
    RefPtr<GridPaintMethod> UpdateOverlayModifier();
    RefPtr<GridPaintMethod> UpdateContentModifier();
    // Single-column grid with a zero-height first item; captures onScrollIndex into reported when given.
    void CreateZeroHeightFirstLineGrid(
        GridModelNG& model, std::vector<std::pair<int32_t, int32_t>>* reported = nullptr);

protected:
    void BuildHiddenSpanGrid(const std::shared_ptr<std::vector<int32_t>>& created,
        const std::shared_ptr<std::vector<int32_t>>& measured)
    {
        auto model = CreateGrid();
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetCachedCount(0);
        CreateItemsInLazyForEach(COLD_ITEM_COUNT, [created, measured](uint32_t index) {
            created->push_back(static_cast<int32_t>(index));
            if (index == FIRST_EXPLICIT_ITEM) {
                GridItemModelNG itemModel;
                itemModel.SetColumnStart(0);
                itemModel.SetColumnEnd(1);
                itemModel.SetRowStart(0);
                itemModel.SetRowEnd(1);
            }
            auto item = ViewStackProcessor::GetInstance()->GetMainFrameNode();
            item->measureCallback_ = std::make_unique<std::function<void(RefPtr<Kit::FrameNode>&)>>(
                [measured, index](RefPtr<Kit::FrameNode>&) { measured->push_back(static_cast<int32_t>(index)); });
            return 80.0f + static_cast<float>(index % VARIABLE_HEIGHT_PERIOD) * 15.0f;
        });
        CreateDone();
    }

protected:
    void ReadSingleCellPhysicalReference(Rect& reference)
    {
        bool inspected = false;
        for (int32_t step = 0;
            step < REFERENCE_STEP_LIMIT && pattern_->GetGridLayoutInfo().endIndex_ < REFERENCE_TARGET_INDEX; ++step) {
            UpdateCurrentOffset(-50.0f);
            if (!inspected && pattern_->GetGridLayoutInfo().startIndex_ <= FIRST_EXPLICIT_ITEM &&
                pattern_->GetGridLayoutInfo().endIndex_ >= SECOND_EXPLICIT_ITEM) {
                for (int32_t index : { FIRST_EXPLICIT_ITEM, SECOND_EXPLICIT_ITEM }) {
                    auto child = frameNode_->GetChildByIndex(index, true);
                    ASSERT_NE(child, nullptr);
                    auto prop = AceType::DynamicCast<GridItemLayoutProperty>(child->GetLayoutProperty());
                    ASSERT_NE(prop, nullptr);
                    EXPECT_EQ(prop->GetRealRowSpanValue(1), 1);
                    EXPECT_EQ(prop->GetRealColumnSpanValue(1), 1);
                    RecordProperty("item" + std::to_string(index) + "_real_span", "1x1");
                }
                inspected = true;
            }
        }
        ASSERT_TRUE(inspected);
        auto visible = pattern_->GetItemRect(REFERENCE_TARGET_INDEX);
        ASSERT_GT(visible.Width(), 0.0f);
        UpdateCurrentOffset(-static_cast<float>(visible.Top()));
        reference = pattern_->GetItemRect(REFERENCE_TARGET_INDEX);
        RecordProperty("physical_scroll_reference", reference.ToString());
    }
};

void GridScrollLayoutTestNg::UpdateLayoutInfo()
{
    GetGrid();
    ViewStackProcessor::GetInstance()->Finish();
    FlushUITasks(frameNode_);
    pattern_->info_.lineHeightMap_[0] = ITEM_MAIN_SIZE;
    pattern_->info_.gridMatrix_[0][0] = 0;
    pattern_->info_.gridMatrix_[0][1] = 1;
    pattern_->info_.gridMatrix_[1][0] = 0;
    pattern_->info_.gridMatrix_[1][1] = 1;
}

RefPtr<GridPaintMethod> GridScrollLayoutTestNg::UpdateOverlayModifier()
{
    auto paintWrapper = frameNode_->CreatePaintWrapper();
    RefPtr<GridPaintMethod> paintMethod = AceType::DynamicCast<GridPaintMethod>(paintWrapper->nodePaintImpl_);
    paintMethod->UpdateOverlayModifier(AceType::RawPtr(paintWrapper));
    return paintMethod;
}

RefPtr<GridPaintMethod> GridScrollLayoutTestNg::UpdateContentModifier()
{
    auto paintWrapper = frameNode_->CreatePaintWrapper();
    RefPtr<GridPaintMethod> paintMethod = AceType::DynamicCast<GridPaintMethod>(paintWrapper->nodePaintImpl_);
    paintMethod->UpdateContentModifier(AceType::RawPtr(paintWrapper));
    return paintMethod;
}

/**
 * @tc.name: GridPaintMethodTest001
 * @tc.desc: Test grid paint method GetForegroundDrawFunction function
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GridPaintMethodTest001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr");
    CreateFixedItems(10);
    CreateDone();
    auto paintMethod = AceType::DynamicCast<GridPaintMethod>(pattern_->CreateNodePaintMethod());
    auto paintProperty = pattern_->CreatePaintProperty();
    PaintWrapper paintWrapper(frameNode_->GetRenderContext(), frameNode_->GetGeometryNode(), paintProperty);
    auto drawFunction = paintMethod->GetForegroundDrawFunction(&paintWrapper);
    Testing::MockCanvas rsCanvas;
    EXPECT_CALL(rsCanvas, DetachPen()).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DrawRect(_)).WillRepeatedly(Return());
    drawFunction(rsCanvas);
    paintMethod = nullptr;
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
 * @tc.name: ScrollLayout001
 * @tc.desc: Test UpdateOffsetOnVirtualKeyboardHeightChange
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, ScrollLayout001, TestSize.Level1)
{
    auto textFieldManager = AceType::MakeRefPtr<TextFieldManagerNG>();
    MockPipelineContext::GetCurrent()->SetTextFieldManager(textFieldManager);
    auto textFieldPattern = AceType::MakeRefPtr<Pattern>();
    textFieldManager->SetOnFocusTextField(textFieldPattern);
    const Offset clickPosition = Offset(100.f, HEIGHT + ITEM_MAIN_SIZE);
    textFieldManager->SetClickPosition(clickPosition);

    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    model.SetMaxCount(2);
    CreateFixedItems(18);
    CreateDone();
    const float smallerHeight = HEIGHT - ITEM_MAIN_SIZE;
    RectF gridRect(0, 0, WIDTH, smallerHeight);
    auto mockRenderContext = AceType::DynamicCast<MockRenderContext>(frameNode_->renderContext_);
    mockRenderContext->rect_ = gridRect;

    /**
     * @tc.steps: step1. Change to smaller mainSize
     * @tc.expected: The mainSize is correct
     */
    auto gridFocus = frameNode_->GetOrCreateFocusHub();
    gridFocus->RequestFocusImmediately();
    // change grid height and trigger Measure
    ViewAbstract::SetHeight(AceType::RawPtr(frameNode_), CalcLength(smallerHeight));
    FlushUITasks();
    float gridHeight = frameNode_->GetGeometryNode()->GetFrameSize().Height();
    EXPECT_FLOAT_EQ(gridHeight, smallerHeight);
}

/**
 * @tc.name: ScrollLayout002
 * @tc.desc: Test UpdateOffsetOnVirtualKeyboardHeightChange that currentOffset wuold not change
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, ScrollLayout002, TestSize.Level1)
{
    auto textFieldManager = AceType::MakeRefPtr<TextFieldManagerNG>();
    MockPipelineContext::GetCurrent()->SetTextFieldManager(textFieldManager);
    auto textFieldPattern = AceType::MakeRefPtr<Pattern>();
    textFieldManager->SetOnFocusTextField(textFieldPattern);
    const Offset clickPosition = Offset(100.f, 100.f);
    textFieldManager->SetClickPosition(clickPosition);

    GridModelNG model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr 1fr 1fr");
    model.SetMaxCount(2);
    CreateFixedItems(18);
    CreateDone();

    /**
     * @tc.steps: step1. While axis_ == Axis::HORIZONTAL
     * @tc.expected: currentOffset_ would not change
     */
    pattern_->info_.axis_ = Axis::HORIZONTAL;
    const float smallerHeight = HEIGHT - ITEM_MAIN_SIZE;
    // change grid height and trigger Measure
    ViewAbstract::SetHeight(AceType::RawPtr(frameNode_), CalcLength(smallerHeight));
    FlushUITasks();
    float currentOffset = pattern_->info_.currentOffset_;
    EXPECT_FLOAT_EQ(currentOffset, 0);

    /**
     * @tc.steps: step2. While Grid !IsCurrentFocus()
     * @tc.expected: currentOffset_ would not change
     */
    ViewAbstract::SetHeight(AceType::RawPtr(frameNode_), CalcLength(HEIGHT));
    FlushUITasks();
    pattern_->info_.axis_ = Axis::VERTICAL;
    // change grid height and trigger Measure
    ViewAbstract::SetHeight(AceType::RawPtr(frameNode_), CalcLength(smallerHeight));
    FlushUITasks();
    currentOffset = pattern_->info_.currentOffset_;
    EXPECT_FLOAT_EQ(currentOffset, 0);

    /**
     * @tc.steps: step3. While clickPosition is in Grid
     * @tc.expected: currentOffset_ would not change
     */
    ViewAbstract::SetHeight(AceType::RawPtr(frameNode_), CalcLength(HEIGHT));
    FlushUITasks();
    pattern_->info_.axis_ = Axis::VERTICAL;
    // change grid height and trigger Measure
    ViewAbstract::SetHeight(AceType::RawPtr(frameNode_), CalcLength(smallerHeight));
    FlushUITasks();
    currentOffset = pattern_->info_.currentOffset_;
    EXPECT_FLOAT_EQ(currentOffset, 0);
}

/**
 * @tc.name: GridScrollTest001
 * @tc.desc: Test FireOnScrollBarUpdate Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GridScrollTest001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    ScrollBarUpdateFunc scrollFunc = [](int32_t index, Dimension offset) {
        std::optional<float> horizontalOffset = offset.ConvertToPx();
        std::optional<float> verticalOffset = offset.ConvertToPx();
        return std::make_pair(horizontalOffset, verticalOffset);
    };
    model.SetRowsTemplate("1fr 1fr");
    model.SetRowsGap(Dimension(5));
    model.SetOnScrollBarUpdate(std::move(scrollFunc));
    CreateBigItem(1, 1, 1, 2);
    CreateFixedItems(2);
    CreateDone();
    Dimension offset(1.0);
    auto fireOnScroll = eventHub_->FireOnScrollBarUpdate(1.0, offset);
    EXPECT_FLOAT_EQ(fireOnScroll.first.value(), 1.0f);
    EXPECT_FLOAT_EQ(fireOnScroll.second.value(), 1.0f);
}

/**
 * @tc.name: GridScrollTest002
 * @tc.desc: Test CalculateLargeItemOffset Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GridScrollTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create gridItem and initialize related properties.
     */
    GridModelNG model;
    model.Create(nullptr, nullptr);
    ViewAbstract::SetWidth(CalcLength(WIDTH));
    ViewAbstract::SetHeight(CalcLength(HEIGHT));
    model.SetRowsTemplate("1fr 1fr");
    model.SetRowsGap(Dimension(5));
    UpdateLayoutInfo();
    auto gridScrollLayoutAlgorithm = AceType::MakeRefPtr<GridScrollLayoutAlgorithm>(pattern_->info_);
    ASSERT_NE(gridScrollLayoutAlgorithm, nullptr);
    auto ret = gridScrollLayoutAlgorithm->CalculateLargeItemOffset(OffsetF(100, 100), 0, 1, 0);
    EXPECT_EQ(ret.GetX(), 100.f - ITEM_MAIN_SIZE);
    EXPECT_EQ(ret.GetY(), 100.f);
}

/**
 * @tc.name: GridScrollTest003
 * @tc.desc: Test CalculateLargeItemOffset Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GridScrollTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create gridItem and initialize related properties.
     */
    GridModelNG model;
    model.Create(nullptr, nullptr);
    ViewAbstract::SetWidth(CalcLength(WIDTH));
    ViewAbstract::SetHeight(CalcLength(HEIGHT));
    model.SetColumnsTemplate("1fr 1fr");
    model.SetRowsGap(Dimension(5));
    CreateFixedItems(10);
    UpdateLayoutInfo();
    auto gridScrollLayoutAlgorithm = AceType::MakeRefPtr<GridScrollLayoutAlgorithm>(pattern_->info_);
    ASSERT_NE(gridScrollLayoutAlgorithm, nullptr);
    auto ret = gridScrollLayoutAlgorithm->CalculateLargeItemOffset(OffsetF(0, 100), 1, 1, 0);
    EXPECT_EQ(ret.GetY(), 100.0f);
    EXPECT_EQ(ret.GetX(), 0.0f);
}

/**
 * @tc.name: GridScrollTest004
 * @tc.desc: Test CalculateLargeItemOffset Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GridScrollTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create gridItem and initialize related properties.
     */
    GridModelNG model;
    model.Create(nullptr, nullptr);
    ViewAbstract::SetWidth(CalcLength(WIDTH));
    ViewAbstract::SetHeight(CalcLength(HEIGHT));
    model.SetRowsTemplate("1fr 1fr");
    model.SetRowsGap(Dimension(5));
    UpdateLayoutInfo();
    auto gridScrollLayoutAlgorithm = AceType::MakeRefPtr<GridScrollLayoutAlgorithm>(pattern_->info_);
    gridScrollLayoutAlgorithm->info_.crossCount_ = 2;
    auto ret1 = gridScrollLayoutAlgorithm->CalculateLargeItemOffset(OffsetF(0, 100), 1, 1, 0);
    EXPECT_EQ(ret1.GetY(), 100.0f);
    EXPECT_EQ(ret1.GetX(), 0.0f);
}

/**
 * @tc.name: GridScrollTest005
 * @tc.desc: Test AdjustRowColSpan Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GridScrollTest005, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr");
    GridItemModelNG itemModel;
    itemModel.Create(GridItemStyle::NONE);
    itemModel.SetRowStart(NULL_VALUE);
    itemModel.SetRowEnd(NULL_VALUE);
    itemModel.SetColumnStart(NULL_VALUE);
    itemModel.SetColumnEnd(NULL_VALUE);
    ViewStackProcessor::GetInstance()->Pop();
    CreateDone();
    auto layoutProperty = GetChildLayoutProperty<GridItemLayoutProperty>(frameNode_, 0);
    EXPECT_EQ(layoutProperty->GetRowStart(), NULL_VALUE);
    EXPECT_EQ(layoutProperty->GetRowEnd(), NULL_VALUE);
    EXPECT_EQ(layoutProperty->GetColumnStart(), NULL_VALUE);
    EXPECT_EQ(layoutProperty->GetColumnEnd(), NULL_VALUE);
}

/**
 * @tc.name: GetTotalHeight001
 * @tc.desc: Test GetTotalHeight Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GetTotalHeight001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create 10 gridItem
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_EQ(pattern_->GetTotalHeight(), ITEM_MAIN_SIZE * 3);

    /**
     * @tc.steps: step2. Create 20 gridItem
     */
    model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    CreateFixedItems(20);
    CreateDone();
    EXPECT_EQ(pattern_->GetTotalHeight(), ITEM_MAIN_SIZE * 5);
}

/**
 * @tc.name: GetTotalHeightWithOptions
 * @tc.desc: Test total height of grid with different irregular heights.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GetTotalHeightWithOptions, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    GridLayoutOptions option;
    option.irregularIndexes = { 0, 2, 4 };
    model.SetLayoutOptions(option);
    CreateFixedHeightItems(1, 100);
    CreateFixedHeightItems(1, 50);
    CreateFixedHeightItems(1, 80);
    CreateFixedHeightItems(1, 50);
    CreateFixedHeightItems(1, 30);
    CreateDone();

    EXPECT_EQ(pattern_->GetTotalHeight(), 310);
}

/**
 * @tc.name: GetAverageHeight001
 * @tc.desc: Test GetAverageHeight Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GetAverageHeight001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create 10 gridItem
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_EQ(pattern_->GetAverageHeight(), 30);

    /**
     * @tc.steps: step2. Create 20 gridItem
     */
    ClearOldNodes();
    model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    CreateFixedItems(20);
    CreateDone();
    EXPECT_EQ(pattern_->GetAverageHeight(), 25);
}

/**
 * @tc.name: ChangeItemNumber001
 * @tc.desc: Test ChangeItemNumber
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, ChangeItemNumber001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    CreateFixedItems(5);
    CreateDone();

    /**
     * @tc.steps: step1. Add item
     * @tc.expected: The added item in the correct position
     */
    for (int32_t i = 0; i < 4; i++) {
        GridItemModelNG itemModel;
        itemModel.Create(GridItemStyle::NONE);
        ViewAbstract::SetHeight(CalcLength(Dimension(ITEM_MAIN_SIZE)));
        RefPtr<UINode> currentNode = ViewStackProcessor::GetInstance()->Finish();
        auto currentFrameNode = AceType::DynamicCast<FrameNode>(currentNode);
        currentFrameNode->MountToParent(frameNode_);
    }
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks();
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 6), OffsetF(120.0f, ITEM_MAIN_SIZE)));
    EXPECT_TRUE(IsEqual(GetChildOffset(frameNode_, 8), OffsetF(0, ITEM_MAIN_SIZE * 2)));
}

/**
 * @tc.name: UpdateGridMatrix001
 * @tc.desc: Test UpdateGridMatrix
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, UpdateGridMatrix001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create 100 items
     * @tc.expected: The added item in the correct position
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    CreateGridItems(100, ITEM_MAIN_SIZE, NULL_VALUE, GridItemStyle::NONE);
    CreateDone();

    /**
     * @tc.steps: step2. Scroll To 99 item
     * @tc.expected: Scroll to the correct position,lineHeightMap_ size is 25
     */
    pattern_->ScrollToIndex(99, true, ScrollAlign::END);
    EXPECT_TRUE(IsEqual(pattern_->info_.lineHeightMap_.size(), 25));
}

/**
 * @tc.name: GridLayout004
 * @tc.desc: Test GridLayoutAlgorithm for coverage
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GridLayout004, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    CreateFixedItems(10);
    CreateDone();

    /**
     * @tc.steps: step1. isVertical_ is true
     * @tc.expected: The curRow and curCol is correct
     */
    int32_t curRow = 0;
    int32_t curCol = 0;
    auto pattern = frameNode_->GetPattern<GridPattern>();
    auto algorithm = AceType::MakeRefPtr<GridLayoutAlgorithm>(GridLayoutInfo {});
    algorithm->crossCount_ = 2;
    algorithm->mainCount_ = 5;
    algorithm->GetNextGrid(curRow, curCol);
    EXPECT_EQ(curRow, 0);
    EXPECT_EQ(curCol, 1);
    EXPECT_TRUE(algorithm->isVertical_);
    algorithm->GetNextGrid(curRow, curCol);
    EXPECT_EQ(curRow, 1);
    EXPECT_EQ(curCol, 0);
    curRow = 1;
    curCol = 1;

    /**
     * @tc.steps: step2. isVertical_ is false
     * @tc.expected: The curRow and curCol is correct
     */
    algorithm->isVertical_ = false;
    algorithm->GetNextGrid(curRow, curCol);
    EXPECT_EQ(curRow, 2);
    EXPECT_EQ(curCol, 1);
    algorithm->GetNextGrid(curRow, curCol);
    EXPECT_EQ(curRow, 3);
    EXPECT_EQ(curCol, 1);
    curRow = 5;
    algorithm->GetNextGrid(curRow, curCol);
    EXPECT_EQ(curRow, 0);
    EXPECT_EQ(curCol, 2);
}

/**
 * @tc.name: UpdateOverlayModifier001
 * @tc.desc: Test grid paint method UpdateOverlayModifier function
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, UpdateOverlayModifier001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr");
    CreateFixedItems(10);
    CreateDone();

    /**
     * @tc.steps: step1. call UpdateOverlayModifier
     * @tc.expected: scrollBar is nullptr
     */
    auto paintMethod = UpdateOverlayModifier();
    auto scrollBar = paintMethod->scrollBar_.Upgrade();
    EXPECT_EQ(scrollBar, nullptr);
}

/**
 * @tc.name: UpdateOverlayModifier002
 * @tc.desc: Test grid paint method UpdateOverlayModifier function
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, UpdateOverlayModifier002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr");
    CreateFixedItems(10);
    CreateDone();

    /**
     * @tc.steps: step1. create scrollBar and paintMethod
     * @tc.expected: scrollBar is !nullptr
     */
    pattern_->scrollBar_ = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    auto paintMethod = UpdateOverlayModifier();
    auto scrollBar = paintMethod->scrollBar_.Upgrade();
    EXPECT_NE(scrollBar, nullptr);

    /**
     * @tc.steps: step2. call UpdateOverlayModifier
     * @tc.expected: AnimationTyp is correct
     */
    paintMethod = UpdateOverlayModifier();
    EXPECT_EQ(scrollBar->GetOpacityAnimationType(), OpacityAnimationType::NONE);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);

    /**
     * @tc.steps: step3. call UpdateOverlayModifier when scrollBarOverlayModifier is seted
     * @tc.expected: AnimationTyp is correct
     */
    pattern_->CreateScrollBarOverlayModifier();
    EXPECT_EQ(pattern_->scrollBarOverlayModifier_, nullptr);
    paintMethod->SetScrollBarOverlayModifier(pattern_->GetScrollBarOverlayModifier());
    auto scrollBarOverlayModifier = paintMethod->scrollBarOverlayModifier_.Upgrade();
    EXPECT_EQ(scrollBarOverlayModifier, nullptr);
    paintMethod = UpdateOverlayModifier();
    EXPECT_EQ(scrollBar->GetOpacityAnimationType(), OpacityAnimationType::NONE);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    scrollBar = nullptr;
}

/**
 * @tc.name: UpdateOverlayModifier003
 * @tc.desc: Test grid paint method UpdateOverlayModifier function
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, UpdateOverlayModifier003, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr");
    CreateFixedItems(10);
    CreateDone();

    /**
     * @tc.steps: step1. create scrollBar and paintMethod and ScrollBarOverlayModifier
     * @tc.expected: scrollBar is !nullptr
     */
    pattern_->scrollBar_ = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    pattern_->scrollBar_->SetScrollable(true);
    pattern_->scrollBar_->SetScrollBarHeight(Dimension(1.0, DimensionUnit::PERCENT));
    auto paintMethod = AceType::DynamicCast<GridPaintMethod>(pattern_->CreateNodePaintMethod());
    EXPECT_TRUE(pattern_->scrollBar_->NeedPaint());
    pattern_->CreateScrollBarOverlayModifier();
    EXPECT_NE(pattern_->scrollBarOverlayModifier_, nullptr);
    paintMethod->SetScrollBarOverlayModifier(pattern_->GetScrollBarOverlayModifier());
    auto scrollBarOverlayModifier = paintMethod->scrollBarOverlayModifier_.Upgrade();
    EXPECT_NE(scrollBarOverlayModifier, nullptr);
    auto scrollBar = paintMethod->scrollBar_.Upgrade();
    EXPECT_NE(scrollBar, nullptr);

    /**
     * @tc.steps: step2. call UpdateOverlayModifier
     * @tc.expected: AnimationTyp is correct
     */
    paintMethod = UpdateOverlayModifier();
    EXPECT_EQ(scrollBar->GetOpacityAnimationType(), OpacityAnimationType::NONE);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);

    /**
     * @tc.steps: step3. change PositionMode and call UpdateOverlayModifier
     * @tc.expected: AnimationTyp is correct
     */
    scrollBar->SetPositionMode(PositionMode::BOTTOM);
    paintMethod = UpdateOverlayModifier();
    scrollBar = paintMethod->scrollBar_.Upgrade();
    EXPECT_EQ(scrollBar->GetOpacityAnimationType(), OpacityAnimationType::NONE);
    EXPECT_EQ(scrollBar->GetHoverAnimationType(), HoverAnimationType::NONE);
    scrollBar = nullptr;
}

/**
 * @tc.name: PaintEdgeEffect001
 * @tc.desc: Test grid paint method PaintEdgeEffect function
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, PaintEdgeEffect001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr");
    CreateFixedItems(10);
    CreateDone();

    /**
     * @tc.steps: step1. init scrollBar
     * @tc.expected: scrollBar is !nullptr
     */
    pattern_->scrollBar_ = AceType::MakeRefPtr<ScrollBar>(DisplayMode::AUTO);
    pattern_->scrollBar_->SetScrollable(true);
    pattern_->scrollBar_->SetScrollBarHeight(Dimension(1.0, DimensionUnit::PERCENT));
    auto paintMethod = AceType::DynamicCast<GridPaintMethod>(pattern_->CreateNodePaintMethod());
    auto paintProperty = pattern_->CreatePaintProperty();
    PaintWrapper paintWrapper(frameNode_->GetRenderContext(), frameNode_->GetGeometryNode(), paintProperty);
    EXPECT_TRUE(pattern_->scrollBar_->NeedPaint());
    pattern_->CreateScrollBarOverlayModifier();
    EXPECT_NE(pattern_->scrollBarOverlayModifier_, nullptr);
    paintMethod->SetScrollBarOverlayModifier(pattern_->GetScrollBarOverlayModifier());
    auto scrollBarOverlayModifier = paintMethod->scrollBarOverlayModifier_.Upgrade();
    EXPECT_NE(scrollBarOverlayModifier, nullptr);
    auto scrollBar = paintMethod->scrollBar_.Upgrade();
    EXPECT_NE(scrollBar, nullptr);

    /**
     * @tc.steps: step2. call PaintEdgeEffect
     * @tc.expected: edgeEffect_ is !nullptr
     */
    auto scrollEdgeEffect = AceType::MakeRefPtr<ScrollEdgeEffect>();
    paintMethod->SetEdgeEffect(scrollEdgeEffect);
    Testing::MockCanvas rsCanvas;
    EXPECT_CALL(rsCanvas, DetachPen()).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, AttachBrush(_)).WillRepeatedly(ReturnRef(rsCanvas));
    EXPECT_CALL(rsCanvas, DrawRect(_)).WillRepeatedly(Return());
    EXPECT_CALL(rsCanvas, DetachBrush()).WillRepeatedly(ReturnRef(rsCanvas));
    paintMethod->PaintEdgeEffect(nullptr, rsCanvas);
    paintMethod->PaintEdgeEffect(&paintWrapper, rsCanvas);
    EXPECT_NE(paintMethod->edgeEffect_.Upgrade(), nullptr);
}

/**
 * @tc.name: GridScrollTest006
 * @tc.desc: Test SetOnScrollBarUpdate Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GridScrollTest006, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    ASSERT_NE(paintProperty_, nullptr);
    ScrollBarUpdateFunc scrollFunc = [](int32_t index, Dimension offset) {
        std::optional<float> horizontalOffset = offset.ConvertToPx();
        std::optional<float> verticalOffset = offset.ConvertToPx();
        return std::make_pair(horizontalOffset, verticalOffset);
    };
    model.SetRowsTemplate("1fr 1fr");
    CreateFixedItems(2);
    model.SetGridHeight(Dimension(5));
    model.SetScrollBarMode(DisplayMode::AUTO);
    model.SetScrollBarColor(Color());
    model.SetScrollBarColor(std::nullopt);
    auto scrollBarColor = paintProperty_->GetScrollBarColor();
    EXPECT_EQ(scrollBarColor, std::nullopt);
    model.SetScrollBarColor("#FF0000");
    model.SetScrollBarWidth("10vp");
    model.SetIsRTL(TextDirection::LTR);

    NestedScrollOptions nestedOpt;
    model.SetNestedScroll(std::move(nestedOpt));
    ScrollToIndexFunc value;
    model.SetOnScrollToIndex(std::move(value));
    CreateDone();
    auto paintProperty = frameNode_->GetPaintProperty<ScrollablePaintProperty>();
    EXPECT_EQ(paintProperty->GetBarStateString(), "BarState.Auto");

    auto pattern = frameNode_->GetPattern<GridPattern>();
    EXPECT_TRUE(pattern->isConfigScrollable_);
    auto eventHub = frameNode_->GetEventHub<GridEventHub>();
    EXPECT_FALSE(eventHub->onScrollToIndex_);
}

/**
 * @tc.name: GridScroll001
 * @tc.desc: Test SetSelected Function.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GridScroll001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create GridItemModelNG object
     */
    GridModelNG model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr");
    GridItemModelNG itemModel;
    itemModel.Create(GridItemStyle::NONE);
    itemModel.SetRowStart(NULL_VALUE);
    itemModel.SetRowEnd(NULL_VALUE);
    itemModel.SetColumnStart(NULL_VALUE);
    itemModel.SetColumnEnd(NULL_VALUE);
    ViewStackProcessor::GetInstance()->Pop();

    /**
     * @tc.steps: step2. Test Create function
     */
    std::function<void(int32_t)> deepRenderFunc = [](int32_t innerNodeId) {};
    bool isLazy = true;
    itemModel.Create(std::move(deepRenderFunc), isLazy, GridItemStyle::PLAIN);

    /**
     * @tc.steps: step3. invoke SetSelected function
     */
    itemModel.SetSelected(true);
    CreateDone();

    /**
     * @tc.expected: gridItemPattern->isSelected_ is true
     */
    auto gridItemPattern = GetChildPattern<GridItemPattern>(frameNode_, 1);
    EXPECT_TRUE(gridItemPattern->isSelected_);
}

/**
 * @tc.name: SupplyAllData2ZeroIndex001
 * @tc.desc: Test GridScrollLayoutAlgorithm::SupplyAllData2ZeroIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, SupplyAllData2ZeroIndex001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    CreateFixedItems(30);
    CreateDone();

    ScrollToIndex(20, true, ScrollAlign::END);

    EXPECT_EQ(pattern_->GetGridLayoutInfo().gridMatrix_.size(), 4);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().lineHeightMap_.size(), 4);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().gridMatrix_.at(0).at(0), 0);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().gridMatrix_.at(3).at(2), 11);
}

/**
 * @tc.name: SupplyAllData2ZeroIndex002
 * @tc.desc: Test GridScrollLayoutAlgorithm::SupplyAllData2ZeroIndex
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, SupplyAllData2ZeroIndex002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    CreateFixedItems(30);
    CreateDone();

    ScrollToIndex(20, true, ScrollAlign::START);
    ScrollToIndex(10, true, ScrollAlign::CENTER);

    EXPECT_EQ(pattern_->GetGridLayoutInfo().gridMatrix_.size(), 4);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().lineHeightMap_.size(), 4);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().gridMatrix_.at(0).at(0), 0);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().gridMatrix_.at(3).at(2), 11);
}

/**
 * @tc.name: SupplyAllData2ZeroIndex003
 * @tc.desc: Test supplying data with a multi-line first item and an empty recorded matrix
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, SupplyAllData2ZeroIndex003, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    CreateBigItem(0, 1, NULL_VALUE, NULL_VALUE, FILL_VALUE, ITEM_MAIN_SIZE);
    CreateFixedItems(19);
    CreateDone();

    auto layoutAlgorithmWrapper = frameNode_->GetLayoutAlgorithm();
    auto algorithm = AceType::DynamicCast<GridScrollLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_TRUE(algorithm);
    auto& info = algorithm->info_;
    info.gridMatrix_.clear();
    info.lineHeightMap_.clear();
    info.startIndex_ = 3;
    info.startMainLineIndex_ = 1;
    info.endIndex_ = 2;
    info.endMainLineIndex_ = 1;
    info.targetIndex_ = 2;

    algorithm->SupplyAllData2ZeroIndex(HEIGHT, WIDTH, AceType::RawPtr(frameNode_));

    ASSERT_TRUE(algorithm->infoCopy_);
    EXPECT_EQ(algorithm->infoCopy_->startIndex_, 0);
    EXPECT_EQ(algorithm->infoCopy_->startMainLineIndex_, 0);
    EXPECT_EQ(algorithm->infoCopy_->endIndex_, 2);
    EXPECT_EQ(algorithm->infoCopy_->endMainLineIndex_, 2);
    ASSERT_GT(algorithm->infoCopy_->gridMatrix_.count(0), 0);
    ASSERT_GT(algorithm->infoCopy_->gridMatrix_.count(1), 0);
    EXPECT_EQ(algorithm->infoCopy_->gridMatrix_.at(0).at(2), 2);
    EXPECT_EQ(algorithm->infoCopy_->gridMatrix_.at(1).at(0), 0);
    EXPECT_EQ(info.startIndex_, 3);
    EXPECT_EQ(info.startMainLineIndex_, 1);
    EXPECT_EQ(info.endIndex_, 2);
    EXPECT_EQ(info.endMainLineIndex_, 1);
    EXPECT_TRUE(info.gridMatrix_.empty());
}

/**
 * @tc.name: AnimateToIndexDuringReload
 * @tc.desc: The animated target is aligned using the new columns when the viewport is reset off the first page.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, AnimateToIndexDuringReload, TestSize.Level1)
{
    MockAnimationManager::GetInstance().Reset();
    MockAnimationManager::GetInstance().SetTicks(3);
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    CreateFixedItems(60);
    CreateDone();

    ScrollToIndex(18, false, ScrollAlign::START);
    ASSERT_EQ(pattern_->GetGridLayoutInfo().startIndex_, 18);
    layoutProperty_->UpdateColumnsTemplate("1fr 1fr");
    ASSERT_TRUE(pattern_->GetGridLayoutInfo().IsResetted());

    ScrollToIndex(36, true, ScrollAlign::START);
    EXPECT_FALSE(MockAnimationManager::GetInstance().AllFinished());
    TickToFinish();
    EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, 36);
    EXPECT_EQ(pattern_->GetTotalOffset(), 18 * ITEM_MAIN_SIZE);
    EXPECT_FLOAT_EQ(pattern_->GetItemRect(36).Top(), 0.0f);
}

/**
 * @tc.name: AnimateToIndexDuringPartialReload
 * @tc.desc: A partial reload of a grid with a spanning item keeps an animated target in the invalidated rows.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, AnimateToIndexDuringPartialReload, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    CreateBigColItem(0, 1);
    CreateFixedItems(30);
    CreateDone();
    ASSERT_TRUE(pattern_->GetGridLayoutInfo().hasBigItem_);
    ASSERT_EQ(pattern_->GetItemRect(5).Top(), 2 * ITEM_MAIN_SIZE);

    frameNode_->ChildrenUpdatedFrom(5);
    AnimateToIndexWithTicks(5, ScrollAlign::START);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, 5);
    EXPECT_EQ(pattern_->GetItemRect(5).Top(), 0.0f);
}

/**
 * @tc.name: AnimateToIndexReloadShrinksLastRow
 * @tc.desc: Shrinking the partially visible last row must not discard a pending animation to an earlier row.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, AnimateToIndexReloadShrinksLastRow, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedHeightItems(19, ITEM_MAIN_SIZE);
    CreateFixedHeightItems(1, 1000.0f);
    CreateDone();
    ScrollToIndex(19, false, ScrollAlign::START);
    UpdateCurrentOffset(-200.0f);
    ASSERT_EQ(pattern_->GetGridLayoutInfo().startIndex_, 19);
    ASSERT_FLOAT_EQ(pattern_->GetItemRect(19).Top(), -200.0f);

    auto lastItem = GetChildFrameNode(frameNode_, 19);
    ASSERT_NE(lastItem, nullptr);
    ViewAbstract::SetHeight(AceType::RawPtr(lastItem), CalcLength(ITEM_MAIN_SIZE));
    frameNode_->ChildrenUpdatedFrom(0);
    MockAnimationManager::GetInstance().SetTicks(3);
    ScrollToIndex(2, true, ScrollAlign::START);
    EXPECT_FALSE(MockAnimationManager::GetInstance().AllFinished());
    TickToFinish();
    EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, 2);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 2 * ITEM_MAIN_SIZE);
    EXPECT_FLOAT_EQ(pattern_->GetItemRect(2).Top(), 0.0f);
}

/**
 * @tc.name: AnimateToIndexReloadKeepsSpanningRow
 * @tc.desc: A span extending past the resized viewport affects the target row and the cached item's column.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, AnimateToIndexReloadKeepsSpanningRow, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(1, true);
    ViewAbstract::SetHeight(CalcLength(500.0f));
    CreateFixedHeightItems(4, ITEM_MAIN_SIZE);
    CreateBigItem(2, 3, 0, 0, NULL_VALUE, 2 * ITEM_MAIN_SIZE);
    CreateFixedHeightItems(20, ITEM_MAIN_SIZE);
    CreateDone();
    ASSERT_FLOAT_EQ(pattern_->GetItemRect(7).Top(), 4 * ITEM_MAIN_SIZE);

    // The same resize without a pending animation supplies a reference for the real cached child's geometry.
    auto resizeAndReload = [this](float height) {
        ViewAbstract::SetHeight(AceType::RawPtr(frameNode_), CalcLength(height));
        frameNode_->ChildrenUpdatedFrom(0);
    };
    resizeAndReload(300.0f);
    FlushUITasks();
    std::vector<RectF> referenceRects;
    for (int32_t index = 0; index <= 6; ++index) {
        referenceRects.emplace_back(GetChildRect(frameNode_, index));
    }
    ASSERT_FLOAT_EQ(referenceRects.back().Left(), WIDTH / 2);
    resizeAndReload(500.0f);
    FlushUITasks();

    resizeAndReload(300.0f);
    MockAnimationManager::GetInstance().SetTicks(3);
    ScrollToIndex(7, true, ScrollAlign::START);
    ASSERT_NE(pattern_->infoCopy_, nullptr);
    int32_t targetRow = -1;
    ASSERT_TRUE(pattern_->infoCopy_->GetLineIndexByIndex(7, targetRow));
    EXPECT_EQ(targetRow, 4);
    for (size_t index = 0; index < referenceRects.size(); ++index) {
        EXPECT_EQ(GetChildRect(frameNode_, static_cast<int32_t>(index)), referenceRects[index]) << "Item " << index;
    }
    TickToFinish();
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 4 * ITEM_MAIN_SIZE);
    EXPECT_FLOAT_EQ(pattern_->GetItemRect(7).Top(), 0.0f);
}

/**
 * @tc.name: AnimateToIndexReloadRebuildsTwoSpanningLines
 * @tc.desc: A span continuing beyond a nonzero viewport anchor must use its own height and retain both occupied cells.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, AnimateToIndexReloadRebuildsTwoSpanningLines, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    ViewAbstract::SetHeight(CalcLength(700.0f));
    CreateFixedHeightItems(6, ITEM_MAIN_SIZE);
    CreateBigItem(3, 5, 0, 0, NULL_VALUE, 3 * ITEM_MAIN_SIZE);
    CreateFixedHeightItems(1, 180.0f);
    CreateFixedHeightItems(20, ITEM_MAIN_SIZE);
    CreateDone();
    ScrollToIndex(4, false, ScrollAlign::START);
    ASSERT_EQ(pattern_->GetGridLayoutInfo().startMainLineIndex_, 2);

    ViewAbstract::SetHeight(AceType::RawPtr(frameNode_), CalcLength(280.0f));
    frameNode_->ChildrenUpdatedFrom(0);
    MockAnimationManager::GetInstance().SetTicks(3);
    ScrollToIndex(10, true, ScrollAlign::END);
    ASSERT_EQ(pattern_->GetGridLayoutInfo().startMainLineIndex_, 2);
    ASSERT_EQ(pattern_->GetGridLayoutInfo().endMainLineIndex_, 3);
    ASSERT_FALSE(MockAnimationManager::GetInstance().AllFinished());
    ASSERT_NE(pattern_->infoCopy_, nullptr);
    const auto& snapshot = *pattern_->infoCopy_;
    int32_t targetRow = -1;
    ASSERT_TRUE(snapshot.GetLineIndexByIndex(10, targetRow));
    EXPECT_EQ(targetRow, 6);
    for (int32_t line = 4; line <= 5; ++line) {
        auto row = snapshot.gridMatrix_.find(line);
        ASSERT_NE(row, snapshot.gridMatrix_.end());
        auto cell = row->second.find(0);
        ASSERT_NE(cell, row->second.end());
        EXPECT_EQ(cell->second, 6);
        auto height = snapshot.lineHeightMap_.find(line);
        ASSERT_NE(height, snapshot.lineHeightMap_.end());
        EXPECT_FLOAT_EQ(height->second, ITEM_MAIN_SIZE);
    }
    TickToFinish();
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 500.0f);
    EXPECT_FLOAT_EQ(pattern_->GetItemRect(10).Bottom(), 280.0f);
}

/**
 * @tc.name: AnimateToIndexReloadConsumesRequestAtStart
 * @tc.desc: A fresh layout after a successful animation at the start must not retain the old layout target.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, AnimateToIndexReloadConsumesRequestAtStart, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedHeightItems(20, ITEM_MAIN_SIZE);
    CreateDone();

    frameNode_->ChildrenUpdatedFrom(0);
    MockAnimationManager::GetInstance().SetTicks(3);
    ScrollToIndex(8, true, ScrollAlign::CENTER);
    ASSERT_FALSE(MockAnimationManager::GetInstance().AllFinished());
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().targetIndex_.has_value());
    TickToFinish();
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 650.0f);
    EXPECT_FLOAT_EQ(pattern_->GetItemRect(8).Top(), 150.0f);

    frameNode_->ChildrenUpdatedFrom(0);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    FlushUITasks();
    EXPECT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().targetIndex_.has_value());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 650.0f);
}

/**
 * @tc.name: AnimateToIndexReloadRemovesTarget
 * @tc.desc: Removing a requested last item before layout does not animate to its stale row.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, AnimateToIndexReloadRemovesTarget, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedHeightItems(8, ITEM_MAIN_SIZE);
    CreateDone();
    const int32_t target = 7;
    MockAnimationManager::GetInstance().SetTicks(3);
    pattern_->ScrollToIndex(target, true, ScrollAlign::START);
    frameNode_->RemoveChildAtIndex(target);
    frameNode_->ChildrenUpdatedFrom(target);
    FlushUITasks();

    EXPECT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    EXPECT_FALSE(pattern_->targetIndex_.has_value());
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().targetIndex_.has_value());
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);
}

/**
 * @tc.name: OnModifyDone001
 * @tc.desc: Test OnModifyDone
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, OnModifyDone001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Test OnModifyDone
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    model.SetMultiSelectable(true);
    CreateFixedItems(10);
    CreateDone();
    auto paintProperty = pattern_->GetPaintProperty<ScrollablePaintProperty>();
    EXPECT_TRUE(pattern_->multiSelectable_);
    EXPECT_TRUE(pattern_->isMouseEventInit_);
    EXPECT_TRUE(pattern_->GetScrollableEvent());
    EXPECT_TRUE(paintProperty->GetScrollBarProperty());
    EXPECT_TRUE(frameNode_->GetFocusHub());
    EXPECT_TRUE(pattern_->GetScrollableEvent()->GetScrollable());

    /**
     * @tc.steps: step2. Call OnModifyDone
     */
    pattern_->OnModifyDone();
    EXPECT_TRUE(pattern_->multiSelectable_);
    EXPECT_TRUE(pattern_->isMouseEventInit_);
    EXPECT_TRUE(pattern_->GetScrollableEvent());
    EXPECT_TRUE(paintProperty->GetScrollBarProperty());
    EXPECT_TRUE(frameNode_->GetFocusHub());
    EXPECT_TRUE(pattern_->GetScrollableEvent()->GetScrollable());

    /**
     * @tc.steps: step3. Change MultiSelectable and Call OnModifyDone
     */
    pattern_->SetMultiSelectable(false);
    pattern_->OnModifyDone();
    EXPECT_FALSE(pattern_->multiSelectable_);
    EXPECT_FALSE(pattern_->isMouseEventInit_);
}

/**
 * @tc.name: GetEndOffset001
 * @tc.desc: Test GetEndOffset
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GetEndOffset001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    CreateFixedItems(10);
    CreateDone();
    EXPECT_EQ(pattern_->GetEndOffset(), ITEM_MAIN_SIZE);
}

/**
 * @tc.name: GetEndOffset002
 * @tc.desc: Test GetEndOffset
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GetEndOffset002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    CreateFixedItems(10);
    CreateDone();
    pattern_->SetEdgeEffect(EdgeEffect::SPRING, true);
    EXPECT_EQ(pattern_->GetEndOffset(), 0);
}

/**
 * @tc.name: GetEndOffset003
 * @tc.desc: Test GetEndOffset
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GetEndOffset003, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    CreateFixedItems(20);
    CreateDone();
    pattern_->SetEdgeEffect(EdgeEffect::SPRING, true);
    EXPECT_EQ(pattern_->GetEndOffset(), 0);
}

/**
 * @tc.name: GetVisibleSelectedItems001
 * @tc.desc: Test GetVisibleSelectedItems
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GetVisibleSelectedItems001, TestSize.Level1)
{
    /**
     * @tc.cases: Set item(index:1) isSelected and call GetVisibleSelectedItems
     * @tc.expected: Has 1 item selected
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    CreateFixedItems(20);
    CreateDone();
    GetChildPattern<GridItemPattern>(frameNode_, 1)->SetSelected(true);
    EXPECT_EQ(pattern_->GetVisibleSelectedItems().size(), 1);
    EXPECT_FALSE(pattern_->irregular_);
}

/**
 * @tc.name: AdaptToChildMainSize001
 * @tc.desc: Test AdaptToChildMainSize
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, AdaptToChildMainSize001, TestSize.Level1)
{
    /**
     * @tc.cases: Set ColumnsTemplate, not set grid height
     */
    GridModelNG model;
    RefPtr<ScrollControllerBase> positionController = model.CreatePositionController();
    RefPtr<ScrollProxy> scrollBarProxy = model.CreateScrollBarProxy();
    model.Create(positionController, scrollBarProxy);
    ViewAbstract::SetWidth(CalcLength(WIDTH));
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    model.SetCellLength(ITEM_MAIN_SIZE);
    model.SetMaxCount(4);
    CreateFixedItems(20);
    GetGrid();
    CreateDone();
    EXPECT_EQ(pattern_->GetGridLayoutInfo().lastMainSize_, 500.f);
}

/**
 * @tc.name: AdaptToChildMainSize002
 * @tc.desc: Test AdaptToChildMainSize
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, AdaptToChildMainSize002, TestSize.Level1)
{
    /**
     * @tc.cases: Set RowsTemplate, not set grid width
     */
    GridModelNG model;
    RefPtr<ScrollControllerBase> positionController = model.CreatePositionController();
    RefPtr<ScrollProxy> scrollBarProxy = model.CreateScrollBarProxy();
    model.Create(positionController, scrollBarProxy);
    ViewAbstract::SetHeight(CalcLength(HEIGHT));
    model.SetRowsTemplate("1fr 1fr 1fr 1fr");
    model.SetCellLength(ITEM_MAIN_SIZE);
    model.SetMaxCount(4);
    CreateFixedItems(20);
    GetGrid();
    CreateDone();
    EXPECT_EQ(pattern_->GetGridLayoutInfo().lastMainSize_, 500.f);
}

/**
 * @tc.name: SpringAnimationWithReload
 * @tc.desc: Test SpringAnimationWithReload
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, SpringAnimationWithReload, TestSize.Level1)
{
    MockAnimationManager::GetInstance().Reset();
    MockAnimationManager::GetInstance().SetTicks(5);
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    CreateFixedItems(2);
    CreateDone();

    GestureEvent info;
    info.SetMainVelocity(200.f);
    info.SetMainDelta(200.f);
    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    scrollable->HandleTouchDown();
    scrollable->HandleDragStart(info);
    scrollable->HandleDragUpdate(info);
    FlushUITasks();

    EXPECT_TRUE(pattern_->OutBoundaryCallback());
    scrollable->HandleTouchUp();
    scrollable->HandleDragEnd(info);
    FlushUITasks();
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_GE(pattern_->info_.currentOffset_, 0.0);

    frameNode_->childrenUpdatedFrom_ = 1;
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_GE(pattern_->info_.currentOffset_, 0.0);

    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_GE(pattern_->info_.currentOffset_, 0.0);
}

/**
 * @tc.name: IsAtBottom
 * @tc.desc: Test IsAtBottom
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, IsAtBottom, TestSize.Level1)
{
    MockAnimationManager::GetInstance().Reset();
    MockAnimationManager::GetInstance().SetTicks(3);
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedHeightItems(10, 100);
    CreateFixedHeightItems(1, 0);
    CreateFixedHeightItems(1, 0);
    CreateDone();
    EXPECT_FALSE(pattern_->IsAtBottom());

    // mock the scroll up
    GestureEvent info;
    info.SetMainVelocity(-1000.f);
    info.SetMainDelta(-200.f);
    auto scrollable = pattern_->GetScrollableEvent()->GetScrollable();
    scrollable->HandleTouchDown();
    scrollable->HandleDragStart(info);
    scrollable->HandleDragUpdate(info);
    FlushUITasks();

    scrollable->HandleTouchUp();
    scrollable->HandleDragEnd(info);
    FlushUITasks();
    // start animation
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();

    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();

    MockAnimationManager::GetInstance().Tick();
    FlushUITasks();
    EXPECT_TRUE(MockAnimationManager::GetInstance().AllFinished());
    EXPECT_TRUE(pattern_->IsAtBottom());
    FlushUITasks();
    EXPECT_TRUE(pattern_->IsAtBottom());
}

/**
 * @tc.name: GridScrollLayoutAlgorithmWrapperInitTest
 * @tc.desc: Test wrapper_ is nullptr after default construction
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GridScrollLayoutAlgorithmWrapperInitTest, TestSize.Level1)
{
    GridLayoutInfo info;
    GridScrollLayoutAlgorithm algorithm(info);
    EXPECT_EQ(algorithm.wrapper_, nullptr);
}

/**
 * @tc.name: GridScrollLayoutStartLineNonNegative001
 * @tc.desc: After scroll-to-end then reload-to-start sequences, startMainLineIndex_
 *           must stay non-negative and gridMatrix_ must never contain a -1 row key,
 *           and CheckGridMatrix must pass. Regression guard for the
 *           "check grid matrix failed ... row -1" failure caused by ReloadToStartIndex
 *           decrementing startMainLineIndex_ below 0.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GridScrollLayoutStartLineNonNegative001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    CreateFixedItems(60);
    CreateDone();

    ScrollToIndex(50, true, ScrollAlign::END);
    FlushUITasks();
    ScrollToIndex(0, true, ScrollAlign::START);
    FlushUITasks();
    ScrollToIndex(40, true, ScrollAlign::END);
    FlushUITasks();
    ScrollToIndex(0, true, ScrollAlign::START);
    FlushUITasks();
    UpdateCurrentOffset(-80.0f);
    FlushUITasks();

    auto& info = pattern_->info_;
    EXPECT_GE(info.startMainLineIndex_, 0);
    EXPECT_EQ(info.gridMatrix_.find(-1), info.gridMatrix_.end());
    EXPECT_TRUE(info.CheckGridMatrix(5));
}

/**
 * @tc.name: GridScrollLayoutCheckGridMatrixReload001
 * @tc.desc: When CheckGridMatrix detects matrix inconsistency during layout, the grid
 *           should be marked for data reload (ChildrenUpdatedFrom(0) + MarkDirtyNode).
 *           The actual matrix clear and rebuild happens on the next frame via CheckReset.
 *           After reload, the matrix should be rebuilt and CheckGridMatrix should pass.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GridScrollLayoutCheckGridMatrixReload001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    CreateFixedItems(60);
    CreateDone();

    // Scroll down so startIndex_ > 0, ensuring IsResetted() returns true after matrix clear
    UpdateCurrentOffset(-ITEM_MAIN_SIZE * 6);
    auto& info = pattern_->info_;
    ASSERT_GT(info.startIndex_, 0);
    ASSERT_FALSE(info.gridMatrix_.empty());
    ASSERT_EQ(frameNode_->GetChildrenUpdated(), -1);

    // Corrupt the matrix: add a cache line beyond endMainLineIndex_ with an item index
    // less than endIndex_, which causes CheckGridMatrix to return false.
    const int32_t badLine = info.endMainLineIndex_ + 1;
    info.gridMatrix_[badLine] = { { 0, 0 } };

    // Set times_ so that the next layout tick triggers CheckGridMatrix
    info.times_ = GRID_CHECK_INTERVAL - 1;

    // Flush: Layout runs, CheckGridMatrix fails, ChildrenUpdatedFrom(0) + MarkDirtyNode
    // called, then return. The matrix is NOT cleared yet (clear happens next frame).
    FlushUITasks();

    // After failed check, reload is scheduled: childrenUpdatedFrom_ set to 0
    EXPECT_EQ(frameNode_->GetChildrenUpdated(), 0);

    // Flush again: CheckReset detects updateIdx == 0, clears and rebuilds matrix
    // via ReloadToStartIndex. Normal path end calls ChildrenUpdatedFrom(-1).
    FlushUITasks();

    // After reload, matrix should be rebuilt and consistent
    EXPECT_FALSE(info.gridMatrix_.empty());
    EXPECT_TRUE(info.CheckGridMatrix(1));
}

/**
 * @tc.name: FastScrollPrepareRegularMode2Work
 * @tc.desc: Prepare a distant real Grid target without measuring the intervening lazy items.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareRegularMode2Work, TestSize.Level1)
{
    auto created = std::make_shared<std::vector<int32_t>>();
    auto measured = std::make_shared<std::vector<int32_t>>();
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(120, [created, measured](uint32_t index) {
        created->push_back(static_cast<int32_t>(index));

        auto item = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        item->measureCallback_ = std::make_unique<std::function<void(RefPtr<Kit::FrameNode>&)>>(
            [measured, index](RefPtr<Kit::FrameNode>&) { measured->push_back(static_cast<int32_t>(index)); });
        return 80.0f + static_cast<float>(index % 5) * 15.0f;
    });
    CreateDone();
    ASSERT_FALSE(measured->empty());
    const auto first = pattern_->GetGridLayoutInfo().startIndex_;
    created->clear();
    measured->clear();
    pattern_->ScrollToIndexFastScroll(90, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, first);
    EXPECT_NE(std::find(measured->begin(), measured->end(), 90), measured->end());
    const auto middle = [](int32_t index) { return index >= 24 && index < 84; };
    EXPECT_EQ(std::count_if(measured->begin(), measured->end(), middle), 0);
    EXPECT_EQ(std::count_if(created->begin(), created->end(), middle), 0);
    const auto& prepared = pattern_->GetGridLayoutInfo().fastScrollGeometry_;
    ASSERT_TRUE(prepared.has_value());
    EXPECT_TRUE(prepared->ready);
    EXPECT_FLOAT_EQ(prepared->targetRect.Width(), WIDTH / 3.0f);
    EXPECT_FLOAT_EQ(prepared->targetRect.Height(), 80.0f);
    EXPECT_FLOAT_EQ(prepared->targetRect.Left(), 0.0f);
    EXPECT_FLOAT_EQ(prepared->targetRect.Top() - prepared->targetOffsetPx, 0.0f);
    std::string createdIndices;
    std::string measuredIndices;
    for (auto index : *created) {
        createdIndices += std::to_string(index) + ",";
    }
    for (auto index : *measured) {
        measuredIndices += std::to_string(index) + ",";
    }
    RecordProperty("created_indices", createdIndices);
    RecordProperty("measured_indices", measuredIndices);
    RecordProperty("created_count", static_cast<int>(created->size()));
    RecordProperty("measured_count", static_cast<int>(measured->size()));
}

/**
 * @tc.name: FastScrollPrepareMode1ColdSpanWork
 * @tc.desc: Prepare a distant real Grid target without measuring the intervening lazy items.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareMode1ColdSpanWork, TestSize.Level1)
{
    auto created = std::make_shared<std::vector<int32_t>>();
    auto measured = std::make_shared<std::vector<int32_t>>();
    BuildHiddenSpanGrid(created, measured);
    ASSERT_FALSE(measured->empty());
    const auto first = pattern_->GetGridLayoutInfo().startIndex_;
    created->clear();
    measured->clear();
    pattern_->ScrollToIndexFastScroll(90, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_MEASURE);
    FlushUITasks(frameNode_);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, first);
    EXPECT_NE(std::find(measured->begin(), measured->end(), 90), measured->end());
    const auto middle = [](int32_t index) { return index >= 24 && index < 84; };
    EXPECT_EQ(std::count_if(measured->begin(), measured->end(), middle), 0);

    const auto& prepared = pattern_->GetGridLayoutInfo().fastScrollGeometry_;
    ASSERT_TRUE(prepared.has_value());
    EXPECT_TRUE(prepared->ready);
    EXPECT_FLOAT_EQ(prepared->targetRect.Width(), WIDTH / 3.0f);
    EXPECT_FLOAT_EQ(prepared->targetRect.Height(), 80.0f);
    EXPECT_FLOAT_EQ(prepared->targetRect.Left(), 0.0f);
    EXPECT_FLOAT_EQ(prepared->targetRect.Top() - prepared->targetOffsetPx, 0.0f);
    std::string createdIndices;
    std::string measuredIndices;
    for (auto index : *created) {
        createdIndices += std::to_string(index) + ",";
    }
    for (auto index : *measured) {
        measuredIndices += std::to_string(index) + ",";
    }
    RecordProperty("created_indices", createdIndices);
    RecordProperty("measured_indices", measuredIndices);
    RecordProperty("created_count", static_cast<int>(created->size()));
    RecordProperty("measured_count", static_cast<int>(measured->size()));
}

/**
 * @tc.name: FastScrollPrepareMeasuredGeometry
 * @tc.desc: Measured geometry honors alignment and signed offsets without moving the current viewport.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareMeasuredGeometry, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(0);
    CreateGridItems(120, FILL_VALUE, 100.0f);
    CreateDone();
    const auto firstRect = pattern_->GetItemRect(0);
    for (auto align : { ScrollAlign::START, ScrollAlign::CENTER, ScrollAlign::END }) {
        for (float offset : { -550.0f, -25.0f, 0.0f, 25.0f, 550.0f }) {
            pattern_->ScrollToIndexFastScroll(90, true, align, offset, FastScrollMode::SKIP_CREATE_AND_MEASURE);
            FlushUITasks(frameNode_);
            const auto& prepared = pattern_->GetGridLayoutInfo().fastScrollGeometry_;
            ASSERT_TRUE(prepared.has_value());
            ASSERT_TRUE(prepared->ready);
            const float adjustment = align == ScrollAlign::START ? 0.0f :
                (align == ScrollAlign::CENTER ? (HEIGHT - 100.0f) / 2.0f : HEIGHT - 100.0f);
            EXPECT_FLOAT_EQ(prepared->targetRect.Top(), 3000.0f);
            EXPECT_FLOAT_EQ(prepared->targetRect.Height(), 100.0f);
            EXPECT_FLOAT_EQ(prepared->targetOffsetPx, 3000.0f - adjustment + offset);
            EXPECT_EQ(pattern_->GetItemRect(0).ToString(), firstRect.ToString());
        }
    }
}

/**
 * @tc.name: FastScrollPrepareLatestAndCancel
 * @tc.desc: Before first layout only the latest request is prepared; stop and detach clear it.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareLatestAndCancel, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(0);
    CreateGridItems(120, FILL_VALUE, 100.0f);
    pattern_->ScrollToIndexFastScroll(90, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_MEASURE);
    pattern_->ScrollToIndexFastScroll(33, true, ScrollAlign::NONE, 0.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    ASSERT_TRUE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
    EXPECT_EQ(pattern_->GetGridLayoutInfo().fastScrollTarget_->index, 33);
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    CreateDone();
    ASSERT_TRUE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    EXPECT_FLOAT_EQ(pattern_->GetGridLayoutInfo().fastScrollGeometry_->targetRect.Top(), 1100.0f);
    pattern_->StopAnimate();
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    pattern_->ScrollToIndexFastScroll(90, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_MEASURE);
    pattern_->OnDetachFromMainTree();
    FlushUITasks(frameNode_);
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
}

/**
 * @tc.name: FastScrollPrepareZeroViewport
 * @tc.desc: A zero-size viewport cannot publish a ready result; a later real layout can prepare it.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareZeroViewport, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(0);
    ViewAbstract::SetHeight(CalcLength(0.0f));
    CreateGridItems(120, FILL_VALUE, 100.0f);
    pattern_->ScrollToIndexFastScroll(90, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_CREATE_AND_MEASURE);
    CreateDone();
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    layoutProperty_->UpdateUserDefinedIdealSize(CalcSize(CalcLength(WIDTH), CalcLength(HEIGHT)));
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(frameNode_);
    ASSERT_TRUE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    EXPECT_TRUE(pattern_->GetGridLayoutInfo().fastScrollGeometry_->ready);
}

/**
 * @tc.name: FastScrollPrepareHorizontalRtl
 * @tc.desc: Horizontal preparation uses the layout's gap, measured size and RTL coordinates.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareHorizontalRtl, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetRowsTemplate("1fr 1fr");
    model.SetColumnsGap(Dimension(10.0f));
    model.SetCachedCount(0);
    CreateGridItems(120, 100.0f, 100.0f);
    CreateDone();
    const float expectedCrossSize = pattern_->GetItemRect(0).Height();
    ASSERT_GT(expectedCrossSize, 0.0f);
    for (auto direction : { TextDirection::LTR, TextDirection::RTL }) {
        layoutProperty_->UpdateLayoutDirection(direction);
        pattern_->ScrollToIndexFastScroll(90, true, ScrollAlign::CENTER, 25.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
        FlushUITasks(frameNode_);
        const auto& prepared = pattern_->GetGridLayoutInfo().fastScrollGeometry_;
        ASSERT_TRUE(prepared.has_value());
        ASSERT_TRUE(prepared->ready);
        EXPECT_FLOAT_EQ(prepared->targetRect.Width(), 100.0f);
        EXPECT_FLOAT_EQ(prepared->targetRect.Height(), expectedCrossSize);
        EXPECT_FLOAT_EQ(prepared->targetOffsetPx, 4950.0f - (WIDTH - 100.0f) / 2.0f + 25.0f);
        const float viewportX = direction == TextDirection::LTR
            ? prepared->targetRect.Left() - prepared->targetOffsetPx
            : prepared->targetRect.Left() + prepared->targetOffsetPx;
        EXPECT_FLOAT_EQ(viewportX, (WIDTH - 100.0f) / 2.0f + (direction == TextDirection::LTR ? -25.0f : 25.0f));
        EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, 0);
    }
}

/**
 * @tc.name: FastScrollPrepareReverseColdRebuild
 * @tc.desc: Real LazyForEach eviction can rebuild a reverse target without creating the middle path.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareReverseColdRebuild, TestSize.Level1)
{
    auto created = std::make_shared<std::vector<int32_t>>();
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(120, [created](uint32_t index) {
        created->push_back(static_cast<int32_t>(index));
        return 100.0f;
    });
    CreateDone();
    ScrollToIndex(18, false, ScrollAlign::START);
    auto lazy = AceType::DynamicCast<LazyForEachNode>(frameNode_->GetChildren().front());
    ASSERT_NE(lazy, nullptr);
    auto oldTarget = lazy->GetFrameChildByIndex(18, false);
    ASSERT_NE(oldTarget, nullptr);
    WeakPtr<UINode> oldNode(oldTarget);
    oldTarget.Reset();
    ScrollToIndex(90, false, ScrollAlign::START);
    lazy->CleanCache(true);
    ASSERT_EQ(lazy->GetFrameChildByIndex(18, false), nullptr);
    created->clear();
    pattern_->ScrollToIndexFastScroll(18, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    ASSERT_TRUE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    EXPECT_NE(lazy->GetFrameChildByIndex(18, false), oldNode.Upgrade());
    EXPECT_EQ(std::count(created->begin(), created->end(), 18), 1);
    EXPECT_EQ(std::count_if(created->begin(), created->end(), [](int32_t i) { return i >= 36 && i < 84; }), 0);
    EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, 90);
    EXPECT_FLOAT_EQ(pattern_->GetGridLayoutInfo().fastScrollGeometry_->targetRect.Top(), 600.0f);
}

/**
 * @tc.name: FastScrollPrepareInvalidAndLegacy
 * @tc.desc: Invalid input and ordinary calls cannot leave a pending fast preparation behind.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareInvalidAndLegacy, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    CreateGridItems(120, FILL_VALUE, 100.0f);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(90, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_MEASURE);
    pattern_->ScrollToIndexFastScroll(10, false, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_MEASURE);
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
    pattern_->ScrollToIndexFastScroll(-9, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_MEASURE);
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
    pattern_->ScrollToIndexFastScroll(999, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_MEASURE);
    FlushUITasks(frameNode_);
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    pattern_->ScrollToIndexFastScroll(LAST_ITEM, true, ScrollAlign::END, 0.0f, FastScrollMode::SKIP_MEASURE);
    FlushUITasks(frameNode_);
    ASSERT_TRUE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    EXPECT_TRUE(pattern_->GetGridLayoutInfo().fastScrollGeometry_->ready);
    pattern_->ScrollToIndex(0, false, ScrollAlign::START);
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
}

/**
 * @tc.name: FastScrollPrepareAutoAndKnownHeights
 * @tc.desc: AUTO keeps a visible target in place and unknown gaps do not replace known row heights.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareAutoAndKnownHeights, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(120, [](uint32_t index) {
        return index < 3 ? 300.0f : (index < 6 ? 200.0f : 100.0f);
    });
    CreateDone();
    const auto prepare = [this](int32_t index, ScrollAlign align) {
        auto info = pattern_->GetGridLayoutInfo();
        info.fastScrollTarget_ = GridFastScrollTarget { index, align, 0.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE };
        auto algorithm = AceType::MakeRefPtr<GridScrollLayoutAlgorithm>(std::move(info));
        algorithm->Measure(AceType::RawPtr(frameNode_));
        return algorithm->GetGridLayoutInfo().fastScrollGeometry_;
    };
    const auto visible = prepare(0, ScrollAlign::AUTO);
    ASSERT_TRUE(visible.has_value());
    EXPECT_FLOAT_EQ(visible->targetOffsetPx, 0.0f);
    const auto start = prepare(90, ScrollAlign::START);
    ASSERT_TRUE(start.has_value());
    // Both offsets use the current viewport's 250px unknown-row estimate (measured rows are 300px and 200px).
    // These are internal estimated coordinates, not an assertion about the eventual physical content offset.
    EXPECT_FLOAT_EQ(start->targetRect.Top(), 7500.0f);
    EXPECT_FLOAT_EQ(start->targetOffsetPx, 7500.0f);
    EXPECT_FLOAT_EQ(start->targetRect.Top() - start->targetOffsetPx, 0.0f);
    EXPECT_FLOAT_EQ(start->targetRect.Height(), 100.0f);
    const auto automatic = prepare(90, ScrollAlign::AUTO);
    ASSERT_TRUE(automatic.has_value());
    ASSERT_TRUE(automatic.has_value());
    EXPECT_FLOAT_EQ(automatic->targetOffsetPx, 6750.0f);
    EXPECT_FLOAT_EQ(automatic->targetRect.Top() - automatic->targetOffsetPx, 300.0f);
    EXPECT_FLOAT_EQ(pattern_->GetGridLayoutInfo().lineHeightMap_.at(0), 300.0f);
    EXPECT_FLOAT_EQ(pattern_->GetGridLayoutInfo().lineHeightMap_.at(1), 200.0f);
}

/**
 * @tc.name: FastScrollPrepareSpanningTargetOracle
 * @tc.desc: A spanning target's measured rectangle agrees with ordinary sequential layout.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareSpanningTargetOracle, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(120, [](uint32_t index) {
        if (index == 39) {
            GridItemModelNG item;
            item.SetColumnStart(0);
            item.SetColumnEnd(1);
            item.SetRowStart(0);
            item.SetRowEnd(1);
            return 180.0f;
        }
        return 100.0f;
    });
    CreateDone();
    for (int32_t i = 1; i <= 39; ++i) {
        ScrollToIndex(i, false, ScrollAlign::START);
    }
    const auto reference = pattern_->GetItemRect(39);
    ASSERT_GT(reference.Width(), 0.0f);
    ScrollToIndex(0, false, ScrollAlign::START);
    for (auto mode : { FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE }) {
        pattern_->ScrollToIndexFastScroll(39, true, ScrollAlign::START, 0.0f, mode);
        FlushUITasks(frameNode_);
        const auto& result = pattern_->GetGridLayoutInfo().fastScrollGeometry_;
        ASSERT_TRUE(result.has_value());
        EXPECT_FLOAT_EQ(result->targetRect.Left(), reference.Left());
        EXPECT_FLOAT_EQ(result->targetRect.Top() - result->targetOffsetPx, reference.Top());
        EXPECT_FLOAT_EQ(result->targetRect.Width(), reference.Width());
        EXPECT_FLOAT_EQ(result->targetRect.Height(), reference.Height());
    }
}

/**
 * @tc.name: FastScrollPrepareSingleCellMode1Oracle
 * @tc.desc: Cold one-cell items with adjacent explicit columns must agree with physical scrolling.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareSingleCellMode1Oracle, TestSize.Level1)
{
    Rect reference;
    for (bool normal : { true, false }) {
        auto created = std::make_shared<std::vector<int32_t>>();
        auto model = CreateGrid();
        model.SetColumnsTemplate("1fr 1fr 1fr");
        model.SetCachedCount(0);
        CreateItemsInLazyForEach(120, [created](uint32_t index) {
            created->push_back(static_cast<int32_t>(index));
            if (index == 39 || index == 40) {
                GridItemModelNG item;
                item.SetColumnStart(2);
                item.SetColumnEnd(2);
            }
            return 100.0f;
        });
        CreateDone();
        EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, 0);
        EXPECT_EQ(std::count(created->begin(), created->end(), 39), 0);
        EXPECT_EQ(std::count(created->begin(), created->end(), 40), 0);
        RecordProperty(normal ? "normal_first_rect" : "fast_first_rect", pattern_->GetItemRect(0).ToString());
        if (normal) {
            ReadSingleCellPhysicalReference(reference);
            ASSERT_FALSE(HasFatalFailure());
            TearDown();
            SetUp();
            continue;
        }
        ASSERT_EQ(std::count(created->begin(), created->end(), 39), 0);
        ASSERT_EQ(std::count(created->begin(), created->end(), 40), 0);
        created->clear();
        pattern_->ScrollToIndexFastScroll(90, true, ScrollAlign::START, 0.0f,
            FastScrollMode::SKIP_MEASURE);
        FlushUITasks(frameNode_);
        const auto& prepared = pattern_->GetGridLayoutInfo().fastScrollGeometry_;
        ASSERT_TRUE(prepared.has_value());
        EXPECT_TRUE(prepared->ready);
        EXPECT_FLOAT_EQ(prepared->targetRect.Left(), reference.Left());
        EXPECT_FLOAT_EQ(prepared->targetRect.Top() - prepared->targetOffsetPx, reference.Top());
        EXPECT_FLOAT_EQ(prepared->targetRect.Width(), reference.Width());
        EXPECT_FLOAT_EQ(prepared->targetRect.Height(), reference.Height());
        EXPECT_EQ(std::count(created->begin(), created->end(), 39), 1);
        EXPECT_EQ(std::count(created->begin(), created->end(), 40), 1);
        RecordProperty("prepared_rect", prepared->targetRect.ToString());
        RecordProperty("prepared_offset", prepared->targetOffsetPx);
    }
}

// Create a single-column grid whose first item is zero-height and the rest are ITEM_MAIN_SIZE.
// Mimics CreateFixedItems: per-item CreateGridItem + Pop + StopGetAccessRecording.
void GridScrollLayoutTestNg::CreateZeroHeightFirstLineGrid(
    GridModelNG& model, std::vector<std::pair<int32_t, int32_t>>* reported)
{
    if (reported) {
        model.SetOnScrollIndex([reported](int32_t first, int32_t last) {
            reported->emplace_back(first, last);
        });
    }
    CreateGridItem(FILL_VALUE, 0.0f);
    ViewStackProcessor::GetInstance()->Pop();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    CreateFixedHeightItems(FIXED_HEIGHT_ITEM_NUMBER, ITEM_MAIN_SIZE);
    CreateDone();
}

/**
 * @tc.name: GridScrollLayoutReportStartZeroHeightLine001
 * @tc.desc: A zero-height first line sits at the content-area top (currentOffset_ == 0, bottom edge 0).
 *           SyncReportRange's report-start walk must not treat it as scrolled-out: reportStartIndex_
 *           stays 0 and onScrollIndex never reports 1, matching WaterFlow (IsReportStartIndexCandidate)
 *           and List (GetStartIndex) semantics.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GridScrollLayoutReportStartZeroHeightLine001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1");
    std::vector<std::pair<int32_t, int32_t>> reported;
    CreateZeroHeightFirstLineGrid(model, &reported);
    FlushUITasks(frameNode_);

    auto& info = pattern_->info_;
    ASSERT_EQ(info.startMainLineIndex_, 0);
    ASSERT_NE(info.lineHeightMap_.find(0), info.lineHeightMap_.end());
    EXPECT_EQ(info.lineHeightMap_[0], 0.0f) << "precondition: line 0 is the zero-height item";
    EXPECT_EQ(info.reportStartIndex_, 0) << "zero-height first line at content top must stay the report start";
    EXPECT_GE(info.reportStartIndex_, info.startIndex_) << "report range stays within the active range";
    for (const auto& [first, last] : reported) {
        EXPECT_EQ(first, 0) << "onScrollIndex must not report startIndex 1 for a zero-height first item";
    }
}

/**
 * @tc.name: FastScrollPreparePaddingAndCache
 * @tc.desc: Preparation respects real padding and includes the displayed cache in its range envelope.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPreparePaddingAndCache, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(2, true);
    ViewAbstract::SetPadding(CalcLength(10.0f));
    CreateGridItems(120, FILL_VALUE, 100.0f);
    CreateDone();
    const auto source = pattern_->GetItemRect(0);
    pattern_->ScrollToIndexFastScroll(90, true, ScrollAlign::CENTER, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    const auto& geometry = pattern_->GetGridLayoutInfo().fastScrollGeometry_;
    ASSERT_TRUE(geometry.has_value());
    EXPECT_FLOAT_EQ(geometry->targetRect.Left(), source.Left());
    EXPECT_FLOAT_EQ(geometry->targetRect.Width(), source.Width());
    EXPECT_FLOAT_EQ(geometry->targetRect.Top(), 3010.0f);
    EXPECT_FLOAT_EQ(geometry->targetOffsetPx, 3000.0f - (HEIGHT - 20.0f - 100.0f) / 2.0f);
    EXPECT_EQ(pattern_->GetItemRect(0).ToString(), source.ToString());
    EXPECT_LE(geometry->firstPreparedIndex, 0);
    EXPECT_GE(geometry->lastPreparedIndex, 90);
}

/**
 * @tc.name: FastScrollPrepareReplaceDuringBuild
 * @tc.desc: A new request issued while a target is built cannot publish the old target's result.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareReplaceDuringBuild, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(0);
    auto weak = AceType::WeakClaim(AceType::RawPtr(pattern_));
    auto replaced = std::make_shared<bool>(false);
    CreateItemsInLazyForEach(120, [weak, replaced](uint32_t index) {
        if (index == 90 && !*replaced) {
            *replaced = true;
            auto pattern = weak.Upgrade();
            if (pattern) {
                pattern->ScrollToIndexFastScroll(18, true, ScrollAlign::START, 0.0f,
                    FastScrollMode::SKIP_CREATE_AND_MEASURE);
            }
        }
        return 100.0f;
    });
    CreateDone();
    pattern_->ScrollToIndexFastScroll(90, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    FlushUITasks(frameNode_);
    ASSERT_TRUE(*replaced);
    ASSERT_TRUE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
    EXPECT_EQ(pattern_->GetGridLayoutInfo().fastScrollTarget_->index, 18);
    ASSERT_TRUE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    EXPECT_FLOAT_EQ(pattern_->GetGridLayoutInfo().fastScrollGeometry_->targetRect.Top(), 600.0f);
}

/**
 * @tc.name: FastScrollPrepareRepeatWork
 * @tc.desc: Repeat's actual node creation and measurement also skip the middle path in mode2.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareRepeatWork, TestSize.Level1)
{
    auto created = std::make_shared<std::vector<int32_t>>();
    auto measured = std::make_shared<std::vector<int32_t>>();
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(0);
    // The fixture and its nodes outlive these Repeat callbacks.
    auto build = [this, created, measured](uint32_t index, bool update) {
        CreateGridItem(FILL_VALUE, 100.0f);
        created->push_back(static_cast<int32_t>(index));
        auto child = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        child->measureCallback_ = std::make_unique<std::function<void(RefPtr<Kit::FrameNode>&)>>(
            [index, measured](RefPtr<Kit::FrameNode>&) { measured->push_back(static_cast<int32_t>(index)); });
        if (update) {
            ViewStackProcessor::GetInstance()->Finish();
        } else {
            ViewStackProcessor::GetInstance()->Pop();
        }
    };
    RepeatVirtualScrollModelNG repeat;
    repeat.Create(120, {}, [build](uint32_t index) { build(index, false); },
        [build](const std::string&, uint32_t index) { build(index, true); },
        [](uint32_t start, uint32_t end) {
            std::list<std::string> keys;
            for (auto i = start; i <= end; ++i) {
                keys.push_back(std::to_string(i));
            }
            return keys;
        },
        [](uint32_t start, uint32_t end) { return std::list<std::string>(end - start + 1, "0"); },
        [](uint32_t, uint32_t) {});
    CreateDone();
    ASSERT_FALSE(measured->empty());
    created->clear();
    measured->clear();
    pattern_->ScrollToIndexFastScroll(90, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    ASSERT_TRUE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    EXPECT_FLOAT_EQ(pattern_->GetGridLayoutInfo().fastScrollGeometry_->targetRect.Top(), 3000.0f);
    EXPECT_NE(std::find(measured->begin(), measured->end(), 90), measured->end());
    const auto middle = [](int32_t index) { return index >= 24 && index < 84; };
    EXPECT_EQ(std::count_if(created->begin(), created->end(), middle), 0);
    EXPECT_EQ(std::count_if(measured->begin(), measured->end(), middle), 0);
}

/**
 * @tc.name: FastScrollPrepareRelocationLayout
 * @tc.desc: A real relocation layout fills the target and visible cache without walking the middle items.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareRelocationLayout, TestSize.Level1)
{
    auto created = std::make_shared<std::vector<int32_t>>();
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(1, true);
    CreateItemsInLazyForEach(120, [created](uint32_t index) {
        created->push_back(static_cast<int32_t>(index));
        return 100.0f;
    });
    CreateDone();
    created->clear();
    auto algorithm = AceType::MakeRefPtr<GridScrollLayoutAlgorithm>(pattern_->GetGridLayoutInfo());
    algorithm->SetFastScrollTargetForLayout({ 90, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE });
    algorithm->Measure(AceType::RawPtr(frameNode_));
    algorithm->Layout(AceType::RawPtr(frameNode_));
    EXPECT_EQ(algorithm->GetGridLayoutInfo().startIndex_, 90);
    auto target = frameNode_->GetChildByIndex(90, true);
    ASSERT_NE(target, nullptr);
    EXPECT_FLOAT_EQ(target->GetGeometryNode()->GetMarginFrameOffset().GetY(), 0.0f);
    EXPECT_GT(target->GetGeometryNode()->GetMarginFrameSize().Width(), 0.0f);
    EXPECT_NE(frameNode_->GetChildByIndex(102, true), nullptr);
    EXPECT_EQ(std::count_if(created->begin(), created->end(), [](int32_t i) { return i >= 24 && i < 84; }), 0);
    algorithm->info_.currentOffset_ -= 50.0f;
    algorithm->Measure(AceType::RawPtr(frameNode_));
    algorithm->Layout(AceType::RawPtr(frameNode_));
    EXPECT_FLOAT_EQ(target->GetGeometryNode()->GetMarginFrameOffset().GetY(), -50.0f);
}

/**
 * @tc.name: FastScrollPrepareMarginFrameOracle
 * @tc.desc: Handoff reports the actual item frame, including its margin placement, rather than the margin box.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareMarginFrameOracle, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(0);
    for (int32_t i = 0; i < 120; ++i) {
        CreateGridItem(FILL_VALUE, 100.0f);
        ViewAbstract::SetMargin(CalcLength(10.0f));
        ViewStackProcessor::GetInstance()->Pop();
    }
    CreateDone();
    ScrollToIndex(90, false, ScrollAlign::START);
    const auto reference = pattern_->GetItemRect(90);
    ASSERT_GT(reference.Width(), 0.0f);
    ScrollToIndex(0, false, ScrollAlign::START);
    pattern_->ScrollToIndexFastScroll(90, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    const auto& prepared = pattern_->GetGridLayoutInfo().fastScrollGeometry_;
    ASSERT_TRUE(prepared.has_value());
    EXPECT_FLOAT_EQ(prepared->targetRect.Left(), reference.Left());
    EXPECT_FLOAT_EQ(prepared->targetRect.Top() - prepared->targetOffsetPx, reference.Top());
    EXPECT_FLOAT_EQ(prepared->targetRect.Width(), reference.Width());
    EXPECT_FLOAT_EQ(prepared->targetRect.Height(), reference.Height());
}

/**
 * @tc.name: FastScrollPrepareNonScrollableLayout
 * @tc.desc: A request does not survive a switch to a non-scrollable Grid layout.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, FastScrollPrepareNonScrollableLayout, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    CreateGridItems(30, FILL_VALUE, 100.0f);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(20, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_MEASURE);
    ASSERT_TRUE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
    layoutProperty_->UpdateRowsTemplate("1fr 1fr");
    auto staticLayout = pattern_->CreateLayoutAlgorithm();
    ASSERT_NE(AceType::DynamicCast<GridLayoutAlgorithm>(staticLayout), nullptr);
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
}

/**
 * @tc.name: GridScrollLayoutReportStartZeroHeightLine002
 * @tc.desc: Scrolled-past semantics keep working: once the zero-height line 0 moves above the content-area
 *           top it is skipped (reportStartIndex_ 1). After scrolling back to the top the line fills no blank
 *           above the anchor, so the active range keeps starting at line 1 — the report range must stay
 *           within the active range (reportStartIndex_ >= startIndex_ = 1), matching the pre-contentClip
 *           baseline that reported startIndex_ directly.
 * @tc.type: FUNC
 */
HWTEST_F(GridScrollLayoutTestNg, GridScrollLayoutReportStartZeroHeightLine002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1");
    CreateZeroHeightFirstLineGrid(model);
    FlushUITasks(frameNode_);
    ASSERT_EQ(pattern_->info_.reportStartIndex_, 0);
    EXPECT_GE(pattern_->info_.reportStartIndex_, pattern_->info_.startIndex_);

    UpdateCurrentOffset(-SCROLL_PAST_ZERO_HEIGHT_LINE); // line 0 (height 0) now above the content top
    FlushUITasks(frameNode_);
    EXPECT_EQ(pattern_->info_.reportStartIndex_, 1);

    UpdateCurrentOffset(SCROLL_PAST_ZERO_HEIGHT_LINE); // back to the top
    FlushUITasks(frameNode_);
    EXPECT_EQ(pattern_->info_.reportStartIndex_, 1);
    EXPECT_GE(pattern_->info_.reportStartIndex_, pattern_->info_.startIndex_);
}
} // namespace OHOS::Ace::NG
