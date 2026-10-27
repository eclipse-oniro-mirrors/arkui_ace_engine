import { util } from '@kit.ArkTS';
import { pasteboard } from '@kit.BasicServicesKit';
import { promptAction, LengthMetrics } from '@kit.ArkUI';
import { common, Configuration, EnvironmentCallback, AbilityConstant } from '@kit.AbilityKit';
import { AiCardConstants } from './common/AiCardConstants';
import { AiCardLogger } from './common/AiCardLogger';
import {
  AiCardDisplayConfig, AiCardLifecycleCallbacks, CardRecommendConfig, AiCardStreamConfig,
  AiSearchCardDataPayload, AiSearchCardRawData, AiSearchComplianceConfig,
  AiSearchDislikeFeedback, AiSearchError, AiSearchRelevantItem,
  CardCancelPhase, CardRecommendChip, CardRecommendSegment, CardRequestOptions, RecommendChipSegmentType,
  CollapseMode, LineMeasureInfo, MeasureResult, SearchProvider, SearchProviderType
} from './common/AiCardTypes';
import { MarkdownParser, MarkdownStyleType, ParsedMarkdown, MarkdownStyleRange } from './common/MarkdownParser';
import { TextDirectionUtil } from './common/TextDirectionUtil';
import { AiCardController, AiCardControlTarget, bindAiCardController, unbindAiCardController } from './controller/AiCardController';
import { AiCardSessionController, AiCardSessionControllerOptions } from './controller/AiCardSessionController';
import { TitleBar } from './ui/TitleBar';
import { ContentView } from './ui/ContentView';
import { RecommendZone } from './ui/RecommendZone';
import { ComplianceZone } from './ui/ComplianceZone';
import { DislikeDialog } from './ui/DislikeDialog';
import { SkeletonLoader } from './ui/SkeletonLoader';
import { ErrorPlaceholder } from './ui/ErrorPlaceholder';

class AiCardControlTargetImpl implements AiCardControlTarget {
  private readonly doRequest: (query: string, requestOptions?: CardRequestOptions) => void;
  private readonly doCancel: () => void;

  constructor(doRequest: (query: string, requestOptions?: CardRequestOptions) => void, doCancel: () => void) {
    this.doRequest = doRequest;
    this.doCancel = doCancel;
  }

  request(query: string, requestOptions?: CardRequestOptions): void {
    this.doRequest(query, requestOptions);
  }

  cancel(): void {
    this.doCancel();
  }
}

@ComponentV2
export struct AiCard {
  @Param searchProviderType: SearchProviderType = SearchProviderType.DEFAULT;
  @Param streamConfig: AiCardStreamConfig | null = null;
  @Param displayConfig: AiCardDisplayConfig | null = null;
  @Param recommendConfig: CardRecommendConfig | null = null;
  @Param complianceConfig: AiSearchComplianceConfig | null = null;
  @Param lifecycleCallbacks: AiCardLifecycleCallbacks | null = null;
  /** 搜索出错回调（外层直传，aicard-api-refactor：onError 从 lifecycleCallbacks 外层化）。 */
  @Param onError: ((error: AiSearchError) => void) | null = null;
  /** 开发者持有的控制器句柄（文档级必传：未传时卡片挂载永久待命，运行安全；搜索唯一入口为 controller.request(query)；
   * 不支持中途更换 controller 实例——@Param 引用变化不触发重绑，需更换时重建组件实例）。 */
  @Param controller: AiCardController | null = null;
  @BuilderParam cardContent: (data: AiSearchCardDataPayload, index: number) => void = this.DefaultCard;
  private sessionController: AiCardSessionController | null = null;
  private boundController: AiCardController | null = null;
  private controlTarget: AiCardControlTargetImpl | null = null;
  private sessionActive: boolean = false;
  private collapseMode: CollapseMode = CollapseMode.WHOLE_COMPONENT;
  private readonly borderPos: Position = { x: 0, y: 0 };
  private dislikeDialogController: CustomDialogController | null = null;
  private animatedEndFired: boolean = false;
  private recommendFallbackId: number = -1;
  @Local displayedText: string = '';
  @Local activeQuery: string = '';
  @Local titleState: Resource = AiCardConstants.TEXT_TITLE_THINKING;
  @Local isStreaming: boolean = false;
  @Local isTerminal: boolean = false;
  @Local skeletonVisible: boolean = true;
  @Local relevantItems: AiSearchRelevantItem[] = [];
  @Local cardVisible: boolean = false;
  @Local recommendVisible: boolean = false;
  @Local stoppedByUser: boolean = false;
  @Local showMarkdown: boolean = true;
  @Local cardList: AiSearchCardRawData[] = [];
  @Local isLikeActive: boolean = false;
  @Local isDislikeActive: boolean = false;
  @Local markdownMaxWidth: number = 0;
  @Local fullText: string = '';
  @Local isCollapsed: boolean = false;
  @Local userExpanded: boolean = false;
  @Local iconBelowText: boolean = false;
  @Local iconAtStart: boolean = false;
  @Local fontSizeScale: number = 1;
  @Local titlebarBottomGap: number = 0;
  @Local isProcessingTitle: boolean = true;
  @Local cardDismissed: boolean = false;
  @Local contentError: boolean = false;
  private cardTriggered: boolean = false;
  private envCallbackId: number = -1;
  private envCallback: EnvironmentCallback | null = null;

  aboutToAppear() {
    try {
      const ctx = this.getUIContext()?.getHostContext() as common.UIAbilityContext;
      if (ctx !== null && ctx !== undefined) {
        const config: Configuration = ctx.config;
        const scale: number | undefined = config.fontSizeScale;
        this.fontSizeScale = scale !== undefined ? scale : 1;
        this.envCallback = {
          onConfigurationUpdated: (newConfig: Configuration): void => {
            const newScale: number | undefined = newConfig.fontSizeScale;
            this.fontSizeScale = newScale !== undefined ? newScale : 1;
                        console.info('AiCard fontSizeScale updated: ' + this.fontSizeScale);
          },
          onMemoryLevel: (_level: AbilityConstant.MemoryLevel): void => {
          }
        };
        this.envCallbackId = ctx.getApplicationContext().on('environment', this.envCallback);
      }
    } catch (e) {
      this.fontSizeScale = 1;
    }
    this.controlTarget = new AiCardControlTargetImpl(
      (q: string, requestOptions?: CardRequestOptions) => this.handleRequest(q, requestOptions),
      () => this.handleCancelRequest()
    );
    if (this.controller !== null && this.controlTarget !== null) {
      bindAiCardController(this.controller, this.controlTarget);
      this.boundController = this.controller;
    }
    const spt: SearchProviderType = this.searchProviderType;
    if (spt === SearchProviderType.DEFAULT) {
      return;
    }
    this.showMarkdown = this.streamConfig?.showMarkdown ?? true;
    this.collapseMode = this.displayConfig?.collapseMode ?? CollapseMode.WHOLE_COMPONENT;
    this.initDislikeController();
  }

  private createSessionController(borrowedProvider: SearchProvider | null = null,
    providerConnected: boolean = false): AiCardSessionController {
    const dc: AiCardDisplayConfig | null = this.displayConfig;
    const options: AiCardSessionControllerOptions = {
      searchProviderType: this.searchProviderType,
      requestIdGenerator: () => util.generateRandomUUID(),
      engineTickMs: AiCardConstants.STREAM_TICK_MS_DEFAULT,
      showMarkdown: this.showMarkdown,
      onAnimatedStart: () => this.handleAnimatedStart(),
      onDisplayUpdate: (t: string) => this.handleDisplayUpdate(t),
      onMarkdownEnd: (t: string) => this.handleMarkdownEnd(t),
      onCardData: (cards: AiSearchCardRawData[]) => this.handleCardData(cards),
      onRelevant: (items: AiSearchRelevantItem[]) => this.handleRelevant(items),
      onError: (e: AiSearchError) => this.handleError(e),
      onCollapse: () => {
        const cb = dc?.onCollapse;
        if (cb !== undefined) {
          cb();
        }
      },
      onExpand: () => {
        const cb = dc?.onExpand;
        if (cb !== undefined) {
          cb();
        }
      }
    };
    if (borrowedProvider !== null) {
      options.provider = borrowedProvider;
      options.providerConnected = providerConnected;
    }
    return new AiCardSessionController(options);
  }

  aboutToDisappear() {
    if (this.boundController !== null && this.controlTarget !== null) {
      unbindAiCardController(this.boundController, this.controlTarget);
    }
    this.boundController = null;
    this.controlTarget = null;
    if (this.envCallbackId !== -1) {
      try {
        const ctx = this.getUIContext()?.getHostContext() as common.UIAbilityContext;
        ctx?.getApplicationContext().off('environment', this.envCallbackId);
      } catch (e) {
      }
      this.envCallbackId = -1;
    }
    if (this.sessionController !== null) {
      this.sessionController.dispose();
      this.sessionController = null;
    }
    this.clearRecommendFallback();
  }

  private handleSkeletonShown(): void {
    this.startSession();
  }

  private startSession(): void {
    if (this.sessionActive) {
      return;
    }
    if (this.sessionController === null) {
      return;
    }
    const ctx: Context = this.getUIContext().getHostContext() as Context;
    this.sessionActive = true;
    this.sessionController.start(this.activeQuery, ctx);
  }

  private handleSkeletonWidth(width: number): void {
    this.markdownMaxWidth = width - AiCardConstants.SPACING_PADDING * 2;
    AiCardLogger.info(AiCardConstants.AICARD_DOMAIN_UI, AiCardConstants.TAG_AI_CARD,
      'handleSkeletonWidth: markdownMaxWidth=' + this.markdownMaxWidth);
    if (this.collapseMode === CollapseMode.MARKDOWN_AREA
      && this.fullText.length > 0 && !this.userExpanded && this.markdownMaxWidth > 0) {
      this.handleDisplayUpdate(this.fullText);
    }
  }

  private handleAnimatedStart(): void {
    if (this.cardDismissed) {
      return;
    }
    this.titleState = AiCardConstants.TEXT_TITLE_GENERATING;
    this.isProcessingTitle = true;
    this.isStreaming = true;
    this.isTerminal = false;
    this.stoppedByUser = false;
    this.skeletonVisible = false;
    this.contentError = false;
    const lc: AiCardLifecycleCallbacks | null = this.lifecycleCallbacks;
    if (lc !== null && lc.onAnimatedStart !== undefined) {
      lc.onAnimatedStart();
    }
  }

  private handleDisplayUpdate(text: string): void {
    if (this.cardDismissed) {
      return;
    }
    this.fullText = text;
    this.skeletonVisible = false;
    if (this.collapseMode !== CollapseMode.MARKDOWN_AREA) {
      this.displayedText = text;
      this.isCollapsed = false;
      return;
    }
    if (this.userExpanded || this.markdownMaxWidth <= 0) {
      this.displayedText = text;
      this.isCollapsed = false;
      this.iconAtStart = TextDirectionUtil.isRtlText(text);
      return;
    }
    this.iconAtStart = TextDirectionUtil.isRtlText(text);
    try {
      const result: MeasureResult = this.measureWithIcon(text);
      this.displayedText = result.displayText;
      this.isCollapsed = result.isCollapsed;
      if (result.isCollapsed && !this.cardTriggered) {
        this.cardTriggered = true;
        AiCardLogger.info(AiCardConstants.AICARD_DOMAIN_UI, AiCardConstants.TAG_AI_CARD,
          'handleDisplayUpdate: collapsed, skip typewriter to end');
        if (this.sessionController !== null) {
          this.sessionController.skipToEnd();
        }
      }
      AiCardLogger.info(AiCardConstants.AICARD_DOMAIN_UI, AiCardConstants.TAG_AI_CARD,
        'handleDisplayUpdate: measured, isCollapsed=' + result.isCollapsed
          + ' truncIdx=' + result.truncationIndex
          + ' displayLen=' + result.displayText.length
          + ' fullLen=' + text.length);
    } catch (e) {
      AiCardLogger.error(AiCardConstants.AICARD_DOMAIN_UI, AiCardConstants.TAG_AI_CARD,
        'handleDisplayUpdate: measure exception: ' + e);
      this.displayedText = text;
      this.isCollapsed = false;
    }
  }

  private handleExpandToggle(): void {
    this.userExpanded = !this.userExpanded;
    AiCardLogger.info(AiCardConstants.AICARD_DOMAIN_UI, AiCardConstants.TAG_AI_CARD,
      'handleExpandToggle: userExpanded=' + this.userExpanded);
    if (this.userExpanded) {
      this.displayedText = this.fullText;
      this.isCollapsed = false;
      this.iconAtStart = TextDirectionUtil.isRtlText(this.fullText);
      const iconSpace: number = AiCardConstants.COLLAPSE_ICON_SIZE + AiCardConstants.COLLAPSE_BUTTON_GAP;
      try {
        const info: LineMeasureInfo = this.countLinesAndLastWidth(this.fullText, this.markdownMaxWidth);
        this.iconBelowText = info.lastLineWidth + iconSpace > this.markdownMaxWidth;
        AiCardLogger.info(AiCardConstants.AICARD_DOMAIN_UI, AiCardConstants.TAG_AI_CARD,
          'handleExpandToggle: lastW=' + info.lastLineWidth + ' iconSpace=' + iconSpace
            + ' maxW=' + this.markdownMaxWidth + ' below=' + this.iconBelowText
            + ' atStart=' + this.iconAtStart);
      } catch (e) {
        this.iconBelowText = true;
      }
    } else {
      this.iconBelowText = false;
      this.handleDisplayUpdate(this.fullText);
    }
    if (this.sessionController !== null) {
      this.sessionController.notifyCollapseChanged(this.userExpanded ? false : this.isCollapsed);
    }
  }

  private countLinesAndLastWidth(text: string, maxWidth: number): LineMeasureInfo {
    const styledString: StyledString = this.buildMeasureStyledString(text);
    const paragraphs = this.getUIContext().getMeasureUtils().getParagraphs(
      styledString, { constraintWidth: LengthMetrics.vp(maxWidth) }
    );
    let totalCount: number = 0;
    let lastWidth: number = 0;
    for (let i: number = 0; i < paragraphs.length; i++) {
      const lines: number = paragraphs[i].getLineCount();
      totalCount += lines;
      if (lines > 0) {
        const metrics: LineMetrics | undefined = paragraphs[i].getLineMetrics(lines - 1);
        if (metrics !== undefined) {
          lastWidth = px2vp(metrics.width);
        }
      }
    }
    const info: LineMeasureInfo = { lineCount: totalCount, lastLineWidth: lastWidth };
    return info;
  }

  private measureWithIcon(text: string): MeasureResult {
    const maxWidth: number = this.markdownMaxWidth;
    const maxLines: number = AiCardConstants.MARKDOWN_COLLAPSE_MAX_LINES;
    const ellipsis: string = AiCardConstants.ELLIPSIS_TEXT;
    const iconSpace: number = AiCardConstants.COLLAPSE_ICON_SIZE + AiCardConstants.COLLAPSE_BUTTON_GAP;

    if (text.length === 0 || maxWidth <= 0) {
      const empty: MeasureResult = { displayText: text, isCollapsed: false, truncationIndex: -1 };
      return empty;
    }

    const info: LineMeasureInfo = this.countLinesAndLastWidth(text, maxWidth);
    const charWidthEstimate: number = AiCardConstants.FONT_SIZE_HEADER3;
    const fits: boolean = info.lineCount < maxLines
      || (info.lineCount === maxLines && info.lastLineWidth + iconSpace <= maxWidth);
    if (fits && !(info.lineCount === maxLines
      && info.lastLineWidth + iconSpace + charWidthEstimate > maxWidth)) {
      const ok: MeasureResult = { displayText: text, isCollapsed: false, truncationIndex: -1 };
      return ok;
    }

    for (let i: number = text.length - 1; i > 0; i--) {
      const truncated: string = text.substring(0, i) + ellipsis;
      const tInfo: LineMeasureInfo = this.countLinesAndLastWidth(truncated, maxWidth);
      const tFits: boolean = tInfo.lineCount < maxLines
        || (tInfo.lineCount === maxLines && tInfo.lastLineWidth + iconSpace <= maxWidth);
      if (tFits) {
        const result: MeasureResult = { displayText: truncated, isCollapsed: true, truncationIndex: i };
        return result;
      }
    }

    const fallback: MeasureResult = { displayText: text, isCollapsed: false, truncationIndex: -1 };
    return fallback;
  }

  private buildMeasureStyledString(text: string): StyledString {
    if (text.length === 0) {
      return new StyledString('', []);
    }
    const parsed: ParsedMarkdown = MarkdownParser.parse(text);
    const textLength: number = parsed.plainText.length;
    const baseFont: TextStyle = new TextStyle({
      fontSize: LengthMetrics.fp(AiCardConstants.FONT_SIZE_BODY),
      fontColor: AiCardConstants.COLOR_TEXT_PRIMARY
    });
    const lhStyle: LineHeightStyle = new LineHeightStyle(
      new LengthMetrics(AiCardConstants.LINE_HEIGHT)
    );
    const options: StyleOptions[] = [
      {
        start: 0,
        length: textLength,
        styledKey: StyledStringKey.FONT,
        styledValue: baseFont
      },
      {
        start: 0,
        length: textLength,
        styledKey: StyledStringKey.LINE_HEIGHT,
        styledValue: lhStyle
      }
    ];
    for (let i: number = 0; i < parsed.styles.length; i++) {
      const s: MarkdownStyleRange = parsed.styles[i];
      if (s.type === MarkdownStyleType.BOLD) {
        const style: TextStyle = new TextStyle({ fontWeight: FontWeight.Bold });
        options.push({
          start: s.start,
          length: s.length,
          styledKey: StyledStringKey.FONT,
          styledValue: style
        });
      } else if (s.type === MarkdownStyleType.ITALIC) {
        const style: TextStyle = new TextStyle({ fontStyle: FontStyle.Italic });
        options.push({
          start: s.start,
          length: s.length,
          styledKey: StyledStringKey.FONT,
          styledValue: style
        });
      } else if (s.type === MarkdownStyleType.CODE) {
        const style: TextStyle = new TextStyle({ fontFamily: 'monospace' });
        options.push({
          start: s.start,
          length: s.length,
          styledKey: StyledStringKey.FONT,
          styledValue: style
        });
      } else if (s.type === MarkdownStyleType.HEADER1) {
        const style: TextStyle = new TextStyle({ fontSize: LengthMetrics.fp(AiCardConstants.FONT_SIZE_HEADER1) });
        options.push({
          start: s.start,
          length: s.length,
          styledKey: StyledStringKey.FONT,
          styledValue: style
        });
      } else if (s.type === MarkdownStyleType.HEADER2) {
        const style: TextStyle = new TextStyle({ fontSize: LengthMetrics.fp(AiCardConstants.FONT_SIZE_TITLE) });
        options.push({
          start: s.start,
          length: s.length,
          styledKey: StyledStringKey.FONT,
          styledValue: style
        });
      } else if (s.type === MarkdownStyleType.HEADER3) {
        const style: TextStyle = new TextStyle({ fontSize: LengthMetrics.fp(AiCardConstants.FONT_SIZE_HEADER3) });
        options.push({
          start: s.start,
          length: s.length,
          styledKey: StyledStringKey.FONT,
          styledValue: style
        });
      }
    }
    return new StyledString(parsed.plainText, options);
  }

  private handleMarkdownEnd(text: string): void {
    if (this.cardDismissed) {
      return;
    }
    this.fullText = text;
    if (this.collapseMode === CollapseMode.MARKDOWN_AREA
      && !this.cardTriggered && !this.userExpanded && this.markdownMaxWidth > 0) {
      this.iconAtStart = TextDirectionUtil.isRtlText(text);
      try {
        const result: MeasureResult = this.measureWithIcon(text);
        this.displayedText = result.displayText;
        this.isCollapsed = result.isCollapsed;
      } catch (e) {
        AiCardLogger.error(AiCardConstants.AICARD_DOMAIN_UI, AiCardConstants.TAG_AI_CARD,
          'handleMarkdownEnd: measure exception: ' + e);
        this.displayedText = text;
        this.isCollapsed = false;
      }
    }
    this.isStreaming = false;
    this.isTerminal = true;
    this.isProcessingTitle = false;
    if (!this.stoppedByUser) {
      this.titleState = AiCardConstants.TEXT_TITLE_FOUND;
    }
    setTimeout(() => {
      this.tryFireAnimatedEnd();
    }, 0);
  }

  private recommendExpected(): boolean {
    const showRec: boolean = this.recommendConfig?.showRecommend ?? false;
    if (!showRec) {
      return false;
    }
    const configChips: CardRecommendChip[] | undefined = this.recommendConfig?.chips;
    const hasChips: boolean = (configChips !== undefined && configChips.length > 0)
      || this.relevantItems.length > 0;
    return hasChips;
  }

  private tryFireAnimatedEnd(): void {
    if (this.animatedEndFired) {
      return;
    }
    if (this.cardVisible && this.recommendVisible) {
      return;
    }
    if (this.cardVisible && !this.recommendVisible && this.recommendExpected()) {
      return;
    }
    this.fireOnAnimatedEnd();
  }

  private fireOnAnimatedEnd(): void {
    if (this.animatedEndFired) {
      return;
    }
    this.animatedEndFired = true;
    this.clearRecommendFallback();
    const lc: AiCardLifecycleCallbacks | null = this.lifecycleCallbacks;
    if (lc !== null && lc.onAnimatedEnd !== undefined) {
      lc.onAnimatedEnd();
    }
  }

  private clearRecommendFallback(): void {
    if (this.recommendFallbackId !== -1) {
      clearTimeout(this.recommendFallbackId);
      this.recommendFallbackId = -1;
    }
  }

  private handleCardData(cards: AiSearchCardRawData[]): void {
    if (this.cardDismissed) {
      return;
    }
    this.cardList = cards;
    this.cardVisible = true;
    if (this.collapseMode !== CollapseMode.WHOLE_COMPONENT) {
      this.recommendVisible = true;
    }
  }

  private getRecommendChips(): CardRecommendChip[] {
    const configChips: CardRecommendChip[] | undefined = this.recommendConfig?.chips;
    if (configChips !== undefined && configChips !== null && configChips.length > 0) {
      return configChips;
    }
    if (this.relevantItems.length > 0) {
      const chips: CardRecommendChip[] = [];
      for (const item of this.relevantItems) {
        const segment: CardRecommendSegment = {
          type: RecommendChipSegmentType.TEXT,
          text: item.title
        };
        const chip: CardRecommendChip = { segments: [segment] };
        chips.push(chip);
      }
      return chips;
    }
    return [];
  }

  private handleRelevant(items: AiSearchRelevantItem[]): void {
    if (this.cardDismissed) {
      return;
    }
    this.relevantItems = items;
  }

  private handleError(error: AiSearchError): void {
    if (this.cardDismissed) {
      return;
    }
    const wasSearching: boolean = !this.isStreaming;
    this.titleState = AiCardConstants.TEXT_TITLE_ERROR;
    this.isProcessingTitle = false;
    this.isStreaming = false;
    this.isTerminal = true;
    if (wasSearching) {
      this.skeletonVisible = false;
      this.contentError = true;
    }
    const onErrorCb: ((error: AiSearchError) => void) | null = this.onError;
    if (onErrorCb !== null) {
      onErrorCb(error);
    }
  }

  private handleCancelRequest(): void {
    this.performStop();
  }

  private performStop(): void {
    if (this.isTerminal || this.stoppedByUser || this.cardDismissed) {
      return;
    }
    const phase: CardCancelPhase = this.isStreaming ? CardCancelPhase.GENERATING : CardCancelPhase.THINKING;
    this.notifyCancel(phase);
    if (!this.isStreaming) {
      if (this.sessionController !== null) {
        this.sessionController.pause();
      }
      this.dismissCard();
      return;
    }
    this.stoppedByUser = true;
    this.titleState = AiCardConstants.TEXT_TITLE_STOPPED;
    this.isProcessingTitle = false;
    if (this.sessionController !== null) {
      this.sessionController.pause();
    }
  }

  private notifyCancel(phase: CardCancelPhase): void {
    const lc: AiCardLifecycleCallbacks | null = this.lifecycleCallbacks;
    if (lc !== null && lc.onCancel !== undefined) {
      lc.onCancel(phase);
    }
  }

  private handleRequest(query: string, _requestOptions?: CardRequestOptions): void {
    const q: string = query.trim();
    if (q.length === 0) {
      AiCardLogger.warn(AiCardConstants.AICARD_DOMAIN_UI, AiCardConstants.TAG_AI_CARD,
        'handleRequest: empty query, no-op');
      return;
    }
    if (this.searchProviderType === SearchProviderType.DEFAULT) {
      AiCardLogger.warn(AiCardConstants.AICARD_DOMAIN_UI, AiCardConstants.TAG_AI_CARD,
        'handleRequest: DEFAULT provider type, no-op');
      return;
    }
    let borrowed: SearchProvider | null = null;
    if (this.sessionController !== null && !this.sessionController.isFinalized()) {
      this.sessionController.pause(true);
      borrowed = this.sessionController.getDetachedProvider();
    }
    this.activeQuery = q;
    this.resetForNewSession();
    this.sessionController = this.createSessionController(borrowed, borrowed !== null);
    this.startSession();
  }

  private resetCardState(showSkeleton: boolean): void {
    this.titleState = AiCardConstants.TEXT_TITLE_THINKING;
    this.isStreaming = false;
    this.isTerminal = false;
    this.isProcessingTitle = true;
    this.stoppedByUser = false;
    this.skeletonVisible = showSkeleton;
    this.displayedText = '';
    this.fullText = '';
    this.cardVisible = false;
    this.recommendVisible = false;
    this.relevantItems = [];
    this.cardList = [];
    this.isCollapsed = false;
    this.userExpanded = false;
    this.iconBelowText = false;
    this.iconAtStart = false;
    this.isLikeActive = false;
    this.isDislikeActive = false;
    this.contentError = false;
    this.cardTriggered = false;
    this.animatedEndFired = false;
    this.clearRecommendFallback();
  }

  private dismissCard(): void {
    this.cardDismissed = true;
    this.resetCardState(false);
  }

  private resetForNewSession(): void {
    this.cardDismissed = false;
    this.resetCardState(true);
    this.sessionActive = false;
  }

  private initDislikeController(): void {
    this.dislikeDialogController = new CustomDialogController({
      builder: () => {
        this.DislikeContentBuilder();
      },
      maskColor: $r('sys.color.mask_fourth'),
      alignment: DialogAlignment.Center,
      autoCancel: true,
      isModal: true,
      width: AiCardConstants.DIALOG_MAX_WIDTH
    });
  }

  @Builder
  DislikeContentBuilder() {
    Column() {
      DislikeDialog({
        dislikeTags: this.complianceConfig?.dislikeTags ?? [],
        fontSizeScale: this.fontSizeScale,
        onConfirm: (selectedTags: string[], description: string) => {
          this.handleDislikeConfirm(selectedTags, description);
        },
        onCancel: () => {
        },
        onClose: () => {
          this.closeDislikeDialog();
        }
      })
    }
    .width('100%')
    .margin({
      left: AiCardConstants.DIALOG_MARGIN_HORIZONTAL,
      right: AiCardConstants.DIALOG_MARGIN_HORIZONTAL
    })
    .padding({
      left: AiCardConstants.DIALOG_PADDING_HORIZONTAL,
      right: AiCardConstants.DIALOG_PADDING_HORIZONTAL
    });
  }

  private closeDislikeDialog(): void {
    this.dislikeDialogController?.close();
  }

  private handleCopy(): void {
    const text: string = this.displayedText;
    const pb = pasteboard.getSystemPasteboard();
    const data = pasteboard.createData(pasteboard.MIMETYPE_TEXT_PLAIN, text);
    pb.setDataSync(data);
    promptAction.showToast({
      message: $r('app.string.markdown_copied')
    });
    const cb = this.complianceConfig?.onCopy;
    if (cb !== undefined) {
      cb(text);
    }
  }

  private handleLikeToggle(): void {
    this.isLikeActive = !this.isLikeActive;
    const cb = this.complianceConfig?.onLike;
    if (cb !== undefined) {
      cb(this.isLikeActive);
    }
  }

  private handleDislikeToggle(): void {
    if (this.isDislikeActive) {
      this.isDislikeActive = false;
      const feedback: AiSearchDislikeFeedback = { tags: [], description: '' };
      const cb = this.complianceConfig?.onDislike;
      if (cb !== undefined) {
        cb(feedback, false);
      }
    } else {
      if (this.dislikeDialogController !== null) {
        this.dislikeDialogController.open();
      }
    }
  }

  private handleDislikeConfirm(selectedTags: string[], description: string): void {
    this.isDislikeActive = true;
    const feedback: AiSearchDislikeFeedback = { tags: selectedTags, description: description };
    const cb = this.complianceConfig?.onDislike;
    if (cb !== undefined) {
      cb(feedback, true);
    }
  }

  @Builder
  DefaultCard(_: AiSearchCardDataPayload, _index: number) {
  }

  @Builder
  CardContent() {
    TitleBar({
      title: this.titleState,
      isTerminal: this.isTerminal,
      stoppedByUser: this.stoppedByUser,
      fontSizeScale: this.fontSizeScale,
      titleClip: this.isProcessingTitle,
      onStop: () => this.performStop(),
      onBottomGapChanged: (gap: number) => {
        this.titlebarBottomGap = gap;
      }
    });
    if (this.contentError) {
      ErrorPlaceholder({
        fontSizeScale: this.fontSizeScale,
        titlebarBottomGap: this.titlebarBottomGap
      });
    } else if (this.skeletonVisible) {
      SkeletonLoader({
        onSkeletonShown: () => this.handleSkeletonShown(),
        onWidthChanged: (w: number) => this.handleSkeletonWidth(w)
      });
    } else {
      ContentView({
        markdownText: this.displayedText,
        showMarkdown: this.showMarkdown,
        collapseMode: this.collapseMode,
        cardVisible: this.cardVisible,
        cardList: this.cardList,
        cardContent: this.cardContent,
        isCollapsed: this.isCollapsed,
        userExpanded: this.userExpanded,
        iconBelowText: this.iconBelowText,
        iconAtStart: this.iconAtStart,
        fontSizeScale: this.fontSizeScale,
        onExpandToggle: () => this.handleExpandToggle(),
        onCardFinish: () => this.tryFireAnimatedEnd(),
        onToggleAnimationEnd: () => {
          this.recommendVisible = true;
          this.clearRecommendFallback();
        }
      });
    }
    RecommendZone({
      title: this.recommendConfig?.title ?? AiCardConstants.TEXT_RECOMMEND_TITLE,
      chips: this.getRecommendChips(),
      maxChips: this.recommendConfig?.maxChips ?? AiCardConstants.RECOMMEND_CHIP_MAX_DEFAULT,
      showRecommend: this.recommendConfig?.showRecommend ?? false,
      visible: this.recommendVisible,
      fontSizeScale: this.fontSizeScale,
      onRecommendAnimEnd: () => this.tryFireAnimatedEnd(),
      onRecommendClick: (index: number, content: string) => {
        const cb = this.recommendConfig?.onRecommendClick;
        if (cb !== undefined) {
          cb(index, content);
        }
      }
    });
    ComplianceZone({
      showCopy: this.complianceConfig?.showCopy ?? true,
      showFeedbackButtons: this.complianceConfig?.showFeedbackButtons ?? true,
      markdownText: this.displayedText,
      isLikeActive: this.isLikeActive,
      isDislikeActive: this.isDislikeActive,
      interactive: this.recommendVisible,
      fontSizeScale: this.fontSizeScale,
      onCopy: () => this.handleCopy(),
      onLikeToggle: () => this.handleLikeToggle(),
      onDislikeToggle: () => this.handleDislikeToggle()
    });
  }

  build() {
    Column() {
      if (!this.cardDismissed) {
        Column() {
          this.CardContent();
        }
        .clip(true)
        .width('100%')
        .padding(AiCardConstants.GRADIENT_BORDER_WIDTH)
        .backgroundColor(this.displayConfig?.cardBackgroundColor
          ?? $r('sys.color.comp_background_list_card'))
        .borderRadius(AiCardConstants.CARD_BORDER_RADIUS)

        Stack() {
          Column() {
          }
          .width('100%')
          .height(LayoutPolicy.matchParent)
          .backgroundColor(Color.Black)
          .borderRadius(AiCardConstants.GRADIENT_BORDER_RADIUS - AiCardConstants.GRADIENT_BORDER_WIDTH)
          .blendMode(BlendMode.DST_OUT, BlendApplyType.OFFSCREEN)
        }
        .height(LayoutPolicy.matchParent)
        .position(this.borderPos)
        .width('100%')
        .padding(AiCardConstants.GRADIENT_BORDER_WIDTH)
        .linearGradient({
          angle: AiCardConstants.GRADIENT_INITIAL_ANGLE,
          colors: AiCardConstants.GRADIENT_COLORS,
          repeating: false
        })
        .borderRadius(AiCardConstants.CARD_BORDER_RADIUS)
        .blendMode(BlendMode.SRC_OVER, BlendApplyType.OFFSCREEN)
        .hitTestBehavior(HitTestMode.None)
      }
    }
    .height(LayoutPolicy.wrapContent)
    .width('100%')

  }
}
