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
 * ImageAnimator 帧动画控制器示例
 * ========================================
 * 本示例演示 ImageAnimator 组件的完整控制能力
 *
 * 涉及的 ImageAnimator API:
 * 1. images 属性 - 帧图片列表
 * 2. duration 属性 - 单帧持续时间
 * 3. state 属性 - 动画状态控制
 * 4. reverse 属性 - 反向播放
 * 5. fillMode 属性 - 填充模式
 * 6. iterations 属性 - 循环次数
 * 7. fixedSize 属性 - 固定尺寸模式
 * 8. 回调事件 (onStart/onPause/onRepeat/onCancel/onFinish)
 *
 * ========================================
 * 如何测试
 * ========================================
 * 1. 使用控制按钮操作动画状态
 * 2. 调整 duration 观察帧速率变化
 * 3. 切换 fillMode 观察动画停止时的帧显示
 * 4. 设置不同 iterations 观察循环行为
 * 5. 查看回调日志确认事件触发
 *
 * 预期结果:
 * - 动画按设定帧率播放
 * - 状态切换即时生效
 * - 回调事件正确触发
 * - fillMode 影响动画结束后的显示帧
 * ========================================
 */

@Entry
@Component
struct ImageAnimatorExample019 {
  @State animState: AnimationStatus = AnimationStatus.Initial;
  @State reverse: boolean = false;
  @State iterations: number = 1;
  @State duration: number = 500;
  @State selectedFillMode: string = 'None';
  @State fixedSize: boolean = false;
  @State logMessages: string[] = [];
  @State currentFrame: number = 0;
  @State enableCustomFrame: boolean = false;
  @State customFrameIndex: number = 0;
  @State animatorWidth: number = 240;
  @State animatorHeight: number = 240;

  private fillModes: string[] = ['None', 'Forwards', 'Backwards', 'Both'];
  private frameColors: ResourceColor[] = ['#FF6B6B', '#4ECDC4', '#45B7D1', '#96CEB4', '#FFEAA7', '#DDA0DD'];

  build() {
    Scroll() {
      Column({ space: 15 }) {
        Text('帧动画控制器示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .fontColor(Color.Black)
          .margin({ top: 20, bottom: 5 })

        Column({ space: 10 }) {
          Text('动画预览')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Stack() {
            ImageAnimator()
              .images([
                { src: $r('app.media.startIcon') },
                { src: $r('app.media.icon') },
                { src: $r('app.media.heart') },
                { src: $r('app.media.star') },
                { src: $r('app.media.svg') },
              ])
              .duration(this.duration)
              .state(this.animState)
              .reverse(this.reverse)
              .fillMode(this.getFillMode())
              .iterations(this.iterations)
              .fixedSize(this.fixedSize)
              .width(this.animatorWidth)
              .height(this.animatorHeight)
              .objectFit(ImageFit.Contain)
              .onStart(() => {
                this.addLog('▶ 动画开始');
              })
              .onPause(() => {
                this.addLog('⏸ 动画暂停');
              })
              .onRepeat(() => {
                this.addLog('🔄 动画重复');
              })
              .onCancel(() => {
                this.addLog('⏹ 动画取消');
              })
              .onFinish(() => {
                this.addLog('✓ 动画完成');
                this.animState = AnimationStatus.Stopped;
              })
          }
          .width(this.animatorWidth + 20)
          .height(this.animatorHeight + 20)
          .backgroundColor('#F0F0F0')
          .borderRadius(15)
          .shadow({ radius: 15, color: '#30000000' })
          .justifyContent(FlexAlign.Center)

          Row({ space: 15 }) {
            Text('状态: ' + this.getStateText())
              .fontSize(13)
              .fontColor(Color.Blue)
            Text('时长: ' + this.duration + 'ms/帧')
              .fontSize(13)
              .fontColor(Color.Green)
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F8F8FF')
        .borderRadius(15)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 10 }) {
          Text('播放控制')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Row({ space: 8 }) {
            Button('▶ 播放')
              .fontSize(13)
              .backgroundColor(Color.Green)
              .onClick(() => {
                this.animState = AnimationStatus.Running;
              })

            Button('⏸ 暂停')
              .fontSize(13)
              .backgroundColor(Color.Orange)
              .onClick(() => {
                this.animState = AnimationStatus.Paused;
              })

            Button('⏹ 停止')
              .fontSize(13)
              .backgroundColor(Color.Red)
              .onClick(() => {
                this.animState = AnimationStatus.Stopped;
              })
          }

          Row({ space: 8 }) {
            Button('⏮ 初始')
              .fontSize(13)
              .backgroundColor(Color.Gray)
              .onClick(() => {
                this.animState = AnimationStatus.Initial;
                this.addLog('⏮ 回到初始状态');
              })

            Button('🔄 反向: ' + (this.reverse ? '开' : '关'))
              .fontSize(13)
              .backgroundColor(this.reverse ? Color.Purple : Color.LightGray)
              .fontColor(this.reverse ? Color.White : Color.Black)
              .onClick(() => {
                this.reverse = !this.reverse;
                this.addLog('反向: ' + (this.reverse ? '开' : '关'));
              })
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#FFF8DC')
        .borderRadius(10)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 10 }) {
          Text('帧速率控制')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Text('单帧时长: ' + this.duration + 'ms (约 ' + (1000 / this.duration).toFixed(1) + ' FPS)')
            .fontSize(14)
            .fontColor(Color.Gray)

          Slider({ value: this.duration, min: 50, max: 2000, step: 50 })
            .width('100%')
            .onChange((value: number) => {
              this.duration = value;
            })

          Row({ space: 8 }) {
            ForEach([50, 100, 200, 500, 1000], (val: number) => {
              Button(val + 'ms')
                .fontSize(11)
                .backgroundColor(this.duration === val ? Color.Blue : Color.LightGray)
                .fontColor(this.duration === val ? Color.White : Color.Black)
                .onClick(() => {
                  this.duration = val;
                })
            })
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 10 }) {
          Text('循环与填充')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Text('循环次数: ' + (this.iterations === -1 ? '无限' : this.iterations.toString()))
            .fontSize(14)
            .fontColor(Color.Gray)

          Row({ space: 8 }) {
            ForEach([1, 2, 3, 5, -1], (val: number) => {
              Button(val === -1 ? '无限' : val.toString() + '次')
                .fontSize(11)
                .backgroundColor(this.iterations === val ? Color.Green : Color.LightGray)
                .fontColor(this.iterations === val ? Color.White : Color.Black)
                .onClick(() => {
                  this.iterations = val;
                })
            })
          }

          Text('填充模式 (fillMode):')
            .fontSize(14)
            .fontColor(Color.Gray)
            .margin({ top: 5 })

          Row({ space: 8 }) {
            ForEach(this.fillModes, (mode: string) => {
              Button(mode)
                .fontSize(11)
                .backgroundColor(this.selectedFillMode === mode ? Color.Purple : Color.LightGray)
                .fontColor(this.selectedFillMode === mode ? Color.White : Color.Black)
                .onClick(() => {
                  this.selectedFillMode = mode;
                  this.addLog('fillMode: ' + mode);
                })
            })
          }

          Toggle({ type: ToggleType.Switch, isOn: this.fixedSize })
            .onChange((isOn: boolean) => {
              this.fixedSize = isOn;
              this.addLog('fixedSize: ' + (isOn ? '开' : '关'));
            })
          Text('fixedSize 模式')
            .fontSize(12)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#E6E6FA')
        .borderRadius(10)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 10 }) {
          Text('尺寸控制')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Text('宽度: ' + this.animatorWidth)
            .fontSize(13)
            .fontColor(Color.Gray)
          Slider({ value: this.animatorWidth, min: 100, max: 340, step: 10 })
            .width('100%')
            .onChange((value: number) => { this.animatorWidth = value; })

          Text('高度: ' + this.animatorHeight)
            .fontSize(13)
            .fontColor(Color.Gray)
          Slider({ value: this.animatorHeight, min: 100, max: 340, step: 10 })
            .width('100%')
            .onChange((value: number) => { this.animatorHeight = value; })
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F0FFF0')
        .borderRadius(10)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 10 }) {
          Text('事件日志')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Scroll() {
            Column() {
              if (this.logMessages.length === 0) {
                Text('暂无日志，请操作动画...')
                  .fontSize(13)
                  .fontColor(Color.Gray)
              } else {
                ForEach(this.logMessages.slice().reverse(), (msg: string, index: number) => {
                  Text(msg)
                    .fontSize(12)
                    .fontColor(Color.Black)
                    .padding(5)
                    .backgroundColor(index % 2 === 0 ? '#F8F8F8' : '#FFFFFF')
                    .width('100%')
                    .borderRadius(4)
                })
              }
            }
            .width('100%')
          }
          .height(150)
          .backgroundColor('#FAFAFA')
          .borderRadius(8)
          .border({ width: 1, color: '#E0E0E0' })

          Row({ space: 10 }) {
            Button('清空日志')
              .fontSize(12)
              .backgroundColor(Color.Gray)
              .onClick(() => {
                this.logMessages = [];
              })

            Button('重置全部')
              .fontSize(12)
              .backgroundColor(Color.Red)
              .onClick(() => {
                this.animState = AnimationStatus.Initial;
                this.reverse = false;
                this.iterations = 1;
                this.duration = 500;
                this.selectedFillMode = 'None';
                this.fixedSize = false;
                this.logMessages = [];
                this.animatorWidth = 240;
                this.animatorHeight = 240;
              })
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#FFE4E1')
        .borderRadius(10)

        Text('ImageAnimator 支持完整的帧动画控制能力')
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

  private addLog(msg: string) {
    const time = new Date();
    const timeStr = time.getHours().toString().padStart(2, '0') + ':' +
      time.getMinutes().toString().padStart(2, '0') + ':' +
      time.getSeconds().toString().padStart(2, '0');
    this.logMessages.push('[' + timeStr + '] ' + msg);
    if (this.logMessages.length > 50) {
      this.logMessages.shift();
    }
  }

  private getStateText(): string {
    switch (this.animState) {
      case AnimationStatus.Initial: return '初始';
      case AnimationStatus.Running: return '播放中';
      case AnimationStatus.Paused: return '已暂停';
      case AnimationStatus.Stopped: return '已停止';
      default: return '未知';
    }
  }

  private getFillMode(): FillMode {
    switch (this.selectedFillMode) {
      case 'None': return FillMode.None;
      case 'Forwards': return FillMode.Forwards;
      case 'Backwards': return FillMode.Backwards;
      case 'Both': return FillMode.Both;
      default: return FillMode.None;
    }
  }
}
