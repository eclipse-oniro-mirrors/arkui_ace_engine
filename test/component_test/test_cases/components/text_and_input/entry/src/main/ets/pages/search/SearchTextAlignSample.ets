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
//测试代码路径
import { loadComponentTestEntry } from '@ohos.arkui.componentTest'
import searchTextAlignTest from '../MyTest/SearchTextAlignTest'

@Entry
@Component
struct SearchTextAlignSample {
  controller: SearchController = new SearchController()
  @State textAlignType: TextAlign = TextAlign.Center

  aboutToAppear(): void {
    loadComponentTestEntry(searchTextAlignTest)
  }

  build() {
    Column() {
      Search({ controller: this.controller })
        .textAlign(TextAlign.Start)
        .id('searchTextAlign1')
      Search({ controller: this.controller })
        .textAlign(this.textAlignType)
        .id('searchTextAlign2')
      Button('SearchTextAlign1')
        .onClick(() => {
          this.textAlignType = TextAlign.End
        })
      Button('SearchTextAlign2')
        .onClick(() => {
          this.textAlignType = TextAlign.JUSTIFY
        })
    }
  }
}