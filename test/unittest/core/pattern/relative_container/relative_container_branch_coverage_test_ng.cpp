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

#include <algorithm>
#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#define protected public
#define private public
#include "test/unittest/core/pattern/relative_container/relative_container_base_test_ng.h"

#include "core/components/common/properties/state_attributes.h"
#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

namespace OHOS::Ace::NG {
using namespace testing;
using namespace testing::ext;

namespace {
constexpr float CONTAINER_WIDTH = 300.0f;
constexpr float CONTAINER_HEIGHT = 300.0f;
const std::string CONTAINER_ID = "__container__";
const std::string ROW1 = "row1";
const std::string ROW2 = "row2";
const std::string ROW3 = "row3";
const std::string BARRIER1 = "barrier1";
const std::string BARRIER2 = "barrier2";
const std::string GUIDELINE_H = "guidelineH";
const std::string GUIDELINE_V = "guidelineV";
const char* CONFIG_CHANGE_KEY = "relativeContainer.guideLine";
} // namespace

class RelativeContainerCoverageTestNG : public RelativeContainerBaseTestNG {
public:
    static void SetUpTestSuite()
    {
        MockPipelineContext::SetUp();
        auto pipeline = PipelineContext::GetCurrentContext();
        if (pipeline) {
            pipeline->SetMinPlatformVersion(11);
        }
    }

    static void TearDownTestSuite()
    {
        TestNG::TearDownTestSuite();
    }

    void SetUp() override
    {
        auto pipeline = PipelineContext::GetCurrentContext();
        if (pipeline) {
            pipeline->SetMinPlatformVersion(11);
        }
    }

    void TearDown() override {}

    static std::map<AlignDirection, AlignRule> HorizontalRule(
        AlignDirection direction, const std::string& anchor, HorizontalAlign align)
    {
        std::map<AlignDirection, AlignRule> alignRules;
        AddAlignRule(alignRules, direction, anchor, align);
        return alignRules;
    }

    static std::map<AlignDirection, AlignRule> VerticalRule(
        AlignDirection direction, const std::string& anchor, VerticalAlign align)
    {
        std::map<AlignDirection, AlignRule> alignRules;
        AddAlignRule(alignRules, direction, anchor, align);
        return alignRules;
    }

    static void UpdateConstraint(const RefPtr<FrameNode>& container, float width = CONTAINER_WIDTH,
        float height = CONTAINER_HEIGHT, bool withSelfIdealSize = false)
    {
        LayoutConstraintF constraint;
        constraint.percentReference = { width, height };
        constraint.parentIdealSize = { width, height };
        if (withSelfIdealSize) {
            constraint.selfIdealSize = OptionalSizeF(width, height);
        }
        container->GetLayoutProperty()->UpdateLayoutConstraint(constraint);
        container->GetLayoutProperty()->UpdateContentConstraint();
    }

    static void PrepareAlgorithm(
        const RefPtr<FrameNode>& container, RelativeContainerLayoutAlgorithm& algorithm, bool measureChild = false)
    {
        algorithm.versionGreatorOrEqualToEleven_ = true;
        UpdateConstraint(container);
        PrepareMeasureChild(Referenced::RawPtr(container), algorithm);
        if (measureChild) {
            algorithm.MeasureChild(Referenced::RawPtr(container));
        }
    }

    static void AddBarrier(std::vector<BarrierInfo>& barrierInfos, const std::string& id,
        const BarrierDirection& direction, const std::vector<std::string>& referencedId)
    {
        BarrierInfo info = { .id = id, .direction = direction, .referencedId = referencedId };
        barrierInfos.emplace_back(info);
    }

    RefPtr<FrameNode> CreateRowNode(const std::string& id, float width, float height,
        const std::map<AlignDirection, AlignRule>& alignRules = {})
    {
        return CreateRow([id, width, height, alignRules](RowModelNG rowModel) {
            ViewAbstract::SetInspectorId(id);
            ViewAbstract::SetWidth(CalcLength(width, DimensionUnit::PX));
            ViewAbstract::SetHeight(CalcLength(height, DimensionUnit::PX));
            if (!alignRules.empty()) {
                ViewAbstract::SetAlignRules(alignRules);
            }
        });
    }

    RefPtr<FrameNode> CreateChainRowNode(const std::string& id, float width, float height,
        const std::map<AlignDirection, AlignRule>& alignRules, const ChainInfo& chainInfo,
        const std::optional<ChainWeightPair>& chainWeight = std::nullopt)
    {
        return CreateRow([id, width, height, alignRules, chainInfo, chainWeight](RowModelNG rowModel) {
            ViewAbstract::SetInspectorId(id);
            ViewAbstract::SetWidth(CalcLength(width, DimensionUnit::PX));
            ViewAbstract::SetHeight(CalcLength(height, DimensionUnit::PX));
            ViewAbstract::SetAlignRules(alignRules);
            ViewAbstract::SetChainStyle(chainInfo);
            if (chainWeight.has_value()) {
                ViewAbstract::SetChainWeight(chainWeight.value());
            }
        });
    }
};

/**
 * @tc.name: ModelStaticNullApiTest
 * @tc.desc: Test model static interfaces with a null frame node and the config change switch off and on.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, ModelStaticNullApiTest, TestSize.Level1)
{
    bool store = g_isConfigChangePerform;
    g_isConfigChangePerform = false;
    std::vector<BarrierInfo> barriers;
    AddBarrier(barriers, BARRIER1, BarrierDirection::RIGHT, { ROW1 });
    std::vector<GuidelineInfo> guidelines;
    AddGuidelineStart(guidelines, GUIDELINE_H, LineDirection::HORIZONTAL, Dimension(10.0f));
    RelativeContainerModelNG::SetBarrier(nullptr, barriers);
    RelativeContainerModelNG::SetGuideline(nullptr, guidelines);
    RelativeContainerModelNG::ResetResObj(nullptr, CONFIG_CHANGE_KEY);
    EXPECT_TRUE(RelativeContainerModelNG::GetBarrier(nullptr).empty());
    EXPECT_TRUE(RelativeContainerModelNG::GetGuideline(nullptr).empty());
    RelativeContainerModelNG::ResetGuideline(nullptr);
    RelativeContainerModelNG::ResetBarrier(nullptr);
    g_isConfigChangePerform = true;
    EXPECT_TRUE(SystemProperties::ConfigChangePerform());
    RelativeContainerModelNG::ResetResObj(nullptr, CONFIG_CHANGE_KEY);
    RelativeContainerModelNG::SetGuideline(nullptr, guidelines);
    g_isConfigChangePerform = store;
}

/**
 * @tc.name: ModelStaticFrameNodeApiTest
 * @tc.desc: Test model static barrier/guideline set-get-reset on a real frame node created by CreateFrameNode.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, ModelStaticFrameNodeApiTest, TestSize.Level1)
{
    bool store = g_isConfigChangePerform;
    g_isConfigChangePerform = false;
    auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
    auto frameNode = RelativeContainerModelNG::CreateFrameNode(nodeId);
    ASSERT_NE(frameNode, nullptr);
    auto rawNode = AceType::RawPtr(frameNode);
    std::vector<GuidelineInfo> guidelines;
    AddGuidelineStart(guidelines, GUIDELINE_H, LineDirection::HORIZONTAL, Dimension(10.0f));
    AddGuidelineStart(guidelines, GUIDELINE_V, LineDirection::VERTICAL, Dimension(20.0f));
    RelativeContainerModelNG::SetGuideline(rawNode, guidelines);
    auto gotGuidelines = RelativeContainerModelNG::GetGuideline(rawNode);
    ASSERT_EQ(gotGuidelines.size(), 2u);
    EXPECT_EQ(gotGuidelines[0].id, GUIDELINE_H);
    std::vector<BarrierInfo> barriers;
    AddBarrier(barriers, BARRIER1, BarrierDirection::RIGHT, { ROW1 });
    RelativeContainerModelNG::SetBarrier(rawNode, barriers);
    auto gotBarriers = RelativeContainerModelNG::GetBarrier(rawNode);
    ASSERT_EQ(gotBarriers.size(), 1u);
    EXPECT_EQ(gotBarriers[0].id, BARRIER1);
    RelativeContainerModelNG::ResetGuideline(rawNode);
    EXPECT_TRUE(RelativeContainerModelNG::GetGuideline(rawNode).empty());
    RelativeContainerModelNG::ResetBarrier(rawNode);
    EXPECT_TRUE(RelativeContainerModelNG::GetBarrier(rawNode).empty());
    g_isConfigChangePerform = store;
}

/**
 * @tc.name: ModelVisualStateGuardTest
 * @tc.desc: Test that instance SetBarrier/SetGuideline do not update the property outside a visual state process.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, ModelVisualStateGuardTest, TestSize.Level1)
{
    RefPtr<FrameNode> frameNode = CreateRelativeContainer([this](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        std::vector<GuidelineInfo> guidelines;
        AddGuidelineStart(guidelines, GUIDELINE_H, LineDirection::HORIZONTAL, Dimension(10.0f));
        model.SetGuideline(guidelines);
        std::vector<BarrierInfo> barriers;
        AddBarrier(barriers, BARRIER1, BarrierDirection::RIGHT, { ROW1 });
        model.SetBarrier(barriers);
    });
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<RelativeContainerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    ASSERT_TRUE(layoutProperty->HasGuideline());
    ASSERT_TRUE(layoutProperty->HasBarrier());
    ViewStackProcessor::GetInstance()->SetVisualState(VisualState::DISABLED);
    ViewStackProcessor::GetInstance()->ClearStack();
    EXPECT_FALSE(ViewStackProcessor::GetInstance()->IsCurrentVisualStateProcess());
    RelativeContainerModelNG model;
    std::vector<GuidelineInfo> newGuidelines;
    AddGuidelineStart(newGuidelines, GUIDELINE_V, LineDirection::VERTICAL, Dimension(50.0f));
    model.SetGuideline(newGuidelines);
    std::vector<BarrierInfo> newBarriers;
    AddBarrier(newBarriers, BARRIER2, BarrierDirection::BOTTOM, { ROW2 });
    model.SetBarrier(newBarriers);
    /**
     * @tc.expected: The layout property keeps the original guideline and barrier.
     */
    EXPECT_EQ(layoutProperty->GetGuidelineValue().size(), 1u);
    EXPECT_EQ(layoutProperty->GetGuidelineValue()[0].id, GUIDELINE_H);
    EXPECT_EQ(layoutProperty->GetBarrierValue().size(), 1u);
    EXPECT_EQ(layoutProperty->GetBarrierValue()[0].id, BARRIER1);
    ViewStackProcessor::GetInstance()->ClearVisualState();
}

/**
 * @tc.name: ModelConfigChangeGuidelineTest
 * @tc.desc: Test the resource object registration and reload path of the instance SetGuideline.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, ModelConfigChangeGuidelineTest, TestSize.Level1)
{
    bool store = g_isConfigChangePerform;
    g_isConfigChangePerform = true;
    ASSERT_TRUE(SystemProperties::ConfigChangePerform());
    std::vector<GuidelineInfo> guidelines;
    AddGuidelineStart(guidelines, GUIDELINE_H, LineDirection::HORIZONTAL, Dimension(10.0f));
    AddGuidelineStart(guidelines, GUIDELINE_V, LineDirection::VERTICAL, Dimension(20.0f));
    RefPtr<FrameNode> frameNode = CreateRelativeContainer([this, guidelines](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        model.SetGuideline(guidelines);
    });
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<RelativeContainerPattern>();
    ASSERT_NE(pattern, nullptr);
    ASSERT_NE(pattern->resourceMgr_, nullptr);
    const auto& keys = pattern->resourceMgr_->GetResKeyArray();
    EXPECT_NE(std::find(keys.begin(), keys.end(), std::string(CONFIG_CHANGE_KEY)), keys.end());
    pattern->resourceMgr_->ReloadResources();
    auto layoutProperty = frameNode->GetLayoutProperty<RelativeContainerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_TRUE(layoutProperty->HasGuideline());
    EXPECT_EQ(layoutProperty->GetGuidelineValue().size(), 2u);
    RefPtr<FrameNode> emptyNode = CreateRelativeContainer([](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        model.SetGuideline(std::vector<GuidelineInfo> {});
    });
    ASSERT_NE(emptyNode, nullptr);
    auto emptyPattern = emptyNode->GetPattern<RelativeContainerPattern>();
    ASSERT_NE(emptyPattern, nullptr);
    ASSERT_NE(emptyPattern->resourceMgr_, nullptr);
    emptyPattern->resourceMgr_->ReloadResources();
    auto emptyLayoutProperty = emptyNode->GetLayoutProperty<RelativeContainerLayoutProperty>();
    ASSERT_NE(emptyLayoutProperty, nullptr);
    EXPECT_TRUE(emptyLayoutProperty->GetGuidelineValue().empty());
    RelativeContainerModelNG model;
    model.SetGuideline(guidelines);
    RefPtr<FrameNode> row = CreateRow([this, guidelines](RowModelNG rowModel) {
        RelativeContainerModelNG innerModel;
        innerModel.SetGuideline(guidelines);
    });
    ASSERT_NE(row, nullptr);
    EXPECT_EQ(row->GetPattern<RelativeContainerPattern>(), nullptr);
    g_isConfigChangePerform = store;
}

/**
 * @tc.name: ModelResetResObjTest
 * @tc.desc: Test instance and static ResetResObj with config change disabled, enabled and a missing main node.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, ModelResetResObjTest, TestSize.Level1)
{
    bool store = g_isConfigChangePerform;
    std::vector<GuidelineInfo> guidelines;
    AddGuidelineStart(guidelines, GUIDELINE_H, LineDirection::HORIZONTAL, Dimension(10.0f));
    g_isConfigChangePerform = false;
    RefPtr<FrameNode> frameNode = CreateRelativeContainer([this, guidelines](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        model.SetGuideline(guidelines);
        model.ResetResObj(CONFIG_CHANGE_KEY);
    });
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<RelativeContainerPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(pattern->resourceMgr_, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<RelativeContainerLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_TRUE(layoutProperty->HasGuideline());
    g_isConfigChangePerform = true;
    RefPtr<FrameNode> removedNode = CreateRelativeContainer([this, guidelines](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        model.SetGuideline(guidelines);
        model.ResetResObj(CONFIG_CHANGE_KEY);
    });
    ASSERT_NE(removedNode, nullptr);
    auto removedPattern = removedNode->GetPattern<RelativeContainerPattern>();
    ASSERT_NE(removedPattern, nullptr);
    EXPECT_EQ(removedPattern->resourceMgr_, nullptr);
    ViewStackProcessor::GetInstance()->ClearStack();
    RelativeContainerModelNG model;
    model.ResetResObj(CONFIG_CHANGE_KEY);
    RefPtr<FrameNode> row = CreateRow([](RowModelNG rowModel) {
        RelativeContainerModelNG innerModel;
        innerModel.ResetResObj(CONFIG_CHANGE_KEY);
    });
    ASSERT_NE(row, nullptr);
    EXPECT_EQ(row->GetPattern<RelativeContainerPattern>(), nullptr);
    g_isConfigChangePerform = store;
}

/**
 * @tc.name: ModelGuidelineResObjReloadTest
 * @tc.desc: Test SetPositionResObj key dispatch for the start/end resource keys.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, ModelGuidelineResObjReloadTest, TestSize.Level1)
{
    bool store = g_isConfigChangePerform;
    auto resObj = AceType::MakeRefPtr<ResourceObject>("", "", -1);
    const std::string startKey = "relativeContainer.guideLine.position.start";
    const std::string endKey = "relativeContainer.guideLine.position.end";
    g_isConfigChangePerform = false;
    GuidelineInfo disabledInfo;
    disabledInfo.id = GUIDELINE_H;
    RelativeContainerModelNG::SetPositionResObj(resObj, disabledInfo, startKey);
    EXPECT_TRUE(disabledInfo.resMap_.empty());
    g_isConfigChangePerform = true;
    GuidelineInfo nullResInfo;
    nullResInfo.id = GUIDELINE_H;
    RelativeContainerModelNG::SetPositionResObj(nullptr, nullResInfo, startKey);
    EXPECT_TRUE(nullResInfo.resMap_.empty());
    GuidelineInfo startInfo;
    startInfo.id = GUIDELINE_H;
    RelativeContainerModelNG::SetPositionResObj(resObj, startInfo, startKey);
    ASSERT_EQ(startInfo.resMap_.size(), 1u);
    EXPECT_FALSE(startInfo.start.has_value());
    startInfo.ReloadResources();
    EXPECT_TRUE(startInfo.start.has_value());
    GuidelineInfo endInfo;
    endInfo.id = GUIDELINE_H;
    RelativeContainerModelNG::SetPositionResObj(resObj, endInfo, endKey);
    ASSERT_EQ(endInfo.resMap_.size(), 1u);
    endInfo.ReloadResources();
    EXPECT_TRUE(endInfo.end.has_value());
    EXPECT_FALSE(endInfo.start.has_value());
    GuidelineInfo otherInfo;
    otherInfo.id = GUIDELINE_H;
    RelativeContainerModelNG::SetPositionResObj(resObj, otherInfo, "relativeContainer.guideLine.other");
    ASSERT_EQ(otherInfo.resMap_.size(), 1u);
    otherInfo.ReloadResources();
    EXPECT_FALSE(otherInfo.start.has_value());
    EXPECT_FALSE(otherInfo.end.has_value());
    g_isConfigChangePerform = store;
}

/**
 * @tc.name: ModelStaticConfigChangeTest
 * @tc.desc: Test static SetGuideline/ResetResObj resource paths with config change enabled and a wrong node.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, ModelStaticConfigChangeTest, TestSize.Level1)
{
    bool store = g_isConfigChangePerform;
    g_isConfigChangePerform = true;
    std::vector<GuidelineInfo> guidelines;
    AddGuidelineStart(guidelines, GUIDELINE_H, LineDirection::HORIZONTAL, Dimension(10.0f));
    AddGuidelineStart(guidelines, GUIDELINE_V, LineDirection::VERTICAL, Dimension(20.0f));
    auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
    auto frameNode = RelativeContainerModelNG::CreateFrameNode(nodeId);
    ASSERT_NE(frameNode, nullptr);
    auto rawNode = AceType::RawPtr(frameNode);
    auto pattern = frameNode->GetPattern<RelativeContainerPattern>();
    ASSERT_NE(pattern, nullptr);
    RelativeContainerModelNG::SetGuideline(rawNode, guidelines);
    ASSERT_NE(pattern->resourceMgr_, nullptr);
    pattern->resourceMgr_->ReloadResources();
    EXPECT_EQ(RelativeContainerModelNG::GetGuideline(rawNode).size(), 2u);
    RelativeContainerModelNG::ResetResObj(rawNode, CONFIG_CHANGE_KEY);
    EXPECT_EQ(pattern->resourceMgr_, nullptr);
    RefPtr<FrameNode> row = CreateRow([](RowModelNG rowModel) {});
    ASSERT_NE(row, nullptr);
    RelativeContainerModelNG::SetGuideline(AceType::RawPtr(row), guidelines);
    RelativeContainerModelNG::ResetResObj(AceType::RawPtr(row), CONFIG_CHANGE_KEY);
    EXPECT_EQ(row->GetPattern<RelativeContainerPattern>(), nullptr);
    g_isConfigChangePerform = store;
}

/**
 * @tc.name: TopologicalResultHitTest
 * @tc.desc: Test the topological cache probing with equal sizes, a valid cache and a stale cached id.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, TopologicalResultHitTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    auto container = CreateRelativeContainer([this, &row1, &row2](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        row1 = CreateRowNode(ROW1, 40.0f, 40.0f);
        row2 = CreateRowNode(ROW2, 40.0f, 40.0f);
    });
    ASSERT_NE(container, nullptr);
    UpdateConstraint(container);
    auto layoutWrapper = Referenced::RawPtr(container);
    RelativeContainerLayoutAlgorithm algorithm;
    algorithm.versionGreatorOrEqualToEleven_ = true;
    algorithm.Initialize(layoutWrapper);
    ASSERT_EQ(algorithm.idNodeMap_.size(), 2u);
    auto pattern = container->GetPattern<RelativeContainerPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->SetChildAlignRulesChanged(false);
    algorithm.renderList_ = { ROW1, ROW2 };
    EXPECT_FALSE(algorithm.TopologicalResultHit(layoutWrapper));
    pattern->SetTopologicalResultCache(std::list<std::string> { ROW2, ROW1 });
    algorithm.renderList_ = { ROW1, ROW2 };
    EXPECT_TRUE(algorithm.TopologicalResultHit(layoutWrapper));
    EXPECT_EQ(algorithm.renderList_, std::list<std::string>({ ROW2, ROW1 }));
    pattern->SetTopologicalResultCache(std::list<std::string> { ROW1, "ghost" });
    algorithm.renderList_ = { ROW1, ROW2 };
    EXPECT_FALSE(algorithm.TopologicalResultHit(layoutWrapper));
    pattern->SetChildAlignRulesChanged(true);
    EXPECT_FALSE(algorithm.TopologicalResultHit(layoutWrapper));
}

/**
 * @tc.name: TopologicalOrderCacheHitTest
 * @tc.desc: Test that DetermineTopologicalOrder returns early when the cached order can be reused.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, TopologicalOrderCacheHitTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    auto container = CreateRelativeContainer([this, &row1, &row2](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        row1 = CreateRowNode(ROW1, 40.0f, 40.0f);
        std::map<AlignDirection, AlignRule> rules2 = HorizontalRule(AlignDirection::LEFT, ROW1,
            HorizontalAlign::END);
        row2 = CreateRowNode(ROW2, 40.0f, 40.0f, rules2);
    });
    ASSERT_NE(container, nullptr);
    UpdateConstraint(container);
    auto layoutWrapper = Referenced::RawPtr(container);
    RelativeContainerLayoutAlgorithm algorithm;
    algorithm.versionGreatorOrEqualToEleven_ = true;
    algorithm.Initialize(layoutWrapper);
    auto pattern = container->GetPattern<RelativeContainerPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->SetChildAlignRulesChanged(false);
    pattern->SetTopologicalResultCache(std::list<std::string> { ROW1, ROW2 });
    algorithm.renderList_ = { ROW1, ROW2 };
    algorithm.DetermineTopologicalOrder(layoutWrapper);
    EXPECT_TRUE(algorithm.reliedOnMap_.empty());
    EXPECT_EQ(algorithm.renderList_, std::list<std::string>({ ROW1, ROW2 }));
    algorithm.renderList_ = {};
    pattern->SetChildAlignRulesChanged(true);
    algorithm.DetermineTopologicalOrder(layoutWrapper);
    EXPECT_EQ(algorithm.renderList_.size(), 2u);
}

/**
 * @tc.name: MeasureEmptyChildrenTest
 * @tc.desc: Test the end to end measure of a container without children.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, MeasureEmptyChildrenTest, TestSize.Level1)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->SetMinPlatformVersion(11);
    auto container = CreateRelativeContainer([this](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
    });
    ASSERT_NE(container, nullptr);
    auto pattern = container->GetPattern<RelativeContainerPattern>();
    ASSERT_NE(pattern, nullptr);
    CreateLayoutTask(container);
    EXPECT_EQ(container->GetGeometryNode()->GetFrameSize(), SizeF(CONTAINER_WIDTH, CONTAINER_HEIGHT));
    EXPECT_FALSE(pattern->GetChildAlignRulesChanged());
    EXPECT_FALSE(pattern->GetTopologicalResultCacheCopy().has_value());
}

/**
 * @tc.name: MeasureEndToEndTwiceTest
 * @tc.desc: Test two consecutive end-to-end measures and the produced offsets.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, MeasureEndToEndTwiceTest, TestSize.Level0)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->SetMinPlatformVersion(11);
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    auto container = CreateRelativeContainer([this, &row1, &row2](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        std::map<AlignDirection, AlignRule> rules1 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::START);
        AddAlignRule(rules1, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row1 = CreateRowNode(ROW1, 40.0f, 40.0f, rules1);
        std::map<AlignDirection, AlignRule> rules2 = HorizontalRule(AlignDirection::LEFT, ROW1,
            HorizontalAlign::END);
        AddAlignRule(rules2, AlignDirection::TOP, ROW1, VerticalAlign::TOP);
        row2 = CreateRowNode(ROW2, 40.0f, 40.0f, rules2);
    });
    ASSERT_NE(container, nullptr);
    auto pattern = container->GetPattern<RelativeContainerPattern>();
    ASSERT_NE(pattern, nullptr);
    CreateLayoutTask(container);
    EXPECT_EQ(container->GetGeometryNode()->GetFrameSize(), SizeF(CONTAINER_WIDTH, CONTAINER_HEIGHT));
    EXPECT_EQ(row1->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0f, 0.0f));
    EXPECT_EQ(row2->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(40.0f, 0.0f));
    ASSERT_TRUE(pattern->GetTopologicalResultCacheCopy().has_value());
    EXPECT_EQ(pattern->GetTopologicalResultCacheCopy()->size(), 2u);
    container->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    CreateLayoutTask(container);
    EXPECT_EQ(row1->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0f, 0.0f));
    EXPECT_EQ(row2->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(40.0f, 0.0f));
    EXPECT_TRUE(pattern->GetTopologicalResultCacheCopy().has_value());
}

/**
 * @tc.name: MeasureLoopErrorTest
 * @tc.desc: Test the dependency cycle error path of DetermineTopologicalOrder.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, MeasureLoopErrorTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    auto container = CreateRelativeContainer([this, &row1, &row2](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        std::map<AlignDirection, AlignRule> rules1 = VerticalRule(AlignDirection::TOP, ROW2,
            VerticalAlign::BOTTOM);
        row1 = CreateRowNode(ROW1, 40.0f, 40.0f, rules1);
        std::map<AlignDirection, AlignRule> rules2 = VerticalRule(AlignDirection::TOP, ROW1,
            VerticalAlign::BOTTOM);
        row2 = CreateRowNode(ROW2, 40.0f, 40.0f, rules2);
    });
    ASSERT_NE(container, nullptr);
    UpdateConstraint(container);
    auto layoutWrapper = Referenced::RawPtr(container);
    RelativeContainerLayoutAlgorithm algorithm;
    algorithm.versionGreatorOrEqualToEleven_ = true;
    algorithm.Initialize(layoutWrapper);
    algorithm.DetermineTopologicalOrder(layoutWrapper);
    EXPECT_TRUE(algorithm.renderList_.empty());
    EXPECT_EQ(algorithm.incomingDegreeMap_.size(), 2u);
    EXPECT_FALSE(algorithm.idNodeMap_[ROW1].layoutWrapper->IsActive());
    EXPECT_FALSE(algorithm.idNodeMap_[ROW2].layoutWrapper->IsActive());
    EXPECT_EQ(row1->GetGeometryNode()->GetFrameSize(), SizeF(0.0f, 0.0f));
    EXPECT_EQ(row2->GetGeometryNode()->GetFrameSize(), SizeF(0.0f, 0.0f));
}

/**
 * @tc.name: MeasureLoopErrorDebugTest
 * @tc.desc: Test the debug-only branches of the cycle detection and the pattern dump helpers.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, MeasureLoopErrorDebugTest, TestSize.Level1)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->SetMinPlatformVersion(11);
    bool storeDebug = SystemProperties::debugEnabled_;
    SystemProperties::debugEnabled_ = true;
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    auto container = CreateRelativeContainer([this, &row1, &row2](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        std::map<AlignDirection, AlignRule> rules1 = VerticalRule(AlignDirection::TOP, ROW2,
            VerticalAlign::BOTTOM);
        row1 = CreateRowNode(ROW1, 40.0f, 40.0f, rules1);
        std::map<AlignDirection, AlignRule> rules2 = VerticalRule(AlignDirection::TOP, ROW1,
            VerticalAlign::BOTTOM);
        row2 = CreateRowNode(ROW2, 40.0f, 40.0f, rules2);
    });
    ASSERT_NE(container, nullptr);
    auto pattern = container->GetPattern<RelativeContainerPattern>();
    ASSERT_NE(pattern, nullptr);
    CreateLayoutTask(container);
    EXPECT_EQ(container->GetGeometryNode()->GetFrameSize(), SizeF(CONTAINER_WIDTH, CONTAINER_HEIGHT));
    ASSERT_TRUE(pattern->GetLoopDependentNodes().has_value());
    EXPECT_FALSE(pattern->GetLoopDependentNodes().value().empty());
    EXPECT_EQ(row1->GetGeometryNode()->GetFrameSize(), SizeF(0.0f, 0.0f));
    pattern->DumpInfo();
    RefPtr<FrameNode> row3;
    auto acyclicContainer = CreateRelativeContainer([this, &row3](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        row3 = CreateRowNode(ROW3, 40.0f, 40.0f);
    });
    ASSERT_NE(acyclicContainer, nullptr);
    auto acyclicPattern = acyclicContainer->GetPattern<RelativeContainerPattern>();
    ASSERT_NE(acyclicPattern, nullptr);
    CreateLayoutTask(acyclicContainer);
    EXPECT_FALSE(acyclicPattern->GetLoopDependentNodes().has_value());
    acyclicPattern->DumpInfo();
    EXPECT_EQ(RelativeContainerPattern::TopoListToString({}), "[]");
    EXPECT_EQ(RelativeContainerPattern::TopoListToString({ ROW1, ROW2 }), "[row1,row2]");
    EXPECT_EQ(RelativeContainerPattern::LoopDependentNodesToString(std::nullopt), "");
    std::unordered_map<std::string, uint32_t> degreeMap = { { ROW1, 1 } };
    EXPECT_EQ(RelativeContainerPattern::LoopDependentNodesToString(degreeMap), "row1,");
    SystemProperties::debugEnabled_ = storeDebug;
}

/**
 * @tc.name: MeasureRtlOffsetTest
 * @tc.desc: Test the RTL mirroring pass including a barrier entry in the render list.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, MeasureRtlOffsetTest, TestSize.Level0)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->SetMinPlatformVersion(11);
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    auto container = CreateRelativeContainer([this, &row1, &row2](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        std::map<AlignDirection, AlignRule> rules1 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::START);
        AddAlignRule(rules1, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row1 = CreateRowNode(ROW1, 40.0f, 40.0f, rules1);
        std::vector<BarrierInfo> barriers;
        AddBarrier(barriers, BARRIER1, BarrierDirection::RIGHT, { ROW1 });
        model.SetBarrier(barriers);
        std::map<AlignDirection, AlignRule> rules2 = HorizontalRule(AlignDirection::LEFT, BARRIER1,
            HorizontalAlign::START);
        AddAlignRule(rules2, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row2 = CreateRowNode(ROW2, 20.0f, 20.0f, rules2);
    });
    ASSERT_NE(container, nullptr);
    container->GetLayoutProperty()->UpdateLayoutDirection(TextDirection::RTL);
    CreateLayoutTask(container);
    EXPECT_EQ(container->GetGeometryNode()->GetFrameSize(), SizeF(CONTAINER_WIDTH, CONTAINER_HEIGHT));
    EXPECT_EQ(row1->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(260.0f, 0.0f));
    EXPECT_EQ(row2->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(240.0f, 0.0f));
}

/**
 * @tc.name: UpdateSizeWhenChildrenEmptyTest
 * @tc.desc: Test UpdateSizeWhenChildrenEmpty with a null wrapper, a missing constraint and a zero ideal size.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, UpdateSizeWhenChildrenEmptyTest, TestSize.Level1)
{
    RelativeContainerLayoutAlgorithm algorithm;
    algorithm.versionGreatorOrEqualToEleven_ = true;
    algorithm.UpdateSizeWhenChildrenEmpty(nullptr);
    auto container = CreateRelativeContainer([](RelativeContainerModelNG model) {});
    ASSERT_NE(container, nullptr);
    algorithm.UpdateSizeWhenChildrenEmpty(Referenced::RawPtr(container));
    EXPECT_EQ(container->GetGeometryNode()->GetFrameSize(), SizeF(0.0f, 0.0f));
    LayoutConstraintF constraint;
    constraint.percentReference = { 0.0f, 0.0f };
    constraint.parentIdealSize = { 0.0f, 0.0f };
    constraint.selfIdealSize = OptionalSizeF(0.0f, 0.0f);
    container->GetLayoutProperty()->UpdateLayoutConstraint(constraint);
    algorithm.UpdateSizeWhenChildrenEmpty(Referenced::RawPtr(container));
    EXPECT_EQ(container->GetGeometryNode()->GetFrameSize(), SizeF(0.0f, 0.0f));
}

/**
 * @tc.name: UpdateSizeWhenChildrenEmptyAutoTest
 * @tc.desc: Test UpdateSizeWhenChildrenEmpty with auto width/height, padding and a missing self ideal size.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, UpdateSizeWhenChildrenEmptyAutoTest, TestSize.Level1)
{
    RelativeContainerLayoutAlgorithm algorithm;
    algorithm.versionGreatorOrEqualToEleven_ = true;
    auto autoContainer = CreateRelativeContainer([](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(0.0f, DimensionUnit::AUTO));
        ViewAbstract::SetHeight(CalcLength(0.0f, DimensionUnit::AUTO));
        ViewAbstract::SetPadding(CalcLength(10.0f, DimensionUnit::PX));
    });
    ASSERT_NE(autoContainer, nullptr);
    UpdateConstraint(autoContainer);
    algorithm.UpdateSizeWhenChildrenEmpty(Referenced::RawPtr(autoContainer));
    EXPECT_EQ(autoContainer->GetGeometryNode()->GetFrameSize(), SizeF(20.0f, 20.0f));
    auto paddingContainer = CreateRelativeContainer([](RelativeContainerModelNG model) {
        ViewAbstract::SetPadding(CalcLength(10.0f, DimensionUnit::PX));
    });
    ASSERT_NE(paddingContainer, nullptr);
    UpdateConstraint(paddingContainer);
    algorithm.UpdateSizeWhenChildrenEmpty(Referenced::RawPtr(paddingContainer));
    EXPECT_EQ(paddingContainer->GetGeometryNode()->GetFrameSize(),
        SizeF(CONTAINER_WIDTH, CONTAINER_HEIGHT));
}

/**
 * @tc.name: MeasureSelfAutoSizeTest
 * @tc.desc: Test MeasureSelf for auto sized containers with a barrier, a gone child and a relying child.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, MeasureSelfAutoSizeTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    auto container = CreateRelativeContainer([this, &row1, &row2](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(0.0f, DimensionUnit::AUTO));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        ViewAbstract::SetPadding(CalcLength(10.0f, DimensionUnit::PX));
        std::map<AlignDirection, AlignRule> rules1 = VerticalRule(AlignDirection::TOP, CONTAINER_ID,
            VerticalAlign::TOP);
        row1 = CreateRowNode(ROW1, 40.0f, 40.0f, rules1);
        row2 = CreateRowNode(ROW2, 30.0f, 30.0f);
        std::vector<BarrierInfo> barriers;
        AddBarrier(barriers, BARRIER1, BarrierDirection::RIGHT, { ROW1 });
        model.SetBarrier(barriers);
    });
    ASSERT_NE(container, nullptr);
    ASSERT_NE(row2, nullptr);
    row2->GetLayoutProperty()->UpdateVisibility(VisibleType::GONE);
    auto layoutWrapper = Referenced::RawPtr(container);
    RelativeContainerLayoutAlgorithm algorithm;
    PrepareAlgorithm(container, algorithm, true);
    ASSERT_EQ(algorithm.idNodeMap_.size(), 2u);
    algorithm.MeasureSelf(layoutWrapper);
    EXPECT_EQ(container->GetGeometryNode()->GetFrameSize(), SizeF(60.0f, CONTAINER_HEIGHT));
    RefPtr<FrameNode> row3;
    RefPtr<FrameNode> row4;
    auto heightAutoContainer = CreateRelativeContainer([this, &row3, &row4](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(0.0f, DimensionUnit::AUTO));
        ViewAbstract::SetPadding(CalcLength(10.0f, DimensionUnit::PX));
        std::map<AlignDirection, AlignRule> rules1 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::START);
        row3 = CreateRowNode(ROW3, 40.0f, 40.0f, rules1);
        row4 = CreateRowNode("row4", 30.0f, 30.0f);
    });
    ASSERT_NE(heightAutoContainer, nullptr);
    RelativeContainerLayoutAlgorithm heightAlgorithm;
    PrepareAlgorithm(heightAutoContainer, heightAlgorithm, true);
    heightAlgorithm.MeasureSelf(Referenced::RawPtr(heightAutoContainer));
    EXPECT_EQ(heightAutoContainer->GetGeometryNode()->GetFrameSize(), SizeF(CONTAINER_WIDTH, 60.0f));
    algorithm.MeasureSelf(nullptr);
    auto emptyContainer = CreateRelativeContainer([](RelativeContainerModelNG model) {});
    ASSERT_NE(emptyContainer, nullptr);
    UpdateConstraint(emptyContainer);
    algorithm.MeasureSelf(Referenced::RawPtr(emptyContainer));
    EXPECT_EQ(emptyContainer->GetGeometryNode()->GetFrameSize(), SizeF(0.0f, 0.0f));
}

/**
 * @tc.name: DetermineSelfWrapContentTest
 * @tc.desc: Test DetermineSelf with a wrap content width policy where only one child contributes to the size.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, DetermineSelfWrapContentTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    auto container = CreateRelativeContainer([this, &row1, &row2](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        std::map<AlignDirection, AlignRule> rules1 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::START);
        AddAlignRule(rules1, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row1 = CreateRowNode(ROW1, 40.0f, 40.0f, rules1);
        std::map<AlignDirection, AlignRule> rules2 = HorizontalRule(AlignDirection::LEFT, ROW1,
            HorizontalAlign::END);
        AddAlignRule(rules2, AlignDirection::TOP, ROW1, VerticalAlign::BOTTOM);
        row2 = CreateRowNode(ROW2, 50.0f, 50.0f, rules2);
        CreateRowNode(ROW3, 30.0f, 30.0f);
    });
    ASSERT_NE(container, nullptr);
    container->GetLayoutProperty()->UpdateLayoutPolicyProperty(LayoutCalPolicy::WRAP_CONTENT, true);
    auto layoutWrapper = Referenced::RawPtr(container);
    RelativeContainerLayoutAlgorithm algorithm;
    PrepareAlgorithm(container, algorithm);
    algorithm.DetermineSelf(layoutWrapper);
    EXPECT_EQ(algorithm.recordOffsetMap_[ROW1], OffsetF(0.0f, 0.0f));
    EXPECT_EQ(algorithm.recordOffsetMap_[ROW2], OffsetF(40.0f, 40.0f));
    EXPECT_EQ(container->GetGeometryNode()->GetFrameSize(), SizeF(30.0f, CONTAINER_HEIGHT));
    EXPECT_EQ(algorithm.containerSizeWithoutPaddingBorder_.Width(), 30.0f);
    container->GetLayoutProperty()->UpdateLayoutPolicyProperty(LayoutCalPolicy::WRAP_CONTENT, false);
    algorithm.DetermineSelf(layoutWrapper);
    EXPECT_EQ(container->GetGeometryNode()->GetFrameSize(), SizeF(30.0f, 30.0f));
}

/**
 * @tc.name: DetermineSelfFixIdealSizeTest
 * @tc.desc: Test DetermineSelf with the fix at ideal size policy on both axes.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, DetermineSelfFixIdealSizeTest, TestSize.Level1)
{
    auto container = CreateRelativeContainer([this](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        CreateRowNode(ROW1, 30.0f, 30.0f);
    });
    ASSERT_NE(container, nullptr);
    auto layoutProperty = container->GetLayoutProperty();
    layoutProperty->UpdateLayoutPolicyProperty(LayoutCalPolicy::FIX_AT_IDEAL_SIZE, true);
    layoutProperty->UpdateLayoutPolicyProperty(LayoutCalPolicy::FIX_AT_IDEAL_SIZE, false);
    RelativeContainerLayoutAlgorithm algorithm;
    PrepareAlgorithm(container, algorithm);
    algorithm.DetermineSelf(Referenced::RawPtr(container));
    EXPECT_EQ(container->GetGeometryNode()->GetFrameSize(), SizeF(30.0f, 30.0f));
    EXPECT_EQ(algorithm.containerSizeWithoutPaddingBorder_, SizeF(30.0f, 30.0f));
}

/**
 * @tc.name: DetermineSelfAllAdaptiveTest
 * @tc.desc: Test DetermineSelf when every child relies on the container on both axes and all policies are adaptive.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, DetermineSelfAllAdaptiveTest, TestSize.Level1)
{
    auto container = CreateRelativeContainer([this](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        std::map<AlignDirection, AlignRule> rules1 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::START);
        AddAlignRule(rules1, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        CreateRowNode(ROW1, 40.0f, 40.0f, rules1);
        std::map<AlignDirection, AlignRule> rules2 = HorizontalRule(AlignDirection::LEFT, ROW1,
            HorizontalAlign::END);
        AddAlignRule(rules2, AlignDirection::TOP, ROW1, VerticalAlign::BOTTOM);
        CreateRowNode(ROW2, 50.0f, 50.0f, rules2);
    });
    ASSERT_NE(container, nullptr);
    auto layoutProperty = container->GetLayoutProperty();
    layoutProperty->UpdateLayoutPolicyProperty(LayoutCalPolicy::WRAP_CONTENT, true);
    layoutProperty->UpdateLayoutPolicyProperty(LayoutCalPolicy::WRAP_CONTENT, false);
    RelativeContainerLayoutAlgorithm algorithm;
    PrepareAlgorithm(container, algorithm);
    algorithm.DetermineSelf(Referenced::RawPtr(container));
    EXPECT_TRUE(algorithm.isHorizontalRelyOnContainer_);
    EXPECT_TRUE(algorithm.isVerticalRelyOnContainer_);
    EXPECT_TRUE(algorithm.IsUnionRelyOnContainer(LineDirection::HORIZONTAL, ROW2));
    EXPECT_TRUE(algorithm.IsUnionRelyOnContainer(LineDirection::VERTICAL, ROW2));
    EXPECT_EQ(container->GetGeometryNode()->GetFrameSize(), SizeF(0.0f, 0.0f));
}

/**
 * @tc.name: DetermineSelfBarrierAndInactiveTest
 * @tc.desc: Test DetermineSelf with a barrier in the render list and an inactive child.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, DetermineSelfBarrierAndInactiveTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    auto container = CreateRelativeContainer([this, &row1, &row2](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        std::map<AlignDirection, AlignRule> rules1 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::START);
        AddAlignRule(rules1, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row1 = CreateRowNode(ROW1, 40.0f, 40.0f, rules1);
        std::vector<BarrierInfo> barriers;
        AddBarrier(barriers, BARRIER1, BarrierDirection::RIGHT, { ROW1 });
        model.SetBarrier(barriers);
        std::map<AlignDirection, AlignRule> rules2 = HorizontalRule(AlignDirection::LEFT, BARRIER1,
            HorizontalAlign::START);
        AddAlignRule(rules2, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row2 = CreateRowNode(ROW2, 20.0f, 20.0f, rules2);
        CreateRowNode(ROW3, 30.0f, 30.0f);
    });
    ASSERT_NE(container, nullptr);
    container->GetLayoutProperty()->UpdateLayoutPolicyProperty(LayoutCalPolicy::WRAP_CONTENT, true);
    RelativeContainerLayoutAlgorithm algorithm;
    PrepareAlgorithm(container, algorithm);
    ASSERT_TRUE(algorithm.idNodeMap_.count(ROW3) > 0);
    algorithm.idNodeMap_[ROW3].layoutWrapper->SetActive(false);
    algorithm.DetermineSelf(Referenced::RawPtr(container));
    EXPECT_EQ(algorithm.recordOffsetMap_[BARRIER1], OffsetF(40.0f, 0.0f));
    EXPECT_EQ(algorithm.recordOffsetMap_[ROW2], OffsetF(40.0f, 0.0f));
    EXPECT_EQ(container->GetGeometryNode()->GetFrameSize(), SizeF(60.0f, CONTAINER_HEIGHT));
}

/**
 * @tc.name: PreCalcChildSizeTest
 * @tc.desc: Test PreCalcChildSize for a plain child, a child without align rules and an aligned child.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, PreCalcChildSizeTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    RefPtr<FrameNode> row3;
    auto container = CreateRelativeContainer([this, &row1, &row2, &row3](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        row1 = CreateRowNode(ROW1, 40.0f, 40.0f);
        row2 = CreateRow([this](RowModelNG rowModel) {
            ViewAbstract::SetInspectorId(ROW2);
            ViewAbstract::SetWidth(CalcLength(30.0f, DimensionUnit::PX));
            ViewAbstract::SetHeight(CalcLength(30.0f, DimensionUnit::PX));
            ViewAbstract::SetBias(BiasPair(0.5f, 0.5f));
        });
        std::map<AlignDirection, AlignRule> rules3 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::CENTER);
        AddAlignRule(rules3, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::CENTER);
        row3 = CreateRowNode(ROW3, 20.0f, 20.0f, rules3);
    });
    ASSERT_NE(container, nullptr);
    UpdateConstraint(container);
    auto layoutWrapper = Referenced::RawPtr(container);
    RelativeContainerLayoutAlgorithm algorithm;
    algorithm.versionGreatorOrEqualToEleven_ = true;
    algorithm.Initialize(layoutWrapper);
    auto childConstraint = container->GetLayoutProperty()->CreateChildConstraint();
    auto child1 = algorithm.idNodeMap_[ROW1].layoutWrapper;
    ASSERT_NE(child1, nullptr);
    EXPECT_TRUE(algorithm.PreCalcChildSize(layoutWrapper, child1, ROW1, childConstraint));
    EXPECT_EQ(algorithm.recordOffsetMap_[ROW1], OffsetF(0.0f, 0.0f));
    auto child2 = algorithm.idNodeMap_[ROW2].layoutWrapper;
    ASSERT_NE(child2, nullptr);
    EXPECT_TRUE(algorithm.PreCalcChildSize(layoutWrapper, child2, ROW2, childConstraint));
    EXPECT_EQ(algorithm.recordOffsetMap_[ROW2], OffsetF(0.0f, 0.0f));
    auto child3 = algorithm.idNodeMap_[ROW3].layoutWrapper;
    ASSERT_NE(child3, nullptr);
    EXPECT_TRUE(algorithm.PreCalcChildSize(layoutWrapper, child3, ROW3, childConstraint));
    EXPECT_EQ(algorithm.recordOffsetMap_[ROW3], OffsetF(150.0f, 150.0f));
}

/**
 * @tc.name: MeasureWrapContentTest
 * @tc.desc: Test MeasureWrapContent for missing, plain, inactive and aligned children and the match policy lift.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, MeasureWrapContentTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    auto container = CreateRelativeContainer([this, &row1, &row2](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        row1 = CreateRowNode(ROW1, 40.0f, 40.0f);
        row2 = CreateRowNode(ROW2, 30.0f, 30.0f);
        std::map<AlignDirection, AlignRule> rules3 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::START);
        CreateRowNode(ROW3, 20.0f, 20.0f, rules3);
    });
    ASSERT_NE(container, nullptr);
    ASSERT_NE(row2, nullptr);
    row2->GetLayoutProperty()->UpdateLayoutPolicyProperty(LayoutCalPolicy::MATCH_PARENT, true);
    row2->GetLayoutProperty()->UpdateLayoutPolicyProperty(LayoutCalPolicy::MATCH_PARENT, false);
    UpdateConstraint(container);
    auto layoutWrapper = Referenced::RawPtr(container);
    RelativeContainerLayoutAlgorithm algorithm;
    algorithm.versionGreatorOrEqualToEleven_ = true;
    algorithm.Initialize(layoutWrapper);
    LayoutConstraintF childConstraint = container->GetLayoutProperty()->CreateChildConstraint();
    EXPECT_FALSE(algorithm.MeasureWrapContent("ghost", childConstraint));
    EXPECT_FALSE(algorithm.MeasureWrapContent(ROW1, childConstraint));
    EXPECT_EQ(algorithm.recordOffsetMap_[ROW1], OffsetF(0.0f, 0.0f));
    EXPECT_FALSE(algorithm.MeasureWrapContent(ROW2, childConstraint));
    EXPECT_EQ(childConstraint.parentIdealSize.Width().value_or(-1.0f), CONTAINER_WIDTH);
    EXPECT_EQ(childConstraint.parentIdealSize.Height().value_or(-1.0f), CONTAINER_HEIGHT);
    EXPECT_TRUE(algorithm.MeasureWrapContent(ROW3, childConstraint));
    ASSERT_TRUE(algorithm.idNodeMap_.count(ROW3) > 0);
    algorithm.idNodeMap_[ROW3].layoutWrapper->SetActive(false);
    EXPECT_FALSE(algorithm.MeasureWrapContent(ROW3, childConstraint));
    algorithm.idNodeMap_[ROW3].layoutWrapper->SetActive(true);
}

/**
 * @tc.name: GuidelineCalculateTest
 * @tc.desc: Test CalcHorizontalGuideline/CalcVerticalGuideline with pixel, percent, end and empty values.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, GuidelineCalculateTest, TestSize.Level1)
{
    RelativeContainerLayoutAlgorithm algorithm;
    std::optional<CalcSize> fixedSize = CalcSize(CalcLength(100.0f, DimensionUnit::PX),
        CalcLength(200.0f, DimensionUnit::PX));
    std::optional<CalcSize> autoSize = CalcSize(CalcLength(0.0f, DimensionUnit::AUTO),
        CalcLength(0.0f, DimensionUnit::AUTO));
    GuidelineInfo pixelStart;
    pixelStart.id = "h1";
    pixelStart.start = Dimension(30.0f, DimensionUnit::PX);
    algorithm.CalcHorizontalGuideline(fixedSize, 300.0f, pixelStart);
    EXPECT_EQ(algorithm.guidelines_["h1"].first, LineDirection::HORIZONTAL);
    EXPECT_EQ(algorithm.guidelines_["h1"].second, 30.0f);
    GuidelineInfo percentAuto;
    percentAuto.id = "h2";
    percentAuto.start = Dimension(0.5f, DimensionUnit::PERCENT);
    algorithm.CalcHorizontalGuideline(autoSize, 300.0f, percentAuto);
    EXPECT_EQ(algorithm.guidelines_["h2"].second, 0.0f);
    GuidelineInfo percentFixed;
    percentFixed.id = "h3";
    percentFixed.start = Dimension(0.5f, DimensionUnit::PERCENT);
    algorithm.CalcHorizontalGuideline(fixedSize, 300.0f, percentFixed);
    EXPECT_EQ(algorithm.guidelines_["h3"].second, 150.0f);
    GuidelineInfo pixelEnd;
    pixelEnd.id = "h4";
    pixelEnd.end = Dimension(40.0f, DimensionUnit::PX);
    algorithm.CalcHorizontalGuideline(fixedSize, 300.0f, pixelEnd);
    EXPECT_EQ(algorithm.guidelines_["h4"].second, 260.0f);
    GuidelineInfo autoEnd;
    autoEnd.id = "h5";
    autoEnd.end = Dimension(40.0f, DimensionUnit::PX);
    algorithm.CalcHorizontalGuideline(autoSize, 300.0f, autoEnd);
    EXPECT_EQ(algorithm.guidelines_["h5"].second, 0.0f);
    GuidelineInfo emptyInfo;
    emptyInfo.id = "h6";
    algorithm.CalcHorizontalGuideline(fixedSize, 300.0f, emptyInfo);
    EXPECT_TRUE(algorithm.guidelines_.count("h6") == 0);
    GuidelineInfo vPixelStart;
    vPixelStart.id = "v1";
    vPixelStart.start = Dimension(25.0f, DimensionUnit::PX);
    algorithm.CalcVerticalGuideline(fixedSize, 200.0f, vPixelStart);
    EXPECT_EQ(algorithm.guidelines_["v1"].first, LineDirection::VERTICAL);
    EXPECT_EQ(algorithm.guidelines_["v1"].second, 25.0f);
    GuidelineInfo vPercentAuto;
    vPercentAuto.id = "v2";
    vPercentAuto.start = Dimension(0.5f, DimensionUnit::PERCENT);
    algorithm.CalcVerticalGuideline(autoSize, 200.0f, vPercentAuto);
    EXPECT_EQ(algorithm.guidelines_["v2"].second, 0.0f);
    GuidelineInfo vPercentFixed;
    vPercentFixed.id = "v3";
    vPercentFixed.start = Dimension(0.5f, DimensionUnit::PERCENT);
    algorithm.CalcVerticalGuideline(fixedSize, 200.0f, vPercentFixed);
    EXPECT_EQ(algorithm.guidelines_["v3"].second, 100.0f);
    GuidelineInfo vPixelEnd;
    vPixelEnd.id = "v4";
    vPixelEnd.end = Dimension(40.0f, DimensionUnit::PX);
    algorithm.CalcVerticalGuideline(fixedSize, 200.0f, vPixelEnd);
    EXPECT_EQ(algorithm.guidelines_["v4"].second, 160.0f);
    GuidelineInfo vAutoEnd;
    vAutoEnd.id = "v5";
    vAutoEnd.end = Dimension(40.0f, DimensionUnit::PX);
    algorithm.CalcVerticalGuideline(autoSize, 200.0f, vAutoEnd);
    EXPECT_EQ(algorithm.guidelines_["v5"].second, 0.0f);
    GuidelineInfo vEmpty;
    vEmpty.id = "v6";
    algorithm.CalcVerticalGuideline(fixedSize, 200.0f, vEmpty);
    EXPECT_TRUE(algorithm.guidelines_.count("v6") == 0);
}

/**
 * @tc.name: GuidelineIntegrationTest
 * @tc.desc: Test CalcGuideline end to end through Initialize, including the recorded guideline offsets.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, GuidelineIntegrationTest, TestSize.Level1)
{
    auto container = CreateRelativeContainer([this](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(400.0f, DimensionUnit::PX));
        std::vector<GuidelineInfo> guidelines;
        AddGuidelineStart(guidelines, GUIDELINE_H, LineDirection::HORIZONTAL, Dimension(30.0f, DimensionUnit::PX));
        AddGuidelineStart(guidelines, GUIDELINE_V, LineDirection::VERTICAL, Dimension(40.0f, DimensionUnit::PX));
        AddGuidelineEnd(guidelines, "guidelineEnd", LineDirection::HORIZONTAL, Dimension(50.0f, DimensionUnit::PX));
        model.SetGuideline(guidelines);
    });
    ASSERT_NE(container, nullptr);
    UpdateConstraint(container, CONTAINER_WIDTH, 400.0f, true);
    RelativeContainerLayoutAlgorithm algorithm;
    algorithm.versionGreatorOrEqualToEleven_ = true;
    algorithm.Initialize(Referenced::RawPtr(container));
    ASSERT_EQ(algorithm.guidelines_.size(), 3u);
    EXPECT_EQ(algorithm.recordOffsetMap_[GUIDELINE_H], OffsetF(0.0f, 30.0f));
    EXPECT_EQ(algorithm.recordOffsetMap_[GUIDELINE_V], OffsetF(40.0f, 0.0f));
    EXPECT_EQ(algorithm.recordOffsetMap_["guidelineEnd"], OffsetF(0.0f, 350.0f));
    EXPECT_TRUE(algorithm.IsGuideline(GUIDELINE_H));
    EXPECT_FALSE(algorithm.IsGuideline(ROW1));
    EXPECT_TRUE(algorithm.IsGuidelineOrBarrier(GUIDELINE_V));
    EXPECT_FALSE(algorithm.IsGuidelineOrBarrier(ROW1));
    RelativeContainerLayoutAlgorithm oldAlgorithm;
    oldAlgorithm.versionGreatorOrEqualToEleven_ = false;
    oldAlgorithm.Initialize(Referenced::RawPtr(container));
    EXPECT_TRUE(oldAlgorithm.guidelines_.empty());
    EXPECT_FALSE(oldAlgorithm.IsGuideline(GUIDELINE_H));
}

/**
 * @tc.name: BarrierTopologyTest
 * @tc.desc: Test barrier to barrier dependencies, guideline references, empty references and a stale id.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, BarrierTopologyTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    auto container = CreateRelativeContainer([this, &row1](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        std::map<AlignDirection, AlignRule> rules1 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::START);
        AddAlignRule(rules1, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row1 = CreateRowNode(ROW1, 40.0f, 40.0f, rules1);
        std::vector<GuidelineInfo> guidelines;
        AddGuidelineStart(guidelines, GUIDELINE_H, LineDirection::HORIZONTAL, Dimension(30.0f, DimensionUnit::PX));
        AddGuidelineStart(guidelines, GUIDELINE_V, LineDirection::VERTICAL, Dimension(50.0f, DimensionUnit::PX));
        model.SetGuideline(guidelines);
        std::vector<BarrierInfo> barriers;
        AddBarrier(barriers, BARRIER1, BarrierDirection::RIGHT, { ROW1 });
        AddBarrier(barriers, BARRIER2, BarrierDirection::LEFT,
            { BARRIER1, GUIDELINE_H, GUIDELINE_V, "missing" });
        AddBarrier(barriers, ROW3, BarrierDirection::BOTTOM, {});
        model.SetBarrier(barriers);
    });
    ASSERT_NE(container, nullptr);
    RelativeContainerLayoutAlgorithm algorithm;
    PrepareAlgorithm(container, algorithm, true);
    ASSERT_EQ(algorithm.barriers_.size(), 3u);
    // Initialize() only collects nodes/guidelines/barriers (CollectNodesById, CalcGuideline, CalcBarrier,
    // CheckChain); the incoming degree of every barrier is computed by the topological pre-pass, in
    // UpdateDegreeMapWithBarrier. Run it so incomingDegreeMap_ holds the barrier degrees.
    algorithm.PreTopologicalLoopDetection(Referenced::RawPtr(container));
    // barrier2 references barrier1 plus one guideline, one guideline and one unknown id: only the
    // barrier->barrier edge counts, guidelines and ids absent from idNodeMap_ are skipped.
    EXPECT_EQ(algorithm.incomingDegreeMap_[BARRIER2], 1u);
    EXPECT_EQ(algorithm.incomingDegreeMap_[ROW3], 0u);
    EXPECT_EQ(algorithm.recordOffsetMap_[BARRIER1], OffsetF(40.0f, 0.0f));
    EXPECT_EQ(algorithm.recordOffsetMap_[BARRIER2], OffsetF(50.0f, 0.0f));
    EXPECT_EQ(algorithm.recordOffsetMap_[ROW3], OffsetF(0.0f, 0.0f));
    algorithm.recordOffsetMap_["ghost"] = OffsetF(500.0f, 500.0f);
    algorithm.barriers_[BARRIER1].second = { ROW1, "ghost" };
    algorithm.MeasureBarrier(BARRIER1);
    EXPECT_EQ(algorithm.recordOffsetMap_[BARRIER1], OffsetF(40.0f, 0.0f));
}

/**
 * @tc.name: BarrierDirectionAndMeasureTest
 * @tc.desc: Test the barrier direction switch arms including the default arm and the gone referenced child.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, BarrierDirectionAndMeasureTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    auto container = CreateRelativeContainer([this, &row1](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        std::map<AlignDirection, AlignRule> rules1 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::START);
        AddAlignRule(rules1, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row1 = CreateRowNode(ROW1, 40.0f, 40.0f, rules1);
        std::vector<BarrierInfo> barriers;
        AddBarrier(barriers, BARRIER1, BarrierDirection::START, { ROW1 });
        model.SetBarrier(barriers);
    });
    ASSERT_NE(container, nullptr);
    RelativeContainerLayoutAlgorithm algorithm;
    PrepareAlgorithm(container, algorithm, true);
    ASSERT_EQ(algorithm.barriers_.size(), 1u);
    EXPECT_EQ(algorithm.barriers_[BARRIER1].first, BarrierDirection::LEFT);
    algorithm.barriers_[BARRIER1] = { BarrierDirection::LEFT, { ROW1 } };
    algorithm.MeasureBarrier(BARRIER1);
    EXPECT_EQ(algorithm.recordOffsetMap_[BARRIER1], OffsetF(0.0f, 0.0f));
    algorithm.barriers_[BARRIER1] = { BarrierDirection::RIGHT, { ROW1 } };
    algorithm.MeasureBarrier(BARRIER1);
    EXPECT_EQ(algorithm.recordOffsetMap_[BARRIER1], OffsetF(40.0f, 0.0f));
    algorithm.barriers_[BARRIER1] = { BarrierDirection::TOP, { ROW1 } };
    algorithm.MeasureBarrier(BARRIER1);
    EXPECT_EQ(algorithm.recordOffsetMap_[BARRIER1], OffsetF(0.0f, 0.0f));
    algorithm.barriers_[BARRIER1] = { BarrierDirection::BOTTOM, { ROW1 } };
    algorithm.MeasureBarrier(BARRIER1);
    EXPECT_EQ(algorithm.recordOffsetMap_[BARRIER1], OffsetF(0.0f, 40.0f));
    algorithm.recordOffsetMap_.erase(BARRIER1);
    algorithm.barriers_[BARRIER1] = { BarrierDirection::START, { ROW1 } };
    algorithm.MeasureBarrier(BARRIER1);
    EXPECT_TRUE(algorithm.recordOffsetMap_.count(BARRIER1) == 0);
    row1->GetLayoutProperty()->UpdateVisibility(VisibleType::GONE);
    algorithm.barriers_[BARRIER1] = { BarrierDirection::BOTTOM, { ROW1 } };
    algorithm.MeasureBarrier(BARRIER1);
    EXPECT_EQ(algorithm.recordOffsetMap_[BARRIER1], OffsetF(0.0f, 0.0f));
}

/**
 * @tc.name: AnchorValueByAlignRuleTest
 * @tc.desc: Test the horizontal and vertical anchor value resolution with container, child and margin anchors.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, AnchorValueByAlignRuleTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    auto container = CreateRelativeContainer([this, &row1](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        row1 = CreateRowNode(ROW1, 50.0f, 50.0f);
    });
    ASSERT_NE(container, nullptr);
    RelativeContainerLayoutAlgorithm algorithm;
    PrepareAlgorithm(container, algorithm, true);
    MarginPropertyF margin;
    margin.left = 10.0f;
    margin.top = 20.0f;
    row1->GetGeometryNode()->UpdateMargin(margin);
    algorithm.recordOffsetMap_[ROW1] = OffsetF(20.0f, 20.0f);
    AlignRule endRule;
    endRule.anchor = ROW1;
    endRule.horizontal = HorizontalAlign::END;
    EXPECT_EQ(algorithm.GetHorizontalAnchorValueByAlignRule(endRule), 80.0f);
    AlignRule centerRule;
    centerRule.anchor = ROW1;
    centerRule.horizontal = HorizontalAlign::CENTER;
    EXPECT_EQ(algorithm.GetHorizontalAnchorValueByAlignRule(centerRule), 55.0f);
    AlignRule defaultRule;
    defaultRule.anchor = ROW1;
    defaultRule.horizontal = static_cast<HorizontalAlign>(0);
    EXPECT_EQ(algorithm.GetHorizontalAnchorValueByAlignRule(defaultRule), 30.0f);
    AlignRule containerEnd;
    containerEnd.anchor = CONTAINER_ID;
    containerEnd.horizontal = HorizontalAlign::END;
    EXPECT_EQ(algorithm.GetHorizontalAnchorValueByAlignRule(containerEnd), CONTAINER_WIDTH);
    AlignRule containerStart;
    containerStart.anchor = CONTAINER_ID;
    containerStart.horizontal = HorizontalAlign::START;
    EXPECT_EQ(algorithm.GetHorizontalAnchorValueByAlignRule(containerStart), 0.0f);
    AlignRule bottomRule;
    bottomRule.anchor = ROW1;
    bottomRule.vertical = VerticalAlign::BOTTOM;
    EXPECT_EQ(algorithm.GetVerticalAnchorValueByAlignRule(bottomRule), 90.0f);
    AlignRule verticalCenterRule;
    verticalCenterRule.anchor = ROW1;
    verticalCenterRule.vertical = VerticalAlign::CENTER;
    EXPECT_EQ(algorithm.GetVerticalAnchorValueByAlignRule(verticalCenterRule), 65.0f);
    AlignRule verticalDefaultRule;
    verticalDefaultRule.anchor = ROW1;
    verticalDefaultRule.vertical = VerticalAlign::BASELINE;
    EXPECT_EQ(algorithm.GetVerticalAnchorValueByAlignRule(verticalDefaultRule), 40.0f);
    AlignRule containerBottom;
    containerBottom.anchor = CONTAINER_ID;
    containerBottom.vertical = VerticalAlign::BOTTOM;
    EXPECT_EQ(algorithm.GetVerticalAnchorValueByAlignRule(containerBottom), CONTAINER_HEIGHT);
    algorithm.versionGreatorOrEqualToEleven_ = false;
    AlignRule oldCenterRule;
    oldCenterRule.anchor = ROW1;
    oldCenterRule.horizontal = HorizontalAlign::CENTER;
    EXPECT_EQ(algorithm.GetHorizontalAnchorValueByAlignRule(oldCenterRule), 50.0f);
    AlignRule oldVerticalCenterRule;
    oldVerticalCenterRule.anchor = ROW1;
    oldVerticalCenterRule.vertical = VerticalAlign::CENTER;
    EXPECT_EQ(algorithm.GetVerticalAnchorValueByAlignRule(oldVerticalCenterRule), 55.0f);
}

/**
 * @tc.name: OffsetHelperTest
 * @tc.desc: Test CalcHorizontalOffset/CalcVerticalOffset dispatch, CalcAnchorWidth and GetOriginMarginLeft.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, OffsetHelperTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    auto container = CreateRelativeContainer([this, &row1, &row2](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        row1 = CreateRowNode(ROW1, 50.0f, 50.0f);
        row2 = CreateRowNode(ROW2, 30.0f, 30.0f);
    });
    ASSERT_NE(container, nullptr);
    RelativeContainerLayoutAlgorithm algorithm;
    PrepareAlgorithm(container, algorithm, true);
    algorithm.recordOffsetMap_[ROW1] = OffsetF(20.0f, 20.0f);
    AlignRule endRule;
    endRule.anchor = ROW1;
    endRule.horizontal = HorizontalAlign::END;
    EXPECT_EQ(algorithm.CalcHorizontalOffset(AlignDirection::LEFT, endRule, CONTAINER_WIDTH, ROW2), 70.0f);
    EXPECT_EQ(algorithm.CalcHorizontalOffset(AlignDirection::TOP, endRule, CONTAINER_WIDTH, ROW2), 20.0f);
    AlignRule bottomRule;
    bottomRule.anchor = ROW1;
    bottomRule.vertical = VerticalAlign::BOTTOM;
    EXPECT_EQ(algorithm.CalcVerticalOffset(AlignDirection::TOP, bottomRule, CONTAINER_HEIGHT, ROW2), 70.0f);
    EXPECT_EQ(algorithm.CalcVerticalOffset(AlignDirection::LEFT, bottomRule, CONTAINER_HEIGHT, ROW2), 20.0f);
    EXPECT_EQ(algorithm.CalcAnchorWidth(false, CONTAINER_WIDTH, ROW1), 50.0f);
    EXPECT_EQ(algorithm.CalcAnchorWidth(true, CONTAINER_WIDTH, CONTAINER_ID), CONTAINER_WIDTH);
    algorithm.guidelines_[GUIDELINE_V] = { LineDirection::VERTICAL, 40.0f };
    EXPECT_EQ(algorithm.CalcAnchorWidth(false, CONTAINER_WIDTH, GUIDELINE_V), 0.0f);
    EXPECT_EQ(algorithm.CalcHorizontalOffset(AlignDirection::LEFT, endRule, CONTAINER_WIDTH, "ghost"), 0.0f);
    EXPECT_EQ(algorithm.CalcVerticalOffset(AlignDirection::TOP, bottomRule, CONTAINER_HEIGHT, "ghost"), 0.0f);
    MarginPropertyF margin;
    margin.left = 10.0f;
    margin.right = 20.0f;
    auto marginProp = std::make_unique<MarginPropertyF>(margin);
    EXPECT_EQ(algorithm.GetOriginMarginLeft(TextDirection::LTR, marginProp), 10.0f);
    EXPECT_EQ(algorithm.GetOriginMarginLeft(TextDirection::RTL, marginProp), 20.0f);
    std::unique_ptr<MarginPropertyF> nullMargin;
    EXPECT_EQ(algorithm.GetOriginMarginLeft(TextDirection::LTR, nullMargin), 0.0f);
}

/**
 * @tc.name: LayoutParamEnumDefaultTest
 * @tc.desc: Test the layout param calculations for guideline/barrier anchors and out of range align enums.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, LayoutParamEnumDefaultTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    auto container = CreateRelativeContainer([this, &row1, &row2](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        std::map<AlignDirection, AlignRule> rules1 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::START);
        AddAlignRule(rules1, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row1 = CreateRowNode(ROW1, 50.0f, 50.0f, rules1);
        row2 = CreateRow([this](RowModelNG rowModel) {
            ViewAbstract::SetInspectorId(ROW2);
            ViewAbstract::SetWidth(CalcLength(30.0f, DimensionUnit::PX));
            ViewAbstract::SetHeight(CalcLength(30.0f, DimensionUnit::PX));
            ViewAbstract::SetBias(BiasPair(0.5f, 0.5f));
        });
    });
    ASSERT_NE(container, nullptr);
    RelativeContainerLayoutAlgorithm algorithm;
    PrepareAlgorithm(container, algorithm, true);
    auto layoutWrapper = Referenced::RawPtr(container);
    ASSERT_NE(row2, nullptr);
    const auto& row2Flex = row2->GetLayoutProperty()->GetFlexItemProperty();
    ASSERT_NE(row2Flex, nullptr);
    row2Flex->ClearAlignValue();
    algorithm.guidelines_[GUIDELINE_V] = { LineDirection::VERTICAL, 40.0f };
    algorithm.recordOffsetMap_[GUIDELINE_V] = OffsetF(40.0f, 0.0f);
    AlignRule guidelineRule;
    guidelineRule.anchor = GUIDELINE_V;
    guidelineRule.horizontal = HorizontalAlign::START;
    algorithm.CalcHorizontalLayoutParam(AlignDirection::LEFT, guidelineRule, layoutWrapper, ROW2);
    EXPECT_EQ(row2Flex->GetAlignValue(AlignDirection::LEFT), 40.0f);
    algorithm.guidelines_[GUIDELINE_H] = { LineDirection::HORIZONTAL, 30.0f };
    algorithm.recordOffsetMap_[GUIDELINE_H] = OffsetF(0.0f, 30.0f);
    AlignRule horizontalGuidelineRule;
    horizontalGuidelineRule.anchor = GUIDELINE_H;
    horizontalGuidelineRule.horizontal = HorizontalAlign::START;
    algorithm.CalcHorizontalLayoutParam(AlignDirection::LEFT, horizontalGuidelineRule, layoutWrapper, ROW2);
    EXPECT_EQ(row2Flex->GetAlignValue(AlignDirection::LEFT), 0.0f);
    algorithm.barriers_[BARRIER1] = { BarrierDirection::RIGHT, {} };
    algorithm.recordOffsetMap_[BARRIER1] = OffsetF(40.0f, 0.0f);
    AlignRule barrierRule;
    barrierRule.anchor = BARRIER1;
    barrierRule.horizontal = HorizontalAlign::END;
    algorithm.CalcHorizontalLayoutParam(AlignDirection::LEFT, barrierRule, layoutWrapper, ROW2);
    EXPECT_EQ(row2Flex->GetAlignValue(AlignDirection::LEFT), 40.0f);
    algorithm.barriers_[BARRIER1] = { BarrierDirection::TOP, {} };
    algorithm.CalcHorizontalLayoutParam(AlignDirection::LEFT, barrierRule, layoutWrapper, ROW2);
    EXPECT_EQ(row2Flex->GetAlignValue(AlignDirection::LEFT), 0.0f);
    row2Flex->ClearAlignValue();
    AlignRule defaultRule;
    defaultRule.anchor = ROW1;
    defaultRule.horizontal = static_cast<HorizontalAlign>(0);
    algorithm.recordOffsetMap_[ROW1] = OffsetF(20.0f, 20.0f);
    algorithm.CalcHorizontalLayoutParam(AlignDirection::LEFT, defaultRule, layoutWrapper, ROW2);
    EXPECT_FALSE(row2Flex->GetAligned(AlignDirection::LEFT));
    row2Flex->ClearAlignValue();
    AlignRule verticalGuidelineRule;
    verticalGuidelineRule.anchor = GUIDELINE_H;
    verticalGuidelineRule.vertical = VerticalAlign::TOP;
    algorithm.CalcVerticalLayoutParam(AlignDirection::TOP, verticalGuidelineRule, layoutWrapper, ROW2);
    EXPECT_EQ(row2Flex->GetAlignValue(AlignDirection::TOP), 30.0f);
    AlignRule verticalWrongGuidelineRule;
    verticalWrongGuidelineRule.anchor = GUIDELINE_V;
    verticalWrongGuidelineRule.vertical = VerticalAlign::TOP;
    algorithm.CalcVerticalLayoutParam(AlignDirection::TOP, verticalWrongGuidelineRule, layoutWrapper, ROW2);
    EXPECT_EQ(row2Flex->GetAlignValue(AlignDirection::TOP), 0.0f);
    algorithm.barriers_[BARRIER1] = { BarrierDirection::BOTTOM, {} };
    algorithm.recordOffsetMap_[BARRIER1] = OffsetF(0.0f, 40.0f);
    AlignRule verticalBarrierRule;
    verticalBarrierRule.anchor = BARRIER1;
    verticalBarrierRule.vertical = VerticalAlign::BOTTOM;
    algorithm.CalcVerticalLayoutParam(AlignDirection::TOP, verticalBarrierRule, layoutWrapper, ROW2);
    EXPECT_EQ(row2Flex->GetAlignValue(AlignDirection::TOP), 40.0f);
    algorithm.barriers_[BARRIER1] = { BarrierDirection::LEFT, {} };
    algorithm.CalcVerticalLayoutParam(AlignDirection::TOP, verticalBarrierRule, layoutWrapper, ROW2);
    EXPECT_EQ(row2Flex->GetAlignValue(AlignDirection::TOP), 0.0f);
    row2Flex->ClearAlignValue();
    AlignRule verticalDefaultRule;
    verticalDefaultRule.anchor = ROW1;
    verticalDefaultRule.vertical = VerticalAlign::NONE;
    algorithm.CalcVerticalLayoutParam(AlignDirection::TOP, verticalDefaultRule, layoutWrapper, ROW2);
    EXPECT_FALSE(row2Flex->GetAligned(AlignDirection::TOP));
}

/**
 * @tc.name: ApiTenBehaviourTest
 * @tc.desc: Test the api ten early returns for barrier, guideline, chain, bias and offset helpers.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, ApiTenBehaviourTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    auto container = CreateRelativeContainer([this, &row1](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        ViewAbstract::SetBorderWidth(Dimension(2.0f));
        std::map<AlignDirection, AlignRule> rules1 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::START);
        AddAlignRule(rules1, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        ChainInfo chainInfo;
        chainInfo.direction = LineDirection::HORIZONTAL;
        chainInfo.style = ChainStyle::SPREAD;
        row1 = CreateChainRowNode(ROW1, 40.0f, 40.0f, rules1, chainInfo);
        std::vector<GuidelineInfo> guidelines;
        AddGuidelineStart(guidelines, GUIDELINE_H, LineDirection::HORIZONTAL, Dimension(30.0f, DimensionUnit::PX));
        model.SetGuideline(guidelines);
        std::vector<BarrierInfo> barriers;
        AddBarrier(barriers, BARRIER1, BarrierDirection::RIGHT, { ROW1 });
        model.SetBarrier(barriers);
    });
    ASSERT_NE(container, nullptr);
    UpdateConstraint(container);
    auto layoutWrapper = Referenced::RawPtr(container);
    RelativeContainerLayoutAlgorithm algorithm;
    algorithm.versionGreatorOrEqualToEleven_ = false;
    std::string chainName;
    algorithm.Initialize(layoutWrapper);
    EXPECT_TRUE(algorithm.guidelines_.empty());
    EXPECT_TRUE(algorithm.barriers_.empty());
    EXPECT_TRUE(algorithm.horizontalChains_.empty());
    EXPECT_TRUE(algorithm.verticalChains_.empty());
    EXPECT_FALSE(algorithm.IsGuideline(GUIDELINE_H));
    EXPECT_FALSE(algorithm.IsBarrier(BARRIER1));
    EXPECT_FALSE(algorithm.IsNodeInHorizontalChain(ROW1, chainName));
    EXPECT_FALSE(algorithm.IsNodeInVerticalChain(ROW1, chainName));
    EXPECT_FALSE(algorithm.IsAlignRuleInChain(AlignDirection::LEFT, ROW1));
    algorithm.CalcGuideline(layoutWrapper);
    algorithm.CalcBarrier(layoutWrapper);
    algorithm.CheckChain(layoutWrapper);
    algorithm.RecordSizeInChain(ROW1);
    algorithm.ResetCalculatedChain();
    algorithm.MeasureSelf(layoutWrapper);
    EXPECT_EQ(algorithm.CalcBias(ROW1), OffsetF(0.0f, 0.0f));
    AlignRule horizontalRule;
    horizontalRule.anchor = CONTAINER_ID;
    horizontalRule.horizontal = HorizontalAlign::START;
    AlignRule verticalRule;
    verticalRule.anchor = CONTAINER_ID;
    verticalRule.vertical = VerticalAlign::TOP;
    EXPECT_EQ(algorithm.CalcHorizontalOffset(AlignDirection::LEFT, horizontalRule, CONTAINER_WIDTH, ROW1), 0.0f);
    EXPECT_EQ(algorithm.CalcVerticalOffset(AlignDirection::TOP, verticalRule, CONTAINER_HEIGHT, ROW1), 0.0f);
    EXPECT_EQ(algorithm.CalcAnchorWidth(false, CONTAINER_WIDTH, ROW1), 0.0f);
    algorithm.CalcBarrier(nullptr);
    algorithm.CalcGuideline(nullptr);
    algorithm.MeasureSelf(nullptr);
    LayoutConstraintF childConstraint = container->GetLayoutProperty()->CreateChildConstraint();
    algorithm.MeasureWrapContent(ROW1, childConstraint);
}

/**
 * @tc.name: ChainRegisterTest
 * @tc.desc: Test the horizontal chain registration and the resulting child offsets.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, ChainRegisterTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    RefPtr<FrameNode> row3;
    auto container = CreateRelativeContainer([this, &row1, &row2, &row3](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        ChainInfo chainInfo;
        chainInfo.direction = LineDirection::HORIZONTAL;
        chainInfo.style = ChainStyle::SPREAD;
        std::map<AlignDirection, AlignRule> rules1 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::START);
        AddAlignRule(rules1, AlignDirection::RIGHT, ROW2, HorizontalAlign::START);
        AddAlignRule(rules1, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row1 = CreateChainRowNode(ROW1, 80.0f, 50.0f, rules1, chainInfo);
        std::map<AlignDirection, AlignRule> rules2 = HorizontalRule(AlignDirection::LEFT, ROW1,
            HorizontalAlign::END);
        AddAlignRule(rules2, AlignDirection::RIGHT, ROW3, HorizontalAlign::START);
        AddAlignRule(rules2, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row2 = CreateRowNode(ROW2, 80.0f, 50.0f, rules2);
        std::map<AlignDirection, AlignRule> rules3 = HorizontalRule(AlignDirection::LEFT, ROW2,
            HorizontalAlign::END);
        AddAlignRule(rules3, AlignDirection::RIGHT, CONTAINER_ID, HorizontalAlign::END);
        AddAlignRule(rules3, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row3 = CreateRowNode(ROW3, 80.0f, 50.0f, rules3);
    });
    ASSERT_NE(container, nullptr);
    auto layoutWrapper = Referenced::RawPtr(container);
    RelativeContainerLayoutAlgorithm algorithm;
    PrepareAlgorithm(container, algorithm);
    ASSERT_EQ(algorithm.horizontalChains_.size(), 1u);
    ASSERT_TRUE(algorithm.horizontalChains_.count(ROW1) > 0);
    auto& chainParam = algorithm.horizontalChains_[ROW1];
    EXPECT_EQ(chainParam.ids.size(), 3u);
    EXPECT_EQ(chainParam.chainStyle, ChainStyle::SPREAD);
    EXPECT_FALSE(chainParam.isCalculated);
    EXPECT_FALSE(algorithm.isChainWeightMode_);
    EXPECT_TRUE(algorithm.isHorizontalRelyOnContainer_);
    algorithm.MeasureChild(layoutWrapper);
    EXPECT_EQ(algorithm.recordOffsetMap_[ROW1], OffsetF(15.0f, 0.0f));
    EXPECT_EQ(algorithm.recordOffsetMap_[ROW2], OffsetF(110.0f, 0.0f));
    EXPECT_EQ(algorithm.recordOffsetMap_[ROW3], OffsetF(205.0f, 0.0f));
    EXPECT_TRUE(chainParam.isCalculated);
    algorithm.Layout(layoutWrapper);
    EXPECT_EQ(row1->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(15.0f, 0.0f));
    EXPECT_EQ(row2->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(110.0f, 0.0f));
    EXPECT_EQ(row3->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(205.0f, 0.0f));
}

/**
 * @tc.name: ChainRejectTest
 * @tc.desc: Test the chain rejection matrix for missing direction/style/anchors and degenerate chains.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, ChainRejectTest, TestSize.Level1)
{
    auto buildContainer = [this](const ChainInfo& chainInfo, const std::map<AlignDirection, AlignRule>& row1Rules,
        const std::map<AlignDirection, AlignRule>& row2Rules) -> RefPtr<FrameNode> {
        return CreateRelativeContainer([this, chainInfo, row1Rules, row2Rules](RelativeContainerModelNG model) {
            ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
            ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
            CreateChainRowNode(ROW1, 80.0f, 80.0f, row1Rules, chainInfo);
            CreateRowNode(ROW2, 40.0f, 40.0f, row2Rules);
            CreateRowNode(ROW3, 40.0f, 40.0f);
        });
    };
    auto hasNoChain = [](const RefPtr<FrameNode>& container, bool verticalChain) {
        if (container == nullptr) {
            return false;
        }
        RelativeContainerLayoutAlgorithm algorithm;
        PrepareAlgorithm(container, algorithm);
        return verticalChain ? algorithm.verticalChains_.empty() : algorithm.horizontalChains_.empty();
    };
    std::map<AlignDirection, AlignRule> emptyRules;
    std::map<AlignDirection, AlignRule> neighbourRules = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
        HorizontalAlign::START);
    AddAlignRule(neighbourRules, AlignDirection::RIGHT, ROW2, HorizontalAlign::START);
    std::map<AlignDirection, AlignRule> leftRightRules = neighbourRules;
    std::map<AlignDirection, AlignRule> rightOnlyRules = HorizontalRule(AlignDirection::RIGHT, ROW2,
        HorizontalAlign::START);
    std::map<AlignDirection, AlignRule> leftOnlyRules = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
        HorizontalAlign::START);
    std::map<AlignDirection, AlignRule> illegalLeftRules = HorizontalRule(AlignDirection::LEFT, "ghost",
        HorizontalAlign::START);
    AddAlignRule(illegalLeftRules, AlignDirection::RIGHT, CONTAINER_ID, HorizontalAlign::END);
    std::map<AlignDirection, AlignRule> containerOnlyRules = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
        HorizontalAlign::START);
    AddAlignRule(containerOnlyRules, AlignDirection::RIGHT, CONTAINER_ID, HorizontalAlign::END);
    std::map<AlignDirection, AlignRule> nextTopRules = VerticalRule(AlignDirection::TOP, CONTAINER_ID,
        VerticalAlign::TOP);
    std::map<AlignDirection, AlignRule> wrongAnchorRules = HorizontalRule(AlignDirection::LEFT, ROW3,
        HorizontalAlign::END);
    AddAlignRule(wrongAnchorRules, AlignDirection::RIGHT, CONTAINER_ID, HorizontalAlign::END);
    std::map<AlignDirection, AlignRule> startAnchorRules = HorizontalRule(AlignDirection::LEFT, ROW1,
        HorizontalAlign::START);
    AddAlignRule(startAnchorRules, AlignDirection::RIGHT, CONTAINER_ID, HorizontalAlign::END);
    std::map<AlignDirection, AlignRule> topBottomRules = VerticalRule(AlignDirection::TOP, CONTAINER_ID,
        VerticalAlign::TOP);
    AddAlignRule(topBottomRules, AlignDirection::BOTTOM, ROW2, VerticalAlign::TOP);
    std::map<AlignDirection, AlignRule> topOnlyRules = VerticalRule(AlignDirection::TOP, CONTAINER_ID,
        VerticalAlign::TOP);
    std::map<AlignDirection, AlignRule> illegalTopRules = VerticalRule(AlignDirection::TOP, "ghost",
        VerticalAlign::TOP);
    AddAlignRule(illegalTopRules, AlignDirection::BOTTOM, CONTAINER_ID, VerticalAlign::BOTTOM);
    ChainInfo directionOnly;
    directionOnly.direction = LineDirection::HORIZONTAL;
    ChainInfo horizontalChain;
    horizontalChain.direction = LineDirection::HORIZONTAL;
    horizontalChain.style = ChainStyle::SPREAD;
    ChainInfo verticalDirectionOnly;
    verticalDirectionOnly.direction = LineDirection::VERTICAL;
    ChainInfo verticalChain;
    verticalChain.direction = LineDirection::VERTICAL;
    verticalChain.style = ChainStyle::SPREAD;
    // Chain style without direction: neither a horizontal nor a vertical chain is registered.
    auto directionless = buildContainer(ChainInfo {}, emptyRules, emptyRules);
    EXPECT_TRUE(hasNoChain(directionless, false));
    EXPECT_TRUE(hasNoChain(directionless, true));
    // Direction without a style / missing rules / illegal anchors: all rejected before registration.
    EXPECT_TRUE(hasNoChain(buildContainer(directionOnly, leftRightRules, emptyRules), false));
    EXPECT_TRUE(hasNoChain(buildContainer(horizontalChain, rightOnlyRules, emptyRules), false));
    EXPECT_TRUE(hasNoChain(buildContainer(horizontalChain, leftOnlyRules, emptyRules), false));
    EXPECT_TRUE(hasNoChain(buildContainer(horizontalChain, illegalLeftRules, emptyRules), false));
    EXPECT_TRUE(hasNoChain(buildContainer(horizontalChain, containerOnlyRules, emptyRules), false));
    // Chain walk stops on a plain next node, on incomplete rules and on a wrong anchor/alignment.
    EXPECT_TRUE(hasNoChain(buildContainer(horizontalChain, neighbourRules, emptyRules), false));
    EXPECT_TRUE(hasNoChain(buildContainer(horizontalChain, neighbourRules, nextTopRules), false));
    EXPECT_TRUE(hasNoChain(buildContainer(horizontalChain, neighbourRules, wrongAnchorRules), false));
    EXPECT_TRUE(hasNoChain(buildContainer(horizontalChain, neighbourRules, startAnchorRules), false));
    // Vertical chain rejections mirror the horizontal ones.
    EXPECT_TRUE(hasNoChain(buildContainer(verticalDirectionOnly, topBottomRules, emptyRules), true));
    EXPECT_TRUE(hasNoChain(buildContainer(verticalChain, topOnlyRules, emptyRules), true));
    EXPECT_TRUE(hasNoChain(buildContainer(verticalChain, illegalTopRules, emptyRules), true));
}

/**
 * @tc.name: ChainWeightHorizontalTest
 * @tc.desc: Test the horizontal chain weight mode including the remaining space distribution.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, ChainWeightHorizontalTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    RefPtr<FrameNode> row3;
    auto container = CreateRelativeContainer([this, &row1, &row2, &row3](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        ChainInfo chainInfo;
        chainInfo.direction = LineDirection::HORIZONTAL;
        chainInfo.style = ChainStyle::SPREAD;
        std::map<AlignDirection, AlignRule> rules1 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::START);
        AddAlignRule(rules1, AlignDirection::RIGHT, ROW2, HorizontalAlign::START);
        AddAlignRule(rules1, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row1 = CreateChainRowNode(ROW1, 80.0f, 50.0f, rules1, chainInfo,
            ChainWeightPair(1.0f, 0.0f));
        std::map<AlignDirection, AlignRule> rules2 = HorizontalRule(AlignDirection::LEFT, ROW1,
            HorizontalAlign::END);
        AddAlignRule(rules2, AlignDirection::RIGHT, ROW3, HorizontalAlign::START);
        AddAlignRule(rules2, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row2 = CreateChainRowNode(ROW2, 80.0f, 50.0f, rules2, ChainInfo {},
            ChainWeightPair(1.0f, 0.0f));
        std::map<AlignDirection, AlignRule> rules3 = HorizontalRule(AlignDirection::LEFT, ROW2,
            HorizontalAlign::END);
        AddAlignRule(rules3, AlignDirection::RIGHT, CONTAINER_ID, HorizontalAlign::END);
        AddAlignRule(rules3, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row3 = CreateRowNode(ROW3, 80.0f, 50.0f, rules3);
    });
    ASSERT_NE(container, nullptr);
    auto layoutWrapper = Referenced::RawPtr(container);
    RelativeContainerLayoutAlgorithm algorithm;
    PrepareAlgorithm(container, algorithm);
    ASSERT_EQ(algorithm.horizontalChains_.size(), 1u);
    ASSERT_TRUE(algorithm.horizontalChains_.count(ROW1) > 0);
    auto& chainParam = algorithm.horizontalChains_[ROW1];
    EXPECT_TRUE(algorithm.isChainWeightMode_);
    EXPECT_FLOAT_EQ(chainParam.totalChainWeight, 2.0f);
    algorithm.MeasureChild(layoutWrapper);
    algorithm.MeasureChainWeight(layoutWrapper);
    EXPECT_TRUE(chainParam.isWeightCalculated);
    EXPECT_FLOAT_EQ(chainParam.remainingSpace, 220.0f);
    EXPECT_FLOAT_EQ(chainParam.itemSize[ROW1].value_or(0.0f), 110.0f);
    EXPECT_FLOAT_EQ(chainParam.itemSize[ROW2].value_or(0.0f), 110.0f);
    EXPECT_FLOAT_EQ(chainParam.itemSize[ROW3].value_or(0.0f), 80.0f);
    EXPECT_FLOAT_EQ(row1->GetGeometryNode()->GetFrameSize().Width(), 110.0f);
    EXPECT_FLOAT_EQ(row2->GetGeometryNode()->GetFrameSize().Width(), 110.0f);
    EXPECT_FLOAT_EQ(row3->GetGeometryNode()->GetFrameSize().Width(), 80.0f);
    EXPECT_TRUE(algorithm.recordOffsetMap_.count(ROW1) > 0);
    EXPECT_TRUE(algorithm.recordOffsetMap_.count(ROW2) > 0);
    EXPECT_TRUE(algorithm.recordOffsetMap_.count(ROW3) > 0);
}

/**
 * @tc.name: ChainWeightVerticalTest
 * @tc.desc: Test the vertical chain weight mode and the vertical spread offsets.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, ChainWeightVerticalTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    RefPtr<FrameNode> row3;
    auto container = CreateRelativeContainer([this, &row1, &row2, &row3](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        ChainInfo chainInfo;
        chainInfo.direction = LineDirection::VERTICAL;
        chainInfo.style = ChainStyle::SPREAD;
        std::map<AlignDirection, AlignRule> rules1 = VerticalRule(AlignDirection::TOP, CONTAINER_ID,
            VerticalAlign::TOP);
        AddAlignRule(rules1, AlignDirection::BOTTOM, ROW2, VerticalAlign::TOP);
        row1 = CreateChainRowNode(ROW1, 80.0f, 50.0f, rules1, chainInfo, ChainWeightPair(0.0f, 1.0f));
        std::map<AlignDirection, AlignRule> rules2 = VerticalRule(AlignDirection::TOP, ROW1,
            VerticalAlign::BOTTOM);
        AddAlignRule(rules2, AlignDirection::BOTTOM, ROW3, VerticalAlign::TOP);
        row2 = CreateChainRowNode(ROW2, 80.0f, 50.0f, rules2, ChainInfo {}, ChainWeightPair(0.0f, 1.0f));
        std::map<AlignDirection, AlignRule> rules3 = VerticalRule(AlignDirection::TOP, ROW2,
            VerticalAlign::BOTTOM);
        AddAlignRule(rules3, AlignDirection::BOTTOM, CONTAINER_ID, VerticalAlign::BOTTOM);
        row3 = CreateRowNode(ROW3, 80.0f, 50.0f, rules3);
    });
    ASSERT_NE(container, nullptr);
    auto layoutWrapper = Referenced::RawPtr(container);
    RelativeContainerLayoutAlgorithm algorithm;
    PrepareAlgorithm(container, algorithm);
    ASSERT_EQ(algorithm.verticalChains_.size(), 1u);
    ASSERT_TRUE(algorithm.verticalChains_.count(ROW1) > 0);
    auto& chainParam = algorithm.verticalChains_[ROW1];
    EXPECT_TRUE(algorithm.isChainWeightMode_);
    EXPECT_FLOAT_EQ(chainParam.totalChainWeight, 2.0f);
    algorithm.MeasureChild(layoutWrapper);
    algorithm.MeasureChainWeight(layoutWrapper);
    EXPECT_TRUE(chainParam.isWeightCalculated);
    EXPECT_FLOAT_EQ(chainParam.remainingSpace, 250.0f);
    EXPECT_FLOAT_EQ(row1->GetGeometryNode()->GetFrameSize().Height(), 125.0f);
    EXPECT_FLOAT_EQ(row2->GetGeometryNode()->GetFrameSize().Height(), 125.0f);
    EXPECT_FLOAT_EQ(row3->GetGeometryNode()->GetFrameSize().Height(), 50.0f);
    EXPECT_TRUE(algorithm.recordOffsetMap_.count(ROW1) > 0);
    EXPECT_TRUE(algorithm.recordOffsetMap_.count(ROW2) > 0);
    EXPECT_TRUE(algorithm.recordOffsetMap_.count(ROW3) > 0);
}

/**
 * @tc.name: ChainWeightHelperTest
 * @tc.desc: Test CalcChainWeightSize, HasWeight, InitRemainingSpace and RecordSizeInChain in isolation.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, ChainWeightHelperTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    auto container = CreateRelativeContainer([this, &row1, &row2](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        row1 = CreateRow([this](RowModelNG rowModel) {
            ViewAbstract::SetInspectorId(ROW1);
            ViewAbstract::SetWidth(CalcLength(40.0f, DimensionUnit::PX));
            ViewAbstract::SetHeight(CalcLength(40.0f, DimensionUnit::PX));
            ViewAbstract::SetChainWeight(ChainWeightPair(1.0f, 1.0f));
        });
        ChainInfo chainInfo;
        chainInfo.direction = LineDirection::HORIZONTAL;
        chainInfo.style = ChainStyle::SPREAD;
        row2 = CreateChainRowNode(ROW2, 30.0f, 30.0f, {}, chainInfo);
    });
    ASSERT_NE(container, nullptr);
    UpdateConstraint(container);
    RelativeContainerLayoutAlgorithm algorithm;
    algorithm.versionGreatorOrEqualToEleven_ = true;
    algorithm.Initialize(Referenced::RawPtr(container));
    ASSERT_TRUE(algorithm.idNodeMap_.count(ROW1) > 0);
    ASSERT_TRUE(algorithm.idNodeMap_.count(ROW2) > 0);
    algorithm.idNodeMap_[ROW1].layoutWrapper->GetGeometryNode()->SetFrameSize(SizeF(40.0f, 40.0f));
    algorithm.idNodeMap_[ROW2].layoutWrapper->GetGeometryNode()->SetFrameSize(SizeF(30.0f, 30.0f));
    const auto& weightedFlex = row1->GetLayoutProperty()->GetFlexItemProperty();
    const auto& plainFlex = row2->GetLayoutProperty()->GetFlexItemProperty();
    ASSERT_NE(weightedFlex, nullptr);
    ASSERT_NE(plainFlex, nullptr);
    EXPECT_TRUE(algorithm.HasWeight(weightedFlex, LineDirection::HORIZONTAL));
    EXPECT_TRUE(algorithm.HasWeight(weightedFlex, LineDirection::VERTICAL));
    EXPECT_FALSE(algorithm.HasWeight(plainFlex, LineDirection::HORIZONTAL));
    EXPECT_FALSE(algorithm.HasWeight(plainFlex, LineDirection::VERTICAL));
    algorithm.horizontalChains_["chainH"] = RelativeContainerLayoutAlgorithm::ChainParam {};
    algorithm.horizontalChainNodeMap_[ROW1] = "chainH";
    algorithm.horizontalChainNodeMap_[ROW2] = "chainH";
    algorithm.verticalChains_["chainV"] = RelativeContainerLayoutAlgorithm::ChainParam {};
    algorithm.verticalChainNodeMap_[ROW1] = "chainV";
    algorithm.verticalChainNodeMap_[ROW2] = "chainV";
    algorithm.RecordSizeInChain("ghost");
    EXPECT_TRUE(algorithm.horizontalChains_["chainH"].itemSize.empty());
    algorithm.RecordSizeInChain(ROW2);
    EXPECT_FLOAT_EQ(algorithm.horizontalChains_["chainH"].itemSize[ROW2].value_or(0.0f), 30.0f);
    EXPECT_FLOAT_EQ(algorithm.horizontalChains_["chainH"].remainingSpace, -30.0f);
    EXPECT_FLOAT_EQ(algorithm.verticalChains_["chainV"].itemSize[ROW2].value_or(0.0f), 30.0f);
    EXPECT_FLOAT_EQ(algorithm.verticalChains_["chainV"].remainingSpace, -30.0f);
    algorithm.RecordSizeInChain(ROW1);
    EXPECT_TRUE(algorithm.horizontalChains_["chainH"].itemSize.count(ROW1) == 0);
    auto& horizontalChain = algorithm.horizontalChains_["chainH"];
    horizontalChain.anchorHead = { .anchor = CONTAINER_ID, .horizontal = HorizontalAlign::START };
    horizontalChain.anchorTail = { .anchor = CONTAINER_ID, .horizontal = HorizontalAlign::END };
    algorithm.InitRemainingSpace(horizontalChain, LineDirection::HORIZONTAL);
    EXPECT_TRUE(horizontalChain.isWeightCalculated);
    EXPECT_FLOAT_EQ(horizontalChain.remainingSpace, 270.0f);
    algorithm.InitRemainingSpace(horizontalChain, LineDirection::HORIZONTAL);
    EXPECT_FLOAT_EQ(horizontalChain.remainingSpace, 270.0f);
    auto& verticalChain = algorithm.verticalChains_["chainV"];
    verticalChain.anchorHead = { .anchor = CONTAINER_ID, .vertical = VerticalAlign::TOP };
    verticalChain.anchorTail = { .anchor = CONTAINER_ID, .vertical = VerticalAlign::BOTTOM };
    algorithm.InitRemainingSpace(verticalChain, LineDirection::VERTICAL);
    EXPECT_FLOAT_EQ(verticalChain.remainingSpace, 270.0f);
    LayoutConstraintF childConstraint;
    horizontalChain.remainingSpace = -10.0f;
    horizontalChain.totalChainWeight = 2.0f;
    horizontalChain.isWeightCalculated = true;
    algorithm.CalcChainWeightSize(weightedFlex, childConstraint, ROW1, LineDirection::HORIZONTAL);
    EXPECT_FLOAT_EQ(childConstraint.selfIdealSize.Width().value_or(-1.0f), 0.0f);
    childConstraint = LayoutConstraintF {};
    horizontalChain.remainingSpace = 100.0f;
    algorithm.CalcChainWeightSize(weightedFlex, childConstraint, ROW1, LineDirection::HORIZONTAL);
    EXPECT_FLOAT_EQ(childConstraint.selfIdealSize.Width().value_or(-1.0f), 50.0f);
    childConstraint = LayoutConstraintF {};
    verticalChain.remainingSpace = 80.0f;
    verticalChain.totalChainWeight = 2.0f;
    verticalChain.isWeightCalculated = true;
    algorithm.CalcChainWeightSize(weightedFlex, childConstraint, ROW1, LineDirection::VERTICAL);
    EXPECT_FLOAT_EQ(childConstraint.selfIdealSize.Height().value_or(-1.0f), 40.0f);
    childConstraint = LayoutConstraintF {};
    algorithm.CalcChainWeightSize(weightedFlex, childConstraint, "ghost", LineDirection::HORIZONTAL);
    EXPECT_FALSE(childConstraint.selfIdealSize.Width().has_value());
}

/**
 * @tc.name: ChainQueryTest
 * @tc.desc: Test the chain query, reliance and reset helpers with hand crafted algorithm state.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, ChainQueryTest, TestSize.Level1)
{
    RelativeContainerLayoutAlgorithm algorithm;
    std::string chainName;
    EXPECT_FALSE(algorithm.IsNodeInHorizontalChain(ROW1, chainName));
    EXPECT_FALSE(algorithm.IsNodeInVerticalChain(ROW1, chainName));
    algorithm.versionGreatorOrEqualToEleven_ = true;
    EXPECT_FALSE(algorithm.IsNodeInHorizontalChain(ROW1, chainName));
    EXPECT_FALSE(algorithm.IsNodeInVerticalChain(ROW1, chainName));
    algorithm.horizontalChains_["chainH"] = RelativeContainerLayoutAlgorithm::ChainParam {};
    algorithm.horizontalChainNodeMap_[ROW1] = "chainH";
    EXPECT_TRUE(algorithm.IsNodeInHorizontalChain(ROW1, chainName));
    EXPECT_EQ(chainName, "chainH");
    EXPECT_FALSE(algorithm.IsNodeInHorizontalChain(ROW2, chainName));
    algorithm.verticalChains_["chainV"] = RelativeContainerLayoutAlgorithm::ChainParam {};
    algorithm.verticalChainNodeMap_[ROW1] = "chainV";
    algorithm.verticalChainNodeMap_[ROW2] = "chainV";
    EXPECT_TRUE(algorithm.IsNodeInVerticalChain(ROW2, chainName));
    EXPECT_EQ(chainName, "chainV");
    EXPECT_TRUE(algorithm.IsNodeInVerticalChain(ROW1, chainName));
    EXPECT_EQ(chainName, "chainV");
    EXPECT_TRUE(algorithm.IsNodeInChain(ROW1, chainName, LineDirection::HORIZONTAL));
    EXPECT_TRUE(algorithm.IsNodeInChain(ROW2, chainName, LineDirection::VERTICAL));
    algorithm.horizontalChainNodeMap_["stale"] = "missingChain";
    chainName.clear();
    EXPECT_FALSE(algorithm.IsNodeInHorizontalChain("stale", chainName));
    algorithm.horizontalChains_.clear();
    algorithm.verticalChains_["chainV"].isCalculated = true;
    algorithm.ResetCalculatedChain();
    EXPECT_TRUE(algorithm.verticalChains_["chainV"].isCalculated);
    algorithm.horizontalChains_["chainH"] = RelativeContainerLayoutAlgorithm::ChainParam {};
    algorithm.horizontalChains_["chainH"].isCalculated = true;
    algorithm.ResetCalculatedChain();
    EXPECT_FALSE(algorithm.verticalChains_["chainV"].isCalculated);
    EXPECT_FALSE(algorithm.horizontalChains_["chainH"].isCalculated);
    algorithm.InsertToHorizontalReliedOnMap(ROW1, ROW2);
    algorithm.InsertToHorizontalReliedOnMap(ROW3, ROW2);
    EXPECT_EQ(algorithm.horizontalRelyMap_[ROW2].size(), 2u);
    algorithm.InsertToVerticalReliedOnMap(ROW1, ROW2);
    algorithm.InsertToVerticalReliedOnMap(ROW3, ROW2);
    EXPECT_EQ(algorithm.verticalRelyMap_[ROW2].size(), 2u);
    algorithm.InsertToReliedOnMap(ROW1, ROW2);
    algorithm.InsertToReliedOnMap(ROW1, ROW3);
    EXPECT_EQ(algorithm.reliedOnMap_[ROW1].size(), 2u);
    algorithm.horizontalRelyMap_["other"] = { ROW2 };
    EXPECT_FALSE(algorithm.IsUnionRelyOnContainer(LineDirection::HORIZONTAL, "other"));
    EXPECT_FALSE(algorithm.IsUnionRelyOnContainer(LineDirection::HORIZONTAL, "unknown"));
    algorithm.horizontalRelyMap_["direct"] = { CONTAINER_ID };
    algorithm.horizontalRelyMap_["deep"] = { ROW1 };
    algorithm.horizontalRelyMap_[ROW1] = { "direct" };
    EXPECT_TRUE(algorithm.IsUnionRelyOnContainer(LineDirection::HORIZONTAL, "direct"));
    EXPECT_TRUE(algorithm.IsUnionRelyOnContainer(LineDirection::HORIZONTAL, "deep"));
    algorithm.verticalRelyMap_["vdirect"] = { CONTAINER_ID };
    algorithm.verticalRelyMap_["vdeep"] = { "vdirect" };
    algorithm.verticalRelyMap_["vother"] = { ROW2 };
    EXPECT_TRUE(algorithm.IsUnionRelyOnContainer(LineDirection::VERTICAL, "vdeep"));
    EXPECT_FALSE(algorithm.IsUnionRelyOnContainer(LineDirection::VERTICAL, "vother"));
    EXPECT_FALSE(algorithm.IsUnionRelyOnContainer(LineDirection::VERTICAL, "unknown"));
    algorithm.horizontalChainNodeMap_[ROW1] = "chainH";
    algorithm.verticalChainNodeMap_[ROW2] = "chainV";
    EXPECT_TRUE(algorithm.IsAlignRuleInChain(AlignDirection::LEFT, ROW1));
    EXPECT_TRUE(algorithm.IsAlignRuleInChain(AlignDirection::RIGHT, ROW1));
    EXPECT_TRUE(algorithm.IsAlignRuleInChain(AlignDirection::TOP, ROW2));
    EXPECT_TRUE(algorithm.IsAlignRuleInChain(AlignDirection::BOTTOM, ROW2));
    EXPECT_FALSE(algorithm.IsAlignRuleInChain(AlignDirection::MIDDLE, ROW1));
    EXPECT_FALSE(algorithm.IsAlignRuleInChain(AlignDirection::CENTER, ROW2));
    EXPECT_FALSE(algorithm.IsAlignRuleInChain(AlignDirection::LEFT, ROW3));
    algorithm.guidelines_[GUIDELINE_V] = { LineDirection::VERTICAL, 10.0f };
    EXPECT_TRUE(algorithm.IsRelyOnContainer(CONTAINER_ID));
    EXPECT_TRUE(algorithm.IsRelyOnContainer(GUIDELINE_V));
    EXPECT_FALSE(algorithm.IsRelyOnContainer(ROW1));
}

/**
 * @tc.name: GoneAnchorApiVersionTest
 * @tc.desc: Test the gone anchor deactivation which depends on the target api version.
 * @tc.type: FUNC
 */
HWTEST_F(RelativeContainerCoverageTestNG, GoneAnchorApiVersionTest, TestSize.Level1)
{
    RefPtr<FrameNode> row1;
    RefPtr<FrameNode> row2;
    auto container = CreateRelativeContainer([this, &row1, &row2](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        std::map<AlignDirection, AlignRule> rules1 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::START);
        AddAlignRule(rules1, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row1 = CreateRowNode(ROW1, 40.0f, 40.0f, rules1);
        std::map<AlignDirection, AlignRule> rules2 = HorizontalRule(AlignDirection::LEFT, ROW1,
            HorizontalAlign::END);
        AddAlignRule(rules2, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row2 = CreateRowNode(ROW2, 20.0f, 20.0f, rules2);
    });
    ASSERT_NE(container, nullptr);
    ASSERT_NE(row1, nullptr);
    row1->GetLayoutProperty()->UpdateVisibility(VisibleType::GONE);
    {
        RelativeContainerLayoutAlgorithm algorithm;
        PrepareAlgorithm(container, algorithm);
        EXPECT_FALSE(algorithm.idNodeMap_[ROW2].layoutWrapper->IsActive());
    }
    APIVersionGuard guard(12);
    RefPtr<FrameNode> row3;
    RefPtr<FrameNode> row4;
    auto modernContainer = CreateRelativeContainer([this, &row3, &row4](RelativeContainerModelNG model) {
        ViewAbstract::SetWidth(CalcLength(CONTAINER_WIDTH, DimensionUnit::PX));
        ViewAbstract::SetHeight(CalcLength(CONTAINER_HEIGHT, DimensionUnit::PX));
        std::map<AlignDirection, AlignRule> rules1 = HorizontalRule(AlignDirection::LEFT, CONTAINER_ID,
            HorizontalAlign::START);
        AddAlignRule(rules1, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row3 = CreateRowNode(ROW3, 40.0f, 40.0f, rules1);
        std::map<AlignDirection, AlignRule> rules2 = HorizontalRule(AlignDirection::LEFT, ROW3,
            HorizontalAlign::END);
        AddAlignRule(rules2, AlignDirection::TOP, CONTAINER_ID, VerticalAlign::TOP);
        row4 = CreateRowNode("row4", 20.0f, 20.0f, rules2);
    });
    ASSERT_NE(modernContainer, nullptr);
    ASSERT_NE(row3, nullptr);
    row3->GetLayoutProperty()->UpdateVisibility(VisibleType::GONE);
    RelativeContainerLayoutAlgorithm modernAlgorithm;
    PrepareAlgorithm(modernContainer, modernAlgorithm);
    EXPECT_TRUE(modernAlgorithm.idNodeMap_["row4"].layoutWrapper->IsActive());
}

} // namespace OHOS::Ace::NG
