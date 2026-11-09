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

#include <limits>

#include "gtest/gtest.h"

#include "core/components_ng/manager/page_scene/split_scene_recognizer.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr float WINDOW_WIDTH = 1000.0f;
constexpr float WINDOW_HEIGHT = 1000.0f;
const RectF WINDOW_RECT(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT);

SplitSceneCandidate MakeCandidate(int32_t nodeId, const std::string& compName, const RectF& rect)
{
    SplitSceneCandidate candidate;
    candidate.nodeId = nodeId;
    candidate.compName = compName;
    candidate.rect = rect;
    return candidate;
}
} // namespace

class SplitSceneRecognizerTest : public testing::Test {};

/**
 * @tc.name: SplitSceneRecognizer_Navigation001
 * @tc.desc: A visible Navigation in its final SPLIT mode wins and exits before custom recognition.
 * @tc.type: FUNC
 */
HWTEST_F(SplitSceneRecognizerTest, SplitSceneRecognizer_Navigation001, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.compNames = { "Column" };
    auto navigation = MakeCandidate(1, "Navigation", WINDOW_RECT);
    navigation.navigation = true;
    navigation.navigationSplit = true;
    auto custom = MakeCandidate(2, "Column", RectF(0.0f, 0.0f, 500.0f, 900.0f));

    auto result = SplitSceneRecognizer::Recognize(rule, { navigation, custom }, WINDOW_RECT);

    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.splitType, "Navigation");
    EXPECT_EQ(result.visitedCandidates, 1u);
}

/**
 * @tc.name: SplitSceneRecognizer_Navigation002
 * @tc.desc: Hidden, covered, inactive or non-SPLIT Navigation does not match.
 * @tc.type: FUNC
 */
HWTEST_F(SplitSceneRecognizerTest, SplitSceneRecognizer_Navigation002, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    auto navigation = MakeCandidate(1, "Navigation", WINDOW_RECT);
    navigation.navigation = true;
    navigation.navigationSplit = true;
    navigation.navBarVisible = false;
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { navigation }, WINDOW_RECT).matched);

    navigation.navBarVisible = true;
    navigation.fullyCovered = true;
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { navigation }, WINDOW_RECT).matched);

    navigation.fullyCovered = false;
    navigation.navigationSplit = false;
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { navigation }, WINDOW_RECT).matched);

    navigation.navigationSplit = true;
    navigation.rect = RectF();
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { navigation }, WINDOW_RECT).matched);

    navigation.rect = RectF(WINDOW_WIDTH, 0.0f, 100.0f, WINDOW_HEIGHT);
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { navigation }, WINDOW_RECT).matched);

    navigation.navigationSplit = false;
    navigation.rect = WINDOW_RECT;
    rule.compNames = { "Pane" };
    auto left = MakeCandidate(2, "Pane", RectF(0.0f, 0.0f, 400.0f, 900.0f));
    auto right = MakeCandidate(3, "Pane", RectF(400.0f, 0.0f, 401.0f, 900.0f));
    auto customResult = SplitSceneRecognizer::Recognize(rule, { left, right, navigation }, WINDOW_RECT);
    EXPECT_TRUE(customResult.matched);
    EXPECT_EQ(customResult.splitType, "Custom");
    EXPECT_EQ(customResult.visitedCandidates, 3u);
}

/**
 * @tc.name: SplitSceneRecognizer_Custom001
 * @tc.desc: Gapped two-column and three-column layouts match with an explicitly relaxed per-candidate width cap.
 * @tc.type: FUNC
 */
HWTEST_F(SplitSceneRecognizerTest, SplitSceneRecognizer_Custom001, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.allowNavigation = false;
    rule.compNames = { "CustomPane" };
    rule.widthRatioLte = 1.0;
    auto wide = MakeCandidate(1, "CustomPane", RectF(0.0f, 0.0f, 600.0f, 900.0f));
    auto narrow = MakeCandidate(2, "CustomPane", RectF(620.0f, 0.0f, 300.0f, 900.0f));
    auto result = SplitSceneRecognizer::Recognize(rule, { wide, narrow }, WINDOW_RECT);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.splitType, "Custom");

    auto first = MakeCandidate(3, "CustomPane", RectF(0.0f, 0.0f, 280.0f, 900.0f));
    auto second = MakeCandidate(4, "CustomPane", RectF(300.0f, 0.0f, 280.0f, 900.0f));
    auto third = MakeCandidate(5, "CustomPane", RectF(600.0f, 0.0f, 280.0f, 900.0f));
    EXPECT_TRUE(SplitSceneRecognizer::Recognize(rule, { first, second, third }, WINDOW_RECT).matched);

    // A wider overlapping candidate must not hide a valid gapped pair from
    // the interval DP merely because the single candidate has more width.
    auto overlappingWide = MakeCandidate(6, "CustomPane", RectF(0.0f, 0.0f, 900.0f, 900.0f));
    auto pairLeft = MakeCandidate(7, "CustomPane", RectF(0.0f, 0.0f, 400.0f, 900.0f));
    auto pairRight = MakeCandidate(8, "CustomPane", RectF(500.0f, 0.0f, 410.0f, 900.0f));
    EXPECT_TRUE(SplitSceneRecognizer::Recognize(rule, { overlappingWide, pairLeft, pairRight }, WINDOW_RECT).matched);
}

/**
 * @tc.name: SplitSceneRecognizer_Custom002
 * @tc.desc: Strict height/width thresholds and positive-area overlap are rejected while edge contact is allowed.
 * @tc.type: FUNC
 */
HWTEST_F(SplitSceneRecognizerTest, SplitSceneRecognizer_Custom002, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.allowNavigation = false;
    rule.compNames = { "Pane" };
    auto left = MakeCandidate(1, "Pane", RectF(0.0f, 0.0f, 400.0f, 900.0f));
    auto right = MakeCandidate(2, "Pane", RectF(400.0f, 0.0f, 401.0f, 900.0f));
    EXPECT_TRUE(SplitSceneRecognizer::Recognize(rule, { left, right }, WINDOW_RECT).matched);

    right.rect = RectF(399.0f, 0.0f, 402.0f, 900.0f);
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { left, right }, WINDOW_RECT).matched);

    right.rect = RectF(400.0f, 0.0f, 400.0f, 900.0f);
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { left, right }, WINDOW_RECT).matched);

    right.rect = RectF(400.0f, 0.0f, 401.0f, 800.0f);
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { left, right }, WINDOW_RECT).matched);
}

/**
 * @tc.name: SplitSceneRecognizer_Custom003
 * @tc.desc: Exact component names and early rejection avoid unrelated geometry work.
 * @tc.type: FUNC
 */
HWTEST_F(SplitSceneRecognizerTest, SplitSceneRecognizer_Custom003, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.allowNavigation = false;
    rule.compNames = { "Pane" };
    auto unrelated = MakeCandidate(1, "pane", WINDOW_RECT);

    auto result = SplitSceneRecognizer::Recognize(rule, { unrelated }, WINDOW_RECT);

    EXPECT_FALSE(result.matched);
    EXPECT_EQ(result.visitedCandidates, 0u);
    EXPECT_NE(result.earlyExitStage, 0u);
}
// Snapshot-level scenario tests. Real device E2E uses verify_pagescene_split.py.

HWTEST_F(SplitSceneRecognizerTest, SplitSceneRecognizer_Scenario_NavigationSplit, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.widthRatioLte = 0.5;
    rule.heightRatioGt = 0.8;
    rule.totalWidthRatioGt = 0.8;
    rule.nonOverlapping = true;
    rule.compNames = { "Column" };
    RectF window(0.0f, 0.0f, 720.0f, 1280.0f);
    SplitSceneCandidate nav;
    nav.nodeId = 100;
    nav.compName = "Navigation";
    nav.rect = window;
    nav.navigation = true;
    nav.navigationSplit = true;
    nav.navBarVisible = true;
    auto result = SplitSceneRecognizer::Recognize(rule, { nav }, window);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.splitType, "Navigation");
}

HWTEST_F(SplitSceneRecognizerTest, SplitSceneRecognizer_Scenario_NavFallback, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.widthRatioLte = 0.5;
    rule.heightRatioGt = 0.8;
    rule.totalWidthRatioGt = 0.8;
    rule.nonOverlapping = true;
    rule.compNames = { "Column" };
    RectF window(0.0f, 0.0f, 720.0f, 1280.0f);
    SplitSceneCandidate nav;
    nav.nodeId = 100;
    nav.compName = "Navigation";
    nav.rect = window;
    nav.navigation = true;
    nav.navigationSplit = true;
    nav.navBarVisible = false;
    SplitSceneCandidate left;
    left.nodeId = 101;
    left.compName = "Column";
    left.rect = RectF(0.0f, 0.0f, 340.0f, 1200.0f);
    SplitSceneCandidate right;
    right.nodeId = 102;
    right.compName = "Column";
    right.rect = RectF(380.0f, 0.0f, 340.0f, 1200.0f);
    auto result = SplitSceneRecognizer::Recognize(rule, { nav, left, right }, window);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.splitType, "Custom");
}

HWTEST_F(SplitSceneRecognizerTest, SplitSceneRecognizer_Scenario_ThreeColumn, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.widthRatioLte = 0.5;
    rule.heightRatioGt = 0.8;
    rule.totalWidthRatioGt = 0.8;
    rule.nonOverlapping = true;
    rule.allowNavigation = false;
    rule.compNames = { "Column" };
    RectF window(0.0f, 0.0f, 1200.0f, 1000.0f);
    SplitSceneCandidate c1, c2, c3;
    c1.nodeId = 1;
    c1.compName = "Column";
    c1.rect = RectF(0.0f, 0.0f, 350.0f, 950.0f);
    c2.nodeId = 2;
    c2.compName = "Column";
    c2.rect = RectF(400.0f, 0.0f, 350.0f, 950.0f);
    c3.nodeId = 3;
    c3.compName = "Column";
    c3.rect = RectF(800.0f, 0.0f, 350.0f, 950.0f);
    auto result = SplitSceneRecognizer::Recognize(rule, { c1, c2, c3 }, window);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.splitType, "Custom");
}

HWTEST_F(SplitSceneRecognizerTest, SplitSceneRecognizer_Scenario_WideContainer, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.widthRatioLte = 0.5;
    rule.heightRatioGt = 0.8;
    rule.totalWidthRatioGt = 0.8;
    rule.nonOverlapping = true;
    rule.allowNavigation = false;
    rule.compNames = { "Column" };
    RectF window(0.0f, 0.0f, 1000.0f, 1000.0f);
    SplitSceneCandidate wide;
    wide.nodeId = 1;
    wide.compName = "Column";
    wide.rect = RectF(0.0f, 0.0f, 700.0f, 950.0f);
    SplitSceneCandidate narrow;
    narrow.nodeId = 2;
    narrow.compName = "Column";
    narrow.rect = RectF(720.0f, 0.0f, 280.0f, 950.0f);
    auto limitedResult = SplitSceneRecognizer::Recognize(rule, { wide, narrow }, window);
    EXPECT_FALSE(limitedResult.matched); // 70% 超过显式配置的 50% 上限
    rule.widthRatioLte = 1.0;
    auto result = SplitSceneRecognizer::Recognize(rule, { wide, narrow }, window);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.splitType, "Custom");
}

HWTEST_F(SplitSceneRecognizerTest, SplitSceneRecognizer_Scenario_OverlapRejected, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.widthRatioLte = 0.5;
    rule.heightRatioGt = 0.8;
    rule.totalWidthRatioGt = 0.8;
    rule.nonOverlapping = true;
    rule.allowNavigation = false;
    rule.compNames = { "Column" };
    RectF window(0.0f, 0.0f, 1000.0f, 1000.0f);
    SplitSceneCandidate left;
    left.nodeId = 1;
    left.compName = "Column";
    left.rect = RectF(0.0f, 0.0f, 500.0f, 950.0f);
    SplitSceneCandidate right;
    right.nodeId = 2;
    right.compName = "Column";
    right.rect = RectF(400.0f, 0.0f, 500.0f, 950.0f);
    auto result = SplitSceneRecognizer::Recognize(rule, { left, right }, window);
    EXPECT_FALSE(result.matched);
}

HWTEST_F(SplitSceneRecognizerTest, SplitSceneRecognizer_Scenario_HeightInsufficient, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.widthRatioLte = 0.5;
    rule.heightRatioGt = 0.8;
    rule.totalWidthRatioGt = 0.8;
    rule.nonOverlapping = true;
    rule.allowNavigation = false;
    rule.compNames = { "Column" };
    RectF window(0.0f, 0.0f, 1000.0f, 1000.0f);
    SplitSceneCandidate left;
    left.nodeId = 1;
    left.compName = "Column";
    left.rect = RectF(0.0f, 0.0f, 450.0f, 700.0f);
    SplitSceneCandidate right;
    right.nodeId = 2;
    right.compName = "Column";
    right.rect = RectF(500.0f, 0.0f, 450.0f, 700.0f);
    auto result = SplitSceneRecognizer::Recognize(rule, { left, right }, window);
    EXPECT_FALSE(result.matched);
}

// 新输入契约边界：上限为 <=，恰好 50% 允许参与，超过即排除。
HWTEST_F(SplitSceneRecognizerTest, SplitSceneRecognizer_Scenario_WidthRatioBoundary, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.allowNavigation = false;
    rule.widthRatioLte = 0.5;
    rule.heightRatioGt = 0.8;
    rule.totalWidthRatioGt = 0.8;
    rule.nonOverlapping = true;
    rule.compNames = { "Column" };
    RectF window(0.0f, 0.0f, 1000.0f, 1000.0f);
    SplitSceneCandidate left;
    left.nodeId = 1;
    left.compName = "Column";
    left.rect = RectF(0.0f, 0.0f, 500.0f, 950.0f);
    SplitSceneCandidate right;
    right.nodeId = 2;
    right.compName = "Column";
    right.rect = RectF(500.0f, 0.0f, 500.0f, 950.0f);
    EXPECT_TRUE(SplitSceneRecognizer::Recognize(rule, { left, right }, window).matched);
    left.rect = RectF(0.0f, 0.0f, 501.0f, 950.0f);
    right.rect = RectF(501.0f, 0.0f, 499.0f, 950.0f);
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { left, right }, window).matched);
}

// 非推荐高度/总宽阈值与重叠开关：同一组候选在配置变更后重新判断。
HWTEST_F(SplitSceneRecognizerTest, SplitSceneRecognizer_Scenario_ConfigurableGeometry, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.allowNavigation = false;
    rule.compNames = { "Column" };
    rule.heightRatioGt = 0.6;
    rule.widthRatioLte = 0.5;
    rule.totalWidthRatioGt = 0.7;
    rule.nonOverlapping = false;
    RectF window(0.0f, 0.0f, 1000.0f, 1000.0f);
    SplitSceneCandidate left;
    left.nodeId = 1;
    left.compName = "Column";
    left.rect = RectF(0.0f, 0.0f, 400.0f, 700.0f);
    SplitSceneCandidate right;
    right.nodeId = 2;
    right.compName = "Column";
    right.rect = RectF(300.0f, 0.0f, 400.0f, 700.0f);
    EXPECT_TRUE(SplitSceneRecognizer::Recognize(rule, { left, right }, window).matched);
    rule.nonOverlapping = true;
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { left, right }, window).matched);
    right.rect = RectF(500.0f, 0.0f, 400.0f, 700.0f);
    EXPECT_TRUE(SplitSceneRecognizer::Recognize(rule, { left, right }, window).matched);
    rule.heightRatioGt = 0.7; // 等于高度阈值，不满足严格大于。
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { left, right }, window).matched);
    rule.heightRatioGt = 0.6;
    rule.totalWidthRatioGt = 0.8; // 合计宽度等于阈值，不满足严格大于。
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { left, right }, window).matched);
}
// 上下分栏即使满足所有尺寸阈值，也不能命中左右分栏。
HWTEST_F(SplitSceneRecognizerTest, SplitSceneRecognizer_Scenario_VerticalSplitRejected, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.allowNavigation = false;
    rule.compNames = { "Column" };
    rule.heightRatioGt = 0.4;
    rule.widthRatioLte = 1.0;
    rule.totalWidthRatioGt = 0.8;
    RectF window(0.0f, 0.0f, 1000.0f, 1000.0f);
    SplitSceneCandidate top;
    top.nodeId = 1;
    top.compName = "Column";
    top.rect = RectF(0.0f, 0.0f, 1000.0f, 450.0f);
    SplitSceneCandidate bottom;
    bottom.nodeId = 2;
    bottom.compName = "Column";
    bottom.rect = RectF(0.0f, 550.0f, 1000.0f, 450.0f);
    for (bool nonOverlapping : { true, false }) {
        rule.nonOverlapping = nonOverlapping;
        EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { top, bottom }, window).matched);
    }
    // 允许重叠也不应使同一水平位置的上下容器成为左右分栏。
    rule.nonOverlapping = false;
    bottom.rect = RectF(0.0f, 300.0f, 1000.0f, 450.0f);
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { top, bottom }, window).matched);
    top.rect = RectF(0.0f, 0.0f, 450.0f, 450.0f);
    bottom.rect = RectF(550.0f, 0.0f, 450.0f, 450.0f);
    EXPECT_TRUE(SplitSceneRecognizer::Recognize(rule, { top, bottom }, window).matched);
}

HWTEST_F(SplitSceneRecognizerTest, SplitSceneRecognizer_ConfiguredDoubleBoundaries, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.allowNavigation = false;
    rule.compNames = { "Column" };
    rule.heightRatioGt = 0.8;
    rule.widthRatioLte = 0.5 - 1e-10;
    rule.totalWidthRatioGt = 0.8;
    auto left = MakeCandidate(1, "Column", RectF(0, 0, 500, 900));
    auto right = MakeCandidate(2, "Column", RectF(500, 0, 500, 900));
    // A legal double threshold must not be rounded up to permit a wider node.
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { left, right }, WINDOW_RECT).matched);
    rule.widthRatioLte = 0.5;
    rule.heightRatioGt = 0.9 - 1e-10;
    EXPECT_TRUE(SplitSceneRecognizer::Recognize(rule, { left, right }, WINDOW_RECT).matched);
    rule.heightRatioGt = 0.9;
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { left, right }, WINDOW_RECT).matched);
    rule.heightRatioGt = 0.8;
    rule.totalWidthRatioGt = 1.0 - 1e-10;
    EXPECT_TRUE(SplitSceneRecognizer::Recognize(rule, { left, right }, WINDOW_RECT).matched);
    rule.totalWidthRatioGt = 1.0;
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { left, right }, WINDOW_RECT).matched);
}

namespace {
constexpr int32_t PREFIX_NODE_ID = 99601;
constexpr int32_t WIDE_NODE_ID = 99602;
constexpr int32_t OVERLAP_NODE_ID = 99603;
constexpr float SHORT_BAND_HEIGHT = 100.0f;
constexpr float LOWER_BAND_TOP = 800.0f;
constexpr float FIRST_PANE_LEFT = 100.0f;
constexpr float SECOND_PANE_LEFT = 200.0f;
constexpr float HALF_WINDOW_WIDTH = WINDOW_WIDTH / 2.0f;
constexpr double SHORT_HEIGHT_RATIO = 0.05;
constexpr double UNLIMITED_WIDTH_RATIO = 1.0;
constexpr size_t PAIR_NODE_COUNT = 2;

SplitSceneRuleSpec ShortBandRule(bool nonOverlapping)
{
    SplitSceneRuleSpec rule;
    rule.allowNavigation = false;
    rule.compNames = { "Pane" };
    rule.heightRatioGt = SHORT_HEIGHT_RATIO;
    rule.widthRatioLte = UNLIMITED_WIDTH_RATIO;
    rule.nonOverlapping = nonOverlapping;
    return rule;
}
} // namespace

HWTEST_F(SplitSceneRecognizerTest, SplitScene_InactivePredecessorCannotFormPair, TestSize.Level1)
{
    auto prefix = MakeCandidate(PREFIX_NODE_ID, "Pane", RectF(0.0f, 0.0f, FIRST_PANE_LEFT, SHORT_BAND_HEIGHT));
    auto wide = MakeCandidate(WIDE_NODE_ID, "Pane",
        RectF(FIRST_PANE_LEFT, LOWER_BAND_TOP, WINDOW_WIDTH - FIRST_PANE_LEFT, SHORT_BAND_HEIGHT));
    auto overlap = MakeCandidate(OVERLAP_NODE_ID, "Pane",
        RectF(SECOND_PANE_LEFT, LOWER_BAND_TOP, WINDOW_WIDTH - SECOND_PANE_LEFT, SHORT_BAND_HEIGHT));
    auto result = SplitSceneRecognizer::Recognize(ShortBandRule(true), { prefix, wide, overlap }, WINDOW_RECT);
    EXPECT_FALSE(result.matched);
    EXPECT_TRUE(result.matchedNodeIds.empty());
}

HWTEST_F(SplitSceneRecognizerTest, SplitScene_OverlappingSweepRejectsVerticalEdgeContact, TestSize.Level1)
{
    auto first = MakeCandidate(PREFIX_NODE_ID, "Pane", RectF(0.0f, 0.0f, HALF_WINDOW_WIDTH, SHORT_BAND_HEIGHT));
    auto second = MakeCandidate(
        WIDE_NODE_ID, "Pane", RectF(HALF_WINDOW_WIDTH, SHORT_BAND_HEIGHT, HALF_WINDOW_WIDTH, SHORT_BAND_HEIGHT));
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(ShortBandRule(false), { first, second }, WINDOW_RECT).matched);
}

HWTEST_F(SplitSceneRecognizerTest, SplitScene_OverlappingSweepFindsLaterBand, TestSize.Level1)
{
    auto first = MakeCandidate(PREFIX_NODE_ID, "Pane", RectF(0.0f, 0.0f, HALF_WINDOW_WIDTH, SHORT_BAND_HEIGHT));
    auto left = MakeCandidate(WIDE_NODE_ID, "Pane", RectF(0.0f, LOWER_BAND_TOP, HALF_WINDOW_WIDTH, SHORT_BAND_HEIGHT));
    auto right = MakeCandidate(
        OVERLAP_NODE_ID, "Pane", RectF(HALF_WINDOW_WIDTH, LOWER_BAND_TOP, HALF_WINDOW_WIDTH, SHORT_BAND_HEIGHT));
    auto result = SplitSceneRecognizer::Recognize(ShortBandRule(false), { first, left, right }, WINDOW_RECT);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.matchedNodeIds.size(), PAIR_NODE_COUNT);
    EXPECT_EQ(result.matchedNodeIds, (std::vector<int32_t> { WIDE_NODE_ID, OVERLAP_NODE_ID }));
}

HWTEST_F(SplitSceneRecognizerTest, SplitScene_CommonBandHasDeterministicPair, TestSize.Level1)
{
    auto left = MakeCandidate(PREFIX_NODE_ID, "Pane", RectF(0.0f, 0.0f, HALF_WINDOW_WIDTH, WINDOW_HEIGHT));
    auto right = MakeCandidate(WIDE_NODE_ID, "Pane", RectF(HALF_WINDOW_WIDTH, 0.0f, HALF_WINDOW_WIDTH, WINDOW_HEIGHT));
    auto wide = MakeCandidate(OVERLAP_NODE_ID, "Pane", WINDOW_RECT);
    auto forward = SplitSceneRecognizer::Recognize(ShortBandRule(true), { wide, left, right }, WINDOW_RECT);
    auto reverse = SplitSceneRecognizer::Recognize(ShortBandRule(true), { right, left, wide }, WINDOW_RECT);
    EXPECT_TRUE(forward.matched);
    EXPECT_EQ(forward.matchedNodeIds.size(), PAIR_NODE_COUNT);
    EXPECT_EQ(forward.matchedNodeIds, reverse.matchedNodeIds);
}

HWTEST_F(SplitSceneRecognizerTest, SplitScene_BatchNavigationAvoidsCustomPreparation, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.compNames = { "Pane" };
    auto nav = MakeCandidate(1, "Navigation", WINDOW_RECT);
    nav.navigation = true;
    nav.navigationSplit = true;
    std::vector<SplitSceneCandidate> candidates { nav, MakeCandidate(2, "Pane", RectF(0, 0, 450, 900)),
        MakeCandidate(3, "Pane", RectF(500, 0, 450, 900)) };
    SplitSceneBatch batch(candidates, WINDOW_RECT);
    EXPECT_EQ(batch.Recognize(rule).splitType, "Navigation");
    EXPECT_EQ(batch.GetStats().rectClips, 1u);
    EXPECT_EQ(batch.GetStats().geometrySorts, 0u);
    rule.allowNavigation = false;
    EXPECT_EQ(batch.Recognize(rule).splitType, "Custom");
    EXPECT_EQ(batch.GetStats().rectClips, 3u);
}

HWTEST_F(SplitSceneRecognizerTest, SplitScene_BatchReusesClippingOrderingAndEquivalentRules, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.allowNavigation = false;
    rule.compNames = { "Column", "Row" };
    std::vector<SplitSceneCandidate> candidates { MakeCandidate(1, "Column", RectF(-50, 0, 500, 900)),
        MakeCandidate(2, "Row", RectF(550, 0, 500, 900)) };
    SplitSceneBatch batch(candidates, WINDOW_RECT);
    auto first = batch.Recognize(rule);
    ASSERT_TRUE(first.matched);
    rule.totalWidthRatioGt = 0.95;
    EXPECT_FALSE(batch.Recognize(rule).matched);
    rule.totalWidthRatioGt = 0.85;
    EXPECT_TRUE(batch.Recognize(rule).matched);
    rule.totalWidthRatioGt = 0.8;
    rule.compNames = { "Row", "Column" };
    EXPECT_EQ(batch.Recognize(rule).matchedNodeIds, first.matchedNodeIds);
    EXPECT_EQ(batch.GetStats().rectClips, 2u);
    EXPECT_EQ(batch.GetStats().geometrySorts, 1u);
    EXPECT_EQ(batch.GetStats().recognitions, 3u);
    EXPECT_EQ(batch.GetStats().cacheHits, 1u);
}

HWTEST_F(SplitSceneRecognizerTest, SplitScene_BatchKeepsGeometryPoliciesIndependent, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.allowNavigation = false;
    rule.compNames = { "Pane" };
    rule.heightRatioGt = 0.6;
    rule.nonOverlapping = false;
    std::vector<SplitSceneCandidate> candidates { MakeCandidate(1, "Pane", RectF(0, 0, 450, 700)),
        MakeCandidate(2, "Pane", RectF(350, 0, 450, 700)) };
    SplitSceneBatch batch(candidates, WINDOW_RECT);
    EXPECT_TRUE(batch.Recognize(rule).matched);
    rule.nonOverlapping = true;
    EXPECT_FALSE(batch.Recognize(rule).matched);
    rule.nonOverlapping = false;
    rule.heightRatioGt = 0.7;
    EXPECT_FALSE(batch.Recognize(rule).matched);
    rule.heightRatioGt = 0.6;
    rule.widthRatioLte = 0.4;
    EXPECT_FALSE(batch.Recognize(rule).matched);
    rule.widthRatioLte = 0.5;
    EXPECT_TRUE(batch.Recognize(rule).matched);
    EXPECT_EQ(batch.GetStats().rectClips, 2u);
    EXPECT_EQ(batch.GetStats().cacheHits, 1u);
}

HWTEST_F(SplitSceneRecognizerTest, SplitScene_BatchReusesBandEventsWithoutChangingNodeOrder, TestSize.Level1)
{
    auto rule = ShortBandRule(false);
    std::vector<SplitSceneCandidate> candidates { MakeCandidate(1, "Pane", RectF(0, 0, 500, 100)),
        MakeCandidate(3, "Pane", RectF(500, 800, 500, 100)), MakeCandidate(2, "Pane", RectF(0, 800, 500, 100)) };
    SplitSceneBatch batch(candidates, WINDOW_RECT);
    EXPECT_EQ(batch.Recognize(rule).matchedNodeIds, (std::vector<int32_t> { 3, 2 }));
    rule.totalWidthRatioGt = 0.9;
    EXPECT_EQ(batch.Recognize(rule).matchedNodeIds, (std::vector<int32_t> { 3, 2 }));
    EXPECT_EQ(batch.GetStats().geometrySorts, 1u);
    EXPECT_EQ(batch.GetStats().rectClips, 3u);
}

HWTEST_F(SplitSceneRecognizerTest, SplitScene_BatchDoesNotReuseChangedSnapshot, TestSize.Level1)
{
    SplitSceneRuleSpec rule;
    rule.allowNavigation = false;
    rule.compNames = { "Pane" };
    std::vector<SplitSceneCandidate> candidates { MakeCandidate(1, "Pane", RectF(0, 0, 450, 900)),
        MakeCandidate(2, "Pane", RectF(500, 0, 450, 900)) };
    {
        SplitSceneBatch batch(candidates, WINDOW_RECT);
        EXPECT_TRUE(batch.Recognize(rule).matched);
    }
    candidates[1].rect = RectF(100, 0, 450, 900);
    SplitSceneBatch batch(candidates, WINDOW_RECT);
    EXPECT_FALSE(batch.Recognize(rule).matched);
    EXPECT_EQ(batch.GetStats().rectClips, 2u);
    EXPECT_EQ(batch.GetStats().cacheHits, 0u);
}

HWTEST_F(SplitSceneRecognizerTest, SplitScene_BatchDistinguishesUniqueNodesSelectedByDifferentRules, TestSize.Level1)
{
    constexpr int32_t COLUMN_NODE_ID = 1;
    constexpr int32_t ROW_NODE_ID = 2;
    constexpr int32_t PANE_NODE_ID = 3;
    constexpr float WIDE_PANE_WIDTH = 450.0f;
    constexpr float NARROW_PANE_WIDTH = 100.0f;
    constexpr float PANE_HEIGHT = 900.0f;
    SplitSceneRuleSpec rule;
    rule.allowNavigation = false;
    rule.compNames = { "Column", "Pane" };
    std::vector<SplitSceneCandidate> candidates {
        MakeCandidate(COLUMN_NODE_ID, "Column", RectF(0, 0, WIDE_PANE_WIDTH, PANE_HEIGHT)),
        MakeCandidate(ROW_NODE_ID, "Row", RectF(0, 0, NARROW_PANE_WIDTH, PANE_HEIGHT)),
        MakeCandidate(PANE_NODE_ID, "Pane", RectF(HALF_WINDOW_WIDTH, 0, WIDE_PANE_WIDTH, PANE_HEIGHT)) };
    SplitSceneBatch batch(candidates, WINDOW_RECT);
    const auto first = batch.Recognize(rule);
    ASSERT_TRUE(first.matched);
    EXPECT_EQ(first.matchedNodeIds, (std::vector<int32_t> { COLUMN_NODE_ID, PANE_NODE_ID }));
    rule.compNames = { "Row", "Pane" };
    EXPECT_FALSE(batch.Recognize(rule).matched);
    rule.compNames = { "Column", "Pane" };
    EXPECT_EQ(batch.Recognize(rule).matchedNodeIds, first.matchedNodeIds);
    EXPECT_EQ(batch.GetStats().cacheHits, 1u);
}


namespace {
constexpr float OFFSET_WINDOW_LEFT = 100.0f;
constexpr float OFFSET_WINDOW_TOP = 200.0f;
constexpr float WINDOW_OVERSCAN = 100.0f;
constexpr float VISIBLE_PANE_HEIGHT = 900.0f;
constexpr double CLIPPED_HEIGHT_RATIO_LIMIT = 0.8;
constexpr double CLIPPED_WIDTH_RATIO_LIMIT = 0.5;
const RectF OFFSET_WINDOW(OFFSET_WINDOW_LEFT, OFFSET_WINDOW_TOP, WINDOW_WIDTH, WINDOW_HEIGHT);
} // namespace

HWTEST_F(SplitSceneRecognizerTest, SplitScene_ShiftedWindowClipsBeforeWidthLimit, TestSize.Level1)
{
    auto rule = ShortBandRule(true);
    rule.heightRatioGt = CLIPPED_HEIGHT_RATIO_LIMIT;
    rule.widthRatioLte = CLIPPED_WIDTH_RATIO_LIMIT;
    auto left = MakeCandidate(PREFIX_NODE_ID, "Pane",
        RectF(OFFSET_WINDOW.Left() - WINDOW_OVERSCAN, OFFSET_WINDOW.Top(),
            HALF_WINDOW_WIDTH + WINDOW_OVERSCAN, VISIBLE_PANE_HEIGHT));
    auto right = MakeCandidate(WIDE_NODE_ID, "Pane",
        RectF(OFFSET_WINDOW.Left() + HALF_WINDOW_WIDTH, OFFSET_WINDOW.Top(),
            HALF_WINDOW_WIDTH + WINDOW_OVERSCAN, VISIBLE_PANE_HEIGHT));
    const auto result = SplitSceneRecognizer::Recognize(rule, { left, right }, OFFSET_WINDOW);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.matchedNodeIds, (std::vector<int32_t> { PREFIX_NODE_ID, WIDE_NODE_ID }));
    const auto clipped = SplitSceneRecognizer::ClipToWindow(left.rect, OFFSET_WINDOW);
    EXPECT_FLOAT_EQ(clipped.Left(), OFFSET_WINDOW.Left());
    EXPECT_FLOAT_EQ(clipped.Width(), HALF_WINDOW_WIDTH);
}

HWTEST_F(SplitSceneRecognizerTest, SplitScene_ShiftedWindowUsesClippedHeightAtStrictBoundary, TestSize.Level1)
{
    auto rule = ShortBandRule(true);
    rule.heightRatioGt = CLIPPED_HEIGHT_RATIO_LIMIT;
    auto left = MakeCandidate(PREFIX_NODE_ID, "Pane",
        RectF(OFFSET_WINDOW.Left(), OFFSET_WINDOW.Top() - WINDOW_OVERSCAN,
            HALF_WINDOW_WIDTH, VISIBLE_PANE_HEIGHT));
    auto right = MakeCandidate(WIDE_NODE_ID, "Pane",
        RectF(OFFSET_WINDOW.Left() + HALF_WINDOW_WIDTH, OFFSET_WINDOW.Top(),
            HALF_WINDOW_WIDTH, VISIBLE_PANE_HEIGHT));
    EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { left, right }, OFFSET_WINDOW).matched);
    left.rect = RectF(OFFSET_WINDOW.Left(), OFFSET_WINDOW.Top(), HALF_WINDOW_WIDTH, VISIBLE_PANE_HEIGHT);
    EXPECT_TRUE(SplitSceneRecognizer::Recognize(rule, { left, right }, OFFSET_WINDOW).matched);
}

HWTEST_F(SplitSceneRecognizerTest, SplitScene_NonFiniteGeometryCannotContributeToMatch, TestSize.Level1)
{
    const auto rule = ShortBandRule(true);
    auto left = MakeCandidate(PREFIX_NODE_ID, "Pane", RectF(0, 0, HALF_WINDOW_WIDTH, VISIBLE_PANE_HEIGHT));
    auto right = MakeCandidate(WIDE_NODE_ID, "Pane",
        RectF(HALF_WINDOW_WIDTH, 0, HALF_WINDOW_WIDTH, VISIBLE_PANE_HEIGHT));
    for (float invalid : { std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity() }) {
        auto malformed = MakeCandidate(OVERLAP_NODE_ID, "Pane",
            RectF(invalid, 0, HALF_WINDOW_WIDTH, VISIBLE_PANE_HEIGHT));
        EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { left, malformed }, WINDOW_RECT).matched);
        auto valid = SplitSceneRecognizer::Recognize(rule, { malformed, left, right }, WINDOW_RECT);
        EXPECT_TRUE(valid.matched);
        EXPECT_EQ(valid.matchedNodeIds, (std::vector<int32_t> { PREFIX_NODE_ID, WIDE_NODE_ID }));
        const RectF invalidWindow(invalid, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        EXPECT_FALSE(SplitSceneRecognizer::Recognize(rule, { left, right }, invalidWindow).matched);
    }
}

} // namespace OHOS::Ace::NG
