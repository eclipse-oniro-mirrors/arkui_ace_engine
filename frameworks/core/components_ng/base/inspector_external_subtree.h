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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_EXTERNAL_SUBTREE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_EXTERNAL_SUBTREE_H

#include <memory>

#include "base/memory/ace_type.h"

namespace OHOS::Ace {
class JsonValue;
namespace NG {
class UINode;
class InspectorFilter;

void AppendExternalSubtreeToChildren(const RefPtr<NG::UINode>& parent,
    std::unique_ptr<OHOS::Ace::JsonValue>& jsonChildrenArray, const InspectorFilter& filter);
} // namespace NG
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_EXTERNAL_SUBTREE_H
