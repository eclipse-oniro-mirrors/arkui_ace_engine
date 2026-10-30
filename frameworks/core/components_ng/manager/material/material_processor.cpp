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

#include "core/components_ng/manager/material/material_processor.h"

#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/log/material_statistic_event_info.h"
#include "core/common/statistic_event_reporter.h"
#include "core/components/common/properties/ui_material.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/geometry_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/render/render_context.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/components_ng/pattern/sheet/sheet_presentation_pattern.h"
#include "core/components_ng/pattern/navrouter/navdestination_pattern.h"
#include "core/components_ng/pattern/stage/page_pattern.h"

namespace OHOS::Ace::NG {
namespace {
const std::vector<std::string> TARGET_TAG = { "ActionSheet", "AlertDialog", "AlphabetIndexer", "SheetPage", "Dialog",
    "Slider", "Toast", "Toggle" };
const std::string TARGET_TEXT_COPY = "SelectOverlayNode ShowSelectOverlay";
const std::string TARGET_TEXT_COPY_EXT = "extensionMenu";
const std::string TARGET_MENU_SELECT_MENU = "MenuManager OnShowMenuAnimationFinished";
const std::string TARGET_SUB_MENU = "MenuItem ShowSubMenuHelper";
const std::string TARGET_POPUP_TIPS = "overlay ShowPopupAnimationNG";
const std::string TARGET_SELECT = "Select ShowSelectMenu";
constexpr float MATERIAL_AREA_REPORT_THRESHOLD = 0.2f;
const std::string NAVBAR_DEFAULT_NAME = "navbar";

struct PageContextStatBatch {
    int32_t navContainerId = -1;
    std::vector<MaterialNodeInfo> navNodes;
    int32_t tabsContainerId = -1;
    std::vector<MaterialNodeInfo> tabsNodes;
    int32_t pageContainerId = -1;
    std::vector<MaterialNodeInfo> pageNodes;
    double screenWidth = 0.0;
    double screenHeight = 0.0;
    std::string navName;
    std::string pageName;
    int32_t tabIndex = -1;
};

struct ContainerInfo {
    int32_t navigationId = -1;
    int32_t tabsId = -1;
    int32_t pageId = -1;
    int32_t navDestinationId = -1;
    int32_t navBarId = -1;
    int32_t tabContentId = -1;
    int32_t containerId = -1;
};

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

void FlushOverlayMaterialArea(int32_t nodeId, std::string tag, double width, double height, double area,
    double screenWidth, double screenHeight, int64_t deadline, const WeakPtr<PipelineContext>& pipelineWk)
{
    if (GetSysTimestamp() > deadline) {
        auto pipeline = pipelineWk.Upgrade();
        CHECK_NULL_VOID(pipeline);
        pipeline->AddPredictTask([nodeId, tag = std::move(tag), width, height, area, screenWidth, screenHeight,
                                     pipelineWk](int64_t deadline, bool /*canUseLongPredictTask*/) {
            FlushOverlayMaterialArea(
                nodeId, std::move(tag), width, height, area, screenWidth, screenHeight, deadline, pipelineWk);
        });
        return;
    }
    auto pipeline = pipelineWk.Upgrade();
    CHECK_NULL_VOID(pipeline);
    RichOverlayMaterialAreaData data;
    data.nodeId = nodeId;
    data.tag = std::move(tag);
    data.width = width;
    data.height = height;
    data.area = area;
    data.screenWidth = screenWidth;
    data.screenHeight = screenHeight;
    double screenArea = std::max(1e-6, screenWidth * screenHeight);
    data.areaPercent = area / screenArea;
    if (data.areaPercent < MATERIAL_AREA_REPORT_THRESHOLD) {
        return;
    }
    pipeline->GetStatisticEventReporter()->SendOverlayMaterialArea(data);
}

void FlushPageContextStats(PageContextStatBatch batch, int64_t deadline, const WeakPtr<PipelineContext>& pipelineWk)
{
    if (GetSysTimestamp() > deadline) {
        auto pipeline = pipelineWk.Upgrade();
        CHECK_NULL_VOID(pipeline);
        pipeline->AddPredictTask(
            [batch = std::move(batch), pipelineWk](int64_t deadline, bool /*canUseLongPredictTask*/) {
                FlushPageContextStats(std::move(batch), deadline, pipelineWk);
            });
        return;
    }
    int32_t navCount = batch.navNodes.size();
    int32_t tabsCount = batch.tabsNodes.size();
    int32_t pageCount = batch.pageNodes.size();
    std::vector<MaterialNodeInfo> totalNodes;
    totalNodes.insert(totalNodes.end(), batch.navNodes.begin(), batch.navNodes.end());
    totalNodes.insert(totalNodes.end(), batch.tabsNodes.begin(), batch.tabsNodes.end());
    totalNodes.insert(totalNodes.end(), batch.pageNodes.begin(), batch.pageNodes.end());
    if (!totalNodes.empty()) {
        auto pipeline = pipelineWk.Upgrade();
        CHECK_NULL_VOID(pipeline);
        RichMaterialNodeStatData data;
        data.containerId = batch.navContainerId;
        data.containerType = "navigation-tabs-page";
        data.entries.reserve(totalNodes.size());
        for (const auto& node : totalNodes) {
            RichMaterialNodeEntry entry;
            entry.nodeId = node.nodeId;
            entry.tag = node.tag;
            entry.containerId = node.containerId;
            entry.width = node.width;
            entry.height = node.height;
            entry.area = node.area;
            entry.inTitleBar = node.inTitleBar;
            entry.inBottomTabBar = node.inBottomTabBar;
            data.entries.emplace_back(std::move(entry));
        }
        data.navCount = navCount;
        data.tabsCount = tabsCount;
        data.pageCount = pageCount;
        data.screenWidth = batch.screenWidth;
        data.screenHeight = batch.screenHeight;
        double screenArea = std::max(1e-6, batch.screenWidth * batch.screenHeight);
        double materialArea = 0.0;
        for (const auto& node : totalNodes) {
            materialArea += node.area;
        }
        data.areaPercent = materialArea / screenArea;
        if (data.areaPercent < MATERIAL_AREA_REPORT_THRESHOLD) {
            return;
        }
        data.navName = batch.navName;
        data.pageName = batch.pageName;
        data.tabIndex = batch.tabIndex;
        pipeline->GetStatisticEventReporter()->SendMaterialNodeStat(data);
    }
}

std::string GetTargetTag(const std::string& tag, const std::string& parentTag, const std::string& source)
{
    std::string targetTag = "";
    for (auto str : TARGET_TAG) {
        if (parentTag == str) {
            targetTag = str;
            break;
        }
        if (tag == str) {
            targetTag = str;
            break;
        }
    }
    if (source == TARGET_TEXT_COPY) {
        targetTag = "Text Copy";
    }
    if (source == TARGET_TEXT_COPY_EXT) {
        targetTag = "Text Copy ExtMenu";
    }
    if (source == TARGET_MENU_SELECT_MENU) {
        targetTag = "Menu or Select Menu";
    }
    if (source == TARGET_POPUP_TIPS) {
        targetTag = "Popup or Tips";
    }
    if (source == TARGET_SUB_MENU) {
        targetTag = "Sub Menu";
    }
    return targetTag;
}
} // namespace

void MaterialProcessor::Register(const RefPtr<FrameNode>& node)
{
    CHECK_NULL_VOID(node);
    materialNodes_[node->GetId()] = WeakPtr<FrameNode>(node);
}

void MaterialProcessor::RegisterPageContextNode(const RefPtr<FrameNode>& node)
{
    CHECK_NULL_VOID(node);
    std::lock_guard<std::mutex> lock(pageContextMutex_);
    pageContextStats_[node->GetId()] = WeakPtr<FrameNode>(node);
}

void MaterialProcessor::Unregister(int32_t nodeId)
{
    materialNodes_.erase(nodeId);
}

void MaterialProcessor::UnregisterPageContextNode(int32_t nodeId)
{
    std::lock_guard<std::mutex> lock(pageContextMutex_);
    pageContextStats_.erase(nodeId);
}

void MaterialProcessor::ReportOverlayNodeMaterialArea(const RefPtr<FrameNode>& host, const std::string& source)
{
    if (source == TARGET_SELECT) {
        return;
    }
    CHECK_NULL_VOID(host);

    auto parentNode = host->GetParent();
    std::string parentTag = "";
    if (parentNode) {
        parentTag = parentNode->GetTag();
    }

    auto rc = host->GetRenderContext();
    CHECK_NULL_VOID(rc);
    auto material = rc->GetSystemMaterial();
    bool notImmersive = (!material || material->GetType() != static_cast<int32_t>(MaterialType::IMMERSIVE));
    if (parentTag != V2::TOGGLE_ETS_TAG && parentTag != V2::SLIDER_ETS_TAG) {
        if (notImmersive) {
            return;
        }
    }

    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto size = geometryNode->GetFrameSize();
    double width = size.Width();
    double height = size.Height();
    if (host->GetTag() == V2::SHEET_PAGE_TAG) {
        auto pattern = host->GetPattern<SheetPresentationPattern>();
        CHECK_NULL_VOID(pattern);
        if (!pattern->IsSheetBottomStyle()) {
            height = pattern->GetCenterHeight();
        } else {
            height = pattern->GetHeight();
        }
    }
    double area = width * height;
    if (NearZero(area)) {
        return;
    }

    auto pipeline = NG::PipelineContext::GetCurrentContextSafelyWithCheck();
    CHECK_NULL_VOID(pipeline);
    double screenWidth = pipeline->GetRootWidth();
    double screenHeight = pipeline->GetRootHeight();
    if (NearZero(screenHeight * screenWidth)) {
        return;
    }
    auto nodeId = host->GetId();
    auto tag = host->GetTag();
    auto pipelineWk = WeakPtr<PipelineContext>(pipeline);

    std::string targetTag = GetTargetTag(tag, parentTag, source);

    pipeline->AddPredictTask([nodeId, tag = std::move(targetTag), width, height, area, screenWidth, screenHeight,
                                 pipelineWk](int64_t deadline, bool /*canUseLongPredictTask*/) {
        FlushOverlayMaterialArea(
            nodeId, std::move(tag), width, height, area, screenWidth, screenHeight, deadline, pipelineWk);
    });
}

void MaterialProcessor::ApplyScopeGate(const RefPtr<PipelineContext>& pipeline)
{
    CHECK_NULL_VOID(pipeline);
    if (MaterialUtils::IsMaterialDisabled()) {
        return;
    }
    for (auto it = materialNodes_.begin(); it != materialNodes_.end();) {
        auto frameNode = it->second.Upgrade();
        if (!frameNode || frameNode->IsInDestroying() || frameNode->GetContext() != pipeline.GetRawPtr()) {
            it = materialNodes_.erase(it);
            continue;
        }
        const auto& rc = frameNode->GetRenderContext();
        if (!rc || !frameNode->IsVisibleAndActive()) {
            ++it;
            continue;
        }
        // Scope gate: system apps keep material effective everywhere; non-system
        // apps only inside a titleBar or a bottom Tabs TabBar subtree (both tracked
        // by inherited flags, O(1)). Unrestricted components (Slider / Toggle) and
        // overlay-exempt components (popup / dialog / menu / sheet / toast / select)
        // are always effective regardless of scope or system-app status.
        bool inScope = frameNode->IsInTitleBar() || frameNode->IsInBottomTabBar();
        bool effective = MaterialUtils::IsMaterialUnrestrictedComponent(frameNode->GetTag()) ||
                         MaterialUtils::IsSystemApp() || inScope ||
                         rc->IsMaterialScopeExempt() ||
                         frameNode->IsInCustomSelectMenu();
        if (!effective) {
            // Not in scope -> fully remove the material. Save it first so it can be
            // restored if the node re-enters a titleBar / bottom TabBar.
            if (!rc->IsMaterialSuppressed()) {
                auto material = rc->GetSystemMaterial();
                if (material) {
                    rc->SetSavedMaterialForSuppress(material);
                    // Mark suppressed BEFORE the clear so RenderContext::SetSystemMaterial
                    // keeps the node registered (intended = suppressed).
                    rc->SetMaterialSuppressed(true);
                    // Guard this limiter-owned clear so ViewAbstract::SetSystemMaterial
                    // does not block it as an "external call on a suppressed node".
                    rc->SetMaterialLimiterUpdating(true);
                    ViewAbstract::SetSystemMaterial(AceType::RawPtr(frameNode), nullptr);
                    LOGW("Material inactive: out of scope. Use component in navigation title bar or Tabbar.");
                    rc->SetMaterialLimiterUpdating(false);
                    frameNode->MarkDirtyNode();
                    auto pattern = frameNode->GetPattern();
                    if (pattern) {
                        pattern->OnMaterialDisable();
                    }
                }
            }
        } else if (rc->IsMaterialSuppressed()) {
            // Back in scope: restore the saved material.
            auto saved = rc->GetSavedMaterialForSuppress();
            rc->SetSavedMaterialForSuppress(nullptr);
            // Guard this limiter-owned restore so ViewAbstract::SetSystemMaterial
            // does not block it (the node is still flagged suppressed here).
            rc->SetMaterialLimiterUpdating(true);
            ViewAbstract::SetSystemMaterial(AceType::RawPtr(frameNode), saved.GetRawPtr());
            rc->SetMaterialSuppressed(false);
            rc->SetMaterialLimiterUpdating(false);
            frameNode->MarkDirtyNode();
        }
        ++it;
    }
}

void MaterialProcessor::ReportPageContextStats(int32_t targetContainerId, PageLogContainerType type, int32_t tabIndex)
{
    std::vector<MaterialNodeInfo> navNodes;
    std::vector<MaterialNodeInfo> tabsNodes;
    std::vector<MaterialNodeInfo> pageNodes;

    std::unique_lock<std::mutex> lock(pageContextMutex_);
    for (auto it = pageContextStats_.begin(); it != pageContextStats_.end();) {
        auto node = it->second.Upgrade();
        if (!node) {
            it = pageContextStats_.erase(it);
            continue;
        }

        auto ci = CollectContainerInfo(node);
        int32_t navId = ci.navigationId;
        int32_t tabsId = ci.tabsId;
        int32_t pageId = ci.pageId;
        int32_t nearestComponentId = ci.containerId;
        int32_t navDesContainerId = ci.navDestinationId;
        int32_t navBarContainerId = ci.navBarId;
        int32_t tabContainerId = ci.tabContentId;

        bool needContinue = false;
        {
            auto tabContainerNode =
                AceType::DynamicCast<NG::FrameNode>(ElementRegister::GetInstance()->GetUINodeById(tabContainerId));
            auto navContainerNode = AceType::DynamicCast<NG::FrameNode>(ElementRegister::GetInstance()->GetUINodeById(
                (navDesContainerId != -1) ? navDesContainerId : navBarContainerId));
            if (tabContainerNode) {
                bool tabContainerVisible = tabContainerNode->IsVisible();
                if (!tabContainerVisible) {
                    needContinue = true;
                }
            }
            if (navContainerNode) {
                bool navContainerVisible = navContainerNode->IsVisible();
                if (!navContainerVisible) {
                    needContinue = true;
                }
            }

            if (!node->IsVisibleAndActive()) {
                needContinue = true;
            }
        }

        if (type == PageLogContainerType::Page) {
            if (tabsId == -1 && navId == -1 && pageId == -1) {
                needContinue = true;
            }
            if (pageId != -1 && pageId != targetContainerId) {
                needContinue = true;
            }
        } else if (type == PageLogContainerType::NavContainer) {
            if (navId == -1 || (navBarContainerId == -1 && navDesContainerId == -1)) {
                needContinue = true;
            }
            if ((navBarContainerId != -1 && navBarContainerId != targetContainerId) ||
                (navDesContainerId != -1 && navDesContainerId != targetContainerId)) {
                needContinue = true;
            }
        } else if (type == PageLogContainerType::TabContainer) {
            if (tabsId == -1) {
                needContinue = true;
            } else if (tabContainerId != -1 && tabContainerId != targetContainerId) {
                needContinue = true;
            }
        }
        auto geometryNode = node->GetGeometryNode();
        if (!geometryNode) {
            needContinue = true;
        }
        if (needContinue) {
            ++it;
            continue;
        }
        bool inTitleBar = node->IsInTitleBar();
        bool inBottomTabBar = node->IsInBottomTabBar();
        auto size = geometryNode->GetFrameSize();
        double width = size.Width();
        double height = size.Height();
        MaterialNodeInfo info;
        info.nodeId = node->GetId();
        info.tag = node->GetTag();
        info.width = width;
        info.height = height;
        info.area = width * height;
        info.inTitleBar = inTitleBar;
        info.inBottomTabBar = inBottomTabBar;
        if (navId != -1 && navId == nearestComponentId) {
            info.containerId = navId;
            navNodes.push_back(info);
        } else if (tabsId != -1 && tabsId == nearestComponentId) {
            info.containerId = tabsId;
            tabsNodes.push_back(info);
        } else {
            info.containerId = pageId;
            pageNodes.push_back(info);
        }
        ++it;
    }
    lock.unlock();

    int32_t navContainerId = navNodes.empty() ? -1 : navNodes[0].containerId;
    int32_t tabsContainerId = tabsNodes.empty() ? -1 : tabsNodes[0].containerId;
    int32_t pageContainerId = pageNodes.empty() ? -1 : pageNodes[0].containerId;

    std::string navName;
    std::string pageName;
    auto targetNode =
        AceType::DynamicCast<NG::FrameNode>(ElementRegister::GetInstance()->GetUINodeById(targetContainerId));
    if (targetNode) {
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

    auto pipeline = NG::PipelineContext::GetCurrentContextSafelyWithCheck();
    CHECK_NULL_VOID(pipeline);
    PageContextStatBatch batch;
    batch.navContainerId = navContainerId;
    batch.navNodes = std::move(navNodes);
    batch.tabsContainerId = tabsContainerId;
    batch.tabsNodes = std::move(tabsNodes);
    batch.pageContainerId = pageContainerId;
    batch.pageNodes = std::move(pageNodes);
    batch.screenWidth = pipeline->GetRootWidth();
    batch.screenHeight = pipeline->GetRootHeight();
    batch.navName = std::move(navName);
    batch.pageName = std::move(pageName);
    batch.tabIndex = tabIndex;
    if (NearZero(batch.screenHeight * batch.screenWidth)) {
        return;
    }
    auto pipelineWk = WeakPtr<PipelineContext>(pipeline);
    pipeline->AddPredictTask([batch = std::move(batch), pipelineWk](int64_t deadline, bool /*canUseLongPredictTask*/) {
        FlushPageContextStats(std::move(batch), deadline, pipelineWk);
    });
}

} // namespace OHOS::Ace::NG
