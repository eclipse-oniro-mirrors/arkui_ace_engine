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

import { LengthMetrics } from '@ohos.arkui.node';

@Component
export struct ResponseRegions {
  @State clicks: number = 0;
  @State touches: number = 0;
  @State mouseEvents: number = 0;
  build() {
    Column({ space: 20 }) {
      Text(`clicks=${this.clicks}, touches=${this.touches}, mouse=${this.mouseEvents}`).id('event-counts')
      Button('Reset counts').onClick(() => { this.clicks = 0; this.touches = 0; this.mouseEvents = 0; })
      Button('Tools').width(100).height(50).id('tool-regions')
        .responseRegionList([
          { tool: ResponseRegionSupportedTool.ALL, x: LengthMetrics.vp(0), y: LengthMetrics.vp(0),
            width: LengthMetrics.vp(30), height: LengthMetrics.vp(40) },
          { tool: ResponseRegionSupportedTool.FINGER, x: LengthMetrics.vp(40), y: LengthMetrics.vp(0),
            width: LengthMetrics.vp(40), height: LengthMetrics.vp(40) },
          { tool: ResponseRegionSupportedTool.PEN, x: LengthMetrics.vp(-10), y: LengthMetrics.vp(-10),
            width: LengthMetrics.vp(30), height: LengthMetrics.vp(30) },
          { tool: ResponseRegionSupportedTool.MOUSE, x: LengthMetrics.vp(-20), y: LengthMetrics.vp(0),
            width: LengthMetrics.vp(120), height: LengthMetrics.vp(50) }])
        .onClick(() => { this.clicks++; }).onTouch(() => { this.touches++; })
        .onMouse(() => { this.mouseEvents++; })
      Button('Traditional').responseRegion({ x: -10, y: -5, width: 120, height: 60 })
        .mouseResponseRegion({ x: -20, y: -10, width: 140, height: 70 }).id('traditional-regions')
      Column() {
        Button('Transformed').width(100).height(50)
          .responseRegion({ x: 0, y: 0, width: 100, height: 50 })
          .rotate({ angle: 30 }).scale({ x: 1.25, y: 0.75 }).translate({ x: 20, y: 10 }).id('transformed-region')
      }.width(130).height(75).clip(true)
      Button('No explicit region').id('no-region')
    }
  }
}
