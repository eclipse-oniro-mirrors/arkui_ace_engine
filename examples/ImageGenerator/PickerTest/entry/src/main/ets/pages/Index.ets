import { router } from '@kit.ArkUI';

interface TestItem {
  name: string;
  page: string;
}

@Entry
@Component
struct Index {
  private testList: TestItem[] = [
    { name: 'TextPicker 测试', page: 'pages/TextPickerTestPage' },
    { name: 'DatePicker 测试', page: 'pages/DatePickerTestPage' },
    { name: 'TimePicker 测试', page: 'pages/TimePickerTestPage' },
    { name: 'CalendarPicker 测试', page: 'pages/CalendarPickerTestPage' },
    { name: 'PickerDialog 测试', page: 'pages/PickerDialogTestPage' },
    { name: 'UIPickerComponent 测试', page: 'pages/UIPickerComponentTestPage' },
  ];

  build() {
    Column() {
      Text('PickerTest 测试首页')
        .fontSize(24)
        .fontWeight(FontWeight.Bold)
        .margin({ top: 40, bottom: 30 })

      Text('请选择测试类型')
        .fontSize(16)
        .fontColor('#666666')
        .margin({ bottom: 20 })

      Scroll() {
        Column() {
          ForEach(this.testList, (item: TestItem) => {
            Button(item.name)
              .width('80%')
              .height(48)
              .fontSize(18)
              .margin({ top: 12, bottom: 12 })
              .onClick(() => {
                router.pushUrl({ url: item.page });
              })
          })
        }
        .width('100%')
        .padding({ bottom: 40 })
      }
      .width('100%')
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#F5F5F5')
  }
}
