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
 * Image 事件时序观测示例（时间线）
 *
 * 依时间戳把下列事件记录进数组，ForEach 渲染时间线列表，console.log 输出同内容：
 *   - aboutToAppear / onAppear  组件生命周期（文档说明：Image 无 onLoad 事件，
 *                               生命周期可见性由通用事件 onAppear 承担，API7+ 支持）
 *   - onComplete                数据加载成功(loadingStatus=0)与解码成功(loadingStatus=1)各触发一次
 *   - onError                   非法字符串图源加载异常
 *   - onFinish                  testSvg.svg 动效播放完成（无限循环动效不触发）
 *
 * 观测点：
 *   1. 同一张图 onComplete(0) 先于 onComplete(1)；
 *   2. onAppear 早于加载类事件（先挂载后加载）；
 *   3. 变更构造参数 reloadKey 后，全部事件按相同相对顺序重放；
 *   4. onFinish 晚于该 SVG 的 onComplete(1)，触发时机为动效播完。
 */

/**
 * 时间线条目
 */
interface TimelineEntry {
  seq: number;
  tag: string;
  src: string;
  detail: string;
  absTime: string;
  relMs: number;
  gapMs: number; // 与上一条事件的间隔
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
 * 逐字段展示行
 */
interface FieldRow {
  name: string;
  since: string;
  value: string;
}

/**
 * 时间线图源定义
 */
interface TimelineSource {
  label: string;
  kind: number; // 0=icon.svg 1=非法字符串 2=testSvg.svg 3=heart.svg
  brokenStr: string;
}

// 时间线观测图源数组（ForEach 数据源）
const TIMELINE_SOURCES: TimelineSource[] = [
  { label: 'icon（静态SVG）', kind: 0, brokenStr: '' },
  { label: '非法字符串图源', kind: 1, brokenStr: 'timeline://no-such-uri.png' },
  { label: 'testSvg（动效SVG）', kind: 2, brokenStr: '' },
  { label: 'heart（静态SVG）', kind: 3, brokenStr: '' }
];

// 预期事件顺序解读（对照时间线核验；语义均摘自官方文档）
const ORDER_EXPECT_ROWS: FieldRow[] = [
  { name: '1. onAppear', since: '通用', value: '组件挂载即触发，应先于该图源的加载事件' },
  { name: '2. onComplete(0)', since: '9+', value: '数据加载成功先上报（loadingStatus=0）' },
  { name: '3. onComplete(1)', since: '9+', value: '解码成功后上报（loadingStatus=1），绘制字段此时有效' },
  { name: '4. onFinish', since: '9+', value: 'testSvg动效播完才触发，应晚于其onComplete(1)' },
  { name: 'X. onError', since: '9+', value: '非法图源只有onError，无onComplete/onFinish' },
  { name: 'R. 重载重放', since: '-', value: '变更reloadKey后上述序列整体重放，Δ间隔反映加载耗时' }
];

// 时间线过滤标签集合
const FILTER_TAGS: string[] = ['ALL', 'onAppear', 'onComplete', 'onError', 'onFinish', 'User'];

// 事件图例（标签/@since/语义）
const EVENT_LEGEND: FieldRow[] = [
  { name: 'onAppear', since: '通用', value: '组件挂载显示时触发，早于加载事件' },
  { name: 'onComplete', since: '9+', value: '数据加载成功(0)/解码成功(1)各触发一次' },
  { name: 'onError', since: '9+', value: '加载异常时触发，携带ImageError' },
  { name: 'onFinish', since: '9+', value: '动效SVG播放完成时触发，无限循环不触发' }
];

@Entry
@Component
struct ImageEventTimelineExample {
  // 时间线数组（新事件插入头部，seq记录到达顺序）
  @State timeline: TimelineEntry[] = [];
  // 重载标识：变更它使全部Image按新key重新加载，重放事件
  @State reloadKey: string = 'tl_0';
  // 最近一次 onComplete(1) 的完整参数
  @State lastComplete: CompleteEventFields | undefined = undefined;
  // 重载轮次
  @State reloadRound: number = 0;
  // 事件序号自增器
  private seqCounter: number = 0;
  // 时间基准（aboutToAppear记录，用于相对毫秒）
  private baseTs: number = 0;
  // 上一条事件时间戳（用于计算间隔）
  private lastEventTs: number = 0;
  // 当前过滤标签
  @State filterTag: string = 'ALL';

  aboutToAppear(): void {
    this.baseTs = Date.now();
    this.lastEventTs = this.baseTs;
    this.push('aboutToAppear', '页面', '组件生命周期：结构体即将创建build');
  }

  /**
   * 记录一条时间线：数组 + console.log 同内容。
   */
  private push(tag: string, src: string, detail: string): void {
    const now: number = Date.now();
    this.seqCounter += 1;
    const entry: TimelineEntry = {
      seq: this.seqCounter,
      tag: tag,
      src: src,
      detail: detail,
      absTime: new Date(now).toLocaleTimeString() + '.' + (now % 1000).toString(),
      relMs: now - this.baseTs,
      gapMs: now - this.lastEventTs
    };
    this.lastEventTs = now;
    this.timeline.unshift(entry);
    console.info('[ImageTimeline] #' + entry.seq.toString() + ' +' + entry.relMs.toString()
      + 'ms (gap ' + entry.gapMs.toString() + 'ms) ' + tag + ' [' + src + '] ' + detail);
  }

  /**
   * 按当前过滤标签返回时间线子集。
   */
  private filteredTimeline(): TimelineEntry[] {
    if (this.filterTag === 'ALL') {
      return this.timeline;
    }
    const out: TimelineEntry[] = [];
    for (let i: number = 0; i < this.timeline.length; i++) {
      if (this.timeline[i].tag.indexOf(this.filterTag) === 0) {
        out.push(this.timeline[i]);
      }
    }
    return out;
  }

  /**
   * 导出前若干条时间线的原始JSON留档。
   */
  private exportJson(limit: number): string {
    const n: number = this.timeline.length < limit ? this.timeline.length : limit;
    const out: TimelineEntry[] = [];
    for (let i: number = 0; i < n; i++) {
      out.push(this.timeline[i]);
    }
    return JSON.stringify(out);
  }

  /**
   * 按图源kind返回Image构造参数1。
   */
  private sourceOf(item: TimelineSource): ResourceStr {
    if (item.kind === 0) {
      return $r('app.media.icon');
    } else if (item.kind === 1) {
      return item.brokenStr;
    } else if (item.kind === 2) {
      return $r('app.media.testSvg');
    }
    return $r('app.media.heart');
  }

  /**
   * onComplete 回调统一入口：拆字段落时间线并留存最近参数。
   */
  private onImgComplete(srcLabel: string, event?: CompleteEventFields): void {
    if (event === undefined) {
      this.push('onComplete', srcLabel, '无参数回调');
      return;
    }
    if (event.loadingStatus === 1) {
      this.lastComplete = event;
      this.push('onComplete(1)', srcLabel,
        '解码成功 图片' + event.width.toString() + 'x' + event.height.toString()
          + ' 组件' + event.componentWidth.toString() + 'x' + event.componentHeight.toString()
          + ' 绘制' + event.contentWidth.toString() + 'x' + event.contentHeight.toString()
          + ' 偏移(' + event.contentOffsetX.toString() + ',' + event.contentOffsetY.toString() + ')');
    } else {
      this.push('onComplete(0)', srcLabel,
        '数据加载成功 图片' + event.width.toString() + 'x' + event.height.toString());
    }
  }

  /**
   * onError 回调统一入口：逐关键字段摘录。
   */
  private onImgError(srcLabel: string, e: ImageError): void {
    this.push('onError', srcLabel,
      'message=' + (e.message !== undefined ? e.message : '(API<10)')
        + ' code=' + (e.error !== undefined ? e.error.code.toString() : '(API<20)')
        + ' 组件=' + e.componentWidth.toFixed(1) + 'x' + e.componentHeight.toFixed(1));
  }

  /**
   * 最近 onComplete(1) 参数的逐字段行。
   */
  private buildCompleteRows(e: CompleteEventFields): FieldRow[] {
    const rows: FieldRow[] = [
      { name: 'width', since: '9+', value: e.width.toString() + ' px' },
      { name: 'height', since: '9+', value: e.height.toString() + ' px' },
      { name: 'componentWidth', since: '9+', value: e.componentWidth.toString() + ' px' },
      { name: 'componentHeight', since: '9+', value: e.componentHeight.toString() + ' px' },
      { name: 'loadingStatus', since: '9+', value: e.loadingStatus.toString() + ' (1=解码成功)' },
      { name: 'contentWidth', since: '10+', value: e.contentWidth.toString() + ' px' },
      { name: 'contentHeight', since: '10+', value: e.contentHeight.toString() + ' px' },
      { name: 'contentOffsetX', since: '10+', value: e.contentOffsetX.toString() + ' px' },
      { name: 'contentOffsetY', since: '10+', value: e.contentOffsetY.toString() + ' px' }
    ];
    return rows;
  }

  /**
   * 事件标签着色（时间线列表用）。
   */
  private tagColor(tag: string): string {
    if (tag.indexOf('onError') === 0) {
      return '#C62828';
    }
    if (tag.indexOf('onComplete') === 0) {
      return '#2E7D32';
    }
    if (tag.indexOf('onFinish') === 0) {
      return '#6A1B9A';
    }
    if (tag.indexOf('onAppear') === 0) {
      return '#EF6C00';
    }
    return '#1565C0';
  }

  /**
   * 某标签的条数统计（从timeline数组即时计算，不额外维护状态）。
   */
  private countOf(prefix: string): number {
    let n: number = 0;
    for (let i: number = 0; i < this.timeline.length; i++) {
      if (this.timeline[i].tag.indexOf(prefix) === 0) {
        n += 1;
      }
    }
    return n;
  }

  private replayAll(): void {
    this.reloadRound += 1;
    this.reloadKey = 'tl_' + this.reloadRound.toString();
    this.push('User', '页面', '变更reloadKey=' + this.reloadKey + '，全部图源强制重新加载');
  }

  private clearTimeline(): void {
    this.timeline = [];
    this.seqCounter = 0;
    this.baseTs = Date.now();
    this.lastEventTs = this.baseTs;
    this.push('User', '页面', '时间线已清空，重新计时');
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('Image 事件时序观测（时间线）')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })
        // 事件图例卡
        Column({ space: 8 }) {
          Text('事件图例与语义（对照官方文档）').fontSize(16).fontColor(Color.Gray)
          ForEach(EVENT_LEGEND, (row: FieldRow, index: number) => {
            Row({ space: 8 }) {
              Text(row.name).fontSize(13).fontWeight(FontWeight.Medium).width(110)
                .fontColor(this.tagColor(row.name))
              Text(row.since).fontSize(12).fontColor(Color.Blue).width(40)
              Text(row.value).fontSize(12).layoutWeight(1).maxLines(2)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
            }
            .width('100%')
          }, (row: FieldRow, index: number) => 'lgd_' + index.toString())
          Text('注：Image组件不存在 onLoad 事件；组件生命周期可见性使用通用事件 onAppear 观测。')
            .fontSize(11).fontColor('#999999')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 观测图源阵列卡：ForEach 渲染四路 Image 及各自事件
        Column({ space: 8 }) {
          Text('观测图源阵列（reloadKey第' + this.reloadRound.toString() + '轮）')
            .fontSize(16).fontColor(Color.Gray)
          GridRow({ columns: 2, gutter: 10 }) {
            ForEach(TIMELINE_SOURCES, (item: TimelineSource, index: number) => {
              GridCol({ span: 1 }) {
                Column({ space: 4 }) {
                  Text(item.label).fontSize(12).maxLines(1)
                    .textOverflow({ overflow: TextOverflow.Ellipsis })
                  Image(this.sourceOf(item), this.reloadKey)
                    .width('100%')
                    .height(72)
                    .objectFit(ImageFit.Contain)
                    .borderWidth(1)
                    .borderColor('#D0D0D0')
                    .borderRadius(6)
                    .onAppear(() => {
                      this.push('onAppear', item.label, '组件挂载显示');
                    })
                    .onComplete((event?: CompleteEventFields) => {
                      this.onImgComplete(item.label, event);
                    })
                    .onError((e: ImageError) => {
                      this.onImgError(item.label, e);
                    })
                    .onFinish(() => {
                      this.push('onFinish', item.label, 'SVG动效播放完成');
                    })
                }
                .width('100%')
              }
            }, (item: TimelineSource, index: number) => 'tl_' + index.toString())
          }
          Row({ space: 10 }) {
            Button('全部重载(变更reloadKey)').fontSize(13).onClick(() => {
              this.replayAll();
            })
            Button('清空时间线').fontSize(13).backgroundColor('#888888').onClick(() => {
              this.clearTimeline();
            })
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 预期顺序解读卡
        Column({ space: 8 }) {
          Text('预期事件顺序解读（与时间线对照核验）').fontSize(16).fontColor(Color.Gray)
          ForEach(ORDER_EXPECT_ROWS, (row: FieldRow, index: number) => {
            Row({ space: 8 }) {
              Text(row.name).fontSize(12).fontWeight(FontWeight.Medium).width(110)
              Text(row.value).fontSize(12).layoutWeight(1).maxLines(2)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
            }
            .width('100%')
            .padding({ top: 2, bottom: 2 })
          }, (row: FieldRow, index: number) => 'ord_' + index.toString())
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 事件计数概览卡
        Column({ space: 8 }) {
          Text('事件计数概览').fontSize(16).fontColor(Color.Gray)
          Flex({ wrap: FlexWrap.Wrap }) {
            Text('onAppear x' + this.countOf('onAppear').toString())
              .fontSize(12).fontColor('#EF6C00').margin({ right: 12 })
            Text('onComplete x' + this.countOf('onComplete').toString())
              .fontSize(12).fontColor('#2E7D32').margin({ right: 12 })
            Text('onError x' + this.countOf('onError').toString())
              .fontSize(12).fontColor('#C62828').margin({ right: 12 })
            Text('onFinish x' + this.countOf('onFinish').toString())
              .fontSize(12).fontColor('#6A1B9A').margin({ right: 12 })
            Text('总计 x' + this.timeline.length.toString())
              .fontSize(12).fontColor('#1565C0')
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 时间线列表卡（核心，含过滤）
        Column({ space: 8 }) {
          Text('事件时间线（最新在前，+为相对页面创建毫秒）').fontSize(16).fontColor(Color.Gray)
          Flex({ wrap: FlexWrap.Wrap, justifyContent: FlexAlign.Start }) {
            ForEach(FILTER_TAGS, (tag: string, index: number) => {
              Button(tag).fontSize(11)
                .height(28)
                .backgroundColor(this.filterTag === tag ? '#1565C0' : '#E0E0E0')
                .fontColor(this.filterTag === tag ? Color.White : '#333333')
                .margin({ right: 8, bottom: 6 })
                .onClick(() => {
                  this.filterTag = tag;
                })
            }, (tag: string, index: number) => 'flt_' + tag)
          }
          ForEach(this.filteredTimeline(), (e: TimelineEntry, index: number) => {
            Row({ space: 6 }) {
              Text('#' + e.seq.toString()).fontSize(11).fontColor('#999999').width(34)
              Text('+' + e.relMs.toString() + 'ms').fontSize(11).fontColor('#666666').width(64)
              Text('Δ' + e.gapMs.toString() + 'ms').fontSize(10).fontColor('#AAAAAA').width(58)
              Text(e.tag).fontSize(12).fontWeight(FontWeight.Medium).width(100)
                .fontColor(this.tagColor(e.tag)).maxLines(1)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
              Text(e.src).fontSize(11).width(76).maxLines(1)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
              Text(e.detail).fontSize(11).layoutWeight(1).maxLines(2)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
            }
            .width('100%')
            .padding({ top: 3, bottom: 3 })
            .border({ width: { bottom: 1 }, color: '#E8E8E8' })
          }, (e: TimelineEntry, index: number) => 'tme_' + e.seq.toString())
          if (this.filteredTimeline().length === 0) {
            Text('（当前过滤无匹配事件）').fontSize(12).fontColor('#BBBBBB')
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 时间线导出卡
        Column({ space: 8 }) {
          Text('时间线原始导出（前10条 JSON.stringify，与console.log同内容）')
            .fontSize(16).fontColor(Color.Gray)
          Text(this.exportJson(10))
            .fontSize(11)
            .fontFamily('monospace')
            .padding(8)
            .backgroundColor('#FFFFFF')
            .borderRadius(6)
            .borderWidth(1)
            .borderColor('#E0E0E0')
            .width('100%')
            .maxLines(8)
            .textOverflow({ overflow: TextOverflow.Ellipsis })
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 最近 onComplete(1) 逐字段卡
        Column({ space: 8 }) {
          Text('最近一次 onComplete(1) 回调参数').fontSize(16).fontColor(Color.Gray)
          if (this.lastComplete !== undefined) {
            ForEach(this.buildCompleteRows(this.lastComplete),
              (row: FieldRow, index: number) => {
                Row({ space: 8 }) {
                  Text(row.name).fontSize(12).fontWeight(FontWeight.Medium).width(130)
                  Text(row.since).fontSize(11).fontColor(Color.Blue).width(40)
                  Text(row.value).fontSize(12).layoutWeight(1)
                }
                .width('100%')
              }, (row: FieldRow, index: number) => 'cpn_' + index.toString() + row.name)
          } else {
            Text('（尚未捕获解码成功事件）').fontSize(12).fontColor('#BBBBBB')
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 约束脚注
        Text('约束：onFinish 仅对带动效SVG生效且无限循环动效不触发；AnimatedDrawableDescriptor'
          + '图源不触发onComplete/onError/onFinish；contentWidth等绘制字段(10+)仅在'
          + 'loadingStatus=1时有效。')
          .fontSize(12).fontColor(Color.Blue).margin({ bottom: 20 })
      }.width('100%').padding({ left: 20, right: 20 })
    }.width('100%').height('100%').backgroundColor(Color.White)
  }
}
