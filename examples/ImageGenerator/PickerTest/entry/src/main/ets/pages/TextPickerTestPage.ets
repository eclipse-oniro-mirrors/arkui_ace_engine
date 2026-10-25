import { LengthMetrics } from '@kit.ArkUI';

@Entry
@Component
struct TextPickerTestPage {
  private singleFruits: string[] = ['apple1', 'apple2', 'apple3', 'apple4', 'apple5', 'apple6']
  private multiFruits: string[][] = [
    ['apple1', 'apple2', 'apple3', 'apple4'],
    ['orange1', 'orange2', 'orange3', 'orange4'],
    ['peach1', 'peach2', 'peach3', 'peach4'],
  ]
  private cascadeData: TextCascadePickerRangeContent[] = [
    {
      text: '辽宁省',
      children: [
        { text: '沈阳市', children: [{ text: '沈河区' }, { text: '和平区' }, { text: '浑南区' }] },
        { text: '大连市', children: [{ text: '中山区' }, { text: '金州区' }, { text: '长海县' }] },
      ],
    },
    {
      text: '吉林省',
      children: [
        { text: '长春市', children: [{ text: '南关区' }, { text: '宽城区' }, { text: '朝阳区' }] },
        { text: '四平市', children: [{ text: '铁西区' }, { text: '铁东区' }, { text: '梨树县' }] },
      ],
    },
  ]
  private rangeContent: TextPickerRangeContent[] = [
    { text: 'apple', icon: $r('app.media.startIcon') },
    { text: 'orange', icon: $r('app.media.startIcon') },
    { text: 'peach', icon: $r('app.media.startIcon') },
    { text: 'grape', icon: $r('app.media.startIcon') },
  ]
  @State selectedIdx: number = 2
  @State resultText: string = ''
  @State isDisabled: boolean = false
  @State fontSize: number = 18

  @Builder
  sectionTitle(title: string) {
    Text(title)
      .fontSize(18)
      .fontWeight(FontWeight.Medium)
      .margin({ top: 20, bottom: 10 })
  }

  build() {
    Scroll() {
      Column() {
        Text('TextPicker 测试')
          .fontSize(22)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 30, bottom: 10 })

        this.sectionTitle('1. 基础单列选择器')

        TextPicker({ range: this.singleFruits, selected: 1 })
          .onChange((value: string | string[], index: number | number[]) => {
            this.resultText = 'onChange: ' + value + ', index: ' + index
          })
          .onScrollStop((value: string | string[], index: number | number[]) => {
            this.resultText = 'onScrollStop: ' + value + ', index: ' + index
          })

        Text(this.resultText).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('2. 多列选择器')

        TextPicker({ range: this.multiFruits })
          .onChange((value: string | string[], index: number | number[]) => {
            this.resultText = '多列 onChange: ' + JSON.stringify(value)
          })
        Text(this.resultText).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('3. 多列联动选择器（省市区）')

        TextPicker({ range: this.cascadeData })
          .onChange((value: string | string[], index: number | number[]) => {
            this.resultText = '联动 onChange: ' + JSON.stringify(value)
          })

        Divider().margin({ top: 15 })
        this.sectionTitle('4. 带图标的RangeContent选择器')

        TextPicker({ range: this.rangeContent, selected: 0 })
          .onChange((value: string | string[], index: number | number[]) => {
            this.resultText = 'RangeContent onChange: ' + value
          })

        Divider().margin({ top: 15 })
        this.sectionTitle('5. 自定义列宽')

        TextPicker({
          range: this.multiFruits,
          columnWidths: [LengthMetrics.vp(80), LengthMetrics.percent(40), LengthMetrics.vp(100)],
        })
          .onChange((value: string | string[], index: number | number[]) => {
            this.resultText = '列宽 onChange: ' + JSON.stringify(value)
          })

        Divider().margin({ top: 15 })
        this.sectionTitle('6. 设置选中项索引')

        TextPicker({ range: this.singleFruits, selected: this.selectedIdx })
          .onChange((value: string | string[], index: number | number[]) => {
            this.selectedIdx = index as number
          })

        Text('selectedIndex: ' + this.selectedIdx).fontSize(16).margin({ top: 5 })
        Row() {
          Button('0').fontSize(12).height(30).margin(2).onClick(() => { this.selectedIdx = 0 })
          Button('2').fontSize(12).height(30).margin(2).onClick(() => { this.selectedIdx = 2 })
          Button('4').fontSize(12).height(30).margin(2).onClick(() => { this.selectedIdx = 4 })
        }.width('100%').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('7. 禁用状态')

        TextPicker({ range: this.singleFruits }).enabled(!this.isDisabled)
        Row() {
          Toggle({ type: ToggleType.Switch, isOn: false })
            .onChange((isOn: boolean) => { this.isDisabled = isOn })
          Text('禁用选择器').fontSize(16)
        }.width('70%').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('8. 自定义样式')

        TextPicker({ range: this.singleFruits })
          .selectedTextStyle({ color: Color.Blue, font: { size: this.fontSize } })
          .textStyle({ color: '#999999', font: { size: this.fontSize - 2 } })
          .disappearTextStyle({ color: '#CCCCCC', font: { size: this.fontSize - 4 } })

        Text('选中字体大小: ' + this.fontSize).fontSize(14).margin({ top: 5 })
        Slider({ value: this.fontSize, min: 12, max: 28, step: 1 })
          .width('70%').blockColor('#007DFF')
          .onChange((value: number) => { this.fontSize = Math.round(value) })

        Divider().margin({ top: 15 })
        this.sectionTitle('9. canLoop 测试')

        TextPicker({ range: this.singleFruits })
          .canLoop(false)
          .onChange((value: string | string[], index: number | number[]) => {
            this.resultText = 'noLoop onChange: ' + value
          })

        Text('canLoop: false（不可循环滚动）').fontSize(13).fontColor('#999999').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('10. defaultPickerItemHeight 测试')

        TextPicker({ range: this.singleFruits })
          .defaultPickerItemHeight(50)
          .onChange((value: string | string[], index: number | number[]) => {
            this.resultText = 'itemHeight onChange: ' + value
          })

        Text('defaultPickerItemHeight: 50vp').fontSize(13).fontColor('#999999').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('11. 单列与多列样式对比')

        Row() {
          Column() {
            Text('单列').fontSize(12).margin({ bottom: 5 })
            TextPicker({ range: this.singleFruits, selected: 0 })
          }.width('48%')
          Column() {
            Text('多列').fontSize(12).margin({ bottom: 5 })
            TextPicker({ range: this.multiFruits })
          }.width('48%')
        }.width('100%')

        Divider().margin({ top: 15 })
        this.sectionTitle('12. disappearTextStyle 测试')

        TextPicker({ range: this.singleFruits })
          .selectedTextStyle({ color: Color.Red, font: { size: 20, weight: FontWeight.Bold } })
          .textStyle({ color: '#666666', font: { size: 16 } })
          .disappearTextStyle({ color: '#CCCCCC', font: { size: 12, weight: FontWeight.Lighter } })

        Text('选中: 20px红色粗体 / 普通: 16px灰色 / 消失: 12px浅灰细体')
          .fontSize(13).fontColor('#999999').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('13. 超长文本选择器')

        TextPicker({
          range: ['这是一个超长的选项文本用于测试TextPicker的显示效果一', '这是另一个超长的选项文本用于测试二', '短文本三'],
          selected: 0,
        })
          .onChange((value: string | string[], index: number | number[]) => {
            this.resultText = '超长文本 onChange: ' + value
          })

        Text(this.resultText).fontSize(13).fontColor('#007DFF').margin({ top: 5 })

        Divider().margin({ top: 15 })
        this.sectionTitle('14. 事件回调综合测试')

        TextPicker({ range: this.singleFruits })
          .onChange((value: string | string[], index: number | number[]) => {
            this.resultText = 'onChange: ' + value + ', index: ' + index
          })
          .onScrollStop((value: string | string[], index: number | number[]) => {
            this.resultText = 'onScrollStop: ' + value + ', index: ' + index
          })
          .onEnterSelectedArea((value: string | string[], index: number | number[]) => {
            this.resultText = 'onEnterSelectedArea: ' + value + ', index: ' + index
          })

        Text(this.resultText).fontSize(13).fontColor('#007DFF').margin({ top: 5 })
        Text('滚动选择器查看onChange/onScrollStop/onEnterSelectedArea回调')
          .fontSize(13).fontColor('#999999').margin({ top: 3 })

        Divider().margin({ top: 15, bottom: 30 })
      }
      .width('100%').padding(20)
    }
    .width('100%').height('100%').backgroundColor('#F5F5F5')
  }
}
