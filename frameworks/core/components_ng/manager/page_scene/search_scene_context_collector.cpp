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

#include "core/components_ng/manager/page_scene/search_scene_context_collector.h"

#include "base/log/log_wrapper.h"
#include "base/subwindow/subwindow_manager.h"
#include "core/common/ace_engine.h"
#include "core/components_ng/dump_utils/dump_util.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

SearchSceneContextCollector::SearchPageContext SearchSceneContextCollector::Collect()
{
    SearchPageContext context;
    auto pipeline = NG::PipelineContext::GetCurrentContextSafely();
    if (!pipeline) {
        LOGW("SearchSceneContextCollector pipeline is null");
        return context;
    }
    context.navDestinationName = pipeline->GetCurrentNavDestinationName();
    context.pageUrl = pipeline->GetCurrentPageUrl();
    context.pageName = pipeline->GetCurrentPageName();

    CollectWindowContexts(pipeline->GetInstanceId(), false, context);
    auto subContainerIds = SubwindowManager::GetInstance()->GetAllSubContainerId(pipeline->GetInstanceId());
    for (const auto& subContainerId : subContainerIds) {
        CollectWindowContexts(subContainerId, true, context);
    }
    return context;
}

void SearchSceneContextCollector::CollectWindowContexts(
    int32_t instanceId, bool isSubwindow, SearchPageContext& context)
{
    auto container = AceEngine::Get().GetContainer(instanceId);
    if (!container) {
        return;
    }
    auto pipelineBase = container->GetPipelineContext();
    auto pipeline = AceType::DynamicCast<NG::PipelineContext>(pipelineBase);
    if (!pipeline) {
        return;
    }
    auto mainPipeline = NG::PipelineContext::GetCurrentContextSafely();
    if (!mainPipeline) {
        return;
    }
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    if (isSubwindow) {
        auto mainOffset = mainPipeline->GetCurrentWindowRect().GetOffset();
        auto subOffset = pipeline->GetCurrentWindowRect().GetOffset();
        offsetX = subOffset.GetX() - mainOffset.GetX();
        offsetY = subOffset.GetY() - mainOffset.GetY();
    }
    auto startNodes = pipeline->GetDumpStartNodes();
    for (const auto& root : startNodes.pageStartNodes) {
        if (root) {
            context.windows.emplace_back(SearchWindowContext { root, offsetX, offsetY, isSubwindow });
        }
    }
    for (const auto& overlay : startNodes.overlayNodes) {
        if (overlay) {
            context.windows.emplace_back(SearchWindowContext { overlay, offsetX, offsetY, isSubwindow });
        }
    }
}

} // namespace OHOS::Ace::NG
