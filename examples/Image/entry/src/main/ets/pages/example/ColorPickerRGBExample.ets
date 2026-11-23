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
 * ColorPicker RGB sliders test example
 */
@Entry
@Component
struct ColorPickerRGBExample {
  @State red: number = 128
  @State green: number = 128
  @State blue: number = 128
  @State alpha: number = 255
  @State resultText: string = ''
  @State fontSize: number = 18
  @State colorHistory: string[] = ['#FF2196F3', '#FF4CAF50', '#FFF44336', '#FFFF9800', '#FF9C27B0', '#FF00BCD4']
  @State selectedPreset: number = 0

  private toHex(value: number): string {
    let hex = value.toString(16)
    return hex.length === 1 ? '0' + hex : hex
  }

  private getColorStr(): string {
    return '#FF' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)
  }

  private getColorWithAlpha(): string {
    return '#' + this.toHex(this.alpha) + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)
  }

  private getComplementaryColor(): string {
    return '#FF' + this.toHex(255 - this.red) + this.toHex(255 - this.green) + this.toHex(255 - this.blue)
  }

  private getBrightness(): number {
    return Math.round((this.red * 0.299 + this.green * 0.587 + this.blue * 0.114) / 255 * 100)
  }

  private getWarmCool(): string {
    let warmth = this.red - this.blue
    if (warmth > 50) return '鏆栬壊璋?
    if (warmth < -50) return '鍐疯壊璋?
    return '涓€ц壊�?
  }

  private getHsvStr(): string {
    let redRatio = this.red / 255
    let greenRatio = this.green / 255
    let blueRatio = this.blue / 255
    let max = Math.max(redRatio, greenRatio, blueRatio)
    let min = Math.min(redRatio, greenRatio, blueRatio)
    let d = max - min
    let h = 0
    if (d !== 0) {
      if (max === redRatio) { h = ((greenRatio - blueRatio) / d) % 6 }
      else if (max === greenRatio) { h = (blueRatio - redRatio) / d + 2 }
      else { h = (redRatio - greenRatio) / d + 4 }
      h = Math.round(h * 60)
      if (h < 0) { h += 360 }
    }
    let s = max === 0 ? 0 : Math.round(d / max * 100)
    let v = Math.round(max * 100)
    return 'HSV(' + h + ', ' + s + '%, ' + v + '%)'
  }

  private grayToHex(gray: number): string {
    let h = gray.toString(16)
    let hh = h.length === 1 ? '0' + h : h
    return '#FF' + hh + hh + hh
  }

  private getColorName(): string {
    let r = this.red
    let g = this.green
    let b = this.blue
    if (r > 200 && g < 60 && b < 60) return '绾㈣�?
    if (r < 60 && g > 200 && b < 60) return '缁胯�?
    if (r < 60 && g < 60 && b > 200) return '钃濊�?
    if (r > 200 && g > 200 && b < 60) return '榛勮�?
    if (r < 60 && g > 200 && b > 200) return '闈掕�?
    if (r > 200 && g < 60 && b > 200) return '绱�?
    if (r > 200 && g > 100 && b < 60) return '姗欒�?
    if (r > 240 && g > 240 && b > 240) return '鐧借壊'
    if (r < 30 && g < 30 && b < 30) return '榛戣�?
    if (r > 100 && r < 200 && g > 100 && g < 200 && b > 100 && b < 200) return '鐏拌�?
    return '娣峰悎鑹?
  }

  @Builder
  sectionTitle(title: string) {
    Text(title).fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
  }

  @Builder
  colorBlock(hex: string, label: string, w: number, h: number) {
    Column() {
      Column().width(w).height(h).backgroundColor(hex).borderRadius(8)
      Text(label).fontSize(10).margin({ top: 3 })
    }
  }

  build() {
    Scroll() {
      Column() {
        Text('ColorPicker RGB 婊戝潡娴嬭瘯')
          .fontSize(22).fontWeight(FontWeight.Bold).margin({ top: 30, bottom: 10 })

        Text('閫氳繃RGBA婊戝潡璋冭妭棰滆壊锛屽疄鏃堕瑙堟晥�?)
          .fontSize(14).fontColor('#999999').margin({ bottom: 10 })

        this.sectionTitle('1. 棰滆壊棰勮�?)

        Column() {
          Text('褰撳墠棰滆壊').fontSize(14).fontColor('#FFFFFF').margin({ top: 10 })
          Text(this.getColorStr().toUpperCase()).fontSize(16).fontColor('#FFFFFF').margin({ top: 5 })
          Text(this.getColorName()).fontSize(14).fontColor('#FFFFFF').margin({ top: 5, bottom: 10 })
        }
        .width('100%').height(100).backgroundColor(this.getColorStr())
        .borderRadius(8).justifyContent(FlexAlign.Center)

        Divider().margin({ top: 15 })
        this.sectionTitle('2. R 绾㈣壊閫氶亾')

        Row() {
          Text('R').fontSize(16).fontWeight(FontWeight.Bold).width(30).fontColor('#F44336')
          Slider({ value: this.red, min: 0, max: 255, step: 1 })
            .layoutWeight(1).blockColor('#F44336').trackColor('#E0E0E0').selectedColor('#F44336')
            .onChange((value: number) => { this.red = Math.round(value); this.resultText = 'R=' + this.red })
          Text(this.red.toString()).fontSize(14).width(40).textAlign(TextAlign.End)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('3. G 缁胯壊閫氶亾')

        Row() {
          Text('G').fontSize(16).fontWeight(FontWeight.Bold).width(30).fontColor('#4CAF50')
          Slider({ value: this.green, min: 0, max: 255, step: 1 })
            .layoutWeight(1).blockColor('#4CAF50').trackColor('#E0E0E0').selectedColor('#4CAF50')
            .onChange((value: number) => { this.green = Math.round(value); this.resultText = 'G=' + this.green })
          Text(this.green.toString()).fontSize(14).width(40).textAlign(TextAlign.End)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('4. B 钃濊壊閫氶亾')

        Row() {
          Text('B').fontSize(16).fontWeight(FontWeight.Bold).width(30).fontColor('#2196F3')
          Slider({ value: this.blue, min: 0, max: 255, step: 1 })
            .layoutWeight(1).blockColor('#2196F3').trackColor('#E0E0E0').selectedColor('#2196F3')
            .onChange((value: number) => { this.blue = Math.round(value); this.resultText = 'B=' + this.blue })
          Text(this.blue.toString()).fontSize(14).width(40).textAlign(TextAlign.End)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('5. A 閫忔槑搴﹂€氶亾')

        Row() {
          Text('A').fontSize(16).fontWeight(FontWeight.Bold).width(30).fontColor('#999999')
          Slider({ value: this.alpha, min: 0, max: 255, step: 1 })
            .layoutWeight(1).blockColor('#999999').trackColor('#E0E0E0').selectedColor('#999999')
            .onChange((value: number) => { this.alpha = Math.round(value); this.resultText = 'A=' + this.alpha })
          Text(this.alpha.toString()).fontSize(14).width(40).textAlign(TextAlign.End)
        }.width('100%')

        Text(this.resultText).fontSize(13).fontColor('#007DFF').margin({ top: 10 })

        Divider().margin({ top: 15 })
        this.sectionTitle('6. 棰滆壊鍊煎鏍煎紡杈撳嚭')

        Text('HEX: ' + this.getColorStr().toUpperCase()).fontSize(16).margin({ top: 5 })
        Text('HEX (鍚獳lpha): ' + this.getColorWithAlpha().toUpperCase()).fontSize(16).margin({ top: 5 })
        Text('RGB: rgb(' + this.red + ', ' + this.green + ', ' + this.blue + ')').fontSize(16).margin({ top: 5 })
        Text('RGBA: rgba(' + this.red + ', ' + this.green + ', ' + this.blue + ', ' + (this.alpha / 255).toFixed(2) + ')').fontSize(16).margin({ top: 5 })
        Text(this.getHsvStr()).fontSize(16).margin({ top: 5 })
        Text('棰滆壊鍚嶇�? ' + this.getColorName()).fontSize(16).margin({ top: 5 })
        Text('浜�? ' + this.getBrightness() + '%').fontSize(16).margin({ top: 5 })
        Text('鑹叉�? ' + this.getWarmCool()).fontSize(16).margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('7. 閫忔槑搴﹀姣斿睍绀?)

        Row() {
          this.colorBlock('#FF' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue), 'alpha=255', 60, 60)
          Column() { }.width(10)
          this.colorBlock('#C0' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue), 'alpha=192', 60, 60)
          Column() { }.width(10)
          this.colorBlock('#80' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue), 'alpha=128', 60, 60)
          Column() { }.width(10)
          this.colorBlock('#40' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue), 'alpha=64', 60, 60)
          Column() { }.width(10)
          this.colorBlock('#00' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue), 'alpha=0', 60, 60)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('8. 浜掕ˉ鑹插睍绀?)

        Row() {
          Column() {
            Text('鍘熻�?).fontSize(12).margin({ bottom: 5 })
            Column().width(80).height(80).backgroundColor(this.getColorStr()).borderRadius(8)
            Text(this.getColorStr().toUpperCase()).fontSize(10).margin({ top: 3 })
          }.margin({ right: 20 })
          Column() {
            Text('浜掕ˉ�?).fontSize(12).margin({ bottom: 5 })
            Column().width(80).height(80).backgroundColor(this.getComplementaryColor()).borderRadius(8)
            Text(this.getComplementaryColor().toUpperCase()).fontSize(10).margin({ top: 3 })
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('9. RGB鍒嗛噺瀵规�?)

        Row() {
          Column() {
            Text('R=' + this.red).fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF' + this.toHex(this.red) + '0000').borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('G=' + this.green).fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF00' + this.toHex(this.green) + '00').borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('B=' + this.blue).fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF0000' + this.toHex(this.blue)).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('A=' + this.alpha).fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor(this.getColorWithAlpha()).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('鍚堟�?).fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor(this.getColorStr()).borderRadius(8)
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('10. 棰勮棰滆壊蹇嵎閫夋嫨')

        Row() {
          Button('绾㈣�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 255; this.green = 0; this.blue = 0; this.alpha = 255 })
          Button('缁胯�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 0; this.green = 255; this.blue = 0; this.alpha = 255 })
          Button('钃濊�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 0; this.green = 0; this.blue = 255; this.alpha = 255 })
          Button('鐧借壊').fontSize(12).height(30).margin(2).onClick(() => { this.red = 255; this.green = 255; this.blue = 255; this.alpha = 255 })
          Button('榛戣�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 0; this.green = 0; this.blue = 0; this.alpha = 255 })
        }.width('100%')

        Row() {
          Button('榛勮�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 255; this.green = 255; this.blue = 0; this.alpha = 255 })
          Button('闈掕�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 0; this.green = 255; this.blue = 255; this.alpha = 255 })
          Button('绱�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 255; this.green = 0; this.blue = 255; this.alpha = 255 })
          Button('姗欒�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 255; this.green = 165; this.blue = 0; this.alpha = 255 })
          Button('鐏拌�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 128; this.green = 128; this.blue = 128; this.alpha = 255 })
        }.width('100%').margin({ top: 5 })

        Row() {
          Button('娣辩�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 139; this.green = 0; this.blue = 0; this.alpha = 255 })
          Button('娣辩�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 0; this.green = 100; this.blue = 0; this.alpha = 255 })
          Button('娣辫�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 0; this.green = 0; this.blue = 139; this.alpha = 255 })
          Button('绮夎�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 255; this.green = 192; this.blue = 203; this.alpha = 255 })
          Button('妫曡�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 165; this.green = 42; this.blue = 42; this.alpha = 255 })
        }.width('100%').margin({ top: 5 })

        Row() {
          Button('闈涜�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 75; this.green = 0; this.blue = 130; this.alpha = 255 })
          Button('钖勮�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 152; this.green = 255; this.blue = 152; this.alpha = 255 })
          Button('鐝婄�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 255; this.green = 127; this.blue = 80; this.alpha = 255 })
          Button('姗勬�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 128; this.green = 128; this.blue = 0; this.alpha = 255 })
          Button('闈掓�?).fontSize(12).height(30).margin(2).onClick(() => { this.red = 50; this.green = 205; this.blue = 50; this.alpha = 255 })
        }.width('100%').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('11. 鍩虹鑹叉澘')

        Grid() {
          GridItem() { Column() { Column().width(50).height(50).backgroundColor('#FFFF0000').borderRadius(8).onClick(() => { this.red = 255; this.green = 0; this.blue = 0 }); Text('绾㈣�?).fontSize(10).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(50).height(50).backgroundColor('#FF00FF00').borderRadius(8).onClick(() => { this.red = 0; this.green = 255; this.blue = 0 }); Text('缁胯�?).fontSize(10).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(50).height(50).backgroundColor('#FF0000FF').borderRadius(8).onClick(() => { this.red = 0; this.green = 0; this.blue = 255 }); Text('钃濊�?).fontSize(10).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(50).height(50).backgroundColor('#FFFFFF00').borderRadius(8).onClick(() => { this.red = 255; this.green = 255; this.blue = 0 }); Text('榛勮�?).fontSize(10).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(50).height(50).backgroundColor('#FFFF00FF').borderRadius(8).onClick(() => { this.red = 255; this.green = 0; this.blue = 255 }); Text('绱�?).fontSize(10).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(50).height(50).backgroundColor('#FF00FFFF').borderRadius(8).onClick(() => { this.red = 0; this.green = 255; this.blue = 255 }); Text('闈掕�?).fontSize(10).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(50).height(50).backgroundColor('#FFFFA500').borderRadius(8).onClick(() => { this.red = 255; this.green = 165; this.blue = 0 }); Text('姗欒�?).fontSize(10).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(50).height(50).backgroundColor('#FFFFFFFF').borderRadius(8).border({ width: 1, color: '#CCC' }).onClick(() => { this.red = 255; this.green = 255; this.blue = 255 }); Text('鐧借壊').fontSize(10).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(50).height(50).backgroundColor('#FF000000').borderRadius(8).onClick(() => { this.red = 0; this.green = 0; this.blue = 0 }); Text('榛戣�?).fontSize(10).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(50).height(50).backgroundColor('#FF808080').borderRadius(8).onClick(() => { this.red = 128; this.green = 128; this.blue = 128 }); Text('鐏拌�?).fontSize(10).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(50).height(50).backgroundColor('#FFFFC0CB').borderRadius(8).onClick(() => { this.red = 255; this.green = 192; this.blue = 203 }); Text('绮夎�?).fontSize(10).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(50).height(50).backgroundColor('#FFA52A2A').borderRadius(8).onClick(() => { this.red = 165; this.green = 42; this.blue = 42 }); Text('妫曡�?).fontSize(10).margin({ top: 3 }) } }
        }
        .columnsTemplate('1fr 1fr 1fr 1fr').rowsGap(15).columnsGap(10).width('100%').height(420)

        Divider().margin({ top: 15 })
        this.sectionTitle('12. Material Design 鑹叉�?)

        Grid() {
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FFE53935').borderRadius(8).onClick(() => { this.red = 229; this.green = 57; this.blue = 53 }); Text('Red').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FFE91E63').borderRadius(8).onClick(() => { this.red = 216; this.green = 27; this.blue = 96 }); Text('Pink').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FF8E24AA').borderRadius(8).onClick(() => { this.red = 142; this.green = 36; this.blue = 170 }); Text('Purple').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FF5E35B1').borderRadius(8).onClick(() => { this.red = 94; this.green = 53; this.blue = 177 }); Text('DeepP').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FF3949AB').borderRadius(8).onClick(() => { this.red = 57; this.green = 73; this.blue = 171 }); Text('Indigo').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FF1E88E5').borderRadius(8).onClick(() => { this.red = 30; this.green = 136; this.blue = 229 }); Text('Blue').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FF039BE5').borderRadius(8).onClick(() => { this.red = 3; this.green = 155; this.blue = 229 }); Text('LBlue').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FF00ACC1').borderRadius(8).onClick(() => { this.red = 0; this.green = 172; this.blue = 193 }); Text('Cyan').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FF00897B').borderRadius(8).onClick(() => { this.red = 0; this.green = 137; this.blue = 123 }); Text('Teal').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FF43A047').borderRadius(8).onClick(() => { this.red = 67; this.green = 160; this.blue = 71 }); Text('Green').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FF7CB342').borderRadius(8).onClick(() => { this.red = 124; this.green = 179; this.blue = 66 }); Text('LGreen').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FFC0CA33').borderRadius(8).onClick(() => { this.red = 192; this.green = 202; this.blue = 51 }); Text('Lime').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FFFFC107').borderRadius(8).onClick(() => { this.red = 253; this.green = 216; this.blue = 53 }); Text('Amber').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FFFB8C00').borderRadius(8).onClick(() => { this.red = 251; this.green = 140; this.blue = 0 }); Text('Orange').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FFF4511E').borderRadius(8).onClick(() => { this.red = 244; this.green = 81; this.blue = 30 }); Text('DOrange').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FF6D4C41').borderRadius(8).onClick(() => { this.red = 109; this.green = 76; this.blue = 65 }); Text('Brown').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FF546E7A').borderRadius(8).onClick(() => { this.red = 84; this.green = 110; this.blue = 122 }); Text('BGrey').fontSize(8).margin({ top: 3 }) } }
          GridItem() { Column() { Column().width(45).height(45).backgroundColor('#FF757575').borderRadius(8).onClick(() => { this.red = 117; this.green = 117; this.blue = 117 }); Text('Grey').fontSize(8).margin({ top: 3 }) } }
        }
        .columnsTemplate('1fr 1fr 1fr').rowsGap(15).columnsGap(10).width('100%').height(420)

        Divider().margin({ top: 15 })
        this.sectionTitle('13. 鐏伴樁鑹叉澘')

        Row() {
          this.colorBlock(this.grayToHex(0), '0', 28, 60)
          Column() { }.width(3)
          this.colorBlock(this.grayToHex(32), '32', 28, 60)
          Column() { }.width(3)
          this.colorBlock(this.grayToHex(64), '64', 28, 60)
          Column() { }.width(3)
          this.colorBlock(this.grayToHex(96), '96', 28, 60)
          Column() { }.width(3)
          this.colorBlock(this.grayToHex(128), '128', 28, 60)
          Column() { }.width(3)
          this.colorBlock(this.grayToHex(160), '160', 28, 60)
          Column() { }.width(3)
          this.colorBlock(this.grayToHex(192), '192', 28, 60)
          Column() { }.width(3)
          this.colorBlock(this.grayToHex(224), '224', 28, 60)
          Column() { }.width(3)
          this.colorBlock(this.grayToHex(255), '255', 28, 60)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('14. 棰滆壊鍘嗗彶璁板�?)

        Row() {
          ForEach(this.colorHistory, (color: string, index: number) => {
            Column() {
              Column().width(50).height(50).backgroundColor(color).borderRadius(8)
                .border({ width: 1, color: '#EEEEEE' })
                .onClick(() => {
                  this.red = parseInt(color.substring(3, 5), 16)
                  this.green = parseInt(color.substring(5, 7), 16)
                  this.blue = parseInt(color.substring(7, 9), 16)
                  this.resultText = '浠庡巻鍙查€夋嫨: ' + color
                })
              Text(color.substring(3).toUpperCase()).fontSize(8).margin({ top: 3 })
            }
          })
        }.width('100%')

        Row() {
          Button('娣诲姞褰撳墠棰滆�?).fontSize(14).height(36).margin({ top: 10 })
            .onClick(() => {
              this.colorHistory.unshift(this.getColorStr())
              if (this.colorHistory.length > 6) {
                this.colorHistory.splice(this.colorHistory.length - 1, 1)
              }
              this.resultText = '宸叉坊鍔? ' + this.getColorStr().toUpperCase()
            })
          Button('娓呯┖鍘嗗彶').fontSize(14).height(36).margin({ left: 10, top: 10 })
            .onClick(() => {
              this.colorHistory = []
              this.resultText = '鍘嗗彶宸叉竻�?
            })
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('15. 浜害瀵规�?)

        Row() {
          Column() {
            Text('浜�? ' + this.getBrightness() + '%').fontSize(12).margin({ bottom: 5 })
            Column().width(80).height(80).backgroundColor(this.getColorStr()).borderRadius(8)
          }.margin({ right: 20 })
          Column() {
            Text('楂樹�?).fontSize(12).margin({ bottom: 5 })
            Column().width(80).height(80).backgroundColor('#FF' + this.toHex(Math.min(255, this.red + 50)) + this.toHex(Math.min(255, this.green + 50)) + this.toHex(Math.min(255, this.blue + 50))).borderRadius(8)
          }.margin({ right: 20 })
          Column() {
            Text('鏆楄�?).fontSize(12).margin({ bottom: 5 })
            Column().width(80).height(80).backgroundColor('#FF' + this.toHex(Math.max(0, this.red - 50)) + this.toHex(Math.max(0, this.green - 50)) + this.toHex(Math.max(0, this.blue - 50))).borderRadius(8)
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('16. 鏆栬壊鍐疯壊瀵规�?)

        Row() {
          Column() {
            Text('鏆栬壊鍋忕�?).fontSize(12).margin({ bottom: 5 })
            Column().width(70).height(70).backgroundColor('#FF' + this.toHex(Math.min(255, this.red + 40)) + this.toHex(this.green) + this.toHex(Math.max(0, this.blue - 40))).borderRadius(8)
          }.margin({ right: 10 })
          Column() {
            Text('褰撳�?).fontSize(12).margin({ bottom: 5 })
            Column().width(70).height(70).backgroundColor(this.getColorStr()).borderRadius(8)
          }.margin({ right: 10 })
          Column() {
            Text('鍐疯壊鍋忕�?).fontSize(12).margin({ bottom: 5 })
            Column().width(70).height(70).backgroundColor('#FF' + this.toHex(Math.max(0, this.red - 40)) + this.toHex(this.green) + this.toHex(Math.min(255, this.blue + 40))).borderRadius(8)
          }.margin({ right: 10 })
          Column() {
            Text('鑹叉俯鍒ゆ柇').fontSize(12).margin({ bottom: 5 })
            Text(this.getWarmCool()).fontSize(14).fontWeight(FontWeight.Bold)
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('17. 涓嶅悓鑳屾櫙棰滆壊瀵规�?)

        Column() {
          Text('娴呰壊鑳屾櫙涓婄殑棰滆壊').fontSize(16).fontColor(this.getColorStr())
          Divider().margin({ top: 10, bottom: 10 })
          Text('鑹插潡灞曠ず').fontSize(14).margin({ bottom: 5 })
          Row() {
            Column().width(80).height(40).backgroundColor(this.getColorStr()).borderRadius(4)
            Text(this.getColorStr().toUpperCase()).fontSize(14).margin({ left: 10 })
          }
        }
        .width('100%').padding(15).backgroundColor('#F5F5F5').borderRadius(8)

        Column() {
          Text('娣辫壊鑳屾櫙涓婄殑棰滆壊').fontSize(16).fontColor(this.getColorStr())
          Divider().margin({ top: 10, bottom: 10 })
          Text('鑹插潡灞曠ず').fontSize(14).fontColor('#FFFFFF').margin({ bottom: 5 })
          Row() {
            Column().width(80).height(40).backgroundColor(this.getColorStr()).borderRadius(4)
            Text(this.getColorStr().toUpperCase()).fontSize(14).fontColor('#FFFFFF').margin({ left: 10 })
          }
        }
        .width('100%').padding(15).backgroundColor('#333333').borderRadius(8).margin({ top: 10 })

        Divider().margin({ top: 15 })
        this.sectionTitle('18. 棰滆壊娓愬彉棰勮�?)

        Column().width('100%').height(60).borderRadius(8)
          .linearGradient({ angle: 0, colors: [[this.getColorStr(), 0.0], [this.getComplementaryColor(), 1.0]] })

        Text('鍘熻�?�?浜掕ˉ�?娓愬�?).fontSize(13).fontColor('#999999').margin({ top: 5 })

        Column().width('100%').height(60).borderRadius(8).margin({ top: 10 })
          .linearGradient({ angle: 90, colors: [['#FF000000', 0.0], [this.getColorStr(), 0.5], ['#FFFFFFFF', 1.0]] })

        Text('榛戣�?�?褰撳墠鑹?�?鐧借壊 娓愬�?).fontSize(13).fontColor('#999999').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('19. RGB閫氶亾鍗曠嫭鍙樺寲瀵规�?)

        Row() {
          Column() {
            Text('R+50').fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF' + this.toHex(Math.min(255, this.red + 50)) + this.toHex(this.green) + this.toHex(this.blue)).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('R-50').fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF' + this.toHex(Math.max(0, this.red - 50)) + this.toHex(this.green) + this.toHex(this.blue)).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('G+50').fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF' + this.toHex(this.red) + this.toHex(Math.min(255, this.green + 50)) + this.toHex(this.blue)).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('G-50').fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF' + this.toHex(this.red) + this.toHex(Math.max(0, this.green - 50)) + this.toHex(this.blue)).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('B+50').fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(Math.min(255, this.blue + 50))).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('B-50').fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(Math.max(0, this.blue - 50))).borderRadius(8)
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('20. 楗卞拰搴﹀彉鍖栧�?)

        Row() {
          Column() {
            Text('100%').fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor(this.getColorStr()).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('75%').fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF' + this.toHex(Math.round(this.red * 0.75 + 128 * 0.25)) + this.toHex(Math.round(this.green * 0.75 + 128 * 0.25)) + this.toHex(Math.round(this.blue * 0.75 + 128 * 0.25))).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('50%').fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF' + this.toHex(Math.round(this.red * 0.5 + 128 * 0.5)) + this.toHex(Math.round(this.green * 0.5 + 128 * 0.5)) + this.toHex(Math.round(this.blue * 0.5 + 128 * 0.5))).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('25%').fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF' + this.toHex(Math.round(this.red * 0.25 + 128 * 0.75)) + this.toHex(Math.round(this.green * 0.25 + 128 * 0.75)) + this.toHex(Math.round(this.blue * 0.25 + 128 * 0.75))).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('0%').fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF808080').borderRadius(8)
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('21. 闅忔満棰滆壊鐢熸垚')

        Row() {
          Button('闅忔満棰滆�?).fontSize(14).height(36).margin(2)
            .onClick(() => {
              this.red = Math.floor(Math.random() * 256)
              this.green = Math.floor(Math.random() * 256)
              this.blue = Math.floor(Math.random() * 256)
              this.alpha = 255
              this.resultText = '闅忔満: ' + this.getColorStr().toUpperCase()
            })
          Button('闅忔満鏆栬�?).fontSize(14).height(36).margin(2)
            .onClick(() => {
              this.red = Math.floor(Math.random() * 156) + 100
              this.green = Math.floor(Math.random() * 156)
              this.blue = Math.floor(Math.random() * 100)
              this.alpha = 255
              this.resultText = '闅忔満鏆栬�? ' + this.getColorStr().toUpperCase()
            })
          Button('闅忔満鍐疯�?).fontSize(14).height(36).margin(2)
            .onClick(() => {
              this.red = Math.floor(Math.random() * 100)
              this.green = Math.floor(Math.random() * 156)
              this.blue = Math.floor(Math.random() * 156) + 100
              this.alpha = 255
              this.resultText = '闅忔満鍐疯�? ' + this.getColorStr().toUpperCase()
            })
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('22. 棰滆壊娣峰悎灞曠�?)

        Row() {
          Column() {
            Text('褰撳墠鑹?).fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor(this.getColorStr()).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('鐧借壊娣峰�?).fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF' + this.toHex(Math.round((this.red + 255) / 2)) + this.toHex(Math.round((this.green + 255) / 2)) + this.toHex(Math.round((this.blue + 255) / 2))).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('榛戣壊娣峰悎').fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF' + this.toHex(Math.round(this.red / 2)) + this.toHex(Math.round(this.green / 2)) + this.toHex(Math.round(this.blue / 2))).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('鐏拌壊娣峰悎').fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF' + this.toHex(Math.round((this.red + 128) / 2)) + this.toHex(Math.round((this.green + 128) / 2)) + this.toHex(Math.round((this.blue + 128) / 2))).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('鍙嶈壊娣峰悎').fontSize(10).margin({ bottom: 3 })
            Column().width(50).height(50).backgroundColor('#FF' + this.toHex(Math.round((this.red + 255 - this.red) / 2)) + this.toHex(Math.round((this.green + 255 - this.green) / 2)) + this.toHex(Math.round((this.blue + 255 - this.blue) / 2))).borderRadius(8)
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('23. 瀛椾綋澶у皬璋冭妭')

        Row() {
          Text('棰勮瀛椾綋澶у�? ' + this.fontSize).fontSize(14).width(120)
          Slider({ value: this.fontSize, min: 12, max: 30, step: 1 })
            .layoutWeight(1).blockColor('#007DFF').trackColor('#E0E0E0').selectedColor('#007DFF')
            .onChange((value: number) => { this.fontSize = Math.round(value) })
        }.width('100%')

        Column() {
          Text(this.getColorStr().toUpperCase()).fontSize(this.fontSize).fontColor('#FFFFFF').margin({ top: 5 })
          Text(this.getColorName()).fontSize(this.fontSize - 2).fontColor('#FFFFFF').margin({ top: 5, bottom: 5 })
        }
        .width('100%').height(60).backgroundColor(this.getColorStr())
        .borderRadius(8).justifyContent(FlexAlign.Center).margin({ top: 10 })

        Divider().margin({ top: 15 })
        this.sectionTitle('24. 澶氱粍棰勮鑹插�?)

        Row() {
          this.colorBlock('#FFF44336', 'Red500', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FFE91E63', 'Pink', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FF9C27B0', 'Purple', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FF673AB7', 'DPurple', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FF3F51B5', 'Indigo', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FF2196F3', 'Blue', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FF03A9F4', 'LBlue', 45, 45)
        }.width('100%')

        Row() {
          this.colorBlock('#FF00BCD4', 'Cyan', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FF009688', 'Teal', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FF4CAF50', 'Green', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FF8BC34A', 'LGreen', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FFCDDC39', 'Lime', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FFFFC107', 'Amber', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FFFF9800', 'Orange', 45, 45)
        }.width('100%').margin({ top: 5 })

        Row() {
          this.colorBlock('#FFFF5722', 'DOrange', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FF795548', 'Brown', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FF9E9E9E', 'Grey', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FF78909C', 'BGrey', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FF000000', 'Black', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FFFFFFFF', 'White', 45, 45)
          Column() { }.width(3)
          this.colorBlock('#FF808080', 'Grey50', 45, 45)
        }.width('100%').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('25. 棰滆壊淇℃伅姹囨�?)

        Column() {
          Row() {
            Text('棰滆壊鍚嶇�?).fontSize(14).width(80).fontColor('#999999')
            Text(this.getColorName()).fontSize(16).layoutWeight(1)
          }.width('100%').padding(8)
          Row() {
            Text('HEX').fontSize(14).width(80).fontColor('#999999')
            Text(this.getColorStr().toUpperCase()).fontSize(16).layoutWeight(1)
          }.width('100%').padding(8)
          Row() {
            Text('RGBA').fontSize(14).width(80).fontColor('#999999')
            Text(this.red + ', ' + this.green + ', ' + this.blue + ', ' + this.alpha).fontSize(16).layoutWeight(1)
          }.width('100%').padding(8)
          Row() {
            Text('HSV').fontSize(14).width(80).fontColor('#999999')
            Text(this.getHsvStr()).fontSize(16).layoutWeight(1)
          }.width('100%').padding(8)
          Row() {
            Text('浜�?).fontSize(14).width(80).fontColor('#999999')
            Text(this.getBrightness() + '%').fontSize(16).layoutWeight(1)
          }.width('100%').padding(8)
          Row() {
            Text('鑹叉�?).fontSize(14).width(80).fontColor('#999999')
            Text(this.getWarmCool()).fontSize(16).layoutWeight(1)
          }.width('100%').padding(8)
          Row() {
            Text('浜掕ˉ�?).fontSize(14).width(80).fontColor('#999999')
            Text(this.getComplementaryColor().toUpperCase()).fontSize(16).layoutWeight(1)
          }.width('100%').padding(8)
          Row() {
            Text('閫忔槑搴?).fontSize(14).width(80).fontColor('#999999')
            Text(Math.round(this.alpha / 255 * 100) + '%').fontSize(16).layoutWeight(1)
          }.width('100%').padding(8)
        }
        .width('100%').backgroundColor('#FFFFFF').borderRadius(8)

        Divider().margin({ top: 15 })
        this.sectionTitle('26. 鎿嶄綔鍥炶皟鏃ュ�?)

        Text(this.resultText).fontSize(13).fontColor('#007DFF').margin({ top: 5 })
        Text('鎷栧姩婊戝潡鎴栫偣鍑婚璁鹃鑹叉煡鐪嬪洖璋冩棩�?).fontSize(13).fontColor('#999999').margin({ top: 3 })

        Divider().margin({ top: 15 })
        this.sectionTitle('27. 棰滆壊瀵规瘮搴︽�?)

        Row() {
          Column() {
            Text('褰撳墠鑹?on 鐧借壊').fontSize(10).margin({ bottom: 3 })
            Column() {
              Text('AaBbCc').fontSize(16).fontColor(this.getColorStr())
            }.width(80).height(40).backgroundColor('#FFFFFFFF').borderRadius(4).justifyContent(FlexAlign.Center)
          }.margin({ right: 10 })
          Column() {
            Text('褰撳墠鑹?on 榛戣�?).fontSize(10).margin({ bottom: 3 })
            Column() {
              Text('AaBbCc').fontSize(16).fontColor(this.getColorStr())
            }.width(80).height(40).backgroundColor('#FF000000').borderRadius(4).justifyContent(FlexAlign.Center)
          }.margin({ right: 10 })
          Column() {
            Text('鐧借壊 on 褰撳墠鑹?).fontSize(10).margin({ bottom: 3 })
            Column() {
              Text('AaBbCc').fontSize(16).fontColor('#FFFFFFFF')
            }.width(80).height(40).backgroundColor(this.getColorStr()).borderRadius(4).justifyContent(FlexAlign.Center)
          }.margin({ right: 10 })
          Column() {
            Text('榛戣�?on 褰撳墠鑹?).fontSize(10).margin({ bottom: 3 })
            Column() {
              Text('AaBbCc').fontSize(16).fontColor('#FF000000')
            }.width(80).height(40).backgroundColor(this.getColorStr()).borderRadius(4).justifyContent(FlexAlign.Center)
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('28. 娓愬彉鑹叉柟妗堥璁?)

        Column() {
          Text('鏃ヨ�?).fontSize(12).fontColor('#FFFFFF').margin({ top: 5 })
        }
        .width('100%').height(40).borderRadius(8)
        .linearGradient({ angle: 90, colors: [['#FFFF6B35', 0.0], ['#FFF7C59F', 1.0]] })
        .margin({ bottom: 5 })

        Column() {
          Text('娴锋�?).fontSize(12).fontColor('#FFFFFF').margin({ top: 5 })
        }
        .width('100%').height(40).borderRadius(8)
        .linearGradient({ angle: 90, colors: [['#FF0077B6', 0.0], ['#FF90E0EF', 1.0]] })
        .margin({ bottom: 5 })

        Column() {
          Text('妫�?).fontSize(12).fontColor('#FFFFFF').margin({ top: 5 })
        }
        .width('100%').height(40).borderRadius(8)
        .linearGradient({ angle: 90, colors: [['#FF2D6A4F', 0.0], ['#FF83C5BE', 1.0]] })
        .margin({ bottom: 5 })

        Column() {
          Text('绱�?).fontSize(12).fontColor('#FFFFFF').margin({ top: 5 })
        }
        .width('100%').height(40).borderRadius(8)
        .linearGradient({ angle: 90, colors: [['#FF7209B7', 0.0], ['#FFFF006E', 1.0]] })
        .margin({ bottom: 5 })

        Column() {
          Text('鏋佸�?).fontSize(12).fontColor('#FFFFFF').margin({ top: 5 })
        }
        .width('100%').height(40).borderRadius(8)
        .linearGradient({ angle: 90, colors: [['#FF00C9FF', 0.0], ['#FF92FE9D', 1.0]] })

        Divider().margin({ top: 15 })
        this.sectionTitle('29. 鑹茬浉鐜繎浼煎睍绀?)

        Row() {
          this.colorBlock('#FFFF0000', '0', 25, 50)
          Column() { }.width(2)
          this.colorBlock('#FFFFA500', '30', 25, 50)
          Column() { }.width(2)
          this.colorBlock('#FFFFFF00', '60', 25, 50)
          Column() { }.width(2)
          this.colorBlock('#FF80FF00', '90', 25, 50)
          Column() { }.width(2)
          this.colorBlock('#FF00FF00', '120', 25, 50)
          Column() { }.width(2)
          this.colorBlock('#FF00FF80', '150', 25, 50)
          Column() { }.width(2)
          this.colorBlock('#FF00FFFF', '180', 25, 50)
          Column() { }.width(2)
          this.colorBlock('#FF0080FF', '210', 25, 50)
          Column() { }.width(2)
          this.colorBlock('#FF0000FF', '240', 25, 50)
          Column() { }.width(2)
          this.colorBlock('#FF8000FF', '270', 25, 50)
          Column() { }.width(2)
          this.colorBlock('#FFFF00FF', '300', 25, 50)
          Column() { }.width(2)
          this.colorBlock('#FFFF0080', '330', 25, 50)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('30. 鍚岃壊绯绘繁娴呭姣?)

        Row() {
          Column() {
            Text('鏈€�?).fontSize(10).margin({ bottom: 3 })
            Column().width(40).height(50).backgroundColor('#FF' + this.toHex(Math.min(255, this.red + 100)) + this.toHex(Math.min(255, this.green + 100)) + this.toHex(Math.min(255, this.blue + 100))).borderRadius(4)
          }.margin({ right: 3 })
          Column() {
            Text('�?).fontSize(10).margin({ bottom: 3 })
            Column().width(40).height(50).backgroundColor('#FF' + this.toHex(Math.min(255, this.red + 50)) + this.toHex(Math.min(255, this.green + 50)) + this.toHex(Math.min(255, this.blue + 50))).borderRadius(4)
          }.margin({ right: 3 })
          Column() {
            Text('褰撳�?).fontSize(10).margin({ bottom: 3 })
            Column().width(40).height(50).backgroundColor(this.getColorStr()).borderRadius(4)
          }.margin({ right: 3 })
          Column() {
            Text('�?).fontSize(10).margin({ bottom: 3 })
            Column().width(40).height(50).backgroundColor('#FF' + this.toHex(Math.max(0, this.red - 50)) + this.toHex(Math.max(0, this.green - 50)) + this.toHex(Math.max(0, this.blue - 50))).borderRadius(4)
          }.margin({ right: 3 })
          Column() {
            Text('鏈€�?).fontSize(10).margin({ bottom: 3 })
            Column().width(40).height(50).backgroundColor('#FF' + this.toHex(Math.max(0, this.red - 100)) + this.toHex(Math.max(0, this.green - 100)) + this.toHex(Math.max(0, this.blue - 100))).borderRadius(4)
          }.margin({ right: 3 })
          Column() {
            Text('鏈€�?').fontSize(10).margin({ bottom: 3 })
            Column().width(40).height(50).backgroundColor('#FF' + this.toHex(Math.max(0, this.red - 150)) + this.toHex(Math.max(0, this.green - 150)) + this.toHex(Math.max(0, this.blue - 150))).borderRadius(4)
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('31. 棰滆壊绾害瀵规�?)

        Row() {
          Column() {
            Text('100%').fontSize(10).margin({ bottom: 3 })
            Column().width(45).height(45).backgroundColor(this.getColorStr()).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('80%').fontSize(10).margin({ bottom: 3 })
            Column().width(45).height(45).backgroundColor('#FF' + this.toHex(Math.round(this.red * 0.8 + 128 * 0.2)) + this.toHex(Math.round(this.green * 0.8 + 128 * 0.2)) + this.toHex(Math.round(this.blue * 0.8 + 128 * 0.2))).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('60%').fontSize(10).margin({ bottom: 3 })
            Column().width(45).height(45).backgroundColor('#FF' + this.toHex(Math.round(this.red * 0.6 + 128 * 0.4)) + this.toHex(Math.round(this.green * 0.6 + 128 * 0.4)) + this.toHex(Math.round(this.blue * 0.6 + 128 * 0.4))).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('40%').fontSize(10).margin({ bottom: 3 })
            Column().width(45).height(45).backgroundColor('#FF' + this.toHex(Math.round(this.red * 0.4 + 128 * 0.6)) + this.toHex(Math.round(this.green * 0.4 + 128 * 0.6)) + this.toHex(Math.round(this.blue * 0.4 + 128 * 0.6))).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('20%').fontSize(10).margin({ bottom: 3 })
            Column().width(45).height(45).backgroundColor('#FF' + this.toHex(Math.round(this.red * 0.2 + 128 * 0.8)) + this.toHex(Math.round(this.green * 0.2 + 128 * 0.8)) + this.toHex(Math.round(this.blue * 0.2 + 128 * 0.8))).borderRadius(8)
          }.margin({ right: 5 })
          Column() {
            Text('0%').fontSize(10).margin({ bottom: 3 })
            Column().width(45).height(45).backgroundColor('#FF808080').borderRadius(8)
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('32. 绾㈣壊绯诲垪鑹插�?)

        Row() {
          this.colorBlock('#FFFFEBEE', 'R50', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FFFFCDD2', 'R100', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FFEF9A9A', 'R200', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FFE57373', 'R300', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FFEF5350', 'R400', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FFF44336', 'R500', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FFE53935', 'R600', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FFD32F2F', 'R700', 40, 40)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('33. 钃濊壊绯诲垪鑹插�?)

        Row() {
          this.colorBlock('#FFE3F2FD', 'B50', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FFBBDEFB', 'B100', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FF90CAF9', 'B200', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FF64B5F6', 'B300', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FF42A5F5', 'B400', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FF2196F3', 'B500', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FF1E88E5', 'B600', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FF1565C0', 'B700', 40, 40)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('34. 缁胯壊绯诲垪鑹插�?)

        Row() {
          this.colorBlock('#FFE8F5E9', 'G50', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FFC8E6C9', 'G100', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FFA5D6A7', 'G200', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FF81C784', 'G300', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FF66BB6A', 'G400', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FF4CAF50', 'G500', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FF43A047', 'G600', 40, 40)
          Column() { }.width(3)
          this.colorBlock('#FF2E7D32', 'G700', 40, 40)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('35. 棰滆壊涓夎绱犲垎鏋?)

        Column() {
          Row() {
            Text('鑹茬�?H)').fontSize(14).width(80).fontColor('#999999')
            Text(this.getHsvStr()).fontSize(16).layoutWeight(1)
          }.width('100%').padding(8)
          Row() {
            Text('楗卞拰搴?S)').fontSize(14).width(80).fontColor('#999999')
            Slider({ value: 100, min: 0, max: 100, step: 1 }).layoutWeight(1).blockColor('#007DFF')
          }.width('100%').padding(8)
          Row() {
            Text('鏄庡�?V)').fontSize(14).width(80).fontColor('#999999')
            Text(this.getBrightness() + '%').fontSize(16).layoutWeight(1)
          }.width('100%').padding(8)
        }
        .width('100%').backgroundColor('#FFFFFF').borderRadius(8)

        Divider().margin({ top: 15 })
        this.sectionTitle('36. 澶氳壊娓愬彉娴嬭�?)

        Column().width('100%').height(50).borderRadius(8)
          .linearGradient({ angle: 90, colors: [['#FFFF0000', 0.0], ['#FFFFFF00', 0.25], ['#FF00FF00', 0.5], ['#FF00FFFF', 0.75], ['#FF0000FF', 1.0]] })

        Text('绾⑩啋榛勨啋缁库啋闈掆啋�?褰╄櫣娓愬彉').fontSize(13).fontColor('#999999').margin({ top: 5 })

        Column().width('100%').height(50).borderRadius(8).margin({ top: 10 })
          .linearGradient({ angle: 90, colors: [['#FF000000', 0.0], ['#FF808080', 0.5], ['#FFFFFFFF', 1.0]] })

        Text('榛戔啋鐏扳啋�?鐏板害娓愬彉').fontSize(13).fontColor('#999999').margin({ top: 5 })

        Column().width('100%').height(50).borderRadius(8).margin({ top: 10 })
          .linearGradient({ angle: 90, colors: [[this.getColorStr(), 0.0], [this.getComplementaryColor(), 1.0]] })

        Text('褰撳墠鑹测啋浜掕ˉ�?娓愬�?).fontSize(13).fontColor('#999999').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('37. 棰滆壊鎼厤鏂规�?)

        Row() {
          Column() {
            Text('绫绘瘮鑹?).fontSize(10).margin({ bottom: 3 })
            Row() {
              Column().width(25).height(40).backgroundColor('#FF' + this.toHex(Math.max(0, this.red - 30)) + this.toHex(Math.min(255, this.green + 30)) + this.toHex(this.blue)).borderRadius(4)
              Column().width(25).height(40).backgroundColor(this.getColorStr()).borderRadius(4)
              Column().width(25).height(40).backgroundColor('#FF' + this.toHex(Math.min(255, this.red + 30)) + this.toHex(Math.max(0, this.green - 30)) + this.toHex(this.blue)).borderRadius(4)
            }
          }.margin({ right: 15 })
          Column() {
            Text('涓夊厓鑹?).fontSize(10).margin({ bottom: 3 })
            Row() {
              Column().width(25).height(40).backgroundColor(this.getColorStr()).borderRadius(4)
              Column().width(25).height(40).backgroundColor('#FF' + this.toHex(Math.min(255, this.green + 50)) + this.toHex(this.blue) + this.toHex(this.red)).borderRadius(4)
              Column().width(25).height(40).backgroundColor('#FF' + this.toHex(this.blue) + this.toHex(this.red) + this.toHex(Math.min(255, this.green + 50))).borderRadius(4)
            }
          }.margin({ right: 15 })
          Column() {
            Text('浜掕ˉ�?).fontSize(10).margin({ bottom: 3 })
            Row() {
              Column().width(25).height(40).backgroundColor(this.getColorStr()).borderRadius(4)
              Column().width(25).height(40).backgroundColor(this.getComplementaryColor()).borderRadius(4)
            }
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('38. 棰滆壊搴旂敤鍦烘櫙棰勮')

        Column() {
          Text('鎸夐挳棰滆壊棰勮�?).fontSize(14).margin({ bottom: 10 })
          Row() {
            Button('Default').backgroundColor(this.getColorStr()).margin(5)
            Button('Outlined').borderColor(this.getColorStr()).margin(5)
            Button('Text').fontColor(this.getColorStr()).margin(5)
          }
          Text('鏂囧瓧棰滆壊棰勮�?).fontSize(14).margin({ top: 10, bottom: 5 })
          Text('杩欐槸涓€娈典娇鐢ㄥ綋鍓嶉鑹茬殑鏂囧瓧').fontSize(16).fontColor(this.getColorStr())
          Text('鑳屾櫙鑹查�?).fontSize(14).margin({ top: 10, bottom: 5 })
          Column().width('100%').height(40).backgroundColor(this.getColorStr()).borderRadius(8)
        }
        .width('100%').padding(15).backgroundColor('#FFFFFF').borderRadius(8)

        Divider().margin({ top: 15 })
        this.sectionTitle('39. Web瀹夊叏鑹?)

        Grid() {
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FFFF0000').borderRadius(4); Text('FF0000').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FFFF3300').borderRadius(4); Text('FF3300').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FFFF6600').borderRadius(4); Text('FF6600').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FFFF9900').borderRadius(4); Text('FF9900').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FFFFCC00').borderRadius(4); Text('FFCC00').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FFFFFF00').borderRadius(4); Text('FFFF00').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FFCCFF00').borderRadius(4); Text('CCFF00').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FF99FF00').borderRadius(4); Text('99FF00').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FF66FF00').borderRadius(4); Text('66FF00').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FF33FF00').borderRadius(4); Text('33FF00').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FF00FF00').borderRadius(4); Text('00FF00').fontSize(7).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FF00FF33').borderRadius(4); Text('00FF33').fontSize(7).margin({ top: 2 }) } }
        }
        .columnsTemplate('1fr 1fr 1fr 1fr 1fr 1fr').rowsGap(10).columnsGap(5).width('100%').height(180)

        Divider().margin({ top: 15 })
        this.sectionTitle('40. 棰滆壊鎰熺煡娴嬭�?)

        Row() {
          Column() {
            Text('鏆栬壊绯?).fontSize(12).margin({ bottom: 5 })
            Column().width(60).height(60).backgroundColor('#FFFF5722').borderRadius(8)
            Text('鐑�?娲诲�?).fontSize(10).margin({ top: 3 }).fontColor('#999999')
          }.margin({ right: 10 })
          Column() {
            Text('鍐疯壊绯?).fontSize(12).margin({ bottom: 5 })
            Column().width(60).height(60).backgroundColor('#FF2196F3').borderRadius(8)
            Text('鍐烽�?涓撲�?).fontSize(10).margin({ top: 3 }).fontColor('#999999')
          }.margin({ right: 10 })
          Column() {
            Text('涓€ц壊').fontSize(12).margin({ bottom: 5 })
            Column().width(60).height(60).backgroundColor('#FF9E9E9E').borderRadius(8)
            Text('骞宠　/绋抽�?).fontSize(10).margin({ top: 3 }).fontColor('#999999')
          }.margin({ right: 10 })
          Column() {
            Text('褰撳墠鑹?).fontSize(12).margin({ bottom: 5 })
            Column().width(60).height(60).backgroundColor(this.getColorStr()).borderRadius(8)
            Text(this.getWarmCool()).fontSize(10).margin({ top: 3 }).fontColor('#999999')
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('41. 涓浗浼犵粺鑹插�?)

        Grid() {
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FFFF0000').borderRadius(4).onClick(() => { this.red = 255; this.green = 0; this.blue = 0 }); Text('鏈辩�?).fontSize(8).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FFFF4C00').borderRadius(4).onClick(() => { this.red = 255; this.green = 76; this.blue = 0 }); Text('姗樼�?).fontSize(8).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FFFF7500').borderRadius(4).onClick(() => { this.red = 255; this.green = 117; this.blue = 0 }); Text('璧�?).fontSize(8).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FFFFD700').borderRadius(4).onClick(() => { this.red = 255; this.green = 215; this.blue = 0 }); Text('閲戣�?).fontSize(8).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FFE6601E').borderRadius(4).onClick(() => { this.red = 230; this.green = 96; this.blue = 30 }); Text('鏉忛�?).fontSize(8).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FFD25519').borderRadius(4).onClick(() => { this.red = 210; this.green = 85; this.blue = 25 }); Text('鑼惰�?).fontSize(8).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FFA3D36B').borderRadius(4).onClick(() => { this.red = 163; this.green = 211; this.blue = 107 }); Text('钁辩�?).fontSize(8).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FF4CAF50').borderRadius(4).onClick(() => { this.red = 76; this.green = 175; this.blue = 80 }); Text('缈犵�?).fontSize(8).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FF00897B').borderRadius(4).onClick(() => { this.red = 0; this.green = 137; this.blue = 123 }); Text('闈掔�?).fontSize(8).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FF0288D1').borderRadius(4).onClick(() => { this.red = 2; this.green = 136; this.blue = 209 }); Text('闈涢�?).fontSize(8).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FF1A237E').borderRadius(4).onClick(() => { this.red = 26; this.green = 35; this.blue = 126 }); Text('瀹濊�?).fontSize(8).margin({ top: 2 }) } }
          GridItem() { Column() { Column().width(40).height(40).backgroundColor('#FF6A1B9A').borderRadius(4).onClick(() => { this.red = 106; this.green = 27; this.blue = 154 }); Text('绱�?).fontSize(8).margin({ top: 2 }) } }
        }
        .columnsTemplate('1fr 1fr 1fr 1fr').rowsGap(12).columnsGap(8).width('100%').height(360)

        Divider().margin({ top: 15 })
        this.sectionTitle('42. 浜害闃舵娴嬭�?)

        Row() {
          Column() {
            Text('0%').fontSize(8).margin({ bottom: 2 })
            Column().width(28).height(40).backgroundColor('#FF000000').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('10%').fontSize(8).margin({ bottom: 2 })
            Column().width(28).height(40).backgroundColor('#FF1A1A1A').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('20%').fontSize(8).margin({ bottom: 2 })
            Column().width(28).height(40).backgroundColor('#FF333333').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('30%').fontSize(8).margin({ bottom: 2 })
            Column().width(28).height(40).backgroundColor('#FF4D4D4D').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('40%').fontSize(8).margin({ bottom: 2 })
            Column().width(28).height(40).backgroundColor('#FF666666').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('50%').fontSize(8).margin({ bottom: 2 })
            Column().width(28).height(40).backgroundColor('#FF808080').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('60%').fontSize(8).margin({ bottom: 2 })
            Column().width(28).height(40).backgroundColor('#FF999999').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('70%').fontSize(8).margin({ bottom: 2 })
            Column().width(28).height(40).backgroundColor('#FFB3B3B3').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('80%').fontSize(8).margin({ bottom: 2 })
            Column().width(28).height(40).backgroundColor('#FFCCCCCC').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('90%').fontSize(8).margin({ bottom: 2 })
            Column().width(28).height(40).backgroundColor('#FFE6E6E6').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('100%').fontSize(8).margin({ bottom: 2 })
            Column().width(28).height(40).backgroundColor('#FFFFFFFF').borderRadius(4).border({ width: 1, color: '#CCC' })
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('43. 閫忔槑搴﹂樁姊祴璇?)

        Row() {
          Column() {
            Text('100%').fontSize(8).margin({ bottom: 2 })
            Column().width(40).height(50).backgroundColor('#FF' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)).borderRadius(4)
          }.margin({ right: 3 })
          Column() {
            Text('90%').fontSize(8).margin({ bottom: 2 })
            Column().width(40).height(50).backgroundColor('#E6' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)).borderRadius(4)
          }.margin({ right: 3 })
          Column() {
            Text('80%').fontSize(8).margin({ bottom: 2 })
            Column().width(40).height(50).backgroundColor('#CC' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)).borderRadius(4)
          }.margin({ right: 3 })
          Column() {
            Text('70%').fontSize(8).margin({ bottom: 2 })
            Column().width(40).height(50).backgroundColor('#B3' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)).borderRadius(4)
          }.margin({ right: 3 })
          Column() {
            Text('60%').fontSize(8).margin({ bottom: 2 })
            Column().width(40).height(50).backgroundColor('#99' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)).borderRadius(4)
          }.margin({ right: 3 })
          Column() {
            Text('50%').fontSize(8).margin({ bottom: 2 })
            Column().width(40).height(50).backgroundColor('#80' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)).borderRadius(4)
          }.margin({ right: 3 })
          Column() {
            Text('40%').fontSize(8).margin({ bottom: 2 })
            Column().width(40).height(50).backgroundColor('#66' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)).borderRadius(4)
          }.margin({ right: 3 })
          Column() {
            Text('30%').fontSize(8).margin({ bottom: 2 })
            Column().width(40).height(50).backgroundColor('#4D' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)).borderRadius(4)
          }.margin({ right: 3 })
          Column() {
            Text('20%').fontSize(8).margin({ bottom: 2 })
            Column().width(40).height(50).backgroundColor('#33' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)).borderRadius(4)
          }.margin({ right: 3 })
          Column() {
            Text('10%').fontSize(8).margin({ bottom: 2 })
            Column().width(40).height(50).backgroundColor('#1A' + this.toHex(this.red) + this.toHex(this.green) + this.toHex(this.blue)).borderRadius(4)
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('44. RGB鐧惧垎姣旇〃绀?)

        Text('R: ' + Math.round(this.red / 255 * 100) + '% (' + this.red + '/255)').fontSize(16).margin({ top: 5 })
        Text('G: ' + Math.round(this.green / 255 * 100) + '% (' + this.green + '/255)').fontSize(16).margin({ top: 5 })
        Text('B: ' + Math.round(this.blue / 255 * 100) + '% (' + this.blue + '/255)').fontSize(16).margin({ top: 5 })
        Text('A: ' + Math.round(this.alpha / 255 * 100) + '% (' + this.alpha + '/255)').fontSize(16).margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('45. CMYK杩戜技杞�?)

        Text('C: ' + Math.round((255 - this.red) / 255 * 100) + '%').fontSize(16).margin({ top: 5 })
        Text('M: ' + Math.round((255 - this.green) / 255 * 100) + '%').fontSize(16).margin({ top: 5 })
        Text('Y: ' + Math.round((255 - this.blue) / 255 * 100) + '%').fontSize(16).margin({ top: 5 })
        Text('K: ' + Math.round(Math.min(this.red, this.green, this.blue) / 255 * 100) + '%').fontSize(16).margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('46. 棰滆壊娓╁害鍒诲�?)

        Row() {
          Column() {
            Text('1000K').fontSize(8).margin({ bottom: 2 })
            Column().width(30).height(50).backgroundColor('#FFFF9B0').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('3000K').fontSize(8).margin({ bottom: 2 })
            Column().width(30).height(50).backgroundColor('#FFD2B4').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('4000K').fontSize(8).margin({ bottom: 2 })
            Column().width(30).height(50).backgroundColor('#FFE6C4').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('5000K').fontSize(8).margin({ bottom: 2 })
            Column().width(30).height(50).backgroundColor('#FFFFF5').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('6500K').fontSize(8).margin({ bottom: 2 })
            Column().width(30).height(50).backgroundColor('#FFE9F0').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('8000K').fontSize(8).margin({ bottom: 2 })
            Column().width(30).height(50).backgroundColor('#FFD4E5').borderRadius(4)
          }.margin({ right: 2 })
          Column() {
            Text('10000K').fontSize(8).margin({ bottom: 2 })
            Column().width(30).height(50).backgroundColor('#FFB0CC').borderRadius(4)
          }
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('47. 棰滆壊蹇冪悊�?)

        Column() {
          Row() {
            Text('绾㈣�?).fontSize(14).width(60).fontColor('#F44336')
            Text('鐑儏銆佹椿鍔涖€佸嵄闄?).fontSize(14).layoutWeight(1)
          }.width('100%').padding(6)
          Row() {
            Text('姗欒�?).fontSize(14).width(60).fontColor('#FF9800')
            Text('娓╂殩銆佸垱鎰忋€佸弸濂?).fontSize(14).layoutWeight(1)
          }.width('100%').padding(6)
          Row() {
            Text('榛勮�?).fontSize(14).width(60).fontColor('#FFC107')
            Text('涔愯銆佹敞鎰忋€佽鍛?).fontSize(14).layoutWeight(1)
          }.width('100%').padding(6)
          Row() {
            Text('缁胯�?).fontSize(14).width(60).fontColor('#4CAF50')
            Text('鑷劧銆佸仴搴枫€佹垚鍔?).fontSize(14).layoutWeight(1)
          }.width('100%').padding(6)
          Row() {
            Text('钃濊�?).fontSize(14).width(60).fontColor('#2196F3')
            Text('淇′换銆佷笓涓氥€佸喎�?).fontSize(14).layoutWeight(1)
          }.width('100%').padding(6)
          Row() {
            Text('绱�?).fontSize(14).width(60).fontColor('#9C27B0')
            Text('绁炵銆侀珮璐点€佸垱�?).fontSize(14).layoutWeight(1)
          }.width('100%').padding(6)
          Row() {
            Text('榛戣�?).fontSize(14).width(60).fontColor('#000000')
            Text('鏉冨▉銆佹寮忋€佷紭闆?).fontSize(14).layoutWeight(1)
          }.width('100%').padding(6)
          Row() {
            Text('鐧借壊').fontSize(14).width(60).fontColor('#999999')
            Text('绾磥銆佺畝绾︺€佺┖鐧?).fontSize(14).layoutWeight(1)
          }.width('100%').padding(6)
        }
        .width('100%').backgroundColor('#FFFFFF').borderRadius(8)

        Divider().margin({ top: 15 })
        this.sectionTitle('48. 鑹插€奸€熸煡琛?)

        Column() {
          Row() {
            Text('棰滆�?).fontSize(12).width(60).fontColor('#999999')
            Text('HEX').fontSize(12).width(80).fontColor('#999999')
            Text('RGB').fontSize(12).layoutWeight(1).fontColor('#999999')
          }.width('100%').padding(4)
          Row() {
            Column().width(20).height(20).backgroundColor('#FFFF0000').borderRadius(4)
            Text('绾㈣�?).fontSize(12).width(40).margin({ left: 5 })
            Text('#FF0000').fontSize(12).width(80)
            Text('255, 0, 0').fontSize(12).layoutWeight(1)
          }.width('100%').padding(4)
          Row() {
            Column().width(20).height(20).backgroundColor('#FF00FF00').borderRadius(4)
            Text('缁胯�?).fontSize(12).width(40).margin({ left: 5 })
            Text('#00FF00').fontSize(12).width(80)
            Text('0, 255, 0').fontSize(12).layoutWeight(1)
          }.width('100%').padding(4)
          Row() {
            Column().width(20).height(20).backgroundColor('#FF0000FF').borderRadius(4)
            Text('钃濊�?).fontSize(12).width(40).margin({ left: 5 })
            Text('#0000FF').fontSize(12).width(80)
            Text('0, 0, 255').fontSize(12).layoutWeight(1)
          }.width('100%').padding(4)
          Row() {
            Column().width(20).height(20).backgroundColor('#FFFFFF00').borderRadius(4)
            Text('榛勮�?).fontSize(12).width(40).margin({ left: 5 })
            Text('#FFFF00').fontSize(12).width(80)
            Text('255, 255, 0').fontSize(12).layoutWeight(1)
          }.width('100%').padding(4)
          Row() {
            Column().width(20).height(20).backgroundColor('#FF00FFFF').borderRadius(4)
            Text('闈掕�?).fontSize(12).width(40).margin({ left: 5 })
            Text('#00FFFF').fontSize(12).width(80)
            Text('0, 255, 255').fontSize(12).layoutWeight(1)
          }.width('100%').padding(4)
          Row() {
            Column().width(20).height(20).backgroundColor('#FFFF00FF').borderRadius(4)
            Text('绱�?).fontSize(12).width(40).margin({ left: 5 })
            Text('#FF00FF').fontSize(12).width(80)
            Text('255, 0, 255').fontSize(12).layoutWeight(1)
          }.width('100%').padding(4)
          Row() {
            Column().width(20).height(20).backgroundColor('#FFFFFFFF').borderRadius(4).border({ width: 1, color: '#CCC' })
            Text('鐧借壊').fontSize(12).width(40).margin({ left: 5 })
            Text('#FFFFFF').fontSize(12).width(80)
            Text('255, 255, 255').fontSize(12).layoutWeight(1)
          }.width('100%').padding(4)
          Row() {
            Column().width(20).height(20).backgroundColor('#FF000000').borderRadius(4)
            Text('榛戣�?).fontSize(12).width(40).margin({ left: 5 })
            Text('#000000').fontSize(12).width(80)
            Text('0, 0, 0').fontSize(12).layoutWeight(1)
          }.width('100%').padding(4)
        }
        .width('100%').backgroundColor('#FFFFFF').borderRadius(8)

        Divider().margin({ top: 15 })
        this.sectionTitle('49. 褰撳墠鑹插鍦烘櫙搴旂敤')

        Column() {
          Text('UI缁勪欢棰滆壊搴旂�?).fontSize(14).margin({ bottom: 8 })
          Row() {
            Column() {
              Button('鎸夐�?).backgroundColor(this.getColorStr()).fontSize(12).height(30)
              Text('Button').fontSize(8).margin({ top: 3 })
            }.margin({ right: 10 })
            Column() {
              Text('鏂囧�?).fontSize(16).fontColor(this.getColorStr())
              Text('Text').fontSize(8).margin({ top: 3 })
            }.margin({ right: 10 })
            Column() {
              Column().width(30).height(30).backgroundColor(this.getColorStr()).borderRadius(15)
              Text('鍦嗗�?).fontSize(8).margin({ top: 3 })
            }.margin({ right: 10 })
            Column() {
              Column().width(30).height(30).backgroundColor(this.getColorStr()).borderRadius(4)
              Text('鏂瑰�?).fontSize(8).margin({ top: 3 })
            }.margin({ right: 10 })
            Column() {
              Column().width(40).height(6).backgroundColor(this.getColorStr()).borderRadius(3)
              Text('杩涘害鏉?).fontSize(8).margin({ top: 8 })
            }
          }
        }
        .width('100%').padding(15).backgroundColor('#FFFFFF').borderRadius(8)

        Divider().margin({ top: 15 })
        this.sectionTitle('50. 棰滆壊瀵规瘮鎬荤粨')

        Column() {
          Row() {
            Text('鍘熻�?).fontSize(12).width(60).fontColor('#999999')
            Column().width(20).height(20).backgroundColor(this.getColorStr()).borderRadius(4)
            Text(this.getColorStr().toUpperCase()).fontSize(12).margin({ left: 10 })
          }.width('100%').padding(6)
          Row() {
            Text('浜掕ˉ�?).fontSize(12).width(60).fontColor('#999999')
            Column().width(20).height(20).backgroundColor(this.getComplementaryColor()).borderRadius(4)
            Text(this.getComplementaryColor().toUpperCase()).fontSize(12).margin({ left: 10 })
          }.width('100%').padding(6)
          Row() {
            Text('娴呰壊鐗?).fontSize(12).width(60).fontColor('#999999')
            Column().width(20).height(20).backgroundColor('#FF' + this.toHex(Math.min(255, this.red + 60)) + this.toHex(Math.min(255, this.green + 60)) + this.toHex(Math.min(255, this.blue + 60))).borderRadius(4)
            Text(('#FF' + this.toHex(Math.min(255, this.red + 60)) + this.toHex(Math.min(255, this.green + 60)) + this.toHex(Math.min(255, this.blue + 60))).toUpperCase()).fontSize(12).margin({ left: 10 })
          }.width('100%').padding(6)
          Row() {
            Text('娣辫壊鐗?).fontSize(12).width(60).fontColor('#999999')
            Column().width(20).height(20).backgroundColor('#FF' + this.toHex(Math.max(0, this.red - 60)) + this.toHex(Math.max(0, this.green - 60)) + this.toHex(Math.max(0, this.blue - 60))).borderRadius(4)
            Text(('#FF' + this.toHex(Math.max(0, this.red - 60)) + this.toHex(Math.max(0, this.green - 60)) + this.toHex(Math.max(0, this.blue - 60))).toUpperCase()).fontSize(12).margin({ left: 10 })
          }.width('100%').padding(6)
          Row() {
            Text('鐏拌壊鐗?).fontSize(12).width(60).fontColor('#999999')
            Column().width(20).height(20).backgroundColor(this.grayToHex(this.getBrightness() * 255 / 100)).borderRadius(4)
            Text(this.grayToHex(this.getBrightness() * 255 / 100).toUpperCase()).fontSize(12).margin({ left: 10 })
          }.width('100%').padding(6)
        }
        .width('100%').backgroundColor('#FFFFFF').borderRadius(8)

        Divider().margin({ top: 15 })
        this.sectionTitle('51. 青色�?)

        Row() {
          this.colorBlock('#FFE0F7FA', 'E0F7FA', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FFB2EBF2', 'B2EBF2', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF80DEEA', '80DEEA', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF4DD0E1', '4DD0E1', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF26C6DA', '26C6DA', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF00BCD4', '00BCD4', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF00ACC1', '00ACC1', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF0097A7', '0097A7', 38, 38)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('52. 琥珀色系')

        Row() {
          this.colorBlock('#FFFFF8E1', 'FFF8E1', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FFFFECB3', 'FFECB3', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FFFFE082', 'FFE082', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FFFFD54F', 'FFD54F', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FFFFCA28', 'FFCA28', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FFFFC107', 'FFC107', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FFFFB300', 'FFB300', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FFFF8F00', 'FF8F00', 38, 38)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('53. 靛蓝�?)

        Row() {
          this.colorBlock('#FFE8EAF6', 'E8EAF6', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FFC5CAE9', 'C5CAE9', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF9FA8DA', '9FA8DA', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF7986CB', '7986CB', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF5C6BC0', '5C6BC0', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF3F51B5', '3F51B5', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF3949AB', '3949AB', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF303F9F', '303F9F', 38, 38)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('54. 深绿�?)

        Row() {
          this.colorBlock('#FFE0F2F1', 'E0F2F1', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FFB2DFDB', 'B2DFDB', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF80CBC4', '80CBC4', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF4DB6AC', '4DB6AC', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF26A69A', '26A69A', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF009688', '009688', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF00897B', '00897B', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF00796B', '00796B', 38, 38)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('55. 棕色�?)

        Row() {
          this.colorBlock('#FFEFEBE9', 'EFEBE9', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FFD7CCC8', 'D7CCC8', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FFBCAAA4', 'BCAAA4', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FFA1887F', 'A1887F', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF8D6E63', '8D6E63', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF795548', '795548', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF6D4C41', '6D4C41', 38, 38)
          Column() { }.width(3)
          this.colorBlock('#FF5D4037', '5D4037', 38, 38)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('56. 随机色卡 0')

        Row() {
          this.colorBlock('#FFDB3EED', 'R219 G62 B237', 60, 60)
          Column() { }.width(10)
          Column() {
            Text('HEX: #FFDB3EED').fontSize(12)
            Text('RGB: 219, 62, 237').fontSize(12).margin({ top: 3 })
            Text('亮度: ' + Math.round(219 * 0.299 + 62 * 0.587 + 237 * 0.114)).fontSize(12).margin({ top: 3 })
          }.alignItems(HorizontalAlign.Start).layoutWeight(1)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('57. 随机色卡 1')

        Row() {
          this.colorBlock('#FF03FA6B', 'R3 G250 B107', 60, 60)
          Column() { }.width(10)
          Column() {
            Text('HEX: #FF03FA6B').fontSize(12)
            Text('RGB: 3, 250, 107').fontSize(12).margin({ top: 3 })
            Text('亮度: ' + Math.round(3 * 0.299 + 250 * 0.587 + 107 * 0.114)).fontSize(12).margin({ top: 3 })
          }.alignItems(HorizontalAlign.Start).layoutWeight(1)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('58. 随机色卡 2')

        Row() {
          this.colorBlock('#FFA91649', 'R169 G22 B73', 60, 60)
          Column() { }.width(10)
          Column() {
            Text('HEX: #FFA91649').fontSize(12)
            Text('RGB: 169, 22, 73').fontSize(12).margin({ top: 3 })
            Text('亮度: ' + Math.round(169 * 0.299 + 22 * 0.587 + 73 * 0.114)).fontSize(12).margin({ top: 3 })
          }.alignItems(HorizontalAlign.Start).layoutWeight(1)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('59. 随机色卡 3')

        Row() {
          this.colorBlock('#FF2E7D32', 'R46 G125 B50', 60, 60)
          Column() { }.width(10)
          Column() {
            Text('HEX: #FF2E7D32').fontSize(12)
            Text('RGB: 29, 132, 45').fontSize(12).margin({ top: 3 })
            Text('亮度: ' + Math.round(29 * 0.299 + 132 * 0.587 + 45 * 0.114)).fontSize(12).margin({ top: 3 })
          }.alignItems(HorizontalAlign.Start).layoutWeight(1)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('60. 随机色卡 4')

        Row() {
          this.colorBlock('#FFAEE901', 'R174 G233 B1', 60, 60)
          Column() { }.width(10)
          Column() {
            Text('HEX: #FFAEE901').fontSize(12)
            Text('RGB: 174, 233, 1').fontSize(12).margin({ top: 3 })
            Text('亮度: ' + Math.round(174 * 0.299 + 233 * 0.587 + 1 * 0.114)).fontSize(12).margin({ top: 3 })
          }.alignItems(HorizontalAlign.Start).layoutWeight(1)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('61. 随机色卡 5')

        Row() {
          this.colorBlock('#FF023350', 'R2 G51 B80', 60, 60)
          Column() { }.width(10)
          Column() {
            Text('HEX: #FF023350').fontSize(12)
            Text('RGB: 2, 51, 80').fontSize(12).margin({ top: 3 })
            Text('亮度: ' + Math.round(2 * 0.299 + 51 * 0.587 + 80 * 0.114)).fontSize(12).margin({ top: 3 })
          }.alignItems(HorizontalAlign.Start).layoutWeight(1)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('62. 随机色卡 6')

        Row() {
          this.colorBlock('#FF80A7D6', 'R128 G167 B214', 60, 60)
          Column() { }.width(10)
          Column() {
            Text('HEX: #FF80A7D6').fontSize(12)
            Text('RGB: 128, 167, 214').fontSize(12).margin({ top: 3 })
            Text('亮度: ' + Math.round(128 * 0.299 + 167 * 0.587 + 214 * 0.114)).fontSize(12).margin({ top: 3 })
          }.alignItems(HorizontalAlign.Start).layoutWeight(1)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('63. 随机色卡 7')

        Row() {
          this.colorBlock('#FF894D59', 'R137 G77 B89', 60, 60)
          Column() { }.width(10)
          Column() {
            Text('HEX: #FF894D59').fontSize(12)
            Text('RGB: 137, 77, 89').fontSize(12).margin({ top: 3 })
            Text('亮度: ' + Math.round(137 * 0.299 + 77 * 0.587 + 89 * 0.114)).fontSize(12).margin({ top: 3 })
          }.alignItems(HorizontalAlign.Start).layoutWeight(1)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('64. 随机色卡 8')

        Row() {
          this.colorBlock('#FF480C13', 'R72 G12 B19', 60, 60)
          Column() { }.width(10)
          Column() {
            Text('HEX: #FF480C13').fontSize(12)
            Text('RGB: 72, 12, 19').fontSize(12).margin({ top: 3 })
            Text('亮度: ' + Math.round(72 * 0.299 + 12 * 0.587 + 19 * 0.114)).fontSize(12).margin({ top: 3 })
          }.alignItems(HorizontalAlign.Start).layoutWeight(1)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('65. 随机色卡 9')

        Row() {
          this.colorBlock('#FF0AECEB', 'R10 G236 B235', 60, 60)
          Column() { }.width(10)
          Column() {
            Text('HEX: #FF0AECEB').fontSize(12)
            Text('RGB: 10, 236, 235').fontSize(12).margin({ top: 3 })
            Text('亮度: ' + Math.round(10 * 0.299 + 236 * 0.587 + 235 * 0.114)).fontSize(12).margin({ top: 3 })
          }.alignItems(HorizontalAlign.Start).layoutWeight(1)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('66. 随机色卡 10')

        Row() {
          this.colorBlock('#FF1742A8', 'R23 G66 B168', 60, 60)
          Column() { }.width(10)
          Column() {
            Text('HEX: #FF1742A8').fontSize(12)
            Text('RGB: 23, 66, 168').fontSize(12).margin({ top: 3 })
            Text('亮度: ' + Math.round(23 * 0.299 + 66 * 0.587 + 168 * 0.114)).fontSize(12).margin({ top: 3 })
          }.alignItems(HorizontalAlign.Start).layoutWeight(1)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('67. 随机色卡 11')

        Row() {
          this.colorBlock('#FF8EF588', 'R142 G245 B136', 60, 60)
          Column() { }.width(10)
          Column() {
            Text('HEX: #FF8EF588').fontSize(12)
            Text('RGB: 142, 245, 136').fontSize(12).margin({ top: 3 })
            Text('亮度: ' + Math.round(142 * 0.299 + 245 * 0.587 + 136 * 0.114)).fontSize(12).margin({ top: 3 })
          }.alignItems(HorizontalAlign.Start).layoutWeight(1)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('68. 随机色卡 12')

        Row() {
          this.colorBlock('#FFF90686', 'R249 G6 B134', 60, 60)
          Column() { }.width(10)
          Column() {
            Text('HEX: #FFF90686').fontSize(12)
            Text('RGB: 249, 6, 134').fontSize(12).margin({ top: 3 })
            Text('亮度: ' + Math.round(249 * 0.299 + 6 * 0.587 + 134 * 0.114)).fontSize(12).margin({ top: 3 })
          }.alignItems(HorizontalAlign.Start).layoutWeight(1)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('69. 随机色卡 13')

        Row() {
          this.colorBlock('#FF1726DF', 'R23 G38 B223', 60, 60)
          Column() { }.width(10)
          Column() {
            Text('HEX: #FF1726DF').fontSize(12)
            Text('RGB: 23, 38, 223').fontSize(12).margin({ top: 3 })
            Text('亮度: ' + Math.round(23 * 0.299 + 38 * 0.587 + 223 * 0.114)).fontSize(12).margin({ top: 3 })
          }.alignItems(HorizontalAlign.Start).layoutWeight(1)
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('70. 随机色卡 14')

        Row() {
          this.colorBlock('#FF32593B', 'R50 G89 B59', 60, 60)
          Column() { }.width(10)
          Column() {
            Text('HEX: #FF32593B').fontSize(12)
            Text('RGB: 50, 89, 59').fontSize(12).margin({ top: 3 })
            Text('亮度: ' + Math.round(50 * 0.299 + 89 * 0.587 + 59 * 0.114)).fontSize(12).margin({ top: 3 })
          }.alignItems(HorizontalAlign.Start).layoutWeight(1)
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('100. 色卡�?0').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('101. 色卡�?1').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('102. 色卡�?2').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
            Column().width(35).height(35).backgroundColor('#FFD32F2F').borderRadius(4)
            Text('D32F2F').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
          Column() {
            Column().width(35).height(35).backgroundColor('#FFC62828').borderRadius(4)
            Text('C62828').fontSize(7).margin({ top: 2 })
          }.margin({ right: 3 })
        }.width('100%')

        Divider().margin({ top: 15 })
        Text('103. 色卡�?3').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
            Column().width(35).height(35).backgroundColor('#FFBA68C8').borderRadius(4)
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
        Text('104. 色卡�?4').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('105. 色卡�?5').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })

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
        Text('106. 色卡�?6').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Divider().margin({ top: 15 })
        Text('300. 色卡300').fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
        Divider().margin({ top: 15, bottom: 30 })
      }
      .width('100%').padding(20)
    }
    .width('100%').height('100%').backgroundColor('#F5F5F5')
  }
}
