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
 * ImageMatrixPipelineExample.ets
 *
 * 主题:imageMatrix<15+> 变换矩阵管线演示。
 * 覆盖点:
 *  1. imageMatrix(matrix: ImageMatrix) 属性,API version 15 起支持;
 *     ImageMatrix 即 matrix4.Matrix4Transit(见 ts-basic-components-image.md#ImageMatrix 对象说明)。
 *  2. 必须配合 objectFit(ImageFit.MATRIX)(枚举值 16,15+)才生效。
 *  3. 用 matrix4 构造 identity / translate / scale / rotate / skew / combine / invert / init / copy
 *     共 19 组矩阵,以 ForEach 卡片阵列展示,点选切换主预览。
 *  4. 链式调用顺序说明:translate→scale 与 scale→translate 结果不同,
 *     Matrix4Transit 的链式方法按调用顺序依次右乘,顺序不可交换。
 *  5. 文档约束:imageMatrix 只对位图/PixelMap 图源生效,SVG 图源不支持;
 *     设置 resizable 或 objectRepeat 时 imageMatrix 不生效;
 *     imageMatrix 只处理图源,不触发 Image 回调事件。
 */
import { image } from '@kit.ImageKit';
import { matrix4 } from '@kit.ArkUI';
import { BusinessError } from '@kit.BasicServicesKit';

/**
 * 单张矩阵卡片的数据模型。
 * name:卡片标题;ops:链式调用描述;matrix:实际应用的 Matrix4Transit。
 */
interface MatrixCardItem {
  name: string;
  ops: string;
  matrix: matrix4.Matrix4Transit;
}

/** 底图边长(px),PixelMap 图源尺寸 */
const BASE_SIZE: number = 96;
/** 主预览组件边长(vp) */
const PREVIEW_SIZE: number = 150;
/** ForEach 缩略卡片边长(vp) */
const THUMB_SIZE: number = 62;

/**
 * 构造 19 组演示矩阵。
 * 注意:Matrix4Transit 的 translate/scale/rotate/skew 等方法会修改自身,
 * 因此每组卡片都必须从 matrix4.identity() 重新起链,禁止复用同一对象。
 */
function createMatrixCards(): MatrixCardItem[] {
  // card19 的基准矩阵:演示 copy() 后再变换、原对象不被修改的语义
  const copyBase: matrix4.Matrix4Transit = matrix4.identity().scale({ x: 1.5, y: 0.8 });
  const cards: MatrixCardItem[] = [
    {
      name: '01 单位矩阵',
      ops: 'identity()',
      matrix: matrix4.identity()
    },
    {
      name: '02 单向平移',
      ops: 'identity().translate({ x: 30, y: 0 })',
      matrix: matrix4.identity().translate({ x: 30, y: 0 })
    },
    {
      name: '03 双向平移',
      ops: 'identity().translate({ x: -20, y: 24 })',
      matrix: matrix4.identity().translate({ x: -20, y: 24 })
    },
    {
      name: '04 等比放大',
      ops: 'identity().scale({ x: 1.6, y: 1.6 })',
      matrix: matrix4.identity().scale({ x: 1.6, y: 1.6 })
    },
    {
      name: '05 非对称缩放',
      ops: 'identity().scale({ x: 0.6, y: 1.5 })',
      matrix: matrix4.identity().scale({ x: 0.6, y: 1.5 })
    },
    {
      name: '06 负向缩放镜像',
      ops: 'identity().scale({ x: -1.2, y: 1.2 })',
      matrix: matrix4.identity().scale({ x: -1.2, y: 1.2 })
    },
    {
      name: '07 指定中心缩放',
      ops: 'identity().scale({ x: 1.8, y: 1.8, centerX: 24, centerY: 24 })',
      matrix: matrix4.identity().scale({ x: 1.8, y: 1.8, centerX: 24, centerY: 24 })
    },
    {
      name: '08 绕Z轴旋转45°',
      ops: 'identity().rotate({ x: 0, y: 0, z: 1, angle: 45 })',
      matrix: matrix4.identity().rotate({ x: 0, y: 0, z: 1, angle: 45 })
    },
    {
      name: '09 偏心旋转90°',
      ops: 'identity().rotate({ x: 0, y: 0, z: 1, angle: 90, centerX: 30, centerY: 0 })',
      matrix: matrix4.identity().rotate({ x: 0, y: 0, z: 1, angle: 90, centerX: 30, centerY: 0 })
    },
    {
      name: '10 斜轴3D旋转',
      ops: 'identity().rotate({ x: 1, y: 0.5, z: 1, angle: 30 })',
      matrix: matrix4.identity().rotate({ x: 1, y: 0.5, z: 1, angle: 30 })
    },
    {
      name: '11 水平错切',
      ops: 'identity().skew(0.5, 0)  // skew为12+接口',
      matrix: matrix4.identity().skew(0.5, 0)
    },
    {
      name: '12 双向错切',
      ops: 'identity().skew(0.3, 0.4)',
      matrix: matrix4.identity().skew(0.3, 0.4)
    },
    {
      name: '13 先平移后缩放',
      ops: 'identity().translate({ x: 26, y: 0 }).scale({ x: 1.4, y: 1.4 })',
      matrix: matrix4.identity().translate({ x: 26, y: 0 }).scale({ x: 1.4, y: 1.4 })
    },
    {
      name: '14 先缩放后平移',
      ops: 'identity().scale({ x: 1.4, y: 1.4 }).translate({ x: 26, y: 0 })',
      matrix: matrix4.identity().scale({ x: 1.4, y: 1.4 }).translate({ x: 26, y: 0 })
    },
    {
      name: '15 旋转叠加错切',
      ops: 'identity().rotate({ z: 1, angle: 20 }).skew(0.25, 0)',
      matrix: matrix4.identity().rotate({ z: 1, angle: 20 }).skew(0.25, 0)
    },
    {
      name: '16 全链式管线',
      ops: 'identity().translate().rotate().scale().skew() 四段链',
      matrix: matrix4.identity()
        .translate({ x: 8, y: 12 })
        .rotate({ x: 0, y: 0, z: 1, angle: 15 })
        .scale({ x: 1.3, y: 0.9 })
        .skew(0.2, 0.1)
    },
    {
      name: '17 combine合并矩阵',
      ops: 'identity().rotate({ z: 1, angle: 30 }).combine(identity().translate({ x: 22, y: 8 }))',
      matrix: matrix4.identity().rotate({ x: 0, y: 0, z: 1, angle: 30 })
        .combine(matrix4.identity().translate({ x: 22, y: 8 }))
    },
    {
      name: '18 invert求逆矩阵',
      ops: 'identity().translate({ x: 30, y: 10 }).scale({ x: 2, y: 2 }).invert()',
      matrix: matrix4.identity().translate({ x: 30, y: 10 }).scale({ x: 2, y: 2 }).invert()
    },
    {
      name: '19 copy保留原件',
      ops: 'copyBase.copy().translate({ x: 15, y: 0 }) 原对象不受影响',
      matrix: copyBase.copy().translate({ x: 15, y: 0 })
    }
  ];
  return cards;
}

/**
 * 构造「变换+逆变换」复合矩阵:forward.combine(forward.copy().invert()) ≈ identity,
 * 用于演示 invert 的数学语义(逆矩阵还原)。
 */
function createRestoreMatrix(): matrix4.Matrix4Transit {
  const forward: matrix4.Matrix4Transit = matrix4.identity()
    .rotate({ x: 0, y: 0, z: 1, angle: 35 })
    .scale({ x: 1.7, y: 1.7 });
  const backward: matrix4.Matrix4Transit = matrix4.identity()
    .rotate({ x: 0, y: 0, z: 1, angle: 35 })
    .scale({ x: 1.7, y: 1.7 })
    .copy()
    .invert();
  return forward.combine(backward);
}

/**
 * 生成对角条纹 ArrayBuffer,便于观察矩阵变换后像素的位移与方向。
 */
function makeStripeBuffer(size: number): ArrayBuffer {
  const buffer: ArrayBuffer = new ArrayBuffer(size * size * 4);
  const data: Uint8Array = new Uint8Array(buffer);
  for (let y: number = 0; y < size; y++) {
    for (let x: number = 0; x < size; x++) {
      const idx: number = (y * size + x) * 4;
      const stripe: number = ((x + y) % 24 < 12) ? 235 : 45;
      const corner: boolean = (x < 10 && y < 10);
      data[idx] = corner ? 220 : stripe;
      data[idx + 1] = corner ? 60 : stripe;
      data[idx + 2] = corner ? 60 : (stripe === 235 ? 250 : 120);
      data[idx + 3] = 255;
    }
  }
  return buffer;
}

@Entry
@Component
struct ImageMatrixPipelineExample {
  @State basePixelMap: image.PixelMap | undefined = undefined;
  @State selectedIndex: number = 0;
  @State statusText: string = '正在创建PixelMap底图…';
  @State pipelineOn: boolean = true;
  private cards: MatrixCardItem[] = createMatrixCards();
  // 正变换矩阵与它的逆矩阵 combine 后 ≈ 单位矩阵,图像回到原位
  private restoreMatrix: matrix4.Matrix4Transit = createRestoreMatrix();
  // 链式顺序对照A:先平移、后缩放(平移量随后被缩放1.5倍)
  private orderTranslateScale: matrix4.Matrix4Transit = matrix4.identity()
    .translate({ x: 24, y: 0 })
    .scale({ x: 1.5, y: 1.5 });
  // 链式顺序对照B:先缩放、后平移(平移量不受缩放影响)
  private orderScaleTranslate: matrix4.Matrix4Transit = matrix4.identity()
    .scale({ x: 1.5, y: 1.5 })
    .translate({ x: 24, y: 0 });
  // 与 initMatrix 视觉对照的链式写法(等效缩放1.4+平移18/12)
  private chainScaleTranslate: matrix4.Matrix4Transit = matrix4.identity()
    .scale({ x: 1.4, y: 1.4 })
    .translate({ x: 18, y: 12 });
  // 卡片19 基准矩阵本体,证明 copy() 后原对象不被后续 translate 修改
  private copyBaseOnly: matrix4.Matrix4Transit = matrix4.identity()
    .scale({ x: 1.5, y: 0.8 });
  // init(16元素行主序数组)直接构造 平移+缩放 矩阵,与链式写法对照
  private initMatrix: matrix4.Matrix4Transit = matrix4.init([
    1.4, 0, 0, 0,
    0, 1.4, 0, 0,
    0, 0, 1, 0,
    18, 12, 0, 1
  ]);

  async aboutToAppear(): Promise<void> {
    try {
      const opts: image.InitializationOptions = {
        size: { width: BASE_SIZE, height: BASE_SIZE },
        srcPixelFormat: image.PixelMapFormat.RGBA_8888,
        pixelFormat: image.PixelMapFormat.RGBA_8888,
        editable: true,
        scaleMode: image.ScaleMode.FIT_TARGET_SIZE
      };
      this.basePixelMap = await image.createPixelMap(makeStripeBuffer(BASE_SIZE), opts);
      this.statusText = '底图就绪 ' + BASE_SIZE + 'x' + BASE_SIZE + ',图源为PixelMap位图';
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      this.statusText = 'PixelMap创建失败,code=' + err.code;
    }
  }

  /** 主预览当前选中的矩阵 */
  private currentMatrix(): matrix4.Matrix4Transit {
    return this.cards[this.selectedIndex].matrix;
  }

  /** 主预览当前选中的描述文本 */
  private currentLabel(): string {
    const card: MatrixCardItem = this.cards[this.selectedIndex];
    return card.name + ' | ' + card.ops;
  }

  /** 循环切换选中索引,delta为+1/-1 */
  private shiftSelection(delta: number): void {
    const total: number = this.cards.length;
    this.selectedIndex = ((this.selectedIndex + delta) % total + total) % total;
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('imageMatrix 矩阵变换管线')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        // 卡片1:主预览,imageMatrix + ImageFit.MATRIX 联动
        Column({ space: 8 }) {
          Text('主预览 imageMatrix<15+>(API15)').fontSize(16).fontColor(Color.Gray)
          Image(this.basePixelMap)
            .objectFit(ImageFit.MATRIX)
            .imageMatrix(this.currentMatrix())
            .width(PREVIEW_SIZE)
            .height(PREVIEW_SIZE)
            .borderWidth(1)
            .borderColor(Color.Blue)
          Text(this.currentLabel())
            .fontSize(13)
            .maxLines(2)
            .textOverflow({ overflow: TextOverflow.Ellipsis })
          Row({ space: 10 }) {
            Button('上一组').fontSize(13).onClick(() => {
              this.shiftSelection(-1);
            })
            Button('下一组').fontSize(13).onClick(() => {
              this.shiftSelection(1);
            })
            Toggle({ type: ToggleType.Switch, isOn: this.pipelineOn })
              .onChange((on: boolean) => {
                this.pipelineOn = on;
              })
          }
          Text(this.pipelineOn ? '当前应用链式矩阵' : '已切回单位矩阵对照')
            .fontSize(13)
          // pipelineOn 为 false 时以单位矩阵对照,验证矩阵开关差异
          Image(this.basePixelMap)
            .objectFit(ImageFit.MATRIX)
            .imageMatrix(this.pipelineOn ? this.currentMatrix() : matrix4.identity())
            .width(90)
            .height(90)
            .borderWidth(1)
            .borderColor(Color.Gray)
          Text('状态: ' + this.statusText).fontSize(12).fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片2:链式调用顺序对照(矩阵乘法不满足交换律)
        Column({ space: 8 }) {
          Text('链式调用顺序说明').fontSize(16).fontColor(Color.Gray)
          Row({ space: 12 }) {
            Column({ space: 4 }) {
              Image(this.basePixelMap)
                .objectFit(ImageFit.MATRIX)
                .imageMatrix(this.orderTranslateScale)
                .width(THUMB_SIZE + 30)
                .height(THUMB_SIZE + 30)
                .borderWidth(1)
              Text('A: translate→scale').fontSize(12)
              Text('平移24被后续scale×1.5放大').fontSize(11).fontColor(Color.Gray)
            }
            Column({ space: 4 }) {
              Image(this.basePixelMap)
                .objectFit(ImageFit.MATRIX)
                .imageMatrix(this.orderScaleTranslate)
                .width(THUMB_SIZE + 30)
                .height(THUMB_SIZE + 30)
                .borderWidth(1)
              Text('B: scale→translate').fontSize(12)
              Text('平移24不受先前scale影响').fontSize(11).fontColor(Color.Gray)
            }
          }
          Text('规则: Matrix4Transit 链式方法按书写顺序依次参与运算,' +
            'A与B结果不同;需要保留原始矩阵时先 copy() 再变换。')
            .fontSize(12)
            .fontColor(Color.Gray)
          Row({ space: 12 }) {
            Column({ space: 4 }) {
              Image(this.basePixelMap)
                .objectFit(ImageFit.MATRIX)
                .imageMatrix(this.initMatrix)
                .width(THUMB_SIZE + 30)
                .height(THUMB_SIZE + 30)
                .borderWidth(1)
              Text('matrix4.init(16元素)').fontSize(12)
            }
            Column({ space: 4 }) {
              Image(this.basePixelMap)
                .objectFit(ImageFit.MATRIX)
                .imageMatrix(this.chainScaleTranslate)
                .width(THUMB_SIZE + 30)
                .height(THUMB_SIZE + 30)
                .borderWidth(1)
              Text('链式 scale+translate').fontSize(12)
            }
            Column({ space: 4 }) {
              Image(this.basePixelMap)
                .objectFit(ImageFit.MATRIX)
                .imageMatrix(this.copyBaseOnly)
                .width(THUMB_SIZE + 30)
                .height(THUMB_SIZE + 30)
                .borderWidth(1)
              Text('copyBase原样(对照)').fontSize(12)
            }
          }
          Text('init 行主序 4x4:对角线1.4为缩放,第4行18/12为平移,效果近似链式写法;' +
            'copyBase 在卡片19中经 copy() 再变换,原对象效果保持不变(右图)。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片3:ForEach 矩阵缩略卡片阵列,点击切换主预览
        Column({ space: 8 }) {
          Text('矩阵卡片阵列(' + this.cards.length + '组,点击切换)').fontSize(16).fontColor(Color.Gray)
          Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Start }) {
            ForEach(this.cards, (item: MatrixCardItem, index: number) => {
              Column({ space: 2 }) {
                Image(this.basePixelMap)
                  .objectFit(ImageFit.MATRIX)
                  .imageMatrix(item.matrix)
                  .width(THUMB_SIZE)
                  .height(THUMB_SIZE)
                  .borderWidth(1)
                  .borderColor(index === this.selectedIndex ? Color.Red : Color.Gray)
                  .backgroundColor(Color.White)
                Text(item.name.slice(0, 2)).fontSize(10).fontColor(Color.Gray)
              }
              .margin(4)
              .onClick(() => {
                this.selectedIndex = index;
              })
            }, (item: MatrixCardItem, index: number): string => index + '-' + item.name)
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片4:矩阵清单文字说明
        Column({ space: 6 }) {
          Text('链式调用顺序清单').fontSize(16).fontColor(Color.Gray)
          ForEach(this.cards, (item: MatrixCardItem, index: number) => {
            Text((index + 1) + '. ' + item.ops)
              .fontSize(11)
              .fontColor(index === this.selectedIndex ? Color.Blue : Color.Black)
              .width('100%')
              .maxLines(1)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
          }, (item: MatrixCardItem, index: number): string => 'ops-' + index)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        .alignItems(HorizontalAlign.Start)

        // 卡片5:invert 逆矩阵还原演示
        Column({ space: 8 }) {
          Text('invert 逆矩阵还原').fontSize(16).fontColor(Color.Gray)
          Row({ space: 12 }) {
            Column({ space: 4 }) {
              Image(this.basePixelMap)
                .objectFit(ImageFit.MATRIX)
                .imageMatrix(this.cards[15].matrix)
                .width(THUMB_SIZE + 26)
                .height(THUMB_SIZE + 26)
                .borderWidth(1)
              Text('仅正变换(卡片16)').fontSize(12)
            }
            Column({ space: 4 }) {
              Image(this.basePixelMap)
                .objectFit(ImageFit.MATRIX)
                .imageMatrix(this.restoreMatrix)
                .width(THUMB_SIZE + 26)
                .height(THUMB_SIZE + 26)
                .borderWidth(1)
              Text('正变换+逆变换').fontSize(12)
            }
          }
          Text('combine(M.copy().invert()) 结果接近单位矩阵,图像基本回到原位;' +
            '注意 invert 作用于当前对象,链式复用前先 copy。')
            .fontSize(12)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Text('约束:imageMatrix仅位图/PixelMap图源生效,SVG不支持;须objectFit(ImageFit.MATRIX)且与resizable、objectRepeat互斥;' +
          '矩阵只作用于图源绘制,不触发onComplete等回调。')
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
