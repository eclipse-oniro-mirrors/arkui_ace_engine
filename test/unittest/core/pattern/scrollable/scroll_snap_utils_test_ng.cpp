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

#include <gtest/gtest.h>

#include <vector>

#include "core/components_ng/pattern/scrollable/scroll_snap_utils.h"
#include "core/components_ng/pattern/scrollable/scroll_snap_strategy.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
std::vector<SnapCandidate> MakeCandidates(const std::vector<float>& offsets)
{
    std::vector<SnapCandidate> candidates;
    int32_t index = 0;
    for (auto offset : offsets) {
        candidates.push_back({ offset, index++ });
    }
    return candidates;
}
} // namespace

class ScrollSnapUtilsTest : public testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: NormalizeCandidates001
 * @tc.desc: Feat-20260806915268 R-6: identical offsets are deduplicated and equal offsets keep the smaller index.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollSnapUtilsTest, NormalizeCandidates001, TestSize.Level1)
{
    auto candidates = MakeCandidates({ 200.0f, 0.0f, 100.0f, 100.0f });
    ScrollSnapUtils::NormalizeCandidates(candidates);
    ASSERT_EQ(candidates.size(), 3u);
    EXPECT_FLOAT_EQ(candidates[0].offset, 0.0f);
    EXPECT_FLOAT_EQ(candidates[1].offset, 100.0f);
    EXPECT_FLOAT_EQ(candidates[2].offset, 200.0f);
    // The duplicate 100.0f keeps the smaller index (2 was dropped, 2 == index of first 100).
    EXPECT_EQ(candidates[1].index, 2);
}

/**
 * @tc.name: SelectByThreshold001
 * @tc.desc: Feat-20260806915268 R-4 / AC-1.3: effective distance below D*T returns the current candidate.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollSnapUtilsTest, SelectByThreshold001, TestSize.Level1)
{
    // Candidates every 100; the user was snapped at 100 and dragged forward to 140
    // (40% of the 100px gap, below the default threshold 0.5).
    auto candidates = MakeCandidates({ 0.0f, 100.0f, 200.0f, 300.0f });
    auto result = ScrollSnapUtils::SelectByThreshold(candidates, 140.0f, 0.5f, true);
    ASSERT_TRUE(result.found);
    EXPECT_FLOAT_EQ(result.targetOffset, 100.0f);
}

/**
 * @tc.name: SelectByThreshold002
 * @tc.desc: Feat-20260806915268 R-4 / AC-1.3: effective distance reaching D*T enters the adjacent candidate.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollSnapUtilsTest, SelectByThreshold002, TestSize.Level1)
{
    auto candidates = MakeCandidates({ 0.0f, 100.0f, 200.0f, 300.0f });
    // Dragged from 100 to 150: exactly D*T (100 * 0.5) enters the adjacent candidate.
    auto result = ScrollSnapUtils::SelectByThreshold(candidates, 150.0f, 0.5f, true);
    ASSERT_TRUE(result.found);
    EXPECT_FLOAT_EQ(result.targetOffset, 200.0f);
}

/**
 * @tc.name: SelectByThreshold003
 * @tc.desc: Feat-20260806915268 AC-1.3: T=0 always advances, T=1 always stays, backward direction works.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollSnapUtilsTest, SelectByThreshold003, TestSize.Level1)
{
    auto candidates = MakeCandidates({ 0.0f, 100.0f, 200.0f, 300.0f });
    // T = 0: even a tiny movement enters the adjacent candidate.
    auto zeroT = ScrollSnapUtils::SelectByThreshold(candidates, 100.5f, 0.0f, true);
    ASSERT_TRUE(zeroT.found);
    EXPECT_FLOAT_EQ(zeroT.targetOffset, 200.0f);
    // T = 1: only a full distance enters the adjacent candidate.
    auto oneT = ScrollSnapUtils::SelectByThreshold(candidates, 199.0f, 1.0f, true);
    ASSERT_TRUE(oneT.found);
    EXPECT_FLOAT_EQ(oneT.targetOffset, 100.0f);
    auto oneTFull = ScrollSnapUtils::SelectByThreshold(candidates, 200.0f, 1.0f, true);
    ASSERT_TRUE(oneTFull.found);
    EXPECT_FLOAT_EQ(oneTFull.targetOffset, 200.0f);
    // Backward: pulled 60% from candidate 200 towards 100 with T=0.5 -> enters 100.
    auto backward = ScrollSnapUtils::SelectByThreshold(candidates, 140.0f, 0.5f, false);
    ASSERT_TRUE(backward.found);
    EXPECT_FLOAT_EQ(backward.targetOffset, 100.0f);
}

/**
 * @tc.name: SelectByThreshold004
 * @tc.desc: Feat-20260806915268 R-11: thresholds outside [0,1] and NaN fall back to a determined behavior.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollSnapUtilsTest, SelectByThreshold004, TestSize.Level1)
{
    auto candidates = MakeCandidates({ 0.0f, 100.0f, 200.0f });
    // All sub-cases release at 150 (dragged from 100 halfway to 200).
    // NaN falls back to the default 0.5: effective 50 >= 100*0.5 advances.
    auto nanT = ScrollSnapUtils::SelectByThreshold(candidates, 150.0f, std::nanf(""), true);
    ASSERT_TRUE(nanT.found);
    EXPECT_FLOAT_EQ(nanT.targetOffset, 200.0f);
    // Out-of-range values are clamped: T=1 requires the full distance, so it stays.
    auto bigT = ScrollSnapUtils::SelectByThreshold(candidates, 150.0f, 5.0f, true);
    EXPECT_FLOAT_EQ(bigT.targetOffset, 100.0f);
    auto negT = ScrollSnapUtils::SelectByThreshold(candidates, 150.0f, -5.0f, true);
    EXPECT_FLOAT_EQ(negT.targetOffset, 200.0f);
}

/**
 * @tc.name: SelectByPredictedLanding001
 * @tc.desc: Feat-20260806915268 R-5 / AC-1.4: nearest candidate to the predicted landing wins and never
 *           moves against the fling direction.
 */
HWTEST_F(ScrollSnapUtilsTest, SelectByPredictedLanding001, TestSize.Level1)
{
    auto candidates = MakeCandidates({ 0.0f, 100.0f, 200.0f, 300.0f, 400.0f });
    // Landing at 265 while flinging forward picks 300 (not the closer-behind 200).
    auto forward = ScrollSnapUtils::SelectByPredictedLanding(candidates, 200.0f, 265.0f, true, 0.5f);
    ASSERT_TRUE(forward.found);
    EXPECT_FLOAT_EQ(forward.targetOffset, 300.0f);
    // Landing at 235 while flinging backward picks 200.
    auto backward = ScrollSnapUtils::SelectByPredictedLanding(candidates, 200.0f, 235.0f, false, 0.5f);
    ASSERT_TRUE(backward.found);
    EXPECT_FLOAT_EQ(backward.targetOffset, 200.0f);
    // Forward fling with no candidate ahead keeps a threshold result instead of reversing.
    auto edge = ScrollSnapUtils::SelectByPredictedLanding(candidates, 390.0f, 600.0f, true, 0.5f);
    ASSERT_TRUE(edge.found);
    EXPECT_FLOAT_EQ(edge.targetOffset, 400.0f);
}

/**
 * @tc.name: NormalizeCandidates002
 * @tc.desc: Feat-20260806915268 review: near-equal offsets within SNAP_OFFSET_EPSILON merge deterministically
 *           and the comparator keeps a strict weak ordering.
 */
HWTEST_F(ScrollSnapUtilsTest, NormalizeCandidates002, TestSize.Level1)
{
    // Dedup is adjacent-based: 0.3 merges into 0.0 (|0.3| <= 0.5) while 0.6 stays (|0.6| > 0.5
    // against the kept 0.0). The pin is the deterministic result of the strict-weak-order sort.
    auto candidates = MakeCandidates({ 0.6f, 0.3f, 0.0f, 100.0f });
    ScrollSnapUtils::NormalizeCandidates(candidates);
    ASSERT_EQ(candidates.size(), 3u);
    EXPECT_FLOAT_EQ(candidates[0].offset, 0.0f);
    EXPECT_FLOAT_EQ(candidates[1].offset, 0.6f);
    EXPECT_FLOAT_EQ(candidates[2].offset, 100.0f);
}

/**
 * @tc.name: SelectByPredictedLanding002
 * @tc.desc: Feat-20260806915268 review: boundaries — currentOffset exactly on a candidate and beyond the
 *           first/last candidate stay deterministic in both directions.
 */
HWTEST_F(ScrollSnapUtilsTest, SelectByPredictedLanding002, TestSize.Level1)
{
    auto candidates = MakeCandidates({ 0.0f, 100.0f, 200.0f, 300.0f, 400.0f });
    // Exactly on candidate 200, flinging forward: the current candidate itself stays selectable.
    auto onCandidateFwd = ScrollSnapUtils::SelectByPredictedLanding(candidates, 200.0f, 265.0f, true, 0.5f);
    ASSERT_TRUE(onCandidateFwd.found);
    EXPECT_FLOAT_EQ(onCandidateFwd.targetOffset, 300.0f);
    // Exactly on candidate 200, flinging backward: nearest to 165 among [0..200] is 200.
    auto onCandidateBwd = ScrollSnapUtils::SelectByPredictedLanding(candidates, 200.0f, 165.0f, false, 0.5f);
    ASSERT_TRUE(onCandidateBwd.found);
    EXPECT_FLOAT_EQ(onCandidateBwd.targetOffset, 200.0f);
    // Beyond the last candidate: no candidate in the fling direction falls back to threshold.
    auto beyondLast = ScrollSnapUtils::SelectByPredictedLanding(candidates, 450.0f, 600.0f, true, 0.5f);
    ASSERT_TRUE(beyondLast.found);
    EXPECT_FLOAT_EQ(beyondLast.targetOffset, 400.0f);
    // Before the first candidate backward: stays on the first candidate.
    auto beforeFirst = ScrollSnapUtils::SelectByPredictedLanding(candidates, -50.0f, -120.0f, false, 0.5f);
    ASSERT_TRUE(beforeFirst.found);
    EXPECT_FLOAT_EQ(beforeFirst.targetOffset, 0.0f);
}

/**
 * @tc.name: SelectByThresholdBoundary001
 * @tc.desc: Feat-20260806915268 review: releasing exactly on a candidate keeps it in both directions.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollSnapUtilsTest, SelectByThresholdBoundary001, TestSize.Level1)
{
    auto candidates = MakeCandidates({ 0.0f, 100.0f, 200.0f, 300.0f });
    auto onSpot = ScrollSnapUtils::SelectByThreshold(candidates, 200.0f, 0.5f, true);
    ASSERT_TRUE(onSpot.found);
    EXPECT_FLOAT_EQ(onSpot.targetOffset, 200.0f);
    auto onSpotBackward = ScrollSnapUtils::SelectByThreshold(candidates, 200.0f, 0.5f, false);
    ASSERT_TRUE(onSpotBackward.found);
    EXPECT_FLOAT_EQ(onSpotBackward.targetOffset, 200.0f);
}

/**
 * @tc.name: SelectByDirection001
 * @tc.desc: Feat-20260806915268 axis step: next candidate strictly beyond the current offset.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollSnapUtilsTest, SelectByDirection001, TestSize.Level1)
{
    auto candidates = MakeCandidates({ 0.0f, 100.0f, 200.0f, 300.0f });
    auto forward = ScrollSnapUtils::SelectByDirection(candidates, 150.0f, true);
    ASSERT_TRUE(forward.found);
    EXPECT_FLOAT_EQ(forward.targetOffset, 200.0f);
    auto backward = ScrollSnapUtils::SelectByDirection(candidates, 150.0f, false);
    ASSERT_TRUE(backward.found);
    EXPECT_FLOAT_EQ(backward.targetOffset, 100.0f);
    // At the end: stays on the last candidate instead of reversing.
    auto end = ScrollSnapUtils::SelectByDirection(candidates, 300.0f, true);
    ASSERT_TRUE(end.found);
    EXPECT_FLOAT_EQ(end.targetOffset, 300.0f);
}

/**
 * @tc.name: SelectByDirectionAnchoredBackward001
 * @tc.desc: Feat-20260806915268 review: stepping backwards while resting exactly on an anchor must move to
 *           the previous candidate instead of stalling on the current one.
 */
HWTEST_F(ScrollSnapUtilsTest, SelectByDirectionAnchoredBackward001, TestSize.Level1)
{
    auto candidates = MakeCandidates({ 0.0f, 100.0f, 200.0f });
    // Exactly on 100, stepping backward: previous candidate 0 wins.
    auto backwardOnAnchor = ScrollSnapUtils::SelectByDirection(candidates, 100.0f, false);
    ASSERT_TRUE(backwardOnAnchor.found);
    EXPECT_FLOAT_EQ(backwardOnAnchor.targetOffset, 0.0f);
    // Mid-gap still resolves to the anchor behind.
    auto backwardMidGap = ScrollSnapUtils::SelectByDirection(candidates, 150.0f, false);
    ASSERT_TRUE(backwardMidGap.found);
    EXPECT_FLOAT_EQ(backwardMidGap.targetOffset, 100.0f);
    // Forward from an anchor unchanged: next candidate.
    auto forwardOnAnchor = ScrollSnapUtils::SelectByDirection(candidates, 100.0f, true);
    ASSERT_TRUE(forwardOnAnchor.found);
    EXPECT_FLOAT_EQ(forwardOnAnchor.targetOffset, 200.0f);
}

/**
 * @tc.name: ScrollSnapStrategyEquality001
 * @tc.desc: Feat-20260806915268 review: provider-bearing strategies never compare equal so the property
 *           update path always stores fresh callbacks; align-only strategies dedupe by align.
 */
HWTEST_F(ScrollSnapUtilsTest, ScrollSnapStrategyEquality001, TestSize.Level1)
{
    ScrollSnapStrategy alignA;
    alignA.align = ScrollSnapAlign::CENTER;
    ScrollSnapStrategy alignB;
    alignB.align = ScrollSnapAlign::CENTER;
    EXPECT_TRUE(alignA == alignB); // align-only: dedupe

    ScrollSnapStrategy providerA;
    providerA.hasProvider = true;
    providerA.calculateSnapOffset = [](double) { return 100.0; };
    ScrollSnapStrategy providerB;
    providerB.hasProvider = true;
    providerB.calculateSnapOffset = [](double) { return 200.0; };
    EXPECT_FALSE(providerA == providerB); // provider: never equal, force re-store
    EXPECT_FALSE(providerA == alignA); // mixed: force re-store
}

/**
 * @tc.name: SelectSnapTarget001
 * @tc.desc: Feat-20260806915268 AC-1.3/AC-1.4: low speed uses the threshold, high speed uses the predicted
 *           landing derived from the fling decay delta.
 */
HWTEST_F(ScrollSnapUtilsTest, SelectSnapTarget001, TestSize.Level1)
{
    auto candidates = MakeCandidates({ 0.0f, 100.0f, 200.0f, 300.0f, 400.0f, 500.0f });
    // Low speed: threshold path (40 < 100 * 0.5 -> stay).
    auto low = ScrollSnapUtils::SelectSnapTarget(candidates, 140.0f, 100.0f, 10.0f, 0.5f, 1.0f);
    ASSERT_TRUE(low.found);
    EXPECT_FLOAT_EQ(low.targetOffset, 100.0f);
    // High speed forward: predicted landing = 140 + 180 = 320 -> 300.
    auto high = ScrollSnapUtils::SelectSnapTarget(candidates, 140.0f, 1500.0f, 180.0f, 0.5f, 1.0f);
    ASSERT_TRUE(high.found);
    EXPECT_FLOAT_EQ(high.targetOffset, 300.0f);
    // Zero velocity and zero delta: nearest-candidate threshold with forward default.
    auto still = ScrollSnapUtils::SelectSnapTarget(candidates, 260.0f, 0.0f, 0.0f, 0.5f, 1.0f);
    ASSERT_TRUE(still.found);
    EXPECT_FLOAT_EQ(still.targetOffset, 300.0f);
}

/**
 * @tc.name: ClampSnapOffset001
 * @tc.desc: Feat-20260806915268 R-3/R-7 / AC-2.1: targets are clamped into the legal scroll range.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollSnapUtilsTest, ClampSnapOffset001, TestSize.Level1)
{
    EXPECT_FLOAT_EQ(ScrollSnapUtils::ClampSnapOffset(-50.0f, 0.0f, 800.0f), 0.0f);
    EXPECT_FLOAT_EQ(ScrollSnapUtils::ClampSnapOffset(900.0f, 0.0f, 800.0f), 800.0f);
    EXPECT_FLOAT_EQ(ScrollSnapUtils::ClampSnapOffset(123.5f, 0.0f, 800.0f), 123.5f);
    // Inverted range yields the lower bound instead of an invalid position.
    EXPECT_FLOAT_EQ(ScrollSnapUtils::ClampSnapOffset(500.0f, 800.0f, 0.0f), 800.0f);
}

/**
 * @tc.name: EmptyCandidates001
 * @tc.desc: Feat-20260806915268: empty candidate set reports "not found" for every strategy.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollSnapUtilsTest, EmptyCandidates001, TestSize.Level1)
{
    std::vector<SnapCandidate> empty;
    EXPECT_FALSE(ScrollSnapUtils::SelectByThreshold(empty, 100.0f, 0.5f, true).found);
    EXPECT_FALSE(ScrollSnapUtils::SelectByPredictedLanding(empty, 100.0f, 400.0f, true, 0.5f).found);
    EXPECT_FALSE(ScrollSnapUtils::SelectByDirection(empty, 100.0f, true).found);
    EXPECT_FALSE(ScrollSnapUtils::SelectSnapTarget(empty, 100.0f, 1500.0f, 300.0f, 0.5f, 1.0f).found);
}

/**
 * @tc.name: ScrollSnapStrategyProperty001
 * @tc.desc: Feat-20260806915268: shared strategy struct default state disables snapping.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollSnapUtilsTest, ScrollSnapStrategyProperty001, TestSize.Level1)
{
    ScrollSnapStrategy strategy;
    EXPECT_FALSE(strategy.IsEnabled());
    EXPECT_EQ(strategy.align, ScrollSnapAlign::NONE);
    EXPECT_FALSE(strategy.hasProvider);

    strategy.align = ScrollSnapAlign::CENTER;
    EXPECT_TRUE(strategy.IsEnabled());

    ScrollSnapStrategy providerOnly;
    providerOnly.hasProvider = true;
    EXPECT_TRUE(providerOnly.IsEnabled());
    EXPECT_EQ(providerOnly.align, ScrollSnapAlign::NONE);

    // Equality only covers the declarative part.
    ScrollSnapStrategy other;
    other.align = ScrollSnapAlign::CENTER;
    EXPECT_TRUE(strategy == other);
    other.hasProvider = true;
    EXPECT_TRUE(strategy != other);
}

/**
 * @tc.name: SelectByPredictedLandingEquidistantTieBreak018
 * @tc.desc: Feat-20260806915268 R-6: predicted landing exactly between two candidates resolves the
 *           equidistant tie toward the fling direction (forward takes the farther candidate,
 *           backward the nearer one).
 * @tc.type: FUNC
 */
HWTEST_F(ScrollSnapUtilsTest, SelectByPredictedLandingEquidistantTieBreak018, TestSize.Level1)
{
    std::vector<SnapCandidate> candidates = MakeCandidates({ 100.0f, 200.0f, 300.0f });
    ScrollSnapUtils::NormalizeCandidates(candidates);
    // Landing exactly at 150: equidistant to 100 and 200.
    auto forward = ScrollSnapUtils::SelectByPredictedLanding(candidates, 50.0f, 150.0f, true, 0.5f);
    EXPECT_TRUE(forward.found);
    EXPECT_FLOAT_EQ(forward.targetOffset, 200.0f);
    auto backward = ScrollSnapUtils::SelectByPredictedLanding(candidates, 250.0f, 150.0f, false, 0.5f);
    EXPECT_TRUE(backward.found);
    EXPECT_FLOAT_EQ(backward.targetOffset, 100.0f);
}

/**
 * @tc.name: SelectSnapTargetBranchMatrix019
 * @tc.desc: Feat-20260806915268 R-3/R-5: the combined entry picks the branch by |velocity| against
 *           SNAP_VELOCITY_THRESHOLD * scale — negative velocity keeps the forward-fling
 *           semantics in the backward frame, a non-unity scale shifts the branch boundary,
 *           and |velocity| exactly at 780 enters the predicted-landing branch.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollSnapUtilsTest, SelectSnapTargetBranchMatrix019, TestSize.Level1)
{
    std::vector<SnapCandidate> candidates = MakeCandidates({ 0.0f, 100.0f, 200.0f, 300.0f, 400.0f,
        500.0f, 600.0f, 700.0f, 800.0f });
    ScrollSnapUtils::NormalizeCandidates(candidates);

    // Negative velocity: fling prediction runs in the backward frame — landing 80 behind the
    // current 200 (at 120) snaps to the nearest candidate behind it (100), never forward.
    auto backward = ScrollSnapUtils::SelectSnapTarget(candidates, 200.0f, -1500.0f, -80.0f, 0.5f, 1.0f);
    EXPECT_TRUE(backward.found);
    EXPECT_FLOAT_EQ(backward.targetOffset, 100.0f);

    // Scale 2.0 doubles the threshold: released at 260 with 1000 px/s stays below 780*2 and
    // keeps the threshold branch (past the half-gap of the 200 candidate -> one step to 300).
    auto slow = ScrollSnapUtils::SelectSnapTarget(candidates, 260.0f, 1000.0f, 500.0f, 0.5f, 2.0f);
    EXPECT_TRUE(slow.found);
    EXPECT_FLOAT_EQ(slow.targetOffset, 300.0f);
    // The same release crosses the single-scale threshold and follows the landing at 760 -> 800.
    auto fast = ScrollSnapUtils::SelectSnapTarget(candidates, 260.0f, 1000.0f, 500.0f, 0.5f, 1.0f);
    EXPECT_TRUE(fast.found);
    EXPECT_FLOAT_EQ(fast.targetOffset, 800.0f);

    // Boundary: |velocity| exactly 780 with scale 1.0 enters the predicted-landing branch
    // (200 + 210 = 410 -> nearest candidate 400); at 779.9f it stays on the threshold branch.
    auto edge = ScrollSnapUtils::SelectSnapTarget(candidates, 200.0f, 780.0f, 210.0f, 0.5f, 1.0f);
    EXPECT_TRUE(edge.found);
    EXPECT_FLOAT_EQ(edge.targetOffset, 400.0f);
    auto belowEdge = ScrollSnapUtils::SelectSnapTarget(candidates, 200.0f, 779.9f, 210.0f, 0.5f, 1.0f);
    EXPECT_TRUE(belowEdge.found);
    EXPECT_FLOAT_EQ(belowEdge.targetOffset, 200.0f);
}

/**
 * @tc.name: SelectSnapTargetInvalidVelocityInputs020
 * @tc.desc: Feat-20260806915268 review #5: non-finite velocity / non-positive or non-finite scale degrade
 *           deterministically to the zero-velocity behavior instead of a random branch.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollSnapUtilsTest, SelectSnapTargetInvalidVelocityInputs020, TestSize.Level1)
{
    std::vector<SnapCandidate> candidates = MakeCandidates({ 0.0f, 100.0f, 200.0f });
    ScrollSnapUtils::NormalizeCandidates(candidates);

    // NaN velocity: direction falls back to predictDelta; the comparison never selects the
    // fling branch, so the threshold rule decides from the current candidate (0).
    auto nanVelocity = ScrollSnapUtils::SelectSnapTarget(
        candidates, 0.0f, std::numeric_limits<float>::quiet_NaN(), 30.0f, 0.5f, 1.0f);
    EXPECT_TRUE(nanVelocity.found);
    EXPECT_FLOAT_EQ(nanVelocity.targetOffset, 0.0f);

    // Negative scale would make the speed threshold negative (always fling); the guard
    // restores the default 1.0 scale, so 100 px/s stays on the threshold branch.
    auto negativeScale = ScrollSnapUtils::SelectSnapTarget(candidates, 0.0f, 100.0f, 30.0f, 0.5f, -2.0f);
    EXPECT_TRUE(negativeScale.found);
    EXPECT_FLOAT_EQ(negativeScale.targetOffset, 0.0f);
}
} // namespace OHOS::Ace::NG
