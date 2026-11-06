/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#define private public
#include "core/common/frontend.h"
#include "core/common/window.h"
#include "core/components/web/resource/web_delegate.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/web/web_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/event/touch_event.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "mock_web_delegate.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::NWeb;
using namespace OHOS::Ace;
using namespace OHOS::Rosen;

namespace OHOS::Ace::NG {
namespace {
#ifdef OHOS_STANDARD_SYSTEM
bool g_isEnable = false;
float g_alpha = 0;
int32_t g_Y = 1;
float g_height = 0;

int32_t g_editStateFlags = 0;

int32_t CONTRNT_WIDTH_SIZE = 1024;
int32_t CONTRNT_HEIGHT_SIZE = 99999;

std::shared_ptr<NWebTouchHandleState> g_insertHandle = nullptr;
std::shared_ptr<NWebTouchHandleState> g_startSelectionHandle = nullptr;
std::shared_ptr<NWebTouchHandleState> g_endSelectionHandle = nullptr;

RefPtr<WebPattern> g_webPattern = nullptr;

// Constants for WebPattern security hit test tests.
const int32_t SECURITY_TEST_TOUCH_X = 100;
const int32_t SECURITY_TEST_TOUCH_Y = 200;
const int32_t SECURITY_TEST_NEG_TOUCH_X = -10;
const int32_t SECURITY_TEST_NEG_TOUCH_Y = -20;
const int32_t SECURITY_TEST_TOUCH_ID = 0;
const int32_t SECURITY_TEST_SECOND_TOUCH_ID = 2;
const int32_t SECURITY_TEST_DOM_NODE_ID = 123;
const int32_t SECURITY_TEST_DEFAULT_DOM_NODE_ID = -1;
const std::string SECURITY_TEST_ALLOW_LABEL = "normalLabel";
const std::string SECURITY_TEST_DENY_LABEL = "agentDeny";
const std::string SECURITY_TEST_PAGE_UUID = "page-uuid-001";
const std::string SECURITY_TEST_XPATH = "/html/body/div[1]";
const std::string SECURITY_TEST_ELEMENT_ID = "element-456";
const std::string SECURITY_TEST_DENY_ELEMENT_ID = "element-789";
// extraParams in the response json is no longer parsed by WebPattern, kept here to mirror the
// real response shape and pin the ignore behavior.
const std::string SECURITY_TEST_VALID_RESPONSE = "{\"domNodeId\":123,\"securityLabel\":\"normalLabel\","
    "\"pageUuid\":\"page-uuid-001\",\"xpath\":\"/html/body/div[1]\",\"elementId\":\"element-456\","
    "\"extraParams\":{\"nodeType\":\"button\",\"pageUrl\":\"https://example.com\"}}";
const std::string SECURITY_TEST_DENY_RESPONSE = "{\"domNodeId\":123,\"securityLabel\":\"agentDeny\","
    "\"pageUuid\":\"page-uuid-001\",\"xpath\":\"/html/body/div[1]\",\"elementId\":\"element-789\","
    "\"extraParams\":{\"nodeType\":\"button\"}}";
const std::string SECURITY_TEST_INVALID_RESPONSE = "{invalid-json-response";
const std::string SECURITY_TEST_WRONG_TYPE_RESPONSE = "{\"domNodeId\":\"123\",\"securityLabel\":\"normalLabel\","
    "\"pageUuid\":\"page-uuid-001\",\"xpath\":\"/html/body/div[1]\",\"elementId\":\"element-456\","
    "\"extraParams\":{}}";
const std::string SECURITY_TEST_MISSING_FIELD_RESPONSE = "{\"domNodeId\":123,\"xpath\":\"/html/body/div[1]\"}";
const std::string SECURITY_TEST_EMPTY_RESPONSE = "";
// All required fields present but extraParams key missing, extraParams is not parsed.
const std::string SECURITY_TEST_NO_EXTRA_PARAMS_RESPONSE = "{\"domNodeId\":123,\"securityLabel\":\"normalLabel\","
    "\"pageUuid\":\"page-uuid-001\",\"xpath\":\"/html/body/div[1]\",\"elementId\":\"element-456\"}";
const bool SECURITY_TEST_FROM_OVERLAY_FALSE = false;
const bool SECURITY_TEST_FROM_OVERLAY_TRUE = true;
const int32_t SECURITY_TEST_DEFAULT_CALLING_UID = -1;
const uint64_t SECURITY_TEST_DEFAULT_SESSION_ID = 0;
const int32_t SECURITY_TEST_CALLING_UID = 2000;
const uint64_t SECURITY_TEST_SESSION_ID = 67890;
const std::string SECURITY_TEST_EVENT_TYPE = "webtest";
#endif
} // namespace

class WebPatternTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WebPatternTest::SetUpTestCase()
{
#ifdef OHOS_STANDARD_SYSTEM
    std::string src = "web_test";
    RefPtr<WebController> controller = AceType::MakeRefPtr<WebController>();
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::WEB_ETS_TAG, nodeId, [src, controller]() { return AceType::MakeRefPtr<WebPattern>(src, controller); });
    stack->Push(frameNode);

    g_webPattern = frameNode->GetPattern<WebPattern>();
    CHECK_NULL_VOID(g_webPattern);
    g_webPattern->SetWebSrc(src);
    g_webPattern->SetWebController(controller);
#endif
}
void WebPatternTest::TearDownTestCase()
{
#ifdef OHOS_STANDARD_SYSTEM
    g_webPattern = nullptr;
#endif
}

void WebPatternTest::SetUp() {}
void WebPatternTest::TearDown() {}

#ifdef OHOS_STANDARD_SYSTEM
class NWebTouchHandleStateMock : public NWebTouchHandleState {
public:
    int32_t GetTouchHandleId() override
    {
        return 0;
    }

    int32_t GetX() override
    {
        return 0;
    }

    int32_t GetY() override
    {
        return g_Y;
    }

    int32_t GetViewPortX() override
    {
        return 0;
    }

    int32_t GetViewPortY() override
    {
        return 0;
    }

    TouchHandleType GetTouchHandleType() override
    {
        return TouchHandleType::INSERT_HANDLE;
    }

    bool IsEnable() override
    {
        return g_isEnable;
    }

    float GetAlpha() override
    {
        return g_alpha;
    }

    float GetEdgeHeight() override
    {
        return g_height;
    }
};

class NWebQuickMenuParamsMock : public NWebQuickMenuParams {
public:
    int32_t GetXCoord() override
    {
        return 0;
    }

    int32_t GetYCoord() override
    {
        return 0;
    }

    int32_t GetWidth() override
    {
        return 0;
    }

    int32_t GetHeight() override
    {
        return 0;
    }

    int32_t GetEditStateFlags() override
    {
        return g_editStateFlags;
    }

    int32_t GetSelectX() override
    {
        return 0;
    }

    int32_t GetSelectY() override
    {
        return 0;
    }

    int32_t GetSelectWidth() override
    {
        return 0;
    }

    int32_t GetSelectXHeight() override
    {
        return 0;
    }

    std::shared_ptr<NWebTouchHandleState> GetTouchHandleState(NWebTouchHandleState::TouchHandleType type) override
    {
        if (type == NWebTouchHandleState::TouchHandleType::INSERT_HANDLE) {
            return g_insertHandle;
        } else if (type == NWebTouchHandleState::TouchHandleType::SELECTION_BEGIN_HANDLE) {
            return g_startSelectionHandle;
        }
        return g_endSelectionHandle;
    }

    bool GetIsLongPressActived() override
    {
        return false;
    }
};

class NWebQuickMenuCallbackMock : public NWebQuickMenuCallback {
public:
    void Continue(int32_t commandId, MenuEventFlags flag) {}

    void Cancel() {}
};
#endif

/**
 * @tc.name: OnModifyDoneTest001
 * @tc.desc: Test OnModifyDone.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, OnModifyDoneTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    g_webPattern->OnModifyDone();
    int32_t width = 1;
    int32_t height = 1;
    double keyboard = 0;
    g_webPattern->isFocus_ = false;
    bool result = g_webPattern->ProcessVirtualKeyBoard(width, height, keyboard);
    EXPECT_FALSE(result);
    g_webPattern->isVirtualKeyBoardShow_ = WebPattern::VkState::VK_SHOW;
    result = g_webPattern->ProcessVirtualKeyBoard(width, height, keyboard);
    EXPECT_FALSE(result);
    g_webPattern->isFocus_ = true;
    result = g_webPattern->ProcessVirtualKeyBoard(width, height, keyboard);
    EXPECT_FALSE(result);
    keyboard = 1;
    result = g_webPattern->ProcessVirtualKeyBoard(width, height, keyboard);
    EXPECT_FALSE(result);
    g_webPattern->isVirtualKeyBoardShow_ = WebPattern::VkState::VK_HIDE;
    result = g_webPattern->ProcessVirtualKeyBoard(width, height, keyboard);
    EXPECT_FALSE(result);
#endif
}

/**
 * @tc.name: HandleTouchDownTest002
 * @tc.desc: Test HandleTouchDownTest002.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, HandleTouchDownTest002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    int32_t fingerId = 0;
    g_webPattern->OnModifyDone();
    int32_t width = 1;
    int32_t height = 1;
    double keyboard = 1;
    g_webPattern->isFocus_ = true;
    g_webPattern->isVirtualKeyBoardShow_ = WebPattern::VkState::VK_HIDE;
    auto drawSize = Size(CONTRNT_WIDTH_SIZE, CONTRNT_HEIGHT_SIZE);
    g_webPattern->drawSizeCache_ = drawSize;
    bool result = g_webPattern->ProcessVirtualKeyBoard(width, height, keyboard);
    EXPECT_FALSE(result);
    TouchLocationInfo info("webtest", fingerId);
    TouchEventInfo event("webtest");
    g_webPattern->HandleTouchUp(event, true);
    g_webPattern->HandleTouchDown(event, true);
    g_webPattern->HandleTouchMove(event, true);
    g_webPattern->isDragging_ = true;
    g_webPattern->HandleTouchMove(event, true);

    event.AddTouchLocationInfo(std::move(info));
    g_webPattern->HandleTouchUp(event, true);
    g_webPattern->HandleTouchDown(event, true);
    g_webPattern->HandleTouchMove(event, true);
    g_webPattern->HandleTouchCancel(event);
    g_webPattern->RequestFullScreen();
    g_webPattern->ExitFullScreen();
    g_webPattern->isFullScreen_ = true;
    g_webPattern->ExitFullScreen();
#endif
}

/**
 * @tc.name: OnOverviewUpdateTest008
 * @tc.desc: Test OnOverviewModeAccessEnabledUpdate.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, OnOverviewUpdateTest008, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    g_webPattern->OnModifyDone();
    int32_t value = 0;
    float scale = 0;
    int x = 0;
    int y = 0;
    EXPECT_NE(g_webPattern->delegate_, nullptr);
    g_webPattern->OnPinchSmoothModeEnabledUpdate(true);
    g_webPattern->OnWebDebuggingAccessEnabledAndPortUpdate(std::make_tuple(true, 0));
    g_webPattern->OnTextZoomRatioUpdate(value);
    g_webPattern->OnDatabaseAccessEnabledUpdate(true);
    g_webPattern->OnFileFromUrlAccessEnabledUpdate(true);
    g_webPattern->OnOverviewModeAccessEnabledUpdate(true);
    g_webPattern->OnBackgroundColorUpdate(value);
    g_webPattern->OnInitialScaleUpdate(scale);
    g_webPattern->OnMultiWindowAccessEnabledUpdate(true);
    g_webPattern->HandleDragStart(x, y);
    g_webPattern->isW3cDragEvent_ = true;
    DragDropInfo dragDropInfo;
    bool result = g_webPattern->GenerateDragDropInfo(dragDropInfo);
    EXPECT_FALSE(result);
    g_webPattern->delegate_->pixelMap_ = nullptr;
    result = g_webPattern->GenerateDragDropInfo(dragDropInfo);
    EXPECT_FALSE(result);
    RefPtr<WebController> controller = AceType::MakeRefPtr<WebController>();
    RefPtr<WebPattern> webPattern = AceType::MakeRefPtr<WebPattern>("test", controller);
    EXPECT_NE(webPattern, nullptr);
    webPattern->OnPinchSmoothModeEnabledUpdate(true);
    webPattern->OnWebDebuggingAccessEnabledAndPortUpdate(std::make_tuple(true, 0));
    webPattern->OnTextZoomRatioUpdate(value);
    webPattern->OnDatabaseAccessEnabledUpdate(true);
    webPattern->OnFileFromUrlAccessEnabledUpdate(true);
    webPattern->OnOverviewModeAccessEnabledUpdate(true);
    webPattern->OnBackgroundColorUpdate(value);
    webPattern->OnInitialScaleUpdate(scale);
    webPattern->OnMultiWindowAccessEnabledUpdate(true);
    webPattern->isW3cDragEvent_ = false;
    result = webPattern->GenerateDragDropInfo(dragDropInfo);
    EXPECT_FALSE(result);
#endif
}

/**
 * @tc.name: HandleDoubleClickEventTest009
 * @tc.desc: Test HandleDoubleClickEvent.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, HandleDoubleClickEventTest009, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    MouseInfo info;
    info.SetButton(MouseButton::LEFT_BUTTON);
    info.SetAction(MouseAction::NONE);
    std::queue<MouseClickInfo> empty;
    swap(empty, g_webPattern->mouseClickQueue_);
    g_webPattern->HandleDoubleClickEvent(info);
    EXPECT_FALSE(g_webPattern->HandleDoubleClickEvent(info));
#endif
}

/**
 * @tc.name: HandleDragUpdateTest010
 * @tc.desc: Test HandleDragUpdate.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, HandleDragUpdateTest010, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    g_webPattern->OnModifyDone();
    GestureEvent info;
    int x = 0;
    int y = 0;
    g_webPattern->isW3cDragEvent_ = false;
    g_webPattern->HandleDragCancel();
    g_webPattern->HandleDragEnd(x, y);
    g_webPattern->isW3cDragEvent_ = true;
    g_webPattern->HandleDragCancel();
    g_webPattern->HandleDragEnd(x, y);
    g_webPattern->needUpdateWeb_ = false;
    auto pipelineContext = PipelineContext::GetCurrentContext();
    g_webPattern->RegistVirtualKeyBoardListener(pipelineContext);

    RefPtr<WebController> controller = AceType::MakeRefPtr<WebController>();
    RefPtr<WebPattern> webPattern = AceType::MakeRefPtr<WebPattern>("test", controller);
    EXPECT_NE(webPattern, nullptr);
    webPattern->isW3cDragEvent_ = true;
    webPattern->HandleDragEnd(x, y);
    g_webPattern->HandleDragCancel();
    webPattern->isW3cDragEvent_ = false;
    webPattern->HandleDragEnd(x, y);
    g_webPattern->HandleDragCancel();
#endif
}

/**
 * @tc.name: OnWindowShowTest011
 * @tc.desc: Test OnWindowShow.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, OnWindowShowTest011, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    g_webPattern->OnModifyDone();
    g_webPattern->isWindowShow_ = true;
    g_webPattern->OnWindowShow();
    g_webPattern->OnWindowHide();
    g_webPattern->isWindowShow_ = false;
    g_webPattern->OnWindowHide();
    g_webPattern->OnWindowShow();
    EXPECT_TRUE(g_webPattern->isWindowShow_);

    g_webPattern->isActive_ = true;
    g_webPattern->OnActive();
    g_webPattern->OnInActive();
    g_webPattern->isActive_ = false;
    g_webPattern->OnInActive();
    g_webPattern->OnActive();
    EXPECT_TRUE(g_webPattern->isActive_);

    g_webPattern->OnVisibleChange(false);
    g_webPattern->OnVisibleChange(true);
#endif
}

/**
 * @tc.name: OnWindowShowTest02
 * @tc.desc: Test OnWindowShow.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, OnWindowShowTest02, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    g_webPattern->OnModifyDone();
    auto host = g_webPattern->GetHost();
    EXPECT_NE(host, nullptr);

    g_webPattern->isWindowShow_ = false;
    g_webPattern->isVisible_ = false;
    
    g_webPattern->offlineWebInited_ = false;
    host->UpdateNodeStatus(NodeStatus::BUILDER_NODE_ON_MAINTREE);
    g_webPattern->OnWindowShow();
    EXPECT_TRUE(g_webPattern->isWindowShow_);

    g_webPattern->offlineWebInited_ = false;
    host->UpdateNodeStatus(NodeStatus::BUILDER_NODE_OFF_MAINTREE);
    g_webPattern->OnWindowShow();
    EXPECT_TRUE(g_webPattern->isWindowShow_);

    g_webPattern->offlineWebInited_ = true;
    host->UpdateNodeStatus(NodeStatus::BUILDER_NODE_ON_MAINTREE);
    g_webPattern->OnWindowShow();
    EXPECT_TRUE(g_webPattern->isWindowShow_);

    g_webPattern->offlineWebInited_ = true;
    host->UpdateNodeStatus(NodeStatus::BUILDER_NODE_OFF_MAINTREE);
    g_webPattern->OnWindowShow();
    EXPECT_TRUE(g_webPattern->isWindowShow_);
#endif
}

/**
 * @tc.name: OnWindowShowTest03
 * @tc.desc: Test OnWindowShow.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, OnWindowShowTest03, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    g_webPattern->OnModifyDone();
    auto host = g_webPattern->GetHost();
    EXPECT_NE(host, nullptr);
    auto layoutProperty = host->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    g_webPattern->offlineWebInited_ = false;
    host->UpdateNodeStatus(NodeStatus::BUILDER_NODE_OFF_MAINTREE);

    g_webPattern->isWindowShow_ = false;
    g_webPattern->isVisible_ = true;
    layoutProperty->UpdateVisibility(VisibleType::INVISIBLE);
    EXPECT_EQ(layoutProperty->GetVisibility(), VisibleType::INVISIBLE);
    g_webPattern->OnWindowShow();
    EXPECT_TRUE(g_webPattern->isWindowShow_);

    g_webPattern->isWindowShow_ = false;
    g_webPattern->isVisible_ = true;
    layoutProperty->UpdateVisibility(VisibleType::VISIBLE);
    EXPECT_EQ(layoutProperty->GetVisibility(), VisibleType::VISIBLE);
    g_webPattern->OnWindowShow();
    EXPECT_TRUE(g_webPattern->isWindowShow_);
#endif
}

/**
 * @tc.name: OnWindowShowTest04
 * @tc.desc: Test OnWindowShow.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, OnWindowShowTest04, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    g_webPattern->OnModifyDone();
    auto host = g_webPattern->GetHost();
    EXPECT_NE(host, nullptr);
    auto layoutProperty = host->GetLayoutProperty();
    g_webPattern->offlineWebInited_ = true;
    host->UpdateNodeStatus(NodeStatus::BUILDER_NODE_ON_MAINTREE);

    g_webPattern->isWindowShow_ = true;
    g_webPattern->isOfflineWebEvictFrameBuffersEnable_ = false;
    g_webPattern->OnWindowShow();
    EXPECT_TRUE(g_webPattern->isWindowShow_);

    g_webPattern->isOfflineWebEvictFrameBuffersEnable_ = true;
    g_webPattern->OnWindowShow();
    EXPECT_TRUE(g_webPattern->isWindowShow_);
#endif
}

/**
 * @tc.name: MenuAvoidKeyboard
 * @tc.desc: Test MenuAvoidKeyboard.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, MenuAvoidKeyboard, TestSize.Level1)
{
    MockPipelineContext::SetUp();
    bool result = g_webPattern->MenuAvoidKeyboard(true, 0.0);
    EXPECT_TRUE(result);
    result = g_webPattern->MenuAvoidKeyboard(false, 0.0);
    EXPECT_TRUE(result);
    MockPipelineContext::TearDown();
}

/**
 * @tc.name: SetLinkPreviewSelectionMenu001
 * @tc.desc: Test SetPreviewSelectionMenu for link.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, SetLinkPreviewSelectionMenu001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    std::function<void()> menuBuilder = nullptr;
    std::function<void()> previewBuilder = nullptr;
    NG::MenuParam menuParam;
    std::shared_ptr<WebPreviewSelectionMenuParam> param = std::make_shared<WebPreviewSelectionMenuParam>(
        WebElementType::LINK, ResponseType::LONG_PRESS, menuBuilder, previewBuilder, menuParam);
    g_webPattern->SetPreviewSelectionMenu(param);
#endif
}

/**
 * @tc.name: ProcessVirtualKeyBoardShowAvoidMenu
 * @tc.desc: Test ProcessVirtualKeyBoardShowAvoidMenu.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ProcessVirtualKeyBoardShowAvoidMenu, TestSize.Level1)
{
    bool result;
    result = g_webPattern->ProcessVirtualKeyBoardShowAvoidMenu(0, 1280, 0.0, false);
    EXPECT_TRUE(result);
    result = g_webPattern->ProcessVirtualKeyBoardShowAvoidMenu(0, 1280, 0.0, true);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ProcessVirtualKeyBoardHideAvoidMenu
 * @tc.desc: Test ProcessVirtualKeyBoardHideAvoidMenu.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ProcessVirtualKeyBoardHideAvoidMenu, TestSize.Level1)
{
    bool result;
    g_webPattern->isVirtualKeyBoardShow_ = WebPattern::VkState::VK_SHOW;
    result = g_webPattern->ProcessVirtualKeyBoardHideAvoidMenu(0, 1280, false);
    EXPECT_TRUE(result);

    g_webPattern->isVirtualKeyBoardShow_ = WebPattern::VkState::VK_SHOW;
    result = g_webPattern->ProcessVirtualKeyBoardHideAvoidMenu(0, 1280, true);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: VirtualKeyBoard UpdateTextFieldStatus
 * @tc.desc: Test UpdateTextFieldStatus.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, UpdateTextFieldStatus, TestSize.Level1)
{
    g_webPattern->isImeStatus_ = WebPattern::VkState::VK_HIDE;
    g_webPattern->UpdateTextFieldStatus(true, true);
    EXPECT_TRUE(g_webPattern->isImeStatus_ == WebPattern::VkState::VK_SHOW);

    g_webPattern->isImeStatus_ = WebPattern::VkState::VK_SHOW;
    g_webPattern->UpdateTextFieldStatus(false, false);
    EXPECT_TRUE(g_webPattern->isImeStatus_ == WebPattern::VkState::VK_HIDE);
}

/**
 * @tc.name: VirtualKeyBoard IsImeStatusShow
 * @tc.desc: Test IsImeStatusShow.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, IsImeStatusShow, TestSize.Level1)
{
    g_webPattern->isImeStatus_ = WebPattern::VkState::VK_HIDE;
    EXPECT_TRUE(!g_webPattern->IsImeStatusShow());

    g_webPattern->isImeStatus_ = WebPattern::VkState::VK_SHOW;
    EXPECT_TRUE(g_webPattern->IsImeStatusShow());
}

/**
 * @tc.name: ProcessVirtualKeyBoardShow
 * @tc.desc: Test ProcessVirtualKeyBoardShow.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ProcessVirtualKeyBoardShow, TestSize.Level1)
{
    bool result;
    g_webPattern->isVirtualKeyBoardShow_ = WebPattern::VkState::VK_SHOW;
    g_webPattern->keyBoardAvoidMode_ = WebKeyboardAvoidMode::DEFAULT;
    result = g_webPattern->ProcessVirtualKeyBoardShow(0, 200, 300, false);
    EXPECT_TRUE(result);

    g_webPattern->isVirtualKeyBoardShow_ = WebPattern::VkState::VK_SHOW;
    g_webPattern->keyBoardAvoidMode_ = WebKeyboardAvoidMode::RETURN_TO_UICONTEXT;
    result = g_webPattern->ProcessVirtualKeyBoardShow(0, 200, 300, false);
    EXPECT_FALSE(result);

    g_webPattern->isVirtualKeyBoardShow_ = WebPattern::VkState::VK_SHOW;
    g_webPattern->keyBoardAvoidMode_ = WebKeyboardAvoidMode::RETURN_TO_UICONTEXT;
    result = g_webPattern->ProcessVirtualKeyBoardShow(0, 200, 300, true);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ProcessVirtualKeyBoardHide
 * @tc.desc: Test ProcessVirtualKeyBoardHide.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ProcessVirtualKeyBoardHide, TestSize.Level1)
{
    bool result;
    g_webPattern->isVirtualKeyBoardShow_ = WebPattern::VkState::VK_SHOW;
    g_webPattern->keyBoardAvoidMode_ = WebKeyboardAvoidMode::DEFAULT;
    result = g_webPattern->ProcessVirtualKeyBoardHide(0, 200, false);
    EXPECT_TRUE(result);

    g_webPattern->isVirtualKeyBoardShow_ = WebPattern::VkState::VK_SHOW;
    g_webPattern->keyBoardAvoidMode_ = WebKeyboardAvoidMode::RETURN_TO_UICONTEXT;
    result = g_webPattern->ProcessVirtualKeyBoardHide(0, 200, false);
    EXPECT_FALSE(result);

    g_webPattern->isVirtualKeyBoardShow_ = WebPattern::VkState::VK_SHOW;
    g_webPattern->keyBoardAvoidMode_ = WebKeyboardAvoidMode::RETURN_TO_UICONTEXT;
    result = g_webPattern->ProcessVirtualKeyBoardHide(0, 200, true);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: JudgeWebKeyBoardAvoidMode
 * @tc.desc: Test JudgeWebKeyBoardAvoidMode.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, JudgeWebKeyBoardAvoidMode, TestSize.Level1)
{
    bool result;
    g_webPattern->isVirtualKeyBoardShow_ = WebPattern::VkState::VK_SHOW;
    g_webPattern->keyBoardAvoidMode_ = WebKeyboardAvoidMode::DEFAULT;
    result = g_webPattern->JudgeWebKeyBoardAvoidMode(false);
    EXPECT_TRUE(result);

    g_webPattern->isVirtualKeyBoardShow_ = WebPattern::VkState::VK_SHOW;
    g_webPattern->keyBoardAvoidMode_ = WebKeyboardAvoidMode::RETURN_TO_UICONTEXT;
    result = g_webPattern->JudgeWebKeyBoardAvoidMode(false);
    EXPECT_FALSE(result);

    g_webPattern->isVirtualKeyBoardShow_ = WebPattern::VkState::VK_SHOW;
    g_webPattern->keyBoardAvoidMode_ = WebKeyboardAvoidMode::RETURN_TO_UICONTEXT;
    result = g_webPattern->JudgeWebKeyBoardAvoidMode(true);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: UpdateScrollBarWithBorderRadius
 * @tc.desc: Test UpdateScrollBarWithBorderRadius.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, UpdateScrollBarWithBorderRadius, TestSize.Level1)
{
    std::string src = "web_test";
    RefPtr<WebController> controller = AceType::MakeRefPtr<WebController>();
    ASSERT_NE(controller, nullptr);
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::WEB_ETS_TAG, nodeId, [src, controller]() { return AceType::MakeRefPtr<WebPattern>(src, controller); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);

    RefPtr<WebPattern> webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    auto host = webPattern->GetHost();
    ASSERT_NE(host, nullptr);
    auto renderContext = host->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);
    BorderRadiusProperty borderRadius;

    webPattern->UpdateScrollBarWithBorderRadius();
    bool borderRadiusStructHasValue = renderContext->GetBorderRadius().has_value();
    EXPECT_FALSE(borderRadiusStructHasValue);
    webPattern->UpdateScrollBarWithBorderRadius();
    bool hasBorderRadiusValue = borderRadius.radiusTopLeft.has_value();
    EXPECT_FALSE(hasBorderRadiusValue);
    
    borderRadius.radiusTopLeft = Dimension(10.0f);
    borderRadius.radiusTopRight = Dimension(20.0f);
    borderRadius.radiusBottomLeft = Dimension(30.0f);
    borderRadius.radiusBottomRight = Dimension(40.0f);
    renderContext->UpdateBorderRadius(borderRadius);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    auto clipState = renderContext->GetClipEdge().value_or(false);
    webPattern->UpdateScrollBarWithBorderRadius();
    renderContext->UpdateClipEdge(true);
    clipState = renderContext->GetClipEdge().value_or(false);
    webPattern->UpdateScrollBarWithBorderRadius();

    borderRadiusStructHasValue = renderContext->GetBorderRadius().has_value();
    webPattern->UpdateScrollBarWithBorderRadius();
    EXPECT_TRUE(borderRadiusStructHasValue);
    hasBorderRadiusValue = !borderRadius.radiusTopLeft.has_value();
    webPattern->UpdateScrollBarWithBorderRadius();
    EXPECT_FALSE(hasBorderRadiusValue);
}

/**
 * @tc.name: IsShowHandle
 * @tc.desc: Test IsShowHandle
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, IsShowHandle, TestSize.Level1)
{
    std::string src = "web_test";
    RefPtr<WebController> controller = AceType::MakeRefPtr<WebController>();
    ASSERT_NE(controller, nullptr);
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::WEB_ETS_TAG, nodeId, [src, controller]() { return AceType::MakeRefPtr<WebPattern>(src, controller); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);

    RefPtr<WebPattern> webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    auto result = webPattern->IsShowHandle();
    EXPECT_FALSE(result);
    webPattern->webSelectOverlay_ = AceType::MakeRefPtr<WebSelectOverlay>(webPattern);
    ASSERT_NE(webPattern->webSelectOverlay_, nullptr);
    webPattern->webSelectOverlay_->isShowHandle_ = true;
    result = webPattern->IsShowHandle();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: HandleMouseToTouchEvent
 * @tc.desc: Test HandleMouseToTouchEvent.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, HandleMouseToTouchEvent, TestSize.Level1)
{
    bool result;
    g_webPattern->emulateTouchFromMouseEvent_ = false;
    MouseInfo info;
    result = g_webPattern->HandleMouseToTouchEvent(0.0, false, info);
    EXPECT_FALSE(result);
    g_webPattern->emulateTouchFromMouseEvent_ = true;
    info.SetButton(MouseButton::NONE_BUTTON);

    result = g_webPattern->HandleMouseToTouchEvent(0.0, false, info);
    EXPECT_FALSE(result);
    info.SetButton(MouseButton::LEFT_BUTTON);
    info.SetAction(MouseAction::NONE);
    result = g_webPattern->HandleMouseToTouchEvent(0.0, false, info);
    EXPECT_FALSE(result);

    for (TouchLocationInfo& location : g_webPattern->touchEventInfo_.changedTouches_) {
        location.SetTouchType(TouchType::DOWN);
    }
    info.SetAction(MouseAction::PRESS);
    result = g_webPattern->HandleMouseToTouchEvent(0.0, false, info);
    EXPECT_TRUE(result);

    for (TouchLocationInfo& location : g_webPattern->touchEventInfo_.changedTouches_) {
        location.SetTouchType(TouchType::MOVE);
    }
    info.SetAction(MouseAction::PRESS);
    result = g_webPattern->HandleMouseToTouchEvent(0.0, false, info);
    EXPECT_TRUE(result);

    for (TouchLocationInfo& location : g_webPattern->touchEventInfo_.changedTouches_) {
        location.SetTouchType(TouchType::UP);
    }
    info.SetAction(MouseAction::PRESS);
    result = g_webPattern->HandleMouseToTouchEvent(0.0, false, info);
    EXPECT_TRUE(result);

    for (TouchLocationInfo& location : g_webPattern->touchEventInfo_.changedTouches_) {
        location.SetTouchType(TouchType::DOWN);
    }
    info.SetAction(MouseAction::PRESS);
    result = g_webPattern->HandleMouseToTouchEvent(0.0, false, info);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OnClippedSelectionBoundsChanged
 * @tc.desc: Test OnClippedSelectionBoundsChanged
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, OnClippedSelectionBoundsChanged, TestSize.Level1)
{
    std::string src = "web_test";
    RefPtr<WebController> controller = AceType::MakeRefPtr<WebController>();
    ASSERT_NE(controller, nullptr);
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::WEB_ETS_TAG, nodeId, [src, controller]() { return AceType::MakeRefPtr<WebPattern>(src, controller); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);

    RefPtr<WebPattern> webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->webSelectOverlay_ = nullptr;
    webPattern->OnClippedSelectionBoundsChanged(1, 2, 3, 4);
    webPattern->webSelectOverlay_ = AceType::MakeRefPtr<WebSelectOverlay>(webPattern);
    ASSERT_NE(webPattern->webSelectOverlay_, nullptr);
    webPattern->OnClippedSelectionBoundsChanged(1, 2, 3, 4);
    EXPECT_EQ(webPattern->webSelectOverlay_->selectArea_.Left(), 1);
    EXPECT_EQ(webPattern->webSelectOverlay_->selectArea_.Top(), 2);
    EXPECT_EQ(webPattern->webSelectOverlay_->selectArea_.Width(), 3);
    EXPECT_EQ(webPattern->webSelectOverlay_->selectArea_.Height(), 4);
}

/**
 * @tc.name: CleanupWebPatternResource
 * @tc.desc: Test CleanupWebPatternResource
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, CleanupWebPatternResource, TestSize.Level1)
{
    MockPipelineContext::SetUp();
    g_webPattern->offlineWebInited_ = true;
    g_webPattern->CleanupWebPatternResource();
    EXPECT_TRUE(g_webPattern->offlineWebInited_);
    MockPipelineContext::TearDown();
}

/**
 * @tc.name: NotifyOverlayRotation
 * @tc.desc: Test NotifyOverlayRotation
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, NotifyOverlayRotation, TestSize.Level1)
{
    std::string src = "web_test";
    RefPtr<WebController> controller = AceType::MakeRefPtr<WebController>();
    ASSERT_NE(controller, nullptr);
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::WEB_ETS_TAG, nodeId, [src, controller]() { return AceType::MakeRefPtr<WebPattern>(src, controller); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);

    RefPtr<WebPattern> webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->webSelectOverlay_ = AceType::MakeRefPtr<WebSelectOverlay>(webPattern);
    ASSERT_NE(webPattern->webSelectOverlay_, nullptr);
    webPattern->webSelectOverlay_->selectTemporarilyHidden_ = true;
    webPattern->webSelectOverlay_->webSelectInfo_.menuInfo.menuIsShow = true;
    webPattern->NotifyOverlayRotation();
    EXPECT_EQ(webPattern->webSelectOverlay_->webSelectInfo_.menuInfo.menuIsShow, true);
    webPattern->webSelectOverlay_->webSelectInfo_.menuInfo.menuIsShow = false;
    webPattern->NotifyOverlayRotation();
    EXPECT_EQ(webPattern->webSelectOverlay_->webSelectInfo_.menuInfo.menuIsShow, false);
}

/**
 * @tc.name: ShowMagnifierTest001
 * @tc.desc: Test ShowMagnifier with isMove=false.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ShowMagnifierTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    std::string src = "web_test";
    RefPtr<WebController> controller = AceType::MakeRefPtr<WebController>();
    ASSERT_NE(controller, nullptr);
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::WEB_ETS_TAG, nodeId, [src, controller]() { return AceType::MakeRefPtr<WebPattern>(src, controller); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);

    RefPtr<WebPattern> webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    webPattern->magnifierController_ = AceType::MakeRefPtr<MagnifierController>(WeakPtr<Pattern>(webPattern));
    ASSERT_NE(webPattern->magnifierController_, nullptr);

    int32_t centerX = 100;
    int32_t centerY = 200;

    webPattern->ShowMagnifier(centerX, centerY, false);
    OffsetF offset = webPattern->magnifierController_->GetLocalOffset();
    EXPECT_NEAR(offset.GetX(), centerX, 0.01f);
    EXPECT_NEAR(offset.GetY(), centerY, 0.01f);
#endif
}

/**
 * @tc.name: ShowMagnifierTest002
 * @tc.desc: Test ShowMagnifier with isMove=true and same position (early return).
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ShowMagnifierTest002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    std::string src = "web_test";
    RefPtr<WebController> controller = AceType::MakeRefPtr<WebController>();
    ASSERT_NE(controller, nullptr);
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::WEB_ETS_TAG, nodeId, [src, controller]() { return AceType::MakeRefPtr<WebPattern>(src, controller); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);

    RefPtr<WebPattern> webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    webPattern->magnifierController_ = AceType::MakeRefPtr<MagnifierController>(WeakPtr<Pattern>(webPattern));
    ASSERT_NE(webPattern->magnifierController_, nullptr);

    int32_t centerX = 100;
    int32_t centerY = 200;

    webPattern->ShowMagnifier(centerX, centerY, false);
    OffsetF initialOffset = webPattern->magnifierController_->GetLocalOffset();

    webPattern->ShowMagnifier(centerX, centerY, true);
    OffsetF sameOffset = webPattern->magnifierController_->GetLocalOffset();
    EXPECT_NEAR(sameOffset.GetX(), initialOffset.GetX(), 0.01f);
    EXPECT_NEAR(sameOffset.GetY(), initialOffset.GetY(), 0.01f);
#endif
}

/**
 * @tc.name: ShowMagnifierTest003
 * @tc.desc: Test ShowMagnifier with isMove=true and different X coordinate.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ShowMagnifierTest003, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    std::string src = "web_test";
    RefPtr<WebController> controller = AceType::MakeRefPtr<WebController>();
    ASSERT_NE(controller, nullptr);
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::WEB_ETS_TAG, nodeId, [src, controller]() { return AceType::MakeRefPtr<WebPattern>(src, controller); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);

    RefPtr<WebPattern> webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    webPattern->magnifierController_ = AceType::MakeRefPtr<MagnifierController>(WeakPtr<Pattern>(webPattern));
    ASSERT_NE(webPattern->magnifierController_, nullptr);

    int32_t centerX = 100;
    int32_t centerY = 200;
    int32_t newCenterX = 150;

    webPattern->ShowMagnifier(centerX, centerY, false);
    webPattern->ShowMagnifier(newCenterX, centerY, true);

    OffsetF offset = webPattern->magnifierController_->GetLocalOffset();
    EXPECT_NEAR(offset.GetX(), newCenterX, 0.01f);
    EXPECT_NEAR(offset.GetY(), centerY, 0.01f);
#endif
}

/**
 * @tc.name: ShowMagnifierTest004
 * @tc.desc: Test ShowMagnifier with isMove=true and different Y coordinate.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ShowMagnifierTest004, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    std::string src = "web_test";
    RefPtr<WebController> controller = AceType::MakeRefPtr<WebController>();
    ASSERT_NE(controller, nullptr);
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::WEB_ETS_TAG, nodeId, [src, controller]() { return AceType::MakeRefPtr<WebPattern>(src, controller); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);

    RefPtr<WebPattern> webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    webPattern->magnifierController_ = AceType::MakeRefPtr<MagnifierController>(WeakPtr<Pattern>(webPattern));
    ASSERT_NE(webPattern->magnifierController_, nullptr);

    int32_t centerX = 100;
    int32_t centerY = 200;
    int32_t newCenterY = 250;

    webPattern->ShowMagnifier(centerX, centerY, false);
    webPattern->ShowMagnifier(centerX, newCenterY, true);

    OffsetF offset = webPattern->magnifierController_->GetLocalOffset();
    EXPECT_NEAR(offset.GetX(), centerX, 0.01f);
    EXPECT_NEAR(offset.GetY(), newCenterY, 0.01f);
#endif
}

/**
 * @tc.name: ShowMagnifierTest005
 * @tc.desc: Test ShowMagnifier with isMove=true and different X and Y coordinates.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ShowMagnifierTest005, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    std::string src = "web_test";
    RefPtr<WebController> controller = AceType::MakeRefPtr<WebController>();
    ASSERT_NE(controller, nullptr);
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::WEB_ETS_TAG, nodeId, [src, controller]() { return AceType::MakeRefPtr<WebPattern>(src, controller); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);

    RefPtr<WebPattern> webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    webPattern->magnifierController_ = AceType::MakeRefPtr<MagnifierController>(WeakPtr<Pattern>(webPattern));
    ASSERT_NE(webPattern->magnifierController_, nullptr);

    int32_t centerX = 100;
    int32_t centerY = 200;
    int32_t newCenterX = 150;
    int32_t newCenterY = 250;

    webPattern->ShowMagnifier(centerX, centerY, false);
    webPattern->ShowMagnifier(newCenterX, newCenterY, true);

    OffsetF offset = webPattern->magnifierController_->GetLocalOffset();
    EXPECT_NEAR(offset.GetX(), newCenterX, 0.01f);
    EXPECT_NEAR(offset.GetY(), newCenterY, 0.01f);
#endif
}

/**
 * @tc.name: ParseSecurityHitTestElementInfo001
 * @tc.desc: Test ParseSecurityHitTestElementInfo returns default info when response is empty.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ParseSecurityHitTestElementInfo001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto result = g_webPattern->ParseSecurityHitTestElementInfo(SECURITY_TEST_EMPTY_RESPONSE);
    EXPECT_EQ(result.domNodeId, SECURITY_TEST_DEFAULT_DOM_NODE_ID);
    EXPECT_TRUE(result.securityLabel.empty());
    EXPECT_TRUE(result.pageUuid.empty());
    EXPECT_TRUE(result.xpath.empty());
    EXPECT_TRUE(result.elementId.empty());
    EXPECT_TRUE(result.extraParams.empty());
#endif
}

/**
 * @tc.name: ParseSecurityHitTestElementInfo002
 * @tc.desc: Test ParseSecurityHitTestElementInfo returns default info when response is invalid json.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ParseSecurityHitTestElementInfo002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto result = g_webPattern->ParseSecurityHitTestElementInfo(SECURITY_TEST_INVALID_RESPONSE);
    EXPECT_EQ(result.domNodeId, SECURITY_TEST_DEFAULT_DOM_NODE_ID);
    EXPECT_TRUE(result.securityLabel.empty());
    EXPECT_TRUE(result.pageUuid.empty());
    EXPECT_TRUE(result.xpath.empty());
    EXPECT_TRUE(result.elementId.empty());
    EXPECT_TRUE(result.extraParams.empty());
#endif
}

/**
 * @tc.name: ParseSecurityHitTestElementInfo003
 * @tc.desc: Test ParseSecurityHitTestElementInfo parses all required fields from a valid response
 *           and ignores extraParams.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ParseSecurityHitTestElementInfo003, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto result = g_webPattern->ParseSecurityHitTestElementInfo(SECURITY_TEST_VALID_RESPONSE);
    EXPECT_EQ(result.domNodeId, SECURITY_TEST_DOM_NODE_ID);
    EXPECT_EQ(result.securityLabel, SECURITY_TEST_ALLOW_LABEL);
    EXPECT_EQ(result.pageUuid, SECURITY_TEST_PAGE_UUID);
    EXPECT_EQ(result.xpath, SECURITY_TEST_XPATH);
    EXPECT_EQ(result.elementId, SECURITY_TEST_ELEMENT_ID);
    // extraParams in response is no longer parsed
    EXPECT_TRUE(result.extraParams.empty());
#endif
}

/**
 * @tc.name: ParseSecurityHitTestElementInfo004
 * @tc.desc: Test ParseSecurityHitTestElementInfo returns default info when domNodeId is not a number.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ParseSecurityHitTestElementInfo004, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto result = g_webPattern->ParseSecurityHitTestElementInfo(SECURITY_TEST_WRONG_TYPE_RESPONSE);
    EXPECT_EQ(result.domNodeId, SECURITY_TEST_DEFAULT_DOM_NODE_ID);
    EXPECT_TRUE(result.securityLabel.empty());
    EXPECT_TRUE(result.extraParams.empty());
#endif
}

/**
 * @tc.name: ParseSecurityHitTestElementInfo005
 * @tc.desc: Test ParseSecurityHitTestElementInfo returns default info when required fields are missing.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ParseSecurityHitTestElementInfo005, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto result = g_webPattern->ParseSecurityHitTestElementInfo(SECURITY_TEST_MISSING_FIELD_RESPONSE);
    EXPECT_EQ(result.domNodeId, SECURITY_TEST_DEFAULT_DOM_NODE_ID);
    EXPECT_TRUE(result.securityLabel.empty());
    EXPECT_TRUE(result.extraParams.empty());
#endif
}

/**
 * @tc.name: ParseSecurityHitTestElementInfo006
 * @tc.desc: Test ParseSecurityHitTestElementInfo returns the deny security label from response.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ParseSecurityHitTestElementInfo006, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto result = g_webPattern->ParseSecurityHitTestElementInfo(SECURITY_TEST_DENY_RESPONSE);
    EXPECT_EQ(result.domNodeId, SECURITY_TEST_DOM_NODE_ID);
    EXPECT_EQ(result.securityLabel, SECURITY_TEST_DENY_LABEL);
    EXPECT_EQ(result.elementId, SECURITY_TEST_DENY_ELEMENT_ID);
#endif
}

/**
 * @tc.name: ParseSecurityHitTestElementInfo007
 * @tc.desc: Test ParseSecurityHitTestElementInfo parses required fields when extraParams is
 *           missing from response.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ParseSecurityHitTestElementInfo007, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    // extraParams is not parsed, required fields are still parsed when the key is missing
    auto result = g_webPattern->ParseSecurityHitTestElementInfo(SECURITY_TEST_NO_EXTRA_PARAMS_RESPONSE);
    EXPECT_EQ(result.domNodeId, SECURITY_TEST_DOM_NODE_ID);
    EXPECT_EQ(result.securityLabel, SECURITY_TEST_ALLOW_LABEL);
    EXPECT_EQ(result.pageUuid, SECURITY_TEST_PAGE_UUID);
    EXPECT_EQ(result.xpath, SECURITY_TEST_XPATH);
    EXPECT_EQ(result.elementId, SECURITY_TEST_ELEMENT_ID);
    EXPECT_TRUE(result.extraParams.empty());
#endif
}

/**
 * @tc.name: ReportDecsionForSecurityHitTest001
 * @tc.desc: Test ReportDecsionForSecurityHitTest returns false for deny security label.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ReportDecsionForSecurityHitTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    std::string response = SECURITY_TEST_DENY_RESPONSE;
    bool allowed = g_webPattern->ReportDecsionForSecurityHitTest(
        SECURITY_TEST_CALLING_UID, SECURITY_TEST_SESSION_ID, response);
    EXPECT_FALSE(allowed);
#endif
}

/**
 * @tc.name: ReportDecsionForSecurityHitTest002
 * @tc.desc: Test ReportDecsionForSecurityHitTest returns true for allowed security label.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ReportDecsionForSecurityHitTest002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    std::string response = SECURITY_TEST_VALID_RESPONSE;
    bool allowed = g_webPattern->ReportDecsionForSecurityHitTest(
        SECURITY_TEST_CALLING_UID, SECURITY_TEST_SESSION_ID, response);
    EXPECT_TRUE(allowed);
#endif
}

/**
 * @tc.name: ReportDecsionForSecurityHitTest003
 * @tc.desc: Test ReportDecsionForSecurityHitTest returns true when response is empty.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ReportDecsionForSecurityHitTest003, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    std::string response = SECURITY_TEST_EMPTY_RESPONSE;
    bool allowed = g_webPattern->ReportDecsionForSecurityHitTest(
        SECURITY_TEST_CALLING_UID, SECURITY_TEST_SESSION_ID, response);
    EXPECT_TRUE(allowed);
#endif
}

/**
 * @tc.name: ReportDecsionForSecurityHitTest004
 * @tc.desc: Test ReportDecsionForSecurityHitTest returns false when pattern has no host node.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, ReportDecsionForSecurityHitTest004, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto webPattern = AceType::MakeRefPtr<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    std::string response = SECURITY_TEST_VALID_RESPONSE;
    // host node is null, reporting returns false before parsing
    bool allowed = webPattern->ReportDecsionForSecurityHitTest(
        SECURITY_TEST_CALLING_UID, SECURITY_TEST_SESSION_ID, response);
    EXPECT_FALSE(allowed);
#endif
}

/**
 * @tc.name: JudgeSecurityHitTestForValidation001
 * @tc.desc: Test JudgeSecurityHitTestForValidation does not delegate when delegate is null.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, JudgeSecurityHitTestForValidation001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    ResetMockSecurityHitTestInfo();
    TouchEventInfo eventInfo(SECURITY_TEST_EVENT_TYPE);
    // delegate_ is null, validation returns before delegating
    webPattern->JudgeSecurityHitTestForValidation(SECURITY_TEST_TOUCH_ID, SECURITY_TEST_TOUCH_X,
        SECURITY_TEST_TOUCH_Y, SECURITY_TEST_FROM_OVERLAY_FALSE, eventInfo);
    int32_t id = 0;
    double x = 0.0;
    double y = 0.0;
    bool fromOverlay = false;
    int32_t callingUid = 0;
    uint64_t sessionId = 0;
    std::string response;
    EXPECT_FALSE(GetRecordedSecurityHitTestInfo(id, x, y, fromOverlay, callingUid, sessionId, response));
#endif
}

/**
 * @tc.name: JudgeSecurityHitTestForValidation002
 * @tc.desc: Test JudgeSecurityHitTestForValidation delegates hit test info with default agent
 *           identity when event has no pointer event.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, JudgeSecurityHitTestForValidation002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    MockPipelineContext::SetUp();
    ResetMockSecurityHitTestInfo();
    g_webPattern->OnModifyDone();
    ASSERT_NE(g_webPattern->delegate_, nullptr);
    TouchEventInfo eventInfo(SECURITY_TEST_EVENT_TYPE);
    g_webPattern->JudgeSecurityHitTestForValidation(SECURITY_TEST_TOUCH_ID, SECURITY_TEST_TOUCH_X,
        SECURITY_TEST_TOUCH_Y, SECURITY_TEST_FROM_OVERLAY_FALSE, eventInfo);
    int32_t id = 0;
    double x = 0.0;
    double y = 0.0;
    bool fromOverlay = false;
    int32_t callingUid = 0;
    uint64_t sessionId = 0;
    std::string response;
    EXPECT_TRUE(GetRecordedSecurityHitTestInfo(id, x, y, fromOverlay, callingUid, sessionId, response));
    EXPECT_EQ(id, SECURITY_TEST_TOUCH_ID);
    EXPECT_DOUBLE_EQ(x, SECURITY_TEST_TOUCH_X);
    EXPECT_DOUBLE_EQ(y, SECURITY_TEST_TOUCH_Y);
    EXPECT_FALSE(fromOverlay);
    // event has no pointer event, agent identity falls back to defaults
    EXPECT_EQ(callingUid, SECURITY_TEST_DEFAULT_CALLING_UID);
    EXPECT_EQ(sessionId, SECURITY_TEST_DEFAULT_SESSION_ID);
    EXPECT_TRUE(response.empty());
    ResetMockSecurityHitTestInfo();
    MockPipelineContext::TearDown();
#endif
}

/**
 * @tc.name: JudgeSecurityHitTestForValidation003
 * @tc.desc: Test JudgeSecurityHitTestForValidation delegates hit test info with from overlay.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTest, JudgeSecurityHitTestForValidation003, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    MockPipelineContext::SetUp();
    ResetMockSecurityHitTestInfo();
    g_webPattern->OnModifyDone();
    ASSERT_NE(g_webPattern->delegate_, nullptr);
    TouchEventInfo eventInfo(SECURITY_TEST_EVENT_TYPE);
    g_webPattern->JudgeSecurityHitTestForValidation(SECURITY_TEST_SECOND_TOUCH_ID, SECURITY_TEST_NEG_TOUCH_X,
        SECURITY_TEST_NEG_TOUCH_Y, SECURITY_TEST_FROM_OVERLAY_TRUE, eventInfo);
    int32_t id = 0;
    double x = 0.0;
    double y = 0.0;
    bool fromOverlay = false;
    int32_t callingUid = 0;
    uint64_t sessionId = 0;
    std::string response;
    EXPECT_TRUE(GetRecordedSecurityHitTestInfo(id, x, y, fromOverlay, callingUid, sessionId, response));
    EXPECT_EQ(id, SECURITY_TEST_SECOND_TOUCH_ID);
    EXPECT_DOUBLE_EQ(x, SECURITY_TEST_NEG_TOUCH_X);
    EXPECT_DOUBLE_EQ(y, SECURITY_TEST_NEG_TOUCH_Y);
    EXPECT_TRUE(fromOverlay);
    // event has no pointer event, agent identity falls back to defaults
    EXPECT_EQ(callingUid, SECURITY_TEST_DEFAULT_CALLING_UID);
    EXPECT_EQ(sessionId, SECURITY_TEST_DEFAULT_SESSION_ID);
    EXPECT_TRUE(response.empty());
    ResetMockSecurityHitTestInfo();
    MockPipelineContext::TearDown();
#endif
}

} // namespace OHOS::Ace::NG
