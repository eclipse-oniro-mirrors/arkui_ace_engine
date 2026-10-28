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
import { image, videoProcessingEngine } from '@kit.ImageKit';
import { BusinessError } from '@kit.BasicServicesKit';

@Entry
@Component
struct VideoProcessingEngineExample {
  @State logs: string[] = [];
  @State sourcePixelMap: image.PixelMap | undefined = undefined;
  @State outputPixelMap: image.PixelMap | undefined = undefined;
  private processor: videoProcessingEngine.ImageProcessor | undefined = undefined;
  appendLog(line: string): void { this.logs = this.logs.concat([line]); }
  build() {
    Scroll() {
      Column({ space: 8 }) {
        Text('视频处理引擎示例').fontSize(22).fontWeight(FontWeight.Bold)
        Button('1 initializeEnvironment').onClick(() => { this.initEnv(); })
        Button('2 create创建ImageProcessor').onClick(() => { this.createProcessor(); })
        Button('3 构建输入PixelMap').onClick(() => { this.buildInput(); })
        Button('4 enhanceDetail异步(MEDIUM)').onClick(() => { this.enhanceAsync(); })
        Button('5 enhanceDetailSync同步(LOW缩放)').onClick(() => { this.enhanceSync(); })
        Button('6 deinitializeEnvironment').onClick(() => { this.deinitEnv(); })
        Column({ space: 4 }) {
          ForEach(this.logs, (item: string) => {
            Text(item).fontSize(13).fontColor(Color.Gray)
          })
        }
      }
      .width('100%').padding(16)
    }
    .width('100%').height('100%')
  }

  async initEnv(): Promise<void> {
    try {
      await videoProcessingEngine.initializeEnvironment();
      this.appendLog('initializeEnvironment成功');
    } catch (error) {
      this.appendLog('初始化环境失败: ' + (error as BusinessError).message);
    }
  }
  createProcessor(): void {
    try {
      this.processor = videoProcessingEngine.create() as videoProcessingEngine.ImageProcessor;
      this.appendLog('create成功, ImageProcessor已就绪');
    } catch (error) {
      this.appendLog('create失败: ' + (error as BusinessError).message);
    }
  }
  async buildInput(): Promise<void> {
    try {
      const pixels: Uint8Array = new Uint8Array(512 * 512 * 4);
      const options: image.InitializationOptions = {
        size: { width: 512, height: 512 },
        srcPixelFormat: image.PixelMapFormat.RGBA_8888,
        pixelFormat: image.PixelMapFormat.RGBA_8888,
        editable: true
      };
      this.sourcePixelMap = await image.createPixelMap(pixels.buffer, options);
      this.appendLog('输入PixelMap创建成功 512x512');
    } catch (error) {
      this.appendLog('创建PixelMap失败: ' + (error as BusinessError).message);
    }
  }
  async enhanceAsync(): Promise<void> {
    const proc = this.processor;
    const src = this.sourcePixelMap;
    if (proc === undefined || src === undefined) {
      this.appendLog('请先完成步骤1-3');
      return;
    }
    try {
      const out = await proc.enhanceDetail(src, 512, 512, videoProcessingEngine.QualityLevel.MEDIUM);
      this.outputPixelMap = out;
      this.appendLog('enhanceDetail成功 输出宽: ' + out.getImageInfoSync().size.width);
    } catch (error) {
      this.appendLog('enhanceDetail失败: ' + (error as BusinessError).message);
    }
  }
  enhanceSync(): void {
    const proc = this.processor;
    const src = this.sourcePixelMap;
    if (proc === undefined || src === undefined) {
      this.appendLog('请先完成步骤1-3');
      return;
    }
    try {
      const out = proc.enhanceDetailSync(src, 2.0, videoProcessingEngine.QualityLevel.LOW);
      this.outputPixelMap = out;
      this.appendLog('enhanceDetailSync成功 输出高: ' + out.getImageInfoSync().size.height);
    } catch (error) {
      this.appendLog('enhanceDetailSync失败: ' + (error as BusinessError).message);
    }
  }
  async deinitEnv(): Promise<void> {
    try {
      await videoProcessingEngine.deinitializeEnvironment();
      this.processor = undefined;
      this.appendLog('deinitializeEnvironment成功');
    } catch (error) {
      this.appendLog('释放环境失败: ' + (error as BusinessError).message);
    }
  }
}
