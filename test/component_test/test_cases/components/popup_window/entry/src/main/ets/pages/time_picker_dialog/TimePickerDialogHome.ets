/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { CommonButton } from '../../util/CommonButton';

@Entry
@Component
struct TimePickerDialogHome {
  build() {
    Scroll() {
      Flex({ direction: FlexDirection.Row, wrap: FlexWrap.Wrap }) {
        Text('TimePickerDialogHome')
          .padding('15vp')
          .width('100%')
          .fontSize('25fp')
          .textAlign(TextAlign.Center)
        CommonButton({
          buttonName: 'TimePickerDialogAlignmentSample',
          buttonUrl: 'pages/time_picker_dialog/TimePickerDialogAlignmentSample'
        });
        CommonButton({
          buttonName: 'TimePickerDialogBackgroundBlurStyleSample',
          buttonUrl: 'pages/time_picker_dialog/TimePickerDialogBackgroundBlurStyleSample'
        });
        CommonButton({
          buttonName: 'TimePickerDialogBackgroundColorSample',
          buttonUrl: 'pages/time_picker_dialog/TimePickerDialogBackgroundColorSample'
        });
        CommonButton({
          buttonName: 'TimePickerDialogDisappearTextStyleSample',
          buttonUrl: 'pages/time_picker_dialog/TimePickerDialogDisappearTextStyleSample'
        });
        CommonButton({
          buttonName: 'TimePickerDialogMaskRectSample',
          buttonUrl: 'pages/time_picker_dialog/TimePickerDialogMaskRectSample'
        });
        CommonButton({
          buttonName: 'TimePickerDialogOffsetSample',
          buttonUrl: 'pages/time_picker_dialog/TimePickerDialogOffsetSample'
        });
        CommonButton({
          buttonName: 'TimePickerDialogOnAcceptSample',
          buttonUrl: 'pages/time_picker_dialog/TimePickerDialogOnAcceptSample'
        });
        CommonButton({
          buttonName: 'TimePickerDialogOnCancelSample',
          buttonUrl: 'pages/time_picker_dialog/TimePickerDialogOnCancelSample'
        });
        CommonButton({
          buttonName: 'TimePickerDialogOnChangeSample',
          buttonUrl: 'pages/time_picker_dialog/TimePickerDialogOnChangeSample'
        });
        CommonButton({
          buttonName: 'TimePickerDialogSelectedTextStyleSample',
          buttonUrl: 'pages/time_picker_dialog/TimePickerDialogSelectedTextStyleSample'
        });
        CommonButton({
          buttonName: 'TimePickerDialogTextStyleSample',
          buttonUrl: 'pages/time_picker_dialog/TimePickerDialogTextStyleSample'
        });
        CommonButton({
          buttonName: 'TimePickerDialogUseMilitaryTimeSample',
          buttonUrl: 'pages/time_picker_dialog/TimePickerDialogUseMilitaryTimeSample'
        });
      }
      .padding('5vp')
      .width('100%')
    }.margin({ bottom: 60 })
    .width('100%')
  }
}