/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// node --experimental-vm-modules advanced_ui_component/segmentbuttonv2/test/segmentbuttonv2_following_release.test.mjs
// Runs shipped JS gesture callbacks with UI services stubbed. Static helper source parity is checked below;
// native gesture routing, static compilation and device acceptance are verified separately.
import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import vm from 'node:vm';

const root = process.env.SEGMENT_BUTTON_REPO ?? fileURLToPath(new URL('../../../', import.meta.url));
const dynamicPath = 'advanced_ui_component/segmentbuttonv2/interfaces/segmentbuttonv2.js';
const source = fs.readFileSync(path.join(root, dynamicPath), 'utf8');
const enums = new Proxy({}, { get: (_, key) => key });
const curves = new Proxy({}, { get: (_, key) => (...args) => ({ key, args }) });
class ViewStub {}
const napi = {
  'arkui.node': { LengthMetrics: { vp: value => ({ value, unit: 'VP' }), resource: () => ({ value: 4, unit: 'VP' }) }, LengthUnit: enums },
  'arkui.uiMaterial': { MaterialState: { ENABLE: 1, DISABLE: 0 }, getMaterialInfo: () => ({ state: 2 }) },
  i18n: { isRTL: () => false, System: { getSystemLanguage: () => 'en' } },
  util: { generateRandomUUID: () => 'test-id' },
  deviceInfo: { sdkApiVersion: 26 },
};
const context = vm.createContext({
  console, curves, ViewPU: ViewStub, ViewV2: ViewStub, PUV2ViewBase: { contextStack: [] },
  ObservedV2: target => target, Trace: () => undefined, Param: () => undefined,
  Event: () => undefined, Computed: () => undefined, Local: () => undefined,
  requireNapi: name => napi[name] ?? {}, requireNativeModule: () => curves,
  FontWeight: enums, BlurStyle: enums, Direction: enums, Color: enums,
});
const module = new vm.SourceTextModule(source + '\nexport { SimpleSegmentButtonV2 };', { context });
await module.link(() => { throw new Error('Unexpected external import'); });
await module.evaluate();
const { SimpleSegmentButtonV2, SegmentButtonV2InteractionMode } = module.namespace;
context.SegmentButtonV2InteractionMode = SegmentButtonV2InteractionMode;

function block(text, opening) {
  let depth = 0;
  for (let end = opening; end < text.length; end++) {
    if (text[end] === '{') depth++;
    if (text[end] === '}' && --depth === 0) return text.slice(opening, end + 1);
  }
  throw new Error('Unbalanced block');
}
function callback(name, view) {
  const prefix = `PanGesture.${name}(`;
  const begin = source.indexOf(prefix);
  assert.notEqual(begin, -1, name);
  const start = begin + prefix.length;
  const body = block(source, source.indexOf('{', start));
  const parameters = source.slice(start, source.indexOf('{', start));
  context.view = view;
  return vm.runInContext(`(function () { return (${parameters}${body}); }).call(view)`, context);
}
function fixture({ rtl = false, padding = 0, gap = 0, selected = 1, mode = 0, material = false, disabled = -1 } = {}) {
  const widths = [80, 100, 120];
  let cursor = padding;
  const physical = (rtl ? [2, 1, 0] : [0, 1, 2]).map(index => {
    const rect = { index, position: { x: cursor, y: 4 }, size: { width: widths[index], height: 34 } };
    cursor += widths[index] + gap;
    return rect;
  });
  const rects = physical.sort((a, b) => a.index - b.index);
  const changes = [];
  const completions = [];
  const view = Object.create(SimpleSegmentButtonV2.prototype);
  Object.assign(view, {
    items: widths.map((_, index) => ({ enabled: index !== disabled })), selectedIndex: selected,
    interactionMode: mode, backgroundSystemMaterial: material ? {} : undefined,
    itemRects: rects, backplatePosition: { ...rects[selected].position }, itemScale: 1,
    openSelectedItemSystemMaterial: false, isTapGesture: false, isPressing: false,
    isDragging: false, followingDragActive: false, dragWithPress: false,
    isMouseWheelScroll: false, panStartGlobalX: 0, panStartIndex: -1, tempDisableAnimation: false,
    getUIContext: () => ({ animateTo(options, action) { action(); if (options.onFinish) completions.push(options.onFinish); } }),
    isRTL: () => rtl, $selectedIndex(index) { changes.push(index); this.selectedIndex = index; },
  });
  const actions = Object.fromEntries(['onActionStart', 'onActionUpdate', 'onActionEnd', 'onActionCancel'].map(name => [name, callback(name, view)]));
  return { view, rects, changes, actions, flush: () => completions.splice(0).forEach(action => action()) };
}
const center = rect => rect.position.x + rect.size.width / 2;
const event = (x, y = 21) => ({ fingerList: [{ localX: x, localY: y }], axisHorizontal: 0, axisVertical: 0, offsetX: 0, offsetY: 0 });
function release(f, x, y, expected) {
  const previous = f.view.selectedIndex;
  f.actions.onActionStart(event(center(f.rects[previous])));
  f.actions.onActionUpdate(event(x, y));
  assert.equal(f.view.selectedIndex, previous, 'following drag does not submit while held');
  f.actions.onActionEnd(event(x, y));
  f.flush();
  assert.equal(f.view.selectedIndex, expected, `release (${x}, ${y})`);
  assert.equal(f.view.isDragging, false);
  assert.equal(f.view.followingDragActive, false);
}
let gestureCases = 0;
for (const rtl of [false, true]) for (const padding of [0, 24]) for (const gap of [0, 8]) {
  const config = { rtl, padding, gap };
  for (const y of [-200, 21, 200]) for (const target of [0, 1, 2]) {
    const f = fixture(config);
    release(f, center(f.rects[target]), y, target);
    assert.deepEqual(f.changes, target === 1 ? [] : [target]);
    gestureCases++;
  }
  for (const side of ['left', 'right']) for (const y of [-200, 200]) {
    const f = fixture(config);
    const x = side === 'left' ? -100 : 1000;
    const expected = side === 'left' ? (rtl ? 2 : 0) : (rtl ? 0 : 2);
    release(f, x, y, expected);
    assert.deepEqual(f.changes, [expected]);
    gestureCases++;
  }
  const consecutive = fixture(config);
  release(consecutive, 1000, 200, rtl ? 0 : 2);
  release(consecutive, -100, -200, rtl ? 2 : 0);
  release(consecutive, center(consecutive.rects[1]), 200, 1);
  release(consecutive, center(consecutive.rects[1]), -200, 1);
  assert.equal(consecutive.changes.length, 3, 'same-value release is not notified');
  gestureCases++;
  for (const target of [0, 2]) {
    const f = fixture({ ...config, disabled: target });
    release(f, center(f.rects[target]), 200, 1);
    assert.deepEqual(f.changes, [], 'disabled target is not selected');
    const x = target === (rtl ? 2 : 0) ? -100 : 1000;
    release(f, x, -200, 1);
    assert.deepEqual(f.changes, [], 'disabled edge is not selected');
    gestureCases++;
  }
  if (gap) {
    const f = fixture(config);
    const physical = [...f.rects].sort((a, b) => a.position.x - b.position.x);
    const x = (physical[0].position.x + physical[0].size.width + physical[1].position.x) / 2;
    release(f, x, 200, 1);
    assert.deepEqual(f.changes, [], 'an internal gap retains the existing no-target behavior');
    gestureCases++;
  }
  const cancelled = fixture(config);
  cancelled.actions.onActionStart(event(center(cancelled.rects[1])));
  cancelled.actions.onActionUpdate(event(1000, 200));
  cancelled.actions.onActionCancel();
  cancelled.flush();
  assert.equal(cancelled.view.selectedIndex, 1);
  assert.deepEqual(cancelled.changes, []);
  release(cancelled, center(cancelled.rects[2]), 21, 2);
  gestureCases++;
}
// Existing normal/default strategies and material following continue through their original paths.
for (const material of [false, true]) for (const mode of [undefined, 0, 1, 99]) {
  const f = fixture({ material, mode, selected: 0 });
  f.view.interactionMode = mode;
  const following = mode === 0 || (material && mode !== 1);
  f.actions.onActionStart(event(center(f.rects[0])));
  f.actions.onActionUpdate(event(center(f.rects[1])));
  assert.equal(f.view.selectedIndex, following ? 0 : 1);
  f.actions.onActionEnd(event(center(f.rects[1])));
  f.flush();
  assert.equal(f.view.selectedIndex, 1);
  assert.deepEqual(f.changes, [1]);
  gestureCases++;
}

function methodBody(text) {
  const marker = text.indexOf('getIndexByHorizontalPosition(localX');
  assert.notEqual(marker, -1, 'horizontal helper is present');
  return block(text, text.indexOf('{', marker));
}
function normalize(text) {
  return text.replace(/:\s*(number|int|double)\b/g, '')
    .replace(/\s+as\s+(number|int|double)\b/g, '').replace(/rect!/g, 'rect')
    .replace(/\((rect\.(?:position\.x|size\.width))\)/g, '$1')
    .replace(/this\.itemRects\[index\]/g, 'this.itemRects.get(index)').replace(/\s+/g, '');
}
const dynamicEts = fs.readFileSync(path.join(root, 'advanced_ui_component/segmentbuttonv2/source/segmentbuttonv2.ets'), 'utf8');
const staticEts = fs.readFileSync(path.join(root, 'advanced_ui_component_static/assembled_advanced_ui_component/@ohos.arkui.advanced.SegmentButtonV2.ets'), 'utf8');
assert.equal(normalize(methodBody(dynamicEts)), normalize(methodBody(source)), 'shipped JS matches dynamic ETS helper');
assert.equal(normalize(methodBody(staticEts)), normalize(methodBody(source)), 'static and dynamic helper logic match');
assert.match(staticEts, /realIndex = this\.getIndexByHorizontalPosition\(finger\.localX\);/);
const staticBody = methodBody(staticEts).replace(/:\s*(int|double)\b/g, '').replace(/\s+as\s+(int|double)\b/g, '').replace(/rect!/g, 'rect');
const staticHelper = vm.runInNewContext(`(function(localX) ${staticBody})`);
let staticHelperCases = 0;
for (const rtl of [false, true]) for (const padding of [0, 24]) for (const gap of [0, 8]) {
  const f = fixture({ rtl, padding, gap });
  const receiver = { items: f.view.items, itemRects: new Map(f.rects.map((rect, index) => [index, rect])) };
  for (let x = -100; x <= 1000; x += 5) {
    assert.equal(staticHelper.call(receiver, x), f.view.getIndexByHorizontalPosition(x));
    staticHelperCases++;
  }
}
console.log(JSON.stringify({ gestureCases, staticHelperCases, scope: 'Shipped JS callbacks with UI stubs; static helper source parity and type-erased geometry checks, not static native runtime acceptance' }));
