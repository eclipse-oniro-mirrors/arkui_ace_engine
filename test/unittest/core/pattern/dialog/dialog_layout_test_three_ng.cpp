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
} // namespace

class DialogLayoutThreeTestNg : public testing::Test {
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

void DialogLayoutThreeTestNg::SetUpTestCase()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    MockContainer::Current()->pipelineContext_ = MockPipelineContext::GetCurrentContext();
    MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
}

void DialogLayoutThreeTestNg::TearDownTestCase()
{
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

void DialogLayoutThreeTestNg::SetDialogTheme()
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
 * @tc.name: GetDeviceColumns001
 * @tc.desc: Test DialogLayoutAlgorithm::GetDeviceColumns for WATCH device type
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, GetDeviceColumns001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    EXPECT_EQ(algo.GetDeviceColumns(GridSizeType::SM, DeviceType::WATCH), 3);
    EXPECT_EQ(algo.GetDeviceColumns(GridSizeType::MD, DeviceType::WATCH), 4);
    EXPECT_EQ(algo.GetDeviceColumns(GridSizeType::LG, DeviceType::WATCH), 5);
}

/**
 * @tc.name: GetDeviceColumns002
 * @tc.desc: Test DialogLayoutAlgorithm::GetDeviceColumns for PHONE device type
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, GetDeviceColumns002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    EXPECT_EQ(algo.GetDeviceColumns(GridSizeType::SM, DeviceType::PHONE), 4);
    EXPECT_EQ(algo.GetDeviceColumns(GridSizeType::MD, DeviceType::PHONE), 5);
    EXPECT_EQ(algo.GetDeviceColumns(GridSizeType::LG, DeviceType::PHONE), 6);
}

/**
 * @tc.name: GetDeviceColumns003
 * @tc.desc: Test DialogLayoutAlgorithm::GetDeviceColumns for CAR device type
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, GetDeviceColumns003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    EXPECT_EQ(algo.GetDeviceColumns(GridSizeType::SM, DeviceType::CAR), 4);
    EXPECT_EQ(algo.GetDeviceColumns(GridSizeType::MD, DeviceType::CAR), 6);
    EXPECT_EQ(algo.GetDeviceColumns(GridSizeType::LG, DeviceType::CAR), 8);
}

/**
 * @tc.name: GetDeviceColumns004
 * @tc.desc: Test DialogLayoutAlgorithm::GetDeviceColumns for TABLET device type with API >= 11
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, GetDeviceColumns004, TestSize.Level1)
{
    auto container = Container::Current();
    ASSERT_NE(container, nullptr);
    container->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN);

    DialogLayoutAlgorithm algo;
    EXPECT_EQ(algo.GetDeviceColumns(GridSizeType::MD, DeviceType::TABLET), 5);
}

/**
 * @tc.name: GetDeviceColumns005
 * @tc.desc: Test DialogLayoutAlgorithm::GetDeviceColumns for default device type
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, GetDeviceColumns005, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    EXPECT_EQ(algo.GetDeviceColumns(GridSizeType::SM, DeviceType::UNKNOWN), 2);
    EXPECT_EQ(algo.GetDeviceColumns(GridSizeType::MD, DeviceType::UNKNOWN), 3);
    EXPECT_EQ(algo.GetDeviceColumns(GridSizeType::LG, DeviceType::UNKNOWN), 4);
}

/**
 * @tc.name: GetMaxWidthBasedOnGridType001
 * @tc.desc: Test GetMaxWidthBasedOnGridType with gridCount_ >= 0
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, GetMaxWidthBasedOnGridType001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.gridCount_ = 2;
    GridContainerInfo::Builder parentBuilder;
    parentBuilder.SetColumns(8);
    auto parent = parentBuilder.Build();
    ASSERT_NE(parent, nullptr);
    GridColumnInfo::Builder builder;
    builder.SetColumns(8);
    builder.SetParent(parent);
    auto info = builder.Build();
    ASSERT_NE(info, nullptr);
    auto result = algo.GetMaxWidthBasedOnGridType(info, GridSizeType::SM, DeviceType::PHONE);
    EXPECT_GE(result, 0.0);
}

/**
 * @tc.name: GetMaxWidthBasedOnGridType002
 * @tc.desc: Test GetMaxWidthBasedOnGridType with gridCount_ < 0 (uses GetDeviceColumns)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, GetMaxWidthBasedOnGridType002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.gridCount_ = -1;
    GridContainerInfo::Builder parentBuilder;
    parentBuilder.SetColumns(8);
    auto parent = parentBuilder.Build();
    ASSERT_NE(parent, nullptr);
    GridColumnInfo::Builder builder;
    builder.SetColumns(8);
    builder.SetParent(parent);
    auto info = builder.Build();
    ASSERT_NE(info, nullptr);
    auto result = algo.GetMaxWidthBasedOnGridType(info, GridSizeType::SM, DeviceType::PHONE);
    EXPECT_GE(result, 0.0);
}

/**
 * @tc.name: Distribute001
 * @tc.desc: Test Distribute when sum <= restHeight (no distribution needed)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, Distribute001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    float scrollHeight = 100.0f;
    float listHeight = 50.0f;
    float restHeight = 200.0f;
    algo.Distribute(scrollHeight, listHeight, restHeight);
    EXPECT_EQ(scrollHeight, 100.0f);
    EXPECT_EQ(listHeight, 50.0f);
}

/**
 * @tc.name: Distribute002
 * @tc.desc: Test Distribute when both scroll and list exceed restHeight/2
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, Distribute002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    float scrollHeight = 150.0f;
    float listHeight = 120.0f;
    float restHeight = 200.0f;
    algo.Distribute(scrollHeight, listHeight, restHeight);
    EXPECT_EQ(scrollHeight, 100.0f);
    EXPECT_EQ(listHeight, 100.0f);
}

/**
 * @tc.name: Distribute003
 * @tc.desc: Test Distribute when only scroll exceeds restHeight/2
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, Distribute003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    float scrollHeight = 175.0f;
    float listHeight = 30.0f;
    float restHeight = 200.0f;
    algo.Distribute(scrollHeight, listHeight, restHeight);
    EXPECT_EQ(scrollHeight, 170.0f);
    EXPECT_EQ(listHeight, 30.0f);
}

/**
 * @tc.name: Distribute004
 * @tc.desc: Test Distribute when only list exceeds restHeight/2
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, Distribute004, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    float scrollHeight = 30.0f;
    float listHeight = 175.0f;
    float restHeight = 200.0f;
    algo.Distribute(scrollHeight, listHeight, restHeight);
    EXPECT_EQ(scrollHeight, 30.0f);
    EXPECT_EQ(listHeight, 170.0f);
}

/**
 * @tc.name: IsHandednessActive001
 * @tc.desc: Test IsHandednessActive with NONE mode
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, IsHandednessActive001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.handednessMode_ = HandednessFollowMode::NONE;
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    EXPECT_FALSE(algo.IsHandednessActive(prop));
}

/**
 * @tc.name: IsHandednessActive002
 * @tc.desc: Test IsHandednessActive with FOLLOW_ANCHOR and default position (no custom style)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, IsHandednessActive002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.handednessMode_ = HandednessFollowMode::FOLLOW_ANCHOR;
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    prop->UpdateDialogAlignment(DialogAlignment::DEFAULT);
    prop->UpdateDialogOffset(DimensionOffset(Dimension(0.0), Dimension(0.0)));
    EXPECT_TRUE(algo.IsHandednessActive(prop));
}

/**
 * @tc.name: IsHandednessActive003
 * @tc.desc: Test IsHandednessActive with FOLLOW_ANCHOR but non-default position
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, IsHandednessActive003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.handednessMode_ = HandednessFollowMode::FOLLOW_ANCHOR;
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    prop->UpdateDialogAlignment(DialogAlignment::TOP);
    EXPECT_FALSE(algo.IsHandednessActive(prop));
}

/**
 * @tc.name: IsHandednessActive004
 * @tc.desc: Test IsHandednessActive with FOLLOW_ANCHOR, default position, but custom style
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, IsHandednessActive004, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.handednessMode_ = HandednessFollowMode::FOLLOW_ANCHOR;
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    prop->UpdateDialogAlignment(DialogAlignment::DEFAULT);
    prop->UpdateDialogOffset(DimensionOffset(Dimension(0.0), Dimension(0.0)));
    prop->UpdateUseCustomStyle(true);
    EXPECT_FALSE(algo.IsHandednessActive(prop));
}

/**
 * @tc.name: IsHandednessActive005
 * @tc.desc: Test IsHandednessActive with null dialogProp
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, IsHandednessActive005, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.handednessMode_ = HandednessFollowMode::FOLLOW_ANCHOR;
    EXPECT_FALSE(algo.IsHandednessActive(nullptr));
}

/**
 * @tc.name: IsAlignmentByWholeScreen001
 * @tc.desc: Test IsAlignmentByWholeScreen with API < 12
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, IsAlignmentByWholeScreen001, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN);
    DialogLayoutAlgorithm algo;
    algo.alignment_ = DialogAlignment::CENTER;
    EXPECT_FALSE(algo.IsAlignmentByWholeScreen());
}

/**
 * @tc.name: IsAlignmentByWholeScreen002
 * @tc.desc: Test IsAlignmentByWholeScreen with API >= 12, TOP/BOTTOM alignments
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, IsAlignmentByWholeScreen002, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.alignment_ = DialogAlignment::TOP;
    EXPECT_FALSE(algo.IsAlignmentByWholeScreen());
    algo.alignment_ = DialogAlignment::BOTTOM;
    EXPECT_FALSE(algo.IsAlignmentByWholeScreen());
    algo.alignment_ = DialogAlignment::TOP_START;
    EXPECT_FALSE(algo.IsAlignmentByWholeScreen());
    algo.alignment_ = DialogAlignment::BOTTOM_END;
    EXPECT_FALSE(algo.IsAlignmentByWholeScreen());
}

/**
 * @tc.name: IsAlignmentByWholeScreen003
 * @tc.desc: Test IsAlignmentByWholeScreen with API >= 12, CENTER/DEFAULT alignments
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, IsAlignmentByWholeScreen003, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.alignment_ = DialogAlignment::CENTER;
    EXPECT_TRUE(algo.IsAlignmentByWholeScreen());
    algo.alignment_ = DialogAlignment::DEFAULT;
    EXPECT_TRUE(algo.IsAlignmentByWholeScreen());
    algo.alignment_ = DialogAlignment::CENTER_START;
    EXPECT_TRUE(algo.IsAlignmentByWholeScreen());
}

/**
 * @tc.name: IsDialogTouchingBoundary001
 * @tc.desc: Test IsDialogTouchingBoundary when dialog touches bottom boundary
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, IsDialogTouchingBoundary001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    OffsetF topLeftPoint(0.0f, 900.0f);
    SizeF childSize(100.0f, 200.0f);
    SizeF selfSize(600.0f, 1000.0f);
    EXPECT_TRUE(algo.IsDialogTouchingBoundary(topLeftPoint, childSize, selfSize));
    EXPECT_EQ(algo.touchingBoundaryFlag_, TouchingBoundaryType::TouchBottomBoundary);
}

/**
 * @tc.name: IsDialogTouchingBoundary002
 * @tc.desc: Test IsDialogTouchingBoundary when dialog touches right boundary
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, IsDialogTouchingBoundary002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    OffsetF topLeftPoint(550.0f, 0.0f);
    SizeF childSize(100.0f, 200.0f);
    SizeF selfSize(600.0f, 1000.0f);
    EXPECT_TRUE(algo.IsDialogTouchingBoundary(topLeftPoint, childSize, selfSize));
    EXPECT_EQ(algo.touchingBoundaryFlag_, TouchingBoundaryType::TouchRightBoundary);
}

/**
 * @tc.name: IsDialogTouchingBoundary003
 * @tc.desc: Test IsDialogTouchingBoundary when dialog does not touch any boundary
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, IsDialogTouchingBoundary003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    OffsetF topLeftPoint(100.0f, 100.0f);
    SizeF childSize(100.0f, 200.0f);
    SizeF selfSize(600.0f, 1000.0f);
    EXPECT_FALSE(algo.IsDialogTouchingBoundary(topLeftPoint, childSize, selfSize));
    EXPECT_EQ(algo.touchingBoundaryFlag_, TouchingBoundaryType::NotTouchBoundary);
}

/**
 * @tc.name: GetStackRootDialogOffsetY001
 * @tc.desc: Test GetStackRootDialogOffsetY with null parent
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, GetStackRootDialogOffsetY001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto node = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(node, nullptr);
    node->parent_ = nullptr;
    EXPECT_EQ(algo.GetStackRootDialogOffsetY(node), 0.0f);
}

/**
 * @tc.name: GetStackRootDialogOffsetY002
 * @tc.desc: Test GetStackRootDialogOffsetY with non-STACK parent tag
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, GetStackRootDialogOffsetY002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto parent = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(parent, nullptr);
    auto child = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 2, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(child, nullptr);
    child->MountToParent(parent);
    algo.expandDisplay_ = true;
    EXPECT_EQ(algo.GetStackRootDialogOffsetY(child), 0.0f);
}

/**
 * @tc.name: GetStackRootDialogOffsetY003
 * @tc.desc: Test GetStackRootDialogOffsetY with STACK parent tag and expandDisplay_ true
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, GetStackRootDialogOffsetY003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.expandDisplay_ = true;
    auto parent = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(parent, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetFrameOffset(OffsetF(0.0f, 50.0f));
    parent->geometryNode_ = geometryNode;
    auto child = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 2, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(child, nullptr);
    child->MountToParent(parent);
    EXPECT_EQ(algo.GetStackRootDialogOffsetY(child), 50.0f);
}

/**
 * @tc.name: GetStackRootDialogOffsetY004
 * @tc.desc: Test GetStackRootDialogOffsetY with STACK parent tag but expandDisplay_ false
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, GetStackRootDialogOffsetY004, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.expandDisplay_ = false;
    auto parent = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(parent, nullptr);
    auto child = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 2, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(child, nullptr);
    child->MountToParent(parent);
    EXPECT_EQ(algo.GetStackRootDialogOffsetY(child), 0.0f);
}

/**
 * @tc.name: DialogOverflowAdjust001
 * @tc.desc: Test DialogOverflowAdjust with dialogCorrectionEnabled false (early return)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, DialogOverflowAdjust001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.wrapperSize_ = SizeF(600.0f, 1000.0f);
    OffsetF childOffset(100.0f, 100.0f);
    SizeF childSize(200.0f, 200.0f);
    algo.DialogOverflowAdjust(childOffset, childSize, false);
    EXPECT_EQ(childOffset.GetX(), 100.0f);
    EXPECT_EQ(childOffset.GetY(), 100.0f);
}

/**
 * @tc.name: DialogOverflowAdjust002
 * @tc.desc: Test DialogOverflowAdjust with limitTop > limitBottom (early return)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, DialogOverflowAdjust002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.wrapperSize_ = SizeF(600.0f, 100.0f);
    algo.safeAreaInsets_.top_ = SafeAreaInsets::Inset { 0, 100 };
    algo.safeAreaInsets_.bottom_ = SafeAreaInsets::Inset { 50, 100 };
    OffsetF childOffset(100.0f, 50.0f);
    SizeF childSize(200.0f, 200.0f);
    algo.DialogOverflowAdjust(childOffset, childSize, true);
    EXPECT_EQ(childOffset.GetY(), 50.0f);
}

/**
 * @tc.name: DialogOverflowAdjust003
 * @tc.desc: Test DialogOverflowAdjust Case 1: dialog exceeds upper limit
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, DialogOverflowAdjust003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.wrapperSize_ = SizeF(600.0f, 1000.0f);
    algo.safeAreaInsets_.top_ = SafeAreaInsets::Inset { 0, 100 };
    algo.safeAreaInsets_.bottom_ = SafeAreaInsets::Inset { 900, 1000 };
    OffsetF childOffset(100.0f, 10.0f);
    SizeF childSize(200.0f, 200.0f);
    algo.embeddedDialogOffsetY_ = 0.0f;
    algo.stackRootDialogOffsetY_ = 0.0f;
    algo.resizeFlag_ = false;
    algo.DialogOverflowAdjust(childOffset, childSize, true);
    EXPECT_EQ(childOffset.GetY(), 140.0f);
}

/**
 * @tc.name: DialogOverflowAdjust004
 * @tc.desc: Test DialogOverflowAdjust Case 2: dialog exceeds lower limit
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, DialogOverflowAdjust004, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.wrapperSize_ = SizeF(600.0f, 1000.0f);
    algo.safeAreaInsets_.top_ = SafeAreaInsets::Inset { 0, 100 };
    algo.safeAreaInsets_.bottom_ = SafeAreaInsets::Inset { 900, 1000 };
    OffsetF childOffset(100.0f, 800.0f);
    SizeF childSize(200.0f, 200.0f);
    algo.embeddedDialogOffsetY_ = 0.0f;
    algo.stackRootDialogOffsetY_ = 0.0f;
    algo.resizeFlag_ = false;
    algo.DialogOverflowAdjust(childOffset, childSize, true);
    EXPECT_LE(childOffset.GetY(), 800.0f);
}

/**
 * @tc.name: DialogOverflowAdjust005
 * @tc.desc: Test DialogOverflowAdjust with hoverMode TOP_SCREEN (limitBottom = foldCreaseRect.Top())
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, DialogOverflowAdjust005, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.wrapperSize_ = SizeF(600.0f, 1000.0f);
    algo.safeAreaInsets_.top_ = SafeAreaInsets::Inset { 0, 0 };
    algo.safeAreaInsets_.bottom_ = SafeAreaInsets::Inset { 1000, 1000 };
    algo.isHoverMode_ = true;
    algo.hoverModeArea_ = HoverModeAreaType::TOP_SCREEN;
    algo.foldCreaseRect = Rect(0.0f, 0.0f, 600.0f, 500.0f);
    OffsetF childOffset(100.0f, 10.0f);
    SizeF childSize(200.0f, 200.0f);
    algo.embeddedDialogOffsetY_ = 0.0f;
    algo.stackRootDialogOffsetY_ = 0.0f;
    algo.resizeFlag_ = false;
    algo.DialogOverflowAdjust(childOffset, childSize, true);
    float expectedLimitBottom = 500.0f;
    EXPECT_LE(childOffset.GetY() + childSize.Height(), expectedLimitBottom + 1.0f);
}

/**
 * @tc.name: DialogOverflowAdjust006
 * @tc.desc: Test DialogOverflowAdjust with hoverMode BOTTOM_SCREEN (limitTop = foldCreaseRect.Bottom())
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, DialogOverflowAdjust006, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.wrapperSize_ = SizeF(600.0f, 1000.0f);
    algo.safeAreaInsets_.top_ = SafeAreaInsets::Inset { 0, 0 };
    algo.safeAreaInsets_.bottom_ = SafeAreaInsets::Inset { 1000, 1000 };
    algo.isHoverMode_ = true;
    algo.hoverModeArea_ = HoverModeAreaType::BOTTOM_SCREEN;
    algo.foldCreaseRect = Rect(0.0f, 500.0f, 600.0f, 400.0f);
    OffsetF childOffset(100.0f, 400.0f);
    SizeF childSize(200.0f, 200.0f);
    algo.embeddedDialogOffsetY_ = 0.0f;
    algo.stackRootDialogOffsetY_ = 0.0f;
    algo.resizeFlag_ = false;
    algo.DialogOverflowAdjust(childOffset, childSize, true);
    float expectedLimitTop = 900.0f;
    EXPECT_GE(childOffset.GetY(), expectedLimitTop - 1.0f);
}

/**
 * @tc.name: UpdateContentMaxSizeForScroll001
 * @tc.desc: Test UpdateContentMaxSizeForScroll when remaining > INNER_SCROLL_MIN_HEIGHT
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, UpdateContentMaxSizeForScroll001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto contentColumn =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 1, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    ASSERT_NE(contentColumn, nullptr);
    auto contentLayoutProp = contentColumn->GetLayoutProperty();
    ASSERT_NE(contentLayoutProp, nullptr);
    LayoutConstraintF constraint;
    constraint.maxSize = SizeF(400.0f, 500.0f);
    float contentHeightWithoutScroll = 100.0f;
    algo.UpdateContentMaxSizeForScroll(contentLayoutProp, constraint, contentHeightWithoutScroll);
    SUCCEED();
}

/**
 * @tc.name: UpdateContentMaxSizeForScroll002
 * @tc.desc: Test UpdateContentMaxSizeForScroll when remaining <= INNER_SCROLL_MIN_HEIGHT (skip)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, UpdateContentMaxSizeForScroll002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto contentColumn =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 1, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    ASSERT_NE(contentColumn, nullptr);
    auto contentLayoutProp = contentColumn->GetLayoutProperty();
    ASSERT_NE(contentLayoutProp, nullptr);
    LayoutConstraintF constraint;
    constraint.maxSize = SizeF(400.0f, 100.0f);
    float contentHeightWithoutScroll = 50.0f;
    algo.UpdateContentMaxSizeForScroll(contentLayoutProp, constraint, contentHeightWithoutScroll);
}

/**
 * @tc.name: UpdateContentMaxSizeForScroll003
 * @tc.desc: Test UpdateContentMaxSizeForScroll with null contentLayoutProp
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, UpdateContentMaxSizeForScroll003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    LayoutConstraintF constraint;
    constraint.maxSize = SizeF(400.0f, 500.0f);
    algo.UpdateContentMaxSizeForScroll(nullptr, constraint, 100.0f);
}

/**
 * @tc.name: MeasureFixedContentHeight001
 * @tc.desc: Test MeasureFixedContentHeight with null contentColumn
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, MeasureFixedContentHeight001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    std::optional<LayoutConstraintF> constraint;
    EXPECT_EQ(algo.MeasureFixedContentHeight(nullptr, constraint), 0.0f);
}

/**
 * @tc.name: MeasureFixedContentHeight002
 * @tc.desc: Test MeasureFixedContentHeight with children (Scroll/List are skipped)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, MeasureFixedContentHeight002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto contentColumn =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 1, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    ASSERT_NE(contentColumn, nullptr);
    auto scrollChild = FrameNode::CreateFrameNode("Scroll", 2, AceType::MakeRefPtr<Pattern>());
    auto textChild = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 3, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(scrollChild, nullptr);
    ASSERT_NE(textChild, nullptr);
    textChild->GetLayoutProperty()->UpdateUserDefinedIdealSize(CalcSize(CalcLength(100.0f), CalcLength(50.0f)));
    contentColumn->AddChild(scrollChild);
    contentColumn->AddChild(textChild);
    std::optional<LayoutConstraintF> constraint;
    auto height = algo.MeasureFixedContentHeight(contentColumn, constraint);
    EXPECT_EQ(height, 50.0f);
}

/**
 * @tc.name: ApplyDialogSizeToContent001
 * @tc.desc: Test ApplyDialogSizeToContent with null dialog
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ApplyDialogSizeToContent001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto contentLayoutProp = AceType::MakeRefPtr<LinearLayoutProperty>(false);
    ASSERT_NE(contentLayoutProp, nullptr);
    LayoutConstraintF constraint;
    algo.ApplyDialogSizeToContent(nullptr, contentLayoutProp, constraint);
}

/**
 * @tc.name: ApplyDialogSizeToContent002
 * @tc.desc: Test ApplyDialogSizeToContent with null contentLayoutProp
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ApplyDialogSizeToContent002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto dialog =
        FrameNode::CreateFrameNode(V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<DialogPattern>(nullptr, nullptr));
    ASSERT_NE(dialog, nullptr);
    LayoutConstraintF constraint;
    algo.ApplyDialogSizeToContent(dialog, nullptr, constraint);
}

/**
 * @tc.name: ApplyDialogSizeToContent003
 * @tc.desc: Test ApplyDialogSizeToContent with positive width and height
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ApplyDialogSizeToContent003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto dialogProp = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(dialogProp, nullptr);
    dialogProp->UpdateWidth(Dimension(320.0, DimensionUnit::VP));
    dialogProp->UpdateHeight(Dimension(240.0, DimensionUnit::VP));
    auto dialog = FrameNode::CreateFrameNode(V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(dialog, nullptr);
    dialog->layoutProperty_ = dialogProp;
    auto contentLayoutProp = AceType::MakeRefPtr<LinearLayoutProperty>(false);
    ASSERT_NE(contentLayoutProp, nullptr);
    LayoutConstraintF constraint;
    constraint.maxSize = SizeF(600.0f, 1000.0f);
    algo.ApplyDialogSizeToContent(dialog, contentLayoutProp, constraint);
    SUCCEED();
}

/**
 * @tc.name: ApplyDialogSizeToContent004
 * @tc.desc: Test ApplyDialogSizeToContent with negative width and height
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ApplyDialogSizeToContent004, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto dialogProp = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(dialogProp, nullptr);
    dialogProp->UpdateWidth(Dimension(-1.0, DimensionUnit::VP));
    dialogProp->UpdateHeight(Dimension(-1.0, DimensionUnit::VP));
    auto dialog = FrameNode::CreateFrameNode(V2::DIALOG_ETS_TAG, 1, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(dialog, nullptr);
    dialog->layoutProperty_ = dialogProp;
    auto contentLayoutProp = AceType::MakeRefPtr<LinearLayoutProperty>(false);
    ASSERT_NE(contentLayoutProp, nullptr);
    LayoutConstraintF constraint;
    constraint.maxSize = SizeF(600.0f, 1000.0f);
    algo.ApplyDialogSizeToContent(dialog, contentLayoutProp, constraint);
}

/**
 * @tc.name: ResetConstraint001
 * @tc.desc: Test ResetConstraint with null frameNode
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ResetConstraint001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.ResetConstraint(nullptr);
}

/**
 * @tc.name: ResetConstraint002
 * @tc.desc: Test ResetConstraint with valid frameNode containing scroll and list children
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ResetConstraint002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    auto contentColumn =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 1, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    ASSERT_NE(contentColumn, nullptr);
    auto scrollChild = FrameNode::CreateFrameNode("Scroll", 2, AceType::MakeRefPtr<Pattern>());
    auto listChild = FrameNode::CreateFrameNode("List", 3, AceType::MakeRefPtr<Pattern>());
    auto textChild = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 4, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(scrollChild, nullptr);
    ASSERT_NE(listChild, nullptr);
    ASSERT_NE(textChild, nullptr);
    contentColumn->AddChild(scrollChild);
    contentColumn->AddChild(listChild);
    contentColumn->AddChild(textChild);
    algo.ResetConstraint(contentColumn);
    SUCCEED();
}

/**
 * @tc.name: IsGetExpandDisplayValidHeight001
 * @tc.desc: Test IsGetExpandDisplayValidHeight with expandDisplay_ false
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, IsGetExpandDisplayValidHeight001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.expandDisplay_ = false;
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    EXPECT_FALSE(algo.IsGetExpandDisplayValidHeight(prop));
}

/**
 * @tc.name: IsGetExpandDisplayValidHeight002
 * @tc.desc: Test IsGetExpandDisplayValidHeight with expandDisplay_ true but isShowInSubWindow_ false
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, IsGetExpandDisplayValidHeight002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.expandDisplay_ = true;
    algo.isShowInSubWindow_ = false;
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    EXPECT_FALSE(algo.IsGetExpandDisplayValidHeight(prop));
}

/**
 * @tc.name: IsGetExpandDisplayValidHeight003
 * @tc.desc: Test IsGetExpandDisplayValidHeight with isModal_ && isUIExtensionSubWindow_
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, IsGetExpandDisplayValidHeight003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.expandDisplay_ = true;
    algo.isShowInSubWindow_ = true;
    algo.isModal_ = true;
    algo.isUIExtensionSubWindow_ = true;
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    EXPECT_FALSE(algo.IsGetExpandDisplayValidHeight(prop));
}

/**
 * @tc.name: IsGetExpandDisplayValidHeight004
 * @tc.desc: Test IsGetExpandDisplayValidHeight with null dialogProp
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, IsGetExpandDisplayValidHeight004, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.expandDisplay_ = true;
    algo.isShowInSubWindow_ = true;
    algo.isModal_ = false;
    algo.isUIExtensionSubWindow_ = false;
    EXPECT_FALSE(algo.IsGetExpandDisplayValidHeight(nullptr));
}

/**
 * @tc.name: ComputeInnerLayoutSizeParam001
 * @tc.desc: Test ComputeInnerLayoutSizeParam with gridCount_ >= 0 (returns false)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ComputeInnerLayoutSizeParam001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.gridCount_ = 2;
    LayoutConstraintF innerLayout;
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    EXPECT_FALSE(algo.ComputeInnerLayoutSizeParam(innerLayout, prop));
}

/**
 * @tc.name: ComputeInnerLayoutSizeParam002
 * @tc.desc: Test ComputeInnerLayoutSizeParam with API < 12 (CHECK_NULL_RETURN returns false)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ComputeInnerLayoutSizeParam002, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN);
    DialogLayoutAlgorithm algo;
    algo.gridCount_ = -1;
    LayoutConstraintF innerLayout;
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    EXPECT_FALSE(algo.ComputeInnerLayoutSizeParam(innerLayout, prop));
}

/**
 * @tc.name: ComputeInnerLayoutSizeParam003
 * @tc.desc: Test ComputeInnerLayoutSizeParam with null dialogTheme_ (returns false)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ComputeInnerLayoutSizeParam003, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.gridCount_ = -1;
    algo.dialogTheme_ = nullptr;
    LayoutConstraintF innerLayout;
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    EXPECT_FALSE(algo.ComputeInnerLayoutSizeParam(innerLayout, prop));
}

/**
 * @tc.name: ComputeInnerLayoutSizeParam004
 * @tc.desc: Test ComputeInnerLayoutSizeParam with valid setup and no width/height
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ComputeInnerLayoutSizeParam004, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.gridCount_ = -1;
    algo.dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
    ASSERT_NE(algo.dialogTheme_, nullptr);
    algo.dialogTheme_->containerMaxWidth_ = 400.0_vp;
    LayoutConstraintF innerLayout;
    innerLayout.maxSize = SizeF(600.0f, 1000.0f);
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    EXPECT_TRUE(algo.ComputeInnerLayoutSizeParam(innerLayout, prop));
    EXPECT_GT(innerLayout.maxSize.Width(), 0.0f);
    EXPECT_GT(innerLayout.maxSize.Height(), 0.0f);
}

/**
 * @tc.name: ComputeInnerLayoutSizeParam005
 * @tc.desc: Test ComputeInnerLayoutSizeParam with positive width has_value
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ComputeInnerLayoutSizeParam005, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.gridCount_ = -1;
    algo.dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
    ASSERT_NE(algo.dialogTheme_, nullptr);
    LayoutConstraintF innerLayout;
    innerLayout.maxSize = SizeF(600.0f, 1000.0f);
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    prop->UpdateWidth(Dimension(300.0, DimensionUnit::PX));
    EXPECT_TRUE(algo.ComputeInnerLayoutSizeParam(innerLayout, prop));
    EXPECT_EQ(innerLayout.maxSize.Width(), 300.0f);
}

/**
 * @tc.name: ComputeInnerLayoutSizeParam006
 * @tc.desc: Test ComputeInnerLayoutSizeParam with positive height has_value
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ComputeInnerLayoutSizeParam006, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.gridCount_ = -1;
    algo.dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
    ASSERT_NE(algo.dialogTheme_, nullptr);
    LayoutConstraintF innerLayout;
    innerLayout.maxSize = SizeF(600.0f, 1000.0f);
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    prop->UpdateHeight(Dimension(400.0, DimensionUnit::PX));
    EXPECT_TRUE(algo.ComputeInnerLayoutSizeParam(innerLayout, prop));
    EXPECT_EQ(innerLayout.maxSize.Height(), 400.0f);
}

/**
 * @tc.name: ComputeInnerLayoutSizeParam007
 * @tc.desc: Test ComputeInnerLayoutSizeParam with negative height (NonPositive realHeight branch)
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ComputeInnerLayoutSizeParam007, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.gridCount_ = -1;
    algo.dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
    ASSERT_NE(algo.dialogTheme_, nullptr);
    LayoutConstraintF innerLayout;
    innerLayout.maxSize = SizeF(600.0f, 1000.0f);
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    prop->UpdateHeight(Dimension(-10.0, DimensionUnit::PX));
    EXPECT_TRUE(algo.ComputeInnerLayoutSizeParam(innerLayout, prop));
    double ratioHeight = algo.dialogTheme_->GetDialogRatioHeight();
    EXPECT_EQ(innerLayout.maxSize.Height(), 1000.0f * ratioHeight);
}

/**
 * @tc.name: ComputeInnerLayoutSizeParam008
 * @tc.desc: Test ComputeInnerLayoutSizeParam with PERCENT width
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ComputeInnerLayoutSizeParam008, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    DialogLayoutAlgorithm algo;
    algo.gridCount_ = -1;
    algo.dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
    ASSERT_NE(algo.dialogTheme_, nullptr);
    LayoutConstraintF innerLayout;
    innerLayout.maxSize = SizeF(600.0f, 1000.0f);
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    prop->UpdateWidth(Dimension(50.0, DimensionUnit::PERCENT));
    EXPECT_TRUE(algo.ComputeInnerLayoutSizeParam(innerLayout, prop));
    EXPECT_EQ(innerLayout.maxSize.Width(), 600.0f);
}
} // namespace OHOS::Ace::NG
