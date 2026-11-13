/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 * ========================================
 * Image 九宫格切片拉伸示例
 * ========================================
 * 本示例演示 Image 组件的 resizable 属性（九宫格切片拉伸）功能
 *
 * 涉及的Image API:
 * 1. resizable 属性 - 九宫格切片拉伸
 * 2. ImageFit 模式对比
 * 3. 不同尺寸下的拉伸效果对比
 *
 * resizable 参数说明:
 * - slice: 定义四边不拉伸区域 { top, right, bottom, left }
 * - left/right/top/bottom: 是否保持原始像素不拉伸
 *
 * ========================================
 * 如何测试
 * ========================================
 * 1. 进入页面后观察默认九宫格拉伸效果
 * 2. 调整 slice 参数观察不同切割区域的效果
 * 3. 对比 resizable 与 ImageFit 的差异
 * 4. 切换不同尺寸观察拉伸行为变化
 *
 * 预期结果:
 * - 九宫格拉伸保持边角不变形
 * - 中间区域被拉伸填充
 * - 适合用于聊天气泡、按钮背景等场景
 * ========================================
 */

@Entry
@Component
struct ImageExample121 {
  @State sliceTop: number = 30;
  @State sliceRight: number = 30;
  @State sliceBottom: number = 30;
  @State sliceLeft: number = 30;
  @State imageWidth: number = 300;
  @State imageHeight: number = 200;
  @State showOriginal: boolean = false;
  @State selectedPreset: string = '默认';
  @State enableResizable: boolean = true;

  private presets: Record<string, ResizableOptions> = {
    '默认': { slice: { top: 30, right: 30, bottom: 30, left: 30 }, resizable: true },
    '大圆角': { slice: { top: 50, right: 50, bottom: 50, left: 50 }, resizable: true },
    '仅水平拉伸': { slice: { top: 60, right: 30, bottom: 60, left: 30 }, resizable: true },
    '仅垂直拉伸': { slice: { top: 30, right: 60, bottom: 30, left: 60 }, resizable: true },
    '小边距': { slice: { top: 10, right: 10, bottom: 10, left: 10 }, resizable: true },
    '大边距': { slice: { top: 80, right: 80, bottom: 80, left: 80 }, resizable: true },
  };

  build() {
    Scroll() {
      Column({ space: 20 }) {
        Text('九宫格切片拉伸示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .fontColor(Color.Black)
          .margin({ top: 20, bottom: 10 })

        Column({ space: 15 }) {
          Text('效果预览 - ' + this.selectedPreset)
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Stack() {
            if (this.showOriginal) {
              Image($r('app.media.startIcon'))
                .width(this.imageWidth)
                .height(this.imageHeight)
                .objectFit(ImageFit.Fill)
                .borderRadius(5)
            } else if (this.enableResizable) {
              Image($r('app.media.startIcon'))
                .width(this.imageWidth)
                .height(this.imageHeight)
                .objectFit(ImageFit.Fill)
                .resizable({
                  slice: {
                    top: this.sliceTop,
                    right: this.sliceRight,
                    bottom: this.sliceBottom,
                    left: this.sliceLeft
                  }
                })
            } else {
              Image($r('app.media.startIcon'))
                .width(this.imageWidth)
                .height(this.imageHeight)
                .objectFit(ImageFit.Contain)
            }
          }
          .width(this.imageWidth + 20)
          .height(this.imageHeight + 20)
          .backgroundColor('#F0F0F0')
          .borderRadius(10)
          .shadow({ radius: 15, color: '#30000000' })

          Text('尺寸: ' + this.imageWidth + ' x ' + this.imageHeight)
            .fontSize(14)
            .fontColor(Color.Gray)

          Row({ space: 10 }) {
            Button(this.showOriginal ? '原始模式' : (this.enableResizable ? '拉伸模式' : 'Contain模式'))
              .fontSize(12)
              .backgroundColor(Color.Blue)
              .onClick(() => {
                if (this.showOriginal) {
                  this.showOriginal = false;
                  this.enableResizable = false;
                } else if (!this.enableResizable) {
                  this.enableResizable = true;
                } else {
                  this.showOriginal = true;
                }
              })

            Button('重置参数')
              .fontSize(12)
              .backgroundColor(Color.Gray)
              .onClick(() => {
                this.sliceTop = 30;
                this.sliceRight = 30;
                this.sliceBottom = 30;
                this.sliceLeft = 30;
                this.imageWidth = 300;
                this.imageHeight = 200;
                this.selectedPreset = '默认';
              })
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F8F8FF')
        .borderRadius(15)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 12 }) {
          Text('Slice 参数调节')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          this.SliceSlider('Top: ' + this.sliceTop, (val: number) => { this.sliceTop = val; })
          this.SliceSlider('Right: ' + this.sliceRight, (val: number) => { this.sliceRight = val; })
          this.SliceSlider('Bottom: ' + this.sliceBottom, (val: number) => { this.sliceBottom = val; })
          this.SliceSlider('Left: ' + this.sliceLeft, (val: number) => { this.sliceLeft = val; })
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 12 }) {
          Text('尺寸调节')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Text('宽度: ' + this.imageWidth)
            .fontSize(14)
            .fontColor(Color.Gray)
          Slider({ value: this.imageWidth, min: 100, max: 350, step: 10 })
            .width('100%')
            .onChange((value: number) => { this.imageWidth = value; })

          Text('高度: ' + this.imageHeight)
            .fontSize(14)
            .fontColor(Color.Gray)
          Slider({ value: this.imageHeight, min: 80, max: 400, step: 10 })
            .width('100%')
            .onChange((value: number) => { this.imageHeight = value; })
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#FFF8DC')
        .borderRadius(10)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 15 }) {
          Text('预设方案')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Start }) {
            ForEach(['默认', '大圆角', '仅水平拉伸', '仅垂直拉伸', '小边距', '大边距'], (name: string) => {
              Button(name)
                .fontSize(12)
                .margin({ right: 8, bottom: 8 })
                .backgroundColor(this.selectedPreset === name ? Color.Green : Color.LightGray)
                .onClick(() => {
                  this.selectedPreset = name;
                  const preset = this.presets[name];
                  this.sliceTop = preset.slice.top;
                  this.sliceRight = preset.slice.right;
                  this.sliceBottom = preset.slice.bottom;
                  this.sliceLeft = preset.slice.left;
                  this.enableResizable = true;
                  this.showOriginal = false;
                })
            })
          }
          .width('100%')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#E6E6FA')
        .borderRadius(10)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 10 }) {
          Text('ImageFit 对比')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Row({ space: 10 }) {
            Column({ space: 5 }) {
              Text('Contain')
                .fontSize(12)
                .fontColor(Color.Gray)
              Image($r('app.media.startIcon'))
                .width(140)
                .height(120)
                .objectFit(ImageFit.Contain)
                .backgroundColor('#EEEEEE')
                .borderRadius(8)
            }

            Column({ space: 5 }) {
              Text('Cover')
                .fontSize(12)
                .fontColor(Color.Gray)
              Image($r('app.media.startIcon'))
                .width(140)
                .height(120)
                .objectFit(ImageFit.Cover)
                .backgroundColor('#EEEEEE')
                .borderRadius(8)
            }
          }
          .width('100%')
          .justifyContent(FlexAlign.Center)

          Row({ space: 10 }) {
            Column({ space: 5 }) {
              Text('Fill')
                .fontSize(12)
                .fontColor(Color.Gray)
              Image($r('app.media.startIcon'))
                .width(140)
                .height(120)
                .objectFit(ImageFit.Fill)
                .backgroundColor('#EEEEEE')
                .borderRadius(8)
            }

            Column({ space: 5 }) {
              Text('ScaleDown')
                .fontSize(12)
                .fontColor(Color.Gray)
              Image($r('app.media.startIcon'))
                .width(140)
                .height(120)
                .objectFit(ImageFit.ScaleDown)
                .backgroundColor('#EEEEEE')
                .borderRadius(8)
            }
          }
          .width('100%')
          .justifyContent(FlexAlign.Center)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F0FFF0')
        .borderRadius(10)

        Column({ space: 10 }) {
          Text('应用场景: 聊天气泡')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Column() {
            Text('这是一条聊天消息，使用九宫格拉伸保持圆角不变形。')
              .fontSize(14)
              .fontColor(Color.White)
              .padding({ left: 20, right: 20, top: 12, bottom: 12 })
          }
          .backgroundImage($r('app.media.startIcon'))
          .backgroundImageResizable({
            slice: { top: 30, right: 30, bottom: 30, left: 30 }
          })
          .margin({ top: 10 })

          Column() {
            Text('这是另一条较长的消息内容，展示九宫格拉伸在不同长度文本下的效果表现。')
              .fontSize(14)
              .fontColor(Color.White)
              .padding({ left: 20, right: 20, top: 12, bottom: 12 })
          }
          .backgroundImage($r('app.media.startIcon'))
          .backgroundImageResizable({
            slice: { top: 30, right: 30, bottom: 30, left: 30 }
          })
          .margin({ top: 10 })
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#FFE4E1')
        .borderRadius(10)

        Text('九宫格拉伸适合用于需要保持边角不变形的背景图场景')
          .fontSize(12)
          .fontColor(Color.Blue)
          .margin({ top: 10, bottom: 20 })
      }
      .width('100%')
      .padding({ left: 20, right: 20 })
    }
    .width('100%')
    .height('100%')
    .backgroundColor(Color.White)
  }

  @Builder
  SliceSlider(label: string, onChange: (val: number) => void) {
    Column({ space: 4 }) {
      Text(label)
        .fontSize(14)
        .fontColor(Color.Gray)
      Slider({ value: this.sliceTop, min: 0, max: 100, step: 5 })
        .width('100%')
        .onChange((value: number) => { onChange(value); })
    }
    .width('100%')
  }
}
