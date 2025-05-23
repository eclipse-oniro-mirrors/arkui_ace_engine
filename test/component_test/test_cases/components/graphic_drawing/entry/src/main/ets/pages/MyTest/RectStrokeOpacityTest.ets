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

import { assertEqual, assertContain, assertFalse, assertTrue } from '@ohos.arkui.componentTest';
import { TestCaseLevel, defineTest } from '@ohos.arkui.componentTest'
import { Tester, Component, Matcher } from '@ohos.arkui.componentTest'

export default async function rectStrokeOpacityTest() {
  /*
   * Rect组件 strokeOpacity属性
   * 找到画面id为rectStrokeOpacity1,rectStrokeOpacity2的Rect组件
   * 找到画面文本为RectStrokeOpacity的Button组件
   * 获取rect1的strokeOpacity属性与期待值0.0比对
   * 点击button,变更rect1的strokeOpacity属性值为'5px'
   * 获取rect1的strokeOpacity属性与期待值'1.0'比对
   * 获取rect2的strokeOpacity属性与期待值'0.8'比对
   */
  defineTest('rectStrokeOpacityTestDemo', {
    level: TestCaseLevel.LEVEL0
  }, async () => {
    console.log("rectStrokeOpacityTestDemo running")
    let driver: Tester = Tester.create()
    let rect1: Component = await driver.findComponent(Matcher.create().id('rectStrokeOpacity1').enabled(true))
    let rect2: Component = await driver.findComponent(Matcher.create().id('rectStrokeOpacity2').enabled(true))
    let button: Component = await driver.findComponent(Matcher.create().text('RectStrokeOpacity').enabled(true))

    let strokeOpacity1: string = await rect1.getInspectorInfo()
    assertEqual(JSON.parse(strokeOpacity1).$attrs.strokeOpacity, '0.000000')

    await button.tap()
    let strokeOpacity2: string = await rect1.getInspectorInfo()
    assertEqual(JSON.parse(strokeOpacity2).$attrs.strokeOpacity, '1.000000')

    let strokeOpacity3: string = await rect2.getInspectorInfo()
    assertEqual(JSON.parse(strokeOpacity3).$attrs.strokeOpacity, $r('app.string.stroke_opacity'))
  }, 0)
}