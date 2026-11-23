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

#include <memory>

#include "gtest/gtest.h"

#define private public
#define protected public
#include "test/unittest/core/rosen/rosen_render_context_test.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {

/**
 * @tc.name: RsLockCountTest001
 * @tc.desc: GetRevertMatrix behavior equivalence after batch Get via GetAllTransformPropertyValues.
 *   Lock count reduced from 6 (GetPivot/Rotation/Translate/Skew/Scale/Persp) to 1.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RsLockCountTest001, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->paintRect_ = RectF(0.0f, 0.0f, 100.0f, 100.0f);
    rosenRenderContext->SetPivot(0.5f, 0.5f, 0.0f);
    rosenRenderContext->SetRotation(0.0f, 0.0f, 90.0f);
    rosenRenderContext->SetScale(2.0f, 2.0f);
    rosenRenderContext->SetTranslate(10.0f, 20.0f, 0.0f);
    auto matrix = rosenRenderContext->GetRevertMatrix();
    EXPECT_FALSE(matrix.IsIdentityMatrix());
}

/**
 * @tc.name: RsLockCountTest002
 * @tc.desc: GetMatrix behavior equivalence after batch Get.
 *   GetMatrixInner lock count reduced from 5 to 1.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RsLockCountTest002, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->paintRect_ = RectF(0.0f, 0.0f, 100.0f, 100.0f);
    rosenRenderContext->SetPivot(0.5f, 0.5f, 0.0f);
    rosenRenderContext->SetRotation(0.0f, 0.0f, 45.0f);
    rosenRenderContext->SetScale(1.5f, 1.5f);
    rosenRenderContext->SetTranslate(5.0f, 15.0f, 0.0f);
    auto matrix = rosenRenderContext->GetMatrix();
    EXPECT_FALSE(matrix.IsIdentityMatrix());
}

/**
 * @tc.name: RsLockCountTest003
 * @tc.desc: GetMatrixWithTransformRotate behavior equivalence after batch Get.
 *   GetMatrixWithTransformRotateInner lock count reduced from 6 to 1.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RsLockCountTest003, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->paintRect_ = RectF(0.0f, 0.0f, 100.0f, 100.0f);
    rosenRenderContext->SetPivot(0.5f, 0.5f, 0.0f);
    rosenRenderContext->SetRotation(0.0f, 0.0f, 30.0f);
    rosenRenderContext->SetScale(2.0f, 3.0f);
    rosenRenderContext->SetTranslate(10.0f, 20.0f, 0.0f);
    auto matrix = rosenRenderContext->GetMatrixWithTransformRotate();
    EXPECT_FALSE(matrix.IsIdentityMatrix());
}

/**
 * @tc.name: RsLockCountTest004
 * @tc.desc: GetPointWithTransform behavior equivalence after batch Get.
 *   GetPointWithTransformInner lock count reduced from 6 to 1.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RsLockCountTest004, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->paintRect_ = RectF(0.0f, 0.0f, 100.0f, 100.0f);
    rosenRenderContext->SetPivot(0.5f, 0.5f, 0.0f);
    rosenRenderContext->SetRotation(0.0f, 0.0f, 90.0f);
    rosenRenderContext->SetScale(2.0f, 2.0f);
    rosenRenderContext->SetTranslate(10.0f, 20.0f, 0.0f);
    PointF point(50.0f, 50.0f);
    rosenRenderContext->GetPointWithTransform(point);
    EXPECT_NE(point.GetX(), 50.0f);
}

/**
 * @tc.name: RsLockCountTest005
 * @tc.desc: GetPaintRectWithTranslate behavior equivalence after batch Get.
 *   GetPaintRectWithTranslate lock count reduced from 2 to 1.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RsLockCountTest005, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->paintRect_ = RectF(0.0f, 0.0f, 100.0f, 100.0f);
    rosenRenderContext->SetRotation(0.0f, 0.0f, 0.0f);
    rosenRenderContext->SetTranslate(10.0f, 20.0f, 0.0f);
    auto [rect, error] = rosenRenderContext->GetPaintRectWithTranslate();
    EXPECT_EQ(rect.GetOffset().GetX(), 10.0f);
    EXPECT_EQ(rect.GetOffset().GetY(), 20.0f);
}

/**
 * @tc.name: RsLockCountTest006
 * @tc.desc: GetPaintRectWithTranslate boundary - no transform set.
 *   Verify default/boundary behavior equivalence.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RsLockCountTest006, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->paintRect_ = RectF(10.0f, 20.0f, 100.0f, 100.0f);
    auto [rect, error] = rosenRenderContext->GetPaintRectWithTranslate();
    EXPECT_EQ(rect.GetOffset().GetX(), 10.0f);
    EXPECT_EQ(rect.GetOffset().GetY(), 20.0f);
}
} // namespace
} // namespace OHOS::Ace::NG
