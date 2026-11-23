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


/** 整合自: ImageTestcase065.ets, ImageTestcase066.ets */

@Entry
@Component
struct ImageOrientationAntialiasingTestCase065 {
  @State currentOrientationIndex: number = 0
  @State antialiasingEnabled: boolean = true
  @State imageWidth: number = 120
  @State imageHeight: number = 120

  private readonly ORIENTATIONS: Array<{ orientation: ImageRotateOrientation; label: string; desc: string }> = [
    { orientation: ImageRotateOrientation.UP, label: 'UP(默认)', desc: '0度,原始方向' },
    { orientation: ImageRotateOrientation.DOWN, label: 'DOWN(180)', desc: '180度旋转' },
    { orientation: ImageRotateOrientation.LEFT, label: 'LEFT(90)', desc: '逆时针90度' },
    { orientation: ImageRotateOrientation.RIGHT, label: 'RIGHT(270)', desc: '顺时针90度' },
    { orientation: ImageRotateOrientation.UP_MIRRORED, label: 'UP_MIRRORED', desc: '水平镜像后0度' },
    { orientation: ImageRotateOrientation.DOWN_MIRRORED, label: 'DOWN_MIRRORED', desc: '水平镜像后180度' },
    { orientation: ImageRotateOrientation.LEFT_MIRRORED, label: 'LEFT_MIRRORED', desc: '水平镜像后逆时针90度' },
    { orientation: ImageRotateOrientation.RIGHT_MIRRORED, label: 'RIGHT_MIRRORED', desc: '水平镜像后顺时针90度' }
  ]

  build() {
    Column() {
      Text('Image orientation与edgeAntialiasing测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Column() {
          Text('原图(无orientation)')
            .fontSize(11)
            .fontColor(Color.Gray)
            .margin({ bottom: 8 })
          Image($r('app.media.startIcon'))
            .width(this.imageWidth)
            .height(this.imageHeight)
            .objectFit(ImageFit.Cover)
            .border({ width: 1, color: Color.Gray })
            .borderRadius(8)
        }
        .margin({ right: 16 })

        Column() {
          Text(`orientation: ${this.ORIENTATIONS[this.currentOrientationIndex].label}`)
            .fontSize(11)
            .fontColor(Color.Blue)
            .margin({ bottom: 8 })
          Image($r('app.media.startIcon'))
            .width(this.imageWidth)
            .height(this.imageHeight)
            .objectFit(ImageFit.Cover)
            .orientation(this.ORIENTATIONS[this.currentOrientationIndex].orientation)
            .edgeAntialiasing(this.antialiasingEnabled ? 1 : 0)
            .border({ width: 2, color: Color.Blue })
            .borderRadius(8)
        }
      }
      .margin({ bottom: 16 })

      Text(`方向: ${this.ORIENTATIONS[this.currentOrientationIndex].desc}`)
        .fontSize(12)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })

      Text(`edgeAntialiasing: ${this.antialiasingEnabled ? '开启(1)' : '关闭(0)'}`)
        .fontSize(12)
        .fontColor(this.antialiasingEnabled ? Color.Green : Color.Orange)
        .margin({ bottom: 16 })

      Divider()

      Text('方向选择(点击切换)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Grid() {
        ForEach(this.ORIENTATIONS, (config: { orientation: ImageRotateOrientation; label: string; desc: string },
            index: number) => {
          GridItem() {
            Column() {
              Image($r('app.media.startIcon'))
                .width(50)
                .height(50)
                .objectFit(ImageFit.Cover)
                .orientation(config.orientation)
                .borderRadius(4)
                .border({ width: 1, color: index === this.currentOrientationIndex ? Color.Blue : '#EEEEEE' })

              Text(config.label)
                .fontSize(8)
                .fontColor(index === this.currentOrientationIndex ? Color.Blue : Color.Gray)
                .margin({ top: 4 })
                .maxLines(1)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
            }
            .onClick(() => { this.currentOrientationIndex = index })
          }
        })
      }
      .columnsTemplate('repeat(4, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(8)
      .margin({ bottom: 16 })

      Divider()

      Text('edgeAntialiasing开关')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 12 }) {
        Button(this.antialiasingEnabled ? '抗锯齿: 开' : '抗锯齿: 关')
          .fontSize(11)
          .backgroundColor(this.antialiasingEnabled ? '#4CAF50' : '#FF9800')
          .fontColor(Color.White)
          .onClick(() => { this.antialiasingEnabled = !this.antialiasingEnabled })
      }
      .margin({ bottom: 12 })

      Text('提示: 边缘抗锯齿在旋转/缩放时效果更明显')
        .fontSize(10)
        .fontColor(Color.Gray)
        .margin({ bottom: 16 })

      Divider()

      Text('尺寸调节(观察旋转后布局变化)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 8 }) {
        Button('80x80')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 80; this.imageHeight = 80 })
        Button('120x120')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 120; this.imageHeight = 120 })
        Button('180x80')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 180; this.imageHeight = 80 })
        Button('80x180')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 80; this.imageHeight = 180 })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

@Component
struct ImageSvgSourceTestCase066 {
  @State currentSvgIndex: number = 0
  @State currentFitIndex: number = 0
  @State imageWidth: number = 120
  @State imageHeight: number = 120
  @State fillColorEnabled: boolean = false
  @State currentFilterIndex: number = 0
  @State loadLog: string = '等待SVG加载...'

  private readonly SVG_SOURCES: Array<{ src: Resource; label: string; desc: string }> = [
    { src: $r('app.media.svg'), label: 'svg(勾选图标)', desc: '基础路径+圆形' },
    { src: $r('app.media.heart'), label: 'heart(心形)', desc: '贝塞尔曲线心形' },
    { src: $r('app.media.star'), label: 'star(五角星)', desc: '多边形五角星' },
    { src: $r('app.media.icon'), label: 'icon(复合图标)', desc: '矩形+圆形组合' },
    { src: $r('app.media.svg2_gradient_linear'), label: '线性渐变', desc: 'SVG2 linearGradient' },
    { src: $r('app.media.svg2_gradient_radial'), label: '径向渐变', desc: 'SVG2 radialGradient' },
    { src: $r('app.media.svg2_filter_complex'), label: '滤镜效果', desc: 'feGaussianBlur/feColorMatrix' },
    { src: $r('app.media.svg2_mask_pattern'), label: '蒙版与图案', desc: 'SVG2 mask/pattern' },
    { src: $r('app.media.svg2_mixed'), label: '混合特性', desc: 'SVG2综合特性' },
    { src: $r('app.media.svg2_coordinates'), label: '坐标系测试', desc: 'SVG2坐标系' }
  ]

  private readonly FIT_MODES: Array<{ fit: ImageFit; label: string }> = [
    { fit: ImageFit.Cover, label: 'Cover' },
    { fit: ImageFit.Contain, label: 'Contain' },
    { fit: ImageFit.Fill, label: 'Fill' },
    { fit: ImageFit.ScaleDown, label: 'ScaleDown' },
    { fit: ImageFit.None, label: 'None' },
    { fit: ImageFit.Auto, label: 'Auto' }
  ]

  private readonly COLOR_FILTERS: Array<{ name: string; matrix: number[] }> = [
    { name: '原图', matrix: [1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0] },
    { name: '灰度', matrix: [0.299, 0.587, 0.114, 0, 0, 0.299, 0.587, 0.114, 0, 0, 0.299, 0.587, 0.114, 0, 0, 0, 0, 0, 1, 0] },
    { name: '反色', matrix: [-1, 0, 0, 0, 1, 0, -1, 0, 0, 1, 0, 0, -1, 0, 1, 0, 0, 0, 1, 0] }
  ]

  build() {
    Column() {
      Text('Image SVG图片源渲染测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Column() {
          Text('PNG对照(startIcon)')
            .fontSize(11)
            .fontColor(Color.Gray)
            .margin({ bottom: 8 })
          Image($r('app.media.startIcon'))
            .width(this.imageWidth)
            .height(this.imageHeight)
            .objectFit(this.FIT_MODES[this.currentFitIndex].fit)
            .border({ width: 1, color: Color.Gray })
            .borderRadius(8)
        }
        .margin({ right: 16 })

        Column() {
          Text(`SVG: ${this.SVG_SOURCES[this.currentSvgIndex].label}`)
            .fontSize(11)
            .fontColor(Color.Blue)
            .margin({ bottom: 8 })
          Image(this.SVG_SOURCES[this.currentSvgIndex].src)
            .width(this.imageWidth)
            .height(this.imageHeight)
            .objectFit(this.FIT_MODES[this.currentFitIndex].fit)
            .fillColor(this.fillColorEnabled ? Color.Magenta : undefined)
            .colorFilter(this.COLOR_FILTERS[this.currentFilterIndex].matrix)
            .renderMode(this.fillColorEnabled ? ImageRenderMode.Template : ImageRenderMode.Original)
            .border({ width: 2, color: Color.Blue })
            .borderRadius(8)
            .onComplete(() => {
              this.loadLog = `${this.SVG_SOURCES[this.currentSvgIndex].label} 加载完成`
            })
            .onError((err: { componentWidth: number; componentHeight: number; message: string }) => {
              this.loadLog = `${this.SVG_SOURCES[this.currentSvgIndex].label} 加载失败: ${err.message}`
            })
        }
      }
      .margin({ bottom: 16 })

      Text(this.SVG_SOURCES[this.currentSvgIndex].desc)
        .fontSize(12)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })

      Text(this.loadLog)
        .fontSize(11)
        .fontColor(this.loadLog.includes('完成') ? Color.Green : Color.Orange)
        .margin({ bottom: 12 })

      Divider()

      Text('SVG图源选择')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Grid() {
        ForEach(this.SVG_SOURCES, (config: { src: Resource; label: string; desc: string }, index: number) => {
          GridItem() {
            Column() {
              Image(config.src)
                .width(45)
                .height(45)
                .objectFit(ImageFit.Contain)
                .borderRadius(4)
                .border({ width: 1, color: index === this.currentSvgIndex ? Color.Blue : '#EEEEEE' })

              Text(config.label)
                .fontSize(8)
                .fontColor(index === this.currentSvgIndex ? Color.Blue : Color.Gray)
                .margin({ top: 4 })
                .maxLines(1)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
            }
            .onClick(() => {
              this.currentSvgIndex = index
              this.loadLog = `已切换到 ${config.label}, 等待加载...`
            })
          }
        })
      }
      .columnsTemplate('repeat(5, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(8)
      .margin({ bottom: 16 })

      Divider()

      Text('objectFit切换(观察SVG缩放)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row() {
        ForEach(this.FIT_MODES, (config: { fit: ImageFit; label: string }, index: number) => {
          Button(config.label)
            .fontSize(9)
            .onClick(() => { this.currentFitIndex = index })
            .backgroundColor(index === this.currentFitIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentFitIndex ? Color.White : Color.Gray)
            .margin({ left: 3, right: 3 })
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('fillColor + Template模式(SVG着色)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 12 }) {
        Button(this.fillColorEnabled ? '着色: 开' : '着色: 关')
          .fontSize(11)
          .backgroundColor(this.fillColorEnabled ? '#4CAF50' : '#FF9800')
          .fontColor(Color.White)
          .onClick(() => { this.fillColorEnabled = !this.fillColorEnabled })
      }
      .margin({ bottom: 12 })

      Text('colorFilter叠加')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 8, bottom: 8 })

      Row() {
        ForEach(this.COLOR_FILTERS, (config: { name: string; matrix: number[] }, index: number) => {
          Button(config.name)
            .fontSize(9)
            .onClick(() => { this.currentFilterIndex = index })
            .backgroundColor(index === this.currentFilterIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentFilterIndex ? Color.White : Color.Gray)
            .margin({ left: 3, right: 3 })
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('尺寸调节(观察SVG矢量缩放)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 8 }) {
        Button('40x40')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 40; this.imageHeight = 40 })
        Button('120x120')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 120; this.imageHeight = 120 })
        Button('200x80')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 200; this.imageHeight = 80 })
        Button('80x200')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 80; this.imageHeight = 200 })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}
