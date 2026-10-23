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


@Component
struct ImageDynamicRangeScene {
  @State currentModeIndex: number = 0
  @State imageWidth: number = 180
  @State imageHeight: number = 180

  private readonly DYNAMIC_MODES: Array<{ mode: DynamicRangeMode; label: string; desc: string }> = [
    { mode: DynamicRangeMode.STANDARD, label: 'STANDARD(标准)', desc: 'SDR 标准动态范围' },
    { mode: DynamicRangeMode.HIGH, label: 'HIGH(高动态)', desc: 'HDR 高动态范围,需设备支持' }
  ]

  build() {
    Column() {
      Text('Image dynamicRangeMode属性测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        ForEach(this.DYNAMIC_MODES, (config: { mode: DynamicRangeMode; label: string; desc: string }) => {
          Column() {
            Text(config.label)
              .fontSize(13)
              .fontWeight(FontWeight.Medium)
              .margin({ bottom: 8 })
            Image($r('app.media.startIcon'))
              .width(this.imageWidth)
              .height(this.imageHeight)
              .objectFit(ImageFit.Cover)
              .dynamicRangeMode(config.mode)
              .interpolation(ImageInterpolation.High)
              .border({ width: 2, color: Color.Blue })
              .borderRadius(8)
            Text(config.desc)
              .fontSize(10)
              .fontColor(Color.Gray)
              .margin({ top: 4 })
          }
          .margin({ left: 8, right: 8 })
        })
      }
      .margin({ bottom: 16 })

      Text(`当前模式: ${this.DYNAMIC_MODES[this.currentModeIndex].label}`)
        .fontSize(14)
        .fontColor(Color.Gray)
        .margin({ bottom: 12 })

      Row() {
        ForEach(this.DYNAMIC_MODES, (config: { mode: DynamicRangeMode; label: string; desc: string },
            index: number) => {
          Button(config.label)
            .fontSize(11)
            .onClick(() => { this.currentModeIndex = index })
            .backgroundColor(index === this.currentModeIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentModeIndex ? Color.White : Color.Gray)
            .margin({ left: 4, right: 4 })
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('尺寸调节观察效果')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 12 }) {
        Button('120x120')
          .fontSize(11)
          .onClick(() => { this.imageWidth = 120; this.imageHeight = 120 })
        Button('180x180')
          .fontSize(11)
          .onClick(() => { this.imageWidth = 180; this.imageHeight = 180 })
        Button('240x240')
          .fontSize(11)
          .onClick(() => { this.imageWidth = 240; this.imageHeight = 240 })
        Button('重置')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .onClick(() => { this.imageWidth = 180; this.imageHeight = 180; this.currentModeIndex = 0 })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 原 055：sourceSize 源尺寸解码 */
@Component
struct ImageSourceSizeScene {
  @State currentSizeIndex: number = 0
  @State displayWidth: number = 200
  @State displayHeight: number = 200

  private readonly SOURCE_SIZES: Array<{ width: number; height: number; label: string }> = [
    { width: 50, height: 50, label: '50x50(小)' },
    { width: 100, height: 100, label: '100x100(中)' },
    { width: 200, height: 200, label: '200x200(原)' },
    { width: 400, height: 400, label: '400x400(大)' },
    { width: 20, height: 20, label: '20x20(极小)' },
    { width: 100, height: 50, label: '100x50(扁)' },
    { width: 50, height: 100, label: '50x100(长)' }
  ]

  build() {
    Column() {
      Text('Image sourceSize属性测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Column() {
          Text('有sourceSize约束')
            .fontSize(13)
            .margin({ bottom: 8 })
          Image($r('app.media.startIcon'))
            .width(this.displayWidth)
            .height(this.displayHeight)
            .objectFit(ImageFit.Cover)
            .sourceSize({
              width: this.SOURCE_SIZES[this.currentSizeIndex].width,
              height: this.SOURCE_SIZES[this.currentSizeIndex].height
            })
            .interpolation(ImageInterpolation.High)
            .border({ width: 2, color: Color.Blue })
            .borderRadius(8)
        }
        .margin({ right: 16 })

        Column() {
          Text('无约束(对照)')
            .fontSize(13)
            .margin({ bottom: 8 })
          Image($r('app.media.startIcon'))
            .width(this.displayWidth)
            .height(this.displayHeight)
            .objectFit(ImageFit.Cover)
            .interpolation(ImageInterpolation.High)
            .border({ width: 1, color: Color.Gray })
            .borderRadius(8)
        }
      }
      .margin({ bottom: 16 })

      Text(`当前源尺寸: ${this.SOURCE_SIZES[this.currentSizeIndex].label}`)
        .fontSize(14)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })
      Text(`显示尺寸: ${this.displayWidth}x${this.displayHeight} (放大观察解码细节)`)
        .fontSize(12)
        .fontColor(Color.Orange)
        .margin({ bottom: 12 })

      Grid() {
        ForEach(this.SOURCE_SIZES, (config: { width: number; height: number; label: string }, index: number) => {
          GridItem() {
            Column() {
              Text(config.label)
                .fontSize(11)
                .fontColor(index === this.currentSizeIndex ? Color.Blue : Color.Gray)
            }
            .padding(8)
            .backgroundColor(index === this.currentSizeIndex ? '#E3F2FD' : '#FAFAFA')
            .borderRadius(4)
            .onClick(() => { this.currentSizeIndex = index })
          }
        })
      }
      .columnsTemplate('repeat(4, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(16)

      Divider()

      Row({ space: 12 }) {
        Text('显示尺寸:')
          .fontSize(13)
        Button('100x100')
          .fontSize(11)
          .onClick(() => { this.displayWidth = 100; this.displayHeight = 100 })
        Button('200x200')
          .fontSize(11)
          .onClick(() => { this.displayWidth = 200; this.displayHeight = 200 })
        Button('300x300')
          .fontSize(11)
          .onClick(() => { this.displayWidth = 300; this.displayHeight = 300 })
      }
      .margin({ top: 16 })
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 原 056：enableAnalyzer + privacySensitive */
@Component
struct ImageAnalyzerPrivacyScene {
  @State analyzerEnabled: boolean = false
  @State privacySensitive: boolean = false
  @State analyzerType: number = 0
  @State log: string = '点击下方按钮切换状态'

  private readonly ANALYZER_TYPES: Array<{ type: number; label: string }> = [
    { type: 0, label: '默认(通用识别)' },
    { type: 1, label: '主体识别' },
    { type: 2, label: '文字识别' },
    { type: 3, label: '分类标签' }
  ]

  build() {
    Column() {
      Text('Image enableAnalyzer与privacySensitive测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Image($r('app.media.startIcon'))
          .width(160)
          .height(160)
          .objectFit(ImageFit.Cover)
          .enableAnalyzer(this.analyzerEnabled)
          .privacySensitive(this.privacySensitive)
          .border({ width: 2, color: this.analyzerEnabled ? Color.Green : Color.Blue })
          .borderRadius(8)
          .margin({ right: 16 })

        Column({ space: 8 }) {
          Text('当前状态')
            .fontSize(14)
            .fontWeight(FontWeight.Medium)
          Text(`analyzer: ${this.analyzerEnabled ? '开启' : '关闭'}`)
            .fontSize(12)
            .fontColor(this.analyzerEnabled ? Color.Green : Color.Gray)
          Text(`privacy: ${this.privacySensitive ? '敏感' : '正常'}`)
            .fontSize(12)
            .fontColor(this.privacySensitive ? Color.Red : Color.Gray)
          Text(`识别类型: ${this.ANALYZER_TYPES[this.analyzerType].label}`)
            .fontSize(12)
            .fontColor(Color.Gray)
        }
        .alignItems(HorizontalAlign.Start)
      }
      .margin({ bottom: 16 })

      Text(this.log)
        .fontSize(12)
        .fontColor(Color.Orange)
        .margin({ bottom: 12 })

      Divider()

      Text('属性开关')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 12 }) {
        Button(this.analyzerEnabled ? '分析: 开' : '分析: 关')
          .fontSize(11)
          .backgroundColor(this.analyzerEnabled ? '#4CAF50' : '#EEEEEE')
          .fontColor(this.analyzerEnabled ? Color.White : Color.Gray)
          .onClick(() => {
            this.analyzerEnabled = !this.analyzerEnabled
            this.log = `AI分析已${this.analyzerEnabled ? '开启' : '关闭'}`
          })

        Button(this.privacySensitive ? '隐私: 敏感' : '隐私: 正常')
          .fontSize(11)
          .backgroundColor(this.privacySensitive ? '#FF6666' : '#EEEEEE')
          .fontColor(this.privacySensitive ? Color.White : Color.Gray)
          .onClick(() => {
            this.privacySensitive = !this.privacySensitive
            this.log = `隐私状态切换为${this.privacySensitive ? '敏感(遮挡)' : '正常'}`
          })

        Button('同时开启')
          .fontSize(11)
          .backgroundColor('#2196F3')
          .fontColor(Color.White)
          .onClick(() => {
            this.analyzerEnabled = true
            this.privacySensitive = true
            this.log = '分析+敏感 同时开启(验证互斥/优先级)'
          })

        Button('全部关闭')
          .fontSize(11)
          .backgroundColor('#9E9E9E')
          .fontColor(Color.White)
          .onClick(() => {
            this.analyzerEnabled = false
            this.privacySensitive = false
            this.log = '所有属性已重置'
          })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('识别类型(analyzerType)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row() {
        ForEach(this.ANALYZER_TYPES, (config: { type: number; label: string }, index: number) => {
          Button(config.label)
            .fontSize(10)
            .onClick(() => {
              this.analyzerType = config.type
              this.analyzerEnabled = true
              this.log = `切换到 ${config.label} 并开启分析`
            })
            .backgroundColor(index === this.analyzerType ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.analyzerType ? Color.White : Color.Gray)
            .margin({ left: 4, right: 4 })
        })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 原 057：加载回调 onComplete / onError */
@Component
struct ImageLoadCallbackScene {
  @State sourceIndex: number = 0
  @State useAlt: boolean = true
  @State completeCount: number = 0
  @State errorCount: number = 0
  @State lastCompleteInfo: string = ''
  @State lastErrorInfo: string = ''
  @State log: string = '等待图片加载...'

  private readonly SOURCES: Array<{ src: ResourceStr; label: string; valid: boolean }> = [
    { src: $r('app.media.startIcon'), label: 'startIcon(有效)', valid: true },
    { src: $r('app.media.background'), label: 'background(有效)', valid: true },
    { src: $r('app.media.foreground'), label: 'foreground(有效)', valid: true },
    { src: 'invalid://nonexistent.png', label: '无效URL', valid: false },
    { src: '', label: '空字符串', valid: false }
  ]

  build() {
    Column() {
      Text('Image 加载回调测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Image(this.SOURCES[this.sourceIndex].src)
        .width(180)
        .height(180)
        .objectFit(ImageFit.Cover)
        .alt(this.useAlt ? $r('app.media.startIcon') : undefined)
        .border({ width: 2, color: this.SOURCES[this.sourceIndex].valid ? Color.Blue : Color.Red })
        .borderRadius(8)
        .backgroundColor('#F5F5F5')
        .onComplete((event: { width: number; height: number; componentWidth: number; componentHeight: number }) => {
          this.completeCount++
          this.lastCompleteInfo = `宽${event.width}x高${event.height}, 组件${event.componentWidth}x${event.componentHeight}`
          this.log = `✓ onComplete 触发 [${this.SOURCES[this.sourceIndex].label}]`
        })
        .onError((event: { componentWidth: number; componentHeight: number; message: string }) => {
          this.errorCount++
          this.lastErrorInfo = `组件${event.componentWidth}x${event.componentHeight}, msg: ${event.message}`
          this.log = `✗ onError 触发 [${this.SOURCES[this.sourceIndex].label}]`
        })
        .margin({ bottom: 16 })

      Text(this.log)
        .fontSize(13)
        .fontColor(this.log.startsWith('✓') ? Color.Green : (this.log.startsWith('✗') ? Color.Red : Color.Gray))
        .margin({ bottom: 8 })

      Row({ space: 24 }) {
        Column() {
          Text('onComplete 次数')
            .fontSize(11)
            .fontColor(Color.Gray)
          Text(`${this.completeCount}`)
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.Green)
        }
        Column() {
          Text('onError 次数')
            .fontSize(11)
            .fontColor(Color.Gray)
          Text(`${this.errorCount}`)
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.Red)
        }
      }
      .margin({ bottom: 12 })

      Divider()

      Text('最近回调详情')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Text(`完成: ${this.lastCompleteInfo || '无'}`)
        .fontSize(11)
        .fontColor(Color.Gray)
        .margin({ bottom: 4 })
      Text(`错误: ${this.lastErrorInfo || '无'}`)
        .fontSize(11)
        .fontColor(Color.Gray)
        .margin({ bottom: 12 })

      Divider()

      Text('图源选择(观察回调触发)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Grid() {
        ForEach(this.SOURCES, (item: { src: ResourceStr; label: string; valid: boolean }, index: number) => {
          GridItem() {
            Column() {
              Text(item.label)
                .fontSize(11)
                .fontColor(index === this.sourceIndex ? Color.Blue : Color.Gray)
            }
            .padding(8)
            .backgroundColor(index === this.sourceIndex ? '#E3F2FD' : '#FAFAFA')
            .borderRadius(4)
            .onClick(() => {
              this.sourceIndex = index
              this.log = `已切换到 ${item.label}, 等待回调...`
            })
          }
        })
      }
      .columnsTemplate('repeat(3, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(8)

      Row({ space: 12 }) {
        Button(this.useAlt ? '占位图: 开' : '占位图: 关')
          .fontSize(11)
          .onClick(() => { this.useAlt = !this.useAlt })
        Button('重置计数')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .fontColor(Color.White)
          .onClick(() => {
            this.completeCount = 0
            this.errorCount = 0
            this.lastCompleteInfo = ''
            this.lastErrorInfo = ''
            this.log = '计数已重置'
          })
      }
      .margin({ top: 12 })
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 新增 058：syncLoad 同步/异步加载对比 */
@Component
struct ImageSyncLoadScene {
  @State syncLoad: boolean = false
  @State loadCount: number = 0
  @State lastLoadMode: string = '异步(默认)'
  @State renderLog: string = ''
  @State switchTimes: number = 0

  build() {
    Column() {
      Text('Image syncLoad同步加载测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Column() {
          Text('syncLoad = ' + this.syncLoad)
            .fontSize(13)
            .fontWeight(FontWeight.Medium)
            .margin({ bottom: 8 })
          Image($r('app.media.startIcon'))
            .width(150)
            .height(150)
            .objectFit(ImageFit.Cover)
            .syncLoad(this.syncLoad)
            .border({ width: 2, color: this.syncLoad ? Color.Red : Color.Blue })
            .borderRadius(8)
            .onComplete(() => {
              this.loadCount++
              this.renderLog = `完成渲染 (${this.lastLoadMode})`
            })
          Text(this.syncLoad ? '同步:阻塞主线程解码' : '异步:不阻塞主线程')
            .fontSize(10)
            .fontColor(this.syncLoad ? Color.Red : Color.Gray)
            .margin({ top: 4 })
        }
        .margin({ right: 16 })

        Column() {
          Text('加载统计')
            .fontSize(13)
            .fontWeight(FontWeight.Medium)
            .margin({ bottom: 8 })
          Text(`完成次数: ${this.loadCount}`)
            .fontSize(12)
            .fontColor(Color.Gray)
            .margin({ bottom: 4 })
          Text(`当前模式: ${this.lastLoadMode}`)
            .fontSize(12)
            .fontColor(this.syncLoad ? Color.Red : Color.Blue)
            .margin({ bottom: 4 })
          Text(`切换次数: ${this.switchTimes}`)
            .fontSize(12)
            .fontColor(Color.Orange)
            .margin({ bottom: 4 })
          Text(this.renderLog || '尚未渲染')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .alignItems(HorizontalAlign.Start)
      }
      .margin({ bottom: 16 })

      Divider()

      Text('模式切换')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 12 }) {
        Button('异步加载(默认)')
          .fontSize(11)
          .backgroundColor(!this.syncLoad ? '#2196F3' : '#EEEEEE')
          .fontColor(!this.syncLoad ? Color.White : Color.Gray)
          .onClick(() => {
            this.syncLoad = false
            this.lastLoadMode = '异步'
            this.switchTimes++
          })
        Button('同步加载')
          .fontSize(11)
          .backgroundColor(this.syncLoad ? '#FF6666' : '#EEEEEE')
          .fontColor(this.syncLoad ? Color.White : Color.Gray)
          .onClick(() => {
            this.syncLoad = true
            this.lastLoadMode = '同步'
            this.switchTimes++
          })
      }
      .margin({ bottom: 12 })

      Divider()

      Text('快速连续切换(观察渲染时序)')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 8 }) {
        Button('连切5次')
          .fontSize(11)
          .onClick(() => {
            for (let i = 0; i < 5; i++) {
              this.syncLoad = !this.syncLoad
              this.switchTimes++
            }
            this.lastLoadMode = this.syncLoad ? '同步' : '异步'
          })
        Button('重置')
          .fontSize(11)
          .backgroundColor('#9E9E9E')
          .fontColor(Color.White)
          .onClick(() => {
            this.syncLoad = false
            this.loadCount = 0
            this.switchTimes = 0
            this.lastLoadMode = '异步(默认)'
            this.renderLog = ''
          })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 新增 059：autoResize 自动尺寸调整 */
@Component
struct ImageAutoResizeScene {
  @State autoResize: boolean = true
  @State imageWidth: number = 200
  @State imageHeight: number = 200
  @State currentPresetIndex: number = 0
  @State log: string = 'autoResize 开启时按组件尺寸解码,节省内存'

  private readonly SIZE_PRESETS: Array<{ w: number; h: number; label: string }> = [
    { w: 80, h: 80, label: '80x80(小)' },
    { w: 150, h: 150, label: '150x150(中)' },
    { w: 250, h: 250, label: '250x250(大)' },
    { w: 320, h: 180, label: '320x180(横)' },
    { w: 180, h: 320, label: '180x320(竖)' },
    { w: 50, h: 50, label: '50x50(极小)' },
    { w: 400, h: 400, label: '400x400(超大)' },
    { w: 120, h: 200, label: '120x200(异形)' }
  ]

  build() {
    Column() {
      Text('Image autoResize属性测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Column() {
          Text('autoResize = ' + this.autoResize)
            .fontSize(13)
            .fontWeight(FontWeight.Medium)
            .margin({ bottom: 8 })
          Image($r('app.media.startIcon'))
            .width(this.imageWidth)
            .height(this.imageHeight)
            .objectFit(ImageFit.Cover)
            .autoResize(this.autoResize)
            .border({ width: 2, color: this.autoResize ? Color.Green : Color.Orange })
            .borderRadius(8)
        }
        .margin({ right: 16 })

        Column({ space: 6 }) {
          Text('对比说明')
            .fontSize(13)
            .fontWeight(FontWeight.Medium)
          Text('开: 按组件尺寸解码(省内存)')
            .fontSize(10)
            .fontColor(Color.Green)
          Text('关: 按原图尺寸解码(清晰但耗内存)')
            .fontSize(10)
            .fontColor(Color.Orange)
          Text(`当前: ${this.imageWidth}x${this.imageHeight}`)
            .fontSize(11)
            .fontColor(Color.Blue)
            .margin({ top: 4 })
          Text(`预设: ${this.SIZE_PRESETS[this.currentPresetIndex].label}`)
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .alignItems(HorizontalAlign.Start)
      }
      .margin({ bottom: 16 })

      Text(this.log)
        .fontSize(11)
        .fontColor(Color.Gray)
        .margin({ bottom: 12 })

      Divider()

      Text('autoResize 开关')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 12 }) {
        Button(this.autoResize ? 'autoResize: 开' : 'autoResize: 关')
          .fontSize(11)
          .backgroundColor(this.autoResize ? '#4CAF50' : '#FF9800')
          .fontColor(Color.White)
          .onClick(() => {
            this.autoResize = !this.autoResize
            this.log = `autoResize 已${this.autoResize ? '开启(按组件尺寸解码)' : '关闭(按原图解码)'}`
          })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('尺寸预设(切换后观察内存/清晰度)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Grid() {
        ForEach(this.SIZE_PRESETS, (preset: { w: number; h: number; label: string }, index: number) => {
          GridItem() {
            Column() {
              Text(preset.label)
                .fontSize(10)
                .fontColor(index === this.currentPresetIndex ? Color.Blue : Color.Gray)
            }
            .padding(6)
            .backgroundColor(index === this.currentPresetIndex ? '#E3F2FD' : '#FAFAFA')
            .borderRadius(4)
            .onClick(() => {
              this.currentPresetIndex = index
              this.imageWidth = preset.w
              this.imageHeight = preset.h
              this.log = `切换到 ${preset.label}, autoResize=${this.autoResize}`
            })
          }
        })
      }
      .columnsTemplate('repeat(4, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(8)

      Button('重置')
        .fontSize(11)
        .backgroundColor('#FF6666')
        .fontColor(Color.White)
        .margin({ top: 12 })
        .onClick(() => {
          this.autoResize = true
          this.imageWidth = 200
          this.imageHeight = 200
          this.currentPresetIndex = 0
          this.log = '已重置为默认配置'
        })
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 新增 060：fillColor 填充色矩阵 */
@Component
struct ImageFillColorScene {
  @State currentColorIndex: number = 0
  @State currentModeIndex: number = 0
  @State currentOpacityIndex: number = 0

  private readonly FILL_COLORS: Array<{ color: ResourceColor; label: string }> = [
    { color: Color.Red, label: '红' },
    { color: Color.Green, label: '绿' },
    { color: Color.Blue, label: '蓝' },
    { color: Color.Yellow, label: '黄' },
    { color: Color.Pink, label: '粉' },
    { color: Color.Purple, label: '紫' },
    { color: Color.Orange, label: '橙' },
    { color: Color.Cyan, label: '青' },
    { color: '#FF6B6B', label: '珊瑚红' },
    { color: '#4ECDC4', label: '薄荷' },
    { color: '#95E1D3', label: '薄荷淡' },
    { color: undefined, label: '无填充' }
  ]

  private readonly RENDER_MODES: Array<{ mode: ImageRenderMode; label: string }> = [
    { mode: ImageRenderMode.Original, label: 'Original' },
    { mode: ImageRenderMode.Template, label: 'Template' }
  ]

  private readonly OPACITIES: Array<{ value: number; label: string }> = [
    { value: 1.0, label: '100%' },
    { value: 0.8, label: '80%' },
    { value: 0.6, label: '60%' },
    { value: 0.4, label: '40%' },
    { value: 0.2, label: '20%' }
  ]

  build() {
    Column() {
      Text('Image fillColor填充色矩阵测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Image($r('app.media.startIcon'))
          .width(120)
          .height(120)
          .objectFit(ImageFit.Cover)
          .renderMode(this.RENDER_MODES[this.currentModeIndex].mode)
          .fillColor(this.FILL_COLORS[this.currentColorIndex].color)
          .opacity(this.OPACITIES[this.currentOpacityIndex].value)
          .border({ width: 2, color: Color.Blue })
          .borderRadius(8)
          .margin({ right: 16 })

        Column({ space: 6 }) {
          Text('当前配置')
            .fontSize(13)
            .fontWeight(FontWeight.Medium)
          Text(`颜色: ${this.FILL_COLORS[this.currentColorIndex].label}`)
            .fontSize(11)
            .fontColor(Color.Gray)
          Text(`模式: ${this.RENDER_MODES[this.currentModeIndex].label}`)
            .fontSize(11)
            .fontColor(this.RENDER_MODES[this.currentModeIndex].mode === ImageRenderMode.Template ?
                Color.Red : Color.Gray)
          Text(`透明度: ${this.OPACITIES[this.currentOpacityIndex].label}`)
            .fontSize(11)
            .fontColor(Color.Gray)
          Text('提示: Template模式下fillColor生效')
            .fontSize(9)
            .fontColor(Color.Orange)
        }
        .alignItems(HorizontalAlign.Start)
      }
      .margin({ bottom: 16 })

      Divider()

      Text('颜色选择(点击切换)')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Grid() {
        ForEach(this.FILL_COLORS, (config: { color: ResourceColor; label: string }, index: number) => {
          GridItem() {
            Column() {
              Column()
                .width(24)
                .height(24)
                .backgroundColor(config.color === undefined ? Color.Transparent : config.color)
                .borderRadius(4)
                .border({ width: 1, color: Color.Gray })
              Text(config.label)
                .fontSize(9)
                .margin({ top: 2 })
                .fontColor(index === this.currentColorIndex ? Color.Blue : Color.Gray)
            }
            .padding(4)
            .backgroundColor(index === this.currentColorIndex ? '#E3F2FD' : '#FAFAFA')
            .borderRadius(4)
            .onClick(() => { this.currentColorIndex = index })
          }
        })
      }
      .columnsTemplate('repeat(6, 1fr)')
      .rowsGap(6)
      .columnsGap(6)
      .padding(8)

      Divider()

      Text('渲染模式')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Row({ space: 8 }) {
        ForEach(this.RENDER_MODES, (config: { mode: ImageRenderMode; label: string }, index: number) => {
          Button(config.label)
            .fontSize(10)
            .onClick(() => { this.currentModeIndex = index })
            .backgroundColor(index === this.currentModeIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentModeIndex ? Color.White : Color.Gray)
        })
      }
      .margin({ bottom: 12 })

      Text('透明度')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 4, bottom: 8 })

      Row({ space: 8 }) {
        ForEach(this.OPACITIES, (config: { value: number; label: string }, index: number) => {
          Button(config.label)
            .fontSize(10)
            .onClick(() => { this.currentOpacityIndex = index })
            .backgroundColor(index === this.currentOpacityIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentOpacityIndex ? Color.White : Color.Gray)
        })
      }
      .margin({ bottom: 12 })

      Divider()

      Text('Template模式色卡矩阵(12色)')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Grid() {
        ForEach(this.FILL_COLORS, (config: { color: ResourceColor; label: string }) => {
          GridItem() {
            Image($r('app.media.startIcon'))
              .width(40)
              .height(40)
              .renderMode(ImageRenderMode.Template)
              .fillColor(config.color)
              .borderRadius(4)
              .border({ width: 1, color: '#EEEEEE' })
          }
        })
      }
      .columnsTemplate('repeat(6, 1fr)')
      .rowsGap(6)
      .columnsGap(6)
      .padding(8)
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

@Entry
@Component
struct ImageTestcaseAdvancedLoading {
  @State currentIndex: number = 0

  build() {
    Column() {
      Tabs({ barPosition: BarPosition.Start, index: this.currentIndex }) {
        TabContent() { ImageDynamicRangeScene() }
          .tabBar('HDR动态范围')
        TabContent() { ImageSourceSizeScene() }
          .tabBar('sourceSize')
        TabContent() { ImageAnalyzerPrivacyScene() }
          .tabBar('分析·隐私')
        TabContent() { ImageLoadCallbackScene() }
          .tabBar('加载回调')
        TabContent() { ImageSyncLoadScene() }
          .tabBar('syncLoad')
        TabContent() { ImageAutoResizeScene() }
          .tabBar('autoResize')
        TabContent() { ImageFillColorScene() }
          .tabBar('fillColor')
      }
      .barMode(BarMode.Scrollable)
      .scrollable(true)
      .onChange((index: number) => { this.currentIndex = index })
    }
    .width('100%')
    .height('100%')
  }
}
