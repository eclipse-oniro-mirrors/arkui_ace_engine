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
 * ========================================
 * renderMode 图源类型约束矩阵示例 (API 7 / 10+ 语义)
 * ========================================
 * 功能说明:
 * 本示例把 ImageRenderMode 的两个枚举成员与不同图源类型组成渲染矩阵，
 * 逐格验证文档约束“SVG类型图源不支持该属性”。位图格子使用运行时
 * @kit.ImageKit 创建的 PixelMap（彩色分带图），可真实观察到 Template
 * 的黑白（模板）渲染；SVG 格子两种模式下画面不变，即 renderMode 不生效。
 * 与 ImageRenderModeExample（图标主题化演示页）不同，本页聚焦
 * “模式 × 图源类型”约束矩阵与 fillColor/renderMode 互斥关系说明。
 *
 * ========================================
 * 涉及的 Image API
 * ========================================
 * 1. renderMode(value: ImageRenderMode) - API 7 起，默认 ImageRenderMode.Original
 * 2. ImageRenderMode.Original  (值 0) 原色渲染模式
 * 3. ImageRenderMode.Template  (值 1) 黑白渲染模式
 * 4. fillColor / colorFilter 与 renderMode 的互斥与替代关系（文档语义）
 *
 * ========================================
 * 文档约束（本页逐条演示）
 * ========================================
 * 1. SVG 类型图源不支持 renderMode。
 * 2. 设置 ColorFilter 时，renderMode 属性设置不生效。
 * 3. 组件入参为 AnimatedDrawableDescriptor 时 renderMode 不生效。
 * 4. fillColor 仅对 SVG 图源生效，与 renderMode（不支持 SVG）在图源维度互斥：
 *    SVG 想整体变色用 fillColor，位图想黑白化用 renderMode(Template)，
 *    位图想改色文档建议使用 colorFilter。
 */

import { image } from '@kit.ImageKit';
import { BusinessError } from '@kit.BasicServicesKit';

interface RenderModeCell {
  label: string;
  mode: ImageRenderMode;
  value: number;
  desc: string;
}

interface RenderSourceRow {
  name: string;
  kind: string;
  resource: Resource | undefined;
  isBitmap: boolean;
  supportRenderMode: boolean;
  note: string;
}

@Entry
@Component
struct ImageRenderModeConstraintsExample {
  @State bitmap: image.PixelMap | undefined = undefined;
  @State bitmapStatus: string = '正在创建 PixelMap…';
  @State mainModeIndex: number = 0;
  // 两个枚举成员，取值与文档枚举表一致（Original=0 / Template=1）
  private modeCells: RenderModeCell[] = [
    {
      label: 'Original',
      mode: ImageRenderMode.Original,
      value: 0,
      desc: '原色渲染模式（默认值）'
    },
    {
      label: 'Template',
      mode: ImageRenderMode.Template,
      value: 1,
      desc: '黑白渲染模式'
    }
  ];
  // 图源矩阵行：1 个位图行 + 2 个 SVG 行，位图源为运行时创建的 PixelMap
  private sourceRows: RenderSourceRow[] = [
    {
      name: 'PixelMap 位图',
      kind: 'bitmap',
      resource: undefined,
      isBitmap: true,
      supportRenderMode: true,
      note: '位图：Template 应呈现黑白效果'
    },
    {
      name: 'icon.svg',
      kind: 'svg',
      resource: $r('app.media.icon'),
      isBitmap: false,
      supportRenderMode: false,
      note: 'SVG：文档明确不支持 renderMode，两列应无差异'
    },
    {
      name: 'star.svg',
      kind: 'svg',
      resource: $r('app.media.star'),
      isBitmap: false,
      supportRenderMode: false,
      note: 'SVG：整体改色应改用 fillColor'
    }
  ];

  async aboutToAppear(): Promise<void> {
    await this.createColorBandBitmap();
  }

  // 用 @kit.ImageKit 运行时生成 120×120 RGBA_8888 彩色分带位图，
  // 作为“PNG 类位图”对照组（工程 media 目录内没有位图资源）。
  private async createColorBandBitmap(): Promise<void> {
    const size: number = 120;
    const buffer: ArrayBuffer = new ArrayBuffer(size * size * 4);
    const pixels: Uint8Array = new Uint8Array(buffer);
    for (let y: number = 0; y < size; y++) {
      for (let x: number = 0; x < size; x++) {
        const offset: number = (y * size + x) * 4;
        const band: number = Math.floor(y / 30); // 4 条水平色带
        pixels[offset] = band % 2 === 0 ? 0xFF : 0x20;
        pixels[offset + 1] = band >= 2 ? 0xC8 : 0x40;
        pixels[offset + 2] = x < 60 ? 0x30 : 0xE0;
        pixels[offset + 3] = 0xFF;
      }
    }
    const opts: image.InitializationOptions = {
      size: { height: size, width: size },
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      editable: true
    };
    try {
      this.bitmap = await image.createPixelMap(buffer, opts);
      this.bitmapStatus = 'PixelMap 创建成功（120×120 RGBA_8888 彩色分带）';
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      this.bitmapStatus = 'PixelMap 创建失败: code=' + err.code + ', message=' + err.message;
    }
  }

  private sourceOfRow(row: RenderSourceRow): image.PixelMap | Resource | undefined {
    return row.isBitmap ? this.bitmap : row.resource;
  }

  private mainMode(): RenderModeCell {
    return this.modeCells[this.mainModeIndex];
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('renderMode 图源类型约束矩阵示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        this.BitmapStatusCard()
        this.MainCompareCard()
        this.MatrixCard()
        this.ExclusionCard()

        Text('验证要点：位图行 Template 列应黑白化，SVG 行两列无差异即“SVG类型图源不支持该属性”；如需 SVG 改色请使用 fillColor。')
          .fontSize(12)
          .fontColor(Color.Blue)
          .margin({ top: 10, bottom: 20 })
      }
      .width('100%')
      .padding({ left: 16, right: 16 })
    }
    .width('100%')
    .height('100%')
    .backgroundColor(Color.White)
  }

  @Builder
  BitmapStatusCard(): void {
    Column({ space: 8 }) {
      Text('位图对照组准备 (image.createPixelMap, API 8+)').fontSize(16).fontColor(Color.Gray)
      Text(this.bitmapStatus)
        .fontSize(12)
        .fontColor(this.bitmap === undefined ? Color.Orange : Color.Green)
      if (this.bitmap !== undefined) {
        Row({ space: 12 }) {
          Column({ space: 4 }) {
            Image(this.bitmap)
              .width(96)
              .height(96)
              .borderWidth(1)
            Text('Original 原色').fontSize(11).fontColor(Color.Gray)
          }
          .layoutWeight(1)
          .alignItems(HorizontalAlign.Center)

          Column({ space: 4 }) {
            Image(this.bitmap)
              .width(96)
              .height(96)
              .renderMode(ImageRenderMode.Template)
              .borderWidth(1)
            Text('Template 黑白').fontSize(11).fontColor(Color.Gray)
          }
          .layoutWeight(1)
          .alignItems(HorizontalAlign.Center)
        }
        .width('100%')
      }
      Text('PixelMap 无 EXIF/编码信息，作为“PNG 类位图图源”代表参与 renderMode 验证。')
        .fontSize(12)
        .fontColor(Color.Gray)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  MainCompareCard(): void {
    Column({ space: 8 }) {
      Text('主对比区 · 位图 × 模式切换').fontSize(16).fontColor(Color.Gray)
      if (this.bitmap !== undefined) {
        Image(this.bitmap)
          .width(150)
          .height(150)
          .objectFit(ImageFit.Contain)
          .renderMode(this.mainMode().mode)
          .borderWidth(1)
      } else {
        Text('位图未就绪').fontSize(12).fontColor(Color.Gray)
      }
      Row({ space: 10 }) {
        ForEach(
          this.modeCells,
          (cell: RenderModeCell, index: number) => {
            Button(cell.label + ' (' + cell.value + ')')
              .fontSize(12)
              .backgroundColor(this.mainModeIndex === index ? '#007DFF' : '#E0E0E0')
              .fontColor(this.mainModeIndex === index ? Color.White : Color.Black)
              .onClick(() => {
                this.mainModeIndex = index;
              })
          },
          (cell: RenderModeCell) => cell.label
        )
      }
      .width('100%')
      .justifyContent(FlexAlign.Center)
      Text('当前: ' + this.mainMode().label + ' — ' + this.mainMode().desc)
        .fontSize(13)
        .fontColor(Color.Black)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  MatrixCard(): void {
    Column({ space: 8 }) {
      Text('图源 × 渲染模式约束矩阵').fontSize(16).fontColor(Color.Gray)
      // 表头
      Row({ space: 8 }) {
        Text('图源 \\ 模式')
          .fontSize(11)
          .fontColor(Color.Gray)
          .width(92)
        ForEach(
          this.modeCells,
          (cell: RenderModeCell) => {
            Text(cell.label)
              .fontSize(11)
              .fontColor('#007DFF')
              .width(96)
              .textAlign(TextAlign.Center)
          },
          (cell: RenderModeCell) => 'head_' + cell.label
        )
      }
      .width('100%')

      ForEach(
        this.sourceRows,
        (row: RenderSourceRow) => {
          Column({ space: 6 }) {
            Row({ space: 8 }) {
              Column({ space: 2 }) {
                Text(row.name)
                  .fontSize(12)
                  .fontWeight(FontWeight.Medium)
                Text(row.kind === 'bitmap' ? '位图' : 'SVG')
                  .fontSize(10)
                  .fontColor(row.supportRenderMode ? Color.Green : Color.Orange)
              }
              .width(92)
              .alignItems(HorizontalAlign.Start)

              ForEach(
                this.modeCells,
                (cell: RenderModeCell) => {
                  Column({ space: 2 }) {
                    if (this.sourceOfRow(row) !== undefined) {
                      Image(this.sourceOfRow(row))
                        .width(64)
                        .height(64)
                        .objectFit(ImageFit.Contain)
                        .renderMode(cell.mode)
                        .borderWidth(1)
                    } else {
                      Text('未就绪')
                        .fontSize(10)
                        .fontColor(Color.Gray)
                        .width(64)
                        .height(64)
                        .textAlign(TextAlign.Center)
                        .borderWidth(1)
                    }
                    Text(cell.label)
                      .fontSize(9)
                      .fontColor(Color.Gray)
                  }
                  .width(96)
                  .alignItems(HorizontalAlign.Center)
                },
                (cell: RenderModeCell) => row.name + '_' + cell.label
              )
            }
            .width('100%')
            Text(row.note)
              .fontSize(11)
              .fontColor(Color.Gray)
          }
          .width('100%')
          .padding(8)
          .backgroundColor('#FFFFFF')
          .borderRadius(8)
        },
        (row: RenderSourceRow) => row.name
      )
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  ExclusionCard(): void {
    Column({ space: 8 }) {
      Text('fillColor / colorFilter / renderMode 关系说明卡').fontSize(16).fontColor(Color.Gray)
      Column({ space: 6 }) {
        Text('● renderMode：SVG 类型图源不支持；设置 ColorFilter 时不生效；AnimatedDrawableDescriptor 不生效。')
          .fontSize(12)
        Text('● fillColor：仅对 SVG 图源生效（替换全部可绘制元素填充色）；AnimatedDrawableDescriptor 不生效。')
          .fontSize(12)
        Text('● colorFilter：位图滤镜矩阵；文档明确“设置该属性时 renderMode 属性设置不生效”，二者互斥。')
          .fontSize(12)
        Text('● 选型口诀：SVG 改色 → fillColor；位图黑白 → renderMode(Template)；位图改色 → 文档建议 colorFilter。')
          .fontSize(12)
          .fontColor('#007DFF')
      }
      .width('100%')
      .alignItems(HorizontalAlign.Start)

      Row({ space: 12 }) {
        Column({ space: 4 }) {
          Text('Template 位图')
            .fontSize(11)
            .fontColor(Color.Gray)
          if (this.bitmap !== undefined) {
            Image(this.bitmap)
              .width(72)
              .height(72)
              .renderMode(ImageRenderMode.Template)
              .borderWidth(1)
          }
          Text('renderMode 生效')
            .fontSize(10)
            .fontColor(Color.Green)
        }
        .layoutWeight(1)
        .alignItems(HorizontalAlign.Center)

        Column({ space: 4 }) {
          Text('Template 位图 + colorFilter')
            .fontSize(11)
            .fontColor(Color.Gray)
          if (this.bitmap !== undefined) {
            Image(this.bitmap)
              .width(72)
              .height(72)
              .renderMode(ImageRenderMode.Template)
              .colorFilter([
                0.2, 0.2, 0.2, 0, 20,
                0.2, 0.2, 0.2, 0, 20,
                0.2, 0.2, 0.2, 0, 20,
                0, 0, 0, 1, 0
              ])
              .borderWidth(1)
          }
          Text('文档：设置 ColorFilter 时 renderMode 不生效')
            .fontSize(10)
            .fontColor(Color.Orange)
        }
        .layoutWeight(1)
        .alignItems(HorizontalAlign.Center)

        Column({ space: 4 }) {
          Text('SVG + Template')
            .fontSize(11)
            .fontColor(Color.Gray)
          Image($r('app.media.testSvg'))
            .width(72)
            .height(72)
            .renderMode(ImageRenderMode.Template)
            .borderWidth(1)
          Text('SVG 不支持，保持原样')
            .fontSize(10)
            .fontColor(Color.Red)
        }
        .layoutWeight(1)
        .alignItems(HorizontalAlign.Center)
      }
      .width('100%')
      .alignItems(VerticalAlign.Top)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }
}
