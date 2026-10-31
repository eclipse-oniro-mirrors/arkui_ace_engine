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
#ifndef ARKUI_UI_TREE_API_H
#define ARKUI_UI_TREE_API_H

#include <cstdint>

#include "interfaces/native/ui_info_collection.h"

// Version 1 is independent of ArkUIFullNodeAPI and the UiSession ParamConfig ABI.
struct ArkUITreeOptions {
    int32_t treeType = OH_ARKUI_NATIVE_MODULE_UI_AGENT_TREE_FULL;
    int32_t format = OH_ARKUI_NATIVE_MODULE_UI_JSON_COMPACT;
    bool filterPureLayout = false;
    bool interaction = false;
    bool accessibility = false;
    bool visual = false;
    bool functional = false;
    // Occlusion filtering applies to VISIBLE collections only and is off by default.
    bool filterOccludedNodes = false;
    float occlusionOpacityThreshold = 1.0f;
};

struct ArkUITreeCompletion {
    ArkUI_ContextHandle context = nullptr;
    OH_ArkUI_NativeModule_UIAgentJsonCallback callback = nullptr;
    void* userData = nullptr;
    // Bytes are borrowed only for this call. The NDK side creates the owned Wrapper.
    void (*deliver)(const ArkUITreeCompletion&, uint64_t, int32_t, const char*, uint32_t) = nullptr;
};

struct ArkUITreeAPI {
    int32_t (*collect)(int32_t, const ArkUITreeOptions*, char**, uint32_t*);
    int32_t (*submit)(int32_t, const ArkUITreeOptions*, const ArkUITreeCompletion*, uint64_t*);
    void (*release)(char*);
};

namespace OHOS::Ace::NodeModel {
const ArkUITreeAPI* GetTreeAPI();
}
#endif
