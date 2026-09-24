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

} // namespace OHOS::Ace::NG
