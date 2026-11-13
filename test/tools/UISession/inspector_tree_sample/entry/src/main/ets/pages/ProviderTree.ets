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

@Component
export struct ProviderTree {
  caseId: string = 'provider_normal';
  @State mode: string = 'provider_normal';
  aboutToAppear(): void {
    this.mode = this.caseId;
    provider.configure(this.mode, this.mode === 'provider_limits' ? 10001 : (this.mode === 'provider_large_payload' ? 12 : 2),
      this.mode === 'provider_large_payload' ? 1024 * 1024 : 0, 0);
  }
  build() {
    Column({ space: 8 }) {
      Text(`Provider input=${this.mode}`)
      Button('10001 nodes').onClick(() => { provider.configure('provider_limits', 10001, 0, 0); })
      Button('Large provider text').onClick(() => { provider.configure('provider_large_payload', 12, 1024 * 1024, 0); })
      Button('1100 ms return').onClick(() => { provider.configure('provider_delay', 2, 0, 1100); })
      Button('Two hosts, 600 ms each').onClick(() => { provider.configure('provider_delay', 2, 0, 600); })
      XComponent({ id: 'provider-first', type: XComponentType.SURFACE, libraryname: 'inspector_tree_provider' })
        .width(240).height(100).id('provider-first')
      XComponent({ id: 'provider-second', type: XComponentType.SURFACE, libraryname: 'inspector_tree_provider' })
        .width(240).height(100).translate({ x: 20, y: 10 }).id('provider-second')
      XComponent({ id: 'provider-hidden', type: XComponentType.SURFACE, libraryname: 'inspector_tree_provider' })
        .width(240).height(100).visibility(Visibility.Hidden).id('provider-hidden')
      Text(`queries=${provider.queryCount()}`).id('provider-count')
      Button('Refresh query count').onClick(() => { this.mode = `${this.caseId}:${provider.queryCount()}`; })
    }
  }
}
