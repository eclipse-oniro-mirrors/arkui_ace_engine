/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_DIALOG_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_DIALOG_MANAGER_H

#include "base/utils/singleton.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"

namespace OHOS::Ace::NG {
class ACE_FORCE_EXPORT DialogManager : public Singleton<DialogManager> {
    DECLARE_SINGLETON(DialogManager);
public:
    static void ShowInEmbeddedOverlay(std::function<void(RefPtr<NG::OverlayManager>)>&& task,
        const std::string& name, int32_t uniqueId);
    static RefPtr<OverlayManager> GetEmbeddedOverlay(int32_t uniqueId, const RefPtr<PipelineContext>& context);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_DIALOG_MANAGER_H