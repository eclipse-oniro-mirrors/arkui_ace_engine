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

#include "test/unittest/core/pattern/test_ng.h"

#include "core/components_ng/pattern/linear_layout/column_model_ng.h"
#include "frameworks/core/components_ng/syntax/arkoala_lazy_node.h"
#include "base/log/dump_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
void SetCallbacksForTest(const RefPtr<ArkoalaLazyNode>& node, ArkoalaLazyNode::CreateItemCb create,
    ArkoalaLazyNode::UpdateRangeCb update)
{
    node->SetCallbacks(create, update, []() {}, [](int32_t) {});
}
} // namespace

class TestUINode : public UINode {
    DECLARE_ACE_TYPE(TestUINode, UINode);

    bool IsAtomicNode() const override
    {
        return true;
    }

    explicit TestUINode(int32_t nodeId) : UINode("TestUINode", nodeId) {}

    ~TestUINode() override = default;
};

class ArkoalaLazyNodeTest : public TestNG {
public:
    RefPtr<ArkoalaLazyNode> CreateLazyForEachNode(int32_t nodeId)
    {
        auto* stack = ViewStackProcessor::GetInstance();
        auto lazyForEachNode = AceType::MakeRefPtr<ArkoalaLazyNode>(nodeId);
        stack->Push(lazyForEachNode);
        SetCallbacksForTest(lazyForEachNode, createItemCb_, updateRangeCb_);
        return lazyForEachNode;
    }

    RefPtr<ArkoalaLazyNode> CreateRepeatNode(int32_t nodeId)
    {
        auto* stack = ViewStackProcessor::GetInstance();
        auto repeatNode = AceType::MakeRefPtr<ArkoalaLazyNode>(nodeId, true);
        stack->Push(repeatNode);
        SetCallbacksForTest(repeatNode, createItemCb_, updateRangeCb_);
        return repeatNode;
    }

    void CreateChildren(RefPtr<ArkoalaLazyNode>& lazyNode, int32_t totalCount)
    {
        for (int i = 0; i < totalCount; ++i) {
            lazyNode->GetFrameChildByIndex(i, true, false, true);
        }
        lazyNode->SetTotalCount(totalCount);
    }

    RefPtr<TestUINode> CreateTestUINode(int32_t nodeId)
    {
        return AceType::MakeRefPtr<TestUINode>(nodeId);
    }

    RefPtr<FrameNode> CreateFrameNode(int32_t nodeId)
    {
        return ColumnModelNG::CreateFrameNode(nodeId);
    }

    std::string DumpUINode(const RefPtr<UINode>& node) const
    {
        return (node == nullptr) ? "null" : node->GetTag() + "(" + std::to_string(node->GetId()) + ")";
    }

    int32_t GetNextId()
    {
        return ++nodeId_;
    }

    int32_t nodeId_ = 0;

    ArkoalaLazyNode::CreateItemCb createItemCb_ = [this](int32_t idx, bool skipCreate) {
        RefPtr<TestUINode> uiNode = CreateTestUINode(GetNextId());
        auto columnNode = ColumnModelNG::CreateFrameNode(GetNextId());
        uiNode->AddChild(columnNode);
        return uiNode;
    };
    ArkoalaLazyNode::UpdateRangeCb updateRangeCb_ = [](
        int32_t start, int32_t end, int32_t cacheStart, int32_t cacheEnd, bool isLoop) {};
};

const int32_t INDEX_1 = 1;
const int32_t INDEX_2 = 2;
const int32_t TOTAL_COUNT = 10;
const int32_t CACHED_COUNT = 2;


/**
 * @tc.name: ArkoalaLazyNodeTest002
 * @tc.desc: Test ArkoalaLazyNode GetChildByIndex
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest002)
{
    /**
     * @tc.steps: step1. Create LazyForEach node and set callbacks
     */
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    ArkoalaLazyNode::CreateItemCb createItemCb = [this](int32_t idx, bool skipCreate) {
        RefPtr<TestUINode> uiNode = CreateTestUINode(GetNextId());
        auto columnNode = ColumnModelNG::CreateFrameNode(GetNextId());
        uiNode->AddChild(columnNode);
        EXPECT_EQ(uiNode->GetChildren().size(), 1);
        return uiNode;
    };
    ArkoalaLazyNode::UpdateRangeCb updateRangeCb = [](
        int32_t start, int32_t end, int32_t cacheStart, int32_t cacheEnd, bool isLoop) {};
    SetCallbacksForTest(lazyNode, createItemCb, updateRangeCb);

    /**
     * @tc.steps: step2. Test LazyForEach node GetChildByIndex and GetFrameChildByIndex
     */
    lazyNode->SetTotalCount(TOTAL_COUNT);
    RefPtr<UINode> childNode = lazyNode->GetFrameChildByIndex(INDEX_1, true, false, true);
    EXPECT_TRUE(childNode);
    EXPECT_TRUE(AceType::DynamicCast<FrameNode>(childNode)->IsActive());
    EXPECT_FALSE(lazyNode->GetChildByIndex(INDEX_2));

    /**
     * @tc.steps: step3. Test LazyForEach node DoSetActiveChildRange to cache childNode
     */
    childNode = lazyNode->GetFrameChildByIndex(INDEX_2, true, true, false);
    EXPECT_TRUE(childNode);
    lazyNode->DoSetActiveChildRange(INDEX_1, INDEX_1, CACHED_COUNT, CACHED_COUNT, false);
    EXPECT_FALSE(AceType::DynamicCast<FrameNode>(childNode)->IsActive());
}

/**
 * @tc.name: ArkoalaLazyNodeTest003
 * @tc.desc: Test ArkoalaLazyNode GetFrameNode
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest003)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    ArkoalaLazyNode::CreateItemCb createItemCb = [this](int32_t idx, bool skipCreate) {
        return ColumnModelNG::CreateFrameNode(GetNextId());
    };
    ArkoalaLazyNode::UpdateRangeCb updateRangeCb = [](
        int32_t start, int32_t end, int32_t cacheStart, int32_t cacheEnd, bool isLoop) {};
    SetCallbacksForTest(lazyNode, createItemCb, updateRangeCb);
    lazyNode->SetTotalCount(TOTAL_COUNT);

    EXPECT_TRUE(lazyNode->GetFrameChildByIndex(INDEX_1, true, false, true));
    EXPECT_TRUE(lazyNode->GetFrameNode(INDEX_1));
}

/**
 * @tc.name: ArkoalaLazyNodeTest004
 * @tc.desc: Test ArkoalaLazyNode DumpInfo
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest004)
{
    /**
     * @tc.steps: step1. Test LazyForEach node dump info
     */
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    EXPECT_NE(lazyNode, nullptr);
    lazyNode->DumpInfo();
    EXPECT_NE(DumpLog::GetInstance().description_.back(), "VirtualScroll: true\n");

    /**
     * @tc.steps: step2. Test Repeat node dump info
     */
    auto repeatNode = CreateRepeatNode(GetNextId());
    EXPECT_NE(repeatNode, nullptr);
    repeatNode->DumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_.back(), "VirtualScroll: true\n");
}

/**
 * @tc.name: ArkoalaLazyNodeTest005
 * @tc.desc: Test ArkoalaLazyNode OnDataChange
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest005)
{
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    CreateChildren(lazyNode, TOTAL_COUNT);
    lazyNode->OnDataChange(0, TOTAL_COUNT, UINode::NotificationType::START_CHANGE_POSITION);
    EXPECT_EQ(lazyNode->node4Index_.Size(), 0);
}

/**
 * @tc.name: ArkoalaLazyNodeTest006
 * @tc.desc: Test ArkoalaLazyNode OnDataChange
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest006)
{
    auto repeatNode = CreateRepeatNode(GetNextId());
    EXPECT_NE(repeatNode, nullptr);
    repeatNode->OnDataChange(0, TOTAL_COUNT, UINode::NotificationType::START_CHANGE_POSITION);
}

/**
 * @tc.name: ArkoalaLazyNodeTest007
 * @tc.desc: Test ArkoalaLazyNode SetJSViewActive.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest007)
{
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    EXPECT_EQ(lazyNode->node4Index_.Size(), 0);
    auto repeatNode = CreateRepeatNode(GetNextId());
    lazyNode->node4Index_.Put(0, repeatNode);
    EXPECT_TRUE(lazyNode->isActive_);
    auto child = AceType::DynamicCast<ArkoalaLazyNode>(lazyNode->node4Index_.Get(0).value());
    EXPECT_TRUE(child->isActive_);
    lazyNode->SetJSViewActive(false, true);
    EXPECT_FALSE(lazyNode->isActive_);
}

/**
 * @tc.name: ArkoalaLazyNodeTest008
 * @tc.desc: Test ArkoalaLazyNode UpdateIsCache.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest008)
{
    auto repeatNode = CreateRepeatNode(GetNextId());
    EXPECT_EQ(repeatNode->recycleNodeIds_.size(), 0);
    int32_t nodeId = GetNextId();
    auto frameNode = CreateFrameNode(nodeId);
    repeatNode->UpdateIsCache(frameNode, false, false);
    EXPECT_EQ(repeatNode->recycleNodeIds_.size(), 0);
    repeatNode->UpdateIsCache(frameNode, false, true);
    EXPECT_EQ(repeatNode->recycleNodeIds_.size(), 1);
    repeatNode->UpdateIsCache(frameNode, true, true);
    EXPECT_EQ(repeatNode->recycleNodeIds_.size(), 0);
}

/**
 * @tc.name: ArkoalaLazyNodeTest010
 * @tc.desc: Test ArkoalaLazyNode GetFrameNodeIndex.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest010)
{
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    EXPECT_EQ(lazyNode->GetFrameNodeIndex(nullptr), -1);
    auto frameNode = CreateFrameNode(GetNextId());
    EXPECT_EQ(lazyNode->GetFrameNodeIndex(frameNode), -1);
    lazyNode->node4Index_.Put(0, frameNode);
    EXPECT_EQ(lazyNode->GetFrameNodeIndex(frameNode), 0);
}

/**
 * @tc.name: ArkoalaLazyNodeTest011
 * @tc.desc: Test ArkoalaLazyNode GetChildren.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest011)
{
    auto repeatNode = CreateRepeatNode(GetNextId());
    EXPECT_EQ(repeatNode->children_.size(), 0);
    CreateChildren(repeatNode, TOTAL_COUNT);
    auto ret = repeatNode->GetChildren();
    EXPECT_EQ(ret.size(), TOTAL_COUNT);

    repeatNode->children_.clear();
    ret = repeatNode->GetChildren();
    EXPECT_EQ(ret.size(), TOTAL_COUNT);
}

/**
 * @tc.name: ArkoalaLazyNodeTest012
 * @tc.desc: Test ArkoalaLazyNode ForEachL1Node.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest012)
{
    auto repeatNode = CreateRepeatNode(GetNextId());
    CreateChildren(repeatNode, TOTAL_COUNT);
    std::list<RefPtr<UINode>> ret;
    repeatNode->ForEachL1Node([&ret, this](int32_t index, const RefPtr<UINode>& node) {
        ret.push_back(node);
    });
    EXPECT_EQ(ret.size(), TOTAL_COUNT);
}

/**
 * @tc.name: ArkoalaLazyNodeTest013
 * @tc.desc: Test ArkoalaLazyNode DumpUINode.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest013)
{
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    EXPECT_EQ(lazyNode->DumpUINode(nullptr), "UINode: nullptr");
    int32_t nodeId = GetNextId();
    auto node = CreateTestUINode(nodeId);
    const std::string ret = "UINode: TestUINode(" + std::to_string(nodeId) + ")";
    EXPECT_EQ(lazyNode->DumpUINode(node), ret);
}

/**
 * @tc.name: ArkoalaLazyNodeTest014
 * @tc.desc: Test ArkoalaLazyNode RequestSyncTree.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest014)
{
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    CreateChildren(lazyNode, TOTAL_COUNT);
    lazyNode->children_.clear();
    lazyNode->RequestSyncTree();
    EXPECT_EQ(lazyNode->children_.size(), 0);
}

/**
 * @tc.name: ArkoalaLazyNodeTest015
 * @tc.desc: Test ArkoalaLazyNode PostIdleTask.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest015)
{
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    CreateChildren(lazyNode, TOTAL_COUNT);

    lazyNode->postUpdateTaskHasBeenScheduled_ = true;
    lazyNode->PostIdleTask();
    EXPECT_TRUE(lazyNode->postUpdateTaskHasBeenScheduled_);

    lazyNode->postUpdateTaskHasBeenScheduled_ = false;
    lazyNode->PostIdleTask();
    EXPECT_EQ(lazyNode->children_.size(), 0);
    EXPECT_TRUE(lazyNode->postUpdateTaskHasBeenScheduled_);
}

/**
 * @tc.name: ArkoalaLazyNodeTest016
 * @tc.desc: Test ArkoalaLazyNode BuildAllChildren.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest016)
{
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    int32_t totalCount = 50;
    CreateChildren(lazyNode, totalCount);
    lazyNode->BuildAllChildren();
    EXPECT_EQ(lazyNode->node4Index_.Size(), totalCount);
}

/**
 * @tc.name: ArkoalaLazyNodeTest018
 * @tc.desc: Test ArkoalaLazyNode ForEachL1NodeWithOnMove.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest018)
{
    auto repeatNode = CreateRepeatNode(GetNextId());
    CreateChildren(repeatNode, TOTAL_COUNT);
    std::list<RefPtr<UINode>> ret;
    repeatNode->ForEachL1NodeWithOnMove([&ret, this](const RefPtr<UINode>& node) {
        ret.push_back(node);
    });
    EXPECT_EQ(ret.size(), TOTAL_COUNT);
}

/**
 * @tc.name: ArkoalaLazyNodeTest019
 * @tc.desc: Test ArkoalaLazyNode BuildAllChildren.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest019)
{
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    int32_t totalCount = 50;
    CreateChildren(lazyNode, totalCount);
    lazyNode->BuildAllChildren();
    EXPECT_EQ(lazyNode->children_.size(), totalCount);
}

/**
 * @tc.name: ConvertFromToIndex001
 * @tc.desc: Test ArkoalaLazyNode ConvertFromToIndex and ConvertFromToIndexRevert.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ConvertFromToIndex001)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    lazyNode->UpdateMoveFromTo(0, 1);
    lazyNode->UpdateMoveFromTo(1, 2);
    lazyNode->UpdateMoveFromTo(2, 3);

    /**
     * @tc.steps: step1.
     * @tc.expected: index is 0, mappedIndex should be 1.
     */
    int32_t mappedIndex = lazyNode->ConvertFromToIndex(0);
    EXPECT_EQ(mappedIndex, 1);

    /**
     * @tc.steps: step2.
     * @tc.expected: index is 1, mappedIndex should be 2.
     */
    mappedIndex = lazyNode->ConvertFromToIndex(1);
    EXPECT_EQ(mappedIndex, 2);

    /**
     * @tc.steps: step3.
     * @tc.expected: index is 2, mappedIndex should be 3.
     */
    mappedIndex = lazyNode->ConvertFromToIndex(2);
    EXPECT_EQ(mappedIndex, 3);

    /**
     * @tc.steps: step4.
     * @tc.expected: index is 3, mappedIndex should be 0.
     */
    mappedIndex = lazyNode->ConvertFromToIndex(3);
    EXPECT_EQ(mappedIndex, 0);

    /**
     * @tc.steps: step5.
     * @tc.expected: index is 4, 5, 6, mappedIndex should be same with index.
     */
    mappedIndex = lazyNode->ConvertFromToIndex(4);
    EXPECT_EQ(mappedIndex, 4);
    mappedIndex = lazyNode->ConvertFromToIndex(5);
    EXPECT_EQ(mappedIndex, 5);
    mappedIndex = lazyNode->ConvertFromToIndex(6);
    EXPECT_EQ(mappedIndex, 6);
}

/**
 * @tc.name: ConvertFromToIndex002
 * @tc.desc: Test ArkoalaLazyNode ConvertFromToIndexRevert.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ConvertFromToIndex002)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    lazyNode->UpdateMoveFromTo(0, 1);
    lazyNode->UpdateMoveFromTo(1, 2);
    lazyNode->UpdateMoveFromTo(2, 3);

    /**
     * @tc.steps: step1.
     * @tc.expected: index is 0, mappedIndex should be 3.
     */
    int32_t mappedIndex = lazyNode->ConvertFromToIndexRevert(0);
    EXPECT_EQ(mappedIndex, 3);

    /**
     * @tc.steps: step2.
     * @tc.expected: index is 1, mappedIndex should be 0.
     */
    mappedIndex = lazyNode->ConvertFromToIndexRevert(1);
    EXPECT_EQ(mappedIndex, 0);

    /**
     * @tc.steps: step3.
     * @tc.expected: index is 2, mappedIndex should be 1.
     */
    mappedIndex = lazyNode->ConvertFromToIndexRevert(2);
    EXPECT_EQ(mappedIndex, 1);

    /**
     * @tc.steps: step4.
     * @tc.expected: index is 3, mappedIndex should be 2.
     */
    mappedIndex = lazyNode->ConvertFromToIndexRevert(3);
    EXPECT_EQ(mappedIndex, 2);

    /**
     * @tc.steps: step5.
     * @tc.expected: index is 4, 5, 6, mappedIndex should be same with index.
     */
    mappedIndex = lazyNode->ConvertFromToIndexRevert(4);
    EXPECT_EQ(mappedIndex, 4);
    mappedIndex = lazyNode->ConvertFromToIndexRevert(5);
    EXPECT_EQ(mappedIndex, 5);
    mappedIndex = lazyNode->ConvertFromToIndexRevert(6);
    EXPECT_EQ(mappedIndex, 6);
}

/**
 * @tc.name: IsInActiveRange001
 * @tc.desc: Test ArkoalaLazyNode IsInActiveRange.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, IsInActiveRange001)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    lazyNode->totalCount_ = 10;
    lazyNode->isLoop_ = false;
    int32_t cachedCount = 2;
    int32_t activeStart = 3;
    int32_t activeEnd = 5;
    ActiveRangeParam activeRangeParam = {activeStart, activeEnd, cachedCount, cachedCount};
    bool isInRange;

    /**
     * @tc.steps: step1. Test index in active range
     * @tc.expected: index [3,5] should be in active range
     */
    isInRange = lazyNode->IsInActiveRange(3, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInActiveRange(5, activeRangeParam);
    EXPECT_TRUE(isInRange);

    /**
     * @tc.steps: step2. Test index out of active range
     * @tc.expected: index [0,2] & [6,9] should be out of active range
     */
    isInRange = lazyNode->IsInActiveRange(0, activeRangeParam);
    EXPECT_FALSE(isInRange);
    isInRange = lazyNode->IsInActiveRange(2, activeRangeParam);
    EXPECT_FALSE(isInRange);
    isInRange = lazyNode->IsInActiveRange(6, activeRangeParam);
    EXPECT_FALSE(isInRange);
    isInRange = lazyNode->IsInActiveRange(9, activeRangeParam);
    EXPECT_FALSE(isInRange);
}

/**
 * @tc.name: IsInActiveRange002
 * @tc.desc: Test ArkoalaLazyNode IsInActiveRange with isLoop = true.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, IsInActiveRange002)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    lazyNode->totalCount_ = 10;
    lazyNode->isLoop_ = true;
    int32_t cachedCount = 2;
    int32_t activeStart = 7;
    int32_t activeEnd = 1;
    ActiveRangeParam activeRangeParam = {activeStart, activeEnd, cachedCount, cachedCount};
    bool isInRange;

    /**
     * @tc.steps: step1. Test index in active range
     * @tc.expected: index [7,9] & [0,1] should be in active range
     */
    isInRange = lazyNode->IsInActiveRange(7, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInActiveRange(9, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInActiveRange(0, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInActiveRange(1, activeRangeParam);
    EXPECT_TRUE(isInRange);

    /**
     * @tc.steps: step2. Test index out of active range
     * @tc.expected: index [2,6] should be out of active range
     */
    isInRange = lazyNode->IsInActiveRange(2, activeRangeParam);
    EXPECT_FALSE(isInRange);
    isInRange = lazyNode->IsInActiveRange(6, activeRangeParam);
    EXPECT_FALSE(isInRange);
}

/**
 * @tc.name: IsInCacheRange001
 * @tc.desc: Test ArkoalaLazyNode IsInCacheRange.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, IsInCacheRange001)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    lazyNode->totalCount_ = 10;
    lazyNode->isLoop_ = false;
    int32_t cachedCount = 2;
    bool isInRange;
    ActiveRangeParam activeRangeParam;

    /**
     * @tc.steps: step1. Test linear container boundary scenario
     * @tc.expected: (0,2,2,2) -> [0,4]
     */
    activeRangeParam = {0, 2, cachedCount, cachedCount};
    isInRange = lazyNode->IsInCacheRange(0, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(4, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(-1, activeRangeParam);
    EXPECT_FALSE(isInRange);
    isInRange = lazyNode->IsInCacheRange(5, activeRangeParam);
    EXPECT_FALSE(isInRange);
    /**
     * @tc.steps: step2. Test linear container normal scenario
     * @tc.expected: (3,5,2,2) -> [1,7]
     */
    activeRangeParam = {3, 5, cachedCount, cachedCount};
    isInRange = lazyNode->IsInCacheRange(1, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(7, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(0, activeRangeParam);
    EXPECT_FALSE(isInRange);
    isInRange = lazyNode->IsInCacheRange(8, activeRangeParam);
    EXPECT_FALSE(isInRange);
    /**
     * @tc.steps: step3. Test linear container boundary scenario
     * @tc.expected: (7,9,2,2) -> [5,9]
     */
    activeRangeParam = {7, 9, cachedCount, cachedCount};
    isInRange = lazyNode->IsInCacheRange(5, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(9, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(4, activeRangeParam);
    EXPECT_FALSE(isInRange);
    isInRange = lazyNode->IsInCacheRange(10, activeRangeParam);
    EXPECT_FALSE(isInRange);
}

/**
 * @tc.name: IsInCacheRange002
 * @tc.desc: Test ArkoalaLazyNode IsInCacheRange with isLoop = true.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, IsInCacheRange002)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    lazyNode->totalCount_ = 10;
    lazyNode->isLoop_ = true;
    int32_t cachedCount = 2;
    bool isInRange;
    ActiveRangeParam activeRangeParam;

    /**
     * @tc.steps: step1. Test loop container normal scenario
     * @tc.expected: (3,5,2,2,true) -> [1,7]
     */
    activeRangeParam = {3, 5, cachedCount, cachedCount};
    isInRange = lazyNode->IsInCacheRange(1, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(7, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(0, activeRangeParam);
    EXPECT_FALSE(isInRange);
    isInRange = lazyNode->IsInCacheRange(8, activeRangeParam);
    EXPECT_FALSE(isInRange);
    /**
     * @tc.steps: step2. Test loop container boundary scenario
     * @tc.expected: (0,2,2,2,true) -> [0,4] & [8,9]
     */
    activeRangeParam = {0, 2, cachedCount, cachedCount};
    isInRange = lazyNode->IsInCacheRange(0, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(4, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(8, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(9, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(5, activeRangeParam);
    EXPECT_FALSE(isInRange);
    isInRange = lazyNode->IsInCacheRange(7, activeRangeParam);
    EXPECT_FALSE(isInRange);
    /**
     * @tc.steps: step3. Test loop container boundary scenario
     * @tc.expected: (7,9,2,2,true) -> [0,1] & [5,9]
     */
    activeRangeParam = {7, 9, cachedCount, cachedCount};
    isInRange = lazyNode->IsInCacheRange(0, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(1, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(5, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(9, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(2, activeRangeParam);
    EXPECT_FALSE(isInRange);
    isInRange = lazyNode->IsInCacheRange(4, activeRangeParam);
    EXPECT_FALSE(isInRange);
}

/**
 * @tc.name: IsInCacheRange003
 * @tc.desc: Test ArkoalaLazyNode IsInCacheRange with isLoop = true.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, IsInCacheRange003)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    lazyNode->totalCount_ = 10;
    lazyNode->isLoop_ = true;
    int32_t cachedCount = 2;
    bool isInRange;
    ActiveRangeParam activeRangeParam;

    /**
     * @tc.steps: step1. Test loop container end to end connected scenario
     * @tc.expected: (9,1,2,2,true) -> [0,3] & [7,9]
     */
    activeRangeParam = {9, 1, cachedCount, cachedCount};
    isInRange = lazyNode->IsInCacheRange(0, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(3, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(7, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(9, activeRangeParam);
    EXPECT_TRUE(isInRange);
    isInRange = lazyNode->IsInCacheRange(4, activeRangeParam);
    EXPECT_FALSE(isInRange);
    isInRange = lazyNode->IsInCacheRange(6, activeRangeParam);
    EXPECT_FALSE(isInRange);
    /**
     * @tc.steps: step2. Test loop container full scenario
     * @tc.expected: (5,2,2,2,true) -> all in range
     */
    activeRangeParam = {5, 2, cachedCount, cachedCount};
    for (int32_t i = 0; i < lazyNode->totalCount_; ++i) {
        isInRange = lazyNode->IsInCacheRange(i, activeRangeParam);
        EXPECT_TRUE(isInRange);
    }
    isInRange = lazyNode->IsInCacheRange(-1, activeRangeParam);
    EXPECT_FALSE(isInRange);
    isInRange = lazyNode->IsInCacheRange(10, activeRangeParam);
    EXPECT_FALSE(isInRange);
}

/**
 * @tc.name: IsInCacheRange004
 * @tc.desc: Test ArkoalaLazyNode IsInCacheRange with isLoop = true.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, IsInCacheRange004)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    int32_t total = 3;
    lazyNode->totalCount_ = total;
    lazyNode->isLoop_ = true;
    int32_t cachedCount = 1;
    bool isInRange;
    ActiveRangeParam activeRangeParam;

    /**
     * @tc.steps: step1. Test loop container full coverage case
     * @tc.expected: (0,1,1,1,true) -> [0,2]
     */
    activeRangeParam = {0, 1, cachedCount, cachedCount};
    for (int32_t i = 0; i < lazyNode->totalCount_; ++i) {
        isInRange = lazyNode->IsInCacheRange(i, activeRangeParam);
        EXPECT_TRUE(isInRange);
    }
}

/**
 * @tc.name: NeedBuildAll001
 * @tc.desc: Test ArkoalaLazyNode needBuildAll_ flag and BuildAll
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, NeedBuildAll001)
{
    /**
     * @tc.steps: step1. Test needBuildAll_ flag
     * @tc.expected: needBuildAll_ flag should be set and get correctly
     */
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    EXPECT_TRUE(lazyNode->needBuildAll_);
    lazyNode->SetNeedBuildAll(false);
    EXPECT_FALSE(lazyNode->needBuildAll_);

    auto repeatNode = CreateRepeatNode(GetNextId());
    EXPECT_TRUE(repeatNode->needBuildAll_);
    repeatNode->SetNeedBuildAll(false);
    EXPECT_FALSE(repeatNode->needBuildAll_);

    /**
     * @tc.steps: step2. Test ArkoalaLazyNode BuildAllChildren
     * @tc.expected: all children should be built
     */
    int32_t totalCount = 50;
    lazyNode->SetNeedBuildAll(true);
    lazyNode->SetTotalCount(totalCount);
    lazyNode->BuildAllChildren();
    EXPECT_EQ(lazyNode->GetChildren().size(), totalCount);

    repeatNode->SetNeedBuildAll(true);
    repeatNode->SetTotalCount(totalCount);
    repeatNode->BuildAllChildren();
    EXPECT_EQ(repeatNode->GetChildren().size(), totalCount);
}

/**
 * @tc.name: CleanCache002
 * @tc.desc: Test ArkoalaLazyNode CleanCache skips cache release for a non-scroll parent (needBuildAll_).
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, CleanCache002)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    int32_t releaseCount = 0;
    int32_t updateStart = -100; // sentinel: updateRange_ must not be invoked
    lazyNode->SetCallbacks(
        [this](int32_t, bool) { return CreateFrameNode(GetNextId()); },
        [&updateStart](int32_t start, int32_t end, int32_t cacheStart, int32_t cacheEnd, bool isLoop) {
            updateStart = start;
        },
        []() {},
        [&releaseCount](int32_t index) { ++releaseCount; });
 
    // Non-scroll parent (e.g. Column): needBuildAll_ keeps every child on screen, nothing to release.
    lazyNode->SetNeedBuildAll(true);
    lazyNode->options_.memOptStrategy = LazyForEachMemOptStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION;
    lazyNode->totalCount_ = TOTAL_COUNT;
    lazyNode->activeRangeParam_ = { 2, 2, 2, 2 };
    for (int32_t i = 0; i < 6; ++i) {
        lazyNode->node4Index_.Put(i, CreateFrameNode(GetNextId()));
    }
    lazyNode->pendingCleanCache_ = true;
    lazyNode->pendingRestoreCache_ = true;
 
    lazyNode->CleanCache(true);
 
    EXPECT_EQ(lazyNode->node4Index_.Size(), 6);
    EXPECT_FALSE(lazyNode->pendingCleanCache_);
    EXPECT_FALSE(lazyNode->pendingRestoreCache_);
}
 
/**
 * @tc.name: CleanCache003
 * @tc.desc: Test ArkoalaLazyNode CleanCache retains a reduced cache band capped at 2 (memory optimization).
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, CleanCache003)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    int32_t releaseCount = 0;
    std::vector<int32_t> releasedIndexes;
    int32_t updateCacheStart = -1;
    int32_t updateCacheEnd = -1;
    lazyNode->SetCallbacks(
        [this](int32_t, bool) { return CreateFrameNode(GetNextId()); },
        [&updateCacheStart, &updateCacheEnd](
            int32_t start, int32_t end, int32_t cacheStart, int32_t cacheEnd, bool isLoop) {
            updateCacheStart = cacheStart;
            updateCacheEnd = cacheEnd;
        },
        []() {},
        [&releaseCount, &releasedIndexes](int32_t index) {
            ++releaseCount;
            releasedIndexes.emplace_back(index);
        });
 
    lazyNode->SetNeedBuildAll(false);
    lazyNode->options_.memOptStrategy = LazyForEachMemOptStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION;
    lazyNode->totalCount_ = TOTAL_COUNT;
    // cacheStart/cacheEnd 4 is clamped to the max retained idle cache count (2): band is [0, 4].
    lazyNode->activeRangeParam_ = { 2, 2, 4, 4 };
    for (int32_t i = 0; i < 6; ++i) {
        lazyNode->node4Index_.Put(i, CreateFrameNode(GetNextId()));
    }
 
    lazyNode->CleanCache(true);
 
    // Active index 2 and cached band [0, 4] are kept, only index 5 is released.
    EXPECT_EQ(lazyNode->node4Index_.Size(), 5);
    EXPECT_FALSE(lazyNode->node4Index_.ContainsKey(5));
    EXPECT_EQ(releaseCount, 1);
    ASSERT_EQ(releasedIndexes.size(), 1);
    EXPECT_EQ(releasedIndexes[0], 5);
    // TS side is notified with the reduced (clamped) cache band.
    EXPECT_EQ(updateCacheStart, 2);
    EXPECT_EQ(updateCacheEnd, 2);
}
 
/**
 * @tc.name: CleanCache004
 * @tc.desc: Test ArkoalaLazyNode CleanCache with clearAll releases all idle nodes even without memOpt strategy.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, CleanCache004)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    int32_t releaseCount = 0;
    int32_t updateCacheStart = -1;
    int32_t updateCacheEnd = -1;
    lazyNode->SetCallbacks(
        [this](int32_t, bool) { return CreateFrameNode(GetNextId()); },
        [&updateCacheStart, &updateCacheEnd](
            int32_t start, int32_t end, int32_t cacheStart, int32_t cacheEnd, bool isLoop) {
            updateCacheStart = cacheStart;
            updateCacheEnd = cacheEnd;
        },
        []() {},
        [&releaseCount](int32_t index) { ++releaseCount; });
 
    lazyNode->SetNeedBuildAll(false);
    // clearAll bypasses the memory optimization strategy check.
    lazyNode->options_.memOptStrategy = LazyForEachMemOptStrategy::DEFAULT;
    lazyNode->totalCount_ = TOTAL_COUNT;
    lazyNode->activeRangeParam_ = { 2, 2, 4, 4 };
    for (int32_t i = 0; i < 6; ++i) {
        lazyNode->node4Index_.Put(i, CreateFrameNode(GetNextId()));
    }
 
    lazyNode->CleanCache(true, true);
 
    // Only the active range [2, 2] survives; the retained cache band shrinks to 0.
    EXPECT_EQ(lazyNode->node4Index_.Size(), 1);
    EXPECT_TRUE(lazyNode->node4Index_.ContainsKey(2));
    EXPECT_EQ(releaseCount, 5);
    EXPECT_EQ(updateCacheStart, 0);
    EXPECT_EQ(updateCacheEnd, 0);
}
 
/**
 * @tc.name: CleanCache005
 * @tc.desc: Test ArkoalaLazyNode CleanCache before layout (never-set range) retains nothing.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, CleanCache005)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    int32_t releaseCount = 0;
    int32_t updateStart = -100;
    int32_t updateEnd = -100;
    int32_t updateCacheStart = -1;
    int32_t updateCacheEnd = -1;
    lazyNode->SetCallbacks(
        [this](int32_t, bool) { return CreateFrameNode(GetNextId()); },
        [&updateStart, &updateEnd, &updateCacheStart, &updateCacheEnd](
            int32_t start, int32_t end, int32_t cacheStart, int32_t cacheEnd, bool isLoop) {
            updateStart = start;
            updateEnd = end;
            updateCacheStart = cacheStart;
            updateCacheEnd = cacheEnd;
        },
        []() {},
        [&releaseCount](int32_t index) { ++releaseCount; });
 
    lazyNode->SetNeedBuildAll(false);
    lazyNode->options_.memOptStrategy = LazyForEachMemOptStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION;
    lazyNode->totalCount_ = TOTAL_COUNT;
    // Never laid out: active range stays {-1, -1, -1, -1}.
    lazyNode->activeRangeParam_ = { -1, -1, -1, -1 };
    for (int32_t i = 0; i < 3; ++i) {
        lazyNode->node4Index_.Put(i, CreateFrameNode(GetNextId()));
    }
 
    lazyNode->CleanCache(true);
 
    // Parity with the dynamic side: startIndex_/endIndex_ == -1 retains nothing.
    EXPECT_EQ(lazyNode->node4Index_.Size(), 0);
    EXPECT_EQ(releaseCount, 3);
    EXPECT_EQ(updateStart, -1);
    EXPECT_EQ(updateEnd, -1);
    EXPECT_EQ(updateCacheStart, 0);
    EXPECT_EQ(updateCacheEnd, 0);
}
 
/**
 * @tc.name: PurgeNode001
 * @tc.desc: Test ArkoalaLazyNode PurgeNode keeps the whole node4Index_ map for a non-scroll parent.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, PurgeNode001)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    lazyNode->SetNeedBuildAll(true);
    lazyNode->options_.releaseStrategy = LazyForEachReleaseStrategy::BATCH;
    lazyNode->totalCount_ = TOTAL_COUNT;
    lazyNode->activeRangeParam_ = { 2, 2, 1, 1 };
    for (int32_t i = 0; i < 5; ++i) {
        lazyNode->node4Index_.Put(i, CreateFrameNode(GetNextId()));
    }
 
    lazyNode->PurgeNode();
 
    // The idle-task backstop must not drop on-screen children of a non-scroll parent.
    EXPECT_EQ(lazyNode->node4Index_.Size(), 5);
}
 
/**
 * @tc.name: PurgeNode002
 * @tc.desc: Test ArkoalaLazyNode PurgeNode removes indexes outside the cache range for a scroll parent.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, PurgeNode002)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    lazyNode->SetNeedBuildAll(false);
    lazyNode->options_.releaseStrategy = LazyForEachReleaseStrategy::BATCH;
    lazyNode->totalCount_ = TOTAL_COUNT;
    lazyNode->activeRangeParam_ = { 2, 2, 1, 1 };
    for (int32_t i = 0; i < 5; ++i) {
        lazyNode->node4Index_.Put(i, CreateFrameNode(GetNextId()));
    }
 
    lazyNode->PurgeNode();
 
    // Cache band is [1, 3]: indexes 0 and 4 are purged, 1..3 are kept.
    EXPECT_EQ(lazyNode->node4Index_.Size(), 3);
    EXPECT_FALSE(lazyNode->node4Index_.ContainsKey(0));
    EXPECT_TRUE(lazyNode->node4Index_.ContainsKey(1));
    EXPECT_TRUE(lazyNode->node4Index_.ContainsKey(2));
    EXPECT_TRUE(lazyNode->node4Index_.ContainsKey(3));
    EXPECT_FALSE(lazyNode->node4Index_.ContainsKey(4));
}
 
/**
 * @tc.name: RestoreCache001
 * @tc.desc: Test ArkoalaLazyNode RestoreCache skips rebuild for a non-scroll parent.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, RestoreCache001)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    int32_t createCount = 0;
    lazyNode->SetCallbacks(
        [this, &createCount](int32_t index, bool skipCreate) {
            ++createCount;
            return CreateFrameNode(GetNextId());
        },
        [](int32_t start, int32_t end, int32_t cacheStart, int32_t cacheEnd, bool isLoop) {},
        []() {},
        [](int32_t index) {});
 
    lazyNode->SetNeedBuildAll(true);
    lazyNode->options_.memOptStrategy = LazyForEachMemOptStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION;
    lazyNode->totalCount_ = TOTAL_COUNT;
    lazyNode->activeRangeParam_ = { 2, 2, 1, 1 };
    lazyNode->pendingCleanCache_ = true;
    lazyNode->pendingRestoreCache_ = true;
 
    lazyNode->RestoreCache();
 
    // Nothing was released on hide for a non-scroll parent, so nothing is rebuilt.
    EXPECT_EQ(lazyNode->node4Index_.Size(), 0);
    EXPECT_FALSE(lazyNode->pendingCleanCache_);
    EXPECT_FALSE(lazyNode->pendingRestoreCache_);
}
 
/**
 * @tc.name: RestoreCache002
 * @tc.desc: Test ArkoalaLazyNode RestoreCache rebuilds only the cache band for a scroll parent.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, RestoreCache002)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    int32_t createCount = 0;
    std::vector<int32_t> createdIndexes;
    lazyNode->SetCallbacks(
        [this, &createCount, &createdIndexes](int32_t index, bool skipCreate) {
            ++createCount;
            createdIndexes.emplace_back(index);
            return CreateFrameNode(GetNextId());
        },
        [](int32_t start, int32_t end, int32_t cacheStart, int32_t cacheEnd, bool isLoop) {},
        []() {},
        [](int32_t index) {});
 
    lazyNode->SetNeedBuildAll(false);
    lazyNode->options_.memOptStrategy = LazyForEachMemOptStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION;
    lazyNode->totalCount_ = TOTAL_COUNT;
    lazyNode->activeRangeParam_ = { 2, 2, 1, 1 };
 
    lazyNode->RestoreCache();
 
    // Cache band is [1, 3] with active [2, 2]: only idle caches 1 and 3 are rebuilt.
    EXPECT_EQ(createCount, 2);
    ASSERT_EQ(createdIndexes.size(), 2);
    EXPECT_EQ(createdIndexes[0], 1);
    EXPECT_EQ(createdIndexes[1], 3);
    EXPECT_EQ(lazyNode->node4Index_.Size(), 2);
    EXPECT_TRUE(lazyNode->node4Index_.ContainsKey(1));
    EXPECT_TRUE(lazyNode->node4Index_.ContainsKey(3));
}
 
/**
 * @tc.name: GetReducedCacheRangeParam001
 * @tc.desc: Test GetReducedCacheRangeParam when start >= 0 but end < 0 (short-circuit branch: start ok, end < 0).
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, GetReducedCacheRangeParam001)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    lazyNode->SetCallbacks(createItemCb_, updateRangeCb_, clearCacheCb_, releaseItemByIndexCb_);
    lazyNode->SetNeedBuildAll(false);
    lazyNode->options_.memOptStrategy = LazyForEachMemOptStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION;
    lazyNode->totalCount_ = TOTAL_COUNT;
    // start >= 0, end < 0: should take the "end < 0" branch and retain nothing.
    lazyNode->activeRangeParam_ = { 2, -1, 1, 1 };
    for (int32_t i = 0; i < 3; ++i) {
        lazyNode->node4Index_.Put(i, CreateFrameNode(GetNextId()));
    }
 
    lazyNode->CleanCache(true);
 
    // end < 0 means retainNothing = true, all nodes released.
    EXPECT_EQ(lazyNode->node4Index_.Size(), 0);
}
 
/**
 * @tc.name: CleanCache006
 * @tc.desc: Test CleanCache with isRepeat_ true but no repeat memOpt strategy and clearAll false —
 *           should skip both the repeat early-return and the lazyForEach release block.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, CleanCache006)
{
    auto repeatNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId(), true);
    int32_t releaseCount = 0;
    repeatNode->SetCallbacks(
        [this](int32_t, bool) { return CreateFrameNode(GetNextId()); },
        [](int32_t, int32_t, int32_t, int32_t, bool) {},
        []() {},
        [&releaseCount](int32_t index) { ++releaseCount; });
 
    // isRepeat_ == true, but repeat strategy is DEFAULT and clearAll is false,
    // so the repeat early-return is skipped and the !isRepeat_ block is also skipped.
    repeatNode->SetNeedBuildAll(false);
    repeatNode->repeatMemoryOptimizationStrategy_ = RepeatMemoryOptimizationStrategy::DEFAULT;
    repeatNode->options_.memOptStrategy = LazyForEachMemOptStrategy::DEFAULT;
    repeatNode->totalCount_ = TOTAL_COUNT;
    repeatNode->activeRangeParam_ = { 2, 2, 1, 1 };
    for (int32_t i = 0; i < 5; ++i) {
        repeatNode->node4Index_.Put(i, CreateFrameNode(GetNextId()));
    }
    repeatNode->pendingCleanCache_ = true;
    repeatNode->pendingRestoreCache_ = true;
 
    repeatNode->CleanCache(true);
 
    // Neither the repeat path nor the lazyForEach path executes: node4Index_ is untouched.
    EXPECT_EQ(repeatNode->node4Index_.Size(), 5);
    EXPECT_EQ(releaseCount, 0);
    EXPECT_FALSE(repeatNode->pendingCleanCache_);
    EXPECT_FALSE(repeatNode->pendingRestoreCache_);
}
 
/**
 * @tc.name: CleanCache007
 * @tc.desc: Test CleanCache with !isRepeat_ && !needBuildAll_ but memOptStrategy != ENABLE and clearAll false —
 *           should skip the release block (covers the false branch of the || in line 1016).
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, CleanCache007)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    int32_t releaseCount = 0;
    lazyNode->SetCallbacks(
        [this](int32_t, bool) { return CreateFrameNode(GetNextId()); },
        [](int32_t, int32_t, int32_t, int32_t, bool) {},
        []() {},
        [&releaseCount](int32_t index) { ++releaseCount; });
 
    // !isRepeat_ && !needBuildAll_ but memOptStrategy is DEFAULT and clearAll is false.
    lazyNode->SetNeedBuildAll(false);
    lazyNode->options_.memOptStrategy = LazyForEachMemOptStrategy::DEFAULT;
    lazyNode->totalCount_ = TOTAL_COUNT;
    lazyNode->activeRangeParam_ = { 2, 2, 1, 1 };
    for (int32_t i = 0; i < 5; ++i) {
        lazyNode->node4Index_.Put(i, CreateFrameNode(GetNextId()));
    }
 
    lazyNode->CleanCache(true);
 
    // The release block is skipped: no nodes released.
    EXPECT_EQ(lazyNode->node4Index_.Size(), 5);
    EXPECT_EQ(releaseCount, 0);
}
 
/**
 * @tc.name: CleanCache008
 * @tc.desc: Test CleanCache when willRelease is true but releaseItemByIndex_ is not set —
 *           nodes are still removed from node4Index_ but release callback is not invoked.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, CleanCache008)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    // Set releaseItemByIndex_ to nullptr by not providing it.
    lazyNode->SetCallbacks(
        [this](int32_t, bool) { return CreateFrameNode(GetNextId()); },
        [](int32_t, int32_t, int32_t, int32_t, bool) {},
        []() {},
        nullptr);
 
    lazyNode->SetNeedBuildAll(false);
    lazyNode->options_.memOptStrategy = LazyForEachMemOptStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION;
    lazyNode->totalCount_ = TOTAL_COUNT;
    lazyNode->activeRangeParam_ = { 2, 2, 1, 1 };
    for (int32_t i = 0; i < 5; ++i) {
        lazyNode->node4Index_.Put(i, CreateFrameNode(GetNextId()));
    }
 
    // Should not crash even though releaseItemByIndex_ is null.
    lazyNode->CleanCache(true);
 
    // Nodes outside the active+cache range are still removed from node4Index_.
    // Active [2,2] + cache [1,3] -> indexes 0 and 4 are released (removed from map).
    EXPECT_EQ(lazyNode->node4Index_.Size(), 3);
    EXPECT_FALSE(lazyNode->node4Index_.ContainsKey(0));
    EXPECT_FALSE(lazyNode->node4Index_.ContainsKey(4));
}
 
/**
 * @tc.name: RestoreCache003
 * @tc.desc: Test RestoreCache with isRepeat_ true — should enter the repeat branch path.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, RestoreCache003)
{
    auto repeatNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId(), true);
    int32_t createCount = 0;
    repeatNode->SetCallbacks(
        [this, &createCount](int32_t index, bool skipCreate) {
            ++createCount;
            return CreateFrameNode(GetNextId());
        },
        [](int32_t, int32_t, int32_t, int32_t, bool) {},
        []() {},
        [](int32_t) {});
 
    // isRepeat_ == true with repeat strategy ENABLE_AUTO_CACHE_OPTIMIZATION.
    repeatNode->SetNeedBuildAll(false);
    repeatNode->repeatMemoryOptimizationStrategy_ =
        RepeatMemoryOptimizationStrategy::ENABLE_AUTO_CACHE_OPTIMIZATION;
    repeatNode->totalCount_ = TOTAL_COUNT;
    repeatNode->activeRangeParam_ = { 2, 2, 1, 1 };
    repeatNode->pendingCleanCache_ = true;
    repeatNode->pendingRestoreCache_ = true;
 
    repeatNode->RestoreCache();
 
    // Should have attempted to rebuild cache items outside the cache range.
    EXPECT_FALSE(repeatNode->pendingCleanCache_);
    EXPECT_FALSE(repeatNode->pendingRestoreCache_);
}
 
/**
 * @tc.name: RestoreCache004
 * @tc.desc: Test RestoreCache with !isRepeat_ && !needBuildAll_ but memOptStrategy != ENABLE —
 *           should skip the lazyForEach restore block (covers false branch on line 1058).
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, RestoreCache004)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    int32_t createCount = 0;
    lazyNode->SetCallbacks(
        [this, &createCount](int32_t index, bool skipCreate) {
            ++createCount;
            return CreateFrameNode(GetNextId());
        },
        [](int32_t, int32_t, int32_t, int32_t, bool) {},
        []() {},
        [](int32_t) {});
 
    // !isRepeat_ && !needBuildAll_ but memOptStrategy is DEFAULT.
    lazyNode->SetNeedBuildAll(false);
    lazyNode->options_.memOptStrategy = LazyForEachMemOptStrategy::DEFAULT;
    lazyNode->totalCount_ = TOTAL_COUNT;
    lazyNode->activeRangeParam_ = { 2, 2, 1, 1 };
    lazyNode->pendingCleanCache_ = true;
    lazyNode->pendingRestoreCache_ = true;
 
    lazyNode->RestoreCache();
 
    // The lazyForEach restore block is skipped: no items rebuilt.
    EXPECT_EQ(createCount, 0);
    EXPECT_FALSE(lazyNode->pendingCleanCache_);
    EXPECT_FALSE(lazyNode->pendingRestoreCache_);
}

/**
 * @tc.name: ArkoalaLazyNodeRepeatMemOptDump001
 * @tc.desc: Test ArkoalaLazyNode(Repeat) DumpInfo shows RepeatMemOpt enablement.
 *           Covers the mem-opt (auto cache clear) dump display on Repeat nodes.
 * @tc.type: FUNC
 */
HWTEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeRepeatMemOptDump001, TestSize.Level1)
{
    auto repeatNode = CreateRepeatNode(GetNextId());
    ASSERT_NE(repeatNode, nullptr);

    auto findMemOpt = [](const std::string& needle) -> bool {
        for (const auto& desc : DumpLog::GetInstance().description_) {
            if (desc.find(needle) != std::string::npos) {
                return true;
            }
        }
        return false;
    };

    /**
     * @tc.steps: step1. ENABLE_AUTO_CACHE_OPTIMIZATION -> dump shows "enabled".
     */
    repeatNode->SetRepeatMemoryOptimizationStrategy(1);
    DumpLog::GetInstance().description_.clear();
    repeatNode->DumpInfo();
    EXPECT_TRUE(findMemOpt("RepeatMemOpt: enabled"));

    /**
     * @tc.steps: step2. DEFAULT -> dump shows "disabled".
     */
    repeatNode->SetRepeatMemoryOptimizationStrategy(0);
    DumpLog::GetInstance().description_.clear();
    repeatNode->DumpInfo();
    EXPECT_TRUE(findMemOpt("RepeatMemOpt: disabled"));

    DumpLog::GetInstance().description_.clear();
}

/**
 * @tc.name: ArkoalaLazyNodeRepeatMemOptDump002
 * @tc.desc: Non-repeat ArkoalaLazyNode must NOT emit RepeatMemOpt (only isRepeat_ nodes do).
 * @tc.type: FUNC
 */
HWTEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeRepeatMemOptDump002, TestSize.Level1)
{
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    ASSERT_NE(lazyNode, nullptr);
    lazyNode->SetRepeatMemoryOptimizationStrategy(1);
    DumpLog::GetInstance().description_.clear();
    lazyNode->DumpInfo();
    bool found = false;
    for (const auto& desc : DumpLog::GetInstance().description_) {
        if (desc.find("RepeatMemOpt:") != std::string::npos) {
            found = true;
            break;
        }
    }
    EXPECT_FALSE(found);
    DumpLog::GetInstance().description_.clear();
}
} // namespace OHOS::Ace::NG
