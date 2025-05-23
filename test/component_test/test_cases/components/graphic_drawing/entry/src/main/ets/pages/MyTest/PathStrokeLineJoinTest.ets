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

export default async function pathStrokeLineJoinTest() {
  /*
   * Path组件 strokeLineJoin属性
   * 找到画面id为pathStrokeLineJoin1,pathStrokeLineJoin2的Path组件
   * 找到画面文本为PathStrokeLineJoin的Button组件
   * 获取path1的strokeLineJoin属性与期待值LineJoinStyle.Bevel比对
   * 获取path2的strokeLineJoin属性与期待值LineJoinStyle.Round比对
   * 点击button，变更path2的strokeLineJoin属性值为LineJoinStyle.Miter
   * 获取path2的strokeLineJoin属性与期待值LineJoinStyle.Miter比对
   */
  defineTest('pathStrokeLineJoinTestDemo', {
    level: TestCaseLevel.LEVEL0
  }, async () => {
    console.log("pathStrokeLineJoinTestDemo running")
    let driver: Tester = Tester.create()
    let path1: Component = await driver.findComponent(Matcher.create().id('pathStrokeLineJoin1').enabled(true))
    let path2: Component = await driver.findComponent(Matcher.create().id('pathStrokeLineJoin2').enabled(true))
    let button: Component = await driver.findComponent(Matcher.create().text('PathStrokeLineJoin').enabled(true))

    let strokeLineJoin1: string = await path1.getInspectorInfo()
    assertEqual(JSON.parse(strokeLineJoin1).$attrs.strokeLineJoin, 'LineJoinStyle.Bevel')

    let strokeLineJoin2: string = await path2.getInspectorInfo()
    assertEqual(JSON.parse(strokeLineJoin2).$attrs.strokeLineJoin, 'LineJoinStyle.Round')

    await button.tap()
    let strokeLineJoin3: string = await path2.getInspectorInfo()
    assertEqual(JSON.parse(strokeLineJoin3).$attrs.strokeLineJoin, 'LineJoinStyle.Miter')
  }, 0)
}