import { describe, it, expect } from '@ohos/hypium';
import { TestSearchProvider } from '../main/ets/components/aicard/mocks/TestSearchProvider';
import {
  AiSearchCardRawData, AiSearchRelevantItem,
  SearchCallback, SearchError, SearchResult,
  TestFixtureAction, TestFixtureResult, TestScenarioFixture
} from '../main/ets/components/aicard/common/AiCardTypes';

class RecordingCallback implements SearchCallback {
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

const FULL_TEXT: string = '人工智能卡片组件用于展示智能搜索结果，支持流式打字机渲染、乱序重组、grounding替换与折叠展开序重组，**开发者可经配置快速集成1**';
const CARD_TITLES: string[] = ['定义概念', '核心特性', '应用场景', '技术架构', '发展历程', '未来趋势', '行业案例', '常见问题', '最佳实践', '相关工具'];
const RELEVANT: AiSearchRelevantItem[] = [
  { title: '什么是人工智能' },
  { title: 'AI 能帮我做什么' },
  { title: '推荐一些 AI 学习资源' }
];

function buildCards(): AiSearchCardRawData[] {
  const arr: AiSearchCardRawData[] = [];
  for (const t of CARD_TITLES) {
    const data: Record<string, Object> = {};
    const c: AiSearchCardRawData = { type: 'card', title: t, data: data };
    arr.push(c);
  }
  return arr;
}

function tf(index: number, text: string, status: number, isFinal: boolean): TestFixtureResult {
  const r: TestFixtureResult = { index: index, status: status, isFinal: isFinal, streamingText: text };
  if (isFinal) {
    r.relevant = RELEVANT;
    r.cards = buildCards();
  }
  return r;
}

function fx(actions: TestFixtureAction[], connectFail?: boolean): TestScenarioFixture {
  const f: TestScenarioFixture = { actions: actions };
  if (connectFail === true) {
    f.connectFail = true;
  }
  return f;
}

const NORMAL_FX: TestScenarioFixture = fx([
  { result: tf(0, FULL_TEXT.substring(0, 40), 17, false) },
  { result: tf(1, FULL_TEXT.substring(40), 17, false) },
  { result: tf(2, '', 0, true) }
]);
const OUT_OF_ORDER_FX: TestScenarioFixture = fx([
  { result: tf(2, FULL_TEXT.substring(54), 17, false) },
  { result: tf(0, FULL_TEXT.substring(0, 27), 17, false) },
  { result: tf(1, FULL_TEXT.substring(27, 54), 17, false) },
  { result: tf(3, '', 0, true) }
]);
const GROUNDING_FX: TestScenarioFixture = fx([
  { result: tf(0, FULL_TEXT.substring(0, 10), 17, false) },
  { result: tf(1, '<grounding>' + FULL_TEXT + '</grounding>', 0, false) },
  { result: tf(2, '(忽略)', 17, false) },
  { result: tf(3, '', 0, true) }
]);
const IS_FINAL_FX: TestScenarioFixture = fx([
  { result: tf(0, FULL_TEXT, 17, false) },
  { result: tf(1, '', 0, true) }
]);
const FIRST_PACKET_TIMEOUT_FX: TestScenarioFixture = fx([]);
const STREAM_GAP_TIMEOUT_FX: TestScenarioFixture = fx([
  { result: tf(0, FULL_TEXT, 17, false) }
]);
const CONNECT_FAIL_FX: TestScenarioFixture = fx([], true);
const PROVIDER_ERROR_FX: TestScenarioFixture = fx([
  { result: tf(0, FULL_TEXT.substring(0, 30), 17, false) },
  { error: { code: 999, message: 'mock error' } }
]);
const CANCEL_RESPONSIVE_FX: TestScenarioFixture = fx([
  { result: tf(0, FULL_TEXT.substring(0, 16), 17, false) },
  { result: tf(1, FULL_TEXT.substring(16, 32), 17, false) },
  { result: tf(2, FULL_TEXT.substring(32, 48), 17, false) },
  { result: tf(3, FULL_TEXT.substring(48, 64), 17, false) },
  { result: tf(4, FULL_TEXT.substring(64), 17, false) }
]);

export default function testSearchProviderTest() {
  describe('TestSearchProvider', () => {
    it('normal_emits_in_order_then_isFinal', 0, (done: Function) => {
      const p: TestSearchProvider = new TestSearchProvider({ fixture: NORMAL_FX, chunkDelayMs: 2 });
      const cb: RecordingCallback = new RecordingCallback();
      p.connect(nullContext()).then(() => {
        p.search({ requestId: 'r', query: 'q' }, cb);
        waitFor(() => cb.results.length + cb.errors.length >= 3, () => {
          const r0: SearchResult = cb.results[0]!;
          const r1: SearchResult = cb.results[1]!;
          const r2: SearchResult = cb.results[2]!;
          expect(cb.results.length).assertEqual(3);
          expect(r0.index).assertEqual(0);
          expect(r1.index).assertEqual(1);
          expect(r2.index).assertEqual(2);
          expect(r2.isFinal).assertTrue();
          expect((r2.relevant ?? []).length).assertEqual(3);
          expect(cb.errors.length).assertEqual(0);
          p.release(nullContext());
          done();
        });
      });
    });

    it('out_of_order_emits_ascending_index', 0, (done: Function) => {
      const p: TestSearchProvider = new TestSearchProvider({ fixture: OUT_OF_ORDER_FX, chunkDelayMs: 2 });
      const cb: RecordingCallback = new RecordingCallback();
      p.connect(nullContext()).then(() => {
        p.search({ requestId: 'r', query: 'q' }, cb);
        waitFor(() => cb.results.length >= 4, () => {
          expect(cb.results[0]!.index).assertEqual(2);
          expect(cb.results[1]!.index).assertEqual(0);
          expect(cb.results[2]!.index).assertEqual(1);
          expect(cb.results[3]!.index).assertEqual(3);
          expect(cb.results[3]!.isFinal).assertTrue();
          p.release(nullContext());
          done();
        });
      });
    });

    it('grounding_emits_replacement_body', 0, (done: Function) => {
      const p: TestSearchProvider = new TestSearchProvider({ fixture: GROUNDING_FX, chunkDelayMs: 2 });
      const cb: RecordingCallback = new RecordingCallback();
      p.connect(nullContext()).then(() => {
        p.search({ requestId: 'r', query: 'q' }, cb);
        waitFor(() => cb.results.length >= 4, () => {
          const g: SearchResult = cb.results[1]!;
          expect(g.streamingText).assertContain('<grounding>');
          expect(g.streamingText).assertContain('</grounding>');
          expect(cb.results[3]!.isFinal).assertTrue();
          p.release(nullContext());
          done();
        });
      });
    });

    it('isFinal_emits_final_with_relevant_cards', 0, (done: Function) => {
      const p: TestSearchProvider = new TestSearchProvider({ fixture: IS_FINAL_FX, chunkDelayMs: 2 });
      const cb: RecordingCallback = new RecordingCallback();
      p.connect(nullContext()).then(() => {
        p.search({ requestId: 'r', query: 'q' }, cb);
        waitFor(() => cb.results.length >= 2, () => {
          const f: SearchResult = cb.results[1]!;
          expect(f.isFinal).assertTrue();
          expect((f.relevant ?? []).length).assertLarger(0);
          expect((f.cards ?? []).length).assertLarger(0);
          p.release(nullContext());
          done();
        });
      });
    });

    it('first_packet_timeout_emits_nothing', 0, (done: Function) => {
      const p: TestSearchProvider = new TestSearchProvider({ fixture: FIRST_PACKET_TIMEOUT_FX, chunkDelayMs: 2 });
      const cb: RecordingCallback = new RecordingCallback();
      p.connect(nullContext()).then(() => {
        p.search({ requestId: 'r', query: 'q' }, cb);
        setTimeout(() => {
          expect(cb.results.length).assertEqual(0);
          expect(cb.errors.length).assertEqual(0);
          p.release(nullContext());
          done();
        }, 50);
      });
    });

    it('stream_gap_timeout_emits_one_then_silent', 0, (done: Function) => {
      const p: TestSearchProvider = new TestSearchProvider({ fixture: STREAM_GAP_TIMEOUT_FX, chunkDelayMs: 2 });
      const cb: RecordingCallback = new RecordingCallback();
      p.connect(nullContext()).then(() => {
        p.search({ requestId: 'r', query: 'q' }, cb);
        waitFor(() => cb.results.length >= 1, () => {
          setTimeout(() => {
            expect(cb.results.length).assertEqual(1);
            expect(cb.errors.length).assertEqual(0);
            p.release(nullContext());
            done();
          }, 40);
        });
      });
    });

    it('connect_fail_rejects', 0, (done: Function) => {
      const p: TestSearchProvider = new TestSearchProvider({ fixture: CONNECT_FAIL_FX, chunkDelayMs: 2 });
      let rejected: boolean = false;
      p.connect(nullContext()).then(() => {
      }).catch(() => {
        rejected = true;
      });
      setTimeout(() => {
        expect(rejected).assertTrue();
        p.release(nullContext());
        done();
      }, 30);
    });

    it('provider_error_emits_onError', 0, (done: Function) => {
      const p: TestSearchProvider = new TestSearchProvider({ fixture: PROVIDER_ERROR_FX, chunkDelayMs: 2 });
      const cb: RecordingCallback = new RecordingCallback();
      p.connect(nullContext()).then(() => {
        p.search({ requestId: 'r', query: 'q' }, cb);
        waitFor(() => cb.errors.length >= 1, () => {
          expect(cb.errors.length).assertEqual(1);
          expect(cb.results.length).assertEqual(1);
          p.release(nullContext());
          done();
        });
      });
    });

    it('cancel_responsive_stops_and_invokes_hook', 0, (done: Function) => {
      let cancelCalledId: string = '';
      const p: TestSearchProvider = new TestSearchProvider({
        fixture: CANCEL_RESPONSIVE_FX,
        chunkDelayMs: 2,
        cancelCalled: (id: string) => { cancelCalledId = id; }
      });
      const cb: RecordingCallback = new RecordingCallback();
      p.connect(nullContext()).then(() => {
        p.search({ requestId: 'rid-cancel', query: 'q' }, cb);
        setTimeout(() => {
          p.cancel('rid-cancel');
          const countAtCancel: number = cb.results.length;
          setTimeout(() => {
            expect(cancelCalledId).assertEqual('rid-cancel');
            expect(cb.results.length).assertEqual(countAtCancel);
            p.release(nullContext());
            done();
          }, 30);
        }, 5);
      });
    });
  });
}
