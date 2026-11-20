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
 * Image LazyForEach 列表示例
 * 展示 LazyForEach 在图片列表中的性能优化
 */
@Entry
@Component
struct ImageLazyForEachExample {
  @State itemCount: number = 50
  @State enableLazyForEach: boolean = true
  @State loadLog: string = '等待加载...'

  private logs: string[] = []
  private dataSource: ImageDataSource = new ImageDataSource(50)

  private localResources: Resource[] = [
    $r('app.media.startIcon'),
    $r('app.media.background'),
    $r('app.media.foreground'),
    $r('app.media.heart'),
    $r('app.media.star'),
    $r('app.media.icon')
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
      Text('Image LazyForEach 示例')
        .fontSize(24)
        .fontWeight(FontWeight.Bold)
        .margin({ top: 20 })

      this.buildControlSection()
      this.buildListSection()
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

      Text(`列表项数: ${this.itemCount}`)
        .fontSize(14)
      Slider({
        value: this.itemCount,
        min: 10,
        max: 200,
        step: 10
      })
        .width('100%')
        .onChange((value: number) => {
          this.itemCount = value
          this.dataSource = new ImageDataSource(value)
        })

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.enableLazyForEach })
          .onChange((isOn: boolean) => {
            this.enableLazyForEach = isOn
            this.addLog(`LazyForEach: ${isOn ? '启用' : '禁用'}`)
          })
        Text('启用 LazyForEach')
          .fontSize(14)
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildListSection() {
    Column({ space: 12 }) {
      Text('图片列表')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      List({ space: 4 }) {
        if (this.enableLazyForEach) {
          LazyForEach(this.dataSource, (item: ImageDataItem, index: number) => {
            ListItem() {
              Row({ space: 12 }) {
                Image(this.localResources[index % this.localResources.length])
                  .width(60)
                  .height(60)
                  .objectFit(ImageFit.Cover)
                  .borderRadius(8)
                  .sourceSize({ width: 60, height: 60 })
                  .onComplete(() => {
                    this.addLog(`加载: Item ${index}`)
                  })

                Column() {
                  Text(`Item ${index}`)
                    .fontSize(14)
                    .fontWeight(FontWeight.Medium)
                  Text(`资源: ${index % this.localResources.length}`)
                    .fontSize(12)
                    .fontColor(0x999999)
                }
                .layoutWeight(1)
                .alignItems(HorizontalAlign.Start)
              }
              .width('100%')
              .padding(12)
              .backgroundColor(Color.White)
              .borderRadius(8)
            }
          }, (item: ImageDataItem) => item.id.toString())
        } else {
          ForEach(this.dataSource.getAllData(), (item: ImageDataItem, index: number) => {
            ListItem() {
              Row({ space: 12 }) {
                Image(this.localResources[index % this.localResources.length])
                  .width(60)
                  .height(60)
                  .objectFit(ImageFit.Cover)
                  .borderRadius(8)

                Column() {
                  Text(`Item ${index}`)
                    .fontSize(14)
                  Text('(非 LazyForEach)')
                    .fontSize(11)
                    .fontColor(0xFF6B6B)
                }
                .layoutWeight(1)
                .alignItems(HorizontalAlign.Start)
              }
              .width('100%')
              .padding(12)
              .backgroundColor(Color.White)
              .borderRadius(8)
            }
          }, (item: ImageDataItem) => item.id.toString())
        }
      }
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

      Text('1. LazyForEach 按需加载，仅渲染可见项')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 长列表场景性能优势明显')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 配合 sourceSize 可进一步降低内存')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. ForEach 会一次性创建所有项')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 大数据量时推荐使用 LazyForEach')
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

interface ImageDataItem {
  id: number
}

class ImageDataSource implements IDataSource {
  private items: ImageDataItem[] = []
  private listeners: DataChangeListener[] = []

  constructor(count: number) {
    for (let i = 0; i < count; i++) {
      this.items.push({ id: i })
    }
  }

  totalCount(): number {
    return this.items.length
  }

  getData(index: number): ImageDataItem {
    return this.items[index]
  }

  getAllData(): ImageDataItem[] {
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
