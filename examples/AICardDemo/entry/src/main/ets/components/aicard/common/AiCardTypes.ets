/**
 * AiCard 公共类型定义。
 * 行为口径以 spec.md 为真相源，命名修正对照见 design.md 附录 A。
 * @since AICardDemo-v1.0
 */

export interface AiCardStreamConfig {
  showMarkdown?: boolean;
}

export interface AiCardDisplayConfig {
  collapseMode?: CollapseMode;
  cardBackgroundColor?: ResourceColor;
  onCollapse?: () => void;
  onExpand?: () => void;
}

export enum RecommendChipSegmentType {
  TEXT = 'Text',
  SYMBOL = 'Symbol',
}

export interface CardRecommendSegment {
  type: RecommendChipSegmentType;
  text?: ResourceStr;
  symbol?: Resource;
}

export interface CardRecommendChip {
  segments: CardRecommendSegment[];
}

export interface CardRecommendConfig {
  showRecommend?: boolean;
  title?: ResourceStr;
  maxChips?: number;
  chips?: CardRecommendChip[];
  onRecommendClick?: (index: number, content: string) => void;
}

export enum CardCancelPhase {
  THINKING = 0,
  GENERATING = 1
}

export interface AiCardLifecycleCallbacks {
  onAnimatedStart?: () => void;
  onAnimatedEnd?: () => void;
  onCancel?: (phase: CardCancelPhase) => void;
}

export enum ThemeColorMode {
  LIGHT = 0,
  DARK = 1
}

export enum CollapseMode {
  WHOLE_COMPONENT = 0,
  MARKDOWN_AREA = 1,
  NONE = 2
}

export interface AiSearchComplianceConfig {
  showCopy?: boolean;
  showFeedbackButtons?: boolean;
  dislikeTags?: string[];
  onLike?: (active: boolean) => void;
  onDislike?: (feedback: AiSearchDislikeFeedback, active: boolean) => void;
  onCopy?: (text: string) => void;
}

export interface AiSearchDislikeFeedback {
  tags: string[];
  description: string;
}

export interface AiSearchCardDataPayload {
  streamingText: string;
  cards: AiSearchCardRawData[];
}

export interface AiSearchCardRawData {
  type: string;
  title?: string;
  data: Record<string, Object>;
  iconUrl?: string;
}

export interface AiSearchRelevantItem {
  title: string;
  iconUrl?: string;
}

export interface SearchProvider {
  connect(context: Context): Promise<void>;
  search(request: SearchRequest, callback: SearchCallback): void;
  cancel(requestId: string): void;
  release(context: Context): void;
}

export interface SearchRequest {
  requestId: string;
  query: string;
  model?: string;
  scene?: string;
  thinkMode?: string;
  referenceType?: string;
}

export interface SearchCallback {
  onResult(result: SearchResult): void;
  onError(error: SearchError): void;
}

export interface SearchResult {
  requestId: string;
  status: number;
  isFinal: boolean;
  streamingText: string;
  reference?: Object[];
  relevant?: AiSearchRelevantItem[];
  cards?: AiSearchCardRawData[];
  index: number;
}

export interface SearchError {
  code: number;
  message: string;
}

export type AiSearchStatus = 'idle' | 'searching' | 'streaming' | 'completed' | 'error';

export type StreamStatus = 'START' | 'PARTIAL' | 'END';

export interface AiSearchError {
  code: AiSearchErrorCode;
  message: string;
}

export type AiSearchErrorCode = 1001 | 1002 | 1003 | 1004;

/**
 * request() 可选请求选项：空接口预留扩展位（本版本无字段）。
 * 为后续请求级配置（如超时覆盖等）演进预留；新增字段属向前兼容演进，不改变方法签名。
 * @since AICardDemo-v1.2
 */
export interface CardRequestOptions {
}

export enum MockScenario {
  NORMAL,
  OUT_OF_ORDER,
  GROUNDING,
  IS_FINAL,
  FIRST_PACKET_TIMEOUT,
  STREAM_GAP_TIMEOUT,
  CONNECT_FAIL,
  PROVIDER_ERROR,
  CANCEL_RESPONSIVE
}

export interface MockOptions {
  firstPacketSilentMs?: number;
  streamGapSilentMs?: number;
  chunkDelayMs?: number;
  cancelCalled?: (requestId: string) => void;
  fixture?: TestScenarioFixture;
}

export enum SearchProviderType {
  DEFAULT = 0,
  TEST = 1
}

export interface TestScenarioFixture {
  connectFail?: boolean;
  actions: TestFixtureAction[];
}

export interface TestFixtureAction {
  result?: TestFixtureResult;
  error?: SearchError;
}

export interface TestFixtureResult {
  index: number;
  status: number;
  isFinal: boolean;
  streamingText: string;
  relevant?: AiSearchRelevantItem[];
  cards?: AiSearchCardRawData[];
}

export interface MeasureResult {
  displayText: string;
  isCollapsed: boolean;
  truncationIndex: number;
}

export interface LineMeasureInfo {
  lineCount: number;
  lastLineWidth: number;
}
