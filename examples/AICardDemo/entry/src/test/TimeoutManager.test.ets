import { describe, it, expect } from '@ohos/hypium';
import { TimeoutManager } from '../main/ets/components/aicard/engine/TimeoutManager';

function waitFor(cond: () => boolean, done: () => void, timeoutMs: number = 1000): void {
  const start: number = Date.now();
  const h: number = setInterval(() => {
    if (cond() || Date.now() - start > timeoutMs) {
      clearInterval(h);
      done();
    }
  }, 5);
}

export default function timeoutManagerTest() {
  describe('TimeoutManager', () => {
    it('first_packet_fires', 0, (done: Function) => {
      let fired: boolean = false;
      const tm: TimeoutManager = new TimeoutManager(() => {
        fired = true;
      }, () => {}, 50, 100);
      tm.startFirstPacket();
      waitFor(() => fired, () => {
        expect(fired).assertTrue();
        done();
      });
    });

    it('first_packet_stopped_no_fire', 0, (done: Function) => {
      let fired: boolean = false;
      const tm: TimeoutManager = new TimeoutManager(() => {
        fired = true;
      }, () => {}, 50, 100);
      tm.startFirstPacket();
      tm.stopFirstPacket();
      setTimeout(() => {
        expect(fired).assertFalse();
        done();
      }, 90);
    });

    it('stream_gap_fires', 0, (done: Function) => {
      let fired: boolean = false;
      const tm: TimeoutManager = new TimeoutManager(() => {}, () => {
        fired = true;
      }, 50, 60);
      tm.resetStreamGap();
      waitFor(() => fired, () => {
        expect(fired).assertTrue();
        done();
      });
    });

    it('stream_gap_reset_no_fire', 0, (done: Function) => {
      let fired: boolean = false;
      const tm: TimeoutManager = new TimeoutManager(() => {}, () => {
        fired = true;
      }, 50, 80);
      tm.resetStreamGap();
      setTimeout(() => {
        tm.resetStreamGap();
      }, 40);
      setTimeout(() => {
        tm.stopAll();
        expect(fired).assertFalse();
        done();
      }, 100);
    });

    it('stop_all_clears', 0, (done: Function) => {
      let f1: boolean = false;
      let f2: boolean = false;
      const tm: TimeoutManager = new TimeoutManager(() => {
        f1 = true;
      }, () => {
        f2 = true;
      }, 50, 60);
      tm.startFirstPacket();
      tm.resetStreamGap();
      tm.stopAll();
      setTimeout(() => {
        expect(f1).assertFalse();
        expect(f2).assertFalse();
        done();
      }, 100);
    });
  });
}
