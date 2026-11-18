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
#include <sstream>

#include "test/unittest/core/pattern/test_ng.h"

#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/log/dump_log.h"
#include "core/components_ng/base/inspector.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/safe_area_inset_view.h"
#include "core/components_ng/base/view_abstract_model_ng.h"
#include "core/components_ng/manager/safe_area/safe_area_manager.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/custom/custom_node.h"
#include "core/components_ng/syntax/if_else_node.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/pattern/safe_area_inset/safe_area_inset_pattern.h"
#include "core/components_ng/pattern/stage/stage_manager.h"
#include "core/components_ng/pattern/stage/page_pattern.h"
#include "core/components_ng/property/safe_area_inset_property.h"

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

class FocusablePattern : public Pattern {
    DECLARE_ACE_TYPE(FocusablePattern, Pattern);

public:
    explicit FocusablePattern(FocusPattern pattern) : pattern_(pattern) {}

    FocusPattern GetFocusPattern() const override
    {
        return pattern_;
    }

private:
    FocusPattern pattern_;
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

RefPtr<FrameNode> MakeFocusableContent(const char* tag, FocusType focusType, bool focusable)
{
    return FrameNode::CreateFrameNode(tag, ViewStackProcessor::GetInstance()->ClaimNodeId(),
        AceType::MakeRefPtr<FocusablePattern>(FocusPattern(focusType, focusable)));
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

std::unique_ptr<JsonValue> GetInsetDump(const RefPtr<FrameNode>& node)
{
    auto json = JsonUtil::Create(true);
    node->DumpSafeAreaInfo(json);
    return json;
}

std::string CaptureInsetDump(const std::function<void()>& operation)
{
    auto& log = DumpLog::GetInstance();
    auto previousStream = std::move(log.ostream_);
    auto previousResult = std::move(log.result_);
    auto previousDescriptions = std::move(log.description_);
    const bool previousAll = log.IsDumpAllNodes();
    auto stream = std::make_unique<std::ostringstream>();
    auto* output = stream.get();
    log.SetDumpFile(std::move(stream));
    log.SetDumpAllNodes(true);
    log.result_.clear();
    log.description_.clear();
    operation();
    auto result = output->str() + log.result_;
    for (const auto& description : log.description_) {
        result += description;
    }
    log.SetDumpFile(std::move(previousStream));
    log.result_ = std::move(previousResult);
    log.description_ = std::move(previousDescriptions);
    log.SetDumpAllNodes(previousAll);
    return result;
}
} // namespace

class SafeAreaInsetTestNg : public TestNG {
protected:
    static void SetUpTestSuite()
    {
        TestNG::SetUpTestSuite();
        MockPipelineContext::GetCurrent()->SetUseFlushUITasks(true);
    }
};

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
        SafeAreaInsetView::SetSafeAreaInset(frameNode, MakeFourOptions(), MakeFourContents());
    });
    ASSERT_NE(host, nullptr);
    // Mirror the real build flow (ViewStackProcessor::Finish -> MarkModifyDone) so that
    // CheckLocalized resolves logical edges against the layout direction before layout.
    host->MarkModifyDone();
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

    const auto& contribution = host->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInsetContribution();
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
        SafeAreaInsetView::SetSafeAreaInset(frameNode, options, MakeFourContents());
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
    const auto& contribution = host->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInsetContribution();
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
        SafeAreaInsetView::SetSafeAreaInset(frameNode, MakeFourOptions(), MakeFourContents());
    });
    ASSERT_NE(host, nullptr);
    auto originalContainer = host->GetSafeAreaInsetNode();
    ASSERT_NE(originalContainer, nullptr);

    auto bottomContent = MakeContent("bottomSnapshot", 90.0f, 25.0f);
    SafeAreaInsetContentArray replacement;
    replacement[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::BOTTOM)] = bottomContent;
    SafeAreaInsetView::SetSafeAreaInset(AceType::RawPtr(host), MakeOptions(SafeAreaInsetEdge::BOTTOM), replacement);
    EXPECT_EQ(host->GetSafeAreaInsetNode(), originalContainer);
    auto pattern = GetInsetPattern(host);
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(pattern->GetEdgeContent(SafeAreaInsetEdge::TOP), nullptr);
    EXPECT_EQ(pattern->GetEdgeContent(SafeAreaInsetEdge::END), nullptr);
    EXPECT_EQ(pattern->GetEdgeContent(SafeAreaInsetEdge::BOTTOM), bottomContent);
    EXPECT_EQ(pattern->GetEdgeContent(SafeAreaInsetEdge::START), nullptr);
    FlushUITasks(host);
    const auto& contribution = host->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInsetContribution();
    EXPECT_FALSE(contribution.top.has_value());
    EXPECT_FALSE(contribution.left.has_value());
    EXPECT_FALSE(contribution.right.has_value());
    EXPECT_FLOAT_EQ(contribution.bottom.value_or(0.0f), 31.0f);

    SafeAreaInsetView::ResetSafeAreaInset(AceType::RawPtr(host));
    EXPECT_EQ(host->GetSafeAreaInsetNode(), nullptr);
    EXPECT_EQ(host->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInset(), nullptr);
    EXPECT_FALSE(host->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInsetContribution().HasValue());
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
        SafeAreaInsetView::SetSafeAreaInset(frameNode, options, contents);
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
    SafeAreaInsetView::SetSafeAreaInset(AceType::RawPtr(host), options, replacement);
    EXPECT_EQ(GetContentFrame(pattern, SafeAreaInsetEdge::START), startFrameNode);
    ASSERT_EQ(host->GetSafeAreaInsetNode()->GetChildren().size(), 2);
    EXPECT_EQ(host->GetSafeAreaInsetNode()->GetFirstChild(), GetContentFrame(pattern, SafeAreaInsetEdge::END));
    EXPECT_EQ(host->GetSafeAreaInsetNode()->GetLastChild(), startFrameNode);

    auto startFrame = AceType::DynamicCast<FrameNode>(startContent);
    ASSERT_NE(startFrame, nullptr);
    ASSERT_NE(endContent, nullptr);
    startFrame->GetRenderContext()->UpdateZIndex(1);
    endContent->GetRenderContext()->UpdateZIndex(5);
    // Mirror the real active lifecycle (SetJSViewActive from the page) so render-tree
    // generation (OnGenerateOneDepthVisibleFrameWithTransition) includes the contents.
    startFrame->SetActive(true);
    endContent->SetActive(true);
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
 * @tc.name: SafeAreaInsetModelExplicitTarget001
 * @tc.desc: The explicit-frameNode model overload mounts the carrier on the given node, not on the stack top.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetModelExplicitTarget001, TestSize.Level0)
{
    RefPtr<FrameNode> content;
    RefPtr<FrameNode> target;
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes(false);
        auto outer = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        ASSERT_NE(outer, nullptr);
        target = MakeContent("explicitTarget", HOST_WIDTH, HOST_HEIGHT);
        outer->AddChild(target);
        SafeAreaInsetBuilderArray builders;
        builders[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::TOP)] = [&]() {
            content = MakeContent("explicitTop", 80.0f, 20.0f);
            ViewStackProcessor::GetInstance()->Push(content);
        };
        ViewAbstractModelNG::SetSafeAreaInset(
            AceType::RawPtr(target), MakeOptions(SafeAreaInsetEdge::TOP), std::move(builders));
        EXPECT_EQ(ViewStackProcessor::GetInstance()->GetMainFrameNode(), outer);
    });
    ASSERT_NE(host, nullptr);
    ASSERT_NE(target, nullptr);
    ASSERT_NE(content, nullptr);
    FlushUITasks(host);

    EXPECT_EQ(host->GetSafeAreaInsetNode(), nullptr);
    EXPECT_NE(target->GetSafeAreaInsetNode(), nullptr);
    auto pattern = GetInsetPattern(target);
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(pattern->GetEdgeContent(SafeAreaInsetEdge::TOP), content);
    EXPECT_EQ(content->GetParent(), target->GetSafeAreaInsetNode());
    EXPECT_FLOAT_EQ(
        target->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInsetContribution().top.value_or(0.0f), 26.0f);
}

/**
 * @tc.name: SafeAreaInsetModelExplicitReset001
 * @tc.desc: The explicit-frameNode reset overload clears the carrier and options on the given node only.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetModelExplicitReset001, TestSize.Level0)
{
    RefPtr<FrameNode> target;
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes(false);
        auto outer = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        ASSERT_NE(outer, nullptr);
        target = MakeContent("explicitResetTarget", HOST_WIDTH, HOST_HEIGHT);
        outer->AddChild(target);
        SafeAreaInsetBuilderArray builders;
        builders[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::BOTTOM)] = []() {
            ViewStackProcessor::GetInstance()->Push(MakeContent("explicitBottom", 60.0f, 15.0f));
        };
        ViewAbstractModelNG::SetSafeAreaInset(
            AceType::RawPtr(target), MakeOptions(SafeAreaInsetEdge::BOTTOM), std::move(builders));
    });
    ASSERT_NE(host, nullptr);
    ASSERT_NE(target, nullptr);
    FlushUITasks(host);
    ASSERT_NE(target->GetSafeAreaInsetNode(), nullptr);
    ASSERT_NE(target->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInset(), nullptr);

    ViewAbstractModelNG::ResetSafeAreaInset(AceType::RawPtr(target));
    FlushUITasks(host);
    EXPECT_EQ(target->GetSafeAreaInsetNode(), nullptr);
    EXPECT_EQ(target->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInset(), nullptr);
    EXPECT_FALSE(target->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInsetContribution().HasValue());
    EXPECT_EQ(host->GetSafeAreaInsetNode(), nullptr);
}

/**
 * @tc.name: SafeAreaInsetModelExplicitEmptySnapshot001
 * @tc.desc: A builder that yields no root drops its edge and the empty snapshot resets the given node.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetModelExplicitEmptySnapshot001, TestSize.Level0)
{
    int32_t executeCount = 0;
    RefPtr<FrameNode> target;
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes(false);
        auto outer = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        ASSERT_NE(outer, nullptr);
        target = MakeContent("explicitEmptyTarget", HOST_WIDTH, HOST_HEIGHT);
        outer->AddChild(target);
        SafeAreaInsetBuilderArray builders;
        builders[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::START)] = [&]() { ++executeCount; };
        ViewAbstractModelNG::SetSafeAreaInset(
            AceType::RawPtr(target), MakeOptions(SafeAreaInsetEdge::START), std::move(builders));
    });
    ASSERT_NE(host, nullptr);
    ASSERT_NE(target, nullptr);
    FlushUITasks(host);

    EXPECT_EQ(executeCount, 1);
    EXPECT_EQ(target->GetSafeAreaInsetNode(), nullptr);
    EXPECT_EQ(target->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInset(), nullptr);
    EXPECT_EQ(host->GetSafeAreaInsetNode(), nullptr);
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
    EXPECT_FLOAT_EQ(
        host->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInsetContribution().bottom.value_or(0.0f), 31.0f);
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
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(), options, contents);
    });
    FlushUITasks(host);
    std::swap(contents[0], contents[2]);
    SafeAreaInsetView::SetSafeAreaInset(AceType::RawPtr(host), options, contents);
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
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::TOP), contents);
    });
    FlushUITasks(host);
    EXPECT_EQ(content->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(95.0f, 0.0f));
    EXPECT_EQ(content->GetGeometryNode()->GetFrameOffset(), OffsetF(97.0f, 4.0f));
    EXPECT_FLOAT_EQ(
        host->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInsetContribution().top.value_or(0.0f), 36.0f);
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
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
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
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(), options, contents);
    });
    FlushUITasks(host);
    EXPECT_EQ(bottom->GetGeometryNode()->GetFrameOffset(), OffsetF(100.0f, 200.0f));
    const auto& contribution = host->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInsetContribution();
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
    SafeAreaInsetView::SetSafeAreaInset(AceType::RawPtr(host), MakeOptions(SafeAreaInsetEdge::TOP), contents);
    auto property = host->GetLayoutProperty();
    LayoutConstraintF constraint;
    constraint.maxSize = SizeF(300.0f, 400.0f);
    constraint.percentReference = SizeF(300.0f, 400.0f);
    constraint.selfIdealSize = OptionalSizeF(300.0f, 400.0f);
    host->GetPattern()->CheckLocalized();
    property->UpdateLayoutConstraint(constraint);
    EXPECT_FLOAT_EQ(property->GetSafeAreaInsetState().GetSafeAreaInsetContribution().top.value_or(0.0f), 26.0f);
    auto clone = property->Clone();
    property->UpdateSafeAreaInsetContribution({});
    clone->UpdateLayoutConstraint(constraint);
    EXPECT_FALSE(property->GetSafeAreaInsetState().GetSafeAreaInsetContribution().HasValue());
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
    EXPECT_EQ(property->GetSafeAreaInsetState().GetResolvedSafeAreaInsetEdge(SafeAreaInsetEdge::START).edge,
        SafeAreaInsetPhysicalEdge::RIGHT);
    EXPECT_EQ(property->GetSafeAreaInsetState().GetResolvedSafeAreaInsetEdge(SafeAreaInsetEdge::TOP).alignment,
        SafeAreaInsetAlignment::END);
    host->GetPattern()->CheckLocalized();
    EXPECT_EQ(property->GetSafeAreaInsetState().GetResolvedSafeAreaInsetEdge(SafeAreaInsetEdge::TOP).alignment,
        SafeAreaInsetAlignment::END);
    EXPECT_EQ(property->GetSafeAreaInsetState().GetSafeAreaInset()->GetEdge(SafeAreaInsetEdge::TOP)->alignment,
        SafeAreaInsetAlignment::START);
    property->UpdateLayoutDirection(TextDirection::LTR);
    host->GetPattern()->CheckLocalized();
    EXPECT_EQ(property->GetSafeAreaInsetState().GetResolvedSafeAreaInsetEdge(SafeAreaInsetEdge::START).edge,
        SafeAreaInsetPhysicalEdge::LEFT);
    EXPECT_EQ(property->GetSafeAreaInsetState().GetResolvedSafeAreaInsetEdge(SafeAreaInsetEdge::TOP).alignment,
        SafeAreaInsetAlignment::START);
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
        SafeAreaInsetView::SetSafeAreaInset(frameNode, MakeOptions(SafeAreaInsetEdge::TOP), contents);
    });
    ASSERT_NE(host, nullptr);
    FlushUITasks(host);
    EXPECT_FLOAT_EQ(
        host->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInsetContribution().top.value_or(0.0f), 26.0f);
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
            AceType::MakeRefPtr<CountingPattern>(measureCount));
        SafeAreaInsetContentArray contents;
        contents[0] = content;
        auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
        ASSERT_NE(frameNode, nullptr);
        SafeAreaInsetView::SetSafeAreaInset(frameNode, MakeOptions(SafeAreaInsetEdge::TOP), contents);
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
    // Accumulation reports the geometric distance of the child into each safe-area band
    // (ParseSafeAreaPaddingSides), not the raw padding values: child rect is 10,20 with size
    // 75x95 inside the host (offset y+30, 100x140) on a 200x200 stage.
    auto systemResult = wrapper->GetAccumulatedSafeAreaExpand(false,
        { .type = LAYOUT_SAFE_AREA_TYPE_SYSTEM, .edges = LAYOUT_SAFE_AREA_EDGE_ALL });
    EXPECT_EQ(systemResult,
        (PaddingPropertyF { .left = 10.0f, .right = 15.0f, .top = 50.0f, .bottom = 55.0f }));
    auto keyboardResult = wrapper->GetAccumulatedSafeAreaExpand(false,
        { .type = LAYOUT_SAFE_AREA_TYPE_KEYBOARD, .edges = LAYOUT_SAFE_AREA_EDGE_ALL });
    EXPECT_EQ(keyboardResult,
        (PaddingPropertyF { .left = 10.0f, .right = 15.0f, .top = 20.0f, .bottom = 25.0f }));
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
    // UpdateFloatNavSafeArea normalizes valid edges to the root bounds
    // (GenerateEdgeSafeAreaWithRoot): right {190,200} becomes {190, rootWidth=720} -> 530.
    EXPECT_EQ(result,
        (PaddingPropertyF { .left = 5.0f, .right = 530.0f, .top = 10.0f, .bottom = 10.0f }));
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
    SafeAreaInsetView::SetSafeAreaInset(AceType::RawPtr(host), MakeOptions(SafeAreaInsetEdge::TOP), contents);
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

    SafeAreaInsetView::ResetSafeAreaInset(AceType::RawPtr(host));
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
        SafeAreaInsetView::SetSafeAreaInset(frameNode, options, contents);
        EXPECT_EQ(frameNode->lpxAttributes_.count(LpxAttribute::LPX_SAFE_AREA_INSET_SPACING), 1);
    });
    ASSERT_NE(host, nullptr);
    EXPECT_TRUE(host->GetLayoutProperty()->HasSafeAreaPaddingSource());
    EXPECT_FALSE(host->GetLayoutProperty()->HasEffectiveSafeAreaPadding());
    FlushUITasks(host);
    host->GetLayoutProperty()->GetOrCreateSafeAreaPadding(true);
    EXPECT_TRUE(host->GetLayoutProperty()->HasEffectiveSafeAreaPadding());

    SafeAreaInsetView::ResetSafeAreaInset(AceType::RawPtr(host));
    EXPECT_EQ(host->lpxAttributes_.count(LpxAttribute::LPX_SAFE_AREA_INSET_SPACING), 0);
    EXPECT_FALSE(host->GetLayoutProperty()->HasSafeAreaPaddingSource());
    EXPECT_FALSE(host->GetLayoutProperty()->HasEffectiveSafeAreaPadding());
}

/**
 * @tc.name: SafeAreaInsetDumpConfiguration001
 * @tc.desc: Keep legacy padding output and expose sparse carrier configuration with node IDs.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetDumpConfiguration001, TestSize.Level0)
{
    auto content = MakeContent("dumpInsetContent", 100.0f, 40.0f);
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes();
        SafeAreaInsetContentArray contents;
        contents[0] = content;
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::TOP), contents);
    });
    FlushUITasks(host);
    auto carrier = host->GetSafeAreaInsetNode();
    ASSERT_NE(carrier, nullptr);
    auto hostJson = JsonUtil::Create(true);
    host->BuildLayoutInfo(hostJson);
    host->DumpSafeAreaInfo(hostJson);
    EXPECT_EQ(hostJson->GetString("Padding"), "[10.00px,10.00px,10.00px,10.00px]");
    EXPECT_EQ(hostJson->GetString("SafeArea Padding"), "[8.00px,8.00px,8.00px,8.00px]");
    EXPECT_EQ(hostJson->GetInt("SafeAreaInsetContainerNodeId"), carrier->GetId());
    EXPECT_FALSE(hostJson->Contains("SafeAreaPaddingContribution"));
    EXPECT_FALSE(hostJson->Contains("SafeAreaInset"));
    auto carrierJson = GetInsetDump(carrier);
    auto config = carrierJson->GetValue("SafeAreaInset");
    auto top = config->GetValue("top");
    EXPECT_EQ(top->GetInt("contentNodeId"), content->GetId());
    EXPECT_EQ(top->GetString("spacing"), "6.00px");
    EXPECT_EQ(top->GetString("alignment"), "CENTER");
    EXPECT_FALSE(config->Contains("bottom"));
    EXPECT_FALSE(config->Contains("start"));
    EXPECT_FALSE(config->Contains("end"));
    EXPECT_EQ(carrierJson->GetString("SafeAreaInsetContribution"), "[NA,NA,46.00,NA]");
    EXPECT_EQ(hostJson->GetString("EffectiveSafeAreaContribution"),
        "[8.00,8.00,46.00,8.00]");

    auto text = CaptureInsetDump([&]() { host->DumpCommonInfo(); carrier->DumpSafeAreaInfo(); });
    EXPECT_NE(text.find("SafeArea Padding: [8.00px,8.00px,8.00px,8.00px]"), std::string::npos);
    EXPECT_NE(text.find("SafeAreaInsetContainerNodeId: " + std::to_string(carrier->GetId())), std::string::npos);
    EXPECT_NE(text.find("SafeAreaInsetContribution: [NA,NA,46.00,NA]"), std::string::npos);
}

/**
 * @tc.name: SafeAreaInsetDumpTwoDecimalRounding001
 * @tc.desc: New contribution fields keep two decimals for values, zero and NA edges, with rounding boundary.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetDumpTwoDecimalRounding001, TestSize.Level0)
{
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes();
        SafeAreaInsetOptions options;
        options.GetEdge(SafeAreaInsetEdge::TOP) = MakeEdgeOptions(SafeAreaInsetAlignment::CENTER, 6.25f);
        options.GetEdge(SafeAreaInsetEdge::END) = MakeEdgeOptions(SafeAreaInsetAlignment::CENTER, 0.0f);
        options.GetEdge(SafeAreaInsetEdge::BOTTOM) = MakeEdgeOptions(SafeAreaInsetAlignment::CENTER, 0.0f);
        SafeAreaInsetContentArray contents;
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::TOP)] = MakeContent("topInset", 120.0f, 40.125f);
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::END)] = MakeContent("endInset", 40.0f, 120.0f);
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::BOTTOM)] = MakeContent("bottomInset", 100.0f, 0.0f);
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(), options, contents);
    });
    FlushUITasks(host);
    auto carrier = host->GetSafeAreaInsetNode();
    ASSERT_NE(carrier, nullptr);
    // left NA (start edge unset), right 40+0, top 40.125+6.25=46.375 rounding to 46.38, bottom 0+0 as 0.00.
    EXPECT_EQ(GetInsetDump(carrier)->GetString("SafeAreaInsetContribution"), "[NA,40.00,46.38,0.00]");
    // Effective takes the per-edge max with the 8px safeAreaPadding: 8.00 / 40.00 / 46.38 / 8.00.
    EXPECT_EQ(GetInsetDump(host)->GetString("EffectiveSafeAreaContribution"), "[8.00,40.00,46.38,8.00]");
}

/**
 * @tc.name: SafeAreaInsetDumpCacheReadOnly001
 * @tc.desc: Dump never computes a missing result and suppresses stale values while layout is dirty.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetDumpCacheReadOnly001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        SetHostAttributes();
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeFourOptions(), MakeFourContents());
    });
    FlushUITasks(host);
    auto carrier = host->GetSafeAreaInsetNode();
    ASSERT_NE(carrier, nullptr);
    auto geometry = host->GetGeometryNode();
    ASSERT_NE(geometry->GetResolvedSingleSafeAreaPadding(), nullptr);
    const auto* cached = geometry->GetResolvedSingleSafeAreaPadding().get();
    const auto value = *cached;
    const auto contribution = host->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInsetContribution();
    const auto flag = host->GetLayoutProperty()->GetPropertyChangeFlag();
    EXPECT_TRUE(GetInsetDump(host)->Contains("EffectiveSafeAreaContribution"));
    EXPECT_TRUE(GetInsetDump(carrier)->Contains("SafeAreaInsetContribution"));
    EXPECT_EQ(geometry->GetResolvedSingleSafeAreaPadding().get(), cached);
    EXPECT_EQ(*cached, value);
    EXPECT_EQ(host->GetLayoutProperty()->GetSafeAreaInsetState().GetSafeAreaInsetContribution(), contribution);
    EXPECT_EQ(host->GetLayoutProperty()->GetPropertyChangeFlag(), flag);

    host->SetLayoutDirtyMarked(true);
    EXPECT_FALSE(GetInsetDump(host)->Contains("EffectiveSafeAreaContribution"));
    EXPECT_FALSE(GetInsetDump(carrier)->Contains("SafeAreaInsetContribution"));
    host->SetLayoutDirtyMarked(false);
    carrier->SetLayoutDirtyMarked(true);
    EXPECT_FALSE(GetInsetDump(host)->Contains("EffectiveSafeAreaContribution"));
    EXPECT_FALSE(GetInsetDump(carrier)->Contains("SafeAreaInsetContribution"));
    carrier->SetLayoutDirtyMarked(false);
    host->ResetSafeAreaPadding();
    auto dump = GetInsetDump(host);
    EXPECT_TRUE(dump->Contains("SafeAreaInsetContainerNodeId"));
    EXPECT_FALSE(dump->Contains("EffectiveSafeAreaContribution"));
    EXPECT_TRUE(GetInsetDump(carrier)->Contains("SafeAreaInset"));
    EXPECT_FALSE(GetInsetDump(carrier)->Contains("SafeAreaInsetContribution"));
    EXPECT_EQ(geometry->GetResolvedSingleSafeAreaPadding(), nullptr);
}

/**
 * @tc.name: SafeAreaInsetDumpUpdateReset001
 * @tc.desc: Configuration updates, replacement and reset do not retain stale diagnostic fields.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetDumpUpdateReset001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        SetHostAttributes();
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeFourOptions(), MakeFourContents());
    });
    FlushUITasks(host);
    auto carrier = host->GetSafeAreaInsetNode();
    auto content = MakeContent("replacementInset", 100.0f, 20.0f);
    SafeAreaInsetContentArray contents;
    contents[0] = content;
    SafeAreaInsetView::SetSafeAreaInset(AceType::RawPtr(host),
        MakeOptions(SafeAreaInsetEdge::TOP, SafeAreaInsetAlignment::END, 0.0f), contents);
    EXPECT_FALSE(GetInsetDump(host)->Contains("EffectiveSafeAreaContribution"));
    auto carrierDump = GetInsetDump(carrier);
    auto config = carrierDump->GetValue("SafeAreaInset");
    EXPECT_EQ(config->GetValue("top")->GetInt("contentNodeId"), content->GetId());
    EXPECT_EQ(config->GetValue("top")->GetString("spacing"), "0.00px");
    EXPECT_FALSE(config->Contains("bottom"));
    FlushUITasks(host);
    EXPECT_EQ(GetInsetDump(carrier)->GetString("SafeAreaInsetContribution"), "[NA,NA,20.00,NA]");

    PaddingProperty padding;
    padding.SetEdges(CalcLength(60.0f, DimensionUnit::PX));
    host->GetLayoutProperty()->UpdateSafeAreaPadding(padding);
    EXPECT_FALSE(GetInsetDump(host)->Contains("EffectiveSafeAreaContribution"));
    EXPECT_FALSE(GetInsetDump(carrier)->Contains("SafeAreaInsetContribution"));
    FlushUITasks(host);
    EXPECT_EQ(GetInsetDump(host)->GetString("EffectiveSafeAreaContribution"),
        "[60.00,60.00,60.00,60.00]");

    SafeAreaInsetView::ResetSafeAreaInset(AceType::RawPtr(host));
    EXPECT_FALSE(GetInsetDump(host)->Contains("SafeAreaInsetContainerNodeId"));
    EXPECT_FALSE(GetInsetDump(host)->Contains("EffectiveSafeAreaContribution"));
    EXPECT_FALSE(GetInsetDump(carrier)->Contains("SafeAreaInset"));
    EXPECT_FALSE(GetInsetDump(carrier)->Contains("SafeAreaInsetContribution"));
    auto json = JsonUtil::Create(true);
    host->BuildLayoutInfo(json);
    EXPECT_EQ(json->GetString("SafeArea Padding"), "[60.00px,60.00px,60.00px,60.00px]");
}

/**
 * @tc.name: SafeAreaInsetDumpRtl001
 * @tc.desc: Logical configuration stays start/end while physical contributions follow layout direction.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetDumpRtl001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        SetHostAttributes();
        ViewAbstract::SetLayoutDirection(TextDirection::RTL);
        auto contents = MakeFourContents();
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::START), contents);
    });
    // Mirror the real build flow: ViewStackProcessor::Finish drives MarkModifyDone, which runs
    // Pattern::CheckLocalized to resolve logical edges against the layout direction.
    host->MarkModifyDone();
    FlushUITasks(host);
    auto carrier = host->GetSafeAreaInsetNode();
    auto dump = GetInsetDump(carrier);
    auto config = dump->GetValue("SafeAreaInset");
    EXPECT_TRUE(config->Contains("start"));
    EXPECT_FALSE(config->Contains("left"));
    EXPECT_FALSE(config->Contains("right"));
    EXPECT_EQ(dump->GetString("SafeAreaInsetContribution"), "[NA,36.00,NA,NA]");
    host->GetLayoutProperty()->UpdateLayoutDirection(TextDirection::LTR);
    EXPECT_FALSE(GetInsetDump(carrier)->Contains("SafeAreaInsetContribution"));
    host->MarkModifyDone();
    FlushUITasks(host);
    EXPECT_EQ(GetInsetDump(carrier)->GetString("SafeAreaInsetContribution"), "[36.00,NA,NA,NA]");
}

/**
 * @tc.name: SafeAreaInsetDumpInspectorFilter001
 * @tc.desc: Inspector adds only retained configuration and respects existing filters and hidden carrier semantics.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetDumpInspectorFilter001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        SetHostAttributes();
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::TOP), MakeFourContents());
    });
    FlushUITasks(host);
    auto json = JsonUtil::Create(true);
    InspectorFilter all;
    host->ToJsonValue(json, all);
    EXPECT_TRUE(json->Contains("safeAreaInset"));
    EXPECT_TRUE(json->Contains("safeAreaPadding"));
    EXPECT_FALSE(json->Contains("SafeAreaInsetContainerNodeId"));
    EXPECT_FALSE(json->Contains("EffectiveSafeAreaContribution"));
    EXPECT_FALSE(json->Contains("SafeAreaInsetContribution"));
    InspectorFilter fixed;
    fixed.AddFilterAttr("id");
    auto fixedJson = JsonUtil::Create(true);
    host->ToJsonValue(fixedJson, fixed);
    EXPECT_FALSE(fixedJson->Contains("safeAreaInset"));
    InspectorFilter unrelated;
    unrelated.AddFilterAttr("safeAreaPadding");
    auto unrelatedJson = JsonUtil::Create(true);
    host->ToJsonValue(unrelatedJson, unrelated);
    EXPECT_FALSE(unrelatedJson->Contains("safeAreaInset"));
    auto carrierJson = JsonUtil::Create(true);
    host->GetSafeAreaInsetNode()->ToJsonValue(carrierJson, all);
    EXPECT_FALSE(carrierJson->Contains("safeAreaInset"));
    // GetInspector walks the stage's last page; mount the host under one (mirrors the real tree).
    auto stageNode = GetStageNode();
    ASSERT_NE(stageNode, nullptr);
    auto pageNode = FrameNode::CreateFrameNode("PageDump", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
    ASSERT_NE(pageNode, nullptr);
    stageNode->AddChild(pageNode);
    pageNode->AddChild(host);
    auto tree = Inspector::GetInspector();
    EXPECT_NE(tree.find("safeAreaInset"), std::string::npos);
    EXPECT_EQ(tree.find("SafeAreaInsetContainer"), std::string::npos);
    SafeAreaInsetView::ResetSafeAreaInset(AceType::RawPtr(host));
    auto resetJson = JsonUtil::Create(true);
    host->ToJsonValue(resetJson, all);
    EXPECT_FALSE(resetJson->Contains("safeAreaInset"));
}

/**
 * @tc.name: SafeAreaInsetDumpTreeLookup001
 * @tc.desc: Both dump formats preserve the real carrier hierarchy and the same fields in ID lookup.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetDumpTreeLookup001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        SetHostAttributes();
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::TOP), MakeFourContents());
    });
    FlushUITasks(host);
    auto carrier = host->GetSafeAreaInsetNode();
    auto content = GetInsetPattern(host)->GetEdgeContent(SafeAreaInsetEdge::TOP);
    for (bool json : { false, true }) {
        auto tree = CaptureInsetDump([&]() { host->DumpTree(0, json); });
        EXPECT_NE(tree.find("SafeAreaInsetContainer"), std::string::npos);
        EXPECT_NE(tree.find("SafeAreaInsetContainerNodeId"), std::string::npos);
        EXPECT_NE(tree.find("contentNodeId"), std::string::npos);
        auto ownerDump = CaptureInsetDump([&]() {
            EXPECT_TRUE(host->DumpTreeById(0, std::to_string(host->GetId()), json));
        });
        EXPECT_NE(ownerDump.find("EffectiveSafeAreaContribution"), std::string::npos);
        auto carrierDump = CaptureInsetDump([&]() {
            EXPECT_TRUE(host->DumpTreeById(0, std::to_string(carrier->GetId()), json));
        });
        EXPECT_NE(carrierDump.find("SafeAreaInsetContribution"), std::string::npos);
        auto childDump = CaptureInsetDump([&]() {
            EXPECT_TRUE(host->DumpTreeById(0, std::to_string(content->GetId()), json));
        });
        EXPECT_EQ(childDump.find("SafeAreaInsetContribution"), std::string::npos);
    }
}

/**
 * @tc.name: SafeAreaInsetDumpSaturationAndClone001
 * @tc.desc: Dump reports the cached truncated result and preserves cloned cache ownership.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetDumpSaturationAndClone001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        SetHostAttributes();
        ViewAbstract::SetWidth(CalcLength(100.0f, DimensionUnit::PX));
        SafeAreaInsetOptions options;
        options.GetEdge(SafeAreaInsetEdge::START) = MakeEdgeOptions();
        options.GetEdge(SafeAreaInsetEdge::END) = MakeEdgeOptions();
        SafeAreaInsetContentArray contents;
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::START)] = MakeContent("startInset", 50.0f, 20.0f);
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::END)] = MakeContent("endInset", 50.0f, 20.0f);
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(), options, contents);
    });
    FlushUITasks(host);
    auto carrier = host->GetSafeAreaInsetNode();
    EXPECT_EQ(GetInsetDump(carrier)->GetString("SafeAreaInsetContribution"), "[56.00,56.00,NA,NA]");
    EXPECT_EQ(GetInsetDump(host)->GetString("EffectiveSafeAreaContribution"),
        "[56.00,24.00,8.00,8.00]");
    auto geometry = host->GetGeometryNode();
    auto clone = geometry->Clone();
    ASSERT_NE(clone->GetResolvedSingleSafeAreaPadding(), nullptr);
    ASSERT_NE(geometry->GetResolvedSingleSafeAreaPadding(), nullptr);
    EXPECT_NE(clone->GetResolvedSingleSafeAreaPadding().get(), geometry->GetResolvedSingleSafeAreaPadding().get());
    EXPECT_EQ(*clone->GetResolvedSingleSafeAreaPadding(), *geometry->GetResolvedSingleSafeAreaPadding());
    clone->Reset();
    EXPECT_NE(geometry->GetResolvedSingleSafeAreaPadding(), nullptr);

    host->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(200.0f, DimensionUnit::PX), CalcLength(HOST_HEIGHT, DimensionUnit::PX)));
    EXPECT_FALSE(GetInsetDump(host)->Contains("EffectiveSafeAreaContribution"));
    FlushUITasks(host);
    EXPECT_EQ(GetInsetDump(host)->GetString("EffectiveSafeAreaContribution"),
        "[56.00,56.00,8.00,8.00]");
}

/**
 * @tc.name: SafeAreaInsetDumpZeroAndMissingContent001
 * @tc.desc: A valid zero contribution is distinguishable from an absent result or dropped content.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetDumpZeroAndMissingContent001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        SetHostAttributes(false);
        auto options = MakeOptions(SafeAreaInsetEdge::TOP, SafeAreaInsetAlignment::CENTER, 0.0f);
        options.GetEdge(SafeAreaInsetEdge::BOTTOM) = MakeEdgeOptions();
        SafeAreaInsetContentArray contents;
        contents[0] = MakeContent("zeroInset", 0.0f, 0.0f);
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(), options, contents);
    });
    FlushUITasks(host);
    auto carrier = host->GetSafeAreaInsetNode();
    auto dump = GetInsetDump(carrier);
    EXPECT_TRUE(dump->GetValue("SafeAreaInset")->Contains("top"));
    EXPECT_FALSE(dump->GetValue("SafeAreaInset")->Contains("bottom"));
    EXPECT_EQ(dump->GetString("SafeAreaInsetContribution"), "[NA,NA,0.00,NA]");
    EXPECT_TRUE(GetInsetDump(host)->Contains("EffectiveSafeAreaContribution"));
    SafeAreaInsetView::ResetSafeAreaInset(AceType::RawPtr(host));
    EXPECT_FALSE(GetInsetDump(host)->Contains("EffectiveSafeAreaContribution"));
    EXPECT_FALSE(GetInsetDump(carrier)->Contains("SafeAreaInsetContribution"));
}

/**
 * @tc.name: SafeAreaInsetFocusWholePath001
 * @tc.desc: Inset content passes whole-path focus admission through the carrier scope and gains focus.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetFocusWholePath001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        SetHostAttributes();
        SafeAreaInsetContentArray contents;
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::TOP)] =
            MakeFocusableContent("insetInput", FocusType::NODE, true);
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::TOP), contents);
    });
    // Mount the tree: focus scope admission and switching rely on on-main-tree children.
    MountToStageNode(host);
    FlushUITasks(host);
    auto carrier = host->GetSafeAreaInsetNode();
    ASSERT_NE(carrier, nullptr);
    auto pattern = GetInsetPattern(host);
    auto input = AceType::DynamicCast<FrameNode>(pattern->GetEdgeContent(SafeAreaInsetEdge::TOP));
    ASSERT_NE(input, nullptr);
    // The carrier is the nearest focus scope ancestor of the inset content.
    auto focusParent = input->GetFocusParentWithBoundary();
    ASSERT_NE(focusParent, nullptr);
    EXPECT_EQ(focusParent->GetTag(), SAFE_AREA_INSET_CONTAINER_TAG);
    // The carrier acts as a focus scope that admits content requests instead of rejecting them.
    auto carrierHub = carrier->GetFocusHub();
    ASSERT_NE(carrierHub, nullptr);
    EXPECT_EQ(carrierHub->GetFocusType(), FocusType::SCOPE);
    // Whole-path admission and the actual request both succeed for inset content.
    auto inputHub = input->GetFocusHub();
    ASSERT_NE(inputHub, nullptr);
    EXPECT_TRUE(inputHub->IsFocusableWholePath());
    EXPECT_TRUE(inputHub->RequestFocusImmediately());
    EXPECT_TRUE(inputHub->IsCurrentFocus());
    RemoveFromStageNode();
}

/**
 * @tc.name: SafeAreaInsetFocusDisabledHost001
 * @tc.desc: A disabled or non-focusable host still blocks inset content focus through ancestor admission.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetFocusDisabledHost001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        SetHostAttributes();
        SafeAreaInsetContentArray contents;
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::TOP)] =
            MakeFocusableContent("insetInput", FocusType::NODE, true);
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::TOP), contents);
    });
    // Mount the tree: focus scope admission and switching rely on on-main-tree children.
    MountToStageNode(host);
    FlushUITasks(host);
    auto pattern = GetInsetPattern(host);
    auto input = AceType::DynamicCast<FrameNode>(pattern->GetEdgeContent(SafeAreaInsetEdge::TOP));
    ASSERT_NE(input, nullptr);
    auto inputHub = input->GetFocusHub();
    ASSERT_NE(inputHub, nullptr);
    // Host disabled by the developer keeps blocking the request.
    host->GetEventHub<EventHub>()->SetEnabled(false);
    EXPECT_FALSE(inputHub->IsFocusableWholePath());
    EXPECT_FALSE(inputHub->RequestFocusImmediately());
    EXPECT_FALSE(inputHub->IsCurrentFocus());
    // Restoring the host restores admission.
    host->GetEventHub<EventHub>()->SetEnabled(true);
    EXPECT_TRUE(inputHub->RequestFocusImmediately());
    EXPECT_TRUE(inputHub->IsCurrentFocus());
    RemoveFromStageNode();
}

/**
 * @tc.name: SafeAreaInsetFocusOrdinaryChild001
 * @tc.desc: Ordinary focusable children of the host keep working and switch focus with inset content.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetFocusOrdinaryChild001, TestSize.Level0)
{
    RefPtr<FrameNode> ordinary;
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes();
        ordinary = MakeFocusableContent("ordinaryChild", FocusType::NODE, true);
        SafeAreaInsetContentArray contents;
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::TOP)] =
            MakeFocusableContent("insetInput", FocusType::NODE, true);
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::TOP), contents);
    });
    host->AddChild(ordinary);
    // Mount the tree: focus scope admission and switching rely on on-main-tree children.
    MountToStageNode(host);
    FlushUITasks(host);
    ASSERT_NE(ordinary, nullptr);
    auto pattern = GetInsetPattern(host);
    auto input = AceType::DynamicCast<FrameNode>(pattern->GetEdgeContent(SafeAreaInsetEdge::TOP));
    ASSERT_NE(input, nullptr);
    auto ordinaryHub = ordinary->GetFocusHub();
    auto inputHub = input->GetFocusHub();
    ASSERT_NE(ordinaryHub, nullptr);
    ASSERT_NE(inputHub, nullptr);
    // Ordinary children are unaffected by the carrier scope.
    EXPECT_TRUE(ordinaryHub->RequestFocusImmediately());
    EXPECT_TRUE(ordinaryHub->IsCurrentFocus());
    // Switch from the ordinary child into inset content.
    EXPECT_TRUE(inputHub->RequestFocusImmediately());
    EXPECT_TRUE(inputHub->IsCurrentFocus());
    EXPECT_FALSE(ordinaryHub->IsCurrentFocus());
    // Switch back from inset content to the ordinary child.
    EXPECT_TRUE(ordinaryHub->RequestFocusImmediately());
    EXPECT_TRUE(ordinaryHub->IsCurrentFocus());
    EXPECT_FALSE(inputHub->IsCurrentFocus());
    RemoveFromStageNode();
}

/**
 * @tc.name: SafeAreaInsetFocusEventCallback001
 * @tc.desc: Focus/blur internal callbacks (the production wiring of TextInput onFocus/onBlur,
 *           text_field_pattern.cpp InitFocusEvent) fire on every acquisition/loss cycle between
 *           inset content and a node outside the inset host. Guards the device finding where the
 *           first switch-away loses the blur callback and later re-focus events go silent.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetFocusEventCallback001, TestSize.Level0)
{
    RefPtr<FrameNode> ordinary;
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes();
        ordinary = MakeFocusableContent("ordinaryChild", FocusType::NODE, true);
        SafeAreaInsetContentArray contents;
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::TOP)] =
            MakeFocusableContent("insetInput", FocusType::NODE, true);
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::TOP), contents);
    });
    host->AddChild(ordinary);
    MountToStageNode(host);
    FlushUITasks(host);
    ASSERT_NE(ordinary, nullptr);
    auto pattern = GetInsetPattern(host);
    auto input = AceType::DynamicCast<FrameNode>(pattern->GetEdgeContent(SafeAreaInsetEdge::TOP));
    ASSERT_NE(input, nullptr);
    auto ordinaryHub = ordinary->GetFocusHub();
    auto inputHub = input->GetFocusHub();
    ASSERT_NE(ordinaryHub, nullptr);
    ASSERT_NE(inputHub, nullptr);

    int32_t insetFocus = 0;
    int32_t insetBlur = 0;
    int32_t ordinaryFocus = 0;
    int32_t ordinaryBlur = 0;
    inputHub->SetOnFocusInternal([&insetFocus](FocusReason) { ++insetFocus; });
    inputHub->SetOnBlurInternal([&insetBlur]() { ++insetBlur; });
    ordinaryHub->SetOnFocusInternal([&ordinaryFocus](FocusReason) { ++ordinaryFocus; });
    ordinaryHub->SetOnBlurInternal([&ordinaryBlur]() { ++ordinaryBlur; });

    // First focus of the inset content fires its focus callback.
    EXPECT_TRUE(inputHub->RequestFocusImmediately());
    EXPECT_TRUE(inputHub->IsCurrentFocus());
    FlushUITasks(host);
    EXPECT_EQ(insetFocus, 1);
    EXPECT_EQ(insetBlur, 0);

    // Switching out to the ordinary node must blur the inset content.
    EXPECT_TRUE(ordinaryHub->RequestFocusImmediately());
    EXPECT_TRUE(ordinaryHub->IsCurrentFocus());
    EXPECT_FALSE(inputHub->IsCurrentFocus());
    FlushUITasks(host);
    EXPECT_EQ(ordinaryFocus, 1);
    EXPECT_EQ(insetBlur, 1);

    // Re-focusing the inset content fires its focus callback again.
    EXPECT_TRUE(inputHub->RequestFocusImmediately());
    EXPECT_TRUE(inputHub->IsCurrentFocus());
    EXPECT_FALSE(ordinaryHub->IsCurrentFocus());
    FlushUITasks(host);
    EXPECT_EQ(insetFocus, 2);
    EXPECT_EQ(ordinaryBlur, 1);
    RemoveFromStageNode();
}

/**
 * @tc.name: SafeAreaInsetFocusEventCallbackNoFlush001
 * @tc.desc: Same as EventCallback001 but without inter-step layout flushes, isolating the trigger.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetFocusEventCallbackNoFlush001, TestSize.Level0)
{
    RefPtr<FrameNode> ordinary;
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes();
        ordinary = MakeFocusableContent("ordinaryChild", FocusType::NODE, true);
        SafeAreaInsetContentArray contents;
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::TOP)] =
            MakeFocusableContent("insetInput", FocusType::NODE, true);
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::TOP), contents);
    });
    host->AddChild(ordinary);
    MountToStageNode(host);
    FlushUITasks(host);
    auto pattern = GetInsetPattern(host);
    auto input = AceType::DynamicCast<FrameNode>(pattern->GetEdgeContent(SafeAreaInsetEdge::TOP));
    auto ordinaryHub = ordinary->GetFocusHub();
    auto inputHub = input->GetFocusHub();
    int32_t insetFocus = 0;
    int32_t insetBlur = 0;
    int32_t ordinaryFocus = 0;
    int32_t ordinaryBlur = 0;
    inputHub->SetOnFocusInternal([&insetFocus](FocusReason) { ++insetFocus; });
    inputHub->SetOnBlurInternal([&insetBlur]() { ++insetBlur; });
    ordinaryHub->SetOnFocusInternal([&ordinaryFocus](FocusReason) { ++ordinaryFocus; });
    ordinaryHub->SetOnBlurInternal([&ordinaryBlur]() { ++ordinaryBlur; });

    EXPECT_TRUE(inputHub->RequestFocusImmediately());
    EXPECT_EQ(insetFocus, 1);
    EXPECT_TRUE(ordinaryHub->RequestFocusImmediately());
    EXPECT_EQ(ordinaryFocus, 1);
    EXPECT_FALSE(inputHub->IsCurrentFocus());
    EXPECT_EQ(insetBlur, 1);
    EXPECT_TRUE(inputHub->RequestFocusImmediately());
    EXPECT_EQ(insetFocus, 2);
    EXPECT_FALSE(ordinaryHub->IsCurrentFocus());
    EXPECT_EQ(ordinaryBlur, 1);
    RemoveFromStageNode();
}

/**
 * @tc.name: SafeAreaInsetFocusEventCallbackOrdinaryFirst001
 * @tc.desc: Ordinary-first order with flushes and callbacks, isolating the order dependency.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetFocusEventCallbackOrdinaryFirst001, TestSize.Level0)
{
    RefPtr<FrameNode> ordinary;
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes();
        ordinary = MakeFocusableContent("ordinaryChild", FocusType::NODE, true);
        SafeAreaInsetContentArray contents;
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::TOP)] =
            MakeFocusableContent("insetInput", FocusType::NODE, true);
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::TOP), contents);
    });
    host->AddChild(ordinary);
    MountToStageNode(host);
    FlushUITasks(host);
    auto pattern = GetInsetPattern(host);
    auto input = AceType::DynamicCast<FrameNode>(pattern->GetEdgeContent(SafeAreaInsetEdge::TOP));
    auto ordinaryHub = ordinary->GetFocusHub();
    auto inputHub = input->GetFocusHub();
    int32_t insetFocus = 0;
    int32_t insetBlur = 0;
    int32_t ordinaryFocus = 0;
    int32_t ordinaryBlur = 0;
    inputHub->SetOnFocusInternal([&insetFocus](FocusReason) { ++insetFocus; });
    inputHub->SetOnBlurInternal([&insetBlur]() { ++insetBlur; });
    ordinaryHub->SetOnFocusInternal([&ordinaryFocus](FocusReason) { ++ordinaryFocus; });
    ordinaryHub->SetOnBlurInternal([&ordinaryBlur]() { ++ordinaryBlur; });

    EXPECT_TRUE(ordinaryHub->RequestFocusImmediately());
    FlushUITasks(host);
    EXPECT_EQ(ordinaryFocus, 1);
    EXPECT_TRUE(inputHub->RequestFocusImmediately());
    FlushUITasks(host);
    EXPECT_EQ(insetFocus, 1);
    EXPECT_EQ(ordinaryBlur, 1);
    EXPECT_TRUE(ordinaryHub->RequestFocusImmediately());
    FlushUITasks(host);
    EXPECT_EQ(ordinaryFocus, 2);
    EXPECT_EQ(insetBlur, 1);
    EXPECT_FALSE(inputHub->IsCurrentFocus());
    RemoveFromStageNode();
}

/**
 * @tc.name: SafeAreaInsetFocusEventCallbackDiag001
 * @tc.desc: Dumps the focus-hub chain state after each step to localize the stale hub.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetFocusEventCallbackDiag001, TestSize.Level0)
{
    RefPtr<FrameNode> ordinary;
    auto host = CreateStack([&](StackModelNG) {
        SetHostAttributes();
        ordinary = MakeFocusableContent("ordinaryChild", FocusType::NODE, true);
        SafeAreaInsetContentArray contents;
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::TOP)] =
            MakeFocusableContent("insetInput", FocusType::NODE, true);
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::TOP), contents);
    });
    host->AddChild(ordinary);
    MountToStageNode(host);
    FlushUITasks(host);
    auto pattern = GetInsetPattern(host);
    auto input = AceType::DynamicCast<FrameNode>(pattern->GetEdgeContent(SafeAreaInsetEdge::TOP));
    auto ordinaryHub = ordinary->GetFocusHub();
    auto inputHub = input->GetFocusHub();
    auto carrierHub = host->GetSafeAreaInsetNode()->GetFocusHub();
    auto hostHub = host->GetFocusHub();
    auto inputParent = inputHub->GetParentFocusHub();
    auto carrierParent = carrierHub->GetParentFocusHub();
    auto rootHub = hostHub->GetParentFocusHub();
    GTEST_LOG_(INFO) << "inputParent=" << (inputParent ? inputParent->GetFrameName() : "null")
                     << " carrierParent=" << (carrierParent ? carrierParent->GetFrameName() : "null")
                     << " inputOnTree=" << input->IsOnMainTree()
                     << " carrierOnTree=" << host->GetSafeAreaInsetNode()->IsOnMainTree();
    {
        auto mHostLast = hostHub->GetLastWeakFocusNode().Upgrade();
        auto mCarrierLast = carrierHub->GetLastWeakFocusNode().Upgrade();
        GTEST_LOG_(INFO) << "MOUNT STATE: inputCur=" << inputHub->IsCurrentFocus()
                         << " ordinaryCur=" << ordinaryHub->IsCurrentFocus()
                         << " carrierCur=" << carrierHub->IsCurrentFocus()
                         << " hostCur=" << hostHub->IsCurrentFocus()
                         << " rootCur=" << (rootHub ? rootHub->IsCurrentFocus() : -1)
                         << " hostLast=" << (mHostLast ? mHostLast->GetFrameName() : "null")
                         << " carrierLast=" << (mCarrierLast ? mCarrierLast->GetFrameName() : "null");
    }

    EXPECT_TRUE(inputHub->RequestFocusImmediately());
    auto lastOnCarrier = carrierHub->GetLastWeakFocusNode().Upgrade();
    auto lastOnHost = hostHub->GetLastWeakFocusNode().Upgrade();
    GTEST_LOG_(INFO) << "after input focus: inputCur=" << inputHub->IsCurrentFocus()
                     << " carrierCur=" << carrierHub->IsCurrentFocus()
                     << " hostCur=" << hostHub->IsCurrentFocus()
                     << " carrierLast=" << (lastOnCarrier ? lastOnCarrier->GetFrameName() : "null")
                     << " hostLast=" << (lastOnHost ? lastOnHost->GetFrameName() : "null");

    EXPECT_TRUE(ordinaryHub->RequestFocusImmediately());
    lastOnCarrier = carrierHub->GetLastWeakFocusNode().Upgrade();
    lastOnHost = hostHub->GetLastWeakFocusNode().Upgrade();
    GTEST_LOG_(INFO) << "after ordinary focus: inputCur=" << inputHub->IsCurrentFocus()
                     << " ordinaryCur=" << ordinaryHub->IsCurrentFocus()
                     << " carrierCur=" << carrierHub->IsCurrentFocus()
                     << " hostCur=" << hostHub->IsCurrentFocus()
                     << " carrierLast=" << (lastOnCarrier ? lastOnCarrier->GetFrameName() : "null")
                     << " hostLast=" << (lastOnHost ? lastOnHost->GetFrameName() : "null");

    EXPECT_TRUE(inputHub->RequestFocusImmediately());
    GTEST_LOG_(INFO) << "after input refocus: inputCur=" << inputHub->IsCurrentFocus()
                     << " ordinaryCur=" << ordinaryHub->IsCurrentFocus()
                     << " carrierCur=" << carrierHub->IsCurrentFocus()
                     << " hostCur=" << hostHub->IsCurrentFocus();
    RemoveFromStageNode();
}

/**
 * @tc.name: SafeAreaInsetFocusSwitchBetweenEdges001
 * @tc.desc: Focus switches between contents of two different edges without stale state.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetFocusSwitchBetweenEdges001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        SetHostAttributes();
        SafeAreaInsetContentArray contents;
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::TOP)] =
            MakeFocusableContent("topInput", FocusType::NODE, true);
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::BOTTOM)] =
            MakeFocusableContent("bottomInput", FocusType::NODE, true);
        auto options = MakeOptions(SafeAreaInsetEdge::TOP);
        options.GetEdge(SafeAreaInsetEdge::BOTTOM) = MakeEdgeOptions();
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(), options, contents);
    });
    // Mount the tree: focus scope admission and switching rely on on-main-tree children.
    MountToStageNode(host);
    FlushUITasks(host);
    auto pattern = GetInsetPattern(host);
    auto topInput = AceType::DynamicCast<FrameNode>(pattern->GetEdgeContent(SafeAreaInsetEdge::TOP));
    auto bottomInput = AceType::DynamicCast<FrameNode>(pattern->GetEdgeContent(SafeAreaInsetEdge::BOTTOM));
    ASSERT_NE(topInput, nullptr);
    ASSERT_NE(bottomInput, nullptr);
    auto topHub = topInput->GetFocusHub();
    auto bottomHub = bottomInput->GetFocusHub();
    ASSERT_NE(topHub, nullptr);
    ASSERT_NE(bottomHub, nullptr);
    auto carrierHub = host->GetSafeAreaInsetNode()->GetFocusHub();
    ASSERT_NE(carrierHub, nullptr);
    EXPECT_TRUE(topHub->RequestFocusImmediately());
    EXPECT_TRUE(topHub->IsCurrentFocus());
    // The carrier scope itself becomes current so later switches blur the previous child.
    EXPECT_TRUE(carrierHub->IsCurrentFocus());
    EXPECT_TRUE(bottomHub->RequestFocusImmediately());
    EXPECT_TRUE(bottomHub->IsCurrentFocus());
    EXPECT_FALSE(topHub->IsCurrentFocus());
    EXPECT_TRUE(carrierHub->IsCurrentFocus());
    EXPECT_TRUE(topHub->RequestFocusImmediately());
    EXPECT_TRUE(topHub->IsCurrentFocus());
    EXPECT_FALSE(bottomHub->IsCurrentFocus());
    RemoveFromStageNode();
}

/**
 * @tc.name: SafeAreaInsetFocusResetAndRebuild001
 * @tc.desc: Resetting the inset clears the focused content state and a rebuilt inset is focusable again.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetFocusResetAndRebuild001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        SetHostAttributes();
        SafeAreaInsetContentArray contents;
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::TOP)] =
            MakeFocusableContent("insetInput", FocusType::NODE, true);
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::TOP), contents);
    });
    // Mount the tree so the reset exercises the real detach/blur cleanup path.
    MountToStageNode(host);
    FlushUITasks(host);
    auto pattern = GetInsetPattern(host);
    auto input = AceType::DynamicCast<FrameNode>(pattern->GetEdgeContent(SafeAreaInsetEdge::TOP));
    ASSERT_NE(input, nullptr);
    EXPECT_TRUE(input->IsOnMainTree());
    auto inputHub = input->GetFocusHub();
    ASSERT_NE(inputHub, nullptr);
    EXPECT_TRUE(inputHub->RequestFocusImmediately());
    EXPECT_TRUE(inputHub->IsCurrentFocus());
    // Reset detaches the carrier; the previously focused content must not keep focus state.
    SafeAreaInsetView::ResetSafeAreaInset(AceType::RawPtr(host));
    EXPECT_EQ(host->GetSafeAreaInsetNode(), nullptr);
    EXPECT_FALSE(input->IsOnMainTree());
    EXPECT_FALSE(inputHub->IsCurrentFocus());
    // Rebuilding the inset with new content restores focusability.
    SafeAreaInsetContentArray contents;
    contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::TOP)] =
        MakeFocusableContent("reboundInput", FocusType::NODE, true);
    SafeAreaInsetView::SetSafeAreaInset(AceType::RawPtr(host), MakeOptions(SafeAreaInsetEdge::TOP), contents);
    FlushUITasks(host);
    auto rebuilt = AceType::DynamicCast<FrameNode>(GetInsetPattern(host)->GetEdgeContent(SafeAreaInsetEdge::TOP));
    ASSERT_NE(rebuilt, nullptr);
    auto rebuiltHub = rebuilt->GetFocusHub();
    ASSERT_NE(rebuiltHub, nullptr);
    EXPECT_TRUE(rebuiltHub->RequestFocusImmediately());
    EXPECT_TRUE(rebuiltHub->IsCurrentFocus());
    RemoveFromStageNode();
}

/**
 * @tc.name: SafeAreaInsetFocusTabTraversal001
 * @tc.desc: Documents the independent downward traversal gap: the host focus scope does not enumerate the
 *           carrier field, so Tab/default focus cannot enter inset content until focus-tree traversal
 *           includes SafeAreaInsetNode children. Upward requests are not affected by this gap.
 * @tc.type: FUNC
 */
HWTEST_F(SafeAreaInsetTestNg, SafeAreaInsetFocusTabTraversal001, TestSize.Level0)
{
    auto host = CreateStack([](StackModelNG) {
        SetHostAttributes();
        SafeAreaInsetContentArray contents;
        contents[SafeAreaInsetOptions::ToIndex(SafeAreaInsetEdge::TOP)] =
            MakeFocusableContent("insetInput", FocusType::NODE, true);
        SafeAreaInsetView::SetSafeAreaInset(ViewStackProcessor::GetInstance()->GetMainFrameNode(),
            MakeOptions(SafeAreaInsetEdge::TOP), contents);
    });
    // Mount the tree so traversal sees the same on-main-tree state as a real page.
    MountToStageNode(host);
    FlushUITasks(host);
    auto carrier = host->GetSafeAreaInsetNode();
    ASSERT_NE(carrier, nullptr);
    EXPECT_TRUE(carrier->IsOnMainTree());
    auto pattern = GetInsetPattern(host);
    auto input = AceType::DynamicCast<FrameNode>(pattern->GetEdgeContent(SafeAreaInsetEdge::TOP));
    ASSERT_NE(input, nullptr);
    EXPECT_TRUE(input->IsOnMainTree());
    auto inputHub = input->GetFocusHub();
    ASSERT_NE(inputHub, nullptr);
    auto inputId = inputHub->GetFrameId();
    // Current known gap: downward enumeration from the host skips the carrier field.
    auto hostHub = host->GetFocusHub();
    ASSERT_NE(hostHub, nullptr);
    EXPECT_FALSE(hostHub->AnyChildFocusHub(
        [inputId](const RefPtr<FocusHub>& focusHub) { return focusHub->GetFrameId() == inputId; }));
    // From the carrier itself the content is reachable, so once traversal includes the carrier
    // field the content becomes discoverable without structural changes.
    auto carrierHub = carrier->GetFocusHub();
    ASSERT_NE(carrierHub, nullptr);
    EXPECT_TRUE(carrierHub->AnyChildFocusHub(
        [inputId](const RefPtr<FocusHub>& focusHub) { return focusHub->GetFrameId() == inputId; }));
    RemoveFromStageNode();
}
} // namespace OHOS::Ace::NG
