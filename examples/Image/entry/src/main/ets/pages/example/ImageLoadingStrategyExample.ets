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
 * Image 加载策略示例
 * 展示 syncLoad、autoResize 等加载策略
 */
@Entry
@Component
struct ImageLoadingStrategyExample {
  @State syncLoad: boolean = false
  @State autoResize: boolean = false
  @State loadLog: string = '等待加载...'
  @State loadCount: number = 0

  private logs: string[] = []

  addLog(msg: string) {
    const time = new Date().toLocaleTimeString()
    this.logs.unshift(`[${time}] ${msg}`)
    if (this.logs.length > 5) {
      this.logs.pop()
    }
    this.loadLog = this.logs.join('\n')
  }

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 加载策略示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildPreviewSection()
        this.buildLogSection()
        this.buildStrategySection()
        this.buildComparisonSection()
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
  buildPreviewSection() {
    Column({ space: 12 }) {
      Text('预览')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Image($r('app.media.startIcon'))
        .width(150)
        .height(150)
        .objectFit(ImageFit.Cover)
        .borderRadius(12)
        .border({ width: 1, color: 0xCCCCCC })
        .syncLoad(this.syncLoad)
        .autoResize(this.autoResize)
        .onComplete(() => {
          this.loadCount++
          this.addLog(`加载完成 #${this.loadCount}`)
        })

      Text(`syncLoad: ${this.syncLoad ? '同步' : '异步'}`)
        .fontSize(12)
        .fontColor(0x666666)
      Text(`autoResize: ${this.autoResize ? '启用' : '禁用'}`)
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
      Text('加载日志')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Scroll() {
        Text(this.loadLog)
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
  buildStrategySection() {
    Column({ space: 12 }) {
      Text('加载策略')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.syncLoad })
          .onChange((isOn: boolean) => {
            this.syncLoad = isOn
            this.addLog(`syncLoad: ${isOn ? '同步' : '异步'}`)
          })
        Column() {
          Text('syncLoad')
            .fontSize(14)
          Text('同步加载（阻塞UI）')
            .fontSize(11)
            .fontColor(0x999999)
        }
      }

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.autoResize })
          .onChange((isOn: boolean) => {
            this.autoResize = isOn
            this.addLog(`autoResize: ${isOn ? '启用' : '禁用'}`)
          })
        Column() {
          Text('autoResize')
            .fontSize(14)
          Text('自动调整解码尺寸')
            .fontSize(11)
            .fontColor(0x999999)
        }
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildComparisonSection() {
    Column({ space: 12 }) {
      Text('策略对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('不同策略组合的加载效果')
        .fontSize(12)
        .fontColor(0x999999)

      Row({ space: 8 }) {
        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .syncLoad(false)
            .autoResize(false)
          Text('默认')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .syncLoad(true)
            .autoResize(false)
          Text('同步')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .syncLoad(false)
            .autoResize(true)
          Text('自动调整')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(60)
            .height(60)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .syncLoad(true)
            .autoResize(true)
          Text('两者')
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

  @Builder
  buildTipsSection() {
    Column({ space: 8 }) {
      Text('使用提示')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('1. syncLoad: 同步加载会阻塞UI线程')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 仅适合小尺寸关键图片')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. autoResize: 根据显示尺寸自动调整')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 可降低内存占用')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 默认异步加载，推荐使用')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
