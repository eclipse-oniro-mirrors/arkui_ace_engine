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
 * ImageMatrixAntialiasedExample.ets
 *
 * 主题:antialiased<23+> 边缘抗锯齿 x 矩阵放大 x interpolation 插值三方对照。
 * 覆盖点:
 *  1. antialiased(isAntialiased: Optional<boolean>) 三种入参形态:
 *     true(开启边缘抗锯齿)、false(不开启)、undefined(不开启,文档默认行为)。
 *     注:Image 组件文档中 antialiased 入参为 Optional<boolean>,
 *     未定义 LineantialiasingType 枚举(该类型在本文档集 Image 章节未找到,
 *     为避免使用未验证 API,本例严格按文档签名取三态布尔)。
 *  2. 用 imageMatrix<15+>(matrix4 identity().scale)把硬边棋盘/阶梯对角线图源
 *     放大 1/2/4 倍,放大后边缘锯齿在 antialiased 开关下差异最明显。
 *  3. interpolation 三档对照:None(最近邻)/Low(双线性,默认)/Medium(MipMap),
 *     与抗锯齿开关正交,同图同矩阵并排比较。
 *  4. onAreaChange 通用事件:组件尺寸变化时回读宽高并显示。
 *  5. 文档约束:antialiased 仅位图边缘、默认不开启、与 resizable 同时设置不生效、
 *     不影响 backgroundColor 的锯齿、SVG 图源不支持;
 *     interpolation 与 imageMatrix 均不支持 SVG 图源。
 */
import { image } from '@kit.ImageKit';
import { matrix4 } from '@kit.ArkUI';
import { BusinessError } from '@kit.BasicServicesKit';

/** 抗锯齿三态卡片 */
interface AaCardItem {
  label: string;
  value: boolean | undefined;
  valueText: string;
  note: string;
}

/** 插值档位卡片 */
interface InterpCardItem {
  label: string;
  value: ImageInterpolation;
  note: string;
}

/** 放大倍率描述 */
interface ZoomStepItem {
  factor: number;
  caption: string;
}

/** 棋盘图边长(px)与格宽(px),小图放大后格线即硬边样本 */
const CHECKER_SIZE: number = 32;
const CHECKER_CELL: number = 4;
/** 对角阶梯图边长(px) */
const DIAG_SIZE: number = 48;
/** 缩略显示边长(vp) */
const TILE_SIZE: number = 84;

/** 放大倍率序列:1x / 2x / 4x */
const ZOOM_STEPS: ZoomStepItem[] = [
  { factor: 1, caption: '1x 原大' },
  { factor: 2, caption: '2x 放大' },
  { factor: 4, caption: '4x 放大' }
];

/** 预生成与 ZOOM_STEPS 一一对应的矩阵放大对象,避免 build 内构造 */
function createZoomMatrices(): matrix4.Matrix4Transit[] {
  const list: matrix4.Matrix4Transit[] = [];
  for (let i: number = 0; i < ZOOM_STEPS.length; i++) {
    const f: number = ZOOM_STEPS[i].factor;
    list.push(matrix4.identity().scale({ x: f, y: f }));
  }
  return list;
}

/** antialiased 三态卡片数据(Optional<boolean> 的三种合法入参) */
const AA_CARDS: AaCardItem[] = [
  {
    label: 'antialiased(true)',
    value: true,
    valueText: 'true',
    note: '开启位图边缘抗锯齿'
  },
  {
    label: 'antialiased(false)',
    value: false,
    valueText: 'false',
    note: '不开启边缘抗锯齿'
  },
  {
    label: 'antialiased(undefined)',
    value: undefined,
    valueText: 'undefined',
    note: '文档:undefined 时不开启边缘抗锯齿'
  }
];

/** 边缘质量观察要点清单 */
const EDGE_NOTES: string[] = [
  'antialiased 只作用于位图图片边缘,不开启时硬边随放大出现台阶。',
  'interpolation 作用于整幅图的采样方式,None 保硬边、Low 全图发软,与抗锯齿作用域不同。',
  '两者同设时分别生效:抗锯齿管边缘平滑,插值管像素采样,可在联动卡对照。',
  '设置 backgroundColor 后,背景矩形的锯齿不受 antialiased 影响(文档明确说明)。'
];

/** interpolation 三档对照数据(第四档 High=Cubic 见卡片尾注) */
const INTERP_CARDS: InterpCardItem[] = [
  {
    label: 'None',
    value: ImageInterpolation.None,
    note: '最近邻插值,放大后硬边最锐利'
  },
  {
    label: 'Low',
    value: ImageInterpolation.Low,
    note: '双线性插值,默认档位'
  },
  {
    label: 'Medium',
    value: ImageInterpolation.Medium,
    note: 'MipMap 插值,缩小质量更好'
  }
];

/** 生成硬边棋盘 ArrayBuffer:4px 格,黑白相间,alpha 全 255 */
function makeCheckerBuffer(size: number, cell: number): ArrayBuffer {
  const buffer: ArrayBuffer = new ArrayBuffer(size * size * 4);
  const data: Uint8Array = new Uint8Array(buffer);
  for (let y: number = 0; y < size; y++) {
    for (let x: number = 0; x < size; x++) {
      const idx: number = (y * size + x) * 4;
      const dark: boolean = (Math.floor(x / cell) + Math.floor(y / cell)) % 2 === 0;
      const v: number = dark ? 25 : 235;
      data[idx] = v;
      data[idx + 1] = v;
      data[idx + 2] = v;
      data[idx + 3] = 255;
    }
  }
  return buffer;
}

/** 生成 45 度阶梯对角线 ArrayBuffer:单像素台阶线,是抗锯齿最敏感样本 */
function makeDiagonalBuffer(size: number): ArrayBuffer {
  const buffer: ArrayBuffer = new ArrayBuffer(size * size * 4);
  const data: Uint8Array = new Uint8Array(buffer);
  for (let y: number = 0; y < size; y++) {
    for (let x: number = 0; x < size; x++) {
      const idx: number = (y * size + x) * 4;
      data[idx] = 20;
      data[idx + 1] = 20;
      data[idx + 2] = 30;
      data[idx + 3] = 255;
      if (x === y || x === y + 1) {
        data[idx] = 225;
        data[idx + 1] = 120;
        data[idx + 2] = 40;
      }
      if (x === size - 1 - y) {
        data[idx] = 60;
        data[idx + 1] = 200;
        data[idx + 2] = 120;
      }
    }
  }
  return buffer;
}

@Entry
@Component
struct ImageMatrixAntialiasedExample {
  @State checkerMap: image.PixelMap | undefined = undefined;
  @State diagonalMap: image.PixelMap | undefined = undefined;
  @State zoomIdx: number = 2;
  @State aaToggle: boolean = true;
  @State bigBox: boolean = false;
  @State areaText: string = 'onAreaChange 等待回调';
  private zoomMatrices: matrix4.Matrix4Transit[] = createZoomMatrices();

  async aboutToAppear(): Promise<void> {
    try {
      const opts: image.InitializationOptions = {
        size: { width: CHECKER_SIZE, height: CHECKER_SIZE },
        srcPixelFormat: image.PixelMapFormat.RGBA_8888,
        pixelFormat: image.PixelMapFormat.RGBA_8888,
        editable: false,
        scaleMode: image.ScaleMode.FIT_TARGET_SIZE
      };
      this.checkerMap = await image.createPixelMap(makeCheckerBuffer(CHECKER_SIZE, CHECKER_CELL), opts);
      const diagOpts: image.InitializationOptions = {
        size: { width: DIAG_SIZE, height: DIAG_SIZE },
        srcPixelFormat: image.PixelMapFormat.RGBA_8888,
        pixelFormat: image.PixelMapFormat.RGBA_8888,
        editable: false,
        scaleMode: image.ScaleMode.FIT_TARGET_SIZE
      };
      this.diagonalMap = await image.createPixelMap(makeDiagonalBuffer(DIAG_SIZE), diagOpts);
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      this.areaText = '图源创建失败,code=' + err.code;
    }
  }

  /** 循环切换放大倍率 */
  private cycleZoom(): void {
    this.zoomIdx = (this.zoomIdx + 1) % ZOOM_STEPS.length;
  }

  /** 当前放大倍率描述文字 */
  private zoomCaption(): string {
    return ZOOM_STEPS[this.zoomIdx].caption;
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('antialiased 抗锯齿与矩阵放大')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        // 卡片1:同矩阵放大下 true/false 开关对照
        Column({ space: 8 }) {
          Text('矩阵放大 ' + this.zoomCaption() + ' 抗锯齿开关对照').fontSize(16).fontColor(Color.Gray)
          Row({ space: 14 }) {
            Column({ space: 4 }) {
              Image(this.checkerMap)
                .objectFit(ImageFit.MATRIX)
                .imageMatrix(this.zoomMatrices[this.zoomIdx])
                .antialiased(true)
                .width(TILE_SIZE)
                .height(TILE_SIZE)
                .borderWidth(1)
              Text('antialiased(true)').fontSize(11).fontColor(Color.Blue)
            }
            Column({ space: 4 }) {
              Image(this.checkerMap)
                .objectFit(ImageFit.MATRIX)
                .imageMatrix(this.zoomMatrices[this.zoomIdx])
                .antialiased(false)
                .width(TILE_SIZE)
                .height(TILE_SIZE)
                .borderWidth(1)
              Text('antialiased(false)').fontSize(11)
            }
          }
          Button('切换倍率(当前 ' + this.zoomCaption() + ')').fontSize(13).onClick(() => {
            this.cycleZoom();
          })
          Text('imageMatrix scale 放大硬边棋盘,格线边缘在 4x 下开/关抗锯齿差异最直观。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片2:antialiased Optional<boolean> 三态 ForEach
        Column({ space: 8 }) {
          Text('antialiased<23+> 三态入参').fontSize(16).fontColor(Color.Gray)
          Row({ space: 10 }) {
            ForEach(AA_CARDS, (item: AaCardItem) => {
              Column({ space: 3 }) {
                Image(this.diagonalMap)
                  .objectFit(ImageFit.MATRIX)
                  .imageMatrix(this.zoomMatrices[this.zoomIdx])
                  .antialiased(item.value)
                  .width(TILE_SIZE)
                  .height(TILE_SIZE)
                  .borderWidth(1)
                Text(item.label).fontSize(10).fontColor(Color.Blue)
                Text(item.note).fontSize(9).fontColor(Color.Gray)
                  .maxLines(2).textOverflow({ overflow: TextOverflow.Ellipsis })
              }
              .alignItems(HorizontalAlign.Center)
            }, (item: AaCardItem): string => item.valueText)
          }
          Text('对角阶梯线在 true 下过渡平滑,false/undefined 下台阶分明;undefined 与 false 行为一致。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片3:interpolation 三档对照(同图源同矩阵)
        Column({ space: 8 }) {
          Text('interpolation 三档对照').fontSize(16).fontColor(Color.Gray)
          Row({ space: 10 }) {
            ForEach(INTERP_CARDS, (item: InterpCardItem) => {
              Column({ space: 3 }) {
                Image(this.checkerMap)
                  .objectFit(ImageFit.MATRIX)
                  .imageMatrix(this.zoomMatrices[this.zoomIdx])
                  .interpolation(item.value)
                  .width(TILE_SIZE)
                  .height(TILE_SIZE)
                  .borderWidth(1)
                Text(item.label).fontSize(10).fontColor(Color.Blue)
                Text(item.note).fontSize(9).fontColor(Color.Gray)
              }
              .alignItems(HorizontalAlign.Center)
            }, (item: InterpCardItem): string => item.label)
          }
          Text('None 最近邻保留硬边;Low 双线性(默认)整体变软;Medium MipMap 利于缩小;另有 High=Cubic 质量最高但耗性能。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片4:抗锯齿开关 x 插值联动 + 背景色锯齿说明
        Column({ space: 8 }) {
          Text('开关联动').fontSize(16).fontColor(Color.Gray)
          Row({ space: 12 }) {
            Image(this.diagonalMap)
              .objectFit(ImageFit.MATRIX)
              .imageMatrix(this.zoomMatrices[this.zoomIdx])
              .antialiased(this.aaToggle)
              .interpolation(this.aaToggle ? ImageInterpolation.Low : ImageInterpolation.None)
              .backgroundColor('#20242E')
              .width(110)
              .height(110)
              .borderWidth(1)
            Column({ space: 6 }) {
              Toggle({ type: ToggleType.Switch, isOn: this.aaToggle })
                .onChange((on: boolean) => {
                  this.aaToggle = on;
                })
              Text(this.aaToggle ?
                '抗锯齿开 + 双线性插值' :
                '抗锯齿关 + 最近邻插值').fontSize(12)
              Button('切换倍率').fontSize(12).onClick(() => {
                this.cycleZoom();
              })
            }
          }
          Text('文档:图片设置了 backgroundColor 时,antialiased=true 不影响背景色自身的锯齿效果。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片5:onAreaChange 组件尺寸监听
        Column({ space: 8 }) {
          Text('onAreaChange 尺寸回读').fontSize(16).fontColor(Color.Gray)
          Image(this.checkerMap)
            .objectFit(ImageFit.MATRIX)
            .imageMatrix(this.zoomMatrices[this.zoomIdx])
            .antialiased(this.aaToggle)
            .width(this.bigBox ? 180 : 110)
            .height(this.bigBox ? 180 : 110)
            .borderWidth(1)
            .borderColor(Color.Blue)
            .onAreaChange((_oldValue: Area, newValue: Area) => {
              this.areaText = '组件尺寸 w=' + newValue.width + ' h=' + newValue.height;
            })
          Button(this.bigBox ? '缩小组件 180→110' : '放大组件 110→180').fontSize(13).onClick(() => {
            this.bigBox = !this.bigBox;
          })
          Text(this.areaText).fontSize(12)
          Text('onAreaChange 在组件宽高变化时回调;矩阵放大只改绘制,不改组件尺寸,故不触发该回调。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片6:三档放大倍率横排(恒开抗锯齿)
        Column({ space: 8 }) {
          Text('矩阵放大倍率 1x/2x/4x 对照').fontSize(16).fontColor(Color.Gray)
          Row({ space: 10 }) {
            ForEach(ZOOM_STEPS, (step: ZoomStepItem, idx: number) => {
              Column({ space: 3 }) {
                Image(this.checkerMap)
                  .objectFit(ImageFit.MATRIX)
                  .imageMatrix(this.zoomMatrices[idx])
                  .antialiased(true)
                  .width(TILE_SIZE)
                  .height(TILE_SIZE)
                  .borderWidth(1)
                  .borderColor(idx === this.zoomIdx ? Color.Red : Color.Gray)
                Text(step.caption).fontSize(10)
              }
              .alignItems(HorizontalAlign.Center)
            }, (step: ZoomStepItem): string => 'zoom-' + step.factor)
          }
          Text('同一图源同一开关,仅 matrix4.scale 倍率不同;红框为当前联动卡使用的倍率档。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片7:插值档位 x 抗锯齿开关 二维对照(3 档插值,每档 true/false 两列)
        Column({ space: 8 }) {
          Text('interpolation x antialiased 二维对照').fontSize(16).fontColor(Color.Gray)
          ForEach(INTERP_CARDS, (item: InterpCardItem) => {
            Row({ space: 10 }) {
              Text(item.label)
                .fontSize(11)
                .fontColor(Color.Blue)
                .width(56)
              Image(this.diagonalMap)
                .objectFit(ImageFit.MATRIX)
                .imageMatrix(this.zoomMatrices[this.zoomIdx])
                .interpolation(item.value)
                .antialiased(true)
                .width(64)
                .height(64)
                .borderWidth(1)
              Image(this.diagonalMap)
                .objectFit(ImageFit.MATRIX)
                .imageMatrix(this.zoomMatrices[this.zoomIdx])
                .interpolation(item.value)
                .antialiased(false)
                .width(64)
                .height(64)
                .borderWidth(1)
              Text('左:true 右:false').fontSize(9).fontColor(Color.Gray)
            }
            .width('100%')
          }, (item: InterpCardItem): string => 'x-' + item.label)
          Text('每行同插值、仅抗锯齿开关不同;对角线台阶在 true 列出现灰度过渡即为生效迹象。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片8:边缘质量观察要点清单
        Column({ space: 6 }) {
          Text('边缘质量观察要点').fontSize(16).fontColor(Color.Gray)
          ForEach(EDGE_NOTES, (note: string, idx: number) => {
            Text((idx + 1) + '. ' + note)
              .fontSize(11)
              .width('100%')
              .fontColor(Color.Black)
          }, (note: string, idx: number): string => 'edge-' + idx)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        .alignItems(HorizontalAlign.Start)

        Text('约束:antialiased 自 API 23 起支持,仅位图边缘、默认不开启;与 resizable 同设不生效;SVG 图源不支持;' +
          'LineantialiasingType 枚举在 Image 文档未找到,本例严格采用文档签名 Optional<boolean> 三态。')
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
