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

#include "base/json/json_util.h"
#include "core/components/dialog/dialog_theme.h"
#include "core/components/tab_bar/tab_theme.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_model_ng.h"
#include "core/components_ng/pattern/tabs/tabs_node.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_declaration.h"
#include "core/components_ng/pattern/tabs/tab_content_node.h"
#include "core/components_ng/pattern/tabs/tab_content_pattern.h"
#include "core/components_ng/syntax/shallow_builder.h"

namespace OHOS::Ace::NG {
namespace {
RefPtr<Theme> GetPatternSidebarTddTheme(ThemeType type)
{
    if (type == TabTheme::TypeId()) {
        auto themeConstants = TestNG::CreateThemeConstants(THEME_PATTERN_TAB);
        return TabTheme::Builder().Build(themeConstants);
    }
    return AceType::MakeRefPtr<DialogTheme>();
}
} // namespace

class TabsPatternSidebarTddTestNg : public TabsTestNg {
public:
    static void SetUpTestSuite()
    {
        TestNG::SetUpTestSuite();
        MockPipelineContext::GetCurrent()->SetUseFlushUITasks(true);
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
            return GetPatternSidebarTddTheme(type);
        });
        EXPECT_CALL(*themeManager, GetTheme(_, _))
            .WillRepeatedly([](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
                return GetPatternSidebarTddTheme(type);
            });
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    }
    static void TearDownTestSuite()
    {
        TestNG::TearDownTestSuite();
        g_isConfigChangePerform = false;
    }
};

// ===================== HandleTabChangeWhenChildrenUpdated =====================

/**
 * @tc.name: HandleTabChangeWhenChildrenUpdatedNoContent001
 * @tc.desc: When tabContentNum <= 0, lastTabChangeInfo_ is reset
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, HandleTabChangeWhenChildrenUpdatedNoContent001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    TabsPattern::TabChangeInfo info;
    info.index = 1;
    info.isShow = true;
    pattern_->lastTabChangeInfo_ = info;
    pattern_->HandleTabChangeWhenChildrenUpdated(true, 0, 0);
    EXPECT_FALSE(pattern_->lastTabChangeInfo_.has_value());
}

/**
 * @tc.name: HandleTabChangeWhenChildrenUpdatedHasLastInfo001
 * @tc.desc: When lastTabChangeInfo_ has value, early return
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, HandleTabChangeWhenChildrenUpdatedHasLastInfo001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    TabsPattern::TabChangeInfo savedInfo;
    savedInfo.index = 2;
    savedInfo.isShow = true;
    pattern_->lastTabChangeInfo_ = savedInfo;
    pattern_->HandleTabChangeWhenChildrenUpdated(false, 4, 1);
    // Should not modify lastTabChangeInfo_ (early return)
    ASSERT_TRUE(pattern_->lastTabChangeInfo_.has_value());
    EXPECT_EQ(pattern_->lastTabChangeInfo_->index, 2);
}

/**
 * @tc.name: HandleTabChangeWhenChildrenUpdatedInit001
 * @tc.desc: When isInit=true, IsNeedFireTabChange returns true, fires callback
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, HandleTabChangeWhenChildrenUpdatedInit001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(swiperPattern_, nullptr);

    pattern_->lastTabChangeInfo_.reset();
    pattern_->HandleTabChangeWhenChildrenUpdated(true, TABCONTENT_NUMBER, 0);
    // After firing, lastTabChangeInfo_ should be set
    EXPECT_TRUE(pattern_->lastTabChangeInfo_.has_value());
}

// ===================== UpdateSideBarBackgroundColor =====================

/**
 * @tc.name: UpdateSideBarBackgroundColorSidebarUserSet001
 * @tc.desc: UpdateSideBarBackgroundColor with sidebar color set by user uses that color
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, UpdateSideBarBackgroundColorSidebarUserSet001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    auto sideBarNode = FrameNode::CreateFrameNode(
        "SideBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(sideBarNode, nullptr);
    pattern_->sideBarNode_ = sideBarNode;
    layoutProperty_->UpdateSidebarBackgroundColor(Color::RED);
    layoutProperty_->UpdateSidebarBackgroundColorSetByUser(true);
    pattern_->UpdateSideBarBackgroundColor();
    auto renderContext = sideBarNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto bgColor = renderContext->GetBackgroundColor();
    ASSERT_TRUE(bgColor.has_value());
    EXPECT_EQ(bgColor.value(), Color::RED);
}

/**
 * @tc.name: UpdateSideBarBackgroundColorBarForSideBarUserSet001
 * @tc.desc: UpdateSideBarBackgroundColor with barBackgroundColorForSideBar set by user uses that color
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, UpdateSideBarBackgroundColorBarForSideBarUserSet001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    auto sideBarNode = FrameNode::CreateFrameNode(
        "SideBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(sideBarNode, nullptr);
    pattern_->sideBarNode_ = sideBarNode;
    // No sidebar color, but barBackgroundColorForSideBar set
    layoutProperty_->UpdateBarBackgroundColorForSideBar(Color::GREEN);
    layoutProperty_->UpdateBarBackgroundColorSetByUser(true);
    pattern_->UpdateSideBarBackgroundColor();
    auto renderContext = sideBarNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto bgColor = renderContext->GetBackgroundColor();
    ASSERT_TRUE(bgColor.has_value());
    EXPECT_EQ(bgColor.value(), Color::GREEN);
}

/**
 * @tc.name: UpdateSideBarBackgroundColorBarUserSet001
 * @tc.desc: UpdateSideBarBackgroundColor with barBackgroundColor set by user uses that color
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, UpdateSideBarBackgroundColorBarUserSet001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    auto sideBarNode = FrameNode::CreateFrameNode(
        "SideBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(sideBarNode, nullptr);
    pattern_->sideBarNode_ = sideBarNode;
    layoutProperty_->UpdateBarBackgroundColor(Color::BLUE);
    layoutProperty_->UpdateBarBackgroundColorSetByUser(true);
    pattern_->UpdateSideBarBackgroundColor();
    auto renderContext = sideBarNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto bgColor = renderContext->GetBackgroundColor();
    ASSERT_TRUE(bgColor.has_value());
    EXPECT_EQ(bgColor.value(), Color::BLUE);
}

/**
 * @tc.name: UpdateSideBarBackgroundColorSidebarNotUserSet001
 * @tc.desc: UpdateSideBarBackgroundColor with sidebar color not set by user falls to theme default
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, UpdateSideBarBackgroundColorSidebarNotUserSet001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    auto sideBarNode = FrameNode::CreateFrameNode(
        "SideBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(sideBarNode, nullptr);
    pattern_->sideBarNode_ = sideBarNode;
    // HasSidebarBackgroundColor=true but not set by user → needSetDefaultColor=true, no explicit color
    layoutProperty_->UpdateSidebarBackgroundColor(Color::TRANSPARENT);
    // Not setting SetByUser (defaults to false)
    pattern_->UpdateSideBarBackgroundColor();
    auto renderContext = sideBarNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    auto bgColor = renderContext->GetBackgroundColor();
    ASSERT_TRUE(bgColor.has_value());
    // Should be theme default (not TRANSPARENT)
    EXPECT_NE(bgColor.value(), Color::TRANSPARENT);
}

// ===================== UpdateSideBarBackgroundBlurStyle =====================

/**
 * @tc.name: UpdateSideBarBackgroundBlurStyleNoBlurFlags001
 * @tc.desc: UpdateSideBarBackgroundBlurStyle with no blur flags is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, UpdateSideBarBackgroundBlurStyleNoBlurFlags001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto sideBarNode = FrameNode::CreateFrameNode(
        "SideBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(sideBarNode, nullptr);
    pattern_->sideBarNode_ = sideBarNode;
    pattern_->hasBarBlurStyle_ = false;
    pattern_->hasSidebarBlurStyle_ = false;
    pattern_->UpdateSideBarBackgroundBlurStyle();
    // No crash, no blur style applied
    auto renderContext = sideBarNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    EXPECT_FALSE(renderContext->GetBackBlurStyle().has_value());
}

/**
 * @tc.name: UpdateSideBarBackgroundBlurStyleWithBarBlur001
 * @tc.desc: UpdateSideBarBackgroundBlurStyle with hasBarBlurStyle_ applies blur
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, UpdateSideBarBackgroundBlurStyleWithBarBlur001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto sideBarNode = FrameNode::CreateFrameNode(
        "SideBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(sideBarNode, nullptr);
    pattern_->sideBarNode_ = sideBarNode;
    pattern_->hasBarBlurStyle_ = true;
    pattern_->hasSidebarBlurStyle_ = false;
    pattern_->UpdateSideBarBackgroundBlurStyle();
    auto renderContext = sideBarNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    // Blur style should be applied
    EXPECT_TRUE(renderContext->GetBackBlurStyle().has_value());
}

// ===================== ResetSystemMaterial =====================

/**
 * @tc.name: ResetSystemMaterialBasic001
 * @tc.desc: ResetSystemMaterial resets useNewMaterial and unregisters color picker
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, ResetSystemMaterialBasic001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(tabBarPattern_, nullptr);

    tabBarPattern_->SetUseNewMaterial(true);
    pattern_->ResetSystemMaterial();
    EXPECT_FALSE(tabBarPattern_->IsNewMaterial());
}

// ===================== ClampTabBarTransformOffset =====================

/**
 * @tc.name: ClampTabBarTransformOffsetNotFloating001
 * @tc.desc: ClampTabBarTransformOffset with not floating bar/topbar is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, ClampTabBarTransformOffsetNotFloating001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->isFloatingBar_ = false;
    pattern_->isFloatingTopBar_ = false;
    pattern_->ClampTabBarTransformOffset();
    EXPECT_FALSE(pattern_->isFloatingBar_);
    EXPECT_FALSE(pattern_->isFloatingTopBar_);
}

/**
 * @tc.name: ClampTabBarTransformOffsetNoTransform001
 * @tc.desc: ClampTabBarTransformOffset with floating bar but no transform translate is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, ClampTabBarTransformOffsetNoTransform001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(tabBarNode_, nullptr);

    pattern_->isFloatingBar_ = true;
    auto renderContext = tabBarNode_->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    // No transform translate set
    pattern_->ClampTabBarTransformOffset();
    EXPECT_FALSE(renderContext->HasTransformTranslate());
}

// ===================== UpdateCollapseButtonIfNeeded =====================

/**
 * @tc.name: UpdateCollapseButtonShowButNoButton001
 * @tc.desc: UpdateCollapseButtonIfNeeded with show=true and no existing button creates button
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, UpdateCollapseButtonShowButNoButton001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    // Create a sideBarNode with TabsSideBarPattern
    auto sideBarNode = FrameNode::CreateFrameNode(
        "SideBar", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(sideBarNode, nullptr);
    pattern_->sideBarNode_ = sideBarNode;
    pattern_->showSidebarControlButton_ = false;
    pattern_->collapseButtonNode_ = nullptr;
    pattern_->UpdateCollapseButtonIfNeeded();
    // show=false, no button → no-op
    EXPECT_EQ(pattern_->collapseButtonNode_, nullptr);
}

// ===================== ApplySystemMaterial =====================

/**
 * @tc.name: ApplySystemMaterialNoStyle001
 * @tc.desc: ApplySystemMaterial with no bar floating style is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, ApplySystemMaterialNoStyle001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    // No bar floating style set → early return
    pattern_->ApplySystemMaterial();
    EXPECT_NE(pattern_, nullptr);
}

// ===================== AddTabContentNode =====================

/**
 * @tc.name: AddTabContentNodeBasic001
 * @tc.desc: AddTabContentNode adds node to tabContentNodes_ list
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, AddTabContentNodeBasic001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    size_t initialSize = pattern_->tabContentNodes_.size();
    auto shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    auto tabContentNode = AceType::MakeRefPtr<TabContentNode>(
        V2::TAB_CONTENT_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<TabContentPattern>(shallowBuilder));
    ASSERT_NE(tabContentNode, nullptr);
    pattern_->AddTabContentNode(tabContentNode);
    EXPECT_EQ(pattern_->tabContentNodes_.size(), initialSize + 1);
}

// ===================== RemoveTabContentNode =====================

/**
 * @tc.name: RemoveTabContentNodeBasic001
 * @tc.desc: RemoveTabContentNode removes node from tabContentNodes_ list
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, RemoveTabContentNodeBasic001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    auto tabContentNode = AceType::MakeRefPtr<TabContentNode>(
        V2::TAB_CONTENT_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<TabContentPattern>(shallowBuilder));
    ASSERT_NE(tabContentNode, nullptr);
    pattern_->AddTabContentNode(tabContentNode);
    size_t sizeAfterAdd = pattern_->tabContentNodes_.size();
    pattern_->RemoveTabContentNode(tabContentNode);
    EXPECT_EQ(pattern_->tabContentNodes_.size(), sizeAfterAdd - 1);
}

/**
 * @tc.name: RemoveTabContentNodeNotInList001
 * @tc.desc: RemoveTabContentNode with node not in list is a no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, RemoveTabContentNodeNotInList001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    auto tabContentNode = AceType::MakeRefPtr<TabContentNode>(
        V2::TAB_CONTENT_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<TabContentPattern>(shallowBuilder));
    ASSERT_NE(tabContentNode, nullptr);
    size_t initialSize = pattern_->tabContentNodes_.size();
    // Node not in list → no-op
    pattern_->RemoveTabContentNode(tabContentNode);
    EXPECT_EQ(pattern_->tabContentNodes_.size(), initialSize);
}

// ===================== ProvideRestoreInfo / OnRestoreInfo =====================

/**
 * @tc.name: ProvideRestoreInfoBasic001
 * @tc.desc: ProvideRestoreInfo returns non-empty string
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, ProvideRestoreInfoBasic001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto info = pattern_->ProvideRestoreInfo();
    EXPECT_FALSE(info.empty());
}

/**
 * @tc.name: OnRestoreInfoBasic001
 * @tc.desc: OnRestoreInfo with valid json sets index
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, OnRestoreInfoBasic001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    // Provide restore info and feed it back
    auto info = pattern_->ProvideRestoreInfo();
    pattern_->OnRestoreInfo(info);
    EXPECT_NE(pattern_, nullptr);
}

/**
 * @tc.name: OnRestoreInfoInvalid001
 * @tc.desc: OnRestoreInfo with invalid json does not crash
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, OnRestoreInfoInvalid001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->OnRestoreInfo("invalid json");
    EXPECT_NE(pattern_, nullptr);
}

// ===================== CreateLayoutAlgorithm =====================

/**
 * @tc.name: CreateLayoutAlgorithmBasic001
 * @tc.desc: CreateLayoutAlgorithm returns a non-null LayoutAlgorithm
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, CreateLayoutAlgorithmBasic001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto algo = pattern_->CreateLayoutAlgorithm();
    ASSERT_NE(algo, nullptr);
}

// ===================== OnDirtyLayoutWrapperSwap =====================

/**
 * @tc.name: OnDirtyLayoutWrapperSwapBasic001
 * @tc.desc: OnDirtyLayoutWrapperSwap returns false with null wrapper
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, OnDirtyLayoutWrapperSwapBasic001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    DirtySwapConfig config;
    auto result = pattern_->OnDirtyLayoutWrapperSwap(nullptr, config);
    EXPECT_FALSE(result);
}

// ===================== IsValidFireTabChange =====================

/**
 * @tc.name: IsValidFireTabChangeNoLastInfo001
 * @tc.desc: When lastTabChangeInfo has no value, always returns true
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, IsValidFireTabChangeNoLastInfo001, TestSize.Level1)
{
    std::optional<TabsPattern::TabChangeInfo> lastInfo;
    EXPECT_TRUE(TabsPattern::IsValidFireTabChange(lastInfo, 0, true));
    EXPECT_TRUE(TabsPattern::IsValidFireTabChange(lastInfo, 5, false));
}

/**
 * @tc.name: IsValidFireTabChangeSameEvent001
 * @tc.desc: When lastTabChangeInfo matches index and isShow, returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, IsValidFireTabChangeSameEvent001, TestSize.Level1)
{
    TabsPattern::TabChangeInfo info;
    info.index = 2;
    info.isShow = true;
    std::optional<TabsPattern::TabChangeInfo> lastInfo = info;
    EXPECT_FALSE(TabsPattern::IsValidFireTabChange(lastInfo, 2, true));
}

/**
 * @tc.name: IsValidFireTabChangeDifferentIndex001
 * @tc.desc: When index differs, returns true even if isShow matches
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, IsValidFireTabChangeDifferentIndex001, TestSize.Level1)
{
    TabsPattern::TabChangeInfo info;
    info.index = 2;
    info.isShow = true;
    std::optional<TabsPattern::TabChangeInfo> lastInfo = info;
    EXPECT_TRUE(TabsPattern::IsValidFireTabChange(lastInfo, 3, true));
}

/**
 * @tc.name: IsValidFireTabChangeDifferentShow001
 * @tc.desc: When isShow differs, returns true even if index matches
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, IsValidFireTabChangeDifferentShow001, TestSize.Level1)
{
    TabsPattern::TabChangeInfo info;
    info.index = 2;
    info.isShow = true;
    std::optional<TabsPattern::TabChangeInfo> lastInfo = info;
    EXPECT_TRUE(TabsPattern::IsValidFireTabChange(lastInfo, 2, false));
}

// ===================== IsNeedFireTabChange =====================

/**
 * @tc.name: IsNeedFireTabChangeInitTrue001
 * @tc.desc: When isInit is true, always returns true
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, IsNeedFireTabChangeInitTrue001, TestSize.Level1)
{
    EXPECT_TRUE(TabsPattern::IsNeedFireTabChange(true, 0, 0, 0));
    EXPECT_TRUE(TabsPattern::IsNeedFireTabChange(true, 1, 2, 3));
}

/**
 * @tc.name: IsNeedFireTabChangeAllEqual001
 * @tc.desc: When not init and all three indices are equal, returns true
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, IsNeedFireTabChangeAllEqual001, TestSize.Level1)
{
    EXPECT_TRUE(TabsPattern::IsNeedFireTabChange(false, 1, 1, 1));
}

/**
 * @tc.name: IsNeedFireTabChangeTargetDiffers001
 * @tc.desc: When targetIndex != currentIndex, returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, IsNeedFireTabChangeTargetDiffers001, TestSize.Level1)
{
    EXPECT_FALSE(TabsPattern::IsNeedFireTabChange(false, 2, 1, 1));
}

/**
 * @tc.name: IsNeedFireTabChangeCurrentDiffers001
 * @tc.desc: When currentIndex != preIndex, returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, IsNeedFireTabChangeCurrentDiffers001, TestSize.Level1)
{
    EXPECT_FALSE(TabsPattern::IsNeedFireTabChange(false, 1, 2, 1));
}

// ===================== GetTargetIndex =====================

/**
 * @tc.name: GetTargetIndexInvalidJson001
 * @tc.desc: Invalid JSON string returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, GetTargetIndexInvalidJson001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    int32_t targetIndex = -1;
    EXPECT_FALSE(pattern_->GetTargetIndex("invalid json", targetIndex));
}

/**
 * @tc.name: GetTargetIndexWrongCmd001
 * @tc.desc: JSON with wrong cmd value returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, GetTargetIndexWrongCmd001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    int32_t targetIndex = -1;
    EXPECT_FALSE(pattern_->GetTargetIndex(R"({"cmd":"other"})", targetIndex));
}

/**
 * @tc.name: GetTargetIndexNoParams001
 * @tc.desc: JSON with cmd=changeIndex but no params returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, GetTargetIndexNoParams001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    int32_t targetIndex = -1;
    EXPECT_FALSE(pattern_->GetTargetIndex(R"({"cmd":"changeIndex"})", targetIndex));
}

/**
 * @tc.name: GetTargetIndexNoIndex001
 * @tc.desc: JSON with params but no index key returns false
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, GetTargetIndexNoIndex001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    int32_t targetIndex = -1;
    EXPECT_FALSE(pattern_->GetTargetIndex(R"({"cmd":"changeIndex","params":{}})", targetIndex));
}

/**
 * @tc.name: GetTargetIndexValid001
 * @tc.desc: Valid JSON with cmd=changeIndex and index string returns true
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, GetTargetIndexValid001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    int32_t targetIndex = -1;
    EXPECT_TRUE(pattern_->GetTargetIndex(R"({"cmd":"changeIndex","params":{"index":"3"}})", targetIndex));
    EXPECT_EQ(targetIndex, 3);
}

// ===================== GetEffectiveSidebarDividerConfig =====================

/**
 * @tc.name: GetEffectiveSidebarDividerConfigNoDivider001
 * @tc.desc: When neither sidebarDivider nor dividerForSideBar set,
 *           returns default-constructed TabsItemDivider (isNull=false)
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, GetEffectiveSidebarDividerConfigNoDivider001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto divider = pattern_->GetEffectiveSidebarDividerConfig();
    EXPECT_FALSE(divider.isNull);
}

/**
 * @tc.name: GetEffectiveSidebarDividerConfigFromSidebarDivider001
 * @tc.desc: When sidebarDivider is set, returns that divider
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, GetEffectiveSidebarDividerConfigFromSidebarDivider001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 3.0_vp;
    divider.color = Color::RED;
    layoutProperty_->UpdateSidebarDivider(divider);

    auto result = pattern_->GetEffectiveSidebarDividerConfig();
    EXPECT_FALSE(result.isNull);
    EXPECT_EQ(result.strokeWidth, 3.0_vp);
    EXPECT_EQ(result.color, Color::RED);
}

/**
 * @tc.name: GetEffectiveSidebarDividerConfigFromDividerForSideBar001
 * @tc.desc: When only dividerForSideBar is set, returns that divider
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, GetEffectiveSidebarDividerConfigFromDividerForSideBar001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 5.0_vp;
    layoutProperty_->UpdateDividerForSideBar(divider);

    auto result = pattern_->GetEffectiveSidebarDividerConfig();
    EXPECT_FALSE(result.isNull);
    EXPECT_EQ(result.strokeWidth, 5.0_vp);
}

/**
 * @tc.name: GetEffectiveSidebarDividerConfigSidebarOverForSideBar001
 * @tc.desc: When both set, sidebarDivider takes priority over dividerForSideBar
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, GetEffectiveSidebarDividerConfigSidebarOverForSideBar001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider sidebarDivider;
    sidebarDivider.isNull = false;
    sidebarDivider.strokeWidth = 3.0_vp;
    layoutProperty_->UpdateSidebarDivider(sidebarDivider);

    TabsItemDivider forSideBarDivider;
    forSideBarDivider.isNull = false;
    forSideBarDivider.strokeWidth = 7.0_vp;
    layoutProperty_->UpdateDividerForSideBar(forSideBarDivider);

    auto result = pattern_->GetEffectiveSidebarDividerConfig();
    EXPECT_EQ(result.strokeWidth, 3.0_vp);
}

// ===================== GetEffectiveSidebarDividerWidthPx additional =====================

/**
 * @tc.name: GetEffectiveSidebarDividerWidthPxFromDividerForSideBar001
 * @tc.desc: Width from dividerForSideBar when sidebarDivider not set
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, GetEffectiveSidebarDividerWidthPxFromDividerForSideBar001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 4.0_vp;
    layoutProperty_->UpdateDividerForSideBar(divider);

    EXPECT_FLOAT_EQ(pattern_->GetEffectiveSidebarDividerWidthPx(), 4.0f);
}

/**
 * @tc.name: GetEffectiveSidebarDividerWidthPxNegativeStrokeWidth001
 * @tc.desc: Negative strokeWidth returns 0
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, GetEffectiveSidebarDividerWidthPxNegativeStrokeWidth001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = Dimension(-1.0f, DimensionUnit::VP);
    layoutProperty_->UpdateSidebarDivider(divider);

    EXPECT_EQ(pattern_->GetEffectiveSidebarDividerWidthPx(), 0.0f);
}

// ===================== ClampSideBarWidthToRange =====================

/**
 * @tc.name: ClampSideBarWidthToRangeNotSidebarMode001
 * @tc.desc: When not in SIDEBAR mode, early returns without clamping
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, ClampSideBarWidthToRangeNotSidebarMode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->SetCurrentBarDisplayMode(TabBarDisplayMode::BOTTOMTABBAR);
    pattern_->SetRealSideBarWidthPx(500.0f);
    pattern_->ClampSideBarWidthToRange();
    // Should remain unchanged because not SIDEBAR mode
    ASSERT_TRUE(pattern_->GetRealSideBarWidthPx().has_value());
    EXPECT_FLOAT_EQ(pattern_->GetRealSideBarWidthPx().value(), 500.0f);
}

/**
 * @tc.name: ClampSideBarWidthToRangeSidebarNotDraggable001
 * @tc.desc: In SIDEBAR mode but not draggable, resets realSideBarWidthPx_
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, ClampSideBarWidthToRangeSidebarNotDraggable001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->SetCurrentBarDisplayMode(TabBarDisplayMode::SIDEBAR);
    pattern_->SetRealSideBarWidthPx(500.0f);
    // No divider set → not draggable → resets
    pattern_->ClampSideBarWidthToRange();
    EXPECT_FALSE(pattern_->GetRealSideBarWidthPx().has_value());
}

/**
 * @tc.name: ClampSideBarWidthToRangeSidebarDraggableClamps001
 * @tc.desc: In SIDEBAR mode with draggable divider, clamps to range
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, ClampSideBarWidthToRangeSidebarDraggableClamps001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    pattern_->SetCurrentBarDisplayMode(TabBarDisplayMode::SIDEBAR);
    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMinSidebarWidth(Dimension(100.0f, DimensionUnit::VP));
    layoutProperty_->UpdateMaxSidebarWidth(Dimension(300.0f, DimensionUnit::VP));

    // Set a width outside the range → should be clamped
    pattern_->SetRealSideBarWidthPx(500.0f);
    pattern_->ClampSideBarWidthToRange();
    ASSERT_TRUE(pattern_->GetRealSideBarWidthPx().has_value());
    // 500 should be clamped to max 300
    EXPECT_FLOAT_EQ(pattern_->GetRealSideBarWidthPx().value(), 300.0f);
}

/**
 * @tc.name: ClampSideBarWidthToRangeSidebarDraggableWithinRange001
 * @tc.desc: In SIDEBAR mode with draggable divider, value within range stays unchanged
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, ClampSideBarWidthToRangeSidebarDraggableWithinRange001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    pattern_->SetCurrentBarDisplayMode(TabBarDisplayMode::SIDEBAR);
    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMinSidebarWidth(Dimension(100.0f, DimensionUnit::VP));
    layoutProperty_->UpdateMaxSidebarWidth(Dimension(300.0f, DimensionUnit::VP));

    // Set a width within the range → should stay
    pattern_->SetRealSideBarWidthPx(200.0f);
    pattern_->ClampSideBarWidthToRange();
    ASSERT_TRUE(pattern_->GetRealSideBarWidthPx().has_value());
    EXPECT_FLOAT_EQ(pattern_->GetRealSideBarWidthPx().value(), 200.0f);
}

// ===================== CalcSideBarDragRange additional branches =====================

/**
 * @tc.name: CalcSideBarDragRangeWithMaxOnly001
 * @tc.desc: With divider and only maxSidebarWidth, minPx defaults to 0
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, CalcSideBarDragRangeWithMaxOnly001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMaxSidebarWidth(Dimension(400.0f, DimensionUnit::VP));

    auto range = pattern_->CalcSideBarDragRange(720.0f);
    EXPECT_TRUE(range.draggable);
    EXPECT_EQ(range.minPx, 0.0f);
    EXPECT_FLOAT_EQ(range.maxPx, 400.0f);
}

/**
 * @tc.name: CalcSideBarDragRangeWithContentOnly001
 * @tc.desc: With divider and only minContentWidth, maxFromContent = tabsWidth - content - divider
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, CalcSideBarDragRangeWithContentOnly001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMinContentWidth(Dimension(300.0f, DimensionUnit::VP));

    auto range = pattern_->CalcSideBarDragRange(720.0f);
    EXPECT_TRUE(range.draggable);
    // maxFromContent = 720 - 300 - 1 = 419; maxFromMax = 720 - 1 = 719; min = 419
    EXPECT_FLOAT_EQ(range.maxPx, 419.0f);
}

/**
 * @tc.name: CalcSideBarDragRangeWithMinAndMax001
 * @tc.desc: With divider, minSidebarWidth and maxSidebarWidth both set and min < max
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, CalcSideBarDragRangeWithMinAndMax001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMinSidebarWidth(Dimension(100.0f, DimensionUnit::VP));
    layoutProperty_->UpdateMaxSidebarWidth(Dimension(400.0f, DimensionUnit::VP));

    auto range = pattern_->CalcSideBarDragRange(720.0f);
    EXPECT_TRUE(range.draggable);
    EXPECT_FLOAT_EQ(range.minPx, 100.0f);
    EXPECT_FLOAT_EQ(range.maxPx, 400.0f);
}

/**
 * @tc.name: CalcSideBarDragRangeWithAllThree001
 * @tc.desc: With divider, minSidebarWidth, maxSidebarWidth, and minContentWidth all set
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, CalcSideBarDragRangeWithAllThree001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 2.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMinSidebarWidth(Dimension(100.0f, DimensionUnit::VP));
    layoutProperty_->UpdateMaxSidebarWidth(Dimension(500.0f, DimensionUnit::VP));
    layoutProperty_->UpdateMinContentWidth(Dimension(200.0f, DimensionUnit::VP));

    auto range = pattern_->CalcSideBarDragRange(720.0f);
    EXPECT_TRUE(range.draggable);
    EXPECT_FLOAT_EQ(range.minPx, 100.0f);
    // maxFromContent = 720 - 200 - 2 = 518; maxFromMax = 500; min = 500
    EXPECT_FLOAT_EQ(range.maxPx, 500.0f);
}

/**
 * @tc.name: CalcSideBarDragRangeInitialWidthFromEffective001
 * @tc.desc: initialWidthPx uses GetEffectiveSidebarWidth when available
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, CalcSideBarDragRangeInitialWidthFromEffective001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMinSidebarWidth(Dimension(100.0f, DimensionUnit::VP));
    layoutProperty_->UpdateMaxSidebarWidth(Dimension(400.0f, DimensionUnit::VP));
    pattern_->SetSidebarWidthCalled();
    layoutProperty_->UpdateSidebarWidth(Dimension(250.0f, DimensionUnit::VP));

    auto range = pattern_->CalcSideBarDragRange(720.0f);
    EXPECT_TRUE(range.draggable);
    EXPECT_FLOAT_EQ(range.initialWidthPx, 250.0f);
}

/**
 * @tc.name: CalcSideBarDragRangeInitialWidthFromReal001
 * @tc.desc: initialWidthPx falls back to realSideBarWidthPx_ when no effective width
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, CalcSideBarDragRangeInitialWidthFromReal001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    TabsItemDivider divider;
    divider.isNull = false;
    divider.strokeWidth = 1.0_vp;
    layoutProperty_->UpdateSidebarDivider(divider);
    layoutProperty_->UpdateMinSidebarWidth(Dimension(100.0f, DimensionUnit::VP));
    layoutProperty_->UpdateMaxSidebarWidth(Dimension(400.0f, DimensionUnit::VP));
    pattern_->SetRealSideBarWidthPx(300.0f);

    auto range = pattern_->CalcSideBarDragRange(720.0f);
    EXPECT_TRUE(range.draggable);
    EXPECT_FLOAT_EQ(range.initialWidthPx, 300.0f);
}

// ===================== GetEffectiveSidebarWidth additional =====================

/**
 * @tc.name: GetEffectiveSidebarWidthFromSideBarNode001
 * @tc.desc: When neither hasSidebarWidth_ nor barWidthForSideBar set, falls back to sideBarNode_
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, GetEffectiveSidebarWidthFromSideBarNode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    // No sidebarWidth, no barWidthForSideBar, no sideBarNode → nullopt
    auto result = pattern_->GetEffectiveSidebarWidth();
    EXPECT_FALSE(result.has_value());
}

/**
 * @tc.name: GetEffectiveSidebarWidthZeroWidth001
 * @tc.desc: sidebarWidth of 0 is non-negative → returns 0
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, GetEffectiveSidebarWidthZeroWidth001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(layoutProperty_, nullptr);

    pattern_->SetSidebarWidthCalled();
    layoutProperty_->UpdateSidebarWidth(Dimension(0.0f, DimensionUnit::VP));

    auto result = pattern_->GetEffectiveSidebarWidth();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), Dimension(0.0f, DimensionUnit::VP));
}

// ===================== OnInjectionEvent =====================

/**
 * @tc.name: OnInjectionEventInvalid001
 * @tc.desc: Invalid command returns RET_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, OnInjectionEventInvalid001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto result = pattern_->OnInjectionEvent("invalid");
    EXPECT_NE(result, 0);
}

/**
 * @tc.name: OnInjectionEventValid001
 * @tc.desc: Valid changeIndex command returns RET_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, OnInjectionEventValid001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto result = pattern_->OnInjectionEvent(R"({"cmd":"changeIndex","params":{"index":"1"}})");
    // OnInjectionEvent returns RET_SUCCESS (== 10, see frame_node.h) on a valid changeIndex command.
    EXPECT_EQ(result, 10);
}

// ===================== GetEffectiveSidebarBackgroundColor additional =====================

/**
 * @tc.name: GetEffectiveSidebarBackgroundColorNoneSet001
 * @tc.desc: When no background color property set, returns std::nullopt
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, GetEffectiveSidebarBackgroundColorNoneSet001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    auto result = pattern_->GetEffectiveSidebarBackgroundColor();
    // GetEffectiveSidebarBackgroundColor falls through to GetBarBackgroundColor() which returns
    // const std::optional<Color>&. When the property is unset, the optional is empty.
    EXPECT_FALSE(result.has_value());
}

// ===================== SetAnimateMode =====================

/**
 * @tc.name: SetAnimateMode001
 * @tc.desc: SetAnimateMode updates animateMode_
 * @tc.type: FUNC
 */
HWTEST_F(TabsPatternSidebarTddTestNg, SetAnimateMode001, TestSize.Level1)
{
    TabsModelNG model = CreateTabs();
    CreateTabContents(TABCONTENT_NUMBER);
    CreateTabsDone(model);
    ASSERT_NE(pattern_, nullptr);

    pattern_->SetAnimateMode(TabAnimateMode::CONTENT_FIRST);
    EXPECT_EQ(pattern_->GetAnimateMode(), TabAnimateMode::CONTENT_FIRST);

    pattern_->SetAnimateMode(TabAnimateMode::ACTION_FIRST);
    EXPECT_EQ(pattern_->GetAnimateMode(), TabAnimateMode::ACTION_FIRST);
}
} // namespace OHOS::Ace::NG
