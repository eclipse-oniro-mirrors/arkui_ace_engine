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

import gridSpanTest from '../MyTest/GridSpanTest'
import { loadComponentTestEntry } from '@ohos.arkui.componentTest'

@Entry
@Component
struct GridSpanSample {
  aboutToAppear(): void {
    loadComponentTestEntry(gridSpanTest)
  }

  build() {
    Flex() {
      Row() {
        Text('Center').fontSize(25)
      }
      .id('rowGridSpan')
      .gridSpan(2)
      .gridOffset(1)
      .height("100%")
      .backgroundColor(0x66b6c5d1)
    }
  }
}
