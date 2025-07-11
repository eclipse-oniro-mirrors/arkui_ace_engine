/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "test/mock/core/common/mock_container.h"
#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#include "test/mock/core/render/mock_render_context.h"
#include "test/mock/core/rosen/mock_canvas.h"
#include "test/mock/core/rosen/testing_canvas.h"

#include "core/components/common/layout/constants.h"
#include "core/components/common/layout/grid_system_manager.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/container_modal/container_modal_constants.h"
#include "core/components/select/select_theme.h"
#include "core/components/theme/shadow_theme.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_model_ng.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/menu/menu_item_group/menu_item_group_pattern.h"
#include "core/components_ng/pattern/menu/menu_item_group/menu_item_group_view.h"
#include "core/components_ng/pattern/menu/menu_model_ng.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/menu_theme.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/menu/multi_menu_layout_algorithm.h"
#include "core/components_ng/pattern/menu/preview/menu_preview_layout_algorithm.h"
#include "core/components_ng/pattern/menu/preview/menu_preview_pattern.h"
#include "core/components_ng/pattern/menu/sub_menu_layout_algorithm.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/root/root_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/property/border_property.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/syntax/lazy_for_each_model.h"
#include "core/components_ng/syntax/lazy_layout_wrapper_builder.h"
#include "core/event/touch_event.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::Framework;

namespace OHOS::Ace::NG {
namespace {
const InspectorFilter filter;
constexpr int32_t TARGET_ID = 3;
constexpr MenuType TYPE = MenuType::MENU;
const std::string EMPTY_TEXT = "";
const std::string TEXT_TAG = "text";
const std::string MENU_TAG = "menu";
const std::string MENU_ITEM_TEXT = "menuItem";
const std::string MENU_ITEM_GROUP_TEXT = "menuItemGroup";
const std::string MENU_TOUCH_EVENT_TYPE = "1";
const std::string IMAGE_SRC_URL = "file://data/data/com.example.test/res/example.svg";
constexpr float FULL_SCREEN_WIDTH = 720.0f;
constexpr float FULL_SCREEN_HEIGHT = 1136.0f;
constexpr float NEW_FULL_SCREEN_WIDTH = 360.0f;
constexpr float TARGET_SIZE_WIDTH = 100.0f;
constexpr float TARGET_SIZE_HEIGHT = 100.0f;
constexpr float MENU_ITEM_SIZE_WIDTH = 100.0f;
constexpr float MENU_ITEM_SIZE_HEIGHT = 50.0f;
const SizeF FULL_SCREEN_SIZE(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
constexpr double DIP_SCALE = 1.5;
const std::vector<std::string> FONT_FAMILY_VALUE = {"cursive"};

} // namespace
class MenuPatternTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void InitMenuTestNg();
    void InitMenuItemTestNg();
    PaintWrapper* GetPaintWrapper(RefPtr<MenuPaintProperty> paintProperty);
    RefPtr<FrameNode> GetPreviewMenuWrapper(
        SizeF itemSize = SizeF(0.0f, 0.0f), std::optional<MenuPreviewAnimationOptions> scaleOptions = std::nullopt);
    RefPtr<FrameNode> menuFrameNode_;
    RefPtr<MenuAccessibilityProperty> menuAccessibilityProperty_;
    RefPtr<FrameNode> menuItemFrameNode_;
    RefPtr<MenuItemPattern> menuItemPattern_;
    RefPtr<MenuItemAccessibilityProperty> menuItemAccessibilityProperty_;
};

void MenuPatternTestNg::SetUpTestCase()
{
    MockContainer::SetUp();
}

void MenuPatternTestNg::TearDownTestCase()
{
    MockContainer::TearDown();
}

void MenuPatternTestNg::SetUp()
{
    MockPipelineContext::SetUp();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<SelectTheme>()));
}

void MenuPatternTestNg::TearDown()
{
    MockPipelineContext::TearDown();
    menuFrameNode_ = nullptr;
    menuAccessibilityProperty_ = nullptr;
    menuItemFrameNode_ = nullptr;
    menuItemPattern_ = nullptr;
    menuItemAccessibilityProperty_ = nullptr;
    SystemProperties::SetDeviceType(DeviceType::PHONE);
    ScreenSystemManager::GetInstance().dipScale_ = 1.0;
    SystemProperties::orientation_ = DeviceOrientation::PORTRAIT;
}

void MenuPatternTestNg::InitMenuTestNg()
{
    menuFrameNode_ = FrameNode::GetOrCreateFrameNode(V2::MENU_TAG, ViewStackProcessor::GetInstance()->ClaimNodeId(),
        []() { return AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "", TYPE); });
    ASSERT_NE(menuFrameNode_, nullptr);

    menuAccessibilityProperty_ = menuFrameNode_->GetAccessibilityProperty<MenuAccessibilityProperty>();
    ASSERT_NE(menuAccessibilityProperty_, nullptr);
}

void MenuPatternTestNg::InitMenuItemTestNg()
{
    menuItemFrameNode_ = FrameNode::GetOrCreateFrameNode(V2::MENU_ITEM_ETS_TAG,
        ViewStackProcessor::GetInstance()->ClaimNodeId(), []() { return AceType::MakeRefPtr<MenuItemPattern>(); });
    ASSERT_NE(menuItemFrameNode_, nullptr);

    menuItemPattern_ = menuItemFrameNode_->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern_, nullptr);

    menuItemAccessibilityProperty_ = menuItemFrameNode_->GetAccessibilityProperty<MenuItemAccessibilityProperty>();
    ASSERT_NE(menuItemAccessibilityProperty_, nullptr);
}

PaintWrapper* MenuPatternTestNg::GetPaintWrapper(RefPtr<MenuPaintProperty> paintProperty)
{
    WeakPtr<RenderContext> renderContext;
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    PaintWrapper* paintWrapper = new PaintWrapper(renderContext, geometryNode, paintProperty);
    return paintWrapper;
}

RefPtr<FrameNode> MenuPatternTestNg::GetPreviewMenuWrapper(
    SizeF itemSize, std::optional<MenuPreviewAnimationOptions> scaleOptions)
{
    auto rootNode = FrameNode::CreateFrameNode(
        V2::ROOT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<RootPattern>());
    CHECK_NULL_RETURN(rootNode, nullptr);
    auto targetNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(targetNode, nullptr);
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(textNode, nullptr);
    if (!(LessOrEqual(itemSize.Width(), 0.0) || LessOrEqual(itemSize.Height(), 0.0))) {
        auto itemGeometryNode = textNode->GetGeometryNode();
        CHECK_NULL_RETURN(itemGeometryNode, nullptr);
        itemGeometryNode->SetFrameSize(itemSize);
    }
    targetNode->MountToParent(rootNode);
    MenuParam menuParam;
    menuParam.type = MenuType::CONTEXT_MENU;
    menuParam.previewMode = MenuPreviewMode::CUSTOM;
    if (scaleOptions != std::nullopt) {
        menuParam.previewAnimationOptions = scaleOptions.value();
    }
    auto customNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(customNode, nullptr);
    auto customGeometryNode = customNode->GetGeometryNode();
    CHECK_NULL_RETURN(customGeometryNode, nullptr);
    customGeometryNode->SetFrameSize(SizeF(TARGET_SIZE_WIDTH, TARGET_SIZE_HEIGHT));
    auto menuWrapperNode =
        MenuView::Create(textNode, targetNode->GetId(), V2::TEXT_ETS_TAG, menuParam, true, customNode);
    return menuWrapperNode;
}

/**
 * @tc.name: MenuPatternTest038
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg038, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    auto context = PipelineBase::GetCurrentContext();
    if (context) {
        context->dipScale_ = DIP_SCALE;
    }
    SystemProperties::orientation_ = DeviceOrientation::LANDSCAPE;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest039
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg039, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    auto context = PipelineBase::GetCurrentContext();
    if (context) {
        context->dipScale_ = DIP_SCALE;
    }
    SystemProperties::orientation_ = DeviceOrientation::PORTRAIT;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest040
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg040, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    auto context = PipelineBase::GetCurrentContext();
    if (context) {
        context->dipScale_ = DIP_SCALE;
    }
    SystemProperties::orientation_ = DeviceOrientation::ORIENTATION_UNDEFINED;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest041
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg041, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    SystemProperties::orientation_ = DeviceOrientation::LANDSCAPE;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest042
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg042, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    SystemProperties::orientation_ = DeviceOrientation::PORTRAIT;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest043
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg043, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    SystemProperties::orientation_ = DeviceOrientation::ORIENTATION_UNDEFINED;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest044
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg044, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().screenWidth_ = NEW_FULL_SCREEN_WIDTH;
    SystemProperties::orientation_ = DeviceOrientation::LANDSCAPE;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest045
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg045, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().screenWidth_ = NEW_FULL_SCREEN_WIDTH;
    SystemProperties::orientation_ = DeviceOrientation::PORTRAIT;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest046
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg046, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().screenWidth_ = NEW_FULL_SCREEN_WIDTH;
    SystemProperties::orientation_ = DeviceOrientation::ORIENTATION_UNDEFINED;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest047
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg047, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::SUB_MENU);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest048
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg048, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::MENU);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest049
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg049, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::CONTEXT_MENU);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest050
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg050, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::MULTI_MENU);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest051
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg051, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::DESKTOP_MENU);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest052
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg052, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::NAVIGATION_MENU);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest053
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg053, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::SELECT_OVERLAY_EXTENSION_MENU);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest054
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg054, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::SELECT_OVERLAY_CUSTOM_MENU);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest055
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg055, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::SELECT_OVERLAY_SUB_MENU);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest056
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg056, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::SELECT_OVERLAY_RIGHT_CLICK_MENU);

    menuPattern->FireBuilder();
    auto val = menuPattern->IsSelectMenu();

    EXPECT_EQ(val, false);
}

/**
 * @tc.name: MenuPatternTest057
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg057, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    auto context = PipelineBase::GetCurrentContext();
    if (context) {
        context->dipScale_ = DIP_SCALE;
    }
    SystemProperties::orientation_ = DeviceOrientation::LANDSCAPE;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest058
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg058, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    auto context = PipelineBase::GetCurrentContext();
    if (context) {
        context->dipScale_ = DIP_SCALE;
    }
    SystemProperties::orientation_ = DeviceOrientation::PORTRAIT;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest059
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg059, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    auto context = PipelineBase::GetCurrentContext();
    if (context) {
        context->dipScale_ = DIP_SCALE;
    }
    SystemProperties::orientation_ = DeviceOrientation::ORIENTATION_UNDEFINED;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest060
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg060, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    SystemProperties::orientation_ = DeviceOrientation::LANDSCAPE;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest061
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg061, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    SystemProperties::orientation_ = DeviceOrientation::PORTRAIT;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest062
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg062, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    SystemProperties::orientation_ = DeviceOrientation::ORIENTATION_UNDEFINED;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest063
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg063, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().screenWidth_ = NEW_FULL_SCREEN_WIDTH;
    SystemProperties::orientation_ = DeviceOrientation::LANDSCAPE;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest064
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg064, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().screenWidth_ = NEW_FULL_SCREEN_WIDTH;
    SystemProperties::orientation_ = DeviceOrientation::PORTRAIT;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest065
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg065, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().screenWidth_ = NEW_FULL_SCREEN_WIDTH;
    SystemProperties::orientation_ = DeviceOrientation::ORIENTATION_UNDEFINED;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest066
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg066, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    auto menuNode = AceType::DynamicCast<FrameNode>(GetPreviewMenuWrapper()->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::SUB_MENU);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest067
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg067, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    auto menuNode = AceType::DynamicCast<FrameNode>(GetPreviewMenuWrapper()->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::MENU);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest068
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg068, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    auto menuNode = AceType::DynamicCast<FrameNode>(GetPreviewMenuWrapper()->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::CONTEXT_MENU);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest069
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg069, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    auto menuNode = AceType::DynamicCast<FrameNode>(GetPreviewMenuWrapper()->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::MULTI_MENU);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest070
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg070, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    auto menuNode = AceType::DynamicCast<FrameNode>(GetPreviewMenuWrapper()->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::DESKTOP_MENU);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest071
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg071, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    auto menuNode = AceType::DynamicCast<FrameNode>(GetPreviewMenuWrapper()->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::NAVIGATION_MENU);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest072
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg072, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    auto menuNode = AceType::DynamicCast<FrameNode>(GetPreviewMenuWrapper()->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::SELECT_OVERLAY_EXTENSION_MENU);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest073
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg073, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    auto menuNode = AceType::DynamicCast<FrameNode>(GetPreviewMenuWrapper()->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::SELECT_OVERLAY_CUSTOM_MENU);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest074
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg074, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    auto menuNode = AceType::DynamicCast<FrameNode>(GetPreviewMenuWrapper()->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::SELECT_OVERLAY_SUB_MENU);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest075
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg075, TestSize.Level1)
{
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    // create parent menu item
    auto item = AceType::MakeRefPtr<FrameNode>("MenuItem", -1, itemPattern);
    auto itemGeometryNode = item->GetGeometryNode();
    ASSERT_TRUE(itemGeometryNode);
    auto menuNode = AceType::DynamicCast<FrameNode>(GetPreviewMenuWrapper()->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    item->GetGeometryNode()->SetFrameSize(SizeF(MENU_ITEM_SIZE_WIDTH, MENU_ITEM_SIZE_HEIGHT));
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(-1, "", MenuType::SELECT_OVERLAY_RIGHT_CLICK_MENU);

    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}

/**
 * @tc.name: MenuPatternTest076
 * @tc.desc: Test OnItemPressed.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg076, TestSize.Level1)
{
    auto parent = FrameNode::CreateFrameNode(
        V2::JS_FOR_EACH_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(parent, nullptr);
    auto firstNode = FrameNode::CreateFrameNode(
        V2::JS_FOR_EACH_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    ASSERT_NE(firstNode, nullptr);
    auto nextNode = FrameNode::CreateFrameNode(V2::JS_FOR_EACH_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    ASSERT_NE(nextNode, nullptr);
    auto menuItemGroup1 = FrameNode::CreateFrameNode(
        V2::MENU_ITEM_GROUP_ETS_TAG, -1, AceType::MakeRefPtr<MenuItemGroupPattern>());
    ASSERT_NE(menuItemGroup1, nullptr);
    auto menuItemGroup2 = FrameNode::CreateFrameNode(
        V2::MENU_ITEM_ETS_TAG, -1, AceType::MakeRefPtr<MenuItemPattern>());
    ASSERT_NE(menuItemGroup2, nullptr);
    auto menuItemGroup3 = FrameNode::CreateFrameNode(
        V2::MENU_ITEM_GROUP_ETS_TAG, -1, AceType::MakeRefPtr<MenuItemGroupPattern>());
    ASSERT_NE(menuItemGroup3, nullptr);
    firstNode->MountToParent(parent);
    nextNode->MountToParent(parent);
    menuItemGroup1->MountToParent(parent);
    menuItemGroup2->MountToParent(parent);
    menuItemGroup3->MountToParent(parent);
    auto menuPattern = parent->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    menuPattern->type_ = MenuType::CONTEXT_MENU;
    menuPattern->OnItemPressed(parent, 5, false);
    EXPECT_EQ(parent->GetTag(), V2::JS_FOR_EACH_ETS_TAG);
    menuPattern->OnItemPressed(parent, 1, false);
    EXPECT_EQ(parent->GetChildAtIndex(1)->GetTag(), V2::JS_FOR_EACH_ETS_TAG);
    menuPattern->OnItemPressed(parent, 3, false);
    EXPECT_EQ(parent->GetChildAtIndex(4)->GetTag(), V2::MENU_ITEM_GROUP_ETS_TAG);
    menuPattern->OnItemPressed(parent, 2, false, true);
    EXPECT_EQ(parent->GetChildAtIndex(3)->GetTag(), V2::MENU_ITEM_ETS_TAG);
    menuPattern->OnItemPressed(parent, 2, false);
    EXPECT_EQ(parent->GetChildAtIndex(3)->GetTag(), V2::MENU_ITEM_ETS_TAG);
}

/**
 * @tc.name: MenuPatternTest077
 * @tc.desc: Test GetSelectMenuWidth.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg077, TestSize.Level1)
{
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    menuPattern->SetIsWidthModifiedBySelect(true);
    menuLayoutProperty->propSelectMenuModifiedWidth_ = 0.1f;
    menuPattern->GetSelectMenuWidth();
    menuLayoutProperty->propSelectMenuModifiedWidth_ = 100.0f;
    auto with = menuPattern->GetSelectMenuWidth();
    EXPECT_EQ(with, 100.0f);
}

/**
 * @tc.name: MenuPatternTest078
 * @tc.desc: Test OnColorConfigurationUpdate.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg078, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    std::vector<SelectParam> selectParam = { { "content1", "icon1" }, { "content2", "" },
    { "", "icon3" }, { "", "" } };
    auto wrapperNode = MenuView::Create(selectParam, TARGET_ID, EMPTY_TEXT);
    ASSERT_NE(wrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(wrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);

    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    ASSERT_EQ(menuPattern->GetOptions().size(), 4);
    menuPattern->OnColorConfigurationUpdate();
    EXPECT_EQ(menuNode->needCallChildrenUpdate_, false);
    menuPattern->isDisableMenuBgColorByUser_ = true;
    menuPattern->OnColorConfigurationUpdate();
}

/**
 * @tc.name: MenuPatternTest079
 * @tc.desc: Test OnDirtyLayoutWrapperSwap.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg079, TestSize.Level1)
{
    const DirtySwapConfig configDirtySwap = { false, false, false, false, false, false };

    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<SelectTheme>()));

    int32_t setApiVersion = 11;
    int32_t rollbackApiVersion = MockContainer::Current()->GetApiTargetVersion();
    MockContainer::Current()->SetApiTargetVersion(setApiVersion);

    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto property = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(property, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    geometryNode->SetFrameSize(SizeF(30, 30));
    auto layoutWrapperNode =
        AceType::MakeRefPtr<LayoutWrapperNode>(menuNode, geometryNode, menuNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapperNode, nullptr);
    layoutWrapperNode->layoutProperty_ = property;
    layoutWrapperNode->SetLayoutAlgorithm(menuAlgorithmWrapper);
    layoutWrapperNode->skipMeasureContent_.emplace(false);
    EXPECT_EQ(layoutWrapperNode->SkipMeasureContent(), false);

    EXPECT_TRUE(menuPattern->OnDirtyLayoutWrapperSwap(layoutWrapperNode, configDirtySwap));

    BorderRadiusProperty radius;
    radius.SetRadius(Dimension(20));
    property->propBorderRadius_ = radius;
    EXPECT_TRUE(menuPattern->OnDirtyLayoutWrapperSwap(layoutWrapperNode, configDirtySwap));

    radius.SetRadius(Dimension(10));
    property->propBorderRadius_ = radius;
    EXPECT_TRUE(menuPattern->OnDirtyLayoutWrapperSwap(layoutWrapperNode, configDirtySwap));

    setApiVersion = 12;
    MockContainer::Current()->SetApiTargetVersion(setApiVersion);
    EXPECT_TRUE(menuPattern->OnDirtyLayoutWrapperSwap(layoutWrapperNode, configDirtySwap));

    radius.SetRadius(Dimension(20));
    property->propBorderRadius_ = radius;
    EXPECT_TRUE(menuPattern->OnDirtyLayoutWrapperSwap(layoutWrapperNode, configDirtySwap));

    layoutWrapperNode->GetGeometryNode()->SetFrameSize(SizeF(50, 30));
    EXPECT_TRUE(menuPattern->OnDirtyLayoutWrapperSwap(layoutWrapperNode, configDirtySwap));
    MockContainer::Current()->SetApiTargetVersion(rollbackApiVersion);
}

/**
 * @tc.name: MenuPatternTest081
 * @tc.desc: Test UpdateMenuItemChildren with OnModifyDone.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg081, TestSize.Level1)
{
    MenuModelNG MneuModelInstance;
    MneuModelInstance.Create();
    auto menuNode = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    auto firstNode = FrameNode::CreateFrameNode(
        V2::JS_FOR_EACH_ETS_TAG, 1, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(firstNode, nullptr);
    auto secondNode = FrameNode::CreateFrameNode(
        V2::JS_SYNTAX_ITEM_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(secondNode, nullptr);
    auto menuItem1 = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 3, AceType::MakeRefPtr<MenuItemPattern>());
    ASSERT_NE(menuItem1, nullptr);
    auto menuItemPattern = menuItem1->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    menuItemPattern->expandingMode_ = SubMenuExpandingMode::STACK;
    auto menuItem2 = FrameNode::CreateFrameNode("", 4, AceType::MakeRefPtr<MenuItemPattern>());
    ASSERT_NE(menuItem2, nullptr);

    firstNode->MountToParent(menuNode);
    secondNode->MountToParent(menuNode);
    menuItem1->MountToParent(menuNode);
    menuItem2->MountToParent(menuNode);
    menuPattern->OnModifyDone();
    EXPECT_EQ(menuNode->GetChildren().size(), 4);
}

/**
 * @tc.name: MenuPatternTest0410
 * @tc.desc: Test FireBuilder.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg0410, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    ScreenSystemManager::GetInstance().dipScale_ = DIP_SCALE;
    ScreenSystemManager::GetInstance().screenWidth_ = FULL_SCREEN_WIDTH;
    auto context = PipelineBase::GetCurrentContext();
    if (context) {
        context->dipScale_ = DIP_SCALE;
    }
    SystemProperties::orientation_ = DeviceOrientation::ORIENTATION_UNDEFINED;
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuAlgorithmWrapper = menuNode->GetLayoutAlgorithm();
    ASSERT_NE(menuAlgorithmWrapper, nullptr);
    auto menuAlgorithm = AceType::DynamicCast<MenuLayoutAlgorithm>(menuAlgorithmWrapper->GetLayoutAlgorithm());
    ASSERT_NE(menuAlgorithm, nullptr);
    auto menuGeometryNode = menuNode->GetGeometryNode();
    ASSERT_NE(menuGeometryNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    SelectProperties properties;
    properties.value = "Value 1";
    properties.icon = "Icon 1";
    properties.index = 1;
    menuPattern->selectProperties_.push_back(properties);
    menuPattern->makeFunc_ = [](const MenuItemConfiguration& menuItemConfiguration) -> RefPtr<FrameNode> {
        return FrameNode::CreateFrameNode(V2::SCROLL_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    };
    menuPattern->FireBuilder();
    auto val = menuNode->isRestoreInfoUsed_;

    EXPECT_NE(val, false);
}
/**
 * @tc.name: MenuPatternTest083
 * @tc.desc: Test MenuPattern::IsMenuScrollable.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg083, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create menu and srcoll node, get menu frameNode.
     * @tc.expected: call IsMenuScrollable and result is IsScrollable & GetScrollableDistance.
     */
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem1", "fakeIcon", nullptr);
    optionParams.emplace_back("MenuItem2", "", nullptr);
    MenuParam menuParam;
    auto menuWrapperNode = MenuView::Create(std::move(optionParams), TARGET_ID, "", TYPE, menuParam);
    ASSERT_NE(menuWrapperNode, nullptr);
    ASSERT_EQ(menuWrapperNode->GetChildren().size(), 1);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto scrollNode = FrameNode::CreateFrameNode(V2::SCROLL_ETS_TAG, 1, AceType::MakeRefPtr<ScrollPattern>());
    scrollNode->MountToParent(menuNode);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    auto ret = menuPattern->IsMenuScrollable();
    ASSERT_FALSE(ret);
    /**
     * @tc.steps: step2. Create menu and srcoll node, get menu frameNode.
     * @tc.expected: call IsMenuScrollable and result is false.
     */
    RefPtr<FrameNode> menuWrapperNodeEx =
        FrameNode::GetOrCreateFrameNode(V2::MENU_TAG, ViewStackProcessor::GetInstance()->ClaimNodeId(),
            []() { return AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "", TYPE); });
    ASSERT_NE(menuWrapperNodeEx, nullptr);
    auto textNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 1, AceType::MakeRefPtr<TextPattern>());
    textNode->MountToParent(menuWrapperNodeEx);
    auto menuPatternEx = menuWrapperNodeEx->GetPattern<MenuPattern>();
    ASSERT_NE(menuPatternEx, nullptr);
    ret = menuPatternEx->IsMenuScrollable();
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: MenuPatternTest084
 * @tc.desc: Test HideStackMenu
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg084, TestSize.Level1)
{
    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);

    menuPattern->HideStackMenu();
    EXPECT_TRUE(true);
}

// /**
//  * @tc.name: MenuPatternTest085
//  * @tc.desc: Test OnTouchEvent
//  * @tc.type: FUNC
//  */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg085, TestSize.Level1)
{
    MenuPattern* menuPattern = new MenuPattern(TARGET_ID, "", TYPE);
    std::string type = "1";
    TouchEventInfo info(type);
    menuPattern->needHideAfterTouch_ = false;
    menuPattern->OnTouchEvent(info);
    EXPECT_TRUE(true);
}

 /**
 * @tc.name: MenuPatternTest086
 * @tc.desc: Test DisableTabInMenu
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg086, TestSize.Level1)
{
    RefPtr<FrameNode> menuNode =
        FrameNode::GetOrCreateFrameNode(V2::MENU_TAG, ViewStackProcessor::GetInstance()->ClaimNodeId(),
            []() { return AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "", TYPE); });
    ASSERT_NE(menuNode, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    menuPattern->type_ = MenuType::DESKTOP_MENU;
    ASSERT_NE(menuPattern, nullptr);
    menuPattern->DisableTabInMenu();
    ASSERT_TRUE(menuPattern->IsDesktopMenu());
}

/**
 * @tc.name: MenuPatternTestNg088
 * @tc.desc: Verify MenuPattern::GetInnerMenuOffset
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTestNg088, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create menuitem childnode and isNeedRestoreNodeId if false;
     */
    RefPtr<FrameNode> menuNode =
        FrameNode::GetOrCreateFrameNode(V2::MENU_TAG, ViewStackProcessor::GetInstance()->ClaimNodeId(),
            []() { return AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "", TYPE); });
    auto child = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 1, AceType::MakeRefPtr<MenuItemPattern>());
    child->MountToParent(menuNode);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    menuPattern->type_ = MenuType::CONTEXT_MENU;
    auto menuItemPattern = child->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    menuItemPattern->SetClickMenuItemId(child->GetId());
    RefPtr<FrameNode> subMenuNode =
        FrameNode::GetOrCreateFrameNode(V2::MENU_TAG, ViewStackProcessor::GetInstance()->ClaimNodeId(),
            []() { return AceType::MakeRefPtr<MenuPattern>(2, "", TYPE); });
    ASSERT_NE(subMenuNode, nullptr);
    auto testInfo = menuPattern->GetInnerMenuOffset(child, subMenuNode, false);
    EXPECT_TRUE(testInfo.isFindTargetId);
    /**
     * @tc.steps: step1+. test GetInnerMenuOffset and isNeedRestoreNodeId if true;
     */
    testInfo = menuPattern->GetInnerMenuOffset(child, subMenuNode, true);
    EXPECT_TRUE(testInfo.isFindTargetId);
    /**
     * @tc.steps: step2. Create menuitemgroup node and isNeedRestoreNodeId if false;
     */
    auto menuitemgroupNode = FrameNode::CreateFrameNode(
        V2::MENU_ITEM_GROUP_ETS_TAG, 1, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    auto itemchildOne =
        FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 2, AceType::MakeRefPtr<MenuItemPattern>());
    auto itemchildTwo =
        FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 3, AceType::MakeRefPtr<MenuItemPattern>());
    itemchildOne->MountToParent(menuitemgroupNode);
    itemchildTwo->MountToParent(menuitemgroupNode);
    menuPattern = menuNode->GetPattern<MenuPattern>();
    testInfo = menuPattern->GetInnerMenuOffset(menuitemgroupNode, subMenuNode, false);
    EXPECT_FALSE(testInfo.isFindTargetId);
    /**
     * @tc.steps: step2. Create menuitemgroup node and isNeedRestoreNodeId if true;
     */
    testInfo = menuPattern->GetInnerMenuOffset(menuitemgroupNode, subMenuNode, true);
    EXPECT_EQ(testInfo.originOffset, OffsetF(0.0, 0.0));
    EXPECT_FALSE(testInfo.isFindTargetId);
}

/**
 * @tc.name: MenuPatternTest089
 * @tc.desc: Test GetSelectMenuWidthFromTheme.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTest089, TestSize.Level1)
{
    /**
     * @tc.steps: step0. Create mock theme manager
     */
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto selectTheme = AceType::MakeRefPtr<SelectTheme>();
    selectTheme->optionNormalWidth_ = 100.0_vp;
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(selectTheme));

    auto menuWrapperNode = GetPreviewMenuWrapper();
    ASSERT_NE(menuWrapperNode, nullptr);
    auto menuNode = AceType::DynamicCast<FrameNode>(menuWrapperNode->GetChildAtIndex(0));
    ASSERT_NE(menuNode, nullptr);
    auto menuLayoutProperty = menuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(menuLayoutProperty, nullptr);
    auto menuPattern = menuNode->GetPattern<MenuPattern>();
    ASSERT_NE(menuPattern, nullptr);
    auto width = menuPattern->GetSelectMenuWidthFromTheme();
    EXPECT_EQ(width, 108);
}

/**
 * @tc.name: MenuPatternTest090
 * @tc.desc: Test HandleNextPressed.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTest090, TestSize.Level1)
{
    auto parent = FrameNode::CreateFrameNode(
        V2::MENU_ITEM_GROUP_ETS_TAG, 1, AceType::MakeRefPtr<MenuPattern>(1, V2::JS_IF_ELSE_ETS_TAG, MenuType::MENU));
    auto childrenOne =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(2, "menu", MenuType::MENU));
    auto childrenTwo = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, TARGET_ID, AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "text", MenuType::MULTI_MENU));
    auto childrenThree = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, 4, AceType::MakeRefPtr<MenuPattern>(4, "menuItem", MenuType::SUB_MENU));
    parent->children_ = { childrenOne, childrenTwo, childrenThree };
    parent->tag_ = V2::JS_IF_ELSE_ETS_TAG;
    int32_t index = 1;
    bool press = true;
    bool hover = true;
    auto menuPattern = parent->GetPattern<MenuPattern>();
    menuPattern->HandleNextPressed(parent, index, press, hover);
    EXPECT_NE(parent->GetChildAtIndex(index + 1), nullptr);
    index = 2;
    childrenOne->tag_ = V2::JS_IF_ELSE_ETS_TAG;
    auto uiNode = AceType::DynamicCast<UINode>(childrenOne);
    uiNode->children_ = { childrenOne, childrenTwo, childrenThree };
    ASSERT_NE(uiNode, nullptr);
    parent->parent_ = uiNode;
    menuPattern->HandleNextPressed(parent, index, press, hover);
    EXPECT_NE(menuPattern->GetOutsideForEachMenuItem(parent, true), nullptr);
}

/**
 * @tc.name: MenuPatternTest091
 * @tc.desc: Test HandlePrevPressed.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuPatternTest091, TestSize.Level1)
{
    auto parent = FrameNode::CreateFrameNode(
        V2::MENU_ITEM_GROUP_ETS_TAG, 1, AceType::MakeRefPtr<MenuPattern>(1, V2::JS_IF_ELSE_ETS_TAG, MenuType::MENU));
    auto childrenOne =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(2, "menu", MenuType::MENU));
    auto childrenTwo = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, TARGET_ID, AceType::MakeRefPtr<MenuPattern>(TARGET_ID, "text", MenuType::MULTI_MENU));
    auto childrenThree = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, 4, AceType::MakeRefPtr<MenuPattern>(4, "menuItem", MenuType::SUB_MENU));
    parent->children_ = { childrenOne, childrenTwo, childrenThree };
    parent->tag_ = V2::MENU_ITEM_GROUP_ETS_TAG;
    int32_t index = -1;
    bool press = true;
    auto uiNode = AceType::DynamicCast<UINode>(childrenOne);
    uiNode->tag_ = V2::JS_IF_ELSE_ETS_TAG;
    ASSERT_NE(uiNode, nullptr);
    parent->parent_ = uiNode;
    auto menuPattern = parent->GetPattern<MenuPattern>();
    menuPattern->HandlePrevPressed(parent, index, press);
    EXPECT_EQ(parent->GetParent()->GetChildIndex(parent), -1);
    uiNode->children_ = { parent };
    parent->parent_ = uiNode;
    menuPattern->HandlePrevPressed(parent, index, press);
    EXPECT_EQ(menuPattern->GetOutsideForEachMenuItem(parent->GetParent(), false), nullptr);
    uiNode->children_ = { childrenTwo, parent };
    parent->parent_ = uiNode;
    menuPattern->HandlePrevPressed(parent, index, press);
    EXPECT_NE(menuPattern->GetOutsideForEachMenuItem(parent, false), nullptr);
}

/**
 * @tc.name: MenuLifecycleCallbackTest001
 * @tc.desc: Test MenuLifecycleCallback.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, MenuLifecycleCallbackTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1.Mock data.
     */
    bool onWillAppearFlag = false;
    auto onWillAppearEvent = [&onWillAppearFlag]() { onWillAppearFlag = true; };
    bool onDidAppearFlag = false;
    auto onDidAppearEvent = [&onDidAppearFlag]() { onDidAppearFlag = true; };
    bool onWillDisappearFlag = false;
    auto onWillDisappearEvent = [&onWillDisappearFlag]() { onWillDisappearFlag = true; };
    bool onDidDisappearFlag = false;
    auto onDidDisappearEvent = [&onDidDisappearFlag]() { onDidDisappearFlag = true; };

    std::function<void()> buildFunc;
    MenuParam menuParam;
    menuParam.isShow = true;
    menuParam.isShowInSubWindow = false;
    menuParam.onWillAppear = std::move(onWillAppearEvent);
    menuParam.onDidAppear = std::move(onDidAppearEvent);
    menuParam.onWillDisappear = std::move(onWillDisappearEvent);
    menuParam.onDidDisappear = std::move(onDidDisappearEvent);
    std::vector<OptionParam> optionParams;
    optionParams.emplace_back("MenuItem1", "fakeIcon", nullptr);
    optionParams.emplace_back("MenuItem2", "", nullptr);

    auto menuNode =
        MenuView::Create(std::move(optionParams), TARGET_ID, MENU_TAG, MenuType::MENU, menuParam);
    CHECK_NULL_VOID(menuNode);
    auto menuWrapperPattern = menuNode->GetPattern<MenuWrapperPattern>();
    CHECK_NULL_VOID(menuWrapperPattern);
    menuWrapperPattern->RegisterMenuCallback(menuNode, menuParam);
    /**
     * @tc.steps: step2. Call Callback.
     * @tc.expected: Check Callback.
     */
    menuWrapperPattern->CallMenuOnWillAppearCallback();
    menuWrapperPattern->CallMenuOnDidAppearCallback();
    menuWrapperPattern->CallMenuOnWillDisappearCallback();
    menuWrapperPattern->CallMenuOnDidDisappearCallback();

    EXPECT_EQ(onWillAppearFlag, true);
    EXPECT_EQ(onDidAppearFlag, true);
    EXPECT_EQ(onWillDisappearFlag, true);
    EXPECT_EQ(onDidDisappearFlag, true);
}

/**
 * @tc.name: RegisterAccessibilityChildActionNotify001
 * @tc.desc: Test callback function.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, RegisterAccessibilityChildActionNotify001, TestSize.Level1)
{
    auto wrapperNode =
        FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, 1, AceType::MakeRefPtr<MenuWrapperPattern>(1));
    ASSERT_NE(wrapperNode, nullptr);
    /**
     * @tc.steps: step1. create outter menu and set show in subwindow true
     */
    auto outterMenuNode =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(outterMenuNode, nullptr);
    auto outterMenuLayoutProps = outterMenuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(outterMenuLayoutProps, nullptr);
    outterMenuLayoutProps->UpdateShowInSubWindow(true);
    /**
     * @tc.steps: step2. create inner menu and set show in subwindow false
     */
    auto innerMenuNode =
        FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, 2, AceType::MakeRefPtr<MenuPattern>(1, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(innerMenuNode, nullptr);
    auto innerMenuLayoutProps = innerMenuNode->GetLayoutProperty<MenuLayoutProperty>();
    ASSERT_NE(innerMenuLayoutProps, nullptr);
    innerMenuLayoutProps->UpdateShowInSubWindow(false);
    innerMenuLayoutProps->UpdateExpandingMode(SubMenuExpandingMode::STACK);
    auto menuItemNode = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 4, AceType::MakeRefPtr<MenuItemPattern>());
    ASSERT_NE(menuItemNode, nullptr);
    menuItemNode->MountToParent(innerMenuNode);
    innerMenuNode->MountToParent(outterMenuNode);
    outterMenuNode->MountToParent(wrapperNode);
    auto menuItemPattern = menuItemNode->GetPattern<MenuItemPattern>();
    ASSERT_NE(menuItemPattern, nullptr);
    menuItemPattern->expandingMode_ = innerMenuLayoutProps->GetExpandingMode().value_or(SubMenuExpandingMode::SIDE);
    /**
     * @tc.steps: step3. call ShowSubMenu to create submenu
     * @tc.expected: expect subMenu's showInSubwindow param is true
     */
    std::function<void()> buildFun = []() {
        MenuModelNG MenuModelInstance;
        MenuModelInstance.Create();
    };
    menuItemPattern->SetSubBuilder(buildFun);
    menuItemPattern->ShowSubMenu(ShowSubMenuType::LONG_PRESS);
    auto menuAccessibilityProperty_ = innerMenuNode->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(menuAccessibilityProperty_, nullptr);
    auto callback = menuAccessibilityProperty_->GetNotifyChildActionFunc();
    ASSERT_NE(callback, nullptr);
    auto reuslt = callback(menuItemNode, NotifyChildActionType::ACTION_CLICK);
    EXPECT_EQ(reuslt, AccessibilityActionResult::ACTION_RISE);
}

/**
 * @tc.name: UpdateSelectOptionTextByIndex
 * @tc.desc: Test UpdateSelectOptionTextByIndex function.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, UpdateSelectOptionTextByIndex, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create parent and child frame nodes.
     * @tc.expected: step1. Parent and child nodes are not null.
     */
    auto parent = FrameNode::CreateFrameNode(
        V2::MENU_ITEM_GROUP_ETS_TAG, 1, AceType::MakeRefPtr<MenuPattern>(1, V2::JS_IF_ELSE_ETS_TAG, MenuType::MENU));
    ASSERT_NE(parent, nullptr);
    auto child = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 2, AceType::MakeRefPtr<MenuItemPattern>());
    ASSERT_NE(child, nullptr);
    auto parentPattern = parent->GetPattern<MenuPattern>();
    ASSERT_NE(parentPattern, nullptr);
    auto childPattern = child->GetPattern<MenuItemPattern>();
    ASSERT_NE(childPattern, nullptr);

    /**
     * @tc.steps: step2. Test UpdateSelectOptionTextByIndex with non-select menu.
     * @tc.expected: step2. No changes should occur.
     */
    parentPattern->isSelectMenu_ = false;
    std::string text = "Text";
    parentPattern->UpdateSelectOptionTextByIndex(2, text);

    /**
     * @tc.steps: step3. Test UpdateSelectOptionTextByIndex with select menu.
     * @tc.expected: step3. Text should be updated correctly.
     */
    parentPattern->isSelectMenu_ = true;
    parentPattern->AddOptionNode(child);
    auto textNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 3, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode, nullptr);
    childPattern->SetTextNode(textNode);
    auto textProp = textNode->GetLayoutProperty<TextLayoutProperty>();
    ASSERT_NE(textProp, nullptr);
    parentPattern->UpdateSelectOptionTextByIndex(2, text);
    auto content = textProp->GetContent();
    EXPECT_FALSE(content.has_value());
    parentPattern->UpdateSelectOptionTextByIndex(0, text);
    auto ret = childPattern->GetText();
    EXPECT_EQ(text, ret);
}

/**
 * @tc.name: UpdateSelectOptionIconByIndex
 * @tc.desc: Test UpdateSelectOptionIconByIndex function.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternTestNg, UpdateSelectOptionIconByIndex, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create parent and child frame nodes.
     * @tc.expected: step1. Parent and child nodes are not null.
     */
    auto parent = FrameNode::CreateFrameNode(
        V2::MENU_ITEM_GROUP_ETS_TAG, 1, AceType::MakeRefPtr<MenuPattern>(1, V2::JS_IF_ELSE_ETS_TAG, MenuType::MENU));
    ASSERT_NE(parent, nullptr);
    auto child = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, 2, AceType::MakeRefPtr<MenuItemPattern>());
    ASSERT_NE(child, nullptr);
    auto parentPattern = parent->GetPattern<MenuPattern>();
    ASSERT_NE(parentPattern, nullptr);
    auto childPattern = child->GetPattern<MenuItemPattern>();
    ASSERT_NE(childPattern, nullptr);

    /**
     * @tc.steps: step2. Test UpdateSelectOptionIconByIndex with non-select menu.
     * @tc.expected: step2. No changes should occur.
     */
    parentPattern->isSelectMenu_ = false;
    std::string icon = "TestChildIcon";
    parentPattern->UpdateSelectOptionIconByIndex(0, icon);

    /**
     * @tc.steps: step3. Test UpdateSelectOptionIconByIndex with select menu.
     * @tc.expected: step3. Icon should be updated correctly.
     */
    parentPattern->isSelectMenu_ = true;
    parentPattern->AddOptionNode(child);
    parentPattern->UpdateSelectOptionIconByIndex(2, icon);
    auto ret = childPattern->GetIcon();
    EXPECT_NE(icon, ret);
    parentPattern->UpdateSelectOptionIconByIndex(0, icon);
    ret = childPattern->GetIcon();
    EXPECT_EQ(icon, ret);
}
} // namespace OHOS::Ace::NG
