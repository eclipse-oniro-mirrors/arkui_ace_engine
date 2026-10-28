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
#ifndef ARKUI_CONTENT_CHANGE_API_H
#define ARKUI_CONTENT_CHANGE_API_H

#include <cstdint>

inline constexpr int32_t ARKUI_CONTENT_CHANGE_API_VERSION = 1;

// This table is separate from ArkUIFullNodeAPI to preserve its ABI.
struct ArkUIContentChangeAPI {
    int32_t (*registerEvent)(int32_t instanceId, bool withStart, uint32_t eventMask, uint32_t ignoreMask,
        void* cookie, void (*sink)(uint32_t, uint64_t, const char*, uint32_t, void*),
        void (*release)(void*), uint64_t* subscriptionId);
    int32_t (*unregisterEvent)(int32_t instanceId, uint64_t subscriptionId);
};

namespace OHOS::Ace::NodeModel {
const ArkUIContentChangeAPI* GetContentChangeAPI();
}

#endif
