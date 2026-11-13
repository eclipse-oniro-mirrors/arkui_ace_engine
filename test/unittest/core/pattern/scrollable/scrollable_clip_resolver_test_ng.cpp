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

#include "test/unittest/core/pattern/test_ng.h"

#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/pattern/scrollable/scrollable_utils.h"
#include "core/components_ng/pattern/scrollable/scrollable_paint_property.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
namespace {
// Real enumerators of the frozen ImmersiveStrategy contract (PR #88798 skeleton +
// the four scrollable values appended by this feature). Constructing values via
// static_cast of out-of-range integers is forbidden (UB under -fsanitize=enum).
const std::unordered_set<ImmersiveStrategy> ALL_SCROLL_STRATEGIES = {
    ImmersiveStrategy::GRID_EXPAND_SAFE_AREA,
    ImmersiveStrategy::LIST_EXPAND_SAFE_AREA,
    ImmersiveStrategy::SCROLL_EXPAND_SAFE_AREA,
    ImmersiveStrategy::WATER_FLOW_EXPAND_SAFE_AREA,
};

RefPtr<FrameNode> CreateTagNode(const char* tag, int32_t nodeId)
{
    return FrameNode::CreateFrameNode(tag, nodeId, AceType::MakeRefPtr<Pattern>());
}
} // namespace

// Applies a strategy set for the duration of one test and restores the empty set
// (replace semantics) so cases never leak state into each other.
class ScrollableClipResolverTestNg : public TestNG {
public:
    void SetUp() override
    {
        TestNG::SetUp();
        ClearStrategies();
    }

    void TearDown() override
    {
        ClearStrategies();
        TestNG::TearDown();
    }

    void ApplyStrategies(const std::unordered_set<ImmersiveStrategy>& types)
    {
        auto pipeline = MockPipelineContext::GetCurrent();
        ASSERT_NE(pipeline, nullptr);
        pipeline->ApplyDefaultImmersiveStrategy(types);
    }

    void ClearStrategies()
    {
        auto pipeline = MockPipelineContext::GetCurrent();
        if (pipeline) {
            pipeline->ApplyDefaultImmersiveStrategy({});
        }
    }

    int32_t NextNodeId()
    {
        return ++nodeIdSeed_;
    }

private:
    int32_t nodeIdSeed_ = 50000;
};

/**
 * @tc.name: ClipResolverTagMapping001
 * @tc.desc: The four scrollable tags map to their own strategy; any other tag maps to none.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableClipResolverTestNg, ClipResolverTagMapping001, TestSize.Level1)
{
    EXPECT_EQ(ScrollableUtils::StrategyForClipHostTag(V2::GRID_ETS_TAG), ImmersiveStrategy::GRID_EXPAND_SAFE_AREA);
    EXPECT_EQ(ScrollableUtils::StrategyForClipHostTag(V2::LIST_ETS_TAG), ImmersiveStrategy::LIST_EXPAND_SAFE_AREA);
    EXPECT_EQ(ScrollableUtils::StrategyForClipHostTag(V2::SCROLL_ETS_TAG), ImmersiveStrategy::SCROLL_EXPAND_SAFE_AREA);
    EXPECT_EQ(
        ScrollableUtils::StrategyForClipHostTag(V2::WATERFLOW_ETS_TAG),
        ImmersiveStrategy::WATER_FLOW_EXPAND_SAFE_AREA);

    EXPECT_FALSE(ScrollableUtils::StrategyForClipHostTag(V2::COLUMN_ETS_TAG).has_value());
    EXPECT_FALSE(ScrollableUtils::StrategyForClipHostTag(V2::TEXT_ETS_TAG).has_value());
    EXPECT_FALSE(ScrollableUtils::StrategyForClipHostTag(V2::ARC_LIST_ETS_TAG).has_value());
    EXPECT_FALSE(ScrollableUtils::StrategyForClipHostTag("").has_value());
}

/**
 * @tc.name: ClipResolverNullSafety001
 * @tc.desc: Null host / null props degrade to the built-in default without crashing.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableClipResolverTestNg, ClipResolverNullSafety001, TestSize.Level1)
{
    auto gridProps = AceType::MakeRefPtr<GridPaintProperty>();
    auto gridNode = CreateTagNode(V2::GRID_ETS_TAG, NextNodeId());
    ASSERT_NE(gridNode, nullptr);

    // Null host: there is no pipeline to query, so the built-in default survives
    // even with every strategy applied (conservative degradation).
    ApplyStrategies(ALL_SCROLL_STRATEGIES);
    EXPECT_EQ(
        ScrollableUtils::ResolveEffectiveDefaultContentClip(nullptr, AceType::RawPtr(gridProps)),
        ContentClipMode::BOUNDARY);

    // Null props: there is no built-in default source, so the resolver degrades to
    // CONTENT_ONLY and never promotes, whatever the strategy set holds.
    ClearStrategies();
    EXPECT_EQ(ScrollableUtils::ResolveEffectiveDefaultContentClip(gridNode, nullptr), ContentClipMode::CONTENT_ONLY);
    ApplyStrategies(ALL_SCROLL_STRATEGIES);
    EXPECT_EQ(ScrollableUtils::ResolveEffectiveDefaultContentClip(gridNode, nullptr), ContentClipMode::CONTENT_ONLY);
}

/**
 * @tc.name: ClipResolverDisabledKeepsBuiltInDefault001
 * @tc.desc: AC-3.1/R-4 - with no strategy applied every component keeps its built-in default.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableClipResolverTestNg, ClipResolverDisabledKeepsBuiltInDefault001, TestSize.Level1)
{
    auto baseProps = AceType::MakeRefPtr<ScrollablePaintProperty>();
    auto gridProps = AceType::MakeRefPtr<GridPaintProperty>();
    auto scrollProps = AceType::MakeRefPtr<ScrollPaintProperty>();

    auto gridNode = CreateTagNode(V2::GRID_ETS_TAG, NextNodeId());
    auto listNode = CreateTagNode(V2::LIST_ETS_TAG, NextNodeId());
    auto scrollNode = CreateTagNode(V2::SCROLL_ETS_TAG, NextNodeId());
    auto waterFlowNode = CreateTagNode(V2::WATERFLOW_ETS_TAG, NextNodeId());
    ASSERT_NE(gridNode, nullptr);
    ASSERT_NE(listNode, nullptr);
    ASSERT_NE(scrollNode, nullptr);
    ASSERT_NE(waterFlowNode, nullptr);

    EXPECT_EQ(
        ScrollableUtils::ResolveEffectiveDefaultContentClip(gridNode, AceType::RawPtr(gridProps)),
        ContentClipMode::BOUNDARY);
    EXPECT_EQ(
        ScrollableUtils::ResolveEffectiveDefaultContentClip(scrollNode, AceType::RawPtr(scrollProps)),
        ContentClipMode::BOUNDARY);
    EXPECT_EQ(
        ScrollableUtils::ResolveEffectiveDefaultContentClip(listNode, AceType::RawPtr(baseProps)),
            ContentClipMode::CONTENT_ONLY);
    EXPECT_EQ(ScrollableUtils::ResolveEffectiveDefaultContentClip(waterFlowNode, AceType::RawPtr(baseProps)),
        ContentClipMode::CONTENT_ONLY);
}

/**
 * @tc.name: ClipResolverEnabledPromotesToSafeArea001
 * @tc.desc: AC-1.1/R-1 - an enabled strategy promotes the effective default to SAFE_AREA.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableClipResolverTestNg, ClipResolverEnabledPromotesToSafeArea001, TestSize.Level1)
{
    ApplyStrategies(ALL_SCROLL_STRATEGIES);

    auto baseProps = AceType::MakeRefPtr<ScrollablePaintProperty>();
    auto gridProps = AceType::MakeRefPtr<GridPaintProperty>();
    auto scrollProps = AceType::MakeRefPtr<ScrollPaintProperty>();

    auto gridNode = CreateTagNode(V2::GRID_ETS_TAG, NextNodeId());
    auto listNode = CreateTagNode(V2::LIST_ETS_TAG, NextNodeId());
    auto scrollNode = CreateTagNode(V2::SCROLL_ETS_TAG, NextNodeId());
    auto waterFlowNode = CreateTagNode(V2::WATERFLOW_ETS_TAG, NextNodeId());

    EXPECT_EQ(
        ScrollableUtils::ResolveEffectiveDefaultContentClip(gridNode, AceType::RawPtr(gridProps)),
        ContentClipMode::SAFE_AREA);
    EXPECT_EQ(
        ScrollableUtils::ResolveEffectiveDefaultContentClip(listNode, AceType::RawPtr(baseProps)),
        ContentClipMode::SAFE_AREA);
    EXPECT_EQ(
        ScrollableUtils::ResolveEffectiveDefaultContentClip(scrollNode, AceType::RawPtr(scrollProps)),
            ContentClipMode::SAFE_AREA);
    EXPECT_EQ(
        ScrollableUtils::ResolveEffectiveDefaultContentClip(waterFlowNode, AceType::RawPtr(baseProps)),
            ContentClipMode::SAFE_AREA);
}

/**
 * @tc.name: ClipResolverPerComponentIsolation001
 * @tc.desc: AC-1.3/R-1 - a strategy only affects its own component type.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableClipResolverTestNg, ClipResolverPerComponentIsolation001, TestSize.Level1)
{
    ApplyStrategies({ ImmersiveStrategy::GRID_EXPAND_SAFE_AREA });

    auto baseProps = AceType::MakeRefPtr<ScrollablePaintProperty>();
    auto gridProps = AceType::MakeRefPtr<GridPaintProperty>();
    auto scrollProps = AceType::MakeRefPtr<ScrollPaintProperty>();

    auto gridNode = CreateTagNode(V2::GRID_ETS_TAG, NextNodeId());
    auto listNode = CreateTagNode(V2::LIST_ETS_TAG, NextNodeId());
    auto scrollNode = CreateTagNode(V2::SCROLL_ETS_TAG, NextNodeId());
    auto waterFlowNode = CreateTagNode(V2::WATERFLOW_ETS_TAG, NextNodeId());

    EXPECT_EQ(
        ScrollableUtils::ResolveEffectiveDefaultContentClip(gridNode, AceType::RawPtr(gridProps)),
        ContentClipMode::SAFE_AREA);
    EXPECT_EQ(
        ScrollableUtils::ResolveEffectiveDefaultContentClip(listNode, AceType::RawPtr(baseProps)),
            ContentClipMode::CONTENT_ONLY);
    EXPECT_EQ(
        ScrollableUtils::ResolveEffectiveDefaultContentClip(scrollNode, AceType::RawPtr(scrollProps)),
        ContentClipMode::BOUNDARY);
    EXPECT_EQ(ScrollableUtils::ResolveEffectiveDefaultContentClip(waterFlowNode, AceType::RawPtr(baseProps)),
        ContentClipMode::CONTENT_ONLY);
}

/**
 * @tc.name: ClipResolverReplaceSemantics001
 * @tc.desc: R-1 - the strategy set has replace semantics; clearing it restores the built-in default.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableClipResolverTestNg, ClipResolverReplaceSemantics001, TestSize.Level1)
{
    auto baseProps = AceType::MakeRefPtr<ScrollablePaintProperty>();
    auto listNode = CreateTagNode(V2::LIST_ETS_TAG, NextNodeId());
    ASSERT_NE(listNode, nullptr);

    ApplyStrategies({ ImmersiveStrategy::LIST_EXPAND_SAFE_AREA });
    EXPECT_EQ(
        ScrollableUtils::ResolveEffectiveDefaultContentClip(listNode, AceType::RawPtr(baseProps)),
        ContentClipMode::SAFE_AREA);

    // Replace with a set that no longer contains the List strategy.
    ApplyStrategies({ ImmersiveStrategy::GRID_EXPAND_SAFE_AREA });
    EXPECT_EQ(
        ScrollableUtils::ResolveEffectiveDefaultContentClip(listNode, AceType::RawPtr(baseProps)),
            ContentClipMode::CONTENT_ONLY);

    ApplyStrategies({});
    EXPECT_EQ(
        ScrollableUtils::ResolveEffectiveDefaultContentClip(listNode, AceType::RawPtr(baseProps)),
            ContentClipMode::CONTENT_ONLY);
}

/**
 * @tc.name: ClipResolverNonScrollableTagUnaffected001
 * @tc.desc: R-4 - non-scrollable tags never consume the scrollable strategies.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableClipResolverTestNg, ClipResolverNonScrollableTagUnaffected001, TestSize.Level1)
{
    ApplyStrategies(ALL_SCROLL_STRATEGIES);

    auto baseProps = AceType::MakeRefPtr<ScrollablePaintProperty>();
    auto columnNode = CreateTagNode(V2::COLUMN_ETS_TAG, NextNodeId());
    ASSERT_NE(columnNode, nullptr);
    EXPECT_EQ(
        ScrollableUtils::ResolveEffectiveDefaultContentClip(columnNode, AceType::RawPtr(baseProps)),
            ContentClipMode::CONTENT_ONLY);
}

/**
 * @tc.name: ClipResolverNoCachingAcrossQueries001
 * @tc.desc: INV-3 - the resolver is a pure query: repeated calls track the current strategy set.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableClipResolverTestNg, ClipResolverNoCachingAcrossQueries001, TestSize.Level1)
{
    auto gridProps = AceType::MakeRefPtr<GridPaintProperty>();
    auto gridNode = CreateTagNode(V2::GRID_ETS_TAG, NextNodeId());
    ASSERT_NE(gridNode, nullptr);

    for (int32_t round = 0; round < 3; ++round) {
        EXPECT_EQ(
            ScrollableUtils::ResolveEffectiveDefaultContentClip(gridNode, AceType::RawPtr(gridProps)),
            ContentClipMode::BOUNDARY);
        ApplyStrategies({ ImmersiveStrategy::GRID_EXPAND_SAFE_AREA });
        EXPECT_EQ(
            ScrollableUtils::ResolveEffectiveDefaultContentClip(gridNode, AceType::RawPtr(gridProps)),
            ContentClipMode::SAFE_AREA);
        EXPECT_EQ(
            ScrollableUtils::ResolveEffectiveDefaultContentClip(gridNode, AceType::RawPtr(gridProps)),
            ContentClipMode::SAFE_AREA);
        ClearStrategies();
    }
}
} // namespace OHOS::Ace::NG
