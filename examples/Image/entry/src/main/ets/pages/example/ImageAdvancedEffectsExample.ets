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
 * Image 高级效果示例
 * 展示模糊、阴影、遮罩、混合模式等高级视觉效果
 */
@Entry
@Component
struct ImageAdvancedEffectsExample {
  @State blurRadius: number = 0
  @State shadowRadius: number = 0
  @State shadowOffsetX: number = 0
  @State shadowOffsetY: number = 8
  @State shadowColor: string = '#40000000'
  @State opacityValue: number = 1.0
  @State maskEnabled: boolean = false
  @State maskOpacity: number = 50
  @State blendModeIndex: number = 0
  @State overlayEnabled: boolean = false
  @State overlayColor: string = '#33007DFF'
  @State grayscaleValue: number = 0
  @State brightnessValue: number = 1.0
  @State contrastValue: number = 1.0
  @State saturateValue: number = 1.0
  @State hueRotateValue: number = 0
  @State invertValue: number = 0
  @State sepiaValue: number = 0

  blendModes: { name: string, value: BlendMode }[] = [
    { name: 'SrcOver', value: BlendMode.SrcOver },
    { name: 'Screen', value: BlendMode.Screen },
    { name: 'Multiply', value: BlendMode.Multiply },
    { name: 'Overlay', value: BlendMode.Overlay },
    { name: 'Lighten', value: BlendMode.Lighten },
    { name: 'Darken', value: BlendMode.Darken },
    { name: 'Difference', value: BlendMode.Difference },
    { name: 'Exclusion', value: BlendMode.Exclusion }
  ]

  build() {
    Scroll() {
      Column({ space: 20 }) {
        Text('Image 高级效果示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildPreviewSection()
        this.buildBlurSection()
        this.buildShadowSection()
        this.buildOpacitySection()
        this.buildMaskSection()
        this.buildBlendModeSection()
        this.buildOverlaySection()
        this.buildFilterSection()
        this.buildResetButton()
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
    Column({ space: 10 }) {
      Text('预览效果')
        .fontSize(18)
        .fontWeight(FontWeight.Medium)

      Stack() {
        Image($r('app.media.startIcon'))
          .width(240)
          .height(240)
          .objectFit(ImageFit.Cover)
          .borderRadius(16)
          .blur(this.blurRadius)
          .shadow({
            radius: this.shadowRadius,
            color: this.shadowColor,
            offsetX: this.shadowOffsetX,
            offsetY: this.shadowOffsetY
          })
          .opacity(this.opacityValue)
          .grayscale(this.grayscaleValue)
          .brightness(this.brightnessValue)
          .contrast(this.contrastValue)
          .saturate(this.saturateValue)
          .hueRotate(this.hueRotateValue)
          .invert(this.invertValue)
          .sepia(this.sepiaValue)

        if (this.maskEnabled) {
          Column()
            .width(240)
            .height(240)
            .borderRadius(16)
            .backgroundColor(`rgba(0, 0, 0, ${this.maskOpacity / 100})`)
        }

        if (this.overlayEnabled) {
          Column()
            .width(240)
            .height(240)
            .borderRadius(16)
            .backgroundColor(this.overlayColor)
            .blendMode(this.blendModes[this.blendModeIndex].value)
        }
      }
      .width(240)
      .height(240)
      .alignContent(Alignment.Center)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildBlurSection() {
    Column({ space: 10 }) {
      Text(`模糊半径: ${this.blurRadius}`)
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Slider({
        value: this.blurRadius,
        min: 0,
        max: 30,
        step: 1
      })
        .width('100%')
        .onChange((value: number) => {
          this.blurRadius = value
        })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildShadowSection() {
    Column({ space: 12 }) {
      Text('阴影效果')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Text(`阴影半径: ${this.shadowRadius}`)
        .fontSize(14)
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

      Text(`X 偏移: ${this.shadowOffsetX}`)
        .fontSize(14)
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
  }

  @Builder
  buildOpacitySection() {
    Column({ space: 10 }) {
      Text(`不透明度: ${this.opacityValue.toFixed(2)}`)
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Slider({
        value: this.opacityValue,
        min: 0,
        max: 1,
        step: 0.05
      })
        .width('100%')
        .onChange((value: number) => {
          this.opacityValue = value
        })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildMaskSection() {
    Column({ space: 12 }) {
      Row({ space: 20 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.maskEnabled })
          .onChange((isOn: boolean) => {
            this.maskEnabled = isOn
          })
        Text('启用遮罩')
          .fontSize(16)
          .fontWeight(FontWeight.Medium)
      }

      if (this.maskEnabled) {
        Text(`遮罩不透明度: ${this.maskOpacity}%`)
          .fontSize(14)
        Slider({
          value: this.maskOpacity,
          min: 0,
          max: 100,
          step: 5
        })
          .width('100%')
          .onChange((value: number) => {
            this.maskOpacity = value
          })
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildBlendModeSection() {
    Column({ space: 12 }) {
      Row({ space: 20 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.overlayEnabled })
          .onChange((isOn: boolean) => {
            this.overlayEnabled = isOn
          })
        Text('启用混合叠加')
          .fontSize(16)
          .fontWeight(FontWeight.Medium)
      }

      if (this.overlayEnabled) {
        Text('混合模式')
          .fontSize(14)

        Flex({ wrap: FlexWrap.Wrap }) {
          ForEach(this.blendModes, (item: { name: string, value: BlendMode }, index: number) => {
            Text(item.name)
              .fontSize(12)
              .padding({ left: 12, right: 12, top: 6, bottom: 6 })
              .margin(4)
              .borderRadius(16)
              .backgroundColor(this.blendModeIndex === index ? 0x007DFF : 0xE8E8E8)
              .fontColor(this.blendModeIndex === index ? Color.White : 0x333333)
              .onClick(() => {
                this.blendModeIndex = index
              })
          })
        }

        Text('叠加颜色')
          .fontSize(14)
        Row({ space: 10 }) {
          ForEach(['#33FF0000', '#3300FF00', '#330000FF', '#33FFFF00', '#33FF00FF'], (color: string, index: number) => {
            Column()
              .width(40)
              .height(40)
              .borderRadius(20)
              .backgroundColor(color)
              .border({ width: this.overlayColor === color ? 3 : 0, color: Color.Black })
              .onClick(() => {
                this.overlayColor = color
              })
          })
        }
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildOverlaySection() {
    Column({ space: 10 }) {
      Text('颜色叠加预设')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Row({ space: 10 }) {
        Button('暖色调')
          .fontSize(12)
          .onClick(() => {
            this.overlayEnabled = true
            this.overlayColor = '#40FF8C00'
            this.blendModeIndex = 3
          })
        Button('冷色调')
          .fontSize(12)
          .onClick(() => {
            this.overlayEnabled = true
            this.overlayColor = '#4000BFFF'
            this.blendModeIndex = 3
          })
        Button('复古')
          .fontSize(12)
          .onClick(() => {
            this.overlayEnabled = true
            this.overlayColor = '#308B4513'
            this.blendModeIndex = 2
          })
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildFilterSection() {
    Column({ space: 12 }) {
      Text('图像滤镜')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Text(`灰度: ${this.grayscaleValue}`)
        .fontSize(14)
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

      Text(`亮度: ${this.brightnessValue.toFixed(2)}`)
        .fontSize(14)
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

      Text(`对比度: ${this.contrastValue.toFixed(2)}`)
        .fontSize(14)
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

      Text(`饱和度: ${this.saturateValue.toFixed(2)}`)
        .fontSize(14)
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

      Text(`色相旋转: ${this.hueRotateValue}°`)
        .fontSize(14)
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

      Text(`反色: ${this.invertValue}`)
        .fontSize(14)
      Slider({
        value: this.invertValue,
        min: 0,
        max: 1,
        step: 0.1
      })
        .width('100%')
        .onChange((value: number) => {
          this.invertValue = value
        })

      Text(`怀旧: ${this.sepiaValue}`)
        .fontSize(14)
      Slider({
        value: this.sepiaValue,
        min: 0,
        max: 1,
        step: 0.1
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
  }

  @Builder
  buildResetButton() {
    Button('重置所有效果')
      .width('100%')
      .height(48)
      .fontSize(16)
      .backgroundColor(0x007DFF)
      .onClick(() => {
        this.blurRadius = 0
        this.shadowRadius = 0
        this.shadowOffsetX = 0
        this.shadowOffsetY = 8
        this.opacityValue = 1.0
        this.maskEnabled = false
        this.maskOpacity = 50
        this.blendModeIndex = 0
        this.overlayEnabled = false
        this.overlayColor = '#33007DFF'
        this.grayscaleValue = 0
        this.brightnessValue = 1.0
        this.contrastValue = 1.0
        this.saturateValue = 1.0
        this.hueRotateValue = 0
        this.invertValue = 0
        this.sepiaValue = 0
      })
  }
}
