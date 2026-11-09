/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

import { Theme } from '@ohos.arkui.theme';
import { LengthMetrics, LengthUnit, ColorMetrics } from '@ohos.arkui.node';
import { DividerModifier, SymbolGlyphModifier } from '@ohos.arkui.modifier';
import hilog from '@ohos.hilog';
import window from '@ohos.window';
import common from '@ohos.app.ability.common';
import { BusinessError } from '@ohos.base';
import promptAction from '@ohos.promptAction';
import uiMaterial from '@ohos.arkui.uiMaterial';

export enum ToolBarV2ItemState {
  ENABLE = 1,
  DISABLE = 2,
  ACTIVATE = 3,
}

export enum ToolbarV2ItemPlacement {
  MAIN = 0,
  LEADING = 1,
  TRAILING = 2,
}

export enum ToolbarV2MaterialStyle {
  NONE = 0,
  DEFAULT = 1,
}

// “更多”栏图标
const PUBLIC_MORE: Resource = $r('sys.symbol.dot_grid_2x2');
const IMAGE_SIZE: VP = '24vp';
const DEFAULT_TOOLBAR_HEIGHT: number = 56;
const TOOLBAR_MAX_LENGTH: number = 5;
const MAX_FONT_SIZE: number = 3.2;
const DIALOG_IMAGE_SIZE: VP = '64vp';
const MAX_DIALOG: VP = '256vp';
const MIN_DIALOG: VP = '216vp';
const TEXT_TOOLBAR_DIALOG: FP = '18.3fp';
const SCREEN_WIDTH_BREAK_POINT: number = 640;
const VERTICAL_SCREEN_TEXT_MAX_LINES: number = 6;
const HORIZONTAL_SCREEN_TEXT_MAX_LINES: number = 1;
const FOCUS_BOX_MARGIN: number = -2;
const FOCUS_BOX_BORDER_WIDTH: number = 2;
const RESOURCE_TYPE_SYMBOL: number = 40000;
const DEFAULT_ITEM_MAIN_AXIS: number = 56;
const MAX_ITEM_MAIN_AXIS: number = 76;
const MIN_BUTTON_WIDTH: number = 36;
const ITEM_PADDING: number = 4;
const ICON_TEXT_MARGIN: number = 2;
const ITEM_MARGIN: number = 0;
const PRESSED_AREA_SIZE: number = 48;
const PRESSED_AREA_RADIUS: number = 24;
const HOVER_BORDER_RADIUS: number = 26;
const DEFAULT_TITLE_FONT_SIZE: number = 20;
const DEFAULT_TITLE_MIN_LENGTH: number = 48;
const DEFAULT_TITLE_FONT_WEIGHT: FontWeight = FontWeight.Bold;
const DEFAULT_ITEM_SPACE: number = 0;
const GROUP_SPACE_MIN: number = 8;
const GROUP_SPACE_MAX: number = 20;
const DEFAULT_GROUP_SPACE: number = 12;
const DEFAULT_ZONE_GAP: number = 24;
const DISPLAY_PRIORITY_MIN: number = 0;
const DISPLAY_PRIORITY_MAX: number = 10;
const DEFAULT_ITEM_CROSS_AXIS: number = 48;
const DEFAULT_MAIN_AXIS_PADDING: number = 4;
const GROUP_MAIN_AXIS_PADDING: number = 12;
const DEFAULT_STYLE_STATE_EFFECT: boolean = true;
const GROUP_DIVIDER_LENGTH: number = 18;
const DISPLAY_PRIORITY_DEFAULT: number = 1;
const ADV_FIT_EPSILON: number = 0.5;
const OVERFLOW_MENU_ITEM_HEIGHT: number = 48;
const OVERFLOW_MENU_PADDING_VERTICAL: number = 4;
const DEFAULT_OVERFLOW_MENU_RADIUS: number = 16;

interface MenuController {
  value: ResourceStr;
  action: () => void;
  enabled?: boolean;
}

class Util {
  public static isSymbolResource(resourceStr: ResourceStr | undefined | null): boolean {
    if (!Util.isResourceType(resourceStr)) {
      return false;
    }
    let resource: Resource = resourceStr as Resource;
    return resource.type === RESOURCE_TYPE_SYMBOL;
  }

  public static isResourceType(resource: ResourceStr | Resource | undefined | null): boolean {
    if (!resource) {
      return false;
    }
    if (typeof resource === 'string' || typeof resource === 'undefined') {
      return false;
    }
    return true;
  }

  public static lengthMetricsToVp(lengthMetrics: LengthMetrics): number {
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

@ObservedV2
export class ToolBarV2SymbolGlyph {
  @Trace
  public normal: SymbolGlyphModifier;
  @Trace
  public activated?: SymbolGlyphModifier;

  constructor(options: ToolBarSymbolGlyphOptions) {
    this.normal = options.normal;
    this.activated = options.activated;
  }
}

export interface ToolBarSymbolGlyphOptions {
  normal: SymbolGlyphModifier;
  activated?: SymbolGlyphModifier;
}

class ButtonGestureModifier implements GestureModifier {
  public static readonly longPressTime: number = 500;
  public static readonly minFontSize: number = 1.75;
  public gestureCallBack?: (event: UIGestureEvent) => void = undefined;

  applyGesture(event: UIGestureEvent): void {
    this.gestureCallBack?.(event);
  }
}

@ObservedV2
export class ToolBarV2ItemText {
  @Trace
  public text: ResourceStr;
  @Trace
  public color?: ColorMetrics = ColorMetrics.resourceColor($r('sys.color.font_primary'));
  @Trace
  public activatedColor?: ColorMetrics = ColorMetrics.resourceColor($r('sys.color.font_emphasize'));

  constructor(options: ToolBarV2ItemTextOptions) {
    this.text = options.text;
    this.color = options.color;
    this.activatedColor = options.activatedColor;
  }
}

export interface ToolBarV2ItemTextOptions {
  text: ResourceStr;
  color?: ColorMetrics;
  activatedColor?: ColorMetrics;
}

@ObservedV2
export class ToolBarV2ItemImage {
  @Trace
  public src: ResourceStr;
  @Trace
  public color?: ColorMetrics = undefined;
  @Trace
  public activatedColor?: ColorMetrics = undefined;

  constructor(options: ToolBarV2ItemImageOptions) {
    this.src = options.src;
    this.color = options.color;
    this.activatedColor = options.activatedColor;
  }
}

export declare type ToolBarV2ItemIconType = ToolBarV2ItemImage | ToolBarV2SymbolGlyph;

export interface ToolBarV2ItemImageOptions {
  src: ResourceStr;
  color?: ColorMetrics;
  activatedColor?: ColorMetrics;
}

export type ToolBarV2ItemAction = (index: number) => void;

@ObservedV2
export class ToolBarV2Item {
  @Trace
  public content: ToolBarV2ItemText = new ToolBarV2ItemText({ text: '' });
  @Trace
  public action?: (index: number) => void = undefined;
  @Trace
  public icon?: ToolBarV2ItemIconType = undefined;
  @Trace
  public state?: ToolBarV2ItemState = 1;
  @Trace
  public accessibilityText?: ResourceStr = '';
  @Trace
  public accessibilityDescription?: ResourceStr = '';
  @Trace
  public accessibilityLevel?: string = 'auto';
  // item background, not exported
  @Trace
  public backgroundColor: ResourceColor = Color.Transparent;

  constructor(options: ToolBarV2ItemOptions) {
    this.content = options.content;
    this.action = options.action;
    this.icon = options.icon;
    this.state = options.state;
    this.accessibilityText = options.accessibilityText;
    this.accessibilityDescription = options.accessibilityDescription;
    this.accessibilityLevel = options.accessibilityLevel;
  }

  @Computed
  get symbol(): ToolBarV2SymbolGlyph | undefined {
    if (this.icon instanceof ToolBarV2SymbolGlyph) {
      return this.icon;
    }
    return undefined;
  }

  @Computed
  get image(): ToolBarV2ItemImage | undefined {
    if (!(this.icon instanceof ToolBarV2SymbolGlyph)) {
      return this.icon;
    }
    return undefined;
  }
}

export interface ToolBarV2ItemOptions {
  content: ToolBarV2ItemText;
  action?: (index: number) => void;
  icon?: ToolBarV2ItemIconType;
  state?: ToolBarV2ItemState;
  accessibilityText?: ResourceStr;
  accessibilityDescription?: ResourceStr;
  accessibilityLevel?: string;
}

export interface ToolbarV2PaddingOptions {
  mainAxisStart?: LengthMetrics;
  mainAxisEnd?: LengthMetrics;
}

@ObservedV2
export class ToolbarV2ItemGroupStyle {
  @Trace
  public placement?: ToolbarV2ItemPlacement;
  @Trace
  public space?: LengthMetrics;
  @Trace
  public backgroundColor?: ColorMetrics;
  @Trace
  public padding?: ToolbarV2PaddingOptions;
  @Trace
  public borderRadius?: LengthMetrics;
  @Trace
  public borderWidth?: LengthMetrics;
  @Trace
  public borderColor?: ColorMetrics;
  @Trace
  public stateEffect?: boolean;

  constructor(init?: ToolbarV2ItemGroupStyleOptions) {
    this.placement = init?.placement;
    this.space = init?.space;
    this.backgroundColor = init?.backgroundColor;
    this.padding = init?.padding;
    this.borderRadius = init?.borderRadius;
    this.borderWidth = init?.borderWidth;
    this.borderColor = init?.borderColor;
    this.stateEffect = init?.stateEffect;
  }
}

export interface ToolbarV2ItemGroupStyleOptions {
  placement?: ToolbarV2ItemPlacement;
  space?: LengthMetrics;
  backgroundColor?: ColorMetrics;
  padding?: ToolbarV2PaddingOptions;
  borderRadius?: LengthMetrics;
  borderWidth?: LengthMetrics;
  borderColor?: ColorMetrics;
  stateEffect?: boolean;
}

interface ToolbarV2ItemGroupOptions {
  placement?: ToolbarV2ItemPlacement;
  items?: AdvancedToolbarV2Item[];
  space?: LengthMetrics;
  style?: ToolbarV2ItemGroupStyle;
}

@ObservedV2
class ToolbarV2ItemGroup {
  @Trace
  public placement?: ToolbarV2ItemPlacement;
  @Trace
  public items: AdvancedToolbarV2Item[];
  @Trace
  public space?: LengthMetrics;
  @Trace
  public style?: ToolbarV2ItemGroupStyle;

  constructor(options?: ToolbarV2ItemGroupOptions) {
    this.placement = options?.placement;
    this.items = options?.items ?? [];
    this.space = options?.space;
    this.style = options?.style;
  }
}

@ObservedV2
export class AdvancedToolbarV2Item extends ToolBarV2Item {
  @Trace
  public builder?: CustomBuilder;
  @Trace
  public titleConfigs?: ToolbarV2TitleConfigs;
  @Trace
  public placement?: ToolbarV2ItemPlacement;
  @Trace
  public displayPriority?: number;
  @Trace
  public activatedBackgroundColor?: ColorMetrics;
  @Trace
  public id?: string;
  @Trace
  public itemSize?: Size;
  @Trace
  public isShowSeparator?: boolean;
  @Trace
  public isShowText?: boolean;
  @Trace
  public isShowOverflowMenu?: boolean;
  @Trace
  public groupID?: string;
  @Trace
  public groupStyle?: ToolbarV2ItemGroupStyle;
  public onStructureChange?: () => void = undefined;

  constructor(options: AdvancedToolbarV2ItemOptions) {
    super({
      content: options.content,
      action: options.action,
      icon: options.icon,
      state: options.state,
      accessibilityText: options.accessibilityText,
      accessibilityDescription: options.accessibilityDescription,
      accessibilityLevel: options.accessibilityLevel,
    });
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

  @Monitor('groupID')
  onGroupIDChange(monitor: IMonitor): void {
    this.onStructureChange?.();
  }

  @Monitor('groupStyle')
  onGroupStyleChange(monitor: IMonitor): void {
    this.onStructureChange?.();
  }

  @Monitor('titleConfigs')
  onTitleConfigsChange(monitor: IMonitor): void {
    this.onStructureChange?.();
  }

  @Monitor('displayPriority')
  onDisplayPriorityChange(monitor: IMonitor): void {
    this.onStructureChange?.();
  }
}

export interface AdvancedToolbarV2ItemOptions extends ToolBarV2ItemOptions {
  builder?: CustomBuilder;
  titleConfigs?: ToolbarV2TitleConfigs;
  placement?: ToolbarV2ItemPlacement;
  displayPriority?: number;
  activatedBackgroundColor?: ColorMetrics;
  id?: string;
  itemSize?: Size;
  isShowSeparator?: boolean;
  isShowText?: boolean;
  isShowOverflowMenu?: boolean;
  groupID?: string;
  groupStyle?: ToolbarV2ItemGroupStyle;
}

export interface ToolbarV2OverflowMenuStyle {
  font?: Font;
  fontColor?: ColorMetrics;
  radius?: Dimension | BorderRadiuses;
  menuItemDivider?: DividerStyleOptions;
  menuItemGroupDivider?: DividerStyleOptions;
}

@ObservedV2
export class ToolbarV2TitleStyle {
  @Trace
  public fontSize?: LengthMetrics;
  @Trace
  public fontColor?: ColorMetrics;
  @Trace
  public fontWeight?: FontWeight;
  @Trace
  public minLength?: LengthMetrics;

  constructor(init?: ToolbarV2TitleStyleOptions) {
    this.fontSize = init?.fontSize;
    this.fontColor = init?.fontColor;
    this.fontWeight = init?.fontWeight;
    this.minLength = init?.minLength;
  }
}

export interface ToolbarV2TitleStyleOptions {
  fontSize?: LengthMetrics;
  fontColor?: ColorMetrics;
  fontWeight?: FontWeight;
  minLength?: LengthMetrics;
}

@ObservedV2
export class ToolbarV2TitleConfigs {
  @Trace
  public content: ResourceStr;
  @Trace
  public placement?: ToolbarV2ItemPlacement;
  @Trace
  public visible?: boolean;
  @Trace
  public style?: ToolbarV2TitleStyle;

  constructor(init?: ToolbarV2TitleOptions) {
    this.content = init?.content ?? '';
    this.placement = init?.placement;
    this.visible = init?.visible;
    this.style = init?.style;
  }
}

export interface ToolbarV2TitleOptions {
  content?: ResourceStr;
  placement?: ToolbarV2ItemPlacement;
  visible?: boolean;
  style?: ToolbarV2TitleStyle;
}

@ObservedV2
export class ToolbarV2StyleConfigs {
  @Trace
  public padding?: ToolbarV2PaddingOptions;
  @Trace
  public backgroundColor?: ColorMetrics;
  @Trace
  public backgroundBlurStyle?: BlurStyle;
  @Trace
  public systemBackgroundMaterial?: uiMaterial.Material;
  @Trace
  public systemMaterialStyle?: ToolbarV2MaterialStyle = ToolbarV2MaterialStyle.NONE;
  @Trace
  public groupSpace?: LengthMetrics;
  @Trace
  public itemSize?: Size;
  @Trace
  public fontSize?: LengthMetrics;
  @Trace
  public borderWidth?: LengthMetrics;
  @Trace
  public borderColor?: ColorMetrics;
  @Trace
  public borderRadius?: LengthMetrics;
  @Trace
  public stateEffect?: boolean;

  constructor(init?: ToolbarV2StyleOptions) {
    this.padding = init?.padding;
    this.backgroundColor = init?.backgroundColor;
    this.backgroundBlurStyle = init?.backgroundBlurStyle;
    this.systemBackgroundMaterial = init?.systemBackgroundMaterial;
    this.systemMaterialStyle = init?.systemMaterialStyle ?? ToolbarV2MaterialStyle.NONE;
    this.groupSpace = init?.groupSpace;
    this.itemSize = init?.itemSize;
    this.fontSize = init?.fontSize;
    this.borderWidth = init?.borderWidth;
    this.borderColor = init?.borderColor;
    this.borderRadius = init?.borderRadius;
    this.stateEffect = init?.stateEffect;
  }
}

export interface ToolbarV2StyleOptions {
  padding?: ToolbarV2PaddingOptions;
  backgroundColor?: ColorMetrics;
  backgroundBlurStyle?: BlurStyle;
  systemBackgroundMaterial?: uiMaterial.Material;
  systemMaterialStyle?: ToolbarV2MaterialStyle;
  groupSpace?: LengthMetrics;
  itemSize?: Size;
  fontSize?: LengthMetrics;
  borderWidth?: LengthMetrics;
  borderColor?: ColorMetrics;
  borderRadius?: LengthMetrics;
  stateEffect?: boolean;
}

@ObservedV2
export class AdvancedToolBarV2Modifier {
  @Trace
  public mainAxisLengthValue?: LengthMetrics = LengthMetrics.percent(1);
  @Trace
  public crossAxisLengthValue?: LengthMetrics;
  @Trace
  public directionValue: Axis = Axis.Horizontal;
  @Trace
  public styleConfigsValue?: ToolbarV2StyleConfigs;
  @Trace
  public overflowMenuStyleValue?: ToolbarV2OverflowMenuStyle;

  mainAxisLength(value: LengthMetrics): AdvancedToolBarV2Modifier {
    this.mainAxisLengthValue = value;
    return this;
  }

  crossAxisLength(value: LengthMetrics): AdvancedToolBarV2Modifier {
    this.crossAxisLengthValue = value;
    return this;
  }

  direction(value: Axis): AdvancedToolBarV2Modifier {
    this.directionValue = value;
    return this;
  }

  style(styleConfigs: ToolbarV2StyleConfigs): AdvancedToolBarV2Modifier {
    this.styleConfigsValue = styleConfigs;
    return this;
  }

  overflowMenuStyle(menuStyle: ToolbarV2OverflowMenuStyle): AdvancedToolBarV2Modifier {
    this.overflowMenuStyleValue = menuStyle;
    return this;
  }
}

@ObservedV2
export class ToolBarV2Modifier implements AttributeModifier<ColumnAttribute> {
  @Trace
  public backgroundColorValue?: ResourceColor = $r('sys.color.ohos_id_color_toolbar_bg');
  @Trace
  public heightValue?: LengthMetrics = LengthMetrics.vp(DEFAULT_TOOLBAR_HEIGHT);
  @Trace
  public stateEffectValue?: boolean = true;
  @Trace
  public paddingValue?: LengthMetrics = LengthMetrics.resource($r('sys.float.padding_level12'));

  applyNormalAttribute(instance: ColumnAttribute): void {
    instance.backgroundColor(this.backgroundColorValue ?? $r('sys.color.ohos_id_color_toolbar_bg'));
  }

  public backgroundColor(backgroundColor: ColorMetrics): ToolBarV2Modifier {
    this.backgroundColorValue = backgroundColor.color;
    return this;
  }

  public height(height: LengthMetrics): ToolBarV2Modifier {
    this.heightValue = height;
    return this;
  }

  public stateEffect(stateEffect: boolean): ToolBarV2Modifier {
    this.stateEffectValue = stateEffect;
    return this;
  }

  public padding(padding: LengthMetrics): ToolBarV2Modifier {
    this.paddingValue = padding;
    return this;
  }
}

@ObservedV2
class ToolBarV2Theme {
  @Trace
  public iconPrimaryColor: ResourceColor = $r('sys.color.icon_primary');
  @Trace
  public iconActivePrimaryColor: ResourceColor = $r('sys.color.icon_emphasize');
  @Trace
  public iconOnPrimaryColor: ResourceColor = $r('sys.color.icon_on_primary');
  @Trace
  public fontPrimaryColor: ResourceColor = $r('sys.color.font_primary');
  @Trace
  public fontActivatedPrimaryColor: ResourceColor = $r('sys.color.font_emphasize');
  @Trace
  public fontSecondaryColor: ResourceColor = $r('sys.color.font_secondary');
}

interface AdvPriorityEntry {
  item: AdvancedToolbarV2Item;
  index: number;
  priority: number;
}

interface AdvZoneMetrics {
  leading: number;
  main: number;
  trailing: number;
  titleTextWidth: number;
}

@ComponentV2
export struct ToolBarV2 {
  @Require @Param
  toolBarList: ToolBarV2Item[];
  @Param
  activatedIndex?: number = -1;
  @Param
  dividerModifier?: DividerModifier = new DividerModifier();
  @Param
  toolBarModifier?: ToolBarV2Modifier =
    new ToolBarV2Modifier()
      .padding(LengthMetrics.resource($r('sys.float.padding_level12')))
      .stateEffect(true)
      .height(LengthMetrics.vp(DEFAULT_TOOLBAR_HEIGHT));
  @Param
  advancedToolBarModifier?: AdvancedToolBarV2Modifier = undefined;
  @Local
  localActivatedIndex: number = -1;
  @Local
  menuContent: MenuController[] = [];
  @Local
  fontSize: number = 1;
  @Local
  theme: ToolBarV2Theme = new ToolBarV2Theme();
  @Local
  isAdvancedMode: boolean = false;
  @Local
  advMainAxisSize: number = 0;
  @Local
  advIsMeasuring: boolean = false;
  @Local
  advHoveredItems: Set<AdvancedToolbarV2Item> = new Set();
  @Local
  advTitleConfigs: ToolbarV2TitleConfigs | undefined = undefined;
  @BuilderParam
  itemContentBuilder?: CustomBuilder;
  @Local
  advEntries: Array<AdvancedToolbarV2Item | ToolbarV2ItemGroup> = [];
  @Local
  advHiddenItems: AdvancedToolbarV2Item[] = [];
  @Local
  advVisibilityRevision: number = 0;
  @Local
  advTitleRenderWidth: number = 0;
  @Local
  advMainFlowOffset: number = 0;
  @Local
  advMainClamped: boolean = false;
  private advTitleNaturalWidth: number = 0;
  private advIsComputingVisibility: boolean = false;
  private advTitleSignature: string = '';
  private advItemOriginalIndex: Map<AdvancedToolbarV2Item, number> = new Map();
  private itemCardTextMaxLine: number = 1;
  private itemDialogId?: number = undefined;
  private isFollowSystem: boolean = false;
  private maxFontSizeScale: number = 3.2;
  private moreItem: ToolBarV2Item = new ToolBarV2Item({
    content: new ToolBarV2ItemText({
      text: $r('sys.string.ohos_toolbar_more'),
    }),
    icon: new ToolBarV2ItemImage({
      src: PUBLIC_MORE
    })
  })
  private moreText: Resource = $r('sys.string.ohos_toolbar_more');


  @Computed
  get menus(): MenuController[] {
    this.menuContent = [];
    this.toolBarList.forEach((value: ToolBarV2Item, index: number) => {
      if (index >= TOOLBAR_MAX_LENGTH - 1) {
        this.menuContent.push({
          value: this.toolBarList[index].content.text,
          action: () => {
            let callback: ToolBarV2ItemAction | undefined = this.toolBarList[index].action;
            if (callback) {
              callback(index);
            }
          },
          enabled: this.toolBarList[index].state !== ToolBarV2ItemState.DISABLE,
        });
      }
    })
    return this.menuContent;
  }

  @Monitor('activatedIndex')
  onActivateIndexChange(monitor: IMonitor) {
    this.localActivatedIndex = monitor.value<number>('activatedIndex')?.now ?? -1;
  }

  @Monitor('toolBarList')
  onToolBarListChange(monitor: IMonitor): void {
    if (this.isAdvancedMode) {
      this.rebuildAdvState();
    }
  }

  aboutToAppear(): void {
    this.localActivatedIndex = this.activatedIndex ?? -1;
    this.isAdvancedMode = this.advancedToolBarModifier !== undefined;
    try {
      this.isFollowSystem = this.getUIContext()?.isFollowingSystemFontScale();
      this.maxFontSizeScale = this.getUIContext()?.getMaxFontScale();
    } catch (err) {
      let code: number = (err as BusinessError)?.code;
      let message: string = (err as BusinessError)?.message;
      hilog.error(0x3900, 'Ace', `Faild to toolBarV2 getMaxFontScale, code: ${code}, message: ${message}`);
    }
    if (this.isAdvancedMode) {
      this.rebuildAdvState();
    }
  }

  onWillApplyTheme(theme: Theme): void {
    this.theme.iconPrimaryColor = theme.colors.iconPrimary;
    this.theme.iconActivePrimaryColor = theme.colors.iconEmphasize;
    this.theme.iconOnPrimaryColor = theme.colors.iconOnPrimary;
    this.theme.fontPrimaryColor = theme.colors.fontPrimary;
    this.theme.fontActivatedPrimaryColor = theme.colors.fontEmphasize;
    this.theme.fontSecondaryColor = theme.colors.fontSecondary;
  }

  @Builder
  MoreTabBuilder(index: number): void {
    Button({ type: ButtonType.Normal, stateEffect: false }) {
      Column() {
        SymbolGlyph(PUBLIC_MORE)
          .fontSize(IMAGE_SIZE)
          .fontColor([this.theme.iconPrimaryColor])
          .draggable(false)
          .margin({ bottom: $r('sys.float.padding_level1') })
        Text(this.moreText)
          .fontColor(this.theme.fontPrimaryColor)
          .fontSize($r('sys.float.ohos_id_text_size_caption'))
          .fontWeight(FontWeight.Medium)
          .maxLines(1)
          .textOverflow({ overflow: TextOverflow.Ellipsis })
          .textAlign(TextAlign.Center)
          .focusable(true)
          .focusOnTouch(true)
      }
      .width('100%')
      .height('100%')
      .justifyContent(FlexAlign.Center)
      .padding({
        start: LengthMetrics.resource($r('sys.float.padding_level2')),
        end: LengthMetrics.resource($r('sys.float.padding_level2')),
      })
      .borderRadius($r('sys.float.ohos_id_corner_radius_clicked'))
    }
    .accessibilityGroup(true)
    .focusable(true)
    .focusOnTouch(true)
    .focusBox({
      margin: LengthMetrics.vp(FOCUS_BOX_MARGIN),
      strokeWidth: LengthMetrics.vp(FOCUS_BOX_BORDER_WIDTH),
      strokeColor: ColorMetrics.resourceColor($r('sys.color.ohos_id_color_focused_outline'))
    })
    .width('100%')
    .height('100%')
    .bindMenu(this.menuContent, { placement: Placement.TopRight, offset: { x: -12, y: -10 } })
    .borderRadius($r('sys.float.ohos_id_corner_radius_clicked'))
    .backgroundColor(this.toolBarList[index].backgroundColor)
    .onHover((isHover: boolean) => {
      if (isHover) {
        this.toolBarList[index].backgroundColor = $r('sys.color.ohos_id_color_hover');
      } else {
        this.toolBarList[index].backgroundColor = Color.Transparent;
      }
    })
    .stateStyles({
      pressed: {
        .backgroundColor((!this.toolBarModifier?.stateEffectValue) ?
          this.toolBarList[index].backgroundColor : $r('sys.color.ohos_id_color_click_effect'))
      }
    })
    .gestureModifier(this.getItemGestureModifier(this.moreItem, index))
  }

  @Builder
  TabBuilder(index: number): void {
    Button({ type: ButtonType.Normal, stateEffect: false }) {
      Column() {
        if (this.toolBarList[index]?.symbol) {
          SymbolGlyph()
            .fontSize(IMAGE_SIZE)
            .symbolEffect(new SymbolEffect(), false)
            .attributeModifier(this.getToolBarSymbolModifier(index))
            .margin({ bottom: $r('sys.float.padding_level1') })
        } else if (Util.isSymbolResource(this.toolBarList[index]?.image?.src)) {
          SymbolGlyph(this.toolBarList[index]?.image?.src as Resource)
            .fontSize(IMAGE_SIZE)
            .fontColor([this.getIconColor(index)])
            .margin({ bottom: $r('sys.float.padding_level1') })
        } else {
          Image(this.toolBarList[index]?.image?.src)
            .width(IMAGE_SIZE)
            .height(IMAGE_SIZE)
            .fillColor(this.getIconColor(index))
            .margin({ bottom: $r('sys.float.padding_level1') })
            .objectFit(ImageFit.Contain)
            .draggable(false)
        }
        Text(this.toolBarList[index]?.content.text)
          .fontColor(this.getTextColor(index))
          .fontSize($r('sys.float.ohos_id_text_size_caption'))
          .maxFontSize($r('sys.float.ohos_id_text_size_caption'))
          .minFontSize(9)
          .fontWeight(FontWeight.Medium)
          .maxLines(1)
          .textOverflow({ overflow: TextOverflow.Ellipsis })
          .textAlign(TextAlign.Center)
          .focusable(!(this.toolBarList[index]?.state === ToolBarV2ItemState.DISABLE))
          .focusOnTouch(!(this.toolBarList[index]?.state === ToolBarV2ItemState.DISABLE))
      }
      .justifyContent(FlexAlign.Center)
      .width('100%')
      .height('100%')
      .borderRadius($r('sys.float.ohos_id_corner_radius_clicked'))
      .padding({
        start: LengthMetrics.resource($r('sys.float.padding_level2')),
        end: LengthMetrics.resource($r('sys.float.padding_level2')),
      })
    }
    .accessibilityGroup(true)
    .accessibilityText(this.toolBarList[index]?.accessibilityText as Resource ??
      this.toolBarList[index]?.content?.text as Resource)
    .accessibilityDescription(this.toolBarList[index]?.accessibilityDescription as string ?? '')
    .accessibilityLevel(this.toolBarList[index]?.accessibilityLevel ?? 'auto')
    .enabled(this.toolBarList[index]?.state !== ToolBarV2ItemState.DISABLE)
    .width('100%')
    .height('100%')
    .borderRadius($r('sys.float.ohos_id_corner_radius_clicked'))
    .focusable(!(this.toolBarList[index]?.state === ToolBarV2ItemState.DISABLE))
    .focusOnTouch(!(this.toolBarList[index]?.state === ToolBarV2ItemState.DISABLE))
    .focusBox({
      margin: LengthMetrics.vp(FOCUS_BOX_MARGIN),
      strokeWidth: LengthMetrics.vp(FOCUS_BOX_BORDER_WIDTH),
      strokeColor: ColorMetrics.resourceColor($r('sys.color.ohos_id_color_focused_outline'))
    })
    .backgroundColor(this.toolBarList[index].backgroundColor)
    .onHover((isHover: boolean) => {
      if (isHover && this.toolBarList[index]?.state !== ToolBarV2ItemState.DISABLE) {
        this.toolBarList[index].backgroundColor = $r('sys.color.ohos_id_color_hover');
      } else {
        this.toolBarList[index].backgroundColor = Color.Transparent;
      }
    })
    .stateStyles({
      pressed: {
        .backgroundColor((this.toolBarList[index]?.state === ToolBarV2ItemState.DISABLE) ||
          (!this.toolBarModifier?.stateEffectValue) ?
          this.toolBarList[index].backgroundColor : $r('sys.color.ohos_id_color_click_effect'))
      }
    })
    .onClick(() => {
      this.clickEventAction(index);
    })
    .gestureModifier(this.getItemGestureModifier(this.toolBarList[index], index))
  }

  @Builder
  itemCardDialogBuilder(item: ToolBarV2Item, index: number): void {
    if (item.content && item.content.text) {
      Column() {
        if (item.symbol) {
          SymbolGlyph()
            .attributeModifier(this.getToolBarSymbolModifier(index))
            .symbolEffect(new SymbolEffect(), false)
            .fontColor([$r('sys.color.icon_primary')])
            .fontSize(DIALOG_IMAGE_SIZE)
            .margin({
              top: $r('sys.float.padding_level24'),
              bottom: $r('sys.float.padding_level8'),
            })
        } else if (Util.isSymbolResource(item.image?.src)) {
          SymbolGlyph(item.image?.src as Resource)
            .fontColor([$r('sys.color.icon_primary')])
            .fontSize(DIALOG_IMAGE_SIZE)
            .margin({
              top: $r('sys.float.padding_level24'),
              bottom: $r('sys.float.padding_level8'),
            })
        } else {
          Image(item.image?.src)
            .width(DIALOG_IMAGE_SIZE)
            .height(DIALOG_IMAGE_SIZE)
            .margin({
              top: $r('sys.float.padding_level24'),
              bottom: $r('sys.float.padding_level8'),
            })
            .fillColor($r('sys.color.icon_primary'))
        }
        Column() {
          Text(item.content.text)
            .fontSize(TEXT_TOOLBAR_DIALOG)
            .textOverflow({ overflow: TextOverflow.Ellipsis })
            .maxLines(this.itemCardTextMaxLine)
            .width('100%')
            .textAlign(TextAlign.Center)
            .fontColor($r('sys.color.font_primary'))
        }
        .width('100%')
        .padding({
          left: $r('sys.float.padding_level4'),
          right: $r('sys.float.padding_level4'),
          bottom: $r('sys.float.padding_level12'),
        })
      }
      .constraintSize({ minHeight: this.fontSize === MAX_FONT_SIZE ? MAX_DIALOG : MIN_DIALOG })
    } else {
      Column() {
        if (item.symbol) {
          SymbolGlyph()
            .attributeModifier(this.getToolBarSymbolModifier(index))
            .symbolEffect(new SymbolEffect(), false)
            .fontColor([$r('sys.color.icon_primary')])
            .fontSize(DIALOG_IMAGE_SIZE)
        } else if (Util.isSymbolResource(item.image?.src)) {
          SymbolGlyph(item.image?.src as Resource)
            .fontColor([$r('sys.color.icon_primary')])
            .fontSize(DIALOG_IMAGE_SIZE)
        } else {
          Image(item.image?.src)
            .width(DIALOG_IMAGE_SIZE)
            .height(DIALOG_IMAGE_SIZE)
            .fillColor($r('sys.color.icon_primary'))
        }
      }
      .constraintSize({ minHeight: this.fontSize === MAX_FONT_SIZE ? MAX_DIALOG : MIN_DIALOG })
      .justifyContent(FlexAlign.Center)
    }
  }

  onMeasureSize(selfLayoutInfo: GeometryInfo, children: Measurable[], constraint: ConstraintSizeOptions): SizeResult {
    this.fontSize = this.getFontSizeScale();
    let sizeResult: SizeResult = { height: 0, width: 0 };
    children.forEach((child) => {
      let childMeasureResult: MeasureResult = child.measure(constraint);
      sizeResult.width = childMeasureResult.width;
      sizeResult.height = childMeasureResult.height;
    });
    return sizeResult;
  }

  @Builder
  renderAdvItem(item: AdvancedToolbarV2Item, inGroup: boolean = false): void {
    if (this.setItemContentBuilder(item) && this.itemContentBuilder) {
      if (this.getEffectiveDirection() === Axis.Vertical) {
        Column() {
          this.itemContentBuilder();
        }
        .width(this.getEffectiveItemWidth(item, inGroup).value)
        .height(this.getEffectiveItemHeight(item, inGroup).value)
        .justifyContent(FlexAlign.Center)
        .alignItems(HorizontalAlign.Center)
        .borderRadius(this.getEffectiveItemBorderRadius(inGroup))
        .backgroundColor(this.getEffectiveItemDisplayBg(item, inGroup))
        .onHover((isHover: boolean) => {
          this.setItemHovered(item, isHover);
        })
        .stateStyles({
          pressed: {
            .backgroundColor(this.getEffectiveItemPressedBg(item, inGroup))
          }
        })
        .onClick(() => {
          if (this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE) {
            return;
          }
          this.advClickEventAction(item);
        })
        .accessibilityGroup(true)
        .accessibilityText(this.getAdvAccessibilityText(item))
        .accessibilityDescription(this.getAdvAccessibilityDescription(item))
        .accessibilityLevel(this.getAdvAccessibilityLevel(item))
        .enabled(this.getEffectiveItemState(item) !== ToolBarV2ItemState.DISABLE)
        .opacity(this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE ? 0.4 : 1)
        .id(item.id)
      } else {
        Row() {
          this.itemContentBuilder();
        }
        .width(this.getEffectiveItemWidth(item, inGroup).value)
        .height(this.getEffectiveItemHeight(item, inGroup).value)
        .justifyContent(FlexAlign.Center)
        .alignItems(VerticalAlign.Center)
        .borderRadius(this.getEffectiveItemBorderRadius(inGroup))
        .backgroundColor(this.getEffectiveItemDisplayBg(item, inGroup))
        .onHover((isHover: boolean) => {
          this.setItemHovered(item, isHover);
        })
        .stateStyles({
          pressed: {
            .backgroundColor(this.getEffectiveItemPressedBg(item, inGroup))
          }
        })
        .onClick(() => {
          if (this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE) {
            return;
          }
          this.advClickEventAction(item);
        })
        .accessibilityGroup(true)
        .accessibilityText(this.getAdvAccessibilityText(item))
        .accessibilityDescription(this.getAdvAccessibilityDescription(item))
        .accessibilityLevel(this.getAdvAccessibilityLevel(item))
        .enabled(this.getEffectiveItemState(item) !== ToolBarV2ItemState.DISABLE)
        .opacity(this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE ? 0.4 : 1)
        .id(item.id)
      }
    } else {
      Button({ type: ButtonType.Normal, stateEffect: false }) {
        Button({ type: ButtonType.Normal, stateEffect: false }) {
          Column() {
            if (item.icon) {
              if (item.icon instanceof ToolBarV2SymbolGlyph) {
                SymbolGlyph()
                  .fontSize(IMAGE_SIZE)
                  .symbolEffect(new SymbolEffect(), false)
                  .attributeModifier(
                    this.isItemActivated(item) ? item.icon.activated ?? item.icon.normal : item.icon.normal)
                  .fontColor([this.getEffectiveItemIconColor(item)])
                  .margin({
                    bottom: (item.content && item.isShowText !== false) ? LengthMetrics.vp(ICON_TEXT_MARGIN) :
                      LengthMetrics.vp(0)
                  });
              } else if (Util.isSymbolResource((item.icon as ToolBarV2ItemImage).src)) {
                SymbolGlyph((item.icon as ToolBarV2ItemImage).src as Resource)
                  .fontSize(IMAGE_SIZE)
                  .fontColor([this.getEffectiveItemIconColor(item)])
                  .margin({
                    bottom: (item.content && item.isShowText !== false) ? LengthMetrics.vp(ICON_TEXT_MARGIN) :
                      LengthMetrics.vp(0)
                  });
              } else {
                Image((item.icon as ToolBarV2ItemImage).src)
                  .width(IMAGE_SIZE)
                  .height(IMAGE_SIZE)
                  .fillColor(this.getEffectiveItemIconColor(item))
                  .objectFit(ImageFit.Contain)
                  .draggable(false)
                  .margin({
                    bottom: (item.content && item.isShowText !== false) ? LengthMetrics.vp(ICON_TEXT_MARGIN) :
                      LengthMetrics.vp(0)
                  });
              }
              if (item.content && item.isShowText !== false) {
                Text(item.content.text)
                  .fontColor(this.getEffectiveItemTextColor(item))
                  .fontSize(this.getEffectiveItemFontSize())
                  .fontWeight(FontWeight.Regular)
                  .maxLines(1)
                  .textOverflow({ overflow: TextOverflow.Ellipsis })
                  .textAlign(TextAlign.Center);
              }
            } else if (item.content) {
              Text(item.content.text)
                .fontColor(this.getEffectiveItemTextColor(item))
                .fontSize(this.getEffectiveItemFontSize())
                .fontWeight(FontWeight.Regular)
                .maxLines(1)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
                .textAlign(TextAlign.Center);
            }
          }
          .justifyContent(FlexAlign.Center)
          .alignItems(HorizontalAlign.Center)
          .padding({
            top: LengthMetrics.vp(ITEM_PADDING),
            bottom: LengthMetrics.vp(ITEM_PADDING),
          })
        }
        .width(this.getEffectiveItemInnerWidth(item, inGroup).value)
        .height(this.getEffectiveItemInnerHeight(item, inGroup).value)
        .padding({
          start: LengthMetrics.vp(ITEM_PADDING),
          end: LengthMetrics.vp(ITEM_PADDING),
        })
        .borderRadius(this.getEffectiveItemBorderRadius(inGroup, item))
        .backgroundColor(this.getEffectiveItemInnerDisplayBg(item))
      }
      .width(this.getEffectiveItemWidth(item, inGroup).value)
      .height(this.getEffectiveItemHeight(item, inGroup).value)
      .hoverEffect(HoverEffect.None)
      .borderRadius(this.getEffectiveItemBorderRadius(false, item))
      .backgroundColor(this.getEffectiveItemDisplayBg(item, inGroup))
      .onHover((isHover: boolean) => {
        this.setItemHovered(item, isHover);
      })
      .stateStyles({
        pressed: {
          .backgroundColor(this.getEffectiveItemPressedBg(item, inGroup))
        }
      })
      .onClick(() => {
        if (this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE) {
          return;
        }
        this.advClickEventAction(item);
      })
      .accessibilityGroup(true)
      .accessibilityText(this.getAdvAccessibilityText(item))
      .accessibilityDescription(this.getAdvAccessibilityDescription(item))
      .accessibilityLevel(this.getAdvAccessibilityLevel(item))
      .enabled(this.getEffectiveItemState(item) !== ToolBarV2ItemState.DISABLE)
      .opacity(this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE ? 0.4 : 1)
      .padding(0)
      .id(item.id)
    }
  }

  @Builder
  renderAdvItemGroup(group: ToolbarV2ItemGroup): void {
    if (this.getEffectiveDirection() === Axis.Vertical) {
      Column() {
        Column({ space: this.getEffectiveGroupSpace(group) }) {
          ForEach(group.items, (item: AdvancedToolbarV2Item, index: number) => {
            if (this.shouldRenderAdvItem(item)) {
              if (this.isAdvOverflowTriggerActive(item)) {
                this.renderOverflowButtonItem(item, true);
              } else {
                this.renderAdvItem(item, true);
              }
              if (item.isShowSeparator === true) {
                Divider()
                  .width(GROUP_DIVIDER_LENGTH)
                  .color($r('sys.color.comp_divider'))
              }
            }
          }, (item: AdvancedToolbarV2Item, index: number) => this.getGroupItemKey(item, index))
        }
      }
      .width(this.toLengthString(this.getEffectiveCrossAxisLength()))
      .alignItems(HorizontalAlign.Center)
      .backgroundColor(this.getEffectiveGroupBg(group))
      .backgroundBlurStyle(this.getEffectiveBackgroundBlurStyle())
      .systemMaterial(this.resolveGroupMaterial())
      .borderRadius(this.getEffectiveGroupBorderRadius(group))
      .borderWidth(this.getEffectiveGroupBorderWidth(group))
      .borderColor(this.getEffectiveGroupBorderColor(group))
      .padding(this.getEffectiveGroupPadding(group))
      .clip(true)
    } else {
      Row() {
        Row({ space: this.getEffectiveGroupSpace(group) }) {
          ForEach(group.items, (item: AdvancedToolbarV2Item, index: number) => {
            if (this.shouldRenderAdvItem(item)) {
              if (this.isAdvOverflowTriggerActive(item)) {
                this.renderOverflowButtonItem(item, true);
              } else {
                this.renderAdvItem(item, true);
              }
              if (item.isShowSeparator === true) {
                Divider()
                  .height(GROUP_DIVIDER_LENGTH)
                  .vertical(true)
                  .color($r('sys.color.comp_divider'))
              }
            }
          }, (item: AdvancedToolbarV2Item, index: number) => this.getGroupItemKey(item, index))
        }
      }
      .height(this.toLengthString(this.getEffectiveCrossAxisLength()))
      .alignItems(VerticalAlign.Center)
      .backgroundColor(this.getEffectiveGroupBg(group))
      .backgroundBlurStyle(this.getEffectiveBackgroundBlurStyle())
      .systemMaterial(this.resolveGroupMaterial())
      .borderRadius(this.getEffectiveGroupBorderRadius(group))
      .borderWidth(this.getEffectiveGroupBorderWidth(group))
      .borderColor(this.getEffectiveGroupBorderColor(group))
      .padding(this.getEffectiveGroupPadding(group))
      .clip(true)
    }
  }

  @Builder
  renderAdvSingleItem(item: AdvancedToolbarV2Item): void {
    if (this.getEffectiveDirection() === Axis.Vertical) {
      Column() {
        this.renderAdvItem(item, true);
      }
      .width(this.toLengthString(this.getEffectiveCrossAxisLength()))
      .alignItems(HorizontalAlign.Center)
      .backgroundColor(this.getDefaultItemBg())
      .backgroundBlurStyle(this.getEffectiveBackgroundBlurStyle())
      .systemMaterial(this.resolveItemMaterial())
      .borderRadius(this.getEffectiveBorderRadius())
      .borderWidth(this.getEffectiveBorderWidth())
      .borderColor(this.getEffectiveBorderColor())
      .padding(this.getEffectivePadding())
      .clip(true)
    } else {
      Row() {
        this.renderAdvItem(item, true);
      }
      .height(this.toLengthString(this.getEffectiveCrossAxisLength()))
      .alignItems(VerticalAlign.Center)
      .backgroundColor(this.getDefaultItemBg())
      .backgroundBlurStyle(this.getEffectiveBackgroundBlurStyle())
      .systemMaterial(this.resolveItemMaterial())
      .borderRadius(this.getEffectiveBorderRadius())
      .borderWidth(this.getEffectiveBorderWidth())
      .borderColor(this.getEffectiveBorderColor())
      .padding(this.getEffectivePadding())
      .clip(true)
    }
  }

  @Builder
  renderOverflowButtonItem(item: AdvancedToolbarV2Item, inGroup: boolean = false): void {
    Button({ type: ButtonType.Normal, stateEffect: false }) {
      Button({ type: ButtonType.Normal, stateEffect: false }) {
        Column() {
          if (item.icon instanceof ToolBarV2SymbolGlyph) {
            SymbolGlyph()
              .fontSize(IMAGE_SIZE)
              .symbolEffect(new SymbolEffect(), false)
              .attributeModifier(
                this.isItemActivated(item) ? item.icon.activated ?? item.icon.normal : item.icon.normal)
              .fontColor([this.getEffectiveItemIconColor(item)])
          } else if (item.icon !== undefined && Util.isSymbolResource((item.icon as ToolBarV2ItemImage).src)) {
            SymbolGlyph((item.icon as ToolBarV2ItemImage).src as Resource)
              .fontSize(IMAGE_SIZE)
              .fontColor([this.getEffectiveItemIconColor(item)])
          } else if (item.icon !== undefined) {
            Image((item.icon as ToolBarV2ItemImage).src)
              .width(IMAGE_SIZE)
              .height(IMAGE_SIZE)
              .fillColor(this.getEffectiveItemIconColor(item))
              .objectFit(ImageFit.Contain)
              .draggable(false)
          } else {
            SymbolGlyph($r('sys.symbol.dot_grid_2x2'))
              .fontSize(IMAGE_SIZE)
              .fontColor([this.getEffectiveItemIconColor(item)])
          }
        }
        .justifyContent(FlexAlign.Center)
        .alignItems(HorizontalAlign.Center)
        .padding({
          top: LengthMetrics.vp(ITEM_PADDING),
          bottom: LengthMetrics.vp(ITEM_PADDING),
        })
      }
      .width(this.getEffectiveItemInnerWidth(item, inGroup).value)
      .height(this.getEffectiveItemInnerHeight(item, inGroup).value)
      .padding({
        start: LengthMetrics.vp(ITEM_PADDING),
        end: LengthMetrics.vp(ITEM_PADDING),
      })
      .borderRadius(this.getEffectiveItemBorderRadius(inGroup, item))
      .backgroundColor(this.getEffectiveItemInnerDisplayBg(item))
    }
    .width(this.getEffectiveItemWidth(item, inGroup).value)
    .height(this.getEffectiveItemHeight(item, inGroup).value)
    .hoverEffect(HoverEffect.None)
    .borderRadius(this.getEffectiveItemBorderRadius(false, item))
    .backgroundColor(this.getEffectiveItemDisplayBg(item, inGroup))
    .onHover((isHover: boolean) => {
      this.setItemHovered(item, isHover);
    })
    .stateStyles({
      pressed: {
        .backgroundColor(this.getEffectiveItemPressedBg(item, inGroup))
      }
    })
    .bindMenu(this.overflowMenuBuilder, {
      placement: this.getEffectiveDirection() === Axis.Vertical ? Placement.Right : Placement.Bottom,
      scrollBar: BarState.Off,
    })
    .accessibilityGroup(true)
    .accessibilityText(this.getAdvAccessibilityText(item))
    .accessibilityDescription(this.getAdvAccessibilityDescription(item))
    .accessibilityLevel(this.getAdvAccessibilityLevel(item))
    .enabled(this.getEffectiveItemState(item) !== ToolBarV2ItemState.DISABLE)
    .opacity(this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE ? 0.4 : 1)
    .padding(0)
    .id(item.id)
  }

  @Builder
  renderAdvOverflowItem(item: AdvancedToolbarV2Item): void {
    if (this.getEffectiveDirection() === Axis.Vertical) {
      Column() {
        this.renderOverflowButtonItem(item, true);
      }
      .width(this.toLengthString(this.getEffectiveCrossAxisLength()))
      .alignItems(HorizontalAlign.Center)
      .backgroundColor(this.getDefaultItemBg())
      .backgroundBlurStyle(this.getEffectiveBackgroundBlurStyle())
      .systemMaterial(this.resolveItemMaterial())
      .borderRadius(this.getEffectiveBorderRadius())
      .borderWidth(this.getEffectiveBorderWidth())
      .borderColor(this.getEffectiveBorderColor())
      .padding(this.getEffectivePadding())
      .clip(true)
    } else {
      Row() {
        this.renderOverflowButtonItem(item, true);
      }
      .height(this.toLengthString(this.getEffectiveCrossAxisLength()))
      .alignItems(VerticalAlign.Center)
      .backgroundColor(this.getDefaultItemBg())
      .backgroundBlurStyle(this.getEffectiveBackgroundBlurStyle())
      .systemMaterial(this.resolveItemMaterial())
      .borderRadius(this.getEffectiveBorderRadius())
      .borderWidth(this.getEffectiveBorderWidth())
      .borderColor(this.getEffectiveBorderColor())
      .padding(this.getEffectivePadding())
      .clip(true)
    }
  }

  @Builder
  renderAdvItemOrGroup(item: AdvancedToolbarV2Item | ToolbarV2ItemGroup): void {
    if (this.isToolbarItemGroup(item)) {
      if (this.hasVisibleGroupChild(item as ToolbarV2ItemGroup)) {
        this.renderAdvItemGroup(item as ToolbarV2ItemGroup);
      }
    } else if (this.isAdvOverflowTriggerActive(item as AdvancedToolbarV2Item)) {
      if (this.hasAdvOverflowItems()) {
        this.renderAdvOverflowItem(item as AdvancedToolbarV2Item);
      }
    } else if (this.shouldRenderAdvItem(item as AdvancedToolbarV2Item)) {
      this.renderAdvSingleItem(item as AdvancedToolbarV2Item);
    }
  }

  @Builder
  overflowMenuBuilder(): void {
    Menu() {
      this.renderAdvOverflowMenuItems();
    }
    .font(this.getOverflowMenuFont())
    .fontColor(this.getOverflowMenuFontColor())
    .radius(this.getOverflowMenuRadius())
    .padding({
      start: LengthMetrics.vp(0),
      end: LengthMetrics.vp(0),
      top: LengthMetrics.vp(OVERFLOW_MENU_PADDING_VERTICAL),
      bottom: LengthMetrics.vp(OVERFLOW_MENU_PADDING_VERTICAL),
    })
    .menuItemDivider(this.getOverflowMenuItemDivider())
    .menuItemGroupDivider(this.getOverflowMenuGroupDivider())
  }

  @Builder
  renderAdvOverflowMenuItems(): void {
    ForEach(this.getAdvOverflowMenuItems(), (item: AdvancedToolbarV2Item) => {
      MenuItem({
        content: this.getOverflowMenuItemLabel(item),
        startIcon: this.getOverflowMenuItemIcon(item),
      })
        .height(OVERFLOW_MENU_ITEM_HEIGHT)
        .enabled(this.getEffectiveItemState(item) !== ToolBarV2ItemState.DISABLE)
        .onClick(() => {
          if (this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE) {
            return;
          }
          item.action && item.action(this.computeFlatIndex(item));
        })
    }, (item: AdvancedToolbarV2Item) => this.getOverflowMenuItemKey(item))
  }

  @Builder
  renderAdvTitle(): void {
    if (this.hasAdvTitle()) {
      Column() {
        if (this.getEffectiveDirection() === Axis.Vertical) {
          Text(this.advTitleConfigs?.content)
            .fontSize(this.getEffectiveTitleFontSize())
            .fontColor(this.advTitleConfigs?.style?.fontColor?.color ?? this.theme.fontPrimaryColor)
            .fontWeight(this.getEffectiveTitleFontWeight())
            .constraintSize({ minWidth: this.getEffectiveTitleMinLength() })
        } else {
          Text(this.advTitleConfigs?.content)
            .fontSize(this.getEffectiveTitleFontSize())
            .fontColor(this.advTitleConfigs?.style?.fontColor?.color ?? this.theme.fontPrimaryColor)
            .fontWeight(this.getEffectiveTitleFontWeight())
            .maxLines(1)
            .textOverflow({ overflow: TextOverflow.Ellipsis })
            .constraintSize(this.getAdvTitleConstraintSize())
            .onSizeChange((_oldValue: SizeOptions, newValue: SizeOptions) => {
              this.onAdvTitleSizeChange(newValue);
            })
        }
      }
      .justifyContent(FlexAlign.Center)
      .alignItems(HorizontalAlign.Center)
      .padding({
        start: LengthMetrics.vp(ITEM_PADDING),
        end: LengthMetrics.vp(ITEM_PADDING),
        top: LengthMetrics.vp(ITEM_PADDING),
        bottom: LengthMetrics.vp(ITEM_PADDING),
      })
      .flexShrink(0)
    }
  }

  @Builder
  advArea(placement: ToolbarV2ItemPlacement): void {
    if (this.getEffectiveDirection() === Axis.Vertical) {
      Column({ space: this.getEffectiveSpacing().value }) {
        if ((this.advTitleConfigs?.placement ?? ToolbarV2ItemPlacement.LEADING) === placement) {
          this.renderAdvTitle();
        }
        ForEach(this.getItemsByPlacement(placement),
          (item: AdvancedToolbarV2Item | ToolbarV2ItemGroup, index: number) => {
            this.renderAdvItemOrGroup(item);
          }, (item: AdvancedToolbarV2Item | ToolbarV2ItemGroup, index: number) =>
          `toolbar_${placement}_${this.getEntryKey(item, index)}_${this.advVisibilityRevision}`)
      }
      .width('100%')
      .height('100%')
      .justifyContent(this.getAdvZoneJustifyContent(placement))
      .padding(this.getAdvZonePadding(placement))
      .alignItems(HorizontalAlign.Center)
      .hitTestBehavior(HitTestMode.None)
    } else {
      Row({ space: this.getEffectiveSpacing().value }) {
        if ((this.advTitleConfigs?.placement ?? ToolbarV2ItemPlacement.LEADING) === placement) {
          this.renderAdvTitle();
        }
        ForEach(this.getItemsByPlacement(placement),
          (item: AdvancedToolbarV2Item | ToolbarV2ItemGroup, index: number) => {
            this.renderAdvItemOrGroup(item);
          }, (item: AdvancedToolbarV2Item | ToolbarV2ItemGroup, index: number) =>
          `toolbar_${placement}_${this.getEntryKey(item, index)}_${this.advVisibilityRevision}`)
      }
      .width('100%')
      .height('100%')
      .justifyContent(this.getAdvZoneJustifyContent(placement))
      .padding(this.getAdvZonePadding(placement))
      .alignItems(VerticalAlign.Center)
      .hitTestBehavior(HitTestMode.None)
    }
  }

  @Builder
  advToolbarContent(): void {
    Stack() {
      this.advArea(ToolbarV2ItemPlacement.LEADING)
      this.advArea(ToolbarV2ItemPlacement.TRAILING)
      this.advArea(ToolbarV2ItemPlacement.MAIN)
    }
    .width(this.toLengthString(
      this.getEffectiveDirection() === Axis.Vertical
        ? this.getEffectiveCrossAxisLength()
        : this.getEffectiveMainAxisLength()
    ))
    .height(this.toLengthString(
      this.getEffectiveDirection() === Axis.Vertical
        ? this.getEffectiveMainAxisLength()
        : this.getEffectiveCrossAxisLength()
    ))
    .clip(true)
    .onSizeChange((_oldValue, newValue) => {
      this.onAdvContentMainAxisSizeChange(newValue);
    })
  }

  private getAdvAccessibilityText(item: AdvancedToolbarV2Item): string | undefined {
    return item.accessibilityText as ESObject as string ?? item.content?.text as ESObject as string;
  }

  private getAdvAccessibilityDescription(item: AdvancedToolbarV2Item): string | undefined {
    return item.accessibilityDescription as ESObject as string ?? '';
  }

  private getAdvAccessibilityLevel(item: AdvancedToolbarV2Item): string {
    return item.accessibilityLevel ?? 'auto';
  }

  private getFontSizeScale(): number {
    let context = this.getUIContext();
    let fontScaleSystem = (context.getHostContext() as common.UIAbilityContext)?.config?.fontSizeScale ?? 1;
    if (!this.isFollowSystem) {
      return 1;
    } else {
      return Math.min(fontScaleSystem, this.maxFontSizeScale);
    }
  }

  private isItemActivating(index: number): boolean {
    return this.localActivatedIndex === index && (this.toolBarList[index]?.state === ToolBarV2ItemState.ACTIVATE);
  }

  private getToolBarSymbolModifier(index: number): SymbolGlyphModifier | undefined {
    if (this.isItemActivating(index)) {
      return this.toolBarList[index]?.symbol?.activated;
    }
    return this.toolBarList[index]?.symbol?.normal;
  }

  private getIconColor(index: number): ResourceColor {
    if (this.isItemActivating(index)) {
      return this.toolBarList[index]?.image?.activatedColor?.color ?? this.theme.iconActivePrimaryColor;
    }
    return this.toolBarList[index]?.image?.color?.color ?? this.theme.iconPrimaryColor;
  }

  private getTextColor(index: number): ResourceColor {
    if (this.isItemActivating(index)) {
      return this.toolBarList[index]?.content.activatedColor?.color ?? this.theme.fontActivatedPrimaryColor;
    }
    return this.toolBarList[index]?.content.color?.color ?? this.theme.fontPrimaryColor;
  }

  private toLengthString(value?: LengthMetrics): string {
    if (value === undefined) {
      return '';
    }
    const length: number = value.value;
    let lengthString: string = '';
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

  private clickEventAction(index: number): void {
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

  private getItemGestureModifier(item: ToolBarV2Item, index: number): ButtonGestureModifier | undefined {
    if (!item?.icon) {
      return undefined;
    }
    let buttonGestureModifier: ButtonGestureModifier = new ButtonGestureModifier();
    buttonGestureModifier.gestureCallBack = (event: UIGestureEvent) => {
      if (this.fontSize >= ButtonGestureModifier.minFontSize) {
        event.addGesture(
          new LongPressGestureHandler({ repeat: false, duration: ButtonGestureModifier.longPressTime })
            .onAction(() => {
              promptAction.openCustomDialog({
                builder: () => {
                  this.itemCardDialogBuilder(item, index);
                },
                onWillAppear: () => {
                  try {
                    let context = this.getUIContext().getHostContext() as common.UIAbilityContext;
                    let mainWindowStage = context.windowStage.getMainWindowSync();
                    let properties: window.WindowProperties = mainWindowStage.getWindowProperties();
                    if (px2vp(properties.windowRect.height) > SCREEN_WIDTH_BREAK_POINT) {
                      this.itemCardTextMaxLine = VERTICAL_SCREEN_TEXT_MAX_LINES;
                    } else {
                      this.itemCardTextMaxLine = HORIZONTAL_SCREEN_TEXT_MAX_LINES;
                    }
                  } catch (err) {
                    let code: number = (err as BusinessError)?.code;
                    let message: string = (err as BusinessError)?.message;
                    hilog.error(0x3900, 'Ace',
                      `ToolBarV2 get window height failed, code: ${code}, message: ${message}`);
                  }
                },
                maskColor: Color.Transparent,
                isModal: true,
                backgroundBlurStyle: BlurStyle.COMPONENT_ULTRA_THICK,
                backgroundColor: Color.Transparent,
                shadow: ShadowStyle.OUTER_DEFAULT_LG,
                cornerRadius: $r('sys.float.corner_radius_level10'),
                width: this.fontSize === MAX_FONT_SIZE ? MAX_DIALOG : MIN_DIALOG
              }).then((dialogId: number) => {
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
        )
        return;
      }
      event.clearGestures();
    }
    return buttonGestureModifier;
  }

  private isToolbarItemGroup(item: AdvancedToolbarV2Item | ToolbarV2ItemGroup): boolean {
    return Array.isArray((item as ToolbarV2ItemGroup).items);
  }

  private buildAdvEntries(): void {
    for (let k = 0; k < this.advEntries.length; k++) {
      const oldEntry = this.advEntries[k];
      if (this.isToolbarItemGroup(oldEntry)) {
        const group = oldEntry as ToolbarV2ItemGroup;
        for (let j = 0; j < group.items.length; j++) {
          group.items[j].onStructureChange = undefined;
        }
      } else if (oldEntry instanceof AdvancedToolbarV2Item) {
        oldEntry.onStructureChange = undefined;
      }
    }

    const list = this.toolBarList;
    this.advItemOriginalIndex = new Map();
    const entries: Array<AdvancedToolbarV2Item | ToolbarV2ItemGroup> = [];
    const groupMap: Map<string, ToolbarV2ItemGroup> = new Map();

    for (let i = 0; i < list.length; i++) {
      const item = list[i];
      if (!(item instanceof AdvancedToolbarV2Item)) {
        entries.push(item as AdvancedToolbarV2Item);
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
        const group = entry as ToolbarV2ItemGroup;
        for (let j = 0; j < group.items.length; j++) {
          group.items[j].onStructureChange = () => this.rebuildAdvState();
        }
      } else if (entry instanceof AdvancedToolbarV2Item) {
        entry.onStructureChange = () => this.rebuildAdvState();
      }
    }
  }

  private rebuildAdvState(): void {
    this.advTitleConfigs = undefined;
    this.advTitleRenderWidth = 0;
    this.advMainFlowOffset = 0;
    this.advMainClamped = false;
    this.advHiddenItems = [];
    this.buildAdvEntries();
    const signature = this.getAdvTitleSignature();
    if (signature !== this.advTitleSignature) {
      this.advTitleSignature = signature;
      this.advTitleNaturalWidth = 0;
    }
    this.computeAdvVisibility();
  }

  private getAdvTitleSignature(): string {
    const configs = this.advTitleConfigs;
    if (configs === undefined) {
      return '';
    }
    const content = configs.content;
    let contentKey = '';
    if (typeof content === 'string') {
      contentKey = content;
    } else if (content !== undefined && content !== null) {
      const resource = content as Resource;
      contentKey = `${resource.id}_${resource.type ?? ''}`;
    }
    const style = configs.style;
    const fontSize = style?.fontSize;
    return `${contentKey}|${fontSize?.value ?? ''}_${fontSize?.unit ?? ''}|` +
      `${style?.fontWeight ?? ''}|${style?.minLength?.value ?? ''}`;
  }

  private computeFlatIndex(target: AdvancedToolbarV2Item): number {
    return this.advItemOriginalIndex.get(target) ?? -1;
  }

  private setItemContentBuilder(item: AdvancedToolbarV2Item): boolean {
    this.itemContentBuilder = item.builder;
    return this.itemContentBuilder ? true : false;
  }

  private getEffectiveDirection(): Axis {
    return this.advancedToolBarModifier?.directionValue ?? Axis.Horizontal;
  }

  private getEffectiveSpacing(): LengthMetrics {
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

  private getEffectiveMainAxisLength(): LengthMetrics {
    return this.advancedToolBarModifier?.mainAxisLengthValue ?? LengthMetrics.percent(1);
  }

  // 主轴预算：固定长度时直接使用配置值，不依赖 onSizeChange 是否首帧回调；百分比时等待实测
  private getAdvMainAxisBudget(): number {
    const configured: LengthMetrics = this.getEffectiveMainAxisLength();
    if (configured.unit !== LengthUnit.PERCENT) {
      return Math.max(0, Util.lengthMetricsToVp(configured));
    }
    return Math.max(0, this.advMainAxisSize);
  }

  private getEffectiveCrossAxisLength(): LengthMetrics {
    return this.advancedToolBarModifier?.crossAxisLengthValue ?? LengthMetrics.vp(DEFAULT_TOOLBAR_HEIGHT);
  }

  private getEffectiveToolbarCross(): number {
    const cross = this.getEffectiveCrossAxisLength();
    return cross ? Util.lengthMetricsToVp(cross) : DEFAULT_TOOLBAR_HEIGHT;
  }

  private getHalfCrossAxis(): string {
    const cross = this.getEffectiveCrossAxisLength();
    const halfValue = (cross ? Util.lengthMetricsToVp(cross) : DEFAULT_TOOLBAR_HEIGHT) / 2;
    return `${halfValue}vp`;
  }

  private getEffectiveBorderRadius(): Length {
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.borderRadius) {
      return this.toLengthString(style.borderRadius);
    }
    return this.getHalfCrossAxis();
  }

  private getEffectiveBorderWidth(): Length {
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.borderWidth) {
      return this.toLengthString(style.borderWidth);
    }
    return 0;
  }

  private getEffectiveBorderColor(): ResourceColor {
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.borderColor) {
      return style.borderColor.color;
    }
    return Color.Transparent;
  }

  private getEffectiveGroupBorderRadius(group?: ToolbarV2ItemGroup): Length {
    if (group?.style?.borderRadius) {
      return this.toLengthString(group.style.borderRadius);
    }
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.borderRadius) {
      return this.toLengthString(style.borderRadius);
    }
    return this.getHalfCrossAxis();
  }

  private getEffectiveGroupBorderWidth(group?: ToolbarV2ItemGroup): Length {
    if (group?.style?.borderWidth) {
      return this.toLengthString(group.style.borderWidth);
    }
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.borderWidth) {
      return this.toLengthString(style.borderWidth);
    }
    return 0;
  }

  private getEffectiveGroupBorderColor(group?: ToolbarV2ItemGroup): ResourceColor {
    if (group?.style?.borderColor) {
      return group.style.borderColor.color;
    }
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.borderColor) {
      return style.borderColor.color;
    }
    return Color.Transparent;
  }

  private getEffectiveGroupBg(group?: ToolbarV2ItemGroup): ResourceColor {
    if (this.getEffectiveSystemBackgroundMaterial()) {
      return Color.Transparent;
    }
    if (group?.style?.backgroundColor) {
      return group.style.backgroundColor.color;
    }
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.backgroundColor) {
      return style.backgroundColor.color;
    }
    return $r('sys.color.comp_background_primary');
  }

  private getEffectiveBackgroundBlurStyle(): BlurStyle {
    if (this.getEffectiveSystemBackgroundMaterial()) {
      return BlurStyle.NONE;
    }
    return this.advancedToolBarModifier?.styleConfigsValue?.backgroundBlurStyle ?? BlurStyle.NONE;
  }

  private getEffectiveSystemBackgroundMaterial(): uiMaterial.Material | undefined {
    const info = uiMaterial.getMaterialInfo();
    if (info.state !== uiMaterial.MaterialState.ENABLE) {
      return undefined;
    }
    const userMaterial = this.advancedToolBarModifier?.styleConfigsValue?.systemBackgroundMaterial;
    if (userMaterial) {
      return userMaterial;
    }
    const stylePreset = this.advancedToolBarModifier?.styleConfigsValue?.systemMaterialStyle;
    if (stylePreset === ToolbarV2MaterialStyle.DEFAULT) {
      return new uiMaterial.ImmersiveMaterial({
        style: uiMaterial.ImmersiveStyle.THIN,
        interactive: true,
      });
    }
    return undefined;
  }

  private resolveItemMaterial(): uiMaterial.Material | undefined {
    return this.getEffectiveSystemBackgroundMaterial();
  }

  private resolveGroupMaterial(): uiMaterial.Material | undefined {
    return this.getEffectiveSystemBackgroundMaterial();
  }

  private getEffectiveItemHoverBg(item: AdvancedToolbarV2Item): ResourceColor {
    return $r('sys.color.ohos_id_color_hover');
  }

  private getEffectiveItemIconColor(item: AdvancedToolbarV2Item): ResourceColor {
    if (this.isItemActivated(item)) {
      const useOnPrimary = this.isItemOnlyIcon(item) && this.hasActivatedBg(item);
      const defaultColor = useOnPrimary ? this.theme.iconOnPrimaryColor : this.theme.iconActivePrimaryColor;
      const icon = item.icon;
      if (icon instanceof ToolBarV2SymbolGlyph) {
        return defaultColor;
      }
      return (icon as ToolBarV2ItemImage)?.activatedColor?.color ??
        (icon as ToolBarV2ItemImage)?.color?.color ??
        defaultColor;
    }
    const icon = item.icon;
    if (icon instanceof ToolBarV2SymbolGlyph) {
      return this.theme.iconPrimaryColor;
    }
    return (icon as ToolBarV2ItemImage)?.color?.color ?? this.theme.iconPrimaryColor;
  }

  private getEffectiveItemTextColor(item: AdvancedToolbarV2Item): ResourceColor {
    if (this.isItemActivated(item)) {
      return item.content?.activatedColor?.color ?? this.theme.fontActivatedPrimaryColor;
    }
    return item.content?.color?.color ?? this.theme.fontSecondaryColor;
  }

  private getEffectiveItemFontSize(): Resource | number {
    const raw = this.advancedToolBarModifier?.styleConfigsValue?.fontSize;
    if (raw) {
      return raw.value;
    }
    return $r('sys.float.ohos_id_text_size_caption');
  }

  private getEffectiveTitleFontSize(): number {
    const raw = this.advTitleConfigs?.style?.fontSize;
    if (raw) {
      return Util.lengthMetricsToVp(raw);
    }
    return DEFAULT_TITLE_FONT_SIZE;
  }

  private getEffectiveTitleFontWeight(): FontWeight {
    return this.advTitleConfigs?.style?.fontWeight ?? DEFAULT_TITLE_FONT_WEIGHT;
  }

  private getEffectiveTitleMinLength(): number {
    const raw = this.advTitleConfigs?.style?.minLength;
    if (raw) {
      return Util.lengthMetricsToVp(raw);
    }
    return DEFAULT_TITLE_MIN_LENGTH;
  }

  private getEffectiveStyleStateEffect(): boolean {
    return this.advancedToolBarModifier?.styleConfigsValue?.stateEffect ?? DEFAULT_STYLE_STATE_EFFECT;
  }

  private getEffectiveGroupPadding(group: ToolbarV2ItemGroup | undefined): Padding {
    const p = group?.style?.padding;
    const sp = this.advancedToolBarModifier?.styleConfigsValue?.padding;
    const isVertical = this.getEffectiveDirection() === Axis.Vertical;
    const mainStart = p?.mainAxisStart ? Util.lengthMetricsToVp(p.mainAxisStart)
      : (sp?.mainAxisStart ? Util.lengthMetricsToVp(sp.mainAxisStart) : GROUP_MAIN_AXIS_PADDING);
    const mainEnd = p?.mainAxisEnd ? Util.lengthMetricsToVp(p.mainAxisEnd)
      : (sp?.mainAxisEnd ? Util.lengthMetricsToVp(sp.mainAxisEnd) : GROUP_MAIN_AXIS_PADDING);
    const result: Padding = {};
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

  private getEffectiveItemPressedBg(item: AdvancedToolbarV2Item, inGroup: boolean): ResourceColor {
    if (this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE) {
      return Color.Transparent;
    }
    if (!this.getEffectiveStyleStateEffect()) {
      return Color.Transparent;
    }
    if (this.getEffectiveSystemBackgroundMaterial() &&
      uiMaterial.getGlobalMaterialLevel() !== uiMaterial.MaterialLevel.SMOOTH) {
      return Color.Transparent;
    }
    if (inGroup) {
      const groupEffect = this.getGroupStateEffectByItem(item);
      if (groupEffect === false) {
        return Color.Transparent;
      }
    }
    return $r('sys.color.ohos_id_color_click_effect');
  }

  private getGroupStateEffectByItem(item: AdvancedToolbarV2Item): boolean | undefined {
    for (let i = 0; i < this.advEntries.length; i++) {
      const entry = this.advEntries[i];
      if (this.isToolbarItemGroup(entry)) {
        const group = entry as ToolbarV2ItemGroup;
        for (let j = 0; j < group.items.length; j++) {
          if (group.items[j] === item) {
            return group.style?.stateEffect;
          }
        }
      }
    }
    return undefined;
  }

  private hasActivatedBg(item: AdvancedToolbarV2Item): boolean {
    if (!item.activatedBackgroundColor) {
      return false;
    }
    const c = item.activatedBackgroundColor;
    return c.alpha !== 0 || c.red !== 0 || c.green !== 0 || c.blue !== 0;
  }

  private getDefaultItemBg(): ResourceColor {
    if (this.getEffectiveSystemBackgroundMaterial()) {
      return Color.Transparent;
    }
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.backgroundColor) {
      return style.backgroundColor.color;
    }
    return $r('sys.color.comp_background_primary');
  }

  private getEffectiveItemBg(item: AdvancedToolbarV2Item, inGroup: boolean): ResourceColor {
    return inGroup ? Color.Transparent : this.getDefaultItemBg();
  }

  private getEffectiveItemActivatedBg(item: AdvancedToolbarV2Item): ResourceColor {
    if (this.isItemActivated(item) && this.isItemOnlyIcon(item) && this.hasActivatedBg(item)) {
      return item.activatedBackgroundColor!.color;
    }
    return Color.Transparent;
  }

  private getEffectiveItemState(item: AdvancedToolbarV2Item): ToolBarV2ItemState {
    const s = item.state;
    if (s === ToolBarV2ItemState.ENABLE || s === ToolBarV2ItemState.DISABLE || s === ToolBarV2ItemState.ACTIVATE) {
      return s;
    }
    return ToolBarV2ItemState.ENABLE;
  }

  private isItemActivated(item: AdvancedToolbarV2Item): boolean {
    return this.getEffectiveItemState(item) === ToolBarV2ItemState.ACTIVATE;
  }

  private advClickEventAction(item: AdvancedToolbarV2Item): void {
    if (this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE) {
      return;
    }
    const flatIndex = this.computeFlatIndex(item);
    item.action && item.action(flatIndex);
  }

  private getEffectiveItemDisplayBg(item: AdvancedToolbarV2Item, inGroup: boolean): ResourceColor {
    if (this.getEffectiveItemState(item) === ToolBarV2ItemState.DISABLE) {
      return this.getEffectiveItemBg(item, inGroup);
    }
    if (this.advHoveredItems.has(item)) {
      return this.getEffectiveItemHoverBg(item);
    }
    return this.getEffectiveItemBg(item, inGroup);
  }

  private getEffectiveItemInnerDisplayBg(item: AdvancedToolbarV2Item): ResourceColor {
    return this.getEffectiveItemActivatedBg(item);
  }

  private setItemHovered(item: AdvancedToolbarV2Item, isHover: boolean): void {
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

  private getEffectivePadding(): Padding {
    const p = this.advancedToolBarModifier?.styleConfigsValue?.padding;
    const isVertical = this.getEffectiveDirection() === Axis.Vertical;
    const mainStart = p?.mainAxisStart ? Util.lengthMetricsToVp(p.mainAxisStart) : DEFAULT_MAIN_AXIS_PADDING;
    const mainEnd = p?.mainAxisEnd ? Util.lengthMetricsToVp(p.mainAxisEnd) : DEFAULT_MAIN_AXIS_PADDING;
    const result: Padding = {};
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

  private findGroupByItem(item: AdvancedToolbarV2Item): ToolbarV2ItemGroup | undefined {
    for (const entry of this.advEntries) {
      if (this.isToolbarItemGroup(entry)) {
        const group = entry as ToolbarV2ItemGroup;
        if (group.items && group.items.includes(item)) {
          return group;
        }
      }
    }
    return undefined;
  }

  private isItemWithIconAndText(item?: AdvancedToolbarV2Item): boolean {
    if (!item) {
      return false;
    }
    if (item.builder) {
      return false;
    }
    if (item.titleConfigs) {
      return false;
    }
    return !!item.icon && !!item.content && !!item.content.text && item.isShowText !== false;
  }

  private clampMainAxis(raw: number, item?: AdvancedToolbarV2Item): number {
    if (this.isItemWithIconAndText(item)) {
      return Math.max(DEFAULT_ITEM_MAIN_AXIS, Math.min(MAX_ITEM_MAIN_AXIS, raw));
    }
    return raw;
  }

  private clampCrossAxis(raw: number): number {
    const max = Util.lengthMetricsToVp(this.getEffectiveCrossAxisLength());
    return Math.min(max, Math.max(0, raw));
  }

  private getEffectiveItemWidth(item?: AdvancedToolbarV2Item, inGroup: boolean = false): LengthMetrics {
    const isVertical = this.getEffectiveDirection() === Axis.Vertical;
    const styleItemSize = this.advancedToolBarModifier?.styleConfigsValue?.itemSize;
    const effectiveSize = item?.itemSize ?? styleItemSize;

    if (effectiveSize) {
      if (isVertical) {
        return LengthMetrics.vp(this.clampCrossAxis(Math.max(0, effectiveSize.width)));
      }
      return LengthMetrics.vp(this.clampMainAxis(Math.max(0, effectiveSize.width), item));
    }
    if (isVertical) {
      return LengthMetrics.vp(DEFAULT_ITEM_CROSS_AXIS);
    }
    return LengthMetrics.vp(DEFAULT_ITEM_MAIN_AXIS);
  }

  private isItemOnlyIcon(item?: AdvancedToolbarV2Item): boolean {
    return !!item?.icon && (!item.content || !item.content.text || item.isShowText === false);
  }

  private getEffectiveItemHeight(item?: AdvancedToolbarV2Item, inGroup: boolean = false): LengthMetrics {
    const isVertical = this.getEffectiveDirection() === Axis.Vertical;
    const styleItemSize = this.advancedToolBarModifier?.styleConfigsValue?.itemSize;
    const effectiveSize = item?.itemSize ?? styleItemSize;

    if (effectiveSize) {
      if (isVertical) {
        return LengthMetrics.vp(this.clampMainAxis(Math.max(0, effectiveSize.height), item));
      }
      return LengthMetrics.vp(this.clampCrossAxis(Math.max(0, effectiveSize.height)));
    }
    if (isVertical) {
      return LengthMetrics.vp(DEFAULT_ITEM_MAIN_AXIS);
    }
    return LengthMetrics.vp(DEFAULT_ITEM_CROSS_AXIS);
  }

  private getEffectivePlacement(raw: ToolbarV2ItemPlacement | undefined): ToolbarV2ItemPlacement {
    if (raw === ToolbarV2ItemPlacement.MAIN || raw === ToolbarV2ItemPlacement.LEADING ||
      raw === ToolbarV2ItemPlacement.TRAILING) {
      return raw;
    }
    return ToolbarV2ItemPlacement.MAIN;
  }

  private getItemsByPlacement(placement: ToolbarV2ItemPlacement): Array<AdvancedToolbarV2Item | ToolbarV2ItemGroup> {
    const result: Array<AdvancedToolbarV2Item | ToolbarV2ItemGroup> = [];
    for (let i = 0; i < this.advEntries.length; i++) {
      const entry = this.advEntries[i];
      const entryPlacement = this.getEffectivePlacement((entry as ToolbarV2ItemGroup).placement);
      if (entryPlacement === placement) {
        result.push(entry);
      }
    }
    return result;
  }

  private getEntryKey(item: AdvancedToolbarV2Item | ToolbarV2ItemGroup, index: number): string {
    if (this.isToolbarItemGroup(item)) {
      const group = item as ToolbarV2ItemGroup;
      return `group_${index}_${group.items.length}`;
    }
    const advItem = item as AdvancedToolbarV2Item;
    return `item_${index}_${advItem.id ?? ''}`;
  }

  private getGroupItemKey(item: AdvancedToolbarV2Item, index: number): string {
    return item.id ?? `group_item_${index}`;
  }

  private normalizeDisplayPriority(item: AdvancedToolbarV2Item): number {
    const raw: number | undefined | null = item.displayPriority;
    if (raw === undefined || raw === null) {
      return DISPLAY_PRIORITY_DEFAULT;
    }
    if (typeof raw !== 'number' || !Number.isFinite(raw) || raw < DISPLAY_PRIORITY_MIN) {
      return DISPLAY_PRIORITY_DEFAULT;
    }
    if (raw > DISPLAY_PRIORITY_MAX) {
      return DISPLAY_PRIORITY_MAX;
    }
    return raw;
  }

  private collectAdvPriorityEntries(): Array<AdvPriorityEntry> {
    const entries: Array<AdvPriorityEntry> = [];
    for (let i = 0; i < this.toolBarList.length; i++) {
      const raw = this.toolBarList[i];
      if (!(raw instanceof AdvancedToolbarV2Item)) {
        continue;
      }
      const item = raw as AdvancedToolbarV2Item;
      if (item.titleConfigs !== undefined || item.isShowOverflowMenu === true || item.builder) {
        continue;
      }
      const priority = this.normalizeDisplayPriority(item);
      if (priority >= DISPLAY_PRIORITY_MAX) {
        continue;
      }
      entries.push({ item: item, index: i, priority: priority });
    }
    return entries;
  }

  private getAdvFoldOrder(entries: Array<AdvPriorityEntry>): Array<AdvPriorityEntry> {
    const forcedHidden: Array<AdvPriorityEntry> = [];
    const foldables: Array<AdvPriorityEntry> = [];
    for (let i = 0; i < entries.length; i++) {
      if (entries[i].priority <= DISPLAY_PRIORITY_MIN) {
        forcedHidden.push(entries[i]);
      } else {
        foldables.push(entries[i]);
      }
    }
    foldables.sort((a: AdvPriorityEntry, b: AdvPriorityEntry) => {
      if (a.priority !== b.priority) {
        return a.priority - b.priority;
      }
      // 同优先级：后声明先收
      return b.index - a.index;
    });
    return forcedHidden.concat(foldables);
  }

  private isAdvOverflowTrigger(item: AdvancedToolbarV2Item): boolean {
    return item.isShowOverflowMenu === true;
  }

  private isAdvOverflowTriggerActive(item: AdvancedToolbarV2Item): boolean {
    // 竖向同样会收纳 item，溢出按钮需要保留并弹出菜单（Placement.Right）
    return this.isAdvOverflowTrigger(item);
  }

  private hasAdvOverflowItems(): boolean {
    return this.advHiddenItems.length > 0;
  }

  private shouldRenderAdvItem(item: AdvancedToolbarV2Item): boolean {
    // 溢出按钮只在收纳集合非空时显示
    if (this.isAdvOverflowTriggerActive(item) && !this.hasAdvOverflowItems()) {
      return false;
    }
    return !this.advHiddenItems.includes(item);
  }

  private hasVisibleGroupChild(group: ToolbarV2ItemGroup): boolean {
    for (let i = 0; i < group.items.length; i++) {
      const item = group.items[i];
      if (this.isAdvOverflowTriggerActive(item)) {
        if (this.hasAdvOverflowItems()) {
          return true;
        }
      } else if (this.shouldRenderAdvItem(item)) {
        return true;
      }
    }
    return false;
  }

  private hasAdvTitle(): boolean {
    return this.getEffectiveDirection() === Axis.Horizontal &&
      this.advTitleConfigs !== undefined && (this.advTitleConfigs.visible ?? true);
  }

  private getEffectiveTitlePlacement(): ToolbarV2ItemPlacement {
    return this.advTitleConfigs?.placement ?? ToolbarV2ItemPlacement.LEADING;
  }

  private getAdvTitleMaxWidth(): number | undefined {
    if (this.advTitleRenderWidth <= 0) {
      return undefined;
    }
    return this.advTitleRenderWidth;
  }

  private getAdvTitleConstraintSize(): ConstraintSizeOptions {
    const size: ConstraintSizeOptions = { minWidth: this.getEffectiveTitleMinLength() };
    const maxWidth = this.getAdvTitleMaxWidth();
    if (maxWidth !== undefined) {
      size.maxWidth = maxWidth;
    }
    return size;
  }

  private onAdvTitleSizeChange(newValue: SizeOptions): void {
    if (this.getEffectiveDirection() === Axis.Vertical) {
      return;
    }
    const width = Number(newValue.width) || 0;
    if (width <= 0 || this.advTitleNaturalWidth > 0) {
      return;
    }
    if (this.advTitleRenderWidth > 0 && width >= this.advTitleRenderWidth - 1) {
      // 被 maxWidth 截断，量到的不是自然宽度
      return;
    }
    this.advTitleNaturalWidth = width;
    this.computeAdvVisibility();
  }

  private getAdvItemMainSize(item: AdvancedToolbarV2Item): number {
    return Util.lengthMetricsToVp(this.getEffectiveDirection() === Axis.Vertical
      ? this.getEffectiveItemHeight(item)
      : this.getEffectiveItemWidth(item));
  }

  private getAdvItemWrapperPadding(): number {
    const padding = this.getEffectivePadding();
    if (this.getEffectiveDirection() === Axis.Vertical) {
      return this.paddingEdgeVp(padding.top) + this.paddingEdgeVp(padding.bottom);
    }
    return this.paddingEdgeVp(padding.left) + this.paddingEdgeVp(padding.right);
  }

  private getAdvGroupWrapperPadding(group: ToolbarV2ItemGroup): number {
    const padding = this.getEffectiveGroupPadding(group);
    if (this.getEffectiveDirection() === Axis.Vertical) {
      return this.paddingEdgeVp(padding.top) + this.paddingEdgeVp(padding.bottom);
    }
    return this.paddingEdgeVp(padding.left) + this.paddingEdgeVp(padding.right);
  }

  private computeAdvSingleUnitWidth(item: AdvancedToolbarV2Item): number {
    return this.getAdvItemMainSize(item) + this.getAdvItemWrapperPadding();
  }

  private computeAdvGroupUnitWidth(group: ToolbarV2ItemGroup, hidden: Set<AdvancedToolbarV2Item>): number {
    let childCount = 0;
    let width = 0;
    for (let i = 0; i < group.items.length; i++) {
      const child = group.items[i];
      if (hidden.has(child)) {
        continue;
      }
      if (this.isAdvOverflowTrigger(child) && hidden.size === 0) {
        continue;
      }
      width += this.getAdvItemMainSize(child);
      childCount++;
    }
    if (childCount === 0) {
      return 0;
    }
    width += this.getEffectiveGroupSpace(group) * (childCount - 1);
    width += this.getAdvGroupWrapperPadding(group);
    return width;
  }

  private sumAdvUnitWidth(units: Array<number>, spacing: number): number {
    if (units.length === 0) {
      return 0;
    }
    let total = 0;
    for (let i = 0; i < units.length; i++) {
      total += units[i];
    }
    return total + spacing * (units.length - 1);
  }

  private computeAdvZoneWidths(hidden: Set<AdvancedToolbarV2Item>, titleTextWidth: number): AdvZoneMetrics {
    const spacing = Util.lengthMetricsToVp(this.getEffectiveSpacing());
    const titlePlacement = this.getEffectiveTitlePlacement();
    const overflowVisible = hidden.size > 0;
    const leadingUnits: Array<number> = [];
    const mainUnits: Array<number> = [];
    const trailingUnits: Array<number> = [];
    if (this.hasAdvTitle()) {
      const titleUnit = titleTextWidth + 2 * ITEM_PADDING;
      if (titlePlacement === ToolbarV2ItemPlacement.TRAILING) {
        trailingUnits.push(titleUnit);
      } else if (titlePlacement === ToolbarV2ItemPlacement.MAIN) {
        mainUnits.push(titleUnit);
      } else {
        leadingUnits.push(titleUnit);
      }
    }
    for (let i = 0; i < this.advEntries.length; i++) {
      const entry = this.advEntries[i];
      const placement = this.getEffectivePlacement((entry as ToolbarV2ItemGroup).placement);
      let unitWidth = 0;
      if (this.isToolbarItemGroup(entry)) {
        unitWidth = this.computeAdvGroupUnitWidth(entry as ToolbarV2ItemGroup, hidden);
      } else {
        const item = entry as AdvancedToolbarV2Item;
        if (hidden.has(item)) {
          unitWidth = 0;
        } else if (this.isAdvOverflowTrigger(item)) {
          unitWidth = overflowVisible ? this.computeAdvSingleUnitWidth(item) : 0;
        } else {
          unitWidth = this.computeAdvSingleUnitWidth(item);
        }
      }
      if (unitWidth <= 0) {
        continue;
      }
      if (placement === ToolbarV2ItemPlacement.LEADING) {
        leadingUnits.push(unitWidth);
      } else if (placement === ToolbarV2ItemPlacement.TRAILING) {
        trailingUnits.push(unitWidth);
      } else {
        mainUnits.push(unitWidth);
      }
    }
    const metrics: AdvZoneMetrics = {
      leading: this.sumAdvUnitWidth(leadingUnits, spacing),
      main: this.sumAdvUnitWidth(mainUnits, spacing),
      trailing: this.sumAdvUnitWidth(trailingUnits, spacing),
      titleTextWidth: titleTextWidth,
    };
    return metrics;
  }

  private computeAdvGapCount(metrics: AdvZoneMetrics): number {
    let count = 0;
    if (metrics.leading > 0) {
      count++;
    }
    if (metrics.main > 0) {
      count++;
    }
    if (metrics.trailing > 0) {
      count++;
    }
    return count > 0 ? count - 1 : 0;
  }

  private computeAdvIsFeasible(metrics: AdvZoneMetrics, budget: number): boolean {
    const total = metrics.leading + metrics.main + metrics.trailing +
      this.computeAdvGapCount(metrics) * DEFAULT_ZONE_GAP;
    return total <= budget + ADV_FIT_EPSILON;
  }

  private computeAdvMainOffset(metrics: AdvZoneMetrics, budget: number): number {
    if (metrics.main <= 0) {
      return 0;
    }
    const centered = (budget - metrics.main) / 2;
    let lower = 0;
    let upper = budget - metrics.main;
    if (metrics.leading > 0) {
      lower = metrics.leading + DEFAULT_ZONE_GAP;
    }
    if (metrics.trailing > 0) {
      upper = budget - metrics.trailing - DEFAULT_ZONE_GAP - metrics.main;
    }
    if (lower > upper) {
      // 两侧最小间隔无法同时满足：保持头部最小间隔，剩余交给收纳流程
      return Math.max(0, lower);
    }
    return Math.min(Math.max(centered, lower), upper);
  }

  private isAdvMainOffsetClamped(metrics: AdvZoneMetrics, budget: number, offset: number): boolean {
    if (metrics.main <= 0) {
      return false;
    }
    return Math.abs(offset - (budget - metrics.main) / 2) > ADV_FIT_EPSILON;
  }



  private isAdvHiddenItemsEqual(a: AdvancedToolbarV2Item[], b: AdvancedToolbarV2Item[]): boolean {
    if (a.length !== b.length) {
      return false;
    }
    for (let i = 0; i < a.length; i++) {
      if (a[i] !== b[i]) {
        return false;
      }
    }
    return true;
  }

  private applyAdvVisibilityState(hiddenItems: AdvancedToolbarV2Item[], titleWidth: number,
    mainOffset: number, mainClamped: boolean): void {
    let hiddenChanged = false;
    if (this.advMainClamped !== mainClamped) {
      this.advMainClamped = mainClamped;
    }
    if (!this.isAdvHiddenItemsEqual(this.advHiddenItems, hiddenItems)) {
      this.advHiddenItems = hiddenItems;
      hiddenChanged = true;
    }
    if (this.advTitleRenderWidth !== titleWidth) {
      this.advTitleRenderWidth = titleWidth;
    }
    if (this.advMainFlowOffset !== mainOffset) {
      this.advMainFlowOffset = mainOffset;
    }
    if (hiddenChanged) {
      this.advVisibilityRevision++;
    }
  }

  private commitAdvVisibility(ordered: Array<AdvPriorityEntry>, hidden: Set<AdvancedToolbarV2Item>,
    metrics: AdvZoneMetrics, titleWidth: number): void {
    const finalHidden: AdvancedToolbarV2Item[] = [];
    for (let i = 0; i < ordered.length; i++) {
      if (hidden.has(ordered[i].item)) {
        finalHidden.push(ordered[i].item);
      }
    }
    const budget = this.getAdvMainAxisBudget();
    const mainOffset = this.computeAdvMainOffset(metrics, budget);
    const mainClamped = this.isAdvMainOffsetClamped(metrics, budget, mainOffset);
    this.applyAdvVisibilityState(finalHidden, titleWidth, mainOffset, mainClamped);
  }


  private computeAdvVisibility(): void {
    if (this.advIsMeasuring || this.advIsComputingVisibility) {
      return;
    }
    const budget = this.getAdvMainAxisBudget();
    if (budget <= 0) {
      return;
    }
    this.advIsComputingVisibility = true;
    try {
      const ordered = this.getAdvFoldOrder(this.collectAdvPriorityEntries());
      const hidden: Set<AdvancedToolbarV2Item> = new Set();
      for (let i = 0; i < ordered.length; i++) {
        if (ordered[i].priority <= DISPLAY_PRIORITY_MIN) {
          hidden.add(ordered[i].item);
        }
      }
      let titleTextWidth = 0;
      let metrics = this.computeAdvZoneWidths(hidden, 0);
      if (this.hasAdvTitle()) {
        // 标题优先：先把标题压到剩余空间，只有标题到 minLength 还放不下时才收纳 item
        const minLength = this.getEffectiveTitleMinLength();
        const base = this.computeAdvZoneWidths(hidden, 0);
        const otherWidth = base.leading + base.main + base.trailing +
          this.computeAdvGapCount(base) * DEFAULT_ZONE_GAP;
        const available = Math.max(0, budget - otherWidth);
        const natural = this.advTitleNaturalWidth > 0 ? this.advTitleNaturalWidth : available;
        titleTextWidth = Math.max(minLength, Math.min(natural, available));
        metrics = this.computeAdvZoneWidths(hidden, titleTextWidth);
      }
      let cursor = 0;
      while (!this.computeAdvIsFeasible(metrics, budget)) {
        while (cursor < ordered.length &&
          (ordered[cursor].priority <= DISPLAY_PRIORITY_MIN || hidden.has(ordered[cursor].item))) {
          cursor++;
        }
        if (cursor >= ordered.length) {
          break;
        }
        hidden.add(ordered[cursor].item);
        cursor++;
        metrics = this.computeAdvZoneWidths(hidden, titleTextWidth);
      }
      this.commitAdvVisibility(ordered, hidden, metrics, titleTextWidth);
    } finally {
      this.advIsComputingVisibility = false;
    }
  }

  private getAdvZoneJustifyContent(placement: ToolbarV2ItemPlacement): FlexAlign {
    if (placement === ToolbarV2ItemPlacement.LEADING) {
      return FlexAlign.Start;
    }
    if (placement === ToolbarV2ItemPlacement.TRAILING) {
      return FlexAlign.End;
    }
    return this.advMainClamped ? FlexAlign.Start : FlexAlign.Center;
  }

  private getAdvZonePadding(placement: ToolbarV2ItemPlacement): LocalizedPadding {
    const padding: LocalizedPadding = {};
    if (placement === ToolbarV2ItemPlacement.MAIN && this.advMainClamped && this.advMainFlowOffset > 0) {
      if (this.getEffectiveDirection() === Axis.Vertical) {
        padding.top = LengthMetrics.vp(this.advMainFlowOffset);
      } else {
        padding.start = LengthMetrics.vp(this.advMainFlowOffset);
      }
    }
    return padding;
  }

  private getOverflowMenuStyle(): ToolbarV2OverflowMenuStyle | undefined {
    const style = this.advancedToolBarModifier?.overflowMenuStyleValue;
    if (!style || typeof style !== 'object') {
      return undefined;
    }
    return style;
  }

  private getOverflowMenuFont(): Font {
    const font = this.getOverflowMenuStyle()?.font;
    if (font && typeof font === 'object') {
      const keys: string[] = Object.keys(font);
      if (keys.length > 0) {
        return font;
      }
    }
    return { weight: FontWeight.Medium };
  }

  private getOverflowMenuFontColor(): ResourceColor {
    const colorMetrics = this.getOverflowMenuStyle()?.fontColor;
    if (!colorMetrics || typeof colorMetrics !== 'object') {
      return $r('sys.color.font_primary');
    }
    const color = (colorMetrics as ColorMetrics).color;
    if (color === undefined || color === null) {
      return $r('sys.color.font_primary');
    }
    return color;
  }

  private getOverflowMenuRadius(): Dimension | BorderRadiuses {
    const radius = this.getOverflowMenuStyle()?.radius;
    if (radius === undefined || radius === null) {
      return DEFAULT_OVERFLOW_MENU_RADIUS;
    }
    if (typeof radius === 'number') {
      return radius >= 0 ? radius as Dimension : DEFAULT_OVERFLOW_MENU_RADIUS;
    }
    if (typeof radius === 'string') {
      return radius as Dimension;
    }
    if (typeof radius === 'object') {
      return radius as BorderRadiuses;
    }
    return DEFAULT_OVERFLOW_MENU_RADIUS;
  }

  private isValidDivider(divider: DividerStyleOptions | undefined): boolean {
    if (!divider || typeof divider !== 'object') {
      return false;
    }
    const strokeWidth = divider.strokeWidth;
    if (strokeWidth !== undefined && strokeWidth !== null &&
      typeof strokeWidth !== 'number' && typeof strokeWidth !== 'string' && typeof strokeWidth !== 'object') {
      return false;
    }
    const startMargin = divider.startMargin;
    if (startMargin !== undefined && startMargin !== null &&
      typeof startMargin !== 'number' && typeof startMargin !== 'string' && typeof startMargin !== 'object') {
      return false;
    }
    const endMargin = divider.endMargin;
    if (endMargin !== undefined && endMargin !== null &&
      typeof endMargin !== 'number' && typeof endMargin !== 'string' && typeof endMargin !== 'object') {
      return false;
    }
    return true;
  }

  private getOverflowMenuItemDivider(): DividerStyleOptions | undefined {
    const divider = this.getOverflowMenuStyle()?.menuItemDivider;
    if (this.isValidDivider(divider)) {
      return divider;
    }
    return {
      strokeWidth: LengthMetrics.vp(0.5),
      startMargin: LengthMetrics.vp(0),
      endMargin: LengthMetrics.vp(0),
      color: $r('sys.color.comp_divider'),
    };
  }

  private getOverflowMenuGroupDivider(): DividerStyleOptions | undefined {
    const divider = this.getOverflowMenuStyle()?.menuItemGroupDivider;
    return this.isValidDivider(divider) ? divider : undefined;
  }

  private getAdvOverflowMenuItems(): AdvancedToolbarV2Item[] {
    const result: AdvancedToolbarV2Item[] = [];
    const seen: Set<AdvancedToolbarV2Item> = new Set();
    for (let i = 0; i < this.advHiddenItems.length; i++) {
      const item = this.advHiddenItems[i];
      if (!seen.has(item)) {
        seen.add(item);
        result.push(item);
      }
    }
    result.sort((a: AdvancedToolbarV2Item, b: AdvancedToolbarV2Item) => {
      const priorityA = this.normalizeDisplayPriority(a);
      const priorityB = this.normalizeDisplayPriority(b);
      if (priorityA !== priorityB) {
        return priorityB - priorityA;
      }
      return this.computeFlatIndex(a) - this.computeFlatIndex(b);
    });
    return result;
  }

  private getOverflowMenuItemLabel(item: AdvancedToolbarV2Item): ResourceStr {
    const text: ResourceStr | undefined = item.content?.text;
    if (text !== undefined && text !== null) {
      return text;
    }
    return '';
  }

  private getOverflowMenuItemIcon(item: AdvancedToolbarV2Item): ResourceStr | undefined {
    const icon = item.icon;
    if (!icon || icon instanceof ToolBarV2SymbolGlyph) {
      return undefined;
    }
    return (icon as ToolBarV2ItemImage).src;
  }

  private getOverflowMenuItemKey(item: AdvancedToolbarV2Item): string {
    // 菜单项用声明序号做稳定 key（id 允许重复，不能单独当 key）
    const index = this.computeFlatIndex(item);
    if (index >= 0) {
      return `overflow_item_${index}`;
    }
    return `overflow_item_${item.id ?? 'unknown'}`;
  }

  private onAdvContentMainAxisSizeChange(newValue: SizeOptions): void {
    if (this.getEffectiveDirection() === Axis.Vertical) {
      this.advMainAxisSize = Number(newValue.height) || 0;
    } else {
      this.advMainAxisSize = Number(newValue.width) || 0;
    }
    this.computeAdvVisibility();
  }

  private getEffectiveItemBorderRadius(inGroup: boolean, item?: AdvancedToolbarV2Item): Length {
    if (inGroup) {
      if (item && this.isItemActivated(item) && this.isItemOnlyIcon(item) && this.hasActivatedBg(item)) {
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

  private getEffectiveItemInnerWidth(item: AdvancedToolbarV2Item | null, inGroup: boolean): LengthMetrics {
    return this.getEffectiveItemWidth(item ?? undefined, inGroup);
  }

  private getEffectiveItemInnerHeight(item: AdvancedToolbarV2Item | null, inGroup: boolean): LengthMetrics {
    return this.getEffectiveItemHeight(item ?? undefined, inGroup);
  }

  private paddingEdgeVp(edge: Length | undefined): number {
    if (edge === undefined || edge === null) {
      return 0;
    }
    return Number(edge);
  }

  private getEffectiveGroupSpace(group: ToolbarV2ItemGroup): number {
    const raw = group.space ? Util.lengthMetricsToVp(group.space) :
      (group.style?.space ? Util.lengthMetricsToVp(group.style.space) : DEFAULT_ITEM_SPACE);
    return Math.max(0, raw);
  }

  build() {
    if (this.isAdvancedMode) {
      Column() {
        this.advToolbarContent()
      }
    } else {
      Column() {
        Divider()
          .width('100%').height(1)
          .attributeModifier(this.dividerModifier)
        Row() {
          ForEach(this.toolBarList, (item: ToolBarV2Item, index: number) => {
            if (this.toolBarList.length <= TOOLBAR_MAX_LENGTH || index < TOOLBAR_MAX_LENGTH - 1) {
              Row() {
                this.TabBuilder(index);
              }
              .height('100%')
              .flexShrink(1)
            }
          }, (item: ToolBarV2Item, index: number) => {
            return `${this.getUniqueId}__${index}}`;
          })
          if (this.toolBarList.length > TOOLBAR_MAX_LENGTH) {
            Row() {
              this.MoreTabBuilder(TOOLBAR_MAX_LENGTH - 1);
            }
            .height('100%')
            .flexShrink(1)
          }
        }
        .justifyContent(FlexAlign.Center)
        .constraintSize({
          minHeight: this.toLengthString(this.toolBarModifier?.heightValue),
          maxHeight: this.toLengthString(this.toolBarModifier?.heightValue),
        })
        .width('100%')
        .height(this.toLengthString(this.toolBarModifier?.heightValue))
        .padding({
          start: this.toolBarList.length < TOOLBAR_MAX_LENGTH ?
            this.toolBarModifier?.paddingValue : LengthMetrics.resource($r('sys.float.padding_level0')),
          end: this.toolBarList.length < TOOLBAR_MAX_LENGTH ?
            this.toolBarModifier?.paddingValue : LengthMetrics.resource($r('sys.float.padding_level0')),
        })
      }
      .attributeModifier(this.toolBarModifier)
    }
  }
}
