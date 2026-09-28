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
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { test } from 'node:test';
import vm from 'node:vm';
import * as moduleApi from 'node:module';

const file = new URL('../../../../../frameworks/bridge/declarative_frontend/ark_component/components/arkwaterflow.js',
  import.meta.url);

function load() {
  const calls = [];
  class ModifierWithKey {
    constructor(value) { this.value = value; }
  }
  class ArkScrollable {
    constructor() { this._modifiersWithKeys = new Map(); }
  }
  const context = vm.createContext({
    ModifierWithKey,
    ArkScrollable,
    JSContainerBase: class {},
    modifierWithKey: (map, key, Type, value) => map.set(key, new Type(value)),
    getUINativeModule: () => ({ waterFlow: new Proxy({}, {
      get: (_, name) => (...args) => calls.push({ name, args }),
    }) }),
  });
  const source = readFileSync(file, 'utf8').replace(/export default [^;]+;?\s*$/, '');
  vm.runInContext(source + '\nexportComponent(); exportView();', context);
  return { calls, context };
}

for (const method of ['header', 'footer', 'sticky']) {
  // Node 24 runs the real TypeScript proxy without adding a test-only compiler dependency.
  test(`${method}: lazy proxy preserves chaining and undefined`, {
    skip: typeof moduleApi.stripTypeScriptTypes !== 'function',
  }, () => {
    const calls = [];
    const component = new Proxy({}, { get: (_, name) => (value) => calls.push({ name, value }) });
    const context = vm.createContext({
      ArkScrollable: class {},
      requireNapi: () => ({ createComponent: () => component }),
    });
    const proxyFile = new URL('../../../../../frameworks/bridge/declarative_frontend/ark_modifier/src/' +
      'water_flow_modifier.ts', import.meta.url);
    vm.runInContext(moduleApi.stripTypeScriptTypes(readFileSync(proxyFile, 'utf8')) +
      '\nglobalThis.LazyComponent = LazyArkWaterFlowComponent;', context);
    const proxy = new context.LazyComponent('node', 0);
    const value = method === 'sticky' ? 3 : () => {};
    assert.equal(proxy[method](value), proxy);
    assert.equal(proxy[method](undefined), proxy);
    assert.deepEqual(calls, [{ name: method, value }, { name: method, value: undefined }]);
  });
  test(`${method}: declarative entry forwards builder or undefined`, () => {
    const { calls, context } = load();
    const value = method === 'sticky' ? 3 : () => {};
    context.WaterFlow[method](value);
    context.WaterFlow[method](undefined);
    const name = `set${method[0].toUpperCase()}${method.slice(1)}`;
    assert.deepEqual(calls, [{ name, args: [true, value] }, { name, args: [true, undefined] }]);
  });
  test(`${method}: attribute modifier preserves chaining and reset`, () => {
    const { calls, context } = load();
    const component = new context.ArkWaterFlowComponent('node');
    const value = method === 'sticky' ? 3 : () => {};
    assert.equal(component[method](value), component);
    const [modifier] = component._modifiersWithKeys.values();
    modifier.applyPeer('node', false);
    modifier.applyPeer('node', true);
    const suffix = `${method[0].toUpperCase()}${method.slice(1)}`;
    assert.deepEqual(calls, [
      { name: `set${suffix}`, args: ['node', value] },
      { name: `reset${suffix}`, args: ['node'] },
    ]);
  });
}
