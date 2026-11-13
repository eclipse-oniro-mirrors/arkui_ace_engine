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

// TDD coverage for the landscape safe-area branches added to
// DialogLayoutAlgorithm::SetAlignmentSwitch (popup/dialog/toast unified
// avoidance fix). Verifies that left/right safe-area insets shrink the
// horizontal layout span and shift the dialog into the safe region when the
// window is in landscape, while preserving prior behavior when insets are zero.

#include "gtest/gtest.h"
#include "ui/base/geometry/ng/size_t.h"

#define private public
#define protected public
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/pattern/dialog/dialog_layout_algorithm.h"
#include "core/components_ng/pattern/dialog/dialog_layout_property.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
// Mock pipeline default root width/height (matches DISPLAY_WIDTH/HEIGHT).
constexpr double ROOT_WIDTH = 720.0;
constexpr double ROOT_HEIGHT = 1280.0;
// Simulated landscape safe-area insets (px): status bar / nav bar on the sides.
constexpr uint32_t SAFE_LEFT_END = 50;
constexpr uint32_t SAFE_RIGHT_END = 60;
constexpr float CHILD_WIDTH = 200.0f;
constexpr float CHILD_HEIGHT = 200.0f;
constexpr float MAX_HEIGHT = 1000.0f;
} // namespace

class DialogLayoutSafeAreaTestNg : public testing::Test {
public:
    static void SetUpTestCase()
    {
        MockPipelineContext::SetUp();
        MockContainer::SetUp();
        MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
        MockContainer::Current()->pipelineContext_ = MockPipelineContext::GetCurrentContext();
        MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
        // Pin root size so GetRootWidth()/GetRootHeight() are deterministic for
        // the rightInsetNotPreExcluded branch (NearEqual(maxSize.Width(), rootWidth)).
        MockPipelineContext::GetCurrent()->SetRootSize(ROOT_WIDTH, ROOT_HEIGHT);
    }

    static void TearDownTestCase()
    {
        MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }

    // Build an algorithm whose GetPipelineContext() resolves to the mock
    // pipeline (context_ is null on default construct, so inject the mock).
    // RefPtr<MockPipelineContext> -> WeakPtr<PipelineContext> is a safe
    // derived->base conversion (verified via RefPtr::ConvertPointer).
    static void CreateAlgorithm(DialogLayoutAlgorithm& algo)
    {
        auto pipeline = MockPipelineContext::GetCurrent();
        EXPECT_NE(pipeline, nullptr);
        algo.context_ = pipeline;
        algo.customSize_ = true; // suppress CaculateMaxSize shrinking of maxSize
    }

    static void SetLandscapeSafeArea(DialogLayoutAlgorithm& algo)
    {
        algo.safeAreaInsets_.left_ = SafeAreaInsets::Inset { 0, SAFE_LEFT_END };
        algo.safeAreaInsets_.right_ = SafeAreaInsets::Inset { 0, SAFE_RIGHT_END };
    }
};

/**
 * @tc.name: SetAlignmentSwitchSafeArea001
 * @tc.desc: TOP alignment with left+right safe-area insets and full-width
 *           layout (rightInsetNotPreExcluded == true): the dialog X must be
 *           computed from the doubly-shrunk width and shifted by safeAreaLeft.
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutSafeAreaTestNg, SetAlignmentSwitchSafeArea001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    CreateAlgorithm(algo);
    ASSERT_NE(MockPipelineContext::GetCurrent(), nullptr);
    algo.alignment_ = DialogAlignment::TOP;
    SetLandscapeSafeArea(algo);

    // maxSize.Width() == rootWidth (720) => rightInsetNotPreExcluded = true.
    // shrunkWidth = max(0, 720 - 50) = 670, then -60 => 610.
    SizeF maxSize(static_cast<float>(ROOT_WIDTH), MAX_HEIGHT);
    SizeF childSize(CHILD_WIDTH, CHILD_HEIGHT);
    OffsetF topLeftPoint;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    // TOP: x = (shrunkWidth - childW)/HALF + safeAreaLeft = (610-200)/2 + 50 = 255.
    EXPECT_FLOAT_EQ(topLeftPoint.GetX(), 255.0f);
    EXPECT_FLOAT_EQ(topLeftPoint.GetY(), 0.0f);
}

/**
 * @tc.name: SetAlignmentSwitchSafeArea002
 * @tc.desc: CENTER alignment with full-width layout (rightInsetNotPreExcluded
 *           == true): X derived from doubly-shrunk width + safeAreaLeft, Y from
 *           full height.
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutSafeAreaTestNg, SetAlignmentSwitchSafeArea002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    CreateAlgorithm(algo);
    algo.alignment_ = DialogAlignment::CENTER;
    SetLandscapeSafeArea(algo);

    SizeF maxSize(static_cast<float>(ROOT_WIDTH), MAX_HEIGHT);
    SizeF childSize(CHILD_WIDTH, CHILD_HEIGHT);
    OffsetF topLeftPoint;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    // CENTER: (shrunkWidth - childW, height - childH)/HALF then +safeAreaLeft on X.
    // x = (610-200)/2 + 50 = 255 ; y = (1000-200)/2 = 400.
    EXPECT_FLOAT_EQ(topLeftPoint.GetX(), 255.0f);
    EXPECT_FLOAT_EQ(topLeftPoint.GetY(), 400.0f);
}

/**
 * @tc.name: SetAlignmentSwitchSafeArea003
 * @tc.desc: TOP_END alignment where maxSize.Width() != rootWidth
 *           (rightInsetNotPreExcluded == false): the right inset must NOT be
 *           subtracted again (it was already excluded upstream). X collapses to
 *           the screen right edge, confirming the false-branch path.
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutSafeAreaTestNg, SetAlignmentSwitchSafeArea003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    CreateAlgorithm(algo);
    algo.alignment_ = DialogAlignment::TOP_END;
    SetLandscapeSafeArea(algo);

    // width 600 != rootWidth 720 => rightInsetNotPreExcluded = false.
    // shrunkWidth = max(0, 600 - 50) = 550 (right NOT subtracted).
    SizeF maxSize(600.0f, MAX_HEIGHT);
    SizeF childSize(CHILD_WIDTH, CHILD_HEIGHT);
    OffsetF topLeftPoint;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    // TOP_END: x = (shrunkWidth - childW) + safeAreaLeft = (550-200) + 50 = 400.
    // Had the right inset been wrongly subtracted, shrunkWidth would be 490 and
    // x would be 340 instead of 400, so this guards the false branch.
    EXPECT_FLOAT_EQ(topLeftPoint.GetX(), 400.0f);
    EXPECT_FLOAT_EQ(topLeftPoint.GetY(), 0.0f);
}

/**
 * @tc.name: SetAlignmentSwitchSafeArea004
 * @tc.desc: TOP_START alignment: topLeftPoint starts at (0,0) and the new
 *           SetX(+safeAreaLeft) shift must move it to the left safe edge,
 *           independently of shrunkWidth.
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutSafeAreaTestNg, SetAlignmentSwitchSafeArea004, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    CreateAlgorithm(algo);
    algo.alignment_ = DialogAlignment::TOP_START;
    SetLandscapeSafeArea(algo);

    SizeF maxSize(static_cast<float>(ROOT_WIDTH), MAX_HEIGHT);
    SizeF childSize(CHILD_WIDTH, CHILD_HEIGHT);
    OffsetF topLeftPoint;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    // TOP_START: (0,0) then SetX(0 + safeAreaLeft) => x = 50.
    EXPECT_FLOAT_EQ(topLeftPoint.GetX(), static_cast<float>(SAFE_LEFT_END));
    EXPECT_FLOAT_EQ(topLeftPoint.GetY(), 0.0f);
}

/**
 * @tc.name: SetAlignmentSwitchSafeArea005
 * @tc.desc: Regression guard: with zero safe-area insets and a non-full-width
 *           layout, behavior must match the pre-fix result (existing
 *           SetAlignmentSwitch001 expectation), i.e. no shift and no shrink.
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutSafeAreaTestNg, SetAlignmentSwitchSafeArea005, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    CreateAlgorithm(algo);
    algo.alignment_ = DialogAlignment::TOP;
    // safeAreaInsets_ left at default (all-zero Inset).

    SizeF maxSize(600.0f, MAX_HEIGHT);
    SizeF childSize(CHILD_WIDTH, CHILD_HEIGHT);
    OffsetF topLeftPoint;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    // No insets: shrunkWidth = 600, safeAreaLeft = 0 => x = (600-200)/2 = 200.
    EXPECT_FLOAT_EQ(topLeftPoint.GetX(), 200.0f);
    EXPECT_FLOAT_EQ(topLeftPoint.GetY(), 0.0f);
}
} // namespace OHOS::Ace::NG
