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
 * Image 圆角示例
 * 展示 borderRadius 属性的各种圆角效果
 */
@Entry
@Component
struct ImageBorderRadiusExample {
  @State uniformRadius: number = 0
  @State topLeft: number = 0
  @State topRight: number = 0
  @State bottomLeft: number = 0
  @State bottomRight: number = 0
  @State useUniform: boolean = true

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 圆角示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildPreviewSection()
        this.buildUniformSection()
        this.buildIndividualSection()
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
        .width(150)
        .height(150)
        .objectFit(ImageFit.Cover)
        .borderRadius(this.useUniform ? this.uniformRadius : undefined)
        .borderRadius({
          topLeft: this.useUniform ? this.uniformRadius : this.topLeft,
          topRight: this.useUniform ? this.uniformRadius : this.topRight,
          bottomLeft: this.useUniform ? this.uniformRadius : this.bottomLeft,
          bottomRight: this.useUniform ? this.uniformRadius : this.bottomRight
        })
        .border({ width: 1, color: 0xCCCCCC })

      if (this.useUniform) {
        Text(`圆角: ${this.uniformRadius}`)
          .fontSize(14)
          .fontColor(0x666666)
      } else {
        Text(`TL:${this.topLeft} TR:${this.topRight} BL:${this.bottomLeft} BR:${this.bottomRight}`)
          .fontSize(12)
          .fontColor(0x666666)
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildUniformSection() {
    Column({ space: 12 }) {
      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.useUniform })
          .onChange((isOn: boolean) => {
            this.useUniform = isOn
          })
        Text('统一圆角')
          .fontSize(16)
          .fontWeight(FontWeight.Medium)
      }

      if (this.useUniform) {
        Text(`圆角: ${this.uniformRadius}`)
          .fontSize(14)
        Slider({
          value: this.uniformRadius,
          min: 0,
          max: 75,
          step: 1
        })
          .width('100%')
          .onChange((value: number) => {
            this.uniformRadius = value
          })
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildIndividualSection() {
    Column({ space: 12 }) {
      Text('独立圆角')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      if (!this.useUniform) {
        Text(`左上: ${this.topLeft}`)
          .fontSize(14)
        Slider({
          value: this.topLeft,
          min: 0,
          max: 75,
          step: 1
        })
          .width('100%')
          .onChange((value: number) => {
            this.topLeft = value
          })

        Text(`右上: ${this.topRight}`)
          .fontSize(14)
        Slider({
          value: this.topRight,
          min: 0,
          max: 75,
          step: 1
        })
          .width('100%')
          .onChange((value: number) => {
            this.topRight = value
          })

        Text(`左下: ${this.bottomLeft}`)
          .fontSize(14)
        Slider({
          value: this.bottomLeft,
          min: 0,
          max: 75,
          step: 1
        })
          .width('100%')
          .onChange((value: number) => {
            this.bottomLeft = value
          })

        Text(`右下: ${this.bottomRight}`)
          .fontSize(14)
        Slider({
          value: this.bottomRight,
          min: 0,
          max: 75,
          step: 1
        })
          .width('100%')
          .onChange((value: number) => {
            this.bottomRight = value
          })
      } else {
        Text('请先关闭"统一圆角"')
          .fontSize(12)
          .fontColor(0x999999)
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildComparisonSection() {
    Column({ space: 12 }) {
      Text('圆角对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 8 }) {
        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(0)
          Text('0')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(10)
          Text('10')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(20)
          Text('20')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(30)
          Text('30')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(30)
          Text('圆形')
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

      Text('1. 统一圆角: 四个角相同')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 独立圆角: 可分别设置每个角')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 圆角值 = 宽度/2 时为圆形')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 可配合 clip 裁剪内容')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 常用于卡片、头像等场景')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
