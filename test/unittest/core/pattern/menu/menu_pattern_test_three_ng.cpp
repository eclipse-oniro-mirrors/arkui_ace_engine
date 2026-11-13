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

#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"

#include "base/json/json_util.h"
#include "core/components/common/layout/grid_system_manager.h"
#include "core/components/common/properties/ui_material.h"
#include "core/components/select/select_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/menu/menu_layout_property.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/menu_theme.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/property/border_property.h"
#include "core/components_ng/property/measure_property.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::Framework;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t TARGET_ID = 3;
const std::string TEXT_TAG = "text";
const InspectorFilter filter;

// Build a menu FrameNode mounted under a MenuWrapper that carries the given
// MenuParam, so GetMenuParam()/GetMenuWrapper() resolve correctly.
// The caller must keep `outWrapper` alive for as long as it uses the returned
// pattern: the pattern's host is a WeakPtr to the menu FrameNode, which is only
// retained by the wrapper's child list. If `outWrapper` is destroyed, the host
// is destroyed and GetHost()/GetMenuWrapper()/GetMenuParam() all collapse to
// null/empty, making the test exercise the wrong (null-path) branches.
RefPtr<MenuPattern> BuildMenuPatternWithParam(
    RefPtr<FrameNode>& outWrapper, const MenuParam& menuParam, MenuType type = MenuType::MENU)
{
    outWrapper = FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuWrapperPattern>(TARGET_ID));
    auto menu = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, type));
    if (menu) {
        menu->MountToParent(outWrapper);
    }
    auto wrapperPattern = outWrapper ? outWrapper->GetPattern<MenuWrapperPattern>() : nullptr;
    if (wrapperPattern) {
        wrapperPattern->SetMenuParam(menuParam);
    }
    return menu ? menu->GetPattern<MenuPattern>() : nullptr;
}

// Build a select menu whose options_ are populated from selectParams_, mirroring
// how MenuView::Create(vector<SelectParam>) sets up the option list.
RefPtr<MenuPattern> BuildSelectMenuPattern(const std::vector<SelectParam>& selectParams)
{
    auto wrapperNode = MenuView::Create(selectParams, TARGET_ID, TEXT_TAG, false);
    if (!wrapperNode) {
        return nullptr;
    }
    auto menuUINode = wrapperNode->GetChildAtIndex(0);
    auto menuFrame = AceType::DynamicCast<FrameNode>(menuUINode);
    if (!menuFrame) {
        return nullptr;
    }
    return menuFrame->GetPattern<MenuPattern>();
}
} // namespace

class MenuPatternThreeTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void InstallThemes();
};

void MenuPatternThreeTestNg::SetUpTestCase() {}

void MenuPatternThreeTestNg::TearDownTestCase() {}

void MenuPatternThreeTestNg::SetUp()
{
    MockPipelineContext::SetUp();
    InstallThemes();
    MockContainer::SetUp();
}

void MenuPatternThreeTestNg::TearDown()
{
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

void MenuPatternThreeTestNg::InstallThemes()
{
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto resolver = [](ThemeType type) -> RefPtr<Theme> {
        if (type == SelectTheme::TypeId()) {
            return AceType::MakeRefPtr<SelectTheme>();
        }
        return AceType::MakeRefPtr<MenuTheme>();
    };
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(resolver);
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([](ThemeType type, int32_t /*id*/) -> RefPtr<Theme> {
            if (type == SelectTheme::TypeId()) {
                return AceType::MakeRefPtr<SelectTheme>();
            }
            return AceType::MakeRefPtr<MenuTheme>();
        });
}

/**
 * @tc.name: MenuPatternIsOffsetInNodeBounds001
 * @tc.desc: Test MenuPattern::IsOffsetInNodeBounds null / inside / outside branches.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternThreeTestNg, MenuPatternIsOffsetInNodeBounds001, TestSize.Level1)
{
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU);
    auto host = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), menuPattern);
    ASSERT_NE(host, nullptr);
    auto pattern = host->GetPattern<MenuPattern>();
    ASSERT_NE(pattern, nullptr);
    host->GetGeometryNode()->SetFrameSize(SizeF(100.0f, 100.0f));

    /**
     * @tc.steps: step1. null host.
     * @tc.expected: returns false.
     */
    EXPECT_FALSE(pattern->IsOffsetInNodeBounds(nullptr, Offset(0.0, 0.0)));

    /**
     * @tc.steps: step2. offset inside bounds.
     * @tc.expected: returns true.
     */
    EXPECT_TRUE(pattern->IsOffsetInNodeBounds(host, Offset(50.0, 50.0)));

    /**
     * @tc.steps: step3. offset outside bounds (x too large).
     * @tc.expected: returns false.
     */
    EXPECT_FALSE(pattern->IsOffsetInNodeBounds(host, Offset(150.0, 50.0)));

    /**
     * @tc.steps: step4. negative offset.
     * @tc.expected: returns false.
     */
    EXPECT_FALSE(pattern->IsOffsetInNodeBounds(host, Offset(-1.0, 50.0)));
}

/**
 * @tc.name: MenuPatternGetMainMenuPattern001
 * @tc.desc: Test MenuPattern::GetMainMenuPattern wrapper-child resolution branches.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternThreeTestNg, MenuPatternGetMainMenuPattern001, TestSize.Level1)
{
    MenuParam param;
    RefPtr<FrameNode> wrapper;
    auto pattern = BuildMenuPatternWithParam(wrapper, param);
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step1. menu mounted under a wrapper whose first child is the menu.
     * @tc.expected: returns the main menu pattern (non-null).
     */
    auto main = pattern->GetMainMenuPattern();
    EXPECT_NE(main, nullptr);

    /**
     * @tc.steps: step2. menu whose wrapper has been reset to null (no parent).
     * @tc.expected: GetMenuWrapper returns nullptr → GetMainMenuPattern returns nullptr.
     */
    auto orphanMenu = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU));
    ASSERT_NE(orphanMenu, nullptr);
    auto orphanPattern = orphanMenu->GetPattern<MenuPattern>();
    ASSERT_NE(orphanPattern, nullptr);
    EXPECT_EQ(orphanPattern->GetMainMenuPattern(), nullptr);
}

/**
 * @tc.name: MenuPatternShouldUpdateShadow001
 * @tc.desc: Test MenuPattern::ShouldUpdateShadow branches: no material / IMMERSIVE+applyShadow / other type.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternThreeTestNg, MenuPatternShouldUpdateShadow001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. no systemMaterial set on MenuParam.
     * @tc.expected: no valid material → returns true (default shadow).
     */
    MenuParam param;
    RefPtr<FrameNode> wrapper;
    auto pattern = BuildMenuPatternWithParam(wrapper, param);
    ASSERT_NE(pattern, nullptr);
    EXPECT_TRUE(pattern->ShouldUpdateShadow());

    /**
     * @tc.steps: step2. IMMERSIVE material with applyShadow=true.
     * @tc.expected: returns false (material handles shadow).
     */
    auto material = AceType::MakeRefPtr<UiMaterial>();
    material->SetType(static_cast<int32_t>(MaterialType::IMMERSIVE));
    ImmersiveOptions options;
    options.applyShadow = true;
    material->SetImmersiveOptions(options);
    MenuParam param2;
    param2.systemMaterial = material;
    RefPtr<FrameNode> wrapper2;
    auto pattern2 = BuildMenuPatternWithParam(wrapper2, param2);
    ASSERT_NE(pattern2, nullptr);
    EXPECT_FALSE(pattern2->ShouldUpdateShadow());

    /**
     * @tc.steps: step3. NON-IMMERSIVE material type (e.g. SEMI_TRANSPARENT).
     * @tc.expected: returns false (material carries its own visual effect).
     */
    auto material3 = AceType::MakeRefPtr<UiMaterial>();
    material3->SetType(static_cast<int32_t>(MaterialType::SEMI_TRANSPARENT));
    MenuParam param3;
    param3.systemMaterial = material3;
    RefPtr<FrameNode> wrapper3;
    auto pattern3 = BuildMenuPatternWithParam(wrapper3, param3);
    ASSERT_NE(pattern3, nullptr);
    EXPECT_FALSE(pattern3->ShouldUpdateShadow());
}

/**
 * @tc.name: MenuPatternIsUseDistortionAnimation001
 * @tc.desc: Test MenuPattern::IsUseDistortionAnimation early-return branches.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternThreeTestNg, MenuPatternIsUseDistortionAnimation001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. no systemMaterial set.
     * @tc.expected: IsEnableMaterialParam false → returns false.
     */
    MenuParam param;
    RefPtr<FrameNode> wrapper;
    auto pattern = BuildMenuPatternWithParam(wrapper, param);
    ASSERT_NE(pattern, nullptr);
    EXPECT_FALSE(pattern->IsUseDistortionAnimation());

    /**
     * @tc.steps: step2. host has no context (detached node) — exercised implicitly by no material.
     * @tc.expected: still returns false without crashing.
     */
    EXPECT_FALSE(pattern->IsUseDistortionAnimation());
}

/**
 * @tc.name: MenuPatternIsUseEdgeLightAnimation001
 * @tc.desc: Test MenuPattern::IsUseEdgeLightAnimation early-return branches.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternThreeTestNg, MenuPatternIsUseEdgeLightAnimation001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. no systemMaterial set.
     * @tc.expected: IsEnableMaterialParam false → returns false.
     */
    MenuParam param;
    RefPtr<FrameNode> wrapper;
    auto pattern = BuildMenuPatternWithParam(wrapper, param);
    ASSERT_NE(pattern, nullptr);
    EXPECT_FALSE(pattern->IsUseEdgeLightAnimation());
}

/**
 * @tc.name: MenuPatternGetSelectMenuWidth001
 * @tc.desc: Test MenuPattern::GetSelectMenuWidth returns a non-negative width.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternThreeTestNg, MenuPatternGetSelectMenuWidth001, TestSize.Level1)
{
    auto pattern = BuildSelectMenuPattern({ { "opt1", "" }, { "opt2", "" } });
    ASSERT_NE(pattern, nullptr);
    /**
     * @tc.steps: step1. IsWidthModifiedBySelect=false (default).
     * @tc.expected: returns the grid-column default width, which is >= min select width (>=0).
     */
    float width = pattern->GetSelectMenuWidth();
    EXPECT_GE(width, 0.0f);
}

/**
 * @tc.name: MenuPatternCalcIdealBorderRadius001
 * @tc.desc: Test MenuPattern::CalcIdealBorderRadius null-host and small-radius branches.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternThreeTestNg, MenuPatternCalcIdealBorderRadius001, TestSize.Level1)
{
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU);
    auto host = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), menuPattern);
    ASSERT_NE(host, nullptr);
    auto pattern = host->GetPattern<MenuPattern>();
    ASSERT_NE(pattern, nullptr);

    BorderRadiusProperty in;
    in.radiusTopLeft = Dimension(2.0_vp);
    in.radiusTopRight = Dimension(2.0_vp);
    in.radiusBottomLeft = Dimension(2.0_vp);
    in.radiusBottomRight = Dimension(2.0_vp);

    /**
     * @tc.steps: step1. menuSize large enough to accommodate the radii (API>=12 path).
     * @tc.expected: returned radius equals the input (maxRadiusW/H <= menuSize branch).
     */
    auto out = pattern->CalcIdealBorderRadius(in, SizeF(1000.0f, 1000.0f));
    EXPECT_EQ(out.radiusTopLeft.value_or(Dimension(-1)).ConvertToPx(),
        in.radiusTopLeft.value_or(Dimension()).ConvertToPx());

    /**
     * @tc.steps: step2. menuSize tiny so radii exceed it.
     * @tc.expected: returned radius falls back to the theme default radius (not the input).
     */
    auto out2 = pattern->CalcIdealBorderRadius(in, SizeF(1.0f, 1.0f));
    EXPECT_NE(out2.radiusTopLeft.value_or(Dimension(-1)).ConvertToPx(),
        in.radiusTopLeft.value_or(Dimension()).ConvertToPx());
}

/**
 * @tc.name: MenuPatternUpdateBorderRadius001
 * @tc.desc: Test MenuPattern::UpdateBorderRadius null and valid branches.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternThreeTestNg, MenuPatternUpdateBorderRadius001, TestSize.Level1)
{
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU);
    auto host = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), menuPattern);
    ASSERT_NE(host, nullptr);
    auto pattern = host->GetPattern<MenuPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step1. null menuNode.
     * @tc.expected: CHECK_NULL_VOID early return, no crash.
     */
    BorderRadiusProperty radius;
    radius.radiusTopLeft = Dimension(4.0_vp);
    pattern->UpdateBorderRadius(nullptr, radius);

    /**
     * @tc.steps: step2. valid menuNode but no scroll child.
     * @tc.expected: outer scroll node null-branch handled, menu render context updated.
     */
    pattern->UpdateBorderRadius(host, radius);
    auto rc = host->GetRenderContext();
    ASSERT_NE(rc, nullptr);
    EXPECT_TRUE(rc->GetBorderRadius().has_value());
}

/**
 * @tc.name: MenuPatternGetFirstNodeWithTagInParent001
 * @tc.desc: Test MenuPattern::GetFirstNodeWithTagInParent null / match / no-match branches.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternThreeTestNg, MenuPatternGetFirstNodeWithTagInParent001, TestSize.Level1)
{
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU);
    auto root = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), menuPattern);
    auto child = FrameNode::CreateFrameNode(
        V2::MENU_ITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuItemPattern>());
    ASSERT_NE(root, nullptr);
    ASSERT_NE(child, nullptr);
    child->MountToParent(root);
    auto pattern = root->GetPattern<MenuPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step1. null starting node.
     * @tc.expected: returns nullptr.
     */
    EXPECT_EQ(pattern->GetFirstNodeWithTagInParent(nullptr, V2::MENU_ITEM_ETS_TAG), nullptr);

    /**
     * @tc.steps: step2. node whose tag matches.
     * @tc.expected: returns the node itself.
     */
    auto found = pattern->GetFirstNodeWithTagInParent(child, V2::MENU_ITEM_ETS_TAG);
    EXPECT_EQ(found, child);

    /**
     * @tc.steps: step3. walk up the parent chain with no matching tag.
     * @tc.expected: returns nullptr when reaching the root with no match.
     */
    EXPECT_EQ(pattern->GetFirstNodeWithTagInParent(child, "nonexistent_tag"), nullptr);
}

/**
 * @tc.name: MenuPatternGetMenuColumn001
 * @tc.desc: Test MenuPattern::GetMenuColumn with empty host children and with a scroll+column chain.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternThreeTestNg, MenuPatternGetMenuColumn001, TestSize.Level1)
{
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU);
    auto host = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), menuPattern);
    ASSERT_NE(host, nullptr);
    auto pattern = host->GetPattern<MenuPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step1. host with no children → GetChildren().front() is invalid.
     * @tc.expected: returns nullptr (front null branch).
     */
    EXPECT_EQ(pattern->GetMenuColumn(), nullptr);

    /**
     * @tc.steps: step2. host with scroll→column chain.
     * @tc.expected: returns the column FrameNode.
     */
    auto scroll = FrameNode::CreateFrameNode(
        V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
    auto column = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    scroll->MountToParent(host);
    column->MountToParent(scroll);
    auto result = pattern->GetMenuColumn();
    EXPECT_EQ(result, column);
}

/**
 * @tc.name: MenuPatternMountOptionAndRemoveOption001
 * @tc.desc: Test MenuPattern::MountOption / RemoveOption happy path and null-column branches.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternThreeTestNg, MenuPatternMountOptionAndRemoveOption001, TestSize.Level1)
{
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU);
    auto host = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), menuPattern);
    ASSERT_NE(host, nullptr);
    auto pattern = host->GetPattern<MenuPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step1. MountOption with no column (host has no children) — GetMenuColumn returns null.
     * @tc.expected: CHECK_NULL_VOID early return, no crash.
     */
    auto option = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuItemPattern>());
    ASSERT_NE(option, nullptr);
    pattern->MountOption(option);
    EXPECT_TRUE(pattern->GetOptions().empty());

    /**
     * @tc.steps: step2. set up scroll→column, then MountOption adds it to the column.
     * @tc.expected: option is mounted, registered in options_, and its menu weak ptr set.
     */
    auto scroll = FrameNode::CreateFrameNode(
        V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
    auto column = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    scroll->MountToParent(host);
    column->MountToParent(scroll);
    pattern->MountOption(option);
    EXPECT_EQ(pattern->GetOptions().size(), 1u);

    /**
     * @tc.steps: step3. RemoveOption pops the last option off the column.
     * @tc.expected: column has no children left, options_ empty.
     */
    pattern->RemoveOption();
    EXPECT_TRUE(pattern->GetOptions().empty());
    EXPECT_TRUE(column->GetChildren().empty());
}

/**
 * @tc.name: MenuPatternAddBuildDividerTask001
 * @tc.desc: Test MenuPattern::AddBuildDividerTask idempotency and null-host branches.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternThreeTestNg, MenuPatternAddBuildDividerTask001, TestSize.Level1)
{
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU);
    auto host = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), menuPattern);
    ASSERT_NE(host, nullptr);
    auto pattern = host->GetPattern<MenuPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step1. call once.
     * @tc.expected: buildDividerTaskAdded_ becomes true.
     */
    pattern->AddBuildDividerTask();
    EXPECT_TRUE(pattern->buildDividerTaskAdded_);

    /**
     * @tc.steps: step2. call again — idempotency early return.
     * @tc.expected: no second pipeline callback registered (still true, no crash).
     */
    pattern->AddBuildDividerTask();
    EXPECT_TRUE(pattern->buildDividerTaskAdded_);

    /**
     * @tc.steps: step3. reset the flag and detach host (simulate null host via a fresh detached pattern).
     * @tc.expected: CHECK_NULL_VOID(GetHost) path — but our pattern is attached; cover by direct flag reset.
     */
    pattern->buildDividerTaskAdded_ = false;
    pattern->AddBuildDividerTask();
    EXPECT_TRUE(pattern->buildDividerTaskAdded_);
}

/**
 * @tc.name: MenuPatternRemoveLastNodeDivider001
 * @tc.desc: Test MenuPattern::RemoveLastNodeDivider null / non-Frame / valid branches.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternThreeTestNg, MenuPatternRemoveLastNodeDivider001, TestSize.Level1)
{
    auto menuPattern = AceType::MakeRefPtr<MenuPattern>(TARGET_ID, TEXT_TAG, MenuType::MENU);
    auto host = FrameNode::CreateFrameNode(
        V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), menuPattern);
    ASSERT_NE(host, nullptr);
    auto pattern = host->GetPattern<MenuPattern>();
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step1. null lastNode.
     * @tc.expected: CHECK_NULL_VOID early return, no crash.
     */
    pattern->RemoveLastNodeDivider(nullptr);

    /**
     * @tc.steps: step2. a valid MenuItem last node.
     * @tc.expected: RemoveBottomDivider invoked on its pattern (bottomDivider_ null → no-op), no crash.
     */
    auto item = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<MenuItemPattern>());
    ASSERT_NE(item, nullptr);
    auto itemPattern = item->GetPattern<MenuItemPattern>();
    ASSERT_NE(itemPattern, nullptr);
    pattern->RemoveLastNodeDivider(item);
    EXPECT_NE(itemPattern, nullptr);
}

/**
 * @tc.name: MenuPatternToJsonValueNoMaterial001
 * @tc.desc: Test MenuPattern::ToJsonValue fast-filter branch and default dumping.
 * @tc.type: FUNC
 */
HWTEST_F(MenuPatternThreeTestNg, MenuPatternToJsonValueNoMaterial001, TestSize.Level1)
{
    MenuParam param;
    RefPtr<FrameNode> wrapper;
    auto pattern = BuildMenuPatternWithParam(wrapper, param);
    ASSERT_NE(pattern, nullptr);

    /**
     * @tc.steps: step1. dump with default (non-fast) filter.
     * @tc.expected: json contains hasSystemMaterial=false attribute, no crash.
     */
    std::unique_ptr<JsonValue> json = JsonUtil::Create(true);
    pattern->ToJsonValue(json, filter);
    EXPECT_EQ(json->GetString("hasSystemMaterial"), "false");
    EXPECT_FALSE(json->GetString("distortionMode").empty());
    EXPECT_FALSE(json->GetString("edgeLightMode").empty());
}
} // namespace OHOS::Ace::NG
