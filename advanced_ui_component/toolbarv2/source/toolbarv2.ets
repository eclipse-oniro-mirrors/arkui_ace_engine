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

export enum BarBackgroundVisibility {
  VISIBLE = 0,
  HIDDEN = 1,
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
const DISPLAY_PRIORITY_MIN: number = 0;
const DISPLAY_PRIORITY_MAX: number = 10;
const DEFAULT_ITEM_CROSS_AXIS: number = 48;
const DEFAULT_MAIN_AXIS_PADDING: number = 4;
const GROUP_MAIN_AXIS_PADDING: number = 12;
const DEFAULT_STYLE_STATE_EFFECT: boolean = true;
const GROUP_DIVIDER_LENGTH: number = 18;

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
  public backgroundVisibility?: BarBackgroundVisibility;
  @Trace
  public backgroundColor?: ColorMetrics;
  @Trace
  public backgroundBlurStyle?: BlurStyle;
  @Trace
  public systemBackgroundMaterial?: uiMaterial.Material;
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
}

export interface ToolbarV2StyleOptions {
  padding?: ToolbarV2PaddingOptions;
  backgroundVisibility?: BarBackgroundVisibility;
  backgroundColor?: ColorMetrics;
  backgroundBlurStyle?: BlurStyle;
  systemBackgroundMaterial?: uiMaterial.Material;
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
        .accessibilityLevel(item.accessibilityLevel)
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
        .accessibilityLevel(item.accessibilityLevel)
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
      .accessibilityLevel(item.accessibilityLevel)
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
            this.renderAdvItem(item, true);
            if (item.isShowSeparator === true) {
              Divider()
                .width(GROUP_DIVIDER_LENGTH)
                .color($r('sys.color.comp_divider'))
            }
          }, (item: AdvancedToolbarV2Item, index: number) => this.getGroupItemKey(item, index))
        }
      }
      .width(this.toLengthString(this.getEffectiveCrossAxisLength()))
      .alignItems(HorizontalAlign.Center)
      .backgroundColor(this.getEffectiveGroupBg(group))
      .borderRadius(this.getEffectiveGroupBorderRadius(group))
      .borderWidth(this.getEffectiveGroupBorderWidth(group))
      .borderColor(this.getEffectiveGroupBorderColor(group))
      .padding(this.getEffectiveGroupPadding(group))
      .clip(true)
    } else {
      Row() {
        Row({ space: this.getEffectiveGroupSpace(group) }) {
          ForEach(group.items, (item: AdvancedToolbarV2Item, index: number) => {
            this.renderAdvItem(item, true);
            if (item.isShowSeparator === true) {
              Divider()
                .height(GROUP_DIVIDER_LENGTH)
                .vertical(true)
                .color($r('sys.color.comp_divider'))
            }
          }, (item: AdvancedToolbarV2Item, index: number) => this.getGroupItemKey(item, index))
        }
      }
      .height(this.toLengthString(this.getEffectiveCrossAxisLength()))
      .alignItems(VerticalAlign.Center)
      .backgroundColor(this.getEffectiveGroupBg(group))
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
      this.renderAdvItemGroup(item as ToolbarV2ItemGroup);
    } else {
      this.renderAdvSingleItem(item as AdvancedToolbarV2Item);
    }
  }

  @Builder
  renderAdvTitle(): void {
    if (this.advTitleConfigs && (this.advTitleConfigs.visible ?? true)) {
      Column() {
        Text(this.advTitleConfigs.content)
          .fontSize(this.getEffectiveTitleFontSize())
          .fontColor(this.advTitleConfigs.style?.fontColor?.color ?? this.theme.fontPrimaryColor)
          .fontWeight(this.getEffectiveTitleFontWeight())
          .constraintSize({ minWidth: this.getEffectiveTitleMinLength() })
      }
      .justifyContent(FlexAlign.Center)
      .alignItems(HorizontalAlign.Center)
      .padding({
        start: LengthMetrics.vp(ITEM_PADDING),
        end: LengthMetrics.vp(ITEM_PADDING),
        top: LengthMetrics.vp(ITEM_PADDING),
        bottom: LengthMetrics.vp(ITEM_PADDING),
      })
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
          `toolbar_${placement}_${this.getEntryKey(item, index)}`)
      }
      .width('100%')
      .height('100%')
      .justifyContent(
        placement === ToolbarV2ItemPlacement.LEADING ? FlexAlign.Start
          : placement === ToolbarV2ItemPlacement.MAIN ? FlexAlign.Center
          : FlexAlign.End
      )
      .alignItems(HorizontalAlign.Center)
    } else {
      Row({ space: this.getEffectiveSpacing().value }) {
        if ((this.advTitleConfigs?.placement ?? ToolbarV2ItemPlacement.LEADING) === placement) {
          this.renderAdvTitle();
        }
        ForEach(this.getItemsByPlacement(placement),
          (item: AdvancedToolbarV2Item | ToolbarV2ItemGroup, index: number) => {
            this.renderAdvItemOrGroup(item);
          }, (item: AdvancedToolbarV2Item | ToolbarV2ItemGroup, index: number) =>
          `toolbar_${placement}_${this.getEntryKey(item, index)}`)
      }
      .width('100%')
      .height('100%')
      .justifyContent(
        placement === ToolbarV2ItemPlacement.LEADING ? FlexAlign.Start
          : placement === ToolbarV2ItemPlacement.MAIN ? FlexAlign.Center
          : FlexAlign.End
      )
      .alignItems(VerticalAlign.Center)
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
    return item.accessibilityText as ESObject as string;
  }

  private getAdvAccessibilityDescription(item: AdvancedToolbarV2Item): string | undefined {
    return item.accessibilityDescription as ESObject as string;
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
    this.buildAdvEntries();
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
    if (group?.style?.backgroundColor) {
      return group.style.backgroundColor.color;
    }
    const style = this.advancedToolBarModifier?.styleConfigsValue;
    if (style?.backgroundColor) {
      return style.backgroundColor.color;
    }
    return $r('sys.color.comp_background_primary');
  }

  private getEffectiveBackgroundColor(): ResourceColor {
    return Color.Transparent;
  }

  private getEffectiveBackgroundBlurStyle(): BlurStyle {
    return this.advancedToolBarModifier?.styleConfigsValue?.backgroundBlurStyle ?? BlurStyle.NONE;
  }

  private getEffectiveSystemBackgroundMaterial(): uiMaterial.Material | undefined {
    return this.advancedToolBarModifier?.styleConfigsValue?.systemBackgroundMaterial;
  }

  private getEffectiveBackgroundVisibility(): BarBackgroundVisibility {
    return this.advancedToolBarModifier?.styleConfigsValue?.backgroundVisibility ?? BarBackgroundVisibility.VISIBLE;
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

  private computeAdvVisibility(): void {
    if (this.advIsMeasuring) {
      return;
    }
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
