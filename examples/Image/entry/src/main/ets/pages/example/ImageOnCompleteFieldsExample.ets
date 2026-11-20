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
 * Image onComplete 回调字段全拆解示例
 *
 * 演示内容：
 * 1. onComplete 事件对象全部 9 个字段的实时取值：
 *    width/height(图源尺寸px)、componentWidth/componentHeight(组件尺寸px)、
 *    loadingStatus(0=数据加载成功 1=解码成功)、
 *    contentWidth/contentHeight/contentOffsetX/contentOffsetY(API10+，仅解码成功时有效)；
 * 2. 切换图源：同步 $r SVG 资源 vs 代码生成的异步 PixelMap，观察回调尺寸差异；
 * 3. 切换 syncLoad(8+) 模拟"加载模式"：同步/异步加载下回调到达次序与次数统计；
 * 4. 切换 objectFit 观察 contentWidth/contentHeight/contentOffset* 的联动变化。
 *
 * 对应文档：ts-basic-components-image.md #onComplete
 * onComplete(callback: (event?: { width, height, componentWidth, componentHeight,
 *   loadingStatus, contentWidth, contentHeight, contentOffsetX, contentOffsetY }) => void)
 */
import { image } from '@kit.ImageKit';
import { BusinessError } from '@kit.BasicServicesKit';

/**
 * onComplete 事件对象（与文档字段一一对应）。
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
 * 字段状态卡的一行数据。
 */
interface FieldRow {
  name: string;
  value: string;
  unit: string;
  since: string;
  desc: string;
}

/** PixelMap 图源宽（px） */
const PM_SRC_WIDTH: number = 180;
/** PixelMap 图源高（px） */
const PM_SRC_HEIGHT: number = 120;
/** 主预览组件边长（vp） */
const PREVIEW_VP: number = 150;
/** RGBA_8888 每像素字节数 */
const BYTES_PER_PIXEL: number = 4;
/** 字节转 KB 除数 */
const BYTES_PER_KB: number = 1024;
/** loadingStatus=0 含义 */
const STATUS_DATA_OK: string = '0 = 图片数据加载成功';
/** loadingStatus=1 含义 */
const STATUS_DECODE_OK: string = '1 = 图片解码成功';
/** 未回调占位 */
const NOT_FIRED: string = '--';
/** 字段名字号 */
const FIELD_FONT_SIZE: number = 14;
/** 说明文字字号 */
const DESC_FONT_SIZE: number = 12;
/** 标题字号 */
const TITLE_FONT_SIZE: number = 16;
/** 状态字号 */
const STATUS_FONT_SIZE: number = 14;

/** objectFit 取值序列，用于观察 content* 字段变化 */
const FIT_VALUES: ImageFit[] = [ImageFit.Contain, ImageFit.Cover, ImageFit.Fill, ImageFit.None];
/** 与 FIT_VALUES 对应的展示名 */
const FIT_NAMES: string[] = ['Contain', 'Cover', 'Fill', 'None'];
/** 与 FIT_VALUES 对应的文档释义 */
const FIT_DESCS: string[] = [
  '保持宽高比缩放至完全显示',
  '保持宽高比缩放至完全覆盖(默认Cover)',
  '不保持宽高比拉伸填满',
  '不缩放，配合sourceSize可显示小图'
];

@Entry
@Component
struct ImageOnCompleteFieldsExample {
  /** 代码生成的 PixelMap 图源 */
  @State pixelMapSrc: image.PixelMap | undefined = undefined;
  /** true=使用PixelMap图源，false=使用$r SVG图源 */
  @State usePixelSrc: boolean = false;
  /** 加载模式：true=syncLoad同步，false=异步(默认) */
  @State syncOn: boolean = false;
  /** 当前 objectFit 档位 */
  @State fitIndex: number = 0;
  /** 回调字段实时值 */
  @State evWidth: number = 0;
  @State evHeight: number = 0;
  @State evCompWidth: number = 0;
  @State evCompHeight: number = 0;
  @State evLoadingStatus: number = -1;
  @State evContentWidth: number = 0;
  @State evContentHeight: number = 0;
  @State evOffsetX: number = 0;
  @State evOffsetY: number = 0;
  /** 回调统计 */
  @State completeCount: number = 0;
  @State dataCount: number = 0;
  @State decodeCount: number = 0;
  /** 图源状态 */
  @State srcStatus: string = '正在创建PixelMap图源...';
  /** Contain对照图回调摘要 */
  @State cmpContainText: string = NOT_FIRED;
  /** None对照图回调摘要 */
  @State cmpNoneText: string = NOT_FIRED;

  async aboutToAppear(): Promise<void> {
    await this.createPixelMapSource();
  }

  /**
   * 生成横向渐变 + 半透明条纹的 PixelMap，尺寸 180x120。
   */
  async createPixelMapSource(): Promise<void> {
    const buffer: ArrayBuffer = new ArrayBuffer(PM_SRC_WIDTH * PM_SRC_HEIGHT * BYTES_PER_PIXEL);
    const data: Uint8Array = new Uint8Array(buffer);
    for (let y: number = 0; y < PM_SRC_HEIGHT; y++) {
      for (let x: number = 0; x < PM_SRC_WIDTH; x++) {
        const idx: number = (y * PM_SRC_WIDTH + x) * BYTES_PER_PIXEL;
        data[idx] = Math.floor(255 * x / (PM_SRC_WIDTH - 1));
        data[idx + 1] = Math.floor(255 * y / (PM_SRC_HEIGHT - 1));
        data[idx + 2] = 160;
        data[idx + 3] = (x % 20 < 10) ? 255 : 128;
      }
    }
    const size: image.Size = { width: PM_SRC_WIDTH, height: PM_SRC_HEIGHT };
    const opts: image.InitializationOptions = {
      size: size,
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      scaleMode: image.ScaleMode.FIT_TARGET_SIZE,
      editable: false
    };
    try {
      this.pixelMapSrc = await image.createPixelMap(buffer, opts);
      const memKb: number = PM_SRC_WIDTH * PM_SRC_HEIGHT * BYTES_PER_PIXEL / BYTES_PER_KB;
      this.srcStatus = 'PixelMap图源就绪 ' + PM_SRC_WIDTH + 'x' + PM_SRC_HEIGHT +
        'px，占用约 ' + memKb.toFixed(1) + ' KB';
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      this.srcStatus = 'PixelMap创建失败 code=' + err.code;
    }
  }

  /**
   * 主预览 onComplete：拆解全部字段并统计回调次数。
   */
  handleComplete(event?: CompleteEventFields): void {
    if (event === undefined) {
      return;
    }
    this.completeCount += 1;
    this.evWidth = event.width;
    this.evHeight = event.height;
    this.evCompWidth = event.componentWidth;
    this.evCompHeight = event.componentHeight;
    this.evLoadingStatus = event.loadingStatus;
    this.evContentWidth = event.contentWidth;
    this.evContentHeight = event.contentHeight;
    this.evOffsetX = event.contentOffsetX;
    this.evOffsetY = event.contentOffsetY;
    if (event.loadingStatus === 0) {
      this.dataCount += 1;
    } else if (event.loadingStatus === 1) {
      this.decodeCount += 1;
    }
  }

  /**
   * 轮换 objectFit 档位。
   */
  cycleFit(): void {
    this.fitIndex = (this.fitIndex + 1) % FIT_VALUES.length;
  }

  /**
   * 重置所有字段与统计。
   */
  resetFields(): void {
    this.evWidth = 0;
    this.evHeight = 0;
    this.evCompWidth = 0;
    this.evCompHeight = 0;
    this.evLoadingStatus = -1;
    this.evContentWidth = 0;
    this.evContentHeight = 0;
    this.evOffsetX = 0;
    this.evOffsetY = 0;
    this.completeCount = 0;
    this.dataCount = 0;
    this.decodeCount = 0;
    this.cmpContainText = NOT_FIRED;
    this.cmpNoneText = NOT_FIRED;
  }

  /**
   * loadingStatus 数值 → 文档语义。
   */
  statusText(): string {
    if (this.evLoadingStatus === 0) {
      return STATUS_DATA_OK;
    }
    if (this.evLoadingStatus === 1) {
      return STATUS_DECODE_OK;
    }
    return NOT_FIRED;
  }

  /**
   * 数值字段格式化（未回调显示占位符）。
   */
  fmt(value: number): string {
    return this.evLoadingStatus < 0 ? NOT_FIRED : value.toFixed(0);
  }

  /**
   * 偏移字段带符号格式化（可为负值）。
   */
  fmtSigned(value: number): string {
    return this.evLoadingStatus < 0 ? NOT_FIRED : value.toFixed(1);
  }

  /**
   * 生成 9 个字段的展示行（逻辑在方法内，不写在 build 里）。
   */
  getFieldRows(): FieldRow[] {
    const rows: FieldRow[] = [
      {
        name: 'width',
        value: this.fmt(this.evWidth),
        unit: 'px',
        since: 'API7',
        desc: '图片的宽(图源尺寸)'
      },
      {
        name: 'height',
        value: this.fmt(this.evHeight),
        unit: 'px',
        since: 'API7',
        desc: '图片的高(图源尺寸)'
      },
      {
        name: 'componentWidth',
        value: this.fmt(this.evCompWidth),
        unit: 'px',
        since: 'API7',
        desc: '组件的宽'
      },
      {
        name: 'componentHeight',
        value: this.fmt(this.evCompHeight),
        unit: 'px',
        since: 'API7',
        desc: '组件的高'
      },
      {
        name: 'loadingStatus',
        value: this.evLoadingStatus < 0 ? NOT_FIRED : this.evLoadingStatus.toFixed(0),
        unit: '',
        since: 'API7',
        desc: this.statusText()
      },
      {
        name: 'contentWidth',
        value: this.fmt(this.evContentWidth),
        unit: 'px',
        since: 'API10+',
        desc: '实际绘制宽度，仅loadingStatus=1有效'
      },
      {
        name: 'contentHeight',
        value: this.fmt(this.evContentHeight),
        unit: 'px',
        since: 'API10+',
        desc: '实际绘制高度，仅loadingStatus=1有效'
      },
      {
        name: 'contentOffsetX',
        value: this.fmtSigned(this.evOffsetX),
        unit: 'px',
        since: 'API10+',
        desc: '绘制内容相对组件的x偏移'
      },
      {
        name: 'contentOffsetY',
        value: this.fmtSigned(this.evOffsetY),
        unit: 'px',
        since: 'API10+',
        desc: '绘制内容相对组件的y偏移'
      }
    ];
    return rows;
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('onComplete 返回值全字段拆解')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        // 图源与加载模式控制卡
        Column({ space: 8 }) {
          Text('图源 / 加载模式 / 填充模式切换').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Row({ space: 8 }) {
            Button(this.usePixelSrc ? '切到$r SVG源' : '切到PixelMap源')
              .fontSize(13).layoutWeight(1).onClick(() => {
                this.usePixelSrc = !this.usePixelSrc;
                this.resetFields();
              })
            Button(this.syncOn ? '改异步加载' : '改同步加载syncLoad')
              .fontSize(13).layoutWeight(1).onClick(() => {
                this.syncOn = !this.syncOn;
                this.resetFields();
              })
          }.width('100%')
          Row({ space: 8 }) {
            Button('轮换objectFit: ' + FIT_NAMES[this.fitIndex])
              .fontSize(13).layoutWeight(1).onClick(() => {
                this.cycleFit();
              })
            Button('重置字段')
              .fontSize(13).layoutWeight(1).onClick(() => {
                this.resetFields();
              })
          }.width('100%')
          Image(this.usePixelSrc ? this.pixelMapSrc : $r('app.media.icon'))
            .syncLoad(this.syncOn)
            .objectFit(FIT_VALUES[this.fitIndex])
            .width(PREVIEW_VP)
            .height(PREVIEW_VP)
            .borderWidth(1)
            .onComplete((event?: CompleteEventFields) => {
              this.handleComplete(event);
            })
          Text('当前图源: ' + (this.usePixelSrc ? 'PixelMap 180x120' : 'app.media.icon(SVG)') +
            ' | 加载模式: ' + (this.syncOn ? '同步syncLoad(8+)' : '异步(默认)') +
            ' | 填充: ' + FIT_NAMES[this.fitIndex]).fontSize(DESC_FONT_SIZE)
          Text('填充说明: ' + FIT_DESCS[this.fitIndex]).fontSize(DESC_FONT_SIZE).fontColor(Color.Gray)
          Text(this.srcStatus).fontSize(DESC_FONT_SIZE)
          Text('切换图源或加载模式会自动重置字段与计数，便于对照不同来源的回调差异。')
            .fontSize(DESC_FONT_SIZE)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 字段状态卡
        Column({ space: 6 }) {
          Text('回调字段实时值').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Row({ space: 8 }) {
            Text('字段').fontSize(DESC_FONT_SIZE).fontWeight(FontWeight.Bold).width(128)
            Text('值').fontSize(DESC_FONT_SIZE).fontWeight(FontWeight.Bold).width(64)
            Text('单位').fontSize(DESC_FONT_SIZE).fontWeight(FontWeight.Bold).width(36)
            Text('版本').fontSize(DESC_FONT_SIZE).fontWeight(FontWeight.Bold).width(56)
            Text('含义').fontSize(DESC_FONT_SIZE).fontWeight(FontWeight.Bold).layoutWeight(1)
          }.width('100%')
          Divider()
          ForEach(this.getFieldRows(), (row: FieldRow) => {
            Row({ space: 8 }) {
              Text(row.name).fontSize(FIELD_FONT_SIZE).width(128)
              Text(row.value).fontSize(FIELD_FONT_SIZE).width(64).fontColor(Color.Orange)
              Text(row.unit).fontSize(FIELD_FONT_SIZE).width(36).fontColor(Color.Gray)
              Text(row.since).fontSize(DESC_FONT_SIZE).width(56).fontColor(Color.Gray)
              Text(row.desc).fontSize(DESC_FONT_SIZE).layoutWeight(1).fontColor(Color.Gray)
                .maxLines(2)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
            }.width('100%')
          }, (row: FieldRow): string => row.name)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // content* 字段双图对照卡
        Column({ space: 8 }) {
          Text('content* 字段双图对照(同一图源不同填充)').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Row({ space: 12 }) {
            Column({ space: 4 }) {
              Image(this.usePixelSrc ? this.pixelMapSrc : $r('app.media.icon'))
                .objectFit(ImageFit.Contain)
                .width(PREVIEW_VP)
                .height(PREVIEW_VP)
                .borderWidth(1)
                .onComplete((event?: CompleteEventFields) => {
                  if (event !== undefined && event.loadingStatus === 1) {
                    this.cmpContainText = 'content ' + event.contentWidth.toFixed(0) + 'x' +
                      event.contentHeight.toFixed(0) + ' 偏移 ' + event.contentOffsetX.toFixed(0) + '/' +
                      event.contentOffsetY.toFixed(0);
                  }
                })
              Text('Contain 保持宽高比').fontSize(DESC_FONT_SIZE).fontColor(Color.Gray)
              Text(this.cmpContainText).fontSize(DESC_FONT_SIZE)
            }
            Column({ space: 4 }) {
              Image(this.usePixelSrc ? this.pixelMapSrc : $r('app.media.icon'))
                .objectFit(ImageFit.None)
                .width(PREVIEW_VP)
                .height(PREVIEW_VP)
                .borderWidth(1)
                .onComplete((event?: CompleteEventFields) => {
                  if (event !== undefined && event.loadingStatus === 1) {
                    this.cmpNoneText = 'content ' + event.contentWidth.toFixed(0) + 'x' +
                      event.contentHeight.toFixed(0) + ' 偏移 ' + event.contentOffsetX.toFixed(0) + '/' +
                      event.contentOffsetY.toFixed(0);
                  }
                })
              Text('None 不缩放原尺寸绘制').fontSize(DESC_FONT_SIZE).fontColor(Color.Gray)
              Text(this.cmpNoneText).fontSize(DESC_FONT_SIZE)
            }
          }.width('100%')
          .justifyContent(FlexAlign.SpaceEvenly)
          Text('Contain下contentWidth/Height按图源宽高比适配进组件，留边方向产生偏移；' +
            'None下content即图源原始尺寸。')
            .fontSize(DESC_FONT_SIZE)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // loadingStatus 统计卡
        Column({ space: 6 }) {
          Text('回调统计(每次切换图源/模式后重新计数)').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Text('onComplete总回调次数: ' + this.completeCount).fontSize(STATUS_FONT_SIZE)
          Text('loadingStatus=0(数据加载成功): ' + this.dataCount + ' 次').fontSize(STATUS_FONT_SIZE)
          Text('loadingStatus=1(解码成功): ' + this.decodeCount + ' 次').fontSize(STATUS_FONT_SIZE)
          Text('最新状态: ' + this.statusText()).fontSize(STATUS_FONT_SIZE)
            .fontColor(this.evLoadingStatus >= 0 ? Color.Green : Color.Gray)
          Text('文档：图片数据加载成功和解码成功时均触发该回调，' +
            '因此同一张图通常至少回调两次(0与1各一次)。')
            .fontSize(DESC_FONT_SIZE)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        .alignItems(HorizontalAlign.Start)

        // 字段版本说明卡
        Column({ space: 6 }) {
          Text('字段版本与单位速查').fontSize(TITLE_FONT_SIZE).fontColor(Color.Gray)
          Text('1. width/height/componentWidth/componentHeight/loadingStatus：单位px，卡片自API9支持。')
            .fontSize(DESC_FONT_SIZE)
          Text('2. contentWidth/contentHeight/contentOffsetX/contentOffsetY：API10+新增，' +
            '仅在loadingStatus=1(解码成功)时有效。')
            .fontSize(DESC_FONT_SIZE)
          Text('3. 图源为PixelMap时width/height即PixelMap创建尺寸；SVG图源按矢量内容尺寸上报。')
            .fontSize(DESC_FONT_SIZE)
          Text('4. 组件参数为AnimatedDrawableDescriptor时该事件不触发(文档)。')
            .fontSize(DESC_FONT_SIZE)
          Text('5. 同步加载(syncLoad=true)与异步加载均会回调，回调时机差异体现在首帧而非字段值。')
            .fontSize(DESC_FONT_SIZE)
          Text('6. 组件尺寸字段为px单位：vp×densityPixels≈px，与width()/height()设置的vp值不同。')
            .fontSize(DESC_FONT_SIZE)
          Text('7. 同一图源不同objectFit下：width/height不变，contentWidth/Height与offset随填充策略变化。')
            .fontSize(DESC_FONT_SIZE)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        .alignItems(HorizontalAlign.Start)

        Text('提示：本页仅拆解文档定义的9个回调字段；加载模式通过syncLoad(8+)与loadingStatus组合观察，' +
          'contentOffset*在Contain/Cover留边场景下出现非0值。')
          .fontSize(12)
          .fontColor(Color.Blue)
          .margin({ bottom: 20 })
      }.width('100%').padding({ left: 20, right: 20 })
    }.width('100%').height('100%').backgroundColor(Color.White)
  }
}
