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
import { drawing } from '@kit.ArkGraphics2D';

@Entry
@Component
struct ImageSpanAnimatorGapExample {
  @State spanSrc: ResourceStr = $r('app.media.heart');
  @State filterOn: boolean = false;
  @State animatorState: AnimationStatus = AnimationStatus.Initial;
  @State monitorOn: boolean = true;
  @State imageAlt: PixelMap | undefined = undefined;
  @State statusList: string[] = ['等待ImageSpan加载回调'];
  private srcPool: ResourceStr[] = [
    $r('app.media.heart'), $r('app.media.star'),
    $r('app.media.testSvg'), $r('app.media.icon')
  ];
  private srcIndex: number = 0;
  private filterIdentity: ColorFilter =
    new ColorFilter([1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0]);
  private filterTint: ColorFilter =
    new ColorFilter([0.239, 0, 0, 0, 0, 0, 0.616, 0, 0, 0, 0, 0, 0.706, 0, 0, 0, 0, 0, 1, 0]);

  aboutToAppear(): void {
    this.makeAltPixelMap();
  }

  private async makeAltPixelMap(): Promise<void> {
    let ctx = this.getUIContext().getHostContext();
    let uint8Array = await ctx?.resourceManager?.getMediaContent(
      $r('app.media.icon').id);
    if (uint8Array !== undefined) {
      let imageSource: image.ImageSource = image.createImageSource(
        uint8Array.buffer.slice(0, uint8Array.buffer.byteLength));
      this.imageAlt = await imageSource.createPixelMap(
        { desiredPixelFormat: image.PixelMapFormat.RGBA_8888 });
      await imageSource.release();
      this.statusList = ['alt占位图PixelMap已就绪'];
    }
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('ImageSpan: alt / colorFilter / objectFit / onComplete / onError')
          .fontSize(18)
          .fontWeight(FontWeight.Bold)
          .width('92%')
        Text() {
          Span('alt占位: ')
          ImageSpan(this.spanSrc)
            .alt(this.imageAlt)
            .width(48).height(48)
            .onComplete((event: ImageLoadResult) => {
              if (event.loadingStatus === 1) {
                this.statusList = [`onComplete 实际绘制: ${event.contentWidth} x ${event.contentHeight}`];
              }
            })
            .onError((error: ImageError) => {
              this.statusList = [`onError code: ${error.error?.code}, message: ${error.message}`];
            })
          Span(' 矩阵滤镜: ')
          ImageSpan($r('app.media.star'))
            .width(48).height(48)
            .colorFilter(this.filterOn ? this.filterTint : this.filterIdentity)
          Span(' drawing滤镜: ')
          ImageSpan($r('app.media.heart'))
            .width(48).height(48)
            .colorFilter(drawing.ColorFilter.createBlendModeColorFilter(
              { alpha: 255, red: 112, green: 112, blue: 112 },
              drawing.BlendMode.SRC
            ))
          Span(' objectFit填充: ')
          ImageSpan($r('app.media.icon'))
            .width(100).height(60)
            .objectFit(ImageFit.Cover)
        }
        .width('92%')
        .fontSize(16)
        ForEach(this.statusList, (item: string) => {
          Text(item)
            .fontSize(13)
            .fontColor('#666666')
        }, (item: string) => item)
        Row({ space: 10 }) {
          Button('切换图源')
            .fontSize(13)
            .onClick(() => {
              this.srcIndex = (this.srcIndex + 1) % this.srcPool.length;
              this.spanSrc = this.srcPool[this.srcIndex];
              this.statusList = ['已切换ImageSpan图源'];
            })
          Button(this.filterOn ? '关闭颜色滤镜' : '开启颜色滤镜')
            .fontSize(13)
            .onClick(() => {
              this.filterOn = !this.filterOn;
              this.statusList = [`颜色滤镜: ${this.filterOn ? '开' : '关'}`];
            })
        }
        Text('ImageAnimator: monitorInvisibleArea')
          .fontSize(18)
          .fontWeight(FontWeight.Bold)
          .width('92%')
        ImageAnimator()
          .images([
            { src: $r('app.media.heart') },
            { src: $r('app.media.star') },
            { src: $r('app.media.testSvg') },
            { src: $r('app.media.icon') }
          ])
          .state(this.animatorState)
          .monitorInvisibleArea(this.monitorOn)
          .width(160)
          .height(80)
        Row({ space: 10 }) {
          Button(this.animatorState === AnimationStatus.Running ? '暂停' : '播放')
            .fontSize(13)
            .onClick(() => {
              this.animatorState = this.animatorState === AnimationStatus.Running
                ? AnimationStatus.Paused : AnimationStatus.Running;
              this.statusList = [
                `播放状态: ${this.animatorState === AnimationStatus.Running ? 'Running' : 'Paused'}`
              ];
            })
          Button(this.monitorOn ? '关闭不可见监控' : '开启不可见监控')
            .fontSize(13)
            .onClick(() => {
              this.monitorOn = !this.monitorOn;
              this.statusList = [`不可见监控: ${this.monitorOn ? '开' : '关'}`];
            })
        }
      }
      .width('100%')
      .padding(16)
    }
    .width('100%')
    .height('100%')
  }
}
