/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/scroll/scroll_paint_method.h"

#include "core/components_ng/pattern/arc_scroll/inner/arc_scroll_bar.h"
#include "core/components_ng/pattern/arc_scroll/inner/arc_scroll_bar_overlay_modifier.h"

namespace OHOS::Ace::NG {

CanvasDrawFunction ScrollPaintMethod::GetForegroundDrawFunction(PaintWrapper* paintWrapper)
{
    auto paintFunc = [weak = WeakClaim(this), paintWrapper](RSCanvas& canvas) {
        auto scroll = weak.Upgrade();
        CHECK_NULL_VOID(scroll);
        scroll->PaintScrollEffect(canvas, paintWrapper);
    };

    return paintFunc;
}

void ScrollPaintMethod::UpdateContentModifier(PaintWrapper* paintWrapper)
{
    auto renderContext = paintWrapper->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    UpdateFadingGradient(renderContext);
    TryContentClip(paintWrapper);
}

void ScrollPaintMethod::PaintScrollEffect(RSCanvas& canvas, PaintWrapper* paintWrapper) const
{
    auto scrollEdgeEffect = edgeEffect_.Upgrade();
    CHECK_NULL_VOID(scrollEdgeEffect);
    auto frameSize = paintWrapper->GetGeometryNode()->GetFrameSize();
    scrollEdgeEffect->Paint(canvas, frameSize, { 0.0f, 0.0f });
}

void ScrollPaintMethod::UpdateOverlayModifier(PaintWrapper* paintWrapper)
{
    if (scrollBar2d_.Upgrade()) {
        UpdateOverlayModifier2d(paintWrapper);
        return;
    }
    CHECK_NULL_VOID(paintWrapper);
    auto scrollBarOverlayModifier = scrollBarOverlayModifier_.Upgrade();
    CHECK_NULL_VOID(scrollBarOverlayModifier);
    auto scrollBar = scrollBar_.Upgrade();
    CHECK_NULL_VOID(scrollBar);
    if (scrollBar->GetPositionModeUpdate()) {
        scrollBarOverlayModifier->SetPositionMode(scrollBar->GetPositionMode());
    }
    auto shapeMode = scrollBar->GetShapeMode();
    if (shapeMode == ShapeMode::ROUND) {
        auto arcScrollBarOverlayModifier = AceType::DynamicCast<ArcScrollBarOverlayModifier>(scrollBarOverlayModifier);
        CHECK_NULL_VOID(arcScrollBarOverlayModifier);
        auto arcScrollBar = AceType::DynamicCast<ArcScrollBar>(scrollBar);
        CHECK_NULL_VOID(arcScrollBar);
        scrollBarOverlayModifier->SetBarColor(arcScrollBar->GetArcForegroundColor());
        arcScrollBarOverlayModifier->SetBackgroundBarColor(arcScrollBar->GetArcBackgroundColor());
        arcScrollBarOverlayModifier->StartArcBarAnimation(arcScrollBar->GetHoverAnimationType(),
            arcScrollBar->GetOpacityAnimationType(), arcScrollBar->GetNeedAdaptAnimation(),
            arcScrollBar->GetArcActiveRect(), arcScrollBar->GetArcBarRect());
    } else {
        scrollBarOverlayModifier->SetBarColor(scrollBar->GetForegroundColor());
        scrollBarOverlayModifier->StartBarAnimation(scrollBar->GetHoverAnimationType(),
            scrollBar->GetOpacityAnimationType(), scrollBar->GetNeedAdaptAnimation(), scrollBar->GetActiveRect());
    }
    scrollBar->SetHoverAnimationType(HoverAnimationType::NONE);
    scrollBar->SetOpacityAnimationType(OpacityAnimationType::NONE);
}

void ScrollPaintMethod::UpdateOverlayModifier2d(PaintWrapper* paintWrapper)
{
    CHECK_NULL_VOID(paintWrapper);
    const auto scrollBar = scrollBar2d_.Upgrade();
    auto&& overlay = scrollBar->GetPainter();
    CHECK_NULL_VOID(overlay && scrollBar);
    overlay->UpdateFrom(*scrollBar);
    scrollBar->ResetAnimationSignals();
}
} // namespace OHOS::Ace::NG
