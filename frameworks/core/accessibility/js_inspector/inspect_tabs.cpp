/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "inspect_tabs.h"

namespace OHOS::Ace::Framework {
InspectTabs::InspectTabs(NodeId nodeId, const std::string& nodeName) : InspectNode(nodeId, nodeName) {
}

void InspectTabs::PackAttrAndStyle()
{
    // add for the attrs
    attrs_.insert(std::make_pair("index", "0"));
    attrs_.insert(std::make_pair("vertical", "false"));
    attrs_.insert(std::make_pair("disabled", "false"));
    attrs_.insert(std::make_pair("focusable", "true"));

    // only the common styles
}
} // namespace OHOS::Ace::Framework
