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
 * Image 裁剪示例
 * 展示 clip 和 clipShape 属性的裁剪效果
 */
@Entry
@Component
struct ImageClipExample {
  @State clipEnabled: boolean = false
  @State clipShapeIndex: number = 0

  private clipShapes: string[] = ['无裁剪', '圆形', '椭圆', '圆角矩形']

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 裁剪示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildPreviewSection()
        this.buildClipShapeSection()
        this.buildComparisonSection()
        this.buildTipsSection()
      }
      .width('100%')
      .padding({ left: 16, right: 16, bottom: 30 })
    }
    .width('100%')
    .height('100%')
    .backgroundColor(0xF5F5F5)
  }

  @Builder
  buildPreviewSection() {
    Column({ space: 12 }) {
      Text('预览')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Image($r('app.media.startIcon'))
        .width(150)
        .height(150)
        .objectFit(ImageFit.Cover)
        .borderRadius(12)
        .border({ width: 1, color: 0xCCCCCC })
        .clip(this.clipEnabled)
        .clipShape(this.clipShapeIndex === 1 ? ClipShape.Circle :
          this.clipShapeIndex === 2 ? ClipShape.Ellipse :
          this.clipShapeIndex === 3 ? ClipShape.RoundRect : ClipShape.Rect)

      Text(`裁剪: ${this.clipEnabled ? '启用' : '禁用'}`)
        .fontSize(14)
        .fontColor(0x666666)
      Text(`形状: ${this.clipShapes[this.clipShapeIndex]}`)
        .fontSize(14)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildClipShapeSection() {
    Column({ space: 12 }) {
      Row({ space: 16 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.clipEnabled })
          .onChange((isOn: boolean) => {
            this.clipEnabled = isOn
          })
        Text('启用裁剪')
          .fontSize(16)
          .fontWeight(FontWeight.Medium)
      }

      Text('裁剪形状')
        .fontSize(16)
        .fontWeight(FontWeight.Medium)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.clipShapes, (name: string, index: number) => {
          Text(name)
            .fontSize(12)
            .padding({ left: 14, right: 14, top: 8, bottom: 8 })
            .margin(4)
            .borderRadius(18)
            .backgroundColor(this.clipShapeIndex === index ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.clipShapeIndex === index ? Color.White : 0x333333)
            .onClick(() => {
              this.clipShapeIndex = index
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
  buildComparisonSection() {
    Column({ space: 12 }) {
      Text('裁剪对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Row({ space: 12 }) {
        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(70)
            .height(70)
            .objectFit(ImageFit.Cover)
          Text('无裁剪')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(70)
            .height(70)
            .objectFit(ImageFit.Cover)
            .clip(true)
            .clipShape(ClipShape.Circle)
          Text('圆形')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(70)
            .height(70)
            .objectFit(ImageFit.Cover)
            .clip(true)
            .clipShape(ClipShape.Ellipse)
          Text('椭圆')
            .fontSize(10)
            .fontColor(0x666666)
        }

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(70)
            .height(70)
            .objectFit(ImageFit.Cover)
            .clip(true)
            .clipShape(ClipShape.RoundRect)
          Text('圆角矩形')
            .fontSize(10)
            .fontColor(0x666666)
        }
      }
      .justifyContent(FlexAlign.Center)
      .width('100%')
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }

  @Builder
  buildTipsSection() {
    Column({ space: 8 }) {
      Text('使用提示')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Text('1. clip 属性启用/禁用裁剪')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. clipShape 设置裁剪形状')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. Circle: 圆形裁剪')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. Ellipse: 椭圆裁剪')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. RoundRect: 圆角矩形裁剪')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
