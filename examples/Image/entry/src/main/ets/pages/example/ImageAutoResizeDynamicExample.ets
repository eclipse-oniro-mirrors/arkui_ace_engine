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
 * Image autoResize 动态尺寸行为对比示例
 *
 * 演示内容：
 * 1. autoResize(true/false) 对照：同一 320x320 大图源，容器在 120vp/280vp 间切换，
 *    组件尺寸一致、解码内存策略不同（文档：true时按显示区域决定图源尺寸，利于省内存，
 *    降采样可能损失质量出现锯齿，可配 interpolation(Medium) 缓解）；
 * 2. onAreaChange 观察组件实际尺寸：动态改宽高并记录最近5次尺寸变化日志；
 * 3. fitOriginalSize 对照卡：显示尺寸跟随图源(96x64) vs 默认填充父容器，
 *    并验证文档规则"已设置width、height属性时fitOriginalSize不生效"；
 * 4. SVG 对照：文档明确 autoResize 对 SVG 图源不生效。
 *
 * 对应文档：ts-basic-components-image.md #autoResize #fitOriginalSize
 * 通用事件：ts-universal-events-size-change.md #onAreaChange
 *
 * 版本口径：autoResize/fitOriginalSize 为 Image 基础属性(自API7组件起支持，
 * 卡片自API9、原子化服务自API11可用，见文档各节"卡片能力/原子化服务API"说明)；
 * onAreaChange 为通用事件，无额外 @since 门槛。
 */
import { image } from '@kit.ImageKit';
import { BusinessError } from '@kit.BasicServicesKit';

/** 大图源边长（px），autoResize 降采样演示用 */
const BIG_EDGE_PX: number = 320;
/** 小图源宽（px），fitOriginalSize 演示用 */
const SMALL_SRC_W: number = 96;
/** 小图源高（px） */
const SMALL_SRC_H: number = 64;
/** 对照容器收缩档边长（vp） */
const FRAME_MIN_VP: number = 120;
/** 对照容器展开档边长（vp） */
const FRAME_MAX_VP: number = 280;
/** 观察卡基准边长（vp） */
const OBS_BASE_VP: number = 150;
/** 观察卡拉宽档宽（vp） */
const OBS_WIDE_VP: number = 240;
/** 观察卡拉宽档高（vp） */
const OBS_SHORT_VP: number = 90;
/** fitOriginalSize 对照卡容器宽（vp） */
const FIT_ROW_VP: number = 300;
/** 尺寸变化日志最大保留条数 */
const AREA_LOG_MAX: number = 5;
/** RGBA_8888 每像素字节数 */
const BYTES_PER_PIXEL: number = 4;
/** 字节转 KB 除数 */
const BYTES_PER_KB: number = 1024;
/** 同心方环图案环数 */
const RING_COUNT: number = 8;
/** 标题字号 */
const TITLE_FONT_SIZE: number = 16;
/** 说明字号 */
const DESC_FONT_SIZE: number = 12;
/** 状态字号 */
const STATUS_FONT_SIZE: number = 14;
/** 按钮字号 */
const BUTTON_FONT_SIZE: number = 13;
/** 组合对照卡单项边长（vp） */
const COMBO_ITEM_VP: number = 110;

/**
 * autoResize x interpolation 组合配置（文档最佳清晰度建议的落地验证）。
 */
interface ResizeCombo {
  id: string;
  label: string;
  auto: boolean;
  interp: ImageInterpolation;
}

/**
 * 缩小显示 4 组合：文档建议 autoResize(false)+interpolation(Medium) 最佳。
 */
const RESIZE_COMBOS: ResizeCombo[] = [
  { id: 'auto_low', label: 'autoResize(true) + Low', auto: true, interp: ImageInterpolation.Low },
  { id: 'auto_medium', label: 'autoResize(true) + Medium', auto: true, interp: ImageInterpolation.Medium },
  { id: 'fix_low', label: 'autoResize(false) + Low', auto: false, interp: ImageInterpolation.Low },
  {
    id: 'fix_medium',
    label: 'autoResize(false) + Medium(文档建议)',
    auto: false,
    interp: ImageInterpolation.Medium
  }
];

@Entry
@Component
struct ImageAutoResizeDynamicExample {
  /** 320x320 大图源 */
  @State bigMap: image.PixelMap | undefined = undefined;
  /** 96x64 小图源 */
  @State smallMap: image.PixelMap | undefined = undefined;
  /** 对照容器是否展开(280vp)，false为收缩(120vp) */
  @State frameExpanded: boolean = true;
  /** 观察卡 autoResize 开关 */
  @State autoOn: boolean = true;
  /** 观察卡尺寸是否切到 240x90 档 */
  @State obsShrink: boolean = false;
  /** 观察卡最近一次组件尺寸 */
  @State obsAreaText: string = '等待onAreaChange';
  /** 尺寸变化日志 */
  @State obsLog: string[] = [];
  /** fitOriginalSize 开关 */
  @State fitOrigOn: boolean = true;
  /** 对照卡A(autoResize true)组件尺寸 */
  @State areaTextA: string = '等待回调';
  /** 对照卡B(autoResize false)组件尺寸 */
  @State areaTextB: string = '等待回调';
  /** fitOriginalSize 卡实测尺寸 */
  @State fitAreaText: string = '等待回调';
  /** 默认行为卡实测尺寸 */
  @State plainAreaText: string = '等待回调';
  /** 固定宽高+fitOriginalSize 实测尺寸 */
  @State fixedAreaText: string = '等待回调';
  /** 图源状态 */
  @State srcStatus: string = '正在创建图源...';
  /** 组合卡各项实测尺寸 */
  @State comboAreas: string[] = RESIZE_COMBOS.map((combo: ResizeCombo): string => '等待回调');

  async aboutToAppear(): Promise<void> {
    await this.createSources();
  }

  /**
   * 生成同心方环图案 ArrayBuffer，环带交替色便于观察降采样失真。
   */
  buildRingBuffer(width: number, height: number): ArrayBuffer {
    const buffer: ArrayBuffer = new ArrayBuffer(width * height * BYTES_PER_PIXEL);
    const data: Uint8Array = new Uint8Array(buffer);
    for (let y: number = 0; y < height; y++) {
      for (let x: number = 0; x < width; x++) {
        const idx: number = (y * width + x) * BYTES_PER_PIXEL;
        const dist: number = Math.max(Math.abs(x - width / 2), Math.abs(y - height / 2));
        const ring: number = Math.floor(dist / (width / RING_COUNT / 2)) % 2;
        data[idx] = ring === 0 ? 30 : 230;
        data[idx + 1] = ring === 0 ? 120 : 200;
        data[idx + 2] = ring === 0 ? 230 : 40;
        data[idx + 3] = 255;
      }
    }
    return buffer;
  }

  /**
   * 创建大/小两个 PixelMap 图源。
   */
  async createSources(): Promise<void> {
    const bigSize: image.Size = { width: BIG_EDGE_PX, height: BIG_EDGE_PX };
    const smallSize: image.Size = { width: SMALL_SRC_W, height: SMALL_SRC_H };
    const bigOpts: image.InitializationOptions = {
      size: bigSize,
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      scaleMode: image.ScaleMode.FIT_TARGET_SIZE,
      editable: false
    };
    const smallOpts: image.InitializationOptions = {
      size: smallSize,
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      scaleMode: image.ScaleMode.FIT_TARGET_SIZE,
      editable: false
    };
    try {
      this.bigMap = await image.createPixelMap(
        this.buildRingBuffer(BIG_EDGE_PX, BIG_EDGE_PX), bigOpts);
      this.smallMap = await image.createPixelMap(
        this.buildRingBuffer(SMALL_SRC_W, SMALL_SRC_H), smallOpts);
      const bigKb: number = BIG_EDGE_PX * BIG_EDGE_PX * BYTES_PER_PIXEL / BYTES_PER_KB;
      this.srcStatus = '图源就绪: ' + BIG_EDGE_PX + 'x' + BIG_EDGE_PX + '(约' + bigKb.toFixed(0) +
        'KB) 与 ' + SMALL_SRC_W + 'x' + SMALL_SRC_H;
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      this.srcStatus = '图源创建失败 code=' + err.code;
    }
  }

  /**
   * Length(string|number|Resource) 转展示文本。
   */
  lenToStr(len: Length): string {
    if (typeof len === 'number') {
      return len.toFixed(1);
    }
    if (typeof len === 'string') {
      return len;
    }
    return JSON.stringify(len);
  }

  /**
   * Area → "宽x高" 文本。
   */
  areaToStr(area: Area): string {
    return this.lenToStr(area.width) + ' x ' + this.lenToStr(area.height);
  }

  /**
   * 记录一次尺寸变化日志，只保留最近 AREA_LOG_MAX 条。
   */
  pushLog(line: string): void {
    const next: string[] = [line].concat(this.obsLog);
    this.obsLog = next.slice(0, AREA_LOG_MAX);
  }

  /**
   * 对照容器当前边长。
   */
  frameEdge(): number {
    return this.frameExpanded ? FRAME_MAX_VP : FRAME_MIN_VP;
  }

  /**
   * 观察卡当前宽。
   */
  obsWidth(): number {
    return this.obsShrink ? OBS_WIDE_VP : OBS_BASE_VP;
  }

  /**
   * 观察卡当前高。
   */
  obsHeight(): number {
    return this.obsShrink ? OBS_SHORT_VP : OBS_BASE_VP;
  }

  /**
   * 组合卡 onAreaChange 记录。
   */
  onComboArea(index: number, newArea: Area): void {
    this.comboAreas[index] = '实测 ' + this.areaToStr(newArea);
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('autoResize 与 fitOriginalSize 行为对比')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        // 图源控制卡
        Column({ space: 8 }) {
          Text('图源与控制').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Row({ space: 10 }) {
            Button('重建图源').fontSize(BUTTON_FONT_SIZE).layoutWeight(1).onClick(() => {
              this.createSources();
            })
            Button(this.frameExpanded ? '容器收缩120vp' : '容器展开280vp')
              .fontSize(BUTTON_FONT_SIZE).layoutWeight(1).onClick(() => {
                this.frameExpanded = !this.frameExpanded;
              })
          }.width('100%')
          Text(this.srcStatus).fontSize(DESC_FONT_SIZE)
          Text('autoResize的降采样效果发生在解码阶段，组件树上看不到图源尺寸变化，' +
            '需结合内存工具或画质差异间接观察；本页用onAreaChange记录组件侧尺寸。')
            .fontSize(DESC_FONT_SIZE)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // autoResize 双图对照卡
        Column({ space: 8 }) {
          Text('autoResize(true) vs autoResize(false)').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Row({ space: 12 }) {
            Column({ space: 4 }) {
              Image(this.bigMap)
                .autoResize(true)
                .objectFit(ImageFit.Cover)
                .width(this.frameEdge())
                .height(this.frameEdge())
                .borderWidth(1)
                .onAreaChange((oldArea: Area, newArea: Area) => {
                  this.areaTextA = '组件尺寸: ' + this.areaToStr(newArea);
                })
              Text('autoResize(true) 按显示区域降采样解码，省内存').fontSize(DESC_FONT_SIZE)
              Text(this.areaTextA).fontSize(DESC_FONT_SIZE).fontColor(Color.Gray)
            }
            Column({ space: 4 }) {
              Image(this.bigMap)
                .autoResize(false)
                .objectFit(ImageFit.Cover)
                .width(this.frameEdge())
                .height(this.frameEdge())
                .borderWidth(1)
                .onAreaChange((oldArea: Area, newArea: Area) => {
                  this.areaTextB = '组件尺寸: ' + this.areaToStr(newArea);
                })
              Text('autoResize(false) 按原图尺寸解码，画质更稳').fontSize(DESC_FONT_SIZE)
              Text(this.areaTextB).fontSize(DESC_FONT_SIZE).fontColor(Color.Gray)
            }
          }.width('100%')
          .justifyContent(FlexAlign.SpaceEvenly)
          Text('文档例：原图800x1200、显示区200x200时，autoResize(true)会降采样到约200x300解码，' +
            '大幅节省内存；缩小显示建议 autoResize(false)+interpolation(Medium) 保画质。')
            .fontSize(DESC_FONT_SIZE)
            .fontColor(Color.Gray)
          Text('本页内存账：320x320全解码=' +
            (BIG_EDGE_PX * BIG_EDGE_PX * BYTES_PER_PIXEL / BYTES_PER_KB).toFixed(0) +
            'KB；收缩到' + FRAME_MIN_VP + 'vp档且autoResize(true)时，解码目标按显示区域折算，内存同步下降。')
            .fontSize(DESC_FONT_SIZE)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // onAreaChange 观察卡
        Column({ space: 8 }) {
          Text('onAreaChange 观察动态尺寸').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Row({ space: 10 }) {
            Button(this.obsShrink ? '恢复150x150' : '改为240x90')
              .fontSize(BUTTON_FONT_SIZE).layoutWeight(1).onClick(() => {
                this.obsShrink = !this.obsShrink;
              })
            Button('autoResize=' + (this.autoOn ? 'true' : 'false'))
              .fontSize(BUTTON_FONT_SIZE).layoutWeight(1).onClick(() => {
                this.autoOn = !this.autoOn;
              })
            Button('清空日志')
              .fontSize(BUTTON_FONT_SIZE).layoutWeight(1).onClick(() => {
                this.obsLog = [];
              })
          }.width('100%')
          Image(this.bigMap)
            .autoResize(this.autoOn)
            .objectFit(ImageFit.Contain)
            .width(this.obsWidth())
            .height(this.obsHeight())
            .borderWidth(1)
            .onAreaChange((oldArea: Area, newArea: Area) => {
              this.obsAreaText = '组件尺寸: ' + this.areaToStr(newArea);
              this.pushLog(this.areaToStr(oldArea) + ' → ' + this.areaToStr(newArea));
            })
          Text(this.obsAreaText).fontSize(STATUS_FONT_SIZE)
          Text('尺寸变化日志(最近' + AREA_LOG_MAX + '条):').fontSize(DESC_FONT_SIZE).fontColor(Color.Gray)
          ForEach(this.obsLog, (line: string, index: number) => {
            Text((index + 1) + '. ' + line).fontSize(DESC_FONT_SIZE).width('100%')
          }, (line: string, index: number): string => index.toString() + line)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        .alignItems(HorizontalAlign.Start)

        // fitOriginalSize 对照卡
        Column({ space: 8 }) {
          Text('fitOriginalSize 三态对照').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Button(this.fitOrigOn ? '关闭左图fitOriginalSize' : '开启左图fitOriginalSize')
            .fontSize(BUTTON_FONT_SIZE).onClick(() => {
              this.fitOrigOn = !this.fitOrigOn;
            })
          Row({ space: 10 }) {
            Column({ space: 4 }) {
              Image(this.smallMap)
                .fitOriginalSize(this.fitOrigOn)
                .borderWidth(1)
                .onAreaChange((oldArea: Area, newArea: Area) => {
                  this.fitAreaText = this.areaToStr(newArea);
                })
              Text('未设宽高+fitOriginalSize(' + (this.fitOrigOn ? 'true' : 'false') + ')')
                .fontSize(DESC_FONT_SIZE)
              Text('实测: ' + this.fitAreaText).fontSize(DESC_FONT_SIZE).fontColor(Color.Gray)
            }.layoutWeight(1)
            Column({ space: 4 }) {
              Image(this.smallMap)
                .borderWidth(1)
                .onAreaChange((oldArea: Area, newArea: Area) => {
                  this.plainAreaText = this.areaToStr(newArea);
                })
              Text('未设宽高+默认(false)')
                .fontSize(DESC_FONT_SIZE)
              Text('实测: ' + this.plainAreaText).fontSize(DESC_FONT_SIZE).fontColor(Color.Gray)
            }.layoutWeight(1)
            Column({ space: 4 }) {
              Image(this.smallMap)
                .width(FIT_ROW_VP / 3)
                .height(FIT_ROW_VP / 3)
                .fitOriginalSize(true)
                .objectFit(ImageFit.Contain)
                .borderWidth(1)
                .onAreaChange((oldArea: Area, newArea: Area) => {
                  this.fixedAreaText = this.areaToStr(newArea);
                })
              Text('已设宽高+fitOriginalSize(true)不生效')
                .fontSize(DESC_FONT_SIZE)
              Text('实测: ' + this.fixedAreaText).fontSize(DESC_FONT_SIZE).fontColor(Color.Gray)
            }.layoutWeight(1)
          }.width('100%')
          Text('文档：fitOriginalSize(true)时组件显示大小跟随图源大小(96x64)；' +
            '默认不跟随；已设置width、height属性时该属性不生效。')
            .fontSize(DESC_FONT_SIZE)
            .fontColor(Color.Gray)
          Text('onAreaChange报告的宽高为组件布局结果(vp)：fitOriginalSize跟随图源像素尺寸换算显示，' +
            '与屏幕density相关，因此实测值可能与96x64不完全相等。')
            .fontSize(DESC_FONT_SIZE)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // autoResize x interpolation 组合对照卡
        Column({ space: 8 }) {
          Text('缩小显示质量组合(autoResize x interpolation)').fontSize(TITLE_FONT_SIZE)
            .fontColor(Color.Gray)
          Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.SpaceBetween }) {
            ForEach(RESIZE_COMBOS, (combo: ResizeCombo, index: number) => {
              Column({ space: 4 }) {
                Image(this.bigMap)
                  .autoResize(combo.auto)
                  .interpolation(combo.interp)
                  .objectFit(ImageFit.Cover)
                  .width(COMBO_ITEM_VP)
                  .height(COMBO_ITEM_VP)
                  .borderWidth(1)
                  .onAreaChange((oldArea: Area, newArea: Area) => {
                    this.onComboArea(index, newArea);
                  })
                Text(combo.label).fontSize(DESC_FONT_SIZE)
                  .maxLines(2)
                  .textOverflow({ overflow: TextOverflow.Ellipsis })
                Text(this.comboAreas[index]).fontSize(DESC_FONT_SIZE).fontColor(Color.Gray)
              }
              .width('49%')
              .margin({ bottom: 8 })
              .alignItems(HorizontalAlign.Start)
            }, (combo: ResizeCombo): string => combo.id)
          }.width('100%')
          Text('320px源缩小到' + COMBO_ITEM_VP + 'vp：降采样解码(autoResize true)叠加最近邻/双线性时' +
            '方环边缘更易断裂，文档建议的 autoResize(false)+Medium 组合边缘最干净。')
            .fontSize(DESC_FONT_SIZE)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // SVG 对照卡
        Column({ space: 8 }) {
          Text('SVG源 autoResize 不生效对照').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Row({ space: 12 }) {
            Image($r('app.media.heart'))
              .autoResize(true)
              .width(100)
              .height(100)
              .borderWidth(1)
            Image($r('app.media.heart'))
              .autoResize(false)
              .width(100)
              .height(100)
              .borderWidth(1)
          }.width('100%')
          .justifyContent(FlexAlign.SpaceEvenly)
          Text('文档：autoResize 对 SVG 与 AnimatedDrawableDescriptor 图源不生效，' +
            'SVG 始终按矢量指令在显示尺寸上重绘，两图无差异。')
            .fontSize(DESC_FONT_SIZE)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Text('注意：autoResize默认false(文档)；降采样解码会丢失部分信息可能产生锯齿，' +
          '追求缩小显示画质时可 autoResize(false)+interpolation(Medium)，' +
          '追求内存时可 autoResize(true)并观察onAreaChange确认组件尺寸变化。')
          .fontSize(12)
          .fontColor(Color.Blue)
          .margin({ bottom: 20 })
      }.width('100%').padding({ left: 20, right: 20 })
    }.width('100%').height('100%').backgroundColor(Color.White)
  }
}
