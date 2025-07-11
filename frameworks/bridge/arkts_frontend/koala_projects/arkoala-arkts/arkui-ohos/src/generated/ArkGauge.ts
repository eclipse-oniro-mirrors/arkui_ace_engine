/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


// WARNING! THIS FILE IS AUTO-GENERATED, DO NOT MAKE CHANGES, THEY WILL BE LOST ON NEXT GENERATION!

import { int32, float32 } from "@koalaui/common"
import { KStringPtr, KBoolean, RuntimeType, runtimeType } from "@koalaui/interop"
import { NodeAttach, remember } from "@koalaui/runtime"
import { ComponentBase } from "../ComponentBase"
import { TypeChecker } from "#components"
import { ArkCommonMethodComponent } from "./ArkCommon"
import { ArkGaugePeer } from "./peers/ArkGaugePeer"
import { CommonMethod, DrawModifier, Rectangle, Callback_Array_TouchTestInfo_TouchResult, TouchTestInfo, TouchResult, PixelRoundPolicy, BackgroundEffectOptions, ForegroundEffectOptions, VisualEffect, Filter, BorderImageOption, OutlineStyle, Callback_ClickEvent_Void, ClickEvent, Callback_Boolean_HoverEvent_Void, HoverEvent, AccessibilityCallback, Callback_MouseEvent_Void, MouseEvent, Callback_TouchEvent_Void, TouchEvent, Callback_KeyEvent_Void, KeyEvent, Callback_KeyEvent_Boolean, AnimateParam, TransitionOptions, TransitionEffect, MotionBlurOptions, InvertOptions, TranslateOptions, ScaleOptions, RotateOptions, Callback_Area_Area_Void, Literal_Union_Number_Literal_Number_offset_span_lg_md_sm_xs, Literal_Number_offset_span, AlignRuleOption, LocalizedAlignRuleOptions, ClickEffect, Callback_DragEvent_String_Union_CustomBuilder_DragItemInfo, DragEvent, CustomBuilder, DragItemInfo, Callback_DragEvent_String_Void, UniformDataType, Callback_PreDragStatus_Void, PreDragStatus, Type_CommonMethod_linearGradient_value, Tuple_ResourceColor_Number, Type_CommonMethod_sweepGradient_value, Tuple_Length_Length, Type_CommonMethod_radialGradient_value, MotionPathOptions, ShadowOptions, ShadowStyle, ProgressMask, StateStyles, PixelStretchEffectOptions, GestureModifier, BackgroundBrightnessOptions, Callback_GestureInfo_BaseGestureEvent_GestureJudgeResult, GestureRecognizerJudgeBeginCallback, ShouldBuiltInRecognizerParallelWithCallback, Callback_TouchEvent_HitTestMode, SizeChangeCallback, SafeAreaType, SafeAreaEdge, Literal_Alignment_align, BlurStyle, BackgroundBlurStyleOptions, ForegroundBlurStyleOptions, TransitionFinishCallback, BlurOptions, LinearGradientBlurOptions, EffectType, sharedTransitionOptions, ChainStyle, DragPreviewOptions, DragInteractionOptions, ComponentContent, OverlayOptions, BlendMode, BlendApplyType, Blender, GeometryTransitionOptions, PopupOptions, CustomPopupOptions, MenuElement, MenuOptions, ContextMenuOptions, ModalTransition, ContentCoverOptions, SheetOptions, VisibleAreaChangeCallback, ContentModifier } from "./../component/common"
import { Length, SizeOptions, ConstraintSizeOptions, ChainWeightOptions, Padding, LocalizedPadding, Margin, LocalizedMargin, ResourceColor, Position, BorderOptions, EdgeStyles, EdgeWidths, LocalizedEdgeWidths, EdgeColors, LocalizedEdgeColors, BorderRadiuses, LocalizedBorderRadiuses, OutlineOptions, EdgeOutlineStyles, Dimension, EdgeOutlineWidths, OutlineRadiuses, Area, Edges, LocalizedEdges, LocalizedPosition, ResourceStr, AccessibilityOptions } from "./../component/units"
import { HitTestMode, ImageSize, Alignment, BorderStyle, ColoringStrategy, HoverEffect, Color, Visibility, ItemAlign, Direction, GradientDirection, ObscuredReasons, RenderFit, ImageRepeat, Axis, ResponseType, FunctionKey, ModifierKey } from "./../component/enums"
import { LengthMetrics } from "../Graphics"
import { ResizableOptions } from "./../component/image"
import { Resource } from "global/resource";
import { Callback_Void } from "./../component/abilityComponent"
import { FocusBoxStyle, FocusPriority } from "./../component/focus"
import { CircleShape } from "./ArkCircleShapeMaterialized"
import { EllipseShape } from "./ArkEllipseShapeMaterialized"
import { PathShape } from "./ArkPathShapeMaterialized"
import { RectShape } from "./ArkRectShapeMaterialized"
import { AttributeModifier } from "./../component/common" 
import { GestureInfo, BaseGestureEvent, GestureJudgeResult, GestureType, GestureMask } from "./../component/gesture"
import { PixelMap } from "./ArkPixelMapMaterialized"
import { LinearGradient } from "./../component/dataPanel"
import { Tuple_Union_ResourceColor_LinearGradient_Number, GaugeShadowOptions, GaugeIndicatorOptions, GaugeConfiguration, GaugeOptions, GaugeAttribute } from "./../component/gauge"
/** @memo:stable */
export class ArkGaugeComponent extends ArkCommonMethodComponent implements GaugeAttribute {
    getPeer(): ArkGaugePeer {
        return (this.peer as ArkGaugePeer)
    }
    /** @memo */
    public setGaugeOptions(options: GaugeOptions): this {
        if (this.checkPriority("setGaugeOptions")) {
            const options_casted = options as (GaugeOptions)
            this.getPeer()?.setGaugeOptionsAttribute(options_casted)
            return this
        }
        return this
    }
    /** @memo */
    public value(value: number): this {
        if (this.checkPriority("value")) {
            const value_casted = value as (number)
            this.getPeer()?.valueAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public startAngle(value: number): this {
        if (this.checkPriority("startAngle")) {
            const value_casted = value as (number)
            this.getPeer()?.startAngleAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public endAngle(value: number): this {
        if (this.checkPriority("endAngle")) {
            const value_casted = value as (number)
            this.getPeer()?.endAngleAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public colors(value: ResourceColor | LinearGradient | Array<[ ResourceColor | LinearGradient, number ]>): this {
        if (this.checkPriority("colors")) {
            const value_casted = value as (ResourceColor | LinearGradient | Array<[ ResourceColor | LinearGradient, number ]>)
            this.getPeer()?.colorsAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public strokeWidth(value: Length): this {
        if (this.checkPriority("strokeWidth")) {
            const value_casted = value as (Length)
            this.getPeer()?.strokeWidthAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public description(value: CustomBuilder): this {
        if (this.checkPriority("description")) {
            const value_casted = value as (CustomBuilder)
            this.getPeer()?.descriptionAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public trackShadow(value: GaugeShadowOptions): this {
        if (this.checkPriority("trackShadow")) {
            const value_casted = value as (GaugeShadowOptions)
            this.getPeer()?.trackShadowAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public indicator(value: GaugeIndicatorOptions): this {
        if (this.checkPriority("indicator")) {
            const value_casted = value as (GaugeIndicatorOptions)
            this.getPeer()?.indicatorAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public privacySensitive(value?: boolean): this {
        if (this.checkPriority("privacySensitive")) {
            const value_casted = value as (boolean | undefined)
            this.getPeer()?.privacySensitiveAttribute(value_casted)
            return this
        }
        return this
    }
    /** @memo */
    public contentModifier(value: ContentModifier<object>): this {
        if (this.checkPriority("contentModifier")) {
            const value_casted = value as (ContentModifier<object>)
            this.getPeer()?.contentModifierAttribute(value_casted)
            return this
        }
        return this
    }
    public applyAttributesFinish(): void {
        // we calls this function outside of class, so need to make it public
        super.applyAttributesFinish()
    }
}
