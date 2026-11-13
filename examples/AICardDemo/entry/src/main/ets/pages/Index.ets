import { AiCard } from '../components/aicard/AiCard';
import { AiCardController } from '../components/aicard/controller/AiCardController';
import {
  AiCardDisplayConfig,
  AiCardLifecycleCallbacks,
  CardRecommendConfig,
  AiCardStreamConfig,
  AiSearchCardDataPayload,
  AiSearchCardRawData,
  AiSearchComplianceConfig,
  AiSearchDislikeFeedback,
  AiSearchError,
  CardCancelPhase,
  RecommendChipSegmentType,
  CollapseMode,
  SearchProviderType
} from '../components/aicard/common/AiCardTypes';

@Entry
@Component
struct Index {
  @State query: string = '你好';
  @State sessionKey: number = 0;
  @State showCard: boolean = false;
  private aiController: AiCardController = new AiCardController();
  private streamCfg: AiCardStreamConfig = {
    showMarkdown: true
  };
  private displayCfg: AiCardDisplayConfig = {
    collapseMode: CollapseMode.WHOLE_COMPONENT
  };
  private recommendCfg: CardRecommendConfig = {
    showRecommend: true,
    maxChips: 3,
    chips: [
      {
        segments: [
          { type: RecommendChipSegmentType.SYMBOL, symbol: $r('sys.symbol.star') },
          { type: RecommendChipSegmentType.TEXT, text: 'ArkTS 有哪些特性' }
        ]
      },
      {
        segments: [
          { type: RecommendChipSegmentType.TEXT, text: '什么是' },
          { type: RecommendChipSegmentType.SYMBOL, symbol: $r('sys.symbol.ohos_lungs') },
          { type: RecommendChipSegmentType.TEXT, text: 'HarmonyOS' }
        ]
      },
      {
        segments: [
          { type: RecommendChipSegmentType.TEXT, text: '如何使用组件' },
          { type: RecommendChipSegmentType.SYMBOL, symbol: $r('sys.symbol.gearshape') }
        ]
      }
    ],
    onRecommendClick: (index: number, content: string) => {
      console.info(`AiCard onRecommendClick: index=${index} content=${content}`);
      this.query = content;
    }
  };
  private complianceCfg: AiSearchComplianceConfig = {
    showCopy: true,
    showFeedbackButtons: true,
    onLike: (active: boolean) => {
      console.info(`AiCard onLike: ${active}`);
    },
    onCopy: (text: string) => {
      console.info(`AiCard onCopy: ${text.length} chars`);
    },
    onDislike: (feedback: AiSearchDislikeFeedback, active: boolean) => {
      console.info(`AiCard onDislike: ${active}, tags: ${feedback.tags.length}`);
    }
  };
  private lifecycleCallbacks: AiCardLifecycleCallbacks = {
    onAnimatedStart: () => {
      console.info('AiCard onAnimatedStart');
    },
    onAnimatedEnd: () => {
      console.info('AiCard onAnimatedEnd');
    },
    onCancel: (phase: CardCancelPhase) => {
      console.info(`AiCard onCancel: phase=${phase === CardCancelPhase.THINKING ? 'THINKING' : 'GENERATING'}`);
    }
  };
  private onCardError: (error: AiSearchError) => void = (e: AiSearchError) => {
    console.info(`AiCard onError: ${e.code}`);
  };

  startSession() {
    this.sessionKey++;
    this.showCard = true;
  }

  requestAgain() {
    this.aiController.request(this.query);
  }

  cancelGeneration() {
    this.aiController.cancel();
  }

  @Builder
  MyCardBuilder(data: AiSearchCardDataPayload, _index: number) {
    ForEach(data.cards, (card: AiSearchCardRawData, index: number) => {
      Row({ space: 8 }) {
        Text(card.title ?? '')
          .fontSize(14)
          .fontColor('#182431')
          .layoutWeight(1);
      }
      .width('100%');
    }, (card: AiSearchCardRawData, index: number) => `${index}_${card.title ?? ''}`)
  }

  build() {
    Scroll() {
      Column({ space: 8 }) {
        Row({ space: 8 }) {
          TextInput({ text: this.query })
            .layoutWeight(1)
            .fontSize(14)
            .onChange((v: string) => {
              this.query = v;
            });
          Text('开始')
            .fontSize(12)
            .fontColor('#182431')
            .padding(8)
            .backgroundColor($r('sys.color.comp_background_list_card'))
            .borderRadius(6)
            .onClick(() => this.startSession());

        }
        .width('100%')
        .padding(12);

        Row({ space: 8 }) {
          Text('重新搜索')
            .fontSize(12)
            .fontColor('#182431')
            .padding(8)
            .backgroundColor($r('sys.color.comp_background_list_card'))
            .borderRadius(6)
            .onClick(() => this.requestAgain());
          Text('取消')
            .fontSize(12)
            .fontColor('#182431')
            .padding(8)
            .backgroundColor($r('sys.color.comp_background_list_card'))
            .borderRadius(6)
            .onClick(() => this.cancelGeneration());
        }
        .width('100%')
        .padding({ left: 12, right: 12 });

        if (this.showCard) {
          WithTheme({ colorMode: ThemeColorMode.SYSTEM }) {
            Column() {
              ForEach([this.sessionKey], (_k: number) => {
                AiCard({
                  controller: this.aiController,
                  searchProviderType: SearchProviderType.TEST,
                  streamConfig: this.streamCfg,
                  displayConfig: this.displayCfg,
                  recommendConfig: this.recommendCfg,
                  complianceConfig: this.complianceCfg,
                  lifecycleCallbacks: this.lifecycleCallbacks,
                  onError: this.onCardError,
                  cardContent: this.MyCardBuilder
                })
                  .onAppear(() => {
                    this.requestAgain()
                  })
                  .margin({ left: 20, right: 20 })
              }, (k: number) => k.toString())
              // Text("卡死开大会大豪科技")
              //   .backgroundColor(Color.Orange)
            }
          }
        }
      }
      .width('100%')
    }

  }
}
