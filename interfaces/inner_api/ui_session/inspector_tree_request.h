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

#ifndef FOUNDATION_ACE_INTERFACES_INNER_API_UI_SESSION_INSPECTOR_TREE_REQUEST_H
#define FOUNDATION_ACE_INTERFACES_INNER_API_UI_SESSION_INSPECTOR_TREE_REQUEST_H

#include <cstdint>
#include <string>

namespace OHOS::Ace {
struct InspectorTreeRequestToken {
    uint64_t connectionEpoch = 0;
    uint64_t generation = 0;

    bool operator==(const InspectorTreeRequestToken& other) const
    {
        return connectionEpoch == other.connectionEpoch && generation == other.generation;
    }
};

struct InspectorTreeRequestRoute {
    InspectorTreeRequestToken token;
    int32_t callerPid = -1;
    int32_t windowId = -1;
};

struct InspectorWindowSnapshot {
    bool privacyWindow = false;
    std::string windowTitle;
    int32_t pid = 0;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_INTERFACES_INNER_API_UI_SESSION_INSPECTOR_TREE_REQUEST_H
