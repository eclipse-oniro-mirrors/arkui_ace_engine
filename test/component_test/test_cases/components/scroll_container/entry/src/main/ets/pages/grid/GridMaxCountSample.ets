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
import gridMaxCountTest from '../MyTest/GridMaxCountTest'
import { loadComponentTestEntry } from '@ohos.arkui.componentTest'

@Entry
@Component
struct GridMaxCountSample {
  numbers: string[] = []
  @State number: number = -1

  aboutToAppear(): void {
    loadComponentTestEntry(gridMaxCountTest)
    for (let i = 1; i <= 30; i++) {
      this.numbers.push(i + '')
    }
  }

  build() {
    Scroll() {
      Column() {
        Grid() {
          ForEach(this.numbers, (day: string) => {
            GridItem() {
              Text('gridMaxCount' + day).fontSize(16).backgroundColor(0xF9CF93)
            }.width(80).height(80).borderWidth(2).borderColor(Color.Red)
          }, (day: string) => day)
        }
        .height(300)
        .columnsGap(10)
        .rowsGap(10)
        .maxCount(0)
        .minCount(2)
        .id('gridMaxCount1')

        Grid() {
          ForEach(this.numbers, (day: string) => {
            GridItem() {
              Text('gridMaxCount' + day).fontSize(16).backgroundColor(0xF9CF93)
            }.width(80).height(80).borderWidth(2).borderColor(Color.Red)
          }, (day: string) => day)
        }
        .height(300)
        .columnsGap(10)
        .rowsGap(10)
        .maxCount(this.number)
        .id('gridMaxCount2')

        Button('GridMaxCount')
          .onClick(() => {
            this.number = 3
          })
      }
    }
  }
}