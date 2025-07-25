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

export const getParamsInfo = (obj, special = []): string => {
  let str = `ljx\n`;
  if (obj) {
    Object.keys(obj).forEach((key) => {
      let specialItem = special.find(item => item.key === key);
      str += specialItem ? ` ${key}:${specialItem.type[obj[key]]}\n` : ` ${key}:${JSON.stringify(obj[key],
        (key: string, value: string,) => typeof value === 'undefined' ? 'undefined' : value)}\n`;
    });
    console.info(str);
    return str;
  } else {
    return 'undefined';
  }
};