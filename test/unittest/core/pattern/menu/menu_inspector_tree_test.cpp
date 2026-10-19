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
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/select/select_pattern.h"
#undef private
#undef protected
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/base/inspector_tree_request_context.h"
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

TEST_F(InspectorTreeTask11, SelectExpandedFollowsCompletedVisibility)
{
    auto pattern = AceType::MakeRefPtr<SelectPattern>();
    auto wrapperPattern = AceType::MakeRefPtr<MenuWrapperPattern>(100);
    auto wrapper = FrameNode::CreateFrameNode("MenuWrapper", 100, wrapperPattern);
    pattern->SetMenuNode(wrapper);
    const MenuStatus states[] = { MenuStatus::INIT, MenuStatus::ON_SHOW_ANIMATION, MenuStatus::ON_START_ANIMATION,
        MenuStatus::SHOW, MenuStatus::ON_HIDE_ANIMATION, MenuStatus::HIDE };
    ParamConfig interaction;
    interaction.interactionInfo = true;
    InspectorTreeRequestContext interactionContext(interaction, {}, {});
    auto sparseNode = JsonUtil::CreateSharedPtrJson();
    pattern->AddInspectorTreeInfo(sparseNode, interactionContext);
    EXPECT_TRUE(sparseNode->GetValue("selected")->IsBool());
    EXPECT_FALSE(sparseNode->GetBool("selected"));
    const bool expanded[] = { false, false, false, true, true, false };
    InspectorTreeRequestContext context({}, {}, {});
    for (size_t i = 0; i < std::size(states); ++i) {
        wrapperPattern->ForceSetMenuStatus(states[i]);
        auto attrs = JsonUtil::CreateSharedPtrJson();
        pattern->DumpInspectorTreeInfo(attrs, context);
        EXPECT_TRUE(attrs->GetValue("expanded")->IsBool());
        EXPECT_EQ(attrs->GetBool("expanded"), expanded[i]);
    }
}

TEST_F(InspectorTreeTask11, DisappearingSubmenuRemainsExpandedUntilRemoved)
{
    auto pattern = AceType::MakeRefPtr<MenuItemPattern>();
    pattern->GetSubBuilder() = []() {};
    auto parent = FrameNode::CreateFrameNode("test", 101, AceType::MakeRefPtr<Pattern>());
    auto submenu = FrameNode::CreateFrameNode("test", 102, AceType::MakeRefPtr<Pattern>());
    parent->AddChild(submenu);
    pattern->inspectorSubMenu_ = submenu;
    pattern->inspectorExpanded_ = true;
    InspectorTreeRequestContext context({}, {}, {});
    parent->AddDisappearingChild(submenu);
    ASSERT_FALSE(submenu->IsOnMainTree());
    auto hiding = JsonUtil::CreateSharedPtrJson();
    pattern->DumpInspectorTreeInfo(hiding, context);
    EXPECT_TRUE(hiding->GetBool("expanded"));
    parent->RemoveDisappearingChild(submenu);
    auto hidden = JsonUtil::CreateSharedPtrJson();
    pattern->DumpInspectorTreeInfo(hidden, context);
    EXPECT_FALSE(hidden->GetBool("expanded"));
}
TEST_F(InspectorTreeTask11, InspectorTreeTask12OnlyInlinePickersExposeExpanded)
{
    InspectorTreeRequestContext context({}, {}, {});
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
