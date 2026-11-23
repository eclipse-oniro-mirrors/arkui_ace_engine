/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

/**
 * ColorPicker with Image gradient test example
 */
@Entry
@Component
struct ColorPickerGradientExample {
  @State red: number = 33
  @State green: number = 150
  @State blue: number = 243
  @State angle: number = 90
  @State resultText: string = ''

  private toHex(value: number): string {
    let hex = value.toString(16)
    return hex.length === 1 ? '0' + hex : hex
  }

  private getColorStr(): string {
    return '#FF' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)
  }

  private getComplementaryColor(): string {
    return '#FF' + this.toHex(255 - this.red) + this.toHex(255 - this.green) + this.toHex(255 - this.blue)
  }

  build() {
    Scroll() {
      Column() {
        Text('ColorPicker 娓愬彉娴嬭瘯')
          .fontSize(22).fontWeight(FontWeight.Bold).margin({ top: 30, bottom: 10 })
        Text('閫氳繃RGB婊戝潡璋冭妭棰滆壊锛屽疄鏃堕瑙堟笎鍙樻晥鏋?)
          .fontSize(14).fontColor('#999999').margin({ bottom: 10 })

        Column() {
          Text('褰撳墠娓愬彉').fontSize(14).fontColor('#FFFFFF').margin({ top: 10 })
          Text(this.getColorStr().toUpperCase()).fontSize(16).fontColor('#FFFFFF').margin({ top: 5, bottom: 10 })
        }
        .width('100%').height(80).linearGradient({ angle: this.angle, colors: [[this.getColorStr(), 0.0], [this.getComplementaryColor(), 1.0]] })
        .borderRadius(12).justifyContent(FlexAlign.Center)

        Text('R 绾㈣壊閫氶亾').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() { Text('R').fontSize(16).fontWeight(FontWeight.Bold).width(30).fontColor('#F44336'); Slider({ value: this.red, min: 0, max: 255, step: 1 }).layoutWeight(1).blockColor('#F44336').trackColor('#E0E0E0').selectedColor('#F44336').onChange((value: number) => { this.red = Math.round(value); this.resultText = 'R=' + Math.round(value) }); Text(this.red.toString()).fontSize(12).width(35).textAlign(TextAlign.End) }.width('100%')

        Text('G 缁胯壊閫氶亾').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() { Text('G').fontSize(16).fontWeight(FontWeight.Bold).width(30).fontColor('#4CAF50'); Slider({ value: this.green, min: 0, max: 255, step: 1 }).layoutWeight(1).blockColor('#4CAF50').trackColor('#E0E0E0').selectedColor('#4CAF50').onChange((value: number) => { this.green = Math.round(value); this.resultText = 'G=' + Math.round(value) }); Text(this.green.toString()).fontSize(12).width(35).textAlign(TextAlign.End) }.width('100%')

        Text('B 钃濊壊閫氶亾').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() { Text('B').fontSize(16).fontWeight(FontWeight.Bold).width(30).fontColor('#2196F3'); Slider({ value: this.blue, min: 0, max: 255, step: 1 }).layoutWeight(1).blockColor('#2196F3').trackColor('#E0E0E0').selectedColor('#2196F3').onChange((value: number) => { this.blue = Math.round(value); this.resultText = 'B=' + Math.round(value) }); Text(this.blue.toString()).fontSize(12).width(35).textAlign(TextAlign.End) }.width('100%')

        Text('娓愬彉瑙掑害').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() { Text('Angle').fontSize(14).width(50); Slider({ value: this.angle, min: 0, max: 360, step: 1 }).layoutWeight(1).blockColor('#007DFF').trackColor('#E0E0E0').selectedColor('#007DFF').onChange((value: number) => { this.angle = Math.round(value); this.resultText = 'Angle=' + Math.round(value) + 'deg' }); Text(this.angle + 'deg').fontSize(12).width(50).textAlign(TextAlign.End) }.width('100%')

        Text(this.resultText).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        Text('涓嶅悓瑙掑害瀵规瘮').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() {
          Column() { Text('0deg').fontSize(10).margin({ bottom: 3 }); Column().width(70).height(70).borderRadius(8).linearGradient({ angle: 0, colors: [[this.getColorStr(), 0.0], [this.getComplementaryColor(), 1.0]] }) }.margin({ right: 5 })
          Column() { Text('45deg').fontSize(10).margin({ bottom: 3 }); Column().width(70).height(70).borderRadius(8).linearGradient({ angle: 45, colors: [[this.getColorStr(), 0.0], [this.getComplementaryColor(), 1.0]] }) }.margin({ right: 5 })
          Column() { Text('90deg').fontSize(10).margin({ bottom: 3 }); Column().width(70).height(70).borderRadius(8).linearGradient({ angle: 90, colors: [[this.getColorStr(), 0.0], [this.getComplementaryColor(), 1.0]] }) }.margin({ right: 5 })
          Column() { Text('135deg').fontSize(10).margin({ bottom: 3 }); Column().width(70).height(70).borderRadius(8).linearGradient({ angle: 135, colors: [[this.getColorStr(), 0.0], [this.getComplementaryColor(), 1.0]] }) }.margin({ right: 5 })
          Column() { Text('180deg').fontSize(10).margin({ bottom: 3 }); Column().width(70).height(70).borderRadius(8).linearGradient({ angle: 180, colors: [[this.getColorStr(), 0.0], [this.getComplementaryColor(), 1.0]] }) }
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('棰勮棰滆壊').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() { Button('绾㈣壊').fontSize(12).height(30).margin(2).onClick(() => { this.red = 255; this.green = 0; this.blue = 0 }); Button('缁胯壊').fontSize(12).height(30).margin(2).onClick(() => { this.red = 0; this.green = 255; this.blue = 0 }); Button('钃濊壊').fontSize(12).height(30).margin(2).onClick(() => { this.red = 0; this.green = 0; this.blue = 255 }); Button('鐧借壊').fontSize(12).height(30).margin(2).onClick(() => { this.red = 255; this.green = 255; this.blue = 255 }) }.width('100%')

        Divider().margin({ top: 15 })
        Text('棰滆壊鍊艰緭鍑?).fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Text('HEX: ' + this.getColorStr().toUpperCase()).fontSize(16).margin({ top: 5 })
        Text('RGB: ' + this.red + ', ' + this.green + ', ' + this.blue).fontSize(16).margin({ top: 5 })
        Text('浜掕ˉ鑹? ' + this.getComplementaryColor().toUpperCase()).fontSize(16).margin({ top: 5 })

        Divider().margin({ top: 15 })
        Text('100. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('101. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('102. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('103. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('104. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4); Text('FFE0B2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('105. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('106. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('107. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('108. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('109. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('110. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('111. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('112. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('113. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4); Text('FFE0B2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('114. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('115. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('116. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('117. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('118. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('119. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('120. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('121. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('122. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4); Text('FFE0B2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('123. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('124. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('125. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('126. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('127. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('128. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('129. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('130. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('131. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4); Text('FFE0B2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('132. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('133. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('134. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('135. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('136. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('137. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('138. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('139. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4); Text('FFE0B2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('140. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('141. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('142. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('143. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('144. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('145. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('146. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('147. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('148. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4); Text('FFE0B2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('149. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('150. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('151. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('152. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('153. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('154. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('155. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('156. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('157. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4); Text('FFE0B2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('158. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('159. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('160. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('161. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('162. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('163. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('164. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('165. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('166. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4); Text('FFE0B2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('167. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('168. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('169. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('170. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('171. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('172. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('173. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('174. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4); Text('FFE0B2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('175. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('176. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('177. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('178. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('179. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('180. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('181. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('182. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('183. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4); Text('FFE0B2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('184. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('185. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('186. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('187. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('188. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('189. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('190. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('191. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('192. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4); Text('FFE0B2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('193. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('194. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('195. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('196. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('197. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('198. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('199. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('200. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('201. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4); Text('FFE0B2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('202. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('203. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('204. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('205. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('206. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('207. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('208. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('209. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4); Text('FFE0B2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('210. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('211. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('212. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('213. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('214. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('215. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('216. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('217. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('218. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4); Text('FFE0B2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('219. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('220. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('221. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('222. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('223. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('224. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('225. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('226. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('227. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4); Text('FFE0B2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('228. 色卡组').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        Divider().margin({ top: 15, bottom: 30 })
      }
      .width('100%').padding(20)
    }
    .width('100%').height('100%').backgroundColor('#F5F5F5')
  }
}
