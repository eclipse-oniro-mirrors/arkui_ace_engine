/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

@Entry
@Component
struct struct_01_StyledString_01_StyledStringValue_10_ParagraphStyle_03_MaxLength {
  @State message: string = '运动45分钟';
  @State start: number = 0
  @State end: number = 5
  @State showValue1: string = ''
  controller1: TextController = new TextController()
  mutableStyledString1: MutableStyledString = new MutableStyledString(this.message, [{
    start: this.start,
    length: this.end,
    styledKey: StyledStringKey.PARAGRAPH_STYLE,
    styledValue: new ParagraphStyle({ maxLines: 200 })
  }]);

  aboutToAppear(): void {
    setTimeout(() => {
      let styles1 = this.mutableStyledString1.getStyles(this.start, this.end)
      let attribute1 = styles1[0].styledValue as ParagraphStyle;
      console.info('StyledString maxLines:' + attribute1.maxLines)
      this.showValue1 = 'getStyles 的值是 ' + attribute1.maxLines?.toString()

    }, 100)
  }

  build() {
    Column() {
      Column() {
        Text('测试 TextStyle ParagraphStyle maxLines number 类型')
        Text('pass条件：maxLines 的值是 200 ')
        Text(undefined, { controller: this.controller1 })
        Text(this.showValue1)
      }.borderWidth(1)

    }
    .height('100%')
    .width('100%')
  }
}