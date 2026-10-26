/*
 * Copyright (c) 2023 iSoftStone Information Technology (Group) Co.,Ltd.
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

#include <memory>
#include <optional>
#include <type_traits>

#include "gtest/gtest.h"

#define protected public
#define private public
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components/common/layout/grid_layout_info.h"
#include "core/components/common/properties/alignment.h"
#include "core/components_ng/animation/geometry_transition.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/property/grid_property.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/syntax/lazy_for_each_model.h"
#include "core/components_ng/syntax/lazy_layout_wrapper_builder.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
static_assert(!std::is_base_of_v<AceType, LayoutWrapper>);
static_assert(std::is_base_of_v<TypeInfoBase, LayoutWrapper>);
static_assert(sizeof(RefPtr<LayoutWrapper>) == sizeof(LayoutWrapper*));
static_assert(sizeof(WeakPtr<LayoutWrapper>) == sizeof(WeakPtr<FrameNode>));

constexpr int32_t NODE_ID_0 = 0;
constexpr int32_t NODE_ID_1 = 1;
constexpr int32_t NODE_ID_2 = 2;
constexpr int32_t NODE_ID_3 = 3;
const std::string ROW_FRAME_NODE = "rowFrameNode";
// const std::string FIRST_FRAME_NODE = "TabContent";
const std::string FIRST_CHILD_FRAME_NODE = "firstChildFrameNode";
const std::string SECOND_CHILD_FRAME_NODE = "secondChildFrameNode";
const std::string THIRD_CHILD_FRAME_NODE = "thirdChildFrameNode";
const int32_t INDEX = 1;

std::pair<RefPtr<FrameNode>, RefPtr<LayoutWrapperNode>> CreateNodeAndWrapper(const std::string& tag, int32_t nodeId)
{
    auto node = FrameNode::CreateFrameNode(tag, nodeId, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(node, geometryNode, node->GetLayoutProperty());

    return std::make_pair(node, layoutWrapper);
}

RefPtr<LazyLayoutWrapperBuilder> CreateWrapperBuilder()
{
    RefPtr<LazyForEachActuator> actuator = AceType::MakeRefPtr<LazyForEachActuator>();
    auto builder = AceType::DynamicCast<LazyForEachBuilder>(actuator);
    RefPtr<LazyForEachNode> host = AceType::MakeRefPtr<LazyForEachNode>(INDEX, builder);
    RefPtr<LazyLayoutWrapperBuilder> wrapperBuilder = AceType::MakeRefPtr<LazyLayoutWrapperBuilder>(builder, host);
    return wrapperBuilder;
}
} // namespace

class LayoutWrapperNodeTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
};

void LayoutWrapperNodeTestNg::SetUpTestSuite()
{
    MockPipelineContext::SetUp();
}

void LayoutWrapperNodeTestNg::TearDownTestSuite()
{
    MockPipelineContext::TearDown();
}

/**
 * @tc.name: LayoutWrapperNodeTestNg001
 * @tc.desc: Test Update.
 * @tc.type: FUNC
 */
HWTEST_F(LayoutWrapperNodeTestNg, LayoutWrapperNodeTestNg001, TestSize.Level0)
{
    auto [node, wrapper] = CreateNodeAndWrapper(ROW_FRAME_NODE, NODE_ID_0);
    auto [newNode, newWrapper] = CreateNodeAndWrapper(FIRST_CHILD_FRAME_NODE, NODE_ID_1);
    wrapper->Update(newNode, newWrapper->GetGeometryNode(), newNode->GetLayoutProperty());
    EXPECT_EQ(wrapper->GetHostNode()->GetTag(), FIRST_CHILD_FRAME_NODE);
}

/**
 * @tc.name: LayoutWrapperNodeTestNg002
 * @tc.desc: Test AppendChild.
 * @tc.type: FUNC
 */
HWTEST_F(LayoutWrapperNodeTestNg, LayoutWrapperNodeTestNg002, TestSize.Level0)
{
    auto [node, wrapper] = CreateNodeAndWrapper(ROW_FRAME_NODE, NODE_ID_0);
    auto [child, childWrapper] = CreateNodeAndWrapper(FIRST_CHILD_FRAME_NODE, NODE_ID_1);

    wrapper->AppendChild(childWrapper, true);
    EXPECT_EQ(wrapper->overlayChild_->GetHostNode()->GetTag(), FIRST_CHILD_FRAME_NODE);
}

/**
 * @tc.name: LayoutWrapperNodeTestNg003
 * @tc.desc: Test GetAllChildrenWithBuild.
 * @tc.type: FUNC
 */
HWTEST_F(LayoutWrapperNodeTestNg, LayoutWrapperNodeTestNg003, TestSize.Level0)
{
    auto [node, wrapper] = CreateNodeAndWrapper(ROW_FRAME_NODE, NODE_ID_0);
    auto [child, childWrapper] = CreateNodeAndWrapper(FIRST_CHILD_FRAME_NODE, NODE_ID_1);
    auto [child2, childWrapper2] = CreateNodeAndWrapper(SECOND_CHILD_FRAME_NODE, NODE_ID_2);
    auto [child3, childWrapper3] = CreateNodeAndWrapper(THIRD_CHILD_FRAME_NODE, NODE_ID_3);

    wrapper->AppendChild(childWrapper, false);
    wrapper->AppendChild(childWrapper2, false);
    wrapper->AppendChild(childWrapper3, true);
    wrapper->GetAllChildrenWithBuild(true);

    EXPECT_TRUE(wrapper->overlayChild_->IsActive());
}

/**
 * @tc.name: LayoutWrapperNodeTestNg004
 * @tc.desc: Test Layout.
 * @tc.type: FUNC
 */
HWTEST_F(LayoutWrapperNodeTestNg, LayoutWrapperNodeTestNg004, TestSize.Level0)
{
    auto [node, wrapper] = CreateNodeAndWrapper(ROW_FRAME_NODE, NODE_ID_0);

    auto layoutAlgorithmT = AceType::MakeRefPtr<LayoutAlgorithm>();
    auto layoutAlgorithm = AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithmT, false, false);
    wrapper->SetLayoutAlgorithm(layoutAlgorithm);

    wrapper->Layout();
    EXPECT_TRUE(wrapper->layoutProperty_->GetLayoutConstraint());

    LayoutConstraintF layoutConstraint;
    layoutConstraint.parentIdealSize = OptionalSizeF(768, 1024);
    wrapper->layoutProperty_->UpdateLayoutConstraint(layoutConstraint);
    EXPECT_EQ(wrapper->layoutProperty_->layoutConstraint_->parentIdealSize, OptionalSizeF(768, 1024));
}

/**
 * @tc.name: LayoutWrapperNodeTestNg005
 * @tc.desc: Test SetLongPredictTask.
 * @tc.type: FUNC
 */
HWTEST_F(LayoutWrapperNodeTestNg, LayoutWrapperNodeTestNg005, TestSize.Level0)
{
    auto [node, wrapper] = CreateNodeAndWrapper(ROW_FRAME_NODE, NODE_ID_0);
    wrapper->layoutWrapperBuilder_ = CreateWrapperBuilder();
    EXPECT_TRUE(wrapper->layoutWrapperBuilder_);

    wrapper->SetLongPredictTask();
    EXPECT_TRUE(wrapper->layoutWrapperBuilder_->useLongPredictTask_);
}

/**
 * @tc.name: LayoutWrapperNodeTestNg007
 * @tc.desc: Test LayoutOverlay.
 * @tc.type: FUNC
 */
HWTEST_F(LayoutWrapperNodeTestNg, LayoutWrapperNodeTestNg007, TestSize.Level0)
{
    auto [node, wrapper] = CreateNodeAndWrapper(ROW_FRAME_NODE, NODE_ID_0);
    auto frameSize = SizeF(100, 200);
    wrapper->geometryNode_->SetFrameSize(frameSize);

    auto [child, childWrapper] = CreateNodeAndWrapper(FIRST_CHILD_FRAME_NODE, NODE_ID_1);
    auto frameSizeChild = SizeF(50, 100);
    childWrapper->geometryNode_->SetFrameSize(frameSizeChild);
    childWrapper->layoutProperty_->overlayOffsetX_ = Dimension::FromString("1px");
    childWrapper->layoutProperty_->overlayOffsetY_ = Dimension::FromString("1px");

    wrapper->AppendChild(childWrapper, true);
    wrapper->LayoutOverlay();
    EXPECT_EQ(childWrapper->GetGeometryNode()->GetFrameRect().GetOffset(), OffsetF(1, 1));
}

/**
 * @tc.name: LayoutWrapperNodeTestNg08
 * @tc.desc: Test GetAllChildrenWithBuild .
 * @tc.type: FUNC
 */
HWTEST_F(LayoutWrapperNodeTestNg, LayoutWrapperNodeTestNg08, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create frameNode.
     */
    RefPtr<FrameNode> frameNode = FrameNode::CreateFrameNode(ROW_FRAME_NODE, NODE_ID_0, AceType::MakeRefPtr<Pattern>());

    /**
     * @tc.steps: step2. Create geometryNode.
     */
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();

    /**
     * @tc.steps: step3. Create layoutWrapper.
     */
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, frameNode->GetLayoutProperty());

    /**
     * @tc.steps: step4. Add Children.
     * @tc.expected: Children add successful.
     */
    RefPtr<LayoutWrapperNode> layoutWrapper1 =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, frameNode->GetLayoutProperty());
    layoutWrapper->AppendChild(layoutWrapper1, false);
    EXPECT_EQ(layoutWrapper->children_.size(), 1);

    RefPtr<LayoutWrapperNode> layoutWrapper2 =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, frameNode->GetLayoutProperty());
    layoutWrapper->AppendChild(layoutWrapper2, false);
    EXPECT_EQ(layoutWrapper->children_.size(), 2);

    /**
     * @tc.steps: step5. set overlayChild_.
     * @tc.expected: overlayChild_ is not null.
     */
    RefPtr<LayoutWrapperNode> layoutWrapper3 =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, frameNode->GetLayoutProperty());
    layoutWrapper->AppendChild(layoutWrapper3, true);
    EXPECT_EQ(layoutWrapper->children_.size(), 2);
    EXPECT_TRUE(layoutWrapper->overlayChild_);

    /**
     * @tc.steps: step6. GetAllChildrenWithBuild.
     * @tc.expected: cachedList_ is same with children.
     */
    layoutWrapper->overlayChild_->isActive_ = true;
    layoutWrapper->GetAllChildrenWithBuild();
    EXPECT_EQ(layoutWrapper->cachedList_.size(), layoutWrapper->children_.size());
}

/**
 * @tc.name: LayoutWrapperNodeTestNg09
 * @tc.desc: Test Measure .
 * @tc.type: FUNC
 */
HWTEST_F(LayoutWrapperNodeTestNg, LayoutWrapperNodeTestNg09, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create frameNode.
     */
    RefPtr<FrameNode> frameNode = FrameNode::CreateFrameNode(ROW_FRAME_NODE, NODE_ID_0, AceType::MakeRefPtr<Pattern>());
    frameNode->layoutPriority_ = 1;

    /**
     * @tc.steps: step2. Create geometryNode.
     */
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();

    /**
     * @tc.steps: step3. Create layoutWrapper.
     */
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, frameNode->GetLayoutProperty());

    /**
     * @tc.steps: step4. Create LayoutAlgorithmWrapper.
     * @tc.expected: layoutAlgorithm is not null.
     */
    auto layoutAlgorithmT = AceType::MakeRefPtr<LayoutAlgorithm>();
    auto layoutAlgorithm = AceType::MakeRefPtr<LayoutAlgorithmWrapper>(layoutAlgorithmT, false, false);
    layoutWrapper->layoutAlgorithm_ = layoutAlgorithm;
    layoutAlgorithm->skipMeasure_ = false;

    /**
     * @tc.steps: step5. Create GeometryTransition.
     */
    RefPtr<FrameNode> frameNode1 =
        FrameNode::CreateFrameNode(ROW_FRAME_NODE, NODE_ID_1, AceType::MakeRefPtr<Pattern>());
    RefPtr<GeometryTransition> geometryTransition = AceType::MakeRefPtr<GeometryTransition>("test", true, true);
    geometryTransition->state_ = GeometryTransition::State::ACTIVE;
    geometryTransition->inNode_ = frameNode;
    geometryTransition->outNode_ = frameNode1;
    geometryTransition->hasInAnim_ = true;
    layoutWrapper->layoutProperty_->geometryTransition_ = geometryTransition;

    /**
     * @tc.steps: step6. Create LayoutConstraintF and Call Measure.
     * @tc.expected: IsRunning and IsNodeInAndActive is true.
     */
    layoutWrapper->SetRootMeasureNode();
    EXPECT_TRUE(layoutWrapper->IsRootMeasureNode());
    LayoutConstraintF parentLayoutConstraint;
    layoutWrapper->Measure(parentLayoutConstraint);
    EXPECT_TRUE(geometryTransition->IsRunning(layoutWrapper->GetHostNode()));
    EXPECT_TRUE(geometryTransition->IsNodeInAndActive(layoutWrapper->GetHostNode()));

    /**
     * @tc.steps: step7. set overlayChild_.
     * @tc.expected: IsRunning and IsNodeInAndActive is true.
     */
    RefPtr<LayoutWrapperNode> layoutWrapper1 =
        AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, frameNode->GetLayoutProperty());
    layoutWrapper->overlayChild_ = layoutWrapper1;
    layoutWrapper->Measure(parentLayoutConstraint);
    EXPECT_TRUE(geometryTransition->IsRunning(layoutWrapper->GetHostNode()));
    EXPECT_TRUE(geometryTransition->IsNodeInAndActive(layoutWrapper->GetHostNode()));
}
/**
 * @tc.name: LayoutWrapperRef_001
 * @tc.desc: The original smart-pointer API retains the real owner and adjusts the capability address.
 * @tc.type: FUNC
 */
HWTEST_F(LayoutWrapperNodeTestNg, LayoutWrapperRef_001, TestSize.Level1)
{
    auto [node, wrapper] = CreateNodeAndWrapper(ROW_FRAME_NODE, NODE_ID_0);
    WeakPtr<FrameNode> weak(node);
    auto* rawNode = node.GetRawPtr();
    auto count = node->RefCount();
    RefPtr<LayoutWrapper> view(node);
    EXPECT_EQ(node->RefCount(), count + 1);
    const RefPtr<LayoutWrapper>& constView = view;
    EXPECT_EQ(AceType::RawPtr(constView), static_cast<LayoutWrapper*>(rawNode));
    EXPECT_EQ(constView->GetLayoutOwner(), static_cast<AceType*>(rawNode));
    EXPECT_EQ(view, node);
    EXPECT_EQ(node, view);
    EXPECT_EQ(view, rawNode);
    EXPECT_EQ(node, view.GetRawPtr());
    EXPECT_EQ(AceType::DynamicCast<FrameNode>(view), node);
    EXPECT_EQ(AceType::DynamicCast<LayoutWrapper>(node), view);
    EXPECT_EQ(AceType::DynamicCast<LayoutWrapperNode>(view), nullptr);
    EXPECT_TRUE(AceType::InstanceOf<FrameNode>(view));
    auto claimed = AceType::Claim(view.GetRawPtr());
    EXPECT_EQ(view, claimed);
    RefPtr<AceType> owner(view);
    EXPECT_EQ(owner, node);
    owner.Reset();
    node.Reset();
    EXPECT_NE(weak.Upgrade(), nullptr);
    view.Reset();
    EXPECT_NE(weak.Upgrade(), nullptr);
    claimed.Reset();
    EXPECT_EQ(weak.Upgrade(), nullptr);
}

/**
 * @tc.name: LayoutWrapperRef_002
 * @tc.desc: A standalone wrapper outlives its host; weak views expire with the wrapper rather than the host.
 * @tc.type: FUNC
 */
HWTEST_F(LayoutWrapperNodeTestNg, LayoutWrapperRef_002, TestSize.Level1)
{
    auto [node, wrapper] = CreateNodeAndWrapper(ROW_FRAME_NODE, NODE_ID_0);
    WeakPtr<FrameNode> weakHost(node);
    WeakPtr<LayoutWrapperNode> weakWrapper(wrapper);
    RefPtr<LayoutWrapper> view(wrapper);
    WeakPtr<LayoutWrapper> weakView(view);
    auto claimedWeak = AceType::WeakClaim(view.GetRawPtr());
    WeakPtr<AceType> weakOwner(view);
    auto weakOwnerFromView = AceType::DynamicCast<AceType>(weakView);
    EXPECT_EQ(weakOwner.Upgrade(), wrapper);
    EXPECT_EQ(weakOwnerFromView.Upgrade(), wrapper);
    EXPECT_EQ(view.GetRawPtr(), static_cast<LayoutWrapper*>(wrapper.GetRawPtr()));
    EXPECT_EQ(AceType::DynamicCast<FrameNode>(view), nullptr);
    EXPECT_EQ(AceType::DynamicCast<LayoutWrapperNode>(view), wrapper);
    EXPECT_EQ(weakView, weakWrapper);
    EXPECT_EQ(weakView, wrapper);
    EXPECT_EQ(wrapper, weakView);
    EXPECT_EQ(view, weakWrapper);
    EXPECT_EQ(WeakPtr<LayoutWrapper>::Hash()(weakView), WeakPtr<LayoutWrapperNode>::Hash()(weakWrapper));
    node.Reset();
    EXPECT_TRUE(weakHost.Invalid());
    EXPECT_EQ(view->GetHostNode(), nullptr);
    wrapper.Reset();
    auto upgraded = weakView.Upgrade();
    EXPECT_EQ(upgraded, view);
    EXPECT_EQ(claimedWeak.Upgrade(), view);
    view.Reset();
    EXPECT_FALSE(weakView.Invalid());
    upgraded.Reset();
    EXPECT_TRUE(weakView.Invalid());
    EXPECT_EQ(weakView.Upgrade(), nullptr);
    EXPECT_EQ(claimedWeak.Upgrade(), nullptr);
    EXPECT_EQ(weakWrapper.Upgrade(), nullptr);
    auto expiredOwner = AceType::DynamicCast<AceType>(weakView);
    EXPECT_TRUE(expiredOwner.Invalid());
    EXPECT_EQ(expiredOwner.Upgrade(), nullptr);
    auto expiredCopy = weakView;
    EXPECT_TRUE(expiredCopy.Invalid());
    EXPECT_EQ(expiredCopy.Upgrade(), nullptr);
}

/**
 * @tc.name: LayoutWrapperRef_003
 * @tc.desc: Copy/move, ordering and dynamic transfer preserve the owner's reference count and identity.
 * @tc.type: FUNC
 */
HWTEST_F(LayoutWrapperNodeTestNg, LayoutWrapperRef_003, TestSize.Level1)
{
    auto [node, wrapper] = CreateNodeAndWrapper(ROW_FRAME_NODE, NODE_ID_0);
    RefPtr<LayoutWrapper> original(wrapper);
    auto count = wrapper->RefCount();
    RefPtr<LayoutWrapper> copied(original);
    EXPECT_EQ(wrapper->RefCount(), count + 1);
    RefPtr<LayoutWrapper> moved(std::move(original));
    EXPECT_EQ(original, nullptr);
    EXPECT_EQ(wrapper->RefCount(), count + 1);
    EXPECT_EQ(copied, moved);
    RefPtr<LayoutWrapper> nodeView(node);
    EXPECT_EQ(nodeView < copied, node < wrapper);
    EXPECT_EQ(copied < nodeView, wrapper < node);
    EXPECT_FALSE(copied < wrapper);
    EXPECT_FALSE(wrapper < copied);
    nodeView.Swap(moved);
    EXPECT_EQ(moved, node);
    nodeView.Reset();
    count = node->RefCount();
    auto failed = AceType::DynamicCast<LayoutWrapperNode>(std::move(moved));
    EXPECT_EQ(failed, nullptr);
    EXPECT_EQ(moved, node);
    EXPECT_EQ(node->RefCount(), count);
    auto transferred = AceType::DynamicCast<FrameNode>(std::move(moved));
    EXPECT_EQ(moved, nullptr);
    EXPECT_EQ(transferred, node);
    EXPECT_EQ(node->RefCount(), count);
    RefPtr<LayoutWrapper> ownerView(std::move(transferred));
    EXPECT_EQ(transferred, nullptr);
    EXPECT_EQ(node->RefCount(), count);
    RefPtr<AceType> owner(std::move(ownerView));
    EXPECT_EQ(ownerView, nullptr);
    EXPECT_EQ(owner, node);
    EXPECT_EQ(node->RefCount(), count);
    copied = nullptr;
    EXPECT_FALSE(copied);
    EXPECT_EQ(AceType::Claim(static_cast<LayoutWrapper*>(nullptr)), nullptr);
    WeakPtr<LayoutWrapper> empty;
    EXPECT_TRUE(empty.Invalid());
    EXPECT_EQ(empty.Upgrade(), nullptr);
}

} // namespace OHOS::Ace::NG
