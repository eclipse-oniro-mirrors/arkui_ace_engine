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

class SearchSelectionMenuHiddenModifier extends ModifierWithKey<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity = Symbol('searchSelectionMenuHidden');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetSelectionMenuHidden(node);
    } else {
      getUINativeModule().search.setSelectionMenuHidden(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return this.stageValue !== this.value;
  }
}

class SearchCaretStyleModifier extends ModifierWithKey<CaretStyle> {
  constructor(value: CaretStyle) {
    super(value);
  }
  static identity = Symbol('searchCaretStyle');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetCaretStyle(node);
    } else {
      getUINativeModule().search.setCaretStyle(node, this.value.width,
        this.value.color);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue.width, this.value.width) ||
      !isBaseOrResourceEqual(this.stageValue.color, this.value.color);
  }
}

class SearchEnableKeyboardOnFocusModifier extends ModifierWithKey<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity = Symbol('searchEnableKeyboardOnFocus');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetEnableKeyboardOnFocus(node);
    } else {
      getUINativeModule().search.setEnableKeyboardOnFocus(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return this.stageValue !== this.value;
  }
}

class SearchSearchIconModifier extends ModifierWithKey<IconOptions> {
  constructor(value: IconOptions) {
    super(value);
  }
  static identity = Symbol('searchSearchIcon');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetSearchIcon(node);
    } else {
      getUINativeModule().search.setSearchIcon(node, this.value.size,
        this.value.color, this.value.src);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue.size, this.value.size) ||
      !isBaseOrResourceEqual(this.stageValue.color, this.value.color) ||
      !isBaseOrResourceEqual(this.stageValue.src, this.value.src);
  }
}

class SearchPlaceholderFontModifier extends ModifierWithKey<Font> {
  constructor(value: Font) {
    super(value);
  }
  static identity = Symbol('searchPlaceholderFont');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetPlaceholderFont(node);
    } else {
      getUINativeModule().search.setPlaceholderFont(node, this.value.size,
        this.value.weight, this.value.family, this.value.style);
    }
  }
  checkObjectDiff(): boolean {
    return this.stageValue.weight !== this.value.weight ||
      this.stageValue.style !== this.value.style ||
      !isBaseOrResourceEqual(this.stageValue.size, this.value.size) ||
      !isBaseOrResourceEqual(this.stageValue.family, this.value.family);
  }
}

class SearchSearchButtonModifier extends ModifierWithKey<ArkSearchButton> {
  constructor(value: ArkSearchButton) {
    super(value);
  }
  static identity = Symbol('searchSearchButton');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetSearchButton(node);
    } else {
      getUINativeModule().search.setSearchButton(node, this.value.value,
        this.value.fontSize, this.value.fontColor);
    }
  }
  checkObjectDiff(): boolean {
    return this.stageValue.value !== this.value.value ||
      !isBaseOrResourceEqual(this.stageValue.fontSize, this.value.fontSize) ||
      !isBaseOrResourceEqual(this.stageValue.fontColor, this.value.fontColor);
  }
}

class SearchFontColorModifier extends ModifierWithKey<ResourceColor> {
  constructor(value: ResourceColor) {
    super(value);
  }
  static identity = Symbol('searchFontColor');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetFontColor(node);
    } else {
      getUINativeModule().search.setFontColor(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class SearchCopyOptionModifier extends ModifierWithKey<CopyOptions> {
  constructor(value: CopyOptions) {
    super(value);
  }
  static identity = Symbol('searchCopyOption');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetCopyOption(node);
    } else {
      getUINativeModule().search.setCopyOption(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return this.stageValue !== this.value;
  }
}

class SearchTextFontModifier extends ModifierWithKey<Font> {
  constructor(value: Font) {
    super(value);
  }
  static identity = Symbol('searchTextFont');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetTextFont(node);
    } else {
      getUINativeModule().search.setTextFont(node, this.value.size, this.value.weight, this.value.family, this.value.style);
    }
  }
  checkObjectDiff(): boolean {
    return this.stageValue.weight !== this.value.weight ||
      this.stageValue.style !== this.value.style ||
      !isBaseOrResourceEqual(this.stageValue.size, this.value.size) ||
      !isBaseOrResourceEqual(this.stageValue.family, this.value.family);
  }
}

class SearchPlaceholderColorModifier extends ModifierWithKey<ResourceColor> {
  constructor(value: ResourceColor) {
    super(value);
  }
  static identity = Symbol('searchPlaceholderColor');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetPlaceholderColor(node);
    } else {
      getUINativeModule().search.setPlaceholderColor(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class SearchCancelButtonModifier extends ModifierWithKey<{ style?: CancelButtonStyle, icon?: IconOptions }> {
  constructor(value: { style?: CancelButtonStyle, icon?: IconOptions }) {
    super(value);
  }
  static identity = Symbol('searchCancelButton');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetCancelButton(node);
    } else {
      getUINativeModule().search.setCancelButton(node, this.value.style,
        this.value.icon?.size, this.value.icon?.color, this.value.icon?.src);
    }
  }
  checkObjectDiff(): boolean {
    return this.stageValue.style !== this.value.style ||
      !isBaseOrResourceEqual(this.stageValue.icon?.size, this.value.icon?.size) ||
      !isBaseOrResourceEqual(this.stageValue.icon?.color, this.value.icon?.color) ||
      !isBaseOrResourceEqual(this.stageValue.icon?.src, this.value.icon?.src);
  }
}

class SearchTextAlignModifier extends ModifierWithKey<TextAlign> {
  constructor(value: TextAlign) {
    super(value);
  }
  static identity = Symbol('searchTextAlign');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetTextAlign(node);
    } else {
      getUINativeModule().search.setTextAlign(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return this.stageValue !== this.value;
  }
}

class SearchEnterKeyTypeModifier extends ModifierWithKey<number> {
  constructor(value: number) {
    super(value);
  }
  static identity: Symbol = Symbol('searchEnterKeyType');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetSearchEnterKeyType(node);
    } else {
      getUINativeModule().search.setSearchEnterKeyType(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class SearchHeightModifier extends ModifierWithKey<Length> {
  constructor(value: Length) {
    super(value);
  }
  static identity: Symbol = Symbol('searchHeight');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetSearchHeight(node);
    } else {
      getUINativeModule().search.setSearchHeight(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class SearchIdModifier extends ModifierWithKey<string> {
  constructor(value: string) {
    super(value);
  }
  static identity: Symbol = Symbol('searchId');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetSearchInspectorId(node);
    } else {
      getUINativeModule().search.setSearchInspectorId(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class SearchDecorationModifier extends ModifierWithKey<{ type: TextDecorationType; color?: ResourceColor }> {
  constructor(value: { type: TextDecorationType; color?: ResourceColor }) {
    super(value);
  }
  static identity: Symbol = Symbol('searchDecoration');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetDecoration(node);
    } else {
      getUINativeModule().search.setDecoration(node, this.value!.type, this.value!.color);
    }
  }

  checkObjectDiff(): boolean {
    if (this.stageValue.type !== this.value.type) {
      return true;
    }
    if (isResource(this.stageValue.color) && isResource(this.value.color)) {
      return !isResourceEqual(this.stageValue.color, this.value.color);
    } else if (!isResource(this.stageValue.color) && !isResource(this.value.color)) {
      return !(this.stageValue.color === this.value.color);
    } else {
      return true;
    }
  }
}

class SearchLetterSpacingModifier extends ModifierWithKey<number | string> {
  constructor(value: number | string) {
    super(value);
  }
  static identity: Symbol = Symbol('searchLetterSpacing');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetLetterSpacing(node);
    } else {
      getUINativeModule().search.setLetterSpacing(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}
class SearchMinFontSizeModifier extends ModifierWithKey<number | string | Resource> {
  constructor(value: number | string | Resource) {
    super(value);
  }
  static identity: Symbol = Symbol('searchMinFontSize');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetSearchMinFontSize(node);
    } else {
      getUINativeModule().search.setSearchMinFontSize(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class SearchLineHeightModifier extends ModifierWithKey<number | string | Resource> {
  constructor(value: number | string | Resource) {
    super(value);
  }
  static identity: Symbol = Symbol('searchLineHeight');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetLineHeight(node);
    } else {
      getUINativeModule().search.setLineHeight(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}
class SearchMaxFontSizeModifier extends ModifierWithKey<number | string | Resource> {
  constructor(value: number | string | Resource) {
    super(value);
  }
  static identity: Symbol = Symbol('searchMaxFontSize');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetSearchMaxFontSize(node);
    } else {
      getUINativeModule().search.setSearchMaxFontSize(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class SearchFontFeatureModifier extends ModifierWithKey<FontFeature> {
  constructor(value: FontFeature) {
    super(value);
  }
  static identity: Symbol = Symbol('searchFontFeature');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().search.resetFontFeature(node);
    } else {
      getUINativeModule().search.setFontFeature(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class ArkSearchComponent extends ArkComponent implements CommonMethod<SearchAttribute> {
  constructor(nativePtr: KNode, classType?: ModifierType) {
    super(nativePtr, classType);
  }
  onEditChange(callback: (isEditing: boolean) => void): SearchAttribute {
    throw new Error('Method not implemented.');
  }
  type(value: SearchType): SearchAttribute {
    throw new Error('Method not implemented.');
  }
  maxLength(value: number): SearchAttribute {
    throw new Error('Method not implemented.');
  }
  onEditChanged(callback: (isEditing: boolean) => void): SearchAttribute {
    throw new Error('Method not implemented.');
  }
  customKeyboard(event: () => void): SearchAttribute {
    throw new Error('Method not implemented.');
  }
  showUnit(event: () => void): SearchAttribute {
    throw new Error('Method not implemented.');
  }
  onContentScroll(callback: (totalOffsetX: number, totalOffsetY: number) => void): SearchAttribute {
    throw new Error('Method not implemented.');
  }
  onChange(callback: (value: string) => void): SearchAttribute {
    throw new Error('Method not implemented.');
  }
  onTextSelectionChange(callback: (selectionStart: number, selectionEnd: number) => void): SearchAttribute {
    throw new Error('Method not implemented.');
  }
  onCopy(callback: (value: string) => void): SearchAttribute {
    throw new Error('Method not implemented.');
  }
  onCut(callback: (value: string) => void): SearchAttribute {
    throw new Error('Method not implemented.');
  }
  onSubmit(callback: (value: string) => void): SearchAttribute {
    throw new Error('Method not implemented.');
  }
  onPaste(callback: (value: string) => void): SearchAttribute {
    throw new Error('Method not implemented.');
  }
  showCounter(value: boolean): SearchAttribute {
    throw new Error('Method not implemented.');
  }
  searchButton(value: string, option?: SearchButtonOptions): SearchAttribute {
    let searchButton = new ArkSearchButton();
    searchButton.value = value;
    searchButton.fontColor = option?.fontColor;
    searchButton.fontSize = option?.fontSize;
    modifierWithKey(this._modifiersWithKeys, SearchSearchButtonModifier.identity, SearchSearchButtonModifier, searchButton);
    return this;
  }
  selectionMenuHidden(value: boolean): SearchAttribute {
    modifierWithKey(this._modifiersWithKeys, SearchSelectionMenuHiddenModifier.identity, SearchSelectionMenuHiddenModifier, value);
    return this;
  }
  enableKeyboardOnFocus(value: boolean): SearchAttribute {
    modifierWithKey(this._modifiersWithKeys, SearchEnableKeyboardOnFocusModifier.identity, SearchEnableKeyboardOnFocusModifier, value);
    return this;
  }
  caretStyle(value: CaretStyle): SearchAttribute {
    modifierWithKey(this._modifiersWithKeys, SearchCaretStyleModifier.identity, SearchCaretStyleModifier, value);
    return this;
  }
  cancelButton(value: { style?: CancelButtonStyle, icon?: IconOptions }): SearchAttribute {
    modifierWithKey(this._modifiersWithKeys, SearchCancelButtonModifier.identity, SearchCancelButtonModifier, value);
    return this;
  }
  searchIcon(value: IconOptions): SearchAttribute {
    modifierWithKey(this._modifiersWithKeys, SearchSearchIconModifier.identity, SearchSearchIconModifier, value);
    return this;
  }
  fontColor(value: ResourceColor): SearchAttribute {
    modifierWithKey(this._modifiersWithKeys, SearchFontColorModifier.identity, SearchFontColorModifier, value);
    return this;
  }
  placeholderColor(value: ResourceColor): SearchAttribute {
    modifierWithKey(this._modifiersWithKeys, SearchPlaceholderColorModifier.identity, SearchPlaceholderColorModifier, value);
    return this;
  }
  placeholderFont(value?: Font): SearchAttribute {
    modifierWithKey(this._modifiersWithKeys, SearchPlaceholderFontModifier.identity, SearchPlaceholderFontModifier, value);
    return this;
  }
  textFont(value?: Font): SearchAttribute {
    modifierWithKey(this._modifiersWithKeys, SearchTextFontModifier.identity, SearchTextFontModifier, value);
    return this;
  }
  copyOption(value: CopyOptions): SearchAttribute {
    modifierWithKey(this._modifiersWithKeys, SearchCopyOptionModifier.identity, SearchCopyOptionModifier, value);
    return this;
  }
  textAlign(value: TextAlign): SearchAttribute {
    modifierWithKey(this._modifiersWithKeys, SearchTextAlignModifier.identity, SearchTextAlignModifier, value);
    return this;
  }
  enterKeyType(value: EnterKeyType): SearchAttribute {
    modifierWithKey(this._modifiersWithKeys, SearchEnterKeyTypeModifier.identity,
      SearchEnterKeyTypeModifier, value);
    return this;
  }
  fontFeature(value: FontFeature): SearchAttribute {
    modifierWithKey(this._modifiersWithKeys, SearchFontFeatureModifier.identity, SearchFontFeatureModifier, value);
    return this;
  }
  height(value: Length): this {
    modifierWithKey(this._modifiersWithKeys, SearchHeightModifier.identity, SearchHeightModifier, value);
    return this;
  }
  id(value: string): this {
    modifierWithKey(this._modifiersWithKeys, SearchIdModifier.identity, SearchIdModifier, value);
    return this;
  }
  key(value: string): this {
    modifierWithKey(this._modifiersWithKeys, SearchIdModifier.identity, SearchIdModifier, value);
    return this;
  }
  decoration(value: { type: TextDecorationType; color?: ResourceColor }): this {
    modifierWithKey(this._modifiersWithKeys, SearchDecorationModifier.identity, SearchDecorationModifier, value);
    return this;
  }
  letterSpacing(value: number | string): this {
    modifierWithKey(this._modifiersWithKeys, SearchLetterSpacingModifier.identity, SearchLetterSpacingModifier, value);
    return this;
  }
  lineHeight(value: number | string | Resource): this {
    modifierWithKey(this._modifiersWithKeys, SearchLineHeightModifier.identity, SearchLineHeightModifier, value);
    return this;
  }
  minFontSize(value: number | string | Resource): this {
    modifierWithKey(this._modifiersWithKeys, SearchMinFontSizeModifier.identity, SearchMinFontSizeModifier, value);
    return this;
  }
  maxFontSize(value: number | string | Resource): this {
    modifierWithKey(this._modifiersWithKeys, SearchMaxFontSizeModifier.identity, SearchMaxFontSizeModifier, value);
    return this;
  }
}
// @ts-ignore
globalThis.Search.attributeModifier = function (modifier: ArkComponent): void {
  attributeModifierFunc.call(this, modifier, (nativePtr: KNode) => {
    return new ArkSearchComponent(nativePtr);
  }, (nativePtr: KNode, classType: ModifierType, modifierJS: ModifierJS) => {
    return new modifierJS.SearchModifier(nativePtr, classType);
  });
};
