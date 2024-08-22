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

class RefInfo {
  private static obj2ref: WeakMap<object, object> = new WeakMap();

  static get(target: Object): any {
    if (typeof (target) !== "object") {
      throw new Error("target must be a object");
    }
    // makeObserved does not support @Observed and @ObservedV2/@Trace class, will return target directly
    if (ObservedObject.IsObservedObject(target) || ObserveV2.IsObservedObjectV2(target)) {
      stateMgmtConsole.warn(`${target.constructor.name} is Observed ${ObservedObject.IsObservedObject(target)}, IsObservedV2 ${ObserveV2.IsObservedObjectV2(target)}. makeObserved will stop work`)
      return { proxy: target };
    }
    let ret = RefInfo.obj2ref.get(target);
    if (!ret) {
      if (Array.isArray(target) || SendableType.isArray(target)) {
        ret = { proxy: new Proxy(target, ObserveV2.arrayHandlerDeepObserved) };
      } else if (target instanceof Set || SendableType.isSet(target) || target instanceof Map || SendableType.isMap(target)) {
        ret = { proxy: new Proxy(target, ObserveV2.setMapHandlerDeepObserved) };
      } else if (target instanceof Date) {
        ret = { proxy: new Proxy(target, ObserveV2.dateHandlerDeepObserved) };
      } else {
        ret = { proxy: new Proxy(target, ObserveV2.normalObjectHandlerDeepObserved) };
      }
      RefInfo.obj2ref.set(target, ret);
    }
    return ret;
  }
}