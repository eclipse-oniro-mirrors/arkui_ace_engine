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
 * ========================================
 * SVG2 能力逐个走查示例 (supportSvg2 API 21+)
 * ========================================
 * 功能说明:
 * 本示例按《SVG标签解析能力增强》文档的能力分类，对 6 个 svg2_*.svg 图源
 * 逐个走查 supportSvg2(true) 与 supportSvg2(false) 的解析差异。
 * 与 ImageSVG2Example（滤镜/开关综合页）不同，本页聚焦“能力项 → 图源”映射：
 * 1. svg2_coordinates.svg    坐标系与仿射变换（viewBox/preserveAspectRatio、
 *                            transform-origin、rotate局部中心点、matrix、非法值校验）
 * 2. svg2_gradient_linear.svg 线性渐变（gradientUnits 两基准、平移缩放增强）
 * 3. svg2_gradient_radial.svg 径向渐变（gradientUnits、fx/fy 焦点平移缩放）
 * 4. svg2_mask_pattern.svg   遮罩与图案（maskUnits/maskContentUnits、
 *                            patternUnits/patternContentUnits、平铺与偏移值处理）
 * 5. svg2_filter_complex.svg 滤镜（filterUnits/primitiveUnits、原语 feFlood/feOffset/
 *                            feGaussianBlur/feBlend/feColorMatrix/feComposite、参数异常默认区域）
 * 6. svg2_mixed.svg          混合能力（#RGBA 颜色解析、IRI 严格校验、g 多层 opacity、
 *                            fillColor 对 fill='none' 不生效）与无限循环动效
 *
 * ========================================
 * 涉及的 Image API 与注意事项（以文档为准）
 * ========================================
 * 1. supportSvg2(enable: boolean) - API 21+，开启/关闭 SVG 标签解析能力增强，
 *    默认 false；Image 创建后不支持动态修改，切换必须重建组件节点（if/else 分支）
 * 2. onFinish(event: () => void) - API 9+，带动效 SVG 播放完成时触发；无限循环不触发
 * 3. objectFit(ImageFit.Contain) - 文档走查用例统一建议配置，保证图形完整显示
 * 4. supportSvg2(true) 时 fillColor 依赖图源 fill 属性：fill='none' 的元素不再被填充
 * 5. svg2_mixed.svg 的 animate 为 repeatCount="indefinite"，按文档 onFinish 不触发，
 *    本页将其作为“不触发”对照组观察
 */

interface Svg2FeatureCard {
  title: string;
  resName: string;
  resource: Resource;
  category: string;
  features: string[];
}

interface Svg2CapabilityGroup {
  group: string;
  items: string[];
  mappedRes: string;
}

@Entry
@Component
struct ImageSvg2FeatureWalkthroughExample {
  @State svg2On: boolean = true;
  @State mixedFinish: string = '等待触发（无限循环动效预期不触发）';
  @State staticFinish: string = '等待触发';
  @State finishCount: number = 0;
  // 六张走查卡：图源 → SVG2 能力项映射，数据全部定义在 build() 之外
  private cards: Svg2FeatureCard[] = [
    {
      title: '坐标系与仿射变换',
      resName: 'svg2_coordinates.svg',
      resource: $r('app.media.svg2_coordinates'),
      category: '仿射变换能力扩展 + viewBox 解析扩展',
      features: [
        'viewBox + preserveAspectRatio 对齐/缩放规则可配置',
        'transform-origin 全局中心点生效（false 时固定以视口左上角为原点）',
        'rotate 支持局部中心点，matrix 矩阵转换',
        'transform 非法值/非法参数个数：true 时整体不处理，false 时部分容错',
        'clipPath 路径内与组合场景支持仿射变换'
      ]
    },
    {
      title: '线性渐变',
      resName: 'svg2_gradient_linear.svg',
      resource: $r('app.media.svg2_gradient_linear'),
      category: '解析能力扩展：渐变单元 + 显示效果扩展：线性渐变',
      features: [
        'gradientUnits 支持 userSpaceOnUse（绝对坐标）与 objectBoundingBox（包围盒比例）',
        'linearGradient 支持平移与缩放效果',
        'stop-color 十六进制颜色按 #RGBA 解析（false 时按 #ARGB）'
      ]
    },
    {
      title: '径向渐变',
      resName: 'svg2_gradient_radial.svg',
      resource: $r('app.media.svg2_gradient_radial'),
      category: '解析能力扩展：渐变单元 + 显示效果扩展：径向渐变',
      features: [
        'radialGradient 的 gradientUnits 两种坐标基准解析',
        'cx/cy/r 之外支持 fx/fy 焦点偏移的平移缩放',
        '渐变单元异常参数按默认效果处理'
      ]
    },
    {
      title: '遮罩与图案',
      resName: 'svg2_mask_pattern.svg',
      resource: $r('app.media.svg2_mask_pattern'),
      category: '解析能力扩展：遮罩/图案单元 + 显示效果扩展：pattern',
      features: [
        'maskUnits/maskContentUnits 支持 objectBoundingBox 比例坐标',
        'patternUnits/patternContentUnits 支持 objectBoundingBox 比例坐标',
        'pattern 支持重复平铺；x/y 非 0 偏移时完整显示平移后图案',
        'mask 参数类型错误时取默认区域 {-10%, -10%, 120%, 120%}（false 时取 0）'
      ]
    },
    {
      title: '滤镜与原语',
      resName: 'svg2_filter_complex.svg',
      resource: $r('app.media.svg2_filter_complex'),
      category: '解析能力扩展：滤镜单元/原语单元',
      features: [
        'filterUnits/primitiveUnits 支持 objectBoundingBox',
        '支持原语 feFlood/feOffset/feGaussianBlur/feBlend/feColorMatrix/feComposite',
        'stdDeviation 等参数在比例基准下按包围盒宽高换算',
        'filter 参数类型错误时取默认区域 {-10%, -10%, 120%, 120%}'
      ]
    },
    {
      title: '混合能力与动效',
      resName: 'svg2_mixed.svg',
      resource: $r('app.media.svg2_mixed'),
      category: '易用性提升 + 显示效果扩展 + onFinish',
      features: [
        '8 位十六进制颜色按 #RGBA 解析（4 位同理），符合 SVG 标准',
        'filter/clip-path/mask 引用 IRI 类型不匹配时效果不生效',
        'g 分组 opacity 对多层子元素整体生效',
        'fillColor 对 fill=\'none\' 的元素不再填充',
        'animate 无限循环动效：onFinish 不触发（文档行为）'
      ]
    }
  ];
  // 文档《SVG标签解析能力增强》四大能力分组 → 图源映射速览
  private capabilityGroups: Svg2CapabilityGroup[] = [
    {
      group: '易用性提升',
      items: [
        '颜色默认解析 #ARGB → #RGBA',
        'URL 类型(IRI)严格校验',
        'colorFilter 生效范围扩大至全部元素',
        'fillColor 不再填充 fill=none 元素'
      ],
      mappedRes: 'svg2_mixed.svg'
    },
    {
      group: '仿射变换能力扩展',
      items: [
        'transform-origin 全局中心点可配置',
        'rotate 局部中心点 / matrix 矩阵转换',
        '非法值校验（参数非法则整体不处理）',
        '裁剪路径内与组合场景仿射变换'
      ],
      mappedRes: 'svg2_coordinates.svg'
    },
    {
      group: '解析能力扩展',
      items: [
        'viewBox 对齐与缩放规则可配置',
        '渐变单元 / 裁剪路径单元解析',
        '遮罩单元与图案单元解析',
        '滤镜单元与原语单元解析'
      ],
      mappedRes: 'svg2_gradient_*.svg / svg2_mask_pattern.svg / svg2_filter_complex.svg'
    },
    {
      group: '显示效果扩展',
      items: [
        'g 分组 opacity 对多层子元素生效',
        'g 标签内 clip-path 规则增强',
        'pattern 平铺与偏移值处理增强',
        'mask/filter 参数异常默认效果变更'
      ],
      mappedRes: 'svg2_mixed.svg 等全部六图'
    }
  ];

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('SVG2 能力逐个走查示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        this.HeroCard()
        this.FinishObserveCard()
        this.CapabilityMapCard()
        this.UsabilityCard()

        ForEach(
          this.cards,
          (card: Svg2FeatureCard, index: number) => {
            this.WalkthroughCard(card, index)
          },
          (card: Svg2FeatureCard) => card.resName
        )

        Text('supportSvg2 自 API version 21 起可用，默认 false；Image 创建后不支持动态修改该属性，本页顶部开关通过 if/else 分支重建组件实现切换。')
          .fontSize(12)
          .fontColor(Color.Blue)
          .margin({ top: 10, bottom: 20 })
      }
      .width('100%')
      .padding({ left: 16, right: 16 })
    }
    .width('100%')
    .height('100%')
    .backgroundColor(Color.White)
  }

  @Builder
  HeroCard(): void {
    Column({ space: 8 }) {
      Text('全局开关 · supportSvg2 (API 21+)').fontSize(16).fontColor(Color.Gray)
      // 文档约束：Image 创建后不支持动态修改 supportSvg2，
      // 因此用 if/else 切换分支强制重建 Image 节点，而不是在同一节点上改值。
      if (this.svg2On) {
        Image($r('app.media.svg2_mixed'))
          .width(160)
          .height(160)
          .objectFit(ImageFit.Contain)
          .supportSvg2(true)
          .borderWidth(1)
      } else {
        Image($r('app.media.svg2_mixed'))
          .width(160)
          .height(160)
          .objectFit(ImageFit.Contain)
          .supportSvg2(false)
          .borderWidth(1)
      }
      Row({ space: 12 }) {
        Button(this.svg2On ? '切换为 supportSvg2(false)' : '切换为 supportSvg2(true)')
          .fontSize(13)
          .onClick(() => {
            this.svg2On = !this.svg2On;
          })
        Text('当前: ' + (this.svg2On ? 'true（增强解析）' : 'false（原有解析）'))
          .fontSize(14)
          .fontColor(this.svg2On ? Color.Green : Color.Orange)
      }
      .width('100%')
      .justifyContent(FlexAlign.Center)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  FinishObserveCard(): void {
    Column({ space: 8 }) {
      Text('onFinish 动效结束观察 (API 9+)').fontSize(16).fontColor(Color.Gray)
      Row({ space: 16 }) {
        Column({ space: 4 }) {
          Image($r('app.media.testSvg'))
            .width(90)
            .height(90)
            .objectFit(ImageFit.Contain)
            .borderWidth(1)
            .onFinish(() => {
              this.staticFinish = 'testSvg 动效已播放完成';
              this.finishCount += 1;
            })
          Text('testSvg.svg').fontSize(11).fontColor(Color.Gray)
          Text(this.staticFinish).fontSize(11).fontColor(Color.Black)
        }
        .layoutWeight(1)
        .alignItems(HorizontalAlign.Center)

        Column({ space: 4 }) {
          Image($r('app.media.svg2_mixed'))
            .width(90)
            .height(90)
            .objectFit(ImageFit.Contain)
            .supportSvg2(true)
            .borderWidth(1)
            .onFinish(() => {
              this.mixedFinish = 'svg2_mixed 动效完成（非无限循环时才会触发）';
              this.finishCount += 1;
            })
          Text('svg2_mixed.svg').fontSize(11).fontColor(Color.Gray)
          Text(this.mixedFinish).fontSize(11).fontColor(Color.Black)
        }
        .layoutWeight(1)
        .alignItems(HorizontalAlign.Center)
      }
      .width('100%')

      Row({ space: 12 }) {
        Text('触发次数: ' + this.finishCount).fontSize(13)
        Button('重置状态')
          .fontSize(13)
          .onClick(() => {
            this.staticFinish = '等待触发';
            this.mixedFinish = '等待触发（无限循环动效预期不触发）';
            this.finishCount = 0;
          })
      }
      .width('100%')
      .justifyContent(FlexAlign.Center)
      Text('文档说明：仅支持 SVG 格式图片；动效为无限循环时不会触发 onFinish。svg2_mixed 的 animate 为 repeatCount="indefinite"，作为“不触发”对照组。')
        .fontSize(12)
        .fontColor(Color.Blue)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  CapabilityMapCard(): void {
    Column({ space: 8 }) {
      Text('四大能力分组 → 图源映射速览').fontSize(16).fontColor(Color.Gray)
      ForEach(
        this.capabilityGroups,
        (group: Svg2CapabilityGroup) => {
          Column({ space: 4 }) {
            Text(group.group)
              .fontSize(13)
              .fontWeight(FontWeight.Medium)
              .fontColor('#007DFF')
            ForEach(
              group.items,
              (item: string) => {
                Text('- ' + item)
                  .fontSize(12)
                  .fontColor(Color.Black)
                  .width('100%')
              },
              (item: string) => item
            )
            Text('主要对应图源: ' + group.mappedRes)
              .fontSize(11)
              .fontColor(Color.Gray)
          }
          .width('100%')
          .padding(10)
          .backgroundColor('#FFFFFF')
          .borderRadius(8)
        },
        (group: Svg2CapabilityGroup) => group.group
      )
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  UsabilityCard(): void {
    Column({ space: 8 }) {
      Text('易用性提升专项 · fillColor / colorFilter 生效范围').fontSize(16).fontColor(Color.Gray)
      Row({ space: 12 }) {
        Column({ space: 4 }) {
          Text('true: fill=none 不填充')
            .fontSize(11)
            .fontColor(Color.Green)
          Image($r('app.media.svg'))
            .width(100)
            .height(100)
            .objectFit(ImageFit.Contain)
            .supportSvg2(true)
            .fillColor('blue')
            .borderWidth(1)
        }
        .layoutWeight(1)
        .alignItems(HorizontalAlign.Center)

        Column({ space: 4 }) {
          Text('false: 替换全部填充')
            .fontSize(11)
            .fontColor(Color.Orange)
          Image($r('app.media.svg'))
            .width(100)
            .height(100)
            .objectFit(ImageFit.Contain)
            .supportSvg2(false)
            .fillColor('blue')
            .borderWidth(1)
        }
        .layoutWeight(1)
        .alignItems(HorizontalAlign.Center)

        Column({ space: 4 }) {
          Text('true: colorFilter 全局生效')
            .fontSize(11)
            .fontColor(Color.Green)
          Image($r('app.media.icon'))
            .width(100)
            .height(100)
            .objectFit(ImageFit.Contain)
            .supportSvg2(true)
            .colorFilter([
              0.6, 0, 0, 0, 0,
              0.2, 0.8, 0, 0, 0,
              0.2, 0.2, 1.2, 0, 0,
              0, 0, 0, 1, 0
            ])
            .borderWidth(1)
        }
        .layoutWeight(1)
        .alignItems(HorizontalAlign.Center)
      }
      .width('100%')

      Text('文档依据：supportSvg2(true) 时 fillColor 依赖图源 fill 属性（svg.svg 含 fill="none" 节点，不再被填充）；colorFilter 从仅对 stroke 生效调整为对 SVG 图源全部元素生效。')
        .fontSize(12)
        .fontColor(Color.Blue)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  WalkthroughCard(card: Svg2FeatureCard, index: number): void {
    Column({ space: 8 }) {
      Row({ space: 8 }) {
        Text((index + 1).toString() + '. ' + card.title)
          .fontSize(16)
          .fontWeight(FontWeight.Medium)
        Text(card.category)
          .fontSize(11)
          .fontColor(Color.White)
          .padding({ left: 6, right: 6, top: 2, bottom: 2 })
          .backgroundColor('#007DFF')
          .borderRadius(6)
      }
      .width('100%')

      Row({ space: 12 }) {
        Column({ space: 4 }) {
          Text('supportSvg2(true)')
            .fontSize(11)
            .fontColor(Color.Green)
          Image(card.resource)
            .width(110)
            .height(110)
            .objectFit(ImageFit.Contain)
            .supportSvg2(true)
            .borderWidth(1)
            .onFinish(() => {
              this.finishCount += 1;
            })
        }
        .layoutWeight(1)
        .alignItems(HorizontalAlign.Center)

        Column({ space: 4 }) {
          Text('supportSvg2(false)')
            .fontSize(11)
            .fontColor(Color.Orange)
          Image(card.resource)
            .width(110)
            .height(110)
            .objectFit(ImageFit.Contain)
            .supportSvg2(false)
            .borderWidth(1)
            .onFinish(() => {
              this.finishCount += 1;
            })
        }
        .layoutWeight(1)
        .alignItems(HorizontalAlign.Center)
      }
      .width('100%')

      ForEach(
        card.features,
        (feature: string) => {
          Row({ space: 6 }) {
            Text('●')
              .fontSize(10)
              .fontColor('#007DFF')
            Text(feature)
              .fontSize(12)
              .fontColor(Color.Black)
              .layoutWeight(1)
          }
          .width('100%')
        },
        (feature: string) => feature
      )
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }
}
