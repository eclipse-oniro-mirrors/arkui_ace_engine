/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import { int32, } from "@koalaui/common"
import { NativeLog } from "./NativeLog"

export type UserViewBuilder =
/** @memo */
() => void

export class UserView {
    getBuilder(): UserViewBuilder {
        throw new Error("User must override this method");
    }

    // Improve: these native functions are here temporary.
    static startNativeLog(group: int32) {
        NativeLog.Default.startNativeLog(group)
    }

    static stopNativeLog(group: int32) {
        NativeLog.Default.stopNativeLog(group)
    }

    static getNativeLog(group: int32): string {
        return NativeLog.Default.getNativeLog(group)
    }

    static printNativeLog(group: int32) {
        return NativeLog.Default.printNativeLog(group)
    }
}

export function UserMemoWrapper(
    /** @memo */
    page_: (() => void),
): UserViewBuilder {
    /** @memo */
    const wrapper = () => { page_() }
    return wrapper
}
