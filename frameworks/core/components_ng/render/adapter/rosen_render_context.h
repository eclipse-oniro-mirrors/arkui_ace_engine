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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_ADAPTER_ROSEN_RENDER_CONTEXT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_ADAPTER_ROSEN_RENDER_CONTEXT_H

#include <cstdint>
#include <optional>

#include "render_service_client/core/ui/rs_node.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"
#include "third_party/skia/include/core/SkRefCnt.h"

#include "base/geometry/dimension_offset.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/utils/noncopyable.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/image_provider/image_loading_context.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/rosen_modifier_property.h"
#include "core/components_ng/render/render_context.h"

namespace OHOS::Ace::NG {
class BorderImageModifier;
class MouseSelectModifier;
class RosenRenderContext : public RenderContext {
    DECLARE_ACE_TYPE(RosenRenderContext, NG::RenderContext)
public:
    RosenRenderContext() = default;
    ~RosenRenderContext() override;

    void InitContext(bool isRoot, const std::optional<std::string>& surfaceName, bool useExternalNode) override;

    void SyncGeometryProperties(GeometryNode* geometryNode) override;

    void SyncGeometryProperties(const RectF& paintRect) override;

    void RebuildFrame(FrameNode* self, const std::list<RefPtr<FrameNode>>& children) override;

    void AddFrameChildren(FrameNode* self, const std::list<RefPtr<FrameNode>>& children) override;

    void RemoveFrameChildren(FrameNode* self, const std::list<RefPtr<FrameNode>>& children) override;

    void MoveFrame(FrameNode* self, const RefPtr<FrameNode>& child, int32_t index) override;

    void OnModifyDone() override;

    void ResetBlendBgColor() override;

    void BlendBgColor(const Color& color) override;

    void ResetBlendBorderColor() override;

    void BlendBorderColor(const Color& color) override;

    RefPtr<Canvas> GetCanvas() override;
    void Restore() override;

    const std::shared_ptr<Rosen::RSNode>& GetRSNode();

    void SetRSNode(const std::shared_ptr<Rosen::RSNode>& rsNode);

    void StartRecording() override;

    void StopRecordingIfNeeded() override;

    bool IsRecording()
    {
        return !!recordingCanvas_;
    }

    void StartPictureRecording(float x, float y, float width, float height);
    sk_sp<SkPicture> FinishRecordingAsPicture();

    void SetDrawContentAtLast(bool useDrawContentLastOrder) override
    {
        CHECK_NULL_VOID(rsNode_);
        rsNode_->SetPaintOrder(useDrawContentLastOrder);
    }

    void SetClipToFrame(bool useClip) override
    {
        CHECK_NULL_VOID(rsNode_);
        rsNode_->SetClipToFrame(useClip);
    }

    void SetClipToBounds(bool useClip) override
    {
        CHECK_NULL_VOID(rsNode_);
        rsNode_->SetClipToBounds(useClip);
    }

    void SetVisible(bool visible) override
    {
        CHECK_NULL_VOID(rsNode_);
        rsNode_->SetVisible(visible);
    }

    void FlushContentDrawFunction(CanvasDrawFunction&& contentDraw) override;

    void FlushForegroundDrawFunction(CanvasDrawFunction&& foregroundDraw) override;

    void FlushOverlayDrawFunction(CanvasDrawFunction&& overlayDraw) override;

    void AnimateHoverEffectScale(bool isHovered) override;
    void AnimateHoverEffectBoard(bool isHovered) override;
    void UpdateBackBlurRadius(const Dimension& radius) override;
    void UpdateFrontBlurRadius(const Dimension& radius) override;
    void UpdateBackShadow(const Shadow& shadow) override;
    void UpdateBorderWidthF(const BorderWidthPropertyF& value) override;

    void OnTransformMatrixUpdate(const Matrix4& matrix) override;

    void UpdateTransition(const TransitionOptions& options) override;
    bool HasAppearingTransition() const
    {
        return propTransitionAppearing_ != nullptr;
    }
    bool HasDisappearingTransition() const
    {
        return propTransitionDisappearing_ != nullptr;
    }
    void OnNodeAppear() override;
    void OnNodeDisappear(FrameNode* host) override;
    void ClipWithRect(const RectF& rectF) override;

    bool TriggerPageTransition(PageTransitionType type, const std::function<void()>& onFinish) const override;

    void SetSharedTranslate(float xTranslate, float yTranslate) override;
    void ResetSharedTranslate() override;

    static std::list<std::shared_ptr<Rosen::RSNode>> GetChildrenRSNodes(
        const std::list<RefPtr<FrameNode>>& frameChildren);

    static std::shared_ptr<Rosen::RSTransitionEffect> GetRSTransitionWithoutType(const TransitionOptions& options);

    void FlushModifier(const RefPtr<Modifier>& modifier) override;

    void AddChild(const RefPtr<RenderContext>& renderContext, int index) override;
    void SetBounds(float positionX, float positionY, float width, float height) override;
    void OnTransformTranslateUpdate(const Vector3F& value) override;

    RectF GetPaintRectWithTransform() override;

    RectF GetPaintRectWithoutTransform() override;

    void ClearDrawCommands() override;

    void NotifyTransition(
        const AnimationOption& option, const TransitionOptions& transOptions, bool isTransitionIn) override;

    void OpacityAnimation(const AnimationOption& option, double begin, double end) override;

    void PaintAccessibilityFocus() override;

    void OnMouseSelectUpdate(const Color& fillColor, const Color& strokeColor) override;
    void UpdateMouseSelectWithRect(const RectF& rect, const Color& fillColor, const Color& strokeColor) override;

private:
    void OnBackgroundColorUpdate(const Color& value) override;
    void OnBackgroundImageUpdate(const ImageSourceInfo& imageSourceInfo) override;
    void OnBackgroundImageRepeatUpdate(const ImageRepeat& imageRepeat) override;
    void OnBackgroundImageSizeUpdate(const BackgroundImageSize& bgImgSize) override;
    void OnBackgroundImagePositionUpdate(const BackgroundImagePosition& bgImgPosition) override;
    void OnBackgroundBlurStyleUpdate(const BlurStyle& bgBlurStyle) override;

    void OnBorderImageUpdate(const RefPtr<BorderImage>& borderImage) override;
    void OnBorderImageSourceUpdate(const ImageSourceInfo& borderImageSourceInfo) override;
    void OnHasBorderImageSliceUpdate(bool tag) override;
    void OnHasBorderImageWidthUpdate(bool tag) override;
    void OnHasBorderImageOutsetUpdate(bool tag) override;
    void OnHasBorderImageRepeatUpdate(bool tag) override;
    void OnBorderImageGradientUpdate(const Gradient& gradient) override;

    void OnBorderRadiusUpdate(const BorderRadiusProperty& value) override;
    void OnBorderColorUpdate(const BorderColorProperty& value) override;
    void OnBorderStyleUpdate(const BorderStyleProperty& value) override;
    void OnOpacityUpdate(double opacity) override;

    void OnTransformScaleUpdate(const VectorF& value) override;
    void OnTransformCenterUpdate(const DimensionOffset& value) override;
    void OnTransformRotateUpdate(const Vector4F& value) override;

    void OnPositionUpdate(const OffsetT<Dimension>& value) override;
    void OnOffsetUpdate(const OffsetT<Dimension>& value) override;
    void OnAnchorUpdate(const OffsetT<Dimension>& value) override;
    void OnZIndexUpdate(int32_t value) override;

    void OnClipShapeUpdate(const RefPtr<BasicShape>& basicShape) override;
    void OnClipEdgeUpdate(bool isClip) override;
    void OnClipMaskUpdate(const RefPtr<BasicShape>& basicShape) override;

    void OnLinearGradientUpdate(const NG::Gradient& value) override;
    void OnSweepGradientUpdate(const NG::Gradient& value) override;
    void OnRadialGradientUpdate(const NG::Gradient& value) override;

    void OnFrontBrightnessUpdate(const Dimension& brightness) override;
    void OnFrontGrayScaleUpdate(const Dimension& grayScale) override;
    void OnFrontContrastUpdate(const Dimension& contrast) override;
    void OnFrontSaturateUpdate(const Dimension& saturate) override;
    void OnFrontSepiaUpdate(const Dimension& sepia) override;
    void OnFrontInvertUpdate(const Dimension& invert) override;
    void OnFrontHueRotateUpdate(float hueRotate) override;
    void OnFrontColorBlendUpdate(const Color& colorBlend) override;

    void OnOverlayTextUpdate(const OverlayOptions& overlay) override;
    void OnMotionPathUpdate(const MotionPathOption& motionPath) override;

    void OnAccessibilityFocusUpdate(bool isAccessibilityFocus) override;

    void ReCreateRsNodeTree(const std::list<RefPtr<FrameNode>>& children);

    void NotifyTransitionInner(const SizeF& frameSize, bool isTransitionIn);
    void SetTransitionPivot(const SizeF& frameSize, bool transitionIn);
    void SetPivot(float xPivot, float yPivot);

    void PaintBackground();
    void PaintClip(const SizeF& frameSize);
    void PaintGradient(const SizeF& frameSize);
    void PaintGraphics();
    void OnPaintGraphics();
    void PaintOverlayText();
    void PaintBorderImage();
    void PaintBorderImageGradient();
    void PaintMouseSelectRect(const RectF& rect, const Color& fillColor, const Color& strokeColor);


    RectF AdjustPaintRect();

    DataReadyNotifyTask CreateBgImageDataReadyCallback();
    LoadSuccessNotifyTask CreateBgImageLoadSuccessCallback();
    DataReadyNotifyTask CreateBorderImageDataReadyCallback();
    LoadSuccessNotifyTask CreateBorderImageLoadSuccessCallback();

    std::shared_ptr<Rosen::RSNode> rsNode_;
    SkPictureRecorder* recorder_ = nullptr;
    RefPtr<ImageLoadingContext> bgLoadingCtx_;
    RefPtr<ImageLoadingContext> bdImageLoadingCtx_;
    RefPtr<Canvas> recordingCanvas_;
    RefPtr<Canvas> rosenCanvas_;
    bool isHoveredScale_ = false;
    bool isHoveredBoard_ = false;
    bool isPositionChanged_ = false;
    bool firstTransitionIn_ = false;
    Color blendColor_ = Color::TRANSPARENT;
    Color hoveredColor_ = Color::TRANSPARENT;

    std::shared_ptr<BorderImageModifier> borderImageModifier_ = nullptr;
    std::shared_ptr<MouseSelectModifier> mouseSelectModifier_ = nullptr;
    std::optional<TransformMatrixModifier> transformMatrixModifier_;
    std::shared_ptr<Rosen::RSProperty<Rosen::Vector2f>> pivotProperty_;
    std::unique_ptr<SharedTransitionModifier> sharedTransitionModifier_;

    ACE_DISALLOW_COPY_AND_MOVE(RosenRenderContext);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_ADAPTER_ROSEN_RENDER_CONTEXT_H
