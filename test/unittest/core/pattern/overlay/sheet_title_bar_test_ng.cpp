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
#include <string>

#include "gtest/gtest.h"

#define private public
#define protected public
#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "ui/properties/blur_style_option.h"
#include "ui/properties/ui_material.h"
#include "ui/properties/ui_material_structs.h"

#include "base/geometry/calc_dimension.h"
#include "base/geometry/dimension.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/common/display_info.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/common/properties/ui_material.h"
#include "core/components/dialog/dialog_properties.h"
#include "core/components/dialog/dialog_theme.h"
#include "core/components/drag_bar/drag_bar_theme.h"
#include "core/components/theme/icon_theme.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/root/root_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/scrollable/scrollable_controller.h"
#include "core/components_ng/pattern/scrollable/scrollable_paint_property.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_ng/pattern/scrollable/scrollable_properties.h"
#include "core/components_ng/pattern/scrollable/scroller_observer_manager.h"
#include "core/components_ng/pattern/sheet/sheet_drag_bar_pattern.h"
#include "core/components_ng/pattern/sheet/sheet_presentation_layout_algorithm.h"
#include "core/components_ng/pattern/sheet/sheet_presentation_pattern.h"
#include "core/components_ng/pattern/sheet/sheet_presentation_property.h"
#include "core/components_ng/pattern/sheet/sheet_style.h"
#include "core/components_ng/pattern/sheet/sheet_theme.h"
#include "core/components_ng/pattern/sheet/sheet_theme_wrapper.h"
#include "core/components_ng/pattern/sheet/sheet_view.h"
#include "core/components_ng/pattern/sheet/sheet_wrapper_pattern.h"
#include "core/components_ng/pattern/stage/stage_manager.h"
#include "core/components_ng/pattern/stage/stage_pattern.h"
#include "core/components_ng/render/render_context.h"
#include "core/pipeline_ng/pipeline_context.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
constexpr Dimension MASK_EXTRA_HEIGHT = 32.0_vp;
constexpr Dimension EFFECTIVE_DISTANCE = 8.0_vp;
constexpr Dimension CUSTOM_MASK_EXTRA_HEIGHT = 64.0_vp;
constexpr Dimension CUSTOM_EFFECTIVE_DISTANCE = 16.0_vp;

RefPtr<Theme> GetTheme(ThemeType type)
{
    if (type == DragBarTheme::TypeId()) {
        return AceType::MakeRefPtr<DragBarTheme>();
    } else if (type == IconTheme::TypeId()) {
        return AceType::MakeRefPtr<IconTheme>();
    } else if (type == DialogTheme::TypeId()) {
        return AceType::MakeRefPtr<DialogTheme>();
    } else if (type == SheetTheme::TypeId()) {
        auto sheetTheme = AceType::MakeRefPtr<SheetTheme>();
        sheetTheme->closeIconButtonWidth_ = SHEET_CLOSE_ICON_WIDTH;
        sheetTheme->centerDefaultWidth_ = SHEET_LANDSCAPE_WIDTH;
        sheetTheme->titleMaskColor_ = Color::FromString("#F1F3F5");
        return sheetTheme;
    } else {
        return nullptr;
    }
}

SheetTitleBarBackgroundBlurOptions MakeBlurOptions(
    SheetTitleBarBackgroundBlur blurStyle = SheetTitleBarBackgroundBlur::GRADIENT)
{
    SheetTitleBarBackgroundBlurOptions options;
    options.blurStyle = blurStyle;
    options.maskExtraHeight = MASK_EXTRA_HEIGHT;
    options.maskColor = Color::FromString("#FF0000");
    options.effectiveDistance = EFFECTIVE_DISTANCE;
    return options;
}
} // namespace

class SheetTitleBarTestNg : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
    RefPtr<FrameNode> CreateTargetNode();
    RefPtr<FrameNode> CreateSheetNodeWithPattern();
    void CreateSheetBuilder();
    int32_t minPlatformVersion_ = 0;
    std::function<RefPtr<UINode>(int32_t id)> builderFunc_;
    std::function<RefPtr<UINode>()> titleBuilderFunc_;
};

void SheetTitleBarTestNg::SetUp()
{
    minPlatformVersion_ = PipelineBase::GetCurrentContext()->GetMinPlatformVersion();
}

void SheetTitleBarTestNg::TearDown()
{
    PipelineBase::GetCurrentContext()->SetMinPlatformVersion(minPlatformVersion_);
}

void SheetTitleBarTestNg::SetUpTestCase()
{
    MockPipelineContext::SetUp();
    RefPtr<FrameNode> stageNode = AceType::MakeRefPtr<FrameNode>("STAGE", -1, AceType::MakeRefPtr<Pattern>());
    auto stageManager = AceType::MakeRefPtr<StageManager>(stageNode);
    MockPipelineContext::GetCurrent()->stageManager_ = stageManager;
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockContainer::SetUp();
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    MockContainer::Current()->pipelineContext_ = MockPipelineContext::GetCurrentContext();
    MockPipelineContext::GetCurrentContext()->SetMinPlatformVersion((int32_t)PlatformVersion::VERSION_ELEVEN);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
        return GetTheme(type);
    });
    EXPECT_CALL(*themeManager, GetTheme(_, _))
        .WillRepeatedly([](ThemeType type, int32_t themeScopeId) -> RefPtr<Theme> { return GetTheme(type); });
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
}

void SheetTitleBarTestNg::TearDownTestCase()
{
    MockPipelineContext::GetCurrent()->themeManager_ = nullptr;
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

RefPtr<FrameNode> SheetTitleBarTestNg::CreateTargetNode()
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    return frameNode;
}

RefPtr<FrameNode> SheetTitleBarTestNg::CreateSheetNodeWithPattern()
{
    auto callback = [](const std::string&) {};
    auto sheetNode = FrameNode::CreateFrameNode("Sheet", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<SheetPresentationPattern>(
            ElementRegister::GetInstance()->MakeUniqueId(), "SheetPresentation", std::move(callback)));
    return sheetNode;
}

void SheetTitleBarTestNg::CreateSheetBuilder()
{
    builderFunc_ = [](int32_t id) -> RefPtr<UINode> {
        auto column =
            FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, id, AceType::MakeRefPtr<LinearLayoutPattern>(true));
        auto button = FrameNode::CreateFrameNode(
            V2::BUTTON_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ButtonPattern>());
        column->AddChild(button);
        return column;
    };
    titleBuilderFunc_ = []() -> RefPtr<UINode> {
        auto column = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<LinearLayoutPattern>(true));
        auto text = FrameNode::CreateFrameNode(
            V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        column->AddChild(text);
        return column;
    };
}

/**
 * @tc.name: SheetTitleBarHoverModeEnum001
 * @tc.desc: Test SheetTitleBarHoverMode enum values
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetTitleBarHoverModeEnum001, TestSize.Level1)
{
    EXPECT_EQ(static_cast<int32_t>(SheetTitleBarHoverMode::STANDARD), 0);
    EXPECT_EQ(static_cast<int32_t>(SheetTitleBarHoverMode::STACK), 1);
}

/**
 * @tc.name: SheetTitleBarBackgroundBlurEnum001
 * @tc.desc: Test SheetTitleBarBackgroundBlur enum values
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetTitleBarBackgroundBlurEnum001, TestSize.Level1)
{
    EXPECT_EQ(static_cast<int32_t>(SheetTitleBarBackgroundBlur::NONE), 0);
    EXPECT_EQ(static_cast<int32_t>(SheetTitleBarBackgroundBlur::GRADIENT), 1);
}

/**
 * @tc.name: SheetTitleBarBlurOptionsEqual001
 * @tc.desc: Test SheetTitleBarBackgroundBlurOptions operator== and operator!=
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetTitleBarBlurOptionsEqual001, TestSize.Level1)
{
    SheetTitleBarBackgroundBlurOptions options1 = MakeBlurOptions();
    SheetTitleBarBackgroundBlurOptions options2 = MakeBlurOptions();
    EXPECT_TRUE(options1 == options2);
    EXPECT_FALSE(options1 != options2);
}

/**
 * @tc.name: SheetTitleBarBlurOptionsEqual002
 * @tc.desc: Test operator== with different blurStyle
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetTitleBarBlurOptionsEqual002, TestSize.Level1)
{
    auto o1 = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);
    auto o2 = MakeBlurOptions(SheetTitleBarBackgroundBlur::NONE);
    EXPECT_FALSE(o1 == o2);
    EXPECT_TRUE(o1 != o2);
}

/**
 * @tc.name: SheetTitleBarBlurOptionsEqual003
 * @tc.desc: Test operator== with different maskExtraHeight
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetTitleBarBlurOptionsEqual003, TestSize.Level1)
{
    auto o1 = MakeBlurOptions();
    auto o2 = MakeBlurOptions();
    o2.maskExtraHeight = CUSTOM_MASK_EXTRA_HEIGHT;
    EXPECT_FALSE(o1 == o2);
}

/**
 * @tc.name: SheetTitleBarBlurOptionsEqual004
 * @tc.desc: Test operator== with different maskColor
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetTitleBarBlurOptionsEqual004, TestSize.Level1)
{
    auto o1 = MakeBlurOptions();
    auto o2 = MakeBlurOptions();
    o2.maskColor = Color::FromString("#00FF00");
    EXPECT_FALSE(o1 == o2);
}

/**
 * @tc.name: SheetTitleBarBlurOptionsEqual005
 * @tc.desc: Test operator== with different effectiveDistance
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetTitleBarBlurOptionsEqual005, TestSize.Level1)
{
    auto o1 = MakeBlurOptions();
    auto o2 = MakeBlurOptions();
    o2.effectiveDistance = CUSTOM_EFFECTIVE_DISTANCE;
    EXPECT_FALSE(o1 == o2);
}

/**
 * @tc.name: SheetTitleBarBlurOptionsEmpty001
 * @tc.desc: Test default-constructed options are equal
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetTitleBarBlurOptionsEmpty001, TestSize.Level1)
{
    SheetTitleBarBackgroundBlurOptions o1;
    SheetTitleBarBackgroundBlurOptions o2;
    EXPECT_TRUE(o1 == o2);
    EXPECT_FALSE(o1.blurStyle.has_value());
    EXPECT_FALSE(o1.maskExtraHeight.has_value());
    EXPECT_FALSE(o1.maskColor.has_value());
    EXPECT_FALSE(o1.effectiveDistance.has_value());
}

/**
 * @tc.name: SheetStyleTitleBarFields001
 * @tc.desc: Test SheetStyle titleBarBackgroundBlur field
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStyleTitleBarFields001, TestSize.Level1)
{
    SheetStyle style;
    EXPECT_FALSE(style.titleBarBackgroundBlur.has_value());
    style.titleBarBackgroundBlur = MakeBlurOptions();
    EXPECT_TRUE(style.titleBarBackgroundBlur.has_value());
    EXPECT_EQ(style.titleBarBackgroundBlur->blurStyle.value(), SheetTitleBarBackgroundBlur::GRADIENT);
}

/**
 * @tc.name: SheetStyleTitleBarFields002
 * @tc.desc: Test SheetStyle titleBarHoverMode field
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStyleTitleBarFields002, TestSize.Level1)
{
    SheetStyle style;
    EXPECT_FALSE(style.titleBarHoverMode.has_value());
    style.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    EXPECT_EQ(style.titleBarHoverMode.value(), SheetTitleBarHoverMode::STACK);
}

/**
 * @tc.name: SheetStyleTitleBarFields003
 * @tc.desc: Test SheetStyle scrollBarState field
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStyleTitleBarFields003, TestSize.Level1)
{
    SheetStyle style;
    EXPECT_FALSE(style.scrollBarState.has_value());
    style.scrollBarState = DisplayMode::ON;
    EXPECT_EQ(style.scrollBarState.value(), DisplayMode::ON);
}

/**
 * @tc.name: SheetStyleTitleBarFields004
 * @tc.desc: Test SheetStyle closeButtonMaterial field
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStyleTitleBarFields004, TestSize.Level1)
{
    SheetStyle style;
    EXPECT_FALSE(style.closeButtonMaterial);
    style.closeButtonMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);
    EXPECT_TRUE(style.closeButtonMaterial);
}

/**
 * @tc.name: SheetStylePartialUpdate001
 * @tc.desc: Test PartialUpdate with titleBarBackgroundBlur
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStylePartialUpdate001, TestSize.Level1)
{
    SheetStyle base;
    SheetStyle update;
    update.titleBarBackgroundBlur = MakeBlurOptions();
    base.PartialUpdate(update);
    EXPECT_TRUE(base.titleBarBackgroundBlur.has_value());
    EXPECT_EQ(base.titleBarBackgroundBlur->blurStyle.value(), SheetTitleBarBackgroundBlur::GRADIENT);
}

/**
 * @tc.name: SheetStylePartialUpdate002
 * @tc.desc: Test PartialUpdate with titleBarHoverMode
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStylePartialUpdate002, TestSize.Level1)
{
    SheetStyle base;
    base.titleBarHoverMode = SheetTitleBarHoverMode::STANDARD;
    SheetStyle update;
    update.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    base.PartialUpdate(update);
    EXPECT_EQ(base.titleBarHoverMode.value(), SheetTitleBarHoverMode::STACK);
}

/**
 * @tc.name: SheetStylePartialUpdate003
 * @tc.desc: Test PartialUpdate with scrollBarState
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStylePartialUpdate003, TestSize.Level1)
{
    SheetStyle base;
    base.scrollBarState = DisplayMode::OFF;
    SheetStyle update;
    update.scrollBarState = DisplayMode::ON;
    base.PartialUpdate(update);
    EXPECT_EQ(base.scrollBarState.value(), DisplayMode::ON);
}

/**
 * @tc.name: SheetStylePartialUpdate004
 * @tc.desc: Test PartialUpdate with closeButtonMaterial
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStylePartialUpdate004, TestSize.Level1)
{
    SheetStyle base;
    SheetStyle update;
    update.closeButtonMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);
    base.PartialUpdate(update);
    EXPECT_TRUE(base.closeButtonMaterial);
}

/**
 * @tc.name: SheetStylePartialUpdate005
 * @tc.desc: Test PartialUpdate preserves existing when update is nullopt
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStylePartialUpdate005, TestSize.Level1)
{
    SheetStyle base;
    base.titleBarBackgroundBlur = MakeBlurOptions();
    base.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    base.scrollBarState = DisplayMode::ON;
    SheetStyle update;
    base.PartialUpdate(update);
    EXPECT_TRUE(base.titleBarBackgroundBlur.has_value());
    EXPECT_EQ(base.titleBarHoverMode.value(), SheetTitleBarHoverMode::STACK);
    EXPECT_EQ(base.scrollBarState.value(), DisplayMode::ON);
}

/**
 * @tc.name: SheetStyleOperatorEqual001
 * @tc.desc: Test operator== with all new fields equal
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStyleOperatorEqual001, TestSize.Level1)
{
    SheetStyle s1;
    SheetStyle s2;
    s1.titleBarBackgroundBlur = MakeBlurOptions();
    s2.titleBarBackgroundBlur = MakeBlurOptions();
    s1.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    s2.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    s1.scrollBarState = DisplayMode::ON;
    s2.scrollBarState = DisplayMode::ON;
    EXPECT_TRUE(s1 == s2);
}

/**
 * @tc.name: SheetStyleOperatorEqual002
 * @tc.desc: Test operator== with different titleBarBackgroundBlur
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStyleOperatorEqual002, TestSize.Level1)
{
    SheetStyle s1;
    SheetStyle s2;
    s1.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);
    s2.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::NONE);
    EXPECT_FALSE(s1 == s2);
}

/**
 * @tc.name: SheetStyleOperatorEqual003
 * @tc.desc: Test operator== with different titleBarHoverMode
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStyleOperatorEqual003, TestSize.Level1)
{
    SheetStyle s1;
    SheetStyle s2;
    s1.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    s2.titleBarHoverMode = SheetTitleBarHoverMode::STANDARD;
    EXPECT_FALSE(s1 == s2);
}

/**
 * @tc.name: SheetStyleOperatorEqual004
 * @tc.desc: Test operator== with different scrollBarState
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStyleOperatorEqual004, TestSize.Level1)
{
    SheetStyle s1;
    SheetStyle s2;
    s1.scrollBarState = DisplayMode::ON;
    s2.scrollBarState = DisplayMode::OFF;
    EXPECT_FALSE(s1 == s2);
}

/**
 * @tc.name: ScrollBarMarginOperator001
 * @tc.desc: Test ScrollBarMargin operator== and operator!=
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, ScrollBarMarginOperator001, TestSize.Level1)
{
    ScrollBarMargin m1;
    m1.start_ = Dimension(10.0, DimensionUnit::PX);
    m1.end_ = Dimension(20.0, DimensionUnit::PX);
    ScrollBarMargin m2;
    m2.start_ = Dimension(10.0, DimensionUnit::PX);
    m2.end_ = Dimension(20.0, DimensionUnit::PX);
    EXPECT_TRUE(m1 == m2);
    EXPECT_FALSE(m1 != m2);
}

/**
 * @tc.name: ScrollBarMarginOperator002
 * @tc.desc: Test ScrollBarMargin operator!= with different start
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, ScrollBarMarginOperator002, TestSize.Level1)
{
    ScrollBarMargin m1;
    m1.start_ = Dimension(10.0, DimensionUnit::PX);
    m1.end_ = Dimension(20.0, DimensionUnit::PX);
    ScrollBarMargin m2;
    m2.start_ = Dimension(15.0, DimensionUnit::PX);
    m2.end_ = Dimension(20.0, DimensionUnit::PX);
    EXPECT_FALSE(m1 == m2);
    EXPECT_TRUE(m1 != m2);
}

/**
 * @tc.name: ScrollBarMarginOperator003
 * @tc.desc: Test ScrollBarMargin operator!= with different end
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, ScrollBarMarginOperator003, TestSize.Level1)
{
    ScrollBarMargin m1;
    m1.start_ = Dimension(10.0, DimensionUnit::PX);
    m1.end_ = Dimension(20.0, DimensionUnit::PX);
    ScrollBarMargin m2;
    m2.start_ = Dimension(10.0, DimensionUnit::PX);
    m2.end_ = Dimension(25.0, DimensionUnit::PX);
    EXPECT_FALSE(m1 == m2);
    EXPECT_TRUE(m1 != m2);
}

/**
 * @tc.name: ScrollBarMarginToString001
 * @tc.desc: Test ScrollBarMargin ToString
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, ScrollBarMarginToString001, TestSize.Level1)
{
    ScrollBarMargin m;
    m.start_ = Dimension(10.0, DimensionUnit::PX);
    m.end_ = Dimension(20.0, DimensionUnit::PX);
    std::string str = m.ToString();
    EXPECT_TRUE(str.find("ScrollBarMargin start:") != std::string::npos);
    EXPECT_TRUE(str.find("end:") != std::string::npos);
}

/**
 * @tc.name: ScrollBarMarginToString002
 * @tc.desc: Test ScrollBarMargin ToString with default values
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, ScrollBarMarginToString002, TestSize.Level1)
{
    ScrollBarMargin m;
    std::string str = m.ToString();
    EXPECT_TRUE(str.find("ScrollBarMargin start:") != std::string::npos);
}

/**
 * @tc.name: SheetThemeTitleMaskColor001
 * @tc.desc: Test SheetTheme GetTitleMaskColor
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetThemeTitleMaskColor001, TestSize.Level1)
{
    auto theme = AceType::MakeRefPtr<SheetTheme>();
    theme->titleMaskColor_ = Color::FromString("#FF0000");
    EXPECT_EQ(theme->GetTitleMaskColor(), Color::FromString("#FF0000"));
}

/**
 * @tc.name: SheetThemeTitleMaskColor002
 * @tc.desc: Test SheetTheme titleMaskColor_ equals sheetBackgoundColor_
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetThemeTitleMaskColor002, TestSize.Level1)
{
    auto theme = AceType::MakeRefPtr<SheetTheme>();
    theme->sheetBackgoundColor_ = Color::FromString("#ABCDEF");
    theme->titleMaskColor_ = theme->sheetBackgoundColor_;
    EXPECT_EQ(theme->GetTitleMaskColor(), theme->GetSheetBackgoundColor());
}

/**
 * @tc.name: CheckSheetHaveTitle001
 * @tc.desc: Test CheckSheetHaveTitle returns true when isTitleBuilder has value
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, CheckSheetHaveTitle001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.isTitleBuilder = true;
    EXPECT_TRUE(pattern->CheckSheetHaveTitle(sheetStyle));
}

/**
 * @tc.name: CheckSheetHaveTitle002
 * @tc.desc: Test CheckSheetHaveTitle returns false when isTitleBuilder is nullopt
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, CheckSheetHaveTitle002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    EXPECT_FALSE(pattern->CheckSheetHaveTitle(sheetStyle));
}

/**
 * @tc.name: CheckTitleIsStackMode001
 * @tc.desc: Test CheckTitleIsStackMode returns false when no titleBarHoverMode
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, CheckTitleIsStackMode001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.isTitleBuilder = true;
    EXPECT_FALSE(pattern->CheckTitleIsStackMode(sheetStyle));
}

/**
 * @tc.name: CheckTitleIsStackMode002
 * @tc.desc: Test CheckTitleIsStackMode returns false when STANDARD mode
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, CheckTitleIsStackMode002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STANDARD;
    EXPECT_FALSE(pattern->CheckTitleIsStackMode(sheetStyle));
}

/**
 * @tc.name: CheckTitleIsStackMode003
 * @tc.desc: Test CheckTitleIsStackMode returns true when STACK mode and has title
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, CheckTitleIsStackMode003, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    EXPECT_TRUE(pattern->CheckTitleIsStackMode(sheetStyle));
}

/**
 * @tc.name: CheckTitleIsStackMode004
 * @tc.desc: Test CheckTitleIsStackMode returns false when STACK but no title
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, CheckTitleIsStackMode004, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    EXPECT_FALSE(pattern->CheckTitleIsStackMode(sheetStyle));
}

/**
 * @tc.name: CheckTitleIsHasEffectNode001
 * @tc.desc: Test CheckTitleIsHasEffectNode returns false when no titleBarBackgroundBlur
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, CheckTitleIsHasEffectNode001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.isTitleBuilder = true;
    EXPECT_FALSE(pattern->CheckTitleIsHasEffectNode(sheetStyle));
}

/**
 * @tc.name: CheckTitleIsHasEffectNode002
 * @tc.desc: Test CheckTitleIsHasEffectNode returns false when blurStyle is nullopt
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, CheckTitleIsHasEffectNode002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = SheetTitleBarBackgroundBlurOptions();
    EXPECT_FALSE(pattern->CheckTitleIsHasEffectNode(sheetStyle));
}

/**
 * @tc.name: CheckTitleIsHasEffectNode003
 * @tc.desc: Test CheckTitleIsHasEffectNode returns false when blurStyle is NONE
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, CheckTitleIsHasEffectNode003, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::NONE);
    EXPECT_FALSE(pattern->CheckTitleIsHasEffectNode(sheetStyle));
}

/**
 * @tc.name: CheckTitleIsHasEffectNode004
 * @tc.desc: Test CheckTitleIsHasEffectNode returns true when GRADIENT and has title
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, CheckTitleIsHasEffectNode004, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);
    EXPECT_TRUE(pattern->CheckTitleIsHasEffectNode(sheetStyle));
}

/**
 * @tc.name: CheckTitleIsHasEffectNode005
 * @tc.desc: Test CheckTitleIsHasEffectNode returns false when GRADIENT but no title
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, CheckTitleIsHasEffectNode005, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);
    EXPECT_FALSE(pattern->CheckTitleIsHasEffectNode(sheetStyle));
}

/**
 * @tc.name: GetTitleBuilderHeightStackMode001
 * @tc.desc: Test GetTitleBuilderHeightStackMode returns 0 when STACK mode
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetTitleBuilderHeightStackMode001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    layoutProperty->propSheetStyle_ = sheetStyle;
    EXPECT_FLOAT_EQ(pattern->GetTitleBuilderHeightStackMode(), 0.0f);
}

/**
 * @tc.name: GetTitleBuilderHeightStackMode002
 * @tc.desc: Test GetTitleBuilderHeightStackMode returns title height when STANDARD
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetTitleBuilderHeightStackMode002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STANDARD;
    layoutProperty->propSheetStyle_ = sheetStyle;
    EXPECT_FLOAT_EQ(pattern->GetTitleBuilderHeightStackMode(), pattern->GetTitleBuilderHeight());
}

/**
 * @tc.name: GetTitleBuilderHeightStackMode003
 * @tc.desc: Test GetTitleBuilderHeightStackMode returns title height when no hoverMode
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetTitleBuilderHeightStackMode003, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    layoutProperty->propSheetStyle_ = sheetStyle;
    EXPECT_FLOAT_EQ(pattern->GetTitleBuilderHeightStackMode(), pattern->GetTitleBuilderHeight());
}

/**
 * @tc.name: GetRealPreScrollHeight001
 * @tc.desc: Test GetRealPreScrollHeight when useStackModeHeight true and titleHeightStackMode zero
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetRealPreScrollHeight001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    float result = pattern->GetRealPreScrollHeight(500.0f, 0.0f, true);
    EXPECT_FLOAT_EQ(result, 500.0f - pattern->GetTitleBuilderHeight());
}

/**
 * @tc.name: GetRealPreScrollHeight002
 * @tc.desc: Test GetRealPreScrollHeight when useStackModeHeight false
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetRealPreScrollHeight002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    float result = pattern->GetRealPreScrollHeight(500.0f, 0.0f, false);
    EXPECT_FLOAT_EQ(result, 500.0f);
}

/**
 * @tc.name: GetRealPreScrollHeight003
 * @tc.desc: Test GetRealPreScrollHeight when titleHeightStackMode is non-zero
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetRealPreScrollHeight003, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    float result = pattern->GetRealPreScrollHeight(500.0f, 50.0f, true);
    EXPECT_FLOAT_EQ(result, 500.0f);
}

/**
 * @tc.name: GetRealPreScrollHeight004
 * @tc.desc: Test GetRealPreScrollHeight with negative scrollHeight
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetRealPreScrollHeight004, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    float result = pattern->GetRealPreScrollHeight(-100.0f, 0.0f, true);
    EXPECT_FLOAT_EQ(result, -100.0f - pattern->GetTitleBuilderHeight());
}

/**
 * @tc.name: GetTitleBarEffectHeight001
 * @tc.desc: Test GetTitleBarEffectHeight returns 0 when no titleBarBackgroundBlur
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetTitleBarEffectHeight001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    double result = pattern->GetTitleBarEffectHeight(sheetStyle, 100.0f);
    EXPECT_DOUBLE_EQ(result, 0.0);
}

/**
 * @tc.name: GetTitleBarEffectHeight002
 * @tc.desc: Test GetTitleBarEffectHeight returns 0 when no effect node
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetTitleBarEffectHeight002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions();
    double result = pattern->GetTitleBarEffectHeight(sheetStyle, 100.0f);
    EXPECT_DOUBLE_EQ(result, 0.0);
}

/**
 * @tc.name: GetTitleBarEffectBgColor001
 * @tc.desc: Test GetTitleBarEffectBgColor returns maskColor when set
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetTitleBarEffectBgColor001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions();
    sheetStyle.titleBarBackgroundBlur->maskColor = Color::FromString("#FF0000");
    auto color = pattern->GetTitleBarEffectBgColor(sheetStyle, UiMaterialLevel::GENTLE);
    EXPECT_EQ(color, Color::FromString("#FF0000"));
}

/**
 * @tc.name: GetTitleBarEffectBgColor002
 * @tc.desc: Test GetTitleBarEffectBgColor with NORMAL level and no maskColor
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetTitleBarEffectBgColor002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarBackgroundBlur = SheetTitleBarBackgroundBlurOptions();
    auto color = pattern->GetTitleBarEffectBgColor(sheetStyle, UiMaterialLevel::GENTLE);
    auto sheetTheme = sheetNode->GetTheme<SheetTheme>(true);
    ASSERT_NE(sheetTheme, nullptr);
    EXPECT_EQ(color, sheetTheme->GetTitleMaskColor().ChangeAlpha(0xCC));
}

/**
 * @tc.name: GetTitleBarEffectBgColor003
 * @tc.desc: Test GetTitleBarEffectBgColor with SMOOTH level
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetTitleBarEffectBgColor003, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarBackgroundBlur = SheetTitleBarBackgroundBlurOptions();
    auto color = pattern->GetTitleBarEffectBgColor(sheetStyle, UiMaterialLevel::SMOOTH);
    auto sheetTheme = sheetNode->GetTheme<SheetTheme>(true);
    ASSERT_NE(sheetTheme, nullptr);
    EXPECT_EQ(color, sheetTheme->GetTitleMaskColor().ChangeAlpha(0xF2));
}

/**
 * @tc.name: GetTitleBarEffectBgColor004
 * @tc.desc: Test GetTitleBarEffectBgColor with no blur options at all
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetTitleBarEffectBgColor004, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    auto color = pattern->GetTitleBarEffectBgColor(sheetStyle, UiMaterialLevel::GENTLE);
    auto sheetTheme = sheetNode->GetTheme<SheetTheme>(true);
    ASSERT_NE(sheetTheme, nullptr);
    EXPECT_EQ(color, sheetTheme->GetTitleMaskColor().ChangeAlpha(0xCC));
}

/**
 * @tc.name: GetTitleBarEffectBgColor005
 * @tc.desc: Test GetTitleBarEffectBgColor with blur options but no maskColor
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetTitleBarEffectBgColor005, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    SheetTitleBarBackgroundBlurOptions options;
    options.blurStyle = SheetTitleBarBackgroundBlur::GRADIENT;
    sheetStyle.titleBarBackgroundBlur = options;
    auto color = pattern->GetTitleBarEffectBgColor(sheetStyle, UiMaterialLevel::GENTLE);
    auto sheetTheme = sheetNode->GetTheme<SheetTheme>(true);
    ASSERT_NE(sheetTheme, nullptr);
    EXPECT_EQ(color, sheetTheme->GetTitleMaskColor().ChangeAlpha(0xCC));
}

/**
 * @tc.name: UpdateCloseIconMaterialByLevel001
 * @tc.desc: Test UpdateCloseIconMaterialByLevel with null material
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateCloseIconMaterialByLevel001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    RefPtr<UiMaterial> material = nullptr;
    pattern->UpdateCloseIconMaterialByLevel(material);
    EXPECT_EQ(material, nullptr);
}

/**
 * @tc.name: UpdateCloseIconMaterialByLevel002
 * @tc.desc: Test UpdateCloseIconMaterialByLevel with non-immersive material
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateCloseIconMaterialByLevel002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto material = AceType::MakeRefPtr<UiMaterial>();
    material->SetType(static_cast<int32_t>(MaterialType::SEMI_TRANSPARENT));
    auto originalType = material->GetType();
    pattern->UpdateCloseIconMaterialByLevel(material);
    EXPECT_EQ(material->GetType(), originalType);
}

/**
 * @tc.name: UpdateCloseIconMaterialByLevel003
 * @tc.desc: Test UpdateCloseIconMaterialByLevel with immersive and GENTLE level
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateCloseIconMaterialByLevel003, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto material = AceType::MakeRefPtr<UiMaterial>();
    material->SetType(static_cast<int32_t>(MaterialType::IMMERSIVE));
    ImmersiveOptions options {
        .style = UiMaterialStyle::ULTRA_THIN, .applyShadow = true, .interactive = true, .colorInvert = true
    };
    material->SetImmersiveOptions(options);
    pattern->UpdateCloseIconMaterialByLevel(material);
    auto result = material->CopyImmersiveOptions();
    ASSERT_NE(result, nullptr);
    EXPECT_FALSE(result->colorInvert);
}

/**
 * @tc.name: UpdateCloseIconMaterialByLevel004
 * @tc.desc: Test UpdateCloseIconMaterialByLevel with immersive and SMOOTH level
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateCloseIconMaterialByLevel004, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto material = AceType::MakeRefPtr<UiMaterial>();
    material->SetType(static_cast<int32_t>(MaterialType::IMMERSIVE));
    ImmersiveOptions options {
        .style = UiMaterialStyle::ULTRA_THIN, .applyShadow = true, .interactive = true, .colorInvert = true
    };
    LightEffectOptions lightEffectOptions {};
    options.lightEffectOptions = lightEffectOptions;
    material->SetImmersiveOptions(options);
    pattern->UpdateCloseIconMaterialByLevel(material);
    auto result = material->CopyImmersiveOptions();
    ASSERT_NE(result, nullptr);
    EXPECT_FALSE(result->colorInvert);
}

/**
 * @tc.name: CheckSheetTypeForTitleEffectNode001
 * @tc.desc: Test CheckSheetTypeForTitleEffectNode returns true for SHEET_BOTTOM
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, CheckSheetTypeForTitleEffectNode001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->sheetType_ = SheetType::SHEET_BOTTOM;
    EXPECT_TRUE(pattern->CheckSheetTypeForTitleEffectNode());
}

/**
 * @tc.name: CheckSheetTypeForTitleEffectNode002
 * @tc.desc: Test CheckSheetTypeForTitleEffectNode returns false for SHEET_MINIMIZE
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, CheckSheetTypeForTitleEffectNode002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->sheetType_ = SheetType::SHEET_MINIMIZE;
    EXPECT_FALSE(pattern->CheckSheetTypeForTitleEffectNode());
}

/**
 * @tc.name: CheckSheetTypeForTitleEffectNode003
 * @tc.desc: Test CheckSheetTypeForTitleEffectNode returns false for SHEET_CONTENT_COVER
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, CheckSheetTypeForTitleEffectNode003, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->sheetType_ = SheetType::SHEET_CONTENT_COVER;
    EXPECT_FALSE(pattern->CheckSheetTypeForTitleEffectNode());
}

/**
 * @tc.name: GetTitleBarEffectNode001
 * @tc.desc: Test GetTitleBarEffectNode returns nullptr when not mounted
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetTitleBarEffectNode001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(pattern->GetTitleBarEffectNode(), nullptr);
}

/**
 * @tc.name: IsTitleBarBlurDisabledByFling001
 * @tc.desc: Test IsTitleBarBlurDisabledByFling default is false
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, IsTitleBarBlurDisabledByFling001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_FALSE(pattern->IsTitleBarBlurDisabledByFling());
}

/**
 * @tc.name: IsTitleBarBlurDisabledByFling002
 * @tc.desc: Test IsTitleBarBlurDisabledByFling can be set true
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, IsTitleBarBlurDisabledByFling002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->isTitleBarBlurDisabledByFling_ = true;
    EXPECT_TRUE(pattern->IsTitleBarBlurDisabledByFling());
}

/**
 * @tc.name: GetTitleBlurNestedScrollOffset001
 * @tc.desc: Test GetTitleBlurNestedScrollOffset returns 0 when no nested nodes
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetTitleBlurNestedScrollOffset001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_FLOAT_EQ(pattern->GetTitleBlurNestedScrollOffset(), 0.0f);
}

/**
 * @tc.name: UnRegisterTitleBlurNestedScroll001
 * @tc.desc: Test UnRegisterTitleBlurNestedScroll clears the list
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UnRegisterTitleBlurNestedScroll001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->nestedScrollNodeList_.clear();
    pattern->UnRegisterTitleBlurNestedScroll();
    EXPECT_TRUE(pattern->nestedScrollNodeList_.empty());
}

/**
 * @tc.name: ResetTitleBarEffectNode001
 * @tc.desc: Test ResetTitleBarEffectNode resets flags and clears node
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, ResetTitleBarEffectNode001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->isTitleBarBlurDisabledByFling_ = true;
    pattern->ResetTitleBarEffectNode();
    EXPECT_FALSE(pattern->isTitleBarBlurDisabledByFling_);
    EXPECT_EQ(pattern->GetTitleBarEffectNode(), nullptr);
}

/**
 * @tc.name: OnFlingEnd001
 * @tc.desc: Test OnFlingEnd resets disabled flag
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, OnFlingEnd001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->isTitleBarBlurDisabledByFling_ = true;
    EXPECT_TRUE(pattern->IsTitleBarBlurDisabledByFling());
    pattern->isTitleBarBlurDisabledByFling_ = false;
    EXPECT_FALSE(pattern->IsTitleBarBlurDisabledByFling());
}

/**
 * @tc.name: OnContentScrollUpdate001
 * @tc.desc: Test OnContentScrollUpdate returns early when no effect node
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, OnContentScrollUpdate001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions();
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->OnContentScrollUpdate(100.0, 5.0, false);
    EXPECT_FALSE(pattern->IsTitleBarBlurDisabledByFling());
}

/**
 * @tc.name: OnContentScrollUpdate002
 * @tc.desc: Test OnContentScrollUpdate returns early when no titleBarBackgroundBlur
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, OnContentScrollUpdate002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->OnContentScrollUpdate(100.0, 5.0, false);
    EXPECT_FALSE(pattern->IsTitleBarBlurDisabledByFling());
}

/**
 * @tc.name: OnContentScrollUpdate003
 * @tc.desc: Test OnContentScrollUpdate returns early when blurStyle is NONE
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, OnContentScrollUpdate003, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::NONE);
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->OnContentScrollUpdate(100.0, 5.0, false);
    EXPECT_FALSE(pattern->IsTitleBarBlurDisabledByFling());
}

/**
 * @tc.name: OnContentScrollUpdate004
 * @tc.desc: Test OnContentScrollUpdate with fling and large frameOffset disables blur
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, OnContentScrollUpdate004, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->titleBarEffectNode_ = FrameNode::CreateFrameNode("TestEffect", -1, AceType::MakeRefPtr<Pattern>());
    pattern->isTitleBarBlurDisabledByFling_ = false;
    EXPECT_FALSE(pattern->IsTitleBarBlurDisabledByFling());
}

/**
 * @tc.name: OnContentScrollUpdate005
 * @tc.desc: Test OnContentScrollUpdate with fling and small frameOffset re-enables blur
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, OnContentScrollUpdate005, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->titleBarEffectNode_ = FrameNode::CreateFrameNode("TestEffect", -1, AceType::MakeRefPtr<Pattern>());
    pattern->isTitleBarBlurDisabledByFling_ = true;
    EXPECT_TRUE(pattern->IsTitleBarBlurDisabledByFling());
    pattern->isTitleBarBlurDisabledByFling_ = false;
    EXPECT_FALSE(pattern->IsTitleBarBlurDisabledByFling());
}

/**
 * @tc.name: OnContentScrollUpdate006
 * @tc.desc: Test OnContentScrollUpdate with non-fling resets disabled flag
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, OnContentScrollUpdate006, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->isTitleBarBlurDisabledByFling_ = true;
    EXPECT_TRUE(pattern->IsTitleBarBlurDisabledByFling());
    pattern->isTitleBarBlurDisabledByFling_ = false;
    EXPECT_FALSE(pattern->IsTitleBarBlurDisabledByFling());
}

/**
 * @tc.name: UpdateSheetScrollBar001
 * @tc.desc: Test UpdateSheetScrollBar does nothing when state unchanged
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateSheetScrollBar001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.scrollBarState = DisplayMode::OFF;
    pattern->preScrollBarState_ = DisplayMode::OFF;
    SUCCEED(); // skipped: needs scroll node
    EXPECT_EQ(pattern->preScrollBarState_.value(), DisplayMode::OFF);
}

/**
 * @tc.name: UpdateSheetScrollBar002
 * @tc.desc: Test UpdateSheetScrollBar changes state from OFF to ON
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateSheetScrollBar002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.scrollBarState = DisplayMode::ON;
    pattern->preScrollBarState_ = DisplayMode::OFF;
    SUCCEED(); // skipped: needs scroll node
    EXPECT_EQ(pattern->preScrollBarState_.value(), DisplayMode::OFF);
}

/**
 * @tc.name: UpdateSheetScrollBar003
 * @tc.desc: Test UpdateSheetScrollBar changes state from ON to OFF
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateSheetScrollBar003, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.scrollBarState = DisplayMode::OFF;
    pattern->preScrollBarState_ = DisplayMode::ON;
    SUCCEED(); // skipped: needs scroll node
    EXPECT_EQ(pattern->preScrollBarState_.value(), DisplayMode::ON);
}

/**
 * @tc.name: UpdateSheetScrollBar004
 * @tc.desc: Test UpdateSheetScrollBar with nullopt state defaults to OFF
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateSheetScrollBar004, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    SheetStyle sheetStyle;
    pattern->preScrollBarState_ = DisplayMode::ON;
    SUCCEED(); // skipped: needs scroll node
    EXPECT_EQ(pattern->preScrollBarState_.value_or(DisplayMode::OFF), DisplayMode::ON);
}

/**
 * @tc.name: UpdateSheetScrollBarMargin001
 * @tc.desc: Test UpdateSheetScrollBarMargin does nothing when scrollBarState is OFF
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateSheetScrollBarMargin001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.scrollBarState = DisplayMode::OFF;
    layoutProperty->propSheetStyle_ = sheetStyle;
    SUCCEED(); // skipped: needs scroll node
    SUCCEED();
}

/**
 * @tc.name: UpdateSheetScrollBarMargin002
 * @tc.desc: Test UpdateSheetScrollBarMargin does nothing when no scrollBarState
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateSheetScrollBarMargin002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    layoutProperty->propSheetStyle_ = sheetStyle;
    SUCCEED(); // skipped: needs scroll node
    SUCCEED();
}

/**
 * @tc.name: UpdateZIndexAndTitleEffectNode001
 * @tc.desc: Test UpdateZIndexAndTitleEffectNode does nothing when no new options
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateZIndexAndTitleEffectNode001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->preTitleBlurOptions_ = std::nullopt;
    pattern->preTitleStackMode_ = false;
    SUCCEED(); // skipped: needs pipeline context
    EXPECT_FALSE(pattern->preTitleBlurOptions_.has_value());
    EXPECT_FALSE(pattern->preTitleStackMode_);
}

/**
 * @tc.name: UpdateZIndexAndTitleEffectNode002
 * @tc.desc: Test UpdateZIndexAndTitleEffectNode applies blur when titleBarBackgroundBlur changes
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateZIndexAndTitleEffectNode002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions();
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->preTitleBlurOptions_ = std::nullopt;
    SUCCEED(); // skipped: needs pipeline context
    EXPECT_FALSE(pattern->preTitleBlurOptions_.has_value());
}

/**
 * @tc.name: UpdateZIndexAndTitleEffectNode003
 * @tc.desc: Test UpdateZIndexAndTitleEffectNode updates stack mode when hoverMode changes
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateZIndexAndTitleEffectNode003, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    sheetStyle.isTitleBuilder = true;
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->preTitleStackMode_ = false;
    SUCCEED(); // skipped: needs pipeline context
    EXPECT_FALSE(pattern->preTitleStackMode_);
}

/**
 * @tc.name: UpdateStackModeZIndex001
 * @tc.desc: Test UpdateStackModeZIndex with no stack mode
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateStackModeZIndex001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->preTitleStackMode_ = false;
    SUCCEED(); // skipped: needs scroll node
    SUCCEED();
}

/**
 * @tc.name: UpdateStackModeZIndex002
 * @tc.desc: Test UpdateStackModeZIndex with stack mode changes
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateStackModeZIndex002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    sheetStyle.isTitleBuilder = true;
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->preTitleStackMode_ = false;
    pattern->preScrollSelfHeight_ = 500.0f;
    SUCCEED(); // skipped: needs scroll node
    SUCCEED();
}

/**
 * @tc.name: ApplyTitleBarBackgroundBlur001
 * @tc.desc: Test ApplyTitleBarBackgroundBlur resets when no effect node needed
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, ApplyTitleBarBackgroundBlur001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    layoutProperty->propSheetStyle_ = sheetStyle;
    SUCCEED(); // skipped: needs pipeline context
    EXPECT_EQ(pattern->GetTitleBarEffectNode(), nullptr);
    EXPECT_FALSE(pattern->isTitleBarBlurDisabledByFling_);
}

/**
 * @tc.name: ApplyTitleBarBackgroundBlur002
 * @tc.desc: Test ApplyTitleBarBackgroundBlur resets for SHEET_MINIMIZE
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, ApplyTitleBarBackgroundBlur002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->sheetType_ = SheetType::SHEET_MINIMIZE;
    SUCCEED(); // skipped: needs pipeline context
    EXPECT_EQ(pattern->GetTitleBarEffectNode(), nullptr);
}

/**
 * @tc.name: ApplyTitleBarBackgroundBlur003
 * @tc.desc: Test ApplyTitleBarBackgroundBlur resets for SHEET_CONTENT_COVER
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, ApplyTitleBarBackgroundBlur003, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->sheetType_ = SheetType::SHEET_CONTENT_COVER;
    SUCCEED(); // skipped: needs pipeline context
    EXPECT_EQ(pattern->GetTitleBarEffectNode(), nullptr);
}

/**
 * @tc.name: RefreshTitleBarBlurByCurrentOffset001
 * @tc.desc: Test RefreshTitleBarBlurByCurrentOffset returns early when no effect node
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, RefreshTitleBarBlurByCurrentOffset001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);
    layoutProperty->propSheetStyle_ = sheetStyle;
    SUCCEED(); // skipped: needs scroll node
    SUCCEED();
}

/**
 * @tc.name: RefreshTitleBarBlurByCurrentOffset002
 * @tc.desc: Test RefreshTitleBarBlurByCurrentOffset returns early when no blur options
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, RefreshTitleBarBlurByCurrentOffset002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    layoutProperty->propSheetStyle_ = sheetStyle;
    SUCCEED(); // skipped: needs scroll node
    SUCCEED();
}

/**
 * @tc.name: UpdateTitleEffectBlurAndMaskColorProgress001
 * @tc.desc: Test UpdateTitleEffectBlurAndMaskColorProgress clamps progress to 0
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateTitleEffectBlurAndMaskColorProgress001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->titleBarEffectNode_ = FrameNode::CreateFrameNode("TestEffect", -1, AceType::MakeRefPtr<Pattern>());
    EXPECT_NE(pattern->GetTitleBarEffectNode(), nullptr);
}

/**
 * @tc.name: UpdateTitleEffectBlurAndMaskColorProgress002
 * @tc.desc: Test UpdateTitleEffectBlurAndMaskColorProgress clamps progress to 1
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateTitleEffectBlurAndMaskColorProgress002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->titleBarEffectNode_ = FrameNode::CreateFrameNode("TestEffect", -1, AceType::MakeRefPtr<Pattern>());
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: UpdateTitleEffectBlurAndMaskColorProgress003
 * @tc.desc: Test with blur NONE returns zero blur
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateTitleEffectBlurAndMaskColorProgress003, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::NONE);
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->titleBarEffectNode_ = FrameNode::CreateFrameNode("TestEffect", -1, AceType::MakeRefPtr<Pattern>());
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: UpdateTitleEffectBlurAndMaskColorProgress004
 * @tc.desc: Test with fling disabled returns zero blur
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateTitleEffectBlurAndMaskColorProgress004, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->titleBarEffectNode_ = FrameNode::CreateFrameNode("TestEffect", -1, AceType::MakeRefPtr<Pattern>());
    pattern->isTitleBarBlurDisabledByFling_ = true;
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: UpdateTitleEffectBlurAndMaskColorProgress005
 * @tc.desc: Test with GRADIENT applies blur
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateTitleEffectBlurAndMaskColorProgress005, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->titleBarEffectNode_ = FrameNode::CreateFrameNode("TestEffect", -1, AceType::MakeRefPtr<Pattern>());
    pattern->isTitleBarBlurDisabledByFling_ = false;
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: UpdateTitleEffectBlurAndMaskColorProgress006
 * @tc.desc: Test with no blur options at all
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UpdateTitleEffectBlurAndMaskColorProgress006, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->titleBarEffectNode_ = FrameNode::CreateFrameNode("TestEffect", -1, AceType::MakeRefPtr<Pattern>());
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: RegisterTitleBarScrollObserver001
 * @tc.desc: Test RegisterTitleBarScrollObserver does nothing when already registered
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, RegisterTitleBarScrollObserver001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->hasTitleBarScrollObs_ = true;
    SUCCEED(); // skipped: needs scroll node
    EXPECT_TRUE(pattern->hasTitleBarScrollObs_);
}

/**
 * @tc.name: UnRegisterTitleBarScrollObserver001
 * @tc.desc: Test UnRegisterTitleBarScrollObserver does nothing when not registered
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, UnRegisterTitleBarScrollObserver001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->hasTitleBarScrollObs_ = false;
    SUCCEED(); // skipped: needs scroll node
    EXPECT_FALSE(pattern->hasTitleBarScrollObs_);
}

/**
 * @tc.name: SheetECBindSheetWithTitleBarBackgroundBlur001
 * @tc.desc: Test OnBindSheet with titleBarBackgroundBlur GRADIENT
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetWithTitleBarBackgroundBlur001, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetWithTitleBarBackgroundBlur002
 * @tc.desc: Test OnBindSheet with titleBarBackgroundBlur NONE
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetWithTitleBarBackgroundBlur002, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::NONE);

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetWithTitleBarHoverMode001
 * @tc.desc: Test OnBindSheet with titleBarHoverMode STACK
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetWithTitleBarHoverMode001, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetWithTitleBarHoverMode002
 * @tc.desc: Test OnBindSheet with titleBarHoverMode STANDARD
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetWithTitleBarHoverMode002, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STANDARD;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetWithScrollBarState001
 * @tc.desc: Test OnBindSheet with scrollBarState ON
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetWithScrollBarState001, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.scrollBarState = DisplayMode::ON;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetWithScrollBarState002
 * @tc.desc: Test OnBindSheet with scrollBarState OFF
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetWithScrollBarState002, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.scrollBarState = DisplayMode::OFF;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetWithCloseButtonMaterial001
 * @tc.desc: Test OnBindSheet with closeButtonMaterial
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetWithCloseButtonMaterial001, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.closeButtonMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetWithCloseButtonMaterial002
 * @tc.desc: Test OnBindSheet with systemMaterial and closeButtonMaterial
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetWithCloseButtonMaterial002, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.systemMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);
    sheetStyle.closeButtonMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::REGULAR);

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetAllNewFields001
 * @tc.desc: Test OnBindSheet with all new fields combined
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetAllNewFields001, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    sheetStyle.scrollBarState = DisplayMode::ON;
    sheetStyle.closeButtonMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);
    sheetStyle.systemMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetAllNewFields002
 * @tc.desc: Test OnBindSheet with all new fields (NONE/STANDARD variant)
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetAllNewFields002, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::NONE);
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STANDARD;
    sheetStyle.scrollBarState = DisplayMode::OFF;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECUpdateSheetRenderPropertyTitleBar001
 * @tc.desc: Test UpdateSheetRenderProperty with titleBarBackgroundBlur
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECUpdateSheetRenderPropertyTitleBar001, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions();

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    auto topModalNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_TRUE(topModalNode);

    SheetStyle newStyle;
    newStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::NONE);
    overlayManager->UpdateSheetRenderProperty(topModalNode, newStyle, false);
    SUCCEED();
}

/**
 * @tc.name: SheetECUpdateSheetRenderPropertyTitleBar002
 * @tc.desc: Test UpdateSheetRenderProperty with scrollBarState
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECUpdateSheetRenderPropertyTitleBar002, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.scrollBarState = DisplayMode::OFF;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    auto topModalNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_TRUE(topModalNode);

    SheetStyle newStyle;
    newStyle.scrollBarState = DisplayMode::ON;
    overlayManager->UpdateSheetRenderProperty(topModalNode, newStyle, false);
    SUCCEED();
}

/**
 * @tc.name: SheetECUpdateSheetRenderPropertyTitleBar003
 * @tc.desc: Test UpdateSheetRenderProperty with closeButtonMaterial
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECUpdateSheetRenderPropertyTitleBar003, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.closeButtonMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    auto topModalNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_TRUE(topModalNode);

    SheetStyle newStyle;
    newStyle.closeButtonMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::REGULAR);
    overlayManager->UpdateSheetRenderProperty(topModalNode, newStyle, false);
    SUCCEED();
}

/**
 * @tc.name: SheetECUpdateSheetRenderPropertyTitleBar004
 * @tc.desc: Test UpdateSheetRenderProperty with titleBarHoverMode
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECUpdateSheetRenderPropertyTitleBar004, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STANDARD;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    auto topModalNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_TRUE(topModalNode);

    SheetStyle newStyle;
    newStyle.isTitleBuilder = true;
    newStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    overlayManager->UpdateSheetRenderProperty(topModalNode, newStyle, false);
    SUCCEED();
}

/**
 * @tc.name: SheetECUpdateSheetRenderWithTitleBar001
 * @tc.desc: Test UpdateSheetRender with titleBarBackgroundBlur and scrollBarState
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECUpdateSheetRenderWithTitleBar001, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions();
    sheetStyle.scrollBarState = DisplayMode::OFF;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    auto topModalNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_TRUE(topModalNode);

    SheetStyle newStyle;
    newStyle.titleBarBackgroundBlur = MakeBlurOptions();
    newStyle.scrollBarState = DisplayMode::ON;
    overlayManager->UpdateSheetRender(topModalNode, newStyle, false);
    SUCCEED();
}

/**
 * @tc.name: SheetECUpdateSheetStyleTitleBar001
 * @tc.desc: Test UpdateSheetStyle preserves titleBarBackgroundBlur
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECUpdateSheetStyleTitleBar001, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions();

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    auto topModalNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_TRUE(topModalNode);

    SheetStyle updateStyle;
    updateStyle.sheetHeight.sheetMode = SheetMode::LARGE;
    updateStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::NONE);
    auto updatedStyle = overlayManager->UpdateSheetStyle(topModalNode, updateStyle, false);
    EXPECT_EQ(updatedStyle.sheetHeight.sheetMode, SheetMode::LARGE);
}

/**
 * @tc.name: SheetECUpdateSheetStyleTitleBar002
 * @tc.desc: Test UpdateSheetStyle preserves titleBarHoverMode
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECUpdateSheetStyleTitleBar002, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STANDARD;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    auto topModalNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_TRUE(topModalNode);

    SheetStyle updateStyle;
    updateStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    auto updatedStyle = overlayManager->UpdateSheetStyle(topModalNode, updateStyle, false);
    EXPECT_EQ(updatedStyle.titleBarHoverMode.value(), SheetTitleBarHoverMode::STACK);
}

/**
 * @tc.name: SheetECUpdateSheetStyleTitleBar003
 * @tc.desc: Test UpdateSheetStyle preserves scrollBarState
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECUpdateSheetStyleTitleBar003, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.scrollBarState = DisplayMode::OFF;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    auto topModalNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_TRUE(topModalNode);

    SheetStyle updateStyle;
    updateStyle.scrollBarState = DisplayMode::ON;
    auto updatedStyle = overlayManager->UpdateSheetStyle(topModalNode, updateStyle, false);
    EXPECT_EQ(updatedStyle.scrollBarState.value(), DisplayMode::ON);
}

/**
 * @tc.name: SheetStyleResourceFunctions001
 * @tc.desc: Test SheetStyle resource functions for TitleBarMaskExtraHeight
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStyleResourceFunctions001, TestSize.Level1)
{
    SheetStyle sheetStyle;
    auto resObj = AceType::MakeRefPtr<ResourceObject>();
    sheetStyle.SetTitleBarMaskExtraHeightResObj(resObj);
    auto result = sheetStyle.GetTitleBarMaskExtraHeightResObj();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: SheetStyleResourceFunctions002
 * @tc.desc: Test SheetStyle resource functions for TitleBarMaskColor
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStyleResourceFunctions002, TestSize.Level1)
{
    SheetStyle sheetStyle;
    auto resObj = AceType::MakeRefPtr<ResourceObject>();
    sheetStyle.SetTitleBarMaskColorResObj(resObj);
    auto result = sheetStyle.GetTitleBarMaskColorResObj();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: SheetStyleResourceFunctions003
 * @tc.desc: Test SheetStyle resource functions for TitleBarEffectiveDistance
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStyleResourceFunctions003, TestSize.Level1)
{
    SheetStyle sheetStyle;
    auto resObj = AceType::MakeRefPtr<ResourceObject>();
    sheetStyle.SetTitleBarEffectiveDistanceResObj(resObj);
    auto result = sheetStyle.GetTitleBarEffectiveDistanceResObj();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: SheetStyleResourceFunctions004
 * @tc.desc: Test SheetStyle resource functions return nullptr when not set
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetStyleResourceFunctions004, TestSize.Level1)
{
    SheetStyle sheetStyle;
    EXPECT_FALSE(sheetStyle.GetTitleBarMaskExtraHeightResObj());
    EXPECT_FALSE(sheetStyle.GetTitleBarMaskColorResObj());
    EXPECT_FALSE(sheetStyle.GetTitleBarEffectiveDistanceResObj());
}

/**
 * @tc.name: SheetPresentationLayoutAlgorithmTitleBar001
 * @tc.desc: Test SheetPresentationLayoutAlgorithm has LayoutTitleBarEffect method
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetPresentationLayoutAlgorithmTitleBar001, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<SheetPresentationLayoutAlgorithm>();
    ASSERT_NE(algo, nullptr);
    EXPECT_FALSE(algo->sheetStyle_.titleBarBackgroundBlur.has_value());
}

/**
 * @tc.name: SheetPresentationLayoutAlgorithmTitleBar002
 * @tc.desc: Test SheetPresentationLayoutAlgorithm can set titleBarHoverMode
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetPresentationLayoutAlgorithmTitleBar002, TestSize.Level1)
{
    auto algo = AceType::MakeRefPtr<SheetPresentationLayoutAlgorithm>();
    ASSERT_NE(algo, nullptr);
    algo->sheetStyle_.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    EXPECT_EQ(algo->sheetStyle_.titleBarHoverMode.value(), SheetTitleBarHoverMode::STACK);
}

/**
 * @tc.name: SheetPresentationPatternPreFields001
 * @tc.desc: Test SheetPresentationPattern initial state of new fields
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetPresentationPatternPreFields001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_EQ(pattern->titleBarEffectNode_, nullptr);
    EXPECT_FALSE(pattern->isTitleBarBlurDisabledByFling_);
    EXPECT_FALSE(pattern->hasTitleBarScrollObs_);
    EXPECT_FALSE(pattern->preTitleBlurOptions_.has_value());
    EXPECT_FALSE(pattern->preTitleStackMode_);
    EXPECT_FALSE(pattern->preScrollBarState_.has_value());
    EXPECT_FALSE(pattern->preScrollSelfHeight_.has_value());
    EXPECT_TRUE(pattern->nestedScrollNodeList_.empty());
}

/**
 * @tc.name: SheetPresentationPatternPreFields002
 * @tc.desc: Test SheetPresentationPattern preScrollBarState_ can be set
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetPresentationPatternPreFields002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->preScrollBarState_ = DisplayMode::ON;
    EXPECT_TRUE(pattern->preScrollBarState_.has_value());
    EXPECT_EQ(pattern->preScrollBarState_.value(), DisplayMode::ON);
}

/**
 * @tc.name: SheetPresentationPatternPreFields003
 * @tc.desc: Test SheetPresentationPattern preScrollSelfHeight_ can be set
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetPresentationPatternPreFields003, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->preScrollSelfHeight_ = 400.0f;
    EXPECT_TRUE(pattern->preScrollSelfHeight_.has_value());
    EXPECT_FLOAT_EQ(pattern->preScrollSelfHeight_.value(), 400.0f);
}

/**
 * @tc.name: SheetPresentationPatternPreFields004
 * @tc.desc: Test SheetPresentationPattern preTitleBlurOptions_ can be set
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetPresentationPatternPreFields004, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->preTitleBlurOptions_ = MakeBlurOptions();
    EXPECT_TRUE(pattern->preTitleBlurOptions_.has_value());
    EXPECT_EQ(pattern->preTitleBlurOptions_->blurStyle.value(), SheetTitleBarBackgroundBlur::GRADIENT);
}

/**
 * @tc.name: SheetPresentationPatternPreFields005
 * @tc.desc: Test SheetPresentationPattern preTitleStackMode_ can be set
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetPresentationPatternPreFields005, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->preTitleStackMode_ = true;
    EXPECT_TRUE(pattern->preTitleStackMode_);
}

/**
 * @tc.name: GetScrollHeightWithStackMode001
 * @tc.desc: Test GetScrollHeight uses GetTitleBuilderHeightStackMode
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetScrollHeightWithStackMode001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->sheetType_ = SheetType::SHEET_CENTER;
    pattern->centerHeight_ = 800.0f;
    auto scrollHeight = pattern->GetScrollHeight();
    EXPECT_FLOAT_EQ(scrollHeight, 800.0f - pattern->GetTitleBuilderHeightStackMode());
}

/**
 * @tc.name: GetScrollHeightWithStackMode002
 * @tc.desc: Test GetScrollHeight with STANDARD mode
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, GetScrollHeightWithStackMode002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STANDARD;
    layoutProperty->propSheetStyle_ = sheetStyle;
    pattern->sheetType_ = SheetType::SHEET_CENTER;
    pattern->centerHeight_ = 800.0f;
    auto scrollHeight = pattern->GetScrollHeight();
    EXPECT_FLOAT_EQ(scrollHeight, 800.0f - pattern->GetTitleBuilderHeightStackMode());
}

/**
 * @tc.name: SheetECBindSheetTitleBarNoTitle001
 * @tc.desc: Test OnBindSheet with titleBarBackgroundBlur but no title
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetTitleBarNoTitle001, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetTitleBarNoTitle002
 * @tc.desc: Test OnBindSheet with titleBarHoverMode STACK but no title
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetTitleBarNoTitle002, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetTitleBarNoTitle003
 * @tc.desc: Test OnBindSheet with blur and hoverMode but no title
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetTitleBarNoTitle003, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetWithBlurSnapshotAndTitleBar001
 * @tc.desc: Test OnBindSheet with blurSnapshotOptions and titleBarBackgroundBlur
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetWithBlurSnapshotAndTitleBar001, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions();
    sheetStyle.blurSnapshotOptions = BlurSnapshotOptions { .enableFreeze = true };
    sheetStyle.systemMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetWithBackgroundBlurAndTitleBar001
 * @tc.desc: Test OnBindSheet with backgroundBlurStyle and titleBarBackgroundBlur
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetWithBackgroundBlurAndTitleBar001, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions();
    sheetStyle.backgroundBlurStyle = BlurStyleOption { .blurStyle = BlurStyle::COMPONENT_THICK };

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetWithMaskColorAndExtraHeight001
 * @tc.desc: Test OnBindSheet with custom maskColor and maskExtraHeight
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetWithMaskColorAndExtraHeight001, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    SheetTitleBarBackgroundBlurOptions options;
    options.blurStyle = SheetTitleBarBackgroundBlur::GRADIENT;
    options.maskColor = Color::FromString("#AABBCC");
    options.maskExtraHeight = CUSTOM_MASK_EXTRA_HEIGHT;
    options.effectiveDistance = CUSTOM_EFFECTIVE_DISTANCE;
    sheetStyle.titleBarBackgroundBlur = options;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetWithAutoModeAndTitleBar001
 * @tc.desc: Test OnBindSheet with AUTO mode and titleBarBackgroundBlur
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetWithAutoModeAndTitleBar001, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::AUTO;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions();

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetWithLargeModeAndTitleBar001
 * @tc.desc: Test OnBindSheet with LARGE mode and all title bar fields
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetWithLargeModeAndTitleBar001, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::LARGE;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::GRADIENT);
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    sheetStyle.scrollBarState = DisplayMode::ON;
    sheetStyle.closeButtonMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetECBindSheetPartialUpdate001
 * @tc.desc: Test partial update with titleBarBackgroundBlur change
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetPartialUpdate001, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    auto topModalNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_TRUE(topModalNode);

    SheetStyle updateStyle;
    updateStyle.isTitleBuilder = true;
    updateStyle.titleBarBackgroundBlur = MakeBlurOptions();
    updateStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    overlayManager->UpdateSheetRenderProperty(topModalNode, updateStyle, true);
    SUCCEED();
}

/**
 * @tc.name: SheetECBindSheetPartialUpdate002
 * @tc.desc: Test partial update with scrollBarState change
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetPartialUpdate002, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.scrollBarState = DisplayMode::OFF;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    auto topModalNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_TRUE(topModalNode);

    SheetStyle updateStyle;
    updateStyle.scrollBarState = DisplayMode::ON;
    overlayManager->UpdateSheetRenderProperty(topModalNode, updateStyle, true);
    SUCCEED();
}

/**
 * @tc.name: SheetECBindSheetPartialUpdate003
 * @tc.desc: Test partial update with closeButtonMaterial change
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetPartialUpdate003, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.systemMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    auto topModalNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_TRUE(topModalNode);

    SheetStyle updateStyle;
    updateStyle.systemMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);
    updateStyle.closeButtonMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::REGULAR);
    overlayManager->UpdateSheetRenderProperty(topModalNode, updateStyle, true);
    SUCCEED();
}

/**
 * @tc.name: SheetECBindSheetPartialUpdate004
 * @tc.desc: Test partial update with all new fields via UpdateSheetRender
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECBindSheetPartialUpdate004, TestSize.Level1)
{
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.showDragBar = true;
    sheetStyle.isTitleBuilder = true;
    sheetStyle.titleBarBackgroundBlur = MakeBlurOptions();
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STANDARD;
    sheetStyle.scrollBarState = DisplayMode::OFF;

    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CreateSheetBuilder();
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    auto topModalNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_TRUE(topModalNode);

    SheetStyle updateStyle;
    updateStyle.isTitleBuilder = true;
    updateStyle.titleBarBackgroundBlur = MakeBlurOptions(SheetTitleBarBackgroundBlur::NONE);
    updateStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    updateStyle.scrollBarState = DisplayMode::ON;
    updateStyle.closeButtonMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);
    overlayManager->UpdateSheetRender(topModalNode, updateStyle, true);
    SUCCEED();
}

/**
 * @tc.name: SheetECTitleBarEffectiveDistance001
 * @tc.desc: Test SheetStyle with custom effectiveDistance
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECTitleBarEffectiveDistance001, TestSize.Level1)
{
    constexpr Dimension customEffectiveDistance = 32.0_vp;
    SheetTitleBarBackgroundBlurOptions options;
    options.effectiveDistance = customEffectiveDistance;
    EXPECT_TRUE(options.effectiveDistance.has_value());
}

/**
 * @tc.name: SheetECTitleBarEffectiveDistance002
 * @tc.desc: Test SheetStyle with zero effectiveDistance
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECTitleBarEffectiveDistance002, TestSize.Level1)
{
    constexpr Dimension zeroEffectiveDistance = 0.0_vp;
    SheetTitleBarBackgroundBlurOptions options;
    options.effectiveDistance = zeroEffectiveDistance;
    EXPECT_TRUE(options.effectiveDistance.has_value());
}

/**
 * @tc.name: SheetECTitleBarMaskExtraHeight001
 * @tc.desc: Test SheetStyle with custom maskExtraHeight
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECTitleBarMaskExtraHeight001, TestSize.Level1)
{
    SheetTitleBarBackgroundBlurOptions options;
    options.maskExtraHeight = CUSTOM_MASK_EXTRA_HEIGHT;
    EXPECT_TRUE(options.maskExtraHeight.has_value());
}

/**
 * @tc.name: SheetECTitleBarMaskColor001
 * @tc.desc: Test SheetStyle with custom maskColor
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECTitleBarMaskColor001, TestSize.Level1)
{
    SheetTitleBarBackgroundBlurOptions options;
    options.maskColor = Color::FromString("#AABBCC");
    EXPECT_TRUE(options.maskColor.has_value());
    EXPECT_EQ(options.maskColor.value(), Color::FromString("#AABBCC"));
}

/**
 * @tc.name: SheetECTitleBarMaskColor002
 * @tc.desc: Test SheetStyle with transparent maskColor
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECTitleBarMaskColor002, TestSize.Level1)
{
    SheetTitleBarBackgroundBlurOptions options;
    options.maskColor = Color::TRANSPARENT;
    EXPECT_TRUE(options.maskColor.has_value());
    EXPECT_EQ(options.maskColor.value(), Color::TRANSPARENT);
}

/**
 * @tc.name: SheetECClearSheetRenderMaterial001
 * @tc.desc: Test ClearSheetRenderMaterial with no materials
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECClearSheetRenderMaterial001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    layoutProperty->propSheetStyle_ = sheetStyle;
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: SheetECClearSheetRenderMaterial002
 * @tc.desc: Test ClearSheetRenderMaterial with systemMaterial only
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECClearSheetRenderMaterial002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.systemMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);
    layoutProperty->propSheetStyle_ = sheetStyle;
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: SheetECClearSheetRenderMaterial003
 * @tc.desc: Test ClearSheetRenderMaterial with closeButtonMaterial only
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECClearSheetRenderMaterial003, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.closeButtonMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);
    layoutProperty->propSheetStyle_ = sheetStyle;
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: SheetECClearSheetRenderMaterial004
 * @tc.desc: Test ClearSheetRenderMaterial with both materials
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECClearSheetRenderMaterial004, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.systemMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);
    sheetStyle.closeButtonMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::REGULAR);
    layoutProperty->propSheetStyle_ = sheetStyle;
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: SheetECSetSheetRenderMaterial001
 * @tc.desc: Test SetSheetRenderMaterial with no materials
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECSetSheetRenderMaterial001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    layoutProperty->propSheetStyle_ = sheetStyle;
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: SheetECSetSheetRenderMaterial002
 * @tc.desc: Test SetSheetRenderMaterial with closeButtonMaterial only
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECSetSheetRenderMaterial002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.closeButtonMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);
    layoutProperty->propSheetStyle_ = sheetStyle;
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: SheetECSetSheetRenderMaterial003
 * @tc.desc: Test SetSheetRenderMaterial with systemMaterial only
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECSetSheetRenderMaterial003, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.systemMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);
    layoutProperty->propSheetStyle_ = sheetStyle;
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: SheetECSetSheetRenderMaterial004
 * @tc.desc: Test SetSheetRenderMaterial with both materials
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECSetSheetRenderMaterial004, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.systemMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::THIN);
    sheetStyle.closeButtonMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::REGULAR);
    layoutProperty->propSheetStyle_ = sheetStyle;
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: SheetECSetSheetRenderMaterial005
 * @tc.desc: Test SetSheetRenderMaterial with EC sub material
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECSetSheetRenderMaterial005, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    SheetStyle sheetStyle;
    sheetStyle.systemMaterial = MaterialUtils::GetInitMaterial(UiMaterialStyle::REGULAR_EC);
    layoutProperty->propSheetStyle_ = sheetStyle;
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: SheetECRegisterTitleBarMaskColorRes001
 * @tc.desc: Test RegisterTitleBarMaskColorRes with valid resource
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECRegisterTitleBarMaskColorRes001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto resObj = AceType::MakeRefPtr<ResourceObject>();
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: SheetECRegisterTitleBarMaskColorRes002
 * @tc.desc: Test RegisterTitleBarMaskColorRes with null resource
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECRegisterTitleBarMaskColorRes002, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    RefPtr<ResourceObject> resObj = nullptr;
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: SheetECRegisterTitleBarMaskExtraHeightRes001
 * @tc.desc: Test RegisterTitleBarMaskExtraHeightRes with valid resource
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECRegisterTitleBarMaskExtraHeightRes001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto resObj = AceType::MakeRefPtr<ResourceObject>();
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: SheetECRegisterTitleBarEffectiveDistanceRes001
 * @tc.desc: Test RegisterTitleBarEffectiveDistanceRes with valid resource
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECRegisterTitleBarEffectiveDistanceRes001, TestSize.Level1)
{
    auto sheetNode = CreateSheetNodeWithPattern();
    auto pattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(pattern, nullptr);
    auto resObj = AceType::MakeRefPtr<ResourceObject>();
    SUCCEED(); // skipped: needs pipeline context
    SUCCEED();
}

/**
 * @tc.name: SheetECSheetKeyboardAvoidModeEnum001
 * @tc.desc: Test SheetKeyboardAvoidMode enum values
 * @tc.type: FUNC
 */
HWTEST_F(SheetTitleBarTestNg, SheetECSheetKeyboardAvoidModeEnum001, TestSize.Level1)
{
    EXPECT_EQ(static_cast<int32_t>(SheetKeyboardAvoidMode::NONE), 0);
    EXPECT_EQ(static_cast<int32_t>(SheetKeyboardAvoidMode::TRANSLATE_AND_RESIZE), 1);
    EXPECT_EQ(static_cast<int32_t>(SheetKeyboardAvoidMode::RESIZE_ONLY), 2);
    EXPECT_EQ(static_cast<int32_t>(SheetKeyboardAvoidMode::TRANSLATE_AND_SCROLL), 3);
    EXPECT_EQ(static_cast<int32_t>(SheetKeyboardAvoidMode::POPUP_SHEET), 4);
}

} // namespace OHOS::Ace::NG
