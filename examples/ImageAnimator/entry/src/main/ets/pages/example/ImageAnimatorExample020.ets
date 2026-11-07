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
 * ImageAnimator 多动画组合与状态切换示例
 * ========================================
 * 本示例演示多个 ImageAnimator 组合使用及状态联动
 *
 * 涉及的能力:
 * 1. 多个 ImageAnimator 并行/串行控制
 * 2. 动画状态联动（一个完成触发另一个）
 * 3. 不同动画参数组合对比
 * 4. 场景化动画编排
 * 5. 动画预设方案切换
 *
 * ========================================
 * 如何测试
 * ========================================
 * 1. 选择预设动画方案
 * 2. 点击播放观察多动画协调效果
 * 3. 切换不同方案对比动画差异
 * 4. 观察动画联动触发行为
 *
 * 预期结果:
 * - 多个动画按预设方案协调播放
 * - 联动动画在触发条件下自动启动
 * - 不同方案呈现不同视觉效果
 * ========================================
 */

@Entry
@Component
struct ImageAnimatorExample020 {
  @State primaryState: AnimationStatus = AnimationStatus.Initial;
  @State secondaryState: AnimationStatus = AnimationStatus.Initial;
  @State tertiaryState: AnimationStatus = AnimationStatus.Initial;
  @State selectedPreset: string = '序列播放';
  @State primaryDuration: number = 300;
  @State secondaryDuration: number = 500;
  @State tertiaryDuration: number = 700;
  @State chainEnabled: boolean = true;
  @State statusText: string = '就绪';
  @State primaryReverse: boolean = false;
  @State secondaryReverse: boolean = false;
  @State tertiaryReverse: boolean = false;
  @State primaryIterations: number = 1;
  @State secondaryIterations: number = 1;
  @State tertiaryIterations: number = -1;
  @State showLabels: boolean = true;

  private presets: Record<string, {
    desc: string,
    pd: number, sd: number, td: number,
    pr: boolean, sr: boolean, tr: boolean,
    pi: number, si: number, ti: number,
    chain: boolean
  }> = {
    '序列播放': {
      desc: '三个动画依次播放，前一个完成后触发下一个',
      pd: 300, sd: 300, td: 300,
      pr: false, sr: false, tr: false,
      pi: 1, si: 1, ti: 1,
      chain: true
    },
    '同步播放': {
      desc: '三个动画同时播放，不同速率形成对比',
      pd: 200, sd: 400, td: 600,
      pr: false, sr: false, tr: false,
      pi: -1, si: -1, ti: -1,
      chain: false
    },
    '往返运动': {
      desc: '动画正反交替播放形成往复效果',
      pd: 400, sd: 600, td: 800,
      pr: true, sr: true, tr: true,
      pi: -1, si: -1, ti: -1,
      chain: false
    },
    '渐次加速': {
      desc: '从慢到快，形成加速追赶效果',
      pd: 800, sd: 400, td: 200,
      pr: false, sr: false, tr: false,
      pi: -1, si: -1, ti: -1,
      chain: false
    },
    '单次展示': {
      desc: '每个动画只播放一次后停在末帧',
      pd: 500, sd: 700, td: 900,
      pr: false, sr: false, tr: false,
      pi: 1, si: 1, ti: 1,
      chain: false
    },
    '混合循环': {
      desc: '不同循环次数组合，观察错落效果',
      pd: 300, sd: 500, td: 400,
      pr: false, sr: true, tr: false,
      pi: 3, si: 2, ti: 5,
      chain: false
    },
  };

  build() {
    Scroll() {
      Column({ space: 15 }) {
        Text('多动画组合示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .fontColor(Color.Black)
          .margin({ top: 20, bottom: 5 })

        Text('方案: ' + this.selectedPreset)
          .fontSize(16)
          .fontColor(Color.Blue)
        Text(this.presets[this.selectedPreset].desc)
          .fontSize(13)
          .fontColor(Color.Gray)

        Column({ space: 12 }) {
          Text('动画预览区')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Column({ space: 8 }) {
            if (this.showLabels) {
              Text('主动画 (' + this.primaryDuration + 'ms/帧)')
                .fontSize(12)
                .fontColor('#FF6B6B')
            }
            Stack() {
              ImageAnimator()
                .images([
                  { src: $r('app.media.startIcon') },
                  { src: $r('app.media.icon') },
                  { src: $r('app.media.heart') },
                  { src: $r('app.media.star') },
                ])
                .duration(this.primaryDuration)
                .state(this.primaryState)
                .reverse(this.primaryReverse)
                .iterations(this.primaryIterations)
                .fillMode(FillMode.Forwards)
                .width(200)
                .height(120)
                .onStart(() => { this.statusText = '主动画播放中'; })
                .onFinish(() => {
                  this.statusText = '主动画完成';
                  if (this.chainEnabled) {
                    this.secondaryState = AnimationStatus.Running;
                  }
                })
            }
            .width(220)
            .height(130)
            .backgroundColor('#FFF0F0')
            .borderRadius(10)
            .border({ width: 2, color: '#FF6B6B' })
            .justifyContent(FlexAlign.Center)
          }
          .width('100%')
          .alignItems(HorizontalAlign.Center)

          Column({ space: 8 }) {
            if (this.showLabels) {
              Text('副动画 (' + this.secondaryDuration + 'ms/帧)')
                .fontSize(12)
                .fontColor('#4ECDC4')
            }
            Stack() {
              ImageAnimator()
                .images([
                  { src: $r('app.media.svg') },
                  { src: $r('app.media.star') },
                  { src: $r('app.media.heart') },
                  { src: $r('app.media.icon') },
                ])
                .duration(this.secondaryDuration)
                .state(this.secondaryState)
                .reverse(this.secondaryReverse)
                .iterations(this.secondaryIterations)
                .fillMode(FillMode.Forwards)
                .width(200)
                .height(120)
                .onStart(() => { this.statusText = '副动画播放中'; })
                .onFinish(() => {
                  this.statusText = '副动画完成';
                  if (this.chainEnabled) {
                    this.tertiaryState = AnimationStatus.Running;
                  }
                })
            }
            .width(220)
            .height(130)
            .backgroundColor('#F0FFFC')
            .borderRadius(10)
            .border({ width: 2, color: '#4ECDC4' })
            .justifyContent(FlexAlign.Center)
          }
          .width('100%')
          .alignItems(HorizontalAlign.Center)

          Column({ space: 8 }) {
            if (this.showLabels) {
              Text('第三动画 (' + this.tertiaryDuration + 'ms/帧)')
                .fontSize(12)
                .fontColor('#45B7D1')
            }
            Stack() {
              ImageAnimator()
                .images([
                  { src: $r('app.media.heart') },
                  { src: $r('app.media.startIcon') },
                  { src: $r('app.media.svg') },
                  { src: $r('app.media.star') },
                ])
                .duration(this.tertiaryDuration)
                .state(this.tertiaryState)
                .reverse(this.tertiaryReverse)
                .iterations(this.tertiaryIterations)
                .fillMode(FillMode.Forwards)
                .width(200)
                .height(120)
                .onStart(() => { this.statusText = '第三动画播放中'; })
                .onFinish(() => {
                  this.statusText = '第三动画完成';
                })
            }
            .width(220)
            .height(130)
            .backgroundColor('#F0F8FF')
            .borderRadius(10)
            .border({ width: 2, color: '#45B7D1' })
            .justifyContent(FlexAlign.Center)
          }
          .width('100%')
          .alignItems(HorizontalAlign.Center)

          Text('状态: ' + this.statusText)
            .fontSize(14)
            .fontColor(Color.Blue)
            .margin({ top: 5 })
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F8F8FF')
        .borderRadius(15)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 10 }) {
          Text('全局控制')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Row({ space: 8 }) {
            Button('▶ 全部播放')
              .fontSize(13)
              .backgroundColor(Color.Green)
              .onClick(() => {
                this.primaryState = AnimationStatus.Running;
                if (!this.chainEnabled) {
                  this.secondaryState = AnimationStatus.Running;
                  this.tertiaryState = AnimationStatus.Running;
                }
                this.statusText = '全部播放中';
              })

            Button('⏸ 全部暂停')
              .fontSize(13)
              .backgroundColor(Color.Orange)
              .onClick(() => {
                this.primaryState = AnimationStatus.Paused;
                this.secondaryState = AnimationStatus.Paused;
                this.tertiaryState = AnimationStatus.Paused;
                this.statusText = '全部暂停';
              })
          }

          Row({ space: 8 }) {
            Button('⏹ 全部停止')
              .fontSize(13)
              .backgroundColor(Color.Red)
              .onClick(() => {
                this.primaryState = AnimationStatus.Stopped;
                this.secondaryState = AnimationStatus.Stopped;
                this.tertiaryState = AnimationStatus.Stopped;
                this.statusText = '全部停止';
              })

            Button('⏮ 全部重置')
              .fontSize(13)
              .backgroundColor(Color.Gray)
              .onClick(() => {
                this.primaryState = AnimationStatus.Initial;
                this.secondaryState = AnimationStatus.Initial;
                this.tertiaryState = AnimationStatus.Initial;
                this.statusText = '就绪';
              })
          }

          Toggle({ type: ToggleType.Switch, isOn: this.chainEnabled })
            .onChange((isOn: boolean) => {
              this.chainEnabled = isOn;
            })
          Text('链式触发: ' + (this.chainEnabled ? '开启 (依次触发)' : '关闭 (同时播放)'))
            .fontSize(13)
            .fontColor(Color.Gray)

          Toggle({ type: ToggleType.Switch, isOn: this.showLabels })
            .onChange((isOn: boolean) => {
              this.showLabels = isOn;
            })
          Text('显示标签')
            .fontSize(13)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#FFF8DC')
        .borderRadius(10)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 10 }) {
          Text('预设方案')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Grid() {
            ForEach(['序列播放', '同步播放', '往返运动', '渐次加速', '单次展示', '混合循环'], (name: string) => {
              GridItem() {
                Column({ space: 5 }) {
                  Text(name)
                    .fontSize(13)
                    .fontWeight(FontWeight.Medium)
                    .fontColor(this.selectedPreset === name ? Color.White : Color.Black)

                  Text(this.presets[name].desc.substring(0, 12) + '...')
                    .fontSize(10)
                    .fontColor(this.selectedPreset === name ? '#DDDDDD' : Color.Gray)
                    .maxLines(1)
                    .textOverflow({ overflow: TextOverflow.Ellipsis })
                }
                .width('100%')
                .height(65)
                .justifyContent(FlexAlign.Center)
                .padding(5)
                .backgroundColor(this.selectedPreset === name ? Color.Blue : '#F0F0F0')
                .borderRadius(8)
                .onClick(() => {
                  this.applyPreset(name);
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
        .backgroundColor('#E6E6FA')
        .borderRadius(10)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 10 }) {
          Text('各动画时长微调')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          Text('主动画: ' + this.primaryDuration + 'ms')
            .fontSize(13)
            .fontColor('#FF6B6B')
          Slider({ value: this.primaryDuration, min: 50, max: 1500, step: 50 })
            .width('100%')
            .onChange((value: number) => { this.primaryDuration = value; })

          Text('副动画: ' + this.secondaryDuration + 'ms')
            .fontSize(13)
            .fontColor('#4ECDC4')
          Slider({ value: this.secondaryDuration, min: 50, max: 1500, step: 50 })
            .width('100%')
            .onChange((value: number) => { this.secondaryDuration = value; })

          Text('第三动画: ' + this.tertiaryDuration + 'ms')
            .fontSize(13)
            .fontColor('#45B7D1')
          Slider({ value: this.tertiaryDuration, min: 50, max: 1500, step: 50 })
            .width('100%')
            .onChange((value: number) => { this.tertiaryDuration = value; })
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Divider().strokeWidth(2).color('#E0E0E0')

        Column({ space: 10 }) {
          Text('场景化编排说明')
            .fontSize(18)
            .fontWeight(FontWeight.Medium)
            .fontColor(Color.Black)

          this.ScenarioItem('加载动画', '主动画显示加载进度，完成后触发副动画展示内容')
          this.ScenarioItem('引导页', '三个动画依次播放，讲述连贯的故事情节')
          this.ScenarioItem('状态反馈', '操作后触发序列动画，给予用户视觉反馈')
          this.ScenarioItem('装饰效果', '多个动画同步循环播放，营造动态背景氛围')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F0FFF0')
        .borderRadius(10)

        Text('多动画组合可实现丰富的视觉编排效果')
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
  ScenarioItem(title: string, desc: string) {
    Row({ space: 10 }) {
      Text('•')
        .fontSize(16)
        .fontColor(Color.Blue)
      Column({ space: 2 }) {
        Text(title)
          .fontSize(14)
          .fontWeight(FontWeight.Medium)
          .fontColor(Color.Black)
        Text(desc)
          .fontSize(12)
          .fontColor(Color.Gray)
      }
      .alignItems(HorizontalAlign.Start)
    }
    .width('100%')
    .alignItems(VerticalAlign.Top)
  }

  private applyPreset(name: string) {
    this.selectedPreset = name;
    const preset = this.presets[name];
    this.primaryDuration = preset.pd;
    this.secondaryDuration = preset.sd;
    this.tertiaryDuration = preset.td;
    this.primaryReverse = preset.pr;
    this.secondaryReverse = preset.sr;
    this.tertiaryReverse = preset.tr;
    this.primaryIterations = preset.pi;
    this.secondaryIterations = preset.si;
    this.tertiaryIterations = preset.ti;
    this.chainEnabled = preset.chain;

    this.primaryState = AnimationStatus.Initial;
    this.secondaryState = AnimationStatus.Initial;
    this.tertiaryState = AnimationStatus.Initial;
    this.statusText = '就绪 - 已切换到: ' + name;
  }
}
