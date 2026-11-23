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
 * Image onError 版本字段解读示例（API 9 / 20 / 23 三代参数形态）
 *
 * 聚焦 ImageError(9+) 在版本演进中新增的两类字段：
 *   1. error<20+> : BusinessError<void>
 *      文档定义: type BusinessError<T = void> = import('../api/@ohos.base').BusinessError<T>
 *      图片加载异常返回的报错信息，code为错误码、message为错误信息，
 *      默认值 { code : -1, message : "" }。
 *   2. downloadInfo<23+> : RequestDownloadInfo
 *      文档定义: type RequestDownloadInfo =
 *      import('../api/@ohos.request.cacheDownload').default.DownloadInfo
 *      网络图片下载的详细信息（资源/网络/性能三类），默认值 null，
 *      仅当图片来源为网络图片且下载失败时携带。
 *
 * DownloadInfo 字段表逐条摘自 js-apis-request-cacheDownload.md
 * （ResourceInfo/NetworkInfo/PerformanceInfo/DownloadInfo 小节）。
 * 文档未定义的字段（如个别资料流传的 requestSucess/needNetwork）一律不展示。
 *
 * 触发方式说明：本页使用"形态非法的URI字符串"触发网络类错误路径，
 * 引擎在数据加载前置校验阶段即失败，不产生真实网络请求；
 * 因此预期 downloadInfo 为 null（非网络下载失败），页面如实标注。
 */

/**
 * onError(20+/23+字段) 捕获快照
 */
interface V20Capture {
  scenario: string;
  hasErrorField: boolean;
  errCode: number;
  errMsg: string;
  downloadInfoText: string;
  message10: string;
  rawJson: string;
  timeText: string;
}

/**
 * 9/20/23 三版参数差异行
 */
interface VersionDiffRow {
  field: string;
  v9: string;
  v20: string;
  v23: string;
  note: string;
}

/**
 * RequestDownloadInfo(DownloadInfo) 字段解读行
 */
interface DlFieldRow {
  path: string;
  since: string;
  type: string;
  desc: string;
}

/**
 * 非法URI触发场景定义（不产生真实网络请求）
 */
interface TriggerDef {
  title: string;
  src: string;
  expect: string;
  why: string;
}

/**
 * 图源形态 -> 失败阶段 -> 文档错误码 映射行
 */
interface UriStageRow {
  form: string;
  failStage: string;
  docCode: string;
  dl: string;
}

// 图源形态与失败阶段映射（阶段/错误码均摘自文档错误信息表）
const URI_STAGE_ROWS: UriStageRow[] = [
  { form: '未知scheme字符串', failStage: '数据加载/未知类型', docCode: '101000', dl: '不携带(非网络下载)' },
  { form: 'Base64前缀/字符非法', failStage: '数据加载/base64字符串文件', docCode: '102030', dl: '不携带' },
  { form: 'Base64内容非有效图片', failStage: '数据加载/base64字符串文件', docCode: '102031', dl: '不携带' },
  { form: '沙箱文件不存在', failStage: '数据加载/沙箱文件', docCode: '102051', dl: '不携带' },
  { form: '资源名不存在($r)', failStage: '数据加载/资源文件', docCode: '102070', dl: '不携带' },
  { form: '网络URL下载失败', failStage: '数据加载/网络文件', docCode: '102011/102013', dl: '23+携带downloadInfo' }
];

// 三版参数差异表数据（对照文档 ImageError 属性表）
const VERSION_DIFF_ROWS: VersionDiffRow[] = [
  {
    field: 'componentWidth / componentHeight',
    v9: '有(9+)',
    v20: '沿用',
    v23: '沿用',
    note: '组件宽高，单位px；9+即有的基础字段'
  },
  {
    field: 'message',
    v9: '无',
    v20: '有(10+引入)',
    v23: '沿用',
    note: '报错信息字符串，自API10提供，卡片/原子化服务可用'
  },
  {
    field: 'error',
    v9: '无',
    v20: '有(20+新增)',
    v23: '沿用',
    note: 'BusinessError<void>，code+message，默认{code:-1,message:""}'
  },
  {
    field: 'downloadInfo',
    v9: '无',
    v20: '无',
    v23: '有(23+新增)',
    note: 'RequestDownloadInfo，默认null，仅网络图片下载失败时携带'
  }
];

// RequestDownloadInfo(=DownloadInfo 20+) 字段表（摘自cacheDownload文档）
const DOWNLOAD_INFO_ROWS: DlFieldRow[] = [
  { path: 'resource.size', since: '20+', type: 'number', desc: '预下载资源解压后大小(B)；正整数=下载成功，-1=下载失败' },
  { path: 'network.dnsServers', since: '20+', type: 'string[]', desc: '下载资源时使用的DNS服务器列表' },
  { path: 'network.ip', since: '23+', type: 'string(可选)', desc: '下载资源时url的IP地址；DNS解析失败时为undefined' },
  { path: 'performance.dnsTime', since: '20+', type: 'number', desc: '从启动到DNS解析完成耗时(ms)' },
  { path: 'performance.connectTime', since: '20+', type: 'number', desc: '从启动到TCP连接完成耗时(ms)' },
  { path: 'performance.tlsTime', since: '20+', type: 'number', desc: '从启动到TLS连接完成耗时(ms)' },
  { path: 'performance.firstSendTime', since: '20+', type: 'number', desc: '从启动到开始发送第一个字节耗时(ms)' },
  { path: 'performance.firstReceiveTime', since: '20+', type: 'number', desc: '从启动到接收第一个字节耗时(ms)' },
  { path: 'performance.totalTime', since: '20+', type: 'number', desc: '从启动到完成请求耗时(ms)' },
  { path: 'performance.redirectTime', since: '20+', type: 'number', desc: '从启动到完成所有重定向步骤耗时(ms)' }
];

// 非法URI触发队列：形态非法，引擎前置校验失败，不发真实请求
const TRIGGER_DEFS: TriggerDef[] = [
  {
    title: 'T1 scheme拼写错误',
    src: 'htp://typo-scheme.example.com/a.png',
    expect: '101000 unknown source type.',
    why: '"htp"非受支持scheme，源类型无法识别（数据加载/未知类型）'
  },
  {
    title: 'T2 缺少双斜杠的https',
    src: 'https:/single-slash.example.com/a.png',
    expect: '101000/102090 uri is invalid.',
    why: 'URI格式非法，前置解析失败，不进入网络请求阶段'
  },
  {
    title: 'T3 沙箱路径不存在',
    src: 'file:///data/storage/el2/base/haps/entry/files/v20_probe_missing.png',
    expect: '102051 read data failed.',
    why: 'file://沙箱文件不存在或不可读（数据加载/沙箱文件）'
  }
];

// 版本能力速查（卡片能力/原子化服务/模型约束，摘自文档各接口小节；复用DlFieldRow结构）
const CAPABILITY_ROWS: DlFieldRow[] = [
  { path: 'onError', since: '9+', type: '卡片9+/原子化11+', desc: 'SystemCapability.ArkUI.ArkUI.Full' },
  { path: 'message', since: '10+', type: '卡片10+/原子化11+', desc: '仅Stage模型' },
  { path: 'error', since: '20+', type: '卡片20+/原子化20+', desc: '仅Stage模型' },
  { path: 'downloadInfo', since: '23+', type: '卡片23+/原子化23+', desc: '仅Stage模型' },
  { path: 'onComplete', since: '9+', type: '卡片9+/原子化11+', desc: '绘制字段(10+)仅Stage模型' },
  { path: 'onFinish', since: '9+', type: '卡片9+/原子化11+', desc: '仅带动效SVG播放完成触发' }
];

@Entry
@Component
struct ImageErrorV20FieldsExample {
  // 捕获快照列表（最新在前）
  @State captures: V20Capture[] = [];
  // 每个触发场景的最新格式化摘要（与TRIGGER_DEFS下标对齐）
  @State triggerRows: string[] = new Array<string>(TRIGGER_DEFS.length).fill('等待onError回调…');
  // 触发轮次（变更reloadKey重放全部触发源）
  @State triggerRound: number = 0;
  @State reloadKey: string = 'v20_0';
  // DownloadInfo字段表展开状态
  @State showDlTable: boolean = true;

  /**
   * 记录一次 onError 的 20+/23+ 字段形态。
   */
  private record(title: string, index: number, e: ImageError): void {
    const cap: V20Capture = {
      scenario: title,
      hasErrorField: e.error !== undefined,
      errCode: e.error !== undefined ? e.error.code : -1,
      errMsg: e.error !== undefined ? e.error.message : '(运行环境<API20，无error字段)',
      downloadInfoText: e.downloadInfo !== undefined && e.downloadInfo !== null
        ? JSON.stringify(e.downloadInfo)
        : 'null(本页无真实网络下载失败，符合文档默认值语义)',
      message10: e.message !== undefined ? e.message : '(运行环境<API10，无message字段)',
      rawJson: JSON.stringify(e),
      timeText: new Date().toLocaleTimeString()
    };
    this.captures.unshift(cap);
    this.triggerRows[index] = 'error.code(20+) = ' + cap.errCode.toString() + '\n' +
      'error.message(20+) = ' + cap.errMsg + '\n' +
      'message(10+) = ' + cap.message10 + '\n' +
      'downloadInfo(23+) = ' +
      (cap.downloadInfoText.indexOf('null(') === 0 ? 'null' : cap.downloadInfoText);
    console.warn('ImageErrorV20Fields captured: ' + cap.rawJson);
  }

  private retrigger(): void {
    this.triggerRound += 1;
    this.reloadKey = 'v20_' + this.triggerRound.toString();
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('onError 20+/23+ 字段形态解读示例')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })
        // 版本演进总览卡
        Column({ space: 8 }) {
          Text('ImageError 三代字段形态总览').fontSize(16).fontColor(Color.Gray)
          Text('onError(callback: ImageErrorCallback)，' +
            'ImageErrorCallback = (error: ImageError) => void。').fontSize(12)
          Text('API9 : componentWidth/componentHeight 基础组件尺寸字段').fontSize(12)
          Text('API10: message 报错信息字符串').fontSize(12)
          Text('API20: error: BusinessError<void>，默认{ code : -1, message : "" }').fontSize(12)
          Text('API23: downloadInfo: RequestDownloadInfo，默认null').fontSize(12)
          Text('当前已捕获: ' + this.captures.length.toString() + ' 条').fontSize(14)
            .fontColor(this.captures.length > 0 ? Color.Blue : '#999999')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 三版差异对照表卡
        Column({ space: 8 }) {
          Text('9 / 20 / 23 三版参数差异对照').fontSize(16).fontColor(Color.Gray)
          Row({ space: 6 }) {
            Text('字段').fontSize(12).fontWeight(FontWeight.Bold).layoutWeight(6)
            Text('API9').fontSize(12).fontWeight(FontWeight.Bold).layoutWeight(3)
            Text('API20').fontSize(12).fontWeight(FontWeight.Bold).layoutWeight(3)
            Text('API23').fontSize(12).fontWeight(FontWeight.Bold).layoutWeight(3)
          }.width('100%')
          ForEach(VERSION_DIFF_ROWS, (row: VersionDiffRow, index: number) => {
            Column({ space: 2 }) {
              Row({ space: 6 }) {
                Text(row.field).fontSize(12).layoutWeight(6).maxLines(2)
                  .textOverflow({ overflow: TextOverflow.Ellipsis })
                Text(row.v9).fontSize(11).layoutWeight(3).fontColor('#999999')
                Text(row.v20).fontSize(11).layoutWeight(3)
                  .fontColor(row.v20 === '有(20+新增)' ? Color.Green : '#666666')
                Text(row.v23).fontSize(11).layoutWeight(3)
                  .fontColor(row.v23 === '有(23+新增)' ? Color.Green : '#666666')
              }.width('100%')
              Text(row.note).fontSize(11).fontColor('#AAAAAA')
            }
            .width('100%')
            .padding({ top: 3, bottom: 3 })
            .border({ width: { bottom: 1 }, color: '#E8E8E8' })
          }, (row: VersionDiffRow, index: number) => 'dif_' + index.toString())
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 触发卡：ForEach 渲染非法URI图源（不发真实请求）
        Column({ space: 8 }) {
          Text('非法URI字符串触发网络类错误路径（第' + this.triggerRound.toString() + '轮）')
            .fontSize(16).fontColor(Color.Gray)
          ForEach(TRIGGER_DEFS, (t: TriggerDef, index: number) => {
            Column({ space: 6 }) {
              Text(t.title).fontSize(13).fontWeight(FontWeight.Medium)
              Text('why: ' + t.why).fontSize(11).fontColor('#888888')
              Text('expect: ' + t.expect).fontSize(11).fontColor('#BB8800')
              Row({ space: 10 }) {
                Image(t.src, this.reloadKey)
                  .width(72)
                  .height(56)
                  .borderWidth(1)
                  .borderColor('#D0D0D0')
                  .onError((e: ImageError) => {
                    this.record(t.title, index, e);
                  })
                Text(this.triggerRows[index])
                  .fontSize(11)
                  .fontFamily('monospace')
                  .layoutWeight(1)
                  .maxLines(4)
                  .textOverflow({ overflow: TextOverflow.Ellipsis })
              }
              .width('100%')
            }
            .width('100%')
            .padding(10)
            .backgroundColor('#FFFFFF')
            .borderRadius(8)
            .borderWidth(1)
            .borderColor('#E4E4E4')
          }, (t: TriggerDef, index: number) => 'trg_' + index.toString())
          Row({ space: 10 }) {
            Button('重新触发一轮(变更reloadKey)').fontSize(13).onClick(() => {
              this.retrigger();
            })
            Button('清空捕获').fontSize(13).backgroundColor('#888888').onClick(() => {
              this.captures = [];
              this.triggerRows = new Array<string>(TRIGGER_DEFS.length).fill('等待onError回调…');
            })
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // BusinessError<20+> 形态卡
        Column({ space: 8 }) {
          Text('BusinessError<20+> 形态速查').fontSize(16).fontColor(Color.Gray)
          Text('type BusinessError<T = void> = import(\'../api/@ohos.base\').BusinessError<T>')
            .fontSize(11).fontFamily('monospace')
          Text('ImageError.error 即该类型实例：code为错误码、message为错误信息；' +
            '默认值 { code : -1, message : "" }。').fontSize(12)
          Text('网络文件相关错误码（文档错误信息表摘录）：').fontSize(12).fontColor('#666666')
          Text('102010 sync http task of uri cancelled. —— 同步网络请求被取消').fontSize(11)
          Text('102011 sync http task of uri failed. —— 同步加载失败(网络异常/地址不可达/无INTERNET权限)')
            .fontSize(11).maxLines(2).textOverflow({ overflow: TextOverflow.Ellipsis })
          Text('102012 async http task of uri cancelled. —— 异步请求被取消').fontSize(11)
          Text('102013 async http task of uri failed. —— 异步加载失败(网络异常/地址不可达/无INTERNET权限)')
            .fontSize(11).maxLines(2).textOverflow({ overflow: TextOverflow.Ellipsis })
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 图源形态-失败阶段映射卡
        Column({ space: 8 }) {
          Text('图源形态 × 失败阶段 × downloadInfo 携带关系').fontSize(16).fontColor(Color.Gray)
          Row({ space: 6 }) {
            Text('形态').fontSize(12).fontWeight(FontWeight.Bold).layoutWeight(5)
            Text('失败阶段').fontSize(12).fontWeight(FontWeight.Bold).layoutWeight(6)
            Text('错误码').fontSize(12).fontWeight(FontWeight.Bold).layoutWeight(4)
            Text('downloadInfo').fontSize(12).fontWeight(FontWeight.Bold).layoutWeight(5)
          }.width('100%')
          ForEach(URI_STAGE_ROWS, (row: UriStageRow, index: number) => {
            Row({ space: 6 }) {
              Text(row.form).fontSize(11).layoutWeight(5).maxLines(2)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
              Text(row.failStage).fontSize(11).layoutWeight(6).maxLines(2)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
              Text(row.docCode).fontSize(11).layoutWeight(4).fontColor(Color.Red)
              Text(row.dl).fontSize(11).layoutWeight(5)
                .fontColor(row.dl.indexOf('携带(') === 0 || row.dl.indexOf('23+') === 0
                  ? Color.Green : '#999999').maxLines(2)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
            }
            .width('100%')
            .padding({ top: 3, bottom: 3 })
            .border({ width: { bottom: 1 }, color: '#ECECEC' })
          }, (row: UriStageRow, index: number) => 'stg_' + index.toString())
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // RequestDownloadInfo<23+> 字段解读卡
        Column({ space: 8 }) {
          Text('RequestDownloadInfo<23+> 字段解读').fontSize(16).fontColor(Color.Gray)
          Text('type RequestDownloadInfo = import(\'../api/@ohos.request.cacheDownload\')' +
            '.default.DownloadInfo')
            .fontSize(11).fontFamily('monospace')
          Button(this.showDlTable ? '收起字段表' : '展开字段表').fontSize(13).onClick(() => {
            this.showDlTable = !this.showDlTable;
          })
          if (this.showDlTable) {
            ForEach(DOWNLOAD_INFO_ROWS, (row: DlFieldRow, index: number) => {
              Column({ space: 2 }) {
                Row({ space: 6 }) {
                  Text(row.path).fontSize(12).fontWeight(FontWeight.Medium).layoutWeight(6)
                  Text(row.since).fontSize(11).fontColor(Color.Blue).width(36)
                  Text(row.type).fontSize(11).fontColor('#666666').layoutWeight(4)
                }.width('100%')
                Text(row.desc).fontSize(11).fontColor('#999999')
              }
              .width('100%')
              .padding({ top: 3, bottom: 3 })
              .border({ width: { bottom: 1 }, color: '#ECECEC' })
            }, (row: DlFieldRow, index: number) => 'dlf_' + index.toString())
          }
          Text('说明：downloadInfo 仅在"图片来源为网络图片且下载失败"时携带；' +
            '本页刻意不发起真实请求，捕获值预期为 null。' +
            '真实网络图加载需申请 ohos.permission.INTERNET，可对照文档示例29观察非空形态。')
            .fontSize(11).fontColor('#888888')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 捕获明细卡
        Column({ space: 8 }) {
          Text('捕获明细（最新在前）').fontSize(16).fontColor(Color.Gray)
          ForEach(this.captures, (cap: V20Capture, index: number) => {
            Column({ space: 3 }) {
              Row({ space: 8 }) {
                Text(cap.scenario).fontSize(12).fontWeight(FontWeight.Medium).layoutWeight(1)
                Text('code=' + cap.errCode.toString()).fontSize(12).fontColor(Color.Red)
                Text(cap.timeText).fontSize(11).fontColor('#999999')
              }.width('100%')
              Text('message(10+) = ' + cap.message10).fontSize(11).maxLines(2)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
              Text('error(20+) = ' + (cap.hasErrorField ? '存在' : '缺失') +
                '，message = ' + cap.errMsg).fontSize(11).maxLines(2)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
              Text('downloadInfo(23+) = ' + cap.downloadInfoText).fontSize(11)
                .fontColor('#6A1B9A').maxLines(2)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
              Text('raw = ' + cap.rawJson).fontSize(10).fontFamily('monospace')
                .fontColor('#888888').maxLines(3)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
            }
            .width('100%')
            .padding(8)
            .backgroundColor('#FFFFFF')
            .borderRadius(6)
          }, (cap: V20Capture, index: number) => 'cap_' + cap.timeText + '_' + index.toString())
          if (this.captures.length === 0) {
            Text('（空）等待上方非法URI触发').fontSize(12).fontColor('#BBBBBB')
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 版本能力速查卡
        Column({ space: 8 }) {
          Text('版本能力速查（卡片/原子化服务/模型约束）').fontSize(16).fontColor(Color.Gray)
          ForEach(CAPABILITY_ROWS, (row: DlFieldRow, index: number) => {
            Column({ space: 2 }) {
              Row({ space: 6 }) {
                Text(row.path).fontSize(12).fontWeight(FontWeight.Medium).layoutWeight(5)
                Text(row.since).fontSize(11).fontColor(Color.Blue).width(40)
                Text(row.type).fontSize(11).layoutWeight(6).fontColor('#666666')
              }.width('100%')
              Text(row.desc).fontSize(11).fontColor('#999999')
            }
            .width('100%')
            .padding({ top: 3, bottom: 3 })
            .border({ width: { bottom: 1 }, color: '#ECECEC' })
          }, (row: DlFieldRow, index: number) => 'cap_' + index.toString())
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        // 约束脚注
        Text('约束：error字段自API20、downloadInfo字段自API23（卡片能力/原子化服务同步放开）；' +
          '两者仅Stage模型可用；低版本运行环境读取不到高版本字段时本页已按"缺失"标注，' +
          '未引用任何文档不存在的字段名。')
          .fontSize(12).fontColor(Color.Blue).margin({ bottom: 20 })
      }.width('100%').padding({ left: 20, right: 20 })
    }.width('100%').height('100%').backgroundColor(Color.White)
  }
}
