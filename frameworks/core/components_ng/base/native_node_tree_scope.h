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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_NATIVE_NODE_TREE_SCOPE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_NATIVE_NODE_TREE_SCOPE_H

#include <cstdint>
#include <initializer_list>
#include <cstddef>

#include "base/utils/macros.h"
#include "base/utils/noncopyable.h"

namespace OHOS::Ace::NG {
class UINode;

// API-entry snapshots retain all exposed members of an operated tree until the
// outermost public call returns. No UINode mutation or lifecycle hooks are used.
class ACE_FORCE_EXPORT NativeNodeTreeScope final {
public:
    // Takes ownership of the optional snapshot returned by CaptureNodes().
    NativeNodeTreeScope(std::initializer_list<UINode*> nodes, const char* apiName, void* snapshot = nullptr);
    ~NativeNodeTreeScope();
    static void* CaptureNodes(UINode* const* nodes, size_t count);
    void IncludeNode(UINode* node);
    static void SetCheckMode(int32_t mode);
    static bool IsCheckEnabled();
    static bool NeedsSynchronization();

private:
    struct Context;
    struct Preparation;
    void Enter(std::initializer_list<UINode*> nodes, Preparation& preparation);
    Context* context_ = nullptr;
    const char* apiName_ = nullptr;
    bool outermost_ = false;
    ACE_DISALLOW_COPY_AND_MOVE(NativeNodeTreeScope);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_NATIVE_NODE_TREE_SCOPE_H
