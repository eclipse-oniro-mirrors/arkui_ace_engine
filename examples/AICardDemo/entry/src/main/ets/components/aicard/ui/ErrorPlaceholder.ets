import { LengthMetrics } from '@kit.ArkUI';
import { AiCardConstants } from '../common/AiCardConstants';

@ComponentV2
export struct ErrorPlaceholder {
  @Param fontSizeScale: number = 1;
  @Param titlebarBottomGap: number = 0;

  private getPaddingVertical(): number {
    if (this.fontSizeScale >= AiCardConstants.FONT_SCALE_THRESHOLD_LARGE) {
      return AiCardConstants.CONTENT_ERROR_PADDING_V_LARGE;
    }
    return AiCardConstants.CONTENT_ERROR_PADDING_V_SMALL;
  }

  build() {
    Column() {
      Text($r('app.string.content_error_hint'))
        .fontSize(AiCardConstants.FONT_SIZE_BODY)
        .fontColor(AiCardConstants.COLOR_TEXT_SECONDARY)
        .width('100%')
        .textAlign(TextAlign.Center)
        .offset({ y: -this.titlebarBottomGap / 2 })
    }
    .width('100%')
    .constraintSize({ minHeight: AiCardConstants.SKELETON_CONTENT_HEIGHT })
    .padding({
      start: LengthMetrics.vp(AiCardConstants.SPACING_PADDING),
      end: LengthMetrics.vp(AiCardConstants.SPACING_PADDING),
      top: LengthMetrics.vp(this.getPaddingVertical()),
      bottom: LengthMetrics.vp(this.getPaddingVertical())
    })
    .justifyContent(FlexAlign.Center)
    .alignItems(HorizontalAlign.Center)
  }
}
