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

#include <iostream>
#include <optional>

#include "gtest/gtest.h"

// arm32 pre-existing narrowing warnings from ace headers (modifier.h hash constants)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconstant-conversion"

#define private public
#define protected public

#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_surface.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_controller_ng.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_model_ng.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_pattern.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_pattern_v2.h"
#include "core/components_ng/pattern/xcomponent/xcomponent_surface_holder.h"
#include "core/components/xcomponent/native_interface_xcomponent_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
const std::string XCOMPONENT_ID = "xcomponent";
const std::string XCOMPONENT_LIBRARY_NAME = "native_render";
constexpr XComponentType XCOMPONENT_SURFACE_TYPE_VALUE = XComponentType::SURFACE;

int32_t g_surfaceCreateCount = 0;
int32_t g_surfaceDestroyCount = 0;
void OnSurfaceCreatedCallback(OH_NativeXComponent* /*component*/, void* /*window*/)
{
    ++g_surfaceCreateCount;
}
void OnSurfaceDestroyedCallback(OH_NativeXComponent* /*component*/, void* /*window*/)
{
    ++g_surfaceDestroyCount;
}

int32_t g_holderCreateCount = 0;
int32_t g_holderDestroyCount = 0;
void OnHolderCreatedCallback(OH_ArkUI_SurfaceHolder* /*holder*/)
{
    ++g_holderCreateCount;
}
void OnHolderDestroyedCallback(OH_ArkUI_SurfaceHolder* /*holder*/)
{
    ++g_holderDestroyCount;
}
} // namespace

class XComponentTestSurfaceVisibilityLifecycle : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void TearDown() override {}

protected:
    static RefPtr<FrameNode> CreateXComponentNode(XComponentType type);
    static RefPtr<XComponentPattern> GetPattern(const RefPtr<FrameNode>& frameNode);
};

RefPtr<FrameNode> XComponentTestSurfaceVisibilityLifecycle::CreateXComponentNode(XComponentType type)
{
    auto xcomponentController = std::make_shared<XComponentControllerNG>();
    XComponentModelNG().Create(XCOMPONENT_ID, type, XCOMPONENT_LIBRARY_NAME, xcomponentController);
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    return AceType::DynamicCast<FrameNode>(element);
}

RefPtr<XComponentPattern> XComponentTestSurfaceVisibilityLifecycle::GetPattern(const RefPtr<FrameNode>& frameNode)
{
    return frameNode->GetPattern<XComponentPattern>();
}

void XComponentTestSurfaceVisibilityLifecycle::SetUpTestSuite()
{
    MockPipelineContext::SetUp();
}

void XComponentTestSurfaceVisibilityLifecycle::TearDownTestSuite()
{
    MockPipelineContext::TearDown();
}

/**
 * @tc.name: SurfaceVisibilityLifecycleTest001
 * @tc.desc: Test default surface visibility lifecycle without configuration
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestSurfaceVisibilityLifecycle, SurfaceVisibilityLifecycleTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create xcomponent node without visibility lifecycle config
     * @tc.expected: isVisibilityLifecycle_ is false by default
     */
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);
    EXPECT_FALSE(pattern->isVisibilityLifecycle_);
    EXPECT_FALSE(pattern->IsSurfaceVisibilityLifecycle());
}

/**
 * @tc.name: SurfaceVisibilityLifecycleTest002
 * @tc.desc: Test SetSurfaceVisibilityLifecycle with true and false on pattern
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestSurfaceVisibilityLifecycle, SurfaceVisibilityLifecycleTest002, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    pattern->SetSurfaceVisibilityLifecycle(true);
    EXPECT_TRUE(pattern->isVisibilityLifecycle_);
    EXPECT_TRUE(pattern->IsSurfaceVisibilityLifecycle());
    pattern->SetSurfaceVisibilityLifecycle(false);
    EXPECT_FALSE(pattern->isVisibilityLifecycle_);
    EXPECT_FALSE(pattern->IsSurfaceVisibilityLifecycle());
}

/**
 * @tc.name: SurfaceVisibilityLifecycleTest003
 * @tc.desc: Test Model layer static entry SetSurfaceVisibilityLifecycle
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestSurfaceVisibilityLifecycle, SurfaceVisibilityLifecycleTest003, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    XComponentModelNG::SetSurfaceVisibilityLifecycle(AceType::RawPtr(frameNode), true);
    EXPECT_TRUE(pattern->isVisibilityLifecycle_);
    XComponentModelNG::SetSurfaceVisibilityLifecycle(AceType::RawPtr(frameNode), false);
    EXPECT_FALSE(pattern->isVisibilityLifecycle_);
}

/**
 * @tc.name: SurfaceVisibilityLifecycleTest004
 * @tc.desc: Test C API path stores value on NativeXComponentImpl and pattern getter ORs both sources
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestSurfaceVisibilityLifecycle, SurfaceVisibilityLifecycleTest004, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    auto [nativeXComponentImpl, nativeXComponentWeak] = pattern->GetNativeXComponent();
    ASSERT_NE(nativeXComponentImpl, nullptr);
    auto nativeXComponent = nativeXComponentWeak.lock();
    ASSERT_NE(nativeXComponent, nullptr);

    /**
     * @tc.steps: step1. Enable via OH_NativeXComponent C API path only
     * @tc.expected: impl stores true and pattern getter returns true
     */
    EXPECT_EQ(nativeXComponent->EnableSurfaceVisibilityLifecycle(true), OH_NATIVEXCOMPONENT_RESULT_SUCCESS);
    EXPECT_TRUE(nativeXComponentImpl->IsVisibilityLifecycle());
    EXPECT_TRUE(pattern->IsSurfaceVisibilityLifecycle());

    /**
     * @tc.steps: step2. Disable C API path, enable via JS pattern member
     * @tc.expected: pattern getter still returns true (either source enabled)
     */
    EXPECT_EQ(nativeXComponent->EnableSurfaceVisibilityLifecycle(false), OH_NATIVEXCOMPONENT_RESULT_SUCCESS);
    EXPECT_FALSE(nativeXComponentImpl->IsVisibilityLifecycle());
    pattern->SetSurfaceVisibilityLifecycle(true);
    EXPECT_TRUE(pattern->IsSurfaceVisibilityLifecycle());

    /**
     * @tc.steps: step3. Disable both sources
     * @tc.expected: pattern getter returns false
     */
    EXPECT_EQ(nativeXComponent->EnableSurfaceVisibilityLifecycle(false), OH_NATIVEXCOMPONENT_RESULT_SUCCESS);
    pattern->SetSurfaceVisibilityLifecycle(false);
    EXPECT_FALSE(pattern->IsSurfaceVisibilityLifecycle());
}

/**
 * @tc.name: SurfaceVisibilityLifecycleTest005
 * @tc.desc: Test invisible event before surface init does not fire destroy
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestSurfaceVisibilityLifecycle, SurfaceVisibilityLifecycleTest005, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    pattern->SetSurfaceVisibilityLifecycle(true);
    frameNode->onMainTree_ = true;
    // hasXComponentInit_ is false, surface was never created, nothing to pair with
    g_surfaceDestroyCount = 0;
    pattern->OnVisibilityLifecycleChange(false);
    EXPECT_EQ(g_surfaceDestroyCount, 0);
    EXPECT_FALSE(pattern->isVisibilityLifecycleDestroyed_);
}

/**
 * @tc.name: SurfaceVisibilityLifecycleTest006
 * @tc.desc: Test invisible triggers cleanCache and destroy, visible again triggers create, paired
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestSurfaceVisibilityLifecycle, SurfaceVisibilityLifecycleTest006, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    pattern->isNativeXComponent_ = true;
    auto [nativeXComponentImpl, nativeXComponentWeak] = pattern->GetNativeXComponent();
    ASSERT_NE(nativeXComponentImpl, nullptr);
    auto nativeXComponent = nativeXComponentWeak.lock();
    ASSERT_NE(nativeXComponent, nullptr);
    OH_NativeXComponent_Callback surfaceCallback = {};
    surfaceCallback.OnSurfaceCreated = OnSurfaceCreatedCallback;
    surfaceCallback.OnSurfaceChanged = [](OH_NativeXComponent*, void*) {};
    surfaceCallback.OnSurfaceDestroyed = OnSurfaceDestroyedCallback;
    surfaceCallback.DispatchTouchEvent = [](OH_NativeXComponent*, void*) {};
    EXPECT_EQ(nativeXComponent->RegisterCallback(&surfaceCallback), OH_NATIVEXCOMPONENT_RESULT_SUCCESS);

    pattern->SetSurfaceVisibilityLifecycle(true);
    pattern->hasXComponentInit_ = true;
    frameNode->onMainTree_ = true;

    auto mockRenderSurface = AceType::MakeRefPtr<MockRenderSurface>();
    pattern->renderSurface_ = mockRenderSurface;

    g_surfaceCreateCount = 0;
    g_surfaceDestroyCount = 0;

    /**
     * @tc.steps: step1. XComponent becomes invisible
     * @tc.expected: cleanCache (ReleaseSurfaceBuffers) called and native OnSurfaceDestroyed fired once
     */
    EXPECT_CALL(*mockRenderSurface, ReleaseSurfaceBuffers()).Times(1);
    pattern->OnVisibilityLifecycleChange(false);
    EXPECT_EQ(g_surfaceDestroyCount, 1);
    EXPECT_EQ(g_surfaceCreateCount, 0);
    EXPECT_TRUE(pattern->isVisibilityLifecycleDestroyed_);

    /**
     * @tc.steps: step2. XComponent becomes visible again
     * @tc.expected: native OnSurfaceCreated fired once, paired with previous destroy
     */
    pattern->OnVisibilityLifecycleChange(true);
    EXPECT_EQ(g_surfaceCreateCount, 1);
    EXPECT_EQ(g_surfaceDestroyCount, 1);
    EXPECT_FALSE(pattern->isVisibilityLifecycleDestroyed_);
}

/**
 * @tc.name: SurfaceVisibilityLifecycleTest007
 * @tc.desc: Test repeated same-state visibility events do not double fire callbacks
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestSurfaceVisibilityLifecycle, SurfaceVisibilityLifecycleTest007, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    pattern->isNativeXComponent_ = true;
    auto [nativeXComponentImpl, nativeXComponentWeak] = pattern->GetNativeXComponent();
    ASSERT_NE(nativeXComponentImpl, nullptr);
    auto nativeXComponent = nativeXComponentWeak.lock();
    ASSERT_NE(nativeXComponent, nullptr);
    OH_NativeXComponent_Callback surfaceCallback = {};
    surfaceCallback.OnSurfaceCreated = OnSurfaceCreatedCallback;
    surfaceCallback.OnSurfaceChanged = [](OH_NativeXComponent*, void*) {};
    surfaceCallback.OnSurfaceDestroyed = OnSurfaceDestroyedCallback;
    surfaceCallback.DispatchTouchEvent = [](OH_NativeXComponent*, void*) {};
    EXPECT_EQ(nativeXComponent->RegisterCallback(&surfaceCallback), OH_NATIVEXCOMPONENT_RESULT_SUCCESS);

    pattern->SetSurfaceVisibilityLifecycle(true);
    pattern->hasXComponentInit_ = true;
    frameNode->onMainTree_ = true;
    auto mockRenderSurface = AceType::MakeRefPtr<MockRenderSurface>();
    pattern->renderSurface_ = mockRenderSurface;

    g_surfaceCreateCount = 0;
    g_surfaceDestroyCount = 0;

    /**
     * @tc.steps: step1. Fire repeated invisible events and repeated visible events
     * @tc.expected: only one destroy and one create fired, strictly paired
     */
    EXPECT_CALL(*mockRenderSurface, ReleaseSurfaceBuffers()).Times(1);
    pattern->OnVisibilityLifecycleChange(false);
    pattern->OnVisibilityLifecycleChange(false);
    pattern->OnVisibilityLifecycleChange(true);
    pattern->OnVisibilityLifecycleChange(true);
    EXPECT_EQ(g_surfaceDestroyCount, 1);
    EXPECT_EQ(g_surfaceCreateCount, 1);
}

/**
 * @tc.name: SurfaceVisibilityLifecycleTest008
 * @tc.desc: Test disabling the feature while destroyed restores create to keep pairing
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestSurfaceVisibilityLifecycle, SurfaceVisibilityLifecycleTest008, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);

    pattern->isNativeXComponent_ = true;
    auto [nativeXComponentImpl, nativeXComponentWeak] = pattern->GetNativeXComponent();
    ASSERT_NE(nativeXComponentImpl, nullptr);
    auto nativeXComponent = nativeXComponentWeak.lock();
    ASSERT_NE(nativeXComponent, nullptr);
    OH_NativeXComponent_Callback surfaceCallback = {};
    surfaceCallback.OnSurfaceCreated = OnSurfaceCreatedCallback;
    surfaceCallback.OnSurfaceChanged = [](OH_NativeXComponent*, void*) {};
    surfaceCallback.OnSurfaceDestroyed = OnSurfaceDestroyedCallback;
    surfaceCallback.DispatchTouchEvent = [](OH_NativeXComponent*, void*) {};
    EXPECT_EQ(nativeXComponent->RegisterCallback(&surfaceCallback), OH_NATIVEXCOMPONENT_RESULT_SUCCESS);

    pattern->SetSurfaceVisibilityLifecycle(true);
    pattern->hasXComponentInit_ = true;
    frameNode->onMainTree_ = true;
    auto mockRenderSurface = AceType::MakeRefPtr<MockRenderSurface>();
    pattern->renderSurface_ = mockRenderSurface;
    EXPECT_CALL(*mockRenderSurface, ReleaseSurfaceBuffers()).Times(1);

    g_surfaceCreateCount = 0;
    g_surfaceDestroyCount = 0;

    pattern->OnVisibilityLifecycleChange(false);
    EXPECT_EQ(g_surfaceDestroyCount, 1);

    /**
     * @tc.steps: step1. Disable the feature while surface is destroyed by visibility
     * @tc.expected: create fired once to restore pairing, feature unregistered
     */
    pattern->SetSurfaceVisibilityLifecycle(false);
    EXPECT_EQ(g_surfaceCreateCount, 1);
    EXPECT_EQ(g_surfaceDestroyCount, 1);
    EXPECT_FALSE(pattern->isVisibilityLifecycleDestroyed_);
    EXPECT_FALSE(pattern->isVisibilityLifecycleRegistered_);
}

/**
 * @tc.name: SurfaceVisibilityLifecycleTest009
 * @tc.desc: Test V2 surfaceHolder path: window hide/show pairs holder destroy/create callbacks
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestSurfaceVisibilityLifecycle, SurfaceVisibilityLifecycleTest009, TestSize.Level1)
{
    ArkUI_XComponent_Params params;
    params.type = XCOMPONENT_SURFACE_TYPE_VALUE;
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = XComponentModelNG().CreateTypeNode(nodeId, &params);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<XComponentPatternV2>();
    ASSERT_NE(pattern, nullptr);

    OH_ArkUI_SurfaceHolder* surfaceHolder = new OH_ArkUI_SurfaceHolder();
    OH_ArkUI_SurfaceCallback holderCallback = {};
    holderCallback.OnSurfaceCreated = OnHolderCreatedCallback;
    holderCallback.OnSurfaceDestroyed = OnHolderDestroyedCallback;
    ASSERT_EQ(surfaceHolder->AddSurfaceCallback(&holderCallback), OHOS::Ace::ERROR_CODE_NO_ERROR);
    pattern->SetSurfaceHolder(surfaceHolder);

    frameNode->onMainTree_ = true;
    ASSERT_EQ(pattern->HandleSurfaceCreated(), OHOS::Ace::ERROR_CODE_NO_ERROR);

    /**
     * @tc.steps: step1. Enable the feature while the node is on the main tree
     * @tc.expected: visible-area and window callbacks are registered immediately
     */
    pattern->SetSurfaceVisibilityLifecycle(true);
    EXPECT_TRUE(pattern->isVisibilityLifecycleRegistered_);

    g_holderCreateCount = 0;
    g_holderDestroyCount = 0;

    /**
     * @tc.steps: step2. Window hide
     * @tc.expected: holder OnSurfaceDestroyed fired once, surface uninitialized, pairing recorded
     */
    pattern->OnWindowHide();
    EXPECT_EQ(g_holderDestroyCount, 1);
    EXPECT_EQ(g_holderCreateCount, 0);
    EXPECT_FALSE(pattern->isInitialized_);
    EXPECT_TRUE(pattern->isVisibilityLifecycleDestroyed_);

    /**
     * @tc.steps: step3. Window show
     * @tc.expected: holder OnSurfaceCreated fired once, surface re-initialized, pairing restored
     */
    pattern->OnWindowShow();
    EXPECT_EQ(g_holderCreateCount, 1);
    EXPECT_EQ(g_holderDestroyCount, 1);
    EXPECT_TRUE(pattern->isInitialized_);
    EXPECT_FALSE(pattern->isVisibilityLifecycleDestroyed_);

    pattern->SetSurfaceHolder(nullptr);
    delete surfaceHolder;
}

/**
 * @tc.name: SurfaceVisibilityLifecycleTest010
 * @tc.desc: Test V2 holder path: C API entry activates registration immediately without window events
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestSurfaceVisibilityLifecycle, SurfaceVisibilityLifecycleTest010, TestSize.Level1)
{
    ArkUI_XComponent_Params params;
    params.type = XCOMPONENT_SURFACE_TYPE_VALUE;
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = XComponentModelNG().CreateTypeNode(nodeId, &params);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<XComponentPatternV2>();
    ASSERT_NE(pattern, nullptr);
    frameNode->onMainTree_ = true;
    ASSERT_EQ(pattern->HandleSurfaceCreated(), OHOS::Ace::ERROR_CODE_NO_ERROR);

    auto [nativeXComponentImpl, nativeXComponentWeak] = pattern->GetNativeXComponent();
    ASSERT_NE(nativeXComponentImpl, nullptr);
    auto nativeXComponent = nativeXComponentWeak.lock();
    ASSERT_NE(nativeXComponent, nullptr);

    /**
     * @tc.steps: step1. Enable via C API after the node is already on the main tree
     * @tc.expected: registered immediately through the NativeXComponentImpl event callback
     */
    EXPECT_EQ(nativeXComponent->EnableSurfaceVisibilityLifecycle(true), OH_NATIVEXCOMPONENT_RESULT_SUCCESS);
    EXPECT_TRUE(nativeXComponentImpl->IsVisibilityLifecycle());
    EXPECT_TRUE(pattern->isVisibilityLifecycleRegistered_);

    /**
     * @tc.steps: step2. Disable via C API
     * @tc.expected: callbacks unregistered immediately
     */
    EXPECT_EQ(nativeXComponent->EnableSurfaceVisibilityLifecycle(false), OH_NATIVEXCOMPONENT_RESULT_SUCCESS);
    EXPECT_FALSE(pattern->isVisibilityLifecycleRegistered_);
}

/**
 * @tc.name: SurfaceVisibilityLifecycleTest011
 * @tc.desc: Test V2 holder path: disabling while destroyed restores create through HandleSurfaceCreated
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestSurfaceVisibilityLifecycle, SurfaceVisibilityLifecycleTest011, TestSize.Level1)
{
    ArkUI_XComponent_Params params;
    params.type = XCOMPONENT_SURFACE_TYPE_VALUE;
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto frameNode = XComponentModelNG().CreateTypeNode(nodeId, &params);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = frameNode->GetPattern<XComponentPatternV2>();
    ASSERT_NE(pattern, nullptr);

    OH_ArkUI_SurfaceHolder* surfaceHolder = new OH_ArkUI_SurfaceHolder();
    OH_ArkUI_SurfaceCallback holderCallback = {};
    holderCallback.OnSurfaceCreated = OnHolderCreatedCallback;
    holderCallback.OnSurfaceDestroyed = OnHolderDestroyedCallback;
    ASSERT_EQ(surfaceHolder->AddSurfaceCallback(&holderCallback), OHOS::Ace::ERROR_CODE_NO_ERROR);
    pattern->SetSurfaceHolder(surfaceHolder);

    frameNode->onMainTree_ = true;
    ASSERT_EQ(pattern->HandleSurfaceCreated(), OHOS::Ace::ERROR_CODE_NO_ERROR);
    pattern->SetSurfaceVisibilityLifecycle(true);

    g_holderCreateCount = 0;
    g_holderDestroyCount = 0;
    pattern->OnVisibilityLifecycleChange(false);
    EXPECT_EQ(g_holderDestroyCount, 1);
    EXPECT_TRUE(pattern->isVisibilityLifecycleDestroyed_);

    /**
     * @tc.steps: step1. Disable the feature while the surface is destroyed by visibility
     * @tc.expected: holder OnSurfaceCreated fired once to restore pairing, feature unregistered
     */
    pattern->SetSurfaceVisibilityLifecycle(false);
    EXPECT_EQ(g_holderCreateCount, 1);
    EXPECT_TRUE(pattern->isInitialized_);
    EXPECT_FALSE(pattern->isVisibilityLifecycleDestroyed_);
    EXPECT_FALSE(pattern->isVisibilityLifecycleRegistered_);

    pattern->SetSurfaceHolder(nullptr);
    delete surfaceHolder;
}

/**
 * @tc.name: SurfaceVisibilityLifecycleTest012
 * @tc.desc: Test base pattern: C API entry activates registration immediately without window events
 * @tc.type: FUNC
 */
HWTEST_F(XComponentTestSurfaceVisibilityLifecycle, SurfaceVisibilityLifecycleTest012, TestSize.Level1)
{
    auto frameNode = CreateXComponentNode(XCOMPONENT_SURFACE_TYPE_VALUE);
    ASSERT_NE(frameNode, nullptr);
    auto pattern = GetPattern(frameNode);
    ASSERT_NE(pattern, nullptr);
    frameNode->onMainTree_ = true;

    auto [nativeXComponentImpl, nativeXComponentWeak] = pattern->GetNativeXComponent();
    ASSERT_NE(nativeXComponentImpl, nullptr);
    auto nativeXComponent = nativeXComponentWeak.lock();
    ASSERT_NE(nativeXComponent, nullptr);

    /**
     * @tc.steps: step1. Enable via C API after the node is already on the main tree
     * @tc.expected: registered immediately, no window show/hide event required
     */
    EXPECT_EQ(nativeXComponent->EnableSurfaceVisibilityLifecycle(true), OH_NATIVEXCOMPONENT_RESULT_SUCCESS);
    EXPECT_TRUE(pattern->IsSurfaceVisibilityLifecycle());
    EXPECT_TRUE(pattern->isVisibilityLifecycleRegistered_);

    EXPECT_EQ(nativeXComponent->EnableSurfaceVisibilityLifecycle(false), OH_NATIVEXCOMPONENT_RESULT_SUCCESS);
    EXPECT_FALSE(pattern->isVisibilityLifecycleRegistered_);
}
} // namespace OHOS::Ace::NG
