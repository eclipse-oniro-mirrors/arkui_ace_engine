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
#include "core/components_ng/manager/force_split/force_split_manager.h"
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
// Force split geometry mirrors ParallelStageLayoutAlgorithm: splittable = root - divider(1.0),
// secondary = floor(splittable * ratio), primary = splittable - secondary.
constexpr float SPLIT_RATIO = 0.5f;
constexpr float SPLIT_DIVIDER_WIDTH = 1.0f;
constexpr float ROOT_WIDTH = 720.0f;
constexpr float ROOT_HEIGHT = 1000.0f;
constexpr float SPLITTABLE_WIDTH = ROOT_WIDTH - SPLIT_DIVIDER_WIDTH; // 719.0f
constexpr float SECONDARY_WIDTH = 359.0f; // floor(719 * 0.5)
constexpr float PRIMARY_WIDTH = SPLITTABLE_WIDTH - SECONDARY_WIDTH; // 360.0f
constexpr float SECONDARY_START_X = PRIMARY_WIDTH + SPLIT_DIVIDER_WIDTH; // 361.0f
constexpr float SELF_WIDTH = 600.0f;
constexpr float SELF_HEIGHT = 1000.0f;
constexpr float CHILD_WIDTH = 200.0f;
constexpr float CHILD_HEIGHT = 300.0f;
// Handedness cases (migrated from dialog_layout_test_five_ng.cpp) measure a smaller dialog.
constexpr float ANCHOR_CHILD_HEIGHT = 100.0f;
constexpr float KEYBOARD_HEIGHT = 600.0f;
constexpr float KEYBOARD_TOP = ROOT_HEIGHT - KEYBOARD_HEIGHT; // 400.0f
constexpr double LANDSCAPE_WIDTH_RATIO = 0.75;
const Dimension DIMENSION_RADIUS(10.0, DimensionUnit::PX);
const Dimension DIMENSION_WIDTH(2.0, DimensionUnit::PX);
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

namespace {
// Returns the host node as the third element: ElementRegister only keeps a weak reference to it,
// so the returned property's GetHost() resolves only while a RefPtr to the node is alive. Without
// it the host dies with the helper's local RefPtr, GetDialogPattern(prop) returns null, and the
// split column resolution never caches its decision on the pattern.
std::tuple<RefPtr<DialogLayoutProperty>, RefPtr<DialogPattern>, RefPtr<FrameNode>> CreateSplitDialogProp()
{
    auto dialogPattern = AceType::MakeRefPtr<DialogPattern>(nullptr, nullptr);
    auto dialog = FrameNode::CreateFrameNode(
        V2::DIALOG_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), dialogPattern);
    if (!dialog) {
        return { nullptr, nullptr, nullptr };
    }
    auto prop = AceType::DynamicCast<DialogLayoutProperty>(dialog->GetLayoutProperty());
    return { prop, dialogPattern, dialog };
}

void InitSplitLayoutConstraint(const RefPtr<DialogLayoutProperty>& prop)
{
    LayoutConstraintF constraint;
    constraint.maxSize = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    constraint.percentReference = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    prop->UpdateLayoutConstraint(constraint);
}

RefPtr<ForceSplitManager> GetOrCreateForceSplitManager()
{
    auto pipeline = MockPipelineContext::GetCurrent();
    CHECK_NULL_RETURN(pipeline, nullptr);
    if (!pipeline->GetForceSplitManager()) {
        pipeline->forceSplitMgr_ = AceType::MakeRefPtr<ForceSplitManager>();
    }
    return pipeline->GetForceSplitManager();
}
} // namespace

/**
 * @tc.name: NarrowConstraintToSplitColumnNoHost001
 * @tc.desc: Test NarrowConstraintToSplitColumn when prop has no host, resolution falls back to NONE
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, NarrowConstraintToSplitColumnNoHost001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.needAdaptForceSplitMode_ = true;
    algo.forceSplitRatio_ = SPLIT_RATIO;
    auto forceSplitMgr = GetOrCreateForceSplitManager();
    ASSERT_NE(forceSplitMgr, nullptr);
    forceSplitMgr->SetLastTouchedColumnType(ForceSplitPageColumnType::SECONDARY);
    // Standalone property without host: GetDialogPattern returns null, touched column is not consumed.
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    std::optional<LayoutConstraintF> layoutConstraint = LayoutConstraintF();
    layoutConstraint->maxSize = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    layoutConstraint->percentReference = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    algo.NarrowConstraintToSplitColumn(MockPipelineContext::GetCurrentContext(), prop, layoutConstraint);
    EXPECT_FLOAT_EQ(layoutConstraint->maxSize.Width(), SECONDARY_WIDTH);
    EXPECT_FLOAT_EQ(layoutConstraint->percentReference.Width(), SECONDARY_WIDTH);
}

/**
 * @tc.name: NarrowConstraintToSplitColumnManagerNull001
 * @tc.desc: Test NarrowConstraintToSplitColumn when pipeline has no ForceSplitManager
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, NarrowConstraintToSplitColumnManagerNull001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.needAdaptForceSplitMode_ = true;
    algo.forceSplitRatio_ = SPLIT_RATIO;
    auto [prop, dialogPattern, dialogNode] = CreateSplitDialogProp();
    ASSERT_NE(prop, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    ASSERT_NE(GetOrCreateForceSplitManager(), nullptr);
    // Temporarily remove the manager so the resolution path has no touched column record.
    auto savedManager = pipeline->forceSplitMgr_;
    pipeline->forceSplitMgr_ = nullptr;
    std::optional<LayoutConstraintF> layoutConstraint = LayoutConstraintF();
    layoutConstraint->maxSize = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    algo.NarrowConstraintToSplitColumn(MockPipelineContext::GetCurrentContext(), prop, layoutConstraint);
    pipeline->forceSplitMgr_ = savedManager;
    // No manager means NONE, which is cached on the pattern by the first resolution.
    EXPECT_FLOAT_EQ(layoutConstraint->maxSize.Width(), SECONDARY_WIDTH);
    EXPECT_FLOAT_EQ(layoutConstraint->percentReference.Width(), SECONDARY_WIDTH);
    EXPECT_TRUE(dialogPattern->GetCachedSplitColumnType().has_value());
    EXPECT_EQ(dialogPattern->GetCachedSplitColumnType().value(), ForceSplitPageColumnType::NONE);
}

/**
 * @tc.name: NarrowConstraintToSplitColumnTouchedPrimary001
 * @tc.desc: Test NarrowConstraintToSplitColumn with last touched primary column
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, NarrowConstraintToSplitColumnTouchedPrimary001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.needAdaptForceSplitMode_ = true;
    algo.forceSplitRatio_ = SPLIT_RATIO;
    auto [prop, dialogPattern, dialogNode] = CreateSplitDialogProp();
    ASSERT_NE(prop, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    auto forceSplitMgr = GetOrCreateForceSplitManager();
    ASSERT_NE(forceSplitMgr, nullptr);
    forceSplitMgr->SetLastTouchedColumnType(ForceSplitPageColumnType::PRIMARY);
    std::optional<LayoutConstraintF> layoutConstraint = LayoutConstraintF();
    layoutConstraint->maxSize = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    algo.NarrowConstraintToSplitColumn(MockPipelineContext::GetCurrentContext(), prop, layoutConstraint);
    EXPECT_FLOAT_EQ(layoutConstraint->maxSize.Width(), PRIMARY_WIDTH);
    EXPECT_FLOAT_EQ(layoutConstraint->percentReference.Width(), PRIMARY_WIDTH);
    EXPECT_EQ(dialogPattern->GetCachedSplitColumnType().value(), ForceSplitPageColumnType::PRIMARY);
}

/**
 * @tc.name: NarrowConstraintToSplitColumnCachePriority001
 * @tc.desc: Test NarrowConstraintToSplitColumn cache wins over last touched primary column
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, NarrowConstraintToSplitColumnCachePriority001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.needAdaptForceSplitMode_ = true;
    algo.forceSplitRatio_ = SPLIT_RATIO;
    auto [prop, dialogPattern, dialogNode] = CreateSplitDialogProp();
    ASSERT_NE(prop, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    dialogPattern->SetCachedSplitColumnType(ForceSplitPageColumnType::SECONDARY);
    auto forceSplitMgr = GetOrCreateForceSplitManager();
    ASSERT_NE(forceSplitMgr, nullptr);
    forceSplitMgr->SetLastTouchedColumnType(ForceSplitPageColumnType::PRIMARY);
    std::optional<LayoutConstraintF> layoutConstraint = LayoutConstraintF();
    layoutConstraint->maxSize = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    algo.NarrowConstraintToSplitColumn(MockPipelineContext::GetCurrentContext(), prop, layoutConstraint);
    // Cached decision is resolved once per dialog lifecycle and wins over the touched column.
    EXPECT_FLOAT_EQ(layoutConstraint->maxSize.Width(), SECONDARY_WIDTH);
    EXPECT_EQ(dialogPattern->GetCachedSplitColumnType().value(), ForceSplitPageColumnType::SECONDARY);
}

/**
 * @tc.name: NarrowConstraintToSplitColumnRatioBoundary001
 * @tc.desc: Test NarrowConstraintToSplitColumn ratio boundary clamps
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, NarrowConstraintToSplitColumnRatioBoundary001, TestSize.Level1)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    auto forceSplitMgr = GetOrCreateForceSplitManager();
    ASSERT_NE(forceSplitMgr, nullptr);
    forceSplitMgr->SetLastTouchedColumnType(ForceSplitPageColumnType::NONE);
    DialogLayoutAlgorithm algo;
    algo.needAdaptForceSplitMode_ = true;
    auto [prop, dialogPattern, dialogNode] = CreateSplitDialogProp();
    ASSERT_NE(prop, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    // ratio 1.0: secondary column covers the whole splittable width.
    algo.forceSplitRatio_ = 1.0f;
    std::optional<LayoutConstraintF> layoutConstraint = LayoutConstraintF();
    layoutConstraint->maxSize = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    algo.NarrowConstraintToSplitColumn(MockPipelineContext::GetCurrentContext(), prop, layoutConstraint);
    EXPECT_FLOAT_EQ(layoutConstraint->maxSize.Width(), SPLITTABLE_WIDTH);
    // ratio 0.0: secondary width is floored to zero.
    algo.forceSplitRatio_ = 0.0f;
    std::optional<LayoutConstraintF> zeroConstraint = LayoutConstraintF();
    zeroConstraint->maxSize = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    algo.NarrowConstraintToSplitColumn(MockPipelineContext::GetCurrentContext(), prop, zeroConstraint);
    EXPECT_FLOAT_EQ(zeroConstraint->maxSize.Width(), 0.0f);
    // tiny root width: both column widths clamp to zero instead of going negative.
    auto savedRootWidth = pipeline->rootWidth_;
    pipeline->rootWidth_ = 0.0f;
    DialogLayoutAlgorithm tinyAlgo;
    tinyAlgo.needAdaptForceSplitMode_ = true;
    tinyAlgo.forceSplitRatio_ = SPLIT_RATIO;
    auto [tinyProp, tinyPattern, tinyNode] = CreateSplitDialogProp();
    ASSERT_NE(tinyProp, nullptr);
    std::optional<LayoutConstraintF> tinyConstraint = LayoutConstraintF();
    tinyConstraint->maxSize = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    tinyAlgo.NarrowConstraintToSplitColumn(MockPipelineContext::GetCurrentContext(), tinyProp, tinyConstraint);
    pipeline->rootWidth_ = savedRootWidth;
    EXPECT_FLOAT_EQ(tinyConstraint->maxSize.Width(), 0.0f);
}

/**
 * @tc.name: ShiftTopLeftToSplitColumnTouchedPrimary001
 * @tc.desc: Test ShiftTopLeftToSplitColumn with last touched primary column keeps position
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ShiftTopLeftToSplitColumnTouchedPrimary001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.needAdaptForceSplitMode_ = true;
    algo.forceSplitRatio_ = SPLIT_RATIO;
    auto [prop, dialogPattern, dialogNode] = CreateSplitDialogProp();
    ASSERT_NE(prop, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    auto forceSplitMgr = GetOrCreateForceSplitManager();
    ASSERT_NE(forceSplitMgr, nullptr);
    forceSplitMgr->SetLastTouchedColumnType(ForceSplitPageColumnType::PRIMARY);
    OffsetF topLeftPoint(10.0f, 20.0f);
    algo.ShiftTopLeftToSplitColumn(topLeftPoint, MockPipelineContext::GetCurrentContext(), prop);
    // Dialog on the primary column is not shifted.
    EXPECT_FLOAT_EQ(topLeftPoint.GetX(), 10.0f);
    EXPECT_FLOAT_EQ(topLeftPoint.GetY(), 20.0f);
}

/**
 * @tc.name: ShiftTopLeftToSplitColumnNoHost001
 * @tc.desc: Test ShiftTopLeftToSplitColumn when prop has no host shifts by secondary start
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ShiftTopLeftToSplitColumnNoHost001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.needAdaptForceSplitMode_ = true;
    algo.forceSplitRatio_ = SPLIT_RATIO;
    auto forceSplitMgr = GetOrCreateForceSplitManager();
    ASSERT_NE(forceSplitMgr, nullptr);
    forceSplitMgr->SetLastTouchedColumnType(ForceSplitPageColumnType::SECONDARY);
    // Standalone property without host: null dialog pattern resolves to NONE, treated as secondary.
    auto prop = AceType::MakeRefPtr<DialogLayoutProperty>();
    ASSERT_NE(prop, nullptr);
    OffsetF topLeftPoint(10.0f, 20.0f);
    algo.ShiftTopLeftToSplitColumn(topLeftPoint, MockPipelineContext::GetCurrentContext(), prop);
    EXPECT_FLOAT_EQ(topLeftPoint.GetX(), 10.0f + SECONDARY_START_X);
    EXPECT_FLOAT_EQ(topLeftPoint.GetY(), 20.0f);
}

/**
 * @tc.name: ComputeChildPositionNullTheme001
 * @tc.desc: Test ComputeChildPosition with null dialogTheme returns zero offset
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ComputeChildPositionNullTheme001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.dialogTheme_ = nullptr;
    auto [prop, dialogPattern, dialogNode] = CreateSplitDialogProp();
    ASSERT_NE(prop, nullptr);
    InitSplitLayoutConstraint(prop);
    auto childOffset = algo.ComputeChildPosition(
        SizeF(CHILD_WIDTH, CHILD_HEIGHT), prop, SizeF(SELF_WIDTH, ROOT_HEIGHT), dialogPattern);
    EXPECT_FLOAT_EQ(childOffset.GetX(), 0.0f);
    EXPECT_FLOAT_EQ(childOffset.GetY(), 0.0f);
}

/**
 * @tc.name: ComputeChildPositionKeyboardAvoidNone001
 * @tc.desc: Test ComputeChildPosition keyboardAvoidMode NONE skips keyboard avoidance
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ComputeChildPositionKeyboardAvoidNone001, TestSize.Level1)
{
    auto container = Container::Current();
    ASSERT_NE(container, nullptr);
    container->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    // Keyboard inset is idempotent across cases, do not assert the change return value.
    pipeline->GetSafeAreaManager()->UpdateKeyboardSafeArea(KEYBOARD_HEIGHT, ROOT_HEIGHT);
    DialogLayoutAlgorithm algo;
    algo.dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
    ASSERT_NE(algo.dialogTheme_, nullptr);
    algo.keyboardAvoidMode_ = KeyboardAvoidMode::NONE;
    algo.alignment_ = DialogAlignment::BOTTOM;
    auto [prop, dialogPattern, dialogNode] = CreateSplitDialogProp();
    ASSERT_NE(prop, nullptr);
    InitSplitLayoutConstraint(prop);
    auto childOffset = algo.ComputeChildPosition(
        SizeF(CHILD_WIDTH, CHILD_HEIGHT), prop, SizeF(SELF_WIDTH, ROOT_HEIGHT), dialogPattern);
    // NONE mode: dialog bottom overlaps the keyboard but the offset stays at the aligned position.
    // X is centered inside the prop layout constraint (ROOT_WIDTH), matching ComputeChildPosition.
    EXPECT_FLOAT_EQ(childOffset.GetX(), (ROOT_WIDTH - CHILD_WIDTH) / 2.0f);
    EXPECT_FLOAT_EQ(childOffset.GetY(), ROOT_HEIGHT - CHILD_HEIGHT);
}

/**
 * @tc.name: ComputeChildPositionKeyboardExpandOpts001
 * @tc.desc: Test ComputeChildPosition with KEYBOARD safe area expand opts disables avoidance
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ComputeChildPositionKeyboardExpandOpts001, TestSize.Level1)
{
    auto container = Container::Current();
    ASSERT_NE(container, nullptr);
    container->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    // Keyboard inset is idempotent across cases, do not assert the change return value.
    pipeline->GetSafeAreaManager()->UpdateKeyboardSafeArea(KEYBOARD_HEIGHT, ROOT_HEIGHT);
    DialogLayoutAlgorithm algo;
    algo.dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
    ASSERT_NE(algo.dialogTheme_, nullptr);
    algo.keyboardAvoidMode_ = KeyboardAvoidMode::DEFAULT;
    algo.alignment_ = DialogAlignment::BOTTOM;
    auto [prop, dialogPattern, dialogNode] = CreateSplitDialogProp();
    ASSERT_NE(prop, nullptr);
    InitSplitLayoutConstraint(prop);
    SafeAreaExpandOpts opts;
    opts.type = SAFE_AREA_TYPE_KEYBOARD;
    prop->UpdateSafeAreaExpandOpts(opts);
    auto childOffset = algo.ComputeChildPosition(
        SizeF(CHILD_WIDTH, CHILD_HEIGHT), prop, SizeF(SELF_WIDTH, ROOT_HEIGHT), dialogPattern);
    // Dialog expands into the keyboard area, so no keyboard avoidance is applied.
    // X is centered inside the prop layout constraint (ROOT_WIDTH), matching ComputeChildPosition.
    EXPECT_FLOAT_EQ(childOffset.GetX(), (ROOT_WIDTH - CHILD_WIDTH) / 2.0f);
    EXPECT_FLOAT_EQ(childOffset.GetY(), ROOT_HEIGHT - CHILD_HEIGHT);
}

/**
 * @tc.name: ComputeChildPositionKeyboardAvoid001
 * @tc.desc: Test ComputeChildPosition with DEFAULT mode moves the dialog above the keyboard
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ComputeChildPositionKeyboardAvoid001, TestSize.Level1)
{
    auto container = Container::Current();
    ASSERT_NE(container, nullptr);
    container->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    // Keyboard inset is idempotent across cases, do not assert the change return value.
    pipeline->GetSafeAreaManager()->UpdateKeyboardSafeArea(KEYBOARD_HEIGHT, ROOT_HEIGHT);
    DialogLayoutAlgorithm algo;
    algo.dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
    ASSERT_NE(algo.dialogTheme_, nullptr);
    algo.keyboardAvoidMode_ = KeyboardAvoidMode::DEFAULT;
    algo.alignment_ = DialogAlignment::BOTTOM;
    auto [prop, dialogPattern, dialogNode] = CreateSplitDialogProp();
    ASSERT_NE(prop, nullptr);
    InitSplitLayoutConstraint(prop);
    auto childOffset = algo.ComputeChildPosition(
        SizeF(CHILD_WIDTH, CHILD_HEIGHT), prop, SizeF(SELF_WIDTH, ROOT_HEIGHT), dialogPattern);
    // DEFAULT mode: dialog bottom is lifted to the keyboard top minus the bottom padding.
    float paddingBottomPx = static_cast<float>(
        MockPipelineContext::GetCurrentContext()->NormalizeToPx(algo.dialogTheme_->GetDefaultDialogMarginBottom()));
    float expectedY = (ROOT_HEIGHT - CHILD_HEIGHT) - (ROOT_HEIGHT - (KEYBOARD_TOP - paddingBottomPx));
    // X is centered inside the prop layout constraint (ROOT_WIDTH), matching ComputeChildPosition.
    EXPECT_FLOAT_EQ(childOffset.GetX(), (ROOT_WIDTH - CHILD_WIDTH) / 2.0f);
    EXPECT_FLOAT_EQ(childOffset.GetY(), expectedY);
    EXPECT_LT(childOffset.GetY(), ROOT_HEIGHT - CHILD_HEIGHT);
}

/**
 * @tc.name: ComputeChildPositionForceSplitShift001
 * @tc.desc: Test ComputeChildPosition in force split mode narrows constraint and shifts to secondary
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ComputeChildPositionForceSplitShift001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
    ASSERT_NE(algo.dialogTheme_, nullptr);
    algo.needAdaptForceSplitMode_ = true;
    algo.forceSplitRatio_ = SPLIT_RATIO;
    algo.alignment_ = DialogAlignment::CENTER;
    algo.keyboardAvoidMode_ = KeyboardAvoidMode::NONE;
    auto [prop, dialogPattern, dialogNode] = CreateSplitDialogProp();
    ASSERT_NE(prop, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    InitSplitLayoutConstraint(prop);
    auto forceSplitMgr = GetOrCreateForceSplitManager();
    ASSERT_NE(forceSplitMgr, nullptr);
    forceSplitMgr->SetLastTouchedColumnType(ForceSplitPageColumnType::SECONDARY);
    auto childOffset = algo.ComputeChildPosition(
        SizeF(CHILD_WIDTH, CHILD_HEIGHT), prop, SizeF(ROOT_WIDTH, ROOT_HEIGHT), dialogPattern);
    // Constraint is narrowed to the real secondary column width and cached on the pattern.
    EXPECT_FLOAT_EQ(algo.wrapperSize_.Width(), SECONDARY_WIDTH);
    EXPECT_EQ(dialogPattern->GetCachedSplitColumnType().value(), ForceSplitPageColumnType::SECONDARY);
    // Center alignment inside the secondary column, then shifted to the secondary column start.
    float expectedX = (SECONDARY_WIDTH - CHILD_WIDTH) / 2.0f + SECONDARY_START_X;
    float expectedY = (ROOT_HEIGHT - CHILD_HEIGHT) / 2.0f;
    EXPECT_FLOAT_EQ(childOffset.GetX(), expectedX);
    EXPECT_FLOAT_EQ(childOffset.GetY(), expectedY);
}

/**
 * @tc.name: MeasureForceSplitConstraint001
 * @tc.desc: Test Measure in force split mode narrows constraint to the touched secondary column
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, MeasureForceSplitConstraint001, TestSize.Level1)
{
    SetDialogTheme();
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    // ComputeInnerLayoutParam picks its sizing branch from the pipeline min platform version and
    // earlier cases raise it to VERSION_TWELVE without restoring it. Pin it below VERSION_TWELVE
    // so the child constraint asserted here (grid sizing: min height 0, min width == max width)
    // is deterministic regardless of test order.
    auto savedMinPlatformVersion = pipeline->minPlatformVersion_;
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN);
    pipeline->SetIsCurrentInForceSplitMode(true);
    auto forceSplitMgr = GetOrCreateForceSplitManager();
    ASSERT_NE(forceSplitMgr, nullptr);
    forceSplitMgr->SetLastTouchedColumnType(ForceSplitPageColumnType::SECONDARY);
    auto childNode = FrameNode::CreateFrameNode(V2::BLANK_ETS_TAG, 101, AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(childNode, nullptr);
    auto childLayoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        childNode, childNode->GetGeometryNode(), childNode->GetLayoutProperty());
    ASSERT_NE(childLayoutWrapper, nullptr);
    DialogProperties param { .type = DialogType::ACTION_SHEET,
        .title = "dialog test",
        .content = "dialog content test",
        .sheetsInfo = sheetItems,
        .buttons = btnItems,
        .width = -24,
        .backgroundColor = Color::TRANSPARENT };
    NG::BorderRadiusProperty borderRadius;
    borderRadius.SetRadius(DIMENSION_RADIUS);
    param.borderRadius = borderRadius;
    NG::BorderWidthProperty borderWidth;
    borderWidth.SetBorderWidth(DIMENSION_WIDTH);
    param.borderWidth = borderWidth;
    NG::BorderColorProperty borderColor;
    borderColor.SetColor(Color::BLACK);
    param.borderColor = borderColor;
    auto dialog = DialogView::CreateDialogNode(param, nullptr);
    ASSERT_NE(dialog, nullptr);
    auto dialogPattern = dialog->GetPattern<DialogPattern>();
    ASSERT_NE(dialogPattern, nullptr);
    LayoutConstraintF layoutConstraint;
    layoutConstraint.parentIdealSize = { ROOT_WIDTH, ROOT_HEIGHT };
    layoutConstraint.maxSize = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    auto layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(dialog, dialog->GetGeometryNode(), dialog->GetLayoutProperty());
    layoutWrapper->GetLayoutProperty()->UpdateLayoutConstraint(layoutConstraint);
    layoutWrapper->GetLayoutProperty()->UpdateContentConstraint();
    layoutWrapper->AppendChild(childLayoutWrapper);
    DialogLayoutAlgorithm dialogLayoutAlgorithm;
    dialogLayoutAlgorithm.Measure(layoutWrapper.rawPtr_);
    // Measure adapts to force split mode and caches the touched secondary column on the pattern.
    EXPECT_TRUE(dialogLayoutAlgorithm.needAdaptForceSplitMode_);
    EXPECT_FLOAT_EQ(dialogLayoutAlgorithm.forceSplitRatio_, SPLIT_RATIO);
    EXPECT_EQ(dialogPattern->GetCachedSplitColumnType().value(), ForceSplitPageColumnType::SECONDARY);
    // Child constraint is bounded by the secondary column width. The grid sizing applied by
    // ComputeInnerLayoutParam after the force split narrowing sets the standard dialog width as
    // both min and max width (min height 0), so the 1px clamp written before it does not survive.
    const auto& childConstraint = childLayoutWrapper->GetLayoutProperty()->GetLayoutConstraint();
    ASSERT_TRUE(childConstraint.has_value());
    EXPECT_FLOAT_EQ(childConstraint->minSize.Height(), 0.0f);
    EXPECT_FLOAT_EQ(childConstraint->minSize.Width(), childConstraint->maxSize.Width());
    EXPECT_LE(childConstraint->maxSize.Width(), SECONDARY_WIDTH);
    pipeline->SetIsCurrentInForceSplitMode(false);
    forceSplitMgr->SetLastTouchedColumnType(ForceSplitPageColumnType::NONE);
    pipeline->minPlatformVersion_ = savedMinPlatformVersion;
}

/**
 * @tc.name: ComputeInnerLayoutSizeParamAgingForceSplit001
 * @tc.desc: Test ComputeInnerLayoutSizeParam aging width follows force split column width
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutThreeTestNg, ComputeInnerLayoutSizeParamAgingForceSplit001, TestSize.Level1)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    pipeline->minPlatformVersion_ = static_cast<int32_t>(PlatformVersion::VERSION_TWELVE);
    SystemProperties::orientation_ = DeviceOrientation::LANDSCAPE;
    DialogLayoutAlgorithm algo;
    algo.gridCount_ = -1;
    algo.dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
    ASSERT_NE(algo.dialogTheme_, nullptr);
    algo.isSuitableForElderly_ = true;
    algo.needAdaptForceSplitMode_ = true;
    auto [prop, dialogPattern, dialogNode] = CreateSplitDialogProp();
    ASSERT_NE(prop, nullptr);
    LayoutConstraintF innerLayout;
    innerLayout.maxSize = SizeF(SECONDARY_WIDTH, ROOT_HEIGHT);
    EXPECT_TRUE(algo.ComputeInnerLayoutSizeParam(innerLayout, prop));
    // Force split mode: aging width ratio is applied on the narrowed column width.
    EXPECT_FLOAT_EQ(innerLayout.maxSize.Width(), LANDSCAPE_WIDTH_RATIO * SECONDARY_WIDTH);
    // Force split off: aging width ratio is applied on the full root width.
    algo.needAdaptForceSplitMode_ = false;
    LayoutConstraintF fullLayout;
    fullLayout.maxSize = SizeF(SECONDARY_WIDTH, ROOT_HEIGHT);
    EXPECT_TRUE(algo.ComputeInnerLayoutSizeParam(fullLayout, prop));
    EXPECT_FLOAT_EQ(fullLayout.maxSize.Width(), LANDSCAPE_WIDTH_RATIO * ROOT_WIDTH);
    SystemProperties::orientation_ = DeviceOrientation::PORTRAIT;
}

// The suite below is migrated from dialog_layout_test_five_ng.cpp so all force split
// dialog layout TDD cases live in a single translation unit.

class DialogLayoutFiveTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;

    RefPtr<DialogTheme> dialogTheme_ = AceType::MakeRefPtr<DialogTheme>();
};

void DialogLayoutFiveTestNg::SetUpTestCase()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    MockContainer::Current()->pipelineContext_ = MockPipelineContext::GetCurrentContext();
    MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    MockPipelineContext::GetCurrent()->rootWidth_ = ROOT_WIDTH;
    MockPipelineContext::GetCurrent()->rootHeight_ = ROOT_HEIGHT;
}

void DialogLayoutFiveTestNg::TearDownTestCase()
{
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

void DialogLayoutFiveTestNg::SetUp()
{
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    if (!pipeline->GetForceSplitManager()) {
        pipeline->forceSplitMgr_ = AceType::MakeRefPtr<ForceSplitManager>();
    }
    auto forceSplitMgr = pipeline->GetForceSplitManager();
    ASSERT_NE(forceSplitMgr, nullptr);
    // Reset per-test force split state so cases do not affect each other.
    forceSplitMgr->SetLastTouchedColumnType(ForceSplitPageColumnType::NONE);
    forceSplitMgr->MarkNavigationColumnRecorded();
    forceSplitMgr->ConsumeNavigationColumnRecorded();
}

namespace {
// Third element keeps the host node alive for the same reason as CreateSplitDialogProp above.
std::tuple<RefPtr<DialogLayoutProperty>, RefPtr<DialogPattern>, RefPtr<FrameNode>> CreateDialogProp(
    const RefPtr<DialogTheme>& theme)
{
    auto dialogPattern = AceType::MakeRefPtr<DialogPattern>(theme, nullptr);
    auto dialog = FrameNode::CreateFrameNode(V2::DIALOG_ETS_TAG, 1, dialogPattern);
    if (!dialog) {
        return { nullptr, nullptr, nullptr };
    }
    auto prop = AceType::DynamicCast<DialogLayoutProperty>(dialog->GetLayoutProperty());
    return { prop, dialogPattern, dialog };
}
} // namespace

/**
 * @tc.name: SetLastTouchedColumnType001
 * @tc.desc: Test ForceSplitManager last touched column type default and set/get roundtrip
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFiveTestNg, SetLastTouchedColumnType001, TestSize.Level1)
{
    auto manager = AceType::MakeRefPtr<ForceSplitManager>();
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->GetLastTouchedColumnType(), ForceSplitPageColumnType::NONE);
    manager->SetLastTouchedColumnType(ForceSplitPageColumnType::PRIMARY);
    EXPECT_EQ(manager->GetLastTouchedColumnType(), ForceSplitPageColumnType::PRIMARY);
    manager->SetLastTouchedColumnType(ForceSplitPageColumnType::SECONDARY);
    EXPECT_EQ(manager->GetLastTouchedColumnType(), ForceSplitPageColumnType::SECONDARY);
}

/**
 * @tc.name: MarkNavigationColumnRecorded001
 * @tc.desc: Test ForceSplitManager navigation column recorded flag mark/consume lifecycle
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFiveTestNg, MarkNavigationColumnRecorded001, TestSize.Level1)
{
    auto manager = AceType::MakeRefPtr<ForceSplitManager>();
    ASSERT_NE(manager, nullptr);
    EXPECT_FALSE(manager->ConsumeNavigationColumnRecorded());
    manager->MarkNavigationColumnRecorded();
    EXPECT_TRUE(manager->ConsumeNavigationColumnRecorded());
    EXPECT_FALSE(manager->ConsumeNavigationColumnRecorded());
}

/**
 * @tc.name: GetCachedSplitColumnType001
 * @tc.desc: Test DialogPattern cached split column type default and set/get roundtrip
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFiveTestNg, GetCachedSplitColumnType001, TestSize.Level1)
{
    auto dialogPattern = AceType::MakeRefPtr<DialogPattern>(dialogTheme_, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    EXPECT_FALSE(dialogPattern->GetCachedSplitColumnType().has_value());
    dialogPattern->SetCachedSplitColumnType(ForceSplitPageColumnType::PRIMARY);
    EXPECT_EQ(dialogPattern->GetCachedSplitColumnType().value(), ForceSplitPageColumnType::PRIMARY);
    dialogPattern->SetCachedSplitColumnType(ForceSplitPageColumnType::SECONDARY);
    EXPECT_EQ(dialogPattern->GetCachedSplitColumnType().value(), ForceSplitPageColumnType::SECONDARY);
}

/**
 * @tc.name: NarrowConstraintToSplitColumn001
 * @tc.desc: Test NarrowConstraintToSplitColumn guard branch when force split mode is off
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFiveTestNg, NarrowConstraintToSplitColumn001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.needAdaptForceSplitMode_ = false;
    auto [prop, dialogPattern, dialogNode] = CreateDialogProp(dialogTheme_);
    ASSERT_NE(prop, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    std::optional<LayoutConstraintF> layoutConstraint = LayoutConstraintF();
    layoutConstraint->maxSize = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    layoutConstraint->percentReference = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    algo.NarrowConstraintToSplitColumn(MockPipelineContext::GetCurrentContext(), prop, layoutConstraint);
    EXPECT_FLOAT_EQ(layoutConstraint->maxSize.Width(), ROOT_WIDTH);
    EXPECT_FLOAT_EQ(layoutConstraint->percentReference.Width(), ROOT_WIDTH);
    EXPECT_FALSE(dialogPattern->GetCachedSplitColumnType().has_value());
}

/**
 * @tc.name: NarrowConstraintToSplitColumn002
 * @tc.desc: Test NarrowConstraintToSplitColumn without cache and touched column, resolves secondary column
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFiveTestNg, NarrowConstraintToSplitColumn002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.needAdaptForceSplitMode_ = true;
    algo.forceSplitRatio_ = SPLIT_RATIO;
    auto [prop, dialogPattern, dialogNode] = CreateDialogProp(dialogTheme_);
    ASSERT_NE(prop, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    std::optional<LayoutConstraintF> layoutConstraint = LayoutConstraintF();
    layoutConstraint->maxSize = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    algo.NarrowConstraintToSplitColumn(MockPipelineContext::GetCurrentContext(), prop, layoutConstraint);
    EXPECT_FLOAT_EQ(layoutConstraint->maxSize.Width(), SECONDARY_WIDTH);
    EXPECT_FLOAT_EQ(layoutConstraint->percentReference.Width(), SECONDARY_WIDTH);
    EXPECT_EQ(dialogPattern->GetCachedSplitColumnType().value(), ForceSplitPageColumnType::NONE);
}

/**
 * @tc.name: NarrowConstraintToSplitColumn003
 * @tc.desc: Test NarrowConstraintToSplitColumn with cached primary column, resolves primary width
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFiveTestNg, NarrowConstraintToSplitColumn003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.needAdaptForceSplitMode_ = true;
    algo.forceSplitRatio_ = SPLIT_RATIO;
    auto [prop, dialogPattern, dialogNode] = CreateDialogProp(dialogTheme_);
    ASSERT_NE(prop, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    dialogPattern->SetCachedSplitColumnType(ForceSplitPageColumnType::PRIMARY);
    std::optional<LayoutConstraintF> layoutConstraint = LayoutConstraintF();
    layoutConstraint->maxSize = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    algo.NarrowConstraintToSplitColumn(MockPipelineContext::GetCurrentContext(), prop, layoutConstraint);
    EXPECT_FLOAT_EQ(layoutConstraint->maxSize.Width(), PRIMARY_WIDTH);
    EXPECT_FLOAT_EQ(layoutConstraint->percentReference.Width(), PRIMARY_WIDTH);
    EXPECT_EQ(dialogPattern->GetCachedSplitColumnType().value(), ForceSplitPageColumnType::PRIMARY);
}

/**
 * @tc.name: NarrowConstraintToSplitColumn004
 * @tc.desc: Test NarrowConstraintToSplitColumn with last touched secondary column from force split manager
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFiveTestNg, NarrowConstraintToSplitColumn004, TestSize.Level2)
{
    DialogLayoutAlgorithm algo;
    algo.needAdaptForceSplitMode_ = true;
    algo.forceSplitRatio_ = SPLIT_RATIO;
    auto [prop, dialogPattern, dialogNode] = CreateDialogProp(dialogTheme_);
    ASSERT_NE(prop, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    auto forceSplitMgr = MockPipelineContext::GetCurrent()->GetForceSplitManager();
    ASSERT_NE(forceSplitMgr, nullptr);
    forceSplitMgr->SetLastTouchedColumnType(ForceSplitPageColumnType::SECONDARY);
    std::optional<LayoutConstraintF> layoutConstraint = LayoutConstraintF();
    layoutConstraint->maxSize = SizeF(ROOT_WIDTH, ROOT_HEIGHT);
    algo.NarrowConstraintToSplitColumn(MockPipelineContext::GetCurrentContext(), prop, layoutConstraint);
    EXPECT_FLOAT_EQ(layoutConstraint->maxSize.Width(), SECONDARY_WIDTH);
    EXPECT_EQ(dialogPattern->GetCachedSplitColumnType().value(), ForceSplitPageColumnType::SECONDARY);
}

/**
 * @tc.name: ShiftTopLeftToSplitColumn001
 * @tc.desc: Test ShiftTopLeftToSplitColumn guard branch when force split mode is off
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFiveTestNg, ShiftTopLeftToSplitColumn001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.needAdaptForceSplitMode_ = false;
    auto [prop, dialogPattern, dialogNode] = CreateDialogProp(dialogTheme_);
    ASSERT_NE(prop, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    OffsetF topLeftPoint(10.0f, 20.0f);
    algo.ShiftTopLeftToSplitColumn(topLeftPoint, MockPipelineContext::GetCurrentContext(), prop);
    EXPECT_FLOAT_EQ(topLeftPoint.GetX(), 10.0f);
    EXPECT_FLOAT_EQ(topLeftPoint.GetY(), 20.0f);
}

/**
 * @tc.name: ShiftTopLeftToSplitColumn002
 * @tc.desc: Test ShiftTopLeftToSplitColumn with cached primary column, keeps position unchanged
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFiveTestNg, ShiftTopLeftToSplitColumn002, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.needAdaptForceSplitMode_ = true;
    algo.forceSplitRatio_ = SPLIT_RATIO;
    auto [prop, dialogPattern, dialogNode] = CreateDialogProp(dialogTheme_);
    ASSERT_NE(prop, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    dialogPattern->SetCachedSplitColumnType(ForceSplitPageColumnType::PRIMARY);
    OffsetF topLeftPoint(10.0f, 20.0f);
    algo.ShiftTopLeftToSplitColumn(topLeftPoint, MockPipelineContext::GetCurrentContext(), prop);
    EXPECT_FLOAT_EQ(topLeftPoint.GetX(), 10.0f);
    EXPECT_FLOAT_EQ(topLeftPoint.GetY(), 20.0f);
}

/**
 * @tc.name: ShiftTopLeftToSplitColumn003
 * @tc.desc: Test ShiftTopLeftToSplitColumn with secondary column, shifts to secondary start x
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFiveTestNg, ShiftTopLeftToSplitColumn003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.needAdaptForceSplitMode_ = true;
    algo.forceSplitRatio_ = SPLIT_RATIO;
    auto [prop, dialogPattern, dialogNode] = CreateDialogProp(dialogTheme_);
    ASSERT_NE(prop, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    OffsetF topLeftPoint(10.0f, 20.0f);
    algo.ShiftTopLeftToSplitColumn(topLeftPoint, MockPipelineContext::GetCurrentContext(), prop);
    EXPECT_FLOAT_EQ(topLeftPoint.GetX(), 10.0f + SECONDARY_START_X);
    EXPECT_FLOAT_EQ(topLeftPoint.GetY(), 20.0f);
}

/**
 * @tc.name: ComputeTopLeftPoint001
 * @tc.desc: Test ComputeTopLeftPoint when handedness is inactive, falls back to alignment center
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFiveTestNg, ComputeTopLeftPoint001, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.handednessMode_ = HandednessFollowMode::NONE;
    algo.alignment_ = DialogAlignment::CENTER;
    OffsetF topLeftPoint;
    algo.ComputeTopLeftPoint(
        topLeftPoint, SizeF(CHILD_WIDTH, ANCHOR_CHILD_HEIGHT), SizeF(SELF_WIDTH, SELF_HEIGHT), nullptr, nullptr);
    EXPECT_FLOAT_EQ(topLeftPoint.GetX(), (SELF_WIDTH - CHILD_WIDTH) / 2.0f);
    EXPECT_FLOAT_EQ(topLeftPoint.GetY(), (SELF_HEIGHT - ANCHOR_CHILD_HEIGHT) / 2.0f);
}

/**
 * @tc.name: ComputeTopLeftPoint002
 * @tc.desc: Test ComputeTopLeftPoint with active handedness but null dialog pattern, falls back to alignment
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFiveTestNg, ComputeTopLeftPoint002, TestSize.Level2)
{
    DialogLayoutAlgorithm algo;
    algo.dialogTheme_ = dialogTheme_;
    algo.handednessMode_ = HandednessFollowMode::FOLLOW_ANCHOR;
    algo.alignment_ = DialogAlignment::CENTER;
    auto [prop, dialogPattern, dialogNode] = CreateDialogProp(dialogTheme_);
    ASSERT_NE(prop, nullptr);
    OffsetF topLeftPoint;
    algo.ComputeTopLeftPoint(
        topLeftPoint, SizeF(CHILD_WIDTH, ANCHOR_CHILD_HEIGHT), SizeF(SELF_WIDTH, SELF_HEIGHT), prop, nullptr);
    EXPECT_FLOAT_EQ(topLeftPoint.GetX(), (SELF_WIDTH - CHILD_WIDTH) / 2.0f);
    EXPECT_FLOAT_EQ(topLeftPoint.GetY(), (SELF_HEIGHT - ANCHOR_CHILD_HEIGHT) / 2.0f);
}

/**
 * @tc.name: ComputeTopLeftPoint003
 * @tc.desc: Test ComputeTopLeftPoint with active handedness and cached anchor, applies handedness layout
 * @tc.type: FUNC
 */
HWTEST_F(DialogLayoutFiveTestNg, ComputeTopLeftPoint003, TestSize.Level1)
{
    DialogLayoutAlgorithm algo;
    algo.dialogTheme_ = dialogTheme_;
    algo.handednessMode_ = HandednessFollowMode::FOLLOW_ANCHOR;
    auto [prop, dialogPattern, dialogNode] = CreateDialogProp(dialogTheme_);
    ASSERT_NE(prop, nullptr);
    ASSERT_NE(dialogPattern, nullptr);
    // Cached anchor avoids the HandednessManager query in unit test environment.
    dialogPattern->MarkHandednessLayoutCaptured();
    dialogPattern->SetCachedAnchorPos(OffsetF(300.0f, 200.0f));
    dialogPattern->SetNeedRefreshAnchorPos(false);
    OffsetF topLeftPoint;
    algo.ComputeTopLeftPoint(
        topLeftPoint, SizeF(CHILD_WIDTH, ANCHOR_CHILD_HEIGHT), SizeF(SELF_WIDTH, SELF_HEIGHT), prop, dialogPattern);
    EXPECT_TRUE(dialogPattern->IsHandednessLayoutCaptured());
    // Handedness anchor layout: target = anchor - child / 2, clamped to safe area bounds.
    EXPECT_NEAR(topLeftPoint.GetX(), 300.0f - CHILD_WIDTH / 2.0f, 16.0f);
    EXPECT_NEAR(topLeftPoint.GetY(), 200.0f - ANCHOR_CHILD_HEIGHT / 2.0f, 16.0f);
}
} // namespace OHOS::Ace::NG
