@Entry
@Component
struct DatePickerTestPage {
  @State isLunar: boolean = false
  @State selectedDate: Date = new Date('2026-08-08')
  @State dateLog: string = ''
  @State canLoop: boolean = true
  @State fontSize: number = 22

  @Builder
  sectionTitle(title: string) {
    Text(title).fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
  }

  build() {
    Scroll() {
      Column() {
        Text('DatePicker 测试')
          .fontSize(22).fontWeight(FontWeight.Bold).margin({ top: 30, bottom: 10 })

        this.sectionTitle('1. 基础日期选择器（公历/农历切换）')

        Button('切换公历农历').margin({ bottom: 10 }).onClick(() => { this.isLunar = !this.isLunar })
        Text(this.isLunar ? '当前: 农历' : '当前: 公历')
          .fontSize(14).fontColor('#007DFF').margin({ bottom: 10 })

        DatePicker({
          start: new Date('1970-1-1'),
          end: new Date('2100-1-1'),
          selected: this.selectedDate,
        })
          .lunar(this.isLunar)
          .onDateChange((value: Date) => {
            this.selectedDate = value
            this.dateLog = 'onDateChange: ' + value.toString()
          })

        Text(this.dateLog).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('2. 自定义文本样式')

        DatePicker({
          start: new Date('2000-1-1'),
          end: new Date('2030-12-31'),
          selected: new Date('2026-06-22'),
        })
          .disappearTextStyle({ color: '#004aaf', font: { size: 20, weight: FontWeight.Lighter } })
          .textStyle({ color: Color.Black, font: { size: this.fontSize, weight: FontWeight.Normal } })
          .selectedTextStyle({ color: Color.Blue, font: { size: this.fontSize + 4, weight: FontWeight.Bolder } })

        Text('选中字体大小: ' + (this.fontSize + 4)).fontSize(14).margin({ top: 5 })
        Slider({ value: this.fontSize, min: 16, max: 30, step: 1 })
          .width('70%').blockColor('#007DFF')
          .onChange((value: number) => { this.fontSize = Math.round(value) })

        Divider().margin({ top: 15 })
        this.sectionTitle('3. canLoop 循环滚动测试')

        Row() {
          Toggle({ type: ToggleType.Switch, isOn: true })
            .onChange((isOn: boolean) => { this.canLoop = isOn })
          Text('canLoop: ' + this.canLoop).fontSize(16)
        }.width('70%').margin({ bottom: 10 })

        DatePicker({
          start: new Date('2000-1-1'),
          end: new Date('2030-12-31'),
          selected: new Date('2026-06-22'),
        })
          .canLoop(this.canLoop)
          .onDateChange((value: Date) => { this.dateLog = 'loop change: ' + value.toString() })

        Divider().margin({ top: 15 })
        this.sectionTitle('4. 日期范围限制')

        DatePicker({
          start: new Date('2026-01-01'),
          end: new Date('2026-12-31'),
          selected: new Date('2026-06-22'),
        })
          .onDateChange((value: Date) => { this.dateLog = 'range change: ' + value.toString() })

        Text('范围: 2026-01-01 ~ 2026-12-31').fontSize(13).fontColor('#999999').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('5. 完整事件回调')

        DatePicker({
          start: new Date('2000-1-1'),
          end: new Date('2100-1-1'),
          selected: new Date('2026-10-01'),
        })
          .onDateChange((value: Date) => { this.dateLog = 'onDateChange: ' + value.toString() })

        Text(this.dateLog).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('6. 公历农历对比展示')

        Row() {
          Column() {
            Text('公历').fontSize(14).margin({ bottom: 5 })
            DatePicker({
              start: new Date('2000-1-1'),
              end: new Date('2030-12-31'),
              selected: new Date('2026-06-22'),
            }).lunar(false)
          }.width('48%')

          Column() {
            Text('农历').fontSize(14).margin({ bottom: 5 })
            DatePicker({
              start: new Date('2000-1-1'),
              end: new Date('2030-12-31'),
              selected: new Date('2026-06-22'),
            }).lunar(true)
          }.width('48%')
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('7. 不同选中日期初始化')

        Row() {
          Button('2026-01-01').fontSize(12).height(30).margin(2)
            .onClick(() => { this.selectedDate = new Date('2026-01-01') })
          Button('2026-06-22').fontSize(12).height(30).margin(2)
            .onClick(() => { this.selectedDate = new Date('2026-06-22') })
          Button('2026-12-31').fontSize(12).height(30).margin(2)
            .onClick(() => { this.selectedDate = new Date('2026-12-31') })
        }.margin({ bottom: 10 })

        DatePicker({
          start: new Date('2026-01-01'),
          end: new Date('2026-12-31'),
          selected: this.selectedDate,
        })
          .onDateChange((value: Date) => { this.dateLog = 'init onChange: ' + value.toString() })

        Text('当前选中: ' + this.selectedDate.toString()).fontSize(13).fontColor('#999999').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('8. 日期选择器样式对比')

        Row() {
          Column() {
            Text('默认样式').fontSize(14).margin({ bottom: 5 })
            DatePicker({
              start: new Date('2000-1-1'),
              end: new Date('2030-12-31'),
              selected: new Date('2026-06-22'),
            })
          }.width('48%')
          Column() {
            Text('自定义样式').fontSize(14).margin({ bottom: 5 })
            DatePicker({
              start: new Date('2000-1-1'),
              end: new Date('2030-12-31'),
              selected: new Date('2026-06-22'),
            })
              .disappearTextStyle({ color: '#FFA000', font: { size: 18, weight: FontWeight.Lighter } })
              .textStyle({ color: '#333333', font: { size: 22, weight: FontWeight.Normal } })
              .selectedTextStyle({ color: '#FF5722', font: { size: 26, weight: FontWeight.Bolder } })
          }.width('48%')
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('9. onScrollStop回调')

        DatePicker({
          start: new Date('2000-1-1'),
          end: new Date('2100-1-1'),
          selected: new Date('2026-10-23'),
        })
          .canLoop(true)
          .onDateChange((value: Date) => { this.dateLog = 'onDateChange: ' + value.toString() })

        Text(this.dateLog).fontSize(13).fontColor('#007DFF').margin({ top: 5 })
        Text('滚动DatePicker查看事件回调日志').fontSize(13).fontColor('#999999').margin({ top: 3 })
        Text('canLoop: true, 支持循环滚动选择').fontSize(13).fontColor('#999999').margin({ top: 3 })
        Text('监听onDateChange事件获取选中日期').fontSize(13).fontColor('#999999').margin({ top: 3 })

        Divider().margin({ top: 15 })
        this.sectionTitle('10. 不同字体大小对比')

        Row() {
          Column() {
            Text('size=18').fontSize(12).margin({ bottom: 5 })
            DatePicker({
              start: new Date('2000-1-1'),
              end: new Date('2030-12-31'),
              selected: new Date('2026-06-22'),
            })
              .textStyle({ color: Color.Black, font: { size: 18, weight: FontWeight.Normal } })
              .selectedTextStyle({ color: Color.Blue, font: { size: 22, weight: FontWeight.Bolder } })
          }.width('48%')
          Column() {
            Text('size=26').fontSize(12).margin({ bottom: 5 })
            DatePicker({
              start: new Date('2000-1-1'),
              end: new Date('2030-12-31'),
              selected: new Date('2026-06-22'),
            })
              .textStyle({ color: Color.Black, font: { size: 26, weight: FontWeight.Normal } })
              .selectedTextStyle({ color: Color.Red, font: { size: 30, weight: FontWeight.Bolder } })
          }.width('48%')
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('11. 日期范围限制对比')

        Row() {
          Column() {
            Text('宽范围').fontSize(12).margin({ bottom: 5 })
            DatePicker({
              start: new Date('2000-1-1'),
              end: new Date('2100-12-31'),
              selected: new Date('2026-06-22'),
            })
          }.width('48%')
          Column() {
            Text('窄范围').fontSize(12).margin({ bottom: 5 })
            DatePicker({
              start: new Date('2026-01-01'),
              end: new Date('2026-12-31'),
              selected: new Date('2026-06-22'),
            })
          }.width('48%')
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('12. canLoop对比展示')

        Row() {
          Column() {
            Text('canLoop=true').fontSize(12).margin({ bottom: 5 })
            DatePicker({
              start: new Date('2026-01-01'),
              end: new Date('2026-12-31'),
              selected: new Date('2026-06-22'),
            }).canLoop(true)
          }.width('48%')
          Column() {
            Text('canLoop=false').fontSize(12).margin({ bottom: 5 })
            DatePicker({
              start: new Date('2026-01-01'),
              end: new Date('2026-12-31'),
              selected: new Date('2026-06-22'),
            }).canLoop(false)
          }.width('48%')
        }.width('100%')

        Divider().margin({ top: 15, bottom: 30 })
      }
      .width('100%').padding(20)
    }
    .width('100%').height('100%').backgroundColor('#F5F5F5')
  }
}
