import { LengthMetrics } from '@kit.ArkUI';
import { i18n } from '@kit.LocalizationKit';
import { common, Configuration, EnvironmentCallback, AbilityConstant } from '@kit.AbilityKit';
import { AiCardConstants } from '../common/AiCardConstants';

@Component
export struct TitleBar {
  @Prop title: Resource = $r('app.string.titlebar_thinking');
  @Prop isTerminal: boolean = false;
  @Prop stoppedByUser: boolean = false;
  @Prop fontSizeScale: number = 1;
  @Prop titleClip: boolean = true;
  onStop: () => void = () => {};
  onBottomGapChanged: (gap: number) => void = () => {};
  @State useFourColorGradient: boolean = false;
  @State isRtl: boolean = false;
  private envCallbackId: number = -1;
  private envCallback: EnvironmentCallback | null = null;
  private appContext: common.ApplicationContext | null = null;
  private outerRowHeight: number = 0;
  private contentRowHeight: number = 0;
  private lastNotifiedGap: number = -1;

  private detectRtl(): void {
    try {
      const ctx = this.getUIContext()?.getHostContext();
      if (ctx !== undefined) {
        const locale: string = ctx.resourceManager.getConfigurationSync().locale;
        this.isRtl = i18n.isRTL(locale);
      }
    } catch (e) {
      this.isRtl = false;
    }
  }

  private registerEnvCallback(): void {
    if (this.envCallbackId !== -1) {
      return;
    }
    try {
      const ctx = this.getUIContext()?.getHostContext() as common.UIAbilityContext;
      if (ctx !== null && ctx !== undefined) {
        this.appContext = ctx.getApplicationContext();
        this.envCallback = {
          onConfigurationUpdated: (newConfig: Configuration): void => {
            const lang: string | undefined = newConfig.language;
            if (lang !== undefined) {
              this.isRtl = i18n.isRTL(lang);
            }
          },
          onMemoryLevel: (_level: AbilityConstant.MemoryLevel): void => {}
        };
        this.envCallbackId = this.appContext.on('environment', this.envCallback);
      }
    } catch (e) {
      this.envCallbackId = -1;
    }
  }

  aboutToDisappear(): void {
    if (this.envCallbackId !== -1 && this.appContext !== null) {
      try {
        this.appContext.off('environment', this.envCallbackId);
      } catch (e) {
      }
    }
    this.envCallbackId = -1;
    this.appContext = null;
    this.envCallback = null;
  }

  private getConcentricRadius(): number {
    return AiCardConstants.CARD_BORDER_RADIUS - AiCardConstants.BUTTON_RIGHT_MARGIN;
  }

  private getVerticalPadding(): number {
    if (this.fontSizeScale >= AiCardConstants.FONT_SCALE_THRESHOLD_LARGE) {
      return AiCardConstants.TITLEBAR_PADDING_FONTSCALE_LARGE;
    }
    if (this.fontSizeScale >= AiCardConstants.FONT_SCALE_THRESHOLD_MEDIUM) {
      return AiCardConstants.TITLEBAR_PADDING_FONTSCALE_MEDIUM;
    }
    return AiCardConstants.TITLEBAR_PADDING_FONTSCALE_SMALL;
  }

  private notifyBottomGap(): void {
    if (this.outerRowHeight <= 0 || this.contentRowHeight <= 0) {
      return;
    }
    const gap: number = (this.outerRowHeight - this.contentRowHeight) / 2;
    if (gap < 0) {
      return;
    }
    if (Math.abs(gap - this.lastNotifiedGap) < 0.01) {
      return;
    }
    this.lastNotifiedGap = gap;
    this.onBottomGapChanged(gap);
  }

  @Builder
  AiIconNode() {
    Image(AiCardConstants.AI_ICON_SEARCH_RES)
      .width(AiCardConstants.ICON_SIZE_AI)
      .height(AiCardConstants.ICON_SIZE_AI)
  }

  @Builder
  StopButton() {
    Button({ type: ButtonType.Normal }) {
      SymbolGlyph($r('sys.symbol.stop_circle_fill'))
        .fontSize(`${AiCardConstants.FONT_SIZE_STOP}vp`)
        .fontColor([$r('sys.color.icon_primary')])
    }
    .width(AiCardConstants.BUTTON_SIZE)
    .height(AiCardConstants.BUTTON_SIZE)
    .borderRadius(this.getConcentricRadius())
    .padding(0)
    .margin({ start: LengthMetrics.vp(AiCardConstants.SPACING_TITLE_BUTTON), end: LengthMetrics.vp(AiCardConstants.BUTTON_RIGHT_MARGIN) })
    .backgroundColor(Color.Transparent)
    .onClick(() => this.onStop())
  }

  build() {
    Row() {
      Row() {
        Stack() {
          Row({ space: AiCardConstants.SPACING_ICON_TITLE }) {
            this.AiIconNode()
            Text(this.title)
              .fontSize(AiCardConstants.TITLEBAR_FONT_SIZE_TITLE)
              .maxLines(1)
              .textOverflow({ overflow: this.titleClip ? TextOverflow.Clip : TextOverflow.Ellipsis })
          }
          .blendMode(BlendMode.DST_IN, BlendApplyType.OFFSCREEN)
          .onAreaChange((_oldArea: Area, newArea: Area) => {
            this.useFourColorGradient = newArea.width > AiCardConstants.TITLEBAR_GRADIENT_WIDTH_THRESHOLD;
            this.contentRowHeight = newArea.height as number;
            this.notifyBottomGap();
          })
        }
        .linearGradient({
          direction: this.isRtl ? GradientDirection.Left : GradientDirection.Right,
          colors: this.useFourColorGradient
            ? AiCardConstants.TITLEBAR_GRADIENT_COLORS_FOUR
            : AiCardConstants.TITLEBAR_GRADIENT_COLORS
        })
        .blendMode(BlendMode.SRC_OVER, BlendApplyType.OFFSCREEN)
      }
      .layoutWeight(1)

      if (!this.isTerminal && !this.stoppedByUser) {
        this.StopButton()
      }
    }
    .width('100%')
    .constraintSize({ minHeight: AiCardConstants.TITLEBAR_HEIGHT })
    .padding({
      start: LengthMetrics.vp(AiCardConstants.SPACING_RIGHT_MARGIN),
      top: LengthMetrics.vp(this.getVerticalPadding()),
      bottom: LengthMetrics.vp(this.getVerticalPadding())
    })
    .onAppear(() => {
      this.detectRtl();
      this.registerEnvCallback();
    })
    .onAreaChange((_oldArea: Area, newArea: Area) => {
      this.outerRowHeight = newArea.height as number;
      this.notifyBottomGap();
    })
    .alignItems(VerticalAlign.Center)
  }
}
