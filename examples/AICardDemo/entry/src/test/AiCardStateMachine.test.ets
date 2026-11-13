import { describe, it, expect } from '@ohos/hypium';
import { AiCardStateMachine } from '../main/ets/components/aicard/engine/AiCardStateMachine';
import { AiSearchStatus } from '../main/ets/components/aicard/common/AiCardTypes';

export default function stateMachineTest() {
  describe('AiCardStateMachine', () => {
    it('initial_idle', 0, () => {
      const sm: AiCardStateMachine = new AiCardStateMachine(() => {});
      expect(sm.getStatus()).assertEqual('idle');
      expect(sm.isFinalized()).assertFalse();
    });

    it('transitions_to_searching', 0, () => {
      const seq: AiSearchStatus[] = [];
      const sm: AiCardStateMachine = new AiCardStateMachine((s: AiSearchStatus) => {
        seq.push(s);
      });
      sm.toSearching();
      expect(sm.getStatus()).assertEqual('searching');
      expect(seq[0]).assertEqual('searching');
    });

    it('transitions_to_streaming', 0, () => {
      const seq: AiSearchStatus[] = [];
      const sm: AiCardStateMachine = new AiCardStateMachine((s: AiSearchStatus) => {
        seq.push(s);
      });
      sm.toSearching();
      sm.toStreaming();
      expect(sm.getStatus()).assertEqual('streaming');
      expect(seq[1]).assertEqual('streaming');
    });

    it('completed_terminal', 0, () => {
      const sm: AiCardStateMachine = new AiCardStateMachine(() => {});
      sm.toSearching();
      sm.toStreaming();
      sm.toCompleted();
      expect(sm.getStatus()).assertEqual('completed');
      expect(sm.isFinalized()).assertTrue();
    });

    it('stopped_maps_to_completed_external', 0, () => {
      const seq: AiSearchStatus[] = [];
      const sm: AiCardStateMachine = new AiCardStateMachine((s: AiSearchStatus) => {
        seq.push(s);
      });
      sm.toSearching();
      sm.toStreaming();
      sm.toStopped();
      expect(sm.getStatus()).assertEqual('completed');
      expect(sm.isFinalized()).assertTrue();
    });

    it('error_terminal', 0, () => {
      const sm: AiCardStateMachine = new AiCardStateMachine(() => {});
      sm.toSearching();
      sm.toError();
      expect(sm.getStatus()).assertEqual('error');
      expect(sm.isFinalized()).assertTrue();
    });

    it('finalize_idempotent', 0, () => {
      const seq: AiSearchStatus[] = [];
      const sm: AiCardStateMachine = new AiCardStateMachine((s: AiSearchStatus) => {
        seq.push(s);
      });
      sm.toSearching();
      sm.toStreaming();
      sm.toCompleted();
      sm.toError();
      expect(sm.getStatus()).assertEqual('completed');
      let completedCount: number = 0;
      let errorCount: number = 0;
      for (const s of seq) {
        if (s === 'completed') {
          completedCount++;
        } else if (s === 'error') {
          errorCount++;
        }
      }
      expect(completedCount).assertEqual(1);
      expect(errorCount).assertEqual(0);
    });

    it('no_reemit_same_status', 0, () => {
      const seq: AiSearchStatus[] = [];
      const sm: AiCardStateMachine = new AiCardStateMachine((s: AiSearchStatus) => {
        seq.push(s);
      });
      sm.toSearching();
      sm.toStreaming();
      sm.toStreaming();
      let streamingCount: number = 0;
      for (const s of seq) {
        if (s === 'streaming') {
          streamingCount++;
        }
      }
      expect(streamingCount).assertEqual(1);
    });

    it('to_streaming_from_idle_noop', 0, () => {
      const sm: AiCardStateMachine = new AiCardStateMachine(() => {});
      sm.toStreaming();
      expect(sm.getStatus()).assertEqual('idle');
    });
  });
}
