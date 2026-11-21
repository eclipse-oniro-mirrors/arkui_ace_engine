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
 * Image 手势交互示例
 * 展示拖拽、缩放、旋转、长按等手势与图片的交互效果
 */
@Entry
@Component
struct ImageGestureInteractionExample {
  @State offsetX: number = 0
  @State offsetY: number = 0
  @State scaleValue: number = 1.0
  @State rotateAngle: number = 0
  @State lastOffsetX: number = 0
  @State lastOffsetY: number = 0
  @State lastScale: number = 1.0
  @State lastRotate: number = 0
  @State gestureLog: string = '等待手势事件...'
  @State isDragging: boolean = false
  @State isLongPress: boolean = false
  @State showGrid: boolean = false
  @State imageWidth: number = 200
  @State imageHeight: number = 200
  @State enablePan: boolean = true
  @State enablePinch: boolean = true
  @State enableRotation: boolean = true
  @State enableTap: boolean = true
  @State enableLongPress: boolean = true
  @State tapCount: number = 0
  @State doubleTapScale: number = 1.0

  private logs: string[] = []

  addLog(msg: string) {
    const time = new Date().toLocaleTimeString()
    this.logs.unshift(`[${time}] ${msg}`)
    if (this.logs.length > 8) {
      this.logs.pop()
    }
    this.gestureLog = this.logs.join('\n')
  }

  resetTransform() {
    animateTo({ duration: 300, curve: Curve.EaseOut }, () => {
      this.offsetX = 0
      this.offsetY = 0
      this.scaleValue = 1.0
      this.rotateAngle = 0
      this.lastOffsetX = 0
      this.lastOffsetY = 0
      this.lastScale = 1.0
      this.lastRotate = 0
    })
    this.addLog('已重置变换')
  }

  build() {
    Column() {
      Text('Image 手势交互示例')
        .fontSize(24)
        .fontWeight(FontWeight.Bold)
        .margin({ top: 20, bottom: 16 })

      Scroll() {
        Column({ space: 16 }) {
          this.buildGestureArea()
          this.buildLogSection()
          this.buildControlSection()
          this.buildToggleSection()
          this.buildActionSection()
        }
        .width('100%')
        .padding({ left: 16, right: 16, bottom: 30 })
      }
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor(0xF5F5F5)
  }

  @Builder
  buildGestureArea() {
    Column({ space: 10 }) {
      Text('手势交互区域')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Stack() {
        if (this.showGrid) {
          this.buildGridBackground()
        }

        Image($r('app.media.startIcon'))
          .width(this.imageWidth)
          .height(this.imageHeight)
          .objectFit(ImageFit.Contain)
          .borderRadius(12)
          .border({ width: this.isDragging ? 3 : 1, color: this.isDragging ? 0x007DFF : 0xDDDDDD })
          .scale({ x: this.scaleValue, y: this.scaleValue })
          .rotate({ angle: this.rotateAngle })
          .offset({ x: this.offsetX, y: this.offsetY })
          .opacity(this.isLongPress ? 0.7 : 1.0)
          .gesture(
            GestureGroup(GestureMode.Parallel,
              PanGesture({ direction: PanDirection.All, distance: 5 })
                .onActionStart(() => {
                  if (!this.enablePan) {
                    return
                  }
                  this.isDragging = true
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
                  this.isDragging = false
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
                  if (!this.enableTap) {
                    return
                  }
                  this.tapCount++
                  this.addLog(`单击: 第 ${this.tapCount} 次`)
                }),

              TapGesture({ count: 2 })
                .onAction(() => {
                  if (!this.enableTap) {
                    return
                  }
                  animateTo({ duration: 200 }, () => {
                    if (this.scaleValue > 1.0) {
                      this.scaleValue = 1.0
                    } else {
                      this.scaleValue = 2.0
                    }
                  })
                  this.addLog(`双击: 缩放至 ${this.scaleValue.toFixed(1)}x`)
                }),

              LongPressGesture({ repeat: false, duration: 500 })
                .onAction(() => {
                  if (!this.enableLongPress) {
                    return
                  }
                  this.isLongPress = !this.isLongPress
                  this.addLog(`长按: ${this.isLongPress ? '激活' : '取消'}`)
                })
            )
          )
      }
      .width('100%')
      .height(300)
      .backgroundColor(0xFAFAFA)
      .borderRadius(12)
      .alignContent(Alignment.Center)
      .clip(true)

      Text(`缩放: ${this.scaleValue.toFixed(2)}x | 旋转: ${this.rotateAngle.toFixed(0)}°`)
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildGridBackground() {
    Column() {
      ForEach(Array.from({ length: 15 }), (_: number, rowIndex: number) => {
        Row() {
          ForEach(Array.from({ length: 10 }), (_: number, colIndex: number) => {
            Column()
              .width(30)
              .height(20)
              .border({ width: 0.5, color: 0x20000000 })
              .backgroundColor((rowIndex + colIndex) % 2 === 0 ? 0x08000000 : Color.Transparent)
          })
        }
      })
    }
    .width('100%')
    .height('100%')
  }

  @Builder
  buildLogSection() {
    Column({ space: 10 }) {
      Text('手势日志')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

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
  buildControlSection() {
    Column({ space: 12 }) {
      Text('尺寸控制')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Text(`宽度: ${this.imageWidth}`)
        .fontSize(14)
      Slider({
        value: this.imageWidth,
        min: 100,
        max: 300,
        step: 10
      })
        .width('100%')
        .onChange((value: number) => {
          this.imageWidth = value
        })

      Text(`高度: ${this.imageHeight}`)
        .fontSize(14)
      Slider({
        value: this.imageHeight,
        min: 100,
        max: 300,
        step: 10
      })
        .width('100%')
        .onChange((value: number) => {
          this.imageHeight = value
        })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildToggleSection() {
    Column({ space: 12 }) {
      Text('手势开关')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      this.buildToggleRow('拖拽', this.enablePan, (val: boolean) => { this.enablePan = val })
      this.buildToggleRow('缩放', this.enablePinch, (val: boolean) => { this.enablePinch = val })
      this.buildToggleRow('旋转', this.enableRotation, (val: boolean) => { this.enableRotation = val })
      this.buildToggleRow('点击', this.enableTap, (val: boolean) => { this.enableTap = val })
      this.buildToggleRow('长按', this.enableLongPress, (val: boolean) => { this.enableLongPress = val })
      this.buildToggleRow('网格背景', this.showGrid, (val: boolean) => { this.showGrid = val })
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildToggleRow(label: string, isOn: boolean, onChange: (val: boolean) => void) {
    Row() {
      Toggle({ type: ToggleType.Switch, isOn: isOn })
        .onChange(onChange)
      Text(label)
        .fontSize(14)
        .margin({ left: 12 })
    }
    .width('100%')
    .justifyContent(FlexAlign.Start)
  }

  @Builder
  buildActionSection() {
    Column({ space: 10 }) {
      Text('操作')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Row({ space: 10 }) {
        Button('重置变换')
          .layoutWeight(1)
          .height(40)
          .fontSize(14)
          .backgroundColor(0x007DFF)
          .onClick(() => {
            this.resetTransform()
          })

        Button('清除日志')
          .layoutWeight(1)
          .height(40)
          .fontSize(14)
          .backgroundColor(0xFF6B6B)
          .onClick(() => {
            this.logs = []
            this.gestureLog = '日志已清除'
          })
      }

      Row({ space: 10 }) {
        Button('放大 1.5x')
          .layoutWeight(1)
          .height(40)
          .fontSize(14)
          .backgroundColor(0x4CAF50)
          .onClick(() => {
            animateTo({ duration: 200 }, () => {
              this.scaleValue = Math.min(3.0, this.scaleValue * 1.5)
            })
          })

        Button('缩小 0.7x')
          .layoutWeight(1)
          .height(40)
          .fontSize(14)
          .backgroundColor(0xFF9800)
          .onClick(() => {
            animateTo({ duration: 200 }, () => {
              this.scaleValue = Math.max(0.3, this.scaleValue * 0.7)
            })
          })
      }

      Row({ space: 10 }) {
        Button('旋转 90°')
          .layoutWeight(1)
          .height(40)
          .fontSize(14)
          .backgroundColor(0x9C27B0)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.rotateAngle += 90
            })
          })

        Button('居中')
          .layoutWeight(1)
          .height(40)
          .fontSize(14)
          .backgroundColor(0x607D8B)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.offsetX = 0
              this.offsetY = 0
            })
          })
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
