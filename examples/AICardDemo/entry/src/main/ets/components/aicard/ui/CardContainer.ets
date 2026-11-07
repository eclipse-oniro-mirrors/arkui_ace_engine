import { LengthMetrics } from '@kit.ArkUI';
import { AiCardConstants } from '../common/AiCardConstants';
import { AiSearchCardDataPayload, AiSearchCardRawData } from '../common/AiCardTypes';

@Component
export struct CardContainer {
  @Prop visible: boolean = false;
  @Prop cardList: AiSearchCardRawData[] = [];
  @State private cardOpacity: number = 0;
  @BuilderParam cardContent: (data: AiSearchCardDataPayload, index: number) => void = this.DefaultCard;
  onFinish: () => void = () => {
  };

  @Builder
  DefaultCard(_: AiSearchCardDataPayload, _index: number) {
  }

  build() {
    if (this.visible && this.cardList.length > 0) {
      Column() {
        this.cardContent({ streamingText: '', cards: this.cardList }, 0);
      }
      .width('100%')
      .padding({ start: LengthMetrics.vp(AiCardConstants.SPACING_PADDING), end: LengthMetrics.vp(AiCardConstants.SPACING_PADDING) })
      .backgroundColor(AiCardConstants.COLOR_CARD_BG)
      .borderRadius(AiCardConstants.ICON_PLACEHOLDER_RADIUS)
      .opacity(this.cardOpacity)
      .onAppear(() => {
        this.getUIContext()?.animateTo({
          duration: AiCardConstants.CARD_ANIM_DURATION_MS,
          onFinish: () => this.onFinish()
        }, () => {
          this.cardOpacity = 1;
        });
      });
    }
  }
}
