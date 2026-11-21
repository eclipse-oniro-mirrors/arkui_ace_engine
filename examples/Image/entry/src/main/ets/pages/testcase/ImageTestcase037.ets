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
struct ImageComprehensiveTestCase035 {
  @State imageWidth: number = 120
  @State imageHeight: number = 120
  @State objectFit: ImageFit = ImageFit.Cover
  @State borderRadius: number = 8
  @State opacity: number = 1.0
  @State rotateAngle: number = 0
  @State scaleValue: number = 1.0
  @State fillColor: ResourceColor = undefined
  @State renderMode: ImageRenderMode = ImageRenderMode.Original
  @State interpolation: ImageInterpolation = ImageInterpolation.High
  @State dynamicRange: DynamicRangeMode = DynamicRangeMode.STANDARD
  @State syncLoad: boolean = false
  @State autoResize: boolean = true
  @State privacySensitive: boolean = false
  @State analyzerEnabled: boolean = false
  @State draggable: boolean = false

  private readonly PRESET_COMBOS: Array<{
    name: string
    width: number; height: number; fit: ImageFit; radius: number; opacity: number
    angle: number; scale: number; fill: ResourceColor; mode: ImageRenderMode
  }> = [
    { name: '默认', width: 120, height: 120, fit: ImageFit.Cover, radius: 0, opacity: 1,
      angle: 0, scale: 1, fill: undefined, mode: ImageRenderMode.Original },
    { name: '圆角缩放', width: 100, height: 100, fit: ImageFit.Cover, radius: 20, opacity: 1,
      angle: 0, scale: 1.2, fill: undefined, mode: ImageRenderMode.Original },
    { name: '模板填充', width: 120, height: 120, fit: ImageFit.Contain, radius: 8, opacity: 1,
      angle: 0, scale: 1, fill: Color.Red, mode: ImageRenderMode.Template },
    { name: '旋转透明', width: 100, height: 100, fit: ImageFit.Cover, radius: 0, opacity: 0.5,
      angle: 45, scale: 1, fill: undefined, mode: ImageRenderMode.Original },
    { name: 'HDR高清', width: 150, height: 150, fit: ImageFit.Cover, radius: 12, opacity: 1,
      angle: 0, scale: 1, fill: undefined, mode: ImageRenderMode.Original },
    { name: '全属性组合', width: 100, height: 100, fit: ImageFit.ScaleDown, radius: 16, opacity: 0.8,
      angle: 30, scale: 1.1, fill: Color.Blue, mode: ImageRenderMode.Template }
  ]

  build() {
    Column() {
      Text('Image综合属性组合测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Image($r('app.media.startIcon'))
        .width(this.imageWidth)
        .height(this.imageHeight)
        .objectFit(this.objectFit)
        .borderRadius(this.borderRadius)
        .opacity(this.opacity)
        .rotate({ angle: this.rotateAngle })
        .scale({ x: this.scaleValue, y: this.scaleValue })
        .fillColor(this.fillColor)
        .renderMode(this.renderMode)
        .interpolation(this.interpolation)
        .dynamicRangeMode(this.dynamicRange)
        .syncLoad(this.syncLoad)
        .autoResize(this.autoResize)
        .privacySensitive(this.privacySensitive)
        .enableAnalyzer(this.analyzerEnabled)
        .draggable(this.draggable)
        .border({ width: 2, color: Color.Blue })
        .margin({ bottom: 16 })

      Text('属性预设组合')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ bottom: 8 })

      Grid() {
        ForEach(this.PRESET_COMBOS, (combo: {
          name: string
          width: number; height: number; fit: ImageFit; radius: number; opacity: number
          angle: number; scale: number; fill: ResourceColor; mode: ImageRenderMode
        }) => {
          GridItem() {
            Column() {
              Text(combo.name)
                .fontSize(12)
                .fontWeight(FontWeight.Medium)
            }
            .padding(12)
            .backgroundColor('#E3F2FD')
            .borderRadius(6)
            .onClick(() => {
              this.imageWidth = combo.width
              this.imageHeight = combo.height
              this.objectFit = combo.fit
              this.borderRadius = combo.radius
              this.opacity = combo.opacity
              this.rotateAngle = combo.angle
              this.scaleValue = combo.scale
              this.fillColor = combo.fill
              this.renderMode = combo.mode
              if (combo.name === 'HDR高清') {
                this.dynamicRange = DynamicRangeMode.HIGH
                this.interpolation = ImageInterpolation.High
              } else {
                this.dynamicRange = DynamicRangeMode.STANDARD
                this.interpolation = ImageInterpolation.None
              }
            })
          }
        })
      }
      .columnsTemplate('repeat(3, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(16)

      Button('重置所有属性')
        .onClick(() => {
          this.imageWidth = 120
          this.imageHeight = 120
          this.objectFit = ImageFit.Cover
          this.borderRadius = 0
          this.opacity = 1.0
          this.rotateAngle = 0
          this.scaleValue = 1.0
          this.fillColor = undefined
          this.renderMode = ImageRenderMode.Original
          this.interpolation = ImageInterpolation.High
          this.dynamicRange = DynamicRangeMode.STANDARD
          this.syncLoad = false
          this.autoResize = true
          this.privacySensitive = false
          this.analyzerEnabled = false
          this.draggable = false
        })
        .backgroundColor('#FF6666')
        .margin({ top: 16 })
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 原 036：overlay 文字叠加 */
@Component
struct ImageOverlayTestCase036 {
  @State currentOverlayIndex: number = 0

  private readonly OVERLAY_CONFIGS: Array<{ text: string; align: Alignment; offsetX: number; offsetY: number }> = [
    { text: '顶部居中', align: Alignment.Top, offsetX: 0, offsetY: 10 },
    { text: '底部居中', align: Alignment.Bottom, offsetX: 0, offsetY: 10 },
    { text: '居中', align: Alignment.Center, offsetX: 0, offsetY: 0 },
    { text: '左上', align: Alignment.TopStart, offsetX: 10, offsetY: 10 },
    { text: '右上', align: Alignment.TopEnd, offsetX: -10, offsetY: 10 },
    { text: '左下', align: Alignment.BottomStart, offsetX: 10, offsetY: -10 },
    { text: '右下', align: Alignment.BottomEnd, offsetX: -10, offsetY: -10 },
    { text: '偏移大', align: Alignment.Center, offsetX: 50, offsetY: -30 }
  ]

  build() {
    Column() {
      Text('Image overlay属性测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Image($r('app.media.startIcon'))
          .width(200)
          .height(200)
          .overlay(
            this.OVERLAY_CONFIGS[this.currentOverlayIndex].text,
            {
              align: this.OVERLAY_CONFIGS[this.currentOverlayIndex].align,
              offset: {
                x: this.OVERLAY_CONFIGS[this.currentOverlayIndex].offsetX,
                y: this.OVERLAY_CONFIGS[this.currentOverlayIndex].offsetY
              }
            }
          )
          .objectFit(ImageFit.Cover)
          .border({ width: 2, color: Color.Blue })
          .borderRadius(8)

        Column() {
          Text('当前overlay配置')
            .fontSize(14)
            .fontWeight(FontWeight.Medium)
            .margin({ bottom: 8 })
          Text(`文字: ${this.OVERLAY_CONFIGS[this.currentOverlayIndex].text}`)
            .fontSize(12)
            .fontColor(Color.Gray)
          Text(`对齐: ${this.currentOverlayIndex}`)
            .fontSize(12)
            .fontColor(Color.Gray)
          Text('偏移: (' + this.OVERLAY_CONFIGS[this.currentOverlayIndex].offsetX + ', ' +
            this.OVERLAY_CONFIGS[this.currentOverlayIndex].offsetY + ')')
            .fontSize(12)
            .fontColor(Color.Gray)
        }
        .margin({ left: 16 })
      }
      .margin({ bottom: 16 })

      Grid() {
        ForEach(this.OVERLAY_CONFIGS, (config: {
          text: string; align: Alignment; offsetX: number; offsetY: number
        }, index: number) => {
          GridItem() {
            Column() {
              Text(config.text)
                .fontSize(12)
                .fontColor(index === this.currentOverlayIndex ? Color.Blue : Color.Gray)
            }
            .padding(10)
            .backgroundColor(index === this.currentOverlayIndex ? '#E3F2FD' : '#FAFAFA')
            .borderRadius(4)
            .onClick(() => {
              this.currentOverlayIndex = index
            })
          }
        })
      }
      .columnsTemplate('repeat(4, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(16)
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 原 039：边框样式组合 */
@Component
struct ImageBorderStyleTestCase039 {
  @State currentBorderIndex: number = 0

  private readonly BORDER_CONFIGS: Array<{
    width: Length; color: ResourceColor; style: BorderStyle; radius: number; label: string
  }> = [
    { width: 0, color: Color.Gray, style: BorderStyle.Solid, radius: 0, label: '无边框' },
    { width: 1, color: Color.Black, style: BorderStyle.Solid, radius: 0, label: '细实线' },
    { width: 3, color: Color.Blue, style: BorderStyle.Solid, radius: 8, label: '蓝实线圆角' },
    { width: 5, color: Color.Red, style: BorderStyle.Dashed, radius: 12, label: '红虚线圆角' },
    { width: 2, color: Color.Green, style: BorderStyle.Dotted, radius: 4, label: '绿点线' },
    { width: 8, color: Color.Orange, style: BorderStyle.Solid, radius: 20, label: '橙粗线大圆角' },
    { width: 4, color: '#800080', style: BorderStyle.Dashed, radius: 0, label: '紫虚线方角' },
    { width: 6, color: Color.Pink, style: BorderStyle.Dotted, radius: 16, label: '粉点线中圆角' }
  ]

  build() {
    Column() {
      Text('Image边框样式组合测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Image($r('app.media.startIcon'))
        .width(150)
        .height(150)
        .objectFit(ImageFit.Cover)
        .border({
          width: this.BORDER_CONFIGS[this.currentBorderIndex].width,
          color: this.BORDER_CONFIGS[this.currentBorderIndex].color,
          style: this.BORDER_CONFIGS[this.currentBorderIndex].style,
          radius: this.BORDER_CONFIGS[this.currentBorderIndex].radius
        })
        .margin({ bottom: 16 })

      Text(`当前配置: ${this.BORDER_CONFIGS[this.currentBorderIndex].label}`)
        .fontSize(14)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })

      Text(`width=${this.BORDER_CONFIGS[this.currentBorderIndex].width} ` +
        `color=${this.BORDER_CONFIGS[this.currentBorderIndex].color} ` +
        `style=${this.BORDER_CONFIGS[this.currentBorderIndex].style} ` +
        `radius=${this.BORDER_CONFIGS[this.currentBorderIndex].radius}`)
        .fontSize(11)
        .fontColor(Color.Gray)
        .margin({ bottom: 12 })

      Grid() {
        ForEach(this.BORDER_CONFIGS, (config: {
          width: Length; color: ResourceColor; style: BorderStyle; radius: number; label: string
        }, index: number) => {
          GridItem() {
            Column() {
              Image($r('app.media.startIcon'))
                .width(60)
                .height(60)
                .border({
                  width: config.width,
                  color: config.color,
                  style: config.style,
                  radius: config.radius
                })

              Text(config.label)
                .fontSize(10)
                .margin({ top: 4 })
                .fontColor(index === this.currentBorderIndex ? Color.Blue : Color.Gray)
            }
            .padding(8)
            .backgroundColor(index === this.currentBorderIndex ? '#E3F2FD' : '#FAFAFA')
            .borderRadius(4)
            .onClick(() => {
              this.currentBorderIndex = index
            })
          }
        })
      }
      .columnsTemplate('repeat(4, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(16)
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 原 040：margin/padding 布局 */
@Component
struct ImageMarginPaddingTestCase040 {
  @State currentLayoutIndex: number = 0

  private readonly LAYOUT_CONFIGS: Array<{
    margin: Margin | number; padding: Padding | number; label: string
  }> = [
    { margin: 0, padding: 0, label: '无间距' },
    { margin: 10, padding: 10, label: '均匀10' },
    { margin: 20, padding: 5, label: 'margin大' },
    { margin: 5, padding: 20, label: 'padding大' },
    { margin: { left: 30, right: 10, top: 10, bottom: 10 }, padding: 10, label: '左偏margin' },
    { margin: { left: 10, right: 30, top: 10, bottom: 10 }, padding: 10, label: '右偏margin' },
    { margin: { left: 10, right: 10, top: 30, bottom: 10 }, padding: 10, label: '上偏margin' },
    { margin: { left: 10, right: 10, top: 10, bottom: 30 }, padding: 10, label: '下偏margin' },
    { margin: 15, padding: { left: 20, right: 5, top: 5, bottom: 20 }, label: '不对称padding' },
    { margin: { left: 0, right: 0, top: 0, bottom: 40 }, padding: 0, label: '仅底部margin' }
  ]

  build() {
    Column() {
      Text('Image margin/padding布局测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Column() {
          Text('应用间距')
            .fontSize(14)
            .margin({ bottom: 8 })
          Image($r('app.media.startIcon'))
            .width(100)
            .height(100)
            .margin(this.LAYOUT_CONFIGS[this.currentLayoutIndex].margin)
            .padding(this.LAYOUT_CONFIGS[this.currentLayoutIndex].padding)
            .objectFit(ImageFit.Cover)
            .backgroundColor('#F0F0F0')
            .border({ width: 2, color: Color.Blue })
        }

        Column() {
          Text('参考(无间距)')
            .fontSize(14)
            .margin({ bottom: 8 })
          Image($r('app.media.startIcon'))
            .width(100)
            .height(100)
            .objectFit(ImageFit.Cover)
            .backgroundColor('#F5F5F5')
            .border({ width: 1, color: Color.Gray })
        }
      }
      .margin({ bottom: 16 })

      Text(`当前配置: ${this.LAYOUT_CONFIGS[this.currentLayoutIndex].label}`)
        .fontSize(14)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })

      Grid() {
        ForEach(this.LAYOUT_CONFIGS, (config: {
          margin: Margin | number; padding: Padding | number; label: string
        }, index: number) => {
          GridItem() {
            Column() {
              Text(config.label)
                .fontSize(11)
                .fontColor(index === this.currentLayoutIndex ? Color.Blue : Color.Gray)
            }
            .padding(8)
            .backgroundColor(index === this.currentLayoutIndex ? '#E3F2FD' : '#FAFAFA')
            .borderRadius(4)
            .onClick(() => {
              this.currentLayoutIndex = index
            })
          }
        })
      }
      .columnsTemplate('repeat(5, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(16)
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 原 041：backgroundColor 背景色 */
@Component
struct ImageBackgroundColorTestCase041 {
  @State currentBgIndex: number = 0

  private readonly BG_COLORS: Array<{ color: ResourceColor; label: string }> = [
    { color: Color.Transparent, label: '透明' },
    { color: '#F5F5F5', label: '浅灰' },
    { color: '#E0E0E0', label: '中灰' },
    { color: '#333333', label: '深灰' },
    { color: Color.White, label: '白色' },
    { color: Color.Black, label: '黑色' },
    { color: '#FF0000', label: '红色' },
    { color: '#0000FF', label: '蓝色' },
    { color: '#00FF00', label: '绿色' },
    { color: '#80008080', label: '半透明紫' }
  ]

  build() {
    Column() {
      Text('Image backgroundColor属性测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Column() {
          Text('Contain+背景色')
            .fontSize(14)
            .margin({ bottom: 8 })
          Image($r('app.media.startIcon'))
            .width(150)
            .height(150)
            .backgroundColor(this.BG_COLORS[this.currentBgIndex].color)
            .objectFit(ImageFit.Contain)
            .border({ width: 2, color: Color.Blue })
        }
        .margin({ right: 16 })

        Column() {
          Text('Cover+背景色')
            .fontSize(14)
            .margin({ bottom: 8 })
          Image($r('app.media.startIcon'))
            .width(150)
            .height(150)
            .backgroundColor(this.BG_COLORS[this.currentBgIndex].color)
            .objectFit(ImageFit.Cover)
            .border({ width: 2, color: Color.Gray })
        }
      }
      .margin({ bottom: 16 })

      Text(`当前背景色: ${this.BG_COLORS[this.currentBgIndex].label}`)
        .fontSize(14)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })

      Grid() {
        ForEach(this.BG_COLORS, (config: { color: ResourceColor; label: string }, index: number) => {
          GridItem() {
            Column() {
              Column()
                .width(30)
                .height(30)
                .backgroundColor(config.color)
                .borderRadius(4)
                .border({ width: 1, color: Color.Gray })

              Text(config.label)
                .fontSize(10)
                .margin({ top: 4 })
                .fontColor(index === this.currentBgIndex ? Color.Blue : Color.Gray)
            }
            .padding(8)
            .backgroundColor(index === this.currentBgIndex ? '#E3F2FD' : '#FAFAFA')
            .borderRadius(4)
            .onClick(() => {
              this.currentBgIndex = index
            })
          }
        })
      }
      .columnsTemplate('repeat(5, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(16)
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 原 044：宽高比和比例约束 */
@Component
struct ImageAspectRatioTestCase044 {
  @State currentRatioIndex: number = 0

  private readonly RATIO_CONFIGS: Array<{ ratio: number | undefined; label: string }> = [
    { ratio: undefined, label: '不约束' },
    { ratio: 1, label: '1:1 正方形' },
    { ratio: 1.5, label: '3:2 横向' },
    { ratio: 0.667, label: '2:3 纵向' },
    { ratio: 1.778, label: '16:9 宽屏' },
    { ratio: 0.5625, label: '9:16 竖屏' },
    { ratio: 1.333, label: '4:3 标准' },
    { ratio: 0.75, label: '3:4 反向' },
    { ratio: 2, label: '2:1 超宽' },
    { ratio: 0.5, label: '1:2 超高' }
  ]

  build() {
    Column() {
      Text('Image aspectRatio属性测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Column() {
          Text('有aspectRatio约束')
            .fontSize(14)
            .margin({ bottom: 8 })
          Image($r('app.media.startIcon'))
            .width(200)
            .aspectRatio(this.RATIO_CONFIGS[this.currentRatioIndex].ratio)
            .objectFit(ImageFit.Cover)
            .border({ width: 2, color: Color.Blue })
            .borderRadius(8)
        }
        .margin({ right: 16 })

        Column() {
          Text('无约束(原始)')
            .fontSize(14)
            .margin({ bottom: 8 })
          Image($r('app.media.startIcon'))
            .width(200)
            .height(150)
            .objectFit(ImageFit.Cover)
            .border({ width: 1, color: Color.Gray })
            .borderRadius(8)
        }
      }
      .margin({ bottom: 16 })

      Text(`当前比例: ${this.RATIO_CONFIGS[this.currentRatioIndex].label} ` +
        `(ratio=${this.RATIO_CONFIGS[this.currentRatioIndex].ratio})`)
        .fontSize(14)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })

      Grid() {
        ForEach(this.RATIO_CONFIGS, (config: { ratio: number | undefined; label: string }, index: number) => {
          GridItem() {
            Column() {
              Text(config.label)
                .fontSize(11)
                .fontColor(index === this.currentRatioIndex ? Color.Blue : Color.Gray)
            }
            .padding(8)
            .backgroundColor(index === this.currentRatioIndex ? '#E3F2FD' : '#FAFAFA')
            .borderRadius(4)
            .onClick(() => {
              this.currentRatioIndex = index
            })
          }
        })
      }
      .columnsTemplate('repeat(5, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(16)
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

@Component
struct ImageTestcaseStyleLayout {
  @State currentIndex: number = 0

  build() {
    Column() {
      Tabs({ barPosition: BarPosition.Start, index: this.currentIndex }) {
        TabContent() { ImageComprehensiveTestCase035() }
          .tabBar('综合属性')
        TabContent() { ImageOverlayTestCase036() }
          .tabBar('overlay')
        TabContent() { ImageBorderStyleTestCase039() }
          .tabBar('边框样式')
        TabContent() { ImageMarginPaddingTestCase040() }
          .tabBar('margin/padding')
        TabContent() { ImageBackgroundColorTestCase041() }
          .tabBar('背景色')
        TabContent() { ImageAspectRatioTestCase044() }
          .tabBar('宽高比')
      }
      .barMode(BarMode.Scrollable)
      .scrollable(true)
      .onChange((index: number) => { this.currentIndex = index })
    }
    .width('100%')
    .height('100%')
  }
}

/**
 * Image 容器场景合集（由原 047/048/049/052/053 合并）
 * 覆盖：List 列表 / Grid 网格 / Stack 层叠 / Swiper 轮播 / WaterFlow 瀑布流
 */

/** 053 用到的瀑布流数据源 */
class WaterFlowDataSource implements IDataSource {
  private items: Array<number> = []
  private listeners: DataChangeListener[] = []

  constructor(count: number) {
    for (let i = 0; i < count; i++) {
      this.items.push(this.genHeight(i))
    }
  }

  private genHeight(idx: number): number {
    const heights = [120, 180, 240, 160, 200, 140, 220, 100, 260, 180]
    return heights[idx % heights.length]
  }

  totalCount(): number { return this.items.length }

  getData(idx: number): number { return this.items[idx] }

  registerDataChangeListener(listener: DataChangeListener): void {
    this.listeners.push(listener)
  }

  unregisterDataChangeListener(listener: DataChangeListener): void {
    this.listeners = this.listeners.filter(l => l !== listener)
  }

  append(count: number): void {
    const start = this.items.length
    for (let i = 0; i < count; i++) {
      this.items.push(this.genHeight(start + i))
    }
    this.listeners.forEach(l => l.onDataReloaded())
  }

  clear(): void {
    this.items = []
    this.listeners.forEach(l => l.onDataReloaded())
  }
}

/** 原 047：List 容器 */
@Component
struct ImageInListTestCase047 {
  @State itemCount: number = 30
  @State lazyLoadEnabled: boolean = true

  build() {
    Column() {
      Row() {
        Text('Image在List容器中测试')
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
        Blank()
        Text(`条目数: ${this.itemCount}`)
          .fontSize(14)
          .fontColor(Color.Gray)
      }
      .width('100%')
      .padding(16)

      Row({ space: 12 }) {
        Button('增加10条')
          .onClick(() => { this.itemCount += 10 })
        Button('减少10条')
          .onClick(() => { this.itemCount = Math.max(5, this.itemCount - 10) })
        Button(`懒加载: ${this.lazyLoadEnabled ? '开' : '关'}`)
          .onClick(() => { this.lazyLoadEnabled = !this.lazyLoadEnabled })
      }
      .padding({ left: 16, right: 16, bottom: 8 })

      List({ space: 8 }) {
        ForEach(Array.from({ length: this.itemCount }, (_, i) => i), (index: number) => {
          ListItem() {
            Row() {
              Image($r('app.media.startIcon'))
                .width(60)
                .height(60)
                .objectFit(ImageFit.Cover)
                .borderRadius(8)
                .syncLoad(!this.lazyLoadEnabled)
                .border({ width: 1, color: Color.Gray })

              Column() {
                Text(`图片条目 ${index + 1}`)
                  .fontSize(16)
                  .fontWeight(FontWeight.Medium)
                Text('这是List容器中的图片测试项')
                  .fontSize(12)
                  .fontColor(Color.Gray)
                  .margin({ top: 4 })
              }
              .margin({ left: 12 })
              .layoutWeight(1)
            }
            .width('100%')
            .padding(12)
            .backgroundColor(Color.White)
            .borderRadius(8)
          }
        })
      }
      .layoutWeight(1)
      .padding({ left: 16, right: 16 })
      .edgeEffect(EdgeEffect.Spring)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#F5F5F5')
  }
}

/** 原 048：Grid 容器 */
@Component
struct ImageInGridTestCase048 {
  @State columnsCount: number = 3
  @State objectFit: ImageFit = ImageFit.Cover
  @State borderRadius: number = 8
  @State imageCount: number = 12

  private readonly GRID_IMAGES: Resource[] = [
    $r('app.media.startIcon'),
    $r('app.media.background'),
    $r('app.media.foreground'),
    $r('app.media.startIcon'),
    $r('app.media.background'),
    $r('app.media.foreground'),
    $r('app.media.startIcon'),
    $r('app.media.background'),
    $r('app.media.foreground'),
    $r('app.media.startIcon'),
    $r('app.media.background'),
    $r('app.media.foreground')
  ]

  build() {
    Column() {
      Row() {
        Text('Image在Grid容器中测试')
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
        Blank()
        Text(`${this.columnsCount}列 | ${this.imageCount}图`)
          .fontSize(14)
          .fontColor(Color.Gray)
      }
      .width('100%')
      .padding(16)

      Row({ space: 8 }) {
        Button('2列')
          .onClick(() => { this.columnsCount = 2 })
          .fontSize(11)
        Button('3列')
          .onClick(() => { this.columnsCount = 3 })
          .fontSize(11)
        Button('4列')
          .onClick(() => { this.columnsCount = 4 })
          .fontSize(11)
        Button('5列')
          .onClick(() => { this.columnsCount = 5 })
          .fontSize(11)
      }
      .padding({ left: 16, right: 16, bottom: 8 })

      Row({ space: 8 }) {
        Button('Cover')
          .onClick(() => { this.objectFit = ImageFit.Cover })
          .fontSize(11)
        Button('Contain')
          .onClick(() => { this.objectFit = ImageFit.Contain })
          .fontSize(11)
        Button('Fill')
          .onClick(() => { this.objectFit = ImageFit.Fill })
          .fontSize(11)
        Button('圆角0')
          .onClick(() => { this.borderRadius = 0 })
          .fontSize(11)
        Button('圆角12')
          .onClick(() => { this.borderRadius = 12 })
          .fontSize(11)
      }
      .padding({ left: 16, right: 16, bottom: 8 })

      Grid() {
        ForEach(this.GRID_IMAGES.slice(0, this.imageCount), (img: Resource, index: number) => {
          GridItem() {
            Column() {
              Image(img)
                .width('100%')
                .height(120)
                .objectFit(this.objectFit)
                .borderRadius(this.borderRadius)
                .border({ width: 1, color: Color.Gray })

              Text(`图${index + 1}`)
                .fontSize(12)
                .fontColor(Color.Gray)
                .margin({ top: 4 })
            }
            .padding(4)
          }
        })
      }
      .columnsTemplate(`repeat(${this.columnsCount}, 1fr)`)
      .rowsGap(8)
      .columnsGap(8)
      .padding(16)
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#F5F5F5')
  }
}

/** 原 049：Stack 容器层叠 */
@Component
struct ImageInStackTestCase049 {
  @State currentLayoutIndex: number = 0

  private readonly STACK_CONFIGS: Array<{ name: string; zOrders: number[] }> = [
    { name: '三图层叠(0,1,2)', zOrders: [0, 1, 2] },
    { name: '三图反转(2,1,0)', zOrders: [2, 1, 0] },
    { name: '等z序(0,0,0)', zOrders: [0, 0, 0] },
    { name: '中间突出(0,2,0)', zOrders: [0, 2, 0] }
  ]

  build() {
    Column() {
      Text('Image在Stack容器中测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Stack() {
        Image($r('app.media.startIcon'))
          .width(120)
          .height(120)
          .zIndex(this.STACK_CONFIGS[this.currentLayoutIndex].zOrders[0])
          .border({ width: 3, color: Color.Red })
          .borderRadius(8)

        Image($r('app.media.background'))
          .width(80)
          .height(80)
          .zIndex(this.STACK_CONFIGS[this.currentLayoutIndex].zOrders[1])
          .border({ width: 3, color: Color.Blue })
          .borderRadius(8)
          .position({ x: 20, y: 20 })

        Image($r('app.media.foreground'))
          .width(40)
          .height(40)
          .zIndex(this.STACK_CONFIGS[this.currentLayoutIndex].zOrders[2])
          .border({ width: 3, color: Color.Green })
          .borderRadius(8)
          .position({ x: 40, y: 40 })
      }
      .width(160)
      .height(160)
      .margin({ bottom: 16 })

      Text(`当前布局: ${this.STACK_CONFIGS[this.currentLayoutIndex].name}`)
        .fontSize(14)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })

      Row({ space: 12 }) {
        ForEach(this.STACK_CONFIGS, (config: { name: string; zOrders: number[] }, index: number) => {
          Button(config.name)
            .fontSize(10)
            .onClick(() => { this.currentLayoutIndex = index })
            .backgroundColor(index === this.currentLayoutIndex ? '#2196F3' : '#EEEEEE')
        })
      }

      Divider()
        .margin({ top: 16, bottom: 16 })

      Text('叠加透明度演示')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ bottom: 8 })

      Stack() {
        Image($r('app.media.startIcon'))
          .width(100)
          .height(100)
          .opacity(1)

        Image($r('app.media.background'))
          .width(100)
          .height(100)
          .opacity(0.7)
          .blendMode(BlendMode.SRC_OVER)
      }
      .width(100)
      .height(100)
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 原 052：Swiper 轮播 */
@Component
struct ImageSwiperTestCase052 {
  @State autoPlay: boolean = true
  @State loop: boolean = true
  @State vertical: boolean = false
  @State currentIndex: number = 0
  @State interval: number = 2000

  private sources: Array<Resource> = [
    $r('app.media.startIcon'),
    $r('app.media.background'),
    $r('app.media.foreground')
  ]

  build() {
    Column({ space: 12 }) {
      Text('Image Swiper轮播测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)

      Swiper() {
        ForEach(this.sources, (src: Resource, idx: number) => {
          Image(src)
            .width('100%')
            .height(220)
            .objectFit(ImageFit.Cover)
            .borderRadius(12)
        }, (src: Resource) => `${src.id}`)
      }
      .index(this.currentIndex)
      .indicator(this.vertical ? Indicator.dot().right(8) : Indicator.dot().bottom(8))
      .autoPlay(this.autoPlay)
      .interval(this.interval)
      .loop(this.loop)
      .vertical(this.vertical)
      .duration(400)
      .onChange((index: number) => { this.currentIndex = index })
      .width('100%')
      .height(240)

      Text(`当前第 ${this.currentIndex + 1} / ${this.sources.length} 张`)
        .fontSize(13)
        .fontColor(Color.Gray)

      Row({ space: 8 }) {
        Button(this.autoPlay ? '自动播放:开' : '自动播放:关')
          .fontSize(11)
          .onClick(() => { this.autoPlay = !this.autoPlay })
        Button(this.loop ? '循环:开' : '循环:关')
          .fontSize(11)
          .onClick(() => { this.loop = !this.loop })
        Button(this.vertical ? '方向:竖' : '方向:横')
          .fontSize(11)
          .onClick(() => { this.vertical = !this.vertical })
      }

      Row({ space: 8 }) {
        Button('上一张')
          .fontSize(11)
          .onClick(() => { this.currentIndex = (this.currentIndex - 1 + this.sources.length) % this.sources.length })
        Button('下一张')
          .fontSize(11)
          .onClick(() => { this.currentIndex = (this.currentIndex + 1) % this.sources.length })
        Button('加速')
          .fontSize(11)
          .onClick(() => { this.interval = Math.max(500, this.interval - 500) })
        Button('减速')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .onClick(() => { this.interval = Math.min(6000, this.interval + 500) })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 原 053：WaterFlow 瀑布流 */
@Component
struct ImageWaterFlowTestCase053 {
  @State columns: number = 2
  @State scrolledIndex: number = -1
  private data: WaterFlowDataSource = new WaterFlowDataSource(20)

  build() {
    Column({ space: 12 }) {
      Text('Image WaterFlow瀑布流测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)

      Row({ space: 8 }) {
        Button('2列')
          .fontSize(11)
          .backgroundColor(this.columns === 2 ? '#007DFF' : '#E0E0E0')
          .onClick(() => { this.columns = 2 })
        Button('3列')
          .fontSize(11)
          .backgroundColor(this.columns === 3 ? '#007DFF' : '#E0E0E0')
          .onClick(() => { this.columns = 3 })
        Button('追加10')
          .fontSize(11)
          .onClick(() => { this.data.append(10) })
        Button('清空')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .onClick(() => { this.data.clear() })
      }

      WaterFlow() {
        LazyForEach(this.data, (height: number, idx: number) => {
          FlowItem() {
            Column() {
              Image($r('app.media.startIcon'))
                .width('100%')
                .height(height)
                .objectFit(ImageFit.Cover)
                .borderRadius(8)
              Text(`#${idx + 1}  高${height}`)
                .fontSize(10)
                .fontColor(Color.Gray)
                .margin({ top: 4 })
            }
          }
        }, (height: number) => `${height}`)
      }
      .columnsTemplate(this.columns === 2 ? '1fr 1fr' : '1fr 1fr 1fr')
      .columnsGap(8)
      .rowsGap(8)
      .padding(4)
      .layoutWeight(1)
      .onScrollIndex((first: number, last: number) => { this.scrolledIndex = first })
      .backgroundColor('#F5F5F5')

      Text(`可视起始项: ${this.scrolledIndex < 0 ? '-' : this.scrolledIndex + 1}`)
        .fontSize(12)
        .fontColor(Color.Gray)
    }
    .width('100%')
    .height('100%')
    .padding(12)
  }
}

@Component
struct ImageTestcaseContainers {
  @State currentIndex: number = 0

  build() {
    Column() {
      Tabs({ barPosition: BarPosition.Start, index: this.currentIndex }) {
        TabContent() { ImageInListTestCase047() }
          .tabBar('List')
        TabContent() { ImageInGridTestCase048() }
          .tabBar('Grid')
        TabContent() { ImageInStackTestCase049() }
          .tabBar('Stack')
        TabContent() { ImageSwiperTestCase052() }
          .tabBar('Swiper')
        TabContent() { ImageWaterFlowTestCase053() }
          .tabBar('WaterFlow')
      }
      .barMode(BarMode.Scrollable)
      .scrollable(true)
      .onChange((index: number) => { this.currentIndex = index })
    }
    .width('100%')
    .height('100%')
  }
}

/**
 * Image 响应式布局合集
 * 覆盖：尺寸单位(px/vp/%) / GridRow 栅格断点 / 可见区域 onVisibleAreaChange
 */

/** 尺寸单位对比 */
@Component
struct ImageSizeUnitsScene {
  build() {
    Column({ space: 12 }) {
      Text('Image 尺寸单位对比测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)

      Row({ space: 16 }) {
        Column({ space: 6 }) {
          Image($r('app.media.startIcon'))
            .width('100px')
            .height('100px')
            .objectFit(ImageFit.Cover)
            .border({ width: 1, color: Color.Red })
            .borderRadius(8)
          Text('100px')
            .fontSize(11)
            .fontColor(Color.Red)
        }

        Column({ space: 6 }) {
          Image($r('app.media.startIcon'))
            .width('100vp')
            .height('100vp')
            .objectFit(ImageFit.Cover)
            .border({ width: 1, color: Color.Blue })
            .borderRadius(8)
          Text('100vp')
            .fontSize(11)
            .fontColor(Color.Blue)
        }

        Column({ space: 6 }) {
          Image($r('app.media.startIcon'))
            .width('50%')
            .height(100)
            .objectFit(ImageFit.Cover)
            .border({ width: 1, color: Color.Green })
            .borderRadius(8)
          Text('50%(宽度)')
            .fontSize(11)
            .fontColor(Color.Green)
        }
      }
      .width('100%')
      .justifyContent(FlexAlign.Center)

      Text('px 为物理像素,vp 随屏幕密度缩放,% 随父容器宽度变化')
        .fontSize(11)
        .fontColor(Color.Orange)
        .margin({ top: 8 })
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** GridRow 栅格断点 */
@Component
struct ImageGridBreakpointScene {
  @State currentBp: string = 'unknown'
  private readonly COLORS: Color[] = [
    Color.Red, Color.Orange, Color.Yellow, Color.Green,
    Color.Blue, Color.Pink, Color.Brown, Color.Gray
  ]

  build() {
    Column({ space: 12 }) {
      Text('Image GridRow 栅格断点测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)

      Text(`当前断点: ${this.currentBp}`)
        .fontSize(13)
        .fontColor(Color.Orange)

      Text('调整窗口宽度观察列数变化(xs/sm/md/lg)')
        .fontSize(11)
        .fontColor(Color.Gray)

      GridRow({
        columns: 4,
        gutter: { x: 8, y: 8 },
        breakpoints: {
          value: ['320vp', '520vp', '840vp'],
          reference: BreakpointsReference.WindowSize
        },
        direction: GridRowDirection.Row
      }) {
        ForEach(this.COLORS, (color: Color) => {
          GridCol({ span: { xs: 2, sm: 2, md: 1, lg: 1 } }) {
            Image($r('app.media.startIcon'))
              .width('100%')
              .height(80)
              .objectFit(ImageFit.Cover)
              .borderRadius(8)
              .border({ width: 2, color: color })
          }
        })
      }
      .width('100%')
      .onBreakpointChange((breakpoint: string) => {
        this.currentBp = breakpoint
      })
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 可见区域 onVisibleAreaChange */
@Component
struct ImageVisibleAreaScene {
  @State ratio: number = 0
  @State isVisible: boolean = false
  @State changeCount: number = 0

  build() {
    Column({ space: 12 }) {
      Text('Image onVisibleAreaChange 测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)

      Text(`可见比例: ${(this.ratio * 100).toFixed(0)}%  可见: ${this.isVisible ? '是' : '否'}`)
        .fontSize(13)
        .fontColor(Color.Orange)
      Text(`变化次数: ${this.changeCount}`)
        .fontSize(12)
        .fontColor(Color.Gray)

      Scroll() {
        Column({ space: 16 }) {
          Text('向下滚动以改变下方图片的可见区域比例')
            .fontSize(13)
            .fontColor(Color.Gray)
            .height(360)
            .textAlign(TextAlign.Center)
            .width('100%')

          Image($r('app.media.startIcon'))
            .width('100%')
            .height(200)
            .objectFit(ImageFit.Cover)
            .borderRadius(8)
            .border({ width: 2, color: Color.Blue })
            .onVisibleAreaChange([0, 0.25, 0.5, 0.75, 1],
              (isVisible: boolean, currentRatio: number) => {
                this.isVisible = isVisible
                this.ratio = currentRatio
                this.changeCount++
              })

          Text('继续滚动可让图片离开可视区')
            .fontSize(13)
            .fontColor(Color.Gray)
            .height(360)
            .textAlign(TextAlign.Center)
            .width('100%')
        }
      }
      .width('100%')
      .layoutWeight(1)
      .edgeEffect(EdgeEffect.Spring)
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

@Component
struct ImageTestcaseResponsiveLayout {
  @State currentIndex: number = 0

  build() {
    Column() {
      Tabs({ barPosition: BarPosition.Start, index: this.currentIndex }) {
        TabContent() { ImageSizeUnitsScene() }
          .tabBar('尺寸单位')
        TabContent() { ImageGridBreakpointScene() }
          .tabBar('栅格断点')
        TabContent() { ImageVisibleAreaScene() }
          .tabBar('可见区域')
      }
      .barMode(BarMode.Scrollable)
      .scrollable(true)
      .onChange((index: number) => { this.currentIndex = index })
    }
    .width('100%')
    .height('100%')
  }
}

@Entry
@Component
struct ImageTestCategoryStyleLayout {
  @State currentIndex: number = 0

  build() {
    Column() {
      Tabs({ barPosition: BarPosition.Start, index: this.currentIndex }) {
        TabContent() { ImageTestcaseStyleLayout() }
          .tabBar('样式布局')
        TabContent() { ImageTestcaseContainers() }
          .tabBar('容器场景')
        TabContent() { ImageTestcaseResponsiveLayout() }
          .tabBar('响应式布局')
      }
      .barMode(BarMode.Scrollable)
      .scrollable(true)
      .onChange((index: number) => { this.currentIndex = index })
    }
    .width('100%')
    .height('100%')
  }
}
