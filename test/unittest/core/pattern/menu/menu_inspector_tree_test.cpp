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
#define private public
#define protected public
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/menu/menu_manager.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/select/select_pattern.h"
#undef private
#undef protected
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/base/inspector_tree_request_context.h"
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
        trigger_ = FrameNode::CreateFrameNode("Button", 110, AceType::MakeRefPtr<Pattern>());
        trigger_->SetActive(true);
        trigger_->GetOrCreateGestureEventHub()->BindMenu([](GestureEvent&) {});
        trigger_->MountToParent(root_);
        wrapper_ =
            FrameNode::CreateFrameNode("MenuWrapper", 111, AceType::MakeRefPtr<MenuWrapperPattern>(trigger_->GetId()));
        overlay_ = MockPipelineContext::GetCurrent()->GetOverlayManager();
        ASSERT_TRUE(overlay_->CheckMenuManager());
        manager_ = AceType::DynamicCast<MenuManager>(overlay_->menuManager_);
        ASSERT_NE(manager_, nullptr);
    }

    void ExpectExpanded(bool expected)
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
                ASSERT_TRUE(attrs->GetValue("expanded")->IsBool());
                EXPECT_EQ(attrs->GetBool("expanded"), expected);
            }
        }
    }

    RefPtr<FrameNode> root_;
    RefPtr<FrameNode> trigger_;
    RefPtr<FrameNode> wrapper_;
    RefPtr<OverlayManager> overlay_;
    RefPtr<MenuManager> manager_;
};

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
