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
 * fillColor 三种形态矩阵示例 (API 7 / 15+ / 20+)
 * ========================================
 * 功能说明:
 * 按文档梳理 fillColor 三种入参形态，作用于多节点 SVG 图源（icon.svg 6 节点、
 * svg.svg 2 节点含 1 个 fill="none"），观察“替换所有可绘制元素填充颜色”的效果。
 * 与 ImageFillColorContentExample（单一 ColorContent 切换页）不同，本页做
 * “形态 × 图源”矩阵，并给出形态切换按钮与当前形态状态卡。
 * ========================================
 * 涉及的 Image API（以文档为准）与注意事项
 * ========================================
 * 1. fillColor(value: ResourceColor) - 基础形态：string('#RRGGBB')、
 *    number(0xAARRGGBB)、Color 枚举
 * 2. fillColor(color: ResourceColor | ColorContent) - API 15+，ColorContent
 *    文档仅定义 ORIGIN：重置 fillColor，效果与不设置一致
 * 3. fillColor(color: ResourceColor|ColorContent|ColorMetrics) - API 20+，
 *    ColorMetrics 支持 P3 广色域（文档说明 API 24 起支持 BT2020）
 * 注意：仅 SVG 图源生效（png 改色用 colorFilter）；异常值回退主题色；API 21 起
 * supportSvg2(true) 时 fill='none' 元素不再被填充；文档未定义“逗号分隔多色串”
 * 形态，扩展类型仅 ColorContent(ORIGIN) 与 ColorMetrics，不演示未文档化写法。
 */

import { ColorMetrics, ColorContent, ColorSpace } from '@kit.ArkUI';

type FillArg = ResourceColor | ColorContent | ColorMetrics | undefined;

interface FillOption {
  label: string;
  value: FillArg;
  display: string;
  desc: string;
}

interface FillFormMeta {
  name: string;
  since: string;
  signature: string;
  summary: string;
}

@Entry
@Component
struct ImageFillColorFormsExample {
  @State formIndex: number = 0;
  @State optionIndex: number = 0;
  @State fillValue: FillArg = undefined;
  // 三种形态的文档签名与起始版本
  private forms: FillFormMeta[] = [
    {
      name: 'ResourceColor 形态',
      since: '基础形态（API 7 起）',
      signature: 'fillColor(value: ResourceColor)',
      summary: 'string / number / Color 枚举均可，替换 SVG 全部可绘制元素填充色'
    },
    {
      name: 'ColorContent 形态',
      since: 'API 15+',
      signature: 'fillColor(color: ResourceColor | ColorContent)',
      summary: '文档定义 ColorContent.ORIGIN 用于重置填充颜色，效果与不设置一致'
    },
    {
      name: 'ColorMetrics 形态',
      since: 'API 20+',
      signature: 'fillColor(color: ResourceColor | ColorContent | ColorMetrics)',
      summary: '支持 P3 广色域颜色，可在高色域设备获得更丰富色彩表现'
    }
  ];
  // 形态一：ResourceColor 的三种取值写法
  private resourceColorOptions: FillOption[] = [
    {
      label: '字符串色值',
      value: '#FF5722',
      display: "'#FF5722'",
      desc: 'ResourceColor 中的 ColorString 写法'
    },
    {
      label: '数字色值',
      value: 0xFFFF6347,
      display: '0xFFFF6347',
      desc: 'ResourceColor 中的 number 写法（0xAARRGGBB）'
    },
    {
      label: 'Color 枚举',
      value: Color.Blue,
      display: 'Color.Blue',
      desc: 'ResourceColor 中的系统颜色枚举'
    }
  ];
  // 形态二：ColorContent（文档仅定义 ORIGIN）与不设置/undefined 对照
  private colorContentOptions: FillOption[] = [
    {
      label: 'ColorContent.ORIGIN',
      value: ColorContent.ORIGIN,
      display: 'ColorContent.ORIGIN',
      desc: 'API 15+ 重置填充颜色，效果与不设置 fillColor 一致'
    },
    {
      label: 'undefined',
      value: undefined,
      display: 'undefined',
      desc: '文档示例 18：传入 undefined 观察清除填充效果'
    }
  ];
  // 形态三：ColorMetrics P3 与 sRGB 成对色（取自文档示例 23 的色值）
  private colorMetricsOptions: FillOption[] = [
    {
      label: 'P3 红',
      value: ColorMetrics.colorWithSpace(ColorSpace.DISPLAY_P3, 0.631, 0.0392, 0.1294),
      display: 'colorWithSpace(DISPLAY_P3, 0.631, 0.0392, 0.1294)',
      desc: 'Display P3 色域红色，高色域设备上更鲜艳'
    },
    {
      label: 'sRGB 红',
      value: ColorMetrics.colorWithSpace(ColorSpace.SRGB, 0.631, 0.0392, 0.1294),
      display: 'colorWithSpace(SRGB, 0.631, 0.0392, 0.1294)',
      desc: '同参数 sRGB 红色对照'
    },
    {
      label: 'P3 绿',
      value: ColorMetrics.colorWithSpace(ColorSpace.DISPLAY_P3, 0.09, 0.662, 0.552),
      display: 'colorWithSpace(DISPLAY_P3, 0.09, 0.662, 0.552)',
      desc: 'Display P3 色域绿色'
    },
    {
      label: 'sRGB 绿',
      value: ColorMetrics.colorWithSpace(ColorSpace.SRGB, 0.09, 0.662, 0.552),
      display: 'colorWithSpace(SRGB, 0.09, 0.662, 0.552)',
      desc: '同参数 sRGB 绿色对照'
    },
    {
      label: 'P3 蓝',
      value: ColorMetrics.colorWithSpace(ColorSpace.DISPLAY_P3, 0, 0.290, 0.686),
      display: 'colorWithSpace(DISPLAY_P3, 0, 0.290, 0.686)',
      desc: 'Display P3 色域蓝色'
    },
    {
      label: 'sRGB 蓝',
      value: ColorMetrics.colorWithSpace(ColorSpace.SRGB, 0, 0.290, 0.686),
      display: 'colorWithSpace(SRGB, 0, 0.290, 0.686)',
      desc: '同参数 sRGB 蓝色对照'
    }
  ];

  private currentOptions(): FillOption[] {
    if (this.formIndex === 0) {
      return this.resourceColorOptions;
    }
    if (this.formIndex === 1) {
      return this.colorContentOptions;
    }
    return this.colorMetricsOptions;
  }

  private currentOption(): FillOption {
    const options: FillOption[] = this.currentOptions();
    const idx: number = this.optionIndex < options.length ? this.optionIndex : 0;
    return options[idx];
  }

  private selectOption(index: number): void {
    this.optionIndex = index;
    this.fillValue = this.currentOptions()[index].value;
  }

  private switchForm(index: number): void {
    this.formIndex = index;
    this.optionIndex = 0;
    this.fillValue = this.currentOptions()[0].value;
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('fillColor 三种形态矩阵示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        this.FormMatrixCard()
        this.StatusCard()
        this.MultiSourceCard()
        this.ResourceColorDetailCard()
        this.ColorContentDetailCard()
        this.ColorMetricsDetailCard()
        this.ConstraintCard()

        Text('fillColor 仅对 SVG 图源生效；如需对 png 位图改色，文档建议使用 colorFilter。三种形态可在同一应用上按需混用，低版本 SDK 请勿传入高版本类型。')
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
  FormMatrixCard(): void {
    Column({ space: 8 }) {
      Text('形态总览与切换').fontSize(16).fontColor(Color.Gray)
      ForEach(
        this.forms,
        (form: FillFormMeta, index: number) => {
          Column({ space: 4 }) {
            Row({ space: 8 }) {
              Text(form.name)
                .fontSize(13)
                .fontWeight(FontWeight.Medium)
                .fontColor(this.formIndex === index ? Color.White : '#007DFF')
                .padding({ left: 8, right: 8, top: 3, bottom: 3 })
                .backgroundColor(this.formIndex === index ? '#007DFF' : '#E6F0FF')
                .borderRadius(6)
              Text(form.since)
                .fontSize(11)
                .fontColor(Color.Gray)
            }
            .width('100%')
            Text(form.signature)
              .fontSize(11)
              .fontColor(Color.Black)
              .fontFamily('monospace')
            Text(form.summary)
              .fontSize(11)
              .fontColor(Color.Gray)
          }
          .width('100%')
          .alignItems(HorizontalAlign.Start)
          .padding(10)
          .backgroundColor('#FFFFFF')
          .borderRadius(8)
          .onClick(() => {
            this.switchForm(index);
          })
        },
        (form: FillFormMeta) => form.name
      )
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  StatusCard(): void {
    Column({ space: 8 }) {
      Text('当前形态状态卡').fontSize(16).fontColor(Color.Gray)
      Text('形态: ' + this.forms[this.formIndex].name + ' · ' + this.forms[this.formIndex].since)
        .fontSize(13)
        .fontColor(Color.Black)
      Text('入参: ' + this.currentOption().display)
        .fontSize(12)
        .fontColor('#007DFF')
        .fontFamily('monospace')
      Text('说明: ' + this.currentOption().desc)
        .fontSize(12)
        .fontColor(Color.Gray)
      Row({ space: 10 }) {
        Button('上一个')
          .fontSize(12)
          .onClick(() => {
            const len: number = this.currentOptions().length;
            this.selectOption((this.optionIndex + len - 1) % len);
          })
        Button('下一个')
          .fontSize(12)
          .onClick(() => {
            const len: number = this.currentOptions().length;
            this.selectOption((this.optionIndex + 1) % len);
          })
        Button('下一个形态')
          .fontSize(12)
          .onClick(() => {
            this.switchForm((this.formIndex + 1) % this.forms.length);
          })
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
  MultiSourceCard(): void {
    Column({ space: 8 }) {
      Text('形态 × 多节点 SVG 图源').fontSize(16).fontColor(Color.Gray)
      Row({ space: 12 }) {
        Column({ space: 4 }) {
          Image($r('app.media.icon'))
            .width(110)
            .height(110)
            .objectFit(ImageFit.Contain)
            .fillColor(this.fillValue)
            .borderWidth(1)
          Text('icon.svg · 6 个可绘制节点')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .layoutWeight(1)
        .alignItems(HorizontalAlign.Center)

        Column({ space: 4 }) {
          Image($r('app.media.svg'))
            .width(110)
            .height(110)
            .objectFit(ImageFit.Contain)
            .fillColor(this.fillValue)
            .borderWidth(1)
          Text('svg.svg · 2 节点（含 fill="none"）')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .layoutWeight(1)
        .alignItems(HorizontalAlign.Center)

        Column({ space: 4 }) {
          Image($r('app.media.heart'))
            .width(110)
            .height(110)
            .objectFit(ImageFit.Contain)
            .fillColor(this.fillValue)
            .borderWidth(1)
          Text('heart.svg · 单节点')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .layoutWeight(1)
        .alignItems(HorizontalAlign.Center)
      }
      .width('100%')
      Text('文档语义：fillColor 设置后会替换 SVG 图片中所有可绘制元素的填充颜色；多节点图源整体同步变色。')
        .fontSize(12)
        .fontColor(Color.Blue)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  ResourceColorDetailCard(): void {
    Column({ space: 8 }) {
      Text('形态一 · ResourceColor（string / number / Color）').fontSize(16).fontColor(Color.Gray)
      Flex({ wrap: FlexWrap.Wrap }) {
        ForEach(
          this.resourceColorOptions,
          (option: FillOption, index: number) => {
            Button(option.label)
              .fontSize(11)
              .backgroundColor(this.formIndex === 0 && this.optionIndex === index ? '#007DFF' : '#E0E0E0')
              .fontColor(this.formIndex === 0 && this.optionIndex === index ? Color.White : Color.Black)
              .height(30)
              .margin({ right: 6, bottom: 6 })
              .onClick(() => {
                this.switchForm(0);
                this.selectOption(index);
              })
          },
          (option: FillOption) => option.label
        )
      }
      .width('100%')
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  ColorContentDetailCard(): void {
    Column({ space: 8 }) {
      Text('形态二 · ColorContent (API 15+) 对照（文档示例 18 布局）').fontSize(16).fontColor(Color.Gray)
      Row({ space: 8 }) {
        Column({ space: 4 }) {
          Image($r('app.media.svg'))
            .width(72)
            .height(72)
            .objectFit(ImageFit.Contain)
            .borderWidth(1)
          Text('不设置').fontSize(10).fontColor(Color.Gray)
        }
        .layoutWeight(1)

        Column({ space: 4 }) {
          Image($r('app.media.svg'))
            .width(72)
            .height(72)
            .objectFit(ImageFit.Contain)
            .fillColor(ColorContent.ORIGIN)
            .borderWidth(1)
          Text('ORIGIN').fontSize(10).fontColor(Color.Gray)
        }
        .layoutWeight(1)

        Column({ space: 4 }) {
          Image($r('app.media.svg'))
            .width(72)
            .height(72)
            .objectFit(ImageFit.Contain)
            .fillColor(Color.Blue)
            .borderWidth(1)
          Text('Color.Blue').fontSize(10).fontColor(Color.Gray)
        }
        .layoutWeight(1)

        Column({ space: 4 }) {
          Image($r('app.media.svg'))
            .width(72)
            .height(72)
            .objectFit(ImageFit.Contain)
            .fillColor(undefined)
            .borderWidth(1)
          Text('undefined').fontSize(10).fontColor(Color.Gray)
        }
        .layoutWeight(1)
      }
      .width('100%')
      .alignItems(VerticalAlign.Top)
      Text('文档仅定义 ColorContent.ORIGIN 一个成员：重置 fillColor，效果与不设置一致。')
        .fontSize(12)
        .fontColor(Color.Blue)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  ColorMetricsDetailCard(): void {
    Column({ space: 8 }) {
      Text('形态三 · ColorMetrics (API 20+) P3 与 sRGB 成对对照').fontSize(16).fontColor(Color.Gray)
      ForEach(
        this.colorMetricsOptions,
        (option: FillOption, index: number) => {
          Row({ space: 8 }) {
            Image($r('app.media.star'))
              .width(48)
              .height(48)
              .objectFit(ImageFit.Contain)
              .fillColor(option.value)
              .borderWidth(1)
            Column({ space: 2 }) {
              Text(option.label + ' · ' + option.desc)
                .fontSize(12)
                .fontColor(Color.Black)
              Text(option.display)
                .fontSize(10)
                .fontColor(Color.Gray)
                .fontFamily('monospace')
            }
            .layoutWeight(1)
            .alignItems(HorizontalAlign.Start)
            Button('应用')
              .fontSize(10)
              .backgroundColor(this.formIndex === 2 && this.optionIndex === index ? '#007DFF' : '#E0E0E0')
              .onClick(() => {
                this.switchForm(2);
                this.selectOption(index);
              })
          }
          .width('100%')
          .padding(6)
          .backgroundColor('#FFFFFF')
          .borderRadius(8)
        },
        (option: FillOption) => option.label
      )
      Text('色值取自文档示例 23；P3 需设备支持高色域，普通屏上 P3 与 sRGB 差异可能不可见。')
        .fontSize(12)
        .fontColor(Color.Blue)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  ConstraintCard(): void {
    Column({ space: 8 }) {
      Text('约束与互斥关系卡').fontSize(16).fontColor(Color.Gray)
      Column({ space: 6 }) {
        Text('● 仅对 SVG 图源生效：设置后替换 SVG 所有可绘制元素填充色；png 等位图改色请用 colorFilter；AnimatedDrawableDescriptor 图源不生效。')
          .fontSize(12)
        Text('● API 21 起 supportSvg2(true)：fill 属性为 none 的元素不再被 fillColor 填充；supportSvg2(false) 保持替换全部元素；异常入参回退主题色。')
          .fontSize(12)
        Text('● 与 renderMode 的关系：renderMode 不支持 SVG 图源、fillColor 仅对 SVG 生效，两者在图源维度互斥（详见 renderMode 约束页）。')
          .fontSize(12)
      }
      .width('100%')
      .alignItems(HorizontalAlign.Start)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }
}
