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
#ifndef ARKUI_INTERACTION_OBSERVER_API_H
#define ARKUI_INTERACTION_OBSERVER_API_H

#include <cstdint>

// The JSON wrapper type is opaque to the framework side; only the tag is
// shared with interfaces/native/ui_json_wrapper.h so that the callback and
// factory signatures below match the public NDK types without casts.
struct OH_ArkUI_NativeModule_UIJsonWrapper;

// Separate versioned entry keeps the existing node API tables ABI-compatible.
// The NDK layer (libace_ndk.z.so) resolves this table through
// dlsym(GetArkUIInteractionObserverAPI) and never links the framework
// directly. The framework side owns the observer registry, the event
// serialization and the dispatch; the NDK layer only supplies the wrapper
// factory/destroyer trampolines so that the wrapper objects handed to the
// application callback are created by the module that defines them.
//
// Result codes mirror ArkUI_ErrorCode: 0 success, 401 invalid parameter,
// 190001 invalid UI context.
struct ArkUIInteractionObserverAPI {
    // Registers an observer for the given UI instance. eventMask uses the
    // OH_ArkUI_NativeModule_UIInfoCollection_InteractionEventType bits;
    // unsupported bits are ignored and an empty supported mask is rejected.
    // On success *registerID receives a non-zero registration ID; on failure
    // it is set to 0.
    int32_t (*registerObserver)(int32_t instanceId,
        uint32_t eventMask,
        void (*callback)(const struct OH_ArkUI_NativeModule_UIJsonWrapper* json, void* userData),
        void* userData,
        struct OH_ArkUI_NativeModule_UIJsonWrapper* (*createWrapper)(const char* data, uint32_t size),
        void (*destroyWrapper)(const struct OH_ArkUI_NativeModule_UIJsonWrapper* wrapper),
        uint32_t* registerID);
    // Removes the observer registered with the given ID. A second call with
    // the same ID reports 401 because the ID is no longer valid.
    int32_t (*unregisterObserver)(uint32_t registerID);
};

namespace OHOS::Ace::NodeModel {
const ArkUIInteractionObserverAPI* GetInteractionObserverAPI();
}
#endif
