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

#include "core/components_ng/pattern/video/video_layout_algorithm.h"

#include "base/geometry/ng/size_t.h"
#include "base/utils/utils.h"
#include "core/components/video/video_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/video/video_layout_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float SLIDER_RATIO = 2.0;
constexpr uint32_t SLIDER_POS = 2;
SizeF CalculateFitContain(const SizeF& videoSize, const SizeF& layoutSize)
{
    double layoutRatio = NearZero(layoutSize.Height()) ? 0.0 : layoutSize.Width() / layoutSize.Height();
    double sourceRatio = NearZero(videoSize.Height()) ? layoutRatio : videoSize.Width() / videoSize.Height();

    if (NearZero(layoutRatio) || NearZero(sourceRatio)) {
        return layoutSize;
    }
    if (sourceRatio < layoutRatio) {
        return { static_cast<float>(sourceRatio) * layoutSize.Height(), layoutSize.Height() };
    }
    return { layoutSize.Width(), static_cast<float>(layoutSize.Width() / sourceRatio) };
}
SizeF CalculateFitFill(const SizeF& layoutSize)
{
    return layoutSize;
}
SizeF CalculateFitCover(const SizeF& videoSize, const SizeF& layoutSize)
{
    double layoutRatio = NearZero(layoutSize.Height()) ? 0.0 : layoutSize.Width() / layoutSize.Height();
    double sourceRatio = NearZero(videoSize.Height()) ? layoutRatio : videoSize.Width() / videoSize.Height();

    if (NearZero(layoutRatio) || NearZero(sourceRatio)) {
        return layoutSize;
    }
    if (sourceRatio < layoutRatio) {
        return { layoutSize.Width(), static_cast<float>(layoutSize.Width() / sourceRatio) };
    }
    return { static_cast<float>(layoutSize.Height() * sourceRatio), layoutSize.Height() };
}
SizeF CalculateFitNone(const SizeF& videoSize)
{
    return videoSize;
}
SizeF CalculateFitScaleDown(const SizeF& videoSize, const SizeF& layoutSize)
{
    if (layoutSize.Width() > videoSize.Width()) {
        return CalculateFitNone(videoSize);
    }
    return CalculateFitContain(videoSize, layoutSize);
}
float CalControlBarHeight()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, 0.0f);
    auto themeManager = pipelineContext->GetThemeManager();
    CHECK_NULL_RETURN(themeManager, 0.0f);
    auto videoTheme = themeManager->GetTheme<VideoTheme>();
    CHECK_NULL_RETURN(videoTheme, 0.0f);
    auto controlsHeight =
        pipelineContext->NormalizeToPx(Dimension(videoTheme->GetBtnSize().Height(), DimensionUnit::VP));
    controlsHeight += pipelineContext->NormalizeToPx(videoTheme->GetBtnEdge().Top());
    controlsHeight += pipelineContext->NormalizeToPx(videoTheme->GetBtnEdge().Bottom());
    return static_cast<float>(controlsHeight);
}
} // namespace
VideoLayoutAlgorithm::VideoLayoutAlgorithm() = default;

void VideoLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    BoxLayoutAlgorithm::PerformLayout(layoutWrapper);

    auto frameLayoutOffset = layoutWrapper->GetGeometryNode()->GetFrameOffset();
    auto parentGlobalOffset = layoutWrapper->GetGeometryNode()->GetParentGlobalOffset();
    auto contentOffset = layoutWrapper->GetGeometryNode()->GetContentOffset();
    auto offset = frameLayoutOffset + parentGlobalOffset;
    auto frameSize = layoutWrapper->GetGeometryNode()->GetFrameSize();
    auto layoutProperty = DynamicCast<VideoLayoutProperty>(layoutWrapper->GetLayoutProperty());
    if (layoutProperty->GetControlsValue(false)) {
        contentOffset.SetY(offset.GetY());
        layoutWrapper->GetGeometryNode()->SetContentOffset(contentOffset);
    }
    for (auto&& child : layoutWrapper->GetAllChildrenWithBuild()) {
        child->Layout(offset);
        if (child->GetHostTag() == V2::IMAGE_ETS_TAG) {
            child->GetGeometryNode()->SetFrameOffset(offset);
            child->GetGeometryNode()->SetContentOffset(contentOffset);
        } else if (child->GetHostTag() == V2::ROW_ETS_TAG && layoutProperty->GetControlsValue(false)) {
            auto controlBarHeight = CalControlBarHeight();
            auto controlTabOffset = offset;
            controlTabOffset.SetY(controlTabOffset.GetY() + frameSize.Height() - controlBarHeight);
            child->GetGeometryNode()->SetFrameOffset(controlTabOffset);
        }
    }
}

void VideoLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto layoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    auto contentSize = layoutWrapper->GetGeometryNode()->GetContentSize();
    auto layoutProperty = DynamicCast<VideoLayoutProperty>(layoutWrapper->GetLayoutProperty());
    for (auto&& child : layoutWrapper->GetAllChildrenWithBuild()) {
        if (child->GetHostTag() == V2::IMAGE_ETS_TAG) {
            auto layoutConstraintForImage = layoutConstraint;
            layoutConstraintForImage.UpdateSelfIdealSizeWithCheck(OptionalSizeF(contentSize));
            layoutConstraintForImage.UpdateMaxSizeWithCheck(contentSize);
            layoutConstraintForImage.UpdateMinSizeWithCheck(contentSize);
            child->Measure(layoutConstraintForImage);
        } else if (child->GetHostTag() == V2::ROW_ETS_TAG && layoutProperty->GetControlsValue(false)) {
            auto controlBarHeight = CalControlBarHeight();
            SizeF controlBarSize(contentSize.Width(), controlBarHeight);
            auto layoutConstraintForControlBar = layoutConstraint;
            layoutConstraintForControlBar.UpdateSelfIdealSizeWithCheck(OptionalSizeF(controlBarSize));
            layoutConstraintForControlBar.UpdateMaxSizeWithCheck(controlBarSize);
            layoutConstraintForControlBar.UpdateMinSizeWithCheck(controlBarSize);
            auto sliderNode = DynamicCast<FrameNode>(child->GetHostNode()->GetChildAtIndex(SLIDER_POS));
            auto sliderWidth = CalcLength(controlBarSize.Width() / SLIDER_RATIO);
            auto sliderHeight = CalcLength(controlBarHeight);
            CalcSize idealSize = { sliderWidth, sliderHeight };
            MeasureProperty sliderLayoutConstraint;
            sliderLayoutConstraint.maxSize = idealSize;
            sliderNode->UpdateLayoutConstraint(sliderLayoutConstraint);
            child->Measure(layoutConstraintForControlBar);
        } else {
            child->Measure(layoutConstraint);
        }
    }
    PerformMeasureSelf(layoutWrapper);
    if (contentSize.IsPositive()) {
        const auto& padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
        if (layoutProperty->GetControlsValue(false)) {
            auto controlBarHeight = CalControlBarHeight();
            contentSize.AddHeight(controlBarHeight);
        }
        AddPaddingToSize(padding, contentSize);
        layoutWrapper->GetGeometryNode()->SetFrameSize(contentSize);
    }
}

std::optional<SizeF> VideoLayoutAlgorithm::MeasureContent(
    const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper)
{
    auto layoutProperty = DynamicCast<VideoLayoutProperty>(layoutWrapper->GetLayoutProperty());
    auto layoutSize = contentConstraint.selfIdealSize.IsValid() ? contentConstraint.selfIdealSize.ConvertToSizeT()
                                                                : contentConstraint.maxSize;
    auto controlBarHeight = 0.0f;
    if (layoutProperty->GetControlsValue(false)) {
        controlBarHeight = CalControlBarHeight();
    }
    if (!layoutProperty->HasVideoSize()) {
        LOGW("VideoSize has not set");
        layoutSize.MinusHeight(controlBarHeight);
        return layoutSize;
    }
    auto videoSize = layoutProperty->GetVideoSizeValue(SizeF(0, 0));
    LOGD("VideoSize = %{public}s", videoSize.ToString().c_str());
    auto imageFit = layoutProperty->GetObjectFitValue(ImageFit::COVER);
    SizeF contentSize = { 0.0, 0.0 };
    switch (imageFit) {
        case ImageFit::CONTAIN:
            contentSize = CalculateFitContain(videoSize, layoutSize);
            break;
        case ImageFit::FILL:
            contentSize = CalculateFitFill(layoutSize);
            break;
        case ImageFit::COVER:
            contentSize = CalculateFitCover(videoSize, layoutSize);
            break;
        case ImageFit::NONE:
            contentSize = CalculateFitNone(videoSize);
            break;
        case ImageFit::SCALEDOWN:
            contentSize = CalculateFitScaleDown(videoSize, layoutSize);
            break;
        default:
            contentSize = CalculateFitContain(videoSize, layoutSize);
    }
    contentSize.SetHeight(layoutSize.Height() - controlBarHeight);
    return contentSize;
}

} // namespace OHOS::Ace::NG
