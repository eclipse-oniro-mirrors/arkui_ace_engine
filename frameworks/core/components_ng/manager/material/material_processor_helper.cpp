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

#include "core/components_ng/manager/material/material_processor_helper.h"

#include "core/components_ng/base/frame_node.h"
#include "core/pipeline/base/element_register.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "core/components_ng/pattern/sheet/sheet_presentation_pattern.h"
#include "core/components_ng/pattern/stage/page_pattern.h"

namespace OHOS::Ace::NG {
namespace {
const std::string NAVBAR_DEFAULT_NAME = "navbar";

std::string GetPageUrlByContainerId(int32_t containerId)
{
    auto node = AceType::DynamicCast<FrameNode>(ElementRegister::GetInstance()->GetUINodeById(containerId));
    CHECK_NULL_RETURN(node, "");
    if (node->GetTag() == V2::PAGE_ETS_TAG) {
        auto pagePattern = node->GetPattern<PagePattern>();
        CHECK_NULL_RETURN(pagePattern, "");
        return pagePattern->GetPageUrl();
    }
    auto ci = CollectContainerInfo(node);
    if (ci.pageId == -1) {
        return "";
    }
    auto pageFrameNode = AceType::DynamicCast<FrameNode>(ElementRegister::GetInstance()->GetUINodeById(ci.pageId));
    CHECK_NULL_RETURN(pageFrameNode, "");
    auto pagePattern = pageFrameNode->GetPattern<PagePattern>();
    CHECK_NULL_RETURN(pagePattern, "");
    return pagePattern->GetPageUrl();
}
} // namespace

ContainerInfo CollectContainerInfo(const RefPtr<FrameNode>& node)
{
    ContainerInfo info;
    for (auto parent = node->GetParent(); parent; parent = parent->GetParent()) {
        const auto& tag = parent->GetTag();
        if (info.navigationId == -1 && tag == V2::NAVIGATION_VIEW_ETS_TAG) {
            info.navigationId = parent->GetId();
        }
        if (info.tabsId == -1 && tag == V2::TABS_ETS_TAG) {
            info.tabsId = parent->GetId();
        }
        if (info.pageId == -1 && tag == V2::PAGE_ETS_TAG) {
            info.pageId = parent->GetId();
        }
        if (info.navDestinationId == -1 && tag == V2::NAVDESTINATION_VIEW_ETS_TAG) {
            info.navDestinationId = parent->GetId();
        }
        if (info.navBarId == -1 && tag == V2::NAVBAR_ETS_TAG) {
            info.navBarId = parent->GetId();
        }
        if (info.tabContentId == -1 && tag == V2::TAB_CONTENT_ITEM_ETS_TAG) {
            info.tabContentId = parent->GetId();
        }
        if (info.containerId == -1 &&
            (tag == V2::PAGE_ETS_TAG || tag == V2::NAVIGATION_VIEW_ETS_TAG || tag == V2::TABS_ETS_TAG)) {
            info.containerId = parent->GetId();
        }
    }
    return info;
}

void ResolveTargetNames(int32_t targetContainerId, PageLogContainerType type,
    std::string& navName, std::string& pageName)
{
    auto targetNode =
        AceType::DynamicCast<FrameNode>(ElementRegister::GetInstance()->GetUINodeById(targetContainerId));
    if (!targetNode) {
        return;
    }
    if (targetNode->GetTag() == V2::NAVDESTINATION_VIEW_ETS_TAG) {
        auto navDestPattern = targetNode->GetPattern<NavDestinationPattern>();
        if (navDestPattern) {
            navName = navDestPattern->GetName();
        }
    } else if (type == PageLogContainerType::NavContainer && targetNode->GetTag() == V2::NAVBAR_ETS_TAG) {
        navName = NAVBAR_DEFAULT_NAME;
    }
    pageName = GetPageUrlByContainerId(targetContainerId);
}

bool GetOverlayDimensions(const RefPtr<FrameNode>& host, double& width, double& height, double& area)
{
    auto geometryNode = host->GetGeometryNode();
    if (!geometryNode) {
        return false;
    }
    auto size = geometryNode->GetFrameSize();
    width = size.Width();
    height = size.Height();
    if (host->GetTag() == V2::SHEET_PAGE_TAG) {
        auto pattern = host->GetPattern<SheetPresentationPattern>();
        if (!pattern) {
            return false;
        }
        if (!pattern->IsSheetBottomStyle()) {
            height = pattern->GetCenterHeight();
        } else {
            height = pattern->GetHeight();
        }
    }
    area = width * height;
    return !NearZero(area);
}

} // namespace OHOS::Ace::NG
