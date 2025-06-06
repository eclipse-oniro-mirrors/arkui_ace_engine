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

export default async function imageAnimatorDurationTest() {
  /*
 * ImageAnimator组件 Duration属性
 * 找到画面id为imageAnimatorDuration的ImageAnimator组件
 * 找到画面文本为ImageAnimatorDuration的Button组件
 * 获取imageAnimator的Duration属性与期待值0比对
 * 点击button1，变更imageAnimator的Duration属性值为1000
 * 获取imageAnimator的Duration属性与期待值1000比对
 */
  defineTest('imageAnimatorDurationTestDemo', {
    level: TestCaseLevel.LEVEL0
  }, async () => {
    console.log("imageAnimatorDurationTestDemo running")
    let driver: Tester = Tester.create()
    let imageAnimator: Component =
      await driver.findComponent(Matcher.create().id('imageAnimatorDuration').enabled(true))
    let button1: Component = await driver.findComponent(Matcher.create().text('ImageAnimatorDuration').enabled(true))
    let duration1: string = await imageAnimator.getInspectorInfo()
    assertEqual(JSON.parse(duration1).$attrs.duration, '0')
    await button1.tap()
    let duration2: string = await imageAnimator.getInspectorInfo()
    assertEqual(JSON.parse(duration2).$attrs.duration, '1000')
  }, 0)
}