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

import outlineColorTest_two from '../MyTest/OutlineColorTest_two'
import { loadComponentTestEntry } from '@ohos.arkui.componentTest'

@Entry
@Component
struct OutlineColorSample_two {
  @State any_Color: ResourceColor | EdgeColors = 'rgb(255, 201, 0)'

  aboutToAppear(): void {
    loadComponentTestEntry(outlineColorTest_two)
  }

  build() {
    Column({ space: 20 }) {
      Text()
      Row() {
        Row({ space: 20 }) {
          Text()
          Menu() {
            MenuItem({ content: "outlineColor", labelInfo: "outlineColor" })
              .outline({ width: 2 })
              .outlineColor(this.any_Color)
              .id('menuItemOutlineColor2')
            MenuItem({ content: "outlineColor", labelInfo: "outlineColor" })
          }

          Button('ButtonOutlineColor1').onClick(() => {
            this.any_Color = 0xBFAAFA
          })
          Button('ButtonOutlineColor2').onClick(() => {
            this.any_Color = Color.Blue
          })
          Button('ButtonOutlineColor3').onClick(() => {
            this.any_Color = $r("app.string.common_color")
          })
        }
      }
    }
  }
}