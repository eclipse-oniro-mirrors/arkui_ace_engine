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
 * alt<22+> 与 ImageAlt<22+> 接口占位图示例
 *
 * 与既有页面的差异定位:
 * - ImageAltExample 用包内资源直接切换 valid/invalid 图源, 且三种配置卡内容相同;
 *   本页按 22+ 文档语义设计互补场景: PixelMap 异步延迟构造模拟慢加载(观察加载占位),
 *   placeholder/error 三组合严格对齐文档示例28(同时设置/仅placeholder/仅error),
 *   并演示占位源在 Resource 与 PixelMap 之间切换(ImageAlt 字段类型为 ResourceStr | PixelMap).
 *
 * 文档依据 (ts-basic-components-image.md):
 * - alt<22+>(src: ResourceStr | PixelMap | ImageAlt): 设置加载过程中和加载失败时的占位图.
 * - ImageAlt<22+>: placeholder(可选, 加载过程中) / error(可选, 加载失败), 类型均为 ResourceStr | PixelMap.
 * - 未配置时默认不显示; 占位图不支持网络图片, 支持svg; 参数为 AnimatedDrawableDescriptor 时 alt 不生效.
 * - 占位图支持 objectFit 填充, 与图片填充效果一致.
 * - onError<9+>(callback: ImageErrorCallback), ImageError.message<10+> 为报错信息.
 */
import { image } from '@kit.ImageKit';
import { BusinessError } from '@kit.BasicServicesKit';

interface AltComboCard {
  id: number;
  title: string;
  desc: string;
  comboMode: number;
}

interface AltTimelineEntry {
  id: number;
  time: string;
  label: string;
}

interface PlaceholderPreset {
  id: number;
  label: string;
  usePixelMap: boolean;
}

const INVALID_IMAGE_SRC: string = 'alt_v22_missing_bitmap.png';
const SLOW_LOAD_DELAY_MS: number = 1500;

const ALT_COMBO_CARDS: AltComboCard[] = [
  {
    id: 0,
    title: '同时设置 placeholder 与 error',
    desc: '加载过程显示placeholder, 失败显示error(文档示例28第一式)',
    comboMode: 0
  },
  {
    id: 1,
    title: '只设置 placeholder',
    desc: '失败时无占位图可显示, 加载过程仍显示placeholder',
    comboMode: 1
  },
  {
    id: 2,
    title: '只设置 error',
    desc: '加载过程无占位, 失败时显示error占位图',
    comboMode: 2
  }
];

const PLACEHOLDER_PRESETS: PlaceholderPreset[] = [
  { id: 0, label: 'Resource占位(star.svg)', usePixelMap: false },
  { id: 1, label: 'PixelMap占位(运行时)', usePixelMap: true }
];

interface AltFitCard {
  id: number;
  fit: ImageFit;
  label: string;
}

const ALT_FIT_CARDS: AltFitCard[] = [
  { id: 0, fit: ImageFit.Contain, label: 'Contain: 占位图等比完整显示' },
  { id: 1, fit: ImageFit.Cover, label: 'Cover: 占位图等比铺满并裁切' },
  { id: 2, fit: ImageFit.Fill, label: 'Fill: 占位图拉伸填满' },
  { id: 3, fit: ImageFit.None, label: 'None: 占位图按原始尺寸居中' }
];

@Entry
@Component
struct ImageAltV22Example {
  @State slowPixelMap: image.PixelMap | undefined = undefined;
  @State placeholderPixelMap: image.PixelMap | undefined = undefined;
  @State errorPixelMap: image.PixelMap | undefined = undefined;
  @State demoMode: number = 0;
  @State usePixelMapPlaceholder: boolean = false;
  @State statusText: string = '待开始: 点击“模拟慢加载”观察加载占位';
  @State lastErrorMessage: string = '尚未触发onError';
  @State timeline: AltTimelineEntry[] = [];
  private timelineSeq: number = 0;

  async aboutToAppear(): Promise<void> {
    this.placeholderPixelMap = await this.makeSolidPixelMap(255, 193, 7);
    this.errorPixelMap = await this.makeSolidPixelMap(220, 53, 69);
    this.pushTimeline('占位PixelMap(暖黄/赤红)准备完成');
  }

  private async makeSolidPixelMap(r: number, g: number, b: number): Promise<image.PixelMap | undefined> {
    const size: number = 96;
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
      console.error('makeSolidPixelMap failed code=' + err.code);
      return undefined;
    }
  }

  private delay(ms: number): Promise<void> {
    return new Promise<void>((resolve: () => void) => {
      setTimeout(resolve, ms);
    });
  }

  private pushTimeline(label: string): void {
    this.timelineSeq += 1;
    const entry: AltTimelineEntry = {
      id: this.timelineSeq,
      time: new Date().toLocaleTimeString(),
      label: label
    };
    this.timeline = [entry].concat(this.timeline);
  }

  private async startSlowLoad(): Promise<void> {
    this.demoMode = 0;
    this.slowPixelMap = undefined;
    this.statusText = '加载中: 图源尚未就绪, 展示 placeholder 占位';
    this.pushTimeline('发起慢加载, 延迟' + SLOW_LOAD_DELAY_MS + 'ms构造PixelMap');
    await this.delay(SLOW_LOAD_DELAY_MS);
    const pixelMap: image.PixelMap | undefined = await this.makeSolidPixelMap(33, 150, 243);
    this.slowPixelMap = pixelMap;
    if (pixelMap !== undefined) {
      this.demoMode = 2;
      this.statusText = '加载完成: PixelMap就绪, 占位图被真实内容替换';
      this.pushTimeline('慢加载完成, 显示真实图');
    } else {
      this.demoMode = 1;
      this.statusText = '构造失败: 转为error占位演示';
      this.pushTimeline('PixelMap构造失败, 进入失败态');
    }
  }

  private startFailure(): void {
    this.demoMode = 1;
    this.statusText = '失败态: 图源为无效路径, 展示 error 占位';
    this.pushTimeline('切换到无效图源');
  }

  private resetDemo(): void {
    this.demoMode = 0;
    this.slowPixelMap = undefined;
    this.statusText = '已重置: 等待下一次演示';
    this.pushTimeline('重置演示状态');
  }

  private mainSrc(): image.PixelMap | string | undefined {
    if (this.demoMode === 1) {
      return INVALID_IMAGE_SRC;
    }
    if (this.demoMode === 2) {
      return this.slowPixelMap;
    }
    return undefined;
  }

  private mainPlaceholder(): ResourceStr | image.PixelMap | undefined {
    if (this.usePixelMapPlaceholder) {
      return this.placeholderPixelMap;
    }
    return $r('app.media.star');
  }

  private mainError(): ResourceStr | image.PixelMap | undefined {
    if (this.usePixelMapPlaceholder) {
      return this.errorPixelMap;
    }
    return $r('app.media.heart');
  }

  private altForCombo(comboMode: number): ImageAlt {
    if (comboMode === 0) {
      const both: ImageAlt = {
        placeholder: $r('app.media.star'),
        error: $r('app.media.heart')
      };
      return both;
    }
    if (comboMode === 1) {
      const onlyPlaceholder: ImageAlt = {
        placeholder: $r('app.media.star')
      };
      return onlyPlaceholder;
    }
    const onlyError: ImageAlt = {
      error: $r('app.media.heart')
    };
    return onlyError;
  }

  private modeLabel(): string {
    if (this.demoMode === 0) {
      return '加载中(placeholder)';
    }
    if (this.demoMode === 1) {
      return '失败(error)';
    }
    return '成功(真实图)';
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('alt<22+> 与 ImageAlt 占位图示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        Column({ space: 10 }) {
          Text('慢加载 / 失败 / 重置 演示台')
            .fontSize(16)
            .fontColor(Color.Gray)
          Row({ space: 10 }) {
            Button('模拟慢加载')
              .onClick(() => {
                this.startSlowLoad();
              })
            Button('模拟加载失败')
              .onClick(() => {
                this.startFailure();
              })
            Button('重置')
              .onClick(() => {
                this.resetDemo();
              })
          }
          Row({ space: 12 }) {
            Column({ space: 6 }) {
              Image(this.mainSrc())
                .width(120)
                .height(120)
                .objectFit(ImageFit.Contain)
                .alt({
                  placeholder: this.mainPlaceholder(),
                  error: this.mainError()
                })
                .borderWidth(1)
                .borderColor('#CCCCCC')
                .borderRadius(10)
                .onError((error: ImageError) => {
                  this.lastErrorMessage = 'onError.message: ' + error.message;
                  this.pushTimeline('onError触发: ' + error.message);
                })
              Text('当前状态: ' + this.modeLabel())
                .fontSize(13)
            }
            Column({ space: 6 }) {
              Text(this.statusText)
                .fontSize(13)
                .fontColor(0x666666)
              Text(this.lastErrorMessage)
                .fontSize(12)
                .fontColor(Color.Gray)
              Row({ space: 8 }) {
                ForEach(PLACEHOLDER_PRESETS, (preset: PlaceholderPreset) => {
                  Button(preset.label)
                    .fontSize(11)
                    .backgroundColor(this.usePixelMapPlaceholder === preset.usePixelMap ? 0x007DFF : 0xE0E0E0)
                    .fontColor(this.usePixelMapPlaceholder === preset.usePixelMap ? Color.White : 0x333333)
                    .onClick(() => {
                      this.usePixelMapPlaceholder = preset.usePixelMap;
                      this.pushTimeline('占位源切换为' + preset.label);
                    })
                }, (preset: PlaceholderPreset) => preset.id.toString())
              }
              Text('占位源类型: ' + (this.usePixelMapPlaceholder
                ? 'PixelMap(运行时构造)'
                : 'Resource(star/heart.svg)'))
                .fontSize(12)
                .fontColor(Color.Gray)
            }
            .alignItems(HorizontalAlign.Start)
            .layoutWeight(1)
          }
          .width('100%')
          .alignItems(VerticalAlign.Top)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Text('ImageAlt 组合矩阵(图源固定为无效路径, 观察占位差异)')
          .fontSize(16)
          .fontWeight(FontWeight.Medium)

        ForEach(ALT_COMBO_CARDS, (card: AltComboCard) => {
          this.AltComboCardView(card)
        }, (card: AltComboCard) => card.id.toString())

        Column({ space: 8 }) {
          Text('占位源 PixelMap 对照(加载成功图 vs 占位图)')
            .fontSize(16)
            .fontColor(Color.Gray)
          Row({ space: 14 }) {
            Column({ space: 4 }) {
              if (this.placeholderPixelMap !== undefined) {
                Image(INVALID_IMAGE_SRC)
                  .width(80)
                  .height(80)
                  .alt({ placeholder: this.placeholderPixelMap, error: this.placeholderPixelMap })
                  .borderRadius(8)
              }
              Text('PixelMap占位(暖黄)')
                .fontSize(12)
                .fontColor(Color.Gray)
            }
            Column({ space: 4 }) {
              if (this.errorPixelMap !== undefined) {
                Image(INVALID_IMAGE_SRC)
                  .width(80)
                  .height(80)
                  .alt({ placeholder: this.errorPixelMap, error: this.errorPixelMap })
                  .borderRadius(8)
              }
              Text('PixelMap占位(赤红)')
                .fontSize(12)
                .fontColor(Color.Gray)
            }
            Column({ space: 4 }) {
              if (this.slowPixelMap !== undefined) {
                Image(this.slowPixelMap)
                  .width(80)
                  .height(80)
                  .borderRadius(8)
              }
              Text(this.slowPixelMap !== undefined ? '慢加载产物(蓝)' : '慢加载产物(未生成)')
                .fontSize(12)
                .fontColor(this.slowPixelMap !== undefined ? 0x007DFF : Color.Gray)
            }
          }
          .width('100%')
          .justifyContent(FlexAlign.SpaceAround)
          Text('ImageAlt.placeholder/error 支持 ResourceStr 与 PixelMap 两类占位源, 不支持网络图片')
            .fontSize(12)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Column({ space: 8 }) {
          Text('占位图 objectFit 对照(文档: 占位图填充效果与图片一致)')
            .fontSize(16)
            .fontColor(Color.Gray)
          ForEach(ALT_FIT_CARDS, (fitCard: AltFitCard) => {
            Row({ space: 12 }) {
              Image(INVALID_IMAGE_SRC)
                .width(64)
                .height(64)
                .objectFit(fitCard.fit)
                .alt({ placeholder: $r('app.media.star'), error: $r('app.media.heart') })
                .borderWidth(1)
                .borderColor('#EEEEEE')
                .borderRadius(6)
              Text(fitCard.label)
                .fontSize(12)
                .fontColor(0x666666)
                .layoutWeight(1)
            }
            .width('100%')
          }, (fitCard: AltFitCard) => fitCard.id.toString())
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Column({ space: 6 }) {
          Text('状态时间线')
            .fontSize(16)
            .fontColor(Color.Gray)
          ForEach(this.timeline, (entry: AltTimelineEntry) => {
            Text(entry.time + ' · ' + entry.label)
              .fontSize(12)
              .fontColor(0x666666)
          }, (entry: AltTimelineEntry) => entry.id.toString())
          if (this.timeline.length === 0) {
            Text('暂无事件')
              .fontSize(12)
              .fontColor(Color.Gray)
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        .alignItems(HorizontalAlign.Start)

        Text('alt<22+> 相对旧版单占位 alt 的增强: ImageAlt 可分别配置加载中与失败占位, 未配置项默认不显示')
          .fontSize(12)
          .fontColor(Color.Blue)
        Text('占位图同样受 objectFit 控制; 组件参数为 AnimatedDrawableDescriptor 时 alt 不生效; 旧版 alt 由有效值切无效值会保持旧图, 22+ 通过 error 字段显式声明失败占位')
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
  AltComboCardView(card: AltComboCard) {
    Column({ space: 6 }) {
      Text(card.title)
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
      Text(card.desc)
        .fontSize(12)
        .fontColor(Color.Gray)
      Row({ space: 12 }) {
        Image(INVALID_IMAGE_SRC)
          .width(72)
          .height(72)
          .objectFit(ImageFit.Contain)
          .alt(this.altForCombo(card.comboMode))
          .borderWidth(1)
          .borderColor('#EEEEEE')
          .borderRadius(8)
          .onError((error: ImageError) => {
            console.info('combo card load failed: ' + error.message);
          })
        Column({ space: 3 }) {
          Text(card.comboMode === 0 ? '加载与失败各有占位'
            : (card.comboMode === 1 ? '仅加载占位生效' : '仅失败占位生效'))
            .fontSize(12)
            .fontColor(0x666666)
          Text('无效源: ' + INVALID_IMAGE_SRC)
            .fontSize(11)
            .fontColor(Color.Gray)
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
