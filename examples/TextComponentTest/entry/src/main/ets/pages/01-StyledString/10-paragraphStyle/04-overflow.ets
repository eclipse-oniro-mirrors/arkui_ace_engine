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
struct struct_01_StyledString_01_StyledStringValue_10_ParagraphStyle_04_Overflow {
  @State message: string = '运动45分钟';
  @State start: number = 0
  @State end: number = 5
  @State showValue1: string = ''
  controller1: TextController = new TextController()
  mutableStyledString1: MutableStyledString = new MutableStyledString(this.message, [{
    start: this.start,
    length: this.end,
    styledKey: StyledStringKey.PARAGRAPH_STYLE,
    styledValue: new ParagraphStyle({ overflow: TextOverflow.None })
  }]);
  @State showValue2: string = ''
  controller2: TextController = new TextController()
  mutableStyledString2: MutableStyledString = new MutableStyledString(this.message, [{
    start: this.start,
    length: this.end,
    styledKey: StyledStringKey.PARAGRAPH_STYLE,
    styledValue: new ParagraphStyle({ overflow: TextOverflow.Clip })
  }]);
  @State showValue3: string = ''
  controller3: TextController = new TextController()
  mutableStyledString3: MutableStyledString = new MutableStyledString(this.message, [{
    start: this.start,
    length: this.end,
    styledKey: StyledStringKey.PARAGRAPH_STYLE,
    styledValue: new ParagraphStyle({ overflow: TextOverflow.Ellipsis })
  }]);
  @State showValue4: string = ''
  controller4: TextController = new TextController()
  mutableStyledString4: MutableStyledString = new MutableStyledString(this.message, [{
    start: this.start,
    length: this.end,
    styledKey: StyledStringKey.PARAGRAPH_STYLE,
    styledValue: new ParagraphStyle({ overflow: TextOverflow.MARQUEE })
  }]);

  aboutToAppear(): void {
    setTimeout(() => {
      let styles1 = this.mutableStyledString1.getStyles(this.start, this.end)
      let attribute1 = styles1[0].styledValue as ParagraphStyle;
      console.info('StyledString overflow:' + attribute1.overflow)
      this.showValue1 = 'getStyles 的值是 ' + attribute1.overflow?.toString()

      let styles2 = this.mutableStyledString2.getStyles(this.start, this.end)
      let attribute2 = styles2[0].styledValue as ParagraphStyle;
      console.info('StyledString overflow:' + attribute2.overflow)
      this.showValue2 = 'getStyles 的值是 ' + attribute2.overflow?.toString()

      let styles3 = this.mutableStyledString3.getStyles(this.start, this.end)
      let attribute3 = styles3[0].styledValue as ParagraphStyle;
      console.info('StyledString overflow:' + attribute3.overflow)
      this.showValue3 = 'getStyles 的值是 ' + attribute3.overflow?.toString()

      let styles4 = this.mutableStyledString4.getStyles(this.start, this.end)
      let attribute4 = styles4[0].styledValue as ParagraphStyle;
      console.info('StyledString overflow:' + attribute4.overflow)
      this.showValue4 = 'getStyles 的值是 ' + attribute4.overflow?.toString()

    }, 100)
  }

  build() {
    Column() {
      Column() {
        Text('测试 ParagraphStyle overflow TextOverflow 枚举值')
        Text('pass条件：overflow 的值是 0 ')
        Text(undefined, { controller: this.controller1 })
        Text(this.showValue1)
      }.borderWidth(1)

      Column() {
        Text('测试 ParagraphStyle overflow TextOverflow 枚举值')
        Text('pass条件：overflow 的值是 1 ')
        Text(undefined, { controller: this.controller2 })
        Text(this.showValue2)
      }.borderWidth(1)

      Column() {
        Text('测试 ParagraphStyle overflow TextOverflow 枚举值')
        Text('pass条件：overflow 的值是 2 ')
        Text(undefined, { controller: this.controller3 })
        Text(this.showValue3)
      }.borderWidth(1)

      Column() {
        Text('测试 ParagraphStyle overflow TextOverflow 枚举值')
        Text('pass条件：overflow 的值是 3 ')
        Text(undefined, { controller: this.controller4 })
        Text(this.showValue4)
      }.borderWidth(1)

    }
    .height('100%')
    .width('100%')
  }
}