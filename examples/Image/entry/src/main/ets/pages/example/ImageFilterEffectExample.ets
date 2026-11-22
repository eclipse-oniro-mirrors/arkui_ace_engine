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
 * Image 滤镜效果示例
 * 展示 grayscale、brightness、contrast、saturate 等滤镜效果
 */
@Entry
@Component
struct ImageFilterEffectExample {
  @State grayscaleValue: number = 0
  @State brightnessValue: number = 1.0
  @State contrastValue: number = 1.0
  @State saturateValue: number = 1.0
  @State invertValue: number = 0
  @State sepiaValue: number = 0
  @State hueRotateValue: number = 0
  @State presetIndex: number = 0

  private presets: { name: string, grayscale: number, brightness: number, contrast: number,
    saturate: number, invert: number, sepia: number, hueRotate: number }[] = [
    { name: '原图', grayscale: 0, brightness: 1.0, contrast: 1.0, saturate: 1.0, invert: 0, sepia: 0, hueRotate: 0 },
    { name: '黑白', grayscale: 1.0, brightness: 1.0, contrast: 1.0, saturate: 1.0, invert: 0, sepia: 0, hueRotate: 0 },
    { name: '复古', grayscale: 0, brightness: 0.9, contrast: 1.2, saturate: 0.8, invert: 0, sepia: 0.6, hueRotate: 0 },
    { name: '高对比', grayscale: 0, brightness: 1.0, contrast: 1.8, saturate: 1.3, invert: 0, sepia: 0, hueRotate: 0 },
    { name: '暖色调', grayscale: 0, brightness: 1.1, contrast: 1.0, saturate: 1.2, invert: 0, sepia: 0.2, hueRotate: 20 },
    { name: '冷色调', grayscale: 0, brightness: 1.0, contrast: 1.0, saturate: 0.9, invert: 0, sepia: 0, hueRotate: 200 },
    { name: '反色', grayscale: 0, brightness: 1.0, contrast: 1.0, saturate: 1.0, invert: 1.0, sepia: 0, hueRotate: 0 },
    { name: '梦幻', grayscale: 0, brightness: 1.2, contrast: 0.9, saturate: 1.4, invert: 0, sepia: 0, hueRotate: 30 }
  ]

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 滤镜效果示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildPresetSection()
        this.buildGrayscaleSection()
        this.buildBrightnessSection()
        this.buildContrastSection()
        this.buildSaturateSection()
        this.buildSepiaSection()
        this.buildHueRotateSection()
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
  buildPresetSection() {
    Column({ space: 12 }) {
      Text('预设滤镜')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Image($r('app.media.startIcon'))
        .width(180)
        .height(180)
        .objectFit(ImageFit.Cover)
        .borderRadius(12)
        .grayscale(this.presets[this.presetIndex].grayscale)
        .brightness(this.presets[this.presetIndex].brightness)
        .contrast(this.presets[this.presetIndex].contrast)
        .saturate(this.presets[this.presetIndex].saturate)
        .invert(this.presets[this.presetIndex].invert)
        .sepia(this.presets[this.presetIndex].sepia)
        .hueRotate(this.presets[this.presetIndex].hueRotate)

      Text(`预设: ${this.presets[this.presetIndex].name}`)
        .fontSize(14)
        .fontColor(0x666666)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.presets, (item: { name: string }, index: number) => {
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
                this.grayscaleValue = this.presets[index].grayscale
                this.brightnessValue = this.presets[index].brightness
                this.contrastValue = this.presets[index].contrast
                this.saturateValue = this.presets[index].saturate
                this.invertValue = this.presets[index].invert
                this.sepiaValue = this.presets[index].sepia
                this.hueRotateValue = this.presets[index].hueRotate
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
  buildGrayscaleSection() {
    Column({ space: 10 }) {
      Text(`灰度: ${this.grayscaleValue.toFixed(2)}`)
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Image($r('app.media.background'))
        .width(150)
        .height(100)
        .objectFit(ImageFit.Cover)
        .borderRadius(8)
        .grayscale(this.grayscaleValue)

      Slider({
        value: this.grayscaleValue,
        min: 0,
        max: 1,
        step: 0.05
      })
        .width('100%')
        .onChange((value: number) => {
          this.grayscaleValue = value
        })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildBrightnessSection() {
    Column({ space: 10 }) {
      Text(`亮度: ${this.brightnessValue.toFixed(2)}`)
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Image($r('app.media.background'))
        .width(150)
        .height(100)
        .objectFit(ImageFit.Cover)
        .borderRadius(8)
        .brightness(this.brightnessValue)

      Slider({
        value: this.brightnessValue,
        min: 0,
        max: 2,
        step: 0.1
      })
        .width('100%')
        .onChange((value: number) => {
          this.brightnessValue = value
        })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildContrastSection() {
    Column({ space: 10 }) {
      Text(`对比度: ${this.contrastValue.toFixed(2)}`)
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Image($r('app.media.background'))
        .width(150)
        .height(100)
        .objectFit(ImageFit.Cover)
        .borderRadius(8)
        .contrast(this.contrastValue)

      Slider({
        value: this.contrastValue,
        min: 0,
        max: 2,
        step: 0.1
      })
        .width('100%')
        .onChange((value: number) => {
          this.contrastValue = value
        })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildSaturateSection() {
    Column({ space: 10 }) {
      Text(`饱和度: ${this.saturateValue.toFixed(2)}`)
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Image($r('app.media.background'))
        .width(150)
        .height(100)
        .objectFit(ImageFit.Cover)
        .borderRadius(8)
        .saturate(this.saturateValue)

      Slider({
        value: this.saturateValue,
        min: 0,
        max: 2,
        step: 0.1
      })
        .width('100%')
        .onChange((value: number) => {
          this.saturateValue = value
        })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildSepiaSection() {
    Column({ space: 10 }) {
      Text(`怀旧: ${this.sepiaValue.toFixed(2)}`)
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Image($r('app.media.background'))
        .width(150)
        .height(100)
        .objectFit(ImageFit.Cover)
        .borderRadius(8)
        .sepia(this.sepiaValue)

      Slider({
        value: this.sepiaValue,
        min: 0,
        max: 1,
        step: 0.05
      })
        .width('100%')
        .onChange((value: number) => {
          this.sepiaValue = value
        })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildHueRotateSection() {
    Column({ space: 10 }) {
      Text(`色相旋转: ${this.hueRotateValue}°`)
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Image($r('app.media.background'))
        .width(150)
        .height(100)
        .objectFit(ImageFit.Cover)
        .borderRadius(8)
        .hueRotate(this.hueRotateValue)

      Slider({
        value: this.hueRotateValue,
        min: 0,
        max: 360,
        step: 10
      })
        .width('100%')
        .onChange((value: number) => {
          this.hueRotateValue = value
        })
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
      Text('滤镜对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 8 }) {
        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
          Text('原图')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .grayscale(1.0)
          Text('黑白')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .sepia(0.8)
          Text('复古')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .saturate(2.0)
          Text('高饱和')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .invert(1.0)
          Text('反色')
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
