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
 * Image 旋转动画示例
 * 展示 rotate 属性的动画效果
 */
@Entry
@Component
struct ImageRotateAnimationExample {
  @State rotateAngle: number = 0
  @State isAnimating: boolean = false
  @State animationLog: string = '等待动画...'
  @State animationType: number = 0
  @State rotateDirection: number = 0

  private logs: string[] = []
  private animationTypes: string[] = ['线性', '缓入', '缓出', '缓入缓出', '弹性']
  private directions: string[] = ['顺时针', '逆时针']

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
    const targetAngle = this.rotateDirection === 0 ? this.rotateAngle + 360 : this.rotateAngle - 360

    animateTo({ duration: 1000, curve: curve }, () => {
      this.rotateAngle = targetAngle
    })

    setTimeout(() => {
      this.isAnimating = false
      this.addLog(`动画结束: ${this.animationTypes[this.animationType]}`)
    }, 1000)
  }

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 旋转动画示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildAnimationAreaSection()
        this.buildLogSection()
        this.buildAnimationTypeSection()
        this.buildDirectionSection()
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

      Image($r('app.media.background'))
        .width(150)
        .height(150)
        .objectFit(ImageFit.Cover)
        .borderRadius(12)
        .border({ width: this.isAnimating ? 3 : 1, color: this.isAnimating ? 0x007DFF : 0xCCCCCC })
        .rotate({ angle: this.rotateAngle })
        .shadow({ radius: this.isAnimating ? 16 : 4, color: '#40000000', offsetX: 0, offsetY: 4 })

      Text(`当前角度: ${this.rotateAngle.toFixed(0)}°`)
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
  buildDirectionSection() {
    Column({ space: 12 }) {
      Text('旋转方向')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 20 }) {
        ForEach(this.directions, (name: string, index: number) => {
          Text(name)
            .fontSize(13)
            .padding({ left: 16, right: 16, top: 10, bottom: 10 })
            .borderRadius(20)
            .backgroundColor(this.rotateDirection === index ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.rotateDirection === index ? Color.White : 0x333333)
            .onClick(() => {
              this.rotateDirection = index
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
  buildControlSection() {
    Column({ space: 12 }) {
      Text('控制选项')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 10 }) {
        Button('顺时针90°')
          .fontSize(12)
          .layoutWeight(1)
          .height(40)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.rotateAngle += 90
            })
          })

        Button('逆时针90°')
          .fontSize(12)
          .layoutWeight(1)
          .height(40)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.rotateAngle -= 90
            })
          })

        Button('重置')
          .fontSize(12)
          .layoutWeight(1)
          .height(40)
          .backgroundColor(0xFF6B6B)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.rotateAngle = 0
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
      Text('旋转对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 10 }) {
        Column({ space: 4 }) {
          Image($r('app.media.foreground'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .rotate({ angle: 0 })
          Text('0°')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.foreground'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .rotate({ angle: 45 })
          Text('45°')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.foreground'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .rotate({ angle: 90 })
          Text('90°')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.foreground'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .rotate({ angle: 180 })
          Text('180°')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.foreground'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .rotate({ angle: 270 })
          Text('270°')
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

      Text('1. 使用 animateTo 实现平滑旋转动画')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 正角度为顺时针旋转')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 负角度为逆时针旋转')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 360° 等于完整一圈')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 可组合缩放和旋转实现复杂效果')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
