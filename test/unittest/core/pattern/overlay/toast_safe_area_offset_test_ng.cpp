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

// TDD coverage for the center-alignment clamping branch added to
// ToastPattern::GetOffsetX (popup/dialog/toast unified avoidance fix). In
// landscape the toast must be centered within the full window but clamped to
// the safe area so it never overlaps the cutout / gesture bar. Non-center
// alignments must pick up the wrapperRect_.Left() shift (the asymmetry fix vs.
// the Y axis, which already added wrapperRect_.Top()).

#include "gtest/gtest.h"

#include "ui/base/geometry/dimension.h"
#include "ui/base/geometry/ng/size_t.h"

#define private public
#define protected public
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/geometry/dimension.h"
#include "base/geometry/rect.h" // OHOS::Ace::Rect (type of ToastPattern::wrapperRect_)
#include "core/pipeline/base/element_register.h"
#include "core/components/common/properties/alignment.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/geometry_node.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/toast/toast_layout_property.h"
#include "core/components_ng/pattern/toast/toast_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
constexpr float TEXT_WIDTH = 200.0f;
constexpr float TEXT_HEIGHT = 50.0f;
// Landscape wrapper rect: safe left=50, width=620 => safe right edge at 670.
constexpr double WRAPPER_LEFT = 50.0;
constexpr double WRAPPER_TOP = 0.0;
constexpr double WRAPPER_WIDTH = 620.0;
constexpr double WRAPPER_HEIGHT = 1000.0;
} // namespace

class ToastSafeAreaOffsetTestNg : public testing::Test {
public:
    static void SetUpTestCase()
    {
        MockPipelineContext::SetUp();
        MockContainer::SetUp();
        MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
        MockContainer::Current()->pipelineContext_ = MockPipelineContext::GetCurrentContext();
        MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    }

    static void TearDownTestCase()
    {
        MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }
};

/**
 * @tc.name: ToastGetOffsetX001
 * @tc.desc: CENTER alignment, full window wider than safe area so the natural
 *           center (900) exceeds maxLeft (safeRight - textWidth = 470): the new
 *           isCenterAlignment clamp must pin the result to 470.
 * @tc.type: FUNC
 */
HWTEST_F(ToastSafeAreaOffsetTestNg, ToastGetOffsetX001, TestSize.Level1)
{
    auto toastId = ElementRegister::GetInstance()->MakeUniqueId();
    auto toastNode =
        FrameNode::CreateFrameNode(V2::TOAST_ETS_TAG, toastId, AceType::MakeRefPtr<ToastPattern>());
    ASSERT_NE(toastNode, nullptr);
    auto pattern = toastNode->GetPattern<ToastPattern>();
    ASSERT_NE(pattern, nullptr);
    auto toastProp = toastNode->GetLayoutProperty<ToastLayoutProperty>();
    ASSERT_NE(toastProp, nullptr);
    toastProp->UpdateToastAlignment(Alignment::CENTER);
    toastProp->ResetToastOffset();

    auto textId = ElementRegister::GetInstance()->MakeUniqueId();
    auto textNode =
        FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, textId, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode, nullptr);
    textNode->GetGeometryNode()->SetFrameSize(SizeF(TEXT_WIDTH, TEXT_HEIGHT));

    auto hostWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        toastNode, toastNode->GetGeometryNode(), toastNode->GetLayoutProperty());
    auto textWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        textNode, textNode->GetGeometryNode(), textNode->GetLayoutProperty());
    hostWrapper->AppendChild(textWrapper);

    pattern->wrapperRect_ = Rect(WRAPPER_LEFT, WRAPPER_TOP, WRAPPER_WIDTH, WRAPPER_HEIGHT);
    pattern->wrapperFullWidth_ = 2000.0f; // full window much wider than safe area
    pattern->wrapperFullHeight_ = static_cast<float>(WRAPPER_HEIGHT);

    auto offsetX = pattern->GetOffsetX(hostWrapper);
    // centered = (2000 - 200)/2 = 900; maxLeft = max(50, 670-200) = 470;
    // clamp(900, 50, 470) = 470.
    EXPECT_FLOAT_EQ(static_cast<float>(offsetX.ConvertToPx()), 470.0f);
}

/**
 * @tc.name: ToastGetOffsetX002
 * @tc.desc: CENTER alignment, full window narrower so the natural center (0)
 *           falls below safeLeft (50): the clamp must pin the result to 50.
 * @tc.type: FUNC
 */
HWTEST_F(ToastSafeAreaOffsetTestNg, ToastGetOffsetX002, TestSize.Level1)
{
    auto toastId = ElementRegister::GetInstance()->MakeUniqueId();
    auto toastNode =
        FrameNode::CreateFrameNode(V2::TOAST_ETS_TAG, toastId, AceType::MakeRefPtr<ToastPattern>());
    ASSERT_NE(toastNode, nullptr);
    auto pattern = toastNode->GetPattern<ToastPattern>();
    ASSERT_NE(pattern, nullptr);
    auto toastProp = toastNode->GetLayoutProperty<ToastLayoutProperty>();
    ASSERT_NE(toastProp, nullptr);
    toastProp->UpdateToastAlignment(Alignment::CENTER);
    toastProp->ResetToastOffset();

    auto textId = ElementRegister::GetInstance()->MakeUniqueId();
    auto textNode =
        FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, textId, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode, nullptr);
    textNode->GetGeometryNode()->SetFrameSize(SizeF(TEXT_WIDTH, TEXT_HEIGHT));

    auto hostWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        toastNode, toastNode->GetGeometryNode(), toastNode->GetLayoutProperty());
    auto textWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        textNode, textNode->GetGeometryNode(), textNode->GetLayoutProperty());
    hostWrapper->AppendChild(textWrapper);

    pattern->wrapperRect_ = Rect(WRAPPER_LEFT, WRAPPER_TOP, WRAPPER_WIDTH, WRAPPER_HEIGHT);
    pattern->wrapperFullWidth_ = 200.0f; // smaller than textWidth => centered < safeLeft
    pattern->wrapperFullHeight_ = static_cast<float>(WRAPPER_HEIGHT);

    auto offsetX = pattern->GetOffsetX(hostWrapper);
    // centered = (200 - 200)/2 = 0; clamp(0, 50, 470) = 50.
    EXPECT_FLOAT_EQ(static_cast<float>(offsetX.ConvertToPx()), 50.0f);
}

/**
 * @tc.name: ToastGetOffsetX003
 * @tc.desc: CENTER alignment, natural center (260) lies within [50, 470]: no
 *           clamping occurs, result equals the natural center.
 * @tc.type: FUNC
 */
HWTEST_F(ToastSafeAreaOffsetTestNg, ToastGetOffsetX003, TestSize.Level1)
{
    auto toastId = ElementRegister::GetInstance()->MakeUniqueId();
    auto toastNode =
        FrameNode::CreateFrameNode(V2::TOAST_ETS_TAG, toastId, AceType::MakeRefPtr<ToastPattern>());
    ASSERT_NE(toastNode, nullptr);
    auto pattern = toastNode->GetPattern<ToastPattern>();
    ASSERT_NE(pattern, nullptr);
    auto toastProp = toastNode->GetLayoutProperty<ToastLayoutProperty>();
    ASSERT_NE(toastProp, nullptr);
    toastProp->UpdateToastAlignment(Alignment::CENTER);
    toastProp->ResetToastOffset();

    auto textId = ElementRegister::GetInstance()->MakeUniqueId();
    auto textNode =
        FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, textId, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode, nullptr);
    textNode->GetGeometryNode()->SetFrameSize(SizeF(TEXT_WIDTH, TEXT_HEIGHT));

    auto hostWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        toastNode, toastNode->GetGeometryNode(), toastNode->GetLayoutProperty());
    auto textWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        textNode, textNode->GetGeometryNode(), textNode->GetLayoutProperty());
    hostWrapper->AppendChild(textWrapper);

    pattern->wrapperRect_ = Rect(WRAPPER_LEFT, WRAPPER_TOP, WRAPPER_WIDTH, WRAPPER_HEIGHT);
    pattern->wrapperFullWidth_ = 720.0f; // centered = (720-200)/2 = 260, within range
    pattern->wrapperFullHeight_ = static_cast<float>(WRAPPER_HEIGHT);

    auto offsetX = pattern->GetOffsetX(hostWrapper);
    // clamp(260, 50, 470) = 260.
    EXPECT_FLOAT_EQ(static_cast<float>(offsetX.ConvertToPx()), 260.0f);
}

/**
 * @tc.name: ToastGetOffsetX004
 * @tc.desc: BOTTOM_LEFT (non-center) alignment: the isCenterAlignment branch
 *           is skipped and the result must equal wrapperRect_.Left() (the new
 *           +Dimension(wrapperRect_.Left()) shift), i.e. the X/Y asymmetry fix.
 * @tc.type: FUNC
 */
HWTEST_F(ToastSafeAreaOffsetTestNg, ToastGetOffsetX004, TestSize.Level1)
{
    auto toastId = ElementRegister::GetInstance()->MakeUniqueId();
    auto toastNode =
        FrameNode::CreateFrameNode(V2::TOAST_ETS_TAG, toastId, AceType::MakeRefPtr<ToastPattern>());
    ASSERT_NE(toastNode, nullptr);
    auto pattern = toastNode->GetPattern<ToastPattern>();
    ASSERT_NE(pattern, nullptr);
    auto toastProp = toastNode->GetLayoutProperty<ToastLayoutProperty>();
    ASSERT_NE(toastProp, nullptr);
    toastProp->UpdateToastAlignment(Alignment::BOTTOM_LEFT);
    toastProp->ResetToastOffset();

    auto textId = ElementRegister::GetInstance()->MakeUniqueId();
    auto textNode =
        FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, textId, AceType::MakeRefPtr<TextPattern>());
    ASSERT_NE(textNode, nullptr);
    textNode->GetGeometryNode()->SetFrameSize(SizeF(TEXT_WIDTH, TEXT_HEIGHT));

    auto hostWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        toastNode, toastNode->GetGeometryNode(), toastNode->GetLayoutProperty());
    auto textWrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        textNode, textNode->GetGeometryNode(), textNode->GetLayoutProperty());
    hostWrapper->AppendChild(textWrapper);

    pattern->wrapperRect_ = Rect(WRAPPER_LEFT, WRAPPER_TOP, WRAPPER_WIDTH, WRAPPER_HEIGHT);
    pattern->wrapperFullWidth_ = 720.0f;
    pattern->wrapperFullHeight_ = static_cast<float>(WRAPPER_HEIGHT);

    auto offsetX = pattern->GetOffsetX(hostWrapper);
    // offsetX = 0 (BOTTOM_LEFT); result = 0 + Dimension(Left=50) + 0 = 50.
    EXPECT_FLOAT_EQ(static_cast<float>(offsetX.ConvertToPx()), 50.0f);
}
} // namespace OHOS::Ace::NG
