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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_JSI_NATIVEMODULE_ARKTS_NATIVE_RAW_INPUT_EVENT_WRAPPER_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_JSI_NATIVEMODULE_ARKTS_NATIVE_RAW_INPUT_EVENT_WRAPPER_H

#include "ecmascript/napi/include/jsnapi.h"

#include "base/utils/macros.h"
#include "core/common/input_event_monitor_manager.h"

namespace OHOS::Ace::NG {
class FrameNode;
} // namespace OHOS::Ace::NG

namespace OHOS::Ace::Framework {
class ArkTSNativeRawInputEventWrapper {
public:
    static panda::Local<panda::JSValueRef> Create(
        const RawInputEventWrapper& wrapper, panda::ecmascript::EcmaVM* vm);
    ACE_FORCE_EXPORT static panda::Local<panda::JSValueRef> CreateSensitiveInputEvent(
        const RawInputEventWrapper& wrapper, panda::ecmascript::EcmaVM* vm, const RefPtr<NG::FrameNode>& node);
};
} // namespace OHOS::Ace::Framework

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_JSI_NATIVEMODULE_ARKTS_NATIVE_RAW_INPUT_EVENT_WRAPPER_H
