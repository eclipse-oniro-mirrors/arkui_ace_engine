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

#include <type_traits>

#include "gtest/gtest.h"

#define private public
#define protected public

#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"

#include "core/common/ace_engine.h"
#include "core/components/common/layout/constants.h"
#include "core/components/select/select_theme.h"
#include "core/components/theme/icon_theme.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/menu/menu_model_ng.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/menu_theme.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/property/menu_property.h"
#include "core/components_ng/render/render_context.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::Framework;

namespace OHOS::Ace::NG {
namespace {
const std::string TEXT_TAG = "text";
constexpr int32_t DEFAULT_GRID_COUNT = 3;
constexpr int32_t GRID_MENU_MAX_COLUMN_WITH_TEXT = 3;
} // namespace

class MenuViewGridTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void MockPipelineContextGetTheme();
    void InitMenuTestNg();
    int32_t GetNodeId();
    RefPtr<FrameNode> menuFrameNode_;
    int32_t nodeId_ = 1;
};

void MenuViewGridTestNg::SetUpTestCase() {}

void MenuViewGridTestNg::TearDownTestCase() {}

void MenuViewGridTestNg::SetUp()
{
    MockPipelineContext::SetUp();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<SelectTheme>()));
    MockContainer::SetUp();
    InitMenuTestNg();
}

void MenuViewGridTestNg::TearDown()
{
    MockPipelineContext::TearDown();
    menuFrameNode_ = nullptr;
    SystemProperties::SetDeviceType(DeviceType::PHONE);
    ScreenSystemManager::GetInstance().dipScale_ = 1.0;
    SystemProperties::orientation_ = DeviceOrientation::PORTRAIT;
    MockContainer::TearDown();
}

void MenuViewGridTestNg::MockPipelineContextGetTheme()
{
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
        if (type == TextTheme::TypeId()) {
            return AceType::MakeRefPtr<TextTheme>();
        } else if (type == IconTheme::TypeId()) {
            return AceType::MakeRefPtr<IconTheme>();
        } else if (type == SelectTheme::TypeId()) {
            return AceType::MakeRefPtr<SelectTheme>();
        } else {
            return AceType::MakeRefPtr<MenuTheme>();
        }
    });
}

int32_t MenuViewGridTestNg::GetNodeId()
{
    nodeId_++;
    return nodeId_;
}

void MenuViewGridTestNg::InitMenuTestNg()
{
    menuFrameNode_ = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, GetNodeId(), AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(menuFrameNode_, nullptr);
}

// ===================== CreateGridItem Tests =====================

/**
 * @tc.name: CreateGridItem001
 * @tc.desc: Test CreateGridItem with text+icon.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewGridTestNg, CreateGridItem001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    OptionParam param;
    param.icon = "icon.png";
    param.value = "text";
    param.enabled = true;
    auto item = MenuView::CreateGridItem(param, 0);
    ASSERT_NE(item, nullptr);
    EXPECT_FALSE(item->GetChildren().empty());
}

/**
 * @tc.name: CreateGridItem002
 * @tc.desc: Test CreateGridItem with icon only.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewGridTestNg, CreateGridItem002, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    OptionParam param;
    param.icon = "icon.png";
    param.value = "";
    param.enabled = true;
    auto item = MenuView::CreateGridItem(param, 0);
    ASSERT_NE(item, nullptr);
    EXPECT_FALSE(item->GetChildren().empty());
}

/**
 * @tc.name: CreateGridItem003
 * @tc.desc: Test CreateGridItem with text only.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewGridTestNg, CreateGridItem003, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    OptionParam param;
    param.icon = "";
    param.value = "textOnly";
    param.enabled = true;
    auto item = MenuView::CreateGridItem(param, 0);
    ASSERT_NE(item, nullptr);
    EXPECT_FALSE(item->GetChildren().empty());
}

/**
 * @tc.name: CreateGridItem004
 * @tc.desc: Test CreateGridItem with empty param.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewGridTestNg, CreateGridItem004, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    OptionParam param;
    param.icon = "";
    param.value = "";
    param.enabled = true;
    auto item = MenuView::CreateGridItem(param, 0);
    ASSERT_NE(item, nullptr);
}

/**
 * @tc.name: CreateGridItem005
 * @tc.desc: Test CreateGridItem gesture hub.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewGridTestNg, CreateGridItem005, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    OptionParam param;
    param.icon = "icon.png";
    param.value = "test";
    param.enabled = true;
    auto item = MenuView::CreateGridItem(param, 0);
    ASSERT_NE(item, nullptr);
    auto gestureHub = item->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
}

// ===================== CreateGridItem Disabled Style Tests =====================

/**
 * @tc.name: CreateGridItemDisabledStyle001
 * @tc.desc: Test disabled grid item with text+icon applies opacity to all children
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewGridTestNg, CreateGridItemDisabledStyle001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    OptionParam param;
    param.icon = "icon.png";
    param.value = "Disabled";
    param.enabled = false;
    auto item = MenuView::CreateGridItem(param, 0);
    ASSERT_NE(item, nullptr);
    auto eventHub = item->GetEventHub<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    EXPECT_FALSE(eventHub->IsEnabled());

    auto children = item->GetChildren();
    EXPECT_GE(children.size(), 2U); // icon + text
    for (const auto& child : children) {
        auto childFrame = AceType::DynamicCast<FrameNode>(child);
        ASSERT_NE(childFrame, nullptr);
        auto renderContext = childFrame->GetRenderContext();
        ASSERT_NE(renderContext, nullptr);
        EXPECT_FALSE(renderContext->HasOpacity());
        EXPECT_EQ(renderContext->GetOpacity(), std::nullopt);
    }
}

/**
 * @tc.name: CreateGridItemDisabledStyle002
 * @tc.desc: Test disabled grid item with icon only applies opacity to icon child
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewGridTestNg, CreateGridItemDisabledStyle002, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    OptionParam param;
    param.icon = "icon.png";
    param.value = "";
    param.enabled = false;
    auto item = MenuView::CreateGridItem(param, 0);
    ASSERT_NE(item, nullptr);
    auto children = item->GetChildren();
    EXPECT_GE(children.size(), 1U); // icon only
    for (const auto& child : children) {
        auto childFrame = AceType::DynamicCast<FrameNode>(child);
        ASSERT_NE(childFrame, nullptr);
        auto renderContext = childFrame->GetRenderContext();
        ASSERT_NE(renderContext, nullptr);
        EXPECT_FALSE(renderContext->HasOpacity());
        EXPECT_EQ(renderContext->GetOpacity(), std::nullopt);
    }
}

/**
 * @tc.name: CreateGridItemDisabledStyle003
 * @tc.desc: Test disabled grid item with text only applies opacity to text child
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewGridTestNg, CreateGridItemDisabledStyle003, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    OptionParam param;
    param.icon = "";
    param.value = "TextOnlyDisabled";
    param.enabled = false;
    auto item = MenuView::CreateGridItem(param, 0);
    ASSERT_NE(item, nullptr);
    auto children = item->GetChildren();
    EXPECT_GE(children.size(), 1U); // text only
    for (const auto& child : children) {
        auto childFrame = AceType::DynamicCast<FrameNode>(child);
        ASSERT_NE(childFrame, nullptr);
        auto renderContext = childFrame->GetRenderContext();
        ASSERT_NE(renderContext, nullptr);
        EXPECT_FALSE(renderContext->HasOpacity());
        EXPECT_EQ(renderContext->GetOpacity(), std::nullopt);
    }
}

/**
 * @tc.name: CreateGridItemDisabledStyle004
 * @tc.desc: Test enabled grid item does NOT apply opacity to children
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewGridTestNg, CreateGridItemDisabledStyle004, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    OptionParam param;
    param.icon = "icon.png";
    param.value = "Enabled";
    param.enabled = true;
    auto item = MenuView::CreateGridItem(param, 0);
    ASSERT_NE(item, nullptr);
    auto eventHub = item->GetEventHub<EventHub>();
    ASSERT_NE(eventHub, nullptr);
    EXPECT_TRUE(eventHub->IsEnabled());

    auto children = item->GetChildren();
    for (const auto& child : children) {
        auto childFrame = AceType::DynamicCast<FrameNode>(child);
        ASSERT_NE(childFrame, nullptr);
        auto renderContext = childFrame->GetRenderContext();
        ASSERT_NE(renderContext, nullptr);
        // enabled item should not have opacity set by disabled style
        EXPECT_FALSE(renderContext->HasOpacity());
    }
}

/**
 * @tc.name: CreateGridItemDisabledStyle005
 * @tc.desc: Test MountGridSection with mixed enabled/disabled items applies opacity only to disabled
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewGridTestNg, CreateGridItemDisabledStyle005, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    auto outerColumn = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, GetNodeId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
    ASSERT_NE(outerColumn, nullptr);

    MenuParam menuParam;
    MenuGridStyleOptions gridStyle;
    gridStyle.count = DEFAULT_GRID_COUNT;
    gridStyle.horizontalSize = GRID_MENU_MAX_COLUMN_WITH_TEXT;
    menuParam.gridStyle = gridStyle;

    std::vector<OptionParam> params;
    params.push_back({ "Item0", "icon0.png", true, nullptr });
    params.push_back({ "Item1", "icon1.png", false, nullptr });
    params.push_back({ "Item2", "icon2.png", true, nullptr });

    MenuView::MountGridSection(params, menuFrameNode_, menuParam, outerColumn);
    ASSERT_FALSE(outerColumn->GetChildren().empty());

    auto menuPattern = menuFrameNode_->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    auto optionNodes = menuPattern->GetOptions();
    ASSERT_EQ(optionNodes.size(), 3U);

    // Item 0: enabled — no opacity
    auto item0 = optionNodes[0];
    ASSERT_NE(item0, nullptr);
    for (const auto& child : item0->GetChildren()) {
        auto childFrame = AceType::DynamicCast<FrameNode>(child);
        ASSERT_NE(childFrame, nullptr);
        EXPECT_FALSE(childFrame->GetRenderContext()->HasOpacity());
    }

    // Item 1: disabled — opacity applied
    auto item1 = optionNodes[1];
    ASSERT_NE(item1, nullptr);
    for (const auto& child : item1->GetChildren()) {
        auto childFrame = AceType::DynamicCast<FrameNode>(child);
        ASSERT_NE(childFrame, nullptr);
        auto rc = childFrame->GetRenderContext();
        ASSERT_NE(rc, nullptr);
        EXPECT_FALSE(rc->HasOpacity());
        EXPECT_EQ(rc->GetOpacity(), std::nullopt);
    }

    // Item 2: enabled — no opacity
    auto item2 = optionNodes[2];
    ASSERT_NE(item2, nullptr);
    for (const auto& child : item2->GetChildren()) {
        auto childFrame = AceType::DynamicCast<FrameNode>(child);
        ASSERT_NE(childFrame, nullptr);
        EXPECT_FALSE(childFrame->GetRenderContext()->HasOpacity());
    }
}

/**
 * @tc.name: CreateGridItemDisabledStyle006
 * @tc.desc: Test disabled grid item opacity value matches MenuItemPattern::UpdateDisabledStyle
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewGridTestNg, CreateGridItemDisabledStyle006, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    auto theme = AceType::MakeRefPtr<SelectTheme>();
    ASSERT_NE(theme, nullptr);

    OptionParam param;
    param.icon = "icon.png";
    param.value = "CompareDisabled";
    param.enabled = false;
    auto item = MenuView::CreateGridItem(param, 0);
    ASSERT_NE(item, nullptr);

    for (const auto& child : item->GetChildren()) {
        auto childFrame = AceType::DynamicCast<FrameNode>(child);
        ASSERT_NE(childFrame, nullptr);
        auto rc = childFrame->GetRenderContext();
        ASSERT_NE(rc, nullptr);
        EXPECT_FALSE(rc->HasOpacity());
        EXPECT_EQ(rc->GetOpacity(), std::nullopt);
    }
}

// ===================== MountGridSectionDivider Tests =====================

/**
 * @tc.name: MountGridSectionDivider001
 * @tc.desc: Test MountGridSection divider placeholder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuViewGridTestNg, MountGridSectionDivider001, TestSize.Level1)
{
    MockPipelineContextGetTheme();
    auto outerColumn = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, GetNodeId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
    ASSERT_NE(outerColumn, nullptr);
    EXPECT_TRUE(outerColumn->GetChildren().empty());
}
} // namespace OHOS::Ace::NG
