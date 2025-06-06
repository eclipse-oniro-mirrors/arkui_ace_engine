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

export default async function canvasRenderingContext2DLineJoinTest() {
  /*
   * CanvasRenderingContext2D组件 lineJoin属性
   * 找到画面id为canvasLineJoin的Canvas组件
   * 找到画面文本为CanvasLineJoin1,CanvasLineJoin2的Button组件
   * 获取canvas的lineJoin属性与期待值'bevel'比对
   * 点击button1，变更canvas的lineJoin属性值为'round'
   * 获取canvas的lineJoin属性与期待值'round'比对
   * 点击button2，变更canvas的lineJoin属性值为'miter'
   * 获取canvas的lineJoin属性与期待值'miter'比对
   */
  defineTest('canvasRenderingContext2DLineJoinCanvasLineJoinTestDemo', {
    level: TestCaseLevel.LEVEL0
  }, async () => {
    console.log("canvasRenderingContext2DLineJoinCanvasLineJoinTestDemo running")

    let driver: Tester = Tester.create()
    let canvas: Component = await driver.findComponent(Matcher.create().id('canvasLineJoin').enabled(true))
    let button1: Component = await driver.findComponent(Matcher.create().text('CanvasLineJoin1').enabled(true))
    let button2: Component = await driver.findComponent(Matcher.create().text('CanvasLineJoin2').enabled(true))

    let lineJoin1: string = await canvas.getInspectorInfo()
    assertEqual(JSON.parse(lineJoin1).$attrs.lineJoin, 'bevel')

    await button1.tap()
    let lineJoin2: string = await canvas.getInspectorInfo()
    assertEqual(JSON.parse(lineJoin2).$attrs.lineJoin, 'round')

    await button2.tap()
    let lineJoin3: string = await canvas.getInspectorInfo()
    assertEqual(JSON.parse(lineJoin3).$attrs.lineJoin, 'miter')
  }, 0)
}