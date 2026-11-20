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


/** 整合自: ImageTestcase067.ets, ImageTestcase068.ets */

@Entry
@Component
struct ImageContentTransitionTestCase067 {
  @State currentSourceIndex: number = 0
  @State transitionKey: number = 0
  @State transitionLog: string = '点击下方按钮切换图源触发转场'
  @State transitionCount: number = 0
  @State isAnimating: boolean = false

  private readonly SOURCES: Array<{ src: Resource; label: string }> = [
    { src: $r('app.media.startIcon'), label: 'startIcon' },
    { src: $r('app.media.background'), label: 'background' },
    { src: $r('app.media.foreground'), label: 'foreground' },
    { src: $r('app.media.heart'), label: 'heart(SVG)' },
    { src: $r('app.media.star'), label: 'star(SVG)' },
    { src: $r('app.media.icon'), label: 'icon(SVG)' }
  ]

  build() {
    Column() {
      Text('Image contentTransition内容转场测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Stack() {
        Image(this.SOURCES[this.currentSourceIndex].src)
          .width(180)
          .height(180)
          .objectFit(ImageFit.Cover)
          .borderRadius(12)
          .border({ width: 2, color: Color.Blue })
          .transition(this.getTransitionEffect())
          .onComplete(() => {
            this.isAnimating = false
            this.transitionLog = `${this.SOURCES[this.currentSourceIndex].label} 加载完成`
          })
          .key(`img_${this.transitionKey}`)
      }
      .width(200)
      .height(200)
      .alignContent(Alignment.Center)
      .margin({ bottom: 16 })

      Text(`当前图源: ${this.SOURCES[this.currentSourceIndex].label}`)
        .fontSize(14)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })

      Text(this.transitionLog)
        .fontSize(12)
        .fontColor(this.isAnimating ? Color.Orange : Color.Green)
        .margin({ bottom: 8 })

      Text(`转场触发次数: ${this.transitionCount}`)
        .fontSize(12)
        .fontColor(Color.Blue)
        .margin({ bottom: 16 })

      Divider()

      Text('图源切换(触发转场动画)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row() {
        ForEach(this.SOURCES, (config: { src: Resource; label: string }, index: number) => {
          Button(config.label)
            .fontSize(9)
            .onClick(() => {
              this.switchSource(index)
            })
            .backgroundColor(index === this.currentSourceIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentSourceIndex ? Color.White : Color.Gray)
            .margin({ left: 3, right: 3 })
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('自动轮播(每1.5秒切换)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row({ space: 12 }) {
        Button('开始轮播')
          .fontSize(11)
          .backgroundColor('#4CAF50')
          .fontColor(Color.White)
          .onClick(() => { this.startAutoPlay() })
        Button('停止轮播')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .fontColor(Color.White)
          .onClick(() => { this.stopAutoPlay() })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('转场效果说明')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Text('当前使用: opacity淡入淡出 + scale缩放组合转场\n' +
        '组件卸载时: opacity(0) + scale(0.8)\n' +
        '组件挂载时: opacity(1) + scale(1.0)\n' +
        '动画时长: 500ms, 曲线: FastOutSlowIn')
        .fontSize(11)
        .fontColor(Color.Gray)
        .lineHeight(18)
        .margin({ bottom: 12 })

      Text('提示: 切换图源时观察淡入淡出和缩放过渡')
        .fontSize(10)
        .fontColor(Color.Orange)
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }

  private getTransitionEffect(): TransitionEffect {
    return TransitionEffect.OPACITY.animation({ duration: 500, curve: Curve.FastOutSlowIn })
      .combine(TransitionEffect.scale({ x: 0.8, y: 0.8 }).animation({ duration: 500, curve: Curve.FastOutSlowIn }))
  }

  private switchSource(index: number): void {
    if (index === this.currentSourceIndex || this.isAnimating) {
      return
    }
    this.isAnimating = true
    this.transitionCount++
    this.transitionKey++
    this.currentSourceIndex = index
    this.transitionLog = `正在切换到 ${this.SOURCES[index].label}...`
  }

  private autoPlayId: number = -1

  private startAutoPlay(): void {
    if (this.autoPlayId !== -1) {
      return
    }
    this.autoPlayId = setInterval(() => {
      const next = (this.currentSourceIndex + 1) % this.SOURCES.length
      this.switchSource(next)
    }, 1500)
    this.transitionLog = '自动轮播已启动'
  }

  private stopAutoPlay(): void {
    if (this.autoPlayId !== -1) {
      clearInterval(this.autoPlayId)
      this.autoPlayId = -1
      this.transitionLog = '自动轮播已停止'
    }
  }

  aboutToDisappear(): void {
    this.stopAutoPlay()
  }
}

@Component
struct ImageBindMenuPopupTestCase068 {
  @State popupVisible: boolean = false
  @State menuVisible: boolean = false
  @State actionLog: string = '长按图片弹出ContextMenu, 点击图片弹出Popup'
  @State actionCount: number = 0
  @State lastAction: string = '无'
  @State currentImageIndex: number = 0

  private readonly IMAGES: Array<{ src: Resource; label: string }> = [
    { src: $r('app.media.startIcon'), label: 'startIcon' },
    { src: $r('app.media.background'), label: 'background' },
    { src: $r('app.media.foreground'), label: 'foreground' },
    { src: $r('app.media.heart'), label: 'heart' }
  ]

  @Builder
  MenuBuilder() {
    Column() {
      Text('自定义菜单标题')
        .fontSize(14)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 8 })
      Button('保存到相册')
        .fontSize(12)
        .width('100%')
        .margin({ bottom: 4 })
        .onClick(() => {
          this.recordAction('保存到相册')
          this.menuVisible = false
        })
      Button('分享图片')
        .fontSize(12)
        .width('100%')
        .margin({ bottom: 4 })
        .onClick(() => {
          this.recordAction('分享图片')
          this.menuVisible = false
        })
      Button('复制图片')
        .fontSize(12)
        .width('100%')
        .margin({ bottom: 4 })
        .onClick(() => {
          this.recordAction('复制图片')
          this.menuVisible = false
        })
      Button('设为壁纸')
        .fontSize(12)
        .width('100%')
        .margin({ bottom: 4 })
        .onClick(() => {
          this.recordAction('设为壁纸')
          this.menuVisible = false
        })
      Button('图片信息')
        .fontSize(12)
        .width('100%')
        .onClick(() => {
          this.recordAction('图片信息')
          this.menuVisible = false
        })
    }
    .width(180)
    .padding(12)
  }

  @Builder
  PopupBuilder() {
    Column() {
      Text('图片详情')
        .fontSize(14)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 8 })
      Text(`图源: ${this.IMAGES[this.currentImageIndex].label}`)
        .fontSize(12)
        .margin({ bottom: 4 })
      Text(`操作次数: ${this.actionCount}`)
        .fontSize(12)
        .margin({ bottom: 4 })
      Text(`最近操作: ${this.lastAction}`)
        .fontSize(12)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })
      Button('关闭')
        .fontSize(11)
        .onClick(() => {
          this.popupVisible = false
        })
    }
    .width(160)
    .padding(12)
    .backgroundColor(Color.White)
    .borderRadius(8)
  }

  build() {
    Column() {
      Text('Image bindContextMenu与bindPopup测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Image(this.IMAGES[this.currentImageIndex].src)
        .width(180)
        .height(180)
        .objectFit(ImageFit.Cover)
        .border({ width: 2, color: Color.Blue })
        .borderRadius(12)
        .bindContextMenu({
          builder: () => {
            this.MenuBuilder()
          },
          responseType: ContextMenuMediaType.LongPress
        }, ContextMenuType.menu)
        .bindPopup({
          builder: {
            builder: () => {
              this.PopupBuilder()
            }
          },
          placement: Placement.Bottom,
          popupColor: '#FAFAFA',
          enableArrow: true,
          onStateChange: (e: { isVisible: boolean }) => {
            if (e.isVisible) {
              this.recordAction('Popup显示')
            }
          }
        })
        .margin({ bottom: 16 })

      Text(this.actionLog)
        .fontSize(12)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })

      Text(`操作次数: ${this.actionCount}  最近: ${this.lastAction}`)
        .fontSize(12)
        .fontColor(Color.Blue)
        .margin({ bottom: 16 })

      Divider()

      Text('操作说明')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Text('1. 长按图片 → 弹出ContextMenu菜单(保存/分享/复制/壁纸/信息)\n' +
        '2. 点击图片 → 弹出Popup气泡(图片详情)\n' +
        '3. ContextMenu和Popup可同时触发')
        .fontSize(11)
        .fontColor(Color.Gray)
        .lineHeight(18)
        .margin({ bottom: 16 })

      Divider()

      Text('切换图源')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row() {
        ForEach(this.IMAGES, (config: { src: Resource; label: string }, index: number) => {
          Button(config.label)
            .fontSize(9)
            .onClick(() => {
              this.currentImageIndex = index
              this.actionLog = `已切换到 ${config.label}`
            })
            .backgroundColor(index === this.currentImageIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentImageIndex ? Color.White : Color.Gray)
            .margin({ left: 3, right: 3 })
        })
      }
      .margin({ bottom: 16 })

      Divider()

      Text('bindMenu测试(点击弹出)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Image($r('app.media.startIcon'))
        .width(80)
        .height(80)
        .objectFit(ImageFit.Cover)
        .borderRadius(8)
        .bindMenu([
          {
            value: '菜单项1',
            action: () => {
              this.recordAction('bindMenu项1')
            }
          },
          {
            value: '菜单项2',
            action: () => {
              this.recordAction('bindMenu项2')
            }
          },
          {
            value: '菜单项3',
            action: () => {
              this.recordAction('bindMenu项3')
            }
          }
        ])

      Text('点击上方小图触发bindMenu')
        .fontSize(10)
        .fontColor(Color.Gray)
        .margin({ top: 4 })

      Divider()

      Row({ space: 12 }) {
        Button('重置')
          .fontSize(11)
          .backgroundColor('#FF6666')
          .fontColor(Color.White)
          .onClick(() => {
            this.actionCount = 0
            this.lastAction = '无'
            this.actionLog = '已重置'
            this.currentImageIndex = 0
          })
      }
      .margin({ top: 16 })
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }

  private recordAction(action: string): void {
    this.actionCount++
    this.lastAction = action
    this.actionLog = `执行操作: ${action}`
  }
}
