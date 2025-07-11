/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

/// <reference path='./import.ts' />
class ArkTextPickerComponent extends ArkComponent implements TextPickerAttribute {
  constructor(nativePtr: KNode, classType?: ModifierType) {
    super(nativePtr, classType);
  }
  defaultPickerItemHeight(value: string | number): this {
    modifierWithKey(
      this._modifiersWithKeys, TextpickerDefaultPickerItemHeightModifier.identity, TextpickerDefaultPickerItemHeightModifier, value);
    return this;
  }
  canLoop(value: boolean): this {
    modifierWithKey(
      this._modifiersWithKeys, TextpickerCanLoopModifier.identity, TextpickerCanLoopModifier, value);
    return this;
  }
  disappearTextStyle(value: PickerTextStyle): this {
    modifierWithKey(
      this._modifiersWithKeys, TextpickerDisappearTextStyleModifier.identity, TextpickerDisappearTextStyleModifier, value);
    return this;
  }
  textStyle(value: PickerTextStyle): this {
    modifierWithKey(
      this._modifiersWithKeys, TextpickerTextStyleModifier.identity, TextpickerTextStyleModifier, value);
    return this;
  }
  selectedTextStyle(value: PickerTextStyle): this {
    modifierWithKey(
      this._modifiersWithKeys, TextpickerSelectedTextStyleModifier.identity, TextpickerSelectedTextStyleModifier, value);
    return this;
  }
  onAccept(callback: (value: string, index: number) => void): this {
    throw new Error('Method not implemented.');
  }
  onCancel(callback: () => void): this {
    throw new Error('Method not implemented.');
  }
  onChange(callback: Optional<OnTextPickerChangeCallback>): this {
    modifierWithKey(
      this._modifiersWithKeys, TextpickerOnChangeModifier.identity, TextpickerOnChangeModifier, callback);
    return this;
  }
  selectedIndex(value: number | number[]): this {
    modifierWithKey(
      this._modifiersWithKeys, TextpickerSelectedIndexModifier.identity, TextpickerSelectedIndexModifier, value);
    return this;
  }
  divider(value: DividerOptions | null): this {
    modifierWithKey(
      this._modifiersWithKeys, TextpickerDividerModifier.identity, TextpickerDividerModifier, value);
    return this;
  }
  gradientHeight(value: Dimension): this {
    modifierWithKey(
      this._modifiersWithKeys, TextpickerGradientHeightModifier.identity, TextpickerGradientHeightModifier, value);
    return this;
  }
  disableTextStyleAnimation(value: boolean): this {
    modifierWithKey(
      this._modifiersWithKeys, TextpickerDisableTextStyleAnimationModifier.identity, TextpickerDisableTextStyleAnimationModifier, value);
    return this;
  }
  defaultTextStyle(value: PickerTextStyle): this {
    modifierWithKey(
      this._modifiersWithKeys, TextpickerDefaultTextStyleModifier.identity, TextpickerDefaultTextStyleModifier, value);
    return this;
  }

  enableHapticFeedback(value: boolean): this {
    modifierWithKey(this._modifiersWithKeys, TextpickerEnableHapticFeedbackModifier.identity, TextpickerEnableHapticFeedbackModifier, value);
    return this;
  }

  selectedBackgroundStyle(value: PickerBackgroundStyle): this {
    modifierWithKey(
      this._modifiersWithKeys, TextpickerSelectedBackgroundStyleModifier.identity, TextpickerSelectedBackgroundStyleModifier, value);
    return this;
  }

  digitalCrownSensitivity(sensitivity: Optional<CrownSensitivity>): this {
    modifierWithKey(
      this._modifiersWithKeys, TextpickerDigitalCrownSensitivityModifier.identity, TextpickerDigitalCrownSensitivityModifier, value);
    return this;
  }
  onScrollStop(callback: (value: string | string[], index: number | number[]) => void) : this{
    modifierWithKey(
      this._modifiersWithKeys, TextpickerOnScrollStopModifier.identity, TextpickerOnScrollStopModifier, callback);
    return this;
  }
}

class TextpickerCanLoopModifier extends ModifierWithKey<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('textpickerCanLoop');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textpicker.resetCanLoop(node);
    } else {
      getUINativeModule().textpicker.setCanLoop(node, this.value);
    }
  }
}

class TextpickerDigitalCrownSensitivityModifier extends ModifierWithKey<Optional<CrownSensitivity>> {
  constructor(value: Optional<CrownSensitivity>) {
    super(value);
  }
  static identity: Symbol = Symbol('textpickerDigitalCrownSensitivity');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textpicker.resetDigitalCrownSensitivity(node);
    } else {
      getUINativeModule().textpicker.setDigitalCrownSensitivity(node, this.value);
    }
  }
}

class TextpickerSelectedIndexModifier extends ModifierWithKey<number | number[]> {
  constructor(value: number | number[]) {
    super(value);
  }
  static identity: Symbol = Symbol('textpickerSelectedIndex');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textpicker.resetSelectedIndex(node);
    } else {
      getUINativeModule().textpicker.setSelectedIndex(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    if (Array.isArray(this.stageValue) && Array.isArray(this.value)) {
      return !deepCompareArrays(this.stageValue, this.value);
    } else if (Array.isArray(this.stageValue) || Array.isArray(this.value)) {
      return true;
    } else {
      return this.stageValue !== this.value;
    }
  }

}

class TextpickerDividerModifier extends ModifierWithKey<DividerOptions | null> {
  constructor(value: DividerOptions | null) {
    super(value);
  }
  static identity: Symbol = Symbol('textpickerDivider');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textpicker.resetDivider(node, this.value);
    } else {
      getUINativeModule().textpicker.setDivider(node, this.value?.strokeWidth, this.value?.color, this.value?.startMargin, this.value?.endMargin);
    }
  }

  checkObjectDiff(): boolean {
    return !(this.stageValue?.strokeWidth === this.value?.strokeWidth &&
      this.stageValue?.color === this.value?.color &&
      this.stageValue?.startMargin === this.value?.startMargin &&
      this.stageValue?.endMargin === this.value?.endMargin);
  }
}

class TextpickerGradientHeightModifier extends ModifierWithKey<Dimension> {
  constructor(value: Dimension) {
    super(value);
  }
  static identity: Symbol = Symbol('textpickerGradientHeight');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textpicker.resetGradientHeight(node);
    } else {
      getUINativeModule().textpicker.setGradientHeight(node, this.value);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class TextpickerDisableTextStyleAnimationModifier extends ModifierWithKey<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('textpickerDisableTextStyleAnimation');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textpicker.resetDisableTextStyleAnimation(node);
    } else {
      getUINativeModule().textpicker.setDisableTextStyleAnimation(node, this.value);
    }
  }
}

class TextpickerDefaultTextStyleModifier extends ModifierWithKey<PickerTextStyle> {
  constructor(value: PickerTextStyle) {
    super(value);
  }
  static identity: Symbol = Symbol('textpickerDefaultTextStyle');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textpicker.resetDefaultTextStyle(node);
    } else {
      getUINativeModule().textpicker.setDefaultTextStyle(node, this.value?.color ?? undefined,
        this.value?.font?.size ?? undefined,
        this.value?.font?.weight ?? undefined,
        this.value?.font?.family ?? undefined,
        this.value?.font?.style ?? undefined,
        this.value?.minFontSize ?? undefined,
        this.value?.maxFontSize ?? undefined,
        this.value?.overflow ?? undefined);
    }
  }

  checkObjectDiff(): boolean {
    if (!(this.stageValue?.font?.weight === this.value?.font?.weight &&
      this.stageValue?.font?.style === this.value?.font?.style &&
      this.stageValue?.overflow === this.value?.overflow)) {
      return true;
    } else {
      return !isBaseOrResourceEqual(this.stageValue?.color, this.value?.color) ||
        !isBaseOrResourceEqual(this.stageValue?.font?.size, this.value?.font?.size) ||
        !isBaseOrResourceEqual(this.stageValue?.font?.family, this.value?.font?.family) ||
        !isBaseOrResourceEqual(this.stageValue?.minFontSize, this.value?.minFontSize) ||
        !isBaseOrResourceEqual(this.stageValue?.maxFontSize, this.value?.maxFontSize);
    }
  }
}

class TextpickerTextStyleModifier extends ModifierWithKey<PickerTextStyle> {
  constructor(value: PickerTextStyle) {
    super(value);
  }
  static identity: Symbol = Symbol('textpickerTextStyle');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textpicker.resetTextStyle(node);
    } else {
      getUINativeModule().textpicker.setTextStyle(node, this.value?.color ?? undefined,
        this.value?.font?.size ?? undefined,
        this.value?.font?.weight ?? undefined,
        this.value?.font?.family ?? undefined,
        this.value?.font?.style ?? undefined,
        this.value?.minFontSize ?? undefined,
        this.value?.maxFontSize ?? undefined,
        this.value?.overflow ?? undefined);
    }
  }

  checkObjectDiff(): boolean {
    if (!(this.stageValue?.font?.weight === this.value?.font?.weight &&
      this.stageValue?.font?.style === this.value?.font?.style &&
      this.stageValue?.overflow === this.value?.overflow)) {
      return true;
    } else {
      return !isBaseOrResourceEqual(this.stageValue?.color, this.value?.color) ||
        !isBaseOrResourceEqual(this.stageValue?.font?.size, this.value?.font?.size) ||
        !isBaseOrResourceEqual(this.stageValue?.font?.family, this.value?.font?.family) ||
        !isBaseOrResourceEqual(this.stageValue?.minFontSize, this.value?.minFontSize) ||
        !isBaseOrResourceEqual(this.stageValue?.maxFontSize, this.value?.maxFontSize);
    }
  }
}

class TextpickerSelectedTextStyleModifier extends ModifierWithKey<PickerTextStyle> {
  constructor(value: PickerTextStyle) {
    super(value);
  }
  static identity: Symbol = Symbol('textpickerSelectedTextStyle');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textpicker.resetSelectedTextStyle(node);
    } else {
      getUINativeModule().textpicker.setSelectedTextStyle(node, this.value?.color ?? undefined,
        this.value?.font?.size ?? undefined,
        this.value?.font?.weight ?? undefined,
        this.value?.font?.family ?? undefined,
        this.value?.font?.style ?? undefined,
        this.value?.minFontSize ?? undefined,
        this.value?.maxFontSize ?? undefined,
        this.value?.overflow ?? undefined);
    }
  }

  checkObjectDiff(): boolean {
    if (!(this.stageValue?.font?.weight === this.value?.font?.weight &&
      this.stageValue?.font?.style === this.value?.font?.style &&
      this.stageValue?.overflow === this.value?.overflow)) {
      return true;
    } else {
      return !isBaseOrResourceEqual(this.stageValue?.color, this.value?.color) ||
        !isBaseOrResourceEqual(this.stageValue?.font?.size, this.value?.font?.size) ||
        !isBaseOrResourceEqual(this.stageValue?.font?.family, this.value?.font?.family) ||
        !isBaseOrResourceEqual(this.stageValue?.minFontSize, this.value?.minFontSize) ||
        !isBaseOrResourceEqual(this.stageValue?.maxFontSize, this.value?.maxFontSize);
    }
  }
}

class TextpickerDisappearTextStyleModifier extends ModifierWithKey<PickerTextStyle> {
  constructor(value: PickerTextStyle) {
    super(value);
  }
  static identity: Symbol = Symbol('textpickerDisappearTextStyle');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textpicker.resetDisappearTextStyle(node);
    } else {
      getUINativeModule().textpicker.setDisappearTextStyle(node, this.value?.color ?? undefined,
        this.value?.font?.size ?? undefined,
        this.value?.font?.weight ?? undefined,
        this.value?.font?.family ?? undefined,
        this.value?.font?.style ?? undefined,
        this.value?.minFontSize ?? undefined,
        this.value?.maxFontSize ?? undefined,
        this.value?.overflow ?? undefined);
    }
  }

  checkObjectDiff(): boolean {
    if (!(this.stageValue?.font?.weight === this.value?.font?.weight &&
      this.stageValue?.font?.style === this.value?.font?.style &&
      this.stageValue?.overflow === this.value?.overflow)) {
      return true;
    } else {
      return !isBaseOrResourceEqual(this.stageValue?.color, this.value?.color) ||
        !isBaseOrResourceEqual(this.stageValue?.font?.size, this.value?.font?.size) ||
        !isBaseOrResourceEqual(this.stageValue?.font?.family, this.value?.font?.family) ||
        !isBaseOrResourceEqual(this.stageValue?.minFontSize, this.value?.minFontSize) ||
        !isBaseOrResourceEqual(this.stageValue?.maxFontSize, this.value?.maxFontSize);
    }
  }
}

class TextpickerDefaultPickerItemHeightModifier extends ModifierWithKey<number | string> {
  constructor(value: number | string) {
    super(value);
  }
  static identity: Symbol = Symbol('textpickerDefaultPickerItemHeight');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textpicker.resetDefaultPickerItemHeight(node);
    } else {
      getUINativeModule().textpicker.setDefaultPickerItemHeight(node, this.value);
    }
  }
}

class TextpickerEnableHapticFeedbackModifier extends ModifierWithKey<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('textpickerEnableHapticFeedback');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textpicker.resetTextpickerEnableHapticFeedback(node);
    } else {
      getUINativeModule().textpicker.setTextpickerEnableHapticFeedback(node, this.value!);
    }
  }
}

class TextpickerSelectedBackgroundStyleModifier extends ModifierWithKey<PickerBackgroundStyle> {
  constructor(value: PickerBackgroundStyle) {
    super(value);
  }
  static identity: Symbol = Symbol('textpickerSelectedBackgroundStyle');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textpicker.resetTextpickerSelectedBackgroundStyle(node);
    } else if (this.value == null) {
      getUINativeModule().textpicker.setTextpickerSelectedBackgroundStyle(node, undefined, undefined, undefined,
        undefined, undefined);
    } else {
      const { color, borderRadius } = this.value;
      if (borderRadius != null) {
        const borderRadiusKeys = Object.keys(borderRadius);
        let topLeft;
        let topRight;
        let bottomLeft;
        let bottomRight;
        if (borderRadiusKeys.indexOf('value') >= 0) {
          topLeft = topRight = bottomLeft = bottomRight = borderRadius;
        } else if (borderRadiusKeys.indexOf('topLeft') >= 0) {
          topLeft = (borderRadius as BorderRadiuses).topLeft;
          topRight = (borderRadius as BorderRadiuses).topRight;
          bottomLeft = (borderRadius as BorderRadiuses).bottomLeft;
          bottomRight = (borderRadius as BorderRadiuses).bottomRight;
        } else if (borderRadiusKeys.indexOf('topStart') >= 0) {
          topLeft = (borderRadius as LocalizedBorderRadiuses).topStart;
          topRight = (borderRadius as LocalizedBorderRadiuses).topEnd;
          bottomLeft = (borderRadius as LocalizedBorderRadiuses).bottomStart;
          bottomRight = (borderRadius as LocalizedBorderRadiuses).bottomEnd;
        }
        getUINativeModule().textpicker.setTextpickerSelectedBackgroundStyle(node, color, topLeft, topRight, bottomLeft,
          bottomRight);
      } else {
        getUINativeModule().textpicker.setTextpickerSelectedBackgroundStyle(node, color, undefined, undefined,
          undefined, undefined);
      }
    }
  }
  checkObjectDiff(): boolean {
    if (!(this.stageValue.color === this.value.color)) {
      return true;
    } else if (this.stageValue.borderRadius != null && this.value.borderRadius != null) {
      if (Object.keys(this.value.borderRadius).indexOf('value') >= 0) {
        return !(
          (this.stageValue.borderRadius as LengthMetrics).value === (this.value.borderRadius as LengthMetrics).value &&
          (this.stageValue.borderRadius as LengthMetrics).unit === (this.value.borderRadius as LengthMetrics).unit);
      } else if (Object.keys(this.value.borderRadius).indexOf('topLeft') >= 0) {
        return !(
          (this.stageValue.borderRadius as BorderRadiuses).topLeft ===
          (this.value.borderRadius as BorderRadiuses).topLeft &&
          (this.stageValue.borderRadius as BorderRadiuses).topRight ===
          (this.value.borderRadius as BorderRadiuses).topRight &&
          (this.stageValue.borderRadius as BorderRadiuses).bottomLeft ===
          (this.value.borderRadius as BorderRadiuses).bottomLeft &&
          (this.stageValue.borderRadius as BorderRadiuses).bottomRight ===
          (this.value.borderRadius as BorderRadiuses).bottomRight);
      } else if (Object.keys(this.value.borderRadius).indexOf('topStart') >= 0) {
        return !(
          (this.stageValue.borderRadius as LocalizedBorderRadiuses).topStart ===
          (this.value.borderRadius as LocalizedBorderRadiuses).topStart &&
          (this.stageValue.borderRadius as LocalizedBorderRadiuses).topEnd ===
          (this.value.borderRadius as LocalizedBorderRadiuses).topEnd &&
          (this.stageValue.borderRadius as LocalizedBorderRadiuses).bottomStart ===
          (this.value.borderRadius as LocalizedBorderRadiuses).bottomStart &&
          (this.stageValue.borderRadius as LocalizedBorderRadiuses).bottomEnd ===
          (this.value.borderRadius as LocalizedBorderRadiuses).bottomEnd);
      }
    }
    return true;
  }
}
class TextpickerOnChangeModifier extends ModifierWithKey<Optional<OnTextPickerChangeCallback>>{
  constructor(value: Optional<OnTextPickerChangeCallback>) {
    super(value);
  }
  static identity: Symbol = Symbol('textpickerOnChange');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textpicker.resetOnChange(node);
    } else {
      getUINativeModule().textpicker.setOnChange(node, this.value);
    }
  }
}
class TextpickerOnScrollStopModifier extends ModifierWithKey<(value: string | string[], index: number | number[]) => void>{
  constructor(value:(value: string | string[], index: number | number[]) => void){
     super(value);
  }
  static identity: Symbol = Symbol('textpickerOnScrollStop');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().textpicker.resetOnScrollStop(node);
    } else {
      getUINativeModule().textpicker.setOnScrollStop(node, this.value);
    }
  }
}
// @ts-ignore
globalThis.TextPicker.attributeModifier = function (modifier: ArkComponent): void {
  attributeModifierFunc.call(this, modifier, (nativePtr: KNode) => {
    return new ArkTextPickerComponent(nativePtr);
  }, (nativePtr: KNode, classType: ModifierType, modifierJS: ModifierJS) => {
    return new modifierJS.TextPickerModifier(nativePtr, classType);
  });
};
