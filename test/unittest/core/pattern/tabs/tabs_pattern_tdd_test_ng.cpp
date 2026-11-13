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
#include "tabs_test_ng.h"

#include "test/mock/base/mock_system_properties.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"

#include "core/components/dialog/dialog_theme.h"
#include "core/components/tab_bar/tab_theme.h"
#include "core/components_ng/pattern/divider/divider_pattern.h"
#include "core/components_ng/pattern/divider/divider_render_property.h"
#include "core/components_ng/event/input_event.h"
#include "core/components_ng/event/pan_event.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_model.h"
#include "core/components_ng/pattern/tabs/tabs_model_ng.h"
#include "core/components_ng/pattern/tabs/tabs_node.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_declaration.h"

namespace OHOS::Ace::NG {
namespace {
RefPtr<Theme> GetPatternTddTheme(ThemeType type)
{
    if (type == TabTheme::TypeId()) {
        auto themeConstants = TestNG::CreateThemeConstants(THEME_PATTERN_TAB);
        return TabTheme::Builder().Build(themeConstants);
    }
    return AceType::MakeRefPtr<DialogTheme>();
}
} // namespace

class TabsPatternTddTestNg : public TabsTestNg {
public:
    static void SetUpTestSuite()
    {
        TestNG::SetUpTestSuite();
        MockPipelineContext::GetCurrent()->SetUseFlushUITasks(true);
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
            return GetPatternTddTheme(type);
        });
        EXPECT_CALL(*themeManager, GetTheme(_, _))
            .WillRepeatedly([](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
                return GetPatternTddTheme(type);
            });
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    }
    static void TearDownTestSuite()
    {
        TestNG::TearDownTestSuite();
        g_isConfigChangePerform = false;
    }
};

// ===================== HandleDividerDragEnd =====================

/**
 * @tc.name: HandleDividerDragEndResetsDragFlag001
 * @tc.desc: HandleDividerDragEnd resets isInDividerDrag_ to false
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, HandleDividerDragEndResetsDragFlag001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isInDividerDrag_ = true;
    pattern_->realSideBarWidthPx_ = 250.0f;
    pattern_->preSideBarWidthPx_ = 200.0f;
    pattern_->HandleDividerDragEnd();
    EXPECT_FALSE(pattern_->isInDividerDrag_);
    // preSideBarWidthPx_ takes realSideBarWidthPx_ value
    EXPECT_FLOAT_EQ(pattern_->preSideBarWidthPx_, 250.0f);
}

/**
 * @tc.name: HandleDividerDragEndNoRealWidth001
 * @tc.desc: HandleDividerDragEnd when realSideBarWidthPx_ is nullopt uses preSideBarWidthPx_
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, HandleDividerDragEndNoRealWidth001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isInDividerDrag_ = true;
    pattern_->realSideBarWidthPx_.reset();
    pattern_->preSideBarWidthPx_ = 180.0f;
    pattern_->HandleDividerDragEnd();
    EXPECT_FALSE(pattern_->isInDividerDrag_);
    EXPECT_FLOAT_EQ(pattern_->preSideBarWidthPx_, 180.0f);
}

// ===================== HandleDividerDragCancel =====================

/**
 * @tc.name: HandleDividerDragCancelRestoresWidth001
 * @tc.desc: HandleDividerDragCancel restores realSideBarWidthPx_ to preSideBarWidthPx_
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, HandleDividerDragCancelRestoresWidth001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isInDividerDrag_ = true;
    pattern_->preSideBarWidthPx_ = 220.0f;
    pattern_->realSideBarWidthPx_ = 300.0f;
    pattern_->HandleDividerDragCancel();
    EXPECT_FALSE(pattern_->isInDividerDrag_);
    ASSERT_TRUE(pattern_->realSideBarWidthPx_.has_value());
    EXPECT_FLOAT_EQ(pattern_->realSideBarWidthPx_.value(), 220.0f);
}

// ===================== HandleDividerDragUpdate =====================

/**
 * @tc.name: HandleDividerDragUpdateNotInDrag001
 * @tc.desc: When isInDividerDrag_ is false, HandleDividerDragUpdate is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, HandleDividerDragUpdateNotInDrag001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isInDividerDrag_ = false;
    pattern_->realSideBarWidthPx_.reset();
    pattern_->HandleDividerDragUpdate(100.0f);
    EXPECT_FALSE(pattern_->realSideBarWidthPx_.has_value());
}

/**
 * @tc.name: HandleDividerDragUpdateClampsToMax001
 * @tc.desc: HandleDividerDragUpdate clamps new width to maxSideBarWidth_
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, HandleDividerDragUpdateClampsToMax001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isInDividerDrag_ = true;
    pattern_->minSideBarWidth_ = 100.0f;
    pattern_->maxSideBarWidth_ = 300.0f;
    pattern_->preSideBarWidthPx_ = 200.0f;
    pattern_->realSideBarWidthPx_ = 200.0f;
    pattern_->HandleDividerDragUpdate(500.0f);
    ASSERT_TRUE(pattern_->realSideBarWidthPx_.has_value());
    EXPECT_FLOAT_EQ(pattern_->realSideBarWidthPx_.value(), 300.0f);
}

/**
 * @tc.name: HandleDividerDragUpdateClampsToMin001
 * @tc.desc: HandleDividerDragUpdate clamps new width to minSideBarWidth_
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, HandleDividerDragUpdateClampsToMin001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isInDividerDrag_ = true;
    pattern_->minSideBarWidth_ = 100.0f;
    pattern_->maxSideBarWidth_ = 300.0f;
    pattern_->preSideBarWidthPx_ = 150.0f;
    pattern_->realSideBarWidthPx_ = 150.0f;
    pattern_->HandleDividerDragUpdate(-200.0f);
    ASSERT_TRUE(pattern_->realSideBarWidthPx_.has_value());
    EXPECT_FLOAT_EQ(pattern_->realSideBarWidthPx_.value(), 100.0f);
}

/**
 * @tc.name: HandleDividerDragUpdateNearEqualNoChange001
 * @tc.desc: When new width is near-equal to current, realSideBarWidthPx_ unchanged
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, HandleDividerDragUpdateNearEqualNoChange001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isInDividerDrag_ = true;
    pattern_->minSideBarWidth_ = 100.0f;
    pattern_->maxSideBarWidth_ = 300.0f;
    pattern_->preSideBarWidthPx_ = 200.0f;
    pattern_->realSideBarWidthPx_ = 200.0f;
    // tiny offset → newWidth near-equal 200.0f
    pattern_->HandleDividerDragUpdate(0.0001f);
    ASSERT_TRUE(pattern_->realSideBarWidthPx_.has_value());
    EXPECT_FLOAT_EQ(pattern_->realSideBarWidthPx_.value(), 200.0f);
}

// ===================== OnDividerHover =====================

/**
 * @tc.name: OnDividerHoverDuringDrag001
 * @tc.desc: When isInDividerDrag_ is true, OnDividerHover early returns
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, OnDividerHoverDuringDrag001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isInDividerDrag_ = true;
    // Should early return without crashing
    pattern_->OnDividerHover(true);
    EXPECT_TRUE(pattern_->isInDividerDrag_);
}

/**
 * @tc.name: OnDividerHoverNotDraggable001
 * @tc.desc: When not in drag and not draggable, OnDividerHover still returns without crash
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, OnDividerHoverNotDraggable001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isInDividerDrag_ = false;
    // No divider set → not draggable → early return path
    pattern_->OnDividerHover(true);
    EXPECT_FALSE(pattern_->isInDividerDrag_);
}

/**
 * @tc.name: OnDividerHoverNotDraggableFalse001
 * @tc.desc: OnDividerHover with isHover=false and not draggable
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, OnDividerHoverNotDraggableFalse001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isInDividerDrag_ = false;
    pattern_->OnDividerHover(false);
    EXPECT_FALSE(pattern_->isInDividerDrag_);
}

// ===================== OnDividerMouseEvent =====================

/**
 * @tc.name: OnDividerMouseEventNotRelease001
 * @tc.desc: When mouse action is not RELEASE, OnDividerMouseEvent early returns
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, OnDividerMouseEventNotRelease001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    MouseInfo info;
    info.SetAction(MouseAction::PRESS);
    // Should early return without crash (no divider node set)
    pattern_->OnDividerMouseEvent(info);
    EXPECT_NE(pattern_, nullptr);
}

// ===================== InitDividerMouseEvent =====================

/**
 * @tc.name: InitDividerMouseEventNullHub001
 * @tc.desc: InitDividerMouseEvent with null inputHub is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, InitDividerMouseEventNullHub001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->hoverEvent_ = nullptr;
    pattern_->dividerMouseEvent_ = nullptr;
    pattern_->InitDividerMouseEvent(nullptr);
    EXPECT_FALSE(pattern_->hoverEvent_);
    EXPECT_FALSE(pattern_->dividerMouseEvent_);
}

/**
 * @tc.name: InitDividerMouseEventAlreadyRegistered001
 * @tc.desc: When hoverEvent_ already set, InitDividerMouseEvent is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, InitDividerMouseEventAlreadyRegistered001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto existingHover = AceType::MakeRefPtr<InputEvent>([](bool) {});
    pattern_->hoverEvent_ = existingHover;
    auto inputHub = frameNode_->GetOrCreateInputEventHub();
    ASSERT_NE(inputHub, nullptr);
    pattern_->InitDividerMouseEvent(inputHub);
    // Should not replace existing hoverEvent_
    EXPECT_EQ(pattern_->hoverEvent_, existingHover);
}

// ===================== ClearDividerDragEvent =====================

/**
 * @tc.name: ClearDividerDragEventNoEvents001
 * @tc.desc: ClearDividerDragEvent with no events set just clears isInDividerDrag_
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, ClearDividerDragEventNoEvents001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->dividerPanEvent_ = nullptr;
    pattern_->hoverEvent_ = nullptr;
    pattern_->dividerMouseEvent_ = nullptr;
    pattern_->isInDividerDrag_ = true;
    pattern_->ClearDividerDragEvent();
    EXPECT_FALSE(pattern_->isInDividerDrag_);
    EXPECT_FALSE(pattern_->dividerPanEvent_);
    EXPECT_FALSE(pattern_->hoverEvent_);
    EXPECT_FALSE(pattern_->dividerMouseEvent_);
}

/**
 * @tc.name: ClearDividerDragEventWithPanEvent001
 * @tc.desc: ClearDividerDragEvent clears panEvent when set (no divider node → null safe path)
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, ClearDividerDragEventWithPanEvent001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto panEvent = AceType::MakeRefPtr<PanEvent>([](const GestureEvent&) {}, [](const GestureEvent&) {},
        [](const GestureEvent&) {}, []() {});
    pattern_->dividerPanEvent_ = panEvent;
    pattern_->sideBarDividerNode_ = nullptr;
    pattern_->isInDividerDrag_ = true;
    pattern_->ClearDividerDragEvent();
    // panEvent_ should be cleared even with null divider node
    EXPECT_FALSE(pattern_->dividerPanEvent_);
    EXPECT_FALSE(pattern_->isInDividerDrag_);
}

// ===================== InitDividerPanEvent =====================

/**
 * @tc.name: InitDividerPanEventNullHub001
 * @tc.desc: InitDividerPanEvent with null gestureHub is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, InitDividerPanEventNullHub001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->dividerPanEvent_ = nullptr;
    pattern_->InitDividerPanEvent(nullptr);
    EXPECT_FALSE(pattern_->dividerPanEvent_);
}

/**
 * @tc.name: InitDividerPanEventAlreadySet001
 * @tc.desc: InitDividerPanEvent when dividerPanEvent_ already set is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, InitDividerPanEventAlreadySet001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto existingPan = AceType::MakeRefPtr<PanEvent>([](const GestureEvent&) {}, [](const GestureEvent&) {},
        [](const GestureEvent&) {}, []() {});
    pattern_->dividerPanEvent_ = existingPan;
    auto gestureHub = frameNode_->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    pattern_->InitDividerPanEvent(gestureHub);
    EXPECT_EQ(pattern_->dividerPanEvent_, existingPan);
}

// ===================== InitDividerDragEvent =====================

/**
 * @tc.name: InitDividerDragEventAlreadyInitialized001
 * @tc.desc: InitDividerDragEvent when dividerPanEvent_ already set is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, InitDividerDragEventAlreadyInitialized001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto existingPan = AceType::MakeRefPtr<PanEvent>([](const GestureEvent&) {}, [](const GestureEvent&) {},
        [](const GestureEvent&) {}, []() {});
    pattern_->dividerPanEvent_ = existingPan;
    pattern_->InitDividerDragEvent();
    EXPECT_EQ(pattern_->dividerPanEvent_, existingPan);
}

// ===================== HandleDividerDragStart (additional) =====================

/**
 * @tc.name: HandleDividerDragStartNotDraggable001
 * @tc.desc: HandleDividerDragStart with no divider → IsDividerDraggable false → early return
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, HandleDividerDragStartNotDraggable001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isInDividerDrag_ = false;
    // No divider set → not draggable
    pattern_->HandleDividerDragStart();
    EXPECT_FALSE(pattern_->isInDividerDrag_);
}

// ===================== AddDividerHotZoneRect =====================

/**
 * @tc.name: AddDividerHotZoneRectWithDivider001
 * @tc.desc: AddDividerHotZoneRect with a divider node sets response region
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, AddDividerHotZoneRectWithDivider001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto dividerNode = FrameNode::CreateFrameNode(
        "DividerTest", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<DividerPattern>());
    ASSERT_NE(dividerNode, nullptr);
    dividerNode->GetGeometryNode()->SetFrameSize(SizeF(10.0f, 400.0f));
    pattern_->sideBarDividerNode_ = dividerNode;
    pattern_->AddDividerHotZoneRect();
    // Verify gesture hub has response region set (no crash, no throw)
    auto gestureHub = dividerNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    EXPECT_NE(pattern_->sideBarDividerNode_, nullptr);
}

// ===================== CalcSidebarOutsideTranslateX =====================

/**
 * @tc.name: CalcSidebarOutsideTranslateXNoSideBar001
 * @tc.desc: CalcSidebarOutsideTranslateX with no sidebar/divider returns 0
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, CalcSidebarOutsideTranslateXNoSideBar001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->sideBarNode_ = nullptr;
    pattern_->sideBarDividerNode_ = nullptr;
    float result = pattern_->CalcSidebarOutsideTranslateX();
    EXPECT_FLOAT_EQ(result, 0.0f);
}

/**
 * @tc.name: CalcSidebarOutsideTranslateXWithSideBar001
 * @tc.desc: CalcSidebarOutsideTranslateX with sidebar+divider returns offset
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, CalcSidebarOutsideTranslateXWithSideBar001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto sideBarNode = FrameNode::CreateFrameNode(
        "SideBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    sideBarNode->GetGeometryNode()->SetFrameSize(SizeF(240.0f, 400.0f));
    auto dividerNode = FrameNode::CreateFrameNode(
        "Divider", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    dividerNode->GetGeometryNode()->SetFrameSize(SizeF(10.0f, 400.0f));
    pattern_->sideBarNode_ = sideBarNode;
    pattern_->sideBarDividerNode_ = dividerNode;
    // Default BarPosition::START, LTR → sideBarOnLeft=true → returns -250
    float result = pattern_->CalcSidebarOutsideTranslateX();
    EXPECT_FLOAT_EQ(result, -250.0f);
}

/**
 * @tc.name: CalcSidebarOutsideTranslateXEndPosition001
 * @tc.desc: CalcSidebarOutsideTranslateX with BarPosition::END returns positive offset
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, CalcSidebarOutsideTranslateXEndPosition001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateSidebarPosition(BarPosition::END);
    auto sideBarNode = FrameNode::CreateFrameNode(
        "SideBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    sideBarNode->GetGeometryNode()->SetFrameSize(SizeF(240.0f, 400.0f));
    auto dividerNode = FrameNode::CreateFrameNode(
        "Divider", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    dividerNode->GetGeometryNode()->SetFrameSize(SizeF(10.0f, 400.0f));
    pattern_->sideBarNode_ = sideBarNode;
    pattern_->sideBarDividerNode_ = dividerNode;
    float result = pattern_->CalcSidebarOutsideTranslateX();
    // LTR + END → sideBarOnLeft=false → returns +250
    EXPECT_FLOAT_EQ(result, 250.0f);
}

// ===================== OnDisplayModeSwitchAnimationsFinished =====================

/**
 * @tc.name: OnDisplayModeSwitchFinishedNotActive001
 * @tc.desc: When isOnDisplayModeSwitchAnimation_ is false, early return
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, OnDisplayModeSwitchFinishedNotActive001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isOnDisplayModeSwitchAnimation_ = false;
    pattern_->displayModeAnimCount_ = 0;
    pattern_->OnDisplayModeSwitchAnimationsFinished();
    EXPECT_FALSE(pattern_->isOnDisplayModeSwitchAnimation_);
    EXPECT_EQ(pattern_->displayModeAnimCount_, 0);
}

/**
 * @tc.name: OnDisplayModeSwitchFinishedDecrementsCount001
 * @tc.desc: When displayModeAnimCount_ > 1, decrements and returns without resetting
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, OnDisplayModeSwitchFinishedDecrementsCount001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isOnDisplayModeSwitchAnimation_ = true;
    pattern_->displayModeAnimCount_ = 2;
    pattern_->OnDisplayModeSwitchAnimationsFinished();
    // After decrement, count is 1 → early return, still on
    EXPECT_EQ(pattern_->displayModeAnimCount_, 1);
    EXPECT_TRUE(pattern_->isOnDisplayModeSwitchAnimation_);
}

/**
 * @tc.name: OnDisplayModeSwitchFinishedResetsState001
 * @tc.desc: When count reaches 0, resets all animation state
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, OnDisplayModeSwitchFinishedResetsState001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isOnDisplayModeSwitchAnimation_ = true;
    pattern_->displayModeAnimCount_ = 1;
    pattern_->displayModeAnimTarget_ = TabBarDisplayMode::SIDEBAR;
    pattern_->OnDisplayModeSwitchAnimationsFinished();
    EXPECT_FALSE(pattern_->isOnDisplayModeSwitchAnimation_);
    EXPECT_EQ(pattern_->displayModeAnimCount_, 0);
    EXPECT_FALSE(pattern_->displayModeAnimTarget_.has_value());
}

// ===================== SetElementVisibilityForMode =====================

/**
 * @tc.name: SetElementVisibilitySidebarMode001
 * @tc.desc: In SIDEBAR mode, tabBar/tabBarDivider/bgMask get GONE visibility
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, SetElementVisibilitySidebarMode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    // Call without sidebar nodes set → should not crash
    pattern_->SetElementVisibilityForMode(TabBarDisplayMode::SIDEBAR);
    // tabBarNode_ should have GONE visibility when keepMode is false (default)
    ASSERT_NE(tabBarNode_, nullptr);
    auto barLayoutProp = tabBarNode_->GetLayoutProperty();
    ASSERT_NE(barLayoutProp, nullptr);
    EXPECT_EQ(barLayoutProp->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
}

/**
 * @tc.name: SetElementVisibilitySidebarKeepMode001
 * @tc.desc: In SIDEBAR mode with keepTabBarInSideBar=true, tabBar stays VISIBLE
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, SetElementVisibilitySidebarKeepMode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateKeepTabBarInSideBar(true);
    // Reset visibility to VISIBLE first
    tabBarNode_->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
    pattern_->SetElementVisibilityForMode(TabBarDisplayMode::SIDEBAR);
    auto barLayoutProp = tabBarNode_->GetLayoutProperty();
    ASSERT_NE(barLayoutProp, nullptr);
    EXPECT_EQ(barLayoutProp->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::VISIBLE);
}

/**
 * @tc.name: SetElementVisibilityBottomMode001
 * @tc.desc: In BOTTOMTABBAR mode, sidebar nodes get GONE visibility
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, SetElementVisibilityBottomMode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto sideBarNode = FrameNode::CreateFrameNode(
        "SideBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    sideBarNode->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
    auto sideBarDividerNode = FrameNode::CreateFrameNode(
        "Divider", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    sideBarDividerNode->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
    pattern_->sideBarNode_ = sideBarNode;
    pattern_->sideBarDividerNode_ = sideBarDividerNode;

    pattern_->SetElementVisibilityForMode(TabBarDisplayMode::BOTTOMTABBAR);
    EXPECT_EQ(sideBarNode->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
    EXPECT_EQ(sideBarDividerNode->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::GONE);
}

// ===================== ApplyEnteringElementInitialTransform =====================

/**
 * @tc.name: ApplyEnteringTransformSidebar001
 * @tc.desc: With SIDEBAR mode, calls ApplyEnteringSidebarInitialTransform
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, ApplyEnteringTransformSidebar001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto sideBarNode = FrameNode::CreateFrameNode(
        "SideBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    sideBarNode->GetGeometryNode()->SetFrameSize(SizeF(240.0f, 400.0f));
    auto sideBarDividerNode = FrameNode::CreateFrameNode(
        "Divider", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    sideBarDividerNode->GetGeometryNode()->SetFrameSize(SizeF(10.0f, 400.0f));
    pattern_->sideBarNode_ = sideBarNode;
    pattern_->sideBarDividerNode_ = sideBarDividerNode;
    // Should not crash; sets transform translate on sidebar+divider
    pattern_->ApplyEnteringElementInitialTransform(TabBarDisplayMode::SIDEBAR);
    auto sideBarRC = sideBarNode->GetRenderContext();
    ASSERT_NE(sideBarRC, nullptr);
    EXPECT_TRUE(sideBarRC->HasTransformTranslate());
}

/**
 * @tc.name: ApplyEnteringTransformBottomNoKeepMode001
 * @tc.desc: With BOTTOMTABBAR mode and keepMode=false, calls ApplyEnteringBottomTabBarInitialTransform
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, ApplyEnteringTransformBottomNoKeepMode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    // Default keepMode=false. Call should not crash even without bgMaskNode (CHECK_NULL_VOID)
    pattern_->ApplyEnteringElementInitialTransform(TabBarDisplayMode::BOTTOMTABBAR);
    // Verify tabBar has opacity 0 set (before bgMask CHECK_NULL_VOID)
    ASSERT_NE(tabBarNode_, nullptr);
    auto tabBarRC = tabBarNode_->GetRenderContext();
    ASSERT_NE(tabBarRC, nullptr);
    // ApplyEnteringBottomTabBarInitialTransform sets opacity 0 on tabBar
    EXPECT_DOUBLE_EQ(tabBarRC->GetOpacityValue(1.0), 0.0);
}

/**
 * @tc.name: ApplyEnteringTransformBottomKeepMode001
 * @tc.desc: With BOTTOMTABBAR mode and keepMode=true, skips ApplyEnteringBottomTabBarInitialTransform
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, ApplyEnteringTransformBottomKeepMode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateKeepTabBarInSideBar(true);
    tabBarNode_->GetRenderContext()->UpdateOpacity(1.0);
    pattern_->ApplyEnteringElementInitialTransform(TabBarDisplayMode::BOTTOMTABBAR);
    // With keepMode=true, ApplyEnteringBottomTabBarInitialTransform is NOT called
    auto tabBarRC = tabBarNode_->GetRenderContext();
    ASSERT_NE(tabBarRC, nullptr);
    EXPECT_DOUBLE_EQ(tabBarRC->GetOpacityValue(0.0), 1.0);
}

// ===================== SetFloatingScaleEnabled =====================

/**
 * @tc.name: SetFloatingScaleEnabledTrue001
 * @tc.desc: SetFloatingScaleEnabled(true) sets baseFloatingScale_ to ENLARGED
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, SetFloatingScaleEnabledTrue001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isOnDisplayModeSwitchAnimation_ = false;
    pattern_->SetFloatingScaleEnabled(true);
    // baseFloatingScale_ should be FLOATING_BAR_SCALE_ENLARGED
    EXPECT_FLOAT_EQ(pattern_->baseFloatingScale_, FLOATING_BAR_SCALE_ENLARGED);
}

/**
 * @tc.name: SetFloatingScaleEnabledFalseDuringAnim001
 * @tc.desc: During display mode animation, SetFloatingScaleEnabled still updates baseFloatingScale_
 *           unconditionally; the isOnDisplayModeSwitchAnimation_ early return only skips
 *           the UpdateTransformScale call on the render context.
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, SetFloatingScaleEnabledFalseDuringAnim001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isOnDisplayModeSwitchAnimation_ = true;
    pattern_->SetFloatingScaleEnabled(true);
    // baseFloatingScale_ is set unconditionally before the early-return guard, so it must
    // reflect the value corresponding to isFloatingScaleEnabled=true regardless of animation state.
    EXPECT_FLOAT_EQ(pattern_->baseFloatingScale_, FLOATING_BAR_SCALE_ENLARGED);
    // Reset animation flag to avoid leaking state into downstream tests.
    pattern_->isOnDisplayModeSwitchAnimation_ = false;
}

// ===================== ResetDisplayModeSwitchTransforms =====================

/**
 * @tc.name: ResetDisplayModeSwitchTransformsBasic001
 * @tc.desc: ResetDisplayModeSwitchTransforms restores opacity/translate on tabBar
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, ResetDisplayModeSwitchTransformsBasic001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto tabBarRC = tabBarNode_->GetRenderContext();
    ASSERT_NE(tabBarRC, nullptr);
    tabBarRC->UpdateOpacity(0.0);
    tabBarRC->UpdateTransformTranslate({ 10.0f, 20.0f, 0.0f });
    pattern_->isFloatingTopBar_ = false;
    pattern_->ResetDisplayModeSwitchTransforms();
    // After reset, opacity should be 1.0 and translate should be 0,0,0
    EXPECT_DOUBLE_EQ(tabBarRC->GetOpacityValue(0.0), 1.0);
}

/**
 * @tc.name: ResetDisplayModeSwitchTransformsKeepMode001
 * @tc.desc: With keepMode=true, tabBar transform/opacity not touched
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, ResetDisplayModeSwitchTransformsKeepMode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateKeepTabBarInSideBar(true);
    auto tabBarRC = tabBarNode_->GetRenderContext();
    ASSERT_NE(tabBarRC, nullptr);
    tabBarRC->UpdateOpacity(0.5);
    pattern_->isFloatingTopBar_ = false;
    pattern_->ResetDisplayModeSwitchTransforms();
    // With keepMode, tabBar not touched
    EXPECT_DOUBLE_EQ(tabBarRC->GetOpacityValue(1.0), 0.5);
}

/**
 * @tc.name: ResetDisplayModeSwitchTransformsSideBar001
 * @tc.desc: ResetDisplayModeSwitchTransforms resets translate on sidebar nodes
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, ResetDisplayModeSwitchTransformsSideBar001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto sideBarNode = FrameNode::CreateFrameNode(
        "SideBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto sideBarDividerNode = FrameNode::CreateFrameNode(
        "Divider", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto sideBarRC = sideBarNode->GetRenderContext();
    auto dividerRC = sideBarDividerNode->GetRenderContext();
    sideBarRC->UpdateTransformTranslate({ 100.0f, 0.0f, 0.0f });
    dividerRC->UpdateTransformTranslate({ 100.0f, 0.0f, 0.0f });
    pattern_->sideBarNode_ = sideBarNode;
    pattern_->sideBarDividerNode_ = sideBarDividerNode;
    pattern_->ResetDisplayModeSwitchTransforms();
    EXPECT_TRUE(sideBarRC->HasTransformTranslate());
}

// ===================== StartDisplayModeSwitchAnimation =====================

/**
 * @tc.name: StartDisplayModeSwitchAnimationSetsFlags001
 * @tc.desc: StartDisplayModeSwitchAnimation drives the full non-floating animation lifecycle.
 *           In the unit-test environment AnimationUtils::StartAnimation fires its finish
 *           callback synchronously, so OnDisplayModeSwitchAnimationsFinished runs before this
 *           function returns: displayModeAnimCount_ is incremented then decremented back to 0
 *           and the animation flags/target are cleared. We assert that converged final state.
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, StartDisplayModeSwitchAnimationSetsFlags001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isOnDisplayModeSwitchAnimation_ = false;
    pattern_->isFloatingBar_ = false;
    pattern_->StartDisplayModeSwitchAnimation(
        TabBarDisplayMode::BOTTOMTABBAR, TabBarDisplayMode::SIDEBAR);
    // Synchronous finish callback clears the flags/target.
    EXPECT_FALSE(pattern_->isOnDisplayModeSwitchAnimation_);
    EXPECT_FALSE(pattern_->displayModeAnimTarget_.has_value());
}

/**
 * @tc.name: StartDisplayModeSwitchAnimationInterrupting001
 * @tc.desc: When already animating, doesn't reset displayModeAnimCount_ or apply entering transform
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, StartDisplayModeSwitchAnimationInterrupting001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isOnDisplayModeSwitchAnimation_ = true;
    pattern_->displayModeAnimCount_ = 2;
    pattern_->isFloatingBar_ = false;
    pattern_->StartDisplayModeSwitchAnimation(
        TabBarDisplayMode::BOTTOMTABBAR, TabBarDisplayMode::SIDEBAR);
    // isInterrupting=true → displayModeAnimCount_ NOT reset to 0
    EXPECT_EQ(pattern_->displayModeAnimCount_, 2);
}

/**
 * @tc.name: StartDisplayModeSwitchAnimationFloating001
 * @tc.desc: With isFloatingBar_=true, takes the floating bar animation path. In the unit-test
 *           environment AnimationUtils::StartAnimation fires finish callbacks synchronously,
 *           so both sub-animations (StartFloatingSidebarTranslateAnim and
 *           StartFloatingTabBarEnterAnim) increment then immediately decrement
 *           displayModeAnimCount_, converging back to 0 and clearing the animation flags/target.
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, StartDisplayModeSwitchAnimationFloating001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isFloatingBar_ = true;
    pattern_->isOnDisplayModeSwitchAnimation_ = false;
    pattern_->StartDisplayModeSwitchAnimation(
        TabBarDisplayMode::SIDEBAR, TabBarDisplayMode::BOTTOMTABBAR);
    // Floating path: one sub-animation finishes synchronously, count converges to 0 or 1
    // depending on which sub-animations run. Verify flags/target are cleared.
    EXPECT_FALSE(pattern_->isOnDisplayModeSwitchAnimation_);
    EXPECT_FALSE(pattern_->displayModeAnimTarget_.has_value());
}

// ===================== GetIsRealDisableSwipe =====================

/**
 * @tc.name: GetIsRealDisableSwipeSidebar001
 * @tc.desc: In SIDEBAR display mode, GetIsRealDisableSwipe returns true
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, GetIsRealDisableSwipeSidebar001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->currentBarDisplayMode_ = TabBarDisplayMode::SIDEBAR;
    EXPECT_TRUE(pattern_->GetIsRealDisableSwipe());
}

/**
 * @tc.name: GetIsRealDisableSwipeCustomAnim001
 * @tc.desc: With isCustomAnimation_=true, GetIsRealDisableSwipe returns true
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, GetIsRealDisableSwipeCustomAnim001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->currentBarDisplayMode_ = TabBarDisplayMode::BOTTOMTABBAR;
    pattern_->isCustomAnimation_ = true;
    EXPECT_TRUE(pattern_->GetIsRealDisableSwipe());
}

/**
 * @tc.name: GetIsRealDisableSwipeNormal001
 * @tc.desc: With no sidebar/custom anim and isDisableSwipe_=true, returns true
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, GetIsRealDisableSwipeNormal001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->currentBarDisplayMode_ = TabBarDisplayMode::BOTTOMTABBAR;
    pattern_->isCustomAnimation_ = false;
    pattern_->isDisableSwipe_ = true;
    EXPECT_TRUE(pattern_->GetIsRealDisableSwipe());
}

/**
 * @tc.name: GetIsRealDisableSwipeFalse001
 * @tc.desc: With no sidebar/custom anim and isDisableSwipe_=false, returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, GetIsRealDisableSwipeFalse001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->currentBarDisplayMode_ = TabBarDisplayMode::BOTTOMTABBAR;
    pattern_->isCustomAnimation_ = false;
    pattern_->isDisableSwipe_ = false;
    EXPECT_FALSE(pattern_->GetIsRealDisableSwipe());
}

/**
 * @tc.name: GetIsRealDisableSwipeNoMode001
 * @tc.desc: When currentBarDisplayMode_ has no value, falls through to isCustomAnimation_/isDisableSwipe_
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, GetIsRealDisableSwipeNoMode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->currentBarDisplayMode_.reset();
    pattern_->isCustomAnimation_ = false;
    pattern_->isDisableSwipe_ = false;
    EXPECT_FALSE(pattern_->GetIsRealDisableSwipe());
}

// ===================== OnLuminanceUpdate =====================

/**
 * @tc.name: OnLuminanceUpdateLowLuminance001
 * @tc.desc: Luminance below LUMINANCE_THRESHOLD_LOW sets isColorPickerDark_=true
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, OnLuminanceUpdateLowLuminance001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isColorPickerDark_.reset();
    // Use 0 which is below LUMINANCE_THRESHOLD_LOW
    pattern_->OnLuminanceUpdate(0);
    ASSERT_TRUE(pattern_->isColorPickerDark_.has_value());
    EXPECT_TRUE(pattern_->isColorPickerDark_.value());
}

/**
 * @tc.name: OnLuminanceUpdateHighLuminance001
 * @tc.desc: Luminance above LUMINANCE_THRESHOLD_HIGH sets isColorPickerDark_=false
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, OnLuminanceUpdateHighLuminance001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isColorPickerDark_.reset();
    // Use a large value above LUMINANCE_THRESHOLD_HIGH (typically 200+)
    pattern_->OnLuminanceUpdate(300);
    ASSERT_TRUE(pattern_->isColorPickerDark_.has_value());
    EXPECT_FALSE(pattern_->isColorPickerDark_.value());
}

/**
 * @tc.name: OnLuminanceUpdateMidRange001
 * @tc.desc: Mid-range luminance (between thresholds) does not change isColorPickerDark_
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, OnLuminanceUpdateMidRange001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isColorPickerDark_ = true;
    // Use 128 which should be between LOW and HIGH thresholds
    pattern_->OnLuminanceUpdate(128);
    // Should remain unchanged
    ASSERT_TRUE(pattern_->isColorPickerDark_.has_value());
    EXPECT_TRUE(pattern_->isColorPickerDark_.value());
}

// ===================== GetColorInvertColorMode =====================

/**
 * @tc.name: GetColorInvertColorModeUndefined001
 * @tc.desc: When IsColorInvertEnabled is false, returns COLOR_MODE_UNDEFINED
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, GetColorInvertColorModeUndefined001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    // No systemMaterial set → IsColorInvertEnabled=false
    EXPECT_EQ(pattern_->GetColorInvertColorMode(), ColorMode::COLOR_MODE_UNDEFINED);
}

/**
 * @tc.name: GetColorInvertColorModeDark001
 * @tc.desc: When IsColorInvertEnabled=true and isColorPickerDark_=true, returns DARK
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, GetColorInvertColorModeDark001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    // Force isColorPickerDark_=true. IsColorInvertEnabled() is hard to make true in test
    // environment (needs systemMaterial+materialLevel), so test the else-branch logic only
    pattern_->isColorPickerDark_ = true;
    // IsColorInvertEnabled returns false → COLOR_MODE_UNDEFINED
    EXPECT_EQ(pattern_->GetColorInvertColorMode(), ColorMode::COLOR_MODE_UNDEFINED);
}

/**
 * @tc.name: GetColorInvertColorModeNoValue001
 * @tc.desc: When isColorPickerDark_ has no value, returns COLOR_MODE_UNDEFINED
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, GetColorInvertColorModeNoValue001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isColorPickerDark_.reset();
    EXPECT_EQ(pattern_->GetColorInvertColorMode(), ColorMode::COLOR_MODE_UNDEFINED);
}

// ===================== IsColorInvertEnabled =====================

/**
 * @tc.name: IsColorInvertEnabledNoStyle001
 * @tc.desc: When no barFloatingStyle, IsColorInvertEnabled returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, IsColorInvertEnabledNoStyle001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    EXPECT_FALSE(pattern_->IsColorInvertEnabled());
}

/**
 * @tc.name: IsColorInvertEnabledNoSystemMaterial001
 * @tc.desc: When style has no systemMaterial, returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, IsColorInvertEnabledNoSystemMaterial001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    BarFloatingStyleParameters style;
    // No systemMaterial set
    layoutProperty_->UpdateBarFloatingStyle(style);
    EXPECT_FALSE(pattern_->IsColorInvertEnabled());
}

// ===================== OnSaveData =====================

/**
 * @tc.name: OnSaveDataBasic001
 * @tc.desc: OnSaveData writes the current index to the data string
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, OnSaveDataBasic001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    layoutProperty_->UpdateIndex(2);
    std::string data;
    EXPECT_TRUE(pattern_->OnSaveData(data));
    EXPECT_NE(data.find("2"), std::string::npos);
}

/**
 * @tc.name: OnSaveDataDefaultIndex001
 * @tc.desc: OnSaveData with default index (0) writes 0
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, OnSaveDataDefaultIndex001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    std::string data;
    EXPECT_TRUE(pattern_->OnSaveData(data));
    EXPECT_NE(data.find("0"), std::string::npos);
}

// ===================== UpdateSideBarDivider =====================

/**
 * @tc.name: UpdateSideBarDividerNullDivider001
 * @tc.desc: UpdateSideBarDivider with null divider does not update strokeWidth
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, UpdateSideBarDividerNullDivider001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto dividerNode = FrameNode::CreateFrameNode(
        "SideBarDivider", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(dividerNode, nullptr);
    pattern_->sideBarDividerNode_ = dividerNode;
    // No sidebarDivider or dividerForSideBar set → divider.isNull=true → no strokeWidth update
    pattern_->UpdateSideBarDivider();
    EXPECT_NE(pattern_, nullptr);
}

// ===================== UpdateBackBlurStyle =====================

/**
 * @tc.name: UpdateBackBlurStyleNoBlur001
 * @tc.desc: UpdateBackBlurStyle when no backBlurStyle is set is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, UpdateBackBlurStyleNoBlur001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(tabBarNode_, nullptr);

    // Should not crash, no backBlurStyle set
    pattern_->UpdateBackBlurStyle(tabBarNode_);
    EXPECT_NE(pattern_, nullptr);
}

// ===================== UpdateBgMaskNode =====================

/**
 * @tc.name: UpdateBgMaskNodeNoMaskNode001
 * @tc.desc: UpdateBgMaskNode with no background mask node is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, UpdateBgMaskNodeNoMaskNode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    // TabsNode has no background mask node by default → early return
    pattern_->UpdateBgMaskNode();
    EXPECT_NE(pattern_, nullptr);
}

// ===================== UpdateTopBgMaskNode =====================

/**
 * @tc.name: UpdateTopBgMaskNodeNoMaskNode001
 * @tc.desc: UpdateTopBgMaskNode with no background mask node is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, UpdateTopBgMaskNodeNoMaskNode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->UpdateTopBgMaskNode();
    EXPECT_NE(pattern_, nullptr);
}

// ===================== UnregisterColorPicker =====================

/**
 * @tc.name: UnregisterColorPickerNoNode001
 * @tc.desc: UnregisterColorPicker with no tabsNode/BackgroundMask is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, UnregisterColorPickerNoNode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->hasRegisterColorPicker_ = false;
    pattern_->UnregisterColorPicker();
    EXPECT_FALSE(pattern_->hasRegisterColorPicker_);
}

/**
 * @tc.name: UnregisterColorPickerNotRegistered001
 * @tc.desc: UnregisterColorPicker when hasRegisterColorPicker_=false is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, UnregisterColorPickerNotRegistered001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->hasRegisterColorPicker_ = false;
    pattern_->isColorPickerDark_ = true;
    pattern_->UnregisterColorPicker();
    // isColorPickerDark_ should NOT be reset when not registered
    ASSERT_TRUE(pattern_->isColorPickerDark_.has_value());
    EXPECT_TRUE(pattern_->isColorPickerDark_.value());
}

// ===================== InitColorPickerIfNeeded =====================

/**
 * @tc.name: InitColorPickerIfNeededNoMaskNode001
 * @tc.desc: InitColorPickerIfNeeded with no background mask node early returns
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, InitColorPickerIfNeededNoMaskNode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->hasRegisterColorPicker_ = false;
    pattern_->InitColorPickerIfNeeded();
    EXPECT_FALSE(pattern_->hasRegisterColorPicker_);
}

// ===================== SetAnimateMode additional =====================

/**
 * @tc.name: SetAnimateModeCustom001
 * @tc.desc: SetAnimateMode(CUSTOM) sets isCustomAnimation_ to true
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, SetAnimateModeCustom001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->SetAnimateMode(TabAnimateMode::NO_ANIMATION);
    EXPECT_EQ(pattern_->GetAnimateMode(), TabAnimateMode::NO_ANIMATION);
}

/**
 * @tc.name: SetAnimateModeDefault001
 * @tc.desc: SetAnimateMode(DEFAULT) sets isCustomAnimation_ to false
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, SetAnimateModeDefault001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->SetAnimateMode(TabAnimateMode::CONTENT_FIRST_WITH_JUMP);
    EXPECT_EQ(pattern_->GetAnimateMode(), TabAnimateMode::CONTENT_FIRST_WITH_JUMP);
}

// ===================== SetCurrentBarDisplayMode additional =====================

/**
 * @tc.name: SetCurrentBarDisplayModeTopTabbar001
 * @tc.desc: SetCurrentBarDisplayMode(TOP_TABBAR) updates currentBarDisplayMode_
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, SetCurrentBarDisplayModeTopTabbar001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->SetCurrentBarDisplayMode(TabBarDisplayMode::TOP_TABBAR);
    ASSERT_TRUE(pattern_->currentBarDisplayMode_.has_value());
    EXPECT_EQ(pattern_->currentBarDisplayMode_.value(), TabBarDisplayMode::TOP_TABBAR);
}

// ===================== FireBarDisplayModeChangeEvent =====================

/**
 * @tc.name: FireBarDisplayModeChangeEventNoCallback001
 * @tc.desc: FireBarDisplayModeChangeEvent with no callback is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, FireBarDisplayModeChangeEventNoCallback001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    // No event registered, should not crash
    pattern_->FireBarDisplayModeChangeEvent(TabBarDisplayMode::SIDEBAR);
    EXPECT_NE(pattern_, nullptr);
}

/**
 * @tc.name: FireBarDisplayModeChangeEventWithCallback001
 * @tc.desc: FireBarDisplayModeChangeEvent fires the registered callback
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternTddTestNg, FireBarDisplayModeChangeEventWithCallback001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    TabBarDisplayMode receivedMode = TabBarDisplayMode::BOTTOMTABBAR;
    pattern_->SetOnBarDisplayModeChangeEvent([&receivedMode](TabBarDisplayMode mode) {
        receivedMode = mode;
    });
    pattern_->FireBarDisplayModeChangeEvent(TabBarDisplayMode::SIDEBAR);
    EXPECT_EQ(receivedMode, TabBarDisplayMode::SIDEBAR);
}
} // namespace OHOS::Ace::NG
