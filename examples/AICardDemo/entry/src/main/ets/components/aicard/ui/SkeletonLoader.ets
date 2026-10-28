import { LengthMetrics } from '@kit.ArkUI';
import { AiCardConstants } from '../common/AiCardConstants';

@Component
export struct SkeletonLoader {
  @State rowList: number[] = [];
  onSkeletonShown: () => void = () => {
  };
  onWidthChanged: (width: number) => void = () => {
  };

  aboutToAppear() {
    const arr: number[] = [];
    for (let i = 0; i < AiCardConstants.SKELETON_ROWS; i++) {
      arr.push(i);
    }
    this.rowList = arr;
  }

  build() {
    Column({ space: AiCardConstants.SPACING_GAP }) {
      ForEach(this.rowList, (r: number) => {
        Stack() {
        }
        .width('100%')
        .height(AiCardConstants.SKELETON_ROW_HEIGHT)
        .borderRadius(AiCardConstants.SKELETON_RADIUS)
        .backgroundColor(AiCardConstants.COLOR_SKELETON);
      }, (r: number) => r.toString())
    }
    .width('100%')
    .padding({ start: LengthMetrics.vp(AiCardConstants.SPACING_PADDING), end: LengthMetrics.vp(AiCardConstants.SPACING_PADDING) })
    .alignItems(HorizontalAlign.Start)
    .onAppear(() => {
      this.onSkeletonShown();
    })
    .onAreaChange((_: Area, newArea: Area) => {
      this.onWidthChanged(newArea.width as number);
    });
  }
}
