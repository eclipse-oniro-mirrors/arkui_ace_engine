import { describe, it, expect } from '@ohos/hypium';
import { AiCardSessionController } from '../main/ets/components/aicard/controller/AiCardSessionController';
import { TestSearchProvider } from '../main/ets/components/aicard/mocks/TestSearchProvider';
import {
  AiSearchCardRawData, AiSearchError, AiSearchRelevantItem,
  MockOptions, SearchCallback, SearchError, SearchProvider, SearchRequest, SearchResult,
  TestFixtureAction, TestFixtureResult, TestScenarioFixture
} from '../main/ets/components/aicard/common/AiCardTypes';

function makeResult(index: number, text: string, status: number = 17, isFinal: boolean = false,
  relevant: AiSearchRelevantItem[] = [], cards: AiSearchCardRawData[] = []): SearchResult {
  return {
    requestId: 'r',
    status: status,
    isFinal: isFinal,
    streamingText: text,
    index: index,
    relevant: relevant,
    cards: cards
  };
}

interface ScriptedAction {
  result?: SearchResult;
  error?: SearchError;
  delay: number;
}

class FakeProvider implements SearchProvider {
  connectFail: boolean = false;
  scripted: ScriptedAction[] = [];
  connectCount: number = 0;
  searchCount: number = 0;
  cancelCount: number = 0;
  releaseCount: number = 0;
  private cb: SearchCallback | null = null;
  private timers: number[] = [];

  connect(context: Context): Promise<void> {
    this.connectCount++;
    if (this.connectFail) {
      return Promise.reject(new Error('connect fail'));
    }
    return Promise.resolve();
  }

  search(request: SearchRequest, callback: SearchCallback): void {
    this.searchCount++;
    this.cb = callback;
    let t: number = 0;
    for (const a of this.scripted) {
      t += a.delay;
      const action: ScriptedAction = a;
      const id: number = setTimeout(() => {
        if (this.cb === null) {
          return;
        }
        if (action.error !== undefined) {
          this.cb.onError(action.error);
        } else if (action.result !== undefined) {
          this.cb.onResult(action.result);
        }
      }, t);
      this.timers.push(id);
    }
  }

  cancel(requestId: string): void {
    this.cancelCount++;
  }

  release(context: Context): void {
    this.releaseCount++;
  }

  cleanup(): void {
    for (const id of this.timers) {
      clearTimeout(id);
    }
    this.timers = [];
    this.cb = null;
  }
}

class ResultCollector implements SearchCallback {
  results: SearchResult[] = [];
  errors: SearchError[] = [];
  onResult(r: SearchResult): void { this.results.push(r); }
  onError(e: SearchError): void { this.errors.push(e); }
}

function nullContext(): Context {
  const c: Context | null = null;
  return c!;
}

function waitFor(cond: () => boolean, done: () => void, timeoutMs: number = 2000): void {
  const start: number = Date.now();
  const h: number = setInterval(() => {
    if (cond() || Date.now() - start > timeoutMs) {
      clearInterval(h);
      done();
    }
  }, 5);
}

export default function controllerTest() {
  describe('AiCardSessionController', () => {
    it('empty_query_stays_idle', 0, () => {
      const p: FakeProvider = new FakeProvider();
      const c: AiCardSessionController = new AiCardSessionController({
        provider: p,
        requestIdGenerator: () => 'id'
      });
      c.start('');
      expect(p.connectCount).assertEqual(0);
      expect(c.getStatus()).assertEqual('idle');
      c.dispose();
    });

    it('connect_success_starts_search', 0, (done: Function) => {
      const p: FakeProvider = new FakeProvider();
      p.scripted = [];
      const c: AiCardSessionController = new AiCardSessionController({
        provider: p,
        requestIdGenerator: () => 'rid-1',
        firstPacketMs: 5000,
        streamGapMs: 5000
      });
      c.start('hello');
      waitFor(() => p.searchCount > 0, () => {
        expect(p.connectCount).assertEqual(1);
        expect(p.searchCount).assertEqual(1);
        expect(c.getStatus()).assertEqual('searching');
        c.dispose();
        p.cleanup();
        done();
      });
    });

    it('connect_fail_emits_E1', 0, (done: Function) => {
      const p: FakeProvider = new FakeProvider();
      p.connectFail = true;
      let errCode: number = 0;
      const c: AiCardSessionController = new AiCardSessionController({
        provider: p,
        requestIdGenerator: () => 'rid-2',
        onError: (e: AiSearchError) => { errCode = e.code; }
      });
      c.start('hello');
      waitFor(() => c.isFinalized(), () => {
        expect(errCode).assertEqual(1001);
        expect(p.releaseCount).assertEqual(1);
        expect(c.getStatus()).assertEqual('error');
        p.cleanup();
        done();
      });
    });

    it('first_packet_timeout_emits_E2', 0, (done: Function) => {
      const p: FakeProvider = new FakeProvider();
      p.scripted = [];
      let errCode: number = 0;
      const c: AiCardSessionController = new AiCardSessionController({
        provider: p,
        requestIdGenerator: () => 'rid-3',
        firstPacketMs: 50,
        streamGapMs: 5000,
        onError: (e: AiSearchError) => { errCode = e.code; }
      });
      c.start('hello');
      waitFor(() => c.isFinalized(), () => {
        expect(errCode).assertEqual(1002);
        expect(p.releaseCount).assertEqual(1);
        expect(c.getStatus()).assertEqual('error');
        p.cleanup();
        done();
      });
    });

    it('stream_gap_timeout_emits_E3', 0, (done: Function) => {
      const p: FakeProvider = new FakeProvider();
      p.scripted = [{ result: makeResult(0, 'A'), delay: 2 }];
      let errCode: number = 0;
      const c: AiCardSessionController = new AiCardSessionController({
        provider: p,
        requestIdGenerator: () => 'rid-4',
        engineTickMs: 2,
        firstPacketMs: 5000,
        streamGapMs: 60,
        onError: (e: AiSearchError) => { errCode = e.code; }
      });
      c.start('hello');
      waitFor(() => c.isFinalized(), () => {
        expect(errCode).assertEqual(1003);
        expect(p.releaseCount).assertEqual(1);
        expect(c.getStatus()).assertEqual('error');
        p.cleanup();
        done();
      });
    });

    it('provider_error_emits_E4', 0, (done: Function) => {
      const p: FakeProvider = new FakeProvider();
      p.scripted = [{ error: { code: 999, message: 'boom' }, delay: 2 }];
      let errCode: number = 0;
      const c: AiCardSessionController = new AiCardSessionController({
        provider: p,
        requestIdGenerator: () => 'rid-5',
        firstPacketMs: 5000,
        streamGapMs: 5000,
        onError: (e: AiSearchError) => { errCode = e.code; }
      });
      c.start('hello');
      waitFor(() => c.isFinalized(), () => {
        expect(errCode).assertEqual(1004);
        expect(p.releaseCount).assertEqual(1);
        expect(c.getStatus()).assertEqual('error');
        p.cleanup();
        done();
      });
    });

    it('error_dedup_release_once', 0, (done: Function) => {
      const p: FakeProvider = new FakeProvider();
      p.scripted = [{ error: { code: 999, message: 'boom' }, delay: 80 }];
      let errorCount: number = 0;
      const c: AiCardSessionController = new AiCardSessionController({
        provider: p,
        requestIdGenerator: () => 'rid-6',
        firstPacketMs: 50,
        streamGapMs: 5000,
        onError: () => { errorCount++; }
      });
      c.start('hello');
      waitFor(() => c.isFinalized(), () => {
        setTimeout(() => {
          expect(errorCount).assertEqual(1);
          expect(p.releaseCount).assertEqual(1);
          p.cleanup();
          done();
        }, 60);
      });
    });

    it('normal_flow_completed', 0, (done: Function) => {
      const p: FakeProvider = new FakeProvider();
      p.scripted = [
        { result: makeResult(0, 'AB'), delay: 2 },
        { result: makeResult(1, '', 0, true, [{ title: 'R' }], [{ type: 'card', title: 'C', data: {} }]), delay: 6 }
      ];
      let animatedStart: boolean = false;
      let markdownEnd: boolean = false;
      let cardData: boolean = false;
      let relevant: boolean = false;
      const c: AiCardSessionController = new AiCardSessionController({
        provider: p,
        requestIdGenerator: () => 'rid-7',
        engineTickMs: 2,
        firstPacketMs: 5000,
        streamGapMs: 5000,
        onAnimatedStart: () => { animatedStart = true; },
        onMarkdownEnd: () => { markdownEnd = true; },
        onCardData: () => { cardData = true; },
        onRelevant: () => { relevant = true; }
      });
      c.start('hello');
      waitFor(() => c.isFinalized(), () => {
        expect(animatedStart).assertTrue();
        expect(markdownEnd).assertTrue();
        expect(cardData).assertTrue();
        expect(relevant).assertTrue();
        expect(c.getStatus()).assertEqual('completed');
        expect(p.releaseCount).assertEqual(1);
        p.cleanup();
        done();
      });
    });

    it('pause_stopped_no_error', 0, (done: Function) => {
      const p: FakeProvider = new FakeProvider();
      p.scripted = [{ result: makeResult(0, 'hello'), delay: 2 }];
      let errorCount: number = 0;
      const c: AiCardSessionController = new AiCardSessionController({
        provider: p,
        requestIdGenerator: () => 'rid-8',
        engineTickMs: 2,
        firstPacketMs: 5000,
        streamGapMs: 5000,
        onError: () => { errorCount++; }
      });
      c.start('hello');
      waitFor(() => c.getStatus() === 'streaming', () => {
        c.pause();
        expect(c.isFinalized()).assertTrue();
        expect(c.getStatus()).assertEqual('completed');
        expect(p.cancelCount).assertEqual(1);
        expect(p.releaseCount).assertEqual(1);
        expect(errorCount).assertEqual(0);
        p.cleanup();
        done();
      });
    });

    it('pause_keep_connection_skips_release', 0, (done: Function) => {
      const p: FakeProvider = new FakeProvider();
      p.scripted = [{ result: makeResult(0, 'hello'), delay: 2 }];
      const c: AiCardSessionController = new AiCardSessionController({
        provider: p,
        requestIdGenerator: () => 'rid-k1',
        engineTickMs: 2,
        firstPacketMs: 5000,
        streamGapMs: 5000
      });
      c.start('hello');
      waitFor(() => c.getStatus() === 'streaming', () => {
        c.pause(true);
        expect(c.isFinalized()).assertTrue();
        expect(p.cancelCount).assertEqual(1);
        expect(p.releaseCount).assertEqual(0);
        p.cleanup();
        done();
      });
    });

    it('detach_before_connect_returns_null', 0, () => {
      const p: FakeProvider = new FakeProvider();
      const c: AiCardSessionController = new AiCardSessionController({
        provider: p,
        requestIdGenerator: () => 'rid-k2'
      });
      expect(c.getDetachedProvider()).assertNull();
      c.dispose();
    });

    it('pause_keep_when_connect_pending_still_releases', 0, () => {
      const p: FakeProvider = new FakeProvider();
      const c: AiCardSessionController = new AiCardSessionController({
        provider: p,
        requestIdGenerator: () => 'rid-k3',
        firstPacketMs: 5000,
        streamGapMs: 5000
      });
      c.start('hello');
      c.pause(true);
      expect(p.cancelCount).assertEqual(1);
      expect(p.releaseCount).assertEqual(1);
      expect(c.getDetachedProvider()).assertNull();
      p.cleanup();
    });

    it('reattached_provider_skips_connect', 0, (done: Function) => {
      const p: FakeProvider = new FakeProvider();
      p.scripted = [{ result: makeResult(0, 'A'), delay: 2 }];
      const c1: AiCardSessionController = new AiCardSessionController({
        provider: p,
        requestIdGenerator: () => 'rid-k4',
        engineTickMs: 2,
        firstPacketMs: 5000,
        streamGapMs: 5000
      });
      c1.start('hello');
      waitFor(() => c1.getStatus() === 'streaming', () => {
        c1.pause(true);
        const borrowed: SearchProvider | null = c1.getDetachedProvider();
        expect(borrowed !== null).assertTrue();
        if (borrowed === null) {
          p.cleanup();
          done();
          return;
        }
        const c2: AiCardSessionController = new AiCardSessionController({
          provider: borrowed,
          providerConnected: true,
          requestIdGenerator: () => 'rid-k5',
          engineTickMs: 2,
          firstPacketMs: 5000,
          streamGapMs: 5000
        });
        c2.start('world');
        waitFor(() => c2.getStatus() === 'streaming', () => {
          expect(p.connectCount).assertEqual(1);
          expect(p.searchCount).assertEqual(2);
          expect(p.releaseCount).assertEqual(0);
          c2.dispose();
          p.cleanup();
          done();
        });
      });
    });

    it('mock_cancel_then_search_streams_again', 0, (done: Function) => {
      const fr: TestFixtureResult = { index: 0, status: 17, isFinal: false, streamingText: 'AB' };
      const act: TestFixtureAction = { result: fr };
      const fxt: TestScenarioFixture = { actions: [act] };
      const opts: MockOptions = { fixture: fxt, chunkDelayMs: 5 };
      const p: TestSearchProvider = new TestSearchProvider(opts);
      const cb1: ResultCollector = new ResultCollector();
      const cb2: ResultCollector = new ResultCollector();
      p.connect(nullContext()).then(() => {
        const req1: SearchRequest = { requestId: 'q1', query: 'a', model: 'CLOUD', referenceType: '1' };
        p.search(req1, cb1);
        p.cancel('q1');
        const req2: SearchRequest = { requestId: 'q2', query: 'b', model: 'CLOUD', referenceType: '1' };
        p.search(req2, cb2);
        waitFor(() => cb2.results.length > 0, () => {
          expect(cb1.results.length).assertEqual(0);
          expect(cb2.results.length > 0).assertTrue();
          done();
        });
      });
    });

    it('pause_default_still_releases_and_detach_null', 0, (done: Function) => {
      const p: FakeProvider = new FakeProvider();
      p.scripted = [{ result: makeResult(0, 'hello'), delay: 2 }];
      const c: AiCardSessionController = new AiCardSessionController({
        provider: p,
        requestIdGenerator: () => 'rid-k6',
        engineTickMs: 2,
        firstPacketMs: 5000,
        streamGapMs: 5000
      });
      c.start('hello');
      waitFor(() => c.getStatus() === 'streaming', () => {
        c.pause();
        expect(p.cancelCount).assertEqual(1);
        expect(p.releaseCount).assertEqual(1);
        expect(c.getDetachedProvider()).assertNull();
        p.cleanup();
        done();
      });
    });

    it('collapse_debounce', 0, () => {
      const p: FakeProvider = new FakeProvider();
      let collapseCount: number = 0;
      let expandCount: number = 0;
      const c: AiCardSessionController = new AiCardSessionController({
        provider: p,
        requestIdGenerator: () => 'rid-9',
        onCollapse: () => { collapseCount++; },
        onExpand: () => { expandCount++; }
      });
      c.notifyCollapseChanged(true);
      c.notifyCollapseChanged(true);
      c.notifyCollapseChanged(false);
      expect(collapseCount).assertEqual(1);
      expect(expandCount).assertEqual(1);
      c.dispose();
    });
  });
}
