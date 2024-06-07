/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "frameworks/bridge/declarative_frontend/ng/page_router_manager.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <string>

#include "base/i18n/localization.h"
#include "base/memory/referenced.h"
#include "base/ressched/ressched_report.h"
#include "base/utils/utils.h"
#include "base/perfmonitor/perf_monitor.h"
#include "bridge/common/utils/source_map.h"
#include "bridge/common/utils/utils.h"
#include "bridge/declarative_frontend/ng/entry_page_info.h"
#include "bridge/js_frontend/frontend_delegate.h"
#include "bridge/js_frontend/engine/jsi/ark_js_runtime.h"
#include "core/common/container.h"
#include "core/common/recorder/node_data_cache.h"
#include "core/common/thread_checker.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/stage/page_pattern.h"
#include "core/components_ng/pattern/stage/stage_manager.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/bridge/declarative_frontend/engine/jsi/jsi_declarative_engine.h"

namespace OHOS::Ace::NG {

namespace {

constexpr int32_t BUNDLE_START_POS = 8;
constexpr int32_t INVALID_PAGE_INDEX = -1;
constexpr int32_t MAX_ROUTER_STACK_SIZE = 32;
constexpr int32_t JS_FILE_EXTENSION_LENGTH = 3;
constexpr char ETS_PATH[] = "/src/main/ets/";
constexpr char DEBUG_PATH[] = "entry/build/default/cache/default/default@CompileArkTS/esmodule/debug/";
constexpr char TS_SUFFIX[] = ".ts";
constexpr char ETS_SUFFIX[] = ".ets";

void ExitToDesktop()
{
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [] {
            auto pipeline = PipelineContext::GetCurrentContext();
            CHECK_NULL_VOID(pipeline);
            AccessibilityEvent event;
            event.type = AccessibilityEventType::PAGE_CHANGE;
            pipeline->SendEventToAccessibility(event);
            pipeline->Finish(false);
        },
        TaskExecutor::TaskType::UI, "ArkUIPageRouterExitToDesktop");
}

} // namespace

void PageRouterManager::LoadOhmUrl(const RouterPageInfo& target)
{
    RouterPageInfo info = target;
    info.path = info.url + ".js";
    RouterOptScope scope(this);
    LoadPage(GenerateNextPageId(), info);
}

void PageRouterManager::RunPage(const std::string& url, const std::string& params)
{
    PerfMonitor::GetPerfMonitor()->SetAppStartStatus();
    ACE_SCOPED_TRACE("PageRouterManager::RunPage");
    CHECK_RUN_ON(JS);
    RouterPageInfo info { url, params };
#if !defined(PREVIEW)
    if (info.url.substr(0, strlen(BUNDLE_TAG)) == BUNDLE_TAG) {
        auto container = Container::Current();
        CHECK_NULL_VOID(container);
        auto pageUrlChecker = container->GetPageUrlChecker();
        CHECK_NULL_VOID(pageUrlChecker);
        auto instanceId = container->GetInstanceId();
        auto taskExecutor = container->GetTaskExecutor();
        CHECK_NULL_VOID(taskExecutor);
        info.errorCallback = [](const std::string& errorMsg, int32_t errorCode) {};
        auto callback = [weak = AceType::WeakClaim(this), info, taskExecutor, instanceId]() {
            ContainerScope scope(instanceId);
            auto pageRouterManager = weak.Upgrade();
            CHECK_NULL_VOID(pageRouterManager);
            taskExecutor->PostTask(
                [pageRouterManager, info]() { pageRouterManager->LoadOhmUrl(info); },
                TaskExecutor::TaskType::JS, "ArkUIPageRouterLoadOhmUrl");
        };

        auto silentInstallErrorCallBack = [taskExecutor, instanceId](int32_t errorCode, const std::string& errorMsg) {
            ContainerScope scope(instanceId);
            taskExecutor->PostTask(
                [errorCode, errorMsg]() {
                    LOGW("Run page error = %{public}d, errorMsg = %{public}s", errorCode, errorMsg.c_str());
                },
                TaskExecutor::TaskType::JS, "ArkUIPageRouterErrorLog");
        };

        pageUrlChecker->LoadPageUrl(url, callback, silentInstallErrorCallBack);
        return;
    }
#endif
    if (!info.url.empty()) {
        info.path = manifestParser_->GetRouter()->GetPagePath(url);
        if (info.path.empty()) {
            return;
        }
    } else {
        info.path = manifestParser_->GetRouter()->GetEntry();
        info.url = manifestParser_->GetRouter()->GetEntry("");
    }
    RouterOptScope scope(this);
    LoadPage(GenerateNextPageId(), info);
}

void PageRouterManager::RunPage(const std::shared_ptr<std::vector<uint8_t>>& content, const std::string& params)
{
    ACE_SCOPED_TRACE("PageRouterManager::RunPage");
    CHECK_RUN_ON(JS);
    RouterPageInfo info;
    info.content = content;

#if !defined(PREVIEW)
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto instanceId = container->GetInstanceId();
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    ContainerScope scope(instanceId);
    auto pageRouterManager = AceType::Claim(this);
    CHECK_NULL_VOID(pageRouterManager);
    taskExecutor->PostTask(
        [pageRouterManager, info]() { pageRouterManager->LoadOhmUrl(info); },
        TaskExecutor::TaskType::JS, "ArkUIPageRouterLoadOhmUrl");
#endif
}

void PageRouterManager::RunPageByNamedRouter(const std::string& name, const std::string& params)
{
    RouterPageInfo info { name, params };
    info.isNamedRouterMode = true;
    RouterOptScope scope(this);
    LoadPage(GenerateNextPageId(), info);
}

UIContentErrorCode PageRouterManager::RunCard(
    const std::string& url, const std::string& params, int64_t cardId, const std::string& entryPoint)
{
    CHECK_RUN_ON(JS);
    RouterPageInfo info { url };
#ifndef PREVIEW
    if (!info.url.empty()) {
        info.path = manifestParser_->GetRouter()->GetPagePath(url);
    } else {
        info.path = manifestParser_->GetRouter()->GetEntry();
        info.url = manifestParser_->GetRouter()->GetEntry("");
    }
#endif
    return LoadCard(0, info, params, cardId, false, true, entryPoint);
}

void PageRouterManager::Push(const RouterPageInfo& target)
{
    CHECK_RUN_ON(JS);
    if (inRouterOpt_) {
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        context->PostAsyncEvent(
            [weak = WeakClaim(this), target]() {
                auto router = weak.Upgrade();
                CHECK_NULL_VOID(router);
                router->Push(target);
            },
            "ArkUIPageRouterPush", TaskExecutor::TaskType::JS);
        return;
    }
    RouterOptScope scope(this);
    StartPush(target);
}

void PageRouterManager::PushNamedRoute(const RouterPageInfo& target)
{
    CHECK_RUN_ON(JS);
    if (inRouterOpt_) {
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        context->PostAsyncEvent(
            [weak = WeakClaim(this), target]() {
                auto router = weak.Upgrade();
                CHECK_NULL_VOID(router);
                router->PushNamedRoute(target);
            },
            "ArkUIPageRouterPushNamedRoute", TaskExecutor::TaskType::JS);
        return;
    }
    RouterOptScope scope(this);
    if (GetStackSize() >= MAX_ROUTER_STACK_SIZE) {
        LOGW("router stack size is larger than max size 32.");
        if (target.errorCallback != nullptr) {
            target.errorCallback("The pages are pushed too much.", ERROR_CODE_PAGE_STACK_FULL);
        }
        return;
    }
    CleanPageOverlay();
    if (target.routerMode == RouterMode::SINGLE) {
        auto PageInfoByUrl = FindPageInStack(target.url);
        auto pagePath = Framework::JsiDeclarativeEngine::GetPagePath(target.url);
        if (PageInfoByUrl.second) {
            // get pageInfo by url, find page in stack, move postion and update params.
            MovePageToFront(PageInfoByUrl.first, PageInfoByUrl.second, target, true);
            return;
        }
        auto PageInfoByPagePath = FindPageInStack(pagePath);
        if (PageInfoByPagePath.second) {
            // get pageInfo by pagePath, find page in stack, move postion and update params.
            MovePageToFront(PageInfoByPagePath.first, PageInfoByPagePath.second, target, true);
            return;
        }
    }
    RouterPageInfo info = target;
    info.isNamedRouterMode = true;
    LoadPage(GenerateNextPageId(), info);
}

void PageRouterManager::Replace(const RouterPageInfo& target)
{
    CHECK_RUN_ON(JS);
    if (inRouterOpt_) {
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        context->PostAsyncEvent(
            [weak = WeakClaim(this), target]() {
                auto router = weak.Upgrade();
                CHECK_NULL_VOID(router);
                router->Replace(target);
            },
            "ArkUIPageRouterReplace", TaskExecutor::TaskType::JS);
        return;
    }
    RouterOptScope scope(this);
    StartReplace(target);
}

void PageRouterManager::ReplaceNamedRoute(const RouterPageInfo& target)
{
    CHECK_RUN_ON(JS);
    if (inRouterOpt_) {
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        context->PostAsyncEvent(
            [weak = WeakClaim(this), target]() {
                auto router = weak.Upgrade();
                CHECK_NULL_VOID(router);
                router->ReplaceNamedRoute(target);
            },
            "ArkUIPageRouterReplaceNamedRoute", TaskExecutor::TaskType::JS);
        return;
    }
    RouterOptScope scope(this);
    CleanPageOverlay();
    RouterPageInfo info = target;
    info.isNamedRouterMode = true;
    DealReplacePage(info);
}

void PageRouterManager::BackWithTarget(const RouterPageInfo& target)
{
    CHECK_RUN_ON(JS);
    LOGI("Router back path = %{private}s", target.url.c_str());
    if (inRouterOpt_) {
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        context->PostAsyncEvent(
            [weak = WeakClaim(this), target]() {
                auto router = weak.Upgrade();
                CHECK_NULL_VOID(router);
                router->BackWithTarget(target);
            },
            "ArkUIPageRouterBackWithTarget", TaskExecutor::TaskType::JS);
        return;
    }
    RouterOptScope scope(this);
    BackCheckAlert(target);
}

void PageRouterManager::BackToIndexWithTarget(int32_t index, const std::string& params)
{
    CHECK_RUN_ON(JS);
    if (!CheckIndexValid(index)) {
        return;
    }
    if (inRouterOpt_) {
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        context->PostAsyncEvent(
            [weak = WeakClaim(this), index, params]() {
                auto router = weak.Upgrade();
                CHECK_NULL_VOID(router);
                router->BackToIndexWithTarget(index, params);
            },
            "ArkUIPageRouterBackToIndex", TaskExecutor::TaskType::JS);
        return;
    }
    RouterOptScope scope(this);
    BackToIndexCheckAlert(index, params);
}

void PageRouterManager::Clear()
{
    CHECK_RUN_ON(JS);
    if (inRouterOpt_) {
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        context->PostAsyncEvent(
            [weak = WeakClaim(this)]() {
                auto router = weak.Upgrade();
                CHECK_NULL_VOID(router);
                router->Clear();
            },
            "ArkUIPageRouterClear", TaskExecutor::TaskType::JS);
        return;
    }
    RouterOptScope scope(this);
    StartClean();
}

void PageRouterManager::EnableAlertBeforeBackPage(const std::string& message, std::function<void(int32_t)>&& callback)
{
    auto currentPage = pageRouterStack_.back().Upgrade();
    CHECK_NULL_VOID(currentPage);
    auto pagePattern = currentPage->GetPattern<PagePattern>();
    CHECK_NULL_VOID(pagePattern);
    auto pageInfo = pagePattern->GetPageInfo();
    CHECK_NULL_VOID(pageInfo);

    DialogProperties dialogProperties = {
        .content = message,
        .autoCancel = false,
        .buttons = { { .text = Localization::GetInstance()->GetEntryLetters("common.cancel"), .textColor = "" },
            { .text = Localization::GetInstance()->GetEntryLetters("common.ok"), .textColor = "" } },
        .onSuccess =
            [weak = AceType::WeakClaim(this), weakPageInfo = AceType::WeakClaim(AceType::RawPtr(pageInfo))](
                int32_t successType, int32_t successIndex) {
                auto pageInfo = weakPageInfo.Upgrade();
                if (pageInfo && pageInfo->GetAlertCallback() && !successType) {
                    pageInfo->GetAlertCallback()(successIndex);
                    if (successIndex) {
                        auto router = weak.Upgrade();
                        CHECK_NULL_VOID(router);
                        router->StartBack(router->ngBackTarget_);
                    }
                }
            },
    };

    pageInfo->SetDialogProperties(dialogProperties);
    pageInfo->SetAlertCallback(std::move(callback));
}

void PageRouterManager::DisableAlertBeforeBackPage()
{
    auto currentPage = pageRouterStack_.back().Upgrade();
    CHECK_NULL_VOID(currentPage);
    auto pagePattern = currentPage->GetPattern<PagePattern>();
    CHECK_NULL_VOID(pagePattern);
    auto pageInfo = DynamicCast<EntryPageInfo>(pagePattern->GetPageInfo());
    CHECK_NULL_VOID(pageInfo);
    pageInfo->SetAlertCallback(nullptr);
}

void PageRouterManager::StartClean()
{
    if (pageRouterStack_.size() <= 1) {
        return;
    }
    std::list<WeakPtr<FrameNode>> temp;
    std::swap(temp, pageRouterStack_);
    pageRouterStack_.emplace_back(temp.back());
    if (!OnCleanPageStack()) {
        std::swap(temp, pageRouterStack_);
    }
}

bool PageRouterManager::Pop()
{
    CHECK_RUN_ON(JS);
    if (inRouterOpt_) {
        return false;
    }
    RouterOptScope scope(this);
    return StartPop();
}

bool PageRouterManager::StartPop()
{
    CHECK_RUN_ON(JS);
    auto currentPage = pageRouterStack_.empty() ? nullptr : pageRouterStack_.back().Upgrade();
    CHECK_NULL_RETURN(currentPage, false);
    auto pagePattern = currentPage->GetPattern<PagePattern>();
    CHECK_NULL_RETURN(pagePattern, false);
    auto pageInfo = DynamicCast<EntryPageInfo>(pagePattern->GetPageInfo());
    CHECK_NULL_RETURN(pageInfo, false);
    if (pageInfo->GetAlertCallback()) {
        BackCheckAlert(RouterPageInfo());
        return true;
    }

    if (pageRouterStack_.size() <= 1) {
        if (!restorePageStack_.empty()) {
            StartRestore(RouterPageInfo());
            return true;
        }
        // the last page.
        return false;
    }

    // pop top page in page stack
    auto preWeakNode = pageRouterStack_.back();
    pageRouterStack_.pop_back();

    // clean prev top page params
    currentPage = pageRouterStack_.back().Upgrade();
    CHECK_NULL_RETURN(currentPage, false);
    pagePattern = currentPage->GetPattern<PagePattern>();
    CHECK_NULL_RETURN(pagePattern, false);
    pageInfo = DynamicCast<EntryPageInfo>(pagePattern->GetPageInfo());
    CHECK_NULL_RETURN(pageInfo, false);
    std::string params = pageInfo->GetPageParams();
    pageInfo->ReplacePageParams("");

    // do pop page
    if (!OnPopPage(true, true)) {
        pageRouterStack_.emplace_back(preWeakNode);
        pageInfo->ReplacePageParams(params);
        return false;
    }
    return true;
}

void PageRouterManager::StartRestore(const RouterPageInfo& target)
{
    RouterPageInfo info = target;
    auto tempStack = restorePageStack_;
    if (target.url.empty()) {
        info.url = tempStack.back();
        tempStack.pop_back();
    } else {
        info.url = target.url;
        while (!tempStack.empty() && tempStack.back() != info.url) {
            tempStack.pop_back();
        }
        if (tempStack.empty()) {
            return;
        }
        tempStack.pop_back();
    }

    restorePageStack_ = tempStack;
    StartClean();
    PopPage("", false, false);
    StartPush(info);
}

int32_t PageRouterManager::GetStackSize() const
{
    CHECK_RUN_ON(JS);
    return static_cast<int32_t>(pageRouterStack_.size());
}

RouterPageInfo PageRouterManager::GetPageInfoByIndex(int32_t index, const std::string& params)
{
    RouterPageInfo emptyForReturn;
    if (!CheckIndexValid(index) &&
        index != static_cast<int32_t>(pageRouterStack_.size() + 1) /* in case the page is on popping */) {
        return emptyForReturn;
    }
    std::string url;
    int32_t counter = 1;
    for (const auto& iter : pageRouterStack_) {
        if (counter == index) {
            auto pageNode = iter.Upgrade();
            CHECK_NULL_RETURN(pageNode, emptyForReturn);
            auto pagePattern = pageNode->GetPattern<NG::PagePattern>();
            CHECK_NULL_RETURN(pagePattern, emptyForReturn);
            auto pageInfo = pagePattern->GetPageInfo();
            CHECK_NULL_RETURN(pageInfo, emptyForReturn);
            return NG::RouterPageInfo({ pageInfo->GetPageUrl(), params });
        }
        counter++;
    }

    // in case the page is on popping
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipelineContext, emptyForReturn);
    auto stageManager = pipelineContext->GetStageManager();
    CHECK_NULL_RETURN(stageManager, emptyForReturn);
    auto popPage = stageManager->GetLastPage();
    CHECK_NULL_RETURN(popPage, emptyForReturn);
    auto pagePattern = popPage->GetPattern<NG::PagePattern>();
    CHECK_NULL_RETURN(pagePattern, emptyForReturn);
    auto pageInfo = pagePattern->GetPageInfo();
    // make sure the last page is the one with 'index'
    if (pageInfo && pageInfo->GetPageIndex() == index) {
        return NG::RouterPageInfo({ pageInfo->GetPageUrl(), params });
    }
    return emptyForReturn;
}

void PageRouterManager::GetState(int32_t& index, std::string& name, std::string& path)
{
    CHECK_RUN_ON(JS);
    if (pageRouterStack_.empty()) {
        return;
    }
    index = static_cast<int32_t>(pageRouterStack_.size());
    auto pageNode = pageRouterStack_.back().Upgrade();
    CHECK_NULL_VOID(pageNode);
    auto pagePattern = pageNode->GetPattern<NG::PagePattern>();
    CHECK_NULL_VOID(pagePattern);
    auto pageInfo = pagePattern->GetPageInfo();
    CHECK_NULL_VOID(pageInfo);
    auto url = pageInfo->GetPageUrl();
    auto pagePath = Framework::JsiDeclarativeEngine::GetPagePath(url);
    if (!pagePath.empty()) {
        url = pagePath;
    }
    auto pos = url.rfind(".js");
    if (pos == url.length() - JS_FILE_EXTENSION_LENGTH) {
        url = url.substr(0, pos);
    }
    pos = url.rfind("/");
    if (pos != std::string::npos) {
        name = url.substr(pos + 1);
        path = url.substr(0, pos + 1);
    }
    if (name.size() == 0) {
        name = "index";
    }
    if (path.size() == 0) {
        path = "/" + url;
    }
}

void PageRouterManager::GetStateByIndex(int32_t index, std::string& name, std::string& path, std::string& params)
{
    CHECK_RUN_ON(JS);
    if (!CheckIndexValid(index) &&
        index != static_cast<int32_t>(pageRouterStack_.size() + 1) /* in case the page is on popping */) {
        return;
    }

    int32_t counter = 1;
    for (const auto& iter : pageRouterStack_) {
        if (counter == index) {
            auto pageNode = iter.Upgrade();
            CHECK_NULL_VOID(pageNode);
            GetPageNameAndPath(pageNode, name, path);
            auto pagePattern = pageNode->GetPattern<NG::PagePattern>();
            CHECK_NULL_VOID(pagePattern);
            auto pageInfo = DynamicCast<NG::EntryPageInfo>(pagePattern->GetPageInfo());
            CHECK_NULL_VOID(pageInfo);
            params = pageInfo->GetPageParams();
            return;
        }
        counter++;
    }

    // in case the page is on popping
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto stageManager = pipelineContext->GetStageManager();
    CHECK_NULL_VOID(stageManager);
    auto popPage = stageManager->GetLastPage();
    CHECK_NULL_VOID(popPage);
    auto pagePattern = popPage->GetPattern<NG::PagePattern>();
    CHECK_NULL_VOID(pagePattern);
    auto pageInfo = pagePattern->GetPageInfo();
    // make sure the last page is the one with 'index'
    if (pageInfo && pageInfo->GetPageIndex() == index) {
        GetPageNameAndPath(popPage, name, path);
        auto entryPageInfo = DynamicCast<NG::EntryPageInfo>(pageInfo);
        CHECK_NULL_VOID(entryPageInfo);
        params = entryPageInfo->GetPageParams();
    }
}

void PageRouterManager::GetStateByUrl(std::string& url, std::vector<Framework::StateInfo>& stateArray)
{
    CHECK_RUN_ON(JS);
    int32_t counter = 1;
    Framework::StateInfo stateInfo;
    for (auto& iter : pageRouterStack_) {
        auto pageNode = iter.Upgrade();
        CHECK_NULL_VOID(pageNode);
        auto pagePattern = pageNode->GetPattern<PagePattern>();
        CHECK_NULL_VOID(pagePattern);
        auto pageInfo = DynamicCast<EntryPageInfo>(pagePattern->GetPageInfo());
        CHECK_NULL_VOID(pageInfo);
        std::string tempUrl;
        if (pageInfo->GetPageUrl() == url) {
            stateInfo.params = pageInfo->GetPageParams();
            stateInfo.index = counter;
            auto pos = url.rfind(".js");
            if (pos == url.length() - JS_FILE_EXTENSION_LENGTH) {
                tempUrl = url.substr(0, pos);
            }
            tempUrl = url;
            auto pagePath = Framework::JsiDeclarativeEngine::GetPagePath(url);
            if (!pagePath.empty()) {
                tempUrl = pagePath;
            }
            pos = tempUrl.rfind("/");
            if (pos != std::string::npos) {
                stateInfo.name = tempUrl.substr(pos + 1);
                stateInfo.path = tempUrl.substr(0, pos + 1);
            }
            if (stateInfo.name.size() == 0) {
                stateInfo.name = "index";
            }
            if (stateInfo.path.size() == 0) {
                stateInfo.path = "/" + tempUrl;
            }
            stateArray.emplace_back(stateInfo);
        }
        counter++;
    }
}

void PageRouterManager::GetPageNameAndPath(const RefPtr<FrameNode>& pageNode, std::string& name, std::string& path)
{
    auto pagePattern = pageNode->GetPattern<NG::PagePattern>();
    CHECK_NULL_VOID(pagePattern);
    auto pageInfo = pagePattern->GetPageInfo();
    CHECK_NULL_VOID(pageInfo);
    auto url = pageInfo->GetPageUrl();
    auto pagePath = Framework::JsiDeclarativeEngine::GetPagePath(url);
    if (!pagePath.empty()) {
        url = pagePath;
    }
    auto pos = url.rfind(".js");
    if (pos == url.length() - JS_FILE_EXTENSION_LENGTH) {
        url = url.substr(0, pos);
    }
    pos = url.rfind("/");
    if (pos != std::string::npos) {
        name = url.substr(pos + 1);
        path = url.substr(0, pos + 1);
    }
    if (name.size() == 0) {
        name = "index";
    }
    if (path.size() == 0) {
        path = "/" + url;
    }
}

std::string PageRouterManager::GetParams() const
{
    CHECK_RUN_ON(JS);
    if (pageRouterStack_.empty()) {
        return "";
    }
    auto pageNode = pageRouterStack_.back().Upgrade();
    CHECK_NULL_RETURN(pageNode, "");
    auto pagePattern = pageNode->GetPattern<NG::PagePattern>();
    CHECK_NULL_RETURN(pagePattern, "");
    auto pageInfo = DynamicCast<EntryPageInfo>(pagePattern->GetPageInfo());
    CHECK_NULL_RETURN(pageInfo, "");
    return pageInfo->GetPageParams();
}

int32_t PageRouterManager::GetIndexByUrl(const std::string& url) const
{
    int32_t index = 0;
    for (auto iter : pageRouterStack_) {
        auto pageNode = iter.Upgrade();
        if (!pageNode) {
            continue;
        }
        auto pagePattern = pageNode->GetPattern<NG::PagePattern>();
        auto localUrl = pagePattern->GetPageInfo()->GetPageUrl();
        if (localUrl == url) {
            return index;
        }
        ++index;
    }
    return INVALID_PAGE_INDEX;
}

std::string PageRouterManager::GetCurrentPageUrl()
{
    CHECK_RUN_ON(JS);
    if (pageRouterStack_.empty()) {
        return "";
    }
    auto pageNode = pageRouterStack_.back().Upgrade();
    CHECK_NULL_RETURN(pageNode, "");
    auto pagePattern = pageNode->GetPattern<PagePattern>();
    CHECK_NULL_RETURN(pagePattern, "");
    auto entryPageInfo = DynamicCast<EntryPageInfo>(pagePattern->GetPageInfo());
    CHECK_NULL_RETURN(entryPageInfo, "");
    return entryPageInfo->GetPagePath();
}

// Get the currently running JS page information in NG structure.
RefPtr<Framework::RevSourceMap> PageRouterManager::GetCurrentPageSourceMap(const RefPtr<AssetManager>& assetManager)
{
    CHECK_RUN_ON(JS);
    if (pageRouterStack_.empty()) {
        return nullptr;
    }
    auto pageNode = pageRouterStack_.back().Upgrade();
    CHECK_NULL_RETURN(pageNode, nullptr);
    auto pagePattern = pageNode->GetPattern<PagePattern>();
    CHECK_NULL_RETURN(pagePattern, nullptr);
    auto entryPageInfo = DynamicCast<EntryPageInfo>(pagePattern->GetPageInfo());
    CHECK_NULL_RETURN(entryPageInfo, nullptr);
    auto pageMap = entryPageInfo->GetPageMap();
    if (pageMap) {
        return pageMap;
    }
    // initialize page map.
    std::string jsSourceMap;
    // stage mode
    auto container = Container::Current();
    CHECK_NULL_RETURN(container, nullptr);
    if (container->IsUseStageModel()) {
        auto pagePath = entryPageInfo->GetPagePath();
        auto moduleName = container->GetModuleName();
        std::string judgePath = "";
        if (Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_TWELVE)) {
            judgePath = DEBUG_PATH + moduleName + ETS_PATH +
                        pagePath.substr(0, pagePath.size() - JS_FILE_EXTENSION_LENGTH) + TS_SUFFIX;
        } else {
            judgePath = moduleName + ETS_PATH +
                        pagePath.substr(0, pagePath.size() - JS_FILE_EXTENSION_LENGTH) + ETS_SUFFIX;
        }
        if (Framework::GetAssetContentImpl(assetManager, "sourceMaps.map", jsSourceMap)) {
            auto jsonPages = JsonUtil::ParseJsonString(jsSourceMap);
            auto jsonPage = jsonPages->GetValue(judgePath)->ToString();
            auto stagePageMap = MakeRefPtr<Framework::RevSourceMap>();
            stagePageMap->Init(jsonPage);
            entryPageInfo->SetPageMap(stagePageMap);
            return stagePageMap;
        }
    } else {
        if (Framework::GetAssetContentImpl(assetManager, entryPageInfo->GetPagePath() + ".map", jsSourceMap)) {
            auto faPageMap = MakeRefPtr<Framework::RevSourceMap>();
            faPageMap->Init(jsSourceMap);
            entryPageInfo->SetPageMap(faPageMap);
            return faPageMap;
        }
    }
    return nullptr;
}

std::unique_ptr<JsonValue> PageRouterManager::GetStackInfo()
{
    auto jsonRouterStack = JsonUtil::CreateArray(true);
    auto restoreIter = restorePageStack_.begin();
    while (restoreIter != restorePageStack_.end()) {
        auto jsonItem = JsonUtil::Create(true);
        jsonItem->Put("url", restoreIter->c_str());
        jsonRouterStack->Put(jsonItem);
        ++restoreIter;
    }
    auto iter = pageRouterStack_.begin();
    while (iter != pageRouterStack_.end()) {
        auto pageNode = iter->Upgrade();
        CHECK_NULL_RETURN(pageNode, jsonRouterStack);
        auto pagePattern = pageNode->GetPattern<NG::PagePattern>();
        CHECK_NULL_RETURN(pagePattern, jsonRouterStack);
        auto pageInfo = pagePattern->GetPageInfo();
        CHECK_NULL_RETURN(pageInfo, jsonRouterStack);
        auto url = pageInfo->GetPageUrl();
        auto jsonItem = JsonUtil::Create(true);
        jsonItem->Put("url", url.c_str());
        jsonRouterStack->Put(jsonItem);
        ++iter;
    }
    return jsonRouterStack;
}

std::pair<std::string, UIContentErrorCode> PageRouterManager::RestoreRouterStack(std::unique_ptr<JsonValue> stackInfo)
{
    if (!stackInfo->IsValid() || !stackInfo->IsArray()) {
        return std::make_pair("", UIContentErrorCode::WRONG_PAGE_ROUTER);
    }
    int32_t stackSize = stackInfo->GetArraySize();
    if (stackSize < 1) {
        return std::make_pair("", UIContentErrorCode::WRONG_PAGE_ROUTER);
    }

    auto container = Container::Current();
    CHECK_NULL_RETURN(container, std::make_pair("", UIContentErrorCode::NULL_POINTER));
    auto pipeline = container->GetPipelineContext();
    CHECK_NULL_RETURN(pipeline, std::make_pair("", UIContentErrorCode::WRONG_PAGE_ROUTER));
    auto context = DynamicCast<NG::PipelineContext>(pipeline);
    auto stageManager = context ? context->GetStageManager() : nullptr;
    CHECK_NULL_RETURN(stageManager, std::make_pair("", UIContentErrorCode::WRONG_PAGE_ROUTER));

    for (int32_t index = 0; index < stackSize - 1; ++index) {
        std::string url = stackInfo->GetArrayItem(index)->GetValue("url")->ToString();
        // remove 2 useless character, as "XXX" to XXX
        url = url.substr(1, url.size() - 2);
        restorePageStack_.emplace_back(url);
    }
    std::string startUrl = stackInfo->GetArrayItem(stackSize - 1)->GetValue("url")->ToString();
    // remove 2 useless character, as "XXX" to XXX
    return std::make_pair(startUrl.substr(1, startUrl.size() - 2), UIContentErrorCode::NO_ERRORS);
}

int32_t PageRouterManager::GenerateNextPageId()
{
    return ++pageId_;
}

std::pair<int32_t, RefPtr<FrameNode>> PageRouterManager::FindPageInStack(const std::string& url, bool needIgnoreBegin)
{
    auto iter = std::find_if(needIgnoreBegin ? ++pageRouterStack_.rbegin() : pageRouterStack_.rbegin(),
        pageRouterStack_.rend(), [url](const WeakPtr<FrameNode>& item) {
            auto pageNode = item.Upgrade();
            CHECK_NULL_RETURN(pageNode, false);
            auto pagePattern = pageNode->GetPattern<PagePattern>();
            CHECK_NULL_RETURN(pagePattern, false);
            auto entryPageInfo = DynamicCast<EntryPageInfo>(pagePattern->GetPageInfo());
            CHECK_NULL_RETURN(entryPageInfo, false);
            return entryPageInfo->GetPageUrl() == url;
        });
    if (iter == pageRouterStack_.rend()) {
        return { -1, nullptr };
    }
    // Returns to the forward position.
    return { std::distance(iter, pageRouterStack_.rend()) - 1, iter->Upgrade() };
}

void PageRouterManager::PushOhmUrl(const RouterPageInfo& target)
{
    RouterOptScope scope(this);
    if (GetStackSize() >= MAX_ROUTER_STACK_SIZE) {
        LOGW("Router stack size is larger than max size 32.");
        if (target.errorCallback != nullptr) {
            target.errorCallback("The pages are pushed too much.", ERROR_CODE_PAGE_STACK_FULL);
        }
        return;
    }
    RouterPageInfo info = target;
    info.path = info.url + ".js";

    if (target.routerMode == RouterMode::SINGLE) {
        auto pageInfo = FindPageInStack(info.url);
        if (pageInfo.second) {
            // find page in stack, move postion and update params.
            MovePageToFront(pageInfo.first, pageInfo.second, info, true);
            return;
        }
    }

    LoadPage(GenerateNextPageId(), info);
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto pageUrlChecker = container->GetPageUrlChecker();
    CHECK_NULL_VOID(pageUrlChecker);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask([pageUrlChecker, url = target.url]() { pageUrlChecker->CheckPreload(url); },
        TaskExecutor::TaskType::BACKGROUND, "ArkUIPageRouterPushOhmUrl");
}

void PageRouterManager::StartPush(const RouterPageInfo& target)
{
    CHECK_RUN_ON(JS);
    RouterOptScope scope(this);
    if (target.url.empty()) {
        return;
    }
#if !defined(PREVIEW)
    if (target.url.substr(0, strlen(BUNDLE_TAG)) == BUNDLE_TAG) {
        auto container = Container::Current();
        CHECK_NULL_VOID(container);
        auto pageUrlChecker = container->GetPageUrlChecker();
        CHECK_NULL_VOID(pageUrlChecker);
        auto instanceId = container->GetInstanceId();
        auto taskExecutor = container->GetTaskExecutor();
        CHECK_NULL_VOID(taskExecutor);
        auto callback = [weak = AceType::WeakClaim(this), target, taskExecutor, instanceId]() {
            ContainerScope scope(instanceId);
            auto pageRouterManager = weak.Upgrade();
            CHECK_NULL_VOID(pageRouterManager);
            taskExecutor->PostTask(
                [pageRouterManager, target]() { pageRouterManager->PushOhmUrl(target); },
                TaskExecutor::TaskType::JS, "ArkUIPageRouterPushOhmUrl");
        };

        auto silentInstallErrorCallBack = [errorCallback = target.errorCallback, taskExecutor, instanceId](
                                              int32_t errorCode, const std::string& errorMsg) {
            ContainerScope scope(instanceId);
            taskExecutor->PostTask([errorCallback, errorCode, errorMsg]() { errorCallback(errorMsg, errorCode); },
                TaskExecutor::TaskType::JS, "ArkUIPageRouterErrorCallback");
        };

        CleanPageOverlay();
        pageUrlChecker->LoadPageUrl(target.url, callback, silentInstallErrorCallBack);
        return;
    }
#endif
    if (!manifestParser_) {
        return;
    }
    if (GetStackSize() >= MAX_ROUTER_STACK_SIZE) {
        LOGW("Router stack size is larger than max size 32.");
        if (target.errorCallback != nullptr) {
            target.errorCallback("The pages are pushed too much.", ERROR_CODE_PAGE_STACK_FULL);
        }
        return;
    }
    RouterPageInfo info = target;
    info.path = manifestParser_->GetRouter()->GetPagePath(info.url);
    if (info.path.empty()) {
        LOGW("[Engine Log] this uri is empty, not support in route push.");
        if (info.errorCallback != nullptr) {
            info.errorCallback("The uri of router is not exist.", ERROR_CODE_URI_ERROR);
        }
        return;
    }

    CleanPageOverlay();

    if (info.routerMode == RouterMode::SINGLE) {
        auto pageInfo = FindPageInStack(info.url);
        if (pageInfo.second) {
            // find page in stack, move postion and update params.
            MovePageToFront(pageInfo.first, pageInfo.second, info, true);
            return;
        }
    }

    LoadPage(GenerateNextPageId(), info);
}

void PageRouterManager::ReplaceOhmUrl(const RouterPageInfo& target)
{
    RouterOptScope scope(this);
    RouterPageInfo info = target;
    info.path = info.url + ".js";

    PopPage("", false, false);

    if (info.routerMode == RouterMode::SINGLE) {
        auto pageInfo = FindPageInStack(info.url);
        if (pageInfo.second) {
            // find page in stack, move postion and update params.
            MovePageToFront(pageInfo.first, pageInfo.second, info, false, true, false);
            return;
        }
    }

    LoadPage(GenerateNextPageId(), info, false, false);
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto pageUrlChecker = container->GetPageUrlChecker();
    CHECK_NULL_VOID(pageUrlChecker);
    auto taskExecutor = container->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask([pageUrlChecker, url = target.url]() { pageUrlChecker->CheckPreload(url); },
        TaskExecutor::TaskType::BACKGROUND, "ArkUIPageRouterReplaceOhmUrl");
}

void PageRouterManager::StartReplace(const RouterPageInfo& target)
{
    CHECK_RUN_ON(JS);
    CleanPageOverlay();
    RouterOptScope scope(this);
    if (target.url.empty()) {
        return;
    }
#if !defined(PREVIEW)
    if (target.url.substr(0, strlen(BUNDLE_TAG)) == BUNDLE_TAG) {
        auto container = Container::Current();
        CHECK_NULL_VOID(container);
        auto pageUrlChecker = container->GetPageUrlChecker();
        CHECK_NULL_VOID(pageUrlChecker);
        auto instanceId = container->GetInstanceId();
        auto taskExecutor = container->GetTaskExecutor();
        CHECK_NULL_VOID(taskExecutor);
        auto callback = [weak = AceType::WeakClaim(this), target, taskExecutor, instanceId]() {
            ContainerScope scope(instanceId);
            auto pageRouterManager = weak.Upgrade();
            CHECK_NULL_VOID(pageRouterManager);
            taskExecutor->PostTask([pageRouterManager, target]() { pageRouterManager->ReplaceOhmUrl(target); },
                TaskExecutor::TaskType::JS, "ArkUIPageRouterReplaceOhmUrl");
        };

        auto silentInstallErrorCallBack = [errorCallback = target.errorCallback, taskExecutor, instanceId](
                                              int32_t errorCode, const std::string& errorMsg) {
            ContainerScope scope(instanceId);
            taskExecutor->PostTask([errorCallback, errorCode, errorMsg]() { errorCallback(errorMsg, errorCode); },
                TaskExecutor::TaskType::JS, "ArkUIPageRouterErrorCallback");
        };

        pageUrlChecker->LoadPageUrl(target.url, callback, silentInstallErrorCallBack);
        return;
    }
#endif
    if (!manifestParser_) {
        return;
    }
    RouterPageInfo info = target;
    info.path = manifestParser_->GetRouter()->GetPagePath(info.url);
    if (info.path.empty()) {
        LOGW("[Engine Log] this uri is empty, not support in route push.");
        if (info.errorCallback != nullptr) {
            info.errorCallback("The uri of router is not exist.", ERROR_CODE_URI_ERROR_LITE);
        }
        return;
    }

    DealReplacePage(info);
}

void PageRouterManager::StartBack(const RouterPageInfo& target)
{
    CleanPageOverlay();
    if (target.url.empty()) {
        size_t pageRouteSize = pageRouterStack_.size();
        if (pageRouteSize <= 1) {
            if (!restorePageStack_.empty()) {
                StartRestore(RouterPageInfo());
                return;
            }
            LOGI("Router back start ExitToDesktop");
            ExitToDesktop();
            return;
        }
        LOGI("Router back start PopPage");
        PopPage(target.params, true, true);
        return;
    }

    auto pageInfo = FindPageInStack(target.url, true);
    if (pageInfo.second) {
        // find page in stack, pop to specified index.
        RouterPageInfo info = target;
#if !defined(PREVIEW)
        if (info.url.substr(0, strlen(BUNDLE_TAG)) == BUNDLE_TAG) {
            info.path = info.url + ".js";
            PopPageToIndex(pageInfo.first, info.params, true, true);
            return;
        }
#endif
        if (!manifestParser_) {
            return;
        }

        info.path = manifestParser_->GetRouter()->GetPagePath(info.url);
        if (info.path.empty()) {
            LOGW("[Engine Log] this uri is empty, not support in route push.");
            return;
        }
        PopPageToIndex(pageInfo.first, info.params, true, true);
        return;
    }
    if (!restorePageStack_.empty()) {
        StartRestore(target);
    }
}

void PageRouterManager::StartBackToIndex(int32_t index, const std::string& params)
{
    CleanPageOverlay();
    RouterPageInfo target = GetPageInfoByIndex(index, params);
    if (!manifestParser_) {
        return;
    }
    PopPageToIndex(index - 1, params, true, true);
    return;
}

void PageRouterManager::BackCheckAlert(const RouterPageInfo& target)
{
    RouterOptScope scope(this);
    if (pageRouterStack_.empty()) {
        return;
    }
    auto currentPage = pageRouterStack_.back().Upgrade();
    CHECK_NULL_VOID(currentPage);
    auto pagePattern = currentPage->GetPattern<PagePattern>();
    CHECK_NULL_VOID(pagePattern);
    auto pageInfo = DynamicCast<EntryPageInfo>(pagePattern->GetPageInfo());
    CHECK_NULL_VOID(pageInfo);
    if (pageInfo->GetAlertCallback()) {
        ngBackTarget_ = target;
        auto pipelineContext = PipelineContext::GetCurrentContext();
        auto overlayManager = pipelineContext ? pipelineContext->GetOverlayManager() : nullptr;
        CHECK_NULL_VOID(overlayManager);
        overlayManager->ShowDialog(
            pageInfo->GetDialogProperties(), nullptr, AceApplicationInfo::GetInstance().IsRightToLeft());
        return;
    }
    StartBack(target);
}

void PageRouterManager::BackToIndexCheckAlert(int32_t index, const std::string& params)
{
    RouterOptScope scope(this);
    if (pageRouterStack_.empty()) {
        return;
    }
    RouterPageInfo target = GetPageInfoByIndex(index, params);
    auto currentPage = pageRouterStack_.back().Upgrade();
    CHECK_NULL_VOID(currentPage);
    auto pagePattern = currentPage->GetPattern<PagePattern>();
    CHECK_NULL_VOID(pagePattern);
    auto pageInfo = DynamicCast<EntryPageInfo>(pagePattern->GetPageInfo());
    CHECK_NULL_VOID(pageInfo);
    if (pageInfo->GetAlertCallback()) {
        ngBackTarget_ = target;
        auto pipelineContext = PipelineContext::GetCurrentContext();
        auto overlayManager = pipelineContext ? pipelineContext->GetOverlayManager() : nullptr;
        CHECK_NULL_VOID(overlayManager);
        overlayManager->ShowDialog(
            pageInfo->GetDialogProperties(), nullptr, AceApplicationInfo::GetInstance().IsRightToLeft());
        return;
    }
    StartBackToIndex(index, params);
}

void PageRouterManager::LoadPage(int32_t pageId, const RouterPageInfo& target, bool needHideLast, bool needTransition)
{
    ACE_SCOPED_TRACE("PageRouterManager::LoadPage");
    CHECK_RUN_ON(JS);
    LOGI("Page router manager is loading page[%{public}d]: %{public}s.", pageId, target.url.c_str());
    auto entryPageInfo = AceType::MakeRefPtr<EntryPageInfo>(pageId, target.url, target.path, target.params);
    auto pagePattern = AceType::MakeRefPtr<PagePattern>(entryPageInfo);
    std::unordered_map<std::string, std::string> reportData { { "pageUrl", target.url } };
    ResSchedReportScope reportScope("push_page", reportData);
    auto pageNode =
        FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), pagePattern);
    pageNode->SetHostPageId(pageId);
    pageRouterStack_.emplace_back(pageNode);

    if (target.content && !target.content->empty()) {
        loadJsByBuffer_(target.content, target.errorCallback);
    } else {
        loadJs_(target.path, target.errorCallback);
    }

    // record full path info of every pageNode
    auto pageInfo = pagePattern->GetPageInfo();
    CHECK_NULL_VOID(pageInfo);
    auto keyInfo = target.url;
    if (keyInfo.empty() && manifestParser_) {
        auto router = manifestParser_->GetRouter();
        if (router) {
            keyInfo = router->GetEntry("");
        }
    }
#if !defined(PREVIEW)
    if (keyInfo.substr(0, strlen(BUNDLE_TAG)) == BUNDLE_TAG) {
        // deal with @bundle url
        size_t bundleEndPos = keyInfo.find('/');
        size_t moduleStartPos = bundleEndPos + 1;
        size_t moduleEndPos = keyInfo.find('/', moduleStartPos);
        std::string moduleName = keyInfo.substr(moduleStartPos, moduleEndPos - moduleStartPos);
        keyInfo = keyInfo.substr(moduleEndPos + 1);
        keyInfo = moduleName + keyInfo;
    }
#endif
    auto pagePath = Framework::JsiDeclarativeEngine::GetFullPathInfo(keyInfo);
    if (pagePath.empty()) {
        auto container = Container::Current();
        CHECK_NULL_VOID(container);
        auto moduleName = container->GetModuleName();
        keyInfo = moduleName + keyInfo;
        pagePath = Framework::JsiDeclarativeEngine::GetFullPathInfo(keyInfo);
    }
    pageInfo->SetFullPath(pagePath);

#if defined(PREVIEW)
    if (!isComponentPreview_()) {
#endif
    auto result = loadNamedRouter_(target.url, target.isNamedRouterMode);
    if (!result) {
        if (!target.isNamedRouterMode) {
            result = updateRootComponent_();
        } else if (target.errorCallback) {
            target.errorCallback("The named route is not exist.", ERROR_CODE_NAMED_ROUTE_ERROR);
        }
    }

    if (!result) {
#if !defined(PREVIEW)
        if (!target.isNamedRouterMode && target.url.substr(0, strlen(BUNDLE_TAG)) != BUNDLE_TAG) {
            ThrowError("Load Page Failed: " + target.url, ERROR_CODE_LOAD_PAGE_ERROR);
        }
#endif
        pageRouterStack_.pop_back();
        return;
    }

    if (target.isNamedRouterMode) {
        manifestParser_->SetPagePath(target.url);
    }

    if (target.errorCallback != nullptr) {
        target.errorCallback("", ERROR_CODE_NO_ERROR);
    }
#if defined(PREVIEW)
    }
#endif

    if (!OnPageReady(pageNode, needHideLast, needTransition)) {
        pageRouterStack_.pop_back();
        LOGE("LoadPage OnPageReady Failed");
        return;
    }
    pageNode->OnAccessibilityEvent(AccessibilityEventType::CHANGE);
    LOGI("LoadPage Success");
}

UIContentErrorCode PageRouterManager::LoadCard(int32_t pageId, const RouterPageInfo& target, const std::string& params,
    int64_t cardId, bool /* isRestore */, bool needHideLast, const std::string& entryPoint)
{
    CHECK_RUN_ON(JS);
    auto entryPageInfo = AceType::MakeRefPtr<EntryPageInfo>(pageId, target.url, target.path, params);
    auto pagePattern = AceType::MakeRefPtr<PagePattern>(entryPageInfo);
    auto pageNode =
        FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), pagePattern);
    pageNode->SetHostPageId(pageId);
    pageRouterStack_.emplace_back(pageNode);

    if (!loadCard_) {
        return UIContentErrorCode::NULL_CARD_CALLBACK;
    }
    auto result = loadCard_(target.url, cardId, entryPoint);
    if (!result) {
        pageRouterStack_.pop_back();
        return UIContentErrorCode::NULL_CARD_RES;
    }

    if (!OnPageReady(pageNode, needHideLast, false, isCardRouter_, cardId)) {
        LOGE("LoadCard OnPageReady Failed");
        pageRouterStack_.pop_back();
        return UIContentErrorCode::CARD_PAGE_NOT_READY;
    }
    LOGI("LoadCard Success");
    return UIContentErrorCode::NO_ERRORS;
}

void PageRouterManager::MovePageToFront(int32_t index, const RefPtr<FrameNode>& pageNode, const RouterPageInfo& target,
    bool needHideLast, bool forceShowCurrent, bool needTransition)
{
    LOGI("Move page to front to index: %{public}d", index);
    if (target.errorCallback != nullptr) {
        target.errorCallback("", ERROR_CODE_NO_ERROR);
    }

    // update param first.
    CHECK_NULL_VOID(pageNode);
    auto pagePattern = pageNode->GetPattern<PagePattern>();
    CHECK_NULL_VOID(pagePattern);
    auto pageInfo = DynamicCast<EntryPageInfo>(pagePattern->GetPageInfo());
    CHECK_NULL_VOID(pageInfo);

    if (index == static_cast<int32_t>(pageRouterStack_.size() - 1)) {
        pageInfo->ReplacePageParams(target.params);
        if (forceShowCurrent) {
            pageNode->GetRenderContext()->ResetPageTransitionEffect();
            StageManager::FirePageShow(pageNode, PageTransitionType::NONE);
        }
        return;
    }
    CHECK_NULL_VOID(pageNode);
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto pipeline = container->GetPipelineContext();
    CHECK_NULL_VOID(pipeline);
    auto context = DynamicCast<NG::PipelineContext>(pipeline);
    auto stageManager = context ? context->GetStageManager() : nullptr;
    CHECK_NULL_VOID(stageManager);

    // clean pageNode on index position.
    auto iter = pageRouterStack_.begin();
    std::advance(iter, index);
    auto last = pageRouterStack_.erase(iter);
    // push pageNode to top.
    pageRouterStack_.emplace_back(pageNode);
    std::string tempParam = pageInfo->ReplacePageParams(target.params);
    if (!stageManager->MovePageToFront(pageNode, needHideLast, needTransition)) {
        // restore position and param.
        pageRouterStack_.pop_back();
        pageRouterStack_.insert(last, pageNode);
        if (!tempParam.empty()) {
            pageInfo->ReplacePageParams(tempParam);
        }
    }

    // update index in pageInfo
    for (auto iter = last; iter != pageRouterStack_.end(); ++iter, ++index) {
        auto pageNode = iter->Upgrade();
        if (!pageNode) {
            continue;
        }
        auto pagePattern = pageNode->GetPattern<NG::PagePattern>();
        if (pagePattern) {
            pagePattern->GetPageInfo()->SetPageIndex(index);
        }
    }
}

void PageRouterManager::FlushFrontend()
{
    auto currentPage = pageRouterStack_.back().Upgrade();
    CHECK_NULL_VOID(currentPage);
    auto customNode = DynamicCast<CustomNode>(currentPage->GetFirstChild());
    CHECK_NULL_VOID(customNode);
    customNode->FlushReload();
}

void PageRouterManager::PopPage(const std::string& params, bool needShowNext, bool needTransition)
{
    CHECK_RUN_ON(JS);
    if (pageRouterStack_.empty()) {
        LOGW("Page router stack size is zero, can not pop");
        return;
    }
    if (needShowNext && (pageRouterStack_.size() == 1)) {
        LOGW("Page router stack size is only one, can not show next");
        return;
    }
    auto topNode = pageRouterStack_.back();
    pageRouterStack_.pop_back();
    if (!needShowNext) {
        if (!OnPopPage(needShowNext, needTransition)) {
            pageRouterStack_.emplace_back(topNode);
        }
        return;
    }

    // update param first.
    auto nextNode = pageRouterStack_.back().Upgrade();
    CHECK_NULL_VOID(nextNode);
    auto pagePattern = nextNode->GetPattern<PagePattern>();
    CHECK_NULL_VOID(pagePattern);
    auto pageInfo = DynamicCast<EntryPageInfo>(pagePattern->GetPageInfo());
    CHECK_NULL_VOID(pageInfo);
    auto temp = pageInfo->ReplacePageParams(params);

    if (OnPopPage(needShowNext, needTransition)) {
        return;
    }
    // restore stack and pageParam.
    pageRouterStack_.emplace_back(topNode);
    pageInfo->ReplacePageParams(temp);
}

void PageRouterManager::PopPageToIndex(int32_t index, const std::string& params, bool needShowNext, bool needTransition)
{
    std::list<WeakPtr<FrameNode>> temp;
    std::swap(temp, pageRouterStack_);
    auto iter = temp.begin();
    for (int32_t current = 0; current <= index; ++current) {
        pageRouterStack_.emplace_back(*iter);
        ++iter;
    }

    // update param first.
    auto nextNode = pageRouterStack_.back().Upgrade();
    CHECK_NULL_VOID(nextNode);
    auto pagePattern = nextNode->GetPattern<PagePattern>();
    CHECK_NULL_VOID(pagePattern);
    auto pageInfo = DynamicCast<EntryPageInfo>(pagePattern->GetPageInfo());
    CHECK_NULL_VOID(pageInfo);
    auto tempParam = pageInfo->ReplacePageParams(params);

    if (OnPopPageToIndex(index, needShowNext, needTransition)) {
        return;
    }
    // restore stack and pageParam.
    std::swap(temp, pageRouterStack_);
    pageInfo->ReplacePageParams(tempParam);
}

bool PageRouterManager::OnPageReady(
    const RefPtr<FrameNode>& pageNode, bool needHideLast, bool needTransition, bool isCardRouter, int64_t cardId)
{
    Recorder::NodeDataCache::Get().OnPageReady();
    auto container = Container::Current();
    CHECK_NULL_RETURN(container, false);
    RefPtr<PipelineBase> pipeline;
    if (isCardRouter) {
        auto weak = container->GetCardPipeline(cardId);
        pipeline = weak.Upgrade();
        CHECK_NULL_RETURN(pipeline, false);
    } else {
        pipeline = container->GetPipelineContext();
        CHECK_NULL_RETURN(pipeline, false);
    }

    auto context = DynamicCast<NG::PipelineContext>(pipeline);
    auto stageManager = context ? context->GetStageManager() : nullptr;
    if (stageManager) {
        return stageManager->PushPage(pageNode, needHideLast, needTransition);
    }
    return false;
}

bool PageRouterManager::OnPopPage(bool needShowNext, bool needTransition)
{
    auto container = Container::Current();
    CHECK_NULL_RETURN(container, false);
    auto pipeline = container->GetPipelineContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto context = DynamicCast<NG::PipelineContext>(pipeline);
    auto stageManager = context ? context->GetStageManager() : nullptr;
    if (stageManager) {
        Recorder::NodeDataCache::Get().OnBeforePagePop();
        return stageManager->PopPage(needShowNext, needTransition);
    }
    return false;
}

bool PageRouterManager::OnPopPageToIndex(int32_t index, bool needShowNext, bool needTransition)
{
    auto container = Container::Current();
    CHECK_NULL_RETURN(container, false);
    auto pipeline = container->GetPipelineContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto context = DynamicCast<NG::PipelineContext>(pipeline);
    auto stageManager = context ? context->GetStageManager() : nullptr;
    if (stageManager) {
        Recorder::NodeDataCache::Get().OnBeforePagePop();
        return stageManager->PopPageToIndex(index, needShowNext, needTransition);
    }
    return false;
}

bool PageRouterManager::OnCleanPageStack()
{
    auto container = Container::Current();
    CHECK_NULL_RETURN(container, false);
    auto pipeline = container->GetPipelineContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto context = DynamicCast<NG::PipelineContext>(pipeline);
    auto stageManager = context ? context->GetStageManager() : nullptr;
    if (stageManager) {
        return stageManager->CleanPageStack();
    }
    return false;
}

void PageRouterManager::CleanPageOverlay()
{
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto pipeline = container->GetPipelineContext();
    CHECK_NULL_VOID(pipeline);
    auto context = DynamicCast<NG::PipelineContext>(pipeline);
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    auto sharedManager = context->GetSharedOverlayManager();
    if (sharedManager) {
        sharedManager->StopSharedTransition();
    }

    overlayManager->RemoveOverlay(true, true);
}

void PageRouterManager::DealReplacePage(const RouterPageInfo& info)
{
    if (AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE)) {
        auto pipelineContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        auto stageManager = pipelineContext->GetStageManager();
        auto stageNode = stageManager->GetStageNode();
        auto popNode = stageNode->GetChildren().back();
        int8_t popIndex = static_cast<int8_t>(stageNode->GetChildren().size() - 1);
        bool findPage = false;
        if (info.routerMode == RouterMode::SINGLE) {
            auto pageInfo = FindPageInStack(info.url);
            if (pageInfo.second) {
                // find page in stack, move position and update params.
                MovePageToFront(pageInfo.first, pageInfo.second, info, false, true, false);
                findPage = true;
            }
        }
        if (!findPage) {
            LoadPage(GenerateNextPageId(), info, true, false);
        }
        if (popIndex < 0 || popNode == stageNode->GetChildren().back()) {
            return;
        }
        auto iter = pageRouterStack_.begin();
        std::advance(iter, popIndex);
        pageRouterStack_.erase(iter);
        pageRouterStack_.emplace_back(WeakPtr<FrameNode>(AceType::DynamicCast<FrameNode>(popNode)));
        popNode->MovePosition(stageNode->GetChildren().size() - 1);
        PopPage("", false, false);
        return;
    }
    PopPage("", false, false);
    if (info.routerMode == RouterMode::SINGLE) {
        auto pageInfo = FindPageInStack(info.url);
        if (pageInfo.second) {
            // find page in stack, move position and update params.
            MovePageToFront(pageInfo.first, pageInfo.second, info, false, true, false);
            return;
        }
    }
    LoadPage(GenerateNextPageId(), info, false, false);
}

bool PageRouterManager::CheckIndexValid(int32_t index) const
{
    if (index > static_cast<int32_t>(pageRouterStack_.size()) || index <= 0) {
        LOGW("The index is less than or equal to zero or exceeds the maximum length of the page stack");
        return false;
    }
    return true;
}

bool PageRouterManager::CheckOhmUrlValid(const std::string& ohmUrl)
{
    size_t bundleEndPos = ohmUrl.find('/');
    std::string bundleName = ohmUrl.substr(BUNDLE_START_POS, bundleEndPos - BUNDLE_START_POS);
    size_t moduleStartPos = bundleEndPos + 1;
    size_t moduleEndPos = ohmUrl.find('/', moduleStartPos);
    std::string moduleName = ohmUrl.substr(moduleStartPos, moduleEndPos - moduleStartPos);
    auto runtime = std::static_pointer_cast<Framework::ArkJSRuntime>(
        Framework::JsiDeclarativeEngineInstance::GetCurrentRuntime());
    return runtime->IsExecuteModuleInAbcFile(bundleName, moduleName, ohmUrl);
}

void PageRouterManager::ThrowError(const std::string& msg, int32_t code)
{
    auto runtime = std::static_pointer_cast<Framework::ArkJSRuntime>(
        Framework::JsiDeclarativeEngineInstance::GetCurrentRuntime());
    runtime->ThrowError(msg, code);
}
} // namespace OHOS::Ace::NG
