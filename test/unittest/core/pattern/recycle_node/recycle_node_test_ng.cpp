/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <type_traits>

#include "gtest/gtest.h"
#include "gtest/hwext/gtest-ext.h"

#include "base/memory/ace_type.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/custom/custom_measure_layout_node.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/pattern/recycle_view/recycle_dummy_node.h"
#include "core/components_v2/inspector/inspector_constants.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
class RecycleNodeTestNG : public testing::Test {};
/**
 * @tc.name: RecycleNodeTestNg001
 * @tc.desc: destructor RecycleNode
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(RecycleNodeTestNG, RecycleNodeTestNG001, TestSize.Level1)
{
    int32_t nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    int32_t childNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    /**
     * @tc.steps: step1. Create RecycleNode width no child.
     * @tc.expected: expect not recycle any child.
     */
    auto recycleNode0 = RecycleDummyNode::CreateRecycleDummyNode(nodeId);
    EXPECT_EQ(recycleNode0->TotalChildCount(), 0);
    recycleNode0.Reset();

    /**
     * @tc.steps: step2. Create RecycleNode width a custom node child.
     * @tc.expected: expect recycle custom node.
     */
    nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto recycleNode1 = RecycleDummyNode::CreateRecycleDummyNode(nodeId);
    std::string childNodeName = "childNode";
    auto childCustomNode = CustomNode::CreateCustomNode(childNodeId, childNodeName);
    auto child = recycleNode1->GetFirstChild();
    EXPECT_EQ(child, nullptr);
    auto mockRecycleFunc = [childCustomNode](RefPtr<CustomNodeBase> node) { EXPECT_EQ(childCustomNode, node); };
    childCustomNode->SetRecycleFunction(std::move(mockRecycleFunc));
    recycleNode1->AddChild(childCustomNode);
    recycleNode1.Reset();

    /**
     * @tc.steps: step3. Create RecycleNode width a child but not custom node.
     * @tc.expected: expect not recycle any child.
     */
    nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    childNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto recycleNode2 = RecycleDummyNode::CreateRecycleDummyNode(nodeId);
    auto frameNodeChild = FrameNode::CreateFrameNode("child", childNodeId, AceType::MakeRefPtr<Pattern>());
    recycleNode2->AddChild(frameNodeChild);
    auto child2 = recycleNode2->GetFirstChild();
    auto customNode2 = AceType::DynamicCast<CustomNodeBase>(child2);
    EXPECT_EQ(customNode2, nullptr);
    recycleNode2.Reset();
}

/**
 * @tc.name: RecycleNodeTestNg002
 * @tc.desc: destructor RecycleNode with custom node that has children
 * @tc.type: FUNC
 * @tc.author:
 */
HWTEST_F(RecycleNodeTestNG, RecycleNodeTestNG002, TestSize.Level1)
{
    int32_t nodeId = ElementRegister::GetInstance()->MakeUniqueId();
    int32_t childNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    int32_t grandChildNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    
    /**
     * @tc.steps: step1. Create RecycleNode with custom node that has a child.
     * @tc.expected: expect recycle custom node and detach from main tree.
     */
    auto recycleNode = RecycleDummyNode::CreateRecycleDummyNode(nodeId);
    std::string childNodeName = "childNode";
    auto childCustomNode = CustomNode::CreateCustomNode(childNodeId, childNodeName);
    
    std::string grandChildNodeName = "grandChildNode";
    auto grandChildCustomNode = CustomNode::CreateCustomNode(grandChildNodeId, grandChildNodeName);
    
    childCustomNode->AddChild(grandChildCustomNode);
    EXPECT_EQ(childCustomNode->TotalChildCount(), 1);
    
    auto mockRecycleFunc = [childCustomNode](RefPtr<CustomNodeBase> node) { EXPECT_EQ(childCustomNode, node); };
    childCustomNode->SetRecycleFunction(std::move(mockRecycleFunc));
    
    recycleNode->AddChild(childCustomNode);
    EXPECT_EQ(recycleNode->TotalChildCount(), 1);
    
    recycleNode.Reset();
}
/**
 * @tc.name: CustomOwnerLifetime
 * @tc.desc: A custom capability keeps its node alive and preserves copy, move and weak reference semantics.
 * @tc.type: FUNC
 */
HWTEST_F(RecycleNodeTestNG, CustomOwnerLifetime, TestSize.Level1)
{
    static_assert(!std::is_base_of_v<AceType, CustomNodeBase>);
    static_assert(sizeof(RefPtr<CustomNodeBase>) == sizeof(void*));
    static_assert(sizeof(WeakPtr<CustomNodeBase>) == 2 * sizeof(void*));
    int32_t destroyed = 0;
    auto node = AceType::MakeRefPtr<CustomNode>(ElementRegister::GetInstance()->MakeUniqueId(), "owner");
    node->SetDestroyFunction([&destroyed]() { ++destroyed; });
    RefPtr<CustomNodeBase> capability = node;
    WeakPtr<CustomNodeBase> weak = capability;
    EXPECT_TRUE(AceType::InstanceOf<CustomNodeBase>(node));
    EXPECT_EQ(AceType::RawPtr(capability), static_cast<CustomNodeBase*>(AceType::RawPtr(node)));
    EXPECT_EQ(node->RefCount(), 2);
    {
        auto claimed = AceType::Claim(AceType::RawPtr(capability));
        auto copy = claimed;
        RefPtr<CustomNodeBase> moved = std::move(copy);
        EXPECT_EQ(copy, nullptr);
        EXPECT_EQ(moved, node);
        RefPtr<UINode> owner = std::move(moved);
        EXPECT_EQ(moved, nullptr);
        EXPECT_EQ(owner, node);
        EXPECT_EQ(node->RefCount(), 4);
    }
    EXPECT_EQ(node->RefCount(), 2);
    node.Reset();
    EXPECT_EQ(destroyed, 0);
    EXPECT_NE(weak.Upgrade(), nullptr);
    capability.Reset();
    EXPECT_EQ(destroyed, 1);
    EXPECT_EQ(weak.Upgrade(), nullptr);
    EXPECT_EQ(AceType::DynamicCast<UINode>(weak).Upgrade(), nullptr);
}

/**
 * @tc.name: CustomAndLayoutShareOwner
 * @tc.desc: Layout and custom capabilities share one owner, including cross-casts and final release.
 * @tc.type: FUNC
 */
HWTEST_F(RecycleNodeTestNG, CustomAndLayoutShareOwner, TestSize.Level1)
{
    int32_t destroyed = 0;
    auto node = AceType::MakeRefPtr<CustomMeasureLayoutNode>(
        ElementRegister::GetInstance()->MakeUniqueId(), "dual-capability");
    node->SetDestroyFunction([&destroyed]() { ++destroyed; });
    RefPtr<CustomNodeBase> custom = node;
    RefPtr<LayoutWrapper> layout = node;
    WeakPtr<CustomNodeBase> weakCustom = custom;
    WeakPtr<LayoutWrapper> weakLayout = layout;
    EXPECT_EQ(custom, layout);
    EXPECT_EQ(weakCustom, weakLayout);
    EXPECT_EQ(WeakPtr<CustomNodeBase>::Hash()(weakCustom), WeakPtr<LayoutWrapper>::Hash()(weakLayout));
    EXPECT_EQ(AceType::RawPtr(custom), static_cast<CustomNodeBase*>(AceType::RawPtr(node)));
    EXPECT_EQ(AceType::RawPtr(layout), static_cast<LayoutWrapper*>(AceType::RawPtr(node)));
    EXPECT_EQ(AceType::DynamicCast<LayoutWrapper>(custom), layout);
    EXPECT_EQ(AceType::DynamicCast<CustomNodeBase>(layout), custom);
    {
        RefPtr<const CustomNodeBase> constCustom = custom;
        EXPECT_EQ(RefPtrTraits<const CustomNodeBase>::GetOwner(constCustom.GetRawPtr()), AceType::RawPtr(node));
        auto constLayout = AceType::DynamicCast<const LayoutWrapper>(std::move(constCustom));
        EXPECT_EQ(constCustom, nullptr);
        EXPECT_EQ(RefPtrTraits<const LayoutWrapper>::GetOwner(constLayout.GetRawPtr()), AceType::RawPtr(node));
        auto failed = AceType::DynamicCast<CustomNode>(std::move(custom));
        EXPECT_EQ(failed, nullptr);
        EXPECT_NE(custom, nullptr);
        EXPECT_EQ(node->RefCount(), 4);
    }
    auto transferred = AceType::DynamicCast<LayoutWrapper>(std::move(custom));
    EXPECT_EQ(custom, nullptr);
    EXPECT_EQ(node->RefCount(), 3);
    node.Reset();
    layout.Reset();
    EXPECT_EQ(destroyed, 0);
    EXPECT_NE(weakCustom.Upgrade(), nullptr);
    transferred.Reset();
    EXPECT_EQ(destroyed, 1);
    EXPECT_EQ(weakCustom.Upgrade(), nullptr);
    EXPECT_EQ(weakLayout.Upgrade(), nullptr);
}

/**
 * @tc.name: CustomRecycleCallbackRetainsOwner
 * @tc.desc: The original recycle callback can retain a capability and release the node after reuse.
 * @tc.type: FUNC
 */
HWTEST_F(RecycleNodeTestNG, CustomRecycleCallbackRetainsOwner, TestSize.Level1)
{
    int32_t destroyed = 0;
    int32_t recycled = 0;
    int32_t rendered = 0;
    RefPtr<CustomNodeBase> cached;
    auto node = AceType::MakeRefPtr<CustomNode>(ElementRegister::GetInstance()->MakeUniqueId(), "recycle");
    node->SetDestroyFunction([&destroyed]() { ++destroyed; });
    WeakPtr<CustomNodeBase> weak = node;
    node->SetRecycleFunction([&cached, &recycled](RefPtr<CustomNodeBase> capability) {
        ++recycled;
        cached = std::move(capability);
    });
    node->FireRecycleSelf();
    EXPECT_EQ(recycled, 1);
    EXPECT_EQ(cached, node);
    node.Reset();
    ASSERT_NE(cached, nullptr);
    EXPECT_EQ(destroyed, 0);
    cached->SetRecycleRenderFunc([&rendered]() { ++rendered; });
    cached->FireRecycleRenderFunc();
    cached->FireRecycleRenderFunc();
    EXPECT_EQ(rendered, 1);
    EXPECT_NE(weak.Upgrade(), nullptr);
    cached.Reset();
    EXPECT_EQ(destroyed, 1);
    EXPECT_EQ(weak.Upgrade(), nullptr);
}

} // namespace OHOS::Ace::NG
