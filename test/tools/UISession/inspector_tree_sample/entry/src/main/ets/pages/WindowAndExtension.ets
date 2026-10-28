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

import window from '@ohos.window';
import webview from '@ohos.web.webview';

@Component
export struct WindowAndExtension {
  caseId: string = 'window_titles';
  @State title: string = '';
  @State result: string = '';
  @State bundle: string = '';
  @State ability: string = '';
  @State showExtension: boolean = false;
  private web: webview.WebviewController = new webview.WebviewController();
  async setTitle(value: string): Promise<void> {
    try {
      const context = this.getUIContext().getHostContext();
      if (context && canIUse('SystemCapability.Window.SessionManager')) {
        const target = await window.getLastWindow(context);
        await target.setWindowTitle(value);
        this.result = 'Title applied';
      } else {
        this.result = 'Window title is not supported on this device';
      }
    } catch {
      this.result = 'Window title update failed';
    }
  }
  build() {
    Column({ space: 8 }) {
      Text(this.result)
      TextInput({ text: this.title, placeholder: 'Explicit window title' }).onChange((v: string) => { this.title = v; })
      Button('Apply title').onClick(() => { this.setTitle(this.title); })
      Button('Reset to application label').onClick(() => { this.setTitle(''); })
      if (this.caseId === 'web_uie') {
        Web({ src: 'data:text/html,<title>Inspector Web</title><p id="web-text">web-content</p>', controller: this.web })
          .width('100%').height(150).id('web-host')
        TextInput({ placeholder: 'Installed UIExtension bundle' }).onChange((v: string) => { this.bundle = v; })
        TextInput({ placeholder: 'UIExtension ability' }).onChange((v: string) => { this.ability = v; })
        Button('Attach installed extension').onClick(() => { this.showExtension = this.bundle.length > 0 && this.ability.length > 0; })
        if (this.showExtension) {
          UIExtensionComponent({ bundleName: this.bundle, abilityName: this.ability }).height(200).id('extension-host')
        }
      }
    }
  }
}
