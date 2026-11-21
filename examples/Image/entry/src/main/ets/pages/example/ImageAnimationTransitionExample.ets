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
 * Image 动画与过渡示例
 * 展示图片加载过渡、显隐动画、切换动画、路径动画等效果
 */
@Entry
@Component
struct ImageAnimationTransitionExample {
  @State visible1: boolean = true
  @State visible2: boolean = true
  @State visible3: boolean = true
  @State currentGalleryIndex: number = 0
  @State scaleAnim: number = 1.0
  @State rotateAnim: number = 0
  @State opacityAnim: number = 1.0
  @State offsetX: number = 0
  @State offsetY: number = 0
  @State widthAnim: number = 200
  @State heightAnim: number = 200
  @State borderRadiusAnim: number = 0
  @State isPlaying: boolean = false
  @State transitionType: number = 0
  @State fadeInOpacity: number = 0
  @State slideOffset: number = -200
  @State zoomScale: number = 0.5
  @State flipAngle: number = 0
  @State breatheScale: number = 1.0
  @State shakeOffset: number = 0
  @State bounceScale: number = 1.0
  @State pathProgress: number = 0
  @State loadProgress: number = 0
  @State isLoading: boolean = false
  @State showPlaceholder: boolean = true
  @State imageLoaded: boolean = false
  @State staggerItems: boolean[] = [false, false, false, false, false, false]
  @State pageSwitchIndex: number = 0

  private galleryImages: Resource[] = [
    $r('app.media.startIcon'),
    $r('app.media.background'),
    $r('app.media.foreground'),
    $r('app.media.heart'),
    $r('app.media.star'),
    $r('app.media.icon')
  ]

  private galleryNames: string[] = [
    'startIcon', 'background', 'foreground', 'heart', 'star', 'icon'
  ]

  private transitionTypes: string[] = ['淡入', '滑入', '缩放', '翻转']

  breatheTimer: number = -1
  shakeTimer: number = -1

  aboutToAppear() {
    this.startBreatheAnimation()
  }

  aboutToDisappear() {
    if (this.breatheTimer !== -1) {
      clearInterval(this.breatheTimer)
    }
    if (this.shakeTimer !== -1) {
      clearInterval(this.shakeTimer)
    }
  }

  startBreatheAnimation() {
    let increasing = true
    this.breatheTimer = setInterval(() => {
      if (increasing) {
        this.breatheScale += 0.02
        if (this.breatheScale >= 1.1) {
          increasing = false
        }
      } else {
        this.breatheScale -= 0.02
        if (this.breatheScale <= 1.0) {
          increasing = true
        }
      }
    }, 30)
  }

  startShakeAnimation() {
    let step = 0
    this.shakeTimer = setInterval(() => {
      step++
      this.shakeOffset = Math.sin(step * 0.5) * 5
      if (step >= 20) {
        clearInterval(this.shakeTimer)
        this.shakeTimer = -1
        this.shakeOffset = 0
      }
    }, 30)
  }

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 动画与过渡示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildLoadTransitionSection()
        this.buildGalleryTransitionSection()
        this.buildBasicAnimSection()
        this.buildPresetAnimSection()
        this.buildStaggerSection()
        this.buildPageSwitchSection()
        this.buildProgressSection()
      }
      .width('100%')
      .padding({ left: 16, right: 16, bottom: 30 })
    }
    .width('100%')
    .height('100%')
    .backgroundColor(0xF5F5F5)
  }

  @Builder
  buildLoadTransitionSection() {
    Column({ space: 12 }) {
      Text('加载过渡')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Stack() {
        if (this.showPlaceholder) {
          Column()
            .width(200)
            .height(200)
            .borderRadius(12)
            .backgroundColor(0xE0E0E0)
            .opacity(1 - this.fadeInOpacity)
        }

        Image($r('app.media.startIcon'))
          .width(200)
          .height(200)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)
          .opacity(this.fadeInOpacity)
          .scale({ x: this.zoomScale, y: this.zoomScale })
      }
      .width(200)
      .height(200)
      .alignContent(Alignment.Center)

      Row({ space: 10 }) {
        Button('模拟加载')
          .fontSize(13)
          .onClick(() => {
            this.showPlaceholder = true
            this.fadeInOpacity = 0
            this.zoomScale = 0.5
            this.isLoading = true
            this.loadProgress = 0

            let progress = 0
            const timer = setInterval(() => {
              progress += 10
              this.loadProgress = progress
              if (progress >= 100) {
                clearInterval(timer)
                this.isLoading = false
                animateTo({ duration: 500, curve: Curve.EaseOut }, () => {
                  this.fadeInOpacity = 1
                  this.zoomScale = 1.0
                  this.showPlaceholder = false
                  this.imageLoaded = true
                })
              }
            }, 80)
          })

        Button('重置')
          .fontSize(13)
          .backgroundColor(0xFF9800)
          .onClick(() => {
            this.fadeInOpacity = 0
            this.zoomScale = 0.5
            this.showPlaceholder = true
            this.imageLoaded = false
            this.loadProgress = 0
          })
      }

      if (this.isLoading) {
        Progress({ value: this.loadProgress, total: 100, type: ProgressType.Linear })
          .width('80%')
          .color(0x007DFF)
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildGalleryTransitionSection() {
    Column({ space: 12 }) {
      Text('图片切换过渡')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text(`当前: ${this.galleryNames[this.currentGalleryIndex]}`)
        .fontSize(13)
        .fontColor(0x666666)

      Stack() {
        ForEach(this.galleryImages, (img: Resource, index: number) => {
          if (index === this.currentGalleryIndex) {
            Image(img)
              .width(220)
              .height(180)
              .objectFit(ImageFit.Cover)
              .borderRadius(12)
              .transition(TransitionEffect.OPACITY.combine(TransitionEffect.translate({ x: 100 }))
                .animation({ duration: 400, curve: Curve.EaseInOut }))
              .key('img_' + index)
          }
        })
      }
      .width(220)
      .height(180)
      .alignContent(Alignment.Center)

      Row({ space: 8 }) {
        Button('<')
          .width(50)
          .fontSize(16)
          .onClick(() => {
            this.currentGalleryIndex =
              (this.currentGalleryIndex - 1 + this.galleryImages.length) % this.galleryImages.length
          })

        ForEach(this.galleryImages, (_: Resource, index: number) => {
          Column()
            .width(this.currentGalleryIndex === index ? 20 : 8)
            .height(8)
            .borderRadius(4)
            .backgroundColor(this.currentGalleryIndex === index ? 0x007DFF : 0xCCCCCC)
            .onClick(() => {
              this.currentGalleryIndex = index
            })
        })

        Button('>')
          .width(50)
          .fontSize(16)
          .onClick(() => {
            this.currentGalleryIndex =
              (this.currentGalleryIndex + 1) % this.galleryImages.length
          })
      }
      .alignItems(VerticalAlign.Center)

      Text('切换过渡类型')
        .fontSize(14)
        .margin({ top: 8 })

      Row({ space: 8 }) {
        ForEach(this.transitionTypes, (name: string, index: number) => {
          Text(name)
            .fontSize(12)
            .padding({ left: 12, right: 12, top: 6, bottom: 6 })
            .borderRadius(16)
            .backgroundColor(this.transitionType === index ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.transitionType === index ? Color.White : 0x333333)
            .onClick(() => {
              this.transitionType = index
            })
        })
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildBasicAnimSection() {
    Column({ space: 12 }) {
      Text('基础动画')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 20 }) {
        Image($r('app.media.heart'))
          .width(80)
          .height(80)
          .scale({ x: this.scaleAnim, y: this.scaleAnim })

        Image($r('app.media.star'))
          .width(80)
          .height(80)
          .rotate({ angle: this.rotateAnim })

        Image($r('app.media.icon'))
          .width(80)
          .height(80)
          .opacity(this.opacityAnim)
      }
      .justifyContent(FlexAlign.Center)
      .width('100%')

      Row({ space: 10 }) {
        Button('缩放')
          .fontSize(12)
          .layoutWeight(1)
          .onClick(() => {
            animateTo({ duration: 400, curve: Curve.EaseInOut }, () => {
              this.scaleAnim = this.scaleAnim > 1.0 ? 1.0 : 1.5
            })
          })

        Button('旋转')
          .fontSize(12)
          .layoutWeight(1)
          .onClick(() => {
            animateTo({ duration: 600, curve: Curve.EaseInOut }, () => {
              this.rotateAnim += 360
            })
          })

        Button('透明度')
          .fontSize(12)
          .layoutWeight(1)
          .onClick(() => {
            animateTo({ duration: 400, curve: Curve.EaseInOut }, () => {
              this.opacityAnim = this.opacityAnim > 0.5 ? 0.2 : 1.0
            })
          })
      }

      Divider().margin({ top: 8, bottom: 8 })

      Text('尺寸与圆角动画')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)

      Image($r('app.media.startIcon'))
        .width(this.widthAnim)
        .height(this.heightAnim)
        .borderRadius(this.borderRadiusAnim)
        .objectFit(ImageFit.Cover)

      Row({ space: 10 }) {
        Button('变宽')
          .fontSize(12)
          .layoutWeight(1)
          .onClick(() => {
            animateTo({ duration: 400 }, () => {
              this.widthAnim = this.widthAnim === 200 ? 280 : 200
            })
          })

        Button('变高')
          .fontSize(12)
          .layoutWeight(1)
          .onClick(() => {
            animateTo({ duration: 400 }, () => {
              this.heightAnim = this.heightAnim === 200 ? 120 : 200
            })
          })

        Button('圆角')
          .fontSize(12)
          .layoutWeight(1)
          .onClick(() => {
            animateTo({ duration: 400 }, () => {
              this.borderRadiusAnim = this.borderRadiusAnim === 0 ? 60 : 0
            })
          })
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildPresetAnimSection() {
    Column({ space: 12 }) {
      Text('预设动画效果')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 20 }) {
        Column({ space: 4 }) {
          Text('呼吸')
            .fontSize(11)
            .fontColor(0x999999)
          Image($r('app.media.heart'))
            .width(60)
            .height(60)
            .scale({ x: this.breatheScale, y: this.breatheScale })
        }

        Column({ space: 4 }) {
          Text('抖动')
            .fontSize(11)
            .fontColor(0x999999)
          Image($r('app.media.star'))
            .width(60)
            .height(60)
            .offset({ x: this.shakeOffset, y: 0 })
        }

        Column({ space: 4 }) {
          Text('显隐')
            .fontSize(11)
            .fontColor(0x999999)
          Image($r('app.media.icon'))
            .width(60)
            .height(60)
            .opacity(this.visible1 ? 1 : 0)
        }
      }
      .justifyContent(FlexAlign.Center)
      .width('100%')

      Row({ space: 10 }) {
        Button('触发抖动')
          .fontSize(12)
          .layoutWeight(1)
          .onClick(() => {
            if (this.shakeTimer === -1) {
              this.startShakeAnimation()
            }
          })

        Button('切换显隐')
          .fontSize(12)
          .layoutWeight(1)
          .onClick(() => {
            animateTo({ duration: 300 }, () => {
              this.visible1 = !this.visible1
            })
          })
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildStaggerSection() {
    Column({ space: 12 }) {
      Text('交错动画')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.staggerItems, (visible: boolean, index: number) => {
          Image($r('app.media.startIcon'))
            .width(60)
            .height(60)
            .margin(6)
            .opacity(visible ? 1 : 0)
            .scale({ x: visible ? 1 : 0.3, y: visible ? 1 : 0.3 })
        })
      }

      Row({ space: 10 }) {
        Button('全部显示')
          .fontSize(12)
          .layoutWeight(1)
          .onClick(() => {
            for (let i = 0; i < this.staggerItems.length; i++) {
              const delay = i * 100
              setTimeout(() => {
                animateTo({ duration: 300, curve: Curve.EaseOut }, () => {
                  this.staggerItems[i] = true
                })
              }, delay)
            }
          })

        Button('全部隐藏')
          .fontSize(12)
          .layoutWeight(1)
          .onClick(() => {
            for (let i = this.staggerItems.length - 1; i >= 0; i--) {
              const delay = (this.staggerItems.length - 1 - i) * 100
              setTimeout(() => {
                animateTo({ duration: 300, curve: Curve.EaseIn }, () => {
                  this.staggerItems[i] = false
                })
              }, delay)
            }
          })
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildPageSwitchSection() {
    Column({ space: 12 }) {
      Text('翻页切换')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Stack() {
        ForEach([0, 1, 2], (pageIndex: number) => {
          if (pageIndex === this.pageSwitchIndex) {
            Column({ space: 8 }) {
              Image(this.galleryImages[pageIndex])
                .width(200)
                .height(140)
                .borderRadius(12)
                .objectFit(ImageFit.Cover)
              Text(`第 ${pageIndex + 1} 页`)
                .fontSize(14)
                .fontColor(Color.White)
            }
            .width('100%')
            .justifyContent(FlexAlign.Center)
            .transition(TransitionEffect.OPACITY.animation({ duration: 400 }))
            .key('page_' + pageIndex)
          }
        })
      }
      .width('100%')
      .height(200)
      .backgroundColor(0x333333)
      .borderRadius(12)
      .alignContent(Alignment.Center)

      Row({ space: 10 }) {
        Button('上一页')
          .fontSize(13)
          .layoutWeight(1)
          .onClick(() => {
            this.pageSwitchIndex =
              (this.pageSwitchIndex - 1 + 3) % 3
          })

        Button('下一页')
          .fontSize(13)
          .layoutWeight(1)
          .onClick(() => {
            this.pageSwitchIndex = (this.pageSwitchIndex + 1) % 3
          })
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildProgressSection() {
    Column({ space: 12 }) {
      Text('加载进度')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Stack() {
        Image($r('app.media.startIcon'))
          .width(180)
          .height(180)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)

        if (this.isLoading) {
          Column()
            .width(180)
            .height(180)
            .borderRadius(12)
            .backgroundColor('rgba(0,0,0,0.4)')

          Progress({ value: this.loadProgress, total: 100, type: ProgressType.Ring })
            .width(80)
            .height(80)
            .color(Color.White)
        }
      }
      .width(180)
      .height(180)
      .alignContent(Alignment.Center)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }
}
