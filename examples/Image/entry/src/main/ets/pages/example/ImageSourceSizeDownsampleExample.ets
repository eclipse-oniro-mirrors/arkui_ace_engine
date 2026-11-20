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
 * Image sourceSize 降采样解码与内存估算示例
 *
 * 演示内容：
 * 1. sourceSize(value: ImageSourceSize) 的两种写法：
 *    新写法——先声明 ImageSourceSize 接口对象(18+对象说明，width/height字段7+)再传入；
 *    旧写法——直接传对象字面量；
 * 2. sourceSize 与 objectFit(ImageFit.None) 配合在组件内显示小图(文档建议用法)；
 * 3. 生效规则验证：仅在目标尺寸小于图源尺寸时生效，512>256时不生效；
 * 4. onComplete 回读实际解码后尺寸(width/height, 单位px)；
 * 5. 解码内存估算 width*height*4(RGBA_8888)；
 * 6. Image Kit 解码缩放模式 image.ScaleMode(9+)：FIT_TARGET_SIZE(0) 与
 *    CENTER_CROP(1) 从 200x100 原始缓冲解码到 100x100 的对照。
 *
 * 对应文档：ts-basic-components-image.md #sourceSize #ImageSourceSize
 *          arkts-apis-image-e.md #ScaleMode
 */
import { image } from '@kit.ImageKit';
import { BusinessError } from '@kit.BasicServicesKit';

/**
 * onComplete 事件对象（本页只消费解码尺寸相关字段）。
 */
interface DecodeProbeEvent {
  width: number;
  height: number;
  componentWidth: number;
  componentHeight: number;
  loadingStatus: number;
  contentWidth: number;
  contentHeight: number;
  contentOffsetX: number;
  contentOffsetY: number;
}

/**
 * 降采样对照卡配置。
 */
interface DownCardConfig {
  id: string;
  label: string;
  decodeW: number;
  decodeH: number;
  fit: ImageFit;
  fitName: string;
  styleName: string;
  useTypedConst: boolean;
}

/** 图源边长（px），256x256 网格图案 */
const SRC_EDGE_PX: number = 256;
/** 网格图案单格边长（px） */
const GRID_CELL_PX: number = 8;
/** RGBA_8888 每像素字节数 */
const BYTES_PER_PIXEL: number = 4;
/** 字节转 KB 除数 */
const BYTES_PER_KB: number = 1024;
/** 对照卡显示边长（vp） */
const CARD_FRAME_VP: number = 120;
/** 全解码尺寸 */
const SIZE_FULL: number = SRC_EDGE_PX;
/** 四分之一解码尺寸 */
const SIZE_QUARTER: number = 64;
/** 八分之一解码尺寸 */
/** 大于图源尺寸的无效解码尺寸(文档：仅目标<图源时生效) */
const SIZE_OVERSOURCE: number = 512;
/** ScaleMode 对照：原始缓冲宽 */
const RAW_BUF_W: number = 200;
/** ScaleMode 对照：原始缓冲高 */
const RAW_BUF_H: number = 100;
/** ScaleMode 对照：解码目标边长 */
const DECODE_TARGET_EDGE: number = 100;
/** ScaleMode 对照卡显示宽（vp） */
const SCALE_CARD_W_VP: number = 130;
/** ScaleMode 对照卡显示高（vp） */
const SCALE_CARD_H_VP: number = 74;
/** 未回调占位 */
const WAIT_TEXT: string = '等待onComplete';
/** 标题字号 */
const TITLE_FONT_SIZE: number = 16;
/** 说明字号 */
const DESC_FONT_SIZE: number = 12;
/** 状态字号 */
const STATUS_FONT_SIZE: number = 14;
/** 按钮字号 */
const BUTTON_FONT_SIZE: number = 13;

/**
 * 新写法(18+)：先声明 ImageSourceSize 接口对象再传入 sourceSize。
 * ImageSourceSize 为全局 ArkUI 对象说明，字段 width/height 自 API7，单位 vp。
 */
const TYPED_FULL_SIZE: ImageSourceSize = { width: SIZE_FULL, height: SIZE_FULL };
/** 新写法：四分之一解码 */
const TYPED_QUARTER_SIZE: ImageSourceSize = { width: SIZE_QUARTER, height: SIZE_QUARTER };
/** 新写法：大于图源尺寸，文档规则下不生效 */
const TYPED_OVER_SIZE: ImageSourceSize = { width: SIZE_OVERSOURCE, height: SIZE_OVERSOURCE };

/**
 * 降采样对照矩阵：4 张卡覆盖两种写法、ImageFit.None 小图显示与不生效边界。
 */
const DOWN_CARDS: DownCardConfig[] = [
  {
    id: 'full_typed',
    label: '全解码 256x256 (ImageSourceSize对象写法)',
    decodeW: SIZE_FULL,
    decodeH: SIZE_FULL,
    fit: ImageFit.Fill,
    fitName: 'Fill',
    styleName: '新写法: const x: ImageSourceSize',
    useTypedConst: true
  },
  {
    id: 'over_legacy',
    label: '声明512x512 但大于图源256 不生效',
    decodeW: SIZE_OVERSOURCE,
    decodeH: SIZE_OVERSOURCE,
    fit: ImageFit.None,
    fitName: 'None',
    styleName: '旧写法: 验证"仅目标<图源生效"',
    useTypedConst: false
  }
];

/**
 * 解码内存估算表（RGBA_8888，width*height*4）。
 */
interface MemRow {
  decodeLabel: string;
  memKb: string;
  note: string;
}

const MEM_ROWS: MemRow[] = [
  { decodeLabel: '256x256(全解码)', memKb: '256.0 KB', note: '基准：图源原始尺寸' },
  { decodeLabel: '128x128', memKb: '64.0 KB', note: '边长减半，内存降至1/4' },
  { decodeLabel: '32x32', memKb: '4.0 KB', note: '边长1/8，内存降至1/64' },
  { decodeLabel: '512x512(不生效)', memKb: '256.0 KB', note: '大于图源，按原尺寸解码' }
];

@Entry
@Component
struct ImageSourceSizeDownsampleExample {
  /** 256x256 网格图案图源 */
  @State srcMap: image.PixelMap | undefined = undefined;
  /** ScaleMode=FIT_TARGET_SIZE 解码结果 */
  @State fitDecodeMap: image.PixelMap | undefined = undefined;
  /** ScaleMode=CENTER_CROP 解码结果 */
  @State cropDecodeMap: image.PixelMap | undefined = undefined;
  /** 各降采样卡回调摘要 */
  @State cardStatus: string[] = DOWN_CARDS.map((card: DownCardConfig): string => WAIT_TEXT);
  /** 规则演示：true 使用64(生效)，false 使用512(不生效) */
  @State ruleEffective: boolean = true;
  /** 图源状态 */
  @State srcStatus: string = '正在创建图源...';
  /** ScaleMode 对照状态 */
  @State scaleStatus: string = '等待ScaleMode解码';
  /** 规则演示卡回调 */
  @State ruleStatus: string = WAIT_TEXT;

  async aboutToAppear(): Promise<void> {
    await this.createAllSources();
  }

  /**
   * 生成网格 + 十字标记图案缓冲，小尺寸解码后仍可辨认结构完整性。
   */
  buildGridBuffer(edge: number): ArrayBuffer {
    const buffer: ArrayBuffer = new ArrayBuffer(edge * edge * BYTES_PER_PIXEL);
    const data: Uint8Array = new Uint8Array(buffer);
    for (let y: number = 0; y < edge; y++) {
      for (let x: number = 0; x < edge; x++) {
        const idx: number = (y * edge + x) * BYTES_PER_PIXEL;
        const onGrid: boolean = (x % GRID_CELL_PX === 0) || (y % GRID_CELL_PX === 0);
        const onCross: boolean = Math.abs(x - edge / 2) < 3 || Math.abs(y - edge / 2) < 3;
        data[idx] = onCross ? 220 : (onGrid ? 40 : 245);
        data[idx + 1] = onCross ? 40 : (onGrid ? 160 : 245);
        data[idx + 2] = onCross ? 40 : (onGrid ? 40 : 245);
        data[idx + 3] = 255;
      }
    }
    return buffer;
  }

  /**
   * 生成矩形原始缓冲（非方形），用于 ScaleMode 解码到正方形目标的对照。
   */
  buildRectBuffer(width: number, height: number): ArrayBuffer {
    const buffer: ArrayBuffer = new ArrayBuffer(width * height * BYTES_PER_PIXEL);
    const data: Uint8Array = new Uint8Array(buffer);
    for (let y: number = 0; y < height; y++) {
      for (let x: number = 0; x < width; x++) {
        const idx: number = (y * width + x) * BYTES_PER_PIXEL;
        const band: number = Math.floor(x / 10) % 2;
        data[idx] = band === 0 ? 250 : 60;
        data[idx + 1] = band === 0 ? 160 : 220;
        data[idx + 2] = band === 0 ? 60 : 250;
        data[idx + 3] = 255;
      }
    }
    return buffer;
  }

  /**
   * 创建网格图源与两个 ScaleMode 对照解码结果。
   */
  async createAllSources(): Promise<void> {
    const srcSize: image.Size = { width: SRC_EDGE_PX, height: SRC_EDGE_PX };
    const srcOpts: image.InitializationOptions = {
      size: srcSize,
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      scaleMode: image.ScaleMode.FIT_TARGET_SIZE,
      editable: false
    };
    const targetSize: image.Size = { width: DECODE_TARGET_EDGE, height: DECODE_TARGET_EDGE };
    const fitOpts: image.InitializationOptions = {
      size: targetSize,
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      scaleMode: image.ScaleMode.FIT_TARGET_SIZE,
      editable: false
    };
    const cropOpts: image.InitializationOptions = {
      size: targetSize,
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      scaleMode: image.ScaleMode.CENTER_CROP,
      editable: false
    };
    try {
      this.srcMap = await image.createPixelMap(this.buildGridBuffer(SRC_EDGE_PX), srcOpts);
      const rectBuf: ArrayBuffer = this.buildRectBuffer(RAW_BUF_W, RAW_BUF_H);
      this.fitDecodeMap = await image.createPixelMap(rectBuf, fitOpts);
      this.cropDecodeMap = await image.createPixelMap(rectBuf, cropOpts);
      const baseKb: number = SRC_EDGE_PX * SRC_EDGE_PX * BYTES_PER_PIXEL / BYTES_PER_KB;
      this.srcStatus = '图源就绪 ' + SRC_EDGE_PX + 'x' + SRC_EDGE_PX + '，全解码约 ' +
        baseKb.toFixed(1) + ' KB';
      this.scaleStatus = '200x100缓冲 → 100x100目标：两种ScaleMode解码完成';
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      this.srcStatus = '图源创建失败 code=' + err.code;
    }
  }

  /**
   * 解码内存估算：width*height*BYTES_PER_PIXEL，换算为 KB 字符串。
   */
  memKbText(width: number, height: number): string {
    const kb: number = width * height * BYTES_PER_PIXEL / BYTES_PER_KB;
    return kb.toFixed(1) + ' KB';
  }

  /**
   * 当前规则演示卡实际传入的 sourceSize（新写法常量）。
   */
  ruleSourceSize(): ImageSourceSize {
    return this.ruleEffective ? TYPED_QUARTER_SIZE : TYPED_OVER_SIZE;
  }

  /**
   * 规则演示卡当前声明尺寸说明。
   */
  ruleDesc(): string {
    return this.ruleEffective ?
      '声明64x64 < 图源256 → 生效，按小图解码' :
      '声明512x512 > 图源256 → 不生效，按原尺寸解码';
  }

  /**
   * 降采样卡 onComplete：回读实际解码尺寸与组件尺寸。
   */
  onDownCardComplete(index: number, event?: DecodeProbeEvent): void {
    if (event === undefined || event.loadingStatus !== 1) {
      return;
    }
    this.cardStatus[index] = '回调解码尺寸 ' + event.width + 'x' + event.height +
      ' 组件 ' + event.componentWidth.toFixed(0) + 'x' + event.componentHeight.toFixed(0) +
      ' 内存估算 ' + this.memKbText(event.width, event.height);
  }

  /**
   * 规则演示卡 onComplete。
   */
  onRuleComplete(event?: DecodeProbeEvent): void {
    if (event === undefined || event.loadingStatus !== 1) {
      return;
    }
    this.ruleStatus = '回调解码尺寸 ' + event.width + 'x' + event.height +
      ' 内存估算 ' + this.memKbText(event.width, event.height);
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('sourceSize 降采样解码与内存估算')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        // 图源控制卡
        Column({ space: 8 }) {
          Text('图源与写法说明').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Button('重建全部图源').fontSize(BUTTON_FONT_SIZE).width('100%').onClick(() => {
            this.createAllSources();
          })
          Text(this.srcStatus).fontSize(DESC_FONT_SIZE)
          Text('新写法(18+对象说明)：const s: ImageSourceSize = { width, height } 后传入；' +
            '旧写法：直接传对象字面量。字段单位vp，取值须>0，否则按原尺寸解码。')
            .fontSize(DESC_FONT_SIZE)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 降采样对照矩阵（ForEach）
        ForEach(DOWN_CARDS, (card: DownCardConfig, index: number) => {
          Column({ space: 8 }) {
            Text(card.label).fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
            Row({ space: 12 }) {
              Image(this.srcMap)
                .sourceSize(card.useTypedConst ?
                  (card.decodeW === SIZE_FULL ? TYPED_FULL_SIZE : TYPED_QUARTER_SIZE) :
                  { width: card.decodeW, height: card.decodeH })
                .objectFit(card.fit)
                .width(CARD_FRAME_VP)
                .height(CARD_FRAME_VP)
                .borderWidth(1)
                .onComplete((event?: DecodeProbeEvent) => {
                  this.onDownCardComplete(index, event);
                })
              Column({ space: 4 }) {
                Text('写法: ' + card.styleName).fontSize(DESC_FONT_SIZE)
                Text('sourceSize: ' + card.decodeW + 'x' + card.decodeH).fontSize(DESC_FONT_SIZE)
                Text('objectFit: ' + card.fitName).fontSize(DESC_FONT_SIZE)
                Text('声明尺寸内存: ' +
                  (card.decodeW < SRC_EDGE_PX ?
                    this.memKbText(card.decodeW, card.decodeH) :
                    this.memKbText(SRC_EDGE_PX, SRC_EDGE_PX) + '(超限回退原尺寸)'))
                  .fontSize(DESC_FONT_SIZE)
              }.layoutWeight(1)
              .alignItems(HorizontalAlign.Start)
            }.width('100%')
            .alignItems(VerticalAlign.Center)
            Text(this.cardStatus[index]).fontSize(DESC_FONT_SIZE)
          }
          .width('100%')
          .padding(15)
          .backgroundColor('#F5F5F5')
          .borderRadius(10)
        }, (card: DownCardConfig): string => card.id)

        // 生效规则演示卡
        Column({ space: 8 }) {
          Text('生效规则切换演示').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Button(this.ruleEffective ? '改为512x512(不生效)' : '改为64x64(生效)')
            .fontSize(BUTTON_FONT_SIZE).onClick(() => {
              this.ruleEffective = !this.ruleEffective;
            })
          Image(this.srcMap)
            .sourceSize(this.ruleSourceSize())
            .objectFit(ImageFit.None)
            .width(CARD_FRAME_VP)
            .height(CARD_FRAME_VP)
            .borderWidth(1)
            .onComplete((event?: DecodeProbeEvent) => {
              this.onRuleComplete(event);
            })
          Text(this.ruleDesc()).fontSize(DESC_FONT_SIZE)
          Text(this.ruleStatus).fontSize(DESC_FONT_SIZE).fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // ScaleMode 解码对照卡
        Column({ space: 8 }) {
          Text('image.ScaleMode(9+) 解码缩放对照').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Row({ space: 12 }) {
            Column({ space: 4 }) {
              Image(this.fitDecodeMap)
                .width(SCALE_CARD_W_VP)
                .height(SCALE_CARD_H_VP)
                .objectFit(ImageFit.Contain)
                .borderWidth(1)
              Text('FIT_TARGET_SIZE(0) 等比容纳留边').fontSize(DESC_FONT_SIZE)
            }
            Column({ space: 4 }) {
              Image(this.cropDecodeMap)
                .width(SCALE_CARD_W_VP)
                .height(SCALE_CARD_H_VP)
                .objectFit(ImageFit.Contain)
                .borderWidth(1)
              Text('CENTER_CROP(1) 等比铺满裁边').fontSize(DESC_FONT_SIZE)
            }
          }.width('100%')
          .justifyContent(FlexAlign.SpaceEvenly)
          Text(this.scaleStatus).fontSize(DESC_FONT_SIZE)
          Text('ScaleMode属Image Kit InitializationOptions(9+)：FIT_TARGET_SIZE(0)等比缩放至完全容纳、' +
            'CENTER_CROP(1)等比缩放至铺满后居中裁剪；Image组件侧的对应填充能力是objectFit(ImageFit)。')
            .fontSize(DESC_FONT_SIZE)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 内存估算表卡
        Column({ space: 6 }) {
          Text('解码内存估算表(RGBA_8888 = width*height*4)').fontSize(TITLE_FONT_SIZE)
            .fontColor(Color.Gray)
          ForEach(MEM_ROWS, (row: MemRow) => {
            Row({ space: 8 }) {
              Text(row.decodeLabel).fontSize(DESC_FONT_SIZE).width(150)
              Text(row.memKb).fontSize(DESC_FONT_SIZE).width(86).fontColor(Color.Orange)
              Text(row.note).fontSize(DESC_FONT_SIZE).layoutWeight(1).fontColor(Color.Gray)
            }.width('100%')
          }, (row: MemRow): string => row.decodeLabel)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 使用建议卡
        Column({ space: 6 }) {
          Text('sourceSize 使用建议(文档口径)').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Text('1. 需要显示比组件更小的图时：sourceSize + ImageFit.None 组合，按解码尺寸原样呈现。')
            .fontSize(DESC_FONT_SIZE)
          Text('2. width/height取值必须>0，取值小于等于0时该参数不生效，按图片原始尺寸解码。')
            .fontSize(DESC_FONT_SIZE)
          Text('3. 降采样比例参考内存估算表：边长减半内存降为1/4，是长列表大图的首选优化项。')
            .fontSize(DESC_FONT_SIZE)
          Text('4. 与autoResize的区别：sourceSize由开发者显式指定解码尺寸，' +
            'autoResize由组件按显示区域自动决定。')
            .fontSize(DESC_FONT_SIZE)
          Text('5. AnimatedDrawableDescriptor图源参数下sourceSize不生效(文档)，动图场景勿依赖该属性控内存。')
            .fontSize(DESC_FONT_SIZE)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        .alignItems(HorizontalAlign.Start)

        Text('注意：文档明确 SVG类型图源和PixelMap资源不支持sourceSize，且仅在目标尺寸小于图源尺寸时生效；' +
          '本页以代码生成PixelMap演示两种写法与规则边界，真实位图资源场景下sourceSize直接决定解码内存。')
          .fontSize(12)
          .fontColor(Color.Blue)
          .margin({ bottom: 20 })
      }.width('100%').padding({ left: 20, right: 20 })
    }.width('100%').height('100%').backgroundColor(Color.White)
  }
}
