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
  assertFalse,
  assertTrue,
  Component,
  defineTest,
  Matcher,
  TestCaseLevel,
  Tester
} from '@ohos.arkui.componentTest'

export default async function navigationHideNavBarTestTest() {
  /*
* Navigation组件 HideNavBar属性
* 找到画面id为navigationHideNavBar的Navigation组件
* 找到画面文本为NavigationHideNavBar的Button组件
* 获取navigation1的HideNavBar属性与期待值True比对
* 点击button1，变更navigation1的HideNavBar属性值为False
* 获取navigation1的HideNavBar属性与期待值False比对
*/
  defineTest('navigationHideNavBarTestDemo', {
    level: TestCaseLevel.LEVEL0
  }, async () => {
    console.log("navigationHideNavBarTestDemo running")
    let driver: Tester = Tester.create()
    let navigation1: Component = await driver.findComponent(Matcher.create().id('navigationHideNavBar').enabled(true))
    let button1: Component = await driver.findComponent(Matcher.create().text('NavigationHideNavBar').enabled(true))
    let hideNavBar1: string = await navigation1.getInspectorInfo()
    assertTrue(JSON.parse(hideNavBar1).$attrs.hideNavBar)
    await button1.tap()
    let hideNavBar2: string = await navigation1.getInspectorInfo()
    assertFalse(JSON.parse(hideNavBar2).$attrs.hideNavBar)
  }, 0)
}