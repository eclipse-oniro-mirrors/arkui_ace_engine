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
#define protected public
#define private public

#include "test/mock/frameworks/base/subwindow/mock_subwindow.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/subwindow/subwindow_manager.h"
#include "core/common/ace_engine.h"
#include "core/components_ng/base/view_abstract_model_ng.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/pattern/bubble/bubble_pattern.h"
#include "core/components_ng/pattern/menu/menu_manager.h"
#include "core/components_ng/pattern/menu/menu_theme.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/pattern.h"
#include "ui/event/touch_event.h"

#undef private
#undef protected
using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
ViewAbstractModelNG viewAbstractModelNG;
const std::string TEST_TEXT = "testAccessibilityText";
const std::string TEST_DESCRIPTION = "testAccessibilityDescription";
const std::string TEST_IMPORTANCE = "yes";
const std::string TEST_ROLE = "testRole";
} // namespace

class ViewAbstractModelGapTestNg : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void TearDown() override
    {
        ViewStackProcessor::GetInstance()->ClearStack();
        SubwindowManager::GetInstance()->subwindowMap_.clear();
        SubwindowManager::GetInstance()->instanceSubwindowMap_.clear();
    }
};

void ViewAbstractModelGapTestNg::SetUpTestSuite()
{
    MockPipelineContext::SetUp();
    MockContainer::SetUp();
    MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
    MockContainer::Current()->pipelineContext_ = MockPipelineContext::GetCurrentContext();
    MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;
}

void ViewAbstractModelGapTestNg::TearDownTestSuite()
{
    auto containerId = Container::CurrentId();
    if (AceEngine::Get().GetContainer(containerId) == MockContainer::Current()) {
        AceEngine::Get().RemoveContainer(containerId);
    }
    MockPipelineContext::TearDown();
    MockContainer::TearDown();
    ViewStackProcessor::GetInstance()->ClearStack();
}

/**
 * @tc.name: BindMenuGestureWithParams001
 * @tc.desc: Test BindMenuGesture with non-empty params creates showMenu callback with items.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelGapTestNg, BindMenuGestureWithParams001, TestSize.Level1)
{
    const RefPtr<FrameNode> mainNode = FrameNode::CreateFrameNode("main", 1, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(mainNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(mainNode);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<MenuTheme>()));
    SubwindowManager::GetInstance()->RemoveSubwindow(Container::CurrentId(), SubwindowType::TYPE_MENU);

    MenuParam menuParam;
    menuParam.anchorPosition = OffsetF(10, 20);
    std::vector<NG::OptionParam> params;
    params.push_back(OptionParam());
    std::function<void()> buildFunc = []() {};
    viewAbstractModelNG.BindMenuGesture(std::move(params), std::move(buildFunc), menuParam);
    auto gestureHub = mainNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    ASSERT_NE(gestureHub->showMenu_, nullptr);
    ASSERT_NE(gestureHub->bindMenuTouch_, nullptr);

    GestureEvent info;
    (*gestureHub->showMenu_)(info);
    EXPECT_NE(mainNode, nullptr);
}

/**
 * @tc.name: BindMenuGestureWithBuildFunc001
 * @tc.desc: Test BindMenuGesture with buildFunc only, with both empty and with null target node.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelGapTestNg, BindMenuGestureWithBuildFunc001, TestSize.Level1)
{
    const RefPtr<FrameNode> mainNode = FrameNode::CreateFrameNode("main", 2, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(mainNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(mainNode);
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(AceType::MakeRefPtr<MenuTheme>()));
    SubwindowManager::GetInstance()->RemoveSubwindow(Container::CurrentId(), SubwindowType::TYPE_MENU);

    MenuParam menuParam;
    std::vector<NG::OptionParam> params;
    std::function<void()> buildFunc = []() {};
    viewAbstractModelNG.BindMenuGesture(std::move(params), std::move(buildFunc), menuParam);
    auto gestureHub = mainNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    ASSERT_NE(gestureHub->showMenu_, nullptr);

    GestureEvent info;
    (*gestureHub->showMenu_)(info);

    const RefPtr<FrameNode> secondNode = FrameNode::CreateFrameNode("second", 3, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(secondNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(secondNode);
    std::vector<NG::OptionParam> emptyParams;
    std::function<void()> emptyBuildFunc = nullptr;
    viewAbstractModelNG.BindMenuGesture(std::move(emptyParams), std::move(emptyBuildFunc), menuParam);
    auto secondHub = secondNode->GetOrCreateGestureEventHub();
    ASSERT_NE(secondHub, nullptr);
    EXPECT_EQ(secondHub->showMenu_, nullptr);

    ViewStackProcessor::GetInstance()->ClearStack();
    std::vector<NG::OptionParam> nullTargetParams;
    nullTargetParams.push_back(OptionParam());
    viewAbstractModelNG.BindMenuGesture(std::move(nullTargetParams), nullptr, menuParam);
    EXPECT_NE(secondNode, nullptr);
}

/**
 * @tc.name: BindMenuTouchDown001
 * @tc.desc: Test BindMenuTouch callback with DOWN touch type and different menu status.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelGapTestNg, BindMenuTouchDown001, TestSize.Level1)
{
    const RefPtr<FrameNode> targetNode = FrameNode::CreateFrameNode("menuTarget", 100,
        AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(targetNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(targetNode);
    auto containerId = Container::CurrentId();
    auto mockSubwindow = AceType::MakeRefPtr<MockSubwindow>();
    EXPECT_CALL(*mockSubwindow, GetChildContainerId()).WillRepeatedly(Return(containerId));
    EXPECT_CALL(*mockSubwindow, GetShown()).WillRepeatedly(Return(true));
    SubwindowManager::GetInstance()->AddSubwindow(containerId, SubwindowType::TYPE_MENU, mockSubwindow);
    AceEngine::Get().AddContainer(containerId, MockContainer::container_);
    auto pipeline = MockPipelineContext::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    auto overlayManager = pipeline->GetOverlayManager();
    ASSERT_NE(overlayManager, nullptr);
    overlayManager->CheckMenuManager();
    auto menuManager = AceType::DynamicCast<MenuManager>(overlayManager->menuManager_);
    ASSERT_NE(menuManager, nullptr);
    auto targetId = targetNode->GetId();
    auto menu = FrameNode::CreateFrameNode("menuWrapper", targetId,
        AceType::MakeRefPtr<MenuWrapperPattern>(targetId), false);
    menuManager->menuMap_[targetId] = menu;
    auto wrapperPattern = menu->GetPattern<MenuWrapperPattern>();
    ASSERT_NE(wrapperPattern, nullptr);

    auto gestureHub = targetNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureHub, nullptr);
    viewAbstractModelNG.BindMenuTouch(AceType::RawPtr(targetNode), gestureHub);
    auto touchEvent = gestureHub->bindMenuTouch_;
    ASSERT_NE(touchEvent, nullptr);
    auto touchCallback = touchEvent->callback_;

    TouchEventInfo downInfo("touch");
    TouchLocationInfo downLocation(0);
    downLocation.SetTouchType(TouchType::DOWN);
    downInfo.AddTouchLocationInfo(std::move(downLocation));

    wrapperPattern->ForceSetMenuStatus(MenuStatus::INIT);
    touchCallback(downInfo);
    EXPECT_FALSE(overlayManager->CheckSkipMenuShow(targetId));

    wrapperPattern->ForceSetMenuStatus(MenuStatus::SHOW);
    touchCallback(downInfo);
    EXPECT_TRUE(overlayManager->CheckSkipMenuShow(targetId));

    overlayManager->ResumeMenuShow(targetId);
    wrapperPattern->ForceSetMenuStatus(MenuStatus::ON_HIDE_ANIMATION);
    touchCallback(downInfo);
    EXPECT_TRUE(overlayManager->CheckSkipMenuShow(targetId));
}

struct MenuTouchTestEnv {
    RefPtr<FrameNode> targetNode;
    RefPtr<OverlayManager> overlayManager;
    RefPtr<MenuManager> menuManager;
    RefPtr<MenuWrapperPattern> wrapperPattern;
    RefPtr<GestureEventHub> gestureHub;
    int32_t targetId = -1;
};

MenuTouchTestEnv SetUpMenuTouchEnv(int32_t nodeId)
{
    MenuTouchTestEnv env;
    env.targetNode =
        FrameNode::CreateFrameNode("menuTarget", nodeId, AceType::MakeRefPtr<Pattern>(), true);
    ViewStackProcessor::GetInstance()->Push(env.targetNode);
    auto containerId = Container::CurrentId();
    auto mockSubwindow = AceType::MakeRefPtr<MockSubwindow>();
    EXPECT_CALL(*mockSubwindow, GetChildContainerId()).WillRepeatedly(Return(containerId));
    EXPECT_CALL(*mockSubwindow, GetShown()).WillRepeatedly(Return(true));
    SubwindowManager::GetInstance()->AddSubwindow(containerId, SubwindowType::TYPE_MENU, mockSubwindow);
    AceEngine::Get().AddContainer(containerId, MockContainer::container_);
    auto pipeline = MockPipelineContext::GetCurrentContext();
    if (!pipeline) {
        return env;
    }
    env.overlayManager = pipeline->GetOverlayManager();
    env.overlayManager->CheckMenuManager();
    env.menuManager = AceType::DynamicCast<MenuManager>(env.overlayManager->menuManager_);
    env.targetId = env.targetNode->GetId();
    auto menu = FrameNode::CreateFrameNode("menuWrapper", env.targetId,
        AceType::MakeRefPtr<MenuWrapperPattern>(env.targetId), false);
    env.menuManager->menuMap_[env.targetId] = menu;
    env.wrapperPattern = menu->GetPattern<MenuWrapperPattern>();
    env.wrapperPattern->ForceSetMenuStatus(MenuStatus::SHOW);
    env.gestureHub = env.targetNode->GetOrCreateGestureEventHub();
    viewAbstractModelNG.BindMenuTouch(AceType::RawPtr(env.targetNode), env.gestureHub);
    return env;
}

/**
 * @tc.name: BindMenuTouchUpAndCancel001
 * @tc.desc: Test BindMenuTouch callback with UP and CANCEL touch types.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelGapTestNg, BindMenuTouchUpAndCancel001, TestSize.Level1)
{
    auto env = SetUpMenuTouchEnv(101);
    ASSERT_NE(env.overlayManager, nullptr);
    ASSERT_NE(env.gestureHub, nullptr);
    auto touchCallback = env.gestureHub->bindMenuTouch_->callback_;
    ASSERT_TRUE(touchCallback);

    TouchEventInfo downInfo("touch");
    TouchLocationInfo downLocation(0);
    downLocation.SetTouchType(TouchType::DOWN);
    downInfo.AddTouchLocationInfo(std::move(downLocation));
    touchCallback(downInfo);
    EXPECT_TRUE(env.overlayManager->CheckSkipMenuShow(env.targetId));

    TouchEventInfo upInfo("touch");
    TouchLocationInfo upLocation(0);
    upLocation.SetTouchType(TouchType::UP);
    upInfo.AddTouchLocationInfo(std::move(upLocation));
    touchCallback(upInfo);
    EXPECT_FALSE(env.overlayManager->CheckSkipMenuShow(env.targetId));

    env.wrapperPattern->ForceSetMenuStatus(MenuStatus::SHOW);
    touchCallback(downInfo);
    EXPECT_TRUE(env.overlayManager->CheckSkipMenuShow(env.targetId));
    TouchEventInfo cancelInfo("touch");
    TouchLocationInfo cancelLocation(0);
    cancelLocation.SetTouchType(TouchType::CANCEL);
    cancelInfo.AddTouchLocationInfo(std::move(cancelLocation));
    touchCallback(cancelInfo);
    EXPECT_FALSE(env.overlayManager->CheckSkipMenuShow(env.targetId));
}

/**
 * @tc.name: BindMenuTouchUpAndCancel002
 * @tc.desc: Test BindMenuTouch callback with menu or subwindow missing.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelGapTestNg, BindMenuTouchUpAndCancel002, TestSize.Level1)
{
    auto env = SetUpMenuTouchEnv(103);
    ASSERT_NE(env.menuManager, nullptr);
    ASSERT_NE(env.gestureHub, nullptr);
    auto touchCallback = env.gestureHub->bindMenuTouch_->callback_;
    ASSERT_TRUE(touchCallback);

    TouchEventInfo downInfo("touch");
    TouchLocationInfo downLocation(0);
    downLocation.SetTouchType(TouchType::DOWN);
    downInfo.AddTouchLocationInfo(std::move(downLocation));
    TouchEventInfo upInfo("touch");
    TouchLocationInfo upLocation(0);
    upLocation.SetTouchType(TouchType::UP);
    upInfo.AddTouchLocationInfo(std::move(upLocation));

    env.menuManager->menuMap_.clear();
    env.wrapperPattern->ForceSetMenuStatus(MenuStatus::INIT);
    touchCallback(downInfo);
    EXPECT_FALSE(env.overlayManager->CheckSkipMenuShow(env.targetId));

    SubwindowManager::GetInstance()->subwindowMap_.clear();
    touchCallback(downInfo);
    touchCallback(upInfo);
    EXPECT_NE(env.targetNode, nullptr);
}

/**
 * @tc.name: CheckSkipMenuShowFullPath001
 * @tc.desc: Test CheckSkipMenuShow returns overlay manager skip state when subwindow exists.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelGapTestNg, CheckSkipMenuShowFullPath001, TestSize.Level1)
{
    const RefPtr<FrameNode> targetNode = FrameNode::CreateFrameNode("menuTarget", 102,
        AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(targetNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(targetNode);
    auto containerId = Container::CurrentId();
    auto mockSubwindow = AceType::MakeRefPtr<MockSubwindow>();
    EXPECT_CALL(*mockSubwindow, GetChildContainerId()).WillRepeatedly(Return(containerId));
    EXPECT_CALL(*mockSubwindow, GetShown()).WillRepeatedly(Return(true));
    SubwindowManager::GetInstance()->AddSubwindow(containerId, SubwindowType::TYPE_MENU, mockSubwindow);
    AceEngine::Get().AddContainer(containerId, MockContainer::container_);
    auto pipeline = MockPipelineContext::GetCurrentContext();
    ASSERT_NE(pipeline, nullptr);
    auto overlayManager = pipeline->GetOverlayManager();
    ASSERT_NE(overlayManager, nullptr);
    auto targetId = targetNode->GetId();

    EXPECT_FALSE(ViewAbstractModelNG::CheckSkipMenuShow(targetNode));
    overlayManager->SkipMenuShow(targetId);
    EXPECT_TRUE(ViewAbstractModelNG::CheckSkipMenuShow(targetNode));
    ViewAbstractModelNG::CheckSkipMenuShow(nullptr);
    EXPECT_NE(targetNode, nullptr);
}

/**
 * @tc.name: SetOnKeyPreIme001
 * @tc.desc: Test SetOnKeyPreIme and DisableOnKeyPreIme register and clear onKeyPreIme callback.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelGapTestNg, SetOnKeyPreIme001, TestSize.Level1)
{
    const RefPtr<FrameNode> mainNode = FrameNode::CreateFrameNode("main", 4, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(mainNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(mainNode);
    auto focusHub = mainNode->GetOrCreateFocusHub();
    ASSERT_NE(focusHub, nullptr);

    OnKeyConsumeFunc onKeyCallback = [](KeyEventInfo& event) { return true; };
    viewAbstractModelNG.SetOnKeyPreIme(std::move(onKeyCallback));
    EXPECT_NE(focusHub->GetOnKeyPreIme(), nullptr);
    viewAbstractModelNG.DisableOnKeyPreIme();
    EXPECT_EQ(focusHub->GetOnKeyPreIme(), nullptr);

    OnKeyConsumeFunc onKeyCallbackSecond = [](KeyEventInfo& event) { return false; };
    ViewAbstractModelNG::SetOnKeyPreIme(AceType::RawPtr(mainNode), std::move(onKeyCallbackSecond));
    EXPECT_NE(focusHub->GetOnKeyPreIme(), nullptr);
    ViewAbstractModelNG::DisableOnKeyPreIme(AceType::RawPtr(mainNode));
    EXPECT_EQ(focusHub->GetOnKeyPreIme(), nullptr);

    ViewStackProcessor::GetInstance()->ClearStack();
    EXPECT_NE(mainNode, nullptr);
}

/**
 * @tc.name: SetPaddings001
 * @tc.desc: Test SetPaddings with mixed optional edges.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelGapTestNg, SetPaddings001, TestSize.Level1)
{
    const RefPtr<FrameNode> mainNode = FrameNode::CreateFrameNode("main", 5, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(mainNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(mainNode);

    CalcDimension topDim = CalcDimension(std::string("calc(10% - 0vp)"), DimensionUnit::CALC);
    CalcDimension bottomDim = CalcDimension(-5.0, DimensionUnit::VP);
    CalcDimension leftDim = CalcDimension(3.0, DimensionUnit::FP);
    viewAbstractModelNG.SetPaddings(topDim, bottomDim, leftDim, std::nullopt);
    auto layoutProperty = mainNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    const auto& padding = layoutProperty->GetPaddingProperty();
    ASSERT_NE(padding, nullptr);
    EXPECT_TRUE(padding->top.has_value());
    EXPECT_TRUE(padding->bottom.has_value());
    EXPECT_TRUE(padding->left.has_value());
    EXPECT_FALSE(padding->right.has_value());

    NG::PaddingProperty paddings;
    paddings.top = NG::CalcLength(5.0_vp);
    paddings.right = NG::CalcLength(6.0_vp);
    viewAbstractModelNG.SetPaddings(paddings);
    const auto& paddingUpdated = layoutProperty->GetPaddingProperty();
    ASSERT_NE(paddingUpdated, nullptr);
    EXPECT_TRUE(paddingUpdated->top.has_value());
    EXPECT_TRUE(paddingUpdated->right.has_value());
}

/**
 * @tc.name: SetSafeAreaPaddings001
 * @tc.desc: Test SetSafeAreaPaddings with mixed optional edges.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelGapTestNg, SetSafeAreaPaddings001, TestSize.Level1)
{
    const RefPtr<FrameNode> mainNode = FrameNode::CreateFrameNode("main", 6, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(mainNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(mainNode);

    CalcDimension topDim = CalcDimension(std::string("calc(10% - 0vp)"), DimensionUnit::CALC);
    CalcDimension bottomDim = CalcDimension(20.0, DimensionUnit::VP);
    viewAbstractModelNG.SetSafeAreaPaddings(topDim, bottomDim, std::nullopt, std::nullopt);
    auto layoutProperty = mainNode->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    const auto& safeAreaPadding = layoutProperty->GetSafeAreaPaddingProperty();
    ASSERT_NE(safeAreaPadding, nullptr);
    EXPECT_TRUE(safeAreaPadding->top.has_value());
    EXPECT_TRUE(safeAreaPadding->bottom.has_value());
    EXPECT_FALSE(safeAreaPadding->left.has_value());
    EXPECT_FALSE(safeAreaPadding->right.has_value());

    NG::PaddingProperty paddings;
    paddings.left = NG::CalcLength(7.0_vp);
    viewAbstractModelNG.SetSafeAreaPaddings(paddings);
    const auto& safeAreaPaddingUpdated = layoutProperty->GetSafeAreaPaddingProperty();
    ASSERT_NE(safeAreaPaddingUpdated, nullptr);
    EXPECT_TRUE(safeAreaPaddingUpdated->left.has_value());
}

/**
 * @tc.name: GetAccessibilityProperties001
 * @tc.desc: Test GetAccessibilityText/Group/Importance/Description with value and null node.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelGapTestNg, GetAccessibilityProperties001, TestSize.Level1)
{
    EXPECT_EQ(ViewAbstractModelNG::GetAccessibilityText(nullptr), "");
    EXPECT_EQ(ViewAbstractModelNG::GetAccessibilityDescription(nullptr), "");
    EXPECT_EQ(ViewAbstractModelNG::GetAccessibilityImportance(nullptr), "");
    EXPECT_FALSE(ViewAbstractModelNG::GetAccessibilityGroup(nullptr));

    const RefPtr<FrameNode> mainNode = FrameNode::CreateFrameNode("main", 7, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(mainNode, nullptr);
    EXPECT_EQ(ViewAbstractModelNG::GetAccessibilityText(AceType::RawPtr(mainNode)), "");
    EXPECT_EQ(ViewAbstractModelNG::GetAccessibilityImportance(AceType::RawPtr(mainNode)), "auto");
    EXPECT_FALSE(ViewAbstractModelNG::GetAccessibilityGroup(AceType::RawPtr(mainNode)));

    ViewAbstractModelNG::SetAccessibilityText(AceType::RawPtr(mainNode), TEST_TEXT);
    ViewAbstractModelNG::SetAccessibilityDescription(AceType::RawPtr(mainNode), TEST_DESCRIPTION);
    ViewAbstractModelNG::SetAccessibilityImportance(AceType::RawPtr(mainNode), TEST_IMPORTANCE);
    ViewAbstractModelNG::SetAccessibilityGroup(AceType::RawPtr(mainNode), true);
    EXPECT_EQ(ViewAbstractModelNG::GetAccessibilityText(AceType::RawPtr(mainNode)), TEST_TEXT);
    EXPECT_EQ(ViewAbstractModelNG::GetAccessibilityDescription(AceType::RawPtr(mainNode)), TEST_DESCRIPTION);
    EXPECT_EQ(ViewAbstractModelNG::GetAccessibilityImportance(AceType::RawPtr(mainNode)), TEST_IMPORTANCE);
    EXPECT_TRUE(ViewAbstractModelNG::GetAccessibilityGroup(AceType::RawPtr(mainNode)));
}

/**
 * @tc.name: CreateWithColorResourceObj001
 * @tc.desc: Test CreateWithColorResourceObj with null args, resource obj and res cache.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelGapTestNg, CreateWithColorResourceObj001, TestSize.Level1)
{
    auto resObj = AceType::MakeRefPtr<ResourceObject>();
    ASSERT_NE(resObj, nullptr);
    resObj->SetIsResource(false);
    ViewAbstractModelNG::CreateWithColorResourceObj(nullptr, resObj, PopupType::POPUPTYPE_TEXTCOLOR);

    const RefPtr<FrameNode> plainNode = FrameNode::CreateFrameNode("plain", 8, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(plainNode, nullptr);
    ViewAbstractModelNG::CreateWithColorResourceObj(plainNode, resObj, PopupType::POPUPTYPE_TEXTCOLOR);

    const RefPtr<FrameNode> bubbleNode = FrameNode::CreateFrameNode(V2::POPUP_ETS_TAG, 9,
        AceType::MakeRefPtr<BubblePattern>(), true);
    ASSERT_NE(bubbleNode, nullptr);
    auto pattern = bubbleNode->GetPattern<BubblePattern>();
    ASSERT_NE(pattern, nullptr);
    ViewAbstractModelNG::CreateWithColorResourceObj(bubbleNode, nullptr, PopupType::POPUPTYPE_TEXTCOLOR);
    EXPECT_EQ(pattern->GetResCacheMapByKey("popupTextColor"), "");

    ViewAbstractModelNG::CreateWithColorResourceObj(bubbleNode, resObj, PopupType::POPUPTYPE_TEXTCOLOR);
    ASSERT_NE(pattern->resourceMgr_, nullptr);
    pattern->OnColorModeChange(1);
    EXPECT_NE(pattern->GetResCacheMapByKey("popupTextColor"), "");

    pattern->AddResCache("popupTextColor", "#FF0000FF");
    pattern->OnColorModeChange(0);
    EXPECT_EQ(pattern->GetResCacheMapByKey("popupTextColor"), "#FF0000FF");
}

/**
 * @tc.name: CreateWithBoolResourceObj001
 * @tc.desc: Test CreateWithBoolResourceObj with null args, plain pattern and bubble pattern.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelGapTestNg, CreateWithBoolResourceObj001, TestSize.Level1)
{
    auto resObj = AceType::MakeRefPtr<ResourceObject>("", "", -1);
    ASSERT_NE(resObj, nullptr);
    ViewAbstractModelNG::CreateWithBoolResourceObj(nullptr, resObj);

    const RefPtr<FrameNode> plainNode = FrameNode::CreateFrameNode("plain", 10, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(plainNode, nullptr);
    ViewAbstractModelNG::CreateWithBoolResourceObj(plainNode, resObj);

    const RefPtr<FrameNode> bubbleNode = FrameNode::CreateFrameNode(V2::POPUP_ETS_TAG, 11,
        AceType::MakeRefPtr<BubblePattern>(), true);
    ASSERT_NE(bubbleNode, nullptr);
    auto pattern = bubbleNode->GetPattern<BubblePattern>();
    ASSERT_NE(pattern, nullptr);
    ViewAbstractModelNG::CreateWithBoolResourceObj(bubbleNode, resObj);
    ASSERT_NE(pattern->resourceMgr_, nullptr);
    // ParseResBool leaves its output untouched for an unknown resource type, so
    // seed the cache to keep the reload branch deterministic.
    pattern->AddResCache("popupMask", "false");
    pattern->OnColorModeChange(1);
    EXPECT_EQ(pattern->GetResCacheMapByKey("popupMask"), "false");
}

/**
 * @tc.name: ParseOptionsDimensionAndUpdateColor001
 * @tc.desc: Test ParseOptionsDimension, UpdateColor and popup type string lookups.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelGapTestNg, ParseOptionsDimensionAndUpdateColor001, TestSize.Level1)
{
    std::vector<ResourceObjectParams> params = { { "", ResourceObjectParamType::NONE } };
    auto resObj = AceType::MakeRefPtr<ResourceObject>(0, 10007, params, "", "", 0);
    ASSERT_NE(resObj, nullptr);
    CalcDimension dimension(1.0, DimensionUnit::VP);
    ViewAbstractModelNG::ParseOptionsDimension(nullptr, resObj, POPUP_OPTIONTYPE_WIDTH, dimension);

    const RefPtr<FrameNode> plainNode = FrameNode::CreateFrameNode("plain", 12, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(plainNode, nullptr);
    ViewAbstractModelNG::ParseOptionsDimension(plainNode, resObj, POPUP_OPTIONTYPE_WIDTH, dimension);
    ViewAbstractModelNG::UpdateColor(plainNode, PopupType::POPUPTYPE_TEXTCOLOR, Color::BLUE);

    const RefPtr<FrameNode> bubbleNode = FrameNode::CreateFrameNode(V2::POPUP_ETS_TAG, 13,
        AceType::MakeRefPtr<BubblePattern>(), true);
    ASSERT_NE(bubbleNode, nullptr);
    auto pattern = bubbleNode->GetPattern<BubblePattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->SetPopupParam(AceType::MakeRefPtr<PopupParam>());
    ViewAbstractModelNG::ParseOptionsDimension(bubbleNode, resObj, POPUP_OPTIONTYPE_WIDTH, dimension);
    ViewAbstractModelNG::UpdateColor(bubbleNode, PopupType::POPUPTYPE_POPUPCOLOR, Color::BLUE);
    ViewAbstractModelNG::UpdateColor(bubbleNode, PopupType::POPUPTYPE_MASKCOLOR, Color::RED);

    auto unknownType = static_cast<PopupType>(100);
    EXPECT_EQ(ViewAbstractModelNG::PopupTypeStr(unknownType), "");
    EXPECT_EQ(ViewAbstractModelNG::PopupTypeStr(PopupType::POPUPTYPE_TEXTCOLOR), "TextColor");
    EXPECT_EQ(ViewAbstractModelNG::PopupOptionTypeStr(POPUP_OPTIONTYPE_WIDTH), "width");
    EXPECT_EQ(ViewAbstractModelNG::PopupOptionTypeStr(POPUP_OPTIONTYPE_BORDERWIDTH), "borderWidth");
    auto unknownOptionType = static_cast<PopupOptionsType>(100);
    EXPECT_EQ(ViewAbstractModelNG::PopupOptionTypeStr(unknownOptionType), "");
    EXPECT_NE(pattern, nullptr);
}

/**
 * @tc.name: SetAccessibilityVirtualNode001
 * @tc.desc: Test SetAccessibilityVirtualNode with buildFunc building node and empty build.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelGapTestNg, SetAccessibilityVirtualNode001, TestSize.Level1)
{
    const RefPtr<FrameNode> mainNode = FrameNode::CreateFrameNode("main", 14, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(mainNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(mainNode);
    auto accessibilityProperty = mainNode->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);

    std::function<void()> emptyBuildFunc = []() {};
    viewAbstractModelNG.SetAccessibilityVirtualNode(std::move(emptyBuildFunc));
    EXPECT_FALSE(mainNode->hasAccessibilityVirtualNode_);
    EXPECT_EQ(accessibilityProperty->GetAccessibilityVirtualNode(), nullptr);

    std::function<void()> buildFunc = []() {
        auto* stack = ViewStackProcessor::GetInstance();
        auto nodeId = stack->ClaimNodeId();
        auto frameNode =
            FrameNode::GetOrCreateFrameNode("virtualNode", nodeId, []() { return AceType::MakeRefPtr<Pattern>(); });
        stack->Push(frameNode);
    };
    viewAbstractModelNG.SetAccessibilityVirtualNode(std::move(buildFunc));
    EXPECT_TRUE(mainNode->hasAccessibilityVirtualNode_);
    EXPECT_NE(accessibilityProperty->GetAccessibilityVirtualNode(), nullptr);
}

/**
 * @tc.name: SetAccessibilityCustomActions001
 * @tc.desc: Test SetAccessibilityCustomActions and ResetAccessibilityCustomActions on node and stack.
 * @tc.type: FUNC
 */
HWTEST_F(ViewAbstractModelGapTestNg, SetAccessibilityCustomActions001, TestSize.Level1)
{
    std::vector<AccessibilityCustomAction> actions;
    AccessibilityCustomAction action;
    action.actionName = TEST_ROLE;
    actions.push_back(action);
    ViewAbstractModelNG::SetAccessibilityCustomActions(nullptr, actions);
    ViewAbstractModelNG::ResetAccessibilityCustomActions(nullptr);

    const RefPtr<FrameNode> mainNode = FrameNode::CreateFrameNode("main", 15, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(mainNode, nullptr);
    ViewStackProcessor::GetInstance()->Push(mainNode);
    auto accessibilityProperty = mainNode->GetAccessibilityProperty<AccessibilityProperty>();
    ASSERT_NE(accessibilityProperty, nullptr);

    ViewAbstractModelNG::SetAccessibilityCustomActions(AceType::RawPtr(mainNode), actions);
    EXPECT_EQ(accessibilityProperty->GetAccessibilityCustomActions().size(), 1);
    ViewAbstractModelNG::ResetAccessibilityCustomActions(AceType::RawPtr(mainNode));
    EXPECT_EQ(accessibilityProperty->GetAccessibilityCustomActions().size(), 0);

    viewAbstractModelNG.SetAccessibilityCustomActions(actions);
    EXPECT_EQ(accessibilityProperty->GetAccessibilityCustomActions().size(), 1);
    viewAbstractModelNG.ResetAccessibilityCustomActions();
    EXPECT_EQ(accessibilityProperty->GetAccessibilityCustomActions().size(), 0);

    ViewStackProcessor::GetInstance()->ClearStack();
    viewAbstractModelNG.SetAccessibilityCustomActions(actions);
    viewAbstractModelNG.ResetAccessibilityCustomActions();
    EXPECT_NE(mainNode, nullptr);
}
} // namespace OHOS::Ace::NG
