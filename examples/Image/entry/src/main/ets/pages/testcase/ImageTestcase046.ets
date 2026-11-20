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


@Entry
@Component
struct ImageAreaSizeChangeTestCase074 {
  @State imageWidth: number = 120
  @State imageHeight: number = 120
  @State currentWidth: number = 120
  @State currentHeight: number = 120
  @State currentArea: number = 14400
  @State areaChangeCount: number = 0
  @State sizeChangeCount: number = 0
  @State lastAreaChange: string = '无'
  @State lastSizeChange: string = '无'
  @State historyLog: string[] = []
  @State animateWidth: boolean = false
  @State aspectLocked: boolean = false

  build() {
    Column() {
      Text('Image onAreaChange/onSizeChange事件测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Column() {
        Image($r('app.media.startIcon'))
          .width(this.imageWidth)
          .height(this.imageHeight)
          .objectFit(ImageFit.Cover)
          .border({ width: 2, color: Color.Blue })
          .borderRadius(8)
          .onAreaChange((oldValue: Area, newValue: Area) => {
            this.areaChangeCount++
            const oldW = Math.round(Number(oldValue.width))
            const oldH = Math.round(Number(oldValue.height))
            const newW = Math.round(Number(newValue.width))
            const newH = Math.round(Number(newValue.height))
            this.currentArea = newW * newH
            this.lastAreaChange = `${oldW}x${oldH} → ${newW}x${newH}`
            this.addLog(`Area#${this.areaChangeCount}: ${oldW}x${oldH}→${newW}x${newH}`)
          })
          .onSizeChange((oldValue: SizeResult, newValue: SizeResult) => {
            this.sizeChangeCount++
            this.currentWidth = Math.round(newValue.width)
            this.currentHeight = Math.round(newValue.height)
            this.lastSizeChange = `${Math.round(oldValue.width)}x${Math.round(oldValue.height)}` +
              ` → ${Math.round(newValue.width)}x${Math.round(newValue.height)}`
          })
      }
      .width('100%')
      .height(200)
      .justifyContent(FlexAlign.Center)
      .backgroundColor('#FAFAFA')
      .borderRadius(8)
      .margin({ bottom: 16 })

      Text(`当前尺寸: ${this.currentWidth}x${this.currentHeight}  面积: ${this.currentArea}`)
        .fontSize(13)
        .fontColor(Color.Blue)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 8 })

      Text(`AreaChange: ${this.lastAreaChange}`)
        .fontSize(11)
        .fontColor(Color.Gray)
        .margin({ bottom: 4 })

      Text(`SizeChange: ${this.lastSizeChange}`)
        .fontSize(11)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })

      Row({ space: 24 }) {
        Column() {
          Text('AreaChange次数')
            .fontSize(10)
            .fontColor(Color.Gray)
          Text(`${this.areaChangeCount}`)
            .fontSize(20)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.Green)
        }
        Column() {
          Text('SizeChange次数')
            .fontSize(10)
            .fontColor(Color.Gray)
          Text(`${this.sizeChangeCount}`)
            .fontSize(20)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.Orange)
        }
        Column() {
          Text('日志条数')
            .fontSize(10)
            .fontColor(Color.Gray)
          Text(`${this.historyLog.length}`)
            .fontSize(20)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.Pink)
        }
      }
      .margin({ bottom: 16 })

      Divider()

      Text('尺寸预设(触发事件)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Grid() {
        ForEach(this.getSizePresets(), (config: { name: string; w: number; h: number }, index: number) => {
          GridItem() {
            Column() {
              Text(config.name)
                .fontSize(9)
                .fontColor(index === this.getCurrentPresetIndex() ? Color.Blue : Color.Gray)
            }
            .padding(8)
            .backgroundColor('#FAFAFA')
            .borderRadius(4)
            .border({ width: 1, color: '#EEEEEE' })
            .onClick(() => {
              this.setSize(config.w, config.h)
            })
          }
        })
      }
      .columnsTemplate('repeat(4, 1fr)')
      .rowsGap(6)
      .columnsGap(6)
      .padding(8)
      .margin({ bottom: 16 })

      Divider()

      Text('动态操作')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 8 }) {
        Button('动画放大')
          .fontSize(10)
          .onClick(() => {
            this.animateToSize(200, 200)
          })
        Button('动画缩小')
          .fontSize(10)
          .onClick(() => {
            this.animateToSize(80, 80)
          })
        Button('脉冲')
          .fontSize(10)
          .onClick(() => {
            this.pulse()
          })
      }
      .margin({ bottom: 12 })

      Row({ space: 8 }) {
        Button(this.aspectLocked ? '锁定比例: 开' : '锁定比例: 关')
          .fontSize(10)
          .onClick(() => { this.aspectLocked = !this.aspectLocked })
          .backgroundColor(this.aspectLocked ? '#4CAF50' : '#EEEEEE')
          .fontColor(this.aspectLocked ? Color.White : Color.Gray)
        Button('随机尺寸')
          .fontSize(10)
          .onClick(() => {
            const w = Math.floor(Math.random() * 200) + 50
            const h = this.aspectLocked ? w : Math.floor(Math.random() * 200) + 50
            this.setSize(w, h)
          })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('事件历史(最多15条)')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Text(this.historyLog.length > 0 ? this.historyLog.join('\n') : '暂无事件记录')
        .fontSize(10)
        .fontColor(Color.Gray)
        .maxLines(8)
        .textOverflow({ overflow: TextOverflow.Ellipsis })
        .lineHeight(14)

      Divider()

      Row({ space: 12 }) {
        Button('清空日志')
          .fontSize(11)
          .backgroundColor('#FF9800')
          .fontColor(Color.White)
          .onClick(() => {
            this.historyLog = []
            this.areaChangeCount = 0
            this.sizeChangeCount = 0
            this.lastAreaChange = '已清空'
            this.lastSizeChange = '已清空'
          })
        Button('重置尺寸')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .fontColor(Color.White)
          .onClick(() => {
            this.setSize(120, 120)
          })
      }
      .margin({ top: 16 })
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }

  private getSizePresets(): Array<{ name: string; w: number; h: number }> {
    return [
      { name: '60x60', w: 60, h: 60 },
      { name: '80x80', w: 80, h: 80 },
      { name: '120x120', w: 120, h: 120 },
      { name: '160x160', w: 160, h: 160 },
      { name: '200x200', w: 200, h: 200 },
      { name: '240x80', w: 240, h: 80 },
      { name: '80x240', w: 80, h: 240 },
      { name: '300x150', w: 300, h: 150 }
    ]
  }

  private getCurrentPresetIndex(): number {
    const presets = this.getSizePresets()
    for (let i = 0; i < presets.length; i++) {
      if (presets[i].w === this.imageWidth && presets[i].h === this.imageHeight) {
        return i
      }
    }
    return -1
  }

  private setSize(w: number, h: number): void {
    this.imageWidth = w
    this.imageHeight = h
  }

  private animateToSize(w: number, h: number): void {
    animateTo({ duration: 500, curve: Curve.EaseInOut }, () => {
      this.imageWidth = w
      this.imageHeight = h
    })
  }

  private pulse(): void {
    animateTo({ duration: 300, curve: Curve.EaseOut }, () => {
      this.imageWidth = 180
      this.imageHeight = 180
    })
    setTimeout(() => {
      animateTo({ duration: 300, curve: Curve.EaseIn }, () => {
        this.imageWidth = 120
        this.imageHeight = 120
      })
    }, 300)
  }

  private addLog(entry: string): void {
    if (this.historyLog.length >= 15) {
      this.historyLog.shift()
    }
    this.historyLog.push(entry)
  }
}
