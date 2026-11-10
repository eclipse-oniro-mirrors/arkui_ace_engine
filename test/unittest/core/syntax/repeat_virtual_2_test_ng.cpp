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

#include "repeat_virtual_2_test_ng.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {

using CacheItem = RepeatVirtualScroll2Caches::CacheItem;
using OptCacheItem = RepeatVirtualScroll2Caches::OptCacheItem;
using GetFrameChildResult = RepeatVirtualScroll2Caches::GetFrameChildResult;

namespace {
constexpr int64_t CACHE_TASK_DELAY_TIME = 2000000000; // 2 seconds in nanoseconds
constexpr int32_t MEMORY_LEVEL_LOW = 1;
constexpr int32_t MEMORY_LEVEL_CRITICAL = 2;
constexpr int32_t MEMORY_LEVEL_NORMAL = 0;
// Constants for the consolidated RepeatNode / caches / model tests
constexpr int32_t TEST_INDEX_1 = 0;
constexpr int32_t TEST_INDEX_2 = 1;
constexpr int32_t TEST_INDEX_3 = 2;
constexpr int32_t TEST_INDEX_5 = 5;
constexpr uint32_t TEST_RID_1 = 100;
constexpr uint32_t TEST_RID_2 = 200;
constexpr int32_t TEST_NODE_ID = 1;
constexpr int32_t TEST_REPEAT_NODE_ID = 100;
constexpr int32_t TEST_REPEAT_NODE_ID_2 = 200;
constexpr int32_t TEST_LIST_PARENT_ID = 9201;
constexpr int32_t TEST_GRID_PARENT_ID = 9202;
constexpr int32_t TEST_WATERFLOW_PARENT_ID = 9203;
constexpr int32_t TEST_COLUMN_PARENT_ID = 9204;
constexpr uint32_t TEST_ARR_LEN = 100;
constexpr uint32_t TEST_TOTAL_COUNT = 100;
constexpr int32_t TEST_REPEAT_ELMT_ID = 1000;
constexpr int32_t TEST_REPEAT_ELMT_ID_2 = 2000;
constexpr int32_t INVALID_REPEAT_ELMT_ID = -1;
const std::string TEST_INSPECTOR_ID = "repeat_sync_cache_test_id";

std::unique_ptr<RepeatVirtualScroll2Caches> CreateCachesWithMockCallback()
{
    auto onGetRid4Index = [](IndexType index, bool, bool) -> std::pair<RIDType, uint32_t> {
        return { TEST_RID_1, OnGetRid4IndexResult::NO_NODE };
    };
    return std::make_unique<RepeatVirtualScroll2Caches>(onGetRid4Index);
}
} // namespace

/**
 * @tc.name: CreateNode001
 * @tc.desc: Test creation of FrameNode
 * @tc.type: FUNC
 */
RefPtr<FrameNode> RepeatVirtual2TestNg::CreateNode(const std::string& tag)
{
    auto pattern = AceType::MakeRefPtr<Pattern>();
    auto frameNode = AceType::MakeRefPtr<FrameNode>(tag, elmtId_, pattern);
    pattern->AttachToFrameNode(frameNode);
    ViewStackProcessor::GetInstance()->Push(frameNode);
    return frameNode;
}

RefPtr<RepeatVirtualScroll2Node> RepeatVirtual2TestNg::CreateRepeatVirtualNode(uint32_t arrLen, uint32_t totalCount)
{
    constexpr int32_t DEFAULT_MEM_OPT_STRATEGY = 0;
    l1Rid4Index_ = {
        {0, 1},
        {1, 2},
        {2, 3},
        {3, 4},
        {4, 5},
        {5, 6}
    };
    onGetRid4Index_ = [&](IndexType index, bool inAnimation, bool forceCreateNewChild) -> std::pair<RIDType, uint32_t> {
        auto it = l1Rid4Index_.find(0);
        if (it != l1Rid4Index_.end()) {
            return {index, 2};
        }
        return {index, 1};
    };
    onRecycleItems_ = [](IndexType fromIndex, IndexType toIndex) -> void {
        return;
    };
    onActiveRange_ = [](int32_t fromIndex, int32_t toIndex, int32_t vStart, int32_t vEnd, bool isLoop,
        bool forceUpdate) -> void {
        return;
    };
    onMoveFromTo_ = [](IndexType, IndexType) -> void {
        return;
    };
    onPurge_ = []() -> void {
        return;
    };
    onPurgeAll_ = []() -> void {
        return;
    };
    onUpdateDirty_ = []() -> void {
        return;
    };
    return RepeatVirtualScroll2Node::GetOrCreateRepeatNode(
        GetElmtId(), arrLen, totalCount, DEFAULT_MEM_OPT_STRATEGY, onGetRid4Index_, onRecycleItems_, onActiveRange_,
        onMoveFromTo_, onPurge_, onPurgeAll_, onUpdateDirty_);
}

/**
 * @tc.name: CreateListItemNode
 * @tc.desc: Test creation of ListItemNode
 * @tc.type: FUNC
 */
RefPtr<FrameNode> RepeatVirtual2TestNg::CreateListItemNode()
{
    auto tag = "TEXT_ETS_TAG";
    auto* stack = ViewStackProcessor::GetInstance();
    auto listItemFrameNode = FrameNode::GetOrCreateFrameNode(V2::LIST_ITEM_ETS_TAG, GetElmtId(), []() {
        return AceType::MakeRefPtr<ListItemPattern>(nullptr, V2::ListItemStyle::NONE);
    });

    auto textNode = CreateNode(V2::TEXT_ETS_TAG);

    auto pattern = AceType::MakeRefPtr<Pattern>();
    auto textFrameNode = AceType::MakeRefPtr<FrameNode>(tag, GetElmtId(), pattern);
    pattern->AttachToFrameNode(textFrameNode);
    listItemFrameNode->AddChild(textFrameNode);

    pattern = AceType::MakeRefPtr<Pattern>();
    textFrameNode = AceType::MakeRefPtr<FrameNode>(tag, GetElmtId(), pattern);
    pattern->AttachToFrameNode(textFrameNode);
    listItemFrameNode->AddChild(textFrameNode);
    stack->Push(listItemFrameNode);
    return listItemFrameNode;
}

class MockFrameNode : public FrameNode {
    DECLARE_ACE_TYPE(MockFrameNode, FrameNode);

public:
    MockFrameNode(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern)
        : FrameNode(tag, nodeId, pattern)
    {}

    ~MockFrameNode() override = default;

    bool IsAtomicNode() const override
    {
        return true;
    }

    MOCK_METHOD(void, OnRecycle, (), (override));
    MOCK_METHOD(void, OnReuse, (), (override));
};

/**
 * @tc.name: CreateRepeat001
 * @tc.desc: Test creation of GetOrCreateRepeatNode
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, CreateRepeat001, TestSize.Level1)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    /**
     * @tc.steps: step1. Create node object
     * @tc.expected: Object is not nullptr.
     */
    auto repeatNode = RepeatVirtualScroll2Node::GetOrCreateRepeatNode(
        nodeId, 10, 10, 0, onGetRid4Index_, onRecycleItems_, onActiveRange_,
        onMoveFromTo_, onPurge_, onPurgeAll_, onUpdateDirty_);

    EXPECT_NE(repeatNode, nullptr);
}

/**
 * @tc.name: FrameCount001
 * @tc.desc: Test node.FrameCount
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, FrameCount001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);

    /**
     * @tc.steps: step2. Get frame count
     * @tc.expected: frame count should be 10
     */
    auto frameCount = repeatNode->FrameCount();
    EXPECT_EQ(frameCount, 10);

    /**
     * @tc.steps: step2. Update totalCount to 2
     * @tc.expected: frame count should be 2
     */
    repeatNode->UpdateTotalCount(2);
    frameCount = repeatNode->FrameCount();
    EXPECT_EQ(frameCount, 2);
}

/**
 * @tc.name: GetChildren001
 * @tc.desc: Test node.GetChildren
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, GetChildren001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Test node.GetChildren when repeat is empty
     * @tc.expected: children size shoule be 0
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    auto children = repeatNode->GetChildren();
    EXPECT_EQ(children.size(), 0);

    /**
     * @tc.steps: step2. Test node.GetChildren when repeat has 1 item
     * @tc.expected: children size shoule be 0
     */
    repeatNode->caches_.l1Rid4Index_ = {
        {0, 1}
    };
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2002, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    std::map<RIDType, CacheItem> cacheItem4Rid_ = {{ 0, cacheItem }};
    repeatNode->children_ = { uiNode };
    children = repeatNode->GetChildren();
    EXPECT_EQ(children.size(), 1);
}

/**
 * @tc.name: GetChildren002
 * @tc.desc: Test node.GetChildren
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, GetChildren002, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    repeatNode->caches_.l1Rid4Index_ = {
        {0, 1}, {1, 2}, {2, 3}, {3, 4}
    };
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2003, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    CacheItem cacheItem2 = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    cacheItem2->node_ = nullptr;
    repeatNode->caches_.cacheItem4Rid_ = {
        { 1, cacheItem }, { 2, cacheItem }, { 3, cacheItem }, { 4, cacheItem2 }
    };
    EXPECT_EQ(repeatNode->GetChildren().size(), 3);
    repeatNode->caches_.moveFromTo_ = {0, 1};
    repeatNode->children_.clear();
    EXPECT_EQ(repeatNode->GetChildren().size(), 3);
}

/**
 * @tc.name: GetFrameChild001
 * @tc.desc: Test caches.GetFrameChild
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, GetFrameChild001, TestSize.Level1)
{
    RepeatVirtualScroll2Caches caches(onGetRid4Index_);

    /**
     * @tc.steps: step1. give l1 cache items include item (rid == 1)
     * @tc.expected: return rid
     */
    GetFrameChildResult result = caches.GetFrameChild(1, false);
    EXPECT_EQ(result.second, 0);

    /**
     * @tc.steps: step2. give l1 cache items include item (rid == 10)
     * @tc.expected: return nullptr
     */
    result = caches.GetFrameChild(10, false);
    EXPECT_EQ(result.second, nullptr);
}

/**
 * @tc.name: RemoveNode001
 * @tc.desc: Test caches.RemoveNode
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RemoveNode001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    repeatNode->caches_.l1Rid4Index_ = {
        {0, 1}, {1, 2}, {2, 3}, {3, 4}
    };
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2004, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    repeatNode->caches_.cacheItem4Rid_ = {
        { 1, cacheItem }, { 2, cacheItem }, { 3, cacheItem }, { 4, cacheItem }
    };

    /**
     * @tc.steps: step1. remove 1 item from L1
     * @tc.expected: repeat has 3 children left
     */
    EXPECT_EQ(repeatNode->GetChildren().size(), 4);
    repeatNode->caches_.RemoveNode(2);
    repeatNode->children_.clear();
    EXPECT_EQ(repeatNode->GetChildren().size(), 3);
}

/**
 * @tc.name: CheckActiveRange001
 * @tc.desc: Test node.CheckActiveRange for List/Swiper-no-Loop scenario
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, CheckActiveRange001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ActiveRangeType activeRange;

    /**
     * @tc.steps: step1. check active range: [0 1 2] 3 4 5..
     * @tc.expected: active range is {0,4}
     */
    activeRange = repeatNode->CheckActiveRange(0, 2, 0, 2);
    std::pair<IndexType, IndexType> expect_result1(0, 4);
    EXPECT_EQ(activeRange, expect_result1);

    /**
     * @tc.steps: step2. check active range: ..1 2 [3 4 5] 6 7..
     * @tc.expected: active range is {1,7}
     */
    activeRange = repeatNode->CheckActiveRange(3, 5, 2, 2);
    std::pair<IndexType, IndexType> expect_result2(1, 7);
    EXPECT_EQ(activeRange, expect_result2);

    /**
     * @tc.steps: step3. check active range: ..5 6 [7 8 9]
     * @tc.expected: active range is {5,9}
     */
    activeRange = repeatNode->CheckActiveRange(7, 9, 2, 0);
    std::pair<IndexType, IndexType> expect_result3(5, 9);
    EXPECT_EQ(activeRange, expect_result3);
}

/**
 * @tc.name: CheckActiveRange002
 * @tc.desc: Test node.CheckActiveRange for Grid/WaterFlow scenario
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, CheckActiveRange002, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ActiveRangeType activeRange;

    /**
     * @tc.steps: step1. check active range for: [0 1 2] 3 4 5..
     * @tc.expected: active range is {0,4}
     */
    activeRange = repeatNode->CheckActiveRange(0, 2, 2, 2);
    std::pair<IndexType, IndexType> expect_result1(0, 4);
    EXPECT_EQ(activeRange, expect_result1);

    /**
     * @tc.steps: step2. check active range for: ..1 2 [3 4 5] 6 7..
     * @tc.expected: active range is {1,7}
     */
    activeRange = repeatNode->CheckActiveRange(3, 5, 2, 2);
    std::pair<IndexType, IndexType> expect_result2(1, 7);
    EXPECT_EQ(activeRange, expect_result2);

    /**
     * @tc.steps: step3. check active range for: ..5 6 [7 8 9]
     * @tc.expected: active range is {5,9}
     */
    activeRange = repeatNode->CheckActiveRange(7, 9, 2, 2);
    std::pair<IndexType, IndexType> expect_result3(5, 9);
    EXPECT_EQ(activeRange, expect_result3);
}

/**
 * @tc.name: CheckActiveRange003
 * @tc.desc: Test node.CheckActiveRange for Swiper-Loop scenario
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, CheckActiveRange003, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    repeatNode->SetIsLoop(true);
    ActiveRangeType activeRange;

    /**
     * @tc.steps: step1. check active range for: ..8 9 [0 1 2] 3 4..
     * @tc.expected: active range is {-2,4}
     */
    activeRange = repeatNode->CheckActiveRange(0, 2, 2, 2);
    std::pair<IndexType, IndexType> expect_result1(-2, 4);
    EXPECT_EQ(activeRange, expect_result1);

    /**
     * @tc.steps: step2. check active range for: ..7 8 [9 0 1] 2 3..
     * @tc.expected: active range is {7,3}
     */
    activeRange = repeatNode->CheckActiveRange(9, 1, 2, 2);
    std::pair<IndexType, IndexType> expect_result2(7, 3);
    EXPECT_EQ(activeRange, expect_result2);

    /**
     * @tc.steps: step3. check active range for: ..5 6 [7 8 9] 0 1..
     * @tc.expected: active range is {5,11}
     */
    activeRange = repeatNode->CheckActiveRange(7, 9, 2, 2);
    std::pair<IndexType, IndexType> expect_result3(5, 11);
    EXPECT_EQ(activeRange, expect_result3);

    /**
     * @tc.steps: step4. check active range for: 2 [3 0] 1
     * @tc.expected: active range is {3,2} (overlapped)
     */
    auto repeatNode2 = CreateRepeatVirtualNode(4, 4);
    repeatNode2->SetIsLoop(true);
    activeRange = repeatNode2->CheckActiveRange(3, 0, 2, 2);
    std::pair<IndexType, IndexType> expect_result4(3, 2);
    EXPECT_EQ(activeRange, expect_result4);

    /**
     * @tc.steps: step5. check active range for: [0 1] 2 3..
     * @tc.expected: active range is {-2,3}
     */
    auto repeatNode3 = CreateRepeatVirtualNode(10, 10);
    repeatNode3->SetIsLoop(true);
    activeRange = repeatNode3->CheckActiveRange(0, 1, 2, 2);
    std::pair<IndexType, IndexType> expect_result5(-2, 3);
    EXPECT_EQ(activeRange, expect_result5);
}

/**
 * @tc.name: CheckActiveRange004
 * @tc.desc: Test node.CheckActiveRange for List/Grid/WaterFlow/Swiper-no-Loop scenario (multiple components)
 *   X indicates a non-repeat child
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, CheckActiveRange004, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ActiveRangeType activeRange;

    /**
     * @tc.steps: step1. check active range for: ..[X X 0 1 2] 3 4..
     * @tc.expected: active range is {0,4}
     */
    activeRange = repeatNode->CheckActiveRange(-2, 2, 2, 2);
    std::pair<IndexType, IndexType> expect_result1(0, 4);
    EXPECT_EQ(activeRange, expect_result1);

    /**
     * @tc.steps: step2. check active range for: ..[X X X] 0 1 2..
     * @tc.expected: active range is {0,1}
     */
    activeRange = repeatNode->CheckActiveRange(-5, -1, 2, 2);
    std::pair<IndexType, IndexType> expect_result2(0, 1);
    EXPECT_EQ(activeRange, expect_result2);

    /**
     * @tc.steps: step3. check active range for: ..[X X X] X X 0 1..
     * @tc.expected: active range is {INT32_MAX,INT32_MAX}
     */
    activeRange = repeatNode->CheckActiveRange(-5, -3, 2, 2);
    std::pair<IndexType, IndexType> expect_result3(INT32_MAX, INT32_MAX);
    EXPECT_EQ(activeRange, expect_result3);

    /**
     * @tc.steps: step4. check active range for: ..5 6 [7 8 9 X X]..
     * @tc.expected: active range is {5,9}
     */
    activeRange = repeatNode->CheckActiveRange(7, 11, 2, 2);
    std::pair<IndexType, IndexType> expect_result4(5, 9);
    EXPECT_EQ(activeRange, expect_result4);

    /**
     * @tc.steps: step5. check active range for: ..8 9 [X X X]..
     * @tc.expected: active range is {8,9}
     */
    activeRange = repeatNode->CheckActiveRange(10, 12, 2, 2);
    std::pair<IndexType, IndexType> expect_result5(8, 9);
    EXPECT_EQ(activeRange, expect_result5);

    /**
     * @tc.steps: step6. check active range for: ..8 9 X X [X X X]..
     * @tc.expected: active range is {INT32_MAX,INT32_MAX}
     */
    activeRange = repeatNode->CheckActiveRange(12, 14, 2, 2);
    std::pair<IndexType, IndexType> expect_result6(INT32_MAX, INT32_MAX);
    EXPECT_EQ(activeRange, expect_result6);
}

/**
 * @tc.name: CheckNodeInL1001
 * @tc.desc: Test node.CheckNode4IndexInL1
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, CheckNodeInL1001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2009, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    bool remainInL1;

    /**
     * @tc.steps: step1. check normal active range
     * @tc.expected: node in L1 cache, return true
     */
    remainInL1 = repeatNode->CheckNode4IndexInL1(1, 0, 9, cacheItem);
    EXPECT_EQ(remainInL1, true);
    remainInL1 = repeatNode->CheckNode4IndexInL1(5, 6, 9, cacheItem);
    EXPECT_EQ(remainInL1, false);
    remainInL1 = repeatNode->CheckNode4IndexInL1(10, 6, 9, cacheItem);
    EXPECT_EQ(remainInL1, false);

    /**
     * @tc.steps: step2. check swiper-loop range
     * @tc.expected: node in L1 cache, return true
     */
    repeatNode->SetIsLoop(true);
    remainInL1 = repeatNode->CheckNode4IndexInL1(5, 0, 9, cacheItem);
    EXPECT_EQ(remainInL1, true);
    remainInL1 = repeatNode->CheckNode4IndexInL1(3, 2, 1, cacheItem);
    EXPECT_EQ(remainInL1, true);
}

/**
 * @tc.name: SetActiveRange001
 * @tc.desc: Test node.DoSetActiveChildRange
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, SetActiveRange001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);

    /**
     * @tc.steps: step1. check normal active range
     * @tc.expected: repeat has 8 items
     */
    repeatNode->children_.clear();
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2010, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    for (int i = 0; i < 10; ++i) {
        repeatNode->caches_.l1Rid4Index_.insert({i, i + 1});
        repeatNode->caches_.cacheItem4Rid_.insert({i + 1, cacheItem});
    }
    repeatNode->DoSetActiveChildRange(0, 5, 2, 2);
    EXPECT_EQ(repeatNode->GetChildren().size(), 8);

    /**
     * @tc.steps: step2. check swiper-loop range
     * @tc.expected: repeat has 6 items
     */
    repeatNode->children_.clear();
    repeatNode->caches_.l1Rid4Index_.clear();
    for (int i = 0; i < 10; ++i) {
        repeatNode->caches_.l1Rid4Index_.insert({i, i + 1});
        repeatNode->caches_.cacheItem4Rid_.insert({i + 1, cacheItem});
    }
    repeatNode->SetIsLoop(true);
    repeatNode->DoSetActiveChildRange(8, 1, 1, 1);
    EXPECT_EQ(repeatNode->GetChildren().size(), 6);
}

/**
 * @tc.name: GetL1Nodes001
 * @tc.desc: Test caches.GetL1Index4Node
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, GetL1Nodes001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2011, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    repeatNode->caches_.l1Rid4Index_ = {
        {0, 1}
    };
    repeatNode->caches_.cacheItem4Rid_ = {
        {1, cacheItem}
    };

    /**
     * @tc.steps: step1.
     * @tc.expected: GetL1Index4Node return 1
     */
    std::optional<IndexType> result = repeatNode->caches_.GetL1Index4Node(AceType::DynamicCast<FrameNode>(uiNode));
    EXPECT_EQ(result.value(), 0);
}

/**
 * @tc.name: GetL1Nodes002
 * @tc.desc: Test caches.GetL1Index4Node
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, GetL1Nodes002, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    RefPtr<UINode> uiNode = CreateTestUINode(GetElmtId());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    repeatNode->caches_.l1Rid4Index_ = {
        {0, 1}
    };
    repeatNode->caches_.cacheItem4Rid_ = {
        {1, cacheItem}
    };

    /**
     * @tc.steps: step1.
     * @tc.expected: GetL1Index4Node return null
     */
    std::optional<IndexType> result = repeatNode->caches_.GetL1Index4Node(AceType::DynamicCast<FrameNode>(uiNode));
    EXPECT_EQ(result.has_value(), false);
}

/**
 * @tc.name: GetAllNodes001
 * @tc.desc: Test caches.GetCacheItem4RID
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, GetAllNodes001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2012, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);

    /**
     * @tc.steps: step1.
     * @tc.expected: should return cacheItem
     */
    repeatNode->caches_.l1Rid4Index_ = {
        {0, 1}
    };
    repeatNode->caches_.cacheItem4Rid_ = {
        {1, cacheItem}
    };
    OptCacheItem optCacheItem = repeatNode->caches_.GetCacheItem4RID(1);
    EXPECT_EQ(optCacheItem.has_value(), true);
}

/**
 * @tc.name: GetRID4Index001
 * @tc.desc: Test caches.GetRID4Index
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, GetRID4Index001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2013, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);

    /**
     * @tc.steps: step1.
     * @tc.expected: should return rid=1
     */
    repeatNode->caches_.l1Rid4Index_ = {
        {0, 1}
    };
    repeatNode->caches_.cacheItem4Rid_ = {
        {1, cacheItem}
    };
    std::optional<RIDType> rid = repeatNode->caches_.GetRID4Index(0);
    EXPECT_EQ(rid.value(), 1);
}

/**
 * @tc.name: CallOnGetRid4Index001
 * @tc.desc: Test caches.CallOnGetRid4Index
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, CallOnGetRid4Index001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);

    repeatNode->caches_.onGetRid4Index_ = [](IndexType index, bool inAnimation, bool forceCreateNewChild)->
        std::pair<RIDType, uint32_t> { return {1, OnGetRid4IndexResult::CREATED_NEW_NODE}; };
    auto item0 = repeatNode->caches_.CallOnGetRid4Index(0).value();
    EXPECT_EQ(item0, nullptr);

    repeatNode->caches_.onGetRid4Index_ = [&](IndexType index, bool inAnimation, bool forceCreateNewChild)->
        std::pair<RIDType, uint32_t> { CreateListItemNode(); return {1, OnGetRid4IndexResult::CREATED_NEW_NODE}; };
    auto item1 = repeatNode->caches_.CallOnGetRid4Index(0).value();
    EXPECT_EQ(item1->node_->GetId(), 10002);

    repeatNode->caches_.onGetRid4Index_ = [&](IndexType index, bool inAnimation, bool forceCreateNewChild)->
        std::pair<RIDType, uint32_t> { CreateListItemNode(); return {0, OnGetRid4IndexResult::CREATED_NEW_NODE}; };
    auto item2 = repeatNode->caches_.CallOnGetRid4Index(0);
    EXPECT_EQ(item2, std::nullopt);

    repeatNode->caches_.onGetRid4Index_ = [](IndexType index, bool inAnimation, bool forceCreateNewChild)->
        std::pair<RIDType, uint32_t> { return {1, OnGetRid4IndexResult::UPDATED_NODE}; };
    auto item3 = repeatNode->caches_.CallOnGetRid4Index(0).value();
    EXPECT_EQ(item3->node_->GetId(), 10002);

    repeatNode->caches_.onGetRid4Index_ = [](IndexType index, bool inAnimation, bool forceCreateNewChild)->
        std::pair<RIDType, uint32_t> { return {2, OnGetRid4IndexResult::UPDATED_NODE}; };
    auto item4 = repeatNode->caches_.CallOnGetRid4Index(0);
    EXPECT_EQ(item4, std::nullopt);

    repeatNode->caches_.onGetRid4Index_ = [](IndexType index, bool inAnimation, bool forceCreateNewChild)->
        std::pair<RIDType, uint32_t> { return {1, OnGetRid4IndexResult::UPDATED_NODE}; };
    repeatNode->caches_.GetCacheItem4RID(1).value()->node_ = nullptr;
    auto item5 = repeatNode->caches_.CallOnGetRid4Index(0);
    EXPECT_EQ(item5, std::nullopt);

    repeatNode->caches_.onGetRid4Index_ = [](IndexType index, bool inAnimation, bool forceCreateNewChild)->
        std::pair<RIDType, uint32_t> { return {0, OnGetRid4IndexResult::UPDATED_NODE}; };
    auto item6 = repeatNode->caches_.CallOnGetRid4Index(0);
    EXPECT_EQ(item6, std::nullopt);

    repeatNode->caches_.onGetRid4Index_ = [](IndexType index, bool inAnimation, bool forceCreateNewChild)->
        std::pair<RIDType, uint32_t> { return {1, OnGetRid4IndexResult::NO_NODE}; };
    auto item7 = repeatNode->caches_.CallOnGetRid4Index(0);
    EXPECT_EQ(item7, std::nullopt);
}

/**
 * @tc.name: ConvertFromToIndex001
 * @tc.desc: Test caches.ConvertFromToIndex
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, ConvertFromToIndex001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(6, 6);
    repeatNode->MoveData(0, 1);
    repeatNode->MoveData(1, 2);
    repeatNode->MoveData(2, 3);

    /**
     * @tc.steps: step1.
     * @tc.expected: index is 0, mappedIndex should be 1.
     */
    IndexType mappedIndex = repeatNode->caches_.ConvertFromToIndex(0);
    EXPECT_EQ(mappedIndex, 1);

    /**
     * @tc.steps: step2.
     * @tc.expected: index is 1, mappedIndex should be 2.
     */
    mappedIndex = repeatNode->caches_.ConvertFromToIndex(1);
    EXPECT_EQ(mappedIndex, 2);

    /**
     * @tc.steps: step3.
     * @tc.expected: index is 2, mappedIndex should be 3.
     */
    mappedIndex = repeatNode->caches_.ConvertFromToIndex(2);
    EXPECT_EQ(mappedIndex, 3);

    /**
     * @tc.steps: step4.
     * @tc.expected: index is 3, mappedIndex should be 0.
     */
    mappedIndex = repeatNode->caches_.ConvertFromToIndex(3);
    EXPECT_EQ(mappedIndex, 0);

    /**
     * @tc.steps: step5.
     * @tc.expected: index is 4, 5, 6, mappedIndex should be same with index.
     */
    mappedIndex = repeatNode->caches_.ConvertFromToIndex(4);
    EXPECT_EQ(mappedIndex, 4);
    mappedIndex = repeatNode->caches_.ConvertFromToIndex(5);
    EXPECT_EQ(mappedIndex, 5);
    mappedIndex = repeatNode->caches_.ConvertFromToIndex(6);
    EXPECT_EQ(mappedIndex, 6);
}

/**
 * @tc.name: ConvertFromToIndex002
 * @tc.desc: Test caches.ConvertFromToIndexRevert
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, ConvertFromToIndex002, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(6, 6);
    repeatNode->MoveData(0, 1);
    repeatNode->MoveData(1, 2);
    repeatNode->MoveData(2, 3);

    /**
     * @tc.steps: step1.
     * @tc.expected: index is 0, mappedIndex should be 3.
     */
    IndexType mappedIndex = repeatNode->caches_.ConvertFromToIndexRevert(0);
    EXPECT_EQ(mappedIndex, 3);

    /**
     * @tc.steps: step2.
     * @tc.expected: index is 1, mappedIndex should be 0.
     */
    mappedIndex = repeatNode->caches_.ConvertFromToIndexRevert(1);
    EXPECT_EQ(mappedIndex, 0);

    /**
     * @tc.steps: step3.
     * @tc.expected: index is 2, mappedIndex should be 1.
     */
    mappedIndex = repeatNode->caches_.ConvertFromToIndexRevert(2);
    EXPECT_EQ(mappedIndex, 1);

    /**
     * @tc.steps: step4.
     * @tc.expected: index is 3, mappedIndex should be 2.
     */
    mappedIndex = repeatNode->caches_.ConvertFromToIndexRevert(3);
    EXPECT_EQ(mappedIndex, 2);

    /**
     * @tc.steps: step5.
     * @tc.expected: index is 4, 5, 6, mappedIndex should be same with index.
     */
    mappedIndex = repeatNode->caches_.ConvertFromToIndexRevert(4);
    EXPECT_EQ(mappedIndex, 4);
    mappedIndex = repeatNode->caches_.ConvertFromToIndexRevert(5);
    EXPECT_EQ(mappedIndex, 5);
    mappedIndex = repeatNode->caches_.ConvertFromToIndexRevert(6);
    EXPECT_EQ(mappedIndex, 6);
}

/**
 * @tc.name: RecycleItems001
 * @tc.desc: Test node.RecycleItems
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RecycleItems001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    repeatNode->RecycleItems(0, 5);
    EXPECT_EQ(repeatNode->prevRecycleFrom_, 0);
    EXPECT_EQ(repeatNode->prevRecycleTo_, 5);
    repeatNode->RecycleItems(5, 0);
    EXPECT_EQ(repeatNode->prevRecycleFrom_, 0);
    EXPECT_EQ(repeatNode->prevRecycleTo_, 5);
}

/**
 * @tc.name: NotifyColorModeChange001
 * @tc.desc: Test caches.NotifyColorModeChange
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, NotifyColorModeChange001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    repeatNode->caches_.l1Rid4Index_ = {
        {0, 1}, {1, 2}, {2, 3}, {3, 4}
    };
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2016, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    repeatNode->caches_.cacheItem4Rid_ = {
        { 1, cacheItem }, { 2, cacheItem }, { 3, cacheItem }, { 4, cacheItem }
    };
    repeatNode->NotifyColorModeChange(1);
    EXPECT_TRUE(cacheItem->node_->measureAnyWay_);
}

/**
 * @tc.name: NotifyColorModeChange002
 * @tc.desc: Test caches.NotifyColorModeChange
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, NotifyColorModeChange002, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    repeatNode->caches_.l1Rid4Index_ = {
        {0, 1}
    };
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<CustomNode>(2016, "node");
    RefPtr<UINode> childNode = AceType::MakeRefPtr<CustomNode>(2016, "childNode");
    uiNode->children_ = { childNode };
    uiNode->SetDarkMode(true);
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    repeatNode->caches_.cacheItem4Rid_ = {
        { 1, cacheItem }
    };
    EXPECT_FALSE(childNode->CheckIsDarkMode());
    repeatNode->NotifyColorModeChange(1);
    EXPECT_TRUE(childNode->CheckIsDarkMode());
}

/**
 * @tc.name: UpdateFrameChildIndexRecord001
 * @tc.desc: Test node.updateFrameChildIndexRecord
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, UpdateFrameChildIndexRecord001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(6, 6);
    repeatNode->minFrameChildIndex_ = 0;
    repeatNode->maxFrameChildIndex_ = 0;
    repeatNode->needRecordFirstFrameChild_ = true;

    /**
     * @tc.steps: step1.
     * @tc.expected: minFrameChildIndex_ is 2, maxFrameChildIndex_ is 2, needRecordFirstFrameChild_ is false.
     */
    repeatNode->updateFrameChildIndexRecord(2);
    EXPECT_EQ(repeatNode->minFrameChildIndex_, 2);
    EXPECT_EQ(repeatNode->maxFrameChildIndex_, 2);
    EXPECT_EQ(repeatNode->needRecordFirstFrameChild_, false);

    /**
     * @tc.steps: step2.
     * @tc.expected: minFrameChildIndex_ is 1, maxFrameChildIndex_ is 3
     */
    repeatNode->updateFrameChildIndexRecord(1);
    repeatNode->updateFrameChildIndexRecord(3);
    EXPECT_EQ(repeatNode->minFrameChildIndex_, 1);
    EXPECT_EQ(repeatNode->maxFrameChildIndex_, 3);
}

/**
 * @tc.name: UpdateIsL1001
 * @tc.desc: Test caches.UpdateIsL1
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, UpdateIsL1001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    RefPtr<UINode> mockNode = AceType::MakeRefPtr<MockFrameNode>("node", 2017, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem0 = RepeatVirtualScroll2CacheItem::MakeCacheItem(mockNode, true);
    CacheItem cacheItem1 = RepeatVirtualScroll2CacheItem::MakeCacheItem(mockNode, true);
    /**
     * @tc.steps: step1. update cacheItem0 to non-L1 when node_ is nullptr
     * @tc.expected: isL1_ is false, recycledNodeIds_ size is 0
     */
    cacheItem0->node_ = nullptr;
    repeatNode->caches_.UpdateIsL1(cacheItem0, false);
    EXPECT_EQ(cacheItem0->isL1_, false);
    EXPECT_EQ(repeatNode->caches_.recycledNodeIds_.size(), 0);
    /**
     * @tc.steps: step2. update cacheItem1 to non-L1 when node_ is valid
     * @tc.expected: isL1_ is false, recycledNodeIds_ size is 1
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockFrameNode>(cacheItem1->node_)), OnRecycle())
        .Times(1); // expect OnRecycle called once
    repeatNode->caches_.UpdateIsL1(cacheItem1, false);
    EXPECT_EQ(cacheItem1->isL1_, false);
    EXPECT_EQ(repeatNode->caches_.recycledNodeIds_.size(), 1);
    /**
     * @tc.steps: step3. update cacheItem0 to L1
     * @tc.expected: isL1_ is true, recycledNodeIds_ size is 1
     */
    repeatNode->caches_.UpdateIsL1(cacheItem0, true);
    EXPECT_EQ(cacheItem0->isL1_, true);
    EXPECT_EQ(repeatNode->caches_.recycledNodeIds_.size(), 1);
    /**
     * @tc.steps: step4. update cacheItem1 to L1
     * @tc.expected: isL1_ is true, recycledNodeIds_ size is 0
     */
    EXPECT_CALL(*(AceType::DynamicCast<MockFrameNode>(cacheItem1->node_)), OnReuse())
        .Times(1); // expect OnReuse called once
    repeatNode->caches_.UpdateIsL1(cacheItem1, true);
    EXPECT_EQ(cacheItem1->isL1_, true);
    EXPECT_EQ(repeatNode->caches_.recycledNodeIds_.size(), 0);
    /**
     * @tc.steps: step5. update cacheItem1 to non-L1 again
     * @tc.expected: isL1_ is false, recycledNodeIds_ size is 1
     */
    repeatNode->caches_.UpdateIsL1(cacheItem1, false, false);
    EXPECT_EQ(cacheItem1->isL1_, false);
    EXPECT_EQ(repeatNode->caches_.recycledNodeIds_.size(), 0);
}

/**
 * @tc.name: UpdateL1Rid4Index001
 * @tc.desc: Test caches.UpdateL1Rid4Index
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, UpdateL1Rid4Index001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    RefPtr<UINode> uiNode0 = AceType::MakeRefPtr<FrameNode>("node", 2018, AceType::MakeRefPtr<Pattern>());
    RefPtr<UINode> uiNode1 = AceType::MakeRefPtr<FrameNode>("node", 2019, AceType::MakeRefPtr<Pattern>());
    RefPtr<UINode> uiNode2 = AceType::MakeRefPtr<FrameNode>("node", 2020, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem0 = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode0, true);
    CacheItem cacheItem1 = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode1, true);
    CacheItem cacheItem2 = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode2, false);
    repeatNode->caches_.cacheItem4Rid_ = {
        { 1, cacheItem0 }, { 2, cacheItem1 }, { 3, cacheItem2 }
    };
    repeatNode->caches_.UpdateL1Rid4Index({ { 1, 2 }, { 2, 3 } });
    EXPECT_EQ(cacheItem0->isL1_, false);
    EXPECT_EQ(cacheItem1->isL1_, true);
    EXPECT_EQ(cacheItem2->isL1_, true);
}

/**
 * @tc.name: GetFrameChildByIndexImpl001
 * @tc.desc: Test node.GetFrameChildByIndexImpl
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, GetFrameChildByIndexImpl001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2021, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    repeatNode->caches_.cacheItem4Rid_ = { { 1, cacheItem } };
    repeatNode->caches_.l1Rid4Index_ = { { 0, 1} };
    repeatNode->onMainTree_ = true;
    repeatNode->AddDisappearingChild(uiNode);
    EXPECT_EQ(repeatNode->disappearingChildren_.size(), 1);
    repeatNode->GetFrameChildByIndexImpl(0, true, true, false);
    EXPECT_EQ(repeatNode->disappearingChildren_.size(), 0);
}

/**
 * @tc.name: IsAllowAnimation001
 * @tc.desc: Test node.IsAllowAnimation
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, IsAllowAnimation001, TestSize.Level1)
{
    auto listNode = CreateNode(V2::LIST_ETS_TAG);
    auto repeatNode = CreateRepeatVirtualNode(10, 10);

    listNode->AddChild(repeatNode);
    EXPECT_EQ(repeatNode->IsAllowAnimation(), true);
}

/**
 * @tc.name: IsAllowAnimation002
 * @tc.desc: Test node.IsAllowAnimation
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, IsAllowAnimation002, TestSize.Level1)
{
    auto gridNode = CreateNode(V2::GRID_ETS_TAG);
    auto repeatNode = CreateRepeatVirtualNode(10, 10);

    gridNode->AddChild(repeatNode);
    EXPECT_EQ(repeatNode->IsAllowAnimation(), false);
}

/**
 * @tc.name: IsAllowAnimation003
 * @tc.desc: Test node.IsAllowAnimation when parent is null
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, IsAllowAnimation003, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    EXPECT_EQ(repeatNode->IsAllowAnimation(), false);
}

/**
 * @tc.name: IsAllowAnimation004
 * @tc.desc: Test node.IsAllowAnimation when enableRepeatAnimation is true and parent is List
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, IsAllowAnimation004, TestSize.Level1)
{
    auto listNode = CreateNode(V2::LIST_ETS_TAG);
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    listNode->AddChild(repeatNode);
    EXPECT_EQ(repeatNode->IsAllowAnimation(), true);
}

/**
 * @tc.name: IsAllowAnimation005
 * @tc.desc: Test node.IsAllowAnimation when enableRepeatAnimation is false even if parent is List
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, IsAllowAnimation005, TestSize.Level1)
{
    auto gridNode = CreateNode(V2::GRID_ETS_TAG);
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    gridNode->AddChild(repeatNode);
    EXPECT_EQ(repeatNode->IsAllowAnimation(), false);
}

/**
 * @tc.name: IsChildInAnimation001
 * @tc.desc: Test node.IsChildInAnimation
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, IsChildInAnimation001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    EXPECT_EQ(repeatNode->IsChildInAnimation(0), false);
}

/**
 * @tc.name: IsChildOnMainTree001
 * @tc.desc: Test node.IsChildOnMainTree
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, IsChildOnMainTree001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2022, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    repeatNode->caches_.cacheItem4Rid_ = { { 1, cacheItem } };
    repeatNode->caches_.l1Rid4Index_ = { { 0, 1} };
    uiNode->onMainTree_ = true;
    EXPECT_EQ(repeatNode->IsChildOnMainTree(1), true);
    uiNode->onMainTree_ = false;
    EXPECT_EQ(repeatNode->IsChildOnMainTree(1), false);
    EXPECT_EQ(repeatNode->IsChildOnMainTree(2), false);
}

/**
 * @tc.name: ModelIsAllowAnimation001
 * @tc.desc: Test model.IsAllowAnimation
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, ModelIsAllowAnimation001, TestSize.Level1)
{
    auto listNode = CreateNode(V2::LIST_ETS_TAG);
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    auto repeatId = elmtId_;
    listNode->AddChild(repeatNode);
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2023, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    repeatNode->caches_.cacheItem4Rid_ = { { 1, cacheItem } };
    repeatNode->caches_.l1Rid4Index_ = { { 0, 1} };
    uiNode->onMainTree_ = true;

    RepeatVirtualScroll2ModelNG repeatModel;
    EXPECT_EQ(repeatModel.IsAllowAnimation(repeatId), true);
    EXPECT_EQ(repeatModel.IsAllowAnimation(0), false);

    ViewStackProcessor::GetInstance()->Push(repeatNode);
    EXPECT_EQ(repeatModel.IsAllowAnimation(0), true);
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: ModelIsAllowAnimation002
 * @tc.desc: Test model.IsAllowAnimation
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, ModelIsAllowAnimation002, TestSize.Level1)
{
    auto gridNode = CreateNode(V2::GRID_ETS_TAG);
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    auto repeatId = elmtId_;
    gridNode->AddChild(repeatNode);
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2024, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    repeatNode->caches_.cacheItem4Rid_ = { { 1, cacheItem } };
    repeatNode->caches_.l1Rid4Index_ = { { 0, 1} };
    uiNode->onMainTree_ = true;

    RepeatVirtualScroll2ModelNG repeatModel;
    EXPECT_EQ(repeatModel.IsAllowAnimation(repeatId), false);
    EXPECT_EQ(repeatModel.IsAllowAnimation(0), false);

    ViewStackProcessor::GetInstance()->Push(repeatNode);
    EXPECT_EQ(repeatModel.IsAllowAnimation(0), false);
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: ModelIsAllowAnimation003
 * @tc.desc: Test model.IsAllowAnimation
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, ModelIsAllowAnimation003, TestSize.Level1)
{
    auto swiperNode = CreateNode(V2::SWIPER_ETS_TAG);
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    auto repeatId = elmtId_;
    swiperNode->AddChild(repeatNode);
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2024, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    repeatNode->caches_.cacheItem4Rid_ = { { 1, cacheItem } };
    repeatNode->caches_.l1Rid4Index_ = { { 0, 1} };
    uiNode->onMainTree_ = true;

    RepeatVirtualScroll2ModelNG repeatModel;
    EXPECT_EQ(repeatModel.IsAllowAnimation(repeatId), false);
    EXPECT_EQ(repeatModel.IsAllowAnimation(0), false);

    ViewStackProcessor::GetInstance()->Push(repeatNode);
    EXPECT_EQ(repeatModel.IsAllowAnimation(0), false);
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: ModelIsImplicitAnimationOpen001
 * @tc.desc: Test model.IsImplicitAnimationOpen
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, ModelIsImplicitAnimationOpen001, TestSize.Level1)
{
    RepeatVirtualScroll2ModelNG repeatModel;
    EXPECT_EQ(repeatModel.IsImplicitAnimationOpen(), AnimationUtils::IsImplicitAnimationOpen());
}

/**
 * @tc.name: ModelIsChildInAnimation001
 * @tc.desc: Test model.IsChildInAnimation
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, ModelIsChildInAnimation001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    auto repeatId = elmtId_;
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2025, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    repeatNode->caches_.cacheItem4Rid_ = { { 1, cacheItem } };
    repeatNode->caches_.l1Rid4Index_ = { { 0, 1} };
    uiNode->onMainTree_ = true;

    RepeatVirtualScroll2ModelNG repeatModel;
    EXPECT_EQ(repeatModel.IsChildInAnimation(repeatId, 1), false);
    EXPECT_EQ(repeatModel.IsChildInAnimation(0, 1), false);

    ViewStackProcessor::GetInstance()->Push(repeatNode);
    EXPECT_EQ(repeatModel.IsChildInAnimation(0, 1), false);
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: ModelIsChildOnMainTree001
 * @tc.desc: Test model.IsChildOnMainTree
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, ModelIsChildOnMainTree001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2026, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    repeatNode->caches_.cacheItem4Rid_ = { { 1, cacheItem } };
    repeatNode->caches_.l1Rid4Index_ = { { 0, 1} };
    uiNode->onMainTree_ = true;

    RepeatVirtualScroll2ModelNG repeatModel;
    EXPECT_EQ(repeatModel.IsChildOnMainTree(0, 1), false);

    ViewStackProcessor::GetInstance()->Push(repeatNode);
    EXPECT_EQ(repeatModel.IsChildOnMainTree(0, 1), true);
    ViewStackProcessor::GetInstance()->Finish();
}

/**
 * @tc.name: RepeatVirtualScroll2NodeOnWindowShow001
 * @tc.desc: Test OnWindowShow schedules restore cache task
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeOnWindowShow001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);
    ASSERT_FALSE(repeatNode->pendingRestoreCache_);

    /**
     * @tc.steps: step2. Call OnWindowShow
     * @tc.expected: Should schedule restore cache task
     */
    repeatNode->OnWindowShow();

    EXPECT_TRUE(repeatNode->pendingRestoreCache_);
    EXPECT_FALSE(repeatNode->pendingCleanCache_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeOnWindowHide001
 * @tc.desc: Test OnWindowHide triggers synchronous cache cleanup
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeOnWindowHide001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);

    repeatNode->pendingCleanCache_ = true;
    repeatNode->pendingRestoreCache_ = true;

    /**
     * @tc.steps: step2. Call OnWindowHide
     * @tc.expected: Should call CleanCache with syncClean=true and reset flags
     */
    repeatNode->OnWindowHide();

    EXPECT_FALSE(repeatNode->pendingCleanCache_);
    EXPECT_FALSE(repeatNode->pendingRestoreCache_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeOnNotifyMemoryLevel001
 * @tc.desc: Test OnNotifyMemoryLevel with MEMORY_LEVEL_LOW
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeOnNotifyMemoryLevel001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);

    repeatNode->pendingCleanCache_ = true;
    repeatNode->pendingRestoreCache_ = true;

    /**
     * @tc.steps: step2. Call OnNotifyMemoryLevel with MEMORY_LEVEL_LOW
     * @tc.expected: Should call CleanCache with async mode
     */
    repeatNode->OnNotifyMemoryLevel(MEMORY_LEVEL_LOW);

    EXPECT_FALSE(repeatNode->pendingCleanCache_);
    EXPECT_FALSE(repeatNode->pendingRestoreCache_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeOnNotifyMemoryLevel002
 * @tc.desc: Test OnNotifyMemoryLevel with MEMORY_LEVEL_CRITICAL
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeOnNotifyMemoryLevel002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);

    repeatNode->pendingCleanCache_ = true;

    /**
     * @tc.steps: step2. Call OnNotifyMemoryLevel with MEMORY_LEVEL_CRITICAL
     * @tc.expected: Should call CleanCache with async mode
     */
    repeatNode->OnNotifyMemoryLevel(MEMORY_LEVEL_CRITICAL);

    EXPECT_FALSE(repeatNode->pendingCleanCache_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeOnNotifyMemoryLevel003
 * @tc.desc: Test OnNotifyMemoryLevel with normal level (no action)
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeOnNotifyMemoryLevel003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);

    repeatNode->pendingCleanCache_ = true;

    /**
     * @tc.steps: step2. Call OnNotifyMemoryLevel with normal level
     * @tc.expected: Should NOT call CleanCache
     */
    repeatNode->OnNotifyMemoryLevel(MEMORY_LEVEL_NORMAL);

    EXPECT_TRUE(repeatNode->pendingCleanCache_); // Flag should remain unchanged
}

/**
 * @tc.name: RepeatVirtualScroll2NodeScheduleCleanCacheTask001
 * @tc.desc: Test ScheduleCleanCacheTask when pendingCleanCache_ is false
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeScheduleCleanCacheTask001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node with pendingCleanCache_ = false
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);

    repeatNode->pendingCleanCache_ = false;
    repeatNode->pendingRestoreCache_ = true; // Should be cleared

    /**
     * @tc.steps: step2. Call ScheduleCleanCacheTask
     * @tc.expected: pendingCleanCache_ should be set to true, pendingRestoreCache_ cleared, timestamp recorded
     */
    int64_t beforeCall = GetSysTimestamp();
    repeatNode->ScheduleCleanCacheTask();
    int64_t afterCall = GetSysTimestamp();

    EXPECT_TRUE(repeatNode->pendingCleanCache_);
    EXPECT_FALSE(repeatNode->pendingRestoreCache_);
    EXPECT_GE(repeatNode->cacheTaskPostTime_, beforeCall);
    EXPECT_LE(repeatNode->cacheTaskPostTime_, afterCall);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeScheduleCleanCacheTask002
 * @tc.desc: Test ScheduleCleanCacheTask when pendingCleanCache_ is already true
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeScheduleCleanCacheTask002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node with pendingCleanCache_ = true
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);
    repeatNode->pendingCleanCache_ = true;
    int64_t originalTimestamp = repeatNode->cacheTaskPostTime_;

    /**
     * @tc.steps: step2. Call ScheduleCleanCacheTask again
     * @tc.expected: Should return early without modifying timestamp (CHECK_EQUAL_VOID)
     */
    repeatNode->ScheduleCleanCacheTask();

    EXPECT_TRUE(repeatNode->pendingCleanCache_);
    EXPECT_EQ(repeatNode->cacheTaskPostTime_, originalTimestamp);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeScheduleRestoreCacheTask001
 * @tc.desc: Test ScheduleRestoreCacheTask when pendingRestoreCache_ is false
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeScheduleRestoreCacheTask001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node with pendingRestoreCache_ = false
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);
    ASSERT_FALSE(repeatNode->pendingRestoreCache_);

    repeatNode->pendingCleanCache_ = true; // Should be cleared

    /**
     * @tc.steps: step2. Call ScheduleRestoreCacheTask
     * @tc.expected: pendingRestoreCache_ should be set to true, pendingCleanCache_ cleared, timestamp recorded
     */
    int64_t beforeCall = GetSysTimestamp();
    repeatNode->ScheduleRestoreCacheTask();
    int64_t afterCall = GetSysTimestamp();

    EXPECT_TRUE(repeatNode->pendingRestoreCache_);
    EXPECT_FALSE(repeatNode->pendingCleanCache_);
    EXPECT_GE(repeatNode->cacheTaskPostTime_, beforeCall);
    EXPECT_LE(repeatNode->cacheTaskPostTime_, afterCall);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeScheduleRestoreCacheTask002
 * @tc.desc: Test ScheduleRestoreCacheTask when pendingRestoreCache_ is already true
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeScheduleRestoreCacheTask002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node with pendingRestoreCache_ = true
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);
    repeatNode->pendingRestoreCache_ = true;
    int64_t originalTimestamp = repeatNode->cacheTaskPostTime_;

    /**
     * @tc.steps: step2. Call ScheduleRestoreCacheTask again
     * @tc.expected: Should return early without modifying timestamp (CHECK_EQUAL_VOID)
     */
    repeatNode->ScheduleRestoreCacheTask();

    EXPECT_TRUE(repeatNode->pendingRestoreCache_);
    EXPECT_EQ(repeatNode->cacheTaskPostTime_, originalTimestamp);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeTryExecuteScheduledCacheTask001
 * @tc.desc: Test TryExecuteScheduledCacheTask when no tasks are pending
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeTryExecuteScheduledCacheTask001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node with no pending tasks
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);
    repeatNode->pendingCleanCache_ = false;
    repeatNode->pendingRestoreCache_ = false;
    repeatNode->postUpdateTaskHasBeenScheduled_ = false;

    /**
     * @tc.steps: step2. Call TryExecuteScheduledCacheTask
     * @tc.expected: Should return early without executing (CHECK_EQUAL_VOID)
     */
    repeatNode->TryExecuteScheduledCacheTask();

    // No IdleTask posted
    EXPECT_FALSE(repeatNode->postUpdateTaskHasBeenScheduled_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeTryExecuteScheduledCacheTask002
 * @tc.desc: Test TryExecuteScheduledCacheTask when delay time hasn't elapsed
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeTryExecuteScheduledCacheTask002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node and schedule a task
     * @tc.expected: RepeatVirtualScroll2Node is created with pending task
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);
    repeatNode->ScheduleCleanCacheTask();

    /**
     * @tc.steps: step2. Call TryExecuteScheduledCacheTask immediately
     * @tc.expected: Should return early without executing (CHECK_EQUAL_VOID)
     */
    repeatNode->TryExecuteScheduledCacheTask();

    EXPECT_TRUE(repeatNode->pendingCleanCache_); // Flag should remain
}

/**
 * @tc.name: RepeatVirtualScroll2NodeTryExecuteScheduledCacheTask003
 * @tc.desc: Test TryExecuteScheduledCacheTask executes clean cache task
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeTryExecuteScheduledCacheTask003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node and schedule a task with old timestamp
     * @tc.expected: RepeatVirtualScroll2Node is created with pending task
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);
    repeatNode->ScheduleCleanCacheTask();

    /**
     * @tc.steps: step2. Manually set timestamps to simulate elapsed delay
     * @tc.expected: Timestamps are set to past
     */
    repeatNode->cacheTaskPostTime_ = GetSysTimestamp() - CACHE_TASK_DELAY_TIME - 1000000;
    repeatNode->setActiveRangeTime_ = GetSysTimestamp() - CACHE_TASK_DELAY_TIME - 1000000;

    /**
     * @tc.steps: step3. Call TryExecuteScheduledCacheTask
     * @tc.expected: Should call CleanCache and reset flags
     */
    repeatNode->TryExecuteScheduledCacheTask();

    EXPECT_FALSE(repeatNode->pendingCleanCache_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeTryExecuteScheduledCacheTask004
 * @tc.desc: Test TryExecuteScheduledCacheTask executes restore cache task
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeTryExecuteScheduledCacheTask004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node and schedule a task with old timestamp
     * @tc.expected: RepeatVirtualScroll2Node is created with pending task
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);
    repeatNode->ScheduleRestoreCacheTask();

    /**
     * @tc.steps: step2. Manually set timestamps to simulate elapsed delay
     * @tc.expected: Timestamps are set to past
     */
    repeatNode->cacheTaskPostTime_ = GetSysTimestamp() - CACHE_TASK_DELAY_TIME - 1000000;
    repeatNode->setActiveRangeTime_ = GetSysTimestamp() - CACHE_TASK_DELAY_TIME - 1000000;

    /**
     * @tc.steps: step3. Call TryExecuteScheduledCacheTask
     * @tc.expected: Should call CleanCache and reset flags
     */
    repeatNode->TryExecuteScheduledCacheTask();

    EXPECT_FALSE(repeatNode->pendingRestoreCache_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeCleanCache001
 * @tc.desc: Test CleanCache with syncClean=true
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeCleanCache001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);

    repeatNode->pendingCleanCache_ = true;
    repeatNode->pendingRestoreCache_ = true;
    repeatNode->restoringCache_ = true;
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2026, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    repeatNode->caches_.cacheItem4Rid_ = { { 1, cacheItem } };
    repeatNode->pendingRemoveRids_ = { 1, 2 };

    /**
     * @tc.steps: step2. Call CleanCache with syncClean=true
     * @tc.expected: Should reset flags
     */
    repeatNode->CleanCache(true);

    EXPECT_TRUE(repeatNode->pendingRemoveRids_.empty());
    EXPECT_FALSE(repeatNode->pendingCleanCache_);
    EXPECT_FALSE(repeatNode->pendingRestoreCache_);
    EXPECT_FALSE(repeatNode->restoringCache_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeCleanCache002
 * @tc.desc: Test CleanCache with syncClean=false (async mode)
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeCleanCache002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);

    repeatNode->pendingCleanCache_ = true;
    repeatNode->pendingRestoreCache_ = true;
    repeatNode->restoringCache_ = true;
    repeatNode->pendingRemoveRids_ = { 1, 2 };

    /**
     * @tc.steps: step2. Call CleanCache with syncClean=false
     * @tc.expected: Should reset flags
     */
    repeatNode->CleanCache(false);

    EXPECT_FALSE(repeatNode->pendingRemoveRids_.empty());
    EXPECT_FALSE(repeatNode->pendingCleanCache_);
    EXPECT_FALSE(repeatNode->pendingRestoreCache_);
    EXPECT_FALSE(repeatNode->restoringCache_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeStartRestoreCache001
 * @tc.desc: Test StartRestoreCache
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeStartRestoreCache001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);

    repeatNode->pendingCleanCache_ = true;
    repeatNode->pendingRestoreCache_ = true;

    /**
     * @tc.steps: step2. Call StartRestoreCache
     * @tc.expected: Should start cache restore and reset pending flags
     */
    repeatNode->StartRestoreCache();

    EXPECT_FALSE(repeatNode->pendingCleanCache_);
    EXPECT_FALSE(repeatNode->pendingRestoreCache_);
    EXPECT_TRUE(repeatNode->restoringCache_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeRestoreCache001
 * @tc.desc: Test RestoreCache when canUseLongPredictTask is false
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeRestoreCache001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);

    repeatNode->restoringCache_ = true;
    repeatNode->cleanedCacheIndexes_ = {1, 2, 3};

    /**
     * @tc.steps: step2. Call RestoreCache with canUseLongPredictTask=false
     * @tc.expected: Should remain in restoring state
     */
    repeatNode->RestoreCache(GetSysTimestamp() + 1000000, false);

    EXPECT_TRUE(repeatNode->restoringCache_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeRestoreCache002
 * @tc.desc: Test RestoreCache with empty cleanedCacheIndexes_
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeRestoreCache002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);

    repeatNode->restoringCache_ = true;
    repeatNode->cleanedCacheIndexes_.clear();

    /**
     * @tc.steps: step2. Call RestoreCache
     * @tc.expected: Should complete restore
     */
    repeatNode->RestoreCache(GetSysTimestamp() + 1000000, true);

    EXPECT_FALSE(repeatNode->restoringCache_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeAddPendingRemoveNodes001
 * @tc.desc: Test AddPendingRemoveNodes with nodes
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeAddPendingRemoveNodes001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);

    std::vector<uint32_t> rids = {1, 2, 3};
    std::vector<int32_t> indexes = {10, 20, 30};
    repeatNode->pendingRemoveRids_.clear();
    repeatNode->cleanedCacheIndexes_.clear();

    /**
     * @tc.steps: step2. Call AddPendingRemoveNodes
     * @tc.expected: Should add nodes to pendingRemoveRids_
     */
    repeatNode->AddPendingRemoveNodes(rids, indexes);

    EXPECT_EQ(repeatNode->pendingRemoveRids_.size(), 3);
    EXPECT_EQ(repeatNode->cleanedCacheIndexes_.size(), 3);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeRemovingExpiringItem001
 * @tc.desc: Test RemovingExpiringItem with empty pendingRemoveRids_
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeRemovingExpiringItem001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);
    repeatNode->pendingRemoveRids_.clear();

    /**
     * @tc.steps: step2. Call RemovingExpiringItem with empty pendingRemoveRids_
     * @tc.expected: Should return true
     */
    bool result = repeatNode->RemovingExpiringItem(GetSysTimestamp() + 1000000);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeRemovingExpiringItem002
 * @tc.desc: Test RemovingExpiringItem with pending rids
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeRemovingExpiringItem002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node
     * @tc.expected: RepeatVirtualScroll2Node is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);

    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 2026, AceType::MakeRefPtr<Pattern>());
    CacheItem cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    repeatNode->caches_.cacheItem4Rid_ = { { 1, cacheItem } };

    repeatNode->pendingRemoveRids_ = {1, 2, 3};

    /**
     * @tc.steps: step2. Call RemovingExpiringItem
     * @tc.expected: Should remove at least one item
     */

    repeatNode->RemovingExpiringItem(GetSysTimestamp() + 1000000000);

    // After removal, pendingRemoveRids_ should be smaller
    EXPECT_LT(repeatNode->pendingRemoveRids_.size(), 3);
}

/**
* @tc.name: SetEnableSyncLoadTest001
* @tc.desc: Test RepeatVirtualScroll2Caches::SetEnableSyncLoad
* @tc.type: FUNC
*/
HWTEST_F(RepeatVirtual2TestNg, SetEnableSyncLoadTest001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);

    repeatNode->caches_.SetEnableSyncLoad(true);
    EXPECT_EQ(repeatNode->caches_.enableSyncLoad_, true);

    repeatNode->caches_.SetEnableSyncLoad(false);
    EXPECT_EQ(repeatNode->caches_.enableSyncLoad_, false);
}

/**
* @tc.name: SetIsSyncLoadTest001
* @tc.desc: Test RepeatVirtualScroll2Caches::SetIsSyncLoad with true value
* @tc.type: FUNC
*/
HWTEST_F(RepeatVirtual2TestNg, SetIsSyncLoadTest001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);

    repeatNode->caches_.SetIsSyncLoad(true);
    EXPECT_EQ(repeatNode->caches_.isSyncLoad_, true);

    repeatNode->caches_.SetIsSyncLoad(false);
    EXPECT_EQ(repeatNode->caches_.isSyncLoad_, false);
}

/**
 * @tc.name: ProcessSyncLoadTempChildrenTest001
 * @tc.desc: Test RepeatVirtualScroll2Caches::ProcessSyncLoadTempChildren when enableSyncLoad is true
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, ProcessSyncLoadTempChildrenTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create RepeatVirtualNode
     * @tc.expected: RepeatVirtualNode is created successfully
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);
    std::list<RefPtr<UINode>> children;
    RefPtr<UINode> node = AceType::MakeRefPtr<FrameNode>("node", 2027, AceType::MakeRefPtr<Pattern>());

    repeatNode->caches_.SetEnableSyncLoad(true);
    repeatNode->caches_.SetIsSyncLoad(true);
    repeatNode->caches_.syncLoadCache_.try_emplace(0, AceType::WeakClaim(AceType::RawPtr(node)));
    repeatNode->caches_.ProcessSyncLoadTempChildren(children, 0, 5);
    EXPECT_TRUE(children.empty());

    repeatNode->caches_.SetEnableSyncLoad(true);
    repeatNode->caches_.SetIsSyncLoad(false);
    repeatNode->caches_.syncLoadCache_.try_emplace(0, AceType::WeakClaim(AceType::RawPtr(node)));
    repeatNode->caches_.ProcessSyncLoadTempChildren(children, 0, 5);
    EXPECT_TRUE(children.empty());

    repeatNode->caches_.SetEnableSyncLoad(false);
    repeatNode->caches_.SetIsSyncLoad(true);
    repeatNode->caches_.syncLoadCache_.try_emplace(0, AceType::WeakClaim(AceType::RawPtr(node)));
    repeatNode->caches_.ProcessSyncLoadTempChildren(children, 0, 5);
    EXPECT_TRUE(children.empty());

    repeatNode->caches_.SetEnableSyncLoad(false);
    repeatNode->caches_.SetIsSyncLoad(false);
    repeatNode->caches_.syncLoadCache_.try_emplace(0, AceType::WeakClaim(AceType::RawPtr(node)));
    repeatNode->caches_.ProcessSyncLoadTempChildren(children, 0, 5);
    EXPECT_FALSE(children.empty());
}

/**
* @tc.name: LazyForEachUtilsSetRepeatMemOptStrategy001
* @tc.desc: Test LazyForEachUtils::SetRepeatMemOptStrategy
* @tc.type: FUNC
*/
HWTEST_F(RepeatVirtual2TestNg, LazyForEachUtilsSetRepeatMemOptStrategy001, TestSize.Level1)
{
    LazyForEachUtils::SetRepeatMemOptStrategy("ENABLE_AUTO_CACHE_OPTIMIZATION");
    EXPECT_EQ(LazyForEachUtils::GetRepeatMemOptStrategy(),
        RepeatMemOptStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION);
    LazyForEachUtils::SetRepeatMemOptStrategy("DEFAULT");
    EXPECT_EQ(LazyForEachUtils::GetRepeatMemOptStrategy(), RepeatMemOptStrategy::DEFAULT);
    LazyForEachUtils::SetRepeatMemOptStrategy("INVALID_STRATEGY");
    EXPECT_EQ(LazyForEachUtils::GetRepeatMemOptStrategy(), RepeatMemOptStrategy::DEFAULT);
    LazyForEachUtils::SetRepeatMemOptStrategy("");
    EXPECT_EQ(LazyForEachUtils::GetRepeatMemOptStrategy(), RepeatMemOptStrategy::DEFAULT);
}

/**
* @tc.name: RepeatVirtualScroll2NodeGetMemOptStrategy001
* @tc.desc: Test RepeatVirtualScroll2Node::GetMemOptStrategy
* @tc.type: FUNC
*/
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeGetMemOptStrategy001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);

    repeatNode->memOptStrategy_ = RepeatMemOptStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION;
    auto strategy = repeatNode->GetMemOptStrategy();
    EXPECT_EQ(strategy, RepeatMemOptStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION);

    repeatNode->memOptStrategy_ = RepeatMemOptStrategy::UNDEFINED;
    LazyForEachUtils::repeatMemOptStrategy_ = RepeatMemOptStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION;
    strategy = repeatNode->GetMemOptStrategy();
    EXPECT_EQ(strategy, RepeatMemOptStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION);

    repeatNode->memOptStrategy_ = RepeatMemOptStrategy::UNDEFINED;
    LazyForEachUtils::repeatMemOptStrategy_ = RepeatMemOptStrategy::UNDEFINED;
    SystemProperties::syntaxMemOptStrategy_ = 1;
    strategy = repeatNode->GetMemOptStrategy();
    EXPECT_EQ(strategy, RepeatMemOptStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION);

    repeatNode->memOptStrategy_ = RepeatMemOptStrategy::UNDEFINED;
    SystemProperties::syntaxMemOptStrategy_ = -1;
    strategy = repeatNode->GetMemOptStrategy();
    EXPECT_EQ(strategy, RepeatMemOptStrategy::DEFAULT);
}

/**
 * @tc.name: TryBindController001
 * @tc.desc: Test RepeatVirtualScroll2Node TryBindController accepts only the first binding
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, TryBindController001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node
     * @tc.expected: Node is created unbound
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);
    // ElementRegister is not cleared between tests; reset to test the first-bind path.
    repeatNode->controllerBound_ = false;
    EXPECT_FALSE(repeatNode->controllerBound_);
 
    /**
     * @tc.steps: step2. Call TryBindController for the first time
     * @tc.expected: The first binding is accepted (creation-time immutable binding)
     */
    EXPECT_TRUE(repeatNode->TryBindController());
    EXPECT_TRUE(repeatNode->controllerBound_);
 
    /**
     * @tc.steps: step3. Call TryBindController again
     * @tc.expected: Later binds on an already bound node are ignored
     */
    EXPECT_FALSE(repeatNode->TryBindController());
    EXPECT_TRUE(repeatNode->controllerBound_);
}
 
/**
 * @tc.name: SetController002
 * @tc.desc: Test RepeatVirtualScroll2ModelNG::SetController via ElementRegister fallback path
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, SetController002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node (registered in ElementRegister)
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);
    auto repeatElmtId = repeatNode->GetId();
    // Reset binding state polluted by earlier tests sharing the same nodeId via ElementRegister.
    repeatNode->controllerBound_ = false;
 
    /**
     * @tc.steps: step2. Clear ViewStackProcessor so stack->GetMainElementNode() returns nullptr
     * @tc.expected: SetController falls back to ElementRegister lookup
     */
    ViewStackProcessor::GetInstance()->ClearStack();
 
    RefPtr<RepeatVirtualScroll2Node> capturedNode;
    RepeatVirtualScroll2ModelNG repeatModel;
    repeatModel.SetController(repeatElmtId,
        [&capturedNode](const RefPtr<RepeatVirtualScroll2Node>& node) {
            capturedNode = node;
        });
    EXPECT_EQ(capturedNode, repeatNode);
    EXPECT_TRUE(repeatNode->controllerBound_);
}
 
/**
 * @tc.name: SetController003
 * @tc.desc: Test RepeatVirtualScroll2ModelNG::SetController when node is not found
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, SetController003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Clear ViewStackProcessor and use a non-existent elmtId
     * @tc.expected: bindController callback is not invoked
     */
    ViewStackProcessor::GetInstance()->ClearStack();
 
    bool callbackCalled = false;
    RepeatVirtualScroll2ModelNG repeatModel;
    repeatModel.SetController(999999,
        [&callbackCalled](const RefPtr<RepeatVirtualScroll2Node>& node) {
            callbackCalled = true;
        });
    EXPECT_FALSE(callbackCalled);
}
 
/**
 * @tc.name: SetController004
 * @tc.desc: Test RepeatVirtualScroll2ModelNG::SetController when controller already bound
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, SetController004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a node and bind controller for the first time
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);
    // Reset binding state polluted by earlier tests sharing the same nodeId via ElementRegister.
    repeatNode->controllerBound_ = false;
 
    bool firstCallbackCalled = false;
    RepeatVirtualScroll2ModelNG repeatModel;
    repeatModel.SetController(repeatNode->GetId(),
        [&firstCallbackCalled](const RefPtr<RepeatVirtualScroll2Node>& node) {
            firstCallbackCalled = true;
        });
    EXPECT_TRUE(firstCallbackCalled);
    EXPECT_TRUE(repeatNode->controllerBound_);
 
    /**
     * @tc.steps: step2. Call SetController again on the already-bound node
     * @tc.expected: bindController callback is not invoked (first-bind-wins)
     */
    bool secondCallbackCalled = false;
    repeatModel.SetController(repeatNode->GetId(),
        [&secondCallbackCalled](const RefPtr<RepeatVirtualScroll2Node>& node) {
            secondCallbackCalled = true;
        });
    EXPECT_FALSE(secondCallbackCalled);
}
 
/**
 * @tc.name: RepeatControllerPattern001
 * @tc.desc: Test controller BindToNode with null node (CHECK_NULL_VOID branch)
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatControllerPattern001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Simulate JSRepeatController::BindToNode(nullptr)
     * @tc.expected: WeakPtr stays empty after null bind attempt
     */
    WeakPtr<RepeatVirtualScroll2Node> weakNode;
    auto nullNode = RefPtr<RepeatVirtualScroll2Node>(nullptr);
    // CHECK_NULL_VOID equivalent: do not set weak ref when node is null
    if (nullNode) {
        weakNode = nullNode;
    }
    EXPECT_TRUE(weakNode.Upgrade() == nullptr);
}
 
/**
 * @tc.name: RepeatControllerPattern002
 * @tc.desc: Test controller BindToNode with valid node, then ClearCache with live weak ref
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatControllerPattern002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node and bind via weak ref (controller pattern)
     */
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(repeatNode, nullptr);
 
    WeakPtr<RepeatVirtualScroll2Node> weakNode;
    // BindToNode: CHECK_NULL_VOID then set weak ref
    if (repeatNode) {
        weakNode = repeatNode;
    }
    EXPECT_EQ(weakNode.Upgrade(), repeatNode);
 
    /**
     * @tc.steps: step2. ClearCache: upgrade weak ref, node is alive → call node->CleanCache(false)
     * @tc.expected: CleanCache executes, pending flags reset, restoringCache_ false
     */
    repeatNode->pendingCleanCache_ = true;
    repeatNode->pendingRestoreCache_ = true;
    repeatNode->restoringCache_ = true;
    auto node = weakNode.Upgrade();
    ASSERT_TRUE(node);
    node->CleanCache(false);
    EXPECT_FALSE(repeatNode->pendingCleanCache_);
    EXPECT_FALSE(repeatNode->pendingRestoreCache_);
    EXPECT_FALSE(repeatNode->restoringCache_);
}
 
/**
 * @tc.name: RepeatControllerPattern003
 * @tc.desc: Test controller ClearCache with expired weak ref (safe no-op branch)
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatControllerPattern003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a RepeatVirtualScroll2Node, bind via weak ref, then release the strong ref
     */
    WeakPtr<RepeatVirtualScroll2Node> weakNode;
    {
        auto repeatNode = CreateRepeatVirtualNode(10, 10);
        ASSERT_NE(repeatNode, nullptr);
        if (repeatNode) {
            weakNode = repeatNode;
        }
        EXPECT_NE(weakNode.Upgrade(), nullptr);
    }
    /**
     * @tc.steps: step2. ClearCache: upgrade weak ref fails → safe no-op (spec R-6)
     * @tc.expected: No crash, no node method called
     */
    auto node = weakNode.Upgrade();
    EXPECT_EQ(node, nullptr);
}
 
/**
 * @tc.name: RepeatControllerPattern004
 * @tc.desc: Test controller last-bind-wins: rebinding replaces previous weak ref
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatControllerPattern004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create two RepeatVirtualScroll2Nodes, bind to first then second
     * @tc.expected: Weak ref points to the second node (last-bind-wins, spec R-8)
     */
    auto node1 = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(node1, nullptr);
    auto node2 = CreateRepeatVirtualNode(10, 10);
    ASSERT_NE(node2, nullptr);
 
    WeakPtr<RepeatVirtualScroll2Node> weakNode;
    if (node1) {
        weakNode = node1;
    }
    EXPECT_EQ(weakNode.Upgrade(), node1);
    // Rebind: last-bind-wins
    if (node2) {
        weakNode = node2;
    }
    EXPECT_EQ(weakNode.Upgrade(), node2);
    EXPECT_NE(weakNode.Upgrade(), node1);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeSetAllowReuseInAnimation001
 * @tc.desc: SetAllowReuseInAnimation stores the tri-state value
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeSetAllowReuseInAnimation001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    EXPECT_FALSE(repeatNode->isAllowReuseInAnimation_.has_value());
    repeatNode->SetAllowReuseInAnimation(false);
    ASSERT_TRUE(repeatNode->isAllowReuseInAnimation_.has_value());
    EXPECT_FALSE(repeatNode->isAllowReuseInAnimation_.value());
    repeatNode->SetAllowReuseInAnimation(true);
    ASSERT_TRUE(repeatNode->isAllowReuseInAnimation_.has_value());
    EXPECT_TRUE(repeatNode->isAllowReuseInAnimation_.value());
    repeatNode->SetAllowReuseInAnimation(std::nullopt);
    EXPECT_FALSE(repeatNode->isAllowReuseInAnimation_.has_value());
}

/**
 * @tc.name: RepeatVirtualScroll2NodeIsAllowAnimationPriority001
 * @tc.desc: Explicitly set allowReuseInAnimation has priority over the List/Grid parent default
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeIsAllowAnimationPriority001, TestSize.Level1)
{
    auto gridNode = CreateNode(V2::GRID_ETS_TAG);
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    gridNode->AddChild(repeatNode);
    // default: Grid parent -> false
    EXPECT_FALSE(repeatNode->IsAllowAnimation());
    // explicitly disallow reuse in animation -> animation allowed
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_TRUE(repeatNode->IsAllowAnimation());
    // explicitly allow reuse in animation -> animation disallowed
    repeatNode->SetAllowReuseInAnimation(true);
    EXPECT_FALSE(repeatNode->IsAllowAnimation());
    repeatNode->SetAllowReuseInAnimation(std::nullopt);
    EXPECT_FALSE(repeatNode->IsAllowAnimation());

    auto listNode = CreateNode(V2::LIST_ETS_TAG);
    auto listRepeatNode = CreateRepeatVirtualNode(10, 10);
    listNode->AddChild(listRepeatNode);
    // default: List parent -> true
    EXPECT_TRUE(listRepeatNode->IsAllowAnimation());
    // explicitly disallow reuse in animation: same result as the List default
    listRepeatNode->SetAllowReuseInAnimation(false);
    EXPECT_TRUE(listRepeatNode->IsAllowAnimation());
    // explicitly allow reuse in animation overrides the List default
    listRepeatNode->SetAllowReuseInAnimation(true);
    EXPECT_FALSE(listRepeatNode->IsAllowAnimation());
}

/**
 * @tc.name: RepeatVirtualScroll2NodeIsAllowAnimationPriority002
 * @tc.desc: Explicitly set allowReuseInAnimation has priority over the allowReuse id set
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeIsAllowAnimationPriority002, TestSize.Level1)
{
    const std::string inspectorId = "rvscroll2_allow_reuse_id";
    LazyForEachUtils::SetIdsForRepeatAnimationAllowReuse(inspectorId);
    auto listNode = CreateNode(V2::LIST_ETS_TAG);
    listNode->UpdateInspectorId(inspectorId);
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    listNode->AddChild(repeatNode);
    // default: List parent whose id is in the allowReuse set -> false
    EXPECT_FALSE(repeatNode->IsAllowAnimation());
    // explicitly disallow reuse in animation has the highest priority
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_TRUE(repeatNode->IsAllowAnimation());
    repeatNode->SetAllowReuseInAnimation(true);
    EXPECT_FALSE(repeatNode->IsAllowAnimation());
    LazyForEachUtils::SetIdsForRepeatAnimationAllowReuse("");
}

/**
 * @tc.name: RepeatVirtualScroll2NodeOnAttachSyncCache001
 * @tc.desc: Attach/detach holds and releases the sync-cache reference on a Grid parent
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeOnAttachSyncCache001, TestSize.Level1)
{
    auto gridNode = FrameNode::CreateFrameNode(V2::GRID_ETS_TAG, 9001, AceType::MakeRefPtr<GridPattern>());
    auto gridPattern = gridNode->GetPattern<GridPattern>();
    ASSERT_NE(gridPattern, nullptr);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    repeatNode->SetAllowReuseInAnimation(false);
    gridNode->AddChild(repeatNode);
    repeatNode->OnAttachToMainTree(false);
    EXPECT_TRUE(gridPattern->IsSyncCacheLayoutEnabled());
    repeatNode->OnDetachFromMainTree(false, nullptr);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());
}

/**
 * @tc.name: RepeatVirtualScroll2NodeOnAttachSyncCache002
 * @tc.desc: Attach/detach holds and releases the sync-cache reference on a WaterFlow parent
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeOnAttachSyncCache002, TestSize.Level1)
{
    auto waterFlowNode =
        FrameNode::CreateFrameNode(V2::WATERFLOW_ETS_TAG, 9002, AceType::MakeRefPtr<WaterFlowPattern>());
    auto waterFlowPattern = waterFlowNode->GetPattern<WaterFlowPattern>();
    ASSERT_NE(waterFlowPattern, nullptr);
    EXPECT_FALSE(waterFlowPattern->IsSyncCacheLayoutEnabled());
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    repeatNode->SetAllowReuseInAnimation(false);
    waterFlowNode->AddChild(repeatNode);
    repeatNode->OnAttachToMainTree(false);
    EXPECT_TRUE(waterFlowPattern->IsSyncCacheLayoutEnabled());
    repeatNode->OnDetachFromMainTree(false, nullptr);
    EXPECT_FALSE(waterFlowPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());
}

/**
 * @tc.name: RepeatVirtualScroll2NodeOnAttachSyncCache003
 * @tc.desc: Attach does not hold the sync-cache reference when the option is unset or true
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeOnAttachSyncCache003, TestSize.Level1)
{
    auto gridNode = FrameNode::CreateFrameNode(V2::GRID_ETS_TAG, 9003, AceType::MakeRefPtr<GridPattern>());
    auto gridPattern = gridNode->GetPattern<GridPattern>();
    ASSERT_NE(gridPattern, nullptr);
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    gridNode->AddChild(repeatNode);

    // unset option keeps the container default
    repeatNode->SetAllowReuseInAnimation(std::nullopt);
    repeatNode->OnAttachToMainTree(false);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());
    repeatNode->OnDetachFromMainTree(false, nullptr);

    // explicitly true keeps the container default
    repeatNode->SetAllowReuseInAnimation(true);
    repeatNode->OnAttachToMainTree(false);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());
    repeatNode->OnDetachFromMainTree(false, nullptr);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeSetFrameNodeInactive001
 * @tc.desc: SetFrameNodeInactive applies SetActive(false) immediately while no implicit
 *           animation is open, independent of removeFromTree and oldIndex
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeSetFrameNodeInactive001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    // the deferral branch reads the previously recorded visible range members
    // (prevVisibleRangeStart_/prevVisibleRangeEnd_), not the active range ones
    repeatNode->prevVisibleRangeStart_ = 0;
    repeatNode->prevVisibleRangeEnd_ = 3;
    // null frame node: no crash
    repeatNode->SetFrameNodeInactive(nullptr, true, std::optional<IndexType>(0));

    auto frameNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 9100, AceType::MakeRefPtr<Pattern>());
    // removeFromTree && oldIndex unset
    frameNode->isActive_ = true;
    repeatNode->SetFrameNodeInactive(frameNode, true, std::nullopt);
    EXPECT_FALSE(frameNode->IsActive());
    EXPECT_FALSE(frameNode->needSetInActiveAfterTransitionOut_);

    // removeFromTree && oldIndex outside the active range
    frameNode->isActive_ = true;
    repeatNode->SetFrameNodeInactive(frameNode, true, std::optional<IndexType>(5));
    EXPECT_FALSE(frameNode->IsActive());

    // removeFromTree && oldIndex inside the active range: the deferral branch needs an open
    // implicit animation (IsImplicitAnimationOpen is a constant false in the test build),
    // so SetActive(false) is applied immediately here as well
    frameNode->isActive_ = true;
    repeatNode->SetFrameNodeInactive(frameNode, true, std::optional<IndexType>(2));
    EXPECT_FALSE(frameNode->IsActive());
    EXPECT_FALSE(frameNode->needSetInActiveAfterTransitionOut_);

    // keep in tree: inactive immediately as well
    frameNode->isActive_ = true;
    repeatNode->SetFrameNodeInactive(frameNode, false, std::optional<IndexType>(1));
    EXPECT_FALSE(frameNode->IsActive());
}

/**
 * @tc.name: RepeatVirtualScroll2NodeSetFrameNodeInactive002
 * @tc.desc: SetFrameNodeInactive evaluates a loop-wrapped active range
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeSetFrameNodeInactive002, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    auto frameNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 9101, AceType::MakeRefPtr<Pattern>());
    // loop-wrapped visible range [8, 1]: index 0 (index <= end) and index 9 (start <= index)
    // are inside, index 5 is outside; with no implicit animation open all go inactive immediately
    repeatNode->prevVisibleRangeStart_ = 8;
    repeatNode->prevVisibleRangeEnd_ = 1;
    frameNode->isActive_ = true;
    repeatNode->SetFrameNodeInactive(frameNode, true, std::optional<IndexType>(0));
    EXPECT_FALSE(frameNode->IsActive());
    frameNode->isActive_ = true;
    repeatNode->SetFrameNodeInactive(frameNode, true, std::optional<IndexType>(9));
    EXPECT_FALSE(frameNode->IsActive());
    frameNode->isActive_ = true;
    repeatNode->SetFrameNodeInactive(frameNode, true, std::optional<IndexType>(5));
    EXPECT_FALSE(frameNode->IsActive());
    EXPECT_FALSE(frameNode->needSetInActiveAfterTransitionOut_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeProcessActiveL2Nodes001
 * @tc.desc: ProcessActiveL2Nodes skips idle spare items and deactivates active spare items
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeProcessActiveL2Nodes001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    repeatNode->children_.clear();
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 9110, AceType::MakeRefPtr<Pattern>());
    auto cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, false);
    repeatNode->caches_.cacheItem4Rid_[100] = cacheItem;

    // spare item neither active nor on the render tree: skipped entirely
    EXPECT_FALSE(repeatNode->ProcessActiveL2Nodes());

    // spare item active: the cache item's node (a FrameNode) receives SetActive(false) via
    // GetFrameChildByIndex(0) which returns the FrameNode itself, item leaves the render tree
    auto frameNode = AceType::DynamicCast<FrameNode>(uiNode);
    frameNode->isActive_ = true;
    cacheItem->isActive_ = true;
    cacheItem->isOnRenderTree_ = true;
    EXPECT_TRUE(repeatNode->ProcessActiveL2Nodes());
    EXPECT_FALSE(frameNode->IsActive());
    EXPECT_FALSE(cacheItem->isActive_);
    EXPECT_FALSE(cacheItem->isOnRenderTree_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeProcessActiveL2Nodes002
 * @tc.desc: ProcessActiveL2Nodes removes an on-render-tree spare item without touching SetActive
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeProcessActiveL2Nodes002, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    repeatNode->children_.clear();
    RefPtr<UINode> uiNode = AceType::MakeRefPtr<FrameNode>("node", 9120, AceType::MakeRefPtr<Pattern>());
    auto childNode = AceType::MakeRefPtr<FrameNode>("child", 9121, AceType::MakeRefPtr<Pattern>());
    uiNode->AddChild(childNode);
    auto cacheItem = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, false);
    repeatNode->caches_.cacheItem4Rid_[101] = cacheItem;

    // spare item on the render tree but not active: no SetActive(false) on the child,
    // the node still leaves the render tree
    childNode->isActive_ = true;
    cacheItem->isOnRenderTree_ = true;
    EXPECT_TRUE(repeatNode->ProcessActiveL2Nodes());
    EXPECT_TRUE(childNode->IsActive());
    EXPECT_FALSE(cacheItem->isActive_);
    EXPECT_FALSE(cacheItem->isOnRenderTree_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeRebuildL1PrevIndex001
 * @tc.desc: RebuildL1 resolves the pre-rerender index of an out-of-range node via
 *           GetPrevL1Index4Node and applies SetActive(false) without transition-out deferral
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeRebuildL1PrevIndex001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    repeatNode->children_.clear();
    std::map<int32_t, uint32_t> l1Map;
    // cache item nodes are FrameNodes; GetFrameChildByIndex(0) returns the FrameNode itself
    std::vector<RefPtr<FrameNode>> frameNodes;
    for (int32_t i = 0; i < 4; ++i) {
        auto frameNode = AceType::MakeRefPtr<FrameNode>("node", 9130 + i, AceType::MakeRefPtr<Pattern>());
        frameNodes.push_back(frameNode);
        RefPtr<UINode> uiNode = frameNode;
        repeatNode->caches_.cacheItem4Rid_[i + 1] = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
        l1Map[i] = i + 1;
    }
    // update, then record the mapping into prevL1Rid4Index_, so GetPrevL1Index4Node
    // resolves the pre-rerender index of each node
    repeatNode->caches_.UpdateL1Rid4Index(l1Map);
    repeatNode->caches_.RecordPrevL1Rid4Index();
    // nodes falling out of the range below were active before
    frameNodes[2]->isActive_ = true;
    frameNodes[3]->isActive_ = true;

    repeatNode->DoSetActiveChildRange(0, 1, 0, 0);

    // in-range node becomes active
    EXPECT_TRUE(frameNodes[0]->IsActive());
    // out-of-range nodes: SetFrameNodeInactive applied SetActive(false) immediately
    // (no implicit animation open in the test build), no transition-out deferral
    EXPECT_FALSE(frameNodes[2]->IsActive());
    EXPECT_FALSE(frameNodes[3]->IsActive());
    EXPECT_FALSE(frameNodes[2]->needSetInActiveAfterTransitionOut_);
    EXPECT_FALSE(frameNodes[3]->needSetInActiveAfterTransitionOut_);
    // only the two in-range items stay in L1
    EXPECT_EQ(repeatNode->caches_.l1Rid4Index_.size(), static_cast<size_t>(2));
}

/**
 * @tc.name: RepeatVirtualScroll2NodeRebuildL1CancelDeferredDeactivation001
 * @tc.desc: Reactivation through RebuildL1 cancels a pending deferred SetActive(false)
 *           (NeedSetInActiveAfterTransitionOut) armed by an unfinished transition-out
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeRebuildL1CancelDeferredDeactivation001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    repeatNode->children_.clear();
    std::map<int32_t, uint32_t> l1Map;
    std::vector<RefPtr<FrameNode>> frameNodes;
    for (int32_t i = 0; i < 4; ++i) {
        auto frameNode = AceType::MakeRefPtr<FrameNode>("node", 9170 + i, AceType::MakeRefPtr<Pattern>());
        frameNodes.push_back(frameNode);
        RefPtr<UINode> uiNode = frameNode;
        repeatNode->caches_.cacheItem4Rid_[i + 1] = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
        l1Map[i] = i + 1;
    }
    repeatNode->caches_.UpdateL1Rid4Index(l1Map);
    repeatNode->caches_.RecordPrevL1Rid4Index();

    // node 0 scrolls back into the active range while a deferred deactivation is still
    // pending (armed by an earlier SetFrameNodeInactive during an implicit animation;
    // IsImplicitAnimationOpen is a constant false in the test build, so arm the flag
    // directly the way SetInActiveAfterTransitionOut's producer would)
    frameNodes[0]->isActive_ = false;
    frameNodes[0]->NeedSetInActiveAfterTransitionOut(true);
    EXPECT_TRUE(frameNodes[0]->needSetInActiveAfterTransitionOut_);

    repeatNode->DoSetActiveChildRange(0, 1, 0, 0);

    // reactivation applies SetActive(true) and cancels the pending flag
    EXPECT_TRUE(frameNodes[0]->IsActive());
    EXPECT_FALSE(frameNodes[0]->needSetInActiveAfterTransitionOut_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeGetFrameChildCancelDeferredDeactivation001
 * @tc.desc: GetFrameChildByIndex with addToRenderTree cancels a pending deferred
 *           SetActive(false) on the reused node
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeGetFrameChildCancelDeferredDeactivation001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    repeatNode->children_.clear();
    // cache item nodes are FrameNodes; GetFrameChildByIndex(0) returns the FrameNode itself
    auto frameNode = AceType::MakeRefPtr<FrameNode>("node", 9180, AceType::MakeRefPtr<Pattern>());
    frameNode->isActive_ = false;
    RefPtr<UINode> uiNode = frameNode;
    constexpr uint32_t TEST_RID = 5;
    repeatNode->caches_.cacheItem4Rid_[TEST_RID] = RepeatVirtualScroll2CacheItem::MakeCacheItem(uiNode, true);
    repeatNode->caches_.l1Rid4Index_[0] = TEST_RID;

    // a deferred deactivation is pending when the node is fetched back onto the render tree
    frameNode->NeedSetInActiveAfterTransitionOut(true);
    EXPECT_TRUE(frameNode->needSetInActiveAfterTransitionOut_);

    auto child = repeatNode->GetFrameChildByIndex(0, true, false, true);
    ASSERT_NE(child, nullptr);
    EXPECT_TRUE(frameNode->IsActive());
    EXPECT_FALSE(frameNode->needSetInActiveAfterTransitionOut_);

    // isCache fetch keeps the deferred flag untouched: the node is not going back on the
    // render tree
    frameNode->NeedSetInActiveAfterTransitionOut(true);
    auto cacheChild = repeatNode->GetFrameChildByIndex(0, true, true, true);
    ASSERT_NE(cacheChild, nullptr);
    EXPECT_TRUE(frameNode->needSetInActiveAfterTransitionOut_);
}

/**
 * @tc.name: RepeatVirtualScroll2NodeSetAllowReuseInAnimationAttached001
 * @tc.desc: SetAllowReuseInAnimation re-runs the parent sync-cache toggle when the option
 *           arrives while the node is already attached, releases the reference on value
 *           change and ignores same-value repeats
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2NodeSetAllowReuseInAnimationAttached001, TestSize.Level1)
{
    auto gridNode = FrameNode::CreateFrameNode(V2::GRID_ETS_TAG, 9210, AceType::MakeRefPtr<GridPattern>());
    auto gridPattern = gridNode->GetPattern<GridPattern>();
    ASSERT_NE(gridPattern, nullptr);
    auto repeatNode = CreateRepeatVirtualNode(10, 10);
    gridNode->AddChild(repeatNode);

    // attach with the option still unset: container keeps its default, no reference held
    repeatNode->AttachToMainTree(false);
    EXPECT_TRUE(repeatNode->IsOnMainTree());
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());

    // the option arrives after attach (native create() mounts the node synchronously,
    // TS sets the option right afterwards): the re-toggle registers the reference
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_TRUE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_FALSE(repeatNode->syncCacheHost_.Invalid());

    // same value again: early return, the single reference is kept
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_TRUE(gridPattern->IsSyncCacheLayoutEnabled());

    // switching to true while attached releases the reference
    repeatNode->SetAllowReuseInAnimation(true);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());

    // and back to false re-registers it
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_TRUE(gridPattern->IsSyncCacheLayoutEnabled());

    // detach releases the reference and clears the host
    repeatNode->DetachFromMainTree(false, false);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());

    // changing the option while detached does not touch the parent
    repeatNode->SetAllowReuseInAnimation(true);
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
}

/**
 * @tc.name: GetPrevL1Index4RIDTest035
 * @tc.desc: GetPrevL1Index4RID resolves the pre-rerender index recorded by RecordPrevL1Rid4Index
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, GetPrevL1Index4RIDTest035, TestSize.Level1)
{
    auto caches = CreateCachesWithMockCallback();
    // no rerender happened yet: prev mapping empty
    EXPECT_FALSE(caches->GetPrevL1Index4RID(TEST_RID_1).has_value());

    // first rerender: prev still holds nothing
    std::map<int32_t, uint32_t> first;
    first[TEST_INDEX_1] = TEST_RID_1;
    first[TEST_INDEX_2] = TEST_RID_2;
    caches->UpdateL1Rid4Index(first);
    EXPECT_FALSE(caches->GetPrevL1Index4RID(TEST_RID_1).has_value());

    // second rerender: record the current mapping into prev, then replace it —
    // prev now holds the mapping of the first rerender
    caches->RecordPrevL1Rid4Index();
    std::map<int32_t, uint32_t> second;
    second[TEST_INDEX_5] = TEST_RID_2;
    caches->UpdateL1Rid4Index(second);
    auto index4Rid1 = caches->GetPrevL1Index4RID(TEST_RID_1);
    ASSERT_TRUE(index4Rid1.has_value());
    EXPECT_EQ(index4Rid1.value(), TEST_INDEX_1);
    auto index4Rid2 = caches->GetPrevL1Index4RID(TEST_RID_2);
    ASSERT_TRUE(index4Rid2.has_value());
    EXPECT_EQ(index4Rid2.value(), TEST_INDEX_2);
    // rid never seen in a previous mapping
    EXPECT_FALSE(caches->GetPrevL1Index4RID(TEST_RID_2 + 1).has_value());
}

/**
 * @tc.name: GetPrevL1Index4NodeTest036
 * @tc.desc: GetPrevL1Index4Node matches a cached node through the prev mapping
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, GetPrevL1Index4NodeTest036, TestSize.Level1)
{
    auto caches = CreateCachesWithMockCallback();
    RefPtr<UINode> nullNode;
    EXPECT_FALSE(caches->GetPrevL1Index4Node(nullNode).has_value());

    RefPtr<UINode> node = FrameNode::CreateFrameNode("test", TEST_NODE_ID, AceType::MakeRefPtr<Pattern>());
    RefPtr<UINode> otherNode = FrameNode::CreateFrameNode("test", TEST_NODE_ID + 1, AceType::MakeRefPtr<Pattern>());
    caches->cacheItem4Rid_[TEST_RID_1] = RepeatVirtualScroll2CacheItem::MakeCacheItem(node, false);
    caches->cacheItem4Rid_[TEST_RID_2] = RepeatVirtualScroll2CacheItem::MakeCacheItem(otherNode, false);

    std::map<int32_t, uint32_t> first;
    first[TEST_INDEX_1] = TEST_RID_1;
    caches->UpdateL1Rid4Index(first);
    caches->RecordPrevL1Rid4Index();
    std::map<int32_t, uint32_t> second;
    second[TEST_INDEX_2] = TEST_RID_2;
    caches->UpdateL1Rid4Index(second);
    // prev == { TEST_INDEX_1 : TEST_RID_1 }: node resolves, otherNode's rid is only in the current mapping
    auto index4Node = caches->GetPrevL1Index4Node(node);
    ASSERT_TRUE(index4Node.has_value());
    EXPECT_EQ(index4Node.value(), TEST_INDEX_1);
    EXPECT_FALSE(caches->GetPrevL1Index4Node(otherNode).has_value());

    // node not registered in the cache at all
    auto unknownNode = FrameNode::CreateFrameNode("test", TEST_NODE_ID + 2, AceType::MakeRefPtr<Pattern>());
    EXPECT_FALSE(caches->GetPrevL1Index4Node(unknownNode).has_value());
}

/**
 * @tc.name: ClearPrevL1Rid4IndexTest037
 * @tc.desc: ClearPrevL1Rid4Index drops the recorded pre-rerender mapping
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, ClearPrevL1Rid4IndexTest037, TestSize.Level1)
{
    auto caches = CreateCachesWithMockCallback();
    RefPtr<UINode> node = FrameNode::CreateFrameNode("test", TEST_NODE_ID, AceType::MakeRefPtr<Pattern>());
    caches->cacheItem4Rid_[TEST_RID_1] = RepeatVirtualScroll2CacheItem::MakeCacheItem(node, false);
    std::map<int32_t, uint32_t> first;
    first[TEST_INDEX_1] = TEST_RID_1;
    caches->UpdateL1Rid4Index(first);
    caches->RecordPrevL1Rid4Index();
    std::map<int32_t, uint32_t> second;
    second[TEST_INDEX_3] = TEST_RID_1;
    caches->UpdateL1Rid4Index(second);
    ASSERT_TRUE(caches->GetPrevL1Index4RID(TEST_RID_1).has_value());
    ASSERT_TRUE(caches->GetPrevL1Index4Node(node).has_value());

    caches->ClearPrevL1Rid4Index();
    EXPECT_TRUE(caches->prevL1Rid4Index_.empty());
    EXPECT_FALSE(caches->GetPrevL1Index4RID(TEST_RID_1).has_value());
    EXPECT_FALSE(caches->GetPrevL1Index4Node(node).has_value());
    // the current mapping is untouched
    EXPECT_EQ(caches->l1Rid4Index_.size(), static_cast<size_t>(1));
}

/**
 * @tc.name: UpdateL1Rid4IndexModelNG038
 * @tc.desc: ModelNG::UpdateL1Rid4Index fires onUpdateDirty and records the pre-rerender
 *           mapping before replacing it, so a same-frame DoSetActiveChildRange (e.g. the
 *           one UpdateTotalCount(0) runs) can still resolve the old L1 indices
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, UpdateL1Rid4IndexModelNG038, TestSize.Level1)
{
    auto repeatNode = CreateRepeatVirtualNode(6, 6);
    int32_t dirtyCount = 0;
    repeatNode->onUpdateDirty_ = [&dirtyCount]() { ++dirtyCount; };
    // ensure a clean starting state: prevL1Rid4Index_ may be non-empty from
    // earlier tests sharing the same node via ElementRegister
    repeatNode->caches_.ClearPrevL1Rid4Index();
    // L1 mapping as left by the previous rerender
    std::map<int32_t, uint32_t> oldMap = { { 0, 1 }, { 1, 2 } };
    repeatNode->caches_.UpdateL1Rid4Index(oldMap);
    EXPECT_TRUE(repeatNode->caches_.prevL1Rid4Index_.empty());

    std::map<int32_t, uint32_t> newMap = { { 2, 3 }, { 3, 4 } };
    RepeatVirtualScroll2ModelNG repeatModel;
    repeatModel.UpdateL1Rid4Index(repeatNode->GetId(), 4, 10, 0, newMap);

    // onUpdateDirty fired exactly once, before the mapping was replaced
    EXPECT_EQ(dirtyCount, 1);
    // the pre-rerender mapping holds the old L1 indices and survives the replacement
    auto prevIndex = repeatNode->caches_.GetPrevL1Index4RID(1);
    ASSERT_TRUE(prevIndex.has_value());
    EXPECT_EQ(prevIndex.value(), 0);
    // the new mapping is installed, counts are updated
    EXPECT_EQ(repeatNode->caches_.l1Rid4Index_, newMap);
    EXPECT_EQ(repeatNode->totalCount_, 10u);
    EXPECT_EQ(repeatNode->arrLen_, 4u);
}

/**
 * @tc.name: RepeatNodeSetAllowReuseInAnimationTest045
 * @tc.desc: SetAllowReuseInAnimation stores the tri-state value
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatNodeSetAllowReuseInAnimationTest045, TestSize.Level1)
{
    auto repeatNode = RepeatNode::GetOrCreateRepeatNode(TEST_REPEAT_NODE_ID);
    EXPECT_FALSE(repeatNode->isAllowReuseInAnimation_.has_value());
    repeatNode->SetAllowReuseInAnimation(false);
    ASSERT_TRUE(repeatNode->isAllowReuseInAnimation_.has_value());
    EXPECT_FALSE(repeatNode->isAllowReuseInAnimation_.value());
    repeatNode->SetAllowReuseInAnimation(true);
    ASSERT_TRUE(repeatNode->isAllowReuseInAnimation_.has_value());
    EXPECT_TRUE(repeatNode->isAllowReuseInAnimation_.value());
    repeatNode->SetAllowReuseInAnimation(std::nullopt);
    EXPECT_FALSE(repeatNode->isAllowReuseInAnimation_.has_value());
}

/**
 * @tc.name: RepeatNodeIsAllowAnimationTest046
 * @tc.desc: Explicitly set allowReuseInAnimation has priority over the non-List parent default
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatNodeIsAllowAnimationTest046, TestSize.Level1)
{
    auto repeatNode = RepeatNode::GetOrCreateRepeatNode(TEST_REPEAT_NODE_ID_2);
    repeatNode->SetAllowReuseInAnimation(std::nullopt);
    auto parentNode = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, TEST_COLUMN_PARENT_ID, AceType::MakeRefPtr<Pattern>());
    repeatNode->SetParent(parentNode);
    // default: parent is not List -> false
    EXPECT_FALSE(repeatNode->IsAllowAnimation());
    // explicitly disallow reuse in animation -> animation allowed
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_TRUE(repeatNode->IsAllowAnimation());
    // explicitly allow reuse in animation -> animation disallowed
    repeatNode->SetAllowReuseInAnimation(true);
    EXPECT_FALSE(repeatNode->IsAllowAnimation());
}

/**
 * @tc.name: RepeatNodeIsAllowAnimationTest047
 * @tc.desc: Explicitly set allowReuseInAnimation has priority over the List parent default
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatNodeIsAllowAnimationTest047, TestSize.Level1)
{
    auto repeatNode = RepeatNode::GetOrCreateRepeatNode(TEST_REPEAT_NODE_ID_2);
    repeatNode->SetAllowReuseInAnimation(std::nullopt);
    auto listNode = FrameNode::CreateFrameNode(
        V2::LIST_ETS_TAG, TEST_LIST_PARENT_ID, AceType::MakeRefPtr<Pattern>());
    repeatNode->SetParent(listNode);
    // default: parent is List -> true
    EXPECT_TRUE(repeatNode->IsAllowAnimation());
    // explicitly disallow reuse in animation: same result as the List default
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_TRUE(repeatNode->IsAllowAnimation());
    // explicitly allow reuse in animation overrides the List default
    repeatNode->SetAllowReuseInAnimation(true);
    EXPECT_FALSE(repeatNode->IsAllowAnimation());
}

/**
 * @tc.name: RepeatNodeIsAllowAnimationTest048
 * @tc.desc: Explicitly set allowReuseInAnimation has priority over the allowReuse id set
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatNodeIsAllowAnimationTest048, TestSize.Level1)
{
    LazyForEachUtils::SetIdsForRepeatAnimationAllowReuse(TEST_INSPECTOR_ID);
    auto listNode = FrameNode::CreateFrameNode(
        V2::LIST_ETS_TAG, TEST_LIST_PARENT_ID, AceType::MakeRefPtr<Pattern>());
    listNode->UpdateInspectorId(TEST_INSPECTOR_ID);
    auto repeatNode = RepeatNode::GetOrCreateRepeatNode(TEST_REPEAT_NODE_ID_2);
    repeatNode->SetAllowReuseInAnimation(std::nullopt);
    repeatNode->SetParent(listNode);
    // default: List parent whose id is in the allowReuse set -> false
    EXPECT_FALSE(repeatNode->IsAllowAnimation());
    // explicitly disallow reuse in animation has the highest priority
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_TRUE(repeatNode->IsAllowAnimation());
    LazyForEachUtils::SetIdsForRepeatAnimationAllowReuse("");
}

/**
 * @tc.name: RepeatNodeOnAttachSyncCacheGridTest049
 * @tc.desc: Attach/detach holds and releases the sync-cache reference on a Grid parent
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatNodeOnAttachSyncCacheGridTest049, TestSize.Level1)
{
    auto gridNode = FrameNode::CreateFrameNode(
        V2::GRID_ETS_TAG, TEST_GRID_PARENT_ID, AceType::MakeRefPtr<GridPattern>());
    auto gridPattern = gridNode->GetPattern<GridPattern>();
    ASSERT_NE(gridPattern, nullptr);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    auto repeatNode = RepeatNode::GetOrCreateRepeatNode(TEST_REPEAT_NODE_ID);
    repeatNode->SetAllowReuseInAnimation(false);
    gridNode->AddChild(repeatNode);
    repeatNode->OnAttachToMainTree(false);
    EXPECT_TRUE(gridPattern->IsSyncCacheLayoutEnabled());
    repeatNode->OnDetachFromMainTree(false, nullptr);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());
}

/**
 * @tc.name: RepeatNodeOnAttachSyncCacheWaterFlowTest050
 * @tc.desc: Attach/detach holds and releases the sync-cache reference on a WaterFlow parent
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatNodeOnAttachSyncCacheWaterFlowTest050, TestSize.Level1)
{
    auto waterFlowNode = FrameNode::CreateFrameNode(
        V2::WATERFLOW_ETS_TAG, TEST_WATERFLOW_PARENT_ID, AceType::MakeRefPtr<WaterFlowPattern>());
    auto waterFlowPattern = waterFlowNode->GetPattern<WaterFlowPattern>();
    ASSERT_NE(waterFlowPattern, nullptr);
    EXPECT_FALSE(waterFlowPattern->IsSyncCacheLayoutEnabled());
    auto repeatNode = RepeatNode::GetOrCreateRepeatNode(TEST_REPEAT_NODE_ID);
    repeatNode->SetAllowReuseInAnimation(false);
    waterFlowNode->AddChild(repeatNode);
    repeatNode->OnAttachToMainTree(false);
    EXPECT_TRUE(waterFlowPattern->IsSyncCacheLayoutEnabled());
    repeatNode->OnDetachFromMainTree(false, nullptr);
    EXPECT_FALSE(waterFlowPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());
}

/**
 * @tc.name: RepeatNodeOnAttachSyncCacheNotQualifiedTest051
 * @tc.desc: Attach does not hold the sync-cache reference when the option is unset or true
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatNodeOnAttachSyncCacheNotQualifiedTest051, TestSize.Level1)
{
    auto gridNode = FrameNode::CreateFrameNode(
        V2::GRID_ETS_TAG, TEST_GRID_PARENT_ID, AceType::MakeRefPtr<GridPattern>());
    auto gridPattern = gridNode->GetPattern<GridPattern>();
    ASSERT_NE(gridPattern, nullptr);
    auto repeatNode = RepeatNode::GetOrCreateRepeatNode(TEST_REPEAT_NODE_ID);
    gridNode->AddChild(repeatNode);

    // unset option keeps the container default
    repeatNode->SetAllowReuseInAnimation(std::nullopt);
    repeatNode->OnAttachToMainTree(false);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());
    repeatNode->OnDetachFromMainTree(false, nullptr);

    // explicitly true keeps the container default
    repeatNode->SetAllowReuseInAnimation(true);
    repeatNode->OnAttachToMainTree(false);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());
    repeatNode->OnDetachFromMainTree(false, nullptr);
}

/**
 * @tc.name: RepeatNodeSetAllowReuseInAnimationAttached001
 * @tc.desc: SetAllowReuseInAnimation re-runs the parent sync-cache toggle when the option
 *           arrives while the node is already attached, releases the reference on value
 *           change and ignores same-value repeats
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatNodeSetAllowReuseInAnimationAttached001, TestSize.Level1)
{
    auto gridNode = FrameNode::CreateFrameNode(
        V2::GRID_ETS_TAG, TEST_GRID_PARENT_ID, AceType::MakeRefPtr<GridPattern>());
    auto gridPattern = gridNode->GetPattern<GridPattern>();
    ASSERT_NE(gridPattern, nullptr);
    auto repeatNode = RepeatNode::GetOrCreateRepeatNode(TEST_REPEAT_NODE_ID);
    gridNode->AddChild(repeatNode);

    // attach with the option still unset: container keeps its default, no reference held
    repeatNode->AttachToMainTree(false);
    EXPECT_TRUE(repeatNode->IsOnMainTree());
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());

    // the option arrives after attach: the re-toggle registers the reference
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_TRUE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_FALSE(repeatNode->syncCacheHost_.Invalid());

    // same value again: early return, the single reference is kept
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_TRUE(gridPattern->IsSyncCacheLayoutEnabled());

    // switching to true while attached releases the reference, back to false re-registers
    repeatNode->SetAllowReuseInAnimation(true);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_TRUE(gridPattern->IsSyncCacheLayoutEnabled());

    // detach releases the reference; changing the option afterwards does not touch the parent
    repeatNode->DetachFromMainTree(false, false);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());
    repeatNode->SetAllowReuseInAnimation(true);
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
}

/**
 * @tc.name: RepeatNodeOnAttachSyncCacheNonContainerParentTest052
 * @tc.desc: Attach does not hold the sync-cache reference when the parent is not Grid/WaterFlow
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatNodeOnAttachSyncCacheNonContainerParentTest052, TestSize.Level1)
{
    auto parentNode = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, TEST_COLUMN_PARENT_ID, AceType::MakeRefPtr<Pattern>());
    auto repeatNode = RepeatNode::GetOrCreateRepeatNode(TEST_REPEAT_NODE_ID);
    repeatNode->SetAllowReuseInAnimation(false);
    parentNode->AddChild(repeatNode);
    repeatNode->OnAttachToMainTree(false);
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());
    repeatNode->OnDetachFromMainTree(false, nullptr);
    SUCCEED();
}

/**
 * @tc.name: RepeatNodeOnAttachSyncCacheIdempotentTest053
 * @tc.desc: Repeated attach holds only one sync-cache reference
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatNodeOnAttachSyncCacheIdempotentTest053, TestSize.Level1)
{
    auto gridNode = FrameNode::CreateFrameNode(
        V2::GRID_ETS_TAG, TEST_GRID_PARENT_ID, AceType::MakeRefPtr<GridPattern>());
    auto gridPattern = gridNode->GetPattern<GridPattern>();
    ASSERT_NE(gridPattern, nullptr);
    auto repeatNode = RepeatNode::GetOrCreateRepeatNode(TEST_REPEAT_NODE_ID);
    repeatNode->SetAllowReuseInAnimation(false);
    gridNode->AddChild(repeatNode);
    repeatNode->OnAttachToMainTree(false);
    repeatNode->OnAttachToMainTree(false);
    EXPECT_TRUE(gridPattern->IsSyncCacheLayoutEnabled());
    // a single detach fully releases the reference -> only one reference was held
    repeatNode->OnDetachFromMainTree(false, nullptr);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
}

/**
 * @tc.name: LazyForEachUtilsToggleParentSyncCacheLayoutDisableTest054
 * @tc.desc: Disable branch: empty host no-op, Grid/WaterFlow host releases, other host resets
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, LazyForEachUtilsToggleParentSyncCacheLayoutDisableTest054, TestSize.Level1)
{
    auto repeatNode = RepeatNode::GetOrCreateRepeatNode(TEST_REPEAT_NODE_ID);

    // empty host: nothing to release
    WeakPtr<FrameNode> emptyHost;
    LazyForEachUtils::ToggleParentSyncCacheLayout(repeatNode, false, std::optional<bool>(false), true, emptyHost);
    EXPECT_TRUE(emptyHost.Invalid());

    // Grid host: one disable releases one reference (one attach = one host record)
    auto gridNode = FrameNode::CreateFrameNode(
        V2::GRID_ETS_TAG, TEST_GRID_PARENT_ID, AceType::MakeRefPtr<GridPattern>());
    auto gridPattern = gridNode->GetPattern<GridPattern>();
    ASSERT_NE(gridPattern, nullptr);
    gridPattern->AddSyncCacheLayoutChild();
    WeakPtr<FrameNode> gridHost = gridNode;
    LazyForEachUtils::ToggleParentSyncCacheLayout(repeatNode, false, std::optional<bool>(false), true, gridHost);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(gridHost.Invalid());
    // second disable on already-reset host is a no-op (clamped at zero)
    LazyForEachUtils::ToggleParentSyncCacheLayout(repeatNode, false, std::optional<bool>(false), true, gridHost);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());

    // WaterFlow host: releases the reference
    auto waterFlowNode = FrameNode::CreateFrameNode(
        V2::WATERFLOW_ETS_TAG, TEST_WATERFLOW_PARENT_ID, AceType::MakeRefPtr<WaterFlowPattern>());
    auto waterFlowPattern = waterFlowNode->GetPattern<WaterFlowPattern>();
    ASSERT_NE(waterFlowPattern, nullptr);
    waterFlowPattern->AddSyncCacheLayoutChild();
    WeakPtr<FrameNode> waterFlowHost = waterFlowNode;
    LazyForEachUtils::ToggleParentSyncCacheLayout(repeatNode, false, std::optional<bool>(false), true, waterFlowHost);
    EXPECT_FALSE(waterFlowPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(waterFlowHost.Invalid());

    // host without Grid/WaterFlow pattern: host is reset, no crash
    auto columnNode = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, TEST_COLUMN_PARENT_ID, AceType::MakeRefPtr<Pattern>());
    WeakPtr<FrameNode> columnHost = columnNode;
    LazyForEachUtils::ToggleParentSyncCacheLayout(repeatNode, false, std::optional<bool>(false), true, columnHost);
    EXPECT_TRUE(columnHost.Invalid());
}

/**
 * @tc.name: LazyForEachUtilsToggleParentSyncCacheLayoutEnableTest055
 * @tc.desc: Enable branch: qualified node holds exactly one reference and records the host
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, LazyForEachUtilsToggleParentSyncCacheLayoutEnableTest055, TestSize.Level1)
{
    auto repeatNode = RepeatNode::GetOrCreateRepeatNode(TEST_REPEAT_NODE_ID);
    auto gridNode = FrameNode::CreateFrameNode(
        V2::GRID_ETS_TAG, TEST_GRID_PARENT_ID, AceType::MakeRefPtr<GridPattern>());
    auto gridPattern = gridNode->GetPattern<GridPattern>();
    ASSERT_NE(gridPattern, nullptr);
    gridNode->AddChild(repeatNode);

    WeakPtr<FrameNode> host;
    LazyForEachUtils::ToggleParentSyncCacheLayout(repeatNode, true, std::optional<bool>(false), true, host);
    EXPECT_TRUE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_FALSE(host.Invalid());

    // already holding a reference: repeated enable does not add a second one
    LazyForEachUtils::ToggleParentSyncCacheLayout(repeatNode, true, std::optional<bool>(false), true, host);
    EXPECT_TRUE(gridPattern->IsSyncCacheLayoutEnabled());
    LazyForEachUtils::ToggleParentSyncCacheLayout(repeatNode, false, std::optional<bool>(false), true, host);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(host.Invalid());
}

/**
 * @tc.name: LazyForEachUtilsToggleParentSyncCacheLayoutDisqualifyTest056
 * @tc.desc: Enable branch: null node / not repeat / unset / true / non-FrameNode parent never adds
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, LazyForEachUtilsToggleParentSyncCacheLayoutDisqualifyTest056, TestSize.Level1)
{
    auto gridNode = FrameNode::CreateFrameNode(
        V2::GRID_ETS_TAG, TEST_GRID_PARENT_ID, AceType::MakeRefPtr<GridPattern>());
    auto gridPattern = gridNode->GetPattern<GridPattern>();
    ASSERT_NE(gridPattern, nullptr);
    auto repeatNode = RepeatNode::GetOrCreateRepeatNode(TEST_REPEAT_NODE_ID);
    gridNode->AddChild(repeatNode);

    WeakPtr<FrameNode> host;
    // null node
    LazyForEachUtils::ToggleParentSyncCacheLayout(nullptr, true, std::optional<bool>(false), true, host);
    // not in Repeat mode
    LazyForEachUtils::ToggleParentSyncCacheLayout(repeatNode, true, std::optional<bool>(false), false, host);
    // option unset
    LazyForEachUtils::ToggleParentSyncCacheLayout(repeatNode, true, std::optional<bool> {}, true, host);
    // option explicitly true
    LazyForEachUtils::ToggleParentSyncCacheLayout(repeatNode, true, std::optional<bool>(true), true, host);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(host.Invalid());

    // direct parent is not a FrameNode
    auto syntaxParent = RepeatNode::GetOrCreateRepeatNode(TEST_REPEAT_NODE_ID_2);
    syntaxParent->AddChild(repeatNode);
    LazyForEachUtils::ToggleParentSyncCacheLayout(repeatNode, true, std::optional<bool>(false), true, host);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(host.Invalid());
}

/**
 * @tc.name: RepeatSyntaxSetAllowReuseInAnimationTest022
 * @tc.desc: SetAllowReuseInAnimation forwards the tri-state value to the RepeatNode on the stack
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatSyntaxSetAllowReuseInAnimationTest022, TestSize.Level1)
{
    RepeatModelNG repeatModel;
    repeatModel.StartRender();
    auto repeatNode = AceType::DynamicCast<RepeatNode>(ViewStackProcessor::GetInstance()->GetMainElementNode());
    ASSERT_NE(repeatNode, nullptr);
    EXPECT_FALSE(repeatNode->isAllowReuseInAnimation_.has_value());
    repeatModel.SetAllowReuseInAnimation(false);
    ASSERT_TRUE(repeatNode->isAllowReuseInAnimation_.has_value());
    EXPECT_FALSE(repeatNode->isAllowReuseInAnimation_.value());
    repeatModel.SetAllowReuseInAnimation(std::nullopt);
    EXPECT_FALSE(repeatNode->isAllowReuseInAnimation_.has_value());
    std::list<int32_t> removedElmtIds;
    repeatModel.FinishRender(removedElmtIds);
}

/**
 * @tc.name: RepeatSyntaxSetAllowReuseInAnimationTest023
 * @tc.desc: SetAllowReuseInAnimation is a no-op when the stack main element is not a RepeatNode
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatSyntaxSetAllowReuseInAnimationTest023, TestSize.Level1)
{
    // create a RepeatNode that is NOT on the ViewStackProcessor
    auto repeatNode = RepeatNode::GetOrCreateRepeatNode(TEST_REPEAT_NODE_ID);
    ASSERT_NE(repeatNode, nullptr);
    repeatNode->SetAllowReuseInAnimation(std::nullopt);
    EXPECT_FALSE(repeatNode->isAllowReuseInAnimation_.has_value());
    // stack main element is not a RepeatNode: the call is a silent no-op
    RepeatModelNG repeatModel;
    repeatModel.SetAllowReuseInAnimation(false);
    // the RepeatNode's state must remain unchanged
    EXPECT_FALSE(repeatNode->isAllowReuseInAnimation_.has_value());
}

/**
 * @tc.name: RepeatVirtualScroll2SyntaxSetAllowReuseInAnimationTest023
 * @tc.desc: SetAllowReuseInAnimation falls back to the ElementRegister lookup by elmt id
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2SyntaxSetAllowReuseInAnimationTest023, TestSize.Level1)
{
    constexpr int32_t testMemOptStrategy = 0;
    auto onGetRid4Index = [](int32_t index, bool inAnimation, bool forceCreateNewChild) -> std::pair<uint32_t, uint32_t>
    {
        return { static_cast<uint32_t>(index), 0 };
    };
    auto onRecycleItems = [](int32_t start, int32_t end) {};
    auto onActiveRange = [](int32_t start, int32_t end, int32_t cacheStart,
        int32_t cacheEnd, bool isCache, bool isImmediate) {};
    auto onMoveFromTo = [](int32_t from, int32_t to) {};
    auto onPurge = []() {};
    auto onPurgeAll = []() {};
    auto onUpdateDirty = []() {};
    auto repeatNode = RepeatVirtualScroll2Node::GetOrCreateRepeatNode(TEST_REPEAT_ELMT_ID_2, TEST_ARR_LEN,
        TEST_TOTAL_COUNT, testMemOptStrategy, onGetRid4Index, onRecycleItems, onActiveRange, onMoveFromTo, onPurge,
        onPurgeAll, onUpdateDirty);
    ASSERT_NE(repeatNode, nullptr);
    EXPECT_FALSE(repeatNode->isAllowReuseInAnimation_.has_value());
    // stack main element is not a RepeatVirtualScroll2Node: fall back to the elmt id
    auto columnNode =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, TEST_REPEAT_ELMT_ID, AceType::MakeRefPtr<Pattern>());
    ViewStackProcessor::GetInstance()->Push(columnNode);
    RepeatVirtualScroll2ModelNG model;
    model.SetAllowReuseInAnimation(TEST_REPEAT_ELMT_ID_2, false);
    ASSERT_TRUE(repeatNode->isAllowReuseInAnimation_.has_value());
    EXPECT_FALSE(repeatNode->isAllowReuseInAnimation_.value());
    model.SetAllowReuseInAnimation(TEST_REPEAT_ELMT_ID_2, std::nullopt);
    EXPECT_FALSE(repeatNode->isAllowReuseInAnimation_.has_value());
    ViewStackProcessor::GetInstance()->Pop();
}

/**
 * @tc.name: RepeatVirtualScroll2SyntaxSetAllowReuseInAnimationTest024
 * @tc.desc: SetAllowReuseInAnimation prefers the stack main element over the elmt id
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2SyntaxSetAllowReuseInAnimationTest024, TestSize.Level1)
{
    constexpr int32_t testMemOptStrategy = 0;
    auto onGetRid4Index = [](int32_t index, bool inAnimation, bool forceCreateNewChild) -> std::pair<uint32_t, uint32_t>
    {
        return { static_cast<uint32_t>(index), 0 };
    };
    auto onRecycleItems = [](int32_t start, int32_t end) {};
    auto onActiveRange = [](int32_t start, int32_t end, int32_t cacheStart,
        int32_t cacheEnd, bool isCache, bool isImmediate) {};
    auto onMoveFromTo = [](int32_t from, int32_t to) {};
    auto onPurge = []() {};
    auto onPurgeAll = []() {};
    auto onUpdateDirty = []() {};
    auto repeatNode = RepeatVirtualScroll2Node::GetOrCreateRepeatNode(TEST_REPEAT_ELMT_ID_2, TEST_ARR_LEN,
        TEST_TOTAL_COUNT, testMemOptStrategy, onGetRid4Index, onRecycleItems, onActiveRange, onMoveFromTo, onPurge,
        onPurgeAll, onUpdateDirty);
    ASSERT_NE(repeatNode, nullptr);
    repeatNode->SetAllowReuseInAnimation(std::nullopt);
    // stack main element is the node itself: the elmt id is ignored
    ViewStackProcessor::GetInstance()->Push(repeatNode);
    RepeatVirtualScroll2ModelNG model;
    model.SetAllowReuseInAnimation(INVALID_REPEAT_ELMT_ID, true);
    ASSERT_TRUE(repeatNode->isAllowReuseInAnimation_.has_value());
    EXPECT_TRUE(repeatNode->isAllowReuseInAnimation_.value());
    ViewStackProcessor::GetInstance()->Pop();
    repeatNode->SetAllowReuseInAnimation(std::nullopt);
}

/**
 * @tc.name: RepeatVirtualScroll2SyntaxSetAllowReuseInAnimationTest025
 * @tc.desc: SetAllowReuseInAnimation is a no-op when neither the stack nor the elmt id
 *           resolves to a RepeatVirtualScroll2Node
 * @tc.type: FUNC
 */
HWTEST_F(RepeatVirtual2TestNg, RepeatVirtualScroll2SyntaxSetAllowReuseInAnimationTest025, TestSize.Level1)
{
    // stack main element is a plain Column, not a RepeatVirtualScroll2Node
    auto columnNode =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, TEST_REPEAT_ELMT_ID, AceType::MakeRefPtr<Pattern>());
    ViewStackProcessor::GetInstance()->Push(columnNode);
    // INVALID_REPEAT_ELMT_ID does not resolve to any registered node
    RepeatVirtualScroll2ModelNG model;
    model.SetAllowReuseInAnimation(INVALID_REPEAT_ELMT_ID, false);
    // verify the stack main element is still the Column (unchanged)
    auto stackMain = ViewStackProcessor::GetInstance()->GetMainElementNode();
    ASSERT_NE(stackMain, nullptr);
    EXPECT_EQ(stackMain->GetId(), columnNode->GetId());
    // verify no RepeatVirtualScroll2Node was affected: none registered under INVALID_REPEAT_ELMT_ID
    auto notFound =
        ElementRegister::GetInstance()->GetSpecificItemById<RepeatVirtualScroll2Node>(INVALID_REPEAT_ELMT_ID);
    EXPECT_EQ(notFound, nullptr);
    ViewStackProcessor::GetInstance()->Pop();
}

} // namespace OHOS::Ace::NG