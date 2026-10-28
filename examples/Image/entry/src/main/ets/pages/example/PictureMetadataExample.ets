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

class PictureSequence implements rpc.Parcelable {
  public picture: image.Picture | null = null;
  constructor(conPicture: image.Picture) { this.picture = conPicture; }
  marshalling(messageSequence: rpc.MessageSequence): boolean {
    if (this.picture == null) { return false; }
    this.picture.marshalling(messageSequence);
    return true;
  }
  unmarshalling(messageSequence: rpc.MessageSequence): boolean {
    this.picture = image.createPictureFromParcel(messageSequence);
    return true;
  }
}

@Entry
@Component
struct PictureMetadataExample {
  @State results: string[] = [];
  @State mainPreview: image.PixelMap | undefined = undefined;
  @State auxPreview: image.PixelMap | undefined = undefined;
  private picture: image.Picture | null = null;
  private auxPicture: image.AuxiliaryPicture | null = null;
  private metadata: image.Metadata | null = null;

  build() {
    Scroll() {
      Column({ space: 8 }) {
        Text('Picture / AuxiliaryPicture / Metadata 接口示例')
          .fontSize(22).fontWeight(FontWeight.Bold).margin({ top: 16, bottom: 4 })
        Row({ space: 12 }) {
          if (this.mainPreview) {
            Image(this.mainPreview).width(150).height(110)
              .objectFit(ImageFit.Contain).backgroundColor('#EEEEEE').borderRadius(6)
          }
          if (this.auxPreview) {
            Image(this.auxPreview).width(150).height(110)
              .objectFit(ImageFit.Contain).backgroundColor('#EEEEEE').borderRadius(6)
          }
        }
        .width('100%').justifyContent(FlexAlign.Center)
        Text('一、Picture 接口').fontSize(16).fontColor(Color.Blue).margin({ top: 6 })
        this.ApiButton('image.createPicture 创建Picture', () => { this.doCreatePicture(); })
        this.ApiButton('imageSource.createPicture 解码创建', () => { this.doSourcePicture(); })
        this.ApiButton('getMainPixelmap 获取主图', () => { this.doGetMain(); })
        this.ApiButton('getHdrComposedPixelmap HDR合成', () => { this.doGetHdr(); })
        this.ApiButton('getGainmapPixelmap 获取增益图', () => { this.doGetGain(); })
        this.ApiButton('setAuxiliaryPicture / getAuxiliaryPicture', () => { this.doAuxSetGet(); })
        this.ApiButton('Picture setMetadata / getMetadata', () => { this.doPictureMetadata(); })
        this.ApiButton('marshalling + createPictureFromParcel', () => { this.doMarshal(); })
        this.ApiButton('release 释放Picture', () => { this.doReleasePicture(); })
        Text('二、AuxiliaryPicture 接口').fontSize(16).fontColor(Color.Blue).margin({ top: 6 })
        this.ApiButton('createAuxiliaryPicture + getType', () => { this.doCreateAux(); })
        this.ApiButton('getAuxiliaryPictureInfo / setAuxiliaryPictureInfo', () => { this.doAuxInfo(); })
        this.ApiButton('readPixelsToBuffer / writePixelsFromBuffer', () => { this.doAuxBuffer(); })
        this.ApiButton('辅助图 setMetadata / getMetadata', () => { this.doAuxMetadata(); })
        this.ApiButton('release 释放辅助图', () => { this.doReleaseAux(); })
        Text('三、Metadata 接口').fontSize(16).fontColor(Color.Blue).margin({ top: 6 })
        this.ApiButton('getProperties / setProperties', () => { this.doMetaGetSet(); })
        this.ApiButton('getAllProperties / clone', () => { this.doMetaAllClone(); })
        Text('运行结果').fontSize(16).fontColor(Color.Gray).margin({ top: 8 })
        Column({ space: 4 }) {
          ForEach(this.results, (line: string) => {
            Text(line).fontSize(13).fontColor(Color.Black).width('100%')
          }, (line: string, index: number) => index.toString())
        }
        .width('100%').padding(10).backgroundColor('#F5F5F5').borderRadius(8)
      }
      .width('100%').padding({ left: 16, right: 16, bottom: 20 })
    }
    .width('100%').height('100%').backgroundColor(Color.White).align(Alignment.Top)
  }

  @Builder
  ApiButton(label: string, action: () => void) {
    Button(label).fontSize(13).width('100%').backgroundColor('#3D6EFD').borderRadius(6)
      .onClick(() => { action(); })
  }

  log(line: string): void { this.results.push(line); }

  fail(tag: string, error: Error): void {
    const e: BusinessError = error as BusinessError;
    this.log(tag + ' 失败: code=' + e.code + ', message=' + e.message);
  }

  async makePixelMap(width: number, height: number): Promise<image.PixelMap> {
    const buffer: ArrayBuffer = new ArrayBuffer(width * height * 4);
    const bytes: Uint8Array = new Uint8Array(buffer);
    for (let i = 0; i < bytes.length; i += 4) {
      bytes[i] = 60; bytes[i + 1] = 120; bytes[i + 2] = 220; bytes[i + 3] = 255;
    }
    let opts: image.InitializationOptions = {
      size: { height: height, width: width }, pixelFormat: image.PixelMapFormat.RGBA_8888, editable: true
    };
    return image.createPixelMap(buffer, opts);
  }

  async doCreatePicture(): Promise<void> {
    try {
      const pm: image.PixelMap = await this.makePixelMap(160, 120);
      this.picture = image.createPicture(pm);
      this.log('image.createPicture: 创建Picture成功');
    } catch (error) { this.fail('image.createPicture', error); }
  }

  async doSourcePicture(): Promise<void> {
    try {
      const pm: image.PixelMap = await this.makePixelMap(120, 90);
      const packer: image.ImagePacker = image.createImagePacker();
      let packOpts: image.PackingOption = { format: 'image/jpeg', quality: 90 };
      const packed: ArrayBuffer = await packer.packToData(pm, packOpts);
      const source: image.ImageSource = image.createImageSource(packed);
      let options: image.DecodingOptionsForPicture = { desiredAuxiliaryPictures: [image.AuxiliaryPictureType.GAINMAP] };
      this.picture = await source.createPicture(options);
      this.log('imageSource.createPicture(DecodingOptionsForPicture): 解码成功');
      source.release(); packer.release();
    } catch (error) { this.fail('imageSource.createPicture', error); }
  }

  async doGetMain(): Promise<void> {
    if (this.picture == null) { this.log('Picture为空，请先创建'); return; }
    try {
      const main: image.PixelMap = this.picture.getMainPixelmap();
      const info: image.ImageInfo = await main.getImageInfo();
      this.mainPreview = main;
      this.log('getMainPixelmap: 主图 ' + info.size.width + 'x' + info.size.height);
    } catch (error) { this.fail('getMainPixelmap', error); }
  }

  async doGetHdr(): Promise<void> {
    if (this.picture == null) { this.log('Picture为空，请先创建'); return; }
    try {
      const hdr: image.PixelMap = await this.picture.getHdrComposedPixelmap();
      this.auxPreview = hdr;
      this.log('getHdrComposedPixelmap: HDR合成成功');
    } catch (error) { this.fail('getHdrComposedPixelmap', error); }
  }

  doGetGain(): void {
    if (this.picture == null) { this.log('Picture为空，请先创建'); return; }
    const gain: image.PixelMap | null = this.picture.getGainmapPixelmap();
    if (gain != null) { this.auxPreview = gain; }
    this.log(gain != null ? 'getGainmapPixelmap: 获取增益图成功' : 'getGainmapPixelmap: 当前Picture无增益图，返回null');
  }

  async doAuxSetGet(): Promise<void> {
    if (this.picture == null) { this.log('Picture为空，请先创建'); return; }
    try {
      if (this.auxPicture == null) { await this.doCreateAux(); }
      if (this.auxPicture != null) {
        this.picture.setAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP, this.auxPicture);
        const got: image.AuxiliaryPicture | null = this.picture.getAuxiliaryPicture(image.AuxiliaryPictureType.GAINMAP);
        if (got != null) { this.auxPicture = got; }
        this.log('setAuxiliaryPicture/getAuxiliaryPicture: 回读类型=' + (got != null ? got.getType() : 'null'));
      }
    } catch (error) { this.fail('setAuxiliaryPicture', error); }
  }

  async doPictureMetadata(): Promise<void> {
    if (this.picture == null) { this.log('Picture为空，请先创建'); return; }
    try {
      const type: image.MetadataType = image.MetadataType.EXIF_METADATA;
      const md: image.Metadata | null = await this.picture.getMetadata(type);
      this.metadata = md;
      if (md != null) { await this.picture.setMetadata(type, md); }
      this.log(md != null ? 'Picture getMetadata/setMetadata: EXIF元数据成功' : 'getMetadata: 返回null');
    } catch (error) { this.fail('Picture getMetadata/setMetadata', error); }
  }

  async doMarshal(): Promise<void> {
    if (this.picture == null) { this.log('Picture为空，请先创建'); return; }
    try {
      const parcelable: PictureSequence = new PictureSequence(this.picture);
      const data: rpc.MessageSequence = rpc.MessageSequence.create();
      data.writeParcelable(parcelable);
      const ret: PictureSequence = new PictureSequence(this.picture);
      data.readParcelable(ret);
      if (ret.picture != null) {
        const info: image.ImageInfo = await ret.picture.getMainPixelmap().getImageInfo();
        this.log('marshalling/createPictureFromParcel: 还原主图 ' + info.size.width + 'x' + info.size.height);
      }
    } catch (error) { this.fail('marshalling', error); }
  }

  doReleasePicture(): void {
    if (this.picture == null) { this.log('Picture为空，请先创建'); return; }
    try { this.picture.release(); this.picture = null; this.log('release: Picture已释放'); } catch (error) { this.fail('Picture release', error); }
  }

  async doCreateAux(): Promise<void> {
    try {
      let size: image.Size = { height: 48, width: 64 };
      const buffer: ArrayBuffer = new ArrayBuffer(size.height * size.width * 4);
      const aux: image.AuxiliaryPicture =
        image.createAuxiliaryPicture(buffer, size, image.AuxiliaryPictureType.GAINMAP);
      this.auxPicture = aux;
      this.log('createAuxiliaryPicture: 成功, getType=' + aux.getType());
    } catch (error) { this.fail('createAuxiliaryPicture', error); }
  }

  async doAuxInfo(): Promise<void> {
    if (this.auxPicture == null) { this.log('辅助图为空，请先创建'); return; }
    try {
      const before: image.AuxiliaryPictureInfo = this.auxPicture.getAuxiliaryPictureInfo();
      this.log('getAuxiliaryPictureInfo: ' + before.size.width + 'x' + before.size.height + ' rowStride=' + before.rowStride);
      let info: image.AuxiliaryPictureInfo = {
        auxiliaryPictureType: image.AuxiliaryPictureType.GAINMAP, size: { height: 48, width: 64 },
        pixelFormat: image.PixelMapFormat.RGBA_8888, rowStride: 256,
        colorSpace: colorSpaceManager.create(colorSpaceManager.ColorSpace.SRGB)
      };
      this.auxPicture.setAuxiliaryPictureInfo(info);
      this.log('setAuxiliaryPictureInfo: 设置辅助图信息成功');
    } catch (error) { this.fail('setAuxiliaryPictureInfo', error); }
  }

  async doAuxBuffer(): Promise<void> {
    if (this.auxPicture == null) { this.log('辅助图为空，请先创建'); return; }
    try {
      const pixels: ArrayBuffer = await this.auxPicture.readPixelsToBuffer();
      this.log('readPixelsToBuffer: 读出 ' + pixels.byteLength + ' 字节');
      await this.auxPicture.writePixelsFromBuffer(pixels);
      this.log('writePixelsFromBuffer: 写回成功');
    } catch (error) { this.fail('辅助图像素读写', error); }
  }

  async doAuxMetadata(): Promise<void> {
    if (this.auxPicture == null) { this.log('辅助图为空，请先创建'); return; }
    try {
      const type: image.MetadataType = image.MetadataType.EXIF_METADATA;
      if (this.metadata == null && this.picture != null) { this.metadata = await this.picture.getMetadata(type); }
      if (this.metadata == null) { this.log('无可用Metadata，请先获取Picture元数据'); return; }
      await this.auxPicture.setMetadata(type, this.metadata);
      const md: image.Metadata | null = await this.auxPicture.getMetadata(type);
      this.log('AuxiliaryPicture setMetadata/getMetadata: ' + (md != null ? '成功' : '返回null'));
    } catch (error) { this.fail('AuxiliaryPicture setMetadata', error); }
  }

  doReleaseAux(): void {
    if (this.auxPicture == null) { this.log('辅助图为空，请先创建'); return; }
    try { this.auxPicture.release(); this.auxPicture = null; this.log('release: AuxiliaryPicture已释放'); } catch (error) { this.fail('AuxiliaryPicture release', error); }
  }

  async doMetaGetSet(): Promise<void> {
    if (this.metadata == null) { this.log('Metadata为空，请先获取'); return; }
    try {
      const got: Record<string, string | null> = await this.metadata.getProperties(['ImageWidth', 'ImageLength']);
      this.log('getProperties: ' + JSON.stringify(got));
      let setkey: Record<string, string | null> = { 'ImageWidth': '200', 'ImageLength': '300' };
      await this.metadata.setProperties(setkey);
      this.log('setProperties: 写入ImageWidth/ImageLength成功');
    } catch (error) { this.fail('getProperties/setProperties', error); }
  }

  async doMetaAllClone(): Promise<void> {
    if (this.metadata == null) { this.log('Metadata为空，请先获取'); return; }
    try {
      const all: Record<string, string | null> = await this.metadata.getAllProperties();
      this.log('getAllProperties: 共 ' + Object.keys(all).length + ' 项属性');
      const cloned: image.Metadata = await this.metadata.clone();
      this.log('clone: 克隆后读取 ' + JSON.stringify(await cloned.getProperties(['ImageWidth'])));
    } catch (error) { this.fail('getAllProperties/clone', error); }
  }
}
