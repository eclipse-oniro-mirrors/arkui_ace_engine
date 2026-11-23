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


/** 整合自: ImageTestcase069.ets, ImageTestcase070.ets */

@Entry
@Component
struct ImageDimensionEdgeCaseTestCase069 {
  @State currentSizeIndex: number = 0
  @State currentFitIndex: number = 0
  @State layoutLog: string = '选择不同尺寸规格,观察渲染效果'
  @State measuredWidth: number = 0
  @State measuredHeight: number = 0

  private readonly SIZE_PRESETS: Array<{
    name: string
    width: string | number | undefined
    height: string | number | undefined
    desc: string
    valid: boolean
  }> = [
    { name: '正常120x120', width: 120, height: 120, desc: '标准正方形尺寸', valid: true },
    { name: '零宽0x120', width: 0, height: 120, desc: '宽度为0, 验证是否渲染', valid: false },
    { name: '零高120x0', width: 120, height: 0, desc: '高度为0, 验证是否渲染', valid: false },
    { name: '全零0x0', width: 0, height: 0, desc: '宽高均为0', valid: false },
    { name: '极小1x1', width: 1, height: 1, desc: '1像素尺寸', valid: true },
    { name: '极小5x5', width: 5, height: 5, desc: '5像素尺寸', valid: true },
    { name: '极大500x500', width: 500, height: 500, desc: '超大尺寸(可能溢出)', valid: true },
    { name: '宽>高300x80', width: 300, height: 80, desc: '宽扁矩形', valid: true },
    { name: '高>宽80x300', width: 80, height: 300, desc: '窄高矩形', valid: true },
    { name: '百分比100%', width: '100%', height: '100%', desc: '百分比填满父容器', valid: true },
    { name: '半百分比50%', width: '50%', height: '50%', desc: '半百分比', valid: true },
    { name: '宽100%高固定', width: '100%', height: 100, desc: '百分比+固定值混合', valid: true },
    { name: '宽固定高100%', width: 100, height: '100%', desc: '固定值+百分比混合', valid: true },
    { name: '未定义宽', width: undefined, height: 120, desc: 'width为undefined', valid: true },
    { name: '未定义高', width: 120, height: undefined, desc: 'height为undefined', valid: true },
    { name: '双未定义', width: undefined, height: undefined, desc: '宽高均undefined(自适应)', valid: true }
  ]

  private readonly FIT_MODES: Array<{ fit: ImageFit; label: string }> = [
    { fit: ImageFit.Cover, label: 'Cover' },
    { fit: ImageFit.Contain, label: 'Contain' },
    { fit: ImageFit.Fill, label: 'Fill' },
    { fit: ImageFit.ScaleDown, label: 'ScaleDown' },
    { fit: ImageFit.Auto, label: 'Auto' }
  ]

  build() {
    Column() {
      Text('Image尺寸边界值测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Column() {
        Image($r('app.media.startIcon'))
          .width(this.SIZE_PRESETS[this.currentSizeIndex].width as Length)
          .height(this.SIZE_PRESETS[this.currentSizeIndex].height as Length)
          .objectFit(this.FIT_MODES[this.currentFitIndex].fit)
          .border({ width: 2, color: this.SIZE_PRESETS[this.currentSizeIndex].valid ? Color.Blue : Color.Red })
          .borderRadius(4)
          .backgroundColor('#F5F5F5')
          .onComplete((event: { width: number; height: number; componentWidth: number; componentHeight: number }) => {
            this.measuredWidth = event.componentWidth
            this.measuredHeight = event.componentHeight
            this.layoutLog = `onComplete: 组件尺寸 ${event.componentWidth}x${event.componentHeight}`
          })
      }
      .width('100%')
      .height(200)
      .justifyContent(FlexAlign.Center)
      .backgroundColor('#FAFAFA')
      .borderRadius(8)
      .margin({ bottom: 16 })

      Text(this.SIZE_PRESETS[this.currentSizeIndex].name)
        .fontSize(14)
        .fontWeight(FontWeight.Bold)
        .fontColor(this.SIZE_PRESETS[this.currentSizeIndex].valid ? Color.Blue : Color.Red)
        .margin({ bottom: 8 })

      Text(this.SIZE_PRESETS[this.currentSizeIndex].desc)
        .fontSize(12)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })

      Text(this.layoutLog)
        .fontSize(11)
        .fontColor(Color.Green)
        .margin({ bottom: 4 })

      Text(`实测尺寸: ${this.measuredWidth}x${this.measuredHeight}`)
        .fontSize(11)
        .fontColor(Color.Orange)
        .margin({ bottom: 16 })

      Divider()

      Text('尺寸预设(点击切换)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Grid() {
        ForEach(this.SIZE_PRESETS, (config: {
          name: string
          width: string | number | undefined
          height: string | number | undefined
          desc: string
          valid: boolean
        }, index: number) => {
          GridItem() {
            Column() {
              Text(config.name)
                .fontSize(9)
                .fontColor(index === this.currentSizeIndex ? Color.Blue : Color.Gray)
                .maxLines(2)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
            }
            .padding(8)
            .backgroundColor(index === this.currentSizeIndex ? '#E3F2FD' : '#FAFAFA')
            .borderRadius(4)
            .border({ width: 1, color: config.valid ? '#EEEEEE' : '#FFCCCC' })
            .onClick(() => {
              this.currentSizeIndex = index
              this.measuredWidth = 0
              this.measuredHeight = 0
              this.layoutLog = `已切换到 ${config.name}, 等待onComplete...`
            })
          }
        })
      }
      .columnsTemplate('repeat(3, 1fr)')
      .rowsGap(6)
      .columnsGap(6)
      .padding(8)
      .margin({ bottom: 16 })

      Divider()

      Text('objectFit切换')
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

      Text('对比: 同尺寸PNG vs SVG')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row() {
        Column() {
          Text('PNG(startIcon)')
            .fontSize(9)
            .fontColor(Color.Gray)
            .margin({ bottom: 4 })
          Image($r('app.media.startIcon'))
            .width(60)
            .height(60)
            .objectFit(this.FIT_MODES[this.currentFitIndex].fit)
            .border({ width: 1, color: Color.Gray })
            .borderRadius(4)
        }
        .margin({ right: 16 })

        Column() {
          Text('SVG(svg)')
            .fontSize(9)
            .fontColor(Color.Gray)
            .margin({ bottom: 4 })
          Image($r('app.media.svg'))
            .width(60)
            .height(60)
            .objectFit(this.FIT_MODES[this.currentFitIndex].fit)
            .border({ width: 1, color: Color.Blue })
            .borderRadius(4)
        }
      }
      .margin({ bottom: 12 })

      Text('说明: 红色边框=预期可能异常的尺寸\nSVG为矢量图,缩放后仍清晰; PNG放大后可能模糊')
        .fontSize(10)
        .fontColor(Color.Gray)
        .lineHeight(16)
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

@Component
struct ImageForegroundColorTestCase070 {
  @State currentRenderModeIndex: number = 0
  @State currentColorIndex: number = 0
  @State opacityValue: number = 1.0
  @State useForegroundColor: boolean = true
  @State currentImageIndex: number = 0
  @State colorBlendValue: number = 0

  private readonly RENDER_MODES: Array<{ mode: ImageRenderMode; label: string; desc: string }> = [
    { mode: ImageRenderMode.Original, label: 'Original(原图)', desc: '保留原始颜色' },
    { mode: ImageRenderMode.Template, label: 'Template(模板)', desc: '图片转为模板,可被着色' }
  ]

  private readonly COLORS: Array<{ color: ResourceColor; label: string }> = [
    { color: Color.Red, label: '红色' },
    { color: Color.Green, label: '绿色' },
    { color: Color.Blue, label: '蓝色' },
    { color: Color.Black, label: '黑色' },
    { color: Color.White, label: '白色' },
    { color: Color.Gray, label: '灰色' },
    { color: Color.Pink, label: '粉色' },
    { color: Color.Orange, label: '橙色' },
    { color: '#9C27B0', label: '紫色' },
    { color: '#00BCD4', label: '青色' }
  ]

  private readonly IMAGES: Array<{ src: Resource; label: string }> = [
    { src: $r('app.media.startIcon'), label: 'startIcon' },
    { src: $r('app.media.foreground'), label: 'foreground' },
    { src: $r('app.media.heart'), label: 'heart(SVG)' },
    { src: $r('app.media.star'), label: 'star(SVG)' },
    { src: $r('app.media.icon'), label: 'icon(SVG)' }
  ]

  build() {
    Column() {
      Text('Image foregroundColor前景色测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Column() {
          Text('原图(无前景色)')
            .fontSize(11)
            .fontColor(Color.Gray)
            .margin({ bottom: 8 })
          Image(this.IMAGES[this.currentImageIndex].src)
            .width(100)
            .height(100)
            .objectFit(ImageFit.Cover)
            .border({ width: 1, color: Color.Gray })
            .borderRadius(8)
        }
        .margin({ right: 16 })

        Column() {
          Text(this.useForegroundColor ? 'foregroundColor' : 'fillColor')
            .fontSize(11)
            .fontColor(Color.Blue)
            .margin({ bottom: 8 })
          Image(this.IMAGES[this.currentImageIndex].src)
            .width(100)
            .height(100)
            .objectFit(ImageFit.Cover)
            .renderMode(this.RENDER_MODES[this.currentRenderModeIndex].mode)
            .foregroundColor(this.useForegroundColor ? this.COLORS[this.currentColorIndex].color : undefined)
            .fillColor(!this.useForegroundColor ? this.COLORS[this.currentColorIndex].color : undefined)
            .opacity(this.opacityValue)
            .colorBlend(this.colorBlendValue > 0 ? this.COLORS[this.currentColorIndex].color : Color.Transparent)
            .border({ width: 2, color: Color.Blue })
            .borderRadius(8)
        }
      }
      .margin({ bottom: 16 })

      Text(`renderMode: ${this.RENDER_MODES[this.currentRenderModeIndex].label}`)
        .fontSize(12)
        .fontColor(Color.Gray)
        .margin({ bottom: 4 })

      Text(`着色属性: ${this.useForegroundColor ? 'foregroundColor' : 'fillColor'}`)
        .fontSize(12)
        .fontColor(Color.Blue)
        .margin({ bottom: 4 })

      Text(`颜色: ${this.COLORS[this.currentColorIndex].label}  透明度: ${this.opacityValue}`)
        .fontSize(12)
        .fontColor(Color.Orange)
        .margin({ bottom: 16 })

      Divider()

      Text('图源选择')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row() {
        ForEach(this.IMAGES, (config: { src: Resource; label: string }, index: number) => {
          Button(config.label)
            .fontSize(8)
            .onClick(() => { this.currentImageIndex = index })
            .backgroundColor(index === this.currentImageIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentImageIndex ? Color.White : Color.Gray)
            .margin({ left: 2, right: 2 })
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('renderMode切换')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 12 }) {
        ForEach(this.RENDER_MODES, (config: { mode: ImageRenderMode; label: string; desc: string }, index: number) => {
          Button(config.label)
            .fontSize(10)
            .onClick(() => { this.currentRenderModeIndex = index })
            .backgroundColor(index === this.currentRenderModeIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentRenderModeIndex ? Color.White : Color.Gray)
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('着色属性切换(foregroundColor vs fillColor)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 12 }) {
        Button('foregroundColor')
          .fontSize(10)
          .onClick(() => { this.useForegroundColor = true })
          .backgroundColor(this.useForegroundColor ? '#4CAF50' : '#EEEEEE')
          .fontColor(this.useForegroundColor ? Color.White : Color.Gray)
        Button('fillColor')
          .fontSize(10)
          .onClick(() => { this.useForegroundColor = false })
          .backgroundColor(!this.useForegroundColor ? '#4CAF50' : '#EEEEEE')
          .fontColor(!this.useForegroundColor ? Color.White : Color.Gray)
      }
      .margin({ bottom: 12 })

      Text(this.RENDER_MODES[this.currentRenderModeIndex].desc)
        .fontSize(10)
        .fontColor(Color.Gray)
        .margin({ bottom: 16 })

      Divider()

      Text('颜色选择(全色板)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Grid() {
        ForEach(this.COLORS, (config: { color: ResourceColor; label: string }, index: number) => {
          GridItem() {
            Column() {
              Column()
                .width(36)
                .height(36)
                .backgroundColor(config.color)
                .borderRadius(4)
                .border({ width: 2, color: index === this.currentColorIndex ? Color.Blue : '#EEEEEE' })

              Text(config.label)
                .fontSize(8)
                .fontColor(index === this.currentColorIndex ? Color.Blue : Color.Gray)
                .margin({ top: 4 })
                .maxLines(1)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
            }
            .onClick(() => { this.currentColorIndex = index })
          }
        })
      }
      .columnsTemplate('repeat(5, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(8)
      .margin({ bottom: 16 })

      Divider()

      Text('透明度调节')
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
      .margin({ bottom: 12 })

      Text('colorBlend叠加开关')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 8, bottom: 8 })

      Row({ space: 12 }) {
        Button(this.colorBlendValue > 0 ? 'colorBlend: 开' : 'colorBlend: 关')
          .fontSize(10)
          .backgroundColor(this.colorBlendValue > 0 ? '#FF9800' : '#EEEEEE')
          .fontColor(this.colorBlendValue > 0 ? Color.White : Color.Gray)
          .onClick(() => { this.colorBlendValue = this.colorBlendValue > 0 ? 0 : 1 })
      }
      .margin({ bottom: 12 })

      Text('说明: foregroundColor设前景色, 需配合Template模式生效\n' +
        'fillColor对Template模式图片着色, 对Original模式可能无效\n' +
        'colorBlend为颜色混合叠加, 在Original模式下也有效')
        .fontSize(10)
        .fontColor(Color.Gray)
        .lineHeight(16)
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}
