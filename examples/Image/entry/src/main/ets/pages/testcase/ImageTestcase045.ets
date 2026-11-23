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


@Entry
@Component
struct ImageResponseRegionTestCase073 {
  @State currentPresetIndex: number = 0
  @State currentBehaviorIndex: number = 0
  @State tapCount: number = 0
  @State lastTapArea: string = '点击图片或周围区域测试'
  @State showGuide: boolean = true
  @State imageScale: number = 1.0

  private readonly REGION_PRESETS: Array<{
    name: string
    region: X2Y2Z2W2 | [X2Y2Z2W2]
    desc: string
  }> = [
    {
      name: '默认(无扩展)',
      region: { x: 0, y: 0, z: 0, w: 0 },
      desc: '响应区域=图片实际区域'
    },
    {
      name: '四周扩展20',
      region: { x: -20, y: -20, z: 20, w: 20 },
      desc: '向四周扩展20vp触摸区域'
    },
    {
      name: '四周扩展40',
      region: { x: -40, y: -40, z: 40, w: 40 },
      desc: '向四周扩展40vp触摸区域'
    },
    {
      name: '仅左右扩展',
      region: { x: -30, y: 0, z: 30, w: 0 },
      desc: '水平方向扩展30vp'
    },
    {
      name: '仅上下扩展',
      region: { x: 0, y: -30, z: 0, w: 30 },
      desc: '垂直方向扩展30vp'
    },
    {
      name: '右侧扩展',
      region: { x: 0, y: 0, z: 60, w: 0 },
      desc: '仅右侧扩展60vp'
    },
    {
      name: '缩小响应',
      region: { x: 20, y: 20, z: -20, w: -20 },
      desc: '响应区域缩小20vp'
    },
    {
      name: '多区域',
      region: [
        { x: -20, y: -20, z: 20, w: 20 },
        { x: -50, y: 0, z: 50, w: 0 }
      ],
      desc: '多个响应区域叠加'
    }
  ]

  private readonly HIT_BEHAVIORS: Array<{ behavior: HitTestMode; label: string; desc: string }> = [
    { behavior: HitTestMode.Default, label: 'Default', desc: '默认: 自身+子节点响应' },
    { behavior: HitTestMode.Block, label: 'Block', desc: '阻断: 阻止子节点和兄弟节点' },
    { behavior: HitTestMode.Transparent, label: 'Transparent', desc: '透明: 自身不响应,透传' }
  ]

  build() {
    Column() {
      Text('Image responseRegion响应区域测试')
        .fontSize(18)
        .fontWeight(FontWeight.Bold)
        .margin({ bottom: 16 })

      Stack() {
        if (this.showGuide) {
          Column()
            .width(200)
            .height(200)
            .backgroundColor('#E8F5E9')
            .border({ width: 1, color: '#A5D6A7', style: BorderStyle.Dashed })
            .borderRadius(8)
        }

        Column()
          .width(120)
          .height(120)
          .backgroundColor('#FFF3E0')
          .border({ width: 1, color: '#FFB74D', style: BorderStyle.Dashed })
          .borderRadius(8)

        Image($r('app.media.startIcon'))
          .width(120)
          .height(120)
          .objectFit(ImageFit.Cover)
          .border({ width: 2, color: Color.Blue })
          .borderRadius(8)
          .scale({ x: this.imageScale, y: this.imageScale })
          .responseRegion(this.REGION_PRESETS[this.currentPresetIndex].region)
          .hitTestBehavior(this.HIT_BEHAVIORS[this.currentBehaviorIndex].behavior)
          .onClick(() => {
            this.tapCount++
            this.imageScale = 1.2
            this.lastTapArea = `点击命中 #${this.tapCount}`
            setTimeout(() => { this.imageScale = 1.0 }, 200)
          })
      }
      .width(240)
      .height(240)
      .alignContent(Alignment.Center)
      .margin({ bottom: 16 })

      Text(this.lastTapArea)
        .fontSize(13)
        .fontColor(Color.Blue)
        .margin({ bottom: 8 })

      Text(this.REGION_PRESETS[this.currentPresetIndex].name)
        .fontSize(12)
        .fontColor(Color.Gray)
        .margin({ bottom: 4 })

      Text(this.REGION_PRESETS[this.currentPresetIndex].desc)
        .fontSize(11)
        .fontColor(Color.Gray)
        .margin({ bottom: 8 })

      Text(`点击次数: ${this.tapCount}`)
        .fontSize(12)
        .fontColor(Color.Orange)
        .margin({ bottom: 16 })

      Divider()

      Text('响应区域预设(点击切换)')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Grid() {
        ForEach(this.REGION_PRESETS, (config: {
          name: string
          region: X2Y2Z2W2 | [X2Y2Z2W2]
          desc: string
        }, index: number) => {
          GridItem() {
            Text(config.name)
              .fontSize(9)
              .fontColor(index === this.currentPresetIndex ? Color.Blue : Color.Gray)
              .maxLines(2)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
              .padding(6)
              .backgroundColor(index === this.currentPresetIndex ? '#E3F2FD' : '#FAFAFA')
              .borderRadius(4)
              .border({ width: 1, color: index === this.currentPresetIndex ? Color.Blue : '#EEEEEE' })
          }
          .onClick(() => {
            this.currentPresetIndex = index
            this.lastTapArea = `已切换到 ${config.name}`
          })
        })
      }
      .columnsTemplate('repeat(2, 1fr)')
      .rowsGap(6)
      .columnsGap(6)
      .padding(8)
      .margin({ bottom: 16 })

      Divider()

      Text('hitTestBehavior命中模式')
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Row() {
        ForEach(this.HIT_BEHAVIORS, (config: { behavior: HitTestMode; label: string; desc: string }, index: number) => {
          Button(config.label)
            .fontSize(9)
            .onClick(() => { this.currentBehaviorIndex = index })
            .backgroundColor(index === this.currentBehaviorIndex ? '#2196F3' : '#EEEEEE')
            .fontColor(index === this.currentBehaviorIndex ? Color.White : Color.Gray)
            .margin({ left: 3, right: 3 })
        })
      }
      .margin({ bottom: 8 })

      Text(this.HIT_BEHAVIORS[this.currentBehaviorIndex].desc)
        .fontSize(10)
        .fontColor(Color.Gray)
        .margin({ bottom: 16 })

      Divider()

      Row({ space: 12 }) {
        Button(this.showGuide ? '引导框: 显' : '引导框: 隐')
          .fontSize(10)
          .onClick(() => { this.showGuide = !this.showGuide })
          .backgroundColor(this.showGuide ? '#4CAF50' : '#EEEEEE')
          .fontColor(this.showGuide ? Color.White : Color.Gray)
        Button('重置')
          .fontSize(10)
          .backgroundColor('#FF6666')
          .fontColor(Color.White)
          .onClick(() => {
            this.currentPresetIndex = 0
            this.currentBehaviorIndex = 0
            this.tapCount = 0
            this.lastTapArea = '已重置'
            this.imageScale = 1.0
          })
      }
      .margin({ top: 8 })

      Divider()

      Text('图例')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
        .margin({ top: 16, bottom: 8 })

      Text('绿色虚线框 = 扩展后的响应区域(引导参考)\n' +
        '橙色虚线框 = 默认区域参考\n' +
        '蓝色实线 = 图片实际尺寸\n' +
        '点击绿色区域内即可触发图片onClick')
        .fontSize(10)
        .fontColor(Color.Gray)
        .lineHeight(16)
    }
    .width('100%')
    .height('100%')
    .padding(16)
  }
}
