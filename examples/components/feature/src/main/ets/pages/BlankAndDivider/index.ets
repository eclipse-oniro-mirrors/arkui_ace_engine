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

import { Route, RouteGroup } from 'common/src/main/ets/common/route';
import { DividerBootcamp } from './DividerBootcamp';
import { BlankBootcamp } from './BlankBootcamp';

export const blankAndDividerRoute: RouteGroup = {
  name: 'BlankAndDivider',
  label: '空白与分割',
  children: [
    { name: 'divider', label: '分割/divider' },
    { name: 'blank', label: '空白/blank' },
  ]
};

@Builder
export function blankAndDividerDestination(name: string, route: Route) {
  if (name === 'BlankAndDivider/divider') {
    DividerBootcamp({ title: route.label })
  } else if (name === 'BlankAndDivider/blank') {
    BlankBootcamp({ title: route.label })
  }
}
