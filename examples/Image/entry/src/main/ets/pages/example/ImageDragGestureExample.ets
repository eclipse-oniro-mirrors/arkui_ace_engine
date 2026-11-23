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
 * Image 拖拽手势示例
 * 展示 PanGesture 拖拽手势与图片的交互
 */
@Entry
@Component
struct ImageDragGestureExample {
  @State offsetX: number = 0
  @State offsetY: number = 0
  @State lastOffsetX: number = 0
  @State lastOffsetY: number = 0
  @State isDragging: boolean = false
  @State dragLog: string = '等待拖拽...'
  @State enableDrag: boolean = true
  @State boundaryEnabled: boolean = true
  @State boundaryWidth: number = 250
  @State boundaryHeight: number = 250

  private logs: string[] = []

  addLog(msg: string) {
    const time = new Date().toLocaleTimeString()
    this.logs.unshift(`[${time}] ${msg}`)
    if (this.logs.length > 5) {
      this.logs.pop()
    }
    this.dragLog = this.logs.join('\n')
  }

  resetPosition() {
    animateTo({ duration: 300, curve: Curve.EaseOut }, () => {
      this.offsetX = 0
      this.offsetY = 0
      this.lastOffsetX = 0
      this.lastOffsetY = 0
    })
    this.addLog('位置已重置')
  }

  clampOffset(x: number, y: number): { x: number, y: number } {
    if (!this.boundaryEnabled) {
      return { x, y }
    }
    const maxX = this.boundaryWidth / 2
    const maxY = this.boundaryHeight / 2
    return {
      x: Math.max(-maxX, Math.min(maxX, x)),
      y: Math.max(-maxY, Math.min(maxY, y))
    }
  }

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 拖拽手势示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildDragAreaSection()
        this.buildLogSection()
        this.buildControlSection()
        this.buildDragDirectionSection()
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
  buildDragAreaSection() {
    Column({ space: 12 }) {
      Text('拖拽区域')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Stack() {
        Column()
          .width(this.boundaryWidth)
          .height(this.boundaryHeight)
          .border({ width: 1, color: this.isDragging ? 0x007DFF : 0xDDDDDD, style: BorderStyle.Dashed })
          .borderRadius(12)
          .backgroundColor(this.isDragging ? 0x08007DFF : 0xFAFAFA)

        Image($r('app.media.startIcon'))
          .width(80)
          .height(80)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)
          .border({ width: this.isDragging ? 3 : 1, color: this.isDragging ? 0x007DFF : 0xCCCCCC })
          .shadow({ radius: this.isDragging ? 12 : 4, color: '#40000000', offsetX: 0, offsetY: this.isDragging ? 6 : 2 })
          .scale({ x: this.isDragging ? 1.1 : 1.0, y: this.isDragging ? 1.1 : 1.0 })
          .offset({ x: this.offsetX, y: this.offsetY })
          .gesture(
            PanGesture({ direction: PanDirection.All, distance: 5 })
              .onActionStart(() => {
                if (!this.enableDrag) {
                  return
                }
                this.isDragging = true
                this.lastOffsetX = this.offsetX
                this.lastOffsetY = this.offsetY
                this.addLog('拖拽开始')
              })
              .onActionUpdate((event: GestureEvent) => {
                if (!this.enableDrag) {
                  return
                }
                const newPos = this.clampOffset(
                  this.lastOffsetX + event.offsetX,
                  this.lastOffsetY + event.offsetY
                )
                this.offsetX = newPos.x
                this.offsetY = newPos.y
              })
              .onActionEnd(() => {
                this.isDragging = false
                this.addLog(`拖拽结束: (${this.offsetX.toFixed(0)}, ${this.offsetY.toFixed(0)})`)
              })
          )
      }
      .width(this.boundaryWidth)
      .height(this.boundaryHeight)
      .alignContent(Alignment.Center)

      Text(`位置: (${this.offsetX.toFixed(0)}, ${this.offsetY.toFixed(0)})`)
        .fontSize(12)
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
      Text('拖拽日志')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Scroll() {
        Text(this.dragLog)
          .fontSize(12)
          .fontColor(0x333333)
          .fontFamily('monospace')
      }
      .width('100%')
      .height(100)
      .backgroundColor(0xF8F8F8)
      .borderRadius(8)
      .padding(10)

      Button('清除日志')
        .fontSize(12)
        .height(32)
        .backgroundColor(0xFF6B6B)
        .onClick(() => {
          this.logs = []
          this.dragLog = '日志已清除'
        })
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

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.enableDrag })
          .onChange((isOn: boolean) => {
            this.enableDrag = isOn
          })
        Text('启用拖拽')
          .fontSize(14)
      }

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.boundaryEnabled })
          .onChange((isOn: boolean) => {
            this.boundaryEnabled = isOn
          })
        Text('边界限制')
          .fontSize(14)
      }

      Row({ space: 10 }) {
        Button('重置位置')
          .fontSize(13)
          .layoutWeight(1)
          .height(40)
          .backgroundColor(0x007DFF)
          .onClick(() => {
            this.resetPosition()
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

      Text('1. 按住图片并拖动即可移动位置')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 拖拽时图片会放大并显示阴影')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 启用边界限制后，图片不会超出区域')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 可通过开关禁用拖拽功能')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 点击"重置位置"可回到初始位置')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildDragDirectionSection() {
    Column({ space: 12 }) {
      Text('拖拽方向对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('不同方向限制的拖拽效果')
        .fontSize(12)
        .fontColor(0x999999)

      Row({ space: 16 }) {
        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .gesture(
              PanGesture({ direction: PanDirection.Horizontal })
                .onActionUpdate((event: GestureEvent) => {})
            )
          Text('水平')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .gesture(
              PanGesture({ direction: PanDirection.Vertical })
                .onActionUpdate((event: GestureEvent) => {})
            )
          Text('垂直')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .gesture(
              PanGesture({ direction: PanDirection.Left })
                .onActionUpdate((event: GestureEvent) => {})
            )
          Text('向左')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .gesture(
              PanGesture({ direction: PanDirection.All })
                .onActionUpdate((event: GestureEvent) => {})
            )
          Text('全方向')
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
