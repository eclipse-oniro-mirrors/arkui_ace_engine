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
 * Image 点击手势示例
 * 展示 TapGesture 点击手势与图片的交互
 */
@Entry
@Component
struct ImageTapGestureExample {
  @State tapCount: number = 0
  @State doubleTapCount: number = 0
  @State longPressCount: number = 0
  @State isTapped: boolean = false
  @State isDoubleTapped: boolean = false
  @State isLongPressed: boolean = false
  @State tapLog: string = '等待点击...'
  @State scaleOnDoubleTap: boolean = true
  @State highlightOnTap: boolean = true
  @State vibrateOnLongPress: boolean = true

  private logs: string[] = []

  addLog(msg: string) {
    const time = new Date().toLocaleTimeString()
    this.logs.unshift(`[${time}] ${msg}`)
    if (this.logs.length > 6) {
      this.logs.pop()
    }
    this.tapLog = this.logs.join('\n')
  }

  resetCounters() {
    this.tapCount = 0
    this.doubleTapCount = 0
    this.longPressCount = 0
    this.addLog('计数器已重置')
  }

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 点击手势示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildTapAreaSection()
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
  buildTapAreaSection() {
    Column({ space: 12 }) {
      Text('点击区域')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('单击/双击/长按图片')
        .fontSize(12)
        .fontColor(0x999999)

      Image($r('app.media.heart'))
        .width(150)
        .height(150)
        .objectFit(ImageFit.Cover)
        .borderRadius(12)
        .border({
          width: (this.isTapped || this.isDoubleTapped || this.isLongPressed) ? 4 : 1,
          color: this.isLongPressed ? 0xFF9800 : this.isDoubleTapped ? 0x4CAF50 : this.isTapped ? 0x007DFF : 0xCCCCCC
        })
        .shadow({
          radius: this.isLongPressed ? 20 : this.isDoubleTapped ? 16 : this.isTapped ? 12 : 4,
          color: '#40000000',
          offsetX: 0,
          offsetY: this.isLongPressed ? 10 : this.isDoubleTapped ? 8 : this.isTapped ? 6 : 2
        })
        .scale({
          x: this.isDoubleTapped && this.scaleOnDoubleTap ? 1.2 : 1.0,
          y: this.isDoubleTapped && this.scaleOnDoubleTap ? 1.2 : 1.0
        })
        .opacity(this.isTapped && this.highlightOnTap ? 0.7 : 1.0)
        .gesture(
          GestureGroup(GestureMode.Exclusive,
            TapGesture({ count: 2 })
              .onAction(() => {
                this.doubleTapCount++
                this.isDoubleTapped = true
                this.addLog(`双击: 第 ${this.doubleTapCount} 次`)
                setTimeout(() => {
                  this.isDoubleTapped = false
                }, 300)
              }),

            TapGesture({ count: 1 })
              .onAction(() => {
                this.tapCount++
                this.isTapped = true
                this.addLog(`单击: 第 ${this.tapCount} 次`)
                setTimeout(() => {
                  this.isTapped = false
                }, 200)
              }),

            LongPressGesture({ repeat: false, duration: 500 })
              .onAction(() => {
                this.longPressCount++
                this.isLongPressed = true
                this.addLog(`长按: 第 ${this.longPressCount} 次`)
                setTimeout(() => {
                  this.isLongPressed = false
                }, 500)
              })
          )
        )
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
        Text(this.tapLog)
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
      Text('统计信息')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 20 }) {
        Column() {
          Text(this.tapCount.toString())
            .fontSize(28)
            .fontWeight(FontWeight.Bold)
            .fontColor(0x007DFF)
          Text('单击')
            .fontSize(12)
            .fontColor(0x666666)
        }

        Column() {
          Text(this.doubleTapCount.toString())
            .fontSize(28)
            .fontWeight(FontWeight.Bold)
            .fontColor(0x4CAF50)
          Text('双击')
            .fontSize(12)
            .fontColor(0x666666)
        }

        Column() {
          Text(this.longPressCount.toString())
            .fontSize(28)
            .fontWeight(FontWeight.Bold)
            .fontColor(0xFF9800)
          Text('长按')
            .fontSize(12)
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

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.scaleOnDoubleTap })
          .onChange((isOn: boolean) => {
            this.scaleOnDoubleTap = isOn
          })
        Text('双击放大')
          .fontSize(14)
      }

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.highlightOnTap })
          .onChange((isOn: boolean) => {
            this.highlightOnTap = isOn
          })
        Text('单击高亮')
          .fontSize(14)
      }

      Row({ space: 10 }) {
        Button('重置统计')
          .fontSize(13)
          .layoutWeight(1)
          .height(40)
          .backgroundColor(0xFF6B6B)
          .onClick(() => {
            this.resetCounters()
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

      Text('1. 单击图片触发单击事件（蓝色边框）')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 快速双击触发双击事件（绿色边框）')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 长按500ms触发长按事件（橙色边框）')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 双击时可选择放大效果')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 统计信息实时显示各手势触发次数')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
