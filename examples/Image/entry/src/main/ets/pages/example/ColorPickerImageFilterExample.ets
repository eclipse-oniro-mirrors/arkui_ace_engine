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
 * ColorPicker with Image foregroundColor filter test example
 */
@Entry
@Component
struct ColorPickerImageFilterExample {
  @State red: number = 128
  @State green: number = 128
  @State blue: number = 128
  @State opacity: number = 100
  @State resultText: string = ''

  private toHex(value: number): string {
    let hex = value.toString(16)
    return hex.length === 1 ? '0' + hex : hex
  }

  private getColorStr(): string {
    return '#FF' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)
  }

  build() {
    Scroll() {
      Column() {
        Text('ColorPicker 鍥剧墖棰滆壊婊ら暅娴嬭瘯')
          .fontSize(22).fontWeight(FontWeight.Bold).margin({ top: 30, bottom: 10 })

        Row() {
          Column() {
            Text('鍘熷�?).fontSize(12).margin({ bottom: 5 })
            Image($r('app.media.startIcon')).width(100).height(100).borderRadius(8)
          }.margin({ right: 20 })
          Column() {
            Text('棰滆壊婊ら暅').fontSize(12).margin({ bottom: 5 })
            Image($r('app.media.startIcon')).width(100).height(100).borderRadius(8)
              .foregroundColor(this.getColorStr()).opacity(this.opacity / 100)
          }
        }.width('100%')

        Column() {
          Text('鍓嶆櫙鑹? ' + this.getColorStr().toUpperCase()).fontSize(14).fontColor('#FFFFFF').margin({ top: 5 })
          Text('閫忔槑搴? ' + this.opacity + '%').fontSize(14).fontColor('#FFFFFF').margin({ top: 5, bottom: 5 })
        }
        .width('100%').height(60).backgroundColor(this.getColorStr())
        .borderRadius(8).justifyContent(FlexAlign.Center)

        Text('R 绾㈣壊閫氶亾').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() {
          Text('R').fontSize(16).fontWeight(FontWeight.Bold).width(30).fontColor('#F44336')
          Slider({ value: this.red, min: 0, max: 255, step: 1 })
            .layoutWeight(1).blockColor('#F44336').trackColor('#E0E0E0').selectedColor('#F44336')
            .onChange((value: number) => { this.red = Math.round(value); this.resultText = 'R=' + Math.round(value) })
          Text(this.red.toString()).fontSize(12).width(35).textAlign(TextAlign.End)
        }.width('100%')

        Text('G 缁胯壊閫氶亾').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() {
          Text('G').fontSize(16).fontWeight(FontWeight.Bold).width(30).fontColor('#4CAF50')
          Slider({ value: this.green, min: 0, max: 255, step: 1 })
            .layoutWeight(1).blockColor('#4CAF50').trackColor('#E0E0E0').selectedColor('#4CAF50')
            .onChange((value: number) => { this.green = Math.round(value); this.resultText = 'G=' + Math.round(value) })
          Text(this.green.toString()).fontSize(12).width(35).textAlign(TextAlign.End)
        }.width('100%')

        Text('B 钃濊壊閫氶亾').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() {
          Text('B').fontSize(16).fontWeight(FontWeight.Bold).width(30).fontColor('#2196F3')
          Slider({ value: this.blue, min: 0, max: 255, step: 1 })
            .layoutWeight(1).blockColor('#2196F3').trackColor('#E0E0E0').selectedColor('#2196F3')
            .onChange((value: number) => { this.blue = Math.round(value); this.resultText = 'B=' + Math.round(value) })
          Text(this.blue.toString()).fontSize(12).width(35).textAlign(TextAlign.End)
        }.width('100%')

        Text('閫忔槑搴?).fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() {
          Text('O').fontSize(16).fontWeight(FontWeight.Bold).width(30).fontColor('#999999')
          Slider({ value: this.opacity, min: 0, max: 100, step: 1 })
            .layoutWeight(1).blockColor('#999999').trackColor('#E0E0E0').selectedColor('#999999')
            .onChange((value: number) => { this.opacity = Math.round(value); this.resultText = 'Opacity=' + Math.round(value) + '%' })
          Text(this.opacity + '%').fontSize(12).width(40).textAlign(TextAlign.End)
        }.width('100%')

        Text(this.resultText).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        Text('涓嶅悓閫忔槑搴﹀姣?).fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() {
          Column() { Text('100%').fontSize(10).margin({ bottom: 3 }); Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8).foregroundColor(this.getColorStr()).opacity(1.0) }.margin({ right: 5 })
          Column() { Text('75%').fontSize(10).margin({ bottom: 3 }); Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8).foregroundColor(this.getColorStr()).opacity(0.75) }.margin({ right: 5 })
          Column() { Text('50%').fontSize(10).margin({ bottom: 3 }); Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8).foregroundColor(this.getColorStr()).opacity(0.5) }.margin({ right: 5 })
          Column() { Text('25%').fontSize(10).margin({ bottom: 3 }); Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8).foregroundColor(this.getColorStr()).opacity(0.25) }
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('棰勮棰滆壊婊ら�?).fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() {
          Column() { Text('绾㈣�?).fontSize(10).margin({ bottom: 3 }); Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8).foregroundColor('#FFFF0000') }.margin({ right: 5 })
          Column() { Text('缁胯�?).fontSize(10).margin({ bottom: 3 }); Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8).foregroundColor('#FF00FF00') }.margin({ right: 5 })
          Column() { Text('钃濊�?).fontSize(10).margin({ bottom: 3 }); Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8).foregroundColor('#FF0000FF') }.margin({ right: 5 })
          Column() { Text('榛勮�?).fontSize(10).margin({ bottom: 3 }); Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8).foregroundColor('#FFFFFF00') }.margin({ right: 5 })
          Column() { Text('绱�?).fontSize(10).margin({ bottom: 3 }); Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8).foregroundColor('#FFFF00FF') }
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('蹇嵎棰滆壊閫夋�?).fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() {
          Button('绾㈣�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 255; this.green = 0; this.blue = 0 })
          Button('缁胯�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 0; this.green = 255; this.blue = 0 })
          Button('钃濊�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 0; this.green = 0; this.blue = 255 })
          Button('鐧借壊').fontSize(12).height(30).margin(2).onClick(() => { this.red = 255; this.green = 255; this.blue = 255 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('棰滆壊鍊艰緭�?).fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Text('HEX: ' + this.getColorStr().toUpperCase()).fontSize(16).margin({ top: 5 })
        Text('RGBA: ' + this.red + ', ' + this.green + ', ' + this.blue + ', ' + this.opacity + '%').fontSize(16).margin({ top: 5 })

        Divider().margin({ top: 15 })
        Text('10. 色卡�?0').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('11. 色卡�?1').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('12. 色卡�?2').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('13. 色卡�?3').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('BA68C8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('14. 色卡�?4').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('15. 色卡�?5').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('16. 色卡�?6').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('17. 色卡�?7').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('18. 色卡�?8').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('19. 色卡�?9').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('20. 色卡�?10').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('21. 色卡�?11').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('22. 色卡�?12').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('BA68C8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('23. 色卡�?13').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('24. 色卡�?14').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('25. 色卡�?15').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('26. 色卡�?16').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('27. 色卡�?17').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('28. 色卡�?18').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4); Text('FF7043').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('29. 色卡�?19').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('30. 色卡�?20').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('31. 色卡�?21').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('BA68C8').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('32. 色卡�?22').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('33. 色卡�?23').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('34. 色卡�?24').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('35. 日落 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('日落').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FFFF6B35', 0.0], ['#FFF7C59F', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('36. 海洋 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('海洋').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FF0077B6', 0.0], ['#FF90E0EF', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('37. 森林 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('森林').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FF2D6A4F', 0.0], ['#FF83C5BE', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('38. 紫粉 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('紫粉').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FF7209B7', 0.0], ['#FFFF006E', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('39. 极光 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('极光').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FF00C9FF', 0.0], ['#FF92FE9D', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('40. 火焰 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('火焰').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FFED2600', 0.0], ['#FFFA8072', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('41. 薄荷 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('薄荷').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FF00BA88', 0.0], ['#FF00F5A0', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('42. 樱花 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('樱花').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FFFF9A9E', 0.0], ['#FFFAD0C4', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('43. 星空 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('星空').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FF0F2027', 0.0], ['#FF2C5364', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('44. 琥珀 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('琥珀').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FFFFD200', 0.0], ['#FFF7971E', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('45. 晨曦 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('晨曦').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FFFF9A8B', 0.0], ['#FFFF6A88', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('46. 深海 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('深海').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FF001F3F', 0.0], ['#FF0074D9', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('47. 草地 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('草地').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FF56AB2F', 0.0], ['#FFA8E063', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('48. 夕阳 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('夕阳').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FFEE9CA7', 0.0], ['#FFFFDDE1', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('49. 冰川 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('冰川').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FF00B4DB', 0.0], ['#FF0083B0', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('50. 彩虹 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('彩虹').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FF9400D3', 0.0], ['#FF4B0082', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('51. 暖阳 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('暖阳').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FFFA709A', 0.0], ['#FFA1C4FD', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('52. 夜空 渐变').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Column() { Text('夜空').fontSize(12).fontColor('#FFFFFF').margin({ top: 5 }) }.width('100%').height(40).borderRadius(8).linearGradient({ angle: 90, colors: [['#FF232526', 0.0], ['#FF414345', 1.0]] }).margin({ bottom: 3 })

        Divider().margin({ top: 15 })
        Text('53. 随机色卡 0').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF16D462').borderRadius(8); Text('#FF16D462').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF16D462').fontSize(12); Text('RGB: 22, 212, 98').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(22 * 0.299 + 212 * 0.587 + 98 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('54. 随机色卡 1').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF299F8C').borderRadius(8); Text('#FF299F8C').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF299F8C').fontSize(12); Text('RGB: 41, 159, 140').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(41 * 0.299 + 159 * 0.587 + 140 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('55. 随机色卡 2').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF34E1E0').borderRadius(8); Text('#FF34E1E0').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF34E1E0').fontSize(12); Text('RGB: 52, 225, 224').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(52 * 0.299 + 225 * 0.587 + 224 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('56. 随机色卡 3').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF422284').borderRadius(8); Text('#FF422284').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF422284').fontSize(12); Text('RGB: 66, 34, 132').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(66 * 0.299 + 34 * 0.587 + 132 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('57. 随机色卡 4').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF2C9A18').borderRadius(8); Text('#FF2C9A18').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF2C9A18').fontSize(12); Text('RGB: 44, 154, 24').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(44 * 0.299 + 154 * 0.587 + 24 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('58. 随机色卡 5').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF249E1B').borderRadius(8); Text('#FF249E1B').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF249E1B').fontSize(12); Text('RGB: 36, 158, 27').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(36 * 0.299 + 158 * 0.587 + 27 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('59. 随机色卡 6').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFA0DA4F').borderRadius(8); Text('#FFA0DA4F').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFA0DA4F').fontSize(12); Text('RGB: 160, 218, 79').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(160 * 0.299 + 218 * 0.587 + 79 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('60. 随机色卡 7').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF9934DD').borderRadius(8); Text('#FF9934DD').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF9934DD').fontSize(12); Text('RGB: 153, 52, 221').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(153 * 0.299 + 52 * 0.587 + 221 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('61. 随机色卡 8').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFB8AE98').borderRadius(8); Text('#FFB8AE98').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFB8AE98').fontSize(12); Text('RGB: 184, 174, 152').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(184 * 0.299 + 174 * 0.587 + 152 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('62. 随机色卡 9').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF92568D').borderRadius(8); Text('#FF92568D').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF92568D').fontSize(12); Text('RGB: 146, 86, 141').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(146 * 0.299 + 86 * 0.587 + 141 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('63. 随机色卡 10').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFFCFE4C').borderRadius(8); Text('#FFFCFE4C').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFFCFE4C').fontSize(12); Text('RGB: 252, 254, 76').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(252 * 0.299 + 254 * 0.587 + 76 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('64. 随机色卡 11').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFFD05EF').borderRadius(8); Text('#FFFD05EF').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFFD05EF').fontSize(12); Text('RGB: 253, 5, 239').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(253 * 0.299 + 5 * 0.587 + 239 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('65. 随机色卡 12').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF894715').borderRadius(8); Text('#FF894715').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF894715').fontSize(12); Text('RGB: 137, 71, 21').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(137 * 0.299 + 71 * 0.587 + 21 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('66. 随机色卡 13').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFFA59D3').borderRadius(8); Text('#FFFA59D3').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFFA59D3').fontSize(12); Text('RGB: 250, 89, 211').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(250 * 0.299 + 89 * 0.587 + 211 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('67. 随机色卡 14').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF486FC8').borderRadius(8); Text('#FF486FC8').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF486FC8').fontSize(12); Text('RGB: 72, 111, 200').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(72 * 0.299 + 111 * 0.587 + 200 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('68. 随机色卡 15').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF158662').borderRadius(8); Text('#FF158662').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF158662').fontSize(12); Text('RGB: 21, 134, 98').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(21 * 0.299 + 134 * 0.587 + 98 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('69. 随机色卡 16').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF72404E').borderRadius(8); Text('#FF72404E').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF72404E').fontSize(12); Text('RGB: 114, 64, 78').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(114 * 0.299 + 64 * 0.587 + 78 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('70. 随机色卡 17').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFB71B38').borderRadius(8); Text('#FFB71B38').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFB71B38').fontSize(12); Text('RGB: 183, 27, 56').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(183 * 0.299 + 27 * 0.587 + 56 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('71. 随机色卡 18').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF8E6203').borderRadius(8); Text('#FF8E6203').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF8E6203').fontSize(12); Text('RGB: 142, 98, 3').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(142 * 0.299 + 98 * 0.587 + 3 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('72. 随机色卡 19').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF4E06CB').borderRadius(8); Text('#FF4E06CB').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF4E06CB').fontSize(12); Text('RGB: 78, 6, 203').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(78 * 0.299 + 6 * 0.587 + 203 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('73. 色块矩阵 0').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Grid() {
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF0000FF').borderRadius(4); Text('0000FF').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF00FFFF').borderRadius(4); Text('00FFFF').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFF00FF').borderRadius(4); Text('FF00FF').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFFA500').borderRadius(4); Text('FFA500').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF808080').borderRadius(4); Text('808080').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF000000').borderRadius(4); Text('000000').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFFFFFF').borderRadius(4); Text('FFFFFF').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFF1493').borderRadius(4); Text('FF1493').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF00CED1').borderRadius(4); Text('00CED1').fontSize(7).margin({ top: 2 }) } }
        }.columnsTemplate('1fr 1fr 1fr 1fr').rowsGap(8).columnsGap(3).width('100%').height(180)

        Divider().margin({ top: 15 })
        Text('74. 色块矩阵 1').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Grid() {
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF32CD32').borderRadius(4); Text('32CD32').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFF4500').borderRadius(4); Text('FF4500').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF9370DB').borderRadius(4); Text('9370DB').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF20B2AA').borderRadius(4); Text('20B2AA').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFF69B4').borderRadius(4); Text('FF69B4').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFCD5C5C').borderRadius(4); Text('CD5C5C').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF4169E1').borderRadius(4); Text('4169E1').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFDAA520').borderRadius(4); Text('DAA520').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFEF5350').borderRadius(4); Text('EF5350').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFE53935').borderRadius(4); Text('E53935').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFC62828').borderRadius(4); Text('C62828').fontSize(7).margin({ top: 2 }) } }
        }.columnsTemplate('1fr 1fr 1fr 1fr').rowsGap(8).columnsGap(3).width('100%').height(180)

        Divider().margin({ top: 15 })
        Text('75. 色块矩阵 2').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Grid() {
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFAB47BC').borderRadius(4); Text('BA68C8').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFFB300').borderRadius(4); Text('FFB300').fontSize(7).margin({ top: 2 }) } }
        }.columnsTemplate('1fr 1fr 1fr 1fr').rowsGap(8).columnsGap(3).width('100%').height(180)

        Divider().margin({ top: 15 })
        Text('76. 色块矩阵 3').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Grid() {
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFF8F00').borderRadius(4); Text('FF8F00').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF26C6DA').borderRadius(4); Text('26C6DA').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF00BCD4').borderRadius(4); Text('00BCD4').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF00ACC1').borderRadius(4); Text('00ACC1').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFEF9A9A').borderRadius(4); Text('EF9A9A').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFE57373').borderRadius(4); Text('E57373').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF90CAF9').borderRadius(4); Text('90CAF9').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFA5D6A7').borderRadius(4); Text('A5D6A7').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF81C784').borderRadius(4); Text('81C784').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFE0F2F1').borderRadius(4); Text('E0F2F1').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFB2DFDB').borderRadius(4); Text('B2DFDB').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF80CBC4').borderRadius(4); Text('80CBC4').fontSize(7).margin({ top: 2 }) } }
        }.columnsTemplate('1fr 1fr 1fr 1fr').rowsGap(8).columnsGap(3).width('100%').height(180)

        Divider().margin({ top: 15 })
        Text('77. 色块矩阵 4').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Grid() {
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF4DB6AC').borderRadius(4); Text('4DB6AC').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF26A69A').borderRadius(4); Text('26A69A').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF009688').borderRadius(4); Text('009688').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF00897B').borderRadius(4); Text('00897B').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFFECB3').borderRadius(4); Text('FFECB3').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFFE082').borderRadius(4); Text('FFE082').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFFD54F').borderRadius(4); Text('FFD54F').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFC5CAE9').borderRadius(4); Text('C5CAE9').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF9FA8DA').borderRadius(4); Text('9FA8DA').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF7986CB').borderRadius(4); Text('7986CB').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF5C6BC0').borderRadius(4); Text('5C6BC0').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFD7CCC8').borderRadius(4); Text('D7CCC8').fontSize(7).margin({ top: 2 }) } }
        }.columnsTemplate('1fr 1fr 1fr 1fr').rowsGap(8).columnsGap(3).width('100%').height(180)

        Divider().margin({ top: 15 })
        Text('78. 色块矩阵 5').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Grid() {
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFBCAAA4').borderRadius(4); Text('BCAAA4').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFA1887F').borderRadius(4); Text('A1887F').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF8D6E63').borderRadius(4); Text('8D6E63').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF795548').borderRadius(4); Text('795548').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF6D4C41').borderRadius(4); Text('6D4C41').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF5D4037').borderRadius(4); Text('5D4037').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFB2EBF2').borderRadius(4); Text('B2EBF2').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF80DEEA').borderRadius(4); Text('80DEEA').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FF4DD0E1').borderRadius(4); Text('4DD0E1').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFFCCBC').borderRadius(4); Text('FFCCBC').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFFAB91').borderRadius(4); Text('FFAB91').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFFF8A65').borderRadius(4); Text('FF8A65').fontSize(7).margin({ top: 2 }) } }
        }.columnsTemplate('1fr 1fr 1fr 1fr').rowsGap(8).columnsGap(3).width('100%').height(180)

        Divider().margin({ top: 15 })
        Text('50. 色卡�?0').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4)
            Text('FF0000').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4)
            Text('00FF00').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4)
            Text('0000FF').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4)
            Text('FFFF00').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4)
            Text('00FFFF').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4)
            Text('FF00FF').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4)
            Text('FFA500').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4)
            Text('808080').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('51. 色卡�?1').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4)
            Text('000000').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4)
            Text('FFFFFF').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4)
            Text('FF1493').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4)
            Text('00CED1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4)
            Text('32CD32').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4)
            Text('FF4500').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4)
            Text('9370DB').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4)
            Text('20B2AA').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('52. 色卡�?2').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4)
            Text('FF69B4').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4)
            Text('CD5C5C').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4)
            Text('4169E1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4)
            Text('DAA520').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4)
            Text('EF5350').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4)
            Text('E53935').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4)
            Text('C62828').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4)
            Text('C62828').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('53. 色卡�?3').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4)
            Text('AB47BC').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4)
            Text('AB47BC').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4)
            Text('BA68C8').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4)
            Text('64B5F6').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4)
            Text('42A5F5').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4)
            Text('1E88E5').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4)
            Text('66BB6A').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4)
            Text('43A047').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('54. 色卡�?4').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4)
            Text('388E3C').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4)
            Text('FFCA28').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4)
            Text('FFC107').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4)
            Text('FFB300').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4)
            Text('FF8F00').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4)
            Text('26C6DA').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4)
            Text('00BCD4').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4)
            Text('00ACC1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('55. 色卡�?5').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4)
            Text('EF9A9A').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4)
            Text('E57373').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4)
            Text('90CAF9').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4)
            Text('A5D6A7').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4)
            Text('81C784').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4)
            Text('E0F2F1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4)
            Text('B2DFDB').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4)
            Text('80CBC4').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('56. 色卡�?6').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4)
            Text('4DB6AC').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4)
            Text('26A69A').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4)
            Text('009688').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4)
            Text('00897B').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4)
            Text('FFECB3').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4)
            Text('FFE082').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4)
            Text('FFD54F').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4)
            Text('C5CAE9').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('57. 色卡�?7').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4)
            Text('9FA8DA').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4)
            Text('7986CB').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4)
            Text('5C6BC0').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4)
            Text('D7CCC8').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4)
            Text('BCAAA4').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4)
            Text('A1887F').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4)
            Text('8D6E63').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4)
            Text('795548').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('58. 色卡�?8').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4)
            Text('6D4C41').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4)
            Text('5D4037').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4)
            Text('B2EBF2').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4)
            Text('80DEEA').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4)
            Text('4DD0E1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4)
            Text('FFCCBC').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4)
            Text('FFAB91').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4)
            Text('FF8A65').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('59. 色卡�?9').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4)
            Text('FF7043').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4)
            Text('FFE0B2').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4)
            Text('FFD54F').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4)
            Text('FFC107').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4)
            Text('FFB300').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4)
            Text('4DB6AC').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4)
            Text('26A69A').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4)
            Text('00BCD4').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('60. 色卡�?10').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4)
            Text('00ACC1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4)
            Text('FF0000').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4)
            Text('00FF00').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4)
            Text('0000FF').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4)
            Text('FFFF00').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4)
            Text('00FFFF').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4)
            Text('FF00FF').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4)
            Text('FFA500').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('61. 色卡�?11').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4)
            Text('808080').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4)
            Text('000000').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4)
            Text('FFFFFF').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4)
            Text('FF1493').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4)
            Text('00CED1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4)
            Text('32CD32').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4)
            Text('FF4500').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4)
            Text('9370DB').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('62. 色卡�?12').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4)
            Text('20B2AA').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4)
            Text('FF69B4').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4)
            Text('CD5C5C').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4)
            Text('4169E1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4)
            Text('DAA520').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4)
            Text('EF5350').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4)
            Text('E53935').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4)
            Text('C62828').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('63. 色卡�?13').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4)
            Text('C62828').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4)
            Text('AB47BC').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4)
            Text('AB47BC').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4)
            Text('BA68C8').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4)
            Text('64B5F6').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4)
            Text('42A5F5').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4)
            Text('1E88E5').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4)
            Text('66BB6A').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('64. 色卡�?14').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4)
            Text('43A047').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4)
            Text('388E3C').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4)
            Text('FFCA28').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4)
            Text('FFC107').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4)
            Text('FFB300').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4)
            Text('FF8F00').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4)
            Text('26C6DA').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4)
            Text('00BCD4').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('65. 色卡�?15').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4)
            Text('00ACC1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4)
            Text('EF9A9A').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4)
            Text('E57373').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4)
            Text('90CAF9').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4)
            Text('A5D6A7').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4)
            Text('81C784').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4)
            Text('E0F2F1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4)
            Text('B2DFDB').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('66. 色卡�?16').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4)
            Text('80CBC4').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4)
            Text('4DB6AC').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4)
            Text('26A69A').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4)
            Text('009688').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4)
            Text('00897B').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4)
            Text('FFECB3').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4)
            Text('FFE082').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4)
            Text('FFD54F').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('67. 色卡�?17').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4)
            Text('C5CAE9').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4)
            Text('9FA8DA').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4)
            Text('7986CB').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4)
            Text('5C6BC0').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4)
            Text('D7CCC8').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4)
            Text('BCAAA4').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4)
            Text('A1887F').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4)
            Text('8D6E63').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('68. 色卡�?18').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4)
            Text('795548').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4)
            Text('6D4C41').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4)
            Text('5D4037').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4)
            Text('B2EBF2').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4)
            Text('80DEEA').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4)
            Text('4DD0E1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4)
            Text('FFCCBC').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4)
            Text('FFAB91').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('69. 色卡�?19').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF8A65').borderRadius(4)
            Text('FF8A65').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF7043').borderRadius(4)
            Text('FF7043').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFE0B2').borderRadius(4)
            Text('FFE0B2').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4)
            Text('FFD54F').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4)
            Text('FFC107').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4)
            Text('FFB300').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4)
            Text('4DB6AC').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4)
            Text('26A69A').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('70. 色卡�?20').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4)
            Text('00BCD4').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4)
            Text('00ACC1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF0000').borderRadius(4)
            Text('FF0000').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00FF00').borderRadius(4)
            Text('00FF00').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF0000FF').borderRadius(4)
            Text('0000FF').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFFF00').borderRadius(4)
            Text('FFFF00').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00FFFF').borderRadius(4)
            Text('00FFFF').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF00FF').borderRadius(4)
            Text('FF00FF').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('71. 色卡�?21').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFA500').borderRadius(4)
            Text('FFA500').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF808080').borderRadius(4)
            Text('808080').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF000000').borderRadius(4)
            Text('000000').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFFFFF').borderRadius(4)
            Text('FFFFFF').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF1493').borderRadius(4)
            Text('FF1493').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00CED1').borderRadius(4)
            Text('00CED1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF32CD32').borderRadius(4)
            Text('32CD32').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF4500').borderRadius(4)
            Text('FF4500').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('72. 色卡�?22').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF9370DB').borderRadius(4)
            Text('9370DB').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF20B2AA').borderRadius(4)
            Text('20B2AA').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF69B4').borderRadius(4)
            Text('FF69B4').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFCD5C5C').borderRadius(4)
            Text('CD5C5C').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF4169E1').borderRadius(4)
            Text('4169E1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFDAA520').borderRadius(4)
            Text('DAA520').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFEF5350').borderRadius(4)
            Text('EF5350').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFE53935').borderRadius(4)
            Text('E53935').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('73. 色卡�?23').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4)
            Text('C62828').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4)
            Text('C62828').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4)
            Text('AB47BC').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4)
            Text('AB47BC').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4)
            Text('BA68C8').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4)
            Text('64B5F6').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4)
            Text('42A5F5').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4)
            Text('1E88E5').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('74. 色卡�?24').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4)
            Text('66BB6A').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4)
            Text('43A047').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4)
            Text('388E3C').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4)
            Text('FFCA28').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4)
            Text('FFC107').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFB300').borderRadius(4)
            Text('FFB300').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFF8F00').borderRadius(4)
            Text('FF8F00').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF26C6DA').borderRadius(4)
            Text('26C6DA').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('75. 色卡�?25').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00BCD4').borderRadius(4)
            Text('00BCD4').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00ACC1').borderRadius(4)
            Text('00ACC1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFEF9A9A').borderRadius(4)
            Text('EF9A9A').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFE57373').borderRadius(4)
            Text('E57373').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF90CAF9').borderRadius(4)
            Text('90CAF9').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFA5D6A7').borderRadius(4)
            Text('A5D6A7').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF81C784').borderRadius(4)
            Text('81C784').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFE0F2F1').borderRadius(4)
            Text('E0F2F1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('76. 色卡�?26').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FFB2DFDB').borderRadius(4)
            Text('B2DFDB').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF80CBC4').borderRadius(4)
            Text('80CBC4').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF4DB6AC').borderRadius(4)
            Text('4DB6AC').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF26A69A').borderRadius(4)
            Text('26A69A').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF009688').borderRadius(4)
            Text('009688').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF00897B').borderRadius(4)
            Text('00897B').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFECB3').borderRadius(4)
            Text('FFECB3').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFE082').borderRadius(4)
            Text('FFE082').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('77. 色卡�?27').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFD54F').borderRadius(4)
            Text('FFD54F').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFC5CAE9').borderRadius(4)
            Text('C5CAE9').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF9FA8DA').borderRadius(4)
            Text('9FA8DA').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF7986CB').borderRadius(4)
            Text('7986CB').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF5C6BC0').borderRadius(4)
            Text('5C6BC0').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFD7CCC8').borderRadius(4)
            Text('D7CCC8').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFBCAAA4').borderRadius(4)
            Text('BCAAA4').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFA1887F').borderRadius(4)
            Text('A1887F').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('78. 色卡�?28').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FF8D6E63').borderRadius(4)
            Text('8D6E63').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF795548').borderRadius(4)
            Text('795548').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF6D4C41').borderRadius(4)
            Text('6D4C41').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF5D4037').borderRadius(4)
            Text('5D4037').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFB2EBF2').borderRadius(4)
            Text('B2EBF2').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF80DEEA').borderRadius(4)
            Text('80DEEA').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FF4DD0E1').borderRadius(4)
            Text('4DD0E1').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFCCBC').borderRadius(4)
            Text('FFCCBC').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('79. 色卡�?29').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() {
            Column().width(35).height(35).backgroundColor('#FFFFAB91').borderRadius(4)
            Text('FFAB91').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
        Divider().margin({ top: 15 })
        Text('300. 色卡300').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Divider().margin({ top: 15, bottom: 30 })
      }
      .width('100%').padding(20)
    }
    .width('100%').height('100%').backgroundColor('#F5F5F5')
  }
}
