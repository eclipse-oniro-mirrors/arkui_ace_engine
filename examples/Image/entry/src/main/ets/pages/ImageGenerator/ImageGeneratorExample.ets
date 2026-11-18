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
import { common2D } from '@kit.ArkGraphics2D';
import { componentUtils } from '@kit.ArkUI';
import { BusinessError } from '@kit.BasicServicesKit';
import common from '@ohos.app.ability.common';
import { MockImageGenerationModel, MockTextGenerationModel, MockLiveViewInfo,
  StylePreset, SizePreset, StreamLogEntry, ScenarioPreset } from './ImageGeneratorMockModels';

let TAG: string = '[ImageGeneratorExample]';

@Entry
@Component
struct ImageGeneratorExample {
  // ============ 弹窗 / 节点图谱状态 ============
  @State dialogStatus: string = '未打开';
  @State nodeGraphStatus: string = '未打开';
  @State smartCanvasStatus: string = '未启动';
  @State isDialogBusy: boolean = false;
  @State isNodeGraphBusy: boolean = false;

  // ============ 模型接入开关 ============
  @State useImageModel: boolean = true;
  @State useTextModel: boolean = true;
  @State useLiveView: boolean = false;
  @State minimizeDuringGeneration: boolean = false;

  // ============ 输入图片项 (ImageItem) 配置 ============
  @State imageUrl: string = '/data/storage/el2/database/files/sample.png';
  @State imageRectX: number = 0;
  @State imageRectY: number = 0;
  @State imageRectW: number = 240;
  @State imageRectH: number = 240;
  @State imageRotation: number = 0;
  @State imageZIndex: number = 1;
  @State isHandwrite: boolean = false;
  @State usePositionImage: boolean = false;
  @State useSelectPath: boolean = false;

  // ============ 生成任务参数 ============
  @State prompt: string = '夕阳下的海边风景, 水彩风格';
  @State selectedStyleIndex: number = 0;
  @State selectedSizeIndex: number = 0;
  @State imageCount: number = 2;

  // ============ 弹窗初始内容 ============
  @State initContent: string = '帮我生成一张海边日落的图片';

  // ============ 自定义结果页图标 ============
  @State enableCustomIcons: boolean = false;
  @State customIconClickLog: string = '';

  // ============ 自定义导入图标 ============
  @State enableCustomImport: boolean = false;
  @State customImportLog: string = '';

  // ============ onAreaDidChange ============
  @State enableAreaCallback: boolean = false;
  @State areaLog: string = '';

  // ============ 流式结果模拟 ============
  @State streamLog: StreamLogEntry[] = [];
  @State streaming: boolean = false;
  @State partialCount: number = 0;
  @State completedCount: number = 0;
  @State failCount: number = 0;

  // ============ 生成结果展示 ============
  @State lastResults: imageGeneration.GeneratorResult[] = [];
  @State resultSummary: string = '尚无生成结果';

  // ============ 投诉菜单 ============
  @State complainLog: string = '';

  // ============ 模型样式列表 ============
  @State modelStyles: imageGeneration.ImageStyle[] = [];

  // ============ 全局操作日志 ============
  @State operationLog: string = '';

  // ============ 多图列表编辑 ============
  @State multiImageUrls: string[] = [
    '/data/storage/el2/database/files/img_a.png',
    '/data/storage/el2/database/files/img_b.png'
  ];
  @State multiImageZIndex: number[] = [1, 2];
  @State multiImageHandwrite: boolean[] = [false, true];

  // ============ 场景预设 ============
  @State currentScenarioIndex: number = 0;

  // ============ 会话管理 ============
  @State activeSessionId: number = -1;
  @State sessionHistory: string[] = [];

  // ============ 预设数据 ============
  private stylePresets: StylePreset[] = [
    { name: '水彩', desc: '柔和水彩笔触' },
    { name: '涂鸦', desc: '随性涂鸦风格' },
    { name: '油画', desc: '厚重油画质感' },
    { name: '国风', desc: '中国传统画风' },
    { name: '赛博朋克', desc: '霓虹未来感' }
  ];

  private sizePresets: SizePreset[] = [
    { label: '512 x 512 (方形)', size: { width: 512, height: 512 } },
    { label: '768 x 512 (横图)', size: { width: 768, height: 512 } },
    { label: '512 x 768 (竖图)', size: { width: 512, height: 768 } },
    { label: '1024 x 1024 (高清)', size: { width: 1024, height: 1024 } },
    { label: '256 x 256 (缩略)', size: { width: 256, height: 256 } }
  ];

  // 场景预设: 一键配置常见生成场景
  private scenarios: ScenarioPreset[] = [
    {
      name: '文生图',
      desc: '纯文字描述生成图片, 无输入图',
      prompt: '梦幻森林中的发光蘑菇, 水彩风格',
      styleIdx: 0,
      sizeIdx: 0,
      count: 3
    },
    {
      name: '图生图',
      desc: '基于参考图变换风格',
      prompt: '将图片转为赛博朋克霓虹风格',
      styleIdx: 4,
      sizeIdx: 3,
      count: 2
    },
    {
      name: '手绘线稿上色',
      desc: '对手绘线稿进行智能上色',
      prompt: '为手绘线稿填充鲜艳色彩',
      styleIdx: 1,
      sizeIdx: 0,
      count: 1
    },
    {
      name: '多图融合',
      desc: '多张参考图融合生成',
      prompt: '融合多张图片的人物与背景',
      styleIdx: 2,
      sizeIdx: 1,
      count: 2
    },
    {
      name: '贴纸生成',
      desc: '生成透明背景贴纸',
      prompt: '可爱卡通猫咪贴纸',
      styleIdx: 1,
      sizeIdx: 4,
      count: 4
    }
  ];

  // Mock 模型实例
  private mockImageModel: MockImageGenerationModel = new MockImageGenerationModel();
  private mockTextModel: MockTextGenerationModel = new MockTextGenerationModel();
  private mockLiveView: MockLiveViewInfo = new MockLiveViewInfo();

  aboutToAppear(): void {
    this.refreshModelStyles();
    this.appendLog('页面初始化完成, Mock 模型已就绪');
  }

  // ============ 辅助方法 ============
  private appendLog(msg: string): void {
    const now: Date = new Date();
    const stamp: string =
      `${now.getHours().toString().padStart(2, '0')}:` +
      `${now.getMinutes().toString().padStart(2, '0')}:` +
      `${now.getSeconds().toString().padStart(2, '0')}`;
    this.operationLog = `[${stamp}] ${msg}\n` + this.operationLog;
  }

  private appendStreamLog(type: string, detail: string): void {
    const now: Date = new Date();
    const stamp: string =
      `${now.getHours().toString().padStart(2, '0')}:` +
      `${now.getMinutes().toString().padStart(2, '0')}:` +
      `${now.getSeconds().toString().padStart(2, '0')}`;
    this.streamLog = [{ time: stamp, type: type, detail: detail }].concat(this.streamLog);
  }

  private refreshModelStyles(): void {
    if (this.useImageModel) {
      this.modelStyles = this.mockImageModel.getModelSupportStyles();
    } else {
      this.modelStyles = [];
    }
  }
  // 构造输入图片项 ImageItem 数组
  private buildImageItems(): imageGeneration.ImageItem[] {
    const rect: common2D.Rect = {
      left: this.imageRectX,
      top: this.imageRectY,
      width: this.imageRectW,
      height: this.imageRectH
    };
    const rotation: componentUtils.Rotation2D = {
      x: 0,
      y: 0,
      angle: this.imageRotation
    };
    const item: imageGeneration.ImageItem = {
      url: this.imageUrl,
      rect: rect,
      rotation: rotation,
      zIndex: this.imageZIndex,
      isHandwrite: this.isHandwrite
    };
    return [item];
  }
  // 构造 lasso 套索选择路径 (selectPath)
  private buildSelectPath(): common2D.Point[] {
    const path: common2D.Point[] = [];
    path.push({ x: 50, y: 50 });
    path.push({ x: 200, y: 50 });
    path.push({ x: 200, y: 200 });
    path.push({ x: 50, y: 200 });
    return path;
  }
  // 构造结果页自定义图标数组
  private buildCustomIcons(): imageGeneration.GeneratorResultPageIcon[] {
    const icons: imageGeneration.GeneratorResultPageIcon[] = [];
    icons.push({
      image: $r('app.media.heart'),
      text: '收藏',
      callback: (result: imageGeneration.GeneratorResult) => {
        this.customIconClickLog = `点击"收藏", url=${result.url ?? 'N/A'}, ` +
          `耗时=${result.statistic.generationTime}s`;
        this.appendLog('自定义图标"收藏"被点击');
      }
    });
    icons.push({
      image: $r('app.media.star'),
      text: '分享',
      callback: (result: imageGeneration.GeneratorResult) => {
        this.customIconClickLog = `点击"分享", 风格=${result.statistic.style}, ` +
          `图片数=${result.statistic.imageCount}`;
        this.appendLog('自定义图标"分享"被点击');
      }
    });
    icons.push({
      image: $r('app.media.icon'),
      text: '设为壁纸',
      callback: (result: imageGeneration.GeneratorResult) => {
        this.customIconClickLog = `点击"设为壁纸", 贴纸=${result.statistic.isSticker}`;
        this.appendLog('自定义图标"设为壁纸"被点击');
      }
    });
    return icons;
  }

  // 构造自定义导入图标
  private buildCustomImportIcon(): imageGeneration.CustomImportIcon {
    return {
      image: $r('app.media.svg'),
      text: '从相册导入',
      callback: (): Promise<imageGeneration.CustomImportResult> => {
        this.customImportLog = '自定义导入回调被触发, 正在模拟导入...';
        return new Promise<imageGeneration.CustomImportResult>((resolve) => {
          setTimeout(() => {
            const importResult: imageGeneration.CustomImportResult = {
              images: [{
                url: '/data/storage/el2/database/files/imported.png',
                zIndex: 1,
                isHandwrite: false
              }],
              content: '导入的参考图与描述文本'
            };
            this.customImportLog = '自定义导入完成, 已注入 1 张图片 + 描述文本';
            this.appendLog('CustomImportIcon 回调返回结果');
            resolve(importResult);
          }, 800);
        });
      }
    };
  }

  // 构造 GeneratorDialogOptions
  private buildDialogOptions(): imageGeneration.GeneratorDialogOptions {
    const options: imageGeneration.GeneratorDialogOptions = {};

    options.images = this.buildImageItems();
    options.content = this.initContent;

    if (this.enableCustomIcons) {
      options.customIcons = this.buildCustomIcons();
    }
    if (this.useImageModel) {
      options.imageGenerationModel = this.mockImageModel;
    }
    if (this.useTextModel) {
      options.textGenerationModel = this.mockTextModel;
    }
    if (this.enableAreaCallback) {
      options.onAreaDidChange = (rect: common2D.Rect) => {
        this.areaLog = `弹窗区域变更: left=${rect.left}, top=${rect.top}, ` +
          `width=${rect.width}, height=${rect.height}`;
        this.appendLog('onAreaDidChange 触发');
      };
    }
    if (this.enableCustomImport) {
      options.customImportIcon = this.buildCustomImportIcon();
    }
    if (this.useLiveView) {
      options.liveViewInfo = this.mockLiveView;
    }
    options.minimizeDuringGeneration = this.minimizeDuringGeneration;

    return options;
  }

  // 构造 GeneratorNodeGraphOptions
  private buildNodeGraphOptions(): imageGeneration.GeneratorNodeGraphOptions {
    const options: imageGeneration.GeneratorNodeGraphOptions = {};
    if (this.enableCustomIcons) {
      options.customIcons = this.buildCustomIcons();
    }
    if (this.useImageModel) {
      options.imageGenerationModel = this.mockImageModel;
    }
    if (this.useTextModel) {
      options.textGenerationModel = this.mockTextModel;
    }
    if (this.enableCustomImport) {
      options.customImportIcon = this.buildCustomImportIcon();
    }
    if (this.useLiveView) {
      options.liveViewInfo = this.mockLiveView;
    }
    return options;
  }

  // ============ 弹窗操作 ============
  private async doShowDialog(): Promise<void> {
    if (this.isDialogBusy) {
      this.appendLog('弹窗操作进行中, 请稍候');
      return;
    }
    this.isDialogBusy = true;
    this.dialogStatus = '正在打开...';
    this.appendLog('调用 showGeneratorDialog');
    try {
      const uiContext = this.getUIContext();
      const opts = this.buildDialogOptions();
      const results: imageGeneration.GeneratorResult[] =
        await imageGeneration.showGeneratorDialog(uiContext, opts);
      this.lastResults = results;
      if (results.length > 0) {
        const first: imageGeneration.GeneratorResult = results[0];
        this.resultSummary = `共返回 ${results.length} 张结果, 首张 url=${first.url ?? 'N/A'}, ` +
          `风格=${first.statistic.style}, 耗时=${first.statistic.generationTime}s`;
      } else {
        this.resultSummary = '弹窗关闭, 未返回结果';
      }
      this.dialogStatus = '已关闭(返回结果)';
      this.appendLog(`showGeneratorDialog 完成, 结果数=${results.length}`);
    } catch (err) {
      const e = err as BusinessError;
      this.dialogStatus = `打开失败: ${e.code} - ${e.message}`;
      this.appendLog(`showGeneratorDialog 异常: ${e.code} ${e.message}`);
    } finally {
      this.isDialogBusy = false;
    }
  }

  private async doHideDialog(): Promise<void> {
    this.appendLog('调用 hideGeneratorDialog');
    try {
      await imageGeneration.hideGeneratorDialog(this.getUIContext());
      this.dialogStatus = '已隐藏';
      this.appendLog('hideGeneratorDialog 完成');
    } catch (err) {
      const e = err as BusinessError;
      this.appendLog(`hideGeneratorDialog 异常: ${e.code} ${e.message}`);
    }
  }

  private async doRestoreDialog(): Promise<void> {
    this.appendLog('调用 restoreGeneratorDialog');
    try {
      await imageGeneration.restoreGeneratorDialog(this.getUIContext());
      this.dialogStatus = '已恢复';
      this.appendLog('restoreGeneratorDialog 完成');
    } catch (err) {
      const e = err as BusinessError;
      this.appendLog(`restoreGeneratorDialog 异常: ${e.code} ${e.message}`);
    }
  }

  private async doCloseDialog(): Promise<void> {
    this.appendLog('调用 closeGeneratorDialog');
    try {
      await imageGeneration.closeGeneratorDialog(this.getUIContext());
      this.dialogStatus = '已关闭';
      this.appendLog('closeGeneratorDialog 完成');
    } catch (err) {
      const e = err as BusinessError;
      this.appendLog(`closeGeneratorDialog 异常: ${e.code} ${e.message}`);
    }
  }

  // ============ 节点图谱操作 ============
  private async doOpenNodeGraph(): Promise<void> {
    if (this.isNodeGraphBusy) {
      this.appendLog('节点图谱操作进行中, 请稍候');
      return;
    }
    this.isNodeGraphBusy = true;
    this.nodeGraphStatus = '正在打开...';
    this.appendLog('调用 openGeneratorNodeGraph');
    try {
      const opts = this.buildNodeGraphOptions();
      await imageGeneration.openGeneratorNodeGraph(this.getUIContext(), opts);
      this.nodeGraphStatus = '已打开';
      this.appendLog('openGeneratorNodeGraph 完成');
    } catch (err) {
      const e = err as BusinessError;
      this.nodeGraphStatus = `打开失败: ${e.code} - ${e.message}`;
      this.appendLog(`openGeneratorNodeGraph 异常: ${e.code} ${e.message}`);
    } finally {
      this.isNodeGraphBusy = false;
    }
  }

  private async doHideNodeGraph(): Promise<void> {
    this.appendLog('调用 hideGeneratorNodeGraph');
    try {
      await imageGeneration.hideGeneratorNodeGraph(this.getUIContext());
      this.nodeGraphStatus = '已隐藏';
      this.appendLog('hideGeneratorNodeGraph 完成');
    } catch (err) {
      const e = err as BusinessError;
      this.appendLog(`hideGeneratorNodeGraph 异常: ${e.code} ${e.message}`);
    }
  }

  private async doRestoreNodeGraph(): Promise<void> {
    this.appendLog('调用 restoreGeneratorNodeGraph');
    try {
      await imageGeneration.restoreGeneratorNodeGraph(this.getUIContext());
      this.nodeGraphStatus = '已恢复';
      this.appendLog('restoreGeneratorNodeGraph 完成');
    } catch (err) {
      const e = err as BusinessError;
      this.appendLog(`restoreGeneratorNodeGraph 异常: ${e.code} ${e.message}`);
    }
  }

  private async doCloseNodeGraph(): Promise<void> {
    this.appendLog('调用 closeGeneratorNodeGraph');
    try {
      await imageGeneration.closeGeneratorNodeGraph(this.getUIContext());
      this.nodeGraphStatus = '已关闭';
      this.appendLog('closeGeneratorNodeGraph 完成');
    } catch (err) {
      const e = err as BusinessError;
      this.appendLog(`closeGeneratorNodeGraph 异常: ${e.code} ${e.message}`);
    }
  }

  // ============ 智能画布服务 ============
  private async doStartSmartCanvas(): Promise<void> {
    this.appendLog('调用 startSmartCanvasService');
    this.smartCanvasStatus = '正在启动...';
    try {
      const ctx = this.getUIContext().getHostContext() as
        common.UIAbilityContext;
      await imageGeneration.startSmartCanvasService(ctx);
      this.smartCanvasStatus = '已启动';
      this.appendLog('startSmartCanvasService 完成');
    } catch (err) {
      const e = err as BusinessError;
      this.smartCanvasStatus = `启动失败: ${e.code} - ${e.message}`;
      this.appendLog(`startSmartCanvasService 异常: ${e.code} ${e.message}`);
    }
  }

  // ============ 流式结果模拟 (演示回调数据结构) ============
  private simulateImageStream(): void {
    if (this.streaming) {
      this.appendLog('流式模拟进行中, 请稍候');
      return;
    }
    this.streaming = true;
    this.streamLog = [];
    this.partialCount = 0;
    this.completedCount = 0;
    this.failCount = 0;
    this.appendStreamLog('START', `开始模拟, 目标数量=${this.imageCount}`);

    const params: imageGeneration.GenerateImageTaskParams = {
      images: this.buildImageItems(),
      prompt: this.prompt,
      style: this.stylePresets[this.selectedStyleIndex].name,
      imageSize: this.sizePresets[this.selectedSizeIndex].size,
      imageCount: this.imageCount
    };
    if (this.usePositionImage) {
      params.positionImage = undefined;
    }
    if (this.useSelectPath) {
      params.selectPath = this.buildSelectPath();
    }

    const sessionId: number = Date.now();
    this.appendStreamLog('REQUEST', `sessionId=${sessionId}, prompt=${params.prompt}`);

    this.mockImageModel.requestImageGeneration(sessionId, params,
      (ret: imageGeneration.GenerateImageTaskPartialResult) => {
        if (ret.type === imageGeneration.PartialResultType.PARTIAL) {
          this.partialCount++;
          this.appendStreamLog('PARTIAL',
            `imageIndex=${ret.imageIndex ?? -1}, imageData=${ret.imageData ?? ''}`);
        } else if (ret.type === imageGeneration.PartialResultType.COMPLETED) {
          this.completedCount++;
          this.appendStreamLog('COMPLETED', `totalCount=${ret.totalCount ?? 0}`);
          this.streaming = false;
        } else if (ret.type === imageGeneration.PartialResultType.PARTIAL_FAIL) {
          this.failCount++;
          const fail: BusinessError | undefined = ret.partialFail;
          this.appendStreamLog('PARTIAL_FAIL',
            `code=${fail?.code}, msg=${fail?.message ?? ''}`);
          this.streaming = false;
        }
      });
  }

  private simulateTextStream(): void {
    this.streamLog = [];
    this.appendStreamLog('TEXT_START', '开始模拟文本润色流式回调');
    const sessionId: number = Date.now() + 1;
    this.mockTextModel.requestTextGeneration(sessionId, this.prompt,
      (ret: imageGeneration.GenerateTextTaskPartialResult) => {
        if (ret.type === imageGeneration.PartialResultType.PARTIAL) {
          this.appendStreamLog('TEXT_PARTIAL',
            `reasoning=${ret.reasoningContent ?? ''}, content=${ret.content ?? ''}`);
        } else if (ret.type === imageGeneration.PartialResultType.COMPLETED) {
          this.appendStreamLog('TEXT_COMPLETED', '文本润色完成');
        } else if (ret.type === imageGeneration.PartialResultType.PARTIAL_FAIL) {
          this.appendStreamLog('TEXT_FAIL', `msg=${ret.partialFail?.message ?? ''}`);
        }
      });
  }

  // ============ 投诉菜单模拟 ============
  private simulateComplain(): void {
    const sessionId: number = 42;
    const request: imageGeneration.GenerateImageTaskParams = {
      images: this.buildImageItems(),
      prompt: this.prompt,
      style: this.stylePresets[this.selectedStyleIndex].name,
      imageSize: this.sizePresets[this.selectedSizeIndex].size
    };
    const result: imageGeneration.GenerateImageTaskResult = {
      imageData: ['mock://result-0', 'mock://result-1']
    };
    this.mockImageModel.onComplain(sessionId, request, result);
    this.complainLog = `已对 sessionId=${sessionId} 的任务发起投诉, ` +
      `结果图片数=${result.imageData.length}`;
    this.appendLog('onComplain(图片) 已调用');
  }

  private simulateTextComplain(): void {
    const sessionId: number = 43;
    const request: string = this.prompt;
    const result: imageGeneration.GenerateTextTaskResult = {
      reasoningContent: 'mock 思考过程',
      content: 'mock 润色结果'
    };
    this.mockTextModel.onComplain(sessionId, request, result);
    this.complainLog = `已对 sessionId=${sessionId} 的文本任务发起投诉, ` +
      `content=${result.content}`;
    this.appendLog('onComplain(文本) 已调用');
  }

  private clearLog(): void {
    this.operationLog = '';
    this.streamLog = [];
    this.complainLog = '';
    this.areaLog = '';
    this.customIconClickLog = '';
    this.customImportLog = '';
  }

  // ============ 多图列表编辑 ============
  private addMultiImage(): void {
    if (this.multiImageUrls.length >= 4) {
      this.appendLog('最多支持 4 张输入图');
      return;
    }
    this.multiImageUrls.push(`/data/storage/el2/database/files/img_${this.multiImageUrls.length + 1}.png`);
    this.multiImageZIndex.push(this.multiImageUrls.length);
    this.multiImageHandwrite.push(false);
    this.appendLog(`新增输入图, 当前共 ${this.multiImageUrls.length} 张`);
  }

  private removeMultiImage(index: number): void {
    if (this.multiImageUrls.length <= 1) {
      this.appendLog('至少保留 1 张输入图');
      return;
    }
    this.multiImageUrls.splice(index, 1);
    this.multiImageZIndex.splice(index, 1);
    this.multiImageHandwrite.splice(index, 1);
    this.appendLog(`移除第 ${index + 1} 张输入图`);
  }

  private buildMultiImageItems(): imageGeneration.ImageItem[] {
    const items: imageGeneration.ImageItem[] = [];
    for (let i = 0; i < this.multiImageUrls.length; i++) {
      const rect: common2D.Rect = {
        left: i * 80,
        top: 0,
        width: 240,
        height: 240
      };
      items.push({
        url: this.multiImageUrls[i],
        rect: rect,
        zIndex: this.multiImageZIndex[i],
        isHandwrite: this.multiImageHandwrite[i]
      });
    }
    return items;
  }

  // ============ 场景预设 ============
  private applyScenario(index: number): void {
    const s = this.scenarios[index];
    this.currentScenarioIndex = index;
    this.prompt = s.prompt;
    this.selectedStyleIndex = s.styleIdx;
    this.selectedSizeIndex = s.sizeIdx;
    this.imageCount = s.count;
    this.isHandwrite = s.name === '手绘线稿上色';
    if (s.name === '文生图' || s.name === '贴纸生成') {
      this.multiImageUrls = ['/data/storage/el2/database/files/placeholder.png'];
      this.multiImageZIndex = [1];
      this.multiImageHandwrite = [false];
    }
    this.appendLog(`应用场景预设: ${s.name}`);
  }

  // ============ 会话管理 ============
  private recordSession(id: number, tag: string): void {
    this.activeSessionId = id;
    const entry: string = `sessionId=${id} (${tag})`;
    this.sessionHistory = [entry].concat(this.sessionHistory).slice(0, 10);
  }

  private simulateCancelImage(): void {
    if (this.activeSessionId < 0) {
      this.appendLog('无活跃会话可取消');
      return;
    }
    this.mockImageModel.cancelImageGeneration(this.activeSessionId);
    this.appendLog(`cancelImageGeneration(${this.activeSessionId}) 已调用`);
    this.activeSessionId = -1;
  }

  private simulateCancelText(): void {
    const sid: number = this.activeSessionId > 0 ? this.activeSessionId + 1 : Date.now();
    this.mockTextModel.cancelTextGeneration(sid);
    this.appendLog(`cancelTextGeneration(${sid}) 已调用`);
  }

  @Builder
  SectionTitle(title: string) {
    Text(title)
      .fontSize(18)
      .fontWeight(FontWeight.Bold)
      .fontColor('#1A1A1A')
      .width('100%')
      .margin({ top: 6, bottom: 6 })
  }

  @Builder
  StatusBadge(text: string, ok: boolean) {
    Text(text)
      .fontSize(12)
      .fontColor(ok ? '#FFFFFF' : '#666666')
      .backgroundColor(ok ? '#007DFF' : '#E5E5E5')
      .padding({ left: 8, right: 8, top: 3, bottom: 3 })
      .borderRadius(10)
  }

  @Builder
  PrimaryButton(label: string, action: () => void) {
    Button(label)
      .fontSize(14)
      .backgroundColor('#007DFF')
      .fontColor('#FFFFFF')
      .height(40)
      .layoutWeight(1)
      .onClick(() => action())
  }

  @Builder
  SecondaryButton(label: string, action: () => void) {
    Button(label)
      .fontSize(14)
      .backgroundColor('#F2F3F5')
      .fontColor('#333333')
      .height(40)
      .layoutWeight(1)
      .onClick(() => action())
  }

  @Builder
  DangerButton(label: string, action: () => void) {
    Button(label)
      .fontSize(14)
      .backgroundColor('#FA5151')
      .fontColor('#FFFFFF')
      .height(40)
      .layoutWeight(1)
      .onClick(() => action())
  }

  @Builder
  StatusOverviewSection() {
    Column({ space: 10 }) {
      this.SectionTitle('状态总览')
      Row({ space: 12 }) {
        Text('弹窗:')
          .fontSize(13)
        this.StatusBadge(this.dialogStatus, this.dialogStatus.startsWith('已打开') || this.dialogStatus.startsWith('已恢复'))
        Text('图谱:')
          .fontSize(13)
        this.StatusBadge(this.nodeGraphStatus,
          this.nodeGraphStatus.startsWith('已打开') || this.nodeGraphStatus.startsWith('已恢复'))
      }
      .width('100%')
      Row({ space: 12 }) {
        Text('画布服务:')
          .fontSize(13)
        this.StatusBadge(this.smartCanvasStatus, this.smartCanvasStatus.startsWith('已启动'))
        Text('模型样式数:')
          .fontSize(13)
        Text(`${this.modelStyles.length}`)
          .fontSize(13)
          .fontColor('#007DFF')
      }
      .width('100%')
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#F5F7FA')
    .borderRadius(12)
  }

  @Builder
  DialogOperationSection() {
    Column({ space: 10 }) {
      this.SectionTitle('一、AI 图片生成弹窗 (showGeneratorDialog 等)')
      Row({ space: 10 }) {
        this.PrimaryButton('打开弹窗', () => this.doShowDialog())
        this.SecondaryButton('隐藏', () => this.doHideDialog())
        this.SecondaryButton('恢复', () => this.doRestoreDialog())
      }
      Row({ space: 10 }) {
        this.DangerButton('关闭弹窗', () => this.doCloseDialog())
        this.SecondaryButton('清空日志', () => this.clearLog())
      }
      Text('showGeneratorDialog 返回 Promise<GeneratorResult[]>; ' +
        'hide/restore/close 返回 Promise<void>')
        .fontSize(11)
        .fontColor('#888888')
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  NodeGraphOperationSection() {
    Column({ space: 10 }) {
      this.SectionTitle('二、AI 节点图谱 Sheet (openGeneratorNodeGraph 等)')
      Row({ space: 10 }) {
        this.PrimaryButton('打开图谱', () => this.doOpenNodeGraph())
        this.SecondaryButton('隐藏', () => this.doHideNodeGraph())
        this.SecondaryButton('恢复', () => this.doRestoreNodeGraph())
      }
      Row({ space: 10 }) {
        this.DangerButton('关闭图谱', () => this.doCloseNodeGraph())
      }
      Text('openGeneratorNodeGraph 以 Sheet 形式打开节点编辑画布, ' +
        '配置项 GeneratorNodeGraphOptions 与弹窗基本一致')
        .fontSize(11)
        .fontColor('#888888')
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  SmartCanvasSection() {
    Column({ space: 10 }) {
      this.SectionTitle('三、智能画布服务 (startSmartCanvasService)')
      Row({ space: 10 }) {
        this.PrimaryButton('启动画布服务', () => this.doStartSmartCanvas())
      }
      Text('需传入 UIAbilityContext / ServiceExtensionContext / UIExtensionContext (API 26.0.1+)')
        .fontSize(11)
        .fontColor('#888888')
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  ModelSwitchSection() {
    Column({ space: 10 }) {
      this.SectionTitle('四、模型接入开关')
      Row({ space: 20 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.useImageModel })
          .onChange((v: boolean) => {
            this.useImageModel = v;
            this.refreshModelStyles();
            this.appendLog(`图片生成模型: ${v ? '启用' : '禁用'}`);
          })
        Text('图片生成模型 (ImageGenerationModel)')
          .fontSize(14)
      }
      Row({ space: 20 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.useTextModel })
          .onChange((v: boolean) => {
            this.useTextModel = v;
            this.appendLog(`文本润色模型: ${v ? '启用' : '禁用'}`);
          })
        Text('文本润色模型 (TextGenerationModel)')
          .fontSize(14)
      }
      Row({ space: 20 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.useLiveView })
          .onChange((v: boolean) => {
            this.useLiveView = v;
            this.appendLog(`实况窗信息: ${v ? '启用' : '禁用'}`);
          })
        Text('实况窗信息 (LiveViewInfo, API 26+)')
          .fontSize(14)
      }
      Row({ space: 20 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.minimizeDuringGeneration })
          .onChange((v: boolean) => {
            this.minimizeDuringGeneration = v;
            this.appendLog(`生成中最小化: ${v ? '开启' : '关闭'}`);
          })
        Text('生成中最小化 (minimizeDuringGeneration, API 26+)')
          .fontSize(14)
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  ModelStylesSection() {
    Column({ space: 10 }) {
      this.SectionTitle('五、模型支持的样式 (getModelSupportStyles)')
      if (this.modelStyles.length === 0) {
        Text('未启用图片生成模型, 无样式数据')
          .fontSize(13)
          .fontColor('#999999')
      } else {
        ForEach(this.modelStyles, (item: imageGeneration.ImageStyle, index: number) => {
          Row({ space: 10 }) {
            Text(`#${index + 1}`)
              .fontSize(13)
              .fontColor('#007DFF')
              .width(30)
            Text(item.name.toString())
              .fontSize(14)
              .fontWeight(FontWeight.Medium)
              .layoutWeight(1)
          }
          .width('100%')
          .padding(10)
          .backgroundColor('#F5F7FA')
          .borderRadius(8)
        }, (item: imageGeneration.ImageStyle, index: number) => `${index}-${item.name}`)
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  ImageItemConfigSection() {
    Column({ space: 10 }) {
      this.SectionTitle('六、输入图片项配置 (ImageItem)')
      Text('url (图片路径):')
        .fontSize(13)
      TextInput({ text: this.imageUrl })
        .fontSize(13)
        .height(40)
        .onChange((v: string) => { this.imageUrl = v; })

      Text(`rect 容器位置与尺寸 (x/y/w/h):`)
        .fontSize(13)
      Row({ space: 8 }) {
        Text('X')
          .fontSize(12)
          .width(20)
        Slider({ value: this.imageRectX, min: 0, max: 400, step: 1 })
          .layoutWeight(1)
          .onChange((v: number) => { this.imageRectX = v; })
        Text(`${this.imageRectX}`)
          .fontSize(12)
          .width(40)
      }
      Row({ space: 8 }) {
        Text('Y')
          .fontSize(12)
          .width(20)
        Slider({ value: this.imageRectY, min: 0, max: 400, step: 1 })
          .layoutWeight(1)
          .onChange((v: number) => { this.imageRectY = v; })
        Text(`${this.imageRectY}`)
          .fontSize(12)
          .width(40)
      }
      Row({ space: 8 }) {
        Text('W')
          .fontSize(12)
          .width(20)
        Slider({ value: this.imageRectW, min: 64, max: 512, step: 1 })
          .layoutWeight(1)
          .onChange((v: number) => { this.imageRectW = v; })
        Text(`${this.imageRectW}`)
          .fontSize(12)
          .width(40)
      }
      Row({ space: 8 }) {
        Text('H')
          .fontSize(12)
          .width(20)
        Slider({ value: this.imageRectH, min: 64, max: 512, step: 1 })
          .layoutWeight(1)
          .onChange((v: number) => { this.imageRectH = v; })
        Text(`${this.imageRectH}`)
          .fontSize(12)
          .width(40)
      }

      Text(`rotation 旋转角度 (componentUtils.Rotation2D.angle): ${this.imageRotation}`)
        .fontSize(13)
      Slider({ value: this.imageRotation, min: 0, max: 360, step: 1 })
        .width('100%')
        .onChange((v: number) => { this.imageRotation = v; })

      Text(`zIndex 渲染层级: ${this.imageZIndex}`)
        .fontSize(13)
      Slider({ value: this.imageZIndex, min: 0, max: 10, step: 1 })
        .width('100%')
        .onChange((v: number) => { this.imageZIndex = v; })

      Row({ space: 20 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.isHandwrite })
          .onChange((v: boolean) => { this.isHandwrite = v; })
        Text('isHandwrite 手绘线稿')
          .fontSize(13)
      }
      Row({ space: 20 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.usePositionImage })
          .onChange((v: boolean) => { this.usePositionImage = v; })
        Text('positionImage 位置参考图 (多图融合)')
          .fontSize(13)
      }
      Row({ space: 20 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.useSelectPath })
          .onChange((v: boolean) => { this.useSelectPath = v; })
        Text('selectPath 套索选择路径')
          .fontSize(13)
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  TaskParamsSection() {
    Column({ space: 10 }) {
      this.SectionTitle('七、生成任务参数 (GenerateImageTaskParams)')
      Text('prompt 描述文本:')
        .fontSize(13)
      TextArea({ text: this.prompt })
        .fontSize(13)
        .height(70)
        .onChange((v: string) => { this.prompt = v; })

      Text('弹窗初始 content:')
        .fontSize(13)
      TextInput({ text: this.initContent })
        .fontSize(13)
        .height(40)
        .onChange((v: string) => { this.initContent = v; })

      Text('style 样式选择:')
        .fontSize(13)
      Row({ space: 8 }) {
        ForEach(this.stylePresets, (item: StylePreset, index: number) => {
          Button(item.name)
            .fontSize(12)
            .height(32)
            .backgroundColor(this.selectedStyleIndex === index ? '#007DFF' : '#F2F3F5')
            .fontColor(this.selectedStyleIndex === index ? '#FFFFFF' : '#333333')
            .onClick(() => {
              this.selectedStyleIndex = index;
              this.appendLog(`选择样式: ${item.name}`);
            })
        }, (item: StylePreset, index: number) => `${index}-${item.name}`)
      }

      Text('imageSize 输出尺寸:')
        .fontSize(13)
      Column({ space: 6 }) {
        ForEach(this.sizePresets, (item: SizePreset, index: number) => {
          Row({ space: 8 }) {
            Radio({ value: item.label, group: 'sizeGroup' })
              .checked(this.selectedSizeIndex === index)
              .onChange((v: boolean) => {
                if (v) {
                  this.selectedSizeIndex = index;
                  this.appendLog(`选择尺寸: ${item.label}`);
                }
              })
            Text(item.label)
              .fontSize(13)
          }
        }, (item: SizePreset, index: number) => `${index}-${item.label}`)
      }

      Text(`imageCount 生成数量: ${this.imageCount}`)
        .fontSize(13)
      Slider({ value: this.imageCount, min: 1, max: 6, step: 1 })
        .width('100%')
        .onChange((v: number) => { this.imageCount = v; })
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  CustomIconsSection() {
    Column({ space: 10 }) {
      this.SectionTitle('八、结果页自定义图标 (GeneratorResultPageIcon)')
      Row({ space: 20 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.enableCustomIcons })
          .onChange((v: boolean) => { this.enableCustomIcons = v; })
        Text('启用结果页自定义图标 (收藏 / 分享 / 设为壁纸)')
          .fontSize(13)
      }
      if (this.customIconClickLog.length > 0) {
        Text(this.customIconClickLog)
          .fontSize(12)
          .fontColor('#007DFF')
          .padding(8)
          .backgroundColor('#EAF2FF')
          .borderRadius(8)
          .width('100%')
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  CustomImportSection() {
    Column({ space: 10 }) {
      this.SectionTitle('九、自定义导入图标 (CustomImportIcon, API 26+)')
      Row({ space: 20 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.enableCustomImport })
          .onChange((v: boolean) => { this.enableCustomImport = v; })
        Text('启用从应用侧导入图片与文本')
          .fontSize(13)
      }
      if (this.customImportLog.length > 0) {
        Text(this.customImportLog)
          .fontSize(12)
          .fontColor('#007DFF')
          .padding(8)
          .backgroundColor('#EAF2FF')
          .borderRadius(8)
          .width('100%')
      }
      Text('CustomImportCallback 返回 Promise<CustomImportResult>, ' +
        '可注入 images 与 content')
        .fontSize(11)
        .fontColor('#888888')
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  AreaCallbackSection() {
    Column({ space: 10 }) {
      this.SectionTitle('十、弹窗区域变更回调 (onAreaDidChange)')
      Row({ space: 20 }) {
        Toggle({ type: ToggleType.Switch, isOn: this.enableAreaCallback })
          .onChange((v: boolean) => { this.enableAreaCallback = v; })
        Text('监听弹窗尺寸 / 位置变化')
          .fontSize(13)
      }
      if (this.areaLog.length > 0) {
        Text(this.areaLog)
          .fontSize(12)
          .fontColor('#007DFF')
          .padding(8)
          .backgroundColor('#EAF2FF')
          .borderRadius(8)
          .width('100%')
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  StreamSimulationSection() {
    Column({ space: 10 }) {
      this.SectionTitle('十一、流式结果模拟 (PartialResultType)')
      Row({ space: 10 }) {
        this.PrimaryButton('模拟图片流式', () => this.simulateImageStream())
        this.SecondaryButton('模拟文本流式', () => this.simulateTextStream())
      }
      Row({ space: 16 }) {
        Text(`PARTIAL: ${this.partialCount}`)
          .fontSize(12)
          .fontColor('#007DFF')
        Text(`COMPLETED: ${this.completedCount}`)
          .fontSize(12)
          .fontColor('#34C759')
        Text(`PARTIAL_FAIL: ${this.failCount}`)
          .fontSize(12)
          .fontColor('#FA5151')
      }
      if (this.streaming) {
        Text('生成中...')
          .fontSize(12)
          .fontColor('#FA5151')
      }
      if (this.streamLog.length > 0) {
        Column({ space: 4 }) {
          ForEach(this.streamLog, (entry: StreamLogEntry, index: number) => {
            Row({ space: 6 }) {
              Text(entry.time)
                .fontSize(11)
                .fontColor('#999999')
                .width(64)
              Text(entry.type)
                .fontSize(11)
                .fontColor(this.streamColor(entry.type))
                .width(100)
              Text(entry.detail)
                .fontSize(11)
                .fontColor('#333333')
                .layoutWeight(1)
            }
            .width('100%')
          }, (entry: StreamLogEntry, index: number) => `${index}-${entry.type}-${entry.time}`)
        }
        .width('100%')
        .padding(8)
        .backgroundColor('#F5F7FA')
        .borderRadius(8)
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  ComplainSection() {
    Column({ space: 10 }) {
      this.SectionTitle('十二、投诉菜单 (onComplain)')
      Row({ space: 10 }) {
        this.SecondaryButton('投诉图片结果', () => this.simulateComplain())
        this.SecondaryButton('投诉文本结果', () => this.simulateTextComplain())
      }
      if (this.complainLog.length > 0) {
        Text(this.complainLog)
          .fontSize(12)
          .fontColor('#FA5151')
          .padding(8)
          .backgroundColor('#FFF1F0')
          .borderRadius(8)
          .width('100%')
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  ResultDisplaySection() {
    Column({ space: 10 }) {
      this.SectionTitle('十三、生成结果 (GeneratorResult / TaskStatistic)')
      Text(this.resultSummary)
        .fontSize(13)
        .fontColor('#333333')
        .width('100%')
      if (this.lastResults.length > 0) {
        ForEach(this.lastResults, (res: imageGeneration.GeneratorResult, index: number) => {
          Column({ space: 4 }) {
            Text(`结果 #${index + 1}`)
              .fontSize(13)
              .fontWeight(FontWeight.Medium)
            Text(`url: ${res.url ?? 'N/A'}`)
              .fontSize(12)
              .fontColor('#666666')
            Text(`statistic.style: ${res.statistic.style}`)
              .fontSize(12)
              .fontColor('#666666')
            Text(`statistic.isSticker: ${res.statistic.isSticker}`)
              .fontSize(12)
              .fontColor('#666666')
            Text(`statistic.imageCount: ${res.statistic.imageCount}`)
              .fontSize(12)
              .fontColor('#666666')
            Text(`statistic.generationTime: ${res.statistic.generationTime}s`)
              .fontSize(12)
              .fontColor('#666666')
          }
          .width('100%')
          .padding(10)
          .backgroundColor('#F5F7FA')
          .borderRadius(8)
        }, (res: imageGeneration.GeneratorResult, index: number) => `${index}-${res.url ?? ''}`)
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  LiveViewSection() {
    Column({ space: 10 }) {
      this.SectionTitle('十四、实况窗信息 (LiveViewInfo, API 26+)')
      if (this.useLiveView) {
        Column({ space: 4 }) {
          Text(`getWant(): ${JSON.stringify(this.mockLiveView.getWant())}`)
            .fontSize(12)
            .fontColor('#666666')
          Text(`getLongTermTaskId(): ${this.mockLiveView.getLongTermTaskId()}`)
            .fontSize(12)
            .fontColor('#666666')
          Text(`isLiveViewNeeded(): ${this.mockLiveView.isLiveViewNeeded()}`)
            .fontSize(12)
            .fontColor('#666666')
        }
        .width('100%')
        .padding(10)
        .backgroundColor('#F5F7FA')
        .borderRadius(8)
      } else {
        Text('请先在"模型接入开关"中开启实况窗信息')
          .fontSize(12)
          .fontColor('#999999')
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  MultiImageSection() {
    Column({ space: 10 }) {
      this.SectionTitle('十五、多图列表编辑 (ImageItem[])')
      Text('多图融合场景下可配置多张输入图, 每张独立设置 url / zIndex / isHandwrite')
        .fontSize(11)
        .fontColor('#888888')
      ForEach(this.multiImageUrls, (url: string, index: number) => {
        Column({ space: 6 }) {
          Row({ space: 8 }) {
            Text(`图 ${index + 1}`)
              .fontSize(12)
              .fontWeight(FontWeight.Medium)
              .width(40)
            TextInput({ text: url })
              .fontSize(12)
              .height(36)
              .layoutWeight(1)
              .onChange((v: string) => {
                this.multiImageUrls[index] = v;
              })
            Button('删除')
              .fontSize(11)
              .height(32)
              .backgroundColor('#FA5151')
              .fontColor('#FFFFFF')
              .onClick(() => this.removeMultiImage(index))
          }
          Row({ space: 12 }) {
            Text(`zIndex: ${this.multiImageZIndex[index]}`)
              .fontSize(11)
              .width(80)
            Button('-')
              .fontSize(12)
              .width(32)
              .height(28)
              .backgroundColor('#F2F3F5')
              .fontColor('#333333')
              .onClick(() => {
                if (this.multiImageZIndex[index] > 0) {
                  this.multiImageZIndex[index] = this.multiImageZIndex[index] - 1;
                }
              })
            Button('+')
              .fontSize(12)
              .width(32)
              .height(28)
              .backgroundColor('#F2F3F5')
              .fontColor('#333333')
              .onClick(() => {
                if (this.multiImageZIndex[index] < 10) {
                  this.multiImageZIndex[index] = this.multiImageZIndex[index] + 1;
                }
              })
            Text(this.multiImageHandwrite[index] ? '手绘线稿' : '普通图')
              .fontSize(11)
              .fontColor(this.multiImageHandwrite[index] ? '#007DFF' : '#999999')
              .layoutWeight(1)
            Toggle({ type: ToggleType.Switch, isOn: this.multiImageHandwrite[index] })
              .onChange((v: boolean) => {
                this.multiImageHandwrite[index] = v;
              })
          }
        }
        .width('100%')
        .padding(8)
        .backgroundColor('#F5F7FA')
        .borderRadius(8)
      }, (url: string, index: number) => `${index}-${url}`)
      Row({ space: 10 }) {
        this.SecondaryButton('新增图片', () => this.addMultiImage())
        this.SecondaryButton('查看构造结果', () => {
          const items = this.buildMultiImageItems();
          this.appendLog(`多图构造完成, 共 ${items.length} 项`);
        })
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  ScenarioPresetSection() {
    Column({ space: 10 }) {
      this.SectionTitle('十六、场景预设 (一键配置)')
      ForEach(this.scenarios, (s: ScenarioPreset, index: number) => {
        Row({ space: 10 }) {
          Column({ space: 3 }) {
            Text(s.name)
              .fontSize(14)
              .fontWeight(FontWeight.Medium)
              .fontColor(this.currentScenarioIndex === index ? '#007DFF' : '#333333')
            Text(s.desc)
              .fontSize(11)
              .fontColor('#999999')
          }
          .alignItems(HorizontalAlign.Start)
          .layoutWeight(1)
          Button('应用')
            .fontSize(12)
            .height(32)
            .backgroundColor(this.currentScenarioIndex === index ? '#007DFF' : '#F2F3F5')
            .fontColor(this.currentScenarioIndex === index ? '#FFFFFF' : '#333333')
            .onClick(() => this.applyScenario(index))
        }
        .width('100%')
        .padding(10)
        .backgroundColor(this.currentScenarioIndex === index ? '#EAF2FF' : '#F5F7FA')
        .borderRadius(8)
      }, (s: { name: string; desc: string }, index: number) => `${index}-${s.name}`)
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  SessionManagementSection() {
    Column({ space: 10 }) {
      this.SectionTitle('十七、会话管理 (sessionId / cancel)')
      Row({ space: 10 }) {
        Text('活跃 sessionId:')
          .fontSize(13)
        Text(this.activeSessionId >= 0 ? `${this.activeSessionId}` : '无')
          .fontSize(13)
          .fontColor(this.activeSessionId >= 0 ? '#007DFF' : '#999999')
      }
      Row({ space: 10 }) {
        this.SecondaryButton('记录模拟会话', () => {
          this.recordSession(Date.now(), 'mock-image');
          this.appendLog('记录模拟会话');
        })
        this.DangerButton('取消图片会话', () => this.simulateCancelImage())
      }
      Row({ space: 10 }) {
        this.DangerButton('取消文本会话', () => this.simulateCancelText())
      }
      if (this.sessionHistory.length > 0) {
        Text('会话历史 (最近 10 条):')
          .fontSize(12)
        Column({ space: 4 }) {
          ForEach(this.sessionHistory, (entry: string, index: number) => {
            Text(entry)
              .fontSize(11)
              .fontColor('#666666')
              .width('100%')
          }, (entry: string, index: number) => `${index}-${entry}`)
        }
        .width('100%')
        .padding(8)
        .backgroundColor('#F5F7FA')
        .borderRadius(8)
      }
      Text('cancelImageGeneration / cancelTextGeneration 通过 sessionId 取消对应任务, ' +
        'sessionId 取值范围 [0, +∞)')
        .fontSize(11)
        .fontColor('#888888')
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  DataStructureReferenceSection() {
    Column({ space: 10 }) {
      this.SectionTitle('十八、接口数据结构参考')
      Column({ space: 6 }) {
        Text('GenerateImageTaskParams:')
          .fontSize(12)
          .fontWeight(FontWeight.Medium)
        Text('{ images, positionImage?, selectPath?, prompt, style?, imageSize, imageCount? }')
          .fontSize(11)
          .fontColor('#666666')
      }
      Column({ space: 6 }) {
        Text('GenerateImageTaskPartialResult:')
          .fontSize(12)
          .fontWeight(FontWeight.Medium)
        Text('{ type, imageIndex?, imageData?, totalCount?, partialFail? }')
          .fontSize(11)
          .fontColor('#666666')
      }
      Column({ space: 6 }) {
        Text('GenerateTextTaskPartialResult:')
          .fontSize(12)
          .fontWeight(FontWeight.Medium)
        Text('{ type, reasoningContent?, content?, partialFail? }')
          .fontSize(11)
          .fontColor('#666666')
      }
      Column({ space: 6 }) {
        Text('ImageItem:')
          .fontSize(12)
          .fontWeight(FontWeight.Medium)
        Text('{ image?, url?, rect?, rotation?, zIndex?, isHandwrite? }')
          .fontSize(11)
          .fontColor('#666666')
      }
      Column({ space: 6 }) {
        Text('GeneratorResult / TaskStatistic:')
          .fontSize(12)
          .fontWeight(FontWeight.Medium)
        Text('{ image?, url?, statistic:{ style, isSticker, imageCount, generationTime } }')
          .fontSize(11)
          .fontColor('#666666')
      }
      Column({ space: 6 }) {
        Text('GeneratorDialogOptions:')
          .fontSize(12)
          .fontWeight(FontWeight.Medium)
        Text('{ images?, content?, customIcons?, imageGenerationModel?, ' +
          'textGenerationModel?, onAreaDidChange?, customImportIcon?, ' +
          'liveViewInfo?, minimizeDuringGeneration? }')
          .fontSize(11)
          .fontColor('#666666')
      }
      Column({ space: 6 }) {
        Text('GeneratorNodeGraphOptions:')
          .fontSize(12)
          .fontWeight(FontWeight.Medium)
        Text('{ customIcons?, imageGenerationModel?, textGenerationModel?, ' +
          'customImportIcon?, liveViewInfo? }')
          .fontSize(11)
          .fontColor('#666666')
      }
      Column({ space: 6 }) {
        Text('CustomImportIcon / CustomImportResult:')
          .fontSize(12)
          .fontWeight(FontWeight.Medium)
        Text('Icon:{ image, text, callback: ()=>Promise<CustomImportResult> }')
          .fontSize(11)
          .fontColor('#666666')
        Text('Result:{ images?: ImageItem[], content?: ResourceStr }')
          .fontSize(11)
          .fontColor('#666666')
      }
      Column({ space: 6 }) {
        Text('GeneratorResultPageIcon:')
          .fontSize(12)
          .fontWeight(FontWeight.Medium)
        Text('{ image, text, callback: Callback<GeneratorResult> }')
          .fontSize(11)
          .fontColor('#666666')
      }
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  CallFlowSection() {
    Column({ space: 10 }) {
      this.SectionTitle('十九、调用流程说明')
      Text('图片生成弹窗典型流程:')
        .fontSize(13)
        .fontWeight(FontWeight.Medium)
      Text('1. 构造 ImageItem 数组与 prompt')
        .fontSize(11)
        .fontColor('#666666')
      Text('2. (可选) 实现 ImageGenerationModel / TextGenerationModel')
        .fontSize(11)
        .fontColor('#666666')
      Text('3. 组装 GeneratorDialogOptions')
        .fontSize(11)
        .fontColor('#666666')
      Text('4. 调用 showGeneratorDialog(uiContext, options)')
        .fontSize(11)
        .fontColor('#666666')
      Text('5. 弹窗内用户触发生成 -> 模型 requestImageGeneration')
        .fontSize(11)
        .fontColor('#666666')
      Text('6. 通过回调接收 PARTIAL -> ... -> COMPLETED')
        .fontSize(11)
        .fontColor('#666666')
      Text('7. (可选) hide / restore 在生成中最小化')
        .fontSize(11)
        .fontColor('#666666')
      Text('8. closeGeneratorDialog 关闭, resolve 返回 GeneratorResult[]')
        .fontSize(11)
        .fontColor('#666666')
      Divider().strokeWidth(1).color('#EDEDED').margin({ top: 6, bottom: 6 })
      Text('节点图谱流程与弹窗类似, 使用 openGeneratorNodeGraph / closeGeneratorNodeGraph')
        .fontSize(11)
        .fontColor('#888888')
      Text('智能画布服务独立于弹窗, 由 startSmartCanvasService 启动后台能力')
        .fontSize(11)
        .fontColor('#888888')
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  OperationLogSection() {
    Column({ space: 10 }) {
      this.SectionTitle('二十、操作日志')
      Text(this.operationLog.length > 0 ? this.operationLog : '暂无日志')
        .fontSize(11)
        .fontColor('#333333')
        .width('100%')
        .multiline(true)
        .padding(10)
        .backgroundColor('#FAFAFA')
        .borderRadius(8)
        .textAlign(TextAlign.Start)
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  NotesSection() {
    Column({ space: 10 }) {
      this.SectionTitle('二十一、注意事项')
      Text('1. 本接口为 @systemapi, 仅系统应用可用, 需系统签名权限')
        .fontSize(11)
        .fontColor('#666666')
      Text('2. 仅 stage 模型工程可用 (@stagemodelonly)')
        .fontSize(11)
        .fontColor('#666666')
      Text('3. 真实生成依赖设备端 AI 能力, 无能力时弹窗回退到默认 Celia 模型')
        .fontSize(11)
        .fontColor('#666666')
      Text('4. 自定义模型实现 ImageGenerationModel / TextGenerationModel 后, ' +
        '通过 options.imageGenerationModel / textGenerationModel 注入')
        .fontSize(11)
        .fontColor('#666666')
      Text('5. requestImageGeneration 的 callback 通过 PartialResultType 区分流式阶段')
        .fontSize(11)
        .fontColor('#666666')
      Text('6. imageData 为 string 类型路径, GeneratorResult.url 为生成图路径')
        .fontSize(11)
        .fontColor('#666666')
      Text('7. selectPath 套索路径为 common2D.Point[], 用于局部编辑')
        .fontSize(11)
        .fontColor('#666666')
      Text('8. 多图融合建议提供 rect / rotation / zIndex 以获得更好效果')
        .fontSize(11)
        .fontColor('#666666')
      Text('9. isHandwrite=true 时建议手绘线稿场景, 模型将执行上色')
        .fontSize(11)
        .fontColor('#666666')
      Text('10. minimizeDuringGeneration=true 时生成过程中弹窗可最小化 (API 26+)')
        .fontSize(11)
        .fontColor('#666666')
      Text('11. LiveViewInfo / CustomImportIcon / hide / restore 相关接口自 API 26 起支持')
        .fontSize(11)
        .fontColor('#666666')
      Text('12. startSmartCanvasService 自 API 26.0.1 起支持, 需传入合法 ability context')
        .fontSize(11)
        .fontColor('#666666')
      Text('13. 示例中的 Mock 模型仅演示接口实现, 实际接入请替换为真实 AI 模型')
        .fontSize(11)
        .fontColor('#666666')
      Text('14. onAreaDidChange 回调参数为 common2D.Rect, 含 left/top/width/height')
        .fontSize(11)
        .fontColor('#666666')
      Text('15. 投诉 onComplain 不返回结果, 仅记录用户反馈')
        .fontSize(11)
        .fontColor('#666666')
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
  }

  @Builder
  ApiReferenceSection() {
    Column({ space: 8 }) {
      this.SectionTitle('附: API 速查')
      Text('showGeneratorDialog(uiContext, options?) -> Promise<GeneratorResult[]>')
        .fontSize(11)
        .fontColor('#666666')
      Text('closeGeneratorDialog(uiContext) -> Promise<void> (API 24+)')
        .fontSize(11)
        .fontColor('#666666')
      Text('hideGeneratorDialog(uiContext) -> Promise<void> (API 26+)')
        .fontSize(11)
        .fontColor('#666666')
      Text('restoreGeneratorDialog(uiContext) -> Promise<void> (API 26+)')
        .fontSize(11)
        .fontColor('#666666')
      Text('openGeneratorNodeGraph(uiContext, options?) -> Promise<void>')
        .fontSize(11)
        .fontColor('#666666')
      Text('closeGeneratorNodeGraph(uiContext) -> Promise<void>')
        .fontSize(11)
        .fontColor('#666666')
      Text('hideGeneratorNodeGraph(uiContext) -> Promise<void> (API 26+)')
        .fontSize(11)
        .fontColor('#666666')
      Text('restoreGeneratorNodeGraph(uiContext) -> Promise<void> (API 26+)')
        .fontSize(11)
        .fontColor('#666666')
      Text('startSmartCanvasService(context) -> Promise<void> (API 26.0.1+)')
        .fontSize(11)
        .fontColor('#666666')
      Text('ImageGenerationModel: getModelSupportStyles / requestImageGeneration / cancelImageGeneration / onComplain')
        .fontSize(11)
        .fontColor('#666666')
      Text('TextGenerationModel: requestTextGeneration / cancelTextGeneration / onComplain')
        .fontSize(11)
        .fontColor('#666666')
      Text('PartialResultType: PARTIAL(0) / COMPLETED(1) / PARTIAL_FAIL(2)')
        .fontSize(11)
        .fontColor('#666666')
      Text('ImageStyle: { icon: PixelMap|string|Resource, name: ResourceStr }')
        .fontSize(11)
        .fontColor('#666666')
      Text('ImageGenerationModel.getModelSupportStyles() -> ImageStyle[]')
        .fontSize(11)
        .fontColor('#666666')
      Text('requestImageGeneration(sessionId, params, callback) -> void')
        .fontSize(11)
        .fontColor('#666666')
      Text('cancelImageGeneration(sessionId) -> void')
        .fontSize(11)
        .fontColor('#666666')
      Text('onComplain(sessionId, request, result) -> void')
        .fontSize(11)
        .fontColor('#666666')
      Text('requestTextGeneration(sessionId, value, callback) -> void')
        .fontSize(11)
        .fontColor('#666666')
      Text('cancelTextGeneration(sessionId) -> void')
        .fontSize(11)
        .fontColor('#666666')
      Text('LiveViewInfo: getWant() / getLongTermTaskId() / isLiveViewNeeded() (API 26+)')
        .fontSize(11)
        .fontColor('#666666')
      Text('CustomImportCallback: () => Promise<CustomImportResult> (API 26+)')
        .fontSize(11)
        .fontColor('#666666')
      Text('GeneratorResultPageIcon.callback: Callback<GeneratorResult>')
        .fontSize(11)
        .fontColor('#666666')
    }
    .width('100%')
    .padding(14)
    .backgroundColor('#FFFFFF')
    .borderRadius(12)
    .margin({ bottom: 20 })
  }

  build() {
    Column() {
      Text('imageGeneration AI 图片生成示例')
        .fontSize(24)
        .fontWeight(FontWeight.Bold)
        .fontColor('#1A1A1A')
        .margin({ top: 12, bottom: 4 })

      Text('@ohos.arkui.intelligence.imageGeneration (API 23+)')
        .fontSize(12)
        .fontColor('#999999')
        .margin({ bottom: 8 })

      Scroll() {
        Column({ space: 16 }) {
          this.StatusOverviewSection()
          Divider().strokeWidth(1).color('#EDEDED')
          this.DialogOperationSection()
          this.NodeGraphOperationSection()
          this.SmartCanvasSection()
          Divider().strokeWidth(1).color('#EDEDED')
          this.ModelSwitchSection()
          this.ModelStylesSection()
          this.ImageItemConfigSection()
          this.TaskParamsSection()
          this.CustomIconsSection()
          this.CustomImportSection()
          this.AreaCallbackSection()
          Divider().strokeWidth(1).color('#EDEDED')
          this.StreamSimulationSection()
          this.ComplainSection()
          this.ResultDisplaySection()
          this.LiveViewSection()
          this.MultiImageSection()
          this.ScenarioPresetSection()
          this.SessionManagementSection()
          this.DataStructureReferenceSection()
          this.CallFlowSection()
          this.OperationLogSection()
          this.NotesSection()
          this.ApiReferenceSection()
        }
        .width('100%')
        .padding({ left: 16, right: 16 })
      }
      .layoutWeight(1)
      .width('100%')
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#F0F2F5')
  }

  // 根据流式结果类型返回展示颜色
  private streamColor(type: string): ResourceColor {
    if (type === 'COMPLETED' || type === 'TEXT_COMPLETED') {
      return '#34C759';
    }
    if (type.indexOf('FAIL') >= 0) {
      return '#FA5151';
    }
    if (type === 'START' || type === 'TEXT_START' || type === 'REQUEST') {
      return '#999999';
    }
    return '#007DFF';
  }
}
