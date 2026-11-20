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
 * ImageHdrDynamicRangeExample.ets
 *
 * 主题:dynamicRangeMode<12+> 与 hdrBrightness<19+> 的 HDR 动态范围组合。
 * 覆盖点:
 *  1. DynamicRangeMode 枚举(ts-basic-components-image.md#DynamicRangeMode枚举说明):
 *     HIGH=0 不受限提亮、CONSTRAINT=1 受限提亮、STANDARD=2 标准不提亮(默认)。
 *  2. hdrBrightness(brightness: number):取值范围 [0.0, 1.0],越界取 1.0;
 *     0 表示按 SDR 亮度显示(相当于不提亮,Below SDR 一档),
 *     1.0 表示按当前允许的最高 HDR 亮度显示(High/Conforming 一档),
 *     0.5 为中间档。文档中该属性入参为 number,并无枚举类型。
 *  3. 3x3 组合矩阵:3 个 DynamicRangeMode x 3 个 hdrBrightness 取值,
 *     观察文档所述交互规则——两者同时设置时 dynamicRangeMode 不生效。
 *  4. onComplete 事件字段观察:loadingStatus(0 加载成功/1 解码成功)、
 *     contentWidth/contentHeight(10+,仅 loadingStatus=1 时有效),
 *     用于确认 PixelMap 源图的加载与回退路径。
 *     注:contentLoadStatus/loadMode 字段在本文档版本中未找到,故不使用。
 *  5. 图源全部由代码生成 PixelMap(SDR 模拟),imageInfo.isHdr 为 false;
 *     真机 HDR 提亮效果受设备(Phone/PC/2in1/Tablet 生效)与真实 HDR 图源限制。
 */
import { image } from '@kit.ImageKit';
import { BusinessError } from '@kit.BasicServicesKit';

/** 动态范围枚举元数据 */
interface DrmMeta {
  name: string;
  enumValue: number;
  meaning: string;
  mode: DynamicRangeMode;
}

/** 组合矩阵单元格 */
interface ComboCellItem {
  tag: string;
  rowIdx: number;
  colIdx: number;
  drmName: string;
  brightness: number;
  brightLabel: string;
}

/** hdrBrightness 亮度档位(文档入参为 number,此处仅为展示标签) */
interface BrightMeta {
  value: number;
  label: string;
  meaning: string;
}

/** 模拟图源边长(px) */
const CELL_MAP_SIZE: number = 72;

/** 3 个 DynamicRangeMode 枚举行 */
const DRM_ROWS: DrmMeta[] = [
  {
    name: 'HIGH',
    enumValue: 0,
    meaning: '不受限动态范围,最大限度进行图片提亮',
    mode: DynamicRangeMode.HIGH
  },
  {
    name: 'CONSTRAINT',
    enumValue: 1,
    meaning: '受限动态范围,受限进行图片提亮',
    mode: DynamicRangeMode.CONSTRAINT
  },
  {
    name: 'STANDARD',
    enumValue: 2,
    meaning: '标准动态范围,不进行图片提亮(默认值)',
    mode: DynamicRangeMode.STANDARD
  }
];

/** 3 个 hdrBrightness 代表亮度列:0=SDR 档、0.5=中间档、1.0=最高 HDR 档 */
const BRIGHT_COLS: BrightMeta[] = [
  { value: 0.0, label: '0.0', meaning: '按SDR亮度显示' },
  { value: 0.5, label: '0.5', meaning: '中间亮度档' },
  { value: 1.0, label: '1.0', meaning: '当前允许的最高HDR亮度' }
];

/** 越界亮度样本:文档规定小于 0 与大于 1.0 时均取 1.0 */
const CLAMP_VALUES: number[] = [-0.5, 1.6, 1.0];

/** 组合矩阵观察要点清单 */
const OBSERVE_NOTES: string[] = [
  '同一行(同 hdrBrightness、不同 DynamicRangeMode)理论无视觉差异:两者同设时 dynamicRangeMode 被抑制。',
  '同一列(同亮度、不同模式标签)用于交叉验证上述抑制规则是否成立。',
  '仅设 dynamicRangeMode 的三档卡:HIGH 提亮幅度最大,STANDARD 不提亮,CONSTRAINT 介于两者之间。',
  '仅设 hdrBrightness 时:0 与 1.0 差异只会在真实 HDR 图源上呈现,SDR 模拟源预期无变化。'
];

/** 生成 3x3 组合单元格清单 */
function buildComboCells(): ComboCellItem[] {
  const cells: ComboCellItem[] = [];
  for (let r: number = 0; r < DRM_ROWS.length; r++) {
    for (let c: number = 0; c < BRIGHT_COLS.length; c++) {
      cells.push({
        tag: 'cell-' + r + '-' + c,
        rowIdx: r,
        colIdx: c,
        drmName: DRM_ROWS[r].name,
        brightness: BRIGHT_COLS[c].value,
        brightLabel: BRIGHT_COLS[c].label
      });
    }
  }
  return cells;
}

/** 预切分行单元格,避免 build 内做过滤运算:rowCells[r] 即第 r 行 3 个单元格 */
function splitRowCells(cells: ComboCellItem[]): ComboCellItem[][] {
  const rows: ComboCellItem[][] = [[], [], []];
  for (let i: number = 0; i < cells.length; i++) {
    rows[cells[i].rowIdx].push(cells[i]);
  }
  return rows;
}

/**
 * 生成模拟 HDR 高亮内容的 PixelMap 缓冲:
 * 中心放射渐变,亮度按 boost 系数放大到接近满量程,便于观察提亮差异。
 */
function makeRadialBuffer(size: number, boost: number): ArrayBuffer {
  const buffer: ArrayBuffer = new ArrayBuffer(size * size * 4);
  const data: Uint8Array = new Uint8Array(buffer);
  const center: number = (size - 1) / 2;
  const maxDist: number = Math.sqrt(2) * center;
  for (let y: number = 0; y < size; y++) {
    for (let x: number = 0; x < size; x++) {
      const idx: number = (y * size + x) * 4;
      const dx: number = x - center;
      const dy: number = y - center;
      const dist: number = Math.sqrt(dx * dx + dy * dy);
      const base: number = 255 * (1 - dist / maxDist);
      const v: number = Math.min(255, Math.floor(base * boost));
      data[idx] = v;
      data[idx + 1] = Math.floor(v * 0.82);
      data[idx + 2] = Math.floor(70 + v * 0.35);
      data[idx + 3] = 255;
    }
  }
  return buffer;
}

@Entry
@Component
struct ImageHdrDynamicRangeExample {
  @State sdrMap: image.PixelMap | undefined = undefined;
  @State brightMap: image.PixelMap | undefined = undefined;
  @State liveBright: number = 1.0;
  @State comboMode: number = 0;
  @State completeText: string = 'onComplete 尚未触发';
  @State hdrFlag: string = 'isHdr 未读取';
  private rowCells: ComboCellItem[][] = splitRowCells(buildComboCells());
  private comboNames: string[] = ['仅 dynamicRangeMode(HIGH)', '仅 hdrBrightness(0.5)', '两者同时设置'];

  async aboutToAppear(): Promise<void> {
    try {
      const opts: image.InitializationOptions = {
        size: { width: CELL_MAP_SIZE, height: CELL_MAP_SIZE },
        srcPixelFormat: image.PixelMapFormat.RGBA_8888,
        pixelFormat: image.PixelMapFormat.RGBA_8888,
        editable: false,
        scaleMode: image.ScaleMode.FIT_TARGET_SIZE
      };
      const sdr: image.PixelMap = await image.createPixelMap(makeRadialBuffer(CELL_MAP_SIZE, 0.75), opts);
      const bright: image.PixelMap = await image.createPixelMap(makeRadialBuffer(CELL_MAP_SIZE, 1.0), opts);
      this.sdrMap = sdr;
      this.brightMap = bright;
      const info: image.ImageInfo = bright.getImageInfoSync();
      this.hdrFlag = '生成图源 ' + info.size.width + 'x' + info.size.height +
        ',isHdr=' + info.isHdr + '(SDR 模拟,提亮效果受限)';
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      this.hdrFlag = '图源创建失败,code=' + err.code;
    }
  }

  /** 循环切换三种组合形态 */
  private cycleComboMode(): void {
    this.comboMode = (this.comboMode + 1) % this.comboNames.length;
  }

  /** 按行索引取 DynamicRangeMode */
  private drmAt(rowIdx: number): DynamicRangeMode {
    return DRM_ROWS[rowIdx].mode;
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('HDR 动态范围组合矩阵')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        // 卡片1:两属性规则速读
        Column({ space: 6 }) {
          Text('属性规则(API 文档对照)').fontSize(16).fontColor(Color.Gray)
          Text('dynamicRangeMode<12+>:HIGH(0) 不受限提亮 / CONSTRAINT(1) 受限提亮 / STANDARD(2) 默认不提亮。')
            .fontSize(12)
          Text('hdrBrightness<19+>:入参 number,范围 [0.0,1.0],越界取 1.0;0=按 SDR 亮度,1.0=当前允许的最高 HDR 亮度。')
            .fontSize(12)
          Text('交互规则:两者同时设置时 dynamicRangeMode 不生效,以 hdrBrightness 为准。')
            .fontSize(12)
            .fontColor(Color.Red)
          Text('设备差异:dynamicRangeMode 在 Phone、PC/2in1、Tablet 生效,其他设备类型无效果;SVG 图源两属性均不支持。')
            .fontSize(12)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        .alignItems(HorizontalAlign.Start)

        // 卡片2:3x3 组合矩阵
        Column({ space: 8 }) {
          Text('3x3 组合:DynamicRangeMode x hdrBrightness').fontSize(16).fontColor(Color.Gray)
          ForEach(DRM_ROWS, (row: DrmMeta, rIdx: number) => {
            Row({ space: 8 }) {
              Text(row.name + '\n(' + row.enumValue + ')')
                .fontSize(11)
                .fontColor(Color.Blue)
                .width(74)
              ForEach(this.rowCells[rIdx],
                (cell: ComboCellItem) => {
                  Column({ space: 2 }) {
                    Image(this.brightMap)
                      .dynamicRangeMode(this.drmAt(cell.rowIdx))
                      .hdrBrightness(cell.brightness)
                      .width(CELL_MAP_SIZE)
                      .height(CELL_MAP_SIZE)
                      .borderWidth(1)
                    Text('b=' + cell.brightLabel).fontSize(10).fontColor(Color.Gray)
                  }
                  .alignItems(HorizontalAlign.Center)
                }, (cell: ComboCellItem): string => cell.tag)
            }
            .width('100%')
            .alignItems(VerticalAlign.Center)
          }, (row: DrmMeta): string => 'row-' + row.name)
          Text('每格同时设置两属性:按文档 dynamicRangeMode 被抑制,列间差异来自 hdrBrightness;行间应无差异。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片3:仅 dynamicRangeMode 三档横排
        Column({ space: 8 }) {
          Text('仅 dynamicRangeMode<12+> 三档').fontSize(16).fontColor(Color.Gray)
          Row({ space: 10 }) {
            ForEach(DRM_ROWS, (row: DrmMeta) => {
              Column({ space: 3 }) {
                Image(this.sdrMap)
                  .dynamicRangeMode(row.mode)
                  .width(84)
                  .height(84)
                  .borderWidth(1)
                Text(row.name).fontSize(11).fontColor(Color.Blue)
                Text(row.meaning).fontSize(9).fontColor(Color.Gray)
                  .maxLines(2).textOverflow({ overflow: TextOverflow.Ellipsis })
              }
              .alignItems(HorizontalAlign.Center)
            }, (row: DrmMeta): string => 'drm-only-' + row.name)
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片3b:缺省基线对照(两属性均不设置 = 默认 STANDARD + hdrBrightness 1.0)
        Column({ space: 8 }) {
          Text('缺省基线对照').fontSize(16).fontColor(Color.Gray)
          Row({ space: 10 }) {
            Column({ space: 3 }) {
              Image(this.brightMap)
                .width(84)
                .height(84)
                .borderWidth(1)
                .borderColor(Color.Gray)
              Text('均不设置(缺省)').fontSize(11)
            }
            Column({ space: 3 }) {
              Image(this.brightMap)
                .dynamicRangeMode(DynamicRangeMode.STANDARD)
                .width(84)
                .height(84)
                .borderWidth(1)
              Text('显式 STANDARD(2)').fontSize(11)
            }
          }
          Text('缺省即 DynamicRangeMode.STANDARD;两格应无差异,用于确认默认值语义。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片4:hdrBrightness 滑杆连续调节
        Column({ space: 8 }) {
          Text('hdrBrightness<19+> 滑杆 0.0→1.0').fontSize(16).fontColor(Color.Gray)
          Row({ space: 12 }) {
            Image(this.brightMap)
              .hdrBrightness(this.liveBright)
              .width(96)
              .height(96)
              .borderWidth(1)
            Column({ space: 6 }) {
              Slider({
                value: this.liveBright,
                min: 0.0,
                max: 1.0,
                step: 0.05,
                style: SliderStyle.OutSet
              })
                .width(160)
                .onChange((value: number) => {
                  this.liveBright = value;
                })
              Text('brightness = ' + this.liveBright.toFixed(2)).fontSize(13)
              Text(this.liveBright <= 0.01 ? '≈ SDR 亮度显示' :
                (this.liveBright >= 0.99 ? '≈ 最高 HDR 亮度(High/Conforming)' : '中间档(Below SDR→HDR 过渡)'))
                .fontSize(11)
                .fontColor(Color.Gray)
            }
          }
          Text('该接口仅对 HDR 图源生效;本例图源为 SDR 模拟 PixelMap,真机上亮度应无变化。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片5:互斥行为三态切换
        Column({ space: 8 }) {
          Text('互斥行为:' + this.comboNames[this.comboMode]).fontSize(16).fontColor(Color.Gray)
          if (this.comboMode === 0) {
            Image(this.brightMap)
              .dynamicRangeMode(DynamicRangeMode.HIGH)
              .width(110)
              .height(110)
              .borderWidth(1)
              .borderColor(Color.Blue)
          } else if (this.comboMode === 1) {
            Image(this.brightMap)
              .hdrBrightness(0.5)
              .width(110)
              .height(110)
              .borderWidth(1)
              .borderColor(Color.Blue)
          } else {
            Image(this.brightMap)
              .dynamicRangeMode(DynamicRangeMode.HIGH)
              .hdrBrightness(0.0)
              .width(110)
              .height(110)
              .borderWidth(1)
              .borderColor(Color.Red)
          }
          Button('下一形态').fontSize(13).onClick(() => {
            this.cycleComboMode();
          })
          Text('第三态两者同设:按文档 dynamicRangeMode 不生效,实际按 hdrBrightness=0.0 的 SDR 亮度显示。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片6:onComplete 加载状态观察(观察图源回退路径)
        Column({ space: 8 }) {
          Text('onComplete 加载观察').fontSize(16).fontColor(Color.Gray)
          Image(this.brightMap)
            .dynamicRangeMode(DynamicRangeMode.CONSTRAINT)
            .width(100)
            .height(100)
            .borderWidth(1)
            .onComplete((event) => {
              this.completeText = 'loadingStatus=' + event.loadingStatus +
                ' 图=' + event.width + 'x' + event.height +
                ' 绘制=' + event.contentWidth + 'x' + event.contentHeight +
                ' 组件=' + event.componentWidth + 'x' + event.componentHeight;
            })
            .onError(() => {
              this.completeText = 'onError 触发:PixelMap 源加载失败';
            })
          Text(this.completeText)
            .fontSize(11)
            .fontColor(Color.Gray)
            .maxLines(2)
            .textOverflow({ overflow: TextOverflow.Ellipsis })
          Button('重置观察状态').fontSize(13).onClick(() => {
            this.completeText = 'onComplete 尚未触发';
          })
          Text('状态字段:loadingStatus 0=数据加载成功 1=解码成功;contentWidth/Height 仅 loadingStatus=1 有效。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片7:亮度档位语义 + 越界钳制对照
        Column({ space: 8 }) {
          Text('hdrBrightness 档位与越界钳制').fontSize(16).fontColor(Color.Gray)
          Row({ space: 10 }) {
            ForEach(BRIGHT_COLS, (col: BrightMeta) => {
              Column({ space: 3 }) {
                Image(this.sdrMap)
                  .hdrBrightness(col.value)
                  .width(76)
                  .height(76)
                  .borderWidth(1)
                Text(col.label).fontSize(11).fontColor(Color.Blue)
                Text(col.meaning).fontSize(9).fontColor(Color.Gray)
                  .maxLines(2).textOverflow({ overflow: TextOverflow.Ellipsis })
              }
              .alignItems(HorizontalAlign.Center)
            }, (col: BrightMeta): string => 'bm-' + col.label)
            ForEach(CLAMP_VALUES, (v: number) => {
              Column({ space: 3 }) {
                Image(this.sdrMap)
                  .hdrBrightness(v)
                  .width(60)
                  .height(60)
                  .borderWidth(1)
                  .borderColor(Color.Gray)
                Text('b=' + v).fontSize(10)
              }
              .alignItems(HorizontalAlign.Center)
            }, (v: number): string => 'clamp-' + v)
          }
          Text('右侧三格入参为 -0.5 / 1.6 / 1.0:文档规定越界取 1.0,三格应与 b=1.0 档一致。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片8:观察要点清单
        Column({ space: 6 }) {
          Text('组合矩阵观察要点').fontSize(16).fontColor(Color.Gray)
          ForEach(OBSERVE_NOTES, (note: string, idx: number) => {
            Text((idx + 1) + '. ' + note)
              .fontSize(11)
              .width('100%')
              .fontColor(Color.Black)
          }, (note: string, idx: number): string => 'note-' + idx)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        .alignItems(HorizontalAlign.Start)

        Text('说明:' + this.hdrFlag + ';真机 HDR 效果需 HDR 图源与设备支持,SDR 模拟源仅验证属性可调用与不崩溃。')
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
}
