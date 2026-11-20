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


/** 整合自: ImageTestcase071.ets, ImageTestcase072.ets */

@Entry
@Component
struct ImageGestureTestCase071 {
  @State tapCount: number = 0
  @State doubleTapCount: number = 0
  @State longPressCount: number = 0
  @State panOffsetX: number = 0
  @State panOffsetY: number = 0
  @State pinchScale: number = 1.0
  @State rotationAngle: number = 0
  @State lastGesture: string = '无'
  @State gestureLog: string = '在图片上尝试各种手势操作'

  @State imageTranslateX: number = 0
  @State imageTranslateY: number = 0
  @State imageScale: number = 1.0
  @State imageRotate: number = 0

  build() {
    Column() {
      Text('Image手势交互综合测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Stack() {
        Column()
          .width(200)
          .height(200)
          .backgroundColor('#F5F5F5')
          .borderRadius(12)

        Image($r('app.media.startIcon'))
          .width(160)
          .height(160)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)
          .border({ width: 2, color: Color.Blue })
          .scale({ x: this.imageScale, y: this.imageScale })
          .rotate({ angle: this.imageRotate })
          .translate({ x: this.imageTranslateX, y: this.imageTranslateY })
          .gesture(
            GestureGroup(GestureMode.Parallel,
              TapGesture({ count: 1 })
                .onAction(() => {
                  this.tapCount++
                  this.lastGesture = '单击'
                  this.gestureLog = `单击 #${this.tapCount}`
                }),
              TapGesture({ count: 2 })
                .onAction(() => {
                  this.doubleTapCount++
                  this.lastGesture = '双击'
                  this.gestureLog = `双击 #${this.doubleTapCount}`
                  if (this.imageScale > 1.0) {
                    this.imageScale = 1.0
                  } else {
                    this.imageScale = 1.5
                  }
                }),
              LongPressGesture({ repeatInterval: 500 })
                .onAction(() => {
                  this.longPressCount++
                  this.lastGesture = '长按'
                  this.gestureLog = `长按 #${this.longPressCount}`
                })
                .onActionEnd(() => {
                  this.gestureLog = '长按结束'
                }),
              PanGesture({ fingers: 1 })
                .onActionStart(() => {
                  this.lastGesture = '拖拽开始'
                  this.gestureLog = '拖拽开始'
                })
                .onActionUpdate((event: GestureEvent) => {
                  this.panOffsetX = event.offsetX
                  this.panOffsetY = event.offsetY
                  this.imageTranslateX = event.offsetX
                  this.imageTranslateY = event.offsetY
                  this.lastGesture = '拖拽中'
                  this.gestureLog = `拖拽 offset(${Math.round(event.offsetX)}, ${Math.round(event.offsetY)})`
                })
                .onActionEnd(() => {
                  this.lastGesture = '拖拽结束'
                  this.gestureLog = `拖拽结束 offset(${Math.round(this.panOffsetX)}, ${Math.round(this.panOffsetY)})`
                }),
              PinchGesture({ fingers: 2 })
                .onActionStart(() => {
                  this.lastGesture = '缩放开始'
                  this.gestureLog = '缩放开始'
                })
                .onActionUpdate((event: GestureEvent) => {
                  this.pinchScale = event.scale
                  this.imageScale = Math.max(0.3, Math.min(3.0, event.scale))
                  this.lastGesture = '缩放中'
                  this.gestureLog = `缩放 scale=${event.scale.toFixed(2)}`
                })
                .onActionEnd(() => {
                  this.lastGesture = '缩放结束'
                  this.gestureLog = `缩放结束 scale=${this.pinchScale.toFixed(2)}`
                }),
              RotationGesture({ fingers: 2 })
                .onActionStart(() => {
                  this.lastGesture = '旋转开始'
                  this.gestureLog = '旋转开始'
                })
                .onActionUpdate((event: GestureEvent) => {
                  this.rotationAngle = event.angle
                  this.imageRotate = event.angle
                  this.lastGesture = '旋转中'
                  this.gestureLog = `旋转 angle=${event.angle.toFixed(1)}°`
                })
                .onActionEnd(() => {
                  this.lastGesture = '旋转结束'
                  this.gestureLog = `旋转结束 angle=${this.rotationAngle.toFixed(1)}°`
                })
            )
          )
      }
      .width(200)
      .height(200)
      .alignContent(Alignment.Center)
      .margin({ bottom: 16 })

      Text(this.gestureLog)
        .fontSize(13)
        .fontColor(Color.Blue)
        .margin({ bottom: 12 })

      Divider()

      Text('手势统计')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Grid() {
        GridItem() {
          this.StatCard('单击', this.tapCount, Color.Blue)
        }
        GridItem() {
          this.StatCard('双击', this.doubleTapCount, Color.Green)
        }
        GridItem() {
          this.StatCard('长按', this.longPressCount, Color.Orange)
        }
        GridItem() {
          this.StatCard('缩放', this.pinchScale.toFixed(2), Color.Pink)
        }
        GridItem() {
          this.StatCard('旋转', `${this.rotationAngle.toFixed(0)}°`, Color.Purple)
        }
        GridItem() {
          this.StatCard('最近', this.lastGesture, Color.Gray)
        }
      }
      .columnsTemplate('repeat(3, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(8)
      .margin({ bottom: 16 })

      Divider()

      Text('变换状态')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Text(`translate: (${Math.round(this.imageTranslateX)}, ${Math.round(this.imageTranslateY)})\n` +
        `scale: ${this.imageScale.toFixed(2)}\n` +
        `rotate: ${this.imageRotate.toFixed(1)}°`)
        .fontSize(12)
        .fontColor(Color.Gray)
        .lineHeight(20)
        .margin({ bottom: 16 })

      Divider()

      Text('操作说明')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Text('单击: 计数\n' +
        '双击: 切换缩放(1.0↔1.5)\n' +
        '长按: 计数(500ms触发)\n' +
        '拖拽: 平移图片\n' +
        '双指缩放: 0.3~3.0倍\n' +
        '双指旋转: 旋转图片\n' +
        '手势模式: Parallel(并行识别)')
        .fontSize(11)
        .fontColor(Color.Gray)
        .lineHeight(18)
        .margin({ bottom: 16 })

      Divider()

      Row({ space: 12 }) {
        Button('重置变换')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .fontColor(Color.White)
          .onClick(() => {
            this.imageTranslateX = 0
            this.imageTranslateY = 0
            this.imageScale = 1.0
            this.imageRotate = 0
            this.panOffsetX = 0
            this.panOffsetY = 0
            this.pinchScale = 1.0
            this.rotationAngle = 0
            this.gestureLog = '已重置变换'
            this.lastGesture = '重置'
          })
        Button('重置计数')
          .fontSize(11)
          .backgroundColor('#FF9800')
          .fontColor(Color.White)
          .onClick(() => {
            this.tapCount = 0
            this.doubleTapCount = 0
            this.longPressCount = 0
            this.gestureLog = '已重置计数'
          })
      }
      .margin({ top: 8 })
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }

  @Builder
  StatCard(label: string, value: string, color: ResourceColor) {
    Column() {
      Text(label)
        .fontSize(10)
        .fontColor(Color.Gray)
      Text(value)
        .fontSize(14)
        .fontWeight(FontWeight.Bold)
        .fontColor(color)
    }
    .width('100%')
    .padding(8)
    .backgroundColor('#FAFAFA')
    .borderRadius(6)
    .alignItems(HorizontalAlign.Center)
  }
}

@Component
struct ImageBorderImageTestCase072 {
  @State currentModeIndex: number = 0
  @State currentSliceIndex: number = 0
  @State imageWidth: number = 200
  @State imageHeight: number = 200
  @State borderWidth: number = 30
  @State showBorderImage: boolean = true
  @State showPlainBorder: boolean = true

  private readonly BORDER_MODES: Array<{ mode: BorderImageRepeat; label: string; desc: string }> = [
    { mode: BorderImageRepeat.Stretch, label: 'Stretch(拉伸)', desc: '中间区域拉伸填充' },
    { mode: BorderImageRepeat.Repeat, label: 'Repeat(平铺)', desc: '中间区域平铺重复' },
    { mode: BorderImageRepeat.Round, label: 'Round(圆角)', desc: '中间区域拉伸并圆角' },
    { mode: BorderImageRepeat.Space, label: 'Space(间隔)', desc: '中间区域间隔平铺' }
  ]

  private readonly SLICE_PRESETS: Array<{ name: string; slice: BorderImageSlice; desc: string }> = [
    { name: '小切片(10)', slice: { left: 10, top: 10, right: 10, bottom: 10 }, desc: '四边各10vp切片' },
    { name: '中切片(30)', slice: { left: 30, top: 30, right: 30, bottom: 30 }, desc: '四边各30vp切片' },
    { name: '大切片(60)', slice: { left: 60, top: 60, right: 60, bottom: 60 }, desc: '四边各60vp切片' },
    { name: '非对称', slice: { left: 40, top: 20, right: 40, bottom: 20 }, desc: '左右40,上下20' },
    { name: '极小(5)', slice: { left: 5, top: 5, right: 5, bottom: 5 }, desc: '四边各5vp' },
    { name: '极大(80)', slice: { left: 80, top: 80, right: 80, bottom: 80 }, desc: '四边各80vp' }
  ]

  build() {
    Column() {
      Text('Image borderImage边框图片测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        if (this.showPlainBorder) {
          Column() {
            Text('普通border')
              .fontSize(11)
              .fontColor(Color.Gray)
              .margin({ bottom: 8 })
            Image($r('app.media.startIcon'))
              .width(this.imageWidth)
              .height(this.imageHeight)
              .objectFit(ImageFit.Cover)
              .border({ width: this.borderWidth, color: '#4CAF50' })
              .borderRadius(8)
          }
          .margin({ right: 16 })
        }

        if (this.showBorderImage) {
          Column() {
            Text(`borderImage: ${this.BORDER_MODES[this.currentModeIndex].label}`)
              .fontSize(11)
              .fontColor(Color.Blue)
              .margin({ bottom: 8 })
            Image($r('app.media.startIcon'))
              .width(this.imageWidth)
              .height(this.imageHeight)
              .objectFit(ImageFit.Cover)
              .borderImage({
                src: $r('app.media.background'),
                slice: this.SLICE_PRESETS[this.currentSliceIndex].slice,
                width: this.borderWidth,
                repeat: this.BORDER_MODES[this.currentModeIndex].mode,
                fill: false
              })
              .borderRadius(8)
          }
        }
      }
      .margin({ bottom: 16 })

      Text(`模式: ${this.BORDER_MODES[this.currentModeIndex].label} - ${this.BORDER_MODES[this.currentModeIndex].desc}`)
        .fontSize(12)
        .fontColor(Color.Gray)
        .margin({ bottom: 4 })

      Text(`切片: ${this.SLICE_PRESETS[this.currentSliceIndex].name}` +
        ` - ${this.SLICE_PRESETS[this.currentSliceIndex].desc}`)
        .fontSize(12)
        .fontColor(Color.Gray)
        .margin({ bottom: 4 })

      Text(`边框宽度: ${this.borderWidth}vp  显示尺寸: ${this.imageWidth}x${this.imageHeight}`)
        .fontSize(12)
        .fontColor(Color.Orange)
        .margin({ bottom: 16 })

      Divider()

      Text('BorderImageRepeat模式')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row() {
        ForEach(this.BORDER_MODES, (config: { mode: BorderImageRepeat; label: string; desc: string },
            index: number) => {
          Button(config.label)
            .fontSize(9)
            .onClick(() => { this.currentModeIndex = index })
            .backgroundColor(index === this.currentModeIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentModeIndex ? Color.White : Color.Gray)
            .margin({ left: 3, right: 3 })
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('切片预设')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row() {
        ForEach(this.SLICE_PRESETS, (config: { name: string; slice: BorderImageSlice; desc: string },
            index: number) => {
          Button(config.name)
            .fontSize(9)
            .onClick(() => { this.currentSliceIndex = index })
            .backgroundColor(index === this.currentSliceIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentSliceIndex ? Color.White : Color.Gray)
            .margin({ left: 3, right: 3 })
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('边框宽度')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 8 }) {
        Button('10')
          .fontSize(10)
          .onClick(() => { this.borderWidth = 10 })
          .backgroundColor(this.borderWidth === 10 ? '#2196F3' : '#EEEEEE')
          .fontColor(this.borderWidth === 10 ? Color.White : Color.Gray)
        Button('30')
          .fontSize(10)
          .onClick(() => { this.borderWidth = 30 })
          .backgroundColor(this.borderWidth === 30 ? '#2196F3' : '#EEEEEE')
          .fontColor(this.borderWidth === 30 ? Color.White : Color.Gray)
        Button('50')
          .fontSize(10)
          .onClick(() => { this.borderWidth = 50 })
          .backgroundColor(this.borderWidth === 50 ? '#2196F3' : '#EEEEEE')
          .fontColor(this.borderWidth === 50 ? Color.White : Color.Gray)
        Button('80')
          .fontSize(10)
          .onClick(() => { this.borderWidth = 80 })
          .backgroundColor(this.borderWidth === 80 ? '#2196F3' : '#EEEEEE')
          .fontColor(this.borderWidth === 80 ? Color.White : Color.Gray)
      }
      .margin({ bottom: 16 })

      Divider()

      Text('显示尺寸(观察边框拉伸)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 8 }) {
        Button('120x120')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 120; this.imageHeight = 120 })
        Button('200x200')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 200; this.imageHeight = 200 })
        Button('300x120')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 300; this.imageHeight = 120 })
        Button('120x300')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 120; this.imageHeight = 300 })
      }
      .margin({ bottom: 16 })

      Divider()

      Row({ space: 12 }) {
        Button(this.showPlainBorder ? '普通border: 显' : '普通border: 隐')
          .fontSize(10)
          .onClick(() => { this.showPlainBorder = !this.showPlainBorder })
          .backgroundColor(this.showPlainBorder ? '#4CAF50' : '#EEEEEE')
          .fontColor(this.showPlainBorder ? Color.White : Color.Gray)
        Button(this.showBorderImage ? 'borderImage: 显' : 'borderImage: 隐')
          .fontSize(10)
          .onClick(() => { this.showBorderImage = !this.showBorderImage })
          .backgroundColor(this.showBorderImage ? '#4CAF50' : '#EEEEEE')
          .fontColor(this.showBorderImage ? Color.White : Color.Gray)
        Button('重置')
          .fontSize(10)
          .backgroundColor('#FF6666')
          .fontColor(Color.White)
          .onClick(() => {
            this.currentModeIndex = 0
            this.currentSliceIndex = 1
            this.imageWidth = 200
            this.imageHeight = 200
            this.borderWidth = 30
            this.showPlainBorder = true
            this.showBorderImage = true
          })
      }
      .margin({ top: 8 })
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}
