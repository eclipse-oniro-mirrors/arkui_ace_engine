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
 * Image 模糊效果示例
 * 展示 blur 属性的各种模糊效果
 */
@Entry
@Component
struct ImageBlurEffectExample {
  @State blurValue: number = 0
  @State blurAnimated: number = 0
  @State isAnimating: boolean = false
  @State presetIndex: number = 0

  private presets: { name: string, value: number }[] = [
    { name: '无模糊', value: 0 },
    { name: '轻微', value: 4 },
    { name: '中等', value: 10 },
    { name: '强烈', value: 20 },
    { name: '极度', value: 30 }
  ]

  blurTimer: number = -1

  aboutToDisappear() {
    if (this.blurTimer !== -1) {
      clearInterval(this.blurTimer)
    }
  }

  startBlurAnimation() {
    if (this.isAnimating) {
      this.isAnimating = false
      if (this.blurTimer !== -1) {
        clearInterval(this.blurTimer)
        this.blurTimer = -1
      }
      return
    }
    this.isAnimating = true
    let increasing = true
    this.blurTimer = setInterval(() => {
      if (increasing) {
        this.blurAnimated += 1
        if (this.blurAnimated >= 30) {
          increasing = false
        }
      } else {
        this.blurAnimated -= 1
        if (this.blurAnimated <= 0) {
          increasing = true
        }
      }
    }, 50)
  }

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 模糊效果示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildBasicBlurSection()
        this.buildAnimatedBlurSection()
        this.buildPresetBlurSection()
        this.buildBlurComparisonSection()
        this.buildBlurWithRadiusSection()
        this.buildBlurWithOpacitySection()
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
  buildBasicBlurSection() {
    Column({ space: 12 }) {
      Text('基础模糊')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Image($r('app.media.startIcon'))
        .width(200)
        .height(200)
        .objectFit(ImageFit.Cover)
        .borderRadius(12)
        .blur(this.blurValue)

      Text(`模糊半径: ${this.blurValue}`)
        .fontSize(14)
        .fontColor(0x666666)

      Slider({
        value: this.blurValue,
        min: 0,
        max: 30,
        step: 1
      })
        .width('100%')
        .onChange((value: number) => {
          this.blurValue = value
        })

      Row({ space: 10 }) {
        Button('清除')
          .fontSize(12)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.blurValue = 0
            })
          })
        Button('最大')
          .fontSize(12)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.blurValue = 30
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
  buildAnimatedBlurSection() {
    Column({ space: 12 }) {
      Text('动画模糊')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Image($r('app.media.background'))
        .width(200)
        .height(150)
        .objectFit(ImageFit.Cover)
        .borderRadius(12)
        .blur(this.blurAnimated)

      Text(`当前值: ${this.blurAnimated}`)
        .fontSize(14)
        .fontColor(0x666666)

      Button(this.isAnimating ? '停止动画' : '开始动画')
        .fontSize(13)
        .backgroundColor(this.isAnimating ? 0xFF6B6B : 0x4CAF50)
        .onClick(() => {
          this.startBlurAnimation()
        })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildPresetBlurSection() {
    Column({ space: 12 }) {
      Text('预设模糊')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Image($r('app.media.foreground'))
        .width(180)
        .height(180)
        .objectFit(ImageFit.Cover)
        .borderRadius(12)
        .blur(this.presets[this.presetIndex].value)

      Text(`预设: ${this.presets[this.presetIndex].name}`)
        .fontSize(14)
        .fontColor(0x666666)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.presets, (item: { name: string, value: number }, index: number) => {
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
  buildBlurComparisonSection() {
    Column({ space: 12 }) {
      Text('模糊对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('同一图片不同模糊程度对比')
        .fontSize(12)
        .fontColor(0x999999)

      Row({ space: 8 }) {
        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(70)
            .height(70)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
          Text('原图')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(70)
            .height(70)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .blur(5)
          Text('5px')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(70)
            .height(70)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .blur(15)
          Text('15px')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(70)
            .height(70)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .blur(25)
          Text('25px')
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

      Text('1. blur 值范围 0-30，0 表示无模糊')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 模糊值越大，图片越模糊')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 可配合 animateTo 实现平滑过渡')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 常用于背景虚化、毛玻璃效果')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 高模糊值可能影响性能')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildBlurWithRadiusSection() {
    Column({ space: 12 }) {
      Text('模糊与圆角组合')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('模糊效果与圆角组合使用')
        .fontSize(12)
        .fontColor(0x999999)

      Row({ space: 12 }) {
        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(80)
            .height(80)
            .objectFit(ImageFit.Cover)
            .borderRadius(40)
            .blur(0)
          Text('圆形原图')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(80)
            .height(80)
            .objectFit(ImageFit.Cover)
            .borderRadius(40)
            .blur(5)
          Text('圆形模糊')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(80)
            .height(80)
            .objectFit(ImageFit.Cover)
            .borderRadius(16)
            .blur(10)
          Text('圆角模糊')
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
  buildBlurWithOpacitySection() {
    Column({ space: 12 }) {
      Text('模糊与透明度组合')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('模糊效果与透明度组合使用')
        .fontSize(12)
        .fontColor(0x999999)

      Row({ space: 12 }) {
        Column({ space: 4 }) {
          Image($r('app.media.svg'))
            .width(70)
            .height(70)
            .objectFit(ImageFit.Cover)
            .opacity(1.0)
          Text('原图')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.svg'))
            .width(70)
            .height(70)
            .objectFit(ImageFit.Cover)
            .opacity(0.7)
          Text('半透明')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.svg'))
            .width(70)
            .height(70)
            .objectFit(ImageFit.Cover)
            .blur(8)
            .opacity(0.5)
          Text('模糊半透明')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.svg'))
            .width(70)
            .height(70)
            .objectFit(ImageFit.Cover)
            .blur(15)
            .opacity(0.3)
          Text('强模糊透明')
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
