/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "lazy_column_layout_test.h"

#include <vector>

#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/unittest/core/syntax/mock_lazy_for_each_builder.h"
#include "test/unittest/core/pattern/lazy_layout/lazy_layout_test_utils.h"

#include "core/components_ng/pattern/lazy_column_layout/lazy_column_layout_model.h"
#include "core/components_ng/pattern/common_view/common_view_pattern.h"
#include "core/components_ng/pattern/lazy_column_layout/lazy_column_layout_algorithm.h"
#include "core/components_ng/pattern/lazy_layout/lazy_layout_utils.h"
#include "core/components_ng/pattern/lazy_layout/prepared_layout_geometry.h"
#include "core/components_ng/pattern/list/list_model_ng.h"
#include "core/components_ng/pattern/list/list_item_model_ng.h"
#include "core/components_ng/pattern/list/list_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_model_ng.h"
#include "core/components_ng/pattern/scrollable/scrollable_model_ng.h"
#include "core/components_ng/pattern/waterflow/water_flow_item_model_ng.h"
#include "core/components_ng/pattern/waterflow/water_flow_model_ng.h"
#include "core/components_ng/pattern/waterflow/water_flow_pattern.h"
#include "core/components_ng/pattern/waterflow/water_flow_layout_property.h"
#include "core/components_ng/pattern/stack/stack_model_ng.h"
#include "core/components_ng/syntax/lazy_for_each_model_ng.h"
#include "core/components_ng/syntax/repeat_virtual_scroll_2_model_ng.h"
#include "core/components_ng/layout/layout_wrapper_node.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t REJECTED_HEADER_NODE_ID = 4700;
constexpr int32_t REJECTED_ITEM_COUNT = 5;
constexpr int32_t REJECTED_CHILD_NODE_ID = 4701;
constexpr int32_t REJECTED_ITEM_INDEX = 2;
constexpr int32_t COMMITTED_ITEM_COUNT = 8;
constexpr int32_t COMMITTED_NODE_ID = 5100;
constexpr int32_t COMMITTED_TALL_INDEX = 6;
constexpr int32_t COMMITTED_REJECTED_INDEX = 5;
} // namespace


using GetChildByIndexCallback = std::function<std::pair<std::string, RefPtr<NG::UINode>>(int32_t index)>;

class CustomLazyForEachBuilder : public Framework::MockLazyForEachBuilder {
public:
    CustomLazyForEachBuilder(int32_t itemCnt, GetChildByIndexCallback&& callback)
        : itemCnt_(itemCnt), getChildByIndexCallback_(std::move(callback))
    {}

protected:
    int32_t OnGetTotalCount() override
    {
        return itemCnt_;
    }

    std::pair<std::string, RefPtr<NG::UINode>> OnGetChildByIndex(
        int32_t index, std::unordered_map<std::string, NG::LazyForEachCacheChild>& expiringItems) override
    {
        if (getChildByIndexCallback_) {
            return getChildByIndexCallback_(index);
        }
        return { std::to_string(index), nullptr };
    }

private:
    int32_t itemCnt_ = 0;
    GetChildByIndexCallback getChildByIndexCallback_ = nullptr;
};

void LazyColumnLayoutTest::CreateRepeatVirtualScrollNode(
    int32_t itemNumber, const std::function<std::pair<uint32_t, uint32_t>(int32_t, bool, bool)>& onGetRid4Index)
{
    RepeatVirtualScroll2ModelNG repeatModel;
    auto onActiveRange = [](int32_t start, int32_t end, int32_t cacheStart,
        int32_t cacheEnd, bool isCache, bool isImmediate) {};
    auto onPurge = []() {};
    auto onPurgeAll = []() {};
    auto onUpdateDirty = []() {};
    auto onRecycleItems = [](int32_t start, int32_t end) {};
    repeatModel.Create(
        itemNumber,
        itemNumber,
        0,
        onGetRid4Index,
        onRecycleItems,
        onActiveRange,
        nullptr,
        onPurge,
        onPurgeAll,
        onUpdateDirty);
}

void LazyColumnLayoutTest::SetUpTestSuite()
{
    TestNG::SetUpTestSuite();
    MockPipelineContext::GetCurrent()->SetUseFlushUITasks(true);
    MockPipelineContext::GetCurrentContext()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    MockAnimationManager::Enable(true);
}

void LazyColumnLayoutTest::TearDownTestSuite()
{
    TestNG::TearDownTestSuite();
}

void LazyColumnLayoutTest::SetUp() {}

void LazyColumnLayoutTest::TearDown()
{
    RemoveFromStageNode();
    frameNode_ = nullptr;
    pattern_ = nullptr;
    layoutProperty_ = nullptr;
    AceApplicationInfo::GetInstance().isRightToLeft_ = false;
    MockAnimationManager::GetInstance().Reset();
}

/**
 * @tc.name: LazyColumnLayoutPattern_CachedParentMatrix_001
 * @tc.desc: Test cached LazyColumn identity under List, WaterFlow and Scroll before main-tree attachment
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnLayoutPattern_CachedParentMatrix_001, TestSize.Level1)
{
    int32_t nodeId = -1000;
    for (const auto& parentCase : CACHED_LAZY_PARENT_CASES) {
        SCOPED_TRACE(parentCase.name);
        auto context = CreateCachedLazyParentTestContext<LazyColumnLayoutPattern>(
            parentCase, V2::LAZY_COLUMN_LAYOUT_ETS_TAG, nodeId);
        ASSERT_NE(context.parentNode, nullptr);
        ASSERT_NE(context.intermediateNode, nullptr);
        ASSERT_NE(context.lazyNode, nullptr);
        ASSERT_NE(context.layoutWrapper, nullptr);

        auto lazyProperty = context.lazyNode->GetLayoutProperty();
        auto clonedProperty = context.layoutWrapper->GetLayoutProperty();
        ASSERT_NE(lazyProperty, nullptr);
        ASSERT_NE(clonedProperty, nullptr);
        EXPECT_TRUE(context.lazyNode->IsNeedLazyLayout());
        EXPECT_TRUE(lazyProperty->GetNeedLazyLayout());
        EXPECT_TRUE(clonedProperty->GetNeedLazyLayout());

        context.lazyNode->MountToParent(context.intermediateNode, DEFAULT_NODE_SLOT, true);
        EXPECT_FALSE(context.lazyNode->IsOnMainTree());
        EXPECT_TRUE(context.intermediateNode->IsNeedLazyLayout());
        EXPECT_TRUE(context.intermediateNode->GetLayoutProperty()->GetNeedLazyLayout());
        EXPECT_TRUE(LazyLayoutUtils::ValidateAndSetLazyLayoutParent(context.lazyNode, Axis::VERTICAL));
        EXPECT_TRUE(context.lazyNode->IsNeedLazyLayout());
    }
}

/**
 * @tc.name: LazyColumnLayoutCachePrediction001
 * @tc.desc: Cache coverage continues to request prediction independently of FastScroll completion.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnLayoutCachePrediction001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<LazyColumnLayoutPattern>();
    auto frameNode = FrameNode::CreateFrameNode(V2::LAZY_COLUMN_LAYOUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), pattern);
    ASSERT_NE(frameNode, nullptr);
    ASSERT_NE(pattern->layoutInfo_, nullptr);

    auto& layoutInfo = pattern->layoutInfo_;
    layoutInfo->layoutedEnd_ = 100.0f;
    layoutInfo->cacheEndPos_ = 200.0f;
    layoutInfo->layoutedEndIndex_ = 1;
    layoutInfo->totalItemCount_ = 10;
    EXPECT_TRUE(layoutInfo->NeedPredict());

    layoutInfo->layoutedEndIndex_ = layoutInfo->totalItemCount_ - 1;
    EXPECT_FALSE(layoutInfo->NeedPredict());
}

RefPtr<FrameNode> LazyColumnLayoutTest::GetMainFrameNode()
{
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->GetMainElementNode();
    return AceType::DynamicCast<FrameNode>(element);
}

void LazyColumnLayoutTest::CreateLazyColumnLayout(VisibleIndexesChangeEvent&& callback)
{
    LazyColumnLayoutModel::Create();
    if (callback) {
        LazyColumnLayoutModel::SetOnVisibleIndexesChange(std::move(callback));
    }
    frameNode_ = GetMainFrameNode();
    pattern_ = frameNode_->GetPattern<LazyColumnLayoutPattern>();
    layoutProperty_ = frameNode_->GetLayoutProperty<LazyColumnLayoutProperty>();
}

void LazyColumnLayoutTest::CreateLazyVGridLayout()
{
    LazyVGridLayoutModel gridModel;
    gridModel.Create();
    ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    gridModel.SetColumnsTemplate("1fr 1fr");
    lazyGridFrameNode_ = GetMainFrameNode();
    lazyGridPattern_ = lazyGridFrameNode_->GetPattern<LazyGridLayoutPattern>();
    lazyGridPattern_->axis_ = Axis::VERTICAL;
}

void LazyColumnLayoutTest::CreateStack(const CalcLength& width, const CalcLength& height)
{
    StackModelNG stackModel;
    stackModel.Create();
    ViewAbstract::SetWidth(width);
    ViewAbstract::SetHeight(height);
}

void LazyColumnLayoutTest::CreateContent(int32_t count, const CalcLength& width, const CalcLength& height)
{
    for (int32_t i = 0; i < count; i++) {
        CreateStack(width, height);
        ViewStackProcessor::GetInstance()->Pop();
    }
}

void LazyColumnLayoutTest::CreateScroll()
{
    ScrollModelNG model;
    model.Create();
    ViewAbstract::SetHeight(CalcLength(SCROLL_HEIGHT));
    ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    scrollableFrameNode_ = GetMainFrameNode();
    scrollablePattern_ = scrollableFrameNode_->GetPattern<ScrollablePattern>();
}

void LazyColumnLayoutTest::CreateWaterFlow(WaterFlowLayoutMode mode)
{
    WaterFlowModelNG model;
    model.Create();
    ViewAbstract::SetHeight(CalcLength(SCROLL_HEIGHT));
    ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    model.SetLayoutMode(mode);
    scrollableFrameNode_ = GetMainFrameNode();
    scrollablePattern_ = scrollableFrameNode_->GetPattern<ScrollablePattern>();
}

void LazyColumnLayoutTest::CreateFlowItem()
{
    WaterFlowItemModelNG itemModel;
    itemModel.Create();
}

void LazyColumnLayoutTest::CreateList()
{
    ListModelNG model;
    model.Create();
    ViewAbstract::SetHeight(CalcLength(SCROLL_HEIGHT));
    ViewAbstract::SetWidth(CalcLength(SCROLL_WIDTH));
    scrollableFrameNode_ = GetMainFrameNode();
    scrollablePattern_ = scrollableFrameNode_->GetPattern<ScrollablePattern>();
}

void LazyColumnLayoutTest::CreateListItems()
{
    // add 3 list items
    for (int32_t i = 0; i < 3; i++) {
        ListItemModelNG listItemModel;
        listItemModel.Create();
        CreateStack(CalcLength(1, DimensionUnit::PERCENT), CalcLength(50)); // set the height of the list item to 50
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->Pop();
    }
}

void LazyColumnLayoutTest::CreateListWithLazyColumnLayout(int32_t initialIndex, bool stackFromEnd)
{
    CreateList();
    ListModelNG::SetInitialIndex(AceType::RawPtr(scrollableFrameNode_), initialIndex);
    ListModelNG::SetListStackFromEnd(AceType::RawPtr(scrollableFrameNode_), stackFromEnd);
    CreateLazyColumnLayout();
    CreateContent(10);
    ViewStackProcessor::GetInstance()->Pop();
    CreateListItems();
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();
}

PaddingProperty LazyColumnLayoutTest::CreatePadding(float left, float top, float right, float bottom)
{
    PaddingProperty padding;
    padding.left = CalcLength(left);
    padding.right = CalcLength(right);
    padding.top = CalcLength(top);
    padding.bottom = CalcLength(bottom);
    return padding;
}

void LazyColumnLayoutTest::AddChildAtIndex(int32_t index)
{
    CreateStack();
    RefPtr<UINode> currentNode = ViewStackProcessor::GetInstance()->Finish();
    auto currentFrameNode = AceType::DynamicCast<FrameNode>(currentNode);
    frameNode_->AddChild(currentFrameNode, index);
}

void LazyColumnLayoutTest::FlushIdleTask(const RefPtr<LazyColumnLayoutPattern>& pattern)
{
    CHECK_NULL_VOID(pattern);
    int32_t tryCount = 10;
    auto needPredict = pattern->layoutInfo_->NeedPredict();
    while (needPredict && tryCount > 0) {
        const int64_t time = GetSysTimestamp();
        auto pipeline = pattern->GetContext();
        pipeline->OnIdle(time + 16 * 1000000); // 16ms in nanoseconds
        auto host = pattern->GetHost();
        FlushUITasks(host);
        needPredict = pattern->layoutInfo_->NeedPredict();
        tryCount--;
    }
}

void LazyColumnLayoutTest::FlushListIdleTask(const RefPtr<ListPattern>& listPattern)
{
    CHECK_NULL_VOID(listPattern);
    int32_t tryCount = 10;
    auto predictParam = listPattern->GetPredictLayoutParamV2();
    while (predictParam && tryCount > 0) {
        const int64_t time = GetSysTimestamp();
        auto pipeline = listPattern->GetContext();
        pipeline->OnIdle(time + 16 * 1000000); // 16 * 1000000: 16ms
        auto host = listPattern->GetHost();
        FlushUITasks(host);
        predictParam = listPattern->GetPredictLayoutParamV2();
        tryCount--;
    }
}

/**
 * @tc.name: SpaceTest001
 * @tc.desc: Test Space property between items
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, SpaceTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy column layout with 10 items, no space
     * @tc.expected: total main size = 10 * ITEM_HEIGHT = 1000
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 10 * ITEM_HEIGHT);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);

    /**
     * @tc.steps: step2. Update space to 20
     * @tc.expected: total main size = 10 * ITEM_HEIGHT + (10 - 1) * space = 1180
     */
    auto space = 20.0f;
    LazyColumnLayoutModel::SetSpace(AceType::RawPtr(frameNode_), Dimension(space));
    FlushUITasks(frameNode_);
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 10 * ITEM_HEIGHT + 9 * space);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 3);

    /**
     * @tc.steps: step3. Update space to negative value
     * @tc.expected: negative space treated as 0, same as step1
     */
    LazyColumnLayoutModel::SetSpace(AceType::RawPtr(frameNode_), Dimension(-10));
    FlushUITasks(frameNode_);
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 10 * ITEM_HEIGHT);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);
}

/**
 * @tc.name: SpaceTest002
 * @tc.desc: Shrink space repeatedly while scrolled to the bottom; the last item stays bottom-pinned
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, SpaceTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Scroll > LazyColumnLayout with 10 items and space = 30
     * @tc.expected: content height = 10 * ITEM_HEIGHT + 9 * 30 = 1270
     */
    CreateScroll();
    CreateLazyColumnLayout();
    auto space = 30.0f;
    LazyColumnLayoutModel::SetSpace(AceType::RawPtr(frameNode_), Dimension(space));
    CreateContent(10);
    CreateDone();

    /**
     * @tc.steps: step2. Scroll to the bottom
     * @tc.expected: last item (index 9) bottom-aligned, on-screen top y = SCROLL_HEIGHT - ITEM_HEIGHT = 350
     */
    scrollablePattern_->UpdateCurrentOffset(-1000, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 6);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 9);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), SCROLL_HEIGHT - 10 * ITEM_HEIGHT - 9 * space);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 9), 350);

    /**
     * @tc.steps: step3. Shrink space 30 -> 20
     * @tc.expected: last item stays bottom-pinned, on-screen top y stays 350 (no jump)
     */
    space = 20.0f;
    LazyColumnLayoutModel::SetSpace(AceType::RawPtr(frameNode_), Dimension(space));
    FlushUITasks(scrollableFrameNode_);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 6);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 9);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), SCROLL_HEIGHT - 10 * ITEM_HEIGHT - 9 * space);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 9), 350);

    /**
     * @tc.steps: step4. Shrink space 20 -> 10
     * @tc.expected: last item on-screen top y stays 350
     */
    space = 10.0f;
    LazyColumnLayoutModel::SetSpace(AceType::RawPtr(frameNode_), Dimension(space));
    FlushUITasks(scrollableFrameNode_);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 5);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 9);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), SCROLL_HEIGHT - 10 * ITEM_HEIGHT - 9 * space);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 9), 350);

    /**
     * @tc.steps: step5. Shrink space 10 -> 0
     * @tc.expected: last item on-screen top y stays 350
     */
    space = 0.0f;
    LazyColumnLayoutModel::SetSpace(AceType::RawPtr(frameNode_), Dimension(space));
    FlushUITasks(scrollableFrameNode_);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 5);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 9);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), SCROLL_HEIGHT - 10 * ITEM_HEIGHT - 9 * space);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 9), 350);
}

/**
 * @tc.name: AlignItemsTest001
 * @tc.desc: Test HorizontalAlign property (START, CENTER, END)
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, AlignItemsTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy column layout with narrow children, default align is CENTER
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateContent(10, CalcLength(100));
    CreateDone();
    EXPECT_EQ(GetChildWidth(frameNode_, 0), 100);

    /**
     * @tc.steps: step2. Default CENTER align
     * @tc.expected: child x offset is (200 - 100) / 2 = 50
     */
    EXPECT_EQ(GetChildX(frameNode_, 0), 50);

    /**
     * @tc.steps: step3. Update align to START
     * @tc.expected: child x offset is 0
     */
    LazyColumnLayoutModel::SetAlignItems(AceType::RawPtr(frameNode_), HorizontalAlign::START);
    FlushUITasks(frameNode_);
    EXPECT_EQ(GetChildX(frameNode_, 0), 0);

    /**
     * @tc.steps: step4. Update align to END
     * @tc.expected: child x offset is 200 - 100 = 100
     */
    LazyColumnLayoutModel::SetAlignItems(AceType::RawPtr(frameNode_), HorizontalAlign::END);
    FlushUITasks(frameNode_);
    EXPECT_EQ(GetChildX(frameNode_, 0), 100);

    /**
     * @tc.steps: step5. Update align to CENTER
     * @tc.expected: child x offset is (200 - 100) / 2 = 50
     */
    LazyColumnLayoutModel::SetAlignItems(AceType::RawPtr(frameNode_), HorizontalAlign::CENTER);
    FlushUITasks(frameNode_);
    EXPECT_EQ(GetChildX(frameNode_, 0), 50);
}

/**
 * @tc.name: OnVisibleIndexesChangeTest001
 * @tc.desc: Verify callback returns -1,-1 when there are no child items
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, OnVisibleIndexesChangeTest001, TestSize.Level1)
{
    int32_t indexStart = -2; // Initialize to -2 to distinguish from callback value -1
    int32_t indexEnd = -2;
    auto callback = [&indexStart, &indexEnd](int32_t start, int32_t end) {
        indexStart = start;
        indexEnd = end;
    };

    CreateScroll();
    CreateLazyColumnLayout(std::move(callback));
    CreateDone();

    EXPECT_EQ(indexStart, -1);
    EXPECT_EQ(indexEnd, -1);
}

/**
 * @tc.name: OnVisibleIndexesChangeTest002
 * @tc.desc: Verify callback updates when scrolling
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, OnVisibleIndexesChangeTest002, TestSize.Level1)
{
    int32_t indexStart = -1;
    int32_t indexEnd = -1;
    auto callback = [&indexStart, &indexEnd](int32_t start, int32_t end) {
        indexStart = start;
        indexEnd = end;
    };

    CreateScroll();
    CreateLazyColumnLayout(std::move(callback));
    CreateContent(10);
    CreateDone();
    EXPECT_EQ(indexStart, 0);
    EXPECT_EQ(indexEnd, 4);

    /**
     * @tc.steps: step2. Scroll down incrementally: 50 + 50 + 100 = 200
     * @tc.expected: visible range updated via callback after each scroll
     */
    scrollablePattern_->UpdateCurrentOffset(-50, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(indexStart, 0);
    EXPECT_EQ(indexEnd, 5);

    scrollablePattern_->UpdateCurrentOffset(-50, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(indexStart, 0);
    EXPECT_EQ(indexEnd, 5);

    scrollablePattern_->UpdateCurrentOffset(-100, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(indexStart, 1);
    EXPECT_EQ(indexEnd, 6);
}

/**
 * @tc.name: OnVisibleIndexesChangeTest003
 * @tc.desc: Verify OnVisibleIndexesChange callback for two inner LazyColumnLayouts
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, OnVisibleIndexesChangeTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Scroll > LazyColumnLayout > two LazyColumnLayouts with callbacks
     * @tc.expected: first inner LazyColumnLayout callback returns 0, 4;
     *               second inner LazyColumnLayout callback returns -1, -1 (not visible)
     */
    int32_t indexStart1 = -2; // Initialize to -2 to distinguish from callback value -1
    int32_t indexEnd1 = -2;
    auto callback1 = [&indexStart1, &indexEnd1](int32_t start, int32_t end) {
        indexStart1 = start;
        indexEnd1 = end;
    };
    int32_t indexStart2 = -2; // Initialize to -2 to distinguish from callback value -1
    int32_t indexEnd2 = -2;
    auto callback2 = [&indexStart2, &indexEnd2](int32_t start, int32_t end) {
        indexStart2 = start;
        indexEnd2 = end;
    };

    CreateScroll();
    CreateLazyColumnLayout();
    CreateLazyColumnLayout(std::move(callback1));
    CreateContent(5);
    ViewStackProcessor::GetInstance()->Pop();
    CreateLazyColumnLayout(std::move(callback2));
    CreateContent(5);
    ViewStackProcessor::GetInstance()->Pop();
    ViewStackProcessor::GetInstance()->Pop();
    CreateDone();
    auto frameNode = GetChildFrameNode(scrollableFrameNode_, 0);
    auto pattern = frameNode->GetPattern<LazyColumnLayoutPattern>();

    FlushIdleTask(pattern);
    EXPECT_EQ(indexStart1, 0);
    EXPECT_EQ(indexEnd1, 4);
    EXPECT_EQ(indexStart2, -1);
    EXPECT_EQ(indexEnd2, -1);

    /**
     * @tc.steps: step2. Scroll to where both LazyColumnLayouts are visible
     * @tc.expected: both callbacks return valid visible ranges
     */
    scrollablePattern_->UpdateCurrentOffset(-300, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(indexStart1, 2);
    EXPECT_EQ(indexEnd1, 4);
    EXPECT_EQ(indexStart2, 0);
    EXPECT_EQ(indexEnd2, 2);

    /**
     * @tc.steps: step3. Scroll until first LazyColumnLayout is out of view
     * @tc.expected: first inner LazyColumnLayout callback returns -1, -1 (not visible);
     *               second inner LazyColumnLayout callback returns 0, 4
     */
    scrollablePattern_->UpdateCurrentOffset(-300, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    FlushIdleTask(pattern);
    EXPECT_EQ(indexStart1, -1);
    EXPECT_EQ(indexEnd1, -1);
    EXPECT_EQ(indexStart2, 0);
    EXPECT_EQ(indexEnd2, 4);
}

/**
 * @tc.name: PaddingMarginTest001
 * @tc.desc: Test padding and margin effects on layout
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, PaddingMarginTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy column layout
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();

    /**
     * @tc.steps: step2. Assert start and end index before padding
     * @tc.expected: startIndex and endIndex are correct before padding
     */
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);

    /**
     * @tc.steps: step3. Update padding top and bottom
     * @tc.expected: visible range reduced due to padding
     */
    PaddingProperty padding = CreatePadding(0, 100, 0, 100);
    layoutProperty_->UpdatePadding(padding);
    FlushUITasks(frameNode_);
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 1200);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 3);

    /**
     * @tc.steps: step4. Update margin top and bottom
     * @tc.expected: visible range further reduced
     */
    PaddingProperty margin = CreatePadding(0, 100, 0, 100);
    layoutProperty_->UpdateMargin(margin);
    FlushUITasks(frameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 2);
}

/**
 * @tc.name: RTLTest001
 * @tc.desc: Test right-to-left layout with START, CENTER, END alignment
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, RTLTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy column layout with narrow children, align START
     */
    CreateScroll();
    CreateLazyColumnLayout();
    LazyColumnLayoutModel::SetAlignItems(AceType::RawPtr(frameNode_), HorizontalAlign::START);
    CreateContent(10, CalcLength(100));
    CreateDone();

    /**
     * @tc.steps: step2. Verify LTR START alignment
     * @tc.expected: child starts at x=0
     */
    EXPECT_EQ(GetChildX(frameNode_, 0), 0);

    /**
     * @tc.steps: step3. Update to RTL direction with START alignment
     * @tc.expected: child position mirrored to x=100
     */
    layoutProperty_->UpdateLayoutDirection(TextDirection::RTL);
    FlushUITasks(frameNode_);
    EXPECT_EQ(GetChildX(frameNode_, 0), 100);

    /**
     * @tc.steps: step4. RTL with CENTER alignment
     * @tc.expected: child centered at x=50
     */
    LazyColumnLayoutModel::SetAlignItems(AceType::RawPtr(frameNode_), HorizontalAlign::CENTER);
    FlushUITasks(frameNode_);
    EXPECT_EQ(GetChildX(frameNode_, 0), 50);

    /**
     * @tc.steps: step5. RTL with END alignment
     * @tc.expected: child at x=0
     */
    LazyColumnLayoutModel::SetAlignItems(AceType::RawPtr(frameNode_), HorizontalAlign::END);
    FlushUITasks(frameNode_);
    EXPECT_EQ(GetChildX(frameNode_, 0), 0);
}

/**
 * @tc.name: WidthHeightTest001
 * @tc.desc: Test user-defined width and height
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, WidthHeightTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy column layout
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();

    /**
     * @tc.steps: step2. Update width and height
     * @tc.expected: dimensions updated
     */
    auto size = CalcSize(CalcLength(160), CalcLength(750));
    layoutProperty_->UpdateUserDefinedIdealSize(size);
    FlushUITasks(frameNode_);
    EXPECT_EQ(GetChildWidth(scrollableFrameNode_, 0), 160);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 750);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);

    /**
     * @tc.steps: step3. Reset user defined ideal size
     * @tc.expected: dimensions return to default
     */
    layoutProperty_->ClearUserDefinedIdealSize(true, true);
    FlushUITasks(frameNode_);
    EXPECT_EQ(GetChildWidth(scrollableFrameNode_, 0), SCROLL_WIDTH);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 10 * ITEM_HEIGHT);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);
}

/**
 * @tc.name: SizeConstraintTest001
 * @tc.desc: Test min/max size constraints
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, SizeConstraintTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy column layout
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();

    /**
     * @tc.steps: step2. Update max size
     * @tc.expected: dimensions constrained
     */
    auto size = CalcSize(CalcLength(160), CalcLength(750));
    layoutProperty_->UpdateCalcMaxSize(size);
    FlushUITasks(frameNode_);
    EXPECT_EQ(GetChildWidth(scrollableFrameNode_, 0), 160);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 750);

    /**
     * @tc.steps: step3. Reset max size, update min size
     * @tc.expected: dimensions constrained by min
     */
    layoutProperty_->ResetCalcMaxSize();
    size = CalcSize(CalcLength(240), CalcLength(1800));
    layoutProperty_->UpdateCalcMinSize(size);
    FlushUITasks(frameNode_);
    EXPECT_EQ(GetChildWidth(scrollableFrameNode_, 0), 240);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 1800);

    /**
     * @tc.steps: step4. Reset min size
     * @tc.expected: dimensions return to default
     */
    layoutProperty_->ResetCalcMinSize();
    FlushUITasks(frameNode_);
    EXPECT_EQ(GetChildWidth(scrollableFrameNode_, 0), SCROLL_WIDTH);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 10 * ITEM_HEIGHT);
}

/**
 * @tc.name: LayoutPolicyTest001
 * @tc.desc: Test measure result when setting matchParent
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LayoutPolicyTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create LazyColumnLayout with MATCH_PARENT layout policy
     */
    RefPtr<FrameNode> lazyColumn;
    auto column = CreateColumn([this, &lazyColumn](ColumnModelNG model) {
        ViewAbstract::SetWidth(CalcLength(500));
        ViewAbstract::SetHeight(CalcLength(300));
        LazyColumnLayoutModel::Create();
        ViewAbstractModelNG viewModel;
        viewModel.UpdateLayoutPolicyProperty(LayoutCalPolicy::MATCH_PARENT, true);
        viewModel.UpdateLayoutPolicyProperty(LayoutCalPolicy::MATCH_PARENT, false);
        RefPtr<UINode> element = ViewStackProcessor::GetInstance()->GetMainElementNode();
        ViewStackProcessor::GetInstance()->PopContainer();
        lazyColumn = AceType::DynamicCast<FrameNode>(element);
    });
    ViewStackProcessor::GetInstance()->Finish();
    ASSERT_NE(column, nullptr);
    ASSERT_EQ(column->GetChildren().size(), 1);
    CreateLayoutTask(column);

    auto geometryNode = lazyColumn->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    auto size = geometryNode->GetFrameSize();
    auto offset = geometryNode->GetFrameOffset();
    EXPECT_EQ(size, SizeF(500.0f, 300.0f));
    EXPECT_EQ(offset, OffsetF(0.0f, 0.0f));
}

/**
 * @tc.name: AddDelChildrenTest001
 * @tc.desc: Test adding and removing children
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, AddDelChildrenTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy column layout with 10 items
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();
    EXPECT_EQ(pattern_->layoutInfo_->totalMainSize_, 10 * ITEM_HEIGHT);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 10);

    /**
     * @tc.steps: step2. Remove 5 children
     * @tc.expected: totalMainSize and totalItemCount updated
     */
    for (int32_t i = 0; i < 5; i++) {
        frameNode_->RemoveChildAtIndex(0);
    }
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(frameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->totalMainSize_, 5 * ITEM_HEIGHT);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 5);

    /**
     * @tc.steps: step3. Add 6 children
     * @tc.expected: totalMainSize and totalItemCount updated
     */
    for (int32_t i = 0; i < 6; i++) {
        AddChildAtIndex(0);
    }
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(frameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->totalMainSize_, 11 * ITEM_HEIGHT);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 11);
}

/**
 * @tc.name: AddDelChildrenTest002
 * @tc.desc: Test adding and removing all children
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, AddDelChildrenTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create empty lazy column layout
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateDone();
    EXPECT_EQ(pattern_->layoutInfo_->totalMainSize_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 0);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 0);

    /**
     * @tc.steps: step2. Add 6 children
     * @tc.expected: totalMainSize and totalItemCount updated
     */
    for (int32_t i = 0; i < 6; i++) {
        AddChildAtIndex(0);
    }
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(frameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->totalMainSize_, 6 * ITEM_HEIGHT);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 6);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 6 * ITEM_HEIGHT);

    /**
     * @tc.steps: step3. Remove all children
     * @tc.expected: totalMainSize and totalItemCount reset to 0
     */
    for (int32_t i = 0; i < 6; i++) {
        frameNode_->RemoveChildAtIndex(0);
    }
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(frameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->totalMainSize_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 0);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 0);
}

/**
 * @tc.name: AddDelChildrenTest003
 * @tc.desc: Test onVisibleIndexesChange when deleting items while scrolled
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, AddDelChildrenTest003, TestSize.Level1)
{
    int32_t indexStart = -2;
    int32_t indexEnd = -2;
    auto callback = [&indexStart, &indexEnd](int32_t start, int32_t end) {
        indexStart = start;
        indexEnd = end;
    };

    /**
     * @tc.steps: step1. Create WaterFlow > LazyColumnLayout with 10 items and onVisibleIndexesChange callback
     * @tc.expected: callback fires with range 0-4
     */
    CreateWaterFlow();
    CreateLazyColumnLayout(std::move(callback));
    CreateContent(10);
    CreateDone();

    EXPECT_EQ(indexStart, 0);
    EXPECT_EQ(indexEnd, 4);

    /**
     * @tc.steps: step2. Scroll down 150px
     * @tc.expected: visible range updated to 1-6, scroll offset is -150
     */
    scrollablePattern_->UpdateCurrentOffset(-150, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 10);
    EXPECT_EQ(indexStart, 1);
    EXPECT_EQ(indexEnd, 6);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), -150);

    /**
     * @tc.steps: step3. Delete child at index 0 (above visible area)
     * @tc.expected: totalItemCount decreases to 9, visible range and scroll offset unchanged
     */
    frameNode_->RemoveChildAtIndex(0);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 9);
    EXPECT_EQ(indexStart, 1);
    EXPECT_EQ(indexEnd, 6);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), -150);

    /**
     * @tc.steps: step4. Scroll to bottom by additional -300px (total -450px)
     * @tc.expected: visible range updated to 4-8
     */
    scrollablePattern_->UpdateCurrentOffset(-300, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(indexStart, 4);
    EXPECT_EQ(indexEnd, 8);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), -450);

    /**
     * @tc.steps: step5. Delete child at index 0 (above visible area) again
     * @tc.expected: totalItemCount decreases to 8, visible range recalculated to 3-7,
     *               scroll offset adjusted from -450 to -350
     */
    frameNode_->RemoveChildAtIndex(0);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 8);
    EXPECT_EQ(indexStart, 3);
    EXPECT_EQ(indexEnd, 7);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), -350);
}

/**
 * @tc.name: AddDelChildrenTest004
 * @tc.desc: Test onVisibleIndexesChange when add items while scrolled
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, AddDelChildrenTest004, TestSize.Level1)
{
    int32_t indexStart = -2;
    int32_t indexEnd = -2;
    auto callback = [&indexStart, &indexEnd](int32_t start, int32_t end) {
        indexStart = start;
        indexEnd = end;
    };

    /**
     * @tc.steps: step1. Create WaterFlow > LazyColumnLayout with 10 items and onVisibleIndexesChange callback
     * @tc.expected: callback fires with range 0-4
     */
    CreateWaterFlow();
    CreateLazyColumnLayout(std::move(callback));
    CreateContent(10);
    CreateDone();

    EXPECT_EQ(indexStart, 0);
    EXPECT_EQ(indexEnd, 4);

    /**
     * @tc.steps: step2. Scroll down 150px
     * @tc.expected: visible range updated to 1-6, child 1 relative y offset is -50
     */
    scrollablePattern_->UpdateCurrentOffset(-150, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 10);
    EXPECT_EQ(indexStart, 1);
    EXPECT_EQ(indexEnd, 6);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 1), -50);

    /**
     * @tc.steps: step3. Add child at the top (index 0, above visible area)
     * @tc.expected: totalItemCount increases to 11, visible range and child 1 relative position unchanged
     */
    AddChildAtIndex(0);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 11);
    EXPECT_EQ(indexStart, 1);
    EXPECT_EQ(indexEnd, 6);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 1), -50);

    /**
     * @tc.steps: step4. Add child at the end (index 11, below visible area)
     * @tc.expected: totalItemCount increases to 12, visible range and child 1 relative position unchanged
     */
    AddChildAtIndex(11);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 12);
    EXPECT_EQ(indexStart, 1);
    EXPECT_EQ(indexEnd, 6);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 1), -50);
}

/**
 * @tc.name: AddDelChildrenTest005
 * @tc.desc: Repeatedly delete bottom children while scrolled to the bottom; the last item stays bottom-pinned
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, AddDelChildrenTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Scroll > LazyColumnLayout with 20 items
     * @tc.expected: content height = 20 * ITEM_HEIGHT = 2000, scrollable range = 2000 - SCROLL_HEIGHT = 1550
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateContent(20);
    CreateDone();

    /**
     * @tc.steps: step2. Scroll to the bottom
     * @tc.expected: visible range 15-19, scroll offset = SCROLL_HEIGHT - 20 * ITEM_HEIGHT = -1550,
     *               last item bottom-aligned (on-screen top y = SCROLL_HEIGHT - ITEM_HEIGHT = 350)
     */
    scrollablePattern_->UpdateCurrentOffset(-2000, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 20);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 15);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 19);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), -1550);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 19), 350);

    /**
     * @tc.steps: step3. Delete the last 5 (bottom) children, then flush once
     * @tc.expected: the 15 remaining items become the whole content and the scroll re-clamps to the new
     *               bottom; totalItemCount = 15, visible 10-14, offset = SCROLL_HEIGHT - 15 * ITEM_HEIGHT = -1050,
     *               last item on-screen top y stays 350
     */
    for (int32_t i = 0; i < 5; i++) {
        frameNode_->RemoveChild(frameNode_->GetLastChild());
    }
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(scrollableFrameNode_);
    FlushIdleTask(pattern_);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 15);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 10);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 14);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), -1050);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 14), 350);

    /**
     * @tc.steps: step4. Delete the last 5 (bottom) children, then flush once
     * @tc.expected: the 10 remaining items become the whole content and the scroll re-clamps to the new
     *               bottom; totalItemCount = 10, visible 5-9, offset = SCROLL_HEIGHT - 10 * ITEM_HEIGHT = -550,
     *               last item on-screen top y stays 350
     */
    for (int32_t i = 0; i < 5; i++) {
        frameNode_->RemoveChild(frameNode_->GetLastChild());
    }
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(scrollableFrameNode_);
    FlushIdleTask(pattern_);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 10);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 5);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 9);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), -550);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 9), 350);

    /**
     * @tc.steps: step5. Delete the last 5 (bottom) children, then flush once
     * @tc.expected: the 5 remaining items become the whole content and the scroll re-clamps to the new
     *               bottom; totalItemCount = 5, visible 0-4, offset = SCROLL_HEIGHT - 5 * ITEM_HEIGHT = -50,
     *               last item on-screen top y stays 350
     */
    for (int32_t i = 0; i < 5; i++) {
        frameNode_->RemoveChild(frameNode_->GetLastChild());
    }
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(scrollableFrameNode_);
    FlushIdleTask(pattern_);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->totalItemCount_, 5);
    EXPECT_EQ(pattern_->layoutInfo_->visibleStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->visibleEndIndex_, 4);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), -50);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 4), 350);
}

/**
 * @tc.name: UpdateChildSizeTest001
 * @tc.desc: Test child position after dynamically modifying child size while scrolled
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, UpdateChildSizeTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create WaterFlow > LazyColumnLayout with 10 items (each 100px height)
     */
    CreateWaterFlow();
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();

    /**
     * @tc.steps: step2. Scroll down 150px (child 0 partially visible, child 1 visible)
     * @tc.expected: child 1 relative y offset (WaterFlow y + LazyColumnLayout y) is -50
     */
    scrollablePattern_->UpdateCurrentOffset(-150, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 1), -50);

    /**
     * @tc.steps: step3. Shrink child 0 height from 100 to 50 (above visible area)
     * @tc.expected: child 1 relative y offset unchanged (-50), position is stable
     */
    auto child0 = GetChildFrameNode(frameNode_, 0);
    auto child0LayoutProp = child0->GetLayoutProperty();
    child0LayoutProp->UpdateUserDefinedIdealSize(CalcSize(CalcLength(1, DimensionUnit::PERCENT), CalcLength(50)));
    child0->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 1), -50);

    /**
     * @tc.steps: step4. Grow child 0 height from 50 to 200 (above visible area)
     * @tc.expected: child 1 relative y offset unchanged (-50), position is stable
     */
    child0LayoutProp->UpdateUserDefinedIdealSize(CalcSize(CalcLength(1, DimensionUnit::PERCENT), CalcLength(200)));
    child0->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 1), -50);

    /**
     * @tc.steps: step5. Grow child 2 height from 100 to 200 (within visible area)
     * @tc.expected: child 1 relative y offset unchanged (-50), position is stable
     */
    auto child2 = GetChildFrameNode(frameNode_, 2);
    auto child2LayoutProp = child2->GetLayoutProperty();
    child2LayoutProp->UpdateUserDefinedIdealSize(CalcSize(CalcLength(1, DimensionUnit::PERCENT), CalcLength(200)));
    child2->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0) + GetChildY(frameNode_, 1), -50);
}

/**
 * @tc.name: NeedPredictTest001
 * @tc.desc: Test NeedPredict returns correct value
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, NeedPredictTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy column layout
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();

    /**
     * @tc.steps: step2. After initial layout, check NeedPredict
     * @tc.expected: NeedPredict should be true when there are items beyond cache
     */
    bool needPredict = pattern_->layoutInfo_->NeedPredict();
    EXPECT_TRUE(needPredict);

    /**
     * @tc.steps: step3. After flushing idle tasks, check NeedPredict
     * @tc.expected: NeedPredict should be false when all cache is filled
     */
    FlushIdleTask(pattern_);
    needPredict = pattern_->layoutInfo_->NeedPredict();
    EXPECT_FALSE(needPredict);
}

/**
 * @tc.name: LazyColumnInScroll001
 * @tc.desc: Verify Scroll with LazyColumnLayout startIndex/endIndex and cached range
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInScroll001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Scroll with LazyColumnLayout and 10 items
     * @tc.expected: layout visible items and predict items
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();

    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 6);

    /**
     * @tc.steps: step2. Scroll down
     * @tc.expected: start and end index updated
     */
    EXPECT_NE(scrollablePattern_, nullptr);
    scrollablePattern_->UpdateCurrentOffset(-200, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 1);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 6);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 8);

    /**
     * @tc.steps: step3. Scroll to bottom
     * @tc.expected: last items visible
     */
    scrollablePattern_->UpdateCurrentOffset(-350, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 5);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 3);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 9);

    /**
     * @tc.steps: step4. Scroll back to top
     * @tc.expected: start and end index restored to initial values
     */
    scrollablePattern_->UpdateCurrentOffset(550, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 6);
}

/**
 * @tc.name: LazyColumnInScrollWithScrollDirection001
 * @tc.desc: Verify Scroll with different scrollDirection property
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInScrollWithScrollDirection001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create vertical Scroll with LazyColumnLayout
     * @tc.expected: LazyColumnLayout implements lazy loading
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();
    EXPECT_NE(scrollablePattern_, nullptr);
    EXPECT_EQ(scrollablePattern_->GetAxis(), Axis::VERTICAL);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);

    /**
     * @tc.steps: step2. Set Scroll axis to HORIZONTAL
     * @tc.expected: LazyColumnLayout fully expanded without lazy loading
     */
    ScrollModelNG::SetAxis(AceType::RawPtr(scrollableFrameNode_), Axis::HORIZONTAL);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
}

/**
 * @tc.name: LazyColumnInScrollWithInitialOffset001
 * @tc.desc: Verify Scroll with initialOffset affects LazyColumnLayout startIndex/endIndex
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInScrollWithInitialOffset001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Scroll with initialOffset and LazyColumnLayout
     * @tc.expected: LazyColumnLayout starts from offset position with correct indexes
     */
    CreateScroll();
    ScrollModelNG::SetInitialOffset(
        AceType::RawPtr(scrollableFrameNode_),
        OffsetT<CalcDimension>(CalcDimension(0), CalcDimension(200)));
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();

    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 1);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 6);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 8);

    /**
     * @tc.steps: step2. Scroll down further
     * @tc.expected: start and end index updated correctly
     */
    EXPECT_NE(scrollablePattern_, nullptr);
    scrollablePattern_->UpdateCurrentOffset(-200, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 3);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 8);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 1);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 9);
}

/**
 * @tc.name: LazyColumnInList001
 * @tc.desc: Verify List with LazyColumnLayout startIndex/endIndex and cached range
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInList001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List with LazyColumnLayout and 10 items
     * @tc.expected: layout visible items and predict items
     */
    CreateList();
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();

    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 6);

    /**
     * @tc.steps: step2. Scroll down
     * @tc.expected: start and end index updated
     */
    EXPECT_NE(scrollablePattern_, nullptr);
    scrollablePattern_->UpdateCurrentOffset(-200, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 1);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 6);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 8);

    /**
     * @tc.steps: step3. Scroll to bottom
     * @tc.expected: last items visible
     */
    scrollablePattern_->UpdateCurrentOffset(-350, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 5);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 3);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 9);

    /**
     * @tc.steps: step4. Scroll back to top
     * @tc.expected: start and end index restored to initial values
     */
    scrollablePattern_->UpdateCurrentOffset(550, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 6);
}

/**
 * @tc.name: LazyColumnInListWithInitialIndex001
 * @tc.desc: Verify List with initialIndex=0, child 0 (first LazyColumnLayout) at y=0
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInListWithInitialIndex001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List with initialIndex=0
     *            List children: LazyColumnLayout(1000px) + 3 ListItems(50px each) + LazyColumnLayout(1000px)
     * @tc.expected: List has 5 children, child 0 at y=0
     */
    CreateListWithLazyColumnLayout(0);
    EXPECT_NE(scrollableFrameNode_, nullptr);
    auto children = scrollableFrameNode_->GetChildren();
    EXPECT_GE(children.size(), 5u);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), 0);
}

/**
 * @tc.name: LazyColumnInListWithInitialIndex002
 * @tc.desc: Verify List with initialIndex=1, child 1 (first ListItem) at y=0
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInListWithInitialIndex002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List with initialIndex=1
     * @tc.expected: child 1 at y=0
     */
    CreateListWithLazyColumnLayout(1);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 1), 0);

    /**
     * @tc.steps: step2. Flush List idle task and check child 0's lazy layout range
     * @tc.expected: child 0's startIndex=-1, endIndex=-1 (not in viewport)
     */
    FlushListIdleTask(scrollableFrameNode_->GetPattern<ListPattern>());
    auto child0 = GetChildFrameNode(scrollableFrameNode_, 0);
    auto pattern0 = child0->GetPattern<LazyColumnLayoutPattern>();
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, -1);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, -1);

    /**
     * @tc.steps: step3. Flush child 0's idle task to trigger cache layout
     * @tc.expected: child 0's cachedStartIndex=7, cachedEndIndex=9
     */
    FlushIdleTask(pattern0);
    EXPECT_EQ(pattern0->layoutInfo_->cachedStartIndex_, 7);
    EXPECT_EQ(pattern0->layoutInfo_->cachedEndIndex_, 9);

    /**
     * @tc.steps: step4. Check child 4's lazy layout range
     * @tc.expected: child 4's startIndex=0, endIndex=3
     */
    auto child4 = GetChildFrameNode(scrollableFrameNode_, 4);
    auto pattern4 = child4->GetPattern<LazyColumnLayoutPattern>();
    EXPECT_EQ(pattern4->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern4->layoutInfo_->endIndex_, 3);

    /**
     * @tc.steps: step5. Flush child 4's idle task to trigger cache layout
     * @tc.expected: child 4's cachedStartIndex=0, cachedEndIndex=5
     */
    FlushIdleTask(pattern4);
    EXPECT_EQ(pattern4->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern4->layoutInfo_->cachedEndIndex_, 5);
}

/**
 * @tc.name: LazyColumnInListWithInitialIndex003
 * @tc.desc: Verify List with initialIndex=3, child 3 (third ListItem) at y=0
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInListWithInitialIndex003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List with initialIndex=3
     * @tc.expected: List scrolls to child 3 (third ListItem, 50px) at y=0
     */
    CreateListWithLazyColumnLayout(3);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 3), 0);
}

/**
 * @tc.name: LazyColumnInListWithInitialIndex004
 * @tc.desc: Verify List with initialIndex=4, child 4 (second LazyColumnLayout) at y=0
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInListWithInitialIndex004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List with initialIndex=4
     * @tc.expected: List scrolls to child 4 (second LazyColumnLayout, 1000px) at y=0
     */
    CreateListWithLazyColumnLayout(4);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 4), 0);
}

/**
 * @tc.name: LazyColumnInListWithStackFromEnd001
 * @tc.desc: Verify List with stackFromEnd=true and initialIndex=0, child 0 bottom-aligned at y=-550
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInListWithStackFromEnd001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List with stackFromEnd=true, initialIndex=0
     * @tc.expected: child 0 (first LazyColumnLayout, 1000px) bottom-aligned,
     *               bottom edge at viewport bottom (y=450), top at y = 450 - 1000 = -550
     */
    CreateListWithLazyColumnLayout(0, true);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 0), -550);
}

/**
 * @tc.name: LazyColumnInListWithStackFromEnd002
 * @tc.desc: Verify List with stackFromEnd=true and initialIndex=1, child 1 bottom-aligned at y=400
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInListWithStackFromEnd002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List with stackFromEnd=true, initialIndex=1
     * @tc.expected: child 1 (first ListItem, 50px) bottom-aligned,
     *               bottom edge at viewport bottom (y=450), top at y = 450 - 50 = 400
     */
    CreateListWithLazyColumnLayout(1, true);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 1), 400);
}

/**
 * @tc.name: LazyColumnInListWithStackFromEnd003
 * @tc.desc: Verify List with stackFromEnd=true and initialIndex=3, child 3 bottom-aligned at y=400
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInListWithStackFromEnd003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List with stackFromEnd=true, initialIndex=3
     * @tc.expected: child 3 (third ListItem, 50px) bottom-aligned,
     *               bottom edge at viewport bottom (y=450), top at y = 450 - 50 = 400
     */
    CreateListWithLazyColumnLayout(3, true);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 3), 400);
}

/**
 * @tc.name: LazyColumnInListWithStackFromEnd004
 * @tc.desc: Verify List with stackFromEnd=true and initialIndex=4, child 4 bottom-aligned at y=-550
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInListWithStackFromEnd004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List with stackFromEnd=true, initialIndex=4
     * @tc.expected: child 4 (second LazyColumnLayout, 1000px) bottom-aligned,
     *               bottom edge at viewport bottom (y=450), top at y = 450 - 1000 = -550
     */
    CreateListWithLazyColumnLayout(4, true);
    EXPECT_EQ(GetChildY(scrollableFrameNode_, 4), -550);
}

/**
 * @tc.name: LazyColumnInListWithLanes001
 * @tc.desc: Verify LazyColumnLayout in List with lanes=2
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInListWithLanes001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List with lanes=2 and LazyColumnLayout with 10 items
     * @tc.expected: layout all items
     */
    CreateList();
    ListModelNG::SetLanes(AceType::RawPtr(scrollableFrameNode_), 2);
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();

    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
}

/**
 * @tc.name: LazyColumnInWaterFlow001
 * @tc.desc: Test basic lazy layout in WaterFlow container
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInWaterFlow001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy column layout in WaterFlow
     * @tc.expected: layout visible items and predict items
     */
    CreateWaterFlow();
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Width(), SCROLL_WIDTH);
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 10 * ITEM_HEIGHT);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 6);

    /**
     * @tc.steps: step2. Scroll down
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-200, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 1);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 6);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 8);

    /**
     * @tc.steps: step3. Scroll to bottom
     * @tc.expected: last items visible
     */
    scrollablePattern_->UpdateCurrentOffset(-350, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 5);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 3);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 9);

    /**
     * @tc.steps: step4. Scroll back to top
     * @tc.expected: start and end index restored to initial values
     */
    scrollablePattern_->UpdateCurrentOffset(550, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 6);
}

/**
 * @tc.name: LazyColumnInWaterFlow002
 * @tc.desc: Test lazy column in WaterFlow SW mode with space change above display area
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInWaterFlow002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create two lazy columns in WaterFlow SW mode
     */
    CreateWaterFlow(WaterFlowLayoutMode::SLIDING_WINDOW);
    CreateLazyColumnLayout();
    CreateContent(10);
    ViewStackProcessor::GetInstance()->Pop();
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();

    /**
     * @tc.steps: step2. Scroll to second lazy column
     * @tc.expected: first column out of view
     */
    scrollablePattern_->UpdateCurrentOffset(-1200, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    auto pattern1 = GetChildPattern<LazyColumnLayoutPattern>(scrollableFrameNode_, 1);
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, 4);

    /**
     * @tc.steps: step3. Update first column space and scroll backward
     * @tc.expected: scroll offset adjusted by space change
     */
    auto pattern0 = GetChildPattern<LazyColumnLayoutPattern>(scrollableFrameNode_, 0);
    auto hostNode = pattern0->GetHost();
    LazyColumnLayoutModel::SetSpace(AceType::RawPtr(hostNode), Dimension(20));
    scrollablePattern_->UpdateCurrentOffset(400, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 6);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 9);
}

/**
 * @tc.name: LazyColumnInWaterFlow003
 * @tc.desc: Test lazy column with variable item heights in WaterFlow
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInWaterFlow003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy column with variable height items
     */
    CreateWaterFlow(WaterFlowLayoutMode::SLIDING_WINDOW);
    CreateLazyColumnLayout();
    std::array<float, 3> itemHeights = { 80, 100, 90 };
    for (int32_t i = 0; i < 10; i++) {
        StackModelNG stackModel;
        stackModel.Create();
        ViewAbstract::SetWidth(CalcLength(1, DimensionUnit::PERCENT));
        ViewAbstract::SetHeight(CalcLength(itemHeights[i % 3]));
        ViewStackProcessor::GetInstance()->Pop();
    }
    CreateDone();
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 5);

    /**
     * @tc.steps: step2. Scroll down
     * @tc.expected: visible range updated correctly
     */
    scrollablePattern_->UpdateCurrentOffset(-350, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_GE(pattern_->layoutInfo_->startIndex_, 2);
    EXPECT_LE(pattern_->layoutInfo_->startIndex_, 4);
}

/**
 * @tc.name: NestedLazyColumnLayout001
 * @tc.desc: Test nested LazyColumnLayout in Scroll container
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, NestedLazyColumnLayout001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Scroll > LazyColumnLayout > LazyColumnLayout > 10 items
     * @tc.expected: inner LazyColumnLayout layout visible items and predict items
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateLazyColumnLayout();
    CreateContent(10);
    CreateDone();

    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 6);

    /**
     * @tc.steps: step2. Scroll down
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-200, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 1);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 6);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 8);
}

/**
 * @tc.name: NestedLazyGridLayout001
 * @tc.desc: Test nested LazyVGridLayout inside LazyColumnLayout in Scroll container
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, NestedLazyGridLayout001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create Scroll > LazyColumnLayout > LazyVGridLayout > 30 items
     * @tc.expected: LazyVGridLayout layout visible items and predict items
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateLazyVGridLayout();
    CreateContent(30);
    CreateDone();

    /**
     * @tc.steps: step2. Verify LazyVGridLayout visible range
     * @tc.expected: 2 lanes, startIndex=0, endIndex=9
     */
    EXPECT_EQ(lazyGridPattern_->layoutInfo_->lanes_, 2);
    EXPECT_EQ(lazyGridPattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(lazyGridPattern_->layoutInfo_->endIndex_, 9);

    /**
     * @tc.steps: step3. Flush idle task and verify cached range
     * @tc.expected: cachedStartIndex=0, cachedEndIndex=13
     */
    int32_t tryCount = 10;
    while (lazyGridPattern_->layoutInfo_->NeedPredict() && tryCount > 0) {
        const int64_t time = GetSysTimestamp();
        auto pipeline = lazyGridPattern_->GetContext();
        pipeline->OnIdle(time + 16 * 1000000);
        FlushUITasks(lazyGridFrameNode_);
        tryCount--;
    }
    EXPECT_EQ(lazyGridPattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(lazyGridPattern_->layoutInfo_->cachedEndIndex_, 13);
}

/**
 * @tc.name: EdgeCase001
 * @tc.desc: Verify LazyColumnLayout with single item
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, EdgeCase001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create LazyColumnLayout with single item
     * @tc.expected: startIndex and endIndex are 0, totalMainSize equals ITEM_HEIGHT
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateContent(1);
    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->totalMainSize_, ITEM_HEIGHT);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), ITEM_HEIGHT);
}

/**
 * @tc.name: EdgeCase002
 * @tc.desc: Verify LazyColumnLayout with no items
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, EdgeCase002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create LazyColumnLayout with no items
     * @tc.expected: startIndex and endIndex are -1, totalMainSize is 0
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateDone();
    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, -1);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, -1);
    EXPECT_EQ(pattern_->layoutInfo_->totalMainSize_, 0);
    EXPECT_EQ(GetChildHeight(scrollableFrameNode_, 0), 0);
}

/**
 * @tc.name: LazyForEachTest001
 * @tc.desc: Test LazyColumnLayout with LazyForEach generated children.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyForEachTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy column layout with LazyForEach
     * @tc.expected: layout visible items and predict items
     */
    CreateScroll();
    CreateLazyColumnLayout();
    auto mockLazy = AceType::MakeRefPtr<CustomLazyForEachBuilder>(10,
        [](int32_t index) -> std::pair<std::string, RefPtr<NG::UINode>> {
            StackModelNG stackModel;
            stackModel.Create();
            ViewAbstract::SetWidth(CalcLength(1, DimensionUnit::PERCENT));
            ViewAbstract::SetHeight(CalcLength(ITEM_HEIGHT));
            auto node = ViewStackProcessor::GetInstance()->Finish();
            return { std::to_string(index), node };
        }
    );
    LazyForEachModelNG lazyForEachModelNG;
    RefPtr<NG::LazyForEachBuilder> builder = mockLazy;
    lazyForEachModelNG.Create(builder);
    CreateDone();
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 10 * ITEM_HEIGHT);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 6);
    EXPECT_EQ(mockLazy->startIndex_, 0);
    EXPECT_EQ(mockLazy->endIndex_, 6);

    /**
     * @tc.steps: step2. Scroll down
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-200, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 1);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 6);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 8);
    EXPECT_EQ(mockLazy->startIndex_, 0);
    EXPECT_EQ(mockLazy->endIndex_, 8);
}

/**
 * @tc.name: RepeatVirtualScrollTest001
 * @tc.desc: Test LazyColumnLayout with Repeat generated children.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, RepeatVirtualScrollTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create lazy column layout with RepeatVirtualScroll2
     * @tc.expected: layout visible items and predict items
     */
    CreateScroll();
    CreateLazyColumnLayout();
    CreateRepeatVirtualScrollNode(10,
        [](uint32_t index, bool inAnimation, bool restoreCache) -> std::pair<uint32_t, uint32_t> {
            StackModelNG stackModel;
            stackModel.Create();
            ViewAbstract::SetWidth(CalcLength(1, DimensionUnit::PERCENT));
            ViewAbstract::SetHeight(CalcLength(ITEM_HEIGHT));
            ViewStackProcessor::GetInstance()->Pop();
            return { 1, OnGetRid4IndexResult::CREATED_NEW_NODE };
        }
    );
    CreateDone();
    auto repeat = AceType::DynamicCast<RepeatVirtualScroll2Node>(frameNode_->GetChildAtIndex(0));
    ASSERT_NE(repeat, nullptr);
    EXPECT_EQ(frameNode_->GetGeometryNode()->GetFrameSize().Height(), 10 * ITEM_HEIGHT);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 4);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 6);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->layoutedEndIndex_, 6);
    EXPECT_EQ(repeat->prevActiveRangeStart_, 0);
    EXPECT_EQ(repeat->prevActiveRangeEnd_, 6);

    /**
     * @tc.steps: step2. Scroll down
     * @tc.expected: start and end index updated
     */
    scrollablePattern_->UpdateCurrentOffset(-200, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 1);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 6);
    FlushIdleTask(pattern_);
    EXPECT_EQ(pattern_->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->cachedEndIndex_, 8);
    EXPECT_EQ(repeat->prevActiveRangeStart_, 0);
    EXPECT_EQ(repeat->prevActiveRangeEnd_, 8);
}

/**
 * @tc.name: LazyColumnInFlowItem001
 * @tc.desc: Verify LazyColumnLayout inside WaterFlow FlowItem works with lazy loading
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, LazyColumnInFlowItem001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create WaterFlow with 4 FlowItems, each containing LazyColumnLayout
     *           with 5 child items. This tests the nested scenario where LazyColumnLayout
     *           acts as the content of WaterFlow FlowItems.
     * @tc.expected: WaterFlow created with 4 FlowItems, each containing a LazyColumnLayout child
     */
    CreateWaterFlow(WaterFlowLayoutMode::TOP_DOWN);
    for (int32_t i = 0; i < 4; i++) {
        CreateFlowItem();
        CreateLazyColumnLayout();
        CreateContent(5);
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->Pop();
    }
    CreateDone();

    EXPECT_NE(scrollableFrameNode_, nullptr);
    EXPECT_GE(scrollableFrameNode_->GetChildren().size(), 1u);

    /**
     * @tc.steps: step2. Check first FlowItem's LazyColumnLayout visible range.
     *           Verify that only the first FlowItem is laid out with all 5 items visible.
     * @tc.expected: First LazyColumnLayout visible range is [0, 4]
     */
    auto flowItem0 = GetChildFrameNode(scrollableFrameNode_, 0);
    ASSERT_NE(flowItem0, nullptr);
    auto lazyColumn0 = GetChildFrameNode(flowItem0, 0);
    ASSERT_NE(lazyColumn0, nullptr);
    auto pattern0 = lazyColumn0->GetPattern<LazyColumnLayoutPattern>();
    ASSERT_NE(pattern0, nullptr);
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 4);

    /**
     * @tc.steps: step3. Scroll WaterFlow down by 200px to reveal the second FlowItem.
     *           The first LazyColumnLayout loses its first item from viewport,
     *           the second LazyColumnLayout becomes partially visible.
     * @tc.expected: First LazyColumnLayout visible range shifts to [1, 4],
     *               second LazyColumnLayout visible range is [0, 1].
     *               After idle task, cache coverage extends:
     *               first [0, 4], second [0, 3].
     */
    scrollablePattern_->UpdateCurrentOffset(-200, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);
    auto flowItem1 = GetChildFrameNode(scrollableFrameNode_, 1);
    ASSERT_NE(flowItem1, nullptr);
    auto lazyColumn1 = GetChildFrameNode(flowItem1, 0);
    ASSERT_NE(lazyColumn1, nullptr);
    auto pattern1 = lazyColumn1->GetPattern<LazyColumnLayoutPattern>();
    ASSERT_NE(pattern1, nullptr);
    EXPECT_EQ(pattern0->layoutInfo_->startIndex_, 1);
    EXPECT_EQ(pattern0->layoutInfo_->endIndex_, 4);
    EXPECT_EQ(pattern1->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern1->layoutInfo_->endIndex_, 1);
    FlushIdleTask(pattern0);
    FlushIdleTask(pattern1);
    EXPECT_EQ(pattern0->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern0->layoutInfo_->cachedEndIndex_, 4);
    EXPECT_EQ(pattern1->layoutInfo_->cachedStartIndex_, 0);
    EXPECT_EQ(pattern1->layoutInfo_->cachedEndIndex_, 3);
}

/**
 * @tc.name: VisibleIndexWithBoundaryPadding001
 * @tc.desc: Test List with LazyColumnLayout, ContentClip BOUNDARY mode and scroll
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, VisibleIndexWithBoundaryPadding001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List with LazyColumnLayout, content clip and top/bottom padding
     */
    int32_t visibleStart = -1;
    int32_t visibleEnd = -1;

    CreateList();
    ScrollableModelNG::SetContentClip(AceType::RawPtr(scrollableFrameNode_), ContentClipMode::BOUNDARY, nullptr);
    auto listLayoutProperty = scrollableFrameNode_->GetLayoutProperty();
    ASSERT_NE(listLayoutProperty, nullptr);
    PaddingProperty padding = CreatePadding(0, 100, 0, 150);
    listLayoutProperty->UpdatePadding(padding);

    auto callback = [&visibleStart, &visibleEnd](int32_t start, int32_t end) {
        visibleStart = start;
        visibleEnd = end;
    };
    CreateLazyColumnLayout(std::move(callback));
    CreateContent(10);
    CreateDone();

    /**
     * @tc.steps: step2. Check visible indexes
     * @tc.expected: visible indexes should exclude safe area padding
     */
    EXPECT_EQ(visibleStart, 0);
    EXPECT_EQ(visibleEnd, 2);
    EXPECT_EQ(visibleStart, pattern_->layoutInfo_->startIndex_);
    EXPECT_LE(visibleEnd, pattern_->layoutInfo_->endIndex_);

    /**
     * @tc.steps: step3. Scroll and check visible indexes
     * @tc.expected: visible indexes should update correctly after scroll
     */
    scrollablePattern_->UpdateCurrentOffset(-210, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);

    EXPECT_EQ(visibleStart, 2);
    EXPECT_EQ(visibleEnd, 4);
    EXPECT_GT(visibleStart, pattern_->layoutInfo_->startIndex_);
    EXPECT_LE(visibleEnd, pattern_->layoutInfo_->endIndex_);
}

/**
 * @tc.name: VisibleIndexWithSafeAreaPadding001
 * @tc.desc: Test List with LazyColumnLayout and ContentClip SAFE_AREA mode
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, VisibleIndexWithSafeAreaPadding001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List with LazyColumnLayout and SAFE_AREA content clip
     */
    int32_t visibleStart = -1;
    int32_t visibleEnd = -1;

    CreateList();
    ScrollableModelNG::SetContentClip(AceType::RawPtr(scrollableFrameNode_), ContentClipMode::SAFE_AREA, nullptr);
    auto listLayoutProperty = scrollableFrameNode_->GetLayoutProperty();
    ASSERT_NE(listLayoutProperty, nullptr);
    PaddingProperty safeAreaPadding;
    safeAreaPadding.top = std::make_optional<CalcLength>(100);
    safeAreaPadding.bottom = std::make_optional<CalcLength>(150);
    listLayoutProperty->UpdateSafeAreaPadding(safeAreaPadding);

    auto callback = [&visibleStart, &visibleEnd](int32_t start, int32_t end) {
        visibleStart = start;
        visibleEnd = end;
    };
    CreateLazyColumnLayout(std::move(callback));
    CreateContent(10);
    CreateDone();

    /**
     * @tc.steps: step2. Check visible indexes
     * @tc.expected: visible indexes should exclude safe area padding
     */
    EXPECT_EQ(visibleStart, 0);
    EXPECT_EQ(visibleEnd, 2);
    EXPECT_EQ(visibleStart, pattern_->layoutInfo_->startIndex_);
    EXPECT_LE(visibleEnd, pattern_->layoutInfo_->endIndex_);

    /**
     * @tc.steps: step3. Scroll and check visible indexes
     * @tc.expected: visible indexes should update correctly after scroll
     */
    scrollablePattern_->UpdateCurrentOffset(-210, SCROLL_FROM_UPDATE);
    FlushUITasks(scrollableFrameNode_);

    EXPECT_EQ(visibleStart, 2);
    EXPECT_EQ(visibleEnd, 4);
    EXPECT_GT(visibleStart, pattern_->layoutInfo_->startIndex_);
    EXPECT_LE(visibleEnd, pattern_->layoutInfo_->endIndex_);
}

/**
 * @tc.name: FirstFrameWindowSeedsWithUnknownBody001
 * @tc.desc: First layout with an already-scrolled viewport (body extent unknown): the forward window must fall back
 *           to measuring from item 0 (no past-the-end clamp), and the backward window seeds the last item at body 0.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, FirstFrameWindowSeedsWithUnknownBody001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    info->totalItemCount_ = 10;
    LazyColumnLayoutAlgorithm algorithm(info);
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
 * @tc.name: EstimateLayoutWhenDetached001
 * @tc.desc: Test fresh, reused and short detached LazyColumn hosts select the correct measure mode.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, EstimateLayoutWhenDetached001, TestSize.Level1)
{
    auto layoutInfo = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    auto algorithm = AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(layoutInfo);
    algorithm->totalItemCount_ = 10;
    algorithm->hadMeasuredItems_ = false;
    auto context = CreateCachedLazyLayoutTestContext(V2::COLUMN_ETS_TAG,
        AceType::MakeRefPtr<LazyColumnLayoutPattern>(), algorithm, SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    ASSERT_NE(context.host, nullptr);
    ASSERT_NE(context.wrapper, nullptr);
    EXPECT_FALSE(context.host->IsOnMainTree());
    EXPECT_TRUE(context.host->IsNeedLazyLayout());
    ASSERT_NE(context.host->GetParentFrameNode(), nullptr);
    EXPECT_EQ(context.host->GetParentFrameNode()->GetTag(), V2::WATERFLOW_ETS_TAG);

    std::optional<ViewPosReference> posRef;
    algorithm->UpdatePosReference(AceType::RawPtr(context.wrapper), posRef);
    EXPECT_TRUE(algorithm->isEstimatePass_);
    EXPECT_FALSE(algorithm->needSkipLayout_);
    EXPECT_TRUE(layoutInfo->posMap_.empty());

    layoutInfo->posMap_[0] = { 0.0f, 40.0f };
    layoutInfo->posMap_[1] = { 44.0f, 104.0f };
    layoutInfo->totalMainSize_ = 500.0f;
    algorithm->hadMeasuredItems_ = true;
    const auto previousFirstItem = layoutInfo->posMap_.at(0);
    context.host->AttachToMainTree();
    context.host->DetachFromMainTree();
    algorithm->UpdatePosReference(AceType::RawPtr(context.wrapper), posRef);
    EXPECT_FALSE(algorithm->isEstimatePass_);
    EXPECT_TRUE(algorithm->needSkipLayout_);
    EXPECT_FALSE(algorithm->needAllLayout_);
    EXPECT_FLOAT_EQ(layoutInfo->posMap_.at(0).startPos, previousFirstItem.startPos);
    EXPECT_FLOAT_EQ(layoutInfo->posMap_.at(0).endPos, previousFirstItem.endPos);
    EXPECT_FLOAT_EQ(layoutInfo->totalMainSize_, 500.0f);

    algorithm->totalItemCount_ = 1;
    algorithm->UpdatePosReference(AceType::RawPtr(context.wrapper), posRef);
    EXPECT_FALSE(algorithm->isEstimatePass_);
    EXPECT_FALSE(algorithm->needSkipLayout_);
}

/**
 * @tc.name: EstimateLayoutWithParentReference001
 * @tc.desc: Test LazyColumn estimates only fresh hosts inside the parent predictive range.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, EstimateLayoutWithParentReference001, TestSize.Level1)
{
    auto layoutInfo = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    auto algorithm = AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(layoutInfo);
    algorithm->totalItemCount_ = 10;
    algorithm->hadMeasuredItems_ = false;
    auto context = CreateCachedLazyLayoutTestContext(V2::COLUMN_ETS_TAG,
        AceType::MakeRefPtr<LazyColumnLayoutPattern>(), algorithm, SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    ASSERT_NE(context.wrapper, nullptr);
    std::optional<ViewPosReference> posRef = ViewPosReference {
        .viewPosStart = 0.0f,
        .viewPosEnd = SCROLL_HEIGHT,
        .referencePos = SCROLL_HEIGHT,
        .referenceEdge = ReferenceEdge::START,
        .axis = Axis::VERTICAL,
        .deadline = 1,
    };

    algorithm->UpdatePosReference(AceType::RawPtr(context.wrapper), posRef);
    EXPECT_TRUE(algorithm->isEstimatePass_);
    algorithm->hadMeasuredItems_ = true;
    algorithm->UpdatePosReference(AceType::RawPtr(context.wrapper), posRef);
    EXPECT_FALSE(algorithm->isEstimatePass_);
    EXPECT_TRUE(layoutInfo->deadline_.has_value());

    algorithm->hadMeasuredItems_ = false;
    posRef->referencePos = SCROLL_HEIGHT * 2.0f;
    algorithm->UpdatePosReference(AceType::RawPtr(context.wrapper), posRef);
    EXPECT_FALSE(algorithm->isEstimatePass_);
}

/**
 * @tc.name: EstimateItemsTotalHeight001
 * @tc.desc: Test LazyColumn samples at most two items and extrapolates the complete detached host height.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, EstimateItemsTotalHeight001, TestSize.Level1)
{
    auto layoutInfo = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    LazyColumnLayoutAlgorithm algorithm(layoutInfo);
    algorithm.totalItemCount_ = 10;
    algorithm.space_ = 4.0f;

    auto host = FrameNode::CreateFrameNode(
        V2::LAZY_COLUMN_LAYOUT_ETS_TAG, -2000, AceType::MakeRefPtr<LazyColumnLayoutPattern>());
    auto wrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        host, AceType::MakeRefPtr<GeometryNode>(), host->GetLayoutProperty());
    int32_t measureCount = 0;
    AppendFixedHeightChild(wrapper, -2001, 40.0f, measureCount);
    AppendFixedHeightChild(wrapper, -2002, 60.0f, measureCount);
    AppendFixedHeightChild(wrapper, -2003, 200.0f, measureCount);

    algorithm.MeasureEstimateItems(AceType::RawPtr(wrapper));

    EXPECT_EQ(measureCount, 2);
    EXPECT_EQ(layoutInfo->posMap_.size(), 2);
    EXPECT_FLOAT_EQ(layoutInfo->totalMainSize_, 536.0f);
    EXPECT_FLOAT_EQ(layoutInfo->estimateItemSize_, -1.0f);
}
/**
 * @tc.name: PremeasureContainerMeasureLifecycle001
 * @tc.desc: The real container Measure callback runs without publishing preparation geometry.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, PremeasureContainerMeasureLifecycle001, TestSize.Level1)
{
    CreateScroll();
    CreateLazyColumnLayout();
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
 * @tc.name: PremeasureSuffixKeepsDisplayedAnchor001
 * @tc.desc: A suffix keeps the live head and its pending adjustment unchanged while storing real tail sizes.
 * @tc.type: FUNC
 */
class LazyColumnHistoryPremeasureTest : public LazyColumnLayoutTest, public testing::WithParamInterface<bool> {};

/**
 * @tc.name: PremeasureRefreshesHistory001
 * @tc.desc: Current spacing and child count replace historical values without publishing a new window.
 * @tc.type: FUNC
 */
HWTEST_P(LazyColumnHistoryPremeasureTest, PremeasureRefreshesHistory001, TestSize.Level1)
{
    CreateScroll();
    CreateLazyColumnLayout();
    CreateContent(30);
    CreateDone();
    auto info = pattern_->layoutInfo_;
    ASSERT_FALSE(info->posMap_.empty());
    const auto published = frameNode_->GetGeometryNode();
    const auto frameRect = published->GetFrameRect();
    const auto visible = std::make_pair(info->visibleStartIndex_, info->visibleEndIndex_);
    if (GetParam()) {
        layoutProperty_->UpdateSpace(Dimension(10.0f, DimensionUnit::PX));
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
    const int32_t count = GetParam() ? 30 : 25;
    const float space = GetParam() ? 10.0f : 0.0f;
    EXPECT_EQ(info->totalItemCount_, count);
    EXPECT_FLOAT_EQ(info->space_, space);
    EXPECT_FLOAT_EQ(result->marginMainSize, count * ITEM_HEIGHT + (count - 1) * space);
    EXPECT_FLOAT_EQ(info->posMap_.at(count - 1).endPos, info->totalMainSize_);
    EXPECT_EQ(info->posMap_.rbegin()->first, count - 1);
    EXPECT_EQ(std::make_pair(info->visibleStartIndex_, info->visibleEndIndex_), visible);
    EXPECT_EQ(frameNode_->GetGeometryNode(), published);
    EXPECT_EQ(published->GetFrameRect(), frameRect);
}

INSTANTIATE_TEST_SUITE_P(CurrentProperties, LazyColumnHistoryPremeasureTest, testing::Bool());

HWTEST_F(LazyColumnLayoutTest, PremeasureSuffixKeepsDisplayedAnchor001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    auto algorithm = AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(info);
    auto context = CreateCachedLazyLayoutTestContext(V2::LAZY_COLUMN_LAYOUT_ETS_TAG,
        AceType::MakeRefPtr<LazyColumnLayoutPattern>(), algorithm, SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    int32_t measured = 0;
    for (int32_t index = 0; index < 10; ++index) {
        AppendFixedHeightChild(context.wrapper, -4200 - index, index < 6 ? 50.0f : 225.0f, measured);
    }
    info->SetTotalItemCount(10);
    info->SetPosMap(0, { 0.0f, 50.0f });
    info->UpdatePosMap(0.0f);
    info->startIndex_ = 0;
    info->endIndex_ = 0;
    info->visibleStartIndex_ = 0;
    info->visibleEndIndex_ = 0;
    info->adjustOffset_ = { 3.0f, 4.0f };
    context.wrapper->GetGeometryNode()->SetFrameSize(SizeF(SCROLL_WIDTH, 500.0f));
    auto active = context.wrapper->GetOrCreateChildByIndex(0, false, true);
    active->SetActive(true);
    auto constraint = context.parentConstraint;
    constraint.maxSize.SetHeight(Infinity<float>());
    constraint.viewPosRef = ViewPosReference {
        .viewPosStart = -900.0f, .viewPosEnd = 0.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::END, .axis = Axis::VERTICAL,
    };
    auto result = LazyLayoutUtils::Premeasure(context.wrapper, constraint);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->complete);
    EXPECT_FALSE(result->reachedOppositeEdge);
    EXPECT_EQ(measured, 4);
    EXPECT_FLOAT_EQ(result->marginMainSize, 1200.0f);
    EXPECT_FLOAT_EQ(info->totalMainSize_ - info->posMap_.at(6).startPos, 900.0f);
    EXPECT_FLOAT_EQ(result->adjust.start, 0.0f);
    EXPECT_FLOAT_EQ(result->adjust.end, 700.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.start, 3.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.end, 4.0f);
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 0);
    EXPECT_EQ(info->visibleStartIndex_, 0);
    EXPECT_EQ(info->visibleEndIndex_, 0);
    EXPECT_TRUE(active->IsActive());
}

class LazyColumnRepeatedPremeasureTest : public LazyColumnLayoutTest,
    public testing::WithParamInterface<std::tuple<bool, bool>> {};

/**
 * @tc.name: RepeatedPremeasureSeparatesAnchorAndInset
 * @tc.desc: Internal anchor changes are incremental, whereas unpublished outer padding and margin remain absolute.
 * @tc.type: FUNC
 */
HWTEST_P(LazyColumnRepeatedPremeasureTest, RepeatedPremeasureSeparatesAnchorAndInset, TestSize.Level1)
{
    const auto [forward, decorated] = GetParam();
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    auto context = CreateCachedLazyLayoutTestContext(V2::LAZY_COLUMN_LAYOUT_ETS_TAG,
        AceType::MakeRefPtr<LazyColumnLayoutPattern>(), AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(info),
        SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    int32_t measured = 0;
    for (int32_t index = 0; index < 10; ++index) {
        AppendFixedHeightChild(context.wrapper, -5200 - index, index < 5 ? 100.0f : 500.0f, measured);
    }
    info->totalItemCount_ = 10;
    info->estimateItemSize_ = 100.0f;
    info->totalMainSize_ = 1000.0f;
    info->posMap_[7] = { 700.0f, 800.0f };
    info->startIndex_ = info->visibleStartIndex_ = info->endIndex_ = info->visibleEndIndex_ = 7;
    context.wrapper->GetGeometryNode()->SetFrameSize(SizeF(SCROLL_WIDTH, 1000.0f));
    if (decorated) {
        PaddingProperty padding;
        padding.top = CalcLength(13.0f);
        padding.bottom = CalcLength(19.0f);
        MarginProperty margin;
        margin.top = CalcLength(11.0f);
        margin.bottom = CalcLength(17.0f);
        context.wrapper->GetLayoutProperty()->UpdatePadding(padding);
        context.wrapper->GetLayoutProperty()->UpdateMargin(margin);
    }
    context.parentConstraint.maxSize.SetHeight(Infinity<float>());
    context.parentConstraint.viewPosRef = ViewPosReference {
        .viewPosStart = forward ? 0.0f : -10000.0f, .viewPosEnd = forward ? 10000.0f : 0.0f,
        .referencePos = 0.0f, .referenceEdge = forward ? ReferenceEdge::START : ReferenceEdge::END,
        .axis = Axis::VERTICAL,
    };
    for (int32_t attempt = 0; attempt < 3; ++attempt) {
        SCOPED_TRACE(attempt);
        auto result = LazyLayoutUtils::Premeasure(context.wrapper, context.parentConstraint);
        ASSERT_TRUE(result && result->complete);
        EXPECT_FLOAT_EQ(result->startInsetAdjustment, decorated ? 24.0f : 0.0f);
        EXPECT_FLOAT_EQ(result->adjust.start - result->startInsetAdjustment, attempt == 0 ? 800.0f : 0.0f);
        EXPECT_FLOAT_EQ(result->marginMainSize, decorated ? 3060.0f : 3000.0f);
        EXPECT_FLOAT_EQ(result->adjust.start + result->adjust.end, decorated ? 2060.0f : 2000.0f);
    }
}

INSTANTIATE_TEST_SUITE_P(ReferenceEdges, LazyColumnRepeatedPremeasureTest,
    testing::Combine(testing::Bool(), testing::Bool()));

/**
 * @tc.name: PremeasureThroughCommonView001
 * @tc.desc: A legal intermediate forwards the range and includes its padding without activating children.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, PremeasureThroughCommonView001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    auto algorithm = AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(info);
    auto context = CreateCachedLazyLayoutTestContext(V2::LAZY_COLUMN_LAYOUT_ETS_TAG,
        AceType::MakeRefPtr<LazyColumnLayoutPattern>(), algorithm, SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    auto host = FrameNode::CreateFrameNode(V2::COMMON_VIEW_ETS_TAG, -4300, AceType::MakeRefPtr<CommonViewPattern>());
    host->SetNeedLazyLayout(true);
    host->GetLayoutProperty()->UpdatePadding(CreatePadding(0.0f, 10.0f, 0.0f, 20.0f));
    context.parent->RemoveChild(context.host);
    host->AddChild(context.host);
    auto wrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        host, AceType::MakeRefPtr<GeometryNode>(), host->GetLayoutProperty());
    wrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(AceType::MakeRefPtr<BoxLayoutAlgorithm>()));
    wrapper->AppendChild(context.wrapper);
    context.wrapper->SetActive(false);
    int32_t measured = 0;
    for (int32_t index = 0; index < 10; ++index) {
        AppendFixedHeightChild(context.wrapper, -4301 - index, index < 2 ? 100.0f : 225.0f, measured);
    }
    auto constraint = context.parentConstraint;
    constraint.maxSize.SetHeight(Infinity<float>());
    constraint.viewPosRef = ViewPosReference {
        .viewPosStart = 0.0f, .viewPosEnd = 210.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::START, .axis = Axis::VERTICAL,
    };
    auto result = LazyLayoutUtils::Premeasure(wrapper, constraint);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->complete);
    EXPECT_FALSE(result->reachedOppositeEdge);
    EXPECT_EQ(measured, 2);
    EXPECT_FLOAT_EQ(result->marginMainSize, 1030.0f);
    EXPECT_FALSE(context.wrapper->IsActive());
    EXPECT_EQ(info->visibleStartIndex_, -1);
    EXPECT_EQ(info->visibleEndIndex_, -1);
    constraint.viewPosRef->knownMainSize = 2030.0f;
    result = LazyLayoutUtils::Premeasure(wrapper, constraint);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->complete);
    EXPECT_FLOAT_EQ(result->marginMainSize, 2030.0f);
    EXPECT_FLOAT_EQ(info->totalMainSize_, 2000.0f);
    EXPECT_EQ(info->posMap_.size(), 2);
}

namespace {
std::pair<RefPtr<FrameNode>, RefPtr<LayoutWrapperNode>> CreateNestedColumnPremeasureWrapper(
    const CachedLazyLayoutTestContext& inner, const RefPtr<LazyColumnLayoutInfo>& outerInfo)
{
    constexpr int32_t outerNodeId = -4400;
    auto host = FrameNode::CreateFrameNode(
        V2::LAZY_COLUMN_LAYOUT_ETS_TAG, outerNodeId, AceType::MakeRefPtr<LazyColumnLayoutPattern>());
    inner.parent->RemoveChild(inner.host);
    host->AddChild(inner.host);
    auto outer = AceType::MakeRefPtr<LayoutWrapperNode>(
        host, AceType::MakeRefPtr<GeometryNode>(), host->GetLayoutProperty());
    outer->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(
        AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(outerInfo)));
    outer->AppendChild(inner.wrapper);
    return { host, outer };
}
} // namespace

/**
 * @tc.name: PremeasureNestedSuffixIsNotWholeColumn001
 * @tc.desc: A partially measured nested section must not make its one-item parent report an exact whole extent.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, PremeasureNestedSuffixIsNotWholeColumn001, TestSize.Level1)
{
    auto innerInfo = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    auto inner = CreateCachedLazyLayoutTestContext(V2::LAZY_COLUMN_LAYOUT_ETS_TAG,
        AceType::MakeRefPtr<LazyColumnLayoutPattern>(), AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(innerInfo),
        SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    auto outerInfo = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    auto [host, outer] = CreateNestedColumnPremeasureWrapper(inner, outerInfo);
    int32_t measured = 0;
    for (int32_t index = 0; index < 10; ++index) {
        AppendFixedHeightChild(inner.wrapper, -4401 - index, 100.0f, measured);
    }
    auto constraint = inner.parentConstraint;
    constraint.maxSize.SetHeight(Infinity<float>());
    constraint.viewPosRef = ViewPosReference {
        .viewPosStart = -200.0f, .viewPosEnd = 0.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::END, .axis = Axis::VERTICAL,
    };
    auto result = LazyLayoutUtils::Premeasure(outer, constraint);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->complete);
    EXPECT_FALSE(result->reachedOppositeEdge);
    EXPECT_EQ(measured, 2);
    EXPECT_FLOAT_EQ(result->marginMainSize, 1000.0f);
    EXPECT_FLOAT_EQ(outer->GetGeometryNode()->GetFrameSize().Height(), 0.0f);
    EXPECT_EQ(innerInfo->posMap_.begin()->first, 8);
    EXPECT_EQ(innerInfo->visibleStartIndex_, -1);
    EXPECT_EQ(outerInfo->visibleStartIndex_, -1);
    ASSERT_NE(result->geometry, nullptr);
    ASSERT_EQ(result->geometry->children.count(0), 1u);
    EXPECT_EQ(result->geometry->children.at(0)->positions.begin()->first, 8);

    innerInfo->estimateItemSize_ = 50.0f;
    inner.wrapper->GetOrCreateChildByIndex(7, false, true)->GetLayoutAlgorithm()->SetLayoutAlgorithm(nullptr);
    constraint.viewPosRef->viewPosStart = -400.0f;
    result = LazyLayoutUtils::Premeasure(outer, constraint);
    ASSERT_TRUE(result);
    EXPECT_FALSE(result->complete);
    EXPECT_FALSE(result->reachedOppositeEdge);
    EXPECT_FLOAT_EQ(result->marginMainSize, 600.0f);
    // Neither preparation published the outer frame. Compensation remains relative to its original zero extent.
    EXPECT_FLOAT_EQ(result->adjust.start + result->adjust.end, 600.0f);
    EXPECT_EQ(innerInfo->posMap_.size(), 2);
    EXPECT_EQ(innerInfo->visibleStartIndex_, -1);
    EXPECT_EQ(outerInfo->visibleStartIndex_, -1);
}

/**
 * @tc.name: PreparedGeometryColdNestedColumnFillsFromSuffix001
 * @tc.desc: Cold nested normal layout imports request values and fills only the missing window, not the prefix.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, PreparedGeometryColdNestedColumnFillsFromSuffix001, TestSize.Level1)
{
    auto innerPattern = AceType::MakeRefPtr<LazyColumnLayoutPattern>();
    auto inner = CreateCachedLazyLayoutTestContext(V2::LAZY_COLUMN_LAYOUT_ETS_TAG, innerPattern,
        AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(innerPattern->layoutInfo_), SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    auto outerPattern = AceType::MakeRefPtr<LazyColumnLayoutPattern>();
    auto host = FrameNode::CreateFrameNode(V2::LAZY_COLUMN_LAYOUT_ETS_TAG, -4500, outerPattern);
    inner.parent->RemoveChild(inner.host);
    host->AddChild(inner.host);
    auto outer = AceType::MakeRefPtr<LayoutWrapperNode>(
        host, AceType::MakeRefPtr<GeometryNode>(), host->GetLayoutProperty());
    outer->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(
        AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(outerPattern->layoutInfo_)));
    outer->AppendChild(inner.wrapper);
    int32_t measured = 0;
    for (int32_t index = 0; index < 10; ++index) {
        AppendFixedHeightChild(inner.wrapper, -4501 - index, index < 8 ? 50.0f : 100.0f, measured);
    }
    auto constraint = inner.parentConstraint;
    constraint.maxSize.SetHeight(Infinity<float>());
    constraint.viewPosRef = ViewPosReference {
        .viewPosStart = -200.0f, .viewPosEnd = 0.0f, .referencePos = 0.0f,
        .referenceEdge = ReferenceEdge::END, .axis = Axis::VERTICAL,
    };
    auto result = LazyLayoutUtils::Premeasure(outer, constraint);
    ASSERT_TRUE(result && result->complete);
    ASSERT_EQ(measured, 2);
    innerPattern->layoutInfo_ = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    outerPattern->layoutInfo_ = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    inner.wrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(
        AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(innerPattern->layoutInfo_)));
    outer->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(
        AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(outerPattern->layoutInfo_)));
    outerPattern->RestorePreparedGeometry(result->geometry);
    constraint.viewPosRef->viewPosStart = -250.0f;
    constraint.viewPosRef->viewPosEnd = -150.0f;
    outer->Measure(constraint);
    outer->Layout();
    EXPECT_EQ(innerPattern->GetPreparedGeometry(), result->geometry->children.at(0));
    // Normal backward layout includes the item touching the window's leading boundary.
    EXPECT_EQ(measured, 4);
    EXPECT_EQ(innerPattern->layoutInfo_->posMap_.begin()->first, 6);
    EXPECT_FLOAT_EQ(innerPattern->layoutInfo_->totalMainSize_ -
        innerPattern->layoutInfo_->posMap_.at(8).startPos, 200.0f);
}
/**
 * @tc.name: KnownExtentKeepsRealItems001
 * @tc.desc: A known outer extent rebases a measured suffix without resizing its real children.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, KnownExtentKeepsRealItems001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    info->totalItemCount_ = 4;
    info->space_ = 10.0f;
    info->startIndex_ = 3;
    info->posMap_[0] = { 0.0f, 100.0f };
    info->posMap_[3] = { 600.0f, 800.0f };
    ASSERT_TRUE(info->ApplyKnownBodySize(800.0f, 730.0f));
    EXPECT_FLOAT_EQ(info->estimateItemSize_, 200.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(0).endPos, 100.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(3).startPos, 530.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(3).endPos, 730.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.start, -70.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.end, 0.0f);
    EXPECT_EQ(info->posMap_.size(), 2);
    EXPECT_FALSE(info->ApplyKnownBodySize(730.0f, 300.0f));
    EXPECT_FLOAT_EQ(info->posMap_.at(3).startPos, 530.0f);
}

namespace {
CachedLazyLayoutTestContext CreateUnsupportedColumnContext(
    const RefPtr<LazyColumnLayoutInfo>& info, int32_t& measured)
{
    auto pattern = AceType::MakeRefPtr<LazyColumnLayoutPattern>();
    auto context = CreateCachedLazyLayoutTestContext(V2::LAZY_COLUMN_LAYOUT_ETS_TAG, pattern,
        AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(info), SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    AppendFixedHeightChild(context.wrapper, -REJECTED_HEADER_NODE_ID, 70.0f, measured);
    pattern->header_ = context.wrapper->GetOrCreateChildByIndex(0, false, true)->GetHostNode();
    for (int32_t index = 0; index < REJECTED_ITEM_COUNT; ++index) {
        AppendFixedHeightChild(context.wrapper, -REJECTED_CHILD_NODE_ID - index,
            index == 0 ? 300.0f : 100.0f, measured);
        if (index >= REJECTED_ITEM_INDEX) {
            info->posMap_[index] = { index * 100.0f, (index + 1) * 100.0f };
        }
    }
    // This non-forwarding child rejects lazy preflight after the first real item was committed.
    context.wrapper->GetOrCreateChildByIndex(REJECTED_ITEM_INDEX, false, true)->GetHostNode()->SetNeedLazyLayout(true);
    info->totalItemCount_ = REJECTED_ITEM_COUNT;
    info->estimateItemSize_ = 100.0f;
    info->headerMainSize_ = 50.0f;
    info->totalMainSize_ = 550.0f;
    info->startIndex_ = info->layoutedStartIndex_ = info->cachedStartIndex_ = REJECTED_ITEM_INDEX;
    info->endIndex_ = info->layoutedEndIndex_ = info->cachedEndIndex_ = REJECTED_ITEM_INDEX;
    info->visibleStartIndex_ = info->visibleEndIndex_ = REJECTED_ITEM_INDEX;
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
 * @tc.name: PremeasureUnsupportedFinalizesPrefix001
 * @tc.desc: A rejected child retains real prefix sizes, resized edges and the displayed anchor's adjustment.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, PremeasureUnsupportedFinalizesPrefix001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    int32_t measured = 0;
    auto context = CreateUnsupportedColumnContext(info, measured);
    auto active = context.wrapper->GetOrCreateChildByIndex(3, false, true);
    active->SetActive(true);
    auto result = LazyLayoutUtils::Premeasure(context.wrapper, context.parentConstraint);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->unsupported);
    EXPECT_FALSE(result->complete);
    EXPECT_FALSE(result->reachedOppositeEdge);
    EXPECT_EQ(measured, 2);
    EXPECT_FLOAT_EQ(info->headerMainSize_, 70.0f);
    EXPECT_FLOAT_EQ(info->totalMainSize_, 770.0f);
    EXPECT_FLOAT_EQ(result->marginMainSize, 770.0f);
    EXPECT_FLOAT_EQ(result->adjust.start, 220.0f);
    EXPECT_FLOAT_EQ(result->adjust.end, 0.0f);
    EXPECT_EQ(info->posMap_.size(), 4);
    EXPECT_EQ(info->posMap_.count(1), 0);
    EXPECT_FLOAT_EQ(info->posMap_.at(0).endPos, 300.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(2).startPos, 400.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(4).endPos, 700.0f);
    EXPECT_EQ(info->updatedStart_, INT_MAX);
    EXPECT_EQ(info->updatedEnd_, -1);
    EXPECT_FLOAT_EQ(info->layoutedStart_, 400.0f);
    EXPECT_FLOAT_EQ(info->layoutedEnd_, 500.0f);
    EXPECT_FLOAT_EQ(info->cacheStartPos_, 400.0f);
    EXPECT_FLOAT_EQ(info->cacheEndPos_, 500.0f);
    EXPECT_EQ(info->startIndex_, 2);
    EXPECT_EQ(info->endIndex_, 2);
    EXPECT_EQ(info->visibleStartIndex_, 2);
    EXPECT_EQ(info->visibleEndIndex_, 2);
    EXPECT_TRUE(active->IsActive());
    EXPECT_FLOAT_EQ(info->adjustOffset_.start, 7.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.end, 9.0f);
    EXPECT_FLOAT_EQ(context.wrapper->GetGeometryNode()->GetFrameSize().Height(), 550.0f);
}

/**
 * @tc.name: PremeasureUnsupportedThenNormalMeasure001
 * @tc.desc: Normal measurement reuses the finalized partial prefix without overlapping or compensating twice.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, PremeasureUnsupportedThenNormalMeasure001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    int32_t measured = 0;
    auto context = CreateUnsupportedColumnContext(info, measured);
    auto result = LazyLayoutUtils::Premeasure(context.wrapper, context.parentConstraint);
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->unsupported);
    auto constraint = context.parentConstraint;
    constraint.viewPosRef->viewPosStart = 250.0f;
    constraint.viewPosRef->viewPosEnd = 350.0f;
    constraint.viewPosRef->referencePos -= result->adjust.start;
    context.wrapper->SetActive(true);
    context.wrapper->Measure(constraint);
    ASSERT_EQ(info->posMap_.size(), 5);
    EXPECT_FLOAT_EQ(info->posMap_.at(0).endPos, 300.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(1).startPos, 300.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(1).endPos, 400.0f);
    EXPECT_FLOAT_EQ(info->posMap_.at(2).startPos, 400.0f);
    EXPECT_FLOAT_EQ(info->totalMainSize_, 770.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.start, 0.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.end, 0.0f);
}

/**
 * @tc.name: PremeasureNestedUnsupportedFinalizesParent001
 * @tc.desc: A nested terminal result carries its committed extent and internal anchor to its parent.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, PremeasureNestedUnsupportedFinalizesParent001, TestSize.Level1)
{
    auto innerInfo = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    int32_t measured = 0;
    auto inner = CreateUnsupportedColumnContext(innerInfo, measured);
    auto outerInfo = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    auto host = FrameNode::CreateFrameNode(
        V2::LAZY_COLUMN_LAYOUT_ETS_TAG, -4800, AceType::MakeRefPtr<LazyColumnLayoutPattern>());
    inner.parent->RemoveChild(inner.host);
    host->AddChild(inner.host);
    auto outer = AceType::MakeRefPtr<LayoutWrapperNode>(
        host, AceType::MakeRefPtr<GeometryNode>(), host->GetLayoutProperty());
    outer->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(
        AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(outerInfo)));
    outer->AppendChild(inner.wrapper);
    outer->GetGeometryNode()->SetFrameSize(SizeF(SCROLL_WIDTH, 550.0f));
    outerInfo->totalItemCount_ = 1;
    outerInfo->totalMainSize_ = 550.0f;
    outerInfo->posMap_[0] = { 0.0f, 550.0f };
    outerInfo->startIndex_ = outerInfo->endIndex_ = 0;
    auto result = LazyLayoutUtils::Premeasure(outer, inner.parentConstraint);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->unsupported);
    EXPECT_FALSE(result->complete);
    EXPECT_FALSE(result->reachedOppositeEdge);
    EXPECT_EQ(measured, 2);
    EXPECT_FLOAT_EQ(innerInfo->totalMainSize_, 770.0f);
    EXPECT_FLOAT_EQ(outerInfo->totalMainSize_, 770.0f);
    EXPECT_FLOAT_EQ(outerInfo->posMap_.at(0).endPos, 770.0f);
    EXPECT_FLOAT_EQ(result->marginMainSize, 770.0f);
    EXPECT_FLOAT_EQ(result->adjust.start, 220.0f);
    EXPECT_FLOAT_EQ(result->adjust.end, 0.0f);
    EXPECT_FLOAT_EQ(inner.wrapper->GetGeometryNode()->GetFrameSize().Height(), 550.0f);
    EXPECT_FLOAT_EQ(outer->GetGeometryNode()->GetFrameSize().Height(), 550.0f);
}

/**
 * @tc.name: PremeasureRejectedKnownExtentKeepsPartial001
 * @tc.desc: Inconsistent known sizes reject completion without discarding the finalized measured prefix.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, PremeasureRejectedKnownExtentKeepsPartial001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    int32_t measured = 0;
    auto context = CreateUnsupportedColumnContext(info, measured);
    context.parentConstraint.viewPosRef->viewPosEnd = 270.0f;
    context.parentConstraint.viewPosRef->knownMainSize = 170.0f;
    auto result = LazyLayoutUtils::Premeasure(context.wrapper, context.parentConstraint);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->unsupported);
    EXPECT_FALSE(result->complete);
    EXPECT_FALSE(result->reachedOppositeEdge);
    EXPECT_EQ(measured, 2);
    EXPECT_FLOAT_EQ(result->marginMainSize, 770.0f);
    EXPECT_FLOAT_EQ(result->adjust.start, 220.0f);
    EXPECT_FLOAT_EQ(result->adjust.end, 0.0f);
    EXPECT_FLOAT_EQ(info->totalMainSize_, 770.0f);
    EXPECT_EQ(info->updatedStart_, INT_MAX);
}

namespace {
CachedLazyLayoutTestContext CreateUnpublishedColumnContext(
    const RefPtr<LazyColumnLayoutInfo>& info, int32_t& measured)
{
    auto pattern = AceType::MakeRefPtr<LazyColumnLayoutPattern>();
    pattern->layoutInfo_ = info;
    auto context = CreateCachedLazyLayoutTestContext(V2::LAZY_COLUMN_LAYOUT_ETS_TAG, pattern,
        AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(info), SizeF(SCROLL_WIDTH, SCROLL_HEIGHT));
    for (int32_t index = 0; index < COMMITTED_ITEM_COUNT; ++index) {
        AppendFixedHeightChild(context.wrapper, -COMMITTED_NODE_ID - index,
            index == COMMITTED_TALL_INDEX ? 400.0f : 100.0f, measured);
        info->posMap_[index] = { index * 100.0f, (index + 1) * 100.0f };
    }
    context.wrapper->GetOrCreateChildByIndex(COMMITTED_REJECTED_INDEX, false, true)
        ->GetHostNode()->SetNeedLazyLayout(true);
    info->totalItemCount_ = COMMITTED_ITEM_COUNT;
    info->estimateItemSize_ = 100.0f;
    info->totalMainSize_ = 800.0f;
    info->startIndex_ = info->visibleStartIndex_ = 0;
    info->endIndex_ = info->visibleEndIndex_ = 0;
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
 * @tc.desc: A new algorithm sees the real tail even when preparation never published source geometry.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, PremeasureCrossFrameEndUsesCommittedExtent001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    int32_t measured = 0;
    auto context = CreateUnpublishedColumnContext(info, measured);
    auto result = LazyLayoutUtils::Premeasure(context.wrapper, context.parentConstraint);
    ASSERT_TRUE(result && result->unsupported);
    EXPECT_FLOAT_EQ(result->marginMainSize, 1100.0f);
    EXPECT_FLOAT_EQ(context.wrapper->GetGeometryNode()->GetFrameSize().Height(), 800.0f);
    ASSERT_TRUE(info->premeasuredMainSize_);
    auto normal = AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(info);
    context.wrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(normal));
    context.parentConstraint.viewPosRef->viewPosStart = -50.0f;
    context.parentConstraint.viewPosRef->viewPosEnd = 50.0f;
    context.wrapper->SetActive(true);
    context.wrapper->Measure(context.parentConstraint);
    context.wrapper->Layout();
    ASSERT_TRUE(normal->premeasuredMainSize_);
    EXPECT_FLOAT_EQ(normal->premeasuredMainSize_.value(), 1100.0f);
    EXPECT_FLOAT_EQ(normal->startPos_, 1050.0f);
    EXPECT_FALSE(info->premeasuredMainSize_);
    EXPECT_EQ(info->visibleStartIndex_, 7);
    EXPECT_EQ(info->visibleEndIndex_, 7);
    auto tail = context.wrapper->GetOrCreateChildByIndex(7, false, true);
    EXPECT_TRUE(tail->IsActive());
    EXPECT_FLOAT_EQ(tail->GetGeometryNode()->GetFrameOffset().GetY(), 1000.0f);
    EXPECT_FLOAT_EQ(info->totalMainSize_, 1100.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.start, 0.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.end, 0.0f);
}

/**
 * @tc.name: PremeasureSameFrameEndConsumesAdjustOnce001
 * @tc.desc: Parent-consumed end compensation and the committed extent preserve the existing head window once.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, PremeasureSameFrameEndConsumesAdjustOnce001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    int32_t measured = 0;
    auto context = CreateUnpublishedColumnContext(info, measured);
    auto result = LazyLayoutUtils::Premeasure(context.wrapper, context.parentConstraint);
    ASSERT_TRUE(result && result->unsupported);
    ASSERT_FLOAT_EQ(result->adjust.end, 300.0f);
    context.parentConstraint.viewPosRef->viewPosStart = 0.0f;
    context.parentConstraint.viewPosRef->viewPosEnd = 100.0f;
    context.parentConstraint.viewPosRef->referencePos = 750.0f + result->adjust.end;
    context.wrapper->SetActive(true);
    context.wrapper->Measure(context.parentConstraint);
    auto normal = AceType::DynamicCast<LazyColumnLayoutAlgorithm>(
        context.wrapper->GetLayoutAlgorithm()->GetLayoutAlgorithm());
    ASSERT_NE(normal, nullptr);
    EXPECT_FLOAT_EQ(normal->startPos_, 50.0f);
    EXPECT_FLOAT_EQ(normal->endPos_, 150.0f);
    EXPECT_EQ(info->visibleStartIndex_, 0);
    EXPECT_EQ(info->visibleEndIndex_, 1);
    EXPECT_FALSE(info->premeasuredMainSize_);
    EXPECT_FLOAT_EQ(info->adjustOffset_.start, 0.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.end, 0.0f);
    context.wrapper->Measure(context.parentConstraint);
    EXPECT_FLOAT_EQ(normal->startPos_, 50.0f);
    EXPECT_FLOAT_EQ(info->adjustOffset_.end, 0.0f);
}

/**
 * @tc.name: PremeasureExtentSurvivesCountChange001
 * @tc.desc: A subsequent data update is measured against the committed baseline, not unpublished geometry.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, PremeasureExtentSurvivesCountChange001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    int32_t measured = 0;
    auto context = CreateUnpublishedColumnContext(info, measured);
    ASSERT_TRUE(LazyLayoutUtils::Premeasure(context.wrapper, context.parentConstraint));
    AppendFixedHeightChild(context.wrapper, -5110, 100.0f, measured);
    context.parentConstraint.viewPosRef->viewPosStart = -50.0f;
    context.parentConstraint.viewPosRef->viewPosEnd = 50.0f;
    context.wrapper->Measure(context.parentConstraint);
    auto normal = AceType::DynamicCast<LazyColumnLayoutAlgorithm>(
        context.wrapper->GetLayoutAlgorithm()->GetLayoutAlgorithm());
    ASSERT_NE(normal, nullptr);
    ASSERT_TRUE(normal->premeasuredMainSize_);
    EXPECT_FLOAT_EQ(normal->premeasuredMainSize_.value(), 1100.0f);
    EXPECT_FALSE(info->premeasuredMainSize_);
    EXPECT_EQ(info->totalItemCount_, 9);
    EXPECT_EQ(info->visibleEndIndex_, 8);
    EXPECT_FLOAT_EQ(info->totalMainSize_, 1200.0f);
}

/**
 * @tc.name: PremeasureExtentClearsOnlyForPublishedGone001
 * @tc.desc: Temporary GONE measurement preserves the marker until a real zero-size GONE frame is published.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, PremeasureExtentClearsOnlyForPublishedGone001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    int32_t measured = 0;
    auto context = CreateUnpublishedColumnContext(info, measured);
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
    context.host->GetPattern<LazyColumnLayoutPattern>()->OnDirtyLayoutWrapperSwap(context.host, config);
    EXPECT_FALSE(info->premeasuredMainSize_);
}
/**
 * @tc.name: BackwardRangeReadsGeometryAtOriginalPoint001
 * @tc.desc: Ordinary backward layout reads current geometry after frame-baseline capture, not before it.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, BackwardRangeReadsGeometryAtOriginalPoint001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    info->totalMainSize_ = 1000.0f;
    auto algorithm = AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(info);
    auto context = CreateCachedLazyLayoutTestContext(V2::LAZY_COLUMN_LAYOUT_ETS_TAG,
        AceType::MakeRefPtr<LazyColumnLayoutPattern>(), algorithm, SizeF(SCROLL_WIDTH, 200.0f));
    algorithm->CaptureFrameBaseline();
    context.wrapper->GetGeometryNode()->SetFrameSize(SizeF(SCROLL_WIDTH, 450.0f));
    ViewPosReference reference { .viewPosStart = -100.0f, .viewPosEnd = 0.0f,
        .referenceEdge = ReferenceEdge::END, .axis = Axis::VERTICAL };
    ASSERT_TRUE(algorithm->UpdateViewRange(AceType::RawPtr(context.wrapper), reference));
    EXPECT_FLOAT_EQ(algorithm->referencePos_, 550.0f);
    EXPECT_FLOAT_EQ(algorithm->endPos_, 450.0f);
}

/**
 * @tc.name: BackwardRangeUsesOptionalPremeasureBaseline001
 * @tc.desc: A saved extent, including zero, replaces unpublished geometry only for the prepared frame.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, BackwardRangeUsesOptionalPremeasureBaseline001, TestSize.Level1)
{
    for (float baseline : { 0.0f, 1100.0f }) {
        SCOPED_TRACE(baseline);
        auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
        info->totalMainSize_ = 1000.0f;
        info->premeasuredMainSize_ = baseline;
        auto algorithm = AceType::MakeRefPtr<LazyColumnLayoutAlgorithm>(info);
        auto context = CreateCachedLazyLayoutTestContext(V2::LAZY_COLUMN_LAYOUT_ETS_TAG,
            AceType::MakeRefPtr<LazyColumnLayoutPattern>(), algorithm, SizeF(SCROLL_WIDTH, 200.0f));
        algorithm->CaptureFrameBaseline();
        EXPECT_FALSE(info->premeasuredMainSize_);
        context.wrapper->GetGeometryNode()->SetFrameSize(SizeF(SCROLL_WIDTH, 450.0f));
        ViewPosReference reference { .viewPosStart = -100.0f, .viewPosEnd = 0.0f,
            .referenceEdge = ReferenceEdge::END, .axis = Axis::VERTICAL };
        ASSERT_TRUE(algorithm->UpdateViewRange(AceType::RawPtr(context.wrapper), reference));
        EXPECT_FLOAT_EQ(algorithm->endPos_, baseline);
        algorithm->CaptureFrameBaseline();
        ASSERT_TRUE(algorithm->UpdateViewRange(AceType::RawPtr(context.wrapper), reference));
        EXPECT_FLOAT_EQ(algorithm->endPos_, 450.0f);
    }
}

/**
 * @tc.name: BackwardRangeWithoutGeometryKeepsLegacyGuard001
 * @tc.desc: Without a prepared baseline, the original missing-geometry guard still rejects backward layout.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, BackwardRangeWithoutGeometryKeepsLegacyGuard001, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    LazyColumnLayoutAlgorithm algorithm(info);
    LayoutWrapperNode wrapper(nullptr, nullptr, AceType::MakeRefPtr<LazyColumnLayoutProperty>());
    ViewPosReference reference { .referenceEdge = ReferenceEdge::END, .axis = Axis::VERTICAL };
    EXPECT_FALSE(algorithm.UpdateViewRange(&wrapper, reference));
}

/**
 * @tc.name: PublicDumpPreservesFlatLayoutFields
 * @tc.desc: Public filtering preserves legacy field values without a layoutInfo wrapper.
 * @tc.type: FUNC
 */
HWTEST_F(LazyColumnLayoutTest, PublicDumpPreservesFlatLayoutFields, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<LazyColumnLayoutPattern>();
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
            EXPECT_EQ(json->Contains("space"), visual);
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
HWTEST_F(LazyColumnLayoutTest, LegacyDumpMatchesBaselineSnapshot, TestSize.Level1)
{
    auto info = AceType::MakeRefPtr<LazyColumnLayoutInfo>();
    info->startIndex_ = 2;
    info->endIndex_ = 6;
    info->totalItemCount_ = 9;
    info->headerMainSize_ = 12.0f;
    info->footerMainSize_ = 24.0f;
    info->space_ = 3.0f;
    info->totalMainSize_ = 240.0f;
    // Frozen from fdf7d63988ed9fa338051a11c19fadf1f86653dc; compare serialized key order and types.
    const std::string expected = R"({"itemStartIndex":2,"itemEndIndex":6,"itemTotalCount":9,)"
        R"("headerMainSize":12,"footerMainSize":24,"space":3,"totalMainSize":240})";
    auto json = JsonUtil::Create();
    info->DumpAdvanceInfo(json);
    EXPECT_EQ(json->ToString(), expected);
}

} // namespace OHOS::Ace::NG
