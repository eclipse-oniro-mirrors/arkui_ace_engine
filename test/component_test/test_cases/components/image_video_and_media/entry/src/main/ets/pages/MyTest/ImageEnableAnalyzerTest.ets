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
  assertTrue,
  Component,
  defineTest,
  Matcher,
  TestCaseLevel,
  Tester
} from '@ohos.arkui.componentTest'

export default async function imageEnableAnalyzerTest() {
  /*
* Image组件 EnableAnalyzer属性
* 找到画面id为ImageEnableAnalyzer的Image组件
* 找到画面文本为ImageEnableAnalyzer的Button组件
* 获取image的EnableAnalyzer属性与期待值true比对
* 点击button，变更image的EnableAnalyzer属性值为false
* 获取image的EnableAnalyzer属性与期待值false比对
*/
  defineTest('imageEnableAnalyzerTestDemo', {
    level: TestCaseLevel.LEVEL0
  }, async () => {
    console.log("imageEnableAnalyzerTestDemo running")
    let driver: Tester = Tester.create()
    let image: Component = await driver.findComponent(Matcher.create().id('imageEnableAnalyzer').enabled(true))
    let button: Component = await driver.findComponent(Matcher.create().text('ImageEnableAnalyzer').enabled(true))
    let enableAnalyzer1: string = await image.getInspectorInfo()
    assertTrue(JSON.parse(enableAnalyzer1).$attrs.enableAnalyzer)
    await button.tap()
    let enableAnalyzer2: string = await image.getInspectorInfo()
    assertFalse(JSON.parse(enableAnalyzer2).$attrs.enableAnalyzer)
  }, 0)
}