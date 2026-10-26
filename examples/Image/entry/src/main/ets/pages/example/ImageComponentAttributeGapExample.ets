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
import { matrix4 } from '@kit.ArkUI';
import { BusinessError } from '@kit.BasicServicesKit';

@Entry
@Component
struct ImageComponentAttributeGapExample {
  @State analyzerPixelMap: image.PixelMap | undefined = undefined;
  @State matrixOn: boolean = false;
  @State sourceLarge: boolean = true;
  @State privacyOn: boolean = false;
  @State svg2On: boolean = true;
  @State finishStatus: string = 'SVG动效未结束';
  @State analyzerStatus: string = '等待创建PixelMap';
  private aiController: ImageAnalyzerController = new ImageAnalyzerController();
  private options: ImageAIOptions = {
    types: [ImageAnalyzerType.SUBJECT, ImageAnalyzerType.TEXT],
    aiController: this.aiController
  };
  private matrix1 = matrix4.identity()
    .translate({ x: -40, y: -75 })
    .scale({ x: 0.5, y: 0.5 })
    .rotate({ x: 2, y: 0.5, z: 3, centerX: 10, centerY: 10, angle: -10 });
  private identityMatrix = matrix4.identity();

  async aboutToAppear(): Promise<void> {
    const buffer: ArrayBuffer = new ArrayBuffer(100 * 100 * 4);
    const opts: image.InitializationOptions = {
      size: { height: 100, width: 100 },
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      scaleMode: image.ScaleMode.FIT_TARGET_SIZE,
      editable: true
    };
    try {
      this.analyzerPixelMap = await image.createPixelMap(buffer, opts);
      this.analyzerStatus = 'PixelMap创建成功';
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      this.analyzerStatus = 'PixelMap创建失败: ' + err.code;
    }
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('Image属性与分析器接口示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })
        Column({ space: 8 }) {
          Text('imageMatrix 变换矩阵(API15)').fontSize(16).fontColor(Color.Gray)
          Image(this.analyzerPixelMap).objectFit(ImageFit.MATRIX)
            .imageMatrix(this.matrixOn ? this.matrix1 : this.identityMatrix)
            .width(120).height(120).borderWidth(1)
          Button(this.matrixOn ? '重置矩阵' : '应用变换矩阵').onClick(() => {
            this.matrixOn = !this.matrixOn;
          })
          Text('当前: ' + (this.matrixOn ? '平移+缩放+旋转' : '单位矩阵')).fontSize(14)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        Column({ space: 8 }) {
          Text('sourceSize 解码尺寸').fontSize(16).fontColor(Color.Gray)
          Image($r('app.media.icon'))
            .sourceSize(this.sourceLarge ? { width: 1393, height: 1080 } : { width: 13, height: 10 })
            .width(120).height(120).objectFit(ImageFit.Contain).borderWidth(1)
          Button(this.sourceLarge ? '切换小解码尺寸' : '切换大解码尺寸').onClick(() => {
            this.sourceLarge = !this.sourceLarge;
          })
          Text('解码尺寸: ' + (this.sourceLarge ? '1393 x 1080' : '13 x 10')).fontSize(14)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        Column({ space: 8 }) {
          Text('privacySensitive 隐私敏感(API12)').fontSize(16).fontColor(Color.Gray)
          Image($r('app.media.heart')).width(80).height(80).privacySensitive(this.privacyOn)
          Button(this.privacyOn ? '关闭隐私敏感' : '开启隐私敏感').onClick(() => {
            this.privacyOn = !this.privacyOn;
          })
          Text('当前: privacySensitive = ' + (this.privacyOn ? 'true' : 'false')).fontSize(14)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        Column({ space: 8 }) {
          Text('supportSvg2 解析增强(API21)').fontSize(16).fontColor(Color.Gray)
          if (this.svg2On) {
            Image($r('app.media.svg2_mixed')).width(100).height(100).supportSvg2(true)
          } else {
            Image($r('app.media.svg2_mixed')).width(100).height(100).supportSvg2(false)
          }
          Button(this.svg2On ? '切换为false' : '切换为true').onClick(() => {
            this.svg2On = !this.svg2On;
          })
          Text('当前: supportSvg2 = ' + (this.svg2On ? 'true' : 'false')).fontSize(14)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        Column({ space: 8 }) {
          Text('onFinish SVG动效结束事件').fontSize(16).fontColor(Color.Gray)
          Image($r('app.media.testSvg')).width(100).height(100)
            .onFinish(() => {
              this.finishStatus = 'SVG动效播放完成';
            })
          Button('重置状态').onClick(() => {
            this.finishStatus = 'SVG动效未结束';
          })
          Text('状态: ' + this.finishStatus).fontSize(14)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        Column({ space: 8 }) {
          Text('ImageAIOptions 图像AI分析(API12)').fontSize(16).fontColor(Color.Gray)
          Image(this.analyzerPixelMap, this.options)
            .enableAnalyzer(true)
            .width(120).height(120).borderWidth(1)
          Button('获取支持的分析类型').onClick(() => {
            const types: ImageAnalyzerType[] = this.aiController.getImageAnalyzerSupportTypes();
            this.analyzerStatus = '支持类型数量: ' + types.length;
          })
          Text('状态: ' + this.analyzerStatus).fontSize(14)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        Text('图源为PixelMap/位图时imageMatrix与sourceSize才生效').fontSize(12).fontColor(Color.Blue).margin({ bottom: 20 })
      }.width('100%').padding({ left: 20, right: 20 })
    }.width('100%').height('100%').backgroundColor(Color.White)
  }
}
