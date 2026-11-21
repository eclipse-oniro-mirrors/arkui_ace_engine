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
 * enableAnalyzer<11+> + ImageAIOptions/ImageAnalyzerType/ImageAnalyzerController<12+> 分析选项示例
 *
 * 与既有页面的差异定位:
 * - ImageAnalyzerExample 仅有伪结果按钮, 未调用真实分析器接口; ImageAIOptionsExample 是单图 + 类型单选面板;
 *   ImageComponentAttributeGapExample 只有一次 getImageAnalyzerSupportTypes 点击.
 * - 本页首次给出 5 卡类型矩阵(仅SUBJECT/仅TEXT/仅OBJECT_LOOKUP/全类型/未配置types走默认),
 *   每卡独立绑定 ImageAnalyzerController 做支持类型探测, 探测结果带时间戳入日志,
 *   并叠加 enableAnalyzer true/false 并排对比与文档约束速查.
 *
 * 文档依据:
 * - ts-basic-components-image.md#enableanalyzer11: true 开启AI分析, 默认 false; 依赖设备能力, 需 ohos.permission.INTERNET.
 * - ts-image-common.md#imageaioptions12: { types?: ImageAnalyzerType[], aiController?: ImageAnalyzerController }.
 * - ts-image-common.md#imageanalyzertype12: SUBJECT(0)/TEXT(1)/OBJECT_LOOKUP(2); 未设置默认主体+文字识别.
 * - ts-image-common.md#imageanalyzercontroller12: getImageAnalyzerSupportTypes(): ImageAnalyzerType[],
 *   未绑定组件前返回空数组.
 */
import { image } from '@kit.ImageKit';
import { BusinessError } from '@kit.BasicServicesKit';

interface AnalyzerCardMeta {
  id: number;
  title: string;
  typesDesc: string;
  hint: string;
}

interface AnalyzerProbeEntry {
  id: number;
  time: string;
  cardTitle: string;
  typeNames: string;
  count: number;
}

interface AnalyzerConstraintLine {
  id: number;
  title: string;
  detail: string;
}

const ANALYZER_CARD_METAS: AnalyzerCardMeta[] = [
  {
    id: 0,
    title: '仅主体识别 types=[SUBJECT]',
    typesDesc: 'ImageAnalyzerType.SUBJECT(0)',
    hint: '只请求主体分割能力, 探测结果取决于设备侧实现'
  },
  {
    id: 1,
    title: '仅文字识别 types=[TEXT]',
    typesDesc: 'ImageAnalyzerType.TEXT(1)',
    hint: '只请求OCR能力, 适合截图类位图'
  },
  {
    id: 2,
    title: '仅对象查找 types=[OBJECT_LOOKUP]',
    typesDesc: 'ImageAnalyzerType.OBJECT_LOOKUP(2)',
    hint: '只请求对象检索能力'
  },
  {
    id: 3,
    title: '全类型 types=[SUBJECT,TEXT,OBJECT_LOOKUP]',
    typesDesc: '三种类型全部声明',
    hint: 'types 优先级高于 ImageAnalyzerConfig 中的 types'
  },
  {
    id: 4,
    title: '未配置 types(默认)',
    typesDesc: '仅绑定 aiController, 不传 types',
    hint: '文档: 未设置时默认开启主体识别和文字识别'
  }
];

const CONSTRAINT_LINES: AnalyzerConstraintLine[] = [
  {
    id: 0,
    title: '图源要求',
    detail: '分析图像要求静态非矢量图, svg/gif 不支持; 支持 PixelMap, 目前仅支持 RGBA_8888.'
  },
  {
    id: 1,
    title: '权限要求',
    detail: '需要配置 ohos.permission.INTERNET, 且特性依赖设备能力.'
  },
  {
    id: 2,
    title: '与 overlay 互斥',
    detail: '不能和 overlay 属性同时使用, 两者同设时 overlay 的 CustomBuilder 失效.'
  },
  {
    id: 3,
    title: '遮挡与占位',
    detail: 'obscured 隐私遮罩打开时不支持分析; alt 占位图不参与分析.'
  },
  {
    id: 4,
    title: '重复与裁剪',
    detail: 'objectRepeat 仅 NoRepeat 时支持; clip/borderRadius/renderMode 蒙层等仍基于完整原始图像分析.'
  },
  {
    id: 5,
    title: '复制无影响',
    detail: 'copyOption 属性不影响AI分析功能.'
  },
  {
    id: 6,
    title: '探测时机',
    detail: 'getImageAnalyzerSupportTypes 需先经 aiController 绑定组件, 否则返回空数组.'
  },
  {
    id: 7,
    title: '接口版本',
    detail: 'enableAnalyzer 为 API11+, ImageAIOptions/ImageAnalyzerType/ImageAnalyzerController 为 API12+.'
  }
];

const MAX_PROBE_LOG: number = 12;

@Entry
@Component
struct ImageAnalyzerOptionsExample {
  @State analyzerPixelMap: image.PixelMap | undefined = undefined;
  @State analyzerOn: boolean = true;
  @State createStatus: string = '等待创建PixelMap';
  @State cardStatusList: string[] = ['未探测', '未探测', '未探测', '未探测', '未探测'];
  @State probeEntries: AnalyzerProbeEntry[] = [];
  private controllerList: ImageAnalyzerController[] = [
    new ImageAnalyzerController(),
    new ImageAnalyzerController(),
    new ImageAnalyzerController(),
    new ImageAnalyzerController(),
    new ImageAnalyzerController()
  ];
  private optionsList: ImageAIOptions[] = [];
  private unboundController: ImageAnalyzerController = new ImageAnalyzerController();
  @State unboundProbeText: string = '未探测';
  private probeSeq: number = 0;

  async aboutToAppear(): Promise<void> {
    this.optionsList = this.buildOptionsList();
    const size: number = 120;
    const buffer: ArrayBuffer = new ArrayBuffer(size * size * 4);
    const bytes: Uint8Array = new Uint8Array(buffer);
    for (let y = 0; y < size; y++) {
      for (let x = 0; x < size; x++) {
        const offset: number = (y * size + x) * 4;
        const stripe: number = ((x / 12) | 0) % 2;
        bytes[offset] = stripe === 0 ? 240 : 40;
        bytes[offset + 1] = 90;
        bytes[offset + 2] = stripe === 0 ? 60 : 200;
        bytes[offset + 3] = 255;
      }
    }
    const opts: image.InitializationOptions = {
      size: { height: size, width: size },
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      editable: false
    };
    try {
      this.analyzerPixelMap = await image.createPixelMap(buffer, opts);
      this.createStatus = 'PixelMap创建成功(RGBA_8888, 满足分析图源要求)';
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      this.createStatus = 'PixelMap创建失败: code=' + err.code;
    }
  }

  private buildOptionsList(): ImageAIOptions[] {
    const matrix: ImageAnalyzerType[][] = [
      [ImageAnalyzerType.SUBJECT],
      [ImageAnalyzerType.TEXT],
      [ImageAnalyzerType.OBJECT_LOOKUP],
      [ImageAnalyzerType.SUBJECT, ImageAnalyzerType.TEXT, ImageAnalyzerType.OBJECT_LOOKUP],
      []
    ];
    const options: ImageAIOptions[] = [];
    for (let i = 0; i < this.controllerList.length; i++) {
      if (i === 4) {
        const defaultOptions: ImageAIOptions = {
          aiController: this.controllerList[i]
        };
        options.push(defaultOptions);
      } else {
        const typedOptions: ImageAIOptions = {
          types: matrix[i],
          aiController: this.controllerList[i]
        };
        options.push(typedOptions);
      }
    }
    return options;
  }

  private typeNameOf(type: ImageAnalyzerType): string {
    if (type === ImageAnalyzerType.SUBJECT) {
      return 'SUBJECT';
    }
    if (type === ImageAnalyzerType.TEXT) {
      return 'TEXT';
    }
    if (type === ImageAnalyzerType.OBJECT_LOOKUP) {
      return 'OBJECT_LOOKUP';
    }
    return 'UNKNOWN(' + type + ')';
  }

  private probeCard(meta: AnalyzerCardMeta): void {
    const types: ImageAnalyzerType[] = this.controllerList[meta.id].getImageAnalyzerSupportTypes();
    const names: string[] = types.map((type: ImageAnalyzerType): string => this.typeNameOf(type));
    const joined: string = names.length === 0 ? '(空数组: 未绑定或设备不支持)' : names.join(' / ');
    const nextStatus: string[] = this.cardStatusList.slice();
    nextStatus[meta.id] = joined;
    this.cardStatusList = nextStatus;
    this.probeSeq += 1;
    const entry: AnalyzerProbeEntry = {
      id: this.probeSeq,
      time: new Date().toLocaleTimeString(),
      cardTitle: meta.title,
      typeNames: joined,
      count: types.length
    };
    const nextEntries: AnalyzerProbeEntry[] = [entry].concat(this.probeEntries);
    if (nextEntries.length > MAX_PROBE_LOG) {
      nextEntries.pop();
    }
    this.probeEntries = nextEntries;
  }

  private probeUnbound(): void {
    const types: ImageAnalyzerType[] = this.unboundController.getImageAnalyzerSupportTypes();
    const empty: boolean = types.length === 0;
    this.unboundProbeText = empty
      ? '返回空数组(与文档一致: 未绑定组件前无支持类型)'
      : '意外返回 ' + types.length + ' 项';
    this.probeSeq += 1;
    const entry: AnalyzerProbeEntry = {
      id: this.probeSeq,
      time: new Date().toLocaleTimeString(),
      cardTitle: '未绑定控制器对照',
      typeNames: this.unboundProbeText,
      count: types.length
    };
    const nextEntries: AnalyzerProbeEntry[] = [entry].concat(this.probeEntries);
    if (nextEntries.length > MAX_PROBE_LOG) {
      nextEntries.pop();
    }
    this.probeEntries = nextEntries;
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('enableAnalyzer 与 AI 分析选项示例(API11+/12+)')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        Column({ space: 8 }) {
          Text('分析开关与图源状态')
            .fontSize(16)
            .fontColor(Color.Gray)
          Text('状态: ' + this.createStatus)
            .fontSize(13)
          Row({ space: 10 }) {
            Button(this.analyzerOn ? 'enableAnalyzer(true)' : 'enableAnalyzer(false)')
              .backgroundColor(this.analyzerOn ? 0x007DFF : 0xE0E0E0)
              .fontColor(this.analyzerOn ? Color.White : 0x333333)
              .onClick(() => {
                this.analyzerOn = !this.analyzerOn;
              })
            Text('开关将作用于下方全部分析卡')
              .fontSize(12)
              .fontColor(Color.Gray)
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Column({ space: 10 }) {
          Text('enableAnalyzer true / false 并排对比')
            .fontSize(16)
            .fontColor(Color.Gray)
          Row({ space: 16 }) {
            Column({ space: 6 }) {
              if (this.analyzerPixelMap !== undefined) {
                Image(this.analyzerPixelMap, this.optionsList[3])
                  .width(110)
                  .height(110)
                  .enableAnalyzer(true)
                  .borderWidth(1)
                  .borderColor('#007DFF')
                  .borderRadius(8)
              }
              Text('enableAnalyzer(true)')
                .fontSize(12)
                .fontColor(Color.Blue)
            }
            Column({ space: 6 }) {
              if (this.analyzerPixelMap !== undefined) {
                Image(this.analyzerPixelMap, this.optionsList[3])
                  .width(110)
                  .height(110)
                  .enableAnalyzer(false)
                  .borderWidth(1)
                  .borderColor('#CCCCCC')
                  .borderRadius(8)
              }
              Text('enableAnalyzer(false) 默认值')
                .fontSize(12)
                .fontColor(Color.Gray)
            }
          }
          .width('100%')
          .justifyContent(FlexAlign.SpaceAround)
          Text('两卡共用同一 PixelMap 与同一份 types 全量 ImageAIOptions, 仅开关不同')
            .fontSize(12)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Text('ImageAIOptions 类型矩阵(每卡独立 Controller 探测)')
          .fontSize(16)
          .fontWeight(FontWeight.Medium)

        ForEach(ANALYZER_CARD_METAS, (meta: AnalyzerCardMeta) => {
          this.AnalyzerMatrixCard(meta)
        }, (meta: AnalyzerCardMeta) => meta.id.toString())

        Column({ space: 8 }) {
          Text('绑定时机对照(未绑定 aiController 的控制器)')
            .fontSize(16)
            .fontColor(Color.Gray)
          Text('文档: getImageAnalyzerSupportTypes 调用前需先通过组件的 aiController 属性绑定, 否则返回空数组')
            .fontSize(12)
            .fontColor(Color.Gray)
          Row({ space: 10 }) {
            Button('探测未绑定控制器')
              .onClick(() => {
                this.probeUnbound();
              })
            Text(this.unboundProbeText)
              .fontSize(13)
              .fontColor(0x666666)
              .layoutWeight(1)
          }
          .width('100%')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Column({ space: 6 }) {
          Text('支持类型探测日志(最近' + MAX_PROBE_LOG + '次)')
            .fontSize(16)
            .fontColor(Color.Gray)
          ForEach(this.probeEntries, (entry: AnalyzerProbeEntry) => {
            Column({ space: 2 }) {
              Text(entry.time + ' · ' + entry.cardTitle)
                .fontSize(13)
                .fontWeight(FontWeight.Medium)
              Text('数量=' + entry.count + ' · ' + entry.typeNames)
                .fontSize(12)
                .fontColor(0x666666)
            }
            .width('100%')
            .alignItems(HorizontalAlign.Start)
          }, (entry: AnalyzerProbeEntry) => entry.id.toString())
          if (this.probeEntries.length === 0) {
            Text('尚未探测, 点击任意卡片中的探测按钮')
              .fontSize(12)
              .fontColor(Color.Gray)
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        .alignItems(HorizontalAlign.Start)

        Column({ space: 8 }) {
          Text('文档约束速查')
            .fontSize(16)
            .fontColor(Color.Gray)
          ForEach(CONSTRAINT_LINES, (line: AnalyzerConstraintLine) => {
            Column({ space: 2 }) {
              Text(line.title)
                .fontSize(13)
                .fontWeight(FontWeight.Medium)
              Text(line.detail)
                .fontSize(12)
                .fontColor(Color.Gray)
            }
            .width('100%')
            .alignItems(HorizontalAlign.Start)
          }, (line: AnalyzerConstraintLine) => line.id.toString())
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Text('分析结果由系统侧AI能力产生, 设备不支持时 getImageAnalyzerSupportTypes 返回空数组属预期行为')
          .fontSize(12)
          .fontColor(Color.Blue)
        Text('ImageAIOptions 需与 enableAnalyzer(true) 搭配使用; types 优先级高于 ImageAnalyzerConfig.types')
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
  AnalyzerMatrixCard(meta: AnalyzerCardMeta) {
    Column({ space: 8 }) {
      Text(meta.title)
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
      Text('types: ' + meta.typesDesc + ' · ' + meta.hint)
        .fontSize(12)
        .fontColor(Color.Gray)
      Row({ space: 14 }) {
        if (this.analyzerPixelMap !== undefined && this.optionsList.length === ANALYZER_CARD_METAS.length) {
          Image(this.analyzerPixelMap, this.optionsList[meta.id])
            .width(84)
            .height(84)
            .enableAnalyzer(this.analyzerOn)
            .borderWidth(1)
            .borderColor('#DDDDDD')
            .borderRadius(8)
        }
        Column({ space: 6 }) {
          Button('探测支持类型')
            .onClick(() => {
              this.probeCard(meta);
            })
          Text('结果: ' + this.cardStatusList[meta.id])
            .fontSize(12)
            .fontColor(0x666666)
        }
        .alignItems(HorizontalAlign.Start)
        .layoutWeight(1)
      }
      .width('100%')
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
    .alignItems(HorizontalAlign.Start)
  }
}
