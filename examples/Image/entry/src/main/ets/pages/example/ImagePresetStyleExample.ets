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
 * Image 预设样式示例
 * 展示常用的图片预设样式
 */
@Entry
@Component
struct ImagePresetStyleExample {
  @State selectedPreset: number = -1

  private presets: { name: string, width: number, height: number, radius: number,
    borderWidth: number, borderColor: string, shadowRadius: number }[] = [
    { name: '头像', width: 100, height: 100, radius: 50, borderWidth: 3, borderColor: '#007DFF', shadowRadius: 8 },
    { name: '卡片', width: 200, height: 140, radius: 12, borderWidth: 0, borderColor: '#000000', shadowRadius: 12 },
    { name: '徽章', width: 80, height: 80, radius: 40, borderWidth: 4, borderColor: '#FFD700', shadowRadius: 6 },
    { name: '海报', width: 160, height: 220, radius: 8, borderWidth: 0, borderColor: '#000000', shadowRadius: 16 },
    { name: '缩略图', width: 60, height: 60, radius: 8, borderWidth: 1, borderColor: '#CCCCCC', shadowRadius: 4 }
  ]

  build() {
    Scroll() {
      Column({ space: 16 }) {
        Text('Image 预设样式示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20 })

        this.buildPresetSection()
        this.buildPreviewSection()
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
  buildPresetSection() {
    Column({ space: 12 }) {
      Text('预设样式')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.presets, (item: { name: string }, index: number) => {
          Text(item.name)
            .fontSize(13)
            .padding({ left: 16, right: 16, top: 10, bottom: 10 })
            .margin(4)
            .borderRadius(20)
            .backgroundColor(this.selectedPreset === index ? 0x007DFF : 0xE8E8E8)
            .fontColor(this.selectedPreset === index ? Color.White : 0x333333)
            .onClick(() => {
              this.selectedPreset = index
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
  buildPreviewSection() {
    Column({ space: 12 }) {
      Text('预览')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      if (this.selectedPreset >= 0) {
        Image($r('app.media.startIcon'))
          .width(this.presets[this.selectedPreset].width)
          .height(this.presets[this.selectedPreset].height)
          .objectFit(ImageFit.Cover)
          .borderRadius(this.presets[this.selectedPreset].radius)
          .border({
            width: this.presets[this.selectedPreset].borderWidth,
            color: this.presets[this.selectedPreset].borderColor
          })
          .shadow({
            radius: this.presets[this.selectedPreset].shadowRadius,
            color: '#40000000',
            offsetX: 0,
            offsetY: this.presets[this.selectedPreset].shadowRadius / 2
          })

        Text(`样式: ${this.presets[this.selectedPreset].name}`)
          .fontSize(14)
          .fontColor(0x666666)
      } else {
        Text('请选择一个预设样式')
          .fontSize(14)
          .fontColor(0x999999)
      }
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
    .alignItems(HorizontalAlign.Center)
  }

  @Builder
  buildComparisonSection() {
    Column({ space: 12 }) {
      Text('所有预设对比')
        .fontSize(17)
        .fontWeight(FontWeight.Bold)

      Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Center }) {
        ForEach(this.presets, (item: { name: string, width: number, height: number, radius: number,
          borderWidth: number, borderColor: string, shadowRadius: number }) => {
          Column({ space: 4 }) {
            Image($r('app.media.heart'))
              .width(item.width > 100 ? 80 : item.width)
              .height(item.height > 100 ? 80 : item.height)
              .objectFit(ImageFit.Cover)
              .borderRadius(item.radius > 40 ? 40 : item.radius)
              .border({ width: item.borderWidth > 2 ? 2 : item.borderWidth, color: item.borderColor })
              .shadow({ radius: item.shadowRadius > 8 ? 8 : item.shadowRadius, color: '#40000000', offsetX: 0, offsetY: 2 })
            Text(item.name)
              .fontSize(10)
              .fontColor(0x666666)
          }
          .margin(8)
        })
      }
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

      Text('1. 头像: 圆形 + 边框 + 阴影')
        .fontSize(12)
        .fontColor(0x666666)
      Text('2. 卡片: 圆角 + 阴影')
        .fontSize(12)
        .fontColor(0x666666)
      Text('3. 徽章: 圆形 + 金色边框')
        .fontSize(12)
        .fontColor(0x666666)
      Text('4. 海报: 小圆角 + 大阴影')
        .fontSize(12)
        .fontColor(0x666666)
      Text('5. 缩略图: 小圆角 + 细边框')
        .fontSize(12)
        .fontColor(0x666666)
    }
    .width('100%')
    .padding(16)
    .backgroundColor(Color.White)
    .borderRadius(12)
  }
}
