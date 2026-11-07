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

#include "parallel_page_router_manager.h"
#include "core/components_ng/manager/force_split/force_split_manager.h"
#include "bridge/declarative_frontend/ng/force_split/parallel_page_router_manager.h"

#include <vector>
#include <sstream>
#include <algorithm>

#include "bridge/common/utils/utils.h"
#include "bridge/common/utils/engine_helper.h"
#include "core/common/force_split/force_split_utils.h"
#include "core/components_ng/base/view_advanced_register.h"
#include "core/components_ng/pattern/stage/force_split/parallel_page_pattern.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/image/image_layout_property.h"
#include "core/components/theme/resource_adapter.h"
#include "core/components_ng/pattern/stage/force_split/parallel_stage_manager.h"

namespace OHOS::Ace::NG {

namespace {
constexpr char PLACEHOLDER_PAGE_URL[] = "placeholder";
constexpr char PLACEHOLDER_PAGE_PATH[] = "placeholder.js";
constexpr int32_t HOME_PAGE_NODE_THRESHOLD = 100;
constexpr int32_t MAX_ROUTER_STACK_SIZE = 32;
const std::vector<std::string> HOME_PAGE_PREFIX = {"main", "home", "index", "root"};
}

std::vector<std::string> ParallelPageRouterManager::CollectPageStackNames(const std::string& curTopPageName)
{
    std::vector<std::string> pageStack;
    for (const auto& weakNode : pageRouterStack_) {
        auto pageNode = weakNode.Upgrade();
        if (!pageNode) {
            continue;
        }
        auto pagePattern = pageNode->GetPattern<PagePattern>();
        if (!pagePattern) {
            continue;
        }
        auto pageUrl = pagePattern->GetPageUrl();
        if (!pageUrl.empty()) {
            pageStack.emplace_back(std::move(pageUrl));
        }
    }
    // This notification happens before the router stack is updated, so reconcile the
    // incoming top page name: append it for push, or drop pages above it for pop.
    if (!curTopPageName.empty()) {
        if (pageStack.empty()) {
            pageStack.emplace_back(curTopPageName);
        } else if (pageStack.back() != curTopPageName) {
            auto it = std::find(pageStack.rbegin(), pageStack.rend(), curTopPageName);
            if (it != pageStack.rend()) {
                // Keep the last matching page and remove only the pages above it.
                pageStack.erase(it.base(), pageStack.end());
            } else {
                pageStack.emplace_back(curTopPageName);
            }
        }
    }
    return pageStack;
}

void ParallelPageRouterManager::NotifyForceFullScreenChangeIfNeeded(
    const std::string& curTopPageName, const RefPtr<PipelineContext>& context)
{
    CHECK_NULL_VOID(context);
    auto forceSplitMgr = context->GetForceSplitManager();
    CHECK_NULL_VOID(forceSplitMgr);
    if (!forceSplitMgr->IsForceSplitSupported(true)) {
        return;
    }
    bool isTopPageSplitDisabledChanged = false;
    if (forceSplitMgr->IsPageLevelSplitEnabled()) {
        // Sync the page stack first: page-level split inheritance is resolved from it.
        forceSplitMgr->SetPageStack(CollectPageStackNames(curTopPageName));
        bool isPageSplitDisabled = forceSplitMgr->IsPageSplitDisabled(curTopPageName);
        isTopPageSplitDisabledChanged = forceSplitMgr->UpdateIsTopPageSplitDisabled(isPageSplitDisabled);
        TAG_LOGI(AceLogTag::ACE_ROUTER,
            "pageLevelSplit notify: page=%{public}s, disabled=%{public}d, changed=%{public}d",
            curTopPageName.c_str(), isPageSplitDisabled, isTopPageSplitDisabledChanged);
    }
    auto stageManager = AceType::DynamicCast<ParallelStageManager>(context->GetStageManager());
    CHECK_NULL_VOID(stageManager);
    // when page-level split control is on, fullScreenPages takes no effect
    bool isCurFullScreen = forceSplitMgr->IsPageLevelSplitEnabled() ?
        forceSplitMgr->IsTopPageSplitDisabled() : forceSplitMgr->IsFullScreenPage(curTopPageName);
    if (stageManager->IsTopFullScreenPage() != isCurFullScreen) {
        stageManager->UpdateIsTopFullScreenPage(isCurFullScreen);
        forceSplitMgr->NotifyForceFullScreenChange(stageManager->IsTopFullScreenPage());
        bool isPreInSplitMode = stageManager->IsSplitMode();
        // try to update mode immediately
        forceSplitMgr->NotifyForceSplitStateChange();
        bool isCurInSplitMode = stageManager->IsSplitMode();
        stageManager->SetNeedOperationInSplitModeWithFullScreen(isPreInSplitMode || isCurInSplitMode);
        return;
    }
    if (isTopPageSplitDisabledChanged) {
        // page-level split switch state changed, try to update mode immediately
        forceSplitMgr->NotifyForceSplitStateChange();
    }
}

void ParallelPageRouterManager::TryEstablishPageLevelSplitHome(
    const std::string& pageUrl, const RefPtr<FrameNode>& pageNode, const RefPtr<PipelineContext>& context)
{
    CHECK_NULL_VOID(context);
    auto forceSplitMgr = context->GetForceSplitManager();
    CHECK_NULL_VOID(forceSplitMgr);
    if (!forceSplitMgr->IsPageLevelSplitEnabled()) {
        return;
    }
    CHECK_NULL_VOID(pageNode);
    // Collect the effective stack by target page instance, preserving distinct instances of pages that share the
    // same name. LoadPage collects the newly pushed page; StartPop collects the pop target page.
    std::vector<std::string> pageNames;
    std::vector<RefPtr<FrameNode>> effectivePages;
    RefPtr<FrameNode> belowTopPage;
    bool foundTarget = false;
    for (const auto& weakNode : pageRouterStack_) {
        auto node = weakNode.Upgrade();
        if (!node) {
            continue;
        }
        auto pattern = node->GetPattern<PagePattern>();
        if (!pattern) {
            continue;
        }
        pageNames.emplace_back(pattern->GetPageUrl());
        if (node == pageNode) {
            belowTopPage = effectivePages.empty() ? nullptr : effectivePages.back();
            effectivePages.emplace_back(node);
            foundTarget = true;
            break;
        }
        effectivePages.emplace_back(node);
    }
    if (!foundTarget) {
        return;
    }
    // The policy stack must be updated before querying the split policies of the current page and its upstream pages.
    forceSplitMgr->SetPageStack(std::move(pageNames));
    if (forceSplitMgr->IsPageSplitDisabled(pageUrl)) {
        return;
    }
    if (forceSplitMgr->GetBehaviorMode() == ForceSplitBehaviorMode::NAVIGATION) {
        UpdatePageLevelNavigationHome(effectivePages, forceSplitMgr, context);
        return;
    }
    UpdatePageLevelSplitHome(pageNode, belowTopPage, effectivePages, forceSplitMgr, context);
}

RefPtr<FrameNode> ParallelPageRouterManager::SelectPageLevelNavigationHome(
    const std::vector<RefPtr<FrameNode>>& effectivePages,
    const RefPtr<ForceSplitManager>& forceSplitMgr)
{
    CHECK_NULL_RETURN(forceSplitMgr, nullptr);
    if (effectivePages.empty()) {
        return nullptr;
    }
    pageLevelNavigationHomes_.erase(
        std::remove_if(pageLevelNavigationHomes_.begin(), pageLevelNavigationHomes_.end(),
            [&effectivePages](const auto& item) {
                auto node = item.Upgrade();
                return !node ||
                    std::find(effectivePages.begin(), effectivePages.end(), node) == effectivePages.end();
            }),
        pageLevelNavigationHomes_.end());
    auto home = pageLevelNavigationHomes_.empty() ? nullptr : pageLevelNavigationHomes_.back().Upgrade();
    bool reachedHome = false;
    bool separatedByNonSplit = false;
    std::vector<std::string> names;
    for (const auto& page : effectivePages) {
        auto pattern = page->GetPattern<PagePattern>();
        CHECK_NULL_CONTINUE(pattern);
        names.emplace_back(pattern->GetPageUrl());
        // Resolve inheritance using this instance's stack prefix.
        forceSplitMgr->SetPageStack(std::vector<std::string>(names));
        if (page == home) {
            reachedHome = true;
        }
        if (reachedHome && forceSplitMgr->IsPageSplitDisabled(pattern->GetPageUrl())) {
            separatedByNonSplit = true;
        }
    }
    return reachedHome && !separatedByNonSplit ? home : effectivePages.back();
}

void ParallelPageRouterManager::UpdatePageLevelNavigationHome(const std::vector<RefPtr<FrameNode>>& effectivePages,
    const RefPtr<ForceSplitManager>& forceSplitMgr, const RefPtr<PipelineContext>& context)
{
    CHECK_NULL_VOID(context);
    auto stageManager = AceType::DynamicCast<ParallelStageManager>(context->GetStageManager());
    CHECK_NULL_VOID(stageManager);
    auto stageNode = stageManager->GetStageNode();
    CHECK_NULL_VOID(stageNode);
    auto stagePattern = stageNode->GetPattern<ParallelStagePattern>();
    CHECK_NULL_VOID(stagePattern);

    auto home = SelectPageLevelNavigationHome(effectivePages, forceSplitMgr);
    CHECK_NULL_VOID(home);
    auto homePattern = home->GetPattern<ParallelPagePattern>();
    CHECK_NULL_VOID(homePattern);
    auto previousHome = stagePattern->GetHomePage();
    forceSplitMgr->SetHomePageName(homePattern->GetPageUrl());
    if (previousHome != home || homePattern->GetPageType() != RouterPageType::HOME_PAGE) {
        if (!stageManager->OnHomePageDetected(home, pageRouterStack_)) {
            return;
        }
        stageNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    }

    if (pageLevelNavigationHomes_.empty() || pageLevelNavigationHomes_.back().Upgrade() != home) {
        pageLevelNavigationHomes_.push_back(home);
    }
    TAG_LOGI(AceLogTag::ACE_ROUTER,
        "pageLevelSplit navigation home, previous:%{public}d, home:%{public}d, top:%{public}d",
        previousHome ? previousHome->GetId() : -1, home->GetId(), effectivePages.back()->GetId());
}

void ParallelPageRouterManager::UpdatePageLevelSplitHome(const RefPtr<FrameNode>& pageNode,
    const RefPtr<FrameNode>& belowTopPage, const std::vector<RefPtr<FrameNode>>& effectivePages,
    const RefPtr<ForceSplitManager>& forceSplitMgr, const RefPtr<PipelineContext>& context)
{
    // Keep the existing home while the split session remains continuous.
    if (!forceSplitMgr->GetHomePageName().empty()) {
        auto stageManager = context->GetStageManager();
        CHECK_NULL_VOID(stageManager);

        auto stageNode = stageManager->GetStageNode();
        CHECK_NULL_VOID(stageNode);

        auto stagePattern = stageNode->GetPattern<ParallelStagePattern>();
        CHECK_NULL_VOID(stagePattern);

        auto existingHome = stagePattern->GetHomePage();
        const bool homeInStack = existingHome &&
            std::find(effectivePages.begin(), effectivePages.end(), existingHome) != effectivePages.end();

        bool belowTopIsSplit = false;
        auto belowTopPattern = belowTopPage ? belowTopPage->GetPattern<PagePattern>() : nullptr;
        if (belowTopPattern) {
            belowTopIsSplit =
                !forceSplitMgr->IsPageSplitDisabled(belowTopPattern->GetPageUrl());
        }
        TAG_LOGI(AceLogTag::ACE_ROUTER,
            "pageLevelSplit home continuity, home:%{public}d, homeInStack:%{public}d, "
            "below:%{public}d, belowSplit:%{public}d",
            existingHome ? existingHome->GetId() : -1, homeInStack,
            belowTopPage ? belowTopPage->GetId() : -1, belowTopIsSplit);
        if (homeInStack && belowTopIsSplit) {
            return;
        }
    }
    // NAVIGATION: current split page B is the new home -> [B | empty].
    // DISPLACE: the page below B, namely A, is the new home -> [A | B].
    auto homePageNode = pageNode;
    if (forceSplitMgr->GetBehaviorMode() == ForceSplitBehaviorMode::DISPLACE && belowTopPage) {
        homePageNode = belowTopPage;
    }

    auto homePattern = homePageNode->GetPattern<PagePattern>();
    CHECK_NULL_VOID(homePattern);
    forceSplitMgr->SetHomePageName(homePattern->GetPageUrl());
    auto stageManager = AceType::DynamicCast<ParallelStageManager>(context->GetStageManager());
    CHECK_NULL_VOID(stageManager);
    stageManager->OnHomePageDetected(homePageNode, pageRouterStack_);

    auto stageNode = stageManager->GetStageNode();
    CHECK_NULL_VOID(stageNode);
    stageNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void ParallelPageRouterManager::LoadPage(
    int32_t pageId, const RouterPageInfo& target, bool needHideLast, bool needTransition, bool isPush)
{
    ACE_SCOPED_TRACE_COMMERCIAL("load page: %s(id:%d)", target.url.c_str(), pageId);
    CHECK_RUN_ON(JS);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto stageManager = AceType::DynamicCast<ParallelStageManager>(context->GetStageManager());
    CHECK_NULL_VOID(stageManager);
    LoadPlaceholderOrRelatedPageIfNeeded(context, stageManager);

    bool needClearSecondaryPage = CheckSecondaryPageNeedClear(isPush);
    if (!CheckStackSize(target, needClearSecondaryPage)) {
        return;
    }
    NotifyForceFullScreenChangeIfNeeded(target.url, context);
    auto pageNode = CreatePage(pageId, target);
    if (!pageNode) {
        TAG_LOGE(AceLogTag::ACE_ROUTER, "failed to create page: %{public}s", target.url.c_str());
        return;
    }

    RefPtr<FrameNode> preLastPage = nullptr;
    if (!pageRouterStack_.empty()) {
        preLastPage = pageRouterStack_.rbegin()->Upgrade();
    }
    std::list<WeakPtr<FrameNode>> prevPageRouterStack;
    std::copy(pageRouterStack_.begin(), pageRouterStack_.end(), std::back_inserter(prevPageRouterStack));
    int32_t removePageSize = stageManager->IsTopFullScreenPage() ?
        0 : stageManager->UpdateSecondaryPageNeedRemoved(needClearSecondaryPage);
    while (removePageSize > 0 && !pageRouterStack_.empty()) {
        pageRouterStack_.pop_back();
        removePageSize--;
    };
    pageRouterStack_.emplace_back(pageNode);
    if (intentInfo_.has_value()) {
        if (!OnPageReadyAndHandleIntent(pageNode, needHideLast)) {
            intentInfo_.reset();
            std::swap(prevPageRouterStack, pageRouterStack_);
            TAG_LOGW(AceLogTag::ACE_ROUTER, "OnPageReadyAndHandleIntent Failed");
            return;
        }
    } else if (!OnPageReady(pageNode, needHideLast, needTransition)) {
        std::swap(prevPageRouterStack, pageRouterStack_);
        return;
    }
    stageManager->RemoveSecondaryPagesOfPrimaryPage();

    if (DetectHomePage(target, preLastPage)) {
        TAG_LOGI(AceLogTag::ACE_ROUTER, "url: %{public}s was recognised as home page", target.url.c_str());
        stageManager->OnHomePageDetected(pageNode, pageRouterStack_);
    }
    // page-level split: the first split page acts as home in navigation mode
    TryEstablishPageLevelSplitHome(target.url, pageNode, context);

    pageNode->OnAccessibilityEvent(AccessibilityEventType::CHANGE);

    if (!SetOnKeyEvent(pageNode)) {
        TAG_LOGE(AceLogTag::ACE_ROUTER, "Fail to init the keyEvent of page");
        return;
    }
    TAG_LOGI(AceLogTag::ACE_ROUTER, "LoadPage Success");
}

void ParallelPageRouterManager::LoadPageExtender(
    int32_t pageId, const RouterPageInfo& target, bool needHideLast, bool needTransition, bool isPush)
{
    ACE_SCOPED_TRACE_COMMERCIAL("load page: %s(id:%d)", target.url.c_str(), pageId);
    CHECK_RUN_ON(JS);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    auto stageManager = AceType::DynamicCast<ParallelStageManager>(context->GetStageManager());
    CHECK_NULL_RETURN(stageManager, false);
    LoadPlaceholderOrRelatedPageIfNeeded(context, stageManager);

    bool needClearSecondaryPage = CheckSecondaryPageNeedClear(isPush);
    if (!CheckStackSize(target, needClearSecondaryPage)) {
        return;
    }
    NotifyForceFullScreenChangeIfNeeded(target.url, context);
    auto pageNode = CreatePageExtender(pageId, target);
    if (!pageNode) {
        TAG_LOGE(AceLogTag::ACE_ROUTER, "failed to create page: %{public}s", target.url.c_str());
        return;
    }

    RefPtr<FrameNode> preLastPage = nullptr;
    if (!pageRouterStack_.empty()) {
        preLastPage = pageRouterStack_.rbegin()->Upgrade();
    }
    std::list<WeakPtr<FrameNode>> prevPageRouterStack;
    std::copy(pageRouterStack_.begin(), pageRouterStack_.end(), std::back_inserter(prevPageRouterStack));
    auto removePageSize = stageManager->UpdateSecondaryPageNeedRemoved(needClearSecondaryPage);
    while (removePageSize > 0 && !pageRouterStack_.empty()) {
        pageRouterStack_.pop_back();
        removePageSize--;
    };
    pageRouterStack_.emplace_back(pageNode);
    if (intentInfo_.has_value()) {
        if (!OnPageReadyAndHandleIntent(pageNode, needHideLast)) {
            intentInfo_.reset();
            std::swap(prevPageRouterStack, pageRouterStack_);
            TAG_LOGW(AceLogTag::ACE_ROUTER, "OnPageReadyAndHandleIntent Failed");
            return;
        }
    } else if (!OnPageReady(pageNode, needHideLast, needTransition)) {
        std::swap(prevPageRouterStack, pageRouterStack_);
        return;
    }
    stageManager->RemoveSecondaryPagesOfPrimaryPage();

    if (DetectHomePage(target, preLastPage)) {
        TAG_LOGI(AceLogTag::ACE_ROUTER, "url: %{public}s was recognised as home page", target.url.c_str());
        stageManager->OnHomePageDetected(pageNode, pageRouterStack_);
    }
    // page-level split: the first split page acts as home page
    TryEstablishPageLevelSplitHome(target.url, pageNode, context);

    pageNode->OnAccessibilityEvent(AccessibilityEventType::CHANGE);

    if (!SetOnKeyEvent(pageNode)) {
        TAG_LOGE(AceLogTag::ACE_ROUTER, "Fail to init the keyEvent of page");
        return;
    }
    TAG_LOGI(AceLogTag::ACE_ROUTER, "LoadPage Success");
}

void ParallelPageRouterManager::LoadPlaceholderOrRelatedPageIfNeeded(
    const RefPtr<PipelineContext>& context, const RefPtr<ParallelStageManager>& manager)
{
    if (hasTryLoadPlaceholderOrRelatedPage_) {
        return;
    }
    hasTryLoadPlaceholderOrRelatedPage_ = true;
    CHECK_NULL_VOID(context);
    auto forceSplitMgr = context->GetForceSplitManager();
    CHECK_NULL_VOID(forceSplitMgr);
    if (!forceSplitMgr->IsForceSplitSupported(true)) {
        return;
    }
    // page-level split: no placeholder, keep the right column empty
    if (forceSplitMgr->IsPageLevelSplitEnabled()) {
        return;
    }
    do {
        if (!forceSplitMgr->HasRelatedPage()) {
            break;
        }
        auto relatedPage = forceSplitMgr->GetRelatedPageName();
        if (LoadRelatedPage(relatedPage, manager)) {
            return;
        }
        TAG_LOGW(AceLogTag::ACE_ROUTER, "will create placeholder page when failed to create relatedPage.");
    } while (false);
    if (!LoadPlaceHolderPage(manager)) {
        TAG_LOGE(AceLogTag::ACE_ROUTER, "failed to load placeholder page");
    }
}

int32_t ParallelPageRouterManager::GetLastPageIndex()
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, -1);
    auto stageManager = pipelineContext->GetStageManager();
    CHECK_NULL_RETURN(stageManager, -1);
    auto stageNode = stageManager->GetStageNode();
    CHECK_NULL_RETURN(stageNode, -1);
    auto stagePattern = stageNode->GetPattern<ParallelStagePattern>();
    CHECK_NULL_RETURN(stagePattern, -1);
    if (stagePattern->GetIsSplit()) {
        return pageRouterStack_.size() - 1 + stagePattern->GetNonPageChildrenSize();
    }
    return pageRouterStack_.size() - 1;
}

bool ParallelPageRouterManager::DetectHomePage(const RouterPageInfo& target, const RefPtr<FrameNode>& preLastPage)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    auto forceSplitMgr = context->GetForceSplitManager();
    CHECK_NULL_RETURN(forceSplitMgr, false);

    // Page-level split established its runtime anchor from page stack.
    if (forceSplitMgr->IsPageLevelSplitEnabled()) {
        TAG_LOGI(AceLogTag::ACE_ROUTER, "Skip home page detection in page-level split");
        return false;
    }

    if (!forceSplitMgr->IsForceSplitSupported(true)) {
        TAG_LOGI(AceLogTag::ACE_ROUTER, "No need for home page recognition");
        return false;
    }

    if (!ShouldDetectHomePage(context, preLastPage)) {
        return false;
    }

    return JudgeHomePage(target);
}

bool ParallelPageRouterManager::JudgeHomePage(const RouterPageInfo& target)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    auto forceSplitMgr = context->GetForceSplitManager();
    CHECK_NULL_RETURN(forceSplitMgr, false);
    std::string homePageConfig = forceSplitMgr->GetHomePageName();
    std::string pageInfo = target.url;
    if (!homePageConfig.empty()) {
        if (strcmp(homePageConfig.c_str(), pageInfo.c_str()) == 0) {
            TAG_LOGI(AceLogTag::ACE_ROUTER,
                "current page: %{public}s is consistent with home page config", pageInfo.c_str());
            return true;
        }
        TAG_LOGI(AceLogTag::ACE_ROUTER, "current page is NOT consistent with home page config `%{public}s`",
            homePageConfig.c_str());
        return false;
    }

    int32_t elementNum = ElementRegister::GetInstance()->GetLatestElementId();
    TAG_LOGI(AceLogTag::ACE_ROUTER,
        "Judge home page, url: %{public}s, lastest elementId: %{public}d", target.url.c_str(), elementNum);
    if (elementNum >= HOME_PAGE_NODE_THRESHOLD) {
        return true;
    }

    std::transform(pageInfo.begin(), pageInfo.end(), pageInfo.begin(), ::tolower);
    std::istringstream stream(pageInfo);
    std::string temp;
    std::string realPage;
    while (getline(stream, temp, '/')) {
        realPage = temp;
    }
    for (auto it = HOME_PAGE_PREFIX.begin(); it != HOME_PAGE_PREFIX.end(); it++) {
        std::string prefix = *it;
        if (realPage.find(prefix) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool ParallelPageRouterManager::LoadPlaceHolderPage(const RefPtr<ParallelStageManager>& manager)
{
    CHECK_NULL_RETURN(manager, false);
    auto pageId = GenerateNextPageId();
    // create pageNode
    ACE_SCOPED_TRACE_COMMERCIAL("load placeholder(id:%d)", pageId);
    CHECK_RUN_ON(JS);
    auto entryPageInfo = AceType::MakeRefPtr<EntryPageInfo>(pageId, PLACEHOLDER_PAGE_URL, PLACEHOLDER_PAGE_PATH, "");
    auto pagePattern = ViewAdvancedRegister::GetInstance()->CreatePagePattern(entryPageInfo);
    auto placeHolderPageNode =
        FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), pagePattern);
    CHECK_NULL_RETURN(placeHolderPageNode, false);
    auto placeHolderPattern = placeHolderPageNode->GetPattern<ParallelPagePattern>();
    CHECK_NULL_RETURN(placeHolderPattern, false);
    placeHolderPattern->SetPageType(RouterPageType::PLACEHOLDER_PAGE);
    auto context = placeHolderPageNode->GetContextRefPtr();
    CHECK_NULL_RETURN(context, false);
    auto contentNode = ForceSplitUtils::CreatePlaceHolderContent(context);
    CHECK_NULL_RETURN(contentNode, false);
    contentNode->MountToParent(placeHolderPageNode);

    if (!SetOnKeyEvent(placeHolderPageNode)) {
        TAG_LOGE(AceLogTag::ACE_ROUTER, "fail to init key event handler of SystemPlaceHolder");
        return false;
    }
    auto stageNode = manager->GetStageNode();
    CHECK_NULL_RETURN(stageNode, false);
    auto stagePattern = stageNode->GetPattern<ParallelStagePattern>();
    CHECK_NULL_RETURN(stagePattern, false);
    stagePattern->SetPlaceholderPage(placeHolderPageNode);
    TAG_LOGI(AceLogTag::ACE_ROUTER, "success to load SystemPlaceHolder");
    return true;
}

bool ParallelPageRouterManager::LoadRelatedPage(const std::string& url, const RefPtr<ParallelStageManager>& manager)
{
    CHECK_NULL_RETURN(manager, false);
    if (url.empty()) {
        return false;
    }
    RouterPageInfo target;
    target.url = url;
    target.params = "";
    target.recoverable = false;
    target.errorCallback = [url](const std::string& errorMsg, int32_t errorCode) {
        if (errorCode == ERROR_CODE_NO_ERROR) {
            return;
        }
        TAG_LOGW(AceLogTag::ACE_ROUTER, "failed to loadRelatedPage with url:%{public}s, code:%{public}d, "
            "msg:%{public}s", url.c_str(), errorCode, errorMsg.c_str());
    };
    if (!manifestParser_) {
        TAG_LOGW(AceLogTag::ACE_ROUTER, "empty manifestParser found in LoadRelatedPage with url: %{public}s",
            url.c_str());
        return false;
    }
    target.path = manifestParser_->GetRouter()->GetPagePath(target.url);
    if (target.path.empty()) {
        TAG_LOGW(AceLogTag::ACE_ROUTER, "empty path found in LoadRelatedPage with url: %{public}s", url.c_str());
        return false;
    }
    auto relatedPage = CreatePage(GenerateNextPageId(), target);
    if (!relatedPage) {
        TAG_LOGE(AceLogTag::ACE_ROUTER, "failed to create RelatedPage");
        return false;
    }
    auto pattern = relatedPage->GetPattern<ParallelPagePattern>();
    CHECK_NULL_RETURN(pattern, false);
    pattern->SetPageType(RouterPageType::RELATED_PAGE);
    if (!SetOnKeyEvent(relatedPage)) {
        TAG_LOGE(AceLogTag::ACE_ROUTER, "fail to init key event handler of RelatedPage");
        return false;
    }
    auto stageNode = manager->GetStageNode();
    CHECK_NULL_RETURN(stageNode, false);
    auto stagePattern = stageNode->GetPattern<ParallelStagePattern>();
    CHECK_NULL_RETURN(stagePattern, false);
    stagePattern->SetRelatedPage(relatedPage);
    TAG_LOGI(AceLogTag::ACE_ROUTER, "success to load RelatedPage");
    return true;
}

RefPtr<ResourceAdapter> ParallelPageRouterManager::CreateResourceAdapter()
{
    RefPtr<ResourceAdapter> resourceAdapter = nullptr;
    resourceAdapter = ResourceManager::GetInstance().GetResourceAdapter(Container::CurrentIdSafely());
    if (!resourceAdapter) {
        return nullptr;
    }
    return resourceAdapter;
}

bool ParallelPageRouterManager::ShouldDetectHomePage(
    const RefPtr<PipelineContext>& context, const RefPtr<FrameNode>& preLastPage)
{
    CHECK_NULL_RETURN(context, false);
    auto stageManager = context->GetStageManager();
    CHECK_NULL_RETURN(stageManager, false);
    auto stageNode = stageManager->GetStageNode();
    CHECK_NULL_RETURN(stageNode, false);
    auto stagePattern = stageNode->GetPattern<ParallelStagePattern>();
    CHECK_NULL_RETURN(stagePattern, false);
    auto homePage = stagePattern->GetHomePage();
    if (!homePage) {
        return true;
    }

    if (!isNewPageReplacing_) {
        return false;
    }

    CHECK_NULL_RETURN(preLastPage, false);
    auto preLastPattern = preLastPage->GetPattern<ParallelPagePattern>();
    CHECK_NULL_RETURN(preLastPattern, false);
    if (preLastPattern->GetPageType() == RouterPageType::HOME_PAGE) {
        // we need to detect new home page when replace top home page.
        return true;
    }

    return false;
}

void ParallelPageRouterManager::RefreshAllPageIndex()
{
    int32_t startIndex = 1;
    for (auto it = pageRouterStack_.begin(); it != pageRouterStack_.end(); ++it, ++startIndex) {
        auto pageNode = it->Upgrade();
        if (!pageNode) {
            continue;
        }
        auto pagePattern = pageNode->GetPattern<PagePattern>();
        if (pagePattern) {
            pagePattern->GetPageInfo()->SetPageIndex(startIndex);
        }
    }
}

bool ParallelPageRouterManager::SetOnKeyEvent(const RefPtr<FrameNode> &pageNode)
{
    auto focusHub = pageNode->GetFocusHub();
    CHECK_NULL_RETURN(focusHub, false);

    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, false);
    auto stageManager = AceType::DynamicCast<ParallelStageManager>(pipelineContext->GetStageManager());
    CHECK_NULL_RETURN(stageManager, false);
    WeakPtr<ParallelStageManager> weakManager = stageManager;

    bool initFlag = true;
    auto onKeyEventCallback = [weakManager, initFlag](const KeyEvent& event) mutable -> bool {
        if (event.code != KeyCode::KEY_TAB || event.action != KeyAction::DOWN) {
            return false;
        }
        auto stageManager = weakManager.Upgrade();
        CHECK_NULL_RETURN(stageManager, false);
        return stageManager->ExchangePageFocus(initFlag);
    };

    focusHub->SetOnKeyEventInternal(std::move(onKeyEventCallback));
    return true;
}

bool ParallelPageRouterManager::CheckSecondaryPageNeedClear(bool isPush)
{
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, false);
    auto stageManager = AceType::DynamicCast<ParallelStageManager>(pipelineContext->GetStageManager());
    CHECK_NULL_RETURN(stageManager, false);
    if (pageRouterStack_.empty() || !isPush || !stageManager->IsSplitMode()) {
        stageManager->SetNeedClearSecondaryPage(false);
        return false;
    }
    auto preTopPage = pageRouterStack_.back().Upgrade();
    if (preTopPage) {
        auto preTopPagePattern = preTopPage->GetPattern<ParallelPagePattern>();
        CHECK_NULL_RETURN(preTopPagePattern, false);
        auto forceSplitMgr = pipelineContext->GetForceSplitManager();
        auto needClearSecondaryPage = false;
        if (forceSplitMgr && forceSplitMgr->CanPushPageToPrimary()) {
            needClearSecondaryPage = RouterPageType::DETAIL_PAGE == preTopPagePattern->GetPageType() &&
                stageManager->GetTouchedPrimaryColumnPage() != nullptr;
        } else {
            auto homeTouch = stageManager->GetHomePageTouched();
            needClearSecondaryPage = RouterPageType::DETAIL_PAGE == preTopPagePattern->GetPageType() &&
                homeTouch.has_value() && homeTouch.value();
        }
        stageManager->SetNeedClearSecondaryPage(needClearSecondaryPage);
        return needClearSecondaryPage;
    }
    stageManager->SetNeedClearSecondaryPage(false);
    return false;
}

bool ParallelPageRouterManager::CheckStackSize(const RouterPageInfo& target, bool needClearSecondaryPage)
{
    // Dont't block push operation when trigger secondaryPage clear
    if (needClearSecondaryPage) {
        return true;
    }
    if (GetStackSize() >= MAX_ROUTER_STACK_SIZE) {
        TAG_LOGW(AceLogTag::ACE_ROUTER, "Router stack size is larger than max size 32.");
        if (target.errorCallback != nullptr) {
            target.errorCallback("The pages are pushed too much.", ERROR_CODE_PAGE_STACK_FULL);
        }
        return false;
    }
    return true;
}

bool ParallelPageRouterManager::StartPop()
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    auto forceSplitMgr = context->GetForceSplitManager();
    CHECK_NULL_RETURN(forceSplitMgr, false);
    if (forceSplitMgr->IsForceSplitDragging()) {
        TAG_LOGI(AceLogTag::ACE_NAVIGATION, "can't pop during dragging");
        return true;
    }

    if (pageRouterStack_.size() > 1) {
        auto penultimatePage = pageRouterStack_.rbegin();
        std::advance(penultimatePage, 1);
        auto penultimatePageNode = penultimatePage->Upgrade();
        if (penultimatePageNode) {
            auto pattern = penultimatePageNode->GetPattern<PagePattern>();
            if (pattern) {
                auto pageContext = penultimatePageNode->GetContextRefPtr();
                NotifyForceFullScreenChangeIfNeeded(pattern->GetPageUrl(), pageContext);
                // page-level split: re-establish home when popping back into a split page
                TryEstablishPageLevelSplitHome(pattern->GetPageUrl(), penultimatePageNode, pageContext);
            }
        }
    }
    return PageRouterManager::StartPop();
}

} // namespace OHOS::Ace::NG
