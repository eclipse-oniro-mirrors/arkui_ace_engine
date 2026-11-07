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
#include <cmath>

#include "base/geometry/dimension.h"
#include "core/components_ng/pattern/common_text/preview_text_decoration_utils.h"

namespace OHOS::Ace::NG {

namespace {

constexpr float MIN_WIDTH = 0.01f;
constexpr float HALF_DIVISOR = 2.0f;
constexpr int ODD_CHECK = 2;
constexpr float WAVE_PERIOD_FACTOR = 2.0f;
constexpr float DASH_UNIT_DIVISOR = 14.0f;
constexpr float DASH_LENGTH_MULT = 4.0f;
constexpr float DASH_GAP_MULT = 2.0f;
constexpr float EPSILON = 0.001f;

void BuildWavyPath(RSPath& path, float width, float quarterWave, float phaseOffset)
{
    if (quarterWave <= MIN_WIDTH || width <= EPSILON) {
        return;
    }
    float halfWaveWidth = quarterWave * WAVE_PERIOD_FACTOR;
    int waveCount = static_cast<int>(std::floor(phaseOffset / halfWaveWidth));
    float xStart = 0.0f;
    while (xStart + halfWaveWidth <= width + EPSILON) {
        float dy = (waveCount % ODD_CHECK != 0) ? quarterWave : -quarterWave;
        path.RQuadTo(halfWaveWidth / WAVE_PERIOD_FACTOR, dy, halfWaveWidth, 0.0f);
        xStart += halfWaveWidth;
        ++waveCount;
    }
    float remaining = width - xStart;
    if (remaining > EPSILON) {
        float peak = (remaining / halfWaveWidth) * quarterWave;
        float dy = (waveCount % ODD_CHECK != 0) ? peak : -peak;
        path.RQuadTo(remaining / WAVE_PERIOD_FACTOR, dy, remaining, 0.0f);
    }
}

void StrokeWavyPath(RSCanvas& canvas, RSPath& path, float underlineWidth, RSColor color)
{
    canvas.DetachBrush();
    RSPen pen;
    pen.SetAntiAlias(true);
    pen.SetColor(color);
    pen.SetWidth(underlineWidth);
    pen.SetCapStyle(RSPen::CapStyle::ROUND_CAP);
    pen.SetJoinStyle(RSPen::JoinStyle::ROUND_JOIN);
    canvas.AttachPen(pen);
    canvas.DrawPath(path);
    canvas.DetachPen();
}

void DrawWavyUnderline(RSCanvas& canvas, const PreviewDecorationParams& params)
{
    float underlineWidth = DEFAULT_FONTSIZE.ConvertToPx() / WAVY_WIDTH_DIVISOR;
    float width = params.right - params.left;
    if (underlineWidth <= MIN_WIDTH || width <= 0.0f) {
        return;
    }
    float baseline = params.bottom - underlineWidth / HALF_DIVISOR;
    RSPath path;
    path.MoveTo(params.left, baseline);
    BuildWavyPath(path, width, underlineWidth, params.startOffset);
    StrokeWavyPath(canvas, path, underlineWidth, params.color);
}

void DrawSolidUnderline(RSCanvas& canvas, const PreviewDecorationParams& params)
{
    float roundRectRadius = params.underlineWidth / HALF_DIVISOR;
    float padding = params.isLast ? 0.0f : roundRectRadius;
    RSRect rect(params.left, params.bottom - params.underlineWidth,
        params.right + padding, params.bottom);
    canvas.DrawRoundRect(RSRoundRect(rect, roundRectRadius, roundRectRadius));
}

void DrawDashLineUnderline(RSCanvas& canvas, const PreviewDecorationParams& params)
{
    float unit = DEFAULT_FONTSIZE.ConvertToPx() / DASH_UNIT_DIVISOR;
    float dashLength = unit * DASH_LENGTH_MULT;
    float gap = unit * DASH_GAP_MULT;
    float baseline = params.bottom - params.underlineWidth / HALF_DIVISOR;
    if (dashLength <= MIN_WIDTH) {
        return;
    }

    canvas.DetachBrush();
    RSPen pen;
    pen.SetAntiAlias(true);
    pen.SetColor(params.color);
    pen.SetWidth(params.underlineWidth);
    pen.SetCapStyle(RSPen::CapStyle::FLAT_CAP);
    canvas.AttachPen(pen);

    float period = dashLength + gap;
    float phase = std::fmod(params.startOffset, period);
    if (phase < 0.0f) {
        phase += period;
    }
    float x = params.left;
    if (phase > dashLength) {
        float skip = period - phase;
        x += skip;
        phase = 0.0f;
    } else if (phase > 0.0f) {
        float remain = dashLength - phase;
        float segEnd = std::min(x + remain, params.right);
        if (segEnd > x + EPSILON) {
            canvas.DrawLine(RSPoint(x, baseline), RSPoint(segEnd, baseline));
        }
        x = segEnd + gap;
    }
    while (x < params.right - EPSILON) {
        float segEnd = std::min(x + dashLength, params.right);
        if (segEnd > x + EPSILON) {
            canvas.DrawLine(RSPoint(x, baseline), RSPoint(segEnd, baseline));
        }
        x = segEnd + gap;
    }
    canvas.DetachPen();
}

} // namespace

void DrawPreviewTextUnderline(RSCanvas& canvas, const PreviewDecorationParams& params)
{
    switch (params.underlineStyle) {
        case PreviewTextStyle::WAVY:
            DrawWavyUnderline(canvas, params);
            break;
        case PreviewTextStyle::DASH_LINE:
            DrawDashLineUnderline(canvas, params);
            break;
        case PreviewTextStyle::UNDERLINE:
        default:
            DrawSolidUnderline(canvas, params);
            break;
    }
}

} // namespace OHOS::Ace::NG
