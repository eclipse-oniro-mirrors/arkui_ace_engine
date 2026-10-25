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

#include <optional>

#include "gtest/gtest.h"
#define private public
#define protected public
#include "core/components/select/select_theme.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/menu/menu_manager.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/select/select_pattern.h"
#undef private
#undef protected
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/base/inspector_tree_request_context.h"
#include "core/components_ng/base/view_abstract_model_ng.h"
#include "core/components_ng/base/view_abstract_model_static.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/pattern/date_picker/datepicker_pattern.h"
#include "core/components_ng/pattern/text_picker/textpicker_pattern.h"
#include "core/components_ng/pattern/time_picker/timepicker_row_pattern.h"

namespace OHOS::Ace::NG {
class InspectorTreeTask11 : public testing::Test {
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

TEST_F(InspectorTreeTask11, SelectExpandedFollowsExistingMenuVisibility)
{
    auto pattern = AceType::MakeRefPtr<SelectPattern>();
    auto wrapperPattern = AceType::MakeRefPtr<MenuWrapperPattern>(100);
    auto wrapper = FrameNode::CreateFrameNode("MenuWrapper", 100, wrapperPattern);
    pattern->SetMenuNode(wrapper);
    const MenuStatus states[] = { MenuStatus::INIT, MenuStatus::ON_HOVER_SCALE, MenuStatus::ON_SHOW_ANIMATION,
        MenuStatus::ON_START_ANIMATION, MenuStatus::SHOW, MenuStatus::ON_HIDE_ANIMATION, MenuStatus::HIDE };
    ParamConfig interaction;
    interaction.interactionInfo = true;
    InspectorTreeRequestContext interactionContext(interaction, {});
    auto sparseNode = JsonUtil::CreateSharedPtrJson();
    pattern->AddInspectorTreeInfo(sparseNode, interactionContext);
    EXPECT_TRUE(sparseNode->GetValue("selected")->IsBool());
    EXPECT_FALSE(sparseNode->GetBool("selected"));
    const bool expanded[] = { false, true, true, true, true, false, false };
    InspectorTreeRequestContext context({}, {});
    for (size_t i = 0; i < std::size(states); ++i) {
        wrapperPattern->ForceSetMenuStatus(states[i]);
        auto attrs = JsonUtil::CreateSharedPtrJson();
        pattern->DumpInspectorTreeInfo(attrs, context);
        EXPECT_TRUE(attrs->GetValue("expanded")->IsBool());
        EXPECT_EQ(attrs->GetBool("expanded"), expanded[i]);
    }
}

class InspectorTreeTask11SelectState : public InspectorTreeTask11 {
protected:
    void SetUp() override
    {
        InspectorTreeTask11::SetUp();
        constexpr int32_t SELECT_NODE_ID = 116;
        pattern_ = AceType::MakeRefPtr<SelectPattern>();
        frame_ = FrameNode::CreateFrameNode("Select", SELECT_NODE_ID, pattern_);
        frame_->SetActive(true);
    }

    void ExpectSelected(const RefPtr<FrameNode>& frame, std::optional<bool> expected)
    {
        for (bool visible : { false, true }) {
            for (bool interaction : { false, true }) {
                for (bool accessibility : { false, true }) {
                    SCOPED_TRACE(testing::Message() << "visible=" << visible << ", interaction=" << interaction
                        << ", accessibility=" << accessibility);
                    ParamConfig config;
                    config.interactionInfo = interaction;
                    config.accessibilityInfo = accessibility;
                    InspectorTreeRequestContext context(config, {});
                    config.inspectorContext = &context;
                    auto json = JsonUtil::CreateSharedPtrJson();
                    frame->DumpSimplifyTreeWithParamConfig(0, json, visible, config);
                    const bool present = interaction && expected.has_value();
                    size_t selectedCount = 0;
                    for (auto field = json->GetChild(); field && field->IsValid(); field = field->GetNext()) {
                        if (field->GetKey() == "selected") {
                            ++selectedCount;
                        }
                    }
                    EXPECT_EQ(selectedCount, present ? 1u : 0u);
                    if (present) {
                        ASSERT_TRUE(json->GetValue("selected")->IsBool());
                        EXPECT_EQ(json->GetBool("selected"), expected.value());
                    }
                }
            }
        }
    }

    RefPtr<SelectPattern> pattern_;
    RefPtr<FrameNode> frame_;
};

TEST_F(InspectorTreeTask11SelectState, UserSelectedOverridesBothNativeStatesAndResets)
{
    for (int32_t index : { -1, 0 }) {
        pattern_->selected_ = index;
        ExpectSelected(frame_, index >= 0);
        for (bool selected : { false, true }) {
            ViewAbstractModelNG::SetAccessibilitySelected(AceType::RawPtr(frame_), selected, false);
            ExpectSelected(frame_, selected);
            EXPECT_EQ(pattern_->GetSelected(), index);
        }
        ViewAbstractModelNG::SetAccessibilitySelected(AceType::RawPtr(frame_), false, true);
        ExpectSelected(frame_, index >= 0);
    }
}

TEST_F(InspectorTreeTask11SelectState, CustomSelectedKeepsSystemPriority)
{
    auto property = frame_->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(property, nullptr);
    auto custom = AceType::MakeRefPtr<CustomAccessibilityProperty>();
    property->SetCustomAccessibilityProperty(custom);
    for (bool selected : { false, true }) {
        pattern_->selected_ = selected ? -1 : 0;
        ViewAbstractModelNG::SetAccessibilitySelected(AceType::RawPtr(frame_), !selected, false);
        custom->SetSelected(selected);
        ExpectSelected(frame_, selected);
    }
    pattern_->selected_ = 0;
    property->SetCustomAccessibilityProperty(nullptr);
    ExpectSelected(frame_, false);
    ViewAbstractModelNG::SetAccessibilitySelected(AceType::RawPtr(frame_), false, true);
    ExpectSelected(frame_, true);
}

TEST_F(InspectorTreeTask11SelectState, CustomStateDoesNotAddSelectedWithoutSelectionSemantics)
{
    auto frame = FrameNode::CreateFrameNode("Column", 117, AceType::MakeRefPtr<Pattern>());
    frame->SetActive(true);
    auto custom = AceType::MakeRefPtr<CustomAccessibilityProperty>();
    custom->SetSelected(true);
    frame->GetAccessibilityProperty<AccessibilityProperty>()->SetCustomAccessibilityProperty(custom);
    ExpectSelected(frame, std::nullopt);
}

TEST_F(InspectorTreeTask11, MenuItemExpandedUsesStateForItsExpandingMode)
{
    auto pattern = AceType::MakeRefPtr<MenuItemPattern>();
    pattern->GetSubBuilder() = []() {};
    struct Case {
        SubMenuExpandingMode mode;
        bool subMenuShowed;
        bool embeddedExpanded;
        bool expected;
    };
    const Case cases[] = {
        { SubMenuExpandingMode::SIDE, true, false, true },
        { SubMenuExpandingMode::SIDE, false, true, false },
        { SubMenuExpandingMode::STACK, true, false, true },
        { SubMenuExpandingMode::STACK, false, true, false },
        { SubMenuExpandingMode::EMBEDDED, false, true, true },
        { SubMenuExpandingMode::EMBEDDED, true, false, false },
    };
    for (bool interaction : { false, true }) {
        ParamConfig config;
        config.interactionInfo = interaction;
        InspectorTreeRequestContext context(config, {});
        for (const auto& testCase : cases) {
            SCOPED_TRACE(testing::Message() << "mode=" << static_cast<int>(testCase.mode)
                                            << ", interaction=" << interaction << ", expected=" << testCase.expected);
            pattern->expandingMode_ = testCase.mode;
            pattern->SetIsSubMenuShowed(testCase.subMenuShowed);
            pattern->isExpanded_ = testCase.embeddedExpanded;
            auto attrs = JsonUtil::CreateSharedPtrJson();
            pattern->DumpInspectorTreeInfo(attrs, context);
            ASSERT_TRUE(attrs->GetValue("expanded")->IsBool());
            EXPECT_EQ(attrs->GetBool("expanded"), testCase.expected);
        }
    }
}

TEST_F(InspectorTreeTask11, MenuItemWithoutSubmenuOmitsExpanded)
{
    auto pattern = AceType::MakeRefPtr<MenuItemPattern>();
    InspectorTreeRequestContext context({}, {});
    auto attrs = JsonUtil::CreateSharedPtrJson();
    pattern->DumpInspectorTreeInfo(attrs, context);
    EXPECT_FALSE(attrs->Contains("expanded"));
}

class InspectorTreeTask11BoundMenu : public InspectorTreeTask11 {
protected:
    void SetUp() override
    {
        InspectorTreeTask11::SetUp();
        root_ = MockPipelineContext::GetCurrent()->GetRootElement();
        constexpr int32_t TRIGGER_NODE_ID = 110;
        constexpr int32_t WRAPPER_NODE_ID = 111;
        trigger_ = FrameNode::CreateFrameNode("Button", TRIGGER_NODE_ID, AceType::MakeRefPtr<Pattern>());
        trigger_->SetActive(true);
        BindTriggerMenu();
        trigger_->MountToParent(root_);
        wrapper_ = FrameNode::CreateFrameNode(
            "MenuWrapper", WRAPPER_NODE_ID, AceType::MakeRefPtr<MenuWrapperPattern>(trigger_->GetId()));
        overlay_ = MockPipelineContext::GetCurrent()->GetOverlayManager();
        ASSERT_TRUE(overlay_->CheckMenuManager());
        manager_ = AceType::DynamicCast<MenuManager>(overlay_->menuManager_);
        ASSERT_NE(manager_, nullptr);
    }

    virtual void BindTriggerMenu()
    {
        ScopedViewStackProcessor stack;
        ViewStackProcessor::GetInstance()->Push(trigger_);
        MenuParam menuParam;
        menuParam.isShowInSubWindow = false;
        ViewAbstractModelNG model;
        model.BindMenu({}, []() {}, menuParam);
    }

    void ExpectExpanded(std::optional<bool> expected)
    {
        for (bool visible : { false, true }) {
            for (bool interaction : { false, true }) {
                SCOPED_TRACE(testing::Message() << "visible=" << visible << ", interaction=" << interaction);
                ParamConfig config;
                config.interactionInfo = interaction;
                InspectorTreeRequestContext context(config, {});
                config.inspectorContext = &context;
                auto json = JsonUtil::CreateSharedPtrJson();
                trigger_->DumpSimplifyTreeWithParamConfig(0, json, visible, config);
                auto attrs = json->GetObject("$attrs");
                if (!expected.has_value()) {
                    EXPECT_FALSE(attrs->Contains("expanded"));
                    continue;
                }
                ASSERT_TRUE(attrs->GetValue("expanded")->IsBool());
                EXPECT_EQ(attrs->GetBool("expanded"), expected.value());
            }
        }
    }

    RefPtr<FrameNode> root_;
    RefPtr<FrameNode> trigger_;
    RefPtr<FrameNode> wrapper_;
    RefPtr<OverlayManager> overlay_;
    RefPtr<MenuManager> manager_;
};

class InspectorTreeTask11ExplicitMenu : public InspectorTreeTask11BoundMenu, public testing::WithParamInterface<bool> {
protected:
    void BindTriggerMenu() override {}

    void BindExplicitMenu(bool isShow)
    {
        MenuParam menuParam;
        menuParam.setShow = true;
        menuParam.isShow = isShow;
        menuParam.isShowInSubWindow = false;
        BindOrdinaryMenu(menuParam);
        auto gesture = trigger_->GetEventHub<EventHub>()->GetGestureEventHub();
        EXPECT_TRUE(!gesture || (!gesture->showMenu_ && !gesture->bindMenuTouch_));
    }

    void BindOrdinaryMenu(const MenuParam& menuParam, bool hasContent = true)
    {
        ScopedViewStackProcessor stack;
        ViewStackProcessor::GetInstance()->Push(trigger_);
        std::function<void()> builder;
        if (hasContent) {
            builder = []() {};
        }
        if (GetParam()) {
            ViewAbstractModelStatic::BindMenu(AceType::RawPtr(trigger_), {}, std::move(builder), menuParam);
        } else {
            ViewAbstractModelNG model;
            model.BindMenu({}, std::move(builder), menuParam);
        }
    }

    void BindContextMenu()
    {
        ScopedViewStackProcessor stack;
        ViewStackProcessor::GetInstance()->Push(trigger_);
        MenuParam menuParam;
        menuParam.type = MenuType::CONTEXT_MENU;
        menuParam.setShow = true;
        menuParam.isShowInSubWindow = true;
        menuParam.contextMenuRegisterType = ContextMenuRegisterType::CUSTOM_TYPE;
        std::function<void()> builder = []() {};
        std::function<void()> preview;
        if (GetParam()) {
            ViewAbstractModelStatic::BindContextMenuStatic(
                trigger_, ResponseType::LONG_PRESS, std::move(builder), menuParam, std::move(preview));
        } else {
            ViewAbstractModelNG model;
            model.BindContextMenu(ResponseType::LONG_PRESS, builder, menuParam, preview);
        }
    }

    void SetExpandDisplay(bool expandDisplay)
    {
        auto theme = AceType::MakeRefPtr<SelectTheme>();
        theme->expandDisplay_ = expandDisplay;
        auto themes = AceType::MakeRefPtr<testing::NiceMock<MockThemeManager>>();
        EXPECT_CALL(*themes, GetTheme(testing::_)).WillRepeatedly(testing::Return(theme));
        MockPipelineContext::GetCurrent()->SetThemeManager(themes);
    }

    void ExpectSubwindowMenuOmitted()
    {
        ExpectExpanded(std::nullopt);
        auto childRoot = FrameNode::CreateFrameNode("Root", 115, AceType::MakeRefPtr<Pattern>());
        auto childOverlay = AceType::MakeRefPtr<OverlayManager>(childRoot);
        ASSERT_TRUE(childOverlay->CheckMenuManager());
        auto childManager = AceType::DynamicCast<MenuManager>(childOverlay->menuManager_);
        ASSERT_NE(childManager, nullptr);
        wrapper_->MountToParent(childRoot);
        childManager->menuMap_[trigger_->GetId()] = wrapper_;
        ASSERT_EQ(overlay_->GetMenuNode(trigger_->GetId()), nullptr);
        for (auto status : { MenuStatus::ON_SHOW_ANIMATION, MenuStatus::SHOW,
                 MenuStatus::ON_HIDE_ANIMATION, MenuStatus::HIDE }) {
            wrapper_->GetPattern<MenuWrapperPattern>()->ForceSetMenuStatus(status);
            ExpectExpanded(std::nullopt);
        }
        childOverlay->EraseMenuInfo(trigger_->GetId());
        ExpectExpanded(std::nullopt);
    }
};

TEST_P(InspectorTreeTask11ExplicitMenu, ExpandedFollowsMenuVisibilityWithoutMenuGestures)
{
    ExpectExpanded(std::nullopt);
    BindExplicitMenu(false);
    ASSERT_TRUE(trigger_->GetLayoutProperty()->GetIsBindOverlayValue(false));
    ExpectExpanded(false);

    wrapper_->MountToParent(root_);
    manager_->menuMap_[trigger_->GetId()] = wrapper_;
    auto pattern = wrapper_->GetPattern<MenuWrapperPattern>();
    pattern->ForceSetMenuStatus(MenuStatus::ON_SHOW_ANIMATION);
    BindExplicitMenu(true);
    ExpectExpanded(true);
    pattern->ForceSetMenuStatus(MenuStatus::SHOW);
    ExpectExpanded(true);

    // The actual menu state can change before the bound isShow value is updated.
    pattern->ForceSetMenuStatus(MenuStatus::ON_HIDE_ANIMATION);
    ExpectExpanded(false);
    BindExplicitMenu(false);
    pattern->ForceSetMenuStatus(MenuStatus::HIDE);
    overlay_->EraseMenuInfo(trigger_->GetId());
    ExpectExpanded(false);
}

TEST_P(InspectorTreeTask11ExplicitMenu, ExplicitSubwindowMenuOmitsExpanded)
{
    SetExpandDisplay(true);
    MenuParam menuParam;
    menuParam.setShow = true;
    menuParam.isShowInSubWindow = true;
    BindOrdinaryMenu(menuParam);
    ExpectSubwindowMenuOmitted();
}

TEST_P(InspectorTreeTask11ExplicitMenu, GestureSubwindowMenuOmitsExpanded)
{
    SetExpandDisplay(true);
    MenuParam menuParam;
    menuParam.isShowInSubWindow = true;
    BindOrdinaryMenu(menuParam);
    EXPECT_NE(trigger_->GetOrCreateGestureEventHub()->showMenu_, nullptr);
    ExpectSubwindowMenuOmitted();
}

TEST_P(InspectorTreeTask11ExplicitMenu, ContextMenuOmitsExpanded)
{
    SetExpandDisplay(true);
    BindContextMenu();
    ASSERT_TRUE(trigger_->GetLayoutProperty()->GetIsBindOverlayValue(false));
    ExpectSubwindowMenuOmitted();
}

TEST_P(InspectorTreeTask11ExplicitMenu, SubwindowPreferenceFallsBackToCurrentWindow)
{
    SetExpandDisplay(false);
    MenuParam menuParam;
    menuParam.setShow = true;
    menuParam.isShowInSubWindow = true;
    BindOrdinaryMenu(menuParam);
    ExpectExpanded(false);
    wrapper_->MountToParent(root_);
    manager_->menuMap_[trigger_->GetId()] = wrapper_;
    wrapper_->GetPattern<MenuWrapperPattern>()->ForceSetMenuStatus(MenuStatus::SHOW);
    ExpectExpanded(true);
    wrapper_->GetPattern<MenuWrapperPattern>()->ForceSetMenuStatus(MenuStatus::ON_HIDE_ANIMATION);
    ExpectExpanded(false);
}

TEST_P(InspectorTreeTask11ExplicitMenu, BindingScopeUpdatesWithoutContextMenuOverwritingOrdinaryMenu)
{
    SetExpandDisplay(true);
    MenuParam menuParam;
    menuParam.isShowInSubWindow = false;
    BindOrdinaryMenu(menuParam);
    BindContextMenu();
    ExpectExpanded(false);
    menuParam.isShowInSubWindow = true;
    BindOrdinaryMenu(menuParam);
    ExpectExpanded(std::nullopt);
    menuParam.isShowInSubWindow = false;
    BindOrdinaryMenu(menuParam);
    ExpectExpanded(false);
    BindOrdinaryMenu(menuParam, false);
    ExpectExpanded(std::nullopt);
}

INSTANTIATE_TEST_SUITE_P(DynamicAndStatic, InspectorTreeTask11ExplicitMenu, testing::Bool());

TEST_F(InspectorTreeTask11BoundMenu, ExpandedFollowsExistingMenuVisibility)
{
    ExpectExpanded(false);
    wrapper_->MountToParent(root_);
    manager_->menuMap_[trigger_->GetId()] = wrapper_;
    auto pattern = wrapper_->GetPattern<MenuWrapperPattern>();
    for (auto status : { MenuStatus::ON_HOVER_SCALE, MenuStatus::ON_SHOW_ANIMATION, MenuStatus::ON_START_ANIMATION,
             MenuStatus::SHOW }) {
        pattern->ForceSetMenuStatus(status);
        ExpectExpanded(true);
    }
    // HideMenu erases the lookup entry while the invisible wrapper runs its exit transition.
    pattern->ForceSetMenuStatus(MenuStatus::ON_HIDE_ANIMATION);
    ExpectExpanded(false);
    wrapper_->GetLayoutProperty()->UpdateVisibility(VisibleType::INVISIBLE, true);
    overlay_->EraseMenuInfo(trigger_->GetId());
    ASSERT_EQ(overlay_->GetMenuNode(trigger_->GetId()), nullptr);
    ExpectExpanded(false);
    EXPECT_EQ(pattern->GetMenuStatus(), MenuStatus::ON_HIDE_ANIMATION);
    EXPECT_EQ(wrapper_->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE), VisibleType::INVISIBLE);
    pattern->ForceSetMenuStatus(MenuStatus::HIDE);
    ExpectExpanded(false);
    // A retained, detached wrapper must not make the trigger look expanded.
    root_->RemoveChild(wrapper_);
    pattern->ForceSetMenuStatus(MenuStatus::ON_HIDE_ANIMATION);
    ExpectExpanded(false);
}

TEST_F(InspectorTreeTask11BoundMenu, UnregisteredClosingMenuRemainsCollapsed)
{
    auto other = FrameNode::CreateFrameNode("MenuWrapper", 112, AceType::MakeRefPtr<MenuWrapperPattern>(999));
    other->GetPattern<MenuWrapperPattern>()->ForceSetMenuStatus(MenuStatus::ON_HIDE_ANIMATION);
    other->MountToParent(root_);
    ExpectExpanded(false);
    wrapper_->MountToParent(root_);
    wrapper_->GetPattern<MenuWrapperPattern>()->ForceSetMenuStatus(MenuStatus::ON_HIDE_ANIMATION);
    ExpectExpanded(false);
    auto boundary = FrameNode::CreateFrameNode("WindowScene", 113, AceType::MakeRefPtr<Pattern>());
    boundary->SetWindowBoundary();
    boundary->MountToParent(root_);
    root_->RemoveChild(trigger_);
    trigger_->MountToParent(boundary);
    ExpectExpanded(false);
    root_->RemoveChild(wrapper_);
    wrapper_->MountToParent(boundary);
    ExpectExpanded(false);
}

TEST_F(InspectorTreeTask11BoundMenu, RegisteredOpeningMenuTakesPrecedenceOverPreviousClosingMenu)
{
    wrapper_->MountToParent(root_);
    wrapper_->GetPattern<MenuWrapperPattern>()->ForceSetMenuStatus(MenuStatus::ON_HIDE_ANIMATION);
    auto next =
        FrameNode::CreateFrameNode("MenuWrapper", 114, AceType::MakeRefPtr<MenuWrapperPattern>(trigger_->GetId()));
    next->GetPattern<MenuWrapperPattern>()->ForceSetMenuStatus(MenuStatus::ON_START_ANIMATION);
    next->MountToParent(root_);
    manager_->menuMap_[trigger_->GetId()] = next;
    ExpectExpanded(true);
}

TEST_F(InspectorTreeTask11, InspectorTreeTask12OnlyInlinePickersExposeExpanded)
{
    InspectorTreeRequestContext context({}, {});
    const auto verify = [&context](const auto& pattern) {
        pattern->SetIsShowInDialog(false);
        auto inlineAttrs = JsonUtil::CreateSharedPtrJson();
        pattern->DumpInspectorTreeInfo(inlineAttrs, context);
        EXPECT_TRUE(inlineAttrs->GetValue("expanded")->IsBool());
        EXPECT_TRUE(inlineAttrs->GetBool("expanded"));
        pattern->SetIsShowInDialog(true);
        auto dialogAttrs = JsonUtil::CreateSharedPtrJson();
        pattern->DumpInspectorTreeInfo(dialogAttrs, context);
        EXPECT_FALSE(dialogAttrs->Contains("expanded"));
    };
    verify(AceType::MakeRefPtr<TextPickerPattern>());
    verify(AceType::MakeRefPtr<DatePickerPattern>());
    verify(AceType::MakeRefPtr<TimePickerRowPattern>());
}
} // namespace OHOS::Ace::NG
