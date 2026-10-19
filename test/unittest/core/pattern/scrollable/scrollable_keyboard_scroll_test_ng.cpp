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
#include <memory>

#include "test/unittest/core/pattern/test_ng.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/event/event_constants.h"
#include "core/components_ng/event/scrollable_event.h"
#include "core/components_ng/manager/focus/focus_manager.h"
#include "core/components_ng/pattern/list/list_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/scrollable/keyboard_scroll_settings_utils.h"
#include "core/components_ng/pattern/scrollable/scrollable.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/event/key_event.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
using namespace testing;
using namespace testing::ext;

namespace {
constexpr float DELTA_EPSILON = 0.001f;
// Default mouse-scroll rows from SettingDataManager (same source as the wheel setting).
constexpr int32_t DEFAULT_MOUSE_SCROLL_ROWS = 3;
int32_t g_keyboardScrollNodeId = 500;
} // namespace

class ScrollableKeyboardScrollTestNg : public TestNG {
public:
    void SetUp() override
    {
        TestNG::SetUp();
        // Bypass SettingDataManager IPC in unit tests: pretend the observer is already
        // registered and the cached rows value is the default (3).
        KeyboardScrollSettingsUtils::observerRegistered_ = true;
        KeyboardScrollSettingsUtils::rowsValid_ = true;
        KeyboardScrollSettingsUtils::cachedRows_ = DEFAULT_MOUSE_SCROLL_ROWS;
    }
    void TearDown() override
    {
        // Destroy the frame nodes first, while the mock pipelines are still alive; otherwise
        // FrameNode's dtor reaches ScrollablePattern::UnRegister2DragDropManager through a
        // dangling context.
        frameNodes_.clear();
        pipes_.clear();
        TestNG::TearDown();
    }

protected:
    // Wires pattern -> frameNode -> pipeline so HandleArrowKeyScroll passes the
    // host/pipeline/focus-active gates. Returned RefPtrs must be kept alive by the caller.
    void MountPattern(const RefPtr<ScrollablePattern>& pattern, const std::string& tag, bool focusActive)
    {
        auto frameNode = FrameNode::CreateFrameNode(tag, ++g_keyboardScrollNodeId, pattern);
        auto pipe = AceType::MakeRefPtr<PipelineContext>();
        auto focusManager = AceType::MakeRefPtr<FocusManager>(pipe);
        focusManager->isFocusActive_ = focusActive;
        pipe->focusManager_ = focusManager;
        frameNode->context_ = AceType::RawPtr(pipe);
        pattern->frameNode_ = frameNode;
        frameNodes_.emplace_back(frameNode);
        pipes_.emplace_back(pipe);
    }

    // Replaces the pattern's ScrollableEvent with a bare Scrollable (no pipeline context),
    // so ProcessAxisUpdateEvent returns early and lastMainDelta_ records the keyboard delta.
    RefPtr<Scrollable> AttachBareScrollable(const RefPtr<ScrollablePattern>& pattern, Axis axis)
    {
        auto scrollableEvent = AceType::MakeRefPtr<ScrollableEvent>(axis);
        auto scrollable = AceType::MakeRefPtr<Scrollable>();
        scrollableEvent->SetScrollable(scrollable);
        pattern->scrollableEvent_ = scrollableEvent;
        return scrollable;
    }

    KeyEvent CreateKeyEvent(KeyCode code, KeyAction action)
    {
        KeyEvent event;
        event.code = code;
        event.action = action;
        event.pressedCodes = { code };
        return event;
    }

    std::vector<RefPtr<FrameNode>> frameNodes_;
    std::vector<RefPtr<PipelineContext>> pipes_;
};

/**
 * @tc.name: IsArrowKeyMatchAxisVertical001
 * @tc.desc: Test IsArrowKeyMatchAxis with VERTICAL axis
 * @tc.type: FUNC
 * AC-1.1, AC-1.2, AC-1.5: vertical scroll matches up/down keys only
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, IsArrowKeyMatchAxisVertical001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    pattern->axis_ = Axis::VERTICAL;
    EXPECT_TRUE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_UP));
    EXPECT_TRUE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_DOWN));
    EXPECT_FALSE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_LEFT));
    EXPECT_FALSE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_RIGHT));
}

/**
 * @tc.name: IsArrowKeyMatchAxisHorizontal001
 * @tc.desc: Test IsArrowKeyMatchAxis with HORIZONTAL axis
 * @tc.type: FUNC
 * AC-1.3, AC-1.4, AC-1.6: horizontal scroll matches left/right keys only
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, IsArrowKeyMatchAxisHorizontal001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    pattern->axis_ = Axis::HORIZONTAL;
    EXPECT_FALSE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_UP));
    EXPECT_FALSE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_DOWN));
    EXPECT_TRUE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_LEFT));
    EXPECT_TRUE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_RIGHT));
}

/**
 * @tc.name: IsArrowKeyMatchAxisNone001
 * @tc.desc: Test IsArrowKeyMatchAxis with NONE axis
 * @tc.type: FUNC
 * AC-4.5: NONE axis does not match any arrow key
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, IsArrowKeyMatchAxisNone001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ScrollPattern>();
    pattern->axis_ = Axis::NONE;
    EXPECT_FALSE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_UP));
    EXPECT_FALSE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_DOWN));
    EXPECT_FALSE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_LEFT));
    EXPECT_FALSE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_RIGHT));
}

/**
 * @tc.name: IsArrowKeyMatchAxisFree001
 * @tc.desc: Test IsArrowKeyMatchAxis with FREE axis (ScrollPattern)
 * @tc.type: FUNC
 * AC-4.1~4.4: free scroll matches all four arrow keys
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, IsArrowKeyMatchAxisFree001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ScrollPattern>();
    pattern->axis_ = Axis::FREE;
    EXPECT_TRUE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_UP));
    EXPECT_TRUE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_DOWN));
    EXPECT_TRUE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_LEFT));
    EXPECT_TRUE(pattern->IsArrowKeyMatchAxis(KeyCode::KEY_DPAD_RIGHT));
}

/**
 * @tc.name: CalculateScrollDelta001
 * @tc.desc: Test CalculateScrollDelta matches ConvertToOffset formula
 * @tc.type: FUNC
 * AC-3.1: step = rows * MOUSE_WHEEL_DEGREES * (LINE_HEIGHT_DESKTOP / MOUSE_WHEEL_DEGREES).ConvertToPx()
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, CalculateScrollDelta001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    int32_t rows = 3;
    float delta = pattern->CalculateScrollDelta(rows);
    double result = static_cast<double>(rows) * MOUSE_WHEEL_DEGREES;
    Dimension linePerDegree = LINE_HEIGHT_DESKTOP / MOUSE_WHEEL_DEGREES;
    double expected = result * linePerDegree.ConvertToPx();
    EXPECT_NEAR(delta, static_cast<float>(expected), DELTA_EPSILON);
}

/**
 * @tc.name: CalculateScrollDeltaDifferentRows001
 * @tc.desc: Test CalculateScrollDelta with different rows values
 * @tc.type: FUNC
 * AC-3.3: step changes with rows setting
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, CalculateScrollDeltaDifferentRows001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    for (int32_t rows = 1; rows <= 10; ++rows) {
        float delta = pattern->CalculateScrollDelta(rows);
        double result = static_cast<double>(rows) * MOUSE_WHEEL_DEGREES;
        Dimension linePerDegree = LINE_HEIGHT_DESKTOP / MOUSE_WHEEL_DEGREES;
        double expected = result * linePerDegree.ConvertToPx();
        EXPECT_NEAR(delta, static_cast<float>(expected), DELTA_EPSILON);
        if (rows > 1) {
            float prevDelta = pattern->CalculateScrollDelta(rows - 1);
            EXPECT_GT(delta, prevDelta);
        }
    }
}

/**
 * @tc.name: CalculateScrollDeltaEqualsWheelStep001
 * @tc.desc: Verify one key press equals one wheel notch (3 rows x 21vp = 63vp, AC-3.1),
 *           and that the value matches rows * LINE_HEIGHT_DESKTOP (ADR-5: divide 21/15 before px conversion).
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, CalculateScrollDeltaEqualsWheelStep001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    // Default rows = 3: one key press == one wheel notch == 63vp
    EXPECT_NEAR(pattern->CalculateScrollDelta(3), 63.0f, DELTA_EPSILON);
    // Equivalent to rows * LINE_HEIGHT_DESKTOP (ensured by the divide-before-convert order, ADR-5)
    for (int32_t rows = 1; rows <= 5; ++rows) {
        double expected = static_cast<double>(rows) * LINE_HEIGHT_DESKTOP.ConvertToPx();
        EXPECT_NEAR(static_cast<double>(pattern->CalculateScrollDelta(rows)), expected, DELTA_EPSILON);
    }
}

/**
 * @tc.name: HandleArrowKeyScrollFocusActive001
 * @tc.desc: Test that scroll is NOT triggered when focus IS active
 * @tc.type: FUNC
 * AC-2.1: focus active -> navigate, not scroll
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, HandleArrowKeyScrollFocusActive001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    pattern->axis_ = Axis::VERTICAL;
    MountPattern(pattern, V2::LIST_ETS_TAG, true);
    auto scrollable = AttachBareScrollable(pattern, Axis::VERTICAL);
    auto event = CreateKeyEvent(KeyCode::KEY_DPAD_UP, KeyAction::DOWN);
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(event));
    EXPECT_NEAR(scrollable->lastMainDelta_, 0.0, DELTA_EPSILON);
}

/**
 * @tc.name: HandleArrowKeyScrollNotDown001
 * @tc.desc: Test that scroll is NOT triggered when action is not DOWN
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, HandleArrowKeyScrollNotDown001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    pattern->axis_ = Axis::VERTICAL;
    MountPattern(pattern, V2::LIST_ETS_TAG, false);
    auto scrollable = AttachBareScrollable(pattern, Axis::VERTICAL);
    auto event = CreateKeyEvent(KeyCode::KEY_DPAD_UP, KeyAction::UP);
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(event));
    EXPECT_NEAR(scrollable->lastMainDelta_, 0.0, DELTA_EPSILON);
}

/**
 * @tc.name: HandleArrowKeyScrollDirectionMismatch001
 * @tc.desc: Test that scroll is NOT triggered when direction does not match axis
 * @tc.type: FUNC
 * AC-1.5: vertical component does not respond to left/right
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, HandleArrowKeyScrollDirectionMismatch001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    pattern->axis_ = Axis::VERTICAL;
    MountPattern(pattern, V2::LIST_ETS_TAG, false);
    auto scrollable = AttachBareScrollable(pattern, Axis::VERTICAL);
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(CreateKeyEvent(KeyCode::KEY_DPAD_LEFT, KeyAction::DOWN)));
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(CreateKeyEvent(KeyCode::KEY_DPAD_RIGHT, KeyAction::DOWN)));
    EXPECT_NEAR(scrollable->lastMainDelta_, 0.0, DELTA_EPSILON);
}

/**
 * @tc.name: HandleArrowKeyScrollDirectionMismatchHorizontal001
 * @tc.desc: Test horizontal direction mismatch
 * @tc.type: FUNC
 * AC-1.6: horizontal component does not respond to up/down
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, HandleArrowKeyScrollDirectionMismatchHorizontal001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    pattern->axis_ = Axis::HORIZONTAL;
    MountPattern(pattern, V2::LIST_ETS_TAG, false);
    auto scrollable = AttachBareScrollable(pattern, Axis::HORIZONTAL);
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(CreateKeyEvent(KeyCode::KEY_DPAD_UP, KeyAction::DOWN)));
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(CreateKeyEvent(KeyCode::KEY_DPAD_DOWN, KeyAction::DOWN)));
    EXPECT_NEAR(scrollable->lastMainDelta_, 0.0, DELTA_EPSILON);
}

/**
 * @tc.name: HandleArrowKeyScrollNullScrollable001
 * @tc.desc: Test with null scrollable -> returns false
 * @tc.type: FUNC
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, HandleArrowKeyScrollNullScrollable001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    pattern->axis_ = Axis::VERTICAL;
    MountPattern(pattern, V2::LIST_ETS_TAG, false);
    pattern->scrollableEvent_ = nullptr;
    auto event = CreateKeyEvent(KeyCode::KEY_DPAD_UP, KeyAction::DOWN);
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(event));
}

/**
 * @tc.name: HandleArrowKeyScrollTriggered001
 * @tc.desc: Test scroll IS triggered when focus not active + direction matches
 * @tc.type: FUNC
 * AC-1.1: vertical + up key + focus not active -> scroll toward start (positive delta)
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, HandleArrowKeyScrollTriggered001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    pattern->axis_ = Axis::VERTICAL;
    MountPattern(pattern, V2::LIST_ETS_TAG, false);
    auto scrollable = AttachBareScrollable(pattern, Axis::VERTICAL);
    auto event = CreateKeyEvent(KeyCode::KEY_DPAD_UP, KeyAction::DOWN);
    EXPECT_TRUE(pattern->HandleArrowKeyScroll(event));
    // UP key maps to positive delta (scroll toward start), same sign as mouse wheel up.
    EXPECT_NEAR(scrollable->lastMainDelta_, pattern->CalculateScrollDelta(3), DELTA_EPSILON);
}

/**
 * @tc.name: HandleArrowKeyScrollTriggered002
 * @tc.desc: Test scroll IS triggered for down key
 * @tc.type: FUNC
 * AC-1.2: vertical + down key + focus not active -> scroll toward end (negative delta)
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, HandleArrowKeyScrollTriggered002, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    pattern->axis_ = Axis::VERTICAL;
    MountPattern(pattern, V2::LIST_ETS_TAG, false);
    auto scrollable = AttachBareScrollable(pattern, Axis::VERTICAL);
    auto event = CreateKeyEvent(KeyCode::KEY_DPAD_DOWN, KeyAction::DOWN);
    EXPECT_TRUE(pattern->HandleArrowKeyScroll(event));
    EXPECT_NEAR(scrollable->lastMainDelta_, -pattern->CalculateScrollDelta(3), DELTA_EPSILON);
}

/**
 * @tc.name: HandleArrowKeyScrollTriggered003
 * @tc.desc: Test scroll IS triggered for left key on horizontal
 * @tc.type: FUNC
 * AC-1.3: horizontal + left key -> scroll toward start (positive delta)
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, HandleArrowKeyScrollTriggered003, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    pattern->axis_ = Axis::HORIZONTAL;
    MountPattern(pattern, V2::LIST_ETS_TAG, false);
    auto scrollable = AttachBareScrollable(pattern, Axis::HORIZONTAL);
    auto event = CreateKeyEvent(KeyCode::KEY_DPAD_LEFT, KeyAction::DOWN);
    EXPECT_TRUE(pattern->HandleArrowKeyScroll(event));
    EXPECT_NEAR(scrollable->lastMainDelta_, pattern->CalculateScrollDelta(3), DELTA_EPSILON);
}

/**
 * @tc.name: HandleArrowKeyScrollTriggered004
 * @tc.desc: Test scroll IS triggered for right key on horizontal
 * @tc.type: FUNC
 * AC-1.4: horizontal + right key -> scroll toward end (negative delta)
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, HandleArrowKeyScrollTriggered004, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    pattern->axis_ = Axis::HORIZONTAL;
    MountPattern(pattern, V2::LIST_ETS_TAG, false);
    auto scrollable = AttachBareScrollable(pattern, Axis::HORIZONTAL);
    auto event = CreateKeyEvent(KeyCode::KEY_DPAD_RIGHT, KeyAction::DOWN);
    EXPECT_TRUE(pattern->HandleArrowKeyScroll(event));
    EXPECT_NEAR(scrollable->lastMainDelta_, -pattern->CalculateScrollDelta(3), DELTA_EPSILON);
}

/**
 * @tc.name: HandleArrowKeyScrollDefaultRows001
 * @tc.desc: Test default rows=3 when SettingDataManager unavailable
 * @tc.type: FUNC
 * AC-7.1: setting unavailable -> default step (3 rows)
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, HandleArrowKeyScrollDefaultRows001, TestSize.Level1)
{
    KeyboardScrollSettingsUtils::observerRegistered_ = false;
    KeyboardScrollSettingsUtils::rowsValid_ = false;
    KeyboardScrollSettingsUtils::cachedRows_ = 0;
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    int32_t rows = pattern->GetMouseScrollRows();
    EXPECT_GT(rows, 0);
}

/**
 * @tc.name: HandleArrowKeyScrollFreeMode001
 * @tc.desc: Free axis matches all four arrow keys, but without a mounted
 *           FreeScrollController (freeScroll_ null, no OnModifyDone) the key is
 *           not consumed
 * @tc.type: FUNC
 * AC-4.1~4.4: free mode + any arrow key -> scroll triggered (key matching itself
 * is covered by IsArrowKeyMatchAxisFree001 and the on-device KeyboardFree cases;
 * here the null-controller non-consumption is verified)
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, HandleArrowKeyScrollFreeMode001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ScrollPattern>();
    pattern->axis_ = Axis::FREE;
    MountPattern(pattern, V2::SCROLL_ETS_TAG, false);
    AttachBareScrollable(pattern, Axis::FREE);
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(CreateKeyEvent(KeyCode::KEY_DPAD_UP, KeyAction::DOWN)));
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(CreateKeyEvent(KeyCode::KEY_DPAD_DOWN, KeyAction::DOWN)));
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(CreateKeyEvent(KeyCode::KEY_DPAD_LEFT, KeyAction::DOWN)));
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(CreateKeyEvent(KeyCode::KEY_DPAD_RIGHT, KeyAction::DOWN)));
}

/**
 * @tc.name: HandleArrowKeyScrollFreeModeNone001
 * @tc.desc: Test NONE axis does not respond to any arrow key
 * @tc.type: FUNC
 * AC-4.5: NONE mode -> no scroll
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, HandleArrowKeyScrollFreeModeNone001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ScrollPattern>();
    pattern->axis_ = Axis::NONE;
    MountPattern(pattern, V2::SCROLL_ETS_TAG, false);
    AttachBareScrollable(pattern, Axis::NONE);
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(CreateKeyEvent(KeyCode::KEY_DPAD_UP, KeyAction::DOWN)));
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(CreateKeyEvent(KeyCode::KEY_DPAD_DOWN, KeyAction::DOWN)));
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(CreateKeyEvent(KeyCode::KEY_DPAD_LEFT, KeyAction::DOWN)));
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(CreateKeyEvent(KeyCode::KEY_DPAD_RIGHT, KeyAction::DOWN)));
}

/**
 * @tc.name: HandleArrowKeyScrollEnableOptOut001
 * @tc.desc: Test enableScrollWithKeyboard opt-out flag gating
 * @tc.type: FUNC
 * AC-15.1/15.2/15.4: default enabled; false disables scroll and lets the key bubble;
 * true re-enables scrolling
 */
HWTEST_F(ScrollableKeyboardScrollTestNg, HandleArrowKeyScrollEnableOptOut001, TestSize.Level1)
{
    auto pattern = AceType::MakeRefPtr<ListPattern>();
    pattern->axis_ = Axis::VERTICAL;
    MountPattern(pattern, V2::LIST_ETS_TAG, false);
    auto scrollable = AttachBareScrollable(pattern, Axis::VERTICAL);
    auto event = CreateKeyEvent(KeyCode::KEY_DPAD_UP, KeyAction::DOWN);

    // Default: enabled
    EXPECT_TRUE(pattern->GetIsAllowKeyboard());
    EXPECT_TRUE(pattern->HandleArrowKeyScroll(event));
    EXPECT_NEAR(scrollable->lastMainDelta_, pattern->CalculateScrollDelta(3), DELTA_EPSILON);

    // Disabled: not consumed, no scroll (event bubbles, restoring pre-change behavior)
    pattern->SetIsAllowKeyboard(false);
    scrollable->lastMainDelta_ = 0.0;
    EXPECT_FALSE(pattern->HandleArrowKeyScroll(event));
    EXPECT_NEAR(scrollable->lastMainDelta_, 0.0, DELTA_EPSILON);

    // Re-enabled: scroll restored
    pattern->SetIsAllowKeyboard(true);
    EXPECT_TRUE(pattern->HandleArrowKeyScroll(event));
    EXPECT_NEAR(scrollable->lastMainDelta_, pattern->CalculateScrollDelta(3), DELTA_EPSILON);
}

} // namespace OHOS::Ace::NG
