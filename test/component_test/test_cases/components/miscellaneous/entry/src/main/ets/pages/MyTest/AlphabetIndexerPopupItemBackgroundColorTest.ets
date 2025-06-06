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
import {
  assertEqual,
  assertFalse,
  Component,
  defineTest,
  Matcher,
  TestCaseLevel,
  Tester
} from '@ohos.arkui.componentTest'

export default async function alphabetIndexerPopupItemBackgroundColorTest() {
  /*
* AlphabetIndexer组件  popupItemBackgroundColor属性
* 找到画面id为alphabetIndexerPopupItemBackgroundColor1,alphabetIndexerPopupItemBackgroundColor2的AlphabetIndexer组件
* 找到画面文本为AlphabetIndexerPopupItemBackgroundColor1,AlphabetIndexerPopupItemBackgroundColor2的button组件
* 获取alphabetIndexer1的popupItemBackgroundColor属性与期待值Color.Pink比对
* 获取alphabetIndexer2的popupItemBackgroundColor属性与期待值'#FFFFC0CB'比对
* 点击button1，变更alphabetIndexer2的popupItemBackgroundColor属性值为'#ffa500'
* 获取alphabetIndexer2的popupItemBackgroundColor属性与期待值'#FFFFA500'比对
* 点击button2，变更alphabetIndexer2的popupItemBackgroundColor属性值为$r('sys.color.ohos_id_color_text_hyperlink')
* 获取alphabetIndexer2的popupItemBackgroundColor属性与期待值$r('sys.color.ohos_id_color_text_hyperlink')比对
*/
  defineTest('alphabetIndexerPopupItemBackgroundColorTestDemo', {
    level: TestCaseLevel.LEVEL0
  }, async () => {
    console.log("alphabetIndexerPopupItemBackgroundColorTestDemo running")
    let driver: Tester = Tester.create()
    let alphabetIndexer1: Component =
      await driver.findComponent(Matcher.create().id('alphabetIndexerPopupItemBackgroundColor1').enabled(true))
    let alphabetIndexer2: Component =
      await driver.findComponent(Matcher.create().id('alphabetIndexerPopupItemBackgroundColor2').enabled(true))
    let button1: Component =
      await driver.findComponent(Matcher.create().text('AlphabetIndexerPopupItemBackgroundColor1').enabled(true))
    let button2: Component =
      await driver.findComponent(Matcher.create().text('AlphabetIndexerPopupItemBackgroundColor2').enabled(true))

    let popupItemBackgroundColor1: string = await alphabetIndexer1.getInspectorInfo()
    assertEqual(JSON.parse(popupItemBackgroundColor1).$attrs.popupItemBackground.toLowerCase(), Color.Pink)

    let popupItemBackgroundColor2: string = await alphabetIndexer2.getInspectorInfo()
    assertEqual(JSON.parse(popupItemBackgroundColor2).$attrs.popupItemBackground, '#FFFFC0CB')

    await button1.tap()
    let popupItemBackgroundColor3: string = await alphabetIndexer2.getInspectorInfo()
    assertEqual(JSON.parse(popupItemBackgroundColor3).$attrs.popupItemBackground, '#FFFFA500')

    await button2.tap()
    let popupItemBackgroundColor4: string = await alphabetIndexer2.getInspectorInfo()
    assertEqual(JSON.parse(popupItemBackgroundColor4).$attrs.popupItemBackground,
      $r('sys.color.ohos_id_color_text_hyperlink'))
  }, 0)
}