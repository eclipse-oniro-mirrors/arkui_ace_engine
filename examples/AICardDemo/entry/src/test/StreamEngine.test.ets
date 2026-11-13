import { describe, it, expect } from '@ohos/hypium';
import { StreamEngine } from '../main/ets/components/aicard/engine/StreamEngine';
import { SearchResult, StreamStatus } from '../main/ets/components/aicard/common/AiCardTypes';

function makeResult(index: number, text: string, status: number = 17, isFinal: boolean = false): SearchResult {
  return {
    requestId: 'r',
    status: status,
    isFinal: isFinal,
    streamingText: text,
    index: index
  };
}

function waitFor(cond: () => boolean, done: () => void, timeoutMs: number = 1000): void {
  const start: number = Date.now();
  const h: number = setInterval(() => {
    if (cond() || Date.now() - start > timeoutMs) {
      clearInterval(h);
      done();
    }
  }, 5);
}

export default function streamEngineTest() {
  describe('StreamEngine', () => {
    it('assemble_in_order', 0, () => {
      const e: StreamEngine = new StreamEngine(40, true, () => {});
      e.push(makeResult(0, 'A'));
      e.push(makeResult(1, 'B'));
      e.push(makeResult(2, 'C'));
      e.stop();
      expect(e.getTargetText()).assertEqual('ABC');
    });

    it('assemble_out_of_order', 0, () => {
      const e: StreamEngine = new StreamEngine(40, true, () => {});
      e.push(makeResult(2, 'C'));
      e.push(makeResult(0, 'A'));
      e.push(makeResult(1, 'B'));
      e.stop();
      expect(e.getTargetText()).assertEqual('ABC');
    });

    it('same_index_overwrites', 0, () => {
      const e: StreamEngine = new StreamEngine(40, true, () => {});
      e.push(makeResult(1, 'A'));
      e.push(makeResult(1, 'B'));
      e.push(makeResult(0, 'X'));
      e.stop();
      expect(e.getTargetText()).assertEqual('XB');
    });

    it('stale_duplicate_ignored', 0, () => {
      const e: StreamEngine = new StreamEngine(40, true, () => {});
      e.push(makeResult(0, 'A'));
      e.push(makeResult(1, 'B'));
      e.push(makeResult(0, 'stale'));
      e.stop();
      expect(e.getTargetText()).assertEqual('AB');
    });

    it('missing_index_non_blocking', 0, () => {
      const e: StreamEngine = new StreamEngine(40, true, () => {});
      e.push(makeResult(0, 'A'));
      e.push(makeResult(2, 'C'));
      expect(e.getTargetText()).assertEqual('A');
      e.push(makeResult(1, 'B'));
      e.stop();
      expect(e.getTargetText()).assertEqual('ABC');
    });

    it('grounding_replaces_target', 0, () => {
      const e: StreamEngine = new StreamEngine(40, true, () => {});
      e.push(makeResult(0, 'hello'));
      e.push(makeResult(1, '<grounding>world</grounding>'));
      e.stop();
      expect(e.getTargetText()).assertEqual('world');
      expect(e.isGrounded()).assertTrue();
    });

    it('status17_ignored_after_grounding', 0, () => {
      const e: StreamEngine = new StreamEngine(40, true, () => {});
      e.push(makeResult(0, '<grounding>final</grounding>'));
      e.push(makeResult(1, 'ignored', 17));
      e.stop();
      expect(e.getTargetText()).assertEqual('final');
    });

    it('isFinal_marked', 0, () => {
      const e: StreamEngine = new StreamEngine(40, true, () => {});
      e.push(makeResult(0, 'A'));
      e.push(makeResult(1, 'B', 0, true));
      expect(e.isStreamEnded()).assertTrue();
      e.stop();
    });

    it('grounding_cursor_cap', 0, () => {
      const e: StreamEngine = new StreamEngine(40, false, () => {});
      e.push(makeResult(0, 'hello'));
      expect(e.getCursor()).assertEqual(5);
      e.push(makeResult(1, '<grounding>hi</grounding>'));
      expect(e.getCursor()).assertEqual(2);
      e.stop();
    });

    it('showMarkdown_false_jumps_and_emits_start_end', 0, () => {
      const events: StreamStatus[] = [];
      const e: StreamEngine = new StreamEngine(40, false, (s: StreamStatus) => {
        events.push(s);
      });
      e.push(makeResult(0, 'AB'));
      e.push(makeResult(1, '', 0, true));
      expect(e.getCursor()).assertEqual(2);
      expect(events[0]).assertEqual('START');
      expect(events[events.length - 1]).assertEqual('END');
      expect(e.isEnded()).assertTrue();
    });

    it('showMarkdown_false_no_partial', 0, () => {
      const events: StreamStatus[] = [];
      const e: StreamEngine = new StreamEngine(40, false, (s: StreamStatus) => {
        events.push(s);
      });
      e.push(makeResult(0, 'AB'));
      e.push(makeResult(1, '', 0, true));
      expect(events.indexOf('PARTIAL')).assertEqual(-1);
    });

    it('typewriter_advances_and_emits_start_partial_end', 0, (done: Function) => {
      const events: StreamStatus[] = [];
      const e: StreamEngine = new StreamEngine(2, true, (s: StreamStatus) => {
        events.push(s);
      });
      e.push(makeResult(0, 'AB'));
      e.push(makeResult(1, '', 0, true));
      waitFor(() => e.isEnded(), () => {
        e.stop();
        expect(events.indexOf('START')).assertEqual(0);
        expect(events.indexOf('PARTIAL')).assertLarger(0);
        expect(events[events.length - 1]).assertEqual('END');
        done();
      });
    });

    it('grounding_keeps_cursor_progress', 0, (done: Function) => {
      const e: StreamEngine = new StreamEngine(2, true, () => {});
      e.push(makeResult(0, 'hello'));
      waitFor(() => e.getCursor() >= 3, () => {
        const before: number = e.getCursor();
        e.push(makeResult(1, '<grounding>world</grounding>'));
        expect(e.getCursor()).assertEqual(before);
        expect(e.getTargetText()).assertEqual('world');
        e.stop();
        done();
      });
    });

    it('stop_freezes_cursor', 0, (done: Function) => {
      const e: StreamEngine = new StreamEngine(2, true, () => {});
      e.push(makeResult(0, 'hello'));
      waitFor(() => e.getCursor() > 0, () => {
        e.stop();
        const frozen: number = e.getCursor();
        setTimeout(() => {
          expect(e.getCursor()).assertEqual(frozen);
          done();
        }, 20);
      });
    });
  });
}
