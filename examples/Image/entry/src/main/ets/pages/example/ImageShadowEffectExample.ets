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
 * Image 阴影效果示例
 * 展示 shadow 属性的各种阴影效果
 */
@Entry
@Component
struct ImageShadowEffectExample {
  @State shadowRadius: number = 0
  @State shadowOffsetX: number = 0
  @State shadowOffsetY: number = 8
  @State shadowColor: string = '#40000000'
  @State presetIndex: number = 0

  private presets: { name: string, radius: number, offsetX: number, offsetY: number, color: string }[] = [
    { name: '无阴影', radius: 0, offsetX: 0, offsetY: 0, color: '#00000000' },
    { name: '轻微', radius: 8, offsetX: 0, offsetY: 4, color: '#30000000' },
    { name: '中等', radius: 16, offsetX: 0, offsetY: 8, color: '#40000000' },
    { name: '强烈', radius: 24, offsetX: 0, offsetY: 12, color: '#50000000' },
    { name: '悬浮', radius: 32, offsetX: 0, offsetY: 16, color: '#60000000' }
  ]

  private shadowColors: { name: string, color: string }[] = [
    { name: '黑色', color: '#40000000' },
    { name: '红色', color: '#40FF0000' },
    { name: '蓝色', color: '#400000FF' },
    { name: '绿色', color: '#4000FF00' },
    { name: '金色', color: '#40FFD700' },
    { name: '紫色', color: '#40800080' }
  ]

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 阴影效果示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildBasicShadowSection()
        this.buildOffsetSection()
        this.buildColorSection()
        this.buildPresetSection()
        this.buildComparisonSection()
      }
      .width('100%')
      .padding({ left: 16, right: 16, bottom: 30 })
    }
    .width('100%')
    .height('100%')
    .backgroundColor(0xF5F5F5)
  }

  @Builder
  buildBasicShadowSection() {
    Column({ space: 12 }) {
      Text('基础阴影')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Image($r('app.media.startIcon'))
        .width(180)
        .height(180)
        .objectFit(ImageFit.Cover)
        .borderRadius(12)
        .shadow({
          radius: this.shadowRadius,
          color: this.shadowColor,
          offsetX: this.shadowOffsetX,
          offsetY: this.shadowOffsetY
        })

      Text(`阴影半径: ${this.shadowRadius}`)
        .fontSize(14)
        .fontColor(0x666666)

      Slider({
        value: this.shadowRadius,
        min: 0,
        max: 40,
        step: 1
      })
        .width('100%')
        .onChange((value: number) => {
          this.shadowRadius = value
        })

      Row({ space: 10 }) {
        Button('清除')
          .fontSize(12)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.shadowRadius = 0
            })
          })
        Button('最大')
          .fontSize(12)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.shadowRadius = 40
            })
          })
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildOffsetSection() {
    Column({ space: 12 }) {
      Text('阴影偏移')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Image($r('app.media.background'))
        .width(160)
        .height(120)
        .objectFit(ImageFit.Cover)
        .borderRadius(8)
        .shadow({
          radius: 16,
          color: '#40000000',
          offsetX: this.shadowOffsetX,
          offsetY: this.shadowOffsetY
        })

      Text(`X 偏移: ${this.shadowOffsetX}`)
        .fontSize(14)
        .fontColor(0x666666)

      Slider({
        value: this.shadowOffsetX,
        min: -30,
        max: 30,
        step: 1
      })
        .width('100%')
        .onChange((value: number) => {
          this.shadowOffsetX = value
        })

      Text(`Y 偏移: ${this.shadowOffsetY}`)
        .fontSize(14)
        .fontColor(0x666666)

      Slider({
        value: this.shadowOffsetY,
        min: -30,
        max: 30,
        step: 1
      })
        .width('100%')
        .onChange((value: number) => {
          this.shadowOffsetY = value
        })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildColorSection() {
    Column({ space: 12 }) {
      Text('阴影颜色')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Image($r('app.media.foreground'))
        .width(140)
        .height(140)
        .objectFit(ImageFit.Cover)
        .borderRadius(12)
        .shadow({
          radius: 20,
          color: this.shadowColor,
          offsetX: 0,
          offsetY: 8
        })

      Text(`当前颜色: ${this.shadowColor}`)
        .fontSize(12)
        .fontColor(0x666666)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.shadowColors, (item: { name: string, color: string }) => {
          Column({ space: 4 }) {
            Column()
              .width(40)
              .height(40)
              .borderRadius(20)
              .backgroundColor(item.color)
              .border({ width: this.shadowColor === item.color ? 3 : 0, color: '#333333' })
              .onClick(() => {
                this.shadowColor = item.color
              })
            Text(item.name)
              .fontSize(10)
              .fontColor(0x666666)
          }
          .margin(6)
        })
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildPresetSection() {
    Column({ space: 12 }) {
      Text('预设阴影')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Image($r('app.media.icon'))
        .width(120)
        .height(120)
        .objectFit(ImageFit.Cover)
        .borderRadius(60)
        .shadow({
          radius: this.presets[this.presetIndex].radius,
          color: this.presets[this.presetIndex].color,
          offsetX: this.presets[this.presetIndex].offsetX,
          offsetY: this.presets[this.presetIndex].offsetY
        })

      Text(`预设: ${this.presets[this.presetIndex].name}`)
        .fontSize(14)
        .fontColor(0x666666)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.presets,
          (item: { name: string, radius: number, offsetX: number, offsetY: number, color: string },
            index: number) => {
          Text(item.name)
            .fontSize(12)
            .padding({ left: 12, right: 12, top: 6, bottom: 6 })
            .margin(4)
            .borderRadius(16)
            .backgroundColor(this.presetIndex === index ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.presetIndex === index ? Color.White : 0x333333)
            .onClick(() => {
              animateTo({ duration: 300 }, () => {
                this.presetIndex = index
              })
            })
        })
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildComparisonSection() {
    Column({ space: 12 }) {
      Text('阴影对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('同一图片不同阴影效果对比')
        .fontSize(12)
        .fontColor(0x999999)

      Row({ space: 12 }) {
        Column({ space: 4 }) {
          Image($r('app.media.star'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
          Text('无')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.star'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .shadow({ radius: 8, color: '#30000000', offsetX: 0, offsetY: 4 })
          Text('轻微')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.star'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .shadow({ radius: 16, color: '#40000000', offsetX: 0, offsetY: 8 })
          Text('中等')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.star'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .shadow({ radius: 24, color: '#50000000', offsetX: 0, offsetY: 12 })
          Text('强烈')
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
}
