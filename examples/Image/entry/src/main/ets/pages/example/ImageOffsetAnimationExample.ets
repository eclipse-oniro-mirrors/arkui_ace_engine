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
 * Image 位移动画示例
 * 展示 offset 属性的动画效果
 */
@Entry
@Component
struct ImageOffsetAnimationExample {
  @State offsetX: number = 0
  @State offsetY: number = 0
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

  startAnimation(pattern: number) {
    if (this.isAnimating) {
      return
    }
    this.isAnimating = true
    this.addLog('动画开始')

    const curves: Curve[] = [Curve.Linear, Curve.EaseIn, Curve.EaseOut, Curve.EaseInOut, Curve.FastOutSlowIn]
    const curve = curves[this.animationType]

    switch (pattern) {
      case 0:
        animateTo({ duration: 500, curve: curve }, () => {
          this.offsetX = 80
        })
        setTimeout(() => {
          animateTo({ duration: 500, curve: curve }, () => {
            this.offsetX = 0
          })
        }, 500)
        break
      case 1:
        animateTo({ duration: 500, curve: curve }, () => {
          this.offsetY = 80
        })
        setTimeout(() => {
          animateTo({ duration: 500, curve: curve }, () => {
            this.offsetY = 0
          })
        }, 500)
        break
      case 2:
        animateTo({ duration: 500, curve: curve }, () => {
          this.offsetX = 60
          this.offsetY = 60
        })
        setTimeout(() => {
          animateTo({ duration: 500, curve: curve }, () => {
            this.offsetX = -60
            this.offsetY = -60
          })
        }, 500)
        setTimeout(() => {
          animateTo({ duration: 500, curve: curve }, () => {
            this.offsetX = 0
            this.offsetY = 0
          })
        }, 1000)
        break
      case 3:
        animateTo({ duration: 300, curve: curve }, () => {
          this.offsetX = 20
        })
        setTimeout(() => {
          animateTo({ duration: 300, curve: curve }, () => {
            this.offsetX = -20
          })
        }, 300)
        setTimeout(() => {
          animateTo({ duration: 300, curve: curve }, () => {
            this.offsetX = 10
          })
        }, 600)
        setTimeout(() => {
          animateTo({ duration: 300, curve: curve }, () => {
            this.offsetX = 0
          })
        }, 900)
        break
    }

    setTimeout(() => {
      this.isAnimating = false
      this.addLog('动画结束')
    }, 1500)
  }

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 位移动画示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildAnimationAreaSection()
        this.buildLogSection()
        this.buildAnimationTypeSection()
        this.buildPatternSection()
        this.buildControlSection()
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
          .width(200)
          .height(200)
          .border({ width: 1, color: 0xDDDDDD, style: BorderStyle.Dashed })
          .borderRadius(12)
          .backgroundColor(0xFAFAFA)

        Image($r('app.media.star'))
          .width(80)
          .height(80)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)
          .border({ width: this.isAnimating ? 3 : 1, color: this.isAnimating ? 0x007DFF : 0xCCCCCC })
          .offset({ x: this.offsetX, y: this.offsetY })
          .shadow({ radius: 8, color: '#40000000', offsetX: 0, offsetY: 4 })
      }
      .width(200)
      .height(200)
      .alignContent(Alignment.Center)

      Text(`偏移: (${this.offsetX.toFixed(0)}, ${this.offsetY.toFixed(0)})`)
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
  buildPatternSection() {
    Column({ space: 12 }) {
      Text('动画模式')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        Button('左右移动')
          .fontSize(12)
          .margin(4)
          .height(40)
          .onClick(() => {
            this.startAnimation(0)
          })

        Button('上下移动')
          .fontSize(12)
          .margin(4)
          .height(40)
          .onClick(() => {
            this.startAnimation(1)
          })

        Button('对角移动')
          .fontSize(12)
          .margin(4)
          .height(40)
          .onClick(() => {
            this.startAnimation(2)
          })

        Button('抖动效果')
          .fontSize(12)
          .margin(4)
          .height(40)
          .onClick(() => {
            this.startAnimation(3)
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

      Row({ space: 10 }) {
        Button('向左')
          .fontSize(12)
          .layoutWeight(1)
          .height(40)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.offsetX -= 30
            })
          })

        Button('向右')
          .fontSize(12)
          .layoutWeight(1)
          .height(40)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.offsetX += 30
            })
          })

        Button('向上')
          .fontSize(12)
          .layoutWeight(1)
          .height(40)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.offsetY -= 30
            })
          })

        Button('向下')
          .fontSize(12)
          .layoutWeight(1)
          .height(40)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.offsetY += 30
            })
          })
      }

      Button('重置位置')
        .fontSize(13)
        .width('100%')
        .height(40)
        .backgroundColor(0xFF6B6B)
        .onClick(() => {
          animateTo({ duration: 300 }, () => {
            this.offsetX = 0
            this.offsetY = 0
          })
        })
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

      Text('1. offset 用于偏移图片位置')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 正值向右/下，负值向左/上')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 使用 animateTo 实现平滑移动')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 可组合多个动画实现复杂效果')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 抖动效果可用于提示用户')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
