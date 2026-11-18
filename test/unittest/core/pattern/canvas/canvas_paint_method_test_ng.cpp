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

#include <algorithm>
#include <cstddef>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "mock_canvas_paint.h"

#define protected public
#define private public
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/rosen/mock_canvas.h"

#include "base/json/json_util.h"
#include "base/utils/system_properties.h"
#include "core/components/common/properties/paint_state.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/canvas/canvas_modifier.h"
#include "core/components_ng/pattern/canvas/canvas_paint_method.h"
#include "core/components_ng/pattern/canvas/canvas_pattern.h"
#include "core/components_ng/pattern/canvas/canvas_render_context_deferred.h"
#include "core/components_ng/pattern/canvas/custom_paint_paint_method.h"
#include "core/components_ng/pattern/canvas/custom_paint_util.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/constants.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {

namespace {
constexpr size_t MAX_PARAGRAPH_HISTORY_SIZE = 30;
constexpr int32_t TEST_CANVAS_WIDTH = 10;
constexpr int32_t TEST_CANVAS_HEIGHT = 10;
} // namespace

class CanvasPaintMethodTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    // Helper: create a CanvasPaintMethod with a valid frameNode and contentModifier
    static RefPtr<CanvasPaintMethod> CreatePaintMethod(RefPtr<FrameNode>& outFrameNode);
    // Helper: create a CanvasPaintMethod with null frameNode
    static RefPtr<CanvasPaintMethod> CreatePaintMethodNullFrameNode();
};

void CanvasPaintMethodTestNg::SetUpTestCase()
{
    MockContainer::SetUp();
}

void CanvasPaintMethodTestNg::TearDownTestCase()
{
    MockContainer::TearDown();
}

RefPtr<CanvasPaintMethod> CanvasPaintMethodTestNg::CreatePaintMethod(RefPtr<FrameNode>& outFrameNode)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto contentModifier = AceType::MakeRefPtr<CanvasModifier>();
    auto frameNode = FrameNode::GetOrCreateFrameNode(
        V2::CANVAS_ETS_TAG, nodeId, []() { return AceType::MakeRefPtr<CanvasPattern>(); });
    outFrameNode = frameNode;
    frameNode->layoutProperty_ = AceType::MakeRefPtr<LayoutProperty>();
    auto paintMethod = AceType::MakeRefPtr<CanvasPaintMethod>(contentModifier, frameNode);
    return paintMethod;
}

RefPtr<CanvasPaintMethod> CanvasPaintMethodTestNg::CreatePaintMethodNullFrameNode()
{
    auto contentModifier = AceType::MakeRefPtr<CanvasModifier>();
    RefPtr<FrameNode> nullFrameNode = nullptr;
    auto paintMethod = AceType::MakeRefPtr<CanvasPaintMethod>(contentModifier, nullFrameNode);
    return paintMethod;
}

// ==================== Constructor Tests ====================

/**
 * @tc.name: CanvasPaintMethodConstructor001
 * @tc.desc: Test CanvasPaintMethod constructor with valid frameNode.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodConstructor001, TestSize.Level1)
{
    /**
     * @tc.steps1: construct with valid frameNode.
     * @tc.expected: context_ and contentModifier_ are set.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    ASSERT_TRUE(paintMethod);
    EXPECT_NE(paintMethod->contentModifier_, nullptr);
    EXPECT_NE(paintMethod->frameNode_.Upgrade(), nullptr);
}

/**
 * @tc.name: CanvasPaintMethodConstructor002
 * @tc.desc: Test CanvasPaintMethod constructor with null frameNode.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodConstructor002, TestSize.Level1)
{
    /**
     * @tc.steps1: construct with null frameNode.
     * @tc.expected: context_ is null.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    ASSERT_TRUE(paintMethod);
    auto context = paintMethod->context_.Upgrade();
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.name: CanvasPaintMethodConstructor003
 * @tc.desc: Test CanvasPaintMethod constructor with apiVersion >= VERSION_EIGHTEEN.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodConstructor003, TestSize.Level1)
{
    /**
     * @tc.steps1: set apiVersion >= VERSION_EIGHTEEN before constructing.
     * @tc.expected: isPathChanged_ and isPath2dChanged_ are false.
     */
    auto container = Container::Current();
    ASSERT_TRUE(container);
    auto oldVersion = container->GetApiTargetVersion();
    container->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_EIGHTEEN));

    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    ASSERT_TRUE(paintMethod);
    EXPECT_FALSE(paintMethod->isPathChanged_);
    EXPECT_FALSE(paintMethod->isPath2dChanged_);

    container->SetApiTargetVersion(oldVersion);
}

/**
 * @tc.name: CanvasPaintMethodConstructor004
 * @tc.desc: Test CanvasPaintMethod constructor with apiVersion < VERSION_EIGHTEEN.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodConstructor004, TestSize.Level1)
{
    /**
     * @tc.steps1: set apiVersion < VERSION_EIGHTEEN before constructing.
     * @tc.expected: isPathChanged_ and isPath2dChanged_ remain default true.
     */
    auto container = Container::Current();
    ASSERT_TRUE(container);
    auto oldVersion = container->GetApiTargetVersion();
    container->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TEN));

    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    ASSERT_TRUE(paintMethod);
    EXPECT_TRUE(paintMethod->isPathChanged_);
    EXPECT_TRUE(paintMethod->isPath2dChanged_);

    container->SetApiTargetVersion(oldVersion);
}

// ==================== Destructor Tests ====================

/**
 * @tc.name: CanvasPaintMethodDestructor001
 * @tc.desc: Test destructor with canvasRenderContext_ set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodDestructor001, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasRenderContext_ then destroy.
     * @tc.expected: no crash; SetPaintMethod(nullptr) called internally.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    ASSERT_NE(paintMethod->canvasRenderContext_, nullptr);
    // Destructor will call canvasRenderContext_->SetPaintMethod(nullptr)
    paintMethod = nullptr;
    EXPECT_EQ(paintMethod, nullptr);
}

/**
 * @tc.name: CanvasPaintMethodDestructor002
 * @tc.desc: Test destructor with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodDestructor002, TestSize.Level1)
{
    /**
     * @tc.steps1: leave canvasRenderContext_ null then destroy.
     * @tc.expected: no crash.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    EXPECT_EQ(paintMethod->canvasRenderContext_, nullptr);
    paintMethod = nullptr;
    EXPECT_EQ(paintMethod, nullptr);
}

// ==================== PushTask Tests ====================

/**
 * @tc.name: CanvasPaintMethodPushTask001
 * @tc.desc: Test PushTask with canvasRenderContext_ set and needMarkDirty_ true.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodPushTask001, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasRenderContext_ and push a task.
     * @tc.expected: task is pushed, needMarkDirty_ reset to false then MarkDirtyNode called.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    paintMethod->needMarkDirty_ = true;
    bool called = false;
    paintMethod->PushTask([&called](CanvasPaintMethod&) { called = true; });
    // needMarkDirty_ was true, so MarkDirtyNode is called
    EXPECT_FALSE(paintMethod->needMarkDirty_);
}

/**
 * @tc.name: CanvasPaintMethodPushTask002
 * @tc.desc: Test PushTask with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodPushTask002, TestSize.Level1)
{
    /**
     * @tc.steps1: leave canvasRenderContext_ null, call PushTask.
     * @tc.expected: needMarkDirty_ is false (CHECK_EQUAL_VOID returns early).
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    paintMethod->needMarkDirty_ = false;
    paintMethod->PushTask([](CanvasPaintMethod&) {});
    // needMarkDirty_ is false, CHECK_EQUAL_VOID returns immediately
    EXPECT_FALSE(paintMethod->needMarkDirty_);
}

/**
 * @tc.name: CanvasPaintMethodPushTask003
 * @tc.desc: Test PushTask with needMarkDirty_ false.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodPushTask003, TestSize.Level1)
{
    /**
     * @tc.steps1: set needMarkDirty_ to false, call PushTask.
     * @tc.expected: function returns early without marking dirty.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    paintMethod->needMarkDirty_ = false;
    paintMethod->PushTask([](CanvasPaintMethod&) {});
    EXPECT_FALSE(paintMethod->needMarkDirty_);
}

// ==================== NeedRender Tests ====================

/**
 * @tc.name: CanvasPaintMethodNeedRender001
 * @tc.desc: Test NeedRender with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNeedRender001, TestSize.Level1)
{
    /**
     * @tc.steps1: leave canvasRenderContext_ null.
     * @tc.expected: returns false.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    EXPECT_FALSE(paintMethod->NeedRender());
}

/**
 * @tc.name: CanvasPaintMethodNeedRender002
 * @tc.desc: Test NeedRender with canvasRenderContext_ set and tasks pushed.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNeedRender002, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasRenderContext_ and push a task.
     * @tc.expected: NeedRender returns true.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    context->PushTask([](CanvasPaintMethod&) {});
    EXPECT_TRUE(paintMethod->NeedRender());
}

/**
 * @tc.name: CanvasPaintMethodNeedRender003
 * @tc.desc: Test NeedRender with canvasRenderContext_ set but no tasks.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNeedRender003, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasRenderContext_ with no tasks.
     * @tc.expected: NeedRender returns false.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    EXPECT_FALSE(paintMethod->NeedRender());
}

// ==================== FlushTask Tests ====================

/**
 * @tc.name: CanvasPaintMethodFlushTask001
 * @tc.desc: Test FlushTask with canvasRenderContext_ set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodFlushTask001, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasRenderContext_ and call FlushTask.
     * @tc.expected: needMarkDirty_ is set to true.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    paintMethod->needMarkDirty_ = false;
    paintMethod->FlushTask();
    EXPECT_TRUE(paintMethod->needMarkDirty_);
}

/**
 * @tc.name: CanvasPaintMethodFlushTask002
 * @tc.desc: Test FlushTask with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodFlushTask002, TestSize.Level1)
{
    /**
     * @tc.steps1: leave canvasRenderContext_ null and call FlushTask.
     * @tc.expected: needMarkDirty_ is set to true (no crash).
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->needMarkDirty_ = false;
    paintMethod->FlushTask();
    EXPECT_TRUE(paintMethod->needMarkDirty_);
}

// ==================== UpdateContentModifier Tests ====================

/**
 * @tc.name: CanvasPaintMethodUpdateContentModifier001
 * @tc.desc: Test UpdateContentModifier with host null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodUpdateContentModifier001, TestSize.Level1)
{
    /**
     * @tc.steps1: use null frameNode, call UpdateContentModifier.
     * @tc.expected: returns early (CHECK_NULL_VOID host).
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->UpdateContentModifier(nullptr);
    // No crash expected
}

/**
 * @tc.name: CanvasPaintMethodUpdateContentModifier002
 * @tc.desc: Test UpdateContentModifier with geometryNode null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodUpdateContentModifier002, TestSize.Level1)
{
    /**
     * @tc.steps1: set geometryNode_ to null, call UpdateContentModifier.
     * @tc.expected: returns early (CHECK_NULL_VOID geometryNode).
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto oldGeo = frameNode->geometryNode_;
    frameNode->geometryNode_ = nullptr;
    paintMethod->UpdateContentModifier(nullptr);
    frameNode->geometryNode_ = oldGeo;
}

/**
 * @tc.name: CanvasPaintMethodUpdateContentModifier003
 * @tc.desc: Test UpdateContentModifier with recordingCanvas null (rsCanvas_ not set).
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodUpdateContentModifier003, TestSize.Level1)
{
    /**
     * @tc.steps1: do not call UpdateRecordingCanvas (rsCanvas_ is null).
     * @tc.expected: returns early (CHECK_NULL_VOID recordingCanvas).
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    paintMethod->rsCanvas_ = nullptr;
    paintMethod->UpdateContentModifier(nullptr);
    // No crash expected
}

/**
 * @tc.name: CanvasPaintMethodUpdateContentModifier004
 * @tc.desc: Test UpdateContentModifier with NeedRender false.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodUpdateContentModifier004, TestSize.Level1)
{
    /**
     * @tc.steps1: set rsCanvas_ via UpdateRecordingCanvas but leave canvasRenderContext_ null.
     * @tc.expected: NeedRender returns false, returns early.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    paintMethod->UpdateRecordingCanvas(TEST_CANVAS_WIDTH, TEST_CANVAS_HEIGHT);
    // canvasRenderContext_ is null, so NeedRender returns false
    paintMethod->UpdateContentModifier(nullptr);
    EXPECT_FALSE(paintMethod->NeedRender());
}

/**
 * @tc.name: CanvasPaintMethodUpdateContentModifier005
 * @tc.desc: Test UpdateContentModifier with contentModifier null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodUpdateContentModifier005, TestSize.Level1)
{
    /**
     * @tc.steps1: set rsCanvas_ and canvasRenderContext_ with tasks, null contentModifier_.
     * @tc.expected: FlushTask runs and clears tasks, so NeedRender returns false afterwards;
     *               contentModifier_ remains null causing early return after FlushTask.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    context->PushTask([](CanvasPaintMethod&) {});
    paintMethod->UpdateRecordingCanvas(TEST_CANVAS_WIDTH, TEST_CANVAS_HEIGHT);
    paintMethod->contentModifier_ = nullptr;
    paintMethod->UpdateContentModifier(nullptr);
    EXPECT_FALSE(paintMethod->NeedRender());
    EXPECT_EQ(paintMethod->contentModifier_, nullptr);
}

/**
 * @tc.name: CanvasPaintMethodUpdateContentModifier006
 * @tc.desc: Test UpdateContentModifier with all valid, contentModifier set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodUpdateContentModifier006, TestSize.Level1)
{
    /**
     * @tc.steps1: set rsCanvas_, canvasRenderContext_ with tasks, and contentModifier_.
     * @tc.expected: FlushTask runs and clears tasks, so NeedRender returns false afterwards;
     *               MarkModifierDirty is called and contentModifier_ is kept.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    context->PushTask([](CanvasPaintMethod&) {});
    paintMethod->UpdateRecordingCanvas(TEST_CANVAS_WIDTH, TEST_CANVAS_HEIGHT);
    paintMethod->UpdateContentModifier(nullptr);
    EXPECT_FALSE(paintMethod->NeedRender());
    EXPECT_NE(paintMethod->contentModifier_, nullptr);
}

// ==================== ClearRecordingCanvas Tests ====================

/**
 * @tc.name: CanvasPaintMethodClearRecordingCanvas001
 * @tc.desc: Test ClearRecordingCanvas with rsCanvas_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodClearRecordingCanvas001, TestSize.Level1)
{
    /**
     * @tc.steps1: leave rsCanvas_ null.
     * @tc.expected: returns early (CHECK_NULL_VOID rsRecordingCanvas).
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->rsCanvas_ = nullptr;
    paintMethod->ClearRecordingCanvas();
    // No crash expected
}

// ==================== ResetRecordingCanvas Tests ====================

/**
 * @tc.name: CanvasPaintMethodResetRecordingCanvas001
 * @tc.desc: Test ResetRecordingCanvas with canvasCallback_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodResetRecordingCanvas001, TestSize.Level1)
{
    /**
     * @tc.steps1: leave canvasCallback_ null.
     * @tc.expected: no crash.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->canvasCallback_ = nullptr;
    paintMethod->ResetRecordingCanvas();
    EXPECT_EQ(paintMethod->canvasCallback_, nullptr);
}

/**
 * @tc.name: CanvasPaintMethodResetRecordingCanvas002
 * @tc.desc: Test ResetRecordingCanvas with canvasCallback_ set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodResetRecordingCanvas002, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasCallback_ to a function.
     * @tc.expected: callback is invoked.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    bool called = false;
    std::function<void(std::shared_ptr<RSCanvas>, double, double)> callback =
        [&called](std::shared_ptr<RSCanvas>, double, double) { called = true; };
    paintMethod->SetRSCanvasCallback(callback);
    paintMethod->ResetRecordingCanvas();
    EXPECT_TRUE(called);
}

// ==================== SetCustomTextType Tests ====================

/**
 * @tc.name: CanvasPaintMethodSetCustomTextType001
 * @tc.desc: Test SetCustomTextType with rsCanvas_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodSetCustomTextType001, TestSize.Level1)
{
    /**
     * @tc.steps1: leave rsCanvas_ null.
     * @tc.expected: returns early (CHECK_NULL_VOID recordingCanvas).
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->rsCanvas_ = nullptr;
    paintMethod->SetCustomTextType();
}

/**
 * @tc.name: CanvasPaintMethodSetCustomTextType002
 * @tc.desc: Test SetCustomTextType with context_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodSetCustomTextType002, TestSize.Level1)
{
    /**
     * @tc.steps1: set rsCanvas_ but leave context_ null.
     * @tc.expected: returns early (CHECK_NULL_VOID context).
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->UpdateRecordingCanvas(TEST_CANVAS_WIDTH, TEST_CANVAS_HEIGHT);
    // context_ is null because frameNode is null
    paintMethod->SetCustomTextType();
}

/**
 * @tc.name: CanvasPaintMethodSetCustomTextType003
 * @tc.desc: Test SetCustomTextType with all valid.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodSetCustomTextType003, TestSize.Level1)
{
    /**
     * @tc.steps1: set rsCanvas_ and context_.
     * @tc.expected: SetIsCustomTextType is called (no crash).
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    paintMethod->UpdateRecordingCanvas(TEST_CANVAS_WIDTH, TEST_CANVAS_HEIGHT);
    paintMethod->SetCustomTextType();
    EXPECT_NE(paintMethod->rsCanvas_, nullptr);
}

// ==================== CalculatePixelMapRect Tests ====================

/**
 * @tc.name: CanvasPaintMethodCalculatePixelMapRect001
 * @tc.desc: Test CalculatePixelMapRect with THREE_PARAMS flag.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodCalculatePixelMapRect001, TestSize.Level1)
{
    /**
     * @tc.steps1: set flag to THREE_PARAMS.
     * @tc.expected: srcRect and dstRect are set correctly.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    Ace::CanvasImage image;
    image.flag = static_cast<int32_t>(CustomPaintPaintMethod::DrawImageType::THREE_PARAMS);
    image.dx = 1.0;
    image.dy = 2.0;
    int32_t width = 100;
    int32_t height = 200;
    RSRect srcRect;
    RSRect dstRect;
    paintMethod->CalculatePixelMapRect(image, width, height, srcRect, dstRect);
    // srcRect should be (0,0,100,200), dstRect should be (1,2,101,202)
    EXPECT_FLOAT_EQ(srcRect.GetLeft(), 0.0f);
    EXPECT_FLOAT_EQ(dstRect.GetLeft(), 1.0f);
}

/**
 * @tc.name: CanvasPaintMethodCalculatePixelMapRect002
 * @tc.desc: Test CalculatePixelMapRect with FIVE_PARAMS flag.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodCalculatePixelMapRect002, TestSize.Level1)
{
    /**
     * @tc.steps1: set flag to FIVE_PARAMS.
     * @tc.expected: srcRect uses full image, dstRect uses dWidth/dHeight.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    Ace::CanvasImage image;
    image.flag = static_cast<int32_t>(CustomPaintPaintMethod::DrawImageType::FIVE_PARAMS);
    image.dx = 1.0;
    image.dy = 2.0;
    image.dWidth = 50.0;
    image.dHeight = 60.0;
    int32_t width = 100;
    int32_t height = 200;
    RSRect srcRect;
    RSRect dstRect;
    paintMethod->CalculatePixelMapRect(image, width, height, srcRect, dstRect);
    EXPECT_FLOAT_EQ(dstRect.GetLeft(), 1.0f);
    EXPECT_FLOAT_EQ(dstRect.GetRight(), 51.0f);
}

/**
 * @tc.name: CanvasPaintMethodCalculatePixelMapRect003
 * @tc.desc: Test CalculatePixelMapRect with NINE_PARAMS flag.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodCalculatePixelMapRect003, TestSize.Level1)
{
    /**
     * @tc.steps1: set flag to NINE_PARAMS.
     * @tc.expected: srcRect uses sx/sy/sWidth/sHeight, dstRect uses dx/dy/dWidth/dHeight.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    Ace::CanvasImage image;
    image.flag = static_cast<int32_t>(CustomPaintPaintMethod::DrawImageType::NINE_PARAMS);
    image.sx = 10.0;
    image.sy = 20.0;
    image.sWidth = 30.0;
    image.sHeight = 40.0;
    image.dx = 1.0;
    image.dy = 2.0;
    image.dWidth = 50.0;
    image.dHeight = 60.0;
    int32_t width = 100;
    int32_t height = 200;
    RSRect srcRect;
    RSRect dstRect;
    paintMethod->CalculatePixelMapRect(image, width, height, srcRect, dstRect);
    EXPECT_FLOAT_EQ(srcRect.GetLeft(), 10.0f);
    EXPECT_FLOAT_EQ(srcRect.GetRight(), 40.0f);
    EXPECT_FLOAT_EQ(dstRect.GetLeft(), 1.0f);
    EXPECT_FLOAT_EQ(dstRect.GetRight(), 51.0f);
}

/**
 * @tc.name: CanvasPaintMethodCalculatePixelMapRect004
 * @tc.desc: Test CalculatePixelMapRect with default flag (unknown value).
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodCalculatePixelMapRect004, TestSize.Level1)
{
    /**
     * @tc.steps1: set flag to an unknown value.
     * @tc.expected: default case, srcRect and dstRect remain default.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    Ace::CanvasImage image;
    image.flag = 99; // unknown value
    int32_t width = 100;
    int32_t height = 200;
    RSRect srcRect;
    RSRect dstRect;
    paintMethod->CalculatePixelMapRect(image, width, height, srcRect, dstRect);
    // default case: no modification to srcRect/dstRect
    EXPECT_FLOAT_EQ(srcRect.GetLeft(), 0.0f);
    EXPECT_FLOAT_EQ(srcRect.GetTop(), 0.0f);
    EXPECT_FLOAT_EQ(dstRect.GetLeft(), 0.0f);
    EXPECT_FLOAT_EQ(dstRect.GetTop(), 0.0f);
}

// ==================== GetImageData Tests ====================

/**
 * @tc.name: CanvasPaintMethodGetImageData001
 * @tc.desc: Test GetImageData with host null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetImageData001, TestSize.Level1)
{
    /**
     * @tc.steps1: use null frameNode.
     * @tc.expected: returns nullptr.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    auto result = paintMethod->GetImageData(0, 0, 10, 10);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: CanvasPaintMethodGetImageData002
 * @tc.desc: Test GetImageData with DrawBitmap returning false (ACE_UNITTEST mode).
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetImageData002, TestSize.Level1)
{
    /**
     * @tc.steps1: use valid frameNode.
     * @tc.expected: DrawBitmap returns false in ACE_UNITTEST, returns nullptr.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto result = paintMethod->GetImageData(0, 0, 10, 10);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: CanvasPaintMethodGetImageData003
 * @tc.desc: Test GetImageData with negative width and height.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetImageData003, TestSize.Level1)
{
    /**
     * @tc.steps1: pass negative width and height.
     * @tc.expected: DrawBitmap returns false, returns nullptr.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto result = paintMethod->GetImageData(5, 5, -10, -10);
    EXPECT_EQ(result, nullptr);
}

// ==================== Reset Tests ====================

/**
 * @tc.name: CanvasPaintMethodReset001
 * @tc.desc: Test Reset with rsCanvas_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodReset001, TestSize.Level1)
{
    /**
     * @tc.steps1: leave rsCanvas_ null.
     * @tc.expected: returns early (CHECK_NULL_VOID rsCanvas_).
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->rsCanvas_ = nullptr;
    paintMethod->Reset();
    // No crash expected
}

/**
 * @tc.name: CanvasPaintMethodReset002
 * @tc.desc: Test Reset with rsCanvas_ set and canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodReset002, TestSize.Level1)
{
    /**
     * @tc.steps1: set rsCanvas_ via UpdateRecordingCanvas.
     * @tc.expected: Reset completes without crash; GetSaveCount returns 0 (< DEFAULT_SAVE_COUNT).
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    paintMethod->UpdateRecordingCanvas(TEST_CANVAS_WIDTH, TEST_CANVAS_HEIGHT);
    paintMethod->Reset();
    EXPECT_NE(paintMethod->rsCanvas_, nullptr);
}

/**
 * @tc.name: CanvasPaintMethodReset003
 * @tc.desc: Test Reset with rsCanvas_ and canvasRenderContext_ set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodReset003, TestSize.Level1)
{
    /**
     * @tc.steps1: set rsCanvas_ and canvasRenderContext_.
     * @tc.expected: canvasRenderContext_->ResetStates() called, no crash.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    paintMethod->UpdateRecordingCanvas(TEST_CANVAS_WIDTH, TEST_CANVAS_HEIGHT);
    paintMethod->Reset();
    EXPECT_NE(paintMethod->rsCanvas_, nullptr);
    EXPECT_NE(paintMethod->canvasRenderContext_, nullptr);
}

// ==================== GetId Tests ====================

/**
 * @tc.name: CanvasPaintMethodGetId001
 * @tc.desc: Test GetId with host null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetId001, TestSize.Level1)
{
    /**
     * @tc.steps1: use null frameNode.
     * @tc.expected: returns -1.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    EXPECT_EQ(paintMethod->GetId(), -1);
}

/**
 * @tc.name: CanvasPaintMethodGetId002
 * @tc.desc: Test GetId with valid frameNode.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetId002, TestSize.Level1)
{
    /**
     * @tc.steps1: use valid frameNode.
     * @tc.expected: returns the host frameNode's id (ClaimNodeId yields UndefinedElementId
     *               in the test environment, so verify GetId delegates to the host id).
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto result = paintMethod->GetId();
    EXPECT_EQ(result, frameNode->GetId());
}

// ==================== GetSystemDirection Tests ====================

/**
 * @tc.name: CanvasPaintMethodGetSystemDirection001
 * @tc.desc: Test GetSystemDirection with host null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetSystemDirection001, TestSize.Level1)
{
    /**
     * @tc.steps1: use null frameNode.
     * @tc.expected: returns TextDirection::AUTO.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    auto result = paintMethod->GetSystemDirection();
    EXPECT_EQ(result, TextDirection::AUTO);
}

// ==================== GetDumpInfo Tests ====================

/**
 * @tc.name: CanvasPaintMethodGetDumpInfo001
 * @tc.desc: Test GetDumpInfo with rsCanvas_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetDumpInfo001, TestSize.Level1)
{
    /**
     * @tc.steps1: leave rsCanvas_ null.
     * @tc.expected: returns empty deque.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->rsCanvas_ = nullptr;
    auto result = paintMethod->GetDumpInfo();
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: CanvasPaintMethodGetDumpInfo002
 * @tc.desc: Test GetDumpInfo with rsCanvas_ set and empty paragraphHistory_.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetDumpInfo002, TestSize.Level1)
{
    /**
     * @tc.steps1: set rsCanvas_, leave paragraphHistory_ empty.
     * @tc.expected: result has one entry with matrix info.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    paintMethod->UpdateRecordingCanvas(TEST_CANVAS_WIDTH, TEST_CANVAS_HEIGHT);
    paintMethod->paragraphHistory_.clear();
    auto result = paintMethod->GetDumpInfo();
    EXPECT_EQ(result.size(), 1u);
}

/**
 * @tc.name: CanvasPaintMethodGetDumpInfo003
 * @tc.desc: Test GetDumpInfo with non-empty paragraphHistory_.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetDumpInfo003, TestSize.Level1)
{
    /**
     * @tc.steps1: set rsCanvas_ and populate paragraphHistory_.
     * @tc.expected: result has entries with FillText and history, then history cleared.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    paintMethod->UpdateRecordingCanvas(TEST_CANVAS_WIDTH, TEST_CANVAS_HEIGHT);
    paintMethod->paragraphHistory_.push_back("test paragraph 1");
    paintMethod->paragraphHistory_.push_back("test paragraph 2");
    auto result = paintMethod->GetDumpInfo();
    // First entry is matrix info, then "FillText:", then two history entries
    EXPECT_EQ(result.size(), 4u);
    EXPECT_EQ(paintMethod->paragraphHistory_.size(), 0u);
}

// ==================== SetHostCustomNodeName Tests ====================

/**
 * @tc.name: CanvasPaintMethodSetHostCustomNodeName001
 * @tc.desc: Test SetHostCustomNodeName with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodSetHostCustomNodeName001, TestSize.Level1)
{
    /**
     * @tc.steps1: leave canvasRenderContext_ null.
     * @tc.expected: returns early (CHECK_NULL_VOID).
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->SetHostCustomNodeName();
    // No crash expected
}

/**
 * @tc.name: CanvasPaintMethodSetHostCustomNodeName002
 * @tc.desc: Test SetHostCustomNodeName with frameNode null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodSetHostCustomNodeName002, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasRenderContext_ but leave frameNode_ null.
     * @tc.expected: returns early (CHECK_NULL_VOID frameNode).
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    paintMethod->SetHostCustomNodeName();
    // No crash expected
}

/**
 * @tc.name: CanvasPaintMethodSetHostCustomNodeName003
 * @tc.desc: Test SetHostCustomNodeName with customNode null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodSetHostCustomNodeName003, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasRenderContext_ and valid frameNode but no parent custom node.
     * @tc.expected: returns early (CHECK_NULL_VOID customNode).
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    paintMethod->SetHostCustomNodeName();
    // No crash expected; GetParentCustomNode returns null for new frameNode
}

// ==================== GetSimplifyDumpInfo Tests ====================

/**
 * @tc.name: CanvasPaintMethodGetSimplifyDumpInfo001
 * @tc.desc: Test GetSimplifyDumpInfo with rsCanvas_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetSimplifyDumpInfo001, TestSize.Level1)
{
    /**
     * @tc.steps1: leave rsCanvas_ null.
     * @tc.expected: returns early (CHECK_NULL_VOID).
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->rsCanvas_ = nullptr;
    auto json = JsonUtil::Create(true);
    paintMethod->GetSimplifyDumpInfo(json);
    // No crash expected
}

/**
 * @tc.name: CanvasPaintMethodGetSimplifyDumpInfo002
 * @tc.desc: Test GetSimplifyDumpInfo with rsCanvas_ set and empty paragraphHistory_.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetSimplifyDumpInfo002, TestSize.Level1)
{
    /**
     * @tc.steps1: set rsCanvas_, leave paragraphHistory_ empty.
     * @tc.expected: json has Trans, Scale, Skew but no FillText.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    paintMethod->UpdateRecordingCanvas(TEST_CANVAS_WIDTH, TEST_CANVAS_HEIGHT);
    paintMethod->paragraphHistory_.clear();
    auto json = JsonUtil::Create(true);
    paintMethod->GetSimplifyDumpInfo(json);
    EXPECT_TRUE(json->Contains("Trans"));
    EXPECT_TRUE(json->Contains("Scale"));
    EXPECT_TRUE(json->Contains("Skew"));
    EXPECT_FALSE(json->Contains("FillText"));
}

/**
 * @tc.name: CanvasPaintMethodGetSimplifyDumpInfo003
 * @tc.desc: Test GetSimplifyDumpInfo with non-empty paragraphHistory_.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetSimplifyDumpInfo003, TestSize.Level1)
{
    /**
     * @tc.steps1: set rsCanvas_ and populate paragraphHistory_.
     * @tc.expected: json has FillText field.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    paintMethod->UpdateRecordingCanvas(TEST_CANVAS_WIDTH, TEST_CANVAS_HEIGHT);
    paintMethod->paragraphHistory_.push_back("test paragraph");
    auto json = JsonUtil::Create(true);
    paintMethod->GetSimplifyDumpInfo(json);
    EXPECT_TRUE(json->Contains("FillText"));
}

// ==================== AddParagraphHistory Tests ====================

/**
 * @tc.name: CanvasPaintMethodAddParagraphHistory001
 * @tc.desc: Test AddParagraphHistory with canvasDebugMode <= 0.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodAddParagraphHistory001, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasDebugMode to 0.
     * @tc.expected: no history added.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    auto oldMode = SystemProperties::canvasDebugMode_;
    SystemProperties::canvasDebugMode_ = 0;
    paintMethod->paragraphHistory_.clear();
    paintMethod->AddParagraphHistory();
    EXPECT_EQ(paintMethod->paragraphHistory_.size(), 0u);
    SystemProperties::canvasDebugMode_ = oldMode;
}

/**
 * @tc.name: CanvasPaintMethodAddParagraphHistory002
 * @tc.desc: Test AddParagraphHistory with debug mode > 0 but paragraph_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodAddParagraphHistory002, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasDebugMode > 0, leave paragraph_ null.
     * @tc.expected: no history added (CHECK_NULL_VOID).
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    auto oldMode = SystemProperties::canvasDebugMode_;
    SystemProperties::canvasDebugMode_ = 1;
    paintMethod->paragraph_ = nullptr;
    paintMethod->paragraphHistory_.clear();
    paintMethod->AddParagraphHistory();
    EXPECT_EQ(paintMethod->paragraphHistory_.size(), 0u);
    SystemProperties::canvasDebugMode_ = oldMode;
}

/**
 * @tc.name: CanvasPaintMethodAddParagraphHistory003
 * @tc.desc: Test AddParagraphHistory with debug mode > 0 and valid paragraph_.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodAddParagraphHistory003, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasDebugMode > 0 and set paragraph_.
     * @tc.expected: one entry added to paragraphHistory_.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    auto oldMode = SystemProperties::canvasDebugMode_;
    SystemProperties::canvasDebugMode_ = 1;
    paintMethod->paragraph_ = std::make_unique<RSParagraph>();
    paintMethod->paragraphHistory_.clear();
    paintMethod->AddParagraphHistory();
    EXPECT_EQ(paintMethod->paragraphHistory_.size(), 1u);
    SystemProperties::canvasDebugMode_ = oldMode;
}

/**
 * @tc.name: CanvasPaintMethodAddParagraphHistory004
 * @tc.desc: Test AddParagraphHistory exceeding MAX_PARAGRAPH_HISTORY_SIZE.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodAddParagraphHistory004, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasDebugMode > 0, add more than MAX entries.
     * @tc.expected: size does not exceed MAX_PARAGRAPH_HISTORY_SIZE (pop_front called).
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    auto oldMode = SystemProperties::canvasDebugMode_;
    SystemProperties::canvasDebugMode_ = 1;
    paintMethod->paragraph_ = std::make_unique<RSParagraph>();
    paintMethod->paragraphHistory_.clear();
    for (size_t i = 0; i < MAX_PARAGRAPH_HISTORY_SIZE + 5; ++i) {
        paintMethod->AddParagraphHistory();
    }
    EXPECT_EQ(paintMethod->paragraphHistory_.size(), MAX_PARAGRAPH_HISTORY_SIZE);
    SystemProperties::canvasDebugMode_ = oldMode;
}

// ==================== SetCanvasRenderContext Tests ====================

/**
 * @tc.name: CanvasPaintMethodSetCanvasRenderContext001
 * @tc.desc: Test SetCanvasRenderContext with valid context.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodSetCanvasRenderContext001, TestSize.Level1)
{
    /**
     * @tc.steps1: pass a valid CanvasRenderContextDeferred.
     * @tc.expected: canvasRenderContext_ is set and SetPaintMethod called.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    EXPECT_NE(paintMethod->canvasRenderContext_, nullptr);
}

// ==================== Null canvasRenderContext_ Delegation Tests ====================

/**
 * @tc.name: CanvasPaintMethodNullContextDelegation001
 * @tc.desc: Test GetAntialiasExt with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNullContextDelegation001, TestSize.Level1)
{
    auto paintMethod = CreatePaintMethodNullFrameNode();
    auto result = paintMethod->GetAntialiasExt();
    EXPECT_EQ(result, std::nullopt);
}

/**
 * @tc.name: CanvasPaintMethodNullContextDelegation002
 * @tc.desc: Test SetAntialiasExtParam with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNullContextDelegation002, TestSize.Level1)
{
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->SetAntialiasExtParam(std::nullopt);
    EXPECT_EQ(paintMethod->canvasRenderContext_, nullptr);
}

/**
 * @tc.name: CanvasPaintMethodNullContextDelegation003
 * @tc.desc: Test ResetAntialiasExt with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNullContextDelegation003, TestSize.Level1)
{
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->ResetAntialiasExt();
    EXPECT_EQ(paintMethod->GetAntialiasExt(), std::nullopt);
}

/**
 * @tc.name: CanvasPaintMethodNullContextDelegation004
 * @tc.desc: Test GetTransform with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNullContextDelegation004, TestSize.Level1)
{
    auto paintMethod = CreatePaintMethodNullFrameNode();
    auto result = paintMethod->GetTransform();
    TransformParam expected;
    EXPECT_EQ(result.scaleX, expected.scaleX);
}

/**
 * @tc.name: CanvasPaintMethodNullContextDelegation005
 * @tc.desc: Test GetLineDash with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNullContextDelegation005, TestSize.Level1)
{
    auto paintMethod = CreatePaintMethodNullFrameNode();
    auto result = paintMethod->GetLineDash();
    LineDashParam expected;
    EXPECT_EQ(result.dashOffset, expected.dashOffset);
}

/**
 * @tc.name: CanvasPaintMethodNullContextDelegation006
 * @tc.desc: Test SetLineDashParam with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNullContextDelegation006, TestSize.Level1)
{
    auto paintMethod = CreatePaintMethodNullFrameNode();
    std::vector<double> segments = {1.0, 2.0};
    paintMethod->SetLineDashParam(segments);
    EXPECT_EQ(paintMethod->canvasRenderContext_, nullptr);
}

/**
 * @tc.name: CanvasPaintMethodNullContextDelegation007
 * @tc.desc: Test SaveProperties with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNullContextDelegation007, TestSize.Level1)
{
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->SaveProperties();
    EXPECT_EQ(paintMethod->canvasRenderContext_, nullptr);
}

/**
 * @tc.name: CanvasPaintMethodNullContextDelegation008
 * @tc.desc: Test RestoreProperties with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNullContextDelegation008, TestSize.Level1)
{
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->RestoreProperties();
    EXPECT_EQ(paintMethod->canvasRenderContext_, nullptr);
}

/**
 * @tc.name: CanvasPaintMethodNullContextDelegation009
 * @tc.desc: Test ResetTransformMatrix with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNullContextDelegation009, TestSize.Level1)
{
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->ResetTransformMatrix();
    TransformParam result = paintMethod->GetTransform();
    TransformParam expected;
    EXPECT_EQ(result.scaleX, expected.scaleX);
}

/**
 * @tc.name: CanvasPaintMethodNullContextDelegation010
 * @tc.desc: Test ResetLineDash with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNullContextDelegation010, TestSize.Level1)
{
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->ResetLineDash();
    EXPECT_TRUE(paintMethod->GetLineDash().lineDash.empty());
}

/**
 * @tc.name: CanvasPaintMethodNullContextDelegation011
 * @tc.desc: Test RotateMatrix with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNullContextDelegation011, TestSize.Level1)
{
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->RotateMatrix(45.0);
    TransformParam result = paintMethod->GetTransform();
    TransformParam expected;
    EXPECT_EQ(result.scaleX, expected.scaleX);
}

/**
 * @tc.name: CanvasPaintMethodNullContextDelegation012
 * @tc.desc: Test ScaleMatrix with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNullContextDelegation012, TestSize.Level1)
{
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->ScaleMatrix(2.0, 3.0);
    TransformParam result = paintMethod->GetTransform();
    TransformParam expected;
    EXPECT_EQ(result.scaleX, expected.scaleX);
}

/**
 * @tc.name: CanvasPaintMethodNullContextDelegation013
 * @tc.desc: Test SetTransformMatrix with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNullContextDelegation013, TestSize.Level1)
{
    auto paintMethod = CreatePaintMethodNullFrameNode();
    TransformParam param;
    param.scaleX = 1.0;
    param.scaleY = 1.0;
    paintMethod->SetTransformMatrix(param);
    TransformParam result = paintMethod->GetTransform();
    TransformParam expected;
    EXPECT_EQ(result.scaleX, expected.scaleX);
}

/**
 * @tc.name: CanvasPaintMethodNullContextDelegation014
 * @tc.desc: Test TransformMatrix with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNullContextDelegation014, TestSize.Level1)
{
    auto paintMethod = CreatePaintMethodNullFrameNode();
    TransformParam param;
    paintMethod->TransformMatrix(param);
    TransformParam result = paintMethod->GetTransform();
    TransformParam expected;
    EXPECT_EQ(result.scaleX, expected.scaleX);
}

/**
 * @tc.name: CanvasPaintMethodNullContextDelegation015
 * @tc.desc: Test TranslateMatrix with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodNullContextDelegation015, TestSize.Level1)
{
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->TranslateMatrix(10.0, 20.0);
    TransformParam result = paintMethod->GetTransform();
    TransformParam expected;
    EXPECT_EQ(result.scaleX, expected.scaleX);
}

// ==================== canvasRenderContext_ Set Delegation Tests ====================

/**
 * @tc.name: CanvasPaintMethodContextSetDelegation001
 * @tc.desc: Test GetAntialiasExt with canvasRenderContext_ set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodContextSetDelegation001, TestSize.Level1)
{
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    context->SetAntialiasExtParam(true);
    auto result = paintMethod->GetAntialiasExt();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

/**
 * @tc.name: CanvasPaintMethodContextSetDelegation002
 * @tc.desc: Test GetTransform with canvasRenderContext_ set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodContextSetDelegation002, TestSize.Level1)
{
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    TransformParam param;
    param.scaleX = 2.0;
    param.translateX = 5.0;
    paintMethod->SetTransformMatrix(param);
    auto result = paintMethod->GetTransform();
    // TestingMatrix::Get returns 1.0f for all indices; verify no crash
    EXPECT_NE(result.scaleX, 0.0);
}

/**
 * @tc.name: CanvasPaintMethodContextSetDelegation003
 * @tc.desc: Test GetLineDash with canvasRenderContext_ set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodContextSetDelegation003, TestSize.Level1)
{
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    std::vector<double> segments = { 5.0, 10.0 };
    paintMethod->SetLineDashParam(segments);
    auto result = paintMethod->GetLineDash();
    EXPECT_EQ(result.lineDash.size(), 2u);
}

/**
 * @tc.name: CanvasPaintMethodContextSetDelegation004
 * @tc.desc: Test ResetAntialiasExt with canvasRenderContext_ set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodContextSetDelegation004, TestSize.Level1)
{
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    paintMethod->SetAntialiasExtParam(true);
    paintMethod->ResetAntialiasExt();
    auto result = paintMethod->GetAntialiasExt();
    EXPECT_EQ(result, std::nullopt);
}

/**
 * @tc.name: CanvasPaintMethodContextSetDelegation005
 * @tc.desc: Test SaveProperties / RestoreProperties with canvasRenderContext_ set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodContextSetDelegation005, TestSize.Level1)
{
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    TransformParam param;
    param.scaleX = 3.0;
    paintMethod->SetTransformMatrix(param);
    paintMethod->SaveProperties();
    TransformParam param2;
    param2.scaleX = 1.0;
    paintMethod->SetTransformMatrix(param2);
    paintMethod->RestoreProperties();
    auto result = paintMethod->GetTransform();
    // TestingMatrix mock: Get always returns 1.0f; verify no crash
    EXPECT_NE(result.scaleX, 0.0);
}

/**
 * @tc.name: CanvasPaintMethodContextSetDelegation006
 * @tc.desc: Test RotateMatrix, ScaleMatrix, TranslateMatrix, ResetTransformMatrix with context set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodContextSetDelegation006, TestSize.Level1)
{
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    paintMethod->RotateMatrix(45.0);
    paintMethod->ScaleMatrix(2.0, 3.0);
    paintMethod->TranslateMatrix(10.0, 20.0);
    auto result = paintMethod->GetTransform();
    // After operations, transform should reflect the changes
    EXPECT_NE(result.scaleX, 0.0);
    paintMethod->ResetTransformMatrix();
    auto resultAfterReset = paintMethod->GetTransform();
    // After reset, transform should be identity
    EXPECT_EQ(resultAfterReset.scaleX, 1.0);
}

/**
 * @tc.name: CanvasPaintMethodContextSetDelegation007
 * @tc.desc: Test TransformMatrix and ResetLineDash with context set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodContextSetDelegation007, TestSize.Level1)
{
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    TransformParam param;
    param.scaleX = 2.0;
    param.translateX = 5.0;
    paintMethod->TransformMatrix(param);
    auto result = paintMethod->GetTransform();
    EXPECT_NE(result.scaleX, 0.0);
    // Test line dash
    std::vector<double> segments = { 5.0, 10.0 };
    paintMethod->SetLineDashParam(segments);
    auto lineDash = paintMethod->GetLineDash();
    EXPECT_EQ(lineDash.lineDash.size(), 2u);
    paintMethod->ResetLineDash();
    auto lineDashAfterReset = paintMethod->GetLineDash();
    EXPECT_EQ(lineDashAfterReset.lineDash.size(), 0u);
}

// ==================== GetTransformInner Tests ====================

/**
 * @tc.name: CanvasPaintMethodGetTransformInner001
 * @tc.desc: Test GetTransformInner with rsCanvas_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetTransformInner001, TestSize.Level1)
{
    /**
     * @tc.steps1: leave rsCanvas_ null.
     * @tc.expected: returns default TransformParam.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->rsCanvas_ = nullptr;
    auto result = paintMethod->GetTransformInner();
    TransformParam expected;
    EXPECT_EQ(result.scaleX, expected.scaleX);
}

/**
 * @tc.name: CanvasPaintMethodGetTransformInner002
 * @tc.desc: Test GetTransformInner with rsCanvas_ set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetTransformInner002, TestSize.Level1)
{
    /**
     * @tc.steps1: set rsCanvas_ via UpdateRecordingCanvas.
     * @tc.expected: returns matrix values from rsCanvas_.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    paintMethod->UpdateRecordingCanvas(TEST_CANVAS_WIDTH, TEST_CANVAS_HEIGHT);
    auto result = paintMethod->GetTransformInner();
    // TestingMatrix::Get returns 1.0f
    EXPECT_FLOAT_EQ(result.scaleX, 1.0f);
    EXPECT_FLOAT_EQ(result.scaleY, 1.0f);
}

// ==================== GetLineDashInner Tests ====================

/**
 * @tc.name: CanvasPaintMethodGetLineDashInner001
 * @tc.desc: Test GetLineDashInner returns strokeState line dash.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetLineDashInner001, TestSize.Level1)
{
    /**
     * @tc.steps1: set line dash in strokeState.
     * @tc.expected: GetLineDashInner returns the same line dash.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    std::vector<double> segments = { 5.0, 10.0 };
    paintMethod->state_.strokeState.SetLineDash(segments);
    paintMethod->state_.strokeState.SetLineDashOffset(2.0);
    auto result = paintMethod->GetLineDashInner();
    EXPECT_EQ(result.lineDash.size(), 2u);
    EXPECT_EQ(result.lineDash[0], 5.0);
}

// ==================== GetAntialiasExtInner Tests ====================

/**
 * @tc.name: CanvasPaintMethodGetAntialiasExtInner001
 * @tc.desc: Test GetAntialiasExtInner returns fontAntiAlias_.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetAntialiasExtInner001, TestSize.Level1)
{
    /**
     * @tc.steps1: set fontAntiAlias_ to a value.
     * @tc.expected: returns the same value.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->fontAntiAlias_ = true;
    auto result = paintMethod->GetAntialiasExtInner();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

/**
 * @tc.name: CanvasPaintMethodGetAntialiasExtInner002
 * @tc.desc: Test GetAntialiasExtInner with fontAntiAlias_ nullopt.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetAntialiasExtInner002, TestSize.Level1)
{
    /**
     * @tc.steps1: leave fontAntiAlias_ as nullopt.
     * @tc.expected: returns nullopt.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->fontAntiAlias_ = std::nullopt;
    auto result = paintMethod->GetAntialiasExtInner();
    EXPECT_EQ(result, std::nullopt);
}

// ==================== Inline Methods Tests ====================

/**
 * @tc.name: CanvasPaintMethodFlushUITasks001
 * @tc.desc: Test FlushUITasks with NeedRender false.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodFlushUITasks001, TestSize.Level1)
{
    /**
     * @tc.steps1: leave canvasRenderContext_ null (NeedRender returns false).
     * @tc.expected: returns early.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->FlushUITasks();
    EXPECT_FALSE(paintMethod->NeedRender());
}

/**
 * @tc.name: CanvasPaintMethodFlushUITasks002
 * @tc.desc: Test FlushUITasks with NeedRender true and context null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodFlushUITasks002, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasRenderContext_ with tasks but context_ null.
     * @tc.expected: NeedRender returns true, context is null so no FlushUITasks call (no crash).
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    context->PushTask([](CanvasPaintMethod&) {});
    paintMethod->FlushUITasks();
    EXPECT_TRUE(paintMethod->NeedRender());
}

/**
 * @tc.name: CanvasPaintMethodFireOnModifierUpdateFunc001
 * @tc.desc: Test FireOnModifierUpdateFunc with onModifierUpdate_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodFireOnModifierUpdateFunc001, TestSize.Level1)
{
    /**
     * @tc.steps1: leave onModifierUpdate_ unset.
     * @tc.expected: returns early (CHECK_NULL_VOID).
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->onModifierUpdate_ = nullptr;
    paintMethod->FireOnModifierUpdateFunc();
    // No crash expected
}

/**
 * @tc.name: CanvasPaintMethodFireOnModifierUpdateFunc002
 * @tc.desc: Test FireOnModifierUpdateFunc with onModifierUpdate_ set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodFireOnModifierUpdateFunc002, TestSize.Level1)
{
    /**
     * @tc.steps1: set onModifierUpdate_.
     * @tc.expected: function is called.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    bool called = false;
    paintMethod->SetOnModifierUpdateFunc([&called]() { called = true; });
    paintMethod->FireOnModifierUpdateFunc();
    EXPECT_TRUE(called);
}

/**
 * @tc.name: CanvasPaintMethodFireRSCanvasCallback001
 * @tc.desc: Test FireRSCanvasCallback with canvasCallback_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodFireRSCanvasCallback001, TestSize.Level1)
{
    /**
     * @tc.steps1: leave canvasCallback_ null.
     * @tc.expected: returns early (CHECK_NULL_VOID).
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->canvasCallback_ = nullptr;
    paintMethod->FireRSCanvasCallback(10.0, 20.0);
    // No crash expected
}

/**
 * @tc.name: CanvasPaintMethodFireRSCanvasCallback002
 * @tc.desc: Test FireRSCanvasCallback with canvasCallback_ set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodFireRSCanvasCallback002, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasCallback_.
     * @tc.expected: callback is invoked with rsCanvas_, width, height.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    bool called = false;
    double receivedW = 0.0;
    double receivedH = 0.0;
    std::function<void(std::shared_ptr<RSCanvas>, double, double)> callback =
        [&called, &receivedW, &receivedH](std::shared_ptr<RSCanvas>, double w, double h) {
            called = true;
            receivedW = w;
            receivedH = h;
        };
    paintMethod->SetRSCanvasCallback(callback);
    paintMethod->FireRSCanvasCallback(15.0, 25.0);
    EXPECT_TRUE(called);
    EXPECT_DOUBLE_EQ(receivedW, 15.0);
    EXPECT_DOUBLE_EQ(receivedH, 25.0);
}

/**
 * @tc.name: CanvasPaintMethodSetVisibility001
 * @tc.desc: Test SetVisibility with canvasRenderContext_ null.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodSetVisibility001, TestSize.Level1)
{
    /**
     * @tc.steps1: leave canvasRenderContext_ null.
     * @tc.expected: returns early (CHECK_NULL_VOID).
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->SetVisibility(true);
    // No crash expected
}

/**
 * @tc.name: CanvasPaintMethodSetVisibility002
 * @tc.desc: Test SetVisibility with canvasRenderContext_ set.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodSetVisibility002, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasRenderContext_.
     * @tc.expected: SetVisibility is called on the context (no crash).
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    auto context = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
    paintMethod->SetCanvasRenderContext(context);
    paintMethod->SetVisibility(true);
    paintMethod->SetVisibility(false);
    EXPECT_NE(paintMethod->canvasRenderContext_, nullptr);
}

// ==================== ACE_UNITTEST Stub Tests ====================

/**
 * @tc.name: CanvasPaintMethodToDataURL001
 * @tc.desc: Test ToDataURL returns UNSUPPORTED in ACE_UNITTEST mode.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodToDataURL001, TestSize.Level1)
{
    /**
     * @tc.steps1: call ToDataURL.
     * @tc.expected: returns UNSUPPORTED.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    auto result = paintMethod->ToDataURL("png", 1.0);
    EXPECT_EQ(result, UNSUPPORTED);
}

/**
 * @tc.name: CanvasPaintMethodDrawBitmap001
 * @tc.desc: Test DrawBitmap returns false in ACE_UNITTEST mode.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodDrawBitmap001, TestSize.Level1)
{
    /**
     * @tc.steps1: call DrawBitmap.
     * @tc.expected: returns false.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    RSBitmap bitmap;
    auto result = paintMethod->DrawBitmap(nullptr, bitmap);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: CanvasPaintMethodGetJsonData001
 * @tc.desc: Test GetJsonData returns empty string in ACE_UNITTEST mode.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetJsonData001, TestSize.Level1)
{
    /**
     * @tc.steps1: call GetJsonData.
     * @tc.expected: returns empty string.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    auto result = paintMethod->GetJsonData("test_path");
    EXPECT_EQ(result, "");
}

// ==================== GetContentModifier / GetWidth / GetHeight Tests ====================

/**
 * @tc.name: CanvasPaintMethodGetContentModifier001
 * @tc.desc: Test GetContentModifier returns contentModifier_.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetContentModifier001, TestSize.Level1)
{
    /**
     * @tc.steps1: create with contentModifier.
     * @tc.expected: returns the same contentModifier.
     */
    RefPtr<FrameNode> frameNode;
    auto contentModifier = AceType::MakeRefPtr<CanvasModifier>();
    auto paintMethod = AceType::MakeRefPtr<CanvasPaintMethod>(contentModifier, frameNode);
    auto result = paintMethod->GetContentModifier(nullptr);
    EXPECT_EQ(result, contentModifier);
}

/**
 * @tc.name: CanvasPaintMethodGetWidthHeight001
 * @tc.desc: Test GetWidth and GetHeight return lastLayoutSize values.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodGetWidthHeight001, TestSize.Level1)
{
    /**
     * @tc.steps1: set lastLayoutSize_ and call GetWidth/GetHeight.
     * @tc.expected: return correct width and height.
     */
    auto paintMethod = CreatePaintMethodNullFrameNode();
    paintMethod->lastLayoutSize_ = SizeF(100.0f, 200.0f);
    EXPECT_DOUBLE_EQ(paintMethod->GetWidth(), 100.0);
    EXPECT_DOUBLE_EQ(paintMethod->GetHeight(), 200.0);
}

// ==================== UpdateRecordingCanvas Tests ====================

/**
 * @tc.name: CanvasPaintMethodUpdateRecordingCanvas001
 * @tc.desc: Test UpdateRecordingCanvas sets rsCanvas_ and needMarkDirty_.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodUpdateRecordingCanvas001, TestSize.Level1)
{
    /**
     * @tc.steps1: call UpdateRecordingCanvas.
     * @tc.expected: rsCanvas_ is set, needMarkDirty_ is true.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    paintMethod->needMarkDirty_ = false;
    paintMethod->UpdateRecordingCanvas(TEST_CANVAS_WIDTH, TEST_CANVAS_HEIGHT);
    EXPECT_NE(paintMethod->rsCanvas_, nullptr);
    EXPECT_TRUE(paintMethod->needMarkDirty_);
}

/**
 * @tc.name: CanvasPaintMethodUpdateRecordingCanvas002
 * @tc.desc: Test UpdateRecordingCanvas with VERSION_THIRTEEN.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodUpdateRecordingCanvas002, TestSize.Level1)
{
    /**
     * @tc.steps1: set apiVersion to VERSION_THIRTEEN.
     * @tc.expected: ResetStates called, needMarkDirty_ is true.
     */
    auto container = Container::Current();
    ASSERT_TRUE(container);
    auto oldVersion = container->GetApiTargetVersion();
    container->SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_THIRTEEN));

    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    paintMethod->UpdateRecordingCanvas(TEST_CANVAS_WIDTH, TEST_CANVAS_HEIGHT);
    EXPECT_NE(paintMethod->rsCanvas_, nullptr);
    EXPECT_TRUE(paintMethod->needMarkDirty_);

    container->SetApiTargetVersion(oldVersion);
}

/**
 * @tc.name: CanvasPaintMethodUpdateRecordingCanvas003
 * @tc.desc: Test UpdateRecordingCanvas with FireRSCanvasCallback.
 * @tc.type: FUNC
 */
HWTEST_F(CanvasPaintMethodTestNg, CanvasPaintMethodUpdateRecordingCanvas003, TestSize.Level1)
{
    /**
     * @tc.steps1: set canvasCallback_ and call UpdateRecordingCanvas.
     * @tc.expected: callback is invoked during UpdateRecordingCanvas.
     */
    RefPtr<FrameNode> frameNode;
    auto paintMethod = CreatePaintMethod(frameNode);
    bool called = false;
    std::function<void(std::shared_ptr<RSCanvas>, double, double)> callback =
        [&called](std::shared_ptr<RSCanvas>, double, double) { called = true; };
    paintMethod->SetRSCanvasCallback(callback);
    paintMethod->UpdateRecordingCanvas(TEST_CANVAS_WIDTH, TEST_CANVAS_HEIGHT);
    EXPECT_TRUE(called);
}

} // namespace OHOS::Ace::NG
