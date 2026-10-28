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

#include "js_accessibility_manager.h"

#include "accessibility_system_ability_client.h"
#include "js_third_provider_interaction_operation.h"
#include "perf_monitor_adapter.h"

#include "adapter/ohos/osal/accessibility/accessibility_hidumper_osal.h"
#include "adapter/ohos/osal/accessibility/hover/accessibility_hover_virtual_node_utils.h"
#include "core/components_ng/manager/safe_area/safe_area_manager.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_strategy.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_frame_node_utils.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_move_osal.h"
#include "adapter/ohos/entrance/ace_container.h"
#include "core/components_ng/base/observer_handler.h"
#include "base/log/event_report.h"
#include "core/accessibility/accessibility_manager_ng.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_manager.h"
#include "core/components_ng/pattern/web/web_accessibility_child_tree_callback.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/cmp_accessibility_registry.h"
#include "core/pipeline/base/element_register.h"
#include "frameworks/core/accessibility/hidumper/accessibility_hidumper.h"
#include "frameworks/core/accessibility/node_utils/accessibility_frame_node_utils.h"
#include "frameworks/core/accessibility/utils/accessibility_action_function_utils.h"
#include "frameworks/core/accessibility/utils/accessibility_property_utils.h"
#include "frameworks/core/components_ng/pattern/ui_extension/platform_container_handler.h"
#include "frameworks/core/components_ng/property/accessibility_property.h"

#include "frameworks/core/components_ng/pattern/overlay/accessibility_focus_paint_node_pattern.h"
#include "frameworks/core/components_ng/pattern/web/transitional_node_info.h"
#include "frameworks/core/components_ng/pattern/web/web_pattern.h"
#include "core/components_ng/pattern/stage/stage_manager.h"

using namespace OHOS::Accessibility;
using namespace OHOS::AccessibilityConfig;

#define CHECK_NULL_VOID_WITH_ACTION(ptr, ...) \
    do {                                        \
        if (!(ptr)) {                           \
            __VA_ARGS__;                        \
            return;                             \
        }                                       \
    } while (0)

namespace OHOS::Ace::Framework {
namespace {
const char DUMP_ORDER[] = "-accessibility";
const char DUMP_INSPECTOR[] = "-inspector";
const char ACCESSIBILITY_FOCUSED_EVENT[] = "accessibilityfocus";
const char ACCESSIBILITY_CLEAR_FOCUS_EVENT[] = "accessibilityclearfocus";
const char TEXT_CHANGE_EVENT[] = "textchange";
const char PAGE_CHANGE_EVENT[] = "pagechange";
const char SCROLL_END_EVENT[] = "scrollend";
const char SCROLL_START_EVENT[] = "scrollstart";
const char MOUSE_HOVER_ENTER[] = "mousehoverenter";
const char MOUSE_HOVER_EXIT[] = "mousehoverexit";
const char LIST_TAG[] = "List";
const char STRING_DIR_FORWARD[] = "forward";
const char STRING_DIR_BACKWARD[] = "backward";
constexpr int32_t INVALID_PARENT_ID = -2100000;
constexpr int32_t DEFAULT_PARENT_ID = 2100000;
constexpr int32_t ROOT_STACK_BASE = 1100000;
constexpr int32_t ROOT_DECOR_BASE = 3100000;
constexpr int32_t CARD_NODE_ID_RATION = 10000;
constexpr int32_t CARD_ROOT_NODE_ID_RATION = 1000;
constexpr int32_t CARD_BASE = 100000;
constexpr int32_t DELAY_SEND_EVENT_MILLISECOND = 20;
constexpr uint32_t SUB_TREE_OFFSET_IN_PAGE_ID = 16;
constexpr int32_t MAX_PAGE_ID_WITH_SUB_TREE = (1 << SUB_TREE_OFFSET_IN_PAGE_ID);
constexpr size_t MIN_PARAMS_SIZE = 2;
constexpr int32_t MIN_NUM = 2;
constexpr int32_t MAX_CANDIDATE_ID_NUMBER = 3;
constexpr int64_t INVALID_NODE_ID = -1;
constexpr int32_t ACCESSIBILITY_FOCUS_WITHOUT_EVENT = -2100001;
constexpr uint64_t WEB_MAX_ELEMENT_ID = 0xFFFFFFFFFF;
constexpr int32_t WEB_TREE_MODE = 8;
constexpr uint32_t WEB_FOCUS_MOVE_SEARCH_MAX_DEPTH = 8;
constexpr size_t MAX_DUMP_INFO_SIZE = 5000;
constexpr char DETACH_FOCUS_LOG_PREFIX[] = "[DetachFocusFallback]";
constexpr char WEB_ACC_ID_PARAM[] = "-webAccId";
constexpr char WEB_ACC_FUN_PARAM[] = "-webAccFun";
constexpr char WEB_ACC_FUN_GET_ACC[] = "getAcc";
constexpr char WEB_ACC_FUN_ON_ACC[] = "onAcc";
constexpr char WEB_ACC_FUN_OFF_ACC[] = "offAcc";
constexpr char WEB_ACC_FUN_TREE[] = "tree";
constexpr char WEB_ACC_FUN_NODE[] = "node";
constexpr char WEB_ACC_FUN_PRE[] = "pre";
constexpr char WEB_ACC_FUN_NEXT[] = "next";
constexpr char WEB_ACC_FUN_EXE_ACTION[] = "exeAction";
constexpr char WEB_ACC_FUN_SEND_EVENT[] = "sendEvent";
constexpr char WEB_ACC_FUN_HOVER[] = "hover";
constexpr char WEB_ACC_FUN_ACC_JSON[] = "accJson";
const std::string CANDIDATE_IDS = "candidateIds";
const std::string ACTION_ARGU_SCROLL_STUB = "scrolltype"; // wait for change
const std::string ACTION_DEFAULT_PARAM = "ACCESSIBILITY_ACTION_INVALID";
const std::string ACTION_ARGU_FOCUS_TYPE_STUB = "accessibilityFocusScene";
const std::string ACTION_ARGU_FOCUS_MOVE_VALUE =
    std::to_string(static_cast<int32_t>(AccessibilityFocusActionType::SWIPE_FOCUS));

const std::set<std::string> TAGS_EMBED_COMPONENT = {
    "embeddedObject",
};

std::map<std::string, WebAccFun> webAccFunMap = { { WEB_ACC_FUN_GET_ACC, WEB_GET_ACC },
    { WEB_ACC_FUN_ON_ACC, WEB_ON_ACC }, { WEB_ACC_FUN_OFF_ACC, WEB_OFF_ACC }, { WEB_ACC_FUN_TREE, WEB_TREE },
    { WEB_ACC_FUN_NODE, WEB_NODE }, { WEB_ACC_FUN_PRE, WEB_PRE }, { WEB_ACC_FUN_NEXT, WEB_NEXT },
    { WEB_ACC_FUN_EXE_ACTION, WEB_EXE_ACTION }, { WEB_ACC_FUN_SEND_EVENT, WEB_SEND_EVENT },
    { WEB_ACC_FUN_HOVER, WEB_HOVER }, { WEB_ACC_FUN_ACC_JSON, WEB_ACC_JSON } };

FocusMoveResult BuildWebFocusMoveResult(FocusMoveResultType resultType)
{
    return {
        .resultType = resultType,
        .nowLevelBelongTreeId = -1,
        .parentWindowId = 0,
        .changeToNewInfo = false,
        .needTerminate = true,
    };
}

void SetWebFocusMoveSearchWithConditionFailResult(
    AccessibilityElementOperatorCallback& callback, FocusMoveResultType resultType, int32_t requestId)
{
    AccessibilityElementInfo elementInfo;
    elementInfo.SetValidElement(false);
    std::list<AccessibilityElementInfo> infos = { elementInfo };
    callback.SetFocusMoveSearchWithConditionResult(infos, BuildWebFocusMoveResult(resultType), requestId);
    TAG_LOGD(AceLogTag::ACE_WEB, "SetFocusMoveSearchWithConditionResult, requestId: %{public}d", requestId);
}

bool IsWebFocusRuleTypeSupported(FocusRuleType type)
{
    switch (type) {
        case FocusRuleType::DEFAULT:
        case FocusRuleType::FOCUS_BY_LINK:
        case FocusRuleType::FOCUS_BY_TITLE:
            return true;
        default:
            return false;
    }
}

bool GetWebComponentSearchSurfaceIdType(int32_t direction, int32_t focusRuleType, SearchSurfaceIdType& searchType)
{
    if (direction != FocusMoveDirection::FORWARD && direction != FocusMoveDirection::BACKWARD) {
        return false;
    }

    bool isBackward = direction == FocusMoveDirection::BACKWARD;
    switch (static_cast<FocusRuleType>(focusRuleType)) {
        case FocusRuleType::FOCUS_BY_LINK:
            searchType = isBackward ? SearchSurfaceIdType::SEARCH_TAIL_LINK : SearchSurfaceIdType::SEARCH_HEAD_LINK;
            break;
        case FocusRuleType::FOCUS_BY_TITLE:
            searchType = isBackward ? SearchSurfaceIdType::SEARCH_TAIL_TITLE : SearchSurfaceIdType::SEARCH_HEAD_TITLE;
            break;
        default:
            searchType = isBackward ? SearchSurfaceIdType::SEARCH_TAIL : SearchSurfaceIdType::SEARCH_HEAD;
            break;
    }
    return true;
}

bool IsWebDumpParamKey(const std::string& param)
{
    return param == WEB_ACC_ID_PARAM || param == WEB_ACC_FUN_PARAM;
}

bool IsWebFocusMoveDumpFun(const std::string& webAccFun)
{
    return webAccFun == WEB_ACC_FUN_PRE || webAccFun == WEB_ACC_FUN_NEXT;
}

bool IsWebActionDumpFun(const std::string& webAccFun)
{
    return webAccFun == WEB_ACC_FUN_EXE_ACTION || webAccFun == WEB_ACC_FUN_SEND_EVENT ||
        webAccFun == WEB_ACC_FUN_HOVER;
}

bool ParseWebAccIdParam(std::vector<std::string>::const_iterator& arg,
    const std::vector<std::string>::const_iterator& end, DumpInfoArgument& argument)
{
    if (++arg == end) {
        return false;
    }
    argument.mode = DumpMode::WEB_ACC_DUMP;
    argument.webAccId = StringUtils::StringToLongInt(*arg);
    return true;
}

bool ParseWebFocusMoveDumpParam(std::vector<std::string>::const_iterator& arg,
    const std::vector<std::string>::const_iterator& end, DumpInfoArgument& argument)
{
    if (!IsWebFocusMoveDumpFun(argument.webAccFun)) {
        return false;
    }
    if (++arg != end) {
        argument.focusMoveRule = StringUtils::StringToInt(*arg);
    }
    return true;
}

bool ParseWebAccFunParam(std::vector<std::string>::const_iterator& arg,
    const std::vector<std::string>::const_iterator& end, DumpInfoArgument& argument)
{
    if (++arg == end) {
        return false;
    }
    argument.mode = DumpMode::WEB_ACC_DUMP;
    argument.webAccFun = *arg;
    return true;
}

bool ParseWebActionDumpParam(std::vector<std::string>::const_iterator& arg,
    const std::vector<std::string>::const_iterator& end, DumpInfoArgument& argument)
{
    if (!IsWebActionDumpFun(argument.webAccFun)) {
        return true;
    }
    if (++arg == end) {
        return false;
    }
    if (argument.webAccFun == WEB_ACC_FUN_HOVER && arg + 1 == end) {
        return false;
    }
    if (argument.webAccFun == WEB_ACC_FUN_EXE_ACTION) {
        argument.action = StringUtils::StringToInt(*arg);
        return true;
    }
    if (argument.webAccFun == WEB_ACC_FUN_SEND_EVENT) {
        argument.eventId = StringUtils::StringToInt(*arg);
        return true;
    }
    argument.pointX = StringUtils::StringToInt(*arg);
    ++arg;
    argument.pointY = StringUtils::StringToInt(*arg);
    return true;
}

const std::map<Accessibility::ActionType, std::function<bool(const AccessibilityActionParam& param)>> ACTIONS = {
    { ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionScrollForward(param.scrollType);
        } },
    { ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionScrollBackward(param.scrollType);
        } },
    { ActionType::ACCESSIBILITY_ACTION_SET_TEXT,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionSetText(param.setTextArgument);
        } },
    { ActionType::ACCESSIBILITY_ACTION_SET_SELECTION,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionSetSelection(param.setSelectionStart,
                                                                      param.setSelectionEnd, param.setSelectionDir);
        } },
    { ActionType::ACCESSIBILITY_ACTION_COPY,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionCopy(); } },
    { ActionType::ACCESSIBILITY_ACTION_CUT,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionCut(); } },
    { ActionType::ACCESSIBILITY_ACTION_PASTE,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionPaste(); } },
    { ActionType::ACCESSIBILITY_ACTION_CLICK,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionClick(); } },
    { ActionType::ACCESSIBILITY_ACTION_LONG_CLICK,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionLongClick(); } },
    { ActionType::ACCESSIBILITY_ACTION_SELECT,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionSelect(); } },
    { ActionType::ACCESSIBILITY_ACTION_CLEAR_SELECTION,
        [](const AccessibilityActionParam& param) { return param.accessibilityProperty->ActActionClearSelection(); } },
    { ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionMoveText(static_cast<int32_t>(param.moveUnit), true);
        } },
    { ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionMoveText(static_cast<int32_t>(param.moveUnit), false);
        } },
    { ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION,
        [](const AccessibilityActionParam& param) {
            auto switchFunc = param.accessibilityProperty->GetSwitchEditableModeFunc();
            if (switchFunc) {
                switchFunc(true);
            }
            return param.accessibilityProperty->ActActionSetIndex(static_cast<int32_t>(param.setCursorIndex));
        } },
    { ActionType::ACCESSIBILITY_ACTION_SPAN_CLICK,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionExecSubComponent(static_cast<int32_t>(param.spanId));
        } },
    { ActionType::ACCESSIBILITY_ACTION_CUSTOM,
        [](const AccessibilityActionParam& param) {
            return param.accessibilityProperty->ActActionCustom(param.customActionName);
        } },
};

bool IsDynamicComponent(const RefPtr<NG::UINode>& node)
{
    return node && (node->GetTag() == V2::DYNAMIC_COMPONENT_ETS_TAG);
}

bool IsIsolatedComponent(const RefPtr<NG::UINode>& node)
{
    return node && (node->GetTag() == V2::ISOLATED_COMPONENT_ETS_TAG);
}

void SetCursorPositionResult(
    Accessibility::AccessibilityElementOperatorCallback& callback, int32_t cursorPosition, int32_t requestId)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "cursorPosition: %{public}d, requestId: %{public}d",
        cursorPosition, requestId);
    callback.SetCursorPositionResult(cursorPosition, requestId);
}

bool NeedUpdateChildrenOfAccessibilityElementInfo(const RefPtr<NG::UINode>& node)
{
    return !IsDynamicComponent(node);
}

bool CheckChildIsAccessibilityFocus(const RefPtr<NG::UINode> &uiNode, RefPtr<NG::FrameNode> &focusFrameNode)
{
    CHECK_NULL_RETURN(uiNode, false);
    auto nowFrameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
    if (nowFrameNode) {
        auto accessibilityProperty = nowFrameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
        if (accessibilityProperty) {
            auto isFocus = accessibilityProperty->GetAccessibilityFocusState();
            if (isFocus) {
                focusFrameNode = nowFrameNode;
                return true;
            }
        }
    }
    auto children = uiNode->GetChildren(true);
    for (const auto& child : children) {
        if (CheckChildIsAccessibilityFocus(child, focusFrameNode)) {
            return true;
        }
    }
    return false;
}

void UpdatePaintNodeRender(const RefPtr<NG::FrameNode>& focusNode)
{
    CHECK_NULL_VOID(focusNode);
    ACE_SCOPED_TRACE("Node[%s] UpdatePaintNodeRender", focusNode->GetTag().c_str());
    auto paintNode = focusNode->GetPaintNode();
    CHECK_NULL_VOID(paintNode);
    auto pattern = paintNode->GetPattern<NG::AccessibilityFocusPaintNodePattern>();
    CHECK_NULL_VOID(pattern);
    pattern->UpdateRenderWithFocusNode();
    auto renderContext = paintNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto accessibilityProperty = focusNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);

    if (accessibilityProperty->IsMatchAccessibilityResponseRegion(false)) {
        auto rectInt = accessibilityProperty->GetAccessibilityResponseRegionRect(false);
        renderContext->UpdateAccessibilityFocusRect(rectInt);
    } else {
        renderContext->UpdateAccessibilityRoundRect();
    }
}

Accessibility::EventType ConvertStrToEventType(const std::string& type)
{
    // static linear map must be sorted by key.
    static const LinearMapNode<Accessibility::EventType> eventTypeMap[] = {
        { ACCESSIBILITY_CLEAR_FOCUS_EVENT, Accessibility::EventType::TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED_EVENT },
        { ACCESSIBILITY_FOCUSED_EVENT, Accessibility::EventType::TYPE_VIEW_ACCESSIBILITY_FOCUSED_EVENT },
        { DOM_CLICK, Accessibility::EventType::TYPE_VIEW_CLICKED_EVENT },
        { DOM_FOCUS, Accessibility::EventType::TYPE_VIEW_FOCUSED_EVENT },
        { DOM_LONG_PRESS, Accessibility::EventType::TYPE_VIEW_LONG_CLICKED_EVENT },
        { MOUSE_HOVER_ENTER, Accessibility::EventType::TYPE_VIEW_HOVER_ENTER_EVENT },
        { MOUSE_HOVER_EXIT, Accessibility::EventType::TYPE_VIEW_HOVER_EXIT_EVENT },
        { PAGE_CHANGE_EVENT, Accessibility::EventType::TYPE_PAGE_STATE_UPDATE },
        { SCROLL_END_EVENT, Accessibility::EventType::TYPE_VIEW_SCROLLED_EVENT },
        { SCROLL_START_EVENT, Accessibility::EventType::TYPE_VIEW_SCROLLED_START },
        { DOM_SELECTED, Accessibility::EventType::TYPE_VIEW_SELECTED_EVENT },
        { TEXT_CHANGE_EVENT, Accessibility::EventType::TYPE_VIEW_TEXT_UPDATE_EVENT },
        { DOM_TOUCH_END, Accessibility::EventType::TYPE_TOUCH_END },
        { DOM_TOUCH_START, Accessibility::EventType::TYPE_TOUCH_BEGIN },
    };
    Accessibility::EventType eventType = Accessibility::EventType::TYPE_VIEW_INVALID;
    int64_t idx = BinarySearchFindIndex(eventTypeMap, ArraySize(eventTypeMap), type.c_str());
    if (idx >= 0) {
        eventType = eventTypeMap[idx].value;
    }
    return eventType;
}

Accessibility::EventType ConvertAceEventType(AccessibilityEventType type)
{
    static const LinearEnumMapNode<AccessibilityEventType, Accessibility::EventType> eventTypeMap[] = {
        { AccessibilityEventType::CLICK, Accessibility::EventType::TYPE_VIEW_CLICKED_EVENT },
        { AccessibilityEventType::LONG_PRESS, Accessibility::EventType::TYPE_VIEW_LONG_CLICKED_EVENT },
        { AccessibilityEventType::SELECTED, Accessibility::EventType::TYPE_VIEW_SELECTED_EVENT },
        { AccessibilityEventType::FOCUS, Accessibility::EventType::TYPE_VIEW_FOCUSED_EVENT },
        { AccessibilityEventType::TEXT_CHANGE, Accessibility::EventType::TYPE_VIEW_TEXT_UPDATE_EVENT },
        { AccessibilityEventType::HOVER_ENTER_EVENT, Accessibility::EventType::TYPE_VIEW_HOVER_ENTER_EVENT },
        { AccessibilityEventType::PAGE_CHANGE, Accessibility::EventType::TYPE_PAGE_STATE_UPDATE },
        { AccessibilityEventType::HOVER_EXIT_EVENT, Accessibility::EventType::TYPE_VIEW_HOVER_EXIT_EVENT },
        { AccessibilityEventType::CHANGE, Accessibility::EventType::TYPE_PAGE_CONTENT_UPDATE },
        { AccessibilityEventType::COMPONENT_CHANGE, Accessibility::EventType::TYPE_VIEW_TEXT_UPDATE_EVENT },
        { AccessibilityEventType::SCROLL_END, Accessibility::EventType::TYPE_VIEW_SCROLLED_EVENT },
        { AccessibilityEventType::TEXT_SELECTION_UPDATE,
            Accessibility::EventType::TYPE_VIEW_TEXT_SELECTION_UPDATE_EVENT },
        { AccessibilityEventType::ACCESSIBILITY_FOCUSED,
            Accessibility::EventType::TYPE_VIEW_ACCESSIBILITY_FOCUSED_EVENT },
        { AccessibilityEventType::ACCESSIBILITY_FOCUS_CLEARED,
            Accessibility::EventType::TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED_EVENT },
        { AccessibilityEventType::TEXT_MOVE_UNIT, Accessibility::EventType::TYPE_VIEW_TEXT_MOVE_UNIT_EVENT },
        { AccessibilityEventType::REQUEST_FOCUS, Accessibility::EventType::TYPE_VIEW_REQUEST_FOCUS_FOR_ACCESSIBILITY },
        { AccessibilityEventType::SCROLL_START, Accessibility::EventType::TYPE_VIEW_SCROLLED_START },
        { AccessibilityEventType::PAGE_CLOSE, Accessibility::EventType::TYPE_PAGE_CLOSE },
        { AccessibilityEventType::ANNOUNCE_FOR_ACCESSIBILITY,
            Accessibility::EventType::TYPE_VIEW_ANNOUNCE_FOR_ACCESSIBILITY },
        { AccessibilityEventType::PAGE_OPEN, Accessibility::EventType::TYPE_PAGE_OPEN },
        { AccessibilityEventType::ELEMENT_INFO_CHANGE, Accessibility::EventType::TYPE_ELEMENT_INFO_CHANGE },
        { AccessibilityEventType::ANNOUNCE_FOR_ACCESSIBILITY_NOT_INTERRUPT,
            Accessibility::EventType::TYPE_VIEW_ANNOUNCE_FOR_ACCESSIBILITY_NOT_INTERRUPT },
        { AccessibilityEventType::REQUEST_FOCUS_FOR_ACCESSIBILITY_NOT_INTERRUPT,
            Accessibility::EventType::TYPE_VIEW_REQUEST_FOCUS_FOR_ACCESSIBILITY_NOT_INTERRUPT },
        { AccessibilityEventType::SCROLLING_EVENT, Accessibility::EventType::TYPE_VIEW_SCROLLING_EVENT },
        { AccessibilityEventType::FOCUS_INVISIBLE, Accessibility::EventType::TYPE_FOCUS_INVISIBLE },
        { AccessibilityEventType::PAGE_PRE_CHANGE, Accessibility::EventType::TYPE_PAGE_STATE_PRE_UPDATE },
    };
    Accessibility::EventType eventType = Accessibility::EventType::TYPE_VIEW_INVALID;
    int64_t idx = BinarySearchFindIndex(eventTypeMap, ArraySize(eventTypeMap), type);
    if (idx >= 0) {
        eventType = eventTypeMap[idx].value;
    }
    return eventType;
}

inline RangeInfo ConvertAccessibilityValue(const AccessibilityValue& value)
{
    return RangeInfo(value.min, value.max, value.current);
}

int64_t ConvertToCardAccessibilityId(int64_t nodeId, int64_t cardId, int64_t rootNodeId)
{
    // result is integer total ten digits, top five for agp virtualViewId, end five for ace nodeId,
    // for example agp virtualViewId is 32, ace nodeId is 1000001, convert to result is 00032 10001.
    int64_t result = 0;
    if (nodeId == rootNodeId + ROOT_STACK_BASE) {
        // for example agp virtualViewId is 32 root node is 2100000, convert to result is 00032 21000.
        result = cardId * CARD_BASE + (static_cast<int64_t>(nodeId / CARD_BASE)) * CARD_ROOT_NODE_ID_RATION +
                 nodeId % CARD_BASE;
    } else {
        result = cardId * CARD_BASE + (static_cast<int64_t>(nodeId / DOM_ROOT_NODE_ID_BASE)) * CARD_NODE_ID_RATION +
                 nodeId % DOM_ROOT_NODE_ID_BASE;
    }
    return result;
}

void UpdateCacheInfo(std::list<AccessibilityElementInfo>& infos, uint32_t mode, const RefPtr<AccessibilityNode>& node,
    const RefPtr<JsAccessibilityManager>& jsAccessibilityManager, int windowId)
{
    CHECK_NULL_VOID(node);
    CHECK_NULL_VOID(jsAccessibilityManager);
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "elementId: %{public}d, mode: %{public}d", node->GetNodeId(), mode);
    // parent
    uint32_t umode = mode;
    if (umode & static_cast<uint32_t>(PREFETCH_PREDECESSORS)) {
        if (node->GetParentId() != -1 && node->GetParentId() != DEFAULT_PARENT_ID) {
            AccessibilityElementInfo parentNodeInfo;
            jsAccessibilityManager->UpdateAccessibilityNodeInfo(
                node->GetParentNode(), parentNodeInfo, jsAccessibilityManager, windowId);
            infos.emplace_back(parentNodeInfo);
        }
    }
    // sister/brothers
    if (umode & static_cast<uint32_t>(PREFETCH_SIBLINGS)) {
        if (node->GetParentId() != -1 && node->GetParentId() != DEFAULT_PARENT_ID) {
            for (const auto& item : node->GetParentNode()->GetChildList()) {
                if (node->GetNodeId() != item->GetNodeId()) {
                    AccessibilityElementInfo siblingNodeInfo;
                    jsAccessibilityManager->UpdateAccessibilityNodeInfo(
                        item, siblingNodeInfo, jsAccessibilityManager, windowId);
                    infos.emplace_back(siblingNodeInfo);
                }
            }
        }
    }
    // children
    if (umode & static_cast<uint32_t>(PREFETCH_CHILDREN)) {
        for (const auto& item : node->GetChildList()) {
            AccessibilityElementInfo childNodeInfo;
            jsAccessibilityManager->UpdateAccessibilityNodeInfo(item, childNodeInfo, jsAccessibilityManager, windowId);
            infos.emplace_back(childNodeInfo);
        }
    }

    // get all children
    if (umode & static_cast<uint32_t>(PREFETCH_RECURSIVE_CHILDREN)) {
        std::list<RefPtr<AccessibilityNode>> children;
        for (const auto& item : node->GetChildList()) {
            children.emplace_back(item);
        }

        while (!children.empty()) {
            auto parent = children.front();
            children.pop_front();
            AccessibilityElementInfo childNodeInfo;
            jsAccessibilityManager->UpdateAccessibilityNodeInfo(
                parent, childNodeInfo, jsAccessibilityManager, windowId);
            infos.push_back(childNodeInfo);
            for (const auto& item : parent->GetChildList()) {
                children.emplace_back(item);
            }
        }
    }
}

void SortAccessibilityInfosByBreadth(std::list<AccessibilityElementInfo>& infos,
    std::list<int64_t>& accessibilityIdQueue, std::list<AccessibilityElementInfo>& output)
{
    while (!accessibilityIdQueue.empty()) {
        auto accessibilityId = accessibilityIdQueue.front();
        accessibilityIdQueue.pop_front();
        for (std::list<AccessibilityElementInfo>::iterator info = infos.begin(); info != infos.end(); ++info) {
            if (accessibilityId != info->GetAccessibilityId()) {
                continue;
            }
            for (auto& child : info->GetChildIds()) {
                accessibilityIdQueue.emplace_back(child);
            }
            output.emplace_back(*info);
            infos.erase(info);
            break;
        }
        SortAccessibilityInfosByBreadth(infos, accessibilityIdQueue, output);
    }
}

void SortExtensionAccessibilityInfo(std::list<AccessibilityElementInfo>& infos, int64_t rootAccessibilityId)
{
    auto input = infos;
    infos.clear();
    std::list<int64_t> accessibilityIdQueue;
    accessibilityIdQueue.emplace_back(rootAccessibilityId);
    SortAccessibilityInfosByBreadth(input, accessibilityIdQueue, infos);
}

void ConvertExtensionAccessibilityId(AccessibilityElementInfo& info, const RefPtr<NG::FrameNode>& extensionNode,
    int64_t uiExtensionOffset, AccessibilityElementInfo& parentInfo)
{
    auto extensionAbilityId =
            extensionNode->WrapExtensionAbilityId(uiExtensionOffset, info.GetAccessibilityId());
    info.SetAccessibilityId(extensionAbilityId);
    auto parentNodeId =
        extensionNode->WrapExtensionAbilityId(uiExtensionOffset, info.GetParentNodeId());
    info.SetParent(parentNodeId);
    auto childIds = info.GetChildIds();
    for (auto& child : childIds) {
        info.RemoveChild(child);
        info.AddChild(extensionNode->WrapExtensionAbilityId(uiExtensionOffset, child));
    }
    if (V2::ROOT_ETS_TAG == info.GetComponentType()) {
        for (auto& child : info.GetChildIds()) {
            parentInfo.AddChild(child);
        }
    }
}

void ConvertExtensionAccessibilityNodeId(std::list<AccessibilityElementInfo>& infos,
    const RefPtr<NG::FrameNode>& extensionNode, int64_t uiExtensionOffset,
    AccessibilityElementInfo& parentInfo)
{
    CHECK_NULL_VOID(extensionNode);
    for (auto& accessibilityElementInfo : infos) {
        ConvertExtensionAccessibilityId(accessibilityElementInfo, extensionNode, uiExtensionOffset, parentInfo);
    }
    for (auto& accessibilityElementInfo : infos) {
        if (std::find(parentInfo.GetChildIds().begin(), parentInfo.GetChildIds().end(),
            accessibilityElementInfo.GetAccessibilityId()) != parentInfo.GetChildIds().end()) {
            accessibilityElementInfo.SetParent(extensionNode->GetAccessibilityId());
        }
    }
}

inline std::string BoolToString(bool tag)
{
    return tag ? "true" : "false";
}

std::string ConvertInputTypeToString(AceTextCategory type)
{
    switch (type) {
        case AceTextCategory::INPUT_TYPE_DEFAULT:
            return "INPUT_TYPE_DEFAULT";
        case AceTextCategory::INPUT_TYPE_TEXT:
            return "INPUT_TYPE_TEXT";
        case AceTextCategory::INPUT_TYPE_EMAIL:
            return "INPUT_TYPE_EMAIL";
        case AceTextCategory::INPUT_TYPE_DATE:
            return "INPUT_TYPE_DATE";
        case AceTextCategory::INPUT_TYPE_TIME:
            return "INPUT_TYPE_TIME";
        case AceTextCategory::INPUT_TYPE_NUMBER:
            return "INPUT_TYPE_NUMBER";
        case AceTextCategory::INPUT_TYPE_PASSWORD:
            return "INPUT_TYPE_PASSWORD";
        case AceTextCategory::INPUT_TYPE_PHONENUMBER:
            return "INPUT_TYPE_PHONENUMBER";
        case AceTextCategory::INPUT_TYPE_USER_NAME:
            return "INPUT_TYPE_USER_NAME";
        case AceTextCategory::INPUT_TYPE_NEW_PASSWORD:
            return "INPUT_TYPE_NEW_PASSWORD";
        default:
            return "illegal input type";
    }
}

bool FindAccessibilityFocus(const RefPtr<AccessibilityNode>& node, RefPtr<AccessibilityNode>& resultNode)
{
    CHECK_NULL_RETURN(node, false);
    if (node->GetAccessibilityFocusedState()) {
        resultNode = node;
        return true;
    }
    if (!node->GetChildList().empty()) {
        for (const auto& item : node->GetChildList()) {
            if (resultNode != nullptr) {
                return true;
            }
            if (FindAccessibilityFocus(item, resultNode)) {
                return true;
            }
        }
    }

    return false;
}

void FindFocusedExtensionElementInfoNG(const SearchParameter& searchParam,
    const RefPtr<NG::FrameNode>& node, Accessibility::AccessibilityElementInfo& info)
{
    if (NG::UI_EXTENSION_OFFSET_MIN < (searchParam.uiExtensionOffset + 1)) {
        node->FindFocusedExtensionElementInfoNG(searchParam.nodeId, searchParam.mode,
            searchParam.uiExtensionOffset / NG::UI_EXTENSION_ID_FACTOR, info);
    } else {
        info.SetValidElement(false);
    }
}

void SetUiExtensionAbilityParentIdForFocus(const RefPtr<NG::UINode>& uiExtensionNode,
    const int64_t uiExtensionOffset, Accessibility::AccessibilityElementInfo& info)
{
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiExtensionNode);
    if (frameNode) {
        auto parentId = info.GetParentNodeId();
        AccessibilityElementInfo parentInfo;
        ConvertExtensionAccessibilityId(info, frameNode, uiExtensionOffset, parentInfo);
        if (parentId == NG::UI_EXTENSION_ROOT_ID) {
            info.SetParent(frameNode->GetAccessibilityId());
        }
    }
}

bool GetAccessibilityFocusStateByFrameNode(const RefPtr<NG::FrameNode>& frameNode)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto& renderContext = frameNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, false);
    auto isAccessibilityFocus = renderContext->GetAccessibilityFocus().value_or(false);
    CHECK_EQUAL_RETURN(isAccessibilityFocus, true, true);
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, false);
    return accessibilityProperty->GetAccessibilityFocusState();
}

RefPtr<NG::FrameNode> FindAccessibilityFocus(const RefPtr<NG::UINode>& node,
    int32_t focusType, Accessibility::AccessibilityElementInfo& info,
    const int64_t uiExtensionOffset, const RefPtr<PipelineBase>& context, int64_t currentFocusNodeId)
{
    CHECK_NULL_RETURN(node, nullptr);
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    if (GetAccessibilityFocusStateByFrameNode(frameNode)) {
        auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(frameNode, currentFocusNodeId);
        return node;
    }
    if (node->GetChildren(true).empty()) {
        return nullptr;
    }
    for (const auto& child : node->GetChildren(true)) {
        auto extensionNode = AceType::DynamicCast<NG::FrameNode>(child);
        if (IsIsolatedComponent(child) && extensionNode &&
            !IsUIExtensionShowPlaceholder(extensionNode) &&
            (extensionNode->GetUiExtensionId() > NG::UI_EXTENSION_UNKNOW_ID) &&
            (((extensionNode->GetUiExtensionId() <= NG::UI_EXTENSION_ID_FIRST_MAX) &&
            (NG::UI_EXTENSION_OFFSET_MAX == uiExtensionOffset)) ||
            (extensionNode->GetUiExtensionId() <= NG::UI_EXTENSION_ID_OTHER_MAX))) {
            SearchParameter transferSearchParam {NG::UI_EXTENSION_ROOT_ID, "", focusType, uiExtensionOffset};
            OHOS::Ace::Framework::FindFocusedExtensionElementInfoNG(
                transferSearchParam, extensionNode, info);
            if (info.GetAccessibilityId() < 0) {
                continue;
            }
            SetUiExtensionAbilityParentIdForFocus(extensionNode, uiExtensionOffset, info);
            return extensionNode;
        }
        auto result = FindAccessibilityFocus(child, focusType, info, uiExtensionOffset, context, currentFocusNodeId);
        if (result) {
            return result;
        }
    }
    return nullptr;
}

bool FindInputFocus(const RefPtr<AccessibilityNode>& node, RefPtr<AccessibilityNode>& resultNode)
{
    CHECK_NULL_RETURN(node, false);
    if (!node->GetFocusedState() && (node->GetParentId() != -1)) {
        return false;
    }
    if (node->GetFocusedState()) {
        resultNode = node;
    }
    if (!node->GetChildList().empty()) {
        for (const auto& item : node->GetChildList()) {
            if (FindInputFocus(item, resultNode)) {
                return true;
            }
        }
    }
    return node->GetFocusedState();
}

RefPtr<NG::FrameNode> FindInputFocus(const RefPtr<NG::UINode>& node, int32_t focusType,
    Accessibility::AccessibilityElementInfo& info, const int64_t uiExtensionOffset,
    const RefPtr<PipelineBase>& context)
{
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    CHECK_NULL_RETURN(frameNode, nullptr);
    if (!(frameNode->GetFocusHub() ? frameNode->GetFocusHub()->IsCurrentFocus() : false)) {
        return nullptr;
    }
    if (frameNode->GetFocusHub()->IsChild()) {
        if (frameNode->IsInternal()) {
            return frameNode->GetFocusParent();
        }
        return frameNode;
    }
    auto focusHub = frameNode->GetFocusHub();
    RefPtr<NG::FrameNode> target;
    focusHub->AnyChildFocusHub([&target, &info, context, focusType, uiExtensionOffset](
                                   const RefPtr<NG::FocusHub>& focusChild) {
        auto extensionNode = focusChild->GetFrameNode();
        if ((extensionNode && IsIsolatedComponent(extensionNode)) &&
            !IsUIExtensionShowPlaceholder(extensionNode) &&
            (extensionNode->GetUiExtensionId() > NG::UI_EXTENSION_UNKNOW_ID) &&
            (((extensionNode->GetUiExtensionId() <= NG::UI_EXTENSION_ID_FIRST_MAX) &&
            (NG::UI_EXTENSION_OFFSET_MAX == uiExtensionOffset)) ||
            (extensionNode->GetUiExtensionId() <= NG::UI_EXTENSION_ID_OTHER_MAX))) {
            SearchParameter transferSearchParam {NG::UI_EXTENSION_ROOT_ID, "", focusType, uiExtensionOffset};
            OHOS::Ace::Framework::FindFocusedExtensionElementInfoNG(
                transferSearchParam, extensionNode, info);
            if (info.GetAccessibilityId() < 0) {
                return false;
            }
            SetUiExtensionAbilityParentIdForFocus(extensionNode, uiExtensionOffset, info);
            target = extensionNode;
            return true;
        }
        target = FindInputFocus(focusChild->GetFrameNode(), focusType, info, uiExtensionOffset, context);
        return target ? true : false;
    });
    return target;
}

void FindText(
    const RefPtr<AccessibilityNode>& node, const std::string& text, std::list<RefPtr<AccessibilityNode>>& nodeList)
{
    CHECK_NULL_VOID(node);
    if (node->GetText().find(text) != std::string::npos) {
        nodeList.push_back(node);
    }
    if (!node->GetChildList().empty()) {
        for (const auto& child : node->GetChildList()) {
            FindText(child, text, nodeList);
        }
    }
}

void FindText(const RefPtr<NG::UINode>& node, const std::string& text, std::list<RefPtr<NG::FrameNode>>& nodeList)
{
    CHECK_NULL_VOID(node);

    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    if (frameNode && !frameNode->IsInternal()) {
        if (frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>()->GetGroupText().find(text) !=
            std::string::npos) {
            nodeList.push_back(frameNode);
        }
    }

    if (!node->GetChildren(true).empty()) {
        for (const auto& child : node->GetChildren(true)) {
            FindText(child, text, nodeList);
        }
    }
}

using FindCondition = std::function<bool(const RefPtr<NG::FrameNode>&)>;

bool FindFrameNodeByCondition(const std::list<RefPtr<NG::UINode>>& children,
    std::queue<RefPtr<NG::UINode>>& nodes, RefPtr<NG::FrameNode>& result, FindCondition condition)
{
    for (const auto& child : children) {
        auto frameNode = AceType::DynamicCast<NG::FrameNode>(child);
        if (frameNode != nullptr && !frameNode->CheckAccessibilityLevelNo()) {
            if (condition(frameNode)) {
                result = frameNode;
                return true;
            }
        }
        nodes.push(child);
    }
    return false;
}

RefPtr<NG::FrameNode> GetFramenodeByCondition(const RefPtr<NG::FrameNode>& root, FindCondition condition)
{
    CHECK_NULL_RETURN(root, nullptr);

    std::queue<RefPtr<NG::UINode>> nodes;
    nodes.push(root);
    RefPtr<NG::FrameNode> frameNodeResult = nullptr;

    while (!nodes.empty()) {
        auto current = nodes.front();
        nodes.pop();
        if (current->HasVirtualNodeAccessibilityProperty()) {
            auto fnode = AceType::DynamicCast<NG::FrameNode>(current);
            auto property = fnode->GetAccessibilityProperty<NG::AccessibilityProperty>();
            const auto& children = std::list<RefPtr<NG::UINode>> { property->GetAccessibilityVirtualNode() };
            if (FindFrameNodeByCondition(children, nodes, frameNodeResult, condition)) {
                return frameNodeResult;
            }
        } else {
            const auto& children = current->GetChildren(true);
            if (FindFrameNodeByCondition(children, nodes, frameNodeResult, condition)) {
                return frameNodeResult;
            }
        }
        auto frameNode = AceType::DynamicCast<NG::FrameNode>(current);
        if (!frameNode) {
            continue;
        }
        auto overlayNode = frameNode->GetOverlayNode();
        if (overlayNode) {
            const auto& children = std::list<RefPtr<NG::UINode>> { overlayNode };
            if (FindFrameNodeByCondition(children, nodes, frameNodeResult, condition)) {
                return frameNodeResult;
            }
        }
    }
    return nullptr;
}

void GetFrameNodeParent(const RefPtr<NG::UINode>& uiNode, RefPtr<NG::FrameNode>& parent)
{
    if (AceType::InstanceOf<NG::FrameNode>(uiNode)) {
        auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
        if (!frameNode->IsInternal()) {
            parent = frameNode;
            return;
        }
    }
    CHECK_NULL_VOID(uiNode);
    auto parentNode = uiNode->GetParent();
    GetFrameNodeParent(parentNode, parent);
}

bool NeedChangeTreeId(int64_t elementId, int32_t targetTreeId)
{
    if (elementId == INVALID_PARENT_ID) {
        return true;
    }
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);
    if (splitTreeId != 0 &&
        splitTreeId != AccessibilityElementInfo::UNDEFINED_TREE_ID &&
        splitTreeId != targetTreeId) {
        return false;
    }
    return true;
}

std::string GetSurfaceIdByEmbedNode(const RefPtr<NG::FrameNode>& node)
{
    auto surfaceId = ElementRegister::GetInstance()->GetSurfaceIdByEmbedNode(AceType::RawPtr(node));
    return std::to_string(surfaceId);
}

WeakPtr<NG::FrameNode> GetEmbedNodeBySurfaceId(const std::string& surfaceId)
{
    std::stringstream ss(surfaceId);
    uint64_t id;
    if (ss >> id) {
        return ElementRegister::GetInstance()->GetEmbedNodeBySurfaceId(id);
    }
    return nullptr;
}

bool CheckFrameNodeByAccessibilityLevel(const RefPtr<NG::FrameNode>& frameNode, bool isParent)
{
    return true;
}

bool IsInPageNodes(int32_t pageId, const std::vector<RefPtr<NG::FrameNode>>& pageNodes)
{
    for (const auto& page : pageNodes) {
        CHECK_NULL_CONTINUE(page);
        if (pageId == page->GetPageId()) {
            return true;
        }
    }
    return false;
}

bool CheckAndSetAccessibilityVisible(const RefPtr<NG::FrameNode>& node, bool isReduceMode = false)
{
    if (!isReduceMode) {
        return true;
    }
    auto parentNode = node->GetParentFrameNode();
    if (!parentNode) {
        if (node->GetTag() != V2::PAGE_ETS_TAG) {
            node->SetAccessibilityVisible(node->IsActive() && node->IsVisible());
        }
    } else {
        if (node->GetTag() == V2::PAGE_ETS_TAG) {
            return node->GetAccessibilityVisible();
        }
        auto nodeAccessibilityVisible = node->IsActive() && node->IsVisible() && parentNode->GetAccessibilityVisible();
        node->SetAccessibilityVisible(nodeAccessibilityVisible);
    }
    return node->GetAccessibilityVisible();
}

bool IsValidEmbedTarget(const RefPtr<NG::FrameNode>& frameNode, const CommonProperty& commonProperty)
{
    return commonProperty.checkEmbedNode &&
           frameNode && commonProperty.checkGetFunc &&
           (commonProperty.checkGetFunc(frameNode) != INVALID_NODE_ID);
}

void GetFrameNodeChildren(
    const RefPtr<NG::UINode>& uiNode,
    std::list<RefPtr<NG::FrameNode>>& children,
    const CommonProperty& commonProperty);

// Eligibility check for one frame node against the enumeration filters (active / page scope / internal / stage).
bool CollectEligibleFrameNode(const RefPtr<NG::UINode>& uiNode, const RefPtr<NG::FrameNode>& frameNode,
    std::list<RefPtr<NG::FrameNode>>& children, const CommonProperty& commonProperty)
{
    CHECK_NULL_RETURN(frameNode, false);
    CHECK_NULL_RETURN(frameNode->IsActive(), true);
    if (uiNode->GetTag() == "page") {
        if (!commonProperty.pageNodes.empty() && !IsInPageNodes(uiNode->GetPageId(), commonProperty.pageNodes)) {
            return true;
        }
    } else if (!frameNode->IsInternal() && uiNode->GetTag() != "stage") {
        if (CheckFrameNodeByAccessibilityLevel(frameNode, false) &&
            CheckAndSetAccessibilityVisible(frameNode, commonProperty.isReduceMode)) {
            children.emplace_back(frameNode);
            return true;
        }
    }
    return false;
}

// Overlay and accessibility-virtual-node branches of the enumeration.
bool CollectOverlayAndVirtualChildren(const RefPtr<NG::FrameNode>& frameNode,
    std::list<RefPtr<NG::FrameNode>>& children, const CommonProperty& commonProperty)
{
    auto overlayNode = frameNode->GetOverlayNode();
    if (overlayNode) {
        GetFrameNodeChildren(overlayNode, children, commonProperty);
    }

    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    auto uiVirtualNode = accessibilityProperty->GetAccessibilityVirtualNode();
    if (uiVirtualNode != nullptr) {
        auto virtualNode = AceType::DynamicCast<NG::FrameNode>(uiVirtualNode);
        if (virtualNode != nullptr) {
            GetFrameNodeChildren(virtualNode, children, commonProperty);
            return true;
        }
    }
    return false;
}

void GetFrameNodeChildren(
    const RefPtr<NG::UINode>& uiNode,
    std::list<RefPtr<NG::FrameNode>>& children,
    const CommonProperty& commonProperty)
{
    CHECK_NULL_VOID(uiNode);
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
    // Exported-subtree roots are owned by their CMP binding's operator (T_sub path): cut them from any other tree's.
    if (OHOS::Ace::CmpAccessibility::CmpAccessibilityRegistry::IsRegisteredExportedRoot(
        frameNode.GetRawPtr())) {
        return;
    }
    if (IsValidEmbedTarget(frameNode, commonProperty)) {
        return;
    }

    if (CollectEligibleFrameNode(uiNode, frameNode, children, commonProperty)) {
        return;
    }
    if (frameNode && CollectOverlayAndVirtualChildren(frameNode, children, commonProperty)) {
        return;
    }

    for (const auto& frameChild : uiNode->GetChildren(true)) {
        GetFrameNodeChildren(frameChild, children, commonProperty);
    }
}

void SetRootAccessibilityVisible(const RefPtr<NG::UINode>& uiNode, AccessibilityElementInfo& nodeInfo)
{
    NG::AccessibilityFrameNodeUtils::UpdateAccessibilityVisibleToRoot(uiNode);
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
    CHECK_NULL_VOID(frameNode);
    nodeInfo.SetAccessibilityVisible(frameNode->GetAccessibilityVisible());
}

RefPtr<NG::FrameNode> GetAccessibilityNextFocusNode(
    const RefPtr<NG::UINode>& currentNode,
    const RefPtr<NG::FrameNode>& rootNode)
{
    auto currentFrameNode = AceType::DynamicCast<NG::FrameNode>(currentNode);
    CHECK_NULL_RETURN(currentFrameNode, nullptr);
    auto accessibilityProperty = currentFrameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, nullptr);
    if (accessibilityProperty->GetAccessibilityNextFocusInspectorKey() == "") {
        return nullptr;
    }

    FindCondition condition = [accessibilityProperty](const RefPtr<NG::FrameNode>& node) {
        return node->GetInspectorId() == accessibilityProperty->GetAccessibilityNextFocusInspectorKey();
    };
    return GetFramenodeByCondition(rootNode, condition);
}

void SetRootAccessibilityNextFocusId(const RefPtr<NG::UINode>& currentNode, const RefPtr<NG::FrameNode>& rootNode,
                                     AccessibilityElementInfo& nodeInfo)
{
    auto nextNode = GetAccessibilityNextFocusNode(currentNode, rootNode);
    CHECK_NULL_VOID(nextNode);
    auto elementId = nextNode->GetAccessibilityId();
    if (nodeInfo.GetBelongTreeId() > 0) {
        AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(nodeInfo.GetBelongTreeId(), elementId);
    }
    nodeInfo.SetAccessibilityNextFocusId(elementId);
}

RefPtr<NG::FrameNode> GetAccessibilityPrevFocusNode(
    const RefPtr<NG::UINode>& currentNode,
    const RefPtr<NG::FrameNode>& rootNode,
    const NG::NextFocusRelationController::TargetMap& nextFocusIdMap)
{
    auto currentFrameNode = AceType::DynamicCast<NG::FrameNode>(currentNode);
    CHECK_NULL_RETURN(currentFrameNode, nullptr);

    auto currentInspectorId = currentFrameNode->GetInspectorId().value_or("");
    if (currentInspectorId == "") {
        return nullptr;
    }
    auto it = nextFocusIdMap.find(currentInspectorId);
    if (it != nextFocusIdMap.end()) {
        if (currentFrameNode->GetAccessibilityId() == it->second.sourceAccessibilityId) {
            return nullptr;
        }
        return NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(
            rootNode, it->second.sourceAccessibilityId);
    }
    return nullptr;
}

void SetRootAccessibilityPreFocusId(const RefPtr<NG::UINode>& currentNode, const RefPtr<NG::FrameNode>& rootNode,
                                    AccessibilityElementInfo& nodeInfo,
                                    const NG::NextFocusRelationController::TargetMap& nextFocusIdMap)
{
    auto currentFrameNode = AceType::DynamicCast<NG::FrameNode>(currentNode);
    CHECK_NULL_VOID(currentFrameNode);

    auto currentInspectorId = currentFrameNode->GetInspectorId().value_or("");
    if (currentInspectorId == "") {
        return;
    }
    auto it = nextFocusIdMap.find(currentInspectorId);
    if (it != nextFocusIdMap.end()) {
        int64_t preAccessibilityId = it->second.sourceAccessibilityId;
        if (currentFrameNode->GetAccessibilityId() == preAccessibilityId) {
            return;
        }
        if (nodeInfo.GetBelongTreeId() > 0) {
            AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(nodeInfo.GetBelongTreeId(), preAccessibilityId);
        }
        nodeInfo.SetAccessibilityPreviousFocusId(preAccessibilityId);
    }
}

int64_t GetParentId(const RefPtr<NG::UINode>& uiNode)
{
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode.GetRawPtr());
    if (frameNode) {
        if (frameNode->IsFirstVirtualNode()) {
            auto weakNode = frameNode->GetVirtualNodeParent();
            auto refNode = weakNode.Upgrade();
            return refNode == nullptr ? INVALID_PARENT_ID : refNode->GetAccessibilityId();
        }
    }
    auto parent = uiNode->GetParent();
    while (parent) {
        auto parentFrameNode = AceType::DynamicCast<NG::FrameNode>(parent.GetRawPtr());
        if (parentFrameNode) {
            if ((parent->GetTag() == V2::PAGE_ETS_TAG) || (parent->GetTag() == V2::STAGE_ETS_TAG) ||
                parentFrameNode->CheckAccessibilityLevelNo()) {
                parent = parent->GetParent();
                continue;
            }
            return parent->GetAccessibilityId();
        }
        parent = parent->GetParent();
    }
    return INVALID_PARENT_ID;
}

void FillElementInfo(int64_t elementId, AccessibilityElementInfo& elementInfo, const RefPtr<PipelineBase>& context,
    const RefPtr<JsAccessibilityManager>& jsAccessibilityManager, const FillEventInfoParam& param)
{
    int64_t elementIdUnwrap = elementId;
    int64_t uiextensionId = 0;
    std::list<AccessibilityElementInfo> elementInfos;
    int32_t mode = 0;
    CHECK_NULL_VOID(jsAccessibilityManager);
#ifdef WINDOW_SCENE_SUPPORTED
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    if (uiExtensionManager->IsWrapExtensionAbilityId(elementId)) {
        auto unWrapIdPair = uiExtensionManager->UnWrapExtensionAbilityId(
            NG::UI_EXTENSION_OFFSET_MAX, elementIdUnwrap);
        elementIdUnwrap = unWrapIdPair.second;
        uiextensionId = unWrapIdPair.first;
    }
#endif
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
        "start to search elementId: %{public}" PRId64, elementIdUnwrap);
    jsAccessibilityManager->SearchElementInfoByAccessibilityIdNG(
        elementIdUnwrap, mode, elementInfos, context, NG::UI_EXTENSION_OFFSET_MAX);
    if (elementInfos.empty()) {
        LOGE("Element infos is empty. Find element infos failed.");
        return;
    }
    elementInfo = elementInfos.front();
    if (uiextensionId > 0) {
        elementIdUnwrap = (uiextensionId * NG::UI_EXTENSION_OFFSET_MAX) + elementInfo.GetAccessibilityId();
        TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "elementIdUnwrap: %{public}" PRId64, elementIdUnwrap);
    }
    elementInfo.SetAccessibilityId(elementIdUnwrap);
    elementInfo.SetWindowId(param.windowId);
    jsAccessibilityManager->UpdateElementInfoInnerWindowId(elementInfo, elementId);
}

void FillEventInfo(const RefPtr<NG::FrameNode>& node,
                   AccessibilityEventInfo& eventInfo,
                   const RefPtr<PipelineBase>& context,
                   const RefPtr<JsAccessibilityManager>& jsAccessibilityManager,
                   const FillEventInfoParam& param)
{
    CHECK_NULL_VOID(node);
    eventInfo.SetComponentType(node->GetTag());
    eventInfo.SetPageId(node->GetPageId());
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    eventInfo.SetItemCounts(accessibilityProperty->GetCollectionItemCounts());
    eventInfo.SetBeginIndex(accessibilityProperty->GetBeginIndex());
    eventInfo.SetEndIndex(accessibilityProperty->GetEndIndex());
    AccessibilityElementInfo elementInfo;
    FillElementInfo(param.elementId, elementInfo, context, jsAccessibilityManager, param);
    if (param.stackNodeId != -1) {
        int64_t stackNodeId = param.stackNodeId;
        AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(elementInfo.GetBelongTreeId(), stackNodeId);
        elementInfo.SetNavDestinationId(stackNodeId);
    }
    eventInfo.AddContent(elementInfo.GetContent());
    eventInfo.SetElementInfo(elementInfo);
}

uint64_t GetDisplayId(int windowId, const RefPtr<PipelineBase>& pipeline)
{
    // if not sceneboard, displayId get 0.
    if (windowId != 1) {
        return 0;
    }

    CHECK_NULL_RETURN(pipeline, 0);
    RefPtr<NG::PipelineContext> ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    CHECK_NULL_RETURN(ngPipeline, 0);
    auto container = Container::GetContainer(ngPipeline->GetInstanceId());
    CHECK_NULL_RETURN(container, 0);
    return container->GetCurrentDisplayId();
}

#ifdef WEB_SUPPORTED

void FillWebElementInfo(int64_t elementId, AccessibilityElementInfo& elementInfo,
                        const RefPtr<PipelineBase>& context,
                        const RefPtr<JsAccessibilityManager>& jsAccessibilityManager,
                        const FillEventInfoParam& param,
                        const RefPtr<NG::WebPattern>& webPattern)
{
    std::list<AccessibilityElementInfo> elementInfos;
    int32_t mode = 0;
    CHECK_NULL_VOID(jsAccessibilityManager);
    jsAccessibilityManager->SearchWebElementInfoByAccessibilityIdNG(
        elementId, mode, elementInfos, context, webPattern);
    if (elementInfos.empty()) {
        LOGE("Element infos is empty. Find element infos failed.");
        return;
    }
    elementInfo = elementInfos.front();
    elementInfo.SetWindowId(param.windowId);
}

void FillWebEventInfo(AccessibilityEventInfo& eventInfo,
                      const RefPtr<PipelineBase>& context,
                      const RefPtr<JsAccessibilityManager>& jsAccessibilityManager,
                      const FillEventInfoParam& param,
                      const RefPtr<NG::WebPattern>& webPattern)
{
    CHECK_NULL_VOID(webPattern);
    auto webNode = webPattern->GetHost();
    CHECK_NULL_VOID(webNode);
    eventInfo.SetPageId(webNode->GetPageId());
    AccessibilityElementInfo elementInfo;
    FillWebElementInfo(param.elementId, elementInfo, context, jsAccessibilityManager, param, webPattern);
    eventInfo.SetComponentType(elementInfo.GetComponentType());
    eventInfo.AddContent(elementInfo.GetContent());
    eventInfo.SetItemCounts(elementInfo.GetItemCounts());
    eventInfo.SetBeginIndex(elementInfo.GetBeginIndex());
    eventInfo.SetEndIndex(elementInfo.GetEndIndex());
    elementInfo.SetNavDestinationId(param.stackNodeId);
    eventInfo.SetElementInfo(elementInfo);
}
#endif

void FillEventInfo(const RefPtr<AccessibilityNode>& node, AccessibilityEventInfo& eventInfo)
{
    eventInfo.SetComponentType(node->GetTag());
    if (node->GetTag() == LIST_TAG) {
        eventInfo.SetItemCounts(node->GetListItemCounts());
        eventInfo.SetBeginIndex(node->GetListBeginIndex());
        eventInfo.SetEndIndex(node->GetListEndIndex());
    }
    eventInfo.SetPageId(node->GetPageId());
    eventInfo.AddContent(node->GetText());
    eventInfo.SetLatestContent(node->GetText());
}

inline bool IsPopupSupported(const RefPtr<NG::PipelineContext>& pipeline, int64_t nodeId)
{
    CHECK_NULL_RETURN(pipeline, false);
    auto overlayManager = pipeline->GetOverlayManager();
    if (overlayManager) {
        return overlayManager->HasPopupInfo(nodeId);
    }
    return false;
}

void SetAccessibilityFocusAction(AccessibilityElementInfo& nodeInfo, const char* tag)
{
    if (nodeInfo.HasAccessibilityFocus()) {
        AccessibleAction action(ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS, tag);
        nodeInfo.AddAction(action);
    } else {
        AccessibleAction action(ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS, tag);
        nodeInfo.AddAction(action);
    }
}

void UpdateSupportAction(const RefPtr<NG::FrameNode>& node, AccessibilityElementInfo& nodeInfo)
{
    CHECK_NULL_VOID(node);
    if (nodeInfo.IsFocusable()) {
        if (nodeInfo.IsFocused()) {
            AccessibleAction action(ACCESSIBILITY_ACTION_CLEAR_FOCUS, "ace");
            nodeInfo.AddAction(action);
        } else {
            AccessibleAction action(ACCESSIBILITY_ACTION_FOCUS, "ace");
            nodeInfo.AddAction(action);
        }
    }

    auto eventHub = node->GetEventHub<NG::EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto gestureEventHub = eventHub->GetGestureEventHub();
    CHECK_NULL_VOID(gestureEventHub);
    nodeInfo.SetClickable(gestureEventHub->IsAccessibilityClickable());
    if (gestureEventHub->IsAccessibilityClickable()) {
        AccessibleAction action(ACCESSIBILITY_ACTION_CLICK, "ace");
        nodeInfo.AddAction(action);
    }
    nodeInfo.SetLongClickable(gestureEventHub->IsAccessibilityLongClickable());
    if (gestureEventHub->IsAccessibilityLongClickable()) {
        AccessibleAction action(ACCESSIBILITY_ACTION_LONG_CLICK, "ace");
        nodeInfo.AddAction(action);
    }
}

bool IsUserCheckedOrSelected(const RefPtr<NG::FrameNode> frameNode)
{
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, false);
    if (accessibilityProperty->HasUserCheckedType() || accessibilityProperty->HasUserSelected()) {
        return true;
    }
    return false;
}

void UpdateAccessibilityTextValueInfo(
    RefPtr<NG::AccessibilityProperty>& accessibilityProperty, AccessibilityElementInfo& nodeInfo)
{
    auto customProperty = accessibilityProperty->GetCustomAccessibilityProperty();
    if (customProperty && !customProperty->GetAccessibilityText().empty()) {
        nodeInfo.SetAccessibilityText(customProperty->GetAccessibilityText());
    } else {
        nodeInfo.SetAccessibilityText(NG::AccessibilityPropertyUtils::GetAccessibilityText(accessibilityProperty));
    }
    if (customProperty && !customProperty->GetText().empty()) {
        nodeInfo.SetContent(customProperty->GetText());
    } else {
        nodeInfo.SetContent(NG::AccessibilityPropertyUtils::GetContent(accessibilityProperty));
    }
    nodeInfo.SetOriginalText(accessibilityProperty->GetText());
}

void UpdateElementInfoPageIdWithTreeId(Accessibility::AccessibilityElementInfo& info, int32_t treeId)
{
    int32_t pageId = info.GetPageId();
    if ((pageId >= MAX_PAGE_ID_WITH_SUB_TREE) || (pageId < 0)) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "pageId %{public}d cannot set tree id", pageId);
    } else {
        uint32_t unsignedPageId = static_cast<uint32_t>(pageId);
        uint32_t unsignedTreeId = static_cast<uint32_t>(treeId);
        info.SetPageId((unsignedTreeId << SUB_TREE_OFFSET_IN_PAGE_ID) | unsignedPageId);
    }
}

int32_t GetLastPageId(const RefPtr<NG::PipelineContext>& ngPipeline)
{
    CHECK_NULL_RETURN(ngPipeline, -1);
    auto stageManager = ngPipeline->GetStageManager();
    CHECK_NULL_RETURN(stageManager, -1);
    auto page = stageManager->GetLastPageWithTransition();
    CHECK_NULL_RETURN(page, -1);
    return page->GetPageId();
}

void AddHasRegisteredHover(const RefPtr<NG::FrameNode>& node, ExtraElementInfo& extraElementInfo)
{
    CHECK_NULL_VOID(node);
    auto eventHub = node->GetEventHub<NG::EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto inputEventHub = eventHub->GetInputEventHub();
    CHECK_NULL_VOID(inputEventHub);
    if (inputEventHub->HasAccessibilityHoverEvent()) {
        extraElementInfo.SetExtraElementInfo("hasRegisteredHover", true);
    }
}

void UpdateWebEmbedParent(std::list<AccessibilityElementInfo>& infos,
    const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty)
{
    if (infos.empty() || !commonProperty.checkGetFunc) {
        return;
    }
    auto webEmbedNodeId = commonProperty.checkGetFunc(node);
    CHECK_EQUAL_VOID(webEmbedNodeId, INVALID_NODE_ID);
    auto& frontElementInfo = infos.front();
    frontElementInfo.SetParent(webEmbedNodeId);
}

void InitAccessibilityEnabledAndScreenReadEnabled()
{
    auto client = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_VOID(client);
    bool isEnabled = false;
    client->IsEnabled(isEnabled);
    AceApplicationInfo::GetInstance().SetAccessibilityEnabled(isEnabled);
    bool isScreenReadEnabled = false;
    client->IsScreenReaderEnabled(isScreenReadEnabled);
    AceApplicationInfo::GetInstance().SetAccessibilityScreenReadEnabled(isScreenReadEnabled);
}

void UpdateFocusRectToRenderContext(
    const RefPtr<NG::FrameNode>& frameNode,
    const RefPtr<NG::RenderContext>& renderContext,
    const NG::RectT<int32_t>& rectInt)
{
    CHECK_NULL_VOID(frameNode);
    auto isVirtualNode = frameNode->IsAccessibilityVirtualNode();
    if (!isVirtualNode) {
        CHECK_NULL_VOID(renderContext);
        renderContext->UpdateAccessibilityFocusRect(rectInt);
        return;
    }
    auto weakNode = frameNode->GetVirtualNodeParent();
    auto refUiNode = weakNode.Upgrade();
    CHECK_NULL_VOID(refUiNode);
    auto parentNode = AceType::DynamicCast<NG::FrameNode>(refUiNode);
    CHECK_NULL_VOID(parentNode);
    auto parentRenderContext = parentNode->GetRenderContext();
    CHECK_NULL_VOID(parentRenderContext);
    parentRenderContext->UpdateAccessibilityFocusRect(rectInt);
}

void CheckAndReConnectA11ySA()
{
    auto client = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_VOID(client);
    CHECK_NE_VOID(client->NeedToConnect(), true);
    client->ConnectAndInit();
}

} // namespace

bool IsExtensionComponent(const RefPtr<NG::UINode>& node)
{
    return node && (node->GetTag() == V2::UI_EXTENSION_COMPONENT_ETS_TAG
        || node->GetTag() == V2::EMBEDDED_COMPONENT_ETS_TAG
        || node->GetTag() == V2::ISOLATED_COMPONENT_ETS_TAG);
}

bool IsUIExtensionShowPlaceholder(const RefPtr<NG::UINode>& node)
{
    CHECK_NULL_RETURN(node, true);
    if (node->GetTag() == V2::ISOLATED_COMPONENT_ETS_TAG) {
        return false;
    }
#ifdef WINDOW_SCENE_SUPPORTED
    auto pipeline = node->GetContextRefPtr();
    CHECK_NULL_RETURN(pipeline, true);
    auto manager = pipeline->GetUIExtensionManager();
    CHECK_NULL_RETURN(manager, true);
    return manager->IsShowPlaceholder(node->GetId());
#endif
    return true;
}

void GetFrameNodeChildren(
    const RefPtr<NG::UINode>& uiNode,
    std::vector<std::pair<int64_t, int32_t>>& childrenIdInfo,
    const CommonProperty& commonProperty);

namespace {
// Eligibility check for one frame node (id/z-index collection variant).
bool CollectEligibleFrameNodeIdInfo(const RefPtr<NG::UINode>& uiNode,
    const RefPtr<NG::FrameNode>& frameNode,
    std::vector<std::pair<int64_t, int32_t>>& childrenIdInfo, const CommonProperty& commonProperty)
{
    if (!frameNode) {
        return false;
    }
    if (!frameNode->IsFirstVirtualNode()) {
        CHECK_NULL_RETURN(frameNode->IsActive(), true);
    }
    if (uiNode->GetTag() == "stage") {
        return false;
    }
    if (uiNode->GetTag() == "page") {
        return !IsInPageNodes(uiNode->GetPageId(), commonProperty.pageNodes);
    }
    if (!frameNode->IsInternal() || frameNode->IsFirstVirtualNode()) {
        if (CheckFrameNodeByAccessibilityLevel(frameNode, false) &&
            CheckAndSetAccessibilityVisible(frameNode, commonProperty.isReduceMode)) {
            auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
            childrenIdInfo.emplace_back(
                std::make_pair(frameNode->GetAccessibilityId(),
                    accessibilityProperty ? accessibilityProperty->GetAccessibilityZIndex() : -1));
            return true;
        }
    }
    return false;
}

// Overlay and accessibility-virtual-node branches (id/z-index variant).
bool CollectOverlayAndVirtualChildrenIdInfo(const RefPtr<NG::FrameNode>& frameNode,
    std::vector<std::pair<int64_t, int32_t>>& childrenIdInfo, const CommonProperty& commonProperty)
{
    auto overlayNode = frameNode->GetOverlayNode();
    if (overlayNode) {
        GetFrameNodeChildren(overlayNode, childrenIdInfo, commonProperty);
    }

    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    auto uiVirtualNode = accessibilityProperty->GetAccessibilityVirtualNode();
    if (uiVirtualNode != nullptr) {
        auto virtualNode = AceType::DynamicCast<NG::FrameNode>(uiVirtualNode);
        if (virtualNode != nullptr) {
            GetFrameNodeChildren(virtualNode, childrenIdInfo, commonProperty);
            return true;
        }
    }
    return false;
}
} // namespace

void GetFrameNodeChildren(
    const RefPtr<NG::UINode>& uiNode,
    std::vector<std::pair<int64_t, int32_t>>& childrenIdInfo,
    const CommonProperty& commonProperty)
{
    CHECK_NULL_VOID(uiNode);
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(uiNode);
    // Exported-subtree roots are owned by their CMP binding's operator (T_sub path): cut them from any other tree's.
    if (OHOS::Ace::CmpAccessibility::CmpAccessibilityRegistry::IsRegisteredExportedRoot(
        frameNode.GetRawPtr())) {
        return;
    }
    if (IsValidEmbedTarget(frameNode, commonProperty)) {
        return;
    }

    if (CollectEligibleFrameNodeIdInfo(uiNode, frameNode, childrenIdInfo, commonProperty)) {
        return;
    }
    if (frameNode && CollectOverlayAndVirtualChildrenIdInfo(frameNode, childrenIdInfo, commonProperty)) {
        return;
    }

    for (const auto& frameChild : uiNode->GetChildren(true)) {
        GetFrameNodeChildren(frameChild, childrenIdInfo, commonProperty);
    }
}

struct ActionTable {
    AceAction aceAction;
    Accessibility::ActionType action;
};

struct ActionStrTable {
    Accessibility::ActionType action;
    std::string actionStr;
};

ActionType JsAccessibilityManager::ConvertAceAction(AceAction aceAction)
{
    static const ActionTable actionTable[] = {
        { AceAction::ACTION_CLICK, ActionType::ACCESSIBILITY_ACTION_CLICK },
        { AceAction::ACTION_LONG_CLICK, ActionType::ACCESSIBILITY_ACTION_LONG_CLICK },
        { AceAction::ACTION_SCROLL_FORWARD, ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD },
        { AceAction::ACTION_SCROLL_BACKWARD, ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD },
        { AceAction::ACTION_FOCUS, ActionType::ACCESSIBILITY_ACTION_FOCUS },
        { AceAction::ACTION_CLEAR_FOCUS, ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS },
        { AceAction::ACTION_ACCESSIBILITY_FOCUS, ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS },
        { AceAction::ACTION_CLEAR_ACCESSIBILITY_FOCUS, ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS },
        { AceAction::ACTION_NEXT_AT_MOVEMENT_GRANULARITY, ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT },
        { AceAction::ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY, ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT },
        { AceAction::ACTION_SET_TEXT, ActionType::ACCESSIBILITY_ACTION_SET_TEXT },
        { AceAction::ACTION_COPY, ActionType::ACCESSIBILITY_ACTION_COPY },
        { AceAction::ACTION_PASTE, ActionType::ACCESSIBILITY_ACTION_PASTE },
        { AceAction::ACTION_CUT, ActionType::ACCESSIBILITY_ACTION_CUT },
        { AceAction::ACTION_SELECT, ActionType::ACCESSIBILITY_ACTION_SELECT },
        { AceAction::ACTION_CLEAR_SELECTION, ActionType::ACCESSIBILITY_ACTION_CLEAR_SELECTION },
        { AceAction::ACTION_SET_SELECTION, ActionType::ACCESSIBILITY_ACTION_SET_SELECTION },
        { AceAction::ACTION_SET_CURSOR_POSITION, ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION },
        { AceAction::ACTION_EXEC_SUB_COMPONENT, ActionType::ACCESSIBILITY_ACTION_SPAN_CLICK },
        { AceAction::ACTION_NEXT_HTML_ITEM, ActionType::ACCESSIBILITY_ACTION_NEXT_HTML_ITEM },
        { AceAction::ACTION_PREVIOUS_HTML_ITEM, ActionType::ACCESSIBILITY_ACTION_PREVIOUS_HTML_ITEM },
    };
    for (const auto& item : actionTable) {
        if (aceAction == item.aceAction) {
            return item.action;
        }
    }
    return ActionType::ACCESSIBILITY_ACTION_INVALID;
}

RefPtr<NG::FrameNode> JsAccessibilityManager::GetNextFocusNodeByManager(
    const RefPtr<NG::UINode>& currentNode,
    const RefPtr<NG::FrameNode>& rootNode)
{
    CHECK_NULL_RETURN(currentNode, nullptr);
    CHECK_NULL_RETURN(rootNode, nullptr);
    return GetAccessibilityNextFocusNode(currentNode, rootNode);
}

RefPtr<NG::FrameNode> JsAccessibilityManager::GetPrevFocusNodeByManager(
    const RefPtr<NG::UINode>& currentNode,
    const RefPtr<NG::FrameNode>& rootNode,
    const RefPtr<PipelineBase>& context)
{
    CHECK_NULL_RETURN(currentNode, nullptr);
    CHECK_NULL_RETURN(rootNode, nullptr);
    CHECK_NULL_RETURN(context, nullptr);
    return GetAccessibilityPrevFocusNode(
        currentNode, rootNode, nextFocusRelationController_.GetTargetMap(context->GetInstanceId()));
}


bool JsAccessibilityManager::IsTagInEmbedComponent(const std::string& tag)
{
    return TAGS_EMBED_COMPONENT.find(tag) != TAGS_EMBED_COMPONENT.end();
}

void JsAccessibilityManager::UpdateAccessibilityElementInfo(
    const RefPtr<NG::FrameNode>& node, AccessibilityElementInfo& nodeInfo)
{
    CHECK_NULL_VOID(node);
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    auto customProperty = accessibilityProperty->GetCustomAccessibilityProperty();
    if (customProperty && !customProperty->GetRole().empty()) {
        nodeInfo.SetComponentType(customProperty->GetRole());
    } else if (accessibilityProperty->HasAccessibilityRole()) {
        nodeInfo.SetComponentType(accessibilityProperty->GetAccessibilityRole());
    }
    if (customProperty && !customProperty->GetAccessibilityRole().empty()) {
        nodeInfo.SetCustomComponentType(customProperty->GetAccessibilityRole());
    } else if (accessibilityProperty->HasAccessibilityCustomRole()) {
        nodeInfo.SetCustomComponentType(accessibilityProperty->GetAccessibilityCustomRole());
    }

    UpdateAccessibilityTextValueInfo(accessibilityProperty, nodeInfo);
    RangeInfo rangeInfo = ConvertAccessibilityValue(accessibilityProperty->GetAccessibilityValue());
    if (accessibilityProperty->HasUserRangeCurrentValue()) {
        rangeInfo.SetCurrent(accessibilityProperty->GetUserRangeCurrentValue());
    }
    if (accessibilityProperty->HasUserRangeMinValue()) {
        rangeInfo.SetMin(accessibilityProperty->GetUserRangeMinValue());
    }
    if (accessibilityProperty->HasUserRangeMaxValue()) {
        rangeInfo.SetMax(accessibilityProperty->GetUserRangeMaxValue());
    }
    nodeInfo.SetRange(rangeInfo);
    if (accessibilityProperty->HasSubComponent()) {
        std::vector<SubComponentInfo> subComponentInfos;
        accessibilityProperty->GetSubComponentInfo(subComponentInfos);
        for (const auto& subComponent : subComponentInfos) {
            nodeInfo.AddSpan(SpanInfo(subComponent.spanId, subComponent.spanText,
                subComponent.accessibilityText, subComponent.accessibilityDescription,
                subComponent.accessibilityLevel));
        }
    }
    nodeInfo.SetAccessibilityNextFocusInspectorKey(accessibilityProperty->GetAccessibilityNextFocusInspectorKey());
    nodeInfo.SetAccessibilityScrollable(accessibilityProperty->IsUserScrollTriggerable());
    nodeInfo.SetHint(accessibilityProperty->GetHintText());

    if (customProperty) {
        nodeInfo.SetSourceType(AccessibilitySourceType::UPDATED_FROM_ACCESSIBILITY_VIRTUAL_NODE);
        nodeInfo.SetAccessibilityGroup(customProperty->GetAccessibilityGroup());
    } else {
        nodeInfo.SetAccessibilityGroup(accessibilityProperty->IsAccessibilityGroup());
    }

    if (customProperty && !customProperty->GetAccessibilityLevel().empty()) {
        nodeInfo.SetAccessibilityLevel(customProperty->GetAccessibilityLevel());
    } else {
        nodeInfo.SetAccessibilityLevel(accessibilityProperty->GetAccessibilityLevel());
    }

    nodeInfo.SetAccessibilityStateDescription(accessibilityProperty->GetAccessibilityStateDescription());
    nodeInfo.SetTextType(accessibilityProperty->GetTextType());
    nodeInfo.SetTextLengthLimit(accessibilityProperty->GetTextLengthLimit());
    nodeInfo.SetOffset(accessibilityProperty->GetScrollOffSet());
    auto& context = node->GetRenderContext();
    if (context != nullptr) {
        nodeInfo.SetZIndex(context->GetZIndex().value_or(0));
        nodeInfo.SetOpacity(context->GetOpacity().value_or(1));
        nodeInfo.SetBackgroundColor(context->GetBackgroundColor().value_or(Color::TRANSPARENT).ToString());
        nodeInfo.SetBackgroundImage(context->GetBackgroundImage().value_or(ImageSourceInfo("")).ToString());
        if (context->GetForeground() != nullptr) {
            nodeInfo.SetBlur(context->GetForeground()->propBlurRadius.value_or(Dimension(0)).ToString());
        }
    }
    auto eventHub = node->GetEventHub<NG::EventHub>();
    if (eventHub != nullptr) {
        nodeInfo.SetHitTestBehavior(NG::GestureEventHub::GetHitTestModeStr(eventHub->GetGestureEventHub()));
    }

    UpdateUserAccessibilityElementInfo(accessibilityProperty, nodeInfo);

    nodeInfo.SetPassword(accessibilityProperty->IsPassword());
    nodeInfo.SetPluraLineSupported(accessibilityProperty->IsMultiLine());
    nodeInfo.SetHinting(accessibilityProperty->IsHint());
    nodeInfo.SetDescriptionInfo(accessibilityProperty->GetAccessibilityDescription());
    if (accessibilityProperty->HasUserCurrentValue()) {
        nodeInfo.SetCurrentIndex(accessibilityProperty->GetUserCurrentValue());
    } else {
        nodeInfo.SetCurrentIndex(accessibilityProperty->GetCurrentIndex());
    }
    if (accessibilityProperty->HasUserMinValue()) {
        nodeInfo.SetBeginIndex(accessibilityProperty->GetUserMinValue());
    } else {
        nodeInfo.SetBeginIndex(accessibilityProperty->GetBeginIndex());
    }
    if (accessibilityProperty->HasUserMaxValue()) {
        nodeInfo.SetEndIndex(accessibilityProperty->GetUserMaxValue());
    } else {
        nodeInfo.SetEndIndex(accessibilityProperty->GetEndIndex());
    }
    auto tag = node->GetTag();
    if (tag == V2::TOAST_ETS_TAG || tag == V2::POPUP_ETS_TAG || tag == V2::DIALOG_ETS_TAG ||
        tag == V2::ACTION_SHEET_DIALOG_ETS_TAG || tag == V2::ALERT_DIALOG_ETS_TAG || tag == V2::MENU_ETS_TAG ||
        tag == "SelectMenu") {
        nodeInfo.SetLiveRegion(1);
    }
    nodeInfo.SetContentInvalid(accessibilityProperty->GetContentInvalid());
    nodeInfo.SetError(accessibilityProperty->GetErrorText());
    nodeInfo.SetSelectedBegin(accessibilityProperty->GetTextSelectionStart());
    nodeInfo.SetSelectedEnd(accessibilityProperty->GetTextSelectionEnd());
    nodeInfo.SetInputType(static_cast<int>(accessibilityProperty->GetTextInputType()));
    nodeInfo.SetItemCounts(accessibilityProperty->GetCollectionItemCounts());
    nodeInfo.SetChildTreeIdAndWinId(
        accessibilityProperty->GetChildTreeId(), accessibilityProperty->GetChildWindowId());
    if (nodeInfo.GetComponentType() == "FormComponent") {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "elementId: %{public}" PRId64 ", Set: %{public}d %{public}d",
            nodeInfo.GetAccessibilityId(), accessibilityProperty->GetChildTreeId(),
            accessibilityProperty->GetChildWindowId());
    }
    if (nodeInfo.GetWindowId() == static_cast<int32_t>(windowId_)) {
        nodeInfo.SetBelongTreeId(treeId_);
        nodeInfo.SetParentWindowId(parentWindowId_);
    } else {
        nodeInfo.SetBelongTreeId(0);
        nodeInfo.SetParentWindowId(0);
    }

    GridInfo gridInfo(accessibilityProperty->GetCollectionInfo().rows,
        accessibilityProperty->GetCollectionInfo().columns, accessibilityProperty->GetCollectionInfo().selectMode);
    nodeInfo.SetGrid(gridInfo);
    ExtraElementInfo extraElementInfo {};
    accessibilityProperty->GetAllExtraElementInfo(extraElementInfo);
    AddHasRegisteredHover(node, extraElementInfo);
    if (node->IsAccessibilityVirtualNode()) {
        auto parentUinode = node->GetVirtualNodeParent().Upgrade();
        if (parentUinode) {
            auto parentFrame = AceType::DynamicCast<NG::FrameNode>(parentUinode);
            AddHasRegisteredHover(parentFrame, extraElementInfo);
        }
    }
    nodeInfo.SetExtraElement(extraElementInfo);

    // Set custom action list for accessibility
    auto customActions = accessibilityProperty->GetAccessibilityCustomActions();
    if (!customActions.empty()) {
        std::vector<std::string> customActionNames;
        customActionNames.reserve(customActions.size());
        for (const auto& customAction : customActions) {
            customActionNames.push_back(customAction.actionName);
        }
        nodeInfo.SetCustomActionList(customActionNames);
    }

    int32_t row = accessibilityProperty->GetCollectionItemInfo().row;
    int32_t column = accessibilityProperty->GetCollectionItemInfo().column;
    int32_t rowSpan = accessibilityProperty->GetCollectionItemInfo().rowSpan;
    int32_t columnSpan = accessibilityProperty->GetCollectionItemInfo().columnSpan;
    bool heading = accessibilityProperty->GetCollectionItemInfo().heading;
    GridItemInfo gridItemInfo(row, rowSpan, column, columnSpan, heading, nodeInfo.IsSelected());
    nodeInfo.SetGridItem(gridItemInfo);

    SetAccessibilityFocusAction(nodeInfo, "ace");
    if (nodeInfo.IsEnabled()) {
        nodeInfo.SetScrollable(accessibilityProperty->IsScrollable());
        nodeInfo.SetEditable(accessibilityProperty->IsEditable());
        nodeInfo.SetDeletable(accessibilityProperty->IsDeletable());
        UpdateSupportAction(node, nodeInfo);
        accessibilityProperty->ResetSupportAction();
        auto supportAceActions = accessibilityProperty->GetSupportAction();
        for (auto it = supportAceActions.begin(); it != supportAceActions.end(); ++it) {
            AccessibleAction action(ConvertAceAction(*it), "ace");
            nodeInfo.AddAction(action);
        }
    }
    CheckStateTakeOver(node, nodeInfo);
    CheckActionTakeOver(node, nodeInfo);
    UpdateHasChildText(node, nodeInfo);
}
#ifdef WEB_SUPPORTED

void JsAccessibilityManager::UpdateWebAccessibilityElementInfo(
    const std::shared_ptr<NG::TransitionalNodeInfo>& node, AccessibilityElementInfo& nodeInfo, int32_t treeId)
{
    CHECK_NULL_VOID(node);
    nodeInfo.SetContent(node->GetContent());
    nodeInfo.SetOriginalText(node->GetContent());
    nodeInfo.SetAccessibilityText(node->GetContent());
    RangeInfo rangeInfo(node->GetRangeInfoMin(), node->GetRangeInfoMax(), node->GetRangeInfoCurrent());
    nodeInfo.SetRange(rangeInfo);
    nodeInfo.SetHint(node->GetHint());
    nodeInfo.SetHinting(node->GetIsHinting());
    nodeInfo.SetDescriptionInfo(node->GetDescriptionInfo());
    nodeInfo.SetChecked(node->GetIsChecked());
    nodeInfo.SetSelected(node->GetIsSelected());
    nodeInfo.SetPassword(node->GetIsPassword());
    nodeInfo.SetPluraLineSupported(node->GetIsPluralLineSupported());
    nodeInfo.SetLiveRegion(node->GetLiveRegion());
    nodeInfo.SetContentInvalid(node->GetIsContentInvalid());
    nodeInfo.SetError(node->GetError());
    nodeInfo.SetSelectedBegin(node->GetSelectionStart());
    nodeInfo.SetSelectedEnd(node->GetSelectionEnd());
    nodeInfo.SetInputType(node->GetInputType());
    nodeInfo.SetItemCounts(node->GetItemCounts());

    nodeInfo.SetBelongTreeId(treeId);
    nodeInfo.SetParentWindowId(parentWebWindowId_);
    GridInfo gridInfo(node->GetGridRows(), node->GetGridColumns(), node->GetGridSelectedMode());
    nodeInfo.SetGrid(gridInfo);

    int32_t row = node->GetGridItemRow();
    int32_t column = node->GetGridItemColumn();
    int32_t rowSpan = node->GetGridItemRowSpan();
    int32_t columnSpan = node->GetGridItemColumnSpan();
    bool heading = node->GetIsHeading();
    GridItemInfo gridItemInfo(row, rowSpan, column, columnSpan, heading, nodeInfo.IsSelected());
    nodeInfo.SetGridItem(gridItemInfo);

    SetAccessibilityFocusAction(nodeInfo, "web");
    nodeInfo.SetCheckable(node->GetIsCheckable());
    nodeInfo.SetScrollable(node->GetIsScrollable());
    nodeInfo.SetEditable(node->GetIsEditable());
    nodeInfo.SetDeletable(node->GetIsDeletable());
    nodeInfo.SetClickable(node->GetIsClickable());
    auto supportAceActions = node->GetActions();
    for (auto it = supportAceActions.begin(); it != supportAceActions.end(); ++it) {
        AccessibleAction action(ConvertAceAction(static_cast<AceAction>(*it)), "web");
        nodeInfo.AddAction(action);
    }
    nodeInfo.SetAccessibilityGroup(node->GetIsAccessibilityGroup());

    ExtraElementInfo extraElementInfo {};
    extraElementInfo.SetExtraElementInfo("componentTypeDescription", node->GetComponentTypeDescription());
    extraElementInfo.SetExtraElementInfo("CheckboxGroupSelectedStatus", node->GetCheckboxGroupSelectedStatus());
    extraElementInfo.SetExtraElementInfo("expandedState", node->GetExpandedState());
    nodeInfo.SetExtraElement(extraElementInfo);

    nodeInfo.SetZIndex(node->GetZIndex());
}

#endif
namespace {
void UpdateChildrenOfAccessibilityElementInfo(
    const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty, AccessibilityElementInfo& nodeInfo)
{
    if (!NeedUpdateChildrenOfAccessibilityElementInfo(node)) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "Node %{public}s not need update children"
            " of accessibilityElementInfo", node->GetTag().c_str());
        return;
    }
    if (!IsExtensionComponent(node) || IsUIExtensionShowPlaceholder(node)) {
        std::vector<std::pair<int64_t, int32_t>> childrenIdInfo;
        for (const auto& item : node->GetChildren(true)) {
            GetFrameNodeChildren(item, childrenIdInfo, commonProperty);
        }

        auto overlayNode = node->GetOverlayNode();
        if (overlayNode != nullptr) {
            GetFrameNodeChildren(overlayNode, childrenIdInfo, commonProperty);
        }

        auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
        auto uiVirtualNode = accessibilityProperty->GetAccessibilityVirtualNode();
        if (uiVirtualNode != nullptr) {
            auto virtualNode = AceType::DynamicCast<NG::FrameNode>(uiVirtualNode);
            if (virtualNode != nullptr) {
                childrenIdInfo.clear();
                GetFrameNodeChildren(virtualNode, childrenIdInfo, commonProperty);
            }
        }
        if (AceApplicationInfo::GetInstance().IsAccessibilityScreenReadEnabled()) {
            std::sort(childrenIdInfo.begin(), childrenIdInfo.end(),
                [](const auto& zIndexA, const auto& zIndexB) { return zIndexA.second < zIndexB.second; });
        }
        for (const auto& childrenIdPair : childrenIdInfo) {
            nodeInfo.AddChild(childrenIdPair.first);
        }
    }
}

void UpdateAccessibilityFocusState(const RefPtr<NG::FrameNode>& node, AccessibilityElementInfo& nodeInfo)
{
    CHECK_NULL_VOID(node);
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    nodeInfo.SetAccessibilityFocus(accessibilityProperty->GetAccessibilityFocusState());
}
}

void JsAccessibilityManager::UpdateVirtualNodeChildAccessibilityElementInfo(
    const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
    AccessibilityElementInfo& nodeParentInfo, AccessibilityElementInfo& nodeInfo,
    const RefPtr<NG::PipelineContext>& ngPipeline)
{
    CHECK_NULL_VOID(node);
    nodeInfo.SetParent(GetParentId(node));
    UpdateChildrenOfAccessibilityElementInfo(node, commonProperty, nodeInfo);

    nodeInfo.SetAccessibilityId(node->GetAccessibilityId());
    nodeInfo.SetComponentType(node->GetTag());
    nodeInfo.SetUniqueId(node->GetId());

    auto& focusHub = node->GetFocusHub();
    nodeInfo.SetEnabled(focusHub ? focusHub->IsEnabled() : true);
    nodeInfo.SetFocused(focusHub ? focusHub->IsCurrentFocus() : false);
    UpdateAccessibilityFocusState(node, nodeInfo);
    const std::string& inspectorId = node->HasInspectorId() ? node->GetInspectorIdValue() : "";
    nodeInfo.SetInspectorKey(inspectorId);
    auto visible = node->IsVisible();
    nodeInfo.SetVisible(visible);
    if (visible) {
        auto rect = node->GetVirtualNodeTransformRectRelativeToWindow();
        auto left = static_cast<int32_t>(rect.Left() + commonProperty.windowLeft);
        auto top = static_cast<int32_t>(rect.Top() + commonProperty.windowTop);
        auto right = static_cast<int32_t>(rect.Right() + commonProperty.windowLeft);
        auto bottom = static_cast<int32_t>(rect.Bottom() + commonProperty.windowTop);
        Accessibility::Rect bounds { left, top, right, bottom };
        nodeInfo.SetRectInScreen(bounds);
    }
    nodeInfo.SetWindowId(commonProperty.windowId);
    nodeInfo.SetInnerWindowId(commonProperty.innerWindowId);
    nodeInfo.SetPageId(node->GetPageId());
    nodeInfo.SetPagePath(
        GetPagePathInPageNodes(nodeInfo.GetPageId(), commonProperty.pageNodes, commonProperty.pagePaths));
    nodeInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());

    if (nodeInfo.IsEnabled()) {
        nodeInfo.SetFocusable(focusHub ? focusHub->IsFocusable() : false);
        nodeInfo.SetPopupSupported(IsPopupSupported(ngPipeline, node->GetId()));
    }
    nodeInfo.SetComponentResourceId(inspectorId);
    UpdateAccessibilityElementInfo(node, nodeInfo);
}

void JsAccessibilityManager::UpdateVirtualNodeAccessibilityElementInfo(
    const RefPtr<NG::FrameNode>& parent, const RefPtr<NG::FrameNode>& node,
    const CommonProperty& commonProperty, AccessibilityElementInfo& nodeInfo,
    const RefPtr<NG::PipelineContext>& ngPipeline)
{
    CHECK_NULL_VOID(parent);
    CHECK_NULL_VOID(node);
    nodeInfo.SetParent(GetParentId(node));
    UpdateChildrenOfAccessibilityElementInfo(node, commonProperty, nodeInfo);

    nodeInfo.SetAccessibilityId(node->GetAccessibilityId());
    nodeInfo.SetComponentType(node->GetTag());
    nodeInfo.SetUniqueId(node->GetId());

    nodeInfo.SetEnabled(node->GetFocusHub() ? node->GetFocusHub()->IsEnabled() : true);
    nodeInfo.SetFocused(node->GetFocusHub() ? node->GetFocusHub()->IsCurrentFocus() : false);
    UpdateAccessibilityFocusState(node, nodeInfo);
    nodeInfo.SetInspectorKey(node->GetInspectorId().value_or(""));
    nodeInfo.SetVisible(node->IsVisible());
    if (node->IsVisible()) {
        auto virtualNodeRect = node->GetTransformRectRelativeToWindow();
        auto parentRect = parent->GetTransformRectRelativeToWindow();
        auto left = static_cast<int32_t>(parentRect.Left() + commonProperty.windowLeft);
        auto top = static_cast<int32_t>(parentRect.Top() + commonProperty.windowTop);
        auto right = static_cast<int32_t>(parentRect.Left() + virtualNodeRect.Width() + commonProperty.windowLeft);
        if (virtualNodeRect.Width() > (parentRect.Right() - parentRect.Left())) {
            right = static_cast<int32_t>(parentRect.Right() + commonProperty.windowLeft);
        }
        auto bottom = static_cast<int32_t>(parentRect.Top() + virtualNodeRect.Height() + commonProperty.windowTop);
        if (virtualNodeRect.Height() > (parentRect.Bottom() - parentRect.Top())) {
            bottom = static_cast<int32_t>(parentRect.Bottom() + commonProperty.windowTop);
        }
        Accessibility::Rect bounds { left, top, right, bottom };
        nodeInfo.SetRectInScreen(bounds);
    }
    nodeInfo.SetWindowId(commonProperty.windowId);
    nodeInfo.SetInnerWindowId(commonProperty.innerWindowId);
    nodeInfo.SetPageId(node->GetPageId());
    nodeInfo.SetPagePath(
        GetPagePathInPageNodes(nodeInfo.GetPageId(), commonProperty.pageNodes, commonProperty.pagePaths));
    nodeInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());

    if (nodeInfo.IsEnabled()) {
        nodeInfo.SetFocusable(node->GetFocusHub() ? node->GetFocusHub()->IsFocusable() : false);
        nodeInfo.SetPopupSupported(IsPopupSupported(ngPipeline, node->GetId()));
    }
    nodeInfo.SetComponentResourceId(node->GetInspectorId().value_or(""));
    UpdateAccessibilityElementInfo(node, nodeInfo);
}

namespace {
void GetPositionToWindowWithTransform(NG::OffsetF& offset, NG::OffsetF& offsetBottom, const RefPtr<NG::FrameNode>& node)
{
    CHECK_NULL_VOID(node);
    auto& context = node->GetRenderContext();
    CHECK_NULL_VOID(context);
    auto rect = context->GetPaintRectWithoutTransform();

    offset = rect.GetOffset();
    offsetBottom = NG::OffsetF(rect.GetX() + rect.Width(), rect.GetY() + rect.Height());

    NG::PointF leftTopPointNode(offset.GetX(), offset.GetY());
    NG::PointF rightBottomPointNode(rect.GetX() + rect.Width(), rect.GetY() + rect.Height());
    context->GetPointTransformRotate(leftTopPointNode);
    context->GetPointTransformRotate(rightBottomPointNode);
    auto parent = node->GetAncestorNodeOfFrame(true);
    while (parent) {
        // if get the window boundary (window scene), not calculate transform
        if (!parent->IsWindowBoundary()) {
            auto& parentRenderContext = parent->GetRenderContext();
            if (!parentRenderContext) {
                parent = parent->GetAncestorNodeOfFrame(true);
                continue;
            }
            auto parentOffset = parentRenderContext->GetPaintRectWithoutTransform().GetOffset();
            NG::PointF leftTopPointAfterAddOffset(parentOffset.GetX() + leftTopPointNode.GetX(),
                                            parentOffset.GetY() + leftTopPointNode.GetY());
            NG::PointF rightBottomPointAfterAddOffset(parentOffset.GetX() + rightBottomPointNode.GetX(),
                                                parentOffset.GetY() + rightBottomPointNode.GetY());

            auto parentTransformMat = parentRenderContext->GetMatrixWithTransformRotate();
            Point leftTop(leftTopPointAfterAddOffset.GetX(), leftTopPointAfterAddOffset.GetY());
            Point rightBottom(rightBottomPointAfterAddOffset.GetX(), rightBottomPointAfterAddOffset.GetY());
            auto leftTopPoint = parentTransformMat * leftTop;
            auto rightBottomPoint = parentTransformMat * rightBottom;
            leftTopPointNode.SetX(leftTopPoint.GetX());
            leftTopPointNode.SetY(leftTopPoint.GetY());
            rightBottomPointNode.SetX(rightBottomPoint.GetX());
            rightBottomPointNode.SetY(rightBottomPoint.GetY());
        }
        parent = parent->GetAncestorNodeOfFrame(true);
    }
    offset.SetX(leftTopPointNode.GetX());
    offset.SetY(leftTopPointNode.GetY());
    offsetBottom.SetX(rightBottomPointNode.GetX());
    offsetBottom.SetY(rightBottomPointNode.GetY());
}

NG::RectF GetFinalRealRect(const RefPtr<NG::FrameNode>& node)
{
    if ((node->GetTag() == V2::WINDOW_SCENE_ETS_TAG) && node->IsWindowBoundary()) {
        auto& renderContext = node->GetRenderContext();
        CHECK_NULL_RETURN(renderContext, NG::RectF());
        auto rect = renderContext->GetPaintRectWithoutTransform();
        // commonproperty will contain the offset and scale of the window scene
        return {0, 0, rect.Width(), rect.Height()};
    }

    NG::OffsetF offset;
    NG::OffsetF offsetBottom;
    GetPositionToWindowWithTransform(offset, offsetBottom, node);
    return {
        LessNotEqual(offset.GetX(), offsetBottom.GetX()) ? offset.GetX() : offsetBottom.GetX(),
        LessNotEqual(offset.GetY(), offsetBottom.GetY()) ? offset.GetY() : offsetBottom.GetY(),
        LessNotEqual(offset.GetX(), offsetBottom.GetX())
            ? offsetBottom.GetX() - offset.GetX()
            : offset.GetX() - offsetBottom.GetX(),
        LessNotEqual(offset.GetY(), offsetBottom.GetY())
            ? offsetBottom.GetY() - offset.GetY()
            : offset.GetY() - offsetBottom.GetY()
        };
}

void SetRectInScreen(const RefPtr<NG::FrameNode>& node, AccessibilityElementInfo& nodeInfo,
    const CommonProperty& commonProperty)
{
    if (node->IsAccessibilityVirtualNode()) {
        auto rect = node->GetVirtualNodeTransformRectRelativeToWindow();
        auto left = static_cast<int32_t>(rect.Left() + commonProperty.windowLeft);
        auto top = static_cast<int32_t>(rect.Top() + commonProperty.windowTop);
        auto right = static_cast<int32_t>(rect.Right() + commonProperty.windowLeft);
        auto bottom = static_cast<int32_t>(rect.Bottom() + commonProperty.windowTop);
        Accessibility::Rect bounds { left, top, right, bottom };
        nodeInfo.SetRectInScreen(bounds);
    } else if (node->IsVisible()) {
        auto rect = GetFinalRealRect(node);
        auto rotateTransformData = commonProperty.rotateTransform;
        auto currentDegree = rotateTransformData.rotateDegree;
        if (!NearZero(currentDegree, 0)) {
            AccessibilityRect rotateRect(rect.GetX(), rect.GetY(), rect.Width(), rect.Height());
            rotateRect.Rotate(rotateTransformData.innerCenterX, rotateTransformData.innerCenterY, currentDegree);
            rotateRect.ApplyTransformation(rotateTransformData, commonProperty.scaleX, commonProperty.scaleY);
            Accessibility::Rect bounds { static_cast<int32_t>(rotateRect.GetX()),
                static_cast<int32_t>(rotateRect.GetY()),
                static_cast<int32_t>(rotateRect.GetX() + rotateRect.GetWidth()),
                static_cast<int32_t>(rotateRect.GetY() + rotateRect.GetHeight()) };
            nodeInfo.SetRectInScreen(bounds);
            return;
        }
        if (!NearZero(commonProperty.scaleX, 0) && !NearZero(commonProperty.scaleY, 0)) {
            rect.SetRect(rect.GetX() * commonProperty.scaleX, rect.GetY() * commonProperty.scaleY,
                rect.Width() * commonProperty.scaleX, rect.Height() * commonProperty.scaleY);
        }
        auto left = static_cast<int32_t>(rect.Left() + commonProperty.windowLeft);
        auto top = static_cast<int32_t>(rect.Top() + commonProperty.windowTop);
        auto right = static_cast<int32_t>(rect.Right() + commonProperty.windowLeft);
        auto bottom = static_cast<int32_t>(rect.Bottom() + commonProperty.windowTop);
        Accessibility::Rect bounds { left, top, right, bottom };
        nodeInfo.SetRectInScreen(bounds);
    }
}
}

void JsAccessibilityManager::UpdateAccessibilityVisible(
    const RefPtr<NG::FrameNode>& node, AccessibilityElementInfo& nodeInfo)
{
    if (AceApplicationInfo::GetInstance().IsAccessibilityScreenReadEnabled() || IsSelectReaderEnabled()) {
        NG::AccessibilityFrameNodeUtils::UpdateAccessibilityVisibleToRoot(node);
        nodeInfo.SetAccessibilityVisible(node->GetAccessibilityVisible());
        return;
    }

    auto parentNode = node->GetParentFrameNode();
    // Generation-time pass: id splicing only (screen-reader-off branch).
    UpdateElementInfoTreeId(nodeInfo, false);

    if (!parentNode) {
        if (node->GetTag() != V2::PAGE_ETS_TAG) {
            node->SetAccessibilityVisible(node->IsActive() && node->IsVisible());
        }
    } else {
        if (node->GetTag() == V2::PAGE_ETS_TAG) {
            nodeInfo.SetAccessibilityVisible(node->IsActive() && node->IsVisible() && node->GetAccessibilityVisible() &&
                                             parentNode->GetAccessibilityVisible());
            return;
        }
        auto nodeAccessibilityVisible = node->IsActive() && node->IsVisible() && parentNode->GetAccessibilityVisible();
        node->SetAccessibilityVisible(nodeAccessibilityVisible);
        if (!nodeAccessibilityVisible) {
            TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
                "Element %{public}" PRId64 " is invisible. node isActive %{public}d, node isVisible %{public}d"
                "parent accessibilityVisible:%{public}d.parent id %{public}" PRId64,
                nodeInfo.GetAccessibilityId(), node->IsActive(), node->IsVisible(),
                parentNode->GetAccessibilityVisible(), parentNode->GetAccessibilityId());
        }
    }
    nodeInfo.SetAccessibilityVisible(node->GetAccessibilityVisible());
}

void JsAccessibilityManager::UpdateAccessibilityElementInfo(
    const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
    AccessibilityElementInfo& nodeInfo, const RefPtr<NG::PipelineContext>& ngPipeline)
{
    CHECK_NULL_VOID(node);
    nodeInfo.SetParent(GetParentId(node));
    UpdateChildrenOfAccessibilityElementInfo(node, commonProperty, nodeInfo);

    nodeInfo.SetAccessibilityId(node->GetAccessibilityId());
    nodeInfo.SetComponentType(node->GetTag());
    nodeInfo.SetUniqueId(node->GetId());

    nodeInfo.SetEnabled(node->GetFocusHub() ? node->GetFocusHub()->IsEnabled() : true);
    nodeInfo.SetFocused(node->GetFocusHub() ? node->GetFocusHub()->IsCurrentFocus() : false);
    nodeInfo.SetAccessibilityFocus(GetAccessibilityFocusStateByFrameNode(node));
    nodeInfo.SetClip(node->GetRenderContext()->GetClipEdge().value_or(false));
    nodeInfo.SetInspectorKey(node->GetInspectorId().value_or(""));
    nodeInfo.SetVisible(node->IsVisible());
    nodeInfo.SetIsActive(node->IsActive());
    SetRectInScreen(node, nodeInfo, commonProperty);
    nodeInfo.SetWindowId(commonProperty.windowId);
    nodeInfo.SetInnerWindowId(commonProperty.innerWindowId);
    // is abnormal that pageId equals to 0, use last pageId to fix pageId
    if (node->GetPageId()) {
        nodeInfo.SetPageId(node->GetPageId());
    } else {
        nodeInfo.SetPageId(GetLastPageId(ngPipeline));
    }
    nodeInfo.SetPagePath(
        GetPagePathInPageNodes(nodeInfo.GetPageId(), commonProperty.pageNodes, commonProperty.pagePaths));
    nodeInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());

    if (nodeInfo.IsEnabled()) {
        nodeInfo.SetFocusable(node->GetFocusHub() ? node->GetFocusHub()->IsFocusable() : false);
        nodeInfo.SetPopupSupported(IsPopupSupported(ngPipeline, node->GetId()));
    }
    nodeInfo.SetComponentResourceId(node->GetInspectorId().value_or(""));
    UpdateAccessibilityElementInfo(node, nodeInfo);
    UpdateAccessibilityVisible(node, nodeInfo);
}

void JsAccessibilityManager::UpdateElementInfo(
    const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
    AccessibilityElementInfo& nodeInfo, const RefPtr<NG::PipelineContext>& ngPipeline)
{
    CHECK_NULL_VOID(node);
    if (node->IsAccessibilityVirtualNode()) {
        auto parentUinode = node->GetVirtualNodeParent().Upgrade();
        CHECK_NULL_VOID(parentUinode);
        auto parentFrame = AceType::DynamicCast<NG::FrameNode>(parentUinode);
        CHECK_NULL_VOID(parentFrame);
        UpdateVirtualNodeAccessibilityElementInfo(parentFrame, node, commonProperty, nodeInfo, ngPipeline);
    } else {
        UpdateAccessibilityElementInfo(node, commonProperty, nodeInfo, ngPipeline);
    }
}

#ifdef WEB_SUPPORTED

void JsAccessibilityManager::WebSetScreenRect(const std::shared_ptr<NG::TransitionalNodeInfo>& node,
    const CommonProperty& commonProperty, AccessibilityElementInfo& nodeInfo)
{
    auto rotateTransformData = commonProperty.rotateTransform;
    auto currentDegree = rotateTransformData.rotateDegree;
    if (!NearZero(currentDegree, 0)) {
        AccessibilityRect rotateRect(node->GetRectX(), node->GetRectY(),
            node->GetRectWidth(), node->GetRectHeight());
        rotateRect.Rotate(rotateTransformData.innerCenterX, rotateTransformData.innerCenterY, currentDegree);
        rotateRect.ApplyTransformation(rotateTransformData, commonProperty.scaleX, commonProperty.scaleY);
        Accessibility::Rect bounds { static_cast<int32_t>(std::floor(rotateRect.GetX())),
            static_cast<int32_t>(std::floor(rotateRect.GetY())),
            static_cast<int32_t>(std::floor(rotateRect.GetX() + rotateRect.GetWidth())),
            static_cast<int32_t>(std::floor(rotateRect.GetY() + rotateRect.GetHeight())) };
        nodeInfo.SetRectInScreen(bounds);
    } else {
        NG::RectT<int32_t> rectInt {
            node->GetRectX(),
            node->GetRectY(),
            node->GetRectWidth(),
            node->GetRectHeight()
        };
        if (!NearZero(commonProperty.scaleX) || !NearZero(commonProperty.scaleY)) {
            rectInt.SetRect(static_cast<int32_t>(std::floor(rectInt.GetX() * commonProperty.scaleX)),
                static_cast<int32_t>(std::floor(rectInt.GetY() * commonProperty.scaleY)),
                static_cast<int32_t>(std::floor(rectInt.Width() * commonProperty.scaleX)),
                static_cast<int32_t>(std::floor(rectInt.Height() * commonProperty.scaleY)));
        }
        auto left = static_cast<int32_t>(rectInt.Left() + commonProperty.windowLeft);
        auto top = static_cast<int32_t>(rectInt.Top() + commonProperty.windowTop);
        auto right = static_cast<int32_t>(rectInt.Right() + commonProperty.windowLeft);
        auto bottom = static_cast<int32_t>(rectInt.Bottom() + commonProperty.windowTop);
        Accessibility::Rect bounds { left, top, right, bottom };
        nodeInfo.SetRectInScreen(bounds);
    }
}

void JsAccessibilityManager::UpdateWebAccessibilityElementInfo(
    const std::shared_ptr<NG::TransitionalNodeInfo>& node, const CommonProperty& commonProperty,
    AccessibilityElementInfo& nodeInfo, const RefPtr<NG::WebPattern>& webPattern)
{
    if (node->GetParentId() == -1) {
        nodeInfo.SetParent(INVALID_PARENT_ID);
    } else {
        nodeInfo.SetParent(node->GetParentId());
    }
    for (const auto& child : node->GetChildIds()) {
        nodeInfo.AddChild(child);
    }

    nodeInfo.SetAccessibilityId(node->GetAccessibilityId());
    nodeInfo.SetComponentType(node->GetComponentType());
    if (IsTagInEmbedComponent(nodeInfo.GetComponentType())) {
        nodeInfo.SetEnabled(true);
    } else {
        nodeInfo.SetEnabled(node->GetIsEnabled());
    }
    nodeInfo.SetFocused(node->GetIsFocused());
    nodeInfo.SetAccessibilityFocus(node->GetIsAccessibilityFocus());
    nodeInfo.SetVisible(node->GetIsVisible());

    if (node->GetIsVisible()) {
        CHECK_NULL_VOID(webPattern);
        auto webNode = webPattern->GetHost();
        CHECK_NULL_VOID(webNode);
        WebSetScreenRect(node, commonProperty, nodeInfo);
    }

    nodeInfo.SetWindowId(commonProperty.windowId);
    nodeInfo.SetPageId(node->GetPageId());
    nodeInfo.SetPagePath(
        GetPagePathInPageNodes(nodeInfo.GetPageId(), commonProperty.pageNodes, commonProperty.pagePaths));
    nodeInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());

    if (nodeInfo.IsEnabled()) {
        nodeInfo.SetFocusable(node->GetIsFocusable());
        nodeInfo.SetPopupSupported(node->GetIsPopupSupported());
    }
    CHECK_NULL_VOID(webPattern);
    UpdateWebAccessibilityElementInfo(node, nodeInfo, webPattern->GetTreeId());
}

#endif
namespace {
std::list<AccessibilityElementInfo> SearchExtensionElementInfoByAccessibilityIdNG(
    int64_t elementId, int32_t mode, const RefPtr<NG::FrameNode>& node,  int64_t offset)
{
    std::list<AccessibilityElementInfo> extensionElementInfo;
    if (NG::UI_EXTENSION_OFFSET_MIN < (offset + 1)) {
        node->SearchExtensionElementInfoByAccessibilityIdNG(elementId, mode,
            offset / NG::UI_EXTENSION_ID_FACTOR, extensionElementInfo);
    }
    return extensionElementInfo;
}

void UpdateUiExtensionParentIdForFocus(const RefPtr<NG::FrameNode>& rootNode, const int64_t uiExtensionOffset,
    Accessibility::AccessibilityElementInfo& info)
{
    if ((uiExtensionOffset != NG::UI_EXTENSION_OFFSET_MAX) && (info.GetComponentType() != V2::ROOT_ETS_TAG) &&
        (info.GetParentNodeId() == rootNode->GetAccessibilityId())) {
            info.SetParent(NG::UI_EXTENSION_ROOT_ID);
    }
}

void GetChildrenFromFrameNode(
    const RefPtr<NG::FrameNode>& node,
    std::list<RefPtr<NG::FrameNode>>& children,
    const CommonProperty& commonProperty)
{
    std::list<RefPtr<NG::FrameNode>> frameNodeChildren;
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    auto uiVirtualNode = accessibilityProperty->GetAccessibilityVirtualNode();
    if (uiVirtualNode != nullptr) {
        auto virtualNode = AceType::DynamicCast<NG::FrameNode>(uiVirtualNode);
        if (virtualNode != nullptr) {
            GetFrameNodeChildren(virtualNode, frameNodeChildren, commonProperty);
        }
    } else {
        for (const auto& item : node->GetChildren(true)) {
            GetFrameNodeChildren(item, frameNodeChildren, commonProperty);
        }

        auto overlayNode = node->GetOverlayNode();
        if (overlayNode != nullptr) {
            GetFrameNodeChildren(overlayNode, frameNodeChildren, commonProperty);
        }
    }
    while (!frameNodeChildren.empty()) {
        children.emplace_back(frameNodeChildren.front());
        frameNodeChildren.pop_front();
    }
}

RefPtr<NG::FrameNode> GetLastChildFrameNode(const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty)
{
    std::list<RefPtr<NG::FrameNode>> children { node };
    RefPtr<NG::FrameNode> checkNode;

    while (!children.empty()) {
        checkNode = children.back();
        children.clear();
        GetChildrenFromFrameNode(checkNode, children, commonProperty);
    }
    return checkNode;
}
}

void JsAccessibilityManager::UpdateVirtualNodeInfo(std::list<AccessibilityElementInfo>& infos,
    AccessibilityElementInfo& nodeInfo, const RefPtr<NG::UINode>& uiVirtualNode, const CommonProperty& commonProperty,
    const RefPtr<NG::PipelineContext>& ngPipeline)
{
    auto frameParentNode = AceType::DynamicCast<NG::FrameNode>(uiVirtualNode);
    CHECK_NULL_VOID(frameParentNode);
    std::list<RefPtr<NG::FrameNode>> children;

    GetChildrenFromFrameNode(frameParentNode, children, commonProperty);
    for (const auto& frameNodeChild : children) {
        AccessibilityElementInfo virtualInfo;
        UpdateVirtualNodeChildAccessibilityElementInfo(frameNodeChild, commonProperty,
            nodeInfo, virtualInfo, ngPipeline);
        virtualInfo.SetParent(uiVirtualNode->GetAccessibilityId());
        nodeInfo.AddChild(frameNodeChild->GetAccessibilityId());
        UpdateVirtualNodeInfo(infos, virtualInfo, frameNodeChild, commonProperty, ngPipeline);
        infos.push_back(virtualInfo);
    }
}

namespace {
void SearchExtensionElementInfoNG(const SearchParameter& searchParam,
    const RefPtr<NG::FrameNode>& node, std::list<Accessibility::AccessibilityElementInfo>& infos,
    Accessibility::AccessibilityElementInfo& parentInfo)
{
    auto extensionElementInfos = SearchExtensionElementInfoByAccessibilityIdNG(
        searchParam.nodeId, searchParam.mode, node, searchParam.uiExtensionOffset);
    if (!extensionElementInfos.empty()) {
        auto rootParentId = extensionElementInfos.front().GetParentNodeId();
        ConvertExtensionAccessibilityNodeId(extensionElementInfos, node, searchParam.uiExtensionOffset, parentInfo);
        if (rootParentId == NG::UI_EXTENSION_ROOT_ID) {
            extensionElementInfos.front().SetParent(node->GetAccessibilityId());
        }
        if (parentInfo.GetComponentType() == V2::ISOLATED_COMPONENT_ETS_TAG) {
            auto windowId = parentInfo.GetWindowId();
            for (auto& info : extensionElementInfos) {
                info.SetWindowId(windowId);
            }
        }
        for (auto& info : extensionElementInfos) {
            infos.push_back(info);
        }
    }
}
}

bool JsAccessibilityManager::GetAllVirtualNodeElementId(
    std::list<AccessibilityElementInfo>& infos, const CommonProperty& commonProperty,
    const RefPtr<NG::FrameNode>& containerNode, AccessibilityElementInfo& parentNodeInfo)
{
    auto accessibilityProperty = containerNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, false);
    CHECK_NE_RETURN(accessibilityProperty->HasVirtualNodeTreeRoot(), true, false);

    auto virtualNodeTreeRoot = accessibilityProperty->GetVirtualNodeTreeRoot();
    CHECK_NULL_RETURN(virtualNodeTreeRoot, false);
    auto virtualNodeRoot = AceType::DynamicCast<NG::VirtualAccessibilityNode>(virtualNodeTreeRoot);
    CHECK_NULL_RETURN(virtualNodeRoot, false);
    AccessibilityElementInfo virtualNodeInfo;
    uint8_t containerId = NG::VirtualNodeContainerIdManager::GetInstance().GetContainerId(containerNode);
    int64_t virtualRootAccessibilityId =
        NG::VirtualNodeContainerIdManager::EncodeVirtualNodeAccessibilityId(containerId, virtualNodeRoot->GetNodeId());
    UpdateAccessibilityElementInfoForVirtualNode(containerNode, virtualNodeRoot,
        commonProperty, virtualNodeInfo, containerNode->GetAccessibilityId());

    auto childIds = parentNodeInfo.GetChildIds();
    for (auto& childId : childIds) {
        parentNodeInfo.RemoveChild(childId);
    }

    parentNodeInfo.AddChild(virtualRootAccessibilityId);
    infos.push_back(virtualNodeInfo);
    AddVirtualNodeChildrenToInfos(infos, containerNode,
        virtualNodeRoot, commonProperty, virtualRootAccessibilityId);
    return true;
}

bool JsAccessibilityManager::GetAllVirtualNodeElementInfo(std::list<AccessibilityElementInfo>& infos,
    const CommonProperty& commonProperty, const RefPtr<NG::PipelineContext>& ngPipeline,
    const RefPtr<NG::FrameNode>& containerNode, AccessibilityElementInfo& parentNodeInfo)
{
    CHECK_NULL_RETURN(containerNode, false);
    auto accessibilityProperty = containerNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, false);
    CHECK_NE_RETURN(accessibilityProperty->HasVirtualNodeTreeRoot(), true, false);
    auto virtualNodeTreeRoot = accessibilityProperty->GetVirtualNodeTreeRoot();
    CHECK_NULL_RETURN(virtualNodeTreeRoot, false);
    auto virtualNodeRoot = AceType::DynamicCast<NG::VirtualAccessibilityNode>(virtualNodeTreeRoot);
    CHECK_NULL_RETURN(virtualNodeRoot, false);

    AccessibilityElementInfo virtualNodeInfo;
    uint8_t containerId = NG::VirtualNodeContainerIdManager::GetInstance().GetContainerId(containerNode);
    int64_t virtualRootAccessibilityId =
        NG::VirtualNodeContainerIdManager::EncodeVirtualNodeAccessibilityId(containerId, virtualNodeRoot->GetNodeId());
    UpdateAccessibilityElementInfoForVirtualNode(containerNode, virtualNodeRoot,
        commonProperty, virtualNodeInfo, containerNode->GetAccessibilityId());

    auto childIds = parentNodeInfo.GetChildIds();
    for (auto& child : childIds) {
        parentNodeInfo.RemoveChild(child);
    }

    parentNodeInfo.AddChild(virtualRootAccessibilityId);
    infos.push_back(virtualNodeInfo);
    AddVirtualNodeChildrenToInfos(infos, containerNode,
        virtualNodeRoot, commonProperty, virtualRootAccessibilityId);
    return true;
}

namespace {
void ReplaceChildrenWithVirtualRoot(Accessibility::AccessibilityElementInfo& nodeInfo, int64_t virtualRootId)
{
    auto childIds = nodeInfo.GetChildIds();
    for (auto& child : childIds) {
        nodeInfo.RemoveChild(child);
    }
    nodeInfo.AddChild(virtualRootId);
}

void HandleExtensionChildNode(std::list<Accessibility::AccessibilityElementInfo>& infos,
    const RefPtr<NG::FrameNode>& frameNodeParent, const SearchParameter& searchParam,
    Accessibility::AccessibilityElementInfo& nodeInfo)
{
    if (!((frameNodeParent->GetUiExtensionId() > NG::UI_EXTENSION_UNKNOW_ID) &&
        (((frameNodeParent->GetUiExtensionId() <= NG::UI_EXTENSION_ID_FIRST_MAX) &&
        (NG::UI_EXTENSION_OFFSET_MAX == searchParam.uiExtensionOffset)) ||
        (frameNodeParent->GetUiExtensionId() <= NG::UI_EXTENSION_ID_OTHER_MAX)))) {
        return;
    }
    auto transferParam = searchParam;
    transferParam.nodeId = NG::UI_EXTENSION_ROOT_ID;
    SearchExtensionElementInfoNG(transferParam, frameNodeParent, infos, nodeInfo);
    infos.push_back(nodeInfo);
}
} // namespace

void JsAccessibilityManager::UpdateChildrenNodeInCache(std::list<AccessibilityElementInfo>& infos,
    const CommonProperty& commonProperty, const RefPtr<NG::PipelineContext>& ngPipeline,
    const SearchParameter& searchParam, std::list<RefPtr<NG::FrameNode>>& children)
{
    while (!children.empty()) {
        RefPtr<NG::FrameNode> frameNodeParent = children.front();
        children.pop_front();
        AccessibilityElementInfo nodeInfo;
        auto accessibilityProperty = frameNodeParent->GetAccessibilityProperty<NG::AccessibilityProperty>();
        auto uiVirtualNode = accessibilityProperty->GetAccessibilityVirtualNode();
        if (CheckAndSetAccessibilityVisible(frameNodeParent, commonProperty.isReduceMode)) {
            UpdateAccessibilityElementInfo(frameNodeParent, commonProperty, nodeInfo, ngPipeline);
        }
        if (uiVirtualNode != nullptr) {
            auto virtualNode = AceType::DynamicCast<NG::FrameNode>(uiVirtualNode);
            CHECK_NULL_CONTINUE(virtualNode);
            AccessibilityElementInfo virtualInfo;
            UpdateVirtualNodeAccessibilityElementInfo(
                frameNodeParent, virtualNode, commonProperty, virtualInfo, ngPipeline);
            virtualInfo.SetParent(frameNodeParent->GetAccessibilityId());
            ReplaceChildrenWithVirtualRoot(nodeInfo, virtualNode->GetAccessibilityId());
            if (!uiVirtualNode->GetChildren(true).empty()) {
                UpdateVirtualNodeInfo(infos, virtualInfo, uiVirtualNode, commonProperty, ngPipeline);
            }
            infos.push_back(virtualInfo);
            infos.push_back(nodeInfo);
            continue;
        }
        if (GetAllVirtualNodeElementInfo(infos, commonProperty, ngPipeline, frameNodeParent, nodeInfo)) {
            infos.push_back(nodeInfo);
            continue;
        }
        if (!IsExtensionComponent(frameNodeParent) || IsUIExtensionShowPlaceholder(frameNodeParent)) {
            infos.push_back(nodeInfo);
            GetChildrenFromFrameNode(frameNodeParent, children, commonProperty);
            continue;
        }
        HandleExtensionChildNode(infos, frameNodeParent, searchParam, nodeInfo);
    }
}

void JsAccessibilityManager::UpdateCacheInfoNG(std::list<AccessibilityElementInfo>& infos,
    const RefPtr<NG::FrameNode>& node, CommonProperty& commonProperty,
    const RefPtr<NG::PipelineContext>& ngPipeline, const SearchParameter& searchParam)
{
    uint32_t umode = searchParam.mode;
    std::list<RefPtr<NG::FrameNode>> children;
    // get all children
    if ((umode & (static_cast<uint32_t>(PREFETCH_RECURSIVE_CHILDREN) |
                  static_cast<uint32_t>(PREFETCH_RECURSIVE_CHILDREN_REDUCED))) == 0) {
        return;
    }
    commonProperty.isReduceMode = umode & static_cast<uint32_t>(PREFETCH_RECURSIVE_CHILDREN_REDUCED);
    if (!infos.empty()) {
        auto it = std::prev(infos.end());
        auto& parentInfo = *it;
        if (GetAllVirtualNodeElementInfo(infos, commonProperty, ngPipeline, node, parentInfo)) {
            return;
        }
    }
    GetChildrenFromFrameNode(node, children, commonProperty);
    UpdateChildrenNodeInCache(infos, commonProperty, ngPipeline, searchParam, children);
}

void JsAccessibilityManager::UpdateEventWhiteList(const std::vector<uint32_t>& eventList)
{
    eventWhiteList_ = eventList;
}

namespace {
bool CanAccessibilityFocusedNG(const RefPtr<NG::FrameNode>& node)
{
    CHECK_NULL_RETURN(node, false);
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, false);
    auto level = accessibilityProperty->GetAccessibilityLevel();
    return !node->IsRootNode() &&
           node->GetLayoutProperty()->GetVisibilityValue(VisibleType::VISIBLE) == VisibleType::VISIBLE &&
           level != NG::AccessibilityProperty::Level::NO_STR &&
           level != NG::AccessibilityProperty::Level::NO_HIDE_DESCENDANTS;
}
// focus move search
void AddFocusableNode(std::list<RefPtr<NG::FrameNode>>& nodeList, const RefPtr<NG::FrameNode>& node)
{
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    auto level = accessibilityProperty->GetAccessibilityLevel();
    if (CanAccessibilityFocusedNG(node)) {
        nodeList.emplace_back(node);
    }

    if (!accessibilityProperty->IsAccessibilityGroup() &&
        level != NG::AccessibilityProperty::Level::NO_HIDE_DESCENDANTS) {
        std::list<RefPtr<NG::FrameNode>> children;
        CommonProperty commonProperty;
        commonProperty.pageNodes.clear(); // empty means search all page
        commonProperty.pagePaths.clear();
        for (const auto& child : node->GetChildren(true)) {
            GetFrameNodeChildren(child, children, commonProperty);
        }

        for (const auto& child : children) {
            AddFocusableNode(nodeList, child);
        }
    }
}

// execute action
bool RequestFocus(RefPtr<NG::FrameNode>& frameNode)
{
    auto focusHub = frameNode->GetFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    return focusHub->RequestFocusImmediately();
}

bool LostFocus(const RefPtr<NG::FrameNode>& frameNode)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto focusHub = frameNode->GetFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    focusHub->LostFocus();
    return true;
}

void HandleWillClickAccept(const RefPtr<NG::FrameNode>& frameNode)
{
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<NG::EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto gestureEventHub = eventHub->GetGestureEventHub();
    CHECK_NULL_VOID(gestureEventHub);
    auto gestureEventInfo = gestureEventHub->GetGestureEventInfo();
    auto clickInfo = gestureEventHub->GetClickInfo();
    NG::UIObserverHandler::GetInstance().NotifyWillClick(gestureEventInfo, clickInfo, frameNode);
}

void HandleDidClickAccept(const RefPtr<NG::FrameNode>& frameNode)
{
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<NG::EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto gestureEventHub = eventHub->GetGestureEventHub();
    CHECK_NULL_VOID(gestureEventHub);
    auto gestureEventInfo = gestureEventHub->GetGestureEventInfo();
    auto clickInfo = gestureEventHub->GetClickInfo();
    NG::UIObserverHandler::GetInstance().NotifyDidClick(gestureEventInfo, clickInfo, frameNode);
}

bool ActClick(RefPtr<NG::FrameNode>& frameNode, const NG::SecCompEnhanceEvent& secEvent)
{
    if (NG::AccessibilityFunctionUtils::HandleClickBySecComponent(frameNode, secEvent)) {
        // notify child action happened to parent
        NG::AccessibilityFunctionUtils::HandleNotifyChildAction(frameNode, NotifyChildActionType::ACTION_CLICK);
        return true;
    }
    RefPtr<NG::FrameNode> controllerNode;
    auto controllerType = NG::AccessibilityPropertyUtils::CheckAndGetActionController(frameNode, controllerNode);
    if ((controllerType == NG::ActionControllerType::CONTROLLER_CLICK) && (controllerNode)) {
        return ActClick(controllerNode, secEvent);
    }

    auto interceptResult =
        NG::AccessibilityFunctionUtils::HandleAccessibilityActionIntercept(
            frameNode, AccessibilityInterfaceAction::ACCESSIBILITY_CLICK);
    if (interceptResult == AccessibilityActionInterceptResult::ACTION_INTERCEPT) {
        return true;
    }
    auto eventHub = frameNode->GetEventHub<NG::EventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    auto gesture = eventHub->GetGestureEventHub();
    CHECK_NULL_RETURN(gesture, false);
    HandleWillClickAccept(frameNode);
    bool result = gesture->ActClick();
    HandleDidClickAccept(frameNode);
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, result);
    auto accessibilityAction = ACTIONS.find(ACCESSIBILITY_ACTION_CLICK);
    if (accessibilityAction != ACTIONS.end()) {
        AccessibilityActionParam param;
        param.accessibilityProperty = accessibilityProperty;
        result |= accessibilityAction->second(param);
    }

    // notify child action happened to parent
    NG::AccessibilityFunctionUtils::HandleNotifyChildAction(frameNode, NotifyChildActionType::ACTION_CLICK);
    return result;
}

bool ActLongClick(RefPtr<NG::FrameNode>& frameNode)
{
    CHECK_NULL_RETURN(frameNode, false);
    auto eventHub = frameNode->GetEventHub<NG::EventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    auto gesture = eventHub->GetGestureEventHub();
    CHECK_NULL_RETURN(gesture, false);
    return gesture->ActLongClick();
}

void CreateOrUpdateAccessibilityFocusPaint(const RefPtr<NG::FrameNode>& focusFrameNode)
{
    CHECK_NULL_VOID(focusFrameNode);
    ACE_SCOPED_TRACE("CreateOrUpdateAccessibilityFocusPaint: targetNode[%s].", focusFrameNode->GetTag().c_str());
    auto paintNode = NG::FrameNode::CreateFrameNode(V2::ACCESSIBILITY_FOCUS_PAINT_NODE_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<NG::AccessibilityFocusPaintNodePattern>(focusFrameNode));
    CHECK_NULL_VOID(paintNode);
    auto pattern = paintNode->GetPattern<NG::AccessibilityFocusPaintNodePattern>();
    CHECK_NULL_VOID(pattern);
    pattern->UpdateFocusNode(focusFrameNode);
    auto renderContext = paintNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->OnZIndexUpdate(INT32_MAX);
    renderContext->ResetAccessibilityFocusRect();
    renderContext->UpdateAccessibilityFocus(true, focusFrameNode->GetAccessibilityId());

    auto pipeline = focusFrameNode->GetContextRefPtr();
    CHECK_NULL_VOID(pipeline);
    auto overlayManager = pipeline->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);

    auto rootNode = AceType::DynamicCast<NG::FrameNode>(overlayManager->FindWindowScene(focusFrameNode));
    CHECK_NULL_VOID(rootNode);
    pattern->UpdateRootNode(rootNode);
    rootNode->SetFocusPaintNode(paintNode);
    rootNode->MarkNeedSyncRenderTree();
    rootNode->RebuildRenderContextTree();
}

void RemoveAccessibilityFocusPaint(const RefPtr<NG::FrameNode>& focusFrameNode)
{
    CHECK_NULL_VOID(focusFrameNode);
    ACE_SCOPED_TRACE("Node[%s] RemoveAccessibilityFocusPaint", focusFrameNode->GetTag().c_str());
    auto paintNode = focusFrameNode->GetPaintNode();
    CHECK_NULL_VOID(paintNode);
    focusFrameNode->SetPaintNode(nullptr);
    auto renderContext = paintNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->UpdateAccessibilityFocus(false, focusFrameNode->GetAccessibilityId());
    auto pattern = paintNode->GetPattern<NG::AccessibilityFocusPaintNodePattern>();
    CHECK_NULL_VOID(pattern);
    auto rootNode = AceType::DynamicCast<NG::FrameNode>(pattern->GetRootNode().Upgrade());
    CHECK_NULL_VOID(rootNode);
    rootNode->SetFocusPaintNode(nullptr);
    rootNode->MarkNeedSyncRenderTree();
    rootNode->RebuildRenderContextTree();
}

void PaintAccessibilityFocusNode(const RefPtr<NG::FrameNode>& focusNode, bool focus)
{
    CHECK_NULL_VOID(focusNode);
    if (focusNode->IsDrawFocusOnTop()) {
        if (focus) {
            CreateOrUpdateAccessibilityFocusPaint(focusNode);
            UpdatePaintNodeRender(focusNode);
        } else {
            RemoveAccessibilityFocusPaint(focusNode);
        }
    } else {
        auto context = focusNode->GetRenderContext();
        CHECK_NULL_VOID(context);
        context->UpdateAccessibilityFocus(focus);
    }
}

void ClearAccessibilityFocus(const RefPtr<NG::FrameNode>& root, int64_t focusNodeId)
{
    auto oldFocusNode = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(root, focusNodeId);
    CHECK_NULL_VOID(oldFocusNode);
    if (oldFocusNode->IsDrawFocusOnTop()) {
        PaintAccessibilityFocusNode(oldFocusNode, false);
    } else {
        oldFocusNode->GetRenderContext()->UpdateAccessibilityFocus(false);
    }
}

NG::RectT<int32_t> GetFrameNodeRectInt(const RefPtr<NG::FrameNode>& frameNode)
{
    auto rect = frameNode->GetTransformRectRelativeToWindow();
    NG::RectT<int32_t> rectInt {
        static_cast<int32_t>(std::floor(rect.Left())),
        static_cast<int32_t>(std::floor(rect.Top())),
        static_cast<int32_t>(std::floor(rect.Width())),
        static_cast<int32_t>(std::floor(rect.Height()))
    };
    return rectInt;
}

void UpdateAccessibilityFocusRect(const RefPtr<NG::FrameNode>& frameNode,
    RefPtr<NG::RenderContext>& renderContext,
    bool isAccessibilityVirtualNode)
{
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(renderContext);
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    if (accessibilityProperty->IsMatchAccessibilityResponseRegion(isAccessibilityVirtualNode)) {
        auto rectInt = accessibilityProperty->GetAccessibilityResponseRegionRect(isAccessibilityVirtualNode);
        renderContext->UpdateAccessibilityFocusRect(rectInt);
        if (isAccessibilityVirtualNode) {
            renderContext->UpdateAccessibilityFocus(true, frameNode->GetAccessibilityId());
        } else {
            PaintAccessibilityFocusNode(frameNode, true);
        }
    } else {
        if (isAccessibilityVirtualNode) {
            renderContext->UpdateAccessibilityFocusRect(GetFrameNodeRectInt(frameNode));
            renderContext->UpdateAccessibilityFocus(true, frameNode->GetAccessibilityId());
        } else {
            renderContext->ResetAccessibilityFocusRect();
            PaintAccessibilityFocusNode(frameNode, true);
        }
    }
}

void ClearVirtualNodeAccessibilityFocus(const RefPtr<NG::FrameNode>& root, int64_t currentFocusVirtualNodeParentId)
{
    CHECK_EQUAL_VOID(currentFocusVirtualNodeParentId, -1);
    auto currentFocusVirtualNodeParentNode =
        NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(root, currentFocusVirtualNodeParentId);
    CHECK_NULL_VOID(currentFocusVirtualNodeParentNode);
    auto renderContext = currentFocusVirtualNodeParentNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->UpdateAccessibilityFocus(false);
}

void SetParentAccessibilityId(const RefPtr<NG::FrameNode>& frameNode, AccessibilityFocusInfo& focusInfo)
{
    auto parentUinode = frameNode->GetVirtualNodeParent().Upgrade();
    CHECK_NULL_VOID(parentUinode);
    auto parentFrame = AceType::DynamicCast<NG::FrameNode>(parentUinode);
    CHECK_NULL_VOID(parentFrame);
    focusInfo.currentFocusVirtualNodeParentId = parentFrame->GetAccessibilityId();
}

bool ActAccessibilityFocus(int64_t elementId, const RefPtr<NG::FrameNode>& frameNode,
    RefPtr<NG::PipelineContext>& context, AccessibilityFocusInfo& focusInfo, bool isNeedClear)
{
    CHECK_NULL_RETURN(frameNode, false);
    bool isAccessibilityVirtualNode = frameNode->IsAccessibilityVirtualNode();
    RefPtr<NG::RenderContext> renderContext = nullptr;
    if (isAccessibilityVirtualNode) {
        auto parentUinode = frameNode->GetVirtualNodeParent().Upgrade();
        CHECK_NULL_RETURN(parentUinode, false);
        auto parentFrame = AceType::DynamicCast<NG::FrameNode>(parentUinode);
        CHECK_NULL_RETURN(parentFrame, false);
        renderContext = parentFrame->GetRenderContext();
    } else {
        renderContext = frameNode->GetRenderContext();
    }
    CHECK_NULL_RETURN(renderContext, false);
    if (isNeedClear) {
        if (elementId != focusInfo.currentFocusNodeId) {
            return false;
        }
        if (isAccessibilityVirtualNode) {
            renderContext->UpdateAccessibilityFocus(false);
            focusInfo.currentFocusVirtualNodeParentId = -1;
        } else {
            PaintAccessibilityFocusNode(frameNode, false);
        }
        focusInfo.currentFocusNodeId = -1;
        auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
        CHECK_NULL_RETURN(accessibilityProperty, true);
        accessibilityProperty->OnAccessibilityFocusCallback(false);
        accessibilityProperty->SetAccessibilityFocusState(false);
        return true;
    }
    CHECK_EQUAL_RETURN(elementId, focusInfo.currentFocusNodeId, false);
    ClearAccessibilityFocus(context->GetRootElement(), focusInfo.currentFocusNodeId);
    if (focusInfo.currentFocusNodeId != focusInfo.currentFocusVirtualNodeParentId) {
        ClearVirtualNodeAccessibilityFocus(context->GetRootElement(), focusInfo.currentFocusVirtualNodeParentId);
    }
    UpdateAccessibilityFocusRect(frameNode, renderContext, isAccessibilityVirtualNode);
    focusInfo.currentFocusNodeId = frameNode->GetAccessibilityId();
    if (isAccessibilityVirtualNode) {
        SetParentAccessibilityId(frameNode, focusInfo);
    }
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, false);
    accessibilityProperty->OnAccessibilityFocusCallback(true);
    accessibilityProperty->SetAccessibilityFocusState(true);
    if (focusInfo.isFocusMoveAction) {
        NG::AccessibilityFrameNodeUtils::ProcessFocusScroll(frameNode, context);
    }
    return true;
}

inline std::string GetSupportAction(const std::unordered_set<AceAction>& supportAceActions)
{
    std::string actionForDump;
    for (const auto& action : supportAceActions) {
        if (!actionForDump.empty()) {
            actionForDump.append(",");
        }
        actionForDump.append(std::to_string(static_cast<int32_t>(action)));
    }
    return actionForDump;
}

static std::string ConvertActionTypeToString(ActionType action)
{
    static const ActionStrTable actionStrTable[] = {
        { ActionType::ACCESSIBILITY_ACTION_FOCUS, "ACCESSIBILITY_ACTION_FOCUS" },
        { ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS, "ACCESSIBILITY_ACTION_CLEAR_FOCUS" },
        { ActionType::ACCESSIBILITY_ACTION_SELECT, "ACCESSIBILITY_ACTION_SELECT" },
        { ActionType::ACCESSIBILITY_ACTION_CLEAR_SELECTION, "ACCESSIBILITY_ACTION_CLEAR_SELECTION" },
        { ActionType::ACCESSIBILITY_ACTION_CLICK, "ACCESSIBILITY_ACTION_CLICK" },
        { ActionType::ACCESSIBILITY_ACTION_LONG_CLICK, "ACCESSIBILITY_ACTION_LONG_CLICK" },
        { ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS, "ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS" },
        { ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS,
            "ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS" },
        { ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD, "ACCESSIBILITY_ACTION_SCROLL_FORWARD" },
        { ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD, "ACCESSIBILITY_ACTION_SCROLL_BACKWARD" },
        { ActionType::ACCESSIBILITY_ACTION_COPY, "ACCESSIBILITY_ACTION_COPY" },
        { ActionType::ACCESSIBILITY_ACTION_PASTE, "ACCESSIBILITY_ACTION_PASTE" },
        { ActionType::ACCESSIBILITY_ACTION_CUT, "ACCESSIBILITY_ACTION_CUT" },
        { ActionType::ACCESSIBILITY_ACTION_SET_SELECTION, "ACCESSIBILITY_ACTION_SET_SELECTION" },
        { ActionType::ACCESSIBILITY_ACTION_SET_TEXT, "ACCESSIBILITY_ACTION_SET_TEXT" },
        { ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT, "ACCESSIBILITY_ACTION_NEXT_TEXT" },
        { ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT, "ACCESSIBILITY_ACTION_PREVIOUS_TEXT" },
        { ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION, "ACCESSIBILITY_ACTION_SET_CURSOR_POSITION" },
        { ActionType::ACCESSIBILITY_ACTION_SPAN_CLICK, "ACCESSIBILITY_ACTION_SPAN_CLICK" },
        { ActionType::ACCESSIBILITY_ACTION_NEXT_HTML_ITEM, "ACCESSIBILITY_ACTION_NEXT_HTML_ITEM" },
        { ActionType::ACCESSIBILITY_ACTION_PREVIOUS_HTML_ITEM, "ACCESSIBILITY_ACTION_PREVIOUS_HTML_ITEM" },
    };
    for (const auto& item : actionStrTable) {
        if (action == item.action) {
            return item.actionStr;
        }
    }
    return ACTION_DEFAULT_PARAM;
}

static AceAction ConvertAccessibilityAction(ActionType accessibilityAction)
{
    static const ActionTable actionTable[] = {
        { AceAction::ACTION_CLICK, ActionType::ACCESSIBILITY_ACTION_CLICK },
        { AceAction::ACTION_LONG_CLICK, ActionType::ACCESSIBILITY_ACTION_LONG_CLICK },
        { AceAction::ACTION_SCROLL_FORWARD, ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD },
        { AceAction::ACTION_SCROLL_BACKWARD, ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD },
        { AceAction::ACTION_FOCUS, ActionType::ACCESSIBILITY_ACTION_FOCUS },
        { AceAction::ACTION_CLEAR_FOCUS, ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS },
        { AceAction::ACTION_ACCESSIBILITY_FOCUS, ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS },
        { AceAction::ACTION_CLEAR_ACCESSIBILITY_FOCUS, ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS },
        { AceAction::ACTION_NEXT_AT_MOVEMENT_GRANULARITY, ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT },
        { AceAction::ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY, ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT },
        { AceAction::ACTION_SET_TEXT, ActionType::ACCESSIBILITY_ACTION_SET_TEXT },
        { AceAction::ACTION_COPY, ActionType::ACCESSIBILITY_ACTION_COPY },
        { AceAction::ACTION_PASTE, ActionType::ACCESSIBILITY_ACTION_PASTE },
        { AceAction::ACTION_CUT, ActionType::ACCESSIBILITY_ACTION_CUT },
        { AceAction::ACTION_SELECT, ActionType::ACCESSIBILITY_ACTION_SELECT },
        { AceAction::ACTION_CLEAR_SELECTION, ActionType::ACCESSIBILITY_ACTION_CLEAR_SELECTION },
        { AceAction::ACTION_SET_SELECTION, ActionType::ACCESSIBILITY_ACTION_SET_SELECTION },
        { AceAction::ACTION_SET_CURSOR_POSITION, ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION },
        { AceAction::ACTION_EXEC_SUB_COMPONENT, ActionType::ACCESSIBILITY_ACTION_SPAN_CLICK },
        { AceAction::ACTION_NEXT_HTML_ITEM, ActionType::ACCESSIBILITY_ACTION_NEXT_HTML_ITEM },
        { AceAction::ACTION_PREVIOUS_HTML_ITEM, ActionType::ACCESSIBILITY_ACTION_PREVIOUS_HTML_ITEM },
    };
    for (const auto& item : actionTable) {
        if (accessibilityAction == item.action) {
            return item.aceAction;
        }
    }
    return AceAction::ACTION_NONE;
}

static void DumpSupportActionNG(const AccessibilityElementInfo& nodeInfo)
{
    DumpLog::GetInstance().AddDesc(
        "support action instructions: use command to make application components perform corresponding action");
    DumpLog::GetInstance().AddDesc(
        "use support action command: aa dump -i [AbilityRecord] -c -inspector [AccessibilityId] [AceAction]");
    std::string actionForDump;
    for (const auto& action : nodeInfo.GetActionList()) {
        if (!actionForDump.empty()) {
            actionForDump.append(",");
        }
        actionForDump.append(ConvertActionTypeToString(action.GetActionType()));
        actionForDump.append(": ");
        actionForDump.append(std::to_string(static_cast<int32_t>(ConvertAccessibilityAction(action.GetActionType()))));
    }
    DumpLog::GetInstance().AddDesc("support action: ", actionForDump);
}

static void DumpGridInfoNG(const AccessibilityElementInfo& nodeInfo)
{
    DumpLog::GetInstance().AddDesc("grid info rows: ", nodeInfo.GetGrid().GetRowCount());
    DumpLog::GetInstance().AddDesc("grid info columns: ", nodeInfo.GetGrid().GetColumnCount());
    DumpLog::GetInstance().AddDesc("grid info select mode: ", nodeInfo.GetGrid().GetSelectionMode());
    DumpLog::GetInstance().AddDesc("grid item info, row: ", nodeInfo.GetGridItem().GetRowIndex());
    DumpLog::GetInstance().AddDesc("grid item info, column: ", nodeInfo.GetGridItem().GetColumnIndex());
    DumpLog::GetInstance().AddDesc("grid item info, rowSpan: ", nodeInfo.GetGridItem().GetRowSpan());
    DumpLog::GetInstance().AddDesc("grid item info, columnSpan: ", nodeInfo.GetGridItem().GetColumnSpan());
    DumpLog::GetInstance().AddDesc("grid item info, is heading: ", nodeInfo.GetGridItem().IsHeading());
    DumpLog::GetInstance().AddDesc("grid item info, selected: ", nodeInfo.GetGridItem().IsSelected());
}

inline void DumpContentListNG(const AccessibilityElementInfo& nodeInfo)
{
    std::vector<std::string> contentList;
    nodeInfo.GetContentList(contentList);
    std::string contents;
    for (const auto& content : contentList) {
        if (!contents.empty()) {
            contents.append(",");
        }
        contents.append(content);
    }
    DumpLog::GetInstance().AddDesc("content list: ", contents);
}

static void DumpExtraElementInfoNG(const AccessibilityElementInfo& nodeInfo)
{
    ExtraElementInfo extraElementInfo = nodeInfo.GetExtraElement();
    if (!extraElementInfo.GetExtraElementInfoValueStr().empty()) {
        for (auto i = extraElementInfo.GetExtraElementInfoValueStr().begin();
             i != extraElementInfo.GetExtraElementInfoValueStr().end(); ++i) {
            DumpLog::GetInstance().AddDesc("extra element info: ", i->first, i->second);
        }
    }
    if (!extraElementInfo.GetExtraElementInfoValueInt().empty()) {
        for (auto i = extraElementInfo.GetExtraElementInfoValueInt().begin();
             i != extraElementInfo.GetExtraElementInfoValueInt().end(); ++i) {
            DumpLog::GetInstance().AddDesc("extra element info: ", i->first, i->second);
        }
    }
}

static void UpdateSpanList(std::vector<SpanInfo>& spansInfosList, std::string& spans)
{
    for (const auto& span : spansInfosList) {
        if (!spans.empty()) {
            spans.append(",");
        }
        spans.append("\n\t span id: ");
        spans.append(std::to_string(span.GetSpanId()));
        spans.append(", span text: ");
        spans.append(span.GetSpanText());
        spans.append(", accessibility text: ");
        spans.append(span.GetAccessibilityText());
        spans.append(", accessibility description: ");
        spans.append(span.GetAccessibilityDescription());
        spans.append(", accessibility level: ");
        spans.append(span.GetAccessibilityLevel());
    }
}

inline void DumpSpanListNG(const AccessibilityElementInfo& nodeInfo)
{
    std::string spans;
    std::vector<SpanInfo> spansInfosList = nodeInfo.GetSpanList();
    std::size_t spanCount = spansInfosList.size();
    UpdateSpanList(spansInfosList, spans);
    DumpLog::GetInstance().AddDesc("span list count: ", static_cast<int32_t>(spanCount));
    DumpLog::GetInstance().AddDesc("span list: ", spans);
}

void GenerateAccessibilityEventInfo(const AccessibilityEvent& accessibilityEvent, AccessibilityEventInfo& eventInfo)
{
    Accessibility::EventType type = Accessibility::EventType::TYPE_VIEW_INVALID;
    if (accessibilityEvent.type != AccessibilityEventType::UNKNOWN) {
        type = ConvertAceEventType(accessibilityEvent.type);
    } else {
        type = ConvertStrToEventType(accessibilityEvent.eventType);
    }

    if (type == Accessibility::EventType::TYPE_VIEW_INVALID) {
        return;
    }

    eventInfo.SetTimeStamp(GetMicroTickCount());
    eventInfo.SetBeforeText(accessibilityEvent.beforeText);
    eventInfo.SetLatestContent(accessibilityEvent.latestContent);
    eventInfo.SetTextAnnouncedForAccessibility(accessibilityEvent.textAnnouncedForAccessibility);
    eventInfo.SetWindowChangeTypes(static_cast<Accessibility::WindowUpdateType>(accessibilityEvent.windowChangeTypes));
    eventInfo.SetWindowContentChangeTypes(
        static_cast<Accessibility::WindowsContentChangeTypes>(accessibilityEvent.windowContentChangeTypes));
    eventInfo.SetSource(accessibilityEvent.nodeId);
    eventInfo.SetEventType(type);
    eventInfo.SetCurrentIndex(static_cast<int>(accessibilityEvent.currentItemIndex));
    eventInfo.SetItemCounts(static_cast<int>(accessibilityEvent.itemCount));
    eventInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());
    eventInfo.SetBeginIndex(accessibilityEvent.startIndex);
    eventInfo.SetEndIndex(accessibilityEvent.endIndex);
    if (accessibilityEvent.extraEventInfo.size() > 0) {
        ExtraEventInfo extraEventInfo;
        for (const auto& info : accessibilityEvent.extraEventInfo) {
            auto ret = extraEventInfo.SetExtraEventInfo(info.first, info.second);
            TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "The result of SetExtraEventInfo:%{public}d, keyStrLen:%{public}d",
                ret, static_cast<int>(info.first.length()));
        }
        eventInfo.SetExtraEvent(extraEventInfo);
    }
}
} // namespace

void JsAccessibilityManager::DumpAccessibilityPropertyNG(const AccessibilityElementInfo& nodeInfo)
{
    DumpLog::GetInstance().AddDesc("checked: ", BoolToString(nodeInfo.IsChecked()));
    DumpLog::GetInstance().AddDesc("selected: ", BoolToString(nodeInfo.IsSelected()));
    DumpLog::GetInstance().AddDesc("checkable: ", BoolToString(nodeInfo.IsCheckable()));
    DumpLog::GetInstance().AddDesc("scrollable: ", BoolToString(nodeInfo.IsScrollable()));
    DumpLog::GetInstance().AddDesc("accessibility hint: ", BoolToString(nodeInfo.IsGivingHint()));
    DumpLog::GetInstance().AddDesc("hint text: ", nodeInfo.GetHint());
    DumpLog::GetInstance().AddDesc("error text: ", nodeInfo.GetError());
    DumpLog::GetInstance().AddDesc("max text length: ", nodeInfo.GetTextLengthLimit());
    DumpLog::GetInstance().AddDesc("scroll offset: ", nodeInfo.GetOffset());
    DumpLog::GetInstance().AddDesc("text selection start: ", nodeInfo.GetSelectedBegin());
    DumpLog::GetInstance().AddDesc("text selection end: ", nodeInfo.GetSelectedEnd());
    DumpLog::GetInstance().AddDesc("is multi line: ", BoolToString(nodeInfo.IsPluraLineSupported()));
    DumpLog::GetInstance().AddDesc("is password: ", BoolToString(nodeInfo.IsPassword()));
    DumpLog::GetInstance().AddDesc(
        "text input type: ", ConvertInputTypeToString(static_cast<AceTextCategory>(nodeInfo.GetInputType())));
    DumpGridInfoNG(nodeInfo);
    DumpLog::GetInstance().AddDesc("min value: ", nodeInfo.GetRange().GetMin());
    DumpLog::GetInstance().AddDesc("max value: ", nodeInfo.GetRange().GetMax());
    DumpLog::GetInstance().AddDesc("current value: ", nodeInfo.GetRange().GetCurrent());
    DumpLog::GetInstance().AddDesc("current index: ", nodeInfo.GetCurrentIndex());
    DumpLog::GetInstance().AddDesc("begin index: ", nodeInfo.GetBeginIndex());
    DumpLog::GetInstance().AddDesc("end index: ", nodeInfo.GetEndIndex());
    DumpLog::GetInstance().AddDesc("collection item counts: ", nodeInfo.GetItemCounts());
    DumpLog::GetInstance().AddDesc("editable: ", BoolToString(nodeInfo.IsEditable()));
    DumpLog::GetInstance().AddDesc("is essential: ", BoolToString(nodeInfo.IsEssential()));
    DumpLog::GetInstance().AddDesc("deletable: ", nodeInfo.IsDeletable());
    DumpLog::GetInstance().AddDesc("live region: ", nodeInfo.GetLiveRegion());
    DumpLog::GetInstance().AddDesc("content description: ", nodeInfo.GetDescriptionInfo());
    DumpLog::GetInstance().AddDesc("content invalid: ", BoolToString(nodeInfo.GetContentInvalid()));
    DumpLog::GetInstance().AddDesc("accessibility label: ", nodeInfo.GetLabeledAccessibilityId());
    DumpLog::GetInstance().AddDesc("isActive: ", nodeInfo.GetIsActive());
    DumpLog::GetInstance().AddDesc("accessibilityVisible: ", nodeInfo.GetAccessibilityVisible());
    DumpLog::GetInstance().AddDesc("accessibilityNextFocusInspectorKey: ",
                                   nodeInfo.GetAccessibilityNextFocusInspectorKey());
    DumpLog::GetInstance().AddDesc("accessibilityTextHint: ", nodeInfo.GetTextType());
    DumpLog::GetInstance().AddDesc("accessibilityScrollTriggerable: ",
                                   nodeInfo.GetAccessibilityScrollable());
    DumpLog::GetInstance().AddDesc("accessibilityNextFocusId: ", nodeInfo.GetAccessibilityNextFocusId());
    DumpLog::GetInstance().AddDesc("accessibilityPreviousFocusId: ", nodeInfo.GetAccessibilityPreviousFocusId());
    DumpLog::GetInstance().AddDesc("accessibilityStateDescription: ", nodeInfo.GetAccessibilityStateDescription());
    DumpLog::GetInstance().AddDesc("clip: ", nodeInfo.GetClip());
    DumpExtraElementInfoNG(nodeInfo);
    DumpLog::GetInstance().AddDesc(
        "trigger action: ", static_cast<int32_t>(ConvertAccessibilityAction(nodeInfo.GetTriggerAction())));
    DumpLog::GetInstance().AddDesc("text move step: " + std::to_string(nodeInfo.GetTextMovementStep()));
    DumpSpanListNG(nodeInfo);
    DumpSupportActionNG(nodeInfo);
    DumpContentListNG(nodeInfo);
    DumpLog::GetInstance().AddDesc("latest content: ", nodeInfo.GetLatestContent());
}

void JsAccessibilityManager::DumpCommonPropertyNG(const AccessibilityElementInfo& nodeInfo, int32_t treeId)
{
    AccessibilityElementInfoUtils::ToCommonInfo(nodeInfo, treeId_);
}

void JsAccessibilityManager::UpdateVirtualNodeFocus()
{
    auto frameNode = lastFrameNode_.Upgrade();
    CHECK_NULL_VOID(frameNode);
    if (!frameNode->IsAccessibilityVirtualNode()) {
        return;
    }
    RefPtr<NG::RenderContext> renderContext;
    if (frameNode->GetAccessibilityId() == currentFocusNodeId_) {
        auto parentUinode = frameNode->GetVirtualNodeParent().Upgrade();
        CHECK_NULL_VOID(parentUinode);
        auto parentFrame = AceType::DynamicCast<NG::FrameNode>(parentUinode);
        CHECK_NULL_VOID(parentFrame);
        renderContext = parentFrame->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        // FOCUS_WITHOUT_EVENT used to dont send a11y event, just update rect
        renderContext->UpdateAccessibilityFocus(false, ACCESSIBILITY_FOCUS_WITHOUT_EVENT);
        auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
        CHECK_NULL_VOID(accessibilityProperty);
        if (accessibilityProperty->IsMatchAccessibilityResponseRegion(true)) {
            auto rectInt = accessibilityProperty->GetAccessibilityResponseRegionRect(true);
            renderContext->UpdateAccessibilityFocusRect(rectInt);
        } else {
            renderContext->UpdateAccessibilityFocusRect(GetFrameNodeRectInt(frameNode));
        }
        // used to dont send a11y event, just update rect
        renderContext->UpdateAccessibilityFocus(true, ACCESSIBILITY_FOCUS_WITHOUT_EVENT);
        accessibilityProperty->SetAccessibilityFocusState(true);
    }
}

JsAccessibilityManager::~JsAccessibilityManager()
{
    UnsubscribeStateObserver(AccessibilityStateEventType::EVENT_ACCESSIBILITY_STATE_CHANGED);
    UnsubscribeStateObserver(AccessibilityStateEventType::EVENT_SCREEN_READER_STATE_CHANGED);
    UnsubscribeStateObserver(AccessibilityStateEventType::EVENT_CONFIG_EVENT_CHANGED);

    DeregisterInteractionOperation();
}
void JsAccessibilityManager::ToastAccessibilityConfigObserver::OnConfigChanged(
    const AccessibilityConfig::CONFIG_ID id, const AccessibilityConfig::ConfigValue& value)
{
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "accessibility content timeout changed:%{public}u", value.contentTimeout);
    AceApplicationInfo::GetInstance().SetBarrierfreeDuration((int32_t)value.contentTimeout);
}

bool JsAccessibilityManager::SubscribeToastObserver()
{
    if (!toastObserver_) {
        toastObserver_ = std::make_shared<ToastAccessibilityConfigObserver>();
    }
    CHECK_NULL_RETURN(toastObserver_, false);
    auto& config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
    bool isSuccess = config.InitializeContext();
    if (!isSuccess) {
        return false;
    }
    config.SubscribeConfigObserver(CONFIG_CONTENT_TIMEOUT, toastObserver_);
    return true;
}

bool JsAccessibilityManager::UnsubscribeToastObserver()
{
    CHECK_NULL_RETURN(toastObserver_, false);
    auto& config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
    bool isSuccess = config.InitializeContext();
    if (!isSuccess) {
        return false;
    }
    config.UnsubscribeConfigObserver(CONFIG_CONTENT_TIMEOUT, toastObserver_);
    return true;
}

bool JsAccessibilityManager::SubscribeStateObserver(uint32_t eventType)
{
    if (!stateObserver_[eventType]) {
        stateObserver_[eventType] = std::make_shared<JsAccessibilityStateObserver>();
    }

    stateObserver_[eventType]->SetAccessibilityManager(WeakClaim(this));
    stateObserver_[eventType]->SetPipeline(context_);
    stateObserver_[eventType]->SetEventType(eventType);

    auto instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(instance, false);
    Accessibility::RetError ret = instance->SubscribeStateObserver(stateObserver_[eventType], eventType);
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, " the result of SubscribeStateObserver:%{public}d, eventType:%{public}u",
        ret, eventType);
    return ret == RET_OK;
}

bool JsAccessibilityManager::UnsubscribeStateObserver(uint32_t eventType)
{
    CHECK_NULL_RETURN(stateObserver_[eventType], false);
    std::shared_ptr<AccessibilitySystemAbilityClient> instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(instance, false);
    Accessibility::RetError ret = instance->UnsubscribeStateObserver(stateObserver_[eventType], eventType);
    return ret == RET_OK;
}

AccessibilityWorkMode JsAccessibilityManager::GenerateAccessibilityWorkMode()
{
    AccessibilityWorkMode accessibilityWorkMode;
    auto client = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(client, accessibilityWorkMode);
    auto ret = client->IsTouchExplorationEnabled(accessibilityWorkMode.isTouchExplorationEnabled);
    if (ret != RET_OK) {
        accessibilityWorkMode.isTouchExplorationEnabled = true;
    }
    return accessibilityWorkMode;
}

void JsAccessibilityManager::UpdateAccessibilityNextFocusIdMap(int32_t containerId,
                                                               const std::string& nextFocusInspectorKey,
                                                               int64_t preAccessibilityId,
                                                               bool descendantMode)
{
    nextFocusRelationController_.UpdateNextFocusIdMap(
        containerId, nextFocusInspectorKey, preAccessibilityId, descendantMode);
}

bool JsAccessibilityManager::GetNextFocusDescendantMode(int32_t containerId, const std::string& inspectorId)
{
    return nextFocusRelationController_.GetNextFocusDescendantMode(containerId, inspectorId);
}

void JsAccessibilityManager::InitializeCallback()
{
    if (IsRegister()) {
        return;
    }

    auto pipelineContext = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(pipelineContext);
    windowId_ = pipelineContext->GetWindowId();

    auto client = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_VOID(client);
    bool isEnabled = false;
    client->IsEnabled(isEnabled);
    AceApplicationInfo::GetInstance().SetAccessibilityEnabled(isEnabled);
    bool isScreenReadEnabled = false;
    client->IsScreenReaderEnabled(isScreenReadEnabled);
    AceApplicationInfo::GetInstance().SetAccessibilityScreenReadEnabled(isScreenReadEnabled);

    std::vector<uint32_t> needEvents;
    client->SearchNeedEvents(needEvents);
    UpdateEventWhiteList(needEvents);

    SubscribeStateObserver(AccessibilityStateEventType::EVENT_ACCESSIBILITY_STATE_CHANGED);
    SubscribeStateObserver(AccessibilityStateEventType::EVENT_SCREEN_READER_STATE_CHANGED);
    SubscribeStateObserver(AccessibilityStateEventType::EVENT_CONFIG_EVENT_CHANGED);

    auto container = Platform::AceContainer::GetContainer(pipelineContext->GetInstanceId());
    if (container != nullptr && container->IsDynamicRender()) {
        RegisterDynamicRenderGetParentRectHandler();
        return;
    }

    if (pipelineContext->IsFormRender() || pipelineContext->IsJsCard() || pipelineContext->IsJsPlugin()) {
        return;
    }

    if (container != nullptr && container->IsUIExtensionWindow()) {
        pipelineContext->AddUIExtensionCallbackEvent(OHOS::Ace::NG::UIExtCallbackEventId::ON_UEA_ACCESSIBILITY_READY);
        RegisterUIExtBusinessConsumeCallback();
        RegisterGetParentRectHandler();
        if (container->GetUIContentType() == UIContentType::PREVIEW_UI_EXTENSION) {
            SetIsIgnoreAllAction(true);
        }
        return;
    }

    if (isEnabled) {
        RegisterInteractionOperation(windowId_);
    }
}
void JsAccessibilityManager::RegisterDynamicRenderGetParentRectHandler()
{
    auto accessibilityGetParentRect = [weak = WeakClaim(this)](AccessibilityParentRectInfo& parentRectInfo) {
        auto jsAccessibilityManager = weak.Upgrade();
        CHECK_NULL_VOID(jsAccessibilityManager);
        auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(jsAccessibilityManager->context_.Upgrade());
        CHECK_NULL_VOID(ngPipeline);
        auto container = Platform::AceContainer::GetContainer(ngPipeline->GetInstanceId());
        CHECK_NULL_VOID(container);
        auto containerHandler = container->GetContainerHandler();
        CHECK_NULL_VOID(containerHandler);

        HandlerData data = {
            .actionCode = static_cast<int32_t>(NG::DynamicContainerHandleAction::ACCESSIBILITY_GET_RECT) };
        HandlerReply reply;

        containerHandler->SendDataToHost(data, reply);

        parentRectInfo.left = reply.GetParam<int32_t>("left", 0);
        parentRectInfo.top = reply.GetParam<int32_t>("top", 0);
        parentRectInfo.scaleX = reply.GetParam<float>("scaleX", 1.0f);
        parentRectInfo.scaleY = reply.GetParam<float>("scaleY", 1.0f);
        RotateTransform rotateData(reply.GetParam<int32_t>("rotateDegree", 0),
                                   reply.GetParam<int32_t>("centerX", 0),
                                   reply.GetParam<int32_t>("centerY", 0),
                                   reply.GetParam<int32_t>("innerCenterX", 0),
                                   reply.GetParam<int32_t>("innerCenterY", 0));
        parentRectInfo.rotateTransform = rotateData;
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
            "Get DC host rect [left:%{public}d, top:%{public}d, scaleX:%{public}f, scaleY:%{public}f].",
            parentRectInfo.left, parentRectInfo.top, parentRectInfo.scaleX, parentRectInfo.scaleY);
    };
    SetAccessibilityGetParentRectHandler(accessibilityGetParentRect);
}

void JsAccessibilityManager::RegisterGetParentRectHandler()
{
    auto accessibilityGetParentRect = [weak = WeakClaim(this)](AccessibilityParentRectInfo& parentRectInfo) {
        auto jsAccessibilityManager = weak.Upgrade();
        CHECK_NULL_VOID(jsAccessibilityManager);
        auto uecRectInfo = jsAccessibilityManager->GetUECAccessibilityParentRectInfo();
        if (uecRectInfo.isChanged) {
            parentRectInfo.left = uecRectInfo.left;
            parentRectInfo.top = uecRectInfo.top;
            parentRectInfo.scaleX = uecRectInfo.scaleX;
            parentRectInfo.scaleY = uecRectInfo.scaleY;
            parentRectInfo.rotateTransform = uecRectInfo.rotateTransform;
        } else {
            auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(jsAccessibilityManager->context_.Upgrade());
            CHECK_NULL_VOID(ngPipeline);
            parentRectInfo.left = jsAccessibilityManager->GetWindowLeft(ngPipeline->GetWindowId());
            parentRectInfo.top = jsAccessibilityManager->GetWindowTop(ngPipeline->GetWindowId());
            auto container = Container::CurrentSafely();
            if (container) {
                auto windowScale = container->GetWindowScale();
                parentRectInfo.scaleX = windowScale;
                parentRectInfo.scaleY = windowScale;
            }
        }
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
            "Get host rect [scaleX:%{public}f, scaleY:%{public}f].", parentRectInfo.scaleX, parentRectInfo.scaleY);
    };
    SetAccessibilityGetParentRectHandler(accessibilityGetParentRect);
}

void JsAccessibilityManager::RegisterUIExtBusinessConsumeCallback()
{
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context_.Upgrade());
    CHECK_NULL_VOID(ngPipeline);
    auto uiExtManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtManager);
    auto updateAccessibilityParentRectCallback = [weak = WeakClaim(this)](const AAFwk::Want& data) -> int32_t {
        auto jsAccessibilityManager = weak.Upgrade();
        CHECK_NULL_RETURN(jsAccessibilityManager, -1);
        AccessibilityParentRectInfo info;
        info.left = data.GetIntParam("left", 0);
        info.top = data.GetIntParam("top", 0);
        info.scaleX = data.GetFloatParam("scaleX", 1.0f);
        info.scaleY = data.GetFloatParam("scaleY", 1.0f);
        RotateTransform rotateTransform(data.GetIntParam("rotateDegree", 0), data.GetIntParam("centerX", 0),
            data.GetIntParam("centerY", 0), data.GetIntParam("innerCenterX", 0),
            data.GetIntParam("innerCenterY", 0));
        info.rotateTransform = rotateTransform;
        info.isChanged = true;
        jsAccessibilityManager->UpdateUECAccessibilityParentRectInfo(info);
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
            "Update UIExt Accessiblity [scaleX:%{public}f, scaleY:%{public}f].", info.scaleX, info.scaleY);
        // go on transfer info to next uiextension
        auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(jsAccessibilityManager->context_.Upgrade());
        CHECK_NULL_RETURN(ngPipeline, -1);
        auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
        CHECK_NULL_RETURN(uiExtensionManager, -1);
        uiExtensionManager->TransferAccessibilityRectInfo();
        return 0;
    };
    uiExtManager->RegisterBusinessDataConsumeCallback(
        Ace::NG::UIContentBusinessCode::TRANSFORM_PARAM, updateAccessibilityParentRectCallback);
}

AccessibilityParentRectInfo JsAccessibilityManager::GetUECAccessibilityParentRectInfo() const
{
    return uecRectInfo_;
}

void JsAccessibilityManager::UpdateUECAccessibilityParentRectInfo(const AccessibilityParentRectInfo& info)
{
    uecRectInfo_ = info;
}

namespace {
    const char FULL_SILENT[] = "FULL_SILENT";

    bool IsPageEvent(const Accessibility::EventType& eventType)
    {
        return eventType && (eventType == Accessibility::EventType::TYPE_PAGE_STATE_UPDATE
            || eventType == Accessibility::EventType::TYPE_PAGE_CONTENT_UPDATE
            || eventType == Accessibility::EventType::TYPE_PAGE_OPEN
            || eventType == Accessibility::EventType::TYPE_PAGE_CLOSE
            || eventType == Accessibility::EventType::TYPE_PAGE_STATE_PRE_UPDATE);
    }

    bool IsRootOrPageComponent(const std::string& componentType)
    {
        return !componentType.empty() && (componentType == V2::ROOT_ETS_TAG || componentType == V2::PAGE_ETS_TAG);
    }

    bool CheckSendAccessibilityEventByPageMode(
        const std::string& pageMode, const std::string& componentType, const int32_t pageId)
    {
        if (pageMode == FULL_SILENT) {
            TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
                "current pageMode:%{public}s, drop page event", pageMode.c_str());
            return false;
        }
        if (IsRootOrPageComponent(componentType) && pageId <= 1) {
            TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
                "current pageMode:%{public}s componentType:%{public}s pageId:%{public}d, drop page event",
                pageMode.c_str(), componentType.c_str(), pageId);
            return false;
        }
        return true;
    }

    bool CheckExtensionComponentReadyByPageId(
        const int32_t pageId, const std::vector<std::pair<WeakPtr<NG::FrameNode>, bool>>& nodeVec)
    {
        for (const auto& [node, status] : nodeVec) {
            auto frameNode = node.Upgrade();
            if (frameNode && ((frameNode->GetPageId() == pageId || pageId == -1) && !status)) {
                return false;
            }
        }
        return true;
    }

    bool IsDelNode(const WeakPtr<NG::FrameNode>& node)
    {
        auto frameNode = node.Upgrade();
        CHECK_NULL_RETURN(frameNode, true);
        auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
        CHECK_NULL_RETURN(accessibilityProperty, true);
        return !accessibilityProperty->HasAccessibilitySamePage();
    }

    void UpdateExtensionComponentStatusVec(std::vector<std::pair<WeakPtr<NG::FrameNode>, bool>>& nodeVec)
    {
        for (auto it = nodeVec.begin(); it != nodeVec.end();) {
            if (IsDelNode(it->first)) {
                it = nodeVec.erase(it);
            } else {
                ++it;
            }
        }
    }

    int32_t GetNowPageId(const RefPtr<PipelineBase>& context, int32_t originPageId)
    {
        if (originPageId != -1) {
            return originPageId;
        }
        auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
        CHECK_NULL_RETURN(ngPipeline, originPageId);
        auto stageManager = ngPipeline->GetStageManager();
        CHECK_NULL_RETURN(stageManager, originPageId);
        auto node = stageManager->GetLastPage();
        CHECK_NULL_RETURN(node, originPageId);
        return node->GetPageId();
    }

    bool NotInPage(const RefPtr<NG::FrameNode>& node)
    {
        auto parent = node->GetParent();
        while (parent) {
            if (parent->GetTag() == V2::PAGE_ETS_TAG) {
                return false;
            }
            parent = parent->GetParent();
        }
        return true;
    }

    void ClearDefaultFocusList(std::list<WeakPtr<NG::FrameNode>>& nodeList)
    {
        for (auto it = nodeList.begin(); it != nodeList.end();) {
            auto node = it->Upgrade();
            if (!node) {
                it = nodeList.erase(it);
            } else {
                ++it;
            }
        }
    }

    Accessibility::EventType GetEventTypeByAccessibilityEvent(const AccessibilityEvent& accessibilityEvent)
    {
        Accessibility::EventType type = Accessibility::EventType::TYPE_VIEW_INVALID;
        if (accessibilityEvent.type != AccessibilityEventType::UNKNOWN) {
            type = ConvertAceEventType(accessibilityEvent.type);
        } else {
            type = ConvertStrToEventType(accessibilityEvent.eventType);
        }
        return type;
    }

    bool HasAccessibilityStateDescription(const RefPtr<AceType>& node)
    {
        if (!AceType::InstanceOf<NG::FrameNode>(node)) {
            return false;
        }
        auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
        CHECK_NULL_RETURN(frameNode, false);
        auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
        CHECK_NULL_RETURN(accessibilityProperty, false);
        return accessibilityProperty->HasAccessibilityStateDescription();
    }
}

bool JsAccessibilityManager::CachePageEventByController(
    const AccessibilityEvent& accessibilityEvent,
    const std::string& componentType,
    int32_t pageId,
    int32_t containerId)
{
    auto accessibilityWorkMode = GenerateAccessibilityWorkMode();
    if (!accessibilityWorkMode.isTouchExplorationEnabled) {
        return false;
    }

    AccessibilityEvent event = accessibilityEvent;
    event.componentType = componentType;
    auto eventType = GetEventTypeByAccessibilityEvent(event);
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
        "cache page event from pageEventController, componentType:%{public}s event:%{public}d nodeId:%{public}"
        PRId64, event.componentType.c_str(), eventType, event.nodeId);
    pageController_.AddAccessibilityEvent(containerId, pageId, event);
    return true;
}

bool JsAccessibilityManager::IsSendAccessibilityEvent(const AccessibilityEvent& accessibilityEvent)
{
    if (!IsPageEvent(GetEventTypeByAccessibilityEvent(accessibilityEvent))) {
        return true;
    }
    GetInfoByNodeId infoOfNode;
    auto pipelineContext = GetPipelineContext().Upgrade();
    if (!pipelineContext) {
        return IsSendAccessibilityEventForHost(accessibilityEvent, infoOfNode.componentType, infoOfNode.pageId);
    }

    GetComponentTypeAndPageIdByNodeId(accessibilityEvent.nodeId, pipelineContext, infoOfNode);
    auto container = Platform::AceContainer::GetContainer(pipelineContext->GetInstanceId());
    if (container != nullptr && container->IsUIExtensionWindow()) {
        if (!IsSendAccessibilityEventForUEA(accessibilityEvent, infoOfNode.componentType, infoOfNode.pageId)) {
            return false;
        }
        pageController_.Update();
        if (!pageController_.CheckEmpty(infoOfNode.nodeInstanceId)) {
            auto cached = CachePageEventByController(
                accessibilityEvent, infoOfNode.componentType, infoOfNode.pageId, infoOfNode.nodeInstanceId);
            return !cached;
        }
    }
    return IsSendAccessibilityEventForHost(accessibilityEvent, infoOfNode.componentType, infoOfNode.pageId);
}

void JsAccessibilityManager::ReleaseCacheEvent()
{
    if (eventQueue_.empty()) {
        return;
    }
    while (!eventQueue_.empty()) {
        auto event = eventQueue_.front();
        SendAccessibilityAsyncEvent(event);
        eventQueue_.pop();
    }
}

void JsAccessibilityManager::ReleaseUIExtCacheEvent()
{
    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    auto container = Platform::AceContainer::GetContainer(pipeline->GetInstanceId());
    CHECK_NULL_VOID(container);
    CHECK_NE_VOID(container->IsUIExtensionWindow(), true);
    CHECK_NE_VOID(pageMode_.has_value(), true);
    ReleaseCacheEvent();
}

bool JsAccessibilityManager::IsSendAccessibilityEventForUEA(
    const AccessibilityEvent& accessibilityEvent, const std::string& componentType, const int32_t pageId)
{
    if (!pageMode_.has_value()) {
        eventQueue_.push(accessibilityEvent);
        return false;
    }
    const auto& pageMode = pageMode_.value();
    if (pageMode.empty()) {
        if (treeId_ <= 0) {
            cacheEventVec_.push_back(accessibilityEvent);
            return false;
        }
        return true;
    }
    if (!CheckSendAccessibilityEventByPageMode(pageMode, componentType, pageId)) {
        return false;
    }
    if (treeId_ <= 0) {
        cacheEventVec_.push_back(accessibilityEvent);
        return false;
    }
    return true;
}

bool JsAccessibilityManager::IsSendAccessibilityEventForHost(
    const AccessibilityEvent& accessibilityEvent, const std::string& componentType, const int32_t pageId)
{
    UpdateExtensionComponentStatusVec(extensionComponentStatusVec_);
    ClearDefaultFocusList(defaultFocusList_);
    RefPtr<NG::FrameNode> findeNode;
    auto ngPipeline = FindPipelineByElementId(accessibilityEvent.nodeId, findeNode);

    pageController_.Update();
    bool isPageEventControllerEmpty = ngPipeline ? pageController_.CheckEmpty(ngPipeline->GetInstanceId()) : true;
    if (extensionComponentStatusVec_.empty() && isPageEventControllerEmpty) {
        return true;
    }

    auto containerId = ngPipeline ? ngPipeline->GetInstanceId() : 0;
    bool sameInstance = false;
    for (const auto& [node, status] : extensionComponentStatusVec_) {
        auto frameNode = node.Upgrade();
        CHECK_NULL_CONTINUE(frameNode);
        auto uecPipeline = frameNode->GetContextRefPtr();
        auto uecContainerId = uecPipeline ? uecPipeline->GetInstanceId() : 0;
        if (uecContainerId == containerId) {
            sameInstance = true;
        }
        auto nodePageId = frameNode->GetPageId();
        if (!pageIdEventMap_.count(nodePageId)) {
            pageIdEventMap_[nodePageId] = std::nullopt;
        }
    }
    if ((!CheckExtensionComponentReadyByPageId(pageId, extensionComponentStatusVec_)) && sameInstance) {
        if (pageIdEventMap_.count(pageId) && pageIdEventMap_[pageId].has_value()) {
            auto event = pageIdEventMap_[pageId].value();
            auto eventType = GetEventTypeByAccessibilityEvent(event);
            TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
                "override the event, componentType:%{public}s event:%{public}d nodeId:%{public}" PRId64,
                event.componentType.c_str(), eventType, event.nodeId);
        }
        AccessibilityEvent event = accessibilityEvent;
        event.componentType = componentType;
        pageIdEventMap_[pageId] = event;
        return false;
    } else if (!isPageEventControllerEmpty) {
        auto cached = CachePageEventByController(accessibilityEvent, componentType, pageId, containerId);
        return !cached;
    }
    return true;
}

void JsAccessibilityManager::GetComponentTypeAndPageIdByNodeId(
    const int64_t nodeId,
    const RefPtr<PipelineBase>& context,
    GetInfoByNodeId& infoOfNode)
{
    CHECK_NULL_VOID(context);
    if (AceType::InstanceOf<NG::PipelineContext>(context)) {
        RefPtr<NG::FrameNode> node;
        auto nodePipeline = FindPipelineByElementId(nodeId, node);
        CHECK_NULL_VOID(node);
        infoOfNode.componentType = node->GetTag();
        infoOfNode.pageId = node->GetPageId();
        infoOfNode.nodeInstanceId = nodePipeline ? nodePipeline->GetInstanceId() : -1;
    } else {
        auto node = GetAccessibilityNodeFromPage(nodeId);
        CHECK_NULL_VOID(node);
        infoOfNode.componentType = node->GetTag();
        infoOfNode.pageId = node->GetPageId();
        infoOfNode.nodeInstanceId = -1;
    }
}

void JsAccessibilityManager::SendCacheAccessibilityEvent(int32_t instanceId)
{
    auto container = Platform::AceContainer::GetContainer(instanceId);
    if (container == nullptr || !container->IsUIExtensionWindow()) {
        return;
    }

    if (!cacheEventVec_.empty()) {
        for (const auto& event : cacheEventVec_) {
            SendAccessibilityAsyncEventInner(event);
        }
        cacheEventVec_.clear();
    }
}

void JsAccessibilityManager::SendCacheAccessibilityEventForHost(const int32_t pageId)
{
    if (!CheckExtensionComponentReadyByPageId(pageId, extensionComponentStatusVec_)) {
        return;
    }

    if (pageIdEventMap_.count(pageId) && pageIdEventMap_[pageId].has_value()) {
        auto event = pageIdEventMap_[pageId].value();
        SendAccessibilityAsyncEventInner(event);
        pageIdEventMap_[pageId] = std::nullopt;
    }
}

bool JsAccessibilityManager::CheckPageEventValidInCache(int32_t containerId)
{
    return pageController_.HasAnyAccessibilityEvent(containerId);
}

bool JsAccessibilityManager::CheckPageEventByPageInCache(int32_t containerId, int32_t pageId)
{
    return pageController_.HasAccessibilityEvent(containerId, pageId);
}

void JsAccessibilityManager::ReleaseAccessibilityEventList(
    const std::list<std::pair<int32_t, AccessibilityEvent>>& eventList)
{
    for (auto& pair : eventList) {
        auto eventType = GetEventTypeByAccessibilityEvent(pair.second);
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
            "release page event from pageEventController, componentType:%{public}s event:%{public}d nodeId:%{public}"
            PRId64, pair.second.componentType.c_str(), eventType, pair.second.nodeId);
        SendAccessibilityAsyncEventInner(pair.second);
    }
}

void JsAccessibilityManager::ReleaseAllCacheAccessibilityEvent(int32_t containerId)
{
    std::list<std::pair<int32_t, AccessibilityEvent>> eventList;
    pageController_.ReleaseAllAccessibilityEvent(containerId, eventList);
    ReleaseAccessibilityEventList(eventList);
}

void JsAccessibilityManager::ReleaseCacheAccessibilityEvent(int32_t containerId, int32_t pageId)
{
    std::list<std::pair<int32_t, AccessibilityEvent>> eventList;
    pageController_.ReleaseAccessibilityEvent(containerId, pageId, eventList);
    ReleaseAccessibilityEventList(eventList);
    if ((pageId == 0) || (pageId == -1)) {
        ReleaseAllCacheAccessibilityEvent(containerId);
    }
}

void JsAccessibilityManager::ReleasePageEvent(const RefPtr<NG::FrameNode>& node, bool deleteController, bool releaseAll)
{
    CHECK_NULL_VOID(node);
    auto pipeline = node->GetContextRefPtr();
    CHECK_NULL_VOID(pipeline);
    auto containerId = pipeline->GetInstanceId();
    if (pageController_.CheckNode(node, deleteController)) {
        ReleaseCacheAccessibilityEvent(containerId, node->GetPageId());
    }

    if (releaseAll) {
        pageController_.DeleteInstanceNodeAllWithPriority(node);
        if (pageController_.CheckEmpty(containerId)) {
            ReleaseAllCacheAccessibilityEvent(containerId);
        }
    }
}

void JsAccessibilityManager::AddToPageEventController(const RefPtr<NG::FrameNode>& node)
{
    pageController_.Add(node);
}

bool JsAccessibilityManager::DeleteFromPageEventController(const RefPtr<NG::FrameNode>& node)
{
    return pageController_.Delete(node);
}

bool JsAccessibilityManager::CheckPageEventCached(const RefPtr<NG::FrameNode>& node, bool onlyCurrentPage)
{
    CHECK_NULL_RETURN(node, false);
    auto pipeline = node->GetContextRefPtr();
    CHECK_NULL_RETURN(pipeline, false);
    auto containerId = pipeline->GetInstanceId();

    if (onlyCurrentPage == false) {
        auto ret = CheckPageEventValidInCache(containerId);
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
            "CheckPageEventCached not onlyCurrentPage return %{public}d containerId %{public}d control Id :%{public}"
            PRId64, ret, containerId, node->GetAccessibilityId());
        return ret;
    }

    auto pageId = node->GetPageId();
    if ((pageId == 0) || (pageId == -1)) {
        auto ret = CheckPageEventValidInCache(containerId);
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
            "CheckPageEventCached pageId %{public}d return %{public}d containerId %{public}d control Id :%{public}"
            PRId64, pageId, ret, containerId, node->GetAccessibilityId());
        return ret;
    }
    auto ret = CheckPageEventByPageInCache(containerId, pageId);
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
        "CheckPageEventCached pageId %{public}d return %{public}d containerId %{public}d control Id :%{public}"
        PRId64, pageId, ret, containerId, node->GetAccessibilityId());
    return ret;
}

void JsAccessibilityManager::AddFrameNodeToUecStatusVec(const RefPtr<NG::FrameNode>& node)
{
    for (const auto& [vecNode, status] : extensionComponentStatusVec_) {
        auto frameNode = vecNode.Upgrade();
        if (frameNode && (frameNode->GetAccessibilityId() == node->GetAccessibilityId())) {
            return;
        }
    }
    extensionComponentStatusVec_.emplace_back(WeakPtr(node), false);
}

void JsAccessibilityManager::AddFrameNodeToDefaultFocusList(const RefPtr<NG::FrameNode>& node, bool isFocus)
{
    if (isFocus) {
        AddDefaultFocusNode(node);
        return;
    }
    EraseDefaultFocusNode(node);
}

void JsAccessibilityManager::AddDefaultFocusNode(const RefPtr<NG::FrameNode>& defaultFocusNode)
{
    CHECK_NULL_VOID(defaultFocusNode);
    auto nodeId = defaultFocusNode->GetId();
    for (const auto& node : defaultFocusList_) {
        auto frameNode = node.Upgrade();
        if (frameNode && (frameNode->GetId() == nodeId)) {
            return;
        }
    }
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "add frameNode to defaultFocusList, nodeId: %{public}d.", nodeId);
    defaultFocusList_.push_back(WeakPtr(defaultFocusNode));
}

void JsAccessibilityManager::EraseDefaultFocusNode(const RefPtr<NG::FrameNode>& defaultFocusNode)
{
    CHECK_NULL_VOID(defaultFocusNode);
    auto nodeId = defaultFocusNode->GetId();
    for (auto it = defaultFocusList_.begin(); it != defaultFocusList_.end();) {
        auto node = it->Upgrade();
        if (node && (node->GetId() == nodeId)) {
            it = defaultFocusList_.erase(it);
            TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
                "erase frameNode from defaultFocusList, nodeId: %{public}d.", nodeId);
            return;
        } else {
            ++it;
        }
    }
}

void JsAccessibilityManager::AddHoverTransparentCallback(const RefPtr<NG::FrameNode>& node)
{
    hoverTransparentCallbackController_.AddToHoverTransparentCallbackList(node);
}

bool JsAccessibilityManager::CheckHoverTransparentCallbackListEmpty(int32_t containerId)
{
    return hoverTransparentCallbackController_.CheckHoverTransparentCallbackListEmpty(containerId);
}

int64_t JsAccessibilityManager::CheckAndGetEmbedFrameNode(const RefPtr<NG::FrameNode>& node)
{
    auto surfaceId = GetSurfaceIdByEmbedNode(node);
    if (surfaceId == "0" || surfaceId.empty()) {
        return INVALID_NODE_ID;
    }
#ifdef WEB_SUPPORTED
    return GetWebAccessibilityIdBySurfaceId(surfaceId);
#else
    return INVALID_NODE_ID;
#endif
}

bool JsAccessibilityManager::CheckAndGetEmbedWebElementInfo(
    const RefPtr<NG::FrameNode>& node,
    AccessibilityElementInfo& elementInfo)
{
    auto surfaceId = GetSurfaceIdByEmbedNode(node);
    if (surfaceId == "0") {
        return false;
    }
#ifdef WEB_SUPPORTED
    return GetWebAccessibilityInfoBySurfaceId(surfaceId, elementInfo);
#else
    return false;
#endif
}

void JsAccessibilityManager::RegisterUIExtGetPageModeCallback(RefPtr<NG::UIExtensionManager>& uiExtManager)
{
    CHECK_NULL_VOID(uiExtManager);
    auto callback = [weak = WeakClaim(this)](const AAFwk::Want& data) -> int32_t {
        if (!data.HasParameter("pageMode")) {
            return -1;
        }
        auto accessibilityManager = weak.Upgrade();
        CHECK_NULL_RETURN(accessibilityManager, -1);

        auto pageMode = data.GetStringParam("pageMode");
        TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
            "host send pageMode to uea, pageMode: %{public}s.", pageMode.c_str());
        accessibilityManager->UpdatePageMode(pageMode);
        if (accessibilityManager->IsRegister() && accessibilityManager->GetTreeId() > 0) {
            accessibilityManager->ReleaseCacheEvent();
        }
        return 0;
    };
    uiExtManager->RegisterBusinessDataConsumeCallback(NG::UIContentBusinessCode::SEND_PAGE_MODE_TO_UEA, callback);
}

void JsAccessibilityManager::UpdateFrameNodeState(int32_t nodeId)
{
    for (auto& [node, status] : extensionComponentStatusVec_) {
        auto frameNode = node.Upgrade();
        if (frameNode && (frameNode->GetId() == nodeId)) {
            status = true;
            SendCacheAccessibilityEventForHost(frameNode->GetPageId());
            return;
        }
    }
}

bool JsAccessibilityManager::SendAccessibilitySyncEvent(
    const AccessibilityEvent& accessibilityEvent, AccessibilityEventInfo eventInfo)
{
    if (!IsRegister()) {
        return false;
    }
    auto client = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(client, false);
    bool isEnabled = false;
    client->IsEnabled(isEnabled);
    if (!isEnabled) {
        return false;
    }
    AccessibilityElementInfo info = eventInfo.GetElementInfo();
    int64_t elementId = eventInfo.GetAccessibilityId();
    if (info.GetBelongTreeId() > 0) {
        AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(info.GetBelongTreeId(), elementId);
    }
    eventInfo.SetSource(elementId);
    UpdateElementInfoTreeId(info, false);
    eventInfo.SetElementInfo(info);
    eventInfo.SetPageId(info.GetPageId());
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "send accessibility componentType:%{public}s event:%{public}d accessibilityId:%{public}" PRId64,
        eventInfo.GetComponentType().c_str(), eventInfo.GetEventType(), eventInfo.GetAccessibilityId());
    if (IsTagInEmbedComponent(eventInfo.GetComponentType()) &&
        eventInfo.GetEventType() == TYPE_VIEW_HOVER_ENTER_EVENT) {
        TAG_LOGI(AceLogTag::ACE_WEB, "dont hover on embeddedObject without same layer rendering");
        return false;
    }
    return client->SendEvent(eventInfo) == Accessibility::RET_OK;
}

bool JsAccessibilityManager::TransferAccessibilityAsyncEvent(
    const AccessibilityEventInfo& eventInfo, int64_t uiExtensionOffset)
{
#ifdef WINDOW_SCENE_SUPPORTED
    auto client = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(client, false);
    bool isEnabled = false;
    client->IsEnabled(isEnabled);
    if (!isEnabled) {
        return false;
    }

    auto pipeline = context_.Upgrade();
    CHECK_NULL_RETURN(pipeline, false);
    RefPtr<NG::PipelineContext> ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    CHECK_NULL_RETURN(ngPipeline, false);
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_RETURN(uiExtensionManager, false);
    auto container = Container::GetContainer(ngPipeline->GetInstanceId());
    bool isDynamicRender = container && container->IsDynamicRender() &&
        container->GetUIContentType() != UIContentType::ISOLATED_COMPONENT &&
        container->GetUIContentType() != UIContentType::DYNAMIC_COMPONENT;
    if (!IsRegister() && !isDynamicRender) {
        return false;
    }
    AccessibilityEventInfo eventInfoNew = eventInfo;
    if (isDynamicRender) {
        auto focusedContainer = Container::GetFocused();
        if (focusedContainer) {
            eventInfoNew.SetWindowId(focusedContainer->GetWindowId());
        }
    }
    eventInfoNew.SetSource(uiExtensionOffset * GetUiextensionId() + eventInfo.GetViewId());
    AccessibilityElementInfo elementInfo;
    FillElementInfo(eventInfoNew.GetAccessibilityId(), elementInfo, pipeline, Claim(this),
        FillEventInfoParam { eventInfoNew.GetAccessibilityId(), -1,
            static_cast<int32_t>(eventInfoNew.GetWindowId()) });
    eventInfoNew.SetElementInfo(elementInfo);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "send accessibility event:%{public}d accessibilityId:%{public}" PRId64,
        eventInfoNew.GetEventType(), eventInfoNew.GetAccessibilityId());
    return client->SendEvent(eventInfoNew);
#endif
    return false;
}

void JsAccessibilityManager::SendExtensionAccessibilityEvent(
    const AccessibilityEventInfo& eventInfo, int64_t uiExtensionOffset)
{
    TransferAccessibilityAsyncEvent(eventInfo, uiExtensionOffset);
}

void JsAccessibilityManager::FillEventInfoWithNode(
    const RefPtr<NG::FrameNode>& node,
    AccessibilityEventInfo& eventInfo,
    const RefPtr<NG::PipelineContext>& context,
    int64_t elementId)
{
    CHECK_NULL_VOID(node);
    eventInfo.SetComponentType(node->GetTag());
    eventInfo.SetPageId(node->GetPageId());
    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    eventInfo.SetItemCounts(accessibilityProperty->GetCollectionItemCounts());
    eventInfo.SetBeginIndex(accessibilityProperty->GetBeginIndex());
    eventInfo.SetEndIndex(accessibilityProperty->GetEndIndex());
    AccessibilityElementInfo elementInfo;

    CommonProperty commonProperty;
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    GenerateCommonProperty(context, commonProperty, mainContext, node);
    UpdateAccessibilityElementInfo(node, commonProperty, elementInfo, context);
    elementInfo.SetWindowId(eventInfo.GetWindowId());
    eventInfo.AddContent(elementInfo.GetContent());
    eventInfo.SetElementInfo(elementInfo);
}

int64_t JsAccessibilityManager::GetDelayTimeBeforeSendEvent(
    const AccessibilityEvent& accessibilityEvent,
    const RefPtr<AceType>& node)
{
    if (accessibilityEvent.type != AccessibilityEventType::CLICK) {
        return 0;
    }

    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    if (frameNode) {
        if (IsUserCheckedOrSelected(frameNode)) {
            return DELAY_SEND_EVENT_MILLISECOND;
        }
    } else {
        auto context = GetPipelineContext().Upgrade();
        if (!AceType::InstanceOf<NG::PipelineContext>(context)) {
            return 0;
        }
        RefPtr<NG::FrameNode> findeNode;
        auto ngPipeline = FindPipelineByElementId(accessibilityEvent.nodeId, findeNode);
        if ((findeNode) && IsUserCheckedOrSelected(findeNode)) {
            return DELAY_SEND_EVENT_MILLISECOND;
        }
    }

    return 0;
}

bool JsAccessibilityManager::IsEventIgnoredByWorkMode(const AccessibilityEvent& accessibilityEvent)
{
    if (AceApplicationInfo::GetInstance().IsAccessibilityScreenReadEnabled()) {
        return false;
    }
    if (accessibilityEvent.type == AccessibilityEventType::SCROLLING_EVENT && IsSelectReaderEnabled()) {
        return false;
    }
    switch (accessibilityEvent.type) {
        case AccessibilityEventType::ELEMENT_INFO_CHANGE:
        case AccessibilityEventType::COMPONENT_CHANGE:
        case AccessibilityEventType::TEXT_CHANGE:
        case AccessibilityEventType::FOCUS:
        case AccessibilityEventType::SCROLLING_EVENT:
            return true;
        default:
            return false;
    }
}

void JsAccessibilityManager::SendEventToAccessibilityWithNode(
    const AccessibilityEvent& accessibilityEvent, const RefPtr<AceType>& node, const RefPtr<PipelineBase>& context)
{
    // Node-aware AM routing: events fired by nodes inside a bound exported subtree must be sent by the subAM.
    if (CmpAccessibility::CmpAccessibilityRegistry::HasAnySubAM()
        && AceType::InstanceOf<NG::FrameNode>(node)) {
        auto nodeFrame = AceType::DynamicCast<NG::FrameNode>(node);
        auto ownerAM = AceType::DynamicCast<JsAccessibilityManager>(
            GetAccessibilityManagerForNode(nodeFrame));
        if (!ownerAM || ownerAM.GetRawPtr() == this) {
            // Registry-anchored fallback.
            RefPtr<NG::FrameNode> subtreeNode;
            auto subAMBase = CmpAccessibility::CmpAccessibilityRegistry::
                FindSubAMAndNodeInExportedSubtrees(nodeFrame->GetAccessibilityId(), subtreeNode);
            if (subAMBase) {
                ownerAM = AceType::DynamicCast<JsAccessibilityManager>(subAMBase);
            }
        }
        if (ownerAM && ownerAM.GetRawPtr() != this) {
            ownerAM->SendEventToAccessibilityWithNode(accessibilityEvent, node, context);
            return;
        }
    }
    auto eventType = GetEventTypeByAccessibilityEvent(accessibilityEvent);
    if (!CheckWhiteList(eventType)) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
            "send accessibility check whitelist failed event:%{public}d" PRId64, eventType);
        return;
    }

    if (IsEventIgnoredByWorkMode(accessibilityEvent) || !IsSendAccessibilityEvent(accessibilityEvent)) {
        return;
    }

    auto delayTime = GetDelayTimeBeforeSendEvent(accessibilityEvent, node);
    if ((delayTime > 0) && context) {
        context->GetTaskExecutor()->PostDelayedTask(
            [weak = WeakClaim(this), accessibilityEvent, node, context, eventType] {
                auto jsAccessibilityManager = weak.Upgrade();
                CHECK_NULL_VOID(jsAccessibilityManager);
                if (eventType == Accessibility::EventType::TYPE_VIEW_CLICKED_EVENT) {
                    jsAccessibilityManager->SendEventToAccessibilityWithNodeInnerAfterRender(
                        accessibilityEvent, node, context);
                } else {
                    jsAccessibilityManager->SendEventToAccessibilityWithNodeInner(accessibilityEvent, node, context);
                }
            },
            TaskExecutor::TaskType::UI, delayTime, "ArkUIAccessibilitySendSyncEventWithDelay");
        return;
    }
    if (eventType == Accessibility::EventType::TYPE_VIEW_CLICKED_EVENT) {
        SendEventToAccessibilityWithNodeInnerAfterRender(accessibilityEvent, node, context);
    } else {
        SendEventToAccessibilityWithNodeInner(accessibilityEvent, node, context);
    }
}

void JsAccessibilityManager::SendEventToAccessibilityWithNodeInner(
    const AccessibilityEvent& accessibilityEvent, const RefPtr<AceType>& node, const RefPtr<PipelineBase>& context)
{
    ACE_SCOPED_TRACE("SendAccessibilityAsyncEvent");
    CHECK_NULL_VOID(node);
    CHECK_NULL_VOID(context);
    int32_t windowId = static_cast<int32_t>(context->GetRealHostWindowId());
    if (windowId == 0) {
        return;
    }
    if (!AceType::InstanceOf<NG::FrameNode>(node)) {
        return;
    }
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    CHECK_NULL_VOID(frameNode);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);

    auto blockEvent = blockerInAction_.ShouldBlock(frameNode->GetAccessibilityId(), accessibilityEvent.type);
    CHECK_EQUAL_VOID(blockEvent, true);

    if ((!frameNode->IsActive()) || frameNode->CheckAccessibilityLevelNo()) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "node: %{public}" PRId64 ", is not active or level is no",
            frameNode->GetAccessibilityId());
        return;
    }

    AccessibilityEventInfo eventInfo;

    if (accessibilityEvent.type != AccessibilityEventType::PAGE_CHANGE || accessibilityEvent.windowId == 0) {
        eventInfo.SetWindowId(windowId);
    } else {
        eventInfo.SetWindowId(accessibilityEvent.windowId);
    }
    FillEventInfoWithNode(frameNode, eventInfo, ngPipeline, accessibilityEvent.nodeId);
    if ((ngPipeline != nullptr) && (ngPipeline->IsFormRender())) {
        eventInfo.SetWindowId(static_cast<int32_t>(GetWindowId()));
    }
    GenerateAccessibilityEventInfo(accessibilityEvent, eventInfo);
    CompensateCmpEventRect(eventInfo);

    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), accessibilityEvent, eventInfo] {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            jsAccessibilityManager->SendAccessibilitySyncEvent(accessibilityEvent, eventInfo);
        },
        TaskExecutor::TaskType::BACKGROUND, "ArkUIAccessibilitySendSyncEvent");
}

void JsAccessibilityManager::SendEventToAccessibilityWithNodeInnerAfterRender(
    const AccessibilityEvent& accessibilityEvent, const RefPtr<AceType>& node, const RefPtr<PipelineBase>& context)
{
    CHECK_NULL_VOID(context);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    if (ngPipeline && HasAccessibilityStateDescription(node)) {
        ngPipeline->AddAfterRenderTask(
            [weak = WeakClaim(this), weakNode = WeakClaim(AceType::RawPtr(node)),
                weakContext = WeakClaim(AceType::RawPtr(context)), accessibilityEvent]() {
                auto jsAccessibilityManager = weak.Upgrade();
                CHECK_NULL_VOID(jsAccessibilityManager);
                auto node = weakNode.Upgrade();
                CHECK_NULL_VOID(node);
                auto context = weakContext.Upgrade();
                CHECK_NULL_VOID(context);
                jsAccessibilityManager->SendEventToAccessibilityWithNodeInner(accessibilityEvent, node, context);
            }
        );
        ngPipeline->RequestFrame();
    } else {
        SendEventToAccessibilityWithNodeInner(accessibilityEvent, node, context);
    }
}

void GetRealEventWindowId(
    const AccessibilityEvent& accessibilityEvent, const RefPtr<NG::PipelineContext>& ngPipeline, uint32_t& windowId)
{
    if ((accessibilityEvent.type == AccessibilityEventType::PAGE_CHANGE && accessibilityEvent.windowId != 0) ||
        accessibilityEvent.windowChangeTypes == WINDOW_UPDATE_ADDED) {
        windowId = accessibilityEvent.windowId;
        return;
    }
    if ((ngPipeline != nullptr) && (ngPipeline->IsFormRender())) {
        return;
    }
    windowId = ngPipeline->GetRealHostWindowId();
}

bool CheckAndGetVirtualNode(int64_t elementId,
    RefPtr<NG::VirtualAccessibilityNode>& virtualNode, RefPtr<NG::FrameNode>& parentNode)
{
    CHECK_NE_RETURN(NG::VirtualNodeContainerIdManager::IsVirtualNodeContainerId(elementId), true, false);
    uint8_t containerId = NG::VirtualNodeContainerIdManager::ExtractContainerId(elementId);
    parentNode = NG::VirtualNodeContainerIdManager::GetInstance().GetContainerNode(containerId);
    CHECK_NULL_RETURN(parentNode, false);
    auto accessibilityProperty = parentNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, false);
    CHECK_NE_RETURN(accessibilityProperty->HasVirtualNodeTreeRoot(), true, false);
    auto virtualRoot = accessibilityProperty->GetVirtualNodeTreeRoot();
    CHECK_NULL_RETURN(virtualRoot, false);
    auto virtualNodeRoot = AceType::DynamicCast<NG::VirtualAccessibilityNode>(virtualRoot);
    CHECK_NULL_RETURN(virtualNodeRoot, false);
    int32_t virtualNodeId = static_cast<int32_t>(NG::VirtualNodeContainerIdManager::ExtractComponentId(elementId));
    virtualNode = virtualNodeRoot->FindNodeById(virtualNodeId);
    return virtualNode != nullptr;
}

bool CheckAndGetVirtualNodeTree(int64_t elementId,
    RefPtr<NG::VirtualAccessibilityNode>& virtualNode, RefPtr<NG::FrameNode>& parentNode)
{
    CHECK_NE_RETURN(NG::VirtualNodeContainerIdManager::IsVirtualNodeContainerId(elementId), true, false);
    uint8_t containerId = NG::VirtualNodeContainerIdManager::ExtractContainerId(elementId);
    parentNode = NG::VirtualNodeContainerIdManager::GetInstance().GetContainerNode(containerId);
    CHECK_NULL_RETURN(parentNode, false);
    auto accessibilityProperty = parentNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, false);
    CHECK_NE_RETURN(accessibilityProperty->HasVirtualNodeTreeRoot(), true, false);
    auto virtualRoot = accessibilityProperty->GetVirtualNodeTreeRoot();
    CHECK_NULL_RETURN(virtualRoot, false);
    auto virtualNodeRoot = AceType::DynamicCast<NG::VirtualAccessibilityNode>(virtualRoot);
    CHECK_NULL_RETURN(virtualNodeRoot, false);
    int32_t virtualNodeId = static_cast<int32_t>(NG::VirtualNodeContainerIdManager::ExtractComponentId(elementId));
    virtualNode = virtualNodeRoot->FindNodeById(virtualNodeId);
    return virtualNode != nullptr;
}

void JsAccessibilityManager::SendAccessibilityAsyncEvent(const AccessibilityEvent& accessibilityEvent)
{
    auto accessibilityId = accessibilityEvent.nodeId;
    if (NG::VirtualNodeContainerIdManager::IsVirtualNodeContainerId(accessibilityId)) {
        RefPtr<NG::VirtualAccessibilityNode> virtualNode;
        RefPtr<NG::FrameNode> parentNode;
        auto find = CheckAndGetVirtualNode(accessibilityId, virtualNode, parentNode);
        if (find && virtualNode) {
            SendEventToAccessibilityWithVirtualNode(
                accessibilityEvent, parentNode, virtualNode, parentNode->GetContextRefPtr());
            return;
        }
    }
    // Node-aware AM routing (mirror SendEventToAccessibilityWithNode): events fired via the single-argument.
    if (CmpAccessibility::CmpAccessibilityRegistry::HasAnySubAM()) {
        RefPtr<NG::PipelineContext> ngPipeline;
        RefPtr<NG::FrameNode> eventNode;
        ngPipeline = FindPipelineByElementId(accessibilityEvent.nodeId, eventNode);
        if (eventNode) {
            auto ownerAM = AceType::DynamicCast<JsAccessibilityManager>(
                GetAccessibilityManagerForNode(eventNode));
            if (!ownerAM || ownerAM.GetRawPtr() == this) {
                // GetSubAM's parent-chain walk-up misses in-subtree nodes with broken parent_ links.
                RefPtr<NG::FrameNode> subtreeNode;
                auto subAMBase = CmpAccessibility::CmpAccessibilityRegistry::
                    FindSubAMAndNodeInExportedSubtrees(accessibilityEvent.nodeId, subtreeNode);
                if (subAMBase) {
                    ownerAM = AceType::DynamicCast<JsAccessibilityManager>(subAMBase);
                }
            }
            if (ownerAM && ownerAM.GetRawPtr() != this) {
                ownerAM->SendAccessibilityAsyncEvent(accessibilityEvent);
                return;
            }
        }
    }
    auto eventType = GetEventTypeByAccessibilityEvent(accessibilityEvent);
    if (!CheckWhiteList(eventType)) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
            "send accessibility check whitelist failed event:%{public}d" PRId64, eventType);
        return;
    }
    if (IsEventIgnoredByWorkMode(accessibilityEvent) || !IsSendAccessibilityEvent(accessibilityEvent)) {
        return;
    }

    auto delayTime = GetDelayTimeBeforeSendEvent(accessibilityEvent, nullptr);
    if (delayTime > 0) {
        auto context = GetPipelineContext().Upgrade();
        if (context) {
            context->GetTaskExecutor()->PostDelayedTask(
                [weak = WeakClaim(this), accessibilityEvent, eventType] {
                    auto jsAccessibilityManager = weak.Upgrade();
                    CHECK_NULL_VOID(jsAccessibilityManager);
                    if (eventType == Accessibility::EventType::TYPE_VIEW_CLICKED_EVENT) {
                        jsAccessibilityManager->SendAccessibilityAsyncEventInnerAfterRender(accessibilityEvent);
                    } else {
                        jsAccessibilityManager->SendAccessibilityAsyncEventInner(accessibilityEvent);
                    }
                },
                TaskExecutor::TaskType::UI, delayTime, "ArkUIAccessibilitySendSyncEventWithDelay");
        }
        return;
    }
    if (eventType == Accessibility::EventType::TYPE_VIEW_CLICKED_EVENT) {
        SendAccessibilityAsyncEventInnerAfterRender(accessibilityEvent);
    } else {
        SendAccessibilityAsyncEventInner(accessibilityEvent);
    }
}

void JsAccessibilityManager::SendAccessibilityAsyncEventInner(const AccessibilityEvent& accessibilityEvent)
{
    ACE_ACCESS_SCOPED_TRACE("SendAccessibilityAsyncEvent");
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    int32_t windowId = static_cast<int32_t>(context->GetRealHostWindowId());
    if (windowId == 0) {
        return;
    }
    RefPtr<NG::PipelineContext> ngPipeline;
    AccessibilityEventInfo eventInfo;
    uint32_t realWindowId = GetWindowId();
    if (AceType::InstanceOf<NG::PipelineContext>(context)) {
        RefPtr<NG::FrameNode> node;
        ngPipeline = FindPipelineByElementId(accessibilityEvent.nodeId, node);
        if (!ngPipeline || !node) {
            TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
                "SendA11yAsyncInner drop: node resolve failed, id=%{public}" PRId64
                ", pipelineNull=%{public}d, nodeNull=%{public}d",
                accessibilityEvent.nodeId, ngPipeline ? 0 : 1, node ? 0 : 1);
            return;
        }
        GetRealEventWindowId(accessibilityEvent, ngPipeline, realWindowId);
        FillEventInfo(node, eventInfo, ngPipeline, Claim(this),
            FillEventInfoParam {
                accessibilityEvent.nodeId, accessibilityEvent.stackNodeId, realWindowId });
        eventInfo.SetWindowId(realWindowId);
    } else {
        ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
        auto node = GetAccessibilityNodeFromPage(accessibilityEvent.nodeId);
        CHECK_NULL_VOID(node);
        FillEventInfo(node, eventInfo);
        eventInfo.SetWindowId(windowId);
    }
    if (accessibilityEvent.type == AccessibilityEventType::PAGE_CHANGE && accessibilityEvent.windowId != 0) {
        eventInfo.SetWindowId(accessibilityEvent.windowId);
    }
    if (ngPipeline != nullptr) {
        if (ngPipeline->IsFormRender()) {
            eventInfo.SetWindowId(static_cast<int32_t>(GetWindowId()));
        }
    }
    GenerateAccessibilityEventInfo(accessibilityEvent, eventInfo);
    CompensateCmpEventRect(eventInfo);

    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), accessibilityEvent, eventInfo] {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            jsAccessibilityManager->SendAccessibilitySyncEvent(accessibilityEvent, eventInfo);
        },
        TaskExecutor::TaskType::BACKGROUND, "ArkUIAccessibilitySendSyncEvent");
}

void JsAccessibilityManager::SendAccessibilityAsyncEventInnerAfterRender(const AccessibilityEvent& accessibilityEvent)
{
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    RefPtr<NG::FrameNode> node;
    FindPipelineByElementId(accessibilityEvent.nodeId, node);
    if (ngPipeline && node && HasAccessibilityStateDescription(node)) {
        ngPipeline->AddAfterRenderTask(
            [weak = WeakClaim(this), accessibilityEvent]() {
                auto jsAccessibilityManager = weak.Upgrade();
                CHECK_NULL_VOID(jsAccessibilityManager);
                jsAccessibilityManager->SendAccessibilityAsyncEventInner(accessibilityEvent);
            }
        );
        ngPipeline->RequestFrame();
    } else {
        SendAccessibilityAsyncEventInner(accessibilityEvent);
    }
}

#ifdef WEB_SUPPORTED

void JsAccessibilityManager::SendWebAccessibilityAsyncEvent(
    const AccessibilityEvent& accessibilityEvent, const RefPtr<NG::WebPattern>& webPattern)
{
    ACE_ACCESS_SCOPED_TRACE("SendWebAccessibilityAsyncEvent");
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    int32_t windowId = static_cast<int32_t>(context->GetRealHostWindowId());
    if (windowId == 0) {
        return;
    }

    AccessibilityEventInfo eventInfo;
    RefPtr<NG::PipelineContext> ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context_.Upgrade());
    CHECK_NULL_VOID(ngPipeline);
    FillWebEventInfo(eventInfo, ngPipeline, Claim(this),
        FillEventInfoParam {
                accessibilityEvent.nodeId, accessibilityEvent.stackNodeId, ngPipeline->GetRealHostWindowId() },
        webPattern);
    eventInfo.SetWindowId(ngPipeline->GetRealHostWindowId());

    if (accessibilityEvent.type == AccessibilityEventType::PAGE_CHANGE && accessibilityEvent.windowId != 0) {
        eventInfo.SetWindowId(accessibilityEvent.windowId);
    }

    GenerateAccessibilityEventInfo(accessibilityEvent, eventInfo);
    CompensateCmpEventRect(eventInfo);

    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), accessibilityEvent, eventInfo] {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            jsAccessibilityManager->SendAccessibilitySyncEvent(accessibilityEvent, eventInfo);
        },
        TaskExecutor::TaskType::BACKGROUND, "ArkUIAccessibilitySendSyncEvent");
}
#endif

void JsAccessibilityManager::UpdateNodeChildIds(const RefPtr<AccessibilityNode>& node)
{
    CHECK_NULL_VOID(node);
    node->ActionUpdateIds();
    const auto& children = node->GetChildList();
    std::vector<int32_t> childrenVec;
    auto cardId = GetCardId();
    auto rootNodeId = GetRootNodeId();

    // get last stack children to barrier free service.
    if ((node->GetNodeId() == GetRootNodeId() + ROOT_STACK_BASE) && !children.empty() && !IsDeclarative()) {
        auto lastChildNodeId = children.back()->GetNodeId();
        if (isOhosHostCard()) {
            childrenVec.emplace_back(ConvertToCardAccessibilityId(lastChildNodeId, cardId, rootNodeId));
        } else {
            childrenVec.emplace_back(lastChildNodeId);
            for (const auto& child : children) {
                if (child->GetNodeId() == ROOT_DECOR_BASE - 1) {
                    childrenVec.emplace_back(child->GetNodeId());
                    break;
                }
            }
        }
    } else {
        childrenVec.resize(children.size());
        if (isOhosHostCard()) {
            std::transform(children.begin(), children.end(), childrenVec.begin(),
                [cardId, rootNodeId](const RefPtr<AccessibilityNode>& child) {
                    return ConvertToCardAccessibilityId(child->GetNodeId(), cardId, rootNodeId);
                });
        } else {
            std::transform(children.begin(), children.end(), childrenVec.begin(),
                [](const RefPtr<AccessibilityNode>& child) { return child->GetNodeId(); });
        }
    }
    node->SetChildIds(childrenVec);
}

void JsAccessibilityManager::ProcessParameters(
    ActionType op, const std::vector<std::string>& params, std::map<std::string, std::string>& paramsMap)
{
    if (op == ActionType::ACCESSIBILITY_ACTION_SET_TEXT) {
        if (params.size() == (EVENT_DUMP_ACTION_PARAM_INDEX + 1)) {
            paramsMap = { { ACTION_ARGU_SET_TEXT, params[EVENT_DUMP_ACTION_PARAM_INDEX] } };
        }
    }

    if (op == ActionType::ACCESSIBILITY_ACTION_SET_SELECTION) {
        paramsMap[ACTION_ARGU_SELECT_TEXT_START] = "-1";
        paramsMap[ACTION_ARGU_SELECT_TEXT_END] = "-1";
        paramsMap[ACTION_ARGU_SELECT_TEXT_INFORWARD] = STRING_DIR_BACKWARD;
        if (params.size() > EVENT_DUMP_PARAM_LENGTH_LOWER) {
            paramsMap[ACTION_ARGU_SELECT_TEXT_START] = params[EVENT_DUMP_ACTION_PARAM_INDEX];
        }
        if (params.size() > EVENT_DUMP_PARAM_LENGTH_LOWER + 1) {
            paramsMap[ACTION_ARGU_SELECT_TEXT_END] = params[EVENT_DUMP_ACTION_PARAM_INDEX + 1];
        }
        // 2 means params number Offset
        if (params.size() > EVENT_DUMP_PARAM_LENGTH_LOWER + 2) {
            // 2 means params number Offset
            paramsMap[ACTION_ARGU_SELECT_TEXT_INFORWARD] = params[EVENT_DUMP_ACTION_PARAM_INDEX + 2];
        }
    }

    if (op == ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT || op == ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT) {
        if (params.size() == EVENT_DUMP_PARAM_LENGTH_UPPER) {
            paramsMap[ACTION_ARGU_MOVE_UNIT] = std::to_string(TextMoveUnit::STEP_CHARACTER);
        }
        paramsMap[ACTION_ARGU_MOVE_UNIT] = std::to_string(TextMoveUnit::STEP_CHARACTER);
    }

    if (op == ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION) {
        paramsMap[ACTION_ARGU_SET_OFFSET] = params[EVENT_DUMP_ACTION_PARAM_INDEX];
    }

    if ((op == ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD) ||
        (op == ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD)) {
        if (params.size() > EVENT_DUMP_PARAM_LENGTH_LOWER) {
            paramsMap = { { ACTION_ARGU_SCROLL_STUB, params[EVENT_DUMP_ACTION_PARAM_INDEX] } };
        }
    }

    if (op == ActionType::ACCESSIBILITY_ACTION_SPAN_CLICK) {
        if (params.size() > EVENT_DUMP_PARAM_LENGTH_LOWER) {
            paramsMap = { { ACTION_ARGU_SPAN_ID, params[EVENT_DUMP_ACTION_PARAM_INDEX] } };
        }
    }

    if (op == ActionType::ACCESSIBILITY_ACTION_CUSTOM) {
        if (params.size() > EVENT_DUMP_PARAM_LENGTH_LOWER) {
            paramsMap = { { ACTION_ARGU_CUSTOM_ACTION, params[EVENT_DUMP_ACTION_PARAM_INDEX] } };
        }
    }
}

bool TransferExecuteAction(int64_t elementId, const RefPtr<NG::FrameNode>& node,
    const std::map<std::string, std::string>& actionArguments,
    ActionType action, int64_t uiExtensionOffset)
{
    bool isExecuted = false;
    if ((uiExtensionOffset + 1) > NG::UI_EXTENSION_OFFSET_MIN) {
        isExecuted = node->TransferExecuteAction(
            elementId, actionArguments, static_cast<int>(action),
            uiExtensionOffset / NG::UI_EXTENSION_ID_FACTOR);
    }
    return isExecuted;
}

RefPtr<NG::PipelineContext> JsAccessibilityManager::GetPipelineByWindowId(uint32_t windowId)
{
    auto mainPipeline = AceType::DynamicCast<NG::PipelineContext>(context_.Upgrade());
    if (mainPipeline!= nullptr && mainPipeline->GetWindowId() == windowId) {
        return mainPipeline;
    }
    for (auto subPipelineWeak : GetSubPipelineContexts()) {
        auto subContextNG = AceType::DynamicCast<NG::PipelineContext>(subPipelineWeak.Upgrade());
        if (subContextNG!= nullptr && subContextNG->GetWindowId() == windowId) {
            return subContextNG;
        }
    }
    if (GetWindowId() == windowId) {
        return mainPipeline;
    }
    return nullptr;
}

// DFX related
namespace {

bool CheckAndGetEventTestArgument(std::vector<std::string>::const_iterator start,
    const std::vector<std::string>& params, DumpInfoArgument& argument)
{
    auto arg = start;
    argument.mode = DumpMode::EVENT_TEST;
    constexpr int32_t NUM_EVENT_DIMENSION = 2;
    if (std::distance(arg, params.end()) <= NUM_EVENT_DIMENSION) {
        DumpLog::GetInstance().Print(std::string("Error: --event-test is used to send event with node ") +
            "need elementId and eventId, e.g. '--event-test ${elementId} ${eventId}'!");
        return false;
    }
    ++arg;
    argument.nodeId = StringUtils::StringToLongInt(*arg);
    ++arg;
    argument.eventId = StringUtils::StringToInt(*arg);
    return true;
}

bool CheckAndGetHoverTestArgument(std::vector<std::string>::const_iterator& arg,
    const std::vector<std::string>& params, DumpInfoArgument& argument)
{
    argument.mode = DumpMode::HOVER_TEST;
    static constexpr int32_t NUM_POINT_DIMENSION = 2;
    if (std::distance(arg, params.end()) <= NUM_POINT_DIMENSION) {
        DumpLog::GetInstance().Print(std::string("Error: --hover-test is used to get nodes at a point ") +
            "relative to the root node, e.g. '--hover-test ${x} ${y}'!");
        return false;
    }
    ++arg;
    argument.pointX = StringUtils::StringToInt(*arg);
    ++arg;
    argument.pointY = StringUtils::StringToInt(*arg);
    return true;
}

bool DumpProcessEmbedSearchParameters(
    const std::vector<std::string>& params,
    int64_t& nodeId,
    SearchSurfaceIdType& searchType)
{
    constexpr int32_t NUM_PARAMETERS_DIMENSION = 2;
    if (params.size() < 1) {
        return false;
    }

    for (auto arg = params.begin() + 1; arg != params.end(); ++arg) {
        // Process the embed node parameters and dump its subtree info.
        if (*arg == "--embed-search") {
            if (std::distance(arg, params.end()) <= NUM_PARAMETERS_DIMENSION) {
                DumpLog::GetInstance().Print(std::string("Error: parameters need with data"));
                return false;
            }
            ++arg;
            nodeId = StringUtils::StringToLongInt(*arg);
            ++arg;
            searchType = static_cast<SearchSurfaceIdType>(StringUtils::StringToInt(*arg));
            return true;
        }
    }
    return false;
}

bool DumpProcessEmbedHoverParameters(
    const std::vector<std::string>& params,
    int64_t& nodeId,
    int32_t& x,
    int32_t& y)
{
    constexpr int32_t NUM_PARAMETERS_DIMENSION = 3;
    if (params.size() < 1) {
        return false;
    }

    for (auto arg = params.begin() + 1; arg != params.end(); ++arg) {
        // Process the embed node parameters and hover test.
        if (*arg == "--embed-hover") {
            if (std::distance(arg, params.end()) <= NUM_PARAMETERS_DIMENSION) {
                DumpLog::GetInstance().Print(std::string("Error: parameters need with data"));
                return false;
            }
            ++arg;
            nodeId = StringUtils::StringToLongInt(*arg);
            ++arg;
            x = StringUtils::StringToInt(*arg);
            ++arg;
            y = StringUtils::StringToInt(*arg);
            return true;
        }
    }
    return false;
}

bool DumpProcessSetCheckListParameters(
    const std::vector<std::string>& params,
    std::vector<uint32_t>& vec)
{
    if (params.size() < MIN_NUM) {
        return false;
    }
    auto arg = params.begin() + 1;
    if (*arg != "--set-whitelist") {
        return false;
    }
    for (++arg; arg != params.end();) {
        vec.emplace_back(StringUtils::StringToUint(*arg++));
    }
    return true;
}

bool DumpProcessGetCheckListParameters(
    const std::vector<std::string>& params)
{
    if (params.size() < MIN_NUM) {
        return false;
    }
    auto arg = params.begin() + 1;
    if (*arg != "--get-whitelist") {
        return false;
    }
    return true;
}

bool DumpProcessSpecificSearchParameters(
    const std::vector<std::string>& params,
    int64_t& nodeId,
    std::string& propertyTarget,
    int32_t& propertyType)
{
    constexpr int32_t NUM_PARAMETERS_DIMENSION = 3;
    if (params.size() < MIN_PARAMS_SIZE) {
        return false;
    }

    for (auto arg = params.begin() + 1; arg != params.end(); ++arg) {
        if (*arg == "--specific-search") {
            if (std::distance(arg, params.end()) <= NUM_PARAMETERS_DIMENSION) {
                DumpLog::GetInstance().Print(std::string("Error: parameters need with data"));
                return false;
            }
            ++arg;
            nodeId = StringUtils::StringToLongInt(*arg);
            ++arg;
            propertyTarget = *arg;
            ++arg;
            propertyType = StringUtils::StringToInt(*arg);
            return true;
        }
    }
    return false;
}
} // DFX related

void JsAccessibilityManager::DumpInjectActionTest(const std::vector<std::string>& params)
{
    int64_t nodeId = 0;
    int32_t result = 0;
    InjectActionType actionType = InjectActionType::UNDEFINED_ACTION;

    if (!AccessibilityHidumper::DumpProcessInjectActionParameters(params, nodeId, result, actionType)) {
        return;
    }

    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    RefPtr<NG::PipelineContext> ngPipeline;

    RefPtr<NG::FrameNode> frameNode;
    ngPipeline = FindPipelineByElementId(nodeId, frameNode);
    CHECK_NULL_VOID(ngPipeline);
    CHECK_NULL_VOID(frameNode);
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    if (actionType == InjectActionType::NOTIFY_CHILD_ACTION) {
        accessibilityProperty->SetNotifyChildAction([nodeId, result] (
            const RefPtr<NG::FrameNode>& node,
            NotifyChildActionType childActionType) {
                TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "onNotifyChildAction callback: nodeid %{public}" \
                    PRId64 " result %{public}d", nodeId, result);
                return static_cast<AccessibilityActionResult>(result);
        });
    }
    if (actionType == InjectActionType::SECURITY_CLICK_ACTION) {
        accessibilityProperty->SetSecurityClickAction (
            [nodeId] (const NG::SecCompEnhanceEvent& event) {
                std::string hmac(event.dataBuffer.begin(), event.dataBuffer.end());
                uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(
                    event.time.time_since_epoch()).count();
                TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "onSecurityClickAction callback: nodeid %{public}" \
                    PRId64 " hmac %{public}s timesttamp %{public}" PRId64, nodeId, hmac.c_str(), time);
            }
        );
    }
    DumpLog::GetInstance().Print(std::string("Result: inject action done"));
}

void JsAccessibilityManager::DumpCustomActionTest(const std::vector<std::string>& params)
{
    int64_t nodeId = 0;
    std::string actionName;
    bool listActions = false;

    if (!AccessibilityHidumper::DumpProcessCustomActionParameters(params, nodeId, actionName, listActions)) {
        return;
    }

    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    RefPtr<NG::PipelineContext> ngPipeline;

    RefPtr<NG::FrameNode> frameNode;
    ngPipeline = FindPipelineByElementId(nodeId, frameNode);
    CHECK_NULL_VOID(ngPipeline);
    CHECK_NULL_VOID(frameNode);
    AccessibilityManagerHidumper::DumpCustomActionTest(params, frameNode);
}

void JsAccessibilityManager::DumpSetComponentTypeTest(const std::vector<std::string>& params)
{
    constexpr size_t setComponentTypeParamSize = 4;
    constexpr size_t nodeIdIndex = 2;
    constexpr size_t componentTypeIndex = 3;
    if (params.size() != setComponentTypeParamSize || params[componentTypeIndex].empty()) {
        DumpLog::GetInstance().Print(
            "Usage: -accessibility --set-component-type <accessibilityId> <componentType>");
        return;
    }

    auto nodeId = StringUtils::StringToLongInt(params[nodeIdIndex]);
    RefPtr<NG::FrameNode> frameNode;
    auto pipeline = FindPipelineByElementId(nodeId, frameNode);
    if (!pipeline || !frameNode) {
        DumpLog::GetInstance().Print("Error: can't find node with ID " + params[nodeIdIndex]);
        return;
    }

    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    accessibilityProperty->SetAccessibilityRole(params[componentTypeIndex]);
    DumpLog::GetInstance().Print(
        "Set accessibility componentType success. nodeId: " + std::to_string(frameNode->GetAccessibilityId()) +
        ", componentType: " + params[componentTypeIndex]);
}

void JsAccessibilityManager::DumpClearComponentTypeTest(const std::vector<std::string>& params)
{
    constexpr size_t clearComponentTypeParamSize = 3;
    constexpr size_t nodeIdIndex = 2;
    if (params.size() != clearComponentTypeParamSize) {
        DumpLog::GetInstance().Print("Usage: -accessibility --clear-component-type <accessibilityId>");
        return;
    }

    auto nodeId = StringUtils::StringToLongInt(params[nodeIdIndex]);
    RefPtr<NG::FrameNode> frameNode;
    auto pipeline = FindPipelineByElementId(nodeId, frameNode);
    if (!pipeline || !frameNode) {
        DumpLog::GetInstance().Print("Error: can't find node with ID " + params[nodeIdIndex]);
        return;
    }

    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    accessibilityProperty->ResetAccessibilityRole();
    DumpLog::GetInstance().Print(
        "Clear accessibility componentType success. nodeId: " + std::to_string(frameNode->GetAccessibilityId()));
}

void JsAccessibilityManager::DumpEmbedSearchTest(const std::vector<std::string>& params)
{
    int64_t nodeId = 0;
    SearchSurfaceIdType searchType = SearchSurfaceIdType::SEARCH_ALL;

    if (!DumpProcessEmbedSearchParameters(params, nodeId, searchType)) {
        return;
    }

    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    RefPtr<NG::PipelineContext> ngPipeline;

    RefPtr<NG::FrameNode> frameNode;
    ngPipeline = FindPipelineByElementId(nodeId, frameNode);
    CHECK_NULL_VOID(ngPipeline);
    CHECK_NULL_VOID(frameNode);
    auto surfaceId = GetSurfaceIdByEmbedNode(frameNode);
    if (surfaceId.empty()) {
        DumpLog::GetInstance().Print(std::string("Result: embed search not have surfaceId"));
        return;
    }
    std::list<AccessibilityElementInfo> infos;

    SearchElementInfoBySurfaceId(surfaceId, ngPipeline->GetWindowId(), searchType, infos);
    if (infos.empty()) {
        DumpLog::GetInstance().Print(std::string("Result: embed search empty"));
        return;
    }
    auto accessibilityId = infos.front().GetAccessibilityId();
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(
        accessibilityId, splitElementId, splitTreeId);

    for (auto& info : infos) {
        DumpCommonPropertyNG(info, splitTreeId);
    }
    DumpLog::GetInstance().Print(0, surfaceId, infos.size());
    DumpLog::GetInstance().Print(std::string("Result: embed search done"));
}

void JsAccessibilityManager::DumpEmbedHoverTestNG(const std::vector<std::string>& params, uint32_t windowId)
{
    int64_t rootId = INVALID_NODE_ID;
    int32_t x = 0;
    int32_t y = 0;
    if (!DumpProcessEmbedHoverParameters(params, rootId, x, y)) {
        return;
    }

    auto pipeline = GetPipelineByWindowId(windowId);
    CHECK_NULL_VOID(pipeline);
    auto accessibilityManagerNG = pipeline->GetAccessibilityManagerNG();
    CHECK_NULL_VOID(accessibilityManagerNG);
    auto pipelineRoot = pipeline->GetRootElement();
    RefPtr<NG::FrameNode> root = nullptr;
    if (rootId == -1) {
        root = pipelineRoot;
    } else {
        root = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(pipelineRoot, rootId);
    }
    CHECK_NULL_VOID(root);

    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    CommonProperty commonProperty;
    GenerateCommonProperty(pipeline, commonProperty, mainContext, root);
    AccessibilityElementInfo elementInfo;
    UpdateAccessibilityElementInfo(root, commonProperty, elementInfo, pipeline);
    x -= elementInfo.GetRectInScreen().GetLeftTopXScreenPostion();
    y -= elementInfo.GetRectInScreen().GetLeftTopYScreenPostion();

    DumpLog::GetInstance().Print("Window ID: " + std::to_string(windowId));
    DumpLog::GetInstance().Print("Root ID: " + std::to_string(root->GetAccessibilityId()));
    NG::PointF hoverPoint(x, y);
    DumpLog::GetInstance().Print("Hover Point: " + hoverPoint.ToString());
    auto surfaceId = GetSurfaceIdByEmbedNode(root);
    if (surfaceId.empty()) {
        DumpLog::GetInstance().Print(std::string("Result: embed hover test not have surfaceId"));
        return;
    }
    DumpLog::GetInstance().Print("surfaceId: " + surfaceId);

    TimeStamp time;
    NG::HandleHoverEventParam param = {
        hoverPoint, SourceType::TOUCH, NG::AccessibilityHoverEventType::ENTER, time, true };
    accessibilityManagerNG->HandleAccessibilityHoverEventBySurfaceId(surfaceId, param);
}

void JsAccessibilityManager::DumpSpecificPropertySearchTest(const std::vector<std::string>& params, uint32_t windowId)
{
    int64_t nodeId = INVALID_NODE_ID;
    std::string propertyTarget;
    int32_t propertyType = 0;
    if (!DumpProcessSpecificSearchParameters(params, nodeId, propertyTarget, propertyType)) {
        return;
    }

    if (propertyType != SEARCH_TYPE::CUSTOMID) {
        return;
    }
    auto pipeline = GetPipelineByWindowId(windowId);
    CHECK_NULL_VOID(pipeline);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    CHECK_NULL_VOID(ngPipeline);

    std::list<AccessibilityElementInfo> infos;
    std::list<AccessibilityElementInfo> treeInfos;
    SpecificPropertyParam propertyParam {
        .propertyTarget = propertyTarget, .propertyType = static_cast<SEARCH_TYPE>(propertyType)};
    SearchElementInfoByCustomIdNG(nodeId, propertyParam.propertyTarget, infos, treeInfos, ngPipeline);
    DumpLog::GetInstance().Print(std::string("customId search start"));
    DumpLog::GetInstance().Print("info size: " + std::to_string(infos.size()));
    for (const auto& info : infos) {
        DumpLog::GetInstance().Print("info Element ID: " + std::to_string(info.GetAccessibilityId()));
    }
    DumpLog::GetInstance().Print("treeInfos size: " + std::to_string(treeInfos.size()));
    for (const auto& treeInfo : treeInfos) {
        DumpLog::GetInstance().Print("treeInfo Element ID " + std::to_string(treeInfo.GetAccessibilityId()));
    }
    DumpLog::GetInstance().Print(std::string("Result: customId search done"));
}

void JsAccessibilityManager::DumpSetCheckListTest(const std::vector<std::string>& params)
{
    std::vector<uint32_t> vec;
    if (!DumpProcessSetCheckListParameters(params, vec)) {
        return;
    }
    UpdateEventWhiteList(vec);
    DumpLog::GetInstance().Print("white list size: " + std::to_string(eventWhiteList_.size()));
    DumpLog::GetInstance().Print(std::string("Result: Set White List Done"));
}

void JsAccessibilityManager::DumpGetCheckListTest(const std::vector<std::string>& params)
{
    if (!DumpProcessGetCheckListParameters(params)) {
        return;
    }
    DumpLog::GetInstance().Print("white list size: " + std::to_string(eventWhiteList_.size()));
    for (const auto& event : eventWhiteList_) {
        DumpLog::GetInstance().Print("white list No: "+ std::to_string(event));
    }
    DumpLog::GetInstance().Print(std::string("Result: Get White List Done"));
}

void JsAccessibilityManager::DumpTreeNG(bool useWindowId, uint32_t windowId, int64_t rootId, bool isDumpSimplify)
{
    if (!useWindowId && rootId == -1) {
        // used to adapt old function
        DumpTree(0, 0);
        return;
    }

    auto pipeline = GetPipelineByWindowId(windowId);
    if (pipeline == nullptr) {
        DumpLog::GetInstance().Print("Error: pipeline is not found!");
        return;
    }
    auto rootNode = pipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    CommonProperty commonProperty;
    auto mainPipeline = context_.Upgrade();
    CHECK_NULL_VOID(mainPipeline);
    GenerateCommonProperty(pipeline, commonProperty, mainPipeline, rootNode);
    auto nodeId = rootId == -1 ? rootNode->GetAccessibilityId() : rootId;
    DumpTreeNG(rootNode, 0, nodeId, commonProperty, isDumpSimplify);
}

void JsAccessibilityManager::DumpHoverTestNG(uint32_t windowId, int64_t rootId, int32_t x, int32_t y, bool verbose)
{
    auto pipeline = GetPipelineByWindowId(windowId);
    CHECK_NULL_VOID(pipeline);
    auto accessibilityManagerNG = pipeline->GetAccessibilityManagerNG();
    CHECK_NULL_VOID(accessibilityManagerNG);
    auto pipelineRoot = pipeline->GetRootElement();
    RefPtr<NG::FrameNode> root = nullptr;
    if (rootId == -1) {
        root = pipelineRoot;
    } else {
        root = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(pipelineRoot, rootId);
    }
    CHECK_NULL_VOID(root);

    DumpLog::GetInstance().Print("Window ID: " + std::to_string(windowId));
    DumpLog::GetInstance().Print("Root ID: " + std::to_string(root->GetAccessibilityId()));
    NG::PointF hoverPoint(x, y);
    DumpLog::GetInstance().Print("Hover Point: " + hoverPoint.ToString());

    std::string summary;
    std::string detail;
    accessibilityManagerNG->HoverTestDebug(root, hoverPoint, summary, detail);
    DumpLog::GetInstance().Print(summary);
    if (verbose) {
        DumpLog::GetInstance().Print(detail);
    }
}

bool JsAccessibilityManager::DumpProcessEventParameters(
    AccessibilityEvent& event, const std::vector<std::string>& params)
{
    constexpr int32_t NUM_PARAMETERS_DIMENSION = 1;
    if (params.size() < 1) {
        return false;
    }
    for (auto arg = params.begin() + 1; arg != params.end(); ++arg) {
        if (*arg == "--stackNodeId") {
            if (std::distance(arg, params.end()) <= NUM_PARAMETERS_DIMENSION) {
                DumpLog::GetInstance().Print(std::string("Error: parameters need with data"));
                return false;
            }
            ++arg;
            event.stackNodeId = StringUtils::StringToLongInt(*arg);
        } else if (*arg == "--beforeText") {
            if (std::distance(arg, params.end()) <= NUM_PARAMETERS_DIMENSION) {
                DumpLog::GetInstance().Print(std::string("Error: parameters need with data"));
                return false;
            }
            ++arg;
            event.beforeText = *arg;
        } else if (*arg == "--latestContent") {
            if (std::distance(arg, params.end()) <= NUM_PARAMETERS_DIMENSION) {
                DumpLog::GetInstance().Print(std::string("Error: parameters need with data"));
                return false;
            }
            ++arg;
            event.latestContent = *arg;
        } else if (*arg == "--textAnnounced") {
            if (std::distance(arg, params.end()) <= NUM_PARAMETERS_DIMENSION) {
                DumpLog::GetInstance().Print(std::string("Error: parameters need with data"));
                return false;
            }
            ++arg;
            event.textAnnouncedForAccessibility = *arg;
        }
    }
    return true;
}

void JsAccessibilityManager::DumpSendEventTest(int64_t nodeId, int32_t eventId, const std::vector<std::string>& params)
{
    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    RefPtr<NG::PipelineContext> ngPipeline;

    RefPtr<NG::FrameNode> frameNode;
    ngPipeline = FindPipelineByElementId(nodeId, frameNode);
    CHECK_NULL_VOID(ngPipeline);
    CHECK_NULL_VOID(frameNode);

    AccessibilityEvent accessibilityEvent;
    accessibilityEvent.nodeId = nodeId;
    accessibilityEvent.windowChangeTypes = WindowUpdateType::WINDOW_UPDATE_ACTIVE;
    accessibilityEvent.type = static_cast<AccessibilityEventType>(eventId);
    if (DumpProcessEventParameters(accessibilityEvent, params)) {
        SendEventToAccessibilityWithNode(accessibilityEvent, frameNode, ngPipeline);
    }
}

bool JsAccessibilityManager::CheckDumpInfoParams(const std::vector<std::string> &params)
{
    if (params.size() < 1) {
        return false;
    }
    const std::string firstParam = params[0];
    if (firstParam.compare("-inspector") != 0 && firstParam.compare("-accessibility") != 0 &&
        firstParam.compare("-simplify") != 0) {
        return false;
    }
    return true;
}

bool JsAccessibilityManager::GetDumpInfoArgument(const std::vector<std::string>& params, DumpInfoArgument& argument)
{
    if (params.size() < 1) {
        return false;
    }
    argument.isDumpSimplify = params[0].compare("-simplify") == 0;
    bool isArgument = DumpInfoParams(params, argument);
    return isArgument;
}

bool JsAccessibilityManager::DumpInfoParams(const std::vector<std::string>& params, DumpInfoArgument& argument)
{
    static const std::unordered_map<std::string, DumpMode> BREAK_MODE_MAP = {
        { "--inject-action", DumpMode::INJECT_ACTION_TEST },
        { "--embed-search", DumpMode::EMBED_SEARCH_TEST },
        { "--embed-hover", DumpMode::EMBED_HOVER_TEST },
        { "--specific-search", DumpMode::SPECIFIC_SEARCH_TEST },
        { "--set-whitelist", DumpMode::SET_CHECKLIST_TEST },
        { "--get-whitelist", DumpMode::GET_CHECKLIST_TEST },
        { "--execute-action", DumpMode::EXECUTE_ACTION_TEST },
        { "--set-component-type", DumpMode::SET_COMPONENT_TYPE_TEST },
        { "--clear-component-type", DumpMode::CLEAR_COMPONENT_TYPE_TEST },
        { "-json", DumpMode::TREE },
    };

    for (auto arg = params.begin() + 1; arg != params.end(); ++arg) {
        if (*arg == "-w") {
            argument.useWindowId = true;
        } else if (*arg == "--root") {
            ++arg;
            if (arg == params.end()) {
                DumpLog::GetInstance().Print(std::string("Error: --root is used to set the root node, ") +
                    "e.g. '--root ${AccessibilityId}'!");
                return false;
            }
            argument.rootId = StringUtils::StringToLongInt(*arg);
        } else if (*arg == "--hover-test") {
            if (!CheckAndGetHoverTestArgument(arg, params, argument)) {
                return false;
            }
        } else if (*arg == "--event-test") {
            return CheckAndGetEventTestArgument(arg, params, argument);
        } else if (*arg == "--custom-action-test") {
            argument.mode = DumpMode::CUSTOM_ACTION_TEST;
            break;
        } else if (*arg == "-v") {
            argument.verbose = true;
#ifdef WEB_SUPPORTED
        } else if (*arg == WEB_ACC_ID_PARAM || *arg == WEB_ACC_FUN_PARAM) {
            return DumpWebInfoParams(params, argument);
#endif
        } else if (HandleNodeModeParam(*arg, argument)) {
            break;
        } else if (*arg == "--set-custom-property") {
            argument.mode = DumpMode::SET_CUSTOM_PROPERTY;
            if (!ParseSetCustomPropertyParams(arg, params, argument)) {
                return false;
            }
            break;
        } else if (*arg == "--get-custom-property") {
            argument.mode = DumpMode::GET_CUSTOM_PROPERTY;
            if (!ParseGetCustomPropertyParams(arg, params, argument)) {
                return false;
            }
            break;
        } else if (*arg == "--add-virtual-node") {
            argument.mode = DumpMode::ADD_VIRTUAL_NODE;
            if (!ParseAddVirtualNodeParams(arg, params, argument)) {
                return false;
            }
            break;
        } else if (*arg == "--remove-virtual-node") {
            argument.mode = DumpMode::REMOVE_VIRTUAL_NODE;
            if (!ParseRemoveVirtualNodeParams(arg, params, argument)) {
                return false;
            }
            break;
        } else if (*arg == "--get-virtual-node") {
            argument.mode = DumpMode::GET_VIRTUAL_NODE;
            if (!ParseGetVirtualNodeParams(arg, params, argument)) {
                return false;
            }
            break;
        } else if (*arg == "--perform-virtual-node-action") {
            argument.mode = DumpMode::PERFORM_VIRTUAL_NODE_ACTION_TEST;
            if (!ParsePerformVirtualNodeActionParams(arg, params, argument)) {
                return false;
            }
            break;
        } else if (*arg == "-json") {
            argument.mode = DumpMode::TREE;
        } else {
            auto it = BREAK_MODE_MAP.find(*arg);
            if (it != BREAK_MODE_MAP.end()) {
                argument.mode = it->second;
                break;
            }
        }
    }
    return true;
}

bool JsAccessibilityManager::HandleNodeModeParam(const std::string& param, DumpInfoArgument& argument)
{
    if (argument.mode == DumpMode::NODE) {
        argument.mode = DumpMode::HANDLE_EVENT;
        return true;
    } else {
        argument.mode = DumpMode::NODE;
        argument.nodeId = StringUtils::StringToLongInt(param);
        return false;
    }
}

void JsAccessibilityManager::OnDumpInfoNG(const std::vector<std::string>& params, uint32_t windowId, bool hasJson)
{
    if (!CheckDumpInfoParams(params)) {
        DumpLog::GetInstance().Print("Error: invalid arguments!");
        return;
    }
    DumpInfoArgument argument;
    if (!GetDumpInfoArgument(params, argument)) {
        return;
    }
    std::vector<std::string> info;
    bool isChildElement = CheckIsChildElement(argument.nodeId, params, info, argument.mode, argument.rootId);
    if (isChildElement) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "dump child element: %{public}" PRId64, argument.nodeId);
        return;
    }
    ChooseDumpEvent(params, argument, windowId, hasJson);
}

void JsAccessibilityManager::ChooseDumpEvent(const std::vector<std::string>& params,
    DumpInfoArgument& argument, uint32_t windowId, bool hasJson)
{
    switch (argument.mode) {
        case DumpMode::TREE:
            isUseJson_ = hasJson;
            DumpTreeNG(argument.useWindowId, windowId, argument.rootId, argument.isDumpSimplify);
            break;
        case DumpMode::NODE:
            DumpPropertyNG(argument.nodeId);
            break;
        case DumpMode::HANDLE_EVENT:
            DumpHandleEvent(params);
            break;
        case DumpMode::HOVER_TEST:
            DumpHoverTestNG(windowId, argument.rootId, argument.pointX, argument.pointY, argument.verbose);
            break;
        case DumpMode::EVENT_TEST:
            DumpSendEventTest(argument.nodeId, argument.eventId, params);
            break;
        case DumpMode::INJECT_ACTION_TEST:
            DumpInjectActionTest(params);
            break;
        case DumpMode::EMBED_SEARCH_TEST:
            DumpEmbedSearchTest(params);
            break;
        case DumpMode::EMBED_HOVER_TEST:
            DumpEmbedHoverTestNG(params, windowId);
            break;
        case DumpMode::SPECIFIC_SEARCH_TEST:
            DumpSpecificPropertySearchTest(params, windowId);
            break;
        case DumpMode::SET_CHECKLIST_TEST:
            DumpSetCheckListTest(params);
            break;
        case DumpMode::GET_CHECKLIST_TEST:
            DumpGetCheckListTest(params);
            break;
        case DumpMode::EXECUTE_ACTION_TEST:
            DumpExecuteActionTest(params);
            break;
        case DumpMode::SET_CUSTOM_PROPERTY:
            DumpSetCustomPropertyTest(argument);
            break;
        case DumpMode::GET_CUSTOM_PROPERTY:
            DumpGetCustomPropertyTest(argument.nodeId);
            break;
        case DumpMode::ADD_VIRTUAL_NODE:
            DumpAddVirtualNodeTest(argument);
            break;
        case DumpMode::REMOVE_VIRTUAL_NODE:
            DumpRemoveVirtualNodeTest(argument);
            break;
        case DumpMode::GET_VIRTUAL_NODE:
            DumpGetVirtualNodeTest(argument);
            break;
        case DumpMode::PERFORM_VIRTUAL_NODE_ACTION_TEST:
            DumpPerformVirtualNodeActionTest(argument);
            break;
#ifdef WEB_SUPPORTED
        case DumpMode::WEB_ACC_DUMP:
            ChooseWebDumpEvent(argument, windowId);
            break;
#endif
        case DumpMode::CUSTOM_ACTION_TEST:
            DumpCustomActionTest(params);
            break;
        case DumpMode::SET_COMPONENT_TYPE_TEST:
            DumpSetComponentTypeTest(params);
            break;
        case DumpMode::CLEAR_COMPONENT_TYPE_TEST:
            DumpClearComponentTypeTest(params);
            break;
        default:
            DumpLog::GetInstance().Print("Error: invalid arguments!");
            break;
    }
}

bool JsAccessibilityManager::CheckDumpHandleEventParams(const std::vector<std::string>& params)
{
    if (params.size() > EVENT_DUMP_PARAM_LENGTH_UPPER + 1) {
        DumpLog::GetInstance().Print("Error: params length is illegal!");
        return false;
    }
    if (params[EVENT_DUMP_ORDER_INDEX] != DUMP_ORDER && params[EVENT_DUMP_ORDER_INDEX] != DUMP_INSPECTOR) {
        DumpLog::GetInstance().Print("Error: Unrecognized dump command for accessibility!");
        return false;
    }
    return true;
}

bool JsAccessibilityManager::CheckGetActionIdAndOp(
    const std::vector<std::string>& params,
    int64_t& actionAccessibilityId,
    ActionType& actionOp)
{
    if (EVENT_DUMP_ID_INDEX > params.size()) {
        return false;
    }
    if (EVENT_DUMP_ACTION_INDEX != std::clamp(EVENT_DUMP_ACTION_INDEX, EVENT_DUMP_ID_INDEX, params.size())) {
        return false;
    }
    actionAccessibilityId = StringUtils::StringToLongInt(params[EVENT_DUMP_ID_INDEX]);
    auto action = static_cast<AceAction>(StringUtils::StringToInt(params[EVENT_DUMP_ACTION_INDEX]));
    actionOp = ConvertAceAction(action);
    if ((actionOp != ActionType::ACCESSIBILITY_ACTION_SET_SELECTION) &&
        (params.size() > EVENT_DUMP_PARAM_LENGTH_UPPER + 1)) {
        return false;
    }
    if (actionOp == ActionType::ACCESSIBILITY_ACTION_INVALID) {
        return false;
    }
    return true;
}

void JsAccessibilityManager::DumpHandleEvent(const std::vector<std::string>& params)
{
    if (!CheckDumpHandleEventParams(params)) {
        return;
    }
    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    int64_t nodeId = StringUtils::StringToLongInt(params[EVENT_DUMP_ID_INDEX]);

    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(nodeId, splitElementId, splitTreeId);
    nodeId = splitElementId;

    auto action = static_cast<AceAction>(StringUtils::StringToInt(params[EVENT_DUMP_ACTION_INDEX]));
    auto op = ConvertAceAction(action);
    if ((op != ActionType::ACCESSIBILITY_ACTION_SET_SELECTION) && (params.size() > EVENT_DUMP_PARAM_LENGTH_UPPER + 1)) {
        return DumpLog::GetInstance().Print("Error: params is illegal!");
    }
    std::map<std::string, std::string> paramsMap;
    ProcessParameters(op, params, paramsMap);
    if (AceType::InstanceOf<NG::PipelineContext>(pipeline)) {
        RefPtr<NG::FrameNode> node;
#ifdef WINDOW_SCENE_SUPPORTED
        auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
        auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
        CHECK_NULL_VOID(uiExtensionManager);
        if (uiExtensionManager->IsWrapExtensionAbilityId(nodeId)) {
            ExecuteActionNG(nodeId, paramsMap, op, ngPipeline, NG::UI_EXTENSION_OFFSET_MAX);
            return;
        }
#endif
        pipeline = FindPipelineByElementId(nodeId, node);
        CHECK_NULL_VOID(pipeline);
        pipeline->GetTaskExecutor()->PostTask(
            [weak = WeakClaim(this), op, nodeId, paramsMap]() {
                auto jsAccessibilityManager = weak.Upgrade();
                CHECK_NULL_VOID(jsAccessibilityManager);
                RefPtr<NG::FrameNode> node;
                auto pipeline = jsAccessibilityManager->FindPipelineByElementId(nodeId, node);
                CHECK_NULL_VOID(pipeline);
                jsAccessibilityManager->ExecuteActionNG(nodeId, paramsMap, op, pipeline, NG::UI_EXTENSION_OFFSET_MAX);
            },
            TaskExecutor::TaskType::UI, "ArkUIAccessibilityExecuteAction");
        return;
    }
    auto node = GetAccessibilityNodeFromPage(nodeId);
    CHECK_NULL_VOID(node);
    pipeline->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), op, node, paramsMap]() {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            auto pipeline = jsAccessibilityManager->context_.Upgrade();
            CHECK_NULL_VOID(pipeline);
            jsAccessibilityManager->AccessibilityActionEvent(
                op, paramsMap, node, AceType::DynamicCast<PipelineContext>(pipeline));
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilityActionEvent");
}

void JsAccessibilityManager::DumpProperty(const RefPtr<AccessibilityNode>& node)
{
    const auto& supportAceActions = node->GetSupportAction();
    const auto& charValue = node->GetChartValue();

    DumpLog::GetInstance().AddDesc("ID: ", node->GetNodeId());
    DumpLog::GetInstance().AddDesc("parent ID: ", node->GetParentId());
    DumpLog::GetInstance().AddDesc("child IDs: ", GetNodeChildIds(node));
    DumpLog::GetInstance().AddDesc("component type: ", node->GetTag());
    DumpLog::GetInstance().AddDesc("input type: ", node->GetInputType());
    DumpLog::GetInstance().AddDesc("text: ", node->GetText());
    DumpLog::GetInstance().AddDesc("width: ", node->GetWidth());
    DumpLog::GetInstance().AddDesc("height: ", node->GetHeight());
    DumpLog::GetInstance().AddDesc("left: ", node->GetLeft() + GetCardOffset().GetX());
    DumpLog::GetInstance().AddDesc("top: ", node->GetTop() + GetCardOffset().GetY());
    DumpLog::GetInstance().AddDesc("enabled: ", BoolToString(node->GetEnabledState()));
    DumpLog::GetInstance().AddDesc("checked: ", BoolToString(node->GetCheckedState()));
    DumpLog::GetInstance().AddDesc("selected: ", BoolToString(node->GetSelectedState()));
    DumpLog::GetInstance().AddDesc("focusable: ", BoolToString(node->GetFocusableState()));
    DumpLog::GetInstance().AddDesc("focused: ", BoolToString(node->GetFocusedState()));
    DumpLog::GetInstance().AddDesc("checkable: ", BoolToString(node->GetCheckableState()));
    DumpLog::GetInstance().AddDesc("clickable: ", BoolToString(node->GetClickableState()));
    DumpLog::GetInstance().AddDesc("long clickable: ", BoolToString(node->GetLongClickableState()));
    DumpLog::GetInstance().AddDesc("scrollable: ", BoolToString(node->GetScrollableState()));
    DumpLog::GetInstance().AddDesc("editable: ", BoolToString(node->GetEditable()));
    DumpLog::GetInstance().AddDesc("hint text: ", node->GetHintText());
    DumpLog::GetInstance().AddDesc("error text: ", node->GetErrorText());
    DumpLog::GetInstance().AddDesc("js component id: ", node->GetJsComponentId());
    DumpLog::GetInstance().AddDesc("accessibility label: ", node->GetAccessibilityLabel());
    DumpLog::GetInstance().AddDesc("accessibility hint: ", node->GetAccessibilityHint());
    DumpLog::GetInstance().AddDesc("max text length: ", node->GetMaxTextLength());
    DumpLog::GetInstance().AddDesc("text selection start: ", node->GetTextSelectionStart());
    DumpLog::GetInstance().AddDesc("text selection end: ", node->GetTextSelectionEnd());
    DumpLog::GetInstance().AddDesc("is multi line: ", BoolToString(node->GetIsMultiLine()));
    DumpLog::GetInstance().AddDesc("is password", BoolToString(node->GetIsPassword()));
    DumpLog::GetInstance().AddDesc("text input type: ", ConvertInputTypeToString(node->GetTextInputType()));
    DumpLog::GetInstance().AddDesc("min value: ", node->GetAccessibilityValue().min);
    DumpLog::GetInstance().AddDesc("max value: ", node->GetAccessibilityValue().max);
    DumpLog::GetInstance().AddDesc("current value: ", node->GetAccessibilityValue().current);
    DumpLog::GetInstance().AddDesc("collection info rows: ", node->GetCollectionInfo().rows);
    DumpLog::GetInstance().AddDesc("collection info columns: ", node->GetCollectionInfo().columns);
    DumpLog::GetInstance().AddDesc("collection item info, row: ", node->GetCollectionItemInfo().row);
    DumpLog::GetInstance().AddDesc("collection item info, column: ", node->GetCollectionItemInfo().column);
    DumpLog::GetInstance().AddDesc("chart has value: ", BoolToString(charValue && !charValue->empty()));
    DumpLog::GetInstance().AddDesc("accessibilityGroup: ", BoolToString(node->GetAccessible()));
    DumpLog::GetInstance().AddDesc("accessibilityImportance: ", node->GetImportantForAccessibility());
    DumpLog::GetInstance().AddDesc("support action: ", GetSupportAction(supportAceActions));
    DumpLog::GetInstance().Print(0, node->GetTag(), node->GetChildList().size());
}

void JsAccessibilityManager::DumpPropertyInfo(
    const RefPtr<NG::FrameNode>& frameNode, AccessibilityElementInfo& nodeInfo)
{
    DumpCommonPropertyNG(nodeInfo, treeId_);
    DumpAccessibilityPropertyNG(nodeInfo);
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    if (accessibilityProperty) {
        DumpLog::GetInstance().AddDesc("offset: ", accessibilityProperty->GetScrollOffSet());
        DumpLog::GetInstance().AddDesc("childTreeId: ", accessibilityProperty->GetChildTreeId());
        DumpLog::GetInstance().AddDesc("childWindowId: ", accessibilityProperty->GetChildWindowId());
        if (accessibilityProperty->HasAccessibilityNextFocusParams()) {
            DumpLog::GetInstance().AddDesc("descendantMode: ",
                BoolToString(accessibilityProperty->GetAccessibilityNextFocusParams().descendantMode));
        }

        auto customProperty = accessibilityProperty->GetCustomAccessibilityProperty();
        if (customProperty) {
            DumpLog::GetInstance().AddDesc("customAccessibilityText: ", customProperty->GetAccessibilityText());
            DumpLog::GetInstance().AddDesc("customAccessibilityLevel: ", customProperty->GetAccessibilityLevel());
            DumpLog::GetInstance().AddDesc("customAccessibilityGroup: ",
                std::to_string(customProperty->GetAccessibilityGroup()));
            DumpLog::GetInstance().AddDesc("customRole: ", customProperty->GetRole());
            DumpLog::GetInstance().AddDesc("customCheckable: ", std::to_string(customProperty->GetCheckable()));
            DumpLog::GetInstance().AddDesc("customChecked: ", std::to_string(customProperty->GetChecked()));
            DumpLog::GetInstance().AddDesc("customIsEnable: ", std::to_string(customProperty->GetEnabled()));
            DumpLog::GetInstance().AddDesc("customIsSelected: ", std::to_string(customProperty->GetSelected()));
        }

        do {
            if (!accessibilityProperty->HasVirtualNodeTreeRoot()) {
                break;
            }
            auto virtualNodeTreeRoot = accessibilityProperty->GetVirtualNodeTreeRoot();
            if (!virtualNodeTreeRoot) {
                break;
            }
            auto virtualNodeRoot = AceType::DynamicCast<NG::VirtualAccessibilityNode>(virtualNodeTreeRoot);
            if (!virtualNodeRoot) {
                break;
            }
            DumpLog::GetInstance().AddDesc("hasVirtualNodeTreeRoot: true");
            DumpLog::GetInstance().Print(0, nodeInfo.GetComponentType(), virtualNodeRoot->GetChildCount());
            DumpVirtualAccessibilityNodeTree(frameNode, virtualNodeRoot, 1);
        } while (false);
    }

    DumpLog::GetInstance().AddDesc("surfaceId: ", GetSurfaceIdByEmbedNode(frameNode));
    DumpLog::GetInstance().Print(0, nodeInfo.GetComponentType(), nodeInfo.GetChildCount());
}

void JsAccessibilityManager::DumpPropertyNG(int64_t nodeID)
{
    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    RefPtr<NG::PipelineContext> ngPipeline;

#ifdef WINDOW_SCENE_SUPPORTED
    ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    std::list<AccessibilityElementInfo> extensionElementInfos;
    if (uiExtensionManager->IsWrapExtensionAbilityId(nodeID)) {
        SearchElementInfoByAccessibilityIdNG(
            nodeID, PREFETCH_RECURSIVE_CHILDREN, extensionElementInfos, ngPipeline, NG::UI_EXTENSION_OFFSET_MAX);
        for (auto& extensionElementInfo : extensionElementInfos) {
            if (nodeID == extensionElementInfo.GetAccessibilityId()) {
                DumpCommonPropertyNG(extensionElementInfo, treeId_);
                DumpAccessibilityPropertyNG(extensionElementInfo);
                DumpLog::GetInstance().Print(
                    0, extensionElementInfo.GetComponentType(), extensionElementInfo.GetChildCount());
                return;
            }
        }
    }
#endif

    RefPtr<NG::FrameNode> frameNode;
    ngPipeline = FindPipelineByElementId(nodeID, frameNode);
    CHECK_NULL_VOID(ngPipeline);
    CHECK_NULL_VOID(frameNode);

    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, pipeline, frameNode);
    commonProperty.checkEmbedNode = false;
    AccessibilityElementInfo nodeInfo;
    UpdateAccessibilityElementInfo(frameNode, commonProperty, nodeInfo, ngPipeline);
    SetRootAccessibilityVisible(frameNode, nodeInfo);
    SetRootAccessibilityNextFocusId(frameNode, ngPipeline->GetRootElement(), nodeInfo);
    SetRootAccessibilityPreFocusId(frameNode, ngPipeline->GetRootElement(), nodeInfo,
        nextFocusRelationController_.GetTargetMap(pipeline->GetInstanceId()));
    if (IsExtensionComponent(frameNode) && !IsUIExtensionShowPlaceholder(frameNode)) {
        SearchParameter param {-1, "", PREFETCH_RECURSIVE_CHILDREN, NG::UI_EXTENSION_OFFSET_MAX};
        std::list<AccessibilityElementInfo> extensionElementInfos;
        SearchExtensionElementInfoNG(param, frameNode, extensionElementInfos, nodeInfo);
    }
    DumpPropertyInfo(frameNode, nodeInfo);
}

void JsAccessibilityManager::DumpProperty(const std::vector<std::string>& params)
{
    CHECK_NULL_VOID(DumpLog::GetInstance().GetDumpFile());
    if (params.empty()) {
        DumpLog::GetInstance().Print("Error: params cannot be empty!");
        return;
    }
    if (params.size() != PROPERTY_DUMP_PARAM_LENGTH) {
        DumpLog::GetInstance().Print("Error: params length is illegal!");
        return;
    }
    if (params[0] != DUMP_ORDER && params[0] != DUMP_INSPECTOR) {
        DumpLog::GetInstance().Print("Error: not accessibility dump order!");
        return;
    }

    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);

    if (!AceType::InstanceOf<NG::PipelineContext>(pipeline)) {
        auto node = GetAccessibilityNodeFromPage(StringUtils::StringToLongInt(params[1]));
        if (!node) {
            DumpLog::GetInstance().Print("Error: can't find node with ID " + params[1]);
            return;
        }
        DumpProperty(node);
    } else {
        DumpPropertyNG(StringUtils::StringToLongInt(params[1]));
    }
}

#ifdef WEB_SUPPORTED
int64_t JsAccessibilityManager::ConvertToSplitElementId(int64_t elementId)
{
    if (elementId <= 0) {
        return elementId;
    }
    if ((static_cast<uint64_t>(elementId) & WEB_MAX_ELEMENT_ID) != static_cast<uint64_t>(elementId)) {
        int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
        int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
        AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);
        elementId = splitElementId;
    }
    return elementId;
}

bool JsAccessibilityManager::GetWebDumpQueryType(const std::string& webAccFun, int32_t& mode, int32_t& direction)
{
    auto funIt = webAccFunMap.find(webAccFun);
    WebAccFun fun = (funIt != webAccFunMap.end()) ? funIt->second : WEB_ACC_INVALID;
    switch (fun) {
        case WEB_TREE:
            mode = WEB_TREE_MODE;
            return true;
        case WEB_NODE:
            mode = 0;
            return true;
        case WEB_PRE:
            direction = FocusMoveDirection::BACKWARD;
            return true;
        case WEB_NEXT:
            direction = FocusMoveDirection::FORWARD;
            return true;
        default:
            return false;
    }
}

bool JsAccessibilityManager::DumpElementInfosIfNeed(const DumpInfoArgument& argument,
    std::list<AccessibilityElementInfo>& infos, const RefPtr<NG::WebPattern>& webPattern, uint32_t windowId)
{
    int32_t direction = -1;
    int32_t mode = -1;
    if (!GetWebDumpQueryType(argument.webAccFun, mode, direction)) {
        return true;
    }

    int64_t elementId = ConvertToSplitElementId(argument.webAccId);
    AccessibilityElementInfo nodeInfo;
    auto pipeline = GetPipelineByWindowId(windowId);
    CHECK_NULL_RETURN(pipeline, false);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    CHECK_NULL_RETURN(ngPipeline, false);

    if (mode != -1) {
        SearchWebElementInfoByAccessibilityIdNG(elementId, mode, infos, ngPipeline, webPattern);
    } else if (direction != -1) {
        if (argument.focusMoveRule == -1) {
            WebFocusMoveSearchNG(elementId, direction, nodeInfo, pipeline, webPattern);
            auto result = BuildWebFocusMoveResult(FocusMoveResultType::SEARCH_SUCCESS);
            WebFocusMoveSearchByComponent(nodeInfo, webPattern, direction, pipeline, result);
        } else {
            auto result = WebFocusMoveSearchWithConditionNG(
                elementId, direction, argument.focusMoveRule, nodeInfo, pipeline, webPattern);
            WebFocusMoveSearchByComponent(nodeInfo, webPattern, direction, pipeline, result, argument.focusMoveRule);
        }
        infos.push_back(nodeInfo);
    }

    if (infos.empty() || infos.size() > MAX_DUMP_INFO_SIZE) {
        TAG_LOGE(AceLogTag::ACE_WEB, "JsAccessibilityManager::DumpElementInfosIfNeed fail infos.size: %{public}zu",
            infos.size());
        return false;
    }
    return true;
}

void JsAccessibilityManager::ExecuteWebAccStateDump(WebAccFun fun, const RefPtr<NG::WebPattern>& webPattern)
{
    CHECK_NULL_VOID(webPattern);
    switch (fun) {
        case WEB_GET_ACC:
            DumpLog::GetInstance().AddDesc(
                "===> webPattern AccessibilityState: ", BoolToString(webPattern->GetAccessibilityState()));
            DumpLog::GetInstance().Print(0, "webPattern.AccessibilityState", 0);
            break;
        case WEB_ON_ACC:
            webPattern->SetAccessibilityState(true);
            AceApplicationInfo::GetInstance().SetAccessibilityEnabled(true);
            break;
        case WEB_OFF_ACC:
            webPattern->SetAccessibilityState(false);
            AceApplicationInfo::GetInstance().SetAccessibilityEnabled(false);
            break;
        default:
            break;
    }
}

void JsAccessibilityManager::ExecuteWebInfoDump(
    WebAccFun fun, std::list<AccessibilityElementInfo>& infos, const RefPtr<NG::WebPattern>& webPattern)
{
    CHECK_NULL_VOID(webPattern);
    switch (fun) {
        case WEB_TREE:
            if (infos.empty()) {
                TAG_LOGE(AceLogTag::ACE_WEB, "ExecuteWebInfoDump WEB_TREE infos is empty");
                return;
            }
            DumpAccessibilityElementInfosTreeNG(infos, 1, infos.front().GetAccessibilityId(), false);
            DumpLog::GetInstance().Print(0, "Total number of dumped nodes ", infos.size());
            break;
        case WEB_NODE:
        case WEB_PRE:
        case WEB_NEXT:
            if (infos.empty()) {
                TAG_LOGE(AceLogTag::ACE_WEB, "ExecuteWebInfoDump infos is empty");
                return;
            }
            DumpCommonPropertyNG(infos.front(), webPattern->GetTreeId());
            DumpAccessibilityPropertyNG(infos.front());
            DumpLog::GetInstance().Print(0, infos.front().GetComponentType(), infos.front().GetChildCount());
            break;
        default:
            break;
    }
}

void JsAccessibilityManager::ExecuteWebActionDump(
    WebAccFun fun, const DumpInfoArgument& argument, int64_t elementId, const RefPtr<NG::WebPattern>& webPattern)
{
    CHECK_NULL_VOID(webPattern);
    switch (fun) {
        case WEB_EXE_ACTION: {
            std::map<std::string, std::string> args = { { " ", " " } };
            webPattern->ExecuteAction(elementId, static_cast<AceAction>(argument.action), args);
            break;
        }
        case WEB_SEND_EVENT: {
            webPattern->OnAccessibilityEvent(elementId, static_cast<AccessibilityEventType>(argument.eventId), " ");
            break;
        }
        case WEB_HOVER: {
            auto host = webPattern->GetHost();
            CHECK_NULL_VOID(host);
            auto offset = host->GetOffsetRelativeToWindow();
            const NG::PointF point(argument.pointX - offset.GetX(), argument.pointY - offset.GetY());
            webPattern->OnAccessibilityHoverEvent(
                point, SourceType::NONE, NG::AccessibilityHoverEventType::MOVE, TimeStamp());
            break;
        }
        default:
            break;
    }
}

void JsAccessibilityManager::ExecuteWebDump(const DumpInfoArgument& argument,
    std::list<AccessibilityElementInfo>& infos, const RefPtr<NG::WebPattern>& webPattern)
{
    auto funIt = webAccFunMap.find(argument.webAccFun);
    WebAccFun fun = (funIt != webAccFunMap.end()) ? funIt->second : WEB_ACC_INVALID;
    int64_t elementId = ConvertToSplitElementId(argument.webAccId);
    CHECK_NULL_VOID(webPattern);

    switch (fun) {
        case WEB_GET_ACC:
        case WEB_ON_ACC:
        case WEB_OFF_ACC:
            ExecuteWebAccStateDump(fun, webPattern);
            break;
        case WEB_TREE:
        case WEB_NODE:
        case WEB_PRE:
        case WEB_NEXT:
            ExecuteWebInfoDump(fun, infos, webPattern);
            break;
        case WEB_ACC_JSON: {
            auto lambda = [](std::shared_ptr<JsonValue>& jsonValue, int32_t id) {
                CHECK_NULL_VOID(jsonValue);
                DumpLog::GetInstance().Print(0, jsonValue->ToString());
            };
            webPattern->ToJsonValueAccessibility(std::move(lambda), webPattern->GetTreeId());
            break;
        }
        case WEB_EXE_ACTION:
        case WEB_SEND_EVENT:
        case WEB_HOVER:
            ExecuteWebActionDump(fun, argument, elementId, webPattern);
            break;
        default:
            TAG_LOGE(
                AceLogTag::ACE_WEB, "ExecuteWebDump cannot find webAccFun: %{public}s", argument.webAccFun.c_str());
            break;
    }
}

void JsAccessibilityManager::ChooseWebDumpEvent(DumpInfoArgument& argument, uint32_t windowId)
{
    TAG_LOGI(AceLogTag::ACE_WEB,
        "JsAccessibilityManager::ChooseWebDumpEvent webAccFun:%{public}s, webAccId:%{public}" PRId64
        ", nodeId(WEB):%{public}" PRId64 ", windowId:%{public}d",
        argument.webAccFun.c_str(), argument.webAccId, argument.nodeId, windowId);

    RefPtr<NG::WebPattern> webPattern = nullptr;
    int64_t frameNodeId = ConvertToSplitElementId(argument.nodeId);
    {
        std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
        auto callbackIt = childTreeCallbackMap_.find(frameNodeId);
        if (callbackIt != childTreeCallbackMap_.end()) {
            auto callback = static_cast<NG::WebAccessibilityChildTreeCallback*>(callbackIt->second.get());
            CHECK_NULL_VOID(callback);
            webPattern = callback->GetWebPattern().Upgrade();
        }
    }
    if (!webPattern) {
        TAG_LOGE(AceLogTag::ACE_WEB,
            "JsAccessibilityManager::ChooseWebDumpEvent webPattern null Web frameNodeId:%{public}" PRId64, frameNodeId);
        return;
    }

    auto pipeline = GetPipelineByWindowId(windowId);
    CHECK_NULL_VOID(pipeline);

    auto weak = WeakClaim(this);
    auto webPatternWeak = WeakPtr<NG::WebPattern>(webPattern);
    pipeline->GetTaskExecutor()->PostSyncTask(
        [weak, argument, webPatternWeak, windowId] {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            auto webPattern = webPatternWeak.Upgrade();
            CHECK_NULL_VOID(webPattern);
            std::list<AccessibilityElementInfo> infos;
            if (jsAccessibilityManager->DumpElementInfosIfNeed(argument, infos, webPattern, windowId)) {
                jsAccessibilityManager->ExecuteWebDump(argument, infos, webPattern);
            }
        },
        TaskExecutor::TaskType::UI, "ArkUIWebAccessibilityDump");
}

bool JsAccessibilityManager::DumpWebInfoParams(const std::vector<std::string>& params, DumpInfoArgument& argument)
{
    for (auto arg = params.begin() + 1; arg != params.end(); ++arg) {
        if (IsWebDumpParamKey(*arg) && (arg + 1 == params.end())) {
            return false;
        }
        if (*arg == WEB_ACC_ID_PARAM) {
            if (!ParseWebAccIdParam(arg, params.end(), argument)) {
                return false;
            }
            continue;
        }
        if (*arg != WEB_ACC_FUN_PARAM) {
            continue;
        }
        if (!ParseWebAccFunParam(arg, params.end(), argument)) {
            return false;
        }
        if (ParseWebFocusMoveDumpParam(arg, params.end(), argument)) {
            return true;
        }
        if (!ParseWebActionDumpParam(arg, params.end(), argument)) {
            return false;
        }
    }
    return true;
}
#endif

void JsAccessibilityManager::SetCardViewParams(const std::string& key, bool focus)
{
    callbackKey_ = key;
    if (!callbackKey_.empty()) {
        InitializeCallback();
    }
}

void JsAccessibilityManager::UpdateViewScale()
{
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    float scaleX = 1.0;
    float scaleY = 1.0;
    if (context->GetViewScale(scaleX, scaleY)) {
        scaleX_ = scaleX;
        scaleY_ = scaleY;
    }
}

void JsAccessibilityManager::HandleComponentPostBinding()
{
    for (auto targetIter = nodeWithTargetMap_.begin(); targetIter != nodeWithTargetMap_.end();) {
        auto nodeWithTarget = targetIter->second.Upgrade();
        if (nodeWithTarget) {
            if (nodeWithTarget->GetTag() == ACCESSIBILITY_TAG_POPUP) {
                auto idNodeIter = nodeWithIdMap_.find(targetIter->first);
                if (idNodeIter != nodeWithIdMap_.end()) {
                    auto nodeWithId = idNodeIter->second.Upgrade();
                    if (nodeWithId) {
                        nodeWithId->SetAccessibilityHint(nodeWithTarget->GetText());
                    } else {
                        nodeWithIdMap_.erase(idNodeIter);
                    }
                }
            }
            ++targetIter;
        } else {
            // clear the disabled node in the maps
            targetIter = nodeWithTargetMap_.erase(targetIter);
        }
    }

    // clear the disabled node in the maps
    for (auto idItem = nodeWithIdMap_.begin(); idItem != nodeWithIdMap_.end();) {
        if (!idItem->second.Upgrade()) {
            idItem = nodeWithIdMap_.erase(idItem);
        } else {
            ++idItem;
        }
    }
}

RefPtr<AccessibilityNodeManager> AccessibilityNodeManager::Create()
{
    return AceType::MakeRefPtr<JsAccessibilityManager>();
}

RefPtr<PipelineBase> JsAccessibilityManager::GetPipelineByWindowId(const int32_t windowId)
{
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, nullptr);
    if (AceType::InstanceOf<NG::PipelineContext>(context)) {
        CHECK_NULL_RETURN(context, nullptr);
        if (context->GetWindowId() == static_cast<uint32_t>(windowId)) {
            return context;
        }
        if (GetWindowId() == static_cast<uint32_t>(windowId)) {
            return context;
        }
        for (auto& subContext : GetSubPipelineContexts()) {
            context = subContext.Upgrade();
            CHECK_NULL_CONTINUE(context);
            if (context->GetWindowId() == static_cast<uint32_t>(windowId)) {
                return context;
            }
        }
        return nullptr;
    } else {
        return context;
    }
}

RetError JsAccessibilityManager::JsInteractionOperation::SearchElementInfoByAccessibilityId(const int64_t elementId,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback, const int32_t mode)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "ArkUI search by id: %{public}" PRId64 ", mode: %{public}d",
        elementId, mode);
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    if (!jsAccessibilityManager) {
        std::list<AccessibilityElementInfo> infos;
        callback.SetSearchElementInfoByAccessibilityIdResult(infos, requestId);
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "SetSearchElementInfoByAccessibilityIdResult, requestId: %{public}d",
            requestId);
        return RET_OK;
    }
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    if (!context) {
        std::list<AccessibilityElementInfo> infos;
        callback.SetSearchElementInfoByAccessibilityIdResult(infos, requestId);
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "SetSearchElementInfoByAccessibilityIdResult, requestId: %{public}d",
            requestId);
        return RET_OK;
    }
    auto windowId = windowId_;
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), splitElementId, requestId, &callback, mode, windowId]() {
            auto jsAccessibilityManager = weak.Upgrade();
            if (!jsAccessibilityManager) {
                std::list<AccessibilityElementInfo> infos;
                callback.SetSearchElementInfoByAccessibilityIdResult(infos, requestId);
                TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
                         "SetSearchElementInfoByAccessibilityIdResult, requestId: %{public}d", requestId);
                return;
            }
            ACE_SCOPED_TRACE("SearchElementInfoByAccessibilityId");
            jsAccessibilityManager->SearchElementInfoByAccessibilityId(
                splitElementId, requestId, callback, mode, windowId);
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilitySearchElementInfoById");
    return RET_OK;
}

void JsAccessibilityManager::JsInteractionOperation::HandleSearchElementInfoCallback(
    AccessibilityElementOperatorCallback& callback, int32_t requestId)
{
    std::list<AccessibilityElementInfo> infos;
    std::list<AccessibilityElementInfo> treeInfos;
    callback.SetSearchElementInfoBySpecificPropertyResult(infos, treeInfos, requestId);
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
        "SetSearchElementInfoBySpecificPropertyResult, requestId: %{public}d", requestId);
}

void JsAccessibilityManager::JsInteractionOperation::SearchElementInfoBySpecificProperty(const int64_t elementId,
    const SpecificPropertyParam &param, const int32_t requestId, AccessibilityElementOperatorCallback &callback)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "ArkUI search by id: %{public}" PRId64 ", target: %{public}s,"
        "type: %{public}d", elementId, param.propertyTarget.c_str(), param.propertyType);

    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    if (!jsAccessibilityManager) {
        std::list<AccessibilityElementInfo> infos;
        std::list<AccessibilityElementInfo> treeInfos;
        callback.SetSearchElementInfoBySpecificPropertyResult(infos, treeInfos, requestId);
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "SetSearchElementInfoBySpecificPropertyResult, requestId: %{public}d",
            requestId);
        return;
    }

    auto windowId = windowId_;
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    if (!context) {
        std::list<AccessibilityElementInfo> infos;
        std::list<AccessibilityElementInfo> treeInfos;
        callback.SetSearchElementInfoBySpecificPropertyResult(infos, treeInfos, requestId);
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "SetSearchElementInfoBySpecificPropertyResult, requestId: %{public}d",
            requestId);
        return;
    }
    auto executor = context->GetTaskExecutor();
    if (!executor) {
        HandleSearchElementInfoCallback(callback, requestId);
        return;
    }

    executor->PostTask(
        [weak = GetHandler(), splitElementId, param, requestId, &callback, windowId]() {
            auto jsAccessibilityManager = weak.Upgrade();
            if (!jsAccessibilityManager) {
                std::list<AccessibilityElementInfo> infos;
                std::list<AccessibilityElementInfo> treeInfos;
                callback.SetSearchElementInfoBySpecificPropertyResult(infos, treeInfos, requestId);
                TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
                    "SetSearchElementInfoBySpecificPropertyResult, requestId: %{public}d", requestId);
                return;
            }
            ACE_SCOPED_TRACE("SearchElementInfoBySpecificProperty");
            jsAccessibilityManager->SearchElementInfoBySpecificProperty(
                splitElementId, param, requestId, callback, windowId);
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilitySearchElementInfoByProperty");
    return;
}

void SetVirtualAccessibilityNodeByOuter(RefPtr<NG::VirtualAccessibilityNode>& node,
    const AccessibilityVirtualNode& outerNode)
{
    CHECK_NULL_VOID(node);
    auto outerRect = outerNode.GetRect();
    NG::RectT<int32_t> rectInt = {
        outerRect.GetLeftTopXScreenPostion(),
        outerRect.GetLeftTopYScreenPostion(),
        outerRect.GetRightBottomXScreenPostion() - outerRect.GetLeftTopXScreenPostion(),
        outerRect.GetRightBottomYScreenPostion() - outerRect.GetLeftTopYScreenPostion()
    };
    node->SetRect(rectInt);
    node->SetText(outerNode.GetText());
    node->SetAccessibilityText(outerNode.GetAccessibilityText());
    node->SetAccessibilityLevel(outerNode.GetAccessibilityLevel());
    node->SetAccessibilityGroup(outerNode.GetAccessibilityGroup());
    node->SetAccessibilityRole(outerNode.GetCustomComponentType());
    node->SetCheckable(outerNode.GetCheckable());
    node->SetChecked(outerNode.GetChecked());
    node->SetEnabled(outerNode.GetEnabled());
    node->SetSelected(outerNode.GetSelected());
    node->SetClickable(outerNode.GetClickable());
    node->SetSupprtAction(outerNode.GetSupportAction());
}

int32_t SetNodeMapByAccessibilityVirtualNode(
    const std::vector<Accessibility::AccessibilityVirtualNode>& outerNodes,
    std::unordered_map<int64_t, RefPtr<NG::VirtualAccessibilityNode>>& nodeMap,
    RefPtr<NG::VirtualAccessibilityNode>& rootNode, bool& hasErrorNode)
{
    rootNode = nullptr;
    nodeMap.clear();
    int32_t rootCount = 0;
    for (const auto& outer : outerNodes) {
        const auto& outerNodeId = outer.GetId();
        if (nodeMap.find(outerNodeId) != nodeMap.end()) {
            hasErrorNode = true;
            TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
                "ConvertOuterNodesToVirtualTree skip duplicate virtual node id: %{public}" PRId64, outerNodeId);
            continue;
        }
        auto node = AceType::MakeRefPtr<NG::VirtualAccessibilityNode>();
        CHECK_NULL_CONTINUE(node);
        node->SetNodeId(outerNodeId);
        SetVirtualAccessibilityNodeByOuter(node, outer);
        nodeMap[outerNodeId] = node;
        if (outer.GetParentId() == -1) {
            rootNode = node;
            rootCount++;
        }
    }
    return rootCount;
}

void AddAccessibilityVirtualNodesChilds(
    const std::vector<Accessibility::AccessibilityVirtualNode>& outerNodes,
    const std::unordered_map<int64_t, RefPtr<NG::VirtualAccessibilityNode>>& nodeMap,
    bool& hasErrorNode)
{
    for (const auto& outer : outerNodes) {
        const auto outerNodeId = outer.GetId();
        auto parentIt = nodeMap.find(outerNodeId);
        if (parentIt == nodeMap.end()) {
            continue;
        }
        auto& parentNode = parentIt->second;
        int32_t childIdSameNum = 0;
        for (int64_t childId : outer.GetChildNodeIds()) {
            if (childId == outerNodeId) {
                childIdSameNum++;
                continue;
            }
            auto childIt = nodeMap.find(childId);
            if (childIt == nodeMap.end()) {
                continue;
            }
            auto addResult = parentNode->AddChild(childIt->second);
            if (!addResult) {
                hasErrorNode = true;
                TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
                    "ConvertOuterNodesToVirtualTree AddChild failed, "
                    "parentId = %{public}" PRId64 ", childId = %{public}" PRId64,
                    outerNodeId, childId);
            }
        }
        if (childIdSameNum > 0) {
            hasErrorNode = true;
            TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
                "ConvertOuterNodesToVirtualTree got %{public}d, childId(s) same as %{public}" PRId64,
                childIdSameNum, outerNodeId);
        }
    }
}

RefPtr<NG::VirtualAccessibilityNode> GetCycleStartVirtualNodeByAncestorSet(
    const RefPtr<NG::VirtualAccessibilityNode>& node,
    std::unordered_set<const NG::VirtualAccessibilityNode*>& ancestors)
{
    CHECK_NULL_RETURN(node, nullptr);
    if (!ancestors.insert(AceType::RawPtr(node)).second) {
        return node;
    }
    for (const auto& child : node->GetChildren()) {
        auto cycleStartNode = GetCycleStartVirtualNodeByAncestorSet(child, ancestors);
        if (cycleStartNode) {
            return cycleStartNode;
        }
    }
    ancestors.erase(AceType::RawPtr(node));
    return nullptr;
}

RefPtr<NG::VirtualAccessibilityNode> GetCycleStartNodeInVirtualTree(
    const RefPtr<NG::VirtualAccessibilityNode>& rootNode)
{
    std::unordered_set<const NG::VirtualAccessibilityNode*> ancestors;
    return GetCycleStartVirtualNodeByAncestorSet(rootNode, ancestors);
}

RefPtr<NG::VirtualAccessibilityNode> ConvertOuterNodesToVirtualTree(
    const std::vector<Accessibility::AccessibilityVirtualNode>& outerNodes, bool& hasErrorNode)
{
    hasErrorNode = false;

    if (outerNodes.empty()) {
        return nullptr;
    }

    RefPtr<NG::VirtualAccessibilityNode> rootNode;
    std::unordered_map<int64_t, RefPtr<NG::VirtualAccessibilityNode>> nodeMap;
    auto rootCount = SetNodeMapByAccessibilityVirtualNode(outerNodes, nodeMap, rootNode, hasErrorNode);
    if (rootCount != 1) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "ConvertOuterNodesToVirtualTree got %{public}d roots", rootCount);
        return nullptr;
    }

    AddAccessibilityVirtualNodesChilds(outerNodes, nodeMap, hasErrorNode);

    // if has cycle, return null, else return root.
    auto cycleNode = GetCycleStartNodeInVirtualTree(rootNode);
    CHECK_NULL_RETURN(cycleNode, rootNode);
    hasErrorNode = true;
    TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY, "ConvertOuterNodesToVirtualTree cycle detected, start node id: %{public}d",
        cycleNode->GetNodeId());
    return nullptr;
}

void SetAddAccessibilityVirtualNodeFailResult(
    Accessibility::AccessibilityElementOperatorCallback& callback, int32_t requestId,
    Accessibility::OperateVirtualNodeResult result = Accessibility::OperateVirtualNodeResult::INTERNAL_ERROR)
{
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "SetAddAccessibilityVirtualNodeFailResult fail, result: %{public}d, requestId: %{public}d",
        static_cast<int32_t>(result), requestId);
    callback.SetAddAccessibilityVirtualNodeResult(result, requestId);
}

void JsAccessibilityManager::AddAccessibilityVirtualNode(const int64_t elementId,
    const std::vector<AccessibilityVirtualNode>& nodes, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback, const int32_t windowId)
{
    auto pipeline = GetPipelineByWindowId(windowId);
    CHECK_NULL_VOID_WITH_ACTION(pipeline,
        SetAddAccessibilityVirtualNodeFailResult(callback, requestId));
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    CHECK_NULL_VOID_WITH_ACTION(ngPipeline,
        SetAddAccessibilityVirtualNodeFailResult(callback, requestId));
    ContainerScope instance(ngPipeline->GetInstanceId());
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID_WITH_ACTION(rootNode,
        SetAddAccessibilityVirtualNodeFailResult(callback, requestId));

    int64_t nodeId = elementId;
    if (elementId == -1) {
        nodeId = rootNode->GetAccessibilityId();
    }

    auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(rootNode, nodeId);
    CHECK_NULL_VOID_WITH_ACTION(node,
        SetAddAccessibilityVirtualNodeFailResult(callback, requestId,
            Accessibility::OperateVirtualNodeResult::ACCESSIBILITY_ELEMENT_NOT_EXIST));

    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID_WITH_ACTION(accessibilityProperty,
        SetAddAccessibilityVirtualNodeFailResult(callback, requestId,
            Accessibility::OperateVirtualNodeResult::ACCESSIBILITY_PROPERTY_EMPTY));

    auto containerId = NG::VirtualNodeContainerIdManager::GetInstance().AllocateContainerId(node);
    if (containerId == 0) {
        SetAddAccessibilityVirtualNodeFailResult(callback, requestId,
            Accessibility::OperateVirtualNodeResult::ALLOCATE_ID_FAILED);
        return;
    }

    bool hasErrorNode = false;
    auto result = ConvertOuterNodesToVirtualTree(nodes, hasErrorNode);
    Accessibility::OperateVirtualNodeResult resultCode;
    if (hasErrorNode) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
            "ConvertOuterNodesToVirtualTree has error node, elementId = %{public}" PRId64 ", requestId = %{public}d",
            elementId, requestId);
        resultCode = Accessibility::OperateVirtualNodeResult::INTERNAL_ERROR;
    } else if (!result) {
        resultCode = Accessibility::OperateVirtualNodeResult::ADD_NODE_IS_EMPTY;
        SetAddAccessibilityVirtualNodeFailResult(callback, requestId, resultCode);
        return;
    } else {
        resultCode = Accessibility::OperateVirtualNodeResult::SUCCESS;
    }
    accessibilityProperty->SetVirtualNodeTreeRoot(result);
    callback.SetAddAccessibilityVirtualNodeResult(resultCode, requestId);
}

void SetRemoveAccessibilityVirtualNodeFailResult(
    Accessibility::AccessibilityElementOperatorCallback& callback, int32_t requestId,
    Accessibility::OperateVirtualNodeResult result = Accessibility::OperateVirtualNodeResult::INTERNAL_ERROR)
{
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "SetRemoveAccessibilityVirtualNodeFailResult fail, result: %{public}d, "
        "requestId: %{public}d", static_cast<int32_t>(result), requestId);
    callback.SetRemoveAccessibilityVirtualNodeResult(result, requestId);
}

void ClearAllVirtualNodeTreesRecursive(const RefPtr<NG::UINode>& node)
{
    CHECK_NULL_VOID(node);

    do {
        auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
        if (!frameNode) {
            break;
        }
        auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
        if (!accessibilityProperty || !accessibilityProperty->HasVirtualNodeTreeRoot()) {
            break;
        }
        auto renderContext = frameNode->GetRenderContext();
        if (renderContext) {
            renderContext->UpdateAccessibilityFocus(false);
        }
        accessibilityProperty->SetVirtualNodeTreeRoot(nullptr);
        NG::VirtualNodeContainerIdManager::GetInstance().ReleaseContainerId(frameNode);
    } while (false);

    for (const auto& child : node->GetChildren(true)) {
        ClearAllVirtualNodeTreesRecursive(child);
    }
}

void JsAccessibilityManager::RemoveAccessibilityVirtualNode(const int64_t elementId, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback, const int32_t windowId)
{
    auto pipeline = GetPipelineByWindowId(windowId);
    CHECK_NULL_VOID_WITH_ACTION(pipeline,
        SetRemoveAccessibilityVirtualNodeFailResult(callback, requestId));
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    CHECK_NULL_VOID_WITH_ACTION(ngPipeline,
        SetRemoveAccessibilityVirtualNodeFailResult(callback, requestId));
    ContainerScope instance(ngPipeline->GetInstanceId());
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID_WITH_ACTION(rootNode,
        SetRemoveAccessibilityVirtualNodeFailResult(callback, requestId));

    if (elementId == 0) {
        ClearAllVirtualNodeTreesRecursive(rootNode);
        callback.SetRemoveAccessibilityVirtualNodeResult(
            Accessibility::OperateVirtualNodeResult::SUCCESS, requestId);
        return;
    }

    int64_t nodeId = elementId;
    if (elementId == -1) {
        nodeId = rootNode->GetAccessibilityId();
    }

    auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(rootNode, nodeId);
    CHECK_NULL_VOID_WITH_ACTION(node,
        SetRemoveAccessibilityVirtualNodeFailResult(callback, requestId,
            Accessibility::OperateVirtualNodeResult::ACCESSIBILITY_ELEMENT_NOT_EXIST));

    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID_WITH_ACTION(accessibilityProperty,
        SetRemoveAccessibilityVirtualNodeFailResult(callback, requestId,
            Accessibility::OperateVirtualNodeResult::ACCESSIBILITY_PROPERTY_EMPTY));

    auto renderContext = node->GetRenderContext();
    CHECK_NULL_VOID_WITH_ACTION(renderContext,
        SetRemoveAccessibilityVirtualNodeFailResult(callback, requestId));
    renderContext->UpdateAccessibilityFocus(false);

    accessibilityProperty->SetVirtualNodeTreeRoot(nullptr);
    NG::VirtualNodeContainerIdManager::GetInstance().ReleaseContainerId(node);
    callback.SetRemoveAccessibilityVirtualNodeResult(
        Accessibility::OperateVirtualNodeResult::SUCCESS, requestId);
}

#ifdef WEB_SUPPORTED

RetError JsAccessibilityManager::WebInteractionOperation::SearchElementInfoByAccessibilityId(const int64_t elementId,
    const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t mode)
{
    uint32_t realMode = mode;
    if (realMode & static_cast<uint32_t>(PREFETCH_RECURSIVE_CHILDREN_REDUCED)) {
        realMode &= ~static_cast<uint32_t>(PREFETCH_RECURSIVE_CHILDREN_REDUCED);
        realMode |= static_cast<uint32_t>(PREFETCH_RECURSIVE_CHILDREN);
    }
    TAG_LOGD(AceLogTag::ACE_WEB, "ArkWeb search by id: %{public}" PRId64 ", mode: %{public}d",
        elementId, mode);
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    std::list<AccessibilityElementInfo> infos;
    if (!jsAccessibilityManager) {
        callback.SetSearchElementInfoByAccessibilityIdResult(infos, requestId);
        TAG_LOGD(AceLogTag::ACE_WEB, "SetSearchElementInfoByAccessibilityIdResult, requestId: %{public}d", requestId);
        return RET_OK;
    }
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    if (!context) {
        callback.SetSearchElementInfoByAccessibilityIdResult(infos, requestId);
        TAG_LOGD(AceLogTag::ACE_WEB, "SetSearchElementInfoByAccessibilityIdResult, requestId: %{public}d", requestId);
        return RET_OK;
    }
    auto windowId = windowId_;
    auto web = webPattern_;
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), splitElementId, requestId, &callback, realMode, windowId, web, infos]() {
            auto jsAccessibilityManager = weak.Upgrade();
            if (!jsAccessibilityManager) {
                callback.SetSearchElementInfoByAccessibilityIdResult(infos, requestId);
                TAG_LOGD(AceLogTag::ACE_WEB, "SetSearchElementInfoByAccessibilityIdResult, requestId: %{public}d",
                    requestId);
                return;
            }
            auto webPattern = web.Upgrade();
            if (!webPattern) {
                callback.SetSearchElementInfoByAccessibilityIdResult(infos, requestId);
                TAG_LOGD(AceLogTag::ACE_WEB, "SetSearchElementInfoByAccessibilityIdResult, requestId: %{public}d",
                    requestId);
                return;
            }
            ACE_SCOPED_TRACE("SearchWebElementInfoByAccessibilityId");
            jsAccessibilityManager->SearchWebElementInfoByAccessibilityId(
                splitElementId, requestId, callback, realMode, windowId, webPattern);
        },
        TaskExecutor::TaskType::UI, "ArkWebAccessibilitySearchElementInfoById");
    return RET_OK;
}

void JsAccessibilityManager::WebInteractionOperation::SearchElementInfoBySpecificProperty(
    const int64_t elementId, const SpecificPropertyParam &param, const int32_t requestId,
    AccessibilityElementOperatorCallback &callback)
{
    std::list<AccessibilityElementInfo> infos;
    std::list<AccessibilityElementInfo> treeInfos;
    callback.SetSearchElementInfoBySpecificPropertyResult(infos, treeInfos, requestId);
}

void JsAccessibilityManager::WebInteractionOperation::SearchElementInfosByText(const int64_t elementId,
    const std::string& text, const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback& callback)
{
}

void JsAccessibilityManager::WebInteractionOperation::SearchDefaultFocusByWindowId(const int32_t windowId,
    const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback &callback, const int32_t pageId)
{
}

void JsAccessibilityManager::WebInteractionOperation::FindFocusedElementInfo(const int64_t elementId,
    const int32_t focusType, const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGD(AceLogTag::ACE_WEB, "find focus: %{public}" PRId64 ", focusType: %{public}d",
        elementId, focusType);
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    if (!jsAccessibilityManager) {
        AccessibilityElementInfo info;
        callback.SetFindFocusedElementInfoResult(info, requestId);
        TAG_LOGD(AceLogTag::ACE_WEB, "SetFindFocusedElementInfoResult, requestId: %{public}d", requestId);
        return;
    }
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    if (!context) {
        AccessibilityElementInfo info;
        callback.SetFindFocusedElementInfoResult(info, requestId);
        TAG_LOGD(AceLogTag::ACE_WEB, "SetFindFocusedElementInfoResult, requestId: %{public}d", requestId);
        return;
    }
    auto windowId = windowId_;
    auto web = webPattern_;
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), splitElementId, focusType, requestId, &callback, windowId, web]() {
            auto jsAccessibilityManager = weak.Upgrade();
            if (!jsAccessibilityManager) {
                AccessibilityElementInfo info;
                callback.SetFindFocusedElementInfoResult(info, requestId);
                TAG_LOGD(AceLogTag::ACE_WEB, "SetFindFocusedElementInfoResult, requestId: %{public}d",
                    requestId);
                return;
            }
            auto webPattern = web.Upgrade();
            if (!webPattern) {
                AccessibilityElementInfo info;
                callback.SetFindFocusedElementInfoResult(info, requestId);
                TAG_LOGD(AceLogTag::ACE_WEB, "SetFindFocusedElementInfoResult, requestId: %{public}d",
                    requestId);
                return;
            }
            ACE_SCOPED_TRACE("FindWebFocusedElementInfo");
            jsAccessibilityManager->FindWebFocusedElementInfo(
                splitElementId, focusType, requestId, callback, windowId, webPattern);
        },
        TaskExecutor::TaskType::UI, "ArkWebFindFocusedElementInfo");
}

void JsAccessibilityManager::WebInteractionOperation::FocusMoveSearch(const int64_t elementId, const int32_t direction,
    const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGD(AceLogTag::ACE_WEB, "move search: %{public}" PRId64 ", direction: %{public}d",
        elementId, direction);
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    if (!jsAccessibilityManager) {
        AccessibilityElementInfo info;
        callback.SetFocusMoveSearchResult(info, requestId);
        TAG_LOGD(AceLogTag::ACE_WEB, "SetFocusMoveSearchResult, requestId: %{public}d", requestId);
        return;
    }
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    if (!context) {
        AccessibilityElementInfo info;
        callback.SetFocusMoveSearchResult(info, requestId);
        TAG_LOGD(AceLogTag::ACE_WEB, "SetFocusMoveSearchResult, requestId: %{public}d", requestId);
        return;
    }
    auto windowId = windowId_;
    auto web = webPattern_;
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), splitElementId, direction, requestId, &callback, windowId, web] {
            auto jsAccessibilityManager = weak.Upgrade();
            if (!jsAccessibilityManager) {
                AccessibilityElementInfo info;
                callback.SetFocusMoveSearchResult(info, requestId);
                TAG_LOGD(AceLogTag::ACE_WEB, "SetFocusMoveSearchResult, requestId: %{public}d",
                    requestId);
                return;
            }
            auto webPattern = web.Upgrade();
            if (!webPattern) {
                AccessibilityElementInfo info;
                callback.SetFocusMoveSearchResult(info, requestId);
                TAG_LOGD(AceLogTag::ACE_WEB, "SetFocusMoveSearchResult, requestId: %{public}d",
                    requestId);
                return;
            }
            ACE_SCOPED_TRACE("FocusMoveSearch");
            jsAccessibilityManager->WebFocusMoveSearch(splitElementId, direction, requestId, callback,
                windowId, webPattern);
        },
        TaskExecutor::TaskType::UI, "ArkWebFocusMoveSearch");
}

void JsAccessibilityManager::WebInteractionOperation::ExecuteAction(const int64_t elementId, const int32_t action,
    const std::map<std::string, std::string>& actionArguments, const int32_t requestId,
    Accessibility::AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGD(AceLogTag::ACE_WEB, "elementId: %{public}" PRId64 ", action: %{public}d", elementId, action);
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    if (!jsAccessibilityManager) {
        callback.SetExecuteActionResult(false, requestId);
        TAG_LOGD(AceLogTag::ACE_WEB, "SetExecuteActionResult, requestId: %{public}d", requestId);
        return;
    }
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    if (!context) {
        callback.SetExecuteActionResult(false, requestId);
        TAG_LOGD(AceLogTag::ACE_WEB, "SetExecuteActionResult, requestId: %{public}d", requestId);
        return;
    }
    auto actionInfo = static_cast<ActionType>(action);
    ActionParam param { actionInfo, actionArguments };
    auto windowId = windowId_;
    auto web = webPattern_;
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), splitElementId, param, requestId, &callback, windowId, web] {
            auto jsAccessibilityManager = weak.Upgrade();
            if (!jsAccessibilityManager) {
                callback.SetExecuteActionResult(false, requestId);
                TAG_LOGD(AceLogTag::ACE_WEB, "SetExecuteActionResult, requestId: %{public}d",
                    requestId);
                return;
            }
            auto webPattern = web.Upgrade();
            if (!webPattern) {
                callback.SetExecuteActionResult(false, requestId);
                TAG_LOGD(AceLogTag::ACE_WEB, "SetExecuteActionResult, requestId: %{public}d",
                    requestId);
                return;
            }
            ACE_SCOPED_TRACE("ExecuteAction");
            jsAccessibilityManager->ExecuteWebAction(splitElementId, param, requestId, callback, windowId, webPattern);
        },
        TaskExecutor::TaskType::UI, "ArkWebAccessibilityExecuteAction");
}

void JsAccessibilityManager::WebInteractionOperation::ClearFocus() {}

void JsAccessibilityManager::WebInteractionOperation::SetChildTreeIdAndWinId(const int64_t nodeId, const int32_t treeId,
    const int32_t childWindowId) {}

void JsAccessibilityManager::WebInteractionOperation::SetBelongTreeId(const int32_t treeId) {}

void JsAccessibilityManager::WebInteractionOperation::FocusMoveSearchWithCondition(
    const AccessibilityElementInfo& info, const AccessibilityFocusMoveParam param,
    const int32_t requestId, AccessibilityElementOperatorCallback &callback)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(
        info.GetAccessibilityId(), splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    if (!jsAccessibilityManager) {
        SetWebFocusMoveSearchWithConditionFailResult(callback, FocusMoveResultType::SEARCH_FAIL, requestId);
        return;
    }
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    if (!context) {
        SetWebFocusMoveSearchWithConditionFailResult(callback, FocusMoveResultType::SEARCH_FAIL, requestId);
        return;
    }
    auto web = webPattern_;
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), splitElementId, param, requestId, &callback, web] {
            auto jsAccessibilityManager = weak.Upgrade();
            if (!jsAccessibilityManager) {
                SetWebFocusMoveSearchWithConditionFailResult(callback, FocusMoveResultType::SEARCH_FAIL, requestId);
                return;
            }
            auto webPattern = web.Upgrade();
            if (!webPattern) {
                SetWebFocusMoveSearchWithConditionFailResult(callback, FocusMoveResultType::SEARCH_FAIL, requestId);
                return;
            }
            ACE_SCOPED_TRACE("WebFocusMoveSearchWithCondition");
            jsAccessibilityManager->WebFocusMoveSearchWithCondition(
                splitElementId, param, requestId, callback, webPattern);
        },
        TaskExecutor::TaskType::UI, "ArkWebFocusMoveSearchWithCondition");
}

void JsAccessibilityManager::WebInteractionOperation::UpdateCustomAccessibilityProperty(
    const int64_t elementId, const AccessibilityVirtualNode& accessibilityVirtualNode,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGI(AceLogTag::ACE_WEB, "UpdateCustomAccessibilityProperty id: %{public}" PRId64, elementId);
    callback.SetUpdateCustomAccessibilityPropertyResult(
        Accessibility::OperateVirtualNodeResult::VIRTUAL_NODE_NOT_SUPPORT, requestId);
}

void JsAccessibilityManager::WebInteractionOperation::AddAccessibilityVirtualNode(
    const int64_t elementId, const std::vector<AccessibilityVirtualNode>& nodes,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGI(AceLogTag::ACE_WEB, "AddAccessibilityVirtualNode id: %{public}" PRId64, elementId);
    callback.SetAddAccessibilityVirtualNodeResult(
        Accessibility::OperateVirtualNodeResult::VIRTUAL_NODE_NOT_SUPPORT, requestId);
}

void JsAccessibilityManager::WebInteractionOperation::RemoveAccessibilityVirtualNode(
    const int64_t elementId, const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGI(AceLogTag::ACE_WEB, "RemoveAccessibilityVirtualNode id: %{public}" PRId64, elementId);
    callback.SetRemoveAccessibilityVirtualNodeResult(
        Accessibility::OperateVirtualNodeResult::VIRTUAL_NODE_NOT_SUPPORT, requestId);
}

void JsAccessibilityManager::WebInteractionOperation::GetCursorPosition(
    const int64_t elementId, const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGD(AceLogTag::ACE_WEB, "GetCursorPosition id: %{public}" PRId64, elementId);
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    if (!jsAccessibilityManager) {
        callback.SetCursorPositionResult(0, requestId);
        TAG_LOGD(AceLogTag::ACE_WEB, "SetCursorPositionResult, requestId: %{public}d", requestId);
        return;
    }
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    if (!context) {
        callback.SetCursorPositionResult(0, requestId);
        TAG_LOGD(AceLogTag::ACE_WEB, "SetCursorPositionResult, requestId: %{public}d", requestId);
        return;
    }
    auto web = webPattern_;
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), splitElementId, requestId, &callback, web]() {
            auto jsAccessibilityManager = weak.Upgrade();
            if (!jsAccessibilityManager) {
                callback.SetCursorPositionResult(0, requestId);
                TAG_LOGD(AceLogTag::ACE_WEB, "SetCursorPositionResult, requestId: %{public}d",
                    requestId);
                return;
            }
            auto webPattern = web.Upgrade();
            if (!webPattern) {
                callback.SetCursorPositionResult(0, requestId);
                TAG_LOGD(AceLogTag::ACE_WEB, "SetCursorPositionResult, requestId: %{public}d",
                    requestId);
                return;
            }
            ACE_SCOPED_TRACE("GetWebCursorPosition");
            jsAccessibilityManager->GetWebCursorPosition(splitElementId, requestId, callback, webPattern);
        },

        TaskExecutor::TaskType::UI, "GetWebCursorPosition");
}

void JsAccessibilityManager::WebInteractionOperation::OutsideTouch() {}
#endif

bool JsAccessibilityManager::SearchAccessibilityVirtualNode(int64_t elementId, int32_t mode,
    std::list<AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context)
{
    if (!NG::VirtualNodeContainerIdManager::IsVirtualNodeContainerId(elementId)) {
        return false;
    }

    RefPtr<NG::VirtualAccessibilityNode> virtualNode;
    RefPtr<NG::FrameNode> parentNode;
    bool find = CheckAndGetVirtualNode(elementId, virtualNode, parentNode);
    CHECK_NULL_RETURN(virtualNode, false);
    CHECK_NE_RETURN(find, true, false);

    auto mainContext = context_.Upgrade();
    CHECK_NULL_RETURN(mainContext, false);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(mainContext);
    CHECK_NULL_RETURN(ngPipeline, false);

    AccessibilityElementInfo elementInfo;
    CommonProperty commonProperty;
    GenerateCommonProperty(context, commonProperty, mainContext, parentNode);
    UpdateAccessibilityElementInfoForVirtualNode(
        parentNode, virtualNode, commonProperty, elementInfo, parentNode->GetAccessibilityId());
    infos.push_back(elementInfo);
    return true;
}

void JsAccessibilityManager::SearchElementInfoByAccessibilityId(const int64_t elementId, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback, const int32_t mode, const int32_t windowId)
{
    // set perf monitor
    HiviewDFX::PerfMonitorAdapter::GetInstance().OnSceneChanged(HiviewDFX::ACCESSIBLE_FEATURE, true);
    std::list<AccessibilityElementInfo> infos;

    auto pipeline = GetPipelineByWindowId(windowId);
    if (pipeline) {
        auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
        if (ngPipeline) {
            SearchElementInfoByAccessibilityIdNG(elementId, mode, infos, pipeline, NG::UI_EXTENSION_OFFSET_MAX);
            SetSearchElementInfoByAccessibilityIdResult(callback, std::move(infos), requestId);
            return;
        }
    }

    int64_t nodeId = elementId;
    if (elementId == -1) {
        nodeId = 0;
    }
    auto weak = WeakClaim(this);
    auto jsAccessibilityManager = weak.Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto node = jsAccessibilityManager->GetAccessibilityNodeFromPage(nodeId);
    if (!node) {
        SetSearchElementInfoByAccessibilityIdResult(callback, std::move(infos), requestId);
        return;
    }

    AccessibilityElementInfo nodeInfo;
    UpdateAccessibilityNodeInfo(node, nodeInfo, jsAccessibilityManager, jsAccessibilityManager->windowId_);
    infos.push_back(nodeInfo);
    // cache parent/siblings/children infos
    UpdateCacheInfo(infos, mode, node, jsAccessibilityManager, jsAccessibilityManager->windowId_);

    SetSearchElementInfoByAccessibilityIdResult(callback, std::move(infos), requestId);
}

bool JsAccessibilityManager::SearchAccessibilityVirtualNode(int64_t elementId,
    std::list<AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context)
{
    if (!NG::VirtualNodeContainerIdManager::IsVirtualNodeContainerId(elementId)) {
        return false;
    }

    RefPtr<NG::FrameNode> parentNode;
    RefPtr<NG::VirtualAccessibilityNode> virtualNode;
    auto find = CheckAndGetVirtualNode(elementId, virtualNode, parentNode);
    CHECK_NE_RETURN(find, true, false);

    auto mainContext = context_.Upgrade();
    CHECK_NULL_RETURN(mainContext, false);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_RETURN(ngPipeline, false);
    AccessibilityElementInfo elementInfo;
    CommonProperty commonProperty;
    GenerateCommonProperty(context, commonProperty, mainContext, parentNode);
    UpdateAccessibilityElementInfoForVirtualNode(
        parentNode, virtualNode, commonProperty, elementInfo, parentNode->GetAccessibilityId());
    infos.push_back(elementInfo);
    return true;
}

void JsAccessibilityManager::SearchElementInfoByAccessibilityIdNG(int64_t elementId, int32_t mode,
    std::list<AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context, int64_t uiExtensionOffset)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "elementId: %{public}" PRId64 ", treeId: %{public}d, mode: %{public}d",
        elementId, treeId_, mode);
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);

    if (SearchAccessibilityVirtualNode(elementId, infos, context)) {
        return;
    }

    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);

    AccessibilityElementInfo nodeInfo;
    int64_t nodeId = (elementId == -1) ? rootNode->GetAccessibilityId() : elementId;

#ifdef WINDOW_SCENE_SUPPORTED
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    if (uiExtensionManager->IsWrapExtensionAbilityId(nodeId)) {
        SearchParameter param {nodeId, "", mode, uiExtensionOffset};
        return SearchExtensionElementInfoByAccessibilityIdNG(param, rootNode, infos, context, ngPipeline);
    }
#endif

    auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(rootNode, nodeId);
    if (!node && IsCmpSubAM()) {
        // Adopted exported subtrees are unreachable from the main root (children_ excludes adopted children).
        node = CmpAccessibility::CmpAccessibilityRegistry::FindNodeInExportedSubtrees(nodeId);
    }
    CHECK_NULL_VOID(node);
    if (!IsCmpSubAM() && CmpAccessibility::CmpAccessibilityRegistry::HasAnySubAM() &&
        CmpAccessibility::CmpAccessibilityRegistry::HasSubAM(node.GetRawPtr())) {
        // Id spaces: main-tree nodes leave with RAW ids while exported- subtree ids are treeId-spliced.
        RefPtr<NG::FrameNode> subtreeNode;
        CmpAccessibility::CmpAccessibilityRegistry::FindSubAMAndNodeInExportedSubtrees(
            node->GetAccessibilityId(), subtreeNode);
        if (subtreeNode && subtreeNode.GetRawPtr() == node.GetRawPtr()) {
            return;
        }
    }
    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext, node);
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
        "windowId: %{public}d, windowLeft: %{public}d, "
        "windowTop: %{public}d",
        commonProperty.windowId, commonProperty.windowLeft, commonProperty.windowTop);
    UpdateElementInfo(node, commonProperty, nodeInfo, ngPipeline);
    SetRootAccessibilityVisible(node, nodeInfo);
    SetRootAccessibilityNextFocusId(node, rootNode, nodeInfo);
    SetRootAccessibilityPreFocusId(node, rootNode, nodeInfo,
        nextFocusRelationController_.GetTargetMap(context->GetInstanceId()));
    if (IsExtensionComponent(node) && !IsUIExtensionShowPlaceholder(node)) {
        SearchParameter param {-1, "", mode, uiExtensionOffset};
        SearchExtensionElementInfoNG(param, node, infos, nodeInfo);
    }
    infos.push_back(nodeInfo);
    SearchParameter param {nodeId, "", mode, uiExtensionOffset};
    UpdateCacheInfoNG(infos, node, commonProperty, ngPipeline, param);
    SortExtensionAccessibilityInfo(infos, nodeInfo.GetAccessibilityId());
    if ((infos.size() > 0) && (uiExtensionOffset != NG::UI_EXTENSION_OFFSET_MAX) &&
        (infos.front().GetComponentType() != V2::ROOT_ETS_TAG) &&
        (infos.front().GetParentNodeId() == rootNode->GetAccessibilityId())) {
            infos.front().SetParent(NG::UI_EXTENSION_ROOT_ID);
    }
    UpdateWebEmbedParent(infos, node, commonProperty);
}

bool JsAccessibilityManager::SetAccessibilityCustomId(RefPtr<NG::FrameNode> checkNode, const std::string &customId,
    CommonProperty &commonProperty, std::list<AccessibilityElementInfo> &infos, const RefPtr<PipelineBase>& context)
{
    if (checkNode->GetInspectorId().value_or("") == customId) {
        auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
        CHECK_NULL_RETURN(ngPipeline, false);
        auto rootNode = ngPipeline->GetRootElement();
        CHECK_NULL_RETURN(rootNode, false);
        AccessibilityElementInfo nodeInfo;
        UpdateAccessibilityElementInfo(checkNode, commonProperty, nodeInfo, ngPipeline);
        SetRootAccessibilityVisible(checkNode, nodeInfo);
        SetRootAccessibilityNextFocusId(checkNode, rootNode, nodeInfo);
        SetRootAccessibilityPreFocusId(checkNode, rootNode, nodeInfo,
            nextFocusRelationController_.GetTargetMap(context->GetInstanceId()));
        infos.emplace_back(nodeInfo);
        return true;
    }
    return false;
}

bool JsAccessibilityManager::FindUIExtensionAccessibilityElement(const RefPtr<NG::FrameNode>& checkNode,
    const std::string &customId, const CommonProperty& commonProperty,
    std::list<AccessibilityElementInfo> &infos, const RefPtr<PipelineBase>& context)
{
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_RETURN(ngPipeline, false);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_RETURN(rootNode, false);
    if (!((checkNode->GetUiExtensionId() > NG::UI_EXTENSION_UNKNOW_ID) &&
            (checkNode->GetUiExtensionId() <= NG::UI_EXTENSION_ID_FIRST_MAX))) {
        return false;
    }

    SearchParameter searchParam {
        .nodeId = NG::UI_EXTENSION_ROOT_ID,
        .mode = static_cast<int32_t>(PREFETCH_RECURSIVE_CHILDREN),
        .uiExtensionOffset = NG::UI_EXTENSION_OFFSET_MAX,
    };

    AccessibilityElementInfo parentInfo;
    UpdateAccessibilityElementInfo(checkNode, commonProperty, parentInfo, ngPipeline);
    SetRootAccessibilityVisible(checkNode, parentInfo);
    SetRootAccessibilityNextFocusId(checkNode, rootNode, parentInfo);
    SetRootAccessibilityPreFocusId(checkNode, rootNode, parentInfo,
        nextFocusRelationController_.GetTargetMap(ngPipeline->GetInstanceId()));

    std::list<Accessibility::AccessibilityElementInfo> extensionElementInfos;
    SearchExtensionElementInfoNG(searchParam, checkNode, extensionElementInfos, parentInfo);

    for (auto& ueaInfo : extensionElementInfos) {
        if (ueaInfo.GetInspectorKey() == customId) {
            infos.emplace_back(ueaInfo);
            break;
        }
    }
    if (!infos.empty()) {
        return true;
    }
    return false;
}

void JsAccessibilityManager::SearchElementInfoByCustomIdNG(const int64_t elementId, const std::string &customId,
    std::list<AccessibilityElementInfo> &infos, std::list<AccessibilityElementInfo> &treeInfos,
    const RefPtr<PipelineBase>& context)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "elementId: %{public}" PRId64 ", customId: %{public}s",
        elementId, customId.c_str());
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);

    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);

    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext, rootNode);
    commonProperty.checkEmbedNode = false;
    int64_t nodeId = elementId;
    if (elementId == -1) {
        nodeId = rootNode->GetAccessibilityId();
    }
    auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(rootNode, nodeId);
    CHECK_NULL_VOID(node);
    std::list<RefPtr<NG::FrameNode>> children { node };
    while (!children.empty()) {
        auto checkNode = children.front();
        children.pop_front();
        if (SetAccessibilityCustomId(checkNode, customId, commonProperty, infos, context)) {
            treeInfos.clear();
            break;
        }
#ifdef WINDOW_SCENE_SUPPORTED
        auto isFind = FindUIExtensionAccessibilityElement(checkNode, customId, commonProperty, infos, context);
        if (isFind) {
            treeInfos.clear();
            break;
        }
#endif
        auto accessibilityProperty = checkNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
        if (accessibilityProperty && accessibilityProperty->GetChildTreeId() != -1) {
            AccessibilityElementInfo nodeInfo;
            UpdateAccessibilityElementInfo(checkNode, commonProperty, nodeInfo, ngPipeline);
            SetRootAccessibilityVisible(checkNode, nodeInfo);
            SetRootAccessibilityNextFocusId(checkNode, rootNode, nodeInfo);
            SetRootAccessibilityPreFocusId(checkNode, rootNode, nodeInfo,
                nextFocusRelationController_.GetTargetMap(context->GetInstanceId()));
            treeInfos.emplace_back(nodeInfo);
        }
        GetChildrenFromFrameNode(checkNode, children, commonProperty);
    }
}

void JsAccessibilityManager::SearchExtensionElementInfoByAccessibilityIdNG(const SearchParameter& searchParam,
    const RefPtr<NG::FrameNode>& node, std::list<Accessibility::AccessibilityElementInfo>& infos,
    const RefPtr<PipelineBase>& context, const RefPtr<NG::PipelineContext>& ngPipeline)
{
#ifdef WINDOW_SCENE_SUPPORTED
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    auto unWrapIdPair = uiExtensionManager->UnWrapExtensionAbilityId(searchParam.uiExtensionOffset, searchParam.nodeId);
    int64_t childWrapId = unWrapIdPair.second;
    int64_t uiExtensionId = unWrapIdPair.first;
    auto uiExtensionNode = FindNodeFromRootByExtensionId(node, uiExtensionId);
    CHECK_NULL_VOID(uiExtensionNode);
    SearchParameter param {childWrapId, "", searchParam.mode, searchParam.uiExtensionOffset};
    AccessibilityElementInfo nodeInfo;
    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext, uiExtensionNode);
    UpdateAccessibilityElementInfo(uiExtensionNode, commonProperty, nodeInfo, ngPipeline);
    SearchExtensionElementInfoNG(param, uiExtensionNode, infos, nodeInfo);
#endif
}

std::list<AccessibilityElementInfo> JsAccessibilityManager::SearchElementInfosByTextNG(
    int64_t elementId, const std::string& text, const RefPtr<NG::FrameNode>& node, int64_t offset)
{
    std::list<AccessibilityElementInfo> extensionElementInfo;
    if (NG::UI_EXTENSION_OFFSET_MIN < (offset + 1)) {
        node->SearchElementInfosByTextNG(elementId, text, offset, extensionElementInfo);
    }
    return extensionElementInfo;
}

void JsAccessibilityManager::SearchElementInfosByTextNG(const SearchParameter& searchParam,
    const RefPtr<NG::FrameNode>& node, std::list<Accessibility::AccessibilityElementInfo>& infos,
    const RefPtr<PipelineBase>& context, const RefPtr<NG::PipelineContext>& ngPipeline)
{
#ifdef WINDOW_SCENE_SUPPORTED
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    auto unWrapIdPair = uiExtensionManager->UnWrapExtensionAbilityId(searchParam.uiExtensionOffset, searchParam.nodeId);
    int64_t childWrapId = unWrapIdPair.second;
    int64_t uiExtensionId = unWrapIdPair.first;
    std::list<AccessibilityElementInfo> extensionElementInfos;
    AccessibilityElementInfo nodeInfo;
    auto uiExtensionNode = FindNodeFromRootByExtensionId(node, uiExtensionId);
    CHECK_NULL_VOID(uiExtensionNode);

    extensionElementInfos = SearchElementInfosByTextNG(
        childWrapId, searchParam.text, uiExtensionNode, searchParam.uiExtensionOffset / NG::UI_EXTENSION_ID_FACTOR);
    if (extensionElementInfos.empty()) {
        return;
    }
    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext, uiExtensionNode);
    UpdateAccessibilityElementInfo(uiExtensionNode, commonProperty, nodeInfo, ngPipeline);
    SetRootAccessibilityVisible(node, nodeInfo);

    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    SetRootAccessibilityNextFocusId(node, rootNode, nodeInfo);
    SetRootAccessibilityPreFocusId(node, rootNode, nodeInfo,
        nextFocusRelationController_.GetTargetMap(context->GetInstanceId()));
    ConvertExtensionAccessibilityNodeId(extensionElementInfos, uiExtensionNode,
        searchParam.uiExtensionOffset, nodeInfo);
    for (auto& info : extensionElementInfos) {
        infos.emplace_back(info);
    }
#endif
}

void JsAccessibilityManager::SearchElementInfoBySpecificProperty(const int64_t elementId,
    const SpecificPropertyParam &param, const int32_t requestId,
    AccessibilityElementOperatorCallback &callback, const int32_t windowId)
{
    auto context = GetPipelineByWindowId(windowId);
    if (!context) {
        std::list<AccessibilityElementInfo> infos;
        std::list<AccessibilityElementInfo> treeInfos;
        callback.SetSearchElementInfoBySpecificPropertyResult(infos, treeInfos, requestId);
    }
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    if (ngPipeline) {
        ngPipeline->AddAfterRenderTask(
            [weak = WeakClaim(this), elementId, param, requestId, &callback, windowId]() {
                auto jsAccessibilityManager = weak.Upgrade();
                if (!jsAccessibilityManager) {
                    std::list<AccessibilityElementInfo> infos;
                    std::list<AccessibilityElementInfo> treeInfos;
                    callback.SetSearchElementInfoBySpecificPropertyResult(infos, treeInfos, requestId);
                    return;
                }
                jsAccessibilityManager->SearchElementInfoBySpecificPropertyInner(
                    elementId, param, requestId, callback, windowId);
            }
        );
        ngPipeline->RequestFrame();
    } else {
        SearchElementInfoBySpecificPropertyInner(elementId, param, requestId, callback, windowId);
    }
}

void JsAccessibilityManager::SearchElementInfoBySpecificPropertyInner(const int64_t elementId,
    const SpecificPropertyParam &param, const int32_t requestId,
    AccessibilityElementOperatorCallback &callback, const int32_t windowId)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "elementId: %{public}" PRId64 ", propType: %{public}d,"
        "propTarget: %{public}s", elementId, param.propertyType, param.propertyTarget.c_str());
    std::list<AccessibilityElementInfo> infos;
    std::list<AccessibilityElementInfo> treeInfos;

    auto pipeline = GetPipelineByWindowId(windowId);
    if (pipeline) {
        auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
        if (ngPipeline) {
            if (param.propertyType == SEARCH_TYPE::CUSTOMID) {
                SearchElementInfoByCustomIdNG(elementId, param.propertyTarget, infos, treeInfos, pipeline);
            }
        } else {
            SearchAccessibilityNodeBySpecificProperty(elementId, param, requestId, callback, windowId);
        }
    }

    SetSearchElementInfoByCustomIdResult(callback, infos, treeInfos, requestId);
    return;
}

RefPtr<NG::FrameNode> JsAccessibilityManager::FindNodeFromRootByExtensionId(
    const RefPtr<NG::FrameNode>& root, const int64_t uiExtensionId)
{
    CHECK_NULL_RETURN(root, nullptr);
    std::queue<RefPtr<NG::UINode>> nodes;
    nodes.push(root);
    RefPtr<NG::FrameNode> frameNode;
    while (!nodes.empty()) {
        auto current = nodes.front();
        nodes.pop();
        frameNode = AceType::DynamicCast<NG::FrameNode>(current);
        if (IsExtensionComponent(frameNode) && !IsUIExtensionShowPlaceholder(frameNode) &&
            (uiExtensionId == frameNode->GetUiExtensionId())) {
            return frameNode;
        }
        const auto& children = current->GetChildren(true);
        for (const auto& child : children) {
            nodes.push(child);
        }
    }
    return nullptr;
}

void JsAccessibilityManager::SearchElementInfosByTextNG(int64_t elementId, const std::string& text,
    std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context,
    int64_t uiExtensionOffset)
{
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);
#ifdef WINDOW_SCENE_SUPPORTED
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    if (uiExtensionManager->IsWrapExtensionAbilityId(elementId)) {
        SearchParameter param {elementId, text, 0, uiExtensionOffset};
        SearchElementInfosByTextNG(param, rootNode, infos, context, ngPipeline);
        return;
    }
#endif
    if (elementId == NG::UI_EXTENSION_ROOT_ID) {
        elementId = rootNode->GetAccessibilityId();
    }
    auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(rootNode, elementId);
    CHECK_NULL_VOID(node);
    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext, node);
    commonProperty.checkEmbedNode = false;
    nlohmann::json textJson = nlohmann::json::parse(text, nullptr, false);
    if (textJson.is_null() || textJson.is_discarded() || !textJson.contains("type")) {
        return;
    }
    if (textJson["type"] == "textType") {
        SearchParameter param {0, text, 0, uiExtensionOffset};
        FindTextByTextHint(node, infos, ngPipeline, commonProperty, param);
        return;
    }
    if (!textJson.contains("value")) {
        return;
    }
    SearchParameter param {0, textJson["value"], 0, uiExtensionOffset};
    FindText(node, infos, ngPipeline, commonProperty, param);
}

void JsAccessibilityManager::JsInteractionOperation::SearchElementInfosByText(const int64_t elementId,
    const std::string& text, const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "search by text: %{public}" PRId64 ", text: %{public}s",
        elementId, text.c_str());
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    if (text.empty()) {
        return;
    }
    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    auto windowId = windowId_;
    if (context) {
        context->GetTaskExecutor()->PostTask(
            [weak = GetHandler(), splitElementId, text, requestId, &callback, windowId]() {
                auto jsAccessibilityManager = weak.Upgrade();
                CHECK_NULL_VOID(jsAccessibilityManager);
                ACE_SCOPED_TRACE("SearchElementInfosByText");
                jsAccessibilityManager->SearchElementInfosByText(
                    splitElementId, text, requestId, callback, windowId);
            },
            TaskExecutor::TaskType::UI, "ArkUIAccessibilitySearchElementInfoByText");
    }
}

void JsAccessibilityManager::SearchElementInfosByText(const int64_t elementId, const std::string& text,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback, const int32_t windowId)
{
    if (text.empty()) {
        return;
    }

    if (elementId == -1) {
        return;
    }

    std::list<AccessibilityElementInfo> infos;

    auto pipeline = GetPipelineByWindowId(windowId);
    if (pipeline) {
        if (AceType::InstanceOf<NG::PipelineContext>(pipeline)) {
            SearchElementInfosByTextNG(elementId, text, infos, pipeline, NG::UI_EXTENSION_OFFSET_MAX);
            SetSearchElementInfoByTextResult(callback, std::move(infos), requestId);
            return;
        }
    }

    auto weak = WeakClaim(this);
    auto jsAccessibilityManager = weak.Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    int64_t nodeId = elementId;
    auto node = jsAccessibilityManager->GetAccessibilityNodeFromPage(nodeId);
    CHECK_NULL_VOID(node);
    std::list<RefPtr<AccessibilityNode>> nodeList;
    OHOS::Ace::Framework::FindText(node, text, nodeList);
    if (!nodeList.empty()) {
        for (const auto& node : nodeList) {
            AccessibilityElementInfo nodeInfo;
            UpdateAccessibilityNodeInfo(node, nodeInfo, jsAccessibilityManager, jsAccessibilityManager->windowId_);
            infos.emplace_back(nodeInfo);
        }
    }

    SetSearchElementInfoByTextResult(callback, std::move(infos), requestId);
}

void JsAccessibilityManager::JsInteractionOperation::SearchDefaultFocusByWindowId(const int32_t windowId,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback, const int32_t pageId)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "search by windowId: %{public}d, pageId: %{public}d.", windowId, pageId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), windowId, pageId, requestId, &callback]() {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            ACE_SCOPED_TRACE("SearchDefaultFocusByWindowId");
            jsAccessibilityManager->SearchDefaultFocusByWindowId(
                windowId, pageId, requestId, callback);
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilitySearchDefaultFocusByWindowId");
}

void JsAccessibilityManager::SearchDefaultFocusByWindowId(const int32_t windowId, int32_t pageId,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    std::list<AccessibilityElementInfo> infos;

    auto pipeline = GetPipelineByWindowId(windowId);
    if (pipeline) {
        auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
        if (ngPipeline) {
            SearchDefaultFocusByWindowIdNG(pageId, infos, pipeline);
            SetSearchDefaultFocusByWindowIdResult(callback, std::move(infos), requestId);
            return;
        }
    }

    auto defaultFocusNodeList = GetDefaultFocusList();
    for (const auto& defaultFocusNode : defaultFocusNodeList) {
        auto frameNode = defaultFocusNode.Upgrade();
        if (frameNode && (frameNode->GetPageId() == pageId || pageId == -1)) {
            auto node = GetAccessibilityNodeFromPage(frameNode->GetAccessibilityId());
            CHECK_NULL_CONTINUE(node);
            AccessibilityElementInfo nodeInfo;
            UpdateAccessibilityNodeInfo(node, nodeInfo, Claim(this), windowId_);
            infos.emplace_back(nodeInfo);
        }
    }
    SetSearchDefaultFocusByWindowIdResult(callback, std::move(infos), requestId);
}

void JsAccessibilityManager::SearchDefaultFocusByWindowIdNG(const int32_t pageId,
    std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context)
{
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);

    for (const auto& defaultFocusNode : defaultFocusList_) {
        auto node = defaultFocusNode.Upgrade();
        CHECK_NULL_CONTINUE(node);
        auto checkPageId = GetNowPageId(context, pageId);
        if ((pageId == -1) && NotInPage(node)) {
            checkPageId = -1;
        }

        if (node->GetHostPageId() == checkPageId) {
            AccessibilityElementInfo nodeInfo;

            CommonProperty commonProperty;
            GenerateCommonProperty(ngPipeline, commonProperty, mainContext, node);
            UpdateAccessibilityElementInfo(node, commonProperty, nodeInfo, ngPipeline);
            infos.emplace_back(nodeInfo);
        }
    }
}

void JsAccessibilityManager::JsInteractionOperation::FindFocusedElementInfo(const int64_t elementId,
    const int32_t focusType, const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "find focus: %{public}" PRId64 ", focusType: %{public}d",
        elementId, focusType);
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    auto windowId = windowId_;
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), splitElementId, focusType, requestId, &callback, windowId]() {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            ACE_SCOPED_TRACE("FindFocusedElementInfo");
            jsAccessibilityManager->FindFocusedElementInfo(splitElementId, focusType, requestId, callback, windowId);
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilityFindFocusedElementInfo");
}

void JsAccessibilityManager::FindFocusedElementInfo(const int64_t elementId, const int32_t focusType,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback, const int32_t windowId)
{
    AccessibilityElementInfo nodeInfo;
    if (focusType != FOCUS_TYPE_INPUT && focusType != FOCUS_TYPE_ACCESSIBILITY) {
        nodeInfo.SetValidElement(false);
        SetFindFocusedElementInfoResult(callback, nodeInfo, requestId);
        return;
    }

    auto context = GetPipelineByWindowId(windowId);
    if (!context) {
        SetFindFocusedElementInfoResult(callback, nodeInfo, requestId);
        return;
    }

    if (AceType::InstanceOf<NG::PipelineContext>(context)) {
        FindFocusedElementInfoNG(elementId, focusType, nodeInfo, context, NG::UI_EXTENSION_OFFSET_MAX);
        SetFindFocusedElementInfoResult(callback, nodeInfo, requestId);
        return;
    }

    int64_t nodeId = static_cast<int64_t>(elementId);
    if (elementId == -1) {
        nodeId = 0;
    }

    auto node = GetAccessibilityNodeFromPage(nodeId);
    if (!node) {
        nodeInfo.SetValidElement(false);
        SetFindFocusedElementInfoResult(callback, nodeInfo, requestId);
        return;
    }

    RefPtr<AccessibilityNode> resultNode = nullptr;
    bool status = false;
    if (focusType == FOCUS_TYPE_ACCESSIBILITY) {
        status = FindAccessibilityFocus(node, resultNode);
    }
    if (focusType == FOCUS_TYPE_INPUT) {
        status = FindInputFocus(node, resultNode);
    }

    if ((status) && (resultNode != nullptr)) {
        UpdateAccessibilityNodeInfo(resultNode, nodeInfo, Claim(this), windowId_);
    }

    SetFindFocusedElementInfoResult(callback, nodeInfo, requestId);
}

void JsAccessibilityManager::FindFocusedElementInfoNG(int64_t elementId, int32_t focusType,
    Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
    const int64_t uiExtensionOffset)
{
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);
#ifdef WINDOW_SCENE_SUPPORTED
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    if (uiExtensionManager->IsWrapExtensionAbilityId(elementId)) {
        SearchParameter param {elementId, "", focusType, uiExtensionOffset};
        return FindFocusedExtensionElementInfoNG(param, info, context, rootNode);
    }
#endif
    int64_t nodeId = elementId;
    if (elementId == -1) {
        nodeId = rootNode->GetAccessibilityId();
    }
    auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(rootNode, nodeId);
    if (!node) {
        return info.SetValidElement(false);
    }
    if (IsExtensionComponent(node) && !IsUIExtensionShowPlaceholder(node)) {
        SearchParameter transferSearchParam {NG::UI_EXTENSION_ROOT_ID, "", focusType, uiExtensionOffset};
        OHOS::Ace::Framework::FindFocusedExtensionElementInfoNG(transferSearchParam, node, info);
        return SetUiExtensionAbilityParentIdForFocus(node, uiExtensionOffset, info);
    }
    RefPtr<NG::FrameNode> resultNode;
    if (focusType == FOCUS_TYPE_ACCESSIBILITY) {
        resultNode = FindAccessibilityFocus(node, focusType, info, uiExtensionOffset, context, currentFocusNodeId_);
    }
    if (focusType == FOCUS_TYPE_INPUT) {
        resultNode = FindInputFocus(node, focusType, info, uiExtensionOffset, context);
    }
    if ((!resultNode) || (IsExtensionComponent(resultNode) && !IsUIExtensionShowPlaceholder(resultNode))) {
        return;
    }
    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext, resultNode);
    UpdateAccessibilityElementInfo(resultNode, commonProperty, info, ngPipeline);
    SetRootAccessibilityVisible(resultNode, info);
    SetRootAccessibilityNextFocusId(resultNode, rootNode, info);
    SetRootAccessibilityPreFocusId(resultNode, rootNode, info,
        nextFocusRelationController_.GetTargetMap(context->GetInstanceId()));
    UpdateUiExtensionParentIdForFocus(rootNode, uiExtensionOffset, info);
}

void JsAccessibilityManager::FindFocusedExtensionElementInfoNG(const SearchParameter& searchParam,
    Accessibility::AccessibilityElementInfo& info,
    const RefPtr<PipelineBase>& context, const RefPtr<NG::FrameNode>& root)
{
#ifdef WINDOW_SCENE_SUPPORTED
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    auto elementIdPair = uiExtensionManager->UnWrapExtensionAbilityId(searchParam.uiExtensionOffset,
        searchParam.nodeId);
    auto uiExtensionNode = uiExtensionManager->GetFocusUiExtensionNode();
    CHECK_NULL_VOID(uiExtensionNode);
    SearchParameter transferSearchParam {elementIdPair.second, "",
        searchParam.mode, searchParam.uiExtensionOffset};
    OHOS::Ace::Framework::FindFocusedExtensionElementInfoNG(transferSearchParam, uiExtensionNode, info);
    SetUiExtensionAbilityParentIdForFocus(uiExtensionNode, searchParam.uiExtensionOffset, info);
#endif
}

RefPtr<NG::FrameNode> JsAccessibilityManager::FindNodeFromPipeline(
    const WeakPtr<PipelineBase>& context, const int64_t elementId)
{
    auto pipeline = context.Upgrade();
    CHECK_NULL_RETURN(pipeline, nullptr);

    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    CHECK_NULL_RETURN(ngPipeline, nullptr);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_RETURN(rootNode, nullptr);

    int64_t nodeId = elementId;
    // accessibility use -1 for first search to get root node
    if (elementId == -1) {
        nodeId = rootNode->GetAccessibilityId();
    }

    auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(rootNode, nodeId);
    if (node) {
        return node;
    }
    return nullptr;
}

RefPtr<NG::PipelineContext> JsAccessibilityManager::FindPipelineByElementId(
    const int64_t elementId, RefPtr<NG::FrameNode>& node)
{
    node = FindNodeFromPipeline(context_, elementId);
    if (node) {
        auto context = AceType::DynamicCast<NG::PipelineContext>(context_.Upgrade());
        return context;
    }
    for (auto subContext : GetSubPipelineContexts()) {
        node = FindNodeFromPipeline(subContext, elementId);
        if (node) {
            auto context = AceType::DynamicCast<NG::PipelineContext>(subContext.Upgrade());
            return context;
        }
    }
    // Adopted subtrees hang off adoptedChildren_ (separate from children_), so the pipeline-root walk never reaches them.
    if (CmpAccessibility::CmpAccessibilityRegistry::HasAnySubAM()) {
        auto found = CmpAccessibility::CmpAccessibilityRegistry::FindNodeInExportedSubtrees(elementId);
        if (found) {
            node = found;
            return AceType::DynamicCast<NG::PipelineContext>(context_.Upgrade());
        }
    }
    return nullptr;
}

void JsAccessibilityManager::JsInteractionOperation::ExecuteAction(const int64_t elementId, const int32_t action,
    const std::map<std::string, std::string>& actionArguments, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "ExecuteAction elementId: %{public}" PRId64 ", action: %{public}d",
             elementId, action);
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    if (!jsAccessibilityManager) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "ExecuteAction failed, requestId: %{public}d", requestId);
        callback.SetExecuteActionResult(false, requestId);
        return;
    }
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    if (!context) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "ExecuteAction failed, requestId: %{public}d", requestId);
        callback.SetExecuteActionResult(false, requestId);
        return;
    }
    auto actionInfo = static_cast<ActionType>(action);
    ActionParam param { actionInfo, actionArguments };
    auto windowId = windowId_;
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), splitElementId, param, requestId, &callback, windowId] {
            auto jsAccessibilityManager = weak.Upgrade();
            if (!jsAccessibilityManager) {
                TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "ExecuteAction failed, requestId: %{public}d", requestId);
                callback.SetExecuteActionResult(false, requestId);
                return;
            }
            ACE_SCOPED_TRACE("ExecuteAction");
            jsAccessibilityManager->ExecuteAction(splitElementId, param, requestId, callback, windowId);
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilityExecuteAction");
}

bool JsAccessibilityManager::AccessibilityActionEvent(const ActionType& action,
    const std::map<std::string, std::string>& actionArguments, const RefPtr<AccessibilityNode>& node,
    const RefPtr<PipelineContext>& context)
{
    if (!node || !context) {
        return false;
    }
    ContainerScope scope(context->GetInstanceId());
    switch (action) {
        case ActionType::ACCESSIBILITY_ACTION_CLICK: {
            node->SetClicked(true);
            if (!node->GetClickEventMarker().IsEmpty()) {
#ifndef NG_BUILD
                context->SendEventToFrontend(node->GetClickEventMarker());
#endif
                node->ActionClick();
                return true;
            }
            return node->ActionClick();
        }
        case ActionType::ACCESSIBILITY_ACTION_LONG_CLICK: {
            if (!node->GetLongPressEventMarker().IsEmpty()) {
#ifndef NG_BUILD
                context->SendEventToFrontend(node->GetLongPressEventMarker());
#endif
                node->ActionLongClick();
                return true;
            }
            return node->ActionLongClick();
        }
        case ActionType::ACCESSIBILITY_ACTION_SET_TEXT: {
            if (!node->GetSetTextEventMarker().IsEmpty()) {
#ifndef NG_BUILD
                context->SendEventToFrontend(node->GetSetTextEventMarker());
#endif
                node->ActionSetText(actionArguments.find(ACTION_ARGU_SET_TEXT)->second);
                return true;
            }
            return node->ActionSetText(actionArguments.find(ACTION_ARGU_SET_TEXT)->second);
        }
        case ActionType::ACCESSIBILITY_ACTION_FOCUS: {
#ifndef NG_BUILD
            context->AccessibilityRequestFocus(std::to_string(node->GetNodeId()));
#endif
            if (!node->GetFocusEventMarker().IsEmpty()) {
#ifndef NG_BUILD
                context->SendEventToFrontend(node->GetFocusEventMarker());
#endif
                node->ActionFocus();
                return true;
            }
            return node->ActionFocus();
        }
        case ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS: {
            return RequestAccessibilityFocus(node);
        }
        case ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS: {
            return ClearAccessibilityFocus(node);
        }
        case ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD:
            return node->ActionScrollForward();
        case ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD:
            return node->ActionScrollBackward();
        default:
            return false;
    }
}

void JsAccessibilityManager::SendActionEvent(const Accessibility::ActionType& action, int64_t nodeId)
{
    static std::unordered_map<Accessibility::ActionType, std::string> actionToStr {
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_CLICK, DOM_CLICK },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_LONG_CLICK, DOM_LONG_PRESS },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_FOCUS, DOM_FOCUS },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS, ACCESSIBILITY_FOCUSED_EVENT },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS, ACCESSIBILITY_CLEAR_FOCUS_EVENT },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD, SCROLL_END_EVENT },
        { Accessibility::ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD, SCROLL_END_EVENT },
    };
    if (actionToStr.find(action) == actionToStr.end()) {
        return;
    }
    AccessibilityEvent accessibilityEvent;
    accessibilityEvent.eventType = actionToStr[action];
    accessibilityEvent.nodeId = static_cast<int64_t>(nodeId);
    SendAccessibilityAsyncEvent(accessibilityEvent);
}

namespace {
void stringToLower(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char &c) {
        return std::tolower(c);
    });
}

bool conversionDirection(std::string dir)
{
    stringToLower(dir);
    if (dir.compare(STRING_DIR_FORWARD) == 0) {
        return true;
    }

    return false;
}

bool IsFocusMoveAction(const std::map<std::string, std::string>& actionArguments)
{
    auto iter = actionArguments.find(ACTION_ARGU_FOCUS_TYPE_STUB);
    if (iter == actionArguments.end()) {
        return false;
    }
    return iter->second == ACTION_ARGU_FOCUS_MOVE_VALUE;
}

int32_t getArgumentByKey(const std::map<std::string, std::string>& actionArguments, const std::string& checkKey)
{
    auto iter = actionArguments.find(checkKey);
    int32_t argument = -1; // -1:default value
    if (iter != actionArguments.end()) {
        std::stringstream strArguments;
        strArguments << iter->second;
        strArguments >> argument;
    }
    return argument;
}

uint64_t GetUnsignedLongArgumentByKey(const std::map<std::string, std::string>& actionArguments,
    const std::string& checkKey)
{
    auto iter = actionArguments.find(checkKey);
    uint64_t argument = 0; // 0:default value
    if (iter != actionArguments.end()) {
        std::stringstream strArguments;
        strArguments << iter->second;
        strArguments >> argument;
    }
    return argument;
}

std::string GetStrArgumentByKey(const std::map<std::string, std::string>& actionArguments, const std::string& checkKey)
{
    std::string result;
    auto iter = actionArguments.find(checkKey);
    if (iter != actionArguments.end()) {
        result = iter->second;
    }
    return result;
}

AccessibilityScrollType findAccessibilityScrollType(int32_t accessibilityScrollTypeValue)
{
    switch (accessibilityScrollTypeValue) {
        case 0:
            return AccessibilityScrollType::SCROLL_HALF;
        case 1:
            return AccessibilityScrollType::SCROLL_FULL;
        default:
            return AccessibilityScrollType::SCROLL_DEFAULT;
    }
}

NG::SecCompEnhanceEvent GetAccessibilitySecCompEnhanceEvent(const std::map<std::string, std::string>& actionArguments)
{
    NG::SecCompEnhanceEvent secEvent;
    auto hmac = GetStrArgumentByKey(actionArguments, ACTION_ARGU_CLICK_ENHANCE_DATA);
    if (hmac.empty()) {
        return secEvent;
    }
    auto time = GetUnsignedLongArgumentByKey(actionArguments, ACTION_ARGU_CLICK_TIMESTAMP);
    secEvent.dataBuffer.assign(hmac.begin(), hmac.end());
    std::chrono::microseconds microseconds(time);
    secEvent.time = TimeStamp { microseconds };
    return secEvent;
}

AccessibilityScrollType getAccessibilityScrollType(const std::map<std::string, std::string>& actionArguments,
    const std::string& checkKey)
{
    auto argument = getArgumentByKey(actionArguments, checkKey);
    return findAccessibilityScrollType(argument);
}
}
void JsAccessibilityManager::ResetBlockedEvent()
{
    blockerInAction_.Reset();
}

void JsAccessibilityManager::ActAccessibilityActionPreHandle(Accessibility::ActionType action,
    const RefPtr<NG::FrameNode>& frameNode)
{
    // text update event not send while in SET_TEXT action
    CHECK_NE_VOID(action, ActionType::ACCESSIBILITY_ACTION_SET_TEXT);
    CHECK_NULL_VOID(frameNode);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(frameNode->GetContextRefPtr());
    CHECK_NULL_VOID(ngPipeline);

    std::vector<AccessibilityEventType> events = {
        AccessibilityEventType::TEXT_CHANGE,
        AccessibilityEventType::COMPONENT_CHANGE
    };
    blockerInAction_.SetBlockedEvents(frameNode->GetAccessibilityId(), events);
    ngPipeline->AddAfterRenderTask(
        [weak = WeakClaim(this)]() {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            jsAccessibilityManager->ResetBlockedEvent();
        }
    );
}

bool JsAccessibilityManager::ActAccessibilityAction(Accessibility::ActionType action,
    const std::map<std::string, std::string>& actionArguments,
    RefPtr<NG::AccessibilityProperty> accessibilityProperty, const RefPtr<NG::FrameNode>& frameNode)
{
    AccessibilityActionParam param;
    if (action == ActionType::ACCESSIBILITY_ACTION_SET_SELECTION) {
        std::string dir = STRING_DIR_BACKWARD;
        auto iter = actionArguments.find(ACTION_ARGU_SELECT_TEXT_INFORWARD);
        if (iter != actionArguments.end()) {
            dir = iter->second;
        }
        param.setSelectionStart = getArgumentByKey(actionArguments, ACTION_ARGU_SELECT_TEXT_START);
        param.setSelectionEnd = getArgumentByKey(actionArguments, ACTION_ARGU_SELECT_TEXT_END);
        param.setSelectionDir = conversionDirection(dir);
    }
    if (action == ActionType::ACCESSIBILITY_ACTION_SET_TEXT) {
        auto iter = actionArguments.find(ACTION_ARGU_SET_TEXT);
        if (iter != actionArguments.end()) {
            param.setTextArgument = iter->second;
        }
    }
    if (action == ActionType::ACCESSIBILITY_ACTION_NEXT_TEXT ||
        action == ActionType::ACCESSIBILITY_ACTION_PREVIOUS_TEXT) {
        int moveUnit = TextMoveUnit::STEP_CHARACTER;
        auto iter = actionArguments.find(ACTION_ARGU_MOVE_UNIT);
        if (iter != actionArguments.end()) {
            std::stringstream str_moveUnit;
            str_moveUnit << iter->second;
            str_moveUnit >> moveUnit;
        }
        param.moveUnit = static_cast<TextMoveUnit>(moveUnit);
    }
    if (action == ActionType::ACCESSIBILITY_ACTION_SET_CURSOR_POSITION) {
        param.setCursorIndex = getArgumentByKey(actionArguments, ACTION_ARGU_SET_OFFSET);
    }
    if ((action == ActionType::ACCESSIBILITY_ACTION_SCROLL_FORWARD) ||
        (action == ActionType::ACCESSIBILITY_ACTION_SCROLL_BACKWARD)) {
        param.scrollType = getAccessibilityScrollType(actionArguments, ACTION_ARGU_SCROLL_STUB);
    }
    if (action == ActionType::ACCESSIBILITY_ACTION_SPAN_CLICK) {
        param.spanId = getArgumentByKey(actionArguments, ACTION_ARGU_SPAN_ID);
    }
    if (action == ActionType::ACCESSIBILITY_ACTION_CUSTOM) {
        auto iter = actionArguments.find(ACTION_ARGU_CUSTOM_ACTION);
        if (iter != actionArguments.end()) {
            param.customActionName = iter->second;
        }
    }
    auto accessibiltyAction = ACTIONS.find(action);
    if (accessibiltyAction != ACTIONS.end()) {
        ActAccessibilityActionPreHandle(action, frameNode);
        param.accessibilityProperty = accessibilityProperty;
        return accessibiltyAction->second(param);
    }
    return false;
}

void JsAccessibilityManager::ClearAccessibilityFocusState()
{
    if (currentFocusNodeId_ != -1 && lastElementId_ != -1) {
        auto focusNode = lastFrameNode_.Upgrade();
        PaintAccessibilityFocusNode(focusNode, false);
    }
    lastFrameNode_.Reset();
    lastElementId_ = -1;
    currentFocusNodeId_ = -1;
}

bool JsAccessibilityManager::IsFormRender()
{
    auto pipelineContext = GetPipelineContext().Upgrade();
    CHECK_NULL_RETURN(pipelineContext, false);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipelineContext);
    CHECK_NULL_RETURN(ngPipeline, false);
    return ngPipeline->IsFormRender();
}

bool JsAccessibilityManager::ExecuteExtensionActionNG(int64_t elementId,
    const std::map<std::string, std::string>& actionArguments, int32_t action, const RefPtr<PipelineBase>& context,
    int64_t uiExtensionOffset)
{
    return ExecuteActionNG(
        elementId, actionArguments, static_cast<ActionType>(action), context, uiExtensionOffset);
}

bool JsAccessibilityManager::ExecuteActionNG(int64_t elementId,
    const std::map<std::string, std::string>& actionArguments, ActionType action, const RefPtr<PipelineBase>& context,
    int64_t uiExtensionOffset)
{
    if (GetIsIgnoreAllAction() && !(action == ActionType::ACCESSIBILITY_ACTION_FOCUS ||
                                    action == ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS)) {
        return false;
    }
    bool result = false;
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_RETURN(ngPipeline, result);
#ifdef WINDOW_SCENE_SUPPORTED
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_RETURN(uiExtensionManager, result);
    if (uiExtensionManager->IsWrapExtensionAbilityId(elementId)) {
        auto unWrapIdPair = uiExtensionManager->UnWrapExtensionAbilityId(uiExtensionOffset, elementId);
        int64_t childWrapId = unWrapIdPair.second;
        int64_t uiExtensionId = unWrapIdPair.first;
        auto rootNode = ngPipeline->GetRootElement();
        CHECK_NULL_RETURN(rootNode, result);
        auto uiExtensionNode = FindNodeFromRootByExtensionId(rootNode, uiExtensionId);
        CHECK_NULL_RETURN(uiExtensionNode, result);
        return OHOS::Ace::Framework::TransferExecuteAction(
            childWrapId, uiExtensionNode, actionArguments, action, uiExtensionOffset);
    }
#endif
    ContainerScope instance(ngPipeline->GetInstanceId());
    auto frameNode =
        NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(ngPipeline->GetRootElement(), elementId);
    if (!frameNode && elementId == lastElementId_) {
        frameNode = lastFrameNode_.Upgrade();
    }
    CHECK_NULL_RETURN(frameNode, result);
    auto enabled = frameNode->GetFocusHub() ? frameNode->GetFocusHub()->IsEnabled() : true;
    if (!enabled && action != ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS &&
        action != ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS) {
        return result;
    }
    result = ConvertActionTypeToBoolen(action, frameNode, elementId, ngPipeline, actionArguments);
    if (!result) {
        auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
        CHECK_NULL_RETURN(accessibilityProperty, false);
        result = ActAccessibilityAction(action, actionArguments, accessibilityProperty, frameNode);
    }
    return result;
}

bool JsAccessibilityManager::ConvertActionTypeToBoolen(ActionType action, RefPtr<NG::FrameNode>& frameNode,
    int64_t elementId, RefPtr<NG::PipelineContext>& context, const std::map<std::string, std::string>& actionArguments)
{
    bool result = false;
    NG::SecCompEnhanceEvent secEvent;
    switch (action) {
        case ActionType::ACCESSIBILITY_ACTION_FOCUS: {
            result = RequestFocus(frameNode);
            break;
        }
        case ActionType::ACCESSIBILITY_ACTION_CLEAR_FOCUS: {
            result = LostFocus(frameNode);
            break;
        }
        case ActionType::ACCESSIBILITY_ACTION_CLICK: {
            secEvent = GetAccessibilitySecCompEnhanceEvent(actionArguments);
            result = ActClick(frameNode, secEvent);
            break;
        }
        case ActionType::ACCESSIBILITY_ACTION_LONG_CLICK: {
            result = ActLongClick(frameNode);
            break;
        }
        case ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS: {
            SaveLast(elementId, frameNode);
            SaveCurrentFocusNodeSize(frameNode);
            AccessibilityFocusInfo focusInfo{ currentFocusNodeId_, currentFocusVirtualNodeParentId_ };
            focusInfo.isFocusMoveAction = IsFocusMoveAction(actionArguments);
            result = ActAccessibilityFocus(elementId, frameNode, context, focusInfo, false);
            currentFocusNodeId_ = focusInfo.currentFocusNodeId;
            currentFocusVirtualNodeParentId_ = focusInfo.currentFocusVirtualNodeParentId;
            if (result) {
                ClearDetachFocusFallbackCache(context->GetInstanceId());
            }
            break;
        }
        case ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS: {
            SaveLast(elementId, frameNode);
            AccessibilityFocusInfo focusInfo{ currentFocusNodeId_, currentFocusVirtualNodeParentId_ };
            result = ActAccessibilityFocus(elementId, frameNode, context, focusInfo, true);
            currentFocusNodeId_ = focusInfo.currentFocusNodeId;
            currentFocusVirtualNodeParentId_ = focusInfo.currentFocusVirtualNodeParentId;
            break;
        }
        default:
            break;
    }
    return result;
}

bool ActAccessibilityVirtualFocus(int64_t elementId, const RefPtr<NG::FrameNode>& parentFrameNode,
    const RefPtr<NG::VirtualAccessibilityNode>& virtualAccessibilityNode, bool isNeedClear)
{
    CHECK_NULL_RETURN(parentFrameNode, false);
    RefPtr<NG::RenderContext> renderContext = parentFrameNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, false);
    if (isNeedClear) {
        renderContext->UpdateAccessibilityFocus(false);
        return true;
    }

    CHECK_NULL_RETURN(virtualAccessibilityNode, false);
    renderContext->UpdateAccessibilityFocusRect(virtualAccessibilityNode->GetRect());
    renderContext->UpdateAccessibilityFocus(true, ACCESSIBILITY_FOCUS_WITHOUT_EVENT);
    return true;
}

void JsAccessibilityManager::ExecuteAction(const int64_t elementId, const ActionParam& param, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback, const int32_t windowId)
{
    auto action = param.action;
    auto actionArguments = param.actionArguments;

    if (NG::VirtualNodeContainerIdManager::IsVirtualNodeContainerId(elementId)) {
        RefPtr<NG::VirtualAccessibilityNode> virtualNode;
        RefPtr<NG::FrameNode> parentNode;
        auto find = CheckAndGetVirtualNode(elementId, virtualNode, parentNode);
        if (find && virtualNode) {
            if (action == ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS) {
                ActAccessibilityVirtualFocus(virtualNode->GetNodeId(), parentNode, virtualNode, false);
                virtualNode->SetAccessibilityFocused(true);
                SetExecuteActionResult(callback, true, requestId);
                return;
            }
            if (action == ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS) {
                ActAccessibilityVirtualFocus(virtualNode->GetNodeId(), parentNode, virtualNode, true);
                virtualNode->SetAccessibilityFocused(false);
                SetExecuteActionResult(callback, true, requestId);
                return;
            }
            SetExecuteActionResult(callback, false, requestId);
            return;
        }
    }

    bool actionResult = false;
    auto context = GetPipelineByWindowId(windowId);
    if (!context) {
        SetExecuteActionResult(callback, actionResult, requestId);
        return;
    }

    if (AceType::InstanceOf<NG::PipelineContext>(context)) {
        actionResult = ExecuteActionNG(elementId, actionArguments, action, context, NG::UI_EXTENSION_OFFSET_MAX);
    } else {
        auto node = GetAccessibilityNodeFromPage(elementId);
        if (!node) {
            SetExecuteActionResult(callback, false, requestId);
            return;
        }

        actionResult =
            AccessibilityActionEvent(action, actionArguments, node, AceType::DynamicCast<PipelineContext>(context));
    }
    SetExecuteActionResult(callback, actionResult, requestId);
    if (actionResult && AceType::InstanceOf<PipelineContext>(context)) {
        SendActionEvent(action, elementId);
    }
}

void JsAccessibilityManager::JsInteractionOperation::GetCursorPosition(const int64_t elementId,
    const int32_t requestId, AccessibilityElementOperatorCallback &callback)
{
    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID_WITH_ACTION(jsAccessibilityManager, SetCursorPositionResult(callback, 0, requestId));
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID_WITH_ACTION(context, SetCursorPositionResult(callback, 0, requestId));
    auto instanceId = context->GetInstanceId();
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), elementId, requestId, &callback, instanceId]() {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            ContainerScope scope(instanceId);
            ACE_SCOPED_TRACE("GetCursorPosition");
            jsAccessibilityManager->GetCursorPosition(elementId, requestId, callback);
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilityGetCursorPosition");
}

void JsAccessibilityManager::GetCursorPosition(
    const int64_t elementId, const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID_WITH_ACTION(context, SetCursorPositionResult(callback, 0, requestId));
    RefPtr<NG::FrameNode> node;
    auto ngPipeline = FindPipelineByElementId(splitElementId, node);
    CHECK_NULL_VOID_WITH_ACTION(ngPipeline, SetCursorPositionResult(callback, 0, requestId));
    auto instanceId = ngPipeline->GetInstanceId();
    ContainerScope scope(instanceId);

    ngPipeline->AddAfterRenderTask(
        [weak = WeakClaim(this), weakNgPipeline = WeakClaim(AceType::RawPtr(ngPipeline)),
            splitElementId, elementId, requestId, &callback]() {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID_WITH_ACTION(jsAccessibilityManager, SetCursorPositionResult(callback, 0, requestId));
            auto ngPipeline = weakNgPipeline.Upgrade();
            CHECK_NULL_VOID_WITH_ACTION(ngPipeline, SetCursorPositionResult(callback, 0, requestId));
            auto instanceId = ngPipeline->GetInstanceId();
            ContainerScope scope(instanceId);
#ifdef WINDOW_SCENE_SUPPORTED
            auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
            CHECK_NULL_VOID_WITH_ACTION(uiExtensionManager, SetCursorPositionResult(callback, 0, requestId));
            if (uiExtensionManager->IsWrapExtensionAbilityId(splitElementId)) {
                auto unWrapIdPair =
                    uiExtensionManager->UnWrapExtensionAbilityId(NG::UI_EXTENSION_OFFSET_MAX, elementId);
                int64_t uiExtensionId = unWrapIdPair.first;
                auto rootNode = ngPipeline->GetRootElement();
                CHECK_NULL_VOID_WITH_ACTION(rootNode, SetCursorPositionResult(callback, 0, requestId));
                auto uiExtensionNode = jsAccessibilityManager->FindNodeFromRootByExtensionId(rootNode, uiExtensionId);
                CHECK_NULL_VOID_WITH_ACTION(uiExtensionNode, SetCursorPositionResult(callback, 0, requestId));
                auto accessibilityProperty = uiExtensionNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
                CHECK_NULL_VOID_WITH_ACTION(accessibilityProperty, SetCursorPositionResult(callback, 0, requestId));
                auto callNumber = accessibilityProperty->ActActionGetIndex();
                SetCursorPositionResult(callback, callNumber, requestId);
                return;
            }
#endif
            auto frameNode =
                NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(
                    ngPipeline->GetRootElement(), splitElementId);
            CHECK_NULL_VOID_WITH_ACTION(frameNode, SetCursorPositionResult(callback, 0, requestId));
            auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
            CHECK_NULL_VOID_WITH_ACTION(accessibilityProperty, SetCursorPositionResult(callback, 0, requestId));
            auto callNumber = accessibilityProperty->ActActionGetIndex();
            SetCursorPositionResult(callback, callNumber, requestId);
        }
    );
    ngPipeline->RequestFrame();
}

void JsAccessibilityManager::JsInteractionOperation::ClearFocus()
{
    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler()] {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            ACE_SCOPED_TRACE("ClearCurrentFocus");
            jsAccessibilityManager->ClearCurrentFocus();
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilityClearCurrentFocus");
}

void JsAccessibilityManager::JsInteractionOperation::OutsideTouch() {}
#ifdef WEB_SUPPORTED

std::shared_ptr<NG::TransitionalNodeInfo> GetChildrenFromWebNode(
    int64_t nodeId, std::list<int64_t>& children,
    const RefPtr<NG::PipelineContext>& ngPipeline, const RefPtr<NG::WebPattern>& webPattern)
{
    std::shared_ptr<NG::TransitionalNodeInfo> node = nullptr;
    std::list<int64_t> webNodeChildren;
    if (AceApplicationInfo::GetInstance().IsAccessibilityEnabled()) {
        int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
        int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
        AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(nodeId, splitElementId, splitTreeId);
        node = webPattern->GetTransitionalNodeById(splitElementId);
        CHECK_NULL_RETURN(node, nullptr);
        auto treeId = webPattern->GetTreeId();
        for (auto& childIdConst : node->GetChildIds()) {
            int64_t mutableChildId = childIdConst;
            AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId, mutableChildId);
            webNodeChildren.emplace_back(mutableChildId);
        }
    }
    while (!webNodeChildren.empty()) {
        children.emplace_back(webNodeChildren.front());
        webNodeChildren.pop_front();
    }
    return node;
}

void JsAccessibilityManager::SearchWebElementInfoByAccessibilityId(const int64_t elementId, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback, const int32_t mode, const int32_t windowId,
    const RefPtr<NG::WebPattern>& webPattern)
{
    std::list<AccessibilityElementInfo> infos;

    auto pipeline = GetPipelineByWindowId(windowId);
    if (!pipeline) {
        callback.SetSearchElementInfoByAccessibilityIdResult(infos, requestId);
        TAG_LOGD(AceLogTag::ACE_WEB, "SetSearchElementInfoByAccessibilityIdResult, requestId: %{public}d", requestId);
        return;
    }
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    if (!ngPipeline) {
        callback.SetSearchElementInfoByAccessibilityIdResult(infos, requestId);
        TAG_LOGD(AceLogTag::ACE_WEB, "SetSearchElementInfoByAccessibilityIdResult, requestId: %{public}d", requestId);
        return;
    }

    if (!ngPipeline->GetOnFocus() && (SystemProperties::GetDeviceType() == DeviceType::TWO_IN_ONE)) {
        TAG_LOGD(AceLogTag::ACE_WEB,
            "SearchWebElementInfo GetOnFocus, elementId: %{public}" PRId64
            ", requestId: %{public}d, mode: %{public}d, windowId: %{public}d",
            elementId, requestId, mode, windowId);
        SetSearchElementInfoByAccessibilityIdResult(callback, std::move(infos), requestId, true);
        return;
    }
    if (!webPattern) {
        callback.SetSearchElementInfoByAccessibilityIdResult(infos, requestId);
        TAG_LOGD(AceLogTag::ACE_WEB, "SetSearchElementInfoByAccessibilityIdResult, requestId: %{public}d", requestId);
        return;
    }

    if (elementId == -1) {
        auto webNode = webPattern->GetHost();
        AccessibilityElementInfo webInfo;
        SetRootAccessibilityVisible(webNode, webInfo);
        if (!webInfo.GetAccessibilityVisible()) {
            TAG_LOGD(AceLogTag::ACE_WEB,
                "SearchWebElementinfo accessibility visible false, elementId: %{public}" PRId64
                ", requestId: %{public}d, mode: %{public}d, windowId: %{public}d",
                elementId, requestId, mode, windowId);
            SetSearchElementInfoByAccessibilityIdResult(callback, std::move(infos), requestId, true);
            return;
        }
    }

    SearchWebElementInfoByAccessibilityIdNG(elementId, mode, infos, ngPipeline, webPattern);
    TAG_LOGD(AceLogTag::ACE_WEB, "SearchWebElementInfo infos.size: %{public}zu",
        infos.size());
    SetSearchElementInfoByAccessibilityIdResult(callback, std::move(infos), requestId, true);
}

void JsAccessibilityManager::SearchWebElementInfoByAccessibilityIdNG(int64_t elementId, int32_t mode,
    std::list<AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context,
    const RefPtr<NG::WebPattern>& webPattern)
{
    TAG_LOGD(AceLogTag::ACE_WEB, "SearchWebElementInfoByIdNG elementId: %{public}" PRId64
        ", treeId: %{public}d, mode: %{public}d", elementId, treeId_, mode);
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);

    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);

    AccessibilityElementInfo nodeInfo;

    CHECK_NULL_VOID(webPattern);
    auto webNode = webPattern->GetHost();
    CHECK_NULL_VOID(webNode);
    CommonProperty commonProperty;
    GenerateCommonPropertyForWeb(ngPipeline, commonProperty, mainContext, webNode);

    auto node = webPattern->GetTransitionalNodeById(elementId);
    CHECK_NULL_VOID(node);
    UpdateWebAccessibilityElementInfo(node, commonProperty, nodeInfo, webPattern);
    if (elementId == -1 && IsCmpSubAM()) {
        nodeInfo.SetParent(INVALID_PARENT_ID);
    }
    NG::AccessibilityFrameNodeUtils::UpdateAccessibilityVisibleToRoot(webNode);
    nodeInfo.SetAccessibilityVisible(webPattern->GetAccessibilityVisible(elementId)
        && webNode->GetAccessibilityVisible());
    TAG_LOGD(AceLogTag::ACE_WEB, "SearchWebElementInfoByIdNG elementId: %{public}" PRId64
        ", element AccessibilityVisible: %{public}d, Web AccessibilityVisible: %{public}d",
        elementId, webPattern->GetAccessibilityVisible(elementId), webNode->GetAccessibilityVisible());
    infos.push_back(nodeInfo);
    SearchParameter param {elementId, "", mode, 0};
    UpdateWebCacheInfo(infos, elementId, commonProperty, ngPipeline, param, webPattern);
}

void JsAccessibilityManager::FindWebFocusedElementInfo(const int64_t elementId, const int32_t focusType,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback, const int32_t windowId,
    const RefPtr<NG::WebPattern>& webPattern)
{
    AccessibilityElementInfo nodeInfo;
    if (focusType != FOCUS_TYPE_INPUT && focusType != FOCUS_TYPE_ACCESSIBILITY) {
        nodeInfo.SetValidElement(false);
        SetFindFocusedElementInfoResult(callback, nodeInfo, requestId);
        return;
    }

    auto context = GetPipelineByWindowId(windowId);
    if (!context || AceType::InstanceOf<NG::PipelineContext>(context)) {
        nodeInfo.SetValidElement(false);
        SetFindFocusedElementInfoResult(callback, nodeInfo, requestId);
        return;
    }

    FindWebFocusedElementInfoNG(elementId, focusType, nodeInfo, context, webPattern);
    SetFindFocusedElementInfoResult(callback, nodeInfo, requestId);
}

void JsAccessibilityManager::FindWebFocusedElementInfoNG(int64_t elementId, int32_t focusType,
    Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
    const RefPtr<NG::WebPattern>& webPattern)
{
    CHECK_NULL_VOID(webPattern);
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto node = webPattern->GetFocusedAccessibilityNode(elementId, focusType == FOCUS_TYPE_ACCESSIBILITY);
    if (!node) {
        info.SetValidElement(false);
        return;
    }
    CHECK_NULL_VOID(node);

    auto webNode = webPattern->GetHost();
    CHECK_NULL_VOID(webNode);
    CommonProperty commonProperty;
    GenerateCommonPropertyForWeb(ngPipeline, commonProperty, mainContext, webNode);
    UpdateWebAccessibilityElementInfo(node, commonProperty, info, webPattern);
}

void JsAccessibilityManager::WebFocusMoveSearch(const int64_t elementId, const int32_t direction,
    const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t windowId,
    const RefPtr<NG::WebPattern>& webPattern)
{
    AccessibilityElementInfo nodeInfo;
    CHECK_NULL_VOID(webPattern);
    auto frameNode = webPattern->GetHost();
    CHECK_NULL_VOID(frameNode);
    auto pipeline = frameNode->GetContextRefPtr();
    CHECK_NULL_VOID(pipeline);
    uint32_t realWindowId = static_cast<uint32_t>(pipeline->GetRealHostWindowId());
    auto context = GetPipelineByWindowId(realWindowId);
    if (!context) {
        nodeInfo.SetValidElement(false);
        SetFocusMoveSearchResult(callback, nodeInfo, requestId);
        return;
    }

    bool webFound = WebFocusMoveSearchNG(elementId, direction, nodeInfo, context, webPattern);
    // Plain-API grandchild exhaustion escalation (CMP exported subtree only): the legacy reply carries a single node.
    if (!webFound && IsCmpSubAM() && direction == Accessibility::FocusMoveDirection::BACKWARD) {
        TryPlainWebExhaustionEscalation(direction, requestId, frameNode, context, nodeInfo);
    }
    auto result = BuildWebFocusMoveResult(FocusMoveResultType::SEARCH_SUCCESS);
    WebFocusMoveSearchByComponent(nodeInfo, webPattern, direction, context, result);
    TAG_LOGD(AceLogTag::ACE_WEB,
        "JsAccessibilityManager WebFocusMoveSearch AccessibilityId: %{public}" PRId64
        ", ComponentType: %{public}s, ParentNodeId: %{public}" PRId64
        ", TreeId: %{public}d, WindowId: %{public}d, ParentWindowId: %{public}d",
        nodeInfo.GetAccessibilityId(),
        nodeInfo.GetComponentType().c_str(),
        nodeInfo.GetParentNodeId(),
        nodeInfo.GetBelongTreeId(),
        nodeInfo.GetWindowId(),
        nodeInfo.GetParentWindowId());
    SetFocusMoveSearchResult(callback, nodeInfo, requestId);
}

void JsAccessibilityManager::WebEmbedFallbackResearch(AccessibilityElementInfo& nodeInfo,
    const RefPtr<NG::WebPattern>& webPattern, const int32_t direction, RefPtr<PipelineBase> context,
    FocusMoveResult& result, int32_t focusRuleType, uint32_t searchDepth)
{
    result = WebFocusMoveSearchWithConditionNG(
        nodeInfo.GetAccessibilityId(), direction, focusRuleType, nodeInfo, context, webPattern);
    WebFocusMoveSearchByComponent(
        nodeInfo, webPattern, direction, context, result, focusRuleType, searchDepth + 1);
}

void JsAccessibilityManager::WebFocusMoveSearchByComponent(AccessibilityElementInfo& nodeInfo,
    const RefPtr<NG::WebPattern>& webPattern, const int32_t direction, RefPtr<PipelineBase> context,
    FocusMoveResult& result, int32_t focusRuleType, uint32_t searchDepth)
{
    if (!IsTagInEmbedComponent(nodeInfo.GetComponentType())) {
        return;
    }
    int64_t accessibilityId = nodeInfo.GetAccessibilityId();
    result = BuildWebFocusMoveResult(FocusMoveResultType::SEARCH_FAIL);
    if (searchDepth >= WEB_FOCUS_MOVE_SEARCH_MAX_DEPTH) {
        TAG_LOGW(AceLogTag::ACE_WEB,
            "WebFocusMoveSearchByComponent reach max depth, accessibilityId: %{public}" PRId64
            ", direction: %{public}d, focusRuleType: %{public}d",
            accessibilityId, direction, focusRuleType);
        return;
    }
    CHECK_NULL_VOID(webPattern);
    std::shared_ptr<NG::TransitionalNodeInfo> transitionalNodeInfo =
        webPattern->GetTransitionalNodeById(accessibilityId);
    if (transitionalNodeInfo == nullptr) {
        TAG_LOGW(AceLogTag::ACE_WEB,
            "WebEmbedHandoff drop: transitional node not found, id=%{public}" PRId64, accessibilityId);
        return;
    }
    std::string surfaceId = webPattern->GetSurfaceIdByHtmlElementId(transitionalNodeInfo->GetHtmlElementId());
    if (surfaceId == "") {
        WebEmbedFallbackResearch(
            nodeInfo, webPattern, direction, context, result, focusRuleType, searchDepth);
        return;
    }
    std::list<AccessibilityElementInfo> embedNodeTreeInfo;
    int32_t windowId = nodeInfo.GetWindowId();
    SearchSurfaceIdRet searchSurfaceIdRet = SearchSurfaceIdRet::NO_MATCH_NODE;
    SearchSurfaceIdType searchType = SearchSurfaceIdType::SEARCH_ALL;
    if (GetWebComponentSearchSurfaceIdType(direction, focusRuleType, searchType)) {
        searchSurfaceIdRet = SearchElementInfoBySurfaceId(surfaceId, windowId, searchType, embedNodeTreeInfo);
    }
    if (searchSurfaceIdRet != SearchSurfaceIdRet::SEARCH_SUCCESS || embedNodeTreeInfo.empty()) {
        WebEmbedFallbackResearch(
            nodeInfo, webPattern, direction, context, result, focusRuleType, searchDepth);
    } else {
        nodeInfo = embedNodeTreeInfo.front();
        result = BuildWebFocusMoveResult(FocusMoveResultType::SEARCH_SUCCESS);
    }
}

bool JsAccessibilityManager::WebFocusMoveSearchNG(int64_t elementId, int32_t direction,
    AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
    const RefPtr<NG::WebPattern>& webPattern)
{
    auto mainContext = context_.Upgrade();
    CHECK_NULL_RETURN(mainContext, false);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_RETURN(ngPipeline, false);

    CHECK_NULL_RETURN(webPattern, false);
    auto webNode = webPattern->GetHost();
    CHECK_NULL_RETURN(webNode, false);
    CommonProperty commonProperty;
    GenerateCommonPropertyForWeb(ngPipeline, commonProperty, mainContext, webNode);

    auto node = webPattern->GetAccessibilityNodeByFocusMove(elementId, direction);
    if (node) {
        UpdateWebAccessibilityElementInfo(node, commonProperty, info, webPattern);
        return true;
    }
    // Exhausted in the search direction: info keeps the host fallback (legacy reply form, no envelope).
    int64_t webId = webNode->GetAccessibilityId();
    int32_t mode = 0;
    std::list<AccessibilityElementInfo> infos;
    SearchElementInfoByAccessibilityIdNG(webId, mode, infos, context, NG::UI_EXTENSION_OFFSET_MAX);
    TAG_LOGD(AceLogTag::ACE_WEB,
        "JsAccessibilityManager WebFocusMoveSearchNG infos.size: %{public}zu, webId:  %{public}" PRId64,
        infos.size(),
        webId);
    if (!infos.empty()) {
        info = infos.front();
    } else {
        UpdateAccessibilityElementInfo(webNode, commonProperty, info, ngPipeline);
    }
    return false;
}

void JsAccessibilityManager::WebFocusMoveSearchWithCondition(const int64_t elementId,
    const Accessibility::AccessibilityFocusMoveParam& param, const int32_t requestId,
    Accessibility::AccessibilityElementOperatorCallback& callback, const RefPtr<NG::WebPattern>& webPattern)
{
    AccessibilityElementInfo nodeInfo;
    CHECK_NULL_VOID(webPattern);
    auto frameNode = webPattern->GetHost();
    CHECK_NULL_VOID(frameNode);
    auto pipeline = frameNode->GetContextRefPtr();
    CHECK_NULL_VOID(pipeline);
    uint32_t realWindowId = static_cast<uint32_t>(pipeline->GetRealHostWindowId());
    auto context = GetPipelineByWindowId(realWindowId);
    if (!context) {
        SetWebFocusMoveSearchWithConditionFailResult(callback, FocusMoveResultType::SEARCH_FAIL, requestId);
        return;
    }

    if (!IsWebFocusRuleTypeSupported(param.type)) {
        SetWebFocusMoveSearchWithConditionFailResult(callback, FocusMoveResultType::NOT_SUPPORT, requestId);
        return;
    }

    auto result = WebFocusMoveSearchWithConditionNG(
        elementId, param.direction, static_cast<int32_t>(param.type), nodeInfo, context, webPattern);
    WebFocusMoveSearchByComponent(
        nodeInfo, webPattern, param.direction, context, result, static_cast<int32_t>(param.type));
    TAG_LOGD(AceLogTag::ACE_WEB,
        "JsAccessibilityManager WebFocusMoveSearchWithCondition AccessibilityId: %{public}" PRId64
        ", ComponentType: %{public}s, ParentNodeId: %{public}" PRId64
        ", TreeId: %{public}d, WindowId: %{public}d, ParentWindowId: %{public}d"
        ", requestId: %{public}d, focusRuleType: %{public}d",
        nodeInfo.GetAccessibilityId(),
        nodeInfo.GetComponentType().c_str(),
        nodeInfo.GetParentNodeId(),
        nodeInfo.GetBelongTreeId(),
        nodeInfo.GetWindowId(),
        nodeInfo.GetParentWindowId(),
        requestId,
        static_cast<int32_t>(param.type));
    if (IsRegister()) {
        UpdateElementInfoTreeId(nodeInfo);
        std::list<AccessibilityElementInfo> infos = { nodeInfo };
        // Grandchild exhaustion continuation (CMP exported subtree): this Web tree is itself a child tree registered.
        int32_t webTreeId = webPattern->GetTreeId();
        if (result.resultType == Accessibility::FocusMoveResultType::SEARCH_FAIL
            && param.direction != Accessibility::FocusMoveDirection::FIND_LAST
            && nodeInfo.IsValidElement() && webTreeId > 0) {
            if (IsCmpSubAM()) {
                result.resultType = Accessibility::FocusMoveResultType::SEARCH_FAIL_IN_CHILDTREE;
                result.nowLevelBelongTreeId = webTreeId;
                result.needTerminate = false;
                TAG_LOGI(AceLogTag::ACE_WEB,
                    "Web grandchild exhaustion continuation: webTreeId=%{public}d, parentTreeId=%{public}d"
                    ", direction=%{public}d, anchorId=%{public}" PRId64 ", requestId=%{public}d",
                    webTreeId, treeId_, static_cast<int32_t>(param.direction),
                    nodeInfo.GetAccessibilityId(), requestId);
            } else {
                // Expect a subAM here (the Web host sits inside a bound exported subtree).
                auto hostNode = webPattern->GetHost();
                if (hostNode
                    && CmpAccessibility::CmpAccessibilityRegistry::HasSubAM(hostNode.GetRawPtr())) {
                    TAG_LOGW(AceLogTag::ACE_WEB,
                        "Web grandchild exhaustion continuation suppressed: handler is not the "
                        "binding subAM (stale main-AM registration?), webTreeId=%{public}d"
                        ", handlerTreeId=%{public}d, requestId=%{public}d",
                        webTreeId, treeId_, requestId);
                }
            }
        }
        callback.SetFocusMoveSearchWithConditionResult(infos, result, requestId);
    }
}

FocusMoveResult JsAccessibilityManager::WebFocusMoveSearchWithConditionNG(int64_t elementId,
    int32_t direction, int32_t focusRuleType, Accessibility::AccessibilityElementInfo& info,
    const RefPtr<PipelineBase>& context, const RefPtr<NG::WebPattern>& webPattern)
{
    auto mainContext = context_.Upgrade();
    CHECK_NULL_RETURN(mainContext, BuildWebFocusMoveResult(FocusMoveResultType::SEARCH_FAIL));
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_RETURN(ngPipeline, BuildWebFocusMoveResult(FocusMoveResultType::SEARCH_FAIL));
    CHECK_NULL_RETURN(webPattern, BuildWebFocusMoveResult(FocusMoveResultType::SEARCH_FAIL));
    auto webNode = webPattern->GetHost();
    CHECK_NULL_RETURN(webNode, BuildWebFocusMoveResult(FocusMoveResultType::SEARCH_FAIL));
    CommonProperty commonProperty;
    GenerateCommonPropertyForWeb(ngPipeline, commonProperty, mainContext, webNode);

    auto defaultNode = webPattern->GetAccessibilityNodeByFocusMove(elementId, direction);
    if (defaultNode && IsTagInEmbedComponent(defaultNode->GetComponentType())) {
        UpdateWebAccessibilityElementInfo(defaultNode, commonProperty, info, webPattern);
        TAG_LOGD(AceLogTag::ACE_WEB,
            "WebFocusMoveSearchWithConditionNG default focus hit embed, accessibilityId: %{public}" PRId64
            ", direction: %{public}d, focusRuleType: %{public}d, componentType: %{public}s",
            info.GetAccessibilityId(), direction, focusRuleType, info.GetComponentType().c_str());
        return BuildWebFocusMoveResult(FocusMoveResultType::SEARCH_SUCCESS);
    }

    std::map<std::string, std::string> params;
    auto node = webPattern->GetAccessibilityNodeByParams(elementId, direction, focusRuleType, params);
    if (node) {
        UpdateWebAccessibilityElementInfo(node, commonProperty, info, webPattern);
        TAG_LOGD(AceLogTag::ACE_WEB,
            "WebFocusMoveSearchWithConditionNG params focus hit node, accessibilityId: %{public}" PRId64
            ", direction: %{public}d, focusRuleType: %{public}d, componentType: %{public}s",
            info.GetAccessibilityId(), direction, focusRuleType, info.GetComponentType().c_str());
        return BuildWebFocusMoveResult(FocusMoveResultType::SEARCH_SUCCESS);
    }

    int64_t webId = webNode->GetAccessibilityId();
    int32_t mode = 0;
    std::list<AccessibilityElementInfo> infos;
    SearchElementInfoByAccessibilityIdNG(webId, mode, infos, context, NG::UI_EXTENSION_OFFSET_MAX);
    TAG_LOGD(AceLogTag::ACE_WEB,
        "JsAccessibilityManager WebFocusMoveSearchWithConditionNG infos.size: %{public}zu, webId:  %{public}" PRId64,
        infos.size(), webId);
    if (!infos.empty()) {
        info = infos.front();
    } else {
        UpdateAccessibilityElementInfo(webNode, commonProperty, info, ngPipeline);
    }
    return BuildWebFocusMoveResult(FocusMoveResultType::SEARCH_FAIL);
}

bool JsAccessibilityManager::ExecuteWebActionNG(int64_t elementId, ActionType action,
    const std::map<std::string, std::string>& actionArguments, const RefPtr<NG::WebPattern>& webPattern)
{
    CHECK_NULL_RETURN(webPattern, false);
    return webPattern->ExecuteAction(elementId, ConvertAccessibilityAction(action), actionArguments);
}

void JsAccessibilityManager::ExecuteWebAction(const int64_t elementId, const ActionParam& param,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback, const int32_t windowId,
    const RefPtr<NG::WebPattern>& webPattern)
{
    auto action = param.action;
    auto actionArguments = param.actionArguments;

    bool actionResult = false;
    auto context = GetPipelineByWindowId(windowId);
    if (!context || !AceType::InstanceOf<NG::PipelineContext>(context)) {
        SetExecuteActionResult(callback, actionResult, requestId);
        return;
    }

    actionResult = ExecuteWebActionNG(elementId, action, actionArguments, webPattern);
    if (action == ActionType::ACCESSIBILITY_ACTION_CLICK && webPattern) {
        auto node = webPattern->GetTransitionalNodeById(elementId);
        if (node && !node->GetIsClickable()) {
            TAG_LOGD(AceLogTag::ACE_WEB,
                "ExecuteWebAction click action result is false because node is not clickable, "
                "elementId: %{public}" PRId64 ", requestId: %{public}d",
                elementId, requestId);
            actionResult = false;
        }
    }
    SetExecuteActionResult(callback, actionResult, requestId);
}

bool JsAccessibilityManager::RegisterWebInteractionOperationAsChildTree(int64_t accessibilityId,
    const WeakPtr<NG::WebPattern>& webPattern)
{
    std::shared_ptr<AccessibilitySystemAbilityClient> instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(instance, false);
    auto pipelineContext = GetPipelineContext().Upgrade();
    CHECK_NULL_RETURN(pipelineContext, false);
    auto container = Platform::AceContainer::GetContainer(pipelineContext->GetInstanceId());
    if (container != nullptr && container->IsUIExtensionWindow()) {
        windowId_ = pipelineContext->GetRealHostWindowId();
    }

    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId_, accessibilityId);

    auto pattern = webPattern.Upgrade();
    CHECK_NULL_RETURN(pattern, false);
    auto frameNode = pattern->GetHost();
    CHECK_NULL_RETURN(frameNode, false);
    auto pipeline = frameNode->GetContextRefPtr();
    CHECK_NULL_RETURN(pipeline, false);
    uint32_t windowId = static_cast<uint32_t>(pipeline->GetRealHostWindowId());
    uint32_t parentWindowId = windowId;
    auto interactionOperation = std::make_shared<WebInteractionOperation>(windowId);
    interactionOperation->SetHandler(WeakClaim(this));
    interactionOperation->SetWebPattern(webPattern);

    Accessibility::Registration registration {
        .windowId = static_cast<int32_t>(windowId),
        .parentWindowId = static_cast<int32_t>(parentWindowId),
        .parentTreeId = treeId_,
        .elementId = accessibilityId,
    };
    parentWebWindowId_ = parentWindowId;
    TAG_LOGI(AceLogTag::ACE_WEB, "windowId: %{public}u, parentWindowId: %{public}u, "
        "parentTreeId: %{public}d, elementId %{public}" PRId64,
        windowId, parentWindowId, treeId_, accessibilityId);
    Accessibility::RetError retReg = instance->RegisterElementOperator(registration, interactionOperation);
    TAG_LOGI(AceLogTag::ACE_WEB,
        "RegisterWebInteractionOperationAsChildTree result: %{public}d, accessibilityId: %{public}" PRId64, retReg,
        accessibilityId);
    return retReg == RET_OK;
}

bool JsAccessibilityManager::DeregisterWebInteractionOperationAsChildTree(int32_t treeId,
    const WeakPtr<NG::WebPattern>& webPattern)
{
    std::shared_ptr<AccessibilitySystemAbilityClient> instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(instance, false);
    auto pattern = webPattern.Upgrade();
    CHECK_NULL_RETURN(pattern, false);
    auto frameNode = pattern->GetHost();
    CHECK_NULL_RETURN(frameNode, false);
    auto pipeline = frameNode->GetContextRefPtr();
    CHECK_NULL_RETURN(pipeline, false);
    uint32_t windowId = static_cast<uint32_t>(pipeline->GetRealHostWindowId());
    Accessibility::RetError retReg = instance->DeregisterElementOperator(windowId, treeId);
    TAG_LOGI(AceLogTag::ACE_WEB,
        "DeregisterWebInteractionOperationAsChildTree result: %{public}d, accessibilityId: %{public}" PRId64, retReg,
        frameNode->GetAccessibilityId());
    return retReg == RET_OK;
}

void JsAccessibilityManager::UpdateWebCacheInfo(std::list<AccessibilityElementInfo>& infos,
    int64_t nodeId, const CommonProperty& commonProperty, const RefPtr<NG::PipelineContext>& ngPipeline,
    const SearchParameter& searchParam, const RefPtr<NG::WebPattern>& webPattern)
{
    uint32_t umode = searchParam.mode;
    std::unordered_map<int64_t, AccessibilityElementInfo> allEmbedNodeTreeInfosMap;
    std::list<int64_t> children;
    // get all children
    if (!(umode & static_cast<uint32_t>(PREFETCH_RECURSIVE_CHILDREN))) {
        return;
    }
    GetChildrenFromWebNode(nodeId, children, ngPipeline, webPattern);
    // The application needs to insert node information in the order of BFS
    while (!children.empty()) {
        int64_t parent = children.front();
        children.pop_front();
        AccessibilityElementInfo nodeInfo;
        if (GetChildrenFromEmbedNode(nodeInfo, parent, children, allEmbedNodeTreeInfosMap)) {
            PushElementsIntoInfos(nodeInfo, infos, webPattern, children, allEmbedNodeTreeInfosMap);
            continue;
        }

        auto node = GetChildrenFromWebNode(parent, children, ngPipeline, webPattern);
        if (node) {
            UpdateWebAccessibilityElementInfo(node, commonProperty, nodeInfo, webPattern);
            PushElementsIntoInfos(nodeInfo, infos, webPattern, children, allEmbedNodeTreeInfosMap);
        }
    }
}

bool JsAccessibilityManager::GetChildrenFromEmbedNode(AccessibilityElementInfo& nodeInfo, int64_t nodeId,
    std::list<int64_t>& children, std::unordered_map<int64_t, AccessibilityElementInfo>& allEmbedNodeTreeInfosMap)
{
    auto it = allEmbedNodeTreeInfosMap.find(nodeId);
    if (it == allEmbedNodeTreeInfosMap.end()) {
        return false;
    }
    nodeInfo = it->second;
    allEmbedNodeTreeInfosMap.erase(it);

    for (auto& childId : nodeInfo.GetChildIds()) {
        children.emplace_back(childId);
    }
    return true;
}

void JsAccessibilityManager::PushElementsIntoInfos(AccessibilityElementInfo& nodeInfo,
    std::list<AccessibilityElementInfo>& infos, const RefPtr<NG::WebPattern>& webPattern, std::list<int64_t>& children,
    std::unordered_map<int64_t, AccessibilityElementInfo>& allEmbedNodeTreeInfosMap)
{
    if (!IsTagInEmbedComponent(nodeInfo.GetComponentType())) {
        infos.push_back(nodeInfo);
        return;
    }
    int64_t accessibilityId = nodeInfo.GetAccessibilityId();
    CHECK_NULL_VOID(webPattern);
    std::shared_ptr<NG::TransitionalNodeInfo> node = webPattern->GetTransitionalNodeById(accessibilityId);
    CHECK_NULL_VOID(node);
    std::string surfaceId = webPattern->GetSurfaceIdByHtmlElementId(node->GetHtmlElementId());
    if (surfaceId == "") {
        infos.push_back(nodeInfo);
        return;
    }
    std::list<AccessibilityElementInfo> embedNodeTreeInfo;
    int32_t windowId = nodeInfo.GetWindowId();
    SearchSurfaceIdRet searchSurfaceIdRet =
        SearchElementInfoBySurfaceId(surfaceId, windowId, SearchSurfaceIdType::SEARCH_ALL, embedNodeTreeInfo);
    if (searchSurfaceIdRet != SearchSurfaceIdRet::SEARCH_SUCCESS) {
        infos.push_back(nodeInfo);
        return;
    }
    AccessibilityElementInfo& root = embedNodeTreeInfo.front();
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(webPattern->GetTreeId(), accessibilityId);
    root.SetParent(accessibilityId);
    nodeInfo.AddChild(root.GetAccessibilityId());
    infos.push_back(nodeInfo);
    children.emplace_back(root.GetAccessibilityId());
    for (const auto& info : embedNodeTreeInfo) {
        allEmbedNodeTreeInfosMap[info.GetAccessibilityId()] = info;
    }
}

int64_t JsAccessibilityManager::GetWebAccessibilityIdBySurfaceId(const std::string& surfaceId)
{
    WeakPtr<NG::WebPattern> weakWebPattern = GetWebPatternBySurfaceIdWithMainFallback(surfaceId);
    if (weakWebPattern.Invalid()) {
        TAG_LOGI(
            AceLogTag::ACE_WEB, "JsAccessibilityManager GetWebAccessibilityIdBySurfaceId weakWebPattern is Invalid");
        return INVALID_NODE_ID;
    }
    auto webPattern = weakWebPattern.Upgrade();
    CHECK_NULL_RETURN(webPattern, INVALID_NODE_ID);
    int64_t webAccessibilityId = webPattern->GetWebAccessibilityIdBySurfaceId(surfaceId);
    if (webAccessibilityId == INVALID_NODE_ID) {
        TAG_LOGD(AceLogTag::ACE_WEB, "JsAccessibilityManager GetWebAccessibilityIdBySurfaceId node is Invalid");
        return INVALID_NODE_ID;
    }
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(webPattern->GetTreeId(), webAccessibilityId);
    TAG_LOGD(AceLogTag::ACE_WEB,
        "JsAccessibilityManager GetWebAccessibilityIdBySurfaceId return webAccessibilityId: %{public}" PRId64,
        webAccessibilityId);
    return webAccessibilityId;
}

bool JsAccessibilityManager::GetWebAccessibilityInfoBySurfaceId(
    const std::string& surfaceId,
    AccessibilityElementInfo& elementInfo)
{
    WeakPtr<NG::WebPattern> weakWebPattern = GetWebPatternBySurfaceIdWithMainFallback(surfaceId);
    CHECK_EQUAL_RETURN(weakWebPattern.Invalid(), true, false);
    auto webPattern = weakWebPattern.Upgrade();
    CHECK_NULL_RETURN(webPattern, false);
    auto webNode = webPattern->GetHost();
    CHECK_NULL_RETURN(webNode, false);
    auto context = webNode->GetContextRefPtr();
    CHECK_NULL_RETURN(context, false);
    auto webAccessibilityId = webPattern->GetWebAccessibilityIdBySurfaceId(surfaceId);
    CHECK_EQUAL_RETURN(webAccessibilityId, INVALID_NODE_ID, false);
    std::list<AccessibilityElementInfo> elementInfos;
    SearchWebElementInfoByAccessibilityIdNG(webAccessibilityId, 0, elementInfos, context, webPattern);
    CHECK_EQUAL_RETURN(elementInfos.empty(), true, false);
    elementInfo = elementInfos.front();
    UpdateElementInfoTreeId(elementInfo);
    return true;
}
#endif //WEB_SUPPORTED

// Map lookup only (no WebPattern calls) — compiled unconditionally for the no-web unittest target.
WeakPtr<NG::WebPattern> JsAccessibilityManager::GetWebPatternBySurfaceIdWithMainFallback(const std::string& surfaceId)
{
    auto pattern = GetWebPatternBySurfaceId(surfaceId);
    if (!pattern.Invalid()) {
        return pattern;
    }
    // CMP exported-subtree parity: the surfaceId→webPattern entries are written by.
    if (!IsCmpSubAM()) {
        return pattern;
    }
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, pattern);
    auto mainAM = AceType::DynamicCast<JsAccessibilityManager>(context->GetAccessibilityManager());
    if (mainAM && mainAM.GetRawPtr() != this) {
        return mainAM->GetWebPatternBySurfaceId(surfaceId);
    }
    return pattern;
}

bool JsAccessibilityManager::RegisterInteractionOperationAsChildTree(
    const Registration& registration)
{
    bool ret = false;
    switch (registration.operatorType) {
        case OperatorType::JS_THIRD_PROVIDER:
            ret = RegisterThirdProviderInteractionOperationAsChildTree(registration);
            break;
        default:
            TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
                "RegisterInteractionOperationAsChildTree operatorType: %{public}d",
                static_cast<int32_t>(registration.operatorType));
    }
    return ret;
}

bool JsAccessibilityManager::RegisterThirdProviderInteractionOperationAsChildTree(
    const Registration& registration)
{
    std::shared_ptr<AccessibilitySystemAbilityClient> instance =
        AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(instance, false);
    // AAMS-facing mount id: spliced with parentTreeId so SetRootParentId stores a routable id.
    int64_t hostElementId = registration.elementId;
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(registration.parentTreeId, hostElementId);
    Accessibility::Registration innerRegistration {
        .windowId = static_cast<int32_t>(registration.windowId),
        .parentWindowId = static_cast<int32_t>(registration.parentWindowId),
        .parentTreeId = registration.parentTreeId,
        .elementId = hostElementId,
    };

    auto provider = registration.accessibilityProvider.Upgrade();
    CHECK_NULL_RETURN(provider, false);
    auto interactionOperation = std::make_shared<JsThirdProviderInteractionOperation>(
        registration.accessibilityProvider, WeakClaim(this), registration.hostNode);
    provider->SendThirdAccessibilityProvider(interactionOperation);
    interactionOperation->Initialize();
    RegisterJsThirdProviderInteractionOperation(registration.elementId,
        interactionOperation);
    Accessibility::RetError retReg = instance->RegisterElementOperator(
        innerRegistration, interactionOperation);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "RegisterWebInteractionOperationAsChildTree result: %{public}d", retReg);
    return retReg == RET_OK;
}

bool JsAccessibilityManager::DeregisterInteractionOperationAsChildTree(
    uint32_t windowId, int32_t treeId)
{
    std::shared_ptr<AccessibilitySystemAbilityClient> instance =
        AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(instance, false);
    Accessibility::RetError retReg = instance->DeregisterElementOperator(windowId, treeId);
    return retReg == RET_OK;
}

int JsAccessibilityManager::RegisterInteractionOperation(int windowId)
{
    if (IsRegister()) {
        return 0;
    }

    uint64_t displayId = GetDisplayId(windowId, context_.Upgrade());
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
        "RegisterInteractionOperation windowId: %{public}u, displayId: %{public}" PRId64, windowId, displayId);
    std::shared_ptr<AccessibilitySystemAbilityClient> instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(instance, -1);
    auto interactionOperation = std::make_shared<JsInteractionOperation>(windowId);
    interactionOperation->SetHandler(WeakClaim(this));
    Accessibility::RetError retReg = instance->RegisterElementOperator(windowId, interactionOperation, displayId);
    if (retReg != RET_OK) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "RegisterInteractionOperation failed, "
            "windowId: %{public}u, displayId: %{public}" PRId64 ", result: %{public}d", windowId, displayId, retReg);
    }
    RefPtr<PipelineBase> context;
    for (const auto& subContext : GetSubPipelineContexts()) {
        context = subContext.Upgrade();
        CHECK_NULL_CONTINUE(context);
        TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "RegisterSubPipeline windowId: %{public}u", context->GetWindowId());
        interactionOperation = std::make_shared<JsInteractionOperation>(context->GetWindowId());
        interactionOperation->SetHandler(WeakClaim(this));
        auto retResult = instance->RegisterElementOperator(context->GetWindowId(), interactionOperation);
        retReg = retResult == RET_OK ? retReg : retResult;
    }
    Register(retReg == RET_OK);
    if (retReg == RET_OK) {
        NotifyChildTreeOnRegister(treeId_);
    }

    return retReg;
}

void JsAccessibilityManager::RegisterSubWindowInteractionOperation(int windowId)
{
    if (!AceApplicationInfo::GetInstance().IsAccessibilityEnabled() || !IsRegister()) {
        return;
    }

    std::shared_ptr<AccessibilitySystemAbilityClient> instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_VOID(instance);
    auto interactionOperation = std::make_shared<JsInteractionOperation>(windowId);
    interactionOperation->SetHandler(WeakClaim(this));
    uint64_t displayId = GetDisplayId(windowId, context_.Upgrade());
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
        "RegisterSubWindowInteractionOperation windowId: %{public}u, displayId: %{public}" PRId64, windowId, displayId);
    Accessibility::RetError retReg = instance->RegisterElementOperator(windowId, interactionOperation, displayId);
    if (retReg != RET_OK) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "RegisterSubWindowInteractionOperation failed, "
            "windowId: %{public}u, displayId: %{public}" PRId64 ", result: %{public}d", windowId, displayId, retReg);
    }
}

void JsAccessibilityManager::DeregisterInteractionOperation()
{
    CHECK_NE_VOID(IsRegister(), true);
    int windowId = static_cast<int>(GetWindowId());

    auto instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_VOID(instance);
    Register(false);
    ClearAccessibilityFocusState();
    uint64_t displayId = GetDisplayId(windowId, context_.Upgrade());
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "DeregisterInteractionOperation "
        "windowId: %{public}u, displayId: %{public}" PRId64 ", parentWindowId: %{public}u, treeId: %{public}d",
        windowId, displayId, parentWindowId_, treeId_);
    Accessibility::RetError retDer;
    if (parentWindowId_ == 0) {
        retDer = instance->DeregisterElementOperator(windowId, displayId);
    } else {
        retDer = instance->DeregisterElementOperator(windowId, treeId_);
        parentElementId_ = INVALID_PARENT_ID;
        parentTreeId_ = 0;
        parentWindowId_ = 0;
    }
    if (retDer != RET_OK) {
        TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "DeregisterInteractionOperation failed, "
            "windowId: %{public}u, displayId: %{public}" PRId64 ", result: %{public}d", windowId, displayId, retDer);
    }

    RefPtr<PipelineBase> context;
    for (const auto& subContext : GetSubPipelineContexts()) {
        context = subContext.Upgrade();
        CHECK_NULL_CONTINUE(context);
        auto subWindowId = context->GetWindowId();
        uint64_t subDisplayId = GetDisplayId(subWindowId, context);
        retDer = instance->DeregisterElementOperator(subWindowId, subDisplayId);
        if (retDer != RET_OK) {
            TAG_LOGE(AceLogTag::ACE_ACCESSIBILITY, "DeregisterSubPipeline failed, subWindowId: %{public}u, "
                "subDisplayId: %{public}" PRId64 ", result: %{public}d", subWindowId, subDisplayId, retDer);
        }
    }
    NotifyChildTreeOnDeregister();

    CHECK_EQUAL_VOID(currentFocusVirtualNodeParentId_, -1);
    auto pipelineContext = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(pipelineContext);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipelineContext);
    CHECK_NULL_VOID(ngPipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    ClearVirtualNodeAccessibilityFocus(rootNode, currentFocusVirtualNodeParentId_);
}

void JsAccessibilityManager::RegisterAccessibilityChildTreeCallback(
    int64_t elementId, const std::shared_ptr<AccessibilityChildTreeCallback> &callback)
{
    std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
    childTreeCallbackMap_[elementId] = callback;
    detachedCmpChildTreeCallbacks_.erase(elementId);
}

void JsAccessibilityManager::DeregisterAccessibilityChildTreeCallback(int64_t elementId)
{
    std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
    childTreeCallbackMap_.erase(elementId);
    detachedCmpChildTreeCallbacks_.erase(elementId);
}

void JsAccessibilityManager::RegisterAccessibilitySAObserverCallback(
    int64_t elementId, const std::shared_ptr<AccessibilitySAObserverCallback> &callback)
{
    std::lock_guard<std::mutex> lock(componentSACallbackMutex_);
    componentSACallbackMap_[elementId] = callback;
}

void JsAccessibilityManager::DeregisterAccessibilitySAObserverCallback(int64_t elementId)
{
    std::lock_guard<std::mutex> lock(componentSACallbackMutex_);
    componentSACallbackMap_.erase(elementId);
}

void JsAccessibilityManager::RegisterScreenReaderObserverCallback(
    int64_t elementId, const std::shared_ptr<AccessibilityScreenReaderObserverCallback>& callback)
{
    std::lock_guard<std::mutex> lock(componentScreenReaderCallbackMutex_);
    componentScreenReaderCallbackMap_[elementId] = callback;
}

void JsAccessibilityManager::DeregisterScreenReaderObserverCallback(int64_t elementId)
{
    std::lock_guard<std::mutex> lock(componentScreenReaderCallbackMutex_);
    componentScreenReaderCallbackMap_.erase(elementId);
}

void JsAccessibilityManager::NotifyAccessibilitySAStateChange(bool state)
{
    std::lock_guard<std::mutex> lock(componentSACallbackMutex_);
    for (auto &item : componentSACallbackMap_) {
        if (item.second == nullptr) {
            continue;
        }
        item.second->OnState(state);
    }
}

void JsAccessibilityManager::NotifyChildTreeOnRegister(int32_t treeId)
{
    std::unordered_map<int64_t, std::shared_ptr<AccessibilityChildTreeCallback>> callbacks;
    {
        std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
        callbacks = childTreeCallbackMap_;
    }
    for (const auto& [elementId, callback] : callbacks) {
        if (callback) {
            callback->OnRegister(GetWindowId(), treeId);
        }
    }
}

void JsAccessibilityManager::NotifyChildTreeOnDeregister()
{
    std::unordered_map<int64_t, std::shared_ptr<AccessibilityChildTreeCallback>> callbacks;
    {
        std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
        callbacks = childTreeCallbackMap_;
    }
    for (const auto& [elementId, callback] : callbacks) {
        if (callback) {
            callback->OnDeregister();
        }
    }
}

void JsAccessibilityManager::NotifySetChildTreeIdAndWinId(
    int64_t elementId, const int32_t treeId, const int32_t childWindowId)
{
    std::shared_ptr<AccessibilityChildTreeCallback> callback;
    {
        std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
        auto it = childTreeCallbackMap_.find(elementId);
        if (it != childTreeCallbackMap_.end()) {
            callback = it->second;
        }
    }
    if (callback) {
        callback->SetChildTreeId(treeId);
        callback->OnSetChildTree(childWindowId, treeId);
        return;
    }
    if (CmpAccessibility::CmpAccessibilityRegistry::HasAnySubAM()) {
        RefPtr<NG::FrameNode> node;
        auto subAM = AceType::DynamicCast<JsAccessibilityManager>(
            CmpAccessibility::CmpAccessibilityRegistry::FindSubAMAndNodeInExportedSubtrees(elementId, node));
        if (subAM && subAM.GetRawPtr() != this) {
            std::lock_guard<std::mutex> lock(subAM->childTreeCallbackMapMutex_);
            auto it = subAM->childTreeCallbackMap_.find(elementId);
            if (it != subAM->childTreeCallbackMap_.end() && it->second &&
                it->second->GetChildTreeId() == treeId) {
                return;
            }
        }
    }
    if (treeId > 0) {
        auto instance = AccessibilitySystemAbilityClient::GetInstance();
        if (instance) {
            instance->DeregisterElementOperator(childWindowId, treeId);
        }
    }
}

void JsAccessibilityManager::NotifyScreenReaderObserverStateChange(bool state)
{
    std::unordered_map<int64_t, std::shared_ptr<AccessibilityScreenReaderObserverCallback>> locals;
    {
        std::lock_guard<std::mutex> lock(componentScreenReaderCallbackMutex_);
        locals = componentScreenReaderCallbackMap_;
    }
    for (auto& item : locals) {
        CHECK_NULL_CONTINUE(item.second);
        item.second->OnState(state);
    }
}

bool JsAccessibilityManager::CheckIsChildElement(
    int64_t &elementId,
    const std::vector<std::string> &params,
    std::vector<std::string> &info,
    DumpMode mode,
    int64_t &rootId)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;

    if (mode == DumpMode::TREE) {
        AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(rootId, splitElementId, splitTreeId);
        if (splitTreeId <= 0 || splitTreeId == treeId_) {
            rootId = splitElementId;
            return false;
        }
    } else {
        if (elementId <= 0) {
            return false;
        }

        AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);
        if (splitTreeId <= 0 || splitTreeId == treeId_) {
            elementId = splitElementId;
            return false;
        }
    }

    std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
    for (const auto &item : childTreeCallbackMap_) {
        if (item.second == nullptr) {
            continue;
        }
        if (item.second->GetChildTreeId() != splitTreeId) {
            continue;
        }
        item.second->OnDumpChildInfo(params, info);
        for (const auto &childInfo : info) {
            DumpLog::GetInstance().Print(childInfo.c_str());
        }
        return true;
    }
    return false;
}

bool JsAccessibilityManager::NeedRegisterChildTree(
    uint32_t parentWindowId, int32_t parentTreeId, int64_t parentElementId)
{
    if (!IsRegister()) {
        return true;
    }

    if (parentWindowId_ == parentWindowId && parentTreeId_ == parentTreeId && parentElementId_ == parentElementId) {
        return false;
    }
    DeregisterInteractionOperationAsChildTree();
    return true;
}

void JsAccessibilityManager::RegisterInteractionOperationAsChildTree(
    uint32_t parentWindowId, int32_t parentTreeId, int64_t parentElementId)
{
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(parentTreeId, parentElementId);
    if (!NeedRegisterChildTree(parentWindowId, parentTreeId, parentElementId)) {
        return;
    }

    std::shared_ptr<AccessibilitySystemAbilityClient> instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_VOID(instance);
    auto pipelineContext = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(pipelineContext);
    auto container = Platform::AceContainer::GetContainer(pipelineContext->GetInstanceId());
    if (container != nullptr && container->IsUIExtensionWindow()) {
        windowId_ = pipelineContext->GetRealHostWindowId();
    } else if (pipelineContext->IsFormRender()) {
        windowId_ = parentWindowId;
    }

    uint32_t windowId = GetWindowId();
    auto interactionOperation = std::make_shared<JsInteractionOperation>(windowId);
    interactionOperation->SetHandler(WeakClaim(this));
    Accessibility::Registration registration {
        .windowId = static_cast<int32_t>(windowId),
        .parentWindowId = static_cast<int32_t>(parentWindowId),
        .parentTreeId = parentTreeId,
        .elementId = parentElementId,
    };
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "windowId: %{public}u, parentWindowId: %{public}u, "
                                           "parentTreeId: %{public}d, %{public}" PRId64,
                                           windowId, parentWindowId, parentTreeId, parentElementId);
    Accessibility::RetError retReg = instance->RegisterElementOperator(registration, interactionOperation);
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "RegisterElementOperator result: %{public}d", retReg);
    Register(retReg == RET_OK);
    AceApplicationInfo::GetInstance().SetAccessibilityEnabled(retReg == RET_OK);
    bool isScreenReadEnabled = false;
    instance->IsScreenReaderEnabled(isScreenReadEnabled);
    AceApplicationInfo::GetInstance().SetAccessibilityScreenReadEnabled(isScreenReadEnabled);
    parentElementId_ = parentElementId;
    parentTreeId_ = parentTreeId;
    parentWindowId_ = parentWindowId;

    for (const auto& subContext : GetSubPipelineContexts()) {
        auto context = subContext.Upgrade();
        CHECK_NULL_CONTINUE(context);
        interactionOperation = std::make_shared<JsInteractionOperation>(context->GetWindowId());
        interactionOperation->SetHandler(WeakClaim(this));
        instance->RegisterElementOperator(context->GetWindowId(), interactionOperation);
    }
}

void JsAccessibilityManager::SetAccessibilityGetParentRectHandler(std::function<void(int32_t &, int32_t &)> &&callback)
{
    getParentRectHandler_ = std::move(callback);
}

void JsAccessibilityManager::SetAccessibilityGetParentRectHandler(
    std::function<void(AccessibilityParentRectInfo &)> &&callback)
{
    getParentRectHandlerNew_ = std::move(callback);
}

void JsAccessibilityManager::DeregisterInteractionOperationAsChildTree()
{
    if (!IsRegister()) {
        return;
    }
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "deregister accessibility childTree");

    std::shared_ptr<AccessibilitySystemAbilityClient> instance = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_VOID(instance);
    uint32_t windowId = GetWindowId();
    Register(false);

    // Clear focus state only for form render
    if (IsFormRender()) {
        ClearAccessibilityFocusState();
    }

    instance->DeregisterElementOperator(windowId, treeId_);
    InitAccessibilityEnabledAndScreenReadEnabled(); // for many subtrees under the same instance
    parentElementId_ = INVALID_PARENT_ID;
    parentTreeId_ = 0;
    parentWindowId_ = 0;
    NotifyChildTreeOnDeregister();

    RefPtr<PipelineBase> context;
    for (const auto& subContext : GetSubPipelineContexts()) {
        context = subContext.Upgrade();
        CHECK_NULL_CONTINUE(context);
        instance->DeregisterElementOperator(context->GetWindowId());
    }
}

void JsAccessibilityManager::SendUecOnTreeEvent(int64_t splitElementId)
{
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    auto frameNode = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(rootNode, splitElementId);
    CHECK_NULL_VOID(frameNode);

    if ((!frameNode) || (IsExtensionComponent(frameNode) && !IsUIExtensionShowPlaceholder(frameNode))) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "Register element is uec: %{public}s ", frameNode->GetTag().c_str());
        rootNode->OnAccessibilityEvent(AccessibilityEventType::CHANGE);
    } else {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "Register element is not uec: %{public}s", frameNode->GetTag().c_str());
    }
}
void JsAccessibilityManager::JsInteractionOperation::SetChildTreeIdAndWinId(
    const int64_t nodeId, const int32_t treeId, const int32_t childWindowId)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(nodeId, splitElementId, splitTreeId);
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "SetChildTreeId node: %{public}" PRId64 " treeId: %{public}d",
        splitElementId, treeId);
    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), splitElementId, treeId, childWindowId] {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            ACE_SCOPED_TRACE("SetChildTreeIdAndWinId");
            jsAccessibilityManager->NotifySetChildTreeIdAndWinId(splitElementId, treeId, childWindowId);
        },
        TaskExecutor::TaskType::UI, "ArkUISetChildTreeIdAndWinId");
}

void JsAccessibilityManager::JsInteractionOperation::SetBelongTreeId(const int32_t treeId)
{
    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    jsAccessibilityManager->treeId_ = treeId;
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    auto instanceId = context->GetInstanceId();
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), treeId, instanceId] {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            ContainerScope scope(instanceId);
            ACE_SCOPED_TRACE("SetBelongTreeId");
            jsAccessibilityManager->SendCacheAccessibilityEvent(instanceId);
            jsAccessibilityManager->NotifyChildTreeOnRegister(treeId);
            jsAccessibilityManager->ReleaseUIExtCacheEvent();
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilityClearCurrentFocus");
}

void JsAccessibilityManager::JsInteractionOperation::FocusMoveSearchWithCondition(
    const AccessibilityElementInfo& info, const AccessibilityFocusMoveParam param,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    HILOG_INFO_FOCUS(
        "focus move search with condition %{public}" PRId64 ", "
        "direction: %{public}d, condition %{public}d, focusRuleType %{public}d, parentId %{public}" PRId64 ", requestId %{public}d",
        info.GetAccessibilityId(), param.direction, param.condition, param.type, param.parentId, requestId);
    auto jsAccessibilityManager = GetHandler().Upgrade();
    std::list<AccessibilityElementInfo> infos;
    FocusMoveResult errorResult = {
        .resultType = FocusMoveResultType::SEARCH_FAIL_LOST_NODE,
        .nowLevelBelongTreeId = -1,
        .parentWindowId = 0,
        .changeToNewInfo = false,
        .needTerminate = true,
    };
    if (!jsAccessibilityManager) {
        Accessibility::AccessibilityElementInfo info;
        info.SetValidElement(false);
        infos.emplace_back(info);
        callback.SetFocusMoveSearchWithConditionResult(infos, errorResult, requestId);
        return;
    }
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    if (!context || !context->GetTaskExecutor()) {
        Accessibility::AccessibilityElementInfo info;
        info.SetValidElement(false);
        infos.emplace_back(info);
        callback.SetFocusMoveSearchWithConditionResult(infos, errorResult, requestId);
        return;
    }
    auto windowId = windowId_;
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), elementInfo = info, param, requestId, &callback, windowId, errorResult] {
            auto jsAccessibilityManager = weak.Upgrade();
            if (!jsAccessibilityManager) {
                Accessibility::AccessibilityElementInfo info;
                std::list<AccessibilityElementInfo> infos;
                info.SetValidElement(false);
                infos.emplace_back(info);
                callback.SetFocusMoveSearchWithConditionResult(infos, errorResult, requestId);
                return;
            }
            ACE_SCOPED_TRACE("FocusMoveSearchWithCondition");
            jsAccessibilityManager->FocusMoveSearchWithCondition(elementInfo, param, requestId, callback, windowId);
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilityFocusMoveSearchWithCondition");
}

void SetUpdateAccessibilityElementInfoFailResult(
    Accessibility::AccessibilityElementOperatorCallback& callback, int32_t requestId,
    Accessibility::OperateVirtualNodeResult result = Accessibility::OperateVirtualNodeResult::INTERNAL_ERROR)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
        "SetUpdateAccessibilityElementInfoFailResult fail, result: %{public}d, requestId: %{public}d",
        static_cast<int32_t>(result), requestId);
    callback.SetUpdateCustomAccessibilityPropertyResult(result, requestId);
}

void JsAccessibilityManager::JsInteractionOperation::UpdateCustomAccessibilityProperty(
    const int64_t elementId, const AccessibilityVirtualNode& accessibilityVirtualNode,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "ArkUI UpdateAccessibilityElementInfo by id: %{public}" PRId64
        ", requestId: %{public}d, windowId: %{public}d",
        elementId, requestId, windowId_);

    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID_WITH_ACTION(jsAccessibilityManager,
        SetUpdateAccessibilityElementInfoFailResult(callback, requestId));

    auto windowId = windowId_;
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID_WITH_ACTION(context, SetUpdateAccessibilityElementInfoFailResult(callback, requestId));

    auto executor = context->GetTaskExecutor();
    CHECK_NULL_VOID_WITH_ACTION(executor, SetUpdateAccessibilityElementInfoFailResult(callback, requestId));
    executor->PostTask(
        [weak = GetHandler(), splitElementId, attribute = accessibilityVirtualNode, requestId, &callback, windowId]() {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID_WITH_ACTION(jsAccessibilityManager,
                SetUpdateAccessibilityElementInfoFailResult(callback, requestId));
            ACE_SCOPED_TRACE("UpdateCustomAccessibilityElementInfo");
            jsAccessibilityManager->UpdateAccessibilityElementInfo(
                splitElementId, attribute, requestId, callback, windowId);
        }, TaskExecutor::TaskType::UI, "ArkUIAccessibilityUpdateAccessibilityElementInfo");
}

void JsAccessibilityManager::JsInteractionOperation::AddAccessibilityVirtualNode(
    const int64_t elementId, const std::vector<AccessibilityVirtualNode>& nodes,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "ArkUI AddAccessibilityVirtualNode by id: %{public}" PRId64
        ", requestId: %{public}d, windowId: %{public}d, nodesize: %{public}zu",
        elementId, requestId, windowId_, nodes.size());

    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID_WITH_ACTION(jsAccessibilityManager, SetAddAccessibilityVirtualNodeFailResult(callback, requestId));

    auto windowId = windowId_;
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID_WITH_ACTION(context, SetAddAccessibilityVirtualNodeFailResult(callback, requestId));

    auto executor = context->GetTaskExecutor();
    CHECK_NULL_VOID_WITH_ACTION(executor, SetAddAccessibilityVirtualNodeFailResult(callback, requestId));
    executor->PostTask(
        [weak = GetHandler(), splitElementId, nodes = std::vector<AccessibilityVirtualNode>(nodes),
            requestId, &callback, windowId]() {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID_WITH_ACTION(jsAccessibilityManager, 
                SetAddAccessibilityVirtualNodeFailResult(callback, requestId));
            ACE_SCOPED_TRACE("AddAccessibilityVirtualNode");
            jsAccessibilityManager->AddAccessibilityVirtualNode(splitElementId, nodes, requestId, callback, windowId);
        },
        TaskExecutor::TaskType::UI, "ArkUIAddAccessibilityVirtualNode");
}

void JsAccessibilityManager::JsInteractionOperation::RemoveAccessibilityVirtualNode(
    const int64_t elementId, const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "ArkUI RemoveAccessibilityVirtualNode by id: %{public}" PRId64
        ", requestId: %{public}d, windowId: %{public}d", elementId, requestId, windowId_);

    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID_WITH_ACTION(jsAccessibilityManager,
        SetRemoveAccessibilityVirtualNodeFailResult(callback, requestId));

    auto windowId = windowId_;
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID_WITH_ACTION(context, SetRemoveAccessibilityVirtualNodeFailResult(callback, requestId));

    auto executor = context->GetTaskExecutor();
    CHECK_NULL_VOID_WITH_ACTION(executor, SetRemoveAccessibilityVirtualNodeFailResult(callback, requestId));
    executor->PostTask(
        [weak = GetHandler(), splitElementId, requestId, &callback, windowId]() {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID_WITH_ACTION(jsAccessibilityManager, 
                SetRemoveAccessibilityVirtualNodeFailResult(callback, requestId));
            ACE_SCOPED_TRACE("RemoveAccessibilityVirtualNode");
            jsAccessibilityManager->RemoveAccessibilityVirtualNode(splitElementId, requestId, callback, windowId);
        },
        TaskExecutor::TaskType::UI, "ArkUIRemoveAccessibilityVirtualNode");
}

namespace {
// Add the exported root's window position to a subtree-local SetRectInScreen value (see JsAccessibilityManager::GetExportedRootWindowOffset).
void OffsetRectInScreenToWindow(Accessibility::AccessibilityElementInfo& info, float offsetX, float offsetY)
{
    auto rect = info.GetRectInScreen();
    Accessibility::Rect bounds {
        static_cast<int32_t>(rect.GetLeftTopXScreenPostion() + offsetX),
        static_cast<int32_t>(rect.GetLeftTopYScreenPostion() + offsetY),
        static_cast<int32_t>(rect.GetRightBottomXScreenPostion() + offsetX),
        static_cast<int32_t>(rect.GetRightBottomYScreenPostion() + offsetY),
    };
    info.SetRectInScreen(bounds);
}
} // namespace

void JsAccessibilityManager::CompensateCmpEventRect(AccessibilityEventInfo& eventInfo)
{
    auto info = eventInfo.GetElementInfo();
    if (!IsCmpSubAM() || info.GetBelongTreeId() <= 0 || info.GetBelongTreeId() != treeId_) {
        return;
    }
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    if (GetExportedRootWindowOffset(offsetX, offsetY)) {
        OffsetRectInScreenToWindow(info, offsetX, offsetY);
        eventInfo.SetElementInfo(info);
    }
}

// CMP mixed-rendering: adoption-mounted detection for an exported subtree.
bool JsAccessibilityManager::IsUnderAdoptedMount(const RefPtr<NG::FrameNode>& node)
{
    CHECK_NULL_RETURN(node, false);
    auto current = node;
    while (current) {
        if (current->IsAdopted()) {
            return true;
        }
        current = current->GetParentFrameNode();
    }
    return false;
}

bool JsAccessibilityManager::GetExportedRootWindowOffset(float& offsetX, float& offsetY)
{
    // Anchors registered at RegisterCmpSubtreeToAaks: the CMP-tree operator (for the host node and the provider) and the bindId mount.
    auto exportedRoot = cmpExportedRoot_.Upgrade();
    if (exportedRoot && !IsUnderAdoptedMount(exportedRoot)) {
        return false;
    }
    auto providerOp = cmpProviderOperation_.lock();
    CHECK_NULL_RETURN(providerOp, false);
    auto hostNode = providerOp->GetHost();
    CHECK_NULL_RETURN(hostNode, false);
    auto hostRect = hostNode->GetTransformRectRelativeToWindow();
    std::list<AccessibilityElementInfo> mountInfos;
    // parentElementId_ carries the T_cmp splice (AAMS-facing mount id — both registration
    // paths splice it in: RegisterCmpSubtreeToAams / RegisterInteractionOperationAsChildTree);
    // the provider query takes the provider's OWN raw id — split before querying (the same
    // convention as every other FromProvider/FindNativeInfoById call site).
    int64_t rawMountElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t mountTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(
        parentElementId_, rawMountElementId, mountTreeId);
    // ignoreHostOffset=true: the provider's own rect space (the same space the hover walk uses), NOT the host-offset-overlaid one.
    if (!providerOp->FindAccessibilityNodeInfosByIdFromProvider(rawMountElementId, 0, 0, mountInfos, true)
        || mountInfos.empty()) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
            "GetExportedRootWindowOffset: mount info unavailable, bindId=%{public}" PRId64
            " (raw=%{public}" PRId64 ", spliced=%{public}" PRId64 ")",
            rawMountElementId, rawMountElementId, parentElementId_);
        return false;
    }
    auto mountRect = mountInfos.front().GetRectInScreen();
    offsetX = hostRect.Left() + static_cast<float>(mountRect.GetLeftTopXScreenPostion());
    offsetY = hostRect.Top() + static_cast<float>(mountRect.GetLeftTopYScreenPostion());
    return true;
}

void JsAccessibilityManager::UpdateElementInfoTreeId(Accessibility::AccessibilityElementInfo& info,
    bool compensateCmpRect)
{
    int32_t treeId = info.GetBelongTreeId();
    if (treeId <= 0) {
        return;
    }

    int64_t elementId = info.GetAccessibilityId();
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId, elementId);
    info.SetAccessibilityId(elementId);

    int64_t parentId = info.GetParentNodeId();
    if ((parentId != INVALID_PARENT_ID) && NeedChangeTreeId(parentId, treeId)) {
        AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId, parentId);
        info.SetParent(parentId);
    }

    UpdateElementInfoPageIdWithTreeId(info, treeId);

    std::vector<int64_t> childIds = info.GetChildIds();
    for (int64_t child : childIds) {
        if (NeedChangeTreeId(child, treeId)) {
            info.RemoveChild(child);
            AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId, child);
            info.AddChild(child);
        }
    }

    // CMP exported-subtree rect compensation (problem 2): this info belongs to THIS subAM's tree and its rect came.
    if (compensateCmpRect && IsCmpSubAM() && treeId == treeId_) {
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        if (GetExportedRootWindowOffset(offsetX, offsetY)) {
            OffsetRectInScreenToWindow(info, offsetX, offsetY);
        }
    }
}

void JsAccessibilityManager::UpdateElementInfosTreeId(
    std::list<Accessibility::AccessibilityElementInfo>& infos, bool checkEmbed)
{
    int32_t rootTreeId = INVALID_NODE_ID;
    // CMP exported-subtree rect compensation (problem 2): computed lazily on the first info belonging to THIS subAM's.
    bool offsetComputed = false;
    bool offsetAvailable = false;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    for (auto& item : infos) {
        int32_t treeId = item.GetBelongTreeId();
        if (treeId <= 0) {
            continue;
        }
        if (rootTreeId == INVALID_NODE_ID) {
            rootTreeId = treeId;
        }

        int64_t elementId = item.GetAccessibilityId();
        AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId, elementId);
        item.SetAccessibilityId(elementId);

        if (IsCmpSubAM() && treeId == treeId_) {
            if (!offsetComputed) {
                offsetComputed = true;
                offsetAvailable = GetExportedRootWindowOffset(offsetX, offsetY);
                if (offsetAvailable) {
                    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
                        "ExportedRootWindowOffset: offX=%{public}f, offY=%{public}f", offsetX, offsetY);
                }
            }
            if (offsetAvailable) {
                OffsetRectInScreenToWindow(item, offsetX, offsetY);
            }
        }

        if (checkEmbed && (rootTreeId != INVALID_NODE_ID) && (rootTreeId != treeId)) {
            continue;
        }

        int64_t parentId = item.GetParentNodeId();
        if (parentId != INVALID_PARENT_ID) {
            if (NeedChangeTreeId(parentId, treeId)) {
                AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId, parentId);
                item.SetParent(parentId);
            }
        }

        UpdateElementInfoPageIdWithTreeId(item, treeId);
        if (checkEmbed && IsTagInEmbedComponent(item.GetComponentType())) {
            continue;
        }

        std::vector<int64_t> childIds = item.GetChildIds();
        for (int64_t child : childIds) {
            if (NeedChangeTreeId(child, treeId)) {
                item.RemoveChild(child);
                AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId, child);
                item.AddChild(child);
            }
        }
    }
}

void JsAccessibilityManager::SetPipelineContext(const RefPtr<PipelineBase>& context)
{
    context_ = context;
    if (stateObserver_[AccessibilityStateEventType::EVENT_ACCESSIBILITY_STATE_CHANGED] != nullptr) {
        stateObserver_[AccessibilityStateEventType::EVENT_ACCESSIBILITY_STATE_CHANGED]->SetPipeline(context_);
    }
}

void JsAccessibilityManager::JsAccessibilityStateObserver::OnStateChanged(const bool state)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "accessibility state changed:%{public}d, event type:%{public}u", state,
        eventType_);

    // Do not upgrade jsAccessibilityManager on async thread, destructor will cause freeze
    auto pipelineRef = pipeline_.Upgrade();
    CHECK_NULL_VOID(pipelineRef);
    pipelineRef->GetTaskExecutor()->PostTask(
        [weak = accessibilityManager_, state, eventType = eventType_]() {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);

            if (eventType == AccessibilityStateEventType::EVENT_ACCESSIBILITY_STATE_CHANGED) {
                auto client = AccessibilitySystemAbilityClient::GetInstance();
                if (client) {
                    std::vector<uint32_t> needEvents;
                    client->SearchNeedEvents(needEvents);
                    jsAccessibilityManager->UpdateEventWhiteList(needEvents);
                }
                auto pipelineRef = jsAccessibilityManager->GetPipelineContext().Upgrade();
                if (jsAccessibilityManager->ShouldSkipAccessibilityStateChange(pipelineRef)) {
                    return;
                }

                if (state) {
                    jsAccessibilityManager->RegisterInteractionOperation(jsAccessibilityManager->GetWindowId());
                } else {
                    jsAccessibilityManager->DeregisterInteractionOperation();
                }
                AceApplicationInfo::GetInstance().SetAccessibilityEnabled(state);
                jsAccessibilityManager->NotifyAccessibilitySAStateChange(state);
            } else if (eventType == AccessibilityStateEventType::EVENT_SCREEN_READER_STATE_CHANGED) {
                jsAccessibilityManager->isScreenReaderEnabledInitialized_ = true;
                jsAccessibilityManager->isScreenReaderEnabled_ = state;
                AceApplicationInfo::GetInstance().SetAccessibilityScreenReadEnabled(state);
                jsAccessibilityManager->NotifyScreenReaderObserverStateChange(state);
            } else if (eventType == AccessibilityStateEventType::EVENT_CONFIG_EVENT_CHANGED) {
                std::vector<uint32_t> needEvents;
                auto client = AccessibilitySystemAbilityClient::GetInstance();
                CHECK_NULL_VOID(client);
                client->SearchNeedEvents(needEvents);
                jsAccessibilityManager->UpdateEventWhiteList(needEvents);
            }
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilityStateChanged");
}

void JsAccessibilityManager::JsInteractionOperation::FocusMoveSearch(
    int64_t elementId, const int32_t direction, const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "move search: %{public}" PRId64 ", direction: %{public}d",
        elementId, direction);
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);

    auto jsAccessibilityManager = GetHandler().Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto context = jsAccessibilityManager->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    auto windowId = windowId_;
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), splitElementId, direction, requestId, &callback, windowId] {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            ACE_SCOPED_TRACE("FocusMoveSearch");
            jsAccessibilityManager->FocusMoveSearch(splitElementId, direction, requestId, callback, windowId);
        },
        TaskExecutor::TaskType::UI, "ArkUIAccessibilityFocusMoveSearch");
}

bool JsAccessibilityManager::ShouldSkipAccessibilityStateChange(const RefPtr<PipelineBase>& pipelineRef)
{
    CHECK_NULL_RETURN(pipelineRef, false);
    auto pipelineContext = AceType::DynamicCast<NG::PipelineContext>(pipelineRef);
    CHECK_NULL_RETURN(pipelineContext, false);
    auto container = Platform::AceContainer::GetContainer(pipelineContext->GetInstanceId());
    if (container != nullptr && container->IsDynamicRender()) {
        return true;
    }
    if (pipelineContext->IsFormRender() || pipelineContext->IsJsCard() || pipelineContext->IsJsPlugin()) {
        return true;
    }
    if (container != nullptr && container->IsUIExtensionWindow()) {
        return true;
    }
    return false;
}

void JsAccessibilityManager::FocusMoveSearch(const int64_t elementId, const int32_t direction, const int32_t requestId,
    Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t windowId)
{
    AccessibilityElementInfo nodeInfo;
    auto context = GetPipelineByWindowId(windowId);
    if (!context) {
        nodeInfo.SetValidElement(false);
        SetFocusMoveSearchResult(callback, nodeInfo, requestId);
        return;
    }

    if (AceType::InstanceOf<NG::PipelineContext>(context)) {
        FocusMoveSearchNG(elementId, direction, nodeInfo, context, NG::UI_EXTENSION_OFFSET_MAX);
        SetFocusMoveSearchResult(callback, nodeInfo, requestId);
        return;
    }

    auto node = GetAccessibilityNodeFromPage(elementId);
    if (!node) {
        nodeInfo.SetValidElement(false);
        SetFocusMoveSearchResult(callback, nodeInfo, requestId);
        return;
    }

    // get root node.
    auto rootNode = node;
    while (rootNode->GetParentNode()) {
        rootNode = rootNode->GetParentNode();
        if (!rootNode->GetParentNode()) {
            break;
        }
    }

    std::list<RefPtr<AccessibilityNode>> nodeList;
    AddFocusableNode(nodeList, rootNode);
    RefPtr<AccessibilityNode> resultNode;

    switch (direction) {
        case FocusMoveDirection::FORWARD:
        case FocusMoveDirection::BACKWARD:
            // forward and backward
            resultNode = FindNodeInRelativeDirection(nodeList, node, direction);
            break;
        case FocusMoveDirection::UP:
        case FocusMoveDirection::DOWN:
        case FocusMoveDirection::LEFT:
        case FocusMoveDirection::RIGHT:
            // up, down, left and right
            resultNode = FindNodeInAbsoluteDirection(nodeList, node, direction);
            break;
        default:
            break;
    }

    if (resultNode) {
        auto jsAccessibilityManager = Claim(this);
        UpdateAccessibilityNodeInfo(resultNode, nodeInfo, jsAccessibilityManager, windowId_);
    }

    SetFocusMoveSearchResult(callback, nodeInfo, requestId);
}

void JsAccessibilityManager::AddFocusableNode(
    std::list<RefPtr<AccessibilityNode>>& nodeList, const RefPtr<AccessibilityNode>& node)
{
    const std::string importance = node->GetImportantForAccessibility();
    if (CanAccessibilityFocused(node)) {
        nodeList.push_back(node);
    }
    if (!node->GetAccessible() && importance != "no-hide-descendants") {
        for (auto& child : node->GetChildList()) {
            AddFocusableNode(nodeList, child);
        }
    }
}

bool JsAccessibilityManager::CanAccessibilityFocused(const RefPtr<AccessibilityNode>& node)
{
    return node != nullptr && !node->IsRootNode() && node->GetVisible() &&
           node->GetImportantForAccessibility() != "no" &&
           node->GetImportantForAccessibility() != "no-hide-descendants";
}

RefPtr<AccessibilityNode> JsAccessibilityManager::FindNodeInRelativeDirection(
    const std::list<RefPtr<AccessibilityNode>>& nodeList, RefPtr<AccessibilityNode>& node, const int direction)
{
    switch (direction) {
        case FocusMoveDirection::FORWARD:
            return GetNextFocusableNode(nodeList, node);
        case FocusMoveDirection::BACKWARD:
            return GetPreviousFocusableNode(nodeList, node);
        default:
            break;
    }

    return nullptr;
}

RefPtr<AccessibilityNode> JsAccessibilityManager::FindNodeInAbsoluteDirection(
    const std::list<RefPtr<AccessibilityNode>>& nodeList, RefPtr<AccessibilityNode>& node, const int direction)
{
    auto tempBest = node->GetRect();
    auto nodeRect = node->GetRect();

    switch (direction) {
        case FocusMoveDirection::LEFT:
            tempBest.SetLeft(node->GetLeft() + node->GetWidth() + 1);
            break;
        case FocusMoveDirection::RIGHT:
            tempBest.SetLeft(node->GetLeft() - node->GetWidth() - 1);
            break;
        case FocusMoveDirection::UP:
            tempBest.SetTop(node->GetTop() + node->GetHeight() + 1);
            break;
        case FocusMoveDirection::DOWN:
            tempBest.SetTop(node->GetTop() - node->GetHeight() - 1);
            break;
        default:
            break;
    }

    RefPtr<AccessibilityNode> nearestNode = nullptr;
    for (auto nodeItem = nodeList.begin(); nodeItem != nodeList.end(); nodeItem++) {
        if ((*nodeItem)->GetNodeId() == node->GetNodeId() || (*nodeItem)->IsRootNode()) {
            continue;
        }
        auto itemRect = (*nodeItem)->GetRect();
        if (CheckBetterRect(nodeRect, direction, itemRect, tempBest)) {
            tempBest = itemRect;
            nearestNode = (*nodeItem);
        }
    }

    return nearestNode;
}

RefPtr<AccessibilityNode> JsAccessibilityManager::GetNextFocusableNode(
    const std::list<RefPtr<AccessibilityNode>>& nodeList, RefPtr<AccessibilityNode>& node)
{
    auto nodeItem = nodeList.begin();
    for (; nodeItem != nodeList.end(); nodeItem++) {
        if ((*nodeItem)->GetNodeId() == node->GetNodeId()) {
            break;
        }
    }

    if (nodeItem != nodeList.end() && ++nodeItem != nodeList.end()) {
        return (*nodeItem);
    }
    if (!nodeList.empty()) {
        return (*nodeList.begin());
    }

    return nullptr;
}

RefPtr<AccessibilityNode> JsAccessibilityManager::GetPreviousFocusableNode(
    const std::list<RefPtr<AccessibilityNode>>& nodeList, RefPtr<AccessibilityNode>& node)
{
    auto nodeItem = nodeList.rbegin();
    for (; nodeItem != nodeList.rend(); nodeItem++) {
        if ((*nodeItem)->GetNodeId() == node->GetNodeId()) {
            break;
        }
    }

    if (nodeItem != nodeList.rend() && ++nodeItem != nodeList.rend()) {
        return (*nodeItem);
    }

    if (!nodeList.empty()) {
        return (*nodeList.rbegin());
    }
    return nullptr;
}

bool JsAccessibilityManager::RequestAccessibilityFocus(const RefPtr<AccessibilityNode>& node)
{
    auto requestNodeId = node->GetNodeId();
    if (currentFocusNodeId_ == requestNodeId) {
        return false;
    }

    ClearCurrentFocus();
    currentFocusNodeId_ = requestNodeId;
    node->SetAccessibilityFocusedState(true);
    return node->ActionAccessibilityFocus(true);
}

bool JsAccessibilityManager::ClearAccessibilityFocus(const RefPtr<AccessibilityNode>& node)
{
    auto requestNodeId = node->GetNodeId();
    if (currentFocusNodeId_ != requestNodeId) {
        return false;
    }

    currentFocusNodeId_ = -1;
    node->SetAccessibilityFocusedState(false);
    return node->ActionAccessibilityFocus(false);
}

bool JsAccessibilityManager::ClearCurrentFocus()
{
    auto currentFocusNode = GetAccessibilityNodeFromPage(currentFocusNodeId_);
    CHECK_NULL_RETURN(currentFocusNode, false);
    currentFocusNodeId_ = -1;
    currentFocusNode->SetFocusedState(false);
    currentFocusNode->SetAccessibilityFocusedState(false);
    return currentFocusNode->ActionAccessibilityFocus(false);
}

void FocusExtensionElementMoveSearchNG(const SearchParameter& searchParam,
    const RefPtr<NG::FrameNode>& node, Accessibility::AccessibilityElementInfo& info)
{
    if (NG::UI_EXTENSION_OFFSET_MIN < (searchParam.uiExtensionOffset + 1)) {
        node->FocusMoveSearchNG(searchParam.nodeId, searchParam.mode,
            searchParam.uiExtensionOffset / NG::UI_EXTENSION_ID_FACTOR, info);
    } else {
        info.SetValidElement(false);
    }
}

void GetExtensionNextFocusableNode(const AccessibilityElementInfo& focusElement,
    const std::list<AccessibilityElementInfo>& nodeList, AccessibilityElementInfo& node)
{
    auto nodeItem = nodeList.begin();
    for (; nodeItem != nodeList.end(); nodeItem++) {
        if ((*nodeItem).GetAccessibilityId() == focusElement.GetAccessibilityId()) {
            break;
        }
    }

    if (nodeItem != nodeList.end() && ++nodeItem != nodeList.end()) {
        node = (*nodeItem);
    }
    if (!nodeList.empty()) {
        node = (*nodeList.begin());
    }
}

void GetExtensionPreviousFocusableNode(const AccessibilityElementInfo& focusElement,
    const std::list<AccessibilityElementInfo>& nodeList, AccessibilityElementInfo& node)
{
    auto nodeItem = nodeList.rbegin();
    for (; nodeItem != nodeList.rend(); nodeItem++) {
        if ((*nodeItem).GetAccessibilityId() == focusElement.GetAccessibilityId()) {
            break;
        }
    }

    if (nodeItem != nodeList.rend() && ++nodeItem != nodeList.rend()) {
        node = (*nodeItem);
    }

    if (!nodeList.empty()) {
        node = (*nodeList.rbegin());
    }
}

// Subtree-scoped relative search for exported subtrees (CMP mixed-rendering).
void GetSubtreeNextFocusableNode(const AccessibilityElementInfo& focusElement,
    const std::list<AccessibilityElementInfo>& nodeList, AccessibilityElementInfo& node)
{
    auto nodeItem = nodeList.begin();
    for (; nodeItem != nodeList.end(); nodeItem++) {
        if ((*nodeItem).GetAccessibilityId() == focusElement.GetAccessibilityId()) {
            break;
        }
    }

    bool originFound = nodeItem != nodeList.end();
    if (originFound) {
        ++nodeItem;
    }
    if (nodeItem != nodeList.end()) {
        node = (*nodeItem);
        return;
    }
    if (!originFound && !nodeList.empty()) {
        node = (*nodeList.begin());
    }
}

void GetSubtreePreviousFocusableNode(const AccessibilityElementInfo& focusElement,
    const std::list<AccessibilityElementInfo>& nodeList, AccessibilityElementInfo& node)
{
    auto nodeItem = nodeList.rbegin();
    for (; nodeItem != nodeList.rend(); nodeItem++) {
        if ((*nodeItem).GetAccessibilityId() == focusElement.GetAccessibilityId()) {
            break;
        }
    }

    bool originFound = nodeItem != nodeList.rend();
    if (originFound) {
        ++nodeItem;
    }
    if (nodeItem != nodeList.rend()) {
        node = (*nodeItem);
        return;
    }
    if (!originFound && !nodeList.empty()) {
        node = (*nodeList.rbegin());
    }
}

void FindSubtreeNodeInRelativeDirection(const AccessibilityElementInfo& focusElement,
    const std::list<AccessibilityElementInfo>& nodeList, AccessibilityElementInfo& node, int direction)
{
    switch (direction) {
        case FocusMoveDirection::FORWARD:
            GetSubtreeNextFocusableNode(focusElement, nodeList, node);
            break;
        case FocusMoveDirection::BACKWARD:
            GetSubtreePreviousFocusableNode(focusElement, nodeList, node);
            break;
        default:
            break;
    }
}

void FindExtensionNodeInAbsoluteDirection(const AccessibilityElementInfo& focusElement,
    const std::list<AccessibilityElementInfo>& nodeList, AccessibilityElementInfo& node, int32_t direction)
{
    auto left = focusElement.GetRectInScreen().GetLeftTopXScreenPostion();
    auto top = focusElement.GetRectInScreen().GetLeftTopYScreenPostion();
    auto width = focusElement.GetRectInScreen().GetRightBottomXScreenPostion() -
        focusElement.GetRectInScreen().GetLeftTopXScreenPostion();
    auto height = focusElement.GetRectInScreen().GetRightBottomYScreenPostion() -
        focusElement.GetRectInScreen().GetLeftTopYScreenPostion();
    Rect rect(left, top, width, height);
    Rect tempBest(left, top, width, height);
    auto nodeRect = tempBest;
    switch (direction) {
        case FocusMoveDirection::LEFT:
            tempBest.SetLeft(left + width + 1);
            break;
        case FocusMoveDirection::RIGHT:
            tempBest.SetLeft(left - width - 1);
            break;
        case FocusMoveDirection::UP:
            tempBest.SetTop(top + height + 1);
            break;
        case FocusMoveDirection::DOWN:
            tempBest.SetTop(top - height - 1);
            break;
        default:
            break;
    }
    for (const auto& nodeItem : nodeList) {
        if (nodeItem.GetAccessibilityId() == focusElement.GetAccessibilityId() ||
            V2::ROOT_ETS_TAG == nodeItem.GetComponentType()) {
            continue;
        }
        left = nodeItem.GetRectInScreen().GetLeftTopXScreenPostion();
        top = nodeItem.GetRectInScreen().GetLeftTopYScreenPostion();
        width = nodeItem.GetRectInScreen().GetRightBottomXScreenPostion() -
            nodeItem.GetRectInScreen().GetLeftTopXScreenPostion();
        height = nodeItem.GetRectInScreen().GetRightBottomYScreenPostion() -
            nodeItem.GetRectInScreen().GetLeftTopYScreenPostion();
        Rect itemRect(left, top, width, height);
        if (CheckBetterRect(nodeRect, direction, itemRect, tempBest)) {
            tempBest = itemRect;
            node = nodeItem;
        }
    }
}

void FindExtensionNodeInRelativeDirection(const AccessibilityElementInfo& focusElement,
    const std::list<AccessibilityElementInfo>& nodeList, AccessibilityElementInfo& node, int direction)
{
    switch (direction) {
        case FocusMoveDirection::FORWARD:
            GetExtensionNextFocusableNode(focusElement, nodeList, node);
            break;
        case FocusMoveDirection::BACKWARD:
            GetExtensionPreviousFocusableNode(focusElement, nodeList, node);
            break;
        default:
            break;
    }
}

void FilterAccessibilityElementByFocusable(std::list<AccessibilityElementInfo>& elementList,
    AccessibilityElementInfo& focusElement, int64_t elementId, bool preserveChildTreeHosts = false)
{
    auto input = elementList;
    elementList.clear();
    std::set<int64_t> filterIds;
    for (auto& element : input) {
        if (preserveChildTreeHosts && element.GetChildTreeId() > 0) {
            elementList.emplace_back(element);
            if (element.GetAccessibilityId() == elementId) {
                focusElement = element;
            }
            continue;
        }
        if (filterIds.find(element.GetParentNodeId()) != filterIds.end()) {
            filterIds.insert(element.GetAccessibilityId());
            continue;
        }
        auto width = element.GetRectInScreen().GetRightBottomXScreenPostion() -
            element.GetRectInScreen().GetLeftTopXScreenPostion();
        auto height = element.GetRectInScreen().GetRightBottomYScreenPostion() -
            element.GetRectInScreen().GetLeftTopYScreenPostion();
        if (width == 0 || height == 0) {
            filterIds.insert(element.GetAccessibilityId());
            continue;
        }
        elementList.emplace_back(element);
        if (element.GetAccessibilityId() == elementId) {
            focusElement = element;
        }
    }
}

void JsAccessibilityManager::GetResultOfFocusMoveSearchNG(
    int64_t elementId, int32_t direction, AccessibilityElementInfo& info)
{
    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    std::list<AccessibilityElementInfo> nodeList;
    SearchElementInfoByAccessibilityIdNG(NG::UI_EXTENSION_ROOT_ID,
        PREFETCH_RECURSIVE_CHILDREN, nodeList, pipeline, NG::UI_EXTENSION_OFFSET_MAX);
    AccessibilityElementInfo focusElement;
    FilterAccessibilityElementByFocusable(nodeList, focusElement, elementId);
    switch (direction) {
        case FocusMoveDirection::FORWARD:
        case FocusMoveDirection::BACKWARD:
            Framework::FindExtensionNodeInRelativeDirection(focusElement, nodeList, info, direction);
            break;
        case FocusMoveDirection::UP:
        case FocusMoveDirection::DOWN:
        case FocusMoveDirection::LEFT:
        case FocusMoveDirection::RIGHT:
            Framework::FindExtensionNodeInAbsoluteDirection(focusElement, nodeList, info, direction);
            break;
        default:
            break;
    }
}

void JsAccessibilityManager::FocusMoveSearchNG(int64_t elementId, int32_t direction,
    Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
    const int64_t uiExtensionOffset)
{
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID(rootNode);
    auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(rootNode, elementId);
    info.SetValidElement(false);
    GetResultOfFocusMoveSearchNG(elementId, direction, info);
}

void JsAccessibilityManager::FocusExtensionElementMoveSearchNG(const SearchParameter& searchParam,
    Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
    const RefPtr<NG::FrameNode>& root, RefPtr<NG::FrameNode>& outputExtensionNode)
{
#ifdef WINDOW_SCENE_SUPPORTED
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    auto uiExtensionManager = ngPipeline->GetUIExtensionManager();
    CHECK_NULL_VOID(uiExtensionManager);
    auto elementIdPair =
        uiExtensionManager->UnWrapExtensionAbilityId(searchParam.uiExtensionOffset, searchParam.nodeId);
    outputExtensionNode = uiExtensionManager->GetFocusUiExtensionNode();
    CHECK_NULL_VOID(outputExtensionNode);
    SearchParameter transferSearchParam {elementIdPair.second, "",
        searchParam.mode, searchParam.uiExtensionOffset};
    OHOS::Ace::Framework::FocusExtensionElementMoveSearchNG(transferSearchParam, outputExtensionNode, info);
    AccessibilityElementInfo parentInfo;
    ConvertExtensionAccessibilityId(info, outputExtensionNode, searchParam.uiExtensionOffset, parentInfo);
#endif
}

// AccessibilitySystemAbilityClient will release callback after DeregisterElementOperator
void JsAccessibilityManager::SetSearchElementInfoByAccessibilityIdResult(AccessibilityElementOperatorCallback& callback,
    std::list<AccessibilityElementInfo>&& infos, const int32_t requestId, bool checkEmbed)
{
    if (!IsRegister()) {
        return;
    }
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), infos = std::move(infos), &callback, requestId, checkEmbed] () mutable {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "winId: %{public}d, treeId: %{public}d, reqId: %{public}d",
                jsAccessibilityManager->windowId_, jsAccessibilityManager->treeId_, requestId);
            if (!jsAccessibilityManager->IsRegister()) {
                return;
            }
            jsAccessibilityManager->UpdateElementInfosTreeId(infos, checkEmbed);
            callback.SetSearchElementInfoByAccessibilityIdResult(infos, requestId);
        }, TaskExecutor::TaskType::BACKGROUND, "ArkUIAccessibilitySetSearchElementInfoById");
}

void JsAccessibilityManager::SetSearchElementInfoByCustomIdResult(AccessibilityElementOperatorCallback& callback,
    std::list<AccessibilityElementInfo> &infos, const std::list<AccessibilityElementInfo> &treeInfos,
    const int32_t requestId)
{
    if (!IsRegister()) {
        return;
    }
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), infos = std::move(infos), treeInfos = std::move(treeInfos),
            &callback, requestId] () mutable {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "winId: %{public}d, treeId: %{public}d, reqId: %{public}d",
                jsAccessibilityManager->windowId_, jsAccessibilityManager->treeId_, requestId);
            if (!jsAccessibilityManager->IsRegister()) {
                return;
            }
            jsAccessibilityManager->UpdateElementInfosTreeId(infos);
            callback.SetSearchElementInfoBySpecificPropertyResult(infos, treeInfos, requestId);
        }, TaskExecutor::TaskType::BACKGROUND, "ArkUIAccessibilitySetSearchElementInfoById");
    return;
}

void JsAccessibilityManager::SetSearchElementInfoByTextResult(AccessibilityElementOperatorCallback& callback,
    std::list<AccessibilityElementInfo>&& infos, const int32_t requestId)
{
    if (!IsRegister()) {
        return;
    }
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), infos = std::move(infos), &callback, requestId] () mutable {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            if (!jsAccessibilityManager->IsRegister()) {
                return;
            }
            jsAccessibilityManager->UpdateElementInfosTreeId(infos);
            callback.SetSearchElementInfoByTextResult(infos, requestId);
        }, TaskExecutor::TaskType::BACKGROUND, "ArkUIAccessibilitySetSearchElementInfoByText");
}

void JsAccessibilityManager::SetSearchDefaultFocusByWindowIdResult(AccessibilityElementOperatorCallback& callback,
    std::list<AccessibilityElementInfo>&& infos, const int32_t requestId)
{
    if (!IsRegister()) {
        return;
    }
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), infos = std::move(infos), &callback, requestId] () mutable {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            if (!jsAccessibilityManager->IsRegister()) {
                return;
            }
            jsAccessibilityManager->UpdateElementInfosTreeId(infos);
            callback.SetSearchDefaultFocusByWindowIdResult(infos, requestId);
        }, TaskExecutor::TaskType::BACKGROUND, "ArkUIAccessibilitySetSearchDefaultFocusByWindowId");
}

void JsAccessibilityManager::SetFindFocusedElementInfoResult(
    AccessibilityElementOperatorCallback& callback, AccessibilityElementInfo& info, const int32_t requestId)
{
    if (IsRegister()) {
        UpdateElementInfoTreeId(info);
        callback.SetFindFocusedElementInfoResult(info, requestId);
    }
}

void JsAccessibilityManager::SetFocusMoveSearchResult(
    AccessibilityElementOperatorCallback& callback, AccessibilityElementInfo& info, const int32_t requestId)
{
    if (IsRegister()) {
        UpdateElementInfoTreeId(info);
        callback.SetFocusMoveSearchResult(info, requestId);
    }
}

void JsAccessibilityManager::SetFocusMoveResultWithNode(
    const WeakPtr<NG::FrameNode>& hostNode, AccessibilityElementOperatorCallback& callback, const int32_t requestId)
{
    if (!IsRegister()) {
        return;
    }
    AccessibilityElementInfo nodeInfo;
    auto host = hostNode.Upgrade();
    if (host) {
        auto context = host->GetContextRefPtr();
        auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
        CommonProperty commonProperty;
        GenerateCommonProperty(ngPipeline, commonProperty, ngPipeline, host);
        UpdateAccessibilityElementInfo(host, commonProperty, nodeInfo, ngPipeline);
    } else {
        nodeInfo.SetValidElement(false);
    }
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
            "SetFocusMoveResultWithNode : %{public}" PRId64, nodeInfo.GetAccessibilityId());
    SetFocusMoveSearchResult(callback, nodeInfo, requestId);
}

void JsAccessibilityManager::SetExecuteActionResult(
    AccessibilityElementOperatorCallback& callback, const bool succeeded, const int32_t requestId)
{
    if (IsRegister()) {
        callback.SetExecuteActionResult(succeeded, requestId);
    }
}

std::string JsAccessibilityManager::GetPagePath()
{
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, "");
    auto frontend = context->GetFrontend();
    CHECK_NULL_RETURN(frontend, "");
    ContainerScope scope(context->GetInstanceId());
    return frontend->GetPagePath();
}

bool JsAccessibilityManager::IsUpdateWindowSceneInfo(const RefPtr<NG::FrameNode>& node,
    NG::WindowSceneInfo& windowSceneInfo)
{
    CHECK_NULL_RETURN(node, false);
    // update windowScene node commonProperty left, top position and get scale data
    auto parent = node->GetAncestorNodeOfFrame(false);
    if (node->GetTag() == V2::WINDOW_SCENE_ETS_TAG) {
        parent = node;
    }

    while (parent) {
        // window scene with windowboundary property, is the same as window
        if ((parent->GetTag() != V2::WINDOW_SCENE_ETS_TAG) || (!parent->IsWindowBoundary())) {
            parent = parent->GetAncestorNodeOfFrame(false);
            continue;
        }
        auto accessibilityProperty = parent->GetAccessibilityProperty<NG::AccessibilityProperty>();
        if (accessibilityProperty) {
            accessibilityProperty->GetWindowScenePosition(windowSceneInfo);
        }
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
            "windowScene nodeId: %{public}" PRId64
            ", left: %{public}d, top: %{public}d, windowSceneScale: [%{public}f, %{public}f], "
            "innerWindowId:%{public}d, type: %{public}d",
            parent->GetAccessibilityId(), windowSceneInfo.left, windowSceneInfo.top,
            windowSceneInfo.scaleX, windowSceneInfo.scaleY, windowSceneInfo.innerWindowId,
            parent->GetWindowPatternType());
        return true;
    }
    return false;
}

void JsAccessibilityManager::UpdateElementInfoInnerWindowId(
    Accessibility::AccessibilityElementInfo& info, const int64_t& elementId)
{
    auto node = FindNodeFromPipeline(context_, elementId);
    if (node) {
        NG::WindowSceneInfo windowSceneInfo;
        IsUpdateWindowSceneInfo(node, windowSceneInfo);
        info.SetInnerWindowId(windowSceneInfo.innerWindowId);
    }
}

const std::string JsAccessibilityManager::GetPagePathInPageNodes(
    int32_t pageId,
    const std::vector<RefPtr<NG::FrameNode>>& pageNodes,
    const std::vector<std::string> pagePaths)
{
    if (pagePaths.empty()) {
        return GetPagePath();
    }
    if (pagePaths.size() != pageNodes.size()) {
        return GetPagePath();
    }
    for (std::size_t i = 0; i < pageNodes.size(); ++i) {
        CHECK_NULL_CONTINUE(pageNodes[i]);
        if (pageId != pageNodes[i]->GetPageId()) {
            continue;
        }
        return pagePaths[i];
    }
    return GetPagePath();
}

void JsAccessibilityManager::GetCurrentWindowPages(
    const RefPtr<NG::PipelineContext>& ngPipeline,
    std::vector<RefPtr<NG::FrameNode>>& pageNodes,
    std::vector<std::string>& pagePaths)
{
    CHECK_NULL_VOID(ngPipeline);
    auto stageManager = ngPipeline->GetStageManager();
    CHECK_NULL_VOID(stageManager);
    if (stageManager->IsSplitMode()) {
        pageNodes = stageManager->GetTopPagesWithTransition();
        pagePaths = stageManager->GetTopPagePaths();
        if (pageNodes.empty()) {
            TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "Split mode enable, cannot get page");
        }
        return;
    }
    auto page = stageManager->GetLastPageWithTransition();
    CHECK_NULL_VOID(page);
    pageNodes.push_back(page);
    pagePaths.push_back(GetPagePath());
}

void JsAccessibilityManager::UpdateWindowInfo(AccessibilityWindowInfo& windowInfo, const RefPtr<PipelineBase>& context)
{
    CHECK_NULL_VOID(context);
    auto container = Platform::AceContainer::GetContainer(context->GetInstanceId());
    CHECK_NULL_VOID(container);
    auto singleHandTransform = container->GetSingleHandTransform();
    windowInfo.top = windowInfo.top * singleHandTransform.scaleY_ + singleHandTransform.y_;
    windowInfo.left = windowInfo.left * singleHandTransform.scaleX_ + singleHandTransform.x_;
    windowInfo.scaleX *= singleHandTransform.scaleX_;
    windowInfo.scaleY *= singleHandTransform.scaleY_;
}

AccessibilityWindowInfo JsAccessibilityManager::GenerateWindowInfo(const RefPtr<NG::FrameNode>& node,
    const RefPtr<PipelineBase>& context)
{
    AccessibilityWindowInfo windowInfo;
    NG::WindowSceneInfo windowSceneInfo;
    if (IsUpdateWindowSceneInfo(node, windowSceneInfo)) {
        windowInfo.left = windowSceneInfo.left;
        windowInfo.top = windowSceneInfo.top;
        windowInfo.scaleX = windowSceneInfo.scaleX;
        windowInfo.scaleY = windowSceneInfo.scaleY;
        windowInfo.innerWindowId = windowSceneInfo.innerWindowId;
        return windowInfo;
    }
    RefPtr<NG::PipelineContext> ngPipeline;
    if (context) {
        ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    } else {
        ngPipeline = AceType::DynamicCast<NG::PipelineContext>(GetPipelineContext().Upgrade());
    }
    CHECK_NULL_RETURN(ngPipeline, windowInfo);
    auto container = Platform::AceContainer::GetContainer(ngPipeline->GetInstanceId());
    if (container && !container->IsSubWindow()) {
        // subwindow by subpipeline, donot use getParentRectHandler when it is registerd in mainpipeline
        if (getParentRectHandler_) {
            getParentRectHandler_(windowInfo.top, windowInfo.left);
            return windowInfo;
        } else if (getParentRectHandlerNew_) {
            if (IsReentrantLimit()) {
                return windowInfo;
            }
            SetReentrantLimit(true);
            AccessibilityParentRectInfo rectInfo;
            getParentRectHandlerNew_(rectInfo);
            windowInfo.top = rectInfo.top;
            windowInfo.left = rectInfo.left;
            windowInfo.scaleX = rectInfo.scaleX;
            windowInfo.scaleY = rectInfo.scaleY;
            windowInfo.rotateTransform = rectInfo.rotateTransform;
            SetReentrantLimit(false);
            return windowInfo;
        }
    }
    windowInfo.left = GetWindowLeft(ngPipeline->GetWindowId());
    windowInfo.top = GetWindowTop(ngPipeline->GetWindowId());
    if (container) {
        auto windowScale = container->GetWindowScale();
        windowInfo.scaleX = windowScale;
        windowInfo.scaleY = windowScale;
    }
    UpdateWindowInfo(windowInfo, ngPipeline);

    return windowInfo;
}
void JsAccessibilityManager::GenerateCommonProperty(const RefPtr<PipelineBase>& context, CommonProperty& output,
    const RefPtr<PipelineBase>& mainContext, const RefPtr<NG::FrameNode>& node)
{
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    if (!ngPipeline->IsFormRender()) {
        output.windowId = static_cast<int32_t>(ngPipeline->GetRealHostWindowId());
    } else {
        output.windowId = static_cast<int32_t>(GetWindowId());
    }

    auto windowInfo = GenerateWindowInfo(node, context);
    output.windowLeft = windowInfo.left;
    output.windowTop = windowInfo.top;
    scaleX_ = windowInfo.scaleX;
    scaleY_ = windowInfo.scaleY;
    output.scaleX = windowInfo.scaleX;
    output.scaleY = windowInfo.scaleY;
    output.innerWindowId = windowInfo.innerWindowId;
    output.rotateTransform = windowInfo.rotateTransform;
    // handle this time page info
    GetCurrentWindowPages(ngPipeline, output.pageNodes, output.pagePaths);
    if (context->GetWindowId() != mainContext->GetWindowId()) {
        output.pageNodes.clear();
        output.pagePaths.clear();
    }

    output.checkGetFunc = [weak = WeakClaim(this)](const RefPtr<NG::FrameNode>& node) -> int64_t {
        auto jsAccessibilityManager = weak.Upgrade();
        CHECK_NULL_RETURN(jsAccessibilityManager, INVALID_NODE_ID);
        return jsAccessibilityManager->CheckAndGetEmbedFrameNode(node);
    };
}

void JsAccessibilityManager::GenerateCommonPropertyForWeb(const RefPtr<PipelineBase>& context, CommonProperty& output,
    const RefPtr<PipelineBase>& mainContext, const RefPtr<NG::FrameNode>& node)
{
    GenerateCommonProperty(context, output, mainContext, node);
    auto parentRectInfo = GetTransformRectInfoRelativeToWindow(node, context);
    output.windowLeft = parentRectInfo.left;
    output.windowTop = parentRectInfo.top;
    // Adopted-mount bridge for a Web host inside an adoption-mounted exported subtree.
    if (CmpAccessibility::CmpAccessibilityRegistry::HasAnySubAM() && IsUnderAdoptedMount(node)) {
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        if (GetExportedRootWindowOffset(offsetX, offsetY)) {
            output.windowLeft += offsetX;
            output.windowTop += offsetY;
        }
    }
    output.rotateTransform = parentRectInfo.rotateTransform;
    output.scaleX = parentRectInfo.scaleX;
    output.scaleY = parentRectInfo.scaleY;
}

bool NeedSearchTextUpdateElementInfo(const RefPtr<NG::FrameNode>& frameNode, std::string searchText)
{
    CHECK_NULL_RETURN(frameNode, false);
    CHECK_NE_RETURN(frameNode->IsInternal(), false, false);

    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_RETURN(accessibilityProperty, false);
    if (accessibilityProperty->GetGroupText().find(searchText) != std::string::npos) {
        return true;
    }

    auto customProperty = accessibilityProperty->GetCustomAccessibilityProperty();
    CHECK_NULL_RETURN(customProperty, false);
    return (customProperty->GetText().find(searchText) != std::string::npos);
}

void JsAccessibilityManager::FindText(const RefPtr<NG::UINode>& node,
    std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<NG::PipelineContext>& context,
    const CommonProperty& commonProperty, const SearchParameter& searchParam)
{
    CHECK_NULL_VOID(node);
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    if (NeedSearchTextUpdateElementInfo(frameNode, searchParam.text)) {
        AccessibilityElementInfo nodeInfo;
        UpdateAccessibilityElementInfo(frameNode, commonProperty, nodeInfo, context);
        infos.emplace_back(nodeInfo);
    }
    if (IsExtensionComponent(frameNode) && !IsUIExtensionShowPlaceholder(frameNode)) {
        auto infosByIPC = SearchElementInfosByTextNG(NG::UI_EXTENSION_ROOT_ID, searchParam.text,
            frameNode, searchParam.uiExtensionOffset / NG::UI_EXTENSION_ID_FACTOR);
        if (!infosByIPC.empty()) {
            AccessibilityElementInfo nodeInfo;
            UpdateAccessibilityElementInfo(frameNode, commonProperty, nodeInfo, context);
            ConvertExtensionAccessibilityNodeId(infosByIPC, frameNode, searchParam.uiExtensionOffset, nodeInfo);
            for (auto& info : infosByIPC) {
                infos.emplace_back(info);
            }
        }
    }
    if (!node->GetChildren(true).empty()) {
        for (const auto& child : node->GetChildren(true)) {
            FindText(child, infos, context, commonProperty, searchParam);
        }
    }
}

void JsAccessibilityManager::FindTextByTextHint(const RefPtr<NG::UINode>& node,
    std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<NG::PipelineContext>& context,
    const CommonProperty& commonProperty, const SearchParameter& searchParam)
{
    CHECK_NULL_VOID(node);
    auto frameNode = AceType::DynamicCast<NG::FrameNode>(node);
    if (frameNode && !frameNode->IsInternal()) {
        std::string text = searchParam.text;
        nlohmann::json textJson = nlohmann::json::parse(text, nullptr, false);
        std::string value = "";
        if (!textJson.is_null() && !textJson.is_discarded() && textJson.contains("value")) {
            value = textJson["value"];
        }
        std::string textType = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>()->GetTextType();
        nlohmann::json textTypeJson = nlohmann::json::parse(textType, nullptr, false);
        if (!textTypeJson.is_null() && !textTypeJson.is_discarded() &&
            textTypeJson.contains("type") && textTypeJson["type"].is_string() && textTypeJson["type"] == value) {
            AccessibilityElementInfo nodeInfo;
            UpdateAccessibilityElementInfo(frameNode, commonProperty, nodeInfo, context);
            infos.emplace_back(nodeInfo);
        }
    }
    if (IsIsolatedComponent(frameNode)) {
        auto infosByIPC = SearchElementInfosByTextNG(NG::UI_EXTENSION_ROOT_ID, searchParam.text,
            frameNode, searchParam.uiExtensionOffset / NG::UI_EXTENSION_ID_FACTOR);
        if (!infosByIPC.empty()) {
            AccessibilityElementInfo nodeInfo;
            UpdateAccessibilityElementInfo(frameNode, commonProperty, nodeInfo, context);
            ConvertExtensionAccessibilityNodeId(infosByIPC, frameNode, searchParam.uiExtensionOffset, nodeInfo);
            for (auto& info : infosByIPC) {
                infos.emplace_back(info);
            }
        }
    }
    if (!node->GetChildren(true).empty()) {
        for (const auto& child : node->GetChildren(true)) {
            FindTextByTextHint(child, infos, context, commonProperty, searchParam);
        }
    }
}

void JsAccessibilityManager::TransferThirdProviderHoverEvent(
    const WeakPtr<NG::FrameNode>& hostNode, const NG::PointF& point, SourceType source,
    NG::AccessibilityHoverEventType eventType, TimeStamp time)
{
    auto pipelineContext = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(pipelineContext);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipelineContext);
    CHECK_NULL_VOID(ngPipeline);
    auto frameNode = hostNode.Upgrade();
    CHECK_NULL_VOID(frameNode);
    AccessibilityHoverForThirdConfig config;
    config.hostElementId = frameNode->GetAccessibilityId();
    config.point = point;
    config.sourceType = source;
    config.eventType = eventType;
    config.time = time;
    config.context = ngPipeline;
    HandleAccessibilityHoverForThird(config);
}

bool JsAccessibilityManager::OnDumpChildInfoForThird(
    int64_t hostElementId,
    const std::vector<std::string>& params,
    std::vector<std::string>& info)
{
    return OnDumpChildInfoForThirdRecursive(hostElementId, params, info, WeakClaim(this));
}

#ifdef WEB_SUPPORTED
void JsAccessibilityManager::GetWebCursorPosition(const int64_t elementId, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback, const RefPtr<NG::WebPattern>& webPattern)
{
    if (!webPattern) {
        callback.SetCursorPositionResult(0, requestId);
        TAG_LOGD(AceLogTag::ACE_WEB, "SetCursorPositionResult, requestId: %{public}d", requestId);
        return;
    }
    auto node = webPattern->GetTransitionalNodeById(elementId);
    if (!node) {
        callback.SetCursorPositionResult(0, requestId);
        TAG_LOGD(AceLogTag::ACE_WEB, "SetCursorPositionResult, requestId: %{public}d", requestId);
        return;
    }

    callback.SetCursorPositionResult(node->GetSelectionStart(), requestId);
}
#endif // WEB_SUPPORTED

void JsAccessibilityManager::FireAccessibilityEventCallback(uint32_t eventId, int64_t parameter)
{
    auto eventType = static_cast<AccessibilityCallbackEventId>(eventId);
    AccessibilityEvent event;
    switch (eventType) {
        case AccessibilityCallbackEventId::ON_LOAD_PAGE:
            event.nodeId = parameter;
            event.windowChangeTypes = WindowUpdateType::WINDOW_UPDATE_ACTIVE;
            event.type = AccessibilityEventType::CHANGE;
            SendAccessibilityAsyncEvent(event);
            break;
        case AccessibilityCallbackEventId::ON_SEND_ELEMENT_INFO_CHANGE:
            event.type = AccessibilityEventType::ELEMENT_INFO_CHANGE;
            event.nodeId = parameter;
            SendAccessibilityAsyncEvent(event);
            break;
        case AccessibilityCallbackEventId::ON_SEND_DETACH_FOCUS_FALLBACK:
            HandleDetachFocusFallbackCallback(static_cast<int32_t>(parameter));
            break;
        default:
            break;
    }
}

int32_t JsAccessibilityManager::GetTransformDegreeRelativeToWindow(const RefPtr<NG::FrameNode>& node, bool excludeSelf)
{
    int32_t rotateDegree = 0;
    auto context = node->GetRenderContext();
    if (context && !excludeSelf) {
        rotateDegree = context->GetRotateDegree();
    }
    auto parent = node->GetAncestorNodeOfFrame(true);
    while (parent) {
        if (parent->IsWindowBoundary()) {
            break;
        }
        auto contextParent = parent->GetRenderContext();
        if (contextParent) {
            rotateDegree += contextParent->GetRotateDegree();
        }
        parent = parent->GetAncestorNodeOfFrame(true);
    }
    return ((rotateDegree % FULL_ANGLE + FULL_ANGLE) % FULL_ANGLE);
}

AccessibilityParentRectInfo JsAccessibilityManager::GetTransformRectInfoRelativeToWindow(
    const RefPtr<NG::FrameNode>& node, const RefPtr<PipelineBase>& context)
{
    AccessibilityParentRectInfo rectInfo;
    CHECK_NULL_RETURN(node, rectInfo);
    auto finalScale = node->GetTransformScaleRelativeToWindow();
    rectInfo.scaleX = finalScale.x;
    rectInfo.scaleY = finalScale.y;
    CHECK_NULL_RETURN(context, rectInfo);
    auto windowInfo = GenerateWindowInfo(node, context);
    auto rectFinal = GetFinalRealRect(node);
    RotateTransform rotateData;
    RotateTransform windowRotateData = windowInfo.rotateTransform;
    rotateData.rotateDegree = GetTransformDegreeRelativeToWindow(node);
    AccessibilityRect rotateRect(rectFinal.GetX(), rectFinal.GetY(),
        rectFinal.Width(), rectFinal.Height());
    if (windowRotateData.rotateDegree) {
        rotateRect.Rotate(windowRotateData.innerCenterX, windowRotateData.innerCenterY,
            windowRotateData.rotateDegree);
        rotateRect.ApplyTransformation(windowRotateData, windowInfo.scaleX, windowInfo.scaleY);
        rotateData.rotateDegree += windowRotateData.rotateDegree;
    } else {
        RotateTransform roateDataTemp(0, windowInfo.left, windowInfo.top, 0, 0);
        rotateRect.ApplyTransformation(roateDataTemp, windowInfo.scaleX, windowInfo.scaleY);
    }
    rectInfo.left = rotateRect.GetX();
    rectInfo.top = rotateRect.GetY();
    rectInfo.scaleX *= windowInfo.scaleX;
    rectInfo.scaleY *= windowInfo.scaleY;
    if (rotateData.rotateDegree) {
        rotateData.centerX = static_cast<int32_t>(rotateRect.GetWidth()) * 0.5f + rotateRect.GetX();
        rotateData.centerY = static_cast<int32_t>(rotateRect.GetHeight()) * 0.5f + rotateRect.GetY();
        auto renderContext = node->GetRenderContext();
        CHECK_NULL_RETURN(renderContext, rectInfo);
        auto rectOrigin = renderContext->GetPaintRectWithoutTransform();
        rotateData.innerCenterX = rectOrigin.Width() * 0.5f;
        rotateData.innerCenterY = rectOrigin.Height() * 0.5f;
    }
    rectInfo.rotateTransform = rotateData;
    return rectInfo;
}

void JsAccessibilityManager::UpdateAccessibilityNodeRect(const RefPtr<NG::FrameNode>& frameNode)
{
    auto accessibilityWorkMode = GenerateAccessibilityWorkMode();
    if (!accessibilityWorkMode.isTouchExplorationEnabled) {
        return;
    }
    CHECK_NULL_VOID(frameNode);
    auto renderContext = frameNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    auto isFocus = accessibilityProperty->GetAccessibilityFocusState();
    if (isFocus && !frameNode->IsDrawFocusOnTop()) {
        auto isVirtualNode = frameNode->IsAccessibilityVirtualNode();
        if (accessibilityProperty->IsMatchAccessibilityResponseRegion(isVirtualNode)) {
            auto rectInt = accessibilityProperty->GetAccessibilityResponseRegionRect(isVirtualNode);
            UpdateFocusRectToRenderContext(frameNode, renderContext, rectInt);
        } else {
            renderContext->UpdateAccessibilityRoundRect();
        }
    } else {
        RefPtr<NG::FrameNode> focusFrameNode = nullptr;
        auto checkflag = CheckChildIsAccessibilityFocus(frameNode, focusFrameNode);
        if (checkflag && focusFrameNode && focusFrameNode->IsDrawFocusOnTop()) {
            UpdatePaintNodeRender(focusFrameNode);
        }
    }
}

bool JsAccessibilityManager::SendRequestFocusToCandidate(const RefPtr<NG::FrameNode>& candidateNode)
{
    CHECK_NULL_RETURN(candidateNode, false);
    auto elementId = candidateNode->GetAccessibilityId();
    if (elementId < 0) {
        return false;
    }
    auto context = candidateNode->GetContextRefPtr();
    CHECK_NULL_RETURN(context, false);
    AccessibilityEvent event;
    event.type = AccessibilityEventType::REQUEST_FOCUS_FOR_ACCESSIBILITY_NOT_INTERRUPT;
    event.nodeId = elementId;
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
        "%{public}s Send a11y request-focus event to candidate nodeId:%{public}" PRId64,
        DETACH_FOCUS_LOG_PREFIX, elementId);
    context->SendEventToAccessibilityWithNode(event, candidateNode);
    return true;
}

namespace {
bool IsFocusNodeInsideScrollableContainer(NG::FrameNode* focusNode)
{
    CHECK_NULL_RETURN(focusNode, false);
    auto parent = focusNode->GetAncestorNodeOfFrame(true);
    while (parent) {
        auto accessibilityProperty = parent->GetAccessibilityProperty<NG::AccessibilityProperty>();
        if (accessibilityProperty && accessibilityProperty->IsScrollable()) {
            return true;
        }
        parent = parent->GetAncestorNodeOfFrame(true);
    }
    return false;
}
} // namespace

bool JsAccessibilityManager::IsDetachFocusCacheClearEvent(AccessibilityEventType eventType) const
{
    switch (eventType) {
        case AccessibilityEventType::PAGE_CHANGE:
        case AccessibilityEventType::CHANGE:
        case AccessibilityEventType::PAGE_OPEN:
        case AccessibilityEventType::PAGE_CLOSE:
        case AccessibilityEventType::PAGE_PRE_CHANGE:
        case AccessibilityEventType::FOCUS:
            return true;
        case AccessibilityEventType::SCROLLING_EVENT:
        case AccessibilityEventType::SCROLL_END:
        case AccessibilityEventType::SCROLL_START: {
            if (currentFocusNodeId_ == INVALID_NODE_ID) {
                return false;
            }
            auto focusNode = ElementRegister::GetInstance()->GetFrameNodePtrById(currentFocusNodeId_);
            if (IsFocusNodeInsideScrollableContainer(focusNode)) {
                return true;
            }
            return false;
        }
        default:
            return false;
    }
}

void JsAccessibilityManager::ClearDetachFocusFallbackCache(int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(detachFocusFallbackCacheMutex_);
    auto erased = detachFocusFallbackCache_.erase(instanceId);
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
        "%{public}s Clear detach-focus cache, instanceId:%{public}d, erased:%{public}zu",
        DETACH_FOCUS_LOG_PREFIX, instanceId, erased);
}

void JsAccessibilityManager::CacheDetachFocusFallbackCandidates(
    int32_t instanceId, int64_t focusNodeId, const std::vector<int64_t>& candidateIds)
{
    std::lock_guard<std::mutex> lock(detachFocusFallbackCacheMutex_);
    DetachFocusFallbackData data;
    data.focusNodeId = focusNodeId;
    data.candidateIds = candidateIds;
    data.treeId = treeId_;
    detachFocusFallbackCache_[instanceId] = std::move(data);
}

JsAccessibilityManager::DetachFocusFallbackData
JsAccessibilityManager::GetAndClearDetachFocusFallbackCandidates(int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(detachFocusFallbackCacheMutex_);
    auto it = detachFocusFallbackCache_.find(instanceId);
    if (it == detachFocusFallbackCache_.end()) {
        return {};
    }
    auto result = std::move(it->second);
    detachFocusFallbackCache_.erase(it);
    return result;
}

RefPtr<NG::FrameNode> JsAccessibilityManager::FindCandidateByFocusMove(
    const RefPtr<NG::FrameNode>& focusNode, int32_t direction)
{
    CHECK_NULL_RETURN(focusNode, nullptr);
    if (IsCmpSubAM()) {
        return FindDetachFocusCandidateFromRoot(focusNode, direction);
    }
    auto context = focusNode->GetContextRefPtr();
    CHECK_NULL_RETURN(context, nullptr);
    AccessibilityElementInfo nodeInfo;
    AccessibilityFocusMoveParam param {
        .direction = static_cast<OHOS::Accessibility::FocusMoveDirection>(direction),
        .condition = DetailCondition::BYPASS_SELF,
    };
    auto result = FocusMoveSearchWithConditionNG(focusNode->GetAccessibilityId(), param, context, nodeInfo);
    if (result.resultType != FocusMoveResultType::SEARCH_SUCCESS) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
            "%{public}s FocusMoveSearchWithConditionNG failed, source:%{public}" PRId64
            ", direction:%{public}d, result:%{public}d",
            DETACH_FOCUS_LOG_PREFIX, focusNode->GetAccessibilityId(), direction,
            static_cast<int32_t>(result.resultType));
        return nullptr;
    }
    if (nodeInfo.GetAccessibilityId() < 0) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
            "%{public}s FocusMoveSearchWithConditionNG invalid target, source:%{public}" PRId64
            ", direction:%{public}d",
            DETACH_FOCUS_LOG_PREFIX, focusNode->GetAccessibilityId(), direction);
        return nullptr;
    }
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
        "%{public}s FocusMoveSearchWithConditionNG candidate found, source:%{public}" PRId64
        ", direction:%{public}d, target:%{public}" PRId64,
        DETACH_FOCUS_LOG_PREFIX, focusNode->GetAccessibilityId(), direction, nodeInfo.GetAccessibilityId());
    auto rootNode = context->GetRootElement();
    CHECK_NULL_RETURN(rootNode, nullptr);
    return NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(rootNode, nodeInfo.GetAccessibilityId());
}

std::vector<int64_t> JsAccessibilityManager::QueryDetachFocusFallbackCandidates(
    const RefPtr<NG::FrameNode>& focusNode, const RefPtr<NG::PipelineContext>& context, size_t maxCount)
{
    std::vector<int64_t> candidateIds;
    CHECK_NULL_RETURN(focusNode, candidateIds);
    CHECK_NULL_RETURN(context, candidateIds);
    auto rootNode = IsCmpSubAM() ? cmpExportedRoot_.Upgrade() : context->GetRootElement();
    CHECK_NULL_RETURN(rootNode, candidateIds);
    std::unordered_set<int64_t> dedup;

    auto appendCandidate = [this, &candidateIds, &dedup, maxCount, focusNode](const RefPtr<NG::FrameNode>& node) {
        if (!node || candidateIds.size() >= maxCount) {
            return;
        }
        auto id = node->GetAccessibilityId();
        if (id <= 0 || id == focusNode->GetAccessibilityId() || dedup.count(id) > 0 ||
            !CheckAccessibilityVisible(node)) {
            return;
        }
        dedup.insert(id);
        candidateIds.emplace_back(id);
    };

    appendCandidate(GetNextFocusNodeByManager(focusNode, rootNode));
    auto relationManager = IsCmpSubAM()
        ? AceType::DynamicCast<JsAccessibilityManager>(context->GetAccessibilityManager()) : Claim(this);
    if (relationManager) {
        appendCandidate(relationManager->GetPrevFocusNodeByManager(focusNode, rootNode, context));
    }

    auto cursor = focusNode;
    while (candidateIds.size() < maxCount && cursor) {
        auto nextNode = FindCandidateByFocusMove(cursor, FocusMoveDirection::FORWARD);
        if (!nextNode) {
            break;
        }
        auto oldSize = candidateIds.size();
        appendCandidate(nextNode);
        if (candidateIds.size() == oldSize) {
            break;
        }
        cursor = nextNode;
    }
    return candidateIds;
}

void JsAccessibilityManager::HandleDetachFocusFallbackCallback(int32_t instanceId)
{
    std::vector<RefPtr<JsAccessibilityManager>> subManagers;
    {
        std::lock_guard<std::mutex> lock(cmpBindingMapMutex_);
        for (const auto& [key, binding] : cmpBindingMap_) {
            if (binding.state == CmpBindingState::State::REGISTERED && binding.subAM) {
                subManagers.emplace_back(binding.subAM);
            }
        }
    }
    for (const auto& subAM : subManagers) {
        subAM->HandleDetachFocusFallbackCallback(instanceId);
    }
    auto fallbackData = GetAndClearDetachFocusFallbackCandidates(instanceId);
    if (fallbackData.focusNodeId == INVALID_NODE_ID) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
            "%{public}s Detach focus fallback callback skipped, focusNodeId invalid. instanceId:%{public}d",
            DETACH_FOCUS_LOG_PREFIX, instanceId);
        return;
    }
    if (IsCmpSubAM() && (!IsRegister() || fallbackData.treeId != treeId_ || cmpExportedRoot_.Invalid())) {
        TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
            "%{public}s Drop stale fallback: instanceId=%{public}d, cachedTreeId=%{public}d, "
            "treeId=%{public}d, registered=%{public}d, rootExpired=%{public}d",
            DETACH_FOCUS_LOG_PREFIX, instanceId, fallbackData.treeId, treeId_, IsRegister(), cmpExportedRoot_.Invalid());
        return;
    }
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);

    int32_t windowId = IsCmpSubAM() ? static_cast<int32_t>(GetWindowId())
        : static_cast<int32_t>(ngPipeline->GetRealHostWindowId());
    auto eventInfo = BuildDetachFocusFallbackEvent(fallbackData, windowId);
    AccessibilityEvent accessibilityEvent;
    accessibilityEvent.type = AccessibilityEventType::FOCUS_INVISIBLE;
    accessibilityEvent.nodeId = fallbackData.focusNodeId;
    auto executor = context->GetTaskExecutor();
    CHECK_NULL_VOID(executor);
    executor->PostTask(
        [weak = WeakClaim(this), accessibilityEvent, eventInfo] {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            if (!jsAccessibilityManager->SendAccessibilitySyncEvent(accessibilityEvent, eventInfo)) {
                TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
                    "%{public}s FOCUS_INVISIBLE not sent, source:%{public}" PRId64,
                    DETACH_FOCUS_LOG_PREFIX, eventInfo.GetAccessibilityId());
            }
        },
        TaskExecutor::TaskType::BACKGROUND, "ArkUIA11yFocusInvisible");
}

AccessibilityEventInfo JsAccessibilityManager::BuildDetachFocusFallbackEvent(
    const DetachFocusFallbackData& fallbackData, int32_t windowId)
{
    AccessibilityEventInfo eventInfo;
    eventInfo.SetWindowId(windowId);
    eventInfo.SetEventType(ConvertAceEventType(AccessibilityEventType::FOCUS_INVISIBLE));
    auto sourceId = fallbackData.focusNodeId;
    if (fallbackData.treeId > 0) {
        AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(fallbackData.treeId, sourceId);
    }
    eventInfo.SetSource(sourceId);
    AccessibilityElementInfo elementInfo;
    elementInfo.SetAccessibilityId(sourceId);
    elementInfo.SetBelongTreeId(fallbackData.treeId);
    elementInfo.SetWindowId(windowId);
    eventInfo.SetElementInfo(elementInfo);
    eventInfo.SetTimeStamp(GetMicroTickCount());
    eventInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());

    ExtraEventInfo extraEventInfo;
    std::ostringstream candidateIdStr;
    for (size_t i = 0; i < fallbackData.candidateIds.size(); ++i) {
        if (i > 0) {
            candidateIdStr << ", ";
        }
        auto candidateId = fallbackData.candidateIds[i];
        if (fallbackData.treeId > 0) {
            AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(fallbackData.treeId, candidateId);
        }
        candidateIdStr << candidateId;
    }
    extraEventInfo.SetExtraEventInfo(CANDIDATE_IDS, candidateIdStr.str());
    eventInfo.SetExtraEvent(extraEventInfo);
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
        "%{public}s Prepare FOCUS_INVISIBLE event, treeId:%{public}d, focusNodeId:%{public}" PRId64
        ", candidateIds:[%{public}s]",
        DETACH_FOCUS_LOG_PREFIX, fallbackData.treeId, sourceId, candidateIdStr.str().c_str());
    return eventInfo;
}

void JsAccessibilityManager::NotifyAccessibilityFocusDetachOrInvisible(const RefPtr<NG::FrameNode>& focusNode)
{
    CHECK_NULL_VOID(focusNode);
    auto context = IsCmpSubAM() ? AceType::DynamicCast<NG::PipelineContext>(GetPipelineContext().Upgrade())
        : focusNode->GetContextRefPtr();
    CHECK_NULL_VOID(context);
    auto instanceId = context->GetInstanceId();
    auto candidateIds = QueryDetachFocusFallbackCandidates(focusNode, context, MAX_CANDIDATE_ID_NUMBER);
    CacheDetachFocusFallbackCandidates(instanceId, focusNode->GetAccessibilityId(), candidateIds);
    TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY,
        "%{public}s Fallback candidates cached, instanceId:%{public}d, treeId:%{public}d, "
        "focusNodeId:%{public}" PRId64 ", subAM:%{public}d, ids:[%{public}" PRId64 ", %{public}" PRId64
        ", %{public}" PRId64 "]",
        DETACH_FOCUS_LOG_PREFIX, instanceId, treeId_, focusNode->GetAccessibilityId(), IsCmpSubAM(),
        candidateIds.size() > 0 ? candidateIds[0] : static_cast<int64_t>(-1),
        candidateIds.size() > 1 ? candidateIds[1] : static_cast<int64_t>(-1),
        candidateIds.size() > 2 ? candidateIds[2] : static_cast<int64_t>(-1));
    if (IsCmpSubAM()) {
        auto executor = context->GetTaskExecutor();
        CHECK_NULL_VOID(executor);
        executor->PostTask(
            [weak = WeakClaim(this), instanceId] {
                auto manager = weak.Upgrade();
                CHECK_NULL_VOID(manager);
                ContainerScope scope(instanceId);
                manager->HandleDetachFocusFallbackCallback(instanceId);
            },
            TaskExecutor::TaskType::UI, "ArkUIA11yDetachFocusFallback");
        return;
    }
    context->AddAccessibilityCallbackEvent(
        AccessibilityCallbackEventId::ON_SEND_DETACH_FOCUS_FALLBACK, static_cast<int64_t>(instanceId));
}

void JsAccessibilityManager::OnAccessbibilityDetachFromMainTree(const RefPtr<NG::FrameNode>& focusNode)
{
    CHECK_NULL_VOID(focusNode);
    if (CmpAccessibility::CmpAccessibilityRegistry::HasAnySubAM()) {
        auto ownerAM = GetAccessibilityManagerForNode(focusNode);
        if (ownerAM && ownerAM.GetRawPtr() != this) {
            ownerAM->OnAccessbibilityDetachFromMainTree(focusNode);
            return;
        }
    }
    if (currentFocusNodeId_ != focusNode->GetAccessibilityId()) {
        return;
    }
    NotifyAccessibilityFocusDetachOrInvisible(focusNode);
    auto paintNode = focusNode->GetPaintNode();
    if (paintNode) {
        auto pattern = paintNode->GetPattern<NG::AccessibilityFocusPaintNodePattern>();
        if (pattern && pattern->GetFocusNode().Upgrade() == focusNode) {
            pattern->OnDetachFromFocusNode();
            RemoveAccessibilityFocusPaint(focusNode);
        }
    }
    ClearAccessibilityFocusState();
    currentFocusVirtualNodeParentId_ = INVALID_NODE_ID;
    auto property = focusNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    if (property) {
        property->SetAccessibilityFocusState(false);
    }
}

bool JsAccessibilityManager::CheckAccessibilityVisible(const RefPtr<NG::FrameNode>& node)
{
    NG::AccessibilityFrameNodeUtils::UpdateAccessibilityVisibleToRoot(node);
    CHECK_NULL_RETURN(node, true);
    return node->GetAccessibilityVisible();
}

bool JsAccessibilityManager::IsScreenReaderEnabled()
{
    if (!isScreenReaderEnabledInitialized_) {
        auto client = AccessibilitySystemAbilityClient::GetInstance();
        if (client) {
            client->IsScreenReaderEnabled(isScreenReaderEnabled_);
            isScreenReaderEnabledInitialized_ = true;
        }
    }
    return isScreenReaderEnabled_;
}

bool JsAccessibilityManager::IsSelectReaderEnabled()
{
    if (!isSelectReaderEnabledInitialized_) {
        auto client = AccessibilitySystemAbilityClient::GetInstance();
        if (client) {
            client->IsSelectReaderEnabled(isSelectReaderEnabled_);
            isSelectReaderEnabledInitialized_ = true;
        }
    }
    return isSelectReaderEnabled_;
}

SearchSurfaceIdRet JsAccessibilityManager::SearchElementInfoBySurfaceId(
    const std::string& surfaceId, const int32_t windowId,
    const SearchSurfaceIdType searchType, std::list<AccessibilityElementInfo>& infos)
{
    auto pipeline = GetPipelineByWindowId(windowId);
    if (pipeline == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
            "SearchBySurfaceId drop: no pipeline for windowId=%{public}d", windowId);
        return SearchSurfaceIdRet::NO_MATCH_NODE;
    }
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    CHECK_NULL_RETURN(ngPipeline, SearchSurfaceIdRet::NO_MATCH_NODE);

    auto node = GetEmbedNodeBySurfaceId(surfaceId).Upgrade();
    if (node == nullptr) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
            "SearchBySurfaceId drop: embed node not registered, surfaceId=%{public}s"
            ", windowId=%{public}d", surfaceId.c_str(), windowId);
        return SearchSurfaceIdRet::NO_MATCH_NODE;
    }

    if (searchType != SearchSurfaceIdType::SEARCH_TAIL) {
        auto mode = searchType == SearchSurfaceIdType::SEARCH_ALL ?
            static_cast<uint32_t>(PREFETCH_RECURSIVE_CHILDREN) : 0;
        SearchElementInfoByAccessibilityIdNG(
            node->GetAccessibilityId(), mode, infos, pipeline, NG::UI_EXTENSION_OFFSET_MAX);
        UpdateElementInfosTreeId(infos);
        return SearchSurfaceIdRet::SEARCH_SUCCESS;
    }

    auto mainContext = context_.Upgrade();
    CHECK_NULL_RETURN(mainContext, SearchSurfaceIdRet::NO_MATCH_NODE);
    CommonProperty commonProperty;
    GenerateCommonProperty(pipeline, commonProperty, mainContext, node);
    NG::AccessibilityFrameNodeUtils::UpdateAccessibilityVisibleToRoot(node);
    commonProperty.checkEmbedNode = false;
    auto lastNode = GetLastChildFrameNode(node, commonProperty);
    CHECK_NULL_RETURN(lastNode, SearchSurfaceIdRet::NO_MATCH_NODE);

    AccessibilityElementInfo elementInfo;
    UpdateAccessibilityElementInfo(lastNode, commonProperty, elementInfo, ngPipeline);
    SetRootAccessibilityVisible(lastNode, elementInfo);
    SetRootAccessibilityNextFocusId(lastNode, node, elementInfo);
    SetRootAccessibilityPreFocusId(
        lastNode, node, elementInfo, nextFocusRelationController_.GetTargetMap(pipeline->GetInstanceId()));
    infos.push_back(elementInfo);
    UpdateElementInfosTreeId(infos);
    return SearchSurfaceIdRet::SEARCH_SUCCESS;
}

WeakPtr<NG::WebPattern> JsAccessibilityManager::GetWebPatternBySurfaceId(const std::string& surfaceId)
{
    std::lock_guard<std::mutex> lock(webPatternMapMutex_);
    auto it = webPatternMap_.find(surfaceId);
    if (it != webPatternMap_.end()) {
        return it->second;
    }
    return WeakPtr<NG::WebPattern>();
}

void JsAccessibilityManager::SetWebPatternBySurfaceId(const std::string& surfaceId, WeakPtr<NG::WebPattern> pattern)
{
    std::lock_guard<std::mutex> lock(webPatternMapMutex_);
    webPatternMap_[surfaceId] = pattern;
}

void JsAccessibilityManager::RemoveWebPatternBySurfaceId(const std::string& surfaceId)
{
    if (!surfaceId.empty()) {
        std::lock_guard<std::mutex> lock(webPatternMapMutex_);
        auto it = webPatternMap_.find(surfaceId);
        if (it != webPatternMap_.end()) {
            webPatternMap_.erase(it);
        }
    }
}

void JsAccessibilityManager::SearchAccessibilityNodeBySpecificProperty(const int64_t elementId,
    const SpecificPropertyParam &param, const int32_t requestId,
    AccessibilityElementOperatorCallback &callback, const int32_t windowId)
{
    std::list<AccessibilityElementInfo> infos;
    std::list<AccessibilityElementInfo> treeInfos;

    if (param.propertyType != SEARCH_TYPE::CUSTOMID) {
        SetSearchElementInfoByCustomIdResult(callback, infos, treeInfos, requestId);
        return;
    }

    auto pipeline = GetPipelineByWindowId(windowId);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    if (!ngPipeline) {
        SetSearchElementInfoByCustomIdResult(callback, infos, treeInfos, requestId);
        return;
    }
    auto rootNode = ngPipeline->GetRootElement();

    int64_t nodeId = elementId;
    if (elementId == -1) {
        nodeId = rootNode->GetAccessibilityId();
    }
    auto weak = WeakClaim(this);
    auto jsAccessibilityManager = weak.Upgrade();
    CHECK_NULL_VOID(jsAccessibilityManager);
    auto node = jsAccessibilityManager->GetAccessibilityNodeFromPage(nodeId);
    if (!node) {
        SetSearchElementInfoByCustomIdResult(callback, infos, treeInfos, requestId);
        return;
    }
    std::list<AccessibilityElementInfo> nodeInfos;
    AccessibilityElementInfo nodeInfo;
    UpdateAccessibilityNodeInfo(node, nodeInfo, jsAccessibilityManager, jsAccessibilityManager->windowId_);
    nodeInfos.push_back(nodeInfo);
    // cache parent/siblings/children infos
    UpdateCacheInfo(
        infos,
        static_cast<uint32_t>(PREFETCH_RECURSIVE_CHILDREN),
        node,
        jsAccessibilityManager,
        jsAccessibilityManager->windowId_);

    for (auto& nodeInfo : nodeInfos) {
        if (nodeInfo.GetInspectorKey() == param.propertyTarget) {
            infos.emplace_back(nodeInfo);
            break;
        }
    }
    SetSearchElementInfoByCustomIdResult(callback, infos, treeInfos, requestId);
    return;
}

bool JsAccessibilityManager::CheckWhiteList(const uint32_t& eventType)
{
    auto whiteList = eventWhiteList_;
    if (whiteList.size() == 0) {
        return true;
    }

    if (whiteList.size() == 1) {
        if (whiteList[0] == Accessibility::EventType::TYPE_VIEW_INVALID) {
            return false;
        } else if (whiteList[0] == Accessibility::EventType::TYPES_ALL_MASK) {
            return true;
        }
    }
    return std::find(whiteList.begin(), whiteList.end(), eventType) != whiteList.end();
}

int32_t JsAccessibilityManager::GetTreeId(int32_t instanceId)
{
    if (instanceId == -1) {
        return treeId_;
    }
    auto container = Platform::AceContainer::GetContainer(instanceId);
    CHECK_NULL_RETURN(container, treeId_);

    if (container->IsSubWindow()) {
        return 0;
    }
    return treeId_;
}

void JsAccessibilityManager::AccessibilityOnShowHide(bool isOnShow,
    [[maybe_unused]] const WeakPtr<PipelineBase>& context)
{
    if (isOnShow) {
        CheckAndReConnectA11ySA();
    }
}

void JsAccessibilityManager::UpdateAccessibilityElementInfoForVirtualNode(
    const RefPtr<NG::FrameNode>& hostNode,
    const RefPtr<NG::VirtualAccessibilityNode>& virtualNode,
    const CommonProperty& commonProperty,
    AccessibilityElementInfo& nodeInfo,
    int64_t parentAccessibilityId)
{
    CHECK_NULL_VOID(hostNode);
    CHECK_NULL_VOID(virtualNode);

    int64_t virtualNodeAccessibilityId = NG::VirtualNodeContainerIdManager::EncodeVirtualNodeAccessibilityId(
        NG::VirtualNodeContainerIdManager::GetInstance().GetContainerId(hostNode), virtualNode->GetNodeId());
    ExtraElementInfo extraElementInfo {};
    nodeInfo.SetExtraElement(extraElementInfo);

    nodeInfo.SetAccessibilityId(virtualNodeAccessibilityId);
    nodeInfo.SetParent(parentAccessibilityId);
    nodeInfo.SetComponentType(virtualNode->GetRole());
    nodeInfo.SetAccessibilityText(virtualNode->GetAccessibilityText());
    nodeInfo.SetClickable(virtualNode->GetClickable());
    nodeInfo.SetAccessibilityLevel(virtualNode->GetAccessibilityLevel());
    nodeInfo.SetAccessibilityGroup(virtualNode->GetAccessibilityGroup());
    nodeInfo.SetCheckable(virtualNode->GetCheckable());
    nodeInfo.SetChecked(virtualNode->GetChecked());
    nodeInfo.SetEnabled(virtualNode->GetEnabled());
    nodeInfo.SetSelected(virtualNode->GetSelected());
    nodeInfo.SetPageId(hostNode->GetPageId());
    nodeInfo.SetWindowId(commonProperty.windowId);
    nodeInfo.SetInnerWindowId(commonProperty.innerWindowId);
    nodeInfo.SetBundleName(AceApplicationInfo::GetInstance().GetPackageName());
    nodeInfo.SetVisible(true); // visible set true for uitest to show
    nodeInfo.SetAccessibilityFocus(virtualNode->GetAccessibilityFocused());
    nodeInfo.SetContent(virtualNode->GetText());
    nodeInfo.SetVirtualSupportAction(virtualNode->GetSupportAction());

    int32_t left = virtualNode->GetLeft();
    int32_t top = virtualNode->GetTop();
    int32_t width = virtualNode->GetWidth();
    int32_t height = virtualNode->GetHeight();

    auto hostRect = hostNode->GetTransformRectRelativeToWindow();
    int32_t screenLeft = static_cast<int32_t>(hostRect.Left() + left + commonProperty.windowLeft);
    int32_t screenTop = static_cast<int32_t>(hostRect.Top() + top + commonProperty.windowTop);
    int32_t screenRight = static_cast<int32_t>(screenLeft + width);
    int32_t screenBottom = static_cast<int32_t>(screenTop + height);
    Accessibility::Rect bounds { screenLeft, screenTop, screenRight, screenBottom };
    nodeInfo.SetRectInScreen(bounds);

    const auto& virtualChildren = virtualNode->GetChildren();
    for (const auto& child : virtualChildren) {
        CHECK_NULL_CONTINUE(child);
        int64_t childAccessibilityId = NG::VirtualNodeContainerIdManager::EncodeVirtualNodeAccessibilityId(
            NG::VirtualNodeContainerIdManager::GetInstance().GetContainerId(hostNode), child->GetNodeId());
        nodeInfo.AddChild(childAccessibilityId);
    }

    nodeInfo.SetSourceType(AccessibilitySourceType::ADDED_FROM_ACCESSIBILITY_VIRTUAL_NODE);
}

void JsAccessibilityManager::AddVirtualNodeChildrenToInfos(
    std::list<AccessibilityElementInfo>& infos,
    const RefPtr<NG::FrameNode>& hostNode,
    const RefPtr<NG::VirtualAccessibilityNode>& virtualNode,
    const CommonProperty& commonProperty,
    int64_t virtualNodeAccessibilityId)
{
    CHECK_NULL_VOID(hostNode);
    CHECK_NULL_VOID(virtualNode);

    const auto& children = virtualNode->GetChildren();
    for (const auto& child : children) {
        CHECK_NULL_CONTINUE(child);
        AccessibilityElementInfo nodeInfo;
        UpdateAccessibilityElementInfoForVirtualNode(
            hostNode, child, commonProperty, nodeInfo, virtualNodeAccessibilityId);
        infos.push_back(nodeInfo);
        int64_t childAccessibilityId = NG::VirtualNodeContainerIdManager::EncodeVirtualNodeAccessibilityId(
            NG::VirtualNodeContainerIdManager::GetInstance().GetContainerId(hostNode), child->GetNodeId());
        AddVirtualNodeChildrenToInfos(infos, hostNode, child, commonProperty, childAccessibilityId);
    }
}

void JsAccessibilityManager::DumpGetCustomPropertyTest(int64_t nodeId)
{
    DumpLog::GetInstance().Print("DumpGetCustomPropertyTest start");
    DumpLog::GetInstance().Print("nodeId: " + std::to_string(nodeId));

    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);

    if (NG::VirtualNodeContainerIdManager::IsVirtualNodeContainerId(nodeId)) {
        RefPtr<NG::VirtualAccessibilityNode> virtualNode = nullptr;
        RefPtr<NG::FrameNode> parentNode = nullptr;
        auto find = CheckAndGetVirtualNodeTree(nodeId, virtualNode, parentNode);
        if (find && virtualNode) {
            DumpVirtualAccessibilityNodeTree(parentNode, virtualNode, 1);
        } else {
            DumpLog::GetInstance().Print("No virtual node found");
        }
        DumpLog::GetInstance().Print("Result: GetCustomProperty Done");
        return;
    }

    RefPtr<NG::FrameNode> frameNode;
    RefPtr<NG::PipelineContext> ngPipeline = FindPipelineByElementId(nodeId, frameNode);
    CHECK_NULL_VOID(ngPipeline);
    CHECK_NULL_VOID(frameNode);

    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);

    auto customProperty = accessibilityProperty->GetCustomAccessibilityProperty();
    if (customProperty) {
        DumpLog::GetInstance().Print("accessibilityText: " + customProperty->GetAccessibilityText());
        DumpLog::GetInstance().Print("accessibilityLevel: " + customProperty->GetAccessibilityLevel());
        DumpLog::GetInstance().Print("accessibilityRole: " + customProperty->GetAccessibilityRole());
        DumpLog::GetInstance().Print("role: " + customProperty->GetRole());
        DumpLog::GetInstance().Print("clickable: " + std::to_string(customProperty->GetClickable()));
        DumpLog::GetInstance().Print("checkable: " + std::to_string(customProperty->GetCheckable()));
        DumpLog::GetInstance().Print("checked: " + std::to_string(customProperty->GetChecked()));
        DumpLog::GetInstance().Print("enabled: " + std::to_string(customProperty->GetEnabled()));
        DumpLog::GetInstance().Print("selected: " + std::to_string(customProperty->GetSelected()));
        DumpLog::GetInstance().Print("accessbilityGroup: " + std::to_string(customProperty->GetAccessibilityGroup()));
    } else {
        DumpLog::GetInstance().Print("No custom property set");
    }
    DumpLog::GetInstance().Print("Result: GetCustomProperty Done");
}

bool IsNumParametersBelowDimension(int32_t argToEndNum, const std::string& arg)
{
    constexpr int32_t NUM_PARAMETERS_DIMENSION = 1;
    if (argToEndNum > NUM_PARAMETERS_DIMENSION) {
        return false;
    }

    std::string belowPrintStr = "Error: ";
    belowPrintStr.append(arg).append(" needs value");
    DumpLog::GetInstance().Print(belowPrintStr.c_str());
    return true;
}

bool CheckParamAndAdvanceValue(std::vector<std::string>::const_iterator& arg, const std::vector<std::string>& params)
{
    CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
        static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
    ++arg;
    return true;
}

bool JsAccessibilityManager::ParseSetCustomPropertyParams(std::vector<std::string>::const_iterator& arg,
        const std::vector<std::string>& params, DumpInfoArgument& argument)
{
    while (arg != params.end()) {
        ++arg;
        if (arg == params.end()) {
            break;
        }
        if (*arg == "--node-id") {
            CHECK_NE_RETURN(CheckParamAndAdvanceValue(arg, params), true, false);
            argument.nodeId = StringUtils::StringToLongInt(*arg);
        } else if (*arg == "--accessibility-text") {
            CHECK_NE_RETURN(CheckParamAndAdvanceValue(arg, params), true, false);
            argument.customAccessibilityText = *arg;
        } else if (*arg == "--accessibility-level") {
            CHECK_NE_RETURN(CheckParamAndAdvanceValue(arg, params), true, false);
            argument.customAccessibilityLevel = *arg;
        } else if (*arg == "--role") {
            CHECK_NE_RETURN(CheckParamAndAdvanceValue(arg, params), true, false);
            argument.customRole = *arg;
        } else if (*arg == "accessibility-role") {
            CHECK_NE_RETURN(CheckParamAndAdvanceValue(arg, params), true, false);
            argument.customAccessibilityRole = *arg;
        } else if (*arg == "--group") {
            CHECK_NE_RETURN(CheckParamAndAdvanceValue(arg, params), true, false);
            argument.customGroup = (StringUtils::StringToInt(*arg) != 0);
        } else if (*arg == "--checkable") {
            CHECK_NE_RETURN(CheckParamAndAdvanceValue(arg, params), true, false);
            argument.customCheckable = (StringUtils::StringToInt(*arg) != 0);
        } else if (*arg == "--checked") {
            CHECK_NE_RETURN(CheckParamAndAdvanceValue(arg, params), true, false);
            argument.customChecked = (StringUtils::StringToInt(*arg) != 0);
        } else if (*arg == "--enabled") {
            CHECK_NE_RETURN(CheckParamAndAdvanceValue(arg, params), true, false);
            argument.customEnabled = (StringUtils::StringToInt(*arg) != 0);
        } else if (*arg == "--selected") {
            CHECK_NE_RETURN(CheckParamAndAdvanceValue(arg, params), true, false);
            argument.customSelected = (StringUtils::StringToInt(*arg) != 0);
        } else if (*arg == "--clickable") {
            CHECK_NE_RETURN(CheckParamAndAdvanceValue(arg, params), true, false);
            argument.customClickable = (StringUtils::StringToInt(*arg) != 0);
        }
    }
    return true;
}

bool JsAccessibilityManager::ParseGetCustomPropertyParams(std::vector<std::string>::const_iterator& arg,
    const std::vector<std::string>& params, DumpInfoArgument& argument)
{
    while (arg != params.end()) {
        ++arg;
        if (arg == params.end()) {
            break;
        }
        if (*arg == "--node-id") {
            CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
                static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
            ++arg;
            argument.nodeId = StringUtils::StringToLongInt(*arg);
        }
    }
    return true;
}

namespace {
class DumpOperatorCallBackStub : public Accessibility::AccessibilityElementOperatorCallback {
public:
    ~DumpOperatorCallBackStub() = default;

    void SetSearchElementInfoByAccessibilityIdResult(const std::list<Accessibility::AccessibilityElementInfo>& infos,
        const int32_t requestId) override
    {
    }

    void SetSearchElementInfoByTextResult(const std::list<Accessibility::AccessibilityElementInfo>& infos,
        const int32_t requestId) override
    {
    }

    void SetSearchDefaultFocusByWindowIdResult(const std::list<Accessibility::AccessibilityElementInfo>& infos,
        const int32_t requestId) override
    {
    }

    void SetFindFocusedElementInfoResult(const Accessibility::AccessibilityElementInfo& info,
        const int32_t requestId) override
    {
    }

    void SetFocusMoveSearchResult(const Accessibility::AccessibilityElementInfo& info,
        const int32_t requestId) override
    {
    }

    void SetExecuteActionResult(const bool succeeded, const int32_t requestId) override
    {
        if (succeeded) {
            DumpLog::GetInstance().Print("Result: action execute success");
        } else {
            DumpLog::GetInstance().Print("Result: action execute fail");
        }
    }

    void SetCursorPositionResult(const int32_t cursorPosition, const int32_t requestId) override
    {
    }

    void SetSearchElementInfoBySpecificPropertyResult(const std::list<Accessibility::AccessibilityElementInfo>& infos,
        const std::list<Accessibility::AccessibilityElementInfo>& treeInfos, const int32_t requestId) override
    {
    }

    void SetFocusMoveSearchWithConditionResult(const std::list<AccessibilityElementInfo>& infos,
        const FocusMoveResult& result, const int32_t requestId) override
    {
    }

    void SetUpdateCustomAccessibilityPropertyResult(
        const OperateVirtualNodeResult result, const int32_t requestId) override
    {
    }

    void SetAddAccessibilityVirtualNodeResult(
        const OperateVirtualNodeResult result, const int32_t requestId) override
    {
    }

    void SetRemoveAccessibilityVirtualNodeResult(
        const OperateVirtualNodeResult result, const int32_t requestId) override
    {
    }
};

} // namespace

void JsAccessibilityManager::DumpSetCustomPropertyTest(const DumpInfoArgument& argument)
{
    DumpLog::GetInstance().Print("DumpSetCustomPropertyTest start");
    DumpLog::GetInstance().Print("nodeId: " + std::to_string(argument.nodeId));

    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);

    AccessibilityVirtualNode accessibilityVirtualNode;
    accessibilityVirtualNode.SetAccessibilityText(argument.customAccessibilityText);
    accessibilityVirtualNode.SetAccessibilityLevel(argument.customAccessibilityLevel);
    accessibilityVirtualNode.SetCustomComponentType(argument.customAccessibilityRole);
    accessibilityVirtualNode.SetCheckable(argument.customCheckable);
    accessibilityVirtualNode.SetChecked(argument.customChecked);
    accessibilityVirtualNode.SetEnabled(argument.customEnabled);
    accessibilityVirtualNode.SetSelected(argument.customSelected);
    accessibilityVirtualNode.SetAccessibilityGroup(argument.customGroup);
    accessibilityVirtualNode.SetClickable(argument.customClickable);

    DumpOperatorCallBackStub callback;
    UpdateAccessibilityElementInfo(argument.nodeId, accessibilityVirtualNode, 0, callback, pipeline->GetWindowId());

    DumpLog::GetInstance().Print("accessibilityText: " + argument.customAccessibilityText);
    DumpLog::GetInstance().Print("accessibilityLevel: " + argument.customAccessibilityLevel);
    DumpLog::GetInstance().Print("accessibilityRole: " + argument.customAccessibilityRole);
    DumpLog::GetInstance().Print("clickable: " + std::to_string(argument.customClickable));
    DumpLog::GetInstance().Print("checkable: " + std::to_string(argument.customCheckable));
    DumpLog::GetInstance().Print("checked: " + std::to_string(argument.customChecked));
    DumpLog::GetInstance().Print("enabled: " + std::to_string(argument.customEnabled));
    DumpLog::GetInstance().Print("selected: " + std::to_string(argument.customSelected));
    DumpLog::GetInstance().Print("accessibilityGroup: " + std::to_string(argument.customGroup));
    DumpLog::GetInstance().Print("Result: SetCustomProperty Done");
}

void JsAccessibilityManager::DumpAddVirtualNodeTest(const DumpInfoArgument& argument)
{
    DumpLog::GetInstance().Print("DumpAddVirtualNodeTest start");
    DumpLog::GetInstance().Print("nodeId: " + std::to_string(argument.nodeId));
    DumpLog::GetInstance().Print("virtualNodeId: " + std::to_string(argument.virtualNodeId));

    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);
    auto windowId = pipeline->GetWindowId();

    std::vector<Accessibility::AccessibilityVirtualNode> nodes;

    Accessibility::AccessibilityVirtualNode virtualNode;
    virtualNode.SetWindowId(windowId);
    virtualNode.SetId(argument.virtualNodeId);
    virtualNode.SetRect({
        argument.virtualNodeLeft,
        argument.virtualNodeTop,
        argument.virtualNodeWidth + argument.virtualNodeLeft,
        argument.virtualNodeHeight + argument.virtualNodeTop
    });
    virtualNode.SetAccessibilityText(argument.customAccessibilityText);
    virtualNode.SetAccessibilityLevel(argument.customAccessibilityLevel);
    virtualNode.SetAccessibilityGroup(argument.customGroup);
    virtualNode.SetCustomComponentType(argument.customRole);
    virtualNode.SetCheckable(argument.customCheckable);
    virtualNode.SetChecked(argument.customChecked);
    virtualNode.SetEnabled(argument.customEnabled);
    virtualNode.SetSelected(argument.customSelected);
    virtualNode.SetClickable(argument.customClickable);
    nodes.push_back(virtualNode);

    DumpOperatorCallBackStub callback;
    AddAccessibilityVirtualNode(argument.nodeId, nodes, 0, callback, windowId);

    DumpLog::GetInstance().Print("windowId: " + std::to_string(windowId));
    DumpLog::GetInstance().Print("accessibilityText: " + argument.customAccessibilityText);
    DumpLog::GetInstance().Print("accessibilityLevel: " + argument.customAccessibilityLevel);
    DumpLog::GetInstance().Print("accessibilityRole: " + argument.customAccessibilityRole);
    DumpLog::GetInstance().Print("role: " + argument.customRole);
    DumpLog::GetInstance().Print("accessibilityGroup: " + std::to_string(argument.customGroup));
    DumpLog::GetInstance().Print("clickable: " + std::to_string(argument.customClickable));
    DumpLog::GetInstance().Print("checkable: " + std::to_string(argument.customCheckable));
    DumpLog::GetInstance().Print("checked: " + std::to_string(argument.customChecked));
    DumpLog::GetInstance().Print("enabled: " + std::to_string(argument.customEnabled));
    DumpLog::GetInstance().Print("selected: " + std::to_string(argument.customSelected));
    DumpLog::GetInstance().Print("Result: Add Virtual Node Done");
}

void JsAccessibilityManager::DumpRemoveVirtualNodeTest(const DumpInfoArgument& argument)
{
    DumpLog::GetInstance().Print("DumpRemoveVirtualNodeTest start");
    DumpLog::GetInstance().Print("nodeId: " + std::to_string(argument.nodeId));
    DumpLog::GetInstance().Print("virtualNodeId: " + std::to_string(argument.virtualNodeId));

    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);

    RefPtr<NG::FrameNode> frameNode;
    RefPtr<NG::PipelineContext> ngPipeline = FindPipelineByElementId(argument.nodeId, frameNode);
    CHECK_NULL_VOID(ngPipeline);
    CHECK_NULL_VOID(frameNode);

    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);

    auto renderContext = frameNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->UpdateAccessibilityFocus(false);

    accessibilityProperty->SetVirtualNodeTreeRoot(nullptr);
    NG::VirtualNodeContainerIdManager::GetInstance().ReleaseContainerId(frameNode);

    DumpLog::GetInstance().Print("Virtual node removed successfully");
    DumpLog::GetInstance().Print("Result: Remove Virtual Node Done");
}

void JsAccessibilityManager::DumpGetVirtualNodeTest(const DumpInfoArgument& argument)
{
    DumpLog::GetInstance().Print("DumpGetVirtualNodeTest start");
    DumpLog::GetInstance().Print("nodeId: " + std::to_string(argument.nodeId));
    DumpLog::GetInstance().Print("virtualNodeId: " + std::to_string(argument.virtualNodeId));

    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);

    RefPtr<NG::FrameNode> frameNode;
    RefPtr<NG::PipelineContext> ngPipeline = FindPipelineByElementId(argument.nodeId, frameNode);
    CHECK_NULL_VOID(ngPipeline);
    CHECK_NULL_VOID(frameNode);

    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    auto virtualRootBase = accessibilityProperty->GetVirtualNodeTreeRoot();
    auto virtualRoot = AceType::DynamicCast<NG::VirtualAccessibilityNode>(virtualRootBase);
    if (virtualRoot) {
        DumpLog::GetInstance().Print("Virtual node found");
        DumpLog::GetInstance().Print("nodeId: " + std::to_string(virtualRoot->GetNodeId()));
        auto rect = virtualRoot->GetRect();
        DumpLog::GetInstance().Print("rect: [" + std::to_string(rect.Left()) + ", " +
            std::to_string(rect.Top()) + ", " +
            std::to_string(rect.Width()) + ", " +
            std::to_string(rect.Height()) + "]");
        DumpLog::GetInstance().Print("accessibilityText: " + virtualRoot->GetAccessibilityText());
        DumpLog::GetInstance().Print("childCount: " + std::to_string(virtualRoot->GetChildCount()));
    } else {
        DumpLog::GetInstance().Print("No virtual node found");
    }
    DumpLog::GetInstance().Print("Result: GetVirtual Node Done");
}

bool ParseVirtualRectParams(std::vector<std::string>::const_iterator& arg,
    const std::vector<std::string>& params, DumpInfoArgument& argument)
{
    DumpLog::GetInstance().Print("--virtualRect");
    ++arg;
    CHECK_EQUAL_RETURN(arg == params.end(), true, false);
    DumpLog::GetInstance().Print("virtualNodeLeft: " + *arg);
    argument.virtualNodeLeft = StringUtils::StringToInt(*arg);
    ++arg;
    CHECK_EQUAL_RETURN(arg == params.end(), true, false);
    DumpLog::GetInstance().Print("virtualNodeTop: " + *arg);
    argument.virtualNodeTop = StringUtils::StringToInt(*arg);
    ++arg;
    CHECK_EQUAL_RETURN(arg == params.end(), true, false);
    DumpLog::GetInstance().Print("virtualNodeWidth: " + *arg);
    argument.virtualNodeWidth = StringUtils::StringToInt(*arg);
    ++arg;
    CHECK_EQUAL_RETURN(arg == params.end(), true, false);
    DumpLog::GetInstance().Print("virtualNodeHeight: " + *arg);
    argument.virtualNodeHeight = StringUtils::StringToInt(*arg);
    return true;
}

bool JsAccessibilityManager::ParseAddVirtualNodeParams(std::vector<std::string>::const_iterator& arg,
        const std::vector<std::string>& params, DumpInfoArgument& argument)
{
    while (arg != params.end()) {
        ++arg;
        if (arg == params.end()) {
            break;
        }
        DumpLog::GetInstance().Print("now arg: " + *arg);
        if (*arg == "--node-id"){
            CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
                static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
            ++arg;
            argument.nodeId = StringUtils::StringToLongInt(*arg);
        } else if (*arg == "--virtual-node-id") {
            CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
                static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
            ++arg;
            argument.virtualNodeId = StringUtils::StringToLongInt(*arg);
        } else if (*arg == "--virtual-rect") {
            if (!ParseVirtualRectParams(arg, params, argument)) {
                return false;
            }
        } else if (*arg == "--text") {
            CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
                static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
            ++arg;
            argument.customText = *arg;
        } else if (*arg == "--accessibility-text") {
            CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
                static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
            ++arg;
            argument.customAccessibilityText = *arg;
        } else if (*arg == "--accessibility-level") {
            CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
                static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
            ++arg;
            argument.customAccessibilityLevel = *arg;
        } else if (*arg == "--role") {
            CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
                static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
            ++arg;
            argument.customRole = *arg;
        } else if (*arg == "--group") {
            CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
                static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
            ++arg;
            argument.customGroup = (StringUtils::StringToInt(*arg) != 0);
        }
    }
    return true;
}

bool JsAccessibilityManager::ParseRemoveVirtualNodeParams(std::vector<std::string>::const_iterator& arg,
    const std::vector<std::string>& params, DumpInfoArgument& argument)
{
    while (arg != params.end()) {
        ++arg;
        if (arg == params.end()) {
            break;
        }
        if (*arg == "--node-id"){
            CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
                static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
            ++arg;
            argument.nodeId = StringUtils::StringToLongInt(*arg);
        } else if (*arg == "--virtual-node-id"){
            CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
                static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
            ++arg;
            argument.virtualNodeId = StringUtils::StringToInt(*arg);
        }
    }
    return true;
}

bool JsAccessibilityManager::ParseGetVirtualNodeParams(std::vector<std::string>::const_iterator& arg,
    const std::vector<std::string>& params, DumpInfoArgument& argument)
{
    while (arg != params.end()) {
        ++arg;
        if (arg == params.end()) {
            break;
        }
        if (*arg == "--node-id"){
            CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
                static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
            ++arg;
            argument.nodeId = StringUtils::StringToLongInt(*arg);
        } else if (*arg == "--virtual-node-id"){
            CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
                static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
            ++arg;
            argument.virtualNodeId = StringUtils::StringToInt(*arg);
        }
    }
    return true;
}

bool JsAccessibilityManager::ParsePerformVirtualNodeActionParams(std::vector<std::string>::const_iterator& arg,
    const std::vector<std::string>& params, DumpInfoArgument& argument)
{
    while (arg != params.end()) {
        ++arg;
        if (arg == params.end()) {
            break;
        }
        if (*arg == "--node-id") {
            CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
                static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
            ++arg;
            argument.nodeId = StringUtils::StringToLongInt(*arg);
        } else if (*arg == "--virtual-node-id") {
            CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
                static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
            ++arg;
            argument.virtualNodeId = StringUtils::StringToLongInt(*arg);
        } else if (*arg == "--action") {
            CHECK_EQUAL_RETURN(IsNumParametersBelowDimension(
                static_cast<int32_t>(std::distance(arg, params.end())), *arg), true, false);
            ++arg;
            argument.performAction = StringUtils::StringToLongInt(*arg);
        }
    }
    return true;
}

void JsAccessibilityManager::DumpPerformVirtualNodeActionTest(const DumpInfoArgument& argument)
{
    DumpLog::GetInstance().Print("DumpPerformVirtualNodeActionTest start");
    DumpLog::GetInstance().Print("nodeId: " + std::to_string(argument.nodeId));
    DumpLog::GetInstance().Print("virtualNodeId: " + std::to_string(argument.virtualNodeId));
    DumpLog::GetInstance().Print("action: " + std::to_string(argument.performAction));

    auto pipeline = context_.Upgrade();
    CHECK_NULL_VOID(pipeline);

    RefPtr<NG::FrameNode> frameNode;
    RefPtr<NG::PipelineContext> ngPipeline = FindPipelineByElementId(argument.nodeId, frameNode);
    CHECK_NULL_VOID(ngPipeline);
    CHECK_NULL_VOID(frameNode);
    DumpLog::GetInstance().Print("DumpPerformVirtualNodeActionTest 0");

    auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    DumpLog::GetInstance().Print("DumpPerformVirtualNodeActionTest 1");

    auto virtualRoot = accessibilityProperty->GetVirtualNodeTreeRoot();
    CHECK_NULL_VOID(virtualRoot);
    DumpLog::GetInstance().Print("DumpPerformVirtualNodeActionTest 2");

    auto virtualNode = AceType::DynamicCast<NG::VirtualAccessibilityNode>(virtualRoot);
    CHECK_NULL_VOID(virtualNode);
    DumpLog::GetInstance().Print("DumpPerformVirtualNodeActionTest 3");

    if (argument.performAction == static_cast<int32_t>(ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS)) {
        DumpLog::GetInstance().Print("Performing accessibility focus action on virtual node");
        auto parentFrameNode = frameNode;
        bool isNeedClear = false;
        bool result = ActAccessibilityVirtualFocus(argument.nodeId, parentFrameNode, virtualNode, isNeedClear);
        DumpLog::GetInstance().Print("Action result: " + std::string(result ? "success" : "failed"));
    } else {
        DumpLog::GetInstance().Print("Unsupported action: " + std::to_string(argument.performAction));
    }

    DumpLog::GetInstance().Print("Result: Perform Virtual Node Action Done");
}

void JsAccessibilityManager::DumpVirtualAccessibilityNodeTree(const RefPtr<NG::FrameNode>& hostNode,
    const RefPtr<NG::VirtualAccessibilityNode>& virtualNode, int32_t depth)
{
    CHECK_NULL_VOID(virtualNode);

    DumpLog::GetInstance().AddDesc("VirtualNode");
    DumpLog::GetInstance().AddDesc("nodeId: ", std::to_string(virtualNode->GetNodeId()));
    DumpLog::GetInstance().AddDesc("accessibilityText: ", virtualNode->GetAccessibilityText());
    DumpLog::GetInstance().AddDesc("accessibilityLevel: ", virtualNode->GetAccessibilityLevel());
    DumpLog::GetInstance().AddDesc("accessibilityGroup: ", std::to_string(virtualNode->GetAccessibilityGroup()));
    DumpLog::GetInstance().AddDesc("role: ", virtualNode->GetRole());
    DumpLog::GetInstance().AddDesc("checkable: ", std::to_string(virtualNode->GetCheckable()));
    DumpLog::GetInstance().AddDesc("checked: ", std::to_string(virtualNode->GetChecked()));
    DumpLog::GetInstance().AddDesc("isEnable: ", std::to_string(virtualNode->GetEnabled()));
    DumpLog::GetInstance().AddDesc("isSelected: ", std::to_string(virtualNode->GetSelected()));
    DumpLog::GetInstance().AddDesc("rect: [",
        std::to_string(virtualNode->GetLeft()), ", ",
        std::to_string(virtualNode->GetTop()), ", ",
        std::to_string(virtualNode->GetWidth()), ", ",
        std::to_string(virtualNode->GetHeight()), "]");
    DumpLog::GetInstance().AddDesc("childCount: ", std::to_string(virtualNode->GetChildCount()));
    const auto& children = virtualNode->GetChildren();
    DumpLog::GetInstance().Print(depth, "VirtualAccessibilityNode", children.size());
    for (const auto& child : children) {
        if (child) {
            DumpVirtualAccessibilityNodeTree(hostNode, child, depth + 1);
        }
    }
}

void JsAccessibilityManager::UpdateAccessibilityElementInfo(const int64_t elementId,
    const AccessibilityVirtualNode& accessibilityVirtualNode, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback, const int32_t windowId)
{
    auto pipeline = GetPipelineByWindowId(windowId);
    CHECK_NULL_VOID_WITH_ACTION(pipeline, SetUpdateAccessibilityElementInfoFailResult(callback, requestId));

    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(pipeline);
    CHECK_NULL_VOID_WITH_ACTION(ngPipeline, SetUpdateAccessibilityElementInfoFailResult(callback, requestId));

    ContainerScope instance(ngPipeline->GetInstanceId());
    auto rootNode = ngPipeline->GetRootElement();
    CHECK_NULL_VOID_WITH_ACTION(rootNode, SetUpdateAccessibilityElementInfoFailResult(callback, requestId));

    int64_t nodeId = (elementId == -1) ? rootNode->GetAccessibilityId() : elementId;
    if (NG::VirtualNodeContainerIdManager::IsVirtualNodeContainerId(nodeId)) {
        RefPtr<NG::VirtualAccessibilityNode> virtualNode;
        RefPtr<NG::FrameNode> parentNode;
        auto find = CheckAndGetVirtualNode(nodeId, virtualNode, parentNode);
        if (find && virtualNode) {
            virtualNode->SetAccessibilityText(accessibilityVirtualNode.GetAccessibilityText());
            virtualNode->SetAccessibilityLevel(accessibilityVirtualNode.GetAccessibilityLevel());
            virtualNode->SetRole(accessibilityVirtualNode.GetCustomComponentType());
            virtualNode->SetCheckable(accessibilityVirtualNode.GetCheckable());
            virtualNode->SetChecked(accessibilityVirtualNode.GetChecked());
            virtualNode->SetEnabled(accessibilityVirtualNode.GetEnabled());
            virtualNode->SetSelected(accessibilityVirtualNode.GetSelected());
            virtualNode->SetAccessibilityGroup(accessibilityVirtualNode.GetAccessibilityGroup());
            virtualNode->SetClickable(accessibilityVirtualNode.GetClickable());
            callback.SetUpdateCustomAccessibilityPropertyResult(
                Accessibility::OperateVirtualNodeResult::SUCCESS, requestId);
            return;
        }
    }

    auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(rootNode, nodeId);
    CHECK_NULL_VOID_WITH_ACTION(node,
        SetUpdateAccessibilityElementInfoFailResult(callback, requestId,
            Accessibility::OperateVirtualNodeResult::ACCESSIBILITY_ELEMENT_NOT_EXIST));

    auto accessibilityProperty = node->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID_WITH_ACTION(accessibilityProperty,
        SetUpdateAccessibilityElementInfoFailResult(callback, requestId,
            Accessibility::OperateVirtualNodeResult::ACCESSIBILITY_ELEMENT_NOT_EXIST));

    auto customProperty = AceType::MakeRefPtr<NG::CustomAccessibilityProperty>();
    customProperty->SetAccessibilityText(accessibilityVirtualNode.GetAccessibilityText());
    customProperty->SetAccessibilityLevel(accessibilityVirtualNode.GetAccessibilityLevel());
    customProperty->SetRole(accessibilityVirtualNode.GetCustomComponentType());
    customProperty->SetCheckable(accessibilityVirtualNode.GetCheckable());
    customProperty->SetChecked(accessibilityVirtualNode.GetChecked());
    customProperty->SetEnabled(accessibilityVirtualNode.GetEnabled());
    customProperty->SetSelected(accessibilityVirtualNode.GetSelected());
    customProperty->SetAccessibilityGroup(accessibilityVirtualNode.GetAccessibilityGroup());
    customProperty->SetClickable(accessibilityVirtualNode.GetClickable());
    accessibilityProperty->SetCustomAccessibilityProperty(customProperty);
    callback.SetUpdateCustomAccessibilityPropertyResult(
        Accessibility::OperateVirtualNodeResult::SUCCESS, requestId);
}

void JsAccessibilityManager::FillEventInfoWithVirtualNode(const RefPtr<NG::FrameNode>& parentNode,
    const RefPtr<NG::VirtualAccessibilityNode>& virtualNode, AccessibilityEventInfo& eventInfo,
    const RefPtr<NG::PipelineContext>& context, int64_t elementId)
{
    CHECK_NULL_VOID(parentNode);
    CHECK_NULL_VOID(virtualNode);
    eventInfo.SetComponentType(virtualNode->GetRole());
    eventInfo.SetPageId(parentNode->GetPageId());

    auto accessibilityProperty = parentNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
    CHECK_NULL_VOID(accessibilityProperty);
    eventInfo.SetItemCounts(accessibilityProperty->GetCollectionItemCounts());
    eventInfo.SetBeginIndex(accessibilityProperty->GetBeginIndex());
    eventInfo.SetEndIndex(accessibilityProperty->GetEndIndex());

    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    AccessibilityElementInfo elementInfo;
    CommonProperty commonProperty;
    GenerateCommonProperty(context, commonProperty, mainContext, parentNode);
    UpdateAccessibilityElementInfoForVirtualNode(
        parentNode, virtualNode, commonProperty, elementInfo, parentNode->GetAccessibilityId());
    elementInfo.SetWindowId(eventInfo.GetWindowId());
    eventInfo.AddContent(elementInfo.GetContent());
    eventInfo.SetElementInfo(elementInfo);
}

void JsAccessibilityManager::SendEventToAccessibilityWithVirtualNode(const AccessibilityEvent& accessibilityEvent,
    const RefPtr<NG::FrameNode>& parentNode, const RefPtr<NG::VirtualAccessibilityNode>& virtualNode,
    const RefPtr<PipelineBase>& context)
{
    ACE_SCOPED_TRACE("SendAccessibilityEventWithVirtualNode");
    CHECK_NULL_VOID(parentNode);
    CHECK_NULL_VOID(virtualNode);
    CHECK_NULL_VOID(context);

    int32_t windowId = static_cast<int32_t>(context->GetRealHostWindowId());
    if (windowId <= 0) {
        return;
    }

    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);

    if ((!parentNode->IsActive()) || parentNode->CheckAccessibilityLevelNo()) {
        TAG_LOGD(AceLogTag::ACE_ACCESSIBILITY, "parent node: %{public}" PRId64 ", is not active or level is no",
            parentNode->GetAccessibilityId());
        return;
    }

    AccessibilityEventInfo eventInfo;
    if ((accessibilityEvent.type != AccessibilityEventType::PAGE_CHANGE) || (accessibilityEvent.windowId == 0)) {
        eventInfo.SetWindowId(windowId);
    } else {
        eventInfo.SetWindowId(accessibilityEvent.windowId);
    }

    FillEventInfoWithVirtualNode(parentNode, virtualNode, eventInfo, ngPipeline, accessibilityEvent.nodeId);
    if (ngPipeline && ngPipeline->IsFormRender()) {
        eventInfo.SetWindowId(static_cast<int32_t>(GetWindowId()));
    }

    GenerateAccessibilityEventInfo(accessibilityEvent, eventInfo);
    CompensateCmpEventRect(eventInfo);

    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), accessibilityEvent, eventInfo] {
            auto jsAccessibilityManager = weak.Upgrade();
            CHECK_NULL_VOID(jsAccessibilityManager);
            jsAccessibilityManager->SendAccessibilitySyncEvent(accessibilityEvent, eventInfo);
        },
        TaskExecutor::TaskType::BACKGROUND, "ArkUIAccessibilitySendSyncEventWithVirtualNode");
}

// ============================================================================
// CMP mixed-rendering: subAM lifecycle.
// ============================================================================

} // namespace OHOS::Ace::Framework

namespace OHOS::Ace::CmpAccessibility {
bool CmpAccessibilityRegistry::BindExportTree(NG::FrameNode* owner, int64_t bindId, NG::FrameNode* exportedRoot)
{
    using namespace OHOS::Ace::Framework;
    CHECK_NULL_RETURN(owner, false);
    CHECK_NULL_RETURN(exportedRoot, false);
    auto mainAMBase = GetMainAccessibilityManager(owner);
    auto mainAM = OHOS::Ace::AceType::DynamicCast<JsAccessibilityManager>(mainAMBase);
    CHECK_NULL_RETURN(mainAM, false);
    // subAM is created inside BindExportTree (binding lifecycle, not adoption lifecycle).
    return mainAM->BindExportTree(owner, bindId, exportedRoot);
}

bool CmpAccessibilityRegistry::UnbindExportTree(NG::FrameNode* owner, int64_t bindId)
{
    using namespace OHOS::Ace::Framework;
    CHECK_NULL_RETURN(owner, false);
    auto mainAMBase = GetMainAccessibilityManager(owner);
    auto mainAM = OHOS::Ace::AceType::DynamicCast<JsAccessibilityManager>(mainAMBase);
    CHECK_NULL_RETURN(mainAM, false);
    mainAM->UnbindExportTree(owner, bindId);
    return true;
}
} // namespace OHOS::Ace::CmpAccessibility

namespace OHOS::Ace::Framework {
// CMP mixed-rendering: CMP third-party framework subtree registered (treeId obtained).
void JsAccessibilityManager::OnCmpTreeRegistered(NG::FrameNode* owner, int32_t treeId)
{
    CHECK_NULL_VOID(owner);
    CHECK_EQUAL_VOID(treeId <= 0, true);
    std::set<int32_t> staleTreeIds;
    {
        std::lock_guard<std::mutex> lock(cmpBindingMapMutex_);
        for (const auto& [key, state] : cmpBindingMap_) {
            if (key.first == owner->GetId() && state.cmpTreeId > 0 && state.cmpTreeId != treeId) {
                staleTreeIds.insert(state.cmpTreeId);
            }
        }
    }
    for (auto staleTreeId : staleTreeIds) {
        OnCmpTreeDeregistered(owner, staleTreeId);
    }
    std::vector<std::pair<int32_t, int64_t>> readyKeys;
    {
        std::lock_guard<std::mutex> lock(cmpBindingMapMutex_);
        for (auto& [key, state] : cmpBindingMap_) {
            if (state.state != CmpBindingState::State::WAIT_TREE_ID) {
                continue;
            }
            auto stateOwner = state.owner.Upgrade();
            if (!stateOwner || stateOwner->GetId() != owner->GetId()) {
                continue;
            }
            state.cmpTreeId = treeId;
            state.state = CmpBindingState::State::REGISTERING;
            readyKeys.push_back(key);
        }
    }
    // Migration + AAMS registration outside the lock (OH-ARCH-LAYERING: no IPC inside mutex).
    for (const auto& key : readyKeys) {
        RegisterCmpSubtreeToAams(key);
    }
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "OnCmpTreeRegistered: owner=%{public}d, treeId=%{public}d, converged=%{public}zu",
        owner->GetId(), treeId, readyKeys.size());
}

void JsAccessibilityManager::OnCmpTreeDeregistered(NG::FrameNode* owner, int32_t treeId)
{
    CHECK_NULL_VOID(owner);
    CHECK_EQUAL_VOID(treeId <= 0, true);
    std::vector<std::pair<int64_t, CmpBindingState>> suspended;
    {
        std::lock_guard<std::mutex> lock(cmpBindingMapMutex_);
        for (auto& [key, state] : cmpBindingMap_) {
            if (key.first != owner->GetId() || state.cmpTreeId != treeId) {
                continue;
            }
            suspended.emplace_back(key.second, state);
            state.state = CmpBindingState::State::WAIT_TREE_ID;
            state.registration.reset();
            state.cmpTreeId = -1;
            state.windowId = 0;
        }
    }
    for (const auto& [bindId, state] : suspended) {
        auto subAM = state.subAM;
        if (subAM) {
            subAM->Register(false);
            auto provider = subAM->cmpProviderOperation_.lock();
            if (provider) {
                provider->RemoveChildTreeIdAndWinId(bindId);
            }
            subAM->cmpProviderOperation_.reset();
            subAM->treeId_ = 0;
            subAM->parentTreeId_ = 0;
            subAM->parentElementId_ = INVALID_PARENT_ID;
            subAM->parentWindowId_ = 0;
            std::unordered_map<int64_t, std::shared_ptr<AccessibilityChildTreeCallback>> callbacks;
            {
                std::lock_guard<std::mutex> lock(subAM->childTreeCallbackMapMutex_);
                callbacks = subAM->childTreeCallbackMap_;
            }
            for (const auto& [elementId, callback] : callbacks) {
                if (callback) {
                    callback->OnDeregister();
                    callback->OnClearRegisterFlag();
                    callback->SetChildTreeId(0);
                    callback->OnSetChildTree(0, 0);
                }
            }
        }
        if (state.registration) {
            state.registration->Cancel();
        }
    }
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "OnCmpTreeDeregistered: owner=%{public}d, treeId=%{public}d, suspended=%{public}zu",
        owner->GetId(), treeId, suspended.size());
}

// Query the CMP treeId through the third-provider operator registered for this host.
int32_t JsAccessibilityManager::GetThirdProviderTreeIdForHost(NG::FrameNode* owner)
{
    CHECK_NULL_RETURN(owner, AccessibilityElementInfo::UNDEFINED_TREE_ID);
    auto& weakOp = GetJsThirdProviderInteractionOperation(owner->GetAccessibilityId());
    auto op = weakOp.lock();
    if (op) {
        return op->GetBelongTreeId();
    }
    return AccessibilityElementInfo::UNDEFINED_TREE_ID;
}

// Migrate child-tree callbacks of four-type nodes (Web/XComponent/Custom/UEC) that initialized before Bind and.
void JsAccessibilityManager::MigrateChildTreeCallbacksToSubAM(
    const RefPtr<NG::FrameNode>& exportedRoot, const RefPtr<JsAccessibilityManager>& subAM)
{
    std::unordered_map<int64_t, std::shared_ptr<AccessibilityChildTreeCallback>> snapshot;
    {
        std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
        snapshot = childTreeCallbackMap_;
        for (auto it = detachedCmpChildTreeCallbacks_.begin(); it != detachedCmpChildTreeCallbacks_.end();) {
            auto callback = it->second.lock();
            if (!callback) {
                it = detachedCmpChildTreeCallbacks_.erase(it);
            } else {
                snapshot.emplace(it->first, std::move(callback));
                ++it;
            }
        }
    }
    for (const auto& [elementId, callback] : snapshot) {
        if (!callback) {
            continue;
        }
        // Global accessibilityId is unique: found inside the exported subtree => belongs here.
        auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(exportedRoot, elementId);
        if (!node) {
            continue;
        }
        // Deregister the grandchild tree if it was already registered (idempotent per callback).
        callback->OnDeregister();
        callback->SetChildTreeId(0);
        DeregisterAccessibilityChildTreeCallback(elementId);
        subAM->RegisterAccessibilityChildTreeCallback(elementId, callback);
        TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
            "MigrateChildTreeCallback: elementId=%{public}" PRId64 " mainAM->subAM", elementId);
    }
}

// Snapshot of one REGISTERING binding (anchors and CMP treeId) under the map lock.
bool JsAccessibilityManager::SnapshotRegisteringCmpBinding(const std::pair<int32_t, int64_t>& key,
    RefPtr<JsAccessibilityManager>& subAM, WeakPtr<NG::FrameNode>& exportedRootWeak,
    WeakPtr<NG::FrameNode>& ownerWeak, int32_t& cmpTreeId)
{
    std::lock_guard<std::mutex> lock(cmpBindingMapMutex_);
    auto it = cmpBindingMap_.find(key);
    if (it == cmpBindingMap_.end() || it->second.state != CmpBindingState::State::REGISTERING) {
        return false;
    }
    subAM = it->second.subAM;
    exportedRootWeak = it->second.exportedRoot;
    ownerWeak = it->second.owner;
    cmpTreeId = it->second.cmpTreeId;
    return true;
}

void JsAccessibilityManager::CmpRegistration::SetTreeId(int32_t id)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        treeId = id;
    }
    CleanupIfCanceled();
}

void JsAccessibilityManager::CmpRegistration::Complete(bool accepted)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        finished = true;
        canceled = canceled || !accepted;
    }
    CleanupIfCanceled();
}

void JsAccessibilityManager::CmpRegistration::Cancel()
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        canceled = true;
    }
    CleanupIfCanceled();
}

int32_t JsAccessibilityManager::CmpRegistration::GetAcceptedTreeId()
{
    std::lock_guard<std::mutex> lock(mutex);
    return finished && !canceled ? treeId : -1;
}

bool JsAccessibilityManager::CmpRegistration::IsCanceled()
{
    std::lock_guard<std::mutex> lock(mutex);
    return canceled;
}

void JsAccessibilityManager::CmpRegistration::CleanupIfCanceled()
{
    int32_t orphanTreeId = -1;
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (!finished || !canceled || cleaned || treeId <= 0) {
            return;
        }
        cleaned = true;
        orphanTreeId = treeId;
    }
    auto instance = AccessibilitySystemAbilityClient::GetInstance();
    if (instance) {
        instance->DeregisterElementOperator(windowId, orphanTreeId);
    }
}

bool JsAccessibilityManager::CompleteCmpRegistration(const std::pair<int32_t, int64_t>& key,
    const RefPtr<JsAccessibilityManager>& subAM, const std::shared_ptr<CmpRegistration>& registration, bool success)
{
    std::lock_guard<std::mutex> lock(cmpBindingMapMutex_);
    auto it = cmpBindingMap_.find(key);
    if (it == cmpBindingMap_.end() || it->second.subAM != subAM ||
        it->second.registration != registration || it->second.state != CmpBindingState::State::REGISTERING) {
        return false;
    }
    if (!success || (registration && registration->IsCanceled())) {
        it->second.state = CmpBindingState::State::WAIT_TREE_ID;
        it->second.registration.reset();
        it->second.windowId = 0;
        return false;
    }
    it->second.windowId = registration->windowId;
    return true;
}

// Wire the subAM's parent/escalation anchors after a successful AAMS registration: parent triple (CMP tree).
void JsAccessibilityManager::WireSubAmAnchors(const RefPtr<JsAccessibilityManager>& subAM,
    const WeakPtr<NG::FrameNode>& exportedRootWeak, NG::FrameNode* owner,
    int32_t cmpTreeId, int64_t elementId, uint32_t windowId)
{
    subAM->parentTreeId_ = cmpTreeId;
    subAM->parentElementId_ = elementId;
    subAM->parentWindowId_ = windowId;
    // Plain-API exhaustion escalation anchors (see TryPlainWebExhaustionEscalation).
    subAM->cmpExportedRoot_ = exportedRootWeak;
    subAM->cmpProviderOperation_ = GetJsThirdProviderInteractionOperation(owner->GetAccessibilityId());
    // Own window of the exported subtree (same host window as the registration): with this set.
    subAM->SetWindowId(windowId);
}

// Register one converged binding to AAMS: parent = CMP tree (T_cmp) at mount element bindId.
void JsAccessibilityManager::RegisterCmpSubtreeToAams(const std::pair<int32_t, int64_t>& key)
{
    RefPtr<JsAccessibilityManager> subAM;
    WeakPtr<NG::FrameNode> exportedRootWeak;
    WeakPtr<NG::FrameNode> ownerWeak;
    int32_t cmpTreeId = -1;
    if (!SnapshotRegisteringCmpBinding(key, subAM, exportedRootWeak, ownerWeak, cmpTreeId)) {
        return;
    }
    auto owner = ownerWeak.Upgrade();
    auto exportedRoot = exportedRootWeak.Upgrade();
    auto context = owner ? owner->GetContext() : nullptr;
    if (!owner || !exportedRoot || !subAM || !context) {
        CompleteCmpRegistration(key, subAM, nullptr, false);
        return;
    }
    uint32_t windowId = static_cast<uint32_t>(context->GetRealHostWindowId());
    auto registration = std::make_shared<CmpRegistration>(windowId);
    {
        std::lock_guard<std::mutex> lock(cmpBindingMapMutex_);
        auto it = cmpBindingMap_.find(key);
        if (it == cmpBindingMap_.end() || it->second.subAM != subAM || it->second.registration ||
            it->second.state != CmpBindingState::State::REGISTERING) {
            return;
        }
        it->second.registration = registration;
    }
    int64_t elementId = key.second;
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(cmpTreeId, elementId);
    auto operation = std::make_shared<CmpInteractionOperation>(cmpTreeId, key.second, exportedRootWeak,
        owner->GetId(), Referenced::WeakClaim(this), registration);
    operation->SetHandler(Referenced::WeakClaim(subAM.GetRawPtr()));
    auto instance = AccessibilitySystemAbilityClient::GetInstance();
    Accessibility::Registration request {
        .windowId = static_cast<int32_t>(windowId),
        .parentWindowId = static_cast<int32_t>(windowId),
        .parentTreeId = cmpTreeId,
        .elementId = elementId,
    };
    auto result = instance ? instance->RegisterElementOperator(request, operation) : Accessibility::RET_ERR_NULLPTR;
    bool accepted = CompleteCmpRegistration(key, subAM, registration, result == RET_OK);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "RegisterCmpSubtreeToAams: bindId=%{public}" PRId64 ", windowId=%{public}u, "
        "cmpTreeId=%{public}d, ret=%{public}d, accepted=%{public}d",
        key.second, windowId, cmpTreeId, static_cast<int32_t>(result), accepted);
    registration->Complete(accepted);
    if (accepted && registration->IsCanceled()) {
        CompleteCmpRegistration(key, subAM, registration, false);
        return;
    }
    if (!accepted) {
        return;
    }
    WireSubAmAnchors(subAM, exportedRootWeak, owner.GetRawPtr(), cmpTreeId, elementId, windowId);
    MarkCmpBindingRegistered(key, subAM, registration);
}

// Binding state transition: T_sub assigned (CmpInteractionOperation::SetBelongTreeId continuation).
void JsAccessibilityManager::MarkCmpBindingRegistered(const std::pair<int32_t, int64_t>& key,
    const RefPtr<JsAccessibilityManager>& subAM, const std::shared_ptr<CmpRegistration>& registration)
{
    CHECK_NULL_VOID(registration);
    auto treeId = registration->GetAcceptedTreeId();
    if (treeId <= 0) {
        return;
    }
    WeakPtr<NG::FrameNode> rootWeak;
    bool current = false;
    {
        std::lock_guard<std::mutex> lock(cmpBindingMapMutex_);
        auto it = cmpBindingMap_.find(key);
        current = it != cmpBindingMap_.end() && it->second.subAM == subAM &&
            it->second.registration == registration;
        if (current) {
            if (it->second.state != CmpBindingState::State::REGISTERING) {
                return;
            }
            it->second.state = CmpBindingState::State::REGISTERED;
            rootWeak = it->second.exportedRoot;
        }
    }
    if (!current) {
        registration->Cancel();
        return;
    }
    auto root = rootWeak.Upgrade();
    if (!root) {
        DetachCmpBinding(key);
        return;
    }
    subAM->treeId_ = treeId;
    subAM->Register(true);
    MigrateChildTreeCallbacksToSubAM(root, subAM);
    subAM->NotifyChildTreeOnRegister(treeId);
}

// Whole-subtree detach for one binding (Unbind semantics): deregister grandchild trees first, then deregister the.
void JsAccessibilityManager::DetachCmpBinding(const std::pair<int32_t, int64_t>& key)
{
    RefPtr<JsAccessibilityManager> subAM;
    WeakPtr<NG::FrameNode> exportedRootWeak;
    WeakPtr<NG::FrameNode> ownerWeak;
    std::shared_ptr<CmpRegistration> registration;
    {
        std::lock_guard<std::mutex> lock(cmpBindingMapMutex_);
        auto it = cmpBindingMap_.find(key);
        if (it == cmpBindingMap_.end()) {
            return; // idempotent: unknown/inactive binding is a no-op (AC-16/AC-19)
        }
        subAM = it->second.subAM;
        exportedRootWeak = it->second.exportedRoot;
        ownerWeak = it->second.owner;
        registration = it->second.registration;
        cmpBindingMap_.erase(it);
    }
    if (subAM) {
        subAM->Register(false);
        std::unordered_map<int64_t, std::shared_ptr<AccessibilityChildTreeCallback>> callbacks;
        {
            std::lock_guard<std::mutex> lock(subAM->childTreeCallbackMapMutex_);
            callbacks.swap(subAM->childTreeCallbackMap_);
        }
        for (const auto& [elementId, callback] : callbacks) {
            if (callback) {
                callback->OnDeregister();
                callback->SetChildTreeId(0);
            }
        }
        {
            std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
            for (const auto& [elementId, callback] : callbacks) {
                if (callback) {
                    detachedCmpChildTreeCallbacks_[elementId] = callback;
                }
            }
        }
    }
    if (registration) {
        registration->Cancel();
    }
    auto owner = ownerWeak.Upgrade();
    if (owner) {
        auto provider = GetJsThirdProviderInteractionOperation(owner->GetAccessibilityId()).lock();
        if (provider) {
            provider->RemoveChildTreeIdAndWinId(key.second);
        }
    }
    // 3. Drop the node-registry entry for this exported subtree (subAM releases with the binding).
    auto exportedRoot = exportedRootWeak.Upgrade();
    if (exportedRoot) {
        CmpAccessibility::CmpAccessibilityRegistry::UnregisterSubAM(exportedRoot.GetRawPtr());
    }
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "DetachCmpBinding: bindId=%{public}" PRId64, key.second);
}


// Node-aware AM routing: returns the binding's subAM for nodes inside a bound exported subtree.
RefPtr<AccessibilityManager> JsAccessibilityManager::GetAccessibilityManagerForNode(
    const RefPtr<NG::FrameNode>& node)
{
    if (node) {
        auto subAMBase = CmpAccessibility::CmpAccessibilityRegistry::GetSubAM(node.GetRawPtr());
        if (subAMBase) {
            auto subAM = AceType::DynamicCast<JsAccessibilityManager>(subAMBase);
            if (subAM) {
                return subAM;
            }
        }
    }
    return Claim(this);
}

// Register one converged binding to AAMS (parentElementId = bindId, per ).
void JsAccessibilityManager::SearchElementInfoByAccessibilityIdFromRoot(int64_t elementId, int32_t mode,
    std::list<AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context,
    const RefPtr<NG::FrameNode>& exportedRoot)
{
    CHECK_NULL_VOID(context);
    CHECK_NULL_VOID(exportedRoot);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);

    // Search root = exportedRoot (Owner rule: not pipeline GetRootElement).
    AccessibilityElementInfo nodeInfo;
    int64_t nodeId = (elementId == -1) ? exportedRoot->GetAccessibilityId() : elementId;
    auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(exportedRoot, nodeId);
    CHECK_NULL_VOID(node);
    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, context, node);
    // Coordinate transform: reuse the main-tree SetRectInScreen mechanism.
    UpdateElementInfo(node, commonProperty, nodeInfo, ngPipeline);
    SetRootAccessibilityVisible(node, nodeInfo);
    // The exported-tree root has no parent inside this subtree: AAMS reaches it from the CMP tree via.
    if (node->GetAccessibilityId() == exportedRoot->GetAccessibilityId()) {
        nodeInfo.SetParent(INVALID_PARENT_ID);
    }
    if (IsExtensionComponent(node) && !IsUIExtensionShowPlaceholder(node)) {
        SearchParameter extensionParam {-1, "", mode, NG::UI_EXTENSION_OFFSET_MAX};
        SearchExtensionElementInfoNG(extensionParam, node, infos, nodeInfo);
    }
    infos.push_back(nodeInfo);
    // Whole-subtree prefetch: AAMS queries this operator with GET_SOURCE_MODE (recursive children), so return the full subtree rooted at elementId.
    SearchParameter param {nodeId, "", mode, NG::UI_EXTENSION_OFFSET_MAX};
    UpdateCacheInfoNG(infos, node, commonProperty, ngPipeline, param);
    SortExtensionAccessibilityInfo(infos, nodeInfo.GetAccessibilityId());
}

void JsAccessibilityManager::FocusMoveSearchFromRoot(int64_t elementId, int32_t direction,
    Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
    const RefPtr<NG::FrameNode>& exportedRoot)
{
    CHECK_NULL_VOID(context);
    CHECK_NULL_VOID(exportedRoot);
    info.SetValidElement(false);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);

#ifdef WEB_SUPPORTED
    // Dissolved-embed-content origin: see the WithCondition variant's hook.
    if (SearchInDissolvedEmbedContent(elementId, direction, false,
            static_cast<int32_t>(Accessibility::FocusRuleType::DEFAULT),
            context, exportedRoot, info) == DissolvedEmbedSearch::FOUND) {
        return;
    }
#endif // WEB_SUPPORTED

    // Search root = exportedRoot (Owner rule).
    std::list<AccessibilityElementInfo> nodeList;
    SearchElementInfoByAccessibilityIdFromRoot(-1, PREFETCH_RECURSIVE_CHILDREN, nodeList, context, exportedRoot);
    if (nodeList.empty()) {
        return;
    }
    AccessibilityElementInfo focusElement;
    FilterAccessibilityElementByFocusable(nodeList, focusElement, elementId, true);
    switch (direction) {
        case FocusMoveDirection::FORWARD:
        case FocusMoveDirection::BACKWARD:
            // No wrap at the subtree tail/head: boundary exhaustion falls through to the SEARCH_FAIL_IN_CHILDTREE defer below.
            FindSubtreeNodeInRelativeDirection(focusElement, nodeList, info, direction);
            break;
        case FocusMoveDirection::UP:
        case FocusMoveDirection::DOWN:
        case FocusMoveDirection::LEFT:
        case FocusMoveDirection::RIGHT:
            FindExtensionNodeInAbsoluteDirection(focusElement, nodeList, info, direction);
            break;
        default:
            break;
    }
}

bool JsAccessibilityManager::ExecuteActionFromRoot(int64_t elementId,
    const std::map<std::string, std::string>& actionArguments, Accessibility::ActionType action,
    const RefPtr<PipelineBase>& context, const RefPtr<NG::FrameNode>& exportedRoot)
{
    bool result = false;
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_RETURN(ngPipeline, result);
    CHECK_NULL_RETURN(exportedRoot, result);
    ContainerScope instance(ngPipeline->GetInstanceId());
    // Search root = exportedRoot (Owner rule, same as SearchFromRoot/FocusMoveSearchFromRoot).
    auto frameNode = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(exportedRoot, elementId);
    CHECK_NULL_RETURN(frameNode, result);
    auto enabled = frameNode->GetFocusHub() ? frameNode->GetFocusHub()->IsEnabled() : true;
    if (!enabled && action != Accessibility::ActionType::ACCESSIBILITY_ACTION_ACCESSIBILITY_FOCUS &&
        action != Accessibility::ActionType::ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS) {
        return result;
    }
    result = ConvertActionTypeToBoolen(action, frameNode, elementId, ngPipeline, actionArguments);
    if (!result) {
        auto accessibilityProperty = frameNode->GetAccessibilityProperty<NG::AccessibilityProperty>();
        CHECK_NULL_RETURN(accessibilityProperty, false);
        result = ActAccessibilityAction(action, actionArguments, accessibilityProperty, frameNode);
    }
    return result;
}

namespace {
// Main-tree candidate-check mirror for the FromRoot list search: the strategy's tree walk validates every.
bool IsSubtreeCandidateFocusable(const RefPtr<NG::FrameNode>& exportedRoot,
    const AccessibilityElementInfo& candidate, Accessibility::FocusRuleType focusRuleType,
    bool bypassDescendants)
{
    if (!bypassDescendants && candidate.GetChildTreeId() > 0) {
        return true;
    }
    auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(
        exportedRoot, candidate.GetAccessibilityId());
    if (!node) {
        return false;
    }
    auto checkNode = std::make_shared<FrameNodeRulesCheckNode>(node, node->GetAccessibilityId());
    if (!checkNode->IsAccessibiltyVisible()) {
        return false;
    }
    if (!CheckNodeMatchedFocusType(checkNode, focusRuleType)) {
        return false;
    }
    auto client = Accessibility::AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(client, false);
    bool isReadable = false;
    auto checkResult = client->CheckNodeIsReadable(checkNode, isReadable);
    if (checkResult != Accessibility::RET_OK || !isReadable) {
        return false;
    }
    return true;
}
} // namespace

#ifdef WEB_SUPPORTED
bool JsAccessibilityManager::TryFocusMoveSearchWebGrandchildFromRoot(const AccessibilityElementInfo& hostInfo,
    const Accessibility::AccessibilityFocusMoveParam& param, const RefPtr<PipelineBase>& context,
    const RefPtr<NG::FrameNode>& exportedRoot, std::list<AccessibilityElementInfo>& infos,
    FocusMoveResult& result)
{
    // Resolve the grandchild-tree host node inside the exported subtree and confirm it is an ArkWeb host before touching the callback table.
    auto hostNode = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(
        exportedRoot, hostInfo.GetAccessibilityId());
    CHECK_NULL_RETURN(hostNode, false);
    CHECK_NE_RETURN(hostNode->GetTag(), V2::WEB_ETS_TAG, false);
    RefPtr<NG::WebPattern> webPattern;
    {
        std::lock_guard<std::mutex> lock(childTreeCallbackMapMutex_);
        auto callbackIt = childTreeCallbackMap_.find(hostInfo.GetAccessibilityId());
        if (callbackIt != childTreeCallbackMap_.end()) {
            auto callback = static_cast<NG::WebAccessibilityChildTreeCallback*>(callbackIt->second.get());
            CHECK_NULL_RETURN(callback, false);
            webPattern = callback->GetWebPattern().Upgrade();
        }
    }
    CHECK_NULL_RETURN(webPattern, false);
    if (!IsWebFocusRuleTypeSupported(param.type)) {
        return false;
    }
    // Drill-down direction contract (mirrors what the DOUBLE_CHECK handoff asks the AA to do): BACKWARD / FIND_LAST.
    int32_t webDirection = (param.direction == Accessibility::FocusMoveDirection::BACKWARD
        || param.direction == Accessibility::FocusMoveDirection::FIND_LAST)
            ? Accessibility::FocusMoveDirection::FIND_LAST
            : Accessibility::FocusMoveDirection::FORWARD;
    AccessibilityElementInfo webNodeInfo;
    auto webResult = WebFocusMoveSearchWithConditionNG(
        -1, webDirection, static_cast<int32_t>(param.type), webNodeInfo, context, webPattern);
    WebFocusMoveSearchByComponent(
        webNodeInfo, webPattern, webDirection, context, webResult, static_cast<int32_t>(param.type));
    if (webResult.resultType != Accessibility::FocusMoveResultType::SEARCH_SUCCESS
        || !webNodeInfo.IsValidElement()) {
        TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
            "Web grandchild drill miss: hostId=%{public}" PRId64 ", childTreeId=%{public}d"
            ", webDirection=%{public}d, webResult=%{public}d",
            hostInfo.GetAccessibilityId(), hostInfo.GetChildTreeId(), webDirection,
            static_cast<int32_t>(webResult.resultType));
        return false;
    }
    // Terminal success in the Web-operator form: splice the Web treeId so follow-ups route to the Web operator.
    UpdateElementInfoTreeId(webNodeInfo);
    infos.emplace_back(webNodeInfo);
    result.resultType = Accessibility::FocusMoveResultType::SEARCH_SUCCESS;
    result.nowLevelBelongTreeId = -1;
    result.parentWindowId = 0;
    result.changeToNewInfo = false;
    result.needTerminate = true;
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "Web grandchild drill hit: hostId=%{public}" PRId64 ", childTreeId=%{public}d"
        ", webDirection=%{public}d, webNodeId=%{public}" PRId64 ", treeId=%{public}d",
        hostInfo.GetAccessibilityId(), hostInfo.GetChildTreeId(), webDirection,
        webNodeInfo.GetAccessibilityId(), webNodeInfo.GetBelongTreeId());
    return true;
}
#endif // WEB_SUPPORTED

#ifdef WEB_SUPPORTED
namespace {
// In-process capture shim for FocusMoveSearchWithConditionFromRoot replies: the plain-API exhaustion escalation.
class FromRootResultCaptureShim : public Accessibility::AccessibilityElementOperatorCallback {
public:
    FromRootResultCaptureShim() = default;
    ~FromRootResultCaptureShim() override = default;

    void SetFocusMoveSearchWithConditionResult(
        const std::list<Accessibility::AccessibilityElementInfo>& infos,
        const Accessibility::FocusMoveResult& result, const int32_t requestId) override
    {
        infos_ = infos;
        result_ = result;
    }

    std::list<Accessibility::AccessibilityElementInfo> infos_;
    Accessibility::FocusMoveResult result_ {};

private:
    void SetSearchElementInfoByAccessibilityIdResult(
        const std::list<Accessibility::AccessibilityElementInfo>& infos,
        const int32_t requestId) override {}
    void SetSearchDefaultFocusByWindowIdResult(
        const std::list<Accessibility::AccessibilityElementInfo>& infos,
        const int32_t requestId) override {}
    void SetSearchElementInfoByTextResult(
        const std::list<Accessibility::AccessibilityElementInfo>& infos,
        const int32_t requestId) override {}
    void SetFindFocusedElementInfoResult(
        const Accessibility::AccessibilityElementInfo& info, const int32_t requestId) override {}
    void SetFocusMoveSearchResult(
        const Accessibility::AccessibilityElementInfo& info, const int32_t requestId) override {}
    void SetExecuteActionResult(const bool succeeded, const int32_t requestId) override {}
    void SetCursorPositionResult(const int32_t cursorPosition, const int32_t requestId) override {}
    void SetSearchElementInfoBySpecificPropertyResult(
        const std::list<Accessibility::AccessibilityElementInfo>& infos,
        const std::list<Accessibility::AccessibilityElementInfo>& treeInfos,
        const int32_t requestId) override {}
    void SetUpdateCustomAccessibilityPropertyResult(
        const Accessibility::OperateVirtualNodeResult result, const int32_t requestId) override {}
    void SetAddAccessibilityVirtualNodeResult(
        const Accessibility::OperateVirtualNodeResult result, const int32_t requestId) override {}
    void SetRemoveAccessibilityVirtualNodeResult(
        const Accessibility::OperateVirtualNodeResult result, const int32_t requestId) override {}
};
} // namespace

bool JsAccessibilityManager::TryPlainWebExhaustionEscalation(int32_t direction, int32_t requestId,
    const RefPtr<NG::FrameNode>& hostNode, const RefPtr<PipelineBase>& context,
    AccessibilityElementInfo& nodeInfo)
{
    // Binding anchors registered at RegisterCmpSubtreeToAaks (absent on the main AM).
    auto exportedRoot = cmpExportedRoot_.Upgrade();
    if (!exportedRoot || !hostNode || !context) {
        TAG_LOGI(AceLogTag::ACE_WEB,
            "Plain Web exhaustion escalation miss: binding anchors unavailable, requestId=%{public}d",
            requestId);
        return false;
    }
    // Hop 1 - exported subtree (T_sub) from the Web host: re-issue the search through the WithCondition.
    {
        AccessibilityElementInfo hostInfo;
        hostInfo.SetAccessibilityId(hostNode->GetAccessibilityId());
        Accessibility::AccessibilityFocusMoveParam subParam {
            .direction = Accessibility::FocusMoveDirection::BACKWARD,
            .condition = Accessibility::DetailCondition::BYPASS_SELF,
        };
        FromRootResultCaptureShim shim;
        FocusMoveSearchWithConditionFromRoot(hostInfo, subParam, requestId, shim, exportedRoot);
        if (shim.result_.resultType == Accessibility::FocusMoveResultType::SEARCH_SUCCESS
            && !shim.infos_.empty() && shim.infos_.front().IsValidElement()) {
            nodeInfo = shim.infos_.front();
            TAG_LOGI(AceLogTag::ACE_WEB,
                "Plain Web exhaustion escalation hop 1 (exported subtree) hit: targetId=%{public}" PRId64
                ", treeId=%{public}d, requestId=%{public}d",
                nodeInfo.GetAccessibilityId(), nodeInfo.GetBelongTreeId(), requestId);
            return true;
        }
        TAG_LOGI(AceLogTag::ACE_WEB,
            "Plain Web exhaustion escalation hop 1 miss: result=%{public}d, needTerminate=%{public}d"
            ", infos=%{public}zu, requestId=%{public}d",
            static_cast<int32_t>(shim.result_.resultType), shim.result_.needTerminate,
            shim.infos_.size(), requestId);
    }
    // Hop 2: search the CMP tree synchronously from the mount (bindId).
    auto providerOp = cmpProviderOperation_.lock();
    if (!providerOp) {
        TAG_LOGI(AceLogTag::ACE_WEB,
            "Plain Web exhaustion escalation hop 2 miss: provider operator unavailable"
            ", requestId=%{public}d", requestId);
        return false;
    }
    TAG_LOGI(AceLogTag::ACE_WEB,
        "Plain Web exhaustion escalation hop 2 enter: opTreeId=%{public}d, parentTreeId=%{public}d"
        ", bindId=%{public}" PRId64 ", requestId=%{public}d",
        providerOp->GetBelongTreeId(), parentTreeId_, parentElementId_, requestId);
    // parentElementId_ carries the T_cmp splice (AAMS-facing mount id); the sync provider
    // search resolves the origin through FindNativeInfoById — the provider's OWN raw id
    // space — so split before querying (same convention as the other provider call sites).
    int64_t rawMountElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t mountTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(
        parentElementId_, rawMountElementId, mountTreeId);
    {
        Accessibility::AccessibilityFocusMoveParam cmpParam {
            .direction = Accessibility::FocusMoveDirection::BACKWARD,
            .condition = Accessibility::DetailCondition::BYPASS_SELF,
        };
        AccessibilityElementInfo cmpInfo;
        auto cmpResult = providerOp->SearchFocusMoveWithConditionSync(rawMountElementId, cmpParam, cmpInfo);
        // Routable-target double check (the sync method already gates this).
        if (cmpResult.resultType == Accessibility::FocusMoveResultType::SEARCH_SUCCESS
            && cmpInfo.IsValidElement() && cmpInfo.GetAccessibilityId() >= 0) {
            nodeInfo = cmpInfo;
            TAG_LOGI(AceLogTag::ACE_WEB,
                "Plain Web exhaustion escalation hop 2 (CMP tree) hit: targetId=%{public}" PRId64
                ", treeId=%{public}d, requestId=%{public}d",
                nodeInfo.GetAccessibilityId(), nodeInfo.GetBelongTreeId(), requestId);
            return true;
        }
        TAG_LOGI(AceLogTag::ACE_WEB,
            "Plain Web exhaustion escalation hop 2 miss: result=%{public}d, needTerminate=%{public}d"
            ", bindId=%{public}" PRId64 " (raw=%{public}" PRId64 "), requestId=%{public}d",
            static_cast<int32_t>(cmpResult.resultType), cmpResult.needTerminate,
            rawMountElementId, rawMountElementId, requestId);
    }
    return false;
}
#endif // WEB_SUPPORTED

// Unconditional (CMP unittest target compiles without WEB_SUPPORTED): the in-content search path is web-free.
JsAccessibilityManager::DissolvedEmbedSearch JsAccessibilityManager::SearchInDissolvedEmbedContent(
    int64_t splitElementId, int32_t direction, bool checkSelf, int32_t focusRuleType,
    const RefPtr<PipelineBase>& context, const RefPtr<NG::FrameNode>& exportedRoot,
    AccessibilityElementInfo& outInfo)
{
    if (direction != Accessibility::FocusMoveDirection::FORWARD &&
        direction != Accessibility::FocusMoveDirection::BACKWARD) {
        return DissolvedEmbedSearch::NOT_APPLICABLE;
    }
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_RETURN(ngPipeline, DissolvedEmbedSearch::NOT_APPLICABLE);
    CHECK_NULL_RETURN(exportedRoot, DissolvedEmbedSearch::NOT_APPLICABLE);
    // Resolve the origin node, then walk up to a registered embed anchor (the dissolved content's root).
    auto originNode = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(
        exportedRoot, splitElementId);
    if (!originNode) {
        return DissolvedEmbedSearch::NOT_APPLICABLE;
    }
    std::string surfaceId;
    auto cursor = originNode;
    while (cursor && cursor->GetId() != exportedRoot->GetId()) {
        auto candidateSurfaceId = GetSurfaceIdByEmbedNode(cursor);
        if (!candidateSurfaceId.empty() && candidateSurfaceId != "0") {
            surfaceId = candidateSurfaceId;
            break;
        }
        cursor = cursor->GetParentFrameNode();
    }
    if (surfaceId.empty()) {
        return DissolvedEmbedSearch::NOT_APPLICABLE;
    }
    // The content's own list.
    std::list<AccessibilityElementInfo> contentInfos;
    int32_t windowId = static_cast<int32_t>(ngPipeline->GetRealHostWindowId());
    auto searchRet = SearchElementInfoBySurfaceId(
        surfaceId, windowId, SearchSurfaceIdType::SEARCH_ALL, contentInfos);
    if (searchRet != SearchSurfaceIdRet::SEARCH_SUCCESS || contentInfos.empty()) {
        return DissolvedEmbedSearch::NOT_APPLICABLE;
    }
    // Resolve the origin in the content list.
    int64_t splicedOriginId = splitElementId;
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(treeId_, splicedOriginId);
    auto originIt = contentInfos.begin();
    for (; originIt != contentInfos.end(); ++originIt) {
        if (originIt->GetAccessibilityId() == splicedOriginId) {
            break;
        }
    }
    if (originIt == contentInfos.end()) {
        return DissolvedEmbedSearch::NOT_APPLICABLE;
    }
    // CHECK_SELF self-acceptance (main-tree parity, round): the AA's backward-entry drill re-issues the move with.
    if (checkSelf && originIt->GetChildTreeId() <= 0) {
        AccessibilityElementInfo selfCheck = *originIt;
        selfCheck.SetAccessibilityId(splitElementId);
        if (IsSubtreeCandidateFocusable(exportedRoot, selfCheck,
                static_cast<Accessibility::FocusRuleType>(focusRuleType), false)) {
            outInfo = *originIt;
            return DissolvedEmbedSearch::FOUND;
        }
    }
    // In-content search: iterate from the origin, take the first non-zero-size candidate.
    bool forward = direction == Accessibility::FocusMoveDirection::FORWARD;
    auto candidate = originIt;
    while (true) {
        if (forward) {
            ++candidate;
            if (candidate == contentInfos.end()) {
                break;
            }
        } else {
            if (candidate == contentInfos.begin()) {
                candidate = contentInfos.end();
                break;
            }
            --candidate;
        }
        auto width = candidate->GetRectInScreen().GetRightBottomXScreenPostion() -
            candidate->GetRectInScreen().GetLeftTopXScreenPostion();
        auto height = candidate->GetRectInScreen().GetRightBottomYScreenPostion() -
            candidate->GetRectInScreen().GetLeftTopYScreenPostion();
        if (width == 0 || height == 0) {
            continue;
        }
        outInfo = *candidate;
        return DissolvedEmbedSearch::FOUND;
    }
    // Content exhausted in this direction: continue in the WEB tree from the embeddedObject.
#ifdef WEB_SUPPORTED
    auto weakWeb = GetWebPatternBySurfaceIdWithMainFallback(surfaceId);
    auto webPattern = weakWeb.Upgrade();
    auto webHost = webPattern ? webPattern->GetHost() : nullptr;
    auto mainContext = context_.Upgrade();
    if (!webPattern || !webHost || !mainContext) {
        return DissolvedEmbedSearch::NOT_APPLICABLE;
    }
    int64_t embedWebId = GetWebAccessibilityIdBySurfaceId(surfaceId);
    if (embedWebId == INVALID_NODE_ID) {
        return DissolvedEmbedSearch::NOT_APPLICABLE;
    }
    int64_t rawEmbedId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t embedTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(embedWebId, rawEmbedId, embedTreeId);
    auto node = webPattern->GetAccessibilityNodeByFocusMove(rawEmbedId, direction);
    if (!node) {
        return DissolvedEmbedSearch::NOT_APPLICABLE;
    }
    // Continuation embed-handoff: the web DOM's next node after this embed may be ANOTHER embeddedObject.
    if (IsTagInEmbedComponent(node->GetComponentType())) {
        std::string nextSurfaceId = webPattern->GetSurfaceIdByHtmlElementId(node->GetHtmlElementId());
        if (!nextSurfaceId.empty()) {
            std::list<AccessibilityElementInfo> nextEmbedInfos;
            auto nextSearchType = forward ? SearchSurfaceIdType::SEARCH_HEAD
                : SearchSurfaceIdType::SEARCH_TAIL;
            auto nextRet = SearchElementInfoBySurfaceId(
                nextSurfaceId, windowId, nextSearchType, nextEmbedInfos);
            if (nextRet == SearchSurfaceIdRet::SEARCH_SUCCESS && !nextEmbedInfos.empty()) {
                outInfo = nextEmbedInfos.front();
                return DissolvedEmbedSearch::FOUND;
            }
        }
    }
    CommonProperty commonProperty;
    GenerateCommonPropertyForWeb(ngPipeline, commonProperty, mainContext, webHost);
    UpdateWebAccessibilityElementInfo(node, commonProperty, outInfo, webPattern);
    UpdateElementInfoTreeId(outInfo);
    return DissolvedEmbedSearch::FOUND;
#else
    // No web support: the content-exhaustion continuation lives in the web tree.
    return DissolvedEmbedSearch::NOT_APPLICABLE;
#endif
}

namespace {
// Directions the subtree search supports.
bool IsSupportedFocusMoveDirection(Accessibility::FocusMoveDirection direction)
{
    switch (direction) {
        case Accessibility::FocusMoveDirection::FORWARD:
        case Accessibility::FocusMoveDirection::BACKWARD:
        case Accessibility::FocusMoveDirection::UP:
        case Accessibility::FocusMoveDirection::DOWN:
        case Accessibility::FocusMoveDirection::LEFT:
        case Accessibility::FocusMoveDirection::RIGHT:
            return true;
        default:
            return false;
    }
}

// Unsupported-direction protocol error reply: terminate the composite immediately with NOT_SUPPORT instead of.
void ReplyDirectionNotSupport(const Accessibility::AccessibilityFocusMoveParam& param,
    const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    std::list<AccessibilityElementInfo> notSupportInfos;
    AccessibilityElementInfo invalidInfo;
    invalidInfo.SetValidElement(false);
    notSupportInfos.emplace_back(invalidInfo);
    FocusMoveResult notSupportResult = {
        .resultType = Accessibility::FocusMoveResultType::NOT_SUPPORT,
        .nowLevelBelongTreeId = -1,
        .parentWindowId = 0,
        .changeToNewInfo = false,
        .needTerminate = true,
    };
    callback.SetFocusMoveSearchWithConditionResult(notSupportInfos, notSupportResult, requestId);
    TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
        "FocusMoveSearchWithConditionFromRoot: unsupported direction=%{public}d, requestId=%{public}d",
        static_cast<int32_t>(param.direction), requestId);
}

void NormalizeSubtreeSearchIds(std::list<AccessibilityElementInfo>& nodeList, int32_t treeId)
{
    if (treeId <= 0) {
        return;
    }
    auto toLocalId = [treeId](int64_t elementId) {
        if (elementId < 0) {
            return elementId;
        }
        int64_t rawId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
        int32_t belongTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
        AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, rawId, belongTreeId);
        return belongTreeId == treeId ? rawId : elementId;
    };
    for (auto& element : nodeList) {
        element.SetAccessibilityId(toLocalId(element.GetAccessibilityId()));
        element.SetParent(toLocalId(element.GetParentNodeId()));
        auto children = element.GetChildIds();
        for (auto childId : children) {
            element.RemoveChild(childId);
            element.AddChild(toLocalId(childId));
        }
    }
}

// Reading-order rebuild: the recursive prefetch collects breadth-first; rebuild DFS pre-order from child links.
void RebuildReadingOrderList(std::list<AccessibilityElementInfo>& nodeList,
    const RefPtr<NG::FrameNode>& exportedRoot)
{
    std::map<int64_t, const AccessibilityElementInfo*> nodesById;
    for (const auto& element : nodeList) {
        nodesById.emplace(element.GetAccessibilityId(), &element);
    }
    auto rootIt = nodesById.find(exportedRoot->GetAccessibilityId());
    if (rootIt == nodesById.end()) {
        return;
    }
    std::list<AccessibilityElementInfo> ordered;
    std::set<int64_t> visited;
    std::vector<const AccessibilityElementInfo*> stack { rootIt->second };
    while (!stack.empty()) {
        auto* current = stack.back();
        stack.pop_back();
        if (!visited.insert(current->GetAccessibilityId()).second) {
            continue;
        }
        ordered.emplace_back(*current);
        const auto& childIds = current->GetChildIds();
        for (auto childIt = childIds.rbegin(); childIt != childIds.rend(); ++childIt) {
            auto nodeIt = nodesById.find(*childIt);
            if (nodeIt != nodesById.end()) {
                stack.emplace_back(nodeIt->second);
            }
        }
    }
    for (const auto& element : nodeList) {
        if (visited.find(element.GetAccessibilityId()) == visited.end()) {
            ordered.emplace_back(element);
        }
    }
    nodeList = std::move(ordered);
}

// Origin-level acceptance (main-tree strategy semantics, GetAceCondition + check-self-first): with.
bool TryAcceptOriginAsCandidate(const RefPtr<NG::FrameNode>& exportedRoot,
    const Accessibility::AccessibilityFocusMoveParam& param,
    const AccessibilityElementInfo& focusElement, int64_t splitElementId, bool bypassDescendants,
    AccessibilityElementInfo& nodeInfo)
{
    if (!bypassDescendants && param.direction == Accessibility::FocusMoveDirection::FORWARD
        && splitElementId >= 0 && focusElement.GetAccessibilityId() == splitElementId
        && focusElement.GetChildTreeId() > 0) {
        nodeInfo = focusElement;
        return true;
    }
    if ((param.condition == Accessibility::DetailCondition::CHECK_SELF
        || param.condition == Accessibility::DetailCondition::CHECK_SELF_BYPASS_DESCENDANTS)
        && splitElementId >= 0 && focusElement.GetAccessibilityId() == splitElementId
        && focusElement.GetChildTreeId() <= 0) {
        // Grandchild-host origins are excluded from self-acceptance (a parked host would bounce focus back).
        if (IsSubtreeCandidateFocusable(exportedRoot, focusElement, param.type, bypassDescendants)) {
            nodeInfo = focusElement;
            return true;
        }
        TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
            "FocusMoveSearchWithConditionFromRoot CHECK_SELF origin rejected: originId=%{public}" PRId64
            ", focusRuleType=%{public}d, bypassDescendants=%{public}d"
            " (visibility / focus-type / readability check failed)",
            splitElementId, static_cast<int32_t>(param.type), bypassDescendants);
    }
    return false;
}

bool TryExplicitFocusRelationJump(const RefPtr<NG::FrameNode>& exportedRoot,
    const Accessibility::AccessibilityFocusMoveParam& param, int64_t splitElementId,
    const std::list<AccessibilityElementInfo>& nodeList, bool bypassDescendants,
    AccessibilityElementInfo& nodeInfo)
{
    if (splitElementId < 0) {
        return false;
    }
    auto originNode = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(exportedRoot, splitElementId);
    CHECK_NULL_RETURN(originNode, false);
    RefPtr<NG::FrameNode> targetNode;
    if (param.direction == Accessibility::FocusMoveDirection::FORWARD) {
        targetNode = GetAccessibilityNextFocusNode(originNode, exportedRoot);
    } else {
        auto context = originNode->GetContextRefPtr();
        CHECK_NULL_RETURN(context, false);
        auto mainAM = AceType::DynamicCast<JsAccessibilityManager>(context->GetAccessibilityManager());
        CHECK_NULL_RETURN(mainAM, false);
        targetNode = mainAM->GetPrevFocusNodeByManager(originNode, exportedRoot, context);
    }
    CHECK_NULL_RETURN(targetNode, false);
    auto targetId = targetNode->GetAccessibilityId();
    if (targetId == splitElementId) {
        return false;
    }
    for (const auto& candidate : nodeList) {
        if (candidate.GetAccessibilityId() == targetId
            && IsSubtreeCandidateFocusable(exportedRoot, candidate, param.type, bypassDescendants)) {
            nodeInfo = candidate;
            TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
                "FocusMoveSearchWithConditionFromRoot explicit jump: originId=%{public}" PRId64
                ", targetId=%{public}" PRId64 ", direction=%{public}d",
                splitElementId, targetId, static_cast<int32_t>(param.direction));
            return true;
        }
    }
    return false;
}

class SubtreeClippedCheckNode : public FrameNodeRulesCheckNode {
public:
    SubtreeClippedCheckNode(const RefPtr<NG::FrameNode>& node, int64_t accessibilityId,
        const RefPtr<NG::FrameNode>& boundary)
        : FrameNodeRulesCheckNode(node, accessibilityId), boundary_(boundary) {}

    std::shared_ptr<FocusRulesCheckNode> GetAceParent() override
    {
        auto node = GetFrameNode();
        if (node && node == boundary_) {
            return nullptr;
        }
        auto parent = FrameNodeRulesCheckNode::GetAceParent();
        CHECK_NULL_RETURN(parent, nullptr);
        auto parentNode = std::static_pointer_cast<FrameNodeRulesCheckNode>(parent)->GetFrameNode();
        CHECK_NULL_RETURN(parentNode, nullptr);
        return std::make_shared<SubtreeClippedCheckNode>(
            parentNode, parentNode->GetAccessibilityId(), boundary_);
    }

private:
    RefPtr<NG::FrameNode> boundary_;
};

std::shared_ptr<FocusRulesCheckNode> FindScrollBoundaryFromRoot(const RefPtr<NG::FrameNode>& exportedRoot,
    int64_t originId, bool forward)
{
    auto origin = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(exportedRoot, originId);
    CHECK_NULL_RETURN(origin, nullptr);
    auto checkNode = std::make_shared<SubtreeClippedCheckNode>(origin, originId, exportedRoot);
    AceFocusMoveDetailCondition condition = {.bypassSelf = true, .bypassDescendants = false};
    std::list<std::shared_ptr<FocusRulesCheckNode>> ancestors;
    AccessibilityFocusStrategy strategy;
    if (forward) {
        strategy.FindForwardScrollAncestor(condition, checkNode, ancestors);
    } else {
        strategy.FindBackwardScrollAncestor(condition, checkNode, ancestors);
    }
    return ancestors.empty() ? nullptr : ancestors.front();
}

bool IsWithinScrollBoundary(const RefPtr<NG::FrameNode>& exportedRoot, int64_t candidateId,
    const std::shared_ptr<FocusRulesCheckNode>& boundary)
{
    auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(exportedRoot, candidateId);
    CHECK_NULL_RETURN(node, false);
    auto checkNode = std::make_shared<SubtreeClippedCheckNode>(node, candidateId, exportedRoot);
    auto parent = checkNode->GetAceParent();
    while (parent) {
        if (parent->GetAccessibilityId() == boundary->GetAccessibilityId()) {
            return true;
        }
        parent = parent->GetAceParent();
    }
    return false;
}

// FORWARD/BACKWARD candidate search over the ordered list with the main-tree candidate check.
std::shared_ptr<FocusRulesCheckNode> SearchForwardBackwardCandidate(const RefPtr<NG::FrameNode>& exportedRoot,
    const Accessibility::AccessibilityFocusMoveParam& param,
    const AccessibilityElementInfo& focusElement, int64_t splitElementId,
    const std::list<AccessibilityElementInfo>& nodeList, AccessibilityElementInfo& nodeInfo)
{
    bool bypassDescendants =
        param.condition == Accessibility::DetailCondition::BYPASS_SELF_DESCENDANTS
        || param.condition == Accessibility::DetailCondition::CHECK_SELF_BYPASS_DESCENDANTS;
    if (TryAcceptOriginAsCandidate(
        exportedRoot, param, focusElement, splitElementId, bypassDescendants, nodeInfo)) {
        return nullptr;
    }
    if (TryExplicitFocusRelationJump(exportedRoot, param, splitElementId, nodeList, bypassDescendants, nodeInfo)) {
        return nullptr;
    }
    bool forward = param.direction == Accessibility::FocusMoveDirection::FORWARD;
    auto scrollBoundary = FindScrollBoundaryFromRoot(exportedRoot, splitElementId, forward);
    auto originIt = nodeList.end();
    for (auto it = nodeList.begin(); it != nodeList.end(); ++it) {
        if (it->GetAccessibilityId() == focusElement.GetAccessibilityId()) {
            originIt = it;
            break;
        }
    }
    if (forward) {
        auto cur = nodeList.begin();
        if (originIt != nodeList.end()) {
            cur = originIt;
            ++cur;
        }
        for (; cur != nodeList.end(); ++cur) {
            if (scrollBoundary && !IsWithinScrollBoundary(exportedRoot, cur->GetAccessibilityId(), scrollBoundary)) {
                return scrollBoundary;
            }
            if (IsSubtreeCandidateFocusable(exportedRoot, *cur, param.type, bypassDescendants)) {
                nodeInfo = *cur;
                return nullptr;
            }
        }
    } else {
        auto cur = nodeList.end();
        if (originIt != nodeList.end()) {
            cur = originIt;
        }
        while (cur != nodeList.begin()) {
            --cur;
            if (scrollBoundary && !IsWithinScrollBoundary(exportedRoot, cur->GetAccessibilityId(), scrollBoundary)) {
                return scrollBoundary;
            }
            if (IsSubtreeCandidateFocusable(exportedRoot, *cur, param.type, bypassDescendants)) {
                nodeInfo = *cur;
                return nullptr;
            }
        }
    }
    return scrollBoundary;
}

// Post-search diagnostics: selected id, grandchild-host presence in the surviving list.
void LogFocusMoveSearchPostSearch(const AccessibilityElementInfo& nodeInfo,
    const std::list<AccessibilityElementInfo>& nodeList)
{
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "FocusMoveSearchWithConditionFromRoot post-search: selectedId=%{public}" PRId64
        ", selectedChildTreeId=%{public}d, valid=%{public}d, listSize=%{public}zu"
        ", hostInList=%{public}d",
        nodeInfo.GetAccessibilityId(), nodeInfo.GetChildTreeId(),
        nodeInfo.IsValidElement() ? 1 : 0, nodeList.size(),
        [&nodeList]() {
            for (const auto& element : nodeList) {
                if (element.GetChildTreeId() > 0) {
                    return 1;
                }
            }
            return 0;
        }());
}
} // namespace

RefPtr<NG::FrameNode> JsAccessibilityManager::FindDetachFocusCandidateFromRoot(
    const RefPtr<NG::FrameNode>& focusNode, int32_t direction)
{
    CHECK_NULL_RETURN(focusNode, nullptr);
    auto root = cmpExportedRoot_.Upgrade();
    CHECK_NULL_RETURN(root, nullptr);
    auto context = GetPipelineContext().Upgrade();
    CHECK_NULL_RETURN(context, nullptr);
    std::list<AccessibilityElementInfo> nodeList;
    SearchElementInfoByAccessibilityIdFromRoot(-1, PREFETCH_RECURSIVE_CHILDREN, nodeList, context, root);
    NormalizeSubtreeSearchIds(nodeList, treeId_);
    AccessibilityElementInfo origin;
    auto originId = focusNode->GetAccessibilityId();
    FilterAccessibilityElementByFocusable(nodeList, origin, originId, true);
    RebuildReadingOrderList(nodeList, root);
    AccessibilityFocusMoveParam param {
        .direction = static_cast<Accessibility::FocusMoveDirection>(direction),
        .condition = DetailCondition::BYPASS_SELF,
    };
    AccessibilityElementInfo candidate;
    candidate.SetValidElement(false);
    auto scrollBoundary = SearchForwardBackwardCandidate(root, param, origin, originId, nodeList, candidate);
    if (scrollBoundary || !candidate.IsValidElement()) {
        return nullptr;
    }
    return NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(root, candidate.GetAccessibilityId());
}

// Grandchild-tree drill-down at the result layer for a selected child-tree host.
bool JsAccessibilityManager::TryReplyChildTreeHandoffFromRoot(const AccessibilityElementInfo& nodeInfo,
    const Accessibility::AccessibilityFocusMoveParam& param, const RefPtr<PipelineBase>& context,
    const RefPtr<NG::FrameNode>& exportedRoot, std::list<AccessibilityElementInfo>& infos,
    FocusMoveResult& result, const int32_t requestId, AccessibilityElementOperatorCallback& callback)
{
    bool bypassDescendants = param.condition == Accessibility::DetailCondition::BYPASS_SELF_DESCENDANTS
        || param.condition == Accessibility::DetailCondition::CHECK_SELF_BYPASS_DESCENDANTS;
    if (bypassDescendants || nodeInfo.GetChildTreeId() <= 0) {
        return false;
    }
#ifdef WEB_SUPPORTED
    // In-process Web grandchild drill-down: the DOUBLE_CHECK envelope delegates the grandchild search to the AA, but.
    if (TryFocusMoveSearchWebGrandchildFromRoot(nodeInfo, param, context, exportedRoot, infos, result)) {
        callback.SetFocusMoveSearchWithConditionResult(infos, result, requestId);
        return true;
    }
#endif
    result.resultType = (param.direction == Accessibility::FocusMoveDirection::BACKWARD)
        ? Accessibility::FocusMoveResultType::DOUBLE_CHECK_CHILD_PROPERTY_AND_GET_LAST
        : Accessibility::FocusMoveResultType::DOUBLE_CHECK_CHILD_PROPERTY;
    // Main-tree DOUBLE_CHECK form (ExecuteFocusMoveSearch + HandleFocusMoveSearchResult): nowLevel=-1 + needTerminate=false.
    infos.emplace_back(nodeInfo);
    result.nowLevelBelongTreeId = -1;
    result.parentWindowId = 0;
    result.needTerminate = false;
    UpdateElementInfosTreeId(infos);
    callback.SetFocusMoveSearchWithConditionResult(infos, result, requestId);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "FocusMoveSearchWithConditionFromRoot childtree handoff: hostId=%{public}" PRId64
        ", childTreeId=%{public}d, direction=%{public}d, result=%{public}d",
        nodeInfo.GetAccessibilityId(), nodeInfo.GetChildTreeId(),
        static_cast<int32_t>(param.direction), static_cast<int32_t>(result.resultType));
    return true;
}

// SEARCH_FAIL_IN_CHILDTREE anchor fill.
void JsAccessibilityManager::FillSearchFailInChildTreeRootFromRoot(const RefPtr<PipelineBase>& context,
    const RefPtr<NG::FrameNode>& exportedRoot, const AccessibilityElementInfo& nodeInfo,
    std::list<AccessibilityElementInfo>& infos)
{
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    if (ngPipeline) {
        CommonProperty commonProperty;
        GenerateCommonProperty(ngPipeline, commonProperty, context_.Upgrade(), exportedRoot);
        AccessibilityElementInfo rootInfo;
        UpdateAccessibilityElementInfo(exportedRoot, commonProperty, rootInfo, ngPipeline);
        SetRootAccessibilityVisible(exportedRoot, rootInfo);
        // Same root convention as SearchElementInfoByAccessibilityIdFromRoot: the exported root has no in-subtree parent.
        rootInfo.SetParent(INVALID_PARENT_ID);
        infos.emplace_back(rootInfo);
    } else {
        infos.emplace_back(nodeInfo);
    }
}

void JsAccessibilityManager::FocusMoveSearchWithConditionFromRoot(const AccessibilityElementInfo& info,
    const Accessibility::AccessibilityFocusMoveParam& param, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback, const RefPtr<NG::FrameNode>& exportedRoot)
{
    // Detect-focusable rounds echo the request info back to AAMS with its own tree splicing intact.
    if (FocusStrategyOsal::IsProcessDetectFocusable(param)) {
        DetectElementInfoFocusableFromRoot(info, param, requestId, callback, exportedRoot);
        return;
    }
    // Scroll-ancestor round 1 searches within the exported subtree.
    if (FocusStrategyOsal::IsProcessGetScrollAncestor(param)) {
        ProcessGetScrollAncestorFromRoot(info, param, requestId, callback, exportedRoot);
        return;
    }
    // FIND_LAST (backward entry into the subtree): descent to the subtree's last node, stopping at a grandchild host.
    if (param.direction == Accessibility::FocusMoveDirection::FIND_LAST) {
        FindLastNodeFromRoot(info, param, requestId, callback, exportedRoot);
        return;
    }
    // Unknown direction values are protocol errors, not "not found here" (A-4, aligned with the main-tree strategy).
    if (!IsSupportedFocusMoveDirection(param.direction)) {
        ReplyDirectionNotSupport(param, requestId, callback);
        return;
    }
    FocusMoveResult result = {
        .resultType = Accessibility::FocusMoveResultType::SEARCH_FAIL,
        .nowLevelBelongTreeId = -1,
        .parentWindowId = 0,
        .changeToNewInfo = false,
        .needTerminate = true,
    };
    AccessibilityElementInfo nodeInfo;
    nodeInfo.SetValidElement(false);
    std::list<AccessibilityElementInfo> infos;
    auto context = GetPipelineContext().Upgrade();
    if (!context || !exportedRoot) {
        infos.emplace_back(nodeInfo);
        callback.SetFocusMoveSearchWithConditionResult(infos, result, requestId);
        return;
    }
    // Direction search scoped to the exported subtree.
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(
        info.GetAccessibilityId(), splitElementId, splitTreeId);
#ifdef WEB_SUPPORTED
    // Dissolved-embed-content origin: the AA's continuation after the web-embed handoff lands the origin inside the same-layer-dissolved content.
    {
        AccessibilityElementInfo embedContentInfo;
        if (SearchInDissolvedEmbedContent(splitElementId, static_cast<int32_t>(param.direction),
                param.condition == Accessibility::DetailCondition::CHECK_SELF
                || param.condition == Accessibility::DetailCondition::CHECK_SELF_BYPASS_DESCENDANTS,
                static_cast<int32_t>(param.type),
                context, exportedRoot, embedContentInfo) == DissolvedEmbedSearch::FOUND) {
            infos.emplace_back(embedContentInfo);
            result.resultType = Accessibility::FocusMoveResultType::SEARCH_SUCCESS;
            result.needTerminate = true;
            callback.SetFocusMoveSearchWithConditionResult(infos, result, requestId);
            return;
        }
    }
#endif // WEB_SUPPORTED
    std::list<AccessibilityElementInfo> nodeList;
    SearchElementInfoByAccessibilityIdFromRoot(-1, PREFETCH_RECURSIVE_CHILDREN, nodeList, context, exportedRoot);
    NormalizeSubtreeSearchIds(nodeList, treeId_);
    AccessibilityElementInfo focusElement;
    FilterAccessibilityElementByFocusable(nodeList, focusElement, splitElementId, true);
    // Reading-order rebuild before the FORWARD/BACKWARD iteration (rationale in RebuildReadingOrderList).
    RebuildReadingOrderList(nodeList, exportedRoot);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "FocusMoveSearchWithConditionFromRoot pre-search: elementId=%{public}" PRId64
        ", splitElementId=%{public}" PRId64 ", splitTreeId=%{public}d, direction=%{public}d"
        ", condition=%{public}d, nodeListSize=%{public}zu, originResolved=%{public}d"
        ", originChildTreeId=%{public}d",
        info.GetAccessibilityId(), splitElementId, splitTreeId,
        static_cast<int32_t>(param.direction), static_cast<int32_t>(param.condition),
        nodeList.size(),
        splitElementId >= 0 && focusElement.GetAccessibilityId() == splitElementId,
        focusElement.GetChildTreeId());
    std::shared_ptr<FocusRulesCheckNode> scrollBoundary;
    switch (param.direction) {
        case Accessibility::FocusMoveDirection::FORWARD:
        case Accessibility::FocusMoveDirection::BACKWARD:
            // Origin-level acceptance + candidate iteration with the main-tree candidate check.
            scrollBoundary = SearchForwardBackwardCandidate(
                exportedRoot, param, focusElement, splitElementId, nodeList, nodeInfo);
            break;
        case Accessibility::FocusMoveDirection::UP:
        case Accessibility::FocusMoveDirection::DOWN:
        case Accessibility::FocusMoveDirection::LEFT:
        case Accessibility::FocusMoveDirection::RIGHT:
            FindExtensionNodeInAbsoluteDirection(focusElement, nodeList, nodeInfo, param.direction);
            break;
        default:
            break;
    }
    LogFocusMoveSearchPostSearch(nodeInfo, nodeList);
    if (scrollBoundary) {
        if (param.type == Accessibility::FocusRuleType::FOCUS_BY_TITLE
            || param.type == Accessibility::FocusRuleType::FOCUS_BY_LINK) {
            infos.emplace_back(nodeInfo);
        } else {
            FillScrollAncestorInfosFromRoot(AceType::DynamicCast<NG::PipelineContext>(context),
                { scrollBoundary }, infos);
            result.resultType = Accessibility::FocusMoveResultType::SEARCH_FAIL_IN_SCROLL;
        }
    } else if (nodeInfo.IsValidElement()) {
        if (TryReplyChildTreeHandoffFromRoot(
            nodeInfo, param, context, exportedRoot, infos, result, requestId, callback)) {
            return;
        }
        result.resultType = Accessibility::FocusMoveResultType::SEARCH_SUCCESS;
        infos.emplace_back(nodeInfo);
    } else {
        // Boundary exhausted.
        result.resultType = Accessibility::FocusMoveResultType::SEARCH_FAIL_IN_CHILDTREE;
        FillSearchFailInChildTreeRootFromRoot(context, exportedRoot, nodeInfo, infos);
    }
    // Cross-tree continuation envelope (mirrors JsThirdProviderInteractionOperation:: FocusMoveSearchWithCondition).
    result.nowLevelBelongTreeId = treeId_;
    result.parentWindowId = 0;
    result.needTerminate = false;
    // Splice T_sub into the outbound ids so AAMS can route follow-up queries back to the exported subtree operator.
    UpdateElementInfosTreeId(infos);
    callback.SetFocusMoveSearchWithConditionResult(infos, result, requestId);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "FocusMoveSearchWithConditionFromRoot: elementId=%{public}" PRId64
        ", direction=%{public}d, result=%{public}d, treeId=%{public}d, needTerminate=%{public}d"
        ", condition=%{public}d, parentId=%{public}" PRId64,
        info.GetAccessibilityId(), static_cast<int32_t>(param.direction),
        static_cast<int32_t>(result.resultType), result.nowLevelBelongTreeId, result.needTerminate,
        static_cast<int32_t>(param.condition), param.parentId);
}

namespace {
// Detect-round check node with the same exportedRoot boundary clipping.
class SubtreeClippedDetectParentCheckNode : public DetectParentRulesCheckNode {
public:
    SubtreeClippedDetectParentCheckNode(const Accessibility::AccessibilityElementInfo& elementInfo,
        const RefPtr<NG::FrameNode>& baseNode, const RefPtr<NG::FrameNode>& boundary)
        : DetectParentRulesCheckNode(elementInfo, baseNode), baseNode_(baseNode), boundary_(boundary) {}

    std::shared_ptr<FocusRulesCheckNode> GetAceParent() override
    {
        auto baseNode = baseNode_.Upgrade();
        CHECK_NULL_RETURN(baseNode, nullptr);
        return std::make_shared<SubtreeClippedCheckNode>(
            baseNode, baseNode->GetAccessibilityId(), boundary_);
    }

private:
    WeakPtr<NG::FrameNode> baseNode_;
    RefPtr<NG::FrameNode> boundary_;
};

// Detect-round base-node resolution.
RefPtr<NG::FrameNode> ResolveDetectBaseNodeFromRoot(const AccessibilityElementInfo& info,
    const Accessibility::AccessibilityFocusMoveParam& param, const RefPtr<NG::FrameNode>& exportedRoot)
{
    int64_t splitParentId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitParentTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(
        param.parentId, splitParentId, splitParentTreeId);
    RefPtr<NG::FrameNode> baseNode;
    if (splitParentId >= 0) {
        baseNode = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(exportedRoot, splitParentId);
    }
    if (!baseNode) {
        int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
        int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
        AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(
            info.GetAccessibilityId(), splitElementId, splitTreeId);
        auto foundNode = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(
            exportedRoot, splitElementId);
        if (foundNode) {
            if (foundNode == exportedRoot) {
                baseNode = exportedRoot;
            } else {
                auto foundCheckNode = std::make_shared<FrameNodeRulesCheckNode>(
                    foundNode, foundNode->GetAccessibilityId());
                auto parentCheckNode = foundCheckNode->GetAceParent();
                baseNode = parentCheckNode
                    ? std::static_pointer_cast<FrameNodeRulesCheckNode>(parentCheckNode)->GetFrameNode()
                    : nullptr;
            }
        }
    }
    return baseNode;
}

// Scroll-ancestor origin id: round 1 starts at the requesting node.
int64_t ResolveScrollOriginIdFromRoot(const AccessibilityElementInfo& info,
    const Accessibility::AccessibilityFocusMoveParam& param)
{
    int64_t splitId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    if (param.detectParent) {
        AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(
            param.parentId, splitId, splitTreeId);
    } else {
        AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(
            info.GetAccessibilityId(), splitId, splitTreeId);
    }
    return splitId;
}

// FIND_LAST addressing: -1 means the subtree root (FromRoot convention).
bool ResolveFindLastOriginNodeFromRoot(const AccessibilityElementInfo& info,
    const RefPtr<NG::FrameNode>& exportedRoot, RefPtr<NG::FrameNode>& originNode)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(
        info.GetAccessibilityId(), splitElementId, splitTreeId);
    if (splitElementId < 0) {
        originNode = exportedRoot;
        return true;
    }
    auto resolved = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(
        exportedRoot, splitElementId);
    if (!resolved) {
        return false;
    }
    originNode = resolved;
    return true;
}
} // namespace

// Hover variant substitution: the touched node is not readable.
std::optional<AccessibilityElementInfo> JsAccessibilityManager::FindReadableAncestorFromRoot(
    const RefPtr<NG::FrameNode>& baseNode, const RefPtr<NG::FrameNode>& exportedRoot,
    const Accessibility::AccessibilityFocusMoveParam& param, const RefPtr<PipelineBase>& context)
{
    auto client = AccessibilitySystemAbilityClient::GetInstance();
    CHECK_NULL_RETURN(client, std::nullopt);
    auto candidateCheckNode = std::make_shared<FrameNodeRulesCheckNode>(
        baseNode, baseNode->GetAccessibilityId());
    while (candidateCheckNode) {
        bool candidateFocusType = CheckNodeMatchedFocusType(candidateCheckNode, param.type);
        bool candidateReadable = false;
        bool readableOk = client->CheckNodeIsReadable(candidateCheckNode, candidateReadable) == RET_OK;
        if (candidateFocusType && readableOk && candidateReadable) {
            auto candidateNode = candidateCheckNode->GetFrameNode();
            auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
            if (candidateNode && ngPipeline) {
                CommonProperty commonProperty;
                GenerateCommonProperty(ngPipeline, commonProperty, context_.Upgrade(), candidateNode);
                AccessibilityElementInfo targetInfo;
                UpdateAccessibilityElementInfo(candidateNode, commonProperty, targetInfo, ngPipeline);
                SetRootAccessibilityVisible(candidateNode, targetInfo);
                UpdateElementInfoTreeId(targetInfo);
                return targetInfo;
            }
            break;
        }
        auto candidateNode = candidateCheckNode->GetFrameNode();
        if (!candidateNode || candidateNode == exportedRoot) {
            break;
        }
        candidateCheckNode = std::static_pointer_cast<FrameNodeRulesCheckNode>(
            candidateCheckNode->GetAceParent());
    }
    return std::nullopt;
}

void JsAccessibilityManager::DetectElementInfoFocusableFromRoot(const AccessibilityElementInfo& info,
    const Accessibility::AccessibilityFocusMoveParam& param, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback, const RefPtr<NG::FrameNode>& exportedRoot)
{
    // Envelope convention of the exported-subtree operator (FocusMoveSearchWithConditionFromRoot): T_sub marking + in-window parent (0).
    FocusMoveResult result = {
        .resultType = Accessibility::FocusMoveResultType::SEARCH_FAIL,
        .nowLevelBelongTreeId = treeId_,
        .parentWindowId = 0,
        .changeToNewInfo = false,
        .needTerminate = false,
    };
    std::list<AccessibilityElementInfo> targetInfos;
    auto context = GetPipelineContext().Upgrade();
    if (!context || !exportedRoot) {
        callback.SetFocusMoveSearchWithConditionResult(targetInfos, result, requestId);
        return;
    }
    auto baseNode = ResolveDetectBaseNodeFromRoot(info, param, exportedRoot);
    if (!baseNode) {
        callback.SetFocusMoveSearchWithConditionResult(targetInfos, result, requestId);
        return;
    }
    bool isInFocusMove = param.direction == Accessibility::FocusMoveDirection::DETECT_FOCUSABLE_IN_FOCUS_MOVE;
    bool changeToAncestorFocusable = !isInFocusMove;
    bool needCheckValid = isInFocusMove;
    // Parent-chain clipping at exportedRoot (SubtreeClippedDetectParentCheckNode): every AA-side rule check below.
    auto checkNode = std::make_shared<SubtreeClippedDetectParentCheckNode>(info, baseNode, exportedRoot);
    result.resultType = Accessibility::FocusMoveResultType::SEARCH_SUCCESS;
    targetInfos.emplace_back(info);
    auto client = AccessibilitySystemAbilityClient::GetInstance();
    if (!client) {
        callback.SetFocusMoveSearchWithConditionResult(targetInfos, result, requestId);
        return;
    }
    bool isFocusType = CheckNodeMatchedFocusType(checkNode, param.type);
    bool isReadable = true;
    auto checkResult = client->CheckNodeIsReadable(checkNode, isReadable);
    if (checkResult != RET_OK) {
        callback.SetFocusMoveSearchWithConditionResult(targetInfos, result, requestId);
        return;
    }
    if (needCheckValid && isFocusType && isReadable) {
        // Availability (popup occlusion) check on the clipped chain: only ancestors up to exportedRoot are consulted.
        bool isAvailable = true;
        auto availableResult = client->CheckNodeIsSpecificType(
            checkNode, Accessibility::ReadableSpecificType::AVAILABLE_TYPE, isAvailable);
        if (availableResult == RET_OK && !isAvailable) {
            result.resultType = Accessibility::FocusMoveResultType::SEARCH_FAIL;
            callback.SetFocusMoveSearchWithConditionResult(targetInfos, result, requestId);
            return;
        }
    }
    if (!isFocusType && !isReadable && changeToAncestorFocusable) {
        bool substituted = false;
        auto targetInfo = FindReadableAncestorFromRoot(baseNode, exportedRoot, param, context);
        if (targetInfo.has_value()) {
            targetInfos.clear();
            targetInfos.emplace_back(*targetInfo);
            substituted = true;
        }
        result.resultType = substituted ? Accessibility::FocusMoveResultType::SEARCH_SUCCESS
                                        : Accessibility::FocusMoveResultType::SEARCH_FAIL;
        result.changeToNewInfo = substituted;
        callback.SetFocusMoveSearchWithConditionResult(targetInfos, result, requestId);
        return;
    }
    bool canFocus = isFocusType && isReadable;
    result.resultType = canFocus ? Accessibility::FocusMoveResultType::SEARCH_SUCCESS
                                 : Accessibility::FocusMoveResultType::SEARCH_FAIL;
    if (isInFocusMove && result.resultType == Accessibility::FocusMoveResultType::SEARCH_FAIL) {
        // Same protocol as the main tree: not a valid focus stop.
        result.resultType = Accessibility::FocusMoveResultType::SEARCH_NEXT;
        result.needTerminate = true;
    }
    callback.SetFocusMoveSearchWithConditionResult(targetInfos, result, requestId);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "DetectElementInfoFocusableFromRoot: elementId=%{public}" PRId64
        ", parentId=%{public}" PRId64 ", direction=%{public}d, result=%{public}d, treeId=%{public}d"
        ", componentType=%{public}s, isFocusType=%{public}d, isReadable=%{public}d",
        info.GetAccessibilityId(), param.parentId, static_cast<int32_t>(param.direction),
        static_cast<int32_t>(result.resultType), result.nowLevelBelongTreeId,
        info.GetComponentType().c_str(), isFocusType ? 1 : 0, isReadable ? 1 : 0);
}

// Fill the scroll-ancestor reply infos from the strategy walk's collected check nodes.
void JsAccessibilityManager::FillScrollAncestorInfosFromRoot(const RefPtr<NG::PipelineContext>& ngPipeline,
    const std::list<std::shared_ptr<FocusRulesCheckNode>>& targetNodes,
    std::list<AccessibilityElementInfo>& infos)
{
    for (auto& targetNode : targetNodes) {
        auto targetCheckNode = std::static_pointer_cast<FrameNodeRulesCheckNode>(targetNode);
        auto node = targetCheckNode ? targetCheckNode->GetFrameNode() : nullptr;
        if (!node) {
            continue;
        }
        CommonProperty commonProperty;
        GenerateCommonProperty(ngPipeline, commonProperty, context_.Upgrade(), node);
        AccessibilityElementInfo targetInfo;
        UpdateAccessibilityElementInfo(node, commonProperty, targetInfo, ngPipeline);
        SetRootAccessibilityVisible(node, targetInfo);
        infos.emplace_back(targetInfo);
    }
}

void JsAccessibilityManager::ProcessGetScrollAncestorFromRoot(const AccessibilityElementInfo& info,
    const Accessibility::AccessibilityFocusMoveParam& param, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback, const RefPtr<NG::FrameNode>& exportedRoot)
{
    // Envelope conventions of the exported-subtree operator: T_sub marking + in-window parent (0).
    FocusMoveResult result = {
        .resultType = Accessibility::FocusMoveResultType::SEARCH_FAIL,
        .nowLevelBelongTreeId = treeId_,
        .parentWindowId = 0,
        .changeToNewInfo = false,
        .needTerminate = false,
    };
    std::list<AccessibilityElementInfo> infos;
    auto context = GetPipelineContext().Upgrade();
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    if (!context || !ngPipeline || !exportedRoot) {
        callback.SetFocusMoveSearchWithConditionResult(infos, result, requestId);
        return;
    }
    // Round 1 starts at the requesting node.
    int64_t originId = ResolveScrollOriginIdFromRoot(info, param);
    auto originNode = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(exportedRoot, originId);
    if (!originNode) {
        result.resultType = Accessibility::FocusMoveResultType::SEARCH_FAIL_LOST_NODE;
        result.needTerminate = true;
        callback.SetFocusMoveSearchWithConditionResult(infos, result, requestId);
        return;
    }
    // Shared main-tree walk (AccessibilityFocusStrategy::FindScrollAncestor) on a check node clipped at exportedRoot.
    std::list<std::shared_ptr<FocusRulesCheckNode>> targetNodes;
    AccessibilityFocusStrategy strategy;
    auto bypassSelf = param.condition == Accessibility::DetailCondition::CHECK_SELF ? false : true;
    AceFocusMoveDetailCondition condition = {.bypassSelf = bypassSelf, .bypassDescendants = false};
    auto checkNode = std::make_shared<SubtreeClippedCheckNode>(
        originNode, originNode->GetAccessibilityId(), exportedRoot);
    switch (param.direction) {
        case Accessibility::FocusMoveDirection::GET_BACKWARD_SCROLL_ANCESTOR:
            strategy.FindBackwardScrollAncestor(condition, checkNode, targetNodes);
            break;
        case Accessibility::FocusMoveDirection::GET_FORWARD_SCROLL_ANCESTOR:
            strategy.FindForwardScrollAncestor(condition, checkNode, targetNodes);
            break;
        default:
            strategy.FindAnyScrollAncestor(condition, checkNode, targetNodes);
            break;
    }
    FillScrollAncestorInfosFromRoot(ngPipeline, targetNodes, infos);
    if (!infos.empty()) {
        UpdateElementInfosTreeId(infos);
        result.resultType = Accessibility::FocusMoveResultType::SEARCH_SUCCESS;
    }
    callback.SetFocusMoveSearchWithConditionResult(infos, result, requestId);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "ProcessGetScrollAncestorFromRoot: elementId=%{public}" PRId64
        ", direction=%{public}d, detectParent=%{public}d, result=%{public}d, treeId=%{public}d, size=%{public}zu",
        info.GetAccessibilityId(), static_cast<int32_t>(param.direction),
        static_cast<int32_t>(param.detectParent), static_cast<int32_t>(result.resultType),
        result.nowLevelBelongTreeId, infos.size());
}

// FIND_LAST result completion: FIND_CHILDTREE hands off with the GET_LAST variant.
bool JsAccessibilityManager::CompleteFindLastResultFromRoot(const AccessibilityElementInfo& targetInfo,
    const Accessibility::AccessibilityFocusMoveParam& param, const RefPtr<PipelineBase>& context,
    const RefPtr<NG::FrameNode>& exportedRoot, AceFocusMoveResult moveResult,
    std::list<AccessibilityElementInfo>& infos, FocusMoveResult& result)
{
    result.nowLevelBelongTreeId = -1;
    if (moveResult == AceFocusMoveResult::FIND_CHILDTREE) {
#ifdef WEB_SUPPORTED
        // In-process Web grandchild drill-down (same rationale as the handoff in FocusMoveSearchWithConditionFromRoot).
        if (TryFocusMoveSearchWebGrandchildFromRoot(targetInfo, param, context, exportedRoot, infos, result)) {
            return true;
        }
#endif
        infos.emplace_back(targetInfo);
        UpdateElementInfosTreeId(infos);
        result.resultType = Accessibility::FocusMoveResultType::DOUBLE_CHECK_CHILD_PROPERTY_AND_GET_LAST;
    } else {
        // Terminal success (Web child-tree precedent): the AA stops on this reply and lands on the last node.
        infos.emplace_back(targetInfo);
        UpdateElementInfosTreeId(infos);
        result.resultType = Accessibility::FocusMoveResultType::SEARCH_SUCCESS;
        result.needTerminate = true;
    }
    return false;
}

void JsAccessibilityManager::FindLastNodeFromRoot(const AccessibilityElementInfo& info,
    const Accessibility::AccessibilityFocusMoveParam& param, const int32_t requestId,
    AccessibilityElementOperatorCallback& callback, const RefPtr<NG::FrameNode>& exportedRoot)
{
    // Envelope conventions of the exported-subtree operator: T_sub marking + in-window parent (0) on failure paths.
    FocusMoveResult result = {
        .resultType = Accessibility::FocusMoveResultType::SEARCH_FAIL,
        .nowLevelBelongTreeId = treeId_,
        .parentWindowId = 0,
        .changeToNewInfo = false,
        .needTerminate = false,
    };
    std::list<AccessibilityElementInfo> infos;
    auto context = GetPipelineContext().Upgrade();
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    if (!context || !ngPipeline || !exportedRoot) {
        AccessibilityElementInfo nodeInfo;
        nodeInfo.SetValidElement(false);
        infos.emplace_back(nodeInfo);
        callback.SetFocusMoveSearchWithConditionResult(infos, result, requestId);
        return;
    }
    // FIND_LAST addressing: an unresolvable id is a lost origin (LOST_NODE terminates).
    RefPtr<NG::FrameNode> originNode;
    if (!ResolveFindLastOriginNodeFromRoot(info, exportedRoot, originNode)) {
        result.resultType = Accessibility::FocusMoveResultType::SEARCH_FAIL_LOST_NODE;
        result.needTerminate = true;
        AccessibilityElementInfo nodeInfo;
        nodeInfo.SetValidElement(false);
        infos.emplace_back(nodeInfo);
        callback.SetFocusMoveSearchWithConditionResult(infos, result, requestId);
        return;
    }
    // Shared main-tree descent (AccessibilityFocusStrategy::FindLastNodeWithoutCheck -> GetLastChildNode): last-child.
    std::shared_ptr<FocusRulesCheckNode> targetNode;
    auto checkNode = std::make_shared<FrameNodeRulesCheckNode>(
        originNode, originNode->GetAccessibilityId());
    auto bypassSelf = param.condition == Accessibility::DetailCondition::CHECK_SELF ? false : true;
    AceFocusMoveDetailCondition condition = {.bypassSelf = bypassSelf, .bypassDescendants = false};
    AccessibilityFocusStrategy strategy;
    auto moveResult = strategy.FindLastNodeWithoutCheck(condition, checkNode, targetNode);
    auto targetCheckNode = std::static_pointer_cast<FrameNodeRulesCheckNode>(targetNode);
    auto node = targetCheckNode ? targetCheckNode->GetFrameNode() : nullptr;
    if ((moveResult != AceFocusMoveResult::FIND_SUCCESS
        && moveResult != AceFocusMoveResult::FIND_CHILDTREE) || !node) {
        AccessibilityElementInfo nodeInfo;
        nodeInfo.SetValidElement(false);
        infos.emplace_back(nodeInfo);
        callback.SetFocusMoveSearchWithConditionResult(infos, result, requestId);
        return;
    }
    // Default-constructed info is valid (main-tree convention): only failure paths above invalidate it explicitly.
    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, context_.Upgrade(), node);
    AccessibilityElementInfo targetInfo;
    UpdateAccessibilityElementInfo(node, commonProperty, targetInfo, ngPipeline);
    SetRootAccessibilityVisible(node, targetInfo);
    bool webDrillHit = CompleteFindLastResultFromRoot(
        targetInfo, param, context, exportedRoot, moveResult, infos, result);
    callback.SetFocusMoveSearchWithConditionResult(infos, result, requestId);
    if (webDrillHit) {
        TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
            "FindLastNodeFromRoot web grandchild drill: elementId=%{public}" PRId64
            ", hostId=%{public}" PRId64 ", result=%{public}d, needTerminate=%{public}d",
            info.GetAccessibilityId(), node->GetAccessibilityId(),
            static_cast<int32_t>(result.resultType), result.needTerminate);
        return;
    }
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "FindLastNodeFromRoot: elementId=%{public}" PRId64
        ", lastNodeId=%{public}" PRId64 ", moveResult=%{public}d, result=%{public}d, treeId=%{public}d"
        ", needTerminate=%{public}d, condition=%{public}d",
        info.GetAccessibilityId(), node->GetAccessibilityId(), static_cast<int32_t>(moveResult),
        static_cast<int32_t>(result.resultType), result.nowLevelBelongTreeId, result.needTerminate,
        static_cast<int32_t>(param.condition));
}

void JsAccessibilityManager::FindFocusedElementInfoFromRoot(int64_t elementId, int32_t focusType,
    Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
    const RefPtr<NG::FrameNode>& exportedRoot)
{
    // Main-tree alignment for the valid flag: the info generation chain below never resets it, so only genuine.
    if (focusType != FOCUS_TYPE_INPUT && focusType != FOCUS_TYPE_ACCESSIBILITY) {
        info.SetValidElement(false);
        return;
    }
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    CHECK_NULL_VOID(exportedRoot);
    int64_t nodeId = elementId;
    if (elementId == -1) {
        nodeId = exportedRoot->GetAccessibilityId();
    }
    auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(exportedRoot, nodeId);
    if (!node) {
        info.SetValidElement(false);
        return;
    }
    // Find the focus within the subtree rooted at the found node.
    RefPtr<NG::FrameNode> resultNode;
    if (focusType == FOCUS_TYPE_ACCESSIBILITY) {
        resultNode = FindAccessibilityFocus(node, focusType, info, NG::UI_EXTENSION_OFFSET_MAX, context,
            currentFocusNodeId_);
    }
    if (focusType == FOCUS_TYPE_INPUT) {
        resultNode = FindInputFocus(node, focusType, info, NG::UI_EXTENSION_OFFSET_MAX, context);
    }
    CHECK_NULL_VOID(resultNode);
    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext, resultNode);
    UpdateAccessibilityElementInfo(resultNode, commonProperty, info, ngPipeline);
    SetRootAccessibilityVisible(resultNode, info);
    SetRootAccessibilityNextFocusId(resultNode, exportedRoot, info);
    SetRootAccessibilityPreFocusId(resultNode, exportedRoot, info,
        nextFocusRelationController_.GetTargetMap(context->GetInstanceId()));
}

void JsAccessibilityManager::SearchElementInfosByTextFromRoot(int64_t elementId, const std::string& text,
    std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context,
    const RefPtr<NG::FrameNode>& exportedRoot)
{
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    CHECK_NULL_VOID(exportedRoot);
    if (text.empty() || elementId == -1) {
        return;
    }
    auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(exportedRoot, elementId);
    CHECK_NULL_VOID(node);
    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext, node);
    commonProperty.checkEmbedNode = false;
    nlohmann::json textJson = nlohmann::json::parse(text, nullptr, false);
    if (textJson.is_null() || textJson.is_discarded() || !textJson.contains("type")) {
        return;
    }
    if (textJson["type"] == "textType") {
        SearchParameter param {0, text, 0, NG::UI_EXTENSION_OFFSET_MAX};
        FindTextByTextHint(node, infos, ngPipeline, commonProperty, param);
        return;
    }
    if (!textJson.contains("value")) {
        return;
    }
    SearchParameter param {0, textJson["value"], 0, NG::UI_EXTENSION_OFFSET_MAX};
    FindText(node, infos, ngPipeline, commonProperty, param);
}

void JsAccessibilityManager::SearchElementInfoByCustomIdFromRoot(int64_t elementId, const std::string& customId,
    std::list<Accessibility::AccessibilityElementInfo>& infos,
    std::list<Accessibility::AccessibilityElementInfo>& treeInfos, const RefPtr<PipelineBase>& context,
    const RefPtr<NG::FrameNode>& exportedRoot)
{
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    CHECK_NULL_VOID(exportedRoot);
    CommonProperty commonProperty;
    GenerateCommonProperty(ngPipeline, commonProperty, mainContext, exportedRoot);
    commonProperty.checkEmbedNode = false;
    int64_t nodeId = elementId;
    if (elementId == -1) {
        nodeId = exportedRoot->GetAccessibilityId();
    }
    auto node = NG::AccessibilityFrameNodeUtils::GetFramenodeByAccessibilityId(exportedRoot, nodeId);
    CHECK_NULL_VOID(node);
    // BFS within the exported subtree.
    std::list<RefPtr<NG::FrameNode>> children { node };
    while (!children.empty()) {
        auto checkNode = children.front();
        children.pop_front();
        if (SetAccessibilityCustomId(checkNode, customId, commonProperty, infos, context)) {
            treeInfos.clear();
            break;
        }
        for (const auto& child : checkNode->GetChildren(true)) {
            children.emplace_back(AceType::DynamicCast<NG::FrameNode>(child));
        }
    }
}

void JsAccessibilityManager::SearchDefaultFocusFromRoot(std::list<Accessibility::AccessibilityElementInfo>& infos,
    const RefPtr<PipelineBase>& context, const RefPtr<NG::FrameNode>& exportedRoot)
{
    auto mainContext = context_.Upgrade();
    CHECK_NULL_VOID(mainContext);
    auto ngPipeline = AceType::DynamicCast<NG::PipelineContext>(context);
    CHECK_NULL_VOID(ngPipeline);
    CHECK_NULL_VOID(exportedRoot);
    // Collect default-focus nodes within the exported subtree.
    std::list<RefPtr<NG::UINode>> nodes { exportedRoot };
    while (!nodes.empty()) {
        auto current = nodes.front();
        nodes.pop_front();
        auto frameNode = AceType::DynamicCast<NG::FrameNode>(current);
        if (frameNode && frameNode->GetFocusHub() && frameNode->GetFocusHub()->IsDefaultFocus()) {
            AccessibilityElementInfo nodeInfo;
            CommonProperty commonProperty;
            GenerateCommonProperty(ngPipeline, commonProperty, mainContext, frameNode);
            UpdateAccessibilityElementInfo(frameNode, commonProperty, nodeInfo, ngPipeline);
            infos.emplace_back(nodeInfo);
        }
        for (const auto& child : current->GetChildren(true)) {
            nodes.emplace_back(child);
        }
    }
}

std::optional<JsAccessibilityManager::CmpBindingQueryResult> JsAccessibilityManager::GetCmpBindingForHover(
    int32_t ownerNodeId, int64_t bindId)
{
    std::lock_guard<std::mutex> lock(cmpBindingMapMutex_);
    auto it = cmpBindingMap_.find({ ownerNodeId, bindId });
    if (it == cmpBindingMap_.end() || it->second.state != CmpBindingState::State::REGISTERED) {
        return std::nullopt;
    }
    return CmpBindingQueryResult { it->second.exportedRoot, it->second.subAM };
}

std::vector<int64_t> JsAccessibilityManager::GetCmpBindingBindIdsForHover(int32_t ownerNodeId)
{
    std::vector<int64_t> bindIds;
    std::lock_guard<std::mutex> lock(cmpBindingMapMutex_);
    for (const auto& [key, state] : cmpBindingMap_) {
        if (key.first == ownerNodeId && state.state == CmpBindingState::State::REGISTERED) {
            bindIds.push_back(key.second);
        }
    }
    return bindIds;
}

namespace {
// Integration contract guard (log-only): the third-party framework is expected to set no-hide-descendants on the.
void WarnBindOwnerLevelIfNeeded(NG::FrameNode* owner)
{
    auto property = owner->GetAccessibilityProperty<NG::AccessibilityProperty>();
    if (property && property->GetAccessibilityLevel() != NG::AccessibilityProperty::Level::NO_HIDE_DESCENDANTS) {
        TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
            "BindExportTree: owner=%{public}d accessibilityLevel is not no-hide-descendants "
            "(third-party contract) — subtree will be exposed in both trees", owner->GetId());
    }
}

// Create the binding's subAM at Bind entry.
RefPtr<JsAccessibilityManager> CreateCmpSubAM(NG::FrameNode* owner)
{
    auto subAM = AceType::MakeRefPtr<JsAccessibilityManager>();
    auto context = owner->GetContext();
    if (!subAM || context == nullptr) {
        return nullptr;
    }
    // context is NG::PipelineContext* owned by the pipeline.
    subAM->SetPipelineContext(AceType::Claim<PipelineBase>(static_cast<PipelineBase*>(context)));
    // Mark the subAM: grandchild Web trees answer exhaustion with the cross-tree continuation envelope.
    subAM->MarkCmpSubAM();
    return subAM;
}
} // namespace

bool JsAccessibilityManager::BindExportTree(NG::FrameNode* owner, int64_t bindId,
    NG::FrameNode* exportedRoot)
{
    CHECK_NULL_RETURN(owner, false);
    CHECK_NULL_RETURN(exportedRoot, false);
    bool retry = false;
    {
        std::lock_guard<std::mutex> lock(cmpBindingMapMutex_);
        auto it = cmpBindingMap_.find(std::make_pair(owner->GetId(), bindId));
        if (it != cmpBindingMap_.end() && it->second.state != CmpBindingState::State::UNBOUND) {
            if (it->second.state == CmpBindingState::State::REGISTERING &&
                it->second.registration && it->second.registration->IsCanceled()) {
                it->second.state = CmpBindingState::State::WAIT_TREE_ID;
                it->second.registration.reset();
                it->second.windowId = 0;
            }
            if (it->second.state != CmpBindingState::State::WAIT_TREE_ID) {
                return true;
            }
            retry = true;
        }
    }
    if (retry) {
        int32_t cmpTreeId = GetThirdProviderTreeIdForHost(owner);
        if (cmpTreeId != AccessibilityElementInfo::UNDEFINED_TREE_ID) {
            OnCmpTreeRegistered(owner, cmpTreeId);
        }
        return true;
    }
    // Adoption of exportedRoot is NOT a precondition (CAPI contract): Bind works on any subtree root.
    WarnBindOwnerLevelIfNeeded(owner);
    auto subAM = CreateCmpSubAM(owner);
    if (!subAM) {
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(cmpBindingMapMutex_);
        auto key = std::make_pair(owner->GetId(), bindId);
        auto it = cmpBindingMap_.find(key);
        if (it != cmpBindingMap_.end() && it->second.state != CmpBindingState::State::UNBOUND) {
            // Idempotent: re-bind on an active binding is a no-op.
            TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
                "BindExportTree: bindId=%{public}" PRId64 " already active (idempotent)", bindId);
            return true;
        }
        // One exportedRoot supports at most one active binding (subAM registry is keyed by it).
        bool rootInUse = false;
        for (const auto& [existingKey, state] : cmpBindingMap_) {
            auto root = state.exportedRoot.Upgrade();
            if (state.state != CmpBindingState::State::UNBOUND && root &&
                root->GetId() == exportedRoot->GetId()) {
                rootInUse = true;
                break;
            }
        }
        if (rootInUse) {
            TAG_LOGW(AceLogTag::ACE_ACCESSIBILITY,
                "BindExportTree: exportedRoot=%{public}d already bound to another bindId",
                exportedRoot->GetId());
            return false;
        }
        auto& state = cmpBindingMap_[key];
        state.owner = WeakClaim(owner);
        state.exportedRoot = WeakClaim(exportedRoot);
        state.subAM = subAM;
        state.cmpTreeId = -1;
        state.windowId = 0;
        state.state = CmpBindingState::State::WAIT_TREE_ID;
    }
    // Node registry entry (single exportedRoot key).
    CmpAccessibility::CmpAccessibilityRegistry::RegisterSubAM(exportedRoot,
        OHOS::Ace::AceType::DynamicCast<OHOS::Ace::AceType>(subAM));
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "BindExportTree: bindId=%{public}" PRId64 ", owner=%{public}d, exportedRoot=%{public}d",
        bindId, owner->GetId(), exportedRoot->GetId());
    // Branch: if the CMP tree already has its treeId, converge immediately.
    int32_t cmpTreeId = GetThirdProviderTreeIdForHost(owner);
    if (cmpTreeId != AccessibilityElementInfo::UNDEFINED_TREE_ID) {
        OnCmpTreeRegistered(owner, cmpTreeId);
    } else {
        TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
            "BindExportTree: bindId=%{public}" PRId64 " waits for CMP treeId", bindId);
    }
    return true;
}

void JsAccessibilityManager::UnbindExportTree(NG::FrameNode* owner, int64_t bindId)
{
    CHECK_NULL_VOID(owner);
    DetachCmpBinding(std::make_pair(owner->GetId(), bindId));
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY, "UnbindExportTree: bindId=%{public}" PRId64, bindId);
}

// ============================================================================
// CmpInteractionOperation.
// ============================================================================

void JsAccessibilityManager::CmpInteractionOperation::ReturnFocusFallback(
    Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t requestId)
{
    // Cross-tree focus fallback: return the host so AAMS routes focus back to the CMP tree.
    AccessibilityElementInfo info;
    // Outbound convention (FillNodeConfig / UpdateElementInfoTreeId): element ids sent to AAMS carry the belongTreeId high bits.
    int64_t hostElementId = hostElementId_;
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(parentTreeId_, hostElementId);
    info.SetAccessibilityId(hostElementId);
    info.SetBelongTreeId(parentTreeId_);
    info.SetValidElement(true);
    callback.SetFocusMoveSearchResult(info, requestId);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "CmpOp focus fallback: hostElementId=%{public}" PRId64 ", parentTreeId=%{public}d",
        hostElementId_, parentTreeId_);
}

void JsAccessibilityManager::CmpInteractionOperation::ReturnFocusFallbackWithCondition(
    int32_t parentTreeId, int64_t hostElementId,
    Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t requestId)
{
    // F-05 cross-tree focus fallback through the with-condition channel: same host anchor as ReturnFocusFallback.
    AccessibilityElementInfo info;
    // Outbound convention (FillNodeConfig / UpdateElementInfoTreeId): element ids sent to AAMS carry the belongTreeId high bits.
    int64_t splicedHostId = hostElementId;
    AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(parentTreeId, splicedHostId);
    info.SetAccessibilityId(splicedHostId);
    info.SetBelongTreeId(parentTreeId);
    info.SetValidElement(true);
    std::list<AccessibilityElementInfo> infos;
    infos.emplace_back(info);
    FocusMoveResult result = {
        .resultType = Accessibility::FocusMoveResultType::SEARCH_FAIL,
        .nowLevelBelongTreeId = parentTreeId,
        .parentWindowId = 0,
        .changeToNewInfo = false,
        .needTerminate = false,
    };
    callback.SetFocusMoveSearchWithConditionResult(infos, result, requestId);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "CmpOp focus fallback with condition: hostElementId=%{public}" PRId64
        ", parentTreeId=%{public}d, requestId=%{public}d",
        hostElementId, parentTreeId, requestId);
}

RetError JsAccessibilityManager::CmpInteractionOperation::SearchElementInfoByAccessibilityId(
    const int64_t elementId, const int32_t requestId,
    Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t mode)
{
    // Inbound ids from AAMS carry the belongTreeId high bits (echoed from our own outbound infos).
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);
    auto subAM = js_.Upgrade();
    auto exportedRoot = exportedRoot_.Upgrade();
    if (!subAM || !exportedRoot) {
        std::list<AccessibilityElementInfo> infos;
        callback.SetSearchElementInfoByAccessibilityIdResult(infos, requestId);
        return RET_OK;
    }
    auto context = subAM->GetPipelineContext().Upgrade();
    if (!context || !context->GetTaskExecutor()) {
        std::list<AccessibilityElementInfo> infos;
        callback.SetSearchElementInfoByAccessibilityIdResult(infos, requestId);
        return RET_OK;
    }
    // Whole-subtree search on the UI thread (same as ByText/ByProperty siblings), then splice T_sub into the outbound.
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), root = exportedRoot_, splitElementId, mode, requestId, &callback] {
            auto subAM = weak.Upgrade();
            auto exportedRoot = root.Upgrade();
            std::list<AccessibilityElementInfo> infos;
            if (subAM && exportedRoot) {
                subAM->SearchElementInfoByAccessibilityIdFromRoot(splitElementId, mode, infos,
                    subAM->GetPipelineContext().Upgrade(), exportedRoot);
                subAM->UpdateElementInfosTreeId(infos);
            }
            callback.SetSearchElementInfoByAccessibilityIdResult(infos, requestId);
        }, TaskExecutor::TaskType::UI, "ArkUICmpSearchById");
    return RET_OK;
}

void JsAccessibilityManager::CmpInteractionOperation::SearchElementInfoBySpecificProperty(
    const int64_t elementId, const SpecificPropertyParam& param, const int32_t requestId,
    Accessibility::AccessibilityElementOperatorCallback& callback)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);
    auto subAM = js_.Upgrade();
    auto exportedRoot = exportedRoot_.Upgrade();
    if (!subAM || !exportedRoot) {
        callback.SetSearchElementInfoBySpecificPropertyResult({}, {}, requestId);
        return;
    }
    auto context = subAM->GetPipelineContext().Upgrade();
    if (!context || !context->GetTaskExecutor()) {
        callback.SetSearchElementInfoBySpecificPropertyResult({}, {}, requestId);
        return;
    }
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), root = exportedRoot_, splitElementId, param, requestId, &callback] {
            auto subAM = weak.Upgrade();
            auto exportedRoot = root.Upgrade();
            std::list<AccessibilityElementInfo> infos;
            std::list<AccessibilityElementInfo> treeInfos;
            if (subAM && exportedRoot &&
                param.propertyType == Accessibility::SEARCH_TYPE::CUSTOMID) {
                subAM->SearchElementInfoByCustomIdFromRoot(splitElementId, param.propertyTarget, infos, treeInfos,
                    subAM->GetPipelineContext().Upgrade(), exportedRoot);
                subAM->UpdateElementInfosTreeId(infos);
                subAM->UpdateElementInfosTreeId(treeInfos);
            }
            callback.SetSearchElementInfoBySpecificPropertyResult(infos, treeInfos, requestId);
        }, TaskExecutor::TaskType::UI, "ArkUICmpSearchByProperty");
}

void JsAccessibilityManager::CmpInteractionOperation::SearchElementInfosByText(
    const int64_t elementId, const std::string& text, const int32_t requestId,
    Accessibility::AccessibilityElementOperatorCallback& callback)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);
    auto subAM = js_.Upgrade();
    auto exportedRoot = exportedRoot_.Upgrade();
    if (!subAM || !exportedRoot || text.empty()) {
        callback.SetSearchElementInfoByTextResult({}, requestId);
        return;
    }
    auto context = subAM->GetPipelineContext().Upgrade();
    if (!context || !context->GetTaskExecutor()) {
        callback.SetSearchElementInfoByTextResult({}, requestId);
        return;
    }
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), root = exportedRoot_, splitElementId, text, requestId, &callback] {
            auto subAM = weak.Upgrade();
            auto exportedRoot = root.Upgrade();
            std::list<AccessibilityElementInfo> infos;
            if (subAM && exportedRoot) {
                subAM->SearchElementInfosByTextFromRoot(splitElementId, text, infos,
                    subAM->GetPipelineContext().Upgrade(), exportedRoot);
                subAM->UpdateElementInfosTreeId(infos);
            }
            callback.SetSearchElementInfoByTextResult(std::move(infos), requestId);
        }, TaskExecutor::TaskType::UI, "ArkUICmpSearchByText");
}

void JsAccessibilityManager::CmpInteractionOperation::SearchDefaultFocusByWindowId(
    const int32_t windowId, const int32_t requestId,
    Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t pageId)
{
    auto subAM = js_.Upgrade();
    auto exportedRoot = exportedRoot_.Upgrade();
    if (!subAM || !exportedRoot) {
        std::list<AccessibilityElementInfo> infos;
        callback.SetSearchDefaultFocusByWindowIdResult(infos, requestId);
        return;
    }
    auto context = subAM->GetPipelineContext().Upgrade();
    if (!context || !context->GetTaskExecutor()) {
        std::list<AccessibilityElementInfo> infos;
        callback.SetSearchDefaultFocusByWindowIdResult(infos, requestId);
        return;
    }
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), root = exportedRoot_, requestId, &callback] {
            auto subAM = weak.Upgrade();
            auto exportedRoot = root.Upgrade();
            std::list<AccessibilityElementInfo> infos;
            if (subAM && exportedRoot) {
                subAM->SearchDefaultFocusFromRoot(infos, subAM->GetPipelineContext().Upgrade(), exportedRoot);
                subAM->UpdateElementInfosTreeId(infos);
            }
            callback.SetSearchDefaultFocusByWindowIdResult(std::move(infos), requestId);
        }, TaskExecutor::TaskType::UI, "ArkUICmpSearchDefaultFocus");
}

void JsAccessibilityManager::CmpInteractionOperation::FindFocusedElementInfo(
    const int64_t elementId, const int32_t focusType, const int32_t requestId,
    Accessibility::AccessibilityElementOperatorCallback& callback)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);
    auto subAM = js_.Upgrade();
    auto exportedRoot = exportedRoot_.Upgrade();
    if (!subAM || !exportedRoot) {
        AccessibilityElementInfo info;
        info.SetValidElement(false);
        callback.SetFindFocusedElementInfoResult(info, requestId);
        return;
    }
    auto context = subAM->GetPipelineContext().Upgrade();
    if (!context || !context->GetTaskExecutor()) {
        AccessibilityElementInfo info;
        info.SetValidElement(false);
        callback.SetFindFocusedElementInfoResult(info, requestId);
        return;
    }
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), root = exportedRoot_, splitElementId, focusType, requestId, &callback] {
            auto subAM = weak.Upgrade();
            auto exportedRoot = root.Upgrade();
            if (!subAM || !exportedRoot) {
                // Binding died mid-flight: answer NOT_SUPPORT (the main tree drops such requests).
                AccessibilityElementInfo info;
                info.SetValidElement(false);
                callback.SetFindFocusedElementInfoResult(info, requestId);
                return;
            }
            // Default-constructed info is valid.
            AccessibilityElementInfo info;
            subAM->FindFocusedElementInfoFromRoot(splitElementId, focusType, info,
                subAM->GetPipelineContext().Upgrade(), exportedRoot);
            subAM->UpdateElementInfoTreeId(info);
            callback.SetFindFocusedElementInfoResult(info, requestId);
        }, TaskExecutor::TaskType::UI, "ArkUICmpFindFocusedElement");
}

void JsAccessibilityManager::CmpInteractionOperation::FocusMoveSearch(
    const int64_t elementId, const int32_t direction, const int32_t requestId,
    Accessibility::AccessibilityElementOperatorCallback& callback)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);
    auto subAM = js_.Upgrade();
    auto context = subAM ? subAM->GetPipelineContext().Upgrade() : nullptr;
    if (!context || !context->GetTaskExecutor()) {
        ReturnFocusFallback(callback, requestId);
        return;
    }
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), root = exportedRoot_, splitElementId, direction, requestId, &callback,
        parentTreeId = parentTreeId_, hostElementId = hostElementId_] {
            auto subAM = weak.Upgrade();
            auto exportedRoot = root.Upgrade();
            AccessibilityElementInfo info;
            info.SetValidElement(false);
            if (subAM && exportedRoot && subAM->IsRegister()) {
                subAM->FocusMoveSearchFromRoot(splitElementId, direction, info,
                    subAM->GetPipelineContext().Upgrade(), exportedRoot);
            }
            if (info.IsValidElement()) {
                subAM->UpdateElementInfoTreeId(info);
            } else {
                int64_t splicedHostId = hostElementId;
                AccessibilitySystemAbilityClient::SetSplicElementIdTreeId(parentTreeId, splicedHostId);
                info.SetAccessibilityId(splicedHostId);
                info.SetBelongTreeId(parentTreeId);
                info.SetValidElement(true);
            }
            callback.SetFocusMoveSearchResult(info, requestId);
        }, TaskExecutor::TaskType::UI, "ArkUICmpFocusMoveSearch");
}

void JsAccessibilityManager::CmpInteractionOperation::ExecuteAction(
    const int64_t elementId, const int32_t action, const std::map<std::string, std::string>& actionArguments,
    const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback& callback)
{
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(elementId, splitElementId, splitTreeId);
    auto subAM = js_.Upgrade();
    auto exportedRoot = exportedRoot_.Upgrade();
    if (!subAM || !exportedRoot) {
        callback.SetExecuteActionResult(false, requestId);
        return;
    }
    auto context = subAM->GetPipelineContext().Upgrade();
    if (!context || !context->GetTaskExecutor()) {
        callback.SetExecuteActionResult(false, requestId);
        return;
    }
    auto actionType = static_cast<Accessibility::ActionType>(action);
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), root = exportedRoot_, splitElementId, actionArguments, actionType, requestId,
        &callback] {
            auto subAM = weak.Upgrade();
            auto exportedRoot = root.Upgrade();
            if (!subAM || !exportedRoot) {
                callback.SetExecuteActionResult(false, requestId);
                return;
            }
            bool result = subAM->ExecuteActionFromRoot(splitElementId, actionArguments, actionType,
                subAM->GetPipelineContext().Upgrade(), exportedRoot);
            callback.SetExecuteActionResult(result, requestId);
        }, TaskExecutor::TaskType::UI, "ArkUICmpExecuteAction");
}

void JsAccessibilityManager::CmpInteractionOperation::ClearFocus()
{
    auto subAM = js_.Upgrade();
    CHECK_NULL_VOID(subAM);
    auto context = subAM->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    // Clear the accessibility focus currently held within the exported subtree only: the subAM tracks its own.
    taskExecutor->PostTask(
        [weak = GetHandler(), root = exportedRoot_] {
            auto subAM = weak.Upgrade();
            auto exportedRoot = root.Upgrade();
            CHECK_NULL_VOID(subAM);
            CHECK_NULL_VOID(exportedRoot);
            int64_t focusNodeId = subAM->currentFocusNodeId_;
            if (focusNodeId == -1) {
                return;
            }
            // Qualify: the member ClearAccessibilityFocus(AccessibilityNode) hides this namespace-scope overload inside the nested class scope.
            OHOS::Ace::Framework::ClearAccessibilityFocus(exportedRoot, focusNodeId);
            subAM->currentFocusNodeId_ = -1;
        }, TaskExecutor::TaskType::UI, "ArkUICmpClearFocus");
}

void JsAccessibilityManager::CmpInteractionOperation::OutsideTouch()
{
    // No-op for exported subtree.
}

void JsAccessibilityManager::CmpInteractionOperation::GetCursorPosition(
    const int64_t elementId, const int32_t requestId,
    Accessibility::AccessibilityElementOperatorCallback& callback)
{
    callback.SetCursorPositionResult(0, requestId);
}

void JsAccessibilityManager::CmpInteractionOperation::SetChildTreeIdAndWinId(
    const int64_t nodeId, const int32_t treeId, const int32_t childWindowId)
{
    // F-10 nested hosting: a grandchild tree (nested CMP content on a CustomNode, or Web/XComponent/UEC) registered.
    int64_t splitElementId = AccessibilityElementInfo::UNDEFINED_ACCESSIBILITY_ID;
    int32_t splitTreeId = AccessibilityElementInfo::UNDEFINED_TREE_ID;
    AccessibilitySystemAbilityClient::GetTreeIdAndElementIdBySplitElementId(nodeId, splitElementId, splitTreeId);
    TAG_LOGI(AceLogTag::ACE_ACCESSIBILITY,
        "CmpOp SetChildTreeIdAndWinId: nodeId=%{public}" PRId64 ", treeId=%{public}d, childWindowId=%{public}d",
        nodeId, treeId, childWindowId);
    auto subAM = js_.Upgrade();
    CHECK_NULL_VOID(subAM);
    auto context = subAM->GetPipelineContext().Upgrade();
    CHECK_NULL_VOID(context);
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [weak = GetHandler(), registration = registration_, splitElementId, treeId, childWindowId] {
            if (registration && registration->IsCanceled()) {
                auto instance = AccessibilitySystemAbilityClient::GetInstance();
                if (instance && treeId > 0) {
                    instance->DeregisterElementOperator(childWindowId, treeId);
                }
                return;
            }
            auto subAM = weak.Upgrade();
            CHECK_NULL_VOID(subAM);
            subAM->NotifySetChildTreeIdAndWinId(splitElementId, treeId, childWindowId);
        }, TaskExecutor::TaskType::UI, "ArkUICmpSetChildTreeIdAndWinId");
}

void JsAccessibilityManager::CmpInteractionOperation::SetBelongTreeId(const int32_t treeId)
{
    CHECK_NULL_VOID(registration_);
    registration_->SetTreeId(treeId);
    if (registration_->IsCanceled()) {
        return;
    }
    auto subAM = js_.Upgrade();
    if (!subAM) {
        registration_->Cancel();
        return;
    }
    auto context = subAM->GetPipelineContext().Upgrade();
    auto executor = context ? context->GetTaskExecutor() : nullptr;
    if (!executor) {
        registration_->Cancel();
        return;
    }
    if (!executor->PostTask(
        [bindingOwner = bindingOwner_, weak = GetHandler(), registration = registration_,
        key = std::make_pair(ownerNodeId_, hostElementId_)] {
            auto mainAM = bindingOwner.Upgrade();
            auto subAM = weak.Upgrade();
            if (!mainAM || !subAM) {
                registration->Cancel();
                return;
            }
            mainAM->MarkCmpBindingRegistered(key, subAM, registration);
        }, TaskExecutor::TaskType::UI, "ArkUICmpSubTreeIdAssigned")) {
        registration_->Cancel();
    }
}

void JsAccessibilityManager::CmpInteractionOperation::FocusMoveSearchWithCondition(
    const AccessibilityElementInfo& info, const AccessibilityFocusMoveParam param,
    const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback& callback)
{
    // The request info keeps its AAMS splicing intact: the direction search splits the id inside.
    auto subAM = js_.Upgrade();
    auto exportedRoot = exportedRoot_.Upgrade();
    if (!subAM || !exportedRoot) {
        ReturnFocusFallbackWithCondition(parentTreeId_, hostElementId_, callback, requestId);
        return;
    }
    auto context = subAM->GetPipelineContext().Upgrade();
    if (!context || !context->GetTaskExecutor()) {
        ReturnFocusFallbackWithCondition(parentTreeId_, hostElementId_, callback, requestId);
        return;
    }
    context->GetTaskExecutor()->PostTask(
        [weak = GetHandler(), root = exportedRoot_, elementInfo = info, param, requestId, &callback,
        parentTreeId = parentTreeId_, hostElementId = hostElementId_] {
            auto subAM = weak.Upgrade();
            auto exportedRoot = root.Upgrade();
            if (!subAM || !exportedRoot) {
                // Binding died mid-flight: same host fallback through the with-condition channel.
                ReturnFocusFallbackWithCondition(parentTreeId, hostElementId, callback, requestId);
                return;
            }
            subAM->FocusMoveSearchWithConditionFromRoot(elementInfo, param, requestId, callback, exportedRoot);
        }, TaskExecutor::TaskType::UI, "ArkUICmpFocusMoveSearchWithCondition");
}

void JsAccessibilityManager::CmpInteractionOperation::UpdateCustomAccessibilityProperty(
    const int64_t elementId, const AccessibilityVirtualNode& accessibilityVirtualNode,
    const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback& callback)
{
    callback.SetSearchElementInfoByAccessibilityIdResult({}, requestId);
}

void JsAccessibilityManager::CmpInteractionOperation::AddAccessibilityVirtualNode(
    const int64_t elementId, const std::vector<AccessibilityVirtualNode>& nodes,
    const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback& callback)
{
    callback.SetSearchElementInfoByAccessibilityIdResult({}, requestId);
}

void JsAccessibilityManager::CmpInteractionOperation::RemoveAccessibilityVirtualNode(
    const int64_t elementId, const int32_t requestId,
    Accessibility::AccessibilityElementOperatorCallback& callback)
{
    callback.SetSearchElementInfoByAccessibilityIdResult({}, requestId);
}

// Get the subAM owning this node (walks up to the nearest registered exported root).
WeakPtr<JsAccessibilityManager> JsAccessibilityManager::GetSubAccessibilityManager(const NG::FrameNode* node)
{
    auto subAM = CmpAccessibility::CmpAccessibilityRegistry::GetSubAM(node);
    return WeakPtr<JsAccessibilityManager>(DynamicCast<JsAccessibilityManager>(subAM));
}
} // namespace OHOS::Ace::Framework
