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
import rectStrokeWidthTest from '../MyTest/RectStrokeWidthTest'
import { loadComponentTestEntry } from '@ohos.arkui.componentTest'

@Entry
@Component
struct RectStrokeWidthSample {
  @State rectStrokeWidthValue: Length = 5

  aboutToAppear(): void {
    loadComponentTestEntry(rectStrokeWidthTest)
  }

  build() {
    Column({ space: 10 }) {
      Rect()
        .width('90%')
        .height(50)
        .stroke(Color.Red)
        .strokeWidth(this.rectStrokeWidthValue)
        .id('rectStrokeWidth1')
      Button('RectStrokeWidth')
        .onClick(() => {
          this.rectStrokeWidthValue = '10px'
        })
      Rect()
        .width('90%')
        .height(50)
        .stroke(Color.Red)
        .strokeWidth($r('app.string.stroke_width'))
        .id('rectStrokeWidth2')
    }
  }
}