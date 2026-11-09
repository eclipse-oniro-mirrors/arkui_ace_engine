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
#include "ui/base/geometry/ng/size_t.h"

#define private public
#define protected public
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/common/recorder/event_recorder.h"
#include "core/components/button/button_theme.h"
#include "core/components/common/layout/grid_column_info.h"
#include "core/components/common/layout/grid_container_info.h"
#include "core/components/dialog/dialog_properties.h"
#include "core/components/dialog/dialog_theme.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/manager/safe_area/safe_area_manager.h"
#include "core/components_ng/pattern/dialog/custom_dialog/custom_dialog_controller_model_ng.h"
#include "core/components_ng/pattern/dialog/dialog_event_hub.h"
#include "core/components_ng/pattern/dialog/dialog_layout_algorithm.h"
#include "core/components_ng/pattern/dialog/dialog_layout_property.h"
#include "core/components_ng/pattern/dialog/dialog_pattern.h"
#include "core/components_ng/pattern/dialog/dialog_view.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/navigation/navigation_declaration.h"
#include "core/components_ng/pattern/overlay/dialog_manager.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/root/root_pattern.h"
#include "core/components_ng/pattern/text/text_layout_algorithm.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
const InspectorFilter filter;
const std::string SHEET_TITLE = "sheet item";
const std::string INTERNAL_SOURCE = "$r('app.media.icon')";
constexpr float DEFAULT_WIDTH = 600.0f;
constexpr float DEFAULT_HEIGHT = 1000.0f;
} // namespace

class DialogLayoutFourTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetDialogTheme();

private:
    vector<ActionSheetInfo> sheetItems = {
        ActionSheetInfo {
            .title = SHEET_TITLE,
            .icon = INTERNAL_SOURCE,
        },
    };
    vector<ButtonInfo> btnItems = {
        ButtonInfo {
            .text = "main button",
            .bgColor = Color::BLACK,
        },
    };
};

void DialogLayoutFourTestNg::SetUpTestCase()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    MockContainer::Current()->pipelineContext_ = MockPipelineContext::GetCurrentContext();
    MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
}

void DialogLayoutFourTestNg::TearDownTestCase()
{
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

void DialogLayoutFourTestNg::SetDialogTheme()
{
    auto themeManager = AceType::DynamicCast<MockThemeManager>(MockPipelineContext::GetCurrent()->GetThemeManager());
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<DialogTheme>()));
    EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(Return(AceType::MakeRefPtr<DialogTheme>()));
    EXPECT_CALL(*themeManager, GetTheme(ButtonTheme::TypeId()))
        .WillRepeatedly(Return(AceType::MakeRefPtr<ButtonTheme>()));
    EXPECT_CALL(*themeManager, GetTheme(ButtonTheme::TypeId(), _))
        .WillRepeatedly(Return(AceType::MakeRefPtr<ButtonTheme>()));
}

/**
 * @tc.name: IsGlobalAvoidEnabled001
 * @tc.desc: Test IsGlobalAvoidEnabled with valid pipeline and safe area manager
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, IsGlobalAvoidEnabled001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto frameNode = FrameNode::CreateFrameNode(V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    auto result = algo.IsGlobalAvoidEnabled(frameNode);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetMaskRect001
 * @tc.desc: Test GetMaskRect with non-UIExtension subwindow
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, GetMaskRect001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.isUIExtensionSubWindow_ = false;
    auto dialogTheme = AceType::MakeRefPtr<DialogTheme>();
    auto dialog = FrameNode::CreateFrameNode(
        V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<DialogPattern>(dialogTheme, nullptr));
    ASSERT_NE(dialog, nullptr);
    auto maskRect = algo.GetMaskRect(dialog);
    EXPECT_FALSE(maskRect.has_value());
}

/**
 * @tc.name: GetMaskRect002
 * @tc.desc: Test GetMaskRect with UIExtension subwindow and expandDisplay + valid hostWindowRect
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, GetMaskRect002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.isUIExtensionSubWindow_ = true;
    algo.expandDisplay_ = true;
    algo.hostWindowRect_ = RectF(10.0f, 20.0f, 300.0f, 400.0f);
    auto dialogTheme = AceType::MakeRefPtr<DialogTheme>();
    auto dialog = FrameNode::CreateFrameNode(
        V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<DialogPattern>(dialogTheme, nullptr));
    ASSERT_NE(dialog, nullptr);
    auto maskRect = algo.GetMaskRect(dialog);
    ASSERT_TRUE(maskRect.has_value());
    EXPECT_EQ(maskRect->GetWidth().Value(), 300.0f);
    EXPECT_EQ(maskRect->GetHeight().Value(), 400.0f);
}

/**
 * @tc.name: GetMaskRect003
 * @tc.desc: Test GetMaskRect with UIExtension subwindow but no expandDisplay (100% fallback)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, GetMaskRect003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.isUIExtensionSubWindow_ = true;
    algo.expandDisplay_ = false;
    algo.hostWindowRect_ = RectF(0.0f, 0.0f, 0.0f, 0.0f);
    auto dialogTheme = AceType::MakeRefPtr<DialogTheme>();
    auto dialog = FrameNode::CreateFrameNode(
        V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<DialogPattern>(dialogTheme, nullptr));
    ASSERT_NE(dialog, nullptr);
    auto maskRect = algo.GetMaskRect(dialog);
    ASSERT_TRUE(maskRect.has_value());
    EXPECT_EQ(maskRect->GetWidth().Unit(), DimensionUnit::PERCENT);
    EXPECT_EQ(maskRect->GetHeight().Unit(), DimensionUnit::PERCENT);
}

/**
 * @tc.name: CreateDialogChildConstraint001
 * @tc.desc: Test CreateDialogChildConstraint with valid layoutWrapper
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, CreateDialogChildConstraint001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto frameNode = FrameNode::CreateFrameNode(V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        frameNode, frameNode->GetGeometryNode(), frameNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.parentIdealSize = OptionalSizeF(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(layoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();
    auto constraint = algo.CreateDialogChildConstraint(layoutWrapper.rawPtr_, 200.0f, 400.0f);
    EXPECT_EQ(constraint.maxSize.Height(), 200.0f);
    EXPECT_EQ(constraint.maxSize.Width(), 400.0f);
    EXPECT_EQ(constraint.minSize.Height(), 200.0f);
    EXPECT_EQ(constraint.minSize.Width(), 400.0f);
}

/**
 * @tc.name: CreateDialogChildConstraint002
 * @tc.desc: Test CreateDialogChildConstraint with null layoutProperty
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, CreateDialogChildConstraint002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto frameNode = FrameNode::CreateFrameNode(V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        frameNode, frameNode->GetGeometryNode(), frameNode->GetLayoutProperty());
    ASSERT_NE(layoutWrapper, nullptr);
    auto constraint = algo.CreateDialogChildConstraint(layoutWrapper.rawPtr_, 200.0f, 400.0f);
    EXPECT_EQ(constraint.maxSize.Height(), 200.0f);
}

/**
 * @tc.name: UpdateChildLayoutConstraint001
 * @tc.desc: Test UpdateChildLayoutConstraint with positive width and height
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, UpdateChildLayoutConstraint001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto childNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 1, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    ASSERT_NE(childNode, nullptr);
    auto childLayoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        childNode, childNode->GetGeometryNode(), childNode->GetLayoutProperty());
    ASSERT_NE(childLayoutWrapper, nullptr);
    auto dialogProp = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(dialogProp, nullptr);
    dialogProp->UpdateWidth(Dimension(300.0, DimensionUnit::PX));
    dialogProp->UpdateHeight(Dimension(200.0, DimensionUnit::PX));
    LayoutConstraintF childLayoutConstraint;
    childLayoutConstraint.maxSize = SizeF(600.0f, 1000.0f);
    RefPtr<LayoutWrapper> wrapper = childLayoutWrapper;
    algo.UpdateChildLayoutConstraint(dialogProp, childLayoutConstraint, wrapper);
    SUCCEED();
}

/**
 * @tc.name: UpdateChildLayoutConstraint002
 * @tc.desc: Test UpdateChildLayoutConstraint with negative width and height (skipped)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, UpdateChildLayoutConstraint002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto childNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 1, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    ASSERT_NE(childNode, nullptr);
    auto childLayoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        childNode, childNode->GetGeometryNode(), childNode->GetLayoutProperty());
    ASSERT_NE(childLayoutWrapper, nullptr);
    auto dialogProp = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(dialogProp, nullptr);
    dialogProp->UpdateWidth(Dimension(-1.0, DimensionUnit::VP));
    dialogProp->UpdateHeight(Dimension(-1.0, DimensionUnit::VP));
    LayoutConstraintF childLayoutConstraint;
    childLayoutConstraint.maxSize = SizeF(600.0f, 1000.0f);
    RefPtr<LayoutWrapper> wrapper = childLayoutWrapper;
    algo.UpdateChildLayoutConstraint(dialogProp, childLayoutConstraint, wrapper);
}

/**
 * @tc.name: UpdateChildLayoutConstraint003
 * @tc.desc: Test UpdateChildLayoutConstraint with null childLayoutWrapper
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, UpdateChildLayoutConstraint003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto dialogProp = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(dialogProp, nullptr);
    LayoutConstraintF childLayoutConstraint;
    RefPtr<LayoutWrapper> wrapper = nullptr;
    algo.UpdateChildLayoutConstraint(dialogProp, childLayoutConstraint, wrapper);
}

/**
 * @tc.name: UpdateChildLayoutConstraint004
 * @tc.desc: Test UpdateChildLayoutConstraint with null dialogProp
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, UpdateChildLayoutConstraint004, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto childNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 1, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    ASSERT_NE(childNode, nullptr);
    auto childLayoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        childNode, childNode->GetGeometryNode(), childNode->GetLayoutProperty());
    ASSERT_NE(childLayoutWrapper, nullptr);
    LayoutConstraintF childLayoutConstraint;
    RefPtr<LayoutWrapper> wrapper = childLayoutWrapper;
    algo.UpdateChildLayoutConstraint(nullptr, childLayoutConstraint, wrapper);
}

/**
 * @tc.name: UpdateChildMaxSizeHeight001
 * @tc.desc: Test UpdateChildMaxSizeHeight in non-hover mode with gridCount < 0 and API >= 12
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, UpdateChildMaxSizeHeight001, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.isHoverMode_ = false;
    algo.gridCount_ = -1;
    algo.safeAreaInsets_.top_ = SafeAreaInsets::Inset { 0, 50 };
    algo.safeAreaInsets_.bottom_ = SafeAreaInsets::Inset { 950, 1000 };
    SizeT<float> maxSize(600.0f, 1000.0f);
    algo.UpdateChildMaxSizeHeight(maxSize);
    EXPECT_LT(maxSize.Height(), 1000.0f);
}

/**
 * @tc.name: UpdateChildMaxSizeHeight002
 * @tc.desc: Test UpdateChildMaxSizeHeight in hover mode with BOTTOM_SCREEN (alignBottomScreen true)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, UpdateChildMaxSizeHeight002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.isHoverMode_ = true;
    algo.isKeyBoardShow_ = false;
    algo.hoverModeArea_ = HoverModeAreaType::BOTTOM_SCREEN;
    algo.foldCreaseRect = Rect(0.0f, 500.0f, 600.0f, 100.0f);
    algo.safeAreaInsets_.bottom_ = SafeAreaInsets::Inset { 950, 1000 };
    SizeT<float> maxSize(600.0f, 1000.0f);
    algo.UpdateChildMaxSizeHeight(maxSize);
    EXPECT_TRUE(algo.alignBottomScreen_);
}

/**
 * @tc.name: UpdateChildMaxSizeHeight003
 * @tc.desc: Test UpdateChildMaxSizeHeight in hover mode with TOP_SCREEN
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, UpdateChildMaxSizeHeight003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.isHoverMode_ = true;
    algo.hoverModeArea_ = HoverModeAreaType::TOP_SCREEN;
    algo.safeAreaInsets_.top_ = SafeAreaInsets::Inset { 0, 50 };
    algo.foldCreaseRect = Rect(0.0f, 0.0f, 600.0f, 500.0f);
    SizeT<float> maxSize(600.0f, 1000.0f);
    algo.UpdateChildMaxSizeHeight(maxSize);
    EXPECT_LT(maxSize.Height(), 1000.0f);
}

/**
 * @tc.name: UpdateChildMaxSizeHeight004
 * @tc.desc: Test UpdateChildMaxSizeHeight in hover mode with keyboard shown (alignBottomScreen false)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, UpdateChildMaxSizeHeight004, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.isHoverMode_ = true;
    algo.isKeyBoardShow_ = true;
    algo.hoverModeArea_ = HoverModeAreaType::BOTTOM_SCREEN;
    algo.safeAreaInsets_.top_ = SafeAreaInsets::Inset { 0, 50 };
    algo.foldCreaseRect = Rect(0.0f, 0.0f, 600.0f, 500.0f);
    SizeT<float> maxSize(600.0f, 1000.0f);
    algo.UpdateChildMaxSizeHeight(maxSize);
    EXPECT_FALSE(algo.alignBottomScreen_);
}

/**
 * @tc.name: SetAlignmentSwitch001
 * @tc.desc: Test SetAlignmentSwitch with TOP alignment
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, SetAlignmentSwitch001, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.alignment_ = DialogAlignment::TOP;
    algo.customSize_ = true;
    SizeF maxSize(600.0f, 1000.0f);
    SizeF childSize(200.0f, 200.0f);
    OffsetF topLeftPoint;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    EXPECT_EQ(topLeftPoint.GetX(), 200.0f);
    EXPECT_EQ(topLeftPoint.GetY(), 0.0f);
}

/**
 * @tc.name: SetAlignmentSwitch002
 * @tc.desc: Test SetAlignmentSwitch with CENTER alignment
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, SetAlignmentSwitch002, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.alignment_ = DialogAlignment::CENTER;
    algo.customSize_ = true;
    SizeF maxSize(600.0f, 1000.0f);
    SizeF childSize(200.0f, 200.0f);
    OffsetF topLeftPoint;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    EXPECT_EQ(topLeftPoint.GetX(), 200.0f);
    EXPECT_EQ(topLeftPoint.GetY(), 400.0f);
}

/**
 * @tc.name: SetAlignmentSwitch003
 * @tc.desc: Test SetAlignmentSwitch with BOTTOM alignment
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, SetAlignmentSwitch003, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.alignment_ = DialogAlignment::BOTTOM;
    algo.customSize_ = true;
    SizeF maxSize(600.0f, 1000.0f);
    SizeF childSize(200.0f, 200.0f);
    OffsetF topLeftPoint;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    EXPECT_EQ(topLeftPoint.GetX(), 200.0f);
    EXPECT_EQ(topLeftPoint.GetY(), 800.0f);
}

/**
 * @tc.name: SetAlignmentSwitch004
 * @tc.desc: Test SetAlignmentSwitch with TOP_START and TOP_END alignments
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, SetAlignmentSwitch004, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.customSize_ = true;
    SizeF maxSize(600.0f, 1000.0f);
    SizeF childSize(200.0f, 200.0f);
    OffsetF topLeftPoint;

    algo.alignment_ = DialogAlignment::TOP_START;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    EXPECT_EQ(topLeftPoint.GetX(), 0.0f);
    EXPECT_EQ(topLeftPoint.GetY(), 0.0f);

    algo.alignment_ = DialogAlignment::TOP_END;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    EXPECT_EQ(topLeftPoint.GetX(), 400.0f);
    EXPECT_EQ(topLeftPoint.GetY(), 0.0f);
}

/**
 * @tc.name: SetAlignmentSwitch005
 * @tc.desc: Test SetAlignmentSwitch with CENTER_START and CENTER_END alignments
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, SetAlignmentSwitch005, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.customSize_ = true;
    SizeF maxSize(600.0f, 1000.0f);
    SizeF childSize(200.0f, 200.0f);
    OffsetF topLeftPoint;

    algo.alignment_ = DialogAlignment::CENTER_START;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    EXPECT_EQ(topLeftPoint.GetX(), 0.0f);
    EXPECT_EQ(topLeftPoint.GetY(), 400.0f);

    algo.alignment_ = DialogAlignment::CENTER_END;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    EXPECT_EQ(topLeftPoint.GetX(), 400.0f);
    EXPECT_EQ(topLeftPoint.GetY(), 400.0f);
}

/**
 * @tc.name: SetAlignmentSwitch006
 * @tc.desc: Test SetAlignmentSwitch with BOTTOM_START and BOTTOM_END alignments
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, SetAlignmentSwitch006, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.customSize_ = true;
    SizeF maxSize(600.0f, 1000.0f);
    SizeF childSize(200.0f, 200.0f);
    OffsetF topLeftPoint;

    algo.alignment_ = DialogAlignment::BOTTOM_START;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    EXPECT_EQ(topLeftPoint.GetX(), 0.0f);
    EXPECT_EQ(topLeftPoint.GetY(), 800.0f);

    algo.alignment_ = DialogAlignment::BOTTOM_END;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    EXPECT_EQ(topLeftPoint.GetX(), 400.0f);
    EXPECT_EQ(topLeftPoint.GetY(), 800.0f);
}

/**
 * @tc.name: SetAlignmentSwitch007
 * @tc.desc: Test SetAlignmentSwitch with alignBottomScreen_ true
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, SetAlignmentSwitch007, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.alignment_ = DialogAlignment::TOP;
    algo.customSize_ = true;
    algo.alignBottomScreen_ = true;
    SizeF maxSize(600.0f, 1000.0f);
    SizeF childSize(200.0f, 200.0f);
    OffsetF topLeftPoint;
    auto halfScreenHeight = maxSize.Height() / 2.0;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    EXPECT_EQ(topLeftPoint.GetY(), halfScreenHeight);
}

/**
 * @tc.name: SetAlignmentSwitch008
 * @tc.desc: Test SetAlignmentSwitch with DEFAULT alignment and API >= 12 (enters switch default case)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, SetAlignmentSwitch008, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.alignment_ = DialogAlignment::DEFAULT;
    algo.customSize_ = true;
    SizeF maxSize(600.0f, 1000.0f);
    SizeF childSize(200.0f, 200.0f);
    OffsetF topLeftPoint;
    EXPECT_TRUE(algo.SetAlignmentSwitch(maxSize, childSize, topLeftPoint));
    EXPECT_EQ(topLeftPoint.GetX(), 200.0f);
    EXPECT_EQ(topLeftPoint.GetY(), 400.0f);
}

/**
 * @tc.name: AdjustHoverModeForWaterfall001
 * @tc.desc: Test AdjustHoverModeForWaterfall with expandDisplay_ false (early return)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, AdjustHoverModeForWaterfall001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.expandDisplay_ = false;
    auto frameNode = FrameNode::CreateFrameNode(V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    algo.isHoverMode_ = false;
    algo.AdjustHoverModeForWaterfall(frameNode);
    EXPECT_FALSE(algo.isHoverMode_);
}

/**
 * @tc.name: AdjustHoverModeForWaterfall002
 * @tc.desc: Test AdjustHoverModeForWaterfall with expandDisplay_ true but null pattern
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, AdjustHoverModeForWaterfall002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.expandDisplay_ = true;
    auto frameNode = FrameNode::CreateFrameNode(V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    algo.isHoverMode_ = false;
    algo.AdjustHoverModeForWaterfall(frameNode);
    EXPECT_FALSE(algo.isHoverMode_);
}

/**
 * @tc.name: AdjustHoverModeForWaterfall003
 * @tc.desc: Test AdjustHoverModeForWaterfall with expandDisplay_ true, pattern and prop valid
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, AdjustHoverModeForWaterfall003, TestSize.Level1)
{
    SetDialogTheme();
    DialogLayoutAlgorithm algo;
    algo.expandDisplay_ = true;
    auto dialogTheme = AceType::MakeRefPtr<DialogTheme>();
    auto dialogProp = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(dialogProp, nullptr);
    auto frameNode = FrameNode::CreateFrameNode(
        V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<DialogPattern>(dialogTheme, nullptr));
    ASSERT_NE(frameNode, nullptr);
    frameNode->layoutProperty_ = dialogProp;
    algo.isHoverMode_ = false;
    algo.AdjustHoverModeForWaterfall(frameNode);
    SUCCEED();
}

/**
 * @tc.name: ComputeHandednessOffset001
 * @tc.desc: Test ComputeHandednessOffset with null dialogProp
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, ComputeHandednessOffset001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    SizeF childSize(200.0f, 200.0f);
    SizeF selfSize(600.0f, 1000.0f);
    auto result = algo.ComputeHandednessOffset(childSize, selfSize, nullptr, nullptr);
    EXPECT_FALSE(result.first);
}

/**
 * @tc.name: ComputeHandednessOffset002
 * @tc.desc: Test ComputeHandednessOffset with NONE mode (returns false)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, ComputeHandednessOffset002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.handednessMode_ = HandednessFollowMode::NONE;
    algo.dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
    auto dialogProp = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(dialogProp, nullptr);
    auto dialogPattern = AceType::MakeRefPtr<DialogPattern>(algo.dialogTheme_, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    SizeF childSize(200.0f, 200.0f);
    SizeF selfSize(600.0f, 1000.0f);
    auto result = algo.ComputeHandednessOffset(childSize, selfSize, dialogProp, dialogPattern);
    EXPECT_FALSE(result.first);
}

/**
 * @tc.name: ComputeHandednessOffset003
 * @tc.desc: Test ComputeHandednessOffset with FOLLOW_ANCHOR but child too large (degrades)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, ComputeHandednessOffset003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.handednessMode_ = HandednessFollowMode::FOLLOW_ANCHOR;
    algo.dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
    algo.safeAreaInsets_.top_ = SafeAreaInsets::Inset { 0, 0 };
    algo.safeAreaInsets_.bottom_ = SafeAreaInsets::Inset { 0, 0 };
    auto dialogProp = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(dialogProp, nullptr);
    auto dialogPattern = AceType::MakeRefPtr<DialogPattern>(algo.dialogTheme_, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    SizeF childSize(800.0f, 1200.0f);
    SizeF selfSize(600.0f, 1000.0f);
    auto result = algo.ComputeHandednessOffset(childSize, selfSize, dialogProp, dialogPattern);
    EXPECT_FALSE(result.first);
}

/**
 * @tc.name: ComputeHandednessOffset004
 * @tc.desc: Test ComputeHandednessOffset with FOLLOW_GRIP_HAND but child too large (degrades)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, ComputeHandednessOffset004, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.handednessMode_ = HandednessFollowMode::FOLLOW_GRIP_HAND;
    algo.dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
    algo.safeAreaInsets_.top_ = SafeAreaInsets::Inset { 0, 0 };
    algo.safeAreaInsets_.bottom_ = SafeAreaInsets::Inset { 0, 0 };
    auto dialogProp = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(dialogProp, nullptr);
    auto dialogPattern = AceType::MakeRefPtr<DialogPattern>(algo.dialogTheme_, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    SizeF childSize(800.0f, 1200.0f);
    SizeF selfSize(600.0f, 1000.0f);
    auto result = algo.ComputeHandednessOffset(childSize, selfSize, dialogProp, dialogPattern);
    EXPECT_FALSE(result.first);
}

/**
 * @tc.name: ComputeHandednessOffset005
 * @tc.desc: Test ComputeHandednessOffset with null dialogPattern
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, ComputeHandednessOffset005, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.handednessMode_ = HandednessFollowMode::FOLLOW_ANCHOR;
    algo.dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
    auto dialogProp = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(dialogProp, nullptr);
    SizeF childSize(200.0f, 200.0f);
    SizeF selfSize(600.0f, 1000.0f);
    auto result = algo.ComputeHandednessOffset(childSize, selfSize, dialogProp, nullptr);
    EXPECT_FALSE(result.first);
}

/**
 * @tc.name: UpdateInnerScrollAndListMaxHeight001
 * @tc.desc: Test UpdateInnerScrollAndListMaxHeight with both scroll and list
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, UpdateInnerScrollAndListMaxHeight001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto scrollNode = FrameNode::CreateFrameNode("Scroll", 1, AceType::MakeRefPtr<Pattern>());
    auto listNode = FrameNode::CreateFrameNode("List", 2, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(scrollNode, nullptr);
    ASSERT_NE(listNode, nullptr);
    auto scrollWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        scrollNode, scrollNode->GetGeometryNode(), scrollNode->GetLayoutProperty());
    auto listWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        listNode, listNode->GetGeometryNode(), listNode->GetLayoutProperty());
    ASSERT_NE(scrollWrapper, nullptr);
    ASSERT_NE(listWrapper, nullptr);
    float scrollHeight = 300.0f;
    float listHeight = 200.0f;
    float restHeight = 500.0f;
    algo.UpdateInnerScrollAndListMaxHeight(scrollWrapper, listWrapper, scrollHeight, listHeight, restHeight);
    SUCCEED();
}

/**
 * @tc.name: UpdateInnerScrollAndListMaxHeight002
 * @tc.desc: Test UpdateInnerScrollAndListMaxHeight with only scroll
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, UpdateInnerScrollAndListMaxHeight002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto scrollNode = FrameNode::CreateFrameNode("Scroll", 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(scrollNode, nullptr);
    auto scrollWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        scrollNode, scrollNode->GetGeometryNode(), scrollNode->GetLayoutProperty());
    ASSERT_NE(scrollWrapper, nullptr);
    float scrollHeight = 300.0f;
    float restHeight = 500.0f;
    algo.UpdateInnerScrollAndListMaxHeight(scrollWrapper, nullptr, scrollHeight, 0.0f, restHeight);
    SUCCEED();
}

/**
 * @tc.name: UpdateInnerScrollAndListMaxHeight003
 * @tc.desc: Test UpdateInnerScrollAndListMaxHeight with only list
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, UpdateInnerScrollAndListMaxHeight003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto listNode = FrameNode::CreateFrameNode("List", 2, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(listNode, nullptr);
    auto listWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        listNode, listNode->GetGeometryNode(), listNode->GetLayoutProperty());
    ASSERT_NE(listWrapper, nullptr);
    float listHeight = 200.0f;
    float restHeight = 500.0f;
    algo.UpdateInnerScrollAndListMaxHeight(nullptr, listWrapper, 0.0f, listHeight, restHeight);
    SUCCEED();
}

/**
 * @tc.name: CaculateMaxSize001
 * @tc.desc: Test CaculateMaxSize with customSize true (no height change from hover/alignment)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, CaculateMaxSize001, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.customSize_ = true;
    algo.isHoverMode_ = false;
    algo.alignment_ = DialogAlignment::BOTTOM;
    SizeF maxSize(600.0f, 1000.0f);
    auto originalHeight = maxSize.Height();
    algo.CaculateMaxSize(maxSize);
    EXPECT_EQ(maxSize.Height(), originalHeight);
}

/**
 * @tc.name: CaculateMaxSize002
 * @tc.desc: Test CaculateMaxSize with customSize false, hoverMode true (half screen height)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, CaculateMaxSize002, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.customSize_ = false;
    algo.isHoverMode_ = true;
    algo.alignment_ = DialogAlignment::CENTER;
    algo.hoverModeArea_ = HoverModeAreaType::BOTTOM_SCREEN;
    SizeF maxSize(600.0f, 1000.0f);
    algo.CaculateMaxSize(maxSize);
    EXPECT_EQ(maxSize.Height(), 500.0f);
}

/**
 * @tc.name: CaculateMaxSize003
 * @tc.desc: Test CaculateMaxSize with customSize false, non-whole-screen alignment, hoverMode TOP_SCREEN
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, CaculateMaxSize003, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.customSize_ = false;
    algo.isHoverMode_ = true;
    algo.alignment_ = DialogAlignment::TOP;
    algo.hoverModeArea_ = HoverModeAreaType::TOP_SCREEN;
    algo.foldCreaseRect = Rect(0.0f, 400.0f, 600.0f, 200.0f);
    SizeF maxSize(600.0f, 1000.0f);
    algo.CaculateMaxSize(maxSize);
    EXPECT_EQ(maxSize.Height(), 400.0f);
}

/**
 * @tc.name: CaculateMaxSize004
 * @tc.desc: Test CaculateMaxSize with customSize false, non-whole-screen alignment, no hover
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, CaculateMaxSize004, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.customSize_ = false;
    algo.isHoverMode_ = false;
    algo.alignment_ = DialogAlignment::BOTTOM;
    algo.safeAreaBottomLength_ = 50.0f;
    SizeF maxSize(600.0f, 1000.0f);
    algo.CaculateMaxSize(maxSize);
    EXPECT_EQ(maxSize.Height(), 950.0f);
}

/**
 * @tc.name: ParseSubwindowId001
 * @tc.desc: Test ParseSubwindowId with showInSubWindow false
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, ParseSubwindowId001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    algo.ParseSubwindowId(prop);
    EXPECT_EQ(algo.subWindowId_, -1);
}

/**
 * @tc.name: ParseSubwindowId002
 * @tc.desc: Test ParseSubwindowId with showInSubWindow true but null host
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, ParseSubwindowId002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.subWindowId_ = -1;
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    prop->UpdateShowInSubWindow(true);
    algo.ParseSubwindowId(prop);
    EXPECT_NE(algo.subWindowId_, -1);
}

/**
 * @tc.name: SetSubWindowHotarea001
 * @tc.desc: Test SetSubWindowHotarea with showInSubWindow false (early return)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, SetSubWindowHotarea001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    SizeF childSize(200.0f, 200.0f);
    SizeF selfSize(600.0f, 1000.0f);
    algo.SetSubWindowHotarea(prop, childSize, selfSize, 1);
    SUCCEED();
}

/**
 * @tc.name: SetSubWindowHotarea002
 * @tc.desc: Test SetSubWindowHotarea with showInSubWindow true, non-sceneBoard dialog
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, SetSubWindowHotarea002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.topLeftPoint_ = OffsetF(100.0f, 200.0f);
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    prop->UpdateShowInSubWindow(true);
    SizeF childSize(200.0f, 200.0f);
    SizeF selfSize(600.0f, 1000.0f);
    algo.SetSubWindowHotarea(prop, childSize, selfSize, 1);
    SUCCEED();
}

/**
 * @tc.name: SetSubWindowHotarea003
 * @tc.desc: Test SetSubWindowHotarea with sceneBoard dialog
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, SetSubWindowHotarea003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    prop->UpdateShowInSubWindow(true);
    prop->UpdateIsSceneBoardDialog(true);
    SizeF childSize(200.0f, 200.0f);
    SizeF selfSize(600.0f, 1000.0f);
    algo.SetSubWindowHotarea(prop, childSize, selfSize, 1);
    SUCCEED();
}

/**
 * @tc.name: SetSubWindowHotarea004
 * @tc.desc: Test SetSubWindowHotarea with UIExtension subwindow and modal
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, SetSubWindowHotarea004, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.isUIExtensionSubWindow_ = true;
    algo.isModal_ = true;
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    prop->UpdateShowInSubWindow(true);
    SizeF childSize(200.0f, 200.0f);
    SizeF selfSize(600.0f, 1000.0f);
    algo.SetSubWindowHotarea(prop, childSize, selfSize, 1);
    SUCCEED();
}

/**
 * @tc.name: ClipCustomMaskNode001
 * @tc.desc: Test ClipCustomMaskNode with null child at index 1
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, ClipCustomMaskNode001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto dialog = FrameNode::CreateFrameNode(V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(dialog, nullptr);
    RectF rect(0.0f, 0.0f, 600.0f, 1000.0f);
    algo.ClipCustomMaskNode(dialog, rect);
    SUCCEED();
}

/**
 * @tc.name: ClipCustomMaskNode002
 * @tc.desc: Test ClipCustomMaskNode with valid mask node child
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, ClipCustomMaskNode002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto dialog = FrameNode::CreateFrameNode(V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(dialog, nullptr);
    auto maskNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 2, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(maskNode, nullptr);
    dialog->AddChild(maskNode);
    RectF rect(0.0f, 0.0f, 600.0f, 1000.0f);
    algo.ClipCustomMaskNode(dialog, rect);
    SUCCEED();
}

/**
 * @tc.name: UpdateCustomMaskNodeLayout001
 * @tc.desc: Test UpdateCustomMaskNodeLayout with no child at index 1
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, UpdateCustomMaskNodeLayout001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto dialog = FrameNode::CreateFrameNode(V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(dialog, nullptr);
    algo.UpdateCustomMaskNodeLayout(dialog);
    SUCCEED();
}

/**
 * @tc.name: UpdateCustomMaskNodeLayout002
 * @tc.desc: Test UpdateCustomMaskNodeLayout with valid mask node
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, UpdateCustomMaskNodeLayout002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto dialog = FrameNode::CreateFrameNode(V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(dialog, nullptr);
    auto maskNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 2, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(maskNode, nullptr);
    dialog->AddChild(maskNode);
    algo.UpdateCustomMaskNodeLayout(dialog);
    SUCCEED();
}

/**
 * @tc.name: ProcessMaskRect002
 * @tc.desc: Test ProcessMaskRect with negative width and height (full screen fallback)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, ProcessMaskRect002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto dialog = FrameNode::CreateFrameNode(V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(dialog, nullptr);
    auto maskRect = DimensionRect(
        Dimension(-1.0f, DimensionUnit::VP), Dimension(-1.0f, DimensionUnit::VP), DimensionOffset(OffsetF(0.0f, 0.0f)));
    algo.ProcessMaskRect(maskRect, dialog, false);
    SUCCEED();
}

/**
 * @tc.name: ProcessMaskRect003
 * @tc.desc: Test ProcessMaskRect with isModal true, isMask true, not full screen, not subwindow, no mask node
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, ProcessMaskRect003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.isModal_ = true;
    algo.isShowInSubWindow_ = false;
    algo.hasAddMaskNode_ = false;
    auto dialogTheme = AceType::MakeRefPtr<DialogTheme>();
    auto dialog = FrameNode::CreateFrameNode(
        V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<DialogPattern>(dialogTheme, nullptr));
    ASSERT_NE(dialog, nullptr);
    auto maskRect = DimensionRect(
        Dimension(300.0f), Dimension(400.0f), DimensionOffset(OffsetF(10.0f, 20.0f)));
    algo.ProcessMaskRect(maskRect, dialog, true);
    SUCCEED();
}

/**
 * @tc.name: ProcessMaskRect004
 * @tc.desc: Test ProcessMaskRect with hasAddMaskNode true
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFourTestNg, ProcessMaskRect004, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.isModal_ = true;
    algo.isShowInSubWindow_ = false;
    algo.hasAddMaskNode_ = true;
    auto dialogTheme = AceType::MakeRefPtr<DialogTheme>();
    auto dialog = FrameNode::CreateFrameNode(
        V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<DialogPattern>(dialogTheme, nullptr));
    ASSERT_NE(dialog, nullptr);
    auto maskNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 2, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(maskNode, nullptr);
    dialog->AddChild(FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 0, AceType::MakeRefPtr<Pattern>()));
    dialog->AddChild(maskNode);
    auto maskRect = DimensionRect(
        Dimension(300.0f), Dimension(400.0f), DimensionOffset(OffsetF(10.0f, 20.0f)));
    algo.ProcessMaskRect(maskRect, dialog, true);
    SUCCEED();
}
} // namespace OHOS::Ace::NG
