/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import router from '@ohos.router';

@Entry
@Component
struct PickerHideYearExample003 {
  @State selectedDate: Date = new Date('2025-06-15');
  @State isYearHidden: boolean = false;
  @State currentMode: DatePickerMode = DatePickerMode.DATE;
  @State isLunar: boolean = false;
  @State canLoop: boolean = true;
  @State textColor: string = '#182431';
  @State fontSize: number = 16;
  @State fontWeightNum: number = 400;
  @State dateOrder: string = 'yMd';
  @State logText: string = '';

  private dateOrderOptions: string[] = ['yMd', 'dMy', 'Md-y'];
  private dateOrderIndex: number = 0;

  private fontWeightOptions: number[] = [300, 400, 500, 700];
  private fontWeightIndex: number = 1;

  private textColorOptions: string[] = ['#182431', '#1890FF', '#FA8C16', '#52C41A'];
  private textColorIndex: number = 0;

  build() {
    Scroll() {
      Column() {
        Text('HideYear Advanced Example')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 10 });

        Text('Hide year with lunar, textStyle and dateOrder')
          .fontSize(14)
          .fontColor('#666666')
          .margin({ bottom: 20 });

        Column() {
          Text('Configuration')
            .fontSize(16)
            .fontWeight(FontWeight.Medium)
            .margin({ bottom: 10 });

          Row() {
            Text('Hide Year:').fontSize(14).width(120)
            Toggle({ type: ToggleType.Switch, isOn: this.isYearHidden })
              .onChange((isOn: boolean) => {
                this.isYearHidden = isOn;
                this.currentMode = this.isYearHidden ? DatePickerMode.MONTH_AND_DAY : DatePickerMode.DATE;
                this.addLog('Year ' + (this.isYearHidden ? 'hidden' : 'shown'));
              })
              .selectedColor('#007DFF')
            Text(this.isYearHidden ? 'Hidden' : 'Shown').fontSize(14).margin({ left: 10 })
          }
          .width('100%')
          .margin({ bottom: 10 })

          Row() {
            Text('Lunar:').fontSize(14).width(120)
            Toggle({ type: ToggleType.Switch, isOn: this.isLunar })
              .onChange((isOn: boolean) => {
                this.isLunar = isOn;
                this.addLog('Lunar: ' + (isOn ? 'ON' : 'OFF'));
              })
              .selectedColor('#007DFF')
            Text(this.isLunar ? 'ON' : 'OFF').fontSize(14).margin({ left: 10 })
          }
          .width('100%')
          .margin({ bottom: 10 })

          Row() {
            Text('Loop:').fontSize(14).width(120)
            Toggle({ type: ToggleType.Switch, isOn: this.canLoop })
              .onChange((isOn: boolean) => {
                this.canLoop = isOn;
                this.addLog('Loop: ' + (isOn ? 'ON' : 'OFF'));
              })
              .selectedColor('#007DFF')
            Text(this.canLoop ? 'ON' : 'OFF').fontSize(14).margin({ left: 10 })
          }
          .width('100%')
        }
        .width('90%')
        .padding(16)
        .backgroundColor('#F5F5F5')
        .borderRadius(12);

        Column() {
          Text('Text Style')
            .fontSize(16)
            .fontWeight(FontWeight.Medium)
            .margin({ bottom: 10 });

          Row() {
            Text('Font Size:').fontSize(14).width(120)
            Slider({ value: this.fontSize, min: 10, max: 30, step: 1 })
              .onChange((value: number) => {
                this.fontSize = Math.round(value);
                this.addLog('Font size: ' + this.fontSize);
              })
              .width('40%')
            Text(String(this.fontSize)).fontSize(14).margin({ left: 10 })
          }
          .width('100%')
          .margin({ bottom: 10 })

          Row() {
            Text('Font Weight:').fontSize(14).width(120)
            Button(this.fontWeightOptions[this.fontWeightIndex].toString())
              .onClick(() => {
                this.fontWeightIndex = (this.fontWeightIndex + 1) % this.fontWeightOptions.length;
                this.addLog('Font weight: ' + this.fontWeightOptions[this.fontWeightIndex]);
              })
              .margin({ left: 8 })
          }
          .width('100%')
          .margin({ bottom: 10 })

          Row() {
            Text('Text Color:').fontSize(14).width(120)
            ForEach(this.textColorOptions, (color: string, index: number) => {
              Button('')
                .width(30)
                .height(30)
                .backgroundColor(color)
                .borderRadius(15)
                .border(this.textColorIndex === index ? { width: 2, color: '#1890FF' } : { width: 0 })
                .onClick(() => {
                  this.textColorIndex = index;
                  this.textColor = color;
                  this.addLog('Text color: ' + color);
                })
                .margin({ right: 8 })
            })
          }
          .width('100%')
        }
        .width('90%')
        .margin({ top: 15 })
        .padding(16)
        .backgroundColor('#F5F5F5')
        .borderRadius(12);

        Column() {
          Text('Date Order')
            .fontSize(16)
            .fontWeight(FontWeight.Medium)
            .margin({ bottom: 10 });

          Row() {
            Text('Order:').fontSize(14).width(120)
            Button(this.dateOrderOptions[this.dateOrderIndex])
              .onClick(() => {
                this.dateOrderIndex = (this.dateOrderIndex + 1) % this.dateOrderOptions.length;
                this.dateOrder = this.dateOrderOptions[this.dateOrderIndex];
                this.addLog('Date order: ' + this.dateOrder);
              })
              .margin({ left: 8 })
          }
          .width('100%')
        }
        .width('90%')
        .margin({ top: 15 })
        .padding(16)
        .backgroundColor('#F5F5F5')
        .borderRadius(12);

        Text('Mode: ' + (this.isYearHidden ? 'MONTH_AND_DAY' : 'DATE') + ' | Order: ' + this.dateOrder)
          .fontSize(16)
          .fontWeight(FontWeight.Medium)
          .fontColor(this.isYearHidden ? '#FA8C16' : '#1890FF')
          .margin({ top: 20, bottom: 10 });

        DatePicker({
          start: new Date('1970-1-1'),
          end: new Date('2100-12-31'),
          selected: this.selectedDate,
          mode: this.currentMode
        })
          .lunar(this.isLunar)
          .canLoop(this.canLoop)
          .textStyle({
            color: this.textColor,
            fontSize: this.fontSize,
            weight: this.mapFontWeight(this.fontWeightOptions[this.fontWeightIndex])
          })
          .onDateChange((value: Date) => {
            this.selectedDate = value;
            this.addLog('Date changed: ' + this.formatDate(value));
          })
          .width('90%')
          .height(200);

        Column() {
          Text('Selected Date:')
            .fontSize(16)
            .fontWeight(FontWeight.Medium)
            .margin({ top: 20, bottom: 10 });

          Text(this.formatDate(this.selectedDate))
            .fontSize(20)
            .fontColor('#1890FF')
            .fontWeight(FontWeight.Bold);
        }
        .width('90%')
        .margin({ top: 20 })
        .padding(16)
        .backgroundColor('#F5F5F5')
        .borderRadius(12);

        Column() {
          Text('Event Log:')
            .fontSize(16)
            .fontWeight(FontWeight.Medium)
            .margin({ bottom: 10 });

          Scroll() {
            Text(this.logText)
              .fontSize(12)
              .fontColor('#333333')
              .width('100%')
          }
          .height(120)
          .width('100%')
        }
        .width('90%')
        .margin({ top: 20 })
        .padding(16)
        .backgroundColor('#FAFAFA')
        .borderRadius(12);

        Button('Back to Index')
          .margin({ top: 30, bottom: 20 })
          .onClick(() => {
            router.back();
          });
      }
      .width('100%')
      .padding(16);
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF');
  }

  mapFontWeight(weight: number): FontWeight {
    switch (weight) {
      case 300: return FontWeight.Lighter;
      case 500: return FontWeight.Medium;
      case 700: return FontWeight.Bolder;
      default: return FontWeight.Normal;
    }
  }

  formatDate(date: Date): string {
    const year = date.getFullYear();
    const month = String(date.getMonth() + 1).padStart(2, '0');
    const day = String(date.getDate()).padStart(2, '0');
    return `${year}-${month}-${day}`;
  }

  addLog(message: string) {
    const timestamp = new Date().toLocaleTimeString();
    this.logText = `[${timestamp}] ${message}\n` + this.logText;
  }
}
