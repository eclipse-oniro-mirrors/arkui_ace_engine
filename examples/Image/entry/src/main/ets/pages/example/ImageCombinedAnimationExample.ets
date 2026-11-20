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
 * Image 组合动画示例
 * 展示多种动画属性组合使用的效果
 */
@Entry
@Component
struct ImageCombinedAnimationExample {
  @State scaleValue: number = 1.0
  @State rotateAngle: number = 0
  @State opacityValue: number = 1.0
  @State offsetX: number = 0
  @State offsetY: number = 0
  @State isAnimating: boolean = false
  @State animationLog: string = '等待动画...'
  @State currentPreset: number = -1

  private logs: string[] = []
  private presets:
    { name: string, scale: number, rotate: number, opacity: number, offsetX: number, offsetY: number }[] = [
    { name: '初始', scale: 1.0, rotate: 0, opacity: 1.0, offsetX: 0, offsetY: 0 },
    { name: '放大旋转', scale: 1.5, rotate: 45, opacity: 1.0, offsetX: 0, offsetY: 0 },
    { name: '缩小淡出', scale: 0.5, rotate: 0, opacity: 0.3, offsetX: 0, offsetY: 0 },
    { name: '右移淡入', scale: 1.0, rotate: 0, opacity: 1.0, offsetX: 80, offsetY: 0 },
    { name: '综合效果', scale: 1.3, rotate: 180, opacity: 0.8, offsetX: 50, offsetY: 30 }
  ]

  addLog(msg: string) {
    const time = new Date().toLocaleTimeString()
    this.logs.unshift(`[${time}] ${msg}`)
    if (this.logs.length > 5) {
      this.logs.pop()
    }
    this.animationLog = this.logs.join('\n')
  }

  applyPreset(index: number) {
    if (this.isAnimating) {
      return
    }
    this.isAnimating = true
    this.currentPreset = index
    this.addLog(`应用预设: ${this.presets[index].name}`)

    animateTo({ duration: 600, curve: Curve.EaseInOut }, () => {
      this.scaleValue = this.presets[index].scale
      this.rotateAngle = this.presets[index].rotate
      this.opacityValue = this.presets[index].opacity
      this.offsetX = this.presets[index].offsetX
      this.offsetY = this.presets[index].offsetY
    })

    setTimeout(() => {
      this.isAnimating = false
      this.addLog('动画结束')
    }, 600)
  }

  resetAll() {
    if (this.isAnimating) {
      return
    }
    this.isAnimating = true
    this.currentPreset = 0
    this.addLog('重置所有属性')

    animateTo({ duration: 400, curve: Curve.EaseOut }, () => {
      this.scaleValue = 1.0
      this.rotateAngle = 0
      this.opacityValue = 1.0
      this.offsetX = 0
      this.offsetY = 0
    })

    setTimeout(() => {
      this.isAnimating = false
    }, 400)
  }

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 组合动画示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildAnimationAreaSection()
        this.buildLogSection()
        this.buildPresetSection()
        this.buildStatsSection()
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
          .width(250)
          .height(250)
          .border({ width: 1, color: 0xDDDDDD, style: BorderStyle.Dashed })
          .borderRadius(12)
          .backgroundColor(0xFAFAFA)

        Image($r('app.media.icon'))
          .width(100)
          .height(100)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)
          .border({ width: this.isAnimating ? 3 : 1, color: this.isAnimating ? 0x007DFF : 0xCCCCCC })
          .scale({ x: this.scaleValue, y: this.scaleValue })
          .rotate({ angle: this.rotateAngle })
          .opacity(this.opacityValue)
          .offset({ x: this.offsetX, y: this.offsetY })
          .shadow({ radius: this.isAnimating ? 20 : 8, color: '#40000000', offsetX: 0, offsetY: 4 })
      }
      .width(250)
      .height(250)
      .alignContent(Alignment.Center)

      if (this.currentPreset >= 0) {
        Text(`当前预设: ${this.presets[this.currentPreset].name}`)
          .fontSize(14)
          .fontColor(0x007DFF)
      }
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
  buildPresetSection() {
    Column({ space: 12 }) {
      Text('预设动画')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.presets, (item: { name: string }, index: number) => {
          Button(item.name)
            .fontSize(12)
            .margin(4)
            .height(40)
            .backgroundColor(this.currentPreset === index ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.currentPreset === index ? Color.White : 0x333333)
            .onClick(() => {
              this.applyPreset(index)
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
  buildStatsSection() {
    Column({ space: 12 }) {
      Text('当前状态')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 12 }) {
        Column() {
          Text(`${this.scaleValue.toFixed(1)}x`)
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .fontColor(0x007DFF)
          Text('缩放')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column() {
          Text(`${this.rotateAngle.toFixed(0)}°`)
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .fontColor(0x4CAF50)
          Text('旋转')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column() {
          Text(`${(this.opacityValue * 100).toFixed(0)}%`)
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .fontColor(0xFF9800)
          Text('透明度')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column() {
          Text(`${this.offsetX.toFixed(0)}`)
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .fontColor(0x9C27B0)
          Text('X偏移')
            .fontSize(10)
            .fontColor(0x666666)
        }
      }
      .justifyContent(FlexAlign.SpaceEvenly)
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

      Button('重置所有')
        .fontSize(14)
        .width('100%')
        .height(44)
        .backgroundColor(0xFF6B6B)
        .onClick(() => {
          this.resetAll()
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

      Text('1. 可同时应用缩放、旋转、透明度、偏移')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 所有属性在同一个 animateTo 中变化')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 预设动画展示不同组合效果')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 综合效果展示多属性同时变化')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 可创建更多自定义预设')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
