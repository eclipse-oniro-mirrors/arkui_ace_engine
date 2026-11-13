@Entry
@Component
struct CalendarPickerTestPage {
  @State selectedDate: Date = new Date('2026-03-05')
  @State calAlign: CalendarAlign = CalendarAlign.END
  @State hintRadius: number = 10
  @State bgColor: string = '#F1F3F5'
  @State calLog: string = ''
  @State textColor: string = '#ff182431'
  @State textFontSize: number = 20
  @State textFontWeight: FontWeight = FontWeight.Normal

  @Builder
  sectionTitle(title: string) {
    Text(title).fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
  }

  build() {
    Scroll() {
      Column() {
        Text('CalendarPicker 测试')
          .fontSize(22).fontWeight(FontWeight.Bold).margin({ top: 30, bottom: 10 })

        this.sectionTitle('1. 基础日历选择器')

        CalendarPicker({ hintRadius: 10, selected: this.selectedDate })
          .edgeAlign(CalendarAlign.END)
          .textStyle({ color: '#ff182431', font: { size: 20, weight: FontWeight.Normal } })
          .onChange((value) => { this.calLog = '选中日期: ' + value.toString() })

        Text(this.calLog).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('2. 边缘对齐方式')

        CalendarPicker({ hintRadius: 10, selected: this.selectedDate })
          .edgeAlign(this.calAlign)
          .textStyle({ color: '#ff182431', font: { size: 20, weight: FontWeight.Normal } })
          .onChange((value) => { this.calLog = 'align onChange: ' + value.toString() })

        Row() {
          Button('START').fontSize(12).height(30).margin(2).onClick(() => { this.calAlign = CalendarAlign.START })
          Button('CENTER').fontSize(12).height(30).margin(2).onClick(() => { this.calAlign = CalendarAlign.CENTER })
          Button('END').fontSize(12).height(30).margin(2).onClick(() => { this.calAlign = CalendarAlign.END })
        }.width('100%').margin({ top: 5 })
        Text(this.calLog).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('3. 设置背景色')

        CalendarPicker({ hintRadius: 10, selected: this.selectedDate })
          .edgeAlign(CalendarAlign.END)
          .textStyle({ color: '#ff182431', font: { size: 20, weight: FontWeight.Normal } })
          .backgroundColor(this.bgColor)
          .onChange((value) => { this.calLog = 'bgColor onChange: ' + value.toString() })

        Row() {
          Button('灰色').fontSize(12).height(30).margin(2).onClick(() => { this.bgColor = '#F1F3F5' })
          Button('浅蓝').fontSize(12).height(30).margin(2).onClick(() => { this.bgColor = '#E3F2FD' })
          Button('浅绿').fontSize(12).height(30).margin(2).onClick(() => { this.bgColor = '#E8F5E9' })
        }.width('100%').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('4. 设置提示圆角')

        CalendarPicker({ hintRadius: this.hintRadius, selected: this.selectedDate })
          .edgeAlign(CalendarAlign.END)
          .textStyle({ color: '#ff182431', font: { size: 20, weight: FontWeight.Normal } })
          .onChange((value) => { this.calLog = 'hintRadius onChange: ' + value.toString() })

        Text('hintRadius: ' + this.hintRadius).fontSize(16).margin({ top: 5 })
        Row() {
          Button('0').fontSize(12).height(30).margin(2).onClick(() => { this.hintRadius = 0 })
          Button('10').fontSize(12).height(30).margin(2).onClick(() => { this.hintRadius = 10 })
          Button('20').fontSize(12).height(30).margin(2).onClick(() => { this.hintRadius = 20 })
          Button('30').fontSize(12).height(30).margin(2).onClick(() => { this.hintRadius = 30 })
        }.width('100%').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('5. 自定义文本样式')

        CalendarPicker({ hintRadius: 10, selected: this.selectedDate })
          .edgeAlign(CalendarAlign.END)
          .textStyle({ color: this.textColor, font: { size: this.textFontSize, weight: this.textFontWeight } })
          .onChange((value) => { this.calLog = 'textStyle onChange: ' + value.toString() })

        Row() {
          Button('黑色').fontSize(12).height(30).margin(2).onClick(() => { this.textColor = '#ff182431' })
          Button('红色').fontSize(12).height(30).margin(2).onClick(() => { this.textColor = '#ffff0000' })
          Button('蓝色').fontSize(12).height(30).margin(2).onClick(() => { this.textColor = '#ff0000FF' })
        }.width('100%').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('6. 字体粗细对比')

        Row() {
          Column() {
            Text('Normal').fontSize(12).margin({ bottom: 5 })
            CalendarPicker({ hintRadius: 10, selected: this.selectedDate })
              .edgeAlign(CalendarAlign.END)
              .textStyle({ color: '#ff182431', font: { size: 18, weight: FontWeight.Normal } })
          }.width('48%')
          Column() {
            Text('Bold').fontSize(12).margin({ bottom: 5 })
            CalendarPicker({ hintRadius: 10, selected: this.selectedDate })
              .edgeAlign(CalendarAlign.END)
              .textStyle({ color: '#ff182431', font: { size: 18, weight: FontWeight.Bold } })
          }.width('48%')
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('7. 不同选中日期对比')

        Row() {
          Column() {
            Text('2026-01').fontSize(12).margin({ bottom: 5 })
            CalendarPicker({ hintRadius: 10, selected: new Date('2026-01-15') })
              .edgeAlign(CalendarAlign.END)
              .textStyle({ color: '#ff182431', font: { size: 14, weight: FontWeight.Normal } })
          }.width('48%')
          Column() {
            Text('2026-06').fontSize(12).margin({ bottom: 5 })
            CalendarPicker({ hintRadius: 10, selected: new Date('2026-06-20') })
              .edgeAlign(CalendarAlign.END)
              .textStyle({ color: '#ff182431', font: { size: 14, weight: FontWeight.Normal } })
          }.width('48%')
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('8. 不同对齐方式与圆角组合')

        Row() {
          Column() {
            Text('START + r=0').fontSize(12).margin({ bottom: 5 })
            CalendarPicker({ hintRadius: 0, selected: this.selectedDate })
              .edgeAlign(CalendarAlign.START)
              .textStyle({ color: '#ff182431', font: { size: 16, weight: FontWeight.Normal } })
          }.width('48%')
          Column() {
            Text('CENTER + r=20').fontSize(12).margin({ bottom: 5 })
            CalendarPicker({ hintRadius: 20, selected: this.selectedDate })
              .edgeAlign(CalendarAlign.CENTER)
              .textStyle({ color: '#ff182431', font: { size: 16, weight: FontWeight.Normal } })
          }.width('48%')
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('9. 多种背景色对比')

        Row() {
          Column() {
            Text('灰色背景').fontSize(12).margin({ bottom: 5 })
            CalendarPicker({ hintRadius: 10, selected: this.selectedDate })
              .edgeAlign(CalendarAlign.END)
              .textStyle({ color: '#ff182431', font: { size: 14, weight: FontWeight.Normal } })
              .backgroundColor('#F1F3F5')
          }.width('48%')
          Column() {
            Text('蓝色背景').fontSize(12).margin({ bottom: 5 })
            CalendarPicker({ hintRadius: 10, selected: this.selectedDate })
              .edgeAlign(CalendarAlign.END)
              .textStyle({ color: '#ff182431', font: { size: 14, weight: FontWeight.Normal } })
              .backgroundColor('#E3F2FD')
          }.width('48%')
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('10. 综合属性测试')

        CalendarPicker({ hintRadius: this.hintRadius, selected: this.selectedDate })
          .edgeAlign(this.calAlign)
          .textStyle({ color: this.textColor, font: { size: this.textFontSize, weight: this.textFontWeight } })
          .backgroundColor(this.bgColor)
          .onChange((value) => { this.calLog = '综合 onChange: ' + value.toString() })

        Text('对齐: ' + this.calAlign + ' 圆角: ' + this.hintRadius + ' 颜色: ' + this.textColor)
          .fontSize(13).fontColor('#999999').margin({ top: 5 })
        Text(this.calLog).fontSize(13).fontColor('#007DFF').margin({ top: 3 })

        Divider().margin({ top: 15 })
        this.sectionTitle('11. 完整事件回调展示')

        CalendarPicker({ hintRadius: 10, selected: new Date('2026-01-01') })
          .edgeAlign(CalendarAlign.CENTER)
          .textStyle({ color: '#ff182431', font: { size: 20, weight: FontWeight.Normal } })
          .onChange((value) => { this.calLog = 'onCalendarChange: ' + value.toString() })

        Text(this.calLog).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('12. hintRadius动态调整')

        CalendarPicker({ hintRadius: this.hintRadius, selected: this.selectedDate })
          .edgeAlign(CalendarAlign.END)
          .textStyle({ color: '#ff182431', font: { size: 18, weight: FontWeight.Normal } })
          .backgroundColor('#E3F2FD')
          .onChange((value) => { this.calLog = '动态 onChange: ' + value.toString() })

        Slider({ value: this.hintRadius, min: 0, max: 30, step: 1 })
          .width('70%').blockColor('#007DFF')
          .onChange((value: number) => { this.hintRadius = Math.round(value) })

        Text('hintRadius: ' + this.hintRadius).fontSize(14).margin({ top: 5 })
        Text(this.calLog).fontSize(13).fontColor('#007DFF').margin({ top: 3 })

        Divider().margin({ top: 15, bottom: 30 })
      }
      .width('100%').padding(20)
    }
    .width('100%').height('100%').backgroundColor('#F5F5F5')
  }
}
