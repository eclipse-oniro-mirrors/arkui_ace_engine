import { LengthMetrics, SubHeader } from '@kit.ArkUI';
import { AiCardConstants } from '../common/AiCardConstants';
import { common } from '@kit.AbilityKit';

const DEFAULT_REASON_RESOURCES: Resource[] = [
  $r('app.string.compliance_dislike_reason_1'),
  $r('app.string.compliance_dislike_reason_2'),
  $r('app.string.compliance_dislike_reason_3'),
  $r('app.string.compliance_dislike_reason_4'),
  $r('app.string.compliance_dislike_reason_5'),
  $r('app.string.compliance_dislike_reason_6'),
  $r('app.string.compliance_dislike_reason_7')
];

const MAX_DESCRIPTION_LENGTH: number = AiCardConstants.DIALOG_DESC_MAX_LENGTH;

@Component
export struct DislikeDialog {
  @Prop dislikeTags: string[] = [];
  @Prop fontSizeScale: number = 1;
  @State selectedTags: boolean[] = [];
  @State description: string = '';
  @State reasonItems: ResourceStr[] = [];
  onSelectionChange: (hasSelection: boolean) => void = () => {
  };
  onConfirm: (selectedTags: string[], description: string) => void = () => {
  };
  onCancel: () => void = () => {
  };
  onClose: () => void = () => {
  };

  aboutToAppear() {
    const count: number = this.getReasonCount();
    const arr: boolean[] = [];
    for (let i = 0; i < count; i++) {
      arr.push(false);
    }
    this.selectedTags = arr;
    this.reasonItems = this.buildReasonItems();
  }

  private getReasonCount(): number {
    if (this.dislikeTags.length > 0) {
      return this.dislikeTags.length;
    }
    return DEFAULT_REASON_RESOURCES.length;
  }

  private buildReasonItems(): ResourceStr[] {
    if (this.dislikeTags.length > 0) {
      const result: ResourceStr[] = [];
      for (let i = 0; i < this.dislikeTags.length; i++) {
        result.push(this.dislikeTags[i]);
      }
      return result;
    }
    return DEFAULT_REASON_RESOURCES;
  }

  private getReasonItem(index: number): ResourceStr {
    if (index < this.reasonItems.length) {
      return this.reasonItems[index];
    }
    return '';
  }

  getSelectedTags(): string[] {
    const result: string[] = [];
    const selected: boolean[] = this.selectedTags;
    const ctx = getContext(this) as common.UIAbilityContext;
    const rm = ctx.resourceManager;
    for (let i = 0; i < selected.length; i++) {
      if (selected[i]) {
        const item: ResourceStr = this.getReasonItem(i);
        if (typeof item === 'string') {
          result.push(item);
        } else {
          result.push(rm.getStringSync((item as Resource).id));
        }
      }
    }
    return result;
  }

  hasSelectedTag(): boolean {
    const selected: boolean[] = this.selectedTags;
    for (let i = 0; i < selected.length; i++) {
      if (selected[i]) {
        return true;
      }
    }
    return false;
  }

  handleConfirm(): void {
    if (!this.hasSelectedTag()) {
      return;
    }
    this.onConfirm(this.getSelectedTags(), this.description);
    this.onClose();
  }

  handleCancel(): void {
    this.onCancel();
    this.onClose();
  }

  private toggleTag(index: number): void {
    const arr: boolean[] = this.selectedTags.slice();
    if (index < arr.length) {
      arr[index] = !arr[index];
    }
    this.selectedTags = arr;
    this.onSelectionChange(this.hasSelectedTag());
  }

  private getChipPaddingVertical(): number {
    if (this.fontSizeScale >= AiCardConstants.FONT_SCALE_THRESHOLD_LARGE) {
      return AiCardConstants.DIALOG_CHIP_PADDING_V_LARGE;
    }
    if (this.fontSizeScale >= AiCardConstants.FONT_SCALE_THRESHOLD_MEDIUM) {
      return AiCardConstants.DIALOG_CHIP_PADDING_V_MEDIUM;
    }
    return AiCardConstants.DIALOG_CHIP_PADDING_V_SMALL;
  }

  private buildRowIndices(): number[] {
    const count: number = this.getReasonCount();
    const rows: number = Math.ceil(count / AiCardConstants.DIALOG_COLUMNS_PER_ROW);
    const arr: number[] = [];
    for (let i = 0; i < rows; i++) {
      arr.push(i);
    }
    return arr;
  }

  private buildRowItems(row: number): number[] {
    const count: number = this.getReasonCount();
    const start: number = row * AiCardConstants.DIALOG_COLUMNS_PER_ROW;
    const items: number[] = [];
    for (let i = start; i < start + AiCardConstants.DIALOG_COLUMNS_PER_ROW && i < count; i++) {
      items.push(i);
    }
    return items;
  }

  build() {
    Column() {
      SubHeader({
        secondaryTitle: $r('app.string.compliance_dialog_feedback_title'),
        contentMargin: {
          start: LengthMetrics.vp(0),
          end: LengthMetrics.vp(0)
        },
        contentPadding: {
          start: LengthMetrics.vp(0),
          end: LengthMetrics.vp(0)
        }
      })
      .width('100%');

      Column() {
        ForEach(this.buildRowIndices(), (row: number) => {
          Row({ space: AiCardConstants.DIALOG_CHIP_COL_GAP }) {
            ForEach(this.buildRowItems(row), (idx: number) => {
              Button({ type: ButtonType.ROUNDED_RECTANGLE }) {
                Text(this.getReasonItem(idx))
                  .fontSize($r('sys.float.Body_S'))
                  .fontColor($r('sys.color.font_primary'));
              }
              .layoutWeight(1)
              .borderRadius($r('sys.float.corner_radius_level7'))
              .padding({
                top: this.getChipPaddingVertical(),
                bottom: this.getChipPaddingVertical()
              })
              .backgroundColor(
                this.selectedTags[idx] ? $r('sys.color.comp_emphasize_secondary') : $r('sys.color.comp_background_tertiary'))
              .onClick(() => this.toggleTag(idx));
            }, (idx: number) => idx.toString())

            if (this.buildRowItems(row).length === 1) {
              Column()
                .layoutWeight(1)
                .height(1);
            }
          }
          .width('100%')
          .margin({ top: AiCardConstants.DIALOG_CHIP_MARGIN_TOP });
        }, (row: number) => row.toString())
      }
      .width('100%');

      SubHeader({
        secondaryTitle: $r('app.string.compliance_dialog_description_title'),
        contentMargin: {
          start: LengthMetrics.vp(0),
          end: LengthMetrics.vp(0)
        },
        contentPadding: {
          start: LengthMetrics.vp(0),
          end: LengthMetrics.vp(0)
        }
      })
      .width('100%');

      TextArea({ text: this.description, placeholder: $r('app.string.compliance_dialog_description_placeholder') })
        .width('100%')
        .backgroundColor($r('sys.color.comp_background_primary'))
        .fontSize($r('sys.float.Body_L'))
        .fontWeight(FontWeight.Regular)
        .constraintSize({
          minHeight: AiCardConstants.DIALOG_INPUT_MIN_HEIGHT,
          maxHeight: AiCardConstants.DIALOG_INPUT_MAX_HEIGHT
        })
        .maxLength(MAX_DESCRIPTION_LENGTH)
        .showCounter(true)
        .onChange((v: string) => {
          this.description = v;
        });

      Row({ space: AiCardConstants.DIALOG_CHIP_COL_GAP }) {
        Button({ type: ButtonType.Normal }) {
          Text($r('app.string.compliance_dialog_cancel'))
            .fontSize($r('sys.float.Body_L'))
            .fontColor($r('sys.color.font_emphasize'));
        }
        .layoutWeight(1)
        .padding({
          top: AiCardConstants.DIALOG_BUTTON_PADDING_V,
          bottom: AiCardConstants.DIALOG_BUTTON_PADDING_V
        })
        .backgroundColor(Color.Transparent)
        .onClick(() => this.handleCancel());

        Button({ type: ButtonType.Normal }) {
          Text($r('app.string.compliance_dialog_confirm'))
            .fontSize($r('sys.float.Body_L'))
            .fontColor($r('sys.color.font_emphasize'));
        }
        .layoutWeight(1)
        .padding({
          top: AiCardConstants.DIALOG_BUTTON_PADDING_V,
          bottom: AiCardConstants.DIALOG_BUTTON_PADDING_V
        })
        .backgroundColor(Color.Transparent)
        .enabled(this.hasSelectedTag())
        .onClick(() => this.handleConfirm());
      }
      .width('100%');
    }
    .width('100%');
  }
}
