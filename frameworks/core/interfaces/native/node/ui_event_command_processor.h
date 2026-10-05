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

#ifndef FRAMEWORKS_CORE_INTERFACES_NATIVE_NODE_UI_EVENT_COMMAND_PROCESSOR_H
#define FRAMEWORKS_CORE_INTERFACES_NATIVE_NODE_UI_EVENT_COMMAND_PROCESSOR_H

#include "core/interfaces/arkoala/arkoala_api.h"

namespace OHOS::Ace::NG {
// callback transports an OH_ArkUI_NativeModule_UIEventInjectionCallback; restore that type before invoking it.
ArkUI_Int32 InjectCompositeCommandImpl(ArkUI_Int32 instanceId, ArkUI_Uint32 uniqueId,
    const ArkUI_CharPtr json, ArkUI_Uint32 jsonSize,
    void (*callback)(ArkUI_Int32, void*), void* userData);
} // namespace OHOS::Ace::NG

#endif // FRAMEWORKS_CORE_INTERFACES_NATIVE_NODE_UI_EVENT_COMMAND_PROCESSOR_H
