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

import { CommonButton } from '../util/CommonButton';

@Entry
@Component
struct Index {
  build() {
    Scroll() {
      Flex({ direction: FlexDirection.Row, wrap: FlexWrap.Wrap }) {
        Text('popup_window')
          .padding('15vp')
          .width('100%')
          .fontSize('25fp')
          .textAlign(TextAlign.Center)

        CommonButton({ buttonName: 'AlertDialog', buttonUrl: 'pages/alert_dialog/AlertDialogHome' });
        CommonButton({ buttonName: 'TextPickerDialog', buttonUrl: 'pages/text_picker_dialog/TextPickerDialogHome' })
        CommonButton({
          buttonName: 'TimePickerDialogHome',
          buttonUrl: 'pages/time_picker_dialog/TimePickerDialogHome'
        });
        CommonButton({
          buttonName: 'CalendarPickerDialog',
          buttonUrl: 'pages/calendar_picker_dialog/CalendarPickerDialogHome'
        });
        CommonButton({ buttonName: 'CustomDialog', buttonUrl: 'pages/custom_dialog/CustomDialogHome' });
        CommonButton({ buttonName: 'ActionSheet', buttonUrl: 'pages/action_sheet/ActionSheetHome' });
        CommonButton({ buttonName: 'DatePickerDialog', buttonUrl: 'pages/date_picker_dialog/DatePickerDialogHome' });
      }
      .padding('20vp')
      .width('100%')
    }.margin({ bottom: 60 })
    .width('100%')
  }
}