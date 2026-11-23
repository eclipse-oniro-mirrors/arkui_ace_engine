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
 * Image 网格性能示例
 * 展示 Grid 组件中图片的性能优化
 */
@Entry
@Component
struct ImageGridPerformanceExample {
  @State columnCount: number = 3
  @State itemCount: number = 30
  @State enableSourceSize: boolean = true
  @State loadLog: string = '等待加载...'

  private logs: string[] = []
  private dataSource: GridDataSource = new GridDataSource(30)

  private localResources: Resource[] = [
    $r('app.media.startIcon'),
    $r('app.media.background'),
    $r('app.media.foreground'),
    $r('app.media.heart'),
    $r('app.media.star'),
    $r('app.media.icon'),
    $r('app.media.svg'),
    $r('app.media.testSvg')
  ]

  addLog(msg: string) {
    const time = new Date().toLocaleTimeString()
    this.logs.unshift(`[${time}] ${msg}`)
    if (this.logs.length > 5) {
      this.logs.pop()
    }
    this.loadLog = this.logs.join('\n')
  }

  build() {
    Column({ space: 16 }) {
      Text('Image 网格性能示例')
        .fontSize(24)
        .fontWeight(FontWeight.Bold)
        .margin({ top: 20 })

      this.buildControlSection()
      this.buildGridSection()
      this.buildLogSection()
      this.buildTipsSection()
    }
    .width('100%')
    .height('100%')
    .backgroundColor(0xF5F5F5)
  }

  @Builder
  buildControlSection() {
    Column({ space: 12 }) {
      Text('控制选项')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text(`列数: ${this.columnCount}`)
        .fontSize(14)
      Slider({
        value: this.columnCount,
        min: 2,
        max: 6,
        step: 1
      })
        .width('100%')
        .onChange((value: number) => {
          this.columnCount = value
        })

      Text(`项目数: ${this.itemCount}`)
        .fontSize(14)
      Slider({
        value: this.itemCount,
        min: 10,
        max: 100,
        step: 5
      })
        .width('100%')
        .onChange((value: number) => {
          this.itemCount = value
          this.dataSource = new GridDataSource(value)
        })

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.enableSourceSize })
          .onChange((isOn: boolean) => {
            this.enableSourceSize = isOn
          })
        Text('启用 sourceSize')
          .fontSize(14)
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildGridSection() {
    Column({ space: 12 }) {
      Text('图片网格')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Grid() {
        ForEach(this.dataSource.getAllData(), (item: GridDataItem, index: number) => {
          GridItem() {
            Image(this.localResources[index % this.localResources.length])
              .width('100%')
              .aspectRatio(1)
              .objectFit(ImageFit.Cover)
              .borderRadius(8)
              .sourceSize(this.enableSourceSize ? { width: 100, height: 100 } : undefined)
              .onComplete(() => {
                this.addLog(`加载: Item ${index}`)
              })
          }
        }, (item: GridDataItem) => item.id.toString())
      }
      .columnsTemplate(`repeat(${this.columnCount}, 1fr)`)
      .columnsGap(6)
      .rowsGap(6)
      .width('100%')
      .layoutWeight(1)
      .padding({ left: 12, right: 12 })
    }
    .width('100%')
    .layoutWeight(1)
    .padding({ left: 16, right: 16 })
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
    .padding({ left: 16, right: 16 })
  }

  @Builder
  buildTipsSection() {
    Column({ space: 8 }) {
      Text('使用提示')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('1. Grid 组件适合展示图片网格')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 配合 sourceSize 降低内存')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 列数可根据屏幕宽度调整')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 大数据量时注意性能')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 可使用 LazyForEach 进一步优化')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .margin({ left: 16, right: 16, bottom: 20 })
  }
}

interface GridDataItem {
  id: number
}

class GridDataSource implements IDataSource {
  private items: GridDataItem[] = []
  private listeners: DataChangeListener[] = []

  constructor(count: number) {
    for (let i = 0; i < count; i++) {
      this.items.push({ id: i })
    }
  }

  totalCount(): number {
    return this.items.length
  }

  getData(index: number): GridDataItem {
    return this.items[index]
  }

  getAllData(): GridDataItem[] {
    return this.items
  }

  registerDataChangeListener(listener: DataChangeListener): void {
    if (this.listeners.indexOf(listener) < 0) {
      this.listeners.push(listener)
    }
  }

  unregisterDataChangeListener(listener: DataChangeListener): void {
    const pos = this.listeners.indexOf(listener)
    if (pos >= 0) {
      this.listeners.splice(pos, 1)
    }
  }
}
