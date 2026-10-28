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
 * Image 图片手势交互与变换示例
 * ========================================
 * 本示例演示 Image 组件与手势结合使用的交互效果
 *
 * 涉及的Image API + 手势:
 * 1. Image 基础属性 (width/height/borderRadius/opacity)
 * 2. 缩放手势 (PinchGesture)
 * 3. 旋转手势 (RotationGesture)
 * 4. 拖动手势 (PanGesture)
 * 5. 组合手势 (GestureGroup)
 * 6. 动画过渡 (animateTo)
 *
 * ========================================
 * 如何测试
 * ========================================
 * 1. 双指缩放图片
 * 2. 双指旋转图片
 * 3. 单指拖动图片
 * 4. 点击按钮重置变换
 * 5. 切换不同交互模式
 *
 * 预期结果:
 * - 图片响应手势进行实时变换
 * - 变换过程平滑流畅
 * - 重置后恢复初始状态
 * ========================================
 */

@Entry
@Component
struct ImageExample123 {
  @State scaleValue: number = 1.0;
  @State rotationValue: number = 0;
  @State offsetX: number = 0;
  @State offsetY: number = 0;
  @State imageOpacity: number = 1.0;
  @State currentMode: string = '自由变换';
  @State gestureLog: string = '';
  @State enableAnimation: boolean = true;
  @State imageWidth: number = 200;
  @State imageHeight: number = 200;
  @State borderRadius: number = 0;
  @State showGrid: boolean = false;

  private scaleHistory: number[] = [1.0];
  private maxScale: number = 3.0;
  private minScale: number = 0.3;

  build() {
    Stack() {
      Column({ space: 15 }) {
        Text('图片手势交互示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .fontColor(Color.Black)
          .margin({ top: 10 })

        Text('模式: ' + this.currentMode)
          .fontSize(16)
          .fontColor(Color.Blue)

        Column() {
          if (this.showGrid) {
            Grid() {
              ForEach([1, 2, 3, 4, 5, 6, 7, 8, 9], (item: number) => {
                GridItem() {
                  Column()
                    .width('100%')
                    .height('100%')
                    .border({ width: 0.5, color: '#30000000' })
                }
              })
            }
            .columnsTemplate('1fr 1fr 1fr')
            .rowsTemplate('1fr 1fr 1fr')
            .width(300)
            .height(300)
            .position({ x: 0, y: 0 })
          }

          Image($r('app.media.startIcon'))
            .width(this.imageWidth)
            .height(this.imageHeight)
            .borderRadius(this.borderRadius)
            .opacity(this.imageOpacity)
            .objectFit(ImageFit.Contain)
            .scale({ x: this.scaleValue, y: this.scaleValue })
            .rotate({ angle: this.rotationValue })
            .translate({ x: this.offsetX, y: this.offsetY })
            .shadow({ radius: 20, color: '#40000000', offsetX: 5, offsetY: 5 })
            .gesture(
              this.currentMode === '自由变换' ?
                GestureGroup(GestureMode.Parallel,
                  PinchGesture({ fingers: 2, distance: 3 })
                    .onActionUpdate((event: GestureEvent) => {
                      let newScale = event.scale;
                      newScale = Math.max(this.minScale, Math.min(this.maxScale, newScale));
                      if (this.enableAnimation) {
                        animateTo({ duration: 100, curve: Curve.EaseOut }, () => {
                          this.scaleValue = newScale;
                        });
                      } else {
                        this.scaleValue = newScale;
                      }
                      this.gestureLog = '缩放: ' + newScale.toFixed(2);
                    })
                    .onActionEnd(() => {
                      this.scaleHistory.push(this.scaleValue);
                      this.gestureLog = '缩放结束: ' + this.scaleValue.toFixed(2);
                    }),
                  RotationGesture()
                    .onActionUpdate((event: GestureEvent) => {
                      if (this.enableAnimation) {
                        animateTo({ duration: 100, curve: Curve.EaseOut }, () => {
                          this.rotationValue = event.angle;
                        });
                      } else {
                        this.rotationValue = event.angle;
                      }
                      this.gestureLog = '旋转: ' + event.angle.toFixed(1) + '°';
                    })
                    .onActionEnd(() => {
                      this.gestureLog = '旋转结束: ' + this.rotationValue.toFixed(1) + '°';
                    }),
                  PanGesture({ fingers: 1, distance: 5 })
                    .onActionUpdate((event: GestureEvent) => {
                      if (this.enableAnimation) {
                        animateTo({ duration: 50, curve: Curve.Linear }, () => {
                          this.offsetX = event.offsetX;
                          this.offsetY = event.offsetY;
                        });
                      } else {
                        this.offsetX = event.offsetX;
                        this.offsetY = event.offsetY;
                      }
                      this.gestureLog = '拖动: (' + event.offsetX.toFixed(0) + ', ' + event.offsetY.toFixed(0) + ')';
                    })
                    .onActionEnd(() => {
                      this.gestureLog = '拖动结束';
                    })
                ) :
                this.currentMode === '仅缩放' ?
                  PinchGesture({ fingers: 2, distance: 3 })
                    .onActionUpdate((event: GestureEvent) => {
                      let newScale = Math.max(this.minScale, Math.min(this.maxScale, event.scale));
                      this.scaleValue = newScale;
                      this.gestureLog = '缩放: ' + newScale.toFixed(2);
                    }) :
                  this.currentMode === '仅旋转' ?
                    RotationGesture()
                      .onActionUpdate((event: GestureEvent) => {
                        this.rotationValue = event.angle;
                        this.gestureLog = '旋转: ' + event.angle.toFixed(1) + '°';
                      }) :
                    PanGesture({ fingers: 1, distance: 5 })
                      .onActionUpdate((event: GestureEvent) => {
                        this.offsetX = event.offsetX;
                        this.offsetY = event.offsetY;
                        this.gestureLog = '拖动: (' + event.offsetX.toFixed(0) + ', ' + event.offsetY.toFixed(0) + ')';
                      })
            )
        }
        .width(300)
        .height(300)
        .backgroundColor('#F5F5F5')
        .borderRadius(15)
        .border({ width: 1, color: '#E0E0E0' })
        .justifyContent(FlexAlign.Center)
        .alignItems(HorizontalAlign.Center)
        .clip(true)

        Text(this.gestureLog)
          .fontSize(14)
          .fontColor(Color.Gray)
          .height(20)

        Row({ space: 10 }) {
          Text('缩放: ' + this.scaleValue.toFixed(2) + 'x')
            .fontSize(12)
            .fontColor(Color.Blue)
          Text('旋转: ' + this.rotationValue.toFixed(0) + '°')
            .fontSize(12)
            .fontColor(Color.Green)
          Text('偏移: (' + this.offsetX.toFixed(0) + ',' + this.offsetY.toFixed(0) + ')')
            .fontSize(12)
            .fontColor(Color.Orange)
        }

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 10 }) {
          Text('交互模式')
            .fontSize(16)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Row({ space: 8 }) {
            ForEach(['自由变换', '仅缩放', '仅旋转', '仅拖动'], (mode: string) => {
              Button(mode)
                .fontSize(12)
                .backgroundColor(this.currentMode === mode ? Color.Blue : Color.LightGray)
                .fontColor(this.currentMode === mode ? Color.White : Color.Black)
                .onClick(() => {
                  this.currentMode = mode;
                })
            })
          }
        }
        .width('100%')
        .padding(12)
        .backgroundColor('#F8F8FF')
        .borderRadius(10)

        Column({ space: 10 }) {
          Text('图片属性')
            .fontSize(16)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Text('透明度: ' + (this.imageOpacity * 100).toFixed(0) + '%')
            .fontSize(13)
            .fontColor(Color.Gray)
          Slider({ value: this.imageOpacity * 100, min: 10, max: 100, step: 5 })
            .width('100%')
            .onChange((value: number) => {
              this.imageOpacity = value / 100;
            })

          Text('圆角: ' + this.borderRadius + 'px')
            .fontSize(13)
            .fontColor(Color.Gray)
          Slider({ value: this.borderRadius, min: 0, max: 100, step: 5 })
            .width('100%')
            .onChange((value: number) => {
              this.borderRadius = value;
            })

          Text('图片宽度: ' + this.imageWidth + 'px')
            .fontSize(13)
            .fontColor(Color.Gray)
          Slider({ value: this.imageWidth, min: 80, max: 280, step: 10 })
            .width('100%')
            .onChange((value: number) => {
              this.imageWidth = value;
              this.imageHeight = value;
            })
        }
        .width('100%')
        .padding(12)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Row({ space: 10 }) {
          Button('动画: ' + (this.enableAnimation ? '开' : '关'))
            .fontSize(12)
            .backgroundColor(this.enableAnimation ? Color.Green : Color.Gray)
            .onClick(() => {
              this.enableAnimation = !this.enableAnimation;
            })

          Button('参考网格: ' + (this.showGrid ? '开' : '关'))
            .fontSize(12)
            .backgroundColor(this.showGrid ? Color.Orange : Color.Gray)
            .onClick(() => {
              this.showGrid = !this.showGrid;
            })

          Button('重置')
            .fontSize(12)
            .backgroundColor(Color.Red)
            .onClick(() => {
              if (this.enableAnimation) {
                animateTo({ duration: 300, curve: Curve.EaseInOut }, () => {
                  this.scaleValue = 1.0;
                  this.rotationValue = 0;
                  this.offsetX = 0;
                  this.offsetY = 0;
                  this.imageOpacity = 1.0;
                  this.borderRadius = 0;
                  this.imageWidth = 200;
                  this.imageHeight = 200;
                });
              } else {
                this.scaleValue = 1.0;
                this.rotationValue = 0;
                this.offsetX = 0;
                this.offsetY = 0;
                this.imageOpacity = 1.0;
                this.borderRadius = 0;
                this.imageWidth = 200;
                this.imageHeight = 200;
              }
              this.gestureLog = '已重置';
              this.scaleHistory = [1.0];
            })
        }
        .width('100%')
        .justifyContent(FlexAlign.Center)

        Text('双指操作可在设备上体验手势交互')
          .fontSize(12)
          .fontColor(Color.Blue)
          .margin({ bottom: 20 })
      }
      .width('100%')
      .height('100%')
      .padding({ left: 15, right: 15 })
    }
    .width('100%')
    .height('100%')
    .backgroundColor(Color.White)
  }
}
