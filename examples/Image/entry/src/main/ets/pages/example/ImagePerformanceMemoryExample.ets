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
 * Image 性能与内存管理示例
 * 展示图片按需加载、缓存策略、内存回收、LazyForEach 优化等
 *
 * 注意：网络图片 URL 仅使用占位地址，实际使用需：
 * 1. 在 module.json5 中声明 ohos.permission.INTERNET 权限
 * 2. 使用 HTTPS 协议确保安全传输
 * 3. 配置网络安全配置(network_security_config)限制可信域名
 */
@Entry
@Component
struct ImagePerformanceMemoryExample {
  @State listCount: number = 20
  @State itemWidth: number = 100
  @State itemHeight: number = 100
  @State enableSourceSize: boolean = true
  @State sourceWidth: number = 100
  @State sourceHeight: number = 100
  @State enableAutoResize: boolean = false
  @State enableSyncLoad: boolean = false
  @State cacheCount: number = 0
  @State loadTimes: number[] = []
  @State avgLoadTime: number = 0
  @State totalLoaded: number = 0
  @State showStats: boolean = false
  @State currentTab: number = 0
  @State gridColumns: number = 3
  @State enableLazyForEach: boolean = true
  @State imageFit: ImageFit = ImageFit.Cover
  @State interpolation: ImageInterpolation = ImageInterpolation.Low
  @State logMessages: string[] = []

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

  private fitOptions: { name: string, value: ImageFit }[] = [
    { name: 'Cover', value: ImageFit.Cover },
    { name: 'Contain', value: ImageFit.Contain },
    { name: 'Fill', value: ImageFit.Fill },
    { name: 'Auto', value: ImageFit.Auto },
    { name: 'None', value: ImageFit.None },
    { name: 'ScaleDown', value: ImageFit.ScaleDown }
  ]

  private interpOptions: { name: string, value: ImageInterpolation }[] = [
    { name: 'None', value: ImageInterpolation.None },
    { name: 'Low', value: ImageInterpolation.Low },
    { name: 'Medium', value: ImageInterpolation.Medium },
    { name: 'High', value: ImageInterpolation.High }
  ]

  private dataSource: ImageDataSource = new ImageDataSource(20)

  addLog(msg: string) {
    const time = new Date().toLocaleTimeString()
    this.logMessages.unshift(`[${time}] ${msg}`)
    if (this.logMessages.length > 20) {
      this.logMessages.pop()
    }
  }

  build() {
    Column() {
      Text('Image 性能与内存管理')
        .fontSize(24)
        .fontWeight(FontWeight.Bold)
        .margin({ top: 20, bottom: 12 })

      Tabs({ barPosition: BarPosition.Start, index: this.currentTab }) {
        TabContent() {
          this.buildBasicTab()
        }
        .tabBar('基础配置')

        TabContent() {
          this.buildLazyForEachTab()
        }
        .tabBar('LazyForEach')

        TabContent() {
          this.buildGridTab()
        }
        .tabBar('网格优化')

        TabContent() {
          this.buildStatsTab()
        }
        .tabBar('统计日志')
      }
      .onChange((index: number) => {
        this.currentTab = index
      })
      .barWidth(80)
      .barHeight(40)
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor(0xF5F5F5)
  }

  @Builder
  buildBasicTab() {
    Scroll() {
      Column({ space: 14 }) {
        this.buildSourceSizeSection()
        this.buildFitSection()
        this.buildInterpolationSection()
        this.buildBooleanOptionsSection()
        this.buildSingleImagePreview()
      }
      .width('100%')
      .padding({ left: 16, right: 16, bottom: 20 })
    }
    .width('100%')
    .layoutWeight(1)
  }

  @Builder
  buildSourceSizeSection() {
    Column({ space: 10 }) {
      Text('sourceSize 解码尺寸')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.enableSourceSize })
          .onChange((isOn: boolean) => {
            this.enableSourceSize = isOn
          })
        Text('启用 sourceSize')
          .fontSize(14)
      }

      if (this.enableSourceSize) {
        Text(`解码宽度: ${this.sourceWidth}`)
          .fontSize(13)
        Slider({
          value: this.sourceWidth,
          min: 32,
          max: 400,
          step: 8
        })
          .width('100%')
          .onChange((value: number) => {
            this.sourceWidth = value
          })

        Text(`解码高度: ${this.sourceHeight}`)
          .fontSize(13)
        Slider({
          value: this.sourceHeight,
          min: 32,
          max: 400,
          step: 8
        })
          .width('100%')
          .onChange((value: number) => {
            this.sourceHeight = value
          })
      }

      Text('提示: sourceSize 应接近实际显示尺寸，过大会浪费内存，过小会模糊')
        .fontSize(11)
        .fontColor(0x999999)
    }
    .width('100%')
    .padding(14)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildFitSection() {
    Column({ space: 10 }) {
      Text('objectFit 填充模式')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Flex({ wrap: FlexWrap.Wrap }) {
        ForEach(this.fitOptions, (item: { name: string, value: ImageFit }, index: number) => {
          Text(item.name)
            .fontSize(12)
            .padding({ left: 12, right: 12, top: 6, bottom: 6 })
            .margin(4)
            .borderRadius(16)
            .backgroundColor(this.imageFit === item.value ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.imageFit === item.value ? Color.White : 0x333333)
            .onClick(() => {
              this.imageFit = item.value
            })
        })
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildInterpolationSection() {
    Column({ space: 10 }) {
      Text('interpolation 插值质量')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Flex({ wrap: FlexWrap.Wrap }) {
        ForEach(this.interpOptions, (item: { name: string, value: ImageInterpolation }, index: number) => {
          Text(item.name)
            .fontSize(12)
            .padding({ left: 12, right: 12, top: 6, bottom: 6 })
            .margin(4)
            .borderRadius(16)
            .backgroundColor(this.interpolation === item.value ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.interpolation === item.value ? Color.White : 0x333333)
            .onClick(() => {
              this.interpolation = item.value
            })
        })
      }

      Text('提示: 高插值质量更耗 CPU，列表滚动时建议使用 Low 或 None')
        .fontSize(11)
        .fontColor(0x999999)
    }
    .width('100%')
    .padding(14)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildBooleanOptionsSection() {
    Column({ space: 10 }) {
      Text('加载策略')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.enableAutoResize })
          .onChange((isOn: boolean) => {
            this.enableAutoResize = isOn
            this.addLog(`autoResize: ${isOn}`)
          })
        Text('autoResize')
          .fontSize(14)
      }

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.enableSyncLoad })
          .onChange((isOn: boolean) => {
            this.enableSyncLoad = isOn
            this.addLog(`syncLoad: ${isOn}`)
          })
        Text('syncLoad (同步加载)')
          .fontSize(14)
      }

      Text('提示: syncLoad 会阻塞 UI 线程，仅适合小尺寸关键图片')
        .fontSize(11)
        .fontColor(0x999999)
    }
    .width('100%')
    .padding(14)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildSingleImagePreview() {
    Column({ space: 10 }) {
      Text('单图预览')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Image($r('app.media.startIcon'))
        .width(this.itemWidth)
        .height(this.itemHeight)
        .objectFit(this.imageFit)
        .interpolation(this.interpolation)
        .autoResize(this.enableAutoResize)
        .syncLoad(this.enableSyncLoad)
        .sourceSize(this.enableSourceSize ? {
          width: this.sourceWidth,
          height: this.sourceHeight
        } : undefined)
        .borderRadius(8)
        .border({ width: 1, color: 0xDDDDDD })
        .onComplete(() => {
          this.totalLoaded++
          this.addLog('图片加载完成')
        })
        .onError(() => {
          this.addLog('图片加载失败')
        })

      Text(`${this.itemWidth} x ${this.itemHeight}`)
        .fontSize(12)
        .fontColor(0x999999)

      Row({ space: 10 }) {
        Button('小 64x64')
          .fontSize(11)
          .onClick(() => {
            this.itemWidth = 64
            this.itemHeight = 64
          })
        Button('中 150x150')
          .fontSize(11)
          .onClick(() => {
            this.itemWidth = 150
            this.itemHeight = 150
          })
        Button('大 280x280')
          .fontSize(11)
          .onClick(() => {
            this.itemWidth = 280
            this.itemHeight = 280
          })
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildLazyForEachTab() {
    Column({ space: 10 }) {
      Row({ space: 10 }) {
        Text(`数量: ${this.listCount}`)
          .fontSize(14)
        Slider({
          value: this.listCount,
          min: 5,
          max: 200,
          step: 5
        })
          .layoutWeight(1)
          .onChange((value: number) => {
            this.listCount = value
            this.dataSource = new ImageDataSource(value)
          })
      }
      .padding({ left: 16, right: 16 })

      List({ space: 4 }) {
        if (this.enableLazyForEach) {
          LazyForEach(this.dataSource, (item: ImageDataItem, index: number) => {
            ListItem() {
              Row({ space: 10 }) {
                Image(this.localResources[index % this.localResources.length])
                  .width(60)
                  .height(60)
                  .objectFit(ImageFit.Cover)
                  .borderRadius(8)
                  .sourceSize({ width: 60, height: 60 })
                  .interpolation(ImageInterpolation.Low)
                  .onComplete(() => {
                    this.totalLoaded++
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
              .padding({ left: 12, right: 12, top: 8, bottom: 8 })
              .backgroundColor(Color.White)
              .borderRadius(8)
            }
          }, (item: ImageDataItem) => item.id.toString())
        } else {
          ForEach(this.dataSource.getAllData(), (item: ImageDataItem, index: number) => {
            ListItem() {
              Row({ space: 10 }) {
                Image(this.localResources[index % this.localResources.length])
                  .width(60)
                  .height(60)
                  .objectFit(ImageFit.Cover)
                  .borderRadius(8)

                Column() {
                  Text(`Item ${index}`)
                    .fontSize(14)
                  Text(`(非 LazyForEach)`)
                    .fontSize(11)
                    .fontColor(0xFF6B6B)
                }
                .layoutWeight(1)
                .alignItems(HorizontalAlign.Start)
              }
              .width('100%')
              .padding({ left: 12, right: 12, top: 8, bottom: 8 })
              .backgroundColor(Color.White)
              .borderRadius(8)
            }
          }, (item: ImageDataItem) => item.id.toString())
        }
      }
      .width('100%')
      .layoutWeight(1)
      .padding({ left: 12, right: 12 })

      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.enableLazyForEach })
          .onChange((isOn: boolean) => {
            this.enableLazyForEach = isOn
            this.addLog(`LazyForEach: ${isOn}`)
          })
        Text('启用 LazyForEach')
          .fontSize(14)
      }
      .padding({ left: 16, bottom: 12 })
    }
    .width('100%')
    .height('100%')
  }

  @Builder
  buildGridTab() {
    Column({ space: 10 }) {
      Row({ space: 10 }) {
        Text(`列数: ${this.gridColumns}`)
          .fontSize(14)
        Slider({
          value: this.gridColumns,
          min: 2,
          max: 6,
          step: 1
        })
          .layoutWeight(1)
          .onChange((value: number) => {
            this.gridColumns = value
          })
      }
      .padding({ left: 16, right: 16 })

      Grid() {
        ForEach(this.dataSource.getAllData(), (item: ImageDataItem, index: number) => {
          GridItem() {
            Image(this.localResources[index % this.localResources.length])
              .width('100%')
              .aspectRatio(1)
              .objectFit(ImageFit.Cover)
              .borderRadius(8)
              .sourceSize({ width: 100, height: 100 })
              .onComplete(() => {
                this.totalLoaded++
              })
          }
        }, (item: ImageDataItem) => item.id.toString())
      }
      .columnsTemplate(`repeat(${this.gridColumns}, 1fr)`)
      .columnsGap(6)
      .rowsGap(6)
      .width('100%')
      .layoutWeight(1)
      .padding({ left: 12, right: 12 })
    }
    .width('100%')
    .height('100%')
  }

  @Builder
  buildStatsTab() {
    Column({ space: 12 }) {
      Row({ space: 20 }) {
        Column() {
          Text(this.totalLoaded.toString())
            .fontSize(28)
            .fontWeight(FontWeight.Bold)
            .fontColor(0x007DFF)
          Text('已加载')
            .fontSize(12)
            .fontColor(0x999999)
        }

        Column() {
          Text(this.listCount.toString())
            .fontSize(28)
            .fontWeight(FontWeight.Bold)
            .fontColor(0x4CAF50)
          Text('列表项')
            .fontSize(12)
            .fontColor(0x999999)
        }

        Column() {
          Text(this.enableLazyForEach ? '是' : '否')
            .fontSize(28)
            .fontWeight(FontWeight.Bold)
            .fontColor(0xFF9800)
          Text('LazyForEach')
            .fontSize(12)
            .fontColor(0x999999)
        }
      }
      .width('100%')
      .justifyContent(FlexAlign.SpaceEvenly)
      .padding(16)
      .backgroundColor(Color.White)
      .borderRadius(12)

      Column({ space: 8 }) {
        Text('优化建议')
          .fontSize(16)
          .fontWeight(FontWeight.Medium)

        Text('1. 使用 sourceSize 限制解码尺寸，避免解码原图')
          .fontSize(13)
          .fontColor(0x333333)
        Text('2. 列表场景优先使用 LazyForEach 按需加载')
          .fontSize(13)
          .fontColor(0x333333)
        Text('3. 滚动列表中使用低插值 (Low/None)')
          .fontSize(13)
          .fontColor(0x333333)
        Text('4. 避免在列表项中使用 syncLoad 同步加载')
          .fontSize(13)
          .fontColor(0x333333)
        Text('5. 网络图片需声明 ohos.permission.INTERNET')
          .fontSize(13)
          .fontColor(0x333333)
        Text('6. 使用 HTTPS 防止中间人攻击')
          .fontSize(13)
          .fontColor(0x333333)
        Text('7. 及时回收不可见图片的内存')
          .fontSize(13)
          .fontColor(0x333333)
      }
      .width('100%')
      .padding(14)
      .backgroundColor(Color.White)
      .borderRadius(12)

      Column({ space: 8 }) {
        Row({ space: 10 }) {
          Text('运行日志')
            .fontSize(16)
            .fontWeight(FontWeight.Medium)
          Blank()
          Button('清除')
            .fontSize(11)
            .height(28)
            .onClick(() => {
              this.logMessages = []
            })
        }

        Scroll() {
          Column() {
            if (this.logMessages.length === 0) {
              Text('暂无日志')
                .fontSize(13)
                .fontColor(0x999999)
                .margin({ top: 20 })
            } else {
              ForEach(this.logMessages, (msg: string) => {
                Text(msg)
                  .fontSize(11)
                  .fontColor(0x333333)
                  .fontFamily('monospace')
                  .width('100%')
                  .padding({ top: 2, bottom: 2 })
              })
            }
          }
          .width('100%')
        }
        .width('100%')
        .height(200)
        .backgroundColor(0xF8F8F8)
        .borderRadius(8)
        .padding(10)
      }
      .width('100%')
      .padding(14)
      .backgroundColor(Color.White)
      .borderRadius(12)
    }
    .width('100%')
    .padding({ left: 16, right: 16, top: 10 })
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

  reloadData(count: number): void {
    this.items = []
    for (let i = 0; i < count; i++) {
      this.items.push({ id: i })
    }
    for (const listener of this.listeners) {
      listener.onDataReloaded()
    }
  }
}
