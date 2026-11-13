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
import { fileIo } from '@kit.CoreFileKit';

@Entry
@Component
struct ImagePackerAdvancedExample {
  @State preview: image.PixelMap | undefined = undefined;
  @State lines: Array<string> = [];
  private seqOption: image.PackingOptionsForSequence = { frameCount: 3, delayTimeList: [10, 10, 10],
    disposalTypes: [3, 2, 3], loopCount: 0 };

  build() {
    Scroll() {
      Column({ space: 12 }) {
        Text('ImagePacker 编码进阶示例').fontSize(24).fontWeight(FontWeight.Bold).margin({ top: 20, bottom: 10 })
        Button('packToData: PixelMap 编码为 PNG 数据').onClick(() => { this.packPixelMapToData(); })
        Button('packToData: ImageSource 编码为 JPEG 数据').onClick(() => { this.packImageSourceToData(); })
        Button('packToFile: ImageSource 编码入 fd (callback)').onClick(() => { this.packImageSourceToFileCb(); })
        Button('packToFile: ImageSource 编码入 fd (Promise)').onClick(() => { this.packImageSourceToFilePromise(); })
        Button('packToFile: PixelMap 编码入 fd (callback)').onClick(() => { this.packPixelMapToFileCb(); })
        Button('packToFile: PixelMap 编码入 fd (Promise)').onClick(() => { this.packPixelMapToFilePromise(); })
        Button('packToFile: Picture 编码入 fd (Promise)').onClick(() => { this.packPictureToFile(); })
        Button('packToDataFromPixelmapSequence: 编码 GIF 数据').onClick(() => { this.packSequenceToData(); })
        Button('packToFileFromPixelmapSequence: 编码 GIF 文件').onClick(() => { this.packSequenceToFile(); })
        Button('getImagePackerSupportedFormats: 查询支持格式').onClick(() => { this.queryFormats(); })
        Button('release: 释放编码器实例 (callback)').onClick(() => { this.releasePacker(); })
        Button('清空结果').onClick(() => { this.lines = []; this.preview = undefined; })
        Divider().strokeWidth(2).color('#E0E0E0')
        if (this.preview) {
          Image(this.preview).width(220).height(160).objectFit(ImageFit.Contain).border({ width: 2, color: Color.Blue })
        }
        ForEach(this.lines, (line: string) => {
          Text(line).fontSize(13).fontColor(Color.Gray).width('100%')
        })
      }.width('100%').padding({ left: 20, right: 20 })
    }.width('100%').height('100%').backgroundColor(Color.White)
  }

  addLine(text: string): void {
    this.lines = this.lines.concat([text]);
  }

  openFile(name: string): fileIo.File {
    return fileIo.openSync(getContext(this).filesDir + name, fileIo.OpenMode.CREATE | fileIo.OpenMode.READ_WRITE);
  }

  showFilePreview(name: string): void {
    const source: image.ImageSource = image.createImageSource(getContext(this).filesDir + name);
    source.createPixelMap().then((decoded: image.PixelMap) => {
      this.preview = decoded;
      source.release();
    });
  }

  async makeGradient(width: number, height: number): Promise<image.PixelMap> {
    const buffer: ArrayBuffer = new ArrayBuffer(width * height * 4);
    const bytes: Uint8Array = new Uint8Array(buffer);
    for (let i = 0; i < width * height; i++) {
      bytes[i * 4] = Math.floor(255 * (i % width) / (width - 1));
      bytes[i * 4 + 1] = Math.floor(255 * Math.floor(i / width) / (height - 1));
      bytes[i * 4 + 2] = 128;
      bytes[i * 4 + 3] = 255;
    }
    const opts: image.InitializationOptions = { editable: true, pixelFormat: image.PixelMapFormat.RGBA_8888,
      size: { height: height, width: width } };
    return image.createPixelMap(buffer, opts);
  }

  async makeSource(): Promise<image.ImageSource> {
    const pm: image.PixelMap = await this.makeGradient(120, 80);
    const packer: image.ImagePacker = image.createImagePacker();
    const option: image.PackingOption = { format: 'image/png', quality: 90 };
    const data: ArrayBuffer = await packer.packToData(pm, option);
    await packer.release();
    await pm.release();
    return image.createImageSource(data);
  }

  async showUnpacked(data: ArrayBuffer, tag: string): Promise<void> {
    const source: image.ImageSource = image.createImageSource(data);
    this.preview = await source.createPixelMap();
    await source.release();
    this.addLine(tag + ' 解码预览已更新');
  }

  async packPixelMapToData(): Promise<void> {
    const pm: image.PixelMap = await this.makeGradient(120, 80);
    const packer: image.ImagePacker = image.createImagePacker();
    const option: image.PackingOption = { format: 'image/png', quality: 90 };
    try {
      const data: ArrayBuffer = await packer.packToData(pm, option);
      this.addLine('packToData(PixelMap) 成功, ' + data.byteLength + ' 字节');
      await this.showUnpacked(data, 'PNG');
    } catch (error) { this.addLine('packToData(PixelMap) 失败: ' + (error as BusinessError).code); }
    await packer.release();
    await pm.release();
  }

  async packImageSourceToData(): Promise<void> {
    const source: image.ImageSource = await this.makeSource();
    const packer: image.ImagePacker = image.createImagePacker();
    const option: image.PackingOption = { format: 'image/jpeg', quality: 85, bufferSize: 5242880,
      desiredDynamicRange: image.PackingDynamicRange.AUTO, needsPackProperties: true };
    try {
      const data: ArrayBuffer = await packer.packToData(source, option);
      this.addLine('packToData(ImageSource) 成功, ' + data.byteLength + ' 字节');
      await this.showUnpacked(data, 'JPEG');
    } catch (error) { this.addLine('packToData(ImageSource) 失败: ' + (error as BusinessError).code); }
    await packer.release();
    await source.release();
  }

  async packImageSourceToFileCb(): Promise<void> {
    const source: image.ImageSource = await this.makeSource();
    const packer: image.ImagePacker = image.createImagePacker();
    const option: image.PackingOption = { format: 'image/jpeg', quality: 80 };
    const file: fileIo.File = this.openFile('/is_cb.jpg');
    packer.packToFile(source, file.fd, option, (err: BusinessError) => {
      fileIo.closeSync(file);
      this.addLine(err ? 'packToFile(ImageSource,cb) 失败: ' + err.code : 'packToFile(ImageSource,cb) 成功');
      packer.release().then(() => { source.release(); });
    });
  }

  async packImageSourceToFilePromise(): Promise<void> {
    const source: image.ImageSource = await this.makeSource();
    const packer: image.ImagePacker = image.createImagePacker();
    const option: image.PackingOption = { format: 'image/jpeg', quality: 80 };
    const file: fileIo.File = this.openFile('/is_promise.jpg');
    try {
      await packer.packToFile(source, file.fd, option);
      this.addLine('packToFile(ImageSource,Promise) 成功');
    } catch (error) { this.addLine('packToFile(ImageSource,Promise) 失败: ' + (error as BusinessError).code); }
    fileIo.closeSync(file);
    await packer.release();
    await source.release();
  }

  async packPixelMapToFileCb(): Promise<void> {
    const pm: image.PixelMap = await this.makeGradient(120, 80);
    const packer: image.ImagePacker = image.createImagePacker();
    const option: image.PackingOption = { format: 'image/png', quality: 95 };
    const file: fileIo.File = this.openFile('/pm_cb.png');
    packer.packToFile(pm, file.fd, option, (err: BusinessError) => {
      fileIo.closeSync(file);
      this.addLine(err ? 'packToFile(PixelMap,cb) 失败: ' + err.code : 'packToFile(PixelMap,cb) 成功');
      if (!err) { this.showFilePreview('/pm_cb.png'); }
      packer.release().then(() => { pm.release(); });
    });
  }

  async packPixelMapToFilePromise(): Promise<void> {
    const pm: image.PixelMap = await this.makeGradient(120, 80);
    const packer: image.ImagePacker = image.createImagePacker();
    const option: image.PackingOption = { format: 'image/jpeg', quality: 88, bufferSize: 5242880,
      desiredDynamicRange: image.PackingDynamicRange.AUTO, needsPackProperties: true };
    const file: fileIo.File = this.openFile('/pm_promise.jpg');
    try {
      await packer.packToFile(pm, file.fd, option);
      this.addLine('packToFile(PixelMap,Promise) 成功');
    } catch (error) { this.addLine('packToFile(PixelMap,Promise) 失败: ' + (error as BusinessError).code); }
    fileIo.closeSync(file);
    await packer.release();
    await pm.release();
  }

  async packPictureToFile(): Promise<void> {
    const pm: image.PixelMap = await this.makeGradient(120, 80);
    const picture: image.Picture = image.createPicture(pm);
    const packer: image.ImagePacker = image.createImagePacker();
    const option: image.PackingOption = { format: 'image/jpeg', quality: 90,
      desiredDynamicRange: image.PackingDynamicRange.SDR, needsPackProperties: true };
    const file: fileIo.File = this.openFile('/picture.jpg');
    try {
      await packer.packToFile(picture, file.fd, option);
      this.addLine('packToFile(Picture,Promise) 成功');
    } catch (error) { this.addLine('packToFile(Picture,Promise) 失败: ' + (error as BusinessError).code); }
    fileIo.closeSync(file);
    await packer.release();
    picture.release();
    await pm.release();
  }

  async makeFrames(): Promise<image.PixelMap[]> {
    const frames: image.PixelMap[] = [];
    for (let i = 0; i < 3; i++) {
      frames.push(await this.makeGradient(40 + i * 20, 30 + i * 10));
    }
    return frames;
  }

  async packSequenceToData(): Promise<void> {
    const frames: image.PixelMap[] = await this.makeFrames();
    const packer: image.ImagePacker = image.createImagePacker();
    try {
      const data: ArrayBuffer = await packer.packToDataFromPixelmapSequence(frames, this.seqOption);
      this.addLine('packToDataFromPixelmapSequence 成功, ' + data.byteLength + ' 字节');
      await this.showUnpacked(data, 'GIF');
    } catch (error) { this.addLine('packToDataFromPixelmapSequence 失败: ' + (error as BusinessError).code); }
    await packer.release();
    frames.forEach((frame: image.PixelMap): void => { frame.release(); });
  }

  async packSequenceToFile(): Promise<void> {
    const frames: image.PixelMap[] = await this.makeFrames();
    const packer: image.ImagePacker = image.createImagePacker();
    const file: fileIo.File = this.openFile('/seq.gif');
    try {
      await packer.packToFileFromPixelmapSequence(frames, file.fd, this.seqOption);
      this.addLine('packToFileFromPixelmapSequence 成功');
    } catch (error) { this.addLine('packToFileFromPixelmapSequence 失败: ' + (error as BusinessError).code); }
    fileIo.closeSync(file);
    await packer.release();
    frames.forEach((frame: image.PixelMap): void => { frame.release(); });
    this.showFilePreview('/seq.gif');
  }

  queryFormats(): void {
    const formats: string[] = image.getImagePackerSupportedFormats();
    this.addLine('支持的编码格式: ' + formats.join(', '));
  }

  releasePacker(): void {
    const packer: image.ImagePacker = image.createImagePacker();
    packer.release((err: BusinessError) => {
      this.addLine(err ? 'release(callback) 失败: ' + err.code : 'release(callback) 成功');
    });
  }
}
