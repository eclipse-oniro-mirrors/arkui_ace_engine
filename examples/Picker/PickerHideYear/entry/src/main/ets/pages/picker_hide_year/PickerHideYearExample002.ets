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
struct PickerHideYearExample002 {
  @State selectedDate: Date = new Date('2025-06-15');
  @State isYearHidden: boolean = false;
  @State isLunar: boolean = false;
  @State logText: string = '';

  build() {
    Scroll() {
      Column() {
        Text('HideYear Dialog Example')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 10 });

        Text('DatePickerDialog with hide year option')
          .fontSize(14)
          .fontColor('#666666')
          .margin({ bottom: 20 });

        Column() {
          Text('Dialog Settings')
            .fontSize(16)
            .fontWeight(FontWeight.Medium)
            .margin({ bottom: 10 });

          Row() {
            Text('Hide Year:').fontSize(14).width(120)
            Toggle({ type: ToggleType.Switch, isOn: this.isYearHidden })
              .onChange((isOn: boolean) => {
                this.isYearHidden = isOn;
                this.addLog('Hide Year: ' + (isOn ? 'ON' : 'OFF'));
              })
              .selectedColor('#007DFF')
            Text(this.isYearHidden ? 'ON' : 'OFF').fontSize(14).margin({ left: 10 })
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
        }
        .width('90%')
        .padding(16)
        .backgroundColor('#F5F5F5')
        .borderRadius(12);

        Text('Dialog Mode: ' + (this.isYearHidden ? 'MONTH_AND_DAY (year hidden)' : 'DATE (year shown)'))
          .fontSize(16)
          .fontWeight(FontWeight.Medium)
          .fontColor(this.isYearHidden ? '#FA8C16' : '#1890FF')
          .margin({ top: 20, bottom: 10 });

        Button('Show DatePickerDialog')
          .width('90%')
          .height(50)
          .fontSize(18)
          .margin({ top: 10, bottom: 10 })
          .onClick(() => {
            this.showDatePickerDialog();
          });

        Button('Show TextPickerDialog (Month-Day only)')
          .width('90%')
          .height(50)
          .fontSize(16)
          .margin({ top: 10, bottom: 10 })
          .onClick(() => {
            this.showMonthDayPickerDialog();
          });

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

  showDatePickerDialog(): void {
    DatePickerDialog.show({
      start: new Date('2000-1-1'),
      end: new Date('2100-12-31'),
      selected: this.selectedDate,
      mode: this.isYearHidden ? DatePickerMode.MONTH_AND_DAY : DatePickerMode.DATE,
      lunar: this.isLunar,
      onDateAccept: (value: Date) => {
        this.selectedDate = value;
        this.addLog('onDateAccept: ' + this.formatDate(value));
      },
      onCancel: () => {
        this.addLog('onCancel');
      },
      onDateChange: (value: Date) => {
        this.addLog('onDateChange: ' + this.formatDate(value));
      }
    });
  }

  showMonthDayPickerDialog(): void {
    TextPickerDialog.show({
      range: this.getMonthDayRange(),
      selected: 0,
      onAccept: (value: TextPickerResult) => {
        this.addLog('TextPicker onAccept: ' + JSON.stringify(value.value));
      },
      onCancel: () => {
        this.addLog('TextPicker onCancel');
      }
    });
  }

  getMonthDayRange(): string[] {
    let range: string[] = [];
    for (let month = 1; month <= 12; month++) {
      let maxDay = this.getMaxDay(2025, month);
      for (let day = 1; day <= maxDay; day++) {
        range.push(`${month}月${day}日`);
      }
    }
    return range;
  }

  getMaxDay(year: number, month: number): number {
    const daysInMonth = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
    if (month === 2 && this.isLeapYear(year)) {
      return 29;
    }
    return daysInMonth[month - 1];
  }

  isLeapYear(year: number): boolean {
    return (year % 4 === 0 && year % 100 !== 0) || (year % 400 === 0);
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
