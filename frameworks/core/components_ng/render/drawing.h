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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_DRAWING_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_DRAWING_H

#ifndef ACE_UNITTEST
#include "draw/canvas.h"
#include "draw/shadow.h"
#include "drawing/engine_adapter/skia_adapter/skia_canvas.h"
#include "drawing/engine_adapter/skia_adapter/skia_path.h"
#include "image/image.h"
#ifdef USE_ROSEN_DRAWING
#include "recording/recording_canvas.h"
#include "recording/recording_color_filter.h"
#include "recording/recording_color_space.h"
#include "recording/recording_image_filter.h"
#include "recording/recording_mask_filter.h"
#include "recording/recording_path_effect.h"
#include "recording/recording_path.h"
#include "recording/recording_region.h"
#include "recording/recording_shader_effect.h"
#endif
#include "rosen_text/properties/text_style.h"
#include "rosen_text/ui/font_collection.h"
#include "rosen_text/ui/typography.h"
#include "rosen_text/ui/typography_create.h"
#include "utils/camera3d.h"
#include "utils/point3.h"
#include "utils/scalar.h"

namespace OHOS::Ace {
namespace RSDrawing = Rosen::Drawing;
using RSCanvas = Rosen::Drawing::Canvas;
using RSImage = Rosen::Drawing::Image;
using RSBrush = Rosen::Drawing::Brush;
using RSFilter = Rosen::Drawing::Filter;
using RSColorFilter = Rosen::Drawing::ColorFilter;
using RSColorMatrix = Rosen::Drawing::ColorMatrix;
using RSPen = Rosen::Drawing::Pen;
using RSColor = Rosen::Drawing::Color;
using RSRect = Rosen::Drawing::RectF;
using RSRoundRect = Rosen::Drawing::RoundRect;
using RSRRect = Rosen::Drawing::Rect;
using RSPoint = Rosen::Drawing::PointF;
using RSRPoint = Rosen::Drawing::Point;
using RSPoint3 = Rosen::Drawing::Point3;
using RSShadowFlags = Rosen::Drawing::ShadowFlags;
using RSBlendMode = Rosen::Drawing::BlendMode;
using RSSamplingOptions = Rosen::Drawing::SamplingOptions;
using RSRoundRect = Rosen::Drawing::RoundRect;
using RSPath = Rosen::Drawing::Path;
using RSPathOp = Rosen::Drawing::PathOp;
using RSBitmap = Rosen::Drawing::Bitmap;
using RSBitmapFormat = Rosen::Drawing::BitmapFormat;
using RSColorType = Rosen::Drawing::ColorType;
using RSAlphaType = Rosen::Drawing::AlphaType;
using RSScalar = Rosen::Drawing::scalar;
using RSClipOp = Rosen::Drawing::ClipOp;
using RSSkCanvas = Rosen::Drawing::SkiaCanvas;
using RSMatrix = Rosen::Drawing::Matrix;
using RSCamera3D = Rosen::Drawing::Camera3D;
using RSMaskFilter = Rosen::Drawing::MaskFilter;
using RSBlurType = Rosen::Drawing::BlurType;
using RSPathFillType = Rosen::Drawing::PathFillType;
using RSSkPath = Rosen::Drawing::SkiaPath;
using RSColorQuad = Rosen::Drawing::ColorQuad;
using RSShaderEffect = Rosen::Drawing::ShaderEffect;
using RSTileMode = Rosen::Drawing::TileMode;
#ifdef USE_ROSEN_DRAWING
using RSAutoCanvasRestore = Rosen::Drawing::AutoCanvasRestore;
using RSCubicResampler = Rosen::Drawing::CubicResampler;
using RSColorSpace = Rosen::Drawing::ColorSpace;
using RSData = Rosen::Drawing::Data;
using RSDrawCmdList = Rosen::Drawing::DrawCmdList;
using RSFilterMode = Rosen::Drawing::FilterMode;
using RSImageFilter = Rosen::Drawing::ImageFilter;
using RSMipmapMode = Rosen::Drawing::MipmapMode;
using RSPathDashStyle = Rosen::Drawing::PathDashStyle;
using RSPathDirection = Rosen::Drawing::PathDirection;
using RSPathEffect = Rosen::Drawing::PathEffect;
using RSPicture = Rosen::Drawing::Picture;
using RSRecordingCanvas = Rosen::Drawing::RecordingCanvas;
using RSRecordingColorFilter = Rosen::Drawing::RecordingColorFilter;
using RSRecordingColorSpace = Rosen::Drawing::RecordingColorSpace;
using RSRecordingImageFilter = Rosen::Drawing::RecordingImageFilter;
using RSRecordingMaskFilter = Rosen::Drawing::RecordingMaskFilter;
using RSRecordingPath = Rosen::Drawing::RecordingPath;
using RSRecordingPathEffect = Rosen::Drawing::RecordingPathEffect;
using RSRecordingRegion = Rosen::Drawing::RecordingRegion;
using RSRecordingShaderEffect = Rosen::Drawing::RecordingShaderEffect;
using RSSaveLayerOps = Rosen::Drawing::SaveLayerOps;
using RSSize = Rosen::Drawing::Size;
using RSSrcRectConstraint = Rosen::Drawing::SrcRectConstraint;
#endif
using RSPathEffect = rosen::PathEffect;
using RSPathDirection = rosen::PathDirection;
using RSPathDashStyle = rosen::PathDashStyle;
using RSParagraph = rosen::Typography;
using RSTypographyProperties = rosen::TypographyProperties;
using RSParagraphBuilder = rosen::TypographyCreate;
using RSFontCollection = rosen::FontCollection;
using RSParagraphStyle = rosen::TypographyStyle;
using RSColorQuad = rosen::ColorQuad;
using RSShaderEffect = rosen::ShaderEffect;
using RSTileMode = rosen::TileMode;
using RSTextDirection = rosen::TextDirection;
using RSTextAlign = rosen::TextAlign;
using RSWordBreakType = rosen::WordBreakType;
using RSTextStyle = rosen::TextStyle;
using RSTextDecoration = rosen::TextDecoration;
using RSFontWeight = rosen::FontWeight;
using RSFontStyle = rosen::FontStyle;
using RSTextBaseline = rosen::TextBaseline;
} // namespace OHOS::Ace
#else
#include "core/components_ng/render/drawing_mock.h"
#endif
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_RENDER_DRAWING_H
