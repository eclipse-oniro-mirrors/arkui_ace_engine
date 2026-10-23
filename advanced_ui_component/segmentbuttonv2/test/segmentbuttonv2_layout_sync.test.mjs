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

// Run with: node --experimental-vm-modules advanced_ui_component/segmentbuttonv2/test/segmentbuttonv2_layout_sync.test.mjs
// Executes the shipped JS with UI services stubbed; does not replace rendering/device acceptance.
import assert from 'node:assert/strict';
import fs from 'node:fs';
import vm from 'node:vm';

const source = fs.readFileSync(new URL('../interfaces/segmentbuttonv2.js', import.meta.url), 'utf8');
const enums = new Proxy({}, { get: (_, name) => name });
const curves = new Proxy({}, { get: (_, name) => (...args) => ({ name, args }) });
class ViewStub {}
const ignoredDecorator = () => undefined;
const napi = {
  'arkui.node': { LengthMetrics: { vp: value => ({ value, unit: 'VP' }), resource: () => ({ value: 4, unit: 'VP' }) }, LengthUnit: enums },
  'arkui.layoutAlgorithm': { CustomLayoutAlgorithm: class {} },
  'arkui.components.ArkDynamicLayout': { DynamicLayout: {} },
  'arkui.uiMaterial': { MaterialState: { ENABLE: 1, DISABLE: 0 }, getMaterialInfo: () => ({ state: 2 }) },
  i18n: { isRTL: () => false, System: { getSystemLanguage: () => 'en' } },
  util: { generateRandomUUID: () => 'test-id' },
  deviceInfo: { sdkApiVersion: 26 },
};
const context = vm.createContext({
  console, curves, ViewPU: ViewStub, ViewV2: ViewStub, PUV2ViewBase: { contextStack: [] },
  ObservedV2: target => target, Trace: ignoredDecorator, Param: ignoredDecorator, Event: ignoredDecorator,
  Computed: ignoredDecorator, Local: ignoredDecorator, Monitor: () => ignoredDecorator,
  requireNapi: name => napi[name] ?? {}, requireNativeModule: () => curves,
  FontWeight: enums, BlurStyle: enums, Direction: enums, Color: enums,
});
const module = new vm.SourceTextModule(source + '\nexport { SimpleSegmentButtonV2, SegmentButtonV2LayoutAlgorithm };', { context });
await module.link(() => { throw new Error('Unexpected external import'); });
await module.evaluate();
const { SimpleSegmentButtonV2, SegmentButtonV2LayoutAlgorithm, SegmentButtonV2InteractionMode } = module.namespace;
context.SegmentButtonV2InteractionMode = SegmentButtonV2InteractionMode;

function callback(name, view) {
  const prefix = `PanGesture.${name}(`;
  const start = source.indexOf(prefix) + prefix.length;
  assert(start >= prefix.length, name);
  const opening = source.indexOf('{', start);
  let depth = 0;
  for (let end = opening; end < source.length; end++) {
    if (source[end] === '{') depth++;
    if (source[end] === '}' && --depth === 0) {
      context.view = view;
      return vm.runInContext(`(function () { return (${source.slice(start, end + 1)}); }).call(view)`, context);
    }
  }
  throw new Error('Unbalanced callback');
}

function fixture({ mode = 0, material = false, rtl = false, disabled = false, density = 1 } = {}) {
  const changes = [];
  const animations = [];
  const completions = [];
  const view = Object.create(SimpleSegmentButtonV2.prototype);
  const rects = [0, 1, 2].map(i => ({ position: { x: rtl ? 200 - i * 100 : i * 100, y: 0 }, size: { width: 100, height: 40 } }));
  const ui = {
    vp2px: value => value * density, px2vp: value => value / density,
    animateTo(options, action) { animations.push(options); action(); if (options.onFinish) completions.push(options.onFinish); },
  };
  Object.assign(view, {
    items: [0, 1, 2].map(i => ({ enabled: !(disabled && i === 1) })), selectedIndex: 0,
    interactionMode: mode, backgroundSystemMaterial: material ? {} : undefined,
    itemRects: rects, itemScale: 1, openSelectedItemSystemMaterial: false,
    isTapGesture: false, isPressing: false, isDragging: false, followingDragActive: false,
    dragWithPress: false, isMouseWheelScroll: false, panStartGlobalX: 0, panStartIndex: -1,
    selectedItemOffsetX: 0, tempDisableAnimation: false,
    getUIContext: () => ui, isRTL: () => rtl,
    $selectedIndex(index) { changes.push(index); this.selectedIndex = index; this.onSelectedIndexChange(); },
  });
  const algorithm = new SegmentButtonV2LayoutAlgorithm();
  const content = {
    measure() {}, layout() {}, getMeasuredSize: () => ({ width: 300, height: 40 }), getChildrenCount: () => rects.length,
    getChild: index => rects[index] ? {
      getLayoutPosition: () => ({ x: rects[index].position.x * density, y: rects[index].position.y * density }),
      getMeasuredSize: () => ({ width: rects[index].size.width * density, height: rects[index].size.height * density }),
    } : undefined,
  };
  const layer = { measure() {}, layout() {} };
  const node = { getChild: index => [layer, layer, content][index], setMeasuredSize() {}, setLayoutPosition() {} };
  algorithm.onMeasure(node, { percentReference: { width: 300, height: 40 } });
  algorithm.onLayout(node, { x: 0, y: 0 });
  view.layoutAlgorithm = algorithm;
  const actions = Object.fromEntries(['onActionStart', 'onActionUpdate', 'onActionEnd', 'onActionCancel'].map(name => [name, callback(name, view)]));
  return { view, changes, animations, actions, flush: () => completions.splice(0).forEach(action => action()) };
}

const event = x => ({ fingerList: [{ localX: x, localY: 20 }], axisHorizontal: 0, axisVertical: 0, offsetX: 0, offsetY: 0 });
let interactionCases = 0;
for (const rtl of [false, true]) {
  for (const material of [false, true]) {
    for (const mode of [undefined, 0, 1, null, 99]) {
      const following = mode === 0 || (material && mode !== 1);
      for (const scenario of ['cross', 'return', 'outside', 'disabled', 'cancel']) {
        const f = fixture({ rtl, material, mode, disabled: scenario === 'disabled' });
        // Passing undefined must exercise the unset mode, rather than the fixture's default.
        f.view.interactionMode = mode;
        const start = rtl ? 250 : 50;
        f.actions.onActionStart(event(start));
        f.actions.onActionUpdate(event(150));
        assert.equal(f.view.selectedIndex, following || scenario === 'disabled' ? 0 : 1);
        if (scenario === 'cancel') f.actions.onActionCancel();
        else f.actions.onActionEnd(event(scenario === 'return' ? start : scenario === 'outside' ? 400 : 150));
        f.flush();
        if (following && !material && scenario !== 'cross') assert.equal(f.changes.length, 0);
        if (following && scenario === 'cross') assert.deepEqual(f.changes, [1]);
        if (!following && scenario === 'cross') assert.deepEqual(f.changes, [1]);
        if (scenario === 'disabled' && !material) assert.equal(f.view.selectedIndex, 0);
        interactionCases++;
      }
    }
  }
}

let switchCases = 0;
for (const rtl of [false, true]) {
  for (const density of [1, 3]) {
    const f = fixture({ rtl, density });
    const start = rtl ? 250 : 50;
    const moved = rtl ? 220 : 80;
    f.actions.onActionStart(event(start));
    f.actions.onActionUpdate(event(moved));
    assert.equal(f.view.selectedItemOffsetX, rtl ? -30 : 30);
    f.view.interactionMode = 1;
    f.view.onInteractionConfigChange();
    f.actions.onActionUpdate(event(moved));
    f.actions.onActionEnd(event(moved));
    assert.equal(f.view.selectedItemOffsetX, 0);
    assert.equal(f.changes.length, 0);
    switchCases++;
  }
}

let externalChanges = 0;
for (const animated of [false, true]) {
  const f = fixture();
  f.view.enableStateAnimation = animated;
  f.view.selectedIndex = 2;
  f.view.onSelectedIndexChange();
  assert.equal(f.view.layoutAlgorithm.selectedIndex, 2);
  assert.equal(f.view.selectedItemOffsetX, 0);
  assert.equal(f.changes.length, 0);
  assert.equal(f.animations.length, animated ? 1 : 0);
  externalChanges++;
}
{
  const f = fixture({ material: true });
  f.view.interactionMode = undefined;
  f.view.selectedItemOffsetX = 30;
  f.view.backgroundSystemMaterial = undefined;
  f.view.onInteractionConfigChange();
  assert.equal(f.view.selectedItemOffsetX, 0);
  assert.equal(f.changes.length, 0);
  externalChanges++;
}

let geometryFrames = 0;
for (const rtl of [false, true]) {
  let resolved = [];
  let contentLaidOut = false;
  const algorithm = new SegmentButtonV2LayoutAlgorithm();
  algorithm.selectedIndex = 1;
  const backplate = { measure(constraint) { this.size = constraint.minSize; }, layout(position) { this.position = position; } };
  const effect = { measure() {}, layout() {} };
  const content = {
    measure() { contentLaidOut = false; }, layout() { contentLaidOut = true; },
    getMeasuredSize: () => ({ width: 400, height: 100 }), getChildrenCount: () => resolved.length,
    getChild: index => resolved[index] ? { getMeasuredSize: () => resolved[index].size,
      getLayoutPosition: () => { assert(contentLaidOut); return resolved[index].position; } } : undefined,
  };
  const self = { getChild: index => [effect, backplate, content][index], setMeasuredSize() {}, setLayoutPosition() {} };
  for (let frame = 0; frame < 60; frame++) {
    resolved = [0, 1, 2].map(i => ({ position: { x: rtl ? 320 - i * 110 - frame : 20 + i * 110 + frame, y: 10 + frame / 5 },
      size: { width: 70 + i * 10 + frame / 2, height: 35 + i + frame / 10 } }));
    algorithm.onMeasure(self, { percentReference: { width: 400, height: 100 } });
    algorithm.onLayout(self, { x: 0, y: 0 });
    assert.deepEqual(JSON.parse(JSON.stringify(backplate.position)), resolved[1].position);
    assert.deepEqual(JSON.parse(JSON.stringify(backplate.size)), resolved[1].size);
    geometryFrames++;
  }
}
console.log(JSON.stringify({ interactionCases, switchCases, externalChanges, geometryFrames,
  scope: 'shipped JS callbacks and same-pass layout with UI stubs' }));
