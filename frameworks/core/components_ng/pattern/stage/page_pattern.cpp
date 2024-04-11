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

#include "core/components_ng/pattern/stage/page_pattern.h"

#include "base/log/jank_frame_report.h"
#include "base/log/log_wrapper.h"
#include "base/perfmonitor/perf_monitor.h"
#include "base/utils/time_util.h"
#include "base/utils/utils.h"
#include "core/animation/animator.h"
#include "core/common/container.h"
#include "core/common/recorder/event_recorder.h"
#include "core/components/common/properties/alignment.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/components_ng/base/observer_handler.h"
#include "bridge/common/utils/engine_helper.h"

namespace OHOS::Ace::NG {

namespace {
constexpr int32_t INVALID_PAGE_INDEX = -1;
std::string KEY_PAGE_TRANSITION_PROPERTY = "pageTransitionProperty";
void IterativeAddToSharedMap(const RefPtr<UINode>& node, SharedTransitionMap& map)
{
    const auto& children = node->GetChildren();
    for (const auto& child : children) {
        auto frameChild = AceType::DynamicCast<FrameNode>(child);
        if (!frameChild) {
            IterativeAddToSharedMap(child, map);
            continue;
        }
        auto id = frameChild->GetRenderContext()->GetShareId();
        if (!id.empty()) {
            map[id] = frameChild;
        }
        IterativeAddToSharedMap(frameChild, map);
    }
}
} // namespace

void PagePattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    MeasureType measureType = MeasureType::MATCH_PARENT;
    auto container = Container::Current();
    if (container && container->IsDynamicRender()) {
        measureType = MeasureType::MATCH_CONTENT;
    }
    host->GetLayoutProperty()->UpdateMeasureType(measureType);
    host->GetLayoutProperty()->UpdateAlignment(Alignment::TOP_LEFT);
}

bool PagePattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& wrapper, const DirtySwapConfig& /* config */)
{
    if (isFirstLoad_) {
        isFirstLoad_ = false;
        if (firstBuildCallback_) {
            firstBuildCallback_();
            firstBuildCallback_ = nullptr;
        }
    }
    if (dynamicPageSizeCallback_) {
        auto node = wrapper->GetGeometryNode();
        CHECK_NULL_RETURN(node, false);
        dynamicPageSizeCallback_(node->GetFrameSize());
    }
    return false;
}

bool PagePattern::TriggerPageTransition(PageTransitionType type, const std::function<void()>& onFinish)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, false);
    if (pageTransitionFunc_) {
        pageTransitionFunc_();
    }
    auto effect = FindPageTransitionEffect(type);
    pageTransitionFinish_ = std::make_shared<std::function<void()>>(onFinish);
    auto wrappedOnFinish = [weak = WeakClaim(this), sharedFinish = pageTransitionFinish_]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto host = pattern->GetHost();
        CHECK_NULL_VOID(host);
        if (sharedFinish == pattern->pageTransitionFinish_) {
            // ensure this is exactly the finish callback saved in pagePattern,
            // otherwise means new pageTransition started
            pattern->FirePageTransitionFinish();
            host->DeleteAnimatablePropertyFloat(KEY_PAGE_TRANSITION_PROPERTY);
        }
    };
    if (effect && effect->GetUserCallback()) {
        RouteType routeType = (type == PageTransitionType::ENTER_POP || type == PageTransitionType::EXIT_POP)
                                  ? RouteType::POP
                                  : RouteType::PUSH;
        host->CreateAnimatablePropertyFloat(KEY_PAGE_TRANSITION_PROPERTY, 0.0f,
            [routeType, handler = effect->GetUserCallback()](const float& progress) { handler(routeType, progress); });
        auto handler = effect->GetUserCallback();
        handler(routeType, 0.0f);
        AnimationOption option(effect->GetCurve(), effect->GetDuration());
        option.SetDelay(effect->GetDelay());
        AnimationUtils::OpenImplicitAnimation(option, option.GetCurve(), wrappedOnFinish);
        host->UpdateAnimatablePropertyFloat(KEY_PAGE_TRANSITION_PROPERTY, 1.0f);
        AnimationUtils::CloseImplicitAnimation();
        return renderContext->TriggerPageTransition(type, nullptr);
    }
    return renderContext->TriggerPageTransition(type, wrappedOnFinish);
}

void PagePattern::ProcessAutoSave()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (!host->NeedRequestAutoSave()) {
        return;
    }
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    container->RequestAutoSave(host);
}

void PagePattern::ProcessHideState()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->SetActive(false);
    host->NotifyVisibleChange(false);
    host->GetLayoutProperty()->UpdateVisibility(VisibleType::INVISIBLE);
    auto parent = host->GetAncestorNodeOfFrame();
    CHECK_NULL_VOID(parent);
    parent->MarkNeedSyncRenderTree();
    parent->RebuildRenderContextTree();
}

void PagePattern::ProcessShowState()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->SetActive(true);
    host->NotifyVisibleChange(true);
    host->GetLayoutProperty()->UpdateVisibility(VisibleType::VISIBLE);
    auto parent = host->GetAncestorNodeOfFrame();
    CHECK_NULL_VOID(parent);
    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto manager = context->GetSafeAreaManager();
    if (manager) {
        auto safeArea = manager->GetSafeArea();
        auto parentGlobalOffset = host->GetParentGlobalOffsetDuringLayout();
        auto geometryNode = host->GetGeometryNode();
        auto frame = geometryNode->GetFrameRect() + parentGlobalOffset;
        // if page's frameRect not fit current safeArea, need layout page again
        if (!NearEqual(frame.GetY(), safeArea.top_.end)) {
            host->MarkDirtyNode(manager->KeyboardSafeAreaEnabled() ? PROPERTY_UPDATE_LAYOUT : PROPERTY_UPDATE_MEASURE);
        }
        if (!NearEqual(frame.GetY() + frame.Height(), safeArea.bottom_.start)) {
            host->MarkDirtyNode(manager->KeyboardSafeAreaEnabled() ? PROPERTY_UPDATE_LAYOUT : PROPERTY_UPDATE_MEASURE);
        }
    }
    parent->MarkNeedSyncRenderTree();
    parent->RebuildRenderContextTree();
}

void PagePattern::OnAttachToMainTree()
{
    int32_t index = INVALID_PAGE_INDEX;
    auto delegate = EngineHelper::GetCurrentDelegate();
    if (delegate) {
        index = delegate->GetStackSize();
    }
    GetPageInfo()->SetPageIndex(index);
    state_ = RouterPageState::ABOUT_TO_APPEAR;
    UIObserverHandler::GetInstance().NotifyRouterPageStateChange(GetPageInfo(), state_);
}

void PagePattern::OnDetachFromMainTree()
{
    state_ = RouterPageState::ABOUT_TO_DISAPPEAR;
    UIObserverHandler::GetInstance().NotifyRouterPageStateChange(GetPageInfo(), state_);
}

void PagePattern::OnShow()
{
    // Do not invoke onPageShow unless the initialRender function has been executed.
    CHECK_NULL_VOID(isRenderDone_);
    CHECK_NULL_VOID(!isOnShow_);
    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    if (pageInfo_) {
        context->FirePageChanged(pageInfo_->GetPageId(), true);
    }
    auto container = Container::Current();
    if (!container || !container->WindowIsShow()) {
        LOGW("no need to trigger onPageShow callback when not in the foreground");
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->SetJSViewActive(true);
    isOnShow_ = true;
    state_ = RouterPageState::ON_PAGE_SHOW;
    UIObserverHandler::GetInstance().NotifyRouterPageStateChange(GetPageInfo(), state_);
    JankFrameReport::GetInstance().StartRecord(pageInfo_->GetPageUrl());
    PerfMonitor::GetPerfMonitor()->SetPageUrl(pageInfo_->GetPageUrl());
    auto pageUrlChecker = container->GetPageUrlChecker();
    if (pageUrlChecker != nullptr) {
        pageUrlChecker->NotifyPageShow(pageInfo_->GetPageUrl());
    }
    if (onPageShow_) {
        onPageShow_();
    }
    if (onHiddenChange_) {
        onHiddenChange_(true);
    }
    if (Recorder::EventRecorder::Get().IsPageRecordEnable()) {
        std::string param;
        auto entryPageInfo = DynamicCast<EntryPageInfo>(pageInfo_);
        if (entryPageInfo) {
            param = entryPageInfo->GetPageParams();
            entryPageInfo->SetShowTime(GetCurrentTimestamp());
        }
        Recorder::EventRecorder::Get().OnPageShow(pageInfo_->GetPageUrl(), param);
    }
}

void PagePattern::OnHide()
{
    CHECK_NULL_VOID(isOnShow_);
    JankFrameReport::GetInstance().FlushRecord();
    auto context = NG::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    if (pageInfo_) {
        context->FirePageChanged(pageInfo_->GetPageId(), false);
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->SetJSViewActive(false);
    isOnShow_ = false;
    state_ = RouterPageState::ON_PAGE_HIDE;
    UIObserverHandler::GetInstance().NotifyRouterPageStateChange(GetPageInfo(), state_);
    auto container = Container::Current();
    if (container) {
        auto pageUrlChecker = container->GetPageUrlChecker();
        // ArkTSCard container no SetPageUrlChecker
        if (pageUrlChecker != nullptr) {
            pageUrlChecker->NotifyPageHide(pageInfo_->GetPageUrl());
        }
    }
    if (onPageHide_) {
        onPageHide_();
    }
    if (onHiddenChange_) {
        onHiddenChange_(false);
    }
    if (Recorder::EventRecorder::Get().IsPageRecordEnable()) {
        auto entryPageInfo = DynamicCast<EntryPageInfo>(pageInfo_);
        int64_t duration = 0;
        if (entryPageInfo && entryPageInfo->GetShowTime() > 0) {
            duration = GetCurrentTimestamp() - entryPageInfo->GetShowTime();
        }
        Recorder::EventRecorder::Get().OnPageHide(pageInfo_->GetPageUrl(), duration);
    }
}

bool PagePattern::OnBackPressed()
{
    if (RemoveOverlay()) {
        TAG_LOGI(AceLogTag::ACE_OVERLAY, "page removes it's overlay when on backpressed");
        return true;
    }
    if (isPageInTransition_) {
        return true;
    }
    // if in page transition, do not set to ON_BACK_PRESS
    state_ = RouterPageState::ON_BACK_PRESS;
    UIObserverHandler::GetInstance().NotifyRouterPageStateChange(GetPageInfo(), state_);
    if (onBackPressed_) {
        return onBackPressed_();
    }
    return false;
}

void PagePattern::BuildSharedTransitionMap()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    sharedTransitionMap_.clear();
    IterativeAddToSharedMap(host, sharedTransitionMap_);
}

void PagePattern::ReloadPage()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto customNode = DynamicCast<CustomNodeBase>(host->GetFirstChild());
    CHECK_NULL_VOID(customNode);
    customNode->FireReloadFunction(true);
}

RefPtr<PageTransitionEffect> PagePattern::FindPageTransitionEffect(PageTransitionType type)
{
    RefPtr<PageTransitionEffect> result;
    for (auto iter = pageTransitionEffects_.rbegin(); iter != pageTransitionEffects_.rend(); ++iter) {
        auto effect = *iter;
        if (effect->CanFit(type)) {
            result = effect;
            break;
        }
    }
    return result;
}

void PagePattern::ClearPageTransitionEffect()
{
    pageTransitionEffects_.clear();
}

RefPtr<PageTransitionEffect> PagePattern::GetTopTransition() const
{
    return pageTransitionEffects_.empty() ? nullptr : pageTransitionEffects_.back();
}

void PagePattern::AddPageTransition(const RefPtr<PageTransitionEffect>& effect)
{
    pageTransitionEffects_.emplace_back(effect);
}

void PagePattern::AddJsAnimator(const std::string& animatorId, const RefPtr<Framework::AnimatorInfo>& animatorInfo)
{
    CHECK_NULL_VOID(animatorInfo);
    auto animator = animatorInfo->GetAnimator();
    CHECK_NULL_VOID(animator);
    animator->AttachScheduler(PipelineContext::GetCurrentContext());
    jsAnimatorMap_[animatorId] = animatorInfo;
}

RefPtr<Framework::AnimatorInfo> PagePattern::GetJsAnimator(const std::string& animatorId)
{
    auto iter = jsAnimatorMap_.find(animatorId);
    if (iter != jsAnimatorMap_.end()) {
        return iter->second;
    }
    return nullptr;
}

void PagePattern::SetFirstBuildCallback(std::function<void()>&& buildCallback)
{
    if (isFirstLoad_) {
        firstBuildCallback_ = std::move(buildCallback);
    } else if (buildCallback) {
        buildCallback();
    }
}

void PagePattern::FirePageTransitionFinish()
{
    if (pageTransitionFinish_) {
        auto onFinish = *pageTransitionFinish_;
        pageTransitionFinish_ = nullptr;
        if (onFinish) {
            onFinish();
        }
    }
}

void PagePattern::StopPageTransition()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto property = host->GetAnimatablePropertyFloat(KEY_PAGE_TRANSITION_PROPERTY);
    if (property) {
        FirePageTransitionFinish();
        return;
    }
    AnimationOption option(Curves::LINEAR, 0);
    AnimationUtils::Animate(
        option, [host]() { host->UpdateAnimatablePropertyFloat(KEY_PAGE_TRANSITION_PROPERTY, 0.0f); },
        nullptr);
    host->DeleteAnimatablePropertyFloat(KEY_PAGE_TRANSITION_PROPERTY);
    FirePageTransitionFinish();
}

void PagePattern::BeforeCreateLayoutWrapper()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    // SafeArea already applied to AppBar (AtomicServicePattern)
    if (pipeline->GetInstallationFree()) {
        return;
    }
    ContentRootPattern::BeforeCreateLayoutWrapper();
}

bool PagePattern::AvoidKeyboard() const
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    return pipeline->GetSafeAreaManager()->KeyboardSafeAreaEnabled();
}

bool PagePattern::RemoveOverlay()
{
    CHECK_NULL_RETURN(overlayManager_, false);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto taskExecutor = pipeline->GetTaskExecutor();
    CHECK_NULL_RETURN(taskExecutor, false);
    return overlayManager_->RemoveOverlay(true);
}

void PagePattern::MarkDirtyOverlay()
{
    CHECK_NULL_VOID(overlayManager_);
    overlayManager_->MarkDirtyOverlay();
}
} // namespace OHOS::Ace::NG
