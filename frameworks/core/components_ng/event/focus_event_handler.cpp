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
#include "focus_event_handler.h"
#include "core/common/container.h"

#include "base/input_manager/input_manager.h"
#include "core/common/event_manager.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/node_sensitive_input.h"
#include "core/components_ng/event/event_constants.h"
#include "core/components/theme/app_theme.h"
#include "core/components_ng/manager/focus/focus_manager.h"
#include "core/event/focus_axis_event.h"
#include "core/event/key_event.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/event/crown_event.h"
namespace OHOS::Ace::NG {
struct NodeSensitiveKeyDispatch {
    WeakPtr<FrameNode> initialNode;
    bool notified = false;
    std::unordered_set<int32_t> reportedNodes;
};

std::vector<RefPtr<FrameNode>> FocusEventHandler::CollectSensitiveKeyNodes(const FocusEvent& event)
{
    std::vector<RefPtr<FrameNode>> nodes;
    auto current = Claim(this);
    while (current) {
        auto node = current->GetFrameNode();
        if (node) {
            nodes.emplace_back(node);
        }
        if (current->focusType_ != FocusType::SCOPE || current->HasCustomKeyEventDispatch(event)) {
            break;
        }
        current = current->lastWeakFocusNode_.Upgrade();
        if (current && !current->IsCurrentFocus()) {
            break;
        }
    }
    return nodes;
}

void FocusEventHandler::PrepareSensitiveKeyDispatch(const FocusEvent& event)
{
    if (event.sensitiveInput || event.event.eventType != UIInputEventType::KEY) {
        return;
    }
    auto dispatch = event.sensitiveInput = std::make_shared<NodeSensitiveKeyDispatch>();
    auto nodes = CollectSensitiveKeyNodes(event);
    if (!nodes.empty()) {
        dispatch->initialNode = nodes.back();
    }
}

bool FocusEventHandler::RejectAgentKeyEvent(const FocusEvent& event)
{
    auto dispatch = event.sensitiveInput;
    CHECK_NULL_RETURN(dispatch, false);
    const auto& key = static_cast<const KeyEvent&>(event.event);
    auto node = GetFrameNode();
    CHECK_NULL_RETURN(node, false);
    auto hit = dispatch->initialNode.Upgrade();
    if (!dispatch->notified && hit && !key.isRedispatch) {
        dispatch->notified = true;
        NodeSensitiveInputInfo info;
        info.isAgent = key.isAgent;
        info.type = NodeSensitiveInputEventType::KEY;
        info.keyEvent = &key;
        NotifyNodeSensitiveInput(hit, info);
    }
    const bool controlled = key.isAgent && !key.isRedispatch &&
        (key.code == KeyCode::KEY_ENTER || key.code == KeyCode::KEY_NUMPAD_ENTER);
    const bool fenced = controlled && InputManager::HasControlComponentPermission();
    if (controlled && key.hasSafSessionId && node->HasAgentControlMode() &&
        !dispatch->reportedNodes.count(node->GetId()) &&
        (key.action == KeyAction::DOWN || key.action == KeyAction::UP || key.action == KeyAction::CANCEL)) {
        dispatch->reportedNodes.insert(node->GetId());
        auto compId = node->GetInspectorId().value_or("");
        if (compId.empty()) {
            compId = std::to_string(node->GetId());
        }
        AgentInputResult result;
        result.sessionId = key.safSessionId;
        result.callingUid = key.callingUid;
        result.instanceId = Container::CurrentId();
        result.nodeId = node->GetId();
        result.compId = compId;
        result.tag = node->GetTag();
        result.allowed = !fenced || node->GetAgentControlMode() != AgentControlMode::DENY;
        InputManager::ReportAgentInputDecision(result);
    }
    return fenced && node->GetAgentControlMode() == AgentControlMode::DENY;
}

FocusIntension FocusEvent::GetFocusIntension(const NonPointerEvent& event)
{
    if (event.eventType != UIInputEventType::KEY) {
        return FocusIntension::NONE;
    }
    const KeyEvent& keyEvent = static_cast<const KeyEvent&>(event);
    if (keyEvent.isPreIme || keyEvent.action != KeyAction::DOWN) {
        return FocusIntension::NONE;
    }
    // Arrow key event is used to trasfer focus regardless of its pressed keys
    switch (keyEvent.code) {
        case KeyCode::KEY_DPAD_UP:
            return FocusIntension::UP;
        case KeyCode::KEY_DPAD_DOWN:
            return FocusIntension::DOWN;
        case KeyCode::KEY_DPAD_LEFT:
            return FocusIntension::LEFT;
        case KeyCode::KEY_DPAD_RIGHT:
            return FocusIntension::RIGHT;
        default:;
    }

    if (keyEvent.pressedCodes.size() != 1) {
        return keyEvent.IsExactlyShiftWith(KeyCode::KEY_TAB) ? FocusIntension::SHIFT_TAB : FocusIntension::NONE;
    }
    switch (keyEvent.code) {
        case KeyCode::KEY_TAB:
            return FocusIntension::TAB;
        case KeyCode::KEY_MOVE_HOME:
            return FocusIntension::HOME;
        case KeyCode::KEY_MOVE_END:
            return FocusIntension::END;
        case KeyCode::KEY_ENTER:
        case KeyCode::KEY_NUMPAD_ENTER:
            return FocusIntension::SELECT;
        case KeyCode::KEY_SPACE:
            return FocusIntension::SPACE;
        default:;
    }
    return GetFocusIntensionFromKey(keyEvent.keyIntention);
}

FocusIntension FocusEvent::GetFocusIntensionFromKey(KeyIntention keyIntention)
{
    switch (keyIntention) {
        case KeyIntention::INTENTION_SELECT:
            return FocusIntension::SELECT;
        case KeyIntention::INTENTION_ESCAPE:
            return FocusIntension::ESC;
        case KeyIntention::INTENTION_HOME:
            return FocusIntension::HOME;
        default:;
            return FocusIntension::NONE;
    }
}

bool FocusEventHandler::HasCustomKeyEventDispatch(const FocusEvent& event)
{
    if (event.event.eventType != UIInputEventType::KEY) {
        return false;
    }
    const KeyEvent& keyEvent = static_cast<const KeyEvent&>(event.event);
    if (keyEvent.isPreIme) {
        return false;
    }
    return GetOnKeyEventDispatchCallback() != nullptr;
}

bool FocusEventHandler::HandleCustomEventDispatch(const FocusEvent& event)
{
    auto onKeyEventDispatchCallback = GetOnKeyEventDispatchCallback();
    if (onKeyEventDispatchCallback) {
        const KeyEvent& keyEvent = static_cast<const KeyEvent&>(event.event);
        auto info = KeyEventInfo(keyEvent);
        return onKeyEventDispatchCallback(info);
    }
    return false;
}

bool FocusEventHandler::OnFocusEvent(const FocusEvent& event)
{
    if (!IsCurrentFocus()) {
        TAG_LOGI(AceLogTag::ACE_FOCUS,
            "node: %{public}s/%{public}d not current focus, cannot handle key event",
            GetFrameName().c_str(), GetFrameId());
        return false;
    }

    const bool customDispatch = HasCustomKeyEventDispatch(event);
    PrepareSensitiveKeyDispatch(event);
    // A scope without a custom dispatcher visits its focused child before checking its own policy.
    bool result = false;
    if (customDispatch) {
        TAG_LOGI(AceLogTag::ACE_FOCUS, "node: %{public}s/%{public}d hasCustomKeyEventDispatch",
            GetFrameName().c_str(), GetFrameId());
        result = RejectAgentKeyEvent(event) || HandleCustomEventDispatch(event);
    } else if (focusType_ == FocusType::SCOPE) {
        result = OnFocusEventScope(event);
    } else if (focusType_ == FocusType::NODE) {
        result = OnFocusEventNode(event);
    } else {
        TAG_LOGW(AceLogTag::ACE_FOCUS, "Current node focus type: %{public}d is invalid.", focusType_);
    }
    return result;
}

bool FocusEventHandler::OnFocusEventScope(const FocusEvent& event)
{
    ACE_DCHECK(IsCurrentFocus());
    auto lastFocusNode = lastWeakFocusNode_.Upgrade();
    if (lastFocusNode && lastFocusNode->OnFocusEvent(event)) {
        TAG_LOGD(AceLogTag::ACE_FOCUS,
            "OnKeyEvent: Node %{public}s/%{public}d will not handle Event(type:%{private}d). "
            "Because its child %{public}s/%{public}d already has consumed this event.",
            GetFrameName().c_str(), GetFrameId(), event.event.eventType, lastFocusNode->GetFrameName().c_str(),
            lastFocusNode->GetFrameId());
        return true;
    }
    return OnFocusEventNode(event);
}

bool FocusEventHandler::OnFocusEventNode(const FocusEvent& focusEvent)
{
    ACE_DCHECK(IsCurrentFocus());

    bool ret = false;
    if (focusEvent.event.eventType == UIInputEventType::KEY) {
        const KeyEvent& keyEvent = static_cast<const KeyEvent&>(focusEvent.event);
        if (RejectAgentKeyEvent(focusEvent)) {
            return true;
        }
        ret = HandleKeyEventInternal(keyEvent, focusEvent.intension);
    }
    if (focusEvent.event.eventType == UIInputEventType::FOCUS_AXIS) {
        const FocusAxisEvent& focusAxisEvent = static_cast<const FocusAxisEvent&>(focusEvent.event);
        return HandleFocusAxisEvent(focusAxisEvent);
    }
    if (focusEvent.event.eventType == UIInputEventType::CROWN) {
        const CrownEvent& crownEvent = static_cast<const CrownEvent&>(focusEvent.event);
        return HandleCrownEvent(crownEvent);
    }

    auto keyProcessingMode = static_cast<KeyProcessingMode>(GetKeyProcessingMode());
    if (keyProcessingMode == KeyProcessingMode::ANCESTOR_EVENT) {
        return ret;
    }
    return ret ? true : HandleFocusTravel(focusEvent);
}

int32_t FocusEventHandler::GetKeyProcessingMode()
{
    auto frameNode = GetFrameNode();
    CHECK_NULL_RETURN(frameNode, static_cast<int32_t>(KeyProcessingMode::FOCUS_NAVIGATION));
    auto context = frameNode->GetContextRefPtr();
    CHECK_NULL_RETURN(context, static_cast<int32_t>(KeyProcessingMode::FOCUS_NAVIGATION));
    auto focusManager = context->GetOrCreateFocusManager();
    CHECK_NULL_RETURN(focusManager, static_cast<int32_t>(KeyProcessingMode::FOCUS_NAVIGATION));
    return static_cast<int32_t>(focusManager->GetKeyProcessingMode());
}

bool FocusEventHandler::HandleKeyEvent(const KeyEvent& event, FocusIntension intension)
{
    FocusEvent focusEvent(event);
    PrepareSensitiveKeyDispatch(focusEvent);
    return RejectAgentKeyEvent(focusEvent) || HandleKeyEventInternal(event, intension);
}

bool FocusEventHandler::HandleKeyEventInternal(const KeyEvent& event, FocusIntension intension)
{
    // ReDispatch event will not be consumed again.
    if (event.isRedispatch) {
        return false;
    }

    auto node = GetFrameNode();
    CHECK_NULL_RETURN(node, false);
    auto* pipeline = node->GetContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto info = KeyEventInfo(event);
    if (event.HasKey(KeyCode::KEY_META_LEFT) || event.HasKey(KeyCode::KEY_META_RIGHT)) {
        info.SetMetaKey(1);
    }
    if (event.isPreIme) {
        return OnKeyPreIme(info, event);
    }

    bool retInternal = OnKeyEventNodeInternal(event);
    bool retCallback = OnKeyEventNodeUser(info, event);
    if (retInternal || retCallback) {
        return true;
    }
    // Handle on click
    auto appTheme = pipeline->GetTheme<AppTheme>();
    CHECK_NULL_RETURN(appTheme, false);
    if (!pipeline->GetIsFocusActive() && (Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_EIGHTEEN) ||
                                             !appTheme->NeedFocusHandleClick())) {
        return false;
    }
    if (intension == FocusIntension::SELECT && !IsTabStop()) {
        intension = FocusIntension::SPACE;
    }
    auto ret = false;
    if (intension == FocusIntension::SPACE) {
        ret = OnClick(event);
        TAG_LOGI(AceLogTag::ACE_FOCUS,
            "OnClick: %{public}s/%{public}d KeyEvent("
            SEC_PLD(%{private}d) ", %{public}d) ret: %{public}d",
            GetFrameName().c_str(), GetFrameId(), SEC_PARAM(event.code), event.action, ret);
    }
    return ret;
}

bool FocusEventHandler::HandleFocusAxisEvent(const FocusAxisEvent& event)
{
    auto node = GetFrameNode();
    CHECK_NULL_RETURN(node, false);
    auto* pipeline = node->GetContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto onFocusAxisCallback = GetOnFocusAxisCallback();
    CHECK_NULL_RETURN(onFocusAxisCallback, false);
    auto info = FocusAxisEventInfo(event);
    auto eventHub = node->GetEventHub<EventHub>();
    if (eventHub) {
        auto targetImpl = eventHub->CreateGetEventTargetImpl();
        info.SetTarget(targetImpl().value_or(EventTarget()));
    }
    onFocusAxisCallback(info);
    return info.IsStopPropagation();
}

bool FocusEventHandler::HandleCrownEvent(const CrownEvent& CrownEvent)
{
    ACE_DCHECK(IsCurrentFocus());
    bool retCallback = false;
    auto onCrownEventCallback = GetOnCrownCallback();
    if (onCrownEventCallback) {
        CrownEventInfo crownInfo(CrownEvent);
        onCrownEventCallback(crownInfo);
        retCallback = crownInfo.IsStopPropagation();
        TAG_LOGI(AceLogTag::ACE_FOCUS,
            "OnCrownEventUser: %{public}s/%{public}d CrownAction:%{public}d",
            GetFrameName().c_str(), GetFrameId(), CrownEvent.action);
    } else {
        retCallback = ProcessOnCrownEventInternal(CrownEvent);
        TAG_LOGI(AceLogTag::ACE_FOCUS,
            "OnCrownEventInternal: %{public}s/%{public}d CrownAction:%{public}d",
            GetFrameName().c_str(), GetFrameId(), CrownEvent.action);
    }
    return retCallback;
}

bool FocusEventHandler::ProcessOnCrownEventInternal(const CrownEvent& event)
{
    bool result = false;
    auto onCrownEventCallbackInternal = GetOnCrownEventInternal();
    if (onCrownEventCallbackInternal) {
        onCrownEventCallbackInternal(event);
        result = true;
    }
    return result;
}

bool FocusEventHandler::OnKeyPreIme(KeyEventInfo& info, const KeyEvent& keyEvent)
{
    TAG_LOGD(AceLogTag::ACE_FOCUS,
        "node: %{public}s/%{public}d try to handle OnKeyPreIme by "
        "code:%{private}d/action:%{public}d/isPreIme:%{public}d",
        GetFrameName().c_str(), GetFrameId(), keyEvent.code, keyEvent.action, keyEvent.isPreIme);
    auto onKeyPreIme = GetOnKeyPreIme();
    if (onKeyPreIme) {
        bool retPreIme = onKeyPreIme(info);
        auto pipeline = PipelineContext::GetCurrentContextSafelyWithCheck();
        auto eventManager = pipeline->GetEventManager();
        if (eventManager) {
            eventManager->SetIsKeyConsumed(retPreIme);
        }
        TAG_LOGI(AceLogTag::ACE_FOCUS, "node: %{public}s/%{public}d handle OnKeyPreIme",
            GetFrameName().c_str(), GetFrameId());
        return info.IsStopPropagation();
    } else if (GetFrameName() == V2::UI_EXTENSION_COMPONENT_ETS_TAG ||
               GetFrameName() == V2::EMBEDDED_COMPONENT_ETS_TAG ||
               GetFrameName() == V2::DYNAMIC_COMPONENT_ETS_TAG) {
        TAG_LOGI(AceLogTag::ACE_FOCUS, "node: %{public}s/%{public}d try process OnKeyEventInternal",
            GetFrameName().c_str(), GetFrameId());
        return ProcessOnKeyEventInternal(keyEvent);
    } else {
        return false;
    }
}

bool FocusEventHandler::OnClick(const KeyEvent& event)
{
    auto onClickCallback = GetOnClickCallback();
    if (onClickCallback) {
        auto info = GestureEvent();
        info.SetTimeStamp(event.timeStamp);
        auto geometryNode = GetGeometryNode();
        CHECK_NULL_RETURN(geometryNode, false);
        auto rect = geometryNode->GetFrameRect();
        auto centerToWindow = Offset((rect.Left() + rect.Right()) / 2, (rect.Top() + rect.Bottom()) / 2);
        auto centerToNode = Offset((rect.Right() - rect.Left()) / 2, (rect.Bottom() - rect.Top()) / 2);
        info.SetGlobalLocation(centerToWindow);
        info.SetLocalLocation(centerToNode);
        info.SetSourceDevice(event.sourceType);
        info.SetDeviceId(event.deviceId);
        info.SetInputEventType(InputEventType::KEYBOARD);
        auto node = GetFrameNode();
        CHECK_NULL_RETURN(node, false);
        auto pipelineContext = node->GetContextRefPtr();
        if (pipelineContext) {
            auto windowOffset = pipelineContext->GetCurrentWindowRect().GetOffset() + centerToWindow;
            auto globalWindowOffset = pipelineContext->GetGlobalDisplayWindowRect().GetOffset() + centerToWindow;
            info.SetScreenLocation(windowOffset);
            info.SetGlobalDisplayLocation(globalWindowOffset);
        }
        info.SetSourceTool(SourceTool::UNKNOWN);
        info.SetPatternName(node->GetTag().c_str());
        auto eventHub = node->GetEventHub<EventHub>();
        if (eventHub) {
            auto targetImpl = eventHub->CreateGetEventTargetImpl();
            info.SetTarget(targetImpl().value_or(EventTarget()));
        }
        info.SetTargetDisplayId(event.targetDisplayId);
        onClickCallback(info);
        return true;
    }
    return false;
}

bool FocusEventHandler::OnKeyEventNodeInternal(const KeyEvent& keyEvent)
{
    auto node = GetFrameNode();
    CHECK_NULL_RETURN(node, false);
    auto* pipeline = node->GetContext();
    CHECK_NULL_RETURN(pipeline, false);
    bool isBypassInner = keyEvent.IsKey({ KeyCode::KEY_TAB }) && pipeline && pipeline->IsTabJustTriggerOnKeyEvent();
    auto retInternal = false;
    if (isNodeNeedKey_) {
        retInternal =  ProcessOnKeyEventInternal(keyEvent);
        TAG_LOGI(AceLogTag::ACE_FOCUS,
            "OnKeyEventInternal self: %{public}s/%{public}d KeyEvent(%{private}d, %{public}d) "
            "ret: %{public}d",
            GetFrameName().c_str(), GetFrameId(), keyEvent.code, keyEvent.action, retInternal);
        return retInternal;
    }
    if (!isBypassInner && !onKeyEventsInternal_.empty()) {
        retInternal = ProcessOnKeyEventInternal(keyEvent);
        TAG_LOGI(AceLogTag::ACE_FOCUS,
            "OnKeyEventInternal self: %{public}s/%{public}d "
            "KeyEvent(" SEC_PLD(%{private}d) ", %{public}d) "
            "ret: %{public}d",
            GetFrameName().c_str(), GetFrameId(), SEC_PARAM(keyEvent.code), keyEvent.action, retInternal);
    }
    return retInternal;
}

bool FocusEventHandler::OnKeyEventNodeUser(KeyEventInfo& info, const KeyEvent& keyEvent)
{
    auto node = GetFrameNode();
    CHECK_NULL_RETURN(node, false);
    auto* pipeline = node->GetContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto retCallback = false;
    auto onKeyEventCallback = GetOnKeyCallback();
    if (onKeyEventCallback) {
        auto result = onKeyEventCallback(info);
        retCallback = info.IsStopPropagation() || result;
        auto eventManager = pipeline->GetEventManager();
        PrintOnKeyEventUserInfo(keyEvent, retCallback);
    }

    auto onJSFrameNodeKeyCallback = GetJSFrameNodeOnKeyCallback();
    if (onJSFrameNodeKeyCallback) {
        onJSFrameNodeKeyCallback(info);
        retCallback = info.IsStopPropagation();
        PrintOnKeyEventUserInfo(keyEvent, retCallback);
    }
    return retCallback;
}

void FocusEventHandler::PrintOnKeyEventUserInfo(const KeyEvent& keyEvent, bool retCallback)
{
    TAG_LOGI(AceLogTag::ACE_FOCUS,
        "OnKeyEventUser: %{public}s/%{public}d "
        "KeyEvent(" SEC_PLD(%{private}d) ", %{public}d) ret: %{public}d",
        GetFrameName().c_str(), GetFrameId(), SEC_PARAM(keyEvent.code), keyEvent.action, retCallback);
}

bool FocusEventHandler::ProcessOnKeyEventInternal(const KeyEvent& event)
{
    bool result = false;
    for (const auto& onKeyEvent : onKeyEventsInternal_) {
        auto callback = onKeyEvent.second;
        if (callback && callback(event)) {
            result = true;
        }
    }
    return result;
}
} // namespace OHOS::Ace::NG
