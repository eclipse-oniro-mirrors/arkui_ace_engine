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
  assertContain,
  assertEqual,
  assertFalse,
  assertTrue,
  Component,
  defineTest,
  Matcher,
  TestCaseLevel,
  Tester
} from '@ohos.arkui.componentTest';

export default async function shapeAntiAliasTest() {
  /*
   * Shape组件 antiAlias属性
   * 找到画面id为shapeAntiAlias的Shape组件
   * 找到画面文本为ShapeAntiAlias的Button组件
   * 获取shape的antiAlias属性与期待值判断是否为真
   * 点击button,变更shape的antiAlias属性值为假
   * 获取shape的antiAlias属性与期待值判断是否为假
   */
  defineTest('shapeAntiAliasTestDemo', {
    level: TestCaseLevel.LEVEL0
  }, async () => {
    console.log("shapeAntiAliasTestDemo running")
    let driver: Tester = Tester.create()
    let shape: Component = await driver.findComponent(Matcher.create().id('shapeAntiAlias').enabled(true))
    let button: Component = await driver.findComponent(Matcher.create().text('ShapeAntiAlias').enabled(true))

    let antiAlias1: string = await shape.getInspectorInfo()
    assertTrue(JSON.parse(antiAlias1).$attrs.antiAlias)

    await button.tap()
    let antiAlias2: string = await shape.getInspectorInfo()
    assertFalse(JSON.parse(antiAlias2).$attrs.antiAlias)
  }, 0)
}