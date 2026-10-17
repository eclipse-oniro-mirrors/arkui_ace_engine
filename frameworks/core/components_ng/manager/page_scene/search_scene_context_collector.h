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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_SEARCH_SCENE_CONTEXT_COLLECTOR_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_SEARCH_SCENE_CONTEXT_COLLECTOR_H

#include <string>
#include <vector>

#include "core/components_ng/manager/page_scene/search_scene_snapshot.h"

namespace OHOS::Ace::NG {

// Collects the current UIContent's main window (page + class-dialog overlays)
// and ArkUI subwindow traversal contexts, mapping subwindow rects into
// main-window coordinates (ADR-2, O-4 confirmed formula:
// subPipeline->GetCurrentWindowRect().GetOffset() - mainPipeline->GetCurrentWindowRect().GetOffset()).
class ACE_FORCE_EXPORT SearchSceneContextCollector final {
public:
    struct SearchPageContext {
        // Event PageName (main top page name; subwindow hits use `<main>+Dialog` or `<main>+Sheet`).
        std::string pageName;
        // Rule page matching values, fetched independently by pageType.
        std::string navDestinationName;
        std::string pageUrl;
        std::vector<SearchWindowContext> windows;
    };

    SearchPageContext Collect();

private:
    void CollectWindowContexts(int32_t instanceId, bool isSubwindow, SearchPageContext& context);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_PAGE_SCENE_SEARCH_SCENE_CONTEXT_COLLECTOR_H
