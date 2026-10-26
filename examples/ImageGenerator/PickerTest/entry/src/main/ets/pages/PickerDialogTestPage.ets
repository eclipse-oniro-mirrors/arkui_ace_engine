import { LengthMetrics } from '@kit.ArkUI';

@Entry
@Component
struct PickerDialogTestPage {
  private select: number | number[] = 0
  private fruits: string[] = ['apple1', 'orange2', 'peach3', 'grape4', 'banana5']
  private multiRange: string[][] = [
    ['apple1', 'apple2', 'apple3'],
    ['orange1', 'orange2', 'orange3'],
    ['peach1', 'peach2', 'peach3'],
  ]
  private cascadeRange: TextCascadePickerRangeContent[] = [
    {
      text: '辽宁省',
      children: [
        { text: '沈阳市', children: [{ text: '沈河区' }, { text: '和平区' }] },
        { text: '大连市', children: [{ text: '中山区' }, { text: '金州区' }] },
      ],
    },
    {
      text: '吉林省',
      children: [
        { text: '长春市', children: [{ text: '南关区' }, { text: '宽城区' }] },
        { text: '四平市', children: [{ text: '铁西区' }, { text: '铁东区' }] },
      ],
    },
  ]
  private selectDate: Date = new Date('2026-1-1')
  private selectTime: Date = new Date('2026-12-25T08:30:00')
  @State dialogResult: string = '等待选择...'

  @Builder
  sectionTitle(title: string) {
    Text(title).fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
  }

  build() {
    Scroll() {
      Column() {
        Text('PickerDialog 测试')
          .fontSize(22).fontWeight(FontWeight.Bold).margin({ top: 30, bottom: 10 })

        Text(this.dialogResult).fontSize(16).fontColor('#007DFF').margin({ bottom: 10 })

        this.sectionTitle('1. TextPickerDialog 单列')

        Button('弹出 TextPickerDialog').width('80%').margin({ bottom: 10 })
          .onClick(() => {
            this.getUIContext().showTextPickerDialog({
              range: this.fruits,
              selected: this.select,
              defaultPickerItemHeight: 40,
              onAccept: (value: TextPickerResult) => {
                this.select = value.index
                this.dialogResult = 'TextPickerDialog: ' + value.value
              },
              onCancel: () => { this.dialogResult = 'TextPickerDialog: onCancel' },
              onChange: (value: TextPickerResult) => {
                console.info('TextPickerDialog: onChange ' + JSON.stringify(value))
              },
            })
          })

        Divider().margin({ top: 10 })
        this.sectionTitle('2. TextPickerDialog 多列')

        Button('弹出多列 TextPickerDialog').width('80%').margin({ bottom: 10 })
          .onClick(() => {
            this.getUIContext().showTextPickerDialog({
              range: this.multiRange,
              selected: [0, 0, 0],
              defaultPickerItemHeight: 40,
              onAccept: (value: TextPickerResult) => {
                this.dialogResult = '多列 TextPickerDialog: ' + JSON.stringify(value.value)
              },
              onCancel: () => { this.dialogResult = '多列 TextPickerDialog: onCancel' },
            })
          })

        Divider().margin({ top: 10 })
        this.sectionTitle('3. TextPickerDialog 联动')

        Button('弹出联动 TextPickerDialog').width('80%').margin({ bottom: 10 })
          .onClick(() => {
            this.getUIContext().showTextPickerDialog({
              range: this.cascadeRange,
              selected: 0,
              defaultPickerItemHeight: 40,
              onAccept: (value: TextPickerResult) => {
                this.dialogResult = '联动 TextPickerDialog: ' + JSON.stringify(value.value)
              },
              onCancel: () => { this.dialogResult = '联动 TextPickerDialog: onCancel' },
            })
          })

        Divider().margin({ top: 10 })
        this.sectionTitle('4. TextPickerDialog 自定义列宽')

        Button('弹出自定义列宽 TextPickerDialog').width('80%').margin({ bottom: 10 })
          .onClick(() => {
            this.getUIContext().showTextPickerDialog({
              range: this.multiRange,
              selected: [0, 0, 0],
              columnWidths: [LengthMetrics.vp(80), LengthMetrics.percent(40), LengthMetrics.vp(100)],
              onAccept: (value: TextPickerResult) => {
                this.dialogResult = '列宽 TextPickerDialog: ' + JSON.stringify(value.value)
              },
              onCancel: () => { this.dialogResult = '列宽 TextPickerDialog: onCancel' },
            })
          })

        Divider().margin({ top: 10 })
        this.sectionTitle('5. DatePickerDialog 基础')

        Button('弹出 DatePickerDialog').width('80%').margin({ bottom: 10 })
          .onClick(() => {
            this.getUIContext().showDatePickerDialog({
              start: new Date('2000-1-1'),
              end: new Date('2100-12-31'),
              selected: this.selectDate,
              showTime: true,
              useMilitaryTime: false,
              dateTimeOptions: { hour: 'numeric', minute: '2-digit' },
              onDateAccept: (value: Date) => {
                this.selectDate = value
                this.dialogResult = 'DatePickerDialog: ' + value.toString()
              },
              onCancel: () => { this.dialogResult = 'DatePickerDialog: onCancel' },
            })
          })

        Divider().margin({ top: 10 })
        this.sectionTitle('6. DatePickerDialog 农历')

        Button('弹出农历 DatePickerDialog').width('80%').margin({ bottom: 10 })
          .onClick(() => {
            this.getUIContext().showDatePickerDialog({
              start: new Date('2000-1-1'),
              end: new Date('2100-12-31'),
              selected: this.selectDate,
              lunar: true,
              onDateAccept: (value: Date) => {
                this.selectDate = value
                this.dialogResult = '农历 DatePickerDialog: ' + value.toString()
              },
              onCancel: () => { this.dialogResult = '农历 DatePickerDialog: onCancel' },
            })
          })

        Divider().margin({ top: 10 })
        this.sectionTitle('7. TimePickerDialog 12小时制')

        Button('弹出 12h TimePickerDialog').width('80%').margin({ bottom: 10 })
          .onClick(() => {
            this.getUIContext().showTimePickerDialog({
              selected: this.selectTime,
              format: TimePickerFormat.HOUR_MINUTE,
              useMilitaryTime: false,
              dateTimeOptions: { hour: 'numeric', minute: '2-digit' },
              onAccept: (value: TimePickerResult) => {
                if (value.hour != undefined && value.minute != undefined) {
                  this.selectTime.setHours(value.hour, value.minute)
                  this.dialogResult = '12h TimePickerDialog: ' + JSON.stringify(value)
                }
              },
              onCancel: () => { this.dialogResult = '12h TimePickerDialog: onCancel' },
            })
          })

        Divider().margin({ top: 10 })
        this.sectionTitle('8. TimePickerDialog 24小时制')

        Button('弹出 24h TimePickerDialog').width('80%').margin({ bottom: 10 })
          .onClick(() => {
            this.getUIContext().showTimePickerDialog({
              selected: this.selectTime,
              format: TimePickerFormat.HOUR_MINUTE_SECOND,
              useMilitaryTime: true,
              onAccept: (value: TimePickerResult) => {
                if (value.hour != undefined && value.minute != undefined) {
                  this.selectTime.setHours(value.hour, value.minute)
                  this.dialogResult = '24h TimePickerDialog: ' + JSON.stringify(value)
                }
              },
              onCancel: () => { this.dialogResult = '24h TimePickerDialog: onCancel' },
            })
          })

        Divider().margin({ top: 10 })
        this.sectionTitle('9. TextPickerDialog 样式自定义')

        Button('弹出样式自定义 TextPickerDialog').width('80%').margin({ bottom: 10 })
          .onClick(() => {
            this.getUIContext().showTextPickerDialog({
              range: this.fruits,
              selected: this.select,
              defaultPickerItemHeight: 50,
              canLoop: true,
              disappearTextStyle: { color: '#CCCCCC', font: { size: 14, weight: FontWeight.Lighter } },
              textStyle: { color: '#666666', font: { size: 18, weight: FontWeight.Normal } },
              selectedTextStyle: { color: '#FF5722', font: { size: 22, weight: FontWeight.Bolder } },
              onAccept: (value: TextPickerResult) => {
                this.select = value.index
                this.dialogResult = '样式 TextPickerDialog: ' + value.value
              },
              onCancel: () => { this.dialogResult = '样式 TextPickerDialog: onCancel' },
            })
          })

        Divider().margin({ top: 10 })
        this.sectionTitle('10. DatePickerDialog 不显示时间')

        Button('弹出无时间 DatePickerDialog').width('80%').margin({ bottom: 10 })
          .onClick(() => {
            this.getUIContext().showDatePickerDialog({
              start: new Date('2026-1-1'),
              end: new Date('2030-12-31'),
              selected: this.selectDate,
              showTime: false,
              onDateAccept: (value: Date) => {
                this.selectDate = value
                this.dialogResult = '无时间 DatePickerDialog: ' + value.toString()
              },
              onCancel: () => { this.dialogResult = '无时间 DatePickerDialog: onCancel' },
            })
          })

        Divider().margin({ top: 10 })
        this.sectionTitle('11. TimePickerDialog 样式自定义')

        Button('弹出样式 TimePickerDialog').width('80%').margin({ bottom: 10 })
          .onClick(() => {
            this.getUIContext().showTimePickerDialog({
              selected: this.selectTime,
              format: TimePickerFormat.HOUR_MINUTE,
              useMilitaryTime: true,
              disappearTextStyle: { color: '#FFA000', font: { size: 16, weight: FontWeight.Lighter } },
              textStyle: { color: '#333333', font: { size: 20, weight: FontWeight.Normal } },
              selectedTextStyle: { color: '#FF5722', font: { size: 24, weight: FontWeight.Bolder } },
              onAccept: (value: TimePickerResult) => {
                if (value.hour != undefined && value.minute != undefined) {
                  this.selectTime.setHours(value.hour, value.minute)
                  this.dialogResult = '样式 TimePickerDialog: ' + JSON.stringify(value)
                }
              },
              onCancel: () => { this.dialogResult = '样式 TimePickerDialog: onCancel' },
            })
          })

        Divider().margin({ top: 10 })
        this.sectionTitle('12. 多次弹出对比测试')

        Row() {
          Button('单列').fontSize(12).height(30).margin(2)
            .onClick(() => {
              this.getUIContext().showTextPickerDialog({
                range: this.fruits,
                selected: this.select,
                onAccept: (value: TextPickerResult) => {
                  this.dialogResult = '单列: ' + value.value
                },
              })
            })
          Button('多列').fontSize(12).height(30).margin(2)
            .onClick(() => {
              this.getUIContext().showTextPickerDialog({
                range: this.multiRange,
                selected: [0, 0, 0],
                onAccept: (value: TextPickerResult) => {
                  this.dialogResult = '多列: ' + JSON.stringify(value.value)
                },
              })
            })
          Button('联动').fontSize(12).height(30).margin(2)
            .onClick(() => {
              this.getUIContext().showTextPickerDialog({
                range: this.cascadeRange,
                selected: 0,
                onAccept: (value: TextPickerResult) => {
                  this.dialogResult = '联动: ' + JSON.stringify(value.value)
                },
              })
            })
        }.width('100%')

        Divider().margin({ top: 10 })
        this.sectionTitle('13. Dialog生命周期回调测试')

        Button('弹出带完整回调的Dialog').width('80%').margin({ bottom: 10 })
          .onClick(() => {
            this.getUIContext().showTextPickerDialog({
              range: this.fruits,
              selected: this.select,
              onWillAppear: () => { console.info('onWillAppear') },
              onDidAppear: () => { console.info('onDidAppear') },
              onWillDisappear: () => { console.info('onWillDisappear') },
              onDidDisappear: () => { console.info('onDidDisappear') },
              onAccept: (value: TextPickerResult) => {
                this.select = value.index
                this.dialogResult = '生命周期: ' + value.value
              },
              onCancel: () => { this.dialogResult = '生命周期: onCancel' },
            })
          })

        Text('查看hilog日志确认生命周期回调执行顺序')
          .fontSize(13).fontColor('#999999').margin({ top: 5 })

        Divider().margin({ top: 15, bottom: 30 })
      }
      .width('100%').padding(20)
    }
    .width('100%').height('100%').backgroundColor('#F5F5F5')
  }
}
