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
import { image } from '@kit.ImageKit';
import { BusinessError } from '@kit.BasicServicesKit';
import { rpc } from '@kit.IPCKit';
import { colorSpaceManager } from '@kit.ArkGraphics2D';

class PixelMapParcel implements rpc.Parcelable {
  public pixelMap: image.PixelMap;
  public failCode: number = 0;

  constructor(pixelMap: image.PixelMap) {
    this.pixelMap = pixelMap;
  }

  marshalling(messageSequence: rpc.MessageSequence): boolean {
    this.pixelMap.marshalling(messageSequence);
    return true;
  }

  unmarshalling(messageSequence: rpc.MessageSequence): boolean {
    try {
      this.pixelMap = image.createPixelMapFromParcel(messageSequence);
      return true;
    } catch (err) {
      let e = err as BusinessError;
      this.failCode = e.code;
      return false;
    }
  }
}

@Entry
@Component
struct ImagePixelMapTransformExample {
  @State baseImage: image.PixelMap | undefined = undefined;
  @State previewImage: image.PixelMap | undefined = undefined;
  @State alphaImage: image.PixelMap | undefined = undefined;
  @State statusText: string = '请先点击"创建渐变图"';
  @State results: string[] = [];
  private imgWidth: number = 60;
  private imgHeight: number = 60;

  build() {
    Scroll() {
      Column({ space: 12 }) {
        Text('PixelMap 变换与编辑示例').fontSize(22).fontWeight(FontWeight.Bold).margin({ top: 16, bottom: 4 })
        Text('状态: ' + this.statusText).fontSize(14).fontColor(Color.Gray)
        Row({ space: 10 }) {
          Button('创建渐变图').fontSize(13).layoutWeight(1).onClick(() => { this.runCreate(); })
          Button('分配器创建').fontSize(13).layoutWeight(1).onClick(() => { this.runAllocator(); })
        }.width('100%')
        Row({ space: 10 }) {
          Button('预乘反预乘').fontSize(13).layoutWeight(1).onClick(() => { this.runAlphaConvert(); })
          Button('裁剪与缩放').fontSize(13).layoutWeight(1).onClick(() => { this.runCropScale(); })
        }.width('100%')
        Row({ space: 10 }) {
          Button('平移旋转翻转').fontSize(13).layoutWeight(1).onClick(() => { this.runTransform(); })
          Button('透明度与Alpha').fontSize(13).layoutWeight(1).onClick(() => { this.runOpacityAlpha(); })
        }.width('100%')
        Row({ space: 10 }) {
          Button('像素读写').fontSize(13).layoutWeight(1).onClick(() => { this.runPixelRW(); })
          Button('信息与格式').fontSize(13).layoutWeight(1).onClick(() => { this.runInfo(); })
        }.width('100%')
        Row({ space: 10 }) {
          Button('色彩空间').fontSize(13).layoutWeight(1).onClick(() => { this.runColorSpace(); })
          Button('序列化与释放').fontSize(13).layoutWeight(1).onClick(() => { this.runMarshalRelease(); })
        }.width('100%')
        Row({ space: 10 }) {
          Column({ space: 6 }) {
            Text('变换结果预览').fontSize(14)
            if (this.previewImage) {
              Image(this.previewImage).width(140).height(140).objectFit(ImageFit.Contain)
                .border({ width: 1, color: Color.Blue, radius: 6 })
            }
          }.layoutWeight(1)
          Column({ space: 6 }) {
            Text('Alpha 通道图').fontSize(14)
            if (this.alphaImage) {
              Image(this.alphaImage).width(140).height(140).objectFit(ImageFit.Contain)
                .border({ width: 1, color: Color.Gray, radius: 6 })
            }
          }.layoutWeight(1)
        }.width('100%')
        Column({ space: 4 }) {
          Text('运行结果').fontSize(16).fontWeight(FontWeight.Medium)
          ForEach(this.results, (item: string, index: number) => {
            Text(`${index + 1}. ${item}`).fontSize(12).fontColor(Color.Black).width('100%')
              .maxLines(2).textOverflow({ overflow: TextOverflow.Ellipsis })
          }, (item: string, index: number): string => index.toString() + item)
        }
        .width('100%')
        .padding(10)
        .backgroundColor('#F5F5F5')
        .borderRadius(8)
        .alignItems(HorizontalAlign.Start)
      }
      .width('100%')
      .padding({ left: 16, right: 16, bottom: 20 })
    }
    .width('100%')
    .height('100%')
    .backgroundColor(Color.White)
    .scrollBar(BarState.Auto)
    .align(Alignment.Top)
  }

  addResult(line: string): void {
    this.results = this.results.concat(line);
  }

  makeGradientBuffer(width: number, height: number): ArrayBuffer {
    const buffer: ArrayBuffer = new ArrayBuffer(width * height * 4);
    const data: Uint8Array = new Uint8Array(buffer);
    for (let y: number = 0; y < height; y++) {
      for (let x: number = 0; x < width; x++) {
        const idx: number = (y * width + x) * 4;
        data[idx] = Math.floor(255 * x / (width - 1));
        data[idx + 1] = Math.floor(255 * y / (height - 1));
        data[idx + 2] = 128;
        data[idx + 3] = 255;
      }
    }
    return buffer;
  }

  makeOptions(width: number, height: number, alphaType: image.AlphaType): image.InitializationOptions {
    const size: image.Size = { width: width, height: height };
    const opts: image.InitializationOptions = {
      size: size,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      editable: true,
      alphaType: alphaType,
      scaleMode: image.ScaleMode.FIT_TARGET_SIZE
    };
    return opts;
  }

  async runCreate(): Promise<void> {
    const buffer: ArrayBuffer = this.makeGradientBuffer(this.imgWidth, this.imgHeight);
    const opts: image.InitializationOptions = this.makeOptions(this.imgWidth, this.imgHeight, image.AlphaType.PREMUL);
    try {
      const asyncMap: image.PixelMap = await image.createPixelMap(buffer, opts);
      this.baseImage = asyncMap;
      const asyncInfo: image.ImageInfo = asyncMap.getImageInfoSync();
      this.addResult('createPixelMap 成功 ' + asyncInfo.size.width + 'x' + asyncInfo.size.height);
      const syncMap: image.PixelMap = image.createPixelMapSync(buffer, opts);
      this.previewImage = syncMap;
      this.addResult('createPixelMapSync(数据) 成功 isEditable=' + syncMap.isEditable);
      const emptyMap: image.PixelMap = image.createPixelMapSync(opts);
      this.addResult('createPixelMapSync(空白) 成功 isStrideAlignment=' + emptyMap.isStrideAlignment);
      await emptyMap.release();
      this.statusText = '底图与预览已创建';
    } catch (err) { let e = err as BusinessError; this.addResult('创建失败 code=' + e.code); }
  }

  async runAllocator(): Promise<void> {
    const buffer: ArrayBuffer = this.makeGradientBuffer(this.imgWidth, this.imgHeight);
    const opts: image.InitializationOptions = this.makeOptions(this.imgWidth, this.imgHeight, image.AlphaType.UNPREMUL);
    try {
      const autoMap: image.PixelMap = await image.createPixelMapUsingAllocator(buffer, opts, image.AllocatorType.AUTO);
      this.addResult('createPixelMapUsingAllocator 成功 bytes=' + autoMap.getPixelBytesNumber());
      await autoMap.release();
      const dmaMap: image.PixelMap = image.createPixelMapUsingAllocatorSync(buffer, opts, image.AllocatorType.DMA);
      this.addResult('createPixelMapUsingAllocatorSync(数据) 成功 rowBytes=' + dmaMap.getBytesNumberPerRow());
      dmaMap.release((err: BusinessError) => {
        this.addResult(err ? 'release(回调) 失败 code=' + err.code : 'release(回调) 成功');
      });
      const emptyMap: image.PixelMap = image.createPixelMapUsingAllocatorSync(opts, image.AllocatorType.SHARE_MEMORY);
      const emptyInfo: image.ImageInfo = emptyMap.getImageInfoSync();
      this.addResult('createPixelMapUsingAllocatorSync(空白) 成功 ' + emptyInfo.size.width + 'x' + emptyInfo.size.height);
      await emptyMap.release();
      this.statusText = '分配器创建完成';
    } catch (err) { let e = err as BusinessError; this.addResult('分配器创建失败 code=' + e.code); }
  }

  async runAlphaConvert(): Promise<void> {
    const buffer: ArrayBuffer = this.makeGradientBuffer(8, 8);
    const unpreOpts: image.InitializationOptions = this.makeOptions(8, 8, image.AlphaType.UNPREMUL);
    const preOpts: image.InitializationOptions = this.makeOptions(8, 8, image.AlphaType.PREMUL);
    try {
      const srcUnpre: image.PixelMap = image.createPixelMapSync(buffer, unpreOpts);
      const dstPre: image.PixelMap = image.createPixelMapSync(preOpts);
      await image.createPremultipliedPixelMap(srcUnpre, dstPre);
      this.addResult('createPremultipliedPixelMap 成功');
      await srcUnpre.release();
      await dstPre.release();
      const srcPre: image.PixelMap = image.createPixelMapSync(buffer, preOpts);
      const dstUnpre: image.PixelMap = image.createPixelMapSync(unpreOpts);
      image.createUnpremultipliedPixelMap(srcPre, dstUnpre, (err: BusinessError) => {
        this.addResult(err ? 'createUnpremultipliedPixelMap 失败 code=' + err.code : 'createUnpremultipliedPixelMap 成功');
        srcPre.release();
        dstUnpre.release();
      });
      this.statusText = '预乘/反预乘转换完成';
    } catch (err) { let e = err as BusinessError; this.addResult('预乘转换失败 code=' + e.code); }
  }

  async runCropScale(): Promise<void> {
    const base: image.PixelMap | undefined = this.baseImage;
    if (base === undefined) { this.addResult('请先创建底图'); return; }
    try {
      const workA: image.PixelMap = base.cloneSync();
      const workB: image.PixelMap = await base.clone();
      this.addResult('cloneSync/clone 成功');
      const regionA: image.Region = { x: 6, y: 6, size: { width: 40, height: 30 } };
      const regionB: image.Region = { x: 0, y: 10, size: { width: 44, height: 44 } };
      await workA.crop(regionA);
      workB.cropSync(regionB);
      this.addResult('crop/cropSync 成功');
      await workA.scale(1.5, 1.0);
      workB.scaleSync(0.8, 0.8);
      this.addResult('scale/scaleSync 成功');
      await workA.scale(1.2, 1.2, image.AntiAliasingLevel.LOW);
      workB.scaleSync(1.1, 1.1, image.AntiAliasingLevel.MEDIUM);
      this.addResult('scale/scaleSync(抗锯齿) 成功');
      const scaledA: image.PixelMap = await workA.createScaledPixelMap(0.6, 0.6, image.AntiAliasingLevel.HIGH);
      const scaledB: image.PixelMap = workB.createScaledPixelMapSync(0.7, 0.7);
      const infoA: image.ImageInfo = scaledA.getImageInfoSync();
      const infoB: image.ImageInfo = scaledB.getImageInfoSync();
      this.addResult('createScaledPixelMap 成功 ' + infoA.size.width + 'x' + infoA.size.height);
      this.addResult('createScaledPixelMapSync 成功 ' + infoB.size.width + 'x' + infoB.size.height);
      await workB.release();
      await scaledB.release();
      await workA.release();
      this.previewImage = scaledA;
      this.statusText = '裁剪与缩放完成';
    } catch (err) { let e = err as BusinessError; this.addResult('裁剪缩放失败 code=' + e.code); }
  }

  async runTransform(): Promise<void> {
    const base: image.PixelMap | undefined = this.baseImage;
    if (base === undefined) { this.addResult('请先创建底图'); return; }
    try {
      const work: image.PixelMap = await base.clone();
      await work.translate(8, 12);
      work.translateSync(-4, -4);
      this.addResult('translate/translateSync 成功');
      await work.rotate(45);
      work.rotateSync(90);
      this.addResult('rotate/rotateSync 成功');
      const bufSmall: ArrayBuffer = this.makeGradientBuffer(16, 16);
      const optsSmall: image.InitializationOptions = this.makeOptions(16, 16, image.AlphaType.PREMUL);
      const tempRot: image.PixelMap = image.createPixelMapSync(bufSmall, optsSmall);
      tempRot.rotate(30, (err: BusinessError) => {
        this.addResult(err ? 'rotate(回调) 失败 code=' + err.code : 'rotate(回调) 成功');
        tempRot.release();
      });
      await work.flip(true, false);
      work.flipSync(false, true);
      this.addResult('flip/flipSync 成功');
      const tempFlip: image.PixelMap = image.createPixelMapSync(bufSmall, optsSmall);
      tempFlip.flip(false, true, (err: BusinessError) => {
        this.addResult(err ? 'flip(回调) 失败 code=' + err.code : 'flip(回调) 成功');
        tempFlip.release();
      });
      this.previewImage = work;
      this.statusText = '平移旋转翻转完成';
    } catch (err) { let e = err as BusinessError; this.addResult('变换失败 code=' + e.code); }
  }

  async runOpacityAlpha(): Promise<void> {
    const base: image.PixelMap | undefined = this.baseImage;
    if (base === undefined) { this.addResult('请先创建底图'); return; }
    try {
      const work: image.PixelMap = await base.clone();
      await work.opacity(0.6);
      work.opacitySync(0.8);
      this.addResult('opacity/opacitySync 成功');
      const temp: image.PixelMap = image.createPixelMapSync(this.makeGradientBuffer(16, 16),
        this.makeOptions(16, 16, image.AlphaType.PREMUL));
      temp.opacity(0.5, (err: BusinessError) => {
        this.addResult(err ? 'opacity(回调) 失败 code=' + err.code : 'opacity(回调) 成功');
        temp.release();
      });
      const alphaMap: image.PixelMap = await work.createAlphaPixelmap();
      const alphaInfo: image.ImageInfo = alphaMap.getImageInfoSync();
      this.addResult('createAlphaPixelmap 成功 pixelFormat=' + alphaInfo.pixelFormat);
      const alphaSync: image.PixelMap = work.createAlphaPixelmapSync();
      this.addResult('createAlphaPixelmapSync 成功');
      await alphaSync.release();
      this.alphaImage = alphaMap;
      this.previewImage = work;
      this.statusText = '透明度与Alpha完成';
    } catch (err) { let e = err as BusinessError; this.addResult('透明度失败 code=' + e.code); }
  }

  async runPixelRW(): Promise<void> {
    const base: image.PixelMap | undefined = this.baseImage;
    if (base === undefined) { this.addResult('请先创建底图'); return; }
    try {
      const work: image.PixelMap = await base.clone();
      const readArea: image.PositionArea = {
        pixels: new ArrayBuffer(4 * 8 * 4),
        offset: 0,
        stride: 16,
        region: { x: 0, y: 0, size: { width: 4, height: 8 } }
      };
      await work.readPixels(readArea);
      const readArr: Uint8Array = new Uint8Array(readArea.pixels);
      this.addResult('readPixels 成功 首字节=' + readArr[0] + ',' + readArr[1]);
      work.readPixelsSync(readArea);
      this.addResult('readPixelsSync 成功');
      const writeArea: image.PositionArea = {
        pixels: new ArrayBuffer(4 * 4 * 4),
        offset: 0,
        stride: 16,
        region: { x: 2, y: 2, size: { width: 4, height: 4 } }
      };
      const writeArr: Uint8Array = new Uint8Array(writeArea.pixels);
      for (let i: number = 0; i < writeArr.length; i++) {
        writeArr[i] = 255 - i;
      }
      await work.writePixels(writeArea);
      work.writePixelsSync(writeArea);
      this.addResult('writePixels/writePixelsSync 成功');
      const fullBuf: ArrayBuffer = new ArrayBuffer(work.getPixelBytesNumber());
      const fullArr: Uint8Array = new Uint8Array(fullBuf);
      for (let i: number = 0; i < fullArr.length; i++) {
        fullArr[i] = (i % 251) + 4;
      }
      await work.writeBufferToPixels(fullBuf);
      work.writeBufferToPixelsSync(fullBuf);
      this.addResult('writeBufferToPixels/Sync 成功');
      const outBuf: ArrayBuffer = new ArrayBuffer(work.getPixelBytesNumber());
      work.readPixelsToBufferSync(outBuf);
      this.addResult('readPixelsToBufferSync 成功 bytes=' + outBuf.byteLength);
      this.previewImage = work;
      this.statusText = '像素读写完成';
    } catch (err) { let e = err as BusinessError; this.addResult('像素读写失败 code=' + e.code); }
  }

  async runInfo(): Promise<void> {
    const pm: image.PixelMap | undefined = this.baseImage;
    if (pm === undefined) {
      this.addResult('请先创建底图');
      return;
    }
    try {
      const info: image.ImageInfo = pm.getImageInfoSync();
      this.addResult('getImageInfoSync: ' + info.size.width + 'x' + info.size.height +
        ' density=' + info.density + ' isHdr=' + info.isHdr);
      this.addResult('getBytesNumberPerRow=' + pm.getBytesNumberPerRow() +
        ' getPixelBytesNumber=' + pm.getPixelBytesNumber() + ' getDensity=' + pm.getDensity());
      this.addResult('isEditable=' + pm.isEditable + ' isStrideAlignment=' + pm.isStrideAlignment);
      try {
        pm.setMemoryNameSync('TransformDemoMemory');
        this.addResult('setMemoryNameSync 成功');
      } catch (err) { let e = err as BusinessError; this.addResult('setMemoryNameSync 失败 code=' + e.code); }
      const conv: image.PixelMap = await pm.clone();
      await conv.convertPixelFormat(image.PixelMapFormat.NV12);
      const nvInfo: image.ImageInfo = conv.getImageInfoSync();
      this.addResult('convertPixelFormat 转NV12 成功 pixelFormat=' + nvInfo.pixelFormat);
      await conv.convertPixelFormat(image.PixelMapFormat.RGBA_8888);
      this.addResult('convertPixelFormat 转回RGBA_8888 成功');
      await conv.release();
      this.statusText = '信息与格式完成';
    } catch (err) { let e = err as BusinessError; this.addResult('信息获取失败 code=' + e.code); }
  }

  async runColorSpace(): Promise<void> {
    const base: image.PixelMap | undefined = this.baseImage;
    if (base === undefined) { this.addResult('请先创建底图'); return; }
    const pm: image.PixelMap = await base.clone();
    try {
      const csm: colorSpaceManager.ColorSpaceManager = pm.getColorSpace();
      this.addResult('getColorSpace: ' + csm.getColorSpaceName());
    } catch (err) { let e = err as BusinessError; this.addResult('getColorSpace 失败 code=' + e.code); }
    const srgb: colorSpaceManager.ColorSpaceManager = colorSpaceManager.create(colorSpaceManager.ColorSpace.SRGB);
    try {
      pm.setColorSpace(srgb);
      await pm.applyColorSpace(srgb);
      this.addResult('setColorSpace/applyColorSpace 成功');
    } catch (err) { let e = err as BusinessError; this.addResult('色彩空间设置失败 code=' + e.code); }
    const p3: colorSpaceManager.ColorSpaceManager = colorSpaceManager.create(colorSpaceManager.ColorSpace.DISPLAY_P3);
    pm.applyColorSpace(p3, (err: BusinessError) => {
      this.addResult(err ? 'applyColorSpace(回调) 失败 code=' + err.code : 'applyColorSpace(回调) 成功');
    });
    try {
      await pm.toSdr();
      this.addResult('toSdr 成功');
    } catch (err) { let e = err as BusinessError; this.addResult('toSdr 失败 code=' + e.code); }
    this.previewImage = pm;
    this.statusText = '色彩空间完成';
  }

  async runMarshalRelease(): Promise<void> {
    const base: image.PixelMap | undefined = this.baseImage;
    if (base === undefined) { this.addResult('请先创建底图'); return; }
    try {
      const pm: image.PixelMap = await base.clone();
      const data: rpc.MessageSequence = rpc.MessageSequence.create();
      const writeSeq: PixelMapParcel = new PixelMapParcel(pm);
      data.writeParcelable(writeSeq);
      this.addResult('marshalling 成功');
      const readSeq: PixelMapParcel = new PixelMapParcel(pm);
      data.readParcelable(readSeq);
      if (readSeq.failCode === 0) {
        const restoredInfo: image.ImageInfo = readSeq.pixelMap.getImageInfoSync();
        this.addResult('createPixelMapFromParcel 成功 ' + restoredInfo.size.width + 'x' + restoredInfo.size.height);
      } else {
        this.addResult('createPixelMapFromParcel 失败 code=' + readSeq.failCode);
      }
      const seedOpts: image.InitializationOptions = { size: { height: 4, width: 6 } };
      const seed: image.PixelMap = await image.createPixelMap(new ArrayBuffer(96), seedOpts);
      const data2: rpc.MessageSequence = rpc.MessageSequence.create();
      data2.writeParcelable(new PixelMapParcel(pm));
      const unmarshalled: image.PixelMap = await seed.unmarshalling(data2);
      const umInfo: image.ImageInfo = unmarshalled.getImageInfoSync();
      this.addResult('unmarshalling 成功 ' + umInfo.size.width + 'x' + umInfo.size.height);
      await unmarshalled.release();
      await seed.release();
      await readSeq.pixelMap.release();
      pm.setTransferDetached(true);
      await pm.release();
      this.addResult('setTransferDetached/release 成功');
      const temp: image.PixelMap = image.createPixelMapSync(this.makeGradientBuffer(8, 8),
        this.makeOptions(8, 8, image.AlphaType.PREMUL));
      temp.release((err: BusinessError) => {
        this.addResult(err ? 'release(回调) 失败 code=' + err.code : 'release(回调) 成功');
      });
      this.previewImage = undefined;
      this.statusText = '序列化与释放完成';
    } catch (err) { let e = err as BusinessError; this.addResult('序列化失败 code=' + e.code); }
  }
}
