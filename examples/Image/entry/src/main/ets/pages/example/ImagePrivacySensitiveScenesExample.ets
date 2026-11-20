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
 * Image privacySensitive<12+> 敏感隐私场景示例
 *
 * 与既有页面的差异定位:
 * - ImageExample088 以单图开关与文字说明为主; 本页用 ForEach 组织多图源(3种SVG + 3种运行时PixelMap)
 *   的 true/false 成对对比卡矩阵, 并叠加父容器 blur 形成"卡片隐私预览"模拟场景.
 * - ImageComponentAttributeGapExample 仅一张 privacySensitive 开关小卡; 本页不重复该写法,
 *   聚焦"敏感信息卡"整体预览与语义说明矩阵.
 *
 * 文档依据 (ts-basic-components-image.md#privacysensitive12):
 * - privacySensitive(supported: boolean), 默认 false.
 * - 设置为 true 时, 隐私模式下图片将显示为半透明底板样式.
 * - 进入隐私模式需要卡片框架支持; 从API version 12开始支持ArkTS卡片与原子化服务.
 * - 仅可在Stage模型下使用; SystemCapability.ArkUI.ArkUI.Full.
 */
import { image } from '@kit.ImageKit';
import { BusinessError } from '@kit.BasicServicesKit';

interface PrivacyCompareCard {
  id: number;
  title: string;
  desc: string;
  usePixelMap: boolean;
  svgIndex: number;
  pixelMapIndex: number;
}

interface PrivacySemanticLine {
  id: number;
  label: string;
  detail: string;
}

interface BlurPreset {
  label: string;
  radius: number;
}

interface MixedPrivacyRow {
  id: number;
  name: string;
  info: string;
  pixelMapIndex: number;
  sensitive: boolean;
}

const SEMANTIC_LINES: PrivacySemanticLine[] = [
  {
    id: 0,
    label: '属性语义',
    detail: 'privacySensitive 标记图片为"卡片敏感隐私信息", 默认 false 表示不敏感。'
  },
  {
    id: 1,
    label: '隐私模式表现',
    detail: '设置为 true 后, 进入隐私模式时图片显示为半透明底板样式, 而非真实内容。'
  },
  {
    id: 2,
    label: '框架依赖',
    detail: '进入隐私模式需要卡片框架支持, 应用侧仅能声明敏感性, 无法自行切换隐私模式。'
  },
  {
    id: 3,
    label: '截屏防护语义',
    detail: '在非信任应用接管显示/投屏/截屏等场景下, 敏感图片以底板样式呈现, 避免隐私内容外泄。'
  },
  {
    id: 4,
    label: '卡片与原子化服务',
    detail: '从 API version 12 开始, 该接口支持在 ArkTS 卡片与原子化服务中使用。'
  },
  {
    id: 5,
    label: '模型约束',
    detail: '此接口仅可在 Stage 模型下使用, 系统能力为 SystemCapability.ArkUI.ArkUI.Full。'
  },
  {
    id: 6,
    label: '与 blur 的关系',
    detail: 'blur 是应用侧自绘模糊, privacySensitive 是系统隐私模式策略, 两者机制不同、可叠加演示。'
  },
  {
    id: 7,
    label: '逐图声明',
    detail: 'privacySensitive 作用于单个 Image 实例, 同一页面可混合敏感与非敏感图片。'
  }
];

const BLUR_PRESETS: BlurPreset[] = [
  { label: '无模糊', radius: 0 },
  { label: '轻模糊', radius: 8 },
  { label: '重模糊', radius: 20 }
];

const COMPARE_CARD_TOTAL: number = 6;

const MIXED_PRIVACY_ROWS: MixedPrivacyRow[] = [
  { id: 0, name: '身份证缩略图', info: '证件类图片默认标记为敏感', pixelMapIndex: 0, sensitive: true },
  { id: 1, name: '银行卡正面', info: '卡号区域属于隐私信息', pixelMapIndex: 1, sensitive: true },
  { id: 2, name: '健康码截图', info: '含个人健康状态, 声明敏感', pixelMapIndex: 2, sensitive: true },
  { id: 3, name: '活动海报', info: '公开素材, 无需敏感声明', pixelMapIndex: 0, sensitive: false },
  { id: 4, name: '门店招牌', info: '公共场景图, 保持普通显示', pixelMapIndex: 1, sensitive: false }
];

const MIXED_SENSITIVE_COUNT: number =
  MIXED_PRIVACY_ROWS.filter((row: MixedPrivacyRow): boolean => row.sensitive).length;

@Entry
@Component
struct ImagePrivacySensitiveScenesExample {
  @State pixelMapsReady: boolean = false;
  @State blurRadius: number = 8;
  @State privacyRightOn: boolean = true;
  @State scenePrivacyOn: boolean = true;
  @State statusText: string = '正在准备运行时PixelMap图源…';
  @State cardConfigs: PrivacyCompareCard[] = [];
  private pixelMaps: image.PixelMap[] = [];
  private svgResources: Resource[] = [
    $r('app.media.icon'),
    $r('app.media.heart'),
    $r('app.media.star')
  ];

  async aboutToAppear(): Promise<void> {
    const colors: number[][] = [
      [220, 60, 60],
      [60, 160, 90],
      [70, 110, 220]
    ];
    const created: image.PixelMap[] = [];
    for (let i = 0; i < colors.length; i++) {
      const pixelMap: image.PixelMap | undefined =
        await this.createSolidPixelMap(colors[i][0], colors[i][1], colors[i][2]);
      if (pixelMap !== undefined) {
        created.push(pixelMap);
      }
    }
    this.pixelMaps = created;
    this.cardConfigs = this.buildCardConfigs();
    this.pixelMapsReady = created.length === colors.length;
    this.statusText = this.pixelMapsReady
      ? '6张对比卡就绪: 3组SVG图源 + 3组PixelMap图源'
      : '部分PixelMap创建失败, 仅展示可用图源';
  }

  private buildCardConfigs(): PrivacyCompareCard[] {
    const configs: PrivacyCompareCard[] = [];
    const svgTitles: string[] = ['图标SVG', '爱心SVG', '星标SVG'];
    const svgDescs: string[] = [
      '矢量图标: 验证 privacySensitive 对SVG图源可正常声明',
      '矢量爱心: 与位图图源混合编排对比矩阵',
      '矢量星标: SVG动效图同样支持该属性声明'
    ];
    const pmTitles: string[] = ['纯色位图A', '纯色位图B', '纯色位图C'];
    const pmDescs: string[] = [
      '运行时PixelMap(红): 位图隐私卡常见图源形态',
      '运行时PixelMap(绿): 无文件依赖, 便于构造敏感样例',
      '运行时PixelMap(蓝): PixelMap在卡片隐私模式下同样受控'
    ];
    for (let i = 0; i < 3; i++) {
      configs.push({
        id: i,
        title: '对比卡' + (i + 1) + ' · ' + svgTitles[i],
        desc: svgDescs[i],
        usePixelMap: false,
        svgIndex: i,
        pixelMapIndex: -1
      });
    }
    for (let i = 0; i < 3; i++) {
      configs.push({
        id: 3 + i,
        title: '对比卡' + (i + 4) + ' · ' + pmTitles[i],
        desc: pmDescs[i],
        usePixelMap: true,
        svgIndex: -1,
        pixelMapIndex: i
      });
    }
    return configs;
  }

  private async createSolidPixelMap(r: number, g: number, b: number): Promise<image.PixelMap | undefined> {
    const size: number = 64;
    const buffer: ArrayBuffer = new ArrayBuffer(size * size * 4);
    const bytes: Uint8Array = new Uint8Array(buffer);
    for (let i = 0; i < size * size; i++) {
      bytes[i * 4] = r;
      bytes[i * 4 + 1] = g;
      bytes[i * 4 + 2] = b;
      bytes[i * 4 + 3] = 255;
    }
    const opts: image.InitializationOptions = {
      size: { height: size, width: size },
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      editable: false
    };
    try {
      return await image.createPixelMap(buffer, opts);
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      console.error('createSolidPixelMap failed code=' + err.code);
      return undefined;
    }
  }

  private srcOf(card: PrivacyCompareCard): image.PixelMap | Resource {
    if (card.usePixelMap) {
      return this.pixelMaps[card.pixelMapIndex];
    }
    return this.svgResources[card.svgIndex];
  }

  private blurLabel(): string {
    if (this.blurRadius === 0) {
      return '关闭';
    }
    return this.blurRadius + 'vp';
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('privacySensitive 敏感隐私场景示例(API12+)')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        Column({ space: 8 }) {
          Text('全局控制 · 隐私声明与父级模糊')
            .fontSize(16)
            .fontColor(Color.Gray)
          Text('状态: ' + this.statusText)
            .fontSize(14)
          Row({ space: 10 }) {
            Button('右侧列开启敏感')
              .onClick(() => {
                this.privacyRightOn = true;
              })
            Button('右侧列关闭敏感')
              .onClick(() => {
                this.privacyRightOn = false;
              })
          }
          Row({ space: 10 }) {
            ForEach(BLUR_PRESETS, (preset: BlurPreset) => {
              Button(preset.label)
                .backgroundColor(this.blurRadius === preset.radius ? 0x007DFF : 0xE0E0E0)
                .fontColor(this.blurRadius === preset.radius ? Color.White : 0x333333)
                .onClick(() => {
                  this.blurRadius = preset.radius;
                })
            }, (preset: BlurPreset) => preset.radius.toString())
          }
          Text('当前父容器模糊: ' + this.blurLabel() + ' · 对比卡右侧列 privacySensitive = ' +
            (this.privacyRightOn ? 'true' : 'false'))
            .fontSize(13)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Text('多图源 true/false 对比矩阵(每张卡整体套用父级模糊, 模拟隐私预览)')
          .fontSize(16)
          .fontWeight(FontWeight.Medium)

        ForEach(this.cardConfigs, (card: PrivacyCompareCard) => {
          this.PrivacyCompareCardView(card)
        }, (card: PrivacyCompareCard) => card.id.toString())

        Column({ space: 10 }) {
          Text('敏感信息卡预览场景(父容器 blur + privacySensitive)')
            .fontSize(16)
            .fontColor(Color.Gray)
          Column({ space: 8 }) {
            Row({ space: 12 }) {
              if (this.pixelMapsReady) {
                Image(this.pixelMaps[0])
                  .width(56)
                  .height(56)
                  .borderRadius(28)
                  .privacySensitive(this.scenePrivacyOn)
              }
              Column({ space: 4 }) {
                Text('账户余额卡')
                  .fontSize(15)
                  .fontWeight(FontWeight.Medium)
                Text('尾号 **** 储蓄卡')
                  .fontSize(13)
                  .fontColor(Color.Gray)
              }
              .alignItems(HorizontalAlign.Start)
              Blank()
              Text('¥ 8,888.00')
                .fontSize(16)
                .fontWeight(FontWeight.Bold)
            }
            .width('100%')
            Row({ space: 12 }) {
              if (this.pixelMapsReady) {
                Image(this.pixelMaps[1])
                  .width(56)
                  .height(56)
                  .borderRadius(28)
                  .privacySensitive(this.scenePrivacyOn)
              }
              Column({ space: 4 }) {
                Text('行程隐私卡')
                  .fontSize(15)
                  .fontWeight(FontWeight.Medium)
                Text('家 · 某小区 3 栋')
                  .fontSize(13)
                  .fontColor(Color.Gray)
              }
              .alignItems(HorizontalAlign.Start)
              Blank()
              Text('10:24 到达')
                .fontSize(14)
            }
            .width('100%')
          }
          .width('100%')
          .padding(12)
          .backgroundColor('#FFFFFF')
          .borderRadius(10)
          .blur(this.blurRadius)
          Row({ space: 10 }) {
            Button(this.scenePrivacyOn ? '场景卡关闭敏感声明' : '场景卡开启敏感声明')
              .onClick(() => {
                this.scenePrivacyOn = !this.scenePrivacyOn;
              })
            Button(this.blurRadius > 0 ? '清除场景模糊' : '叠加场景模糊')
              .onClick(() => {
                this.blurRadius = this.blurRadius > 0 ? 0 : 20;
              })
          }
          Text('说明: blur 为应用自绘预览效果; privacySensitive 仅在系统隐私模式下将图片替换为半透明底板样式')
            .fontSize(12)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Column({ space: 10 }) {
          Text('混合敏感声明清单(逐行 privacySensitive)')
            .fontSize(16)
            .fontColor(Color.Gray)
          ForEach(MIXED_PRIVACY_ROWS, (row: MixedPrivacyRow) => {
            Row({ space: 12 }) {
              if (this.pixelMapsReady) {
                Image(this.pixelMaps[row.pixelMapIndex])
                  .width(44)
                  .height(44)
                  .borderRadius(6)
                  .privacySensitive(row.sensitive)
              }
              Column({ space: 3 }) {
                Text(row.name)
                  .fontSize(14)
                  .fontWeight(FontWeight.Medium)
                Text(row.info)
                  .fontSize(12)
                  .fontColor(Color.Gray)
              }
              .alignItems(HorizontalAlign.Start)
              Blank()
              Text(row.sensitive ? '敏感' : '普通')
                .fontSize(12)
                .fontColor(row.sensitive ? 0xE36001 : Color.Green)
            }
            .width('100%')
            .padding(8)
            .backgroundColor(Color.White)
            .borderRadius(8)
          }, (row: MixedPrivacyRow) => row.id.toString())
          Text('敏感行数: ' + MIXED_SENSITIVE_COUNT + ' / ' + MIXED_PRIVACY_ROWS.length)
            .fontSize(13)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Column({ space: 8 }) {
          Text('截屏防护与非信任应用语义说明')
            .fontSize(16)
            .fontColor(Color.Gray)
          ForEach(SEMANTIC_LINES, (line: PrivacySemanticLine) => {
            Column({ space: 2 }) {
              Text(line.label)
                .fontSize(14)
                .fontWeight(FontWeight.Medium)
              Text(line.detail)
                .fontSize(13)
                .fontColor(Color.Gray)
            }
            .width('100%')
            .alignItems(HorizontalAlign.Start)
          }, (line: PrivacySemanticLine) => line.id.toString())
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Text('对比卡总数: ' + this.cardConfigs.length + ' / 计划 ' + COMPARE_CARD_TOTAL +
          ' · PixelMap就绪: ' + (this.pixelMapsReady ? '是' : '否'))
          .fontSize(13)
        Text('privacySensitive 不改变正常模式显示; 半透明底板样式仅在系统隐私模式(需卡片框架支持)下生效')
          .fontSize(12)
          .fontColor(Color.Blue)
          .margin({ bottom: 20 })
      }
      .width('100%')
      .padding({ left: 20, right: 20 })
    }
    .width('100%')
    .height('100%')
    .backgroundColor(Color.White)
  }

  @Builder
  PrivacyCompareCardView(card: PrivacyCompareCard) {
    Column({ space: 8 }) {
      Text(card.title)
        .fontSize(15)
        .fontWeight(FontWeight.Medium)
      Text(card.desc)
        .fontSize(13)
        .fontColor(Color.Gray)
      Row({ space: 16 }) {
        Column({ space: 6 }) {
          Image(this.srcOf(card))
            .width(88)
            .height(88)
            .objectFit(ImageFit.Contain)
            .privacySensitive(false)
            .borderWidth(1)
            .borderColor('#CCCCCC')
            .borderRadius(8)
          Text('privacySensitive(false)')
            .fontSize(12)
            .fontColor(Color.Gray)
        }
        Column({ space: 6 }) {
          Image(this.srcOf(card))
            .width(88)
            .height(88)
            .objectFit(ImageFit.Contain)
            .privacySensitive(this.privacyRightOn)
            .borderWidth(1)
            .borderColor('#FFB53D')
            .borderRadius(8)
          Text('privacySensitive(' + (this.privacyRightOn ? 'true' : 'false') + ')')
            .fontSize(12)
            .fontColor(this.privacyRightOn ? 0xE36001 : Color.Gray)
        }
      }
      .width('100%')
      .justifyContent(FlexAlign.SpaceAround)
      Text('卡内父级模糊半径: ' + this.blurLabel())
        .fontSize(12)
        .fontColor(Color.Gray)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
    .blur(this.blurRadius)
  }
}
