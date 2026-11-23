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


/** 整合自: ImageTestcase034.ets, ImageTestcase061.ets, ImageTestcase062.ets */

@Entry
@Component
struct ImageRenderModeTestCase034 {
  @State currentModeIndex: number = 0
  @State currentFillColor: ResourceColor = Color.Red

  private readonly RENDER_MODES: Array<{ mode: ImageRenderMode; label: string }> = [
    { mode: ImageRenderMode.Original, label: 'Original(原图)' },
    { mode: ImageRenderMode.Template, label: 'Template(模板)' }
  ]

  private readonly FILL_COLORS: ResourceColor[] = [
    Color.Red, Color.Green, Color.Blue, Color.Yellow, Color.Pink, undefined
  ]

  build() {
    Column() {
      Text('Image renderMode属性测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        ForEach(this.RENDER_MODES, (config: { mode: ImageRenderMode; label: string }) => {
          Column() {
            Text(config.label)
              .fontSize(14)
              .margin({ bottom: 8 })
            Image($r('app.media.startIcon'))
              .width(120)
              .height(120)
              .renderMode(config.mode)
              .fillColor(this.currentFillColor)
              .border({ width: 2, color: Color.Blue })
              .borderRadius(8)
          }
          .margin({ left: 8, right: 8 })
        })
      }
      .margin({ bottom: 16 })

      Text(`当前模式: ${this.RENDER_MODES[this.currentModeIndex].label}`)
        .fontSize(14)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })

      Row({ space: 12 }) {
        ForEach(this.RENDER_MODES, (config: { mode: ImageRenderMode; label: string }, index: number) => {
          Button(config.label)
            .onClick(() => {
              this.currentModeIndex = index
            })
            .backgroundColor(index === this.currentModeIndex ? '#2196F3' : '#EEEEEE')
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('fillColor配合Template模式')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row() {
        ForEach(this.FILL_COLORS, (color: ResourceColor, index: number) => {
          Column() {
            Image($r('app.media.startIcon'))
              .width(50)
              .height(50)
              .renderMode(ImageRenderMode.Template)
              .fillColor(color)
              .borderRadius(4)

            Text(color !== undefined ? `颜色${index + 1}` : '无填充')
              .fontSize(10)
              .fontColor(Color.Gray)
              .margin({ top: 4 })
          }
          .margin({ left: 4, right: 4 })
          .onClick(() => {
            this.currentFillColor = color
          })
        })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

@Component
struct ImageOnProgressLoadTestCase061 {
  @State sourceIndex: number = 0
  @State progressLog: string = '等待加载...'
  @State currentProgress: number = 0
  @State totalSize: number = 0
  @State receivedSize: number = 0
  @State progressCount: number = 0
  @State maxProgress: number = 0
  @State isComplete: boolean = false
  @State historyLog: string[] = []

  private readonly SOURCES: Array<{ src: ResourceStr; label: string; valid: boolean }> = [
    { src: $r('app.media.startIcon'), label: 'startIcon(本地)', valid: true },
    { src: $r('app.media.background'), label: 'background(本地)', valid: true },
    { src: $r('app.media.foreground'), label: 'foreground(本地)', valid: true },
    { src: '', label: '空字符串', valid: false }
  ]

  build() {
    Column() {
      Text('Image onProgressLoad回调测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Image(this.SOURCES[this.sourceIndex].src)
        .width(180)
        .height(180)
        .objectFit(ImageFit.Cover)
        .alt($r('app.media.startIcon'))
        .border({ width: 2, color: this.isComplete ? Color.Green : Color.Blue })
        .borderRadius(8)
        .backgroundColor('#F5F5F5')
        .onProgressLoad((event: { progress: number; total: number }) => {
          this.progressCount++
          this.currentProgress = event.progress
          this.totalSize = event.total
          this.receivedSize = event.progress * event.total
          this.maxProgress = Math.max(this.maxProgress, event.progress)
          this.progressLog = `进度: ${Math.round(event.progress * 100)}% (${event.progress}/${event.total})`
          if (this.historyLog.length < 20) {
            this.historyLog.push(`#${this.progressCount} ${Math.round(event.progress * 100)}%`)
          }
        })
        .onComplete(() => {
          this.isComplete = true
          this.progressLog = '加载完成'
        })
        .onError((err: { componentWidth: number; componentHeight: number; message: string }) => {
          this.progressLog = `加载失败: ${err.message}`
        })
        .margin({ bottom: 16 })

      Text(this.progressLog)
        .fontSize(13)
        .fontColor(this.isComplete ? Color.Green : Color.Orange)
        .margin({ bottom: 12 })

      Row({ space: 24 }) {
        Column() {
          Text('当前进度')
            .fontSize(11)
            .fontColor(Color.Gray)
          Text(`${Math.round(this.currentProgress * 100)}%`)
            .fontSize(20)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.Blue)
        }
        Column() {
          Text('回调次数')
            .fontSize(11)
            .fontColor(Color.Gray)
          Text(`${this.progressCount}`)
            .fontSize(20)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.Orange)
        }
        Column() {
          Text('最大进度')
            .fontSize(11)
            .fontColor(Color.Gray)
          Text(`${Math.round(this.maxProgress * 100)}%`)
            .fontSize(20)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.Green)
        }
      }
      .margin({ bottom: 16 })

      Divider()

      Text('图源选择(触发onProgressLoad)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row() {
        ForEach(this.SOURCES, (item: { src: ResourceStr; label: string; valid: boolean }, index: number) => {
          Button(item.label)
            .fontSize(10)
            .onClick(() => {
              this.sourceIndex = index
              this.currentProgress = 0
              this.progressCount = 0
              this.maxProgress = 0
              this.isComplete = false
              this.totalSize = 0
              this.receivedSize = 0
              this.historyLog = []
              this.progressLog = `已切换到 ${item.label}, 等待回调...`
            })
            .backgroundColor(index === this.sourceIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.sourceIndex ? Color.White : Color.Gray)
            .margin({ left: 4, right: 4 })
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('进度历史(最多20条)')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Text(this.historyLog.length > 0 ? this.historyLog.join('  |  ') : '暂无进度记录')
        .fontSize(11)
        .fontColor(Color.Gray)
        .maxLines(6)
        .textOverflow({ overflow: TextOverflow.Ellipsis })

      Row({ space: 12 }) {
        Button('重置')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .fontColor(Color.White)
          .onClick(() => {
            this.currentProgress = 0
            this.progressCount = 0
            this.maxProgress = 0
            this.isComplete = false
            this.totalSize = 0
            this.receivedSize = 0
            this.historyLog = []
            this.progressLog = '已重置,等待加载...'
          })
      }
      .margin({ top: 16 })
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

@Component
struct ImageCopyOptionTestCase062 {
  @State currentOptionIndex: number = 0
  @State log: string = '选择不同的copyOption,然后长按图片测试复制'

  private readonly COPY_OPTIONS: Array<{ option: CopyOptions; label: string; desc: string }> = [
    { option: CopyOptions.None, label: 'None(不可复制)', desc: '图片不可被复制' },
    { option: CopyOptions.InApp, label: 'InApp(应用内)', desc: '仅可在应用内粘贴' },
    { option: CopyOptions.LocalDevice, label: 'LocalDevice(本地)', desc: '可粘贴到其他应用' }
  ]

  build() {
    Column() {
      Text('Image copyOption属性测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        ForEach(this.COPY_OPTIONS, (config: { option: CopyOptions; label: string; desc: string }) => {
          Column() {
            Text(config.label)
              .fontSize(12)
              .fontWeight(FontWeight.Medium)
              .margin({ bottom: 8 })
            Image($r('app.media.startIcon'))
              .width(100)
              .height(100)
              .objectFit(ImageFit.Cover)
              .copyOption(config.option)
              .border({ width: 2, color: Color.Blue })
              .borderRadius(8)
            Text(config.desc)
              .fontSize(9)
              .fontColor(Color.Gray)
              .margin({ top: 4 })
              .maxLines(2)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
          }
          .margin({ left: 8, right: 8 })
        })
      }
      .margin({ bottom: 16 })

      Text(`当前选项: ${this.COPY_OPTIONS[this.currentOptionIndex].label}`)
        .fontSize(14)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })

      Text(this.log)
        .fontSize(11)
        .fontColor(Color.Orange)
        .margin({ bottom: 16 })

      Divider()

      Text('单独切换(交互测试)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row() {
        Column() {
          Image($r('app.media.background'))
            .width(120)
            .height(120)
            .objectFit(ImageFit.Cover)
            .copyOption(this.COPY_OPTIONS[this.currentOptionIndex].option)
            .border({ width: 2, color: Color.Red })
            .borderRadius(8)
          Text(`copyOption: ${this.COPY_OPTIONS[this.currentOptionIndex].label}`)
            .fontSize(10)
            .fontColor(Color.Gray)
            .margin({ top: 4 })
        }
      }
      .margin({ bottom: 16 })

      Row({ space: 8 }) {
        ForEach(this.COPY_OPTIONS, (config: { option: CopyOptions; label: string; desc: string }, index: number) => {
          Button(config.label)
            .fontSize(10)
            .onClick(() => {
              this.currentOptionIndex = index
              this.log = `已切换到 ${config.label}, 长按图片测试复制行为`
            })
            .backgroundColor(index === this.currentOptionIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentOptionIndex ? Color.White : Color.Gray)
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('说明')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Text('None: 长按无响应\nInApp: 长按后可复制,仅在应用内粘贴\nLocalDevice: 长按后可复制到系统剪贴板')
        .fontSize(11)
        .fontColor(Color.Gray)
        .lineHeight(18)
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}
