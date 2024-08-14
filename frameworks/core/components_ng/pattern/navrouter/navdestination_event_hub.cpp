/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "frameworks/core/components_ng/pattern/navrouter/navdestination_event_hub.h"

#include "core/common/container.h"
#include "frameworks/core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "frameworks/core/components_ng/pattern/navrouter/navdestination_pattern.h"
namespace OHOS::Ace::NG {
void NavDestinationEventHub::FireOnDisappear()
{
    auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(GetFrameNode());
    CHECK_NULL_VOID(navDestination);
    if (navDestination->GetIsAnimated()) {
        auto pattern = navDestination->GetPattern<NavDestinationPattern>();
        CHECK_NULL_VOID(pattern);
        state_ = NavDestinationState::ON_DISAPPEAR;
        UIObserverHandler::GetInstance().NotifyNavigationStateChange(pattern, NavDestinationState::ON_DISAPPEAR);
        FireDisappearCallback();
        pattern->SetCustomNode(nullptr);
        return;
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    pipelineContext->AddAfterLayoutTask([destination = navDestination]() {
        auto eventHub = destination->GetEventHub<NavDestinationEventHub>();
        CHECK_NULL_VOID(eventHub);
        auto pattern = destination->GetPattern<NavDestinationPattern>();
        CHECK_NULL_VOID(pattern);
        eventHub->state_ = NavDestinationState::ON_DISAPPEAR;
        UIObserverHandler::GetInstance().NotifyNavigationStateChange(pattern, NavDestinationState::ON_DISAPPEAR);
        eventHub->FireDisappearCallback();
        pattern->SetCustomNode(nullptr);
    });
}

void NavDestinationEventHub::FireAutoSave()
{
    auto node = GetFrameNode();
    CHECK_NULL_VOID(node);
    if (!node->NeedRequestAutoSave()) {
        return;
    }
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    container->RequestAutoSave(node);
}

void NavDestinationEventHub::FireOnShownEvent(const std::string& name, const std::string& param)
{
    TAG_LOGI(AceLogTag::ACE_NAVIGATION, "%{public}s lifecycle change to onShown state.", name_.c_str());
    state_= NavDestinationState::ON_SHOWN;
    UIObserverHandler::GetInstance().NotifyNavigationStateChange(GetNavDestinationPattern(),
        NavDestinationState::ON_SHOWN);
    if (onShownEvent_) {
        auto onShownEvent = onShownEvent_;
        onShownEvent();
    }
    if (!onHiddenChange_.empty()) {
        FireOnHiddenChange(true);
    }
    if (Recorder::EventRecorder::Get().IsPageRecordEnable()) {
        auto host = GetFrameNode();
        CHECK_NULL_VOID(host);
        auto id = host->GetInspectorIdValue("");
        Recorder::EventParamsBuilder builder;
        builder.SetId(id)
            .SetText(name)
            .SetExtra(Recorder::KEY_PAGE_PARAM, param)
            .SetDescription(host->GetAutoEventParamValue(""));
        Recorder::EventRecorder::Get().OnNavDstShow(std::move(builder));
    }
}

void NavDestinationEventHub::FireOnHiddenEvent(const std::string& name)
{
    TAG_LOGI(AceLogTag::ACE_NAVIGATION, "%{public}s lifecycle chang to onHidden state.", name_.c_str());
    state_ = NavDestinationState::ON_HIDDEN;
    UIObserverHandler::GetInstance().NotifyNavigationStateChange(GetNavDestinationPattern(),
        NavDestinationState::ON_HIDDEN);
    if (onHiddenEvent_) {
        onHiddenEvent_();
    }
    if (!onHiddenChange_.empty()) {
        FireOnHiddenChange(false);
    }
    if (Recorder::EventRecorder::Get().IsPageRecordEnable()) {
        auto host = GetFrameNode();
        CHECK_NULL_VOID(host);
        auto id = host->GetInspectorIdValue("");
        Recorder::EventParamsBuilder builder;
        builder.SetId(id).SetText(name).SetDescription(host->GetAutoEventParamValue(""));
        Recorder::EventRecorder::Get().OnNavDstHide(std::move(builder));
    }
}

void NavDestinationEventHub::FireOnAppear()
{
    TAG_LOGI(AceLogTag::ACE_NAVIGATION, "%{public}s lifecycle change to onAppear state.", name_.c_str());
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto navigationManager = pipeline->GetNavigationManager();
    auto navDestination = AceType::DynamicCast<NavDestinationGroupNode>(GetFrameNode());
    CHECK_NULL_VOID(navDestination);
    auto pattern = navDestination->GetPattern<NavDestinationPattern>();
    CHECK_NULL_VOID(pattern);
    if (pattern->GetNavigationNode()) {
        navigationManager->AddNavigationUpdateCallback([weakEventHub = WeakClaim(this)]() {
            auto eventHub = weakEventHub.Upgrade();
            CHECK_NULL_VOID(eventHub);
            eventHub->state_ = NavDestinationState::ON_APPEAR;
            UIObserverHandler::GetInstance().NotifyNavigationStateChange(eventHub->GetNavDestinationPattern(),
                NavDestinationState::ON_APPEAR);
            if (eventHub->onAppear_) {
                auto onAppear = eventHub->onAppear_;
                onAppear();
            }
            if (eventHub->onJSFrameNodeAppear_) {
                auto onJSFrameNodeAppear = eventHub->onJSFrameNodeAppear_;
                onJSFrameNodeAppear();
            }
        });
    } else {
        EventHub::FireOnAppear();
    }
}

void NavDestinationEventHub::FireOnWillAppear()
{
    TAG_LOGI(AceLogTag::ACE_NAVIGATION, "%{public}s lifecycle change to onWillAppear state.", name_.c_str());
    state_ = NavDestinationState::ON_WILL_APPEAR;
    UIObserverHandler::GetInstance().NotifyNavigationStateChange(GetNavDestinationPattern(),
        NavDestinationState::ON_WILL_APPEAR);
    if (onWillAppear_) {
        onWillAppear_();
    }
}

void NavDestinationEventHub::FireOnWillShow()
{
    TAG_LOGI(AceLogTag::ACE_NAVIGATION, "%{public}s lifecycle change to onWillShow state.", name_.c_str());
    state_ = NavDestinationState::ON_WILL_SHOW;
    UIObserverHandler::GetInstance().NotifyNavigationStateChange(GetNavDestinationPattern(),
        NavDestinationState::ON_WILL_SHOW);
    if (onWillShow_) {
        onWillShow_();
    }
}

void NavDestinationEventHub::FireOnWillHide()
{
    TAG_LOGI(AceLogTag::ACE_NAVIGATION, "%{public}s lifecycle change to onWillHide state.", name_.c_str());
    state_ = NavDestinationState::ON_WILL_HIDE;
    UIObserverHandler::GetInstance().NotifyNavigationStateChange(GetNavDestinationPattern(),
        NavDestinationState::ON_WILL_HIDE);
    if (onWillHide_) {
        onWillHide_();
    }

    FireAutoSave();
}

void NavDestinationEventHub::FireOnWillDisAppear()
{
    TAG_LOGI(AceLogTag::ACE_NAVIGATION, "%{public}s lifecycle change to onWillDisappear state.", name_.c_str());
    state_ = NavDestinationState::ON_WILL_DISAPPEAR;
    UIObserverHandler::GetInstance().NotifyNavigationStateChange(GetNavDestinationPattern(),
        NavDestinationState::ON_WILL_DISAPPEAR);
    if (onWillDisAppear_) {
        onWillDisAppear_();
    }
}

bool NavDestinationEventHub::FireOnBackPressedEvent()
{
    state_ = NavDestinationState::ON_BACKPRESS;
    UIObserverHandler::GetInstance().NotifyNavigationStateChange(GetNavDestinationPattern(),
        NavDestinationState::ON_BACKPRESS);
    if (onBackPressedEvent_) {
        TAG_LOGI(AceLogTag::ACE_NAVIGATION, "navDestination backButton press is happening.");
        return onBackPressedEvent_();
    }
    return false;
}
}