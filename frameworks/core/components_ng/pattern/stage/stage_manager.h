/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_STAGE_STAGE_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_STAGE_STAGE_MANAGER_H

#include "base/memory/ace_type.h"

namespace OHOS::Ace::NG {

class ElementNode;
class FrameNode;
class StagePattern;

// StageManager is the base class for root render node to perform page switch.
class ACE_EXPORT StageManager : public virtual AceType {
    DECLARE_ACE_TYPE(StageManager, AceType);

public:
    explicit StageManager(const RefPtr<FrameNode>& root);
    ~StageManager() override = default;

    void PushPage(const RefPtr<ElementNode>& node);

private:
    RefPtr<FrameNode> rootNode_;
    RefPtr<StagePattern> stagePattern_;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_STAGE_STAGE_MANAGER_H
