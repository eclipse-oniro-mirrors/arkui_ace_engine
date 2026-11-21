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
#include "grid_test_ng.h"

#include <iomanip>
#include <limits>
#include <set>
#include <sstream>
#include <tuple>

#include "test/mock/frameworks/core/animation/mock_animation_manager.h"

#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#include "test/unittest/core/pattern/scrollable/scrollable_utils_test_ng.h"

#include "core/components_ng/base/simplified_inspector.h"
#include "core/components_ng/pattern/grid/grid_custom/grid_custom_layout_algorithm.h"
#include "core/components_ng/pattern/grid/grid_scroll/grid_scroll_with_options_layout_algorithm.h"
#include "core/components_ng/pattern/grid/irregular/grid_irregular_layout_algorithm.h"
#include "core/components_ng/pattern/grid/grid_utils.h"
#include "core/components_ng/pattern/grid/grid_scroll/grid_scroll_layout_algorithm.h"
#include "core/components_ng/pattern/scrollable/scrollable_model_ng.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t ANIMATION_ITEM_COUNT = 150;
constexpr int32_t ANIMATION_TARGET_INDEX = 120;
constexpr int32_t ANIMATION_TICKS = 8;
constexpr int32_t MAX_ANIMATION_TICKS = 64;
constexpr int32_t CUSTOM_GROUP_ITEMS = 3;
constexpr int32_t CUSTOM_GROUP_ROWS = 2;
constexpr int32_t COMPLETION_ROUNDS_WITH_WARMUP = 23;
constexpr int32_t CANCEL_PHASE_COUNT = 3;
constexpr int32_t CANCEL_TAIL_TICKS = 4;
constexpr int32_t LEGACY_TARGET_INDEX = 5;

class ScopedFastScrollAnimationClock final {
public:
    ScopedFastScrollAnimationClock() : previous_(MockAnimationManager::GetVersion())
    {
        MockAnimationManager::SetVersion(MockAnimationManager::Version::V2);
    }
    ~ScopedFastScrollAnimationClock()
    {
        MockAnimationManager::SetVersion(previous_);
    }

private:
    MockAnimationManager::Version previous_;
    ACE_DISALLOW_COPY_AND_MOVE(ScopedFastScrollAnimationClock);
};

class ScopedFastScrollOptionsAlgorithm final {
public:
    ScopedFastScrollOptionsAlgorithm() : previous_(SystemProperties::gridIrregularLayoutEnable_)
    {
        SystemProperties::gridIrregularLayoutEnable_ = false;
    }
    ~ScopedFastScrollOptionsAlgorithm()
    {
        SystemProperties::gridIrregularLayoutEnable_ = previous_;
    }

private:
    bool previous_;
    ACE_DISALLOW_COPY_AND_MOVE(ScopedFastScrollOptionsAlgorithm);
};
} // namespace

struct FastScrollAnimationObservation {
    std::vector<float> positions;
    std::vector<std::pair<size_t, size_t>> workCounts;
    std::string ranges;
    int32_t headEnd = 0;
    int32_t tailStart = ANIMATION_ITEM_COUNT;
    bool hasSkipped = false;
    float headOffset = 0.0f;
};

class GridPatternTestNg : public GridTestNg {
protected:
    void RecordWorkObservation(int32_t mode, const std::vector<int32_t>& built,
        const std::vector<int32_t>& measured, const std::vector<float>& offsets,
        const std::vector<std::pair<size_t, size_t>>& workCounts, int32_t target)
    {
        const auto indices = [](const std::vector<int32_t>& values) {
            std::string result;
            for (auto value : values) { result += std::to_string(value) + ","; }
            return result;
        };
        std::string positions;
        for (auto value : offsets) { positions += std::to_string(value) + ","; }
        std::string counts;
        for (const auto& value : workCounts) {
            counts += std::to_string(value.first) + ":" + std::to_string(value.second) + ",";
        }
        const auto rectangle = pattern_->GetItemRect(target);
        RecordProperty("observation_scope", "host_grid_fixture");
        RecordProperty("creation_source",
            "GridMockLazy callback after GridItemModelNG::Create; pool reuse not separated");
        RecordProperty("measurement_source", "FrameNode measureCallback after real measure completion");
        RecordProperty("display_frames_observed", "false");
        RecordProperty("mode", mode);
        RecordProperty("target_index", target);
        RecordProperty("viewport_px", std::to_string(pattern_->GetMainContentSize()));
        RecordProperty("build_callback_indices", indices(built));
        RecordProperty("actual_measure_indices", indices(measured));
        RecordProperty("host_layout_offsets", positions);
        RecordProperty("work_count_prefixes", counts);
        RecordProperty("target_rect", std::to_string(rectangle.Left()) + "," + std::to_string(rectangle.Top()) + "," +
            std::to_string(rectangle.Width()) + "," + std::to_string(rectangle.Height()));
        RecordProperty("request_cleared", pattern_->GetFastScrollRequest() ? "false" : "true");
    }

protected:
    void CheckLongAnimationJumps(const std::vector<float>& positions, float viewport)
    {
        int32_t jumps = 0;
        for (size_t i = 1; i < positions.size(); ++i) {
            const float delta = positions[i] - positions[i - 1];
            EXPECT_GT(delta, 0.0f);
            if (delta > viewport) {
                ++jumps;
                EXPECT_GT(i, 1);
                EXPECT_LT(i, positions.size() - 1);
                EXPECT_GE(positions[i - 1], 1.5f * viewport);
                EXPECT_NEAR(12000.0f - positions[i], 2.0f * viewport, 0.1f);
            }
        }
        EXPECT_EQ(jumps, 1);
    }
    FastScrollAnimationObservation ObserveTopologyAnimation(
        const std::shared_ptr<std::vector<int32_t>>& created,
        const std::shared_ptr<std::vector<int32_t>>& measured)
    {
        FastScrollAnimationObservation observation;
        observation.positions.push_back(pattern_->GetTotalOffset());
        observation.workCounts.emplace_back(created->size(), measured->size());
        observation.headEnd = pattern_->GetGridLayoutInfo().endIndex_;
        for (int32_t tick = 0; tick < MAX_ANIMATION_TICKS; ++tick) {
            MockAnimationManager::GetInstance().Tick();
            const bool skipped = pattern_->fastScrollSkip_;
            FlushUITasks(frameNode_);
            observation.hasSkipped = observation.hasSkipped || skipped;
            if (observation.hasSkipped) {
                observation.tailStart = std::min(observation.tailStart, pattern_->GetGridLayoutInfo().startIndex_);
            } else {
                observation.headEnd = std::max(observation.headEnd, pattern_->GetGridLayoutInfo().endIndex_);
                observation.headOffset = pattern_->GetTotalOffset();
            }
            observation.ranges += std::to_string(tick) + ":" +
                std::to_string(pattern_->GetGridLayoutInfo().startIndex_) +
                "-" + std::to_string(pattern_->GetGridLayoutInfo().endIndex_) + (skipped ? "s;" : ";");
            observation.positions.push_back(pattern_->GetTotalOffset());
            observation.workCounts.emplace_back(created->size(), measured->size());
            if (!pattern_->AnimateRunning() && !pattern_->GetFastScrollRequest()) {
                break;
            }
        }
        return observation;
    }
    void RecordTopologyAnimation(const FastScrollAnimationObservation& observation,
        const std::shared_ptr<std::vector<int32_t>>& created,
        const std::shared_ptr<std::vector<int32_t>>& measured)
    {
        std::string frames;
        for (auto position : observation.positions) {
            frames += std::to_string(position) + ",";
        }
        RecordProperty("actual_frame_offsets", frames);
        RecordProperty("visible_ranges", observation.ranges);
        RecordProperty("head_end", observation.headEnd);
        RecordProperty("tail_start", observation.tailStart);
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
        RecordWorkObservation(static_cast<int32_t>(FastScrollMode::SKIP_CREATE_AND_MEASURE),
            *created, *measured, observation.positions, observation.workCounts, ANIMATION_TARGET_INDEX);
    }
    void ConfigureCustomAnimationOptions(GridModelNG& model)
    {
        GridLayoutOptions options;
        for (int32_t index = 0; index < ANIMATION_ITEM_COUNT; index += CUSTOM_GROUP_ITEMS) {
            options.irregularIndexes.insert(index);
        }
        options.getSizeByIndex = [](int32_t) { return GridItemSize { CUSTOM_GROUP_ROWS, CUSTOM_GROUP_ROWS }; };
        options.getStartIndexByIndex = [](int32_t index) {
            const int32_t group = index / CUSTOM_GROUP_ITEMS;
            const float offset = index % CUSTOM_GROUP_ITEMS == CUSTOM_GROUP_ROWS ? -100.0f : 0.0f;
            return GridStartLineInfo { group * CUSTOM_GROUP_ITEMS, group * CUSTOM_GROUP_ROWS,
                offset, group * 200.0f - offset };
        };
        options.getStartIndexByOffset = [](float offset) {
            const int32_t group = std::max(0, static_cast<int32_t>(offset / 200.0f));
            return GridStartLineInfo { group * CUSTOM_GROUP_ITEMS, group * CUSTOM_GROUP_ROWS,
                group * 200.0f - offset, offset };
        };
        model.SetLayoutOptions(options);
    }
    void AdvanceFastScrollTicks(int32_t count)
    {
        for (int32_t tick = 0; tick < count; ++tick) {
            MockAnimationManager::GetInstance().Tick();
            FlushUITasks(frameNode_);
        }
    }
    void FinishFastScrollAnimation()
    {
        for (int32_t tick = 0; tick < MAX_ANIMATION_TICKS; ++tick) {
            if (!pattern_->AnimateRunning() && !pattern_->GetFastScrollRequest()) {
                break;
            }
            MockAnimationManager::GetInstance().Tick();
            FlushUITasks(frameNode_);
        }
    }
    void BuildIndependentGrid()
    {
        ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(GetElmtId());
        GridModelNG otherModel;
        otherModel.Create(otherModel.CreatePositionController(), otherModel.CreateScrollBarProxy());
        ViewAbstract::SetWidth(CalcLength(WIDTH));
        ViewAbstract::SetHeight(CalcLength(HEIGHT));
        otherModel.SetColumnsTemplate("1fr");
        GetGrid();
        CreateFixedHeightItems(ANIMATION_ITEM_COUNT, 100.0f);
        CreateDone();
    }
    void PhysicalScrollTo(float offset)
    {
        ScrollToIndex(0, false, ScrollAlign::START);
        for (float position = 0.0f; position < offset;) {
            const float step = std::min(40.0f, offset - position);
            pattern_->ScrollBy(step);
            FlushUITasks(frameNode_);
            position += step;
        }
    }
    struct CatalogOffsetCase {
        ScrollAlign align;
        int32_t target;
        int32_t start;
        float factor;
    };
    float CatalogExpectedOffset(const std::vector<float>& itemTops, const CatalogOffsetCase& input) const
    {
        const auto [align, target, start, factor] = input;
        const float viewport = pattern_->GetMainContentSize();
        const float itemHeight = itemTops[target + 1] - itemTops[target];
        float anchor = itemTops[target];
        if (align == ScrollAlign::CENTER) {
            anchor -= (viewport - itemHeight) / 2.0f;
        } else if (align == ScrollAlign::END) {
            anchor -= viewport - itemHeight;
        } else if (align == ScrollAlign::AUTO) {
            const float top = itemTops[target] - itemTops[start];
            const float bottom = top + itemHeight;
            if ((top >= 0.0f && bottom <= viewport) || (top <= 0.0f && bottom >= viewport)) {
                anchor = itemTops[start];
            } else if (top >= 0.0f) {
                anchor -= viewport - itemHeight;
            }
        }
        return std::clamp(anchor + factor * viewport,
            0.0f, itemTops.back() - viewport);
    }
    void CheckCatalogOffsetModes(
        const std::vector<float>& itemTops, const CatalogOffsetCase& input, int32_t& combinations)
    {
        const auto [align, target, start, factor] = input;
        const float viewport = pattern_->GetMainContentSize();
        const float expectedOffset = CatalogExpectedOffset(itemTops, input);
        PhysicalScrollTo(expectedOffset);
        const auto first = pattern_->GetGridLayoutInfo().startIndex_;
        const auto last = pattern_->GetGridLayoutInfo().endIndex_;
        std::map<int32_t, Rect> reference;
        for (int32_t index = first; index <= last; ++index) {
            reference[index] = pattern_->GetItemRect(index);
        }
        ASSERT_FALSE(reference.empty());
        for (auto mode : { FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE }) {
            SCOPED_TRACE(static_cast<int32_t>(mode));
            PhysicalScrollTo(itemTops[start]);
            MockAnimationManager::GetInstance().SetTicks(ANIMATION_TICKS);
            pattern_->ScrollToIndexFastScroll(target, true, align, factor * viewport, mode);
            FlushUITasks(frameNode_);
            FinishFastScrollAnimation();
            ASSERT_FALSE(pattern_->AnimateRunning());
            ASSERT_EQ(pattern_->GetFastScrollRequest(), nullptr);
            EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, first);
            EXPECT_EQ(pattern_->GetGridLayoutInfo().endIndex_, last);
            for (const auto& [index, rectangle] : reference) {
                const auto actual = pattern_->GetItemRect(index);
                EXPECT_NEAR(actual.Left(), rectangle.Left(), 0.1f) << index;
                EXPECT_NEAR(actual.Top(), rectangle.Top(), 0.1f) << index;
                EXPECT_NEAR(actual.Width(), rectangle.Width(), 0.1f) << index;
                EXPECT_NEAR(actual.Height(), rectangle.Height(), 0.1f) << index;
            }
            ++combinations;
        }
    }
    void CheckCatalogFactors(const std::vector<float>& itemTops, ScrollAlign align,
        int32_t target, int32_t start, int32_t& combinations)
    {
        for (float factor : { 0.0f, 0.5f, -0.5f, 2.0f, -2.0f }) {
            SCOPED_TRACE(std::to_string(static_cast<int32_t>(align)) + ":" + std::to_string(target) +
                ":" + std::to_string(start) + ":" + std::to_string(factor));
            CheckCatalogOffsetModes(itemTops, { align, target, start, factor }, combinations);
            ASSERT_FALSE(HasFatalFailure());
        }
    }
    void CheckCatalogCycle(FastScrollMode mode, int32_t round)
    {
        SCOPED_TRACE(std::to_string(static_cast<int32_t>(mode)) + ":" + std::to_string(round));
        ScrollToIndex(0, false, ScrollAlign::START);
        MockAnimationManager::GetInstance().SetTicks(ANIMATION_TICKS);
        pattern_->ScrollToIndexFastScroll(ANIMATION_TARGET_INDEX, true, ScrollAlign::START, 0.0f, mode);
        if (round < COMPLETION_ROUNDS_WITH_WARMUP) {
            FlushUITasks(frameNode_);
            FinishFastScrollAnimation();
            EXPECT_NEAR(pattern_->GetItemRect(ANIMATION_TARGET_INDEX).Top(), 0.0f, 0.1f);
            EXPECT_NEAR(pattern_->GetTotalOffset(), 12000.0f, 0.1f);
        } else {
            if (round % CANCEL_PHASE_COUNT != 0) {
                FlushUITasks(frameNode_);
                for (int32_t tick = 0; tick < (round % CANCEL_PHASE_COUNT == 1 ? 1 : CANCEL_TAIL_TICKS); ++tick) {
                    MockAnimationManager::GetInstance().Tick();
                    FlushUITasks(frameNode_);
                }
            }
            pattern_->StopAnimate();
            MockAnimationManager::GetInstance().Tick();
            FlushUITasks(frameNode_);
        }
        EXPECT_FALSE(pattern_->AnimateRunning());
        EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
        EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
        EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
        EXPECT_FALSE(pattern_->fastScrollPendingMovement_.has_value());
        EXPECT_FALSE(pattern_->fastScrollPendingDelta_.has_value());
        ScrollToIndex(LEGACY_TARGET_INDEX, false, ScrollAlign::START);
        EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 500.0f);
    }
    void RecordFrameOffsets(const std::vector<float>& positions)
    {
        std::string frames;
        for (auto position : positions) {
            frames += std::to_string(position) + ",";
        }
        RecordProperty("actual_frame_offsets", frames);
    }
};

/**
 * @tc.name: ParseArgsTest001
 * @tc.desc: Test Empty String
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, ParseArgsTest001, TestSize.Level1)
{
    // Arrange
    std::string args = "";

    // Act
    std::string result = GridUtils::ParseArgs(args);

    // Assert
    EXPECT_EQ(args, result);
}

/**
 * @tc.name: ParseArgsTest002
 * @tc.desc: Test No Auto Keyword
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, ParseArgsTest002, TestSize.Level1)
{
    // Arrange
    std::string args = "1 2 3";

    // Act
    std::string result = GridUtils::ParseArgs(args);

    // Assert
    EXPECT_EQ(args, result);
}

/**
 * @tc.name: ParseArgsTest003
 * @tc.desc: Test Single Auto Keyword
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, ParseArgsTest003, TestSize.Level1)
{
    // Arrange
    std::string args = "auto";

    // Act
    std::string result = GridUtils::ParseArgs(args);

    // Assert
    EXPECT_EQ(" 1fr", result);
}

/**
 * @tc.name: ParseArgsTest004
 * @tc.desc: Test Multiple Auto Keywords
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, ParseArgsTest004, TestSize.Level1)
{
    // Arrange
    std::string args = "auto 2 auto";

    // Act
    std::string result = GridUtils::ParseArgs(args);

    // Assert
    EXPECT_EQ(" 1fr 2 1fr", result);
}

/**
 * @tc.name: ParseArgsTest005
 * @tc.desc: Test Mixed Args
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, ParseArgsTest005, TestSize.Level1)
{
    // Arrange
    std::string args = "1fr 2 3 auto";

    // Act
    std::string result = GridUtils::ParseArgs(args);

    // Assert
    EXPECT_EQ(" 1fr 2 3 1fr", result);
}

/**
 * @tc.name: ScrollToTagetTest001
 * @tc.desc: Test ScrollToTaget with id in viewport
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, ScrollToTagetTest001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    model.SetCachedCount(2, false);
    CreateItemsInLazyForEach(50, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();
    auto mockContext = AceType::DynamicCast<MockRenderContext>(frameNode_->GetRenderContext());
    mockContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, WIDTH, HEIGHT));
    float targetOffset = 50.0f;
    ScrollAlign align = ScrollAlign::CENTER;
    // No target node
    RefPtr<FrameNode> target;
    EXPECT_EQ(ScrollablePattern::ScrollToTarget(frameNode_, target, targetOffset, align), RET_FAILED);
    // Move index 3 to middle of Grid with targetOffset
    auto child = frameNode_->GetChildByIndex(3);
    ASSERT_NE(child, nullptr);
    auto childNode = child->GetHostNode();
    ASSERT_NE(childNode, nullptr);
    auto mockChildContext = AceType::DynamicCast<MockRenderContext>(childNode->GetRenderContext());
    mockChildContext->SetPaintRectWithTransform(RectF(0.0f, 3 * ITEM_MAIN_SIZE, WIDTH, ITEM_MAIN_SIZE));
    EXPECT_EQ(ScrollablePattern::ScrollToTarget(frameNode_, childNode, targetOffset, align), RET_SUCCESS);
    EXPECT_TRUE(TickPosition(-150.0f));

    // Move index 0 to start of Grid
    align = ScrollAlign::START;
    child = frameNode_->GetChildByIndex(0);
    ASSERT_NE(child, nullptr);
    childNode = child->GetHostNode();
    ASSERT_NE(childNode, nullptr);
    mockChildContext = AceType::DynamicCast<MockRenderContext>(childNode->GetRenderContext());
    mockChildContext->SetPaintRectWithTransform(RectF(0.0f, -150.0f, WIDTH, ITEM_MAIN_SIZE));
    EXPECT_EQ(ScrollablePattern::ScrollToTarget(frameNode_, childNode, 0.0f, align), RET_SUCCESS);
    EXPECT_TRUE(TickPosition(0.0f));
}

/**
 * @tc.name: ScrollToTagetTest002
 * @tc.desc: Test ScrollToTaget from dump
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, ScrollToTagetTest002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetEdgeEffect(EdgeEffect::SPRING, true);
    model.SetCachedCount(2, true);
    CreateItemsInLazyForEach(50, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();
    auto mockContext = AceType::DynamicCast<MockRenderContext>(frameNode_->GetRenderContext());
    mockContext->SetPaintRectWithTransform(RectF(0.0f, 0.0f, WIDTH, HEIGHT));
    float targetOffset = 50.0f;
    ScrollAlign align = ScrollAlign::CENTER;
    // Move index 5 to middle of Grid with targetOffset
    auto child = frameNode_->GetChildByIndex(5);
    ASSERT_NE(child, nullptr);
    auto childNode = child->GetHostNode();
    ASSERT_NE(childNode, nullptr);
    auto mockChildContext = AceType::DynamicCast<MockRenderContext>(childNode->GetRenderContext());
    mockChildContext->SetPaintRectWithTransform(RectF(0.0f, 5 * ITEM_MAIN_SIZE, WIDTH, ITEM_MAIN_SIZE));
    std::vector<std::string> params = { "-element", "-lastpage", std::to_string(childNode->GetId()),
        std::to_string(targetOffset), std::to_string(static_cast<int32_t>(align)) };
    SimplifiedInspector::TestScrollToTarget(params, frameNode_);
    EXPECT_TRUE(TickPosition(-350.0f));
}

/**
 * @tc.name: GetOffsetWithLimit001
 * @tc.desc: Test offset is Positive and totalOffset is Less than offset
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, GetOffsetWithLimit001, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    float contentOffset = 20.f;
    ScrollableModelNG::SetContentStartOffset(contentOffset);
    ScrollableModelNG::SetContentEndOffset(contentOffset);
    CreateItemsInLazyForEach(1, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    // total item height ITEM_MAIN_SIZE + 20 + 20, totalOffset = 0
    float offset = pattern_->GetOffsetWithLimit(ITEM_MAIN_SIZE + contentOffset * 3);
    EXPECT_EQ(offset, 0.0f);
}

/**
 * @tc.name: GetOffsetWithLimit002
 * @tc.desc: Test offset is Positive and totalOffset is larger than offset
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, GetOffsetWithLimit002, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    float contentOffset = 20.f;
    ScrollableModelNG::SetContentStartOffset(contentOffset);
    ScrollableModelNG::SetContentEndOffset(contentOffset);
    CreateItemsInLazyForEach(20, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();
    ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, false);
    // total item height ITEM_MAIN_SIZE * 10 + 20 + 20
    float offset = pattern_->GetOffsetWithLimit(ITEM_MAIN_SIZE);
    EXPECT_EQ(offset, ITEM_MAIN_SIZE);
}

/**
 * @tc.name: GetOffsetWithLimit003
 * @tc.desc: Test offset is Negative and remainHeight is enough
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, GetOffsetWithLimit003, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    float contentOffset = 20.f;
    ScrollableModelNG::SetContentStartOffset(contentOffset);
    ScrollableModelNG::SetContentEndOffset(contentOffset);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    // total item height ITEM_MAIN_SIZE * 10 + 20 + 20
    float offset = pattern_->GetOffsetWithLimit(-ITEM_MAIN_SIZE);
    EXPECT_EQ(offset, -ITEM_MAIN_SIZE);
}

/**
 * @tc.name: GetOffsetWithLimit004
 * @tc.desc: Test offset is Negative and remainHeight is not enough
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, GetOffsetWithLimit004, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    float contentOffset = 20.f;
    ScrollableModelNG::SetContentStartOffset(contentOffset);
    ScrollableModelNG::SetContentEndOffset(contentOffset * 1.5);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, false);
    // total item height ITEM_MAIN_SIZE * 10 + 20 + 20
    float offset = pattern_->GetOffsetWithLimit(-ITEM_MAIN_SIZE);
    EXPECT_EQ(offset, 0.0f);
}

/**
 * @tc.name: GetOffsetWithLimit005
 * @tc.desc: Test offset is 0
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, GetOffsetWithLimit005, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    float contentOffset = 20.f;
    ScrollableModelNG::SetContentStartOffset(contentOffset);
    ScrollableModelNG::SetContentEndOffset(contentOffset * 1.5);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    float offset = pattern_->GetOffsetWithLimit(0);
    EXPECT_EQ(offset, 0.0f);
}

/**
 * @tc.name: GridPattern::IsAtTopWithDelta
 * @tc.desc: test IsAtTopWithDelta with contentOffset
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, IsAtTopWithDelta, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    float contentOffset = 20.f;
    ScrollableModelNG::SetContentStartOffset(contentOffset);
    ScrollableModelNG::SetContentEndOffset(contentOffset * 1.5);
    CreateItemsInLazyForEach(50, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    EXPECT_TRUE(pattern_->IsAtTopWithDelta());
    EXPECT_EQ(pattern_->info_.currentOffset_, 20.0f);

    ScrollBy(0, 20);
    EXPECT_EQ(pattern_->info_.currentOffset_, 0.0f);
    EXPECT_FALSE(pattern_->IsAtTopWithDelta());
}

/**
 * @tc.name: GridPattern::IsAtTopOrBottom
 * @tc.desc: test IsAtTop and IsAtBottom
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, IsAtTopOrBottom, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    float contentOffset = 20.f;
    ScrollableModelNG::SetContentStartOffset(contentOffset);
    ScrollableModelNG::SetContentEndOffset(contentOffset * 1.5);
    CreateItemsInLazyForEach(50, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    EXPECT_TRUE(pattern_->IsAtTop());
    EXPECT_FALSE(pattern_->IsAtBottom());

    ScrollBy(0, 20);
    EXPECT_FALSE(pattern_->IsAtTop());
    EXPECT_FALSE(pattern_->IsAtBottom());

    ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, false);
    EXPECT_FALSE(pattern_->IsAtTop());
    EXPECT_TRUE(pattern_->IsAtBottom());

    ScrollBy(0, -20);
    EXPECT_FALSE(pattern_->IsAtTop());
    EXPECT_FALSE(pattern_->IsAtBottom());
}

/**
 * @tc.name: GridPattern.endHeight
 * @tc.desc: test endHeight with contentOffset
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, EndHeightTest, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    float contentOffset = 20.f;
    ScrollableModelNG::SetContentStartOffset(contentOffset);
    ScrollableModelNG::SetContentEndOffset(contentOffset * 1.5);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    EXPECT_EQ(pattern_->endHeight_, 0.0f);

    ScrollToEdge(ScrollEdgeType::SCROLL_BOTTOM, false);
    EXPECT_FLOAT_EQ(pattern_->endHeight_, pattern_->GetTotalHeight() - contentOffset - HEIGHT);
}

/**
 * @tc.name: GetContentStartOffsetTest
 * @tc.desc: test GridPattern::GetContentStartOffsetTest
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, GetContentStartOffsetTest, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    float contentOffset = 20.f;
    ScrollableModelNG::SetContentStartOffset(contentOffset);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    EXPECT_EQ(pattern_->GetContentStartOffset(), contentOffset);
}

/**
 * @tc.name: GetContentStartOffsetWithInvalidValueTest
 * @tc.desc: test GridPattern::GetContentStartOffsetTest with invalid value
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, GetContentStartOffsetWithInvalidValueTest, TestSize.Level1)
{
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    float contentOffset = HEIGHT / 2;
    ScrollableModelNG::SetContentStartOffset(contentOffset);
    ScrollableModelNG::SetContentEndOffset(contentOffset);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    EXPECT_FLOAT_EQ(pattern_->GetContentStartOffset(), 0.0f);
}
/**
 * @tc.name: HandleBlurEvent
 * @tc.desc: test GridPattern::HandleBlurEvent
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, HandleBlurEventTest, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Test OnModifyDone
     */
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr 1fr");
    CreateFocusableGridItems(10, ITEM_MAIN_SIZE, ITEM_MAIN_SIZE);
    CreateDone();
    EXPECT_TRUE(frameNode_->GetFocusHub());
    /**
     * @tc.steps: step2. Call OnModifyDone
     */
    pattern_->OnModifyDone();
    auto focusHub = frameNode_->GetFocusHub();
    FlushUITasks();
    EXPECT_TRUE(focusHub);
    /**
     * @tc.steps: step2. Handle item focus
     */
    RefPtr<FocusHub> currentFocusNode = GetChildFocusHub(frameNode_, 0);
    currentFocusNode->RequestFocusImmediately();
    FlushUITasks();
    EXPECT_EQ(focusHub->GetFocusDependence(), FocusDependence::AUTO);

    /**
     * @tc.steps: step4. lostChild
     */
    focusHub->LostChildFocusToSelf();
    EXPECT_EQ(focusHub->GetFocusDependence(), FocusDependence::SELF);

    /**
     * @tc.steps: step5. Call HandleBlurEvent
     */
    pattern_->HandleBlurEvent();
    EXPECT_EQ(focusHub->GetFocusDependence(), FocusDependence::AUTO);
}

/**
 * @tc.name: GetLayoutMode001
 * @tc.desc: Test GetLayoutMode returns "adaptive" when no template is set
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, GetLayoutMode001, TestSize.Level1)
{
    // Arrange: Create Grid without any template
    GridModelNG model = CreateGrid();
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    // Act & Assert
    EXPECT_EQ(pattern_->GetLayoutMode(), "adaptive");
}

/**
 * @tc.name: GetLayoutMode002
 * @tc.desc: Test GetLayoutMode returns "static" when both row and column templates are set
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, GetLayoutMode002, TestSize.Level1)
{
    // Arrange: Create Grid with both templates
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetRowsTemplate("1fr 1fr");
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    // Act & Assert
    EXPECT_EQ(pattern_->GetLayoutMode(), "static");
}

/**
 * @tc.name: GetLayoutMode003
 * @tc.desc: Test GetLayoutMode returns "custom" when userDefined is set
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, GetLayoutMode003, TestSize.Level1)
{
    int32_t crossCount = 2;
    GridLayoutOptions options;
    auto onGetStartIndexByOffset = [crossCount](float offset) -> GridStartLineInfo {
        int32_t line = offset / static_cast<int32_t>(ITEM_MAIN_SIZE);
        float startOffset = std::fmod(offset, ITEM_MAIN_SIZE);
        return {
            .startIndex = line * crossCount, .startLine = line, .startOffset = 0.f - startOffset, .totalOffset = offset
        };
    };
    options.getStartIndexByOffset = std::move(onGetStartIndexByOffset);

    auto onGetStartIndexByIndex = [crossCount](int32_t targetIndex) -> GridStartLineInfo {
        int32_t line = targetIndex / crossCount;
        return { .startIndex = line * crossCount,
            .startLine = line,
            .startOffset = 0.0f,
            .totalOffset = line * ITEM_MAIN_SIZE };
    };
    options.getStartIndexByIndex = std::move(onGetStartIndexByIndex);

    // Arrange: Create Grid with custom layout
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(options);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    // Act & Assert
    EXPECT_EQ(pattern_->GetLayoutMode(), "custom");
}

/**
 * @tc.name: GetLayoutMode004
 * @tc.desc: Test GetLayoutMode returns "irregular" when irregular layout is used
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, GetLayoutMode004, TestSize.Level1)
{
    GridLayoutOptions option;
    option.irregularIndexes = { 0 };
    auto onGetIrregularSizeByIndex = [](int32_t index) -> GridItemSize { return { .columns = 2, .rows = 2 }; };
    option.getSizeByIndex = std::move(onGetIrregularSizeByIndex);
    // Arrange: Create Grid with irregular layout
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetLayoutOptions(option);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    // Act & Assert
    EXPECT_EQ(pattern_->GetLayoutMode(), "irregular");
}

/**
 * @tc.name: GetLayoutMode005
 * @tc.desc: Test GetLayoutMode returns "scroll" when only column template is set
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, GetLayoutMode005, TestSize.Level1)
{
    // Arrange: Create Grid with only column template (scrollable layout)
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    // Act & Assert
    EXPECT_EQ(pattern_->GetLayoutMode(), "scroll");
}

/**
 * @tc.name: GetLayoutMode006
 * @tc.desc: Test GetLayoutMode returns "scrollWithOptions" when column template and layout options are set
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, GetLayoutMode006, TestSize.Level1)
{
    // Arrange: Create Grid with column template and layout options
    GridModelNG model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr");
    GridLayoutOptions options;
    options.regularSize = { 1, 1 };
    model.SetLayoutOptions(options);
    CreateItemsInLazyForEach(10, [](uint32_t idx) { return ITEM_MAIN_SIZE; });
    CreateDone();

    // Act & Assert
    EXPECT_EQ(pattern_->GetLayoutMode(), "scrollWithOptions");
}
/**
 * @tc.name: FastScrollStartsMappedAnimation
 * @tc.desc: A prepared Grid request starts the shared four-viewport animation mapping.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollStartsMappedAnimation, TestSize.Level1)
{
    MockAnimationManager::GetInstance().SetTicks(8);
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(150, [](uint32_t) { return 100.0f; });
    CreateDone();
    const float viewport = pattern_->GetMainContentSize();
    ASSERT_GT(viewport, 0.0f);
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    const auto* request = pattern_->GetFastScrollRequest();
    ASSERT_NE(request, nullptr);
    EXPECT_TRUE(pattern_->AnimateRunning());
    EXPECT_FLOAT_EQ(request->viewportSize, viewport);
    EXPECT_FLOAT_EQ(request->targetOffset, 12000.0f);
    EXPECT_FLOAT_EQ(request->skipDistance, 12000.0f - 4.0f * viewport);
    EXPECT_TRUE(request->skipPending);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);
}

/**
 * @tc.name: FastScrollLongAnimationWork
 * @tc.desc: The real Grid animation retains both ends, skips the middle layout work and finishes at its item.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollLongAnimationWork, TestSize.Level1)
{
    MockAnimationManager::GetInstance().SetTicks(8);
    auto created = std::make_shared<std::vector<int32_t>>();
    auto measured = std::make_shared<std::vector<int32_t>>();
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(150, [created, measured](uint32_t index) {
        created->push_back(static_cast<int32_t>(index));
        auto node = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        node->measureCallback_ = std::make_unique<std::function<void(RefPtr<Kit::FrameNode>&)>>(
            [measured, index](RefPtr<Kit::FrameNode>&) { measured->push_back(static_cast<int32_t>(index)); });
        return 100.0f;
    });
    CreateDone();
    created->clear();
    measured->clear();
    const float viewport = pattern_->GetMainContentSize();
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    ASSERT_TRUE(pattern_->AnimateRunning());
    std::vector<float> positions { static_cast<float>(pattern_->GetTotalOffset()) };
    std::vector<std::pair<size_t, size_t>> workCounts { { created->size(), measured->size() } };
    for (int32_t tick = 0; tick < 8; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks(frameNode_);
        positions.push_back(pattern_->GetTotalOffset());
        workCounts.emplace_back(created->size(), measured->size());
    }
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 12000.0f);
    const auto rect = pattern_->GetItemRect(120);
    ASSERT_GT(rect.Width(), 0.0f);
    EXPECT_FLOAT_EQ(rect.Top(), 0.0f);
    EXPECT_FLOAT_EQ(rect.Height(), 100.0f);
    const auto middle = [](int32_t index) { return index >= 24 && index < 96; };
    EXPECT_EQ(std::count_if(created->begin(), created->end(), middle), 0);
    EXPECT_EQ(std::count_if(measured->begin(), measured->end(), middle), 0);
    CheckLongAnimationJumps(positions, viewport);
    RecordFrameOffsets(positions);
    RecordWorkObservation(2, *created, *measured, positions, workCounts, 120);
}

/**
 * @tc.name: FastScrollVariableSizeEnd
 * @tc.desc: Unknown measured sizes converge to the actual target frame without endless retargeting.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollVariableSizeEnd, TestSize.Level1)
{
    MockAnimationManager::GetInstance().SetTicks(8);
    auto created = std::make_shared<std::vector<int32_t>>();
    auto measured = std::make_shared<std::vector<int32_t>>();
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(150, [created, measured](uint32_t index) {
        created->push_back(static_cast<int32_t>(index));
        auto node = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        node->measureCallback_ = std::make_unique<std::function<void(RefPtr<Kit::FrameNode>&)>>(
            [measured, index](RefPtr<Kit::FrameNode>&) { measured->push_back(static_cast<int32_t>(index)); });
        return 80.0f + static_cast<float>(index % 5) * 20.0f;
    });
    CreateDone();
    created->clear();
    measured->clear();
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    ASSERT_TRUE(pattern_->AnimateRunning());
    std::vector<float> positions { static_cast<float>(pattern_->GetTotalOffset()) };
    std::vector<std::pair<size_t, size_t>> workCounts { { created->size(), measured->size() } };
    for (int32_t tick = 0; tick < 40; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks(frameNode_);
        positions.push_back(pattern_->GetTotalOffset());
        workCounts.emplace_back(created->size(), measured->size());
        if (!pattern_->AnimateRunning() && !pattern_->GetFastScrollRequest()) {
            break;
        }
    }
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    const auto rect = pattern_->GetItemRect(120);
    ASSERT_GT(rect.Width(), 0.0f);
    EXPECT_FLOAT_EQ(rect.Top(), 0.0f);
    EXPECT_FLOAT_EQ(rect.Height(), 80.0f);
    const auto middle = [](int32_t index) { return index >= 24 && index < 96; };
    EXPECT_EQ(std::count_if(created->begin(), created->end(), middle), 0);
    EXPECT_EQ(std::count_if(measured->begin(), measured->end(), middle), 0);
    RecordFrameOffsets(positions);
    RecordWorkObservation(2, *created, *measured, positions, workCounts, 120);
}

/**
 * @tc.name: FastScrollThresholdAndReverse
 * @tc.desc: Zero, exactly four viewports and either side of the threshold use the requested mapping in both directions.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollThresholdAndReverse, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateFixedHeightItems(150, 100.0f);
    CreateDone();
    const float viewport = pattern_->GetMainContentSize();
    const int32_t boundary = static_cast<int32_t>(4.0f * viewport / 100.0f);
    ASSERT_FLOAT_EQ(boundary * 100.0f, 4.0f * viewport);
    for (auto mode : { FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE }) {
        for (int32_t distance : { 0, boundary - 1, boundary, boundary + 1 }) {
            for (bool reverse : { false, true }) {
                SCOPED_TRACE(std::to_string(static_cast<int32_t>(mode)) + ":" +
                    std::to_string(distance) + ":" + std::to_string(reverse));
                const int32_t start = reverse ? boundary + 8 : 0;
                const int32_t target = reverse ? start - distance : distance;
                ScrollToIndex(start, false, ScrollAlign::START);
                MockAnimationManager::GetInstance().SetTicks(8);
                pattern_->ScrollToIndexFastScroll(target, true, ScrollAlign::START, 0.0f, mode);
                FlushUITasks(frameNode_);
                if (distance == 0) {
                    EXPECT_FALSE(pattern_->AnimateRunning());
                    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
                    continue;
                }
                ASSERT_NE(pattern_->GetFastScrollRequest(), nullptr);
                const float expectedSkip = std::max(0.0f, distance * 100.0f - 4.0f * viewport);
                EXPECT_FLOAT_EQ(pattern_->GetFastScrollRequest()->skipDistance, reverse ? -expectedSkip : expectedSkip);
                int32_t skipFrames = 0;
                for (int32_t tick = 0; tick < 8; ++tick) {
                    MockAnimationManager::GetInstance().Tick();
                    skipFrames += pattern_->fastScrollSkip_ ? 1 : 0;
                    FlushUITasks(frameNode_);
                }
                EXPECT_EQ(skipFrames, distance > boundary ? 1 : 0);
                EXPECT_FALSE(pattern_->AnimateRunning());
                EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
                EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), target * 100.0f);
                EXPECT_FLOAT_EQ(pattern_->GetItemRect(target).Top(), 0.0f);
            }
        }
    }
}

/**
 * @tc.name: FastScrollSizeChangeAfterSkip
 * @tc.desc: A target enlarged after the first skip is measured again and may require another skip before ending.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollSizeChangeAfterSkip, TestSize.Level1)
{
    MockAnimationManager::GetInstance().SetTicks(8);
    auto lastTarget = std::make_shared<WeakPtr<FrameNode>>();
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(150, [lastTarget](uint32_t index) {
        if (index == 120) {
            *lastTarget = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        }
        return 100.0f;
    });
    CreateDone();
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::END, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    int32_t skips = 0;
    bool resized = false;
    for (int32_t tick = 0; tick < 80; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        const bool skipped = pattern_->fastScrollSkip_;
        skips += skipped ? 1 : 0;
        FlushUITasks(frameNode_);
        if (skipped && !resized) {
            auto target = lastTarget->Upgrade();
            ASSERT_NE(target, nullptr);
            target->GetLayoutProperty()->UpdateUserDefinedIdealSize(CalcSize(CalcLength(WIDTH), CalcLength(3000.0f)));
            target->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
            frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
            FlushUITasks(frameNode_);
            resized = true;
        }
        if (!pattern_->AnimateRunning() && !pattern_->GetFastScrollRequest()) {
            break;
        }
    }
    EXPECT_TRUE(resized);
    EXPECT_GE(skips, 2);
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    const auto actual = pattern_->GetItemRect(120);
    ASSERT_GT(actual.Width(), 0.0f);
    EXPECT_FLOAT_EQ(actual.Height(), 3000.0f);
    ScrollToIndex(120, false, ScrollAlign::END);
    const auto reference = pattern_->GetItemRect(120);
    EXPECT_EQ(actual, reference);
}

/**
 * @tc.name: FastScrollHorizontalRtlOffsets
 * @tc.desc: Signed offsets, including an offscreen target, match ordinary horizontal RTL layout for every alignment.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollHorizontalRtlOffsets, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetRowsTemplate("1fr");
    model.SetCachedCount(0);
    layoutProperty_->UpdateLayoutDirection(TextDirection::RTL);
    CreateGridItems(150, 100.0f, FILL_VALUE);
    CreateDone();
    for (auto align : { ScrollAlign::START, ScrollAlign::CENTER, ScrollAlign::END, ScrollAlign::AUTO }) {
        for (float extra : { 0.0f, -50.0f, 50.0f, -1000.0f, 1000.0f }) {
            SCOPED_TRACE(std::to_string(static_cast<int32_t>(align)) + ":" + std::to_string(extra));
            ScrollToIndex(0, false, ScrollAlign::START);
            // Match the feature's smooth=true contract; the non-animated AUTO branch is a different path.
            MockAnimationManager::GetInstance().SetTicks(8);
            pattern_->ScrollToIndex(120, true, align, extra);
            FlushUITasks(frameNode_);
            for (int32_t tick = 0; tick < 8; ++tick) {
                MockAnimationManager::GetInstance().Tick();
                FlushUITasks(frameNode_);
            }
            ASSERT_FALSE(pattern_->AnimateRunning());
            const auto first = pattern_->GetGridLayoutInfo().startIndex_;
            const auto last = pattern_->GetGridLayoutInfo().endIndex_;
            std::map<int32_t, Rect> reference;
            for (int32_t index = first; index <= last; ++index) {
                reference[index] = pattern_->GetItemRect(index);
            }
            ScrollToIndex(0, false, ScrollAlign::START);
            MockAnimationManager::GetInstance().SetTicks(8);
            pattern_->ScrollToIndexFastScroll(120, true, align, extra, FastScrollMode::SKIP_CREATE_AND_MEASURE);
            FlushUITasks(frameNode_);
            for (int32_t tick = 0;
                tick < MAX_ANIMATION_TICKS && (pattern_->AnimateRunning() || pattern_->GetFastScrollRequest());
                ++tick) {
                MockAnimationManager::GetInstance().Tick();
                FlushUITasks(frameNode_);
            }
            EXPECT_FALSE(pattern_->AnimateRunning());
            EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
            EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, first);
            EXPECT_EQ(pattern_->GetGridLayoutInfo().endIndex_, last);
            for (const auto& [index, rect] : reference) {
                EXPECT_EQ(pattern_->GetItemRect(index), rect) << "index=" << index;
            }
        }
    }
}

/**
 * @tc.name: FastScrollOptionsAnimation
 * @tc.desc: Unknown measured sizes converge to the actual target frame without endless retargeting.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollOptionsAnimation, TestSize.Level1)
{
    ScopedFastScrollOptionsAlgorithm scope;
    MockAnimationManager::GetInstance().SetTicks(8);
    auto created = std::make_shared<std::vector<int32_t>>();
    auto measured = std::make_shared<std::vector<int32_t>>();
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(0);
    GridLayoutOptions options;
    options.irregularIndexes = { 39, 40 };
    options.getSizeByIndex = [](int32_t) { return GridItemSize { 1, 2 }; };
    model.SetLayoutOptions(options);
    CreateItemsInLazyForEach(150, [created, measured](uint32_t index) {
        created->push_back(static_cast<int32_t>(index));
        auto node = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        node->measureCallback_ = std::make_unique<std::function<void(RefPtr<Kit::FrameNode>&)>>(
            [measured, index](RefPtr<Kit::FrameNode>&) { measured->push_back(static_cast<int32_t>(index)); });
        return 80.0f + static_cast<float>(index % 5) * 20.0f;
    });
    CreateDone();
    ASSERT_NE(AceType::DynamicCast<GridScrollWithOptionsLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm()), nullptr);
    created->clear();
    measured->clear();
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    ASSERT_TRUE(pattern_->AnimateRunning());
    const auto observation = ObserveTopologyAnimation(created, measured);
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    const auto rect = pattern_->GetItemRect(120);
    ASSERT_GT(rect.Width(), 0.0f);
    EXPECT_FLOAT_EQ(rect.Top(), 0.0f);
    ScrollToIndex(120, false, ScrollAlign::START);
    EXPECT_EQ(rect, pattern_->GetItemRect(120));
    EXPECT_TRUE(observation.hasSkipped);
    EXPECT_LT(observation.headEnd + 20, observation.tailStart);
    EXPECT_LE(observation.headOffset, 2.0f * pattern_->GetMainContentSize() + 0.1f);
    const auto middle = [headEnd = observation.headEnd, tailStart = observation.tailStart](int32_t index) {
        return index > headEnd && index < tailStart;
    };
    EXPECT_EQ(std::count_if(created->begin(), created->end(), middle), 0);
    EXPECT_EQ(std::count_if(measured->begin(), measured->end(), middle), 0);
    RecordTopologyAnimation(observation, created, measured);
}

/**
 * @tc.name: FastScrollIrregularAnimation
 * @tc.desc: Unknown measured sizes converge to the actual target frame without endless retargeting.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollIrregularAnimation, TestSize.Level1)
{
    MockAnimationManager::GetInstance().SetTicks(8);
    auto created = std::make_shared<std::vector<int32_t>>();
    auto measured = std::make_shared<std::vector<int32_t>>();
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(0);
    GridLayoutOptions options;
    options.irregularIndexes = { 39, 40, 120 };
    options.getSizeByIndex = [](int32_t) { return GridItemSize { 2, 2 }; };
    model.SetLayoutOptions(options);
    CreateItemsInLazyForEach(150, [created, measured](uint32_t index) {
        created->push_back(static_cast<int32_t>(index));
        auto node = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        node->measureCallback_ = std::make_unique<std::function<void(RefPtr<Kit::FrameNode>&)>>(
            [measured, index](RefPtr<Kit::FrameNode>&) { measured->push_back(static_cast<int32_t>(index)); });
        return 80.0f + static_cast<float>(index % 5) * 20.0f;
    });
    CreateDone();
    ASSERT_NE(AceType::DynamicCast<GridIrregularLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm()), nullptr);
    created->clear();
    measured->clear();
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    ASSERT_TRUE(pattern_->AnimateRunning());
    const auto observation = ObserveTopologyAnimation(created, measured);
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    const auto rect = pattern_->GetItemRect(120);
    ASSERT_GT(rect.Width(), 0.0f);
    EXPECT_NEAR(rect.Top(), 0.0f, 0.1f); // Subpixel rounding in the float content-coordinate conversion.
    ScrollToIndex(120, false, ScrollAlign::START);
    EXPECT_EQ(rect, pattern_->GetItemRect(120));
    EXPECT_TRUE(observation.hasSkipped);
    EXPECT_LT(observation.headEnd + 20, observation.tailStart);
    EXPECT_LE(observation.headOffset, 2.0f * pattern_->GetMainContentSize() + 0.1f);
    const auto middle = [headEnd = observation.headEnd, tailStart = observation.tailStart](int32_t index) {
        return index > headEnd && index < tailStart;
    };
    EXPECT_EQ(std::count_if(created->begin(), created->end(), middle), 0);
    EXPECT_EQ(std::count_if(measured->begin(), measured->end(), middle), 0);
    RecordTopologyAnimation(observation, created, measured);
}

/**
 * @tc.name: FastScrollCustomAnimation
 * @tc.desc: Unknown measured sizes converge to the actual target frame without endless retargeting.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollCustomAnimation, TestSize.Level1)
{
    MockAnimationManager::GetInstance().SetTicks(8);
    auto created = std::make_shared<std::vector<int32_t>>();
    auto measured = std::make_shared<std::vector<int32_t>>();
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(0);
    ConfigureCustomAnimationOptions(model);
    CreateItemsInLazyForEach(150, [created, measured](uint32_t index) {
        created->push_back(static_cast<int32_t>(index));
        auto node = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        node->measureCallback_ = std::make_unique<std::function<void(RefPtr<Kit::FrameNode>&)>>(
            [measured, index](RefPtr<Kit::FrameNode>&) { measured->push_back(static_cast<int32_t>(index)); });
        return index % 3 == 0 ? 200.0f : 100.0f;
    });
    CreateDone();
    ASSERT_NE(AceType::DynamicCast<GridCustomLayoutAlgorithm>(pattern_->CreateLayoutAlgorithm()), nullptr);
    created->clear();
    measured->clear();
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    ASSERT_TRUE(pattern_->AnimateRunning());
    std::vector<float> positions { static_cast<float>(pattern_->GetTotalOffset()) };
    std::vector<std::pair<size_t, size_t>> workCounts { { created->size(), measured->size() } };
    for (int32_t tick = 0; tick < 64; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks(frameNode_);
        positions.push_back(pattern_->GetTotalOffset());
        workCounts.emplace_back(created->size(), measured->size());
        if (!pattern_->AnimateRunning() && !pattern_->GetFastScrollRequest()) {
            break;
        }
    }
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    const auto rect = pattern_->GetItemRect(120);
    ASSERT_GT(rect.Width(), 0.0f);
    EXPECT_FLOAT_EQ(rect.Top(), 0.0f);
    ScrollToIndex(120, false, ScrollAlign::START);
    EXPECT_EQ(rect, pattern_->GetItemRect(120));
    const auto middle = [](int32_t index) { return index >= 24 && index < 96; };
    EXPECT_EQ(std::count_if(created->begin(), created->end(), middle), 0);
    EXPECT_EQ(std::count_if(measured->begin(), measured->end(), middle), 0);
    RecordFrameOffsets(positions);
    RecordWorkObservation(2, *created, *measured, positions, workCounts, 120);
}

/**
 * @tc.name: FastScrollLongMode1Work
 * @tc.desc: The real Grid animation retains both ends, skips the middle layout work and finishes at its item.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollLongMode1Work, TestSize.Level1)
{
    MockAnimationManager::GetInstance().SetTicks(8);
    auto created = std::make_shared<std::vector<int32_t>>();
    auto measured = std::make_shared<std::vector<int32_t>>();
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(150, [created, measured](uint32_t index) {
        created->push_back(static_cast<int32_t>(index));
        auto node = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        node->measureCallback_ = std::make_unique<std::function<void(RefPtr<Kit::FrameNode>&)>>(
            [measured, index](RefPtr<Kit::FrameNode>&) { measured->push_back(static_cast<int32_t>(index)); });
        return 100.0f;
    });
    CreateDone();
    created->clear();
    measured->clear();
    const float viewport = pattern_->GetMainContentSize();
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_MEASURE);
    FlushUITasks(frameNode_);
    ASSERT_TRUE(pattern_->AnimateRunning());
    std::vector<float> positions { static_cast<float>(pattern_->GetTotalOffset()) };
    std::vector<std::pair<size_t, size_t>> workCounts { { created->size(), measured->size() } };
    for (int32_t tick = 0; tick < 8; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks(frameNode_);
        positions.push_back(pattern_->GetTotalOffset());
        workCounts.emplace_back(created->size(), measured->size());
    }
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 12000.0f);
    const auto rect = pattern_->GetItemRect(120);
    ASSERT_GT(rect.Width(), 0.0f);
    EXPECT_FLOAT_EQ(rect.Top(), 0.0f);
    EXPECT_FLOAT_EQ(rect.Height(), 100.0f);
    const auto middle = [](int32_t index) { return index >= 24 && index < 96; };
    EXPECT_EQ(std::count_if(measured->begin(), measured->end(), middle), 0);
    CheckLongAnimationJumps(positions, viewport);
    RecordFrameOffsets(positions);
    RecordWorkObservation(1, *created, *measured, positions, workCounts, 120);
}

/**
 * @tc.name: FastScrollAutoVisibleOffset
 * @tc.desc: AUTO for an already visible target applies the offset once and matches the ordinary animated call.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollAutoVisibleOffset, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateGridItems(150, FILL_VALUE, 100.0f);
    CreateDone();
    for (auto align : { ScrollAlign::AUTO }) {
        for (float extra : { 0.0f, -50.0f, 50.0f, -1000.0f, 1000.0f }) {
            SCOPED_TRACE(std::to_string(static_cast<int32_t>(align)) + ":" + std::to_string(extra));
            ScrollToIndex(50, false, ScrollAlign::START);
            MockAnimationManager::GetInstance().SetTicks(8);
            pattern_->ScrollToIndex(51, true, align, extra);
            FlushUITasks(frameNode_);
            for (int32_t tick = 0; tick < 8; ++tick) {
                MockAnimationManager::GetInstance().Tick();
                FlushUITasks(frameNode_);
            }
            ASSERT_FALSE(pattern_->AnimateRunning());
            const auto first = pattern_->GetGridLayoutInfo().startIndex_;
            const auto last = pattern_->GetGridLayoutInfo().endIndex_;
            std::map<int32_t, Rect> reference;
            for (int32_t index = first; index <= last; ++index) {
                reference[index] = pattern_->GetItemRect(index);
            }
            ScrollToIndex(50, false, ScrollAlign::START);
            MockAnimationManager::GetInstance().SetTicks(8);
            pattern_->ScrollToIndexFastScroll(51, true, align, extra, FastScrollMode::SKIP_CREATE_AND_MEASURE);
            FlushUITasks(frameNode_);
            for (int32_t tick = 0;
                tick < MAX_ANIMATION_TICKS && (pattern_->AnimateRunning() || pattern_->GetFastScrollRequest());
                ++tick) {
                MockAnimationManager::GetInstance().Tick();
                FlushUITasks(frameNode_);
            }
            EXPECT_FALSE(pattern_->AnimateRunning());
            EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
            EXPECT_EQ(pattern_->GetGridLayoutInfo().startIndex_, first);
            EXPECT_EQ(pattern_->GetGridLayoutInfo().endIndex_, last);
            for (const auto& [index, rect] : reference) {
                EXPECT_EQ(pattern_->GetItemRect(index), rect) << "index=" << index;
            }
        }
    }
}

/**
 * @tc.name: FastScrollDetachStopsAnimation
 * @tc.desc: Removing a Grid stops the controller animation before discarding its fast-scroll mapping.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollDetachStopsAnimation, TestSize.Level1)
{
    ScopedFastScrollAnimationClock clock;
    MockAnimationManager::GetInstance().SetTicks(8);
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(150, [](uint32_t) { return 100.0f; });
    CreateDone();
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks(frameNode_);
    const float before = pattern_->GetTotalOffset();
    ASSERT_TRUE(pattern_->AnimateRunning());
    pattern_->OnDetachFromMainTree();
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
    for (int32_t tick = 0; tick < 8; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks(frameNode_);
    }
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), before);
}

/**
 * @tc.name: FastScrollStopAndReplace
 * @tc.desc: Waiting and running requests can be stopped or replaced without later revival.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollStopAndReplace, TestSize.Level1)
{
    ScopedFastScrollAnimationClock clock;
    auto created = std::make_shared<std::vector<int32_t>>();
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(150, [created](uint32_t index) {
        created->push_back(static_cast<int32_t>(index));
        return 100.0f;
    });
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    pattern_->StopAnimate();
    CreateDone();
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(std::count(created->begin(), created->end(), 120), 0);
    MockAnimationManager::GetInstance().SetTicks(8);
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks(frameNode_);
    const float stopped = pattern_->GetTotalOffset();
    pattern_->StopAnimate();
    FlushUITasks(frameNode_);
    AdvanceFastScrollTicks(8);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), stopped);
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollGeometry_.has_value());
    EXPECT_FALSE(pattern_->fastScrollPendingDelta_.has_value());
    EXPECT_FALSE(pattern_->fastScrollPendingMovement_.has_value());
    pattern_->ScrollToIndexFastScroll(130, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_MEASURE);
    pattern_->ScrollToIndexFastScroll(20, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    for (int32_t tick = 0; tick < 32 && (pattern_->AnimateRunning() || pattern_->GetFastScrollRequest()); ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks(frameNode_);
    }
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 2000.0f);
    EXPECT_EQ(std::count(created->begin(), created->end(), 130), 0);
    pattern_->ScrollToIndex(5, false, ScrollAlign::START, 0.0f);
    FlushUITasks(frameNode_);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 500.0f);
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
}

/**
 * @tc.name: FastScrollTouchInterruptsWaitingAndRunning
 * @tc.desc: The registered touch callback cancels preparation and running motion, allowing subsequent manual scrolling.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollTouchInterruptsWaitingAndRunning, TestSize.Level1)
{
    ScopedFastScrollAnimationClock clock;
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(150, [](uint32_t) { return 100.0f; });
    CreateDone();
    ASSERT_NE(pattern_->touchEvent_, nullptr);
    const auto touch = pattern_->touchEvent_->GetTouchEventCallback();
    const auto makeTouch = [this](TouchType type) {
        auto info = CreateTouchEventInfo(type, Offset::Zero());
        TouchLocationInfo changed(1);
        changed.SetTouchType(type);
        changed.SetLocalLocation(Offset::Zero());
        info.AddChangedTouchLocationInfo(std::move(changed));
        return info;
    };
    auto down = makeTouch(TouchType::DOWN);
    auto up = makeTouch(TouchType::UP);
    ASSERT_FALSE(down.GetChangedTouches().empty());
    for (bool running : { false, true }) {
        SCOPED_TRACE(running);
        MockAnimationManager::GetInstance().SetTicks(8);
        pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
        if (running) {
            FlushUITasks(frameNode_);
            MockAnimationManager::GetInstance().Tick();
            FlushUITasks(frameNode_);
        }
        const float before = pattern_->GetTotalOffset();
        touch(down);
        FlushUITasks(frameNode_);
        EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
        EXPECT_FALSE(pattern_->AnimateRunning());
        for (int32_t tick = 0; tick < 8; ++tick) {
            MockAnimationManager::GetInstance().Tick();
            FlushUITasks(frameNode_);
        }
        EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), before);
        touch(up);
        pattern_->UpdateCurrentOffset(-30.0f, SCROLL_FROM_UPDATE);
        FlushUITasks(frameNode_);
        EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), before + 30.0f);
    }
}

/**
 * @tc.name: FastScrollOtherScrollTakesOver
 * @tc.desc: Ordinary offset input supersedes a fast controller request and cannot be followed by the old animation.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollOtherScrollTakesOver, TestSize.Level1)
{
    ScopedFastScrollAnimationClock clock;
    MockAnimationManager::GetInstance().SetTicks(8);
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(150, [](uint32_t) { return 100.0f; });
    CreateDone();
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks(frameNode_);
    const float before = pattern_->GetTotalOffset();
    pattern_->ScrollBy(30.0f);
    FlushUITasks(frameNode_);
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_FALSE(pattern_->AnimateRunning());
    for (int32_t tick = 0; tick < 8; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks(frameNode_);
    }
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), before + 30.0f);
}

/**
 * @tc.name: FastScrollEmptyFirstLayoutDoesNotRevive
 * @tc.desc: A first layout with no valid target drops the pending request before later data arrives.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollEmptyFirstLayoutDoesNotRevive, TestSize.Level1)
{
    ScopedFastScrollAnimationClock clock;
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    pattern_->ScrollToIndexFastScroll(20, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    CreateDone();
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
    AddFixedHeightItems(30, 100.0f);
    frameNode_->ChildrenUpdatedFrom(0);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    FlushUITasks(frameNode_);
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 0.0f);
    ScrollToIndex(20, false, ScrollAlign::START);
    EXPECT_GT(pattern_->GetItemRect(20).Height(), 0.0f);
}

/**
 * @tc.name: FastScrollDataRemovalDropsInvalidTarget
 * @tc.desc: Shrinking data invalidates a running target safely and does not leave a suspended request.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollDataRemovalDropsInvalidTarget, TestSize.Level1)
{
    ScopedFastScrollAnimationClock clock;
    MockAnimationManager::GetInstance().SetTicks(8);
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateFixedHeightItems(150, 100.0f);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks(frameNode_);
    while (frameNode_->GetChildren().size() > 14) {
        auto child = frameNode_->GetChildren().back();
        frameNode_->RemoveChild(child);
    }
    frameNode_->ChildrenUpdatedFrom(14);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    FlushUITasks(frameNode_);
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    for (int32_t tick = 0; tick < 8; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks(frameNode_);
    }
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    ScrollToIndex(5, false, ScrollAlign::START);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 500.0f);
}

/**
 * @tc.name: FastScrollStaticLayoutStopsAndRecovers
 * @tc.desc: Switching to a non-scrollable Grid drops the animation and later scrollable layout remains usable.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollStaticLayoutStopsAndRecovers, TestSize.Level1)
{
    ScopedFastScrollAnimationClock clock;
    MockAnimationManager::GetInstance().SetTicks(8);
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedHeightItems(150, 100.0f);
    CreateDone();
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    MockAnimationManager::GetInstance().Tick();
    FlushUITasks(frameNode_);
    layoutProperty_->UpdateRowsTemplate("1fr 1fr");
    frameNode_->MarkModifyDone();
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    FlushUITasks(frameNode_);
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    layoutProperty_->ResetRowsTemplate();
    frameNode_->MarkModifyDone();
    frameNode_->ChildrenUpdatedFrom(0);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    FlushUITasks(frameNode_);
    ScrollToIndex(20, false, ScrollAlign::START);
    const float after = pattern_->GetTotalOffset();
    for (int32_t tick = 0; tick < 8; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks(frameNode_);
    }
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), after);
    EXPECT_FLOAT_EQ(pattern_->GetItemRect(20).Top(), 0.0f);
}

/**
 * @tc.name: FastScrollPartialAcceptedMotion
 * @tc.desc: App-reduced movement is applied and reported without forcibly completing the original index request.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollPartialAcceptedMotion, TestSize.Level1)
{
    ScopedFastScrollAnimationClock clock;
    MockAnimationManager::GetInstance().SetTicks(8);
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(150, [](uint32_t) { return 100.0f; });
    CreateDone();
    auto accepted = std::make_shared<float>(0.0f);
    auto reported = std::make_shared<float>(0.0f);
    eventHub_->SetOnWillScroll([accepted](Dimension offset, ScrollState, ScrollSource) {
        ScrollFrameResult result;
        result.offset = Dimension(offset.ConvertToPx() * 0.5f, DimensionUnit::PX);
        *accepted += result.offset.ConvertToPx();
        return result;
    });
    eventHub_->SetOnDidScroll([reported](Dimension offset, ScrollState) { *reported += offset.ConvertToPx(); });
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
        FastScrollMode::SKIP_CREATE_AND_MEASURE);
    FlushUITasks(frameNode_);
    for (int32_t tick = 0; tick < 8; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks(frameNode_);
    }
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_FLOAT_EQ(*accepted, 6000.0f);
    EXPECT_NEAR(*reported, *accepted, 0.1f);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), *accepted);
    const float stopped = pattern_->GetTotalOffset();
    for (int32_t tick = 0; tick < 8; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks(frameNode_);
    }
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), stopped);
    eventHub_->SetOnWillScroll({});
    ScrollToIndex(5, false, ScrollAlign::START);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 500.0f);
}

/**
 * @tc.name: FastScrollLayoutChangesRemainUsable
 * @tc.desc: Changed spans and templates allow the current request to end and a later request to run.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollLayoutChangesRemainUsable, TestSize.Level1)
{
    ScopedFastScrollAnimationClock clock;
    ScopedFastScrollOptionsAlgorithm scope;
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    GridLayoutOptions options;
    options.irregularIndexes = { 39, 40 };
    options.getSizeByIndex = [](int32_t) { return GridItemSize { 1, 2 }; };
    model.SetLayoutOptions(options);
    CreateFixedHeightItems(150, 100.0f);
    CreateDone();
    for (bool changeTemplate : { false, true }) {
        SCOPED_TRACE(changeTemplate);
        MockAnimationManager::GetInstance().SetTicks(8);
        pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
            FastScrollMode::SKIP_CREATE_AND_MEASURE);
        FlushUITasks(frameNode_);
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks(frameNode_);
        ASSERT_TRUE(pattern_->AnimateRunning());
        if (changeTemplate) {
            layoutProperty_->UpdateColumnsTemplate("1fr 1fr 1fr 1fr");
        } else {
            options.getSizeByIndex = [](int32_t) { return GridItemSize { 1, 3 }; };
            layoutProperty_->UpdateLayoutOptions(options);
        }
        frameNode_->ChildrenUpdatedFrom(0);
        frameNode_->MarkModifyDone();
        FlushUITasks(frameNode_);
        for (int32_t tick = 0; tick < 32 && (pattern_->AnimateRunning() || pattern_->GetFastScrollRequest()); ++tick) {
            MockAnimationManager::GetInstance().Tick();
            FlushUITasks(frameNode_);
        }
        EXPECT_FALSE(pattern_->AnimateRunning());
        EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
        EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
        EXPECT_TRUE(std::isfinite(pattern_->GetTotalOffset()));
        ScrollToIndex(0, false, ScrollAlign::START);
        pattern_->ScrollToIndexFastScroll(60, true, ScrollAlign::START, 0.0f, FastScrollMode::SKIP_MEASURE);
        FlushUITasks(frameNode_);
        for (int32_t tick = 0; tick < 32 && (pattern_->AnimateRunning() || pattern_->GetFastScrollRequest()); ++tick) {
            MockAnimationManager::GetInstance().Tick();
            FlushUITasks(frameNode_);
        }
        EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
        EXPECT_NEAR(pattern_->GetItemRect(60).Top(), 0.0f, 0.1f);
        ScrollToIndex(0, false, ScrollAlign::START);
    }
}

/**
 * @tc.name: FastScrollRepeatedRequestsAreInstanceLocal
 * @tc.desc: Repeated completion and cancellation release temporary state without changing another Grid.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollRepeatedRequestsAreInstanceLocal, TestSize.Level1)
{
    ScopedFastScrollAnimationClock clock;
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    CreateFixedHeightItems(150, 100.0f);
    CreateDone();
    auto firstNode = frameNode_;
    auto first = pattern_;
    BuildIndependentGrid();
    ASSERT_NE(firstNode, frameNode_);
    firstNode->MountToParent(GetStageNode());
    FlushUITasks(firstNode);
    for (int32_t round = 0; round < 3; ++round) {
        SCOPED_TRACE(round);
        MockAnimationManager::GetInstance().SetTicks(8);
        const auto mode = round == 1 ? FastScrollMode::SKIP_MEASURE : FastScrollMode::SKIP_CREATE_AND_MEASURE;
        first->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f, mode);
        FlushUITasks(firstNode);
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks(firstNode);
        const float otherOffset = pattern_->GetTotalOffset();
        pattern_->ScrollBy(30.0f);
        FlushUITasks(frameNode_);
        if (round != 1) {
            first->StopAnimate();
        }
        for (int32_t tick = 0; tick < 32; ++tick) {
            MockAnimationManager::GetInstance().Tick();
            FlushUITasks(firstNode);
        }
        EXPECT_FALSE(first->AnimateRunning());
        EXPECT_EQ(first->GetFastScrollRequest(), nullptr);
        EXPECT_FALSE(first->GetGridLayoutInfo().fastScrollTarget_.has_value());
        EXPECT_FALSE(first->GetGridLayoutInfo().fastScrollGeometry_.has_value());
        EXPECT_FALSE(first->fastScrollPendingMovement_.has_value());
        EXPECT_FALSE(first->fastScrollPendingDelta_.has_value());
        EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
        EXPECT_FALSE(pattern_->GetGridLayoutInfo().fastScrollTarget_.has_value());
        EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), otherOffset + 30.0f);
        first->ScrollToIndex(0, false, ScrollAlign::START);
        FlushUITasks(firstNode);
        EXPECT_FLOAT_EQ(first->GetTotalOffset(), 0.0f);
    }
    GetStageNode()->RemoveChild(firstNode);
}

/**
 * @tc.name: FastScrollLegacyWorkReference
 * @tc.desc: Record the existing animated path for the same uniform content and request as the mode1/mode2 fixtures.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollLegacyWorkReference, TestSize.Level1)
{
    MockAnimationManager::GetInstance().SetTicks(8);
    auto created = std::make_shared<std::vector<int32_t>>();
    auto measured = std::make_shared<std::vector<int32_t>>();
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(150, [created, measured](uint32_t index) {
        created->push_back(static_cast<int32_t>(index));
        auto node = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        node->measureCallback_ = std::make_unique<std::function<void(RefPtr<Kit::FrameNode>&)>>(
            [measured, index](RefPtr<Kit::FrameNode>&) { measured->push_back(static_cast<int32_t>(index)); });
        return 100.0f;
    });
    CreateDone();
    created->clear();
    measured->clear();
    pattern_->ScrollToIndexFastScroll(120, true, ScrollAlign::START, 0.0f,
        FastScrollMode::NONE);
    FlushUITasks(frameNode_);
    ASSERT_TRUE(pattern_->AnimateRunning());
    std::vector<float> positions { static_cast<float>(pattern_->GetTotalOffset()) };
    std::vector<std::pair<size_t, size_t>> workCounts { { created->size(), measured->size() } };
    for (int32_t tick = 0; tick < 8; ++tick) {
        MockAnimationManager::GetInstance().Tick();
        FlushUITasks(frameNode_);
        positions.push_back(pattern_->GetTotalOffset());
        workCounts.emplace_back(created->size(), measured->size());
    }
    EXPECT_FALSE(pattern_->AnimateRunning());
    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
    EXPECT_FLOAT_EQ(pattern_->GetTotalOffset(), 12000.0f);
    const auto rect = pattern_->GetItemRect(120);
    ASSERT_GT(rect.Width(), 0.0f);
    EXPECT_FLOAT_EQ(rect.Top(), 0.0f);
    EXPECT_FLOAT_EQ(rect.Height(), 100.0f);
    RecordFrameOffsets(positions);
    RecordWorkObservation(0, *created, *measured, positions, workCounts, 120);
}

/**
 * @tc.name: FastScrollCatalogFarDirections
 * @tc.desc: GF-001/002 use the catalog's three-column content and nonzero forward/reverse starts.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollCatalogFarDirections, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr 1fr 1fr");
    model.SetCachedCount(0);
    ViewAbstract::SetHeight(CalcLength(600.0f));
    CreateItemsInLazyForEach(600, [](uint32_t) { return 100.0f; });
    CreateDone();
    ASSERT_FLOAT_EQ(pattern_->GetMainContentSize(), 600.0f);
    for (auto mode : { FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE }) {
        for (bool reverse : { false, true }) {
            const int32_t start = reverse ? 300 : 60;
            const int32_t target = reverse ? 60 : 300;
            SCOPED_TRACE(std::to_string(static_cast<int32_t>(mode)) + ":" + std::to_string(reverse));
            ScrollToIndex(start, false, ScrollAlign::START);
            MockAnimationManager::GetInstance().SetTicks(8);
            pattern_->ScrollToIndexFastScroll(target, true, ScrollAlign::START, 0.0f, mode);
            FlushUITasks(frameNode_);
            for (int32_t tick = 0;
                tick < MAX_ANIMATION_TICKS && (pattern_->AnimateRunning() || pattern_->GetFastScrollRequest());
                ++tick) {
                MockAnimationManager::GetInstance().Tick();
                FlushUITasks(frameNode_);
            }
            ASSERT_FALSE(pattern_->AnimateRunning());
            ASSERT_EQ(pattern_->GetFastScrollRequest(), nullptr);
            EXPECT_NEAR(pattern_->GetTotalOffset(), target / 3 * 100.0f, 0.1f);
            const auto rect = pattern_->GetItemRect(target);
            EXPECT_NEAR(rect.Top(), 0.0f, 0.1f);
            EXPECT_NEAR(rect.Width(), WIDTH / 3, 0.1f);
            EXPECT_NEAR(rect.Height(), 100.0f, 0.1f);
        }
    }
    RecordProperty("catalog_cases", "GF-001,GF-002");
    RecordProperty("checked_combinations", 4);
}

/**
 * @tc.name: FastScrollCatalogPreciseThreshold
 * @tc.desc: GF-007 checks exact physical distances around four viewports in both directions and modes.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollCatalogPreciseThreshold, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateFixedHeightItems(600, 100.0f);
    CreateDone();
    const float viewport = pattern_->GetMainContentSize();
    for (auto mode : { FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE }) {
        for (float factor : { 0.0f, 1.0f, 3.99f, 4.0f, 4.01f, 10.0f }) {
            for (bool reverse : { false, true }) {
                SCOPED_TRACE(std::to_string(static_cast<int32_t>(mode)) + ":" +
                    std::to_string(factor) + ":" + std::to_string(reverse));
                constexpr int32_t start = 200;
                const float distance = factor * viewport;
                const int32_t itemDelta = static_cast<int32_t>(distance / 100.0f);
                const float sign = reverse ? -1.0f : 1.0f;
                const int32_t target = start + (reverse ? -itemDelta : itemDelta);
                const float extra = sign * (distance - itemDelta * 100.0f);
                ScrollToIndex(start, false, ScrollAlign::START);
                MockAnimationManager::GetInstance().SetTicks(8);
                pattern_->ScrollToIndexFastScroll(target, true, ScrollAlign::START, extra, mode);
                FlushUITasks(frameNode_);
                if (factor == 0.0f) {
                    EXPECT_FALSE(pattern_->AnimateRunning());
                    EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
                } else {
                    ASSERT_NE(pattern_->GetFastScrollRequest(), nullptr);
                    EXPECT_NEAR(pattern_->GetFastScrollRequest()->skipDistance,
                        sign * std::max(0.0f, distance - 4.0f * viewport), 0.1f);
                }
                int32_t skips = 0;
                for (int32_t tick = 0;
                    tick < MAX_ANIMATION_TICKS && (pattern_->AnimateRunning() || pattern_->GetFastScrollRequest());
                    ++tick) {
                    MockAnimationManager::GetInstance().Tick();
                    skips += pattern_->fastScrollSkip_ ? 1 : 0;
                    FlushUITasks(frameNode_);
                }
                EXPECT_EQ(skips, factor > 4.0f ? 1 : 0);
                EXPECT_FALSE(pattern_->AnimateRunning());
                EXPECT_EQ(pattern_->GetFastScrollRequest(), nullptr);
                EXPECT_NEAR(pattern_->GetTotalOffset(), start * 100.0f + sign * distance, 0.1f);
            }
        }
    }
    RecordProperty("catalog_cases", "GF-007");
    RecordProperty("checked_combinations", 24);
}

/**
 * @tc.name: FastScrollCatalogLargeItemOffsets
 * @tc.desc: GF-004/005/013 compare both modes with physical scrolling, including an item larger than the viewport.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollCatalogLargeItemOffsets, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(120, [](uint32_t index) { return index == 60 ? 650.0f : 80.0f + 15.0f * (index % 5); });
    CreateDone();
    const float viewport = pattern_->GetMainContentSize();
    ASSERT_GT(650.0f, viewport);
    std::vector<float> itemTops { 0.0f };
    for (int32_t index = 0; index < 120; ++index) {
        itemTops.push_back(itemTops.back() + (index == 60 ? 650.0f : 80.0f + 15.0f * (index % 5)));
    }
    int32_t combinations = 0;
    for (auto align : { ScrollAlign::START, ScrollAlign::CENTER, ScrollAlign::END, ScrollAlign::AUTO }) {
        for (int32_t target : { 1, 60, 118 }) {
            for (int32_t start : { 0, 90 }) {
                CheckCatalogFactors(itemTops, align, target, start, combinations);
                ASSERT_FALSE(HasFatalFailure());
            }
        }
    }
    RecordProperty("catalog_cases", "GF-004,GF-005,GF-013");
    RecordProperty("geometry_reference", "ordinary ScrollBy steps <=40px; dataset-defined physical anchor");
    RecordProperty("checked_combinations", combinations);
    EXPECT_EQ(combinations, 240);
}

/**
 * @tc.name: FastScrollCatalogRepeatedCycles
 * @tc.desc: GF-028 runs three warmups, twenty completions and twenty cancellations per mode, then legacy scrolling.
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, FastScrollCatalogRepeatedCycles, TestSize.Level1)
{
    auto model = CreateGrid();
    model.SetColumnsTemplate("1fr");
    model.SetCachedCount(0);
    CreateItemsInLazyForEach(150, [](uint32_t) { return 100.0f; });
    CreateDone();
    for (auto mode : { FastScrollMode::SKIP_MEASURE, FastScrollMode::SKIP_CREATE_AND_MEASURE }) {
        for (int32_t round = 0; round < 43; ++round) {
            CheckCatalogCycle(mode, round);
        }
    }
    RecordProperty("catalog_cases", "GF-010,GF-011,GF-028");
    RecordProperty("warmup_requests", 6);
    RecordProperty("completed_requests", 40);
    RecordProperty("cancelled_requests", 40);
    RecordProperty("scope_limit", "request-state cleanup only; allocator/cache memory trend not measured");
}


/**
 * @tc.name: SyncCacheLayoutChildCount001
 * @tc.desc: Test GridPattern sync-cache-in-layout reference counting
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, SyncCacheLayoutChildCount001, TestSize.Level1)
{
    // Arrange
    CreateGrid();
    CreateDone();

    // Act & Assert: no reference held, strategy disabled by default
    EXPECT_FALSE(pattern_->IsSyncCacheLayoutEnabled());
    // each attached Repeat child holds one reference
    pattern_->AddSyncCacheLayoutChild();
    EXPECT_TRUE(pattern_->IsSyncCacheLayoutEnabled());
    pattern_->AddSyncCacheLayoutChild();
    EXPECT_TRUE(pattern_->IsSyncCacheLayoutEnabled());
    // releasing one reference keeps the other one active
    pattern_->RemoveSyncCacheLayoutChild();
    EXPECT_TRUE(pattern_->IsSyncCacheLayoutEnabled());
    // last reference released: strategy disabled, counter clamped at zero
    pattern_->RemoveSyncCacheLayoutChild();
    EXPECT_FALSE(pattern_->IsSyncCacheLayoutEnabled());
    pattern_->RemoveSyncCacheLayoutChild();
    EXPECT_FALSE(pattern_->IsSyncCacheLayoutEnabled());
}

/**
 * @tc.name: IsSyncCacheLayoutEnabledWrapper001
 * @tc.desc: Test GridLayoutBaseAlgorithm::IsSyncCacheLayoutEnabled wrapper resolution chain
 * @tc.type: FUNC
 */
HWTEST_F(GridPatternTestNg, IsSyncCacheLayoutEnabledWrapper001, TestSize.Level1)
{
    // Arrange
    GridLayoutInfo info;
    GridScrollLayoutAlgorithm algorithm(info);

    // Act & Assert: null wrapper
    EXPECT_FALSE(algorithm.IsSyncCacheLayoutEnabled(nullptr));

    // host without a GridPattern
    auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 901, AceType::MakeRefPtr<Pattern>());
    auto columnWrapper = columnNode->CreateLayoutWrapper();
    ASSERT_NE(columnWrapper, nullptr);
    EXPECT_FALSE(algorithm.IsSyncCacheLayoutEnabled(columnWrapper.GetRawPtr()));

    // grid host: disabled by default, enabled while the pattern holds a reference
    CreateGrid();
    CreateDone();
    auto gridWrapper = pattern_->GetHost()->CreateLayoutWrapper();
    ASSERT_NE(gridWrapper, nullptr);
    EXPECT_FALSE(algorithm.IsSyncCacheLayoutEnabled(gridWrapper.GetRawPtr()));
    pattern_->AddSyncCacheLayoutChild();
    EXPECT_TRUE(algorithm.IsSyncCacheLayoutEnabled(gridWrapper.GetRawPtr()));
    pattern_->RemoveSyncCacheLayoutChild();
    EXPECT_FALSE(algorithm.IsSyncCacheLayoutEnabled(gridWrapper.GetRawPtr()));
}
} // namespace OHOS::Ace::NG
