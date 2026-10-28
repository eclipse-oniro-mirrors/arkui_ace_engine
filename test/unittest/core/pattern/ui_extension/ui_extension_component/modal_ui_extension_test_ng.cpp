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

#define private public
#define protected public

#include "core/components_ng/pattern/ui_extension/ui_extension_component/ui_extension_pattern.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_component/ui_extension_node.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_component/ui_extension_proxy.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_component/modal_ui_extension_proxy_impl.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_component/session_wrapper_impl.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_model_ng.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_config.h"
#include "core/components_ng/property/transition_property.h"
#include "interfaces/inner_api/ace/modal_ui_extension_config.h"
#include "adapter/ohos/osal/want_wrap_ohos.h"

#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/base/thread/mock_task_executor.h"
#include "test/unittest/core/pattern/ui_extension/mock/mock_accessibility_child_tree_callback.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
const std::string UI_EXTENSION_COMPONENT_ETS_TAG = "UIExtensionComponent";
}

#ifdef WINDOW_SCENE_SUPPORTED
const RefPtr<UIExtensionManager>& PipelineContext::GetUIExtensionManager()
{
    return uiExtensionManager_;
}
#endif

class ModalUIExtensionTestNg : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
    static void SetUpTestSuite()
    {
        MockPipelineContext::SetUp();
        MockContainer::SetUp();
        MockContainer::Current()->pipelineContext_ = MockPipelineContext::GetCurrentContext();
        MockContainer::Current()->pipelineContext_->taskExecutor_ = MockContainer::Current()->taskExecutor_;
    }
    static void TearDownTestSuite()
    {
        MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }

    RefPtr<UIExtensionPattern> CreatePattern()
    {
        auto nodeId = ElementRegister::GetInstance()->MakeUniqueId();
        auto node = FrameNode::GetOrCreateFrameNode(
            UI_EXTENSION_COMPONENT_ETS_TAG, nodeId,
            []() { return AceType::MakeRefPtr<UIExtensionPattern>(false, true, false); });
        EXPECT_NE(node, nullptr);
        auto pattern = node->GetPattern<UIExtensionPattern>();
        EXPECT_NE(pattern, nullptr);
        pattern->AttachToFrameNode(node);
        return pattern;
    }

    ModalUIExtensionCallbacks CreateCallbacks()
    {
        return {
            .onRelease = [](int32_t code) {},
            .onResult = [](int32_t code, const AAFwk::Want& want) {},
            .onReceive = [](const AAFwk::WantParams& params) {},
            .onError = [](int32_t code, const std::string& name, const std::string& message) {},
            .onRemoteReady = [](const std::shared_ptr<ModalUIExtensionProxy>& proxy) {},
            .onDestroy = []() {},
            .onDrawReady = []() {},
        };
    }
};

/**
 * @tc.name: ModalUIExtensionCreateTest001
 * @tc.desc: Test UIExtensionModelNG::Create with modal callbacks
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionCreateTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto callbacks = CreateCallbacks();
    AAFwk::Want want;
    want.SetElementName("com.test.bundle", "com.test.ability");
    InnerModalUIExtensionConfig config;
    config.isModal = true;
    auto node = UIExtensionModelNG::Create(want, callbacks, config);
    ASSERT_NE(node, nullptr);
    auto pattern = node->GetPattern<UIExtensionPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_TRUE(pattern->isModal_);
#endif
}

/**
 * @tc.name: ModalUIExtensionCreateTest002
 * @tc.desc: Test UIExtensionModelNG::Create with empty callbacks
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionCreateTest002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    ModalUIExtensionCallbacks callbacks;
    AAFwk::Want want;
    want.SetElementName("com.test.bundle", "com.test.ability");
    InnerModalUIExtensionConfig config;
    auto node = UIExtensionModelNG::Create(want, callbacks, config);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetTag(), V2::UI_EXTENSION_COMPONENT_ETS_TAG);
#endif
}

/**
 * @tc.name: ModalUIExtensionCreateTest003
 * @tc.desc: Test UIExtensionModelNG::Create with config flags
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionCreateTest003, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto callbacks = CreateCallbacks();
    AAFwk::Want want;
    want.SetElementName("com.test.bundle", "com.test.ability");
    InnerModalUIExtensionConfig config;
    config.isModal = true;
    config.isAsyncModalBinding = true;
    config.isDensityFollowHost = true;
    config.isWindowModeFollowHost = true;
    config.isModalRequestFocus = false;
    config.isModalFixFocus = true;
    auto node = UIExtensionModelNG::Create(want, callbacks, config);
    ASSERT_NE(node, nullptr);
    auto pattern = node->GetPattern<UIExtensionPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_TRUE(pattern->isModal_);
    EXPECT_FALSE(pattern->isModalRequestFocus_);
    EXPECT_TRUE(pattern->GetIsModalFixFocus());
#endif
}

/**
 * @tc.name: ModalUIExtensionProxyTest001
 * @tc.desc: Test ModalUIExtensionProxyImpl construction and SendData
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionProxyTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto pattern = CreatePattern();
    ASSERT_NE(pattern, nullptr);
    auto sessionWrapper = AceType::MakeRefPtr<SessionWrapperImpl>(pattern, pattern->instanceId_, true,
        SessionType::UI_EXTENSION_ABILITY);
    ASSERT_NE(sessionWrapper, nullptr);
    auto innerProxy = AceType::MakeRefPtr<UIExtensionProxy>(sessionWrapper, pattern);
    ASSERT_NE(innerProxy, nullptr);
    auto proxy = std::make_shared<ModalUIExtensionProxyImpl>(innerProxy);
    ASSERT_NE(proxy, nullptr);
    AAFwk::WantParams params;
    proxy->SendData(params);
#endif
}

/**
 * @tc.name: ModalUIExtensionPatternCallbackTest001
 * @tc.desc: Test modal callbacks set and fire on pattern
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionPatternCallbackTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto pattern = CreatePattern();
    ASSERT_NE(pattern, nullptr);

    bool onRemoteReadyCalled = false;
    bool onDestroyCalled = false;
    bool onDrawReadyCalled = false;

    pattern->SetModalOnRemoteReadyCallback(
        [&onRemoteReadyCalled](const std::shared_ptr<ModalUIExtensionProxy>& proxy) {
            onRemoteReadyCalled = true;
        });
    pattern->SetModalOnDestroy([&onDestroyCalled]() { onDestroyCalled = true; });
    pattern->SetOnDrawReadyCallback([&onDrawReadyCalled]() { onDrawReadyCalled = true; });

    pattern->FireModalOnDestroy();
    EXPECT_TRUE(onDestroyCalled);

    pattern->FireOnDrawReadyCallback();
    EXPECT_TRUE(onDrawReadyCalled);
#endif
}

/**
 * @tc.name: ModalUIExtensionPatternCallbackTest002
 * @tc.desc: Test onRelease and onError callbacks
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionPatternCallbackTest002, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto pattern = CreatePattern();
    ASSERT_NE(pattern, nullptr);

    int32_t releaseCode = -1;
    int32_t errorCode = -1;
    std::string errorName;
    std::string errorMessage;

    pattern->SetOnReleaseCallback(
        [&releaseCode](int32_t code) { releaseCode = code; });
    pattern->SetOnErrorCallback(
        [&errorCode, &errorName, &errorMessage](int32_t code, const std::string& name, const std::string& msg) {
            errorCode = code;
            errorName = name;
            errorMessage = msg;
        });

    pattern->FireOnReleaseCallback(1);
    EXPECT_EQ(releaseCode, 1);

    pattern->FireOnErrorCallback(100, "TestError", "test message");
    EXPECT_EQ(errorCode, -1);
    EXPECT_EQ(errorName, "");
    EXPECT_EQ(errorMessage, "");
#endif
}

/**
 * @tc.name: ModalUIExtensionPatternCallbackTest003
 * @tc.desc: Test onReceive callback with WantParams
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionPatternCallbackTest003, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto pattern = CreatePattern();
    ASSERT_NE(pattern, nullptr);

    bool onReceiveCalled = false;
    pattern->SetOnReceiveCallback(
        [&onReceiveCalled](const AAFwk::WantParams& params) { onReceiveCalled = true; });

    AAFwk::WantParams params;
    pattern->FireOnReceiveCallback(params);
    EXPECT_TRUE(onReceiveCalled);
#endif
}

/**
 * @tc.name: ModalUIExtensionPatternCallbackTest004
 * @tc.desc: Test onTerminated callback
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionPatternCallbackTest004, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto pattern = CreatePattern();
    ASSERT_NE(pattern, nullptr);

    int32_t terminatedCode = -1;
    pattern->SetOnTerminatedCallback(
        [&terminatedCode](int32_t code, const RefPtr<WantWrap>& wantWrap) { terminatedCode = code; });

    pattern->FireOnTerminatedCallback(42, nullptr);
    EXPECT_EQ(terminatedCode, 42);
#endif
}

/**
 * @tc.name: ModalUIExtensionConfigTest001
 * @tc.desc: Test ModalUIExtensionConfig default values
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionConfigTest001, TestSize.Level1)
{
    ModalUIExtensionConfig config;
    EXPECT_FALSE(config.isProhibitBack);
    EXPECT_FALSE(config.isAsyncModalBinding);
    EXPECT_FALSE(config.isAllowedBeCovered);
    EXPECT_FALSE(config.prohibitedRemoveByRouter);
    EXPECT_FALSE(config.isAllowAddChildBelowModalUec);
    EXPECT_FALSE(config.isDensityFollowHost);
    EXPECT_TRUE(config.prohibitedRemoveByNavigation);
    EXPECT_FALSE(config.isWindowModeFollowHost);
    EXPECT_TRUE(config.isModalRequestFocus);
    EXPECT_FALSE(config.isModalFixFocus);
}

/**
 * @tc.name: ModalUIExtensionConfigTest002
 * @tc.desc: Test ModalUIExtensionConfig field assignment
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionConfigTest002, TestSize.Level1)
{
    ModalUIExtensionConfig config;
    config.isProhibitBack = true;
    config.isAsyncModalBinding = true;
    config.isAllowedBeCovered = true;
    config.prohibitedRemoveByRouter = true;
    config.isAllowAddChildBelowModalUec = true;
    config.isDensityFollowHost = true;
    config.prohibitedRemoveByNavigation = false;
    config.isWindowModeFollowHost = true;
    config.isModalRequestFocus = false;
    config.isModalFixFocus = true;

    EXPECT_TRUE(config.isProhibitBack);
    EXPECT_TRUE(config.isAsyncModalBinding);
    EXPECT_TRUE(config.isAllowedBeCovered);
    EXPECT_TRUE(config.prohibitedRemoveByRouter);
    EXPECT_TRUE(config.isAllowAddChildBelowModalUec);
    EXPECT_TRUE(config.isDensityFollowHost);
    EXPECT_FALSE(config.prohibitedRemoveByNavigation);
    EXPECT_TRUE(config.isWindowModeFollowHost);
    EXPECT_FALSE(config.isModalRequestFocus);
    EXPECT_TRUE(config.isModalFixFocus);
}

/**
 * @tc.name: ModalUIExtensionConfigTest003
 * @tc.desc: Test InnerModalUIExtensionConfig default values
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionConfigTest003, TestSize.Level1)
{
    InnerModalUIExtensionConfig config;
    EXPECT_FALSE(config.isAsyncModalBinding);
    EXPECT_TRUE(config.isModal);
    EXPECT_FALSE(config.isDensityFollowHost);
    EXPECT_FALSE(config.isWindowModeFollowHost);
    EXPECT_TRUE(config.isModalRequestFocus);
    EXPECT_FALSE(config.isModalFixFocus);
    EXPECT_EQ(config.transition, nullptr);
}

/**
 * @tc.name: ModalUIExtensionCallbacksTest001
 * @tc.desc: Test ModalUIExtensionCallbacks construction and assignment
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionCallbacksTest001, TestSize.Level1)
{
    ModalUIExtensionCallbacks callbacks;
    EXPECT_EQ(callbacks.onRelease, nullptr);
    EXPECT_EQ(callbacks.onResult, nullptr);
    EXPECT_EQ(callbacks.onReceive, nullptr);
    EXPECT_EQ(callbacks.onError, nullptr);
    EXPECT_EQ(callbacks.onRemoteReady, nullptr);
    EXPECT_EQ(callbacks.onDestroy, nullptr);
    EXPECT_EQ(callbacks.onDrawReady, nullptr);
}

/**
 * @tc.name: ModalUIExtensionCallbacksTest002
 * @tc.desc: Test ModalUIExtensionCallbacks with lambda assignment
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionCallbacksTest002, TestSize.Level1)
{
    int32_t releaseCode = -1;
    bool destroyCalled = false;
    bool drawReadyCalled = false;

    ModalUIExtensionCallbacks callbacks = {
        .onRelease = [&releaseCode](int32_t code) { releaseCode = code; },
        .onDestroy = [&destroyCalled]() { destroyCalled = true; },
        .onDrawReady = [&drawReadyCalled]() { drawReadyCalled = true; },
    };

    ASSERT_NE(callbacks.onRelease, nullptr);
    ASSERT_NE(callbacks.onDestroy, nullptr);
    ASSERT_NE(callbacks.onDrawReady, nullptr);

    callbacks.onRelease(5);
    EXPECT_EQ(releaseCode, 5);

    callbacks.onDestroy();
    EXPECT_TRUE(destroyCalled);

    callbacks.onDrawReady();
    EXPECT_TRUE(drawReadyCalled);
}

/**
 * @tc.name: ModalUIExtensionProxyImplTest001
 * @tc.desc: Test ModalUIExtensionProxyImpl with null session wrapper
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionProxyImplTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto proxy = std::make_shared<ModalUIExtensionProxyImpl>(nullptr);
    ASSERT_NE(proxy, nullptr);
    AAFwk::WantParams params;
    proxy->SendData(params);
    SUCCEED();
#endif
}

/**
 * @tc.name: ModalUIExtensionDensityDpiTest001
 * @tc.desc: Test density DPI follow host setting on pattern
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionDensityDpiTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto callbacks = CreateCallbacks();
    AAFwk::Want want;
    want.SetElementName("com.test.bundle", "com.test.ability");
    InnerModalUIExtensionConfig config;
    config.isDensityFollowHost = true;
    auto node = UIExtensionModelNG::Create(want, callbacks, config);
    ASSERT_NE(node, nullptr);
    auto pattern = node->GetPattern<UIExtensionPattern>();
    ASSERT_NE(pattern, nullptr);
#endif
}

/**
 * @tc.name: ModalUIExtensionWindowModeTest001
 * @tc.desc: Test window mode follow host setting on pattern
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionWindowModeTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto callbacks = CreateCallbacks();
    AAFwk::Want want;
    want.SetElementName("com.test.bundle", "com.test.ability");
    InnerModalUIExtensionConfig config;
    config.isWindowModeFollowHost = true;
    auto node = UIExtensionModelNG::Create(want, callbacks, config);
    ASSERT_NE(node, nullptr);
    auto pattern = node->GetPattern<UIExtensionPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_TRUE(pattern->GetIsWindowModeFollowHost());
#endif
}

/**
 * @tc.name: ModalUIExtensionFocusTest001
 * @tc.desc: Test modal request focus and fix focus settings
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionFocusTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    auto callbacks = CreateCallbacks();
    AAFwk::Want want;
    want.SetElementName("com.test.bundle", "com.test.ability");
    InnerModalUIExtensionConfig config;
    config.isModalRequestFocus = false;
    config.isModalFixFocus = true;
    auto node = UIExtensionModelNG::Create(want, callbacks, config);
    ASSERT_NE(node, nullptr);
    auto pattern = node->GetPattern<UIExtensionPattern>();
    ASSERT_NE(pattern, nullptr);
    EXPECT_FALSE(pattern->isModalRequestFocus_);
    EXPECT_TRUE(pattern->GetIsModalFixFocus());
#endif
}

/**
 * @tc.name: ModalUIExtensionOnDrawReadyTest001
 * @tc.desc: Test onDrawReady callback wired in modal create path
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionOnDrawReadyTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    bool drawReadyCalled = false;
    auto callbacks = CreateCallbacks();
    callbacks.onDrawReady = [&drawReadyCalled]() { drawReadyCalled = true; };
    AAFwk::Want want;
    want.SetElementName("com.test.bundle", "com.test.ability");
    InnerModalUIExtensionConfig config;
    auto node = UIExtensionModelNG::Create(want, callbacks, config);
    ASSERT_NE(node, nullptr);
    auto pattern = node->GetPattern<UIExtensionPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->FireOnDrawReadyCallback();
    EXPECT_TRUE(drawReadyCalled);
#endif
}

/**
 * @tc.name: ModalUIExtensionTransitionTest002
 * @tc.desc: Test transition field in InnerModalUIExtensionConfig
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionTransitionTest002, TestSize.Level1)
{
    auto effect = AceType::MakeRefPtr<ChainedIdentityEffect>();
    InnerModalUIExtensionConfig config;
    config.transition = effect;
    EXPECT_NE(config.transition, nullptr);
    EXPECT_EQ(config.transition, effect);

    config.transition = nullptr;
    EXPECT_EQ(config.transition, nullptr);
}

/**
 * @tc.name: ModalUIExtensionAllowedUpdateConfigTest001
 * @tc.desc: Test ModalUIExtensionAllowedUpdateConfig default values and assignment
 * @tc.type: FUNC
 */
HWTEST_F(ModalUIExtensionTestNg, ModalUIExtensionAllowedUpdateConfigTest001, TestSize.Level1)
{
    ModalUIExtensionAllowedUpdateConfig config;
    EXPECT_FALSE(config.prohibitedRemoveByRouter);
    EXPECT_TRUE(config.prohibitedRemoveByNavigation);

    config.prohibitedRemoveByRouter = true;
    config.prohibitedRemoveByNavigation = false;
    EXPECT_TRUE(config.prohibitedRemoveByRouter);
    EXPECT_FALSE(config.prohibitedRemoveByNavigation);
}
} // namespace OHOS::Ace::NG
