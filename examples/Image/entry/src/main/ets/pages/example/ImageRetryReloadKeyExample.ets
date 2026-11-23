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

/**
 * Image 错误计数 + reloadKey 强制重载示例
 *
 * 与 ImageReloadKeyScenarioExample.ets 的差异（刻意不重复）：
 *   - 该页走 Image(src, reloadKey?: string) 构造形参 + @State 状态驱动；
 *     ImageReloadKeyScenarioExample 走 AttributeUpdater.updateConstructorParams 属性直通。
 *   - 该页聚焦"失败计数 -> 重试 -> 成功恢复"闭环与 onComplete/onError 触发顺序观测；
 *     ImageReloadKeyScenarioExample 聚焦相同路径/不同路径 × reloadKey 变化的缓存绕过矩阵。
 *
 * reloadKey 语义（摘自官方文档 Image 接口参数表）：
 *   图片重新加载的标识。当reloadKey值发生变化时，即使使用相同的图片，也会重新加载图片。
 *   适用图片源地址不变但图片内容已更新的场景（例如本地图片内容被覆写）。默认值为空字符串。
 *
 * 事件顺序观测点：
 *   1. 点击重试后旧一次加载的迟到事件与新一轮事件可能交错，序列卡按到达顺序记录；
 *   2. onComplete 触发两次（loadingStatus=0 数据加载成功、loadingStatus=1 解码成功）；
 *   3. 非法图源仅触发 onError，不触发 onComplete。
 */
import { image } from '@kit.ImageKit';
import { BusinessError } from '@kit.BasicServicesKit';

/**
 * 重试图源队列元素
 */
interface RetrySourceDef {
  label: string;
  kind: number; // 0=非法字符串 1=合法SVG资源 2=非法Base64 3=合法位图PixelMap 4=合法SVG(testSvg)
  expect: string; // 预期触发的事件
}

/**
 * 事件序列条目（onComplete/onError/User 统一入列）
 */
interface RetryEventEntry {
  seq: number;
  name: string;
  srcName: string;
  detail: string;
  timeText: string;
}

/**
 * 每个队列图源的失败/成功计数
 */
interface SourceStat {
  fail: number;
  complete: number;
  decode: number;
}

/**
 * onComplete 回调参数结构（与文档字段一一对应）
 */
interface CompleteEventFields {
  width: number;
  height: number;
  componentWidth: number;
  componentHeight: number;
  loadingStatus: number;
  contentWidth: number;
  contentHeight: number;
  contentOffsetX: number;
  contentOffsetY: number;
}

/**
 * 生成与图源队列等长的零值统计数组。
 */
function makeInitStats(): SourceStat[] {
  const stats: SourceStat[] = [];
  for (let i: number = 0; i < RETRY_SOURCES.length; i++) {
    stats.push({ fail: 0, complete: 0, decode: 0 });
  }
  return stats;
}

// 重试图源循环队列：坏 -> 好 -> 坏 -> 好(位图) -> 好(SVG动效)
const RETRY_SOURCES: RetrySourceDef[] = [
  { label: 'retry://broken-uri-001.png（非法scheme）', kind: 0, expect: '预期 onError' },
  { label: 'app.media.icon（合法SVG）', kind: 1, expect: '预期 onComplete x2' },
  { label: 'data:image/png;base64,QUJDRA==（内容非图片）', kind: 2, expect: '预期 onError' },
  { label: '运行时合成 PixelMap（合法位图）', kind: 3, expect: '预期 onComplete x2' },
  { label: 'app.media.testSvg（动效SVG）', kind: 4, expect: '预期 onComplete x2 + onFinish' }
];

@Entry
@Component
struct ImageRetryReloadKeyExample {
  // 当前图源在队列中的下标
  @State srcIndex: number = 0;
  // reloadKey：每次重试自增，驱动相同/不同图源均强制重新加载
  @State reloadKey: string = 'round_0';
  // 重试轮次
  @State round: number = 0;
  // onError 累计失败次数
  @State failCount: number = 0;
  // onComplete 累计成功次数（含 loadingStatus 0/1 两次计数的原始值）
  @State successCount: number = 0;
  // 解码成功次数（loadingStatus=1 口径的"成功恢复展示"）
  @State decodeCount: number = 0;
  // 事件序列（新到达事件插入头部）
  @State eventSeq: RetryEventEntry[] = [];
  // 最近一次结果是否为成功
  @State recovered: boolean = false;
  // 运行时合成的合法位图图源（@kit.ImageKit createImageSource/createPixelMap）
  @State synthPixelMap: image.PixelMap | undefined = undefined;
  // 合成位图创建状态文案
  @State synthStatus: string = '合成PixelMap创建中…';
  // 每个图源各自的失败/成功计数（与队列下标对齐）
  @State stats: SourceStat[] = makeInitStats();
  // 自动重试开关
  @State autoOn: boolean = false;
  // 事件序号自增器（非渲染态，仅用于编号）
  private eventCounter: number = 0;
  // 自动重试定时器ID（-1表示未启动）
  private autoTimer: number = -1;

  async aboutToAppear(): Promise<void> {
    await this.createSynthPixelMap();
    this.pushEvent('User', '页面初始化', '初始图源: ' + this.currentLabel());
  }

  aboutToDisappear(): void {
    this.stopAutoRetry();
  }

  /**
   * 更新队列下标对应图源的计数（整元素替换以触发@State刷新）。
   */
  private bumpStat(index: number, field: string): void {
    const old: SourceStat = this.stats[index];
    const next: SourceStat = { fail: old.fail, complete: old.complete, decode: old.decode };
    if (field === 'fail') {
      next.fail += 1;
    } else if (field === 'complete') {
      next.complete += 1;
    } else {
      next.decode += 1;
    }
    this.stats[index] = next;
  }

  private startAutoRetry(): void {
    this.autoOn = true;
    this.pushEvent('User', '自动重试开启(每2s切换图源)', this.currentLabel());
    this.autoTimer = setInterval(() => {
      this.doRetry();
    }, 2000);
  }

  private stopAutoRetry(): void {
    if (this.autoTimer !== -1) {
      clearInterval(this.autoTimer);
      this.autoTimer = -1;
    }
    if (this.autoOn) {
      this.autoOn = false;
      this.pushEvent('User', '自动重试停止', this.currentLabel());
    }
  }

  /**
   * 用 ImageKit 合成一张 64x64 纯色位图作为"合法重试图源"。
   * 数据由 ArrayBuffer 填充 RGBA 合成，不依赖白名单外的任何 app.media 位图。
   */
  private async createSynthPixelMap(): Promise<void> {
    const size: number = 64;
    const buffer: ArrayBuffer = new ArrayBuffer(size * size * 4);
    const view: Uint8Array = new Uint8Array(buffer);
    for (let i: number = 0; i < view.length; i += 4) {
      view[i] = 66;      // R
      view[i + 1] = 133; // G
      view[i + 2] = 244; // B
      view[i + 3] = 255; // A
    }
    const opts: image.InitializationOptions = {
      size: { height: size, width: size },
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      editable: false
    };
    try {
      this.synthPixelMap = await image.createPixelMap(buffer, opts);
      this.synthStatus = '合成PixelMap就绪 (64x64 RGBA_8888)';
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      this.synthStatus = '合成失败 code=' + err.code + ' message=' + err.message;
    }
  }

  /**
   * 当前图源（构造参数1），kind 分发到字符串/资源/PixelMap。
   */
  private currentSource(): ResourceStr | image.PixelMap {
    const kind: number = RETRY_SOURCES[this.srcIndex].kind;
    if (kind === 0) {
      return 'retry://broken-uri-001.png';
    } else if (kind === 1) {
      return $r('app.media.icon');
    } else if (kind === 2) {
      return 'data:image/png;base64,QUJDRA==';
    } else if (kind === 3) {
      return this.synthPixelMap !== undefined ? this.synthPixelMap : $r('app.media.star');
    }
    return $r('app.media.testSvg');
  }

  private currentLabel(): string {
    return RETRY_SOURCES[this.srcIndex].label;
  }

  /**
   * 统一事件入列：记录到达顺序与时间戳，供时序观测卡渲染。
   */
  private pushEvent(name: string, detail: string, srcName: string): void {
    this.eventCounter += 1;
    const entry: RetryEventEntry = {
      seq: this.eventCounter,
      name: name,
      srcName: srcName,
      detail: detail,
      timeText: new Date().toLocaleTimeString() + '.' + (Date.now() % 1000).toString()
    };
    this.eventSeq.unshift(entry);
    console.info('ImageRetrySeq#' + entry.seq + ' ' + name + ' src=' + srcName +
      ' detail=' + detail);
  }

  /**
   * 核心重试动作：切换图源 + reloadKey 自增，强制重新解码加载。
   */
  private doRetry(): void {
    this.round += 1;
    this.srcIndex = (this.srcIndex + 1) % RETRY_SOURCES.length;
    this.reloadKey = 'round_' + this.round.toString() + '_idx_' + this.srcIndex.toString();
    this.pushEvent('User', '切换图源并重载 reloadKey=' + this.reloadKey, this.currentLabel());
  }

  /**
   * 不切图源、仅改 reloadKey：验证"地址不变也强制重载"语义。
   */
  private reloadOnly(): void {
    this.round += 1;
    this.reloadKey = 'reloadonly_' + this.round.toString();
    this.pushEvent('User', '仅变更reloadKey(图源不变) reloadKey=' + this.reloadKey,
      this.currentLabel());
  }

  private resetAll(): void {
    this.stopAutoRetry();
    this.srcIndex = 0;
    this.round = 0;
    this.reloadKey = 'round_0';
    this.failCount = 0;
    this.successCount = 0;
    this.decodeCount = 0;
    this.recovered = false;
    this.eventCounter = 0;
    this.stats = makeInitStats();
    this.eventSeq = [];
    this.pushEvent('User', '计数与序列已重置', this.currentLabel());
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('错误计数 + reloadKey 强制重载示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })
        // 控制台卡：主图 + 重试按钮 + 统计
        Column({ space: 8 }) {
          Text('重试控制台（Image(src, reloadKey) 构造形参）').fontSize(16).fontColor(Color.Gray)
          Image(this.currentSource(), this.reloadKey)
            .width(160)
            .height(120)
            .borderWidth(1)
            .borderColor(this.recovered ? '#2E7D32' : '#C62828')
            .borderRadius(8)
            .alt($r('app.media.star'))
            .onComplete((event?: CompleteEventFields) => {
              this.successCount += 1;
              this.bumpStat(this.srcIndex, 'complete');
              if (event !== undefined && event.loadingStatus === 1) {
                this.decodeCount += 1;
                this.bumpStat(this.srcIndex, 'decode');
                this.recovered = true;
                this.pushEvent('onComplete',
                  'loadingStatus=1 解码成功 ' + event.width.toString() + 'x' +
                    event.height.toString() + ' 绘制' + event.contentWidth.toString() + 'x' +
                    event.contentHeight.toString(), this.currentLabel());
              } else {
                this.pushEvent('onComplete',
                  event !== undefined ? 'loadingStatus=0 数据加载成功' : 'onComplete(无参数)',
                  this.currentLabel());
              }
            })
            .onError((e: ImageError) => {
              this.failCount += 1;
              this.bumpStat(this.srcIndex, 'fail');
              this.recovered = false;
              this.pushEvent('onError',
                'message=' + (e.message !== undefined ? e.message : '(API<10无message)') +
                  ' errCode=' + (e.error !== undefined ? e.error.code.toString() : '(API<20无error)'),
                this.currentLabel());
            })
            .onFinish(() => {
              this.pushEvent('onFinish', 'SVG动效播放完成', this.currentLabel());
            })
          Text('当前图源: ' + this.currentLabel()).fontSize(12).maxLines(2)
            .textOverflow({ overflow: TextOverflow.Ellipsis })
          Text('reloadKey = ' + this.reloadKey).fontSize(12).fontFamily('monospace')
          Text('合成位图状态: ' + this.synthStatus).fontSize(12).fontColor('#888888')
          Row({ space: 10 }) {
            Button('切换图源并重载').fontSize(13).onClick(() => {
              this.doRetry();
            })
            Button('仅改reloadKey重载').fontSize(13).onClick(() => {
              this.reloadOnly();
            })
          }
          Button('重置全部计数').fontSize(13).backgroundColor('#888888').onClick(() => {
            this.resetAll();
          })
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 统计卡
        Column({ space: 8 }) {
          Text('失败/成功统计（onError 累计口径）').fontSize(16).fontColor(Color.Gray)
          Row({ space: 12 }) {
            Text('重试轮次: ' + this.round.toString()).fontSize(14).layoutWeight(1)
            Text('onError: ' + this.failCount.toString()).fontSize(14).fontColor(Color.Red)
              .layoutWeight(1)
            Text('onComplete: ' + this.successCount.toString()).fontSize(14)
              .fontColor(Color.Blue).layoutWeight(1)
          }.width('100%')
          Row({ space: 12 }) {
            Text('解码成功(loadingStatus=1): ' + this.decodeCount.toString())
              .fontSize(14).layoutWeight(1)
            Text(this.recovered ? '状态: 已成功恢复展示' : '状态: 最近一次加载失败')
              .fontSize(14)
              .fontColor(this.recovered ? Color.Green : Color.Red)
              .layoutWeight(1)
          }.width('100%')
          Text('提示：连续点击"切换图源并重载"，坏图源使 failCount 增长，' +
            '切回好图源后 successCount/decodeCount 增长且状态恢复。')
            .fontSize(12).fontColor('#888888')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 自动重试卡：定时切换图源，观察 onError/onComplete 交替累计
        Column({ space: 8 }) {
          Text('自动重试（每2秒切换图源+变更reloadKey）').fontSize(16).fontColor(Color.Gray)
          Row({ space: 10 }) {
            Button(this.autoOn ? '停止自动重试' : '开始自动重试').fontSize(13)
              .backgroundColor(this.autoOn ? '#C62828' : '#2E7D32')
              .onClick(() => {
                if (this.autoOn) {
                  this.stopAutoRetry();
                } else {
                  this.startAutoRetry();
                }
              })
            Text(this.autoOn ? '运行中…' : '已停止').fontSize(14)
              .fontColor(this.autoOn ? Color.Green : '#999999')
          }
          Text('自动模式下坏图源持续推高 failCount，好图源使 decodeCount 回升，' +
            '直观呈现"错误计数—重试—恢复"闭环。').fontSize(12).fontColor('#888888')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 分图源计数卡：与队列一一对齐
        Column({ space: 8 }) {
          Text('分图源 onError/onComplete 计数').fontSize(16).fontColor(Color.Gray)
          ForEach(RETRY_SOURCES, (item: RetrySourceDef, index: number) => {
            Column({ space: 2 }) {
              Row({ space: 6 }) {
                Text('[' + index.toString() + ']').fontSize(12).fontColor('#999999').width(28)
                Text(item.label).fontSize(12).layoutWeight(1).maxLines(1)
                  .textOverflow({ overflow: TextOverflow.Ellipsis })
              }.width('100%')
              Row({ space: 10 }) {
                Text('fail=' + this.stats[index].fail.toString())
                  .fontSize(11).fontColor(Color.Red)
                Text('complete=' + this.stats[index].complete.toString())
                  .fontSize(11).fontColor(Color.Blue)
                Text('decode=' + this.stats[index].decode.toString())
                  .fontSize(11).fontColor(Color.Green)
              }.width('100%')
            }
            .width('100%')
            .padding({ top: 3, bottom: 3 })
          }, (item: RetrySourceDef, index: number) => 'stat_' + index.toString())
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 图源队列卡
        Column({ space: 8 }) {
          Text('重试图源队列（循环）').fontSize(16).fontColor(Color.Gray)
          ForEach(RETRY_SOURCES, (item: RetrySourceDef, index: number) => {
            Row({ space: 8 }) {
              Text(index === this.srcIndex ? '▶' : '·')
                .fontSize(13).fontColor(index === this.srcIndex ? Color.Blue : '#BBBBBB').width(18)
              Text(item.label).fontSize(12).layoutWeight(1).maxLines(2)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
                .fontColor(index === this.srcIndex ? '#222222' : '#888888')
              Text(item.expect).fontSize(11).fontColor('#999999').width(110)
            }
            .width('100%')
            .padding({ top: 3, bottom: 3 })
          }, (item: RetrySourceDef, index: number) => 'q_' + index.toString())
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 事件顺序观测卡
        Column({ space: 8 }) {
          Text('onComplete/onError 触发顺序观测（最新在前）').fontSize(16).fontColor(Color.Gray)
          Text('观测点：切换图源瞬间，旧一轮的迟到事件可能先于新一轮事件到达；' +
            '成功加载的 onComplete 会先后到达两条（loadingStatus 0 → 1）。')
            .fontSize(12).fontColor('#888888')
          ForEach(this.eventSeq, (e: RetryEventEntry, index: number) => {
            Row({ space: 8 }) {
              Text('#' + e.seq.toString()).fontSize(12).fontColor('#999999').width(36)
              Text(e.name).fontSize(12).fontWeight(FontWeight.Medium).width(88)
                .fontColor(e.name === 'onError' ? Color.Red
                  : (e.name === 'onComplete' ? Color.Green : Color.Blue))
              Text(e.detail).fontSize(11).layoutWeight(1).maxLines(2)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
              Text(e.timeText).fontSize(10).fontColor('#AAAAAA').width(92)
            }
            .width('100%')
            .padding({ top: 2, bottom: 2 })
          }, (e: RetryEventEntry, index: number) => 'ev_' + e.seq.toString())
          if (this.eventSeq.length === 0) {
            Text('（空）点击按钮开始观测').fontSize(12).fontColor('#BBBBBB')
          }
          if (this.eventSeq.length > 0) {
            Button('清空事件序列').fontSize(12).onClick(() => {
              this.eventSeq = [];
            })
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // reloadKey 语义卡
        Column({ space: 8 }) {
          Text('reloadKey 语义速查（摘自官方文档）').fontSize(16).fontColor(Color.Gray)
          Text('接口: Image(src: PixelMap | ResourceStr | DrawableDescriptor | ImageContent,' +
            ' reloadKey?: string)').fontSize(12).fontFamily('monospace')
          Text('当reloadKey值发生变化时，即使使用相同的图片，也会重新加载图片。' +
            '适用于图片源地址不变但图片内容已更新的场景（例如本地图片内容被覆写）。' +
            '默认值：空字符串。').fontSize(12)
          Text('本页面用法：状态变量驱动构造参数重算，"仅改reloadKey重载"按钮演示同址强刷；' +
            '属性直通式(updateConstructorParams)见 ImageReloadKeyScenarioExample 页面，两页互补不重复。')
            .fontSize(12).fontColor('#888888')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 约束脚注
        Text('约束：onComplete/onError/onFinish 在图源为 AnimatedDrawableDescriptor 时不触发；' +
            'onError 的 error 字段(BusinessError<void>)自API20、message自API10；' +
            'contentWidth等绘制字段仅在loadingStatus=1时有效(API10+)。')
          .fontSize(12).fontColor(Color.Blue).margin({ bottom: 20 })
      }.width('100%').padding({ left: 20, right: 20 })
    }.width('100%').height('100%').backgroundColor(Color.White)
  }
}
