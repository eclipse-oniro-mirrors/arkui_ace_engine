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
import gridScrollBarTest from '../MyTest/GridScrollBarTest'
import { loadComponentTestEntry } from '@ohos.arkui.componentTest'

@Entry
@Component
struct GridScrollBarSample {
  numbers: String[] = ['0', '1', '2', '3', '4']
  @State state: BarState = BarState.Off

  aboutToAppear(): void {
    loadComponentTestEntry(gridScrollBarTest)
  }

  build() {
    Column({ space: 5 }) {
      Grid() {
        ForEach(this.numbers, () => {
          ForEach(this.numbers, (day: string) => {
            GridItem() {
              Text('gridScrollBarTest' + day)
                .backgroundColor(0xF9CF93)
                .width('100%')
                .height(80)
            }
          }, (day: string) => day)
        }, (day: string) => day)
      }
      .columnsTemplate('1fr 1fr 1fr 1fr 1fr')
      .scrollBar(BarState.On)
      .height(250)
      .id('gridScrollBar1')

      Grid() {
        ForEach(this.numbers, () => {
          ForEach(this.numbers, (day: string) => {
            GridItem() {
              Text('gridScrollBarTest' + day)
                .backgroundColor(0xF9CF93)
                .width('100%')
                .height(80)
            }
          }, (day: string) => day)
        }, (day: string) => day)
      }
      .columnsTemplate('1fr 1fr 1fr 1fr 1fr')
      .scrollBar(this.state)
      .height(250)
      .id('gridScrollBar2')

      Button('GridScrollBar')
        .onClick(() => {
          this.state = BarState.Auto
        })
    }.width('100%').margin({ top: 5 })
  }
}