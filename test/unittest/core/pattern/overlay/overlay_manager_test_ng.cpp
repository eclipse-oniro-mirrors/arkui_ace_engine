/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "interfaces/native/ui_info_collection.h"

#include "core/accessibility/accessibility_manager.h"
#define private public
#define protected public
#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/ng/size_t.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "base/window/foldable_window.h"
#include "core/common/display_info.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/common/properties/ui_material.h"
#include "core/components/dialog/dialog_properties.h"
#include "core/components/dialog/dialog_theme.h"
#include "core/components/drag_bar/drag_bar_theme.h"
#include "core/components/select/select_theme.h"
#include "core/components/theme/icon_theme.h"
#include "core/components/toast/toast_theme.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/manager/drag_drop/drag_drop_global_controller.h"
#include "core/components_ng/pattern/bubble/bubble_event_hub.h"
#include "core/components_ng/pattern/bubble/bubble_pattern.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/container_modal/container_modal_pattern.h"
#include "core/components_ng/pattern/date_picker/picker_theme.h"
#include "core/components_ng/pattern/dialog/dialog_event_hub.h"
#include "core/components_ng/pattern/dialog/dialog_inner_manager.h"
#include "core/components_ng/pattern/dialog/dialog_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/menu/menu_theme.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/menu/preview/menu_preview_pattern.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/overlay/modal_presentation_pattern.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/sheet/sheet_drag_bar_paint_method.h"
#include "core/components_ng/pattern/sheet/sheet_drag_bar_pattern.h"
#include "core/components_ng/pattern/sheet/sheet_presentation_layout_algorithm.h"
#include "core/components_ng/pattern/sheet/sheet_presentation_pattern.h"
#include "core/components_ng/pattern/sheet/sheet_style.h"
#include "core/components_ng/pattern/sheet/sheet_theme.h"
#include "core/components_ng/pattern/sheet/sheet_view.h"
#include "core/components_ng/pattern/sheet/sheet_wrapper_pattern.h"
#include "core/components_ng/pattern/root/root_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/stage/stage_manager.h"
#include "core/components_ng/pattern/stage/stage_pattern.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/common/text_field_manager_ng.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/pattern/text_picker/textpicker_types.h"
#include "core/components_ng/pattern/toast/toast_layout_property.h"
#include "core/components_ng/pattern/toast/toast_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/components_ng/pattern/bubble/bridge/inner_modifier/bubble_inner_modifier.h"
#include "core/components_ng/pattern/menu/menu_manager.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/select_overlay/select_overlay_pattern.h"
#include "core/components_ng/manager/content_change_manager/content_change_manager.h"
#include "core/pipeline/base/element_register.h"
#include "test/mock/interfaces/inner_api/ui_session/mock_ui_session_manager.h"
#include "core/pipeline_ng/pipeline_context.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
const NG::BorderWidthProperty BORDER_WIDTH_TEST = { 1.0_vp, 1.0_vp, 1.0_vp, 0.0_vp };
const NG::BorderStyleProperty BORDER_STYLE_TEST = {BorderStyle::SOLID,
    BorderStyle::SOLID, BorderStyle::SOLID, BorderStyle::SOLID};
const NG::BorderColorProperty BORDER_COLOR_TEST = { Color::BLUE,
    Color::BLUE, Color::BLUE, Color::BLUE };
const NG::BorderWidthProperty NEW_BORDER_WIDTH_TEST = { 10.0_vp, 15.0_vp, 5.0_vp, 0.0_vp };
const NG::BorderStyleProperty NEW_BORDER_STYLE_TEST = {BorderStyle::SOLID,
    BorderStyle::DASHED, BorderStyle::DOTTED, BorderStyle::NONE};
const NG::BorderColorProperty NEW_BORDER_COLOR_TEST = { Color::RED,
    Color::GREEN, Color::GRAY, Color::BLACK };
const std::string TEXT_TAG = "text";
const OffsetF MENU_OFFSET(10.0, 10.0);
const std::string MESSAGE = "hello world";
const std::string BOTTOMSTRING = "test";
constexpr int32_t DURATION = 2;
constexpr float MINUS_HEIGHT = -5.0f;
constexpr float PIXELMAP_WIDTH = 100.0;
constexpr float PIXELMAP_HEIGHT = 100.0;
constexpr float BORDER_VALUE = 10.0;
const std::vector<std::string> FONT_FAMILY_VALUE = { "cursive" };

RefPtr<Theme> GetTheme(ThemeType type)
{
    if (type == DragBarTheme::TypeId()) {
        return AceType::MakeRefPtr<DragBarTheme>();
    } else if (type == IconTheme::TypeId()) {
        return AceType::MakeRefPtr<IconTheme>();
    } else if (type == DialogTheme::TypeId()) {
        return AceType::MakeRefPtr<DialogTheme>();
    } else if (type == PickerTheme::TypeId()) {
        return AceType::MakeRefPtr<PickerTheme>();
    } else if (type == SelectTheme::TypeId()) {
        return AceType::MakeRefPtr<SelectTheme>();
    } else if (type == MenuTheme::TypeId()) {
        return AceType::MakeRefPtr<MenuTheme>();
    } else if (type == ToastTheme::TypeId()) {
        return AceType::MakeRefPtr<ToastTheme>();
    } else if (type == SheetTheme::TypeId()) {
        auto sheetTheme = AceType::MakeRefPtr<SheetTheme>();
        sheetTheme->closeIconButtonWidth_ = SHEET_CLOSE_ICON_WIDTH;
        sheetTheme->centerDefaultWidth_ = SHEET_LANDSCAPE_WIDTH;
        return sheetTheme;
    } else {
        return nullptr;
    }
}
} // namespace

class OverlayManagerTestNg : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::function<RefPtr<UINode>(int32_t id)> builderFunc_;
    std::function<RefPtr<UINode>()> titleBuilderFunc_;
    RefPtr<FrameNode> sheetContentNode_;
    std::function<RefPtr<UINode>()> sheetTitleBuilderFunc_;
protected:
    static RefPtr<FrameNode> CreateBubbleNode(const TestProperty& testProperty);
    static RefPtr<FrameNode> CreateTargetNode();
    static void CreateSheetStyle(SheetStyle& sheetStyle);
    void CreateSheetBuilder();
    void CreateSheetContentNode();
    int32_t minPlatformVersion_ = 0;
};

void OverlayManagerTestNg::SetUp()
{
    minPlatformVersion_ = PipelineBase::GetCurrentContext()->GetMinPlatformVersion();
}

void OverlayManagerTestNg::TearDown()
{
    PipelineBase::GetCurrentContext()->SetMinPlatformVersion(minPlatformVersion_);
}

void OverlayManagerTestNg::SetUpTestCase()
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
void OverlayManagerTestNg::TearDownTestCase()
{
    MockPipelineContext::GetCurrent()->themeManager_ = nullptr;
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
}

RefPtr<FrameNode> OverlayManagerTestNg::CreateTargetNode()
{
    auto frameNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    return frameNode;
}

void OverlayManagerTestNg::CreateSheetStyle(SheetStyle& sheetStyle)
{
    if (!sheetStyle.sheetHeight.sheetMode.has_value()) {
        sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    }
    if (!sheetStyle.showDragBar.has_value()) {
        sheetStyle.showDragBar = true;
    }
}

void OverlayManagerTestNg::CreateSheetBuilder()
{
    auto builderFunc = [](int32_t id) -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };
    auto buildTitleNodeFunc = []() -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };
    builderFunc_ = builderFunc;
    titleBuilderFunc_ = buildTitleNodeFunc;
}

void OverlayManagerTestNg::CreateSheetContentNode()
{
    sheetContentNode_ =
    FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
    auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
    ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    sheetContentNode_->AddChild(childFrameNode);
    sheetTitleBuilderFunc_ = []() -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };
}

/**
 * @tc.name: DeleteModal001
 * @tc.desc: Test OverlayManager::DeleteModal
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, DeleteModal001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create target node and toast node.
     */
    auto targetNode = CreateTargetNode();
    auto targetId = targetNode->GetId();
    auto targetTag = targetNode->GetTag();
    auto toastId = ElementRegister::GetInstance()->MakeUniqueId();
    auto toastNode =
        FrameNode::CreateFrameNode(V2::TOAST_ETS_TAG, toastId, AceType::MakeRefPtr<BubblePattern>(targetId, targetTag));

    /**
     * @tc.steps: step2. create overlayManager and call ShowToast when rootElement is nullptr.
     * @tc.expected: toastMap_ is empty
     */
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    auto toastInfo =
        NG::ToastInfo { .message = MESSAGE, .duration = DURATION, .bottom = BOTTOMSTRING, .isRightToLeft = true };
    toastInfo.shadow = ShadowConfig::DefaultShadowL;
    overlayManager->ShowToast(toastInfo, nullptr);
    EXPECT_FALSE(overlayManager->toastMap_.empty());

    auto builderFunc = [](int32_t id) -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    auto buildTitleNodeFunc = []() -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    /**
     * @tc.steps: step3. create sheet node and run DeleteModal.
     */
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    bool isShow = true;
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc), std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    overlayManager->modalList_.emplace_back(nullptr);
    overlayManager->DeleteModal(targetId);
    EXPECT_EQ(overlayManager->modalList_.size(), 1);

    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc), std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    overlayManager->modalList_.emplace_back(nullptr);
    overlayManager->DeleteModal(targetId + 1);
    EXPECT_EQ(overlayManager->modalList_.size(), 3);
}

/**
 * @tc.name: OnBindSheet001
 * @tc.desc: Test OverlayManager::OnBindSheet create sheet page.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, OnBindSheet001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    ViewStackProcessor::GetInstance()->Push(targetNode);
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create builder func.
     */
    auto builderFunc = [](int32_t id) -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    auto buildTitleNodeFunc = []() -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    /**
     * @tc.steps: step3. create sheet node and get sheet node, get pattern.
     * @tc.expected: related function is called.
     */
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    bool isShow = true;
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc), std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    EXPECT_FALSE(topSheetNode == nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    EXPECT_FALSE(topSheetPattern == nullptr);
    auto sheetLayoutProperty = topSheetNode->GetLayoutProperty<SheetPresentationProperty>();
    EXPECT_FALSE(sheetLayoutProperty == nullptr);
    auto sheetChildren = topSheetNode->GetChildren();
    auto oprationNode = sheetChildren.front();
    EXPECT_FALSE(oprationNode == nullptr);
    auto scrollNode = *(std::next(sheetChildren.begin(), 1));
    EXPECT_FALSE(scrollNode == nullptr);
    auto closeIconNode = topSheetNode->GetLastChild();
    EXPECT_FALSE(closeIconNode == nullptr);
    auto sheetDragBarNode = topSheetPattern->GetDragBarNode();
    EXPECT_FALSE(sheetDragBarNode == nullptr);
    auto sheetDragBarPattern = sheetDragBarNode->GetPattern<SheetDragBarPattern>();
    EXPECT_FALSE(sheetDragBarPattern == nullptr);
    auto sheetDragBarPaintProperty = sheetDragBarNode->GetPaintProperty<SheetDragBarPaintProperty>();
    EXPECT_FALSE(sheetDragBarPaintProperty == nullptr);
    SheetStyle sheetStyle1;
    topSheetPattern->pageHeight_ = 10;

    // sheetStyle1.sheetHeight.sheetMode is null.
    sheetStyle1.sheetHeight.sheetMode = std::nullopt;
    topSheetPattern->sheetHeightForTranslate_ = 0;
    sheetStyle1.sheetHeight.height->SetUnit(DimensionUnit::PERCENT);
    sheetStyle1.sheetHeight.height->SetValue(2.0);
    overlayManager->ComputeSheetOffset(sheetStyle1, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 2));

    topSheetPattern->sheetHeightForTranslate_ = 0;
    sheetStyle1.sheetHeight.height->SetUnit(DimensionUnit::PERCENT);
    sheetStyle1.sheetHeight.height->SetValue(-2.0);
    overlayManager->ComputeSheetOffset(sheetStyle1, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 2));

    topSheetPattern->sheetHeightForTranslate_ = 0;
    sheetStyle1.sheetHeight.height->SetUnit(DimensionUnit::PERCENT);
    sheetStyle1.sheetHeight.height->SetValue(0.1);
    overlayManager->ComputeSheetOffset(sheetStyle1, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 1.0));

    topSheetPattern->sheetHeightForTranslate_ = 0;
    sheetStyle1.sheetHeight.height->SetUnit(DimensionUnit::VP);
    sheetStyle1.sheetHeight.height->SetValue(2);
    overlayManager->ComputeSheetOffset(sheetStyle1, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 2));

    // sheetStyle1.sheetHeight.sheetMode is not null.
    sheetStyle1.sheetHeight.sheetMode = SheetMode(5);
    topSheetPattern->sheetHeightForTranslate_ = 0;
    sheetStyle1.sheetHeight.height->SetUnit(DimensionUnit::PERCENT);
    sheetStyle1.sheetHeight.height->SetValue(2.0);
    overlayManager->ComputeSheetOffset(sheetStyle1, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 0));

    std::string title = "11";
    std::string subtitle = "22";
    sheetStyle1.sheetTitle = title;
    EXPECT_EQ(sheetStyle1.sheetTitle, title);
    sheetStyle1.sheetSubtitle = subtitle;
    EXPECT_EQ(sheetStyle1.sheetSubtitle, subtitle);
    std::stack<WeakPtr<FrameNode>> modalStack;
    overlayManager->modalStack_ = modalStack;
    EXPECT_FALSE(sheetDragBarPaintProperty == nullptr);
}

/**
 * @tc.name: OpenBindSheetByUIContext001
 * @tc.desc: Test OverlayManager::OpenBindSheetByUIContext create sheet page.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, OpenBindSheetByUIContext001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    ViewStackProcessor::GetInstance()->Push(targetNode);
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheet-content node.
     */
    auto sheetContentNode =
        FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
    auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    sheetContentNode->AddChild(childFrameNode);
    auto buildTitleNodeFunc = []() -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    /**
     * @tc.steps: step3. create sheet node and get sheet node, get pattern.
     * @tc.expected: related function is called.
     */
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OpenBindSheetByUIContext(sheetContentNode, std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    EXPECT_FALSE(topSheetNode == nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    EXPECT_FALSE(topSheetPattern == nullptr);
    auto sheetLayoutProperty = topSheetNode->GetLayoutProperty<SheetPresentationProperty>();
    EXPECT_FALSE(sheetLayoutProperty == nullptr);
    auto sheetChildren = topSheetNode->GetChildren();
    auto oprationNode = sheetChildren.front();
    EXPECT_FALSE(oprationNode == nullptr);
    auto scrollNode = *(std::next(sheetChildren.begin(), 1));
    EXPECT_FALSE(scrollNode == nullptr);
    auto closeIconNode = topSheetNode->GetLastChild();
    EXPECT_FALSE(closeIconNode == nullptr);
    auto sheetDragBarNode = topSheetPattern->GetDragBarNode();
    EXPECT_FALSE(sheetDragBarNode == nullptr);
    auto sheetDragBarPattern = sheetDragBarNode->GetPattern<SheetDragBarPattern>();
    EXPECT_FALSE(sheetDragBarPattern == nullptr);
    auto sheetDragBarPaintProperty = sheetDragBarNode->GetPaintProperty<SheetDragBarPaintProperty>();
    EXPECT_FALSE(sheetDragBarPaintProperty == nullptr);
    SheetStyle sheetStyle1;
    topSheetPattern->pageHeight_ = 10;

    // sheetStyle1.sheetHeight.sheetMode is null.
    sheetStyle1.sheetHeight.sheetMode = std::nullopt;
    topSheetPattern->sheetHeightForTranslate_ = 0;
    sheetStyle1.sheetHeight.height->SetUnit(DimensionUnit::PERCENT);
    sheetStyle1.sheetHeight.height->SetValue(2.0);
    overlayManager->ComputeSheetOffset(sheetStyle1, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 2));

    topSheetPattern->sheetHeightForTranslate_ = 0;
    sheetStyle1.sheetHeight.height->SetUnit(DimensionUnit::PERCENT);
    sheetStyle1.sheetHeight.height->SetValue(-2.0);
    overlayManager->ComputeSheetOffset(sheetStyle1, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 2));

    topSheetPattern->sheetHeightForTranslate_ = 0;
    sheetStyle1.sheetHeight.height->SetUnit(DimensionUnit::PERCENT);
    sheetStyle1.sheetHeight.height->SetValue(0.1);
    overlayManager->ComputeSheetOffset(sheetStyle1, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 1.0));

    topSheetPattern->sheetHeightForTranslate_ = 0;
    sheetStyle1.sheetHeight.height->SetUnit(DimensionUnit::VP);
    sheetStyle1.sheetHeight.height->SetValue(2);
    overlayManager->ComputeSheetOffset(sheetStyle1, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 2));

    // sheetStyle1.sheetHeight.sheetMode is not null.
    sheetStyle1.sheetHeight.sheetMode = SheetMode(5);
    topSheetPattern->sheetHeightForTranslate_ = 0;
    sheetStyle1.sheetHeight.height->SetUnit(DimensionUnit::PERCENT);
    sheetStyle1.sheetHeight.height->SetValue(2.0);
    overlayManager->ComputeSheetOffset(sheetStyle1, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 0));

    std::string title = "11";
    std::string subtitle = "22";
    sheetStyle1.sheetTitle = title;
    EXPECT_EQ(sheetStyle1.sheetTitle, title);
    sheetStyle1.sheetSubtitle = subtitle;
    EXPECT_EQ(sheetStyle1.sheetSubtitle, subtitle);
    std::stack<WeakPtr<FrameNode>> modalStack;
    overlayManager->modalStack_ = modalStack;
    EXPECT_FALSE(sheetDragBarPaintProperty == nullptr);
}

/**
 * @tc.name: UpdateBindSheetByUIContext001
 * @tc.desc: Test OverlayManager::UpdateBindSheetByUIContext create sheet page.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, UpdateBindSheetByUIContext001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    int32_t targetId = targetNode->GetId();
    ViewStackProcessor::GetInstance()->Push(targetNode);
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheet node.
     */
    auto sheetContentNode =
        FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
    auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    sheetContentNode->AddChild(childFrameNode);
    auto buildTitleNodeFunc = []() -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    /**
     * @tc.steps: step3. OpenBindSheetByUIContext
     */
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    SheetKey sheetKey = SheetKey(true, sheetContentNode->GetId(), targetId);
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OpenBindSheetByUIContext(sheetContentNode, std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, targetNode);

    /**
     * @tc.steps: step4. UpdateBindSheetByUIContext
     * @tc.expected: related property is updated.
     */
    sheetStyle.sheetHeight.height = Dimension(0.0f, DimensionUnit::AUTO);
    sheetStyle.sheetHeight.sheetMode = SheetMode::AUTO;
    sheetStyle.showDragBar = false;
    sheetStyle.showCloseIcon = false;
    sheetStyle.isTitleBuilder = false;
    sheetStyle.sheetType = SheetType::SHEET_BOTTOM_FREE_WINDOW;
    sheetStyle.backgroundColor = Color::BLACK;
    sheetStyle.maskColor = Color::BLACK;
    sheetStyle.sheetTitle = "Title";
    sheetStyle.sheetSubtitle = "SubTitle";
    sheetStyle.interactive = true;
    sheetStyle.scrollSizeMode = ScrollSizeMode::CONTINUOUS;
    sheetStyle.shadow = ShadowConfig::DefaultShadowL;
    sheetStyle.width = Dimension(0.0f, DimensionUnit::AUTO);
    overlayManager->UpdateBindSheetByUIContext(sheetContentNode, sheetStyle, targetId, false);

    auto sheetNode = overlayManager->sheetMap_[sheetKey].Upgrade();
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    auto currentStyle = layoutProperty->GetSheetStyleValue();

    EXPECT_EQ(currentStyle.sheetHeight.height, sheetStyle.sheetHeight.height);
    EXPECT_EQ(currentStyle.sheetHeight.sheetMode, sheetStyle.sheetHeight.sheetMode);
    EXPECT_EQ(currentStyle.showDragBar, sheetStyle.showDragBar);
    EXPECT_EQ(currentStyle.showCloseIcon, sheetStyle.showCloseIcon);
    EXPECT_EQ(currentStyle.isTitleBuilder, sheetStyle.isTitleBuilder);
    EXPECT_EQ(currentStyle.sheetType, sheetStyle.sheetType);
    EXPECT_EQ(currentStyle.backgroundColor, sheetStyle.backgroundColor);
    EXPECT_EQ(currentStyle.maskColor, sheetStyle.maskColor);
    EXPECT_EQ(currentStyle.sheetTitle, sheetStyle.sheetTitle);
    EXPECT_EQ(currentStyle.sheetSubtitle, sheetStyle.sheetSubtitle);
    EXPECT_EQ(currentStyle.interactive, sheetStyle.interactive);
    EXPECT_EQ(currentStyle.scrollSizeMode, sheetStyle.scrollSizeMode);
    EXPECT_EQ(currentStyle.shadow, sheetStyle.shadow);
    EXPECT_EQ(currentStyle.width, sheetStyle.width);
}

/**
 * @tc.name: UpdateBindSheetByUIContext002
 * @tc.desc: Test OverlayManager::UpdateBindSheetByUIContext create sheet page.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, UpdateBindSheetByUIContext002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    int32_t targetId = targetNode->GetId();
    ViewStackProcessor::GetInstance()->Push(targetNode);
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheet node.
     */
    auto sheetContentNode =
        FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
    auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    sheetContentNode->AddChild(childFrameNode);
    auto buildTitleNodeFunc = []() -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    /**
     * @tc.steps: step3. OpenBindSheetByUIContext
     */
    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.height = Dimension(0.0f, DimensionUnit::AUTO);
    sheetStyle.sheetHeight.sheetMode = SheetMode::AUTO;
    sheetStyle.showDragBar = false;
    sheetStyle.showCloseIcon = false;
    sheetStyle.isTitleBuilder = false;
    sheetStyle.sheetType = SheetType::SHEET_BOTTOM_FREE_WINDOW;
    sheetStyle.backgroundColor = Color::BLACK;
    sheetStyle.maskColor = Color::BLACK;
    sheetStyle.sheetTitle = "Title";
    sheetStyle.sheetSubtitle = "SubTitle";
    sheetStyle.interactive = true;
    sheetStyle.scrollSizeMode = ScrollSizeMode::CONTINUOUS;
    sheetStyle.shadow = ShadowConfig::DefaultShadowL;
    sheetStyle.width = Dimension(0.0f, DimensionUnit::AUTO);
    SheetKey sheetKey = SheetKey(true, sheetContentNode->GetId(), targetId);
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OpenBindSheetByUIContext(sheetContentNode, std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, targetNode);

    /**
     * @tc.steps: step4. UpdateBindSheetByUIContext
     * @tc.expected: related property is updated.
     */
    sheetStyle.sheetHeight.height = Dimension(0.0f, DimensionUnit::AUTO);
    sheetStyle.sheetHeight.sheetMode = SheetMode::AUTO;
    sheetStyle.showDragBar = false;
    sheetStyle.showCloseIcon = false;
    sheetStyle.isTitleBuilder = false;
    sheetStyle.sheetType = SheetType::SHEET_BOTTOM_FREE_WINDOW;
    sheetStyle.backgroundColor = Color::BLACK;
    sheetStyle.maskColor = Color::BLACK;
    sheetStyle.sheetTitle.reset();
    sheetStyle.sheetSubtitle.reset();
    sheetStyle.interactive.reset();
    sheetStyle.scrollSizeMode.reset();
    sheetStyle.shadow.reset();
    sheetStyle.width.reset();
    overlayManager->UpdateBindSheetByUIContext(sheetContentNode, sheetStyle, targetId, true);

    auto sheetNode = overlayManager->sheetMap_[sheetKey].Upgrade();
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    auto currentStyle = layoutProperty->GetSheetStyleValue();

    EXPECT_EQ(currentStyle.sheetHeight.height, sheetStyle.sheetHeight.height);
    EXPECT_EQ(currentStyle.sheetHeight.sheetMode, sheetStyle.sheetHeight.sheetMode);
    EXPECT_EQ(currentStyle.showDragBar, sheetStyle.showDragBar);
    EXPECT_EQ(currentStyle.showCloseIcon, sheetStyle.showCloseIcon);
    EXPECT_EQ(currentStyle.isTitleBuilder, sheetStyle.isTitleBuilder);
    EXPECT_EQ(currentStyle.sheetType, sheetStyle.sheetType);
    EXPECT_EQ(currentStyle.backgroundColor, sheetStyle.backgroundColor);
    EXPECT_EQ(currentStyle.maskColor, sheetStyle.maskColor);
    EXPECT_TRUE(currentStyle.sheetTitle.has_value());
    EXPECT_TRUE(currentStyle.sheetSubtitle.has_value());
    EXPECT_TRUE(currentStyle.interactive.has_value());
    EXPECT_TRUE(currentStyle.scrollSizeMode.has_value());
    EXPECT_TRUE(currentStyle.shadow.has_value());
    EXPECT_TRUE(currentStyle.width.has_value());
}

/**
 * @tc.name: UpdateBindSheetByUIContext003
 * @tc.desc: Test OverlayManager::UpdateBindSheetByUIContext create sheet page.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, UpdateBindSheetByUIContext003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. set api version over api 11.
     */
    int32_t orignApiVersion = AceApplicationInfo::GetInstance().GetApiTargetVersion();
    AceApplicationInfo::GetInstance().SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));

    /**
     * @tc.steps: step2. create target node.
     */
    auto targetNode = CreateTargetNode();
    int32_t targetId = targetNode->GetId();
    ViewStackProcessor::GetInstance()->Push(targetNode);
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step3. create sheetNode, get sheetPattern.
     */
    CreateSheetContentNode();
    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    sheetStyle.sheetType = SheetType::SHEET_BOTTOM;
    SheetKey sheetKey = SheetKey(true, sheetContentNode_->GetId(), targetId);
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OpenBindSheetByUIContext(sheetContentNode_, std::move(sheetTitleBuilderFunc_), sheetStyle, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

    /**
     * @tc.steps: step4. test set pageHeight = 1000.
     * @tc.expected: over api11 mediumPercent = 0.6, height = pageHeight_ * 0.6 = 1000 * 0.6 = 600.
     */
    topSheetPattern->pageHeight_ = 1000;
    float expectSheetHeight = topSheetPattern->pageHeight_ * 0.6;
    overlayManager->ComputeSheetOffset(sheetStyle, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, expectSheetHeight));

    /**
     * @tc.steps: step5. UpdateBindSheetByUIContext
     * @tc.expected: related property is isPartialUpdate.
     */
    SheetStyle updateSheetStyle;
    updateSheetStyle.backgroundColor = Color::BLACK;
    overlayManager->UpdateBindSheetByUIContext(sheetContentNode_, updateSheetStyle, targetId, true);

    auto sheetNode = overlayManager->sheetMap_[sheetKey].Upgrade();
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    auto currentStyle = layoutProperty->GetSheetStyleValue();
    ASSERT_TRUE(currentStyle.sheetHeight.sheetMode.has_value());
    EXPECT_EQ(currentStyle.sheetHeight.sheetMode, sheetStyle.sheetHeight.sheetMode);
    EXPECT_EQ(currentStyle.backgroundColor, updateSheetStyle.backgroundColor);
    EXPECT_EQ(topSheetPattern->sheetHeightForTranslate_, expectSheetHeight);

    /**
     * @tc.steps: step7. recover api version info.
     */
    AceApplicationInfo::GetInstance().SetApiTargetVersion(orignApiVersion);
}

/**
 * @tc.name: CloseBindSheetByUIContext001
 * @tc.desc: Test OverlayManager::CloseBindSheetByUIContext create sheet page.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, CloseBindSheetByUIContext001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    int32_t targetId = targetNode->GetId();
    ViewStackProcessor::GetInstance()->Push(targetNode);
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheet node.
     */
    auto sheetContentNode =
        FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
    auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
    sheetContentNode->AddChild(childFrameNode);
    auto buildTitleNodeFunc = []() -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    /**
     * @tc.steps: step3. OpenBindSheetByUIContext
     */
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    SheetKey sheetKey = SheetKey(true, sheetContentNode->GetId(), targetId);
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OpenBindSheetByUIContext(sheetContentNode, std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, targetNode);

    /**
     * @tc.steps: step4. CloseBindSheetByUIContext
     * @tc.expected: sheet is deleted.
     */
    overlayManager->CloseBindSheetByUIContext(sheetContentNode, targetId);

    auto iter = overlayManager->sheetMap_.find(sheetKey);
    EXPECT_EQ(iter, overlayManager->sheetMap_.end());
    EXPECT_TRUE(overlayManager->IsModalEmpty());
    EXPECT_TRUE(overlayManager->modalList_.empty());
}

/**
 * @tc.name: RemoveAllModalInOverlay001
 * @tc.desc: Test OverlayManager::RemoveAllModalInOverlay.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, RemoveAllModalInOverlay001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create builder.
     */
    auto builderFunc = [](int32_t id) -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    auto buildTitleNodeFunc = []() -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    /**
     * @tc.steps: step3. Run OnBindSheet to add something to modalStack and modalList.
     */
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    bool isShow = true;
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc), std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto sheetNode = overlayManager->modalStack_.top().Upgrade();
    EXPECT_EQ(sheetNode->GetTag(), V2::SHEET_PAGE_TAG);

    /**
     * @tc.steps: step4. run RemoveAllModalInOverlay func.
     */
    overlayManager->modalStack_.emplace(nullptr);
    overlayManager->modalList_.pop_back();
    EXPECT_TRUE(overlayManager->RemoveAllModalInOverlay());
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc), std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_TRUE(overlayManager->RemoveAllModalInOverlay());

    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc), std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    sheetNode = overlayManager->modalStack_.top().Upgrade();
    sheetNode->tag_ = V2::ROOT_ETS_TAG;
    EXPECT_TRUE(overlayManager->RemoveAllModalInOverlay());
}

/**
 * @tc.name: OnBindSheet002
 * @tc.desc: Test OverlayManager::OnBindSheet change sheetStyle dynamically.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, OnBindSheet002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create builder.
     */
    auto builderFunc = [](int32_t id) -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    auto buildTitleNodeFunc = []() -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    /**
     * @tc.steps: step3. create sheet node and get sheet node, get pattern.
     * @tc.expected: related function is called.
     */
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    bool isShow = true;
    auto onWillAppear = []() {};
    auto onAppear = []() {};
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc), std::move(buildTitleNodeFunc), sheetStyle,
        onAppear, nullptr, nullptr, nullptr, onWillAppear, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    EXPECT_FALSE(topSheetNode == nullptr);
    auto sheetNodeLayoutProperty = topSheetNode->GetLayoutProperty<SheetPresentationProperty>();
    auto style = sheetNodeLayoutProperty->GetSheetStyle();
    EXPECT_EQ(style->sheetHeight.sheetMode.value(), SheetMode::MEDIUM);
    EXPECT_EQ(style->showDragBar.value(), true);

    /**
     * @tc.steps: step4. Change the sheetStyle.
     * @tc.expected: the sheetStyle is updated successfully
     */
    sheetStyle.sheetHeight.sheetMode = SheetMode::AUTO;
    sheetStyle.showDragBar = false;
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc), std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    auto sheetNode = overlayManager->modalStack_.top().Upgrade();
    EXPECT_FALSE(topSheetNode == nullptr);
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    sheetPattern->InitialLayoutProps();
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc), std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    sheetNode = overlayManager->modalStack_.top().Upgrade();
    sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    sheetPattern->InitialLayoutProps();
    EXPECT_EQ(sheetPattern->GetTargetId(), topSheetNode->GetPattern<SheetPresentationPattern>()->GetTargetId());
    sheetNodeLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    style = sheetNodeLayoutProperty->GetSheetStyle();
    EXPECT_EQ(style->sheetHeight.sheetMode.value(), SheetMode::MEDIUM);
    EXPECT_EQ(style->showDragBar.value(), false);

    /**
     * @tc.steps: step4. Change the backgroundColor.
     * @tc.expected: the backgroundColor is updated successfully
     */
    sheetStyle.backgroundColor = Color::GREEN;
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc), std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    sheetNode = overlayManager->modalStack_.top().Upgrade();
    EXPECT_FALSE(topSheetNode == nullptr);
    EXPECT_EQ(sheetNode->GetRenderContext()->GetBackgroundColorValue(), Color::GREEN);
    overlayManager->OnBindSheet(!isShow, nullptr, nullptr, nullptr, sheetStyle, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
}

/**
 * @tc.name: OnBindSheet003
 * @tc.desc: Test OverlayManager::OnBindSheet destroy sheet node.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, OnBindSheet003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto targetId = targetNode->GetId();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create builder.
     */
    auto builderFunc = [](int32_t id) -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    auto buildTitleNodeFunc = []() -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    /**
     * @tc.steps: step3. create sheet node.
     * @tc.expected: Make sure the modalStack holds the sheetNode.
     */
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    bool isShow = true;
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc), std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto sheetNode = overlayManager->modalStack_.top().Upgrade();
    EXPECT_EQ(sheetNode->GetTag(), V2::SHEET_PAGE_TAG);

    /**
     * @tc.steps: step4. destroy modal page.
     * @tc.expected: destroy modal successfully.
     */
    auto onWillDisappear = []() {};
    auto onDisappear = []() {};
    overlayManager->OnBindSheet(!isShow, nullptr, nullptr, nullptr, sheetStyle, nullptr, onDisappear, nullptr,
        nullptr, nullptr, onWillDisappear, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    overlayManager->modalList_.emplace_back(AceType::WeakClaim(AceType::RawPtr(stageNode)));
    overlayManager->RemoveSheet(sheetNode);
    overlayManager->FindWindowScene(targetNode);
    overlayManager->DeleteModal(targetId);
    EXPECT_TRUE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: GetSheetMask001
 * @tc.desc: Test OverlayManager::GetSheetMask.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, GetSheetMask001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto targetId = targetNode->GetId();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create builder.
     */
    auto builderFunc = [](int32_t id) -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    auto buildTitleNodeFunc = []() -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    /**
     * @tc.steps: step3. create sheet node.
     * @tc.expected: Make sure the modalStack holds the sheetNode.
     */
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    bool isShow = true;
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc), std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto sheetNode = overlayManager->modalStack_.top().Upgrade();
    EXPECT_EQ(sheetNode->GetTag(), V2::SHEET_PAGE_TAG);

    /**
     * @tc.steps: step4. Run GetSheetMask Func.
     * @tc.expected: if the color is set, Make sure the maskNode is exist and it's color is right.
     */
    auto maskNode = overlayManager->GetSheetMask(sheetNode);
    auto onWillDisappear = []() {};
    auto onDisappear = []() {};
    overlayManager->OnBindSheet(!isShow, nullptr, nullptr, nullptr, sheetStyle, nullptr, onDisappear, nullptr, nullptr,
    nullptr, onWillDisappear, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    sheetStyle.maskColor = Color::BLUE;
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc), std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    sheetNode = overlayManager->modalStack_.top().Upgrade();
    EXPECT_FALSE(sheetNode == nullptr);
    EXPECT_EQ(sheetNode->GetTag(), V2::SHEET_PAGE_TAG);
    maskNode = overlayManager->GetSheetMask(sheetNode);
    EXPECT_FALSE(maskNode == nullptr);
    EXPECT_EQ(maskNode->GetTag(), V2::SHEET_WRAPPER_TAG);
    EXPECT_EQ(maskNode->GetRenderContext()->GetBackgroundColorValue(), Color::BLUE);

    /**
     * @tc.steps: step5. destroy sheetNode.
     * @tc.expected: Make sure the maskNode is destroyed.
     */
    overlayManager->OnBindSheet(!isShow, nullptr, nullptr, nullptr, sheetStyle, nullptr, onDisappear, nullptr, nullptr,
        nullptr, onWillDisappear, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    overlayManager->modalList_.emplace_back(AceType::WeakClaim(AceType::RawPtr(stageNode)));
    overlayManager->RemoveSheet(sheetNode);
    overlayManager->FindWindowScene(targetNode);
    overlayManager->DeleteModal(targetId);
    EXPECT_TRUE(overlayManager->modalStack_.empty());
}

/**
 * @tc.name: SheetPresentationPattern1
 * @tc.desc: Test SheetPresentationPattern create sheet page.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern1, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();
    /**
     * @tc.steps: step2. create builder.
     */
    auto builderFunc = [](int32_t id) -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::BUTTON_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ButtonPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    auto buildTitleNodeFunc = []() -> RefPtr<UINode> {
        auto frameNode =
            FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
                []() { return AceType::MakeRefPtr<LinearLayoutPattern>(true); });
        auto childFrameNode = FrameNode::GetOrCreateFrameNode(V2::TEXT_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
        frameNode->AddChild(childFrameNode);
        return frameNode;
    };

    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    bool isShow = true;
    auto onWillAppear = []() {};
    auto onAppear = []() {};
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc), std::move(buildTitleNodeFunc), sheetStyle,
        onAppear, nullptr, nullptr, nullptr, onWillAppear, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    EXPECT_FALSE(topSheetNode == nullptr);
    auto safeAreaManager = AceType::MakeRefPtr<SafeAreaManager>();
    MockPipelineContext::GetCurrent()->safeAreaManager_ = safeAreaManager;
    auto sheetNodeLayoutProperty = topSheetNode->GetLayoutProperty<SheetPresentationProperty>();
    auto style = sheetNodeLayoutProperty->GetSheetStyle();
    EXPECT_EQ(style->sheetHeight.sheetMode.value(), SheetMode::MEDIUM);
    EXPECT_EQ(style->showDragBar.value(), true);

    sheetStyle.sheetHeight.sheetMode = SheetMode::LARGE;
    sheetStyle.showDragBar = false;
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc), std::move(buildTitleNodeFunc), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    auto sheetNode = overlayManager->modalStack_.top().Upgrade();
    EXPECT_FALSE(topSheetNode == nullptr);
    auto geometryNode = sheetNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    EXPECT_EQ(sheetPattern->GetTargetId(), topSheetNode->GetPattern<SheetPresentationPattern>()->GetTargetId());
    sheetPattern->InitPanEvent();
    GestureEvent info;
    sheetPattern->HandleDragUpdate(info);
    sheetPattern->HandleDragEnd({});
    sheetNodeLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    style = sheetNodeLayoutProperty->GetSheetStyle();
    auto sheetLayoutAlgorithm = sheetPattern->CreateLayoutAlgorithm();
    ASSERT_NE(sheetLayoutAlgorithm, nullptr);
    RefPtr<LayoutWrapperNode> layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        AceType::WeakClaim(AceType::RawPtr(sheetNode)), geometryNode->Clone(), sheetNodeLayoutProperty->Clone());
    EXPECT_FALSE(layoutWrapper == nullptr);
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(sheetLayoutAlgorithm));
    DirtySwapConfig dirtySwapConfig;
    EXPECT_TRUE(sheetPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, dirtySwapConfig));
    sheetPattern->InitPanEvent();
    EXPECT_EQ(style->sheetHeight.sheetMode.value(), SheetMode::LARGE);
    EXPECT_EQ(style->showDragBar.value(), false);
}

/**
 * @tc.name: OnBindSheet004
 * @tc.desc: Test OverlayManager::OnBindSheet create center sheet page.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, OnBindSheet004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, targetNode);
    // assert
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

    /**
     * @tc.steps: step3. test sheetThemeType_ = popup, sheetStyle.sheetType = center.
     * @tc.expected: height = (setHeight + screenHeight) / 2.
     */
    topSheetPattern->sheetThemeType_ = "popup";
    auto layoutProperty = topSheetPattern->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateSheetStyle(sheetStyle);
    sheetStyle.sheetType = SheetType::SHEET_CENTER;
    topSheetPattern->pageHeight_ = 1000;
    auto setSheetSize = SizeF({ 500, 500 });
    topSheetNode->GetGeometryNode()->SetFrameSize(setSheetSize);
    topSheetPattern->sheetHeightForTranslate_ = 0;
    overlayManager->ComputeSheetOffset(sheetStyle, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 0));

    sheetStyle.sheetType = SheetType::SHEET_POPUP;
    layoutProperty->UpdateSheetStyle(sheetStyle);
    overlayManager->ComputeSheetOffset(sheetStyle, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 0));
    sheetStyle.sheetType = SheetType::SHEET_BOTTOMLANDSPACE;
    layoutProperty->UpdateSheetStyle(sheetStyle);
    overlayManager->ComputeSheetOffset(sheetStyle, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 0));
}

/**
 * @tc.name: OnBindSheet005
 * @tc.desc: Test OverlayManager::OnBindSheet create detent sheet page.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, OnBindSheet005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

    /**
     * @tc.steps: step3. test sheetThemeType_ = auto, sheetStyle.sheetType = bottom, set pageHeight = 1000.
     */
    topSheetPattern->sheetThemeType_ = "auto";
    sheetStyle.sheetType = SheetType::SHEET_BOTTOM;
    topSheetPattern->pageHeight_ = 1000;

    /**
     * @tc.steps: step4. test sheetStyle.detents.sheetMode has value, sheetMode = MEDIUM.
     * @tc.expected: height = pageHeight_*0.6 = 1000*0.6 = 600.
     */
    SheetHeight detent;
    detent.sheetMode = SheetMode::MEDIUM;
    sheetStyle.detents.emplace_back(detent);
    topSheetPattern->sheetHeightForTranslate_ = 0;
    overlayManager->ComputeSheetOffset(sheetStyle, topSheetNode);
    EXPECT_FALSE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 500));
    sheetStyle.detents.clear();

    /**
     * @tc.steps: step5. test sheetStyle.detents.sheetMode has value, sheetMode = MEDIUM.
     * @tc.expected: height = pageHeight_-8 = 1000-8 = 992.
     */
    detent.sheetMode = SheetMode::LARGE;
    sheetStyle.detents.emplace_back(detent);
    topSheetPattern->sheetHeightForTranslate_ = 0;
    overlayManager->ComputeSheetOffset(sheetStyle, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 992));

    /**
     * @tc.steps: step6. test sheetStyle.detents.height has value, height unit is %.
     * @tc.expected: height = pageHeight*0.5 = 1000*0.5 = 500.
     */
    sheetStyle.detents.clear();
    detent.sheetMode = std::nullopt;
    Dimension detentHeight { 0.5, DimensionUnit::PERCENT };
    detent.height = detentHeight;
    sheetStyle.detents.emplace_back(detent);
    topSheetPattern->sheetHeightForTranslate_ = 0;
    overlayManager->ComputeSheetOffset(sheetStyle, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 500));

    /**
     * @tc.steps: step7. test sheetStyle.detents.height has value, height unit is vp.
     * @tc.expected: height = setHeight = 600.
     */
    sheetStyle.detents.clear();
    detent.height->SetUnit(DimensionUnit::VP);
    detent.height->SetValue(600);
    sheetStyle.detents.emplace_back(detent);
    topSheetPattern->sheetHeightForTranslate_ = 0;
    overlayManager->ComputeSheetOffset(sheetStyle, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 600));

    /**
     * @tc.steps: step8. test sheetStyle.detents.height has value, height unit is vp, setHeight > maxHeight.
     * @tc.expected: height = setHeight = maxHeight = pageHeight-8 = 992.
     */
    sheetStyle.detents.clear();
    detent.height->SetUnit(DimensionUnit::VP);
    detent.height->SetValue(1500);
    sheetStyle.detents.emplace_back(detent);
    topSheetPattern->sheetHeightForTranslate_ = 0;
    overlayManager->ComputeSheetOffset(sheetStyle, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 992));

    /**
     * @tc.steps: step9. test sheetStyle.detents.height has value, height unit is vp, setHeight < 0.
     * @tc.expected: height = setHeight = maxHeight = pageHeight-8 = 992.
     */
    sheetStyle.detents.clear();
    detent.height->SetUnit(DimensionUnit::VP);
    detent.height->SetValue(-100);
    sheetStyle.detents.emplace_back(detent);
    topSheetPattern->sheetHeightForTranslate_ = 0;
    overlayManager->ComputeSheetOffset(sheetStyle, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 992));
}

/**
 * @tc.name: OnBindSheet006
 * @tc.desc: Test OverlayManager::PlayBubbleStyleSheetTransition.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, OnBindSheet006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

    /**
     * @tc.steps: step3. test PlayBubbleStyleSheetTransition().
     */
    overlayManager->PlayBubbleStyleSheetTransition(topSheetNode, true);
    EXPECT_EQ(topSheetPattern->height_, topSheetPattern->sheetHeightForTranslate_);
}

/**
 * @tc.name: IsNeedAvoidFoldCrease001
 * @tc.desc: Test IsNeedAvoidFoldCrease with EXTEND source mode in expand display
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, IsNeedAvoidFoldCrease001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create frameNode with pipeline context
     */
    auto frameNode = OverlayManagerTestNg::CreateTargetNode();
    ASSERT_NE(frameNode, nullptr);

    /**
     * @tc.steps: step2. Set DisplaySourceMode to EXTEND to trigger branch
     */
    auto mockContainer = MockContainer::Current();
    ASSERT_NE(mockContainer, nullptr);

    auto displayInfo = mockContainer->GetMockDisplayInfo();
    ASSERT_NE(displayInfo, nullptr);
    displayInfo->SetDisplaySourceMode(DisplaySourceMode::EXTEND);
    mockContainer->SetDisplayInfo(displayInfo);

    /**
     * @tc.steps: step3. Call IsNeedAvoidFoldCrease with expandDisplay=true
     * @tc.expected: Returns false because branch is taken:
     *               if (expandDisplay && sourceMode == DisplaySourceMode::EXTEND)
     */
    bool result = OverlayManager::IsNeedAvoidFoldCrease(frameNode, false, true, std::nullopt);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: HandleDragUpdate001
 * @tc.desc: Test SheetPresentationPattern::HandleDragUpdate().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, HandleDragUpdate001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);
    topSheetPattern->OnDirtyLayoutWrapperSwap(topSheetNode->CreateLayoutWrapper(), DirtySwapConfig());

    /**
     * @tc.steps: step3. Init height , sheetDetentHeight and currentOffset, set minDelta < 0.
     * @tc.expected: currentOffset is -5.
     */
    topSheetPattern->sheetDetentHeight_.emplace_back(20);
    topSheetPattern->sheetDetentHeight_.emplace_back(30);
    topSheetPattern->height_ = 20;
    topSheetPattern->pageHeight_ = 50;
    topSheetPattern->sheetMaxHeight_ = 30;
    topSheetPattern->HandleDragStart();
    GestureEvent info;
    info.SetMainDelta(MINUS_HEIGHT);
    topSheetPattern->HandleDragUpdate(info);
    EXPECT_TRUE(NearEqual(topSheetPattern->currentOffset_, -5));

    /**
     * @tc.steps: step4. Init height , sheetDetentHeight and set currentOffset < 0,
     * @tc.expected: currentOffset = height_ - sheetMaxHeight_.
     */
    topSheetPattern->currentOffset_ = -5;
    topSheetPattern->HandleDragUpdate(info);
    EXPECT_TRUE(NearEqual(topSheetPattern->currentOffset_, -10));
}

/**
 * @tc.name: TestOnBindSheet
 * @tc.desc: SheetPresentationPattern::HandleScroll
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, HandleScroll001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);
    topSheetPattern->InitSheetObject();
    auto sheetObject = topSheetPattern->GetSheetObject();
    ASSERT_NE(sheetObject, nullptr);
    topSheetPattern->OnDirtyLayoutWrapperSwap(topSheetNode->CreateLayoutWrapper(), DirtySwapConfig());

    /**
     * @tc.steps: step3. Set currentOffset_ = 0, NestedState = CHILD_SCROLL.
     */
    topSheetPattern->currentOffset_ = 0;
    auto result = sheetObject->HandleScroll(20.f, SCROLL_FROM_UPDATE, NestedState::CHILD_SCROLL);
    EXPECT_TRUE(result.reachEdge);
    EXPECT_EQ(result.remain, 0);

    /**
     * @tc.steps: step4. Set currentOffset_ = 0, NestedState = CHILD_OVER_SCROLL.
     */
    topSheetPattern->currentOffset_ = 0;
    result = sheetObject->HandleScroll(20.f, SCROLL_FROM_UPDATE, NestedState::CHILD_OVER_SCROLL);
    EXPECT_TRUE(result.reachEdge);
    EXPECT_EQ(result.remain, 20.f);

    /**
     * @tc.steps: step5. Set currentOffset_ < 0, NestedState = CHILD_SCROLL.
     */
    topSheetPattern->currentOffset_ = -5;
    result = sheetObject->HandleScroll(20.f, SCROLL_FROM_UPDATE, NestedState::CHILD_SCROLL);
    EXPECT_TRUE(result.reachEdge);
    EXPECT_EQ(result.remain, 20.f);
}

/**
 * @tc.name: TestOnBindSheet
 * @tc.desc: SheetPresentationPattern::HandleScroll
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, HandleScroll002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    CHECK_NULL_VOID(targetNode);
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    CHECK_NULL_VOID(stageNode);
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    CHECK_NULL_VOID(rootNode);
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    CreateSheetBuilder();
    SheetStyle sheetStyle;
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    CHECK_NULL_VOID(overlayManager);
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);
    topSheetPattern->InitSheetObject();
    auto sheetObject = topSheetPattern->GetSheetObject();
    ASSERT_NE(sheetObject, nullptr);
    topSheetPattern->OnDirtyLayoutWrapperSwap(topSheetNode->CreateLayoutWrapper(), DirtySwapConfig());
    /**
     * @tc.steps: step3. Set source = SCROLL_FROM_JUMP, NestedState = CHILD_SCROLL, deal with HandleDragEnd.
     */
    topSheetPattern->currentOffset_ = 100.0f;
    sheetObject->isSheetNeedScroll_ = true;
    auto result = sheetObject->HandleScroll(20.f, SCROLL_FROM_ANIMATION, NestedState::CHILD_SCROLL);
    EXPECT_FALSE(sheetObject->isSheetPosChanged_);
    EXPECT_TRUE(result.reachEdge);
    EXPECT_EQ(result.remain, 0);
    /**
     * @tc.steps: step4. isSheetNeedScroll_ = false, and will Not HandleDragEnd.
     */
    topSheetPattern->currentOffset_ = 0;
    sheetObject->isSheetNeedScroll_ = false;
    result = sheetObject->HandleScroll(20.f, SCROLL_FROM_ANIMATION, NestedState::CHILD_SCROLL);
    EXPECT_FALSE(sheetObject->isSheetNeedScroll_);
    EXPECT_TRUE(result.reachEdge);
    EXPECT_EQ(result.remain, 20.f);
}

/**
 * @tc.name: TestOnBindSheet
 * @tc.desc: Test SheetPresentationPattern::OnDirtyLayoutWrapperSwap() and root Rotates.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, TestSheetAvoidSafeArea2, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create sheet node and parent node ,then sheet node mount to parent node and initialize
     * sheet pattern.
     */
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    auto sheetNode = FrameNode::CreateFrameNode(
        V2::SHEET_PAGE_TAG, 1, AceType::MakeRefPtr<SheetPresentationPattern>(-1, V2::BUTTON_ETS_TAG, nullptr));
    ASSERT_NE(sheetNode, nullptr);
    sheetNode->MountToParent(rootNode);
    auto dragBarNode = FrameNode::CreateFrameNode(
        "SheetDragBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SheetDragBarPattern>());
    ASSERT_NE(dragBarNode, nullptr);
    auto scroll = FrameNode::CreateFrameNode(
        V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
    ASSERT_NE(scroll, nullptr);
    dragBarNode->MountToParent(sheetNode);
    auto contentNode = FrameNode::CreateFrameNode(
        "SheetDragBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SheetDragBarPattern>());
    ASSERT_NE(contentNode, nullptr);
    contentNode->MountToParent(scroll);
    scroll->MountToParent(sheetNode);
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    auto renderContext = sheetNode->GetRenderContext();
    auto safeAreaManager = AceType::MakeRefPtr<SafeAreaManager>();
    auto geometryNode = sheetNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    auto sheetLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(sheetLayoutProperty, nullptr);
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);
    geometryNode->SetFrameSize(SizeF(800, 2000));
    MockPipelineContext::GetCurrent()->safeAreaManager_ = safeAreaManager;
    MockPipelineContext::GetCurrent()->SetRootSize(800, 2000);
    sheetPattern->pageHeight_ = 2000;
    sheetPattern->UpdateSheetType();
    sheetPattern->InitSheetObject();
    ASSERT_NE(sheetPattern->GetSheetObject(), nullptr);
    sheetPattern->GetSheetObject()->SetSheetHeight(2000);
    sheetPattern->height_ = 500;
    auto sheetLayoutAlgorithm = sheetPattern->CreateLayoutAlgorithm();
    AceType::DynamicCast<SheetPresentationLayoutAlgorithm>(sheetLayoutAlgorithm)->sheetMaxHeight_ = 500;
    ASSERT_NE(sheetLayoutAlgorithm, nullptr);
    auto layoutWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(sheetNode, sheetNode->GetGeometryNode(), sheetNode->GetLayoutProperty());
    layoutWrapper->SetLayoutAlgorithm(AccessibilityManager::MakeRefPtr<LayoutAlgorithmWrapper>(sheetLayoutAlgorithm));
    /**
     * @tc.cases: case1. window rotates after layout.
     */
    sheetPattern->OnWindowSizeChanged(2000, 800, WindowSizeChangeReason::ROTATION);
    sheetPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, DirtySwapConfig());
    EXPECT_EQ(static_cast<int>(renderContext->GetTransformTranslate()->y.ConvertToPx()),
        sheetPattern->pageHeight_ - sheetPattern->height_);
    /**
     * @tc.cases: case2. window rotates to vertical screen.
     */
    sheetPattern->OnWindowSizeChanged(800, 2000, WindowSizeChangeReason::RESIZE);
    sheetPattern->OnDirtyLayoutWrapperSwap(layoutWrapper, DirtySwapConfig());
    EXPECT_FALSE(sheetPattern->windowRotate_);
}

/**
 * @tc.type: FUNC
 * @tc.name: Test BindSheet
 * @tc.desc: Test SheetPresentationPattern::AvoidSafeArea() when sheetType is Center.
 */
HWTEST_F(OverlayManagerTestNg, TestSheetAvoidSafeArea3, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    /**
     * @tc.steps: step1. create sheet node and initialize sheet pattern.
     */
    auto sheetNode = FrameNode::CreateFrameNode(
        V2::SHEET_PAGE_TAG, 1, AceType::MakeRefPtr<SheetPresentationPattern>(-1, V2::BUTTON_ETS_TAG, nullptr));
    auto dragBarNode = FrameNode::CreateFrameNode(
        "SheetDragBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SheetDragBarPattern>());
    auto scroll = FrameNode::CreateFrameNode(
        V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
    auto builderContent =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    builderContent->MountToParent(scroll);
    dragBarNode->MountToParent(sheetNode);
    scroll->MountToParent(sheetNode);
    sheetNode->GetFocusHub()->currentFocus_ = true;
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    sheetPattern->SetScrollNode(WeakPtr<FrameNode>(scroll));
    sheetPattern->sheetType_ = SheetType::SHEET_CENTER;
    auto renderContext = sheetNode->GetRenderContext();
    auto safeAreaManager = AceType::MakeRefPtr<SafeAreaManager>();
    auto geometryNode = sheetNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetFrameSize(SizeF(800, 1800));
    MockPipelineContext::GetCurrent()->safeAreaManager_ = safeAreaManager;
    MockPipelineContext::GetCurrent()->SetRootSize(800, 2000);
    auto textFieldManager = AceType::MakeRefPtr<TextFieldManagerNG>();
    textFieldManager->SetHeight(20);
    MockPipelineContext::GetCurrent()->SetTextFieldManager(textFieldManager);
    SafeAreaInsets::Inset upKeyboard { 0, 200 };
    sheetPattern->pageHeight_ = 2000;
    sheetPattern->UpdateSheetType();
    sheetPattern->InitSheetObject();
    ASSERT_NE(sheetPattern->GetSheetObject(), nullptr);
    sheetPattern->GetSheetObject()->SetSheetHeight(1800);
    auto sheetLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    EXPECT_FALSE(sheetLayoutProperty == nullptr);
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);
    /**
     * @tc.steps: step2. keyboard up, and sheet will goes to correct position.
     * @tc.cases: case1. keyboard up, but sheet needs not up beacure hsafe is enough.
     */
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    ASSERT_NE(pattern->selectController_, nullptr);

    safeAreaManager->keyboardInset_ = upKeyboard;
    pattern->selectController_->caretInfo_.rect.SetTop(1000.0f);
    textFieldManager->onFocusTextField_ = AceType::DynamicCast<Pattern>(pattern);
    sheetPattern->height_ = 1800;
    sheetPattern->AvoidSafeArea();
    EXPECT_EQ(sheetPattern->keyboardHeight_, 200);
    /**
     * @tc.cases: case2. keyboard up, sheet needs not to go up.
     */
    sheetPattern->keyboardHeight_ = 0;
    textFieldManager->SetClickPosition(Offset(500, 300));
    sheetPattern->AvoidSafeArea();
    EXPECT_EQ(static_cast<int>(renderContext->GetTransformTranslate()->y.ConvertToPx()), 2000 - sheetPattern->height_);
    /**
     * @tc.cases: case3. sheet offset = 1800, sheet goes up with h and not goes up to LARGE.
     */
    sheetPattern->keyboardHeight_ = 0;
    pattern->selectController_->caretInfo_.rect.SetTop(1900.0f);
    textFieldManager->onFocusTextField_ = AceType::DynamicCast<Pattern>(pattern);
    sheetPattern->AvoidSafeArea();
    EXPECT_EQ(static_cast<int>(renderContext->GetTransformTranslate()->y.ConvertToPx()), 56);
    EXPECT_FALSE(sheetPattern->isScrolling_);
    /**
     * @tc.cases: case4. sheet offset = 1800, sheet goes up to LARGE and scrolling.
     */
    sheetPattern->keyboardHeight_ = 0;
    sheetPattern->height_ = 1950;
    sheetPattern->AvoidSafeArea();
    EXPECT_EQ(static_cast<int>(renderContext->GetTransformTranslate()->y.ConvertToPx()), 8);
    EXPECT_EQ(sheetPattern->scrollHeight_, 102.0f);
    EXPECT_TRUE(sheetPattern->isScrolling_);
    /**
     * @tc.cases: case5. softkeyboard is down.
     */
    SafeAreaInsets::Inset downKeyboard { 0, 0 };
    safeAreaManager->keyboardInset_ = downKeyboard;
    sheetPattern->AvoidSafeArea();
    EXPECT_EQ(static_cast<int>(renderContext->GetTransformTranslate()->y.ConvertToPx()), 50);
    EXPECT_EQ(sheetPattern->keyboardHeight_, 0);
    EXPECT_FALSE(sheetPattern->isScrolling_);
}

/**
 * @tc.type: FUNC
 * @tc.name: Test BindSheet keyboardAvoidMode
 * @tc.desc: Test SheetPresentationPattern::AvoidSafeArea() when sheetType is Bottom,
 *           and SheetKeyboardAvoidMode is NONE When api version greater than 12.
 */
HWTEST_F(OverlayManagerTestNg, TestSheetAvoidSafeArea4, TestSize.Level1)
{
    /**
     * @tc.steps: step1. initalize environment and set api version to api 13.
     */
    int32_t backupApiVersion = AceApplicationInfo::GetInstance().GetApiTargetVersion();
    AceApplicationInfo::GetInstance().SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_THIRTEEN));

    /**
     * @tc.steps: step2. create sheet node and initialize sheet pattern.
     */
    auto sheetNode = FrameNode::CreateFrameNode(
        V2::SHEET_PAGE_TAG, 1, AceType::MakeRefPtr<SheetPresentationPattern>(-1, V2::BUTTON_ETS_TAG, nullptr));
    auto dragBarNode = FrameNode::CreateFrameNode(
        "SheetDragBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SheetDragBarPattern>());
    auto scroll = FrameNode::CreateFrameNode(
        V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
    auto builderContent =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    builderContent->MountToParent(scroll);
    dragBarNode->MountToParent(sheetNode);
    scroll->MountToParent(sheetNode);
    sheetNode->GetFocusHub()->currentFocus_ = true;
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    sheetPattern->sheetType_ = SheetType::SHEET_BOTTOM;
    auto renderContext = sheetNode->GetRenderContext();
    auto safeAreaManager = AceType::MakeRefPtr<SafeAreaManager>();
    auto geometryNode = sheetNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetFrameSize(SizeF(800, 1800));
    MockPipelineContext::GetCurrent()->safeAreaManager_ = safeAreaManager;
    MockPipelineContext::GetCurrent()->SetRootSize(800, 2000);
    auto textFieldManager = AceType::MakeRefPtr<TextFieldManagerNG>();
    textFieldManager->SetHeight(20);
    MockPipelineContext::GetCurrent()->SetTextFieldManager(textFieldManager);
    SafeAreaInsets::Inset upKeyboard { 0, 600 };
    SafeAreaInsets::Inset emptyKeyboard { 0, 0 };
    sheetPattern->pageHeight_ = 2000;
    sheetPattern->UpdateSheetType();
    sheetPattern->InitSheetObject();
    ASSERT_NE(sheetPattern->GetSheetObject(), nullptr);
    sheetPattern->GetSheetObject()->SetSheetHeight(1800);
    sheetPattern->height_ = 1800;
    auto sheetLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    EXPECT_FALSE(sheetLayoutProperty == nullptr);
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    if (!sheetStyle.sheetKeyboardAvoidMode.has_value()) {
        sheetStyle.sheetKeyboardAvoidMode = SheetKeyboardAvoidMode::NONE;
    }
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);

    /**
     * @tc.steps: step3. set sheet keyboardAvoidMode to NONE.
     */
    sheetPattern->keyboardAvoidMode_ =
        sheetStyle.sheetKeyboardAvoidMode.value_or(SheetKeyboardAvoidMode::TRANSLATE_AND_SCROLL);

    /**
     * @tc.steps: step4. keyboard up, and sheet will not to go up beacure keyboardAvoidMode is NONE.
     * @tc.cases: case1. keyboard up to height 600, click position above keyboard, but sheet needs not up.
     * @tc.expected: The sheetHeightUp value is zero, and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1000));
    sheetPattern->AvoidSafeArea();
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case1. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. keyboard up to height 600, click position under keyboard, but sheet needs not up.
     * @tc.expected: The sheetHeightUp value is zero, and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1500));
    sheetPattern->AvoidSafeArea();
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. keyboard up to height 600, click position under sheetPage, but sheet needs not up.
     * @tc.expected: The sheetHeightUp value is zero, and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1900));
    sheetPattern->AvoidSafeArea();
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.steps: step5. recover api version info.
     */
    AceApplicationInfo::GetInstance().SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.type: FUNC
 * @tc.name: Test BindSheet keyboardAvoidMode
 * @tc.desc: Test SheetPresentationPattern::AvoidSafeArea() when sheetType is Center,
 *           and SheetKeyboardAvoidMode is NONE When api version greater than 12.
 */
HWTEST_F(OverlayManagerTestNg, TestSheetAvoidSafeArea5, TestSize.Level1)
{
    /**
     * @tc.steps: step1. initalize environment and set api version to api 13.
     */
    int32_t backupApiVersion = AceApplicationInfo::GetInstance().GetApiTargetVersion();
    AceApplicationInfo::GetInstance().SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_THIRTEEN));

    /**
     * @tc.steps: step2. create sheet node and initialize sheet pattern.
     */
    auto sheetNode = FrameNode::CreateFrameNode(
        V2::SHEET_PAGE_TAG, 1, AceType::MakeRefPtr<SheetPresentationPattern>(-1, V2::BUTTON_ETS_TAG, nullptr));
    auto dragBarNode = FrameNode::CreateFrameNode(
        "SheetDragBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SheetDragBarPattern>());
    auto scroll = FrameNode::CreateFrameNode(
        V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
    auto builderContent =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    builderContent->MountToParent(scroll);
    dragBarNode->MountToParent(sheetNode);
    scroll->MountToParent(sheetNode);
    sheetNode->GetFocusHub()->currentFocus_ = true;
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    sheetPattern->sheetType_ = SheetType::SHEET_CENTER;
    auto renderContext = sheetNode->GetRenderContext();
    auto safeAreaManager = AceType::MakeRefPtr<SafeAreaManager>();
    auto geometryNode = sheetNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetFrameSize(SizeF(800, 1800));
    MockPipelineContext::GetCurrent()->safeAreaManager_ = safeAreaManager;
    MockPipelineContext::GetCurrent()->SetRootSize(800, 2000);
    auto textFieldManager = AceType::MakeRefPtr<TextFieldManagerNG>();
    textFieldManager->SetHeight(20);
    MockPipelineContext::GetCurrent()->SetTextFieldManager(textFieldManager);
    SafeAreaInsets::Inset upKeyboard { 0, 600 };
    SafeAreaInsets::Inset emptyKeyboard { 0, 0 };
    sheetPattern->pageHeight_ = 2000;
    sheetPattern->UpdateSheetType();
    sheetPattern->InitSheetObject();
    ASSERT_NE(sheetPattern->GetSheetObject(), nullptr);
    sheetPattern->GetSheetObject()->SetSheetHeight(1800);
    sheetPattern->centerHeight_ = 1800;
    sheetPattern->height_ = 1900;
    auto sheetLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    EXPECT_FALSE(sheetLayoutProperty == nullptr);
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    if (!sheetStyle.sheetKeyboardAvoidMode.has_value()) {
        sheetStyle.sheetKeyboardAvoidMode = SheetKeyboardAvoidMode::NONE;
    }
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);

    /**
     * @tc.steps: step3. set sheet keyboardAvoidMode to NONE.
     */
    sheetPattern->keyboardAvoidMode_ =
        sheetStyle.sheetKeyboardAvoidMode.value_or(SheetKeyboardAvoidMode::TRANSLATE_AND_SCROLL);

    /**
     * @tc.steps: step4. keyboard up, and sheet will not to go up beacure keyboardAvoidMode is NONE.
     * @tc.cases: case1. keyboard up to height 600, click position above keyboard, but sheet needs not up.
     * @tc.expected: The sheetHeightUp value is zero, and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1000));
    sheetPattern->AvoidSafeArea();
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case1. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. keyboard up to height 600, click position under keyboard, but sheet needs not up.
     * @tc.expected: The sheetHeightUp value is zero, and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1500));
    sheetPattern->AvoidSafeArea();
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. keyboard up to height 600, click position under sheetPage, but sheet needs not up.
     * @tc.expected: The sheetHeightUp value is zero, and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1900));
    sheetPattern->AvoidSafeArea();
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, 0.f));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.steps: step5. recover api version info.
     */
    AceApplicationInfo::GetInstance().SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.type: FUNC
 * @tc.name: Test BindSheet keyboardAvoidMode
 * @tc.desc: Test SheetPresentationPattern::AvoidSafeArea() when sheetType is Bottom,
 *           and SheetKeyboardAvoidMode is TRANSLATE_AND_RESIZE When api version greater than 12.
 */
HWTEST_F(OverlayManagerTestNg, TestSheetAvoidSafeArea6, TestSize.Level1)
{
    /**
     * @tc.steps: step1. initalize environment and set api version to api 13.
     */
    int32_t backupApiVersion = AceApplicationInfo::GetInstance().GetApiTargetVersion();
    AceApplicationInfo::GetInstance().SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_THIRTEEN));

    /**
     * @tc.steps: step2. create sheet node and initialize sheet pattern.
     */
    auto sheetNode = FrameNode::CreateFrameNode(
        V2::SHEET_PAGE_TAG, 1, AceType::MakeRefPtr<SheetPresentationPattern>(-1, V2::BUTTON_ETS_TAG, nullptr));
    auto dragBarNode = FrameNode::CreateFrameNode(
        "SheetDragBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SheetDragBarPattern>());
    auto scroll = FrameNode::CreateFrameNode(
        V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
    auto builderContent =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    builderContent->MountToParent(scroll);
    dragBarNode->MountToParent(sheetNode);
    scroll->MountToParent(sheetNode);
    sheetNode->GetFocusHub()->currentFocus_ = true;
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    sheetPattern->SetScrollNode(WeakPtr<FrameNode>(scroll));
    sheetPattern->sheetType_ = SheetType::SHEET_BOTTOM;
    auto renderContext = sheetNode->GetRenderContext();
    auto safeAreaManager = AceType::MakeRefPtr<SafeAreaManager>();
    auto geometryNode = sheetNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetFrameSize(SizeF(800, 1800));
    MockPipelineContext::GetCurrent()->safeAreaManager_ = safeAreaManager;
    MockPipelineContext::GetCurrent()->SetRootSize(800, 2000);
    auto textFieldManager = AceType::MakeRefPtr<TextFieldManagerNG>();
    textFieldManager->SetHeight(20);
    MockPipelineContext::GetCurrent()->SetTextFieldManager(textFieldManager);
    SafeAreaInsets::Inset upKeyboard { 0, 600 };
    SafeAreaInsets::Inset emptyKeyboard { 0, 0 };
    sheetPattern->pageHeight_ = 2000;
    sheetPattern->UpdateSheetType();
    sheetPattern->InitSheetObject();
    ASSERT_NE(sheetPattern->GetSheetObject(), nullptr);
    sheetPattern->GetSheetObject()->SetSheetHeight(1800);
    sheetPattern->height_ = 1800;
    auto sheetLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    EXPECT_FALSE(sheetLayoutProperty == nullptr);
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    if (!sheetStyle.sheetKeyboardAvoidMode.has_value()) {
        sheetStyle.sheetKeyboardAvoidMode = SheetKeyboardAvoidMode::TRANSLATE_AND_RESIZE;
    }
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    ASSERT_NE(pattern->selectController_, nullptr);

    /**
     * @tc.steps: step3. set sheet keyboardAvoidMode to TRANSLATE_AND_RESIZE.
     */
    sheetPattern->keyboardAvoidMode_ =
        sheetStyle.sheetKeyboardAvoidMode.value_or(SheetKeyboardAvoidMode::TRANSLATE_AND_SCROLL);

    /**
     * @tc.steps: step4. keyboard up, and sheet will go up to correct position.
     * @tc.cases: case1. keyboard up to height 600, click position above keyboard, sheet needs not up.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_.
     *              and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1000));
    sheetPattern->AvoidSafeArea();
    auto expectedKeyboardHeight = 600.f;
    auto expectedSheetHeightUp = 0.f;
    auto expectedTranslateValue = 2000.0f - sheetPattern->height_;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case1. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_,
     *              and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. keyboard up to height 600, click position under keyboard, sheet will lift up.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to expectedSheetHeightUp,
     *              and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    pattern->selectController_->caretInfo_.rect.SetTop(1500.0f);
    textFieldManager->onFocusTextField_ = AceType::DynamicCast<Pattern>(pattern);
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 600.f;
    expectedSheetHeightUp = expectedKeyboardHeight - (MockPipelineContext::GetCurrent()->GetRootHeight() -
        textFieldManager->GetFocusedNodeCaretRect().Top() - textFieldManager->GetHeight());
    expectedTranslateValue = sheetPattern->pageHeight_ - sheetPattern->height_ - expectedSheetHeightUp;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_,
     *              and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    expectedSheetHeightUp = 0.f;
    expectedTranslateValue = 2000.0f - sheetPattern->height_;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. keyboard up to height 600, click position under sheetPage, sheet will lift up and resize.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to expectedSheetHeightUp.
     *              and isScrolling state is true.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    pattern->selectController_->caretInfo_.rect.SetTop(1900.0f);
    textFieldManager->onFocusTextField_ = AceType::DynamicCast<Pattern>(pattern);
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 600.f;
    expectedSheetHeightUp = sheetPattern->pageHeight_ -
        (SHEET_BLANK_MINI_HEIGHT.ConvertToPx() + sheetPattern->sheetTopSafeArea_) - sheetPattern->height_;
    expectedTranslateValue = SHEET_BLANK_MINI_HEIGHT.ConvertToPx() + sheetPattern->sheetTopSafeArea_;
    auto bottomHeight = sheetPattern->sheetType_ == SheetType::SHEET_CENTER ?
        sheetPattern->height_ - sheetPattern->centerHeight_ : 0.0f;
    auto maxSheetHeighUp = MockPipelineContext::GetCurrent()->GetRootHeight() -
        SHEET_BLANK_MINI_HEIGHT.ConvertToPx() - sheetPattern->sheetTopSafeArea_ - sheetPattern->height_;
    auto expectedResizeDecreasedHeight = expectedKeyboardHeight - bottomHeight - maxSheetHeighUp;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_TRUE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_,
     *              and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    expectedSheetHeightUp = 0.f;
    expectedTranslateValue = 2000.0f - sheetPattern->height_;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.steps: step5. recover api version info.
     */
    AceApplicationInfo::GetInstance().SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.type: FUNC
 * @tc.name: Test BindSheet keyboardAvoidMode
 * @tc.desc: Test SheetPresentationPattern::AvoidSafeArea() when sheetType is Center,
 *           and SheetKeyboardAvoidMode is TRANSLATE_AND_RESIZE When api version greater than 12.
 */
HWTEST_F(OverlayManagerTestNg, TestSheetAvoidSafeArea7, TestSize.Level1)
{
    /**
     * @tc.steps: step1. initalize environment and set api version to api 13.
     */
    int32_t backupApiVersion = AceApplicationInfo::GetInstance().GetApiTargetVersion();
    AceApplicationInfo::GetInstance().SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_THIRTEEN));

    /**
     * @tc.steps: step2. create sheet node and initialize sheet pattern.
     */
    auto sheetNode = FrameNode::CreateFrameNode(
        V2::SHEET_PAGE_TAG, 1, AceType::MakeRefPtr<SheetPresentationPattern>(-1, V2::BUTTON_ETS_TAG, nullptr));
    auto dragBarNode = FrameNode::CreateFrameNode(
        "SheetDragBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SheetDragBarPattern>());
    auto scroll = FrameNode::CreateFrameNode(
        V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
    auto builderContent =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    builderContent->MountToParent(scroll);
    dragBarNode->MountToParent(sheetNode);
    scroll->MountToParent(sheetNode);
    sheetNode->GetFocusHub()->currentFocus_ = true;
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    sheetPattern->SetScrollNode(WeakPtr<FrameNode>(scroll));
    sheetPattern->sheetType_ = SheetType::SHEET_CENTER;
    auto renderContext = sheetNode->GetRenderContext();
    auto safeAreaManager = AceType::MakeRefPtr<SafeAreaManager>();
    auto geometryNode = sheetNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetFrameSize(SizeF(800, 1800));
    MockPipelineContext::GetCurrent()->safeAreaManager_ = safeAreaManager;
    MockPipelineContext::GetCurrent()->SetRootSize(800, 2000);
    auto textFieldManager = AceType::MakeRefPtr<TextFieldManagerNG>();
    textFieldManager->SetHeight(20);
    MockPipelineContext::GetCurrent()->SetTextFieldManager(textFieldManager);
    SafeAreaInsets::Inset upKeyboard { 0, 600 };
    SafeAreaInsets::Inset emptyKeyboard { 0, 0 };
    sheetPattern->pageHeight_ = 2000;
    sheetPattern->UpdateSheetType();
    sheetPattern->InitSheetObject();
    ASSERT_NE(sheetPattern->GetSheetObject(), nullptr);
    sheetPattern->GetSheetObject()->SetSheetHeight(1800);
    sheetPattern->centerHeight_ = 1800;
    sheetPattern->height_ = 1900;
    auto sheetLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    EXPECT_FALSE(sheetLayoutProperty == nullptr);
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    if (!sheetStyle.sheetKeyboardAvoidMode.has_value()) {
        sheetStyle.sheetKeyboardAvoidMode = SheetKeyboardAvoidMode::TRANSLATE_AND_RESIZE;
    }
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    ASSERT_NE(pattern->selectController_, nullptr);

    /**
     * @tc.steps: step3. set sheet keyboardAvoidMode to TRANSLATE_AND_RESIZE.
     */
    sheetPattern->keyboardAvoidMode_ =
        sheetStyle.sheetKeyboardAvoidMode.value_or(SheetKeyboardAvoidMode::TRANSLATE_AND_SCROLL);

    /**
     * @tc.steps: step4. keyboard up, and sheet will go up to correct position.
     * @tc.cases: case1. keyboard up to height 600, click position above keyboard, sheet needs not up.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_.
     *              and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1000));
    sheetPattern->AvoidSafeArea();
    auto expectedKeyboardHeight = 600.f;
    auto expectedSheetHeightUp = 0.f;
    auto expectedTranslateValue = 2000.0f - sheetPattern->height_;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case1. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_,
     *              and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. keyboard up to height 600, click position under keyboard, sheet will lift up.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to expectedSheetHeightUp,
     *              and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    pattern->selectController_->caretInfo_.rect.SetTop(1400.0f);
    textFieldManager->onFocusTextField_ = AceType::DynamicCast<Pattern>(pattern);
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 600.f;
    expectedSheetHeightUp = expectedKeyboardHeight - (MockPipelineContext::GetCurrent()->GetRootHeight() -
        textFieldManager->GetFocusedNodeCaretRect().Top() - textFieldManager->GetHeight());
    expectedTranslateValue = sheetPattern->pageHeight_ - sheetPattern->height_ - expectedSheetHeightUp;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_,
     *              and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    expectedSheetHeightUp = 0.f;
    expectedTranslateValue = 2000.0f - sheetPattern->height_;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, 0.f));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. keyboard up to height 600, click position under sheetPage, sheet will scroll up and resize.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to expectedSheetHeightUp.
     *              and isScrolling state is true.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    pattern->selectController_->caretInfo_.rect.SetTop(1800.0f);
    textFieldManager->onFocusTextField_ = AceType::DynamicCast<Pattern>(pattern);
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 600.f;
    expectedSheetHeightUp = sheetPattern->pageHeight_ -
        (SHEET_BLANK_MINI_HEIGHT.ConvertToPx() + sheetPattern->sheetTopSafeArea_) - sheetPattern->height_;
    expectedTranslateValue = SHEET_BLANK_MINI_HEIGHT.ConvertToPx() + sheetPattern->sheetTopSafeArea_;
    auto bottomHeight = sheetPattern->sheetType_ == SheetType::SHEET_CENTER ?
        sheetPattern->height_ - sheetPattern->centerHeight_ : 0.0f;
    auto maxSheetHeighUp = MockPipelineContext::GetCurrent()->GetRootHeight() -
        SHEET_BLANK_MINI_HEIGHT.ConvertToPx() - sheetPattern->sheetTopSafeArea_ - sheetPattern->height_;
    auto expectedResizeDecreasedHeight = expectedKeyboardHeight - bottomHeight - maxSheetHeighUp;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_TRUE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_,
     *              and isScrolling state is false.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    expectedSheetHeightUp = 0.f;
    expectedTranslateValue = 2000.0f - sheetPattern->height_;
    expectedResizeDecreasedHeight = 0.f;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.steps: step5. recover api version info.
     */
    AceApplicationInfo::GetInstance().SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.type: FUNC
 * @tc.name: Test BindSheet keyboardAvoidMode
 * @tc.desc: Test SheetPresentationPattern::AvoidSafeArea() when sheetType is Bottom,
 *           and SheetKeyboardAvoidMode is RESIZE_ONLY When api version greater than 12.
 */
HWTEST_F(OverlayManagerTestNg, TestSheetAvoidSafeArea8, TestSize.Level1)
{
    /**
     * @tc.steps: step1. initalize environment and set api version to api 13.
     */
    int32_t backupApiVersion = AceApplicationInfo::GetInstance().GetApiTargetVersion();
    AceApplicationInfo::GetInstance().SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_THIRTEEN));

    /**
     * @tc.steps: step2. create sheet node and initialize sheet pattern.
     */
    auto sheetNode = FrameNode::CreateFrameNode(
        V2::SHEET_PAGE_TAG, 1, AceType::MakeRefPtr<SheetPresentationPattern>(-1, V2::BUTTON_ETS_TAG, nullptr));
    auto dragBarNode = FrameNode::CreateFrameNode(
        "SheetDragBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SheetDragBarPattern>());
    auto scroll = FrameNode::CreateFrameNode(
        V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
    auto builderContent =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    builderContent->MountToParent(scroll);
    dragBarNode->MountToParent(sheetNode);
    scroll->MountToParent(sheetNode);
    sheetNode->GetFocusHub()->currentFocus_ = true;
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    sheetPattern->SetScrollNode(WeakPtr<FrameNode>(scroll));
    sheetPattern->sheetType_ = SheetType::SHEET_BOTTOM;
    auto renderContext = sheetNode->GetRenderContext();
    auto safeAreaManager = AceType::MakeRefPtr<SafeAreaManager>();
    auto geometryNode = sheetNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetFrameSize(SizeF(800, 1800));
    MockPipelineContext::GetCurrent()->safeAreaManager_ = safeAreaManager;
    MockPipelineContext::GetCurrent()->SetRootSize(800, 2000);
    auto textFieldManager = AceType::MakeRefPtr<TextFieldManagerNG>();
    textFieldManager->SetHeight(20);
    MockPipelineContext::GetCurrent()->SetTextFieldManager(textFieldManager);
    SafeAreaInsets::Inset upKeyboard { 0, 600 };
    SafeAreaInsets::Inset emptyKeyboard { 0, 0 };
    sheetPattern->pageHeight_ = 2000;
    sheetPattern->UpdateSheetType();
    sheetPattern->InitSheetObject();
    ASSERT_NE(sheetPattern->GetSheetObject(), nullptr);
    sheetPattern->GetSheetObject()->SetSheetHeight(1800);
    sheetPattern->height_ = 1800;
    auto sheetLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    EXPECT_FALSE(sheetLayoutProperty == nullptr);
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    if (!sheetStyle.sheetKeyboardAvoidMode.has_value()) {
        sheetStyle.sheetKeyboardAvoidMode = SheetKeyboardAvoidMode::RESIZE_ONLY;
    }
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);

    /**
     * @tc.steps: step3. set sheet keyboardAvoidMode to RESIZE_ONLY.
     */
    sheetPattern->keyboardAvoidMode_ =
        sheetStyle.sheetKeyboardAvoidMode.value_or(SheetKeyboardAvoidMode::TRANSLATE_AND_SCROLL);

    /**
     * @tc.steps: step4. keyboard up, and sheet will go up to correct position.
     * @tc.cases: case1. keyboard up to height 600, click position above keyboard, sheet will resize scroll height.
     * @tc.expected: The sheetHeightUp value is zero,
     *              and isScrolling state is true, sheet decreaseHeight is equal to keyboardHeight.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1000));
    sheetPattern->AvoidSafeArea();
    auto expectedKeyboardHeight = 600.f;
    auto expectedSheetHeightUp = 0.f;
    auto bottomHeight = sheetPattern->sheetType_ == SheetType::SHEET_CENTER ?
        sheetPattern->height_ - sheetPattern->centerHeight_ : 0.0f;
    auto expectedResizeDecreasedHeight = expectedKeyboardHeight - bottomHeight;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_TRUE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case1. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero,
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    expectedResizeDecreasedHeight = 0.f;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. keyboard up to height 600, click position under keyboard, sheet will resize scroll height.
     * @tc.expected: The sheetHeightUp value is zero,
     *              and isScrolling state is true, sheet decreaseHeight is equal to keyboardHeight.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1500));
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 600.f;
    expectedResizeDecreasedHeight = expectedKeyboardHeight - bottomHeight;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_TRUE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero,
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    expectedResizeDecreasedHeight = 0.f;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. keyboard up to height 600, click position under sheetPage, sheet will resize scroll height.
     * @tc.expected: The sheetHeightUp value is zero,
     *              and isScrolling state is true, sheet decreaseHeight is equal to keyboardHeight.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1900));
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 600.f;
    expectedResizeDecreasedHeight = expectedKeyboardHeight - bottomHeight;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_TRUE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero,
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    expectedResizeDecreasedHeight = 0.f;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.steps: step5. recover api version info.
     */
    AceApplicationInfo::GetInstance().SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.type: FUNC
 * @tc.name: Test BindSheet keyboardAvoidMode
 * @tc.desc: Test SheetPresentationPattern::AvoidSafeArea() when sheetType is Center,
 *           and SheetKeyboardAvoidMode is RESIZE_ONLY When api version greater than 12.
 */
HWTEST_F(OverlayManagerTestNg, TestSheetAvoidSafeArea9, TestSize.Level1)
{
    /**
     * @tc.steps: step1. initalize environment and set api version to api 13.
     */
    int32_t backupApiVersion = AceApplicationInfo::GetInstance().GetApiTargetVersion();
    AceApplicationInfo::GetInstance().SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_THIRTEEN));

    /**
     * @tc.steps: step2. create sheet node and initialize sheet pattern.
     */
    auto sheetNode = FrameNode::CreateFrameNode(
        V2::SHEET_PAGE_TAG, 1, AceType::MakeRefPtr<SheetPresentationPattern>(-1, V2::BUTTON_ETS_TAG, nullptr));
    auto dragBarNode = FrameNode::CreateFrameNode(
        "SheetDragBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SheetDragBarPattern>());
    auto scroll = FrameNode::CreateFrameNode(
        V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
    auto builderContent =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    builderContent->MountToParent(scroll);
    dragBarNode->MountToParent(sheetNode);
    scroll->MountToParent(sheetNode);
    sheetNode->GetFocusHub()->currentFocus_ = true;
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    sheetPattern->SetScrollNode(WeakPtr<FrameNode>(scroll));
    sheetPattern->sheetType_ = SheetType::SHEET_CENTER;
    auto renderContext = sheetNode->GetRenderContext();
    auto safeAreaManager = AceType::MakeRefPtr<SafeAreaManager>();
    auto geometryNode = sheetNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetFrameSize(SizeF(800, 1800));
    MockPipelineContext::GetCurrent()->safeAreaManager_ = safeAreaManager;
    MockPipelineContext::GetCurrent()->SetRootSize(800, 2000);
    auto textFieldManager = AceType::MakeRefPtr<TextFieldManagerNG>();
    textFieldManager->SetHeight(20);
    MockPipelineContext::GetCurrent()->SetTextFieldManager(textFieldManager);
    SafeAreaInsets::Inset upKeyboard { 0, 600 };
    SafeAreaInsets::Inset emptyKeyboard { 0, 0 };
    sheetPattern->pageHeight_ = 2000;
    sheetPattern->UpdateSheetType();
    sheetPattern->InitSheetObject();
    ASSERT_NE(sheetPattern->GetSheetObject(), nullptr);
    sheetPattern->GetSheetObject()->SetSheetHeight(1800);
    sheetPattern->centerHeight_ = 1800;
    sheetPattern->height_ = 1900;
    auto sheetLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    EXPECT_FALSE(sheetLayoutProperty == nullptr);
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    if (!sheetStyle.sheetKeyboardAvoidMode.has_value()) {
        sheetStyle.sheetKeyboardAvoidMode = SheetKeyboardAvoidMode::RESIZE_ONLY;
    }
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);

    /**
     * @tc.steps: step3. set sheet keyboardAvoidMode to RESIZE_ONLY.
     */
    sheetPattern->keyboardAvoidMode_ =
        sheetStyle.sheetKeyboardAvoidMode.value_or(SheetKeyboardAvoidMode::TRANSLATE_AND_SCROLL);

    /**
     * @tc.steps: step4. keyboard up, and sheet will go up to correct position.
     * @tc.cases: case1. keyboard up to height 600, click position above keyboard, sheet will resize scroll height.
     * @tc.expected: The sheetHeightUp value is zero,
     *              and isScrolling state is true, sheet decreaseHeight is equal to keyboardHeight - bottomHeight.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1000));
    sheetPattern->AvoidSafeArea();
    auto expectedKeyboardHeight = 600.f;
    auto expectedSheetHeightUp = 0.f;
    auto bottomHeight = sheetPattern->sheetType_ == SheetType::SHEET_CENTER ?
        sheetPattern->height_ - sheetPattern->centerHeight_ : 0.0f;
    auto expectedResizeDecreasedHeight = expectedKeyboardHeight - bottomHeight;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_TRUE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case1. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero,
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    expectedResizeDecreasedHeight = 0.f;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. keyboard up to height 600, click position under keyboard, sheet will resize scroll height.
     * @tc.expected: The sheetHeightUp value is zero,
     *              and isScrolling state is true, sheet decreaseHeight is equal to keyboardHeight - bottomHeight.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1400));
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 600.f;
    expectedResizeDecreasedHeight = expectedKeyboardHeight - bottomHeight;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_TRUE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero,
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    expectedResizeDecreasedHeight = 0.f;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. keyboard up to height 600, click position under sheetPage, sheet will resize scroll height.
     * @tc.expected: The sheetHeightUp value is zero,
     *              and isScrolling state is true, sheet decreaseHeight is equal to keyboardHeight - bottomHeight.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1800));
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 600.f;
    expectedResizeDecreasedHeight = expectedKeyboardHeight - bottomHeight;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_TRUE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_,
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    expectedResizeDecreasedHeight = 0.f;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.steps: step5. recover api version info.
     */
    AceApplicationInfo::GetInstance().SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.type: FUNC
 * @tc.name: Test BindSheet keyboardAvoidMode
 * @tc.desc: Test SheetPresentationPattern::AvoidSafeArea() when sheetType is Bottom,
 *           and SheetKeyboardAvoidMode is TRANSLATE_AND_SCROLL When api version greater than 12.
 */
HWTEST_F(OverlayManagerTestNg, TestSheetAvoidSafeArea10, TestSize.Level1)
{
    /**
     * @tc.steps: step1. initalize environment and set api version to api 13.
     */
    int32_t backupApiVersion = AceApplicationInfo::GetInstance().GetApiTargetVersion();
    AceApplicationInfo::GetInstance().SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_THIRTEEN));

    /**
     * @tc.steps: step2. create sheet node and initialize sheet pattern.
     */
    auto sheetNode = FrameNode::CreateFrameNode(
        V2::SHEET_PAGE_TAG, 1, AceType::MakeRefPtr<SheetPresentationPattern>(-1, V2::BUTTON_ETS_TAG, nullptr));
    auto dragBarNode = FrameNode::CreateFrameNode(
        "SheetDragBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SheetDragBarPattern>());
    auto scroll = FrameNode::CreateFrameNode(
        V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
    auto builderContent =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    builderContent->MountToParent(scroll);
    dragBarNode->MountToParent(sheetNode);
    scroll->MountToParent(sheetNode);
    sheetNode->GetFocusHub()->currentFocus_ = true;
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    sheetPattern->SetScrollNode(WeakPtr<FrameNode>(scroll));
    sheetPattern->sheetType_ = SheetType::SHEET_BOTTOM;
    auto renderContext = sheetNode->GetRenderContext();
    auto safeAreaManager = AceType::MakeRefPtr<SafeAreaManager>();
    auto geometryNode = sheetNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetFrameSize(SizeF(800, 1800));
    MockPipelineContext::GetCurrent()->safeAreaManager_ = safeAreaManager;
    MockPipelineContext::GetCurrent()->SetRootSize(800, 2000);
    auto textFieldManager = AceType::MakeRefPtr<TextFieldManagerNG>();
    textFieldManager->SetHeight(20);
    MockPipelineContext::GetCurrent()->SetTextFieldManager(textFieldManager);
    SafeAreaInsets::Inset upKeyboard { 0, 600 };
    SafeAreaInsets::Inset emptyKeyboard { 0, 0 };
    sheetPattern->pageHeight_ = 2000;
    sheetPattern->UpdateSheetType();
    sheetPattern->InitSheetObject();
    ASSERT_NE(sheetPattern->GetSheetObject(), nullptr);
    sheetPattern->GetSheetObject()->SetSheetHeight(1800);
    sheetPattern->height_ = 1800;
    auto sheetLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    EXPECT_FALSE(sheetLayoutProperty == nullptr);
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    if (!sheetStyle.sheetKeyboardAvoidMode.has_value()) {
        sheetStyle.sheetKeyboardAvoidMode = SheetKeyboardAvoidMode::TRANSLATE_AND_SCROLL;
    }
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    ASSERT_NE(pattern->selectController_, nullptr);

    /**
     * @tc.steps: step3. set sheet keyboardAvoidMode to TRANSLATE_AND_SCROLL.
     */
    sheetPattern->keyboardAvoidMode_ =
        sheetStyle.sheetKeyboardAvoidMode.value_or(SheetKeyboardAvoidMode::TRANSLATE_AND_SCROLL);

    /**
     * @tc.steps: step4. keyboard up, and sheet will go up to correct position.
     * @tc.cases: case1. keyboard up to height 600, click position above keyboard, sheet will not lift up.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_.
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1000));
    sheetPattern->AvoidSafeArea();
    auto expectedKeyboardHeight = 600.f;
    auto expectedSheetHeightUp = 0.f;
    auto expectedTranslateValue = 2000.0f - sheetPattern->height_;
    auto expectedResizeDecreasedHeight = 0.f;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case1. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_,
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. keyboard up to height 600, click position under keyboard, sheet will lift up.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to expectedSheetHeightUp,
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    pattern->selectController_->caretInfo_.rect.SetTop(1500.0f);
    textFieldManager->onFocusTextField_ = AceType::DynamicCast<Pattern>(pattern);
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 600.f;
    expectedSheetHeightUp = expectedKeyboardHeight - (MockPipelineContext::GetCurrent()->GetRootHeight() -
        textFieldManager->GetFocusedNodeCaretRect().Top() - textFieldManager->GetHeight());
    expectedTranslateValue = sheetPattern->pageHeight_ - sheetPattern->height_ - expectedSheetHeightUp;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_,
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    expectedSheetHeightUp = 0.f;
    expectedTranslateValue = 2000.0f - sheetPattern->height_;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. keyboard up to height 600, click position under sheetPage, sheet will lift up and scroll.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to expectedSheetHeightUp.
     *              and isScrolling state is true, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    pattern->selectController_->caretInfo_.rect.SetTop(1900.0f);
    textFieldManager->onFocusTextField_ = AceType::DynamicCast<Pattern>(pattern);
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 600.f;
    expectedSheetHeightUp = sheetPattern->pageHeight_ -
        (SHEET_BLANK_MINI_HEIGHT.ConvertToPx() + sheetPattern->sheetTopSafeArea_) - sheetPattern->height_;
    expectedTranslateValue = SHEET_BLANK_MINI_HEIGHT.ConvertToPx() + sheetPattern->sheetTopSafeArea_;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_TRUE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_,
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    expectedSheetHeightUp = 0.f;
    expectedTranslateValue = 2000.0f - sheetPattern->height_;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.steps: step5. recover api version info.
     */
    AceApplicationInfo::GetInstance().SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.type: FUNC
 * @tc.name: Test BindSheet keyboardAvoidMode
 * @tc.desc: Test SheetPresentationPattern::AvoidSafeArea() when sheetType is Center,
 *           and SheetKeyboardAvoidMode is TRANSLATE_AND_SCROLL When api version greater than 12.
 */
HWTEST_F(OverlayManagerTestNg, TestSheetAvoidSafeArea11, TestSize.Level1)
{
    /**
     * @tc.steps: step1. initalize environment and set api version to api 13.
     */
    int32_t backupApiVersion = AceApplicationInfo::GetInstance().GetApiTargetVersion();
    AceApplicationInfo::GetInstance().SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_THIRTEEN));

    /**
     * @tc.steps: step2. create sheet node and initialize sheet pattern.
     */
    auto sheetNode = FrameNode::CreateFrameNode(
        V2::SHEET_PAGE_TAG, 1, AceType::MakeRefPtr<SheetPresentationPattern>(-1, V2::BUTTON_ETS_TAG, nullptr));
    auto dragBarNode = FrameNode::CreateFrameNode(
        "SheetDragBar", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SheetDragBarPattern>());
    auto scroll = FrameNode::CreateFrameNode(
        V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
    auto builderContent =
        FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 0, AceType::MakeRefPtr<LinearLayoutPattern>(false));
    builderContent->MountToParent(scroll);
    dragBarNode->MountToParent(sheetNode);
    scroll->MountToParent(sheetNode);
    sheetNode->GetFocusHub()->currentFocus_ = true;
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    sheetPattern->SetScrollNode(WeakPtr<FrameNode>(scroll));
    sheetPattern->sheetType_ = SheetType::SHEET_CENTER;
    auto renderContext = sheetNode->GetRenderContext();
    auto safeAreaManager = AceType::MakeRefPtr<SafeAreaManager>();
    auto geometryNode = sheetNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetFrameSize(SizeF(800, 1800));
    MockPipelineContext::GetCurrent()->safeAreaManager_ = safeAreaManager;
    MockPipelineContext::GetCurrent()->SetRootSize(800, 2000);
    auto textFieldManager = AceType::MakeRefPtr<TextFieldManagerNG>();
    textFieldManager->SetHeight(20);
    MockPipelineContext::GetCurrent()->SetTextFieldManager(textFieldManager);
    SafeAreaInsets::Inset upKeyboard { 0, 600 };
    SafeAreaInsets::Inset emptyKeyboard { 0, 0 };
    sheetPattern->pageHeight_ = 2000;
    sheetPattern->UpdateSheetType();
    sheetPattern->InitSheetObject();
    ASSERT_NE(sheetPattern->GetSheetObject(), nullptr);
    sheetPattern->GetSheetObject()->SetSheetHeight(1800);
    sheetPattern->centerHeight_ = 1800;
    sheetPattern->height_ = 1900;
    auto sheetLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    EXPECT_FALSE(sheetLayoutProperty == nullptr);
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    if (!sheetStyle.sheetKeyboardAvoidMode.has_value()) {
        sheetStyle.sheetKeyboardAvoidMode = SheetKeyboardAvoidMode::TRANSLATE_AND_SCROLL;
    }
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    ASSERT_NE(pattern->selectController_, nullptr);

    /**
     * @tc.steps: step3. set sheet keyboardAvoidMode to TRANSLATE_AND_SCROLL.
     */
    sheetPattern->keyboardAvoidMode_ =
        sheetStyle.sheetKeyboardAvoidMode.value_or(SheetKeyboardAvoidMode::TRANSLATE_AND_SCROLL);

    /**
     * @tc.steps: step4. keyboard up, and sheet will go up to correct position.
     * @tc.cases: case1. keyboard up to height 600, click position above keyboard, sheet will not lift up.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_.
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    textFieldManager->SetClickPosition(Offset(500, 1000));
    sheetPattern->AvoidSafeArea();
    auto expectedKeyboardHeight = 600.f;
    auto expectedSheetHeightUp = 0.f;
    auto expectedTranslateValue = 2000.0f - sheetPattern->height_;
    auto expectedResizeDecreasedHeight = 0.f;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case1. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_,
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. keyboard up to height 600, click position under keyboard, sheet will lift up.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to expectedSheetHeightUp,
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    pattern->selectController_->caretInfo_.rect.SetTop(1400.0f);
    textFieldManager->onFocusTextField_ = AceType::DynamicCast<Pattern>(pattern);
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 600.f;
    expectedSheetHeightUp = expectedKeyboardHeight - (MockPipelineContext::GetCurrent()->GetRootHeight() -
        textFieldManager->GetFocusedNodeCaretRect().Top() - textFieldManager->GetHeight());
    expectedTranslateValue = sheetPattern->pageHeight_ - sheetPattern->height_ - expectedSheetHeightUp;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case2. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_,
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    expectedSheetHeightUp = 0.f;
    expectedTranslateValue = 2000.0f - sheetPattern->height_;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. keyboard up to height 600, click position under sheetPage, sheet will lift and scroll.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to expectedSheetHeightUp,
     *              and isScrolling state is true, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = upKeyboard;
    pattern->selectController_->caretInfo_.rect.SetTop(1800.0f);
    textFieldManager->onFocusTextField_ = AceType::DynamicCast<Pattern>(pattern);
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 600.f;
    expectedSheetHeightUp = sheetPattern->pageHeight_ -
        (SHEET_BLANK_MINI_HEIGHT.ConvertToPx() + sheetPattern->sheetTopSafeArea_) - sheetPattern->height_;
    expectedTranslateValue = SHEET_BLANK_MINI_HEIGHT.ConvertToPx() + sheetPattern->sheetTopSafeArea_;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_TRUE(sheetPattern->isScrolling_);

    /**
     * @tc.cases: case3. recover keyboardHeight and sheet avoid status by clear on focusTextField.
     * @tc.expected: The sheetHeightUp value is zero, and transformTranslate.y value is equal to rootHeight - height_,
     *              and isScrolling state is false, sheet decreaseHeight value is zero.
     */
    safeAreaManager->keyboardInset_ = emptyKeyboard;
    textFieldManager->ClearOnFocusTextField();
    sheetPattern->AvoidSafeArea();
    expectedKeyboardHeight = 0.f;
    expectedSheetHeightUp = 0.f;
    expectedTranslateValue = 2000.0f - sheetPattern->height_;
    EXPECT_TRUE(NearEqual(sheetPattern->keyboardHeight_, expectedKeyboardHeight));
    EXPECT_TRUE(NearEqual(sheetPattern->sheetHeightUp_, expectedSheetHeightUp));
    EXPECT_TRUE(NearEqual(renderContext->GetTransformTranslate()->y.ConvertToPx(), expectedTranslateValue));
    EXPECT_TRUE(NearEqual(sheetPattern->resizeDecreasedHeight_, expectedResizeDecreasedHeight));
    EXPECT_FALSE(sheetPattern->isScrolling_);

    /**
     * @tc.steps: step5. recover api version info.
     */
    AceApplicationInfo::GetInstance().SetApiTargetVersion(backupApiVersion);
}

/**
 * @tc.name: TestOnBindSheet
 * @tc.desc: Test Sheet avoids aiBar.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, TestSheetAvoidaiBar, TestSize.Level1)
{
    MockPipelineContext::GetCurrent()->SetMinPlatformVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    auto operationColumn = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto callback = [](const std::string&) {};
    NG::SheetStyle style;
    auto sheetNode = SheetView::CreateSheetPage(0, "", operationColumn, operationColumn, std::move(callback), style);
    ASSERT_NE(sheetNode, nullptr);
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(sheetPattern, nullptr);
    auto scrollNode = sheetPattern->GetSheetScrollNode();
    ASSERT_NE(scrollNode, nullptr);
    sheetPattern->SetScrollNode(WeakPtr<FrameNode>(scrollNode));
    auto scrollPattern = scrollNode->GetPattern<ScrollPattern>();
    ASSERT_NE(scrollPattern, nullptr);
    auto scrollLayoutProperty = scrollNode->GetLayoutProperty<ScrollLayoutProperty>();
    ASSERT_NE(scrollLayoutProperty, nullptr);
    sheetPattern->AvoidAiBar();
    EXPECT_EQ(scrollLayoutProperty->GetContentEndOffsetValue(.0f), .0f);
    scrollPattern->scrollableDistance_ = 10.0f;
    sheetPattern->AvoidAiBar();
    EXPECT_EQ(scrollLayoutProperty->GetContentEndOffsetValue(.0f),
        PipelineContext::GetCurrentContext()->GetSafeArea().bottom_.Length());
}

/**
 * @tc.name: SheetPresentationPattern2
 * @tc.desc: Test SheetPresentationPattern::CheckSheetHeightChange().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern2, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

    /**
     * @tc.steps: step3. test CheckSheetHeightChange().
     */
    topSheetPattern->pageHeight_ = 1000;
    topSheetPattern->sheetMaxHeight_ = 1000;
    topSheetPattern->isFirstInit_ = true;
    topSheetPattern->InitialLayoutProps();
    EXPECT_FALSE(topSheetPattern->isFirstInit_);
    topSheetPattern->windowChanged_ = true;
    topSheetPattern->CheckSheetHeightChange();
    EXPECT_FALSE(topSheetPattern->windowChanged_);
}

/**
 * @tc.name: SheetPresentationPattern3
 * @tc.desc: Test SheetPresentationPattern::InitSheetDetents().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern3, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

    /**
     * @tc.steps: step3. test InitSheetDetents(), sheetType = BOTTOM.
     */
    topSheetPattern->pageHeight_ = 1000;
    topSheetPattern->sheetMaxHeight_ = 1000;
    topSheetPattern->sheetThemeType_ = "auto";
    auto sheetLayoutProperty = topSheetNode->GetLayoutProperty<SheetPresentationProperty>();
    EXPECT_FALSE(sheetLayoutProperty == nullptr);

    /**
     * @tc.steps: step4. test sheetMode has value.
     * @tc.expected: sheetStyle.detents = {600,992}.
     */
    SheetHeight detent;
    detent.sheetMode = SheetMode::MEDIUM;
    sheetStyle.detents.emplace_back(detent);
    detent.sheetMode = SheetMode::LARGE;
    sheetStyle.detents.emplace_back(detent);
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);
    topSheetPattern->InitSheetDetents();
    EXPECT_FALSE(NearEqual(topSheetPattern->sheetDetentHeight_.front(), 500));
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetDetentHeight_.back(), 992));

    /**
     * @tc.steps: step5. test sheetStyle.sheetHeight.height has value.
     */
    sheetStyle.detents.clear();
    SheetHeight detent1;
    Dimension detentHeight { 0.5, DimensionUnit::PERCENT };
    detent1.height = detentHeight;
    sheetStyle.detents.emplace_back(detent1);

    /**
     * @tc.steps: step6. set height > maxHeight.
     * @tc.expected: height = pageHeight_-8 = 992.
     */
    detent1.height->SetUnit(DimensionUnit::VP);
    detent1.height->SetValue(1200);
    sheetStyle.detents.emplace_back(detent1);

    /**
     * @tc.steps: step6. set height < 0.
     * @tc.expected: height = pageHeight_-8 = 992.
     */
    detent1.height->SetUnit(DimensionUnit::VP);
    detent1.height->SetValue(-10);
    sheetStyle.detents.emplace_back(detent1);

    /**
     * @tc.steps: step7. InitSheetDetents(), sheetStyle.detents push{500,992,992}.
     * @tc.expected: sheetStyle.detents = {500,992}.
     */
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);
    topSheetPattern->InitSheetDetents();
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetDetentHeight_.size(), 2));
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetDetentHeight_.front(), 500));
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetDetentHeight_.back(), 992));

    /**
     * @tc.steps: step8. test InitSheetDetents(), sheetType = CENTER.
     * @tc.expected: height = (centerHeight_+pageHeight_)/2 = 750.
     */
    sheetStyle.detents.clear();
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);
    topSheetPattern->sheetThemeType_ = "popup";
    topSheetPattern->centerHeight_ = 500;
    topSheetPattern->InitSheetDetents();
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetDetentHeight_.front(), 0));
}

/**
 * @tc.name: OnBindSheet007
 * @tc.desc: Test OverlayManager::DismissSheet().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, OnBindSheet007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

    /**
     * @tc.steps: step3. test DismissSheet().
     * @tc.expected: callback is false.
     */
    overlayManager->DismissSheet();
    EXPECT_FALSE(topSheetPattern->callback_);
}

/**
 * @tc.name: OnBindSheet008
 * @tc.desc: Test OverlayManager::OnBindSheet change sheetStyle width.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, OnBindSheet008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto sheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(sheetNode, nullptr);
    auto sheetLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    EXPECT_FALSE(sheetLayoutProperty == nullptr);
    auto style = sheetLayoutProperty->GetSheetStyle();
    EXPECT_EQ(style->width.has_value(), false);

    /**
     * @tc.steps: step3. Change the sheetStyle width.
     * @tc.expected: the sheetStyle width is updated successfully
     */
    Dimension width{ 300, DimensionUnit::VP };
    sheetStyle.width = width;
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, targetNode);
    sheetNode = overlayManager->modalStack_.top().Upgrade();
    sheetLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    style = sheetLayoutProperty->GetSheetStyle();
    EXPECT_EQ(style->width.value(), width);

    Dimension widthNew{ 400, DimensionUnit::VP };
    sheetStyle.width = widthNew;
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, targetNode);
    sheetNode = overlayManager->modalStack_.top().Upgrade();
    sheetLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    style = sheetLayoutProperty->GetSheetStyle();
    EXPECT_EQ(style->width.value(), widthNew);
}

/**
 * @tc.name: OnBindSheet009
 * @tc.desc: Test OverlayManager::OnBindSheet change sheetStyle border and shadow.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, OnBindSheet009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();
    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto sheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(sheetNode, nullptr);
    auto renderContext = sheetNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    EXPECT_EQ(renderContext->GetBorderWidth().has_value(), false);
    EXPECT_EQ(renderContext->GetBorderColor().has_value(), false);
    EXPECT_EQ(renderContext->GetBorderStyle().has_value(), false);
    EXPECT_EQ(renderContext->GetBackShadow().has_value(), false);
    /**
     * @tc.steps: step3. Change the sheetStyle border and shadow.
     * @tc.expected: the sheetStyle is updated successfully
     */
    Shadow shadow = ShadowConfig::DefaultShadowL;
    sheetStyle.borderWidth = BORDER_WIDTH_TEST;
    sheetStyle.borderColor = BORDER_COLOR_TEST;
    sheetStyle.borderStyle = BORDER_STYLE_TEST;
    sheetStyle.shadow = shadow;
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    sheetNode = overlayManager->modalStack_.top().Upgrade();
    renderContext = sheetNode->GetRenderContext();
    EXPECT_EQ(renderContext->GetBorderWidth().value(), BORDER_WIDTH_TEST);
    EXPECT_EQ(renderContext->GetBorderColor().value(), BORDER_COLOR_TEST);
    EXPECT_EQ(renderContext->GetBorderStyle().value(), BORDER_STYLE_TEST);
    EXPECT_EQ(renderContext->GetBackShadow().value(), shadow);
    Shadow shadowNew = ShadowConfig::NoneShadow;
    sheetStyle.borderWidth = NEW_BORDER_WIDTH_TEST;
    sheetStyle.borderColor = NEW_BORDER_COLOR_TEST;
    sheetStyle.borderStyle = NEW_BORDER_STYLE_TEST;
    sheetStyle.shadow = shadowNew;
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    sheetNode = overlayManager->modalStack_.top().Upgrade();
    renderContext = sheetNode->GetRenderContext();
    EXPECT_EQ(renderContext->GetBorderWidth().value(), NEW_BORDER_WIDTH_TEST);
    EXPECT_EQ(renderContext->GetBorderColor().value(), NEW_BORDER_COLOR_TEST);
    EXPECT_EQ(renderContext->GetBorderStyle().value(), NEW_BORDER_STYLE_TEST);
    EXPECT_EQ(renderContext->GetBackShadow().value(), shadowNew);
}

/**
 * @tc.name: SheetPresentationPattern5
 * @tc.desc: Test SheetPresentationPattern::BubbleStyleSheetTransition().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern5, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

    /**
     * @tc.steps: step3. test SheetPresentationPattern::BubbleStyleSheetTransition().
     * @tc.expected: callback is false.
     */
    topSheetPattern->BubbleStyleSheetTransition(false);
    EXPECT_FALSE(topSheetPattern->callback_);
}

/**
 * @tc.name: SheetPresentationPattern6
 * @tc.desc: Test SheetPresentationPattern::ClipSheetNode().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern6, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

    /**
     * @tc.steps: step3. test clip sheet node, set sheetSize.
     * @tc.expected: clipPath is as same as set.
     */
    auto sheetSize = SizeF(1000, 720);
    auto pipeline = PipelineContext::GetCurrentContext();
    auto sheetTheme = pipeline->GetTheme<SheetTheme>();
    auto sheetRadius = sheetTheme->GetSheetRadius();
    std::string substring = "720.000000 Z";

    /**
     * @tc.steps: step4. test clipPath.
     */
    auto popupPath = topSheetPattern->GetPopupStyleSheetClipPath(sheetSize, BorderRadiusProperty(sheetRadius));
    EXPECT_EQ(popupPath.length(), 406);
    EXPECT_EQ(popupPath.substr(394, 12), substring);
    auto centerPath = topSheetPattern->GetCenterStyleSheetClipPath(sheetSize, sheetRadius);
    EXPECT_EQ(centerPath.length(), 297);
    EXPECT_EQ(centerPath.substr(285, 12), substring);
    auto bottomPath = topSheetPattern->GetBottomStyleSheetClipPath(sheetSize, sheetRadius);
    EXPECT_EQ(bottomPath.length(), 190);
    EXPECT_EQ(bottomPath.substr(178, 12), substring);
}

/**
 * @tc.name: SheetPresentationPattern7
 * @tc.desc: Test SheetPresentationPattern::UpdateInteractive().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern7, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    AceApplicationInfo::GetInstance().SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_ELEVEN));
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(true, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();

    /**
     * @tc.steps: step3. test without setting enableOutsideInteractive.
     * @tc.expected: The backplanes are not interactive by default, maskNode should be visible.
     */
    EXPECT_FALSE(sheetStyle.interactive);
    auto maskNode = overlayManager->GetSheetMask(topSheetNode);
    ASSERT_NE(maskNode, nullptr);
    auto maskRenderContext = maskNode->GetRenderContext();
    ASSERT_NE(maskRenderContext, nullptr);
    EXPECT_NE(maskRenderContext->GetBackgroundColor(), Color::TRANSPARENT);

    /**
     * @tc.steps: step4. test set enableOutsideInteractive true.
     * @tc.expected: maskNode is invisible, the backplane can be interactive.
     */
    auto sheetLayoutProperty = topSheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(sheetLayoutProperty, nullptr);
    sheetStyle.interactive = true;
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);
    topSheetPattern->UpdateMaskBackgroundColor();
    overlayManager->PlaySheetMaskTransition(maskNode, topSheetNode, true);
    EXPECT_EQ(maskRenderContext->GetBackgroundColor(), Color::TRANSPARENT);

    /**
     * @tc.steps: step5. test set enableOutsideInteractive false.
     * @tc.expected: maskNode is visible, the backplane can not be interactive.
     */
    sheetStyle.interactive = false;
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);
    topSheetPattern->UpdateMaskBackgroundColor();
    overlayManager->PlaySheetMaskTransition(maskNode, topSheetNode, true);
    EXPECT_NE(maskRenderContext->GetBackgroundColor(), Color::TRANSPARENT);
}

/**
 * @tc.name: SheetPresentationPattern8
 * @tc.desc: Test SheetPresentationPattern::SheetInteractiveDismiss().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern8, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();

    /**
     * @tc.steps: step3. set shouldDismissFunc, sheetDetents.
     */
    topSheetPattern->sheetDetentHeight_.emplace_back(100);
    bool isDismiss = false;
    auto shouldDismissFunc = [&isDismiss]() -> void { isDismiss = true; };

    /**
     * @tc.steps: step4. Trigger a shutdown event, test when the velocity is illegal.
     * @tc.expected: shutdown faild, shouldDismissFunc is not called.
     */
    topSheetPattern->UpdateShouldDismiss(shouldDismissFunc);
    topSheetPattern->HandleDragEnd(-2000);
    EXPECT_FALSE(isDismiss);

    /**
     * @tc.steps: step5. Trigger a shutdown event.
     * @tc.expected: shouldDismissFunc is called, isDismiss = true.
     */
    topSheetPattern->HandleDragEnd(100);
    EXPECT_TRUE(isDismiss);

    /**
     * @tc.steps: step6. Trigger a shutdown event, test when the velocity reaches the threshold.
     * @tc.expected: shouldDismissFunc is called, isDismiss = true.
     */
    isDismiss = false;
    topSheetPattern->HandleDragEnd(2000);
    EXPECT_TRUE(isDismiss);
}

/**
 * @tc.name: SheetPresentationPattern9
 * @tc.desc: Test SheetPresentationPattern::SheetTransition().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern9, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    auto heightVal = 500.0f;
    auto onHeightDidChange = [&heightVal](float height) { heightVal = height; };
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, onHeightDidChange, nullptr, nullptr, nullptr,
        nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();

    /**
     * @tc.steps: step3. set params of sheetTransition, enter ModifyFireSheetTransition.
     */
    topSheetPattern->SheetTransition(false);
    topSheetPattern->sheetType_ = SheetType::SHEET_BOTTOM;
    topSheetPattern->SheetTransition(true);

    /**
     * @tc.steps: step4. end of animation callback.
     * @tc.expected: sheetTransition is called, isAnimationProcess_ = false.
     */
    topSheetPattern->isAnimationBreak_ = false;
    topSheetPattern->SetStartProp(1.0);
    topSheetPattern->ModifyFireSheetTransition();
    EXPECT_FALSE(topSheetPattern->isAnimationProcess_);

    /**
     * @tc.steps: step5. end of animation callback.
     * @tc.expected: sheetTransition is called, isAnimationBreak_ = false.
     */
    topSheetPattern->isAnimationBreak_ = true;
    topSheetPattern->SetStartProp(1.0);
    topSheetPattern->ModifyFireSheetTransition();
    EXPECT_FALSE(topSheetPattern->isAnimationBreak_);

    /**
     * @tc.steps: step6. create property callback.
     * @tc.expected: property_ is not nullptr.
     */
    topSheetPattern->property_ = nullptr;
    topSheetPattern->CreatePropertyCallback();
    EXPECT_NE(topSheetPattern->property_, nullptr);
}

/**
 * @tc.name: SheetPresentationPattern10
 * @tc.desc: Test SheetPresentationPattern::FireOnHeightDidChange().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern10, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    auto heightVal = 500.0f;
    auto onHeightDidChange = [&heightVal](float height) { heightVal = height; };
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, onHeightDidChange, nullptr, nullptr, nullptr,
        nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    
    /**
     * @tc.steps: step3. create property callback.
     * @tc.expected: topSheetNode->GetGeometryNode()->GetFrameSize().Height() 500.
     */
    topSheetPattern->sheetType_ = SheetType::SHEET_CENTER;
    topSheetNode->GetGeometryNode()->SetFrameSize(SizeF({ 500, 500 }));
    topSheetPattern->FireOnHeightDidChange();
    EXPECT_EQ(topSheetNode->GetGeometryNode()->GetFrameSize().Height(), 500);

    /**
     * @tc.steps: step4. create property callback.
     * @tc.expected: height_ equal 500.
     */
    topSheetPattern->sheetType_ = SheetType::SHEET_BOTTOM;
    topSheetPattern->SetCurrentHeight(500);
    topSheetPattern->currentOffset_ = 0;
    topSheetPattern->FireOnHeightDidChange();
    EXPECT_EQ(topSheetPattern->height_, 500);
}

/**
 * @tc.name: SheetPresentationPattern11
 * @tc.desc: Test SheetPresentationPattern::FireOnDetentsDidChange().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern11, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    sheetStyle.sheetType = SheetType::SHEET_BOTTOM;
    bool isShow = true;
    auto heightVal = 500.0f;
    auto onDetentsDidChange = [&heightVal](float height) { heightVal = height; };
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, onDetentsDidChange, nullptr,
        nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);
    
    /**
     * @tc.steps: step3. test InitSheetDetents(), sheetType = BOTTOM.
     */
    topSheetPattern->pageHeight_ = 1000;
    topSheetPattern->sheetMaxHeight_ = 1000;
    topSheetPattern->sheetThemeType_ = "auto";
    sheetStyle.sheetType = SheetType::SHEET_BOTTOM;
    auto sheetLayoutProperty = topSheetNode->GetLayoutProperty<SheetPresentationProperty>();
    EXPECT_FALSE(sheetLayoutProperty == nullptr);

    /**
     * @tc.steps: step4. test sheetMode has value and sheetStyle.detents = {600,992}.
     * @tc.expected: Call FireOnDetentsDidChange.
     */
    SheetHeight detent;
    detent.sheetMode = SheetMode::MEDIUM;
    sheetStyle.detents.emplace_back(detent);
    topSheetPattern->FireOnDetentsDidChange(500);
    detent.sheetMode = SheetMode::LARGE;
    sheetStyle.detents.emplace_back(detent);
    topSheetPattern->FireOnDetentsDidChange(992);
    sheetLayoutProperty->UpdateSheetStyle(sheetStyle);
    topSheetPattern->InitSheetDetents();
    EXPECT_FALSE(NearEqual(topSheetPattern->sheetDetentHeight_.front(), 500));
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetDetentHeight_.back(), 992));

    /**
     * @tc.steps: step5. test sheetStyle.sheetHeight.height has value.
     * @tc.expected: Call FireOnDetentsDidChange.
     */
    sheetStyle.detents.clear();
    SheetHeight detent1;
    Dimension detentHeight { 0.5, DimensionUnit::PERCENT };
    detent1.height = detentHeight;
    sheetStyle.detents.emplace_back(detent1);
    topSheetPattern->FireOnDetentsDidChange(500);
}

/**
 * @tc.name: SheetPresentationPattern12
 * @tc.desc: Test SheetPresentationPattern::GetSheetType().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern12, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

     /**
     * @tc.steps: step3. create sheetNode, set width and mode floating.
     */
    MockPipelineContext::GetCurrent()->SetWindowModal(WindowModal::CONTAINER_MODAL);
    MockPipelineContext::GetCurrent()->windowManager_ = AceType::MakeRefPtr<WindowManager>();
    MockPipelineContext::GetCurrent()->windowManager_->SetWindowGetModeCallBack(
        []() -> WindowMode { return WindowMode::WINDOW_MODE_FLOATING; });

    RefPtr<PipelineBase> pipelineContext = NG::MockPipelineContext::pipeline_;
    auto windowGlobalRect = pipelineContext->GetDisplayWindowRectInfo();
    windowGlobalRect.SetSize(Size(200, 300));
    auto mode = topSheetPattern->GetSheetType();
    EXPECT_EQ(mode, SheetType::SHEET_BOTTOM);
}

/**
 * @tc.name: SheetPresentationPattern13
 * @tc.desc: Test SheetPresentationPattern::SheetInteractiveDismiss().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern13, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();

    /**
     * @tc.steps: step3. set onWillDismissFunc, sheetDetents.
     */
    topSheetPattern->sheetDetentHeight_.emplace_back(100);
    bool isDismiss = false;
    auto onWillDismissFunc = [&isDismiss](const int32_t) -> void { isDismiss = true; };
    bool isSpringBack = false;
    auto springBackFunc = [&isSpringBack]() -> void { isSpringBack = true; };

    /**
     * @tc.steps: step4. Trigger a shutdown event, test when the velocity is illegal.
     * @tc.expected: shutdown faild, onWillDismissFunc is not called.
     */
    topSheetPattern->UpdateOnWillDismiss(onWillDismissFunc);
    topSheetPattern->HandleDragEnd(-2000);
    EXPECT_FALSE(isDismiss);

    /**
     * @tc.steps: step5. Trigger a shutdown event.
     * @tc.expected: onWillDismissFunc is called, isDismiss = true, isSpringBack = true.
     */
    topSheetPattern->UpdateSheetSpringBack(springBackFunc);
    topSheetPattern->HandleDragEnd(100);
    EXPECT_TRUE(isDismiss);

    /**
     * @tc.steps: step6. Trigger a shutdown event, test when the velocity reaches the threshold.
     * @tc.expected: onWillDismissFunc is called, isDismiss = true, isSpringBack = false.
     */
    auto springBackFunc2 = [&isSpringBack]() -> void { isSpringBack = false; };
    topSheetPattern->UpdateSheetSpringBack(springBackFunc2);
    topSheetPattern->HandleDragEnd(2000);
    EXPECT_TRUE(isDismiss);
}

/**
 * @tc.name: SheetPresentationPattern13
 * @tc.desc: Test onWidthDidChange and onTypeDidChange.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern14, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);
    auto sheetLayoutAlgorithm =
    AceType::DynamicCast<SheetPresentationLayoutAlgorithm>(topSheetPattern->CreateLayoutAlgorithm());
    ASSERT_NE(sheetLayoutAlgorithm, nullptr);

    auto typeVal = float(SheetType::SHEET_BOTTOM);
    auto onTypeDidChangeFunc = [&typeVal](float type) { typeVal = type; };
    topSheetPattern->UpdateOnTypeDidChange(onTypeDidChangeFunc);
    topSheetPattern->FireOnTypeDidChange();

    auto maxSize = SizeF(10.0f, 10.0f);
    sheetLayoutAlgorithm->sheetType_ = SHEET_CENTER;
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(topSheetNode, topSheetNode->GetGeometryNode(),
        topSheetNode->GetLayoutProperty());
    auto widthVal = sheetLayoutAlgorithm->GetWidthByScreenSizeType(maxSize.Width(), Referenced::RawPtr(layoutWrapper));
    auto onWidthDidChangeFunc = [&widthVal](float width) { widthVal = width; };
    topSheetPattern->UpdateOnWidthDidChange(onWidthDidChangeFunc);
    topSheetPattern->FireOnWidthDidChange();
    EXPECT_EQ(widthVal, SHEET_LANDSCAPE_WIDTH.ConvertToPx());
}

/**
 * @tc.name: SheetPresentationPattern15
 * @tc.desc: Test height and detents when sheet is bottomLandsapce.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern15, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

    /**
     * @tc.steps: step3. test sheetThemeType_ = auto, sheetStyle.sheetType = bottomLandspace, set pageHeight = 500.
     */
    Container::Current()->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    topSheetPattern->sheetThemeType_ = "auto";
    sheetStyle.sheetType = SheetType::SHEET_BOTTOMLANDSPACE;
    topSheetPattern->pageHeight_ = 500;

    /**
     * @tc.steps: step4. test sheetStyle.detents.sheetMode has value, sheetMode = MEDIUM.
     */
    SheetHeight detent;
    detent.sheetMode = SheetMode::MEDIUM;
    sheetStyle.detents.emplace_back(detent);
    topSheetPattern->sheetHeightForTranslate_ = 0;
    overlayManager->ComputeSheetOffset(sheetStyle, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 300));
    sheetStyle.detents.clear();

    /**
     * @tc.steps: step5. test sheetStyle.detents.height has value, height unit is %.
     */
    sheetStyle.detents.clear();
    detent.sheetMode = std::nullopt;
    Dimension detentHeight { 0.5, DimensionUnit::PERCENT };
    detent.height = detentHeight;
    sheetStyle.detents.emplace_back(detent);
    topSheetPattern->sheetHeightForTranslate_ = 0;
    overlayManager->ComputeSheetOffset(sheetStyle, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 250));

    /**
     * @tc.steps: step6. test sheetStyle.detents.height has value, height unit is vp, setHeight > maxHeight.
     */
    sheetStyle.detents.clear();
    detent.height->SetUnit(DimensionUnit::VP);
    detent.height->SetValue(1500);
    sheetStyle.detents.emplace_back(detent);
    topSheetPattern->sheetHeightForTranslate_ = 0;
    overlayManager->ComputeSheetOffset(sheetStyle, topSheetNode);
    EXPECT_TRUE(NearEqual(topSheetPattern->sheetHeightForTranslate_, 492));
}

/**
 * @tc.name: SheetPresentationPattern16
 * @tc.desc: Test SheetPresentationPattern::InitialSingleGearHeight().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern16, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto sheetTheme = AceType::MakeRefPtr<SheetTheme>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(sheetTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(Return(sheetTheme));
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

    /**
     * @tc.steps: step3. test InitialSingleGearHeight().
     */
    topSheetPattern->pageHeight_ = 1000;
    topSheetPattern->sheetMaxHeight_ = 1000;
    topSheetPattern->isFirstInit_ = true;

    /**
     * @tc.steps: step4. set sheetStyle.sheetHeight.sheetMode = SheetMode::MEDIUM.
     * @tc.expected: height = 1000 * 0.6 = 600.
     */
    EXPECT_TRUE(NearEqual(topSheetPattern->InitialSingleGearHeight(sheetStyle), 600));
}

/**
 * @tc.name: SheetPresentationPattern17
 * @tc.desc: Test SheetPresentationPattern::InitialSingleGearHeight().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern17, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto sheetTheme = AceType::MakeRefPtr<SheetTheme>();
    sheetTheme->largePercent_ = 0.5;
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(sheetTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(Return(sheetTheme));
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::LARGE;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

    /**
     * @tc.steps: step3. test InitialSingleGearHeight().
     */
    topSheetPattern->pageHeight_ = 1000;
    topSheetPattern->sheetMaxHeight_ = 1000;
    topSheetPattern->isFirstInit_ = true;
    /**
     * @tc.steps: step5. set sheetStyle.sheetHeight.sheetMode = SheetMode::LARGE && heightApplyFullScreen_ = true.
     * @tc.expected: height = (1000 - 8) * 0.5 = 1000.
     */
    EXPECT_EQ(topSheetPattern->InitialSingleGearHeight(sheetStyle), 496);
}

/**
 * @tc.name: SheetPresentationPattern18
 * @tc.desc: Test SheetPresentationPattern::InitialSingleGearHeight().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern18, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto sheetTheme = AceType::MakeRefPtr<SheetTheme>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(sheetTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(Return(sheetTheme));
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    sheetStyle.sheetHeight.sheetMode = SheetMode::LARGE;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

    /**
     * @tc.steps: step3. test InitialSingleGearHeight().
     */
    topSheetPattern->pageHeight_ = 1000;
    topSheetPattern->sheetMaxHeight_ = 1000;
    topSheetPattern->isFirstInit_ = true;
    /**
     * @tc.steps: step4. set sheetStyle.sheetHeight.sheetMode = SheetMode::LARGE && heightApplyFullScreen_ = false.
     * @tc.expected: height = (1000 - 8) * 1.0.
     */
    EXPECT_EQ(topSheetPattern->InitialSingleGearHeight(sheetStyle), 992);
}

/**
 * @tc.name: SheetPresentationPattern4
 * @tc.desc: Test SheetPresentationPattern::InitialSingleGearHeight().
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetPresentationPattern4, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

    /**
     * @tc.steps: step3. test InitialSingleGearHeight().
     */
    topSheetPattern->pageHeight_ = 1000;
    topSheetPattern->sheetMaxHeight_ = 1000;
    topSheetPattern->isFirstInit_ = true;

    /**
     * @tc.steps: step4. set sheetStyle.sheetHeight.height = 0.5, unit is %.
     * @tc.expected: height = 1000*0.5 = 500.
     */
    Dimension singleHeight { 0.5, DimensionUnit::PERCENT };
    sheetStyle.sheetHeight.height = singleHeight;
    EXPECT_TRUE(NearEqual(topSheetPattern->InitialSingleGearHeight(sheetStyle), 500));

    /**
     * @tc.steps: step5. set sheetStyle.sheetHeight.height > maxHeight.
     * @tc.expected: height = 1000-8 = 992.
     */
    sheetStyle.sheetHeight.height->SetUnit(DimensionUnit::VP);
    sheetStyle.sheetHeight.height->SetValue(1200);
    EXPECT_TRUE(NearEqual(topSheetPattern->InitialSingleGearHeight(sheetStyle), 992));

    /**
     * @tc.steps: step6. set sheetStyle.sheetHeight.height < 0.
     * @tc.expected: height = 1000-8 = 992.
     */
    sheetStyle.sheetHeight.height->SetUnit(DimensionUnit::VP);
    sheetStyle.sheetHeight.height->SetValue(-10);
    EXPECT_TRUE(NearEqual(topSheetPattern->InitialSingleGearHeight(sheetStyle), 992));
}

/**
 * @tc.name: SheetMaterial001
 * @tc.desc: Test OverlayManager::OnBindSheet set SheetMaterial.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, SheetMaterial001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();
    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);

    Shadow shadow = ShadowConfig::DefaultShadowL;
    sheetStyle.borderWidth = BORDER_WIDTH_TEST;
    sheetStyle.borderColor = BORDER_COLOR_TEST;
    sheetStyle.borderStyle = BORDER_STYLE_TEST;
    sheetStyle.shadow = shadow;
    auto material = AceType::MakeRefPtr<UiMaterial>();
    auto type = static_cast<int32_t>(MaterialType::NONE);
    material->SetType(type);
    sheetStyle.systemMaterial = nullptr;

    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto sheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(sheetNode, nullptr);
    auto renderContext = sheetNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    EXPECT_EQ(renderContext->GetBorderWidth().value(), BORDER_WIDTH_TEST);
    EXPECT_EQ(renderContext->GetBorderColor().value(), BORDER_COLOR_TEST);
    EXPECT_EQ(renderContext->GetBorderStyle().value(), BORDER_STYLE_TEST);
    EXPECT_EQ(renderContext->GetBackShadow().value(), shadow);
}

/**
 * @tc.name: TestSheetPage001
 * @tc.desc: Test CreateSheetPage.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, TestSheetPage001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create sheet page.
     */
    auto builder = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto callback = [](const std::string&) {};
    SheetStyle style;
    style.isTitleBuilder = true;
    auto sheetNode = SheetView::CreateSheetPage(0, "", builder, builder, std::move(callback), style);
    ASSERT_NE(sheetNode, nullptr);

    /**
     * @tc.steps: step2. set style.isTitleBuilder = true.
     * @tc.expected: create titleColumn and operationColumn.GetChildren().size() equal 2.
     */
    auto sheetLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(sheetLayoutProperty, nullptr);
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(sheetPattern, nullptr);
    EXPECT_TRUE(sheetLayoutProperty->GetSheetStyle()->isTitleBuilder);
    auto operationColumn = sheetPattern->GetTitleBuilderNode();
    ASSERT_NE(operationColumn, nullptr);
    EXPECT_EQ(operationColumn->GetChildren().size(), 1);
}
/**
 * @tc.name: TestSheetPage002
 * @tc.desc: Test CreateSheetPage.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, TestSheetPage002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create sheet page.
     */
    auto builder = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto callback = [](const std::string&) {};
    SheetStyle style;
    style.isTitleBuilder = true;
    style.sheetTitle = MESSAGE;
    style.sheetSubtitle = MESSAGE;
    auto sheetNode = SheetView::CreateSheetPage(0, "", builder, builder, std::move(callback), style);
    ASSERT_NE(sheetNode, nullptr);

    /**
     * @tc.steps: step2. set style.isTitleBuilder = true、 sheetTitle and sheetSubtitle.
     * @tc.expected: create titleColumn and titleColumn.GetChildren().size() equal 3.
     */
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(sheetPattern, nullptr);
    auto sheetLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(sheetLayoutProperty, nullptr);
    EXPECT_TRUE(sheetLayoutProperty->GetSheetStyle()->isTitleBuilder);
    auto operationColumn = sheetPattern->GetTitleBuilderNode();
    ASSERT_NE(operationColumn, nullptr);
    EXPECT_EQ(operationColumn->GetChildren().size(), 1);
    auto titleColumn = operationColumn->GetLastChild();
    ASSERT_NE(titleColumn, nullptr);
    EXPECT_EQ(titleColumn->GetChildren().size(), 3);
}

/**
 * @tc.name: TestSheetPage003
 * @tc.desc: Test SheetPresentationLayoutAlgorithm::Measure.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, TestSheetPage003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create sheet page.
     */
    auto builder = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto callback = [](const std::string&) {};
    SheetStyle style;
    style.isTitleBuilder = true;
    style.sheetTitle = MESSAGE;
    style.sheetSubtitle = MESSAGE;
    auto sheetNode = SheetView::CreateSheetPage(0, "", builder, builder, std::move(callback), style);
    ASSERT_NE(sheetNode, nullptr);
    auto sheetWrapperNode = FrameNode::CreateFrameNode(V2::SHEET_WRAPPER_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SheetWrapperPattern>());
    ASSERT_NE(sheetWrapperNode, nullptr);
    sheetNode->MountToParent(sheetWrapperNode);
    /**
     * @tc.steps: step2. call Measure function.
     * @tc.expected: sheetHeight_ equal 320.
     */
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(sheetPattern, nullptr);
    auto sheetLayoutAlgorithm =
        AceType::DynamicCast<SheetPresentationLayoutAlgorithm>(sheetPattern->CreateLayoutAlgorithm());
    ASSERT_NE(sheetLayoutAlgorithm, nullptr);
    sheetNode->layoutAlgorithm_ = AceType::MakeRefPtr<LayoutAlgorithmWrapper>(sheetLayoutAlgorithm);
    sheetNode->Measure(sheetNode->GetLayoutConstraint());
    EXPECT_EQ(sheetLayoutAlgorithm->sheetHeight_, 2000);

    sheetLayoutAlgorithm->sheetType_ = SHEET_CENTER;
    sheetLayoutAlgorithm->sheetStyle_.sheetHeight.sheetMode = SheetMode::AUTO;
    auto layoutProperty = AceType::DynamicCast<SheetPresentationProperty>(sheetNode->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    layoutProperty->UpdateSheetStyle(sheetLayoutAlgorithm->sheetStyle_);
    auto maxSize = SizeF(10.0f, 10.0f);
    sheetLayoutAlgorithm->Measure(AceType::RawPtr(sheetNode));
    sheetLayoutAlgorithm->GetHeightByScreenSizeType(maxSize.Height(), maxSize.Height(), AceType::RawPtr(sheetNode));
    sheetLayoutAlgorithm->sheetType_ = SHEET_POPUP;
    sheetLayoutAlgorithm->GetHeightByScreenSizeType(maxSize.Height(), maxSize.Width(), AceType::RawPtr(sheetNode));
    EXPECT_NE(sheetLayoutAlgorithm->sheetHeight_, 320);
}

/**
 * @tc.name: TestSheetPage004
 * @tc.desc: Test SheetPresentationLayoutAlgorithm::GetHeightBySheetStyle.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, TestSheetPage004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create sheet page.
     */
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto sheetTheme = AceType::MakeRefPtr<SheetTheme>();
    sheetTheme->centerDefaultWidth_ = SHEET_LANDSCAPE_WIDTH;
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(sheetTheme));
    EXPECT_CALL(*themeManager, GetTheme(_, _)).WillRepeatedly(Return(sheetTheme));
    auto builder = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto callback = [](const std::string&) {};
    SheetStyle style;
    style.isTitleBuilder = true;
    style.sheetTitle = MESSAGE;
    style.sheetSubtitle = MESSAGE;
    auto sheetNode = SheetView::CreateSheetPage(0, "", builder, builder, std::move(callback), style);
    ASSERT_NE(sheetNode, nullptr);

    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(sheetPattern, nullptr);
    auto sheetLayoutAlgorithm =
        AceType::DynamicCast<SheetPresentationLayoutAlgorithm>(sheetPattern->CreateLayoutAlgorithm());
    ASSERT_NE(sheetLayoutAlgorithm, nullptr);

    /**
     * @tc.steps: step2. set sheetStyle_.height and sheetStyle_.width.
     * @tc.expected: height and width value are equal expected value.
     */
    auto maxHeight = 1000;
    auto maxWidth = 1000;
    sheetLayoutAlgorithm->GetHeightBySheetStyle(maxHeight, maxWidth, AceType::RawPtr(sheetNode));

    sheetLayoutAlgorithm->sheetStyle_.sheetHeight.height = 2.5_pct;
    sheetLayoutAlgorithm->GetHeightBySheetStyle(maxHeight, maxWidth, AceType::RawPtr(sheetNode));
    sheetLayoutAlgorithm->sheetStyle_.sheetHeight.height = 2.5_px;
    sheetLayoutAlgorithm->GetHeightBySheetStyle(maxHeight, maxWidth, AceType::RawPtr(sheetNode));
    sheetLayoutAlgorithm->sheetStyle_.sheetHeight.height = 0.0_px;
    auto height = sheetLayoutAlgorithm->GetHeightBySheetStyle(maxHeight, maxWidth, AceType::RawPtr(sheetNode));
    EXPECT_EQ(height, sheetTheme->bigWindowMinHeight_.ConvertToPx());
    sheetLayoutAlgorithm->sheetStyle_.sheetHeight.height = -1.0_px;
    height = sheetLayoutAlgorithm->GetHeightBySheetStyle(maxHeight, maxWidth, AceType::RawPtr(sheetNode));
    EXPECT_EQ(height, SHEET_BIG_WINDOW_HEIGHT.ConvertToPx());

    sheetLayoutAlgorithm->sheetType_ = SHEET_CENTER;
    auto maxSize = SizeF(10.0f, 10.0f);
    auto layoutWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(sheetNode, sheetNode->GetGeometryNode(),
        sheetNode->GetLayoutProperty());
    auto width = sheetLayoutAlgorithm->GetWidthByScreenSizeType(maxSize.Width(), Referenced::RawPtr(layoutWrapper));
    EXPECT_EQ(width, SHEET_LANDSCAPE_WIDTH.ConvertToPx());
    sheetLayoutAlgorithm->sheetType_ = SHEET_POPUP;
    width = sheetLayoutAlgorithm->GetWidthByScreenSizeType(maxSize.Width(), Referenced::RawPtr(layoutWrapper));
    EXPECT_EQ(width, SHEET_POPUP_WIDTH.ConvertToPx());
}

/**
 * @tc.name: GetSheetType001
 * @tc.desc: Test SheetPresentationPattern::GetSheetType::
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, GetSheetType001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create builder.
     */
    CreateSheetBuilder();

    /**
     * @tc.steps: step3. create sheet node and get sheet node, get pattern.
     * @tc.expected: related function is called.
     */
    SheetStyle sheetStyle;
    CreateSheetStyle(sheetStyle);
    bool isShow = true;
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    EXPECT_FALSE(topSheetNode == nullptr);
    auto sheetNodeLayoutProperty = topSheetNode->GetLayoutProperty<SheetPresentationProperty>();
    auto style = sheetNodeLayoutProperty->GetSheetStyle();
    EXPECT_FALSE(style->sheetType.has_value());

    /**
     * @tc.steps: step4. Change the sheetType.
     * @tc.expected: the sheetType is updated successfully
     */
    sheetStyle.sheetType = SheetType::SHEET_BOTTOM;
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);
    auto sheetNode = overlayManager->modalStack_.top().Upgrade();
    EXPECT_FALSE(sheetNode == nullptr);
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    EXPECT_EQ(sheetPattern->GetSheetType(), SheetType::SHEET_BOTTOM);
    sheetNodeLayoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    style = sheetNodeLayoutProperty->GetSheetStyle();
    EXPECT_TRUE(style->sheetType.has_value());
    EXPECT_EQ(style->sheetType.value(), SheetType::SHEET_BOTTOM);
}

/**
 * @tc.name: GetSheetType002
 * @tc.desc: Test SheetPresentationPattern::GetSheetType.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, GetSheetType002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create sheet page, get sheet pattern.
     */
    auto callback = [](const std::string&) {};
    auto sheetNode = FrameNode::CreateFrameNode(V2::SHEET_PAGE_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<SheetPresentationPattern>(0, "", std::move(callback)));
    ASSERT_NE(sheetNode, nullptr);
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(sheetPattern, nullptr);
    sheetPattern->sheetThemeType_ = "auto";

    auto pipelineContext = MockPipelineContext::GetCurrentContext();
    ASSERT_NE(pipelineContext, nullptr);
    pipelineContext->SetDisplayWindowRectInfo({0, 0, 800, 800});

    /**
     * @tc.steps: step2. Change the sheetType.
     * @tc.expected: the sheetType is updated successfully
     */
    SheetStyle sheetStyle;
    sheetStyle.sheetType = SheetType::SHEET_POPUP;
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateSheetStyle(sheetStyle);
    sheetPattern->GetSheetType();
    auto style = layoutProperty->GetSheetStyle();
    EXPECT_TRUE(style->sheetType.has_value());
    EXPECT_EQ(style->sheetType.value(), SheetType::SHEET_POPUP);
}

/**
 * @tc.name: IsShowCloseIcon
 * @tc.desc: Test SheetPresentationPattern::IsShowCloseIcon.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, IsShowCloseIcon001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create sheet page, get sheet pattern.
     */
    auto callback = [](const std::string&) {};
    auto sheetNode = FrameNode::CreateFrameNode(V2::SHEET_PAGE_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<SheetPresentationPattern>(0, "", std::move(callback)));
    ASSERT_NE(sheetNode, nullptr);
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(sheetPattern, nullptr);

    /**
     * @tc.steps: step2. Change the showCloseIcon.
     * @tc.expected: the showCloseIcon is updated successfully
     */
    SheetStyle sheetStyle;
    auto layoutProperty = sheetNode->GetLayoutProperty<SheetPresentationProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    layoutProperty->UpdateSheetStyle(sheetStyle);
    auto showCloseIcon = sheetPattern->IsShowCloseIcon();
    EXPECT_EQ(showCloseIcon, true);

    sheetStyle.showCloseIcon = true;
    layoutProperty->UpdateSheetStyle(sheetStyle);
    showCloseIcon = sheetPattern->IsShowCloseIcon();
    EXPECT_EQ(showCloseIcon, true);

    sheetStyle.showCloseIcon = false;
    layoutProperty->UpdateSheetStyle(sheetStyle);
    showCloseIcon = sheetPattern->IsShowCloseIcon();
    EXPECT_EQ(showCloseIcon, false);
}

/**
 * @tc.name: GetTitleNode001
 * @tc.desc: Test SheetPresentationPattern::GetTitleNode001
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, GetTitleNode001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create builder.
     */
    CreateSheetBuilder();

    /**
     * @tc.steps: step3. create sheet node and get sheet node, get pattern.
     * @tc.expected: related function is called.
     */
    SheetStyle sheetStyle;
    sheetStyle.isTitleBuilder = true;
    CreateSheetStyle(sheetStyle);
    bool isShow = true;
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, targetNode);

    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto topSheetNode = overlayManager->modalStack_.top().Upgrade();
    EXPECT_FALSE(topSheetNode == nullptr);
    auto sheetPattern = topSheetNode->GetPattern<SheetPresentationPattern>();
    auto titleNode = sheetPattern->GetTitleNode();
    EXPECT_NE(titleNode, nullptr);
}

/**
 * @tc.name: TestSuitAgingScale
 * @tc.desc: Test SheetPresentationPattern::UpdateFontScaleStatus.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, TestSuitAgingScale, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create sheet page, get sheet pattern.
     */
    auto builder = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto callback = [](const std::string&) {};
    SheetStyle style;
    style.isTitleBuilder = true;
    style.sheetTitle = MESSAGE;
    style.sheetSubtitle = MESSAGE;
    auto sheetNode = SheetView::CreateSheetPage(0, "", builder, builder, std::move(callback), style);
    ASSERT_NE(sheetNode, nullptr);
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(sheetPattern, nullptr);

    /**
     * @tc.steps: step2. test when fontcale changed.
     * @tc.expected: sheetPattern->scale_ = pipeline->GetFontScale().
     */
    sheetPattern->scale_ = 1.75f;
    auto pipeline = PipelineContext::GetCurrentContext();
    pipeline->fontScale_ = 1.0f;
    sheetPattern->UpdateFontScaleStatus();
    EXPECT_EQ(sheetPattern->scale_, pipeline->GetFontScale());
}

/**
 * @tc.name: TestSheetPage005
 * @tc.desc: Test SheetView::CreateTitleColumn.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, TestSheetPage005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create sheet page with title and subtitle, get sheet pattern.
     */
    auto builder = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto callback = [](const std::string&) {};
    SheetStyle style;
    style.isTitleBuilder = true;
    style.sheetTitle = MESSAGE;
    style.sheetSubtitle = MESSAGE;
    auto sheetNode = SheetView::CreateSheetPage(0, "", builder, nullptr, std::move(callback), style);
    ASSERT_NE(sheetNode, nullptr);
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(sheetPattern, nullptr);

    /**
     * @tc.steps: step2. get main title node, save title positionY.
     */
    auto titleNode =
        AceType::DynamicCast<FrameNode>(ElementRegister::GetInstance()->GetNodeById(sheetPattern->GetTitleId()));
    ASSERT_NE(titleNode, nullptr);
    auto offsetY = titleNode->GetPositionToScreen().GetY();

    /**
     * @tc.steps: step3. create sheet page only with title, get sheet pattern.
     */
    SheetStyle sheetStyle;
    sheetStyle.sheetTitle = MESSAGE;
    auto topSheetNode = SheetView::CreateSheetPage(0, "", builder, builder, std::move(callback), sheetStyle);
    ASSERT_NE(topSheetNode, nullptr);
    auto topSheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(topSheetPattern, nullptr);

    /**
     * @tc.steps: step2. get main title node, save title positionY, compare two positionY.
     * @tc.expected: two positionY equal.
     */
    auto topTitleNode =
        AceType::DynamicCast<FrameNode>(ElementRegister::GetInstance()->GetNodeById(topSheetPattern->GetTitleId()));
    ASSERT_NE(topTitleNode, nullptr);
    auto topOffsetY = topTitleNode->GetPositionToScreen().GetY();
    EXPECT_EQ(topOffsetY, offsetY);
}

/**
 * @tc.name: TestSheetPage006
 * @tc.desc: Test CreateSheetPage.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, TestSheetPage006, TestSize.Level1)
{
    auto builder = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto callback = [](const std::string&) {};
    SheetStyle style;
    style.isTitleBuilder = true;
    style.detents = {
        SheetHeight{std::make_optional(Dimension(100, DimensionUnit::VP)), std::make_optional(SheetMode::MEDIUM)},
        SheetHeight{std::make_optional(Dimension(300, DimensionUnit::VP)), std::make_optional(SheetMode::LARGE)},
        SheetHeight{std::make_optional(Dimension(500, DimensionUnit::VP)), std::make_optional(SheetMode::AUTO)}
    };
    style.enableFloatingDragBar = true;
    style.showDragBar = true;
    RefPtr<FrameNode> titleBuilder = builder;

    auto sheetNode = SheetView::CreateSheetPage(
        0, "", builder, titleBuilder, std::move(callback), style);
    ASSERT_NE(sheetNode, nullptr);
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(sheetPattern, nullptr);
    auto scrollNode = sheetPattern->GetSheetScrollNode();
    ASSERT_NE(scrollNode, nullptr);

    auto operationColumn = sheetPattern->GetTitleBuilderNode();
    ASSERT_NE(operationColumn, nullptr);
    EXPECT_TRUE(scrollNode->GetParent() == sheetNode->GetPattern()->frameNode_.Upgrade());
    EXPECT_EQ(operationColumn->GetChildren().size(), 1);
}

/**
 * @tc.name: PlaySheetTransition001
 * @tc.desc: Test PlaySheetTransition.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, PlaySheetTransition001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto targetNode = CreateTargetNode();
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    stageNode->MountToParent(rootNode);
    targetNode->MountToParent(stageNode);
    rootNode->MarkDirtyNode();

    /**
     * @tc.steps: step2. create sheetNode, get sheetPattern.
     */
    SheetStyle sheetStyle;
    bool isShow = true;
    CreateSheetBuilder();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    overlayManager->OnBindSheet(isShow, nullptr, std::move(builderFunc_), std::move(titleBuilderFunc_), sheetStyle,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr, nullptr, targetNode);
    EXPECT_FALSE(overlayManager->modalStack_.empty());
    auto sheetNode = overlayManager->modalStack_.top().Upgrade();
    ASSERT_NE(sheetNode, nullptr);
    auto sheetPattern = sheetNode->GetPattern<SheetPresentationPattern>();
    ASSERT_NE(sheetPattern, nullptr);
    /**
     * @tc.steps: step. set sheetHeightForTranslate_ is 0.0f, PlaySheetTransition.
     * @tc.expected: NearZero(sheetPattern->sheetHeightForTranslate_) is true.
     */
    sheetPattern->sheetHeightForTranslate_ = 0.0f;
    overlayManager->PlaySheetTransition(sheetNode, true, true);
    EXPECT_TRUE(NearZero(sheetPattern->sheetHeightForTranslate_));
}

/**
 * @tc.name: RebuildCustomBuilder001
 * @tc.desc: Test OverlayManager::RebuildCustomBuilder
 * @tc.type: FUNC
 */
 HWTEST_F(OverlayManagerTestNg, RebuildCustomBuilder001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create overlayManager
     */
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    ASSERT_NE(overlayManager, nullptr);
    overlayManager->CheckDialogInnerManager();
    auto dialogInnerManager = AceType::DynamicCast<DialogInnerManager>(overlayManager->dialogInnerManager_);
    ASSERT_NE(dialogInnerManager, nullptr);
    RefPtr<UINode> customNode;
    /**
     * @tc.steps: step2. test RebuildCustomBuilder
     */
    auto result = dialogInnerManager->RebuildCustomBuilder(customNode);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: RemovePixelMapAnimationTest001
 * @tc.desc: Verify early return when isOnAnimation_ is true
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, RemovePixelMapAnimationTest001, TestSize.Level1)
{
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(nullptr);
    ASSERT_NE(overlayManager, nullptr);
    overlayManager->isOnAnimation_ = true;
    overlayManager->hasPixelMap_ = true;
    overlayManager->RemovePixelMapAnimation(true, PIXELMAP_WIDTH, PIXELMAP_HEIGHT, false);
    EXPECT_TRUE(overlayManager->isOnAnimation_);
}

/**
 * @tc.name: RemovePixelMapAnimationTest002
 * @tc.desc: Verify early return when hasPixelMap_ is false
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, RemovePixelMapAnimationTest002, TestSize.Level1)
{
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(nullptr);
    ASSERT_NE(overlayManager, nullptr);
    overlayManager->isOnAnimation_ = false;
    overlayManager->hasPixelMap_ = false;

    overlayManager->RemovePixelMapAnimation(true, PIXELMAP_WIDTH, PIXELMAP_HEIGHT, false);
    EXPECT_FALSE(overlayManager->isOnAnimation_);
}

/**
 * @tc.name: RemovePixelMapAnimationTest003
 * @tc.desc: Verify RemovePixelMap is triggered when startDrag = true and isSubwindowOverlay = false
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, RemovePixelMapAnimationTest003, TestSize.Level1)
{
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(nullptr);
    ASSERT_NE(overlayManager, nullptr);
    overlayManager->isOnAnimation_ = false;
    overlayManager->hasPixelMap_ = true;
    overlayManager->RemovePixelMapAnimation(true, PIXELMAP_WIDTH, PIXELMAP_HEIGHT, false);
    EXPECT_FALSE(overlayManager->hasPixelMap_);
    EXPECT_FALSE(overlayManager->isOnAnimation_);
}

/**
 * @tc.name: RemovePixelMapAnimationTest004
 * @tc.desc: Verify RemovePixelMap is skipped when isSubwindowOverlay = true
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, RemovePixelMapAnimationTest004, TestSize.Level1)
{
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(nullptr);
    ASSERT_NE(overlayManager, nullptr);
    overlayManager->isOnAnimation_ = false;
    overlayManager->hasPixelMap_ = true;
    overlayManager->RemovePixelMapAnimation(true, PIXELMAP_WIDTH, PIXELMAP_HEIGHT, true);
    EXPECT_TRUE(overlayManager->hasPixelMap_);
    EXPECT_FALSE(overlayManager->isOnAnimation_);
}

/**
 * @tc.name: RemovePixelMapAnimationTest005
 * @tc.desc: Verify RemoveEventColumn is called when pixmapColumnNode is null
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, RemovePixelMapAnimationTest005, TestSize.Level1)
{
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(nullptr);
    ASSERT_NE(overlayManager, nullptr);
    overlayManager->isOnAnimation_ = false;
    overlayManager->hasPixelMap_ = true;
    overlayManager->pixmapColumnNodeWeak_ = WeakPtr<FrameNode>();
    overlayManager->RemovePixelMapAnimation(false, PIXELMAP_WIDTH, PIXELMAP_HEIGHT, false);
    EXPECT_FALSE(overlayManager->hasPixelMap_);
    EXPECT_FALSE(overlayManager->isOnAnimation_);
}

/**
 * @tc.name: GetPrepareDragFrameNodeBorderRadiusTest001
 * @tc.desc: Return default radius when mainPipeline is null
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, GetPrepareDragFrameNodeBorderRadiusTest001, TestSize.Level1)
{
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(nullptr);
    ASSERT_NE(overlayManager, nullptr);
    MockPipelineContext::SetUp();

    auto result = overlayManager->GetPrepareDragFrameNodeBorderRadius();
    BorderRadiusProperty defaultRadius(Dimension(0), Dimension(0), Dimension(0), Dimension(0));
    EXPECT_EQ(result, defaultRadius);
}

/**
 * @tc.name: GetPrepareDragFrameNodeBorderRadiusTest002
 * @tc.desc: Return default radius when dragDropManager is null
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, GetPrepareDragFrameNodeBorderRadiusTest002, TestSize.Level1)
{
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(nullptr);
    ASSERT_NE(overlayManager, nullptr);
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    MockPipelineContext::GetCurrent()->dragDropManager_ = nullptr;
    MockPipelineContext::SetUp();

    auto result = overlayManager->GetPrepareDragFrameNodeBorderRadius();
    BorderRadiusProperty defaultRadius(Dimension(0), Dimension(0), Dimension(0), Dimension(0));
    EXPECT_EQ(result, defaultRadius);
}

/**
 * @tc.name: GetPrepareDragFrameNodeBorderRadiusTest003
 * @tc.desc: Return default radius when dragFrameNode is null
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, GetPrepareDragFrameNodeBorderRadiusTest003, TestSize.Level1)
{
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(nullptr);
    ASSERT_NE(overlayManager, nullptr);
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    auto dragDropManager = pipeline->GetDragDropManager();
    ASSERT_NE(dragDropManager, nullptr);
    MockPipelineContext::SetUp();

    DragDropGlobalController::GetInstance().SetPrepareDragFrameNode(WeakPtr<FrameNode>());
    auto result = overlayManager->GetPrepareDragFrameNodeBorderRadius();
    BorderRadiusProperty defaultRadius(Dimension(0), Dimension(0), Dimension(0), Dimension(0));
    EXPECT_EQ(result, defaultRadius);
}

/**
 * @tc.name: GetPrepareDragFrameNodeBorderRadiusTest004
 * @tc.desc: Return computed radius when all dependencies valid
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, GetPrepareDragFrameNodeBorderRadiusTest004, TestSize.Level1)
{
    MockPipelineContext::SetUp();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(nullptr);
    ASSERT_NE(overlayManager, nullptr);
    auto frameNode = FrameNode::CreateFrameNode(
        V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(frameNode, nullptr);
    auto layoutProp = frameNode->GetLayoutProperty();
    auto renderContext = frameNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    renderContext->UpdateBorderRadius(BorderRadiusProperty(
        Dimension(BORDER_VALUE), Dimension(BORDER_VALUE), Dimension(BORDER_VALUE), Dimension(BORDER_VALUE)));

    DragDropGlobalController::GetInstance().SetPrepareDragFrameNode(frameNode);

    auto result = overlayManager->GetPrepareDragFrameNodeBorderRadius();
    BorderRadiusProperty defaultRadius(Dimension(0), Dimension(0), Dimension(0), Dimension(0));
    EXPECT_NE(result.radiusTopLeft, defaultRadius.radiusTopLeft);
    EXPECT_NE(result.radiusTopRight, defaultRadius.radiusTopRight);
    EXPECT_NE(result.radiusBottomLeft, defaultRadius.radiusBottomLeft);
    EXPECT_NE(result.radiusBottomRight, defaultRadius.radiusBottomRight);
}

/**
 * @tc.name: MountPixelMapToRootNode001
 * @tc.desc: Test MountPixelMapToRootNode when Pattern not is ContainerModalPattern.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, MountPixelMapToRootNode001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ContainerModalNode and overlay
     */
    auto pipeline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    auto overlayManage = pipeline->overlayManager_;
    ASSERT_NE(overlayManage, nullptr);
    auto rootNode = pipeline->rootNode_;
    ASSERT_NE(rootNode, nullptr);
    /**
     * @tc.steps: step2. call MountPixelMapToRootNode when hostNode is nullptr, pipeline->windowModal_ is
     * WindowModal::NORMAL.
     * @tc.expected: column->GetParent() is rootNode
     */
    pipeline->windowModal_ = WindowModal::NORMAL;
    auto column =
        FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), nullptr);
    overlayManage->MountPixelMapToRootNode(column, false, nullptr);
    EXPECT_EQ(column->GetParent(), rootNode);

    /**
     * @tc.steps: step3. call MountPixelMapToRootNode when hostNode is nullptr, pipeline->windowModal_ is
     * WindowModal::CONTAINER_MODAL.
     * @tc.expected: column->GetParent() is rootNode
     */
    pipeline->windowModal_ = WindowModal::CONTAINER_MODAL;
    overlayManage->MountPixelMapToRootNode(column, false, nullptr);
    EXPECT_EQ(column->GetParent(), rootNode);
    pipeline->windowModal_ = WindowModal::NORMAL;
}

/**
 * @tc.name: MountPixelMapToRootNode002
 * @tc.desc: Test MountPixelMapToRootNode when Pattern is ContainerModalPattern.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, MountPixelMapToRootNode002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create ContainerModalNode and overlay
     */
    auto pipeline = PipelineContext::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    auto overlayManage = pipeline->overlayManager_;
    ASSERT_NE(overlayManage, nullptr);
    auto containerModalNode = FrameNode::CreateFrameNode(
        "ContainerModal", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ContainerModalPattern>());
    auto rootNode = pipeline->rootNode_;
    ASSERT_NE(rootNode, nullptr);
    rootNode->AddChild(containerModalNode, 0);
    /**
     * @tc.steps: step2. call MountPixelMapToRootNode when hostNode is nullptr, pipeline->windowModal_ is
     * WindowModal::CONTAINER_MODAL.
     * @tc.expected: column->GetParent() is rootNode
     */
    pipeline->windowModal_ = WindowModal::CONTAINER_MODAL;
    auto column =
        FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), nullptr);
    overlayManage->MountPixelMapToRootNode(column, false, nullptr);
    EXPECT_EQ(column->GetParent(), rootNode);

    /**
     * @tc.steps: step3. call MountPixelMapToRootNode when hostNode is node and parent is toolbarItem.
     * @tc.expected: column->GetParent() is ContainerModalNode
     */
    rootNode->RemoveChild(column);
    auto hostNode =
        FrameNode::GetOrCreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), nullptr);
    auto toolbarItem = FrameNode::GetOrCreateFrameNode(
        V2::TOOLBARITEM_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), nullptr);
    toolbarItem->AddChild(hostNode);
    overlayManage->MountPixelMapToRootNode(column, false, hostNode);
    EXPECT_EQ(column->GetParent(), containerModalNode);
    rootNode->RemoveChild(containerModalNode);
    pipeline->windowModal_ = WindowModal::NORMAL;
}

/**
 * @tc.name: OverlayManagerTest_RemoveChildWithService001
 * @tc.desc: Test RemoveChildWithService calls OnRemoveChild for popup nodes.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, OverlayManagerTest_RemoveChildWithService001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create rootNode and popup node with target.
     */
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    ASSERT_NE(rootNode, nullptr);

    auto targetNode = CreateTargetNode();
    ASSERT_NE(targetNode, nullptr);
    auto targetId = targetNode->GetId();
    auto targetTag = targetNode->GetTag();

    auto popupId = ElementRegister::GetInstance()->MakeUniqueId();
    auto popupNode = FrameNode::CreateFrameNode(
        V2::POPUP_ETS_TAG, popupId, AceType::MakeRefPtr<BubblePattern>(targetId, targetTag));
    ASSERT_NE(popupNode, nullptr);
    auto bubblePattern = popupNode->GetPattern<BubblePattern>();
    ASSERT_NE(bubblePattern, nullptr);

    /**
     * @tc.steps: step2. Mount popup node to rootNode and set up test conditions.
     */
    rootNode->AddChild(popupNode);
    bubblePattern->hasOnAreaChange_ = false;

    /**
     * @tc.steps: step3. Create OverlayManager and call RemoveChildWithService.
     * @tc.expected: Method should execute without crash and call OnRemoveChild.
     */
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    ASSERT_NE(overlayManager, nullptr);
    overlayManager->RemoveChildWithService(rootNode, popupNode);

    /**
     * @tc.steps: step4. Verify popup node is removed from rootNode.
     * @tc.expected: rootNode should no longer have popupNode as child.
     */
    auto children = rootNode->GetChildren();
    auto it = std::find_if(children.begin(), children.end(),
        [&popupNode](const RefPtr<UINode>& child) { return child == popupNode; });
    EXPECT_EQ(it, children.end());
}

/**
 * @tc.name: OverlayManagerTest_RemoveChildWithService002
 * @tc.desc: Test RemoveChildWithService with null node parameter.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, OverlayManagerTest_RemoveChildWithService002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create rootNode and OverlayManager.
     */
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    ASSERT_NE(rootNode, nullptr);
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    ASSERT_NE(overlayManager, nullptr);

    /**
     * @tc.steps: step2. Call RemoveChildWithService with null node.
     * @tc.expected: Method should handle null node gracefully without crash.
     */
    overlayManager->RemoveChildWithService(rootNode, nullptr);

    /**
     * @tc.steps: step3. Verify method completes successfully.
     * @tc.expected: No crash occurs when node is null.
     */
    EXPECT_TRUE(true);
}

/**
 * @tc.name: OverlayManagerTest_RemoveChildWithService003
 * @tc.desc: Test RemoveChildWithService with null rootNode parameter.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, OverlayManagerTest_RemoveChildWithService003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create popup node.
     */
    auto targetNode = CreateTargetNode();
    ASSERT_NE(targetNode, nullptr);
    auto targetId = targetNode->GetId();
    auto targetTag = targetNode->GetTag();

    auto popupId = ElementRegister::GetInstance()->MakeUniqueId();
    auto popupNode = FrameNode::CreateFrameNode(
        V2::POPUP_ETS_TAG, popupId, AceType::MakeRefPtr<BubblePattern>(targetId, targetTag));
    ASSERT_NE(popupNode, nullptr);

    /**
     * @tc.steps: step2. Create OverlayManager with null rootNode.
     * @tc.expected: OverlayManager can be created but RemoveChildWithService should handle null gracefully.
     */
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(nullptr);
    ASSERT_NE(overlayManager, nullptr);

    /**
     * @tc.steps: step3. Call RemoveChildWithService with null rootNode.
     * @tc.expected: Method should handle null rootNode gracefully without crash.
     */
    overlayManager->RemoveChildWithService(nullptr, popupNode);

    /**
     * @tc.steps: step4. Verify method completes successfully.
     * @tc.expected: No crash occurs when rootNode is null.
     */
    EXPECT_TRUE(true);
}

/**
 * @tc.name: OverlayManagerTest_RemoveChildWithService004
 * @tc.desc: Test RemoveChildWithService calls OnRemoveChild when hasOnAreaChange is true.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, OverlayManagerTest_RemoveChildWithService004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create rootNode and popup node with target.
     */
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    ASSERT_NE(rootNode, nullptr);

    auto targetNode = CreateTargetNode();
    ASSERT_NE(targetNode, nullptr);
    auto targetId = targetNode->GetId();
    auto targetTag = targetNode->GetTag();

    auto popupId = ElementRegister::GetInstance()->MakeUniqueId();
    auto popupNode = FrameNode::CreateFrameNode(
        V2::POPUP_ETS_TAG, popupId, AceType::MakeRefPtr<BubblePattern>(targetId, targetTag));
    ASSERT_NE(popupNode, nullptr);
    auto bubblePattern = popupNode->GetPattern<BubblePattern>();
    ASSERT_NE(bubblePattern, nullptr);

    /**
     * @tc.steps: step2. Mount popup node to rootNode and set hasOnAreaChange to true.
     */
    rootNode->AddChild(popupNode);
    bubblePattern->hasOnAreaChange_ = true;

    /**
     * @tc.steps: step3. Create OverlayManager and call RemoveChildWithService.
     * @tc.expected: Method should execute without crash and call OnRemoveChild.
     */
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    ASSERT_NE(overlayManager, nullptr);
    overlayManager->RemoveChildWithService(rootNode, popupNode);

    /**
     * @tc.steps: step4. Verify popup node is removed from rootNode.
     * @tc.expected: rootNode should no longer have popupNode as child.
     */
    auto children = rootNode->GetChildren();
    auto it = std::find_if(children.begin(), children.end(),
        [&popupNode](const RefPtr<UINode>& child) { return child == popupNode; });
    EXPECT_EQ(it, children.end());
}

/**
 * @tc.name: OverlayManagerTest_RemoveChildWithService005
 * @tc.desc: Test RemoveChildWithService with non-popup node (no PopupBasePattern).
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, OverlayManagerTest_RemoveChildWithService005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create rootNode and regular node (ButtonNode, not a popup).
     */
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    ASSERT_NE(rootNode, nullptr);

    auto buttonNode = CreateTargetNode();
    ASSERT_NE(buttonNode, nullptr);

    /**
     * @tc.steps: step2. Mount button node to rootNode.
     */
    rootNode->AddChild(buttonNode);

    /**
     * @tc.steps: step3. Create OverlayManager and call RemoveChildWithService.
     * @tc.expected: Method should execute without crash, OnRemoveChild should not be called.
     */
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);
    ASSERT_NE(overlayManager, nullptr);
    overlayManager->RemoveChildWithService(rootNode, buttonNode);

    /**
     * @tc.steps: step4. Verify button node is removed from rootNode.
     * @tc.expected: rootNode should no longer have buttonNode as child.
     */
    auto children = rootNode->GetChildren();
    auto it = std::find_if(children.begin(), children.end(),
        [&buttonNode](const RefPtr<UINode>& child) { return child == buttonNode; });
    EXPECT_EQ(it, children.end());
}

/**
 * @tc.name: RemoveOverlayOnBackPress001
 * @tc.desc: Test RemoveOverlay with onBackPress callback returning true (intercepted).
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, RemoveOverlayOnBackPress001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create overlayManager with rootNode and stage node.
     */
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    stageNode->MountToParent(rootNode);
    rootNode->MarkDirtyNode();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);

    /**
     * @tc.steps: step2. set overlayManagerOptions with enableBackPressedEvent=true and onBackPress returning true.
     */
    bool callbackInvoked = false;
    OverlayManagerInfo info;
    info.enableBackPressedEvent = true;
    info.onBackPress = [&callbackInvoked]() -> bool {
        callbackInvoked = true;
        return true;
    };
    overlayManager->SetOverlayManagerOptions(info);

    /**
     * @tc.steps: step3. add a component content node to overlayNode so GetLastChildNotRemoving finds it.
     */
    auto contentNode = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    overlayManager->AddFrameNodeToOverlay(contentNode);

    /**
     * @tc.steps: step4. call RemoveOverlay with isBackPressed=true.
     * @tc.expected: onBackPress callback is invoked and RemoveOverlay returns true (intercepted).
     */
    auto result = overlayManager->RemoveOverlay(true);
    EXPECT_TRUE(callbackInvoked);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: RemoveOverlayOnBackPress002
 * @tc.desc: Test RemoveOverlay with onBackPress callback returning false (transparent to lower layer).
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, RemoveOverlayOnBackPress002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create overlayManager with rootNode and stage node.
     */
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    stageNode->MountToParent(rootNode);
    rootNode->MarkDirtyNode();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);

    /**
     * @tc.steps: step2. set overlayManagerOptions with enableBackPressedEvent=true and onBackPress returning false.
     */
    bool callbackInvoked = false;
    OverlayManagerInfo info;
    info.enableBackPressedEvent = true;
    info.onBackPress = [&callbackInvoked]() -> bool {
        callbackInvoked = true;
        return false;
    };
    overlayManager->SetOverlayManagerOptions(info);

    /**
     * @tc.steps: step3. add a component content node to overlayNode.
     */
    auto contentNode = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    overlayManager->AddFrameNodeToOverlay(contentNode);

    /**
     * @tc.steps: step4. call RemoveOverlay with isBackPressed=true.
     * @tc.expected: onBackPress callback is invoked and RemoveOverlay returns false (transparent to lower layer).
     */
    auto result = overlayManager->RemoveOverlay(true);
    EXPECT_TRUE(callbackInvoked);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: RemoveOverlayOnBackPress003
 * @tc.desc: Test RemoveOverlay skips onBackPress when overlayNode has no content.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, RemoveOverlayOnBackPress003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create overlayManager with rootNode and stage node.
     */
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    stageNode->MountToParent(rootNode);
    rootNode->MarkDirtyNode();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);

    /**
     * @tc.steps: step2. set overlayManagerOptions with enableBackPressedEvent=true and onBackPress callback.
     */
    bool callbackInvoked = false;
    OverlayManagerInfo info;
    info.enableBackPressedEvent = true;
    info.onBackPress = [&callbackInvoked]() -> bool {
        callbackInvoked = true;
        return true;
    };
    overlayManager->SetOverlayManagerOptions(info);

    /**
     * @tc.steps: step3. do NOT add any content to overlayNode.
     * @tc.steps: step4. call RemoveOverlay with isBackPressed=true.
     * @tc.expected: onBackPress callback is NOT invoked because overlayNode has no children.
     */
    overlayManager->RemoveOverlay(true);
    EXPECT_FALSE(callbackInvoked);
}

/**
 * @tc.name: RemoveOverlayOnBackPress004
 * @tc.desc: Test RemoveOverlay skips onBackPress when isBackPressed is false (ESC path).
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, RemoveOverlayOnBackPress004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create overlayManager with rootNode and stage node.
     */
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    stageNode->MountToParent(rootNode);
    rootNode->MarkDirtyNode();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);

    /**
     * @tc.steps: step2. set overlayManagerOptions with enableBackPressedEvent=true and onBackPress callback.
     */
    bool callbackInvoked = false;
    OverlayManagerInfo info;
    info.enableBackPressedEvent = true;
    info.onBackPress = [&callbackInvoked]() -> bool {
        callbackInvoked = true;
        return true;
    };
    overlayManager->SetOverlayManagerOptions(info);

    /**
     * @tc.steps: step3. add a component content node to overlayNode.
     */
    auto contentNode = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    overlayManager->AddFrameNodeToOverlay(contentNode);

    /**
     * @tc.steps: step4. call RemoveOverlay with isBackPressed=false.
     * @tc.expected: onBackPress callback is NOT invoked because isBackPressed is false.
     */
    overlayManager->RemoveOverlay(false);
    EXPECT_FALSE(callbackInvoked);
}

/**
 * @tc.name: RemoveOverlayOnBackPress005
 * @tc.desc: Test RemoveOverlay skips onBackPress when enableBackPressedEvent is false.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, RemoveOverlayOnBackPress005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create overlayManager with rootNode and stage node.
     */
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    stageNode->MountToParent(rootNode);
    rootNode->MarkDirtyNode();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);

    /**
     * @tc.steps: step2. set overlayManagerOptions with enableBackPressedEvent=false and onBackPress callback.
     */
    bool callbackInvoked = false;
    OverlayManagerInfo info;
    info.enableBackPressedEvent = false;
    info.onBackPress = [&callbackInvoked]() -> bool {
        callbackInvoked = true;
        return true;
    };
    overlayManager->SetOverlayManagerOptions(info);

    /**
     * @tc.steps: step3. add a component content node to overlayNode.
     */
    auto contentNode = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    overlayManager->AddFrameNodeToOverlay(contentNode);

    /**
     * @tc.steps: step4. call RemoveOverlay with isBackPressed=true.
     * @tc.expected: onBackPress callback is NOT invoked because enableBackPressedEvent is false.
     */
    overlayManager->RemoveOverlay(true);
    EXPECT_FALSE(callbackInvoked);
}

/**
 * @tc.name: RemoveOverlayOnBackPress006
 * @tc.desc: Test RemoveOverlay removes content by default when onBackPress is not registered.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayManagerTestNg, RemoveOverlayOnBackPress006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create overlayManager with rootNode and stage node.
     */
    auto rootNode = FrameNode::CreateFrameNode(V2::ROOT_ETS_TAG, 1, AceType::MakeRefPtr<RootPattern>());
    auto stageNode = FrameNode::CreateFrameNode(
        V2::STAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
    stageNode->MountToParent(rootNode);
    rootNode->MarkDirtyNode();
    auto overlayManager = AceType::MakeRefPtr<OverlayManager>(rootNode);

    /**
     * @tc.steps: step2. set overlayManagerOptions with enableBackPressedEvent=true but no onBackPress.
     */
    OverlayManagerInfo info;
    info.enableBackPressedEvent = true;
    overlayManager->SetOverlayManagerOptions(info);

    /**
     * @tc.steps: step3. add a component content node to overlayNode.
     */
    auto contentNode = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    overlayManager->AddFrameNodeToOverlay(contentNode);

    /**
     * @tc.steps: step4. call RemoveOverlay with isBackPressed=true.
     * @tc.expected: default removal detaches the content and empty overlay, preserving the stage.
     */
    ASSERT_NE(overlayManager->overlayNode_, nullptr);
    ASSERT_EQ(contentNode->GetParent(), overlayManager->overlayNode_);
    ASSERT_EQ(overlayManager->overlayNode_->GetParent(), rootNode);
    EXPECT_TRUE(overlayManager->RemoveOverlay(true));
    EXPECT_EQ(contentNode->GetParent(), nullptr);
    EXPECT_EQ(overlayManager->overlayNode_, nullptr);
    EXPECT_TRUE(overlayManager->frameNodeMapOnOverlay_.empty());
    EXPECT_EQ(stageNode->GetParent(), rootNode);
    EXPECT_EQ(rootNode->GetFirstChild(), stageNode);
    EXPECT_EQ(rootNode->GetLastChild(), stageNode);
}

namespace {
constexpr uint32_t OVERLAY_CATEGORY = OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_OVERLAY;
constexpr uint32_t OVERLAY_SHOW = OH_ARKUI_NATIVEMODULE_EVENT_OVERLAY_SHOW;
constexpr uint32_t OVERLAY_HIDE = OH_ARKUI_NATIVEMODULE_EVENT_OVERLAY_HIDE;
constexpr uint32_t NO_IGNORED_EVENTS = 0;

RefPtr<ContentChangeManager> GetContentChangeManager()
{
    auto pipeline = MockPipelineContext::GetCurrent();
    return pipeline ? pipeline->GetContentChangeManager() : nullptr;
}

MockUiSessionManager* GetMockUiSessionManager()
{
    return reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
}

struct CapiTestState {
    RefPtr<ContentChangeManager> manager;
    uint64_t removeId = 0;
    int32_t calls = 0;
    int32_t releases = 0;
    std::string json;
    std::vector<uint32_t> types;
    std::function<void(uint32_t)> onEvent;
};

struct CapiTestCookie {
    CapiTestState* state;
};

void CapiContentChangeTestSink(uint32_t type, uint64_t timestamp, const char* json, uint32_t size, void* cookie)
{
    auto* state = static_cast<CapiTestCookie*>(cookie)->state;
    EXPECT_GT(timestamp, 0U);
    ++state->calls;
    state->types.emplace_back(type);
    state->json.assign(json, size);
    auto onEvent = state->onEvent;
    if (onEvent) {
        onEvent(type);
    }
    if (state->removeId) {
        EXPECT_TRUE(state->manager->UnregisterCapiContentChangeEvent(state->removeId));
        state->removeId = 0;
    }
}

void ReleaseCapiContentChangeTestCookie(void* cookie)
{
    auto* record = static_cast<CapiTestCookie*>(cookie);
    ++record->state->releases;
    delete record;
}
} // namespace

// Keep reporting tests isolated from OverlayManagerTestNg's theme and container setup.
class OverlayContentChangeTestNg : public testing::Test {
public:
    static void SetUpTestCase()
    {
        MockPipelineContext::SetUp();
    }

    static void TearDownTestCase()
    {
        MockPipelineContext::TearDown();
    }
};

namespace {
void VerifyOverlayPageLifecycle(const RefPtr<FrameNode>& node, const std::function<void()>& appear,
    const std::function<void()>& disappear, std::vector<std::string>& order)
{
    constexpr int32_t SHOW_HIDE_REPORT_COUNT = 2;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    manager->currentContentChangeConfig_ = ContentChangeConfig {};
    auto ipc = GetMockUiSessionManager();
    EXPECT_CALL(*ipc, ReportContentChangeEvent(ChangeType::DIALOG, _))
        .Times(SHOW_HIDE_REPORT_COUNT).WillRepeatedly([&](ChangeType, const std::string& data) {
            auto json = JsonUtil::ParseJsonString(data);
            EXPECT_EQ(json->GetString("$type"), node->GetTag());
            order.emplace_back(json->GetBool("show") ? "ipc-show" : "ipc-hide");
        });
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        false, OVERLAY_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    state.onEvent = [&](uint32_t type) {
        order.emplace_back(type == OVERLAY_SHOW ? "capi-show" : "capi-hide");
        auto json = JsonUtil::ParseJsonString(state.json);
        EXPECT_EQ(json->GetString("category"), "overlay");
        EXPECT_EQ(json->GetString("overlayType"), "dialog");
        EXPECT_EQ(json->GetValue("targetNode")->GetString("id"), std::to_string(node->GetId()));
        EXPECT_EQ(json->GetValue("targetNode")->GetString("type"), node->GetTag());
        EXPECT_TRUE(json->GetValue("triggerNode")->IsNull());
        EXPECT_EQ(json->GetValue("subtree")->IsNull(), type == OVERLAY_HIDE);
    };
    appear();
    disappear();
    EXPECT_EQ(state.types, (std::vector<uint32_t> { OVERLAY_SHOW, OVERLAY_HIDE }));
    EXPECT_EQ(order, (std::vector<std::string> {
        "appear", "ipc-show", "capi-show", "disappear", "ipc-hide", "capi-hide" }));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
    manager->StopContentChangeReport();
    Mock::VerifyAndClearExpectations(ipc);
}
} // namespace

/**
 * @tc.name: CapiSheetLifecycleMatchesIpc
 * @tc.desc: Sheet lifecycle callbacks and event reports retain their existing IPC ordering.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiSheetLifecycleMatchesIpc, TestSize.Level1)
{
    constexpr int32_t NODE_NODE_ID = 551;
    constexpr int32_t TARGET_NODE_ID = 550;
    auto pattern = AceType::MakeRefPtr<SheetPresentationPattern>(TARGET_NODE_ID, "target", [](const std::string&) {});
    auto node = FrameNode::CreateFrameNode(V2::SHEET_PAGE_TAG, NODE_NODE_ID, pattern);
    std::vector<std::string> order;
    pattern->onAppear_ = [&] { order.emplace_back("appear"); };
    pattern->onDisappear_ = [&] { order.emplace_back("disappear"); };
    VerifyOverlayPageLifecycle(node, [&] { pattern->OnAppear(); }, [&] { pattern->OnDisappear(); }, order);
    EXPECT_FALSE(pattern->isOnAppearing_);
    EXPECT_FALSE(pattern->isOnDisappearing_);
    EXPECT_FALSE(pattern->isDismissProcess_);
    EXPECT_TRUE(pattern->isExecuteOnDisappear_);
    pattern->onAppear_ = nullptr;
    pattern->onDisappear_ = nullptr;
}

/**
 * @tc.name: CapiContentCoverLifecycleMatchesIpc
 * @tc.desc: ContentCover lifecycle callbacks and event reports retain their existing IPC ordering.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiContentCoverLifecycleMatchesIpc, TestSize.Level1)
{
    constexpr int32_t NODE_NODE_ID = 553;
    constexpr int32_t TARGET_NODE_ID = 552;
    auto pattern = AceType::MakeRefPtr<ModalPresentationPattern>(TARGET_NODE_ID, ModalTransition::NONE,
        [](const std::string&) {});
    auto node = FrameNode::CreateFrameNode(V2::MODAL_PAGE_TAG, NODE_NODE_ID, pattern);
    std::vector<std::string> order;
    pattern->onAppear_ = [&] { order.emplace_back("appear"); };
    pattern->onDisappear_ = [&] { order.emplace_back("disappear"); };
    VerifyOverlayPageLifecycle(node, [&] { pattern->OnAppear(); }, [&] { pattern->OnDisappear(); }, order);
    EXPECT_TRUE(pattern->isExecuteOnDisappear_);
    pattern->onAppear_ = nullptr;
    pattern->onDisappear_ = nullptr;
}

namespace {
constexpr int32_t MENU_TARGET_NODE_ID = 601;

class MenuReportEnvironment {
public:
    MenuReportEnvironment()
    {
        pipeline = MockPipelineContext::GetCurrent();
        oldTheme = pipeline->GetThemeManager();
        oldOverlay = pipeline->overlayManager_;
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        auto theme = AceType::MakeRefPtr<SelectTheme>();
        ON_CALL(*themeManager, GetTheme(SelectTheme::TypeId())).WillByDefault(Return(theme));
        ON_CALL(*themeManager, GetTheme(SelectTheme::TypeId(), _)).WillByDefault(Return(theme));
        pipeline->SetThemeManager(themeManager);
        constexpr int32_t ROOT_NODE_ID = 600;
        root = FrameNode::CreateFrameNode("root", ROOT_NODE_ID, AceType::MakeRefPtr<Pattern>());
        overlay = AceType::MakeRefPtr<OverlayManager>(root);
        pipeline->overlayManager_ = overlay;
        manager = GetContentChangeManager();
        manager->StopContentChangeReport();
        manager->InvalidateCapiContentChangeSubscriptions();
        manager->currentContentChangeConfig_ = ContentChangeConfig {};
    }

    ~MenuReportEnvironment()
    {
        manager->InvalidateCapiContentChangeSubscriptions();
        manager->StopContentChangeReport();
        Mock::VerifyAndClearExpectations(GetMockUiSessionManager());
        pipeline->overlayManager_ = oldOverlay;
        pipeline->SetThemeManager(oldTheme);
    }

    RefPtr<MockPipelineContext> pipeline;
    RefPtr<ThemeManager> oldTheme;
    RefPtr<OverlayManager> oldOverlay;
    RefPtr<FrameNode> root;
    RefPtr<OverlayManager> overlay;
    RefPtr<ContentChangeManager> manager;
};

class OverlayContentEnvironment : public MenuReportEnvironment {
public:
    OverlayContentEnvironment()
    {
        oldFlush = pipeline->UseFlushUITasks();
        pipeline->SetUseFlushUITasks(true);
        oldStage = pipeline->stageManager_;
        auto stage = FrameNode::CreateFrameNode(V2::STAGE_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<StagePattern>());
        stage->MountToParent(root);
        pipeline->stageManager_ = AceType::MakeRefPtr<StageManager>(stage);
        root->AttachToMainTree();
    }

    ~OverlayContentEnvironment()
    {
        Flush();
        pipeline->stageManager_ = oldStage;
        pipeline->SetUseFlushUITasks(oldFlush);
    }

    void Flush()
    {
        pipeline->taskScheduler_->FlushAfterRenderTask();
    }

    bool oldFlush = false;
    RefPtr<StageManager> oldStage;
};

void VerifyMenuContentReports(bool selectionMenu)
{
    constexpr int32_t SELECTION_MENU_REPORT_COUNT = 2;
    constexpr int32_t MENU_REPORT_COUNT = 5;
    constexpr int32_t WRAPPER_NODE_ID = 602;
    constexpr int32_t MENU_NODE_ID = 603;
    constexpr int32_t ITEM_NODE_ID = 604;
    constexpr int32_t SUB_MENU_NODE_ID = 605;
    constexpr int32_t CHILD_NODE_ID = 606;
    MenuReportEnvironment env;
    RefPtr<Pattern> wrapperPattern;
    if (selectionMenu) {
        wrapperPattern = AceType::MakeRefPtr<SelectOverlayPattern>(
            std::make_shared<SelectOverlayInfo>(), SelectOverlayMode::ALL);
    } else {
        wrapperPattern = AceType::MakeRefPtr<MenuWrapperPattern>(MENU_TARGET_NODE_ID);
    }
    auto tag = selectionMenu ? V2::SELECT_OVERLAY_ETS_TAG : V2::MENU_WRAPPER_ETS_TAG;
    auto wrapper = FrameNode::CreateFrameNode(tag, WRAPPER_NODE_ID, wrapperPattern);
    wrapper->AttachToMainTree();
    auto menu = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, MENU_NODE_ID,
        AceType::MakeRefPtr<MenuPattern>(MENU_TARGET_NODE_ID, "Button", MenuType::MENU));
    menu->MountToParent(wrapper);
    auto itemPattern = AceType::MakeRefPtr<MenuItemPattern>();
    auto item = FrameNode::CreateFrameNode(V2::MENU_ITEM_ETS_TAG, ITEM_NODE_ID, itemPattern);
    item->MountToParent(menu);
    CapiTestState state { env.manager };
    auto subscription = env.manager->RegisterCapiContentChangeEvent(
        false, OVERLAY_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(ChangeType::DIALOG, _))
        .Times(selectionMenu ? SELECTION_MENU_REPORT_COUNT : MENU_REPORT_COUNT);
    if (!selectionMenu) {
        env.overlay->ContentChangeReport(wrapper, true);
    }
    auto subMenu = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, SUB_MENU_NODE_ID,
        AceType::MakeRefPtr<MenuPattern>(ITEM_NODE_ID, V2::MENU_ITEM_ETS_TAG, MenuType::SUB_MENU));
    subMenu->MountToParent(wrapper);
    itemPattern->ShowSubMenuWithAnimation(subMenu);
    env.pipeline->taskScheduler_->FlushAfterLayoutTask();
    auto child = FrameNode::CreateFrameNode("Text", CHILD_NODE_ID, AceType::MakeRefPtr<Pattern>());
    child->MountToParent(subMenu);
    itemPattern->ShowSubMenuWithAnimation(subMenu);
    env.pipeline->taskScheduler_->FlushAfterLayoutTask();
    EXPECT_EQ(state.types, selectionMenu ? (std::vector<uint32_t> { OVERLAY_SHOW }) :
        (std::vector<uint32_t> { OVERLAY_SHOW, OVERLAY_SHOW }));
    auto json = JsonUtil::ParseJsonString(state.json);
    EXPECT_EQ(json->GetString("overlayType"), "menu");
    EXPECT_EQ(json->GetValue("targetNode")->GetString("type"), V2::MENU_ETS_TAG);
    EXPECT_EQ(json->GetValue("targetNode")->GetString("id"), std::to_string(SUB_MENU_NODE_ID));
    if (!selectionMenu) {
        env.overlay->ContentChangeReport(wrapper, false);
        env.overlay->ContentChangeReport(wrapper, false);
        EXPECT_EQ(state.types, (std::vector<uint32_t> {
            OVERLAY_SHOW, OVERLAY_SHOW, OVERLAY_HIDE, OVERLAY_HIDE }));
    }
    EXPECT_TRUE(env.manager->UnregisterCapiContentChangeEvent(subscription));
}
} // namespace

/**
 * @tc.name: CapiOverlayContentVisibility
 * @tc.desc: Generic overlay reports actual frame visibility changes, including removal after a late subscription.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiOverlayContentVisibility, TestSize.Level1)
{
    OverlayContentEnvironment env;
    auto node = FrameNode::CreateFrameNode("Text", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    env.overlay->AddFrameNodeToOverlay(node, std::nullopt);
    env.Flush();
    CapiTestState state { env.manager };
    auto subscription = env.manager->RegisterCapiContentChangeEvent(
        false, OVERLAY_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    env.overlay->HideNodeOnOverlay(node);
    env.Flush();
    EXPECT_EQ(state.types, (std::vector<uint32_t> { OVERLAY_HIDE }));
    env.overlay->HideNodeOnOverlay(node);
    env.Flush();
    EXPECT_EQ(state.types.size(), 1U);
    env.overlay->ShowNodeOnOverlay(node);
    env.Flush();
    env.overlay->ShowNodeOnOverlay(node);
    env.Flush();
    EXPECT_EQ(state.types, (std::vector<uint32_t> { OVERLAY_HIDE, OVERLAY_SHOW }));
    env.overlay->RemoveFrameNodeOnOverlay(node);
    env.Flush();
    EXPECT_EQ(state.types, (std::vector<uint32_t> { OVERLAY_HIDE, OVERLAY_SHOW, OVERLAY_HIDE }));
    auto json = JsonUtil::ParseJsonString(state.json);
    EXPECT_EQ(json->GetString("overlayType"), "overlay");
    EXPECT_EQ(json->GetValue("targetNode")->GetString("id"), std::to_string(node->GetId()));
    EXPECT_TRUE(env.manager->UnregisterCapiContentChangeEvent(subscription));
}

/**
 * @tc.name: CapiOverlayTransientAndOrderedContent
 * @tc.desc: Same-frame transient content is silent; ordered content reports one SHOW/HIDE pair.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiOverlayTransientAndOrderedContent, TestSize.Level1)
{
    OverlayContentEnvironment env;
    CapiTestState state { env.manager };
    auto subscription = env.manager->RegisterCapiContentChangeEvent(
        true, OVERLAY_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto node = FrameNode::CreateFrameNode("Text", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    env.overlay->AddFrameNodeToOverlay(node, std::nullopt);
    env.overlay->RemoveFrameNodeOnOverlay(node);
    env.Flush();
    EXPECT_TRUE(state.types.empty());
    env.overlay->AddFrameNodeWithOrder(node, 1.0);
    env.Flush();
    EXPECT_EQ(state.types, (std::vector<uint32_t> { OVERLAY_SHOW }));
    env.overlay->AddFrameNodeWithOrder(node, 2.0);
    env.Flush();
    EXPECT_EQ(state.types.size(), 1U);
    env.overlay->RemoveFrameNodeOnOverlay(node);
    env.Flush();
    EXPECT_EQ(state.types, (std::vector<uint32_t> { OVERLAY_SHOW, OVERLAY_HIDE }));
    EXPECT_TRUE(env.manager->UnregisterCapiContentChangeEvent(subscription));
}

/**
 * @tc.name: CapiSubmenuDoesNotRepeatParentShow
 * @tc.desc: Submenu display reports its own SHOW once and preserves legacy IPC updates.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiSubmenuDoesNotRepeatParentShow, TestSize.Level1)
{
    VerifyMenuContentReports(false);
}

/**
 * @tc.name: CapiSelectionSubmenuDoesNotRepeatParentShow
 * @tc.desc: Custom selection submenus retain menu classification and IPC notification coverage.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiSelectionSubmenuDoesNotRepeatParentShow, TestSize.Level1)
{
    VerifyMenuContentReports(true);
}

/**
 * @tc.name: CapiMenuPreviewDoesNotDuplicateCapiShow
 * @tc.desc: Preview notification remains IPC-only; actual display completion and ordinary wrapper updates remain.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiMenuPreviewDoesNotDuplicateCapiShow, TestSize.Level1)
{
    constexpr int32_t EXPECTED_IPC_REPORT_COUNT = 5;
    constexpr int32_t WRAPPER_NODE_ID = 607;
    constexpr int32_t MENU_NODE_ID = 608;
    constexpr int32_t PREVIEW_NODE_ID = 609;
    MenuReportEnvironment env;
    auto menuManager = AceType::MakeRefPtr<MenuManager>(env.root);
    auto wrapper = FrameNode::CreateFrameNode(V2::MENU_WRAPPER_ETS_TAG, WRAPPER_NODE_ID,
        AceType::MakeRefPtr<MenuWrapperPattern>(MENU_TARGET_NODE_ID));
    auto menu = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, MENU_NODE_ID,
        AceType::MakeRefPtr<MenuPattern>(MENU_TARGET_NODE_ID, "Button", MenuType::MENU));
    menu->GetLayoutProperty<MenuLayoutProperty>()->UpdateIsRectInTarget(true);
    menu->MountToParent(wrapper);
    auto previewPattern = AceType::MakeRefPtr<MenuPreviewPattern>();
    auto preview = FrameNode::CreateFrameNode(V2::MENU_PREVIEW_ETS_TAG, PREVIEW_NODE_ID, previewPattern);
    preview->MountToParent(wrapper);
    CapiTestState state { env.manager };
    auto subscription = env.manager->RegisterCapiContentChangeEvent(
        false, OVERLAY_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    std::vector<uint32_t> ipcTypes;
    EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(ChangeType::DIALOG, _))
        .Times(EXPECTED_IPC_REPORT_COUNT).WillRepeatedly([&](ChangeType, const std::string& data) {
            auto json = JsonUtil::ParseJsonString(data);
            ipcTypes.emplace_back(json->GetBool("show") ? OVERLAY_SHOW : OVERLAY_HIDE);
        });
    menuManager->SetPreviewFirstShow(wrapper, env.overlay);
    EXPECT_TRUE(previewPattern->isFirstShow_);
    EXPECT_TRUE(state.types.empty());
    EXPECT_EQ(ipcTypes, (std::vector<uint32_t> { OVERLAY_SHOW }));
    menuManager->OnShowMenuAnimationFinished(wrapper, env.overlay, env.pipeline->GetInstanceId());
    EXPECT_EQ(state.types, (std::vector<uint32_t> { OVERLAY_SHOW }));
    EXPECT_EQ(ipcTypes, (std::vector<uint32_t> { OVERLAY_SHOW, OVERLAY_SHOW }));
    // Every completion or HIDE notification follows the IPC call, even for the same node.
    menuManager->OnShowMenuAnimationFinished(wrapper, env.overlay, env.pipeline->GetInstanceId());
    env.overlay->ContentChangeReport(wrapper, false);
    env.overlay->ContentChangeReport(wrapper, false);
    EXPECT_EQ(state.types, (std::vector<uint32_t> {
        OVERLAY_SHOW, OVERLAY_SHOW, OVERLAY_HIDE, OVERLAY_HIDE }));
    EXPECT_EQ(ipcTypes.size(), EXPECTED_IPC_REPORT_COUNT);
    EXPECT_TRUE(env.manager->UnregisterCapiContentChangeEvent(subscription));
}

/**
 * @tc.name: CapiOverlayWithoutShowMatchesEveryIpcNotification
 * @tc.desc: Overlay HIDE does not require a preceding SHOW notification.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiOverlayWithoutShowMatchesEveryIpcNotification, TestSize.Level1)
{
    constexpr int32_t EXPECTED_IPC_REPORT_COUNT = 4;
    MenuReportEnvironment env;
    auto ipc = GetMockUiSessionManager();
    CapiTestState state { env.manager };
    auto subscription = env.manager->RegisterCapiContentChangeEvent(
        false, OVERLAY_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    int32_t nodeId = 610;
    for (auto tag : { V2::SHEET_PAGE_TAG, V2::MODAL_PAGE_TAG, V2::MENU_ETS_TAG }) {
        auto node = FrameNode::CreateFrameNode(tag, nodeId++, AceType::MakeRefPtr<Pattern>());
        std::vector<uint32_t> ipcTypes;
        std::vector<std::string> order;
        state.types.clear();
        state.onEvent = [&](uint32_t type) {
            order.emplace_back("capi");
            ASSERT_FALSE(ipcTypes.empty());
            EXPECT_EQ(type, ipcTypes.back());
        };
        EXPECT_CALL(*ipc, ReportContentChangeEvent(ChangeType::DIALOG, _))
            .Times(EXPECTED_IPC_REPORT_COUNT).WillRepeatedly([&](ChangeType, const std::string& data) {
                auto json = JsonUtil::ParseJsonString(data);
                ipcTypes.emplace_back(json->GetBool("show") ? OVERLAY_SHOW : OVERLAY_HIDE);
                order.emplace_back("ipc");
            });
        for (bool show : { false, false, true, true }) {
            env.overlay->ContentChangeReport(node, show);
        }
        EXPECT_EQ(state.types, (std::vector<uint32_t> { OVERLAY_HIDE, OVERLAY_HIDE, OVERLAY_SHOW, OVERLAY_SHOW }));
        EXPECT_EQ(state.types, ipcTypes);
        EXPECT_EQ(order,
            (std::vector<std::string> { "ipc", "capi", "ipc", "capi", "ipc", "capi", "ipc", "capi" }));
        state.onEvent = nullptr;
        Mock::VerifyAndClearExpectations(ipc);
    }
    EXPECT_TRUE(env.manager->UnregisterCapiContentChangeEvent(subscription));
}

/**
 * @tc.name: CapiMenuCommonReportResolvesTrigger
 * @tc.desc: Common menu notifications resolve the initiating node without explicit trigger metadata.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiMenuCommonReportResolvesTrigger, TestSize.Level1)
{
    constexpr int32_t BUTTON_NODE_ID = 436;
    constexpr int32_t MENU_NODE_ID = 437;
    constexpr int32_t WRAPPER_NODE_ID = 438;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        false, OVERLAY_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto button = FrameNode::CreateFrameNode(V2::BUTTON_ETS_TAG, BUTTON_NODE_ID, AceType::MakeRefPtr<Pattern>());
    auto menu = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, MENU_NODE_ID,
        AceType::MakeRefPtr<MenuPattern>(button->GetId(), button->GetTag(), MenuType::MENU));
    auto wrapper = FrameNode::CreateFrameNode(
        V2::MENU_WRAPPER_ETS_TAG, WRAPPER_NODE_ID, AceType::MakeRefPtr<Pattern>());
    wrapper->AddChild(menu);
    for (bool show : { true, false }) {
        manager->OnDialogChangeEnd(wrapper, show);
        auto json = JsonUtil::ParseJsonString(state.json);
        EXPECT_EQ(json->GetString("overlayType"), "menu");
        auto trigger = json->GetValue("triggerNode");
        EXPECT_EQ(trigger->GetString("id"), std::to_string(BUTTON_NODE_ID));
        EXPECT_EQ(trigger->GetString("type"), V2::BUTTON_ETS_TAG);
    }
    EXPECT_EQ(state.types, (std::vector<uint32_t> { OVERLAY_SHOW, OVERLAY_HIDE }));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiOverlayClassificationPreservesPriority
 * @tc.desc: Overlay classification preserves dialog, menu and popup type precedence.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiOverlayClassificationPreservesPriority, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        false, OVERLAY_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto content = FrameNode::CreateFrameNode(
        V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto pattern = AceType::MakeRefPtr<DialogPattern>(AceType::MakeRefPtr<DialogTheme>(), content);
    auto node = FrameNode::CreateFrameNode(
        V2::CUSTOM_DIALOG_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), pattern);
    pattern->dialogProperties_.type = DialogType::ACTION_SHEET;
    manager->OnDialogChangeEnd(node, false);
    EXPECT_EQ(JsonUtil::ParseJsonString(state.json)->GetString("overlayType"), "actionSheet");
    pattern->dialogProperties_.type = DialogType::COMMON;
    manager->OnDialogChangeEnd(node, false);
    EXPECT_EQ(JsonUtil::ParseJsonString(state.json)->GetString("overlayType"), "customDialog");
    auto plain = FrameNode::CreateFrameNode(V2::DIALOG_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<DialogPattern>(AceType::MakeRefPtr<DialogTheme>(), nullptr));
    manager->OnDialogChangeEnd(plain, false);
    EXPECT_EQ(JsonUtil::ParseJsonString(state.json)->GetString("overlayType"), "dialog");
    auto tagOnly = FrameNode::CreateFrameNode(
        V2::CUSTOM_DIALOG_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    const auto count = state.types.size();
    manager->OnDialogChangeEnd(tagOnly, false);
    manager->OnDialogChangeEnd(nullptr, false);
    EXPECT_EQ(state.types.size(), count);
    manager->ReportCapiOverlayEvent(tagOnly, false, "popup", content);
    auto json = JsonUtil::ParseJsonString(state.json);
    EXPECT_EQ(json->GetString("overlayType"), "popup");
    EXPECT_EQ(json->GetValue("triggerNode")->GetString("id"), std::to_string(content->GetId()));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiMenuExplicitTriggerOverridesLookup
 * @tc.desc: An explicit menu trigger takes precedence over lookup-derived metadata.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiMenuExplicitTriggerOverridesLookup, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        false, OVERLAY_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto original = FrameNode::CreateFrameNode(
        V2::BUTTON_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto explicitTrigger = FrameNode::CreateFrameNode(
        V2::BUTTON_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto menu = FrameNode::CreateFrameNode(V2::MENU_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<MenuPattern>(original->GetId(), original->GetTag(), MenuType::MENU));
    manager->ReportCapiOverlayEvent(menu, false, nullptr, explicitTrigger);
    auto json = JsonUtil::ParseJsonString(state.json);
    EXPECT_EQ(json->GetString("overlayType"), "menu");
    EXPECT_EQ(json->GetValue("triggerNode")->GetString("id"), std::to_string(explicitTrigger->GetId()));
    manager->OnDialogChangeEnd(menu, false);
    json = JsonUtil::ParseJsonString(state.json);
    EXPECT_EQ(json->GetValue("triggerNode")->GetString("id"), std::to_string(original->GetId()));
    auto empty = FrameNode::CreateFrameNode(
        V2::MENU_WRAPPER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    manager->OnDialogChangeEnd(empty, false);
    json = JsonUtil::ParseJsonString(state.json);
    EXPECT_EQ(json->GetString("overlayType"), "menu");
    EXPECT_TRUE(json->GetValue("triggerNode")->IsNull());
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiPopupHideKeepsWeakTriggerAfterLookupRemoved
 * @tc.desc: Popup HIDE resolves its weak trigger after the lookup entry is removed.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiPopupHideKeepsWeakTriggerAfterLookupRemoved, TestSize.Level1)
{
    constexpr int32_t ROOT_NODE_ID = 440;
    constexpr int32_t TRIGGER_NODE_ID = 442;
    constexpr int32_t POPUP_NODE_ID = 441;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        false, OVERLAY_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    constexpr int32_t REPEATED_HIDE_COUNT = 2;
    for (bool destroyTrigger : { false, true }) {
        auto root = FrameNode::CreateFrameNode("root", ROOT_NODE_ID, AceType::MakeRefPtr<Pattern>());
        auto overlay = AceType::MakeRefPtr<OverlayManager>(root);
        auto trigger = FrameNode::CreateFrameNode(V2::BUTTON_ETS_TAG, TRIGGER_NODE_ID, AceType::MakeRefPtr<Pattern>());
        auto pattern = AceType::MakeRefPtr<BubblePattern>(trigger->GetId(), trigger->GetTag());
        auto popup = FrameNode::CreateFrameNode(V2::POPUP_ETS_TAG, POPUP_NODE_ID, pattern);
        popup->MountToParent(root);
        auto& info = overlay->popupMap_[trigger->GetId()];
        info.popupNode = popup;
        info.target = trigger;
        info.isCurrentOnShow = false;
        BubbleHidePopupParams params;
        params.targetId = trigger->GetId();
        const auto references = trigger->RefCount();
        auto finish = InnerModifier::GetBubbleInnerModifier()->createHidePopupOnFinish(
            pattern, AceType::RawPtr(overlay), params, popup, root);
        EXPECT_EQ(trigger->RefCount(), references);
        overlay->ErasePopupInfo(params.targetId);
        WeakPtr<FrameNode> weakTrigger = trigger;
        if (destroyTrigger) {
            trigger.Reset();
            EXPECT_EQ(weakTrigger.Upgrade(), nullptr);
        }
        // No prior SHOW is needed, and repeated invocations are not filtered by the reporting layer.
        for (int32_t invocation = 0; invocation < REPEATED_HIDE_COUNT; ++invocation) {
            finish();
            auto json = JsonUtil::ParseJsonString(state.json);
            auto reportedTrigger = json->GetValue("triggerNode");
            if (destroyTrigger) {
                EXPECT_TRUE(reportedTrigger->IsNull());
            } else {
                EXPECT_EQ(reportedTrigger->GetString("id"), std::to_string(trigger->GetId()));
            }
        }
    }
    EXPECT_EQ(state.types, (std::vector<uint32_t> { OVERLAY_HIDE, OVERLAY_HIDE, OVERLAY_HIDE, OVERLAY_HIDE }));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiPopupFinishReportsBeforeLastReferenceIsReleased
 * @tc.desc: Popup completion reports HIDE before releasing the final node reference.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiPopupFinishReportsBeforeLastReferenceIsReleased, TestSize.Level1)
{
    constexpr size_t SHOW_HIDE_EVENT_COUNT = 2;
    constexpr int32_t ROOT_NODE_ID = 535;
    constexpr int32_t POPUP_NODE_ID = 537;
    constexpr int32_t TARGET_NODE_ID = 536;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        false, OVERLAY_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto root = FrameNode::CreateFrameNode("root", ROOT_NODE_ID, AceType::MakeRefPtr<Pattern>());
    auto overlay = AceType::MakeRefPtr<OverlayManager>(root);
    auto pattern = AceType::MakeRefPtr<BubblePattern>(TARGET_NODE_ID, V2::BUTTON_ETS_TAG);
    auto popup = FrameNode::CreateFrameNode(V2::POPUP_ETS_TAG, POPUP_NODE_ID, pattern);
    popup->MountToParent(root);
    overlay->ReportCapiOverlayEvent(popup, true, "popup");
    auto& info = overlay->popupMap_[TARGET_NODE_ID];
    info.popupNode = popup;
    info.isCurrentOnShow = false;
    BubbleHidePopupParams params;
    params.targetId = TARGET_NODE_ID;
    auto finish = InnerModifier::GetBubbleInnerModifier()->createHidePopupOnFinish(
        pattern, AceType::RawPtr(overlay), params, popup, root);
    WeakPtr<FrameNode> weakPopup = popup;
    popup.Reset();
    finish();
    EXPECT_EQ(weakPopup.Upgrade(), nullptr);
    EXPECT_TRUE(overlay->popupMap_.empty());
    EXPECT_TRUE(root->GetChildren().empty());
    EXPECT_EQ(state.types, (std::vector<uint32_t> { OVERLAY_SHOW, OVERLAY_HIDE }));
    auto json = JsonUtil::ParseJsonString(state.json);
    EXPECT_EQ(json->GetValue("targetNode")->GetString("id"), std::to_string(POPUP_NODE_ID));
    finish();
    EXPECT_EQ(state.types.size(), SHOW_HIDE_EVENT_COUNT);
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiPopupFinishDoesNotRetainRemovedNode
 * @tc.desc: The popup completion callback does not keep an already removed node alive.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiPopupFinishDoesNotRetainRemovedNode, TestSize.Level1)
{
    constexpr int32_t ROOT_NODE_ID = 538;
    constexpr int32_t POPUP_NODE_ID = 540;
    constexpr int32_t TARGET_NODE_ID = 539;
    auto root = FrameNode::CreateFrameNode("root", ROOT_NODE_ID, AceType::MakeRefPtr<Pattern>());
    auto overlay = AceType::MakeRefPtr<OverlayManager>(root);
    auto pattern = AceType::MakeRefPtr<BubblePattern>(TARGET_NODE_ID, V2::BUTTON_ETS_TAG);
    auto popup = FrameNode::CreateFrameNode(V2::POPUP_ETS_TAG, POPUP_NODE_ID, pattern);
    BubbleHidePopupParams params;
    params.targetId = TARGET_NODE_ID;
    const auto references = popup->RefCount();
    auto finish = InnerModifier::GetBubbleInnerModifier()->createHidePopupOnFinish(
        pattern, AceType::RawPtr(overlay), params, popup, root);
    EXPECT_EQ(popup->RefCount(), references);
    WeakPtr<FrameNode> weakPopup = popup;
    popup.Reset();
    EXPECT_EQ(weakPopup.Upgrade(), nullptr);
    finish();
    EXPECT_TRUE(overlay->popupMap_.empty());
}

namespace {
class DialogReportTaskExecutor : public MockTaskExecutor {
    DECLARE_ACE_TYPE(DialogReportTaskExecutor, MockTaskExecutor);

public:
    bool OnPostTask(Task&& task, TaskType type, uint32_t delayTime, const std::string& name,
        PriorityType priorityType = PriorityType::LOW,
        VsyncBarrierOption barrierOption = VsyncBarrierOption::NO_BARRIER) const override
    {
        if (name == "ArkUIOverlayDialogCloseEvent") {
            tasks.emplace_back(std::move(task));
            return true;
        }
        return MockTaskExecutor::OnPostTask(std::move(task), type, delayTime, name, priorityType, barrierOption);
    }

    mutable std::vector<Task> tasks;
};

class DialogCloseReportRenderContext : public RenderContext {
    DECLARE_ACE_TYPE(DialogCloseReportRenderContext, RenderContext);

public:
    void OpacityAnimation(const AnimationOption& option, double begin, double end) override
    {
        finish = option.GetOnFinishEvent();
    }

    bool HasDisappearTransition() const override
    {
        return hasTransition;
    }

    void SetTransitionOutCallback(std::function<void()>&& callback) override
    {
        finish = std::move(callback);
    }

    bool hasTransition = false;
    std::function<void()> finish;
};
} // namespace

/**
 * @tc.name: CapiDialogCloseReportsOncePerCompletion
 * @tc.desc: Common dialog closes match IPC once; dedicated close paths retain their CAPI-only HIDE.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiDialogCloseReportsOncePerCompletion, TestSize.Level1)
{
    enum class ClosePath { ANIMATION, TRANSITION, NO_TRANSITION, MASK_CONTENT, MASK_CONTENT_TRANSITION };
    auto pipeline = MockPipelineContext::GetCurrent();
    auto manager = GetContentChangeManager();
    auto ipc = GetMockUiSessionManager();
    auto previousExecutor = pipeline->GetTaskExecutor();
    auto previousTheme = pipeline->GetThemeManager();
    auto theme = AceType::MakeRefPtr<MockThemeManager>();
    EXPECT_CALL(*theme, GetTheme(_)).WillRepeatedly([](ThemeType type) { return GetTheme(type); });
    EXPECT_CALL(*theme, GetTheme(_, _)).WillRepeatedly([](ThemeType type, int32_t) { return GetTheme(type); });
    pipeline->SetThemeManager(theme);
    MockContainer::SetUp();
    MockContainer::Current()->pipelineContext_ = pipeline;
    auto executor = AceType::MakeRefPtr<DialogReportTaskExecutor>();
    MockContainer::Current()->taskExecutor_ = executor;
    pipeline->SetTaskExecutor(executor);
    for (auto path : { ClosePath::ANIMATION, ClosePath::TRANSITION,
        ClosePath::NO_TRANSITION, ClosePath::MASK_CONTENT, ClosePath::MASK_CONTENT_TRANSITION }) {
        SCOPED_TRACE(static_cast<int>(path));
        manager->InvalidateCapiContentChangeSubscriptions();
        manager->currentContentChangeConfig_ = ContentChangeConfig {};
        CapiTestState state { manager };
        auto id = manager->RegisterCapiContentChangeEvent(false, OVERLAY_CATEGORY, NO_IGNORED_EVENTS,
            new CapiTestCookie { &state }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
        auto root = FrameNode::CreateFrameNode(
            V2::ROOT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        auto node = FrameNode::CreateFrameNode(
            V2::DIALOG_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
            AceType::MakeRefPtr<DialogPattern>(AceType::MakeRefPtr<DialogTheme>(), nullptr));
        auto render = AceType::MakeRefPtr<DialogCloseReportRenderContext>();
        render->hasTransition = path == ClosePath::TRANSITION;
        node->renderContext_ = render;
        node->MountToParent(root);
        auto overlay = AceType::MakeRefPtr<OverlayManager>(root);
        auto dialogManager = AceType::MakeRefPtr<DialogInnerManager>(root);
        std::vector<RefPtr<DialogCloseReportRenderContext>> childRenders;
        if (path == ClosePath::MASK_CONTENT_TRANSITION) {
            for (const auto* tag : { "DialogContent", "DialogMask" }) {
                auto child = FrameNode::CreateFrameNode(
                    tag, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
                auto childRender = AceType::MakeRefPtr<DialogCloseReportRenderContext>();
                childRender->hasTransition = true;
                child->renderContext_ = childRender;
                child->MountToParent(node);
                childRenders.emplace_back(childRender);
            }
        }
        const bool commonReport = path == ClosePath::ANIMATION || path == ClosePath::TRANSITION;
        EXPECT_CALL(*ipc, ReportContentChangeEvent(ChangeType::DIALOG, _)).Times(commonReport ? 1 : 0);
        if (path == ClosePath::ANIMATION) {
            dialogManager->CloseDialogAnimation(overlay, node);
        } else if (path == ClosePath::MASK_CONTENT || path == ClosePath::MASK_CONTENT_TRANSITION) {
            dialogManager->CloseMaskAndContentMatchTransition(overlay, node);
        } else {
            dialogManager->CloseDialogMatchTransition(overlay, node);
        }
        if (render->finish) {
            render->finish();
        }
        for (const auto& childRender : childRenders) {
            ASSERT_TRUE(childRender->finish);
            childRender->finish();
        }
        EXPECT_EQ(state.types.size(), commonReport ? 1U : 0U);
        auto pending = std::move(executor->tasks);
        executor->tasks.clear();
        EXPECT_FALSE(pending.empty());
        for (auto& task : pending) {
            task();
        }
        EXPECT_EQ(node->GetParent(), nullptr);
        EXPECT_EQ(state.types, (std::vector<uint32_t> { OVERLAY_HIDE }));
        EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
        manager->StopContentChangeReport();
        Mock::VerifyAndClearExpectations(ipc);
    }
    pipeline->SetTaskExecutor(previousExecutor);
    pipeline->SetThemeManager(previousTheme);
    MockContainer::TearDown();
}

/**
 * @tc.name: CapiSubMenuLateSubscriptionAndDistinctTargets
 * @tc.desc: A submenu shown before subscription can close; different submenu instances remain independent.
 * @tc.type: FUNC
 */
HWTEST_F(OverlayContentChangeTestNg, CapiSubMenuLateSubscriptionAndDistinctTargets, TestSize.Level1)
{
    MenuReportEnvironment env;
    auto a = FrameNode::CreateFrameNode("Menu", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    auto b = FrameNode::CreateFrameNode("Menu", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    env.overlay->ReportSubMenuEvent(a, true);
    CapiTestState state { env.manager };
    auto id = env.manager->RegisterCapiContentChangeEvent(false, OVERLAY_CATEGORY, 0,
        new CapiTestCookie { &state }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(_, _)).Times(0);
    env.overlay->ReportSubMenuEvent(a, false);
    EXPECT_EQ(state.types, (std::vector<uint32_t> { OVERLAY_HIDE }));
    EXPECT_EQ(JsonUtil::ParseJsonString(state.json)->GetValue("targetNode")->GetString("id"),
        std::to_string(a->GetId()));
    env.overlay->ReportSubMenuEvent(b, true);
    env.overlay->ReportSubMenuEvent(b, true);
    env.overlay->ReportSubMenuEvent(b, false);
    env.overlay->ReportSubMenuEvent(b, false);
    EXPECT_EQ(state.types, (std::vector<uint32_t> { OVERLAY_HIDE, OVERLAY_SHOW, OVERLAY_HIDE }));
    EXPECT_EQ(JsonUtil::ParseJsonString(state.json)->GetValue("targetNode")->GetString("id"),
        std::to_string(b->GetId()));
    env.manager->UnregisterCapiContentChangeEvent(id);
}

} // namespace OHOS::Ace::NG
