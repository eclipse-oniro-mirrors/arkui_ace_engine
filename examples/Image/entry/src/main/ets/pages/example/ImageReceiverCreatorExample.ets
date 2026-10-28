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
import { BusinessError, AsyncCallback } from '@kit.BasicServicesKit';

@Entry
@Component
struct ImageReceiverCreatorExample {
  @State receiverLogs: string[] = [];
  @State creatorLogs: string[] = [];
  @State arrivalReceived: boolean = false;
  @State releaseNotified: boolean = false;
  @State surfaceIdText: string = '未获取';
  private receiver: image.ImageReceiver | null = null;
  private creator: image.ImageCreator | null = null;
  private arrivalCb: AsyncCallback<void> = () => { this.arrivalReceived = true; this.receiverLogs.push('imageArrival 事件触发'); };
  private releaseCb: AsyncCallback<void> = () => { this.releaseNotified = true; this.creatorLogs.push('imageRelease 事件触发'); };

  createReceiver(): void {
    let size: image.Size = { width: 1920, height: 1080 };
    const receiver: image.ImageReceiver = image.createImageReceiver(size, image.ImageFormat.JPEG, 8);
    this.receiver = receiver;
    const receiverSize: image.Size = receiver.size;
    this.receiverLogs.push(`receiver.size 只读属性读取成功 ${receiverSize.width}x${receiverSize.height}`);
    receiver.on('imageArrival', this.arrivalCb);
    this.receiverLogs.push(`createImageReceiver 成功 capacity=${receiver.capacity} format=${receiver.format}`);
  }

  getSurfaceIdByCallback(): void {
    if (this.receiver === null) { this.receiverLogs.push('请先创建 ImageReceiver'); return; }
    const receiver: image.ImageReceiver = this.receiver;
    receiver.getReceivingSurfaceId((err: BusinessError, id: string) => {
      if (err) { this.receiverLogs.push(`callback 获取 surfaceId 失败 code=${err.code}`); return; }
      this.surfaceIdText = id;
      this.receiverLogs.push('callback 获取 surfaceId 成功');
    });
  }

  getSurfaceIdByPromise(): void {
    if (this.receiver === null) { this.receiverLogs.push('请先创建 ImageReceiver'); return; }
    this.receiver.getReceivingSurfaceId().then((id: string) => {
      this.surfaceIdText = id;
      this.receiverLogs.push('Promise 获取 surfaceId 成功');
    }).catch((error: BusinessError) => {
      this.receiverLogs.push(`Promise 获取 surfaceId 失败 code=${error.code}`);
    });
  }

  readNextByCallback(): void {
    if (this.receiver === null) { this.receiverLogs.push('请先创建 ImageReceiver'); return; }
    this.receiver.readNextImage((err: BusinessError, nextImage: image.Image) => {
      if (err || nextImage === undefined) { this.receiverLogs.push('callback readNextImage 失败'); return; }
      nextImage.getComponent(image.ComponentType.JPEG, (compErr: BusinessError, comp: image.Component) => {
        if (compErr || comp === undefined) { this.receiverLogs.push('callback getComponent 失败'); return; }
        this.receiverLogs.push(`callback 读图 ${nextImage.size.width}x${nextImage.size.height} rowStride=${comp.rowStride} buffer=${comp.byteBuffer.byteLength}`);
        nextImage.release((relErr: BusinessError) => {
          this.receiverLogs.push(relErr ? `Image release 失败 code=${relErr.code}` : 'callback Image release 成功');
        });
      });
    });
  }

  readNextByPromise(): void {
    if (this.receiver === null) { this.receiverLogs.push('请先创建 ImageReceiver'); return; }
    this.receiver.readNextImage().then((nextImage: image.Image) => {
      nextImage.getComponent(image.ComponentType.JPEG).then((comp: image.Component) => {
        this.receiverLogs.push(`Promise 读图 pixelStride=${comp.pixelStride} rowStride=${comp.rowStride} buffer=${comp.byteBuffer.byteLength}`);
        nextImage.release().then(() => {
          this.receiverLogs.push('Promise Image release 成功');
        }).catch((relErr: BusinessError) => {
          this.receiverLogs.push(`Promise Image release 失败 code=${relErr.code}`);
        });
      }).catch((compErr: BusinessError) => {
        this.receiverLogs.push(`Promise getComponent 失败 code=${compErr.code}`);
      });
    }).catch((error: BusinessError) => {
      this.receiverLogs.push(`Promise readNextImage 失败 code=${error.code}`);
    });
  }

  readLatestImage(): void {
    if (this.receiver === null) { this.receiverLogs.push('请先创建 ImageReceiver'); return; }
    this.receiver.readLatestImage().then((latestImage: image.Image) => {
      this.receiverLogs.push(`readLatestImage 成功 ${latestImage.size.width}x${latestImage.size.height}`);
      latestImage.release();
    }).catch((error: BusinessError) => {
      this.receiverLogs.push(`readLatestImage 失败 code=${error.code}`);
    });
  }

  offImageArrival(): void {
    if (this.receiver === null) { this.receiverLogs.push('请先创建 ImageReceiver'); return; }
    this.receiver.off('imageArrival', this.arrivalCb);
    this.arrivalReceived = false;
    this.receiverLogs.push('off 注销 imageArrival 事件');
  }

  releaseReceiverByCallback(): void {
    if (this.receiver === null) { this.receiverLogs.push('请先创建 ImageReceiver'); return; }
    const receiver: image.ImageReceiver = this.receiver;
    receiver.release((err: BusinessError) => {
      if (err) { this.receiverLogs.push(`callback release 失败 code=${err.code}`); return; }
      this.receiver = null;
      this.receiverLogs.push('callback ImageReceiver release 成功');
    });
  }

  releaseReceiverByPromise(): void {
    if (this.receiver === null) { this.receiverLogs.push('请先创建 ImageReceiver'); return; }
    this.receiver.release().then(() => {
      this.receiver = null;
      this.receiverLogs.push('Promise ImageReceiver release 成功');
    }).catch((error: BusinessError) => {
      this.receiverLogs.push(`Promise release 失败 code=${error.code}`);
    });
  }

  createCreator(): void {
    let size: image.Size = { width: 640, height: 480 };
    const creator: image.ImageCreator = image.createImageCreator(size, image.ImageFormat.JPEG, 8);
    this.creator = creator;
    creator.on('imageRelease', this.releaseCb);
    this.creatorLogs.push(`createImageCreator 成功 capacity=${creator.capacity} format=${creator.format}`);
  }

  dequeueQueueByCallback(): void {
    if (this.creator === null) { this.creatorLogs.push('请先创建 ImageCreator'); return; }
    const creator: image.ImageCreator = this.creator;
    creator.dequeueImage((err: BusinessError, img: image.Image) => {
      if (err) { this.creatorLogs.push(`callback dequeueImage 失败 code=${err.code}`); return; }
      this.creatorLogs.push(`callback dequeue 成功 ${img.size.width}x${img.size.height}`);
      creator.queueImage(img, (queueErr: BusinessError) => {
        this.creatorLogs.push(queueErr ? `callback queueImage 失败 code=${queueErr.code}` : 'callback queueImage 入队成功');
      });
    });
  }

  dequeueQueueByPromise(): void {
    if (this.creator === null) { this.creatorLogs.push('请先创建 ImageCreator'); return; }
    const creator: image.ImageCreator = this.creator;
    creator.dequeueImage().then((img: image.Image) => {
      this.creatorLogs.push(`Promise dequeue 成功 timestamp=${img.timestamp}`);
      return creator.queueImage(img);
    }).then(() => {
      this.creatorLogs.push('Promise queueImage 入队成功');
    }).catch((error: BusinessError) => {
      this.creatorLogs.push(`Promise 生产者流程失败 code=${error.code}`);
    });
  }

  offImageRelease(): void {
    if (this.creator === null) { this.creatorLogs.push('请先创建 ImageCreator'); return; }
    this.creator.off('imageRelease', this.releaseCb);
    this.releaseNotified = false;
    this.creatorLogs.push('off 注销 imageRelease 事件');
  }

  releaseCreatorByCallback(): void {
    if (this.creator === null) { this.creatorLogs.push('请先创建 ImageCreator'); return; }
    const creator: image.ImageCreator = this.creator;
    creator.release((err: BusinessError) => {
      if (err) { this.creatorLogs.push(`callback release 失败 code=${err.code}`); return; }
      this.creator = null;
      this.creatorLogs.push('callback ImageCreator release 成功');
    });
  }

  releaseCreatorByPromise(): void {
    if (this.creator === null) { this.creatorLogs.push('请先创建 ImageCreator'); return; }
    this.creator.release().then(() => {
      this.creator = null;
      this.creatorLogs.push('Promise ImageCreator release 成功');
    }).catch((error: BusinessError) => {
      this.creatorLogs.push(`Promise release 失败 code=${error.code}`);
    });
  }

  build() {
    Scroll() {
      Column({ space: 12 }) {
        Text('ImageReceiver与ImageCreator示例').fontSize(22).fontWeight(FontWeight.Bold).margin({ top: 16 })
        Row({ space: 16 }) {
          Text(`图片到达:${this.arrivalReceived ? '已触发' : '未触发'}`).fontSize(14).fontColor(this.arrivalReceived ? Color.Green : Color.Gray)
          Text(`释放通知:${this.releaseNotified ? '已触发' : '未触发'}`).fontSize(14).fontColor(this.releaseNotified ? Color.Green : Color.Gray)
        }
        Text(`surfaceId: ${this.surfaceIdText}`).fontSize(12).fontColor(Color.Gray)
        Column({ space: 8 }) {
          Text('接收方 ImageReceiver').fontSize(18).fontWeight(FontWeight.Medium)
          Button('创建Receiver注册事件').onClick(() => { this.createReceiver(); })
          Button('surfaceId回调获取').onClick(() => { this.getSurfaceIdByCallback(); })
          Button('surfaceIdPromise获取').onClick(() => { this.getSurfaceIdByPromise(); })
          Button('readNext回调读图').onClick(() => { this.readNextByCallback(); })
          Button('readNextPromise读图').onClick(() => { this.readNextByPromise(); })
          Button('readLatest读最新').onClick(() => { this.readLatestImage(); })
          Button('注销imageArrival').onClick(() => { this.offImageArrival(); })
          Button('Receiver回调释放').onClick(() => { this.releaseReceiverByCallback(); })
          Button('ReceiverPromise释放').onClick(() => { this.releaseReceiverByPromise(); })
        }
        .width('100%').padding(12).backgroundColor('#F5F5F5').borderRadius(10)
        Column({ space: 8 }) {
          Text('生产方 ImageCreator').fontSize(18).fontWeight(FontWeight.Medium)
          Button('创建Creator注册事件').onClick(() => { this.createCreator(); })
          Button('dequeue+queue回调').onClick(() => { this.dequeueQueueByCallback(); })
          Button('dequeue+queuePromise').onClick(() => { this.dequeueQueueByPromise(); })
          Button('注销imageRelease').onClick(() => { this.offImageRelease(); })
          Button('Creator回调释放').onClick(() => { this.releaseCreatorByCallback(); })
          Button('CreatorPromise释放').onClick(() => { this.releaseCreatorByPromise(); })
        }
        .width('100%').padding(12).backgroundColor('#F0F8FF').borderRadius(10)
        Column({ space: 6 }) {
          Text('接收方日志').fontSize(16).fontWeight(FontWeight.Medium)
          ForEach(this.receiverLogs, (item: string) => {
            Text(item).fontSize(12).fontColor(Color.Gray)
          }, (item: string, index: number) => index.toString())
          Text('生产方日志').fontSize(16).fontWeight(FontWeight.Medium)
          ForEach(this.creatorLogs, (item: string) => {
            Text(item).fontSize(12).fontColor('#B8860B')
          }, (item: string, index: number) => index.toString())
        }
        .width('100%').padding(12).backgroundColor('#FFF8DC').borderRadius(10)
      }
      .width('100%').padding({ left: 16, right: 16, bottom: 16 })
    }
    .width('100%').height('100%').backgroundColor(Color.White).align(Alignment.Top)
  }
}
