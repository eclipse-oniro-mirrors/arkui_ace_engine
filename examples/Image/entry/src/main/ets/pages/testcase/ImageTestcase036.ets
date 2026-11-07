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
struct ImageBlendModeScene {
  @State currentModeIndex: number = 0
  @State currentBgIndex: number = 0
  @State currentApplyIndex: number = 0

  private readonly BLEND_MODES: Array<{ mode: BlendMode; label: string }> = [
    { mode: BlendMode.NONE, label: 'NONE' },
    { mode: BlendMode.CLEAR, label: 'CLEAR' },
    { mode: BlendMode.SRC, label: 'SRC' },
    { mode: BlendMode.DST, label: 'DST' },
    { mode: BlendMode.SRC_OVER, label: 'SRC_OVER' },
    { mode: BlendMode.DST_OVER, label: 'DST_OVER' },
    { mode: BlendMode.SRC_IN, label: 'SRC_IN' },
    { mode: BlendMode.DST_IN, label: 'DST_IN' },
    { mode: BlendMode.SRC_OUT, label: 'SRC_OUT' },
    { mode: BlendMode.DST_OUT, label: 'DST_OUT' },
    { mode: BlendMode.SRC_ATOP, label: 'SRC_ATOP' },
    { mode: BlendMode.DST_ATOP, label: 'DST_ATOP' },
    { mode: BlendMode.XOR, label: 'XOR' },
    { mode: BlendMode.PLUS, label: 'PLUS' }
  ]

  private readonly BG_COLORS: Array<{ color: ResourceColor; label: string }> = [
    { color: '#FF6B6B', label: '珊瑚红' },
    { color: '#4ECDC4', label: '薄荷绿' },
    { color: '#FFE66D', label: '柠檬黄' },
    { color: '#6C5CE7', label: '紫罗兰' },
    { color: '#000000', label: '纯黑' },
    { color: '#FFFFFF', label: '纯白' }
  ]

  private readonly APPLY_TYPES: Array<{ type: BlendApplyType; label: string }> = [
    { type: BlendApplyType.QUICK_VIEW, label: 'QUICK_VIEW' },
    { type: BlendApplyType.OFFSCREEN, label: 'OFFSCREEN' }
  ]

  build() {
    Column() {
      Text('Image blendMode混合模式测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Stack() {
          Column()
            .width(140)
            .height(140)
            .backgroundColor(this.BG_COLORS[this.currentBgIndex].color)
            .borderRadius(8)
          Image($r('app.media.startIcon'))
            .width(140)
            .height(140)
            .objectFit(ImageFit.Cover)
            .blendMode(
              this.BLEND_MODES[this.currentModeIndex].mode,
              this.APPLY_TYPES[this.currentApplyIndex].type
            )
            .border({ width: 2, color: Color.Blue })
            .borderRadius(8)
        }
        .margin({ right: 16 })

        Column({ space: 6 }) {
          Text('当前配置')
            .fontSize(13)
            .fontWeight(FontWeight.Medium)
          Text(`blendMode: ${this.BLEND_MODES[this.currentModeIndex].label}`)
            .fontSize(11)
            .fontColor(Color.Blue)
          Text(`背景: ${this.BG_COLORS[this.currentBgIndex].label}`)
            .fontSize(11)
            .fontColor(Color.Gray)
          Text(`apply: ${this.APPLY_TYPES[this.currentApplyIndex].label}`)
            .fontSize(11)
            .fontColor(Color.Gray)
          Text('提示: 混合图片与背景色')
            .fontSize(9)
            .fontColor(Color.Orange)
        }
        .alignItems(HorizontalAlign.Start)
      }
      .margin({ bottom: 16 })

      Divider()

      Text('混合模式')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Grid() {
        ForEach(this.BLEND_MODES, (config: { mode: BlendMode; label: string }, index: number) => {
          GridItem() {
            Column() {
              Text(config.label)
                .fontSize(9)
                .fontColor(index === this.currentModeIndex ? Color.Blue : Color.Gray)
            }
            .padding(6)
            .backgroundColor(index === this.currentModeIndex ? '#E3F2FD' : '#FAFAFA')
            .borderRadius(4)
            .onClick(() => { this.currentModeIndex = index })
          }
        })
      }
      .columnsTemplate('repeat(7, 1fr)')
      .rowsGap(6)
      .columnsGap(6)
      .padding(8)

      Divider()

      Text('背景色选择')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Row() {
        ForEach(this.BG_COLORS, (config: { color: ResourceColor; label: string }, index: number) => {
          Column()
            .width(28)
            .height(28)
            .backgroundColor(config.color)
            .borderRadius(4)
            .border({ width: index === this.currentBgIndex ? 2 : 1,
                color: index === this.currentBgIndex ? Color.Blue : Color.Gray })
            .margin({ left: 4, right: 4 })
            .onClick(() => { this.currentBgIndex = index })
        })
      }
      .margin({ bottom: 12 })

      Text('应用类型')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ bottom: 8 })

      Row({ space: 8 }) {
        ForEach(this.APPLY_TYPES, (config: { type: BlendApplyType; label: string }, index: number) => {
          Button(config.label)
            .fontSize(10)
            .onClick(() => { this.currentApplyIndex = index })
            .backgroundColor(index === this.currentApplyIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentApplyIndex ? Color.White : Color.Gray)
        })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 新增 068：enabled 禁用状态 */
@Component
struct ImageEnabledScene {
  @State enabled: boolean = true
  @State opacity: number = 1.0
  @State grayscale: number = 0
  @State log: string = '禁用状态下观察交互响应'
  @State clickCount: number = 0

  build() {
    Column() {
      Text('Image enabled禁用状态测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        Image($r('app.media.startIcon'))
          .width(140)
          .height(140)
          .objectFit(ImageFit.Cover)
          .enabled(this.enabled)
          .opacity(this.opacity)
          .grayscale(this.grayscale)
          .border({ width: 2, color: this.enabled ? Color.Blue : Color.Gray })
          .borderRadius(8)
          .onClick(() => {
            this.clickCount++
            this.log = `点击响应(${this.clickCount}) - ${this.enabled ? '启用态' : '禁用态(不应响应)'}`
          })
          .margin({ right: 16 })

        Column({ space: 8 }) {
          Text('当前状态')
            .fontSize(13)
            .fontWeight(FontWeight.Medium)
          Text(`enabled: ${this.enabled}`)
            .fontSize(12)
            .fontColor(this.enabled ? Color.Green : Color.Red)
          Text(`opacity: ${(this.opacity * 100).toFixed(0)}%`)
            .fontSize(12)
            .fontColor(Color.Gray)
          Text(`grayscale: ${this.grayscale}`)
            .fontSize(12)
            .fontColor(Color.Gray)
          Text(`点击次数: ${this.clickCount}`)
            .fontSize(12)
            .fontColor(Color.Orange)
        }
        .alignItems(HorizontalAlign.Start)
      }
      .margin({ bottom: 16 })

      Text(this.log)
        .fontSize(11)
        .fontColor(this.enabled ? Color.Green : Color.Red)
        .margin({ bottom: 12 })

      Divider()

      Text('状态切换')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Row({ space: 12 }) {
        Button('启用')
          .fontSize(11)
          .backgroundColor('#4CAF50')
          .fontColor(Color.White)
          .onClick(() => {
            this.enabled = true
            this.opacity = 1.0
            this.grayscale = 0
            this.log = '已启用,可交互'
          })
        Button('禁用')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .fontColor(Color.White)
          .onClick(() => {
            this.enabled = false
            this.opacity = 0.4
            this.grayscale = 1
            this.log = '已禁用,不可交互(灰度+半透明)'
          })
        Button('重置计数')
          .fontSize(11)
          .backgroundColor('#9E9E9E')
          .fontColor(Color.White)
          .onClick(() => {
            this.clickCount = 0
            this.log = '计数已重置'
          })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('禁用样式预设')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Row({ space: 8 }) {
        Button('半透明')
          .fontSize(10)
          .onClick(() => { this.enabled = false; this.opacity = 0.5; this.grayscale = 0 })
        Button('灰度')
          .fontSize(10)
          .onClick(() => { this.enabled = false; this.opacity = 1.0; this.grayscale = 1 })
        Button('灰度+半透明')
          .fontSize(10)
          .onClick(() => { this.enabled = false; this.opacity = 0.4; this.grayscale = 1 })
        Button('不明显')
          .fontSize(10)
          .onClick(() => { this.enabled = false; this.opacity = 0.9; this.grayscale = 0.3 })
      }

      Text('多图批量启用/禁用')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row() {
        ForEach([0, 1, 2, 3, 4], (i: number) => {
          Image($r('app.media.startIcon'))
            .width(50)
            .height(50)
            .objectFit(ImageFit.Cover)
            .enabled(i % 2 === 0 ? this.enabled : !this.enabled)
            .opacity(i % 2 === 0 ? 1.0 : 0.4)
            .borderRadius(6)
            .border({ width: 1, color: i % 2 === 0 ? Color.Blue : Color.Gray })
            .margin({ left: 4, right: 4 })
        })
      }
      Text('↑ 奇数项恒定禁用,偶数项跟随主控制')
        .fontSize(9)
        .fontColor(Color.Gray)
        .margin({ top: 4 })
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 新增 069：hitTestBehavior 命中测试 */
@Component
struct ImageHitTestScene {
  @State currentModeIndex: number = 0
  @State log: string = '点击不同区域观察命中行为'
  @State innerClicks: number = 0
  @State outerClicks: number = 0

  private readonly HIT_MODES: Array<{ mode: HitTestMode; label: string; desc: string }> = [
    { mode: HitTestMode.Default, label: 'Default', desc: '默认:自身+子节点可命中' },
    { mode: HitTestMode.Block, label: 'Block', desc: '阻断:自身命中,阻断下层' },
    { mode: HitTestMode.Transparent, label: 'Transparent', desc: '透明:自身+透传下层' },
    { mode: HitTestMode.None, label: 'None', desc: '不命中:自身不响应,透传' }
  ]

  build() {
    Column() {
      Text('Image hitTestBehavior命中测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Stack() {
        Column()
          .width('100%')
          .height(220)
          .backgroundColor('#E3F2FD')
          .borderRadius(8)
          .onClick(() => {
            this.outerClicks++
            this.log = `外层命中(${this.outerClicks}) - 当前模式: ${this.HIT_MODES[this.currentModeIndex].label}`
          })

        Column() {
          Image($r('app.media.startIcon'))
            .width(140)
            .height(140)
            .objectFit(ImageFit.Cover)
            .hitTestBehavior(this.HIT_MODES[this.currentModeIndex].mode)
            .border({ width: 2, color: Color.Red })
            .borderRadius(8)
            .onClick(() => {
              this.innerClicks++
              this.log = `图片命中(${this.innerClicks}) - 模式: ${this.HIT_MODES[this.currentModeIndex].label}`
            })
        }
      }
      .width('100%')
      .height(240)
      .margin({ bottom: 16 })

      Text(this.log)
        .fontSize(11)
        .fontColor(this.log.startsWith('图片') ? Color.Red : Color.Blue)
        .margin({ bottom: 8 })

      Row({ space: 24 }) {
        Column() {
          Text('图片命中')
            .fontSize(10)
            .fontColor(Color.Gray)
          Text(`${this.innerClicks}`)
            .fontSize(16)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.Red)
        }
        Column() {
          Text('外层命中')
            .fontSize(10)
            .fontColor(Color.Gray)
          Text(`${this.outerClicks}`)
            .fontSize(16)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.Blue)
        }
      }
      .margin({ bottom: 12 })

      Divider()

      Text('命中模式(点击图片区域测试)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Row() {
        ForEach(this.HIT_MODES, (config: { mode: HitTestMode; label: string; desc: string }, index: number) => {
          Button(config.label)
            .fontSize(10)
            .onClick(() => {
              this.currentModeIndex = index
              this.log = `切换为 ${config.label}: ${config.desc}`
            })
            .backgroundColor(index === this.currentModeIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentModeIndex ? Color.White : Color.Gray)
            .margin({ left: 4, right: 4 })
        })
      }
      .margin({ bottom: 12 })

      Text(`说明: ${this.HIT_MODES[this.currentModeIndex].desc}`)
        .fontSize(10)
        .fontColor(Color.Gray)
        .margin({ bottom: 12 })

      Button('重置计数')
        .fontSize(11)
        .backgroundColor('#FF6666')
        .fontColor(Color.White)
        .onClick(() => {
          this.innerClicks = 0
          this.outerClicks = 0
          this.log = '计数已重置'
        })
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 新增 070：事件回调 onClick / onTouch / onHover */
@Component
struct ImageEventScene {
  @State clickCount: number = 0
  @State touchCount: number = 0
  @State hoverCount: number = 0
  @State keyCount: number = 0
  @State lastEvent: string = '对图片进行操作观察事件'
  @State isHovering: boolean = false

  build() {
    Column() {
      Text('Image 事件回调测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Image($r('app.media.startIcon'))
        .width(160)
        .height(160)
        .objectFit(ImageFit.Cover)
        .border({
          width: 3,
          color: this.isHovering ? Color.Green : Color.Blue
        })
        .borderRadius(8)
        .backgroundColor(this.isHovering ? '#E8F5E9' : '#F5F5F5')
        .onClick(() => {
          this.clickCount++
          this.lastEvent = `onClick 触发(${this.clickCount})`
        })
        .onTouch((event: TouchEvent) => {
          if (event.type === TouchType.Down) {
            this.touchCount++
            this.lastEvent = `onTouch Down(${this.touchCount})`
          }
        })
        .onHover((isHover: boolean) => {
          this.hoverCount++
          this.isHovering = isHover
          this.lastEvent = `onHover ${isHover ? '进入' : '离开'}(${this.hoverCount})`
        })
        .onKeyEvent((event: KeyEvent) => {
          if (event.type === KeyType.Down) {
            this.keyCount++
            this.lastEvent = `onKeyEvent ${event.keyText}(${this.keyCount})`
          }
        })
        .focusable(true)
        .margin({ bottom: 16 })

      Text(this.lastEvent)
        .fontSize(13)
        .fontColor(this.isHovering ? Color.Green : Color.Orange)
        .margin({ bottom: 12 })

      Row({ space: 16 }) {
        Column() {
          Text('onClick')
            .fontSize(10)
            .fontColor(Color.Gray)
          Text(`${this.clickCount}`)
            .fontSize(16)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.Blue)
        }
        Column() {
          Text('onTouch')
            .fontSize(10)
            .fontColor(Color.Gray)
          Text(`${this.touchCount}`)
            .fontSize(16)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.Green)
        }
        Column() {
          Text('onHover')
            .fontSize(10)
            .fontColor(Color.Gray)
          Text(`${this.hoverCount}`)
            .fontSize(16)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.Orange)
        }
        Column() {
          Text('onKey')
            .fontSize(10)
            .fontColor(Color.Gray)
          Text(`${this.keyCount}`)
            .fontSize(16)
            .fontWeight(FontWeight.Bold)
            .fontColor(Color.Purple)
        }
      }
      .margin({ bottom: 12 })

      Divider()

      Text('事件说明')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Text('• 点击: 触发 onClick + onTouch Down/Up')
        .fontSize(10)
        .fontColor(Color.Gray)
        .margin({ bottom: 2 })
      Text('• 悬停: 鼠标移入移出触发 onHover(设备支持)')
        .fontSize(10)
        .fontColor(Color.Gray)
        .margin({ bottom: 2 })
      Text('• 键盘: 聚焦后按键触发 onKeyEvent')
        .fontSize(10)
        .fontColor(Color.Gray)
        .margin({ bottom: 12 })

      Row({ space: 12 }) {
        Button('重置计数')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .fontColor(Color.White)
          .onClick(() => {
            this.clickCount = 0
            this.touchCount = 0
            this.hoverCount = 0
            this.keyCount = 0
            this.lastEvent = '计数已重置'
          })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 新增 071：渐变叠加 linear / sweep / radial */
@Component
struct ImageGradientScene {
  @State currentTypeIndex: number = 0
  @State currentColorIndex: number = 0
  @State currentDirIndex: number = 0

  private readonly GRADIENT_TYPES: Array<{ type: number; label: string }> = [
    { type: 0, label: 'linear(线性)' },
    { type: 1, label: 'sweep(扫描)' },
    { type: 2, label: 'radial(径向)' }
  ]

  private readonly COLOR_SETS: Array<{ colors: Array<[ResourceColor, number]>; label: string }> = [
    { colors: [[Color.Red, 0], [Color.Blue, 1]], label: '红蓝' },
    { colors: [[Color.Yellow, 0], [Color.Green, 0.5], [Color.Cyan, 1]], label: '黄绿青' },
    { colors: [['#FF6B6B', 0], ['#4ECDC4', 1]], label: '珊瑚薄荷' },
    { colors: [[Color.White, 0], [Color.Black, 1]], label: '白黑' },
    { colors: [['#6C5CE7', 0], ['#FFE66D', 1]], label: '紫黄' },
    { colors: [[Color.Transparent, 0], [Color.Red, 1]], label: '透明红' }
  ]

  private readonly DIRECTIONS: Array<{ direction: GradientDirection; label: string }> = [
    { direction: GradientDirection.Left, label: '向左' },
    { direction: GradientDirection.Right, label: '向右' },
    { direction: GradientDirection.Top, label: '向上' },
    { direction: GradientDirection.Bottom, label: '向下' },
    { direction: GradientDirection.LeftTop, label: '左上' },
    { direction: GradientDirection.RightBottom, label: '右下' }
  ]

  @Builder
  buildImageWithGradient() {
    Image($r('app.media.startIcon'))
      .width(150)
      .height(150)
      .objectFit(ImageFit.Cover)
      .border({ width: 2, color: Color.Blue })
      .borderRadius(8)
      .linearGradient({
        angle: 45,
        direction: this.DIRECTIONS[this.currentDirIndex].direction,
        colors: this.COLOR_SETS[this.currentColorIndex].colors,
        repeating: false
      })
  }

  build() {
    Column() {
      Text('Image 渐变叠加测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Row() {
        if (this.GRADIENT_TYPES[this.currentTypeIndex].type === 0) {
          Image($r('app.media.startIcon'))
            .width(150)
            .height(150)
            .objectFit(ImageFit.Cover)
            .border({ width: 2, color: Color.Blue })
            .borderRadius(8)
            .linearGradient({
              angle: 45,
              direction: this.DIRECTIONS[this.currentDirIndex].direction,
              colors: this.COLOR_SETS[this.currentColorIndex].colors,
              repeating: false
            })
        } else if (this.GRADIENT_TYPES[this.currentTypeIndex].type === 1) {
          Image($r('app.media.startIcon'))
            .width(150)
            .height(150)
            .objectFit(ImageFit.Cover)
            .border({ width: 2, color: Color.Blue })
            .borderRadius(8)
            .sweepGradient({
              center: [75, 75],
              start: 0,
              end: 360,
              rotation: 0,
              colors: this.COLOR_SETS[this.currentColorIndex].colors,
              repeating: false
            })
        } else {
          Image($r('app.media.startIcon'))
            .width(150)
            .height(150)
            .objectFit(ImageFit.Cover)
            .border({ width: 2, color: Color.Blue })
            .borderRadius(8)
            .radialGradient({
              center: [75, 75],
              radius: 75,
              colors: this.COLOR_SETS[this.currentColorIndex].colors,
              repeating: false
            })
        }

        Column({ space: 6 }) {
          Text('当前配置')
            .fontSize(13)
            .fontWeight(FontWeight.Medium)
          Text(`类型: ${this.GRADIENT_TYPES[this.currentTypeIndex].label}`)
            .fontSize(11)
            .fontColor(Color.Blue)
          Text(`配色: ${this.COLOR_SETS[this.currentColorIndex].label}`)
            .fontSize(11)
            .fontColor(Color.Gray)
          if (this.GRADIENT_TYPES[this.currentTypeIndex].type === 0) {
            Text(`方向: ${this.DIRECTIONS[this.currentDirIndex].label}`)
              .fontSize(11)
              .fontColor(Color.Gray)
          }
          Text('渐变叠加在图片之上')
            .fontSize(9)
            .fontColor(Color.Orange)
        }
        .alignItems(HorizontalAlign.Start)
        .margin({ left: 16 })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('渐变类型')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Row() {
        ForEach(this.GRADIENT_TYPES, (config: { type: number; label: string }, index: number) => {
          Button(config.label)
            .fontSize(10)
            .onClick(() => { this.currentTypeIndex = index })
            .backgroundColor(index === this.currentTypeIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentTypeIndex ? Color.White : Color.Gray)
            .margin({ left: 4, right: 4 })
        })
      }
      .margin({ bottom: 12 })

      Text('配色方案')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ bottom: 8 })

      Grid() {
        ForEach(this.COLOR_SETS, (config: { colors: Array<[ResourceColor, number]>; label: string }, index: number) => {
          GridItem() {
            Column() {
              Row() {
                ForEach(config.colors, (c: [ResourceColor, number]) => {
                  Column()
                    .width(16)
                    .height(16)
                    .backgroundColor(c[0])
                    .border({ width: 1, color: Color.Gray })
                })
              }
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
      .columnsTemplate('repeat(3, 1fr)')
      .rowsGap(6)
      .columnsGap(6)
      .padding(8)

      if (this.GRADIENT_TYPES[this.currentTypeIndex].type === 0) {
        Divider()

        Text('方向(linear专用)')
          .fontSize(13)
          .fontWeight(FontWeight.Medium)
          .margin({ top: 12, bottom: 8 })

        Row() {
          ForEach(this.DIRECTIONS, (config: { direction: GradientDirection; label: string }, index: number) => {
            Button(config.label)
              .fontSize(9)
              .onClick(() => { this.currentDirIndex = index })
              .backgroundColor(index === this.currentDirIndex ? '#2196F3' : '#EEEEEE')
              .fontColor(index === this.currentDirIndex ? Color.White : Color.Gray)
              .margin({ left: 2, right: 2 })
          })
        }
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 新增 072：opacity 分层叠加 */
@Component
struct ImageOpacityLayerScene {
  @State baseOpacity: number = 1.0
  @State layerCount: number = 3
  @State currentPresetIndex: number = 0
  @State stackOffset: number = 30

  private readonly OPACITY_PRESETS: Array<{
    name: string; base: number; count: number; offset: number
  }> = [
    { name: '3层正常', base: 0.5, count: 3, offset: 30 },
    { name: '5层渐淡', base: 0.3, count: 5, offset: 25 },
    { name: '2层对比', base: 0.8, count: 2, offset: 50 },
    { name: '4层紧密', base: 0.4, count: 4, offset: 15 },
    { name: '6层堆叠', base: 0.25, count: 6, offset: 12 },
    { name: '1层(单图)', base: 1.0, count: 1, offset: 0 }
  ]

  build() {
    Column() {
      Text('Image opacity分层叠加测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Stack() {
        ForEach([0, 1, 2, 3, 4, 5], (i: number) => {
          if (i < this.layerCount) {
            Image($r('app.media.startIcon'))
              .width(120)
              .height(120)
              .objectFit(ImageFit.Cover)
              .opacity(this.baseOpacity)
              .border({ width: 1, color: `rgba(33,150,243,${0.3 + i * 0.1})` })
              .borderRadius(8)
              .position({ x: 100 + i * this.stackOffset, y: 40 + i * this.stackOffset })
          }
        })
      }
      .width('100%')
      .height(220)
      .backgroundColor('#F5F5F5')
      .borderRadius(8)
      .margin({ bottom: 16 })

      Text(`层数:${this.layerCount} 透明度:${(this.baseOpacity * 100).toFixed(0)}% 偏移:${this.stackOffset}`)
        .fontSize(12)
        .fontColor(Color.Gray)
        .margin({ bottom: 4 })
      Text(`预设: ${this.OPACITY_PRESETS[this.currentPresetIndex].name}`)
        .fontSize(11)
        .fontColor(Color.Blue)
        .margin({ bottom: 12 })

      Divider()

      Text('分层预设')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Grid() {
        ForEach(this.OPACITY_PRESETS, (preset: {
          name: string; base: number; count: number; offset: number
        }, index: number) => {
          GridItem() {
            Column() {
              Text(preset.name)
                .fontSize(10)
                .fontColor(index === this.currentPresetIndex ? Color.Blue : Color.Gray)
            }
            .padding(8)
            .backgroundColor(index === this.currentPresetIndex ? '#E3F2FD' : '#FAFAFA')
            .borderRadius(4)
            .onClick(() => {
              this.currentPresetIndex = index
              this.baseOpacity = preset.base
              this.layerCount = preset.count
              this.stackOffset = preset.offset
            })
          }
        })
      }
      .columnsTemplate('repeat(3, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(8)

      Divider()

      Text('透明度调节')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 12, bottom: 8 })

      Row({ space: 12 }) {
        Slider({ value: this.baseOpacity * 100, min: 10, max: 100, step: 5 })
          .width('60%')
          .onChange((value: number) => { this.baseOpacity = value / 100 })
        Text(`${(this.baseOpacity * 100).toFixed(0)}%`)
          .fontSize(12)
          .fontColor(Color.Blue)
          .width(50)
      }
      .margin({ bottom: 12 })

      Text('层数调节')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ bottom: 8 })

      Row({ space: 8 }) {
        Button('-1')
          .fontSize(10)
          .onClick(() => { this.layerCount = Math.max(1, this.layerCount - 1) })
        Text(`${this.layerCount}`)
          .fontSize(14)
          .fontWeight(FontWeight.Bold)
          .fontColor(Color.Blue)
          .width(30)
          .textAlign(TextAlign.Center)
        Button('+1')
          .fontSize(10)
          .onClick(() => { this.layerCount = Math.min(6, this.layerCount + 1) })
        Button('重置')
          .fontSize(10)
          .backgroundColor('#FF6666')
          .fontColor(Color.White)
          .onClick(() => {
            this.currentPresetIndex = 0
            this.baseOpacity = 0.5
            this.layerCount = 3
            this.stackOffset = 30
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
struct ImageTestcaseStateEvents {
  @State currentIndex: number = 0

  build() {
    Column() {
      Tabs({ barPosition: BarPosition.Start, index: this.currentIndex }) {
        TabContent() { ImageBlendModeScene() }
          .tabBar('blendMode')
        TabContent() { ImageEnabledScene() }
          .tabBar('enabled')
        TabContent() { ImageHitTestScene() }
          .tabBar('hitTest')
        TabContent() { ImageEventScene() }
          .tabBar('事件回调')
        TabContent() { ImageGradientScene() }
          .tabBar('渐变叠加')
        TabContent() { ImageOpacityLayerScene() }
          .tabBar('opacity分层')
      }
      .barMode(BarMode.Scrollable)
      .scrollable(true)
      .onChange((index: number) => { this.currentIndex = index })
    }
    .width('100%')
    .height('100%')
  }
}
