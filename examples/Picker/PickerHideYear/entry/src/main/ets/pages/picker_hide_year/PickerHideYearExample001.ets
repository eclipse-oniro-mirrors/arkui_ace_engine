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
struct PickerHideYearExample001 {
  @State selectedDate: Date = new Date('2025-06-15');
  @State isYearHidden: boolean = false;
  @State currentMode: DatePickerMode = DatePickerMode.DATE;
  @State isLunar: boolean = false;
  @State canLoop: boolean = true;
  @State logText: string = '';

  build() {
    Scroll() {
      Column() {
        Text('HideYear Basic Example')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 10 });

        Text('Toggle hide/show year column with DatePicker mode')
          .fontSize(14)
          .fontColor('#666666')
          .margin({ bottom: 20 });

        Column() {
          Text('Year Visibility')
            .fontSize(16)
            .fontWeight(FontWeight.Medium)
            .margin({ bottom: 10 });

          Row() {
            Text('Show Year:').fontSize(14).width(120)
            Toggle({ type: ToggleType.Switch, isOn: !this.isYearHidden })
              .onChange((isOn: boolean) => {
                this.isYearHidden = !isOn;
                this.currentMode = this.isYearHidden ? DatePickerMode.MONTH_AND_DAY : DatePickerMode.DATE;
                this.addLog('Year ' + (this.isYearHidden ? 'hidden' : 'shown') + ', mode: ' + (this.isYearHidden ? 'MONTH_AND_DAY' : 'DATE'));
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

        Text('Current Mode: ' + (this.isYearHidden ? 'MONTH_AND_DAY (year hidden)' : 'DATE (year shown)'))
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
          .height(100)
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
