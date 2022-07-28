/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "core/common/event_manager.h"

#include "base/log/ace_trace.h"
#include "base/utils/utils.h"
#include "core/event/key_event.h"
#include "core/gestures/gesture_referee.h"
#include "core/pipeline/base/element.h"
#include "core/pipeline/base/render_node.h"

namespace OHOS::Ace {

void EventManager::TouchTest(const TouchEvent& touchPoint, const RefPtr<RenderNode>& renderNode,
    const TouchRestrict& touchRestrict, const Offset& offset, float viewScale, bool needAppend)
{
    ContainerScope scope(instanceId_);

    ACE_FUNCTION_TRACE();
    if (!renderNode) {
        LOGW("renderNode is null.");
        return;
    }
    // first clean.
    GestureReferee::GetInstance().CleanGestureScope(touchPoint.id);
    // collect
    TouchTestResult hitTestResult;
    const Point point { touchPoint.x, touchPoint.y, touchPoint.sourceType };
    // For root node, the parent local point is the same as global point.
    renderNode->TouchTest(point, point, touchRestrict, hitTestResult);
    if (needAppend) {
#ifdef OHOS_STANDARD_SYSTEM
        for (auto entry = hitTestResult.begin(); entry != hitTestResult.end(); ++entry) {
            if ((*entry)) {
                (*entry)->SetSubPipelineGlobalOffset(offset, viewScale);
            }
        }
#endif
        TouchTestResult prevHitTestResult = touchTestResults_[touchPoint.id];
        hitTestResult.splice(hitTestResult.end(), prevHitTestResult);
    }
    touchTestResults_[touchPoint.id] = std::move(hitTestResult);
}

void EventManager::HandleGlobalEvent(const TouchEvent& touchPoint, const RefPtr<TextOverlayManager>& textOverlayManager)
{
    if (touchPoint.type != TouchType::DOWN) {
        return;
    }
    const Point point { touchPoint.x, touchPoint.y, touchPoint.sourceType };
    if (!textOverlayManager) {
        return;
    }
    auto textOverlayBase = textOverlayManager->GetTextOverlayBase();
    if (!textOverlayBase) {
        return;
    }
    auto targetNode = textOverlayManager->GetTargetNode();
    if (!targetNode) {
        return;
    }
    for (auto& rect : textOverlayManager->GetTextOverlayRect()) {
        if (rect.IsInRegion(point)) {
            inSelectedRect_ = true;
        }
    }
    for (auto& rect : textOverlayBase->GetSelectedRect()) {
        if (rect.IsInRegion(point)) {
            inSelectedRect_ = true;
        }
    }
    if (!inSelectedRect_) {
        textOverlayManager->PopTextOverlay();
        textOverlayBase->ChangeSelection(0, 0);
        textOverlayBase->MarkIsOverlayShowed(false);
        targetNode->MarkNeedRender();
    }
    inSelectedRect_ = false;
}

void EventManager::TouchTest(
    const AxisEvent& event, const RefPtr<RenderNode>& renderNode, const TouchRestrict& touchRestrict)
{
    ContainerScope scope(instanceId_);

    ACE_FUNCTION_TRACE();
    if (!renderNode) {
        LOGW("renderNode is null.");
        return;
    }
    // collect
    const Point point { event.x, event.y, event.sourceType };
    // For root node, the parent local point is the same as global point.
    renderNode->TouchTest(point, point, touchRestrict, axisTouchTestResult_);
}

bool EventManager::DispatchTouchEvent(const TouchEvent& point)
{
    ContainerScope scope(instanceId_);

    ACE_FUNCTION_TRACE();
    const auto iter = touchTestResults_.find(point.id);
    if (iter != touchTestResults_.end()) {
        bool dispatchSuccess = true;
        for (auto entry = iter->second.rbegin(); entry != iter->second.rend(); ++entry) {
            if (!(*entry)->DispatchMultiContainerEvent(point)) {
                dispatchSuccess = false;
                break;
            }
        }
        // If one gesture recognizer has already been won, other gesture recognizers will still be affected by
        // the event, each recognizer needs to filter the extra events by itself.
        if (dispatchSuccess) {
            for (const auto& entry : iter->second) {
                if (!entry->HandleMultiContainerEvent(point)) {
                    break;
                }
            }
        }

        if (point.type == TouchType::UP || point.type == TouchType::CANCEL) {
            GestureReferee::GetInstance().CleanGestureScope(point.id);
            touchTestResults_.erase(point.id);
        }

        return true;
    }
    LOGI("the %{public}d touch test result does not exist!", point.id);
    return false;
}

bool EventManager::DispatchTouchEvent(const AxisEvent& event)
{
    ContainerScope scope(instanceId_);

    ACE_FUNCTION_TRACE();
    for (const auto& entry : axisTouchTestResult_) {
        if (!entry->HandleEvent(event)) {
            break;
        }
    }
    if (event.action == AxisAction::END || event.action == AxisAction::NONE) {
        axisTouchTestResult_.clear();
    }
    return true;
}

void EventManager::CollectTabIndexNodes(const RefPtr<FocusNode>& rootNode)
{
    if (!rootNode) {
        LOGE("param is null.");
        return;
    }
    RefPtr<FocusGroup> rootScope = AceType::DynamicCast<FocusGroup>(rootNode);
    if (rootScope) {
        auto children = rootScope->GetChildrenList();
        if (children.size() == 1 && !AceType::DynamicCast<FocusGroup>(children.front())) {
            if (rootScope->GetTabIndex() > 0) {
                tabIndexNodes_.emplace_back(rootScope->GetTabIndex(), WeakClaim(AceType::RawPtr(rootScope)));
            }
            return;
        }
        for (auto& child : children) {
            CollectTabIndexNodes(child);
        }
    }
    if (rootNode->GetTabIndex() > 0) {
        tabIndexNodes_.emplace_back(rootNode->GetTabIndex(), WeakClaim(AceType::RawPtr(rootNode)));
    }
}

bool EventManager::DispatchKeyEvent(const KeyEvent& event, const RefPtr<FocusNode>& focusNode)
{
    if (!focusNode) {
        LOGW("focusNode is null.");
        return false;
    }
    LOGD("The key code is %{public}d, the key action is %{public}d, the repeat time is %{public}d.", event.code,
        event.action, event.repeatTime);
    int32_t tabStep = 0;
    if (event.code == KeyCode::KEY_TAB && event.action == KeyAction::DOWN) {
        tabIndexNodes_.clear();
        CollectTabIndexNodes(focusNode);
        tabIndexNodes_.sort(
            [](std::pair<int32_t, WeakPtr<FocusNode>>& a, std::pair<int32_t, WeakPtr<FocusNode>>& b) {
                return a.first < b.first;
            }
        );
        if (!isTabNodesCollected_) {
            isTabNodesCollected_ = true;
            tabPressedIndex_ = -1;
        }
        if (event.IsKey({ KeyCode::KEY_SHIFT_LEFT, KeyCode::KEY_TAB }) ||
                event.IsKey({ KeyCode::KEY_SHIFT_RIGHT, KeyCode::KEY_TAB })) {
            LOGI("RequestNextFocus by 'SHIFT-TAB'");
            tabStep = -1;
            --tabPressedIndex_;
        } else {
            LOGI("RequestNextFocus by 'TAB'");
            tabStep = 1;
            ++tabPressedIndex_;
        }
        if (0 <= tabPressedIndex_ && tabPressedIndex_ < static_cast<int32_t>(tabIndexNodes_.size())) {
            auto iter = tabIndexNodes_.begin();
            std::advance(iter, tabPressedIndex_);
            if (iter == tabIndexNodes_.end()) {
                LOGE("Tab node index is out of boundary");
                return false;
            }
            auto nodeNeedToFocus = (*iter).second.Upgrade();
            if (!nodeNeedToFocus) {
                LOGE("Tab focus node is null");
                return false;
            }
            nodeNeedToFocus->RequestFocusImmediately();
            isLastInTabNodes_ = true;
            return true;
        }
    }
    if (!focusNode->HandleKeyEvent(event)) {
        LOGD("use platform to handle this event");
        tabPressedIndex_ -= tabStep;
        return false;
    }
    return true;
}

void EventManager::MouseTest(const MouseEvent& event, const RefPtr<RenderNode>& renderNode)
{
    if (!renderNode) {
        LOGW("renderNode is null.");
        return;
    }
    const Point point { event.x, event.y };
    MouseHoverTestList hitTestResult;
    WeakPtr<RenderNode> hoverNode = nullptr;
    renderNode->MouseDetect(point, point, hitTestResult, hoverNode);
    if (hitTestResult.empty()) {
        LOGD("mouse hover test result is empty");
    }
    mouseHoverTestResultsPre_ = std::move(mouseHoverTestResults_);
    mouseHoverTestResults_ = std::move(hitTestResult);
    mouseHoverNodePre_ = mouseHoverNode_;
    mouseHoverNode_ = hoverNode;
    LOGI("MouseDetect hit test last/new result size = %{public}zu/%{public}zu", mouseHoverTestResultsPre_.size(),
        mouseHoverTestResults_.size());
}

bool EventManager::DispatchMouseEvent(const MouseEvent& event)
{
    if (event.action == MouseAction::PRESS || event.action == MouseAction::RELEASE ||
        event.action == MouseAction::MOVE) {
        LOGD("RenderBox::HandleMouseEvent, button is %{public}d, action is %{public}d", event.button, event.action);
        for (const auto& wp : mouseHoverTestResults_) {
            auto hoverNode = wp.Upgrade();
            if (hoverNode) {
                if (hoverNode->HandleMouseEvent(event)) {
                    LOGI("Do HandleMouseEvent. Dispatch node: %{public}s", AceType::TypeName(hoverNode));
                    break;
                }
            }
        }
        return true;
    }
    return false;
}

bool EventManager::DispatchMouseHoverEvent(const MouseEvent& event)
{
    auto hoverNodeCur = mouseHoverNode_.Upgrade();
    auto hoverNodePre = mouseHoverNodePre_.Upgrade();
    if (event.action == MouseAction::PRESS) {
        if (hoverNodeCur) {
            hoverNodeCur->AnimateMouseHoverExit();
            hoverNodeCur->OnMouseClickDownAnimation();
        }
    } else if (event.action == MouseAction::RELEASE) {
        if (hoverNodeCur) {
            hoverNodeCur->OnMouseClickUpAnimation();
            hoverNodeCur->AnimateMouseHoverEnter();
        }
    } else if (event.button != MouseButton::NONE_BUTTON && event.action == MouseAction::MOVE) {
        if (hoverNodeCur != hoverNodePre) {
            if (hoverNodePre) {
                hoverNodePre->OnMouseClickUpAnimation();
            }
            if (hoverNodeCur) {
                hoverNodeCur->OnMouseClickDownAnimation();
            }
        }
    } else if (event.button == MouseButton::NONE_BUTTON && event.action == MouseAction::MOVE) {
        if (hoverNodeCur != hoverNodePre) {
            if (hoverNodeCur) {
                hoverNodeCur->AnimateMouseHoverEnter();
            }
            if (hoverNodePre) {
                hoverNodePre->AnimateMouseHoverExit();
            }
        }
    }
    for (const auto& wp : mouseHoverTestResultsPre_) {
        // get all previous hover nodes while it's not in current hover nodes. Those nodes exit hoverd
        auto it = std::find(mouseHoverTestResults_.begin(), mouseHoverTestResults_.end(), wp);
        if (it == mouseHoverTestResults_.end()) {
            auto hoverNode = wp.Upgrade();
            if (hoverNode) {
                hoverNode->HandleMouseHoverEvent(MouseState::NONE);
            }
        }
    }
    for (const auto& wp : mouseHoverTestResults_) {
        // get all current hover nodes while it's not in previous hover nodes. Thoes nodes are new hoverd
        auto it = std::find(mouseHoverTestResultsPre_.begin(), mouseHoverTestResultsPre_.end(), wp);
        if (it == mouseHoverTestResultsPre_.end()) {
            auto hoverNode = wp.Upgrade();
            if (hoverNode) {
                hoverNode->HandleMouseHoverEvent(MouseState::HOVER);
            }
        }
    }
    return true;
}

void EventManager::AxisTest(const AxisEvent& event, const RefPtr<RenderNode>& renderNode)
{
    if (!renderNode) {
        LOGW("renderNode is null.");
        return;
    }
    const Point point { event.x, event.y };
    WeakPtr<RenderNode> axisNode = nullptr;
    renderNode->AxisDetect(point, point, axisNode, event.GetDirection());
    axisNode_ = axisNode;
    LOGI("Current axis node is %{public}s", AceType::TypeName(axisNode_.Upgrade()));
}

bool EventManager::DispatchAxisEvent(const AxisEvent& event)
{
    auto responseNode = axisNode_.Upgrade();
    if (responseNode) {
        responseNode->HandleAxisEvent(event);
    }
    return true;
}

bool EventManager::DispatchRotationEvent(
    const RotationEvent& event, const RefPtr<RenderNode>& renderNode, const RefPtr<RenderNode>& requestFocusNode)
{
    if (!renderNode) {
        LOGW("renderNode is null.");
        return false;
    }

    if (requestFocusNode && renderNode->RotationMatchTest(requestFocusNode)) {
        LOGD("RotationMatchTest: dispatch rotation to request node.");
        return requestFocusNode->RotationTestForward(event);
    } else {
        LOGD("RotationMatchTest: dispatch rotation to statck render node.");
        return renderNode->RotationTest(event);
    }
}

void EventManager::ClearResults()
{
    touchTestResults_.clear();
    mouseTestResults_.clear();
}

} // namespace OHOS::Ace
