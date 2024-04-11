/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "accessibility_property.h"

#include "core/components_ng/base/frame_node.h"

namespace OHOS::Ace::NG {
constexpr uint64_t ACTIONS = std::numeric_limits<uint64_t>::max();
std::unordered_set<AceAction> AccessibilityProperty::GetSupportAction() const
{
    static const AceAction allActions[] = {
        AceAction::ACTION_NONE,
        AceAction::GLOBAL_ACTION_BACK,
        AceAction::CUSTOM_ACTION,
        AceAction::ACTION_CLICK,
        AceAction::ACTION_LONG_CLICK,
        AceAction::ACTION_SCROLL_FORWARD,
        AceAction::ACTION_SCROLL_BACKWARD,
        AceAction::ACTION_FOCUS,
        AceAction::ACTION_CLEAR_FOCUS,
        AceAction::ACTION_ACCESSIBILITY_FOCUS,
        AceAction::ACTION_CLEAR_ACCESSIBILITY_FOCUS,
        AceAction::ACTION_NEXT_AT_MOVEMENT_GRANULARITY,
        AceAction::ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY,
        AceAction::ACTION_SET_TEXT,
        AceAction::ACTION_COPY,
        AceAction::ACTION_PASTE,
        AceAction::ACTION_CUT,
        AceAction::ACTION_SELECT,
        AceAction::ACTION_SET_SELECTION,
        AceAction::ACTION_CLEAR_SELECTION,
    };

    std::unordered_set<AceAction> supportActions;
    if (supportActions_ == 0) {
        return supportActions;
    }

    auto finalSupportActions = supportActions_ & ACTIONS;
    for (auto action : allActions) {
        if ((finalSupportActions & (1UL << static_cast<uint32_t>(action))) != 0) {
            supportActions.emplace(action);
        }
    }
    return supportActions;
}

std::string AccessibilityProperty::GetText() const
{
    return propText_.value_or("");
}

std::string AccessibilityProperty::GetGroupText(bool forceGetChildren) const
{
    std::string text;
    GetGroupTextRecursive(forceGetChildren, text);
    return text;
}

void AccessibilityProperty::GetGroupTextRecursive(bool forceGetChildren, std::string& text) const
{
    auto node = host_.Upgrade();
    CHECK_NULL_VOID(node);
    if (node->IsInternal()) {
        return;
    }
    auto level = GetAccessibilityLevel();
    if (level == Level::AUTO || level == Level::YES) {
        auto nodeText = GetText();
        if (!text.empty() && !nodeText.empty()) {
            text += ", ";
        }
        text += nodeText;
    } else if (level == Level::NO_HIDE_DESCENDANTS) {
        return;
    }
    // Do not change text if level is no

    if (!(forceGetChildren || IsAccessibilityGroup())) {
        return;
    }
    auto& children = node->GetFrameChildren();
    for (auto& childWeak : children) {
        auto child = childWeak.Upgrade();
        if (child == nullptr) {
            continue;
        }
        child->GetAccessibilityProperty<AccessibilityProperty>()->GetGroupTextRecursive(true, text);
    }
}

std::string AccessibilityProperty::GetAccessibilityText() const
{
    return accessibilityText_.value_or("");
}

AccessibilityHoverTestPath AccessibilityProperty::HoverTest(
    const PointF& point,
    const RefPtr<FrameNode>& root,
    std::unique_ptr<HoverTestDebugTraceInfo>& debugInfo)
{
    AccessibilityHoverTestPath path;
    CHECK_NULL_RETURN(root, path);
    ACE_SCOPED_TRACE("AccessibilityHoverTest");
    AccessibilityProperty::HoverTestRecursive(point, root, path, debugInfo);
    return path;
}

std::unique_ptr<JsonValue> AccessibilityProperty::CreateNodeSearchInfo(const RefPtr<FrameNode>& node,
    const PointF& parentPoint)
{
    auto nodeInfo = JsonUtil::Create();
    nodeInfo->Put("id", node->GetAccessibilityId());
    nodeInfo->Put("tag", node->GetTag().c_str());
    if (!node->IsRootNode()) {
        nodeInfo->Put("parent", node->GetParent()->GetAccessibilityId());
    }
    nodeInfo->Put("visible", node->IsVisible());
    auto [shouldSearchSelf, shouldSearchChildren] = AccessibilityProperty::GetSearchStrategy(node);
    nodeInfo->Put("shouldSearchSelf", shouldSearchSelf);
    nodeInfo->Put("shouldSearchChildren", shouldSearchChildren);

    auto renderContext = node->GetRenderContext();
    auto rect = renderContext->GetPaintRectWithoutTransform();
    PointF selfPoint = parentPoint;
    renderContext->GetPointWithRevert(selfPoint);
    bool hitSelf = rect.IsInnerRegion(selfPoint);
    nodeInfo->Put("hitNode", hitSelf);
    nodeInfo->Put("rect", rect.ToString().c_str());
    nodeInfo->Put("hoverPoint", selfPoint.ToString().c_str());
    nodeInfo->Put("clip", renderContext->GetClipEdge().value_or(false));

    auto eventHub = node->GetEventHub<EventHub>();
    nodeInfo->Put("enabled", eventHub->IsEnabled());

    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    if (accessibilityProperty != nullptr) {
        nodeInfo->Put("accessibilityLevel", accessibilityProperty->GetAccessibilityLevel().c_str());
        nodeInfo->Put("accessibilityGroup", accessibilityProperty->IsAccessibilityGroup());
        nodeInfo->Put("hasVirtualNode", accessibilityProperty->HasAccessibilityVirtualNode());
        nodeInfo->Put("accessibilityText", accessibilityProperty->GetAccessibilityText().c_str());
        nodeInfo->Put("accessibilityDescription", accessibilityProperty->GetAccessibilityDescription().c_str());
    }

    std::string testModeStr = "";
    switch (node->GetHitTestMode()) {
        case HitTestMode::HTMDEFAULT:
            testModeStr = "Default";
            break;
        case HitTestMode::HTMBLOCK:
            testModeStr = "Block";
            break;
        case HitTestMode::HTMTRANSPARENT:
            testModeStr = "Transparent";
            break;
        case HitTestMode::HTMNONE:
            testModeStr = "None";
            break;
        default:
            testModeStr = "Unsupported";
    }
    nodeInfo->Put("hitTestMode", testModeStr.c_str());
    return nodeInfo;
}

bool AccessibilityProperty::HoverTestRecursive(
    const PointF& parentPoint,
    const RefPtr<FrameNode>& node,
    AccessibilityHoverTestPath& path,
    std::unique_ptr<HoverTestDebugTraceInfo>& debugInfo)
{
    if (!node->IsActive() || node->IsInternal() || node->GetBypass()) {
        return false;
    }
    if (debugInfo != nullptr) {
        auto nodeInfo = CreateNodeSearchInfo(node, parentPoint);
        debugInfo->trace.push_back(std::move(nodeInfo));
    }
    bool hitTarget = false;
    if (!node->IsVisible()) {
        return false;
    }

    auto [shouldSearchSelf, shouldSearchChildren] = AccessibilityProperty::GetSearchStrategy(node);

    auto renderContext = node->GetRenderContext();
    auto rect = renderContext->GetPaintRectWithoutTransform();
    PointF selfPoint = parentPoint;
    renderContext->GetPointWithRevert(selfPoint);
    bool hitSelf = rect.IsInnerRegion(selfPoint);
    if (hitSelf && shouldSearchSelf) {
        hitTarget = true;
        path.push_back(node);
    }
    bool hasClip = renderContext->GetClipEdge().value_or(false);
    if (hasClip && !hitSelf) {
        return false;
    }

    if (shouldSearchChildren) {
        PointF noOffsetPoint = selfPoint - rect.GetOffset();
        auto children = node->GetFrameChildren();
        for (auto childWeak = children.rbegin(); childWeak != children.rend();  ++childWeak) {
            auto child = childWeak->Upgrade();
            if (child == nullptr) {
                continue;
            }
            if (AccessibilityProperty::HoverTestRecursive(noOffsetPoint, child, path, debugInfo)) {
                return true;
            }
        }
    }
    return hitTarget;
}

void UpdateSearchStrategyByHitTestMode(HitTestMode hitTestMode, bool& shouldSearchSelf, bool& shouldSearchChildren)
{
    switch (hitTestMode) {
        case HitTestMode::HTMBLOCK:
            shouldSearchChildren = false;
            break;
        case HitTestMode::HTMTRANSPARENT:
            shouldSearchSelf = false;
            shouldSearchChildren = false;
            break;
        case HitTestMode::HTMNONE:
            shouldSearchSelf = false;
            break;
        default:
            break;
    }
}

std::pair<bool, bool> AccessibilityProperty::GetSearchStrategy(const RefPtr<FrameNode>& node)
{
    bool shouldSearchSelf = true;
    bool shouldSearchChildren = true;
    do {
        auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
        if (accessibilityProperty != nullptr) {
            auto level = accessibilityProperty->GetAccessibilityLevel();
            bool hasGroupOrVirtualNode = accessibilityProperty->IsAccessibilityGroup() ||
                accessibilityProperty->HasAccessibilityVirtualNode();
            bool hasAccessibilityText = accessibilityProperty->HasAccessibilityTextOrDescription();
            if (level == AccessibilityProperty::Level::YES) {
                if (hasGroupOrVirtualNode) {
                    shouldSearchChildren = false;
                }
                break;
            } else if (level == AccessibilityProperty::Level::NO_HIDE_DESCENDANTS) {
                shouldSearchSelf = false;
                shouldSearchChildren = false;
                break;
            } else {
                if (level == AccessibilityProperty::Level::NO) {
                    shouldSearchSelf = false;
                } else {
                    // shouldSearchSelf is true here
                    if (hasGroupOrVirtualNode) {
                        shouldSearchChildren = false;
                        break;
                    }
                    if (hasAccessibilityText) {
                        break;
                    }
                }
            }
        }
        auto eventHub = node->GetEventHub<EventHub>();
        if (!eventHub->IsEnabled()) {
            shouldSearchChildren = false;
            // Fall through to update `shouldSearchSelf`
        }
        HitTestMode hitTestMode = node->GetHitTestMode();
        UpdateSearchStrategyByHitTestMode(hitTestMode, shouldSearchSelf, shouldSearchChildren);
    } while (0);
    return std::make_pair(shouldSearchSelf, shouldSearchChildren);
}

static const std::set<std::string> TAGS_FOCUSABLE = {
    V2::CHECKBOX_ETS_TAG,
    V2::CHECKBOXGROUP_ETS_TAG,
    V2::GAUGE_ETS_TAG,
    V2::MARQUEE_ETS_TAG,
    V2::MENU_ETS_TAG,
    V2::MENU_ITEM_ETS_TAG,
    V2::MENU_ITEM_GROUP_ETS_TAG,
    V2::NAVIGATION_VIEW_ETS_TAG,
    V2::DATE_PICKER_ETS_TAG,
    V2::PROGRESS_ETS_TAG,
    V2::RADIO_ETS_TAG,
    V2::RATING_ETS_TAG,
    V2::SCROLL_BAR_ETS_TAG,
    V2::SELECT_ETS_TAG,
    V2::SLIDER_ETS_TAG,
    V2::STEPPER_ETS_TAG,
    V2::TEXT_ETS_TAG,
    V2::TEXTCLOCK_ETS_TAG,
    V2::TEXT_PICKER_ETS_TAG,
    V2::TEXTTIMER_ETS_TAG,
    V2::TIME_PICKER_ETS_TAG,
    V2::TOGGLE_ETS_TAG,
    V2::WEB_ETS_TAG,
    V2::XCOMPONENT_ETS_TAG,
    V2::UI_EXTENSION_COMPONENT_ETS_TAG,
    V2::EMBEDDED_COMPONENT_ETS_TAG,
    V2::FORM_ETS_TAG
};

bool AccessibilityProperty::IsAccessibilityFocusableDebug(const RefPtr<FrameNode>& node,
    std::unique_ptr<JsonValue>& info)
{
    bool focusable = IsAccessibilityFocusable(node);
    info->Put("id", node->GetAccessibilityId());
    info->Put("tag", node->GetTag().c_str());
    if (!node->IsRootNode()) {
        info->Put("parent", node->GetParent()->GetAccessibilityId());
    }
    info->Put("selected", focusable);

    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    if (accessibilityProperty != nullptr) {
        info->Put("accessibilityLevel", accessibilityProperty->GetAccessibilityLevel().c_str());
        info->Put("accessibilityGroup", accessibilityProperty->IsAccessibilityGroup());
        info->Put("hasVirtualNode", accessibilityProperty->HasAccessibilityVirtualNode());
        info->Put("accessibilityText", accessibilityProperty->GetAccessibilityText().c_str());
        info->Put("accessibilityDescription", accessibilityProperty->GetAccessibilityDescription().c_str());
        info->Put("text", accessibilityProperty->GetText().c_str());
        info->Put("hasAction", accessibilityProperty->HasAction());
    }

    auto eventHub = node->GetEventHub<EventHub>();
    info->Put("enabled", eventHub->IsEnabled());
    auto gestureEventHub = eventHub->GetGestureEventHub();
    if (gestureEventHub != nullptr) {
        info->Put("clickable", gestureEventHub->IsAccessibilityClickable());
        info->Put("longClickable", gestureEventHub->IsAccessibilityLongClickable());
    }
    return focusable;
}


bool AccessibilityProperty::IsAccessibilityFocusable(const RefPtr<FrameNode>& node)
{
    if (node->IsRootNode()) {
        return false;
    }
    bool focusable = false;
    do {
        auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
        if (accessibilityProperty != nullptr) {
            auto level = accessibilityProperty->GetAccessibilityLevel();
            if (level == AccessibilityProperty::Level::YES) {
                focusable = true;
                break;
            }
            if (level == AccessibilityProperty::Level::NO) {
                break;
            }
            if (accessibilityProperty->IsAccessibilityGroup() ||
                accessibilityProperty->HasAccessibilityVirtualNode() ||
                accessibilityProperty->HasAction() ||
                accessibilityProperty->HasAccessibilityTextOrDescription() ||
                !accessibilityProperty->GetText().empty()) {
                focusable = true;
                break;
            }
        }

        auto eventHub = node->GetEventHub<EventHub>();
        if (!eventHub->IsEnabled()) {
            focusable = true;
            break;
        }
        auto gestureEventHub = eventHub->GetGestureEventHub();
        if (gestureEventHub != nullptr) {
            if (gestureEventHub->IsAccessibilityClickable() ||
                gestureEventHub->IsAccessibilityLongClickable()) {
                focusable = true;
                break;
            }
        }

        if (TAGS_FOCUSABLE.find(node->GetTag()) != TAGS_FOCUSABLE.end()) {
            focusable = true;
            break;
        }
    } while (0);
    return focusable;
}

bool AccessibilityProperty::HasAccessibilityTextOrDescription() const
{
    return !accessibilityText_.value_or("").empty() ||
        !accessibilityDescription_.value_or("").empty();
}

bool AccessibilityProperty::HasAction() const
{
    return (supportActions_ & ACTIONS) != 0 ||
        IsCheckable() ||
        IsScrollable() ||
        IsEditable() ||
        IsDeletable();
}
} // namespace OHOS::Ace::NG
