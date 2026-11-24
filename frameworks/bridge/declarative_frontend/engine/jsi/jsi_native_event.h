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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_JSI_JSI_NATIVE_EVENT_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_JSI_JSI_NATIVE_EVENT_H

#include <memory>

#include "ecmascript/napi/include/jsnapi.h"

namespace OHOS::Ace::Framework {
class JsiNativeEvent {
public:
    template<typename T>
    static void Bind(const panda::ecmascript::EcmaVM* vm, const panda::Local<panda::ObjectRef>& object,
        const std::shared_ptr<T>& event)
    {
        // Keep the event pointer layout used by native methods, and retain its owner in the finalizer hint.
        // Each JS object owns a separate reference, including independently retained event subobjects.
        auto* owner = new std::shared_ptr<T>(event);
        object->SetNativePointerFieldCount(vm, 1);
        object->SetNativePointerField(vm, 0, event.get(), [](void*, void*, void* hint) {
            auto* owner = static_cast<std::shared_ptr<T>*>(hint);
            delete owner;
            owner = nullptr;
        }, owner);
    }
};
} // namespace OHOS::Ace::Framework

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_JSI_JSI_NATIVE_EVENT_H
