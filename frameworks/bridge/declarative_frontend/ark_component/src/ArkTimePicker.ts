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
class ArkTimePickerComponent extends ArkComponent implements TimePickerAttribute {
  constructor(nativePtr: KNode, classType?: ModifierType) {
    super(nativePtr, classType);
  }
  loop(value: boolean): this {
    modifierWithKey(this._modifiersWithKeys, TimepickerLoopModifier.identity, TimepickerLoopModifier, value);
    return this;
  }
  digitalCrownSensitivity(value: Optional<CrownSensitivity>): this {
    modifierWithKey(this._modifiersWithKeys, TimepickerDigitalCrownSensitivityModifier.identity, TimepickerDigitalCrownSensitivityModifier, value);
    return this;
  }
  useMilitaryTime(value: boolean): this {
    modifierWithKey(this._modifiersWithKeys, TimepickerUseMilitaryTimeModifier.identity,
      TimepickerUseMilitaryTimeModifier, value);
    return this;
  }
  disappearTextStyle(value: PickerTextStyle): this {
    modifierWithKey(this._modifiersWithKeys, TimepickerDisappearTextStyleModifier.identity,
      TimepickerDisappearTextStyleModifier, value);
    return this;
  }
  textStyle(value: PickerTextStyle): this {
    modifierWithKey(this._modifiersWithKeys, TimepickerTextStyleModifier.identity,
      TimepickerTextStyleModifier, value);
    return this;
  }
  selectedTextStyle(value: PickerTextStyle): this {
    modifierWithKey(this._modifiersWithKeys, TimepickerSelectedTextStyleModifier.identity,
      TimepickerSelectedTextStyleModifier, value);
    return this;
  }
  onChange(callback: (value: TimePickerResult) => void): this {
    modifierWithKey(this._modifiersWithKeys, TimepickerOnChangeModifier.identity,
      TimepickerOnChangeModifier, callback);
    return this;
  }
  dateTimeOptions(value: DateTimeOptions): this {
    modifierWithKey(this._modifiersWithKeys, TimepickerDateTimeOptionsModifier.identity,
      TimepickerDateTimeOptionsModifier, value);
    return this;
  }
  enableHapticFeedback(value: boolean): this {
    modifierWithKey(this._modifiersWithKeys, TimepickerEnableHapticFeedbackModifier.identity, TimepickerEnableHapticFeedbackModifier, value);
    return this;
  }
  enableCascade(value: boolean): this {
    modifierWithKey(this._modifiersWithKeys, TimepickerEnableCascadeModifier.identity,
      TimepickerEnableCascadeModifier, value);
    return this;
  }
}

class TimepickerTextStyleModifier extends ModifierWithKey<PickerTextStyle> {
  constructor(value: PickerTextStyle) {
    super(value);
  }
  static identity: Symbol = Symbol('textStyle');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().timepicker.resetTextStyle(node);
    } else {
      getUINativeModule().timepicker.setTextStyle(node, this.value?.color ?? undefined,
        this.value?.font?.size ?? undefined,
        this.value?.font?.weight ?? undefined,
        this.value?.font?.family ?? undefined,
        this.value?.font?.style ?? undefined);
    }
  }

  checkObjectDiff(): boolean {
    return true;
  }
}

class TimepickerSelectedTextStyleModifier extends ModifierWithKey<PickerTextStyle> {
  constructor(value: PickerTextStyle) {
    super(value);
  }
  static identity: Symbol = Symbol('selectedTextStyle');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().timepicker.resetSelectedTextStyle(node);
    } else {
      getUINativeModule().timepicker.setSelectedTextStyle(node, this.value?.color ?? undefined,
        this.value?.font?.size ?? undefined,
        this.value?.font?.weight ?? undefined,
        this.value?.font?.family ?? undefined,
        this.value?.font?.style ?? undefined);
    }
  }

  checkObjectDiff(): boolean {
    return true;
  }
}

class TimepickerDisappearTextStyleModifier extends ModifierWithKey<PickerTextStyle> {
  constructor(value: PickerTextStyle) {
    super(value);
  }
  static identity: Symbol = Symbol('disappearTextStyle');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().timepicker.resetDisappearTextStyle(node);
    } else {
      getUINativeModule().timepicker.setDisappearTextStyle(node, this.value?.color ?? undefined,
        this.value?.font?.size ?? undefined,
        this.value?.font?.weight ?? undefined,
        this.value?.font?.family ?? undefined,
        this.value?.font?.style ?? undefined);
    }
  }

  checkObjectDiff(): boolean {
    return true;
  }
}

class TimepickerUseMilitaryTimeModifier extends ModifierWithKey<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('timepickerUseMilitaryTime');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().timepicker.resetTimepickerUseMilitaryTime(node);
    } else {
      getUINativeModule().timepicker.setTimepickerUseMilitaryTime(node, this.value);
    }
  }
}

class TimepickerLoopModifier extends ModifierWithKey<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('timepickerLoop');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().timepicker.resetTimepickerLoop(node);
    } else {
      getUINativeModule().timepicker.setTimepickerLoop(node, this.value);
    }
  }
}

class TimepickerDigitalCrownSensitivityModifier extends ModifierWithKey<Optional<CrownSensitivity>> {
  constructor(value: Optional<CrownSensitivity>) {
    super(value);
  }
  static identity: Symbol = Symbol('timepickerDigitalCrownSensitivity');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().timepicker.resetTimepickerDigitalCrownSensitivity(node);
    } else {
      getUINativeModule().timepicker.setTimepickerDigitalCrownSensitivity(node, this.value);
    }
  }
}

class TimepickerDateTimeOptionsModifier extends ModifierWithKey<DateTimeOptions> {
  constructor(value: DateTimeOptions) {
    super(value);
  }
  static identity: Symbol = Symbol('timepickerDateTimeOptions');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().timepicker.resetTimepickerDateTimeOptions(node);
    } else {
      getUINativeModule().timepicker.setTimepickerDateTimeOptions(node, this.value.hour, this.value.minute, this.value.second);
    }
  }
}

class TimepickerEnableHapticFeedbackModifier extends ModifierWithKey<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('timepickerEnableHapticFeedback');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().timepicker.resetTimepickerEnableHapticFeedback(node);
    } else {
      getUINativeModule().timepicker.setTimepickerEnableHapticFeedback(node, this.value);
    }
  }
}

class TimepickerEnableCascadeModifier extends ModifierWithKey<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity: Symbol = Symbol('timepickerEnableCascade');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().timepicker.resetTimepickerEnableCascade(node);
    } else {
      getUINativeModule().timepicker.setTimepickerEnableCascade(node, this.value);
    }
  }
}

class TimepickerOnChangeModifier extends ModifierWithKey<(value:TimePickerResult)=> void>
{
  constructor(value: (value: TimePickerResult) => void) {
    super(value);
  }
  static identity: Symbol = Symbol('timePickerOnChange');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().timepicker.resetTimepickerOnChange(node);
    } else {
      getUINativeModule().timepicker.setTimepickerOnChange(node, this.value);
    }
  }
}
// @ts-ignore
globalThis.TimePicker.attributeModifier = function (modifier: ArkComponent): void {
  attributeModifierFunc.call(this, modifier, (nativePtr: KNode) => {
    return new ArkTimePickerComponent(nativePtr);
  }, (nativePtr: KNode, classType: ModifierType, modifierJS: ModifierJS) => {
    return new modifierJS.TimePickerModifier(nativePtr, classType);
  });
};
