import { describe, it, expect } from '@ohos/hypium';
import {
  AiCardControlTarget, AiCardController, bindAiCardController, unbindAiCardController
} from '../main/ets/components/aicard/controller/AiCardController';
import { CardRequestOptions } from '../main/ets/components/aicard/common/AiCardTypes';

class FakeTarget implements AiCardControlTarget {
  requestQueries: string[] = [];
  requestOptionsList: (CardRequestOptions | undefined)[] = [];
  cancelCount: number = 0;

  request(query: string, requestOptions?: CardRequestOptions): void {
    this.requestQueries.push(query);
    this.requestOptionsList.push(requestOptions);
  }

  cancel(): void {
    this.cancelCount++;
  }
}

export default function facadeTest() {
  describe('AiCardControllerFacade', () => {
    it('unbound_request_cancel_no_op', 0, () => {
      const c: AiCardController = new AiCardController();
      c.request('hello');
      c.cancel();
    });

    it('bound_request_cancel_forwarded', 0, () => {
      const c: AiCardController = new AiCardController();
      const t: FakeTarget = new FakeTarget();
      bindAiCardController(c, t);
      c.request('query-a');
      c.cancel();
      c.cancel();
      expect(t.requestQueries.length).assertEqual(1);
      expect(t.requestQueries[0]).assertEqual('query-a');
      expect(t.cancelCount).assertEqual(2);
      unbindAiCardController(c, t);
    });

    it('bind_overrides_previous_target', 0, () => {
      const c: AiCardController = new AiCardController();
      const t1: FakeTarget = new FakeTarget();
      const t2: FakeTarget = new FakeTarget();
      bindAiCardController(c, t1);
      bindAiCardController(c, t2);
      c.request('after-rebind');
      expect(t1.requestQueries.length).assertEqual(0);
      expect(t2.requestQueries.length).assertEqual(1);
      expect(t2.requestQueries[0]).assertEqual('after-rebind');
      unbindAiCardController(c, t2);
    });

    it('checked_unbind_keeps_new_binding', 0, () => {
      const c: AiCardController = new AiCardController();
      const t1: FakeTarget = new FakeTarget();
      const t2: FakeTarget = new FakeTarget();
      bindAiCardController(c, t1);
      bindAiCardController(c, t2);
      unbindAiCardController(c, t1);
      c.request('still-bound');
      expect(t2.requestQueries.length).assertEqual(1);
      unbindAiCardController(c, t2);
    });

    it('after_unbound_calls_are_no_op', 0, () => {
      const c: AiCardController = new AiCardController();
      const t: FakeTarget = new FakeTarget();
      bindAiCardController(c, t);
      unbindAiCardController(c, t);
      c.request('gone');
      c.cancel();
      expect(t.requestQueries.length).assertEqual(0);
      expect(t.cancelCount).assertEqual(0);
    });

    it('two_controllers_isolated_in_registry', 0, () => {
      const c1: AiCardController = new AiCardController();
      const c2: AiCardController = new AiCardController();
      const t1: FakeTarget = new FakeTarget();
      const t2: FakeTarget = new FakeTarget();
      bindAiCardController(c1, t1);
      bindAiCardController(c2, t2);
      c1.request('one');
      c2.cancel();
      expect(t1.requestQueries.length).assertEqual(1);
      expect(t1.cancelCount).assertEqual(0);
      expect(t2.requestQueries.length).assertEqual(0);
      expect(t2.cancelCount).assertEqual(1);
      unbindAiCardController(c1, t1);
      unbindAiCardController(c2, t2);
    });

    it('request_options_forwarded_by_reference', 0, () => {
      const c: AiCardController = new AiCardController();
      const t: FakeTarget = new FakeTarget();
      bindAiCardController(c, t);
      const opts: CardRequestOptions = {};
      c.request('with-options', opts);
      expect(t.requestQueries.length).assertEqual(1);
      expect(t.requestQueries[0]).assertEqual('with-options');
      expect(t.requestOptionsList.length).assertEqual(1);
      expect(t.requestOptionsList[0] === opts).assertTrue();
      unbindAiCardController(c, t);
    });

    it('request_without_options_forwards_undefined', 0, () => {
      const c: AiCardController = new AiCardController();
      const t: FakeTarget = new FakeTarget();
      bindAiCardController(c, t);
      c.request('plain');
      expect(t.requestQueries.length).assertEqual(1);
      expect(t.requestOptionsList.length).assertEqual(1);
      expect(t.requestOptionsList[0] === undefined).assertTrue();
      unbindAiCardController(c, t);
    });
  });
}
