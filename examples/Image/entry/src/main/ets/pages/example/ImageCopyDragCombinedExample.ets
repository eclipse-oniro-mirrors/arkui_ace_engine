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
 * Image copyOption × draggable 组合场景示例 (均 API9+)
 *
 * 与既有页面的差异定位:
 * - ImageCopyOptionsExample 只演示 copyOptions(复制范围)单选, ImageDraggableExample/ImageExample105
 *   只演示 draggable 开关; 本页首次把 copyOption 与 draggable 组成 3×2 组合矩阵,
 *   并在同一页面内完成"拖出源 + 落点区 + 结果回传"的闭环.
 * - onDragStart 返回 DragItemInfo(自定义拖拽预览图 + extraInfo), 而非仅返回预览图,
 *   该写法以 OpenHarmony 官方文档 onDragStart 回调签名 CustomBuilder | DragItemInfo 为准.
 *
 * 文档依据 (ts-basic-components-image.md):
 * - copyOption(value: CopyOptions): 非 CopyOptions.None 时支持长按/右击/CTRL+C 复制; SVG图片不支持复制; 默认 None.
 * - CopyOptions: None(0)/InApp(1)/LocalDevice(2)/CROSS_DEVICE(3, API12废弃).
 * - draggable(value: boolean): true 时组件可拖拽且绑定长按手势不生效; API10+ 默认 true.
 * - ts-universal-events-drag-drop.md: 长按超500ms且移动>10vp 触发 onDragStart;
 *   onDrop 中 setResult(DragResult), onDragEnd 中 getResult() 回传结果.
 */
import { image } from '@kit.ImageKit';
import { BusinessError } from '@kit.BasicServicesKit';

interface CopyDragComboCell {
  id: number;
  copyOption: CopyOptions;
  draggableOn: boolean;
  label: string;
  note: string;
}

interface DragLogEntry {
  id: number;
  time: string;
  message: string;
}

const COMBO_CELLS: CopyDragComboCell[] = [
  { id: 0, copyOption: CopyOptions.None, draggableOn: true, label: 'None × draggable(true)', note: '可拖拽发起, 但长按复制菜单不可用' },
  { id: 1, copyOption: CopyOptions.None, draggableOn: false, label: 'None × draggable(false)', note: '既不可复制也不可拖拽, 完全封闭' },
  { id: 2, copyOption: CopyOptions.InApp, draggableOn: true, label: 'InApp × draggable(true)', note: '可拖拽; 复制内容仅应用内粘贴' },
  { id: 3, copyOption: CopyOptions.InApp, draggableOn: false, label: 'InApp × draggable(false)', note: '可复制不可拖拽, 长按手势归应用' },
  { id: 4, copyOption: CopyOptions.LocalDevice, draggableOn: true, label: 'LocalDevice × draggable(true)', note: '可拖拽; 复制后本设备所有应用可粘贴' },
  { id: 5, copyOption: CopyOptions.LocalDevice, draggableOn: false, label: 'LocalDevice × draggable(false)', note: '仅保留复制能力, 关闭拖出' }
];

const MAX_LOG_SIZE: number = 10;

@Entry
@Component
struct ImageCopyDragCombinedExample {
  @State sourcePixelMap: image.PixelMap | undefined = undefined;
  @State mainCopyOption: CopyOptions = CopyOptions.InApp;
  @State mainDraggable: boolean = true;
  @State dragStartCount: number = 0;
  @State dropCount: number = 0;
  @State lastDragResultText: string = '尚无拖拽结果';
  @State dropZoneActive: boolean = false;
  @State acceptZoneActive: boolean = false;
  @State rejectZoneActive: boolean = false;
  @State acceptCount: number = 0;
  @State rejectCount: number = 0;
  @State logEntries: DragLogEntry[] = [];
  private logSeq: number = 0;

  async aboutToAppear(): Promise<void> {
    this.sourcePixelMap = await this.createPatternPixelMap(160, 160);
    this.appendLog(this.sourcePixelMap !== undefined
      ? 'PixelMap图源创建成功(位图源, 复制不受SVG限制)'
      : 'PixelMap图源创建失败');
  }

  private async createPatternPixelMap(width: number, height: number): Promise<image.PixelMap | undefined> {
    const buffer: ArrayBuffer = new ArrayBuffer(width * height * 4);
    const bytes: Uint8Array = new Uint8Array(buffer);
    for (let y = 0; y < height; y++) {
      for (let x = 0; x < width; x++) {
        const offset: number = (y * width + x) * 4;
        bytes[offset] = (x * 255) / width;
        bytes[offset + 1] = (y * 255) / height;
        bytes[offset + 2] = 128;
        bytes[offset + 3] = 255;
      }
    }
    const opts: image.InitializationOptions = {
      size: { height: height, width: width },
      srcPixelFormat: image.PixelMapFormat.RGBA_8888,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      editable: false
    };
    try {
      return await image.createPixelMap(buffer, opts);
    } catch (error) {
      const err: BusinessError = error as BusinessError;
      console.error('createPatternPixelMap failed code=' + err.code);
      return undefined;
    }
  }

  private appendLog(message: string): void {
    this.logSeq += 1;
    const entry: DragLogEntry = {
      id: this.logSeq,
      time: new Date().toLocaleTimeString(),
      message: message
    };
    const next: DragLogEntry[] = [entry].concat(this.logEntries);
    if (next.length > MAX_LOG_SIZE) {
      next.pop();
    }
    this.logEntries = next;
  }

  private copyOptionDesc(option: CopyOptions): string {
    if (option === CopyOptions.None) {
      return 'None: 不支持复制(长按/右击/CTRL+C均无效)';
    }
    if (option === CopyOptions.InApp) {
      return 'InApp: 仅当前应用内复制粘贴';
    }
    if (option === CopyOptions.LocalDevice) {
      return 'LocalDevice: 本设备所有应用可粘贴';
    }
    return 'CROSS_DEVICE: API12已废弃, 本页不使用';
  }

  private resultText(result: DragResult): string {
    if (result === DragResult.DRAG_SUCCESSFUL) {
      return 'DRAG_SUCCESSFUL(拖拽成功)';
    }
    if (result === DragResult.DRAG_FAILED) {
      return 'DRAG_FAILED(拖拽失败)';
    }
    if (result === DragResult.DRAG_CANCELED) {
      return 'DRAG_CANCELED(拖拽取消)';
    }
    return '其他结果码: ' + result;
  }

  build() {
    Scroll() {
      Column({ space: 14 }) {
        Text('copyOption × draggable 组合拖拽示例(API9+)')
          .fontSize(24)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 6 })

        Column({ space: 8 }) {
          Text('主拖拽源配置')
            .fontSize(16)
            .fontColor(Color.Gray)
          Row({ space: 8 }) {
            Button('None')
              .backgroundColor(this.mainCopyOption === CopyOptions.None ? 0x007DFF : 0xE0E0E0)
              .fontColor(this.mainCopyOption === CopyOptions.None ? Color.White : 0x333333)
              .onClick(() => {
                this.mainCopyOption = CopyOptions.None;
                this.appendLog('copyOption 切换为 None');
              })
            Button('InApp')
              .backgroundColor(this.mainCopyOption === CopyOptions.InApp ? 0x007DFF : 0xE0E0E0)
              .fontColor(this.mainCopyOption === CopyOptions.InApp ? Color.White : 0x333333)
              .onClick(() => {
                this.mainCopyOption = CopyOptions.InApp;
                this.appendLog('copyOption 切换为 InApp');
              })
            Button('LocalDevice')
              .backgroundColor(this.mainCopyOption === CopyOptions.LocalDevice ? 0x007DFF : 0xE0E0E0)
              .fontColor(this.mainCopyOption === CopyOptions.LocalDevice ? Color.White : 0x333333)
              .onClick(() => {
                this.mainCopyOption = CopyOptions.LocalDevice;
                this.appendLog('copyOption 切换为 LocalDevice');
              })
          }
          Row({ space: 8 }) {
            Button(this.mainDraggable ? 'draggable(true)' : 'draggable(false)')
              .backgroundColor(this.mainDraggable ? 0x007DFF : 0xE0E0E0)
              .fontColor(this.mainDraggable ? Color.White : 0x333333)
              .onClick(() => {
                this.mainDraggable = !this.mainDraggable;
                this.appendLog('draggable 切换为 ' + (this.mainDraggable ? 'true' : 'false'));
              })
            Text('当前: ' + this.copyOptionDesc(this.mainCopyOption))
              .fontSize(12)
              .fontColor(Color.Gray)
              .layoutWeight(1)
          }
          .width('100%')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Column({ space: 10 }) {
          Text('主拖拽源(onDragStart 返回 DragItemInfo)')
            .fontSize(16)
            .fontColor(Color.Gray)
          Row({ space: 16 }) {
            if (this.sourcePixelMap !== undefined) {
              Image(this.sourcePixelMap)
                .width(110)
                .height(110)
                .borderRadius(8)
                .copyOption(this.mainCopyOption)
                .draggable(this.mainDraggable)
                .onDragStart((event: DragEvent, extraParams?: string): DragItemInfo => {
                  this.dragStartCount += 1;
                  this.appendLog('拖拽发起 #' + this.dragStartCount +
                    ' (长按≥500ms且移动>10vp触发)');
                  const info: DragItemInfo = {
                    pixelMap: this.sourcePixelMap,
                    extraInfo: 'copyOption=' + this.mainCopyOption
                  };
                  return info;
                })
                .onDragEnd((event: DragEvent) => {
                  const result: DragResult = event.getResult();
                  this.lastDragResultText = this.resultText(result);
                  this.appendLog('拖拽结束, getResult()=' + this.resultText(result));
                })
            }
            Column({ space: 6 }) {
              Text('拖出次数: ' + this.dragStartCount)
                .fontSize(14)
              Text('最近结果: ' + this.lastDragResultText)
                .fontSize(13)
                .fontColor(Color.Gray)
              Text(this.mainDraggable
                ? 'draggable=true 时组件绑定长按手势不生效'
                : 'draggable=false 后拖拽类事件不再触发')
                .fontSize(12)
                .fontColor(Color.Gray)
            }
            .alignItems(HorizontalAlign.Start)
            .layoutWeight(1)
          }
          .width('100%')
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Column({ space: 10 }) {
          Text('落点区域(onDrop 中 setResult 回传拖出方)')
            .fontSize(16)
            .fontColor(Color.Gray)
          Column({ space: 6 }) {
            Text(this.dropZoneActive ? '拖拽进入, 松手可放置' : '把上方图片拖到这里')
              .fontSize(14)
              .fontColor(this.dropZoneActive ? Color.Green : Color.Gray)
            Text('放置次数: ' + this.dropCount)
              .fontSize(13)
          }
          .width('100%')
          .height(130)
          .justifyContent(FlexAlign.Center)
          .backgroundColor(this.dropZoneActive ? '#E8F5E9' : '#FAFAFA')
          .borderRadius(10)
          .border({
            width: 2,
            style: this.dropZoneActive ? BorderStyle.Solid : BorderStyle.Dashed,
            color: this.dropZoneActive ? Color.Green : '#CCCCCC'
          })
          .onDragEnter((event?: DragEvent, extraParams?: string) => {
            this.dropZoneActive = true;
          })
          .onDragLeave((event?: DragEvent, extraParams?: string) => {
            this.dropZoneActive = false;
          })
          .onDrop((event?: DragEvent, extraParams?: string) => {
            this.dropZoneActive = false;
            this.dropCount += 1;
            if (event !== undefined) {
              event.setResult(DragResult.DRAG_SUCCESSFUL);
            }
            this.appendLog('onDrop: setResult(DRAG_SUCCESSFUL)');
          })
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Text('copyOption × draggable 组合矩阵(6格)')
          .fontSize(16)
          .fontWeight(FontWeight.Medium)

        ForEach(COMBO_CELLS, (cell: CopyDragComboCell) => {
          this.ComboCellView(cell)
        }, (cell: CopyDragComboCell) => cell.id.toString())

        Column({ space: 10 }) {
          Text('双落点对照: 接收成功 vs 拒绝落入')
            .fontSize(16)
            .fontColor(Color.Gray)
          Row({ space: 12 }) {
            Column({ space: 4 }) {
              Text('接收区')
                .fontSize(14)
                .fontWeight(FontWeight.Medium)
              Text(this.acceptZoneActive ? 'onDragEnter: DROP_ENABLED' : '等待拖入')
                .fontSize(12)
                .fontColor(this.acceptZoneActive ? Color.Green : Color.Gray)
              Text('成功次数: ' + this.acceptCount)
                .fontSize(12)
            }
            .layoutWeight(1)
            .height(110)
            .justifyContent(FlexAlign.Center)
            .backgroundColor(this.acceptZoneActive ? '#E8F5E9' : '#FAFAFA')
            .borderRadius(10)
            .border({
              width: 2,
              style: this.acceptZoneActive ? BorderStyle.Solid : BorderStyle.Dashed,
              color: this.acceptZoneActive ? Color.Green : '#CCCCCC'
            })
            .onDragEnter((event?: DragEvent, extraParams?: string) => {
              this.acceptZoneActive = true;
              if (event !== undefined) {
                event.setResult(DragResult.DROP_ENABLED);
              }
            })
            .onDragLeave((event?: DragEvent, extraParams?: string) => {
              this.acceptZoneActive = false;
            })
            .onDrop((event?: DragEvent, extraParams?: string) => {
              this.acceptZoneActive = false;
              this.acceptCount += 1;
              if (event !== undefined) {
                event.setResult(DragResult.DRAG_SUCCESSFUL);
              }
              this.appendLog('接收区 onDrop(DRAG_SUCCESSFUL)');
            })
            Column({ space: 4 }) {
              Text('拒绝区')
                .fontSize(14)
                .fontWeight(FontWeight.Medium)
              Text(this.rejectZoneActive ? 'onDragEnter: DROP_DISABLED' : '永不接收')
                .fontSize(12)
                .fontColor(this.rejectZoneActive ? Color.Red : Color.Gray)
              Text('拒绝次数: ' + this.rejectCount)
                .fontSize(12)
            }
            .layoutWeight(1)
            .height(110)
            .justifyContent(FlexAlign.Center)
            .backgroundColor(this.rejectZoneActive ? '#FFEBEE' : '#FAFAFA')
            .borderRadius(10)
            .border({
              width: 2,
              style: this.rejectZoneActive ? BorderStyle.Solid : BorderStyle.Dashed,
              color: this.rejectZoneActive ? Color.Red : '#CCCCCC'
            })
            .onDragEnter((event?: DragEvent, extraParams?: string) => {
              this.rejectZoneActive = true;
              if (event !== undefined) {
                event.setResult(DragResult.DROP_DISABLED);
              }
            })
            .onDragLeave((event?: DragEvent, extraParams?: string) => {
              this.rejectZoneActive = false;
            })
            .onDrop((event?: DragEvent, extraParams?: string) => {
              this.rejectZoneActive = false;
              this.rejectCount += 1;
              if (event !== undefined) {
                event.setResult(DragResult.DRAG_FAILED);
              }
              this.appendLog('拒绝区 onDrop(DRAG_FAILED)');
            })
          }
          .width('100%')
          Text('拖出方可在 onDragEnd 中通过 getResult() 区分落点反馈的结果码')
            .fontSize(12)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)

        Column({ space: 6 }) {
          Text('事件日志(最近' + MAX_LOG_SIZE + '条)')
            .fontSize(16)
            .fontColor(Color.Gray)
          ForEach(this.logEntries, (entry: DragLogEntry) => {
            Text(entry.time + ' · ' + entry.message)
              .fontSize(12)
              .fontColor(0x666666)
          }, (entry: DragLogEntry) => entry.id.toString())
          if (this.logEntries.length === 0) {
            Text('暂无事件')
              .fontSize(12)
              .fontColor(Color.Gray)
          }
        }
        .width('100%')
        .padding(15)
        .backgroundColor('#F5F5F5')
        .borderRadius(10)
        .alignItems(HorizontalAlign.Start)

        Text('脚注: 拖拽由长按≥500ms并移动>10vp发起(鼠标左键移动>1vp); copyOption 只影响复制能力, 拖出数据内容需配合 DragEvent.setData 的 UDMF 记录, 该数据通道依赖 ArkData 能力, 本页以事件链路与自定义预览图演示为主')
          .fontSize(12)
          .fontColor(Color.Blue)
        Text('SVG图片不支持复制, 故本页主图源为运行时 PixelMap 位图; CROSS_DEVICE 自 API12 废弃, 矩阵仅取 None/InApp/LocalDevice')
          .fontSize(12)
          .fontColor(Color.Blue)
          .margin({ bottom: 20 })
      }
      .width('100%')
      .padding({ left: 20, right: 20 })
    }
    .width('100%')
    .height('100%')
    .backgroundColor(Color.White)
  }

  @Builder
  ComboCellView(cell: CopyDragComboCell) {
    Column({ space: 6 }) {
      Text(cell.label)
        .fontSize(14)
        .fontWeight(FontWeight.Medium)
      Text(cell.note)
        .fontSize(12)
        .fontColor(Color.Gray)
      Row({ space: 10 }) {
        if (this.sourcePixelMap !== undefined) {
          Image(this.sourcePixelMap)
            .width(48)
            .height(48)
            .borderRadius(6)
            .copyOption(cell.copyOption)
            .draggable(cell.draggableOn)
            .onDragStart((event: DragEvent, extraParams?: string): DragItemInfo => {
              const info: DragItemInfo = {
                extraInfo: 'cell#' + cell.id + ' ' + cell.label
              };
              this.appendLog('组合格拖出: ' + cell.label);
              return info;
            })
        }
        Text(cell.draggableOn
          ? (cell.copyOption === CopyOptions.None ? '仅拖拽' : '拖拽+复制')
          : (cell.copyOption === CopyOptions.None ? '均关闭' : '仅复制'))
          .fontSize(12)
          .fontColor(cell.draggableOn ? 0x007DFF : Color.Gray)
      }
      .width('100%')
    }
    .width('100%')
    .padding(15)
    .backgroundColor('#F5F5F5')
    .borderRadius(10)
    .alignItems(HorizontalAlign.Start)
  }
}
