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
﻿import { sendableImage } from '@kit.ImageKit';
import { image } from '@kit.ImageKit';
import { fileIo } from '@kit.CoreFileKit';
import { BusinessError } from '@kit.BasicServicesKit';
import { taskpool } from '@kit.ArkTS';

@Concurrent
async function describePixelMapInWorker(pixelMap: sendableImage.PixelMap): Promise<string> {
  const info: image.ImageInfo = await pixelMap.getImageInfo();
  const bytes: number = pixelMap.getPixelBytesNumber();
  await pixelMap.release();
  return 'worker read ' + info.size.width + 'x' + info.size.height + ', ' + bytes + ' bytes';
}

@Entry
@Component
struct SendableImageTaskPoolExample {
  @State results: string[] = [];

  build() {
    Scroll() {
      Column({ space: 12 }) {
        Text('Sendable图片处理与TaskPool示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 10 })
        Button('createPixelMap 异步创建')
          .onClick(() => this.runCreatePixelMap())
        Button('createPixelMapSync 同步创建')
          .onClick(() => this.runCreatePixelMapSync())
        Button('convertFrom / convertTo 往返转换')
          .onClick(() => this.runConvertRoundTrip())
        Button('createImageSource 三种重载')
          .onClick(() => this.runImageSource())
        Button('createImageReceiver 监听与释放')
          .onClick(() => this.runImageReceiver())
        Button('PixelMap 成员方法组合')
          .onClick(() => this.runPixelMapMembers())
        Button('Sendable PixelMap 传入 TaskPool')
          .onClick(() => this.runTaskpool())
        Button('清空结果')
          .backgroundColor(Color.Gray)
          .onClick(() => { this.results = []; })
        Column({ space: 6 }) {
          ForEach(this.results, (item: string) => {
            Text(item).fontSize(14).fontColor(Color.Black)
          })
        }
        .width('100%')
        .padding(12)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
      }
      .width('100%')
      .padding({ left: 20, right: 20 })
    }
    .width('100%')
    .height('100%')
    .backgroundColor(Color.White)
  }

  private addResult(item: string): void {
    this.results.push(item);
  }

  private makeOptions(): image.InitializationOptions {
    return {
      size: { height: 4, width: 6 },
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.BGRA_8888,
      editable: true
    };
  }

  async runCreatePixelMap(): Promise<void> {
    const color: ArrayBuffer = new ArrayBuffer(96);
    try {
      const pixelMap: sendableImage.PixelMap = await sendableImage.createPixelMap(color, this.makeOptions());
      this.addResult('createPixelMap 成功, 总字节=' + pixelMap.getPixelBytesNumber());
      await pixelMap.release();
      this.addResult('createPixelMap PixelMap 已释放');
    } catch (error) {
      const err = error as BusinessError;
      this.addResult('createPixelMap 失败: ' + err.code);
    }
  }

  async runCreatePixelMapSync(): Promise<void> {
    const color: ArrayBuffer = new ArrayBuffer(96);
    try {
      const pixelMap: sendableImage.PixelMap = sendableImage.createPixelMapSync(color, this.makeOptions());
      this.addResult('createPixelMapSync 成功, 行字节=' + pixelMap.getBytesNumberPerRow() +
        ', 密度=' + pixelMap.getDensity());
      await pixelMap.release();
    } catch (error) {
      const err = error as BusinessError;
      this.addResult('createPixelMapSync 失败: ' + err.code);
    }
  }

  async runConvertRoundTrip(): Promise<void> {
    const color: ArrayBuffer = new ArrayBuffer(96);
    const normal: image.PixelMap = image.createPixelMapSync(color, this.makeOptions());
    try {
      const sendable: sendableImage.PixelMap = sendableImage.convertFromPixelMap(normal);
      const info1: image.ImageInfo = sendable.getImageInfoSync();
      this.addResult('convertFromPixelMap 成功 ' + info1.size.width + 'x' + info1.size.height);
      const back: image.PixelMap = sendableImage.convertToPixelMap(sendable);
      const info2: image.ImageInfo = back.getImageInfoSync();
      this.addResult('convertToPixelMap 成功, 像素格式=' + info2.pixelFormat);
      await back.release();
    } catch (error) {
      const err = error as BusinessError;
      this.addResult('convert 往返失败: ' + err.code);
    }
  }

  async runImageSource(): Promise<void> {
    const ctx: Context = getContext(this);
    const path: string = ctx.cacheDir + '/test.jpg';
    const buf: ArrayBuffer = new ArrayBuffer(96);
    const file: fileIo.File = fileIo.openSync(path, fileIo.OpenMode.CREATE | fileIo.OpenMode.READ_WRITE);
    const srcFd: sendableImage.ImageSource = sendableImage.createImageSource(file.fd);
    fileIo.closeSync(file);
    const srcUri: sendableImage.ImageSource = sendableImage.createImageSource(path);
    const srcBuf: sendableImage.ImageSource = sendableImage.createImageSource(buf);
    this.addResult('createImageSource fd/uri/buf 三个重载已调用');
    try {
      const pixelMap: sendableImage.PixelMap = await srcBuf.createPixelMap();
      this.addResult('ImageSource.createPixelMap 成功');
      await pixelMap.release();
    } catch (error) {
      const err = error as BusinessError;
      this.addResult('ImageSource.createPixelMap 按预期拒绝: ' + err.code);
    }
    await srcBuf.release();
    await srcUri.release();
    await srcFd.release();
    this.addResult('三个 ImageSource 实例已释放');
  }

  async runImageReceiver(): Promise<void> {
    const size: image.Size = { height: 100, width: 100 };
    const receiver: sendableImage.ImageReceiver = sendableImage.createImageReceiver(size, image.ImageFormat.JPEG, 8);
    receiver.on('imageArrival', () => {
      receiver.readLatestImage().then((img: sendableImage.Image) => {
        this.addResult('imageArrival 触发, 已读取最新图片');
        img.release().catch((err: BusinessError) => this.addResult('Image 释放失败: ' + err.code));
      }).catch((err: BusinessError) => this.addResult('readLatestImage 失败: ' + err.code));
    });
    try {
      const surfaceId: string = await receiver.getReceivingSurfaceId();
      this.addResult('getReceivingSurfaceId 成功: ' + surfaceId);
    } catch (error) {
      const err = error as BusinessError;
      this.addResult('getReceivingSurfaceId 失败: ' + err.code);
    }
    await receiver.release();
    this.addResult('ImageReceiver 已释放');
  }

  async runPixelMapMembers(): Promise<void> {
    const color: ArrayBuffer = new ArrayBuffer(96);
    const pixelMap: sendableImage.PixelMap = sendableImage.createPixelMapSync(color, this.makeOptions());
    const area: image.PositionArea = {
      pixels: new ArrayBuffer(8), offset: 0, stride: 8,
      region: { size: { height: 1, width: 2 }, x: 0, y: 0 }
    };
    pixelMap.readPixelsSync(area);
    await pixelMap.readPixels(area);
    pixelMap.scaleSync(2.0, 1.0);
    await pixelMap.crop({ x: 0, y: 0, size: { height: 4, width: 6 } });
    await pixelMap.rotate(90.0);
    await pixelMap.opacity(0.5);
    const info: image.ImageInfo = await pixelMap.getImageInfo();
    this.addResult('成员方法组合完成, 尺寸=' + info.size.width + 'x' + info.size.height);
    await pixelMap.release();
    this.addResult('PixelMap 已释放');
  }

  async runTaskpool(): Promise<void> {
    const color: ArrayBuffer = new ArrayBuffer(96);
    const pixelMap: sendableImage.PixelMap = sendableImage.createPixelMapSync(color, this.makeOptions());
    const sendableObj: sendableImage.ISendable = pixelMap;
    const task: taskpool.Task = new taskpool.Task(describePixelMapInWorker, sendableObj);
    try {
      const ret: Object = await taskpool.execute(task);
      this.addResult('taskpool 结果: ' + ret.toString());
    } catch (error) {
      const err = error as BusinessError;
      this.addResult('taskpool 执行失败: ' + err.code);
    }
  }
}
