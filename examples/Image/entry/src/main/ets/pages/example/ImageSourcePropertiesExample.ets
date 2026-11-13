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
struct ImageSourcePropertiesExample {
  @State statusText: string = '正在初始化';
  @State resultLines: string[] = [];
  @State previewPixelMap: image.PixelMap | undefined = undefined;
  private sourceBuffer: ArrayBuffer = new ArrayBuffer(0);
  private sandboxPath: string = '';

  async aboutToAppear(): Promise<void> {
    const context = this.getUIContext().getHostContext();
    if (context === undefined) {
      this.statusText = 'Context 不可用';
      return;
    }
    const unit8Array: Uint8Array = await context.resourceManager.getMediaContent($r('app.media.icon'));
    this.sourceBuffer = unit8Array.buffer.slice(0, unit8Array.buffer.byteLength);
    const path: string = context.filesDir + '/image_source_demo.svg';
    const file: fileIo.File = fileIo.openSync(path, fileIo.OpenMode.CREATE | fileIo.OpenMode.READ_WRITE);
    fileIo.writeSync(file.fd, this.sourceBuffer);
    fileIo.closeSync(file);
    this.sandboxPath = path;
    const setupSource: image.ImageSource = image.createImageSource(path);
    const setupInfo: image.ImageInfo = setupSource.getImageInfoSync(0);
    this.statusText = `初始化完成 尺寸 ${setupInfo.size.width}x${setupInfo.size.height}`;
    await setupSource.release();
  }

  private appendLine(line: string): void {
    this.resultLines = this.resultLines.concat([line]);
  }

  private errorLine(apiName: string, err: BusinessError): string {
    return `${apiName} 失败 code=${err.code} message=${err.message}`;
  }

  private async runCreateFd(): Promise<void> {
    this.resultLines = [];
    const file: fileIo.File = fileIo.openSync(this.sandboxPath, fileIo.OpenMode.READ_ONLY);
    const imageSource: image.ImageSource = image.createImageSource(file.fd);
    try {
      const imageInfo: image.ImageInfo = await imageSource.getImageInfo(0);
      this.appendLine(`fd 重载创建, getImageInfo(Promise): ${imageInfo.size.width}x${imageInfo.size.height}, mimeType=${imageInfo.mimeType}`);
    } catch (err) {
      const e: BusinessError = err as BusinessError;
      this.appendLine(this.errorLine('fd 创建 getImageInfo', e));
    }
    await imageSource.release();
    fileIo.closeSync(file);
  }

  private runCreateUri(): Promise<void> {
    this.resultLines = [];
    const imageSource: image.ImageSource = image.createImageSource(this.sandboxPath);
    return new Promise<void>((resolve: () => void) => {
      imageSource.getImageInfo((err: BusinessError, imageInfo: image.ImageInfo) => {
        this.appendLine(err ? this.errorLine('uri 创建 getImageInfo 回调', err)
          : `uri 重载创建, getImageInfo(回调): ${imageInfo.size.width}x${imageInfo.size.height}`);
        imageSource.release().then(() => {
          resolve();
        });
      });
    });
  }

  private runCreateUriOptions(): Promise<void> {
    this.resultLines = [];
    const sourceOptions: image.SourceOptions = { sourceDensity: 120 };
    const imageSource: image.ImageSource = image.createImageSource(this.sandboxPath, sourceOptions);
    return new Promise<void>((resolve: () => void) => {
      imageSource.getImageInfo(0, (err: BusinessError, imageInfo: image.ImageInfo) => {
        this.appendLine(err ? this.errorLine('uri+SourceOptions getImageInfo(index,回调)', err)
          : `uri+SourceOptions(sourceDensity=120), 帧0: ${imageInfo.size.width}x${imageInfo.size.height}, density=${imageInfo.density}`);
        imageSource.release().then(() => {
          resolve();
        });
      });
    });
  }

  private async runIncremental(): Promise<void> {
    this.resultLines = [];
    const view: Uint8Array = new Uint8Array(this.sourceBuffer);
    const half: number = Math.floor(this.sourceBuffer.byteLength / 2);
    const chunk1: ArrayBuffer = new ArrayBuffer(half);
    const chunk1View: Uint8Array = new Uint8Array(chunk1);
    chunk1View.set(view.subarray(0, half));
    const chunk2: ArrayBuffer = new ArrayBuffer(this.sourceBuffer.byteLength - half);
    const chunk2View: Uint8Array = new Uint8Array(chunk2);
    chunk2View.set(view.subarray(half));
    const sourceOptions: image.SourceOptions = { sourceDensity: 120 };
    const imageSource: image.ImageSource = image.CreateIncrementalSource(
      new ArrayBuffer(this.sourceBuffer.byteLength), sourceOptions);
    try {
      await imageSource.updateData(chunk1, false, 0, chunk1.byteLength);
      await imageSource.updateData(chunk2, true, 0, chunk2.byteLength);
      const imageInfo: image.ImageInfo = await imageSource.getImageInfo();
      this.appendLine(`CreateIncrementalSource(buf, options) + updateData(Promise): ${imageInfo.size.width}x${imageInfo.size.height}`);
    } catch (err) {
      const e: BusinessError = err as BusinessError;
      this.appendLine(this.errorLine('增量 updateData Promise', e));
    }
    await imageSource.release();
    const callbackSource: image.ImageSource =
      image.CreateIncrementalSource(new ArrayBuffer(this.sourceBuffer.byteLength));
    await new Promise<void>((resolve: () => void) => {
      callbackSource.updateData(this.sourceBuffer, true, 0, this.sourceBuffer.byteLength, (err: BusinessError) => {
        this.appendLine(err ? this.errorLine('增量 updateData 回调', err)
          : 'CreateIncrementalSource(buf) + updateData(回调) 完成');
        callbackSource.release().then(() => {
          resolve();
        });
      });
    });
  }

  private async runGetProperty(): Promise<void> {
    this.resultLines = [];
    const imageSource: image.ImageSource = image.createImageSource(this.sandboxPath);
    const options: image.ImagePropertyOptions = { index: 0, defaultValue: '9999' };
    try {
      const bits: string = await imageSource.getImageProperty(image.PropertyKey.BITS_PER_SAMPLE, options);
      this.appendLine(`getImageProperty(BITS_PER_SAMPLE, {index:0, defaultValue:'9999'}) = ${bits}`);
      const width: string = await imageSource.getImageProperty(image.PropertyKey.IMAGE_WIDTH);
      this.appendLine(`getImageProperty(IMAGE_WIDTH) = ${width}`);
    } catch (err) {
      const e: BusinessError = err as BusinessError;
      this.appendLine(this.errorLine('getImageProperty', e));
    }
    try {
      const bits: string = imageSource.getImagePropertySync(image.PropertyKey.BITS_PER_SAMPLE);
      this.appendLine(`getImagePropertySync(BITS_PER_SAMPLE) = ${bits}`);
    } catch (err) {
      const e: BusinessError = err as BusinessError;
      this.appendLine(this.errorLine('getImagePropertySync', e));
    }
    await imageSource.release();
  }

  private async runGetProperties(): Promise<void> {
    this.resultLines = [];
    const keys: image.PropertyKey[] = [
      image.PropertyKey.IMAGE_WIDTH,
      image.PropertyKey.IMAGE_LENGTH,
      image.PropertyKey.ORIENTATION,
      image.PropertyKey.DATE_TIME,
      image.PropertyKey.EXPOSURE_TIME,
      image.PropertyKey.GIF_LOOP_COUNT
    ];
    const imageSource: image.ImageSource = image.createImageSource(this.sandboxPath);
    try {
      const data: Record<image.PropertyKey, string | null> = await imageSource.getImageProperties(keys);
      this.appendLine(`getImageProperties: ${JSON.stringify(data)}`);
    } catch (err) {
      const e: BusinessError = err as BusinessError;
      this.appendLine(this.errorLine('getImageProperties', e));
    }
    this.appendLine(`GifPropertyKey(20) 枚举值: ${image.GifPropertyKey.GIF_DELAY_TIME}, ${image.GifPropertyKey.GIF_DISPOSAL_TYPE}`);
    await imageSource.release();
  }

  private async runModify(): Promise<void> {
    this.resultLines = [];
    const imageSource: image.ImageSource = image.createImageSource(this.sandboxPath);
    try {
      await imageSource.modifyImageProperty(image.PropertyKey.IMAGE_WIDTH, '120');
      const width: string = await imageSource.getImageProperty(image.PropertyKey.IMAGE_WIDTH);
      this.appendLine(`modifyImageProperty(IMAGE_WIDTH,'120') 后读取 = ${width}`);
    } catch (err) {
      const e: BusinessError = err as BusinessError;
      this.appendLine(this.errorLine('modifyImageProperty', e));
    }
    const records: Record<string, string | null> = {};
    records[image.PropertyKey.IMAGE_WIDTH] = '1024';
    records[image.PropertyKey.IMAGE_LENGTH] = '1024';
    try {
      await imageSource.modifyImageProperties(records as Record<image.PropertyKey, string | null>);
      const checkKeys: image.PropertyKey[] = [image.PropertyKey.IMAGE_WIDTH, image.PropertyKey.IMAGE_LENGTH];
      const data: Record<image.PropertyKey, string | null> = await imageSource.getImageProperties(checkKeys);
      this.appendLine(`modifyImageProperties 校验: ${JSON.stringify(data)}`);
    } catch (err) {
      const e: BusinessError = err as BusinessError;
      this.appendLine(this.errorLine('modifyImageProperties', e));
    }
    await imageSource.release();
  }

  private async runFrames(): Promise<void> {
    this.resultLines = [];
    const imageSource: image.ImageSource = image.createImageSource(this.sandboxPath);
    try {
      const count: number = await imageSource.getFrameCount();
      this.appendLine(`getFrameCount(Promise) = ${count}`);
    } catch (err) {
      const e: BusinessError = err as BusinessError;
      this.appendLine(this.errorLine('getFrameCount Promise', e));
    }
    await new Promise<void>((resolve: () => void) => {
      imageSource.getFrameCount((err: BusinessError, frameCount: number) => {
        this.appendLine(err ? this.errorLine('getFrameCount 回调', err) : `getFrameCount(回调) = ${frameCount}`);
        resolve();
      });
    });
    try {
      const delays: number[] = await imageSource.getDelayTimeList();
      this.appendLine(`getDelayTimeList(Promise): ${delays.join(', ')}`);
    } catch (err) {
      const e: BusinessError = err as BusinessError;
      this.appendLine(this.errorLine('getDelayTimeList Promise', e));
    }
    await new Promise<void>((resolve: () => void) => {
      imageSource.getDelayTimeList((err: BusinessError, delayTimes: number[]) => {
        this.appendLine(err ? this.errorLine('getDelayTimeList 回调', err) : `getDelayTimeList(回调): ${delayTimes.join(', ')}`);
        resolve();
      });
    });
    try {
      const disposals: number[] = await imageSource.getDisposalTypeList();
      this.appendLine(`getDisposalTypeList(Promise): ${disposals.join(', ')}`);
    } catch (err) {
      const e: BusinessError = err as BusinessError;
      this.appendLine(this.errorLine('getDisposalTypeList', e));
    }
    await imageSource.release();
  }

  private async runAllocator(): Promise<void> {
    this.resultLines = [];
    const imageSource: image.ImageSource = image.createImageSource(this.sandboxPath);
    const decodingOptions: image.DecodingOptions = {
      editable: true,
      desiredSize: { width: 64, height: 64 },
      desiredPixelFormat: image.PixelMapFormat.RGBA_8888,
      index: 0
    };
    try {
      const pixelMap: image.PixelMap =
        await imageSource.createPixelMapUsingAllocator(decodingOptions, image.AllocatorType.AUTO);
      this.previewPixelMap = pixelMap;
      this.appendLine('createPixelMapUsingAllocator(AUTO) 成功');
      const syncPixelMap: image.PixelMap =
        imageSource.createPixelMapUsingAllocatorSync(decodingOptions, image.AllocatorType.SHARE_MEMORY);
      this.appendLine(`createPixelMapUsingAllocatorSync(SHARE_MEMORY): ${syncPixelMap !== undefined ? '成功' : '失败'}`);
    } catch (err) {
      const e: BusinessError = err as BusinessError;
      this.appendLine(this.errorLine('Allocator 解码', e));
    }
    await imageSource.release();
  }

  private async runPicture(): Promise<void> {
    this.resultLines = [];
    const imageSource: image.ImageSource = image.createImageSource(this.sandboxPath);
    try {
      const frameCount: number = await imageSource.getFrameCount();
      const picture: image.Picture = await imageSource.createPictureAtIndex(0);
      this.appendLine(`createPictureAtIndex(0) 成功, 帧数=${frameCount}`);
      await picture.release();
    } catch (err) {
      const e: BusinessError = err as BusinessError;
      this.appendLine(this.errorLine('createPictureAtIndex', e));
    }
    await imageSource.release();
  }

  private runFormats(): void {
    this.resultLines = [];
    const formats: string[] = image.getImageSourceSupportedFormats();
    this.appendLine(`getImageSourceSupportedFormats 共 ${formats.length} 项: ${formats.join(', ')}`);
  }

  private runReleaseCallback(): Promise<void> {
    this.resultLines = [];
    const imageSource: image.ImageSource = image.createImageSource(this.sandboxPath);
    return new Promise<void>((resolve: () => void) => {
      imageSource.release((err: BusinessError) => {
        this.appendLine(err ? this.errorLine('release 回调', err) : 'release(回调) 成功');
        resolve();
      });
    });
  }

  build() {
    Scroll() {
      Column({ space: 8 }) {
        Text('ImageSource 属性与帧信息接口示例')
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
        Text(this.statusText)
          .fontSize(14)
          .fontColor(Color.Gray)
        if (this.previewPixelMap !== undefined) {
          Image(this.previewPixelMap)
            .width(96)
            .height(96)
            .border({ width: 1, color: Color.Gray })
        }
        Button('createImageSource(fd) + getImageInfo Promise').width('100%').onClick(() => this.runCreateFd())
        Button('createImageSource(uri) + getImageInfo 回调').width('100%').onClick(() => this.runCreateUri())
        Button('createImageSource(uri, SourceOptions) + getImageInfo(index, 回调)').width('100%').onClick(() => this.runCreateUriOptions())
        Button('CreateIncrementalSource + updateData Promise/回调').width('100%').onClick(() => this.runIncremental())
        Button('getImageProperty Promise + getImagePropertySync').width('100%').onClick(() => this.runGetProperty())
        Button('getImageProperties + GifPropertyKey 枚举值').width('100%').onClick(() => this.runGetProperties())
        Button('modifyImageProperty + modifyImageProperties').width('100%').onClick(() => this.runModify())
        Button('getFrameCount + getDelayTimeList + getDisposalTypeList').width('100%').onClick(() => this.runFrames())
        Button('createPixelMapUsingAllocator + Sync').width('100%').onClick(() => this.runAllocator())
        Button('createPictureAtIndex(20)').width('100%').onClick(() => this.runPicture())
        Button('getImageSourceSupportedFormats(20)').width('100%').onClick(() => this.runFormats())
        Button('release 回调').width('100%').onClick(() => this.runReleaseCallback())
        Text('执行结果')
          .fontSize(16)
          .fontWeight(FontWeight.Medium)
        ForEach(this.resultLines, (line: string) => {
          Text(line)
            .fontSize(12)
            .fontColor('#333333')
            .width('100%')
        }, (line: string, index: number) => `${index}_${line}`)
      }
      .width('100%')
      .padding(16)
      .alignItems(HorizontalAlign.Start)
    }
    .width('100%')
    .height('100%')
    .align(Alignment.TopStart)
  }
}
