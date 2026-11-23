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
 * resizable slice 与 lattice 两写法对比示例 (API 11+ / 12+)
 * ========================================
 * 功能说明:
 * 本示例对 resizable(ResizableOptions) 的两种写法做参数边界级对比：
 * - slice: EdgeWidths 九宫格拉伸（API 11+，数字默认 vp，字符串可带 px）
 * - lattice: DrawingLattice 矩形网格拉伸（API 12+，数字默认 px，
 *   由 @ohos.graphics.drawing 的 createImageLattice 生成，网格线数组遍历生成）
 * 差异化于 ImageLatticeExample（直接 lattice 展示）与 ImageSliceStretchExample
 * （slice 预设+滑杆）：本页专注参数边界值（bottom/right 须同时大于 0、
 * top+bottom 超原图高、left+right 超原图宽的不生效条件）、
 * lattice 分割数组的循环生成与 fRectTypes 遍历生成、onComplete 尺寸回显。
 * 位图源由 @kit.ImageKit image.createPixelMap 运行时生成（media 目录无位图）。
 *
 * ========================================
 * 涉及的 Image API 与文档约束
 * ========================================
 * 1. resizable(value: ResizableOptions) - API 11+；slice: EdgeWidths，
 *    lattice: DrawingLattice (API 12+，偶行偶列网格固定，数字默认 px)
 * 2. onComplete 尺寸回显 (contentWidth 等字段 API 10+)
 * 3. top+bottom 大于原图高，或 left+right 大于原图宽时 ResizableOptions 不生效
 * 4. slice 只有当 bottom 和 right 同时大于 0 时生效
 * 5. 动图、AnimatedDrawableDescriptor 与 SVG 图源设置 resizable 不生效
 * 6. 设置合法 resizable 时，objectRepeat、antialiased、orientation 不生效
 */

import { image } from '@kit.ImageKit';
import { drawing } from '@kit.ArkGraphics2D';
import { BusinessError } from '@kit.BasicServicesKit';

interface SliceBoundaryCase {
  name: string;
  top: number;
  right: number;
  bottom: number;
  left: number;
  expect: string;
  valid: boolean;
}

interface LatticeCase {
  name: string;
  fXCount: number;
  fYCount: number;
  withRectTypes: boolean;
  expect: string;
}

interface ImageCompleteEcho {
  width: number;
  height: number;
  componentWidth: number;
  componentHeight: number;
  loadingStatus: number;
  contentWidth?: number;
  contentHeight?: number;
  contentOffsetX?: number;
  contentOffsetY?: number;
}

@Entry
@Component
struct ImageResizableSliceLatticeExample {
  @State bitmap: image.PixelMap | undefined = undefined;
  @State bitmapStatus: string = '正在创建位图源…';
  @State boundaryIndex: number = 0;
  @State latticeIndex: number = 0;
  @State completeText: string = '等待 onComplete 回调';
  private readonly sourceSize: number = 160;
  private readonly targetSize: number = 240;
  // slice 参数边界用例（统一以 px 字符串传入，规避 vp/px 单位歧义）
  private boundaryCases: SliceBoundaryCase[] = [
    {
      name: '合法九宫格',
      top: 20, right: 20, bottom: 20, left: 20,
      expect: '四边 20px 保留，中心与边中拉伸：生效',
      valid: true
    },
    {
      name: 'bottom=0',
      top: 20, right: 20, bottom: 0, left: 20,
      expect: '文档：仅当 bottom 和 right 同时大于 0 才生效 → 不生效',
      valid: false
    },
    {
      name: 'top+bottom 超原图高',
      top: 90, right: 20, bottom: 90, left: 20,
      expect: '90+90=180 > 原图高 160 → ResizableOptions 不生效',
      valid: false
    },
    {
      name: 'left+right 超原图宽',
      top: 20, right: 100, bottom: 20, left: 80,
      expect: '80+100=180 > 原图宽 160 → ResizableOptions 不生效',
      valid: false
    }
  ];
  // lattice 网格用例：fXCount/fYCount 即分割线数组元素个数（文档要求与数组长度一致，范围[0,5]）
  private latticeCases: LatticeCase[] = [
    {
      name: '3×3 分割九宫格',
      fXCount: 3, fYCount: 3,
      withRectTypes: false,
      expect: '偶行偶列网格固定，其余拉伸'
    },
    {
      name: '2×2 分割四象限',
      fXCount: 2, fYCount: 2,
      withRectTypes: false,
      expect: '中心十字固定、边缘拉伸'
    },
    {
      name: '4×4 + fRectTypes',
      fXCount: 4, fYCount: 4,
      withRectTypes: true,
      expect: 'fRectTypes 遍历生成 (4+1)×(4+1) 项，奇偶交替 DEFAULT/TRANSPARENT'
    }
  ];
  private lattices: DrawingLattice[] = [];

  async aboutToAppear(): Promise<void> {
    await this.createCornerMarkBitmap();
    this.buildLattices();
  }

  // 运行时生成 160×160 位图：四角实心块 + 中线十字，拉伸后形变一目了然
  private async createCornerMarkBitmap(): Promise<void> {
    const size: number = this.sourceSize;
    const buffer: ArrayBuffer = new ArrayBuffer(size * size * 4);
    const pixels: Uint8Array = new Uint8Array(buffer);
    for (let y: number = 0; y < size; y++) {
      for (let x: number = 0; x < size; x++) {
        const offset: number = (y * size + x) * 4;
        const inCorner: boolean =
          (x < 24 || x >= size - 24) && (y < 24 || y >= size - 24);
        const inCross: boolean = x === size / 2 || y === size / 2;
        pixels[offset] = inCorner ? 0xE0 : 0x30;
        pixels[offset + 1] = inCross ? 0xFF : 0x60;
        pixels[offset + 2] = inCorner ? 0x40 : 0xE0;
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
      this.bitmapStatus = '位图源就绪：160×160（四角色块 + 中线十字）';
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      this.bitmapStatus = '位图创建失败: code=' + err.code + ', message=' + err.message;
    }
  }

  // 遍历生成分割线数组：生成恰好 count 条分割线（文档要求 count 与数组长度一致），
  // 将 0..sourceSize 均分为 count+1 段；lattice 数字默认单位 px，元素需为整数。
  private buildDivs(count: number): number[] {
    const divs: number[] = [];
    for (let i: number = 1; i <= count; i++) {
      divs.push(Math.round(this.sourceSize * i / (count + 1)));
    }
    return divs;
  }

  // 遍历生成 fRectTypes 数组：最小尺寸 (fXCount+1)*(fYCount+1)，奇偶交替类型
  private buildRectTypes(fXCount: number, fYCount: number): drawing.RectType[] {
    const types: drawing.RectType[] = [];
    const total: number = (fXCount + 1) * (fYCount + 1);
    for (let i: number = 0; i < total; i++) {
      types.push(i % 2 === 0 ? drawing.RectType.DEFAULT : drawing.RectType.TRANSPARENT);
    }
    return types;
  }

  private buildLattices(): void {
    const lattices: DrawingLattice[] = [];
    for (let i: number = 0; i < this.latticeCases.length; i++) {
      const item: LatticeCase = this.latticeCases[i];
      const xDivs: number[] = this.buildDivs(item.fXCount);
      const yDivs: number[] = this.buildDivs(item.fYCount);
      if (item.withRectTypes) {
        const rectTypes: drawing.RectType[] = this.buildRectTypes(item.fXCount, item.fYCount);
        lattices.push(drawing.Lattice.createImageLattice(xDivs, yDivs, item.fXCount, item.fYCount, null, rectTypes));
      } else {
        lattices.push(drawing.Lattice.createImageLattice(xDivs, yDivs, item.fXCount, item.fYCount));
      }
    }
    this.lattices = lattices;
  }

  private currentBoundary(): SliceBoundaryCase {
    return this.boundaryCases[this.boundaryIndex];
  }

  private currentLattice(): DrawingLattice | undefined {
    return this.lattices.length > this.latticeIndex ? this.lattices[this.latticeIndex] : undefined;
  }

  private onImageComplete(event?: ImageCompleteEcho): void {
    if (event === undefined) {
      this.completeText = 'onComplete 未返回尺寸';
      return;
    }
    this.completeText = '图源 ' + event.width + '×' + event.height +
      ' · 组件 ' + event.componentWidth + '×' + event.componentHeight +
      ' · 绘制 ' + (event.contentWidth ?? 0) + '×' + (event.contentHeight ?? 0) +
      ' · 偏移 ' + (event.contentOffsetX ?? 0) + ',' + (event.contentOffsetY ?? 0) +
      ' · loadingStatus=' + event.loadingStatus;
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('resizable slice 与 lattice 对比示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        this.SourceStatusCard()
        this.SliceBoundaryCard()
        this.LatticeCard()
        this.SideBySideCard()
        this.CompleteEchoCard()
        this.ConstraintCard()

        Text('lattice 分割数组与 fRectTypes 均由循环遍历生成；slice 传数字默认 vp、传字符串可指定 px，lattice 数字默认 px——两写法单位规则不同是选型关键。')
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
  SourceStatusCard(): void {
    Column({ space: 8 }) {
      Text('位图源（image.createPixelMap, API 8+）').fontSize(16).fontColor(Color.Gray)
      Text(this.bitmapStatus)
        .fontSize(12)
        .fontColor(this.bitmap === undefined ? Color.Orange : Color.Green)
      if (this.bitmap !== undefined) {
        Image(this.bitmap)
          .width(this.sourceSize)
          .height(this.sourceSize)
          .borderWidth(1)
      }
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  SliceBoundaryCard(): void {
    Column({ space: 8 }) {
      Text('写法一 · slice 参数边界值 (API 11+)').fontSize(16).fontColor(Color.Gray)
      Flex({ wrap: FlexWrap.Wrap }) {
        ForEach(
          this.boundaryCases,
          (item: SliceBoundaryCase, index: number) => {
            Button(item.name)
              .fontSize(11)
              .backgroundColor(this.boundaryIndex === index ? '#007DFF' : '#E0E0E0')
              .fontColor(this.boundaryIndex === index ? Color.White : Color.Black)
              .height(30)
              .margin({ right: 6, bottom: 6 })
              .onClick(() => {
                this.boundaryIndex = index;
              })
          },
          (item: SliceBoundaryCase) => item.name
        )
      }
      .width('100%')

      if (this.bitmap !== undefined) {
        Row({ space: 12 }) {
          Column({ space: 4 }) {
            Image(this.bitmap)
              .width(this.targetSize)
              .height(this.targetSize)
              .objectFit(ImageFit.Fill)
              .borderWidth(1)
            Text('未 resizable 直接拉伸').fontSize(11).fontColor(Color.Gray)
          }
          .layoutWeight(1)
          .alignItems(HorizontalAlign.Center)

          Column({ space: 4 }) {
            Image(this.bitmap)
              .resizable({
                slice: {
                  top: `${this.currentBoundary().top}px`,
                  right: `${this.currentBoundary().right}px`,
                  bottom: `${this.currentBoundary().bottom}px`,
                  left: `${this.currentBoundary().left}px`
                }
              })
              .width(this.targetSize)
              .height(this.targetSize)
              .objectFit(ImageFit.Fill)
              .borderWidth(1)
            Text('resizable slice').fontSize(11).fontColor(Color.Gray)
          }
          .layoutWeight(1)
          .alignItems(HorizontalAlign.Center)
        }
        .width('100%')
      }

      Text('参数(px): top=' + this.currentBoundary().top + ' right=' + this.currentBoundary().right +
        ' bottom=' + this.currentBoundary().bottom + ' left=' + this.currentBoundary().left)
        .fontSize(12)
        .fontFamily('monospace')
      Text('预期: ' + this.currentBoundary().expect)
        .fontSize(12)
        .fontColor(this.currentBoundary().valid ? Color.Green : Color.Red)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  LatticeCard(): void {
    Column({ space: 8 }) {
      Text('写法二 · lattice 网格拉伸 (ResizableOptions.lattice, API 12+)').fontSize(16).fontColor(Color.Gray)
      Row({ space: 10 }) {
        ForEach(
          this.latticeCases,
          (item: LatticeCase, index: number) => {
            Button(item.name)
              .fontSize(11)
              .backgroundColor(this.latticeIndex === index ? '#007DFF' : '#E0E0E0')
              .fontColor(this.latticeIndex === index ? Color.White : Color.Black)
              .height(30)
              .onClick(() => {
                this.latticeIndex = index;
              })
          },
          (item: LatticeCase) => item.name
        )
      }
      .width('100%')
      .justifyContent(FlexAlign.SpaceBetween)

      if (this.bitmap !== undefined && this.currentLattice() !== undefined) {
        Image(this.bitmap)
          .resizable({
            lattice: this.currentLattice()
          })
          .width(this.targetSize)
          .height(this.targetSize)
          .objectFit(ImageFit.Fill)
          .borderWidth(1)
      }
      Text('用例: ' + this.latticeCases[this.latticeIndex].name + ' — ' + this.latticeCases[this.latticeIndex].expect)
        .fontSize(12)
        .fontColor(Color.Black)
      Text('xDivs/yDivs 由 buildDivs 遍历生成（长度与 fXCount/fYCount 一致，范围[0,5]）；fRectTypes 遍历生成 (fXCount+1)×(fYCount+1) 项。')
        .fontSize(11)
        .fontColor(Color.Gray)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  SideBySideCard(): void {
    Column({ space: 8 }) {
      Text('两写法同尺寸对比').fontSize(16).fontColor(Color.Gray)
      if (this.bitmap !== undefined && this.currentLattice() !== undefined) {
        Row({ space: 12 }) {
          Column({ space: 4 }) {
            Image(this.bitmap)
              .resizable({
                slice: { top: '20px', right: '20px', bottom: '20px', left: '20px' }
              })
              .width(this.targetSize)
              .height(this.targetSize)
              .objectFit(ImageFit.Fill)
              .borderWidth(1)
            Text('slice 20px 四边').fontSize(11).fontColor(Color.Green)
          }
          .layoutWeight(1)
          .alignItems(HorizontalAlign.Center)

          Column({ space: 4 }) {
            Image(this.bitmap)
              .resizable({
                lattice: this.currentLattice()
              })
              .width(this.targetSize)
              .height(this.targetSize)
              .objectFit(ImageFit.Fill)
              .borderWidth(1)
            Text('lattice ' + this.latticeCases[this.latticeIndex].name).fontSize(11).fontColor('#007DFF')
          }
          .layoutWeight(1)
          .alignItems(HorizontalAlign.Center)
        }
        .width('100%')
      }
      Text('slice 固定四角拉伸边中；lattice 固定偶行偶列网格，网格更密时中心细节保留更好。')
        .fontSize(12).fontColor(Color.Gray)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  CompleteEchoCard(): void {
    Column({ space: 8 }) {
      Text('onComplete 尺寸回显').fontSize(16).fontColor(Color.Gray)
      if (this.bitmap !== undefined) {
        Image(this.bitmap)
          .resizable({
            slice: { top: '20px', right: '20px', bottom: '20px', left: '20px' }
          })
          .width(this.targetSize)
          .height(this.targetSize)
          .objectFit(ImageFit.Fill)
          .borderWidth(1)
          .onComplete((event?: ImageCompleteEcho): void => {
            this.onImageComplete(event);
          })
      }
      Text(this.completeText)
        .fontSize(12)
        .fontColor(Color.Black)
      Text('loadingStatus=0 表示数据加载成功，=1 表示解码成功；contentWidth/contentHeight 等仅在解码成功时有效 (API 10+)。')
        .fontSize(11)
        .fontColor(Color.Gray)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
  }

  @Builder
  ConstraintCard(): void {
    Column({ space: 6 }) {
      Text('resizable 约束清单').fontSize(16).fontColor(Color.Gray)
      Text('● 动图、AnimatedDrawableDescriptor、SVG 图源：resizable 不生效。')
        .fontSize(12)
      Text('● top+bottom > 原图高 或 left+right > 原图宽：整体不生效（本页边界卡验证）。')
        .fontSize(12)
      Text('● slice 需 bottom 与 right 同时大于 0 才生效。')
        .fontSize(12)
      Text('● 设置合法 resizable 时 objectRepeat / antialiased / orientation 不生效；lattice 对 backgroundImageResizable 不生效；拉伸同样作用于拖拽缩略图与占位图。')
        .fontSize(12)
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
    .alignItems(HorizontalAlign.Start)
  }
}
