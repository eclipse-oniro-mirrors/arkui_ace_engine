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
import actionSheetAlignmentTest from '../MyTest/ActionSheetAlignmentTest'
import { loadComponentTestEntry } from '@ohos.arkui.componentTest'

@Entry
@Component
struct ActionSheetAlignmentSample {
  @State alignment: DialogAlignment[] =
    [DialogAlignment.Top, DialogAlignment.Center, DialogAlignment.Bottom, DialogAlignment.Default,
      DialogAlignment.TopStart, DialogAlignment.TopEnd, DialogAlignment.CenterStart, DialogAlignment.CenterEnd,
      DialogAlignment.BottomStart, DialogAlignment.BottomEnd]
  @State string: string = 'actionText'
  @State index: number = 0

  aboutToAppear(): void {
    loadComponentTestEntry(actionSheetAlignmentTest)
  }

  build() {
    Column() {
      Text(this.string)
      Button('ActionSheetAlignmentTest1')
        .onClick(() => {
          ActionSheet.show({
            title: 'title1',
            message: 'message1',
            alignment: this.alignment[this.index],
            sheets: [{
              title: 'sheetsTitle1', action: () => {
                this.string = 'actionText1'
              }
            },]
          })
        })
      Button('ActionSheetAlignmentTest2')
        .onClick(() => {
          this.index++
          this.index %= 10
        })
    }
  }
}