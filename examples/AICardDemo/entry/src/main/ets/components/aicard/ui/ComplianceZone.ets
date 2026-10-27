import { LengthMetrics } from '@kit.ArkUI';
import { AiCardConstants } from '../common/AiCardConstants';

@Component
export struct ComplianceZone {
  @Prop showCopy: boolean = true;
  @Prop showFeedbackButtons: boolean = true;
  @Prop markdownText: string = '';
  @Prop isLikeActive: boolean = false;
  @Prop isDislikeActive: boolean = false;
  @Prop interactive: boolean = true;
  @Prop fontSizeScale: number = 1;
  onCopy: () => void = () => {
  };
  onLikeToggle: () => void = () => {
  };
  onDislikeToggle: () => void = () => {
  };

  private hasFeedback(): boolean {
    return this.showCopy || this.showFeedbackButtons;
  }

  private getSpacingTop(): number {
    if (this.fontSizeScale >= AiCardConstants.FONT_SCALE_THRESHOLD_LARGE) {
      return AiCardConstants.COMPLIANCE_SPACING_FONTSCALE_LARGE;
    }
    if (this.fontSizeScale >= AiCardConstants.FONT_SCALE_THRESHOLD_MEDIUM) {
      return AiCardConstants.COMPLIANCE_SPACING_FONTSCALE_MEDIUM;
    }
    if (this.hasFeedback()) {
      return AiCardConstants.COMPLIANCE_SPACING_WITH_FEEDBACK;
    }
    return AiCardConstants.COMPLIANCE_SPACING_NO_FEEDBACK;
  }

  private getBottomPadding(): number {
    if (this.fontSizeScale >= AiCardConstants.FONT_SCALE_THRESHOLD_LARGE) {
      return AiCardConstants.COMPLIANCE_PADDING_FONTSCALE_LARGE;
    }
    if (this.fontSizeScale >= AiCardConstants.FONT_SCALE_THRESHOLD_MEDIUM) {
      return AiCardConstants.COMPLIANCE_PADDING_FONTSCALE_MEDIUM;
    }
    return AiCardConstants.SPACING_PADDING;
  }

  @Builder
  CopyButton() {
    Button({ type: ButtonType.Normal }) {
      SymbolGlyph($r('sys.symbol.plus_square_on_square'))
        .fontSize(`${AiCardConstants.COMPLIANCE_ICON_SIZE}vp`)
        .fontColor([$r('sys.color.icon_primary')])
    }
    .width(AiCardConstants.COMPLIANCE_ICON_HOT_SIZE)
    .height(AiCardConstants.COMPLIANCE_ICON_HOT_SIZE)
    .padding(0)
    .backgroundColor(AiCardConstants.COMPLIANCE_ICON_BG)
    .borderRadius(AiCardConstants.COMPLIANCE_ICON_BG_RADIUS)
    .enabled(this.interactive)
    .onClick(() => this.onCopy());
  }

  @Builder
  LikeButton() {
    Button({ type: ButtonType.Normal }) {
      SymbolGlyph(this.isLikeActive ? $r('sys.symbol.hand_thumbsup_fill') : $r('sys.symbol.hand_thumbsup'))
        .fontSize(`${AiCardConstants.COMPLIANCE_ICON_SIZE}vp`)
        .fontColor([this.isLikeActive ? $r('sys.color.icon_emphasize') : $r('sys.color.icon_primary')])
    }
    .width(AiCardConstants.COMPLIANCE_ICON_HOT_SIZE)
    .height(AiCardConstants.COMPLIANCE_ICON_HOT_SIZE)
    .padding(0)
    .backgroundColor(AiCardConstants.COMPLIANCE_ICON_BG)
    .borderRadius(AiCardConstants.COMPLIANCE_ICON_BG_RADIUS)
    .enabled(this.interactive)
    .onClick(() => this.onLikeToggle());
  }

  @Builder
  DislikeButton() {
    Button({ type: ButtonType.Normal }) {
      SymbolGlyph(this.isDislikeActive ? $r('sys.symbol.hand_thumbsdown_fill') : $r('sys.symbol.hand_thumbsdown'))
        .fontSize(`${AiCardConstants.COMPLIANCE_ICON_SIZE}vp`)
        .fontColor([this.isDislikeActive ? $r('sys.color.icon_emphasize') : $r('sys.color.icon_primary')])
    }
    .width(AiCardConstants.COMPLIANCE_ICON_HOT_SIZE)
    .height(AiCardConstants.COMPLIANCE_ICON_HOT_SIZE)
    .padding(0)
    .backgroundColor(AiCardConstants.COMPLIANCE_ICON_BG)
    .borderRadius(AiCardConstants.COMPLIANCE_ICON_BG_RADIUS)
    .enabled(this.interactive)
    .onClick(() => this.onDislikeToggle());
  }

  build() {
    Column() {
      Divider()
        .strokeWidth(AiCardConstants.COMPLIANCE_DIVIDER_WIDTH)
        .color($r('sys.color.comp_divider'));

      Row({ space: AiCardConstants.COMPLIANCE_ICON_GAP }) {
        Text(AiCardConstants.TEXT_COMPLIANCE)
          .fontSize($r('sys.float.Caption_M'))
          .fontWeight(FontWeight.Medium)
          .fontColor($r('sys.color.font_fourth'))
          .layoutWeight(1);

        if (this.showCopy) {
          this.CopyButton();
        }
        if (this.showFeedbackButtons) {
          this.LikeButton();
          this.DislikeButton();
        }
      }
      .width('100%')
      .margin({ top: this.getSpacingTop() });
    }
    .width('100%')
    .padding({ start: LengthMetrics.vp(AiCardConstants.SPACING_PADDING), end: LengthMetrics.vp(AiCardConstants.SPACING_PADDING), bottom: LengthMetrics.vp(this.getBottomPadding()) });
  }
}
