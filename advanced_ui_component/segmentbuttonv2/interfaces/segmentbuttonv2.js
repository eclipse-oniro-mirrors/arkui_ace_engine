/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

var m =
  (this && this.m) ||
  function (l1, target, key, desc) {
    var c = arguments.length,
      r =
        c < 3
          ? target
          : desc === null
          ? (desc = Object.getOwnPropertyDescriptor(target, key))
          : desc,
      d;
    if (typeof Reflect === 'object' && typeof Reflect.h1 === 'function')
      r = Reflect.h1(l1, target, key, desc);
    else
      for (var m1 = l1.length - 1; m1 >= 0; m1--)
        if ((d = l1[m1]))
          r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
  };
if (!('finalizeConstruction' in ViewPU.prototype)) {
  Reflect.set(ViewPU.prototype, 'finalizeConstruction', () => {});
}
if (PUV2ViewBase.contextStack === undefined) {
  Reflect.set(PUV2ViewBase, 'contextStack', []);
}
const curves = requireNativeModule('ohos.curves');
const LengthMetrics = requireNapi('arkui.node').LengthMetrics;
const LengthUnit = requireNapi('arkui.node').LengthUnit;
const i18n = requireNapi('i18n');
const o = 1;
const t = 2;
const u = 0;
const a1 = 1;
const b1 = {
  buttonBackgroundColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.comp_background_tertiary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  buttonBorderRadius: {
    id: -1,
    type: 10002,
    params: ['sys.float.corner_radius_level10'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  buttonMinHeight: 40,
  i1: 64,
  buttonPadding: {
    id: -1,
    type: 10002,
    params: ['sys.float.padding_level1'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSelectedBackgroundColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.white'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemBorderRadius: {
    id: -1,
    type: 10002,
    params: ['sys.float.corner_radius_level9'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  j1: {
    id: -1,
    type: 10002,
    params: ['sys.float.corner_radius_level10'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSpace: LengthMetrics.vp(0),
  itemFontSize: {
    id: -1,
    type: 10002,
    params: ['sys.float.ohos_id_text_size_button2'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemFontColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_secondary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSelectedFontColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_primary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemFontWeight: FontWeight.Medium,
  itemSelectedFontWeight: FontWeight.Medium,
  itemIconSize: 24,
  itemIconFillColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_secondary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSelectedIconFillColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_primary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSymbolFontSize: 20,
  itemSymbolFontColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_secondary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSelectedSymbolFontColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_primary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemMinHeight: 36,
  l1: 60,
  itemPadding: {
    top: LengthMetrics.resource({
      id: -1,
      type: 10002,
      params: ['sys.float.padding_level2'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    }),
    bottom: LengthMetrics.resource({
      id: -1,
      type: 10002,
      params: ['sys.float.padding_level2'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    }),
    start: LengthMetrics.resource({
      id: -1,
      type: 10002,
      params: ['sys.float.padding_level8'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    }),
    end: LengthMetrics.resource({
      id: -1,
      type: 10002,
      params: ['sys.float.padding_level8'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    }),
  },
  itemShadow: ShadowStyle.OUTER_DEFAULT_XS,
  itemMaxFontScale: o,
  m1: o,
  n1: t,
  itemMinFontScale: u,
  o1: u,
  q1: a1,
};
const c1 = {
  buttonBackgroundColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.ohos_id_color_button_normal'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  buttonBorderRadius: {
    id: -1,
    type: 10002,
    params: ['sys.float.corner_radius_level9'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  buttonMinHeight: 36,
  i1: 64,
  buttonPadding: {
    id: -1,
    type: 10002,
    params: ['sys.float.padding_level1'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSelectedBackgroundColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.brand'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemBorderRadius: {
    id: -1,
    type: 10002,
    params: ['sys.float.corner_radius_level8'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  j1: {
    id: -1,
    type: 10002,
    params: ['sys.float.corner_radius_level9'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSpace: LengthMetrics.vp(2),
  itemFontSize: {
    id: -1,
    type: 10002,
    params: ['sys.float.ohos_id_text_size_button2'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemFontColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_secondary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSelectedFontColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_on_primary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemFontWeight: FontWeight.Medium,
  itemSelectedFontWeight: FontWeight.Medium,
  itemIconSize: 24,
  itemIconFillColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.icon_secondary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSelectedIconFillColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_on_primary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSymbolFontSize: 20,
  itemSymbolFontColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_secondary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSelectedSymbolFontColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_on_primary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemMinHeight: 32,
  l1: 60,
  itemPadding: {
    top: LengthMetrics.resource({
      id: -1,
      type: 10002,
      params: ['sys.float.padding_level2'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    }),
    bottom: LengthMetrics.resource({
      id: -1,
      type: 10002,
      params: ['sys.float.padding_level2'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    }),
    start: LengthMetrics.resource({
      id: -1,
      type: 10002,
      params: ['sys.float.padding_level8'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    }),
    end: LengthMetrics.resource({
      id: -1,
      type: 10002,
      params: ['sys.float.padding_level8'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    }),
  },
  itemShadow: ShadowStyle.OUTER_DEFAULT_XS,
  itemMaxFontScale: o,
  m1: o,
  n1: t,
  itemMinFontScale: u,
  o1: u,
  q1: a1,
};
let SegmentButtonV2Item = class SegmentButtonV2Item {
  constructor(options) {
    this.text = options.text;
    this.icon = options.icon;
    this.symbol = options.symbol;
    this.enabled = options.enabled ?? true;
    this.textModifier = options.textModifier;
    this.iconModifier = options.iconModifier;
    this.symbolModifier = options.symbolModifier;
    this.accessibilityText = options.accessibilityText;
    this.accessibilityDescription = options.accessibilityDescription;
    this.accessibilityLevel = options.accessibilityLevel;
  }
  get isHybrid() {
    return !!this.text && (!!this.icon || !!this.symbol);
  }
};
m([Trace], SegmentButtonV2Item.prototype, 'text', void 0);
m([Trace], SegmentButtonV2Item.prototype, 'icon', void 0);
m([Trace], SegmentButtonV2Item.prototype, 'symbol', void 0);
m([Trace], SegmentButtonV2Item.prototype, 'enabled', void 0);
m([Trace], SegmentButtonV2Item.prototype, 'textModifier', void 0);
m([Trace], SegmentButtonV2Item.prototype, 'iconModifier', void 0);
m([Trace], SegmentButtonV2Item.prototype, 'symbolModifier', void 0);
m([Trace], SegmentButtonV2Item.prototype, 'accessibilityText', void 0);
m([Trace], SegmentButtonV2Item.prototype, 'accessibilityDescription', void 0);
m([Trace], SegmentButtonV2Item.prototype, 'accessibilityLevel', void 0);
m([Computed], SegmentButtonV2Item.prototype, 'isHybrid', null);
SegmentButtonV2Item = m([ObservedV2], SegmentButtonV2Item);
export { SegmentButtonV2Item };
let SegmentButtonV2Items = class SegmentButtonV2Items extends Array {
  constructor(items) {
    super();
    for (let options of items) {
      if (options) {
        this.push(new SegmentButtonV2Item(options));
      }
    }
  }
  get hasHybrid() {
    return this.some((item) => item.isHybrid);
  }
};
m([Computed], SegmentButtonV2Items.prototype, 'hasHybrid', null);
SegmentButtonV2Items = m([ObservedV2], SegmentButtonV2Items);
export { SegmentButtonV2Items };
export class TabSegmentButtonV2 extends ViewV2 {
  constructor(
    parent,
    params,
    __localStorage,
    elmtId = -1,
    paramsLambda,
    extraInfo
  ) {
    super(parent, elmtId, extraInfo);
    this.initParam(
      'items',
      params && 'items' in params ? params.items : undefined
    );
    this.initParam(
      'selectedIndex',
      params && 'selectedIndex' in params ? params.selectedIndex : undefined
    );
    this.$selectedIndex =
      '$selectedIndex' in params ? params.$selectedIndex : () => {};
    this.onItemClicked =
      'onItemClicked' in params ? params.onItemClicked : () => {};
    this.initParam(
      'itemMinFontScale',
      params && 'itemMinFontScale' in params
        ? params.itemMinFontScale
        : undefined
    );
    this.initParam(
      'itemMaxFontScale',
      params && 'itemMaxFontScale' in params
        ? params.itemMaxFontScale
        : undefined
    );
    this.initParam(
      'itemSpace',
      params && 'itemSpace' in params ? params.itemSpace : undefined
    );
    this.initParam(
      'itemFontSize',
      params && 'itemFontSize' in params ? params.itemFontSize : undefined
    );
    this.initParam(
      'itemSelectedFontSize',
      params && 'itemSelectedFontSize' in params
        ? params.itemSelectedFontSize
        : undefined
    );
    this.initParam(
      'itemFontColor',
      params && 'itemFontColor' in params ? params.itemFontColor : undefined
    );
    this.initParam(
      'itemSelectedFontColor',
      params && 'itemSelectedFontColor' in params
        ? params.itemSelectedFontColor
        : undefined
    );
    this.initParam(
      'itemFontWeight',
      params && 'itemFontWeight' in params ? params.itemFontWeight : undefined
    );
    this.initParam(
      'itemSelectedFontWeight',
      params && 'itemSelectedFontWeight' in params
        ? params.itemSelectedFontWeight
        : undefined
    );
    this.initParam(
      'itemBorderRadius',
      params && 'itemBorderRadius' in params
        ? params.itemBorderRadius
        : undefined
    );
    this.initParam(
      'itemSelectedBackgroundColor',
      params && 'itemSelectedBackgroundColor' in params
        ? params.itemSelectedBackgroundColor
        : undefined
    );
    this.initParam(
      'itemIconSize',
      params && 'itemIconSize' in params ? params.itemIconSize : undefined
    );
    this.initParam(
      'itemIconFillColor',
      params && 'itemIconFillColor' in params
        ? params.itemIconFillColor
        : undefined
    );
    this.initParam(
      'itemSelectedIconFillColor',
      params && 'itemSelectedIconFillColor' in params
        ? params.itemSelectedIconFillColor
        : undefined
    );
    this.initParam(
      'itemSymbolFontSize',
      params && 'itemSymbolFontSize' in params
        ? params.itemSymbolFontSize
        : undefined
    );
    this.initParam(
      'itemSymbolFontColor',
      params && 'itemSymbolFontColor' in params
        ? params.itemSymbolFontColor
        : undefined
    );
    this.initParam(
      'itemSelectedSymbolFontColor',
      params && 'itemSelectedSymbolFontColor' in params
        ? params.itemSelectedSymbolFontColor
        : undefined
    );
    this.initParam(
      'itemMinHeight',
      params && 'itemMinHeight' in params ? params.itemMinHeight : undefined
    );
    this.initParam(
      'itemPadding',
      params && 'itemPadding' in params ? params.itemPadding : undefined
    );
    this.initParam(
      'itemShadow',
      params && 'itemShadow' in params ? params.itemShadow : undefined
    );
    this.initParam(
      'buttonBackgroundColor',
      params && 'buttonBackgroundColor' in params
        ? params.buttonBackgroundColor
        : undefined
    );
    this.initParam(
      'buttonBackgroundBlurStyle',
      params && 'buttonBackgroundBlurStyle' in params
        ? params.buttonBackgroundBlurStyle
        : undefined
    );
    this.initParam(
      'buttonBackgroundBlurStyleOptions',
      params && 'buttonBackgroundBlurStyleOptions' in params
        ? params.buttonBackgroundBlurStyleOptions
        : undefined
    );
    this.initParam(
      'buttonBackgroundEffect',
      params && 'buttonBackgroundEffect' in params
        ? params.buttonBackgroundEffect
        : undefined
    );
    this.initParam(
      'buttonBorderRadius',
      params && 'buttonBorderRadius' in params
        ? params.buttonBorderRadius
        : undefined
    );
    this.initParam(
      'buttonMinHeight',
      params && 'buttonMinHeight' in params ? params.buttonMinHeight : undefined
    );
    this.initParam(
      'buttonPadding',
      params && 'buttonPadding' in params ? params.buttonPadding : undefined
    );
    this.initParam(
      'languageDirection',
      params && 'languageDirection' in params
        ? params.languageDirection
        : undefined
    );
    this.finalizeConstruction();
  }
  initialRender() {
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.push(this);
    {
      this.observeComponentCreation2(
        (elmtId, isInitialRender) => {
          if (isInitialRender) {
            let componentCall = new d1(
              this,
              {
                theme: b1,
                items: this.items,
                selectedIndex: this.selectedIndex,
                $selectedIndex: (selectedIndex) => {
                  this.$selectedIndex?.(selectedIndex);
                },
                onItemClicked: this.onItemClicked,
                itemMinFontScale: this.itemMinFontScale,
                itemMaxFontScale: this.itemMaxFontScale,
                itemSpace: this.itemSpace,
                itemFontColor: this.itemFontColor,
                itemSelectedFontColor: this.itemSelectedFontColor,
                itemFontSize: this.itemFontSize,
                itemSelectedFontSize: this.itemSelectedFontSize,
                itemFontWeight: this.itemFontWeight,
                itemSelectedFontWeight: this.itemSelectedFontWeight,
                itemSelectedBackgroundColor: this.itemSelectedBackgroundColor,
                itemIconSize: this.itemIconSize,
                itemIconFillColor: this.itemIconFillColor,
                itemSelectedIconFillColor: this.itemSelectedIconFillColor,
                itemSymbolFontSize: this.itemSymbolFontSize,
                itemSymbolFontColor: this.itemSymbolFontColor,
                itemSelectedSymbolFontColor: this.itemSelectedSymbolFontColor,
                itemBorderRadius: this.itemBorderRadius,
                itemMinHeight: this.itemMinHeight,
                itemPadding: this.itemPadding,
                itemShadow: this.itemShadow,
                buttonBackgroundColor: this.buttonBackgroundColor,
                buttonBackgroundBlurStyle: this.buttonBackgroundBlurStyle,
                buttonBackgroundBlurStyleOptions:
                  this.buttonBackgroundBlurStyleOptions,
                buttonBackgroundEffect: this.buttonBackgroundEffect,
                buttonBorderRadius: this.buttonBorderRadius,
                buttonMinHeight: this.buttonMinHeight,
                buttonPadding: this.buttonPadding,
                languageDirection: this.languageDirection,
              },
              undefined,
              elmtId,
              () => {},
              {
                page: 'advancedComponents/src/main/ets/components/SegmentButtonV2.ets',
                line: 289,
                s1: 5,
              }
            );
            ViewV2.create(componentCall);
            let paramsLambda = () => {
              return {
                theme: b1,
                items: this.items,
                selectedIndex: this.selectedIndex,
                $selectedIndex: (selectedIndex) => {
                  this.$selectedIndex?.(selectedIndex);
                },
                onItemClicked: this.onItemClicked,
                itemMinFontScale: this.itemMinFontScale,
                itemMaxFontScale: this.itemMaxFontScale,
                itemSpace: this.itemSpace,
                itemFontColor: this.itemFontColor,
                itemSelectedFontColor: this.itemSelectedFontColor,
                itemFontSize: this.itemFontSize,
                itemSelectedFontSize: this.itemSelectedFontSize,
                itemFontWeight: this.itemFontWeight,
                itemSelectedFontWeight: this.itemSelectedFontWeight,
                itemSelectedBackgroundColor: this.itemSelectedBackgroundColor,
                itemIconSize: this.itemIconSize,
                itemIconFillColor: this.itemIconFillColor,
                itemSelectedIconFillColor: this.itemSelectedIconFillColor,
                itemSymbolFontSize: this.itemSymbolFontSize,
                itemSymbolFontColor: this.itemSymbolFontColor,
                itemSelectedSymbolFontColor: this.itemSelectedSymbolFontColor,
                itemBorderRadius: this.itemBorderRadius,
                itemMinHeight: this.itemMinHeight,
                itemPadding: this.itemPadding,
                itemShadow: this.itemShadow,
                buttonBackgroundColor: this.buttonBackgroundColor,
                buttonBackgroundBlurStyle: this.buttonBackgroundBlurStyle,
                buttonBackgroundBlurStyleOptions:
                  this.buttonBackgroundBlurStyleOptions,
                buttonBackgroundEffect: this.buttonBackgroundEffect,
                buttonBorderRadius: this.buttonBorderRadius,
                buttonMinHeight: this.buttonMinHeight,
                buttonPadding: this.buttonPadding,
                languageDirection: this.languageDirection,
              };
            };
            componentCall.paramsGenerator_ = paramsLambda;
          } else {
            this.updateStateVarsOfChildByElmtId(elmtId, {
              theme: b1,
              items: this.items,
              selectedIndex: this.selectedIndex,
              itemMinFontScale: this.itemMinFontScale,
              itemMaxFontScale: this.itemMaxFontScale,
              itemSpace: this.itemSpace,
              itemFontColor: this.itemFontColor,
              itemSelectedFontColor: this.itemSelectedFontColor,
              itemFontSize: this.itemFontSize,
              itemSelectedFontSize: this.itemSelectedFontSize,
              itemFontWeight: this.itemFontWeight,
              itemSelectedFontWeight: this.itemSelectedFontWeight,
              itemSelectedBackgroundColor: this.itemSelectedBackgroundColor,
              itemIconSize: this.itemIconSize,
              itemIconFillColor: this.itemIconFillColor,
              itemSelectedIconFillColor: this.itemSelectedIconFillColor,
              itemSymbolFontSize: this.itemSymbolFontSize,
              itemSymbolFontColor: this.itemSymbolFontColor,
              itemSelectedSymbolFontColor: this.itemSelectedSymbolFontColor,
              itemBorderRadius: this.itemBorderRadius,
              itemMinHeight: this.itemMinHeight,
              itemPadding: this.itemPadding,
              itemShadow: this.itemShadow,
              buttonBackgroundColor: this.buttonBackgroundColor,
              buttonBackgroundBlurStyle: this.buttonBackgroundBlurStyle,
              buttonBackgroundBlurStyleOptions:
                this.buttonBackgroundBlurStyleOptions,
              buttonBackgroundEffect: this.buttonBackgroundEffect,
              buttonBorderRadius: this.buttonBorderRadius,
              buttonMinHeight: this.buttonMinHeight,
              buttonPadding: this.buttonPadding,
              languageDirection: this.languageDirection,
            });
          }
        },
        { name: 'SimpleSegmentButtonV2' }
      );
    }
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.pop();
  }
  updateStateVars(params) {
    if (params === undefined) {
      return;
    }
    if ('items' in params) {
      this.updateParam('items', params.items);
    }
    if ('selectedIndex' in params) {
      this.updateParam('selectedIndex', params.selectedIndex);
    }
    if ('itemMinFontScale' in params) {
      this.updateParam('itemMinFontScale', params.itemMinFontScale);
    }
    if ('itemMaxFontScale' in params) {
      this.updateParam('itemMaxFontScale', params.itemMaxFontScale);
    }
    if ('itemSpace' in params) {
      this.updateParam('itemSpace', params.itemSpace);
    }
    if ('itemFontSize' in params) {
      this.updateParam('itemFontSize', params.itemFontSize);
    }
    if ('itemSelectedFontSize' in params) {
      this.updateParam('itemSelectedFontSize', params.itemSelectedFontSize);
    }
    if ('itemFontColor' in params) {
      this.updateParam('itemFontColor', params.itemFontColor);
    }
    if ('itemSelectedFontColor' in params) {
      this.updateParam('itemSelectedFontColor', params.itemSelectedFontColor);
    }
    if ('itemFontWeight' in params) {
      this.updateParam('itemFontWeight', params.itemFontWeight);
    }
    if ('itemSelectedFontWeight' in params) {
      this.updateParam('itemSelectedFontWeight', params.itemSelectedFontWeight);
    }
    if ('itemBorderRadius' in params) {
      this.updateParam('itemBorderRadius', params.itemBorderRadius);
    }
    if ('itemSelectedBackgroundColor' in params) {
      this.updateParam(
        'itemSelectedBackgroundColor',
        params.itemSelectedBackgroundColor
      );
    }
    if ('itemIconSize' in params) {
      this.updateParam('itemIconSize', params.itemIconSize);
    }
    if ('itemIconFillColor' in params) {
      this.updateParam('itemIconFillColor', params.itemIconFillColor);
    }
    if ('itemSelectedIconFillColor' in params) {
      this.updateParam(
        'itemSelectedIconFillColor',
        params.itemSelectedIconFillColor
      );
    }
    if ('itemSymbolFontSize' in params) {
      this.updateParam('itemSymbolFontSize', params.itemSymbolFontSize);
    }
    if ('itemSymbolFontColor' in params) {
      this.updateParam('itemSymbolFontColor', params.itemSymbolFontColor);
    }
    if ('itemSelectedSymbolFontColor' in params) {
      this.updateParam(
        'itemSelectedSymbolFontColor',
        params.itemSelectedSymbolFontColor
      );
    }
    if ('itemMinHeight' in params) {
      this.updateParam('itemMinHeight', params.itemMinHeight);
    }
    if ('itemPadding' in params) {
      this.updateParam('itemPadding', params.itemPadding);
    }
    if ('itemShadow' in params) {
      this.updateParam('itemShadow', params.itemShadow);
    }
    if ('buttonBackgroundColor' in params) {
      this.updateParam('buttonBackgroundColor', params.buttonBackgroundColor);
    }
    if ('buttonBackgroundBlurStyle' in params) {
      this.updateParam(
        'buttonBackgroundBlurStyle',
        params.buttonBackgroundBlurStyle
      );
    }
    if ('buttonBackgroundBlurStyleOptions' in params) {
      this.updateParam(
        'buttonBackgroundBlurStyleOptions',
        params.buttonBackgroundBlurStyleOptions
      );
    }
    if ('buttonBackgroundEffect' in params) {
      this.updateParam('buttonBackgroundEffect', params.buttonBackgroundEffect);
    }
    if ('buttonBorderRadius' in params) {
      this.updateParam('buttonBorderRadius', params.buttonBorderRadius);
    }
    if ('buttonMinHeight' in params) {
      this.updateParam('buttonMinHeight', params.buttonMinHeight);
    }
    if ('buttonPadding' in params) {
      this.updateParam('buttonPadding', params.buttonPadding);
    }
    if ('languageDirection' in params) {
      this.updateParam('languageDirection', params.languageDirection);
    }
  }
  rerender() {
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.push(this);
    this.updateDirtyElements();
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.pop();
  }
}
m([Param], TabSegmentButtonV2.prototype, 'items', void 0);
m([Param], TabSegmentButtonV2.prototype, 'selectedIndex', void 0);
m([Event], TabSegmentButtonV2.prototype, '$selectedIndex', void 0);
m([Event], TabSegmentButtonV2.prototype, 'onItemClicked', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemMinFontScale', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemMaxFontScale', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemSpace', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemFontSize', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemSelectedFontSize', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemFontColor', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemSelectedFontColor', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemFontWeight', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemSelectedFontWeight', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemBorderRadius', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemSelectedBackgroundColor', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemIconSize', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemIconFillColor', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemSelectedIconFillColor', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemSymbolFontSize', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemSymbolFontColor', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemSelectedSymbolFontColor', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemMinHeight', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemPadding', void 0);
m([Param], TabSegmentButtonV2.prototype, 'itemShadow', void 0);
m([Param], TabSegmentButtonV2.prototype, 'buttonBackgroundColor', void 0);
m([Param], TabSegmentButtonV2.prototype, 'buttonBackgroundBlurStyle', void 0);
m(
  [Param],
  TabSegmentButtonV2.prototype,
  'buttonBackgroundBlurStyleOptions',
  void 0
);
m([Param], TabSegmentButtonV2.prototype, 'buttonBackgroundEffect', void 0);
m([Param], TabSegmentButtonV2.prototype, 'buttonBorderRadius', void 0);
m([Param], TabSegmentButtonV2.prototype, 'buttonMinHeight', void 0);
m([Param], TabSegmentButtonV2.prototype, 'buttonPadding', void 0);
m([Param], TabSegmentButtonV2.prototype, 'languageDirection', void 0);
export class CapsuleSegmentButtonV2 extends ViewV2 {
  constructor(
    parent,
    params,
    __localStorage,
    elmtId = -1,
    paramsLambda,
    extraInfo
  ) {
    super(parent, elmtId, extraInfo);
    this.initParam(
      'items',
      params && 'items' in params ? params.items : undefined
    );
    this.initParam(
      'selectedIndex',
      params && 'selectedIndex' in params ? params.selectedIndex : undefined
    );
    this.$selectedIndex =
      '$selectedIndex' in params ? params.$selectedIndex : () => {};
    this.onItemClicked =
      'onItemClicked' in params ? params.onItemClicked : () => {};
    this.initParam(
      'itemMinFontScale',
      params && 'itemMinFontScale' in params
        ? params.itemMinFontScale
        : undefined
    );
    this.initParam(
      'itemMaxFontScale',
      params && 'itemMaxFontScale' in params
        ? params.itemMaxFontScale
        : undefined
    );
    this.initParam(
      'itemSpace',
      params && 'itemSpace' in params ? params.itemSpace : undefined
    );
    this.initParam(
      'itemFontColor',
      params && 'itemFontColor' in params ? params.itemFontColor : undefined
    );
    this.initParam(
      'itemSelectedFontColor',
      params && 'itemSelectedFontColor' in params
        ? params.itemSelectedFontColor
        : undefined
    );
    this.initParam(
      'itemFontSize',
      params && 'itemFontSize' in params ? params.itemFontSize : undefined
    );
    this.initParam(
      'itemSelectedFontSize',
      params && 'itemSelectedFontSize' in params
        ? params.itemSelectedFontSize
        : undefined
    );
    this.initParam(
      'itemFontWeight',
      params && 'itemFontWeight' in params ? params.itemFontWeight : undefined
    );
    this.initParam(
      'itemSelectedFontWeight',
      params && 'itemSelectedFontWeight' in params
        ? params.itemSelectedFontWeight
        : undefined
    );
    this.initParam(
      'itemBorderRadius',
      params && 'itemBorderRadius' in params
        ? params.itemBorderRadius
        : undefined
    );
    this.initParam(
      'itemSelectedBackgroundColor',
      params && 'itemSelectedBackgroundColor' in params
        ? params.itemSelectedBackgroundColor
        : undefined
    );
    this.initParam(
      'itemIconSize',
      params && 'itemIconSize' in params ? params.itemIconSize : undefined
    );
    this.initParam(
      'itemIconFillColor',
      params && 'itemIconFillColor' in params
        ? params.itemIconFillColor
        : undefined
    );
    this.initParam(
      'itemSelectedIconFillColor',
      params && 'itemSelectedIconFillColor' in params
        ? params.itemSelectedIconFillColor
        : undefined
    );
    this.initParam(
      'itemSymbolFontSize',
      params && 'itemSymbolFontSize' in params
        ? params.itemSymbolFontSize
        : undefined
    );
    this.initParam(
      'itemSymbolFontColor',
      params && 'itemSymbolFontColor' in params
        ? params.itemSymbolFontColor
        : undefined
    );
    this.initParam(
      'itemSelectedSymbolFontColor',
      params && 'itemSelectedSymbolFontColor' in params
        ? params.itemSelectedSymbolFontColor
        : undefined
    );
    this.initParam(
      'itemMinHeight',
      params && 'itemMinHeight' in params ? params.itemMinHeight : undefined
    );
    this.initParam(
      'itemPadding',
      params && 'itemPadding' in params ? params.itemPadding : undefined
    );
    this.initParam(
      'itemShadow',
      params && 'itemShadow' in params ? params.itemShadow : undefined
    );
    this.initParam(
      'buttonBackgroundColor',
      params && 'buttonBackgroundColor' in params
        ? params.buttonBackgroundColor
        : undefined
    );
    this.initParam(
      'buttonBackgroundBlurStyle',
      params && 'buttonBackgroundBlurStyle' in params
        ? params.buttonBackgroundBlurStyle
        : undefined
    );
    this.initParam(
      'buttonBackgroundBlurStyleOptions',
      params && 'buttonBackgroundBlurStyleOptions' in params
        ? params.buttonBackgroundBlurStyleOptions
        : undefined
    );
    this.initParam(
      'buttonBackgroundEffect',
      params && 'buttonBackgroundEffect' in params
        ? params.buttonBackgroundEffect
        : undefined
    );
    this.initParam(
      'buttonBorderRadius',
      params && 'buttonBorderRadius' in params
        ? params.buttonBorderRadius
        : undefined
    );
    this.initParam(
      'buttonMinHeight',
      params && 'buttonMinHeight' in params ? params.buttonMinHeight : undefined
    );
    this.initParam(
      'buttonPadding',
      params && 'buttonPadding' in params ? params.buttonPadding : undefined
    );
    this.initParam(
      'languageDirection',
      params && 'languageDirection' in params
        ? params.languageDirection
        : undefined
    );
    this.finalizeConstruction();
  }
  initialRender() {
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.push(this);
    {
      this.observeComponentCreation2(
        (elmtId, isInitialRender) => {
          if (isInitialRender) {
            let componentCall = new d1(
              this,
              {
                theme: c1,
                items: this.items,
                selectedIndex: this.selectedIndex,
                $selectedIndex: (selectedIndex) => {
                  this.$selectedIndex?.(selectedIndex);
                },
                onItemClicked: this.onItemClicked,
                itemMinFontScale: this.itemMinFontScale,
                itemMaxFontScale: this.itemMaxFontScale,
                itemSpace: this.itemSpace,
                itemFontColor: this.itemFontColor,
                itemSelectedFontColor: this.itemSelectedFontColor,
                itemFontSize: this.itemFontSize,
                itemSelectedFontSize: this.itemSelectedFontSize,
                itemFontWeight: this.itemFontWeight,
                itemSelectedFontWeight: this.itemSelectedFontWeight,
                itemSelectedBackgroundColor: this.itemSelectedBackgroundColor,
                itemIconSize: this.itemIconSize,
                itemIconFillColor: this.itemIconFillColor,
                itemSelectedIconFillColor: this.itemSelectedIconFillColor,
                itemSymbolFontSize: this.itemSymbolFontSize,
                itemSymbolFontColor: this.itemSymbolFontColor,
                itemSelectedSymbolFontColor: this.itemSelectedSymbolFontColor,
                itemBorderRadius: this.itemBorderRadius,
                itemMinHeight: this.itemMinHeight,
                itemPadding: this.itemPadding,
                itemShadow: this.itemShadow,
                buttonBackgroundColor: this.buttonBackgroundColor,
                buttonBackgroundBlurStyle: this.buttonBackgroundBlurStyle,
                buttonBackgroundBlurStyleOptions:
                  this.buttonBackgroundBlurStyleOptions,
                buttonBackgroundEffect: this.buttonBackgroundEffect,
                buttonBorderRadius: this.buttonBorderRadius,
                buttonMinHeight: this.buttonMinHeight,
                buttonPadding: this.buttonPadding,
                languageDirection: this.languageDirection,
              },
              undefined,
              elmtId,
              () => {},
              {
                page: 'advancedComponents/src/main/ets/components/SegmentButtonV2.ets',
                line: 399,
                s1: 5,
              }
            );
            ViewV2.create(componentCall);
            let paramsLambda = () => {
              return {
                theme: c1,
                items: this.items,
                selectedIndex: this.selectedIndex,
                $selectedIndex: (selectedIndex) => {
                  this.$selectedIndex?.(selectedIndex);
                },
                onItemClicked: this.onItemClicked,
                itemMinFontScale: this.itemMinFontScale,
                itemMaxFontScale: this.itemMaxFontScale,
                itemSpace: this.itemSpace,
                itemFontColor: this.itemFontColor,
                itemSelectedFontColor: this.itemSelectedFontColor,
                itemFontSize: this.itemFontSize,
                itemSelectedFontSize: this.itemSelectedFontSize,
                itemFontWeight: this.itemFontWeight,
                itemSelectedFontWeight: this.itemSelectedFontWeight,
                itemSelectedBackgroundColor: this.itemSelectedBackgroundColor,
                itemIconSize: this.itemIconSize,
                itemIconFillColor: this.itemIconFillColor,
                itemSelectedIconFillColor: this.itemSelectedIconFillColor,
                itemSymbolFontSize: this.itemSymbolFontSize,
                itemSymbolFontColor: this.itemSymbolFontColor,
                itemSelectedSymbolFontColor: this.itemSelectedSymbolFontColor,
                itemBorderRadius: this.itemBorderRadius,
                itemMinHeight: this.itemMinHeight,
                itemPadding: this.itemPadding,
                itemShadow: this.itemShadow,
                buttonBackgroundColor: this.buttonBackgroundColor,
                buttonBackgroundBlurStyle: this.buttonBackgroundBlurStyle,
                buttonBackgroundBlurStyleOptions:
                  this.buttonBackgroundBlurStyleOptions,
                buttonBackgroundEffect: this.buttonBackgroundEffect,
                buttonBorderRadius: this.buttonBorderRadius,
                buttonMinHeight: this.buttonMinHeight,
                buttonPadding: this.buttonPadding,
                languageDirection: this.languageDirection,
              };
            };
            componentCall.paramsGenerator_ = paramsLambda;
          } else {
            this.updateStateVarsOfChildByElmtId(elmtId, {
              theme: c1,
              items: this.items,
              selectedIndex: this.selectedIndex,
              itemMinFontScale: this.itemMinFontScale,
              itemMaxFontScale: this.itemMaxFontScale,
              itemSpace: this.itemSpace,
              itemFontColor: this.itemFontColor,
              itemSelectedFontColor: this.itemSelectedFontColor,
              itemFontSize: this.itemFontSize,
              itemSelectedFontSize: this.itemSelectedFontSize,
              itemFontWeight: this.itemFontWeight,
              itemSelectedFontWeight: this.itemSelectedFontWeight,
              itemSelectedBackgroundColor: this.itemSelectedBackgroundColor,
              itemIconSize: this.itemIconSize,
              itemIconFillColor: this.itemIconFillColor,
              itemSelectedIconFillColor: this.itemSelectedIconFillColor,
              itemSymbolFontSize: this.itemSymbolFontSize,
              itemSymbolFontColor: this.itemSymbolFontColor,
              itemSelectedSymbolFontColor: this.itemSelectedSymbolFontColor,
              itemBorderRadius: this.itemBorderRadius,
              itemMinHeight: this.itemMinHeight,
              itemPadding: this.itemPadding,
              itemShadow: this.itemShadow,
              buttonBackgroundColor: this.buttonBackgroundColor,
              buttonBackgroundBlurStyle: this.buttonBackgroundBlurStyle,
              buttonBackgroundBlurStyleOptions:
                this.buttonBackgroundBlurStyleOptions,
              buttonBackgroundEffect: this.buttonBackgroundEffect,
              buttonBorderRadius: this.buttonBorderRadius,
              buttonMinHeight: this.buttonMinHeight,
              buttonPadding: this.buttonPadding,
              languageDirection: this.languageDirection,
            });
          }
        },
        { name: 'SimpleSegmentButtonV2' }
      );
    }
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.pop();
  }
  updateStateVars(params) {
    if (params === undefined) {
      return;
    }
    if ('items' in params) {
      this.updateParam('items', params.items);
    }
    if ('selectedIndex' in params) {
      this.updateParam('selectedIndex', params.selectedIndex);
    }
    if ('itemMinFontScale' in params) {
      this.updateParam('itemMinFontScale', params.itemMinFontScale);
    }
    if ('itemMaxFontScale' in params) {
      this.updateParam('itemMaxFontScale', params.itemMaxFontScale);
    }
    if ('itemSpace' in params) {
      this.updateParam('itemSpace', params.itemSpace);
    }
    if ('itemFontColor' in params) {
      this.updateParam('itemFontColor', params.itemFontColor);
    }
    if ('itemSelectedFontColor' in params) {
      this.updateParam('itemSelectedFontColor', params.itemSelectedFontColor);
    }
    if ('itemFontSize' in params) {
      this.updateParam('itemFontSize', params.itemFontSize);
    }
    if ('itemSelectedFontSize' in params) {
      this.updateParam('itemSelectedFontSize', params.itemSelectedFontSize);
    }
    if ('itemFontWeight' in params) {
      this.updateParam('itemFontWeight', params.itemFontWeight);
    }
    if ('itemSelectedFontWeight' in params) {
      this.updateParam('itemSelectedFontWeight', params.itemSelectedFontWeight);
    }
    if ('itemBorderRadius' in params) {
      this.updateParam('itemBorderRadius', params.itemBorderRadius);
    }
    if ('itemSelectedBackgroundColor' in params) {
      this.updateParam(
        'itemSelectedBackgroundColor',
        params.itemSelectedBackgroundColor
      );
    }
    if ('itemIconSize' in params) {
      this.updateParam('itemIconSize', params.itemIconSize);
    }
    if ('itemIconFillColor' in params) {
      this.updateParam('itemIconFillColor', params.itemIconFillColor);
    }
    if ('itemSelectedIconFillColor' in params) {
      this.updateParam(
        'itemSelectedIconFillColor',
        params.itemSelectedIconFillColor
      );
    }
    if ('itemSymbolFontSize' in params) {
      this.updateParam('itemSymbolFontSize', params.itemSymbolFontSize);
    }
    if ('itemSymbolFontColor' in params) {
      this.updateParam('itemSymbolFontColor', params.itemSymbolFontColor);
    }
    if ('itemSelectedSymbolFontColor' in params) {
      this.updateParam(
        'itemSelectedSymbolFontColor',
        params.itemSelectedSymbolFontColor
      );
    }
    if ('itemMinHeight' in params) {
      this.updateParam('itemMinHeight', params.itemMinHeight);
    }
    if ('itemPadding' in params) {
      this.updateParam('itemPadding', params.itemPadding);
    }
    if ('itemShadow' in params) {
      this.updateParam('itemShadow', params.itemShadow);
    }
    if ('buttonBackgroundColor' in params) {
      this.updateParam('buttonBackgroundColor', params.buttonBackgroundColor);
    }
    if ('buttonBackgroundBlurStyle' in params) {
      this.updateParam(
        'buttonBackgroundBlurStyle',
        params.buttonBackgroundBlurStyle
      );
    }
    if ('buttonBackgroundBlurStyleOptions' in params) {
      this.updateParam(
        'buttonBackgroundBlurStyleOptions',
        params.buttonBackgroundBlurStyleOptions
      );
    }
    if ('buttonBackgroundEffect' in params) {
      this.updateParam('buttonBackgroundEffect', params.buttonBackgroundEffect);
    }
    if ('buttonBorderRadius' in params) {
      this.updateParam('buttonBorderRadius', params.buttonBorderRadius);
    }
    if ('buttonMinHeight' in params) {
      this.updateParam('buttonMinHeight', params.buttonMinHeight);
    }
    if ('buttonPadding' in params) {
      this.updateParam('buttonPadding', params.buttonPadding);
    }
    if ('languageDirection' in params) {
      this.updateParam('languageDirection', params.languageDirection);
    }
  }
  rerender() {
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.push(this);
    this.updateDirtyElements();
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.pop();
  }
}
m([Param], CapsuleSegmentButtonV2.prototype, 'items', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'selectedIndex', void 0);
m([Event], CapsuleSegmentButtonV2.prototype, '$selectedIndex', void 0);
m([Event], CapsuleSegmentButtonV2.prototype, 'onItemClicked', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemMinFontScale', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemMaxFontScale', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemSpace', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemFontColor', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemSelectedFontColor', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemFontSize', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemSelectedFontSize', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemFontWeight', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemSelectedFontWeight', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemBorderRadius', void 0);
m(
  [Param],
  CapsuleSegmentButtonV2.prototype,
  'itemSelectedBackgroundColor',
  void 0
);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemIconSize', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemIconFillColor', void 0);
m(
  [Param],
  CapsuleSegmentButtonV2.prototype,
  'itemSelectedIconFillColor',
  void 0
);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemSymbolFontSize', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemSymbolFontColor', void 0);
m(
  [Param],
  CapsuleSegmentButtonV2.prototype,
  'itemSelectedSymbolFontColor',
  void 0
);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemMinHeight', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemPadding', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'itemShadow', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'buttonBackgroundColor', void 0);
m(
  [Param],
  CapsuleSegmentButtonV2.prototype,
  'buttonBackgroundBlurStyle',
  void 0
);
m(
  [Param],
  CapsuleSegmentButtonV2.prototype,
  'buttonBackgroundBlurStyleOptions',
  void 0
);
m([Param], CapsuleSegmentButtonV2.prototype, 'buttonBackgroundEffect', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'buttonBorderRadius', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'buttonMinHeight', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'buttonPadding', void 0);
m([Param], CapsuleSegmentButtonV2.prototype, 'languageDirection', void 0);
class d1 extends ViewV2 {
  constructor(
    parent,
    params,
    __localStorage,
    elmtId = -1,
    paramsLambda,
    extraInfo
  ) {
    super(parent, elmtId, extraInfo);
    this.ContentLayer = () => {
      const parent =
        PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.length
          ? PUV2ViewBase.contextStack[PUV2ViewBase.contextStack.length - 1]
          : null;
      this.observeComponentCreation2((elmtId, isInitialRender) => {
        Flex.create({
          alignItems: ItemAlign.Stretch,
          space: { main: this.getItemSpace() },
        });
        Flex.constraintSize({
          minWidth: '100%',
          minHeight: this.getButtonMinHeight(),
        });
        Flex.padding(this.getButtonPadding());
        Flex.direction(this.languageDirection);
        Gesture.create(GesturePriority.High);
        PanGesture.create();
        PanGesture.onActionStart((event) => {
          if (event.axisHorizontal !== 0 || event.axisVertical !== 0) {
            this.isMouseWheelScroll = true;
            return;
          }
          const finger = event.fingerList.find(Boolean);
          if (!finger) {
            return;
          }
          const index = this.getIndexByPosition(finger.globalX, finger.globalY);
          if (index === this.normalizedSelectedIndex) {
            this.isDragging = true;
          }
          this.panStartGlobalX = finger.globalX;
        });
        PanGesture.onActionUpdate((event) => {
          if (!this.isDragging) {
            return;
          }
          const finger = event.fingerList.find(Boolean);
          if (!finger) {
            return;
          }
          const index = this.getIndexByPosition(finger.globalX, finger.globalY);
          if (index !== -1) {
            this.updateSelectedIndex(index);
          }
        });
        PanGesture.onActionEnd((event) => {
          if (this.isMouseWheelScroll) {
            const offset = event.offsetX !== 0 ? event.offsetX : event.offsetY;
            const k1 = offset < 0 ? 1 : -1;
            this.updateSelectedIndex(this.normalizedSelectedIndex + k1);
            this.isMouseWheelScroll = false;
            return;
          }
          if (this.isDragging) {
            this.isDragging = false;
            return;
          }
          const finger = event.fingerList.find(Boolean);
          if (!finger) {
            return;
          }
          let j1 = finger.globalX - this.panStartGlobalX < 0 ? -1 : 1;
          if (this.isRTL()) {
            j1 = -j1;
          }
          this.updateSelectedIndex(this.normalizedSelectedIndex + j1);
        });
        PanGesture.onActionCancel(() => {
          this.isDragging = false;
          this.isMouseWheelScroll = false;
        });
        PanGesture.pop();
        Gesture.pop();
      }, Flex);
      this.observeComponentCreation2((elmtId, isInitialRender) => {
        Repeat(this.items, this)
          .each((repeatItem) => {
            this.observeComponentCreation2((elmtId, isInitialRender) => {
              Button.createWithChild({ type: ButtonType.Normal });
              Button.accessibilityGroup(true);
              Button.accessibilitySelected(this.isSelected(repeatItem));
              Button.accessibilityText(
                this.getItemAccessibilityText(repeatItem)
              );
              Button.accessibilityDescription(
                this.getItemAccessibilityDescription(repeatItem)
              );
              Button.accessibilityLevel(repeatItem.item.accessibilityLevel);
              Button.backgroundColor(Color.Transparent);
              Button.borderRadius(this.getItemBorderRadius());
              Button.enabled(repeatItem.item.enabled);
              Button.hoverEffect(HoverEffect.None);
              Button.layoutWeight(1);
              Button.padding(0);
              Button.scale(this.getItemScale(repeatItem.index));
              Button.stateEffect(false);
              Button.onAreaChange((i1, area) => {
                this.itemRects[repeatItem.index] = {
                  size: {
                    width: area.width,
                    height: area.height,
                  },
                  position: {
                    x: area.position.x,
                    y: area.position.y,
                  },
                  globalPosition: {
                    x: area.globalPosition.x,
                    y: area.globalPosition.y,
                  },
                };
              });
              Gesture.create(GesturePriority.Low);
              TapGesture.create();
              TapGesture.onAction(() => {
                this.onItemClicked?.(repeatItem.index);
                this.updateSelectedIndex(repeatItem.index);
              });
              TapGesture.pop();
              Gesture.pop();
              Button.onTouch((event) => {
                if (event.type === TouchType.Down) {
                  if (this.isSelected(repeatItem)) {
                    this.updateItemScale(0.95);
                  }
                  this.updateTouchPressedItemIndex(repeatItem.index);
                } else if (event.type === TouchType.Up) {
                  this.updateItemScale(1);
                  this.updateTouchPressedItemIndex(-1);
                }
              });
              Button.onHover((isHover) => {
                if (isHover) {
                  this.updateHoveredItemIndex(repeatItem.index);
                } else {
                  this.updateHoveredItemIndex(-1);
                }
              });
              Button.onMouse((event) => {
                if (event.action === MouseAction.Press) {
                  this.updateMousePressedItemIndex(repeatItem.index);
                } else if (event.action === MouseAction.Release) {
                  this.updateMousePressedItemIndex(-1);
                }
              });
            }, Button);
            {
              this.observeComponentCreation2(
                (elmtId, isInitialRender) => {
                  if (isInitialRender) {
                    let componentCall = new f1(
                      parent ? parent : this,
                      {
                        theme: this.theme,
                        item: repeatItem.item,
                        selected: this.isSelected(repeatItem),
                        itemMinFontScale: this.itemMinFontScale,
                        itemMaxFontScale: this.itemMaxFontScale,
                        itemFontColor: this.itemFontColor,
                        itemSelectedFontColor: this.itemSelectedFontColor,
                        itemFontSize: this.itemFontSize,
                        itemSelectedFontSize: this.itemSelectedFontSize,
                        itemFontWeight: this.itemFontWeight,
                        itemSelectedFontWeight: this.itemSelectedFontWeight,
                        itemIconSize: this.itemIconSize,
                        itemIconFillColor: this.itemIconFillColor,
                        itemSelectedIconFillColor:
                          this.itemSelectedIconFillColor,
                        itemSymbolFontSize: this.itemSymbolFontSize,
                        itemSymbolFontColor: this.itemSymbolFontColor,
                        itemSelectedSymbolFontColor:
                          this.itemSelectedSymbolFontColor,
                        itemMinHeight: this.itemMinHeight,
                        itemPadding: this.itemPadding,
                        languageDirection: this.languageDirection,
                        hasHybrid: this.items.hasHybrid,
                      },
                      undefined,
                      elmtId,
                      () => {},
                      {
                        page: 'advancedComponents/src/main/ets/components/SegmentButtonV2.ets',
                        line: 568,
                        s1: 13,
                      }
                    );
                    ViewV2.create(componentCall);
                    let paramsLambda = () => {
                      return {
                        theme: this.theme,
                        item: repeatItem.item,
                        selected: this.isSelected(repeatItem),
                        itemMinFontScale: this.itemMinFontScale,
                        itemMaxFontScale: this.itemMaxFontScale,
                        itemFontColor: this.itemFontColor,
                        itemSelectedFontColor: this.itemSelectedFontColor,
                        itemFontSize: this.itemFontSize,
                        itemSelectedFontSize: this.itemSelectedFontSize,
                        itemFontWeight: this.itemFontWeight,
                        itemSelectedFontWeight: this.itemSelectedFontWeight,
                        itemIconSize: this.itemIconSize,
                        itemIconFillColor: this.itemIconFillColor,
                        itemSelectedIconFillColor:
                          this.itemSelectedIconFillColor,
                        itemSymbolFontSize: this.itemSymbolFontSize,
                        itemSymbolFontColor: this.itemSymbolFontColor,
                        itemSelectedSymbolFontColor:
                          this.itemSelectedSymbolFontColor,
                        itemMinHeight: this.itemMinHeight,
                        itemPadding: this.itemPadding,
                        languageDirection: this.languageDirection,
                        hasHybrid: this.items.hasHybrid,
                      };
                    };
                    componentCall.paramsGenerator_ = paramsLambda;
                  } else {
                    this.updateStateVarsOfChildByElmtId(elmtId, {
                      theme: this.theme,
                      item: repeatItem.item,
                      selected: this.isSelected(repeatItem),
                      itemMinFontScale: this.itemMinFontScale,
                      itemMaxFontScale: this.itemMaxFontScale,
                      itemFontColor: this.itemFontColor,
                      itemSelectedFontColor: this.itemSelectedFontColor,
                      itemFontSize: this.itemFontSize,
                      itemSelectedFontSize: this.itemSelectedFontSize,
                      itemFontWeight: this.itemFontWeight,
                      itemSelectedFontWeight: this.itemSelectedFontWeight,
                      itemIconSize: this.itemIconSize,
                      itemIconFillColor: this.itemIconFillColor,
                      itemSelectedIconFillColor: this.itemSelectedIconFillColor,
                      itemSymbolFontSize: this.itemSymbolFontSize,
                      itemSymbolFontColor: this.itemSymbolFontColor,
                      itemSelectedSymbolFontColor:
                        this.itemSelectedSymbolFontColor,
                      itemMinHeight: this.itemMinHeight,
                      itemPadding: this.itemPadding,
                      languageDirection: this.languageDirection,
                      hasHybrid: this.items.hasHybrid,
                    });
                  }
                },
                { name: 'SegmentButtonV2ItemContent' }
              );
            }
            Button.pop();
          })
          .render(isInitialRender);
      }, Repeat);
      Flex.pop();
    };
    this.BackplateLayer = () => {
      const parent =
        PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.length
          ? PUV2ViewBase.contextStack[PUV2ViewBase.contextStack.length - 1]
          : null;
      this.observeComponentCreation2((elmtId, isInitialRender) => {
        If.create();
        if (this.selectedItemRect) {
          this.ifElseBranchUpdateFunction(0, () => {
            this.observeComponentCreation2((elmtId, isInitialRender) => {
              Stack.create();
              Stack.position({
                x: this.selectedItemRect.position.x,
                y: this.selectedItemRect.position.y,
              });
              Stack.backgroundColor(this.getItemSelectedBackgroundColor());
              Stack.borderRadius(this.getItemBorderRadius());
              Stack.scale({ x: this.itemScale, y: this.itemScale });
              Stack.shadow(this.getItemBackplateShadow());
              Stack.height(this.selectedItemRect.size.height);
              Stack.width(this.selectedItemRect.size.width);
            }, Stack);
            Stack.pop();
          });
        } else {
          this.ifElseBranchUpdateFunction(1, () => {});
        }
      }, If);
      If.pop();
    };
    this.EffectLayer = () => {
      const parent =
        PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.length
          ? PUV2ViewBase.contextStack[PUV2ViewBase.contextStack.length - 1]
          : null;
      this.observeComponentCreation2((elmtId, isInitialRender) => {
        Repeat(this.itemRects, this)
          .each((repeatItem) => {
            this.observeComponentCreation2((elmtId, isInitialRender) => {
              Stack.create();
              Stack.backgroundColor(this.getEffectBackgroundColor(repeatItem));
              Stack.borderRadius(this.getItemBorderRadius());
              Stack.height(repeatItem.item.size.height);
              Stack.position({
                x: repeatItem.item.position.x,
                y: repeatItem.item.position.y,
              });
              Stack.scale(this.getItemScale(repeatItem.index));
              Stack.width(repeatItem.item.size.width);
            }, Stack);
            Stack.pop();
          })
          .render(isInitialRender);
      }, Repeat);
    };
    this.initParam(
      'items',
      params && 'items' in params ? params.items : undefined
    );
    this.initParam(
      'selectedIndex',
      params && 'selectedIndex' in params ? params.selectedIndex : undefined
    );
    this.$selectedIndex =
      '$selectedIndex' in params ? params.$selectedIndex : () => {};
    this.initParam(
      'theme',
      params && 'theme' in params ? params.theme : undefined
    );
    this.onItemClicked =
      'onItemClicked' in params ? params.onItemClicked : () => {};
    this.initParam(
      'itemMinFontScale',
      params && 'itemMinFontScale' in params
        ? params.itemMinFontScale
        : undefined
    );
    this.initParam(
      'itemMaxFontScale',
      params && 'itemMaxFontScale' in params
        ? params.itemMaxFontScale
        : undefined
    );
    this.initParam(
      'itemSpace',
      params && 'itemSpace' in params ? params.itemSpace : undefined
    );
    this.initParam(
      'itemFontColor',
      params && 'itemFontColor' in params ? params.itemFontColor : undefined
    );
    this.initParam(
      'itemSelectedFontColor',
      params && 'itemSelectedFontColor' in params
        ? params.itemSelectedFontColor
        : undefined
    );
    this.initParam(
      'itemFontSize',
      params && 'itemFontSize' in params ? params.itemFontSize : undefined
    );
    this.initParam(
      'itemSelectedFontSize',
      params && 'itemSelectedFontSize' in params
        ? params.itemSelectedFontSize
        : undefined
    );
    this.initParam(
      'itemFontWeight',
      params && 'itemFontWeight' in params ? params.itemFontWeight : undefined
    );
    this.initParam(
      'itemSelectedFontWeight',
      params && 'itemSelectedFontWeight' in params
        ? params.itemSelectedFontWeight
        : undefined
    );
    this.initParam(
      'itemBorderRadius',
      params && 'itemBorderRadius' in params
        ? params.itemBorderRadius
        : undefined
    );
    this.initParam(
      'itemSelectedBackgroundColor',
      params && 'itemSelectedBackgroundColor' in params
        ? params.itemSelectedBackgroundColor
        : undefined
    );
    this.initParam(
      'itemIconSize',
      params && 'itemIconSize' in params ? params.itemIconSize : undefined
    );
    this.initParam(
      'itemIconFillColor',
      params && 'itemIconFillColor' in params
        ? params.itemIconFillColor
        : undefined
    );
    this.initParam(
      'itemSelectedIconFillColor',
      params && 'itemSelectedIconFillColor' in params
        ? params.itemSelectedIconFillColor
        : undefined
    );
    this.initParam(
      'itemSymbolFontSize',
      params && 'itemSymbolFontSize' in params
        ? params.itemSymbolFontSize
        : undefined
    );
    this.initParam(
      'itemSymbolFontColor',
      params && 'itemSymbolFontColor' in params
        ? params.itemSymbolFontColor
        : undefined
    );
    this.initParam(
      'itemSelectedSymbolFontColor',
      params && 'itemSelectedSymbolFontColor' in params
        ? params.itemSelectedSymbolFontColor
        : undefined
    );
    this.initParam(
      'itemMinHeight',
      params && 'itemMinHeight' in params ? params.itemMinHeight : undefined
    );
    this.initParam(
      'itemPadding',
      params && 'itemPadding' in params ? params.itemPadding : undefined
    );
    this.initParam(
      'itemShadow',
      params && 'itemShadow' in params ? params.itemShadow : undefined
    );
    this.initParam(
      'buttonBackgroundColor',
      params && 'buttonBackgroundColor' in params
        ? params.buttonBackgroundColor
        : undefined
    );
    this.initParam(
      'buttonBackgroundBlurStyle',
      params && 'buttonBackgroundBlurStyle' in params
        ? params.buttonBackgroundBlurStyle
        : undefined
    );
    this.initParam(
      'buttonBackgroundBlurStyleOptions',
      params && 'buttonBackgroundBlurStyleOptions' in params
        ? params.buttonBackgroundBlurStyleOptions
        : undefined
    );
    this.initParam(
      'buttonBackgroundEffect',
      params && 'buttonBackgroundEffect' in params
        ? params.buttonBackgroundEffect
        : undefined
    );
    this.initParam(
      'buttonBorderRadius',
      params && 'buttonBorderRadius' in params
        ? params.buttonBorderRadius
        : undefined
    );
    this.initParam(
      'buttonMinHeight',
      params && 'buttonMinHeight' in params ? params.buttonMinHeight : undefined
    );
    this.initParam(
      'buttonPadding',
      params && 'buttonPadding' in params ? params.buttonPadding : undefined
    );
    this.initParam(
      'languageDirection',
      params && 'languageDirection' in params
        ? params.languageDirection
        : undefined
    );
    this.itemRects = [];
    this.itemScale = 1;
    this.hoveredItemIndex = -1;
    this.mousePressedItemIndex = -1;
    this.touchPressedItemIndex = -1;
    this.isMouseWheelScroll = false;
    this.isDragging = false;
    this.panStartGlobalX = 0;
    this.finalizeConstruction();
  }
  get normalizedSelectedIndex() {
    return normalize(this.selectedIndex, 0, this.items.length - 1);
  }
  get selectedItemRect() {
    return this.itemRects[this.normalizedSelectedIndex];
  }
  initialRender() {
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.push(this);
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      Stack.create();
      Stack.backgroundColor(this.getButtonBackgroundColor());
      Stack.backgroundEffect(this.buttonBackgroundEffect);
      Stack.borderRadius(this.getButtonBorderRadius());
      Stack.constraintSize({
        minWidth: '100%',
        minHeight: this.getButtonMinHeight(),
      });
    }, Stack);
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      Stack.create();
      Stack.borderRadius(this.getButtonBorderRadius());
      Stack.backgroundBlurStyle(
        this.getButtonBackgroundBlurStyle(),
        this.getButtonBackgroundBlurStyleOptions()
      );
    }, Stack);
    this.EffectLayer.bind(this)();
    this.BackplateLayer.bind(this)();
    this.ContentLayer.bind(this)();
    Stack.pop();
    Stack.pop();
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.pop();
  }
  getItemBackplateShadow() {
    return this.itemShadow ?? this.theme.itemShadow;
  }
  getButtonBackgroundBlurStyle() {
    if (this.buttonBackgroundEffect) {
      return undefined;
    }
    return this.buttonBackgroundBlurStyle;
  }
  getButtonBackgroundBlurStyleOptions() {
    if (this.buttonBackgroundEffect) {
      return undefined;
    }
    return this.buttonBackgroundBlurStyleOptions;
  }
  getItemScale(index) {
    const pressed = this.isPressed(index);
    const scale = pressed ? 0.95 : 1;
    return { x: scale, y: scale };
  }
  isPressed(index) {
    return this.mousePressedItemIndex === index;
  }
  updateHoveredItemIndex(index) {
    if (index === this.hoveredItemIndex) {
      return;
    }
    Context.animateTo({ duration: 250, curve: Curve.Friction }, () => {
      this.hoveredItemIndex = index;
    });
  }
  updateMousePressedItemIndex(index) {
    if (index === this.mousePressedItemIndex) {
      return;
    }
    Context.animateTo({ duration: 250, curve: Curve.Friction }, () => {
      this.mousePressedItemIndex = index;
    });
  }
  updateTouchPressedItemIndex(index) {
    if (index === this.touchPressedItemIndex) {
      return;
    }
    Context.animateTo({ duration: 250, curve: Curve.Friction }, () => {
      this.touchPressedItemIndex = index;
    });
  }
  isRTL() {
    if (this.languageDirection || this.languageDirection === Direction.Auto) {
      return i18n.isRTL(i18n.System.getSystemLanguage());
    }
    return this.languageDirection === Direction.Rtl;
  }
  getEffectBackgroundColor(repeatItem) {
    if (repeatItem.index === this.mousePressedItemIndex) {
      return {
        id: -1,
        type: 10001,
        params: ['sys.color.ohos_id_color_click_effect'],
        bundleName: '__harDefaultBundleName__',
        moduleName: '__harDefaultModuleName__',
      };
    }
    if (repeatItem.index === this.hoveredItemIndex) {
      return {
        id: -1,
        type: 10001,
        params: ['sys.color.ohos_id_color_hover'],
        bundleName: '__harDefaultBundleName__',
        moduleName: '__harDefaultModuleName__',
      };
    }
    return Color.Transparent;
  }
  getItemBorderRadius() {
    if (this.itemBorderRadius && g1.getInstance().t1(this.itemBorderRadius)) {
      return g1.getInstance().stringify(this.itemBorderRadius);
    }
    return this.items.hasHybrid ? this.theme.j1 : this.theme.itemBorderRadius;
  }
  getItemSelectedBackgroundColor() {
    if (this.itemSelectedBackgroundColor) {
      return this.itemSelectedBackgroundColor.color;
    }
    return this.theme.itemSelectedBackgroundColor;
  }
  getItemSpace() {
    if (this.itemSpace && g1.getInstance().t1(this.itemSpace)) {
      return this.itemSpace;
    }
    return this.theme.itemSpace;
  }
  getIndexByPosition(globalX, globalY) {
    let index = 0;
    while (index < this.itemRects.length) {
      const rect = this.itemRects[index];
      if (this.isPointOnRect(globalX, globalY, rect)) {
        return index;
      }
      ++index;
    }
    return -1;
  }
  isPointOnRect(globalX, globalY, rect) {
    return (
      globalX >= rect.globalPosition.x &&
      globalX <= rect.globalPosition.x + rect.size.width &&
      globalY >= rect.globalPosition.y &&
      globalY <= rect.globalPosition.y + rect.size.height
    );
  }
  updateSelectedIndex(selectedIndex) {
    if (
      selectedIndex < 0 ||
      selectedIndex > this.items.length - 1 ||
      selectedIndex === this.selectedIndex
    ) {
      return;
    }
    this.getUIContext().animateTo(
      { curve: curves.springMotion(0.347, 0.99) },
      () => {
        this.$selectedIndex?.(selectedIndex);
      }
    );
  }
  updateItemScale(scale) {
    if (this.itemScale === scale) {
      return;
    }
    this.getUIContext().animateTo(
      { curve: curves.interpolatingSpring(10, 1, 410, 38) },
      () => {
        this.itemScale = scale;
      }
    );
  }
  getItemAccessibilityDescription(repeatItem) {
    if (typeof repeatItem.item.accessibilityDescription === 'object') {
      return i(this.getUIContext(), repeatItem.item.accessibilityDescription);
    } else if (typeof repeatItem.item.accessibilityDescription === 'string') {
      return repeatItem.item.accessibilityDescription;
    } else {
      return undefined;
    }
  }
  getItemAccessibilityText(repeatItem) {
    if (typeof repeatItem.item.accessibilityText === 'object') {
      return i(this.getUIContext(), repeatItem.item.accessibilityText);
    } else if (typeof repeatItem.item.accessibilityText === 'string') {
      return repeatItem.item.accessibilityText;
    } else {
      return undefined;
    }
  }
  isSelected(repeatItem) {
    return repeatItem.index === this.normalizedSelectedIndex;
  }
  getButtonPadding() {
    if (this.buttonPadding && g1.getInstance().t1(this.buttonPadding)) {
      return g1.getInstance().stringify(this.buttonPadding);
    }
    return this.theme.buttonPadding;
  }
  getButtonBorderRadius() {
    if (
      this.buttonBorderRadius &&
      g1.getInstance().t1(this.buttonBorderRadius)
    ) {
      return g1.getInstance().stringify(this.buttonBorderRadius);
    }
    return this.theme.buttonBorderRadius;
  }
  getButtonBackgroundColor() {
    if (this.buttonBackgroundColor) {
      return this.buttonBackgroundColor.color;
    }
    return this.theme.buttonBackgroundColor;
  }
  getButtonMinHeight() {
    if (this.buttonMinHeight && g1.getInstance().t1(this.buttonMinHeight)) {
      return g1.getInstance().stringify(this.buttonMinHeight);
    }
    return this.items.hasHybrid ? this.theme.i1 : this.theme.buttonMinHeight;
  }
  updateStateVars(params) {
    if (params === undefined) {
      return;
    }
    if ('items' in params) {
      this.updateParam('items', params.items);
    }
    if ('selectedIndex' in params) {
      this.updateParam('selectedIndex', params.selectedIndex);
    }
    if ('theme' in params) {
      this.updateParam('theme', params.theme);
    }
    if ('itemMinFontScale' in params) {
      this.updateParam('itemMinFontScale', params.itemMinFontScale);
    }
    if ('itemMaxFontScale' in params) {
      this.updateParam('itemMaxFontScale', params.itemMaxFontScale);
    }
    if ('itemSpace' in params) {
      this.updateParam('itemSpace', params.itemSpace);
    }
    if ('itemFontColor' in params) {
      this.updateParam('itemFontColor', params.itemFontColor);
    }
    if ('itemSelectedFontColor' in params) {
      this.updateParam('itemSelectedFontColor', params.itemSelectedFontColor);
    }
    if ('itemFontSize' in params) {
      this.updateParam('itemFontSize', params.itemFontSize);
    }
    if ('itemSelectedFontSize' in params) {
      this.updateParam('itemSelectedFontSize', params.itemSelectedFontSize);
    }
    if ('itemFontWeight' in params) {
      this.updateParam('itemFontWeight', params.itemFontWeight);
    }
    if ('itemSelectedFontWeight' in params) {
      this.updateParam('itemSelectedFontWeight', params.itemSelectedFontWeight);
    }
    if ('itemBorderRadius' in params) {
      this.updateParam('itemBorderRadius', params.itemBorderRadius);
    }
    if ('itemSelectedBackgroundColor' in params) {
      this.updateParam(
        'itemSelectedBackgroundColor',
        params.itemSelectedBackgroundColor
      );
    }
    if ('itemIconSize' in params) {
      this.updateParam('itemIconSize', params.itemIconSize);
    }
    if ('itemIconFillColor' in params) {
      this.updateParam('itemIconFillColor', params.itemIconFillColor);
    }
    if ('itemSelectedIconFillColor' in params) {
      this.updateParam(
        'itemSelectedIconFillColor',
        params.itemSelectedIconFillColor
      );
    }
    if ('itemSymbolFontSize' in params) {
      this.updateParam('itemSymbolFontSize', params.itemSymbolFontSize);
    }
    if ('itemSymbolFontColor' in params) {
      this.updateParam('itemSymbolFontColor', params.itemSymbolFontColor);
    }
    if ('itemSelectedSymbolFontColor' in params) {
      this.updateParam(
        'itemSelectedSymbolFontColor',
        params.itemSelectedSymbolFontColor
      );
    }
    if ('itemMinHeight' in params) {
      this.updateParam('itemMinHeight', params.itemMinHeight);
    }
    if ('itemPadding' in params) {
      this.updateParam('itemPadding', params.itemPadding);
    }
    if ('itemShadow' in params) {
      this.updateParam('itemShadow', params.itemShadow);
    }
    if ('buttonBackgroundColor' in params) {
      this.updateParam('buttonBackgroundColor', params.buttonBackgroundColor);
    }
    if ('buttonBackgroundBlurStyle' in params) {
      this.updateParam(
        'buttonBackgroundBlurStyle',
        params.buttonBackgroundBlurStyle
      );
    }
    if ('buttonBackgroundBlurStyleOptions' in params) {
      this.updateParam(
        'buttonBackgroundBlurStyleOptions',
        params.buttonBackgroundBlurStyleOptions
      );
    }
    if ('buttonBackgroundEffect' in params) {
      this.updateParam('buttonBackgroundEffect', params.buttonBackgroundEffect);
    }
    if ('buttonBorderRadius' in params) {
      this.updateParam('buttonBorderRadius', params.buttonBorderRadius);
    }
    if ('buttonMinHeight' in params) {
      this.updateParam('buttonMinHeight', params.buttonMinHeight);
    }
    if ('buttonPadding' in params) {
      this.updateParam('buttonPadding', params.buttonPadding);
    }
    if ('languageDirection' in params) {
      this.updateParam('languageDirection', params.languageDirection);
    }
  }
  rerender() {
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.push(this);
    this.updateDirtyElements();
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.pop();
  }
}
m([Param], d1.prototype, 'items', void 0);
m([Param], d1.prototype, 'selectedIndex', void 0);
m([Event], d1.prototype, '$selectedIndex', void 0);
m([Param], d1.prototype, 'theme', void 0);
m([Event], d1.prototype, 'onItemClicked', void 0);
m([Param], d1.prototype, 'itemMinFontScale', void 0);
m([Param], d1.prototype, 'itemMaxFontScale', void 0);
m([Param], d1.prototype, 'itemSpace', void 0);
m([Param], d1.prototype, 'itemFontColor', void 0);
m([Param], d1.prototype, 'itemSelectedFontColor', void 0);
m([Param], d1.prototype, 'itemFontSize', void 0);
m([Param], d1.prototype, 'itemSelectedFontSize', void 0);
m([Param], d1.prototype, 'itemFontWeight', void 0);
m([Param], d1.prototype, 'itemSelectedFontWeight', void 0);
m([Param], d1.prototype, 'itemBorderRadius', void 0);
m([Param], d1.prototype, 'itemSelectedBackgroundColor', void 0);
m([Param], d1.prototype, 'itemIconSize', void 0);
m([Param], d1.prototype, 'itemIconFillColor', void 0);
m([Param], d1.prototype, 'itemSelectedIconFillColor', void 0);
m([Param], d1.prototype, 'itemSymbolFontSize', void 0);
m([Param], d1.prototype, 'itemSymbolFontColor', void 0);
m([Param], d1.prototype, 'itemSelectedSymbolFontColor', void 0);
m([Param], d1.prototype, 'itemMinHeight', void 0);
m([Param], d1.prototype, 'itemPadding', void 0);
m([Param], d1.prototype, 'itemShadow', void 0);
m([Param], d1.prototype, 'buttonBackgroundColor', void 0);
m([Param], d1.prototype, 'buttonBackgroundBlurStyle', void 0);
m([Param], d1.prototype, 'buttonBackgroundBlurStyleOptions', void 0);
m([Param], d1.prototype, 'buttonBackgroundEffect', void 0);
m([Param], d1.prototype, 'buttonBorderRadius', void 0);
m([Param], d1.prototype, 'buttonMinHeight', void 0);
m([Param], d1.prototype, 'buttonPadding', void 0);
m([Param], d1.prototype, 'languageDirection', void 0);
m([Local], d1.prototype, 'itemRects', void 0);
m([Local], d1.prototype, 'itemScale', void 0);
m([Local], d1.prototype, 'hoveredItemIndex', void 0);
m([Local], d1.prototype, 'mousePressedItemIndex', void 0);
m([Local], d1.prototype, 'touchPressedItemIndex', void 0);
m([Computed], d1.prototype, 'normalizedSelectedIndex', null);
m([Computed], d1.prototype, 'selectedItemRect', null);
const e1 = {
  itemBorderRadius: {
    id: -1,
    type: 10002,
    params: ['sys.float.corner_radius_level10'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemBackgroundColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.comp_background_tertiary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSelectedBackgroundColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.brand'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSpace: LengthMetrics.vp(1),
  itemFontColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_secondary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSelectedFontColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_on_primary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemFontWeight: FontWeight.Medium,
  itemSelectedFontWeight: FontWeight.Medium,
  itemIconFillColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.icon_secondary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSelectedIconFillColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_on_primary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSymbolFontColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_secondary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemSelectedSymbolFontColor: {
    id: -1,
    type: 10001,
    params: ['sys.color.font_on_primary'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemFontSize: {
    id: -1,
    type: 10002,
    params: ['sys.float.ohos_id_text_size_button2'],
    bundleName: '__harDefaultBundleName__',
    moduleName: '__harDefaultModuleName__',
  },
  itemIconSize: 24,
  itemSymbolFontSize: 20,
  itemPadding: {
    top: LengthMetrics.resource({
      id: -1,
      type: 10002,
      params: ['sys.float.padding_level2'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    }),
    bottom: LengthMetrics.resource({
      id: -1,
      type: 10002,
      params: ['sys.float.padding_level2'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    }),
    start: LengthMetrics.resource({
      id: -1,
      type: 10002,
      params: ['sys.float.padding_level8'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    }),
    end: LengthMetrics.resource({
      id: -1,
      type: 10002,
      params: ['sys.float.padding_level8'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    }),
  },
  itemMinHeight: 40,
  l1: 64,
  itemMaxFontScale: o,
  m1: o,
  n1: t,
  itemMinFontScale: u,
  o1: u,
  q1: a1,
};
export class MultiCapsuleSegmentButtonV2 extends ViewV2 {
  constructor(
    parent,
    params,
    __localStorage,
    elmtId = -1,
    paramsLambda,
    extraInfo
  ) {
    super(parent, elmtId, extraInfo);
    this.initParam(
      'items',
      params && 'items' in params ? params.items : undefined
    );
    this.initParam(
      'selectedIndexes',
      params && 'selectedIndexes' in params ? params.selectedIndexes : undefined
    );
    this.$selectedIndexes =
      '$selectedIndexes' in params ? params.$selectedIndexes : () => {};
    this.onItemClicked =
      'onItemClicked' in params ? params.onItemClicked : () => {};
    this.initParam(
      'itemMinFontScale',
      params && 'itemMinFontScale' in params
        ? params.itemMinFontScale
        : undefined
    );
    this.initParam(
      'itemMaxFontScale',
      params && 'itemMaxFontScale' in params
        ? params.itemMaxFontScale
        : undefined
    );
    this.initParam(
      'itemSpace',
      params && 'itemSpace' in params ? params.itemSpace : undefined
    );
    this.initParam(
      'itemFontColor',
      params && 'itemFontColor' in params ? params.itemFontColor : undefined
    );
    this.initParam(
      'itemSelectedFontColor',
      params && 'itemSelectedFontColor' in params
        ? params.itemSelectedFontColor
        : undefined
    );
    this.initParam(
      'itemFontSize',
      params && 'itemFontSize' in params ? params.itemFontSize : undefined
    );
    this.initParam(
      'itemSelectedFontSize',
      params && 'itemSelectedFontSize' in params
        ? params.itemSelectedFontSize
        : undefined
    );
    this.initParam(
      'itemFontWeight',
      params && 'itemFontWeight' in params ? params.itemFontWeight : undefined
    );
    this.initParam(
      'itemSelectedFontWeight',
      params && 'itemSelectedFontWeight' in params
        ? params.itemSelectedFontWeight
        : undefined
    );
    this.initParam(
      'itemBorderRadius',
      params && 'itemBorderRadius' in params
        ? params.itemBorderRadius
        : undefined
    );
    this.initParam(
      'itemBackgroundColor',
      params && 'itemBackgroundColor' in params
        ? params.itemBackgroundColor
        : undefined
    );
    this.initParam(
      'itemSelectedBackgroundColor',
      params && 'itemSelectedBackgroundColor' in params
        ? params.itemSelectedBackgroundColor
        : undefined
    );
    this.initParam(
      'itemIconSize',
      params && 'itemIconSize' in params ? params.itemIconSize : undefined
    );
    this.initParam(
      'itemIconFillColor',
      params && 'itemIconFillColor' in params
        ? params.itemIconFillColor
        : undefined
    );
    this.initParam(
      'itemSelectedIconFillColor',
      params && 'itemSelectedIconFillColor' in params
        ? params.itemSelectedIconFillColor
        : undefined
    );
    this.initParam(
      'itemSymbolFontSize',
      params && 'itemSymbolFontSize' in params
        ? params.itemSymbolFontSize
        : undefined
    );
    this.initParam(
      'itemSymbolFontColor',
      params && 'itemSymbolFontColor' in params
        ? params.itemSymbolFontColor
        : undefined
    );
    this.initParam(
      'itemSelectedSymbolFontColor',
      params && 'itemSelectedSymbolFontColor' in params
        ? params.itemSelectedSymbolFontColor
        : undefined
    );
    this.initParam(
      'itemMinHeight',
      params && 'itemMinHeight' in params ? params.itemMinHeight : undefined
    );
    this.initParam(
      'itemPadding',
      params && 'itemPadding' in params ? params.itemPadding : undefined
    );
    this.initParam(
      'languageDirection',
      params && 'languageDirection' in params
        ? params.languageDirection
        : undefined
    );
    this.theme = e1;
    this.finalizeConstruction();
  }
  initialRender() {
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.push(this);
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      Flex.create({
        alignItems: ItemAlign.Stretch,
        space: { main: this.getItemSpace() },
      });
      Flex.direction(this.languageDirection);
    }, Flex);
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      Repeat(this.items, this)
        .each((repeatItem) => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Button.createWithChild({ type: ButtonType.Normal });
            Button.accessibilityGroup(true);
            Button.accessibilityChecked(this.isSelected(repeatItem));
            Button.accessibilityText(this.getItemAccessibilityText(repeatItem));
            Button.accessibilityDescription(
              this.getItemAccessibilityDescription(repeatItem)
            );
            Button.accessibilityLevel(repeatItem.item.accessibilityLevel);
            Button.backgroundColor(this.getItemBackgroundColor(repeatItem));
            Button.borderRadius(this.getItemButtonBorderRadius(repeatItem));
            Button.constraintSize({ minHeight: this.getItemMinHeight() });
            Button.enabled(repeatItem.item.enabled);
            Button.layoutWeight(1);
            Button.padding(0);
            Button.onClick(() => {
              this.onItemClicked?.(repeatItem.index);
              let selection;
              if (this.isSelected(repeatItem)) {
                selection = this.selectedIndexes.filter((index) => {
                  if (index < 0 || index > this.items.length - 1) {
                    return false;
                  }
                  return index !== repeatItem.index;
                });
              } else {
                selection = this.selectedIndexes
                  .filter(
                    (index) => index >= 0 && index <= this.items.length - 1
                  )
                  .concat(repeatItem.index);
              }
              this.$selectedIndexes(selection);
            });
          }, Button);
          {
            this.observeComponentCreation2(
              (elmtId, isInitialRender) => {
                if (isInitialRender) {
                  let componentCall = new f1(
                    this,
                    {
                      theme: this.theme,
                      item: repeatItem.item,
                      selected: this.isSelected(repeatItem),
                      hasHybrid: this.items.hasHybrid,
                      itemMinFontScale: this.itemMinFontScale,
                      itemMaxFontScale: this.itemMaxFontScale,
                      itemFontColor: this.itemFontColor,
                      itemSelectedFontColor: this.itemSelectedFontColor,
                      itemFontSize: this.itemFontSize,
                      itemSelectedFontSize: this.itemSelectedFontSize,
                      itemFontWeight: this.itemFontWeight,
                      itemSelectedFontWeight: this.itemSelectedFontWeight,
                      itemIconSize: this.itemIconSize,
                      itemIconFillColor: this.itemIconFillColor,
                      itemSelectedIconFillColor: this.itemSelectedIconFillColor,
                      itemSymbolFontSize: this.itemSymbolFontSize,
                      itemSymbolFontColor: this.itemSymbolFontColor,
                      itemSelectedSymbolFontColor:
                        this.itemSelectedSymbolFontColor,
                      itemMinHeight: this.itemMinHeight,
                      itemPadding: this.itemPadding,
                      languageDirection: this.languageDirection,
                    },
                    undefined,
                    elmtId,
                    () => {},
                    {
                      page: 'advancedComponents/src/main/ets/components/SegmentButtonV2.ets',
                      line: 1060,
                      s1: 13,
                    }
                  );
                  ViewV2.create(componentCall);
                  let paramsLambda = () => {
                    return {
                      theme: this.theme,
                      item: repeatItem.item,
                      selected: this.isSelected(repeatItem),
                      hasHybrid: this.items.hasHybrid,
                      itemMinFontScale: this.itemMinFontScale,
                      itemMaxFontScale: this.itemMaxFontScale,
                      itemFontColor: this.itemFontColor,
                      itemSelectedFontColor: this.itemSelectedFontColor,
                      itemFontSize: this.itemFontSize,
                      itemSelectedFontSize: this.itemSelectedFontSize,
                      itemFontWeight: this.itemFontWeight,
                      itemSelectedFontWeight: this.itemSelectedFontWeight,
                      itemIconSize: this.itemIconSize,
                      itemIconFillColor: this.itemIconFillColor,
                      itemSelectedIconFillColor: this.itemSelectedIconFillColor,
                      itemSymbolFontSize: this.itemSymbolFontSize,
                      itemSymbolFontColor: this.itemSymbolFontColor,
                      itemSelectedSymbolFontColor:
                        this.itemSelectedSymbolFontColor,
                      itemMinHeight: this.itemMinHeight,
                      itemPadding: this.itemPadding,
                      languageDirection: this.languageDirection,
                    };
                  };
                  componentCall.paramsGenerator_ = paramsLambda;
                } else {
                  this.updateStateVarsOfChildByElmtId(elmtId, {
                    theme: this.theme,
                    item: repeatItem.item,
                    selected: this.isSelected(repeatItem),
                    hasHybrid: this.items.hasHybrid,
                    itemMinFontScale: this.itemMinFontScale,
                    itemMaxFontScale: this.itemMaxFontScale,
                    itemFontColor: this.itemFontColor,
                    itemSelectedFontColor: this.itemSelectedFontColor,
                    itemFontSize: this.itemFontSize,
                    itemSelectedFontSize: this.itemSelectedFontSize,
                    itemFontWeight: this.itemFontWeight,
                    itemSelectedFontWeight: this.itemSelectedFontWeight,
                    itemIconSize: this.itemIconSize,
                    itemIconFillColor: this.itemIconFillColor,
                    itemSelectedIconFillColor: this.itemSelectedIconFillColor,
                    itemSymbolFontSize: this.itemSymbolFontSize,
                    itemSymbolFontColor: this.itemSymbolFontColor,
                    itemSelectedSymbolFontColor:
                      this.itemSelectedSymbolFontColor,
                    itemMinHeight: this.itemMinHeight,
                    itemPadding: this.itemPadding,
                    languageDirection: this.languageDirection,
                  });
                }
              },
              { name: 'SegmentButtonV2ItemContent' }
            );
          }
          Button.pop();
        })
        .render(isInitialRender);
    }, Repeat);
    Flex.pop();
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.pop();
  }
  getItemAccessibilityDescription(repeatItem) {
    if (typeof repeatItem.item.accessibilityDescription === 'object') {
      return i(this.getUIContext(), repeatItem.item.accessibilityDescription);
    } else if (typeof repeatItem.item.accessibilityDescription === 'string') {
      return repeatItem.item.accessibilityDescription;
    } else {
      return undefined;
    }
  }
  getItemAccessibilityText(repeatItem) {
    if (typeof repeatItem.item.accessibilityText === 'object') {
      return i(this.getUIContext(), repeatItem.item.accessibilityText);
    } else if (typeof repeatItem.item.accessibilityText === 'string') {
      return repeatItem.item.accessibilityText;
    } else {
      return undefined;
    }
  }
  getItemSpace() {
    if (this.itemSpace && g1.getInstance().t1(this.itemSpace)) {
      return this.itemSpace;
    }
    return this.theme.itemSpace;
  }
  getItemMinHeight() {
    if (this.itemMinHeight && g1.getInstance().t1(this.itemMinHeight)) {
      return g1.getInstance().stringify(this.itemMinHeight);
    }
    return this.theme.itemMinHeight;
  }
  getItemBackgroundColor(repeatItem) {
    if (this.isSelected(repeatItem)) {
      return (
        this.itemSelectedBackgroundColor?.color ??
        this.theme.itemSelectedBackgroundColor
      );
    }
    return this.itemBackgroundColor?.color ?? this.theme.itemBackgroundColor;
  }
  isSelected(repeatItem) {
    return this.selectedIndexes.includes(repeatItem.index);
  }
  getItemButtonBorderRadius(repeatItem) {
    const h1 = LengthMetrics.vp(0);
    if (repeatItem.index === 0) {
      const borderRadius =
        this.itemBorderRadius ??
        LengthMetrics.resource(this.theme.itemBorderRadius);
      return {
        topStart: borderRadius,
        bottomStart: borderRadius,
        topEnd: h1,
        bottomEnd: h1,
      };
    }
    if (repeatItem.index === Math.min(this.items.length - 1, 4)) {
      const borderRadius =
        this.itemBorderRadius ??
        LengthMetrics.resource(this.theme.itemBorderRadius);
      return {
        topStart: h1,
        bottomStart: h1,
        topEnd: borderRadius,
        bottomEnd: borderRadius,
      };
    }
    return {
      topStart: h1,
      bottomStart: h1,
      topEnd: h1,
      bottomEnd: h1,
    };
  }
  updateStateVars(params) {
    if (params === undefined) {
      return;
    }
    if ('items' in params) {
      this.updateParam('items', params.items);
    }
    if ('selectedIndexes' in params) {
      this.updateParam('selectedIndexes', params.selectedIndexes);
    }
    if ('itemMinFontScale' in params) {
      this.updateParam('itemMinFontScale', params.itemMinFontScale);
    }
    if ('itemMaxFontScale' in params) {
      this.updateParam('itemMaxFontScale', params.itemMaxFontScale);
    }
    if ('itemSpace' in params) {
      this.updateParam('itemSpace', params.itemSpace);
    }
    if ('itemFontColor' in params) {
      this.updateParam('itemFontColor', params.itemFontColor);
    }
    if ('itemSelectedFontColor' in params) {
      this.updateParam('itemSelectedFontColor', params.itemSelectedFontColor);
    }
    if ('itemFontSize' in params) {
      this.updateParam('itemFontSize', params.itemFontSize);
    }
    if ('itemSelectedFontSize' in params) {
      this.updateParam('itemSelectedFontSize', params.itemSelectedFontSize);
    }
    if ('itemFontWeight' in params) {
      this.updateParam('itemFontWeight', params.itemFontWeight);
    }
    if ('itemSelectedFontWeight' in params) {
      this.updateParam('itemSelectedFontWeight', params.itemSelectedFontWeight);
    }
    if ('itemBorderRadius' in params) {
      this.updateParam('itemBorderRadius', params.itemBorderRadius);
    }
    if ('itemBackgroundColor' in params) {
      this.updateParam('itemBackgroundColor', params.itemBackgroundColor);
    }
    if ('itemSelectedBackgroundColor' in params) {
      this.updateParam(
        'itemSelectedBackgroundColor',
        params.itemSelectedBackgroundColor
      );
    }
    if ('itemIconSize' in params) {
      this.updateParam('itemIconSize', params.itemIconSize);
    }
    if ('itemIconFillColor' in params) {
      this.updateParam('itemIconFillColor', params.itemIconFillColor);
    }
    if ('itemSelectedIconFillColor' in params) {
      this.updateParam(
        'itemSelectedIconFillColor',
        params.itemSelectedIconFillColor
      );
    }
    if ('itemSymbolFontSize' in params) {
      this.updateParam('itemSymbolFontSize', params.itemSymbolFontSize);
    }
    if ('itemSymbolFontColor' in params) {
      this.updateParam('itemSymbolFontColor', params.itemSymbolFontColor);
    }
    if ('itemSelectedSymbolFontColor' in params) {
      this.updateParam(
        'itemSelectedSymbolFontColor',
        params.itemSelectedSymbolFontColor
      );
    }
    if ('itemMinHeight' in params) {
      this.updateParam('itemMinHeight', params.itemMinHeight);
    }
    if ('itemPadding' in params) {
      this.updateParam('itemPadding', params.itemPadding);
    }
    if ('languageDirection' in params) {
      this.updateParam('languageDirection', params.languageDirection);
    }
  }
  rerender() {
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.push(this);
    this.updateDirtyElements();
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.pop();
  }
}
m([Param], MultiCapsuleSegmentButtonV2.prototype, 'items', void 0);
m([Param], MultiCapsuleSegmentButtonV2.prototype, 'selectedIndexes', void 0);
m([Event], MultiCapsuleSegmentButtonV2.prototype, '$selectedIndexes', void 0);
m([Event], MultiCapsuleSegmentButtonV2.prototype, 'onItemClicked', void 0);
m([Param], MultiCapsuleSegmentButtonV2.prototype, 'itemMinFontScale', void 0);
m([Param], MultiCapsuleSegmentButtonV2.prototype, 'itemMaxFontScale', void 0);
m([Param], MultiCapsuleSegmentButtonV2.prototype, 'itemSpace', void 0);
m([Param], MultiCapsuleSegmentButtonV2.prototype, 'itemFontColor', void 0);
m(
  [Param],
  MultiCapsuleSegmentButtonV2.prototype,
  'itemSelectedFontColor',
  void 0
);
m([Param], MultiCapsuleSegmentButtonV2.prototype, 'itemFontSize', void 0);
m(
  [Param],
  MultiCapsuleSegmentButtonV2.prototype,
  'itemSelectedFontSize',
  void 0
);
m([Param], MultiCapsuleSegmentButtonV2.prototype, 'itemFontWeight', void 0);
m(
  [Param],
  MultiCapsuleSegmentButtonV2.prototype,
  'itemSelectedFontWeight',
  void 0
);
m([Param], MultiCapsuleSegmentButtonV2.prototype, 'itemBorderRadius', void 0);
m(
  [Param],
  MultiCapsuleSegmentButtonV2.prototype,
  'itemBackgroundColor',
  void 0
);
m(
  [Param],
  MultiCapsuleSegmentButtonV2.prototype,
  'itemSelectedBackgroundColor',
  void 0
);
m([Param], MultiCapsuleSegmentButtonV2.prototype, 'itemIconSize', void 0);
m([Param], MultiCapsuleSegmentButtonV2.prototype, 'itemIconFillColor', void 0);
m(
  [Param],
  MultiCapsuleSegmentButtonV2.prototype,
  'itemSelectedIconFillColor',
  void 0
);
m([Param], MultiCapsuleSegmentButtonV2.prototype, 'itemSymbolFontSize', void 0);
m(
  [Param],
  MultiCapsuleSegmentButtonV2.prototype,
  'itemSymbolFontColor',
  void 0
);
m(
  [Param],
  MultiCapsuleSegmentButtonV2.prototype,
  'itemSelectedSymbolFontColor',
  void 0
);
m([Param], MultiCapsuleSegmentButtonV2.prototype, 'itemMinHeight', void 0);
m([Param], MultiCapsuleSegmentButtonV2.prototype, 'itemPadding', void 0);
m([Param], MultiCapsuleSegmentButtonV2.prototype, 'languageDirection', void 0);
class f1 extends ViewV2 {
  constructor(
    parent,
    params,
    __localStorage,
    elmtId = -1,
    paramsLambda,
    extraInfo
  ) {
    super(parent, elmtId, extraInfo);
    this.initParam(
      'hasHybrid',
      params && 'hasHybrid' in params ? params.hasHybrid : undefined
    );
    this.initParam(
      'item',
      params && 'item' in params ? params.item : undefined
    );
    this.initParam(
      'selected',
      params && 'selected' in params ? params.selected : undefined
    );
    this.initParam(
      'theme',
      params && 'theme' in params ? params.theme : undefined
    );
    this.initParam(
      'itemMinFontScale',
      params && 'itemMinFontScale' in params
        ? params.itemMinFontScale
        : undefined
    );
    this.initParam(
      'itemMaxFontScale',
      params && 'itemMaxFontScale' in params
        ? params.itemMaxFontScale
        : undefined
    );
    this.initParam(
      'itemFontColor',
      params && 'itemFontColor' in params ? params.itemFontColor : undefined
    );
    this.initParam(
      'itemSelectedFontColor',
      params && 'itemSelectedFontColor' in params
        ? params.itemSelectedFontColor
        : undefined
    );
    this.initParam(
      'itemFontSize',
      params && 'itemFontSize' in params ? params.itemFontSize : undefined
    );
    this.initParam(
      'itemSelectedFontSize',
      params && 'itemSelectedFontSize' in params
        ? params.itemSelectedFontSize
        : undefined
    );
    this.initParam(
      'itemFontWeight',
      params && 'itemFontWeight' in params ? params.itemFontWeight : undefined
    );
    this.initParam(
      'itemSelectedFontWeight',
      params && 'itemSelectedFontWeight' in params
        ? params.itemSelectedFontWeight
        : undefined
    );
    this.initParam(
      'itemIconSize',
      params && 'itemIconSize' in params ? params.itemIconSize : undefined
    );
    this.initParam(
      'itemIconFillColor',
      params && 'itemIconFillColor' in params
        ? params.itemIconFillColor
        : undefined
    );
    this.initParam(
      'itemSelectedIconFillColor',
      params && 'itemSelectedIconFillColor' in params
        ? params.itemSelectedIconFillColor
        : undefined
    );
    this.initParam(
      'itemSymbolFontSize',
      params && 'itemSymbolFontSize' in params
        ? params.itemSymbolFontSize
        : undefined
    );
    this.initParam(
      'itemSymbolFontColor',
      params && 'itemSymbolFontColor' in params
        ? params.itemSymbolFontColor
        : undefined
    );
    this.initParam(
      'itemSelectedSymbolFontColor',
      params && 'itemSelectedSymbolFontColor' in params
        ? params.itemSelectedSymbolFontColor
        : undefined
    );
    this.initParam(
      'itemMinHeight',
      params && 'itemMinHeight' in params ? params.itemMinHeight : undefined
    );
    this.initParam(
      'itemPadding',
      params && 'itemPadding' in params ? params.itemPadding : undefined
    );
    this.initParam(
      'languageDirection',
      params && 'languageDirection' in params
        ? params.languageDirection
        : undefined
    );
    this.finalizeConstruction();
  }
  initialRender() {
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.push(this);
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      Column.create({ space: 2 });
      Column.constraintSize({ minHeight: this.getItemMinHeight() });
      Column.justifyContent(FlexAlign.Center);
      Column.padding(this.getItemPadding());
    }, Column);
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      If.create();
      if (this.item.symbol) {
        this.ifElseBranchUpdateFunction(0, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            SymbolGlyph.create(this.item.symbol);
            SymbolGlyph.fontSize(this.getSymbolFontSize());
            SymbolGlyph.fontColor([this.getItemSymbolFillColor()]);
            SymbolGlyph.attributeModifier.bind(this)(this.item.symbolModifier);
          }, SymbolGlyph);
        });
      } else if (this.item.icon) {
        this.ifElseBranchUpdateFunction(1, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Image.create(this.item.icon);
            Image.fillColor(this.getItemIconFillColor());
            Image.width(this.getItemIconWidth());
            Image.height(this.getItemIconHeight());
            Image.draggable(false);
            Image.attributeModifier.bind(this)(this.item.iconModifier);
          }, Image);
        });
      } else {
        this.ifElseBranchUpdateFunction(2, () => {});
      }
    }, If);
    If.pop();
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      If.create();
      if (this.item.text) {
        this.ifElseBranchUpdateFunction(0, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Text.create(this.item.text);
            Text.fontSize(this.getItemFontSize());
            Text.fontColor(this.getItemFontColor());
            Text.fontWeight(this.getItemFontWeight());
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.minFontScale(this.getItemMinFontScale());
            Text.maxFontScale(this.getItemMaxFontScale());
            Text.attributeModifier.bind(this)(this.item.textModifier);
          }, Text);
          Text.pop();
        });
      } else {
        this.ifElseBranchUpdateFunction(1, () => {});
      }
    }, If);
    If.pop();
    Column.pop();
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.pop();
  }
  getItemFontWeight() {
    if (this.selected) {
      return this.itemSelectedFontWeight ?? this.theme.itemSelectedFontWeight;
    }
    return this.itemFontWeight ?? this.theme.itemFontWeight;
  }
  getItemSymbolFillColor() {
    if (this.selected) {
      return (
        this.itemSelectedSymbolFontColor?.color ??
        this.theme.itemSelectedSymbolFontColor
      );
    }
    return this.itemSymbolFontColor?.color ?? this.theme.itemSymbolFontColor;
  }
  getSymbolFontSize() {
    if (
      this.itemSymbolFontSize &&
      g1.getInstance().t1(this.itemSymbolFontSize)
    ) {
      return g1.getInstance().stringify(this.itemSymbolFontSize);
    }
    return this.theme.itemSymbolFontSize;
  }
  getItemMaxFontScale() {
    if (typeof this.itemMaxFontScale === 'number') {
      return normalize(this.itemMaxFontScale, this.theme.m1, this.theme.n1);
    }
    if (typeof this.itemMaxFontScale === 'object') {
      const itemMaxFontScale =
        j(this.getUIContext(), this.itemMaxFontScale) ?? o;
      return normalize(itemMaxFontScale, this.theme.m1, this.theme.n1);
    }
    return o;
  }
  getItemMinFontScale() {
    if (typeof this.itemMinFontScale === 'number') {
      return normalize(this.itemMinFontScale, this.theme.o1, this.theme.q1);
    }
    if (typeof this.itemMinFontScale === 'object') {
      const itemMinFontScale =
        j(this.getUIContext(), this.itemMinFontScale) ?? u;
      return normalize(itemMinFontScale, this.theme.o1, this.theme.q1);
    }
    return u;
  }
  getItemPadding() {
    const itemPadding = {
      top: this.theme.itemPadding.top,
      bottom: this.theme.itemPadding.bottom,
      start: this.theme.itemPadding.start,
      end: this.theme.itemPadding.end,
    };
    if (this.itemPadding?.top && g1.getInstance().t1(this.itemPadding.top)) {
      itemPadding.top = this.itemPadding.top;
    }
    if (
      this.itemPadding?.bottom &&
      g1.getInstance().t1(this.itemPadding.bottom)
    ) {
      itemPadding.bottom = this.itemPadding.bottom;
    }
    if (
      this.itemPadding?.start &&
      g1.getInstance().t1(this.itemPadding.start)
    ) {
      itemPadding.start = this.itemPadding.start;
    }
    if (this.itemPadding?.end && g1.getInstance().t1(this.itemPadding.end)) {
      itemPadding.end = this.itemPadding.end;
    }
    return itemPadding;
  }
  getItemMinHeight() {
    if (this.itemMinHeight && g1.getInstance().t1(this.itemMinHeight)) {
      return g1.getInstance().stringify(this.itemMinHeight);
    }
    return this.hasHybrid ? this.theme.l1 : this.theme.itemMinHeight;
  }
  getItemFontColor() {
    if (this.selected) {
      if (this.itemSelectedFontColor) {
        return this.itemSelectedFontColor.color;
      }
      return this.theme.itemSelectedFontColor;
    }
    if (this.itemFontColor) {
      return this.itemFontColor.color;
    }
    return this.theme.itemFontColor;
  }
  getItemFontSize() {
    if (this.selected) {
      if (
        this.itemSelectedFontSize &&
        g1.getInstance().t1(this.itemSelectedFontSize)
      ) {
        return g1.getInstance().stringify(this.itemSelectedFontSize);
      }
      return this.theme.itemFontSize;
    }
    if (this.itemFontSize && g1.getInstance().t1(this.itemFontSize)) {
      return g1.getInstance().stringify(this.itemFontSize);
    }
    return this.theme.itemFontSize;
  }
  getItemIconHeight() {
    if (
      this.itemIconSize?.height &&
      g1.getInstance().t1(this.itemIconSize.height)
    ) {
      return g1.getInstance().stringify(this.itemIconSize.height);
    }
    return this.theme.itemIconSize;
  }
  getItemIconWidth() {
    if (
      this.itemIconSize?.width &&
      g1.getInstance().t1(this.itemIconSize.width)
    ) {
      return g1.getInstance().stringify(this.itemIconSize.width);
    }
    return this.theme.itemIconSize;
  }
  getItemIconFillColor() {
    if (this.selected) {
      if (this.itemSelectedIconFillColor) {
        return this.itemSelectedIconFillColor.color;
      }
      return this.theme.itemSelectedIconFillColor;
    }
    if (this.itemIconFillColor) {
      return this.itemIconFillColor.color;
    }
    return this.theme.itemIconFillColor;
  }
  updateStateVars(params) {
    if (params === undefined) {
      return;
    }
    if ('hasHybrid' in params) {
      this.updateParam('hasHybrid', params.hasHybrid);
    }
    if ('item' in params) {
      this.updateParam('item', params.item);
    }
    if ('selected' in params) {
      this.updateParam('selected', params.selected);
    }
    if ('theme' in params) {
      this.updateParam('theme', params.theme);
    }
    if ('itemMinFontScale' in params) {
      this.updateParam('itemMinFontScale', params.itemMinFontScale);
    }
    if ('itemMaxFontScale' in params) {
      this.updateParam('itemMaxFontScale', params.itemMaxFontScale);
    }
    if ('itemFontColor' in params) {
      this.updateParam('itemFontColor', params.itemFontColor);
    }
    if ('itemSelectedFontColor' in params) {
      this.updateParam('itemSelectedFontColor', params.itemSelectedFontColor);
    }
    if ('itemFontSize' in params) {
      this.updateParam('itemFontSize', params.itemFontSize);
    }
    if ('itemSelectedFontSize' in params) {
      this.updateParam('itemSelectedFontSize', params.itemSelectedFontSize);
    }
    if ('itemFontWeight' in params) {
      this.updateParam('itemFontWeight', params.itemFontWeight);
    }
    if ('itemSelectedFontWeight' in params) {
      this.updateParam('itemSelectedFontWeight', params.itemSelectedFontWeight);
    }
    if ('itemIconSize' in params) {
      this.updateParam('itemIconSize', params.itemIconSize);
    }
    if ('itemIconFillColor' in params) {
      this.updateParam('itemIconFillColor', params.itemIconFillColor);
    }
    if ('itemSelectedIconFillColor' in params) {
      this.updateParam(
        'itemSelectedIconFillColor',
        params.itemSelectedIconFillColor
      );
    }
    if ('itemSymbolFontSize' in params) {
      this.updateParam('itemSymbolFontSize', params.itemSymbolFontSize);
    }
    if ('itemSymbolFontColor' in params) {
      this.updateParam('itemSymbolFontColor', params.itemSymbolFontColor);
    }
    if ('itemSelectedSymbolFontColor' in params) {
      this.updateParam(
        'itemSelectedSymbolFontColor',
        params.itemSelectedSymbolFontColor
      );
    }
    if ('itemMinHeight' in params) {
      this.updateParam('itemMinHeight', params.itemMinHeight);
    }
    if ('itemPadding' in params) {
      this.updateParam('itemPadding', params.itemPadding);
    }
    if ('languageDirection' in params) {
      this.updateParam('languageDirection', params.languageDirection);
    }
  }
  rerender() {
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.push(this);
    this.updateDirtyElements();
    PUV2ViewBase.contextStack && PUV2ViewBase.contextStack.pop();
  }
}
m([Param], f1.prototype, 'hasHybrid', void 0);
m([Param], f1.prototype, 'item', void 0);
m([Param], f1.prototype, 'selected', void 0);
m([Param], f1.prototype, 'theme', void 0);
m([Param], f1.prototype, 'itemMinFontScale', void 0);
m([Param], f1.prototype, 'itemMaxFontScale', void 0);
m([Param], f1.prototype, 'itemFontColor', void 0);
m([Param], f1.prototype, 'itemSelectedFontColor', void 0);
m([Param], f1.prototype, 'itemFontSize', void 0);
m([Param], f1.prototype, 'itemSelectedFontSize', void 0);
m([Param], f1.prototype, 'itemFontWeight', void 0);
m([Param], f1.prototype, 'itemSelectedFontWeight', void 0);
m([Param], f1.prototype, 'itemIconSize', void 0);
m([Param], f1.prototype, 'itemIconFillColor', void 0);
m([Param], f1.prototype, 'itemSelectedIconFillColor', void 0);
m([Param], f1.prototype, 'itemSymbolFontSize', void 0);
m([Param], f1.prototype, 'itemSymbolFontColor', void 0);
m([Param], f1.prototype, 'itemSelectedSymbolFontColor', void 0);
m([Param], f1.prototype, 'itemMinHeight', void 0);
m([Param], f1.prototype, 'itemPadding', void 0);
m([Param], f1.prototype, 'languageDirection', void 0);
class g1 {
  constructor() {}
  static getInstance() {
    if (!g1.instance) {
      g1.instance = new g1();
    }
    return g1.instance;
  }
  stringify(metrics) {
    switch (metrics.unit) {
      case LengthUnit.PX:
        return `${metrics.value}px`;
      case LengthUnit.VP:
        return `${metrics.value}vp`;
      case LengthUnit.FP:
        return `${metrics.value}fp`;
      case LengthUnit.PERCENT:
        return `${metrics.value}%`;
      case LengthUnit.LPX:
        return `${metrics.value}lpx`;
    }
  }
  t1(metrics) {
    return metrics.value >= 0;
  }
}
function i(context, resource) {
  const resourceManager = context.getHostContext()?.resourceManager;
  if (!resourceManager) {
    return undefined;
  }
  try {
    return resourceManager.getStringSync(resource);
  } catch (err) {
    return undefined;
  }
}
function j(context, resource) {
  const resourceManager = context.getHostContext()?.resourceManager;
  if (!resourceManager) {
    return undefined;
  }
  try {
    return resourceManager.getNumber(resource);
  } catch (err) {
    return undefined;
  }
}
function normalize(value, min, max) {
  return Math.min(Math.max(value, min), max);
}

export default {
  SegmentButtonV2Item,
  SegmentButtonV2Items,
  TabSegmentButtonV2,
  CapsuleSegmentButtonV2,
  MultiCapsuleSegmentButtonV2,
};