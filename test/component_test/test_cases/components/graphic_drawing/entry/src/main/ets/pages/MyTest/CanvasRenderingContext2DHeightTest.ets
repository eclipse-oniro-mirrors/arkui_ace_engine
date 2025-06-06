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

export default async function canvasRenderingContext2DHeightTest() {
  /*
   * CanvasRenderingContext2D组件 height属性
   * 找到画面id为canvasHeight的Canvas组件
   * 找到画面文本为CanvasHeight的Button组件
   * 获取canvas的height属性与期待值250比对
   * 点击button，变更canvas的height属性值为350
   * 获取canvas的height属性与期待值350比对
   */
  defineTest('canvasRenderingContext2DHeightTestDemo', {
    level: TestCaseLevel.LEVEL0
  }, async () => {
    console.log("canvasRenderingContext2DHeightTestDemo running")

    let driver: Tester = Tester.create()
    let canvas: Component = await driver.findComponent(Matcher.create().id('canvasHeight').enabled(true))
    let button: Component = await driver.findComponent(Matcher.create().text('CanvasHeight').enabled(true))

    let height1: string = await canvas.getInspectorInfo()
    assertFalse(JSON.parse(height1).$attrs.fillRect == undefined)
    assertEqual(JSON.parse(height1).$attrs.fillRect.h, 250)

    await button.tap()
    let height2: string = await canvas.getInspectorInfo()
    assertEqual(JSON.parse(height2).$attrs.fillRect.h, 350)
  }, 0)
}