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
import { assertEqual, Component, defineTest, Matcher, TestCaseLevel, Tester } from '@ohos.arkui.componentTest';

export default async function textMaxLinesTest() {
  /*
* Text组件 MaxLines属性
* 找到画面id为textMaxLines1的Text组件
* 获取text的MaxLines属性与期待值1比对
*/
  defineTest('textMaxLinesTestDemo', {
    level: TestCaseLevel.LEVEL0
  }, async () => {
    console.log("textMaxLinesTestDemo running")
    let driver: Tester = Tester.create()
    let text: Component = await driver.findComponent(Matcher.create().id('textMaxLines1').enabled(true))

    let maxLines1: string = await text.getInspectorInfo()
    assertEqual(JSON.parse(maxLines1).$attrs.maxLines, '1')
  }, 0)
}