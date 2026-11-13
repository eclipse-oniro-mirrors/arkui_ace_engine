import { AiCardConstants } from '../common/AiCardConstants';
import { AiCardLogger } from '../common/AiCardLogger';
import {
  AiSearchCardDataPayload, AiSearchCardRawData, AiSearchError, AiSearchErrorCode,
  AiSearchRelevantItem, AiSearchStatus, SearchCallback, SearchProvider, SearchProviderType, SearchRequest,
  SearchError, SearchResult, StreamStatus
} from '../common/AiCardTypes';
import { StreamEngine } from '../engine/StreamEngine';
import { TimeoutManager } from '../engine/TimeoutManager';
import { AiCardStateMachine } from '../engine/AiCardStateMachine';
import { TestSearchProvider } from '../mocks/TestSearchProvider';

type AiCardTerminal = 'completed' | 'error' | 'stopped';

export interface AiCardSessionControllerOptions {
  provider?: SearchProvider;
  searchProviderType?: SearchProviderType;
  requestIdGenerator: () => string;
  engineTickMs?: number;
  showMarkdown?: boolean;
  firstPacketMs?: number;
  streamGapMs?: number;
  /** v1.2 连接复用：注入的 provider 已建立连接（start 跳过 connect 直发 search）。 */
  providerConnected?: boolean;
  onAnimatedStart?: () => void;
  onDisplayUpdate?: (text: string) => void;
  onMarkdownEnd?: (fullText: string) => void;
  onCardData?: (cards: AiSearchCardRawData[]) => void;
  onRelevant?: (items: AiSearchRelevantItem[]) => void;
  onError?: (error: AiSearchError) => void;
  onCollapse?: () => void;
  onExpand?: () => void;
}

export class AiCardSessionController implements SearchCallback {
  private readonly injectedProvider: SearchProvider | null;
  private readonly providerType: SearchProviderType;
  private provider: SearchProvider | null = null;
  private readonly requestIdGenerator: () => string;
  private readonly engine: StreamEngine;
  private readonly timeoutManager: TimeoutManager;
  private readonly stateMachine: AiCardStateMachine;

  private readonly onAnimatedStart?: () => void;
  private readonly onDisplayUpdate?: (text: string) => void;
  private readonly onMarkdownEnd?: (fullText: string) => void;
  private readonly onCardData?: (cards: AiSearchCardRawData[]) => void;
  private readonly onRelevant?: (items: AiSearchRelevantItem[]) => void;
  private readonly onErrorCallback?: (error: AiSearchError) => void;
  private readonly onCollapse?: () => void;
  private readonly onExpand?: () => void;

  private requestId: string = '';
  private context: Context | null = null;
  private finalized: boolean = false;
  private firstPacketReceived: boolean = false;
  private lastCards: AiSearchCardRawData[] = [];
  private lastRelevant: AiSearchRelevantItem[] = [];
  private collapsed: boolean = false;
  private connected: boolean = false;
  private readonly preConnected: boolean;

  constructor(options: AiCardSessionControllerOptions) {
    this.injectedProvider = options.provider !== undefined ? options.provider : null;
    this.providerType = options.searchProviderType !== undefined ? options.searchProviderType : SearchProviderType.DEFAULT;
    this.requestIdGenerator = options.requestIdGenerator;
    this.preConnected = options.providerConnected === true;
    this.onAnimatedStart = options.onAnimatedStart;
    this.onDisplayUpdate = options.onDisplayUpdate;
    this.onMarkdownEnd = options.onMarkdownEnd;
    this.onCardData = options.onCardData;
    this.onRelevant = options.onRelevant;
    this.onErrorCallback = options.onError;
    this.onCollapse = options.onCollapse;
    this.onExpand = options.onExpand;

    const tickMs: number = options.engineTickMs ?? AiCardConstants.STREAM_TICK_MS_DEFAULT;
    const showMarkdown: boolean = options.showMarkdown ?? true;
    this.engine = new StreamEngine(tickMs, showMarkdown, (s: StreamStatus, t: string) => {
      this.handleStreamEvent(s, t);
    });
    this.timeoutManager = new TimeoutManager(
      () => { this.finalize('error', AiCardConstants.ERROR_CODE_FIRST_PACKET_TIMEOUT); },
      () => { this.finalize('error', AiCardConstants.ERROR_CODE_STREAM_GAP_TIMEOUT); },
      options.firstPacketMs,
      options.streamGapMs
    );
    this.stateMachine = new AiCardStateMachine((_status: AiSearchStatus) => {});
  }

  start(query: string, context?: Context): void {
    if (this.finalized) {
      return;
    }
    const q: string = query.trim();
    if (q.length === 0) {
      AiCardLogger.warn(AiCardConstants.AICARD_DOMAIN_CONTROLLER, AiCardConstants.TAG_CONTROLLER, 'empty query, stay idle');
      return;
    }
    this.context = context ?? null;
    if (this.preConnected && this.injectedProvider !== null) {
      this.provider = this.injectedProvider;
      this.connected = true;
      this.requestId = this.requestIdGenerator();
      AiCardLogger.info(AiCardConstants.AICARD_DOMAIN_CONTROLLER, AiCardConstants.TAG_CONTROLLER,
        'reuse connection ' + this.requestId);
      this.afterConnect(q);
      return;
    }
    if (this.injectedProvider !== null) {
      this.provider = this.injectedProvider;
    } else if (this.providerType === SearchProviderType.TEST) {
      this.provider = new TestSearchProvider();
    } else {
      AiCardLogger.warn(AiCardConstants.AICARD_DOMAIN_CONTROLLER, AiCardConstants.TAG_CONTROLLER, 'DEFAULT provider type, no start');
      return;
    }
    this.requestId = this.requestIdGenerator();
    AiCardLogger.info(AiCardConstants.AICARD_DOMAIN_CONTROLLER, AiCardConstants.TAG_CONTROLLER, 'connect ' + this.requestId);
    this.provider.connect(this.context!).then(() => {
      this.afterConnect(q);
    }).catch(() => {
      this.finalize('error', AiCardConstants.ERROR_CODE_CONNECT_FAILED);
    });
  }

  private afterConnect(query: string): void {
    if (this.finalized) {
      return;
    }
    if (this.provider === null) {
      return;
    }
    this.connected = true;
    this.stateMachine.toSearching();
    AiCardLogger.info(AiCardConstants.AICARD_DOMAIN_CONTROLLER, AiCardConstants.TAG_CONTROLLER, 'search ' + this.requestId);
    this.timeoutManager.startFirstPacket();
    const request: SearchRequest = {
      requestId: this.requestId,
      query: query,
      model: 'CLOUD',
      referenceType: '1'
    };
    this.provider.search(request, this);
  }

  onResult(result: SearchResult): void {
    this.handleResult(result);
  }

  onError(error: SearchError): void {
    this.handleProviderError();
  }

  private handleResult(result: SearchResult): void {
    if (this.finalized) {
      return;
    }
    if (this.engine.isGrounded() && result.status === 17) {
      return;
    }
    this.timeoutManager.stopFirstPacket();
    if (!this.firstPacketReceived) {
      this.firstPacketReceived = true;
      this.stateMachine.toStreaming();
      AiCardLogger.info(AiCardConstants.AICARD_DOMAIN_CONTROLLER, AiCardConstants.TAG_CONTROLLER, 'first packet');
    }
    this.engine.push(result);
    if (this.finalized) {
      return;
    }
    if (result.isFinal || this.engine.isGrounded()) {
      this.timeoutManager.stopStreamGap();
    } else {
      this.timeoutManager.resetStreamGap();
    }
    if (result.isFinal) {
      this.lastCards = result.cards ?? [];
      this.lastRelevant = result.relevant ?? [];
      AiCardLogger.info(AiCardConstants.AICARD_DOMAIN_CONTROLLER, AiCardConstants.TAG_CONTROLLER, 'isFinal');
      if (this.onRelevant) {
        this.onRelevant(this.lastRelevant);
      }
    }
  }

  private handleProviderError(): void {
    this.finalize('error', AiCardConstants.ERROR_CODE_PROVIDER_ERROR);
  }

  private handleStreamEvent(status: StreamStatus, displayedText: string): void {
    if (this.finalized) {
      return;
    }
    if (status === 'START') {
      if (this.onAnimatedStart) {
        this.onAnimatedStart();
      }
    } else if (status === 'PARTIAL') {
      if (this.onDisplayUpdate) {
        this.onDisplayUpdate(displayedText);
      }
    } else if (status === 'END') {
      if (this.onMarkdownEnd) {
        this.onMarkdownEnd(displayedText);
      }
      if (this.onCardData && this.lastCards.length > 0) {
        this.onCardData(this.lastCards);
      }
      this.finalize('completed');
    }
  }

  pause(keepConnection?: boolean): void {
    this.finalize('stopped', undefined, keepConnection === true);
  }

  skipToEnd(): void {
    if (this.finalized) {
      return;
    }
    this.engine.skipToEnd();
  }

  notifyCollapseChanged(collapsed: boolean): void {
    if (collapsed === this.collapsed) {
      return;
    }
    this.collapsed = collapsed;
    if (collapsed) {
      if (this.onCollapse) {
        this.onCollapse();
      }
    } else {
      if (this.onExpand) {
        this.onExpand();
      }
    }
  }

  /** v1.2 连接复用：取走已连接 provider 的所有权（引用置 null，旧会话后续 release/dispose 对 null 安全早退）。仅已连接且未释放时可取。 */
  getDetachedProvider(): SearchProvider | null {
    if (!this.connected || this.provider === null) {
      return null;
    }
    const detached: SearchProvider = this.provider;
    this.provider = null;
    this.connected = false;
    return detached;
  }

  dispose(): void {
    if (this.finalized) {
      this.releaseProvider();
      return;
    }
    this.engine.stop();
    this.timeoutManager.stopAll();
    this.releaseProvider();
    this.finalized = true;
    AiCardLogger.info(AiCardConstants.AICARD_DOMAIN_CONTROLLER, AiCardConstants.TAG_CONTROLLER, 'dispose');
  }

  getStatus(): AiSearchStatus {
    return this.stateMachine.getStatus();
  }

  isFinalized(): boolean {
    return this.finalized;
  }

  private finalize(terminal: AiCardTerminal, errorCode?: AiSearchErrorCode, keepConnection?: boolean): void {
    if (this.finalized) {
      return;
    }
    this.finalized = true;
    if (terminal === 'completed') {
      this.stateMachine.toCompleted();
    } else if (terminal === 'error') {
      this.stateMachine.toError();
    } else {
      this.stateMachine.toStopped();
    }
    this.engine.stop();
    this.timeoutManager.stopAll();
    if (terminal === 'stopped' && this.requestId.length > 0 && this.provider !== null) {
      try {
        this.provider.cancel(this.requestId);
      } catch (e) {
      }
    }
    const keep: boolean = terminal === 'stopped' && keepConnection === true && this.connected;
    if (!keep) {
      this.releaseProvider();
    }
    if (terminal === 'error' && errorCode !== undefined) {
      if (this.onErrorCallback) {
        const err: AiSearchError = { code: errorCode, message: this.errorMessage(errorCode) };
        this.onErrorCallback(err);
      }
    }
    AiCardLogger.info(AiCardConstants.AICARD_DOMAIN_CONTROLLER, AiCardConstants.TAG_CONTROLLER,
      'finalize ' + terminal + (keep ? ' keepConnection' : ''));
  }

  private releaseProvider(): void {
    if (this.provider === null) {
      return;
    }
    try {
      this.provider.release(this.context!);
    } catch (e) {
    }
    this.provider = null;
    this.connected = false;
  }

  private errorMessage(code: number): string {
    switch (code) {
      case AiCardConstants.ERROR_CODE_CONNECT_FAILED:
        return AiCardConstants.ERROR_MSG_CONNECT_FAILED;
      case AiCardConstants.ERROR_CODE_FIRST_PACKET_TIMEOUT:
        return AiCardConstants.ERROR_MSG_FIRST_PACKET_TIMEOUT;
      case AiCardConstants.ERROR_CODE_STREAM_GAP_TIMEOUT:
        return AiCardConstants.ERROR_MSG_STREAM_GAP_TIMEOUT;
      default:
        return AiCardConstants.ERROR_MSG_PROVIDER_ERROR;
    }
  }
}
