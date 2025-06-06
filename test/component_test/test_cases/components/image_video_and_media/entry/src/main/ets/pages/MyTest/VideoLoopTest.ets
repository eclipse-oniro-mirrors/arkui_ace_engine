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

export default async function videoLoopTest() {
  /*
* Video组件 Loop属性
* 找到画面id为videoLoop的video组件
* 找到画面文本为VideoLoop的Button组件
* 获取video的Loop属性与期待值false比对
* 点击button，变更video的Loop属性值为true
* 获取video的Loop属性与期待值true比对
*/
  defineTest('videoLoopTestDemo', {
    level: TestCaseLevel.LEVEL0
  }, async () => {
    console.log('videoLoopTestDemo running')
    let driver: Tester = Tester.create()
    let video: Component = await driver.findComponent(Matcher.create().id('videoLoop').enabled(true))
    let button: Component = await driver.findComponent(Matcher.create().text('VideoLoop').enabled(true))

    let loop1: string = await video.getInspectorInfo()
    assertFalse(JSON.parse(loop1).$attrs.loop == undefined)
    assertFalse(JSON.parse(loop1).$attrs.loop)
    await button.tap()
    let loop2: string = await video.getInspectorInfo()
    assertTrue(JSON.parse(loop2).$attrs.loop)
  })
}