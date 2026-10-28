import { LengthMetrics } from '@kit.ArkUI';

@Entry
@Component
struct UIPickerComponentTestPage {
  private dataArray: string[] = []
  private weekDays: string[] = ['周一', '周二', '周三', '周四', '周五', '周六', '周日']
  private monthArray: string[] = []

  @State loop: boolean = true
  @State hapticFeedback: boolean = true
  @State selectedIndex: number = 0
  @State bgColor: Color = Color.Blue
  @State onChangeDesc: string = ''
  @State onScrollStopDesc: string = ''

  @State provinceIndex: number = 0
  @State cityIndex: number = 0
  @State countyIndex: number = 0
  @State provinces: string[] = ['辽宁省', '吉林省', '黑龙江省']
  @State cities: string[] = ['沈阳市', '大连市']
  @State counties: string[] = ['沈河区', '和平区', '浑南区']

  private regionData: Record<string, Record<string, string[]>> = {
    '辽宁省': {
      '沈阳市': ['沈河区', '和平区', '浑南区'],
      '大连市': ['中山区', '金州区', '长海县'],
    },
    '吉林省': {
      '长春市': ['南关区', '宽城区', '朝阳区'],
      '四平市': ['铁西区', '铁东区', '梨树县'],
    },
    '黑龙江省': {
      '哈尔滨市': ['道里区', '道外区', '南岗区'],
      '牡丹江市': ['东安区', '西安区', '爱民区'],
    },
  }

  @Builder
  sectionTitle(title: string) {
    Text(title).fontSize(18).fontWeight(FontWeight.Medium).margin({ top: 20, bottom: 10 })
  }

  aboutToAppear(): void {
    for (let i = 1; i <= 10; i++) { this.dataArray.push(i.toString()) }
    for (let i = 1; i <= 12; i++) { this.monthArray.push(i + '月') }
    this.flushCityColumn()
  }

  flushCityColumn() {
    let currentProvince = this.provinces[this.provinceIndex]
    this.cities = Object.keys(this.regionData[currentProvince])
    this.cityIndex = 0
    this.flushCountyColumn()
  }

  flushCountyColumn() {
    let currentProvince = this.provinces[this.provinceIndex]
    let currentCity = this.cities[this.cityIndex]
    this.counties = this.regionData[currentProvince][currentCity]
    this.countyIndex = 0
  }

  build() {
    Scroll() {
      Column() {
        Text('UIPickerComponent 测试')
          .fontSize(22).fontWeight(FontWeight.Bold).margin({ top: 30, bottom: 10 })

        this.sectionTitle('1. 基础选择器')

        UIPickerComponent() {
          ForEach(this.dataArray, (item: string) => { Text(item) })
        }
        .width('70%').height(200).canLoop(true)
        .selectionIndicator({ type: PickerIndicatorType.BACKGROUND })
        .onChange((selectedIndex: number) => { this.onChangeDesc = 'onChange: ' + this.dataArray[selectedIndex] })
        .onScrollStop((selectedIndex: number) => { this.onScrollStopDesc = 'onScrollStop: ' + this.dataArray[selectedIndex] })

        Text(this.onChangeDesc).fontSize(13).fontColor('#007DFF').margin({ top: 5 })
        Text(this.onScrollStopDesc).fontSize(13).fontColor('#007DFF').margin({ top: 3 })

        Divider().margin({ top: 15 })
        this.sectionTitle('2. 循环滚动和触控反馈切换')

        UIPickerComponent() {
          ForEach(this.dataArray, (item: string) => { Text(item) })
        }
        .width('70%').height(200)
        .canLoop(this.loop).enableHapticFeedback(this.hapticFeedback)
        .selectionIndicator({ type: PickerIndicatorType.BACKGROUND })

        Row() {
          Toggle({ type: ToggleType.Switch, isOn: true })
            .onChange((isOn: boolean) => { this.loop = isOn })
          Text('canLoop: ' + this.loop).fontSize(16)
        }.width('70%').margin({ top: 5 })
        Row() {
          Toggle({ type: ToggleType.Switch, isOn: true })
            .onChange((isOn: boolean) => { this.hapticFeedback = isOn })
          Text('enableHapticFeedback: ' + this.hapticFeedback).fontSize(16)
        }.width('70%').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('3. 事件回调测试')

        UIPickerComponent() {
          ForEach(['待办', '进行中', '已完成', '已取消', '已延期'], (item: string) => { Text(item) })
        }
        .width('70%').height(200)
        .selectionIndicator({ type: PickerIndicatorType.BACKGROUND })
        .onChange((selectedIndex: number) => { this.onChangeDesc = 'onChange: index=' + selectedIndex })
        .onScrollStop((selectedIndex: number) => { this.onScrollStopDesc = 'onScrollStop: index=' + selectedIndex })

        Text(this.onChangeDesc).fontSize(13).fontColor('#007DFF').margin({ top: 5 })
        Text(this.onScrollStopDesc).fontSize(13).fontColor('#007DFF').margin({ top: 3 })

        Divider().margin({ top: 15 })
        this.sectionTitle('4. 设置选中项索引值')

        UIPickerComponent({ selectedIndex: this.selectedIndex }) {
          ForEach(this.dataArray, (item: string) => { Text(item) })
        }
        .width('70%').height(200)
        .selectionIndicator({ type: PickerIndicatorType.BACKGROUND })
        .onChange((selectedIndex: number) => { this.selectedIndex = selectedIndex })
        .onScrollStop((selectedIndex: number) => { this.selectedIndex = selectedIndex })

        Text('selectedIndex: ' + this.selectedIndex).fontSize(18).margin({ top: 5 })
        Row() {
          Button('0').fontSize(12).height(30).margin(2).onClick(() => { this.selectedIndex = 0 })
          Button('3').fontSize(12).height(30).margin(2).onClick(() => { this.selectedIndex = 3 })
          Button('9').fontSize(12).height(30).margin(2).onClick(() => { this.selectedIndex = 9 })
        }.width('100%').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('5. 选中项指示器（背景）颜色')

        UIPickerComponent() {
          ForEach(this.dataArray, (item: string) => { Text(item) })
        }
        .width('70%').height(200)
        .selectionIndicator({
          type: PickerIndicatorType.BACKGROUND,
          backgroundColor: this.bgColor,
          borderRadius: LengthMetrics.vp(10),
        })

        Row() {
          Button('蓝色').fontSize(12).height(30).margin(2).onClick(() => { this.bgColor = Color.Blue })
          Button('绿色').fontSize(12).height(30).margin(2).onClick(() => { this.bgColor = Color.Green })
          Button('红色').fontSize(12).height(30).margin(2).onClick(() => { this.bgColor = Color.Red })
        }.width('70%').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('6. 指示器类型对比 (BACKGROUND vs DIVIDER)')

        Row() {
          Column() {
            Text('BACKGROUND').fontSize(12).margin({ bottom: 10 })
            UIPickerComponent() {
              ForEach(this.dataArray, (item: string) => { Text(item).fontSize(14) })
            }
            .width('45%').height(150)
            .selectionIndicator({ type: PickerIndicatorType.BACKGROUND })
          }
          Column() {
            Text('DIVIDER').fontSize(12).margin({ bottom: 10 })
            UIPickerComponent() {
              ForEach(this.dataArray, (item: string) => { Text(item).fontSize(14) })
            }
            .width('45%').height(150)
            .selectionIndicator({ type: PickerIndicatorType.DIVIDER })
          }
        }.width('70%')

        Divider().margin({ top: 15 })
        this.sectionTitle('7. 无限循环星期选择')

        UIPickerComponent() {
          ForEach(this.weekDays, (day: string) => { Text(day).fontSize(18).fontWeight(FontWeight.Medium) })
        }
        .width('70%').height(200).canLoop(true)
        .selectionIndicator({ type: PickerIndicatorType.DIVIDER })
        .onChange((selectedIndex: number) => { this.onChangeDesc = 'Selected: ' + this.weekDays[selectedIndex] })

        Text(this.onChangeDesc).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('8. 自定义月份选择器')

        UIPickerComponent() {
          ForEach(this.monthArray, (item: string) => {
            Text(item).fontSize(20).textAlign(TextAlign.Center).fontColor(Color.Black)
          })
        }
        .width('70%').canLoop(true).enableHapticFeedback(false)
        .selectionIndicator({ type: PickerIndicatorType.DIVIDER })
        .onChange((idx: number) => { this.onChangeDesc = '月份: ' + this.monthArray[idx] })
        .onScrollStop((idx: number) => { this.onScrollStopDesc = 'stop: ' + this.monthArray[idx] })

        Text(this.onChangeDesc).fontSize(13).fontColor('#007DFF').margin({ top: 5 })
        Text(this.onScrollStopDesc).fontSize(13).fontColor('#007DFF').margin({ top: 3 })

        Divider().margin({ top: 15 })
        this.sectionTitle('9. 三级联动地区选择器')

        Row() {
          UIPickerComponent({ selectedIndex: this.provinceIndex }) {
            ForEach(this.provinces, (province: string) => { Text(province).fontSize(14) })
          }
          .onChange((selectedIndex: number) => { this.provinceIndex = selectedIndex; this.flushCityColumn() })
          .onScrollStop((selectedIndex: number) => { this.provinceIndex = selectedIndex })
          .selectionIndicator({ type: PickerIndicatorType.DIVIDER }).width('30%')

          UIPickerComponent({ selectedIndex: this.cityIndex }) {
            ForEach(this.cities, (city: string) => { Text(city).fontSize(14) })
          }
          .onChange((selectedIndex: number) => { this.cityIndex = selectedIndex; this.flushCountyColumn() })
          .onScrollStop((selectedIndex: number) => { this.cityIndex = selectedIndex })
          .selectionIndicator({ type: PickerIndicatorType.DIVIDER }).width('30%')

          UIPickerComponent({ selectedIndex: this.countyIndex }) {
            ForEach(this.counties, (county: string) => { Text(county).fontSize(14) })
          }
          .onChange((selectedIndex: number) => { this.countyIndex = selectedIndex })
          .onScrollStop((selectedIndex: number) => { this.countyIndex = selectedIndex })
          .selectionIndicator({ type: PickerIndicatorType.DIVIDER }).width('30%')
        }

        Text(this.provinces[this.provinceIndex] + ' / ' + this.cities[this.cityIndex] + ' / ' + this.counties[this.countyIndex])
          .fontSize(16).fontWeight(FontWeight.Medium).margin({ top: 10 })

        Divider().margin({ top: 15 })
        this.sectionTitle('10. DIVIDER指示器样式')

        UIPickerComponent() {
          ForEach(this.dataArray, (item: string) => { Text(item).fontSize(16).fontColor('#333333') })
        }
        .width('70%').height(200)
        .selectionIndicator({
          type: PickerIndicatorType.DIVIDER,
          dividerColor: '#007DFF',
          strokeWidth: LengthMetrics.vp(2),
          startMargin: LengthMetrics.vp(10),
          endMargin: LengthMetrics.vp(10),
        })
        .onChange((selectedIndex: number) => { this.onChangeDesc = 'divider onChange: ' + this.dataArray[selectedIndex] })

        Text(this.onChangeDesc).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('11. 自定义字体和间距')

        UIPickerComponent() {
          ForEach(this.dataArray, (item: string) => {
            Text(item).fontSize(16).fontColor('#333333').fontWeight(FontWeight.Normal)
          })
        }
        .width('70%').height(200).canLoop(true)
        .selectionIndicator({
          type: PickerIndicatorType.BACKGROUND,
          backgroundColor: '#E3F2FD',
          borderRadius: LengthMetrics.vp(8),
        })

        Text('背景: #E3F2FD, 圆角: 8vp').fontSize(13).fontColor('#999999').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('12. 无指示器模式')

        UIPickerComponent() {
          ForEach(this.dataArray, (item: string) => { Text(item).fontSize(18) })
        }
        .width('70%').height(150).canLoop(true)

        Text('未设置selectionIndicator').fontSize(13).fontColor('#999999').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('13. 不同borderRadius对比')

        Row() {
          Column() {
            Text('r=0').fontSize(12).margin({ bottom: 5 })
            UIPickerComponent() {
              ForEach(this.dataArray, (item: string) => { Text(item).fontSize(14) })
            }
            .width('45%').height(150)
            .selectionIndicator({
              type: PickerIndicatorType.BACKGROUND,
              borderRadius: LengthMetrics.vp(0),
            })
          }
          Column() {
            Text('r=20').fontSize(12).margin({ bottom: 5 })
            UIPickerComponent() {
              ForEach(this.dataArray, (item: string) => { Text(item).fontSize(14) })
            }
            .width('45%').height(150)
            .selectionIndicator({
              type: PickerIndicatorType.BACKGROUND,
              borderRadius: LengthMetrics.vp(20),
            })
          }
        }.width('70%')

        Divider().margin({ top: 15 })
        this.sectionTitle('14. 多列并排选择器')

        Row() {
          Column() {
            Text('年').fontSize(12).margin({ bottom: 5 })
            UIPickerComponent() {
              ForEach(['2024', '2025', '2026'], (item: string) => { Text(item).fontSize(16) })
            }
            .width('90%').height(150)
            .selectionIndicator({ type: PickerIndicatorType.BACKGROUND })
          }.width('33%')
          Column() {
            Text('月').fontSize(12).margin({ bottom: 5 })
            UIPickerComponent() {
              ForEach(['1月', '2月', '3月', '4月', '5月', '6月'], (item: string) => { Text(item).fontSize(16) })
            }
            .width('90%').height(150)
            .selectionIndicator({ type: PickerIndicatorType.BACKGROUND })
          }.width('33%')
          Column() {
            Text('日').fontSize(12).margin({ bottom: 5 })
            UIPickerComponent() {
              ForEach(['1', '2', '3', '4', '5', '6'], (item: string) => { Text(item).fontSize(16) })
            }
            .width('90%').height(150)
            .selectionIndicator({ type: PickerIndicatorType.DIVIDER })
          }.width('33%')
        }.width('100%')

        Text('多列并排选择器，支持不同指示器类型')
          .fontSize(13).fontColor('#999999').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('15. DIVIDER指示器边距对比')

        Row() {
          Column() {
            Text('margin=0').fontSize(12).margin({ bottom: 5 })
            UIPickerComponent() {
              ForEach(this.dataArray, (item: string) => { Text(item).fontSize(14) })
            }
            .width('90%').height(150)
            .selectionIndicator({
              type: PickerIndicatorType.DIVIDER,
              startMargin: LengthMetrics.vp(0),
              endMargin: LengthMetrics.vp(0),
            })
          }.width('48%')
          Column() {
            Text('margin=20').fontSize(12).margin({ bottom: 5 })
            UIPickerComponent() {
              ForEach(this.dataArray, (item: string) => { Text(item).fontSize(14) })
            }
            .width('90%').height(150)
            .selectionIndicator({
              type: PickerIndicatorType.DIVIDER,
              startMargin: LengthMetrics.vp(20),
              endMargin: LengthMetrics.vp(20),
            })
          }.width('48%')
        }.width('100%')

        Divider().margin({ top: 15, bottom: 30 })
      }
      .width('100%').padding(20)
    }
    .width('100%').height('100%').backgroundColor('#F5F5F5')
  }
}
