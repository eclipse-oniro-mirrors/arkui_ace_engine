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
struct ImageTransformScene {
  @State currentPresetIndex: number = 0
  @State rotateAngle: number = 0
  @State scaleX: number = 1
  @State scaleY: number = 1
  @State translateX: number = 0
  @State translateY: number = 0
  @State animDuration: number = 400
  @State log: string = '选择预设或手动调节变换参数'

  private readonly PRESETS: Array<{
    name: string; angle: number; sx: number; sy: number; tx: number; ty: number; dur: number
  }> = [
    { name: '原图', angle: 0, sx: 1, sy: 1, tx: 0, ty: 0, dur: 400 },
    { name: '旋转45', angle: 45, sx: 1, sy: 1, tx: 0, ty: 0, dur: 400 },
    { name: '旋转180', angle: 180, sx: 1, sy: 1, tx: 0, ty: 0, dur: 600 },
    { name: '放大1.5', angle: 0, sx: 1.5, sy: 1.5, tx: 0, ty: 0, dur: 400 },
    { name: '缩小0.6', angle: 0, sx: 0.6, sy: 0.6, tx: 0, ty: 0, dur: 400 },
    { name: '横向拉伸', angle: 0, sx: 2, sy: 1, tx: 0, ty: 0, dur: 400 },
    { name: '纵向拉伸', angle: 0, sx: 1, sy: 2, tx: 0, ty: 0, dur: 400 },
    { name: '右上偏移', angle: 30, sx: 1.2, sy: 1.2, tx: 20, ty: -20, dur: 500 },
    { name: '镜像翻转', angle: 0, sx: -1, sy: 1, tx: 0, ty: 0, dur: 400 },
    { name: '组合特效', angle: 25, sx: 1.3, sy: 0.8, tx: 15, ty: 10, dur: 600 }
  ]

  build() {
    Column() {
      Text('Image transform变换测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Stack() {
        Image($r('app.media.startIcon'))
          .width(120)
          .height(120)
          .objectFit(ImageFit.Cover)
          .rotate({ angle: this.rotateAngle })
          .scale({ x: this.scaleX, y: this.scaleY })
          .translate({ x: this.translateX, y: this.translateY })
          .border({ width: 2, color: Color.Blue })
          .borderRadius(8)
          .animation({ duration: this.animDuration, curve: Curve.EaseInOut })
      }
      .width('100%')
      .height(200)
      .backgroundColor('#F5F5F5')
      .borderRadius(8)
      .margin({ bottom: 16 })

      Text(`旋转:${this.rotateAngle}° 缩放:(${this.scaleX}, ${this.scaleY}) 偏移:(${this.translateX}, ${this.translateY})`)
        .fontSize(11)
        .fontColor(Color.Gray)
        .margin({ bottom: 4 })
      Text(this.log)
        .fontSize(11)
        .fontColor(Color.Orange)
        .margin({ bottom: 12 })

      Divider()

      Text('预设组合(点击切换)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Grid() {
        ForEach(this.PRESETS, (preset: {
          name: string; angle: number; sx: number; sy: number; tx: number; ty: number; dur: number
        }, index: number) => {
          GridItem() {
            Column() {
              Text(preset.name)
                .fontSize(11)
                .fontColor(index === this.currentPresetIndex ? Color.Blue : Color.Gray)
            }
            .padding(8)
            .backgroundColor(index === this.currentPresetIndex ? '#E3F2FD' : '#FAFAFA')
            .borderRadius(4)
            .onClick(() => {
              this.currentPresetIndex = index
              this.rotateAngle = preset.angle
              this.scaleX = preset.sx
              this.scaleY = preset.sy
              this.translateX = preset.tx
              this.translateY = preset.ty
              this.animDuration = preset.dur
              this.log = `应用预设: ${preset.name}`
            })
          }
        })
      }
      .columnsTemplate('repeat(5, 1fr)')
      .rowsGap(6)
      .columnsGap(6)
      .padding(8)

      Divider()

      Text('手动调节')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Row({ space: 8 }) {
        Button('旋转+15')
          .fontSize(10)
          .onClick(() => { this.rotateAngle += 15; this.log = '手动旋转+15' })
        Button('旋转-15')
          .fontSize(10)
          .onClick(() => { this.rotateAngle -= 15; this.log = '手动旋转-15' })
        Button('放大')
          .fontSize(10)
          .onClick(() => {
            this.scaleX = Math.min(3, this.scaleX + 0.2)
            this.scaleY = Math.min(3, this.scaleY + 0.2)
            this.log = '手动放大'
          })
        Button('缩小')
          .fontSize(10)
          .onClick(() => {
            this.scaleX = Math.max(0.2, this.scaleX - 0.2)
            this.scaleY = Math.max(0.2, this.scaleY - 0.2)
            this.log = '手动缩小'
          })
        Button('重置')
          .fontSize(10)
          .backgroundColor('#FF6666')
          .fontColor(Color.White)
          .onClick(() => {
            this.currentPresetIndex = 0
            this.rotateAngle = 0; this.scaleX = 1; this.scaleY = 1
            this.translateX = 0; this.translateY = 0
            this.log = '已重置'
          })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 新增 062：shadow 阴影效果 */
@Component
struct ImageShadowScene {
  @State currentPresetIndex: number = 0
  @State radius: number = 10
  @State offsetX: number = 0
  @State offsetY: number = 0
  @State shadowColor: ResourceColor = Color.Black
  @State fillEnabled: boolean = false

  private readonly SHADOW_PRESETS: Array<{
    name: string; radius: number; x: number; y: number; color: ResourceColor; fill: boolean
  }> = [
    { name: '无阴影', radius: 0, x: 0, y: 0, color: Color.Black, fill: false },
    { name: '标准阴影', radius: 10, x: 0, y: 4, color: Color.Black, fill: false },
    { name: '深阴影', radius: 20, x: 0, y: 8, color: Color.Black, fill: false },
    { name: '红色发光', radius: 25, x: 0, y: 0, color: Color.Red, fill: true },
    { name: '蓝色发光', radius: 30, x: 0, y: 0, color: Color.Blue, fill: true },
    { name: '绿色发光', radius: 20, x: 0, y: 0, color: Color.Green, fill: true },
    { name: '橙色偏移', radius: 15, x: 10, y: 10, color: Color.Orange, fill: false },
    { name: '紫色偏移', radius: 12, x: -8, y: -8, color: Color.Purple, fill: false },
    { name: '细硬阴影', radius: 3, x: 2, y: 2, color: '#333333', fill: false },
    { name: '大柔阴影', radius: 40, x: 0, y: 12, color: '#888888', fill: false }
  ]

  build() {
    Column() {
      Text('Image shadow阴影测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Stack() {
        Image($r('app.media.startIcon'))
          .width(140)
          .height(140)
          .objectFit(ImageFit.Cover)
          .borderRadius(this.SHADOW_PRESETS[this.currentPresetIndex].fill ? 70 : 8)
          .shadow({
            radius: this.radius,
            color: this.shadowColor,
            offsetX: this.offsetX,
            offsetY: this.offsetY,
            fill: this.fillEnabled
          })
      }
      .width('100%')
      .height(200)
      .backgroundColor('#FAFAFA')
      .borderRadius(8)
      .margin({ bottom: 16 })

      Text(`半径:${this.radius} 偏移:(${this.offsetX}, ${this.offsetY}) 颜色:${this.shadowColor} fill:${this.fillEnabled}`)
        .fontSize(11)
        .fontColor(Color.Gray)
        .margin({ bottom: 4 })
      Text(`预设: ${this.SHADOW_PRESETS[this.currentPresetIndex].name}`)
        .fontSize(12)
        .fontColor(Color.Blue)
        .margin({ bottom: 12 })

      Divider()

      Text('阴影预设(点击切换)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Grid() {
        ForEach(this.SHADOW_PRESETS, (preset: {
          name: string; radius: number; x: number; y: number; color: ResourceColor; fill: boolean
        }, index: number) => {
          GridItem() {
            Column() {
              Image($r('app.media.startIcon'))
                .width(40)
                .height(40)
                .borderRadius(preset.fill ? 20 : 4)
                .shadow({
                  radius: preset.radius / 2,
                  color: preset.color,
                  offsetX: preset.x / 2,
                  offsetY: preset.y / 2,
                  fill: preset.fill
                })
              Text(preset.name)
                .fontSize(9)
                .margin({ top: 2 })
                .fontColor(index === this.currentPresetIndex ? Color.Blue : Color.Gray)
            }
            .padding(6)
            .backgroundColor(index === this.currentPresetIndex ? '#E3F2FD' : '#FAFAFA')
            .borderRadius(4)
            .onClick(() => {
              this.currentPresetIndex = index
              this.radius = preset.radius
              this.offsetX = preset.x
              this.offsetY = preset.y
              this.shadowColor = preset.color
              this.fillEnabled = preset.fill
            })
          }
        })
      }
      .columnsTemplate('repeat(5, 1fr)')
      .rowsGap(6)
      .columnsGap(6)
      .padding(8)
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 新增 063：position / markAnchor / offset 定位 */
@Component
struct ImagePositionScene {
  @State currentModeIndex: number = 0
  @State posX: number = 0
  @State posY: number = 0
  @State anchorX: number = 0
  @State anchorY: number = 0
  @State offsetX: number = 0
  @State offsetY: number = 0
  @State log: string = '切换定位模式观察效果'

  private readonly POSITION_PRESETS: Array<{
    name: string; px: number; py: number; ax: number; ay: number; ox: number; oy: number
  }> = [
    { name: '默认(0,0)', px: 0, py: 0, ax: 0, ay: 0, ox: 0, oy: 0 },
    { name: '右下角', px: 200, py: 120, ax: 60, ay: 60, ox: 0, oy: 0 },
    { name: '居中偏移', px: 120, py: 60, ax: 60, ay: 60, ox: 0, oy: 0 },
    { name: '左上锚点', px: 0, py: 0, ax: -30, ay: -30, ox: 0, oy: 0 },
    { name: 'offset偏移', px: 0, py: 0, ax: 0, ay: 0, ox: 40, oy: 40 },
    { name: '组合定位', px: 100, py: 50, ax: 30, ay: 30, ox: 20, oy: -20 }
  ]

  build() {
    Column() {
      Text('Image position/markAnchor/offset定位测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Stack() {
        Column()
          .width('100%')
          .height(200)
          .backgroundColor('#F0F0F0')
          .borderRadius(8)
          .border({ width: 1, color: Color.Gray })

        Image($r('app.media.startIcon'))
          .width(80)
          .height(80)
          .objectFit(ImageFit.Cover)
          .position({ x: this.posX, y: this.posY })
          .markAnchor({ x: this.anchorX, y: this.anchorY })
          .offset({ x: this.offsetX, y: this.offsetY })
          .border({ width: 2, color: Color.Red })
          .borderRadius(8)
      }
      .width('100%')
      .height(220)
      .margin({ bottom: 16 })

      Text(`position:(${this.posX}, ${this.posY})` +
        ` anchor:(${this.anchorX}, ${this.anchorY}) offset:(${this.offsetX}, ${this.offsetY})`)
        .fontSize(10)
        .fontColor(Color.Gray)
        .margin({ bottom: 4 })
      Text(this.log)
        .fontSize(11)
        .fontColor(Color.Orange)
        .margin({ bottom: 12 })

      Divider()

      Text('定位预设')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Grid() {
        ForEach(this.POSITION_PRESETS, (preset: {
          name: string; px: number; py: number; ax: number; ay: number; ox: number; oy: number
        }, index: number) => {
          GridItem() {
            Column() {
              Text(preset.name)
                .fontSize(10)
                .fontColor(index === this.currentModeIndex ? Color.Blue : Color.Gray)
            }
            .padding(8)
            .backgroundColor(index === this.currentModeIndex ? '#E3F2FD' : '#FAFAFA')
            .borderRadius(4)
            .onClick(() => {
              this.currentModeIndex = index
              this.posX = preset.px; this.posY = preset.py
              this.anchorX = preset.ax; this.anchorY = preset.ay
              this.offsetX = preset.ox; this.offsetY = preset.oy
              this.log = `应用: ${preset.name}`
            })
          }
        })
      }
      .columnsTemplate('repeat(3, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(8)

      Divider()

      Text('手动微调')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Row({ space: 8 }) {
        Button('X+20')
          .fontSize(10)
          .onClick(() => { this.posX += 20; this.log = 'position X+20' })
        Button('X-20')
          .fontSize(10)
          .onClick(() => { this.posX = Math.max(0, this.posX - 20); this.log = 'position X-20' })
        Button('Y+20')
          .fontSize(10)
          .onClick(() => { this.posY += 20; this.log = 'position Y+20' })
        Button('Y-20')
          .fontSize(10)
          .onClick(() => { this.posY = Math.max(0, this.posY - 20); this.log = 'position Y-20' })
        Button('重置')
          .fontSize(10)
          .backgroundColor('#FF6666')
          .fontColor(Color.White)
          .onClick(() => {
            this.currentModeIndex = 0
            this.posX = 0; this.posY = 0; this.anchorX = 0; this.anchorY = 0
            this.offsetX = 0; this.offsetY = 0
            this.log = '已重置'
          })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 新增 064：visibility 可见性 */
@Component
struct ImageVisibilityScene {
  @State currentVisIndex: number = 0
  @State toggleCount: number = 0
  @State log: string = '切换可见性模式观察布局变化'

  private readonly VIS_MODES: Array<{ mode: Visibility; label: string; desc: string }> = [
    { mode: Visibility.Visible, label: 'Visible', desc: '可见(占位)' },
    { mode: Visibility.Hidden, label: 'Hidden', desc: '隐藏(保留占位)' },
    { mode: Visibility.None, label: 'None', desc: '隐藏(不占位)' }
  ]

  build() {
    Column() {
      Text('Image visibility可见性测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Column() {
          Text('Visible(参考)')
            .fontSize(11)
            .fontColor(Color.Gray)
            .margin({ bottom: 4 })
          Image($r('app.media.startIcon'))
            .width(80)
            .height(80)
            .objectFit(ImageFit.Cover)
            .visibility(Visibility.Visible)
            .border({ width: 1, color: Color.Gray })
            .borderRadius(8)
        }
        .margin({ right: 12 })

        Column() {
          Text(`当前: ${this.VIS_MODES[this.currentVisIndex].label}`)
            .fontSize(11)
            .fontColor(Color.Blue)
            .margin({ bottom: 4 })
          Image($r('app.media.startIcon'))
            .width(80)
            .height(80)
            .objectFit(ImageFit.Cover)
            .visibility(this.VIS_MODES[this.currentVisIndex].mode)
            .border({ width: 2, color: Color.Blue })
            .borderRadius(8)
        }
        .margin({ right: 12 })

        Column() {
          Text('下方文本')
            .fontSize(11)
            .fontColor(Color.Gray)
          Text('Hidden 保留布局')
            .fontSize(10)
            .fontColor(Color.Orange)
        }
      }
      .margin({ bottom: 16 })

      Text(`模式: ${this.VIS_MODES[this.currentVisIndex].label} - ${this.VIS_MODES[this.currentVisIndex].desc}`)
        .fontSize(12)
        .fontColor(Color.Gray)
        .margin({ bottom: 4 })
      Text(`切换次数: ${this.toggleCount}`)
        .fontSize(11)
        .fontColor(Color.Orange)
        .margin({ bottom: 4 })
      Text(this.log)
        .fontSize(11)
        .fontColor(Color.Blue)
        .margin({ bottom: 12 })

      Divider()

      Text('可见性切换')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Row() {
        ForEach(this.VIS_MODES, (config: { mode: Visibility; label: string; desc: string }, index: number) => {
          Button(config.label)
            .fontSize(11)
            .onClick(() => {
              this.currentVisIndex = index
              this.toggleCount++
              this.log = `切换为 ${config.label}: ${config.desc}`
            })
            .backgroundColor(index === this.currentVisIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentVisIndex ? Color.White : Color.Gray)
            .margin({ left: 4, right: 4 })
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('多图批量可见性(瀑布排列)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Row() {
        ForEach([0, 1, 2, 3, 4], (i: number) => {
          Image($r('app.media.startIcon'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .visibility(i === 1 ? this.VIS_MODES[this.currentVisIndex].mode : Visibility.Visible)
            .borderRadius(6)
            .border({ width: i === 1 ? 2 : 1, color: i === 1 ? Color.Red : Color.Gray })
            .margin({ left: 4, right: 4 })
        })
      }
      Text('↑ 第二张跟随主控制,其余恒定Visible')
        .fontSize(9)
        .fontColor(Color.Gray)
        .margin({ top: 4 })
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 新增 065：colorBlend 颜色混合 */
@Component
struct ImageColorBlendScene {
  @State currentColorIndex: number = 0
  @State currentModeIndex: number = 0
  @State opacity: number = 1.0

  private readonly BLEND_COLORS: Array<{ color: ResourceColor; label: string }> = [
    { color: Color.Red, label: '红' },
    { color: Color.Green, label: '绿' },
    { color: Color.Blue, label: '蓝' },
    { color: Color.Yellow, label: '黄' },
    { color: Color.Cyan, label: '青' },
    { color: Color.Magenta, label: '品红' },
    { color: Color.White, label: '白' },
    { color: Color.Black, label: '黑' },
    { color: '#808080', label: '灰' },
    { color: '#FFA500', label: '橙' },
    { color: '#800080', label: '紫' },
    { color: undefined, label: '无混合' }
  ]

  private readonly RENDER_MODES: Array<{ mode: ImageRenderMode; label: string }> = [
    { mode: ImageRenderMode.Original, label: 'Original' },
    { mode: ImageRenderMode.Template, label: 'Template' }
  ]

  build() {
    Column() {
      Text('Image colorBlend颜色混合测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Image($r('app.media.startIcon'))
          .width(130)
          .height(130)
          .objectFit(ImageFit.Cover)
          .colorBlend(this.BLEND_COLORS[this.currentColorIndex].color === undefined ?
              Color.Transparent : this.BLEND_COLORS[this.currentColorIndex].color)
          .opacity(this.opacity)
          .renderMode(this.RENDER_MODES[this.currentModeIndex].mode)
          .border({ width: 2, color: Color.Blue })
          .borderRadius(8)
          .margin({ right: 16 })

        Column({ space: 6 }) {
          Text('当前配置')
            .fontSize(13)
            .fontWeight(FontWeight.Medium)
          Text(`混合色: ${this.BLEND_COLORS[this.currentColorIndex].label}`)
            .fontSize(11)
            .fontColor(Color.Gray)
          Text(`渲染模式: ${this.RENDER_MODES[this.currentModeIndex].label}`)
            .fontSize(11)
            .fontColor(this.RENDER_MODES[this.currentModeIndex].mode === ImageRenderMode.Template ?
                Color.Red : Color.Gray)
          Text(`透明度: ${(this.opacity * 100).toFixed(0)}%`)
            .fontSize(11)
            .fontColor(Color.Gray)
          Text('colorBlend 叠加颜色到图片')
            .fontSize(9)
            .fontColor(Color.Orange)
        }
        .alignItems(HorizontalAlign.Start)
      }
      .margin({ bottom: 16 })

      Divider()

      Text('混合色选择')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Grid() {
        ForEach(this.BLEND_COLORS, (config: { color: ResourceColor; label: string }, index: number) => {
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
        .margin({ bottom: 8 })

      Row({ space: 8 }) {
        Slider({ value: this.opacity * 100, min: 0, max: 100, step: 10 })
          .width(200)
          .onChange((value: number) => { this.opacity = value / 100 })
      }

      Divider()

      Text('色卡矩阵(colorBlend × 12色)')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Grid() {
        ForEach(this.BLEND_COLORS, (config: { color: ResourceColor; label: string }) => {
          GridItem() {
            Image($r('app.media.startIcon'))
              .width(40)
              .height(40)
              .colorBlend(config.color === undefined ? Color.Transparent : config.color)
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

/** 新增 066：backdropBlur 背景模糊 */
@Component
struct ImageBackdropBlurScene {
  @State blurRadius: number = 0
  @State currentPresetIndex: number = 0
  @State imageWidth: number = 200
  @State imageHeight: number = 200

  private readonly BLUR_PRESETS: Array<{ radius: number; label: string }> = [
    { radius: 0, label: '无模糊' },
    { radius: 5, label: '轻微(5)' },
    { radius: 10, label: '低(10)' },
    { radius: 20, label: '中(20)' },
    { radius: 40, label: '高(40)' },
    { radius: 80, label: '强(80)' },
    { radius: 120, label: '极强(120)' },
    { radius: 200, label: '最大(200)' }
  ]

  build() {
    Column() {
      Text('Image backdropBlur背景模糊测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Stack() {
        Column()
          .width('100%')
          .height(220)
          .backgroundColor('#E3F2FD')
          .borderRadius(8)

        Row({ space: 16 }) {
          Column() {
            Text('backdropBlur')
              .fontSize(11)
              .fontColor(Color.Gray)
              .margin({ bottom: 4 })
            Image($r('app.media.startIcon'))
              .width(this.imageWidth)
              .height(this.imageHeight)
              .objectFit(ImageFit.Cover)
              .backdropBlur(this.blurRadius)
              .border({ width: 2, color: Color.Blue })
              .borderRadius(8)
          }

          Column() {
            Text('对照(无blur)')
              .fontSize(11)
              .fontColor(Color.Gray)
              .margin({ bottom: 4 })
            Image($r('app.media.startIcon'))
              .width(this.imageWidth)
              .height(this.imageHeight)
              .objectFit(ImageFit.Cover)
              .border({ width: 1, color: Color.Gray })
              .borderRadius(8)
          }
        }
      }
      .width('100%')
      .height(240)
      .margin({ bottom: 16 })

      Text(`当前模糊半径: ${this.blurRadius}`)
        .fontSize(12)
        .fontColor(Color.Blue)
        .margin({ bottom: 4 })
      Text(`预设: ${this.BLUR_PRESETS[this.currentPresetIndex].label}`)
        .fontSize(11)
        .fontColor(Color.Gray)
        .margin({ bottom: 12 })

      Divider()

      Text('模糊半径预设')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Row() {
        ForEach(this.BLUR_PRESETS, (preset: { radius: number; label: string }, index: number) => {
          Button(preset.label)
            .fontSize(9)
            .onClick(() => {
              this.currentPresetIndex = index
              this.blurRadius = preset.radius
            })
            .backgroundColor(index === this.currentPresetIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentPresetIndex ? Color.White : Color.Gray)
            .margin({ left: 2, right: 2 })
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('滑动调节半径')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Row({ space: 12 }) {
        Slider({ value: this.blurRadius, min: 0, max: 200, step: 1 })
          .width('60%')
          .onChange((value: number) => {
            this.blurRadius = value
            this.currentPresetIndex = -1
          })
        Text(`${this.blurRadius.toFixed(0)}px`)
          .fontSize(12)
          .fontColor(Color.Blue)
          .width(60)
      }
      .margin({ bottom: 12 })

      Divider()

      Text('尺寸调节(观察模糊范围)')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Row({ space: 8 }) {
        Button('100x100')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 100; this.imageHeight = 100 })
        Button('200x200')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 200; this.imageHeight = 200 })
        Button('300x150')
          .fontSize(10)
          .onClick(() => { this.imageWidth = 300; this.imageHeight = 150 })
        Button('重置')
          .fontSize(10)
          .backgroundColor('#FF6666')
          .fontColor(Color.White)
          .onClick(() => {
            this.blurRadius = 0
            this.currentPresetIndex = 0
            this.imageWidth = 200
            this.imageHeight = 200
          })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

@Entry
@Component
struct ImageTestcaseTransformEffects {
  @State currentIndex: number = 0

  build() {
    Column() {
      Tabs({ barPosition: BarPosition.Start, index: this.currentIndex }) {
        TabContent() { ImageTransformScene() }
          .tabBar('transform')
        TabContent() { ImageShadowScene() }
          .tabBar('shadow')
        TabContent() { ImagePositionScene() }
          .tabBar('position')
        TabContent() { ImageVisibilityScene() }
          .tabBar('visibility')
        TabContent() { ImageColorBlendScene() }
          .tabBar('colorBlend')
        TabContent() { ImageBackdropBlurScene() }
          .tabBar('backdropBlur')
      }
      .barMode(BarMode.Scrollable)
      .scrollable(true)
      .onChange((index: number) => { this.currentIndex = index })
    }
    .width('100%')
    .height('100%')
  }
}
