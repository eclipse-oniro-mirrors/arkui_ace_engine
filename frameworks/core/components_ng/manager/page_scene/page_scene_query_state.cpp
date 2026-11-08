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

#include "core/common/container.h"
#include "core/components_ng/manager/content_change_manager/content_change_manager.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
// Keep window eligibility with PageScene manager implementation; FrameNode only forwards node changes.
// Defined unconditionally: declared in the public header and referenced from unguarded manager code.
bool ContentChangeManager::IsPageSceneSplitWindowSupported(const PipelineContext* pipeline)
{
    if (!pipeline || pipeline->IsSubPipeline()) {
        return false;
    }
    auto container = Container::GetContainer(pipeline->GetInstanceId());
    return !container || (!container->IsSubContainer() && !container->IsSubWindow() && !container->IsDialogWindow());
}

bool ContentChangeManager::IsPageSceneSplitInSubwindow()
{
    auto pipeline = PipelineContext::GetCurrentContextSafely();
    return pipeline && !IsPageSceneSplitWindowSupported(AceType::RawPtr(pipeline));
}

} // namespace OHOS::Ace::NG
