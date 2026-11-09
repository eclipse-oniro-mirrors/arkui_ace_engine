/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
import imageGeneration from '@ohos.arkui.intelligence.imageGeneration';
import { image } from '@kit.ImageKit';
import Want from '@ohos.app.ability.Want';
let TAG: string = '[ImageGeneratorExample]';
export class MockImageGenerationModel implements imageGeneration.ImageGenerationModel {
  private styles: imageGeneration.ImageStyle[] = [
    { icon: $r('app.media.icon'), name: '水彩' },
    { icon: $r('app.media.star'), name: '涂鸦' },
    { icon: $r('app.media.heart'), name: '油画' },
    { icon: $r('app.media.svg'), name: '国风' },
    { icon: $r('app.media.testSvg'), name: '赛博朋克' }
  ];
  getModelSupportStyles(): imageGeneration.ImageStyle[] {
    console.info(TAG, 'getModelSupportStyles called');
    return this.styles;
  }
  requestImageGeneration(sessionId: number, params: imageGeneration.GenerateImageTaskParams,
    callback: Callback<imageGeneration.GenerateImageTaskPartialResult>): void {
    console.info(TAG, `requestImageGeneration sessionId=${sessionId}, prompt=${params.prompt}`);
    const count: number = params.imageCount ?? 1;
    let index: number = 0;
    const pushPartial = (): void => {
      if (index >= count) {
        const completed: imageGeneration.GenerateImageTaskPartialResult = {
          type: imageGeneration.PartialResultType.COMPLETED,
          totalCount: count
        };
        callback(completed);
        return;
      }
      const partial: imageGeneration.GenerateImageTaskPartialResult = {
        type: imageGeneration.PartialResultType.PARTIAL,
        imageIndex: index,
        imageData: `mock://generated-image-${sessionId}-${index}`
      };
      callback(partial);
      index++;
      setTimeout(pushPartial, 600);
    };
    setTimeout(pushPartial, 300);
  }
  cancelImageGeneration(sessionId: number): void {
    console.info(TAG, `cancelImageGeneration sessionId=${sessionId}`);
  }
  onComplain(sessionId: number, request: imageGeneration.GenerateImageTaskParams,
    result: imageGeneration.GenerateImageTaskResult): void {
    console.info(TAG, `onComplain sessionId=${sessionId}, resultLen=${result.imageData.length}`);
  }
}
export class MockTextGenerationModel implements imageGeneration.TextGenerationModel {
  requestTextGeneration(sessionId: number, value: string,
    callback: Callback<imageGeneration.GenerateTextTaskPartialResult>): void {
    console.info(TAG, `requestTextGeneration sessionId=${sessionId}, value=${value}`);
    let thinkIndex: number = 0;
    const thinkChunks: string[] = ['正在理解画面内容...', '提取主体特征...', '匹配风格词库...', '组织描述语言...'];
    const pushThink = (): void => {
      if (thinkIndex >= thinkChunks.length) {
        const completed: imageGeneration.GenerateTextTaskPartialResult = {
          type: imageGeneration.PartialResultType.COMPLETED
        };
        callback(completed);
        return;
      }
      const partial: imageGeneration.GenerateTextTaskPartialResult = {
        type: imageGeneration.PartialResultType.PARTIAL,
        reasoningContent: thinkChunks[thinkIndex]
      };
      callback(partial);
      thinkIndex++;
      setTimeout(pushThink, 500);
    };
    setTimeout(pushThink, 200);
  }
  cancelTextGeneration(sessionId: number): void {
    console.info(TAG, `cancelTextGeneration sessionId=${sessionId}`);
  }
  onComplain(sessionId: number, request: string,
    result: imageGeneration.GenerateTextTaskResult): void {
    console.info(TAG, `onComplain sessionId=${sessionId}, content=${result.content}`);
  }
}
export class MockLiveViewInfo implements imageGeneration.LiveViewInfo {
  private want: Want = {
    bundleName: 'com.example.imagegenerator',
    abilityName: 'EntryAbility',
    parameters: { sessionId: 'mock-session-001' }
  };
  getWant(): Want {
    console.info(TAG, 'LiveViewInfo.getWant called');
    return this.want;
  }
  getLongTermTaskId(): number {
    console.info(TAG, 'LiveViewInfo.getLongTermTaskId called');
    return 1001;
  }
  isLiveViewNeeded(): boolean {
    console.info(TAG, 'LiveViewInfo.isLiveViewNeeded called');
    return true;
  }
}
export interface StylePreset {
  name: string;
  desc: string;
}
export interface SizePreset {
  label: string;
  size: image.Size;
}
export interface StreamLogEntry {
  time: string;
  type: string;
  detail: string;
}
export interface ScenarioPreset {
  name: string;
  desc: string;
  prompt: string;
  styleIdx: number;
  sizeIdx: number;
  count: number;
}
