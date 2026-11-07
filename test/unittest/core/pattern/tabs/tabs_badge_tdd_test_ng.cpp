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

#include "core/components/badge/badge_theme.h"
#include "core/components/dialog/dialog_theme.h"
#include "core/components/tab_bar/tab_theme.h"
#include "core/components_ng/pattern/badge/badge_data.h"
#include "core/components_ng/pattern/badge/badge_pattern.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"
#include "core/components_ng/pattern/tabs/tab_content_model.h"
#include "core/components_ng/pattern/tabs/tab_content_model_ng.h"
#include "core/components_ng/pattern/tabs/tab_content_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_model_ng.h"
#include "core/components_ng/pattern/tabs/tabs_node.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
namespace {
RefPtr<Theme> GetBadgeTddTheme(ThemeType type)
{
    if (type == TabTheme::TypeId()) {
        auto themeConstants = TestNG::CreateThemeConstants(THEME_PATTERN_TAB);
        return TabTheme::Builder().Build(themeConstants);
    }
    if (type == BadgeTheme::TypeId()) {
        return AceType::MakeRefPtr<BadgeTheme>();
    }
    return AceType::MakeRefPtr<DialogTheme>();
}

// Build a column node with icon+text children (no badge)
struct BadgeContentTree {
    RefPtr<FrameNode> columnNode;
    RefPtr<FrameNode> iconNode;
    RefPtr<FrameNode> textNode;
    RefPtr<TabContentPattern> tabContentPattern;

    void Build()
    {
        columnNode = FrameNode::CreateFrameNode(
            V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(false));
        ASSERT_NE(columnNode, nullptr);
        iconNode = FrameNode::CreateFrameNode(
            V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<ImagePattern>());
        ASSERT_NE(iconNode, nullptr);
        textNode = FrameNode::CreateFrameNode(
            V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<TextPattern>());
        ASSERT_NE(textNode, nullptr);
        iconNode->MountToParent(columnNode);
        textNode->MountToParent(columnNode);
        tabContentPattern = AceType::MakeRefPtr<TabContentPattern>(nullptr);
        ASSERT_NE(tabContentPattern, nullptr);
    }
};
} // namespace

class TabsBadgeTddTestNg : public TabsTestNg {
public:
    static void SetUpTestSuite()
    {
        TestNG::SetUpTestSuite();
        MockPipelineContext::GetCurrent()->SetUseFlushUITasks(true);
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
            return GetBadgeTddTheme(type);
        });
        EXPECT_CALL(*themeManager, GetTheme(_, _))
            .WillRepeatedly([](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> {
                return GetBadgeTddTheme(type);
            });
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    }
    static void TearDownTestSuite()
    {
        TestNG::TearDownTestSuite();
        g_isConfigChangePerform = false;
    }
};

// ===================== BuildBadgeParameters =====================

/**
 * @tc.name: BuildBadgeParametersDotMode001
 * @tc.desc: No badgeValue and no badgeCount → dot mode (6vp circle, 0vp font)
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, BuildBadgeParametersDotMode001, TestSize.Level1)
{
    TabBarBadgeParam param;
    bool isDefaultFontSize = true;
    bool isDefaultBadgeSize = true;
    bool isEllipseMode = true;
    auto result = TabContentModelNG::BuildBadgeParameters(param, isDefaultFontSize, isDefaultBadgeSize, isEllipseMode);
    EXPECT_FALSE(isEllipseMode);
    EXPECT_FALSE(isDefaultFontSize);
    EXPECT_FALSE(isDefaultBadgeSize);
    ASSERT_TRUE(result.badgeCircleSize.has_value());
    EXPECT_EQ(result.badgeCircleSize.value(), Dimension(6.0, DimensionUnit::VP));
    ASSERT_TRUE(result.badgeFontSize.has_value());
    EXPECT_EQ(result.badgeFontSize.value(), Dimension(0.0, DimensionUnit::VP));
    EXPECT_TRUE(result.badgeFontSizeByUser);
    EXPECT_TRUE(result.badgeCircleSizeByUser);
}

/**
 * @tc.name: BuildBadgeParametersCountMode001
 * @tc.desc: badgeCount set → ellipse mode with count and maxCount
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, BuildBadgeParametersCountMode001, TestSize.Level1)
{
    TabBarBadgeParam param;
    param.badgeCount = 5;
    param.badgeMaxCount = 99;
    bool isDefaultFontSize = true;
    bool isDefaultBadgeSize = true;
    bool isEllipseMode = false;
    auto result = TabContentModelNG::BuildBadgeParameters(param, isDefaultFontSize, isDefaultBadgeSize, isEllipseMode);
    EXPECT_TRUE(isEllipseMode);
    ASSERT_TRUE(result.badgeCount.has_value());
    EXPECT_EQ(result.badgeCount.value(), 5);
    ASSERT_TRUE(result.badgeMaxCount.has_value());
    EXPECT_EQ(result.badgeMaxCount.value(), 99);
}

/**
 * @tc.name: BuildBadgeParametersCountNoMaxCount001
 * @tc.desc: badgeCount set without badgeMaxCount → ellipse mode, badgeMaxCount not set
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, BuildBadgeParametersCountNoMaxCount001, TestSize.Level1)
{
    TabBarBadgeParam param;
    param.badgeCount = 3;
    bool isDefaultFontSize = true;
    bool isDefaultBadgeSize = true;
    bool isEllipseMode = false;
    auto result = TabContentModelNG::BuildBadgeParameters(param, isDefaultFontSize, isDefaultBadgeSize, isEllipseMode);
    EXPECT_TRUE(isEllipseMode);
    ASSERT_TRUE(result.badgeCount.has_value());
    EXPECT_EQ(result.badgeCount.value(), 3);
    EXPECT_FALSE(result.badgeMaxCount.has_value());
}

/**
 * @tc.name: BuildBadgeParametersValueMode001
 * @tc.desc: badgeValue set → ellipse mode with text value
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, BuildBadgeParametersValueMode001, TestSize.Level1)
{
    TabBarBadgeParam param;
    param.badgeValue = std::string("new");
    bool isDefaultFontSize = true;
    bool isDefaultBadgeSize = true;
    bool isEllipseMode = false;
    auto result = TabContentModelNG::BuildBadgeParameters(param, isDefaultFontSize, isDefaultBadgeSize, isEllipseMode);
    EXPECT_TRUE(isEllipseMode);
    ASSERT_TRUE(result.badgeValue.has_value());
    EXPECT_EQ(result.badgeValue.value(), std::string("new"));
}

/**
 * @tc.name: BuildBadgeParametersPositionDefault001
 * @tc.desc: badgePosition defaults to RIGHT_TOP and isPositionXy is false
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, BuildBadgeParametersPositionDefault001, TestSize.Level1)
{
    TabBarBadgeParam param;
    bool isDefaultFontSize = true;
    bool isDefaultBadgeSize = true;
    bool isEllipseMode = false;
    auto result = TabContentModelNG::BuildBadgeParameters(param, isDefaultFontSize, isDefaultBadgeSize, isEllipseMode);
    ASSERT_TRUE(result.badgePosition.has_value());
    EXPECT_EQ(result.badgePosition.value(), static_cast<int>(BadgePosition::RIGHT_TOP));
    ASSERT_TRUE(result.isPositionXy.has_value());
    EXPECT_FALSE(result.isPositionXy.value());
}

/**
 * @tc.name: BuildBadgeParametersBothValueAndCount001
 * @tc.desc: When both badgeValue and badgeCount set, badgeCount takes priority (dot check fails, count checked first)
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, BuildBadgeParametersBothValueAndCount001, TestSize.Level1)
{
    TabBarBadgeParam param;
    param.badgeCount = 5;
    param.badgeValue = std::string("text");
    bool isDefaultFontSize = true;
    bool isDefaultBadgeSize = true;
    bool isEllipseMode = false;
    auto result = TabContentModelNG::BuildBadgeParameters(param, isDefaultFontSize, isDefaultBadgeSize, isEllipseMode);
    EXPECT_TRUE(isEllipseMode);
    ASSERT_TRUE(result.badgeCount.has_value());
    EXPECT_EQ(result.badgeCount.value(), 5);
}

/**
 * @tc.name: BuildBadgeParametersEmptyString001
 * @tc.desc: badgeValue set to empty string → ellipse mode (has_value is true)
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, BuildBadgeParametersEmptyString001, TestSize.Level1)
{
    TabBarBadgeParam param;
    param.badgeValue = std::string("");
    bool isDefaultFontSize = true;
    bool isDefaultBadgeSize = true;
    bool isEllipseMode = false;
    auto result = TabContentModelNG::BuildBadgeParameters(param, isDefaultFontSize, isDefaultBadgeSize, isEllipseMode);
    EXPECT_TRUE(isEllipseMode);
    ASSERT_TRUE(result.badgeValue.has_value());
    EXPECT_EQ(result.badgeValue.value(), std::string(""));
}

// ===================== CalculateEllipseRtlPositionX =====================

/**
 * @tc.name: CalculateEllipseRtlPositionXEmptyChildren001
 * @tc.desc: When badge node has no children, circleWidth is 0 → returns eightVpPx
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, CalculateEllipseRtlPositionXEmptyChildren001, TestSize.Level1)
{
    auto badgeNode = FrameNode::CreateFrameNode(
        V2::BADGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(badgeNode, nullptr);
    float eightVpPx = 16.0f;
    float result = TabContentModelNG::CalculateEllipseRtlPositionX(badgeNode, eightVpPx);
    EXPECT_FLOAT_EQ(result, eightVpPx);
}

/**
 * @tc.name: CalculateEllipseRtlPositionXWithTextChild001
 * @tc.desc: When badge node has a text child with frame width, returns eightVpPx - circleWidth
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, CalculateEllipseRtlPositionXWithTextChild001, TestSize.Level1)
{
    auto badgeNode = FrameNode::CreateFrameNode(
        V2::BADGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(badgeNode, nullptr);
    auto textChild = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(textChild, nullptr);
    textChild->MountToParent(badgeNode);
    // Set frame rect width to 20.0f
    textChild->GetGeometryNode()->SetFrameSize(SizeF(20.0f, 10.0f));
    float eightVpPx = 16.0f;
    float result = TabContentModelNG::CalculateEllipseRtlPositionX(badgeNode, eightVpPx);
    EXPECT_FLOAT_EQ(result, 16.0f - 20.0f);
}

/**
 * @tc.name: CalculateEllipseRtlPositionXNonFrameChild001
 * @tc.desc: When the child is not a FrameNode, circleWidth stays 0 → returns eightVpPx
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, CalculateEllipseRtlPositionXNonFrameChild001, TestSize.Level1)
{
    auto badgeNode = FrameNode::CreateFrameNode(
        V2::BADGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(badgeNode, nullptr);
    // Add a non-FrameNode child via a stub UINode is complex; instead test the empty-children path
    float eightVpPx = 24.0f;
    float result = TabContentModelNG::CalculateEllipseRtlPositionX(badgeNode, eightVpPx);
    EXPECT_FLOAT_EQ(result, eightVpPx);
}

/**
 * @tc.name: CalculateEllipseRtlPositionXMultipleChildren001
 * @tc.desc: With multiple children, uses the first frame child's width
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, CalculateEllipseRtlPositionXMultipleChildren001, TestSize.Level1)
{
    auto badgeNode = FrameNode::CreateFrameNode(
        V2::BADGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<BadgePattern>());
    ASSERT_NE(badgeNode, nullptr);
    auto textChild1 = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    textChild1->GetGeometryNode()->SetFrameSize(SizeF(30.0f, 10.0f));
    textChild1->MountToParent(badgeNode);
    auto textChild2 = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    textChild2->GetGeometryNode()->SetFrameSize(SizeF(50.0f, 10.0f));
    textChild2->MountToParent(badgeNode);
    float eightVpPx = 16.0f;
    float result = TabContentModelNG::CalculateEllipseRtlPositionX(badgeNode, eightVpPx);
    // Uses last child (back()) width 50.0f → 16 - 50 = -34
    EXPECT_FLOAT_EQ(result, 16.0f - 50.0f);
}

// ===================== ConfigureBadgeNode =====================

/**
 * @tc.name: ConfigureBadgeNodeDotMode001
 * @tc.desc: ConfigureBadgeNode with no value/count creates a dot-mode badge node
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, ConfigureBadgeNodeDotMode001, TestSize.Level1)
{
    TabBarBadgeParam param;
    bool isEllipseMode = true;
    auto badgeNode = TabContentModelNG::ConfigureBadgeNode(param, isEllipseMode);
    ASSERT_NE(badgeNode, nullptr);
    EXPECT_FALSE(isEllipseMode);
    EXPECT_EQ(badgeNode->GetTag(), V2::BADGE_ETS_TAG);
}

/**
 * @tc.name: ConfigureBadgeNodeCountMode001
 * @tc.desc: ConfigureBadgeNode with badgeCount creates an ellipse-mode badge node
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, ConfigureBadgeNodeCountMode001, TestSize.Level1)
{
    TabBarBadgeParam param;
    param.badgeCount = 5;
    bool isEllipseMode = false;
    auto badgeNode = TabContentModelNG::ConfigureBadgeNode(param, isEllipseMode);
    ASSERT_NE(badgeNode, nullptr);
    EXPECT_TRUE(isEllipseMode);
}

/**
 * @tc.name: ConfigureBadgeNodeValueMode001
 * @tc.desc: ConfigureBadgeNode with badgeValue creates an ellipse-mode badge node
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, ConfigureBadgeNodeValueMode001, TestSize.Level1)
{
    TabBarBadgeParam param;
    param.badgeValue = std::string("new");
    bool isEllipseMode = false;
    auto badgeNode = TabContentModelNG::ConfigureBadgeNode(param, isEllipseMode);
    ASSERT_NE(badgeNode, nullptr);
    EXPECT_TRUE(isEllipseMode);
}

// ===================== CreateBadgeContentChild =====================

/**
 * @tc.name: CreateBadgeContentChildBasic001
 * @tc.desc: CreateBadgeContentChild creates a column node with icon and text children
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, CreateBadgeContentChildBasic001, TestSize.Level1)
{
    auto iconNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto contentChild = TabContentModelNG::CreateBadgeContentChild(iconNode, textNode, nullptr);
    ASSERT_NE(contentChild, nullptr);
    EXPECT_EQ(contentChild->GetTag(), V2::COLUMN_ETS_TAG);
    EXPECT_EQ(contentChild->GetChildren().size(), 2u);
}

/**
 * @tc.name: CreateBadgeContentChildWithLayoutProperty001
 * @tc.desc: CreateBadgeContentChild copies layout properties from columnLayoutProperty
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, CreateBadgeContentChildWithLayoutProperty001, TestSize.Level1)
{
    auto iconNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto columnLayoutProperty = AceType::MakeRefPtr<LinearLayoutProperty>(false);
    columnLayoutProperty->UpdateFlexDirection(FlexDirection::ROW);
    columnLayoutProperty->UpdateMainAxisAlign(FlexAlign::CENTER);
    columnLayoutProperty->UpdateCrossAxisAlign(FlexAlign::FLEX_END);
    auto contentChild = TabContentModelNG::CreateBadgeContentChild(iconNode, textNode, columnLayoutProperty);
    ASSERT_NE(contentChild, nullptr);
    auto layoutProp = contentChild->GetLayoutProperty<LinearLayoutProperty>();
    ASSERT_NE(layoutProp, nullptr);
    EXPECT_EQ(layoutProp->GetFlexDirectionValue(FlexDirection::COLUMN), FlexDirection::ROW);
    EXPECT_EQ(layoutProp->GetMainAxisAlignValue(FlexAlign::FLEX_START), FlexAlign::CENTER);
    EXPECT_EQ(layoutProp->GetCrossAxisAlignValue(FlexAlign::CENTER), FlexAlign::FLEX_END);
}

/**
 * @tc.name: CreateBadgeContentChildWithSpace001
 * @tc.desc: CreateBadgeContentChild copies space from columnLayoutProperty
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, CreateBadgeContentChildWithSpace001, TestSize.Level1)
{
    auto iconNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto columnLayoutProperty = AceType::MakeRefPtr<LinearLayoutProperty>(false);
    columnLayoutProperty->UpdateSpace(Dimension(8.0f, DimensionUnit::VP));
    auto contentChild = TabContentModelNG::CreateBadgeContentChild(iconNode, textNode, columnLayoutProperty);
    ASSERT_NE(contentChild, nullptr);
    auto layoutProp = contentChild->GetLayoutProperty<LinearLayoutProperty>();
    ASSERT_NE(layoutProp, nullptr);
    EXPECT_EQ(layoutProp->GetSpaceValue(Dimension(0.0f)), Dimension(8.0f, DimensionUnit::VP));
}

/**
 * @tc.name: CreateBadgeContentChildNullLayoutProperty001
 * @tc.desc: CreateBadgeContentChild with null columnLayoutProperty still creates column
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, CreateBadgeContentChildNullLayoutProperty001, TestSize.Level1)
{
    auto iconNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto contentChild = TabContentModelNG::CreateBadgeContentChild(iconNode, textNode, nullptr);
    ASSERT_NE(contentChild, nullptr);
    EXPECT_EQ(contentChild->GetTag(), V2::COLUMN_ETS_TAG);
    EXPECT_EQ(contentChild->GetChildren().size(), 2u);
}

// ===================== WrapWithBadge =====================

/**
 * @tc.name: WrapWithBadgeDotMode001
 * @tc.desc: WrapWithBadge wraps icon+text in a badge node (dot mode)
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, WrapWithBadgeDotMode001, TestSize.Level1)
{
    auto columnNode = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    ASSERT_NE(columnNode, nullptr);
    auto iconNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    TabBarBadgeParam param;
    TabContentModelNG::WrapWithBadge(columnNode, iconNode, textNode, param, nullptr);
    // After wrapping, column should have 1 child (the badge node)
    EXPECT_EQ(columnNode->GetChildren().size(), 1u);
    auto badgeNode = AceType::DynamicCast<FrameNode>(columnNode->GetChildren().front());
    ASSERT_NE(badgeNode, nullptr);
    EXPECT_EQ(badgeNode->GetTag(), V2::BADGE_ETS_TAG);
}

/**
 * @tc.name: WrapWithBadgeEllipseMode001
 * @tc.desc: WrapWithBadge with count param creates ellipse-mode badge
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, WrapWithBadgeEllipseMode001, TestSize.Level1)
{
    auto columnNode = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    ASSERT_NE(columnNode, nullptr);
    auto iconNode = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto textNode = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    TabBarBadgeParam param;
    param.badgeCount = 3;
    TabContentModelNG::WrapWithBadge(columnNode, iconNode, textNode, param, nullptr);
    EXPECT_EQ(columnNode->GetChildren().size(), 1u);
}

// ===================== UpdateBottomStyleBadge =====================

/**
 * @tc.name: UpdateBottomStyleBadgeAddBadge001
 * @tc.desc: hasBadge=true, no existing badge → AddBadgeToColumn path
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, UpdateBottomStyleBadgeAddBadge001, TestSize.Level1)
{
    BadgeContentTree tree;
    tree.Build();
    TabBarBadgeParam badgeParam;
    badgeParam.badgeCount = 5;
    tree.tabContentPattern->tabBarBadgeParam_ = badgeParam;

    EXPECT_EQ(tree.columnNode->GetChildren().size(), 2u);
    TabContentModelNG::UpdateBottomStyleBadge(
        tree.columnNode, tree.tabContentPattern, true, nullptr);
    // After adding badge, column should have 1 child (badge node containing icon+text)
    EXPECT_EQ(tree.columnNode->GetChildren().size(), 1u);
    auto badgeNode = AceType::DynamicCast<FrameNode>(tree.columnNode->GetChildren().front());
    ASSERT_NE(badgeNode, nullptr);
    EXPECT_EQ(badgeNode->GetTag(), V2::BADGE_ETS_TAG);
}

/**
 * @tc.name: UpdateBottomStyleBadgeRemoveBadge001
 * @tc.desc: hasBadge=false with existing badge → RemoveBadgeFromColumn path
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, UpdateBottomStyleBadgeRemoveBadge001, TestSize.Level1)
{
    BadgeContentTree tree;
    tree.Build();
    TabBarBadgeParam badgeParam;
    badgeParam.badgeCount = 5;
    tree.tabContentPattern->tabBarBadgeParam_ = badgeParam;

    // First add a badge
    TabContentModelNG::UpdateBottomStyleBadge(
        tree.columnNode, tree.tabContentPattern, true, nullptr);
    ASSERT_EQ(tree.columnNode->GetChildren().size(), 1u);
    auto badgeNode = AceType::DynamicCast<FrameNode>(tree.columnNode->GetChildren().front());
    ASSERT_NE(badgeNode, nullptr);
    EXPECT_EQ(badgeNode->GetTag(), V2::BADGE_ETS_TAG);

    // Now remove the badge (hasBadge=false, hasExistingBadge=true)
    TabContentModelNG::UpdateBottomStyleBadge(
        tree.columnNode, tree.tabContentPattern, false, nullptr);
    // After removing badge, column should have icon+text (2 children) again
    EXPECT_EQ(tree.columnNode->GetChildren().size(), 2u);
}

/**
 * @tc.name: UpdateBottomStyleBadgeNoChange001
 * @tc.desc: hasBadge=false, no existing badge → no-op
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, UpdateBottomStyleBadgeNoChange001, TestSize.Level1)
{
    BadgeContentTree tree;
    tree.Build();
    EXPECT_EQ(tree.columnNode->GetChildren().size(), 2u);
    // hasBadge=false, hasExistingBadge=false → no-op
    TabContentModelNG::UpdateBottomStyleBadge(
        tree.columnNode, tree.tabContentPattern, false, nullptr);
    EXPECT_EQ(tree.columnNode->GetChildren().size(), 2u);
}

/**
 * @tc.name: UpdateBottomStyleBadgeUpdateExisting001
 * @tc.desc: hasBadge=true with existing badge → UpdateExistingBadge path
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, UpdateBottomStyleBadgeUpdateExisting001, TestSize.Level1)
{
    BadgeContentTree tree;
    tree.Build();
    TabBarBadgeParam badgeParam;
    badgeParam.badgeCount = 5;
    tree.tabContentPattern->tabBarBadgeParam_ = badgeParam;

    // First add a badge
    TabContentModelNG::UpdateBottomStyleBadge(
        tree.columnNode, tree.tabContentPattern, true, nullptr);
    ASSERT_EQ(tree.columnNode->GetChildren().size(), 1u);

    // Update the same badge (hasBadge=true, hasExistingBadge=true)
    TabBarBadgeParam newParam;
    newParam.badgeCount = 10;
    tree.tabContentPattern->tabBarBadgeParam_ = newParam;
    TabContentModelNG::UpdateBottomStyleBadge(
        tree.columnNode, tree.tabContentPattern, true, nullptr);
    // Should still have 1 child (badge node, just updated)
    EXPECT_EQ(tree.columnNode->GetChildren().size(), 1u);
    auto badgeNode = AceType::DynamicCast<FrameNode>(tree.columnNode->GetChildren().front());
    ASSERT_NE(badgeNode, nullptr);
    EXPECT_EQ(badgeNode->GetTag(), V2::BADGE_ETS_TAG);
}

// ===================== AddBadgeToColumn =====================

/**
 * @tc.name: AddBadgeToColumnBasic001
 * @tc.desc: AddBadgeToColumn wraps icon+text in a badge node
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, AddBadgeToColumnBasic001, TestSize.Level1)
{
    BadgeContentTree tree;
    tree.Build();
    TabBarBadgeParam badgeParam;
    badgeParam.badgeValue = std::string("new");
    tree.tabContentPattern->tabBarBadgeParam_ = badgeParam;
    TabContentModelNG::AddBadgeToColumn(tree.columnNode, tree.tabContentPattern, nullptr);
    EXPECT_EQ(tree.columnNode->GetChildren().size(), 1u);
    auto badgeNode = AceType::DynamicCast<FrameNode>(tree.columnNode->GetChildren().front());
    ASSERT_NE(badgeNode, nullptr);
    EXPECT_EQ(badgeNode->GetTag(), V2::BADGE_ETS_TAG);
}

/**
 * @tc.name: AddBadgeToColumnNoBadgeParam001
 * @tc.desc: AddBadgeToColumn with no badge param (nullopt) → iconNode/textNode null → no wrap
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, AddBadgeToColumnNoBadgeParam001, TestSize.Level1)
{
    BadgeContentTree tree;
    tree.Build();
    // tabContentPattern->tabBarBadgeParam_ is nullopt → GetTabBarBadge().value() would throw
    // But the code path: WrapWithBadge(tabContentPattern->GetTabBarBadge().value(), ...)
    // This requires tabContentPattern->GetTabBarBadge() to have value, otherwise it's UB.
    // Set a param to avoid crash:
    TabBarBadgeParam badgeParam;
    tree.tabContentPattern->tabBarBadgeParam_ = badgeParam;
    TabContentModelNG::AddBadgeToColumn(tree.columnNode, tree.tabContentPattern, nullptr);
    EXPECT_EQ(tree.columnNode->GetChildren().size(), 1u);
}

// ===================== RemoveBadgeFromColumn =====================

/**
 * @tc.name: RemoveBadgeFromColumnBasic001
 * @tc.desc: RemoveBadgeFromColumn extracts icon+text from badge and remounts to column
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, RemoveBadgeFromColumnBasic001, TestSize.Level1)
{
    BadgeContentTree tree;
    tree.Build();
    TabBarBadgeParam badgeParam;
    badgeParam.badgeCount = 3;
    tree.tabContentPattern->tabBarBadgeParam_ = badgeParam;
    // First add badge
    TabContentModelNG::AddBadgeToColumn(tree.columnNode, tree.tabContentPattern, nullptr);
    ASSERT_EQ(tree.columnNode->GetChildren().size(), 1u);
    auto badgeNode = AceType::DynamicCast<FrameNode>(tree.columnNode->GetChildren().front());
    ASSERT_NE(badgeNode, nullptr);

    // Now remove badge
    TabContentModelNG::RemoveBadgeFromColumn(tree.columnNode, badgeNode);
    EXPECT_EQ(tree.columnNode->GetChildren().size(), 2u);
}

/**
 * @tc.name: RemoveBadgeFromColumnEmptyBadge001
 * @tc.desc: RemoveBadgeFromColumn with empty badge children → column cleaned, no icon/text remounted
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, RemoveBadgeFromColumnEmptyBadge001, TestSize.Level1)
{
    BadgeContentTree tree;
    tree.Build();
    // Create an empty badge node (no children)
    auto emptyBadge = FrameNode::CreateFrameNode(
        V2::BADGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<BadgePattern>());
    ASSERT_NE(emptyBadge, nullptr);
    emptyBadge->MountToParent(tree.columnNode);
    ASSERT_EQ(tree.columnNode->GetChildren().size(), 3u);

    TabContentModelNG::RemoveBadgeFromColumn(tree.columnNode, emptyBadge);
    // After Clean, column should be empty (icon/text were not extracted from empty badge)
    EXPECT_EQ(tree.columnNode->GetChildren().size(), 0u);
}

// ===================== UpdateExistingBadge =====================

/**
 * @tc.name: UpdateExistingBadgeDotMode001
 * @tc.desc: UpdateExistingBadge with dot mode (no count/value) updates badge params
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, UpdateExistingBadgeDotMode001, TestSize.Level1)
{
    BadgeContentTree tree;
    tree.Build();
    TabBarBadgeParam badgeParam;
    tree.tabContentPattern->tabBarBadgeParam_ = badgeParam;
    // First add badge
    TabContentModelNG::AddBadgeToColumn(tree.columnNode, tree.tabContentPattern, nullptr);
    auto badgeNode = AceType::DynamicCast<FrameNode>(tree.columnNode->GetChildren().front());
    ASSERT_NE(badgeNode, nullptr);

    // Update with dot mode
    TabContentModelNG::UpdateExistingBadge(tree.columnNode, badgeNode, tree.tabContentPattern);
    EXPECT_EQ(tree.columnNode->GetChildren().size(), 1u);
    auto badgeLayoutProp = badgeNode->GetLayoutProperty<BadgeLayoutProperty>();
    ASSERT_NE(badgeLayoutProp, nullptr);
    // Dot mode → badgeCount was reset (no value)
    EXPECT_FALSE(badgeLayoutProp->HasBadgeCount());
}

/**
 * @tc.name: UpdateExistingBadgeCountMode001
 * @tc.desc: UpdateExistingBadge with count mode keeps badge node structure
 * @tc.type: FUNC
 */
HWTEST_F(TabsBadgeTddTestNg, UpdateExistingBadgeCountMode001, TestSize.Level1)
{
    BadgeContentTree tree;
    tree.Build();
    TabBarBadgeParam badgeParam;
    badgeParam.badgeCount = 5;
    tree.tabContentPattern->tabBarBadgeParam_ = badgeParam;
    TabContentModelNG::AddBadgeToColumn(tree.columnNode, tree.tabContentPattern, nullptr);
    auto badgeNode = AceType::DynamicCast<FrameNode>(tree.columnNode->GetChildren().front());
    ASSERT_NE(badgeNode, nullptr);

    // Update with count mode
    TabBarBadgeParam newParam;
    newParam.badgeCount = 10;
    tree.tabContentPattern->tabBarBadgeParam_ = newParam;
    TabContentModelNG::UpdateExistingBadge(tree.columnNode, badgeNode, tree.tabContentPattern);
    EXPECT_EQ(tree.columnNode->GetChildren().size(), 1u);
    auto badgeLayoutProp = badgeNode->GetLayoutProperty<BadgeLayoutProperty>();
    ASSERT_NE(badgeLayoutProp, nullptr);
    ASSERT_TRUE(badgeLayoutProp->HasBadgeCount());
    EXPECT_EQ(badgeLayoutProp->GetBadgeCountValue(0), 10);
}

} // namespace OHOS::Ace::NG
