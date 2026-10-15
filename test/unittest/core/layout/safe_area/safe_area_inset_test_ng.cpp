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

#include <array>

#include "test/unittest/core/pattern/test_ng.h"

#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_abstract_model_ng.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/syntax/if_else_node.h"
#include "core/components_ng/pattern/safe_area_inset/safe_area_inset_pattern.h"
#include "core/components_ng/pattern/stage/stage_manager.h"
#include "core/components_ng/property/measure_property.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr float HOST_WIDTH = 300.0f;
constexpr float HOST_HEIGHT = 240.0f;
constexpr float PADDING = 10.0f;
constexpr float RAW_SAFE_AREA = 8.0f;
constexpr float SPACING = 6.0f;

constexpr std::array<SafeAreaInsetEdge, SAFE_AREA_INSET_EDGE_COUNT> EDGES = {
    SafeAreaInsetEdge::TOP, SafeAreaInsetEdge::END, SafeAreaInsetEdge::BOTTOM, SafeAreaInsetEdge::START };

class CountingLayoutAlgorithm : public LayoutAlgorithm {
    DECLARE_ACE_TYPE(CountingLayoutAlgorithm, LayoutAlgorithm);

public:
    explicit CountingLayoutAlgorithm(int32_t& count) : count_(count) {}

    void Measure(LayoutWrapper* layoutWrapper) override
    {
        ++count_;
        layoutWrapper->GetGeometryNode()->SetFrameSize(SizeF(50.0f, 20.0f));
    }

private:
    int32_t& count_;
};

class CountingPattern : public Pattern {
    DECLARE_ACE_TYPE(CountingPattern, Pattern);

public:
    explicit CountingPattern(int32_t& count) : count_(count) {}

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<CountingLayoutAlgorithm>(count_);
    }

private:
    int32_t& count_;
};

SafeAreaInsetEdgeOptions MakeEdgeOptions(
    SafeAreaInsetAlignment alignment = SafeAreaInsetAlignment::CENTER, float spacing = SPACING)
{
    return { .spacing = CalcLength(spacing, DimensionUnit::PX), .alignment = alignment };
}

SafeAreaInsetOptions MakeOptions(SafeAreaInsetEdge edge,
    SafeAreaInsetAlignment alignment = SafeAreaInsetAlignment::CENTER, float spacing = SPACING)
{
    SafeAreaInsetOptions options;
    options.GetEdge(edge) = MakeEdgeOptions(alignment, spacing);
    return options;
}

RefPtr<FrameNode> MakeContent(const char* tag, float width, float height)
{
    auto node = FrameNode::CreateFrameNode(
        tag, ViewStackProcessor::GetInstance()->ClaimNodeId(), AceType::MakeRefPtr<Pattern>());
    CHECK_NULL_RETURN(node, nullptr);
    auto layoutProperty = node->GetLayoutProperty();
    CHECK_NULL_RETURN(layoutProperty, nullptr);
    layoutProperty->UpdateUserDefinedIdealSize(CalcSize(
        CalcLength(width, DimensionUnit::PX), CalcLength(height, DimensionUnit::PX)));
    return node;
}

SafeAreaInsetContentArray MakeFourContents()
{
    SafeAreaInsetContentArray contents;
    contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::TOP)] = MakeContent("topInset", 120.0f, 40.0f);
    contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::END)] = MakeContent("endInset", 40.0f, 120.0f);
    contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::BOTTOM)] = MakeContent("bottomInset", 100.0f, 30.0f);
    contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::START)] = MakeContent("startInset", 30.0f, 100.0f);
    return contents;
}

SafeAreaInsetOptions MakeFourOptions()
{
    SafeAreaInsetOptions options;
    for (const auto edge : EDGES) {
        options.GetEdge(edge) = MakeEdgeOptions();
    }
    return options;
}

void SetHostAttributes(bool withSafeAreaPadding = true)
{
    ViewAbstract::SetWidth(CalcLength(HOST_WIDTH, DimensionUnit::PX));
    ViewAbstract::SetHeight(CalcLength(HOST_HEIGHT, DimensionUnit::PX));
    PaddingProperty padding;
    padding.SetEdges(CalcLength(PADDING, DimensionUnit::PX));
    ViewAbstract::SetPadding(padding);
    if (withSafeAreaPadding) {
        PaddingProperty safeAreaPadding;
        safeAreaPadding.SetEdges(CalcLength(RAW_SAFE_AREA, DimensionUnit::PX));
        ViewAbstract::SetSafeAreaPadding(safeAreaPadding);
    }
}

RefPtr<SafeAreaInsetPattern> GetInsetPattern(const RefPtr<FrameNode>& host)
{
    CHECK_NULL_RETURN(host, nullptr);
    auto container = host->GetSafeAreaInsetNode();
    CHECK_NULL_RETURN(container, nullptr);
    return container->GetPattern<SafeAreaInsetPattern>();
}
RefPtr<FrameNode> GetContentFrame(const RefPtr<SafeAreaInsetPattern>& pattern, SafeAreaInsetEdge edge)
{
    auto content = pattern->GetEdgeContent(edge);
    return content ? AceType::DynamicCast<FrameNode>(content->GetFrameChildByIndex(0, true)) : nullptr;
}
} // namespace

class SafeAreaInsetTestNg : public TestNG {};

/**
 * @tc.name: SafeAreaInsetFourEdgesLayout001
 * @tc.desc: Four logical edges share one observation box and resolve with per-edge max semantics.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetFourEdgesLayout001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        SetHostAttributes();
        auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        ASSERT_NE(frameNode, nullptr);
        ViewAbstract::SetSafeAreaInset(AceType::RawPtr(frameNode), MakeFourOptions(), MakeFourContents());
    });
    ASSERT_NE(host, nullptr);
    FlushUITasks(host);

    auto container = host->GetSafeAreaInsetNode();
    auto pattern = GetInsetPattern(host);
    ASSERT_NE(container, nullptr);
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(host->GetTotalChildCount(), 0);
    EXPECT_EQ(container->GetGeometryNode()->GetFrameOffset(), OffsetF(PADDING, PADDING));
    EXPECT_EQ(container->GetGeometryNode()->GetFrameSize(), SizeF(280.0f, 220.0f));
    EXPECT_EQ(GetContentFrame(pattern, SafeAreaInsetEdge::TOP)->GetGeometryNode()->GetFrameOffset(),
        OffsetF(80.0f, 0.0f));
    EXPECT_EQ(GetContentFrame(pattern, SafeAreaInsetEdge::END)->GetGeometryNode()->GetFrameOffset(),
        OffsetF(240.0f, 50.0f));
    EXPECT_EQ(GetContentFrame(pattern, SafeAreaInsetEdge::BOTTOM)->GetGeometryNode()->GetFrameOffset(),
        OffsetF(90.0f, 190.0f));
    EXPECT_EQ(GetContentFrame(pattern, SafeAreaInsetEdge::START)->GetGeometryNode()->GetFrameOffset(),
        OffsetF(0.0f, 60.0f));

    const auto& contribution = host->GetLayoutProperty()->GetSafeAreaInsetContribution();
    EXPECT_FLOAT_EQ(contribution.top.value_or(0.0f), 46.0f);
    EXPECT_FLOAT_EQ(contribution.right.value_or(0.0f), 46.0f);
    EXPECT_FLOAT_EQ(contribution.bottom.value_or(0.0f), 36.0f);
    EXPECT_FLOAT_EQ(contribution.left.value_or(0.0f), 36.0f);
    auto effective = host->GetLayoutProperty()->GetOrCreateSafeAreaPadding(true);
    EXPECT_FLOAT_EQ(effective.top.value_or(0.0f), 46.0f);
    EXPECT_FLOAT_EQ(effective.right.value_or(0.0f), 46.0f);
    EXPECT_FLOAT_EQ(effective.bottom.value_or(0.0f), 36.0f);
    EXPECT_FLOAT_EQ(effective.left.value_or(0.0f), 36.0f);
}

/**
 * @tc.name: SafeAreaInsetRtlLogicalEdges001
 * @tc.desc: START and END contribution and placement mirror in RTL while top alignment follows direction.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetRtlLogicalEdges001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        SetHostAttributes(false);
        ViewAbstract::SetLayoutDirection(TextDirection::RTL);
        auto options = MakeFourOptions();
        options.GetEdge(SafeAreaInsetEdge::TOP)->alignment = SafeAreaInsetAlignment::START;
        auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        ASSERT_NE(frameNode, nullptr);
        ViewAbstract::SetSafeAreaInset(AceType::RawPtr(frameNode), options, MakeFourContents());
    });
    ASSERT_NE(host, nullptr);
    FlushUITasks(host);

    auto pattern = GetInsetPattern(host);
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(GetContentFrame(pattern, SafeAreaInsetEdge::TOP)->GetGeometryNode()->GetFrameOffset(),
        OffsetF(160.0f, 0.0f));
    EXPECT_EQ(GetContentFrame(pattern, SafeAreaInsetEdge::START)->GetGeometryNode()->GetFrameOffset(),
        OffsetF(250.0f, 60.0f));
    EXPECT_EQ(GetContentFrame(pattern, SafeAreaInsetEdge::END)->GetGeometryNode()->GetFrameOffset(),
        OffsetF(0.0f, 50.0f));
    const auto& contribution = host->GetLayoutProperty()->GetSafeAreaInsetContribution();
    EXPECT_FLOAT_EQ(contribution.left.value_or(0.0f), 46.0f);
    EXPECT_FLOAT_EQ(contribution.right.value_or(0.0f), 36.0f);
}

/**
 * @tc.name: SafeAreaInsetRtlSaturationOrder001
 * @tc.desc: Opposing logical insets saturate from the RTL start side when their sum exceeds host width.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetRtlSaturationOrder001, TestSize.Level0)
{
    auto host = FrameNode::CreateFrameNode("rtlSaturation", ViewStackProcessor::GetInstance()->ClaimNodeId(),
        AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(host, nullptr);
    auto property = host->GetLayoutProperty();
    ASSERT_NE(property, nullptr);
    property->UpdateLayoutDirection(TextDirection::RTL);
    SafeAreaInsetOptions options;
    options.GetEdge(SafeAreaInsetEdge::START) = MakeEdgeOptions();
    options.GetEdge(SafeAreaInsetEdge::END) = MakeEdgeOptions();
    property->UpdateSafeAreaInset(options);
    LayoutConstraintF constraint;
    constraint.maxSize = SizeF(50.0f, 100.0f);
    constraint.percentReference = SizeF(50.0f, 100.0f);
    constraint.selfIdealSize = OptionalSizeF(50.0f, 100.0f);
    host->GetPattern()->CheckLocalized();
    property->UpdateLayoutConstraint(constraint);
    property->UpdateSafeAreaInsetContribution(PaddingPropertyF { .left = 40.0f, .right = 40.0f });

    auto effective = property->CreateSafeAreaPadding();
    EXPECT_FLOAT_EQ(effective.right.value_or(0.0f), 40.0f);
    EXPECT_FLOAT_EQ(effective.left.value_or(0.0f), 10.0f);
}

/**
 * @tc.name: SafeAreaInsetSnapshotReplaceReset001
 * @tc.desc: Complete snapshots clear missing edges, preserve the container, and full reset removes it.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetSnapshotReplaceReset001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        ASSERT_NE(frameNode, nullptr);
        ViewAbstract::SetSafeAreaInset(AceType::RawPtr(frameNode), MakeFourOptions(), MakeFourContents());
    });
    ASSERT_NE(host, nullptr);
    auto originalContainer = host->GetSafeAreaInsetNode();
    ASSERT_NE(originalContainer, nullptr);

    auto bottomContent = MakeContent("bottomSnapshot", 90.0f, 25.0f);
    SafeAreaInsetContentArray replacement;
    replacement[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::BOTTOM)] = bottomContent;
    ViewAbstract::SetSafeAreaInset(AceType::RawPtr(host), MakeOptions(SafeAreaInsetEdge::BOTTOM), replacement);
    EXPECT_EQ(host->GetSafeAreaInsetNode(), originalContainer);
    auto pattern = GetInsetPattern(host);
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(pattern->GetEdgeContent(SafeAreaInsetEdge::TOP), nullptr);
    EXPECT_EQ(pattern->GetEdgeContent(SafeAreaInsetEdge::END), nullptr);
    EXPECT_EQ(pattern->GetEdgeContent(SafeAreaInsetEdge::BOTTOM), bottomContent);
    EXPECT_EQ(pattern->GetEdgeContent(SafeAreaInsetEdge::START), nullptr);
    FlushUITasks(host);
    const auto& contribution = host->GetLayoutProperty()->GetSafeAreaInsetContribution();
    EXPECT_FALSE(contribution.top.has_value());
    EXPECT_FALSE(contribution.left.has_value());
    EXPECT_FALSE(contribution.right.has_value());
    EXPECT_FLOAT_EQ(contribution.bottom.value_or(0.0f), 31.0f);

    ViewAbstract::ResetSafeAreaInset(AceType::RawPtr(host));
    EXPECT_EQ(host->GetSafeAreaInsetNode(), nullptr);
    EXPECT_FALSE(host->GetLayoutProperty()->GetSafeAreaInset().has_value());
    EXPECT_FALSE(host->GetLayoutProperty()->GetSafeAreaInsetContribution().HasValue());
    EXPECT_EQ(originalContainer->GetParent(), nullptr);
}

/**
 * @tc.name: SafeAreaInsetSparseOrderAndIdentity001
 * @tc.desc: Sparse content roots stay in logical order and retain their identity.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetSparseOrderAndIdentity001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        ASSERT_NE(frameNode, nullptr);
        auto options = MakeOptions(SafeAreaInsetEdge::TOP);
        options.GetEdge(SafeAreaInsetEdge::START) = MakeEdgeOptions();
        SafeAreaInsetContentArray contents;
        contents[0] = MakeContent("topOrder", 50.0f, 10.0f);
        contents[3] = MakeContent("startOrder", 10.0f, 50.0f);
        ViewAbstract::SetSafeAreaInset(AceType::RawPtr(frameNode), options, contents);
    });
    ASSERT_NE(host, nullptr);
    auto pattern = GetInsetPattern(host);
    ASSERT_NE(pattern, nullptr);
    auto startFrameNode = GetContentFrame(pattern, SafeAreaInsetEdge::START);
    auto startContent = pattern->GetEdgeContent(SafeAreaInsetEdge::START);
    ASSERT_NE(startFrameNode, nullptr);

    auto options = MakeOptions(SafeAreaInsetEdge::END);
    options.GetEdge(SafeAreaInsetEdge::START) = MakeEdgeOptions();
    SafeAreaInsetContentArray replacement;
    auto endContent = MakeContent("endOrder", 10.0f, 50.0f);
    replacement[1] = endContent;
    replacement[3] = startContent;
    ViewAbstract::SetSafeAreaInset(AceType::RawPtr(host), options, replacement);
    EXPECT_EQ(GetContentFrame(pattern, SafeAreaInsetEdge::START), startFrameNode);
    ASSERT_EQ(host->GetSafeAreaInsetNode()->GetChildren().size(), 2);
    EXPECT_EQ(host->GetSafeAreaInsetNode()->GetFirstChild(), GetContentFrame(pattern, SafeAreaInsetEdge::END));
    EXPECT_EQ(host->GetSafeAreaInsetNode()->GetLastChild(), startFrameNode);

    auto startFrame = AceType::DynamicCast<FrameNode>(startContent);
    ASSERT_NE(startFrame, nullptr);
    ASSERT_NE(endContent, nullptr);
    startFrame->GetRenderContext()->UpdateZIndex(1);
    endContent->GetRenderContext()->UpdateZIndex(5);
    host->AttachToMainTree(true, PipelineContext::GetCurrentContext().GetRawPtr());
    auto container = host->GetSafeAreaInsetNode();
    container->MarkNeedSyncRenderTree(true);
    container->RebuildRenderContextTree();
    std::vector<RefPtr<FrameNode>> renderedChildren;
    for (const auto& child : container->GetFrameChildren()) {
        renderedChildren.emplace_back(child.Upgrade());
    }
    ASSERT_EQ(renderedChildren.size(), 2);
    EXPECT_EQ(renderedChildren[0], startFrame);
    EXPECT_EQ(renderedChildren[1], endContent);
    host->DetachFromMainTree(true);
}

/**
 * @tc.name: SafeAreaInsetBuilderSingleRoot001
 * @tc.desc: Each single-root builder executes once in isolation without a framework edge wrapper.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetBuilderSingleRoot001, TestSize.Level0)
{
    int32_t executeCount = 0;
    RefPtr<FrameNode> content;
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes(false);
        auto outer = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        SafeAreaInsetBuilderArray builders;
        builders[0] = [&]() {
            ++executeCount;
            content = MakeContent("singleRoot", 80.0f, 20.0f);
            ViewStackProcessor::GetInstance()->Push(content);
        };
        ViewAbstractModelNG::SetSafeAreaInset(MakeOptions(SafeAreaInsetEdge::TOP), std::move(builders));
        EXPECT_EQ(ViewStackProcessor::GetInstance()->GetMainFrameNode(), outer);
    });
    ASSERT_NE(host, nullptr);
    ASSERT_NE(content, nullptr);
    FlushUITasks(host);
    EXPECT_EQ(executeCount, 1);
    EXPECT_EQ(GetInsetPattern(host)->GetEdgeContent(SafeAreaInsetEdge::TOP), content);
    EXPECT_EQ(content->GetParent(), host->GetSafeAreaInsetNode());
    EXPECT_FALSE(content->GetIsLayoutNode());
    EXPECT_FALSE(content->IsInternal());
    EXPECT_EQ(content->GetGeometryNode()->GetFrameOffset(), OffsetF(100.0f, 0.0f));
}

/**
 * @tc.name: SafeAreaInsetCustomSingleRoot001
 * @tc.desc: A deferred CustomNode is expanded by the ordinary layout path and remains the owned root.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetCustomSingleRoot001, TestSize.Level0)
{
    auto content = MakeContent("customContent", 80.0f, 25.0f);
    auto custom = CustomNode::CreateCustomNode(ViewStackProcessor::GetInstance()->ClaimNodeId(), "insetCustom");
    int32_t builds = 0;
    custom->SetRenderFunction([&](int64_t, bool&) -> RefPtr<UINode> {
        ++builds;
        return content;
    });
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes(false);
        SafeAreaInsetBuilderArray builders;
        builders[2] = [&]() { ViewStackProcessor::GetInstance()->Push(custom); };
        ViewAbstractModelNG::SetSafeAreaInset(MakeOptions(SafeAreaInsetEdge::BOTTOM), std::move(builders));
    });
    FlushUITasks(host);
    EXPECT_EQ(builds, 1);
    EXPECT_EQ(custom->GetParent(), host->GetSafeAreaInsetNode());
    EXPECT_EQ(content->GetParent(), custom);
    EXPECT_EQ(content->GetGeometryNode()->GetFrameOffset(), OffsetF(100.0f, 195.0f));
    EXPECT_FLOAT_EQ(host->GetLayoutProperty()->GetSafeAreaInsetContribution().bottom.value_or(0.0f), 31.0f);
}

/**
 * @tc.name: SafeAreaInsetMoveRootsBetweenEdges001
 * @tc.desc: Reusing roots on different edges in a new snapshot does not detach an already reinstalled root.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetMoveRootsBetweenEdges001, TestSize.Level0)
{
    auto top = MakeContent("movedTop", 80.0f, 20.0f);
    auto bottom = MakeContent("movedBottom", 60.0f, 30.0f);
    auto options = MakeOptions(SafeAreaInsetEdge::TOP);
    options.GetEdge(SafeAreaInsetEdge::BOTTOM) = MakeEdgeOptions();
    SafeAreaInsetContentArray contents;
    contents[0] = top;
    contents[2] = bottom;
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes(false);
        ViewAbstract::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(), options, contents);
    });
    FlushUITasks(host);
    std::swap(contents[0], contents[2]);
    ViewAbstract::SetSafeAreaInset(AceType::RawPtr(host), options, contents);
    FlushUITasks(host);
    auto container = host->GetSafeAreaInsetNode();
    EXPECT_EQ(container->GetChildren().size(), 2);
    EXPECT_EQ(container->GetFirstChild(), bottom);
    EXPECT_EQ(container->GetLastChild(), top);
    EXPECT_EQ(top->GetParent(), container);
    EXPECT_EQ(bottom->GetParent(), container);
    EXPECT_EQ(top->GetGeometryNode()->GetFrameOffset(), OffsetF(100.0f, 200.0f));
    EXPECT_EQ(bottom->GetGeometryNode()->GetFrameOffset(), OffsetF(110.0f, 0.0f));
}

/**
 * @tc.name: SafeAreaInsetDirectContentMargin001
 * @tc.desc: Direct content placement uses its margin frame, and spacing only changes contribution.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetDirectContentMargin001, TestSize.Level0)
{
    auto content = MakeContent("marginContent", 80.0f, 20.0f);
    content->GetLayoutProperty()->UpdateMargin(MarginProperty {
        .left = CalcLength(2.0f, DimensionUnit::PX), .right = CalcLength(8.0f, DimensionUnit::PX),
        .top = CalcLength(4.0f, DimensionUnit::PX), .bottom = CalcLength(6.0f, DimensionUnit::PX) });
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes(false);
        SafeAreaInsetContentArray contents;
        contents[0] = content;
        ViewAbstract::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::TOP), contents);
    });
    FlushUITasks(host);
    EXPECT_EQ(content->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(95.0f, 0.0f));
    EXPECT_EQ(content->GetGeometryNode()->GetFrameOffset(), OffsetF(97.0f, 4.0f));
    EXPECT_FLOAT_EQ(host->GetLayoutProperty()->GetSafeAreaInsetContribution().top.value_or(0.0f), 36.0f);
}

/**
 * @tc.name: SafeAreaInsetDynamicDirectLayout001
 * @tc.desc: Host size and logical direction updates move the actual content without an offset carrier.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetDynamicDirectLayout001, TestSize.Level0)
{
    auto content = MakeContent("movingContent", 80.0f, 20.0f);
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes(false);
        SafeAreaInsetContentArray contents;
        contents[0] = content;
        ViewAbstract::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::TOP, SafeAreaInsetAlignment::END), contents);
    });
    FlushUITasks(host);
    EXPECT_EQ(content->GetGeometryNode()->GetFrameOffset(), OffsetF(200.0f, 0.0f));
    host->GetLayoutProperty()->UpdateUserDefinedIdealSize(CalcSize(
        CalcLength(400.0f, DimensionUnit::PX), CalcLength(HOST_HEIGHT, DimensionUnit::PX)));
    host->MarkModifyDone();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(host);
    EXPECT_EQ(content->GetGeometryNode()->GetFrameOffset(), OffsetF(300.0f, 0.0f));
    host->GetLayoutProperty()->UpdateLayoutDirection(TextDirection::RTL);
    host->MarkModifyDone();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(host);
    EXPECT_EQ(content->GetGeometryNode()->GetFrameOffset(), OffsetF(0.0f, 0.0f));
    EXPECT_EQ(content->GetParent(), host->GetSafeAreaInsetNode());
}

/**
 * @tc.name: SafeAreaInsetEmptySyntaxRootIndex001
 * @tc.desc: An empty syntax root does not steal the following edge's frame wrapper.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetEmptySyntaxRootIndex001, TestSize.Level0)
{
    auto syntax = IfElseNode::GetOrCreateIfElseNode(ViewStackProcessor::GetInstance()->ClaimNodeId());
    auto bottom = MakeContent("afterEmptyRoot", 80.0f, 20.0f);
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes(false);
        auto options = MakeOptions(SafeAreaInsetEdge::TOP);
        options.GetEdge(SafeAreaInsetEdge::BOTTOM) = MakeEdgeOptions();
        SafeAreaInsetContentArray contents;
        contents[0] = syntax;
        contents[2] = bottom;
        ViewAbstract::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(), options, contents);
    });
    FlushUITasks(host);
    EXPECT_EQ(bottom->GetGeometryNode()->GetFrameOffset(), OffsetF(100.0f, 200.0f));
    const auto& contribution = host->GetLayoutProperty()->GetSafeAreaInsetContribution();
    EXPECT_FALSE(contribution.top.has_value());
    EXPECT_FLOAT_EQ(contribution.bottom.value_or(0.0f), 26.0f);
}

/**
 * @tc.name: SafeAreaInsetAttributeConstraintMeasure001
 * @tc.desc: Attribute-owned constraint updates measure inset; property clones do not measure the live host.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetAttributeConstraintMeasure001, TestSize.Level0)
{
    auto host = FrameNode::CreateFrameNode("attributeConstraintHost", ViewStackProcessor::GetInstance()->ClaimNodeId(),
        AceType::MakeRefPtr<Pattern>());
    SafeAreaInsetContentArray contents;
    contents[0] = MakeContent("attributeInset", 100.0f, 20.0f);
    ViewAbstract::SetSafeAreaInset(AceType::RawPtr(host), MakeOptions(SafeAreaInsetEdge::TOP), contents);
    auto property = host->GetLayoutProperty();
    LayoutConstraintF constraint;
    constraint.maxSize = SizeF(300.0f, 400.0f);
    constraint.percentReference = SizeF(300.0f, 400.0f);
    constraint.selfIdealSize = OptionalSizeF(300.0f, 400.0f);
    host->GetPattern()->CheckLocalized();
    property->UpdateLayoutConstraint(constraint);
    EXPECT_FLOAT_EQ(property->GetSafeAreaInsetContribution().top.value_or(0.0f), 26.0f);
    auto clone = property->Clone();
    property->UpdateSafeAreaInsetContribution({});
    clone->UpdateLayoutConstraint(constraint);
    EXPECT_FALSE(property->GetSafeAreaInsetContribution().HasValue());
}

/**
 * @tc.name: SafeAreaInsetAbsentContentPercent001
 * @tc.desc: Without inset, safeAreaPadding uses the legacy content percent reference.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetAbsentContentPercent001, TestSize.Level0)
{
    auto host = FrameNode::CreateFrameNode("legacyPercentHost", ViewStackProcessor::GetInstance()->ClaimNodeId(),
        AceType::MakeRefPtr<Pattern>());
    auto property = host->GetLayoutProperty();
    PaddingProperty padding;
    padding.top = CalcLength(0.1, DimensionUnit::PERCENT);
    property->UpdateSafeAreaPadding(padding);
    LayoutConstraintF constraint;
    constraint.maxSize = SizeF(200.0f, 400.0f);
    constraint.percentReference = SizeF(200.0f, 400.0f);
    constraint.parentIdealSize.SetWidth(100.0f);
    property->UpdateLayoutConstraint(constraint);
    property->UpdateContentConstraint();
    ASSERT_TRUE(property->GetContentLayoutConstraint().has_value());
    EXPECT_FLOAT_EQ(property->GetContentLayoutConstraint()->maxSize.Height(), 390.0f);
}

/**
 * @tc.name: SafeAreaInsetCommonLocalized001
 * @tc.desc: Common localization resolves physical edges/alignment without mutating raw options or double mirroring.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetCommonLocalized001, TestSize.Level0)
{
    auto host = FrameNode::CreateFrameNode("localizedInsetHost", ViewStackProcessor::GetInstance()->ClaimNodeId(),
        AceType::MakeRefPtr<Pattern>());
    auto property = host->GetLayoutProperty();
    auto options = MakeFourOptions();
    options.GetEdge(SafeAreaInsetEdge::TOP)->alignment = SafeAreaInsetAlignment::START;
    property->UpdateSafeAreaInset(options);
    property->UpdateLayoutDirection(TextDirection::RTL);
    host->GetPattern()->CheckLocalized();
    EXPECT_EQ(property->GetResolvedSafeAreaInsetEdge(SafeAreaInsetEdge::START).edge, SafeAreaInsetPhysicalEdge::RIGHT);
    EXPECT_EQ(property->GetResolvedSafeAreaInsetEdge(SafeAreaInsetEdge::TOP).alignment, SafeAreaInsetAlignment::END);
    host->GetPattern()->CheckLocalized();
    EXPECT_EQ(property->GetResolvedSafeAreaInsetEdge(SafeAreaInsetEdge::TOP).alignment, SafeAreaInsetAlignment::END);
    EXPECT_EQ(property->GetSafeAreaInset()->GetEdge(SafeAreaInsetEdge::TOP)->alignment, SafeAreaInsetAlignment::START);
    property->UpdateLayoutDirection(TextDirection::LTR);
    host->GetPattern()->CheckLocalized();
    EXPECT_EQ(property->GetResolvedSafeAreaInsetEdge(SafeAreaInsetEdge::START).edge, SafeAreaInsetPhysicalEdge::LEFT);
    EXPECT_EQ(property->GetResolvedSafeAreaInsetEdge(SafeAreaInsetEdge::TOP).alignment, SafeAreaInsetAlignment::START);
}

/**
 * @tc.name: SafeAreaInsetConstraintDeterminedAxes001
 * @tc.desc: Observation parentIdealSize contains only axes determined before inset measurement.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetConstraintDeterminedAxes001, TestSize.Level0)
{
    auto host = FrameNode::CreateFrameNode("constraintHost", ViewStackProcessor::GetInstance()->ClaimNodeId(),
        AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(host, nullptr);
    auto property = host->GetLayoutProperty();
    ASSERT_NE(property, nullptr);
    LayoutConstraintF constraint;
    constraint.maxSize = SizeF(300.0f, 400.0f);
    constraint.percentReference = SizeF(300.0f, 400.0f);
    constraint.selfIdealSize.SetWidth(120.0f);
    property->UpdateLayoutConstraint(constraint);

    auto insetConstraint = property->CreateSafeAreaInsetConstraint();
    ASSERT_TRUE(insetConstraint.parentIdealSize.Width().has_value());
    EXPECT_FLOAT_EQ(insetConstraint.parentIdealSize.Width().value(), 120.0f);
    EXPECT_FALSE(insetConstraint.parentIdealSize.Height().has_value());
    EXPECT_FLOAT_EQ(insetConstraint.maxSize.Height(), 400.0f);
}

/**
 * @tc.name: SafeAreaInsetLayoutRectPremeasure001
 * @tc.desc: The layoutRect constraint path still premeasures inset content before host content.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetLayoutRectPremeasure001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        ASSERT_NE(frameNode, nullptr);
        frameNode->GetLayoutProperty()->SetLayoutRect(RectF(5.0f, 6.0f, HOST_WIDTH, HOST_HEIGHT));
        SafeAreaInsetContentArray contents;
        contents[0] = MakeContent("layoutRectInset", 100.0f, 20.0f);
        ViewAbstract::SetSafeAreaInset(AceType::RawPtr(frameNode), MakeOptions(SafeAreaInsetEdge::TOP), contents);
    });
    ASSERT_NE(host, nullptr);
    FlushUITasks(host);
    EXPECT_FLOAT_EQ(host->GetLayoutProperty()->GetSafeAreaInsetContribution().top.value_or(0.0f), 26.0f);
    EXPECT_EQ(host->GetGeometryNode()->GetFrameOffset(), OffsetF(5.0f, 6.0f));
}

/**
 * @tc.name: SafeAreaInsetMeasureCountAndStableFrame001
 * @tc.desc: Each valid edge is measured once per dirty solve and stable flushes do not remeasure it.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetMeasureCountAndStableFrame001, TestSize.Level0)
{
    int32_t measureCount = 0;
    RefPtr<FrameNode> content;
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes(false);
        content = FrameNode::CreateFrameNode("countingInset", ViewStackProcessor::GetInstance()->ClaimNodeId(),
            MakeRefPtr<CountingPattern>(measureCount));
        SafeAreaInsetContentArray contents;
        contents[0] = content;
        auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        ASSERT_NE(frameNode, nullptr);
        ViewAbstract::SetSafeAreaInset(AceType::RawPtr(frameNode), MakeOptions(SafeAreaInsetEdge::TOP), contents);
    });
    ASSERT_NE(host, nullptr);
    ASSERT_NE(content, nullptr);
    FlushUITasks(host);
    EXPECT_EQ(measureCount, 1);
    FlushUITasks(host);
    EXPECT_EQ(measureCount, 1);

    content->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushUITasks(host);
    EXPECT_EQ(measureCount, 2);
}

/**
 * @tc.name: SafeAreaInsetSystemAccumulation001
 * @tc.desc: Local per-edge max is accumulated with system safe area while keyboard remains isolated.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetSystemAccumulation001, TestSize.Level0)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    auto stage = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ViewStackProcessor::GetInstance()->ClaimNodeId(), AceType::MakeRefPtr<Pattern>());
    auto host = FrameNode::CreateFrameNode(
        "safeAreaInsetParent", ViewStackProcessor::GetInstance()->ClaimNodeId(), AceType::MakeRefPtr<Pattern>());
    auto child = FrameNode::CreateFrameNode(
        "safeAreaInsetChild", ViewStackProcessor::GetInstance()->ClaimNodeId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(stage, nullptr);
    ASSERT_NE(host, nullptr);
    ASSERT_NE(child, nullptr);
    host->MountToParent(stage);
    child->MountToParent(host);
    stage->GetGeometryNode()->SetFrameSize(SizeF(200.0f, 200.0f));
    host->GetGeometryNode()->SetFrameOffset(OffsetF(0.0f, 30.0f));
    host->GetGeometryNode()->SetFrameSize(SizeF(100.0f, 140.0f));
    child->GetGeometryNode()->SetFrameOffset(OffsetF(10.0f, 20.0f));
    child->GetGeometryNode()->SetFrameSize(SizeF(75.0f, 95.0f));

    pipeline->stageManager_ = AceType::MakeRefPtr<StageManager>(stage);
    auto manager = pipeline->GetSafeAreaManager();
    ASSERT_NE(manager, nullptr);
    manager->SetIgnoreSafeArea(false);
    manager->SetIsFullScreen(false);
    manager->SetIsNeedAvoidWindow(true);
    manager->UpdateSystemSafeArea(
        SafeAreaInsets({ 0.0f, 0.0f }, { 0.0f, 30.0f }, { 0.0f, 0.0f }, { 170.0f, 200.0f }));

    PaddingProperty declared = {
        .left = CalcLength(10.0f),
        .right = CalcLength(15.0f),
        .top = CalcLength(20.0f),
        .bottom = CalcLength(25.0f),
    };
    host->GetLayoutProperty()->UpdateSafeAreaPadding(declared);
    host->GetLayoutProperty()->UpdateSafeAreaInsetContribution(
        PaddingPropertyF { .left = 5.0f, .right = 30.0f, .top = 40.0f, .bottom = 10.0f });

    auto wrapper = child->CreateLayoutWrapper(true, true);
    ASSERT_NE(wrapper, nullptr);
    auto systemResult = wrapper->GetAccumulatedSafeAreaExpand(false,
        { .type = LAYOUT_SAFE_AREA_TYPE_SYSTEM, .edges = LAYOUT_SAFE_AREA_EDGE_ALL });
    EXPECT_EQ(systemResult,
        (PaddingPropertyF { .left = 10.0f, .right = 30.0f, .top = 70.0f, .bottom = 55.0f }));
    auto keyboardResult = wrapper->GetAccumulatedSafeAreaExpand(false,
        { .type = LAYOUT_SAFE_AREA_TYPE_KEYBOARD, .edges = LAYOUT_SAFE_AREA_EDGE_ALL });
    EXPECT_EQ(keyboardResult,
        (PaddingPropertyF { .left = 10.0f, .right = 30.0f, .top = 40.0f, .bottom = 25.0f }));
}

/**
 * @tc.name: SafeAreaInsetSystemSourceMatrix001
 * @tc.desc: System, cutout, navigation, and floating-navigation sources stay on the existing combined path.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetSystemSourceMatrix001, TestSize.Level0)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    auto manager = pipeline->GetSafeAreaManager();
    ASSERT_NE(manager, nullptr);
    manager->SetIgnoreSafeArea(false);
    manager->SetIsFullScreen(false);
    manager->SetIsNeedAvoidWindow(true);
    manager->SetUseCutout(true);
    manager->UpdateSystemSafeArea(
        SafeAreaInsets({ 0.0f, 0.0f }, { 0.0f, 10.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }));
    manager->UpdateCutoutSafeArea(
        SafeAreaInsets({ 0.0f, 5.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }));
    manager->UpdateNavSafeArea(
        SafeAreaInsets({ 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 190.0f, 200.0f }));
    manager->UpdateFloatNavSafeArea(
        SafeAreaInsets({ 0.0f, 0.0f }, { 0.0f, 0.0f }, { 190.0f, 200.0f }, { 0.0f, 0.0f }));

    auto result = manager->SafeAreaToPadding(false, LAYOUT_SAFE_AREA_TYPE_SYSTEM);
    EXPECT_EQ(result,
        (PaddingPropertyF { .left = 5.0f, .right = 10.0f, .top = 10.0f, .bottom = 10.0f }));
}

/**
 * @tc.name: SafeAreaInsetAuxiliaryLifecycle001
 * @tc.desc: The auxiliary container follows main-tree and reset lifecycle without becoming a normal child.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetAuxiliaryLifecycle001, TestSize.Level0)
{
    auto host = FrameNode::CreateFrameNode(
        "safeAreaInsetHost", ViewStackProcessor::GetInstance()->ClaimNodeId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(host, nullptr);
    SafeAreaInsetContentArray contents;
    contents[0] = MakeContent("safeAreaInsetLifecycleContent", 100.0f, 20.0f);
    ViewAbstract::SetSafeAreaInset(AceType::RawPtr(host), MakeOptions(SafeAreaInsetEdge::TOP), contents);
    auto container = host->GetSafeAreaInsetNode();
    ASSERT_NE(container, nullptr);
    EXPECT_EQ(host->GetTotalChildCount(), 0);
    EXPECT_TRUE(container->IsInternal());
    EXPECT_FALSE(container->IsOnMainTree());

    host->AttachToMainTree(true, PipelineContext::GetCurrentContext().GetRawPtr());
    EXPECT_TRUE(container->IsOnMainTree());
    EXPECT_EQ(container->GetParent(), host);
    EXPECT_EQ(container->GetDepth(), host->GetDepth() + 1);
    auto pattern = container->GetPattern<SafeAreaInsetPattern>();
    ASSERT_NE(pattern, nullptr);
    auto edgeWrapper = GetContentFrame(pattern, SafeAreaInsetEdge::TOP);
    ASSERT_NE(edgeWrapper, nullptr);
    EXPECT_EQ(edgeWrapper, contents[0]);
    EXPECT_EQ(edgeWrapper->GetParent(), container);
    EXPECT_FALSE(edgeWrapper->IsInternal());
    EXPECT_FALSE(edgeWrapper->GetIsLayoutNode());

    ViewAbstract::ResetSafeAreaInset(AceType::RawPtr(host));
    EXPECT_FALSE(container->IsOnMainTree());
    EXPECT_EQ(container->GetParent(), nullptr);
    host->DetachFromMainTree(true);
}

/**
 * @tc.name: SafeAreaInsetLpxAndPresence001
 * @tc.desc: LPX registration and source/effective predicates follow snapshot and resolved-value lifecycle.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetLpxAndPresence001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        SetHostAttributes(false);
        auto options = MakeOptions(SafeAreaInsetEdge::TOP);
        options.GetEdge(SafeAreaInsetEdge::TOP)->spacing = CalcLength(SPACING, DimensionUnit::LPX);
        SafeAreaInsetContentArray contents;
        contents[0] = MakeContent("lpxInset", 100.0f, 20.0f);
        auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        ASSERT_NE(frameNode, nullptr);
        ViewAbstract::SetSafeAreaInset(AceType::RawPtr(frameNode), options, contents);
        EXPECT_EQ(frameNode->lpxAttributes_.count(LpxAttribute::LPX_SAFE_AREA_INSET_SPACING), 1);
    });
    ASSERT_NE(host, nullptr);
    EXPECT_TRUE(host->GetLayoutProperty()->HasSafeAreaPaddingSource());
    EXPECT_FALSE(host->GetLayoutProperty()->HasEffectiveSafeAreaPadding());
    FlushUITasks(host);
    host->GetLayoutProperty()->GetOrCreateSafeAreaPadding(true);
    EXPECT_TRUE(host->GetLayoutProperty()->HasEffectiveSafeAreaPadding());

    ViewAbstract::ResetSafeAreaInset(AceType::RawPtr(host));
    EXPECT_EQ(host->lpxAttributes_.count(LpxAttribute::LPX_SAFE_AREA_INSET_SPACING), 0);
    EXPECT_FALSE(host->GetLayoutProperty()->HasSafeAreaPaddingSource());
    EXPECT_FALSE(host->GetLayoutProperty()->HasEffectiveSafeAreaPadding());
}
} // namespace OHOS::Ace::NG
