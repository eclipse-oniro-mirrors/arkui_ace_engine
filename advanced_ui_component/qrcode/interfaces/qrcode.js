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
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
if (!("finalizeConstruction" in ViewPU.prototype)) {
    Reflect.set(ViewPU.prototype, "finalizeConstruction", () => { });
}
const LengthMetrics = requireNapi('arkui.node').LengthMetrics;
const ColorMetrics = requireNapi('arkui.node').ColorMetrics;
const LengthUnit = requireNapi('arkui.node').LengthUnit;
export var QrcodeState;
(function (QrcodeState) {
    QrcodeState[QrcodeState["NORMAL"] = 0] = "NORMAL";
    QrcodeState[QrcodeState["EXPIRED"] = 1] = "EXPIRED";
    QrcodeState[QrcodeState["LOADING"] = 2] = "LOADING";
})(QrcodeState || (QrcodeState = {}));
class ConfigDataConstants {
}
ConfigDataConstants.TIPS = { "id": -1, "type": 10003, params: ['sys.string.qrcode_expiration_prompt'], "bundleName": "__harDefaultBundleName__", "moduleName": "__harDefaultModuleName__" };
ConfigDataConstants.ANIMATION_CURVE = '0.33,0,0.67,1';
ConfigDataConstants.BOTTOM_CONTAINER_LENGTH = 125;
ConfigDataConstants.QRCODE_LENGTH = 101;
ConfigDataConstants.REFRESH_ICON_SIZE = 28;
ConfigDataConstants.LOAD_ICON_SIZE = 34;
ConfigDataConstants.BOTTOM_ROUNDED_CORNERS = 12;
ConfigDataConstants.TEXT_SIZE = '11fp';
ConfigDataConstants.SPACE_SIZE = '6vp';
ConfigDataConstants.WORD_WEIGHT = 500;
ConfigDataConstants.DEFAULT_OPACITY = 0.4;
ConfigDataConstants.DEFAULT_TIMES = 200;
let QrcodeOptions = class QrcodeOptions {
    constructor(options) {
        this.edgeLength = options.edgeLength ?? LengthMetrics.vp(ConfigDataConstants.BOTTOM_CONTAINER_LENGTH);
        this.color = options.color ?? ColorMetrics.resourceColor(Color.Black);
        this.backgroundColor = options.backgroundColor ?? ColorMetrics.resourceColor(Color.White);
        this.textColor = options.textColor ?? ColorMetrics.resourceColor(Color.White);
        this.textContent = options.textContent ?? ConfigDataConstants.TIPS;
        this.borderRadius = options.borderRadius ?? LengthMetrics.vp(ConfigDataConstants.BOTTOM_ROUNDED_CORNERS);
        this.opacity = options.opacity ?? ConfigDataConstants.DEFAULT_OPACITY;
        this.animationCurve = options.animationCurve ?? ConfigDataConstants.ANIMATION_CURVE;
        this.animationDuration = options.animationDuration ?? ConfigDataConstants.DEFAULT_TIMES;
        this.qrcodeSideLength = options.qrcodeSideLength ?? LengthMetrics.vp(ConfigDataConstants.QRCODE_LENGTH);
        this.expiredImageLength = options.expiredImageLength ?? LengthMetrics.vp(ConfigDataConstants.REFRESH_ICON_SIZE);
        this.textSize = options.textSize ?? ConfigDataConstants.TEXT_SIZE;
        this.textWeight = options.textWeight ?? FontWeight.Medium;
        this.space = options.space ?? ConfigDataConstants.SPACE_SIZE;
        this.loadingImageLength = options.loadingImageLength ?? LengthMetrics.vp(ConfigDataConstants.LOAD_ICON_SIZE);
    }
};
__decorate([
    Trace
], QrcodeOptions.prototype, "edgeLength", void 0);
__decorate([
    Trace
], QrcodeOptions.prototype, "color", void 0);
__decorate([
    Trace
], QrcodeOptions.prototype, "backgroundColor", void 0);
__decorate([
    Trace
], QrcodeOptions.prototype, "textColor", void 0);
__decorate([
    Trace
], QrcodeOptions.prototype, "textContent", void 0);
__decorate([
    Trace
], QrcodeOptions.prototype, "borderRadius", void 0);
__decorate([
    Trace
], QrcodeOptions.prototype, "opacity", void 0);
__decorate([
    Trace
], QrcodeOptions.prototype, "animationCurve", void 0);
__decorate([
    Trace
], QrcodeOptions.prototype, "animationDuration", void 0);
__decorate([
    Trace
], QrcodeOptions.prototype, "qrcodeSideLength", void 0);
__decorate([
    Trace
], QrcodeOptions.prototype, "expiredImageLength", void 0);
__decorate([
    Trace
], QrcodeOptions.prototype, "textSize", void 0);
__decorate([
    Trace
], QrcodeOptions.prototype, "textWeight", void 0);
__decorate([
    Trace
], QrcodeOptions.prototype, "space", void 0);
__decorate([
    Trace
], QrcodeOptions.prototype, "loadingImageLength", void 0);
QrcodeOptions = __decorate([
    ObservedV2
], QrcodeOptions);
export { QrcodeOptions };
function cover(params) {
    switch (params.unit) {
        case LengthUnit.VP:
            return `${params.value}vp`;
        case LengthUnit.PX:
            return `${params.value}px`;
        case LengthUnit.FP:
            return `${params.value}fp`;
        case LengthUnit.LPX:
            return `${params.value}lpx`;
        case LengthUnit.PERCENT:
            return `${params.value}%`;
    }
}
export class Qrcode extends ViewV2 {
    constructor(parent, params, __localStorage, elmtId = -1, paramsLambda, extraInfo) {
        super(parent, elmtId, extraInfo);
        this.initParam("options", (params && "options" in params) ? params.options : undefined);
        this.initParam("value", (params && "value" in params) ? params.value : undefined);
        this.initParam("state", (params && "state" in params) ? params.state : undefined);
        this.finalizeConstruction();
    }
    symbolGlyphBuilder(parent = null) {
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            SymbolGlyph.create({ "id": -1, "type": 40000, params: ['sys.symbol.arrow_clockwise'], "bundleName": "__harDefaultBundleName__", "moduleName": "__harDefaultModuleName__" });
            SymbolGlyph.fontSize(this.options.expiredImageLength ? cover(this.options.expiredImageLength) : undefined);
            SymbolGlyph.fontColor([this.options.textColor?.color]);
            SymbolGlyph.visibility(this.state == QrcodeState.EXPIRED ? Visibility.Visible : Visibility.None);
        }, SymbolGlyph);
    }
    textBuilder(parent = null) {
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Text.create(this.options.textContent);
            Text.margin({ top: this.options.space });
            Text.fontColor(this.options.textColor?.color);
            Text.fontSize(this.options.textSize);
            Text.fontWeight(this.options.textWeight);
            Text.textAlign(TextAlign.Center);
            Text.width(this.options.qrcodeSideLength?.value);
            Text.visibility(this.state == QrcodeState.EXPIRED ? Visibility.Visible : Visibility.None);
        }, Text);
        Text.pop();
    }
    loadinProgressBuilder(parent = null) {
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            LoadingProgress.create();
            Context.animation({ duration: this.options.animationDuration, curve: this.options.animationCurve,
                playMode: PlayMode.Normal
            });
            LoadingProgress.color(Color.White);
            LoadingProgress.height(this.options.loadingImageLength?.value);
            LoadingProgress.width(this.options.loadingImageLength?.value);
            LoadingProgress.visibility(this.state == QrcodeState.LOADING ? Visibility.Visible : Visibility.None);
            Context.animation(null);
        }, LoadingProgress);
    }
    initialRender() {
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            Stack.create();
        }, Stack);
        this.observeComponentCreation2((elmtId, isInitialRender) => {
            If.create();
            if (this.options !== void 0) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                        Column.create();
                        Context.animation({ duration: this.options.animationDuration, curve: this.options.animationCurve,
                            playMode: PlayMode.Normal
                        });
                        Column.height(this.options.edgeLength?.value);
                        Column.width(this.options.edgeLength?.value);
                        Column.backgroundColor(this.options.backgroundColor?.color);
                        Column.borderRadius(this.options.borderRadius?.value);
                        Column.opacity(this.state != QrcodeState.NORMAL ? this.options.opacity : 1);
                        Context.animation(null);
                        Column.justifyContent(FlexAlign.Center);
                    }, Column);
                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                        QRCode.create(this.value);
                        QRCode.color(this.options.color?.color);
                        QRCode.backgroundColor(Color.Transparent);
                        QRCode.width(this.options.qrcodeSideLength?.value);
                        QRCode.height(this.options.qrcodeSideLength ? cover(this.options.qrcodeSideLength) : undefined);
                    }, QRCode);
                    QRCode.pop();
                    Column.pop();
                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                        Column.create();
                        Context.animation({ duration: this.options.animationDuration, curve: this.options.animationCurve,
                            playMode: PlayMode.Normal
                        });
                        Column.visibility(this.state == QrcodeState.NORMAL ? Visibility.None : Visibility.Visible);
                        Context.animation(null);
                    }, Column);
                    this.symbolGlyphBuilder.bind(this)(this);
                    this.observeComponentCreation2((elmtId, isInitialRender) => {
                        If.create();
                        if (this.state == QrcodeState.LOADING) {
                            this.ifElseBranchUpdateFunction(0, () => {
                                this.loadinProgressBuilder.bind(this)(this);
                            });
                        }
                        else {
                            this.ifElseBranchUpdateFunction(1, () => {
                            });
                        }
                    }, If);
                    If.pop();
                    this.textBuilder.bind(this)(this);
                    Column.pop();
                });
            }
            else {
                this.ifElseBranchUpdateFunction(1, () => {
                });
            }
        }, If);
        If.pop();
        Stack.pop();
    }
    updateStateVars(params) {
        if (params === undefined) {
            return;
        }
        if ("options" in params) {
            this.updateParam("options", params.options);
        }
        if ("value" in params) {
            this.updateParam("value", params.value);
        }
        if ("state" in params) {
            this.updateParam("state", params.state);
        }
    }
    rerender() {
        this.updateDirtyElements();
    }
}
__decorate([
    Param
], Qrcode.prototype, "options", void 0);
__decorate([
    Param
], Qrcode.prototype, "value", void 0);
__decorate([
    Param
], Qrcode.prototype, "state", void 0);
//# sourceMappingURL=Qrcode.js.map
export default {Qrcode, QrcodeOptions, QrcodeState}