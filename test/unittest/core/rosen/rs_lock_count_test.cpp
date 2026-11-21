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
#include "core/components_ng/render/adapter/drawing_image.h"
#include "render_service_client/core/modifier_ng/appearance/rs_pixel_stretch_modifier.h"
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

/**
 * @tc.name: RsLockCountTest007
 * @tc.desc: Batch Get data source equivalence - GetAllTransformPropertyValues returns the same
 *   field values as N individual staging Gets (pre-optimization data source). Merged Get functions
 *   consume the batch result, so batch == individual proves value equivalence of the merge point.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RsLockCountTest007, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->paintRect_ = RectF(0.0f, 0.0f, 100.0f, 100.0f);
    rosenRenderContext->SetPivot(0.3f, 0.7f);
    rosenRenderContext->SetRotation(0.0f, 0.0f, 45.0f);
    rosenRenderContext->SetScale(2.0f, 3.0f);
    rosenRenderContext->SetTranslate(10.0f, 20.0f, 0.0f);
    ASSERT_NE(rosenRenderContext->rsNode_, nullptr);
    auto& staging = rosenRenderContext->rsNode_->GetStagingProperties();
    auto batch = staging.GetAllTransformPropertyValues();
    auto pivot = staging.GetPivot();
    auto rotation = staging.GetRotation();
    auto translate = staging.GetTranslate();
    auto scale = staging.GetScale();
    auto skew = staging.GetSkew();
    auto persp = staging.GetPersp();
    EXPECT_FLOAT_EQ(batch.pivot.x_, pivot.x_);
    EXPECT_FLOAT_EQ(batch.pivot.y_, pivot.y_);
    EXPECT_FLOAT_EQ(batch.rotation, rotation);
    EXPECT_FLOAT_EQ(batch.translate.x_, translate.x_);
    EXPECT_FLOAT_EQ(batch.translate.y_, translate.y_);
    EXPECT_FLOAT_EQ(batch.scale.x_, scale.x_);
    EXPECT_FLOAT_EQ(batch.scale.y_, scale.y_);
    EXPECT_FLOAT_EQ(batch.skew.x_, skew.x_);
    EXPECT_FLOAT_EQ(batch.skew.y_, skew.y_);
    EXPECT_FLOAT_EQ(batch.skew.z_, skew.z_);
    EXPECT_FLOAT_EQ(batch.persp[0], persp[0]);
    EXPECT_FLOAT_EQ(batch.persp[1], persp[1]);
    EXPECT_FLOAT_EQ(batch.persp[2], persp[2]);
    EXPECT_FLOAT_EQ(batch.persp[3], persp[3]);
}

/**
 * @tc.name: RsLockCountTest008
 * @tc.desc: Batch Get boundary equivalence (AC-4) - no transform set (rotation=0, translate=(0,0),
 *   scale=(1,1), skew=(0,0,0), perspective=(0,0,0,1), pivot=(0.5,0.5)). Batch result equals
 *   individual Gets, Get matrices are identity, GetPointWithTransform keeps the point unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RsLockCountTest008, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->paintRect_ = RectF(0.0f, 0.0f, 100.0f, 100.0f);
    ASSERT_NE(rosenRenderContext->rsNode_, nullptr);
    auto& staging = rosenRenderContext->rsNode_->GetStagingProperties();
    auto batch = staging.GetAllTransformPropertyValues();
    auto pivot = staging.GetPivot();
    auto translate = staging.GetTranslate();
    auto scale = staging.GetScale();
    EXPECT_FLOAT_EQ(batch.pivot.x_, pivot.x_);
    EXPECT_FLOAT_EQ(batch.rotation, 0.0f);
    EXPECT_FLOAT_EQ(batch.translate.x_, translate.x_);
    EXPECT_FLOAT_EQ(batch.translate.y_, translate.y_);
    EXPECT_FLOAT_EQ(batch.scale.x_, scale.x_);
    EXPECT_FLOAT_EQ(batch.scale.y_, scale.y_);
    EXPECT_EQ(batch.pivot.x_, 0.5f);
    EXPECT_EQ(batch.pivot.y_, 0.5f);
    EXPECT_EQ(batch.scale.x_, 1.0f);
    EXPECT_EQ(batch.scale.y_, 1.0f);
    auto revertMatrix = rosenRenderContext->GetRevertMatrix();
    auto matrix = rosenRenderContext->GetMatrix();
    auto rotateMatrix = rosenRenderContext->GetMatrixWithTransformRotate();
    EXPECT_EQ(revertMatrix(0, 0), 1.0f);
    EXPECT_EQ(revertMatrix(1, 1), 1.0f);
    EXPECT_EQ(matrix(0, 0), 1.0f);
    EXPECT_EQ(matrix(1, 1), 1.0f);
    EXPECT_EQ(rotateMatrix(0, 0), 1.0f);
    EXPECT_EQ(rotateMatrix(1, 1), 1.0f);
    PointF point(50.0f, 50.0f);
    rosenRenderContext->GetPointWithTransform(point);
    EXPECT_FLOAT_EQ(point.GetX(), 50.0f);
    EXPECT_FLOAT_EQ(point.GetY(), 50.0f);
}

/**
 * @tc.name: RsLockCountTest009
 * @tc.desc: GetPointWithTransform value equivalence - scale(2,3) divides the point coordinates;
 *   translate without rotation is not applied (only used inside the rotation branch).
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RsLockCountTest009, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->paintRect_ = RectF(0.0f, 0.0f, 100.0f, 100.0f);
    rosenRenderContext->SetScale(2.0f, 3.0f);
    PointF point(50.0f, 60.0f);
    rosenRenderContext->GetPointWithTransform(point);
    EXPECT_FLOAT_EQ(point.GetX(), 25.0f);
    EXPECT_FLOAT_EQ(point.GetY(), 20.0f);
    rosenRenderContext->SetTranslate(10.0f, 20.0f, 0.0f);
    PointF point2(50.0f, 60.0f);
    rosenRenderContext->GetPointWithTransform(point2);
    EXPECT_FLOAT_EQ(point2.GetX(), 25.0f);
    EXPECT_FLOAT_EQ(point2.GetY(), 20.0f);
}

/**
 * @tc.name: RsLockCountTest010
 * @tc.desc: GetPaintRectWithTranslate value equivalence with rotation - rotated node returns the
 *   invalid rect (0, 0, -1, -1) marker, same as the pre-optimization individual-Get behavior.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RsLockCountTest010, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    rosenRenderContext->paintRect_ = RectF(0.0f, 0.0f, 100.0f, 100.0f);
    rosenRenderContext->SetRotation(0.0f, 0.0f, 90.0f);
    auto [rect, error] = rosenRenderContext->GetPaintRectWithTranslate();
    EXPECT_EQ(rect.Width(), -1.0f);
    EXPECT_EQ(rect.Height(), -1.0f);
}

/**
 * @tc.name: RsLockCountTest011
 * @tc.desc: PaintBackground batch Set equivalence (AC-5) - SetBgImageConfig applies the same
 *   width/height/positionX/positionY values that the 5 individual Sets applied before the merge.
 *   Expected values are computed by the same ImagePainter helpers used by production code.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RsLockCountTest011, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    ImageSourceInfo src("test", Dimension(100), Dimension(200));
    rosenRenderContext->UpdateBackgroundImage(src);
    ASSERT_NE(rosenRenderContext->bgLoadingCtx_, nullptr);
    rosenRenderContext->bgImage_ = AceType::MakeRefPtr<DrawingImage>();
    rosenRenderContext->paintRect_ = RectF(0.0f, 0.0f, 120.0f, 240.0f);
    rosenRenderContext->PaintBackground();
    ASSERT_NE(rosenRenderContext->rsNode_, nullptr);
    auto& staging = rosenRenderContext->rsNode_->GetStagingProperties();
    auto srcSize = rosenRenderContext->bgLoadingCtx_->GetImageSize();
    auto renderSize = ImagePainter::CalculateBgImageSize(
        rosenRenderContext->paintRect_.GetSize(), srcSize, rosenRenderContext->GetBackgroundImageSize());
    auto positionOffset = ImagePainter::CalculateBgImagePosition(
        rosenRenderContext->paintRect_.GetSize(), renderSize, rosenRenderContext->GetBackgroundImagePosition());
    EXPECT_FLOAT_EQ(staging.GetBgImageWidth(), renderSize.Width());
    EXPECT_FLOAT_EQ(staging.GetBgImageHeight(), renderSize.Height());
    EXPECT_FLOAT_EQ(staging.GetBgImagePositionX(), positionOffset.GetX());
    EXPECT_FLOAT_EQ(staging.GetBgImagePositionY(), positionOffset.GetY());
}

/**
 * @tc.name: RsLockCountTest012
 * @tc.desc: OnPixelStretchEffectUpdate percent branch equivalence (AC-6) - SetPixelStretch batch
 *   applies percent values and zeroes the px slot, same as the pre-merge SetPixelStretchPercent +
 *   SetPixelStretch({0,0,0,0}) pair.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RsLockCountTest012, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    ASSERT_NE(rosenRenderContext->rsNode_, nullptr);
    PixStretchEffectOption option;
    option.left = Dimension(0.1f, DimensionUnit::PERCENT);
    option.top = Dimension(0.2f, DimensionUnit::PERCENT);
    option.right = Dimension(0.3f, DimensionUnit::PERCENT);
    option.bottom = Dimension(0.4f, DimensionUnit::PERCENT);
    rosenRenderContext->OnPixelStretchEffectUpdate(option);
    std::shared_ptr<Rosen::ModifierNG::RSPixelStretchModifier> pixelStretchModifier;
    for (auto& [id, modifier] : rosenRenderContext->rsNode_->modifiersNG_) {
        if (modifier->GetType() == Rosen::ModifierNG::RSModifierType::PIXEL_STRETCH) {
            pixelStretchModifier = std::static_pointer_cast<Rosen::ModifierNG::RSPixelStretchModifier>(modifier);
        }
    }
    ASSERT_NE(pixelStretchModifier, nullptr);
    auto percent = pixelStretchModifier->GetPixelStretchPercent();
    auto size = pixelStretchModifier->GetPixelStretchSize();
    EXPECT_FLOAT_EQ(percent[0], 0.1f);
    EXPECT_FLOAT_EQ(percent[1], 0.2f);
    EXPECT_FLOAT_EQ(percent[2], 0.3f);
    EXPECT_FLOAT_EQ(percent[3], 0.4f);
    EXPECT_FLOAT_EQ(size[0], 0.0f);
    EXPECT_FLOAT_EQ(size[1], 0.0f);
    EXPECT_FLOAT_EQ(size[2], 0.0f);
    EXPECT_FLOAT_EQ(size[3], 0.0f);
}

/**
 * @tc.name: RsLockCountTest013
 * @tc.desc: OnPixelStretchEffectUpdate px branch equivalence (AC-6) - SetPixelStretch batch applies
 *   px values and zeroes the percent slot, same as the pre-merge SetPixelStretch +
 *   SetPixelStretchPercent({0,0,0,0}) pair.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RsLockCountTest013, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    ASSERT_NE(rosenRenderContext->rsNode_, nullptr);
    PixStretchEffectOption option;
    option.left = Dimension(10.0f, DimensionUnit::VP);
    option.top = Dimension(20.0f, DimensionUnit::VP);
    option.right = Dimension(30.0f, DimensionUnit::VP);
    option.bottom = Dimension(40.0f, DimensionUnit::VP);
    rosenRenderContext->OnPixelStretchEffectUpdate(option);
    std::shared_ptr<Rosen::ModifierNG::RSPixelStretchModifier> pixelStretchModifier;
    for (auto& [id, modifier] : rosenRenderContext->rsNode_->modifiersNG_) {
        if (modifier->GetType() == Rosen::ModifierNG::RSModifierType::PIXEL_STRETCH) {
            pixelStretchModifier = std::static_pointer_cast<Rosen::ModifierNG::RSPixelStretchModifier>(modifier);
        }
    }
    ASSERT_NE(pixelStretchModifier, nullptr);
    auto percent = pixelStretchModifier->GetPixelStretchPercent();
    auto size = pixelStretchModifier->GetPixelStretchSize();
    EXPECT_FLOAT_EQ(size[0], static_cast<float>(option.left.ConvertToPx()));
    EXPECT_FLOAT_EQ(size[1], static_cast<float>(option.top.ConvertToPx()));
    EXPECT_FLOAT_EQ(size[2], static_cast<float>(option.right.ConvertToPx()));
    EXPECT_FLOAT_EQ(size[3], static_cast<float>(option.bottom.ConvertToPx()));
    EXPECT_FLOAT_EQ(percent[0], 0.0f);
    EXPECT_FLOAT_EQ(percent[1], 0.0f);
    EXPECT_FLOAT_EQ(percent[2], 0.0f);
    EXPECT_FLOAT_EQ(percent[3], 0.0f);
}

/**
 * @tc.name: RsLockCountTest014
 * @tc.desc: ModifyCustomBackground SwapModifier equivalence (AC-7) - transitionBackground on/off
 *   branches swap background/transition modifiers on the RS node, same as the pre-merge
 *   RemoveModifier + AddModifier pair.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RsLockCountTest014, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    ASSERT_NE(rosenRenderContext->rsNode_, nullptr);
    rosenRenderContext->UpdateIsTransitionBackground(true);
    rosenRenderContext->ModifyCustomBackground();
    bool transitionInNode = false;
    bool backgroundInNode = false;
    for (auto& [id, modifier] : rosenRenderContext->rsNode_->modifiersNG_) {
        if (modifier.get() == rosenRenderContext->transitionModifier_.get()) {
            transitionInNode = true;
        }
        if (modifier.get() == rosenRenderContext->backgroundModifier_.get()) {
            backgroundInNode = true;
        }
    }
    EXPECT_TRUE(transitionInNode);
    EXPECT_FALSE(backgroundInNode);
    rosenRenderContext->UpdateIsTransitionBackground(false);
    rosenRenderContext->ModifyCustomBackground();
    transitionInNode = false;
    backgroundInNode = false;
    for (auto& [id, modifier] : rosenRenderContext->rsNode_->modifiersNG_) {
        if (modifier.get() == rosenRenderContext->transitionModifier_.get()) {
            transitionInNode = true;
        }
        if (modifier.get() == rosenRenderContext->backgroundModifier_.get()) {
            backgroundInNode = true;
        }
    }
    EXPECT_FALSE(transitionInNode);
    EXPECT_TRUE(backgroundInNode);
}

/**
 * @tc.name: RsLockCountTest015
 * @tc.desc: ClearModifiers batch removal equivalence (AC-8) - RemoveModifiers clears all user
 *   modifiers, staging transform values fall back to defaults, same as removing one by one.
 * @tc.type: FUNC
 */
HWTEST_F(RosenRenderContextTest, RsLockCountTest015, TestSize.Level1)
{
    auto frameNode = FrameNode::GetOrCreateFrameNode("parent", -1, []() { return AceType::MakeRefPtr<Pattern>(); });
    auto rosenRenderContext = InitRosenRenderContext(frameNode);
    ASSERT_NE(rosenRenderContext->rsNode_, nullptr);
    rosenRenderContext->SetRotation(0.0f, 0.0f, 90.0f);
    rosenRenderContext->SetTranslate(10.0f, 20.0f, 0.0f);
    rosenRenderContext->SetScale(2.0f, 3.0f);
    rosenRenderContext->SetOpacity(0.5f);
    auto& staging = rosenRenderContext->rsNode_->GetStagingProperties();
    EXPECT_FLOAT_EQ(staging.GetRotation(), 90.0f);
    EXPECT_FLOAT_EQ(staging.GetTranslate().x_, 10.0f);
    EXPECT_FLOAT_EQ(staging.GetTranslate().y_, 20.0f);
    EXPECT_FLOAT_EQ(staging.GetScale().x_, 2.0f);
    EXPECT_FLOAT_EQ(staging.GetScale().y_, 3.0f);
    EXPECT_FLOAT_EQ(staging.GetAlpha(), 0.5f);
    rosenRenderContext->ClearModifiers();
    EXPECT_FLOAT_EQ(staging.GetRotation(), 0.0f);
    EXPECT_FLOAT_EQ(staging.GetTranslate().x_, 0.0f);
    EXPECT_FLOAT_EQ(staging.GetTranslate().y_, 0.0f);
    EXPECT_FLOAT_EQ(staging.GetScale().x_, 1.0f);
    EXPECT_FLOAT_EQ(staging.GetScale().y_, 1.0f);
    EXPECT_FLOAT_EQ(staging.GetAlpha(), 1.0f);
}
} // namespace
} // namespace OHOS::Ace::NG
