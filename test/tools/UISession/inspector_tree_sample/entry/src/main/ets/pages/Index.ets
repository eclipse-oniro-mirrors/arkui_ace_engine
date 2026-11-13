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

import provider from 'libinspector_tree_provider.so';
import { NativeFields } from './NativeFields';
import { ProviderTree } from './ProviderTree';
import { ResponseRegions } from './ResponseRegions';
import { WindowAndExtension } from './WindowAndExtension';
import window from '@ohos.window';

@Entry
@Component
struct Index {
  @State caseId: string = 'native_fields';
  @State generation: number = 0;
  @State preparation: string = '';
  private cases: string[] = ['native_fields', 'privacy_password', 'privacy_obscured', 'privacy_window',
    'response_regions', 'expanded_states', 'window_titles', 'provider_normal', 'provider_empty',
    'provider_failure', 'provider_delay', 'provider_invalid', 'provider_limits', 'provider_large_payload', 'web_uie'];

  async selectCase(value: string): Promise<void> {
    try {
      const context = this.getUIContext().getHostContext();
      if (context) {
        const target = await window.getLastWindow(context);
        if (this.caseId === 'privacy_window' || value === 'privacy_window') {
          await target.setWindowPrivacyMode(value === 'privacy_window');
        }
        if (canIUse('SystemCapability.Window.SessionManager')) {
          await target.setWindowTitle('');
        }
      }
      this.preparation = 'ready';
    } catch {
      this.preparation = 'Window preparation failed; check test permissions and device support';
      return;
    }
    provider.configure(value, 2, 0, 0);
    this.caseId = value;
    this.generation++;
  }

  build() {
    Column({ space: 8 }) {
      Text('InspectorTree input fixture').id('fixture-title')
      Text(this.preparation).id('fixture-preparation')
      Select(this.cases.map((value: string): SelectOption => ({ value: value })))
        .value(this.caseId).onSelect((index: number) => { this.selectCase(this.cases[index]); })
      Text(`caseId=${this.caseId}, reset=${this.generation}`).id('fixture-case')
      Scroll() {
        Column({ space: 12 }) {
          ForEach([this.generation], (revision: number) => {
            if (this.caseId.startsWith('provider_')) {
              ProviderTree({ caseId: this.caseId })
            } else if (this.caseId === 'response_regions') {
              ResponseRegions()
            } else if (this.caseId === 'window_titles' || this.caseId === 'web_uie') {
              WindowAndExtension({ caseId: this.caseId })
            } else {
              NativeFields({ caseId: this.caseId })
            }
          }, (revision: number) => revision.toString())
        }.width('100%')
      }.layoutWeight(1)
    }.padding(12).width('100%').height('100%')
  }
}
