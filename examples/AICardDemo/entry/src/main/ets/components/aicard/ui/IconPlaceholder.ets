import { AiCardConstants } from '../common/AiCardConstants';

@Component
export struct IconPlaceholder {
  @Prop iconSize: number = AiCardConstants.ICON_PLACEHOLDER_SIZE;

  build() {
    Stack() {}
      .width(this.iconSize)
      .height(this.iconSize)
      .borderRadius(AiCardConstants.ICON_PLACEHOLDER_RADIUS)
      .backgroundColor(AiCardConstants.COLOR_ICON_PLACEHOLDER);
  }
}
