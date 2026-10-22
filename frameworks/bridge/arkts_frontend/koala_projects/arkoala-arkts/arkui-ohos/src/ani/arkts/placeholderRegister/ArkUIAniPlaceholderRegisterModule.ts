/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import { int32, int64 } from "@koalaui/common"

// Native face of UIContext.getPlaceholderRegister() on the static side: the
// registration state lives in the per-pipeline ScrollPlaceholderManager, and
// the admission protocol (isolated execution, restricted-scope probe, clone
// check) runs natively like on the declarative side. The builder crosses as a
// WrappedBuilder for the parameter-form validation only; every execution goes
// through the framework execution closure (third argument), which installs the
// canonical static isolated-creation context around the compiled builder.
export class ArkUIAniPlaceholderRegisterModule {
    static hasLoadLibrary: boolean = false;

    static {
        ArkUIAniPlaceholderRegisterModule.LoadLibrary();
    }

    public static LoadLibrary(): void {
        if (ArkUIAniPlaceholderRegisterModule.hasLoadLibrary) {
            return;
        }
        loadLibrary("arkoala_native_ani")
        ArkUIAniPlaceholderRegisterModule.hasLoadLibrary = true;
    }

    // Returns 0 when admitted; negative values encode the rejection cause.
    native static _PlaceholderRegisterRegister(id: string, builder: Object, execution: Object): int32;
    native static _PlaceholderRegisterUnregister(id: string): void;
    native static _PlaceholderRegisterIsRegistered(id: string): boolean;
    // Product hand-off of one isolated execution: the execution closure pops
    // the built subtree root from its isolated view stack and publishes the
    // pointer here, so the value never depends on ani primitive return boxing.
    native static _PlaceholderRegisterPublishProduct(ptr: int64): void;
}
