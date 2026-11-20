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
 * Image 遮罩效果示例
 * 展示 mask 属性的各种遮罩效果
 */
@Entry
@Component
struct ImageMaskEffectExample {
  @State maskTypeIndex: number = 0
  @State maskOpacity: number = 50
  @State maskText: string = '图片标题'
  @State gradientAngle: number = 180
  @State showMask: boolean = true

  private maskTypes: string[] = ['底部渐变', '顶部渐变', '全透明', '居中聚焦', '左右渐变', '径向渐变']

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 遮罩效果示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildPreviewSection()
        this.buildMaskTypeSection()
        this.buildOpacitySection()
        this.buildTextSection()
        this.buildGradientSection()
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
          .width(220)
          .height(180)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)

        if (this.showMask) {
          this.buildMaskOverlay()
        }
      }
      .width(220)
      .height(180)
      .alignContent(Alignment.Center)

      Text(`遮罩类型: ${this.maskTypes[this.maskTypeIndex]}`)
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
  buildMaskOverlay() {
    if (this.maskTypeIndex === 0) {
      Column()
        .width(220)
        .height(90)
        .linearGradient({
          angle: 180,
          colors: [['rgba(0,0,0,0)', 0], [`rgba(0,0,0,${this.maskOpacity / 100})`, 1.0]]
        })
        .borderRadius({ bottomLeft: 12, bottomRight: 12 })
        .alignContent(Alignment.Bottom)
        .padding({ bottom: 12 })
    } else if (this.maskTypeIndex === 1) {
      Column()
        .width(220)
        .height(90)
        .linearGradient({
          angle: 0,
          colors: [['rgba(0,0,0,0)', 0], [`rgba(0,0,0,${this.maskOpacity / 100})`, 1.0]]
        })
        .borderRadius({ topLeft: 12, topRight: 12 })
        .alignContent(Alignment.Top)
        .padding({ top: 12 })
    } else if (this.maskTypeIndex === 2) {
      Column()
        .width(220)
        .height(180)
        .borderRadius(12)
        .backgroundColor(`rgba(0, 0, 0, ${this.maskOpacity / 100})`)
    } else if (this.maskTypeIndex === 3) {
      Stack() {
        Column()
          .width(220)
          .height(180)
          .linearGradient({
            angle: 90,
            colors: [
              [`rgba(0,0,0,${this.maskOpacity / 100})`, 0],
              ['rgba(0,0,0,0)', 0.3],
              ['rgba(0,0,0,0)', 0.7],
              [`rgba(0,0,0,${this.maskOpacity / 100})`, 1.0]
            ]
          })
          .borderRadius(12)

        Text(this.maskText)
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
          .fontColor(Color.White)
          .shadow({ radius: 4, color: '#80000000', offsetX: 0, offsetY: 2 })
      }
      .width(220)
      .height(180)
    } else if (this.maskTypeIndex === 4) {
      Column()
        .width(220)
        .height(180)
        .linearGradient({
          angle: this.gradientAngle,
          colors: [
            [`rgba(0,0,0,${this.maskOpacity / 100})`, 0],
            ['rgba(0,0,0,0)', 0.5],
            [`rgba(0,0,0,${this.maskOpacity / 100})`, 1.0]
          ]
        })
        .borderRadius(12)
    } else if (this.maskTypeIndex === 5) {
      Column()
        .width(220)
        .height(180)
        .radialGradient({
          center: ['50%', '50%'],
          radius: '50%',
          colors: [
            ['rgba(0,0,0,0)', 0],
            [`rgba(0,0,0,${this.maskOpacity / 100})`, 1.0]
          ]
        })
        .borderRadius(12)
    }
  }

  @Builder
  buildMaskTypeSection() {
    Column({ space: 12 }) {
      Text('遮罩类型')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.maskTypes, (name: string, index: number) => {
          Text(name)
            .fontSize(12)
            .padding({ left: 12, right: 12, top: 6, bottom: 6 })
            .margin(4)
            .borderRadius(16)
            .backgroundColor(this.maskTypeIndex === index ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.maskTypeIndex === index ? Color.White : 0x333333)
            .onClick(() => {
              this.maskTypeIndex = index
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
  buildOpacitySection() {
    Column({ space: 12 }) {
      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.showMask })
          .onChange((isOn: boolean) => {
            this.showMask = isOn
          })
        Text('显示遮罩')
          .fontSize(16)
          .fontWeight(FontWeight.Medium)
      }

      if (this.showMask) {
        Text(`遮罩不透明度: ${this.maskOpacity}%`)
          .fontSize(14)
          .fontColor(0x666666)

        Slider({
          value: this.maskOpacity,
          min: 10,
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
  buildTextSection() {
    Column({ space: 12 }) {
      Text('叠加文字')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      if (this.maskTypeIndex === 3) {
        TextInput({ text: this.maskText, placeholder: '输入叠加文字' })
          .width('100%')
          .onChange((value: string) => {
            this.maskText = value
          })

        Text('提示: 仅"居中聚焦"模式支持文字叠加')
          .fontSize(12)
          .fontColor(0x999999)
      } else {
        Text('提示: 仅"居中聚焦"模式支持文字叠加')
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
  buildGradientSection() {
    Column({ space: 12 }) {
      Text('渐变角度')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      if (this.maskTypeIndex === 4) {
        Text(`角度: ${this.gradientAngle}°`)
          .fontSize(14)
          .fontColor(0x666666)

        Slider({
          value: this.gradientAngle,
          min: 0,
          max: 360,
          step: 10
        })
          .width('100%')
          .onChange((value: number) => {
            this.gradientAngle = value
          })
      } else {
        Text('提示: 仅"左右渐变"模式支持角度调整')
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
      Text('遮罩对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('同一图片不同遮罩效果对比')
        .fontSize(12)
        .fontColor(0x999999)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach([
          { name: '底部', type: 0 },
          { name: '顶部', type: 1 },
          { name: '全透明', type: 2 },
          { name: '居中', type: 3 }
        ], (item: { name: string, type: number }) => {
          Column({ space: 4 }) {
            Stack() {
              Image($r('app.media.heart'))
                .width(70)
                .height(70)
                .objectFit(ImageFit.Cover)
                .borderRadius(8)
              if (item.type === 0) {
                Column()
                  .width(70)
                  .height(35)
                  .linearGradient({ angle: 180, colors: [['rgba(0,0,0,0)', 0], ['rgba(0,0,0,0.5)', 1.0]] })
                  .borderRadius({ bottomLeft: 8, bottomRight: 8 })
              } else if (item.type === 1) {
                Column()
                  .width(70)
                  .height(35)
                  .linearGradient({ angle: 0, colors: [['rgba(0,0,0,0)', 0], ['rgba(0,0,0,0.5)', 1.0]] })
                  .borderRadius({ topLeft: 8, topRight: 8 })
              } else if (item.type === 2) {
                Column()
                  .width(70)
                  .height(70)
                  .borderRadius(8)
                  .backgroundColor('rgba(0,0,0,0.5)')
              } else if (item.type === 3) {
                Column()
                  .width(70)
                  .height(70)
                  .linearGradient({
                    angle: 90,
                    colors: [['rgba(0,0,0,0.5)', 0], ['rgba(0,0,0,0)', 0.3], ['rgba(0,0,0,0)', 0.7], ['rgba(0,0,0,0.5)', 1.0]]
                  })
                  .borderRadius(8)
              }
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

      Text('1. 底部渐变：适合文字说明叠加在底部')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 顶部渐变：适合标题叠加在顶部')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 全透明：适合整体变暗效果')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 居中聚焦：适合突出中心内容')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 左右渐变：适合横向文字叠加')
        .fontSize(12)
        .fontColor(0x666666)
      Text('6. 径向渐变：适合圆形聚焦效果')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
