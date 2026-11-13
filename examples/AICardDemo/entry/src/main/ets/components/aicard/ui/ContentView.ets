import { LengthMetrics } from '@kit.ArkUI';
import { AiCardConstants } from '../common/AiCardConstants';
import { AiSearchCardDataPayload, AiSearchCardRawData, CollapseMode } from '../common/AiCardTypes';
import { MarkdownRender } from './MarkdownRender';
import { CardContainer } from './CardContainer';

@Component
export struct ContentView {
  @Prop markdownText: string = '';
  @Prop showMarkdown: boolean = true;
  @Prop collapseMode: CollapseMode = CollapseMode.WHOLE_COMPONENT;
  @Prop cardVisible: boolean = false;
  @Prop cardList: AiSearchCardRawData[] = [];
  @Prop isCollapsed: boolean = false;
  @Prop userExpanded: boolean = false;
  @Prop iconBelowText: boolean = false;
  @Prop iconAtStart: boolean = false;
  @Prop fontSizeScale: number = 1;
  @State contentCollapsed: boolean = false;
  @State contentUserToggled: boolean = false;
  @State contentHeight: number = 0;
  @State toggleOpacity: number = 0;
  @State toggleEndNotified: boolean = false;
  onCardFinish: () => void = () => {};
  onExpandToggle: () => void = () => {};

  private hasToggleButton(): boolean {
    return this.collapseMode === CollapseMode.WHOLE_COMPONENT &&
      this.contentHeight > AiCardConstants.CONTENT_COLLAPSE_HEIGHT;
  }

  private getTogglePaddingVertical(): number {
    if (this.fontSizeScale >= AiCardConstants.FONT_SCALE_THRESHOLD_LARGE) {
      return AiCardConstants.CARD_TOGGLE_BUTTON_PADDING_V_LARGE;
    }
    if (this.fontSizeScale >= AiCardConstants.FONT_SCALE_THRESHOLD_MEDIUM) {
      return AiCardConstants.CARD_TOGGLE_BUTTON_PADDING_V_MEDIUM;
    }
    return AiCardConstants.CARD_TOGGLE_BUTTON_PADDING_V_SMALL;
  }

  private getMarkdownBottomPadding(): number {
    if (this.fontSizeScale >= AiCardConstants.FONT_SCALE_THRESHOLD_LARGE) {
      return AiCardConstants.MARKDOWN_BOTTOM_PADDING_FONTSCALE_LARGE;
    }
    if (this.fontSizeScale >= AiCardConstants.FONT_SCALE_THRESHOLD_MEDIUM) {
      return AiCardConstants.MARKDOWN_BOTTOM_PADDING_FONTSCALE_MEDIUM;
    }
    return AiCardConstants.MARKDOWN_BOTTOM_PADDING;
  }

  private getMarkdownDirection(): Direction {
    return this.iconAtStart ? Direction.Rtl : Direction.Ltr;
  }
  onToggleAnimationEnd: () => void = () => {};

  @BuilderParam cardContent: (data: AiSearchCardDataPayload, index: number) => void = this.DefaultCard;

  @Builder
  DefaultCard(_: AiSearchCardDataPayload, _index: number) {
  }

  @Builder
  MarkdownCollapseIcon() {
    Button() {
      SymbolGlyph(this.isCollapsed ? AiCardConstants.ICON_EXPAND : AiCardConstants.ICON_COLLAPSE)
        .fontSize(`${AiCardConstants.COLLAPSE_ICON_FONT_SIZE}vp`)
        .fontColor([AiCardConstants.COLOR_TEXT_SECONDARY])
    }
    .width(AiCardConstants.COLLAPSE_ICON_SIZE)
    .height(AiCardConstants.COLLAPSE_ICON_SIZE)
    .backgroundColor(AiCardConstants.COLLAPSE_BUTTON_BG)
    .borderRadius(AiCardConstants.COLLAPSE_BUTTON_RADIUS)
    .padding(0)
    .onClick(() => this.onExpandToggle());
  }

  @Builder
  ToggleButton() {
    Row() {
      Button({ type: ButtonType.ROUNDED_RECTANGLE }) {
        Text() {
          Span(this.contentCollapsed ? AiCardConstants.TEXT_CARD_EXPAND : AiCardConstants.TEXT_CARD_COLLAPSE)
            .fontSize($r('sys.float.Body_S'))
            .fontWeight(FontWeight.Regular)
            .fontColor($r('sys.color.icon_secondary'));
          SymbolSpan(this.contentCollapsed ? $r('sys.symbol.chevron_down') : $r('sys.symbol.chevron_up'))
            .fontSize(`${AiCardConstants.CARD_TOGGLE_ICON_SIZE}vp`)
            .fontColor([$r('sys.color.icon_secondary')]);
        }
        .textVerticalAlign(TextVerticalAlign.CENTER);
      }
      .padding({
        start: LengthMetrics.vp(AiCardConstants.SPACING_PADDING),
        end: LengthMetrics.vp(AiCardConstants.SPACING_PADDING),
        top: LengthMetrics.vp(this.getTogglePaddingVertical()),
        bottom: LengthMetrics.vp(this.getTogglePaddingVertical())
      })
      .backgroundColor(Color.Transparent)
      .opacity(this.toggleOpacity)
      .onClick(() => {
        this.contentUserToggled = true;
        this.contentCollapsed = !this.contentCollapsed;
      })
      .onAppear(() => {
        this.getUIContext()?.animateTo({
          duration: AiCardConstants.CARD_ANIM_DURATION_MS,
          onFinish: () => {
            this.toggleEndNotified = true;
            this.onToggleAnimationEnd();
          }
        }, () => {
          this.toggleOpacity = 1;
        });
      });
    }
    .padding({ top: AiCardConstants.CARD_TOGGLE_ROW_PADDING_VERTICAL, bottom: AiCardConstants.CARD_TOGGLE_ROW_PADDING_VERTICAL });
  }

  build() {
    Column() {
      Column() {
        if (this.showMarkdown && this.markdownText.length > 0) {
          if (this.collapseMode === CollapseMode.MARKDOWN_AREA) {
            if (this.isCollapsed || this.userExpanded) {
              if (this.userExpanded && this.iconBelowText) {
                Column() {
                  MarkdownRender({
                    text: this.markdownText
                  });
                  this.MarkdownCollapseIcon();
                }
                .width('100%')
                .alignItems(HorizontalAlign.End)
                .direction(this.getMarkdownDirection())
                .padding({ bottom: this.getMarkdownBottomPadding() });
              } else {
                Stack({ alignContent: Alignment.BottomEnd }) {
                  MarkdownRender({
                    text: this.markdownText
                  });
                  this.MarkdownCollapseIcon();
                }
                .width('100%')
                .direction(this.getMarkdownDirection())
                .padding({ bottom: this.getMarkdownBottomPadding() });
              }
            } else {
              MarkdownRender({
                text: this.markdownText
              })
              .padding({ bottom: this.getMarkdownBottomPadding() });
            }
          } else {
            MarkdownRender({
              text: this.markdownText
            })
            .padding({ bottom: this.getMarkdownBottomPadding() });
          }
        }
        CardContainer({
          visible: this.cardVisible,
          cardList: this.cardList,
          cardContent: this.cardContent,
          onFinish: () => this.onCardFinish()
        })
          .padding({ bottom: AiCardConstants.CARD_BOTTOM_PADDING });
      }
      .width('100%')
      .clip(this.contentCollapsed && this.collapseMode === CollapseMode.WHOLE_COMPONENT)
      .constraintSize({ maxHeight: this.contentCollapsed && this.collapseMode === CollapseMode.WHOLE_COMPONENT ? AiCardConstants.CONTENT_COLLAPSE_HEIGHT : AiCardConstants.CONTENT_MAX_HEIGHT_UNLIMITED })
      .onAreaChange((_: Area, newArea: Area) => {
        if (this.collapseMode !== CollapseMode.WHOLE_COMPONENT) {
          return;
        }
        if (this.toggleEndNotified || this.contentUserToggled) {
          return;
        }
        if (!this.cardVisible) {
          return;
        }
        const h: number = newArea.height as number;
        if (this.contentCollapsed) {
          return;
        }
        if (h > this.contentHeight) {
          this.contentHeight = h;
          if (h > AiCardConstants.CONTENT_COLLAPSE_HEIGHT) {
            this.contentCollapsed = true;
          }
          return;
        }
        if (h <= this.contentHeight && h > 0 && this.contentHeight <= AiCardConstants.CONTENT_COLLAPSE_HEIGHT) {
          this.toggleEndNotified = true;
          this.onToggleAnimationEnd();
        }
      });

      if (this.hasToggleButton()) {
        this.ToggleButton();
      }
    }
    .width('100%')
    .padding({
      start: LengthMetrics.vp(AiCardConstants.SPACING_PADDING),
      end: LengthMetrics.vp(AiCardConstants.SPACING_PADDING)
    })
    .clip(true);
  }
}
