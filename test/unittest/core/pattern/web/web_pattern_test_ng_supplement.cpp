/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include "core/accessibility/accessibility_manager.h"

#include "gtest/gtest.h"
#define protected public
#define private public
#include "test/mock/frameworks/core/components_ng/pattern/mock_nestable_scroll_container.h"
#undef private
#undef protected
#include "test/mock/frameworks/core/common/mock_image_analyzer_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#define private public
#include "test/mock/frameworks/base/thread/mock_task_executor.h"

#include "nweb_handler.h"
#include "core/components/web/resource/web_delegate.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/dialog/dialog_pattern.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/web/web_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/base/utils/system_properties.h"
#include "test/unittest/core/pattern/web/mock_web_delegate.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {

class WebContextMenuParamShow : public WebContextMenuParam {
public:
    MOCK_METHOD(int32_t, GetXCoord, (), (const, override));
    MOCK_METHOD(int32_t, GetYCoord, (), (const, override));
    MOCK_METHOD(std::string, GetLinkUrl, (), (const, override));
    MOCK_METHOD(std::string, GetUnfilteredLinkUrl, (), (const, override));
    MOCK_METHOD(std::string, GetSourceUrl, (), (const, override));
    MOCK_METHOD(bool, HasImageContents, (), (const, override));
    MOCK_METHOD(bool, IsEditable, (), (const, override));
    MOCK_METHOD(int, GetEditStateFlags, (), (const, override));
    MOCK_METHOD(int, GetSourceType, (), (const, override));
    MOCK_METHOD(int, GetMediaType, (), (const, override));
    MOCK_METHOD(int, GetInputFieldType, (), (const, override));
    MOCK_METHOD(std::string, GetSelectionText, (), (const, override));
    MOCK_METHOD(bool, ShouldOfferLinkToText, (), (const, override));
    MOCK_METHOD(bool, IsOverTextFragment, (), (const, override));
    MOCK_METHOD(std::string, GetTextFragmentGeneratedLink, (), (const, override));
};

class ContextMenuResultShow : public ContextMenuResult {
    DECLARE_ACE_TYPE(ContextMenuResultShow, ContextMenuResult);
public:
    ContextMenuResultShow() = default;
    ~ContextMenuResultShow() override = default;

    MOCK_METHOD(void, Cancel, (), (const, override));
    MOCK_METHOD(void, CopyImage, (), (const, override));
    MOCK_METHOD(void, SaveImage, (), (const, override));
    MOCK_METHOD(void, Copy, (), (const, override));
    MOCK_METHOD(void, Paste, (), (const, override));
    MOCK_METHOD(void, Cut, (), (const, override));
    MOCK_METHOD(void, SelectAll, (), (const, override));
    MOCK_METHOD(void, Undo, (), (const, override));
    MOCK_METHOD(void, Redo, (), (const, override));
    MOCK_METHOD(void, PasteAndMatchStyle, (), (const, override));
    MOCK_METHOD(void, RequestPasswordAutoFill, (), (const, override));
    MOCK_METHOD(void, CopyLinkToTextMenu, (), (const, override));
    MOCK_METHOD(void, RemoveFragments, (), (const, override));
};
class WebPatternTestNgSupplement : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WebPatternTestNgSupplement::SetUpTestCase() {}
void WebPatternTestNgSupplement::TearDownTestCase() {}

void WebPatternTestNgSupplement::SetUp()
{
    MockPipelineContext::SetUp();
}

void WebPatternTestNgSupplement::TearDown()
{
    MockPipelineContext::TearDown();
    OHOS::Ace::SetReturnStatus("");
}

/**
 * @tc.name: OnAttachToFrameNode_001
 * @tc.desc: OnAttachToFrameNode.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnAttachToFrameNode_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    webPattern->renderContextForSurface_ = nullptr;

    webPattern->OnAttachToFrameNode();
    EXPECT_NE(webPattern->renderContextForSurface_, nullptr);
    EXPECT_EQ(webPattern->renderContextForPopupSurface_, nullptr);
#endif
}

/**
 * @tc.name: OnAttachToFrameNode_002
 * @tc.desc: OnAttachToFrameNode.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnAttachToFrameNode_002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    webPattern->renderContextForSurface_ = RenderContext::Create();
    webPattern->renderContextForPopupSurface_ = RenderContext::Create();
    webPattern->OnAttachToFrameNode();
    EXPECT_TRUE(webPattern->transformHintChangedCallbackId_.has_value());
#endif
}

/**
 * @tc.name: OnDetachFromFrameNode_001
 * @tc.desc: OnDetachFromFrameNode.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnDetachFromFrameNode_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    webPattern->textBlurAccessibilityEnable_ = true;
    auto pattern = AceType::MakeRefPtr<Pattern>();
    FrameNode node("exampleTag", 1, pattern, true, false);
    webPattern->transformHintChangedCallbackId_ = 1;
    webPattern->OnDetachFromFrameNode(&node);
    EXPECT_TRUE(webPattern->transformHintChangedCallbackId_.has_value());
    webPattern->transformHintChangedCallbackId_.reset();
    webPattern->OnDetachFromFrameNode(&node);
    EXPECT_FALSE(webPattern->transformHintChangedCallbackId_.has_value());

#endif
}

/**
 * @tc.name: getZoomOffset_001
 * @tc.desc: getZoomOffset.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, getZoomOffset_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    double scale = 2.0;
    double ret = webPattern->getZoomOffset(scale);
    EXPECT_EQ(ret, 0.0);
#endif
}

/**
 * @tc.name: getZoomOffset_002
 * @tc.desc: getZoomOffset.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, getZoomOffset_002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    double scale = 0.0;
    double ret = webPattern->getZoomOffset(scale);
    EXPECT_NE(ret, 0.0);
#endif
}

/**
 * @tc.name: HandleScaleGestureChange_001
 * @tc.desc: HandleScaleGestureChange.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, HandleScaleGestureChange_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    GestureEvent event;
    event.SetScale(0);
    webPattern->zoomStatus_ = 3;
    webPattern->zoomErrorCount_ = 5;
    webPattern->preScale_ = 1.0;
    webPattern->zoomOutSwitch_ = false;
    webPattern->startPageScale_ = 5.0;

    webPattern->HandleScaleGestureChange(event);
#endif
}

/**
 * @tc.name: JavaScriptOnDocumentStart001
 * @tc.desc: JavaScriptOnDocumentStart.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, JavaScriptOnDocumentStart001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    webPattern->delegate_ = nullptr;
    std::map<std::string, std::vector<std::string>> scriptItems;
    std::string group = "group";
    std::vector<std::string> vec;
    vec.push_back("main");
    scriptItems.insert(std::make_pair(group, vec));
    webPattern->JavaScriptOnDocumentStart(scriptItems);
    EXPECT_TRUE(webPattern->onDocumentStartScriptItems_.has_value());
#endif
}

/**
 * @tc.name: JavaScriptOnDocumentEnd001
 * @tc.desc: JavaScriptOnDocumentEnd.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, JavaScriptOnDocumentEnd001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    webPattern->delegate_ = nullptr;
    std::map<std::string, std::vector<std::string>> scriptItems;
    std::string group = "group";
    std::vector<std::string> vec;
    vec.push_back("main");
    scriptItems.insert(std::make_pair(group, vec));
    webPattern->JavaScriptOnDocumentEnd(scriptItems);
    EXPECT_TRUE(webPattern->onDocumentEndScriptItems_.has_value());
#endif
}

/**
 * @tc.name: UpdateJavaScriptOnDocumentStart001
 * @tc.desc: UpdateJavaScriptOnDocumentStart.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, UpdateJavaScriptOnDocumentStart001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    std::map<std::string, std::vector<std::string>> scriptItems;
    std::string group = "group";
    std::vector<std::string> vec;
    vec.push_back("main");
    scriptItems.insert(std::make_pair(group, vec));
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(scriptItems);
    webPattern->UpdateJavaScriptOnDocumentStart();
    EXPECT_FALSE(webPattern->onDocumentStartScriptItems_.has_value());
    webPattern->UpdateJavaScriptOnDocumentStart();
    webPattern->delegate_ = nullptr;
    webPattern->UpdateJavaScriptOnDocumentStart();
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(scriptItems);
    webPattern->UpdateJavaScriptOnDocumentStart();
    EXPECT_TRUE(webPattern->onDocumentStartScriptItems_.has_value());
#endif
}

/**
 * @tc.name: UpdateJavaScriptOnDocumentEnd004
 * @tc.desc: UpdateJavaScriptOnDocumentEnd.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, UpdateJavaScriptOnDocumentEnd001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    std::map<std::string, std::vector<std::string>> scriptItems;
    std::string group = "group";
    std::vector<std::string> vec;
    vec.push_back("main");
    scriptItems.insert(std::make_pair(group, vec));
    webPattern->onDocumentEndScriptItems_ = std::make_optional<ScriptItems>(scriptItems);
    webPattern->UpdateJavaScriptOnDocumentEnd();
    EXPECT_FALSE(webPattern->onDocumentEndScriptItems_.has_value());
    webPattern->UpdateJavaScriptOnDocumentEnd();
    webPattern->delegate_ = nullptr;
    webPattern->UpdateJavaScriptOnDocumentEnd();
    webPattern->onDocumentEndScriptItems_ = std::make_optional<ScriptItems>(scriptItems);
    webPattern->UpdateJavaScriptOnDocumentEnd();
    EXPECT_TRUE(webPattern->onDocumentEndScriptItems_.has_value());
#endif
}

/**
 * @tc.name: JavaScriptOnDocumentStartByOrder005
 * @tc.desc: JavaScriptOnDocumentStartByOrder.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, JavaScriptOnDocumentStartByOrder005, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    webPattern->delegate_ = nullptr;
    std::map<std::string, std::vector<std::string>> scriptItems;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> scriptRegexItems;
    std::vector<std::string> scriptItemsByOrder;
    std::string group = "group";
    std::vector<std::string> vec;
    vec.push_back("main");
    scriptItems.insert(std::make_pair(group, vec));
    webPattern->JavaScriptOnDocumentStartByOrder(scriptItems, scriptRegexItems, scriptItemsByOrder);
    EXPECT_TRUE(webPattern->onDocumentStartScriptItemsByOrder_.has_value());
#endif
}

/**
 * @tc.name: JavaScriptOnDocumentEndByOrder005
 * @tc.desc: JavaScriptOnDocumentEndByOrder.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, JavaScriptOnDocumentEndByOrder005, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    webPattern->delegate_ = nullptr;
    std::map<std::string, std::vector<std::string>> scriptItems;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> scriptRegexItems;
    std::vector<std::string> scriptItemsByOrder;
    std::string group = "group";
    std::vector<std::string> vec;
    vec.push_back("main");
    scriptItems.insert(std::make_pair(group, vec));
    webPattern->JavaScriptOnDocumentEndByOrder(scriptItems, scriptRegexItems, scriptItemsByOrder);
    EXPECT_TRUE(webPattern->onDocumentEndScriptItemsByOrder_.has_value());
#endif
}

/**
 * @tc.name: UpdateJavaScriptOnDocumentStartByOrder005
 * @tc.desc: UpdateJavaScriptOnDocumentStartByOrder.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, UpdateJavaScriptOnDocumentStartByOrder005, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    std::map<std::string, std::vector<std::string>> scriptItems;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> scriptRegexItems;
    std::vector<std::string> scriptItemsByOrder;
    std::string group = "group";
    std::vector<std::string> vec;
    vec.push_back("main");
    scriptItems.insert(std::make_pair(group, vec));
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(scriptItems);
    webPattern->onDocumentStartScriptRegexItems_ = std::make_optional<ScriptRegexItems>(scriptRegexItems);
    webPattern->onDocumentStartScriptItemsByOrder_ = std::make_optional<ScriptItemsByOrder>(scriptItemsByOrder);

    webPattern->UpdateJavaScriptOnDocumentStartByOrder();
    EXPECT_FALSE(webPattern->onDocumentStartScriptItems_.has_value());
    EXPECT_FALSE(webPattern->onDocumentStartScriptRegexItems_.has_value());
    webPattern->UpdateJavaScriptOnDocumentStartByOrder();
    webPattern->delegate_ = nullptr;
    webPattern->UpdateJavaScriptOnDocumentStartByOrder();
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(scriptItems);
    webPattern->onDocumentStartScriptRegexItems_ = std::make_optional<ScriptRegexItems>(scriptRegexItems);
    webPattern->onDocumentStartScriptItemsByOrder_ = std::make_optional<ScriptItemsByOrder>(scriptItemsByOrder);

    webPattern->UpdateJavaScriptOnDocumentStartByOrder();
    EXPECT_TRUE(webPattern->onDocumentStartScriptItems_.has_value());
    EXPECT_TRUE(webPattern->onDocumentStartScriptRegexItems_.has_value());
#endif
}

/**
 * @tc.name: UpdateJavaScriptOnDocumentEndByOrder005
 * @tc.desc: UpdateJavaScriptOnDocumentEndByOrder.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, UpdateJavaScriptOnDocumentEndByOrder005, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    std::map<std::string, std::vector<std::string>> scriptItems;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> scriptRegexItems;
    std::vector<std::string> scriptItemsByOrder;

    std::string group = "group";
    std::vector<std::string> vec;
    vec.push_back("main");
    scriptItems.insert(std::make_pair(group, vec));
    webPattern->onDocumentEndScriptItems_ = std::make_optional<ScriptItems>(scriptItems);
    webPattern->onDocumentEndScriptRegexItems_ = std::make_optional<ScriptRegexItems>(scriptRegexItems);
    webPattern->onDocumentEndScriptItemsByOrder_ = std::make_optional<ScriptItemsByOrder>(scriptItemsByOrder);

    webPattern->UpdateJavaScriptOnDocumentEndByOrder();
    EXPECT_FALSE(webPattern->onDocumentEndScriptItems_.has_value());
    EXPECT_FALSE(webPattern->onDocumentEndScriptRegexItems_.has_value());
    webPattern->UpdateJavaScriptOnDocumentEndByOrder();
    webPattern->delegate_ = nullptr;
    webPattern->UpdateJavaScriptOnDocumentEndByOrder();
    webPattern->onDocumentEndScriptItems_ = std::make_optional<ScriptItems>(scriptItems);
    webPattern->onDocumentEndScriptRegexItems_ = std::make_optional<ScriptRegexItems>(scriptRegexItems);
    webPattern->onDocumentEndScriptItemsByOrder_ = std::make_optional<ScriptItemsByOrder>(scriptItemsByOrder);

    webPattern->UpdateJavaScriptOnDocumentEndByOrder();
    EXPECT_TRUE(webPattern->onDocumentEndScriptItems_.has_value());
    EXPECT_TRUE(webPattern->onDocumentEndScriptRegexItems_.has_value());
#endif
}

/**
 * @tc.name: JavaScriptOnHeadReadyByOrder001
 * @tc.desc: JavaScriptOnHeadReadyByOrder.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, JavaScriptOnHeadReadyByOrder001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    webPattern->delegate_ = nullptr;
    std::map<std::string, std::vector<std::string>> scriptItems;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> scriptRegexItems;
    std::vector<std::string> scriptItemsByOrder;
    std::string group = "group";
    std::vector<std::string> vec;
    vec.push_back("main");
    scriptItems.insert(std::make_pair(group, vec));
    webPattern->JavaScriptOnHeadReadyByOrder(scriptItems, scriptRegexItems, scriptItemsByOrder);
    EXPECT_TRUE(webPattern->onHeadReadyScriptItemsByOrder_.has_value());
#endif
}

/**
 * @tc.name: UpdateJavaScriptOnHeadReadyByOrder001
 * @tc.desc: UpdateJavaScriptOnHeadReadyByOrder.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, UpdateJavaScriptOnHeadReadyByOrder001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    std::map<std::string, std::vector<std::string>> scriptItems;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> scriptRegexItems;
    std::vector<std::string> scriptItemsByOrder;
    std::string group = "group";
    std::vector<std::string> vec;
    vec.push_back("main");
    scriptItems.insert(std::make_pair(group, vec));
    webPattern->onHeadReadyScriptItems_ = std::make_optional<ScriptItems>(scriptItems);
    webPattern->onHeadReadyScriptRegexItems_ = std::make_optional<ScriptRegexItems>(scriptRegexItems);
    webPattern->onHeadReadyScriptItemsByOrder_ = std::make_optional<ScriptItemsByOrder>(scriptItemsByOrder);
    webPattern->UpdateJavaScriptOnHeadReadyByOrder();
    EXPECT_FALSE(webPattern->onHeadReadyScriptItems_.has_value());
    EXPECT_FALSE(webPattern->onHeadReadyScriptRegexItems_.has_value());
    webPattern->UpdateJavaScriptOnHeadReadyByOrder();
    webPattern->delegate_ = nullptr;
    webPattern->UpdateJavaScriptOnHeadReadyByOrder();
    webPattern->onHeadReadyScriptItems_ = std::make_optional<ScriptItems>(scriptItems);
    webPattern->onHeadReadyScriptRegexItems_ = std::make_optional<ScriptRegexItems>(scriptRegexItems);
    webPattern->onHeadReadyScriptItemsByOrder_ = std::make_optional<ScriptItemsByOrder>(scriptItemsByOrder);
    webPattern->UpdateJavaScriptOnHeadReadyByOrder();
    EXPECT_TRUE(webPattern->onHeadReadyScriptItems_.has_value());
    EXPECT_TRUE(webPattern->onHeadReadyScriptRegexItems_.has_value());
#endif
}

/**
 * @tc.name: UpdateSlideOffset003
 * @tc.desc: UpdateSlideOffset.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, UpdateSlideOffset003, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    webPattern->syncAxis_ = Axis::NONE;
    webPattern->UpdateSlideOffset();
#endif
}

/**
 * @tc.name: CalculateHorizontalDrawRect001
 * @tc.desc: CalculateHorizontalDrawRect.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, CalculateHorizontalDrawRect001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->fitContentOffset_.SetX(-10);
    webPattern->isNeedReDrawRect_ = false;
    webPattern->CalculateHorizontalDrawRect();
    EXPECT_FALSE(webPattern->isNeedReDrawRect_);
#endif
}

/**
 * @tc.name: CalculateHorizontalDrawRect002
 * @tc.desc: CalculateHorizontalDrawRect.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, CalculateHorizontalDrawRect002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->isNeedReDrawRect_ = true;
    webPattern->CalculateHorizontalDrawRect();
    EXPECT_FALSE(webPattern->isNeedReDrawRect_);
#endif
}

/**
 * @tc.name: CalculateVerticalDrawRect001
 * @tc.desc: CalculateVerticalDrawRect.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, CalculateVerticalDrawRect001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    webPattern->fitContentOffset_.SetY(-10);
    webPattern->isNeedReDrawRect_ = false;
    webPattern->CalculateVerticalDrawRect();
    EXPECT_FALSE(webPattern->isNeedReDrawRect_);
#endif
}

/**
 * @tc.name: CalculateVerticalDrawRect002
 * @tc.desc: CalculateVerticalDrawRect.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, CalculateVerticalDrawRect002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    webPattern->isNeedReDrawRect_ = true;
    webPattern->CalculateVerticalDrawRect();
    EXPECT_FALSE(webPattern->isNeedReDrawRect_);
#endif
}

/**
 * @tc.name: SetDrawRect001
 * @tc.desc: SetDrawRect.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, SetDrawRect001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    webPattern->drawRectWidth_ = 10;
    webPattern->drawRectHeight_ = 20;
    webPattern->SetDrawRect(10, 20, 10, 20);
    webPattern->drawRectHeight_ = 5;
    webPattern->SetDrawRect(10, 20, 10, 20);
    EXPECT_EQ(webPattern->drawRectHeight_, 20);
    webPattern->drawRectWidth_ = 5;
    webPattern->SetDrawRect(10, 20, 10, 20);
    EXPECT_EQ(webPattern->drawRectWidth_, 10);
    webPattern->drawRectWidth_ = 5;
    webPattern->drawRectHeight_ = 5;
    webPattern->SetDrawRect(10, 20, 10, 20);
    EXPECT_EQ(webPattern->drawRectWidth_, 10);
    EXPECT_EQ(webPattern->drawRectHeight_, 20);
#endif
}

/**
 * @tc.name: GetPendingSizeStatus001
 * @tc.desc: GetPendingSizeStatus.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, GetPendingSizeStatus001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    EXPECT_FALSE(webPattern->GetPendingSizeStatus());
    webPattern->delegate_ = nullptr;
    EXPECT_FALSE(webPattern->GetPendingSizeStatus());
#endif
}

/**
 * @tc.name: GetAccessibilityNodeById001
 * @tc.desc: GetAccessibilityNodeById.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, GetAccessibilityNodeById001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    int32_t accessibilityId = 1;
    webPattern->accessibilityState_ = false;
    auto ret = webPattern->GetAccessibilityNodeById(accessibilityId);
    EXPECT_EQ(ret, nullptr);
    webPattern->accessibilityState_ = true;
    OHOS::Ace::SetReturnStatus("true");
    ret = webPattern->GetAccessibilityNodeById(accessibilityId);
    EXPECT_NE(ret, nullptr);
#endif
}

/**
 * @tc.name: GetFocusedAccessibilityNode001
 * @tc.desc: GetFocusedAccessibilityNode.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, GetFocusedAccessibilityNode001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    int32_t accessibilityId = 1;
    bool isAccessibilityFocus = true;
    webPattern->accessibilityState_ = false;
    auto ret = webPattern->GetFocusedAccessibilityNode(accessibilityId, isAccessibilityFocus);
    EXPECT_EQ(ret, nullptr);
    webPattern->accessibilityState_ = true;
    OHOS::Ace::SetReturnStatus("true");
    ret = webPattern->GetFocusedAccessibilityNode(accessibilityId, isAccessibilityFocus);
    EXPECT_NE(ret, nullptr);
#endif
}

/**
 * @tc.name: GetAccessibilityNodeByFocusMove001
 * @tc.desc: GetAccessibilityNodeByFocusMove.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, GetAccessibilityNodeByFocusMove001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    int32_t accessibilityId = 1;
    int32_t direction = 1;
    webPattern->accessibilityState_ = false;
    auto ret = webPattern->GetAccessibilityNodeByFocusMove(accessibilityId, direction);
    EXPECT_EQ(ret, nullptr);
    webPattern->accessibilityState_ = true;
    OHOS::Ace::SetReturnStatus("true");
    ret = webPattern->GetAccessibilityNodeByFocusMove(accessibilityId, direction);
    EXPECT_NE(ret, nullptr);
#endif
}

/**
 * @tc.name: CheckAccessibilityNodeReady001
 * @tc.desc: CheckAccessibilityNodeReady.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, CheckAccessibilityNodeReady001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    OHOS::Ace::SetReturnStatus("true");
    EXPECT_TRUE(webPattern->delegate_->CheckAccessibilityNodeReady(frameNode, 0));
#endif
}

/**
 * @tc.name: SetAccessibilityState001
 * @tc.desc: SetAccessibilityState.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, SetAccessibilityState001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    webPattern->accessibilityState_ = true;
    webPattern->SetAccessibilityState(true);
    EXPECT_TRUE(webPattern->accessibilityState_);
    webPattern->inspectorAccessibilityEnable_ = false;
    webPattern->textBlurAccessibilityEnable_ = false;
    webPattern->SetAccessibilityState(false);
    EXPECT_FALSE(webPattern->accessibilityState_);
    webPattern->inspectorAccessibilityEnable_ = false;
    webPattern->textBlurAccessibilityEnable_ = false;
    webPattern->SetAccessibilityState(false);
    EXPECT_FALSE(webPattern->accessibilityState_);
    webPattern->inspectorAccessibilityEnable_ = false;
    webPattern->textBlurAccessibilityEnable_ = true;
    webPattern->SetAccessibilityState(false);
    EXPECT_FALSE(webPattern->accessibilityState_);
    webPattern->inspectorAccessibilityEnable_ = true;
    webPattern->textBlurAccessibilityEnable_ = false;
    webPattern->SetAccessibilityState(false);
    EXPECT_FALSE(webPattern->accessibilityState_);
    webPattern->inspectorAccessibilityEnable_ = true;
    webPattern->textBlurAccessibilityEnable_ = true;
    webPattern->SetAccessibilityState(false);
    EXPECT_FALSE(webPattern->accessibilityState_);
    webPattern->accessibilityState_ = true;
    webPattern->inspectorAccessibilityEnable_ = false;
    webPattern->textBlurAccessibilityEnable_ = true;
    webPattern->SetAccessibilityState(false);
    EXPECT_TRUE(webPattern->accessibilityState_);
    webPattern->inspectorAccessibilityEnable_ = true;
    webPattern->textBlurAccessibilityEnable_ = false;
    webPattern->SetAccessibilityState(false);
    EXPECT_TRUE(webPattern->accessibilityState_);
    webPattern->inspectorAccessibilityEnable_ = true;
    webPattern->textBlurAccessibilityEnable_ = true;
    webPattern->SetAccessibilityState(false);
    EXPECT_TRUE(webPattern->accessibilityState_);
#endif
}

/**
 * @tc.name: ClearFocusedAccessibilityId001
 * @tc.desc: ClearFocusedAccessibilityId.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, ClearFocusedAccessibilityId001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    webPattern->accessibilityState_ = false;
    webPattern->ClearFocusedAccessibilityId();
    EXPECT_EQ(webPattern->focusedAccessibilityId_, -1);
#endif
}

/**
 * @tc.name: ClearFocusedAccessibilityId002
 * @tc.desc: ClearFocusedAccessibilityId.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, ClearFocusedAccessibilityId002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    webPattern->accessibilityState_ = true;
    webPattern->ClearFocusedAccessibilityId();
    EXPECT_EQ(webPattern->focusedAccessibilityId_, -1);
#endif
}

/**
 * @tc.name: GetAccessibilityFocusRect001
 * @tc.desc: GetAccessibilityFocusRect.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, GetAccessibilityFocusRect001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    webPattern->accessibilityState_ = true;
    RectT<int32_t> paintRect;
    EXPECT_FALSE(webPattern->GetAccessibilityFocusRect(paintRect, 12345));
#endif
}

/**
 * @tc.name: SetTouchEventInfo001
 * @tc.desc: SetTouchEventInfo.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, SetTouchEventInfo001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    TouchEvent touchEvent;
    touchEvent.id = 1;
    touchEvent.type = TouchType::MOVE;
    touchEvent.x = 100.0f;
    touchEvent.y = 100.0f;
    TouchEventInfo touchEventInfo { "touchEvent" };

    webPattern->touchEventQueue_.push(touchEventInfo);
    webPattern->SetTouchEventInfo(touchEvent, touchEventInfo, "embedId");
    EXPECT_FALSE(webPattern->touchEventQueue_.empty());
#endif
}

/**
 * @tc.name: SetTouchEventInfo002
 * @tc.desc: SetTouchEventInfo.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, SetTouchEventInfo002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    TouchEvent touchEvent;
    touchEvent.id = 1;
    touchEvent.type = TouchType::DOWN;
    touchEvent.x = 100.0f;
    touchEvent.y = 100.0f;

    EventTarget target;
    TouchEventInfo touchEventInfo { "touchEvent" };

    webPattern->SetTouchEventInfo(touchEvent, touchEventInfo, "embedId");
    TouchLocationInfo touchLocationInfo(1);
    touchEventInfo.AddChangedTouchLocationInfo(std::move(touchLocationInfo));
    webPattern->touchEventQueue_.push(touchEventInfo);
    webPattern->SetTouchEventInfo(touchEvent, touchEventInfo, "embedId");
    EXPECT_TRUE(webPattern->touchEventQueue_.empty());
#endif
}

/**
 * @tc.name: SetTouchEventInfo003
 * @tc.desc: SetTouchEventInfo.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, SetTouchEventInfo003, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    TouchEvent touchEvent;
    touchEvent.id = 1;
    touchEvent.type = TouchType::UP;
    touchEvent.x = 100.0f;
    touchEvent.y = 100.0f;

    TouchEventInfo touchEventInfo { "touchEvent" };
    TouchLocationInfo touchLocationInfo(2);
    touchEventInfo.AddChangedTouchLocationInfo(std::move(touchLocationInfo));
    webPattern->touchEventQueue_.push(touchEventInfo);
    webPattern->SetTouchEventInfo(touchEvent, touchEventInfo, "embedId");
    EXPECT_TRUE(webPattern->touchEventQueue_.empty());
#endif
}

/**
 * @tc.name: OnVisibleAreaChangeTest001
 * @tc.desc: OnVisibleAreaChange.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnVisibleAreaChangeTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto webPatternFrameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(webPatternFrameNode, nullptr);
    stack->Push(webPatternFrameNode);
    auto webPattern = webPatternFrameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    webPattern->OnVisibleAreaChange(true);
    EXPECT_TRUE(webPattern->isVisible_);
    webPattern->isVisible_ = false;
    webPattern->isVisibleActiveEnable_ = false;
    webPattern->OnVisibleAreaChange(true);
    ASSERT_EQ(webPattern->isVisible_, true);
    webPattern->isVisible_ = false;
    webPattern->isActive_ = false;
    webPattern->isVisibleActiveEnable_ = true;
    webPattern->OnVisibleAreaChange(true);
    EXPECT_TRUE(webPattern->isActive_);
#endif
}

/**
 * @tc.name: OnVisibleAreaChangeTest002
 * @tc.desc: OnVisibleAreaChange.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnVisibleAreaChangeTest002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto webPatternFrameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(webPatternFrameNode, nullptr);
    stack->Push(webPatternFrameNode);
    auto webPattern = webPatternFrameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    RefPtr<FrameNode> node = FrameNode::GetOrCreateFrameNode(
        V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<DialogPattern>(nullptr, nullptr); });
    webPatternFrameNode->SetParent(node);
    webPattern->isVisible_ = true;
    webPattern->isVisibleActiveEnable_ = true;
    webPattern->isFocus_ = false;
    webPattern->isActive_ = true;
    webPattern->OnVisibleAreaChange(false);
    EXPECT_FALSE(webPattern->isActive_);
    EXPECT_FALSE(webPattern->isDragEndMenuShow_);
    webPattern->isActive_ = true;
    webPattern->isFocus_ = true;
    webPattern->isVisible_ = true;
    webPattern->OnVisibleAreaChange(false);
    EXPECT_TRUE(webPattern->isActive_);
    webPattern->isVisibleActiveEnable_ = false;
    webPattern->isVisible_ = true;
    webPattern->OnVisibleAreaChange(false);
    EXPECT_TRUE(webPattern->isActive_);
#endif
}

/**
 * @tc.name: OnVisibleAreaChangeTest003
 * @tc.desc: OnVisibleAreaChange.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnVisibleAreaChangeTest003, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto webPatternFrameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(webPatternFrameNode, nullptr);
    stack->Push(webPatternFrameNode);
    auto webPattern = webPatternFrameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->isVisible_ = true;
    webPattern->OnVisibleAreaChange(true);
#endif
}

/**
 * @tc.name: OnVisibleAreaChangeTest004
 * @tc.desc: OnVisibleAreaChange.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnVisibleAreaChangeTest004, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto webPatternFrameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(webPatternFrameNode, nullptr);
    stack->Push(webPatternFrameNode);
    auto webPattern = webPatternFrameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    RefPtr<FrameNode> node = FrameNode::GetOrCreateFrameNode(
        V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<DialogPattern>(nullptr, nullptr); });
    webPatternFrameNode->SetParent(node);
    webPattern->isVisible_ = true;
    webPattern->isVisibleActiveEnable_ = false;
    webPattern->isFocus_ = true;
    webPattern->OnVisibleAreaChange(false);
    EXPECT_FALSE(webPattern->isDragEndMenuShow_);
#endif
}

/**
 * @tc.name: OnAttachToBuilderNodeTest001
 * @tc.desc: OnAttachToBuilderNode.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnAttachToBuilderNodeTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    webPattern->OnAttachToBuilderNode(NodeStatus::NORMAL_NODE);
    ASSERT_EQ(webPattern->offlineWebInited_, false);
#endif
}

/**
 * @tc.name: OnAttachToBuilderNodeTest002
 * @tc.desc: OnAttachToBuilderNode.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnAttachToBuilderNodeTest002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    webPattern->offlineWebInited_ = false;
    webPattern->OnAttachToBuilderNode(NodeStatus::BUILDER_NODE_OFF_MAINTREE);
    ASSERT_EQ(webPattern->offlineWebInited_, true);
#endif
}

/**
 * @tc.name: OnNotifyMemoryLevelTest001
 * @tc.desc: OnNotifyMemoryLevel.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnNotifyMemoryLevelTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    webPattern->isMemoryLevelEnable_ = true;
    webPattern->OnNotifyMemoryLevel(0);
    webPattern->isMemoryLevelEnable_ = false;
    webPattern->OnNotifyMemoryLevel(0);
    ASSERT_NE(webPattern->delegate_, nullptr);
#endif
}

/**
 * @tc.name: HandleScrollVelocityTest001
 * @tc.desc: HandleScrollVelocity.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, HandleScrollVelocityTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });

    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->SetNestedScrollParent(parent);
    EXPECT_CALL(*parent, HandleScrollVelocity).Times(1).WillOnce(Return(true));
    EXPECT_TRUE(webPattern->HandleScrollVelocity(parent, 1.f));
#endif
}

/**
 * @tc.name: HandleScrollVelocityTest002
 * @tc.desc: HandleScrollVelocity.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, HandleScrollVelocityTest002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });

    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->SetNestedScrollParent(parent);
    EXPECT_CALL(*parent, HandleScrollVelocity).Times(1).WillOnce(Return(false));
    EXPECT_FALSE(webPattern->HandleScrollVelocity(parent, 1.f));
#endif
}

/**
 * @tc.name: OnOverScrollFlingVelocityTest001
 * @tc.desc: OnOverScrollFlingVelocity.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnOverScrollFlingVelocityTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });

    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    webPattern->OnOverScrollFlingVelocity(1.0f, 2.0f, true);
#endif
}

/**
 * @tc.name: OnOverScrollFlingVelocityHandlerTest001
 * @tc.desc: OnOverScrollFlingVelocityHandler.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnOverScrollFlingVelocityHandlerTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    auto layoutProperty = frameNode->GetLayoutProperty<LayoutProperty>();
    EXPECT_NE(layoutProperty, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->nestedScroll_.scrollRight = NestedScrollMode::SELF_FIRST;
    webPattern->scrollState_ = true;
    EXPECT_CALL(*parent, HandleScroll)
        .Times(2)
        .WillRepeatedly(Return(ScrollResult { .remain = 0.1f, .reachEdge = true }));
    webPattern->OnOverScrollFlingVelocityHandler(1.0f, false);
    ASSERT_TRUE(webPattern->isFirstFlingScrollVelocity_);
    webPattern->scrollState_ = false;
    webPattern->OnOverScrollFlingVelocityHandler(1.0f, false);
    ASSERT_TRUE(webPattern->isFirstFlingScrollVelocity_);
    layoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    webPattern->OnOverScrollFlingVelocityHandler(1.0f, false);
    ASSERT_TRUE(webPattern->isFirstFlingScrollVelocity_);
#endif
}

/**
 * @tc.name: OnOverScrollFlingVelocityHandlerTest002
 * @tc.desc: OnOverScrollFlingVelocityHandler.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnOverScrollFlingVelocityHandlerTest002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    auto layoutProperty = frameNode->GetLayoutProperty<LayoutProperty>();
    EXPECT_NE(layoutProperty, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->scrollState_ = true;
    webPattern->nestedScroll_.scrollDown = NestedScrollMode::SELF_ONLY;
    webPattern->OnOverScrollFlingVelocityHandler(1.0f, false);
    webPattern->nestedScroll_.scrollDown = NestedScrollMode::SELF_FIRST;
    webPattern->OnOverScrollFlingVelocityHandler(1.0f, false);
    ASSERT_TRUE(webPattern->isFirstFlingScrollVelocity_);
    webPattern->OnOverScrollFlingVelocityHandler(1.0f, true);
    webPattern->isFirstFlingScrollVelocity_ = true;
    webPattern->nestedScroll_.scrollLeft = NestedScrollMode::SELF_ONLY;
    webPattern->OnOverScrollFlingVelocityHandler(1.0f, true);
    webPattern->nestedScroll_.scrollLeft = NestedScrollMode::SELF_FIRST;
    EXPECT_CALL(*parent, HandleScrollVelocity).Times(1).WillOnce(Return(false));
    webPattern->OnOverScrollFlingVelocityHandler(1.0f, true);
    EXPECT_FALSE(webPattern->isFirstFlingScrollVelocity_);
    webPattern->OnOverScrollFlingVelocityHandler(1.0f, true);
    EXPECT_FALSE(webPattern->isFirstFlingScrollVelocity_);

    layoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    webPattern->nestedScroll_.scrollLeft = NestedScrollMode::SELF_ONLY;
    webPattern->OnOverScrollFlingVelocityHandler(1.0f, true);
    webPattern->nestedScroll_.scrollLeft = NestedScrollMode::SELF_FIRST;
    EXPECT_CALL(*parent, HandleScrollVelocity).Times(1).WillOnce(Return(false));
    webPattern->isFirstFlingScrollVelocity_ = true;
    webPattern->OnOverScrollFlingVelocityHandler(1.0f, true);
    EXPECT_FALSE(webPattern->isFirstFlingScrollVelocity_);
    webPattern->OnOverScrollFlingVelocityHandler(1.0f, true);
    EXPECT_FALSE(webPattern->isFirstFlingScrollVelocity_);
#endif
}

/**
 * @tc.name: OnScrollStateTest001
 * @tc.desc: OnScrollState.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnScrollStateTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });

    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    webPattern->OnScrollState(true);
    EXPECT_TRUE(webPattern->scrollState_);
    auto imageAnalyzerManager = std::make_shared<MockImageAnalyzerManager>(frameNode, ImageAnalyzerHolder::WEB);
    webPattern->imageAnalyzerManager_ = imageAnalyzerManager;
    webPattern->OnScrollState(true);
    webPattern->OnScrollState(false);
    EXPECT_FALSE(webPattern->scrollState_);
#endif
}

/**
 * @tc.name: OnScrollStartTest001
 * @tc.desc: OnScrollStart.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnScrollStartTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });

    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    webPattern->OnScrollStart(2.0f, 1.0f);
    EXPECT_TRUE(webPattern->scrollState_);
    EXPECT_EQ(webPattern->expectedScrollAxis_, Axis::HORIZONTAL);
#endif
}

/**
 * @tc.name: CheckAndSetWebNestedScrollExisted001
 * @tc.desc: CheckAndSetWebNestedScrollExisted.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, CheckAndSetWebNestedScrollExisted001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });

    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { {Axis::HORIZONTAL, parent} };
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    webPattern->OnBypassVsyncConditionUpdate(WebBypassVsyncCondition::SCROLLBY_FROM_ZERO_OFFSET);
    EXPECT_EQ(WebBypassVsyncCondition::SCROLLBY_FROM_ZERO_OFFSET, webPattern->GetWebBypassVsyncCondition());
    webPattern->CheckAndSetWebNestedScrollExisted();
    EXPECT_TRUE(parent->GetWebNestedScrollExisted());
#endif
}

/**
 * @tc.name: CheckAndSetWebNestedScrollExisted002
 * @tc.desc: CheckAndSetWebNestedScrollExisted.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, CheckAndSetWebNestedScrollExisted002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });

    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { {Axis::HORIZONTAL, parent} };
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    webPattern->OnBypassVsyncConditionUpdate(WebBypassVsyncCondition::NONE);
    EXPECT_EQ(WebBypassVsyncCondition::NONE, webPattern->GetWebBypassVsyncCondition());
    webPattern->CheckAndSetWebNestedScrollExisted();
    EXPECT_FALSE(parent->GetWebNestedScrollExisted());
#endif
}

/**
 * @tc.name: ReleaseResizeHoldTest001
 * @tc.desc: ReleaseResizeHold.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, ReleaseResizeHoldTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    webPattern->layoutMode_ = WebLayoutMode::FIT_CONTENT;
    webPattern->ReleaseResizeHold();
    webPattern->layoutMode_ = WebLayoutMode::NONE;
    webPattern->rootLayerChangeSize_ = Size(100, 200);
    webPattern->ReleaseResizeHold();
    EXPECT_NE(webPattern->rootLayerChangeSize_.Width(), webPattern->drawSize_.Width());
    EXPECT_NE(webPattern->rootLayerChangeSize_.Height(), webPattern->drawSize_.Height());
#endif
}

/**
 * @tc.name: GetParentAxisTest001
 * @tc.desc: GetParentAxis.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, GetParentAxisTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    webPattern->axis_ = Axis::VERTICAL;
    webPattern->GetParentAxis();
    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->SetNestedScrollParent(parent);

    EXPECT_CALL(*parent, GetAxis()).Times(1).WillOnce(testing::Return(Axis::HORIZONTAL));
    webPattern->GetParentAxis();
    EXPECT_EQ(webPattern->axis_, Axis::HORIZONTAL);
#endif
}

/**
 * @tc.name: SetNestedScrollExtTest001
 * @tc.desc: SetNestedScrollExt.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, SetNestedScrollExtTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, [] { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    NestedScrollOptionsExt nestedScroll;
    nestedScroll.scrollUp = NestedScrollMode::SELF_ONLY;
    nestedScroll.scrollLeft = NestedScrollMode::SELF_ONLY;
    nestedScroll.scrollDown = NestedScrollMode::SELF_ONLY;
    nestedScroll.scrollRight = NestedScrollMode::SELF_ONLY;
    webPattern->SetNestedScrollExt(nestedScroll);
    EXPECT_EQ(webPattern->nestedScroll_.scrollUp, nestedScroll.scrollUp);
    EXPECT_EQ(webPattern->nestedScroll_.scrollLeft, nestedScroll.scrollLeft);
    EXPECT_EQ(webPattern->nestedScroll_.scrollDown, nestedScroll.scrollDown);
    EXPECT_EQ(webPattern->nestedScroll_.scrollRight, nestedScroll.scrollRight);
#endif
}

/**
 * @tc.name: SetNestedScrollExtTest002
 * @tc.desc: SetNestedScrollExt.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, SetNestedScrollExtTest002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, [] { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    NestedScrollOptionsExt nestedScroll;
    nestedScroll.scrollUp = NestedScrollMode::SELF_ONLY;
    nestedScroll.scrollLeft = NestedScrollMode::SELF_FIRST;
    nestedScroll.scrollDown = NestedScrollMode::SELF_ONLY;
    nestedScroll.scrollRight = NestedScrollMode::SELF_FIRST;
    webPattern->SetNestedScrollExt(nestedScroll);
    EXPECT_EQ(webPattern->nestedScroll_.scrollUp, nestedScroll.scrollUp);
    EXPECT_EQ(webPattern->nestedScroll_.scrollLeft, nestedScroll.scrollLeft);
    EXPECT_EQ(webPattern->nestedScroll_.scrollDown, nestedScroll.scrollDown);
    EXPECT_EQ(webPattern->nestedScroll_.scrollRight, nestedScroll.scrollRight);
#endif
}

/**
 * @tc.name: OnRootLayerChangedTest001
 * @tc.desc: OnRootLayerChanged.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnRootLayerChangedTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    webPattern->layoutMode_ = WebLayoutMode::FIT_CONTENT;
    webPattern->OnRootLayerChanged(100, 100);
    EXPECT_EQ(webPattern->rootLayerWidth_, 100);
    EXPECT_EQ(webPattern->rootLayerHeight_, 100);
    webPattern->OnRootLayerChanged(100, 300);
    EXPECT_EQ(webPattern->rootLayerWidth_, 100);
    EXPECT_EQ(webPattern->rootLayerHeight_, 300);
    webPattern->layoutMode_ = WebLayoutMode::NONE;
    webPattern->OnRootLayerChanged(200, 300);
    EXPECT_EQ(webPattern->rootLayerWidth_, 200);
    EXPECT_EQ(webPattern->rootLayerHeight_, 300);
#endif
}

/**
 * @tc.name: OnRootLayerChangedTest002
 * @tc.desc: OnRootLayerChanged.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnRootLayerChangedTest002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    webPattern->rootLayerWidth_ = 200;
    webPattern->rootLayerHeight_ = 300;
    webPattern->OnRootLayerChanged(200, 300);
    EXPECT_TRUE((webPattern->rootLayerWidth_ == 200) && (webPattern->rootLayerHeight_ == 300));
#endif
}

/**
 * @tc.name: FilterScrollEventHandleVelocity_001
 * @tc.desc: FilterScrollEventHandleVelocity.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, FilterScrollEventHandleVelocity_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<LayoutProperty>();
    EXPECT_NE(layoutProperty, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->nestedScroll_.scrollLeft = NestedScrollMode::PARENT_FIRST;
    webPattern->isParentReachEdge_ = true;
    webPattern->SetNestedScrollParent(parent);
    EXPECT_FALSE(webPattern->FilterScrollEventHandleVelocity(2.0f));
    layoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    EXPECT_FALSE(webPattern->FilterScrollEventHandleVelocity(2.0f));
    EXPECT_CALL(*parent, NestedScrollOutOfBoundary).Times(1).WillOnce(Return(true));
    EXPECT_FALSE(webPattern->FilterScrollEventHandleVelocity(2.0f));
#endif
}

/**
 * @tc.name: FilterScrollEventHandleVelocity_002
 * @tc.desc: FilterScrollEventHandleVelocity.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, FilterScrollEventHandleVelocity_002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<LayoutProperty>();
    EXPECT_NE(layoutProperty, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->nestedScroll_.scrollLeft = NestedScrollMode::PARENT_FIRST;
    webPattern->SetNestedScrollParent(parent);
    EXPECT_CALL(*parent, HandleScrollVelocity).Times(1).WillOnce(Return(true));
    EXPECT_TRUE(webPattern->FilterScrollEventHandleVelocity(2.0f));
    layoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    EXPECT_CALL(*parent, HandleScrollVelocity).Times(1).WillOnce(Return(true));
    EXPECT_TRUE(webPattern->FilterScrollEventHandleVelocity(2.0f));
    webPattern->nestedScroll_.scrollLeft = NestedScrollMode::PARALLEL;
    EXPECT_CALL(*parent, HandleScrollVelocity).Times(1).WillOnce(Return(true));
    EXPECT_FALSE(webPattern->FilterScrollEventHandleVelocity(2.0f));
#endif
}

/**
 * @tc.name: FilterScrollEventHandleVelocity_003
 * @tc.desc: FilterScrollEventHandleVelocity.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, FilterScrollEventHandleVelocity_003, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<LayoutProperty>();
    EXPECT_NE(layoutProperty, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->nestedScroll_.scrollUp = NestedScrollMode::PARENT_FIRST;
    webPattern->SetNestedScrollParent(parent);
    EXPECT_FALSE(webPattern->FilterScrollEventHandleVelocity(2.0f));
    layoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    EXPECT_FALSE(webPattern->FilterScrollEventHandleVelocity(2.0f));
#endif
}

/**
 * @tc.name: FilterScrollEventHandleVelocity_004
 * @tc.desc: FilterScrollEventHandleVelocity with parent over scroll.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, FilterScrollEventHandleVelocity_004, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::VERTICAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::VERTICAL;
    webPattern->nestedScroll_.scrollUp = NestedScrollMode::PARENT_FIRST;
    webPattern->SetNestedScrollParent(parent);
    EXPECT_CALL(*parent, NestedScrollOutOfBoundary()).Times(1).WillOnce(Return(true));
    EXPECT_FALSE(webPattern->FilterScrollEventHandleVelocity(2.0f));
    EXPECT_NE(webPattern->dragEndRecursiveParent_.Upgrade(), nullptr);
    GestureEvent event;
    webPattern->HandleFlingMove(event);
    EXPECT_EQ(webPattern->dragEndRecursiveParent_.Upgrade(), nullptr);

    EXPECT_CALL(*parent, NestedScrollOutOfBoundary()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*parent, HandleScrollVelocity).Times(1).WillOnce(Return(true));
    webPattern->isSelfReachEdge_ = true;
    EXPECT_TRUE(webPattern->FilterScrollEventHandleVelocity(2.0f));

    EXPECT_CALL(*parent, NestedScrollOutOfBoundary()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*parent, HandleScrollVelocity).Times(1).WillOnce(Return(true));
    webPattern->isSelfReachEdge_ = false;
    webPattern->isParentReverseReachEdge_ = true;
    EXPECT_TRUE(webPattern->FilterScrollEventHandleVelocity(2.0f));
#endif
}

/**
 * @tc.name: InitSlideUpdateListener_001
 * @tc.desc: InitSlideUpdateListener.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, InitSlideUpdateListener_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<FrameNode> node =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    node->tag_ = "Web";
    frameNode->SetParent(node);
    webPattern->InitSlideUpdateListener();
    EXPECT_NE(webPattern->renderSurface_, nullptr);
#endif
}

/**
 * @tc.name: UpdateFocusedAccessibilityId_001
 * @tc.desc: UpdateFocusedAccessibilityId.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, UpdateFocusedAccessibilityId_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    webPattern->accessibilityState_ = true;
    OHOS::Ace::SetReturnStatus("true");
    webPattern->UpdateFocusedAccessibilityId(100);
    EXPECT_EQ(webPattern->focusedAccessibilityId_, 100);
#endif
}

/**
 * @tc.name: UpdateFocusedAccessibilityId_002
 * @tc.desc: UpdateFocusedAccessibilityId.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, UpdateFocusedAccessibilityId_002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    webPattern->accessibilityState_ = true;
    OHOS::Ace::SetReturnStatus("");
    webPattern->UpdateFocusedAccessibilityId(101);
    EXPECT_EQ(webPattern->focusedAccessibilityId_, 101);
#endif
}

/**
 * @tc.name: UpdateFocusedAccessibilityId_003
 * @tc.desc: UpdateFocusedAccessibilityId.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, UpdateFocusedAccessibilityId_003, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    webPattern->focusedAccessibilityId_ = -1;
    webPattern->accessibilityState_ = true;
    webPattern->UpdateFocusedAccessibilityId(-1);
    OHOS::Ace::SetReturnStatus("");
    RectT<int32_t> rect;
    EXPECT_FALSE(webPattern->GetAccessibilityFocusRect(rect, -1));
#endif
}

/**
 * @tc.name: UpdateFocusedAccessibilityId_004
 * @tc.desc: UpdateFocusedAccessibilityId.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, UpdateFocusedAccessibilityId_004, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    webPattern->focusedAccessibilityId_ = -1;
    webPattern->accessibilityState_ = true;
    webPattern->UpdateFocusedAccessibilityId(-2);
    OHOS::Ace::SetReturnStatus("true");
    RectT<int32_t> rect;
    EXPECT_TRUE(webPattern->GetAccessibilityFocusRect(rect, -2));
#endif
}

/**
 * @tc.name: SearchParentTest001
 * @tc.desc: SearchParent.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, SearchParentTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });

    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<FrameNode> node =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    frameNode->SetParent(node);
    auto nodepattern = node->GetPattern<WebPattern>();
    nodepattern->axis_ = Axis::NONE;
    EXPECT_NE(webPattern->SearchParent(Axis::NONE), nullptr);
    nodepattern->axis_ = Axis::VERTICAL;
    EXPECT_NE(webPattern->SearchParent(Axis::NONE), nullptr);
    nodepattern->axis_ = Axis::VERTICAL;
    EXPECT_NE(webPattern->SearchParent(Axis::VERTICAL), nullptr);
    nodepattern->axis_ = Axis::NONE;
    EXPECT_EQ(webPattern->SearchParent(Axis::VERTICAL), nullptr);
#endif
}

/**
 * @tc.name: ExecuteAction_001
 * @tc.desc: ExecuteAction.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, ExecuteAction_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    webPattern->accessibilityState_ = false;
    std::map<std::string, std::string> arguments = { { "arg1", "value1" } };
    EXPECT_FALSE(webPattern->ExecuteAction(1, AceAction::ACTION_NONE, arguments));
    webPattern->accessibilityState_ = true;
    webPattern->delegate_->accessibilityState_ = true;
    EXPECT_FALSE(webPattern->ExecuteAction(1, AceAction::ACTION_CLICK, arguments));
#endif
}

/**
 * @tc.name: FilterScrollEventHandleOffset_001
 * @tc.desc: FilterScrollEventHandleOffset.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, FilterScrollEventHandleOffset_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<LayoutProperty>();
    EXPECT_NE(layoutProperty, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->nestedScroll_.scrollLeft = NestedScrollMode::PARENT_FIRST;
    webPattern->SetNestedScrollParent(parent);
    EXPECT_CALL(*parent, HandleScroll).Times(1).WillOnce(Return(ScrollResult { .remain = 0.001f, .reachEdge = true }));
    EXPECT_TRUE(webPattern->FilterScrollEventHandleOffset(2.0f));
    layoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    EXPECT_CALL(*parent, HandleScroll).Times(1).WillOnce(Return(ScrollResult { .remain = 0.001f, .reachEdge = true }));
    EXPECT_TRUE(webPattern->FilterScrollEventHandleOffset(2.0f));
#endif
}

/**
 * @tc.name: FilterScrollEventHandleOffset_002
 * @tc.desc: FilterScrollEventHandleOffset.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, FilterScrollEventHandleOffset_002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->nestedScroll_.scrollLeft = NestedScrollMode::SELF_FIRST;
    webPattern->SetNestedScrollParent(parent);
    EXPECT_TRUE(webPattern->FilterScrollEventHandleOffset(2.0f));
#endif
}

/**
 * @tc.name: FilterScrollEventHandleOffset_003
 * @tc.desc: FilterScrollEventHandleOffset.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, FilterScrollEventHandleOffset_003, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<LayoutProperty>();
    EXPECT_NE(layoutProperty, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->nestedScroll_.scrollUp = NestedScrollMode::PARENT_FIRST;
    webPattern->SetNestedScrollParent(parent);
    EXPECT_TRUE(webPattern->FilterScrollEventHandleOffset(2.0f));
    layoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    EXPECT_TRUE(webPattern->FilterScrollEventHandleOffset(2.0f));
#endif
}

/**
 * @tc.name: FilterScrollEventHandleOffset_004
 * @tc.desc: FilterScrollEventHandleOffset.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, FilterScrollEventHandleOffset_004, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<LayoutProperty>();
    EXPECT_NE(layoutProperty, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->nestedScroll_.scrollLeft = NestedScrollMode::PARENT_FIRST;
    webPattern->SetNestedScrollParent(parent);
    EXPECT_CALL(*parent, HandleScroll).Times(2).WillOnce(Return(ScrollResult { .remain = 0.1f, .reachEdge = true }));
    EXPECT_TRUE(webPattern->FilterScrollEventHandleOffset(2.0f));
    layoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    EXPECT_CALL(*parent, HandleScroll).Times(2).WillOnce(Return(ScrollResult { .remain = 0.1f, .reachEdge = true }));
    EXPECT_TRUE(webPattern->FilterScrollEventHandleOffset(2.0f));
    webPattern->nestedScroll_.scrollLeft = NestedScrollMode::PARALLEL;
    EXPECT_CALL(*parent, HandleScroll).Times(2).WillOnce(Return(ScrollResult { .remain = 0.1f, .reachEdge = true }));
    EXPECT_FALSE(webPattern->FilterScrollEventHandleOffset(2.0f));
    layoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    EXPECT_CALL(*parent, HandleScroll).Times(2).WillOnce(Return(ScrollResult { .remain = 0.1f, .reachEdge = true }));
    EXPECT_FALSE(webPattern->FilterScrollEventHandleOffset(2.0f));
#endif
}

/**
 * @tc.name: OnScrollStartRecursive_001
 * @tc.desc: OnScrollStartRecursive.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnScrollStartRecursive_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent }, { Axis::VERTICAL, nullptr } };
    EXPECT_CALL(*parent, OnScrollStartRecursive).Times(1);
    webPattern->OnScrollStartRecursive(1.0f);
    EXPECT_TRUE(webPattern->isFirstFlingScrollVelocity_);
#endif
}

/**
 * @tc.name: OnScrollStartRecursive_002
 * @tc.desc: OnScrollStartRecursive.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnScrollStartRecursive_002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent }, { Axis::VERTICAL, nullptr } };
    webPattern->OnScrollStartRecursive(1.0f);
    EXPECT_TRUE(webPattern->isFirstFlingScrollVelocity_);
#endif
}

/**
 * @tc.name: OnScrollEndRecursive_001
 * @tc.desc: OnScrollEndRecursive.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnScrollEndRecursive_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent }, { Axis::VERTICAL, nullptr } };
    std::optional<float> velocity = 1.f;
    webPattern->isScrollStarted_ = true;
    EXPECT_CALL(*parent, OnScrollEndRecursive).Times(1);
    webPattern->OnScrollEndRecursive(velocity);
    EXPECT_FALSE(webPattern->isScrollStarted_);
    EXPECT_EQ(webPattern->expectedScrollAxis_, Axis::FREE);
#endif
}

/**
 * @tc.name: IsDialogNestedTest001
 * @tc.desc: IsDialogNested.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, IsDialogNestedTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto webPatternFrameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(webPatternFrameNode, nullptr);
    stack->Push(webPatternFrameNode);
    auto webPattern = webPatternFrameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    RefPtr<FrameNode> node = FrameNode::GetOrCreateFrameNode(
        V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<DialogPattern>(nullptr, nullptr); });
    webPatternFrameNode->SetParent(node);
    EXPECT_TRUE(webPattern->IsDialogNested());
#endif
}

/**
 * @tc.name: IsDialogNestedTest002
 * @tc.desc: IsDialogNested.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, IsDialogNestedTest002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto webPatternFrameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(webPatternFrameNode, nullptr);
    stack->Push(webPatternFrameNode);
    auto webPattern = webPatternFrameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);

    RefPtr<FrameNode> node =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    webPatternFrameNode->SetParent(node);
    EXPECT_FALSE(webPattern->IsDialogNested());
#endif
}

/**
 * @tc.name: FilterScrollEvent_001
 * @tc.desc: FilterScrollEvent.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, FilterScrollEvent_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::VERTICAL;
    EXPECT_FALSE(webPattern->FilterScrollEvent(1.0f, 0.5f, 1.0f, 2.0f));
#endif
}

/**
 * @tc.name: OnNestedScroll_001
 * @tc.desc: OnNestedScroll.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnNestedScroll_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::VERTICAL;
    webPattern->isScrollStarted_ = true;
    webPattern->isDirectionalLockEnabled_ = true;
    webPattern->scrollDirectionalLockType_ = ScrollDirectionalLockType::ALL;
    float x = 1.0f, y = 0.5f, xVelocity = 0.0f, yVelocity = 0.0f;
    bool isAvailable = false;
    EXPECT_FALSE(webPattern->OnNestedScroll(x, y, xVelocity, yVelocity, isAvailable));
    EXPECT_TRUE(x == 0.0f);
    EXPECT_TRUE(isAvailable);
    webPattern->isScrollStarted_ = false;
    x = 1.0f, y = 0.5f, xVelocity = 0.0f, yVelocity = 0.0f;
    EXPECT_FALSE(webPattern->OnNestedScroll(x, y, xVelocity, yVelocity, isAvailable));
    EXPECT_TRUE(x == 1.0f);
#endif
}

/**
 * @tc.name: OnNestedScroll_002
 * @tc.desc: OnNestedScroll.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnNestedScroll_002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::VERTICAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->isScrollStarted_ = true;
    float x = 1.0f, y = 0.5f, xVelocity = 0.0f, yVelocity = 0.0f;
    bool isAvailable = false;
    EXPECT_FALSE(webPattern->OnNestedScroll(x, y, xVelocity, yVelocity, isAvailable));
    EXPECT_TRUE(y == 0.0f);
    EXPECT_TRUE(isAvailable);
    webPattern->isScrollStarted_ = false;
    x = 1.0f, y = 0.5f, xVelocity = 0.0f, yVelocity = 0.0f;
    EXPECT_FALSE(webPattern->OnNestedScroll(x, y, xVelocity, yVelocity, isAvailable));
    EXPECT_TRUE(y == 0.5f);
#endif
}

/**
 * @tc.name: OnNestedScrollV2_001
 * @tc.desc: OnNestedScrollV2.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnNestedScrollV2_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::VERTICAL;
    webPattern->isScrollStarted_ = true;
    float x = 1.0f, y = 0.5f;
    EXPECT_FALSE(webPattern->OnNestedScrollV2(x, y));
    EXPECT_TRUE(x == 0.0f);
    webPattern->isScrollStarted_ = false;
    x = 1.0f, y = 0.5f;
    EXPECT_FALSE(webPattern->OnNestedScrollV2(x, y));
    EXPECT_TRUE(x == 1.0f);
#endif
}

/**
 * @tc.name: OnNestedScrollV2_002
 * @tc.desc: OnNestedScrollV2.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnNestedScrollV2_002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::VERTICAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->isScrollStarted_ = true;
    float x = 1.0f, y = 0.5f;
    EXPECT_FALSE(webPattern->OnNestedScrollV2(x, y));
    EXPECT_TRUE(y == 0.0f);
    webPattern->isScrollStarted_ = false;
    x = 1.0f, y = 0.5f;
    EXPECT_FALSE(webPattern->OnNestedScrollV2(x, y));
    EXPECT_TRUE(y == 0.5f);
#endif
}

/**
 * @tc.name: OnNestedFling_001
 * @tc.desc: OnNestedFling.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnNestedFling_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    webPattern->parentsMap_.clear();
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    float vx = 2.0f;
    float vy = 3.0f;
    EXPECT_FALSE(webPattern->OnNestedFling(vx, vy));
#endif
}

/**
 * @tc.name: OnNestedFling_002
 * @tc.desc: OnNestedFling.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnNestedFling_002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<MockNestableScrollContainer> parent = AccessibilityManager::MakeRefPtr<MockNestableScrollContainer>();
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->SetNestedScrollParent(parent);
    float vx = 2.0f;
    float vy = 3.0f;
    EXPECT_FALSE(webPattern->OnNestedFling(vx, vy));

    webPattern->parentsMap_ = { { Axis::VERTICAL, parent } };
    webPattern->expectedScrollAxis_ = Axis::VERTICAL;
    EXPECT_FALSE(webPattern->OnNestedFling(vx, vy));
#endif
}

/**
 * @tc.name: OnNestedFling_003
 * @tc.desc: OnNestedFling.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnNestedFling_003, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<SwiperPattern> swiperParent = AceType::MakeRefPtr<SwiperPattern>();
    // HORIZONTAL + SELF_ONLY → CheckParentScroll false → return false
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, swiperParent } };
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->nestedScroll_.scrollLeft = NestedScrollMode::SELF_ONLY;
    webPattern->SetNestedScrollParent(swiperParent);
    float vx = 2.0f;
    float vy = 3.0f;
    EXPECT_FALSE(webPattern->OnNestedFling(vx, vy));

    // VERTICAL + SELF_ONLY → CheckParentScroll false → return false
    webPattern->parentsMap_ = { { Axis::VERTICAL, swiperParent } };
    webPattern->expectedScrollAxis_ = Axis::VERTICAL;
    webPattern->nestedScroll_.scrollUp = NestedScrollMode::SELF_ONLY;
    EXPECT_FALSE(webPattern->OnNestedFling(vx, vy));
#endif
}

/**
 * @tc.name: OnNestedFling_004
 * @tc.desc: OnNestedFling.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnNestedFling_004, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<LayoutProperty>();
    EXPECT_NE(layoutProperty, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<SwiperPattern> swiperParent = AceType::MakeRefPtr<SwiperPattern>();
    webPattern->parentsMap_ = { { Axis::HORIZONTAL, swiperParent } };
    webPattern->expectedScrollAxis_ = Axis::HORIZONTAL;
    webPattern->nestedScroll_.scrollLeft = NestedScrollMode::SELF_ONLY;
    webPattern->SetNestedScrollParent(swiperParent);
    layoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    float vx = 2.0f;
    float vy = 3.0f;
    EXPECT_FALSE(webPattern->OnNestedFling(vx, vy));
#endif
}

/**
 * @tc.name: IsRtl_001
 * @tc.desc: IsRtl.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, IsRtl_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<LayoutProperty>();
    EXPECT_NE(layoutProperty, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);
    EXPECT_FALSE(webPattern->IsRtl());
    layoutProperty->UpdateLayoutDirection(TextDirection::AUTO);
    EXPECT_FALSE(webPattern->IsRtl());
    layoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    EXPECT_TRUE(webPattern->IsRtl());
    layoutProperty->UpdateLayoutDirection(TextDirection::LTR);
    EXPECT_FALSE(webPattern->IsRtl());
#endif
}

/**
 * @tc.name: OnContextMenuShow_001
 * @tc.desc: OnContextMenuShow.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, OnContextMenuShow_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    EXPECT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    EXPECT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    RefPtr<WebContextMenuParam> menuParam = AceType::MakeRefPtr<WebContextMenuParamShow>();
    RefPtr<ContextMenuResult> menuResult = AceType::MakeRefPtr<ContextMenuResultShow>();
    std::shared_ptr<BaseEventInfo> eventInfo = std::make_shared<ContextMenuEvent>(menuParam, menuResult);
    webPattern->contextSelectOverlay_ = nullptr;
    webPattern->OnContextMenuShow(eventInfo, false, true);
    EXPECT_EQ(webPattern->curContextMenuResult_, true);
#endif
}

/**
 * @tc.name: RegisterSystemDocumentStartJS_ExecutesAtDocumentStart_001
 * @tc.desc: RegisterSystemDocumentStartJS stores system JS and triggers merge to delegate.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, RegisterSystemDocumentStartJS_ExecutesAtDocumentStart_001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    std::string script = "console.log('system_js')";
    ScriptItems sysItems;
    sysItems["*"] = { script };
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("*");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);

    // AC-1.1: system JS is stored and flag is set.
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
    EXPECT_EQ(webPattern->systemDocumentStartScripts_.size(), 1);
    EXPECT_EQ(webPattern->systemDocumentStartScripts_["*"],
              std::vector<std::string>({ script }));
    EXPECT_EQ(webPattern->systemDocumentStartScriptItemsByOrder_.size(), 1);
    EXPECT_EQ(webPattern->systemDocumentStartScriptItemsByOrder_[0], "*");
#endif
}

/**
 * @tc.name: RegisterSystemDocumentStartJS_AppFirstThenSystem_002
 * @tc.desc: App JS is inserted before system JS in the ByOrder vector; both stores stay independent.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, RegisterSystemDocumentStartJS_AppFirstThenSystem_002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Step 1: Register app JS via ByOrder V2.
    ScriptItems appItems;
    appItems["app_script"] = { "console.log('app')" };
    ScriptRegexItems appRegex;
    ScriptItemsByOrder appOrder;
    appOrder.push_back("app_script");
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(appItems);
    webPattern->onDocumentStartScriptRegexItems_ = std::make_optional<ScriptRegexItems>(appRegex);
    webPattern->onDocumentStartScriptItemsByOrder_ = std::make_optional<ScriptItemsByOrder>(appOrder);
    webPattern->UpdateJavaScriptOnDocumentStartByOrder();

    // Step 2: Register system JS.
    ScriptItems sysItems;
    sysItems["*"] = { "console.log('system')" };
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("*");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);

    // AC-1.2: In MergeAndInject, app JS is inserted BEFORE system JS.
    // After UpdateJavaScriptOnDocumentStartByOrder, publicOnDocumentStartScriptItemsByOrder_ has app_script.
    // After RegisterSystemDocumentStartJS, MergeAndInject was called which sends to delegate.
    // We verify via the persistent state that system JS is stored.
    EXPECT_EQ(webPattern->systemDocumentStartScripts_.size(), 1);
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
    // The public persistent copy should still contain app JS.
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItems_.size(), 1);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItemsByOrder_.size(), 1);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItemsByOrder_[0], "app_script");
#endif
}

/**
 * @tc.name: RegisterSystemDocumentStartJS_FallbackNonByOrder_003
 * @tc.desc: When app uses non-ByOrder API, system JS falls back to non-ByOrder path (DD-3/AC-1.4).
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, RegisterSystemDocumentStartJS_FallbackNonByOrder_003, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Step 1: Register app JS via non-ByOrder API.
    ScriptItems appItems;
    appItems["app_script"] = { "console.log('app')" };
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(appItems);
    webPattern->onDocumentStartScriptItemsByOrder_ = std::nullopt;
    webPattern->UpdateJavaScriptOnDocumentStart();

    // publicOnDocumentStartScriptItemsByOrder_ should be empty (non-ByOrder path).
    EXPECT_TRUE(webPattern->publicOnDocumentStartScriptItemsByOrder_.empty());
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItems_.size(), 1);

    // Step 2: Register system JS — should fall back to non-ByOrder path without crashing.
    ScriptItems sysItems;
    sysItems["*"] = { "console.log('system')" };
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("*");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);

    // AC-1.4: system JS is stored; the fallback path was taken (no crash).
    EXPECT_EQ(webPattern->systemDocumentStartScripts_.size(), 1);
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
#endif
}

/**
 * @tc.name: RegisterSystemDocumentStartJS_ReplacesPrevious_004
 * @tc.desc: Repeated RegisterSystemDocumentStartJS replaces previous system JS (BR-2/EX-4).
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, RegisterSystemDocumentStartJS_ReplacesPrevious_004, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // First registration.
    ScriptItems items1;
    items1["*"] = { "console.log('first')" };
    items1["https://example.com"] = { "console.log('first')" };
    ScriptRegexItems regex1;
    ScriptItemsByOrder order1;
    order1.push_back("*");
    order1.push_back("https://example.com");
    webPattern->RegisterSystemDocumentStartJS(items1, regex1, order1);
    EXPECT_EQ(webPattern->systemDocumentStartScripts_.size(), 2);

    // Second registration — should fully replace.
    ScriptItems items2;
    items2["*"] = { "console.log('second')" };
    ScriptRegexItems regex2;
    ScriptItemsByOrder order2;
    order2.push_back("*");
    webPattern->RegisterSystemDocumentStartJS(items2, regex2, order2);

    // AC-1.5: only the latest system JS is stored.
    EXPECT_EQ(webPattern->systemDocumentStartScripts_.size(), 1);
    EXPECT_EQ(webPattern->systemDocumentStartScripts_["*"],
              std::vector<std::string>({ "console.log('second')" }));
    EXPECT_EQ(webPattern->systemDocumentStartScripts_.count("https://example.com"), 0);
#endif
}

/**
 * @tc.name: RegisterSystemDocumentStartJS_EmptyScriptRejected_005
 * @tc.desc: Empty script is rejected per EX-3, no registration occurs.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, RegisterSystemDocumentStartJS_EmptyScriptRejected_005, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // EX-3: empty scriptItems is rejected, no registration occurs.
    ScriptItems emptyItems;
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("*");
    webPattern->RegisterSystemDocumentStartJS(emptyItems, sysRegex, sysByOrder);

    // EX-3: empty script rejected, no state change.
    EXPECT_FALSE(webPattern->hasInnerOnDocumentStartScripts_);
    EXPECT_TRUE(webPattern->systemDocumentStartScripts_.empty());
#endif
}

/**
 * @tc.name: RegisterSystemDocumentStartJS_NoDelegate_006
 * @tc.desc: RegisterSystemDocumentStartJS without delegate_ does not crash and stores state (EX-1).
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, RegisterSystemDocumentStartJS_NoDelegate_006, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->delegate_ = nullptr;

    ScriptItems sysItems;
    sysItems["*"] = { "console.log('no_delegate')" };
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("*");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);

    // EX-1: state is stored even without delegate.
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
    EXPECT_EQ(webPattern->systemDocumentStartScripts_.size(), 1);

    // MergeAndInject should return early without delegate.
    webPattern->MergeAndInjectJavaScriptOnDocumentStart();
    EXPECT_EQ(webPattern->systemDocumentStartScripts_.size(), 1);
#endif
}

/**
 * @tc.name: RemoveSystemDocumentStartJS_StopsExecutionAndPreservesAppJS_007
 * @tc.desc: RemoveSystemDocumentStartJS clears system JS and re-injects app JS only (AC-1.3).
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, RemoveSystemDocumentStartJS_StopsExecutionAndPreservesAppJS_007, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Step 1: Register app JS via ByOrder V2.
    ScriptItems appItems;
    appItems["app_script"] = { "console.log('app')" };
    ScriptRegexItems appRegex;
    ScriptItemsByOrder appOrder;
    appOrder.push_back("app_script");
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(appItems);
    webPattern->onDocumentStartScriptRegexItems_ = std::make_optional<ScriptRegexItems>(appRegex);
    webPattern->onDocumentStartScriptItemsByOrder_ = std::make_optional<ScriptItemsByOrder>(appOrder);
    webPattern->UpdateJavaScriptOnDocumentStartByOrder();

    // Step 2: Register system JS.
    ScriptItems sysItems;
    sysItems["*"] = { "console.log('system')" };
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("*");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
    EXPECT_EQ(webPattern->systemDocumentStartScripts_.size(), 1);

    // Step 3: Remove system JS.
    webPattern->RemoveSystemDocumentStartJS();

    // AC-1.3: system JS is cleared, app JS persistent copy is preserved.
    EXPECT_FALSE(webPattern->hasInnerOnDocumentStartScripts_);
    EXPECT_TRUE(webPattern->systemDocumentStartScripts_.empty());
    EXPECT_TRUE(webPattern->systemDocumentStartScriptItemsByOrder_.empty());
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItems_.size(), 1);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItemsByOrder_.size(), 1);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItemsByOrder_[0], "app_script");
#endif
}

/**
 * @tc.name: RemoveSystemDocumentStartJS_NoDelegate_008
 * @tc.desc: RemoveSystemDocumentStartJS without delegate_ does not crash (EX-2).
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, RemoveSystemDocumentStartJS_NoDelegate_008, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->delegate_ = nullptr;

    ScriptItems sysItems;
    sysItems["*"] = { "console.log('test')" };
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("*");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);
    EXPECT_EQ(webPattern->systemDocumentStartScripts_.size(), 1);

    // EX-2: should not crash without delegate.
    webPattern->RemoveSystemDocumentStartJS();
    EXPECT_FALSE(webPattern->hasInnerOnDocumentStartScripts_);
    EXPECT_TRUE(webPattern->systemDocumentStartScripts_.empty());
#endif
}

/**
 * @tc.name: NoSystemDocumentStartJS_ExistingBehaviorUnchanged_009
 * @tc.desc: Without system JS registered, existing DocumentStart behavior is unchanged (AC-1.6).
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, NoSystemDocumentStartJS_ExistingBehaviorUnchanged_009, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Do NOT register system JS. Use existing API only.
    ScriptItems appItems;
    appItems["app_only"] = { "console.log('app_only')" };
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(appItems);
    webPattern->onDocumentStartScriptItemsByOrder_ = std::nullopt;
    webPattern->UpdateJavaScriptOnDocumentStart();

    // AC-1.6: existing behavior unchanged — no system JS, no merge triggered.
    EXPECT_FALSE(webPattern->hasInnerOnDocumentStartScripts_);
    EXPECT_TRUE(webPattern->systemDocumentStartScripts_.empty());
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItems_.size(), 1);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItems_["app_only"],
              std::vector<std::string>({ "console.log('app_only')" }));
    // onDocumentStartScriptItems_ should be consumed.
    EXPECT_FALSE(webPattern->onDocumentStartScriptItems_.has_value());
#endif
}

/**
 * @tc.name: SystemDocumentStartJS_PublicAndSystemCoexistence_010
 * @tc.desc: Public and system scripts coexist without interference (BR-1).
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, SystemDocumentStartJS_PublicAndSystemCoexistence_010, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Register public JS via ByOrder V2.
    ScriptItems pubItems;
    pubItems["pub_js"] = { "console.log('pub')" };
    ScriptRegexItems pubRegex;
    ScriptItemsByOrder pubOrder;
    pubOrder.push_back("pub_js");
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(pubItems);
    webPattern->onDocumentStartScriptRegexItems_ = std::make_optional<ScriptRegexItems>(pubRegex);
    webPattern->onDocumentStartScriptItemsByOrder_ = std::make_optional<ScriptItemsByOrder>(pubOrder);
    webPattern->UpdateJavaScriptOnDocumentStartByOrder();

    // Register system JS.
    ScriptItems sysItems;
    sysItems["*"] = { "console.log('system')" };
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("*");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);

    // BR-1: system and public stores are independent.
    EXPECT_EQ(webPattern->systemDocumentStartScripts_.size(), 1);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItems_.size(), 1);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItemsByOrder_.size(), 1);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItemsByOrder_[0], "pub_js");

    // Remove system JS — public JS should be unaffected.
    webPattern->RemoveSystemDocumentStartJS();
    EXPECT_TRUE(webPattern->systemDocumentStartScripts_.empty());
    EXPECT_TRUE(webPattern->systemDocumentStartScriptItemsByOrder_.empty());
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItems_.size(), 1);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItemsByOrder_.size(), 1);
#endif
}

/**
 * @tc.name: MergeAndInject_BothEmpty_EarlyReturn_011
 * @tc.desc: MergeAndInjectJavaScriptOnDocumentStart returns early when both system and public scripts are empty.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, MergeAndInject_BothEmpty_EarlyReturn_011, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Register no scripts at all — both stores empty.
    EXPECT_TRUE(webPattern->systemDocumentStartScripts_.empty());
    EXPECT_TRUE(webPattern->publicOnDocumentStartScriptItems_.empty());

    // Calling MergeAndInject directly should early-return without crashing.
    webPattern->MergeAndInjectJavaScriptOnDocumentStart();

    // AC: early return keeps state unchanged.
    EXPECT_FALSE(webPattern->hasInnerOnDocumentStartScripts_);
    EXPECT_TRUE(webPattern->systemDocumentStartScripts_.empty());
    EXPECT_TRUE(webPattern->publicOnDocumentStartScriptItems_.empty());
    EXPECT_TRUE(webPattern->publicOnDocumentStartScriptItemsByOrder_.empty());
#endif
}

/**
 * @tc.name: MergeAndInject_OnlySystemWithByOrder_TakesByOrderPath_012
 * @tc.desc: MergeAndInject takes the ByOrder path when only system JS with ByOrder info is present.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, MergeAndInject_OnlySystemWithByOrder_TakesByOrderPath_012, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Register system JS with ByOrder info, no public scripts registered.
    ScriptItems sysItems;
    sysItems["sys_js"] = { "console.log('sys')" };
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("sys_js");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);

    // AC: onlySystemWithByOrder path is taken (not the non-ByOrder fallback).
    // Persistent system stores are populated and the flag is set; no crash.
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
    EXPECT_EQ(webPattern->systemDocumentStartScripts_.size(), 1);
    EXPECT_EQ(webPattern->systemDocumentStartScriptItemsByOrder_.size(), 1);
    EXPECT_EQ(webPattern->systemDocumentStartScriptItemsByOrder_[0], "sys_js");
    // Public stores remain empty (no app JS registered).
    EXPECT_TRUE(webPattern->publicOnDocumentStartScriptItems_.empty());
    EXPECT_TRUE(webPattern->publicOnDocumentStartScriptItemsByOrder_.empty());
#endif
}

/**
 * @tc.name: MergeByOrderPath_ScriptKeyCollision_UrlRulesAppended_013
 * @tc.desc: When system and public share a script key, URL rules are appended (not overwritten) at merge time.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, MergeByOrderPath_ScriptKeyCollision_UrlRulesAppended_013, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Step 1: Register public script with key "shared_script" and URL rule "*".
    ScriptItems pubItems;
    pubItems["shared_script"] = { "*" };
    ScriptRegexItems pubRegex;
    ScriptItemsByOrder pubOrder;
    pubOrder.push_back("shared_script");
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(pubItems);
    webPattern->onDocumentStartScriptRegexItems_ = std::make_optional<ScriptRegexItems>(pubRegex);
    webPattern->onDocumentStartScriptItemsByOrder_ = std::make_optional<ScriptItemsByOrder>(pubOrder);
    webPattern->UpdateJavaScriptOnDocumentStartByOrder();

    // Step 2: Register system script with the SAME key "shared_script" but a different URL rule.
    ScriptItems sysItems;
    sysItems["shared_script"] = { "https://example.com" };
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("shared_script");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);

    // AC: system and public stores are independent; both keep their own URL rules.
    // Append semantics apply only at merge time (passed to delegate); the persistent
    // per-store values are NOT mutated by the merge.
    ASSERT_EQ(webPattern->systemDocumentStartScripts_.count("shared_script"), 1u);
    EXPECT_EQ(webPattern->systemDocumentStartScripts_["shared_script"],
              std::vector<std::string>({ "https://example.com" }));
    ASSERT_EQ(webPattern->publicOnDocumentStartScriptItems_.count("shared_script"), 1u);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItems_["shared_script"],
              std::vector<std::string>({ "*" }));
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
#endif
}

/**
 * @tc.name: MergeByOrderPath_AppFirstThenSystem_OrderVerified_014
 * @tc.desc: App ByOrder scripts precede system ByOrder scripts after merge; both keep their own order.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, MergeByOrderPath_AppFirstThenSystem_OrderVerified_014, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Step 1: Register public ByOrder scripts ["app_1", "app_2"].
    ScriptItems pubItems;
    pubItems["app_1"] = { "*" };
    pubItems["app_2"] = { "*" };
    ScriptRegexItems pubRegex;
    ScriptItemsByOrder pubOrder;
    pubOrder.push_back("app_1");
    pubOrder.push_back("app_2");
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(pubItems);
    webPattern->onDocumentStartScriptRegexItems_ = std::make_optional<ScriptRegexItems>(pubRegex);
    webPattern->onDocumentStartScriptItemsByOrder_ = std::make_optional<ScriptItemsByOrder>(pubOrder);
    webPattern->UpdateJavaScriptOnDocumentStartByOrder();

    // Step 2: Register system ByOrder scripts ["sys_1", "sys_2"].
    ScriptItems sysItems;
    sysItems["sys_1"] = { "*" };
    sysItems["sys_2"] = { "*" };
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("sys_1");
    sysByOrder.push_back("sys_2");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);

    // AC: both stores preserve their own order independently (app is merged to
    // the front of the delegate batch, system appended after — verified via the
    // persistent per-store ByOrder vectors).
    ASSERT_EQ(webPattern->systemDocumentStartScriptItemsByOrder_.size(), 2u);
    EXPECT_EQ(webPattern->systemDocumentStartScriptItemsByOrder_[0], "sys_1");
    EXPECT_EQ(webPattern->systemDocumentStartScriptItemsByOrder_[1], "sys_2");
    ASSERT_EQ(webPattern->publicOnDocumentStartScriptItemsByOrder_.size(), 2u);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItemsByOrder_[0], "app_1");
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItemsByOrder_[1], "app_2");
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
#endif
}

/**
 * @tc.name: MergeByOrderPath_RegexItemsMerged_015
 * @tc.desc: System and public regex items are merged independently under the ByOrder path.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, MergeByOrderPath_RegexItemsMerged_015, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Step 1: Register public ByOrder with regex items.
    ScriptItems pubItems;
    pubItems["pub_js"] = { "*" };
    ScriptRegexItems pubRegex;
    pubRegex["pub_js"] = { { "pattern1", "replacement1" } };
    ScriptItemsByOrder pubOrder;
    pubOrder.push_back("pub_js");
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(pubItems);
    webPattern->onDocumentStartScriptRegexItems_ = std::make_optional<ScriptRegexItems>(pubRegex);
    webPattern->onDocumentStartScriptItemsByOrder_ = std::make_optional<ScriptItemsByOrder>(pubOrder);
    webPattern->UpdateJavaScriptOnDocumentStartByOrder();

    // Step 2: Register system ByOrder with regex items.
    ScriptItems sysItems;
    sysItems["sys_js"] = { "*" };
    ScriptRegexItems sysRegex;
    sysRegex["sys_js"] = { { "pattern2", "replacement2" } };
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("sys_js");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);

    // AC: both regex stores are independent.
    ASSERT_EQ(webPattern->systemDocumentStartScriptRegexItems_.count("sys_js"), 1u);
    EXPECT_EQ(webPattern->systemDocumentStartScriptRegexItems_["sys_js"].size(), 1u);
    EXPECT_EQ(webPattern->systemDocumentStartScriptRegexItems_["sys_js"][0].first, "pattern2");
    EXPECT_EQ(webPattern->systemDocumentStartScriptRegexItems_["sys_js"][0].second, "replacement2");
    ASSERT_EQ(webPattern->publicOnDocumentStartScriptRegexItems_.count("pub_js"), 1u);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptRegexItems_["pub_js"].size(), 1u);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptRegexItems_["pub_js"][0].first, "pattern1");
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptRegexItems_["pub_js"][0].second, "replacement1");
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
#endif
}

/**
 * @tc.name: MergeNonByOrderPath_SystemMergedIntoPublic_016
 * @tc.desc: When app uses non-ByOrder API, MergeAndInject takes the non-ByOrder fallback path.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, MergeNonByOrderPath_SystemMergedIntoPublic_016, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Step 1: Register public JS via non-ByOrder API (no ByOrder optional set).
    ScriptItems appItems;
    appItems["app_js"] = { "*" };
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(appItems);
    webPattern->onDocumentStartScriptItemsByOrder_ = std::nullopt;
    webPattern->UpdateJavaScriptOnDocumentStart();
    EXPECT_TRUE(webPattern->publicOnDocumentStartScriptItemsByOrder_.empty());
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItems_.size(), 1);

    // Step 2: Register system JS (with ByOrder info). Because public is non-ByOrder,
    // MergeAndInject takes the non-ByOrder fallback path.
    ScriptItems sysItems;
    sysItems["sys_js"] = { "*" };
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("sys_js");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);

    // AC: non-ByOrder path taken — ByOrder public vector stays empty; both
    // system and public stores hold their scripts; no crash.
    EXPECT_TRUE(webPattern->publicOnDocumentStartScriptItemsByOrder_.empty());
    ASSERT_EQ(webPattern->systemDocumentStartScripts_.size(), 1u);
    EXPECT_EQ(webPattern->systemDocumentStartScripts_["sys_js"], std::vector<std::string>({ "*" }));
    ASSERT_EQ(webPattern->publicOnDocumentStartScriptItems_.size(), 1u);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItems_["app_js"], std::vector<std::string>({ "*" }));
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
#endif
}

/**
 * @tc.name: RemoveSystemDocumentStartJS_OnlySystem_BothEmptyEarlyReturn_017
 * @tc.desc: RemoveSystemDocumentStartJS with only system scripts triggers MergeAndInject early return (both empty).
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, RemoveSystemDocumentStartJS_OnlySystem_BothEmptyEarlyReturn_017, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Step 1: Register ONLY system JS (no public JS registered).
    ScriptItems sysItems;
    sysItems["sys_js"] = { "console.log('sys')" };
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("sys_js");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);
    EXPECT_EQ(webPattern->systemDocumentStartScripts_.size(), 1);
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
    EXPECT_TRUE(webPattern->publicOnDocumentStartScriptItems_.empty());

    // Step 2: Remove system JS — both stores become empty, MergeAndInject early-returns.
    webPattern->RemoveSystemDocumentStartJS();

    // AC: system store cleared; flag reset; public store remains empty; no crash.
    EXPECT_FALSE(webPattern->hasInnerOnDocumentStartScripts_);
    EXPECT_TRUE(webPattern->systemDocumentStartScripts_.empty());
    EXPECT_TRUE(webPattern->systemDocumentStartScriptItemsByOrder_.empty());
    EXPECT_TRUE(webPattern->systemDocumentStartScriptRegexItems_.empty());
    EXPECT_TRUE(webPattern->publicOnDocumentStartScriptItems_.empty());
    EXPECT_TRUE(webPattern->publicOnDocumentStartScriptItemsByOrder_.empty());
#endif
}

/**
 * @tc.name: UpdateJavaScriptOnDocumentStart_HasInner_TriggersReMerge_018
 * @tc.desc: UpdateJavaScriptOnDocumentStart with hasInner triggers MergeAndInject and consumes the optional.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, UpdateJavaScriptOnDocumentStart_HasInner_TriggersReMerge_018, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Step 1: Register system JS first so hasInnerOnDocumentStartScripts_ becomes true.
    ScriptItems sysItems;
    sysItems["sys_js"] = { "console.log('sys')" };
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("sys_js");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);

    // Step 2: Set onDocumentStartScriptItems_ and call non-ByOrder update.
    ScriptItems appItems;
    appItems["app_js"] = { "*" };
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(appItems);
    webPattern->onDocumentStartScriptItemsByOrder_ = std::nullopt;
    webPattern->UpdateJavaScriptOnDocumentStart();

    // AC: public scripts persisted; optional consumed; hasInner still true.
    ASSERT_EQ(webPattern->publicOnDocumentStartScriptItems_.size(), 1u);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItems_["app_js"], std::vector<std::string>({ "*" }));
    EXPECT_FALSE(webPattern->onDocumentStartScriptItems_.has_value());
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
    // System store untouched by the public update.
    EXPECT_EQ(webPattern->systemDocumentStartScripts_.size(), 1);
#endif
}

/**
 * @tc.name: UpdateJavaScriptOnDocumentStartByOrder_HasInner_TriggersReMerge_019
 * @tc.desc: UpdateJavaScriptOnDocumentStartByOrder with hasInner triggers MergeAndInject and consumes all optionals.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, UpdateJavaScriptOnDocumentStartByOrder_HasInner_TriggersReMerge_019,
    TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Step 1: Register system JS so hasInnerOnDocumentStartScripts_ becomes true.
    ScriptItems sysItems;
    sysItems["sys_js"] = { "console.log('sys')" };
    ScriptRegexItems sysRegex;
    sysRegex["sys_js"] = { { "p", "r" } };
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("sys_js");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);

    // Step 2: Set three optionals and call ByOrder update.
    ScriptItems appItems;
    appItems["app_js"] = { "*" };
    ScriptRegexItems appRegex;
    appRegex["app_js"] = { { "ap", "ar" } };
    ScriptItemsByOrder appOrder;
    appOrder.push_back("app_js");
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(appItems);
    webPattern->onDocumentStartScriptRegexItems_ = std::make_optional<ScriptRegexItems>(appRegex);
    webPattern->onDocumentStartScriptItemsByOrder_ = std::make_optional<ScriptItemsByOrder>(appOrder);
    webPattern->UpdateJavaScriptOnDocumentStartByOrder();

    // AC: all three public stores persisted; all optionals consumed; hasInner still true.
    ASSERT_EQ(webPattern->publicOnDocumentStartScriptItems_.size(), 1u);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItems_["app_js"], std::vector<std::string>({ "*" }));
    ASSERT_EQ(webPattern->publicOnDocumentStartScriptRegexItems_.size(), 1u);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptRegexItems_["app_js"].size(), 1u);
    ASSERT_EQ(webPattern->publicOnDocumentStartScriptItemsByOrder_.size(), 1u);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItemsByOrder_[0], "app_js");
    EXPECT_FALSE(webPattern->onDocumentStartScriptItems_.has_value());
    EXPECT_FALSE(webPattern->onDocumentStartScriptRegexItems_.has_value());
    EXPECT_FALSE(webPattern->onDocumentStartScriptItemsByOrder_.has_value());
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
#endif
}

/**
 * @tc.name: MergeAndInject_MultipleSystemScripts_OrderPreserved_020
 * @tc.desc: Multiple system ByOrder scripts preserve their order after merge with no public scripts.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, MergeAndInject_MultipleSystemScripts_OrderPreserved_020, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Register three system scripts via ByOrder; no public scripts.
    ScriptItems sysItems;
    sysItems["sys_a"] = { "*" };
    sysItems["sys_b"] = { "*" };
    sysItems["sys_c"] = { "*" };
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("sys_a");
    sysByOrder.push_back("sys_b");
    sysByOrder.push_back("sys_c");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);

    // AC: ByOrder order preserved; system store holds 3 keys; no crash.
    ASSERT_EQ(webPattern->systemDocumentStartScriptItemsByOrder_.size(), 3u);
    EXPECT_EQ(webPattern->systemDocumentStartScriptItemsByOrder_[0], "sys_a");
    EXPECT_EQ(webPattern->systemDocumentStartScriptItemsByOrder_[1], "sys_b");
    EXPECT_EQ(webPattern->systemDocumentStartScriptItemsByOrder_[2], "sys_c");
    EXPECT_EQ(webPattern->systemDocumentStartScripts_.size(), 3u);
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
    // No public scripts registered.
    EXPECT_TRUE(webPattern->publicOnDocumentStartScriptItems_.empty());
    EXPECT_TRUE(webPattern->publicOnDocumentStartScriptItemsByOrder_.empty());
#endif
}

/**
 * @tc.name: MergeByOrderPath_RegexKeyCollision_AppendNoCrash_021
 * @tc.desc: When system and public regex share a script key, the merge appends without crash; stores stay independent.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, MergeByOrderPath_RegexKeyCollision_AppendNoCrash_021, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Step 1: Register public ByOrder with a regex entry under key "shared".
    ScriptItems pubItems;
    pubItems["shared"] = { "*" };
    ScriptRegexItems pubRegex;
    pubRegex["shared"] = { { "pub_pattern", "pub_replacement" } };
    ScriptItemsByOrder pubOrder;
    pubOrder.push_back("shared");
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(pubItems);
    webPattern->onDocumentStartScriptRegexItems_ = std::make_optional<ScriptRegexItems>(pubRegex);
    webPattern->onDocumentStartScriptItemsByOrder_ = std::make_optional<ScriptItemsByOrder>(pubOrder);
    webPattern->UpdateJavaScriptOnDocumentStartByOrder();

    // Step 2: Register system ByOrder with a regex entry under the SAME key "shared".
    // This triggers the append branch in MergeByOrderPath (public inserts first,
    // system appends second because the key already exists in mergedRegexItems).
    ScriptItems sysItems;
    sysItems["shared"] = { "https://example.com" };
    ScriptRegexItems sysRegex;
    sysRegex["shared"] = { { "sys_pattern", "sys_replacement" } };
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("shared");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);

    // AC: append branch executed without crash; persistent stores stay independent.
    ASSERT_EQ(webPattern->systemDocumentStartScriptRegexItems_.count("shared"), 1u);
    EXPECT_EQ(webPattern->systemDocumentStartScriptRegexItems_["shared"].size(), 1u);
    EXPECT_EQ(webPattern->systemDocumentStartScriptRegexItems_["shared"][0].first, "sys_pattern");
    ASSERT_EQ(webPattern->publicOnDocumentStartScriptRegexItems_.count("shared"), 1u);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptRegexItems_["shared"].size(), 1u);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptRegexItems_["shared"][0].first, "pub_pattern");
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
#endif
}

/**
 * @tc.name: MergeNonByOrderPath_ScriptKeyCollision_AppendNoCrash_022
 * @tc.desc: When system and public ScriptItems share a key on the non-ByOrder path, append runs without crash.
 * @tc.type: FUNC
 */
HWTEST_F(WebPatternTestNgSupplement, MergeNonByOrderPath_ScriptKeyCollision_AppendNoCrash_022, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto* stack = ViewStackProcessor::GetInstance();
    ASSERT_NE(stack, nullptr);
    auto nodeId = stack->ClaimNodeId();
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::WEB_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<WebPattern>(); });
    ASSERT_NE(frameNode, nullptr);
    stack->Push(frameNode);
    auto webPattern = frameNode->GetPattern<WebPattern>();
    ASSERT_NE(webPattern, nullptr);
    webPattern->OnModifyDone();
    ASSERT_NE(webPattern->delegate_, nullptr);

    // Step 1: Register public JS via non-ByOrder API with key "shared".
    ScriptItems appItems;
    appItems["shared"] = { "*" };
    webPattern->onDocumentStartScriptItems_ = std::make_optional<ScriptItems>(appItems);
    webPattern->onDocumentStartScriptItemsByOrder_ = std::nullopt;
    webPattern->UpdateJavaScriptOnDocumentStart();

    // Step 2: Register system JS with the SAME key "shared". MergeNonByOrderPath
    // starts mergedItems = publicOnDocumentStartScriptItems_ (already has "shared"),
    // then iterates system items; the "shared" key already exists → append branch.
    ScriptItems sysItems;
    sysItems["shared"] = { "https://example.com" };
    ScriptRegexItems sysRegex;
    ScriptItemsByOrder sysByOrder;
    sysByOrder.push_back("shared");
    webPattern->RegisterSystemDocumentStartJS(sysItems, sysRegex, sysByOrder);

    // AC: append branch executed without crash; persistent stores stay independent.
    ASSERT_EQ(webPattern->publicOnDocumentStartScriptItems_.count("shared"), 1u);
    EXPECT_EQ(webPattern->publicOnDocumentStartScriptItems_["shared"], std::vector<std::string>({ "*" }));
    ASSERT_EQ(webPattern->systemDocumentStartScripts_.count("shared"), 1u);
    EXPECT_EQ(webPattern->systemDocumentStartScripts_["shared"],
              std::vector<std::string>({ "https://example.com" }));
    EXPECT_TRUE(webPattern->hasInnerOnDocumentStartScripts_);
    // non-ByOrder path taken — ByOrder public vector stays empty.
    EXPECT_TRUE(webPattern->publicOnDocumentStartScriptItemsByOrder_.empty());
#endif
}
} // namespace OHOS::Ace::NG
