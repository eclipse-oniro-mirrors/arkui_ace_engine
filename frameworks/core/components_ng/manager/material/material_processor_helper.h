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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_MATERIAL_MATERIAL_PROCESSOR_HELPER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_MATERIAL_MATERIAL_PROCESSOR_HELPER_H

#include <cstdint>
#include <string>

#include "core/components_ng/manager/material/material_processor.h"

namespace OHOS::Ace::NG {

// Result of walking a node's ancestor chain to locate the nearest container
// nodes. Pure structural lookup (by tag), no dependency on concrete patterns.
struct ContainerInfo {
    int32_t navigationId = -1;
    int32_t tabsId = -1;
    int32_t pageId = -1;
    int32_t navDestinationId = -1;
    int32_t navBarId = -1;
    int32_t tabContentId = -1;
    int32_t containerId = -1;
};

// Walks the ancestor chain of `node` collecting the nearest container ids
// (navigation / tabs / page / navDestination / navBar / tabContent).
ContainerInfo CollectContainerInfo(const RefPtr<FrameNode>& node);

// Resolves navName / pageName for the page-context stat batch from the target
// container id and the container type. Reaches into NavDestinationPattern and
// PagePattern, so it is isolated here to keep material_processor decoupled from
// concrete component patterns.
void ResolveTargetNames(int32_t targetContainerId, PageLogContainerType type,
    std::string& navName, std::string& pageName);

// Computes the effective overlay area dimensions of `host`. For SheetPage the
// height is taken from SheetPresentationPattern; otherwise from the geometry
// node. Returns false when the dimensions are unusable (no geometry, missing
// sheet pattern, or zero area).
bool GetOverlayDimensions(const RefPtr<FrameNode>& host, double& width, double& height, double& area);

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_MATERIAL_MATERIAL_PROCESSOR_HELPER_H
