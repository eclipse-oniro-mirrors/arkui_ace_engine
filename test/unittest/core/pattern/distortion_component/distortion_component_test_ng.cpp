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

#include "gtest/gtest.h"

#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/base/geometry_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/pattern/distortion_component/distortion_component_layout_algorithm.h"
#include "core/components_ng/pattern/distortion_component/distortion_component_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/element_register.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr float FRAME_WIDTH = 100.0f;
constexpr float FRAME_HEIGHT = 200.0f;
constexpr float ZERO = 0.0f;
} // namespace

class DistortionComponentTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void DistortionComponentTestNg::SetUpTestCase()
{
    MockPipelineContext::SetUp();
}

void DistortionComponentTestNg::TearDownTestCase()
{
    MockPipelineContext::TearDown();
}

void DistortionComponentTestNg::SetUp()
{
    ViewStackProcessor::GetInstance()->ClearStack();
}

void DistortionComponentTestNg::TearDown()
{
    ViewStackProcessor::GetInstance()->ClearStack();
}

RefPtr<FrameNode> CreateDistortionFrameNode()
{
    auto pattern = AceType::MakeRefPtr<DistortionComponentPattern>();
    return FrameNode::CreateFrameNode(V2::DISTORTION_COMPONENT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), pattern);
}

/**
 * @tc.name: DistortionComponentCreateContentNode001
 * @tc.desc: Test CreateContentNode creates once and skips when already created.
 * @tc.type: FUNC
 */
HWTEST_F(DistortionComponentTestNg, DistortionComponentCreateContentNode001, TestSize.Level1)
{
    auto host = CreateDistortionFrameNode();
    ASSERT_NE(host, nullptr);
    auto pattern = host->GetPattern<DistortionComponentPattern>();
    ASSERT_NE(pattern, nullptr);

    // first create, content node added to host
    pattern->CreateContentNode();
    EXPECT_NE(pattern->GetContentNode(), nullptr);
    EXPECT_EQ(host->GetChildren().size(), 1);

    // second create: contentNode_ already created, early return
    pattern->CreateContentNode();
    EXPECT_EQ(host->GetChildren().size(), 1);
}

/**
 * @tc.name: DistortionComponentCreateContentNode002
 * @tc.desc: Test CreateContentNode with null host.
 * @tc.type: FUNC
 */
HWTEST_F(DistortionComponentTestNg, DistortionComponentCreateContentNode002, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<DistortionComponentPattern>();
    EXPECT_EQ(pattern->GetHost(), nullptr);
    // host is null, CHECK_NULL_VOID branch
    pattern->CreateContentNode();
    EXPECT_EQ(pattern->GetContentNode(), nullptr);
}

/**
 * @tc.name: DistortionComponentUpdateDistortion001
 * @tc.desc: Test SetDistortion and UpdateDistortion branches.
 * @tc.type: FUNC
 */
HWTEST_F(DistortionComponentTestNg, DistortionComponentUpdateDistortion001, TestSize.Level1)
{
    // detached pattern: host null, CHECK_NULL_VOID branch
    auto detachedPattern = AceType::MakeRefPtr<DistortionComponentPattern>();
    detachedPattern->UpdateDistortion();

    auto host = CreateDistortionFrameNode();
    ASSERT_NE(host, nullptr);
    auto pattern = host->GetPattern<DistortionComponentPattern>();
    ASSERT_NE(pattern, nullptr);

    // content node not created yet: contentNode_ null branch
    pattern->UpdateDistortion();

    // with content node: apply distortion to host and content
    pattern->CreateContentNode();
    DistortionParam param;
    param.luCorner = VectorF(0.1f, 0.2f);
    pattern->SetDistortion(param);
    EXPECT_FLOAT_EQ(pattern->GetDistortion().luCorner.x, 0.1f);
    EXPECT_FLOAT_EQ(pattern->GetDistortion().luCorner.y, 0.2f);
}

/**
 * @tc.name: DistortionComponentMeasure001
 * @tc.desc: Test Measure null wrapper and empty children branches.
 * @tc.type: FUNC
 */
HWTEST_F(DistortionComponentTestNg, DistortionComponentMeasure001, TestSize.Level1)
{
    DistortionComponentLayoutAlgorithm algorithm;
    // null layoutWrapper
    algorithm.Measure(nullptr);

    auto host = CreateDistortionFrameNode();
    ASSERT_NE(host, nullptr);
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(host, geometryNode, host->GetLayoutProperty());
    // no children: children.empty branch
    algorithm.Measure(AceType::RawPtr(layoutWrapper));
    EXPECT_FLOAT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize().Width(), ZERO);
    EXPECT_FLOAT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize().Height(), ZERO);
}

/**
 * @tc.name: DistortionComponentMeasure002
 * @tc.desc: Test Measure without layout constraint and with full child hierarchy.
 * @tc.type: FUNC
 */
HWTEST_F(DistortionComponentTestNg, DistortionComponentMeasure002, TestSize.Level1)
{
    DistortionComponentLayoutAlgorithm algorithm;
    auto host = CreateDistortionFrameNode();
    ASSERT_NE(host, nullptr);
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(host, geometryNode, host->GetLayoutProperty());

    auto childNode = FrameNode::CreateFrameNode(V2::DISTORTION_CONTENT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(childNode, nullptr);
    // NOTE: LayoutWrapperNode::Build() only collects from wrapper-level AppendChild children
    // (or lazy builder); FrameNode::AddChild on a detached host is invisible to the wrapper.
    RefPtr<LayoutWrapperNode> childWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        childNode, childNode->GetGeometryNode(), childNode->GetLayoutProperty());
    layoutWrapper->AppendChild(childWrapper, false);

    // layoutConstraint_ has no value: CHECK_NULL_VOID branch
    algorithm.Measure(AceType::RawPtr(layoutWrapper));

    // set constraint without selfIdealSize: value_or default branch
    // NOTE: production sets layoutConstraint_ and contentConstraint_ together;
    // CreateChildConstraint() dereferences contentConstraint_.value()
    LayoutConstraintF constraint;
    constraint.minSize = SizeF(10, 10);
    constraint.maxSize = SizeF(500, 600);
    host->GetLayoutProperty()->layoutConstraint_ = constraint;
    host->GetLayoutProperty()->contentConstraint_ = constraint;
    algorithm.Measure(AceType::RawPtr(layoutWrapper));

    // set selfIdealSize: value_or takes ideal size branch, frame applied on wrapper geometry
    constraint.selfIdealSize.SetWidth(FRAME_WIDTH);
    constraint.selfIdealSize.SetHeight(FRAME_HEIGHT);
    host->GetLayoutProperty()->layoutConstraint_ = constraint;
    host->GetLayoutProperty()->contentConstraint_ = constraint;
    algorithm.Measure(AceType::RawPtr(layoutWrapper));
    EXPECT_FLOAT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize().Width(), FRAME_WIDTH);
    EXPECT_FLOAT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize().Height(), FRAME_HEIGHT);

    // grandchild with frame size: contentChildren not empty and front valid branch
    auto grandChild = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(grandChild, nullptr);
    grandChild->GetGeometryNode()->SetFrameSize(SizeF(50, 60));
    RefPtr<LayoutWrapperNode> grandWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        grandChild, grandChild->GetGeometryNode(), grandChild->GetLayoutProperty());
    childWrapper->AppendChild(grandWrapper, false);
    // clear selfIdealSize: frame falls back to content child size (50x60), deterministic
    constraint.selfIdealSize = OptionalSizeF();
    host->GetLayoutProperty()->layoutConstraint_ = constraint;
    host->GetLayoutProperty()->contentConstraint_ = constraint;
    algorithm.Measure(AceType::RawPtr(layoutWrapper));
    EXPECT_FLOAT_EQ(childNode->GetGeometryNode()->GetFrameSize().Width(), 50.0f);
    EXPECT_FLOAT_EQ(childNode->GetGeometryNode()->GetFrameSize().Height(), 60.0f);
    EXPECT_FLOAT_EQ(layoutWrapper->GetGeometryNode()->GetFrameSize().Width(), 50.0f);
    auto builtChildren = layoutWrapper->GetAllChildrenWithBuild();
    ASSERT_EQ(builtChildren.size(), 1);
}

/**
 * @tc.name: DistortionComponentLayout001
 * @tc.desc: Test Layout null wrapper and normal child layout.
 * @tc.type: FUNC
 */
HWTEST_F(DistortionComponentTestNg, DistortionComponentLayout001, TestSize.Level1)
{
    DistortionComponentLayoutAlgorithm algorithm;
    // null layoutWrapper
    algorithm.Layout(nullptr);

    auto host = CreateDistortionFrameNode();
    ASSERT_NE(host, nullptr);
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(host, geometryNode, host->GetLayoutProperty());

    auto childNode = FrameNode::CreateFrameNode(V2::DISTORTION_CONTENT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(childNode, nullptr);
    RefPtr<LayoutWrapperNode> childWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        childNode, childNode->GetGeometryNode(), childNode->GetLayoutProperty());
    layoutWrapper->AppendChild(childWrapper, false);
    algorithm.Layout(AceType::RawPtr(layoutWrapper));
    // NOTE: wrapper-level children must be attached via AppendChild; see Measure002 note.
    auto builtChildren = layoutWrapper->GetAllChildrenWithBuild();
    ASSERT_EQ(builtChildren.size(), 1);
    EXPECT_FLOAT_EQ(builtChildren.front()->GetGeometryNode()->GetMarginFrameOffset().GetX(), ZERO);
}

/**
 * @tc.name: DistortionComponentOnDirtyLayoutWrapperSwap001
 * @tc.desc: Test OnDirtyLayoutWrapperSwap all branches.
 * @tc.type: FUNC
 */
HWTEST_F(DistortionComponentTestNg, DistortionComponentOnDirtyLayoutWrapperSwap001, TestSize.Level1)
{
    auto host = CreateDistortionFrameNode();
    ASSERT_NE(host, nullptr);
    auto pattern = host->GetPattern<DistortionComponentPattern>();
    ASSERT_NE(pattern, nullptr);
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    RefPtr<LayoutWrapperNode> layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(host, geometryNode, host->GetLayoutProperty());

    // SkipMeasureContent() dereferences layoutAlgorithm_, provide a valid one
    auto layoutAlgorithm = host->GetLayoutAlgorithm();
    if (layoutAlgorithm == nullptr) {
        layoutAlgorithm = AceType::MakeRefPtr<LayoutAlgorithmWrapper>(
            AceType::MakeRefPtr<DistortionComponentLayoutAlgorithm>());
    }
    layoutWrapper->SetLayoutAlgorithm(layoutAlgorithm);

    // skipMeasure true: return false
    EXPECT_FALSE(pattern->OnDirtyLayoutWrapperSwap(layoutWrapper, true, false));

    // skipMeasure false but SkipMeasureContent true: return false
    layoutWrapper->skipMeasureContent_ = true;
    EXPECT_FALSE(pattern->OnDirtyLayoutWrapperSwap(layoutWrapper, false, false));

    // otherwise: update distortion and return true
    layoutWrapper->skipMeasureContent_ = false;
    EXPECT_TRUE(pattern->OnDirtyLayoutWrapperSwap(layoutWrapper, false, false));
}
} // namespace OHOS::Ace::NG
