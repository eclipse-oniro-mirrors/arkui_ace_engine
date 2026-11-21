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
 * ColorPicker with Image color overlay test example
 */
@Entry
@Component
struct ColorPickerOverlayExample {
  @State red: number = 33
  @State green: number = 150
  @State blue: number = 243
  @State overlayAlpha: number = 50
  @State resultText: string = ''

  private toHex(value: number): string {
    let hex = value.toString(16)
    return hex.length === 1 ? '0' + hex : hex
  }

  private getColorStr(): string {
    return '#FF' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)
  }

  private getOverlayColor(): string {
    let a = Math.round(this.overlayAlpha * 255 / 100)
    return '#' + this.toHex(a) + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)
  }

  build() {
    Scroll() {
      Column() {
        Text('ColorPicker 棰滆壊鍙犲姞娴嬭�?)
          .fontSize(22).fontWeight(FontWeight.Bold).margin({ top: 30, bottom: 10 })

        Stack() {
          Image($r('app.media.startIcon')).width(150).height(150).borderRadius(12)
          Column().width(150).height(150).borderRadius(12).backgroundColor(this.getOverlayColor())
        }
        .width(150).height(150).alignContent(Alignment.Center)

        Column() {
          Text('鍙犲姞鑹? ' + this.getColorStr().toUpperCase()).fontSize(14).fontColor('#FFFFFF').margin({ top: 5 })
          Text('涓嶉€忔槑搴? ' + this.overlayAlpha + '%').fontSize(14).fontColor('#FFFFFF').margin({ top: 5, bottom: 5 })
        }
        .width('100%').height(60).backgroundColor(this.getColorStr())
        .borderRadius(8).justifyContent(FlexAlign.Center)

        Text('RGB棰滆壊璋冭妭').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() {
          Text('R').fontSize(16).fontWeight(FontWeight.Bold).width(30).fontColor('#F44336')
          Slider({ value: this.red, min: 0, max: 255, step: 1 })
            .layoutWeight(1).blockColor('#F44336').trackColor('#E0E0E0').selectedColor('#F44336')
            .onChange((value: number) => { this.red = Math.round(value); this.resultText = 'R=' + Math.round(value) })
          Text(this.red.toString()).fontSize(12).width(35).textAlign(TextAlign.End)
        }.width('100%')
        Row() {
          Text('G').fontSize(16).fontWeight(FontWeight.Bold).width(30).fontColor('#4CAF50')
          Slider({ value: this.green, min: 0, max: 255, step: 1 })
            .layoutWeight(1).blockColor('#4CAF50').trackColor('#E0E0E0').selectedColor('#4CAF50')
            .onChange((value: number) => { this.green = Math.round(value); this.resultText = 'G=' + Math.round(value) })
          Text(this.green.toString()).fontSize(12).width(35).textAlign(TextAlign.End)
        }.width('100%')
        Row() {
          Text('B').fontSize(16).fontWeight(FontWeight.Bold).width(30).fontColor('#2196F3')
          Slider({ value: this.blue, min: 0, max: 255, step: 1 })
            .layoutWeight(1).blockColor('#2196F3').trackColor('#E0E0E0').selectedColor('#2196F3')
            .onChange((value: number) => { this.blue = Math.round(value); this.resultText = 'B=' + Math.round(value) })
          Text(this.blue.toString()).fontSize(12).width(35).textAlign(TextAlign.End)
        }.width('100%')

        Text('鍙犲姞涓嶉€忔槑�?).fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() {
          Text('A').fontSize(16).fontWeight(FontWeight.Bold).width(30).fontColor('#999999')
          Slider({ value: this.overlayAlpha, min: 0, max: 100, step: 1 })
            .layoutWeight(1).blockColor('#999999').trackColor('#E0E0E0').selectedColor('#999999')
            .onChange((value: number) => { this.overlayAlpha = Math.round(value); this.resultText = 'Alpha=' + Math.round(value) + '%' })
          Text(this.overlayAlpha + '%').fontSize(12).width(40).textAlign(TextAlign.End)
        }.width('100%')

        Text(this.resultText).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        Text('涓嶅悓涓嶉€忔槑搴﹀姣?).fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() {
          Column() {
            Text('0%').fontSize(10).margin({ bottom: 3 })
            Stack() { Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8); Column().width(60).height(60).borderRadius(8).backgroundColor('#00' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)) }.alignContent(Alignment.Center)
          }.margin({ right: 5 })
          Column() {
            Text('25%').fontSize(10).margin({ bottom: 3 })
            Stack() { Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8); Column().width(60).height(60).borderRadius(8).backgroundColor('#40' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)) }.alignContent(Alignment.Center)
          }.margin({ right: 5 })
          Column() {
            Text('50%').fontSize(10).margin({ bottom: 3 })
            Stack() { Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8); Column().width(60).height(60).borderRadius(8).backgroundColor('#80' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)) }.alignContent(Alignment.Center)
          }.margin({ right: 5 })
          Column() {
            Text('75%').fontSize(10).margin({ bottom: 3 })
            Stack() { Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8); Column().width(60).height(60).borderRadius(8).backgroundColor('#C0' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)) }.alignContent(Alignment.Center)
          }.margin({ right: 5 })
          Column() {
            Text('100%').fontSize(10).margin({ bottom: 3 })
            Stack() { Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8); Column().width(60).height(60).borderRadius(8).backgroundColor('#FF' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)) }.alignContent(Alignment.Center)
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('棰勮棰滆壊鍙犲�?).fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Row() {
          Column() { Text('绾㈣�?).fontSize(10).margin({ bottom: 3 }); Stack() { Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8); Column().width(60).height(60).borderRadius(8).backgroundColor('#80FF0000') }.alignContent(Alignment.Center) }.margin({ right: 5 })
          Column() { Text('缁胯�?).fontSize(10).margin({ bottom: 3 }); Stack() { Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8); Column().width(60).height(60).borderRadius(8).backgroundColor('#8000FF00') }.alignContent(Alignment.Center) }.margin({ right: 5 })
          Column() { Text('钃濊�?).fontSize(10).margin({ bottom: 3 }); Stack() { Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8); Column().width(60).height(60).borderRadius(8).backgroundColor('#800000FF') }.alignContent(Alignment.Center) }.margin({ right: 5 })
          Column() { Text('榛勮�?).fontSize(10).margin({ bottom: 3 }); Stack() { Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8); Column().width(60).height(60).borderRadius(8).backgroundColor('#80FFFF00') }.alignContent(Alignment.Center) }.margin({ right: 5 })
          Column() { Text('绱�?).fontSize(10).margin({ bottom: 3 }); Stack() { Image($r('app.media.startIcon')).width(60).height(60).borderRadius(8); Column().width(60).height(60).borderRadius(8).backgroundColor('#80FF00FF') }.alignContent(Alignment.Center) }
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
        Text('鍙犲姞鑹睭EX: ' + this.getOverlayColor().toUpperCase()).fontSize(16).margin({ top: 5 })
        Text('RGB: ' + this.red + ', ' + this.green + ', ' + this.blue).fontSize(16).margin({ top: 5 })
        Text('Alpha: ' + this.overlayAlpha + '%').fontSize(16).margin({ top: 5 })

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
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
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
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
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
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
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

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFD63885').borderRadius(8); Text('#FFD63885').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFD63885').fontSize(12); Text('RGB: 214, 56, 133').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(214 * 0.299 + 56 * 0.587 + 133 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('54. 随机色卡 1').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFDCC161').borderRadius(8); Text('#FFDCC161').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFDCC161').fontSize(12); Text('RGB: 220, 193, 97').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(220 * 0.299 + 193 * 0.587 + 97 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('55. 随机色卡 2').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF9C8FCA').borderRadius(8); Text('#FF9C8FCA').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF9C8FCA').fontSize(12); Text('RGB: 156, 143, 202').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(156 * 0.299 + 143 * 0.587 + 202 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('56. 随机色卡 3').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFA88B5D').borderRadius(8); Text('#FFA88B5D').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFA88B5D').fontSize(12); Text('RGB: 168, 139, 93').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(168 * 0.299 + 139 * 0.587 + 93 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('57. 随机色卡 4').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFBADCC8').borderRadius(8); Text('#FFBADCC8').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFBADCC8').fontSize(12); Text('RGB: 186, 220, 200').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(186 * 0.299 + 220 * 0.587 + 200 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('58. 随机色卡 5').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFE546CF').borderRadius(8); Text('#FFE546CF').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFE546CF').fontSize(12); Text('RGB: 229, 70, 207').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(229 * 0.299 + 70 * 0.587 + 207 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('59. 随机色卡 6').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF1D6402').borderRadius(8); Text('#FF1D6402').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF1D6402').fontSize(12); Text('RGB: 29, 100, 2').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(29 * 0.299 + 100 * 0.587 + 2 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('60. 随机色卡 7').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFF20979').borderRadius(8); Text('#FFF20979').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFF20979').fontSize(12); Text('RGB: 242, 9, 121').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(242 * 0.299 + 9 * 0.587 + 121 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('61. 随机色卡 8').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF8AC620').borderRadius(8); Text('#FF8AC620').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF8AC620').fontSize(12); Text('RGB: 138, 198, 32').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(138 * 0.299 + 198 * 0.587 + 32 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('62. 随机色卡 9').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF2DCCEF').borderRadius(8); Text('#FF2DCCEF').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF2DCCEF').fontSize(12); Text('RGB: 45, 204, 239').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(45 * 0.299 + 204 * 0.587 + 239 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('63. 随机色卡 10').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFED5B0B').borderRadius(8); Text('#FFED5B0B').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFED5B0B').fontSize(12); Text('RGB: 237, 91, 11').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(237 * 0.299 + 91 * 0.587 + 11 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('64. 随机色卡 11').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF1F97CB').borderRadius(8); Text('#FF1F97CB').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF1F97CB').fontSize(12); Text('RGB: 31, 151, 203').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(31 * 0.299 + 151 * 0.587 + 203 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('65. 随机色卡 12').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF5F2899').borderRadius(8); Text('#FF5F2899').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF5F2899').fontSize(12); Text('RGB: 95, 40, 153').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(95 * 0.299 + 40 * 0.587 + 153 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('66. 随机色卡 13').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFAB47BC').borderRadius(8); Text('#FFAB47BC').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFAB47BC').fontSize(12); Text('RGB: 173, 143, 181').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(173 * 0.299 + 143 * 0.587 + 181 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('67. 随机色卡 14').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFB545E3').borderRadius(8); Text('#FFB545E3').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFB545E3').fontSize(12); Text('RGB: 181, 69, 227').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(181 * 0.299 + 69 * 0.587 + 227 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('68. 随机色卡 15').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF2AF53E').borderRadius(8); Text('#FF2AF53E').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF2AF53E').fontSize(12); Text('RGB: 42, 245, 62').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(42 * 0.299 + 245 * 0.587 + 62 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('69. 随机色卡 16').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF050C93').borderRadius(8); Text('#FF050C93').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF050C93').fontSize(12); Text('RGB: 5, 12, 147').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(5 * 0.299 + 12 * 0.587 + 147 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('70. 随机色卡 17').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFA68547').borderRadius(8); Text('#FFA68547').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFA68547').fontSize(12); Text('RGB: 166, 133, 71').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(166 * 0.299 + 133 * 0.587 + 71 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('71. 随机色卡 18').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FFFE4F8C').borderRadius(8); Text('#FFFE4F8C').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FFFE4F8C').fontSize(12); Text('RGB: 254, 79, 140').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(254 * 0.299 + 79 * 0.587 + 140 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

        Divider().margin({ top: 15 })
        Text('72. 随机色卡 19').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() { Column() { Column().width(50).height(50).backgroundColor('#FF2D98CF').borderRadius(8); Text('#FF2D98CF').fontSize(8).margin({ top: 3 }) }.margin({ right: 10 }); Column() { Text('HEX: #FF2D98CF').fontSize(12); Text('RGB: 45, 152, 207').fontSize(12).margin({ top: 3 }); Text('亮度: ' + Math.round(45 * 0.299 + 152 * 0.587 + 207 * 0.114)).fontSize(12).margin({ top: 3 }) }.alignItems(HorizontalAlign.Start).layoutWeight(1) }.width('100%')

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
          GridItem() { Column() { Column().width(30).height(30).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) } }
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
        Text('200. 色卡�?0').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('201. 色卡�?1').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('202. 色卡�?2').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('203. 色卡�?4').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('204. 色卡�?5').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('205. 色卡�?7').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('206. 色卡�?8').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('207. 色卡�?9').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('208. 色卡�?11').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('209. 色卡�?12').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('210. 色卡�?14').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('211. 色卡�?15').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('212. 色卡�?16').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('213. 色卡�?18').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('214. 色卡�?19').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('215. 色卡�?21').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('216. 色卡�?22').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('217. 色卡�?23').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('218. 色卡�?25').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('219. 色卡�?26').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('220. 色卡�?28').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('221. 色卡�?29').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('222. 色卡�?30').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('223. 色卡�?32').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('224. 色卡�?33').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('225. 色卡�?35').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('226. 色卡�?36').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('227. 色卡�?37').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('228. 色卡�?39').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('229. 色卡�?40').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('230. 色卡�?42').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('231. 色卡�?43').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('232. 色卡�?44').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('233. 色卡�?46').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('234. 色卡�?47').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('235. 色卡�?49').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('236. 色卡�?50').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('237. 色卡�?51').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('238. 色卡�?53').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('239. 色卡�?54').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('240. 色卡�?56').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('241. 色卡�?57').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('242. 色卡�?58').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('243. 色卡�?60').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('244. 色卡�?61').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('245. 色卡�?63').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('246. 色卡�?64').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('247. 色卡�?65').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('248. 色卡�?67').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('249. 色卡�?68').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('250. 色卡�?70').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('251. 色卡�?71').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('252. 色卡�?72').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('253. 色卡�?74').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('254. 色卡�?75').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('255. 色卡�?77').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('256. 色卡�?78').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('257. 色卡�?79').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('258. 色卡�?81').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('259. 色卡�?82').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('260. 色卡�?84').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('261. 色卡�?85').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('262. 色卡�?86').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('263. 色卡�?88').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('264. 色卡�?89').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('265. 色卡�?91').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('266. 色卡�?92').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('267. 色卡�?93').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('268. 色卡�?95').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('269. 色卡�?96').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('270. 色卡�?98').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('271. 色卡�?99').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('272. 色卡�?100').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('273. 色卡�?102').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('274. 色卡�?103').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFAB47BC').borderRadius(4); Text('AB47BC').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF64B5F6').borderRadius(4); Text('64B5F6').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF42A5F5').borderRadius(4); Text('42A5F5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF1E88E5').borderRadius(4); Text('1E88E5').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF66BB6A').borderRadius(4); Text('66BB6A').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FF43A047').borderRadius(4); Text('43A047').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('275. 色卡�?105').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('276. 色卡�?106').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('277. 色卡�?107').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('278. 色卡�?109').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

        Row() {
          Column() { Column().width(35).height(35).backgroundColor('#FF388E3C').borderRadius(4); Text('388E3C').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFCA28').borderRadius(4); Text('FFCA28').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
          Column() { Column().width(35).height(35).backgroundColor('#FFFFC107').borderRadius(4); Text('FFC107').fontSize(7).margin({ top: 2 }) }.margin({ right: 3 })
        Divider().margin({ top: 15 })
        Text('300. 色卡300').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Divider().margin({ top: 15, bottom: 30 })
      }
      .width('100%').padding(20)
    }
    .width('100%').height('100%').backgroundColor('#F5F5F5')
  }
}
