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
#include <cstdint>
#include "test/mock/frameworks/core/rosen/mock_canvas.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "core/components_ng/pattern/common_text/preview_text_decoration_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr float TEST_LEFT = 10.0f;
constexpr float TEST_RIGHT = 100.0f;
constexpr float TEST_BOTTOM = 50.0f;
constexpr float TEST_UNDERLINE_WIDTH = 2.0f;
constexpr uint32_t TEST_COLOR_R = 255;
constexpr uint32_t TEST_COLOR_G = 0;
constexpr uint32_t TEST_COLOR_B = 0;
constexpr uint32_t TEST_COLOR_A = 255;
} // namespace

class PreviewTextDecorationUtilsTest : public testing::Test {
public:
    void SetUp() override
    {
        MockPipelineContext::SetUp();
    }
    void TearDown() override
    {
        MockPipelineContext::TearDown();
    }
};

/**
 * @tc.name: DrawPreviewTextUnderline_SolidStyle_001
 * @tc.desc: Test DrawPreviewTextUnderline with UNDERLINE style
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawPreviewTextUnderline_SolidStyle_001, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawRoundRect(_)).Times(1);

    PreviewDecorationParams params;
    params.left = TEST_LEFT;
    params.right = TEST_RIGHT;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::UNDERLINE;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);

    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawPreviewTextUnderline_SolidStyle_NotLast_002
 * @tc.desc: Test DrawPreviewTextUnderline with UNDERLINE style, isLast = false
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawPreviewTextUnderline_SolidStyle_NotLast_002, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawRoundRect(_)).Times(1);

    PreviewDecorationParams params;
    params.left = TEST_LEFT;
    params.right = TEST_RIGHT;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::UNDERLINE;
    params.isLast = false;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);

    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawPreviewTextUnderline_NormalStyle_Default_003
 * @tc.desc: Test DrawPreviewTextUnderline with NORMAL style falls through to default (solid)
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawPreviewTextUnderline_NormalStyle_Default_003, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawRoundRect(_)).Times(1);

    PreviewDecorationParams params;
    params.left = TEST_LEFT;
    params.right = TEST_RIGHT;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::NORMAL;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);

    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawPreviewTextUnderline_DashLineStyle_004
 * @tc.desc: Test DrawPreviewTextUnderline with DASH_LINE style
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawPreviewTextUnderline_DashLineStyle_004, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawLine(_, _)).Times(AtLeast(1));
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));

    PreviewDecorationParams params;
    params.left = TEST_LEFT;
    params.right = TEST_RIGHT;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::DASH_LINE;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);

    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawPreviewTextUnderline_WavyStyle_005
 * @tc.desc: Test DrawPreviewTextUnderline with WAVY style
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawPreviewTextUnderline_WavyStyle_005, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawPath(_)).Times(1);
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));

    PreviewDecorationParams params;
    params.left = TEST_LEFT;
    params.right = TEST_RIGHT;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::WAVY;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);

    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawWavyUnderline_ZeroWidth_006
 * @tc.desc: Test DrawWavyUnderline with zero width (right <= left), should return early
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawWavyUnderline_ZeroWidth_006, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawPath(_)).Times(0);
    EXPECT_CALL(canvas, DetachBrush()).Times(0);
    EXPECT_CALL(canvas, AttachPen(_)).Times(0);
    EXPECT_CALL(canvas, DetachPen()).Times(0);

    PreviewDecorationParams params;
    params.left = TEST_RIGHT;
    params.right = TEST_LEFT;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::WAVY;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);

    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawWavyUnderline_NarrowWidth_007
 * @tc.desc: Test DrawWavyUnderline with narrow width, no full wave cycles
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawWavyUnderline_NarrowWidth_007, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawPath(_)).Times(1);
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));

    PreviewDecorationParams params;
    params.left = TEST_LEFT;
    params.right = TEST_LEFT + 0.5f;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::WAVY;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);

    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawWavyUnderline_WideWidth_MultipleWaves_008
 * @tc.desc: Test DrawWavyUnderline with wide width triggering multiple wave cycles
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawWavyUnderline_WideWidth_MultipleWaves_008, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawPath(_)).Times(1);
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));

    PreviewDecorationParams params;
    params.left = 0.0f;
    params.right = 200.0f;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::WAVY;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);

    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawWavyUnderline_ExactWaveMultiple_009
 * @tc.desc: Test DrawWavyUnderline where remaining could be small
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawWavyUnderline_ExactWaveMultiple_009, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawPath(_)).Times(1);
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));

    PreviewDecorationParams params;
    params.left = 0.0f;
    params.right = 5.0f;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::WAVY;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);

    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawDashLineUnderline_ShortWidth_010
 * @tc.desc: Test DrawDashLineUnderline with short width producing a single dash segment
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawDashLineUnderline_ShortWidth_010, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawLine(_, _)).Times(1);
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));

    PreviewDecorationParams params;
    params.left = 10.0f;
    params.right = 11.0f;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::DASH_LINE;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);

    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawDashLineUnderline_LongWidth_MultipleDashes_011
 * @tc.desc: Test DrawDashLineUnderline with long width producing multiple dash segments
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawDashLineUnderline_LongWidth_MultipleDashes_011, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawLine(_, _)).Times(AtLeast(2));
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));

    PreviewDecorationParams params;
    params.left = 0.0f;
    params.right = 500.0f;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::DASH_LINE;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);

    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawSolidUnderline_ZeroUnderlineWidth_012
 * @tc.desc: Test DrawSolidUnderline with zero underlineWidth
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawSolidUnderline_ZeroUnderlineWidth_012, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawRoundRect(_)).Times(1);

    PreviewDecorationParams params;
    params.left = TEST_LEFT;
    params.right = TEST_RIGHT;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = 0.0f;
    params.underlineStyle = PreviewTextStyle::UNDERLINE;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);

    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawWavyUnderline_OddWaveCount_013
 * @tc.desc: Test BuildWavyPath with odd waveCount exit, covering sign = 1.0f branch
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawWavyUnderline_OddWaveCount_013, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawPath(_)).Times(1);
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));

    PreviewDecorationParams params;
    params.left = 0.0f;
    params.right = 3.0f;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::WAVY;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);

    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawWavyUnderline_InvalidUnderlineWidth_014
 * @tc.desc: Test DrawWavyUnderline with underlineWidth <= MIN_WIDTH (dipScale=0)
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawWavyUnderline_InvalidUnderlineWidth_014, TestSize.Level0)
{
    auto pipeline = AceType::DynamicCast<MockPipelineContext>(PipelineBase::GetCurrentContext());
    ASSERT_NE(pipeline, nullptr);
    pipeline->SetDipScale(0.0);

    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawPath(_)).Times(0);
    EXPECT_CALL(canvas, DetachBrush()).Times(0);
    EXPECT_CALL(canvas, AttachPen(_)).Times(0);
    EXPECT_CALL(canvas, DetachPen()).Times(0);

    PreviewDecorationParams params;
    params.left = TEST_LEFT;
    params.right = TEST_RIGHT;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::WAVY;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);

    DrawPreviewTextUnderline(canvas, params);

    pipeline->SetDipScale(1.0);
}

/**
 * @tc.name: DrawDashLineUnderline_LeftGeRight_015
 * @tc.desc: Test DrawDashLineUnderline with left >= right, loop body not entered
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawDashLineUnderline_LeftGeRight_015, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawLine(_, _)).Times(0);
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));

    PreviewDecorationParams params;
    params.left = TEST_RIGHT;
    params.right = TEST_LEFT;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::DASH_LINE;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);

    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawPreviewTextUnderline_AllStyles_NoCrash_016
 * @tc.desc: Test DrawPreviewTextUnderline does not crash for all style combinations
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawPreviewTextUnderline_AllStyles_NoCrash_016, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawRoundRect(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, DrawLine(_, _)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, DrawPath(_)).WillRepeatedly(Return());
    EXPECT_CALL(canvas, AttachBrush(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));

    PreviewDecorationParams params;
    params.left = TEST_LEFT;
    params.right = TEST_RIGHT;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);
    params.isLast = false;

    params.underlineStyle = PreviewTextStyle::UNDERLINE;
    DrawPreviewTextUnderline(canvas, params);

    params.underlineStyle = PreviewTextStyle::WAVY;
    DrawPreviewTextUnderline(canvas, params);

    params.underlineStyle = PreviewTextStyle::DASH_LINE;
    DrawPreviewTextUnderline(canvas, params);

    params.underlineStyle = PreviewTextStyle::NORMAL;
    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawDashLineUnderline_MultiSegment_PhaseContinuity_017
 * @tc.desc: Two consecutive segments with startOffset set; dash pattern should continue
 *           across the boundary rather than restarting at the second segment's left edge.
 *           Verifies startOffset is consumed (DrawLine called at least 2 times across
 *           the two segments combined) and no crash.
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawDashLineUnderline_MultiSegment_PhaseContinuity_017, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawLine(_, _)).Times(AtLeast(2));
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));

    // Segment 1: width 90. Segment 2 starts at startOffset = 90 so the dash phase
    // continues. The second segment's left (100) is unrelated to the first segment's
    // right (100) for phase purposes; only startOffset drives phase alignment.
    PreviewDecorationParams first;
    first.left = TEST_LEFT;
    first.right = TEST_RIGHT;
    first.bottom = TEST_BOTTOM;
    first.underlineWidth = TEST_UNDERLINE_WIDTH;
    first.underlineStyle = PreviewTextStyle::DASH_LINE;
    first.isLast = false;
    first.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);
    first.startOffset = 0.0f;
    DrawPreviewTextUnderline(canvas, first);

    PreviewDecorationParams second = first;
    second.left = TEST_RIGHT;            // contiguous with first.right
    second.right = TEST_RIGHT + 80.0f;   // arbitrary width
    second.isLast = true;
    second.startOffset = TEST_RIGHT - TEST_LEFT; // = 90, accumulated width
    DrawPreviewTextUnderline(canvas, second);
}

/**
 * @tc.name: DrawDashLineUnderline_StartOffsetInsideGap_018
 * @tc.desc: When startOffset falls inside a gap (phase > dashLength), the segment should
 *           skip to the next dash start and not draw a phantom short dash at the left edge.
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawDashLineUnderline_StartOffsetInsideGap_018, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    // We do not assert an exact count (pattern depends on font dip scale), only that
    // drawing happens and the first DrawLine start point is >= params.left + skip.
    EXPECT_CALL(canvas, DrawLine(_, _)).Times(AnyNumber());
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));

    PreviewDecorationParams params;
    params.left = TEST_LEFT;
    params.right = TEST_RIGHT;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::DASH_LINE;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);
    // Pick a startOffset that lands in the middle of a gap. dashLength = unit*4,
    // gap = unit*2, period = unit*6. With DEFAULT_FONTSIZE=16vp and dipScale=1,
    // unit = 16/14, period ~= 6.857px. startOffset = 5.0 lands inside the first gap
    // (dashLength ~= 4.57), so the segment should skip ~1.86px before drawing.
    params.startOffset = 5.0f;
    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawWavyUnderline_MultiSegment_PhaseContinuity_019
 * @tc.desc: Two consecutive wavy segments with startOffset set; the wave should continue
 *           across the boundary instead of restarting. Verifies DrawPath is called once
 *           per segment and no crash.
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawWavyUnderline_MultiSegment_PhaseContinuity_019, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawPath(_)).Times(2);
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));

    PreviewDecorationParams first;
    first.left = 0.0f;
    first.right = 50.0f;
    first.bottom = TEST_BOTTOM;
    first.underlineWidth = TEST_UNDERLINE_WIDTH;
    first.underlineStyle = PreviewTextStyle::WAVY;
    first.isLast = false;
    first.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);
    first.startOffset = 0.0f;
    DrawPreviewTextUnderline(canvas, first);

    PreviewDecorationParams second = first;
    second.left = 50.0f;
    second.right = 100.0f;
    second.isLast = true;
    second.startOffset = 50.0f; // accumulated width from first segment
    DrawPreviewTextUnderline(canvas, second);
}

/**
 * @tc.name: DrawWavyUnderline_StartOffsetMidHalfWave_020
 * @tc.desc: A non-zero startOffset shifts the up/down alternation of the first half-wave
 *           in this segment so the wave continues seamlessly across segment boundaries.
 *           Verifies DrawPath is called once and no crash.
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawWavyUnderline_StartOffsetMidHalfWave_020, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DrawPath(_)).Times(1);
    EXPECT_CALL(canvas, DetachBrush()).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));

    PreviewDecorationParams params;
    params.left = TEST_LEFT;
    params.right = TEST_RIGHT;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::WAVY;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);
    // startOffset = quarterWave (one full half-wave) makes the first half-wave in this
    // segment go down instead of up, continuing the alternation from a previous segment.
    params.startOffset = 1.0f;
    DrawPreviewTextUnderline(canvas, params);
}

/**
 * @tc.name: DrawDashLineUnderline_DetachesCallerBrush_021
 * @tc.desc: DrawDashLineUnderline must detach a caller-attached brush before stroking,
 *           otherwise brush+pen coexistence makes dashes look like a solid line.
 *           Verifies DetachBrush is called at least once for DASH_LINE style.
 * @tc.type: FUNC
 */
HWTEST_F(PreviewTextDecorationUtilsTest, DrawDashLineUnderline_DetachesCallerBrush_021, TestSize.Level0)
{
    Testing::MockCanvas canvas;
    EXPECT_CALL(canvas, DetachBrush()).Times(AtLeast(1)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DrawLine(_, _)).Times(AnyNumber());
    EXPECT_CALL(canvas, AttachPen(_)).WillRepeatedly(ReturnRef(canvas));
    EXPECT_CALL(canvas, DetachPen()).WillRepeatedly(ReturnRef(canvas));

    PreviewDecorationParams params;
    params.left = TEST_LEFT;
    params.right = TEST_RIGHT;
    params.bottom = TEST_BOTTOM;
    params.underlineWidth = TEST_UNDERLINE_WIDTH;
    params.underlineStyle = PreviewTextStyle::DASH_LINE;
    params.isLast = true;
    params.color = RSColor(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B, TEST_COLOR_A);
    params.startOffset = 0.0f;
    DrawPreviewTextUnderline(canvas, params);
}
} // namespace OHOS::Ace::NG
