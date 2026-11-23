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
 * Image 解码质量矩阵示例
 *
 * 演示内容：
 * 1. interpolation(ImageInterpolation.None/Low/Medium/High) 与 sourceSize 的组合矩阵；
 * 2. 图源为代码生成的 256x256 高频图案 PixelMap（棋盘格 + 斜线），
 *    在不同解码尺寸与显示档位下对比缩放插值质量差异；
 * 3. SVG 矢量源对照：文档明确 SVG 类型图源不支持 interpolation，
 *    放大显示依旧锐利，用于对比位图缩放失真。
 *
 * 对应文档：ts-basic-components-image.md
 * - interpolation(value: ImageInterpolation)
 * - sourceSize(value: ImageSourceSize)
 * - ImageInterpolation: None=0 最近邻 / Low=1 双线性 / Medium=2 MipMap / High=3 Cubic
 */
import { image } from '@kit.ImageKit';
import { BusinessError } from '@kit.BasicServicesKit';

/**
 * onComplete 回调事件对象（字段与文档 ts-basic-components-image.md 一致）。
 * width/height/componentWidth/componentHeight/loadingStatus 为 API7 起字段，
 * content* 四字段为 API10+ 起字段，仅 loadingStatus=1（解码成功）时有效。
 */
interface CompleteEventFields {
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
 * 单张对比卡片的配置（数据定义在 build() 之外）。
 */
interface DecodeCardConfig {
  id: string;
  title: string;
  interp: ImageInterpolation;
  interpName: string;
  interpDesc: string;
  decodeEdge: number;
}

/** 大图源边长（px），代码生成的棋盘格 + 斜线图案 */
const SOURCE_EDGE_PX: number = 256;
/** 棋盘格单格边长（px） */
const CHECK_CELL_PX: number = 16;
/** 斜线周期（px） */
const DIAGONAL_PERIOD_PX: number = 24;
/** 解码尺寸：全尺寸 */
const DECODE_FULL: number = SOURCE_EDGE_PX;
/** 解码尺寸：一半 */
const DECODE_HALF: number = 128;
/** 解码尺寸：八分之一，降采样失真最明显 */
const DECODE_TINY: number = 32;
/** 小显示档位边长（vp） */
const FRAME_SMALL_VP: number = 72;
/** 大显示档位边长（vp），放大时插值差异最明显 */
const FRAME_LARGE_VP: number = 160;
/** SVG 对照卡显示边长（vp） */
const SVG_FRAME_VP: number = 140;
/** RGBA_8888 每像素字节数 */
const BYTES_PER_PIXEL: number = 4;
/** 字节转 KB 除数 */
const BYTES_PER_KB: number = 1024;
/** onComplete 尚未回调时的占位文案 */
const WAIT_COMPLETE_TEXT: string = '等待onComplete回调';
/** 卡片标题字号 */
const TITLE_FONT_SIZE: number = 16;
/** 说明文字字号 */
const DESC_FONT_SIZE: number = 12;
/** 状态文字字号 */
const STATUS_FONT_SIZE: number = 14;

/**
 * 插值 x 解码尺寸 组合矩阵：4 档插值 x 2 档解码尺寸 = 8 张对比卡。
 */
const QUALITY_CARDS: DecodeCardConfig[] = [
  {
    id: 'none_full',
    title: 'None(最近邻) + 全尺寸解码',
    interp: ImageInterpolation.None,
    interpName: 'None',
    interpDesc: '最近邻插值，放大呈像素块',
    decodeEdge: DECODE_FULL
  },
  {
    id: 'low_full',
    title: 'Low(双线性) + 全尺寸解码',
    interp: ImageInterpolation.Low,
    interpName: 'Low',
    interpDesc: '双线性插值，默认值',
    decodeEdge: DECODE_FULL
  },
  {
    id: 'low_tiny',
    title: 'Low(双线性) + 32px解码',
    interp: ImageInterpolation.Low,
    interpName: 'Low',
    interpDesc: '解码到32px后放大，模糊最明显',
    decodeEdge: DECODE_TINY
  },
  {
    id: 'medium_full',
    title: 'Medium(MipMap) + 全尺寸解码',
    interp: ImageInterpolation.Medium,
    interpName: 'Medium',
    interpDesc: 'MipMap插值，适合缩小显示',
    decodeEdge: DECODE_FULL
  },
  {
    id: 'high_full',
    title: 'High(Cubic) + 全尺寸解码',
    interp: ImageInterpolation.High,
    interpName: 'High',
    interpDesc: 'Cubic插值质量最高，可能影响速度',
    decodeEdge: DECODE_FULL
  },
  {
    id: 'high_tiny',
    title: 'High(Cubic) + 32px解码',
    interp: ImageInterpolation.High,
    interpName: 'High',
    interpDesc: '文档建议放大显示配High',
    decodeEdge: DECODE_TINY
  }
];

/**
 * 半尺寸解码对照行：同一解码尺寸(128px)下三种有质量插值的横向对比。
 */
const HALF_DECODE_CARDS: DecodeCardConfig[] = [
  {
    id: 'half_low',
    title: 'Low + 128px解码',
    interp: ImageInterpolation.Low,
    interpName: 'Low',
    interpDesc: '双线性',
    decodeEdge: DECODE_HALF
  },
  {
    id: 'half_medium',
    title: 'Medium + 128px解码',
    interp: ImageInterpolation.Medium,
    interpName: 'Medium',
    interpDesc: 'MipMap',
    decodeEdge: DECODE_HALF
  },
  {
    id: 'half_high',
    title: 'High + 128px解码',
    interp: ImageInterpolation.High,
    interpName: 'High',
    interpDesc: 'Cubic',
    decodeEdge: DECODE_HALF
  }
];

/**
 * ImageInterpolation 枚举释义（与文档枚举表一致）。
 */
interface InterpLegend {
  name: string;
  value: number;
  algo: string;
  note: string;
}

const INTERP_LEGENDS: InterpLegend[] = [
  { name: 'None', value: 0, algo: '最近邻插值', note: '速度最快，放大后像素块明显' },
  { name: 'Low', value: 1, algo: '双线性插值', note: '默认值，均衡质量与速度' },
  { name: 'Medium', value: 2, algo: 'MipMap插值', note: '缩小显示推荐配置' },
  { name: 'High', value: 3, algo: 'Cubic插值', note: '质量最高，可能影响渲染速度' }
];

/**
 * SVG 对照卡配置：验证 SVG 源不受 interpolation 影响。
 */
interface SvgCompareConfig {
  label: string;
  interp: ImageInterpolation;
  resource: Resource;
}

const SVG_COMPARE_CARDS: SvgCompareConfig[] = [
  { label: 'SVG + interpolation.Low', interp: ImageInterpolation.Low, resource: $r('app.media.svg2_gradient_linear') },
  { label: 'SVG + interpolation.High', interp: ImageInterpolation.High, resource: $r('app.media.svg2_gradient_linear') }
];

@Entry
@Component
struct ImageDecodeQualityMatrixExample {
  /** 代码生成的大图源（棋盘格 + 斜线） */
  @State bigPixelMap: image.PixelMap | undefined = undefined;
  /** 显示档位：false=72vp 缩小档，true=160vp 放大档 */
  @State zoomLarge: boolean = true;
  /** 每张矩阵卡最近一次 onComplete 的摘要 */
  @State cardStatus: string[] = QUALITY_CARDS.map((card: DecodeCardConfig): string => WAIT_COMPLETE_TEXT);
  /** 图源创建状态 */
  @State sourceStatus: string = '正在创建大图源...';
  /** SVG 对照卡回调摘要 */
  @State svgStatus: string = WAIT_COMPLETE_TEXT;

  async aboutToAppear(): Promise<void> {
    await this.createSourcePixelMap();
  }

  /**
   * 生成高频细节图案的 ArrayBuffer：
   * 棋盘格底纹叠加对角条纹，缩放后失真（锯齿/模糊）肉眼可辨。
   */
  buildPatternBuffer(edge: number): ArrayBuffer {
    const buffer: ArrayBuffer = new ArrayBuffer(edge * edge * BYTES_PER_PIXEL);
    const data: Uint8Array = new Uint8Array(buffer);
    for (let y: number = 0; y < edge; y++) {
      for (let x: number = 0; x < edge; x++) {
        const idx: number = (y * edge + x) * BYTES_PER_PIXEL;
        const checker: boolean =
          (Math.floor(x / CHECK_CELL_PX) + Math.floor(y / CHECK_CELL_PX)) % 2 === 0;
        const stripe: boolean = ((x + y) % DIAGONAL_PERIOD_PX) < (DIAGONAL_PERIOD_PX / 2);
        const bright: number = (checker ? 235 : 20) + (stripe ? 10 : 0);
        data[idx] = bright;
        data[idx + 1] = checker ? 90 : 200;
        data[idx + 2] = stripe ? 240 : 60;
        data[idx + 3] = 255;
      }
    }
    return buffer;
  }

  /**
   * 创建 256x256 大图源 PixelMap，失败时把错误码写进状态文案。
   */
  async createSourcePixelMap(): Promise<void> {
    const size: image.Size = { width: SOURCE_EDGE_PX, height: SOURCE_EDGE_PX };
    const opts: image.InitializationOptions = {
      size: size,
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      scaleMode: image.ScaleMode.FIT_TARGET_SIZE,
      editable: false
    };
    try {
      const buffer: ArrayBuffer = this.buildPatternBuffer(SOURCE_EDGE_PX);
      this.bigPixelMap = await image.createPixelMap(buffer, opts);
      const memKb: number = SOURCE_EDGE_PX * SOURCE_EDGE_PX * BYTES_PER_PIXEL / BYTES_PER_KB;
      this.sourceStatus = '图源创建成功 ' + SOURCE_EDGE_PX + 'x' + SOURCE_EDGE_PX +
        '，解码内存约 ' + memKb.toFixed(1) + ' KB';
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      this.sourceStatus = '图源创建失败 code=' + err.code;
    }
  }

  /**
   * 当前显示档位边长（vp）。
   */
  frameEdge(): number {
    return this.zoomLarge ? FRAME_LARGE_VP : FRAME_SMALL_VP;
  }

  /**
   * 矩阵卡 onComplete：记录回调报告的图源尺寸与组件尺寸。
   */
  onCardComplete(index: number, event?: CompleteEventFields): void {
    if (event === undefined) {
      this.cardStatus[index] = WAIT_COMPLETE_TEXT;
      return;
    }
    const width: number = event.width;
    const height: number = event.height;
    const compWidth: number = event.componentWidth;
    const compHeight: number = event.componentHeight;
    this.cardStatus[index] = '回调: 图源' + width + 'x' + height +
      ' 组件' + compWidth.toFixed(0) + 'x' + compHeight.toFixed(0) +
      ' loadingStatus=' + event.loadingStatus;
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('interpolation x sourceSize 解码质量矩阵')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        // 图源与显示档位控制卡
        Column({ space: 8 }) {
          Text('大图源与显示档位控制').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Row({ space: 10 }) {
            Button('重建256px大图源').fontSize(13).layoutWeight(1).onClick(() => {
              this.createSourcePixelMap();
            })
            Button(this.zoomLarge ? '切到缩小档72vp' : '切到放大档160vp')
              .fontSize(13).layoutWeight(1).onClick(() => {
                this.zoomLarge = !this.zoomLarge;
              })
          }.width('100%')
          if (this.bigPixelMap !== undefined) {
            Image(this.bigPixelMap)
              .width(96)
              .height(96)
              .objectFit(ImageFit.Fill)
              .borderWidth(1)
          }
          Text('状态: ' + this.sourceStatus).fontSize(STATUS_FONT_SIZE)
          Text('图案构成：16px棋盘格 + 24px周期对角条纹，高频细节对插值算法最敏感。')
            .fontSize(DESC_FONT_SIZE)
            .fontColor(Color.Gray)
          Text('档位: 显示' + this.frameEdge() + 'vp，放大档下32px解码源模糊最明显；' +
            '缩小档下MipMap(Medium)抗锯齿优势最明显')
            .fontSize(DESC_FONT_SIZE)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 插值枚举释义卡
        Column({ space: 6 }) {
          Text('ImageInterpolation 枚举速查').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          ForEach(INTERP_LEGENDS, (legend: InterpLegend) => {
            Row({ space: 8 }) {
              Text(legend.name + '(' + legend.value + ')')
                .fontSize(DESC_FONT_SIZE)
                .fontWeight(FontWeight.Medium)
                .width(86)
              Text(legend.algo).fontSize(DESC_FONT_SIZE).width(84)
              Text(legend.note).fontSize(DESC_FONT_SIZE).fontColor(Color.Gray).layoutWeight(1)
            }.width('100%')
          }, (legend: InterpLegend): string => legend.name)
          Text('默认值为Low；SVG类型图源不支持该属性(文档interpolation一节)。')
            .fontSize(DESC_FONT_SIZE)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 插值 x 解码尺寸 矩阵卡（ForEach 渲染）
        ForEach(QUALITY_CARDS, (card: DecodeCardConfig, index: number) => {
          Column({ space: 8 }) {
            Text(card.title).fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
            Row({ space: 12 }) {
              Image(this.bigPixelMap)
                .interpolation(card.interp)
                .sourceSize({ width: card.decodeEdge, height: card.decodeEdge })
                .objectFit(ImageFit.Fill)
                .width(this.frameEdge())
                .height(this.frameEdge())
                .borderWidth(1)
                .onComplete((event?: CompleteEventFields) => {
                  this.onCardComplete(index, event);
                })
              if (card.interp !== ImageInterpolation.Low) {
                Column({ space: 2 }) {
                  Image(this.bigPixelMap)
                    .interpolation(ImageInterpolation.Low)
                    .objectFit(ImageFit.Fill)
                    .width(this.frameEdge())
                    .height(this.frameEdge())
                    .borderWidth(1)
                  Text('基准: Low全尺寸').fontSize(DESC_FONT_SIZE).fontColor(Color.Gray)
                }
              }
              Column({ space: 4 }) {
                Text('插值: ' + card.interpName).fontSize(STATUS_FONT_SIZE)
                Text(card.interpDesc).fontSize(DESC_FONT_SIZE).fontColor(Color.Gray)
                Text('sourceSize: ' + card.decodeEdge + 'vp').fontSize(DESC_FONT_SIZE)
                Text('解码内存估算: ' +
                  (card.decodeEdge * card.decodeEdge * BYTES_PER_PIXEL / BYTES_PER_KB).toFixed(1) + ' KB')
                  .fontSize(DESC_FONT_SIZE)
                Text('缩放系数: ' + (this.frameEdge() / card.decodeEdge).toFixed(2) + 'x')
                  .fontSize(DESC_FONT_SIZE)
              }.layoutWeight(1)
              .alignItems(HorizontalAlign.Start)
            }.width('100%')
            .alignItems(VerticalAlign.Center)
            Text(this.cardStatus[index]).fontSize(DESC_FONT_SIZE).fontColor(Color.Black)
          }
          .width('100%')
          .padding(15)
          .backgroundColor('#F5F5F5')
          .borderRadius(10)
        }, (card: DecodeCardConfig): string => card.id)

        // 半尺寸解码对照行
        Column({ space: 8 }) {
          Text('同一解码尺寸(128px)下插值横向对照').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Row({ space: 10 }) {
            ForEach(HALF_DECODE_CARDS, (halfCard: DecodeCardConfig) => {
              Column({ space: 4 }) {
                Image(this.bigPixelMap)
                  .interpolation(halfCard.interp)
                  .sourceSize({ width: halfCard.decodeEdge, height: halfCard.decodeEdge })
                  .objectFit(ImageFit.Fill)
                  .width(FRAME_SMALL_VP)
                  .height(FRAME_SMALL_VP)
                  .borderWidth(1)
                Text(halfCard.title).fontSize(DESC_FONT_SIZE).fontColor(Color.Gray)
              }
            }, (halfCard: DecodeCardConfig): string => halfCard.id)
          }.width('100%')
          .justifyContent(FlexAlign.SpaceEvenly)
          Text('128px源缩小到' + FRAME_SMALL_VP + 'vp显示：Low边缘锯齿更硬，' +
            'Medium(MipMap)过渡更平滑，High(Cubic)细节保留最多。')
            .fontSize(DESC_FONT_SIZE)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // SVG 矢量源对照卡
        Column({ space: 8 }) {
          Text('SVG矢量源对照(不受插值影响)').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Row({ space: 12 }) {
            ForEach(SVG_COMPARE_CARDS, (svgCard: SvgCompareConfig) => {
              Column({ space: 4 }) {
                Image(svgCard.resource)
                  .interpolation(svgCard.interp)
                  .width(SVG_FRAME_VP)
                  .height(SVG_FRAME_VP)
                  .borderWidth(1)
                  .onComplete((event?: CompleteEventFields) => {
                    if (event !== undefined) {
                      this.svgStatus = 'SVG回调: 图源' + event.width + 'x' + event.height +
                        ' 组件' + event.componentWidth.toFixed(0) + 'x' + event.componentHeight.toFixed(0);
                    }
                  })
                Text(svgCard.label).fontSize(DESC_FONT_SIZE).fontColor(Color.Gray)
              }
            }, (svgCard: SvgCompareConfig): string => svgCard.label)
          }.width('100%')
          .justifyContent(FlexAlign.SpaceEvenly)
          Text(this.svgStatus).fontSize(DESC_FONT_SIZE)
          Text('两图完全一致：SVG按矢量指令在显示尺寸上重新绘制，' +
            '文档明确 SVG 类型图源不支持 interpolation。')
            .fontSize(DESC_FONT_SIZE)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 质量结论卡
        Column({ space: 6 }) {
          Text('缩放质量结论(来自文档建议)').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Text('1. 缩小显示最佳配置: autoResize(false) + interpolation(Medium)。').fontSize(DESC_FONT_SIZE)
          Text('2. 放大显示最佳配置: interpolation(High)，Cubic插值质量最高但可能影响渲染速度。').fontSize(DESC_FONT_SIZE)
          Text('3. None为最近邻插值，放大后呈明显像素块，仅适合像素风图像。').fontSize(DESC_FONT_SIZE)
          Text('4. sourceSize仅在目标尺寸小于图源尺寸时生效，用于降低解码分辨率。').fontSize(DESC_FONT_SIZE)
          Text('5. 设置undefined时interpolation取值为ImageInterpolation.None(文档参数表)。')
            .fontSize(DESC_FONT_SIZE)
          Text('6. 观察方法：切到放大档160vp对比None与High的方格边缘，像素块与平滑过渡差异直观。')
            .fontSize(DESC_FONT_SIZE)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        .alignItems(HorizontalAlign.Start)

        Text('注意：SVG图源与PixelMap资源不支持sourceSize；AnimatedDrawableDescriptor图源' +
          '不支持interpolation/sourceSize。本页以代码生成PixelMap演示API写法与插值效果差异。')
          .fontSize(12)
          .fontColor(Color.Blue)
          .margin({ bottom: 20 })
      }.width('100%').padding({ left: 20, right: 20 })
    }.width('100%').height('100%').backgroundColor(Color.White)
  }
}
