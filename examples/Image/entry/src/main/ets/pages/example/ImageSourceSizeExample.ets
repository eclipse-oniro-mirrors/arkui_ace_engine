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
 * Image sourceSize 解码尺寸示例
 * 展示 sourceSize 属性对图片解码的影响
 */
@Entry
@Component
struct ImageSourceSizeExample {
  @State sourceWidth: number = 100
  @State sourceHeight: number = 100
  @State displayWidth: number = 100
  @State displayHeight: number = 100
  @State loadLog: string = '等待加载...'

  private logs: string[] = []

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
        Text('Image sourceSize 示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildPreviewSection()
        this.buildLogSection()
        this.buildSourceSizeSection()
        this.buildDisplaySizeSection()
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
        .width(this.displayWidth)
        .height(this.displayHeight)
        .sourceSize({ width: this.sourceWidth, height: this.sourceHeight })
        .objectFit(ImageFit.Cover)
        .borderRadius(12)
        .border({ width: 1, color: 0xCCCCCC })
        .onComplete(() => {
          this.addLog(`加载完成: source=${this.sourceWidth}x${this.sourceHeight}`)
        })

      Text(`解码尺寸: ${this.sourceWidth}x${this.sourceHeight}`)
        .fontSize(12)
        .fontColor(0x666666)
      Text(`显示尺寸: ${this.displayWidth}x${this.displayHeight}`)
        .fontSize(12)
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
      .height(100)
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
  buildSourceSizeSection() {
    Column({ space: 12 }) {
      Text('解码尺寸 (sourceSize)')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text(`宽度: ${this.sourceWidth}`)
        .fontSize(14)
      Slider({
        value: this.sourceWidth,
        min: 32,
        max: 400,
        step: 8
      })
        .width('100%')
        .onChange((value: number) => {
          this.sourceWidth = value
        })

      Text(`高度: ${this.sourceHeight}`)
        .fontSize(14)
      Slider({
        value: this.sourceHeight,
        min: 32,
        max: 400,
        step: 8
      })
        .width('100%')
        .onChange((value: number) => {
          this.sourceHeight = value
        })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildDisplaySizeSection() {
    Column({ space: 12 }) {
      Text('显示尺寸')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text(`宽度: ${this.displayWidth}`)
        .fontSize(14)
      Slider({
        value: this.displayWidth,
        min: 50,
        max: 300,
        step: 10
      })
        .width('100%')
        .onChange((value: number) => {
          this.displayWidth = value
        })

      Text(`高度: ${this.displayHeight}`)
        .fontSize(14)
      Slider({
        value: this.displayHeight,
        min: 50,
        max: 300,
        step: 10
      })
        .width('100%')
        .onChange((value: number) => {
          this.displayHeight = value
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
      Text('解码尺寸对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('同一显示尺寸，不同解码尺寸')
        .fontSize(12)
        .fontColor(0x999999)

      Row({ space: 8 }) {
        Column({ space: 4 }) {
          Image($r('app.media.background'))
            .width(80)
            .height(80)
            .sourceSize({ width: 40, height: 40 })
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
          Text('40x40')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.background'))
            .width(80)
            .height(80)
            .sourceSize({ width: 80, height: 80 })
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
          Text('80x80')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.background'))
            .width(80)
            .height(80)
            .sourceSize({ width: 160, height: 160 })
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
          Text('160x160')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.background'))
            .width(80)
            .height(80)
            .sourceSize({ width: 320, height: 320 })
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
          Text('320x320')
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

      Text('1. sourceSize 应接近实际显示尺寸')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 过大会浪费内存，过小会模糊')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 列表场景建议使用固定尺寸')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 可显著降低内存占用')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 建议 sourceSize = 显示尺寸 x 屏幕密度')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
