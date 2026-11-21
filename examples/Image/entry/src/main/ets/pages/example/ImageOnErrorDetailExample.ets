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
 * Image onError<9+> 全链路示例
 *
 * 覆盖 onError(callback: ImageErrorCallback) 的多种触发场景：
 *   1. 非法字符串图源（未知scheme）        —— 对应文档错误码 101000 unknown source type.
 *   2. Base64 前缀/格式非法               —— 对应文档错误码 102030 wrong code format.
 *   3. Base64 内容非有效图片              —— 对应文档错误码 102031 decode base64 image failed.
 *   4. 沙箱文件不存在（file://字符串路径） —— 对应文档错误码 102051 read data failed.
 *   5. 空ArrayBuffer尝试创建PixelMap      —— ImageKit createPixelMap 直接抛错或在Image解码期报错。
 *   6. 尺寸非法(0x0)尝试创建PixelMap      —— 对应文档错误码 103200 image data size is invalid.
 *
 * 每个场景通过 ForEach 渲染独立错误卡；onError 回调将 ImageError(9+) 的
 * componentWidth / componentHeight / message(10+) / error(20+, BusinessError<void>)
 * / downloadInfo(23+, RequestDownloadInfo) 逐字段快照展示。
 *
 * 错误码文案均逐条摘自官方参考文档 ts-basic-components-image.md
 * BusinessError<20+> 小节错误信息表，未列出的错误码一律不引用。
 */
import { image } from '@kit.ImageKit';
import { BusinessError } from '@kit.BasicServicesKit';

/**
 * onError 触发场景定义（模块级常量数据，不在build()内定义）
 */
interface OnErrorScenario {
  title: string;
  desc: string;
  srcKind: number; // 0=字符串图源 1=PixelMap图源(异步创建)
  srcText: string; // 图源的可读展示
  expectDoc: string; // 文档预期错误码说明
  slot: number; // PixelMap图源槽位（srcKind=1时有效）
}

/**
 * ImageError(9+) 回调参数的逐字段快照
 */
interface ImageErrorSnapshot {
  scenario: string;
  componentWidth: number;
  componentHeight: number;
  message: string; // message字段 10+
  errCode: number; // error.code字段 20+
  errMsg: string; // error.message字段 20+
  hasDownloadInfo: boolean; // downloadInfo字段 23+（本地图源失败时为null）
  timeText: string;
  rawJson: string; // 回调对象整体序列化留档（便于逐字段核对）
}

/**
 * 场景-预期错误码对照行
 */
interface ScenarioExpectRow {
  scenario: string;
  srcForm: string;
  expect: string;
  stage: string;
}

/**
 * 官方文档错误信息表摘录（code/message/阶段/类型）
 */
interface DocErrorCodeRow {
  code: string;
  msg: string;
  stage: string;
  srcType: string;
}

/**
 * 逐字段展示行（字段名 + @since + 实际值）
 */
interface FieldRow {
  name: string;
  since: string;
  value: string;
}

// PixelMap图源槽位数量
const PIXELMAP_SLOT_COUNT: number = 2;

// 场景数组：ForEach 渲染错误卡的数据源
const ERROR_SCENARIOS: OnErrorScenario[] = [
  {
    title: '场景1 非法scheme字符串',
    desc: 'Image(\'demo-not-a-supported-uri://no-such-host/x.png\')',
    srcKind: 0,
    srcText: 'demo-not-a-supported-uri://no-such-host/x.png',
    expectDoc: '101000 unknown source type.（数据加载/未知类型）',
    slot: -1
  },
  {
    title: '场景2 Base64格式非法',
    desc: 'Image(\'data:image/png;base64,%%%缺少合法编码字符%%%\')',
    srcKind: 0,
    srcText: 'data:image/png;base64,%%%缺少合法编码字符%%%',
    expectDoc: '102030 wrong code format.（数据加载/base64字符串文件）',
    slot: -1
  },
  {
    title: '场景3 Base64内容非图片',
    desc: 'Image(\'data:image/png;base64,QUJDRA==\') 解码后为"ABCD"而非PNG数据',
    srcKind: 0,
    srcText: 'data:image/png;base64,QUJDRA==',
    expectDoc: '102031 decode base64 image failed.（数据加载/base64字符串文件）',
    slot: -1
  },
  {
    title: '场景4 沙箱文件不存在',
    desc: 'Image(\'file:///data/storage/el2/base/haps/entry/files/no_this_file.png\')',
    srcKind: 0,
    srcText: 'file:///data/storage/el2/base/haps/entry/files/no_this_file.png',
    expectDoc: '102051 read data failed.（数据加载/沙箱文件）',
    slot: -1
  },
  {
    title: '场景5 空ArrayBuffer构造PixelMap',
    desc: 'image.createPixelMap(new ArrayBuffer(0), 100x100选项)：数据为空',
    srcKind: 1,
    srcText: 'empty ArrayBuffer PixelMap',
    expectDoc: '创建即抛ImageKit错误；若成功则解码期 111000/111001',
    slot: 0
  },
  {
    title: '场景6 尺寸非法PixelMap',
    desc: 'image.createPixelMap(buffer, size 0x0)：宽高非法',
    srcKind: 1,
    srcText: 'zero size PixelMap',
    expectDoc: '103200 image data size is invalid.（数据加载/位图文件）',
    slot: 1
  }
];

// 文档错误码参考卡数据（逐字摘自官方文档错误信息表）
const DOC_ERROR_CODE_ROWS: DocErrorCodeRow[] = [
  { code: '101000', msg: 'unknown source type.', stage: '数据加载', srcType: '未知类型' },
  { code: '102010', msg: 'sync http task of uri cancelled.', stage: '数据加载', srcType: '网络文件' },
  { code: '102011', msg: 'sync http task of uri failed.', stage: '数据加载', srcType: '网络文件' },
  { code: '102012', msg: 'async http task of uri cancelled.', stage: '数据加载', srcType: '网络文件' },
  { code: '102013', msg: 'async http task of uri failed.', stage: '数据加载', srcType: '网络文件' },
  { code: '102030', msg: 'wrong code format.', stage: '数据加载', srcType: 'base64字符串文件' },
  { code: '102031', msg: 'decode base64 image failed.', stage: '数据加载', srcType: 'base64字符串文件' },
  { code: '102050', msg: 'path is too long.', stage: '数据加载', srcType: '沙箱文件' },
  { code: '102051', msg: 'read data failed.', stage: '数据加载', srcType: '沙箱文件' },
  { code: '102070', msg: 'get image data by name failed.', stage: '数据加载', srcType: '资源文件' },
  { code: '102072', msg: 'uri is invalid.', stage: '数据加载', srcType: '资源文件' },
  { code: '102090', msg: 'uri is invalid.', stage: '数据加载', srcType: '包内文件' },
  { code: '102091', msg: 'get asset failed.', stage: '数据加载', srcType: '包内文件' },
  { code: '103100', msg: 'make svg dom failed.', stage: '数据加载', srcType: '矢量图文件' },
  { code: '103200', msg: 'image data size is invalid.', stage: '数据加载', srcType: '位图文件' },
  { code: '111000', msg: 'image source create failed.', stage: '数据解码', srcType: '位图文件' },
  { code: '111001', msg: 'pixelmap create failed.', stage: '数据解码', srcType: '位图文件' }
];

// 场景-预期错误码对照矩阵（与上方 ERROR_SCENARIOS 一一对应）
const SCENARIO_EXPECT_MATRIX: ScenarioExpectRow[] = [
  { scenario: '场景1 非法scheme字符串', srcForm: 'URI字符串(未知scheme)', expect: '101000 unknown source type.', stage: '数据加载' },
  { scenario: '场景2 Base64格式非法', srcForm: 'data:前缀+非法字符', expect: '102030 wrong code format.', stage: '数据加载' },
  { scenario: '场景3 Base64内容非图片', srcForm: 'data:前缀+合法Base64', expect: '102031 decode base64 image failed.', stage: '数据加载' },
  { scenario: '场景4 沙箱文件不存在', srcForm: 'file://沙箱路径', expect: '102051 read data failed.', stage: '数据加载' },
  { scenario: '场景5 空ArrayBuffer PixelMap', srcForm: 'PixelMap(空数据)', expect: 'ImageKit抛错 或 111000/111001', stage: '数据加载/数据解码' },
  { scenario: '场景6 尺寸非法 PixelMap', srcForm: 'PixelMap(0x0)', expect: '103200 image data size is invalid.', stage: '数据加载' }
];

@Entry
@Component
struct ImageOnErrorDetailExample {
  // onError 回调捕获的 ImageError 快照列表（新记录插入头部）
  @State errorSnapshots: ImageErrorSnapshot[] = [];
  // PixelMap 图源槽位（场景5/6），null语义用undefined表示"创建失败或未完成"
  @State brokenPixelMaps: (image.PixelMap | undefined)[] =
    new Array<image.PixelMap | undefined>(PIXELMAP_SLOT_COUNT).fill(undefined);
  // PixelMap 槽位的创建状态文案（失败时展示ImageKit抛出的BusinessError）
  @State pixelMapStatus: string[] =
    new Array<string>(PIXELMAP_SLOT_COUNT).fill('PixelMap创建中…');
  // 是否展示文档错误码参考表（折叠卡开关）
  @State showDocTable: boolean = false;

  async aboutToAppear(): Promise<void> {
    await this.tryCreateEmptyBufferPixelMap(0);
    await this.tryCreateZeroSizePixelMap(1);
  }

  /**
   * 场景5：空ArrayBuffer创建PixelMap。
   * createPixelMap可能直接抛出BusinessError（数据加载在ImageKit侧即失败），
   * 此时onError不会触发，卡片展示抛错信息；若创建成功则交给Image解码验证。
   */
  private async tryCreateEmptyBufferPixelMap(slot: number): Promise<void> {
    const opts: image.InitializationOptions = {
      size: { height: 100, width: 100 },
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      editable: false
    };
    try {
      const pm: image.PixelMap = await image.createPixelMap(new ArrayBuffer(0), opts);
      this.brokenPixelMaps[slot] = pm;
      this.pixelMapStatus[slot] = '空buffer创建意外成功，转由Image解码校验';
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      this.pixelMapStatus[slot] =
        'createPixelMap抛错 code=' + err.code + ' message=' + err.message;
    }
  }

  /**
   * 场景6：尺寸非法(0x0)创建PixelMap，同样双路兜底。
   */
  private async tryCreateZeroSizePixelMap(slot: number): Promise<void> {
    const opts: image.InitializationOptions = {
      size: { height: 0, width: 0 },
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      editable: false
    };
    try {
      const pm: image.PixelMap = await image.createPixelMap(new ArrayBuffer(4), opts);
      this.brokenPixelMaps[slot] = pm;
      this.pixelMapStatus[slot] = '0x0尺寸创建意外成功，转由Image解码校验';
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      this.pixelMapStatus[slot] =
        'createPixelMap抛错 code=' + err.code + ' message=' + err.message;
    }
  }

  /**
   * 统一记录 onError 回调参数：ImageError(9+) 逐字段落快照。
   */
  private recordError(scenarioTitle: string, e: ImageError): void {
    const snapshot: ImageErrorSnapshot = {
      scenario: scenarioTitle,
      componentWidth: e.componentWidth,
      componentHeight: e.componentHeight,
      message: e.message !== undefined ? e.message : '(无message字段, API<10)',
      errCode: e.error !== undefined ? e.error.code : -1,
      errMsg: e.error !== undefined ? e.error.message : '(无error字段, API<20)',
      hasDownloadInfo: e.downloadInfo !== undefined && e.downloadInfo !== null,
      timeText: new Date().toLocaleTimeString(),
      rawJson: JSON.stringify(e)
    };
    this.errorSnapshots.unshift(snapshot);
    console.error('onError captured: ' + JSON.stringify(snapshot));
  }

  /**
   * 将最新一条快照展开为逐字段展示行（字段名/@since/实际值）。
   */
  private buildFieldRows(s: ImageErrorSnapshot): FieldRow[] {
    const rows: FieldRow[] = [
      { name: 'scenario', since: '-', value: s.scenario },
      { name: 'componentWidth', since: '9+', value: s.componentWidth.toFixed(1) + ' px' },
      { name: 'componentHeight', since: '9+', value: s.componentHeight.toFixed(1) + ' px' },
      { name: 'message', since: '10+', value: s.message },
      { name: 'error.code', since: '20+', value: s.errCode.toString() },
      { name: 'error.message', since: '20+', value: s.errMsg },
      { name: 'downloadInfo', since: '23+',
        value: s.hasDownloadInfo ? '存在(网络图片下载失败时携带)' : 'null(本地图源失败不携带)' },
      { name: '捕获时间', since: '-', value: s.timeText }
    ];
    return rows;
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('onError<9+> 全链路错误场景示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })
        // 说明卡
        Column({ space: 8 }) {
          Text('onError 回调签名与触发语义').fontSize(16).fontColor(Color.Gray)
          Text('onError(callback: ImageErrorCallback)：图片加载异常时触发；' +
            'ImageErrorCallback = (error: ImageError) => void。')
            .fontSize(13)
          Text('AnimatedDrawableDescriptor图源不触发onError；' +
            '建议借助ImageError逐字段定位失败原因（本卡片组即逐字段展示）。')
            .fontSize(13)
          Text('当前已捕获错误数: ' + this.errorSnapshots.length).fontSize(14)
            .fontColor(this.errorSnapshots.length > 0 ? Color.Red : Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 场景错误卡：ForEach 渲染 ERROR_SCENARIOS
        ForEach(ERROR_SCENARIOS, (scenario: OnErrorScenario, index: number) => {
          Column({ space: 8 }) {
            Text(scenario.title + '（onError<9+>）').fontSize(16).fontColor(Color.Gray)
            Text('触发方式: ' + scenario.desc).fontSize(12).maxLines(2)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
            Text('文档预期: ' + scenario.expectDoc).fontSize(12).fontColor('#888888')
              .maxLines(2)
            if (scenario.srcKind === 0) {
              Image(scenario.srcText)
                .width(120)
                .height(80)
                .borderWidth(1)
                .borderColor('#D0D0D0')
                .alt($r('app.media.star'))
                .onError((e: ImageError) => {
                  this.recordError(scenario.title, e);
                })
            } else {
              if (this.brokenPixelMaps[scenario.slot] !== undefined) {
                Image(this.brokenPixelMaps[scenario.slot])
                  .width(120)
                  .height(80)
                  .borderWidth(1)
                  .borderColor('#D0D0D0')
                  .onError((e: ImageError) => {
                    this.recordError(scenario.title, e);
                  })
              } else {
                Text(this.pixelMapStatus[scenario.slot])
                  .fontSize(12)
                  .fontColor(Color.Red)
                  .padding(8)
                  .backgroundColor('#FDE8E8')
                  .borderRadius(6)
                  .maxLines(3)
                  .textOverflow({ overflow: TextOverflow.Ellipsis })
              }
            }
            Text('图源: ' + scenario.srcText).fontSize(11).fontColor('#AAAAAA').maxLines(1)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
          }
          .width('100%')
          .padding(15)
          .backgroundColor('#F5F5F5')
          .borderRadius(10)
        }, (scenario: OnErrorScenario, index: number) => 'scn_' + index.toString())
        // 最新一条 ImageError 逐字段快照卡
        Column({ space: 8 }) {
          Text('最近一次 onError 回调参数逐字段解读').fontSize(16).fontColor(Color.Gray)
          if (this.errorSnapshots.length > 0) {
            ForEach(this.buildFieldRows(this.errorSnapshots[0]),
              (row: FieldRow, index: number) => {
                Row({ space: 8 }) {
                  Text(row.name).fontSize(13).fontWeight(FontWeight.Medium).width(130)
                  Text(row.since).fontSize(12).fontColor(Color.Blue).width(46)
                  Text(row.value).fontSize(12).layoutWeight(1).maxLines(2)
                    .textOverflow({ overflow: TextOverflow.Ellipsis })
                }
                .width('100%')
              }, (row: FieldRow, index: number) => 'fld_' + index.toString() + row.name)
            Divider().strokeWidth(1)
            Text('回调对象原始序列化(JSON.stringify)：').fontSize(12).fontColor('#888888')
            Text(this.errorSnapshots[0].rawJson)
              .fontSize(11)
              .fontFamily('monospace')
              .padding(8)
              .backgroundColor('#FFFFFF')
              .borderRadius(6)
              .borderWidth(1)
              .borderColor('#E0E0E0')
              .width('100%')
              .maxLines(4)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
            Button('清空错误记录').fontSize(13).onClick(() => {
              this.errorSnapshots = [];
            })
          } else {
            Text('暂无记录：等待上方任一失败图源触发回调').fontSize(13).fontColor('#999999')
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 场景-预期错误码对照矩阵卡
        Column({ space: 8 }) {
          Text('场景 × 文档错误码对照矩阵').fontSize(16).fontColor(Color.Gray)
          Row({ space: 6 }) {
            Text('场景').fontSize(12).fontWeight(FontWeight.Bold).layoutWeight(5)
            Text('图源形态').fontSize(12).fontWeight(FontWeight.Bold).layoutWeight(4)
            Text('预期错误信息').fontSize(12).fontWeight(FontWeight.Bold).layoutWeight(6)
          }.width('100%')
          ForEach(SCENARIO_EXPECT_MATRIX, (row: ScenarioExpectRow, index: number) => {
            Column({ space: 2 }) {
              Row({ space: 6 }) {
                Text(row.scenario).fontSize(12).layoutWeight(5).maxLines(2)
                  .textOverflow({ overflow: TextOverflow.Ellipsis })
                Text(row.srcForm).fontSize(12).layoutWeight(4).maxLines(2)
                  .textOverflow({ overflow: TextOverflow.Ellipsis })
                Text(row.expect).fontSize(12).layoutWeight(6).maxLines(2)
                  .textOverflow({ overflow: TextOverflow.Ellipsis })
              }.width('100%')
              Text('发生阶段: ' + row.stage).fontSize(11).fontColor('#AAAAAA')
            }
            .width('100%')
            .padding({ top: 3, bottom: 3 })
          }, (row: ScenarioExpectRow, index: number) => 'mtx_' + index.toString())
          Text('注：实际code以设备运行时回调为准；本表仅罗列文档已定义的错误信息。')
            .fontSize(11).fontColor('#999999')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 历史快照列表卡
        Column({ space: 8 }) {
          Text('历史 onError 记录（' + this.errorSnapshots.length + '条）')
            .fontSize(16).fontColor(Color.Gray)
          ForEach(this.errorSnapshots, (s: ImageErrorSnapshot, index: number) => {
            Row({ space: 8 }) {
              Text('#' + (this.errorSnapshots.length - index).toString())
                .fontSize(12).fontColor('#999999').width(30)
              Text(s.scenario).fontSize(12).layoutWeight(1).maxLines(1)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
              Text('code=' + s.errCode.toString()).fontSize(12)
                .fontColor(s.errCode === -1 ? '#BB8800' : Color.Red)
              Text(s.timeText).fontSize(12).fontColor('#999999')
            }
            .width('100%')
          }, (s: ImageErrorSnapshot, index: number) => 'his_' + s.timeText + index.toString())
          if (this.errorSnapshots.length === 0) {
            Text('（空）').fontSize(12).fontColor('#BBBBBB')
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 文档错误码参考表卡（折叠）
        Column({ space: 8 }) {
          Text('图片加载错误信息表（ImageError.error.code，摘自官方文档）')
            .fontSize(16).fontColor(Color.Gray)
          Button(this.showDocTable ? '收起错误码表' : '展开错误码表').fontSize(13).onClick(() => {
            this.showDocTable = !this.showDocTable;
          })
          if (this.showDocTable) {
            Row({ space: 6 }) {
              Text('错误码ID').fontSize(12).fontWeight(FontWeight.Bold).width(64)
              Text('错误信息').fontSize(12).fontWeight(FontWeight.Bold).layoutWeight(1)
              Text('阶段').fontSize(12).fontWeight(FontWeight.Bold).width(56)
            }.width('100%')
            ForEach(DOC_ERROR_CODE_ROWS, (row: DocErrorCodeRow, index: number) => {
              Column({ space: 2 }) {
                Row({ space: 6 }) {
                  Text(row.code).fontSize(12).width(64)
                  Text(row.msg).fontSize(12).layoutWeight(1).maxLines(1)
                    .textOverflow({ overflow: TextOverflow.Ellipsis })
                  Text(row.stage).fontSize(12).width(56)
                }.width('100%')
                Text('图片加载类型: ' + row.srcType).fontSize(11).fontColor('#AAAAAA')
              }
              .width('100%')
              .padding({ top: 3, bottom: 3 })
            }, (row: DocErrorCodeRow, index: number) => 'doc_' + row.code + index.toString())
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 约束脚注
        Text('约束：message字段自API10、error字段(BusinessError<void>)自API20、' +
          'downloadInfo字段(RequestDownloadInfo)自API23；' +
          'downloadInfo仅在\"网络图片下载失败\"时携带，本页面全部为本地/非法图源，' +
          '预期该字段为null。')
          .fontSize(12).fontColor(Color.Blue).margin({ bottom: 20 })
      }.width('100%').padding({ left: 20, right: 20 })
    }.width('100%').height('100%').backgroundColor(Color.White)
  }
}
