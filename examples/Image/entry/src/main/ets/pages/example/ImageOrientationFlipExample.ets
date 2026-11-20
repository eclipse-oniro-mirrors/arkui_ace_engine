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
 * ImageOrientationFlipExample.ets
 *
 * 主题:orientation<14+> 方向翻转与 objectRepeat / objectFit 叠加。
 * 覆盖点:
 *  1. ImageRotateOrientation 全部 9 个枚举(14+:AUTO/UP/RIGHT/DOWN/LEFT;
 *     20+:UP_MIRRORED/RIGHT_MIRRORED/DOWN_MIRRORED/LEFT_MIRRORED),
 *     以 ForEach 卡片阵列对照显示。
 *  2. 开关切换对照卡:同一 PixelMap 在两种 orientation 取值间切换对比。
 *  3. PixelMap 旋转角度对照:90/180/270 度由 image.PixelMap.rotate/rotateSync
 *     真正改写像素数据完成,与 orientation(仅显示方向)形成机制对照,Text 标注角度。
 *  4. 水平/垂直翻转对照:image.PixelMap.flip(horizontal, vertical)/flipSync 与
 *     UP_MIRRORED(水平翻转)、DOWN_MIRRORED(垂直翻转)显示效果对照。
 *  5. orientation 与 objectRepeat(NoRepeat/X/Y/XY)、objectFit(Contain/Cover/None)
 *     的叠加效果,按钮循环切换。
 *  6. 文档约束:orientation 仅支持静态位图显示;对 alt 占位图不生效;
 *     设置合法 resizable 时不生效;AUTO 依赖 EXIF,PixelMap 无头信息时显示不变;
 *     默认值 ImageRotateOrientation.UP,传 undefined/null 时取 AUTO。
 */
import { image } from '@kit.ImageKit';
import { BusinessError } from '@kit.BasicServicesKit';

/** orientation 卡片数据模型 */
interface OrientationCardItem {
  label: string;
  enumValue: number;
  since: string;
  orientation: ImageRotateOrientation;
  note: string;
}

/** PixelMap 旋转对照卡片数据模型(角度 + 对应产物在 struct 内单独持有) */
interface RotateAngleItem {
  angle: number;
  caption: string;
}

/** 底图宽(px) */
const BASE_WIDTH: number = 84;
/** 底图高(px),宽高不等以便观察旋转后的尺寸交换 */
const BASE_HEIGHT: number = 56;

/**
 * ImageRotateOrientation 枚举展示清单。
 * 取值与语义严格对照 ts-basic-components-image.md#ImageRotateOrientation:
 * AUTO=0 读EXIF;UP=1 原样;RIGHT=2 顺旋90°;DOWN=3 顺旋180°;LEFT=4 顺旋270°;
 * UP_MIRRORED=5 水平翻转;RIGHT_MIRRORED=6 水平翻转+顺旋90°;
 * DOWN_MIRRORED=7 垂直翻转;LEFT_MIRRORED=8 水平翻转+顺旋270°(后四者 20+)。
 */
const ORIENT_CARDS: OrientationCardItem[] = [
  {
    label: 'AUTO',
    enumValue: 0,
    since: '14+',
    orientation: ImageRotateOrientation.AUTO,
    note: '读EXIF方向;PixelMap无头信息则不变'
  },
  {
    label: 'UP',
    enumValue: 1,
    since: '14+',
    orientation: ImageRotateOrientation.UP,
    note: '默认值,按像素数据原样显示'
  },
  {
    label: 'RIGHT',
    enumValue: 2,
    since: '14+',
    orientation: ImageRotateOrientation.RIGHT,
    note: '顺时针旋转90度后显示'
  },
  {
    label: 'DOWN',
    enumValue: 3,
    since: '14+',
    orientation: ImageRotateOrientation.DOWN,
    note: '顺时针旋转180度后显示'
  },
  {
    label: 'LEFT',
    enumValue: 4,
    since: '14+',
    orientation: ImageRotateOrientation.LEFT,
    note: '顺时针旋转270度后显示'
  },
  {
    label: 'UP_MIRRORED',
    enumValue: 5,
    since: '20+',
    orientation: ImageRotateOrientation.UP_MIRRORED,
    note: '水平翻转后显示'
  },
  {
    label: 'RIGHT_MIRRORED',
    enumValue: 6,
    since: '20+',
    orientation: ImageRotateOrientation.RIGHT_MIRRORED,
    note: '水平翻转再顺旋90度'
  },
  {
    label: 'DOWN_MIRRORED',
    enumValue: 7,
    since: '20+',
    orientation: ImageRotateOrientation.DOWN_MIRRORED,
    note: '垂直翻转后显示'
  },
  {
    label: 'LEFT_MIRRORED',
    enumValue: 8,
    since: '20+',
    orientation: ImageRotateOrientation.LEFT_MIRRORED,
    note: '水平翻转再顺旋270度'
  }
];

/** rotate 角度对照清单(90/180/270) */
const ROTATE_ANGLES: RotateAngleItem[] = [
  { angle: 90, caption: 'rotateSync(90)' },
  { angle: 180, caption: 'rotateSync(180)' },
  { angle: 270, caption: 'rotate(270) 异步' }
];

/** objectRepeat 循环序列 */
const REPEAT_SEQUENCE: ImageRepeat[] = [
  ImageRepeat.NoRepeat,
  ImageRepeat.X,
  ImageRepeat.Y,
  ImageRepeat.XY
];
/** objectRepeat 名称对照 */
const REPEAT_NAMES: string[] = ['NoRepeat', 'X(水平重复)', 'Y(垂直重复)', 'XY(双向重复)'];
/** objectFit 循环序列(与 orientation 叠加观察) */
const FIT_SEQUENCE: ImageFit[] = [ImageFit.Contain, ImageFit.Cover, ImageFit.None];
/** objectFit 名称对照 */
const FIT_NAMES: string[] = ['Contain', 'Cover', 'None(保持原尺寸)'];

/**
 * 生成带方向性的箭头图案像素缓冲:
 * 左上角红色块标识"上",斜向渐变标识"左下→右上",旋转/翻转后一眼可辨。
 */
function makeArrowBuffer(width: number, height: number): ArrayBuffer {
  const buffer: ArrayBuffer = new ArrayBuffer(width * height * 4);
  const data: Uint8Array = new Uint8Array(buffer);
  for (let y: number = 0; y < height; y++) {
    for (let x: number = 0; x < width; x++) {
      const idx: number = (y * width + x) * 4;
      const grad: number = Math.floor(255 * (x + y) / (width + height - 2));
      data[idx] = 30 + Math.floor(grad * 0.4);
      data[idx + 1] = 60 + Math.floor(grad * 0.7);
      data[idx + 2] = 120;
      data[idx + 3] = 255;
      // 左上 12x12 红色标识块
      if (x < 12 && y < 12) {
        data[idx] = 235;
        data[idx + 1] = 45;
        data[idx + 2] = 45;
      }
      // 顶部一条亮黄横线,强化"上边"方向感
      if (y < 3) {
        data[idx] = 240;
        data[idx + 1] = 220;
        data[idx + 2] = 90;
      }
    }
  }
  return buffer;
}

@Entry
@Component
struct ImageOrientationFlipExample {
  @State baseMap: image.PixelMap | undefined = undefined;
  @State rot90Map: image.PixelMap | undefined = undefined;
  @State rot180Map: image.PixelMap | undefined = undefined;
  @State rot270Map: image.PixelMap | undefined = undefined;
  @State mirrorHMap: image.PixelMap | undefined = undefined;
  @State mirrorVMap: image.PixelMap | undefined = undefined;
  @State flipOn: boolean = true;
  @State repeatIdx: number = 0;
  @State fitIdx: number = 0;
  @State statusText: string = '正在准备方向对照图源…';
  @State selectedOrient: ImageRotateOrientation = ImageRotateOrientation.RIGHT;
  @State selectedLabel: string = 'RIGHT(顺旋90°)';

  async aboutToAppear(): Promise<void> {
    try {
      const opts: image.InitializationOptions = {
        size: { width: BASE_WIDTH, height: BASE_HEIGHT },
        srcPixelFormat: image.PixelMapFormat.RGBA_8888,
        pixelFormat: image.PixelMapFormat.RGBA_8888,
        editable: true,
        scaleMode: image.ScaleMode.FIT_TARGET_SIZE
      };
      const base: image.PixelMap = await image.createPixelMap(makeArrowBuffer(BASE_WIDTH, BASE_HEIGHT), opts);
      this.baseMap = base;
      // 90/180 用同步接口,270 用异步接口演示两种调用形态
      const work90: image.PixelMap = await base.clone();
      work90.rotateSync(90);
      this.rot90Map = work90;
      const work180: image.PixelMap = await base.clone();
      work180.rotateSync(180);
      this.rot180Map = work180;
      const work270: image.PixelMap = await base.clone();
      await work270.rotate(270);
      this.rot270Map = work270;
      // 水平翻转 / 垂直翻转(horizontal、vertical 两参数)
      const workH: image.PixelMap = await base.clone();
      workH.flipSync(true, false);
      this.mirrorHMap = workH;
      const workV: image.PixelMap = await base.clone();
      workV.flipSync(false, true);
      this.mirrorVMap = workV;
      this.statusText = '底图 ' + BASE_WIDTH + 'x' + BASE_HEIGHT +
        ',旋转与翻转副本已生成(像素级改写)';
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      this.statusText = '图源准备失败,code=' + err.code;
    }
  }

  /** 切换开关对照卡的 orientation 取值 */
  private toggleCompare(): void {
    this.flipOn = !this.flipOn;
    this.selectedOrient = this.flipOn ? ImageRotateOrientation.RIGHT : ImageRotateOrientation.LEFT;
    this.selectedLabel = this.flipOn ? 'RIGHT(顺旋90°)' : 'LEFT(顺旋270°)';
  }

  /** 循环切换 objectRepeat */
  private cycleRepeat(): void {
    this.repeatIdx = (this.repeatIdx + 1) % REPEAT_SEQUENCE.length;
  }

  /** 循环切换 objectFit */
  private cycleFit(): void {
    this.fitIdx = (this.fitIdx + 1) % FIT_SEQUENCE.length;
  }

  /** 按角度取对应旋转副本(供 ForEach 渲染) */
  private mapForAngle(angle: number): image.PixelMap | undefined {
    if (angle === 90) {
      return this.rot90Map;
    }
    if (angle === 180) {
      return this.rot180Map;
    }
    return this.rot270Map;
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('orientation 方向与翻转对照')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        // 卡片1:开关切换对照(RIGHT vs LEFT),左图带 orientation,右图原样参照
        Column({ space: 8 }) {
          Text('orientation<14+> 开关对照').fontSize(16).fontColor(Color.Gray)
          Row({ space: 14 }) {
            Column({ space: 4 }) {
              Image(this.baseMap)
                .orientation(this.selectedOrient)
                .width(110)
                .height(80)
                .borderWidth(1)
              Text('orientation=' + (this.flipOn ? 'RIGHT' : 'LEFT')).fontSize(12)
            }
            Column({ space: 4 }) {
              Image(this.baseMap)
                .width(110)
                .height(80)
                .borderWidth(1)
                .borderColor(Color.Gray)
              Text('未设置(默认UP)').fontSize(12).fontColor(Color.Gray)
            }
          }
          Button(this.flipOn ? '切换为 LEFT(270°)' : '切换为 RIGHT(90°)').onClick(() => {
            this.toggleCompare();
          })
          Text('当前选中: ' + this.selectedLabel).fontSize(13)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片2:9 个枚举值 ForEach 阵列
        Column({ space: 8 }) {
          Text('ImageRotateOrientation 全枚举(14+/20+)').fontSize(16).fontColor(Color.Gray)
          Flex({ wrap: FlexWrap.Wrap }) {
            ForEach(ORIENT_CARDS, (item: OrientationCardItem) => {
              Column({ space: 2 }) {
                Image(this.baseMap)
                  .orientation(item.orientation)
                  .width(84)
                  .height(56)
                  .borderWidth(1)
                Text(item.label + '(' + item.enumValue + ')').fontSize(10)
                Text(item.since).fontSize(9).fontColor(Color.Gray)
              }
              .margin(4)
              .alignItems(HorizontalAlign.Center)
            }, (item: OrientationCardItem): string => item.label)
          }
          Text('说明:RIGHT/DOWN/LEFT 分别为顺旋90/180/270度;20+ 的四个 MIRRORED 值支持镜像。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片3:PixelMap 旋转角度对照(像素级 rotate,Text 标注 90/180/270)
        Column({ space: 8 }) {
          Text('image.PixelMap.rotate 角度对照').fontSize(16).fontColor(Color.Gray)
          Row({ space: 10 }) {
            Column({ space: 3 }) {
              Image(this.baseMap).width(76).height(56).borderWidth(1)
              Text('0° 底图').fontSize(11)
            }
            ForEach(ROTATE_ANGLES, (item: RotateAngleItem) => {
              Column({ space: 3 }) {
                Image(this.mapForAngle(item.angle))
                  .width(item.angle % 180 === 0 ? 76 : 56)
                  .height(item.angle % 180 === 0 ? 56 : 76)
                  .borderWidth(1)
                Text(item.angle + '° ' + item.caption).fontSize(11)
              }
            }, (item: RotateAngleItem): string => 'rot-' + item.angle)
          }
          Text('rotate 直接改写像素数据,宽高互换;与 orientation(仅改显示方向、不改图源)机制不同。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片4:水平/垂直翻转对照
        Column({ space: 8 }) {
          Text('PixelMap.flip 水平/垂直翻转对照').fontSize(16).fontColor(Color.Gray)
          Row({ space: 10 }) {
            Column({ space: 3 }) {
              Image(this.mirrorHMap).width(76).height(56).borderWidth(1)
              Text('flip(true,false) 水平').fontSize(11)
            }
            Column({ space: 3 }) {
              Image(this.mirrorVMap).width(76).height(56).borderWidth(1)
              Text('flip(false,true) 垂直').fontSize(11)
            }
            Column({ space: 3 }) {
              Image(this.baseMap)
                .orientation(ImageRotateOrientation.UP_MIRRORED)
                .width(76)
                .height(56)
                .borderWidth(1)
                .borderColor(Color.Blue)
              Text('UP_MIRRORED 显示级').fontSize(11).fontColor(Color.Blue)
            }
          }
          Text('flip 为像素级翻转;UP_MIRRORED/DOWN_MIRRORED(20+)为显示级镜像,效果对应但不动图源。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片5:orientation + objectRepeat 叠加
        Column({ space: 8 }) {
          Text('orientation + objectRepeat 叠加').fontSize(16).fontColor(Color.Gray)
          Image(this.baseMap)
            .orientation(ImageRotateOrientation.LEFT)
            .objectRepeat(REPEAT_SEQUENCE[this.repeatIdx])
            .width(150)
            .height(100)
            .borderWidth(1)
          Button('切换重复模式(当前 ' + REPEAT_NAMES[this.repeatIdx] + ')')
            .fontSize(13)
            .onClick(() => {
              this.cycleRepeat();
            })
          Text('objectRepeat 从中心向两边重复,空间不足则截断;SVG 图源不支持。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片6:orientation + objectFit 叠加
        Column({ space: 8 }) {
          Text('orientation + objectFit 叠加').fontSize(16).fontColor(Color.Gray)
          Image(this.baseMap)
            .orientation(ImageRotateOrientation.RIGHT_MIRRORED)
            .objectFit(FIT_SEQUENCE[this.fitIdx])
            .width(140)
            .height(90)
            .borderWidth(1)
          Button('切换填充(当前 ' + FIT_NAMES[this.fitIdx] + ')')
            .fontSize(13)
            .onClick(() => {
              this.cycleFit();
            })
          Text('RIGHT_MIRRORED=水平翻转再顺旋90度;objectFit 决定内容在组件内的缩放填充方式。')
            .fontSize(11)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片7:AUTO 与 EXIF 说明
        Column({ space: 8 }) {
          Text('AUTO 行为说明').fontSize(16).fontColor(Color.Gray)
          Row({ space: 10 }) {
            Image(this.baseMap)
              .orientation(ImageRotateOrientation.AUTO)
              .width(96)
              .height(64)
              .borderWidth(1)
            Image(this.baseMap)
              .orientation(ImageRotateOrientation.UP)
              .width(96)
              .height(64)
              .borderWidth(1)
          }
          Text('PixelMap 图源不含 EXIF 头信息,左(AUTO)与右(UP)显示一致;' +
            '携带旋转/翻转 EXIF 的文件图建议用 AUTO 还原正确方向。')
            .fontSize(11)
            .fontColor(Color.Gray)
          Text('状态: ' + this.statusText).fontSize(12).fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        // 卡片8:枚举语义速查表(复用 ORIENT_CARDS 的 note 字段)
        Column({ space: 6 }) {
          Text('枚举语义速查').fontSize(16).fontColor(Color.Gray)
          ForEach(ORIENT_CARDS, (item: OrientationCardItem) => {
            Row({ space: 8 }) {
              Text(item.label)
                .fontSize(11)
                .fontColor(Color.Blue)
                .width(120)
              Text(item.note)
                .fontSize(11)
                .fontColor(Color.Black)
                .layoutWeight(1)
            }
            .width('100%')
          }, (item: OrientationCardItem): string => 'note-' + item.label)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        .alignItems(HorizontalAlign.Start)

        Text('约束:orientation 仅支持静态位图显示,对 alt 占位图不生效;设置合法 resizable 时不生效;' +
          '默认 UP,传 undefined/null 取 AUTO;卡片能力自 API 23 起支持。')
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
