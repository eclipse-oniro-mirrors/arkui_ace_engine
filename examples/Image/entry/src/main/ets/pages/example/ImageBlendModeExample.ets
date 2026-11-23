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
 * Image 混合模式示例
 * 展示 blendMode 属性的各种混合效果
 */
@Entry
@Component
struct ImageBlendModeExample {
  @State blendModeIndex: number = 0
  @State overlayColor: string = '#80FF0000'
  @State overlayOpacity: number = 50
  @State showOverlay: boolean = true

  private blendModes: { name: string, value: BlendMode }[] = [
    { name: 'SrcOver', value: BlendMode.SrcOver },
    { name: 'SrcIn', value: BlendMode.SrcIn },
    { name: 'SrcOut', value: BlendMode.SrcOut },
    { name: 'SrcAtop', value: BlendMode.SrcAtop },
    { name: 'DstOver', value: BlendMode.DstOver },
    { name: 'DstIn', value: BlendMode.DstIn },
    { name: 'DstOut', value: BlendMode.DstOut },
    { name: 'DstAtop', value: BlendMode.DstAtop },
    { name: 'Plus', value: BlendMode.Plus },
    { name: 'Xor', value: BlendMode.Xor },
    { name: 'Screen', value: BlendMode.Screen },
    { name: 'Overlay', value: BlendMode.Overlay },
    { name: 'Multiply', value: BlendMode.Multiply },
    { name: 'Darken', value: BlendMode.Darken },
    { name: 'Lighten', value: BlendMode.Lighten },
    { name: 'Difference', value: BlendMode.Difference },
    { name: 'Exclusion', value: BlendMode.Exclusion }
  ]

  private overlayColors: { name: string, color: string }[] = [
    { name: '红色', color: '#80FF0000' },
    { name: '绿色', color: '#8000FF00' },
    { name: '蓝色', color: '#800000FF' },
    { name: '黄色', color: '#80FFFF00' },
    { name: '青色', color: '#8000FFFF' },
    { name: '品红', color: '#80FF00FF' },
    { name: '白色', color: '#80FFFFFF' },
    { name: '黑色', color: '#80000000' }
  ]

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 混合模式示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildPreviewSection()
        this.buildBlendModeSection()
        this.buildColorSection()
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
      Text('预览效果')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Stack() {
        Image($r('app.media.startIcon'))
          .width(200)
          .height(200)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)

        if (this.showOverlay) {
          Column()
            .width(200)
            .height(200)
            .borderRadius(12)
            .backgroundColor(this.overlayColor)
            .blendMode(this.blendModes[this.blendModeIndex].value)
        }
      }
      .width(200)
      .height(200)
      .alignContent(Alignment.Center)

      Text(`模式: ${this.blendModes[this.blendModeIndex].name}`)
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
  buildBlendModeSection() {
    Column({ space: 12 }) {
      Text('混合模式选择')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('当前: ' + this.blendModes[this.blendModeIndex].name)
        .fontSize(14)
        .fontColor(0x007DFF)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.blendModes, (item: { name: string, value: BlendMode }, index: number) => {
          Text(item.name)
            .fontSize(11)
            .padding({ left: 10, right: 10, top: 6, bottom: 6 })
            .margin(3)
            .borderRadius(14)
            .backgroundColor(this.blendModeIndex === index ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.blendModeIndex === index ? Color.White : 0x333333)
            .onClick(() => {
              this.blendModeIndex = index
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
  buildColorSection() {
    Column({ space: 12 }) {
      Text('叠加颜色')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.overlayColors, (item: { name: string, color: string }) => {
          Column({ space: 4 }) {
            Column()
              .width(40)
              .height(40)
              .borderRadius(20)
              .backgroundColor(item.color)
              .border({ width: this.overlayColor === item.color ? 3 : 0, color: '#333333' })
              .onClick(() => {
                this.overlayColor = item.color
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
  }

  @Builder
  buildOpacitySection() {
    Column({ space: 12 }) {
      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.showOverlay })
          .onChange((isOn: boolean) => {
            this.showOverlay = isOn
          })
        Text('显示叠加层')
          .fontSize(16)
          .fontWeight(FontWeight.Medium)
      }

      if (this.showOverlay) {
        Text(`叠加不透明度: ${this.overlayOpacity}%`)
          .fontSize(14)
          .fontColor(0x666666)

        Slider({
          value: this.overlayOpacity,
          min: 10,
          max: 100,
          step: 5
        })
          .width('100%')
          .onChange((value: number) => {
            this.overlayOpacity = value
            const alpha = Math.round(value * 255 / 100).toString(16).padStart(2, '0')
            this.overlayColor = alpha + this.overlayColor.substring(3)
          })
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
      Text('混合模式对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('同一图片不同混合模式对比')
        .fontSize(12)
        .fontColor(0x999999)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach([
          { name: 'Screen', mode: BlendMode.Screen },
          { name: 'Overlay', mode: BlendMode.Overlay },
          { name: 'Multiply', mode: BlendMode.Multiply },
          { name: 'Difference', mode: BlendMode.Difference }
        ], (item: { name: string, mode: BlendMode }) => {
          Column({ space: 4 }) {
            Stack() {
              Image($r('app.media.heart'))
                .width(70)
                .height(70)
                .objectFit(ImageFit.Cover)
                .borderRadius(8)
              Column()
                .width(70)
                .height(70)
                .borderRadius(8)
                .backgroundColor('#8000FFFF')
                .blendMode(item.mode)
            }
            .width(70)
            .height(70)
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
  }

  @Builder
  buildTipsSection() {
    Column({ space: 8 }) {
      Text('使用提示')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('1. Screen 模式：变亮效果，适合光效叠加')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. Multiply 模式：变暗效果，适合阴影叠加')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. Overlay 模式：增强对比度，保留底色')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. Difference 模式：差值效果，产生反色')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 混合模式需要配合叠加层使用')
        .fontSize(12)
        .fontColor(0x666666)
      Text('6. 不同颜色叠加产生不同效果')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
