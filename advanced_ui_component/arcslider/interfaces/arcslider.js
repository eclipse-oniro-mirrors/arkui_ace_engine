/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length,
        r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => {
    });
}

const animator = requireNapi('animator');
const drawing = requireNapi('graphics.drawing');
const vibrator = requireNapi('vibrator');
const ColorMetrics = requireNapi('arkui.node').ColorMetrics;
const hilog = requireNapi('hilog');

const ANGLE_TO_RADIAN = Math.PI / 180;
const RADIAN_TO_ANGLE = 180 / Math.PI;
const PI_RADIAN = 180;
const TWO_PI_ANGLE = 360;
const TWO_PI_RADIAN = 2 * Math.PI;
const APPROXIMATE_NUMBER = Math.pow(10, -7);
const ANGLE_OVER_MIN = 10;
const LENGTH_OVER_MIN = 0.15;
const TOTAL_LENGTH = 233;
const PROGRESS_DEFAULT = 0;
const MIN_DEFAULT = 0;
const MAX_DEFAULT = 100;
const X_DEFAULT = 116.5;
const Y_DEFAULT = 116.5;
const RADIUS_DEFAULT = 116.5;
const PADDING_DEFAULT = 5.5;
const START_ANGLE_DEFAULT = 15;
const END_ANGLE_DEFAULT = 45;
const ACTIVE_START_ANGLE_DEFAULT = -60;
const ACTIVE_END_ANGLE_DEFAULT = 60;
const REVERSE_DEFAULT = true;
const TRACK_THICKNESS_DEFAULT = 5;
const ACTIVE_TRACK_THICKNESS_DEFAULT = 24;
const TRACK_BLUR_DEFAULT = 20;
const TRACK_COLOR_DEFAULT = '#33FFFFFF';
const SELECTED_COLOR_DEFAULT = '#FF5EA1FF';
const BLUR_COLOR_DEFAULT = $r('sys.color.ohos_id_color_subtab_bg');
const VIBRATOR_TYPE_TWO = 'watchhaptic.crown.strength2';
const VIBRATOR_TYPE_SIX = 'watchhaptic.crown.strength6';
const CROWN_EVENT_FLAG = 30;
const CROWN_CONTROL_RATIO = 2.10;
const CROWN_SENSITIVITY_LOW = 0.5;
const CROWN_SENSITIVITY_MEDIUM = 1;
const CROWN_SENSITIVITY_HIGH = 2;
const INVALID_TIMEOUT_ID = -1;
const RESTORE_TIMEOUT = 3000;

export var AnimatorStatus;
(function (AnimatorStatus) {
    AnimatorStatus[AnimatorStatus["MIN"] = 0] = "MIN";
    AnimatorStatus[AnimatorStatus["MAX"] = 1] = "MAX";
    AnimatorStatus[AnimatorStatus["NORMAL"] = 2] = "NORMAL";
})(AnimatorStatus || (AnimatorStatus = {}));

export var ClipPathArc;
(function (ClipPathArc) {
    ClipPathArc[ClipPathArc["ARC1"] = 0] = "ARC1";
    ClipPathArc[ClipPathArc["ARC2"] = 1] = "ARC2";
    ClipPathArc[ClipPathArc["ARC3"] = 2] = "ARC3";
    ClipPathArc[ClipPathArc["ARC4"] = 3] = "ARC4";
})(ClipPathArc || (ClipPathArc = {}));
let ArcSliderValueOptions = class ArcSliderValueOptions {
    constructor(options) {
        this.progress = options?.progress ?? PROGRESS_DEFAULT;
        this.min = options?.min ?? MIN_DEFAULT;
        this.max = options?.max ?? MAX_DEFAULT;
    }
};
__decorate([
    Trace
], ArcSliderValueOptions.prototype, "progress", void 0);
__decorate([
    Trace
], ArcSliderValueOptions.prototype, "min", void 0);
__decorate([
    Trace
], ArcSliderValueOptions.prototype, "max", void 0);
ArcSliderValueOptions = __decorate([
    ObservedV2
], ArcSliderValueOptions);

export { ArcSliderValueOptions };
let ArcSliderLayoutOptions = class ArcSliderLayoutOptions {
    constructor(options) {
        this.x = options?.x ?? X_DEFAULT;
        this.y = options?.y ?? Y_DEFAULT;
        this.radius = options?.radius ?? RADIUS_DEFAULT;
        this.padding = options?.padding ?? PADDING_DEFAULT;
        this.startAngle = options?.startAngle ?? START_ANGLE_DEFAULT;
        this.endAngle = options?.endAngle ?? END_ANGLE_DEFAULT;
        this.activeStartAngle = options?.activeStartAngle ?? ACTIVE_START_ANGLE_DEFAULT;
        this.activeEndAngle = options?.activeEndAngle ?? ACTIVE_END_ANGLE_DEFAULT;
        this.reverse = options?.reverse ?? REVERSE_DEFAULT;
    }
};
__decorate([
    Trace
], ArcSliderLayoutOptions.prototype, "x", void 0);
__decorate([
    Trace
], ArcSliderLayoutOptions.prototype, "y", void 0);
__decorate([
    Trace
], ArcSliderLayoutOptions.prototype, "radius", void 0);
__decorate([
    Trace
], ArcSliderLayoutOptions.prototype, "padding", void 0);
__decorate([
    Trace
], ArcSliderLayoutOptions.prototype, "startAngle", void 0);
__decorate([
    Trace
], ArcSliderLayoutOptions.prototype, "endAngle", void 0);
__decorate([
    Trace
], ArcSliderLayoutOptions.prototype, "activeStartAngle", void 0);
__decorate([
    Trace
], ArcSliderLayoutOptions.prototype, "activeEndAngle", void 0);
__decorate([
    Trace
], ArcSliderLayoutOptions.prototype, "reverse", void 0);
ArcSliderLayoutOptions = __decorate([
    ObservedV2
], ArcSliderLayoutOptions);

export { ArcSliderLayoutOptions };
let ArcSliderStyleOptions = class ArcSliderStyleOptions {
    constructor(options) {
        this.trackThickness = options?.trackThickness ?? TRACK_THICKNESS_DEFAULT;
        this.activeTrackThickness = options?.activeTrackThickness ?? ACTIVE_TRACK_THICKNESS_DEFAULT;
        this.trackColor = options?.trackColor ?? TRACK_COLOR_DEFAULT;
        this.selectedColor = options?.selectedColor ?? SELECTED_COLOR_DEFAULT;
        this.trackBlur = options?.trackBlur ?? TRACK_BLUR_DEFAULT;
    }
};
__decorate([
    Trace
], ArcSliderStyleOptions.prototype, "trackThickness", void 0);
__decorate([
    Trace
], ArcSliderStyleOptions.prototype, "activeTrackThickness", void 0);
__decorate([
    Trace
], ArcSliderStyleOptions.prototype, "trackColor", void 0);
__decorate([
    Trace
], ArcSliderStyleOptions.prototype, "selectedColor", void 0);
__decorate([
    Trace
], ArcSliderStyleOptions.prototype, "trackBlur", void 0);
ArcSliderStyleOptions = __decorate([
    ObservedV2
], ArcSliderStyleOptions);

export { ArcSliderStyleOptions };
let ArcSliderOptions = class ArcSliderOptions {
    constructor(options) {
        this.valueOptions = options?.valueOptions ?? new ArcSliderValueOptions();
        this.layoutOptions = options?.layoutOptions ?? new ArcSliderLayoutOptions();
        this.styleOptions = options?.styleOptions ?? new ArcSliderStyleOptions();
        this.digitalCrownSensitivity = options?.digitalCrownSensitivity ?? CrownSensitivity.MEDIUM;
        this.onTouch = options?.onTouch ?? ((event) => {
        });
        this.onChange = options?.onChange ?? ((progress) => {
        });
    }
};
__decorate([
    Trace
], ArcSliderOptions.prototype, "valueOptions", void 0);
__decorate([
    Trace
], ArcSliderOptions.prototype, "layoutOptions", void 0);
__decorate([
    Trace
], ArcSliderOptions.prototype, "styleOptions", void 0);
__decorate([
    Trace
], ArcSliderOptions.prototype, "digitalCrownSensitivity", void 0);
__decorate([
    Trace
], ArcSliderOptions.prototype, "onTouch", void 0);
__decorate([
    Trace
], ArcSliderOptions.prototype, "onChange", void 0);
ArcSliderOptions = __decorate([
    ObservedV2
], ArcSliderOptions);

export { ArcSliderOptions };

export class DrawParameters {
    constructor() {
        this.lineWidth = 0;
        this.radius = 0;
        this.trackEndAngle = 0;
        this.trackStartAngle = 0;
        this.selectedStartAngle = 0;
        this.selectedEndAngle = 0;
        this.trackColor = ColorMetrics.resourceColor(TRACK_COLOR_DEFAULT);
        this.selectedColor = ColorMetrics.resourceColor(SELECTED_COLOR_DEFAULT);
        this.reverse = REVERSE_DEFAULT;
        this.x = X_DEFAULT;
        this.y = Y_DEFAULT;
        this.blur = TRACK_BLUR_DEFAULT;
        this.uiContext = undefined;
    }
}

export function nearEqual(num1, num2) {
    return Math.abs(num1 - num2) < APPROXIMATE_NUMBER;
}

export class MyFullDrawModifier extends DrawModifier {
    constructor(parameters) {
        super();
        this.parameters = new DrawParameters();
        this.parameters = parameters;
    }

    parseColorString(color) {
        return { alpha: color.alpha, red: color.red, green: color.green, blue: color.blue };
    }

    drawTrack(context) {
        if (this.parameters.uiContext === undefined) {
            hilog.error(0x3900, 'ArcSlider', `uiContext is undefined`);
            return;
        }
        const canvas = context.canvas;
        const pen = new drawing.Pen();
        pen.setAntiAlias(true);
        pen.setColor(this.parseColorString(this.parameters.trackColor));
        pen.setStrokeWidth(this.parameters.uiContext.vp2px(this.parameters.lineWidth));
        pen.setCapStyle(drawing.CapStyle.ROUND_CAP);
        canvas.attachPen(pen);
        const path = new drawing.Path();
        const leftTopX = this.parameters.uiContext.vp2px(this.parameters.x - this.parameters.radius);
        const leftTopY = this.parameters.uiContext.vp2px(this.parameters.y - this.parameters.radius);
        const rightBottomX = this.parameters.uiContext.vp2px(this.parameters.x + this.parameters.radius);
        const rightBottomY = this.parameters.uiContext.vp2px(this.parameters.y + this.parameters.radius);
        let startAngle;
        let sweepAngle;
        const trackEndAngle = this.parameters.trackEndAngle * RADIAN_TO_ANGLE;
        const delta = (this.parameters.trackStartAngle - this.parameters.trackEndAngle) * RADIAN_TO_ANGLE;
        if (this.parameters.reverse && this.parameters.selectedStartAngle < this.parameters.trackEndAngle) {
            startAngle = trackEndAngle;
            sweepAngle = TWO_PI_ANGLE + delta;
        }
        else if (!this.parameters.reverse && this.parameters.selectedStartAngle > this.parameters.trackEndAngle) {
            startAngle = this.parameters.trackStartAngle * RADIAN_TO_ANGLE;
            sweepAngle = TWO_PI_ANGLE - delta;
        }
        else {
            startAngle = trackEndAngle;
            sweepAngle = delta;
        }
        path.arcTo(leftTopX, leftTopY, rightBottomX, rightBottomY, startAngle, sweepAngle);
        canvas.drawPath(path);
        canvas.detachPen();
    }

    drawSelection(context) {
        if (this.parameters.uiContext === undefined) {
            hilog.error(0x3900, 'ArcSlider', `uiContext is undefined`);
            return;
        }
        if (nearEqual(this.parameters.selectedStartAngle, this.parameters.selectedEndAngle)) {
            return;
        }
        const canvas = context.canvas;
        let pen = new drawing.Pen();
        pen.setAntiAlias(true);
        pen.setColor(this.parseColorString(this.parameters.selectedColor));
        pen.setStrokeWidth(this.parameters.uiContext.vp2px(this.parameters.lineWidth));
        pen.setCapStyle(drawing.CapStyle.ROUND_CAP);
        canvas.attachPen(pen);
        let path = new drawing.Path();
        const leftTopX = this.parameters.uiContext.vp2px(this.parameters.x - this.parameters.radius);
        const leftTopY = this.parameters.uiContext.vp2px(this.parameters.y - this.parameters.radius);
        const rightBottomX = this.parameters.uiContext.vp2px(this.parameters.x + this.parameters.radius);
        const rightBottomY = this.parameters.uiContext.vp2px(this.parameters.y + this.parameters.radius);
        let startAngle;
        let sweepAngle;
        const selectedEndAngle = this.parameters.selectedEndAngle * RADIAN_TO_ANGLE;
        const selectedStartAngle = this.parameters.selectedStartAngle * RADIAN_TO_ANGLE;
        const delta = (this.parameters.selectedStartAngle - this.parameters.selectedEndAngle) * RADIAN_TO_ANGLE;
        if (this.parameters.reverse && this.parameters.selectedStartAngle < this.parameters.trackEndAngle &&
            this.parameters.selectedEndAngle > this.parameters.selectedStartAngle) {
            startAngle = selectedEndAngle - TWO_PI_ANGLE;
            sweepAngle = TWO_PI_ANGLE + delta;
        }
        else if (!this.parameters.reverse && this.parameters.selectedStartAngle > this.parameters.trackEndAngle &&
            this.parameters.selectedEndAngle < this.parameters.selectedStartAngle) {
            startAngle = selectedStartAngle - TWO_PI_ANGLE;
            sweepAngle = TWO_PI_ANGLE - delta;
        }
        else {
            startAngle = selectedEndAngle;
            sweepAngle = delta;
        }
        path.arcTo(leftTopX, leftTopY, rightBottomX, rightBottomY, startAngle, sweepAngle);
        canvas.drawPath(path);
        canvas.detachPen();
    }

    drawContent(context) {
        this.drawTrack(context);
    }

    drawFront(context) {
        this.drawSelection(context);
    }
}

export class ArcSlider extends ViewV2 {
    constructor(parent, params, __localStorage, elmtId = -1, paramsLambda, extraInfo) {
        super(parent, elmtId, extraInfo);
        this.initParam("options", (params && "options" in params) ? params.options : new ArcSliderOptions());
        this.lineWidth = 0;
        this.radius = 0;
        this.trackStartAngle = 0;
        this.trackEndAngle = 0;
        this.selectedStartAngle = 0;
        this.selectedEndAngle = 0;
        this.selectRatioNow = 0;
        this.isEnlarged = false;
        this.clipPath = '';
        this.isReverse = false;
        this.isLargeArc = false;
        this.parameters = "parameters" in params ? params.parameters : new DrawParameters();
        this.fullModifier = "fullModifier" in params ? params.fullModifier : new MyFullDrawModifier(this.parameters);
        this.touchAnimator = "touchAnimator" in params ? params.touchAnimator : undefined;
        this.restoreAnimator = "restoreAnimator" in params ? params.restoreAnimator : undefined;
        this.maxRestoreAnimator = "maxRestoreAnimator" in params ? params.maxRestoreAnimator : undefined;
        this.minRestoreAnimator = "minRestoreAnimator" in params ? params.minRestoreAnimator : undefined;
        this.delta = "delta" in params ? params.delta : 0;
        this.crownDeltaAngle = "crownDeltaAngle" in params ? params.crownDeltaAngle : 0;
        this.lineWidthBegin = "lineWidthBegin" in params ? params.lineWidthBegin : 0;
        this.touchY = "touchY" in params ? params.touchY : 0;
        this.endAngleFinish = "endAngleFinish" in params ? params.endAngleFinish : 0;
        this.meter = "meter" in params ? params.meter : 0;
        this.trackStartAngleBegin = "trackStartAngleBegin" in params ? params.trackStartAngleBegin : 0;
        this.selectedEndAngleBegin = "selectedEndAngleBegin" in params ? params.selectedEndAngleBegin : 0;
        this.isTouchAnimatorFinished = "isTouchAnimatorFinished" in params ? params.isTouchAnimatorFinished : false;
        this.clickValue = "clickValue" in params ? params.clickValue : 0;
        this.normalStartAngle = "normalStartAngle" in params ? params.normalStartAngle : -START_ANGLE_DEFAULT;
        this.normalEndAngle = "normalEndAngle" in params ? params.normalEndAngle : -END_ANGLE_DEFAULT;
        this.activeStartAngle = "activeStartAngle" in params ? params.activeStartAngle : -ACTIVE_START_ANGLE_DEFAULT;
        this.activeEndAngle = "activeEndAngle" in params ? params.activeEndAngle : -ACTIVE_END_ANGLE_DEFAULT;
        this.selectedMaxOrMin = "selectedMaxOrMin" in params ? params.selectedMaxOrMin : AnimatorStatus.NORMAL;
        this.totalWidth = "totalWidth" in params ? params.totalWidth : TOTAL_LENGTH;
        this.totalHeight = "totalHeight" in params ? params.totalHeight : TOTAL_LENGTH;
        this.isFocus = "isFocus" in params ? params.isFocus : false;
        this.needVibrate = "needVibrate" in params ? params.needVibrate : true;
        this.crownEventCounter = "crownEventCounter" in params ? params.crownEventCounter : 0;
        this.finalizeConstruction();
    }

    onChange(monitor) {
        monitor.dirty.forEach((path) => {
            this.updateArcSlider();
        });
    }

    aboutToAppear() {
        this.updateArcSlider();
        this.setTouchAnimator();
        this.setMaxRestoreAnimator();
        this.setMinRestoreAnimator();
        this.setRestoreAnimator();
    }

    setTouchAnimator() {
        this.touchAnimator = animator.create({
            duration: 200,
            easing: 'friction',
            delay: 0,
            fill: 'forwards',
            direction: 'normal',
            iterations: 1,
            begin: 0,
            end: 1
        });
        this.touchAnimator.onFrame = (fraction) => {
            this.lineWidth = this.calcAnimatorChange(this.options.styleOptions.trackThickness, this.options.styleOptions.activeTrackThickness, fraction);
            this.selectedStartAngle = this.calcAnimatorChange(this.normalStartAngle, this.activeStartAngle, fraction) *
                ANGLE_TO_RADIAN;
            this.trackStartAngle = this.selectedStartAngle;
            this.trackEndAngle = this.calcAnimatorChange(this.normalEndAngle, this.activeEndAngle, fraction) *
                ANGLE_TO_RADIAN;
            this.setActivation();
            this.updateModifier();
            this.fullModifier.invalidate();
            this.calcBlur();
        };
        this.touchAnimator.onFinish = () => {
            this.isTouchAnimatorFinished = true;
            this.endAngleFinish = this.trackEndAngle;
        };
    }

    startTouchAnimator() {
        if (this.touchAnimator) {
            this.touchAnimator.play();
        }
    }

    setMaxRestoreAnimator() {
        this.maxRestoreAnimator = animator.create({
            duration: 333,
            easing: 'sharp',
            delay: 0,
            fill: 'forwards',
            direction: 'normal',
            iterations: 1,
            begin: 0,
            end: 1
        });
        this.maxRestoreAnimator.onFrame = (fraction) => {
            this.lineWidth = this.calcAnimatorChange(this.lineWidthBegin, this.options.styleOptions.activeTrackThickness, fraction);
            this.selectedEndAngle = this.calcAnimatorChange(this.selectedEndAngleBegin, this.endAngleFinish, fraction);
            this.trackEndAngle = this.selectedEndAngle;
            this.updateModifier();
            this.fullModifier.invalidate();
            this.calcBlur();
        };
        this.maxRestoreAnimator.onFinish = () => {
            this.selectedMaxOrMin = AnimatorStatus.NORMAL;
        };
    }

    startMaxRestoreAnimator() {
        if (this.maxRestoreAnimator) {
            this.maxRestoreAnimator.play();
        }
    }

    setMinRestoreAnimator() {
        this.minRestoreAnimator = animator.create({
            duration: 333,
            easing: 'sharp',
            delay: 0,
            fill: 'forwards',
            direction: 'normal',
            iterations: 1,
            begin: 0,
            end: 1
        });
        this.minRestoreAnimator.onFrame = (fraction) => {
            this.lineWidth = this.calcAnimatorChange(this.lineWidthBegin, this.options.styleOptions.activeTrackThickness, fraction);
            this.trackStartAngle = this.calcAnimatorChange(this.trackStartAngleBegin, this.activeStartAngle * ANGLE_TO_RADIAN, fraction);
            this.updateModifier();
            this.fullModifier.invalidate();
            this.calcBlur();
        };
        this.minRestoreAnimator.onFinish = () => {
            this.selectedMaxOrMin = AnimatorStatus.NORMAL;
        };
    }

    startMinRestoreAnimator() {
        if (this.minRestoreAnimator) {
            this.minRestoreAnimator.play();
        }
    }

    setRestoreAnimator() {
        this.restoreAnimator = animator.create({
            duration: 167,
            easing: 'friction',
            delay: 0,
            fill: 'forwards',
            direction: 'normal',
            iterations: 1,
            begin: 0,
            end: 1
        });
        this.restoreAnimator.onFrame = (fraction) => {
            this.lineWidth = this.calcAnimatorChange(this.options.styleOptions.activeTrackThickness, this.options.styleOptions.trackThickness, fraction);
            this.selectedStartAngle = this.calcAnimatorChange(this.activeStartAngle, this.normalStartAngle, fraction) *
                ANGLE_TO_RADIAN;
            this.trackStartAngle = this.selectedStartAngle;
            this.trackEndAngle = this.calcAnimatorChange(this.activeEndAngle, this.normalEndAngle, fraction) *
                ANGLE_TO_RADIAN;
            this.setNormal();
            this.updateModifier();
            this.fullModifier.invalidate();
            this.calcBlur();
        };
    }

    startRestoreAnimator() {
        if (this.restoreAnimator) {
            this.restoreAnimator.play();
        }
    }

    updateArcSlider() {
        this.checkParam();
        this.setReverse();
        this.calcBlur();
        if (!this.isEnlarged) {
            this.setNormal();
        }
        else {
            this.setActivation();
        }
        this.updateModifier();
        this.fullModifier.invalidate();
    }

    biggerThanZero(number1, number2, canEqualToZero) {
        if ((!canEqualToZero && number1 < 0) || (canEqualToZero && (number1 < 0 || number1 === 0))) {
            return number2;
        }
        else {
            return number1;
        }
    }

    checkParam() {
        if (this.options.valueOptions.max === this.options.valueOptions.min) {
            this.options.valueOptions.max = MAX_DEFAULT;
            this.options.valueOptions.min = MIN_DEFAULT;
            this.options.valueOptions.progress = PROGRESS_DEFAULT;
        }
        const min = this.options.valueOptions.min;
        const max = this.options.valueOptions.max;
        this.options.valueOptions.max = Math.max(min, max);
        this.options.valueOptions.min = Math.min(min, max);
        this.options.valueOptions.max = Math.max(this.options.valueOptions.max, this.options.valueOptions.min);
        this.options.valueOptions.min = Math.min(this.options.valueOptions.max, this.options.valueOptions.min);
        this.options.valueOptions.progress = Math.min(this.options.valueOptions.max, this.options.valueOptions.progress);
        this.options.valueOptions.progress = Math.max(this.options.valueOptions.min, this.options.valueOptions.progress);
        this.options.layoutOptions.startAngle = Math.min(this.options.layoutOptions.startAngle, PI_RADIAN);
        this.options.layoutOptions.startAngle = Math.max(this.options.layoutOptions.startAngle, -PI_RADIAN);
        this.options.layoutOptions.endAngle = Math.min(this.options.layoutOptions.endAngle, PI_RADIAN);
        this.options.layoutOptions.endAngle = Math.max(this.options.layoutOptions.endAngle, -PI_RADIAN);
        this.options.layoutOptions.activeStartAngle = Math.min(this.options.layoutOptions.activeStartAngle, PI_RADIAN);
        this.options.layoutOptions.activeStartAngle = Math.max(this.options.layoutOptions.activeStartAngle, -PI_RADIAN);
        this.options.layoutOptions.activeEndAngle = Math.min(this.options.layoutOptions.activeEndAngle, PI_RADIAN);
        this.options.layoutOptions.activeEndAngle = Math.max(this.options.layoutOptions.activeEndAngle, -PI_RADIAN);
        this.normalStartAngle = -this.options.layoutOptions.startAngle;
        this.normalEndAngle = -this.options.layoutOptions.endAngle;
        this.activeStartAngle = -this.options.layoutOptions.activeStartAngle;
        this.activeEndAngle = -this.options.layoutOptions.activeEndAngle;
        this.options.layoutOptions.padding = this.biggerThanZero(this.options.layoutOptions.padding, PADDING_DEFAULT, false);
        this.options.layoutOptions.radius = this.biggerThanZero(this.options.layoutOptions.radius, RADIUS_DEFAULT, false);
        this.options.styleOptions.trackBlur = this.biggerThanZero(this.options.styleOptions.trackBlur, TRACK_BLUR_DEFAULT, false);
        this.options.styleOptions.trackThickness = this.biggerThanZero(this.options.styleOptions.trackThickness, TRACK_THICKNESS_DEFAULT, true);
        this.options.styleOptions.activeTrackThickness = this.biggerThanZero(this.options.styleOptions.activeTrackThickness, ACTIVE_TRACK_THICKNESS_DEFAULT, true);
    }

    updateModifier() {
        this.fullModifier.parameters.lineWidth = this.lineWidth;
        this.fullModifier.parameters.radius = this.radius;
        this.fullModifier.parameters.selectedStartAngle = this.selectedStartAngle;
        this.fullModifier.parameters.trackEndAngle = this.trackEndAngle;
        this.fullModifier.parameters.trackStartAngle = this.trackStartAngle;
        this.fullModifier.parameters.selectedEndAngle = this.selectedEndAngle;
        this.fullModifier.parameters.trackColor = ColorMetrics.resourceColor(this.options.styleOptions.trackColor);
        this.fullModifier.parameters.selectedColor = ColorMetrics.resourceColor(this.options.styleOptions.selectedColor);
        this.fullModifier.parameters.reverse = this.options.layoutOptions.reverse;
        this.fullModifier.parameters.x = this.options.layoutOptions.x;
        this.fullModifier.parameters.y = this.options.layoutOptions.y;
        this.fullModifier.parameters.blur = this.options.styleOptions.trackBlur;
        this.fullModifier.parameters.uiContext = this.getUIContext();
    }

    setReverse() {
        if (!this.options.layoutOptions.reverse) {
            const startAngleInitial = this.normalStartAngle;
            const endAngleInitial = this.normalEndAngle;
            const activeStartAngleInitial = this.activeStartAngle;
            const activeEndAngleInitial = this.activeEndAngle;
            this.normalStartAngle = endAngleInitial;
            this.normalEndAngle = startAngleInitial;
            this.activeStartAngle = activeEndAngleInitial;
            this.activeEndAngle = activeStartAngleInitial;
        }
    }

    setNormal() {
        this.lineWidth = this.options.styleOptions.trackThickness;
        this.selectedStartAngle = this.normalStartAngle * ANGLE_TO_RADIAN;
        this.trackEndAngle = this.normalEndAngle * ANGLE_TO_RADIAN;
        this.trackStartAngle = this.normalStartAngle * ANGLE_TO_RADIAN;
        if (this.trackEndAngle > this.selectedStartAngle) {
            this.isEndAfterStart();
        }
        else {
            this.isEndBeforeStart();
        }
        this.radius = this.options.layoutOptions.radius - (this.lineWidth / 2) -
        this.options.layoutOptions.padding;
        this.calcBlur();
    }

    setActivation() {
        if (this.trackEndAngle > this.selectedStartAngle) {
            this.isEndAfterStart();
        }
        else {
            this.isEndBeforeStart();
        }
        this.radius = this.options.layoutOptions.radius - (this.lineWidth / 2);
        this.calcBlur();
    }

    isEndAfterStart() {
        const selectedRatio = (this.options.valueOptions.progress - this.options.valueOptions.min) /
            (this.options.valueOptions.max - this.options.valueOptions.min);
        const deltaRadian = this.trackEndAngle - this.selectedStartAngle;
        const selectedAngle = selectedRatio * Math.abs(deltaRadian);
        const specialSelectedAngle = selectedRatio * (TWO_PI_RADIAN - Math.abs(deltaRadian));
        if (this.options.layoutOptions.reverse) {
            this.selectedEndAngle = this.selectedStartAngle - specialSelectedAngle;
            if (this.selectedEndAngle <= -Math.PI) {
                this.selectedEndAngle += TWO_PI_RADIAN;
            }
        }
        else {
            this.selectedEndAngle = this.selectedStartAngle + selectedAngle;
        }
    }

    isEndBeforeStart() {
        const selectedRatio = (this.options.valueOptions.progress - this.options.valueOptions.min) /
            (this.options.valueOptions.max - this.options.valueOptions.min);
        const deltaRadian = this.trackEndAngle - this.selectedStartAngle;
        const selectedAngle = selectedRatio * Math.abs(deltaRadian);
        const specialSelectedAngle = selectedRatio * (TWO_PI_RADIAN - Math.abs(deltaRadian));
        if (this.options.layoutOptions.reverse) {
            this.selectedEndAngle = this.selectedStartAngle - selectedAngle;
        }
        else {
            this.selectedEndAngle = this.selectedStartAngle + specialSelectedAngle;
            if (this.selectedEndAngle >= Math.PI) {
                this.selectedEndAngle -= TWO_PI_RADIAN;
            }
        }
    }

    calcBlurIsReverseTrue() {
        this.isReverse = false;
        this.isLargeArc = false;
        if ((this.trackStartAngle > this.trackEndAngle) && (this.trackStartAngle - this.trackEndAngle > Math.PI) ||
            (this.trackStartAngle <= this.trackEndAngle && (TWO_PI_RADIAN + this.trackStartAngle - this.trackEndAngle >
            Math.PI))) {
            this.isLargeArc = true;
        }
    }

    calcBlurIsReverseFalse() {
        this.isReverse = true;
        this.isLargeArc = false;
        if ((this.trackStartAngle > this.trackEndAngle) && (TWO_PI_RADIAN + this.trackEndAngle - this.trackStartAngle >
        Math.PI) || (this.trackStartAngle <= this.trackEndAngle) && (this.trackEndAngle - this.trackStartAngle > Math.PI)) {
            this.isLargeArc = true;
        }
    }

    calcPathX(isRLarge) {
        if (this.parameters.uiContext) {
            const halfLineWidth = this.parameters.lineWidth / 2;
            let distance = this.parameters.radius;
            let cosAngle = 0;
            if (isRLarge === ClipPathArc.ARC1) {
                distance += halfLineWidth;
                cosAngle = this.parameters.trackStartAngle;
            }
            else if (isRLarge === ClipPathArc.ARC2) {
                distance += halfLineWidth;
                cosAngle = this.parameters.trackEndAngle;
            }
            else if (isRLarge === ClipPathArc.ARC3) {
                distance -= halfLineWidth;
                cosAngle = this.parameters.trackEndAngle;
            }
            else if (isRLarge === ClipPathArc.ARC4) {
                distance -= halfLineWidth;
                cosAngle = this.parameters.trackStartAngle;
            }
            return (this.parameters.uiContext.vp2px(this.parameters.x + distance * Math.cos(cosAngle)));
        }
        return 0;
    }

    calcPathY(isRLarge) {
        if (this.parameters.uiContext) {
            const halfLineWidth = this.parameters.lineWidth / 2;
            let distance = this.parameters.radius;
            let sinAngle = 0;
            if (isRLarge === ClipPathArc.ARC1) {
                distance += halfLineWidth;
                sinAngle = this.parameters.trackStartAngle;
            }
            else if (isRLarge === ClipPathArc.ARC2) {
                distance += halfLineWidth;
                sinAngle = this.parameters.trackEndAngle;
            }
            else if (isRLarge === ClipPathArc.ARC3) {
                distance -= halfLineWidth;
                sinAngle = this.parameters.trackEndAngle;
            }
            else if (isRLarge === ClipPathArc.ARC4) {
                distance -= halfLineWidth;
                sinAngle = this.parameters.trackStartAngle;
            }
            return (this.parameters.uiContext.vp2px(this.parameters.y + (distance) * Math.sin(sinAngle)));
        }
        return 0;
    }

    calcPathR(isRLarge) {
        if (this.parameters.uiContext) {
            const halfLineWidth = this.parameters.lineWidth / 2;
            let pathR = this.parameters.uiContext.vp2px(halfLineWidth);
            if (isRLarge === ClipPathArc.ARC2) {
                pathR += this.parameters.uiContext.vp2px(this.parameters.radius);
            }
            else if (isRLarge === ClipPathArc.ARC4) {
                pathR = this.parameters.uiContext.vp2px(this.parameters.radius) - pathR;
            }
            return pathR;
        }
        return 0;
    }

    setClipPath() {
        if (this.parameters.uiContext) {
            const sourcePoint = `M${this.calcPathX(ClipPathArc.ARC4)} ${this.calcPathY(ClipPathArc.ARC4)}`;
            const arc1 = ` A${this.calcPathR(ClipPathArc.ARC1)} ${this.calcPathR(ClipPathArc.ARC1)} 0 1 ` +
                `${Number(this.isReverse)} ${this.calcPathX(ClipPathArc.ARC1)} ${this.calcPathY(ClipPathArc.ARC1)}`;
            const arc2 = ` A${this.calcPathR(ClipPathArc.ARC2)} ${this.calcPathR(ClipPathArc.ARC2)} 0 ` +
                `${Number(this.isLargeArc)} ${Number(this.isReverse)} ${this.calcPathX(ClipPathArc.ARC2)} ` +
                `${this.calcPathY(ClipPathArc.ARC2)}`;
            const arc3 = ` A${this.calcPathR(ClipPathArc.ARC3)} ${this.calcPathR(ClipPathArc.ARC3)} 0 1 ` +
                `${Number(this.isReverse)} ${this.calcPathX(ClipPathArc.ARC3)} ${this.calcPathY(ClipPathArc.ARC3)}`;
            const arc4 = ` A${this.calcPathR(ClipPathArc.ARC4)} ${this.calcPathR(ClipPathArc.ARC4)} 0 ` +
                `${Number(this.isLargeArc)} ${Number(!this.isReverse)} ${this.calcPathX(ClipPathArc.ARC4)} ` +
                `${this.calcPathY(ClipPathArc.ARC4)}`;
            this.clipPath = sourcePoint + arc1 + arc2 + arc3 + arc4;
        }
    }

    calcBlur() {
        if (this.parameters.reverse) {
            this.calcBlurIsReverseTrue();
        }
        else {
            this.calcBlurIsReverseFalse();
        }
        this.setClipPath();
    }

    calcAnimatorChange(start, end, fraction) {
        return (fraction * (end - start) + start);
    }

    calcClickValue(clickX, clickY) {
        if (clickY - this.options.layoutOptions.y > this.radius) {
            clickY = this.radius + this.options.layoutOptions.y;
        }
        else if (this.options.layoutOptions.y - clickY > this.radius) {
            clickY = this.options.layoutOptions.y - this.radius;
        }
        const sin = Math.abs(clickY - this.options.layoutOptions.y) / this.radius;
        let radian = Math.asin(sin);
        if ((clickX < this.options.layoutOptions.x) && (clickY > this.options.layoutOptions.y)) {
            radian = Math.PI - radian;
        }
        else if ((clickX < this.options.layoutOptions.x) && (clickY < this.options.layoutOptions.y)) {
            radian = radian - Math.PI;
        }
        else if ((clickX > this.options.layoutOptions.x) && (clickY < this.options.layoutOptions.y)) {
            radian = -radian;
        }
        this.selectedEndAngle = radian;
        const delta = this.selectedStartAngle - this.selectedEndAngle;
        if ((this.parameters.reverse) && (this.trackEndAngle > this.selectedStartAngle)) {
            if (this.selectedEndAngle < this.selectedStartAngle) {
                this.selectRatioNow = delta / (TWO_PI_RADIAN + this.selectedStartAngle - this.trackEndAngle);
            }
            else {
                this.selectRatioNow = (TWO_PI_RADIAN + delta) / (TWO_PI_RADIAN + this.selectedStartAngle - this.trackEndAngle);
            }
        }
        else if ((!this.parameters.reverse) && (this.trackEndAngle < this.selectedStartAngle)) {
            if (this.selectedEndAngle < this.selectedStartAngle) {
                this.selectRatioNow = (TWO_PI_RADIAN - delta) / (TWO_PI_RADIAN - this.selectedStartAngle + this.trackEndAngle);
            }
            else {
                this.selectRatioNow = -delta / (TWO_PI_RADIAN - this.selectedStartAngle + this.trackEndAngle);
            }
        }
        else {
            this.selectRatioNow = -delta / (this.trackEndAngle - this.selectedStartAngle);
        }
        this.selectRatioNow = Math.min(1, this.selectRatioNow);
        this.selectRatioNow = Math.max(0, this.selectRatioNow);
        this.clickValue = this.selectRatioNow * (this.options.valueOptions.max - this.options.valueOptions.min) +
        this.options.valueOptions.min;
        this.options.valueOptions.progress = this.clickValue;
        this.setActivation();
        this.updateModifier();
        this.fullModifier.invalidate();
    }

    calcSin(radian) {
        if ((radian >= Math.PI / 2) && (radian <= Math.PI)) {
            return Math.sin(radian);
        }
        if ((radian <= Math.PI / 2) && (radian >= -Math.PI / 2)) {
            return 2 - Math.sin(radian);
        }
        if ((radian >= -Math.PI) && (radian <= -Math.PI / 2)) {
            return 4 + Math.sin(radian);
        }
        return 0;
    }

    calcValue(moveY) {
        this.delta = this.touchY - moveY;
        let total;
        if (this.parameters.reverse) {
            if (this.selectedStartAngle > this.trackEndAngle) {
                total = this.radius * (this.calcSin(this.activeEndAngle * ANGLE_TO_RADIAN) -
                this.calcSin(this.activeStartAngle * ANGLE_TO_RADIAN));
            }
            else {
                total = this.radius * (this.calcSin(this.activeEndAngle * ANGLE_TO_RADIAN) + 4 -
                this.calcSin(this.activeStartAngle * ANGLE_TO_RADIAN));
            }
        }
        else {
            if (this.selectedStartAngle > this.trackEndAngle) {
                total = this.radius * (this.calcSin(this.activeStartAngle * ANGLE_TO_RADIAN) + 4 -
                this.calcSin(this.activeEndAngle * ANGLE_TO_RADIAN));
            }
            else {
                total = this.radius * (this.calcSin(this.activeStartAngle * ANGLE_TO_RADIAN) -
                this.calcSin(this.activeEndAngle * ANGLE_TO_RADIAN));
            }
        }
        let valueNow = (this.options.valueOptions.progress - this.options.valueOptions.min) /
            (this.options.valueOptions.max - this.options.valueOptions.min);
        valueNow += this.delta / total;
        valueNow = Math.min(1, valueNow);
        valueNow = Math.max(0, valueNow);
        this.options.valueOptions.progress = valueNow * (this.options.valueOptions.max - this.options.valueOptions.min) +
        this.options.valueOptions.min;
        this.setActivation();
        this.updateModifier();
        this.fullModifier.invalidate();
        this.touchY = moveY;
    }

    calcCrownTotal(activeStartAngle, activeEndAngle) {
        if (activeEndAngle > activeStartAngle) {
            if (this.options.layoutOptions.reverse) {
                return (2 * PI_RADIAN - Math.abs(activeEndAngle - activeStartAngle)) * ANGLE_TO_RADIAN;
            }
            else {
                return Math.abs(activeEndAngle - activeStartAngle) * ANGLE_TO_RADIAN;
            }
        }
        else {
            if (this.options.layoutOptions.reverse) {
                return Math.abs(activeEndAngle - activeStartAngle) * ANGLE_TO_RADIAN;
            }
            else {
                return (2 * PI_RADIAN - Math.abs(activeEndAngle - activeStartAngle)) * ANGLE_TO_RADIAN;
            }
        }
    }

    calcCrownValue(deltaCrownAngle) {
        const totalAngle = this.calcCrownTotal(this.activeStartAngle, this.activeEndAngle);
        const totalValue = this.options.valueOptions.max - this.options.valueOptions.min;
        let valueNow = (this.options.valueOptions.progress - this.options.valueOptions.min) / totalValue;
        valueNow += deltaCrownAngle / totalAngle;
        valueNow = Math.min(1, valueNow);
        valueNow = Math.max(0, valueNow);
        this.options.valueOptions.progress = valueNow * totalValue + this.options.valueOptions.min;
        if (this.options.valueOptions.progress === this.options.valueOptions.max ||
            this.options.valueOptions.progress === this.options.valueOptions.min) {
            if (this.needVibrate) {
                this.needVibrate = false;
                try {
                    const ret = vibrator.isSupportEffectSync(VIBRATOR_TYPE_SIX);
                    if (ret) {
                        vibrator.startVibration({
                            type: 'preset',
                            effectId: VIBRATOR_TYPE_SIX,
                            count: 1,
                        }, {
                            usage: 'unknown'
                        }, (error) => {
                            if (error) {
                                hilog.error(0x3900, 'ArcSlider', `Failed to start vibration.
                  Code: ${error.code}, message: ${error.message}`);
                                return;
                            }
                            hilog.info(0x3900, 'ArcSlider', 'Succeed in starting vibration');
                        });
                    }
                    else {
                        hilog.error(0x3900, 'ArcSlider', `watchhaptic.crown.strength6 is not supported`);
                    }
                }
                catch (error) {
                    const e = error;
                    hilog.error(0x3900, 'ArcSlider', `An unexpected error occurred in starting vibration.
                    Code: ${e.code}, message: ${e.message}`);
                }
            }
        }
        else {
            this.needVibrate = true;
        }
        this.setActivation();
        this.updateModifier();
        this.fullModifier.invalidate();
    }

    calcMaxValueDeltaIsPositive() {
        if (this.options.layoutOptions.reverse) {
            if ((this.selectedEndAngle >= (this.endAngleFinish - ANGLE_OVER_MIN *
                ANGLE_TO_RADIAN)) && (this.lineWidth >= this.options.styleOptions.activeTrackThickness *
                (1 - LENGTH_OVER_MIN))) {
                this.selectedEndAngle -= (ANGLE_OVER_MIN * ANGLE_TO_RADIAN) * Math.abs(this.delta) /
                    (ANGLE_OVER_MIN * ANGLE_TO_RADIAN * this.radius + Math.abs(this.delta));
                this.lineWidth -= LENGTH_OVER_MIN * this.lineWidth * Math.abs(this.delta) / (LENGTH_OVER_MIN * this.lineWidth +
                Math.abs(this.delta));
                this.trackEndAngle = this.selectedEndAngle;
            }
            if ((this.selectedEndAngle / ANGLE_TO_RADIAN) <= (this.endAngleFinish / ANGLE_TO_RADIAN - ANGLE_OVER_MIN)) {
                this.selectedEndAngle = this.endAngleFinish - ANGLE_OVER_MIN * ANGLE_TO_RADIAN;
                this.trackEndAngle = this.selectedEndAngle;
            }
            if (this.lineWidth <= this.options.styleOptions.activeTrackThickness * (1 - LENGTH_OVER_MIN)) {
                this.lineWidth = this.options.styleOptions.activeTrackThickness * (1 - LENGTH_OVER_MIN);
            }
        }
        else {
            if ((this.selectedEndAngle <= (this.endAngleFinish + ANGLE_OVER_MIN * ANGLE_TO_RADIAN)) &&
                (this.lineWidth >= this.options.styleOptions.activeTrackThickness * (1 - LENGTH_OVER_MIN))) {
                this.selectedEndAngle += (ANGLE_OVER_MIN * ANGLE_TO_RADIAN) * Math.abs(this.delta) /
                    (ANGLE_OVER_MIN * ANGLE_TO_RADIAN * this.radius + Math.abs(this.delta));
                this.lineWidth -= LENGTH_OVER_MIN * this.lineWidth * Math.abs(this.delta) / (LENGTH_OVER_MIN *
                this.lineWidth + Math.abs(this.delta));
                this.trackEndAngle = this.selectedEndAngle;
            }
            if (this.lineWidth <= this.options.styleOptions.activeTrackThickness * (1 - LENGTH_OVER_MIN)) {
                this.lineWidth = this.options.styleOptions.activeTrackThickness * (1 - LENGTH_OVER_MIN);
            }
            if (((this.selectedEndAngle / ANGLE_TO_RADIAN) >= (this.endAngleFinish / ANGLE_TO_RADIAN + ANGLE_OVER_MIN))) {
                this.selectedEndAngle = this.endAngleFinish + ANGLE_OVER_MIN * ANGLE_TO_RADIAN;
                this.trackEndAngle = this.selectedEndAngle;
            }
        }
    }

    calcMaxValueDeltaIsNegative() {
        if (this.options.layoutOptions.reverse) {
            if ((this.selectedEndAngle <= this.endAngleFinish) ||
                (this.lineWidth <= this.options.styleOptions.activeTrackThickness)) {
                this.selectedEndAngle += (ANGLE_OVER_MIN * ANGLE_TO_RADIAN) * Math.abs(this.delta) /
                    (ANGLE_OVER_MIN * ANGLE_TO_RADIAN * this.radius + Math.abs(this.delta));
                this.lineWidth += LENGTH_OVER_MIN * this.lineWidth * Math.abs(this.delta) / (LENGTH_OVER_MIN *
                this.lineWidth + Math.abs(this.delta));
                this.trackEndAngle = this.selectedEndAngle;
            }
            if (this.selectedEndAngle >= this.endAngleFinish) {
                this.selectedEndAngle = this.endAngleFinish;
                this.trackEndAngle = this.selectedEndAngle;
            }
            if (this.lineWidth >= this.options.styleOptions.activeTrackThickness) {
                this.lineWidth = this.options.styleOptions.activeTrackThickness;
            }
        }
        else {
            if (((this.selectedEndAngle >= this.endAngleFinish) && (this.selectedEndAngle < this.selectedStartAngle)) ||
                (this.lineWidth <= this.options.styleOptions.activeTrackThickness)) {
                this.selectedEndAngle -= (ANGLE_OVER_MIN * ANGLE_TO_RADIAN) * Math.abs(this.delta) /
                    (ANGLE_OVER_MIN * ANGLE_TO_RADIAN * this.radius + Math.abs(this.delta));
                this.lineWidth += LENGTH_OVER_MIN * this.lineWidth * Math.abs(this.delta) / (LENGTH_OVER_MIN *
                this.lineWidth + Math.abs(this.delta));
                this.trackEndAngle = this.selectedEndAngle;
            }
            if (this.selectedEndAngle <= this.endAngleFinish) {
                this.selectedEndAngle = this.endAngleFinish;
                this.trackEndAngle = this.selectedEndAngle;
            }
            if (this.lineWidth >= this.options.styleOptions.activeTrackThickness) {
                this.lineWidth = this.options.styleOptions.activeTrackThickness;
            }
        }
    }

    calcMaxValue(moveY) {
        this.delta = this.touchY - moveY;
        if (this.delta >= 0) {
            this.calcMaxValueDeltaIsPositive();
        }
        else {
            this.calcMaxValueDeltaIsNegative();
        }
        this.updateModifier();
        this.fullModifier.invalidate();
        this.touchY = moveY;
        this.calcBlur();
    }

    calcMinValueDeltaIsNegative() {
        if (this.options.layoutOptions.reverse) {
            if ((this.trackStartAngle < this.selectedStartAngle + ANGLE_OVER_MIN * ANGLE_TO_RADIAN) &&
                (this.lineWidth >= this.options.styleOptions.activeTrackThickness * (1 - LENGTH_OVER_MIN))) {
                this.trackStartAngle += (ANGLE_OVER_MIN * ANGLE_TO_RADIAN) *
                Math.abs(this.delta) / (ANGLE_OVER_MIN * ANGLE_TO_RADIAN * this.radius +
                Math.abs(this.delta));
                this.lineWidth -= LENGTH_OVER_MIN * this.lineWidth * Math.abs(this.delta) /
                    (LENGTH_OVER_MIN * this.lineWidth + Math.abs(this.delta));
            }
            if ((this.trackStartAngle / ANGLE_TO_RADIAN) >= this.selectedStartAngle / ANGLE_TO_RADIAN + ANGLE_OVER_MIN) {
                this.trackStartAngle = this.selectedStartAngle + ANGLE_OVER_MIN * ANGLE_TO_RADIAN;
            }
            if (this.lineWidth <= this.options.styleOptions.activeTrackThickness * (1 - LENGTH_OVER_MIN)) {
                this.lineWidth = this.options.styleOptions.activeTrackThickness * (1 - LENGTH_OVER_MIN);
            }
        }
        else {
            if ((this.trackStartAngle > this.selectedStartAngle - ANGLE_OVER_MIN * ANGLE_TO_RADIAN) &&
                (this.lineWidth >= this.options.styleOptions.activeTrackThickness * (1 - LENGTH_OVER_MIN))) {
                this.trackStartAngle -= (ANGLE_OVER_MIN * ANGLE_TO_RADIAN) * Math.abs(this.delta) /
                    (ANGLE_OVER_MIN * ANGLE_TO_RADIAN * this.radius + Math.abs(this.delta));
                this.lineWidth -= LENGTH_OVER_MIN * this.lineWidth * Math.abs(this.delta) /
                    (LENGTH_OVER_MIN * this.lineWidth + Math.abs(this.delta));
            }
            if ((this.trackStartAngle / ANGLE_TO_RADIAN) <= this.selectedStartAngle / ANGLE_TO_RADIAN - ANGLE_OVER_MIN) {
                this.trackStartAngle = this.selectedStartAngle - ANGLE_OVER_MIN * ANGLE_TO_RADIAN;
            }
            if (this.lineWidth <= this.options.styleOptions.activeTrackThickness * (1 - LENGTH_OVER_MIN)) {
                this.lineWidth = this.options.styleOptions.activeTrackThickness * (1 - LENGTH_OVER_MIN);
            }
        }
    }

    calcMinValueDeltaIsPositive() {
        if (this.options.layoutOptions.reverse) {
            if ((this.trackStartAngle > this.selectedStartAngle) ||
                (this.lineWidth <= this.options.styleOptions.activeTrackThickness)) {
                this.trackStartAngle -= (ANGLE_OVER_MIN * ANGLE_TO_RADIAN) * Math.abs(this.delta) /
                    (ANGLE_OVER_MIN * ANGLE_TO_RADIAN * this.radius + Math.abs(this.delta));
                this.lineWidth += LENGTH_OVER_MIN * this.lineWidth * Math.abs(this.delta) /
                    (LENGTH_OVER_MIN * this.lineWidth + Math.abs(this.delta));
            }
            if (this.lineWidth > this.options.styleOptions.activeTrackThickness) {
                this.lineWidth = this.options.styleOptions.activeTrackThickness;
            }
            if (this.trackStartAngle < this.selectedStartAngle) {
                this.trackStartAngle = this.selectedStartAngle;
            }
        }
        else {
            if ((this.trackStartAngle < this.selectedStartAngle) ||
                (this.lineWidth <= this.options.styleOptions.activeTrackThickness)) {
                this.trackStartAngle += (ANGLE_OVER_MIN * ANGLE_TO_RADIAN) * Math.abs(this.delta) /
                    (ANGLE_OVER_MIN * ANGLE_TO_RADIAN * this.radius + Math.abs(this.delta));
                this.lineWidth += LENGTH_OVER_MIN * this.lineWidth * Math.abs(this.delta) /
                    (LENGTH_OVER_MIN * this.lineWidth + Math.abs(this.delta));
            }
            if (this.trackStartAngle > this.selectedStartAngle) {
                this.trackStartAngle = this.selectedStartAngle;
            }
            if (this.lineWidth >= this.options.styleOptions.activeTrackThickness) {
                this.lineWidth = this.options.styleOptions.activeTrackThickness;
            }
        }
    }

    calcMinValue(moveY) {
        this.delta = this.touchY - moveY;
        if (this.delta <= 0) {
            this.calcMinValueDeltaIsNegative();
        }
        else {
            this.calcMinValueDeltaIsPositive();
        }
        this.updateModifier();
        this.fullModifier.invalidate();
        this.touchY = moveY;
        this.calcBlur();
    }

    isHotRegion(touchX, touchY) {
        this.isFocus = false;
        const radius = Math.sqrt(Math.pow(touchX - this.options.layoutOptions.x, 2) +
        Math.pow(touchY - this.options.layoutOptions.y, 2));
        if ((radius < this.options.layoutOptions.radius - this.options.styleOptions.activeTrackThickness) ||
            (radius > this.options.layoutOptions.radius)) {
            return false;
        }
        const sin = Math.abs(touchY - this.options.layoutOptions.y) / radius;
        const radian = Math.asin(sin);
        let angle = radian / ANGLE_TO_RADIAN;
        if ((touchX < this.options.layoutOptions.x) && (touchY > this.options.layoutOptions.y)) {
            angle = PI_RADIAN - angle;
        }
        else if ((touchX < this.options.layoutOptions.x) && (touchY < this.options.layoutOptions.y)) {
            angle = angle - PI_RADIAN;
        }
        else if ((touchX > this.options.layoutOptions.x) && (touchY < this.options.layoutOptions.y)) {
            angle = -angle;
        }
        if ((this.options.layoutOptions.reverse && this.selectedStartAngle >= this.trackEndAngle && angle <=
            this.selectedStartAngle / ANGLE_TO_RADIAN && angle >= this.trackEndAngle / ANGLE_TO_RADIAN) ||
            (this.options.layoutOptions.reverse && this.selectedStartAngle <= this.trackEndAngle &&
                (angle <= this.selectedStartAngle / ANGLE_TO_RADIAN || angle >= this.trackEndAngle / ANGLE_TO_RADIAN)) ||
            (!this.options.layoutOptions.reverse && this.selectedStartAngle >= this.trackEndAngle &&
                (angle >= this.selectedStartAngle / ANGLE_TO_RADIAN || angle <= this.trackEndAngle / ANGLE_TO_RADIAN)) ||
            (!this.options.layoutOptions.reverse && this.selectedStartAngle <= this.trackEndAngle && angle >=
                this.selectedStartAngle / ANGLE_TO_RADIAN && angle <= this.trackEndAngle / ANGLE_TO_RADIAN)) {
            this.isFocus = true;
        }
        if (this.isFocus) {
            return true;
        }
        else {
            return false;
        }
    }

    calcDisplayControlRatio(crownSensitivity) {
        if (crownSensitivity === CrownSensitivity.LOW) {
            return CROWN_CONTROL_RATIO * CROWN_SENSITIVITY_LOW;
        }
        else if (crownSensitivity === CrownSensitivity.MEDIUM) {
            return CROWN_CONTROL_RATIO * CROWN_SENSITIVITY_MEDIUM;
        }
        else if (crownSensitivity === CrownSensitivity.HIGH) {
            return CROWN_CONTROL_RATIO * CROWN_SENSITIVITY_HIGH;
        }
        return CROWN_CONTROL_RATIO * CROWN_SENSITIVITY_MEDIUM;
    }

    onTouchEvent(event) {
        if (event.type === TouchType.Down) {
            this.onTouchDown(event);
        }
        else if (this.isEnlarged && event.type === TouchType.Up) {
            this.options.onTouch?.(event);
            if (this.meter !== INVALID_TIMEOUT_ID) {
                clearTimeout(this.meter);
                this.meter = INVALID_TIMEOUT_ID;
            }
            this.meter = setTimeout(() => {
                if (this.isEnlarged) {
                    this.isTouchAnimatorFinished = false;
                    this.isEnlarged = false;
                    this.startRestoreAnimator();
                    this.calcBlur();
                }
            }, RESTORE_TIMEOUT);
            this.isMaxOrMinAnimator();
        }
        else if ((this.isEnlarged) && (this.isTouchAnimatorFinished) && (event.type === TouchType.Move)) {
            this.options.onTouch?.(event);
            if (this.isFocus) {
                this.onTouchMove(event.touches[0].y);
                this.options.onChange?.(this.options.valueOptions.progress);
                if (this.meter !== INVALID_TIMEOUT_ID) {
                    clearTimeout(this.meter);
                    this.meter = INVALID_TIMEOUT_ID;
                }
            }
        }
    }

    onTouchDown(event) {
        if (!this.isEnlarged) {
            this.touchY = event.touches[0].y;
            if (this.meter !== INVALID_TIMEOUT_ID) {
                clearTimeout(this.meter);
                this.meter = INVALID_TIMEOUT_ID;
            }
            if (this.isHotRegion(event.touches[0].x, event.touches[0].y)) {
                this.options.onTouch?.(event);
                this.isEnlarged = true;
                this.startTouchAnimator();
                this.calcBlur();
            }
        }
        else {
            this.options.onTouch?.(event);
            this.touchY = event.touches[0].y;
            if (this.isHotRegion(event.touches[0].x, event.touches[0].y)) {
                if (this.meter !== INVALID_TIMEOUT_ID) {
                    clearTimeout(this.meter);
                    this.meter = INVALID_TIMEOUT_ID;
                }
                if (this.isTouchAnimatorFinished) {
                    this.calcClickValue(event.touches[0].x, event.touches[0].y);
                }
                this.touchY = event.touches[0].y;
                this.calcValue(event.touches[0].y);
                this.options.onChange?.(this.options.valueOptions.progress);
                this.setActivation();
                this.updateModifier();
                this.fullModifier.invalidate();
            }
        }
    }

    isMaxOrMinAnimator() {
        if (this.options.layoutOptions.reverse) {
            if (this.selectedMaxOrMin === AnimatorStatus.MAX && this.selectedEndAngle < this.endAngleFinish) {
                if (this.selectedStartAngle > this.trackEndAngle || (this.selectedStartAngle < this.trackEndAngle &&
                    this.selectedEndAngle > this.selectedStartAngle)) {
                    this.lineWidthBegin = this.lineWidth;
                    this.selectedEndAngleBegin = this.selectedEndAngle;
                    this.startMaxRestoreAnimator();
                }
            }
            if (this.selectedMaxOrMin === AnimatorStatus.MIN && this.trackStartAngle > this.selectedStartAngle) {
                if (this.selectedStartAngle > this.trackEndAngle || (this.selectedStartAngle < this.trackEndAngle &&
                    this.trackStartAngle < this.trackEndAngle)) {
                    this.lineWidthBegin = this.lineWidth;
                    this.trackStartAngleBegin = this.trackStartAngle;
                    this.startMinRestoreAnimator();
                    this.calcBlur();
                }
            }
        }
        else {
            if (this.selectedMaxOrMin === AnimatorStatus.MAX && this.selectedEndAngle > this.endAngleFinish) {
                if ((this.selectedStartAngle > this.trackEndAngle && this.selectedEndAngle < this.selectedStartAngle) ||
                    this.selectedStartAngle < this.trackEndAngle) {
                    this.lineWidthBegin = this.lineWidth;
                    this.selectedEndAngleBegin = this.selectedEndAngle;
                    this.startMaxRestoreAnimator();
                }
            }
            if (this.selectedMaxOrMin === AnimatorStatus.MIN && this.trackStartAngle < this.selectedStartAngle) {
                if ((this.selectedStartAngle > this.trackEndAngle && this.trackStartAngle > this.trackEndAngle) ||
                    this.selectedStartAngle < this.trackEndAngle) {
                    this.lineWidthBegin = this.lineWidth;
                    this.trackStartAngleBegin = this.trackStartAngle;
                    this.startMinRestoreAnimator();
                    this.calcBlur();
                }
            }
        }
    }

    onTouchMove(touchY) {
        if (this.options.layoutOptions.reverse) {
            this.onTouchMoveIsReverse(touchY);
        }
        else {
            this.onTouchMoveIsNotReverse(touchY);
        }
    }

    onTouchMoveIsReverse(touchY) {
        if (this.selectedStartAngle > this.endAngleFinish) {
            if (((this.selectedEndAngle < this.endAngleFinish || nearEqual(this.selectedEndAngle, this.endAngleFinish)) &&
                this.delta >= 0) || (this.selectedEndAngle < this.endAngleFinish && this.delta <= 0)) {
                this.selectedMaxOrMin = AnimatorStatus.MAX;
                this.calcMaxValue(touchY);
            }
            else if ((this.trackStartAngle >= this.selectedStartAngle && nearEqual(this.options.valueOptions.progress, this.options.valueOptions.min) && this.delta <= 0) || (this.trackStartAngle > this.selectedStartAngle &&
                this.delta >= 0)) {
                this.selectedMaxOrMin = AnimatorStatus.MIN;
                this.calcMinValue(touchY);
                this.calcBlur();
            }
            else {
                this.calcValue(touchY);
                this.selectedMaxOrMin = AnimatorStatus.NORMAL;
            }
        }
        else {
            if ((this.selectedEndAngle > this.selectedStartAngle && this.selectedEndAngle < this.endAngleFinish &&
                !nearEqual(this.selectedEndAngle, this.endAngleFinish) && this.delta <= 0) ||
                (this.selectedEndAngle > this.selectedStartAngle && (this.selectedEndAngle <= this.endAngleFinish ||
                nearEqual(this.selectedEndAngle, this.endAngleFinish)) && this.delta >= 0)) {
                this.selectedMaxOrMin = AnimatorStatus.MAX;
                this.calcMaxValue(touchY);
            }
            else if ((this.trackStartAngle >= this.selectedStartAngle && nearEqual(this.options.valueOptions.progress, this.options.valueOptions.min) && this.delta <= 0) || (this.trackStartAngle > this.selectedStartAngle &&
                this.delta >= 0)) {
                this.selectedMaxOrMin = AnimatorStatus.MIN;
                this.calcMinValue(touchY);
                this.calcBlur();
            }
            else {
                this.calcValue(touchY);
                this.selectedMaxOrMin = AnimatorStatus.NORMAL;
            }
        }
    }

    onTouchMoveIsNotReverse(touchY) {
        if (this.selectedStartAngle > this.endAngleFinish) {
            if ((this.selectedEndAngle < this.selectedStartAngle) && (this.selectedEndAngle > this.endAngleFinish) &&
                (!nearEqual(this.selectedEndAngle, this.endAngleFinish)) && this.delta <= 0) {
                this.selectedMaxOrMin = AnimatorStatus.MAX;
                this.calcMaxValue(touchY);
            }
            else if (this.selectedEndAngle < this.selectedStartAngle && (this.selectedEndAngle > this.endAngleFinish ||
            nearEqual(this.selectedEndAngle, this.endAngleFinish)) && this.delta >= 0) {
                this.selectedMaxOrMin = AnimatorStatus.MAX;
                this.calcMaxValue(touchY);
            }
            else if (nearEqual(this.options.valueOptions.progress, this.options.valueOptions.min) && this.delta <= 0 &&
                (this.trackStartAngle <= this.selectedStartAngle || nearEqual(this.selectedEndAngle, this.endAngleFinish))) {
                this.selectedMaxOrMin = AnimatorStatus.MIN;
                this.calcMinValue(touchY);
                this.calcBlur();
            }
            else if (this.trackStartAngle < this.selectedStartAngle && this.delta >= 0) {
                this.selectedMaxOrMin = AnimatorStatus.MIN;
                this.calcMinValue(touchY);
                this.calcBlur();
            }
            else {
                this.calcValue(touchY);
                this.selectedMaxOrMin = AnimatorStatus.NORMAL;
            }
        }
        else {
            if ((this.selectedEndAngle >= this.endAngleFinish || nearEqual(this.selectedEndAngle, this.endAngleFinish)) &&
                this.delta >= 0) {
                this.selectedMaxOrMin = AnimatorStatus.MAX;
                this.calcMaxValue(touchY);
            }
            else if (this.selectedEndAngle > this.endAngleFinish && this.delta >= 0) {
                this.selectedMaxOrMin = AnimatorStatus.MAX;
                this.calcMaxValue(touchY);
            }
            else if (nearEqual(this.options.valueOptions.progress, this.options.valueOptions.min) && this.delta <= 0 &&
                (this.trackStartAngle <= this.selectedStartAngle || nearEqual(this.selectedEndAngle, this.endAngleFinish))) {
                this.selectedMaxOrMin = AnimatorStatus.MIN;
                this.calcMinValue(touchY);
                this.calcBlur();
            }
            else if (this.trackStartAngle < this.selectedStartAngle && this.delta >= 0) {
                this.selectedMaxOrMin = AnimatorStatus.MIN;
                this.calcMinValue(touchY);
                this.calcBlur();
            }
            else {
                this.calcValue(touchY);
                this.selectedMaxOrMin = AnimatorStatus.NORMAL;
            }
        }
    }

    onDigitalCrownEvent(event) {
        if (event && this.isFocus) {
            this.crownEventCounter += 1;
            if (this.crownEventCounter % CROWN_EVENT_FLAG === 0) {
                try {
                    const ret = vibrator.isSupportEffectSync(VIBRATOR_TYPE_TWO);
                    if (ret) {
                        vibrator.startVibration({
                            type: 'preset',
                            effectId: VIBRATOR_TYPE_TWO,
                            count: 1,
                        }, {
                            usage: 'unknown'
                        }, (error) => {
                            if (error) {
                                hilog.error(0x3900, 'ArcSlider', `Failed to start vibration.
                            Code: ${error.code}, message: ${error.message}`);
                                this.crownEventCounter = 0;
                                return;
                            }
                            hilog.info(0x3900, 'ArcSlider', 'Succeed in starting vibration');
                        });
                    }
                    else {
                        hilog.error(0x3900, 'ArcSlider', `watchhaptic.crown.strength2 is not supported`);
                    }
                }
                catch (error) {
                    const e = error;
                    hilog.error(0x3900, 'ArcSlider', `An unexpected error occurred in starting vibration.
                      Code: ${e.code}, message: ${e.message}`);
                }
                this.crownEventCounter = 0;
            }
        }
        this.crownAction(event);
    }

    crownAction(event) {
        if (event.action === CrownAction.BEGIN && !this.isEnlarged) {
            if (this.meter !== INVALID_TIMEOUT_ID) {
                clearTimeout(this.meter);
                this.meter = INVALID_TIMEOUT_ID;
            }
            this.isEnlarged = true;
            this.startTouchAnimator();
            this.calcBlur();
            this.crownDeltaAngle = this.getUIContext().px2vp(-event.degree *
            this.calcDisplayControlRatio(this.options.digitalCrownSensitivity)) / this.radius;
            this.calcCrownValue(this.crownDeltaAngle);
        }
        else if (event.action === CrownAction.BEGIN && this.isEnlarged) {
            if (this.meter !== INVALID_TIMEOUT_ID) {
                clearTimeout(this.meter);
                this.meter = INVALID_TIMEOUT_ID;
            }
            this.crownDeltaAngle = this.getUIContext().px2vp(-event.degree *
            this.calcDisplayControlRatio(this.options.digitalCrownSensitivity)) / this.radius;
            this.calcCrownValue(this.crownDeltaAngle);
        }
        else if ((this.isEnlarged) && (this.isTouchAnimatorFinished) && (event.action === CrownAction.UPDATE)) {
            if (this.meter !== INVALID_TIMEOUT_ID) {
                clearTimeout(this.meter);
                this.meter = INVALID_TIMEOUT_ID;
            }
            this.crownDeltaAngle = this.getUIContext().px2vp(-event.degree *
            this.calcDisplayControlRatio(this.options.digitalCrownSensitivity)) / this.radius;
            this.calcCrownValue(this.crownDeltaAngle);
        }
        else if (this.isEnlarged && event.action === CrownAction.END) {
            if (this.meter !== INVALID_TIMEOUT_ID) {
                clearTimeout(this.meter);
                this.meter = INVALID_TIMEOUT_ID;
            }
            this.meter = setTimeout(() => {
                if (this.isEnlarged) {
                    this.isTouchAnimatorFinished = false;
                    this.isEnlarged = false;
                    this.startRestoreAnimator();
                    this.calcBlur();
                }
            }, RESTORE_TIMEOUT);
        }
    }

    initialRender() {
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Column.create();
            Column.hitTestBehavior(this.isFocus ? HitTestMode.Default : HitTestMode.Transparent);
        }, Column);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Stack.create();
        }, Stack);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Column.create();
            Column.clipShape(new Path({ commands: this.clipPath }));
        }, Column);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Circle.create({ width: TOTAL_LENGTH, height: TOTAL_LENGTH });
            Circle.width(this.totalWidth);
            Circle.height(this.totalHeight);
            Circle.fill(BLUR_COLOR_DEFAULT);
            Circle.backdropBlur(this.options.styleOptions.trackBlur);
        }, Circle);
        Column.pop();
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Button.createWithLabel();
            Button.stateEffect(false);
            Button.backgroundColor(BLUR_COLOR_DEFAULT);
            Button.drawModifier(this.fullModifier);
            Button.width(this.totalWidth);
            Button.height(this.totalHeight);
            Button.onTouch((event) => {
                if (event) {
                    this.onTouchEvent(event);
                }
            });
            Button.focusable(true);
            Button.focusOnTouch(true);
            Button.onDigitalCrown((event) => {
                this.onDigitalCrownEvent(event);
                this.options.onChange?.(this.options.valueOptions.progress);
            });
        }, Button);
        Button.pop();
        Stack.pop();
        Column.pop();
    }

    updateStateVars(params) {
        if (params === undefined) {
            return;
        }
        if ("options" in params) {
            this.updateParam("options", params.options);
        }
    }

    rerender() {
        this.updateDirtyElements();
    }
}
__decorate([
    Param
], ArcSlider.prototype, "options", void 0);
__decorate([
    Local
], ArcSlider.prototype, "lineWidth", void 0);
__decorate([
    Local
], ArcSlider.prototype, "radius", void 0);
__decorate([
    Local
], ArcSlider.prototype, "trackStartAngle", void 0);
__decorate([
    Local
], ArcSlider.prototype, "trackEndAngle", void 0);
__decorate([
    Local
], ArcSlider.prototype, "selectedStartAngle", void 0);
__decorate([
    Local
], ArcSlider.prototype, "selectedEndAngle", void 0);
__decorate([
    Local
], ArcSlider.prototype, "selectRatioNow", void 0);
__decorate([
    Local
], ArcSlider.prototype, "isEnlarged", void 0);
__decorate([
    Local
], ArcSlider.prototype, "clipPath", void 0);
__decorate([
    Local
], ArcSlider.prototype, "isReverse", void 0);
__decorate([
    Local
], ArcSlider.prototype, "isLargeArc", void 0);
__decorate([
    Monitor('trackStartAngle', 'trackEndAngle', 'selectedStartAngle', 'selectedEndAngle', 'options.valueOptions.min', 'options.valueOptions.max', 'options.valueOptions.progress', 'options.layoutOptions.reverse', 'options.styleOptions.trackThickness', 'options.styleOptions.activeTrackThickness', 'options.styleOptions.trackColor', 'options.styleOptions.selectedColor', 'options.styleOptions.trackBlur', 'clipPath', 'isFocus')
], ArcSlider.prototype, "onChange", null);


export default {
    ArcSlider,
    ArcSliderOptions,
    ArcSliderValueOptions,
    ArcSliderLayoutOptions,
    ArcSliderStyleOptions
};