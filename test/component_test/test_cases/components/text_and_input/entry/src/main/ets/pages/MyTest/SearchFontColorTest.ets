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
import { assertEqual, Component, defineTest, Matcher, TestCaseLevel, Tester } from '@ohos.arkui.componentTest';

export default async function searchFontColorTest() {
  /*
   * Search组件 FontColor属性
   * 找到画面id为searchFontColor1,searchFontColor2的Search组件
   * 找到画面文本为SearchFontColor1,SearchFontColor2的Button组件
   * 获取search1的FontColor属性与期待值Color.Yellow比对
   * 获取search2的FontColor属性与期待值0xffffff比对
   * 点击button1，变更search2的FontColor属性值为'rgb(255, 100, 255)'
   * 获取search2的FontColor属性与期待值'rgb(255, 100, 255)'比对
   * 点击button2，变更search2的FontColor属性值为$r('sys.color.ohos_id_color_text_hyperlink')
   * 获取search2的FontColor属性与期待值$r('sys.color.ohos_id_color_text_hyperlink')比对
   */
  defineTest('searchFontColorTestDemo', {
    level: TestCaseLevel.LEVEL0
  }, async () => {
    console.log("searchFontColorTestDemo running")
    let driver: Tester = Tester.create()
    let search1: Component = await driver.findComponent(Matcher.create().id('searchFontColor1').enabled(true))
    let search2: Component = await driver.findComponent(Matcher.create().id('searchFontColor2').enabled(true))
    let button1: Component = await driver.findComponent(Matcher.create().text('SearchFontColor1').enabled(true))
    let button2: Component = await driver.findComponent(Matcher.create().text('SearchFontColor2').enabled(true))

    let fontColor1: string = await search1.getInspectorInfo()
    assertEqual(JSON.parse(fontColor1).$attrs.fontColor.toLowerCase(), Color.Yellow)
    let fontColor2: string = await search2.getInspectorInfo()
    assertEqual(JSON.parse(fontColor2).$attrs.fontColor.toLowerCase(), '#ffffffff')

    await button1.tap()
    let fontColor3: string = await search2.getInspectorInfo()
    assertEqual(JSON.parse(fontColor3).$attrs.fontColor, '#FFFF64FF')
    await button2.tap()
    let fontColor4: string = await search2.getInspectorInfo()
    assertEqual(JSON.parse(fontColor4).$attrs.fontColor, $r('sys.color.ohos_id_color_text_hyperlink'))
  }, 0)
}