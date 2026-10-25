@Entry
@Component
struct TimePickerTestPage {
  private selectedTime: Date = new Date('2026-07-22T08:00:00')
  @State useMilitary: boolean = true
  @State timeLog: string = ''
  @State fontSize: number = 26

  @Builder
  sectionTitle(title: string) {
    Text(title).fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
  }

  build() {
    Scroll() {
      Column() {
        Text('TimePicker 测试')
          .fontSize(22).fontWeight(FontWeight.Bold).margin({ top: 30, bottom: 10 })

        this.sectionTitle('1. 基础时间选择器')

        TimePicker({ selected: this.selectedTime })
          .disappearTextStyle({ color: '#004aaf', font: { size: 24, weight: FontWeight.Lighter } })
          .textStyle({ color: Color.Black, font: { size: 26, weight: FontWeight.Normal } })
          .selectedTextStyle({ color: Color.Blue, font: { size: 30, weight: FontWeight.Bolder } })
          .onChange((value: TimePickerResult) => {
            if (value.hour >= 0) {
              this.selectedTime.setHours(value.hour, value.minute)
              this.timeLog = 'onChange: ' + JSON.stringify(value)
            }
          })

        Text(this.timeLog).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('2. 12/24小时制切换')

        Row() {
          Button('24小时制').fontSize(12).height(30).margin(2).onClick(() => { this.useMilitary = true })
          Button('12小时制').fontSize(12).height(30).margin(2).onClick(() => { this.useMilitary = false })
        }.margin({ bottom: 10 })

        Text(this.useMilitary ? '当前: 24小时制' : '当前: 12小时制')
          .fontSize(14).fontColor('#007DFF').margin({ bottom: 10 })

        TimePicker({ selected: this.selectedTime })
          .useMilitaryTime(this.useMilitary)
          .onChange((value: TimePickerResult) => {
            if (value.hour >= 0) { this.timeLog = 'onChange: ' + JSON.stringify(value) }
          })

        Text(this.timeLog).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('3. 12小时制AM/PM显示')

        TimePicker({ selected: new Date('2026-07-22T14:30:00') })
          .useMilitaryTime(false)
          .onChange((value: TimePickerResult) => {
            if (value.hour >= 0) { this.timeLog = 'AM/PM onChange: ' + JSON.stringify(value) }
          })

        Text('12小时制, 初始14:30 → 2:30 PM').fontSize(13).fontColor('#999999').margin({ top: 5 })
        Text(this.timeLog).fontSize(13).fontColor('#007DFF').margin({ top: 3 })

        Divider().margin({ top: 15 })
        this.sectionTitle('4. 自定义文本样式')

        TimePicker({ selected: this.selectedTime })
          .disappearTextStyle({ color: '#CCCCCC', font: { size: this.fontSize - 4, weight: FontWeight.Lighter } })
          .textStyle({ color: Color.Black, font: { size: this.fontSize, weight: FontWeight.Normal } })
          .selectedTextStyle({ color: Color.Red, font: { size: this.fontSize + 4, weight: FontWeight.Bolder } })

        Text('选中字体大小: ' + (this.fontSize + 4)).fontSize(14).margin({ top: 5 })
        Slider({ value: this.fontSize, min: 18, max: 34, step: 1 })
          .width('70%').blockColor('#007DFF')
          .onChange((value: number) => { this.fontSize = Math.round(value) })

        Divider().margin({ top: 15 })
        this.sectionTitle('5. 24小时制时分秒')

        TimePicker({ selected: new Date('2026-07-22T08:30:45') })
          .useMilitaryTime(true)
          .onChange((value: TimePickerResult) => {
            if (value.hour >= 0) { this.timeLog = '24h onChange: ' + JSON.stringify(value) }
          })

        Text('useMilitaryTime: true, 时分显示').fontSize(13).fontColor('#999999').margin({ top: 5 })
        Text(this.timeLog).fontSize(13).fontColor('#007DFF').margin({ top: 3 })

        Divider().margin({ top: 15 })
        this.sectionTitle('6. 完整事件回调')

        TimePicker({ selected: this.selectedTime })
          .onChange((value: TimePickerResult) => {
            if (value.hour >= 0) { this.timeLog = 'onChange: ' + JSON.stringify(value) }
          })

        Text(this.timeLog).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('7. 多个TimePicker对比')

        Row() {
          Column() {
            Text('24h制').fontSize(14).margin({ bottom: 5 })
            TimePicker({ selected: this.selectedTime }).useMilitaryTime(true)
          }.width('48%')
          Column() {
            Text('12h制').fontSize(14).margin({ bottom: 5 })
            TimePicker({ selected: this.selectedTime }).useMilitaryTime(false)
          }.width('48%')
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('8. 带秒的选择器')

        TimePicker({ selected: new Date('2026-07-22T08:30:45') })
          .useMilitaryTime(true)
          .onChange((value: TimePickerResult) => {
            if (value.hour >= 0) { this.timeLog = '带秒 onChange: ' + JSON.stringify(value) }
          })

        Text('useMilitaryTime: true, 含秒显示').fontSize(13).fontColor('#999999').margin({ top: 5 })
        Text(this.timeLog).fontSize(13).fontColor('#007DFF').margin({ top: 3 })

        Divider().margin({ top: 15 })
        this.sectionTitle('9. 样式对比展示')

        Row() {
          Column() {
            Text('默认样式').fontSize(14).margin({ bottom: 5 })
            TimePicker({ selected: this.selectedTime })
          }.width('48%')
          Column() {
            Text('自定义样式').fontSize(14).margin({ bottom: 5 })
            TimePicker({ selected: this.selectedTime })
              .disappearTextStyle({ color: '#FFA000', font: { size: 20, weight: FontWeight.Lighter } })
              .textStyle({ color: '#333333', font: { size: 24, weight: FontWeight.Normal } })
              .selectedTextStyle({ color: '#FF5722', font: { size: 28, weight: FontWeight.Bolder } })
          }.width('48%')
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('10. 三种选中颜色对比')

        Row() {
          Column() {
            Text('蓝色选中').fontSize(12).margin({ bottom: 5 })
            TimePicker({ selected: this.selectedTime })
              .selectedTextStyle({ color: Color.Blue, font: { size: 28, weight: FontWeight.Bolder } })
          }.width('33%')
          Column() {
            Text('红色选中').fontSize(12).margin({ bottom: 5 })
            TimePicker({ selected: this.selectedTime })
              .selectedTextStyle({ color: Color.Red, font: { size: 28, weight: FontWeight.Bolder } })
          }.width('33%')
          Column() {
            Text('绿色选中').fontSize(12).margin({ bottom: 5 })
            TimePicker({ selected: this.selectedTime })
              .selectedTextStyle({ color: Color.Green, font: { size: 28, weight: FontWeight.Bolder } })
          }.width('33%')
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('11. 时分秒格式选择器')

        TimePicker({ selected: new Date('2026-07-22T08:30:45') })
          .useMilitaryTime(true)
          .onChange((value: TimePickerResult) => {
            if (value.hour >= 0) {
              this.timeLog = '时分秒 onChange: ' + JSON.stringify(value)
            }
          })

        Text('useMilitaryTime: true, 包含秒')
          .fontSize(13).fontColor('#999999').margin({ top: 5 })
        Text(this.timeLog).fontSize(13).fontColor('#007DFF').margin({ top: 3 })

        Divider().margin({ top: 15 })
        this.sectionTitle('12. 多个TimePicker对比')

        Row() {
          Column() {
            Text('24h制').fontSize(14).margin({ bottom: 5 })
            TimePicker({ selected: this.selectedTime })
              .useMilitaryTime(true)
          }.width('48%')
          Column() {
            Text('12h制').fontSize(14).margin({ bottom: 5 })
            TimePicker({ selected: this.selectedTime })
              .useMilitaryTime(false)
          }.width('48%')
        }.width('100%')

        Divider().margin({ top: 15, bottom: 30 })
      }
      .width('100%').padding(20)
    }
    .width('100%').height('100%').backgroundColor('#F5F5F5')
  }
}
