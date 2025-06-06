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

export default async function offScreenCanvasRenderingContext2DDirectionTest() {
  /*
   * OffScreenCanvasRenderingContext2D组件 direction属性
   * 找到画面id为offScreenCanvasRenderingContext2DDirection的Canvas组件
   * 找到画面文本为OffScreenCanvasRenderingContext2DDirection1,
   * OffScreenCanvasRenderingContext2DDirection2的Button组件
   * 获取offCanvas的direction属性与期待值'inherit'比对
   * 点击button1，变更offCanvas的direction属性值为'rtl'
   * 获取offCanvas的direction属性与期待值'rtl'比对
   * 点击button2，变更offCanvas的direction属性值为'ltr'
   * 获取offCanvas的direction属性与期待值'ltr'比对
   */
  defineTest('offScreenCanvasRenderingContext2DDirectionTestDemo', {
    level: TestCaseLevel.LEVEL0
  }, async () => {
    console.log("offScreenCanvasRenderingContext2DDirectionTestDemo running")

    let driver: Tester = Tester.create()
    let offCanvas: Component =
      await driver.findComponent(Matcher.create().id('offScreenCanvasRenderingContext2DDirection').enabled(true))
    let button1: Component =
      await driver.findComponent(Matcher.create().text('OffScreenCanvasRenderingContext2DDirection1').enabled(true))
    let button2: Component =
      await driver.findComponent(Matcher.create().text('OffScreenCanvasRenderingContext2DDirection2').enabled(true))

    let offDirection1: string = await offCanvas.getInspectorInfo()
    assertEqual(JSON.parse(offDirection1).$attrs.direction, 'inherit')

    await button1.tap()
    let offDirection2: string = await offCanvas.getInspectorInfo()
    assertEqual(JSON.parse(offDirection2).$attrs.direction, 'rtl')

    await button2.tap()
    let offDirection3: string = await offCanvas.getInspectorInfo()
    assertEqual(JSON.parse(offDirection3).$attrs.direction, 'ltr')
  }, 0)
}