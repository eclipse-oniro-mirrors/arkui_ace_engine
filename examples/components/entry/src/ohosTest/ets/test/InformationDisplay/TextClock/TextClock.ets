/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import { abilityDelegatorRegistry, Driver, Component, ON } from '@kit.TestKit';
import { Point } from '@ohos.UiTest';

const TAG = 'TextClockBootcamp001'

/*获取AbilityDelegator实例,管理Ability的生命周期和事件*/
const delegator: abilityDelegatorRegistry.AbilityDelegator = abilityDelegatorRegistry.getAbilityDelegator()

/*初始化与UI元素交互的驱动对象*/
const driver = Driver.create();

export async function TextClockStart(done: Function) {
  await delegator.startAbility({
    bundleName: 'cn.ohos.components',
    abilityName: 'EntryAbility'
  });

  await driver.delayMs(3500);
  console.info(TAG + 'TextClockBootcamp001' + ' begin');

  let ListItemGroup: Array<Component> = await driver.findComponents(ON.type('ListItemGroup'))
  console.info(TAG + 'ListItemGroup' + ListItemGroup.toString());
  let ListItemGroup1: Point = await ListItemGroup[0].getBoundsCenter()
  let ListItemGroup2: Point = await ListItemGroup[2].getBoundsCenter()
  await driver.swipe(ListItemGroup1.x, ListItemGroup2.y, ListItemGroup1.x, ListItemGroup1.y, 2000)
  await driver.delayMs(3500);

  let atomDisplays: Component = await driver.findComponent(ON.text('信息展示'));
  await atomDisplays.click();
  await driver.delayMs(4000);

  let ListItems: Array<Component> = await driver.findComponents(ON.type('ListItem'))
  let width: Point = await ListItems[0].getBoundsCenter()
  let width2: Point = await ListItems[2].getBoundsCenter()
  await driver.swipe(width.x, width2.y, width.x, width.y, 1000)
  await driver.delayMs(2000);

  let ListItems1: Array<Component> = await driver.findComponents(ON.type('ListItem'))
  let width3: Point = await ListItems1[2].getBoundsCenter()
  let width4: Point = await ListItems1[4].getBoundsCenter()
  await driver.swipe(width3.x, width4.y, width3.x, width3.y, 1000)
  await driver.delayMs(2000);


  let scrollSearch: Component = await driver.findComponent(ON.text('文本时钟/TextClock'))
  await scrollSearch.click()
  await driver.delayMs(2500);

  let Buttons: Component[] = await driver.findComponents(ON.type('Button'));
  await Buttons[1].click()
  await driver.delayMs(1000);

  console.info(`beforeAll end`);
  done();
}

