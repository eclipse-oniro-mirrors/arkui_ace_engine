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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_OVERLAY_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_OVERLAY_MANAGER_H

#include <vector>
#include "base/memory/ace_type.h"
#include "core/components_ng/base/ui_node.h"

namespace OHOS::Ace::NG {
class FrameNode;
class StagePattern;

struct ToastInfo {
    int32_t toastId = -1;
    RefPtr<FrameNode> toastNode;
};

// StageManager is the base class for root render node to perform page switch.
class ACE_EXPORT OverlayManager : public virtual AceType {
    DECLARE_ACE_TYPE(OverlayManager, AceType);

public:
    OverlayManager() = default;
    ~OverlayManager() override = default;

    void ShowToast(const RefPtr<UINode>& node, int32_t toastId, const std::string& message, const std::string& bottom, 
        bool isRightToLeft);
    void PopToast(int32_t toastId);

private:
    std::vector<NG::ToastInfo> toastStack_;
    WeakPtr<UINode> rootNodeWeak_;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_STAGE_STAGE_MANAGER_H
