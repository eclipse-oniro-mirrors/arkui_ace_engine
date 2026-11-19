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
#ifndef FOUNDATION_ACE_TEST_MOCK_OHOS_MOCK_RENDER_SERVICE_BASE_INCLUDE_COMMON_RS_MOCK_ENUMS_H
#define FOUNDATION_ACE_TEST_MOCK_OHOS_MOCK_RENDER_SERVICE_BASE_INCLUDE_COMMON_RS_MOCK_ENUMS_H
#include <cstdint>
namespace OHOS::Rosen {
enum class RSNGEffectType : int16_t {
    INVALID = -1,
    NONE = 0,
    // filter type
    BLUR,
    MATERIAL_BLUR,
    PIXEL_STRETCH,
    WATER_RIPPLE,
    FLY_OUT,
    DISTORT,
    RADIUS_GRADIENT_BLUR,
    COLOR_GRADIENT,
    DISPLACEMENT_DISTORT,
    SOUND_WAVE,
    EDGE_LIGHT,
    BEZIER_WARP,
    DISPERSION,
    DIRECTION_LIGHT,
    AIBAR,
    GREY,
    MATERIAL,
    MAGNIFIER,
    MESA,
    MASK_COLOR,
    KAWASE,
    LIGHT_BLUR,
    LINEAR_GRADIENT_BLUR,
    ALWAYS_SNAPSHOT,
    MASK_TRANSITION,
    VARIABLE_RADIUS_BLUR,
    CONTENT_LIGHT,
    GRID_WARP,
    DISTORTION_COLLAPSE,
    HEAT_DISTORTION,
    BLUR_BUBBLES_RISE,
    // mask type
    RIPPLE_MASK,
    DOUBLE_RIPPLE_MASK,
    RADIAL_GRADIENT_MASK,
    PIXEL_MAP_MASK,
    WAVE_GRADIENT_MASK,
    WAVE_DISTURBANCE_MASK,
    IMAGE_MASK,
    USE_EFFECT_MASK,
    FRAME_GRADIENT_MASK,
    DUPOLI_NOISE_MASK,
    NOISY_FRAME_GRADIENT_MASK,
    // shape type
    SDF_UNION_OP_SHAPE,
    SDF_SMOOTH_UNION_OP_SHAPE,
    SDF_RRECT_SHAPE,
    SDF_TRIANGLE_SHAPE,
    SDF_ELLIPSE_SHAPE,
    SDF_PATH_SHAPE,
    SDF_PIXELMAP_SHAPE,
    SDF_TRANSFORM_SHAPE,
    SDF_EMPTY_SHAPE,
    SDF_DISTORT_OP_SHAPE,
    SDF_SUB_OP_SHAPE,
    SDF_SMOOTH_SUB_OP_SHAPE,
    // shader type
    CONTOUR_DIAGONAL_FLOW_LIGHT,
    DOT_MATRIX_SHADER,
    WAVY_RIPPLE_LIGHT,
    AURORA_NOISE,
    PARTICLE_CIRCULAR_HALO,
    LIGHT_CAVE,
    BORDER_LIGHT,
    COLOR_GRADIENT_EFFECT,
    HARMONIUM_EFFECT,
    GASIFY_SCALE_TWIST,
    GASIFY_BLUR,
    GASIFY,
    PARTICLE_ABLATION,
    AIBAR_GLOW,
    AIBAR_RECT_HALO,
    ROUNDED_RECT_FLOWLIGHT,
    GRADIENT_FLOW_COLORS,
    FROSTED_GLASS,
    CIRCLE_FLOWLIGHT,
    FROSTED_GLASS_EFFECT,
    FROSTED_GLASS_BLUR,
    DISTORT_CHROMA,
    SDF_EDGE_LIGHT,
    SDF_EDGE_LIGHT_EFFECT,
    BORDER_SDF_SHADER,
    BORDER_SDF_LG_COLOR,
    SPATIAL_POINT_LIGHT,
    SPATIAL_GLASS_EFFECT,
    MOTION_BLUR,
    FRACTAL_GLASS_MASK,
    BINOCULAR_MASK,
    SWEEP_REFRACTION_MASK,
    SPIN_BLUR,
    HALO_BLOOM,
    WARPED_RING_MASK,
    MAP_COLOR_BY_BRIGHTNESS,
    GLASS_EFFECT,
    ATLAS_FRAME_MASK,
    DISTORT_LIGHT,
    COSMOS_RING_SHADOW,
};
enum class RSUINodeType : uint32_t {
    UNKNOW = 0x0000u,
    RS_NODE = 0x0001u,
    DISPLAY_NODE = 0x0011u,
    SURFACE_NODE = 0x0021u,
    PROXY_NODE = 0x0041u,
    CANVAS_NODE = 0x0081u,
    EFFECT_NODE = 0x0101u,
    DEPTH_NODE = 0x0401u,
    WINDOW_KEYFRAME_NODE = 0x0801u,
    ROOT_NODE = 0x1081u,
    CANVAS_DRAWING_NODE = 0x2081u,
    UNION_NODE = 0x4081u,
};
enum class RSUIFirstSwitch {
    NONE = 0,                   // follow RS rules
    MODAL_WINDOW_CLOSE = 1,     // open app with modal window animation, close uifirst
    FORCE_DISABLE = 2,          // force close uifirst
    FORCE_ENABLE = 3,           // force open uifirst
    FORCE_ENABLE_LIMIT = 4,     // force open uifirst, but for limited
    FORCE_DISABLE_NONFOCUS = 5, // force close uifirst when only in nonfocus window
    FORCE_DISABLE_CARD = 6,     // force close uifirst on card
};
enum DrawNodeType : uint32_t { PureContainerType = 0, MergeableType, DrawPropertyType, GeometryPropertyType };
enum class ContentTransitionType : uint8_t {
    IDENTITY = 0,
    OPACITY,
};
enum class Gravity : int32_t {
    CENTER = 0,
    TOP,
    BOTTOM,
    LEFT,
    RIGHT,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    RESIZE,
    RESIZE_ASPECT,
    RESIZE_ASPECT_TOP_LEFT,
    RESIZE_ASPECT_BOTTOM_RIGHT,
    RESIZE_ASPECT_FILL,
    RESIZE_ASPECT_FILL_TOP_LEFT,
    RESIZE_ASPECT_FILL_BOTTOM_RIGHT,

    DEFAULT = TOP_LEFT
};
enum class ForegroundColorStrategyType {
    INVALID = 0,
    INVERT_BACKGROUNDCOLOR,
};
enum class RSColorBlendMode : int16_t {
    NONE = 0, // Note: The NONE blend mode is different from SRC_OVER. When using it with
              // RSColorBlendApplyType::SAVE_LAYER, it does not create an offscreen buffer. However, when using it
              // with RSColorBlendApplyType::FAST, it does not modify the blend mode of subsequent content.

    CLEAR,    // r = 0
    SRC,      // r = s
    DST,      // r = d
    SRC_OVER, // r = s + (1-sa)*d
    DST_OVER, // r = d + (1-da)*s
    SRC_IN,   // r = s * da
    DST_IN,   // r = d * sa
    SRC_OUT,  // r = s * (1-da)
    DST_OUT,  // r = d * (1-sa)
    SRC_ATOP, // r = s*da + d*(1-sa)
    DST_ATOP, // r = d*sa + s*(1-da)
    XOR,      // r = s*(1-da) + d*(1-sa)
    PLUS,     // r = min(s + d, 1)
    MODULATE, // r = s*d
    SCREEN,   // r = s + d - s*d

    OVERLAY,     // multiply or screen, depending on destination
    DARKEN,      // rc = s + d - max(s*da, d*sa), ra = SRC_OVER
    LIGHTEN,     // rc = s + d - min(s*da, d*sa), ra = SRC_OVER
    COLOR_DODGE, // brighten destination to reflect source
    COLOR_BURN,  // darken destination to reflect source
    HARD_LIGHT,  // multiply or screen, depending on source
    SOFT_LIGHT,  // lighten or darken, depending on source
    DIFFERENCE,  // rc = s + d - 2*(min(s*da, d*sa)), ra = SRC_OVER
    EXCLUSION,   // rc = s + d - two(s*d), ra = SRC_OVER
    MULTIPLY,    // r = s*(1-da) + d*(1-sa) + s*d

    HUE,        // hue of source with saturation and luminosity of destination
    SATURATION, // saturation of source with hue and luminosity of destination
    COLOUR,     // hue and saturation of source with luminosity of destination
    LUMINOSITY, // luminosity of source with hue and saturation of destination

    MAX = LUMINOSITY,
};
enum class RSColorBlendApplyType : int16_t {
    FAST, // Apply blending by drawing the content with the blend mode, without using an offscreen buffer

    SAVE_LAYER, // Apply blending by drawing the content onto an offscreen buffer and blend it when drawing it back to
                // the screen
    SAVE_LAYER_ALPHA, // Similar to SAVE_LAYER, but when drawing back to the screen, it will be multiplied by the
                      // alpha of the screen twice. We add this type to avoid incompatibility changes. Don't use
                      // it in the future unless you have a clear understanding of what will happen!
    // init with a copy of the background for advanced use.
    //  Do NOT use this unless you have a clear understanding of what will happen!
    SAVE_LAYER_INIT_WITH_PREVIOUS_CONTENT,
    MAX = SAVE_LAYER_INIT_WITH_PREVIOUS_CONTENT
};
enum class RSCornerApplyType : int16_t { FAST = 0, OFFSCREEN, MAX = OFFSCREEN };
enum class ColorPickStrategyType : int16_t {
    NONE,
    DOMINANT,
    AVERAGE,
    CONTRAST,
    CLIENT_CALLBACK, // pick average color and callback to notify client side
    MAX = CLIENT_CALLBACK
};
enum class GradientDirection {
    LEFT = 0,
    TOP,
    RIGHT,
    BOTTOM,
    LEFT_TOP,
    LEFT_BOTTOM,
    RIGHT_TOP,
    RIGHT_BOTTOM,
    NONE,
    START_TO_END,
    END_TO_START,
};
enum BLUR_COLOR_MODE : int {
    PRE_DEFINED = 0, // use the pre-defined mask color
    AVERAGE = 1,     // use the average color of the blurred area as mask color
    FASTAVERAGE = 2,
    DEFAULT = PRE_DEFINED
};
enum SHADOW_COLOR_STRATEGY : int {
    COLOR_STRATEGY_NONE = 0,    // use pre-defined shadow color
    COLOR_STRATEGY_AVERAGE = 1, // use average color of the shadow area
    COLOR_STRATEGY_MAIN = 2,    // use main color of the shadow area
};
enum class BorderStyle : uint32_t { SOLID = 0, DASHED, DOTTED, NONE };
enum class FilterQuality {
    DEFAULT = 0,
    ADAPTIVE = 1,
};
} // namespace OHOS::Rosen

#endif // FOUNDATION_ACE_TEST_MOCK_OHOS_MOCK_RENDER_SERVICE_BASE_INCLUDE_COMMON_RS_MOCK_ENUMS_H
