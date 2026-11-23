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
 * Image 图文叠加示例
 * 展示图片与文字叠加的效果
 */
@Entry
@Component
struct ImageTextOverlayExample {
  @State overlayPosition: number = 0
  @State overlayText: string = '图片标题'
  @State overlayOpacity: number = 50

  private positions: string[] = ['顶部', '底部', '居中']

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 图文叠加示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildPreviewSection()
        this.buildPositionSection()
        this.buildTextSection()
        this.buildOpacitySection()
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

      Stack() {
        Image($r('app.media.background'))
          .width(220)
          .height(160)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)

        if (this.overlayPosition === 0) {
          Column()
            .width(220)
            .height(60)
            .linearGradient({
              angle: 180,
              colors: [['rgba(0,0,0,0)', 0], [`rgba(0,0,0,${this.overlayOpacity / 100})`, 1.0]]
            })
            .borderRadius({ topLeft: 12, topRight: 12 })
            .alignContent(Alignment.Top)
            .padding({ top: 12 })
        } else if (this.overlayPosition === 1) {
          Column()
            .width(220)
            .height(60)
            .linearGradient({
              angle: 0,
              colors: [['rgba(0,0,0,0)', 0], [`rgba(0,0,0,${this.overlayOpacity / 100})`, 1.0]]
            })
            .borderRadius({ bottomLeft: 12, bottomRight: 12 })
            .alignContent(Alignment.Bottom)
            .padding({ bottom: 12 })
        } else {
          Column()
            .width(220)
            .height(160)
            .borderRadius(12)
            .backgroundColor(`rgba(0,0,0,${this.overlayOpacity / 200})`)
        }

        Text(this.overlayText)
          .fontSize(18)
          .fontWeight(FontWeight.Bold)
          .fontColor(Color.White)
          .shadow({ radius: 4, color: '#80000000', offsetX: 0, offsetY: 2 })
      }
      .width(220)
      .height(160)
      .alignContent(this.overlayPosition === 0 ? Alignment.Top :
        this.overlayPosition === 1 ? Alignment.Bottom : Alignment.Center)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildPositionSection() {
    Column({ space: 12 }) {
      Text('文字位置')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 10 }) {
        ForEach(this.positions, (name: string, index: number) => {
          Text(name)
            .fontSize(13)
            .padding({ left: 16, right: 16, top: 10, bottom: 10 })
            .borderRadius(20)
            .backgroundColor(this.overlayPosition === index ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.overlayPosition === index ? Color.White : 0x333333)
            .onClick(() => {
              this.overlayPosition = index
            })
        })
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
  buildTextSection() {
    Column({ space: 12 }) {
      Text('叠加文字')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      TextInput({ text: this.overlayText, placeholder: '输入叠加文字' })
        .width('100%')
        .onChange((value: string) => {
          this.overlayText = value
        })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildOpacitySection() {
    Column({ space: 12 }) {
      Text(`遮罩不透明度: ${this.overlayOpacity}%`)
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Slider({
        value: this.overlayOpacity,
        min: 20,
        max: 100,
        step: 5
      })
        .width('100%')
        .onChange((value: number) => {
          this.overlayOpacity = value
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
      Text('位置对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 12 }) {
        Column({ space: 4 }) {
          Stack() {
            Image($r('app.media.foreground'))
              .width(80)
              .height(80)
              .objectFit(ImageFit.Cover)
              .borderRadius(8)
            Column()
              .width(80)
              .height(30)
              .linearGradient({ angle: 180, colors: [['rgba(0,0,0,0)', 0], ['rgba(0,0,0,0.5)', 1.0]] })
              .borderRadius({ topLeft: 8, topRight: 8 })
              .alignContent(Alignment.Top)
              .padding({ top: 6 })
            Text('顶部')
              .fontSize(10)
              .fontColor(Color.White)
          }
          .width(80)
          .height(80)
          .alignContent(Alignment.Top)
          Text('顶部')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Stack() {
            Image($r('app.media.foreground'))
              .width(80)
              .height(80)
              .objectFit(ImageFit.Cover)
              .borderRadius(8)
            Column()
              .width(80)
              .height(30)
              .linearGradient({ angle: 0, colors: [['rgba(0,0,0,0)', 0], ['rgba(0,0,0,0.5)', 1.0]] })
              .borderRadius({ bottomLeft: 8, bottomRight: 8 })
              .alignContent(Alignment.Bottom)
              .padding({ bottom: 6 })
            Text('底部')
              .fontSize(10)
              .fontColor(Color.White)
          }
          .width(80)
          .height(80)
          .alignContent(Alignment.Bottom)
          Text('底部')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Stack() {
            Image($r('app.media.foreground'))
              .width(80)
              .height(80)
              .objectFit(ImageFit.Cover)
              .borderRadius(8)
            Column()
              .width(80)
              .height(80)
              .borderRadius(8)
              .backgroundColor('rgba(0,0,0,0.3)')
            Text('居中')
              .fontSize(10)
              .fontColor(Color.White)
          }
          .width(80)
          .height(80)
          .alignContent(Alignment.Center)
          Text('居中')
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

      Text('1. 使用 Stack 实现图文叠加')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 渐变遮罩让文字更清晰')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 可调整遮罩不透明度')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 文字添加阴影增强可读性')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 常用于卡片、海报等场景')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
