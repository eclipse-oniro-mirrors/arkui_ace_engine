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

#include "frameworks/core/components_ng/syntax/arkoala_for_each_node.h"
#include "core/components_ng/pattern/grid/grid_pattern.h"
#include "core/components_ng/pattern/waterflow/water_flow_pattern.h"

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

class ArkoalaForEachNodeTest : public TestNG {
public:
    RefPtr<ArkoalaForEachNode> CreateForEachNode(int32_t nodeId)
    {
        auto* stack = ViewStackProcessor::GetInstance();
        auto forEachNode = AceType::MakeRefPtr<ArkoalaForEachNode>(nodeId);
        stack->Push(forEachNode);
        return forEachNode;
    }

    RefPtr<ArkoalaForEachNode> CreateRepeatNode(int32_t nodeId)
    {
        auto* stack = ViewStackProcessor::GetInstance();
        auto repeatNode = AceType::MakeRefPtr<ArkoalaForEachNode>(nodeId, true);
        stack->Push(repeatNode);
        return repeatNode;
    }

    RefPtr<TestUINode> CreateTestUINode(int32_t nodeId)
    {
        return AceType::MakeRefPtr<TestUINode>(nodeId);
    }

    RefPtr<FrameNode> CreateTestFrameNode(int32_t nodeId)
    {
        return FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, nodeId, AceType::MakeRefPtr<Pattern>());
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
};

/**
 * @tc.name: ArkoalaForEachNodeTest001
 * @tc.desc: Test ArkoalaForEachNode Create.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaForEachNodeTest, ArkoalaForEachNodeTest001)
{
    /**
     * @tc.steps: step1. Test ForEach node tag
     */
    int32_t nodeId1 = GetNextId();
    auto forEachNode = CreateForEachNode(nodeId1);
    EXPECT_NE(forEachNode, nullptr);

    EXPECT_EQ(forEachNode->GetTag(), V2::JS_FOR_EACH_ETS_TAG);
    EXPECT_EQ(forEachNode->GetId(), nodeId1);
    EXPECT_FALSE(forEachNode->isRepeat_);

    /**
     * @tc.steps: step2. Test Repeat node tag
     */
    int32_t nodeId2 = GetNextId();
    auto repeatNode = CreateRepeatNode(nodeId2);
    EXPECT_NE(repeatNode, nullptr);

    EXPECT_EQ(repeatNode->GetTag(), V2::JS_REPEAT_ETS_TAG);
    EXPECT_EQ(repeatNode->GetId(), nodeId2);
    EXPECT_TRUE(repeatNode->isRepeat_);
}

/**
 * @tc.name: ArkoalaForEachNodeTest002
 * @tc.desc: Test ArkoalaForEachNode DumpInfo.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaForEachNodeTest, ArkoalaForEachNodeTest002)
{
    /**
     * @tc.steps: step1. Test ForEach node dump info
     * ForEach does not have dump info
     */
    auto node = CreateForEachNode(GetNextId());
    EXPECT_NE(node, nullptr);
    node->DumpInfo();

    /**
     * @tc.steps: step2. Test Repeat node dump info
     * Repeat dump info is "VirtualScroll: false"
     */
    auto repeatNode = CreateRepeatNode(GetNextId());
    EXPECT_NE(repeatNode, nullptr);
    repeatNode->DumpInfo();
    EXPECT_EQ(DumpLog::GetInstance().description_.back(), "VirtualScroll: false\n");
}

/**
 * @tc.name: SetOnMoveTest001
 * @tc.desc: Test ArkoalaForEachNode SetOnMove.
 * @tc.type: FUNC
 */
TEST_F(ArkoalaForEachNodeTest, SetOnMoveTest001)
{
    auto forEachNode = CreateForEachNode(GetNextId());
    EXPECT_NE(forEachNode, nullptr);
    auto setOnMoveCbFunc = [](int32_t from, int32_t to) {};
    forEachNode->SetOnMove(setOnMoveCbFunc);

    forEachNode->onMainTree_ = true;
    auto node = CreateTestFrameNode(GetNextId());
    forEachNode->SetParent(node);
    forEachNode->FlushUpdateAndMarkDirty();
    forEachNode->SetOnMove(setOnMoveCbFunc);
    forEachNode->SetOnMove(nullptr);
}

/**
 * @tc.name: SetAllowReuseInAnimation001
 * @tc.desc: Test ArkoalaForEachNode SetAllowReuseInAnimation stores the tri-state value
 * @tc.type: FUNC
 */
TEST_F(ArkoalaForEachNodeTest, SetAllowReuseInAnimation001)
{
    auto repeatNode = CreateRepeatNode(GetNextId());
    EXPECT_NE(repeatNode, nullptr);
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
 * @tc.name: OnAttachSyncCache001
 * @tc.desc: Attach/detach holds and releases the sync-cache-in-layout reference on
 *           Grid/WaterFlow parents for a Repeat-mode ArkoalaForEachNode
 * @tc.type: FUNC
 */
TEST_F(ArkoalaForEachNodeTest, OnAttachSyncCache001)
{
    auto gridNode = FrameNode::CreateFrameNode(V2::GRID_ETS_TAG, GetNextId(), AceType::MakeRefPtr<GridPattern>());
    auto gridPattern = gridNode->GetPattern<GridPattern>();
    ASSERT_NE(gridPattern, nullptr);
    auto repeatNode = CreateRepeatNode(GetNextId());
    repeatNode->SetAllowReuseInAnimation(false);
    gridNode->AddChild(repeatNode);
    repeatNode->OnAttachToMainTree(false);
    EXPECT_TRUE(gridPattern->IsSyncCacheLayoutEnabled());
    repeatNode->OnDetachFromMainTree(false, nullptr);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());

    auto waterFlowNode =
        FrameNode::CreateFrameNode(V2::WATERFLOW_ETS_TAG, GetNextId(), AceType::MakeRefPtr<WaterFlowPattern>());
    auto waterFlowPattern = waterFlowNode->GetPattern<WaterFlowPattern>();
    ASSERT_NE(waterFlowPattern, nullptr);
    waterFlowNode->AddChild(repeatNode);
    repeatNode->OnAttachToMainTree(false);
    EXPECT_TRUE(waterFlowPattern->IsSyncCacheLayoutEnabled());
    repeatNode->OnDetachFromMainTree(false, nullptr);
    EXPECT_FALSE(waterFlowPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());
}

/**
 * @tc.name: OnAttachSyncCache002
 * @tc.desc: A ForEach-mode ArkoalaForEachNode never enables the sync-cache-in-layout
 *           strategy, and neither does a Repeat-mode node with the option unset or true
 * @tc.type: FUNC
 */
TEST_F(ArkoalaForEachNodeTest, OnAttachSyncCache002)
{
    auto gridNode = FrameNode::CreateFrameNode(V2::GRID_ETS_TAG, GetNextId(), AceType::MakeRefPtr<GridPattern>());
    auto gridPattern = gridNode->GetPattern<GridPattern>();
    ASSERT_NE(gridPattern, nullptr);

    // ForEach mode: not qualified even with the option set
    auto forEachNode = CreateForEachNode(GetNextId());
    forEachNode->SetAllowReuseInAnimation(false);
    gridNode->AddChild(forEachNode);
    forEachNode->OnAttachToMainTree(false);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(forEachNode->syncCacheHost_.Invalid());
    forEachNode->OnDetachFromMainTree(false, nullptr);

    // Repeat mode with the option unset keeps the container default
    auto repeatNode = CreateRepeatNode(GetNextId());
    gridNode->AddChild(repeatNode);
    repeatNode->SetAllowReuseInAnimation(std::nullopt);
    repeatNode->OnAttachToMainTree(false);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());
    repeatNode->OnDetachFromMainTree(false, nullptr);

    // explicitly true also keeps the container default
    repeatNode->SetAllowReuseInAnimation(true);
    repeatNode->OnAttachToMainTree(false);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());
    repeatNode->OnDetachFromMainTree(false, nullptr);
}

/**
 * @tc.name: SetAllowReuseInAnimationAttached001
 * @tc.desc: SetAllowReuseInAnimation on an already-attached Repeat-mode ArkoalaForEachNode
 *           re-runs the parent toggle: release the sync-cache reference held for the old
 *           value, register for the new one when it now qualifies
 * @tc.type: FUNC
 */
TEST_F(ArkoalaForEachNodeTest, SetAllowReuseInAnimationAttached001)
{
    auto gridNode = FrameNode::CreateFrameNode(V2::GRID_ETS_TAG, GetNextId(), AceType::MakeRefPtr<GridPattern>());
    auto gridPattern = gridNode->GetPattern<GridPattern>();
    ASSERT_NE(gridPattern, nullptr);
    auto repeatNode = CreateRepeatNode(GetNextId());
    gridNode->AddChild(repeatNode);

    /**
     * @tc.steps: step1. Attach with the option unset: the attach-time toggle keeps the
     *             container default
     */
    repeatNode->AttachToMainTree(false);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());

    /**
     * @tc.steps: step2. The option arrives after attach and switches the parent while
     *             attached; setting the same value again is a no-op
     * @tc.expected: the sync-cache reference is registered for false, kept for a repeat
     *               set of false
     */
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_TRUE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_FALSE(repeatNode->syncCacheHost_.Invalid());
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_TRUE(gridPattern->IsSyncCacheLayoutEnabled());

    /**
     * @tc.steps: step3. Switch back to true and to false again while attached
     * @tc.expected: each transition releases / re-registers the reference
     */
    repeatNode->SetAllowReuseInAnimation(true);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_TRUE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_FALSE(repeatNode->syncCacheHost_.Invalid());

    /**
     * @tc.steps: step4. Detach releases the reference; later sets have no effect
     */
    repeatNode->DetachFromMainTree(false, false);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(repeatNode->syncCacheHost_.Invalid());
    repeatNode->SetAllowReuseInAnimation(true);
    repeatNode->SetAllowReuseInAnimation(false);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());

    /**
     * @tc.steps: step5. ForEach mode never qualifies even when the option arrives attached
     */
    auto forEachNode = CreateForEachNode(GetNextId());
    gridNode->AddChild(forEachNode);
    forEachNode->AttachToMainTree(false);
    forEachNode->SetAllowReuseInAnimation(false);
    EXPECT_FALSE(gridPattern->IsSyncCacheLayoutEnabled());
    EXPECT_TRUE(forEachNode->syncCacheHost_.Invalid());
    forEachNode->DetachFromMainTree(false, false);
}
} // namespace OHOS::Ace::NG
