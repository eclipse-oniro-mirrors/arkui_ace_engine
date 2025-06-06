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

import { assertEqual, Component, defineTest, Matcher, TestCaseLevel, Tester } from '@ohos.arkui.componentTest'

export default async function selectFontColorTest() {
  /*
   * Select组件 FontColor属性
   * 找到画面id为selectFontColor1，selectFontColor2，selectFontColor3的Select组件
   * 找到画面文本为SelectFontColor的Button组件
   * 获取selectFontColor1的FontColor属性与期待值Color.Red比对
   * 获取selectFontColor2的FontColor属性与期待值'#FFA52A2A'比对
   * 获取selectFontColor3的FontColor属性与期待值'#FF8EFF65'比对
   * 点击button，变更selectFontColor3的FontColor属性值为$r('sys.color.ohos_fa_palette1')
   * 获取selectFontColor3的FontColor属性与期待值$r('sys.color.ohos_fa_palette1')比对
   */
  defineTest('selectFontColorTestDemo', {
    level: TestCaseLevel.LEVEL0
  }, async () => {
    console.log('selectFontColorTestDemo running')
    let driver: Tester = Tester.create()
    let selectFontColor1: Component = await driver.findComponent(Matcher.create().id('selectFontColor1').enabled(true))
    let selectFontColor2: Component = await driver.findComponent(Matcher.create().id('selectFontColor2').enabled(true))
    let selectFontColor3: Component = await driver.findComponent(Matcher.create().id('selectFontColor3').enabled(true))
    let button: Component = await driver.findComponent(Matcher.create().text('SelectFontColor').enabled(true))

    let fontColor1: string = await selectFontColor1.getInspectorInfo()
    assertEqual(JSON.parse(fontColor1).$attrs.fontColor.toLowerCase(), Color.Red)

    let fontColor2: string = await selectFontColor2.getInspectorInfo()
    assertEqual(JSON.parse(fontColor2).$attrs.fontColor, '#FFA52A2A')

    let fontColor3: string = await selectFontColor3.getInspectorInfo()
    assertEqual(JSON.parse(fontColor3).$attrs.fontColor, '#FF8EFF65')

    await button.tap()
    let fontColor4: string = await selectFontColor3.getInspectorInfo()
    assertEqual(JSON.parse(fontColor4).$attrs.fontColor, $r('sys.color.ohos_fa_palette1'))
  }, 0)
}