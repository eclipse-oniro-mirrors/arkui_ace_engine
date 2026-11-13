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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_TREE_PROVIDER_REGISTRY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_TREE_PROVIDER_REGISTRY_H

#include <cstdint>
#include <vector>

#include "base/memory/ace_type.h"

namespace OHOS::Ace {
namespace NG {
class UINode;

// External inspector-tree getter entry stored ON the host node
// (UINode::inspectorGetterEntry_, v1.5 storage model). Defined in the .cpp.
struct InspectorGetterEntry;

class ACE_FORCE_EXPORT InspectorTreeProviderRegistry final {
public:
    using InspectorTreeGetter = void (*)(void* userData, char** inspectorTree);

    struct GetterEntry {
        InspectorTreeGetter getter = nullptr;
        void* userData = nullptr;
    };

    static int32_t Register(UINode* anchorNode, InspectorTreeGetter getter, void* userData);

    /**
     * @brief Remove getter registration(s) bound to a node anchor: a non-null
     * <b>getter</b> removes the matching registration (no effect when absent);
     * nullptr removes all registrations of the node.
     */
    static int32_t Unregister(UINode* anchorNode, InspectorTreeGetter getter);

    /**
     * @brief Copy out all getters bound to the given live node (empty when
     * unregistered), in registration order.
     */
    static void GetInspectorGettersForNode(const RefPtr<UINode>& node, std::vector<GetterEntry>& getters);

    static void ReleaseNodeEntry(UINode* node);

    static constexpr int32_t RESULT_OK = 0;
    static constexpr int32_t RESULT_INVALID_PARAM = -1;
    static constexpr int32_t RESULT_UNSUPPORTED_HOST = -2;
    static constexpr int32_t RESULT_NOT_FOUND = -3;

private:
    InspectorTreeProviderRegistry() = delete;
};
} // namespace NG
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_TREE_PROVIDER_REGISTRY_H
