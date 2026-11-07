import { AiCardConstants } from '../common/AiCardConstants';
import { CardRecommendChip, CardRecommendSegment, RecommendChipSegmentType } from '../common/AiCardTypes';
import { LengthMetrics } from '@kit.ArkUI';

@Component
export struct RecommendZone {
  @Prop title: ResourceStr = AiCardConstants.TEXT_RECOMMEND_TITLE;
  @Prop chips: CardRecommendChip[] = [];
  @Prop maxChips: number = AiCardConstants.RECOMMEND_CHIP_MAX_DEFAULT;
  @Prop showRecommend: boolean = true;
  @Prop visible: boolean = false;
  @Prop fontSizeScale: number = 1;
  @State private recommendOpacity: number = 0;
  @State private recommendOffsetY: number = AiCardConstants.RECOMMEND_ANIM_OFFSET_Y;
  onRecommendAnimEnd: () => void = () => {
  };
  onRecommendClick: (index: number, content: string) => void = (index: number, content: string) => {
  };

  private getDisplayChips(): CardRecommendChip[] {
    const all: CardRecommendChip[] = this.chips;
    const max: number = this.maxChips > 0 ? this.maxChips : AiCardConstants.RECOMMEND_CHIP_MAX_DEFAULT;
    if (all.length > max) {
      return all.slice(0, max);
    }
    return all;
  }

  private getChipPaddingVertical(): number {
    if (this.fontSizeScale >= AiCardConstants.FONT_SCALE_THRESHOLD_LARGE) {
      return AiCardConstants.RECOMMEND_CHIP_PADDING_V_LARGE;
    }
    if (this.fontSizeScale >= AiCardConstants.FONT_SCALE_THRESHOLD_MEDIUM) {
      return AiCardConstants.RECOMMEND_CHIP_PADDING_V_MEDIUM;
    }
    return AiCardConstants.RECOMMEND_CHIP_PADDING_V_SMALL;
  }

  private getSegmentKey(segment: CardRecommendSegment, index: number): string {
    return `${index}_${segment.type}`;
  }

  private getChipTextContent(chip: CardRecommendChip): string {
    const parts: string[] = [];
    const segments: CardRecommendSegment[] = chip.segments;
    for (let i: number = 0; i < segments.length; i++) {
      if (segments[i].type === RecommendChipSegmentType.TEXT) {
        const text: ResourceStr | undefined = segments[i].text;
        if (text !== undefined) {
          parts.push(typeof text === 'string' ? text : '');
        }
      }
    }
    return parts.join('');
  }

  @Builder
  ChipItem(chip: CardRecommendChip, chipIndex: number) {
    Button({ type: ButtonType.ROUNDED_RECTANGLE }) {
      Text() {
        ForEach(chip.segments, (segment: CardRecommendSegment, index: number) => {
          if (segment.type === RecommendChipSegmentType.TEXT) {
            Span(segment.text ?? '')
              .fontSize($r('sys.float.Body_M'))
              .fontWeight(FontWeight.Regular)
              .fontColor($r('sys.color.font_primary'));
          } else {
            if (index > 0) {
              Span(' ')
                .fontSize($r('sys.float.Body_M'));
            }
            SymbolSpan(segment.symbol!)
              .fontSize(`${AiCardConstants.RECOMMEND_ICON_SIZE}fp`)
              .fontColor([$r('sys.color.font_primary')]);
            if (index < chip.segments.length - 1) {
              Span(' ')
                .fontSize($r('sys.float.Body_M'));
            }
          }
        }, (segment: CardRecommendSegment, index: number) => this.getSegmentKey(segment, index))
      }
      .maxLines(1)
      .textOverflow({ overflow: TextOverflow.Ellipsis });
    }
    .backgroundColor($r('sys.color.comp_background_tertiary'))
    .constraintSize({ maxWidth: '100%' })
    .padding({
      start: LengthMetrics.vp(AiCardConstants.SPACING_PADDING),
      end: LengthMetrics.vp(AiCardConstants.SPACING_PADDING),
      top: LengthMetrics.vp(this.getChipPaddingVertical()),
      bottom: LengthMetrics.vp(this.getChipPaddingVertical())
    })
    .onClick(() => {
      this.onRecommendClick(chipIndex, this.getChipTextContent(chip));
    });
  }

  build() {
    if (this.visible && this.showRecommend && this.getDisplayChips().length > 0) {
      Column() {
        Column() {
          Text(this.title)
            .fontSize($r('sys.float.Body_M'))
            .fontWeight(FontWeight.Regular)
            .fontColor($r('sys.color.font_secondary'));

          Flex({
            wrap: FlexWrap.Wrap,
            space: {
              main: LengthMetrics.vp(AiCardConstants.SPACING_GAP),
              cross: LengthMetrics.vp(AiCardConstants.SPACING_GAP)
            }
          }) {
            ForEach(this.getDisplayChips(), (chip: CardRecommendChip, index: number) => {
              this.ChipItem(chip, index);
            }, (chip: CardRecommendChip, index: number) => `${index}`)
          }
          .margin({ top: AiCardConstants.RECOMMEND_SPACING_TITLE_CHIPS })
        }
        .width('100%')
        .alignItems(HorizontalAlign.Start)
        .padding({ start: LengthMetrics.vp(AiCardConstants.SPACING_PADDING), end: LengthMetrics.vp(AiCardConstants.SPACING_PADDING), bottom: LengthMetrics.vp(AiCardConstants.RECOMMEND_BOTTOM_PADDING) })
        .translate({ y: this.recommendOffsetY })
        .opacity(this.recommendOpacity)
        .onAppear(() => {
          this.getUIContext()?.animateTo({
            duration: AiCardConstants.RECOMMEND_ANIM_DURATION_MS,
            onFinish: () => this.onRecommendAnimEnd()
          }, () => {
            this.recommendOpacity = 1;
            this.recommendOffsetY = 0;
          });
        });
      }
      .width('100%')
      .clip(true);
    }
  }
}
