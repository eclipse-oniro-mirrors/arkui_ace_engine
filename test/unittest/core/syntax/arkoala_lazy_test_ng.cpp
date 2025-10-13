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

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
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
        lazyForEachNode->SetCallbacks(createItemCb_, updateRangeCb_);
        return lazyForEachNode;
    }

    RefPtr<ArkoalaLazyNode> CreateRepeatNode(int32_t nodeId)
    {
        auto* stack = ViewStackProcessor::GetInstance();
        auto repeatNode = AceType::MakeRefPtr<ArkoalaLazyNode>(nodeId, true);
        stack->Push(repeatNode);
        repeatNode->SetCallbacks(createItemCb_, updateRangeCb_);
        return repeatNode;
    }

    void CreateChildren(RefPtr<ArkoalaLazyNode>& lazyNode, int32_t totalCount)
    {
        for (int i = 0; i < totalCount; ++i) {
            lazyNode->GetFrameChildByIndex(i, true, false, true);
        }
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

    ArkoalaLazyNode::CreateItemCb createItemCb_ = [this](int32_t idx) {
        RefPtr<TestUINode> uiNode = CreateTestUINode(GetNextId());
        auto columnNode = ColumnModelNG::CreateFrameNode(GetNextId());
        uiNode->AddChild(columnNode);
        return uiNode;
    };
    ArkoalaLazyNode::UpdateRangeCb updateRangeCb_ = [](int32_t start, int32_t end) {};
};

const int32_t INDEX_1 = 1;
const int32_t INDEX_2 = 2;
const int32_t INDEX_8 = 8;
const int32_t INDEX_9 = 9;
const int32_t TOTAL_COUNT = 10;
const int32_t CACHED_COUNT = 2;

/**
 * @tc.name: ArkoalaLazyNodeTest001
 * @tc.desc: Test ArkoalaLazyNode GetFrameChildByIndex.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest001)
{
    /**
     * @tc.steps: step1. Test LazyForEach node GetFrameChildByIndex
     */
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    EXPECT_EQ(lazyNode->GetTag(), V2::JS_LAZY_FOR_EACH_ETS_TAG);
    lazyNode->SetCallbacks([](int32_t idx) { return ColumnModelNG::CreateFrameNode(-1); },
        [](int32_t start, int32_t end) {
            EXPECT_EQ(start, INDEX_8);
            EXPECT_EQ(end, INDEX_8);
        });
    lazyNode->SetTotalCount(TOTAL_COUNT);
    EXPECT_EQ(lazyNode->FrameCount(), TOTAL_COUNT);

    EXPECT_FALSE(lazyNode->GetChildByIndex(INDEX_9));
    EXPECT_FALSE(lazyNode->GetFrameChildByIndex(INDEX_9, false, false, false));

    /**
     * @tc.steps: step1.1 Test LazyForEach node GetFrameChildByIndex with needBuild = true
     */
    RefPtr<UINode> childNode = lazyNode->GetFrameChildByIndex(INDEX_9, true, false, true);
    EXPECT_TRUE(childNode);
    EXPECT_TRUE(lazyNode->GetChildByIndex(INDEX_9));
    EXPECT_TRUE(AceType::DynamicCast<FrameNode>(childNode)->IsActive());

    /**
     * @tc.steps: step1.2 Create node for index 8
     */
    childNode = lazyNode->GetFrameChildByIndex(INDEX_8, true, false, true);
    EXPECT_TRUE(childNode);

    /**
     * @tc.steps: step1.3 Test LazyForEach node DoSetActiveChildRange to activate childNode
     */
    lazyNode->DoSetActiveChildRange(INDEX_8, INDEX_8, 0, 0, false);
    EXPECT_FALSE(lazyNode->GetChildByIndex(INDEX_9));
    EXPECT_TRUE(AceType::DynamicCast<FrameNode>(childNode)->IsActive());

    /**
     * @tc.steps: step2. Test GetParent for Repeat node
     */
    auto repeatNode = CreateRepeatNode(GetNextId());
    EXPECT_EQ(repeatNode->GetTag(), V2::JS_REPEAT_ETS_TAG);
    CreateChildren(repeatNode, TOTAL_COUNT);
    auto retNode = repeatNode->GetFrameChildByIndex(INDEX_1, true, false, true);
    EXPECT_TRUE(retNode);
    EXPECT_EQ(retNode->GetParent()->GetParent(), repeatNode);
}

/**
 * @tc.name: ArkoalaLazyNodeTest002
 * @tc.desc: Test ArkoalaLazyNode GetChildByIndex.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest002)
{
    /**
     * @tc.steps: step1. Create LazyForEach node and set callbacks
     */
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    ArkoalaLazyNode::CreateItemCb createItemCb = [this](int32_t idx) {
        RefPtr<TestUINode> uiNode = CreateTestUINode(GetNextId());
        auto columnNode = ColumnModelNG::CreateFrameNode(GetNextId());
        uiNode->AddChild(columnNode);
        EXPECT_EQ(uiNode->GetChildren().size(), 1);
        return uiNode;
    };
    ArkoalaLazyNode::UpdateRangeCb updateRangeCb = [](int32_t start, int32_t end) {};
    lazyNode->SetCallbacks(createItemCb, updateRangeCb);

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
 * @tc.desc: Test ArkoalaLazyNode GetFrameNode.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest003)
{
    auto lazyNode = AceType::MakeRefPtr<ArkoalaLazyNode>(GetNextId());
    ArkoalaLazyNode::CreateItemCb createItemCb = [this](int32_t idx) {
        return ColumnModelNG::CreateFrameNode(GetNextId());
    };
    ArkoalaLazyNode::UpdateRangeCb updateRangeCb = [](int32_t start, int32_t end) {};
    lazyNode->SetCallbacks(createItemCb, updateRangeCb);
    lazyNode->SetTotalCount(TOTAL_COUNT);

    EXPECT_TRUE(lazyNode->GetFrameChildByIndex(INDEX_1, true, false, true));
    EXPECT_TRUE(lazyNode->GetFrameNode(INDEX_1));
}

/**
 * @tc.name: ArkoalaLazyNodeTest004
 * @tc.desc: Test ArkoalaLazyNode DumpInfo.
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
 * @tc.desc: Test ArkoalaLazyNode OnDataChange.
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
 * @tc.desc: Test ArkoalaLazyNode IsNodeInRange.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest006)
{
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    RangeType range = { INDEX_1, INDEX_9 };
    bool ret = lazyNode->IsNodeInRange(0, range);
    EXPECT_FALSE(ret);
    ret = lazyNode->IsNodeInRange(INDEX_8, range);
    EXPECT_TRUE(ret);
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
    EXPECT_FALSE(lazyNode->isActive_);
    lazyNode->SetJSViewActive(true, true);
    EXPECT_TRUE(lazyNode->isActive_);
    auto child = AceType::DynamicCast<ArkoalaLazyNode>(lazyNode->node4Index_.Get(0)->Upgrade());
    EXPECT_TRUE(child->isActive_);
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
 * @tc.name: ArkoalaLazyNodeTest009
 * @tc.desc: Test ArkoalaLazyNode DoSetActiveChildRange.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest009)
{
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    CreateChildren(lazyNode, TOTAL_COUNT);
    int32_t index_11 = 11;
    lazyNode->DoSetActiveChildRange(index_11, index_11, 0, CACHED_COUNT, false);
    EXPECT_EQ(lazyNode->node4Index_.Size(), 0);

    CreateChildren(lazyNode, TOTAL_COUNT);
    lazyNode->DoSetActiveChildRange(INDEX_1, INDEX_1, CACHED_COUNT, CACHED_COUNT, false);
    int32_t result_size = 4;
    EXPECT_EQ(lazyNode->node4Index_.Size(), result_size);
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
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    EXPECT_EQ(lazyNode->children_.size(), 0);
    CreateChildren(lazyNode, TOTAL_COUNT);
    auto ret = lazyNode->GetChildren();
    EXPECT_EQ(ret.size(), TOTAL_COUNT);

    lazyNode->children_.clear();
    ret = lazyNode->GetChildren();
    EXPECT_EQ(ret.size(), TOTAL_COUNT);
}

/**
 * @tc.name: ArkoalaLazyNodeTest012
 * @tc.desc: Test ArkoalaLazyNode ForEachL1Node.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest012)
{
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    CreateChildren(lazyNode, TOTAL_COUNT);
    std::list<RefPtr<UINode>> ret;
    lazyNode->ForEachL1Node([&ret, this](int32_t index, const RefPtr<UINode>& node) {
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

TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest014)
{
    auto lazyNode = CreateLazyForEachNode(GetNextId());
    CreateChildren(lazyNode, TOTAL_COUNT);
    lazyNode->children_.clear();
    lazyNode->RequestSyncTree();
    EXPECT_EQ(lazyNode->children_.size(), 0);
}

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

TEST_F(ArkoalaLazyNodeTest, ArkoalaLazyNodeTest016)
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
} // namespace OHOS::Ace::NG
