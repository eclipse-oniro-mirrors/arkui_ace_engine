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
import { abilityDelegatorRegistry, Component, Driver, ON } from '@kit.TestKit';

const TAG = 'ToggleBootcampTest'

const delegator: abilityDelegatorRegistry.AbilityDelegator = abilityDelegatorRegistry.getAbilityDelegator()

const driver = Driver.create();

export async function ToggleStart(done: Function) {

  try {

    await delegator.startAbility({
      bundleName: 'cn.ohos.components',
      abilityName: 'EntryAbility'
    });

    await driver.delayMs(8000);
    console.info(TAG + 'ToggleBootcamp' + ' begin');

    let atomDisplays: Component = await driver.findComponent(ON.text('按钮与选择'));
    await atomDisplays.click();
    await driver.delayMs(1000);

    let scrollSearch: Component = await driver.findComponent(ON.text('切换按钮/Toggle'))
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
}