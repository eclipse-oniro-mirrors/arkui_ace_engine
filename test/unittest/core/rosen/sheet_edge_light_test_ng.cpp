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

#include <functional>

#include "gtest/gtest.h"

#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components/common/properties/ui_material.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/sheet/sheet_edge_light.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/element_register.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
class SheetEdgeLightTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void SheetEdgeLightTestNg::SetUpTestCase()
{
    MockPipelineContext::SetUp();
}

void SheetEdgeLightTestNg::TearDownTestCase()
{
    MockPipelineContext::TearDown();
}

/**
 * @tc.name: CheckIfNeedShowEdgeLight001
 * @tc.desc: Test all branches of CheckIfNeedShowEdgeLight.
 * @tc.type: FUNC
 */
HWTEST_F(SheetEdgeLightTestNg, CheckIfNeedShowEdgeLight001, TestSize.Level1)
{
    // not SHEET_BOTTOM, always false
    EXPECT_FALSE(
        SheetEdgeLightBase::CheckIfNeedShowEdgeLight(EdgeLightMode::EDGELIGHT_ENABLED, SheetType::SHEET_POPUP));
    EXPECT_FALSE(
        SheetEdgeLightBase::CheckIfNeedShowEdgeLight(EdgeLightMode::EDGELIGHT_DISABLED, SheetType::SHEET_CENTER));
    // SHEET_BOTTOM with disabled mode
    EXPECT_FALSE(
        SheetEdgeLightBase::CheckIfNeedShowEdgeLight(EdgeLightMode::EDGELIGHT_DISABLED, SheetType::SHEET_BOTTOM));
    // SHEET_BOTTOM with enabled mode
    EXPECT_TRUE(
        SheetEdgeLightBase::CheckIfNeedShowEdgeLight(EdgeLightMode::EDGELIGHT_ENABLED, SheetType::SHEET_BOTTOM));
    // SHEET_BOTTOM with auto mode
    EXPECT_FALSE(SheetEdgeLightBase::CheckIfNeedShowEdgeLight(EdgeLightMode::EDGELIGHT_AUTO, SheetType::SHEET_BOTTOM));
    // SHEET_BOTTOM with unknown mode, fall through to last return
    EXPECT_FALSE(SheetEdgeLightBase::CheckIfNeedShowEdgeLight(
        static_cast<EdgeLightMode>(100), SheetType::SHEET_BOTTOM));
}

/**
 * @tc.name: GetSheetEdgeLightAnimateOption001
 * @tc.desc: Test GetSheetEdgeLightAnimateOption and GetSheetEdgeLightAnimatePreShowOption.
 * @tc.type: FUNC
 */
HWTEST_F(SheetEdgeLightTestNg, GetSheetEdgeLightAnimateOption001, TestSize.Level1)
{
    auto option = SheetEdgeLightBase::GetSheetEdgeLightAnimateOption();
    EXPECT_NE(option.GetCurve(), nullptr);
    EXPECT_GT(option.GetDuration(), 0);

    auto preShowOption = SheetEdgeLightBase::GetSheetEdgeLightAnimatePreShowOption();
    EXPECT_NE(preShowOption.GetCurve(), nullptr);
    EXPECT_GT(preShowOption.GetDuration(), 0);
}

/**
 * @tc.name: SetSheetEdgeLightTransitionNullNode001
 * @tc.desc: Test transition functions with null frame node.
 * @tc.type: FUNC
 */
HWTEST_F(SheetEdgeLightTestNg, SetSheetEdgeLightTransitionNullNode001, TestSize.Level1)
{
    RefPtr<FrameNode> nullNode = nullptr;
    SheetEdgeLightBase::SetSheetEdgeLightTransitionPreShow(nullNode);
    SheetEdgeLightBase::SetSheetEdgeLightTransitionStart(nullNode);
    SheetEdgeLightBase::SetSheetEdgeLightTransitionEnd(nullNode);
    SUCCEED();
}

/**
 * @tc.name: SetSheetEdgeLightTransitionValidNode001
 * @tc.desc: Test transition functions with valid frame node.
 * @tc.type: FUNC
 */
HWTEST_F(SheetEdgeLightTestNg, SetSheetEdgeLightTransitionValidNode001, TestSize.Level1)
{
    auto sheetNode = FrameNode::CreateFrameNode(
        V2::SHEET_PAGE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(sheetNode, nullptr);
    auto geometryNode = sheetNode->GetGeometryNode();
    ASSERT_NE(geometryNode, nullptr);
    geometryNode->SetFrameSize(SizeF(300, 400));

    SheetEdgeLightBase::SetSheetEdgeLightTransitionPreShow(sheetNode);
    auto renderContext = sheetNode->GetRenderContext();
    ASSERT_NE(renderContext, nullptr);

    SheetEdgeLightBase::SetSheetEdgeLightTransitionStart(sheetNode);
    SheetEdgeLightBase::SetSheetEdgeLightTransitionEnd(sheetNode);
    SUCCEED();
}

/**
 * @tc.name: GetSheetEdgeLightAnimateEvent001
 * @tc.desc: Test animate event with released node and pattern-mismatch node.
 * @tc.type: FUNC
 */
HWTEST_F(SheetEdgeLightTestNg, GetSheetEdgeLightAnimateEvent001, TestSize.Level1)
{
    std::function<void()> animateEvent;
    {
        auto sheetNode = FrameNode::CreateFrameNode(
            V2::SHEET_PAGE_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        ASSERT_NE(sheetNode, nullptr);
        animateEvent = SheetEdgeLightBase::GetSheetEdgeLightAnimateEvent(sheetNode);
        ASSERT_NE(animateEvent, nullptr);
        // pattern is not SheetPresentationPattern, early return inside event
        animateEvent();
    }
    // node released, weak upgrade fails, CHECK_NULL_VOID branch
    animateEvent();
}

/**
 * @tc.name: GetSheetEdgeLightAnimateFinishEvent001
 * @tc.desc: Test animate finish event creation and released-node branch.
 * @tc.type: FUNC
 */
HWTEST_F(SheetEdgeLightTestNg, GetSheetEdgeLightAnimateFinishEvent001, TestSize.Level1)
{
    std::function<void()> finishEvent;
    {
        auto sheetNode = FrameNode::CreateFrameNode(V2::SHEET_PAGE_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        ASSERT_NE(sheetNode, nullptr);
        finishEvent = SheetEdgeLightBase::GetSheetEdgeLightAnimateFinishEvent(sheetNode);
        ASSERT_NE(finishEvent, nullptr);
        // NOTE: do not invoke finishEvent with a live node here: with a real pipeline context,
        // the outer callback runs AnimationUtils::Animate, which needs an initialized Container
        // (task executor) unavailable in unittest process. Only the released-node branch is safe.
    }
    // node released, weak upgrade fails, CHECK_NULL_VOID branch
    finishEvent();
}
} // namespace OHOS::Ace::NG
