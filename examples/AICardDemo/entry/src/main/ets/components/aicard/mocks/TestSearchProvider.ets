import { util } from '@kit.ArkTS';
import {
  MockOptions,
  SearchCallback, SearchProvider, SearchRequest, SearchResult,
  TestFixtureAction, TestFixtureResult, TestScenarioFixture
} from '../common/AiCardTypes';

const FIXTURE_PATH: string = 'test/active.json';
const DEFAULT_CHUNK_DELAY: number = 200;
const REQUEST_ID: string = 'test';

export class TestSearchProvider implements SearchProvider {
  private options: MockOptions | null;
  private fixture: TestScenarioFixture | null = null;
  private ctx: Context | null = null;
  private timers: number[] = [];
  private cancelled: boolean = false;
  private callback: SearchCallback | null = null;

  constructor(options?: MockOptions) {
    this.options = options !== undefined ? options : null;
  }

  async connect(context: Context): Promise<void> {
    this.ctx = context;
    const fx: TestScenarioFixture = await this.ensureFixture();
    if (fx.connectFail === true) {
      throw new Error('test connect fail');
    }
  }

  search(request: SearchRequest, callback: SearchCallback): void {
    this.cancelled = false;
    this.callback = callback;
    const fx: TestScenarioFixture | null = this.fixture;
    if (fx !== null && fx.connectFail === true) {
      return;
    }
    this.emit();
  }

  cancel(requestId: string): void {
    this.cancelled = true;
    this.clearTimers();
    const cb = this.options !== null ? this.options.cancelCalled : undefined;
    if (cb !== undefined && cb !== null) {
      cb(requestId);
    }
  }

  release(context: Context): void {
    this.cancelled = true;
    this.clearTimers();
    this.callback = null;
  }

  private async ensureFixture(): Promise<TestScenarioFixture> {
    if (this.fixture !== null) {
      return this.fixture;
    }
    if (this.options !== null && this.options.fixture !== undefined) {
      this.fixture = this.options.fixture;
      return this.fixture;
    }
    if (this.ctx === null) {
      throw new Error('no context and no fixture');
    }
    const bytes: Uint8Array = await this.ctx.resourceManager.getRawFileContent(FIXTURE_PATH);
    const decoder: util.TextDecoder = new util.TextDecoder('utf-8');
    const text: string = decoder.decode(bytes);
    const parsed: Object = JSON.parse(text);
    this.fixture = parsed as TestScenarioFixture;
    return this.fixture;
  }

  private emit(): void {
    const fx: TestScenarioFixture | null = this.fixture;
    if (fx === null) {
      return;
    }
    const actions: TestFixtureAction[] = fx.actions;
    const delay: number = (this.options !== null && this.options.chunkDelayMs !== undefined)
      ? this.options.chunkDelayMs : DEFAULT_CHUNK_DELAY;
    let t: number = 0;
    for (const a of actions) {
      t += delay;
      const action: TestFixtureAction = a;
      const id: number = setTimeout(() => {
        if (this.cancelled || this.callback === null) {
          return;
        }
        if (action.error !== undefined && action.error !== null) {
          this.callback.onError(action.error);
        } else if (action.result !== undefined && action.result !== null) {
          this.callback.onResult(this.toSearchResult(action.result));
        }
      }, t);
      this.timers.push(id);
    }
  }

  private toSearchResult(r: TestFixtureResult): SearchResult {
    const out: SearchResult = {
      requestId: REQUEST_ID,
      status: r.status,
      isFinal: r.isFinal,
      streamingText: r.streamingText,
      index: r.index,
      relevant: r.relevant,
      cards: r.cards
    };
    return out;
  }

  private clearTimers(): void {
    for (const id of this.timers) {
      clearTimeout(id);
    }
    this.timers = [];
  }
}
