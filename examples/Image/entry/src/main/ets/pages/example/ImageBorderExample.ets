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
 * Image 边框示例
 * 展示 border 属性的各种边框效果
 */
@Entry
@Component
struct ImageBorderExample {
  @State borderWidth: number = 0
  @State borderColor: string = '#007DFF'
  @State borderStyleIndex: number = 0
  @State borderRadius: number = 0

  private borderStyles: { name: string, value: BorderStyle }[] = [
    { name: '实线', value: BorderStyle.Solid },
    { name: '虚线', value: BorderStyle.Dashed },
    { name: '点线', value: BorderStyle.Dotted }
  ]

  private borderColors: { name: string, color: string }[] = [
    { name: '蓝色', color: '#007DFF' },
    { name: '红色', color: '#FF0000' },
    { name: '绿色', color: '#00FF00' },
    { name: '金色', color: '#FFD700' },
    { name: '黑色', color: '#000000' }
  ]

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 边框示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildPreviewSection()
        this.buildWidthSection()
        this.buildColorSection()
        this.buildStyleSection()
        this.buildRadiusSection()
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

      Image($r('app.media.background'))
        .width(150)
        .height(150)
        .objectFit(ImageFit.Cover)
        .border({
          width: this.borderWidth,
          color: this.borderColor,
          style: this.borderStyles[this.borderStyleIndex].value
        })
        .borderRadius(this.borderRadius)

      Text(`边框: ${this.borderWidth}px ${this.borderStyles[this.borderStyleIndex].name}`)
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
  buildWidthSection() {
    Column({ space: 12 }) {
      Text(`边框宽度: ${this.borderWidth}px`)
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Slider({
        value: this.borderWidth,
        min: 0,
        max: 12,
        step: 1
      })
        .width('100%')
        .onChange((value: number) => {
          this.borderWidth = value
        })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildColorSection() {
    Column({ space: 12 }) {
      Text('边框颜色')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Row({ space: 10 }) {
        ForEach(this.borderColors, (item: { name: string, color: string }) => {
          Column({ space: 4 }) {
            Column()
              .width(40)
              .height(40)
              .borderRadius(20)
              .backgroundColor(item.color)
              .border({ width: this.borderColor === item.color ? 3 : 0, color: '#333333' })
              .onClick(() => {
                this.borderColor = item.color
              })
            Text(item.name)
              .fontSize(10)
              .fontColor(0x666666)
          }
        })
      }
      .justifyContent(FlexAlign.SpaceEvenly)
      .width('100%')
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildStyleSection() {
    Column({ space: 12 }) {
      Text('边框样式')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Row({ space: 10 }) {
        ForEach(this.borderStyles, (item: { name: string, value: BorderStyle }, index: number) => {
          Text(item.name)
            .fontSize(13)
            .padding({ left: 16, right: 16, top: 10, bottom: 10 })
            .borderRadius(20)
            .backgroundColor(this.borderStyleIndex === index ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.borderStyleIndex === index ? Color.White : 0x333333)
            .onClick(() => {
              this.borderStyleIndex = index
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
  buildRadiusSection() {
    Column({ space: 12 }) {
      Text(`圆角: ${this.borderRadius}`)
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Slider({
        value: this.borderRadius,
        min: 0,
        max: 75,
        step: 1
      })
        .width('100%')
        .onChange((value: number) => {
          this.borderRadius = value
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
      Text('边框对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 12 }) {
        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
          Text('无边框')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .border({ width: 2, color: '#007DFF', style: BorderStyle.Solid })
          Text('实线')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .border({ width: 2, color: '#FF0000', style: BorderStyle.Dashed })
          Text('虚线')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .border({ width: 2, color: '#00FF00', style: BorderStyle.Dotted })
          Text('点线')
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

      Text('1. 边框宽度范围 0-12px')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 支持实线、虚线、点线样式')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 可自定义边框颜色')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 边框可配合圆角使用')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 常用于卡片、按钮等组件')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
