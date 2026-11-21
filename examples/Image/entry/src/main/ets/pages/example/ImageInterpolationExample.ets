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

/**
 * Image 插值质量示例
 * 展示 interpolation 属性对图片质量的影响
 */
@Entry
@Component
struct ImageInterpolationExample {
  @State interpolationIndex: number = 1
  @State scaleValue: number = 2.0
  @State loadLog: string = '等待加载...'

  private logs: string[] = []
  private interpolationOptions: { name: string, value: ImageInterpolation }[] = [
    { name: 'None', value: ImageInterpolation.None },
    { name: 'Low', value: ImageInterpolation.Low },
    { name: 'Medium', value: ImageInterpolation.Medium },
    { name: 'High', value: ImageInterpolation.High }
  ]

  addLog(msg: string) {
    const time = new Date().toLocaleTimeString()
    this.logs.unshift(`[${time}] ${msg}`)
    if (this.logs.length > 5) {
      this.logs.pop()
    }
    this.loadLog = this.logs.join('\n')
  }

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 插值质量示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildPreviewSection()
        this.buildLogSection()
        this.buildInterpolationSection()
        this.buildScaleSection()
        this.buildComparisonSection()
        this.buildTipsSection()
      }
      .width('100%')
      .padding({ left: 16, right: 16, bottom: 30 })
    }
    .width('100%')
    .height('100%')
    .backgroundColor(0xF5F5F5)
  }

  @Builder
  buildPreviewSection() {
    Column({ space: 12 }) {
      Text('预览')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Image($r('app.media.startIcon'))
        .width(100)
        .height(100)
        .interpolation(this.interpolationOptions[this.interpolationIndex].value)
        .scale({ x: this.scaleValue, y: this.scaleValue })
        .objectFit(ImageFit.Cover)
        .borderRadius(12)
        .border({ width: 1, color: 0xCCCCCC })

      Text(`插值: ${this.interpolationOptions[this.interpolationIndex].name}`)
        .fontSize(14)
        .fontColor(0x666666)
      Text(`缩放: ${this.scaleValue.toFixed(1)}x`)
        .fontSize(14)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildLogSection() {
    Column({ space: 10 }) {
      Text('加载日志')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Scroll() {
        Text(this.loadLog)
          .fontSize(12)
          .fontColor(0x333333)
          .fontFamily('monospace')
      }
      .width('100%')
      .height(80)
      .backgroundColor(0xF8F8F8)
      .borderRadius(8)
      .padding(10)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildInterpolationSection() {
    Column({ space: 12 }) {
      Text('插值质量')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.interpolationOptions, (item: { name: string, value: ImageInterpolation }, index: number) => {
          Text(item.name)
            .fontSize(13)
            .padding({ left: 16, right: 16, top: 10, bottom: 10 })
            .margin(4)
            .borderRadius(20)
            .backgroundColor(this.interpolationIndex === index ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.interpolationIndex === index ? Color.White : 0x333333)
            .onClick(() => {
              this.interpolationIndex = index
              this.addLog(`切换插值: ${item.name}`)
            })
        })
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildScaleSection() {
    Column({ space: 12 }) {
      Text('缩放级别')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text(`缩放: ${this.scaleValue.toFixed(1)}x`)
        .fontSize(14)
      Slider({
        value: this.scaleValue,
        min: 1.0,
        max: 4.0,
        step: 0.5
      })
        .width('100%')
        .onChange((value: number) => {
          this.scaleValue = value
        })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildComparisonSection() {
    Column({ space: 12 }) {
      Text('插值对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('放大 3x 后的效果对比')
        .fontSize(12)
        .fontColor(0x999999)

      Row({ space: 8 }) {
        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(40)
            .height(40)
            .interpolation(ImageInterpolation.None)
            .scale({ x: 3, y: 3 })
            .objectFit(ImageFit.Cover)
            .borderRadius(4)
          Text('None')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(40)
            .height(40)
            .interpolation(ImageInterpolation.Low)
            .scale({ x: 3, y: 3 })
            .objectFit(ImageFit.Cover)
            .borderRadius(4)
          Text('Low')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(40)
            .height(40)
            .interpolation(ImageInterpolation.Medium)
            .scale({ x: 3, y: 3 })
            .objectFit(ImageFit.Cover)
            .borderRadius(4)
          Text('Medium')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(40)
            .height(40)
            .interpolation(ImageInterpolation.High)
            .scale({ x: 3, y: 3 })
            .objectFit(ImageFit.Cover)
            .borderRadius(4)
          Text('High')
            .fontSize(10)
            .fontColor(0x666666)
        }
      }
      .justifyContent(FlexAlign.Center)
      .width('100%')
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildTipsSection() {
    Column({ space: 8 }) {
      Text('使用提示')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('1. None: 最近邻插值，速度最快，质量最低')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. Low: 双线性插值，速度较快')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. Medium: 质量与速度平衡')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. High: 高质量插值，速度较慢')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 列表滚动时建议使用 Low/None')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
