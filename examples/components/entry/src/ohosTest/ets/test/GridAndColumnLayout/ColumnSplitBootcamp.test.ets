/*
 * Copyright (c) 2025 Hunan OpenValley Digital Industry Development Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from '@ohos/hypium'
import { abilityDelegatorRegistry, Component, Driver, ON } from '@kit.TestKit';

import {
  buttonClick,
  JsonType,
  KeyStruct,
  radioClick,
  Slide,
  sliderDrag,
  ToggleClick,
  scroll,
  Search,
  searchParentBlock,
  searchRadio,
  titleSearch,
  searchToggle,
  getToggle
} from '../../utils/index';
import { On, Point } from '@ohos.UiTest';


/*获取AbilityDelegator实例,管理Ability的生命周期和事件*/
const delegator: abilityDelegatorRegistry.AbilityDelegator = abilityDelegatorRegistry.getAbilityDelegator()

/*初始化与UI元素交互的驱动对象*/
const driver = Driver.create();


export default function ColumnSplitBootcampTest() {
  describe('ColumnSplitBootcampTest', () => {
    // Defines a test suite. Two parameters are supported: test suite name and test suite function.
    beforeAll(async (done: Function) => {

      console.info('beforeAll begin');

      try {

        await delegator.startAbility({
          bundleName: 'cn.ohos.components',
          abilityName: 'EntryAbility'
        });

        await driver.delayMs(1500);

        let atomDisplays: Component = await driver.findComponent(ON.text('栅格与分栏'));
        await atomDisplays.click();
        await driver.delayMs(1000);

        let scrollSearch: Component = await driver.findComponent(ON.text('纵向分割容器/ColumnSplit'))
        await scrollSearch.click()
        await driver.delayMs(1000);

        let Buttons: Component[] = await driver.findComponents(ON.type('Button'));
        await Buttons[1].click()
        await driver.delayMs(1000);

      } catch (err) {

        console.info(`beforeAll error: ${JSON.stringify(err)}`);

      }
      console.info(`beforeAll end`);
      done();
      // Presets an action, which is performed only once before all test cases of the test suite start.
      // This API supports only one parameter: preset action function.
    })
    beforeEach(() => {
      // Presets an action, which is performed before each unit test case starts.
      // The number of execution times is the same as the number of test cases defined by **it**.
      // This API supports only one parameter: preset action function.
    })
    afterEach(() => {
      // Presets a clear action, which is performed after each unit test case ends.
      // The number of execution times is the same as the number of test cases defined by **it**.
      // This API supports only one parameter: clear action function.
    })
    afterAll(() => {
      // Presets a clear action, which is performed after all test cases of the test suite end.
      // This API supports only one parameter: clear action function.
    })
    it('ArkUI_ColumnSplit_0001', 0, async (done: Function) => {
      await titleSearch('参数列表', 'resizeable');
      await radioClick({
        radioClickObj: 0,
        toggleClick: {
          toggleClickType: ToggleClick.Open,
          toggleClickObj: 'resizeable'
        },
        id: 'ColumnSplit',
        arr: [
          new KeyStruct('$attrs', JsonType.obj),
          new KeyStruct('resizeable', JsonType.value)
        ],
        assertEqual: 'true'
      })
      done();
    })
    it('ArkUI_ColumnSplit_0002', 0, async (done: Function) => {
      await titleSearch('参数列表', 'resizeable');
      await radioClick({
        radioClickObj: 1,
        toggleClick: {
          toggleClickType: ToggleClick.Close,
          toggleClickObj: 'resizeable'
        },
        id: 'ColumnSplit',
        arr: [
          new KeyStruct('$attrs', JsonType.obj),
          new KeyStruct('resizeable', JsonType.value)
        ],
        assertEqual: 'false'
      })
      done();
    })
    it('ArkUI_ColumnSplit_0003', 0, async (done: Function) => {
      await titleSearch('参数列表', 'divider.startMargin');
      await sliderDrag({
        slideStar: Slide.start,
        toggleClick: {
          toggleClickObj: 'divider.startMargin'
        },
        id: 'ColumnSplit',
        arr: [
          new KeyStruct('$attrs', JsonType.obj),
          new KeyStruct('divider', JsonType.obj),
          new KeyStruct('startMargin', JsonType.value)
        ],
        assertEqual: '10.00vp'
      })

      await sliderDrag({
        slideStar: Slide.end,
        id: 'ColumnSplit',
        toggleClick: {
          toggleClickObj: 'divider.startMargin',
          toggleClickType: ToggleClick.Close
        },
        arr: [
          new KeyStruct('$attrs', JsonType.obj),
          new KeyStruct('divider', JsonType.obj),
          new KeyStruct('startMargin', JsonType.value)
        ],
        assertEqual: '0.00vp'
      })
      done();

    })
    it('ArkUI_ColumnSplit_0004', 0, async (done: Function) => {
      await titleSearch('参数列表', 'divider.endMargin');
      await sliderDrag({
        slideStar: Slide.start,
        toggleClick: {
          toggleClickObj: 'divider.endMargin'
        },
        id: 'ColumnSplit',
        arr: [
          new KeyStruct('$attrs', JsonType.obj),
          new KeyStruct('divider', JsonType.obj),
          new KeyStruct('endMargin', JsonType.value)
        ],
        assertEqual: '10.00vp'
      })

      await sliderDrag({
        slideStar: Slide.end,
        id: 'ColumnSplit',
        toggleClick: {
          toggleClickObj: 'divider.endMargin',
          toggleClickType: ToggleClick.Close
        },
        arr: [
          new KeyStruct('$attrs', JsonType.obj),
          new KeyStruct('divider', JsonType.obj),
          new KeyStruct('endMargin', JsonType.value)
        ],
        assertEqual: '0.00vp'
      })
      done();

    })
  })
}