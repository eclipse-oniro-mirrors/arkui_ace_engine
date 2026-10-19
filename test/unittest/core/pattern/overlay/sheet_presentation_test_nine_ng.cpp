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

#include "test/mock/interfaces/inner_api/ui_session/mock_ui_session_manager.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/base/view_abstract_model_ng.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/manager/safe_area/safe_area_manager.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/scrollable/scrollable_paint_property.h"
#include "core/components_ng/pattern/sheet/sheet_presentation_pattern.h"
#include "core/components_ng/pattern/sheet/sheet_style.h"
#include "core/components_ng/pattern/sheet/sheet_view.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
const std::string MESSAGE = "hello world";
} // namespace

class SheetPresentationTestNineNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void SheetPresentationTestNineNg::SetUpTestCase()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();
    auto safeAreaManager = AceType::MakeRefPtr<SafeAreaManager>();
    MockPipelineContext::GetCurrent()->safeAreaManager_ = safeAreaManager;
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly([](ThemeType type) -> RefPtr<Theme> {
        if (type == SheetTheme::TypeId()) {
            auto sheetTheme = AceType::MakeRefPtr<SheetTheme>();
            return sheetTheme;
        } else {
            return nullptr;
        }
    });
    EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly([](ThemeType type, int32_t) -> RefPtr<Theme> {
        if (type == SheetTheme::TypeId()) {
            auto sheetTheme = AceType::MakeRefPtr<SheetTheme>();
            return sheetTheme;
        } else {
            return nullptr;
        }
    });
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
}

void SheetPresentationTestNineNg::TearDownTestCase()
{
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

/**
 * @tc.name: SheetTitleBarHoverModeSafeAreaPadding001
 * @tc.desc: Verify SheetTitleBarHoverMode enum contains SAFE_AREA_PADDING=2 (AC-1).
 * @tc.type: FUNC
 */
HWTEST_F(SheetPresentationTestNineNg, SheetTitleBarHoverModeSafeAreaPadding001, TestSize.Level1)
{
    SheetPresentationTestNineNg::SetUpTestCase();
    EXPECT_EQ(static_cast<int32_t>(SheetTitleBarHoverMode::STANDARD), 0);
    EXPECT_EQ(static_cast<int32_t>(SheetTitleBarHoverMode::STACK), 1);
    EXPECT_EQ(static_cast<int32_t>(SheetTitleBarHoverMode::SAFE_AREA_PADDING), 2);
    SheetPresentationTestNineNg::TearDownTestCase();
}

/**
 * @tc.name: SafeAreaPaddingScrollClipMode001
 * @tc.desc: SAFE_AREA_PADDING mode sets ContentClipMode::SAFE_AREA on scroll (AC-2a).
 * @tc.type: FUNC
 */
HWTEST_F(SheetPresentationTestNineNg, SafeAreaPaddingScrollClipMode001, TestSize.Level1)
{
    SheetPresentationTestNineNg::SetUpTestCase();
    auto builder = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto callback = [](const std::string&) {};
    SheetStyle style;
    style.isTitleBuilder = true;
    style.sheetTitle = MESSAGE;
    style.titleBarHoverMode = SheetTitleBarHoverMode::SAFE_AREA_PADDING;
    auto sheetNode = SheetView::CreateSheetPage(0, "", builder, builder, std::move(callback), style);
    ASSERT_NE(sheetNode, nullptr);
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(sheetPattern, nullptr);
    auto scrollNode = sheetPattern->GetSheetScrollNode();
    ASSERT_NE(scrollNode, nullptr);
    auto paintProps = scrollNode->GetPaintProperty<ScrollablePaintProperty>();
    ASSERT_NE(paintProps, nullptr);
    auto contentClip = paintProps->GetContentClip();
    ASSERT_TRUE(contentClip.has_value());
    EXPECT_EQ(contentClip->first, ContentClipMode::SAFE_AREA);
    SheetPresentationTestNineNg::TearDownTestCase();
}

/**
 * @tc.name: SafeAreaPaddingScrollClipMode002
 * @tc.desc: STANDARD mode does not set SAFE_AREA clip on scroll (AC-4).
 * @tc.type: FUNC
 */
HWTEST_F(SheetPresentationTestNineNg, SafeAreaPaddingScrollClipMode002, TestSize.Level1)
{
    SheetPresentationTestNineNg::SetUpTestCase();
    auto builder = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto callback = [](const std::string&) {};
    SheetStyle style;
    style.isTitleBuilder = true;
    style.sheetTitle = MESSAGE;
    style.titleBarHoverMode = SheetTitleBarHoverMode::STANDARD;
    auto sheetNode = SheetView::CreateSheetPage(0, "", builder, builder, std::move(callback), style);
    ASSERT_NE(sheetNode, nullptr);
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(sheetPattern, nullptr);
    auto scrollNode = sheetPattern->GetSheetScrollNode();
    ASSERT_NE(scrollNode, nullptr);
    auto paintProps = scrollNode->GetPaintProperty<ScrollablePaintProperty>();
    ASSERT_NE(paintProps, nullptr);
    auto contentClip = paintProps->GetContentClip();
    ASSERT_FALSE(contentClip.has_value());
    SheetPresentationTestNineNg::TearDownTestCase();
}

/**
 * @tc.name: ApplySheetImmersiveStrategyDefaults001
 * @tc.desc: Strategy set, no explicit hoverMode → default SAFE_AREA_PADDING (AC-3a).
 * @tc.type: FUNC
 */
HWTEST_F(SheetPresentationTestNineNg, ApplySheetImmersiveStrategyDefaults001, TestSize.Level1)
{
    SheetPresentationTestNineNg::SetUpTestCase();
    auto pipeline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->ApplyDefaultImmersiveStrategy({ImmersiveStrategy::BIND_SHEET_EXPAND_SAFE_AREA});
    SheetStyle sheetStyle;
    ViewAbstractModelNG model;
    model.ApplySheetImmersiveStrategyDefaults(pipeline, sheetStyle);
    EXPECT_EQ(sheetStyle.titleBarHoverMode.value_or(SheetTitleBarHoverMode::STANDARD),
        SheetTitleBarHoverMode::SAFE_AREA_PADDING);
    EXPECT_FALSE(sheetStyle.titleBarBackgroundBlur.has_value());
    pipeline->ApplyDefaultImmersiveStrategy({});
    SheetPresentationTestNineNg::TearDownTestCase();
}

/**
 * @tc.name: ApplySheetImmersiveStrategyDefaults002
 * @tc.desc: Strategy not set, no explicit hoverMode → stays nullopt (AC-3d).
 * @tc.type: FUNC
 */
HWTEST_F(SheetPresentationTestNineNg, ApplySheetImmersiveStrategyDefaults002, TestSize.Level1)
{
    SheetPresentationTestNineNg::SetUpTestCase();
    auto pipeline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->ApplyDefaultImmersiveStrategy({});
    SheetStyle sheetStyle;
    ViewAbstractModelNG model;
    model.ApplySheetImmersiveStrategyDefaults(pipeline, sheetStyle);
    EXPECT_FALSE(sheetStyle.titleBarHoverMode.has_value());
    EXPECT_FALSE(sheetStyle.titleBarBackgroundBlur.has_value());
    SheetPresentationTestNineNg::TearDownTestCase();
}

/**
 * @tc.name: ApplySheetImmersiveStrategyDefaults003
 * @tc.desc: Strategy set, explicit hoverMode=STACK → preserved, not overridden (AC-3e).
 * @tc.type: FUNC
 */
HWTEST_F(SheetPresentationTestNineNg, ApplySheetImmersiveStrategyDefaults003, TestSize.Level1)
{
    SheetPresentationTestNineNg::SetUpTestCase();
    auto pipeline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->ApplyDefaultImmersiveStrategy({ImmersiveStrategy::BIND_SHEET_EXPAND_SAFE_AREA});
    SheetStyle sheetStyle;
    sheetStyle.titleBarHoverMode = SheetTitleBarHoverMode::STACK;
    ViewAbstractModelNG model;
    model.ApplySheetImmersiveStrategyDefaults(pipeline, sheetStyle);
    EXPECT_EQ(sheetStyle.titleBarHoverMode.value(), SheetTitleBarHoverMode::STACK);
    EXPECT_FALSE(sheetStyle.titleBarBackgroundBlur.has_value());
    pipeline->ApplyDefaultImmersiveStrategy({});
    SheetPresentationTestNineNg::TearDownTestCase();
}
} // namespace OHOS::Ace::NG
