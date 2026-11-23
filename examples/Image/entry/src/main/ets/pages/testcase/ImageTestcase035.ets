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
struct ImageAsBackgroundScene {
  @State bgIndex: number = 0
  @State blurValue: number = 0
  @State showContent: boolean = true
  @State overlayColor: number = 0
  private readonly BGS: Resource[] = [
    $r('app.media.startIcon'),
    $r('app.media.background'),
    $r('app.media.foreground')
  ]
  private readonly OVERLAYS: Array<ResourceColor> = [
    Color.Transparent, '#80000000', '#60000000', '#40336688'
  ]

  build() {
    Column({ space: 12 }) {
      Text('Image 作背景测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)

      Stack() {
        Image(this.BGS[this.bgIndex])
          .width('100%')
          .height(280)
          .objectFit(ImageFit.Cover)
          .blur(this.blurValue)

        Column()
          .width('100%')
          .height(280)
          .backgroundColor(this.OVERLAYS[this.overlayColor])

        if (this.showContent) {
          Column({ space: 10 }) {
            Text('前景内容层')
              .fontSize(22)
              .fontColor(Color.White)
              .fontWeight(FontWeight.Bold)
            Text('底层图片作背景,可叠加模糊与遮罩,前景文字按钮浮于其上')
              .fontSize(12)
              .fontColor('#F0F0F0')
              .textAlign(TextAlign.Center)
            Row({ space: 8 }) {
              Button('前景按钮 A')
                .fontSize(11)
              Button('前景按钮 B')
                .fontSize(11)
                .backgroundColor('#FF6666')
            }
          }
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
        }
      }
      .width('100%')
      .height(280)
      .borderRadius(12)
      .clip(true)

      Text(`背景索引:${this.bgIndex}  模糊:${this.blurValue}  遮罩:${this.overlayColor}`)
        .fontSize(12)
        .fontColor(Color.Gray)

      Row({ space: 8 }) {
        Button('换背景')
          .fontSize(11)
          .onClick(() => {
            this.bgIndex = (this.bgIndex + 1) % this.BGS.length
          })
        Button('模糊+5')
          .fontSize(11)
          .onClick(() => {
            this.blurValue = Math.min(25, this.blurValue + 5)
          })
        Button('模糊-5')
          .fontSize(11)
          .onClick(() => {
            this.blurValue = Math.max(0, this.blurValue - 5)
          })
        Button('切遮罩')
          .fontSize(11)
          .onClick(() => {
            this.overlayColor = (this.overlayColor + 1) % this.OVERLAYS.length
          })
        Button(this.showContent ? '隐藏前景' : '显示前景')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .onClick(() => {
            this.showContent = !this.showContent
          })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 并发加载计时 */
@Component
struct ImageParallelLoadScene {
  @State toggle: boolean = false
  @State startMs: number = 0
  @State durations: number[] = [0, 0, 0, 0, 0, 0]
  @State loaded: number = 0
  @State totalMs: number = 0

  private readonly SET_A: Resource[] = [
    $r('app.media.startIcon'), $r('app.media.background'), $r('app.media.foreground'),
    $r('app.media.startIcon'), $r('app.media.background'), $r('app.media.foreground')
  ]
  private readonly SET_B: Resource[] = [
    $r('app.media.foreground'), $r('app.media.background'), $r('app.media.startIcon'),
    $r('app.media.foreground'), $r('app.media.startIcon'), $r('app.media.background')
  ]

  private startLoad(): void {
    this.durations = [0, 0, 0, 0, 0, 0]
    this.loaded = 0
    this.totalMs = 0
    this.startMs = Date.now()
    this.toggle = !this.toggle
  }

  private onOneDone(idx: number): void {
    if (this.durations[idx] === 0) {
      this.durations[idx] = Date.now() - this.startMs
      this.loaded++
      if (this.loaded >= 6) {
        this.totalMs = Math.max(...this.durations)
      }
    }
  }

  build() {
    Column({ space: 12 }) {
      Text('Image 并发加载计时测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)

      Text(`已加载:${this.loaded}/6  总耗时:${this.totalMs}ms`)
        .fontSize(13)
        .fontColor(this.totalMs > 0 ? Color.Green : Color.Orange)

      Grid() {
        ForEach(this.toggle ? this.SET_B : this.SET_A, (src: Resource, idx: number) => {
          GridItem() {
            Column({ space: 4 }) {
              Image(src)
                .width(80)
                .height(80)
                .objectFit(ImageFit.Cover)
                .borderRadius(8)
                .border({ width: 1, color: Color.Gray })
                .onComplete(() => this.onOneDone(idx))

              Text(`#${idx + 1}`)
                .fontSize(10)
                .fontColor(Color.Gray)
              Text(this.durations[idx] > 0 ? `${this.durations[idx]}ms` : '...')
                .fontSize(10)
                .fontColor(Color.Blue)
            }
          }
        }, (src: Resource, idx: number) => `${this.toggle}-${idx}`)
      }
      .columnsTemplate('repeat(3, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(8)

      Text('点击重新加载会切换图源集合触发重新解码,并记录每张耗时')
        .fontSize(11)
        .fontColor(Color.Orange)
        .margin({ top: 4 })

      Button('重新加载并计时')
        .fontSize(12)
        .backgroundColor('#FF6666')
        .onClick(() => this.startLoad())
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 水印叠加 */
@Component
struct ImageWatermarkScene {
  @State watermark: string = ' Confidential '
  @State opacity: number = 0.3
  @State angle: number = -30
  @State repeat: boolean = true
  @State wmSize: number = 14

  build() {
    Column({ space: 12 }) {
      Text('Image 水印叠加测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)

      Stack() {
        Image($r('app.media.startIcon'))
          .width('100%')
          .height(240)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)

        if (this.repeat) {
          Column() {
            ForEach([0, 1, 2, 3, 4], (_r: number) => {
              Row() {
                ForEach([0, 1, 2, 3], (_c: number) => {
                  Text(this.watermark)
                    .fontSize(this.wmSize)
                    .fontColor(Color.White)
                    .opacity(this.opacity)
                    .rotate({ angle: this.angle })
                    .margin(8)
                })
              }
            })
          }
          .width('100%')
          .height(240)
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
        } else {
          Text(this.watermark)
            .fontSize(this.wmSize + 8)
            .fontColor(Color.White)
            .opacity(this.opacity)
            .rotate({ angle: this.angle })
        }
      }
      .width('100%')
      .height(240)
      .clip(true)

      Text(`透明度:${this.opacity.toFixed(1)}  角度:${this.angle}°  字号:${this.wmSize}`)
        .fontSize(12)
        .fontColor(Color.Gray)

      Row({ space: 8 }) {
        Button('水印字+')
          .fontSize(11)
          .onClick(() => { this.wmSize = Math.min(24, this.wmSize + 2) })
        Button('水印字-')
          .fontSize(11)
          .onClick(() => { this.wmSize = Math.max(8, this.wmSize - 2) })
        Button('透明+0.1')
          .fontSize(11)
          .onClick(() => { this.opacity = Math.min(1, this.opacity + 0.1) })
        Button('透明-0.1')
          .fontSize(11)
          .onClick(() => { this.opacity = Math.max(0.1, this.opacity - 0.1) })
      }

      Row({ space: 8 }) {
        Button('角度+15')
          .fontSize(11)
          .onClick(() => { this.angle += 15 })
        Button('角度-15')
          .fontSize(11)
          .onClick(() => { this.angle -= 15 })
        Button(this.repeat ? '平铺:开' : '平铺:关')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .onClick(() => { this.repeat = !this.repeat })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 缩放查看器 */
@Component
struct ImagePinchViewerScene {
  @State scale: number = 1.0
  @State offsetX: number = 0
  @State offsetY: number = 0
  @State rotateAngle: number = 0
  @State log: string = '双指缩放/拖动/双击复位'

  private reset(): void {
    this.scale = 1.0
    this.offsetX = 0
    this.offsetY = 0
    this.rotateAngle = 0
    this.log = '已复位'
  }

  build() {
    Column({ space: 12 }) {
      Text('Image 缩放查看器测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)

      Stack() {
        Image($r('app.media.startIcon'))
          .width(180)
          .height(180)
          .objectFit(ImageFit.Cover)
          .scale({ x: this.scale, y: this.scale })
          .translate({ x: this.offsetX, y: this.offsetY })
          .rotate({ angle: this.rotateAngle })
          .border({ width: 2, color: Color.Blue })
          .borderRadius(8)
          .gesture(
            GestureGroup(GestureMode.Parallel,
              PinchGesture({ fingers: 2 })
                .onActionEnd((e: GestureEvent) => {
                  this.scale = Math.max(0.5, Math.min(4, this.scale * e.scale))
                  this.log = `缩放:${this.scale.toFixed(2)}x`
                }),
              PanGesture({ fingers: 1 })
                .onActionUpdate((e: GestureEvent) => {
                  this.offsetX += e.offsetX
                  this.offsetY += e.offsetY
                  this.log = `偏移:(${this.offsetX.toFixed(0)},${this.offsetY.toFixed(0)})`
                }),
              TapGesture({ count: 2 })
                .onAction(() => this.reset())
            )
          )
      }
      .width('100%')
      .height(260)

      Text(this.log)
        .fontSize(13)
        .fontColor(Color.Orange)

      Text(`scale:${this.scale.toFixed(2)} ` +
        `offset:(${this.offsetX.toFixed(0)},${this.offsetY.toFixed(0)}) ` +
        `rotate:${this.rotateAngle}°`)
        .fontSize(11)
        .fontColor(Color.Gray)

      Row({ space: 8 }) {
        Button('放大')
          .fontSize(11)
          .onClick(() => { this.scale = Math.min(4, this.scale + 0.3) })
        Button('缩小')
          .fontSize(11)
          .onClick(() => { this.scale = Math.max(0.5, this.scale - 0.3) })
        Button('旋转+30')
          .fontSize(11)
          .onClick(() => { this.rotateAngle += 30 })
        Button('旋转-30')
          .fontSize(11)
          .onClick(() => { this.rotateAngle -= 30 })
        Button('复位')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .onClick(() => this.reset())
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 缩略图画廊预览 */
@Component
struct ImageThumbnailGalleryScene {
  @State selected: number = -1
  private readonly GALLERY: Resource[] = [
    $r('app.media.startIcon'), $r('app.media.background'), $r('app.media.foreground'),
    $r('app.media.startIcon'), $r('app.media.background'), $r('app.media.foreground'),
    $r('app.media.startIcon'), $r('app.media.background')
  ]

  private prev(): void {
    if (this.selected > 0) {
      this.selected--
    } else {
      this.selected = this.GALLERY.length - 1
    }
  }

  private next(): void {
    this.selected = (this.selected + 1) % this.GALLERY.length
  }

  build() {
    Column({ space: 12 }) {
      Text('Image 缩略图画廊预览测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)

      Text('点击缩略图查看大图,支持上一张/下一张')
        .fontSize(11)
        .fontColor(Color.Gray)

      Grid() {
        ForEach(this.GALLERY, (src: Resource, idx: number) => {
          GridItem() {
            Stack() {
              Image(src)
                .width(90)
                .height(90)
                .objectFit(ImageFit.Cover)
                .borderRadius(8)

              Text(`${idx + 1}`)
                .fontSize(10)
                .fontColor(Color.White)
                .backgroundColor('#80000000')
                .borderRadius(10)
                .padding({ left: 5, right: 5 })
                .position({ x: 4, y: 4 })
            }
            .border({
              width: 2,
              color: this.selected === idx ? Color.Red : Color.Transparent
            })
            .borderRadius(10)
            .onClick(() => { this.selected = idx })
          }
        }, (src: Resource, idx: number) => `${idx}`)
      }
      .columnsTemplate('repeat(4, 1fr)')
      .rowsGap(8)
      .columnsGap(8)
      .padding(8)
      .layoutWeight(1)

      if (this.selected >= 0) {
        Stack() {
          Column()
            .width('100%')
            .height(260)
            .backgroundColor('#CC000000')

          Column({ space: 10 }) {
            Image(this.GALLERY[this.selected])
              .width(220)
              .height(220)
              .objectFit(ImageFit.Contain)
              .borderRadius(12)
              .border({ width: 2, color: Color.White })

            Text(`第 ${this.selected + 1} / ${this.GALLERY.length} 张`)
              .fontSize(12)
              .fontColor(Color.White)

            Row({ space: 16 }) {
              Button('上一张')
                .fontSize(11)
                .onClick(() => this.prev())
              Button('下一张')
                .fontSize(11)
                .onClick(() => this.next())
              Button('关闭')
                .fontSize(11)
                .backgroundColor('#FF6666')
                .onClick(() => { this.selected = -1 })
            }
          }
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
        }
        .width('100%')
        .height(260)
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 前后对比滑块 */
@Component
struct ImageSliderCompareScene {
  @State split: number = 150
  @State pairIndex: number = 0
  @State dragging: boolean = false

  private readonly PAIRS: Array<{ before: Resource; after: Resource; label: string }> = [
    { before: $r('app.media.startIcon'), after: $r('app.media.background'), label: '效果对比' },
    { before: $r('app.media.background'), after: $r('app.media.foreground'), label: '图源对比' },
    { before: $r('app.media.foreground'), after: $r('app.media.startIcon'), label: '反色对比' }
  ]

  build() {
    Column({ space: 12 }) {
      Text('Image 前后对比滑块测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)

      Text(`对比:${this.PAIRS[this.pairIndex].label}  分割:${(this.split / 3).toFixed(0)}%`)
        .fontSize(12)
        .fontColor(Color.Gray)

      Stack() {
        Image(this.PAIRS[this.pairIndex].after)
          .width(300)
          .height(200)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)

        Row() {
          Image(this.PAIRS[this.pairIndex].before)
            .width(300)
            .height(200)
            .objectFit(ImageFit.Cover)
        }
        .width(this.split)
        .height(200)
        .clip(true)

        Text('后')
          .fontSize(12)
          .fontColor(Color.White)
          .backgroundColor('#80000000')
          .borderRadius(8)
          .padding({ left: 6, right: 6 })
          .position({ x: 264, y: 8 })

        Text('前')
          .fontSize(12)
          .fontColor(Color.White)
          .backgroundColor('#80000000')
          .borderRadius(8)
          .padding({ left: 6, right: 6 })
          .position({ x: 8, y: 8 })

        Column() {
          Row() {
            Text('◀').fontSize(12).fontColor(Color.White)
            Text('▶').fontSize(12).fontColor(Color.White).margin({ left: 2 })
          }
          .padding({ left: 4, right: 4 })
          .backgroundColor(this.dragging ? '#FF6666' : '#99000000')
          .borderRadius(10)
        }
        .width(40)
        .height(200)
        .justifyContent(FlexAlign.Center)
        .alignItems(HorizontalAlign.Center)
        .position({ x: this.split - 20, y: 0 })
        .gesture(
          PanGesture({ fingers: 1 })
            .onActionBegin(() => { this.dragging = true })
            .onActionUpdate((e: GestureEvent) => {
              this.split = Math.max(0, Math.min(300, this.split + e.offsetX))
            })
            .onActionEnd(() => { this.dragging = false })
        )
      }
      .width(300)
      .height(200)

      Slider({ value: this.split, min: 0, max: 300, step: 1 })
        .width('80%')
        .blockColor(Color.Blue)
        .selectedColor('#007DFF')
        .onChange((v: number) => { this.split = v })

      Row({ space: 8 }) {
        Button('上一组')
          .fontSize(11)
          .onClick(() => {
            this.pairIndex = (this.pairIndex - 1 + this.PAIRS.length) % this.PAIRS.length
            this.split = 150
          })
        Button('下一组')
          .fontSize(11)
          .onClick(() => {
            this.pairIndex = (this.pairIndex + 1) % this.PAIRS.length
            this.split = 150
          })
        Button('居中')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .onClick(() => { this.split = 150 })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

/** 头像生成 */
@Component
struct ImageAvatarScene {
  @State shape: number = 0
  @State size: number = 96
  @State borderWidth: number = 3
  @State borderColor: number = 0
  @State showStatus: boolean = true
  @State statusColor: number = 0
  @State ringEnabled: boolean = false

  private readonly BORDER_COLORS: Array<ResourceColor> = [
    Color.Blue, Color.Red, Color.Green, Color.Orange, Color.Gray
  ]
  private readonly STATUS_COLORS: Array<ResourceColor> = [
    Color.Green, Color.Orange, Color.Gray
  ]
  private readonly SHAPE_NAMES: Array<string> = ['圆形', '圆角方', '方形']

  build() {
    Column({ space: 12 }) {
      Text('Image 头像生成测试')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)

      Stack() {
        Image($r('app.media.startIcon'))
          .width(this.size)
          .height(this.size)
          .objectFit(ImageFit.Cover)
          .borderRadius(this.shape === 0 ? this.size / 2 : (this.shape === 1 ? 16 : 0))
          .border({
            width: this.borderWidth,
            color: this.BORDER_COLORS[this.borderColor]
          })

        if (this.ringEnabled) {
          Column()
            .width(this.size + 12)
            .height(this.size + 12)
            .borderRadius(this.shape === 0 ? (this.size + 12) / 2 : (this.shape === 1 ? 22 : 6))
            .border({ width: 3, color: this.BORDER_COLORS[this.borderColor] })
            .opacity(0.5)
        }

        if (this.showStatus) {
          Column()
            .width(this.size / 4)
            .height(this.size / 4)
            .borderRadius((this.size / 4) / 2)
            .backgroundColor(this.STATUS_COLORS[this.statusColor])
            .border({ width: 2, color: Color.White })
            .position({ x: this.size - this.size / 4, y: this.size - this.size / 4 })
        }
      }
      .width(this.size + 20)
      .height(this.size + 20)
      .margin({ top: 8, bottom: 8 })

      Text(`形状:${this.SHAPE_NAMES[this.shape]}  尺寸:${this.size}  边框:${this.borderWidth}`)
        .fontSize(12)
        .fontColor(Color.Gray)

      Row({ space: 8 }) {
        ForEach(this.SHAPE_NAMES, (name: string, idx: number) => {
          Button(name)
            .fontSize(11)
            .backgroundColor(idx === this.shape ? '#007DFF' : '#E0E0E0')
            .onClick(() => { this.shape = idx })
        })
      }

      Row({ space: 8 }) {
        Button('尺寸+16')
          .fontSize(11)
          .onClick(() => { this.size = Math.min(160, this.size + 16) })
        Button('尺寸-16')
          .fontSize(11)
          .onClick(() => { this.size = Math.max(48, this.size - 16) })
        Button('边框+1')
          .fontSize(11)
          .onClick(() => { this.borderWidth = Math.min(8, this.borderWidth + 1) })
        Button('边框-1')
          .fontSize(11)
          .onClick(() => { this.borderWidth = Math.max(0, this.borderWidth - 1) })
      }

      Row({ space: 8 }) {
        Button('换边框色')
          .fontSize(11)
          .onClick(() => { this.borderColor = (this.borderColor + 1) % this.BORDER_COLORS.length })
        Button('换状态色')
          .fontSize(11)
          .onClick(() => { this.statusColor = (this.statusColor + 1) % this.STATUS_COLORS.length })
        Button(this.showStatus ? '状态点:开' : '状态点:关')
          .fontSize(11)
          .onClick(() => { this.showStatus = !this.showStatus })
        Button(this.ringEnabled ? '外环:开' : '外环:关')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .onClick(() => { this.ringEnabled = !this.ringEnabled })
      }
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}

@Entry
@Component
struct ImageTestcaseAdvancedUsage {
  @State currentIndex: number = 0

  build() {
    Column() {
      Tabs({ barPosition: BarPosition.Start, index: this.currentIndex }) {
        TabContent() { ImageAsBackgroundScene() }
          .tabBar('背景应用')
        TabContent() { ImageParallelLoadScene() }
          .tabBar('并发计时')
        TabContent() { ImageWatermarkScene() }
          .tabBar('水印叠加')
        TabContent() { ImagePinchViewerScene() }
          .tabBar('缩放查看')
        TabContent() { ImageThumbnailGalleryScene() }
          .tabBar('缩略画廊')
        TabContent() { ImageSliderCompareScene() }
          .tabBar('对比滑块')
        TabContent() { ImageAvatarScene() }
          .tabBar('头像生成')
      }
      .barMode(BarMode.Scrollable)
      .scrollable(true)
      .onChange((index: number) => { this.currentIndex = index })
    }
    .width('100%')
    .height('100%')
  }
}
