/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
// 测试代码路径
import OffScreenCanvasRenderingContext2DStrokeStyleTest
  from '../MyTest/OffScreenCanvasRenderingContext2DStrokeStyleTest'
import { loadComponentTestEntry } from '@ohos.arkui.componentTest'

@Entry
@Component
struct OffScreenCanvasRenderingContext2DStrokeStyleSample {
  private settings: RenderingContextSettings = new RenderingContextSettings(true)
  private context: CanvasRenderingContext2D = new CanvasRenderingContext2D(this.settings)
  private offCanvas: OffscreenCanvas = new OffscreenCanvas(600, 600)

  aboutToAppear(): void {
    loadComponentTestEntry(OffScreenCanvasRenderingContext2DStrokeStyleTest)
  }

  build() {
    Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
      Canvas(this.context)
        .width('100%')
        .height('100%')
        .backgroundColor('#ffff00')
        .id('offScreenCanvasRenderingContext2DStrokeStyle')
        .onReady(() => {
          this.context.strokeStyle = 0x0000ff
          this.context.strokeRect(25, 25, 155, 105)
        })
      Button('OffScreenCanvasRenderingContext2DStrokeStyle')
        .onClick(() => {
          this.context.strokeStyle = '#c12c1f'
          this.context.strokeRect(25, 150, 155, 105)
        })
    }
    .width('100%')
    .height('100%')
  }
}