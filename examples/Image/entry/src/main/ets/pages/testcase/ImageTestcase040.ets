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


/** 整合自: ImageTestcase063.ets, ImageTestcase064.ets */

@Entry
@Component
struct ImageColorFilterTestCase063 {
  @State currentFilterIndex: number = 0
  @State opacityValue: number = 1.0

  private readonly COLOR_FILTERS: Array<{
    name: string
    matrix: number[]
    desc: string
  }> = [
    {
      name: '原图(恒等)',
      matrix: [1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0],
      desc: '不改变颜色'
    },
    {
      name: '灰度',
      matrix: [0.299, 0.587, 0.114, 0, 0, 0.299, 0.587, 0.114, 0, 0, 0.299, 0.587, 0.114, 0, 0, 0, 0, 0, 1, 0],
      desc: '按亮度系数转为灰度'
    },
    {
      name: '反色',
      matrix: [-1, 0, 0, 0, 1, 0, -1, 0, 0, 1, 0, 0, -1, 0, 1, 0, 0, 0, 1, 0],
      desc: '颜色取反'
    },
    {
      name: '红色增强',
      matrix: [2, 0, 0, 0, 0, 0, 0.5, 0, 0, 0, 0, 0, 0.5, 0, 0, 0, 0, 0, 1, 0],
      desc: '增强红色通道'
    },
    {
      name: '绿色增强',
      matrix: [0.5, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0.5, 0, 0, 0, 0, 0, 1, 0],
      desc: '增强绿色通道'
    },
    {
      name: '蓝色增强',
      matrix: [0.5, 0, 0, 0, 0, 0, 0.5, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1, 0],
      desc: '增强蓝色通道'
    },
    {
      name: '暖色调',
      matrix: [1.2, 0, 0, 0, 0.05, 0, 1.0, 0, 0, 0, 0, 0, 0.8, 0, 0, 0, 0, 0, 1, 0],
      desc: '偏红偏黄'
    },
    {
      name: '冷色调',
      matrix: [0.8, 0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 0, 1.2, 0, 0.05, 0, 0, 0, 1, 0],
      desc: '偏蓝'
    },
    {
      name: '高对比',
      matrix: [1.5, 0, 0, 0, -0.25, 0, 1.5, 0, 0, -0.25, 0, 0, 1.5, 0, -0.25, 0, 0, 0, 1, 0],
      desc: '增强对比度'
    },
    {
      name: '复古',
      matrix: [0.9, 0.5, 0.1, 0, 0, 0.3, 0.7, 0.1, 0, 0, 0.2, 0.3, 0.5, 0, 0, 0, 0, 0, 1, 0],
      desc: '怀旧棕色调'
    }
  ]

  build() {
    Column() {
      Text('Image colorFilter颜色滤镜测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Column() {
          Text('原图')
            .fontSize(11)
            .fontColor(Color.Gray)
            .margin({ bottom: 8 })
          Image($r('app.media.startIcon'))
            .width(100)
            .height(100)
            .objectFit(ImageFit.Cover)
            .border({ width: 1, color: Color.Gray })
            .borderRadius(8)
        }
        .margin({ right: 16 })

        Column() {
          Text(`滤镜: ${this.COLOR_FILTERS[this.currentFilterIndex].name}`)
            .fontSize(11)
            .fontColor(Color.Blue)
            .margin({ bottom: 8 })
          Image($r('app.media.startIcon'))
            .width(100)
            .height(100)
            .objectFit(ImageFit.Cover)
            .colorFilter(this.COLOR_FILTERS[this.currentFilterIndex].matrix)
            .opacity(this.opacityValue)
            .border({ width: 2, color: Color.Blue })
            .borderRadius(8)
        }
      }
      .margin({ bottom: 16 })

      Text(this.COLOR_FILTERS[this.currentFilterIndex].desc)
        .fontSize(12)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })

      Text(`矩阵: [${this.COLOR_FILTERS[this.currentFilterIndex].matrix.join(', ')}]`)
        .fontSize(9)
        .fontColor(Color.Gray)
        .margin({ bottom: 12 })

      Divider()

      Text('滤镜选择(点击切换)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row() {
        ForEach(this.COLOR_FILTERS, (config: { name: string; matrix: number[]; desc: string }, index: number) => {
          Button(config.name)
            .fontSize(10)
            .onClick(() => { this.currentFilterIndex = index })
            .backgroundColor(index === this.currentFilterIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentFilterIndex ? Color.White : Color.Gray)
            .margin({ left: 4, right: 4 })
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('全滤镜对比矩阵')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Grid() {
        ForEach(this.COLOR_FILTERS, (config: { name: string; matrix: number[]; desc: string }, index: number) => {
          GridItem() {
            Column() {
              Image($r('app.media.startIcon'))
                .width(60)
                .height(60)
                .objectFit(ImageFit.Cover)
                .colorFilter(config.matrix)
                .borderRadius(4)
                .border({ width: 1, color: index === this.currentFilterIndex ? Color.Blue : '#EEEEEE' })

              Text(config.name)
                .fontSize(9)
                .fontColor(index === this.currentFilterIndex ? Color.Blue : Color.Gray)
                .margin({ top: 4 })
                .maxLines(1)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
            }
            .onClick(() => { this.currentFilterIndex = index })
          }
        })
      }
      .columnsTemplate('repeat(5, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(8)

      Divider()

      Text('透明度叠加')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Row({ space: 8 }) {
        Button('100%')
          .fontSize(10)
          .onClick(() => { this.opacityValue = 1.0 })
          .backgroundColor(this.opacityValue === 1.0 ? '#2196F3' : '#EEEEEE')
          .fontColor(this.opacityValue === 1.0 ? Color.White : Color.Gray)
        Button('60%')
          .fontSize(10)
          .onClick(() => { this.opacityValue = 0.6 })
          .backgroundColor(this.opacityValue === 0.6 ? '#2196F3' : '#EEEEEE')
          .fontColor(this.opacityValue === 0.6 ? Color.White : Color.Gray)
        Button('30%')
          .fontSize(10)
          .onClick(() => { this.opacityValue = 0.3 })
          .backgroundColor(this.opacityValue === 0.3 ? '#2196F3' : '#EEEEEE')
          .fontColor(this.opacityValue === 0.3 ? Color.White : Color.Gray)
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

@Component
struct ImageResizableTestCase064 {
  @State currentPresetIndex: number = 0
  @State imageWidth: number = 200
  @State imageHeight: number = 200
  @State showOriginal: boolean = true
  @State showResized: boolean = true

  private readonly SLICE_PRESETS: Array<{
    name: string
    slice: { left: Length; top: Length; right: Length; bottom: Length }
    desc: string
  }> = [
    {
      name: '小边距(10)',
      slice: { left: 10, top: 10, right: 10, bottom: 10 },
      desc: '四边各10vp切片'
    },
    {
      name: '中边距(30)',
      slice: { left: 30, top: 30, right: 30, bottom: 30 },
      desc: '四边各30vp切片'
    },
    {
      name: '大边距(60)',
      slice: { left: 60, top: 60, right: 60, bottom: 60 },
      desc: '四边各60vp切片'
    },
    {
      name: '非对称',
      slice: { left: 50, top: 20, right: 50, bottom: 20 },
      desc: '左右50,上下20'
    },
    {
      name: '极小(5)',
      slice: { left: 5, top: 5, right: 5, bottom: 5 },
      desc: '四边各5vp'
    },
    {
      name: '极大(80)',
      slice: { left: 80, top: 80, right: 80, bottom: 80 },
      desc: '四边各80vp'
    }
  ]

  build() {
    Column() {
      Text('Image resizable九宫格切片测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        if (this.showOriginal) {
          Column() {
            Text('原图(无resizable)')
              .fontSize(12)
              .fontColor(Color.Gray)
              .margin({ bottom: 8 })
            Image($r('app.media.background'))
              .width(this.imageWidth)
              .height(this.imageHeight)
              .objectFit(ImageFit.Cover)
              .border({ width: 1, color: Color.Gray })
              .borderRadius(8)
          }
          .margin({ right: 16 })
        }

        if (this.showResized) {
          Column() {
            Text(`resizable: ${this.SLICE_PRESETS[this.currentPresetIndex].name}`)
              .fontSize(12)
              .fontColor(Color.Blue)
              .margin({ bottom: 8 })
            Image($r('app.media.background'))
              .width(this.imageWidth)
              .height(this.imageHeight)
              .objectFit(ImageFit.Cover)
              .resizable({
                slice: this.SLICE_PRESETS[this.currentPresetIndex].slice
              })
              .border({ width: 2, color: Color.Blue })
              .borderRadius(8)
          }
        }
      }
      .margin({ bottom: 16 })

      Text(this.SLICE_PRESETS[this.currentPresetIndex].desc)
        .fontSize(12)
        .fontColor(Color.Gray)
        .margin({ bottom: 12 })

      Text(`显示尺寸: ${this.imageWidth}x${this.imageHeight}`)
        .fontSize(11)
        .fontColor(Color.Orange)
        .margin({ bottom: 12 })

      Divider()

      Text('切片预设')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row() {
        ForEach(this.SLICE_PRESETS, (config: {
          name: string
          slice: { left: Length; top: Length; right: Length; bottom: Length }
          desc: string
        }, index: number) => {
          Button(config.name)
            .fontSize(10)
            .onClick(() => { this.currentPresetIndex = index })
            .backgroundColor(index === this.currentPresetIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentPresetIndex ? Color.White : Color.Gray)
            .margin({ left: 4, right: 4 })
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('显示尺寸(观察拉伸效果)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 8 }) {
        Button('80x80')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 80; this.imageHeight = 80 })
        Button('200x200')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 200; this.imageHeight = 200 })
        Button('300x120')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 300; this.imageHeight = 120 })
        Button('120x300')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 120; this.imageHeight = 300 })
      }
      .margin({ bottom: 12 })

      Row({ space: 12 }) {
        Button(this.showOriginal ? '原图: 显' : '原图: 隐')
          .fontSize(10)
          .onClick(() => { this.showOriginal = !this.showOriginal })
          .backgroundColor(this.showOriginal ? '#4CAF50' : '#EEEEEE')
          .fontColor(this.showOriginal ? Color.White : Color.Gray)
        Button(this.showResized ? '切片: 显' : '切片: 隐')
          .fontSize(10)
          .onClick(() => { this.showResized = !this.showResized })
          .backgroundColor(this.showResized ? '#4CAF50' : '#EEEEEE')
          .fontColor(this.showResized ? Color.White : Color.Gray)
        Button('重置')
          .fontSize(10)
          .backgroundColor('#FF6666')
          .fontColor(Color.White)
          .onClick(() => {
            this.currentPresetIndex = 0
            this.imageWidth = 200
            this.imageHeight = 200
            this.showOriginal = true
            this.showResized = true
          })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}
