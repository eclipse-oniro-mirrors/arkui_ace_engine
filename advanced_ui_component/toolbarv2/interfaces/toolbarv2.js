/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

var __decorate =
  (this && this.__decorate) ||
  function (decorators, target, key, desc) {
    var c = arguments.length,
      r =
        c < 3
          ? target
          : desc === null
          ? (desc = Object.getOwnPropertyDescriptor(target, key))
          : desc,
      d;
    if (typeof Reflect === 'object' && typeof Reflect.decorate === 'function') {
      r = Reflect.decorate(decorators, target, key, desc);
    } else {
      for (var i = decorators.length - 1; i >= 0; i--) {
        if ((d = decorators[i])) {
          r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
        }
      }
    }
    return c > 3 && r && Object.defineProperty(target, key, r), r;
  };
if (!('finalizeConstruction' in ViewPU.prototype)) {
  Reflect.set(ViewPU.prototype, 'finalizeConstruction', () => {});
}
const LengthMetrics = requireNapi('arkui.node').LengthMetrics;
const LengthUnit = requireNapi('arkui.node').LengthUnit;
const ColorMetrics = requireNapi('arkui.node').ColorMetrics;
const DividerModifier = requireNapi('arkui.modifier').DividerModifier;
const hilog = requireNapi('hilog');
const promptAction = requireNapi('promptAction');
const uiMaterial = requireNapi('arkui.uiMaterial');

export var ToolBarV2ItemState;
(function (ToolBarV2ItemState) {
  ToolBarV2ItemState[(ToolBarV2ItemState['ENABLE'] = 1)] = 'ENABLE';
  ToolBarV2ItemState[(ToolBarV2ItemState['DISABLE'] = 2)] = 'DISABLE';
  ToolBarV2ItemState[(ToolBarV2ItemState['ACTIVATE'] = 3)] = 'ACTIVATE';
})(ToolBarV2ItemState || (ToolBarV2ItemState = {}));

export var ToolbarV2ItemPlacement;
(function (ToolbarV2ItemPlacement) {
  ToolbarV2ItemPlacement[(ToolbarV2ItemPlacement['MAIN'] = 0)] = 'MAIN';
  ToolbarV2ItemPlacement[(ToolbarV2ItemPlacement['LEADING'] = 1)] = 'LEADING';
  ToolbarV2ItemPlacement[(ToolbarV2ItemPlacement['TRAILING'] = 2)] = 'TRAILING';
})(ToolbarV2ItemPlacement || (ToolbarV2ItemPlacement = {}));

export var BarBackgroundVisibility;
(function (BarBackgroundVisibility) {
  BarBackgroundVisibility[(BarBackgroundVisibility['VISIBLE'] = 0)] = 'VISIBLE';
  BarBackgroundVisibility[(BarBackgroundVisibility['HIDDEN'] = 1)] = 'HIDDEN';
})(BarBackgroundVisibility || (BarBackgroundVisibility = {}));
const PUBLIC_MORE = {
  id: 125831714,
  type: 40000,
  params: ['sys.symbol.dot_grid_2x2'],
  bundleName: '__harDefaultBundleName__',
  moduleName: '__harDefaultModuleName__',
};
const IMAGE_SIZE = '24vp';
const DEFAULT_TOOLBAR_HEIGHT = 56;
const TOOLBAR_MAX_LENGTH = 5;
const MAX_FONT_SIZE = 3.2;
const DIALOG_IMAGE_SIZE = '64vp';
const MAX_DIALOG = '256vp';
const MIN_DIALOG = '216vp';
const TEXT_TOOLBAR_DIALOG = '18.3fp';
const SCREEN_WIDTH_BREAK_POINT = 640;
const VERTICAL_SCREEN_TEXT_MAX_LINES = 6;
const HORIZONTAL_SCREEN_TEXT_MAX_LINES = 1;
const FOCUS_BOX_MARGIN = -2;
const FOCUS_BOX_BORDER_WIDTH = 2;
const RESOURCE_TYPE_SYMBOL = 40000;
const DEFAULT_ITEM_MAIN_AXIS = 56;
const MAX_ITEM_MAIN_AXIS = 76;
const MIN_BUTTON_WIDTH = 36;
const ITEM_PADDING = 4;
const ICON_TEXT_MARGIN = 2;
const ITEM_MARGIN = 0;
const PRESSED_AREA_SIZE = 48;
const PRESSED_AREA_RADIUS = 24;
const HOVER_BORDER_RADIUS = 26;
const DEFAULT_TITLE_FONT_SIZE = 20;
const DEFAULT_TITLE_MIN_LENGTH = 48;
const DEFAULT_TITLE_FONT_WEIGHT = FontWeight.Bold;
const DEFAULT_ITEM_SPACE = 0;
const GROUP_SPACE_MIN = 8;
const GROUP_SPACE_MAX = 20;
const DEFAULT_GROUP_SPACE = 12;
const DISPLAY_PRIORITY_MIN = 0;
const DISPLAY_PRIORITY_MAX = 10;
const DEFAULT_ITEM_CROSS_AXIS = 48;
const DEFAULT_MAIN_AXIS_PADDING = 4;
const GROUP_MAIN_AXIS_PADDING = 12;
const DEFAULT_STYLE_STATE_EFFECT = true;
const GROUP_DIVIDER_LENGTH = 18;

class Util {
  static isSymbolResource(resourceStr) {
    if (!Util.isResourceType(resourceStr)) {
      return false;
    }
    let resource = resourceStr;
    return resource.type === RESOURCE_TYPE_SYMBOL;
  }

  static isResourceType(resource) {
    if (!resource) {
      return false;
    }
    if (typeof resource === 'string' || typeof resource === 'undefined') {
      return false;
    }
    return true;
  }

  static lengthMetricsToVp(lengthMetrics) {
    switch (lengthMetrics.unit) {
      case LengthUnit.PX:
        return px2vp(lengthMetrics.value);
      case LengthUnit.VP:
        return lengthMetrics.value;
      case LengthUnit.FP:
        return px2vp(fp2px(lengthMetrics.value));
      case LengthUnit.PERCENT:
        return Number.NEGATIVE_INFINITY;
      case LengthUnit.LPX:
        return px2vp(lpx2px(lengthMetrics.value));
      default:
        return lengthMetrics.value;
    }
  }
}

let ToolBarV2SymbolGlyph = class ToolBarV2SymbolGlyph {
  constructor(options) {
    this.normal = options.normal;
    this.activated = options.activated;
  }
};
__decorate([Trace], ToolBarV2SymbolGlyph.prototype, 'normal', void 0);
__decorate([Trace], ToolBarV2SymbolGlyph.prototype, 'activated', void 0);
ToolBarV2SymbolGlyph = __decorate([ObservedV2], ToolBarV2SymbolGlyph);

export { ToolBarV2SymbolGlyph };

class ButtonGestureModifier {
  constructor() {
    this.gestureCallBack = undefined;
  }

  applyGesture(event) {
    this.gestureCallBack?.(event);
  }
}

ButtonGestureModifier.longPressTime = 500;
ButtonGestureModifier.minFontSize = 1.75;
let ToolBarV2ItemText = class ToolBarV2ItemText {
  constructor(options) {
    this.color = ColorMetrics.resourceColor({
      id: 125830982,
      type: 10001,
      params: ['sys.color.font_primary'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    });
    this.activatedColor = ColorMetrics.resourceColor({
      id: 125830986,
      type: 10001,
      params: ['sys.color.font_emphasize'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    });
    this.text = options.text;
    this.color = options.color;
    this.activatedColor = options.activatedColor;
  }
};
__decorate([Trace], ToolBarV2ItemText.prototype, 'text', void 0);
__decorate([Trace], ToolBarV2ItemText.prototype, 'color', void 0);
__decorate([Trace], ToolBarV2ItemText.prototype, 'activatedColor', void 0);
ToolBarV2ItemText = __decorate([ObservedV2], ToolBarV2ItemText);

export { ToolBarV2ItemText };
let ToolBarV2ItemImage = class ToolBarV2ItemImage {
  constructor(options) {
    this.color = undefined;
    this.activatedColor = undefined;
    this.src = options.src;
    this.color = options.color;
    this.activatedColor = options.activatedColor;
  }
};
__decorate([Trace], ToolBarV2ItemImage.prototype, 'src', void 0);
__decorate([Trace], ToolBarV2ItemImage.prototype, 'color', void 0);
__decorate([Trace], ToolBarV2ItemImage.prototype, 'activatedColor', void 0);
ToolBarV2ItemImage = __decorate([ObservedV2], ToolBarV2ItemImage);

export { ToolBarV2ItemImage };
let ToolBarV2Item = class ToolBarV2Item {
  constructor(options) {
    this.content = new ToolBarV2ItemText({ text: '' });
    this.action = undefined;
    this.icon = undefined;
    this.state = 1;
    this.accessibilityText = '';
    this.accessibilityDescription = '';
    this.accessibilityLevel = 'auto';
    this.backgroundColor = Color.Transparent;
    this.content = options.content;
    this.action = options.action;
    this.icon = options.icon;
    this.state = options.state;
    this.accessibilityText = options.accessibilityText;
    this.accessibilityDescription = options.accessibilityDescription;
    this.accessibilityLevel = options.accessibilityLevel;
  }

  get symbol() {
    if (this.icon instanceof ToolBarV2SymbolGlyph) {
      return this.icon;
    }
    return undefined;
  }

  get image() {
    if (!(this.icon instanceof ToolBarV2SymbolGlyph)) {
      return this.icon;
    }
    return undefined;
  }
};
__decorate([Trace], ToolBarV2Item.prototype, 'content', void 0);
__decorate([Trace], ToolBarV2Item.prototype, 'action', void 0);
__decorate([Trace], ToolBarV2Item.prototype, 'icon', void 0);
__decorate([Trace], ToolBarV2Item.prototype, 'state', void 0);
__decorate([Trace], ToolBarV2Item.prototype, 'accessibilityText', void 0);
__decorate(
  [Trace],
  ToolBarV2Item.prototype,
  'accessibilityDescription',
  void 0
);
__decorate([Trace], ToolBarV2Item.prototype, 'accessibilityLevel', void 0);
__decorate([Trace], ToolBarV2Item.prototype, 'backgroundColor', void 0);
__decorate([Computed], ToolBarV2Item.prototype, 'symbol', null);
__decorate([Computed], ToolBarV2Item.prototype, 'image', null);
ToolBarV2Item = __decorate([ObservedV2], ToolBarV2Item);

export { ToolBarV2Item };
let ToolbarV2ItemGroupStyle = class ToolbarV2ItemGroupStyle {
  constructor(init) {
    this.placement = init?.placement;
    this.space = init?.space;
    this.backgroundColor = init?.backgroundColor;
    this.padding = init?.padding;
    this.borderRadius = init?.borderRadius;
    this.borderWidth = init?.borderWidth;
    this.borderColor = init?.borderColor;
    this.stateEffect = init?.stateEffect;
  }
};
__decorate([Trace], ToolbarV2ItemGroupStyle.prototype, 'placement', void 0);
__decorate([Trace], ToolbarV2ItemGroupStyle.prototype, 'space', void 0);
__decorate(
  [Trace],
  ToolbarV2ItemGroupStyle.prototype,
  'backgroundColor',
  void 0
);
__decorate([Trace], ToolbarV2ItemGroupStyle.prototype, 'padding', void 0);
__decorate([Trace], ToolbarV2ItemGroupStyle.prototype, 'borderRadius', void 0);
__decorate([Trace], ToolbarV2ItemGroupStyle.prototype, 'borderWidth', void 0);
__decorate([Trace], ToolbarV2ItemGroupStyle.prototype, 'borderColor', void 0);
__decorate([Trace], ToolbarV2ItemGroupStyle.prototype, 'stateEffect', void 0);
ToolbarV2ItemGroupStyle = __decorate([ObservedV2], ToolbarV2ItemGroupStyle);

export { ToolbarV2ItemGroupStyle };
let ToolbarV2ItemGroup = class ToolbarV2ItemGroup {
  constructor(options) {
    this.placement = options?.placement;
    this.items = options?.items ?? [];
    this.space = options?.space;
    this.style = options?.style;
  }
};
__decorate([Trace], ToolbarV2ItemGroup.prototype, 'placement', void 0);
__decorate([Trace], ToolbarV2ItemGroup.prototype, 'items', void 0);
__decorate([Trace], ToolbarV2ItemGroup.prototype, 'space', void 0);
__decorate([Trace], ToolbarV2ItemGroup.prototype, 'style', void 0);
ToolbarV2ItemGroup = __decorate([ObservedV2], ToolbarV2ItemGroup);
let AdvancedToolbarV2Item = class AdvancedToolbarV2Item extends ToolBarV2Item {
  constructor(options) {
    super({
      content: options.content,
      action: options.action,
      icon: options.icon,
      state: options.state,
      accessibilityText: options.accessibilityText,
      accessibilityDescription: options.accessibilityDescription,
      accessibilityLevel: options.accessibilityLevel,
    });
    this.onStructureChange = undefined;
    this.builder = options.builder;
    this.titleConfigs = options.titleConfigs;
    this.placement = options.placement;
    this.displayPriority = options.displayPriority;
    this.activatedBackgroundColor = options.activatedBackgroundColor;
    this.id = options.id;
    this.itemSize = options.itemSize;
    this.isShowSeparator = options.isShowSeparator;
    this.isShowText = options.isShowText;
    this.isShowOverflowMenu = options.isShowOverflowMenu;
    this.groupID = options.groupID;
    this.groupStyle = options.groupStyle;
  }

  onGroupIDChange(monitor) {
    this.onStructureChange?.();
  }

  onGroupStyleChange(monitor) {
    this.onStructureChange?.();
  }

  onTitleConfigsChange(monitor) {
    this.onStructureChange?.();
  }
};
__decorate([Trace], AdvancedToolbarV2Item.prototype, 'builder', void 0);
__decorate([Trace], AdvancedToolbarV2Item.prototype, 'titleConfigs', void 0);
__decorate([Trace], AdvancedToolbarV2Item.prototype, 'placement', void 0);
__decorate([Trace], AdvancedToolbarV2Item.prototype, 'displayPriority', void 0);
__decorate(
  [Trace],
  AdvancedToolbarV2Item.prototype,
  'activatedBackgroundColor',
  void 0
);
__decorate([Trace], AdvancedToolbarV2Item.prototype, 'id', void 0);
__decorate([Trace], AdvancedToolbarV2Item.prototype, 'itemSize', void 0);
__decorate([Trace], AdvancedToolbarV2Item.prototype, 'isShowSeparator', void 0);
__decorate([Trace], AdvancedToolbarV2Item.prototype, 'isShowText', void 0);
__decorate(
  [Trace],
  AdvancedToolbarV2Item.prototype,
  'isShowOverflowMenu',
  void 0
);
__decorate([Trace], AdvancedToolbarV2Item.prototype, 'groupID', void 0);
__decorate([Trace], AdvancedToolbarV2Item.prototype, 'groupStyle', void 0);
__decorate(
  [Monitor('groupID')],
  AdvancedToolbarV2Item.prototype,
  'onGroupIDChange',
  null
);
__decorate(
  [Monitor('groupStyle')],
  AdvancedToolbarV2Item.prototype,
  'onGroupStyleChange',
  null
);
__decorate(
  [Monitor('titleConfigs')],
  AdvancedToolbarV2Item.prototype,
  'onTitleConfigsChange',
  null
);
AdvancedToolbarV2Item = __decorate([ObservedV2], AdvancedToolbarV2Item);

export { AdvancedToolbarV2Item };
let ToolbarV2TitleStyle = class ToolbarV2TitleStyle {
  constructor(init) {
    this.fontSize = init?.fontSize;
    this.fontColor = init?.fontColor;
    this.fontWeight = init?.fontWeight;
    this.minLength = init?.minLength;
  }
};
__decorate([Trace], ToolbarV2TitleStyle.prototype, 'fontSize', void 0);
__decorate([Trace], ToolbarV2TitleStyle.prototype, 'fontColor', void 0);
__decorate([Trace], ToolbarV2TitleStyle.prototype, 'fontWeight', void 0);
__decorate([Trace], ToolbarV2TitleStyle.prototype, 'minLength', void 0);
ToolbarV2TitleStyle = __decorate([ObservedV2], ToolbarV2TitleStyle);

export { ToolbarV2TitleStyle };
let ToolbarV2TitleConfigs = class ToolbarV2TitleConfigs {
  constructor(init) {
    this.content = init?.content ?? '';
    this.placement = init?.placement;
    this.visible = init?.visible;
    this.style = init?.style;
  }
};
__decorate([Trace], ToolbarV2TitleConfigs.prototype, 'content', void 0);
__decorate([Trace], ToolbarV2TitleConfigs.prototype, 'placement', void 0);
__decorate([Trace], ToolbarV2TitleConfigs.prototype, 'visible', void 0);
__decorate([Trace], ToolbarV2TitleConfigs.prototype, 'style', void 0);
ToolbarV2TitleConfigs = __decorate([ObservedV2], ToolbarV2TitleConfigs);

export { ToolbarV2TitleConfigs };
let ToolbarV2StyleConfigs = class ToolbarV2StyleConfigs {
  constructor(init) {
    this.padding = init?.padding;
    this.backgroundVisibility = init?.backgroundVisibility;
    this.backgroundColor = init?.backgroundColor;
    this.backgroundBlurStyle = init?.backgroundBlurStyle;
    this.systemBackgroundMaterial = init?.systemBackgroundMaterial;
    this.groupSpace = init?.groupSpace;
    this.itemSize = init?.itemSize;
    this.fontSize = init?.fontSize;
    this.borderWidth = init?.borderWidth;
    this.borderColor = init?.borderColor;
    this.borderRadius = init?.borderRadius;
    this.stateEffect = init?.stateEffect;
  }
};
__decorate([Trace], ToolbarV2StyleConfigs.prototype, 'padding', void 0);
__decorate(
  [Trace],
  ToolbarV2StyleConfigs.prototype,
  'backgroundVisibility',
  void 0
);
__decorate([Trace], ToolbarV2StyleConfigs.prototype, 'backgroundColor', void 0);
__decorate(
  [Trace],
  ToolbarV2StyleConfigs.prototype,
  'backgroundBlurStyle',
  void 0
);
__decorate(
  [Trace],
  ToolbarV2StyleConfigs.prototype,
  'systemBackgroundMaterial',
  void 0
);
__decorate([Trace], ToolbarV2StyleConfigs.prototype, 'groupSpace', void 0);
__decorate([Trace], ToolbarV2StyleConfigs.prototype, 'itemSize', void 0);
__decorate([Trace], ToolbarV2StyleConfigs.prototype, 'fontSize', void 0);
__decorate([Trace], ToolbarV2StyleConfigs.prototype, 'borderWidth', void 0);
__decorate([Trace], ToolbarV2StyleConfigs.prototype, 'borderColor', void 0);
__decorate([Trace], ToolbarV2StyleConfigs.prototype, 'borderRadius', void 0);
__decorate([Trace], ToolbarV2StyleConfigs.prototype, 'stateEffect', void 0);
ToolbarV2StyleConfigs = __decorate([ObservedV2], ToolbarV2StyleConfigs);

export { ToolbarV2StyleConfigs };
let AdvancedToolBarV2Modifier = class AdvancedToolBarV2Modifier {
  constructor() {
    this.mainAxisLengthValue = LengthMetrics.percent(1);
    this.directionValue = Axis.Horizontal;
  }

  mainAxisLength(value) {
    this.mainAxisLengthValue = value;
    return this;
  }

  crossAxisLength(value) {
    this.crossAxisLengthValue = value;
    return this;
  }

  direction(value) {
    this.directionValue = value;
    return this;
  }

  style(styleConfigs) {
    this.styleConfigsValue = styleConfigs;
    return this;
  }

  overflowMenuStyle(menuStyle) {
    this.overflowMenuStyleValue = menuStyle;
    return this;
  }
};
__decorate(
  [Trace],
  AdvancedToolBarV2Modifier.prototype,
  'mainAxisLengthValue',
  void 0
);
__decorate(
  [Trace],
  AdvancedToolBarV2Modifier.prototype,
  'crossAxisLengthValue',
  void 0
);
__decorate(
  [Trace],
  AdvancedToolBarV2Modifier.prototype,
  'directionValue',
  void 0
);
__decorate(
  [Trace],
  AdvancedToolBarV2Modifier.prototype,
  'styleConfigsValue',
  void 0
);
__decorate(
  [Trace],
  AdvancedToolBarV2Modifier.prototype,
  'overflowMenuStyleValue',
  void 0
);
AdvancedToolBarV2Modifier = __decorate([ObservedV2], AdvancedToolBarV2Modifier);

export { AdvancedToolBarV2Modifier };
let ToolBarV2Modifier = class ToolBarV2Modifier {
  constructor() {
    this.backgroundColorValue = {
      id: 125829483,
      type: 10001,
      params: ['sys.color.ohos_id_color_toolbar_bg'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    };
    this.heightValue = LengthMetrics.vp(DEFAULT_TOOLBAR_HEIGHT);
    this.stateEffectValue = true;
    this.paddingValue = LengthMetrics.resource({
      id: 125830931,
      type: 10002,
      params: ['sys.float.padding_level12'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    });
  }

  applyNormalAttribute(instance) {
    instance.backgroundColor(
      this.backgroundColorValue ?? {
        id: 125829483,
        type: 10001,
        params: ['sys.color.ohos_id_color_toolbar_bg'],
        bundleName: '__harDefaultBundleName__',
        moduleName: '__harDefaultModuleName__',
      }
    );
  }

  backgroundColor(backgroundColor) {
    this.backgroundColorValue = backgroundColor.color;
    return this;
  }

  height(height) {
    this.heightValue = height;
    return this;
  }

  stateEffect(stateEffect) {
    this.stateEffectValue = stateEffect;
    return this;
  }

  padding(padding) {
    this.paddingValue = padding;
    return this;
  }
};
__decorate(
  [Trace],
  ToolBarV2Modifier.prototype,
  'backgroundColorValue',
  void 0
);
__decorate([Trace], ToolBarV2Modifier.prototype, 'heightValue', void 0);
__decorate([Trace], ToolBarV2Modifier.prototype, 'stateEffectValue', void 0);
__decorate([Trace], ToolBarV2Modifier.prototype, 'paddingValue', void 0);
ToolBarV2Modifier = __decorate([ObservedV2], ToolBarV2Modifier);

export { ToolBarV2Modifier };
let ToolBarV2Theme = class ToolBarV2Theme {
  constructor() {
    this.iconPrimaryColor = {
      id: 125830991,
      type: 10001,
      params: ['sys.color.icon_primary'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    };
    this.iconActivePrimaryColor = {
      id: 125830995,
      type: 10001,
      params: ['sys.color.icon_emphasize'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    };
    this.iconOnPrimaryColor = {
      id: 125831057,
      type: 10001,
      params: ['sys.color.icon_on_primary'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    };
    this.fontPrimaryColor = {
      id: 125830982,
      type: 10001,
      params: ['sys.color.font_primary'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    };
    this.fontActivatedPrimaryColor = {
      id: 125830986,
      type: 10001,
      params: ['sys.color.font_emphasize'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    };
    this.fontSecondaryColor = {
      id: 125830983,
      type: 10001,
      params: ['sys.color.font_secondary'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    };
  }
};
__decorate([Trace], ToolBarV2Theme.prototype, 'iconPrimaryColor', void 0);
__decorate([Trace], ToolBarV2Theme.prototype, 'iconActivePrimaryColor', void 0);
__decorate([Trace], ToolBarV2Theme.prototype, 'iconOnPrimaryColor', void 0);
__decorate([Trace], ToolBarV2Theme.prototype, 'fontPrimaryColor', void 0);
__decorate(
  [Trace],
  ToolBarV2Theme.prototype,
  'fontActivatedPrimaryColor',
  void 0
);
__decorate([Trace], ToolBarV2Theme.prototype, 'fontSecondaryColor', void 0);
ToolBarV2Theme = __decorate([ObservedV2], ToolBarV2Theme);

export class ToolBarV2 extends ViewV2 {
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
      'toolBarList',
      params && 'toolBarList' in params ? params.toolBarList : undefined
    );
    this.initParam(
      'activatedIndex',
      params && 'activatedIndex' in params ? params.activatedIndex : -1
    );
    this.initParam(
      'dividerModifier',
      params && 'dividerModifier' in params
        ? params.dividerModifier
        : new DividerModifier()
    );
    this.initParam(
      'toolBarModifier',
      params && 'toolBarModifier' in params
        ? params.toolBarModifier
        : new ToolBarV2Modifier()
            .padding(
              LengthMetrics.resource({
                id: 125830931,
                type: 10002,
                params: ['sys.float.padding_level12'],
                bundleName: '__harDefaultBundleName__',
                moduleName: '__harDefaultModuleName__',
              })
            )
            .stateEffect(true)
            .height(LengthMetrics.vp(DEFAULT_TOOLBAR_HEIGHT))
    );
    this.initParam(
      'advancedToolBarModifier',
      params && 'advancedToolBarModifier' in params
        ? params.advancedToolBarModifier
        : undefined
    );
    this.localActivatedIndex = -1;
    this.menuContent = [];
    this.fontSize = 1;
    this.theme = new ToolBarV2Theme();
    this.isAdvancedMode = false;
    this.advMainAxisSize = 0;
    this.advIsMeasuring = false;
    this.advHoveredItems = new Set();
    this.advTitleConfigs = undefined;
    this.itemContentBuilder =
      'itemContentBuilder' in params ? params.itemContentBuilder : undefined;
    this.advEntries = [];
    this.advItemOriginalIndex = new Map();
    this.itemCardTextMaxLine = 1;
    this.itemDialogId = undefined;
    this.isFollowSystem = false;
    this.maxFontSizeScale = 3.2;
    this.moreItem = new ToolBarV2Item({
      content: new ToolBarV2ItemText({
        text: {
          id: 125833704,
          type: 10003,
          params: ['sys.string.ohos_toolbar_more'],
          bundleName: '__harDefaultBundleName__',
          moduleName: '__harDefaultModuleName__',
        },
      }),
      icon: new ToolBarV2ItemImage({
        src: PUBLIC_MORE,
      }),
    });
    this.moreText = {
      id: 125833704,
      type: 10003,
      params: ['sys.string.ohos_toolbar_more'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    };
    this.finalizeConstruction();
  }

  resetStateVarsOnReuse(params) {
    this.resetParam(
      'toolBarList',
      params && 'toolBarList' in params ? params.toolBarList : undefined
    );
    this.resetParam(
      'activatedIndex',
      params && 'activatedIndex' in params ? params.activatedIndex : -1
    );
    this.resetParam(
      'dividerModifier',
      params && 'dividerModifier' in params
        ? params.dividerModifier
        : new DividerModifier()
    );
    this.resetParam(
      'toolBarModifier',
      params && 'toolBarModifier' in params
        ? params.toolBarModifier
        : new ToolBarV2Modifier()
            .padding(
              LengthMetrics.resource({
                id: 125830931,
                type: 10002,
                params: ['sys.float.padding_level12'],
                bundleName: '__harDefaultBundleName__',
                moduleName: '__harDefaultModuleName__',
              })
            )
            .stateEffect(true)
            .height(LengthMetrics.vp(DEFAULT_TOOLBAR_HEIGHT))
    );
    this.resetParam(
      'advancedToolBarModifier',
      params && 'advancedToolBarModifier' in params
        ? params.advancedToolBarModifier
        : undefined
    );
    this.localActivatedIndex = -1;
    this.menuContent = [];
    this.fontSize = 1;
    this.theme = new ToolBarV2Theme();
    this.isAdvancedMode = false;
    this.advMainAxisSize = 0;
    this.advIsMeasuring = false;
    this.advHoveredItems = new Set();
    this.advTitleConfigs = undefined;
    this.itemContentBuilder =
      'itemContentBuilder' in params ? params.itemContentBuilder : undefined;
    this.advEntries = [];
    this.resetComputed('menus');
    this.resetMonitorsOnReuse();
  }

  get menus() {
    this.menuContent = [];
    this.toolBarList.forEach((value, index) => {
      if (index >= TOOLBAR_MAX_LENGTH - 1) {
        this.menuContent.push({
          value: this.toolBarList[index].content.text,
          action: () => {
            let callback = this.toolBarList[index].action;
            if (callback) {
              callback(index);
            }
          },
          enabled: this.toolBarList[index].state !== ToolBarV2ItemState.DISABLE,
        });
      }
    });
    return this.menuContent;
  }

  onActivateIndexChange(monitor) {
    this.localActivatedIndex = monitor.value('activatedIndex')?.now ?? -1;
  }

  onToolBarListChange(monitor) {
    if (this.isAdvancedMode) {
      this.rebuildAdvState();
    }
  }

  aboutToAppear() {
    this.localActivatedIndex = this.activatedIndex ?? -1;
    this.isAdvancedMode = this.advancedToolBarModifier !== undefined;
    try {
      this.isFollowSystem = this.getUIContext()?.isFollowingSystemFontScale();
      this.maxFontSizeScale = this.getUIContext()?.getMaxFontScale();
    } catch (err) {
      let code = err?.code;
      let message = err?.message;
      hilog.error(
        0x3900,
        'Ace',
        `Faild to toolBarV2 getMaxFontScale, code: ${code}, message: ${message}`
      );
    }
    if (this.isAdvancedMode) {
      this.rebuildAdvState();
    }
  }

  onWillApplyTheme(theme) {
    this.theme.iconPrimaryColor = theme.colors.iconPrimary;
    this.theme.iconActivePrimaryColor = theme.colors.iconEmphasize;
    this.theme.iconOnPrimaryColor = theme.colors.iconOnPrimary;
    this.theme.fontPrimaryColor = theme.colors.fontPrimary;
    this.theme.fontActivatedPrimaryColor = theme.colors.fontEmphasize;
    this.theme.fontSecondaryColor = theme.colors.fontSecondary;
  }

  MoreTabBuilder(index, parent = null) {
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      Button.createWithChild({ type: ButtonType.Normal, stateEffect: false });
      Button.accessibilityGroup(true);
      Button.focusable(true);
      Button.focusOnTouch(true);
      Button.focusBox({
        margin: LengthMetrics.vp(FOCUS_BOX_MARGIN),
        strokeWidth: LengthMetrics.vp(FOCUS_BOX_BORDER_WIDTH),
        strokeColor: ColorMetrics.resourceColor({
          id: 125829180,
          type: 10001,
          params: ['sys.color.ohos_id_color_focused_outline'],
          bundleName: '__harDefaultBundleName__',
          moduleName: '__harDefaultModuleName__',
        }),
      });
      Button.width('100%');
      Button.height('100%');
      Button.bindMenu(this.menuContent, {
        placement: Placement.TopRight,
        offset: { x: -12, y: -10 },
      });
      Button.borderRadius({
        id: 125829706,
        type: 10002,
        params: ['sys.float.ohos_id_corner_radius_clicked'],
        bundleName: '__harDefaultBundleName__',
        moduleName: '__harDefaultModuleName__',
      });
      Button.backgroundColor(this.toolBarList[index].backgroundColor);
      Button.onHover((isHover) => {
        if (isHover) {
          this.toolBarList[index].backgroundColor = {
            id: 125829168,
            type: 10001,
            params: ['sys.color.ohos_id_color_hover'],
            bundleName: '__harDefaultBundleName__',
            moduleName: '__harDefaultModuleName__',
          };
        } else {
          this.toolBarList[index].backgroundColor = Color.Transparent;
        }
      });
      ViewStackProcessor.visualState('pressed');
      Button.backgroundColor(
        !this.toolBarModifier?.stateEffectValue
          ? this.toolBarList[index].backgroundColor
          : {
              id: 125829165,
              type: 10001,
              params: ['sys.color.ohos_id_color_click_effect'],
              bundleName: '__harDefaultBundleName__',
              moduleName: '__harDefaultModuleName__',
            }
      );
      ViewStackProcessor.visualState();
      Button.gestureModifier(this.getItemGestureModifier(this.moreItem, index));
    }, Button);
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      Column.create();
      Column.width('100%');
      Column.height('100%');
      Column.justifyContent(FlexAlign.Center);
      Column.padding({
        start: LengthMetrics.resource({
          id: 125830921,
          type: 10002,
          params: ['sys.float.padding_level2'],
          bundleName: '__harDefaultBundleName__',
          moduleName: '__harDefaultModuleName__',
        }),
        end: LengthMetrics.resource({
          id: 125830921,
          type: 10002,
          params: ['sys.float.padding_level2'],
          bundleName: '__harDefaultBundleName__',
          moduleName: '__harDefaultModuleName__',
        }),
      });
      Column.borderRadius({
        id: 125829706,
        type: 10002,
        params: ['sys.float.ohos_id_corner_radius_clicked'],
        bundleName: '__harDefaultBundleName__',
        moduleName: '__harDefaultModuleName__',
      });
    }, Column);
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      SymbolGlyph.create(PUBLIC_MORE);
      SymbolGlyph.fontSize(IMAGE_SIZE);
      SymbolGlyph.fontColor([this.theme.iconPrimaryColor]);
      SymbolGlyph.draggable(false);
      SymbolGlyph.margin({
        bottom: {
          id: 125830920,
          type: 10002,
          params: ['sys.float.padding_level1'],
          bundleName: '__harDefaultBundleName__',
          moduleName: '__harDefaultModuleName__',
        },
      });
    }, SymbolGlyph);
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      Text.create(this.moreText);
      Text.fontColor(this.theme.fontPrimaryColor);
      Text.fontSize({
        id: 125829687,
        type: 10002,
        params: ['sys.float.ohos_id_text_size_caption'],
        bundleName: '__harDefaultBundleName__',
        moduleName: '__harDefaultModuleName__',
      });
      Text.fontWeight(FontWeight.Medium);
      Text.maxLines(1);
      Text.textOverflow({ overflow: TextOverflow.Ellipsis });
      Text.textAlign(TextAlign.Center);
      Text.focusable(true);
      Text.focusOnTouch(true);
    }, Text);
    Text.pop();
    Column.pop();
    Button.pop();
  }

  TabBuilder(index, parent = null) {
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      Button.createWithChild({ type: ButtonType.Normal, stateEffect: false });
      Button.accessibilityGroup(true);
      Button.accessibilityText(
        this.toolBarList[index]?.accessibilityText ??
          this.toolBarList[index]?.content?.text
      );
      Button.accessibilityDescription(
        this.toolBarList[index]?.accessibilityDescription ?? ''
      );
      Button.accessibilityLevel(
        this.toolBarList[index]?.accessibilityLevel ?? 'auto'
      );
      Button.enabled(
        this.toolBarList[index]?.state !== ToolBarV2ItemState.DISABLE
      );
      Button.width('100%');
      Button.height('100%');
      Button.borderRadius({
        id: 125829706,
        type: 10002,
        params: ['sys.float.ohos_id_corner_radius_clicked'],
        bundleName: '__harDefaultBundleName__',
        moduleName: '__harDefaultModuleName__',
      });
      Button.focusable(
        !(this.toolBarList[index]?.state === ToolBarV2ItemState.DISABLE)
      );
      Button.focusOnTouch(
        !(this.toolBarList[index]?.state === ToolBarV2ItemState.DISABLE)
      );
      Button.focusBox({
        margin: LengthMetrics.vp(FOCUS_BOX_MARGIN),
        strokeWidth: LengthMetrics.vp(FOCUS_BOX_BORDER_WIDTH),
        strokeColor: ColorMetrics.resourceColor({
          id: 125829180,
          type: 10001,
          params: ['sys.color.ohos_id_color_focused_outline'],
          bundleName: '__harDefaultBundleName__',
          moduleName: '__harDefaultModuleName__',
        }),
      });
      Button.backgroundColor(this.toolBarList[index].backgroundColor);
      Button.onHover((isHover) => {
        if (
          isHover &&
          this.toolBarList[index]?.state !== ToolBarV2ItemState.DISABLE
        ) {
          this.toolBarList[index].backgroundColor = {
            id: 125829168,
            type: 10001,
            params: ['sys.color.ohos_id_color_hover'],
            bundleName: '__harDefaultBundleName__',
            moduleName: '__harDefaultModuleName__',
          };
        } else {
          this.toolBarList[index].backgroundColor = Color.Transparent;
        }
      });
      ViewStackProcessor.visualState('pressed');
      Button.backgroundColor(
        this.toolBarList[index]?.state === ToolBarV2ItemState.DISABLE ||
          !this.toolBarModifier?.stateEffectValue
          ? this.toolBarList[index].backgroundColor
          : {
              id: 125829165,
              type: 10001,
              params: ['sys.color.ohos_id_color_click_effect'],
              bundleName: '__harDefaultBundleName__',
              moduleName: '__harDefaultModuleName__',
            }
      );
      ViewStackProcessor.visualState();
      Button.onClick(() => {
        this.clickEventAction(index);
      });
      Button.gestureModifier(
        this.getItemGestureModifier(this.toolBarList[index], index)
      );
    }, Button);
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      Column.create();
      Column.justifyContent(FlexAlign.Center);
      Column.width('100%');
      Column.height('100%');
      Column.borderRadius({
        id: 125829706,
        type: 10002,
        params: ['sys.float.ohos_id_corner_radius_clicked'],
        bundleName: '__harDefaultBundleName__',
        moduleName: '__harDefaultModuleName__',
      });
      Column.padding({
        start: LengthMetrics.resource({
          id: 125830921,
          type: 10002,
          params: ['sys.float.padding_level2'],
          bundleName: '__harDefaultBundleName__',
          moduleName: '__harDefaultModuleName__',
        }),
        end: LengthMetrics.resource({
          id: 125830921,
          type: 10002,
          params: ['sys.float.padding_level2'],
          bundleName: '__harDefaultBundleName__',
          moduleName: '__harDefaultModuleName__',
        }),
      });
    }, Column);
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      If.create();
      if (this.toolBarList[index]?.symbol) {
        this.ifElseBranchUpdateFunction(0, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            SymbolGlyph.create();
            SymbolGlyph.fontSize(IMAGE_SIZE);
            SymbolGlyph.symbolEffect(new SymbolEffect(), false);
            SymbolGlyph.attributeModifier.bind(this)(
              this.getToolBarSymbolModifier(index)
            );
            SymbolGlyph.margin({
              bottom: {
                id: 125830920,
                type: 10002,
                params: ['sys.float.padding_level1'],
                bundleName: '__harDefaultBundleName__',
                moduleName: '__harDefaultModuleName__',
              },
            });
          }, SymbolGlyph);
        });
      } else if (Util.isSymbolResource(this.toolBarList[index]?.image?.src)) {
        this.ifElseBranchUpdateFunction(1, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            SymbolGlyph.create(this.toolBarList[index]?.image?.src);
            SymbolGlyph.fontSize(IMAGE_SIZE);
            SymbolGlyph.fontColor([this.getIconColor(index)]);
            SymbolGlyph.margin({
              bottom: {
                id: 125830920,
                type: 10002,
                params: ['sys.float.padding_level1'],
                bundleName: '__harDefaultBundleName__',
                moduleName: '__harDefaultModuleName__',
              },
            });
          }, SymbolGlyph);
        });
      } else {
        this.ifElseBranchUpdateFunction(2, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Image.create(this.toolBarList[index]?.image?.src);
            Image.width(IMAGE_SIZE);
            Image.height(IMAGE_SIZE);
            Image.fillColor(this.getIconColor(index));
            Image.margin({
              bottom: {
                id: 125830920,
                type: 10002,
                params: ['sys.float.padding_level1'],
                bundleName: '__harDefaultBundleName__',
                moduleName: '__harDefaultModuleName__',
              },
            });
            Image.objectFit(ImageFit.Contain);
            Image.draggable(false);
          }, Image);
        });
      }
    }, If);
    If.pop();
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      Text.create(this.toolBarList[index]?.content.text);
      Text.fontColor(this.getTextColor(index));
      Text.fontSize({
        id: 125829687,
        type: 10002,
        params: ['sys.float.ohos_id_text_size_caption'],
        bundleName: '__harDefaultBundleName__',
        moduleName: '__harDefaultModuleName__',
      });
      Text.maxFontSize({
        id: 125829687,
        type: 10002,
        params: ['sys.float.ohos_id_text_size_caption'],
        bundleName: '__harDefaultBundleName__',
        moduleName: '__harDefaultModuleName__',
      });
      Text.minFontSize(9);
      Text.fontWeight(FontWeight.Medium);
      Text.maxLines(1);
      Text.textOverflow({ overflow: TextOverflow.Ellipsis });
      Text.textAlign(TextAlign.Center);
      Text.focusable(
        !(this.toolBarList[index]?.state === ToolBarV2ItemState.DISABLE)
      );
      Text.focusOnTouch(
        !(this.toolBarList[index]?.state === ToolBarV2ItemState.DISABLE)
      );
    }, Text);
    Text.pop();
    Column.pop();
    Button.pop();
  }

  itemCardDialogBuilder(item, index, parent = null) {
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      If.create();
      if (item.content && item.content.text) {
        this.ifElseBranchUpdateFunction(0, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Column.create();
            Column.constraintSize({
              minHeight:
                this.fontSize === MAX_FONT_SIZE ? MAX_DIALOG : MIN_DIALOG,
            });
          }, Column);
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            If.create();
            if (item.symbol) {
              this.ifElseBranchUpdateFunction(0, () => {
                this.observeComponentCreation2((elmtId, isInitialRender) => {
                  SymbolGlyph.create();
                  SymbolGlyph.attributeModifier.bind(this)(
                    this.getToolBarSymbolModifier(index)
                  );
                  SymbolGlyph.symbolEffect(new SymbolEffect(), false);
                  SymbolGlyph.fontColor([
                    {
                      id: 125830991,
                      type: 10001,
                      params: ['sys.color.icon_primary'],
                      bundleName: '__harDefaultBundleName__',
                      moduleName: '__harDefaultModuleName__',
                    },
                  ]);
                  SymbolGlyph.fontSize(DIALOG_IMAGE_SIZE);
                  SymbolGlyph.margin({
                    top: {
                      id: 125830933,
                      type: 10002,
                      params: ['sys.float.padding_level24'],
                      bundleName: '__harDefaultBundleName__',
                      moduleName: '__harDefaultModuleName__',
                    },
                    bottom: {
                      id: 125830927,
                      type: 10002,
                      params: ['sys.float.padding_level8'],
                      bundleName: '__harDefaultBundleName__',
                      moduleName: '__harDefaultModuleName__',
                    },
                  });
                }, SymbolGlyph);
              });
            } else if (Util.isSymbolResource(item.image?.src)) {
              this.ifElseBranchUpdateFunction(1, () => {
                this.observeComponentCreation2((elmtId, isInitialRender) => {
                  SymbolGlyph.create(item.image?.src);
                  SymbolGlyph.fontColor([
                    {
                      id: 125830991,
                      type: 10001,
                      params: ['sys.color.icon_primary'],
                      bundleName: '__harDefaultBundleName__',
                      moduleName: '__harDefaultModuleName__',
                    },
                  ]);
                  SymbolGlyph.fontSize(DIALOG_IMAGE_SIZE);
                  SymbolGlyph.margin({
                    top: {
                      id: 125830933,
                      type: 10002,
                      params: ['sys.float.padding_level24'],
                      bundleName: '__harDefaultBundleName__',
                      moduleName: '__harDefaultModuleName__',
                    },
                    bottom: {
                      id: 125830927,
                      type: 10002,
                      params: ['sys.float.padding_level8'],
                      bundleName: '__harDefaultBundleName__',
                      moduleName: '__harDefaultModuleName__',
                    },
                  });
                }, SymbolGlyph);
              });
            } else {
              this.ifElseBranchUpdateFunction(2, () => {
                this.observeComponentCreation2((elmtId, isInitialRender) => {
                  Image.create(item.image?.src);
                  Image.width(DIALOG_IMAGE_SIZE);
                  Image.height(DIALOG_IMAGE_SIZE);
                  Image.margin({
                    top: {
                      id: 125830933,
                      type: 10002,
                      params: ['sys.float.padding_level24'],
                      bundleName: '__harDefaultBundleName__',
                      moduleName: '__harDefaultModuleName__',
                    },
                    bottom: {
                      id: 125830927,
                      type: 10002,
                      params: ['sys.float.padding_level8'],
                      bundleName: '__harDefaultBundleName__',
                      moduleName: '__harDefaultModuleName__',
                    },
                  });
                  Image.fillColor({
                    id: 125830991,
                    type: 10001,
                    params: ['sys.color.icon_primary'],
                    bundleName: '__harDefaultBundleName__',
                    moduleName: '__harDefaultModuleName__',
                  });
                }, Image);
              });
            }
          }, If);
          If.pop();
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Column.create();
            Column.width('100%');
            Column.padding({
              left: {
                id: 125830923,
                type: 10002,
                params: ['sys.float.padding_level4'],
                bundleName: '__harDefaultBundleName__',
                moduleName: '__harDefaultModuleName__',
              },
              right: {
                id: 125830923,
                type: 10002,
                params: ['sys.float.padding_level4'],
                bundleName: '__harDefaultBundleName__',
                moduleName: '__harDefaultModuleName__',
              },
              bottom: {
                id: 125830931,
                type: 10002,
                params: ['sys.float.padding_level12'],
                bundleName: '__harDefaultBundleName__',
                moduleName: '__harDefaultModuleName__',
              },
            });
          }, Column);
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Text.create(item.content.text);
            Text.fontSize(TEXT_TOOLBAR_DIALOG);
            Text.textOverflow({ overflow: TextOverflow.Ellipsis });
            Text.maxLines(this.itemCardTextMaxLine);
            Text.width('100%');
            Text.textAlign(TextAlign.Center);
            Text.fontColor({
              id: 125830982,
              type: 10001,
              params: ['sys.color.font_primary'],
              bundleName: '__harDefaultBundleName__',
              moduleName: '__harDefaultModuleName__',
            });
          }, Text);
          Text.pop();
          Column.pop();
          Column.pop();
        });
      } else {
        this.ifElseBranchUpdateFunction(1, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Column.create();
            Column.constraintSize({
              minHeight:
                this.fontSize === MAX_FONT_SIZE ? MAX_DIALOG : MIN_DIALOG,
            });
            Column.justifyContent(FlexAlign.Center);
          }, Column);
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            If.create();
            if (item.symbol) {
              this.ifElseBranchUpdateFunction(0, () => {
                this.observeComponentCreation2((elmtId, isInitialRender) => {
                  SymbolGlyph.create();
                  SymbolGlyph.attributeModifier.bind(this)(
                    this.getToolBarSymbolModifier(index)
                  );
                  SymbolGlyph.symbolEffect(new SymbolEffect(), false);
                  SymbolGlyph.fontColor([
                    {
                      id: 125830991,
                      type: 10001,
                      params: ['sys.color.icon_primary'],
                      bundleName: '__harDefaultBundleName__',
                      moduleName: '__harDefaultModuleName__',
                    },
                  ]);
                  SymbolGlyph.fontSize(DIALOG_IMAGE_SIZE);
                }, SymbolGlyph);
              });
            } else if (Util.isSymbolResource(item.image?.src)) {
              this.ifElseBranchUpdateFunction(1, () => {
                this.observeComponentCreation2((elmtId, isInitialRender) => {
                  SymbolGlyph.create(item.image?.src);
                  SymbolGlyph.fontColor([
                    {
                      id: 125830991,
                      type: 10001,
                      params: ['sys.color.icon_primary'],
                      bundleName: '__harDefaultBundleName__',
                      moduleName: '__harDefaultModuleName__',
                    },
                  ]);
                  SymbolGlyph.fontSize(DIALOG_IMAGE_SIZE);
                }, SymbolGlyph);
              });
            } else {
              this.ifElseBranchUpdateFunction(2, () => {
                this.observeComponentCreation2((elmtId, isInitialRender) => {
                  Image.create(item.image?.src);
                  Image.width(DIALOG_IMAGE_SIZE);
                  Image.height(DIALOG_IMAGE_SIZE);
                  Image.fillColor({
                    id: 125830991,
                    type: 10001,
                    params: ['sys.color.icon_primary'],
                    bundleName: '__harDefaultBundleName__',
                    moduleName: '__harDefaultModuleName__',
                  });
                }, Image);
              });
            }
          }, If);
          If.pop();
          Column.pop();
        });
      }
    }, If);
    If.pop();
  }

  onMeasureSize(selfLayoutInfo, children, constraint) {
    this.fontSize = this.getFontSizeScale();
    let sizeResult = { height: 0, width: 0 };
    children.forEach((child) => {
      let childMeasureResult = child.measure(constraint);
      sizeResult.width = childMeasureResult.width;
      sizeResult.height = childMeasureResult.height;
    });
    return sizeResult;
  }

  renderAdvItem(item, inGroup = false, parent = null) {
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      If.create();
      if (this.setItemContentBuilder(item) && this.itemContentBuilder) {
        this.ifElseBranchUpdateFunction(0, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            If.create();
            if (this.getEffectiveDirection() === Axis.Vertical) {
              this.ifElseBranchUpdateFunction(0, () => {
                if (!If.canRetake(item.id)) {
                  this.observeComponentCreation2((elmtId, isInitialRender) => {
                    Column.create();
                    Column.width(
                      this.getEffectiveItemWidth(item, inGroup).value
                    );
                    Column.height(
                      this.getEffectiveItemHeight(item, inGroup).value
                    );
                    Column.justifyContent(FlexAlign.Center);
                    Column.alignItems(HorizontalAlign.Center);
                    Column.borderRadius(
                      this.getEffectiveItemBorderRadius(inGroup)
                    );
                    Column.backgroundColor(
                      this.getEffectiveItemDisplayBg(item, inGroup)
                    );
                    Column.onHover((isHover) => {
                      this.setItemHovered(item, isHover);
                    });
                    ViewStackProcessor.visualState('pressed');
                    Column.backgroundColor(
                      this.getEffectiveItemPressedBg(item, inGroup)
                    );
                    ViewStackProcessor.visualState();
                    Column.onClick(() => {
                      if (
                        this.getEffectiveItemState(item) ===
                        ToolBarV2ItemState.DISABLE
                      ) {
                        return;
                      }
                      this.advClickEventAction(item);
                    });
                    Column.accessibilityGroup(true);
                    Column.accessibilityText(
                      this.getAdvAccessibilityText(item)
                    );
                    Column.accessibilityDescription(
                      this.getAdvAccessibilityDescription(item)
                    );
                    Column.accessibilityLevel(item.accessibilityLevel);
                    Column.enabled(
                      this.getEffectiveItemState(item) !==
                        ToolBarV2ItemState.DISABLE
                    );
                    Column.opacity(
                      this.getEffectiveItemState(item) ===
                        ToolBarV2ItemState.DISABLE
                        ? 0.4
                        : 1
                    );
                    Column.id(item.id);
                  }, Column);
                  this.itemContentBuilder.bind(this)();
                  Column.pop();
                }
              });
            } else {
              this.ifElseBranchUpdateFunction(1, () => {
                if (!If.canRetake(item.id)) {
                  this.observeComponentCreation2((elmtId, isInitialRender) => {
                    Row.create();
                    Row.width(this.getEffectiveItemWidth(item, inGroup).value);
                    Row.height(
                      this.getEffectiveItemHeight(item, inGroup).value
                    );
                    Row.justifyContent(FlexAlign.Center);
                    Row.alignItems(VerticalAlign.Center);
                    Row.borderRadius(
                      this.getEffectiveItemBorderRadius(inGroup)
                    );
                    Row.backgroundColor(
                      this.getEffectiveItemDisplayBg(item, inGroup)
                    );
                    Row.onHover((isHover) => {
                      this.setItemHovered(item, isHover);
                    });
                    ViewStackProcessor.visualState('pressed');
                    Row.backgroundColor(
                      this.getEffectiveItemPressedBg(item, inGroup)
                    );
                    ViewStackProcessor.visualState();
                    Row.onClick(() => {
                      if (
                        this.getEffectiveItemState(item) ===
                        ToolBarV2ItemState.DISABLE
                      ) {
                        return;
                      }
                      this.advClickEventAction(item);
                    });
                    Row.accessibilityGroup(true);
                    Row.accessibilityText(this.getAdvAccessibilityText(item));
                    Row.accessibilityDescription(
                      this.getAdvAccessibilityDescription(item)
                    );
                    Row.accessibilityLevel(item.accessibilityLevel);
                    Row.enabled(
                      this.getEffectiveItemState(item) !==
                        ToolBarV2ItemState.DISABLE
                    );
                    Row.opacity(
                      this.getEffectiveItemState(item) ===
                        ToolBarV2ItemState.DISABLE
                        ? 0.4
                        : 1
                    );
                    Row.id(item.id);
                  }, Row);
                  this.itemContentBuilder.bind(this)();
                  Row.pop();
                }
              });
            }
          }, If);
          If.pop();
        });
      } else {
        this.ifElseBranchUpdateFunction(1, () => {
          if (!If.canRetake(item.id)) {
            this.observeComponentCreation2((elmtId, isInitialRender) => {
              Button.createWithChild({
                type: ButtonType.Normal,
                stateEffect: false,
              });
              Button.width(this.getEffectiveItemWidth(item, inGroup).value);
              Button.height(this.getEffectiveItemHeight(item, inGroup).value);
              Button.hoverEffect(HoverEffect.None);
              Button.borderRadius(
                this.getEffectiveItemBorderRadius(false, item)
              );
              Button.backgroundColor(
                this.getEffectiveItemDisplayBg(item, inGroup)
              );
              Button.onHover((isHover) => {
                this.setItemHovered(item, isHover);
              });
              ViewStackProcessor.visualState('pressed');
              Button.backgroundColor(
                this.getEffectiveItemPressedBg(item, inGroup)
              );
              ViewStackProcessor.visualState();
              Button.onClick(() => {
                if (
                  this.getEffectiveItemState(item) ===
                  ToolBarV2ItemState.DISABLE
                ) {
                  return;
                }
                this.advClickEventAction(item);
              });
              Button.accessibilityGroup(true);
              Button.accessibilityText(this.getAdvAccessibilityText(item));
              Button.accessibilityDescription(
                this.getAdvAccessibilityDescription(item)
              );
              Button.accessibilityLevel(item.accessibilityLevel);
              Button.enabled(
                this.getEffectiveItemState(item) !== ToolBarV2ItemState.DISABLE
              );
              Button.opacity(
                this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE
                  ? 0.4
                  : 1
              );
              Button.padding(0);
              Button.id(item.id);
            }, Button);
            this.observeComponentCreation2((elmtId, isInitialRender) => {
              Button.createWithChild({
                type: ButtonType.Normal,
                stateEffect: false,
              });
              Button.width(
                this.getEffectiveItemInnerWidth(item, inGroup).value
              );
              Button.height(
                this.getEffectiveItemInnerHeight(item, inGroup).value
              );
              Button.padding({
                start: LengthMetrics.vp(ITEM_PADDING),
                end: LengthMetrics.vp(ITEM_PADDING),
              });
              Button.borderRadius(
                this.getEffectiveItemBorderRadius(inGroup, item)
              );
              Button.backgroundColor(this.getEffectiveItemInnerDisplayBg(item));
            }, Button);
            this.observeComponentCreation2((elmtId, isInitialRender) => {
              Column.create();
              Column.justifyContent(FlexAlign.Center);
              Column.alignItems(HorizontalAlign.Center);
              Column.padding({
                top: LengthMetrics.vp(ITEM_PADDING),
                bottom: LengthMetrics.vp(ITEM_PADDING),
              });
            }, Column);
            this.observeComponentCreation2((elmtId, isInitialRender) => {
              If.create();
              if (item.icon) {
                this.ifElseBranchUpdateFunction(0, () => {
                  this.observeComponentCreation2((elmtId, isInitialRender) => {
                    If.create();
                    if (item.icon instanceof ToolBarV2SymbolGlyph) {
                      this.ifElseBranchUpdateFunction(0, () => {
                        this.observeComponentCreation2(
                          (elmtId, isInitialRender) => {
                            SymbolGlyph.create();
                            SymbolGlyph.fontSize(IMAGE_SIZE);
                            SymbolGlyph.symbolEffect(new SymbolEffect(), false);
                            SymbolGlyph.attributeModifier.bind(this)(
                              this.isItemActivated(item)
                                ? item.icon.activated ?? item.icon.normal
                                : item.icon.normal
                            );
                            SymbolGlyph.fontColor([
                              this.getEffectiveItemIconColor(item),
                            ]);
                            SymbolGlyph.margin({
                              bottom:
                                item.content && item.isShowText !== false
                                  ? LengthMetrics.vp(ICON_TEXT_MARGIN)
                                  : LengthMetrics.vp(0),
                            });
                          },
                          SymbolGlyph
                        );
                      });
                    } else if (Util.isSymbolResource(item.icon.src)) {
                      this.ifElseBranchUpdateFunction(1, () => {
                        this.observeComponentCreation2(
                          (elmtId, isInitialRender) => {
                            SymbolGlyph.create(item.icon.src);
                            SymbolGlyph.fontSize(IMAGE_SIZE);
                            SymbolGlyph.fontColor([
                              this.getEffectiveItemIconColor(item),
                            ]);
                            SymbolGlyph.margin({
                              bottom:
                                item.content && item.isShowText !== false
                                  ? LengthMetrics.vp(ICON_TEXT_MARGIN)
                                  : LengthMetrics.vp(0),
                            });
                          },
                          SymbolGlyph
                        );
                      });
                    } else {
                      this.ifElseBranchUpdateFunction(2, () => {
                        this.observeComponentCreation2(
                          (elmtId, isInitialRender) => {
                            Image.create(item.icon.src);
                            Image.width(IMAGE_SIZE);
                            Image.height(IMAGE_SIZE);
                            Image.fillColor(
                              this.getEffectiveItemIconColor(item)
                            );
                            Image.objectFit(ImageFit.Contain);
                            Image.draggable(false);
                            Image.margin({
                              bottom:
                                item.content && item.isShowText !== false
                                  ? LengthMetrics.vp(ICON_TEXT_MARGIN)
                                  : LengthMetrics.vp(0),
                            });
                          },
                          Image
                        );
                      });
                    }
                  }, If);
                  If.pop();
                  this.observeComponentCreation2((elmtId, isInitialRender) => {
                    If.create();
                    if (item.content && item.isShowText !== false) {
                      this.ifElseBranchUpdateFunction(0, () => {
                        this.observeComponentCreation2(
                          (elmtId, isInitialRender) => {
                            Text.create(item.content.text);
                            Text.fontColor(
                              this.getEffectiveItemTextColor(item)
                            );
                            Text.fontSize(this.getEffectiveItemFontSize());
                            Text.fontWeight(FontWeight.Regular);
                            Text.maxLines(1);
                            Text.textOverflow({
                              overflow: TextOverflow.Ellipsis,
                            });
                            Text.textAlign(TextAlign.Center);
                          },
                          Text
                        );
                        Text.pop();
                      });
                    } else {
                      this.ifElseBranchUpdateFunction(1, () => {});
                    }
                  }, If);
                  If.pop();
                });
              } else if (item.content) {
                this.ifElseBranchUpdateFunction(1, () => {
                  this.observeComponentCreation2((elmtId, isInitialRender) => {
                    Text.create(item.content.text);
                    Text.fontColor(this.getEffectiveItemTextColor(item));
                    Text.fontSize(this.getEffectiveItemFontSize());
                    Text.fontWeight(FontWeight.Regular);
                    Text.maxLines(1);
                    Text.textOverflow({ overflow: TextOverflow.Ellipsis });
                    Text.textAlign(TextAlign.Center);
                  }, Text);
                  Text.pop();
                });
              } else {
                this.ifElseBranchUpdateFunction(2, () => {});
              }
            }, If);
            If.pop();
            Column.pop();
            Button.pop();
            Button.pop();
          }
        });
      }
    }, If);
    If.pop();
  }

  renderAdvItemGroup(group, parent = null) {
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      If.create();
      if (this.getEffectiveDirection() === Axis.Vertical) {
        this.ifElseBranchUpdateFunction(0, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Column.create();
            Column.width(
              this.toLengthString(this.getEffectiveCrossAxisLength())
            );
            Column.alignItems(HorizontalAlign.Center);
            Column.backgroundColor(this.getEffectiveGroupBg(group));
            Column.borderRadius(this.getEffectiveGroupBorderRadius(group));
            Column.borderWidth(this.getEffectiveGroupBorderWidth(group));
            Column.borderColor(this.getEffectiveGroupBorderColor(group));
            Column.padding(this.getEffectiveGroupPadding(group));
            Column.clip(true);
          }, Column);
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Column.create({ space: this.getEffectiveGroupSpace(group) });
          }, Column);
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            ForEach.create();
            const forEachItemGenFunction = (_item, index) => {
              const item = _item;
              this.renderAdvItem.bind(this)(item, true);
              this.observeComponentCreation2((elmtId, isInitialRender) => {
                If.create();
                if (item.isShowSeparator === true) {
                  this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation2(
                      (elmtId, isInitialRender) => {
                        Divider.create();
                        Divider.width(GROUP_DIVIDER_LENGTH);
                        Divider.color({
                          id: 125831013,
                          type: 10001,
                          params: ['sys.color.comp_divider'],
                          bundleName: '__harDefaultBundleName__',
                          moduleName: '__harDefaultModuleName__',
                        });
                      },
                      Divider
                    );
                  });
                } else {
                  this.ifElseBranchUpdateFunction(1, () => {});
                }
              }, If);
              If.pop();
            };
            this.forEachUpdateFunction(
              elmtId,
              group.items,
              forEachItemGenFunction,
              (item, index) => this.getGroupItemKey(item, index),
              true,
              true
            );
          }, ForEach);
          ForEach.pop();
          Column.pop();
          Column.pop();
        });
      } else {
        this.ifElseBranchUpdateFunction(1, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Row.create();
            Row.height(this.toLengthString(this.getEffectiveCrossAxisLength()));
            Row.alignItems(VerticalAlign.Center);
            Row.backgroundColor(this.getEffectiveGroupBg(group));
            Row.borderRadius(this.getEffectiveGroupBorderRadius(group));
            Row.borderWidth(this.getEffectiveGroupBorderWidth(group));
            Row.borderColor(this.getEffectiveGroupBorderColor(group));
            Row.padding(this.getEffectiveGroupPadding(group));
            Row.clip(true);
          }, Row);
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Row.create({ space: this.getEffectiveGroupSpace(group) });
          }, Row);
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            ForEach.create();
            const forEachItemGenFunction = (_item, index) => {
              const item = _item;
              this.renderAdvItem.bind(this)(item, true);
              this.observeComponentCreation2((elmtId, isInitialRender) => {
                If.create();
                if (item.isShowSeparator === true) {
                  this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation2(
                      (elmtId, isInitialRender) => {
                        Divider.create();
                        Divider.height(GROUP_DIVIDER_LENGTH);
                        Divider.vertical(true);
                        Divider.color({
                          id: 125831013,
                          type: 10001,
                          params: ['sys.color.comp_divider'],
                          bundleName: '__harDefaultBundleName__',
                          moduleName: '__harDefaultModuleName__',
                        });
                      },
                      Divider
                    );
                  });
                } else {
                  this.ifElseBranchUpdateFunction(1, () => {});
                }
              }, If);
              If.pop();
            };
            this.forEachUpdateFunction(
              elmtId,
              group.items,
              forEachItemGenFunction,
              (item, index) => this.getGroupItemKey(item, index),
              true,
              true
            );
          }, ForEach);
          ForEach.pop();
          Row.pop();
          Row.pop();
        });
      }
    }, If);
    If.pop();
  }

  renderAdvSingleItem(item, parent = null) {
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      If.create();
      if (this.getEffectiveDirection() === Axis.Vertical) {
        this.ifElseBranchUpdateFunction(0, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Column.create();
            Column.width(
              this.toLengthString(this.getEffectiveCrossAxisLength())
            );
            Column.alignItems(HorizontalAlign.Center);
            Column.backgroundColor(this.getDefaultItemBg());
            Column.borderRadius(this.getEffectiveBorderRadius());
            Column.borderWidth(this.getEffectiveBorderWidth());
            Column.borderColor(this.getEffectiveBorderColor());
            Column.padding(this.getEffectivePadding());
            Column.clip(true);
          }, Column);
          this.renderAdvItem.bind(this)(item, true);
          Column.pop();
        });
      } else {
        this.ifElseBranchUpdateFunction(1, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Row.create();
            Row.height(this.toLengthString(this.getEffectiveCrossAxisLength()));
            Row.alignItems(VerticalAlign.Center);
            Row.backgroundColor(this.getDefaultItemBg());
            Row.borderRadius(this.getEffectiveBorderRadius());
            Row.borderWidth(this.getEffectiveBorderWidth());
            Row.borderColor(this.getEffectiveBorderColor());
            Row.padding(this.getEffectivePadding());
            Row.clip(true);
          }, Row);
          this.renderAdvItem.bind(this)(item, true);
          Row.pop();
        });
      }
    }, If);
    If.pop();
  }

  renderAdvItemOrGroup(item, parent = null) {
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      If.create();
      if (this.isToolbarItemGroup(item)) {
        this.ifElseBranchUpdateFunction(0, () => {
          this.renderAdvItemGroup.bind(this)(item);
        });
      } else {
        this.ifElseBranchUpdateFunction(1, () => {
          this.renderAdvSingleItem.bind(this)(item);
        });
      }
    }, If);
    If.pop();
  }

  renderAdvTitle(parent = null) {
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      If.create();
      if (this.advTitleConfigs && (this.advTitleConfigs.visible ?? true)) {
        this.ifElseBranchUpdateFunction(0, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Column.create();
            Column.justifyContent(FlexAlign.Center);
            Column.alignItems(HorizontalAlign.Center);
            Column.padding({
              start: LengthMetrics.vp(ITEM_PADDING),
              end: LengthMetrics.vp(ITEM_PADDING),
              top: LengthMetrics.vp(ITEM_PADDING),
              bottom: LengthMetrics.vp(ITEM_PADDING),
            });
          }, Column);
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Text.create(this.advTitleConfigs.content);
            Text.fontSize(this.getEffectiveTitleFontSize());
            Text.fontColor(
              this.advTitleConfigs.style?.fontColor?.color ??
                this.theme.fontPrimaryColor
            );
            Text.fontWeight(this.getEffectiveTitleFontWeight());
            Text.constraintSize({
              minWidth: this.getEffectiveTitleMinLength(),
            });
          }, Text);
          Text.pop();
          Column.pop();
        });
      } else {
        this.ifElseBranchUpdateFunction(1, () => {});
      }
    }, If);
    If.pop();
  }

  advArea(placement, parent = null) {
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      If.create();
      if (this.getEffectiveDirection() === Axis.Vertical) {
        this.ifElseBranchUpdateFunction(0, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Column.create({ space: this.getEffectiveSpacing().value });
            Column.width('100%');
            Column.height('100%');
            Column.justifyContent(
              placement === ToolbarV2ItemPlacement.LEADING
                ? FlexAlign.Start
                : placement === ToolbarV2ItemPlacement.MAIN
                ? FlexAlign.Center
                : FlexAlign.End
            );
            Column.alignItems(HorizontalAlign.Center);
          }, Column);
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            If.create();
            if (
              (this.advTitleConfigs?.placement ??
                ToolbarV2ItemPlacement.LEADING) === placement
            ) {
              this.ifElseBranchUpdateFunction(0, () => {
                this.renderAdvTitle.bind(this)();
              });
            } else {
              this.ifElseBranchUpdateFunction(1, () => {});
            }
          }, If);
          If.pop();
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            ForEach.create();
            const forEachItemGenFunction = (_item, index) => {
              const item = _item;
              this.renderAdvItemOrGroup.bind(this)(item);
            };
            this.forEachUpdateFunction(
              elmtId,
              this.getItemsByPlacement(placement),
              forEachItemGenFunction,
              (item, index) =>
                `toolbar_${placement}_${this.getEntryKey(item, index)}`,
              true,
              true
            );
          }, ForEach);
          ForEach.pop();
          Column.pop();
        });
      } else {
        this.ifElseBranchUpdateFunction(1, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Row.create({ space: this.getEffectiveSpacing().value });
            Row.width('100%');
            Row.height('100%');
            Row.justifyContent(
              placement === ToolbarV2ItemPlacement.LEADING
                ? FlexAlign.Start
                : placement === ToolbarV2ItemPlacement.MAIN
                ? FlexAlign.Center
                : FlexAlign.End
            );
            Row.alignItems(VerticalAlign.Center);
          }, Row);
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            If.create();
            if (
              (this.advTitleConfigs?.placement ??
                ToolbarV2ItemPlacement.LEADING) === placement
            ) {
              this.ifElseBranchUpdateFunction(0, () => {
                this.renderAdvTitle.bind(this)();
              });
            } else {
              this.ifElseBranchUpdateFunction(1, () => {});
            }
          }, If);
          If.pop();
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            ForEach.create();
            const forEachItemGenFunction = (_item, index) => {
              const item = _item;
              this.renderAdvItemOrGroup.bind(this)(item);
            };
            this.forEachUpdateFunction(
              elmtId,
              this.getItemsByPlacement(placement),
              forEachItemGenFunction,
              (item, index) =>
                `toolbar_${placement}_${this.getEntryKey(item, index)}`,
              true,
              true
            );
          }, ForEach);
          ForEach.pop();
          Row.pop();
        });
      }
    }, If);
    If.pop();
  }

  advToolbarContent(parent = null) {
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      Stack.create();
      Stack.width(
        this.toLengthString(
          this.getEffectiveDirection() === Axis.Vertical
            ? this.getEffectiveCrossAxisLength()
            : this.getEffectiveMainAxisLength()
        )
      );
      Stack.height(
        this.toLengthString(
          this.getEffectiveDirection() === Axis.Vertical
            ? this.getEffectiveMainAxisLength()
            : this.getEffectiveCrossAxisLength()
        )
      );
      Stack.clip(true);
      Stack.onSizeChange((_oldValue, newValue) => {
        this.onAdvContentMainAxisSizeChange(newValue);
      });
    }, Stack);
    this.advArea.bind(this)(ToolbarV2ItemPlacement.LEADING);
    this.advArea.bind(this)(ToolbarV2ItemPlacement.TRAILING);
    this.advArea.bind(this)(ToolbarV2ItemPlacement.MAIN);
    Stack.pop();
  }

  getAdvAccessibilityText(item) {
    return item.accessibilityText;
  }

  getAdvAccessibilityDescription(item) {
    return item.accessibilityDescription;
  }

  getFontSizeScale() {
    let context = this.getUIContext();
    let fontScaleSystem = context.getHostContext()?.config?.fontSizeScale ?? 1;
    if (!this.isFollowSystem) {
      return 1;
    } else {
      return Math.min(fontScaleSystem, this.maxFontSizeScale);
    }
  }

  isItemActivating(index) {
    return (
      this.localActivatedIndex === index &&
      this.toolBarList[index]?.state === ToolBarV2ItemState.ACTIVATE
    );
  }

  getToolBarSymbolModifier(index) {
    if (this.isItemActivating(index)) {
      return this.toolBarList[index]?.symbol?.activated;
    }
    return this.toolBarList[index]?.symbol?.normal;
  }

  getIconColor(index) {
    if (this.isItemActivating(index)) {
      return (
        this.toolBarList[index]?.image?.activatedColor?.color ??
        this.theme.iconActivePrimaryColor
      );
    }
    return (
      this.toolBarList[index]?.image?.color?.color ??
      this.theme.iconPrimaryColor
    );
  }

  getTextColor(index) {
    if (this.isItemActivating(index)) {
      return (
        this.toolBarList[index]?.content.activatedColor?.color ??
        this.theme.fontActivatedPrimaryColor
      );
    }
    return (
      this.toolBarList[index]?.content.color?.color ??
      this.theme.fontPrimaryColor
    );
  }

  toLengthString(value) {
    if (value === undefined) {
      return '';
    }
    const length = value.value;
    let lengthString = '';
    switch (value.unit) {
      case LengthUnit.PX:
        lengthString = `${length}px`;
        break;
      case LengthUnit.FP:
        lengthString = `${length}fp`;
        break;
      case LengthUnit.LPX:
        lengthString = `${length}lpx`;
        break;
      case LengthUnit.PERCENT:
        lengthString = `${length * 100}%`;
        break;
      case LengthUnit.VP:
        lengthString = `${length}vp`;
        break;
      default:
        lengthString = `${length}vp`;
        break;
    }
    return lengthString;
  }

  clickEventAction(index) {
    let toolbar = this.toolBarList[index];
    if (toolbar.state === ToolBarV2ItemState.ACTIVATE) {
      if (this.localActivatedIndex === index) {
        this.localActivatedIndex = -1;
      } else {
        this.localActivatedIndex = index;
      }
    }
    if (toolbar.state !== ToolBarV2ItemState.DISABLE) {
      toolbar.action && toolbar.action(index);
    }
  }

  getItemGestureModifier(item, index) {
    if (!item?.icon) {
      return undefined;
    }
    let buttonGestureModifier = new ButtonGestureModifier();
    buttonGestureModifier.gestureCallBack = (event) => {
      if (this.fontSize >= ButtonGestureModifier.minFontSize) {
        event.addGesture(
          new LongPressGestureHandler({
            repeat: false,
            duration: ButtonGestureModifier.longPressTime,
          })
            .onAction(() => {
              promptAction
                .openCustomDialog({
                  builder: () => {
                    this.itemCardDialogBuilder(item, index);
                  },
                  onWillAppear: () => {
                    try {
                      let context = this.getUIContext().getHostContext();
                      let mainWindowStage =
                        context.windowStage.getMainWindowSync();
                      let properties = mainWindowStage.getWindowProperties();
                      if (
                        px2vp(properties.windowRect.height) >
                        SCREEN_WIDTH_BREAK_POINT
                      ) {
                        this.itemCardTextMaxLine =
                          VERTICAL_SCREEN_TEXT_MAX_LINES;
                      } else {
                        this.itemCardTextMaxLine =
                          HORIZONTAL_SCREEN_TEXT_MAX_LINES;
                      }
                    } catch (err) {
                      let code = err?.code;
                      let message = err?.message;
                      hilog.error(
                        0x3900,
                        'Ace',
                        `ToolBarV2 get window height failed, code: ${code}, message: ${message}`
                      );
                    }
                  },
                  maskColor: Color.Transparent,
                  isModal: true,
                  backgroundBlurStyle: BlurStyle.COMPONENT_ULTRA_THICK,
                  backgroundColor: Color.Transparent,
                  shadow: ShadowStyle.OUTER_DEFAULT_LG,
                  cornerRadius: {
                    id: 125830915,
                    type: 10002,
                    params: ['sys.float.corner_radius_level10'],
                    bundleName: '__harDefaultBundleName__',
                    moduleName: '__harDefaultModuleName__',
                  },
                  width:
                    this.fontSize === MAX_FONT_SIZE ? MAX_DIALOG : MIN_DIALOG,
                })
                .then((dialogId) => {
                  this.itemDialogId = dialogId;
                });
            })
            .onActionEnd(() => {
              if (this.itemDialogId) {
                promptAction.closeCustomDialog(this.itemDialogId);
              }
            })
            .onActionCancel(() => {
              if (this.itemDialogId) {
                promptAction.closeCustomDialog(this.itemDialogId);
              }
            })
        );
        return;
      }
      event.clearGestures();
    };
    return buttonGestureModifier;
  }

  isToolbarItemGroup(item) {
    return Array.isArray(item.items);
  }

  buildAdvEntries() {
    for (let k = 0; k < this.advEntries.length; k++) {
      const oldEntry = this.advEntries[k];
      if (this.isToolbarItemGroup(oldEntry)) {
        const group = oldEntry;
        for (let j = 0; j < group.items.length; j++) {
          group.items[j].onStructureChange = undefined;
        }
      } else if (oldEntry instanceof AdvancedToolbarV2Item) {
        oldEntry.onStructureChange = undefined;
      }
    }
    const list = this.toolBarList;
    this.advItemOriginalIndex = new Map();
    const entries = [];
    const groupMap = new Map();
    for (let i = 0; i < list.length; i++) {
      const item = list[i];
      if (!(item instanceof AdvancedToolbarV2Item)) {
        entries.push(item);
        continue;
      }
      this.advItemOriginalIndex.set(item, i);
      if (item.titleConfigs) {
        this.advTitleConfigs = item.titleConfigs;
        continue;
      }
      if (item.groupID) {
        const existing = groupMap.get(item.groupID);
        if (existing) {
          existing.items.push(item);
          if (!existing.style && item.groupStyle) {
            existing.style = item.groupStyle;
            existing.placement = item.groupStyle.placement;
            existing.space = item.groupStyle.space;
          }
        } else {
          const group = new ToolbarV2ItemGroup({
            items: [item],
            placement: item.groupStyle?.placement,
            space: item.groupStyle?.space,
            style: item.groupStyle,
          });
          groupMap.set(item.groupID, group);
          entries.push(group);
        }
      } else {
        entries.push(item);
      }
    }
    this.advEntries = entries;
    for (let k = 0; k < entries.length; k++) {
      const entry = entries[k];
      if (this.isToolbarItemGroup(entry)) {
        const group = entry;
        for (let j = 0; j < group.items.length; j++) {
          group.items[j].onStructureChange = () => this.rebuildAdvState();
        }
      } else if (entry instanceof AdvancedToolbarV2Item) {
        entry.onStructureChange = () => this.rebuildAdvState();
      }
    }
  }

  rebuildAdvState() {
    this.advTitleConfigs = undefined;
    this.buildAdvEntries();
  }

  computeFlatIndex(target) {
    return this.advItemOriginalIndex.get(target) ?? -1;
  }

  setItemContentBuilder(item) {
    this.itemContentBuilder = item.builder;
    return this.itemContentBuilder ? true : false;
  }

  getEffectiveDirection() {
    return this.advancedToolBarModifier?.directionValue ?? Axis.Horizontal;
  }

  getEffectiveSpacing() {
    const raw = this.advancedToolBarModifier?.styleConfigsValue?.groupSpace;
    if (raw === undefined) {
      return LengthMetrics.vp(DEFAULT_GROUP_SPACE);
    }
    const rawValue = Util.lengthMetricsToVp(raw);
    if (rawValue < GROUP_SPACE_MIN) {
      return LengthMetrics.vp(GROUP_SPACE_MIN);
    }
    if (rawValue > GROUP_SPACE_MAX) {
      return LengthMetrics.vp(GROUP_SPACE_MAX);
    }
    return LengthMetrics.vp(rawValue);
  }

  getEffectiveMainAxisLength() {
    return (
      this.advancedToolBarModifier?.mainAxisLengthValue ??
      LengthMetrics.percent(1)
    );
  }

  getEffectiveCrossAxisLength() {
    return (
      this.advancedToolBarModifier?.crossAxisLengthValue ??
      LengthMetrics.vp(DEFAULT_TOOLBAR_HEIGHT)
    );
  }

  getEffectiveToolbarCross() {
    const cross = this.getEffectiveCrossAxisLength();
    return cross ? Util.lengthMetricsToVp(cross) : DEFAULT_TOOLBAR_HEIGHT;
  }

  getHalfCrossAxis() {
    const cross = this.getEffectiveCrossAxisLength();
    const halfValue =
      (cross ? Util.lengthMetricsToVp(cross) : DEFAULT_TOOLBAR_HEIGHT) / 2;
    return `${halfValue}vp`;
  }

  getEffectiveBorderRadius() {
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.borderRadius) {
      return this.toLengthString(style.borderRadius);
    }
    return this.getHalfCrossAxis();
  }

  getEffectiveBorderWidth() {
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.borderWidth) {
      return this.toLengthString(style.borderWidth);
    }
    return 0;
  }

  getEffectiveBorderColor() {
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.borderColor) {
      return style.borderColor.color;
    }
    return Color.Transparent;
  }

  getEffectiveGroupBorderRadius(group) {
    if (group?.style?.borderRadius) {
      return this.toLengthString(group.style.borderRadius);
    }
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.borderRadius) {
      return this.toLengthString(style.borderRadius);
    }
    return this.getHalfCrossAxis();
  }

  getEffectiveGroupBorderWidth(group) {
    if (group?.style?.borderWidth) {
      return this.toLengthString(group.style.borderWidth);
    }
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.borderWidth) {
      return this.toLengthString(style.borderWidth);
    }
    return 0;
  }

  getEffectiveGroupBorderColor(group) {
    if (group?.style?.borderColor) {
      return group.style.borderColor.color;
    }
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.borderColor) {
      return style.borderColor.color;
    }
    return Color.Transparent;
  }

  getEffectiveGroupBg(group) {
    if (group?.style?.backgroundColor) {
      return group.style.backgroundColor.color;
    }
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.backgroundColor) {
      return style.backgroundColor.color;
    }
    return {
      id: 125831004,
      type: 10001,
      params: ['sys.color.comp_background_primary'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    };
  }

  getEffectiveBackgroundColor() {
    return Color.Transparent;
  }

  getEffectiveBackgroundBlurStyle() {
    return (
      this.advancedToolBarModifier?.styleConfigsValue?.backgroundBlurStyle ??
      BlurStyle.NONE
    );
  }

  getEffectiveSystemBackgroundMaterial() {
    return this.advancedToolBarModifier?.styleConfigsValue
      ?.systemBackgroundMaterial;
  }

  getEffectiveBackgroundVisibility() {
    return (
      this.advancedToolBarModifier?.styleConfigsValue?.backgroundVisibility ??
      BarBackgroundVisibility.VISIBLE
    );
  }

  getEffectiveItemHoverBg(item) {
    return {
      id: 125829168,
      type: 10001,
      params: ['sys.color.ohos_id_color_hover'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    };
  }

  getEffectiveItemIconColor(item) {
    if (this.isItemActivated(item)) {
      const useOnPrimary =
        this.isItemOnlyIcon(item) && this.hasActivatedBg(item);
      const defaultColor = useOnPrimary
        ? this.theme.iconOnPrimaryColor
        : this.theme.iconActivePrimaryColor;
      const icon = item.icon;
      if (icon instanceof ToolBarV2SymbolGlyph) {
        return defaultColor;
      }
      return icon?.activatedColor?.color ?? icon?.color?.color ?? defaultColor;
    }
    const icon = item.icon;
    if (icon instanceof ToolBarV2SymbolGlyph) {
      return this.theme.iconPrimaryColor;
    }
    return icon?.color?.color ?? this.theme.iconPrimaryColor;
  }

  getEffectiveItemTextColor(item) {
    if (this.isItemActivated(item)) {
      return (
        item.content?.activatedColor?.color ??
        this.theme.fontActivatedPrimaryColor
      );
    }
    return item.content?.color?.color ?? this.theme.fontSecondaryColor;
  }

  getEffectiveItemFontSize() {
    const raw = this.advancedToolBarModifier?.styleConfigsValue?.fontSize;
    if (raw) {
      return raw.value;
    }
    return {
      id: 125829687,
      type: 10002,
      params: ['sys.float.ohos_id_text_size_caption'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    };
  }

  getEffectiveTitleFontSize() {
    const raw = this.advTitleConfigs?.style?.fontSize;
    if (raw) {
      return Util.lengthMetricsToVp(raw);
    }
    return DEFAULT_TITLE_FONT_SIZE;
  }

  getEffectiveTitleFontWeight() {
    return this.advTitleConfigs?.style?.fontWeight ?? DEFAULT_TITLE_FONT_WEIGHT;
  }

  getEffectiveTitleMinLength() {
    const raw = this.advTitleConfigs?.style?.minLength;
    if (raw) {
      return Util.lengthMetricsToVp(raw);
    }
    return DEFAULT_TITLE_MIN_LENGTH;
  }

  getEffectiveStyleStateEffect() {
    return (
      this.advancedToolBarModifier?.styleConfigsValue?.stateEffect ??
      DEFAULT_STYLE_STATE_EFFECT
    );
  }

  getEffectiveGroupPadding(group) {
    const p = group?.style?.padding;
    const sp = this.advancedToolBarModifier?.styleConfigsValue?.padding;
    const isVertical = this.getEffectiveDirection() === Axis.Vertical;
    const mainStart = p?.mainAxisStart
      ? Util.lengthMetricsToVp(p.mainAxisStart)
      : sp?.mainAxisStart
      ? Util.lengthMetricsToVp(sp.mainAxisStart)
      : GROUP_MAIN_AXIS_PADDING;
    const mainEnd = p?.mainAxisEnd
      ? Util.lengthMetricsToVp(p.mainAxisEnd)
      : sp?.mainAxisEnd
      ? Util.lengthMetricsToVp(sp.mainAxisEnd)
      : GROUP_MAIN_AXIS_PADDING;
    const result = {};
    if (isVertical) {
      result.top = mainStart;
      result.bottom = mainEnd;
      result.left = 0;
      result.right = 0;
    } else {
      result.left = mainStart;
      result.right = mainEnd;
      result.top = 0;
      result.bottom = 0;
    }
    return result;
  }

  getEffectiveItemPressedBg(item, inGroup) {
    if (this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE) {
      return Color.Transparent;
    }
    if (!this.getEffectiveStyleStateEffect()) {
      return Color.Transparent;
    }
    if (inGroup) {
      const groupEffect = this.getGroupStateEffectByItem(item);
      if (groupEffect === false) {
        return Color.Transparent;
      }
    }
    return {
      id: 125829165,
      type: 10001,
      params: ['sys.color.ohos_id_color_click_effect'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    };
  }

  getGroupStateEffectByItem(item) {
    for (let i = 0; i < this.advEntries.length; i++) {
      const entry = this.advEntries[i];
      if (this.isToolbarItemGroup(entry)) {
        const group = entry;
        for (let j = 0; j < group.items.length; j++) {
          if (group.items[j] === item) {
            return group.style?.stateEffect;
          }
        }
      }
    }
    return undefined;
  }

  hasActivatedBg(item) {
    if (!item.activatedBackgroundColor) {
      return false;
    }
    const c = item.activatedBackgroundColor;
    return c.alpha !== 0 || c.red !== 0 || c.green !== 0 || c.blue !== 0;
  }

  getDefaultItemBg() {
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.backgroundColor) {
      return style.backgroundColor.color;
    }
    return {
      id: 125831004,
      type: 10001,
      params: ['sys.color.comp_background_primary'],
      bundleName: '__harDefaultBundleName__',
      moduleName: '__harDefaultModuleName__',
    };
  }

  getEffectiveItemBg(item, inGroup) {
    return inGroup ? Color.Transparent : this.getDefaultItemBg();
  }

  getEffectiveItemActivatedBg(item) {
    if (
      this.isItemActivated(item) &&
      this.isItemOnlyIcon(item) &&
      this.hasActivatedBg(item)
    ) {
      return item.activatedBackgroundColor.color;
    }
    return Color.Transparent;
  }

  getEffectiveItemState(item) {
    const s = item.state;
    if (
      s === ToolBarV2ItemState.ENABLE ||
      s === ToolBarV2ItemState.DISABLE ||
      s === ToolBarV2ItemState.ACTIVATE
    ) {
      return s;
    }
    return ToolBarV2ItemState.ENABLE;
  }

  isItemActivated(item) {
    return this.getEffectiveItemState(item) === ToolBarV2ItemState.ACTIVATE;
  }

  advClickEventAction(item) {
    if (this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE) {
      return;
    }
    const flatIndex = this.computeFlatIndex(item);
    item.action && item.action(flatIndex);
  }

  getEffectiveItemDisplayBg(item, inGroup) {
    if (this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE) {
      return this.getEffectiveItemBg(item, inGroup);
    }
    if (this.advHoveredItems.has(item)) {
      return this.getEffectiveItemHoverBg(item);
    }
    return this.getEffectiveItemBg(item, inGroup);
  }

  getEffectiveItemInnerDisplayBg(item) {
    return this.getEffectiveItemActivatedBg(item);
  }

  setItemHovered(item, isHover) {
    if (this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE) {
      return;
    }
    if (isHover) {
      const next = new Set(this.advHoveredItems);
      next.add(item);
      this.advHoveredItems = next;
    } else {
      const next = new Set(this.advHoveredItems);
      next.delete(item);
      this.advHoveredItems = next;
    }
  }

  getEffectivePadding() {
    const p = this.advancedToolBarModifier?.styleConfigsValue?.padding;
    const isVertical = this.getEffectiveDirection() === Axis.Vertical;
    const mainStart = p?.mainAxisStart
      ? Util.lengthMetricsToVp(p.mainAxisStart)
      : DEFAULT_MAIN_AXIS_PADDING;
    const mainEnd = p?.mainAxisEnd
      ? Util.lengthMetricsToVp(p.mainAxisEnd)
      : DEFAULT_MAIN_AXIS_PADDING;
    const result = {};
    if (isVertical) {
      result.top = mainStart;
      result.bottom = mainEnd;
      result.left = 0;
      result.right = 0;
    } else {
      result.left = mainStart;
      result.right = mainEnd;
      result.top = 0;
      result.bottom = 0;
    }
    return result;
  }

  findGroupByItem(item) {
    for (const entry of this.advEntries) {
      if (this.isToolbarItemGroup(entry)) {
        const group = entry;
        if (group.items && group.items.includes(item)) {
          return group;
        }
      }
    }
    return undefined;
  }

  isItemWithIconAndText(item) {
    if (!item) {
      return false;
    }
    if (item.builder) {
      return false;
    }
    if (item.titleConfigs) {
      return false;
    }
    return (
      !!item.icon &&
      !!item.content &&
      !!item.content.text &&
      item.isShowText !== false
    );
  }

  clampMainAxis(raw, item) {
    if (this.isItemWithIconAndText(item)) {
      return Math.max(
        DEFAULT_ITEM_MAIN_AXIS,
        Math.min(MAX_ITEM_MAIN_AXIS, raw)
      );
    }
    return raw;
  }

  clampCrossAxis(raw) {
    const max = Util.lengthMetricsToVp(this.getEffectiveCrossAxisLength());
    return Math.min(max, Math.max(0, raw));
  }

  getEffectiveItemWidth(item, inGroup = false) {
    const isVertical = this.getEffectiveDirection() === Axis.Vertical;
    const styleItemSize =
      this.advancedToolBarModifier?.styleConfigsValue?.itemSize;
    const effectiveSize = item?.itemSize ?? styleItemSize;
    if (effectiveSize) {
      if (isVertical) {
        return LengthMetrics.vp(
          this.clampCrossAxis(Math.max(0, effectiveSize.width))
        );
      }
      return LengthMetrics.vp(
        this.clampMainAxis(Math.max(0, effectiveSize.width), item)
      );
    }
    if (isVertical) {
      return LengthMetrics.vp(DEFAULT_ITEM_CROSS_AXIS);
    }
    return LengthMetrics.vp(DEFAULT_ITEM_MAIN_AXIS);
  }

  isItemOnlyIcon(item) {
    return (
      !!item?.icon &&
      (!item.content || !item.content.text || item.isShowText === false)
    );
  }

  getEffectiveItemHeight(item, inGroup = false) {
    const isVertical = this.getEffectiveDirection() === Axis.Vertical;
    const styleItemSize =
      this.advancedToolBarModifier?.styleConfigsValue?.itemSize;
    const effectiveSize = item?.itemSize ?? styleItemSize;
    if (effectiveSize) {
      if (isVertical) {
        return LengthMetrics.vp(
          this.clampMainAxis(Math.max(0, effectiveSize.height), item)
        );
      }
      return LengthMetrics.vp(
        this.clampCrossAxis(Math.max(0, effectiveSize.height))
      );
    }
    if (isVertical) {
      return LengthMetrics.vp(DEFAULT_ITEM_MAIN_AXIS);
    }
    return LengthMetrics.vp(DEFAULT_ITEM_CROSS_AXIS);
  }

  getEffectivePlacement(raw) {
    if (
      raw === ToolbarV2ItemPlacement.MAIN ||
      raw === ToolbarV2ItemPlacement.LEADING ||
      raw === ToolbarV2ItemPlacement.TRAILING
    ) {
      return raw;
    }
    return ToolbarV2ItemPlacement.MAIN;
  }

  getItemsByPlacement(placement) {
    const result = [];
    for (let i = 0; i < this.advEntries.length; i++) {
      const entry = this.advEntries[i];
      const entryPlacement = this.getEffectivePlacement(entry.placement);
      if (entryPlacement === placement) {
        result.push(entry);
      }
    }
    return result;
  }

  getEntryKey(item, index) {
    if (this.isToolbarItemGroup(item)) {
      const group = item;
      return `group_${index}_${group.items.length}`;
    }
    const advItem = item;
    return `item_${index}_${advItem.id ?? ''}`;
  }

  getGroupItemKey(item, index) {
    return item.id ?? `group_item_${index}`;
  }

  computeAdvVisibility() {
    if (this.advIsMeasuring) {
      return;
    }
  }

  onAdvContentMainAxisSizeChange(newValue) {
    if (this.getEffectiveDirection() === Axis.Vertical) {
      this.advMainAxisSize = Number(newValue.height) || 0;
    } else {
      this.advMainAxisSize = Number(newValue.width) || 0;
    }
    this.computeAdvVisibility();
  }

  getEffectiveItemBorderRadius(inGroup, item) {
    if (inGroup) {
      if (
        item &&
        this.isItemActivated(item) &&
        this.isItemOnlyIcon(item) &&
        this.hasActivatedBg(item)
      ) {
        const isVertical = this.getEffectiveDirection() === Axis.Vertical;
        const size = isVertical
          ? this.getEffectiveItemWidth(item, true)
          : this.getEffectiveItemHeight(item, true);
        return `${Util.lengthMetricsToVp(size) / 2}vp`;
      }
      return 0;
    }
    const isVertical = this.getEffectiveDirection() === Axis.Vertical;
    const firstLayerSize = isVertical
      ? this.getEffectiveItemWidth(undefined)
      : this.getEffectiveItemHeight(undefined);
    return `${Util.lengthMetricsToVp(firstLayerSize) / 2}vp`;
  }

  getEffectiveItemInnerWidth(item, inGroup) {
    return this.getEffectiveItemWidth(item ?? undefined, inGroup);
  }

  getEffectiveItemInnerHeight(item, inGroup) {
    return this.getEffectiveItemHeight(item ?? undefined, inGroup);
  }

  paddingEdgeVp(edge) {
    if (edge === undefined || edge === null) {
      return 0;
    }
    return Number(edge);
  }

  getEffectiveGroupSpace(group) {
    const raw = group.space
      ? Util.lengthMetricsToVp(group.space)
      : group.style?.space
      ? Util.lengthMetricsToVp(group.style.space)
      : DEFAULT_ITEM_SPACE;
    return Math.max(0, raw);
  }

  initialRender() {
    this.observeComponentCreation2((elmtId, isInitialRender) => {
      If.create();
      if (this.isAdvancedMode) {
        this.ifElseBranchUpdateFunction(0, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Column.create();
          }, Column);
          this.advToolbarContent.bind(this)();
          Column.pop();
        });
      } else {
        this.ifElseBranchUpdateFunction(1, () => {
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Column.create();
            Column.attributeModifier.bind(this)(this.toolBarModifier);
          }, Column);
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Divider.create();
            Divider.width('100%');
            Divider.height(1);
            Divider.attributeModifier.bind(this)(this.dividerModifier);
          }, Divider);
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            Row.create();
            Row.justifyContent(FlexAlign.Center);
            Row.constraintSize({
              minHeight: this.toLengthString(this.toolBarModifier?.heightValue),
              maxHeight: this.toLengthString(this.toolBarModifier?.heightValue),
            });
            Row.width('100%');
            Row.height(this.toLengthString(this.toolBarModifier?.heightValue));
            Row.padding({
              start:
                this.toolBarList.length < TOOLBAR_MAX_LENGTH
                  ? this.toolBarModifier?.paddingValue
                  : LengthMetrics.resource({
                      id: 125830919,
                      type: 10002,
                      params: ['sys.float.padding_level0'],
                      bundleName: '__harDefaultBundleName__',
                      moduleName: '__harDefaultModuleName__',
                    }),
              end:
                this.toolBarList.length < TOOLBAR_MAX_LENGTH
                  ? this.toolBarModifier?.paddingValue
                  : LengthMetrics.resource({
                      id: 125830919,
                      type: 10002,
                      params: ['sys.float.padding_level0'],
                      bundleName: '__harDefaultBundleName__',
                      moduleName: '__harDefaultModuleName__',
                    }),
            });
          }, Row);
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            ForEach.create();
            const forEachItemGenFunction = (_item, index) => {
              const item = _item;
              this.observeComponentCreation2((elmtId, isInitialRender) => {
                If.create();
                if (
                  this.toolBarList.length <= TOOLBAR_MAX_LENGTH ||
                  index < TOOLBAR_MAX_LENGTH - 1
                ) {
                  this.ifElseBranchUpdateFunction(0, () => {
                    this.observeComponentCreation2(
                      (elmtId, isInitialRender) => {
                        Row.create();
                        Row.height('100%');
                        Row.flexShrink(1);
                      },
                      Row
                    );
                    this.TabBuilder.bind(this)(index);
                    Row.pop();
                  });
                } else {
                  this.ifElseBranchUpdateFunction(1, () => {});
                }
              }, If);
              If.pop();
            };
            this.forEachUpdateFunction(
              elmtId,
              this.toolBarList,
              forEachItemGenFunction,
              (item, index) => {
                return `${this.getUniqueId}__${index}}`;
              },
              true,
              true
            );
          }, ForEach);
          ForEach.pop();
          this.observeComponentCreation2((elmtId, isInitialRender) => {
            If.create();
            if (this.toolBarList.length > TOOLBAR_MAX_LENGTH) {
              this.ifElseBranchUpdateFunction(0, () => {
                this.observeComponentCreation2((elmtId, isInitialRender) => {
                  Row.create();
                  Row.height('100%');
                  Row.flexShrink(1);
                }, Row);
                this.MoreTabBuilder.bind(this)(TOOLBAR_MAX_LENGTH - 1);
                Row.pop();
              });
            } else {
              this.ifElseBranchUpdateFunction(1, () => {});
            }
          }, If);
          If.pop();
          Row.pop();
          Column.pop();
        });
      }
    }, If);
    If.pop();
  }

  updateStateVars(params) {
    if (params === undefined) {
      return;
    }
    if ('toolBarList' in params) {
      this.updateParam('toolBarList', params.toolBarList);
    }
    if ('activatedIndex' in params) {
      this.updateParam('activatedIndex', params.activatedIndex);
    }
    if ('dividerModifier' in params) {
      this.updateParam('dividerModifier', params.dividerModifier);
    }
    if ('toolBarModifier' in params) {
      this.updateParam('toolBarModifier', params.toolBarModifier);
    }
    if ('advancedToolBarModifier' in params) {
      this.updateParam(
        'advancedToolBarModifier',
        params.advancedToolBarModifier
      );
    }
  }

  rerender() {
    this.updateDirtyElements();
  }
}
__decorate([Param], ToolBarV2.prototype, 'toolBarList', void 0);
__decorate([Param], ToolBarV2.prototype, 'activatedIndex', void 0);
__decorate([Param], ToolBarV2.prototype, 'dividerModifier', void 0);
__decorate([Param], ToolBarV2.prototype, 'toolBarModifier', void 0);
__decorate([Param], ToolBarV2.prototype, 'advancedToolBarModifier', void 0);
__decorate([Local], ToolBarV2.prototype, 'localActivatedIndex', void 0);
__decorate([Local], ToolBarV2.prototype, 'menuContent', void 0);
__decorate([Local], ToolBarV2.prototype, 'fontSize', void 0);
__decorate([Local], ToolBarV2.prototype, 'theme', void 0);
__decorate([Local], ToolBarV2.prototype, 'isAdvancedMode', void 0);
__decorate([Local], ToolBarV2.prototype, 'advMainAxisSize', void 0);
__decorate([Local], ToolBarV2.prototype, 'advIsMeasuring', void 0);
__decorate([Local], ToolBarV2.prototype, 'advHoveredItems', void 0);
__decorate([Local], ToolBarV2.prototype, 'advTitleConfigs', void 0);
__decorate([Local], ToolBarV2.prototype, 'advEntries', void 0);
__decorate([Computed], ToolBarV2.prototype, 'menus', null);
__decorate(
  [Monitor('activatedIndex')],
  ToolBarV2.prototype,
  'onActivateIndexChange',
  null
);
__decorate(
  [Monitor('toolBarList')],
  ToolBarV2.prototype,
  'onToolBarListChange',
  null
);

export default {
  ToolBarV2Item,
  ToolBarV2ItemImage,
  ToolBarV2ItemText,
  ToolBarV2,
  ToolBarV2ItemState,
  ToolBarV2Modifier,
  ToolBarV2SymbolGlyph,
  ToolbarV2ItemPlacement,
  AdvancedToolbarV2Item,
  BarBackgroundVisibility,
  ToolbarV2TitleStyle,
  ToolbarV2TitleConfigs,
  ToolbarV2ItemGroupStyle,
  ToolbarV2StyleConfigs,
  AdvancedToolBarV2Modifier,
};
