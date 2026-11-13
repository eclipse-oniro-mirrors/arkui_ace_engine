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

/**
 * TS wrapper that pulls up the HDS color picker for <input type="color"> in Web.
 * The wrapper owns the ColorPickerController; the C++ side (WebPattern) only
 * sees the two globals registered at the bottom of this file.
 *
 * NOTE: compiled by bare es2abc which accepts plain-JS syntax only (same as
 * in-tree wrappers like jsImageGeneration.js / customtitle.js) — no type
 * annotations, no kit imports. The controller class is resolved at runtime
 * through the platform's HSP require global.
 */
class HdsColorPickerWrapper {
  // HDS color picker holder — class-private, never exposed to C++.
  static colorPickerController = undefined;

  /**
   * Pull up the HDS color picker.
   *
   * @param uiContext    the app UIContext, provided by the C++ side.
   * @param initialColor current color of the web element, e.g. '#rrggbb'.
   * @param onSelected   called once when a color is confirmed ('#rrggbb').
   * @param onClose      called once when the picker is dismissed without a choice.
   */
  static open(uiContext, initialColor, onSelected, onClose) {
    // Re-open protection: drop any previous picker first.
    HdsColorPickerWrapper.close();
    try {
      // Resolve the HDS controller class from the HSP at runtime.
      const ColorPickerController = globalThis.__requireHspModuleForAdvancedUIComponent__(
        '@bundle:com.huawei.hmos.hdscomponent/HdsComponent/ets/pages/HdsColorPicker',
        'ColorPickerController');
      if (!ColorPickerController) {
        onClose();
        return;
      }
      const controller = new ColorPickerController();
      controller.openColorPicker(uiContext, {
        options:undefined,
        initialColor:initialColor,
        onColorSelected:onSelected,
        onClose:onClose});
      HdsColorPickerWrapper.colorPickerController = controller;
    } catch (e) {
      HdsColorPickerWrapper.colorPickerController = undefined;
      onClose();
    }
  }

  /**
   * Close the HDS color picker if it is open (no-op otherwise).
   */
  static close() {
    try {
      const controller = HdsColorPickerWrapper.colorPickerController;
      if (controller) {
        controller.closeColorPicker();
      }
    } catch (e) {
      // Ignore close errors — the picker may already be gone.
    }
    HdsColorPickerWrapper.colorPickerController = undefined;
  }
}

// C++ entry points — the only surface this module exposes.
globalThis.__openHdsColorPicker__ = HdsColorPickerWrapper.open;
globalThis.__closeHdsColorPicker__ = HdsColorPickerWrapper.close;
