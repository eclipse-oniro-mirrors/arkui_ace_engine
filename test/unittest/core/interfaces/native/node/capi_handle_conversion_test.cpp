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
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/canvas/canvas_paint_method.h"
#include "core/components_ng/pattern/canvas/canvas_pattern.h"
#include "core/components_ng/pattern/canvas/canvas_render_context_deferred.h"
#include "core/components_ng/pattern/swiper_indicator/indicator_common/indicator_pattern.h"
#include "core/interfaces/native/node/node_indicator_modifier.h"
#undef protected
#undef private

using namespace testing::ext;

namespace OHOS::Ace::NG {
const ArkUICanvasModifier* GetCanvasDynamicModifier();

class CapiHandleConversionTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        MockContainer::SetUp();
        MockContainer::Current()->SetUseNewPipeline();
        MockPipelineContext::SetUp();
    }

    static void TearDownTestSuite()
    {
        MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }

    void TearDown() override
    {
        ViewStackProcessor::GetInstance()->ClearStack();
    }

    void CheckCanvasCallbacks(bool useFrameNode)
    {
        auto pattern = AceType::MakeRefPtr<CanvasPattern>();
        auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
        auto frameNode = FrameNode::CreateFrameNode("Canvas", nodeId, pattern);
        ASSERT_NE(frameNode, nullptr);
        auto contentModifier = AceType::MakeRefPtr<CanvasModifier>();
        pattern->paintMethod_ = AceType::MakeRefPtr<CanvasPaintMethod>(contentModifier, frameNode);
        pattern->paintMethod_->canvasRenderContext_ = AceType::MakeRefPtr<CanvasRenderContextDeferred>();
        AceType* owner = useFrameNode ? static_cast<AceType*>(AceType::RawPtr(frameNode))
                                     : static_cast<AceType*>(AceType::RawPtr(pattern));
        auto handle = reinterpret_cast<ArkUINodeHandle>(owner);
        const auto* modifier = GetCanvasDynamicModifier();
        ASSERT_NE(modifier, nullptr);
        ASSERT_NE(modifier->setUpdateContextCallback, nullptr);
        ASSERT_NE(modifier->setRSCanvasCallback, nullptr);
        ASSERT_NE(modifier->setRSCanvasForDrawingContext, nullptr);
        ASSERT_NE(modifier->setInvalidate, nullptr);
        EXPECT_FALSE(pattern->paintMethod_->NeedRender());
        modifier->setInvalidate(handle);
        EXPECT_TRUE(pattern->paintMethod_->NeedRender());
        bool unitUpdated = false;
        std::function<void(CanvasUnit)> updateUnit = [&unitUpdated](CanvasUnit unit) {
            unitUpdated = unit == CanvasUnit::PX;
        };
        modifier->setUpdateContextCallback(handle, &updateUnit);
        pattern->UpdateUnit(CanvasUnit::PX);
        EXPECT_TRUE(unitUpdated);

        int32_t canvasCallbackCount = 0;
        std::function<void(std::shared_ptr<RSCanvas>, double, double)> callback =
            [&canvasCallbackCount](std::shared_ptr<RSCanvas>, double, double) { ++canvasCallbackCount; };
        modifier->setRSCanvasCallback(handle, &callback);
        modifier->setRSCanvasForDrawingContext(handle);
        EXPECT_EQ(canvasCallbackCount, 1);
    }
};

/**
 * @tc.name: IndicatorOwnerHandle001
 * @tc.desc: The factory returns the AceType subobject and preserves the controller's reference counter.
 * @tc.type: FUNC
 */
HWTEST_F(CapiHandleConversionTest, IndicatorOwnerHandle001, TestSize.Level1)
{
    const auto* modifier = NodeModifier::GetIndicatorComponentModifier();
    ASSERT_NE(modifier, nullptr);
    ASSERT_NE(modifier->create, nullptr);
    void* handle = modifier->create();
    ASSERT_NE(handle, nullptr);
    auto node = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    ASSERT_NE(node, nullptr);
    auto pattern = node->GetPattern<IndicatorPattern>();
    ASSERT_NE(pattern, nullptr);
    auto controller = pattern->GetIndicatorController();
    ASSERT_NE(controller, nullptr);
    auto* owner = static_cast<AceType*>(AceType::RawPtr(controller));
    ASSERT_EQ(handle, static_cast<void*>(owner));
    const auto count = controller->RefCount();
    {
        auto claimed = AceType::Claim(static_cast<AceType*>(handle));
        EXPECT_EQ(AceType::DynamicCast<IndicatorController>(AceType::RawPtr(claimed)), AceType::RawPtr(controller));
        EXPECT_EQ(controller->RefCount(), count + 1);
    }
    EXPECT_EQ(controller->RefCount(), count);
}

/**
 * @tc.name: CanvasPatternOwnerHandle001
 * @tc.desc: Canvas callbacks accept the AceType subobject of a CanvasPattern.
 * @tc.type: FUNC
 */
HWTEST_F(CapiHandleConversionTest, CanvasPatternOwnerHandle001, TestSize.Level1)
{
    CheckCanvasCallbacks(false);
}

/**
 * @tc.name: CanvasFrameNodeOwnerHandle001
 * @tc.desc: Canvas callbacks also resolve a FrameNode through its AceType subobject.
 * @tc.type: FUNC
 */
HWTEST_F(CapiHandleConversionTest, CanvasFrameNodeOwnerHandle001, TestSize.Level1)
{
    CheckCanvasCallbacks(true);
}

/**
 * @tc.name: CanvasUnrelatedOwnerHandle001
 * @tc.desc: Null and unrelated AceType objects must not be reinterpreted as FrameNode objects.
 * @tc.type: FUNC
 */
HWTEST_F(CapiHandleConversionTest, CanvasUnrelatedOwnerHandle001, TestSize.Level1)
{
    const auto* modifier = GetCanvasDynamicModifier();
    ASSERT_NE(modifier, nullptr);
    auto other = AceType::MakeRefPtr<IndicatorController>();
    auto* owner = static_cast<AceType*>(AceType::RawPtr(other));
    auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
    auto otherNode = FrameNode::CreateFrameNode("Other", nodeId, AceType::MakeRefPtr<Pattern>());
    auto* nodeOwner = static_cast<AceType*>(AceType::RawPtr(otherNode));
    for (auto handle : { ArkUINodeHandle(nullptr), reinterpret_cast<ArkUINodeHandle>(owner),
             reinterpret_cast<ArkUINodeHandle>(nodeOwner) }) {
        bool called = false;
        std::function<void(CanvasUnit)> updateUnit = [&called](CanvasUnit) { called = true; };
        std::function<void(std::shared_ptr<RSCanvas>, double, double)> callback =
            [&called](std::shared_ptr<RSCanvas>, double, double) { called = true; };
        modifier->setInvalidate(handle);
        modifier->setUpdateContextCallback(handle, &updateUnit);
        modifier->setRSCanvasCallback(handle, &callback);
        modifier->setRSCanvasForDrawingContext(handle);
        EXPECT_FALSE(called);
        EXPECT_TRUE(updateUnit);
    }
}
} // namespace OHOS::Ace::NG
