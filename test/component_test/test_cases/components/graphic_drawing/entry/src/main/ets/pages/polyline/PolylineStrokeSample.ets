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
// 测试代码路径
import polylineStrokeTest from '../MyTest/PolylineStrokeTest'
import { loadComponentTestEntry } from '@ohos.arkui.componentTest'

@Entry
@Component
struct PolylineStrokeSample {
  @State polylineStrokeColorValue1: number | Color = 0x0000ff
  @State polylineStrokeColorValue2: string | Resource = 'rgb(0,0,255)'

  aboutToAppear(): void {
    loadComponentTestEntry(polylineStrokeTest)
  }

  build() {
    Column({ space: 10 }) {
      Polyline({ width: 100, height: 100 })
        .points([[0, 0], [20, 60], [100, 100]])
        .stroke(this.polylineStrokeColorValue1)
        .id('polylineStroke1')
      Button('PolylineStroke1')
        .onClick(() => {
          this.polylineStrokeColorValue1 = Color.Red
        })
      Polyline({ width: 100, height: 100 })
        .points([[0, 0], [20, 60], [100, 100]])
        .stroke(this.polylineStrokeColorValue2)
        .id('polylineStroke2')
      Button('PolylineStroke2')
        .onClick(() => {
          this.polylineStrokeColorValue2 = $r('app.color.color_red')
        })
    }
  }
}