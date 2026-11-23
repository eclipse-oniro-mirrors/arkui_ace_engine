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
 * Image 旋转手势示例
 * 展示 RotationGesture 旋转手势与图片的交互
 */
@Entry
@Component
struct ImageRotationGestureExample {
  @State rotateAngle: number = 0
  @State lastRotate: number = 0
  @State isRotating: boolean = false
  @State rotationLog: string = '等待旋转...'
  @State enableRotation: boolean = true

  private logs: string[] = []
  private presets: { name: string, value: number }[] = [
    { name: '0°', value: 0 },
    { name: '90°', value: 90 },
    { name: '180°', value: 180 },
    { name: '270°', value: 270 },
    { name: '360°', value: 360 }
  ]

  addLog(msg: string) {
    const time = new Date().toLocaleTimeString()
    this.logs.unshift(`[${time}] ${msg}`)
    if (this.logs.length > 5) {
      this.logs.pop()
    }
    this.rotationLog = this.logs.join('\n')
  }

  resetRotation() {
    animateTo({ duration: 300, curve: Curve.EaseOut }, () => {
      this.rotateAngle = 0
      this.lastRotate = 0
    })
    this.addLog('旋转已重置')
  }

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 旋转手势示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildRotationAreaSection()
        this.buildLogSection()
        this.buildPresetSection()
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
  buildRotationAreaSection() {
    Column({ space: 12 }) {
      Text('旋转区域')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('双指旋转图片')
        .fontSize(12)
        .fontColor(0x999999)

      Stack() {
        Column()
          .width(200)
          .height(200)
          .border({ width: 1, color: this.isRotating ? 0x007DFF : 0xDDDDDD, style: BorderStyle.Dashed })
          .borderRadius(100)
          .backgroundColor(this.isRotating ? 0x08007DFF : 0xFAFAFA)

        Image($r('app.media.foreground'))
          .width(120)
          .height(120)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)
          .border({ width: this.isRotating ? 3 : 1, color: this.isRotating ? 0x007DFF : 0xCCCCCC })
          .shadow({ radius: this.isRotating ? 16 : 4, color: '#40000000', offsetX: 0, offsetY: this.isRotating ? 8 : 2 })
          .rotate({ angle: this.rotateAngle })
          .gesture(
            RotationGesture({ fingers: 2 })
              .onActionStart(() => {
                if (!this.enableRotation) {
                  return
                }
                this.isRotating = true
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
                this.isRotating = false
                this.addLog(`旋转结束: ${this.rotateAngle.toFixed(0)}°`)
              })
          )
      }
      .width(200)
      .height(200)
      .alignContent(Alignment.Center)

      Text(`当前角度: ${this.rotateAngle.toFixed(0)}°`)
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
      Text('旋转日志')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Scroll() {
        Text(this.rotationLog)
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
      Text('预设角度')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.presets, (item: { name: string, value: number }) => {
          Button(item.name)
            .fontSize(12)
            .margin(4)
            .height(36)
            .backgroundColor(this.rotateAngle === item.value ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.rotateAngle === item.value ? Color.White : 0x333333)
            .onClick(() => {
              animateTo({ duration: 300 }, () => {
                this.rotateAngle = item.value
              })
              this.addLog(`设置角度: ${item.name}`)
            })
        })
      }

      Row({ space: 10 }) {
        Button('顺时针90°')
          .fontSize(12)
          .layoutWeight(1)
          .height(36)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.rotateAngle += 90
            })
          })

        Button('逆时针90°')
          .fontSize(12)
          .layoutWeight(1)
          .height(36)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.rotateAngle -= 90
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
        Toggle({ type: ToggleType.Switch, isOn: this.enableRotation })
          .onChange((isOn: boolean) => {
            this.enableRotation = isOn
          })
        Text('启用旋转')
          .fontSize(14)
      }

      Text(`角度: ${this.rotateAngle.toFixed(0)}°`)
        .fontSize(14)
      Slider({
        value: this.rotateAngle,
        min: -360,
        max: 360,
        step: 5
      })
        .width('100%')
        .onChange((value: number) => {
          this.rotateAngle = value
        })

      Row({ space: 10 }) {
        Button('重置')
          .fontSize(13)
          .layoutWeight(1)
          .height(40)
          .backgroundColor(0x007DFF)
          .onClick(() => {
            this.resetRotation()
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

      Text('1. 双指旋转可旋转图片')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 旋转时图片会显示蓝色边框')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 可通过预设按钮快速设置角度')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 可通过滑块精确调整角度')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 支持负角度（逆时针旋转）')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
