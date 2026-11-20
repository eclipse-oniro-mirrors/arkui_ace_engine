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
 * Image 缩放手势示例
 * 展示 PinchGesture 缩放手势与图片的交互
 */
@Entry
@Component
struct ImagePinchGestureExample {
  @State scaleValue: number = 1.0
  @State lastScale: number = 1.0
  @State isPinching: boolean = false
  @State pinchLog: string = '等待缩放...'
  @State enablePinch: boolean = true
  @State minScale: number = 0.5
  @State maxScale: number = 3.0

  private logs: string[] = []
  private presets: { name: string, value: number }[] = [
    { name: '50%', value: 0.5 },
    { name: '100%', value: 1.0 },
    { name: '150%', value: 1.5 },
    { name: '200%', value: 2.0 },
    { name: '300%', value: 3.0 }
  ]

  addLog(msg: string) {
    const time = new Date().toLocaleTimeString()
    this.logs.unshift(`[${time}] ${msg}`)
    if (this.logs.length > 5) {
      this.logs.pop()
    }
    this.pinchLog = this.logs.join('\n')
  }

  resetScale() {
    animateTo({ duration: 300, curve: Curve.EaseOut }, () => {
      this.scaleValue = 1.0
      this.lastScale = 1.0
    })
    this.addLog('缩放已重置')
  }

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 缩放手势示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildPinchAreaSection()
        this.buildLogSection()
        this.buildPresetSection()
        this.buildControlSection()
        this.buildScaleComparisonSection()
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
  buildPinchAreaSection() {
    Column({ space: 12 }) {
      Text('缩放区域')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('双指捏合缩放图片')
        .fontSize(12)
        .fontColor(0x999999)

      Stack() {
        Column()
          .width(250)
          .height(250)
          .border({ width: 1, color: this.isPinching ? 0x007DFF : 0xDDDDDD, style: BorderStyle.Dashed })
          .borderRadius(12)
          .backgroundColor(this.isPinching ? 0x08007DFF : 0xFAFAFA)

        Image($r('app.media.background'))
          .width(150)
          .height(150)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)
          .border({ width: this.isPinching ? 3 : 1, color: this.isPinching ? 0x007DFF : 0xCCCCCC })
          .shadow({ radius: this.isPinching ? 16 : 4, color: '#40000000', offsetX: 0, offsetY: this.isPinching ? 8 : 2 })
          .scale({ x: this.scaleValue, y: this.scaleValue })
          .gesture(
            PinchGesture({ fingers: 2, distance: 3 })
              .onActionStart(() => {
                if (!this.enablePinch) {
                  return
                }
                this.isPinching = true
                this.lastScale = this.scaleValue
                this.addLog('缩放开始')
              })
              .onActionUpdate((event: GestureEvent) => {
                if (!this.enablePinch) {
                  return
                }
                const newScale = this.lastScale * event.scale
                this.scaleValue = Math.max(this.minScale, Math.min(this.maxScale, newScale))
              })
              .onActionEnd(() => {
                this.isPinching = false
                this.addLog(`缩放结束: ${this.scaleValue.toFixed(2)}x`)
              })
          )
      }
      .width(250)
      .height(250)
      .alignContent(Alignment.Center)

      Text(`当前缩放: ${this.scaleValue.toFixed(2)}x`)
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
      Text('缩放日志')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Scroll() {
        Text(this.pinchLog)
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
      Text('预设缩放')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.presets, (item: { name: string, value: number }) => {
          Button(item.name)
            .fontSize(12)
            .margin(4)
            .height(36)
            .backgroundColor(this.scaleValue === item.value ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.scaleValue === item.value ? Color.White : 0x333333)
            .onClick(() => {
              animateTo({ duration: 300 }, () => {
                this.scaleValue = item.value
              })
              this.addLog(`设置缩放: ${item.name}`)
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

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.enablePinch })
          .onChange((isOn: boolean) => {
            this.enablePinch = isOn
          })
        Text('启用缩放')
          .fontSize(14)
      }

      Text(`最小缩放: ${this.minScale.toFixed(1)}x`)
        .fontSize(14)
      Slider({
        value: this.minScale,
        min: 0.1,
        max: 1.0,
        step: 0.1
      })
        .width('100%')
        .onChange((value: number) => {
          this.minScale = value
        })

      Text(`最大缩放: ${this.maxScale.toFixed(1)}x`)
        .fontSize(14)
      Slider({
        value: this.maxScale,
        min: 1.0,
        max: 5.0,
        step: 0.5
      })
        .width('100%')
        .onChange((value: number) => {
          this.maxScale = value
        })

      Row({ space: 10 }) {
        Button('放大')
          .fontSize(13)
          .layoutWeight(1)
          .height(40)
          .backgroundColor(0x4CAF50)
          .onClick(() => {
            animateTo({ duration: 200 }, () => {
              this.scaleValue = Math.min(this.maxScale, this.scaleValue * 1.2)
            })
          })

        Button('缩小')
          .fontSize(13)
          .layoutWeight(1)
          .height(40)
          .backgroundColor(0xFF9800)
          .onClick(() => {
            animateTo({ duration: 200 }, () => {
              this.scaleValue = Math.max(this.minScale, this.scaleValue / 1.2)
            })
          })

        Button('重置')
          .fontSize(13)
          .layoutWeight(1)
          .height(40)
          .backgroundColor(0x007DFF)
          .onClick(() => {
            this.resetScale()
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

      Text('1. 双指捏合可缩放图片')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 缩放时图片会显示蓝色边框')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 可通过预设按钮快速设置缩放比例')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 可调整最小/最大缩放限制')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 点击放大/缩小按钮可微调缩放')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildScaleComparisonSection() {
    Column({ space: 12 }) {
      Text('缩放对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('同一图片不同缩放比例对比')
        .fontSize(12)
        .fontColor(0x999999)

      Row({ space: 8 }) {
        Column({ space: 4 }) {
          Image($r('app.media.svg'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .scale({ x: 0.5, y: 0.5 })
          Text('0.5x')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.svg'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .scale({ x: 1.0, y: 1.0 })
          Text('1.0x')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.svg'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .scale({ x: 1.5, y: 1.5 })
          Text('1.5x')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.svg'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .scale({ x: 2.0, y: 2.0 })
          Text('2.0x')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.svg'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .scale({ x: 3.0, y: 3.0 })
          Text('3.0x')
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
