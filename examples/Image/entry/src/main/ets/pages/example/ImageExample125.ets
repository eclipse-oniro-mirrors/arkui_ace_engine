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
 * Image 图片解码区域与渐进式加载示例
 * ========================================
 * 本示例演示 Image 组件的 sourceSize 和 interpolate 属性
 *
 * 涉及的Image API:
 * 1. sourceSize 属性 - 解码区域大小控制
 * 2. interpolate 属性 - 图片插值抗锯齿
 * 3. alt 属性 - 占位图
 * 4. 加载事件回调 (onComplete/onError)
 *
 * sourceSize 说明:
 * - 控制图片解码时的目标尺寸，不影响显示尺寸
 * - 较小的 sourceSize 降低内存但可能模糊
 * - 较大的 sourceSize 提高清晰度但增加内存
 *
 * ========================================
 * 如何测试
 * ========================================
 * 1. 观察不同 sourceSize 下的图片清晰度
 * 2. 切换 interpolate 模式对比抗锯齿效果
 * 3. 查看加载状态与回调信息
 * 4. 对比不同解码尺寸对显示质量的影响
 *
 * 预期结果:
 * - sourceSize 越小图片越模糊但内存占用越低
 * - interpolate 开启后边缘更平滑
 * - alt 图在加载过程中显示
 * ========================================
 */

@Entry
@Component
struct ImageExample122 {
  @State sourceWidth: number = 200;
  @State sourceHeight: number = 200;
  @State displayWidth: number = 200;
  @State displayHeight: number = 200;
  @State enableInterpolation: boolean = true;
  @State loadStatus: string = '等待加载';
  @State loadTime: string = '-';
  @State imageInfo: string = '';
  @State showAlt: boolean = false;
  @State selectedMode: string = '标准';
  @State startTime: number = 0;

  private modes: Record<string, { sw: number, sh: number, dw: number, dh: number }> = {
    '缩略图': { sw: 50, sh: 50, dw: 200, dh: 200 },
    '标准': { sw: 200, sh: 200, dw: 200, dh: 200 },
    '高清': { sw: 400, sh: 400, dw: 200, dh: 200 },
    '超高清': { sw: 800, sh: 800, dw: 200, dh: 200 },
    '宽屏': { sw: 400, sh: 200, dw: 300, dh: 150 },
    '纵向': { sw: 200, sh: 400, dw: 150, dh: 300 },
  };

  build() {
    Scroll() {
      Column({ space: 20 }) {
        Text('图片解码与渐进式加载示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .fontColor(Color.Black)
          .margin({ top: 20, bottom: 10 })

        Column({ space: 15 }) {
          Text('图片预览 - ' + this.selectedMode)
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Stack() {
            Image($r('app.media.startIcon'))
              .width(this.displayWidth)
              .height(this.displayHeight)
              .sourceSize({ width: this.sourceWidth, height: this.sourceHeight })
              .interpolate(this.enableInterpolation)
              .objectFit(ImageFit.Contain)
              .borderRadius(10)
              .alt(this.showAlt ? $r('app.media.icon') : undefined)
              .onComplete((event) => {
                const elapsed = Date.now() - this.startTime;
                this.loadTime = elapsed + 'ms';
                this.loadStatus = '加载成功';
                if (event && event.width !== undefined) {
                  this.imageInfo = '实际解码: ' + event.width + 'x' + event.height;
                }
              })
              .onError(() => {
                this.loadStatus = '加载失败';
                this.loadTime = '-';
              })
          }
          .width(this.displayWidth + 20)
          .height(this.displayHeight + 20)
          .backgroundColor('#F0F0F0')
          .borderRadius(15)
          .shadow({ radius: 15, color: '#30000000' })

          Column({ space: 5 }) {
            Text('状态: ' + this.loadStatus)
              .fontSize(14)
              .fontColor(this.loadStatus === '加载成功' ? Color.Green : (this.loadStatus === '加载失败' ? Color.Red : Color.Gray))
            Text('耗时: ' + this.loadTime)
              .fontSize(14)
              .fontColor(Color.Gray)
            Text(this.imageInfo)
              .fontSize(14)
              .fontColor(Color.Blue)
          }

          Row({ space: 10 }) {
            Button('重新加载')
              .fontSize(12)
              .backgroundColor(Color.Blue)
              .onClick(() => {
                this.loadStatus = '等待加载';
                this.startTime = Date.now();
                const temp = this.sourceWidth;
                this.sourceWidth = temp + 1;
                this.sourceWidth = temp;
              })

            Button('切换占位图')
              .fontSize(12)
              .backgroundColor(Color.Orange)
              .onClick(() => {
                this.showAlt = !this.showAlt;
              })
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F8F8FF')
        .borderRadius(15)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 12 }) {
          Text('解码尺寸 (sourceSize)')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Text('解码宽度: ' + this.sourceWidth + 'px')
            .fontSize(14)
            .fontColor(Color.Gray)
          Slider({ value: this.sourceWidth, min: 16, max: 800, step: 8 })
            .width('100%')
            .onChange((value: number) => {
              this.sourceWidth = value;
            })

          Text('解码高度: ' + this.sourceHeight + 'px')
            .fontSize(14)
            .fontColor(Color.Gray)
          Slider({ value: this.sourceHeight, min: 16, max: 800, step: 8 })
            .width('100%')
            .onChange((value: number) => {
              this.sourceHeight = value;
            })

          Text('提示: sourceSize 控制解码分辨率，不影响显示尺寸')
            .fontSize(12)
            .fontColor(Color.Orange)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 12 }) {
          Text('显示尺寸')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Text('显示宽度: ' + this.displayWidth + 'px')
            .fontSize(14)
            .fontColor(Color.Gray)
          Slider({ value: this.displayWidth, min: 50, max: 350, step: 10 })
            .width('100%')
            .onChange((value: number) => {
              this.displayWidth = value;
            })

          Text('显示高度: ' + this.displayHeight + 'px')
            .fontSize(14)
            .fontColor(Color.Gray)
          Slider({ value: this.displayHeight, min: 50, max: 350, step: 10 })
            .width('100%')
            .onChange((value: number) => {
              this.displayHeight = value;
            })
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#FFF8DC')
        .borderRadius(10)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 12 }) {
          Text('插值抗锯齿')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Row({ space: 15 }) {
            Button(this.enableInterpolation ? '插值: 开启' : '插值: 关闭')
              .fontSize(14)
              .backgroundColor(this.enableInterpolation ? Color.Green : Color.Gray)
              .onClick(() => {
                this.enableInterpolation = !this.enableInterpolation;
              })
          }

          Text(this.enableInterpolation ? '开启插值后图片边缘更平滑，适合放大显示' : '关闭插值后图片使用最近邻采样，性能更好')
            .fontSize(12)
            .fontColor(Color.Blue)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#E6E6FA')
        .borderRadius(10)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 15 }) {
          Text('预设模式')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Grid() {
            ForEach(['缩略图', '标准', '高清', '超高清', '宽屏', '纵向'], (name: string) => {
              GridItem() {
                Column({ space: 5 }) {
                  Text(name)
                    .fontSize(14)
                    .fontWeight(FontWeight.Medium)
                    .fontColor(this.selectedMode === name ? Color.White : Color.Black)

                  Text(this.modes[name].sw + 'x' + this.modes[name].sh)
                    .fontSize(10)
                    .fontColor(this.selectedMode === name ? '#DDDDDD' : Color.Gray)
                }
                .width('100%')
                .height(60)
                .justifyContent(FlexAlign.Center)
                .backgroundColor(this.selectedMode === name ? Color.Blue : '#F0F0F0')
                .borderRadius(8)
                .onClick(() => {
                  this.selectedMode = name;
                  const mode = this.modes[name];
                  this.sourceWidth = mode.sw;
                  this.sourceHeight = mode.sh;
                  this.displayWidth = mode.dw;
                  this.displayHeight = mode.dh;
                })
              }
            })
          }
          .columnsTemplate('1fr 1fr 1fr')
          .rowsGap(10)
          .columnsGap(10)
          .width('100%')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F0FFF0')
        .borderRadius(10)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 10 }) {
          Text('清晰度对比')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Row({ space: 10 }) {
            Column({ space: 5 }) {
              Text('低解码 (50px)')
                .fontSize(11)
                .fontColor(Color.Gray)
              Image($r('app.media.startIcon'))
                .width(140)
                .height(140)
                .sourceSize({ width: 50, height: 50 })
                .interpolate(false)
                .objectFit(ImageFit.Contain)
                .backgroundColor('#EEEEEE')
                .borderRadius(8)
            }

            Column({ space: 5 }) {
              Text('标准解码 (200px)')
                .fontSize(11)
                .fontColor(Color.Gray)
              Image($r('app.media.startIcon'))
                .width(140)
                .height(140)
                .sourceSize({ width: 200, height: 200 })
                .interpolate(true)
                .objectFit(ImageFit.Contain)
                .backgroundColor('#EEEEEE')
                .borderRadius(8)
            }
          }
          .width('100%')
          .justifyContent(FlexAlign.Center)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#FFE4E1')
        .borderRadius(10)

        Text('合理设置 sourceSize 可平衡内存占用与显示质量')
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
}
