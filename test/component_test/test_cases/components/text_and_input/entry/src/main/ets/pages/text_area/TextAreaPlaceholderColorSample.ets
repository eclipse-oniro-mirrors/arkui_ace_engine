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
//测试代码路径
import { loadComponentTestEntry } from '@ohos.arkui.componentTest'
import textAreaPlaceholderColorTest from '../MyTest/TextAreaPlaceholderColorTest'

@Entry
@Component
struct TextAreaPlaceholderColorSample {
  @State placeholderColorType: ResourceColor = 0xffffff

  aboutToAppear(): void {
    loadComponentTestEntry(textAreaPlaceholderColorTest)
  }

  build() {
    Column() {
      TextArea({ placeholder: 'text area' })
        .placeholderColor(Color.Blue)
        .id('textAreaPlaceholderColor1')
      TextArea({ placeholder: 'text area' })
        .placeholderColor(this.placeholderColorType)
        .id('textAreaPlaceholderColor2')
      Button('TextAreaPlaceholderColor1')
        .onClick(() => {
          this.placeholderColorType = '#99182431'
        })
      Button('TextAreaPlaceholderColor2')
        .onClick(() => {
          this.placeholderColorType = $r('sys.color.ohos_id_color_text_hyperlink')
        })
    }
  }
}