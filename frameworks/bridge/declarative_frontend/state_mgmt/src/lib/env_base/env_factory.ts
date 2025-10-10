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

/**
 * Maps environment property keys to their corresponding value types.
 *
 * This mapping should be maintained in a **common central location**.
 * Each environment property provider (platform/system team) is expected to:
 * - Add the key for the environment property they provide
 * - Map it to its respective type interface
 *
 */
interface EnvTypeMap {
  'system.arkui.breakpoint': WindowSizeLayoutBreakpoint;
}

/**
 * Factory map for creating environment property value instances.
 *
 * Keys match the entries in `EnvTypeMap`, and each factory function:
 * - Accepts a `context` object
 * - Returns the corresponding `IEnvironmentValue` instance
 *
 * This enables:
 * - On-demand creation of environment value providers
 * - Consistent construction logic for all environment properties
 *
 * To be updated by Platform Environment Provider for each environment property newly introduced(mocked here for testing)
 */

const envFactoryMap: {
  [K in keyof EnvTypeMap]: (context: UIContext) => IEnvironmentValue<EnvTypeMap[K]>;
} = {
  'system.arkui.breakpoint': (context: UIContext) => {
    const WindowSizeLayoutBreakpoint = requireNapi('arkui.windowsizelayoutbreakpoint').WindowSizeLayoutBreakpoint;
    if (typeof WindowSizeLayoutBreakpoint !== 'function') {
      throw new Error('WindowSizeLayoutBreakpoint not found (requireNapi failed).');
    }
    stateMgmtConsole.debug(`create WindowSizeLayoutBreakpoint.`);
    return new WindowSizeLayoutBreakpoint(context);
  }
};
