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
import { LengthMetrics, LengthUnit } from '@kit.ArkUI'

@Entry
@Component
struct struct_01_StyledString_01_StyledStringValue_01_TextStyle {
  @State message: string = '运动45分钟';
  @State start: number = 0
  @State end: number = 5
  @State showValue1: string = ''
  controller1: TextController = new TextController()
  mutableStyledString1: MutableStyledString = new MutableStyledString(this.message, [{
    start: this.start,
    length: this.end,
    styledKey: StyledStringKey.FONT,
    styledValue: new TextStyle({ fontSize: LengthMetrics.px(200), })
  }]);
  @State showValue2: string = ''
  controller2: TextController = new TextController()
  mutableStyledString2: MutableStyledString = new MutableStyledString(this.message, [{
    start: this.start,
    length: this.end,
    styledKey: StyledStringKey.FONT,
    styledValue: new TextStyle({ fontSize: LengthMetrics.vp(200), })
  }]);
  @State showValue3: string = ''
  controller3: TextController = new TextController()
  mutableStyledString3: MutableStyledString = new MutableStyledString(this.message, [{
    start: this.start,
    length: this.end,
    styledKey: StyledStringKey.FONT,
    styledValue: new TextStyle({ fontSize: LengthMetrics.fp(200), })
  }]);
  @State showValue4: string = ''
  controller4: TextController = new TextController()
  mutableStyledString4: MutableStyledString = new MutableStyledString(this.message, [{
    start: this.start,
    length: this.end,
    styledKey: StyledStringKey.FONT,
    styledValue: new TextStyle({ fontSize: LengthMetrics.percent(200), })
  }]);
  @State showValue5: string = ''
  controller5: TextController = new TextController()
  mutableStyledString5: MutableStyledString = new MutableStyledString(this.message, [{
    start: this.start,
    length: this.end,
    styledKey: StyledStringKey.FONT,
    styledValue: new TextStyle({ fontSize: LengthMetrics.lpx(200), })
  }]);

  aboutToAppear(): void {
    setTimeout(() => {
      let styles1 = this.mutableStyledString1.getStyles(this.start, this.end)
      let fontAttr1 = styles1[0].styledValue as TextStyle;
      console.info('StyledString fontSize:' + fontAttr1.fontSize)
      this.showValue1 = 'getStyles 的值是 ' + fontAttr1.fontSize?.toString()

      let styles2 = this.mutableStyledString2.getStyles(this.start, this.end)
      let fontAttr2 = styles2[0].styledValue as TextStyle;
      console.info('StyledString fontSize:' + fontAttr2.fontSize)
      this.showValue2 = 'getStyles 的值是 ' + fontAttr2.fontSize?.toString()

      let styles3 = this.mutableStyledString3.getStyles(this.start, this.end)
      let fontAttr3 = styles3[0].styledValue as TextStyle;
      console.info('StyledString fontSize:' + fontAttr3.fontSize)
      this.showValue3 = 'getStyles 的值是 ' + fontAttr3.fontSize?.toString()

      let styles4 = this.mutableStyledString4.getStyles(this.start, this.end)
      let fontAttr4 = styles4[0].styledValue as TextStyle;
      console.info('StyledString fontSize:' + fontAttr4.fontSize)
      this.showValue4 = 'getStyles 的值是 ' + fontAttr4.fontSize?.toString()

      let styles5 = this.mutableStyledString5.getStyles(this.start, this.end)
      let fontAttr5 = styles5[0].styledValue as TextStyle;
      console.info('StyledString fontSize:' + fontAttr5.fontSize)
      this.showValue5 = 'getStyles 的值是 ' + fontAttr5.fontSize?.toString()

    }, 100)
  }

  build() {
    Column() {
      Column() {
        Text('测试 TextStyle fontSize LengthMetrics unit px 类型')
        Text('pass条件：fontSize 的值是 133.33333333333334 ')
        Text(undefined, { controller: this.controller1 })
        Text(this.showValue1)
      }.borderWidth(1)

      Column() {
        Text('测试 TextStyle fontSize LengthMetrics unit vp 类型')
        Text('pass条件：fontSize 的值是 200 ')
        Text(undefined, { controller: this.controller2 })
        Text(this.showValue2)
      }.borderWidth(1)

      Column() {
        Text('测试 TextStyle fontSize LengthMetrics unit fp 类型')
        Text('pass条件：fontSize 的值是 200 ')
        Text(undefined, { controller: this.controller3 })
        Text(this.showValue3)
      }.borderWidth(1)

      Column() {
        Text('测试 TextStyle fontSize LengthMetrics unit percent 类型')
        Text('pass条件：fontSize 的值是 16 ')
        Text(undefined, { controller: this.controller4 })
        Text(this.showValue4)
      }.borderWidth(1)

      Column() {
        Text('测试 TextStyle fontSize LengthMetrics unit lpx 类型')
        Text('pass条件：fontSize 的值是 133.33333333333334 ')
        Text(undefined, { controller: this.controller5 })
        Text(this.showValue5)
      }.borderWidth(1)

    }
    .height('100%')
    .width('100%')
  }
}