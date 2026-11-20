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
 * Image 合成与裁剪示例
 * 展示边框、阴影、遮罩、裁剪、圆角变体、图文叠加等合成效果
 */
@Entry
@Component
struct ImageCompositingExample {
  @State borderWidth: number = 0
  @State borderColor: string = '#007DFF'
  @State borderRadiusUniform: number = 0
  @State cornerTopLeft: number = 0
  @State cornerTopRight: number = 0
  @State cornerBottomLeft: number = 0
  @State cornerBottomRight: number = 0
  @State useUniformRadius: boolean = true
  @State shadowRadius: number = 0
  @State shadowOffsetY: number = 4
  @State shadowColor: string = '#40000000'
  @State clipShapeIndex: number = 0
  @State maskTypeIndex: number = 0
  @State maskOpacity: number = 50
  @State maskText: string = '图片标题'
  @State overlayGradient: boolean = false
  @State overlayPosition: number = 0
  @State imageWidth: number = 220
  @State imageHeight: number = 180
  @State imageFit: ImageFit = ImageFit.Cover
  @State borderStyleIndex: number = 0
  @State enableReflection: boolean = false
  @State reflectionOpacity: number = 30

  clipShapes: string[] = ['无', '圆形', '椭圆', '圆角矩形']
  maskTypes: string[] = ['底部渐变', '全透明', '顶部渐变', '居中聚焦']
  borderStyles: string[] = ['Solid', 'Dashed', 'Dotted']

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 合成与裁剪示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildPreviewSection()
        this.buildSizeSection()
        this.buildBorderSection()
        this.buildRadiusSection()
        this.buildShadowSection()
        this.buildClipSection()
        this.buildMaskSection()
        this.buildOverlaySection()
        this.buildPresetsSection()
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
      Text('预览')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Stack({ alignContent: Alignment.Bottom }) {
        Image($r('app.media.startIcon'))
          .width(this.imageWidth)
          .height(this.imageHeight)
          .objectFit(this.imageFit)
          .border({
            width: this.borderWidth,
            color: this.borderColor,
            style: this.borderStyles[this.borderStyleIndex] === 'Dashed' ? BorderStyle.Dashed :
              this.borderStyles[this.borderStyleIndex] === 'Dotted' ? BorderStyle.Dotted : BorderStyle.Solid
          })
          .borderRadius(this.useUniformRadius ? this.borderRadiusUniform : undefined)
          .borderRadius({
            topLeft: this.useUniformRadius ? this.borderRadiusUniform : this.cornerTopLeft,
            topRight: this.useUniformRadius ? this.borderRadiusUniform : this.cornerTopRight,
            bottomLeft: this.useUniformRadius ? this.borderRadiusUniform : this.cornerBottomLeft,
            bottomRight: this.useUniformRadius ? this.borderRadiusUniform : this.cornerBottomRight
          })
          .shadow({
            radius: this.shadowRadius,
            color: this.shadowColor,
            offsetX: 0,
            offsetY: this.shadowOffsetY
          })
          .clip(this.clipShapeIndex > 0)

        if (this.clipShapeIndex === 1) {
          Image($r('app.media.startIcon'))
            .width(this.imageWidth)
            .height(this.imageHeight)
            .objectFit(this.imageFit)
            .clipShape(ClipShape.Circle)
        } else if (this.clipShapeIndex === 2) {
          Image($r('app.media.startIcon'))
            .width(this.imageWidth)
            .height(this.imageHeight)
            .objectFit(this.imageFit)
            .clipShape(ClipShape.Ellipse)
        } else if (this.clipShapeIndex === 3) {
          Image($r('app.media.startIcon'))
            .width(this.imageWidth)
            .height(this.imageHeight)
            .objectFit(this.imageFit)
            .clipShape(ClipShape.RoundRect)
        }

        if (this.maskTypeIndex > 0) {
          this.buildMaskOverlay()
        }

        if (this.overlayGradient) {
          this.buildGradientOverlay()
        }
      }
      .width(this.imageWidth)
      .height(this.imageHeight)
      .alignContent(Alignment.Center)

      if (this.enableReflection) {
        Image($r('app.media.startIcon'))
          .width(this.imageWidth)
          .height(this.imageHeight / 2)
          .objectFit(this.imageFit)
          .scale({ x: 1, y: -1 })
          .opacity(this.reflectionOpacity / 100)
          .mask(
            new LinearGradient()
              .angle(180)
              .colors([['rgba(0,0,0,1)', 0], ['rgba(0,0,0,0)', 1.0]])
          )
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildMaskOverlay() {
    if (this.maskTypeIndex === 1) {
      Column()
        .width(this.imageWidth)
        .height(this.imageHeight)
        .backgroundColor(`rgba(0, 0, 0, ${this.maskOpacity / 100})`)
    } else if (this.maskTypeIndex === 2) {
      Column()
        .width(this.imageWidth)
        .height(this.imageHeight / 2)
        .linearGradient({
          angle: 180,
          colors: [['rgba(0,0,0,0.6)', 0], ['rgba(0,0,0,0)', 1.0]]
        })
        .alignContent(Alignment.Top)
    } else if (this.maskTypeIndex === 3) {
      Stack() {
        Column()
          .width(this.imageWidth)
          .height(this.imageHeight)
          .linearGradient({
            angle: 90,
            colors: [
              ['rgba(0,0,0,0.5)', 0],
              ['rgba(0,0,0,0)', 0.3],
              ['rgba(0,0,0,0)', 0.7],
              ['rgba(0,0,0,0.5)', 1.0]
            ]
          })

        Text(this.maskText)
          .fontSize(18)
          .fontWeight(FontWeight.Bold)
          .fontColor(Color.White)
          .shadow({ radius: 4, color: '#80000000', offsetX: 0, offsetY: 2 })
      }
      .width(this.imageWidth)
      .height(this.imageHeight)
    }
  }

  @Builder
  buildGradientOverlay() {
    Column()
      .width(this.imageWidth)
      .height(this.imageHeight * this.overlayPosition / 100)
      .linearGradient({
        angle: 180,
        colors: [['rgba(0,0,0,0.7)', 0], ['rgba(0,0,0,0)', 1.0]]
      })
  }

  @Builder
  buildSizeSection() {
    Column({ space: 10 }) {
      Text('尺寸')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Text(`宽度: ${this.imageWidth}`)
        .fontSize(13)
      Slider({ value: this.imageWidth, min: 80, max: 320, step: 10 })
        .width('100%')
        .onChange((v: number) => { this.imageWidth = v })

      Text(`高度: ${this.imageHeight}`)
        .fontSize(13)
      Slider({ value: this.imageHeight, min: 80, max: 320, step: 10 })
        .width('100%')
        .onChange((v: number) => { this.imageHeight = v })

      Flex({ wrap: FlexWrap.Wrap }) {
        ForEach(['Cover', 'Contain', 'Fill', 'Auto', 'None', 'ScaleDown'], (name: string, index: number) => {
          Text(name)
            .fontSize(11)
            .padding({ left: 10, right: 10, top: 5, bottom: 5 })
            .margin(3)
            .borderRadius(14)
            .backgroundColor([ImageFit.Cover, ImageFit.Contain, ImageFit.Fill,
              ImageFit.Auto, ImageFit.None, ImageFit.ScaleDown][index] === this.imageFit
              ? 0x007DFF : 0xE8E8E8)
            .fontColor([ImageFit.Cover, ImageFit.Contain, ImageFit.Fill,
              ImageFit.Auto, ImageFit.None, ImageFit.ScaleDown][index] === this.imageFit
              ? Color.White : 0x333333)
            .onClick(() => {
              this.imageFit = [ImageFit.Cover, ImageFit.Contain, ImageFit.Fill,
                ImageFit.Auto, ImageFit.None, ImageFit.ScaleDown][index]
            })
        })
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildBorderSection() {
    Column({ space: 10 }) {
      Text('边框')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Text(`边框宽度: ${this.borderWidth}`)
        .fontSize(13)
      Slider({ value: this.borderWidth, min: 0, max: 12, step: 1 })
        .width('100%')
        .onChange((v: number) => { this.borderWidth = v })

      Row({ space: 8 }) {
        ForEach(['#007DFF', '#FF0000', '#00FF00', '#FFD700', '#FF69B4', '#000000'], (color: string) => {
          Column()
            .width(32)
            .height(32)
            .borderRadius(16)
            .backgroundColor(color)
            .border({ width: this.borderColor === color ? 3 : 0, color: '#333333' })
            .onClick(() => { this.borderColor = color })
        })
      }

      Row({ space: 8 }) {
        ForEach(this.borderStyles, (style: string, index: number) => {
          Text(style)
            .fontSize(12)
            .padding({ left: 12, right: 12, top: 6, bottom: 6 })
            .borderRadius(16)
            .backgroundColor(this.borderStyleIndex === index ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.borderStyleIndex === index ? Color.White : 0x333333)
            .onClick(() => { this.borderStyleIndex = index })
        })
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildRadiusSection() {
    Column({ space: 10 }) {
      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.useUniformRadius })
          .onChange((isOn: boolean) => { this.useUniformRadius = isOn })
        Text(this.useUniformRadius ? '统一圆角' : '独立圆角')
          .fontSize(14)
          .fontWeight(FontWeight.Medium)
      }

      if (this.useUniformRadius) {
        Text(`圆角: ${this.borderRadiusUniform}`)
          .fontSize(13)
        Slider({ value: this.borderRadiusUniform, min: 0, max: 100, step: 2 })
          .width('100%')
          .onChange((v: number) => { this.borderRadiusUniform = v })
      } else {
        Text(`左上: ${this.cornerTopLeft}`)
          .fontSize(13)
        Slider({ value: this.cornerTopLeft, min: 0, max: 80, step: 2 })
          .width('100%')
          .onChange((v: number) => { this.cornerTopLeft = v })

        Text(`右上: ${this.cornerTopRight}`)
          .fontSize(13)
        Slider({ value: this.cornerTopRight, min: 0, max: 80, step: 2 })
          .width('100%')
          .onChange((v: number) => { this.cornerTopRight = v })

        Text(`左下: ${this.cornerBottomLeft}`)
          .fontSize(13)
        Slider({ value: this.cornerBottomLeft, min: 0, max: 80, step: 2 })
          .width('100%')
          .onChange((v: number) => { this.cornerBottomLeft = v })

        Text(`右下: ${this.cornerBottomRight}`)
          .fontSize(13)
        Slider({ value: this.cornerBottomRight, min: 0, max: 80, step: 2 })
          .width('100%')
          .onChange((v: number) => { this.cornerBottomRight = v })
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildShadowSection() {
    Column({ space: 10 }) {
      Text('阴影')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Text(`阴影半径: ${this.shadowRadius}`)
        .fontSize(13)
      Slider({ value: this.shadowRadius, min: 0, max: 40, step: 1 })
        .width('100%')
        .onChange((v: number) => { this.shadowRadius = v })

      Text(`Y 偏移: ${this.shadowOffsetY}`)
        .fontSize(13)
      Slider({ value: this.shadowOffsetY, min: -20, max: 20, step: 1 })
        .width('100%')
        .onChange((v: number) => { this.shadowOffsetY = v })
    }
    .width('100%')
    .padding(14)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildClipSection() {
    Column({ space: 10 }) {
      Text('裁剪形状')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Row({ space: 8 }) {
        ForEach(this.clipShapes, (name: string, index: number) => {
          Text(name)
            .fontSize(12)
            .padding({ left: 12, right: 12, top: 6, bottom: 6 })
            .borderRadius(16)
            .backgroundColor(this.clipShapeIndex === index ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.clipShapeIndex === index ? Color.White : 0x333333)
            .onClick(() => { this.clipShapeIndex = index })
        })
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildMaskSection() {
    Column({ space: 10 }) {
      Text('遮罩类型')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Row({ space: 8 }) {
        ForEach(this.maskTypes, (name: string, index: number) => {
          Text(name)
            .fontSize(11)
            .padding({ left: 10, right: 10, top: 6, bottom: 6 })
            .borderRadius(16)
            .backgroundColor(this.maskTypeIndex === index ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.maskTypeIndex === index ? Color.White : 0x333333)
            .onClick(() => { this.maskTypeIndex = index })
        })
      }

      if (this.maskTypeIndex > 0) {
        Text(`遮罩不透明度: ${this.maskOpacity}%`)
          .fontSize(13)
        Slider({ value: this.maskOpacity, min: 0, max: 100, step: 5 })
          .width('100%')
          .onChange((v: number) => { this.maskOpacity = v })
      }

      if (this.maskTypeIndex === 3) {
        TextInput({ text: this.maskText, placeholder: '输入叠加文字' })
          .width('100%')
          .onChange((value: string) => { this.maskText = value })
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildOverlaySection() {
    Column({ space: 10 }) {
      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.overlayGradient })
          .onChange((isOn: boolean) => { this.overlayGradient = isOn })
        Text('顶部渐变叠加')
          .fontSize(14)
      }

      if (this.overlayGradient) {
        Text(`渐变高度: ${this.overlayPosition}%`)
          .fontSize(13)
        Slider({ value: this.overlayPosition, min: 10, max: 100, step: 5 })
          .width('100%')
          .onChange((v: number) => { this.overlayPosition = v })
      }

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.enableReflection })
          .onChange((isOn: boolean) => { this.enableReflection = isOn })
        Text('倒影效果')
          .fontSize(14)
      }

      if (this.enableReflection) {
        Text(`倒影不透明度: ${this.reflectionOpacity}%`)
          .fontSize(13)
        Slider({ value: this.reflectionOpacity, min: 5, max: 60, step: 5 })
          .width('100%')
          .onChange((v: number) => { this.reflectionOpacity = v })
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildPresetsSection() {
    Column({ space: 10 }) {
      Text('预设样式')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Row({ space: 8 }) {
        Button('头像')
          .fontSize(12)
          .layoutWeight(1)
          .height(36)
          .onClick(() => {
            this.imageWidth = 120
            this.imageHeight = 120
            this.useUniformRadius = true
            this.borderRadiusUniform = 60
            this.borderWidth = 3
            this.borderColor = '#007DFF'
            this.shadowRadius = 8
            this.shadowOffsetY = 4
            this.clipShapeIndex = 0
            this.maskTypeIndex = 0
            this.overlayGradient = false
          })

        Button('卡片')
          .fontSize(12)
          .layoutWeight(1)
          .height(36)
          .onClick(() => {
            this.imageWidth = 280
            this.imageHeight = 180
            this.useUniformRadius = true
            this.borderRadiusUniform = 16
            this.borderWidth = 0
            this.shadowRadius = 16
            this.shadowOffsetY = 8
            this.maskTypeIndex = 2
            this.clipShapeIndex = 0
            this.overlayGradient = false
          })
      }

      Row({ space: 8 }) {
        Button('海报')
          .fontSize(12)
          .layoutWeight(1)
          .height(36)
          .onClick(() => {
            this.imageWidth = 200
            this.imageHeight = 280
            this.useUniformRadius = true
            this.borderRadiusUniform = 12
            this.borderWidth = 0
            this.shadowRadius = 20
            this.shadowOffsetY = 10
            this.maskTypeIndex = 3
            this.maskText = '电影海报'
            this.clipShapeIndex = 0
            this.overlayGradient = false
          })

        Button('徽章')
          .fontSize(12)
          .layoutWeight(1)
          .height(36)
          .onClick(() => {
            this.imageWidth = 100
            this.imageHeight = 100
            this.useUniformRadius = true
            this.borderRadiusUniform = 50
            this.borderWidth = 4
            this.borderColor = '#FFD700'
            this.shadowRadius = 6
            this.shadowOffsetY = 3
            this.clipShapeIndex = 1
            this.maskTypeIndex = 0
            this.overlayGradient = false
          })
      }

      Row({ space: 8 }) {
        Button('重置')
          .fontSize(12)
          .layoutWeight(1)
          .height(36)
          .backgroundColor(0xFF6B6B)
          .onClick(() => {
            this.borderWidth = 0
            this.borderRadiusUniform = 0
            this.cornerTopLeft = 0
            this.cornerTopRight = 0
            this.cornerBottomLeft = 0
            this.cornerBottomRight = 0
            this.useUniformRadius = true
            this.shadowRadius = 0
            this.shadowOffsetY = 4
            this.clipShapeIndex = 0
            this.maskTypeIndex = 0
            this.maskOpacity = 50
            this.overlayGradient = false
            this.overlayPosition = 0
            this.imageWidth = 220
            this.imageHeight = 180
            this.imageFit = ImageFit.Cover
            this.borderStyleIndex = 0
            this.enableReflection = false
            this.reflectionOpacity = 30
          })
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
