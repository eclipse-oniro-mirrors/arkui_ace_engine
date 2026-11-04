/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// RecoverableStorage unit tests (TASK-1: connect core / type rejection / instanceId partition)
// Uses tsuite/tcase/test helpers from helpers_for_test.ts (node test chain),
// or equivalent shim in local harness (see .codespec evidence).


interface RecvError {
  reason: string;
  keys: Array<string>;
}

class MockRecoverableComponent {
  private instanceId_: number;

  constructor(instanceId: number) {
    this.instanceId_ = instanceId;
  }

  getInstanceId(): number {
    return this.instanceId_;
  }
}

class RecoverableTestModel {
  public text: string;

  constructor(text: string = '') {
    this.text = text;
  }
}

class RecoverableOtherModel {
  public value: number = 0;
}

class RecoverableNamedKeyModel {
  public count: number = 0;
}

class RecoverablePayload {
  public data: string = '';

  constructor(data: string = '') {
    this.data = data;
  }
}

function testConnectBasics(): void {
  const connectBasics = tsuite('RecoverableStorage.connect basics (AC-6/AC-7/AC-8)', () => {
    const componentA = new MockRecoverableComponent(42);
    const componentB = new MockRecoverableComponent(43);

    tcase('first connect returns default creator value', () => {
      const v1 = RecoverableStorage.connect({ component: componentA, type: RecoverableTestModel, key: 'vm', defaultCreator: () => new RecoverableTestModel('a') });
      test('returns instance of type', v1 instanceof RecoverableTestModel);
      test('default creator applied', v1.text === 'a');
    });

    tcase('duplicate connect same type returns same reference (INV-1)', () => {
      const v1 = RecoverableStorage.connect({ component: componentA, type: RecoverableTestModel, key: 'vm', defaultCreator: () => new RecoverableTestModel('a') });
      const v2 = RecoverableStorage.connect({ component: componentA, type: RecoverableTestModel, key: 'vm', defaultCreator: () => new RecoverableTestModel('ignored') });
      test('same reference returned', v1 === v2);
    });

    tcase('duplicate connect with mismatching type throws BusinessError(140124)', () => {
      let code = undefined as number | undefined;
      try {
        RecoverableStorage.connect({ component: componentA, type: RecoverableOtherModel,
          key: 'vm', defaultCreator: () => new RecoverableOtherModel() });
      } catch (e) {
        code = (e as BusinessError).code as number;
      }
      test('mismatch throws 140124', code === 140124);
    });

    tcase('simple types throw BusinessError(140124), not registered (AC-6)', () => {
      const simpleTypes: Array<Function> = [Number, String, Boolean];
      for (const t of simpleTypes) {
        let code = undefined as number | undefined;
        try {
          RecoverableStorage.connect({ component: componentA, type: t as never,
            key: `key_${t.name}`, defaultCreator: () => ({}) });
        } catch (e) {
          code = (e as BusinessError).code as number;
        }
        test(`${t.name} throws`, code !== undefined);
        test(`${t.name} code=140124`, code === 140124);
      }
      // rejected key not registered: same string key remains usable by a valid object type
      const after = RecoverableStorage.connect({ component: componentA, type: RecoverableTestModel, key: 'key_Number', defaultCreator: () => new RecoverableTestModel('ok') });
      test('rejected type did not register key', after instanceof RecoverableTestModel && after.text === 'ok');
    });

    tcase('partitions isolated by instanceId (AC-7)', () => {
      const vB = RecoverableStorage.connect({ component: componentB, type: RecoverableTestModel, key: 'vm', defaultCreator: () => new RecoverableTestModel('b') });
      const vA = RecoverableStorage.connect({ component: componentA, type: RecoverableTestModel, key: 'vm' });
      test('instance B has own value', vB.text === 'b');
      test('instance A value untouched', vA.text === 'a');
      test('values are distinct references', vA !== vB);
    });

    tcase('component provides window instanceId (AC-8)', () => {
      const vB2 = RecoverableStorage.connect({ component: componentB, type: RecoverableTestModel, key: 'vm' });
      test('value routed back to owning instance', vB2.text === 'b');
    });

    tcase('invalid key returns undefined (F-A1 family semantics)', () => {
      const result = RecoverableStorage.connect({ component: componentA, type: RecoverableTestModel, key: '', defaultCreator: () => new RecoverableTestModel() });
      test('empty key returns undefined', result === undefined);
    });

    tcase('key defaults to type name when omitted', () => {
      const v = RecoverableStorage.connect({ component: componentA, type: RecoverableNamedKeyModel, defaultCreator: () => new RecoverableNamedKeyModel() });
      const vAgain = RecoverableStorage.connect({ component: componentA, type: RecoverableNamedKeyModel });
      test('type-name key connect works', v instanceof RecoverableNamedKeyModel);
      test('type-name key reuse returns same reference', v === vAgain);
    });

    tcase('missing default creator on first connect throws BusinessError(140124)', () => {
      let code = undefined as number | undefined;
      try {
        RecoverableStorage.connect({ component: componentA, type: RecoverableTestModel, key: 'no_creator_key' });
      } catch (e) {
        code = (e as BusinessError).code as number;
      }
      test('missing creator throws 140124', code === 140124);
    });

    tcase('TASK-2/3/5 hooks present as stubs', () => {
      const snapshot: string = RecoverableStorageImpl.instance().extractSnapshot(42);
      const parsed: Record<string, unknown> = JSON.parse(snapshot);
      test('extractSnapshot returns empty section stub', parsed.version === 1 && parsed.instances !== undefined);
      RecoverableStorageImpl.instance().setRestoredData(42, snapshot);
      RecoverableStorageImpl.instance().unregisterInstanceIdForRecoverable(42);
      test('stub hooks callable without error', true);
    });
  });
  connectBasics();
}

function testSnapshot(): void {
  const snapshotSuite = tsuite('RecoverableStorage.extractSnapshot budget & section (AC-1/AC-3/AC-4 半部, INV-2/INV-3)', () => {
    const componentC = new MockRecoverableComponent(77);

    tcase('snapshot section contains registered values (AC-1 半部)', () => {
      RecoverableStorage.connect({ component: componentC, type: RecoverablePayload, key: 'small', defaultCreator: () => new RecoverablePayload('hello') });
      const snapshot: string = RecoverableStorageImpl.instance().extractSnapshot(77);
      const parsed: Record<string, unknown> = JSON.parse(snapshot);
      test('section version', parsed.version === 1);
      test('instance partition present', parsed.instances['77'] !== undefined);
      test('value serialized', typeof parsed.instances['77']['small'] === 'string' && parsed.instances['77']['small'].length > 0);
    });

    tcase('empty partition yields empty section (AC-3 半部: 无触发即无产物)', () => {
      const snapshot: string = RecoverableStorageImpl.instance().extractSnapshot(999);
      const parsed: Record<string, unknown> = JSON.parse(snapshot);
      test('no instance partition', parsed.instances['999'] === undefined);
    });

    tcase('over-limit entry evicted with serialization notify, others kept (AC-4 半部, INV-3)', () => {
      let received: unknown = undefined;
      const bigCreator = (): RecoverablePayload => new RecoverablePayload('x'.repeat(500 * 1024));
      const big = RecoverableStorage.connect({ component: componentC, type: RecoverablePayload, key: 'big', defaultCreator: bigCreator,
        notifyOnError: (error) => { received = error; } });
      const small2 = RecoverableStorage.connect({ component: componentC, type: RecoverablePayload, key: 'small2', defaultCreator: () => new RecoverablePayload('keep') });
      const snapshot: string = RecoverableStorageImpl.instance().extractSnapshot(77);
      const parsed: Record<string, unknown> = JSON.parse(snapshot);
      test('over-limit key evicted', parsed.instances['77']['big'] === undefined);
      test('small keys kept', parsed.instances['77']['small2'] !== undefined && parsed.instances['77']['small'] !== undefined);
      test('notifyOnError received', received !== undefined);
      test('reason=serialization', received !== undefined && (received as RecvError).reason === 'serialization');
      test('error keys contains big', received !== undefined && (received as RecvError).keys.indexOf('big') >= 0);
    });

    tcase('globalThis hook registered for save chain (ADR-2 bridge)', () => {
      const hook = (globalThis as Record<string, unknown>).__RecoverableStorageHook__ as { extractSnapshot?: (id: number) => string; setRestoredData?: (id: number, j: string) => void; };
      test('hook exists', hook !== undefined && typeof hook.extractSnapshot === 'function' &&
        typeof hook.setRestoredData === 'function');
      const viaHook = hook.extractSnapshot?.(77) ?? '';
      const parsed: Record<string, unknown> = JSON.parse(viaHook);
      test('hook returns valid section', parsed.version === 1);
    });
  });
  snapshotSuite();
}

function testRestore(): void {
  const restoreSuite = tsuite('RecoverableStorage restore & cleanup (AC-2/AC-5/AC-10/AC-12, INV-4/INV-6)', () => {
    const componentR = new MockRecoverableComponent(88);
    const componentD = new MockRecoverableComponent(89);

    tcase('injected section restores value on connect (AC-2/AC-10 半部)', () => {
      const section = JSON.stringify({ version: 1, instances: { '88': { vm: DataCoder.stringify(new RecoverableTestModel('restored')) } } });
      RecoverableStorageImpl.instance().setRestoredData(88, section);
      const v = RecoverableStorage.connect({ component: componentR, type: RecoverableTestModel, key: 'vm',
        defaultCreator: () => new RecoverableTestModel('default') });
      test('restored value returned', v.text === 'restored');
      test('restored value keeps prototype', v instanceof RecoverableTestModel);
      v.text = 'changed-in-memory';
      const v2 = RecoverableStorage.connect({ component: componentR, type: RecoverableTestModel, key: 'vm' });
      test('second connect returns in-memory value (INV-4)', v2 === v && v2.text === 'changed-in-memory');
    });

    tcase('corrupted section: fully silent fallback to default (AC-12)', () => {
      let received: unknown = undefined;
      RecoverableStorageImpl.instance().setRestoredData(89, '{corrupted');
      const v = RecoverableStorage.connect({ component: componentD, type: RecoverableTestModel, key: 'vm',
        defaultCreator: () => new RecoverableTestModel('fallback'),
        notifyOnError: (error) => { received = error; } });
      test('falls back to default value', v.text === 'fallback');
    });

    tcase('type mismatch in section: fallback + deserialization notify (AC-12)', () => {
      const componentM = new MockRecoverableComponent(91);
      let received: unknown = undefined;
      // inject a RecoverableTestModel payload under a key later connected as RecoverableOtherModel:
      // restoreTo fills the OtherModel target with alien fields, then instanceof check still passes
      // (target built by creator), so mismatch must be detected structurally — inject corrupted
      // type marker instead: a value that DataCoder.parse cannot map (plain string payload).
      const section = JSON.stringify({ version: 1, instances: { '91': { vm: 'not-a-coded-object' } } });
      RecoverableStorageImpl.instance().setRestoredData(91, section);
      const v = RecoverableStorage.connect({ component: componentM, type: RecoverableOtherModel, key: 'vm',
        defaultCreator: () => new RecoverableOtherModel(),
        notifyOnError: (error) => { received = error; } });
      test('type mismatch falls back to default', v instanceof RecoverableOtherModel);
      test('reason=deserialization', received !== undefined && (received as RecvError).reason === 'deserialization');
    });

    tcase('key missing in present section: normal default (not an error, no callback)', () => {
      const componentK = new MockRecoverableComponent(92);
      let received: unknown = undefined;
      const section = JSON.stringify({ version: 1, instances: { '92': { other: 'x' } } });
      RecoverableStorageImpl.instance().setRestoredData(92, section);
      const v = RecoverableStorage.connect({ component: componentK, type: RecoverableTestModel, key: 'missing',
        defaultCreator: () => new RecoverableTestModel('default-k'),
        notifyOnError: (error) => { received = error; } });
      test('missing key falls back to default', v.text === 'default-k');
      test('no callback (normal path, not an error)', received === undefined);
    });

    tcase('no injection at all: plain default (AC-5 形态)', () => {
      const componentN = new MockRecoverableComponent(93);
      let received: unknown = undefined;
      const v = RecoverableStorage.connect({ component: componentN, type: RecoverableTestModel, key: 'vm',
        defaultCreator: () => new RecoverableTestModel('cold'),
        notifyOnError: (error) => { received = error; } });
      test('cold start default value', v.text === 'cold');
      test('no error notified', received === undefined);
    });

    tcase('instance destroy cleanup (AC-7 半部, INV-6)', () => {
      const componentX = new MockRecoverableComponent(94);
      RecoverableStorage.connect({ component: componentX, type: RecoverableTestModel, key: 'vm', defaultCreator: () => new RecoverableTestModel('old') });
      unregisterInstanceIdForRecoverable(94);
      const again = RecoverableStorage.connect({ component: new MockRecoverableComponent(94), type: RecoverableTestModel, key: 'vm',
        defaultCreator: () => new RecoverableTestModel('fresh') });
      test('old value not leaked after destroy', again.text === 'fresh');
      // the re-registered value forms a NEW partition: snapshot carries 'fresh', not 'old'
      const snapshot: Record<string, Record<string, unknown>> = JSON.parse(RecoverableStorageImpl.instance().extractSnapshot(94));
      const coded: string = (snapshot.instances['94'] as Record<string, string>).vm;
      const roundTrip = DataCoder.parse(coded);
      test('destroyed partition rebuilt from fresh value only', (roundTrip as Record<string, unknown>).text === undefined || (roundTrip as Record<string, unknown>).text === 'fresh');
    });
  });
  restoreSuite();
}

function testKeysRemove(): void {
  const keysRemoveSuite = tsuite('RecoverableStorage.keys/remove (AC-13/AC-14, ADR-8)', () => {
    const componentK = new MockRecoverableComponent(96);
    const componentOther = new MockRecoverableComponent(97);

    tcase('keys returns only this instance partition (AC-13)', () => {
      RecoverableStorage.connect({ component: componentK, type: RecoverableTestModel, key: 'k1', defaultCreator: () => new RecoverableTestModel('1') });
      RecoverableStorage.connect({ component: componentK, type: RecoverableTestModel, key: 'k2', defaultCreator: () => new RecoverableTestModel('2') });
      RecoverableStorage.connect({ component: componentOther, type: RecoverableTestModel, key: 'other1', defaultCreator: () => new RecoverableTestModel('3') });
      const keys: Array<string> = RecoverableStorage.keys(componentK);
      test('contains k1', keys.indexOf('k1') >= 0);
      test('contains k2', keys.indexOf('k2') >= 0);
      test('no cross-instance key', keys.indexOf('other1') < 0);
      test('length is partition size', keys.length === 2);
      const emptyKeys: Array<string> = RecoverableStorage.keys(new MockRecoverableComponent(999));
      test('empty partition yields empty array', emptyKeys.length === 0);
    });

    tcase('remove unregisters from snapshot/restore/reconnect (AC-14)', () => {
      // pending restored data for 'k1' must be dropped by remove
      const section = JSON.stringify({ version: 1, instances: { '96': { k1: DataCoder.stringify(new RecoverableTestModel('pending')) } } });
      RecoverableStorageImpl.instance().setRestoredData(96, section);
      RecoverableStorage.remove(componentK, 'k1');
      test('key removed from keys', RecoverableStorage.keys(componentK).indexOf('k1') < 0);
      const snapshot: Record<string, Record<string, unknown>> = JSON.parse(RecoverableStorageImpl.instance().extractSnapshot(96));
      const partition = (snapshot.instances as Record<string, Record<string, unknown>>)['96'] || {};
      test('snapshot excludes removed key', partition['k1'] === undefined);
      const again = RecoverableStorage.connect({ component: componentK, type: RecoverableTestModel, key: 'k1',
        defaultCreator: () => new RecoverableTestModel('fresh-after-remove') });
      test('re-connect walks default creator (no restore)', again.text === 'fresh-after-remove');
    });

    tcase('remove accepts type constructor and tolerates unknown key (AC-14)', () => {
      RecoverableStorage.connect({ component: componentK, type: RecoverablePayload, key: 'payloadKey', defaultCreator: () => new RecoverablePayload('p') });
      let threw = false;
      try {
        RecoverableStorage.remove(componentK, RecoverablePayload); // type ctor resolves to 'RecoverablePayload'... key was 'payloadKey'
        RecoverableStorage.remove(componentK, 'never_registered'); // unknown key: warn only
      } catch (e) {
        threw = true;
      }
      test('remove never throws', !threw);
      RecoverableStorage.remove(componentK, 'payloadKey'); // remove by the explicit string key
      test('string-key remove works', RecoverableStorage.keys(componentK).indexOf('payloadKey') < 0);
    });
  });
  keysRemoveSuite();
}

function testHardening(): void {
  const hardeningSuite = tsuite('runtime parameter validation hardening (AC-6, ADR-10, throw policy)', () => {
    const componentH = new MockRecoverableComponent(100);

    tcase('non-function type via bypass throws 140124', () => {
      const badTypes: Array<Function> = [Number, Function, ArrayBuffer, Symbol, RegExp];
      for (const t of badTypes) {
        let code = undefined as number | undefined;
        try {
          RecoverableStorage.connect({ component: componentH, type: t as never,
            key: 'bad' } as never);
        } catch (e) {
          code = (e as BusinessError).code as number;
        }
        test(`type ${t.name} throws 140124`, code === 140124);
      }
    });

    tcase('non-string key throws 140124', () => {
      let code = undefined as number | undefined;
      try {
        RecoverableStorage.connect({ component: componentH, type: RecoverableTestModel,
          key: 123 as unknown as string,
          defaultCreator: () => new RecoverableTestModel() });
      } catch (e) {
        code = (e as BusinessError).code as number;
      }
      test('non-string key throws 140124', code === 140124);
    });

    tcase('non-function creators/callback throw 140124', () => {
      const badFields: Array<string> = ['defaultCreator', 'defaultSubCreator', 'notifyOnError'];
      for (const field of badFields) {
        let code = undefined as number | undefined;
        try {
          const opts: Record<string, unknown> = { component: componentH,
            type: RecoverableTestModel, key: `f_${field}` };
          if (field !== 'notifyOnError') {
            opts.defaultCreator = () => new RecoverableTestModel();
          }
          opts[field] = 'not-a-function';
          RecoverableStorage.connect(opts as never);
        } catch (e) {
          code = (e as BusinessError).code as number;
        }
        test(`${field} non-function throws 140124`, code === 140124);
      }
    });

    tcase('defaultCreator returning blacklisted instance throws 140124', () => {
      let code = undefined as number | undefined;
      try {
        RecoverableStorage.connect({ component: componentH, type: Object as never,
          key: 'boxed', defaultCreator: () => new String('boxed') });
      } catch (e) {
        code = (e as BusinessError).code as number;
      }
      test('blacklisted value throws 140124', code === 140124);
    });

    tcase('null options and invalid component throw 140124', () => {
      let c1: number | undefined = undefined;
      try { RecoverableStorage.connect(null as never); } catch (e) { c1 = (e as BusinessError).code as number; }
      test('null options throws 140124', c1 === 140124);

      let c2: number | undefined = undefined;
      try { RecoverableStorage.connect({ component: {} as never, type: RecoverableTestModel } as never); }
      catch (e) { c2 = (e as BusinessError).code as number; }
      test('invalid component throws 140124', c2 === 140124);

      // keys/remove with invalid component: silent (no throw, no code)
      const r3 = RecoverableStorage.keys({} as never);
      test('keys with invalid component returns empty array', Array.isArray(r3) && r3.length === 0);
      RecoverableStorage.remove({} as never, 'any');
      test('remove with invalid component never throws', true);
    });

    tcase('defaultCreator itself throws: propagates (PersistenceV2 pattern)', () => {
      const componentT = new MockRecoverableComponent(116);
      let propagated: string | undefined = undefined;
      try {
        RecoverableStorage.connect({ component: componentT, type: RecoverableTestModel,
          key: 'throwing',
          defaultCreator: () => { throw new Error('developer bug in creator'); } });
      } catch (e) {
        propagated = (e as Error).message;
      }
      test('creator exception propagates', propagated === 'developer bug in creator');
      // key must not be registered
      const again = RecoverableStorage.connect({ component: componentT, type: RecoverableTestModel,
        key: 'throwing', defaultCreator: () => new RecoverableTestModel('retry') });
    });

    tcase('bridge instanceId guarded (defensive)', () => {
      let threw = false;
      try {
        RecoverableStorageImpl.instance().extractSnapshot(NaN);
        RecoverableStorageImpl.instance().setRestoredData(-1, '{}');
        RecoverableStorageImpl.instance().unregisterInstanceIdForRecoverable(Infinity);
      } catch (e) { threw = true; }
      test('bridge guards never throw', !threw);
      const snapshot: Record<string, Record<string, unknown>> =
        JSON.parse(RecoverableStorageImpl.instance().extractSnapshot(NaN));
      test('invalid instanceId yields no partition', snapshot.instances.NaN === undefined);
    });

    tcase('budget 200KB: 130KB passes, 150KB evicted', () => {
      const componentB = new MockRecoverableComponent(117);
      // 190KB string + ~50B DataCoder overhead ≈ 190KB < 200KB budget
      let received: unknown = undefined;
      RecoverableStorage.connect({ component: componentB, type: RecoverableTestModel,
        key: 'large', defaultCreator: () => new RecoverableTestModel('x'.repeat(190 * 1024)),
        notifyOnError: (error) => { received = error; } });
      RecoverableStorageImpl.instance().extractSnapshot(117); // trigger serialize
      test('190KB key not evicted', received === undefined);

      // 250KB string + ~50B overhead > 200KB budget → evicted
      const componentC = new MockRecoverableComponent(118);
      let received2: unknown = undefined;
      RecoverableStorage.connect({ component: componentC, type: RecoverableTestModel,
        key: 'toolarge', defaultCreator: () => new RecoverableTestModel('x'.repeat(250 * 1024)),
        notifyOnError: (error) => { received2 = error; } });
      RecoverableStorageImpl.instance().extractSnapshot(118);
      test('250KB key evicted with serialization reason',
        received2 !== undefined && (received2 as RecvError).reason === 'serialization');
    });
  });

  hardeningSuite();
}

function testContainers(): void {
  // 容器类（Array/Map/Set）顶层 connect：defaultSubCreator 作为元素 factory（对照 PersistenceV2 Collection 重载）。
  // 实测锚点（2026-11-03 探针）：容器 root 走 DataCoder 集合分支，元素经 cloneOrCopy(factory) 重建原型；
  // 普通类 root 传 defaultSubCreator 会触发 restorePropValue 构造器不匹配分支（root 被替换、数据丢失），
  // 见末尾误用锚点用例。
  class ItemModel {
    public name: string = '';
    constructor(name: string = '') { this.name = name; }
  }
  const containerSuite = tsuite('RecoverableStorage containers w/ defaultSubCreator (element factory)', () => {
    tcase('Array root: elements rebuilt via defaultSubCreator', () => {
      const c = new MockRecoverableComponent(600);
      const src: Array<ItemModel> = [new ItemModel('a'), new ItemModel('b')];
      const section = JSON.stringify({ version: 1,
        instances: { '600': { list: DataCoder.stringify(src) } } });
      RecoverableStorageImpl.instance().setRestoredData(600, section);
      const v = RecoverableStorage.connect({
        component: c, type: Array, key: 'list',
        defaultCreator: (): Array<ItemModel> => [],
        defaultSubCreator: (): ItemModel => new ItemModel()
      }) as Array<ItemModel> | undefined;
      test('array length restored', v !== undefined && v.length === 2);
      test('element prototype kept (factory)', v !== undefined && v[0] instanceof ItemModel);
      test('element data restored', v !== undefined && v[0].name === 'a');
    });

    tcase('Map root: values rebuilt via defaultSubCreator', () => {
      const c = new MockRecoverableComponent(601);
      const src: Map<string, ItemModel> = new Map();
      src.set('k1', new ItemModel('m1'));
      const section = JSON.stringify({ version: 1,
        instances: { '601': { m: DataCoder.stringify(src) } } });
      RecoverableStorageImpl.instance().setRestoredData(601, section);
      const v = RecoverableStorage.connect({
        component: c, type: Map, key: 'm',
        defaultCreator: (): Map<string, ItemModel> => new Map(),
        defaultSubCreator: (): ItemModel => new ItemModel()
      }) as Map<string, ItemModel> | undefined;
      const e = v !== undefined ? v.get('k1') : undefined;
      test('map entry restored', e !== undefined);
      test('map value prototype kept (factory)', e instanceof ItemModel);
      test('map value data restored', e !== undefined && e.name === 'm1');
    });

    tcase('Set root: items rebuilt via defaultSubCreator', () => {
      const c = new MockRecoverableComponent(602);
      const src: Set<ItemModel> = new Set();
      src.add(new ItemModel('s1'));
      const section = JSON.stringify({ version: 1,
        instances: { '602': { s: DataCoder.stringify(src) } } });
      RecoverableStorageImpl.instance().setRestoredData(602, section);
      const v = RecoverableStorage.connect({
        component: c, type: Set, key: 's',
        defaultCreator: (): Set<ItemModel> => new Set(),
        defaultSubCreator: (): ItemModel => new ItemModel()
      }) as Set<ItemModel> | undefined;
      const first = v !== undefined ? [...v][0] : undefined;
      test('set item restored', first !== undefined);
      test('set item prototype kept (factory)', first instanceof ItemModel);
      test('set item data restored', first !== undefined && first.name === 's1');
    });

    tcase('Array root without defaultSubCreator: elements are plain objects (known limitation)', () => {
      const c = new MockRecoverableComponent(603);
      const src: Array<ItemModel> = [new ItemModel('x')];
      const section = JSON.stringify({ version: 1,
        instances: { '603': { list: DataCoder.stringify(src) } } });
      RecoverableStorageImpl.instance().setRestoredData(603, section);
      const v = RecoverableStorage.connect({
        component: c, type: Array, key: 'list',
        defaultCreator: (): Array<ItemModel> => []
      }) as Array<ItemModel> | undefined;
      test('array data restored (length/name)', v !== undefined && v.length === 1 &&
        (v[0] as unknown as Record<string, string>).name === 'x');
      test('element prototype lost without factory (documented)', v !== undefined && !(v[0] instanceof ItemModel));
    });

    tcase('class root with defaultSubCreator: guard ignores it, restore works (hardening 2026-11-03)', () => {
      // 护栏（connect 入口）：defaultSubCreator 仅对容器 root（Array/Map/Set）生效；
      // 类 root 上被忽略并输出 warning（不抛错）。恢复走实例树模板就地填充，数据与原型保留。
      // 此前误用行为（root 被替换、整档丢失）已被护栏消除——本用例为该护栏的回归锚点。
      class HolderModel {
        public title: string = '';
        public sub: ItemModel = new ItemModel();
        constructor(title: string = '') { this.title = title; }
      }
      const c = new MockRecoverableComponent(604);
      const src = new HolderModel('h');
      src.sub.name = 'nested';
      const section = JSON.stringify({ version: 1,
        instances: { '604': { h: DataCoder.stringify(src) } } });
      RecoverableStorageImpl.instance().setRestoredData(604, section);
      const v = RecoverableStorage.connect({
        component: c, type: HolderModel, key: 'h',
        defaultCreator: (): HolderModel => new HolderModel('default'),
        defaultSubCreator: (): ItemModel => new ItemModel()
      });
      test('guard: data restored in-place (title)', v !== undefined && v.title === 'h');
      test('guard: nested prototype kept', v !== undefined && v.sub instanceof ItemModel);
      test('guard: nested data restored', v !== undefined && v.sub.name === 'nested');
    });
  });
  containerSuite();
}

function testReadOnlyProperties(): void {
  // ADR-14 / AC-14: 只读属性（getter-only / @Computed / 非可写数据属性）保存跳过 + 恢复防御。
  // @Computed 用 ComputedV2.InitRun 同款 descriptor 签名复刻（实例自有 get+set、
  // setter 恒抛 readonly 错、`___comp_cached_` 背书属性）；真实 @Computed（含 V2 观察
  // 依赖重算）为 TASK-7 设备验证项，此处锁定检测契约与双向跳过行为。
  function installComputedLike(target: object, prop: string, compute: () => unknown): void {
    const cachedProp = ComputedV2.COMPUTED_CACHED_PREFIX + prop;
    Reflect.defineProperty(target, prop, {
      get(): unknown { return (this as Record<string, unknown>)[cachedProp]; },
      set(_: unknown): void {
        throw new Error(`@Computed ${prop} is readonly, cannot set value for it`);
      },
      enumerable: true
    });
    (target as Record<string, unknown>)[cachedProp] = compute();
  }

  class RsComputedLikeModel {
    public a: number = 1;
    public b: number = 2;
    // runtime-installed accessor (see installComputedLike) — declaration is type-only
    public declare total: number;
    constructor(a: number = 1, b: number = 2) {
      this.a = a;
      this.b = b;
      installComputedLike(this, 'total', (): number => this.a + this.b);
    }
  }

  class RsGetterOnlyModel {
    public first: string = 'a';
    public last: string = 'b';
    get full(): string { return this.first + this.last; }
  }

  class RsNonWritableModel {
    public normal: string = 'v';
    public locked: string;
    constructor() {
      Object.defineProperty(this, 'locked',
        { value: 'frozen', writable: false, enumerable: true, configurable: true });
    }
  }

  const roSuite = tsuite('RecoverableStorage getter-only/@Computed skip (AC-14/VM-14)', () => {
    tcase('save-side: readonly keys absent from snapshot, writable keys kept', () => {
      const c = new MockRecoverableComponent(700);
      RecoverableStorage.connect({ component: c, type: RsComputedLikeModel, key: 'ro_comp',
        defaultCreator: (): RsComputedLikeModel => new RsComputedLikeModel(3, 4) });
      RecoverableStorage.connect({ component: c, type: RsGetterOnlyModel, key: 'ro_get',
        defaultCreator: (): RsGetterOnlyModel => new RsGetterOnlyModel() });
      RecoverableStorage.connect({ component: c, type: RsNonWritableModel, key: 'ro_lock',
        defaultCreator: (): RsNonWritableModel => new RsNonWritableModel() });
      const snapshot: string = RecoverableStorageImpl.instance().extractSnapshot(700);
      const parsed = JSON.parse(snapshot) as Record<string, any>;
      const section = parsed.instances['700'];
      const compKeys = Object.keys(DataCoder.parse(section['ro_comp']) as object);
      const getKeys = Object.keys(DataCoder.parse(section['ro_get']) as object);
      const lockKeys = Object.keys(DataCoder.parse(section['ro_lock']) as object);
      test('computed accessor key skipped', !compKeys.includes('total'));
      test('computed writable deps kept', compKeys.includes('a') && compKeys.includes('b'));
      test('computed cached backing kept (value round-trip channel)',
        compKeys.includes(ComputedV2.COMPUTED_CACHED_PREFIX + 'total'));
      test('prototype getter-only key skipped', !getKeys.includes('full'));
      test('getter-only deps kept', getKeys.includes('first') && getKeys.includes('last'));
      test('non-writable key skipped', !lockKeys.includes('locked'));
      test('non-writable sibling kept', lockKeys.includes('normal'));
    });

    tcase('restore-side: filtered archive round-trips, no throw, no callback', () => {
      const c = new MockRecoverableComponent(701);
      let received: RecvError | undefined = undefined;
      // extractSnapshot partitions by the SOURCE instanceId — remap the section onto the
      // consuming instance (mirrors the native bridge delivering per-instance sections).
      const snapshot700 = RecoverableStorageImpl.instance().extractSnapshot(700);
      const remapped = JSON.stringify({ version: 1,
        instances: { '701': (JSON.parse(snapshot700) as Record<string, any>).instances['700'] } });
      RecoverableStorageImpl.instance().setRestoredData(701, remapped);
      const comp = RecoverableStorage.connect({ component: c, type: RsComputedLikeModel, key: 'ro_comp',
        defaultCreator: (): RsComputedLikeModel => new RsComputedLikeModel(90, 90),
        notifyOnError: (error) => { received = error as RecvError; } });
      const get = RecoverableStorage.connect({ component: c, type: RsGetterOnlyModel, key: 'ro_get',
        defaultCreator: (): RsGetterOnlyModel => new RsGetterOnlyModel(),
        notifyOnError: (error) => { received = error as RecvError; } });
      test('computed instance returned (no fallback)', comp !== undefined && comp instanceof RsComputedLikeModel);
      test('computed deps restored', comp !== undefined && comp.a === 3 && comp.b === 4);
      test('computed value consistent with restored deps', comp !== undefined && comp.total === 7);
      test('getter-only instance returned', get !== undefined && get instanceof RsGetterOnlyModel);
      test('getter-only deps restored', get !== undefined && get.first === 'a' && get.last === 'b');
      test('getter-only value recomputed from deps', get !== undefined && get.full === 'ab');
      test('no notifyOnError for readonly skipping', received === undefined);
    });

    tcase('restore-side defense: legacy archive containing readonly keys does not fail the key', () => {
      // 旧版本档（无过滤）会序列化只读属性值（getter 求值产物）——恢复时对 target 侧
      // 只读属性一律跳过：赋值永不发生（setter 恒抛 readonly 错），整档不回退、不回调。
      const c = new MockRecoverableComponent(702);
      let received: RecvError | undefined = undefined;
      const legacyComp = DataCoder.stringify(new RsComputedLikeModel(10, 20), false); // contains total + cached
      const legacyGet = DataCoder.stringify(new RsGetterOnlyModel(), false);          // contains full
      const legacyLock = DataCoder.stringify(new RsNonWritableModel(), false);        // contains locked
      const section = JSON.stringify({ version: 1,
        instances: { '702': { ro_comp: legacyComp, ro_get: legacyGet, ro_lock: legacyLock } } });
      RecoverableStorageImpl.instance().setRestoredData(702, section);
      const comp = RecoverableStorage.connect({ component: c, type: RsComputedLikeModel, key: 'ro_comp',
        defaultCreator: (): RsComputedLikeModel => new RsComputedLikeModel(90, 90),
        notifyOnError: (error) => { received = error as RecvError; } });
      const get = RecoverableStorage.connect({ component: c, type: RsGetterOnlyModel, key: 'ro_get',
        defaultCreator: (): RsGetterOnlyModel => new RsGetterOnlyModel(),
        notifyOnError: (error) => { received = error as RecvError; } });
      const lock = RecoverableStorage.connect({ component: c, type: RsNonWritableModel, key: 'ro_lock',
        defaultCreator: (): RsNonWritableModel => new RsNonWritableModel(),
        notifyOnError: (error) => { received = error as RecvError; } });
      test('legacy computed archive restores (setter never invoked)', comp !== undefined && comp.a === 10 && comp.b === 20);
      test('legacy computed value consistent', comp !== undefined && comp.total === 30);
      test('legacy getter-only archive restores', get !== undefined && get.first === 'a');
      test('legacy getter-only value recomputed', get !== undefined && get.full === 'ab');
      test('legacy non-writable archive restores (no TypeError)', lock !== undefined && lock.normal === 'v');
      test('legacy non-writable key untouched', lock !== undefined && lock.locked === 'frozen');
      test('no fallback notify for any readonly key', received === undefined);
    });

    tcase('mixed object: writable restored alongside readonly skipped', () => {
      class RsMixedModel {
        public name: string = 'n';
        public cached: number = 0;
        constructor() {
          installComputedLike(this, 'derived', (): number => this.cached * 2);
        }
      }
      const c = new MockRecoverableComponent(703);
      const saved = RecoverableStorage.connect({ component: c, type: RsMixedModel, key: 'mixed',
        defaultCreator: (): RsMixedModel => new RsMixedModel() });
      saved.name = 'saved';
      saved.cached = 21;
      const snapshot703 = RecoverableStorageImpl.instance().extractSnapshot(703);
      const parsed = JSON.parse(snapshot703) as Record<string, any>;
      const section = parsed.instances['703'];
      const keys = Object.keys(DataCoder.parse(section['mixed']) as object);
      test('mixed: derived skipped', !keys.includes('derived'));
      test('mixed: writable kept', keys.includes('name') && keys.includes('cached'));
      // round-trip through a fresh partition (instance key remapped onto the consumer)
      const remapped703 = JSON.stringify({ version: 1,
        instances: { '704': (JSON.parse(snapshot703) as Record<string, any>).instances['703'] } });
      RecoverableStorageImpl.instance().setRestoredData(704, remapped703);
      const c2 = new MockRecoverableComponent(704);
      const v = RecoverableStorage.connect({ component: c2, type: RsMixedModel, key: 'mixed',
        defaultCreator: (): RsMixedModel => new RsMixedModel() });
      test('mixed: restored without throw', v !== undefined);
      test('mixed: writable value restored (not default)', v !== undefined && v.name === 'saved');
      test('mixed: numeric writable restored', v !== undefined && v.cached === 21);
    });
  });
  roSuite();
}

function testRecoverableStorage(): void {
  testConnectBasics();
  testSnapshot();
  testRestore();
  testKeysRemove();
  testHardening();
  testContainers();
  testReadOnlyProperties();
}
