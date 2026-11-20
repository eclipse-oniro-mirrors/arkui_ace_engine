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
 * Image 透明度过渡示例
 * 展示 opacity 属性的动画效果
 */
@Entry
@Component
struct ImageOpacityTransitionExample {
  @State opacityValue: number = 1.0
  @State isAnimating: boolean = false
  @State animationLog: string = '等待动画...'
  @State animationType: number = 0

  private logs: string[] = []
  private animationTypes: string[] = ['线性', '缓入', '缓出', '缓入缓出', '弹性']

  addLog(msg: string) {
    const time = new Date().toLocaleTimeString()
    this.logs.unshift(`[${time}] ${msg}`)
    if (this.logs.length > 5) {
      this.logs.pop()
    }
    this.animationLog = this.logs.join('\n')
  }

  startAnimation() {
    if (this.isAnimating) {
      return
    }
    this.isAnimating = true
    this.addLog('动画开始')

    const curves: Curve[] = [Curve.Linear, Curve.EaseIn, Curve.EaseOut, Curve.EaseInOut, Curve.FastOutSlowIn]
    const curve = curves[this.animationType]

    animateTo({ duration: 800, curve: curve }, () => {
      this.opacityValue = this.opacityValue > 0.5 ? 0.1 : 1.0
    })

    setTimeout(() => {
      this.isAnimating = false
      this.addLog(`动画结束: ${this.animationTypes[this.animationType]}`)
    }, 800)
  }

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 透明度过渡示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildAnimationAreaSection()
        this.buildLogSection()
        this.buildAnimationTypeSection()
        this.buildControlSection()
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
  buildAnimationAreaSection() {
    Column({ space: 12 }) {
      Text('动画区域')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Stack() {
        Column()
          .width(150)
          .height(150)
          .borderRadius(12)
          .backgroundColor(0xE0E0E0)

        Image($r('app.media.heart'))
          .width(150)
          .height(150)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)
          .border({ width: this.isAnimating ? 3 : 1, color: this.isAnimating ? 0x007DFF : 0xCCCCCC })
          .opacity(this.opacityValue)
      }
      .width(150)
      .height(150)
      .alignContent(Alignment.Center)

      Text(`当前透明度: ${(this.opacityValue * 100).toFixed(0)}%`)
        .fontSize(14)
        .fontColor(0x666666)

      Button(this.isAnimating ? '动画中...' : '播放动画')
        .fontSize(14)
        .height(44)
        .width(150)
        .backgroundColor(this.isAnimating ? 0xCCCCCC : 0x007DFF)
        .onClick(() => {
          this.startAnimation()
        })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildLogSection() {
    Column({ space: 10 }) {
      Text('动画日志')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Scroll() {
        Text(this.animationLog)
          .fontSize(12)
          .fontColor(0x333333)
          .fontFamily('monospace')
      }
      .width('100%')
      .height(100)
      .backgroundColor(0xF8F8F8)
      .borderRadius(8)
      .padding(10)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildAnimationTypeSection() {
    Column({ space: 12 }) {
      Text('动画曲线')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.animationTypes, (name: string, index: number) => {
          Text(name)
            .fontSize(12)
            .padding({ left: 14, right: 14, top: 8, bottom: 8 })
            .margin(4)
            .borderRadius(18)
            .backgroundColor(this.animationType === index ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.animationType === index ? Color.White : 0x333333)
            .onClick(() => {
              this.animationType = index
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
  buildControlSection() {
    Column({ space: 12 }) {
      Text('控制选项')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text(`透明度: ${(this.opacityValue * 100).toFixed(0)}%`)
        .fontSize(14)
        .fontColor(0x666666)

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

      Row({ space: 10 }) {
        Button('完全显示')
          .fontSize(12)
          .layoutWeight(1)
          .height(40)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.opacityValue = 1.0
            })
          })

        Button('半透明')
          .fontSize(12)
          .layoutWeight(1)
          .height(40)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.opacityValue = 0.5
            })
          })

        Button('隐藏')
          .fontSize(12)
          .layoutWeight(1)
          .height(40)
          .backgroundColor(0xFF6B6B)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.opacityValue = 0
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
  buildComparisonSection() {
    Column({ space: 12 }) {
      Text('透明度对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 8 }) {
        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .opacity(0.2)
          Text('20%')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .opacity(0.4)
          Text('40%')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .opacity(0.6)
          Text('60%')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .opacity(0.8)
          Text('80%')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .opacity(1.0)
          Text('100%')
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

      Text('1. opacity 值范围 0-1，0 为完全透明')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 使用 animateTo 实现平滑过渡')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 常用于淡入淡出效果')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 可配合其他动画属性组合使用')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 透明度为 0 时图片不可见但仍占位')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
