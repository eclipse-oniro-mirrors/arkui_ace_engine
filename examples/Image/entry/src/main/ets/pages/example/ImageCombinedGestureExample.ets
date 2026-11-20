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
 * Image 组合手势示例
 * 展示多种手势组合使用的效果
 */
@Entry
@Component
struct ImageCombinedGestureExample {
  @State offsetX: number = 0
  @State offsetY: number = 0
  @State scaleValue: number = 1.0
  @State rotateAngle: number = 0
  @State lastOffsetX: number = 0
  @State lastOffsetY: number = 0
  @State lastScale: number = 1.0
  @State lastRotate: number = 0
  @State gestureLog: string = '等待手势...'
  @State enablePan: boolean = true
  @State enablePinch: boolean = true
  @State enableRotation: boolean = true
  @State tapCount: number = 0

  private logs: string[] = []

  addLog(msg: string) {
    const time = new Date().toLocaleTimeString()
    this.logs.unshift(`[${time}] ${msg}`)
    if (this.logs.length > 6) {
      this.logs.pop()
    }
    this.gestureLog = this.logs.join('\n')
  }

  resetAll() {
    animateTo({ duration: 400, curve: Curve.EaseOut }, () => {
      this.offsetX = 0
      this.offsetY = 0
      this.scaleValue = 1.0
      this.rotateAngle = 0
      this.lastOffsetX = 0
      this.lastOffsetY = 0
      this.lastScale = 1.0
      this.lastRotate = 0
    })
    this.addLog('已重置所有变换')
  }

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 组合手势示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildGestureAreaSection()
        this.buildLogSection()
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
  buildGestureAreaSection() {
    Column({ space: 12 }) {
      Text('手势区域')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('支持拖拽、缩放、旋转、点击')
        .fontSize(12)
        .fontColor(0x999999)

      Stack() {
        Column()
          .width(280)
          .height(280)
          .border({ width: 1, color: 0xDDDDDD, style: BorderStyle.Dashed })
          .borderRadius(12)
          .backgroundColor(0xFAFAFA)

        Image($r('app.media.star'))
          .width(120)
          .height(120)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)
          .border({ width: 2, color: 0xCCCCCC })
          .shadow({ radius: 8, color: '#40000000', offsetX: 0, offsetY: 4 })
          .scale({ x: this.scaleValue, y: this.scaleValue })
          .rotate({ angle: this.rotateAngle })
          .offset({ x: this.offsetX, y: this.offsetY })
          .gesture(
            GestureGroup(GestureMode.Parallel,
              PanGesture({ direction: PanDirection.All, distance: 5 })
                .onActionStart(() => {
                  if (!this.enablePan) {
                    return
                  }
                  this.lastOffsetX = this.offsetX
                  this.lastOffsetY = this.offsetY
                  this.addLog('拖拽开始')
                })
                .onActionUpdate((event: GestureEvent) => {
                  if (!this.enablePan) {
                    return
                  }
                  this.offsetX = this.lastOffsetX + event.offsetX
                  this.offsetY = this.lastOffsetY + event.offsetY
                })
                .onActionEnd(() => {
                  this.addLog(`拖拽结束: (${this.offsetX.toFixed(0)}, ${this.offsetY.toFixed(0)})`)
                }),

              PinchGesture({ fingers: 2, distance: 3 })
                .onActionStart(() => {
                  if (!this.enablePinch) {
                    return
                  }
                  this.lastScale = this.scaleValue
                  this.addLog('缩放开始')
                })
                .onActionUpdate((event: GestureEvent) => {
                  if (!this.enablePinch) {
                    return
                  }
                  const newScale = this.lastScale * event.scale
                  this.scaleValue = Math.max(0.3, Math.min(3.0, newScale))
                })
                .onActionEnd(() => {
                  this.addLog(`缩放结束: ${this.scaleValue.toFixed(2)}x`)
                }),

              RotationGesture({ fingers: 2 })
                .onActionStart(() => {
                  if (!this.enableRotation) {
                    return
                  }
                  this.lastRotate = this.rotateAngle
                  this.addLog('旋转开始')
                })
                .onActionUpdate((event: GestureEvent) => {
                  if (!this.enableRotation) {
                    return
                  }
                  this.rotateAngle = this.lastRotate + event.angle
                })
                .onActionEnd(() => {
                  this.addLog(`旋转结束: ${this.rotateAngle.toFixed(0)}°`)
                }),

              TapGesture({ count: 1 })
                .onAction(() => {
                  this.tapCount++
                  this.addLog(`单击: 第 ${this.tapCount} 次`)
                }),

              TapGesture({ count: 2 })
                .onAction(() => {
                  animateTo({ duration: 200 }, () => {
                    if (this.scaleValue > 1.0) {
                      this.scaleValue = 1.0
                    } else {
                      this.scaleValue = 2.0
                    }
                  })
                  this.addLog(`双击: 缩放至 ${this.scaleValue.toFixed(1)}x`)
                })
            )
          )
      }
      .width(280)
      .height(280)
      .alignContent(Alignment.Center)

      Text(`位置: (${this.offsetX.toFixed(0)}, ${this.offsetY.toFixed(0)})`)
        .fontSize(11)
        .fontColor(0x666666)
      Text(`缩放: ${this.scaleValue.toFixed(2)}x | 旋转: ${this.rotateAngle.toFixed(0)}°`)
        .fontSize(11)
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
      Text('手势日志')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Scroll() {
        Text(this.gestureLog)
          .fontSize(12)
          .fontColor(0x333333)
          .fontFamily('monospace')
      }
      .width('100%')
      .height(120)
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
  buildStatsSection() {
    Column({ space: 12 }) {
      Text('当前状态')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 16 }) {
        Column() {
          Text(`${this.offsetX.toFixed(0)}`)
            .fontSize(20)
            .fontWeight(FontWeight.Bold)
            .fontColor(0x007DFF)
          Text('X 偏移')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column() {
          Text(`${this.offsetY.toFixed(0)}`)
            .fontSize(20)
            .fontWeight(FontWeight.Bold)
            .fontColor(0x007DFF)
          Text('Y 偏移')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column() {
          Text(`${this.scaleValue.toFixed(1)}x`)
            .fontSize(20)
            .fontWeight(FontWeight.Bold)
            .fontColor(0x4CAF50)
          Text('缩放')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column() {
          Text(`${this.rotateAngle.toFixed(0)}°`)
            .fontSize(20)
            .fontWeight(FontWeight.Bold)
            .fontColor(0xFF9800)
          Text('旋转')
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
      Text('手势开关')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.enablePan })
          .onChange((isOn: boolean) => {
            this.enablePan = isOn
          })
        Text('拖拽')
          .fontSize(14)
      }

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.enablePinch })
          .onChange((isOn: boolean) => {
            this.enablePinch = isOn
          })
        Text('缩放')
          .fontSize(14)
      }

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.enableRotation })
          .onChange((isOn: boolean) => {
            this.enableRotation = isOn
          })
        Text('旋转')
          .fontSize(14)
      }

      Row({ space: 10 }) {
        Button('重置')
          .fontSize(13)
          .layoutWeight(1)
          .height(40)
          .backgroundColor(0x007DFF)
          .onClick(() => {
            this.resetAll()
          })

        Button('清除日志')
          .fontSize(13)
          .layoutWeight(1)
          .height(40)
          .backgroundColor(0xFF6B6B)
          .onClick(() => {
            this.logs = []
            this.gestureLog = '日志已清除'
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

      Text('1. 单指拖拽可移动图片位置')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 双指捏合可缩放图片')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 双指旋转可旋转图片')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 单击计数，双击切换缩放')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 可通过开关单独禁用某种手势')
        .fontSize(12)
        .fontColor(0x666666)
      Text('6. 所有手势可同时使用（并行模式）')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
