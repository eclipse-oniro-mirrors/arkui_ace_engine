/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_ACCESSIBILITY_JS_ACCESSIBILITY_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_ACCESSIBILITY_JS_ACCESSIBILITY_MANAGER_H

#include <optional>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <mutex>
#include <vector>

#include "accessibility_config.h"
#include "accessibility_element_operator.h"
#include "accessibility_event_info.h"
#include "accessibility_state_event.h"

#include "core/accessibility/accessibility_manager.h"
#include "core/accessibility/accessibility_utils.h"
#include "core/accessibility/hidumper/accessibility_hidumper.h"
#include "core/accessibility/utils/accessibility_manager_utils.h"
#include "frameworks/bridge/common/accessibility/accessibility_node_manager.h"
#include "adapter/ohos/osal/accessibility/focus_move/accessibility_focus_strategy.h"

#include "js_third_accessibility_hover_ng.h"

namespace OHOS::NWeb {
class NWebAccessibilityNodeInfo;
} // namespace OHOS::NWeb::NWebAccessibilityNodeInfo

namespace OHOS::Ace::NG {
    class TransitionalNodeInfo;
    struct WindowSceneInfo;
    class VirtualAccessibilityNode;
}

namespace OHOS::Accessibility {
    class AccessibilitySystemAbilityClient;
}

namespace OHOS::Ace::Framework {

struct SearchParameter {
    int64_t nodeId;
    std::string text;
    int32_t mode;
    int64_t uiExtensionOffset;
};

enum WebAccFun {
    WEB_ACC_INVALID,
    WEB_GET_ACC,
    WEB_ON_ACC,
    WEB_OFF_ACC,
    WEB_TREE,
    WEB_NODE,
    WEB_PRE,
    WEB_NEXT,
    WEB_EXE_ACTION,
    WEB_SEND_EVENT,
    WEB_HOVER,
    WEB_ACC_JSON
};

using CheckAndGetEmbedFunc = std::function<int64_t(const RefPtr<NG::FrameNode>& node)>;
struct CommonProperty {
    int32_t innerWindowId = -1;
    int32_t windowId = 0;
    int32_t windowLeft = 0;
    int32_t windowTop = 0;
    std::vector<std::string> pagePaths;
    std::vector<RefPtr<NG::FrameNode>> pageNodes;
    bool isReduceMode = false;
    bool checkEmbedNode = true;
    CheckAndGetEmbedFunc checkGetFunc = nullptr;
    RotateTransform rotateTransform;
    float_t scaleX = 1.0f;
    float_t scaleY = 1.0f;
};

struct FillEventInfoParam {
    int64_t elementId = 0;
    int64_t stackNodeId = 0;
    uint32_t windowId = 0;
};

struct AccessibilityActionParam {
    RefPtr<NG::AccessibilityProperty> accessibilityProperty;
    std::string setTextArgument = "";
    int32_t setSelectionStart = -1;
    int32_t setSelectionEnd = -1;
    bool setSelectionDir = false;
    int32_t setCursorIndex = -1;
    TextMoveUnit moveUnit = TextMoveUnit::STEP_CHARACTER;
    AccessibilityScrollType scrollType = AccessibilityScrollType::SCROLL_DEFAULT;
    int32_t spanId = -1;
    std::string customActionName;
};

struct ActionParam {
    Accessibility::ActionType action;
    std::map<std::string, std::string> actionArguments;
};

struct AccessibilityFocusInfo {
    int64_t currentFocusNodeId;
    int64_t currentFocusVirtualNodeParentId;
    bool isFocusMoveAction = false;

    explicit AccessibilityFocusInfo(int64_t nodeId = -1, int64_t parentId = -1, bool isFocusMove = false)
        : currentFocusNodeId(nodeId), currentFocusVirtualNodeParentId(parentId), isFocusMoveAction(isFocusMove) {}
};

enum class AccessibilityFocusActionType : int32_t {
    HOVER_FOCUS = 1,
    SWIPE_FOCUS = 2,
    SCROLL_FOCUS = 3,
};

struct GetInfoByNodeId {
    std::string componentType;
    int32_t pageId = -1;
    int32_t nodeInstanceId = -1;
};

enum class SearchSurfaceIdRet : int32_t {
    SEARCH_SUCCESS = 0,
    NO_MATCH_NODE,
    TIMEOUT,
};

enum class SearchSurfaceIdType : int32_t {
    SEARCH_ALL = 0,
    SEARCH_HEAD,
    SEARCH_TAIL,
    SEARCH_HEAD_LINK,
    SEARCH_TAIL_LINK,
    SEARCH_HEAD_TITLE,
    SEARCH_TAIL_TITLE,
};

class JsAccessibilityManager : public AccessibilityNodeManager,
    public AccessibilityHoverManagerForThirdNG {
    DECLARE_ACE_TYPE(JsAccessibilityManager, AccessibilityNodeManager);

private:
    struct CmpRegistration;

public:
    JsAccessibilityManager() = default;
    ~JsAccessibilityManager() override;

    static constexpr int32_t INVALID_PARENT_ID = -2100000;
    // JsAccessibilityManager overrides functions.
    void InitializeCallback() override;
    void SendAccessibilityAsyncEvent(const AccessibilityEvent& accessibilityEvent) override;
    void UpdateVirtualNodeFocus() override;
    void SetCardViewParams(const std::string& key, bool focus) override;
    void HandleComponentPostBinding() override;
    void RegisterSubWindowInteractionOperation(int windowId) override;
    void SetPipelineContext(const RefPtr<PipelineBase>& context) override;
    void UpdateElementInfosTreeId(std::list<Accessibility::AccessibilityElementInfo>& infos, bool checkEmbed = false);
    // compensateCmpRect=false: id splicing only, skip the CMP exported-subtree rect compensation.
    void UpdateElementInfoTreeId(Accessibility::AccessibilityElementInfo& info,
        bool compensateCmpRect = true);
    static bool IsTagInEmbedComponent(const std::string& tag);

    // Get the subAM owning this node (nearest registered exported root).
    static WeakPtr<JsAccessibilityManager> GetSubAccessibilityManager(const NG::FrameNode* node);

    // CMP mixed-rendering: adoption-mounted detection for an exported subtree.
    static bool IsUnderAdoptedMount(const RefPtr<NG::FrameNode>& node);

    // CMP mixed-rendering: node-aware AM routing.
    RefPtr<AccessibilityManager> GetAccessibilityManagerForNode(const RefPtr<NG::FrameNode>& node) override;

    // CMP mixed-rendering: bind/unbind exported subtree (CAPI internal implementation).
    bool BindExportTree(NG::FrameNode* owner, int64_t bindId, NG::FrameNode* exportedRoot);
    void UnbindExportTree(NG::FrameNode* owner, int64_t bindId);
    // Three-parameter convergence: CMP subtree registered (treeId obtained).
    void OnCmpTreeRegistered(NG::FrameNode* owner, int32_t treeId);
    void OnCmpTreeDeregistered(NG::FrameNode* owner, int32_t treeId);
    // Register one converged binding to AAMS (parentTreeId=CMP treeId, parentElementId=bindId): migrate four-type.
    void RegisterCmpSubtreeToAams(const std::pair<int32_t, int64_t>& key);
    // Whole-subtree detach for one binding (Unbind semantics): deregister grandchild trees, deregister the exported.
    void DetachCmpBinding(const std::pair<int32_t, int64_t>& key);
    // Binding state transition: T_sub assigned (SetBelongTreeId continuation, UI thread).
    void MarkCmpBindingRegistered(const std::pair<int32_t, int64_t>& key,
        const RefPtr<JsAccessibilityManager>& subAM, const std::shared_ptr<CmpRegistration>& registration);
    // CMP tree treeId query through the third-provider operator registered for this host.
    int32_t GetThirdProviderTreeIdForHost(NG::FrameNode* owner);
    // Migrate four-type child-tree callbacks that landed on the main AM before Bind.
    void MigrateChildTreeCallbacksToSubAM(const RefPtr<NG::FrameNode>& exportedRoot,
        const RefPtr<JsAccessibilityManager>& subAM);
    // CMP mixed-rendering: exportedRoot-parameterized search/focus.
    void SearchElementInfoByAccessibilityIdFromRoot(int64_t elementId, int32_t mode,
        std::list<AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::FrameNode>& exportedRoot);
    void FocusMoveSearchFromRoot(int64_t elementId, int32_t direction,
        Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::FrameNode>& exportedRoot);
    // Execute an accessibility action on a node within the exported subtree (root=exportedRoot).
    bool ExecuteActionFromRoot(int64_t elementId, const std::map<std::string, std::string>& actionArguments,
        Accessibility::ActionType action, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::FrameNode>& exportedRoot);
    // Condition-aware focus move search scoped to the exported subtree (result envelope per AAMS).
    void FocusMoveSearchWithConditionFromRoot(const AccessibilityElementInfo& info,
        const Accessibility::AccessibilityFocusMoveParam& param, const int32_t requestId,
        AccessibilityElementOperatorCallback& callback, const RefPtr<NG::FrameNode>& exportedRoot);
    // Detect-focusable round scoped to the exported subtree.
    void DetectElementInfoFocusableFromRoot(const AccessibilityElementInfo& info,
        const Accessibility::AccessibilityFocusMoveParam& param, const int32_t requestId,
        AccessibilityElementOperatorCallback& callback, const RefPtr<NG::FrameNode>& exportedRoot);
    // Scroll-ancestor round scoped to the exported subtree: round 1 (origin = requesting node) and the AA's.
    void ProcessGetScrollAncestorFromRoot(const AccessibilityElementInfo& info,
        const Accessibility::AccessibilityFocusMoveParam& param, const int32_t requestId,
        AccessibilityElementOperatorCallback& callback, const RefPtr<NG::FrameNode>& exportedRoot);
    // FIND_LAST round scoped to the exported subtree (backward entry): last-child descent via the shared strategy walk.
    void FindLastNodeFromRoot(const AccessibilityElementInfo& info,
        const Accessibility::AccessibilityFocusMoveParam& param, const int32_t requestId,
        AccessibilityElementOperatorCallback& callback, const RefPtr<NG::FrameNode>& exportedRoot);
    // Find the currently focused node (input/accessibility focus) within the exported subtree.
    void FindFocusedElementInfoFromRoot(int64_t elementId, int32_t focusType,
        Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::FrameNode>& exportedRoot);
    // Search nodes matching text within the exported subtree.
    void SearchElementInfosByTextFromRoot(int64_t elementId, const std::string& text,
        std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::FrameNode>& exportedRoot);
    // Search nodes matching a specific property (customId) within the exported subtree.
    void SearchElementInfoByCustomIdFromRoot(int64_t elementId, const std::string& customId,
        std::list<Accessibility::AccessibilityElementInfo>& infos,
        std::list<Accessibility::AccessibilityElementInfo>& treeInfos, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::FrameNode>& exportedRoot);
    // Collect default-focus nodes within the exported subtree.
    void SearchDefaultFocusFromRoot(std::list<Accessibility::AccessibilityElementInfo>& infos,
        const RefPtr<PipelineBase>& context, const RefPtr<NG::FrameNode>& exportedRoot);
    // Hover routing: query binding info by bindId.
    struct CmpBindingQueryResult {
        WeakPtr<NG::FrameNode> exportedRoot;
        RefPtr<JsAccessibilityManager> subAM;
    };
    std::optional<CmpBindingQueryResult> GetCmpBindingForHover(int32_t ownerNodeId, int64_t bindId);
    // Result layer of FocusMoveSearchWithConditionFromRoot: grandchild-tree drill-down / DOUBLE_CHECK handoff for a selected child-tree host.
    bool TryReplyChildTreeHandoffFromRoot(const AccessibilityElementInfo& nodeInfo,
        const Accessibility::AccessibilityFocusMoveParam& param, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::FrameNode>& exportedRoot, std::list<AccessibilityElementInfo>& infos,
        FocusMoveResult& result, const int32_t requestId,
        AccessibilityElementOperatorCallback& callback);
    // SEARCH_FAIL_IN_CHILDTREE anchor fill (boundary-exhausted searches): the exhaustion reply is anchored on the subtree root's info.
    void FillSearchFailInChildTreeRootFromRoot(const RefPtr<PipelineBase>& context,
        const RefPtr<NG::FrameNode>& exportedRoot, const AccessibilityElementInfo& nodeInfo,
        std::list<AccessibilityElementInfo>& infos);
    // Hover-variant ancestor substitution for DetectElementInfoFocusableFromRoot: nearest readable ancestor within.
    std::optional<AccessibilityElementInfo> FindReadableAncestorFromRoot(
        const RefPtr<NG::FrameNode>& baseNode, const RefPtr<NG::FrameNode>& exportedRoot,
        const Accessibility::AccessibilityFocusMoveParam& param, const RefPtr<PipelineBase>& context);
    // Result completion of FindLastNodeFromRoot.
    bool CompleteFindLastResultFromRoot(const AccessibilityElementInfo& targetInfo,
        const Accessibility::AccessibilityFocusMoveParam& param, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::FrameNode>& exportedRoot, AceFocusMoveResult moveResult,
        std::list<AccessibilityElementInfo>& infos, FocusMoveResult& result);
    // Fill the scroll-ancestor reply infos from the strategy walk's collected check nodes.
    void FillScrollAncestorInfosFromRoot(const RefPtr<NG::PipelineContext>& ngPipeline,
        const std::list<std::shared_ptr<FocusRulesCheckNode>>& targetNodes,
        std::list<AccessibilityElementInfo>& infos);
    // Dissolved-embed-content origin search: the same-layer dissolution cuts the embed anchor and its content from.
    enum class DissolvedEmbedSearch { NOT_APPLICABLE, FOUND };
    DissolvedEmbedSearch SearchInDissolvedEmbedContent(int64_t splitElementId, int32_t direction,
        bool checkSelf, int32_t focusRuleType, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::FrameNode>& exportedRoot, AccessibilityElementInfo& outInfo);
    // Binding-map accessors shared by the CMP registration flow.
    bool SnapshotRegisteringCmpBinding(const std::pair<int32_t, int64_t>& key,
        RefPtr<JsAccessibilityManager>& subAM, WeakPtr<NG::FrameNode>& exportedRootWeak,
        WeakPtr<NG::FrameNode>& ownerWeak, int32_t& cmpTreeId);
    bool CompleteCmpRegistration(const std::pair<int32_t, int64_t>& key,
        const RefPtr<JsAccessibilityManager>& subAM, const std::shared_ptr<CmpRegistration>& registration, bool success);
    void WireSubAmAnchors(const RefPtr<JsAccessibilityManager>& subAM,
        const WeakPtr<NG::FrameNode>& exportedRootWeak, NG::FrameNode* owner,
        int32_t cmpTreeId, int64_t elementId, uint32_t windowId);
    // Diagnostic: ids of all REGISTERED bindId mounts (hover hit-test probe).
    std::vector<int64_t> GetCmpBindingBindIdsForHover(int32_t ownerNodeId);

    // The exported root's window position (used to complete subtree-local rects).
    bool GetExportedRootWindowOffset(float& offsetX, float& offsetY);

    // CMP mixed-rendering: true only for subAMs created by BindExportTree.
    void MarkCmpSubAM() { isCmpSubAM_ = true; }
    bool IsCmpSubAM() const { return isCmpSubAM_; }

    void UpdateViewScale();

    float GetScaleX() const
    {
        return scaleX_;
    }

    float GetScaleY() const
    {
        return scaleY_;
    }

    uint32_t GetWindowId() const
    {
        return windowId_;
    }

    void SetWindowId(uint32_t windowId)
    {
        windowId_ = windowId;
    }

    void SaveLast(const int64_t elementId, const RefPtr<NG::FrameNode>& node)
    {
        lastElementId_ = elementId;
        lastFrameNode_ = node;
    }

    void SetReentrantLimit(const bool reentrantLimit)
    {
        reentrantLimit_ = reentrantLimit;
    }

    bool IsReentrantLimit() const
    {
        return reentrantLimit_;
    }

    void SaveCurrentFocusNodeSize(const RefPtr<NG::FrameNode>& currentFocusNode)
    {
        if (currentFocusNode->IsAccessibilityVirtualNode()) {
            auto oldGeometryNode = currentFocusNode->GetGeometryNode();
            CHECK_NULL_VOID(oldGeometryNode);
            oldGeometrySize_ = oldGeometryNode->GetFrameSize();
        }
    }

    bool SubscribeToastObserver();
    bool UnsubscribeToastObserver();
    bool SubscribeStateObserver(uint32_t eventType);
    bool UnsubscribeStateObserver(uint32_t eventType);
    int RegisterInteractionOperation(int windowId);
    void DeregisterInteractionOperation();

    bool IsSendAccessibilityEvent(const AccessibilityEvent& accessibilityEvent);
    bool IsSendAccessibilityEventForUEA(
        const AccessibilityEvent& accessibilityEvent, const std::string& componentType, const int32_t pageId);
    bool IsSendAccessibilityEventForHost(
        const AccessibilityEvent& accessibilityEvent, const std::string& componentType, const int32_t pageId);
    void GetComponentTypeAndPageIdByNodeId(
        const int64_t nodeId,
        const RefPtr<PipelineBase>& context,
        GetInfoByNodeId& infoOfNode);
    bool CachePageEventByController(
        const AccessibilityEvent& accessibilityEvent,
        const std::string& componentType,
        int32_t pageId,
        int32_t containerId);

    void SendCacheAccessibilityEvent(int32_t instanceId);
    void SendCacheAccessibilityEventForHost(const int32_t pageId);

    void AddFrameNodeToUecStatusVec(const RefPtr<NG::FrameNode>& node) override;
    void AddFrameNodeToDefaultFocusList(const RefPtr<NG::FrameNode>& node, bool isFocus) override;
    void AddDefaultFocusNode(const RefPtr<NG::FrameNode>& defaultFocusNode);
    void EraseDefaultFocusNode(const RefPtr<NG::FrameNode>& defaultFocusNode);

    std::list<WeakPtr<NG::FrameNode>>& GetDefaultFocusList()
    {
        return defaultFocusList_;
    }

    void RegisterUIExtGetPageModeCallback(RefPtr<NG::UIExtensionManager>& uiExtManager) override;
    void UpdateFrameNodeState(int32_t nodeId) override;

    void ReleaseCacheEvent();
    void ReleaseUIExtCacheEvent();
    void UpdatePageMode(const std::string& pageMode) override
    {
        pageMode_ = std::make_optional(pageMode);
    }

    void CompensateCmpEventRect(AccessibilityEventInfo& eventInfo);
    bool SendAccessibilitySyncEvent(
        const AccessibilityEvent& accessibilityEvent, Accessibility::AccessibilityEventInfo eventInfo);
    bool TransferAccessibilityAsyncEvent(
        const Accessibility::AccessibilityEventInfo& eventInfo,
        int64_t uiExtensionOffset) override;
    void SendExtensionAccessibilityEvent(
        const Accessibility::AccessibilityEventInfo& eventInfo,
        int64_t uiExtensionOffset) override;
    void SearchElementInfoByAccessibilityId(const int64_t elementId, const int32_t requestId,
        Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t mode, const int32_t windowId);
    void SearchElementInfoBySpecificProperty(const int64_t elementId, const SpecificPropertyParam &param,
        const int32_t requestId, AccessibilityElementOperatorCallback &callback, const int32_t windowId);
    void SearchElementInfoBySpecificPropertyInner(const int64_t elementId, const SpecificPropertyParam &param,
        const int32_t requestId, AccessibilityElementOperatorCallback &callback, const int32_t windowId);
    void SearchElementInfosByText(const int64_t elementId, const std::string& text, const int32_t requestId,
        Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t windowId);
    void SearchDefaultFocusByWindowId(const int32_t windowId, int32_t pageId, const int32_t requestId,
        Accessibility::AccessibilityElementOperatorCallback& callback);
    void FindFocusedElementInfo(const int64_t elementId, const int32_t focusType, const int32_t requestId,
        Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t windowId);
    bool ShouldSkipAccessibilityStateChange(const RefPtr<PipelineBase>& pipelineRef);
    void FocusMoveSearch(const int64_t elementId, const int32_t direction, const int32_t requestId,
        Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t windowId);
    bool IsUpdateWindowSceneInfo(const RefPtr<NG::FrameNode>& node, NG::WindowSceneInfo& windowSceneInfo);
    void UpdateElementInfoInnerWindowId(Accessibility::AccessibilityElementInfo& info, const int64_t& elementId);
    void ExecuteAction(const int64_t accessibilityId, const ActionParam& param, const int32_t requestId,
        Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t windowId);
    bool ClearCurrentFocus();
    void UpdateNodeChildIds(const RefPtr<AccessibilityNode>& node);
    void SendActionEvent(const Accessibility::ActionType& action, int64_t nodeId);
    void SearchElementInfoByAccessibilityIdNG(int64_t elementId, int32_t mode,
        std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context,
        const int64_t uiExtensionOffset = 0) override;
    bool FindUIExtensionAccessibilityElement(const RefPtr<NG::FrameNode>& checkNode, const std::string &customId,
        const CommonProperty& commonProperty, std::list<AccessibilityElementInfo> &infos,
        const RefPtr<PipelineBase>& context);
    bool SetAccessibilityCustomId(RefPtr<NG::FrameNode> checkNode, const std::string &customId,
        CommonProperty &commonProperty, std::list<AccessibilityElementInfo> &infos,
        const RefPtr<PipelineBase>& context);
    void SearchElementInfoByCustomIdNG(const int64_t elementId, const std::string &customId,
        std::list<AccessibilityElementInfo> &infos, std::list<AccessibilityElementInfo> &treeInfos,
        const RefPtr<PipelineBase>& context);
    void SearchDefaultFocusByWindowIdNG(const int32_t pageId,
        std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context);
    void SearchElementInfosByTextNG(int64_t elementId, const std::string& text,
        std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context,
        const int64_t uiExtensionOffset = 0) override;
    void FindFocusedElementInfoNG(int64_t elementId, int32_t focusType, Accessibility::AccessibilityElementInfo& info,
        const RefPtr<PipelineBase>& context, const int64_t uiExtensionOffset = 0) override;
    void FocusMoveSearchNG(int64_t elementId, int32_t direction, Accessibility::AccessibilityElementInfo& info,
        const RefPtr<PipelineBase>& context, const int64_t uiExtensionOffset = 0) override;
    bool ExecuteExtensionActionNG(int64_t elementId, const std::map<std::string, std::string>& actionArguments,
        int32_t action, const RefPtr<PipelineBase>& context, int64_t uiExtensionOffset) override;
    int32_t GetTransformDegreeRelativeToWindow(const RefPtr<NG::FrameNode>& node, bool excludeSelf = false) override;
    AccessibilityParentRectInfo GetTransformRectInfoRelativeToWindow(
        const RefPtr<NG::FrameNode>& node, const RefPtr<PipelineBase>& context) override;
#ifdef WEB_SUPPORTED
    void SendWebAccessibilityAsyncEvent(const AccessibilityEvent& accessibilityEvent,
        const RefPtr<NG::WebPattern>& webPattern) override;
    void SearchWebElementInfoByAccessibilityId(const int64_t elementId, const int32_t requestId,
        Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t mode, const int32_t windowId,
        const RefPtr<NG::WebPattern>& webPattern);
    void SearchWebElementInfoByAccessibilityIdNG(int64_t elementId, int32_t mode,
        std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::WebPattern>& webPattern);
    void FindWebFocusedElementInfo(const int64_t elementId, const int32_t focusType,
        const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t windowId,
        const RefPtr<NG::WebPattern>& webPattern);
    void FindWebFocusedElementInfoNG(int64_t elementId, int32_t focusType,
        Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::WebPattern>& webPattern);
    void WebFocusMoveSearch(const int64_t elementId, const int32_t direction, const int32_t requestId,
        Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t windowId,
        const RefPtr<NG::WebPattern>& webPattern);
    void WebFocusMoveSearchByComponent(AccessibilityElementInfo& nodeInfo, const RefPtr<NG::WebPattern>& webPattern,
        const int32_t direction, RefPtr<PipelineBase> context, FocusMoveResult& result,
        int32_t focusRuleType = static_cast<int32_t>(FocusRuleType::DEFAULT), uint32_t searchDepth = 0);
    // Returns true when the Web side answered with a node (focus target found).
    bool WebFocusMoveSearchNG(int64_t elementId, int32_t direction,
        Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::WebPattern>& webPattern);
    void WebFocusMoveSearchWithCondition(const int64_t elementId,
        const Accessibility::AccessibilityFocusMoveParam& param, const int32_t requestId,
        Accessibility::AccessibilityElementOperatorCallback& callback, const RefPtr<NG::WebPattern>& webPattern);
    FocusMoveResult WebFocusMoveSearchWithConditionNG(int64_t elementId, int32_t direction, int32_t focusRuleType,
        Accessibility::AccessibilityElementInfo& info,
        const RefPtr<PipelineBase>& context, const RefPtr<NG::WebPattern>& webPattern);
    // In-process Web grandchild drill-down at the child-tree handoff sites.
    bool TryFocusMoveSearchWebGrandchildFromRoot(const AccessibilityElementInfo& hostInfo,
        const Accessibility::AccessibilityFocusMoveParam& param, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::FrameNode>& exportedRoot, std::list<AccessibilityElementInfo>& infos,
        FocusMoveResult& result);
    // DOM re-search fallback of WebFocusMoveSearchByComponent's embed handoff (no surfaceId / surface search miss).
    void WebEmbedFallbackResearch(AccessibilityElementInfo& nodeInfo,
        const RefPtr<NG::WebPattern>& webPattern, const int32_t direction, RefPtr<PipelineBase> context,
        FocusMoveResult& result, int32_t focusRuleType, uint32_t searchDepth);
    // Plain-API grandchild exhaustion escalation (CMP exported subtree only): the AA drives Web-tree focus moves.
    bool TryPlainWebExhaustionEscalation(int32_t direction, int32_t requestId,
        const RefPtr<NG::FrameNode>& hostNode, const RefPtr<PipelineBase>& context,
        AccessibilityElementInfo& nodeInfo);
    void ExecuteWebAction(const int64_t elementId, const ActionParam& param, const int32_t requestId,
        Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t windowId,
        const RefPtr<NG::WebPattern>& webPattern);
    bool ExecuteWebActionNG(int64_t elementId, Accessibility::ActionType action,
        const std::map<std::string, std::string>& actionArguments, const RefPtr<NG::WebPattern>& webPattern);

    bool DeregisterWebInteractionOperationAsChildTree(int32_t treeID,
        const WeakPtr<NG::WebPattern>& webPattern) override;
    bool RegisterWebInteractionOperationAsChildTree(int64_t accessibilityId,
        const WeakPtr<NG::WebPattern>& webPattern) override;
    void GetWebCursorPosition(const int64_t elementId, const int32_t requestId,
        AccessibilityElementOperatorCallback& callback, const RefPtr<NG::WebPattern>& webPattern);
    int64_t ConvertToSplitElementId(int64_t elementId);
    bool GetWebDumpQueryType(const std::string& webAccFun, int32_t& mode, int32_t& direction);
    bool DumpElementInfosIfNeed(const DumpInfoArgument& argument, std::list<AccessibilityElementInfo>& infos,
        const RefPtr<NG::WebPattern>& webPattern, uint32_t windowId);
    void ExecuteWebDump(const DumpInfoArgument& argument, std::list<AccessibilityElementInfo>& infos,
        const RefPtr<NG::WebPattern>& webPattern);
    void ChooseWebDumpEvent(DumpInfoArgument& argument, uint32_t windowId);
    bool DumpWebInfoParams(const std::vector<std::string>& params, DumpInfoArgument& argument);
    void ExecuteWebAccStateDump(WebAccFun fun, const RefPtr<NG::WebPattern>& webPattern);
    void ExecuteWebInfoDump(WebAccFun fun, std::list<AccessibilityElementInfo>& infos,
        const RefPtr<NG::WebPattern>& webPattern);
    void ExecuteWebActionDump(WebAccFun fun, const DumpInfoArgument& argument, int64_t elementId,
        const RefPtr<NG::WebPattern>& webPattern);
#endif //WEB_SUPPORTED
    void GetResultOfFocusMoveSearchNG(
        int64_t elementId, int32_t direction, Accessibility::AccessibilityElementInfo& info);

    std::string GetPagePath();

    void RegisterAccessibilityChildTreeCallback(
        int64_t elementId, const std::shared_ptr<AccessibilityChildTreeCallback> &callback) override;

    void DeregisterAccessibilityChildTreeCallback(int64_t elementId) override;

    void RegisterAccessibilitySAObserverCallback(
        int64_t elementId, const std::shared_ptr<AccessibilitySAObserverCallback> &callback) override;

    void DeregisterAccessibilitySAObserverCallback(int64_t elementId) override;

    void RegisterInteractionOperationAsChildTree(uint32_t parentWindowId, int32_t parentTreeId,
        int64_t parentElementId) override;
    void SetAccessibilityGetParentRectHandler(std::function<void(int32_t &, int32_t &)> &&callback) override;
    void SetAccessibilityGetParentRectHandler(
        std::function<void(AccessibilityParentRectInfo &)> &&callback) override;
    void DeregisterInteractionOperationAsChildTree() override;
    void SendEventToAccessibilityWithNode(const AccessibilityEvent& accessibilityEvent,
        const RefPtr<AceType>& node, const RefPtr<PipelineBase>& context) override;

    bool RegisterInteractionOperationAsChildTree(const Registration& registration) override;
    bool DeregisterInteractionOperationAsChildTree(uint32_t windowId, int32_t treeId) override;

    void TransferThirdProviderHoverEvent(
        const WeakPtr<NG::FrameNode>& hostNode, const NG::PointF& point, SourceType source,
        NG::AccessibilityHoverEventType eventType, TimeStamp time) override;

    void DumpAccessibilityPropertyNG(const AccessibilityElementInfo& nodeInfo);
    void DumpCommonPropertyNG(const AccessibilityElementInfo& nodeInfo, int32_t treeId);
    bool OnDumpChildInfoForThird(
        int64_t hostElementId,
        const std::vector<std::string>& params,
        std::vector<std::string>& info) override;
    void ProcessParameters(
        ActionType op, const std::vector<std::string>& params, std::map<std::string, std::string>& paramsMap);
    bool CheckDumpHandleEventParams(const std::vector<std::string> &params);
    bool CheckGetActionIdAndOp(
        const std::vector<std::string>& params,
        int64_t& actionAccessibilityId,
        ActionType& actionOp);
    bool DumpProcessEventParameters(
        AccessibilityEvent& event, const std::vector<std::string>& params);
    bool GetDumpInfoArgument(const std::vector<std::string>& params, DumpInfoArgument& argument);
    bool DumpInfoParams(const std::vector<std::string>& params, DumpInfoArgument& argument);
    bool HandleNodeModeParam(const std::string& param, DumpInfoArgument& argument);
    void FireAccessibilityEventCallback(uint32_t eventId, int64_t parameter) override;
    AccessibilityWindowInfo GenerateWindowInfo(const RefPtr<NG::FrameNode>& node,
        const RefPtr<PipelineBase>& context) override;
    AccessibilityWorkMode GenerateAccessibilityWorkMode() override;
    void UpdateAccessibilityNextFocusIdMap(int32_t containerId,
                                           const std::string& nextFocusInspectorKey,
                                           int64_t preAccessibilityId,
                                           bool descendantMode = false) override;

    bool GetNextFocusDescendantMode(int32_t containerId, const std::string& inspectorId);

    void UpdateWindowInfo(AccessibilityWindowInfo& window, const RefPtr<PipelineBase>& context) override;
    AccessibilityParentRectInfo GetUECAccessibilityParentRectInfo() const;
    void UpdateUECAccessibilityParentRectInfo(const AccessibilityParentRectInfo& info);
    void RegisterUIExtBusinessConsumeCallback();
    void RegisterGetParentRectHandler();

    bool IsScreenReaderEnabled() override;
    bool IsSelectReaderEnabled() override;
    WeakPtr<NG::WebPattern> GetWebPatternBySurfaceId(const std::string& surfaceId) override;
    // Own-map lookup with a main-AM fallback for subAMs.
    WeakPtr<NG::WebPattern> GetWebPatternBySurfaceIdWithMainFallback(const std::string& surfaceId);
    void SetWebPatternBySurfaceId(const std::string& surfaceId, WeakPtr<NG::WebPattern> pattern) override;
    void RemoveWebPatternBySurfaceId(const std::string& surfaceId) override;

    void SearchAccessibilityNodeBySpecificProperty(const int64_t elementId,
        const SpecificPropertyParam &param, const int32_t requestId,
        AccessibilityElementOperatorCallback &callback, const int32_t windowId);
    void UpdateAccessibilityNodeRect(const RefPtr<NG::FrameNode>& frameNode) override;
    void OnAccessbibilityDetachFromMainTree(const RefPtr<NG::FrameNode>& frameNode) override;
    void NotifyAccessibilityFocusDetachOrInvisible(const RefPtr<NG::FrameNode>& focusNode);
    bool SendRequestFocusToCandidate(const RefPtr<NG::FrameNode>& candidateNode);
    RefPtr<NG::FrameNode> FindCandidateByFocusMove(const RefPtr<NG::FrameNode>& focusNode, int32_t direction);
    RefPtr<NG::FrameNode> FindDetachFocusCandidateFromRoot(
        const RefPtr<NG::FrameNode>& focusNode, int32_t direction);
    bool IsDetachFocusCacheClearEvent(AccessibilityEventType eventType) const;
    void ClearDetachFocusFallbackCache(int32_t instanceId);
    void CacheDetachFocusFallbackCandidates(
        int32_t instanceId, int64_t focusNodeId, const std::vector<int64_t>& candidateIds);
    struct DetachFocusFallbackData {
        int64_t focusNodeId = -1;
        std::vector<int64_t> candidateIds;
        int32_t treeId = -1;
    };
    DetachFocusFallbackData GetAndClearDetachFocusFallbackCandidates(int32_t instanceId);
    std::vector<int64_t> QueryDetachFocusFallbackCandidates(
        const RefPtr<NG::FrameNode>& focusNode, const RefPtr<NG::PipelineContext>& context, size_t maxCount);
    void HandleDetachFocusFallbackCallback(int32_t instanceId);
    AccessibilityEventInfo BuildDetachFocusFallbackEvent(const DetachFocusFallbackData& data, int32_t windowId);
    void SetFocusMoveResultWithNode(
        const WeakPtr<NG::FrameNode>& hostNode,
        AccessibilityElementOperatorCallback& callback,
        const int32_t requestId);

    void ReleasePageEvent(
        const RefPtr<NG::FrameNode>& node,
        bool deleteController = true,
        bool releaseAll = false) override;

    void AddToPageEventController(const RefPtr<NG::FrameNode>& node) override;
    bool DeleteFromPageEventController(const RefPtr<NG::FrameNode>& node) override;
    bool CheckPageEventCached(const RefPtr<NG::FrameNode>& node, bool onlyCurrentPage) override;

    bool CheckAccessibilityVisible(const RefPtr<NG::FrameNode>& node) override;

    void AddHoverTransparentCallback(const RefPtr<NG::FrameNode>& node) override;
    bool CheckHoverTransparentCallbackListEmpty(int32_t containerId) override;

    int64_t CheckAndGetEmbedFrameNode(const RefPtr<NG::FrameNode>& node) override;

    void AccessibilityOnShowHide(bool isOnShow, const WeakPtr<PipelineBase>& context) override;

    void ChooseDumpEvent(const std::vector<std::string>& params,
        DumpInfoArgument& argument, uint32_t windowId, bool hasJson);

    void SetIsIgnoreAllAction(bool isIgnoreAllAction)
    {
        isIgnoreAllAction_ = isIgnoreAllAction;
    }
    bool GetIsIgnoreAllAction() const
    {
        return isIgnoreAllAction_;
    }

    void UpdateAccessibilityNodeInfo(
        const RefPtr<AccessibilityNode>& node,
        AccessibilityElementInfo& nodeInfo,
        const RefPtr<JsAccessibilityManager>& manager,
        int windowId);

    RefPtr<NG::FrameNode> GetNextFocusNodeByManager(
        const RefPtr<NG::UINode>& currentNode,
        const RefPtr<NG::FrameNode>& rootNode);
    RefPtr<NG::FrameNode> GetPrevFocusNodeByManager(
        const RefPtr<NG::UINode>& currentNode,
        const RefPtr<NG::FrameNode>& rootNode,
        const RefPtr<PipelineBase>& context);

    bool CheckAndGetEmbedWebElementInfo(
        const RefPtr<NG::FrameNode>& node,
        AccessibilityElementInfo& elementInfo);

    void GenerateCommonProperty(const RefPtr<PipelineBase>& context, CommonProperty& output,
        const RefPtr<PipelineBase>& mainContext, const RefPtr<NG::FrameNode>& node = nullptr);

    void UpdateAccessibilityElementInfo(
        const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
        Accessibility::AccessibilityElementInfo& nodeInfo, const RefPtr<NG::PipelineContext>& ngPipeline);

    void RegisterScreenReaderObserverCallback(
        int64_t elementId, const std::shared_ptr<AccessibilityScreenReaderObserverCallback>& callback) override;
    void DeregisterScreenReaderObserverCallback(int64_t elementId) override;

    void DetectElementInfoFocusableThroughAncestor(
        const Accessibility::AccessibilityElementInfo& info,
        const Accessibility::AccessibilityFocusMoveParam param, const int32_t requestId,
        Accessibility::AccessibilityElementOperatorCallback &callback, const int32_t windowId);

    void FocusMoveSearchWithConditionForNode(
        const AccessibilityElementInfo& info, const Accessibility::AccessibilityFocusMoveParam param,
        const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t windowId);

    void FocusMoveSearchWithCondition(
        const AccessibilityElementInfo& info, const Accessibility::AccessibilityFocusMoveParam param,
        const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t windowId);

    bool NeedChangeToReadableNode(const RefPtr<NG::FrameNode>& curFrameNode,
        RefPtr<NG::FrameNode>& readableNode) override;

    void ResetBlockedEvent();
    int32_t GetTreeId(int32_t instanceId = -1) override;
    void GetCursorPosition(const int64_t elementId, const int32_t requestId,
        Accessibility::AccessibilityElementOperatorCallback& callback);

    void UpdateAccessibilityElementInfoForVirtualNode(
        const RefPtr<NG::FrameNode>& hostNode,
        const RefPtr<NG::VirtualAccessibilityNode>& virtualNode,
        const CommonProperty& commonProperty,
        AccessibilityElementInfo& nodeInfo,
        int64_t parentAccessibilityId);

    void AddAccessibilityVirtualNode(const int64_t elementId,
        const std::vector<AccessibilityVirtualNode>& nodes, const int32_t requestId,
        AccessibilityElementOperatorCallback& callback, const int32_t windowId);
    void RemoveAccessibilityVirtualNode(const int64_t elementId, const int32_t requestId,
        AccessibilityElementOperatorCallback& callback, const int32_t windowId);

    void AddVirtualNodeChildrenToInfos(
        std::list<AccessibilityElementInfo>& infos,
        const RefPtr<NG::FrameNode>& hostNode,
        const RefPtr<NG::VirtualAccessibilityNode>& virtualNode,
        const CommonProperty& commonProperty,
        int64_t virtualNodeAccessibilityId);
    void FillEventInfoWithVirtualNode(
        const RefPtr<NG::FrameNode>& parentNode,
        const RefPtr<NG::VirtualAccessibilityNode>& virtualNode,
        AccessibilityEventInfo& eventInfo,
        const RefPtr<NG::PipelineContext>& context,
        int64_t elementId);
    void SendEventToAccessibilityWithVirtualNode(
        const AccessibilityEvent& accessibilityEvent,
        const RefPtr<NG::FrameNode>& parentNode,
        const RefPtr<NG::VirtualAccessibilityNode>& virtualNode,
        const RefPtr<PipelineBase>& context);

protected:
    void OnDumpInfoNG(const std::vector<std::string>& params, uint32_t windowId, bool hasJson = false) override;
    void DumpHandleEvent(const std::vector<std::string>& params) override;
    void DumpProperty(const std::vector<std::string>& params) override;
    void DumpTree(int32_t depth, int64_t nodeID, bool isDumpSimplify = false) override;

private:
    mutable std::mutex webPatternMapMutex_;
    std::unordered_map<std::string, WeakPtr<NG::WebPattern>> webPatternMap_;

    class JsInteractionOperation : public Accessibility::AccessibilityElementOperator {
    public:
        explicit JsInteractionOperation(int32_t windowId) : windowId_(windowId) {}
        virtual ~JsInteractionOperation() = default;
        // Accessibility override.
        RetError SearchElementInfoByAccessibilityId(const int64_t elementId, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t mode) override;
        void HandleSearchElementInfoCallback(AccessibilityElementOperatorCallback& callback, int32_t requestId);
        void SearchElementInfoBySpecificProperty(const int64_t elementId, const SpecificPropertyParam &param,
            const int32_t requestId, AccessibilityElementOperatorCallback &callback) override;
        void SearchElementInfosByText(const int64_t elementId, const std::string& text, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void SearchDefaultFocusByWindowId(const int32_t windowId, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback &callback, const int32_t pageId) override;
        void FindFocusedElementInfo(const int64_t elementId, const int32_t focusType, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void FocusMoveSearch(const int64_t elementId, const int32_t direction, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void ExecuteAction(const int64_t elementId, const int32_t action,
            const std::map<std::string, std::string>& actionArguments, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void ClearFocus() override;
        void OutsideTouch() override;
        void GetCursorPosition(const int64_t elementId, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback &callback) override;
        void SetChildTreeIdAndWinId(const int64_t nodeId, const int32_t treeId, const int32_t childWindowId) override;
        void SetBelongTreeId(const int32_t treeId) override;
        void FocusMoveSearchWithCondition(const AccessibilityElementInfo& info, const AccessibilityFocusMoveParam param,
            const int32_t requestId, AccessibilityElementOperatorCallback &callback) override;

        void UpdateCustomAccessibilityProperty(
            const int64_t elementId, const AccessibilityVirtualNode& accessibilityVirtualNode,
            const int32_t requestId, AccessibilityElementOperatorCallback& callback) override;
        void AddAccessibilityVirtualNode(
            const int64_t elementId, const std::vector<AccessibilityVirtualNode>& nodes,
            const int32_t requestId, AccessibilityElementOperatorCallback& callback) override;
        void RemoveAccessibilityVirtualNode(const int64_t elementId,
            const int32_t requestId, AccessibilityElementOperatorCallback& callback) override;

        void SetHandler(const WeakPtr<JsAccessibilityManager>& js)
        {
            js_ = js;
        }

        const WeakPtr<JsAccessibilityManager>& GetHandler() const
        {
            return js_;
        }

    private:
        WeakPtr<JsAccessibilityManager> js_;
        uint32_t windowId_ = 0;
    };
#ifdef WEB_SUPPORTED

    class WebInteractionOperation : public Accessibility::AccessibilityElementOperator {
    public:
        explicit WebInteractionOperation(int32_t windowId) : windowId_(windowId) {}
        virtual ~WebInteractionOperation() = default;
        // Accessibility override.
        RetError SearchElementInfoByAccessibilityId(const int64_t elementId, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t mode) override;
        void SearchElementInfoBySpecificProperty(const int64_t elementId, const SpecificPropertyParam &param,
            const int32_t requestId, AccessibilityElementOperatorCallback &callback) override;
        void SearchElementInfosByText(const int64_t elementId, const std::string& text, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void SearchDefaultFocusByWindowId(const int32_t windowId, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback &callback, const int32_t pageId) override;
        void FindFocusedElementInfo(const int64_t elementId, const int32_t focusType, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void FocusMoveSearch(const int64_t elementId, const int32_t direction, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void ExecuteAction(const int64_t elementId, const int32_t action,
            const std::map<std::string, std::string>& actionArguments, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void ClearFocus() override;
        void OutsideTouch() override;
        void GetCursorPosition(const int64_t elementId, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback &callback) override;
        void SetChildTreeIdAndWinId(const int64_t nodeId, const int32_t treeId, const int32_t childWindowId) override;
        void SetBelongTreeId(const int32_t treeId) override;
        void FocusMoveSearchWithCondition(const AccessibilityElementInfo& info, const AccessibilityFocusMoveParam param,
            const int32_t requestId, AccessibilityElementOperatorCallback &callback) override;

        void UpdateCustomAccessibilityProperty(
            const int64_t elementId, const AccessibilityVirtualNode& accessibilityVirtualNode,
            const int32_t requestId, AccessibilityElementOperatorCallback& callback) override;
        void AddAccessibilityVirtualNode(
            const int64_t elementId, const std::vector<AccessibilityVirtualNode>& nodes,
            const int32_t requestId, AccessibilityElementOperatorCallback& callback) override;
        void RemoveAccessibilityVirtualNode(const int64_t elementId,
            const int32_t requestId, AccessibilityElementOperatorCallback& callback) override;

        void SetHandler(const WeakPtr<JsAccessibilityManager>& js)
        {
            js_ = js;
        }

        const WeakPtr<JsAccessibilityManager>& GetHandler() const
        {
            return js_;
        }

        void SetWebPattern(const WeakPtr<NG::WebPattern>& webPattern)
        {
            webPattern_ = webPattern;
        }

        const WeakPtr<NG::WebPattern>& GetWebPattern() const
        {
            return webPattern_;
        }

    private:
        WeakPtr<JsAccessibilityManager> js_;
        uint32_t windowId_ = 0;
        WeakPtr<NG::WebPattern> webPattern_;
    };
#endif // WEB_SUPPORTED

    class ToastAccessibilityConfigObserver : public AccessibilityConfig::AccessibilityConfigObserver {
    public:
        ToastAccessibilityConfigObserver() = default;
        void OnConfigChanged(
            const AccessibilityConfig::CONFIG_ID id, const AccessibilityConfig::ConfigValue& value) override;
    };

    class JsAccessibilityStateObserver : public Accessibility::AccessibilityStateObserver {
    public:
        void OnStateChanged(const bool state) override;
        void SetAccessibilityManager(const WeakPtr<JsAccessibilityManager>& accessibilityManager)
        {
            accessibilityManager_ = accessibilityManager;
        }

        void SetPipeline(const WeakPtr<PipelineBase>& pipeline)
        {
            pipeline_ = pipeline;
        }

        void SetEventType(uint32_t eventType)
        {
            eventType_ = eventType;
        }

    private:
        // Do not upgrade accessibilityManager_ on async thread, destructor might cause freeze
        WeakPtr<JsAccessibilityManager> accessibilityManager_;
        WeakPtr<PipelineBase> pipeline_;
        uint32_t eventType_;
    };

    bool AccessibilityActionEvent(const Accessibility::ActionType& action,
        const std::map<std::string, std::string>& actionArguments, const RefPtr<AccessibilityNode>& node,
        const RefPtr<PipelineContext>& context);
    bool RequestAccessibilityFocus(const RefPtr<AccessibilityNode>& node);

    bool ClearAccessibilityFocus(const RefPtr<AccessibilityNode>& node);

    void AddFocusableNode(std::list<RefPtr<AccessibilityNode>>& nodeList, const RefPtr<AccessibilityNode>& node);
    bool CanAccessibilityFocused(const RefPtr<AccessibilityNode>& node);
    RefPtr<AccessibilityNode> FindNodeInRelativeDirection(
        const std::list<RefPtr<AccessibilityNode>>& nodeList, RefPtr<AccessibilityNode>& node, const int direction);
    RefPtr<AccessibilityNode> FindNodeInAbsoluteDirection(
        const std::list<RefPtr<AccessibilityNode>>& nodeList, RefPtr<AccessibilityNode>& node, const int direction);
    RefPtr<AccessibilityNode> GetNextFocusableNode(
        const std::list<RefPtr<AccessibilityNode>>& nodeList, RefPtr<AccessibilityNode>& node);
    RefPtr<AccessibilityNode> GetPreviousFocusableNode(
        const std::list<RefPtr<AccessibilityNode>>& nodeList, RefPtr<AccessibilityNode>& node);

    bool ExecuteActionNG(int64_t elementId, const std::map<std::string, std::string>& actionArguments,
        Accessibility::ActionType action, const RefPtr<PipelineBase>& context, int64_t uiExtensionOffset);
    bool ConvertActionTypeToBoolen(
        Accessibility::ActionType action,
        RefPtr<NG::FrameNode>& frameNode,
        int64_t elementId,
        RefPtr<NG::PipelineContext>& context,
        const std::map<std::string, std::string>& actionArguments);

    void SetSearchElementInfoByAccessibilityIdResult(Accessibility::AccessibilityElementOperatorCallback& callback,
        std::list<Accessibility::AccessibilityElementInfo>&& infos, const int32_t requestId, bool checkEmbed = false);

    void SetSearchElementInfoByCustomIdResult(Accessibility::AccessibilityElementOperatorCallback& callback,
        std::list<AccessibilityElementInfo> &infos, const std::list<AccessibilityElementInfo> &treeInfos,
        const int32_t requestId);

    void SetSearchElementInfoByTextResult(Accessibility::AccessibilityElementOperatorCallback& callback,
        std::list<Accessibility::AccessibilityElementInfo>&& infos, const int32_t requestId);

    void SetSearchDefaultFocusByWindowIdResult(Accessibility::AccessibilityElementOperatorCallback& callback,
        std::list<Accessibility::AccessibilityElementInfo>&& infos, const int32_t requestId);

    void SetFindFocusedElementInfoResult(Accessibility::AccessibilityElementOperatorCallback& callback,
        Accessibility::AccessibilityElementInfo& info, const int32_t requestId);

    void SetFocusMoveSearchResult(Accessibility::AccessibilityElementOperatorCallback& callback,
        Accessibility::AccessibilityElementInfo& info, const int32_t requestId);

    void SetExecuteActionResult(
        Accessibility::AccessibilityElementOperatorCallback& callback, const bool succeeded, const int32_t requestId);

    void SearchExtensionElementInfoByAccessibilityIdNG(const SearchParameter& searchParam,
        const RefPtr<NG::FrameNode>& node, std::list<Accessibility::AccessibilityElementInfo>& infos,
        const RefPtr<PipelineBase>& context, const RefPtr<NG::PipelineContext>& ngPipeline);
    void SearchElementInfosByTextNG(const SearchParameter& searchParam, const RefPtr<NG::FrameNode>& node,
        std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::PipelineContext>& ngPipeline);
    std::list<Accessibility::AccessibilityElementInfo> SearchElementInfosByTextNG(
        int64_t elementId, const std::string& text, const RefPtr<NG::FrameNode>& node, int64_t offset);
    void FindFocusedExtensionElementInfoNG(const SearchParameter& searchParam,
        Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::FrameNode>& root);
    void FocusExtensionElementMoveSearchNG(const SearchParameter& searchParam,
        Accessibility::AccessibilityElementInfo& info, const RefPtr<PipelineBase>& context,
        const RefPtr<NG::FrameNode>& root, RefPtr<NG::FrameNode>& outputExtensionNode);
    void TransferExecuteAction(int64_t elementId, RefPtr<NG::FrameNode>& node,
        const std::map<std::string, std::string>& actionArguments,
        Accessibility::ActionType& action, int64_t uiExtensionOffset);
    RefPtr<NG::FrameNode> FindNodeFromRootByExtensionId(const RefPtr<NG::FrameNode>& root, const int64_t uiExtensionId);
    bool RegisterThirdProviderInteractionOperationAsChildTree(const Registration& registration);

    void DumpProperty(const RefPtr<AccessibilityNode>& node);
    void DumpPropertyInfo(const RefPtr<NG::FrameNode>& frameNode, AccessibilityElementInfo& nodeInfo);
    void DumpPropertyNG(int64_t nodeID);
    void DumpHoverTestNG(uint32_t windowId, int64_t nodeID, int32_t x, int32_t y, bool verbose);
    RefPtr<NG::PipelineContext> FindPipelineByElementId(const int64_t elementId, RefPtr<NG::FrameNode>& node);
    RefPtr<NG::FrameNode> FindNodeFromPipeline(const WeakPtr<PipelineBase>& context, const int64_t elementId);
    RefPtr<PipelineBase> GetPipelineByWindowId(const int32_t windowId);

    RefPtr<NG::PipelineContext> GetPipelineByWindowId(uint32_t windowId);
    void DumpTreeNG(bool useWindowId, uint32_t windowId, int64_t rootId, bool isDumpSimplify = false);
    void DumpTreeNG(const RefPtr<NG::FrameNode>& parent, int32_t depth,
        int64_t nodeID, const CommonProperty& commonProperty, bool isDumpSimplify = false);
    void DumpTreeNodeSafeAreaInfoNg(const RefPtr<NG::FrameNode>& node);
    void DumpPadding(const std::unique_ptr<NG::PaddingProperty>& padding, std::string label);
    void DumpBorder(const std::unique_ptr<NG::BorderWidthProperty>& border, std::string label);
    void DumpTreeNodeCommonInfoNg(const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty);
    void DumpTreeNodeSimplifyInfoNG(
        const RefPtr<NG::FrameNode>& node, int32_t depth, const CommonProperty& commonProperty, int32_t childSize);
    void DumpTreeAccessibilityNodeNG(const RefPtr<NG::UINode>& uiNodeParent,
        int32_t depth, int64_t nodeID, const CommonProperty& commonProperty);
    bool CheckDumpInfoParams(const std::vector<std::string> &params);
    void DumpSendEventTest(int64_t nodeId, int32_t eventId, const std::vector<std::string>& params);
    void DumpInjectActionTest(const std::vector<std::string>& params);
    void DumpExecuteActionTest(const std::vector<std::string>& params);
    void DumpCustomActionTest(const std::vector<std::string>& params);
    void DumpSetComponentTypeTest(const std::vector<std::string>& params);
    void DumpClearComponentTypeTest(const std::vector<std::string>& params);
    void DumpEmbedSearchTest(const std::vector<std::string>& params);
    void DumpEmbedHoverTestNG(const std::vector<std::string>& params, uint32_t windowId);
    void DumpSetCheckListTest(const std::vector<std::string>& params);
    void DumpGetCheckListTest(const std::vector<std::string>& params);

    void DumpSpecificPropertySearchTest(const std::vector<std::string>& params, uint32_t windowId);

    void DumpSetCustomPropertyTest(const DumpInfoArgument& argument);
    void DumpGetCustomPropertyTest(int64_t nodeId);
    bool ParseSetCustomPropertyParams(std::vector<std::string>::const_iterator& arg,
        const std::vector<std::string>& params, DumpInfoArgument& argument);
    bool ParseGetCustomPropertyParams(std::vector<std::string>::const_iterator& arg,
        const std::vector<std::string>& params, DumpInfoArgument& argument);

    void DumpAddVirtualNodeTest(const DumpInfoArgument& argument);
    void DumpRemoveVirtualNodeTest(const DumpInfoArgument& argument);
    void DumpGetVirtualNodeTest(const DumpInfoArgument& argument);
    bool ParseAddVirtualNodeParams(std::vector<std::string>::const_iterator& arg,
        const std::vector<std::string>& params, DumpInfoArgument& argument);
    bool ParseRemoveVirtualNodeParams(std::vector<std::string>::const_iterator& arg,
        const std::vector<std::string>& params, DumpInfoArgument& argument);
    bool ParseGetVirtualNodeParams(std::vector<std::string>::const_iterator& arg,
        const std::vector<std::string>& params, DumpInfoArgument& argument);
    bool ParsePerformVirtualNodeActionParams(std::vector<std::string>::const_iterator& arg,
        const std::vector<std::string>& params, DumpInfoArgument& argument);

    void DumpPerformVirtualNodeActionTest(const DumpInfoArgument& argument);
    void DumpVirtualAccessibilityNodeTree(const RefPtr<NG::FrameNode>& hostNode,
        const RefPtr<NG::VirtualAccessibilityNode>& virtualNode, int32_t depth);

    void UpdateAccessibilityElementInfo(const int64_t elementId,
        const AccessibilityVirtualNode& accessibilityVirtualNode, const int32_t requestId,
        AccessibilityElementOperatorCallback& callback, const int32_t windowId);

    void GenerateCommonPropertyForWeb(const RefPtr<PipelineBase>& context, CommonProperty& output,
            const RefPtr<PipelineBase>& mainContext, const RefPtr<NG::FrameNode>& node);

    void FindText(const RefPtr<NG::UINode>& node, std::list<Accessibility::AccessibilityElementInfo>& infos,
        const RefPtr<NG::PipelineContext>& context,
        const CommonProperty& commonProperty, const SearchParameter& searchParam);

    void FindTextByTextHint(const RefPtr<NG::UINode>& node, std::list<Accessibility::AccessibilityElementInfo>& infos,
        const RefPtr<NG::PipelineContext>& context,
        const CommonProperty& commonProperty, const SearchParameter& searchParam);

    void UpdateAccessibilityElementInfo(
        const RefPtr<NG::FrameNode>& node, Accessibility::AccessibilityElementInfo& nodeInfo);

    void UpdateAccessibilityVisible(
        const RefPtr<NG::FrameNode>& node, Accessibility::AccessibilityElementInfo& nodeInfo);

    void UpdateVirtualNodeInfo(std::list<Accessibility::AccessibilityElementInfo>& infos,
        Accessibility::AccessibilityElementInfo& nodeInfo,
        const RefPtr<NG::UINode>& uiVirtualNode, const CommonProperty& commonProperty,
        const RefPtr<NG::PipelineContext>& ngPipeline);

    void UpdateVirtualNodeChildAccessibilityElementInfo(
        const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
        Accessibility::AccessibilityElementInfo& nodeParentInfo, Accessibility::AccessibilityElementInfo& nodeInfo,
        const RefPtr<NG::PipelineContext>& ngPipeline);

    void UpdateVirtualNodeAccessibilityElementInfo(
        const RefPtr<NG::FrameNode>& parent, const RefPtr<NG::FrameNode>& node,
        const CommonProperty& commonProperty, Accessibility::AccessibilityElementInfo& nodeInfo,
        const RefPtr<NG::PipelineContext>& ngPipeline);

    void UpdateElementInfo(
        const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
        AccessibilityElementInfo& nodeInfo, const RefPtr<NG::PipelineContext>& ngPipeline);

    void UpdateHasChildText(
        const RefPtr<NG::FrameNode>& node, AccessibilityElementInfo& nodeInfo);

    void UpdateCacheInfoNG(std::list<Accessibility::AccessibilityElementInfo>& infos, const RefPtr<NG::FrameNode>& node,
        CommonProperty& commonProperty, const RefPtr<NG::PipelineContext>& ngPipeline,
        const SearchParameter& searchParam);
    void UpdateEventWhiteList(const std::vector<uint32_t>& eventList);
#ifdef WEB_SUPPORTED

    void WebSetScreenRect(const std::shared_ptr<NG::TransitionalNodeInfo>& node, const CommonProperty& commonProperty,
        AccessibilityElementInfo& nodeInfo);

    void UpdateWebAccessibilityElementInfo(const std::shared_ptr<NG::TransitionalNodeInfo>& node,
        Accessibility::AccessibilityElementInfo& nodeInfo, int32_t treeId);

    void UpdateWebAccessibilityElementInfo(const std::shared_ptr<NG::TransitionalNodeInfo>& node,
        const CommonProperty& commonProperty, Accessibility::AccessibilityElementInfo& nodeInfo,
        const RefPtr<NG::WebPattern>& webPattern);

    void UpdateWebCacheInfo(std::list<Accessibility::AccessibilityElementInfo>& infos, int64_t nodeId,
        const CommonProperty& commonProperty, const RefPtr<NG::PipelineContext>& ngPipeline,
        const SearchParameter& searchParam, const RefPtr<NG::WebPattern>& webPattern);
    bool GetChildrenFromEmbedNode(AccessibilityElementInfo& nodeInfo, int64_t nodeId, std::list<int64_t>& children,
        std::unordered_map<int64_t, AccessibilityElementInfo>& allEmbedNodeTreeInfosMap);
    void PushElementsIntoInfos(AccessibilityElementInfo& nodeInfo, std::list<AccessibilityElementInfo>& infos,
        const RefPtr<NG::WebPattern>& webPattern, std::list<int64_t>& children,
        std::unordered_map<int64_t, AccessibilityElementInfo>& allEmbedNodeTreeInfosMap);

    int64_t GetWebAccessibilityIdBySurfaceId(const std::string& surfaceId);
#endif //WEB_SUPPORTED
    void NotifyChildTreeOnRegister(int32_t treeId);

    void NotifyChildTreeOnDeregister();

    void SendUecOnTreeEvent(int64_t splitElementId);

    void NotifySetChildTreeIdAndWinId(int64_t elementId, const int32_t treeId, const int32_t childWindowId);

    bool CheckIsChildElement(
        int64_t &elementId,
        const std::vector<std::string>& params,
        std::vector<std::string>& info,
        DumpMode mode,
        int64_t &rootId);

    bool NeedRegisterChildTree(uint32_t parentWindowId, int32_t parentTreeId, int64_t parentElementId);

    void FillEventInfoWithNode(
        const RefPtr<NG::FrameNode>& node,
        Accessibility::AccessibilityEventInfo& eventInfo,
        const RefPtr<NG::PipelineContext>& context,
        int64_t elementId);

    void NotifyAccessibilitySAStateChange(bool state);
    void DumpTreeNodeInfoInJson(
        const RefPtr<NG::FrameNode>& node, int32_t depth, const CommonProperty& commonProperty, int32_t childSize);
    void CreateNodeInfoJson(const RefPtr<NG::FrameNode>& node, const CommonProperty& commonProperty,
        std::unique_ptr<JsonValue>& json, int32_t childSize);
    bool IsEventIgnoredByWorkMode(const AccessibilityEvent& accessibilityEvent);
    void UpdateChildrenNodeInCache(std::list<AccessibilityElementInfo>& infos,
        const CommonProperty& commonProperty, const RefPtr<NG::PipelineContext>& ngPipeline,
        const SearchParameter& searchParam, std::list<RefPtr<NG::FrameNode>>& children);

    void RegisterDynamicRenderGetParentRectHandler();

    void SendEventToAccessibilityWithNodeInner(const AccessibilityEvent& accessibilityEvent,
        const RefPtr<AceType>& node, const RefPtr<PipelineBase>& context);
    void SendEventToAccessibilityWithNodeInnerAfterRender(const AccessibilityEvent& accessibilityEvent,
        const RefPtr<AceType>& node, const RefPtr<PipelineBase>& context);

    void SendAccessibilityAsyncEventInner(const AccessibilityEvent& accessibilityEvent);
    void SendAccessibilityAsyncEventInnerAfterRender(const AccessibilityEvent& accessibilityEvent);

    int64_t GetDelayTimeBeforeSendEvent(const AccessibilityEvent& accessibilityEvent, const RefPtr<AceType>& node);

    void GetCurrentWindowPages(
        const RefPtr<NG::PipelineContext>& ngPipeline,
        std::vector<RefPtr<NG::FrameNode>>& pageNodes,
        std::vector<std::string>& pagePaths);
    const std::string GetPagePathInPageNodes(
        int32_t pageId,
        const std::vector<RefPtr<NG::FrameNode>>& pageNodes,
        const std::vector<std::string> pagePaths);

    bool CheckPageEventValidInCache(int32_t containerId);
    bool CheckPageEventByPageInCache(int32_t containerId, int32_t pageId);
    void ReleaseAllCacheAccessibilityEvent(int32_t containerId);
    void ReleaseCacheAccessibilityEvent(int32_t containerId, int32_t pageId);
    void ReleaseAccessibilityEventList(
        const std::list<std::pair<int32_t, AccessibilityEvent>>& eventList);

    SearchSurfaceIdRet SearchElementInfoBySurfaceId(
        const std::string& surfaceId, const int32_t windowId,
        const SearchSurfaceIdType searchType, std::list<AccessibilityElementInfo>& infos);
    bool GetWebAccessibilityInfoBySurfaceId(
        const std::string& surfaceId,
        AccessibilityElementInfo& elementInfo);

    bool CheckWhiteList(const uint32_t& eventType);

    ActionType ConvertAceAction(AceAction aceAction);

    void CheckStateTakeOver(const RefPtr<NG::FrameNode>& node, AccessibilityElementInfo& nodeInfo);
    void CheckActionTakeOver(const RefPtr<NG::FrameNode>& node, AccessibilityElementInfo& nodeInfo);
    void UpdateUserAccessibilityElementInfo(
        const RefPtr<NG::AccessibilityProperty>& accessibilityProperty, AccessibilityElementInfo& nodeInfo);
    void NotifyScreenReaderObserverStateChange(bool state);

    // focus move
    void ProcessGetScrollAncestorNode(
        const int64_t elementId, const Accessibility::AccessibilityFocusMoveParam param,
        const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback& callback);

    void ProcessGetScrollAncestor(
        const int64_t elementId, const Accessibility::AccessibilityFocusMoveParam param,
        const RefPtr<PipelineBase>& context, const int32_t requestId,
        Accessibility::AccessibilityElementOperatorCallback& callback);

    FocusMoveResult FocusMoveSearchWithConditionNG(
        const int64_t elementId, const Accessibility::AccessibilityFocusMoveParam param,
        const RefPtr<PipelineBase>& context, Accessibility::AccessibilityElementInfo& info);

    void FocusMoveSearchAccessibilityNodeWithCondition(
        const int64_t elementId, const Accessibility::AccessibilityFocusMoveParam param,
        const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback &callback);

    void ActAccessibilityActionPreHandle(Accessibility::ActionType action,
     const RefPtr<NG::FrameNode>& frameNode);
    bool ActAccessibilityAction(Accessibility::ActionType action,
        const std::map<std::string, std::string>& actionArguments,
        RefPtr<NG::AccessibilityProperty> accessibilityProperty, const RefPtr<NG::FrameNode>& frameNode);

    void ClearAccessibilityFocusState();

    // Check if current pipeline context is form render
    bool IsFormRender();
    bool SearchAccessibilityVirtualNode(int64_t elementId, int32_t mode,
        std::list<AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context);
    bool GetAllVirtualNodeElementInfo(std::list<AccessibilityElementInfo>& infos,
        const CommonProperty& commonProperty, const RefPtr<NG::PipelineContext>& ngPipeline,
        const RefPtr<NG::FrameNode>& containerNode, AccessibilityElementInfo& parentNodeInfo);

    bool SearchAccessibilityVirtualNode(int64_t elementId,
        std::list<AccessibilityElementInfo>& infos, const RefPtr<PipelineBase>& context);
    bool GetAllVirtualNodeElementId(std::list<AccessibilityElementInfo>& infos,
        const CommonProperty& commonProperty,
        const RefPtr<NG::FrameNode>& containerNode, AccessibilityElementInfo& parentNodeInfo);

    std::string callbackKey_;
    uint32_t windowId_ = 0;
    std::unordered_map<uint32_t, std::shared_ptr<JsAccessibilityStateObserver>> stateObserver_;
    std::shared_ptr<ToastAccessibilityConfigObserver> toastObserver_ = nullptr;
    float scaleX_ = 1.0f;
    float scaleY_ = 1.0f;
    int64_t currentFocusNodeId_ = -1;
    bool isScreenReaderEnabled_ = false;
    int64_t currentFocusVirtualNodeParentId_ = -1;
    bool isScreenReaderEnabledInitialized_ = false;
    bool isSelectReaderEnabled_ = false;
    bool isSelectReaderEnabledInitialized_ = false;

    int64_t lastElementId_ = -1;
    WeakPtr<NG::FrameNode> lastFrameNode_;
    NG::SizeF oldGeometrySize_;
    mutable std::mutex childTreeCallbackMapMutex_;
    std::unordered_map<int64_t, std::shared_ptr<AccessibilityChildTreeCallback>> childTreeCallbackMap_;
    std::unordered_map<int64_t, std::weak_ptr<AccessibilityChildTreeCallback>> detachedCmpChildTreeCallbacks_;
    mutable std::mutex componentSACallbackMutex_;
    std::unordered_map<int64_t, std::shared_ptr<AccessibilitySAObserverCallback>> componentSACallbackMap_;
    mutable std::mutex componentScreenReaderCallbackMutex_;
    std::unordered_map<int64_t, std::shared_ptr<AccessibilityScreenReaderObserverCallback>>
        componentScreenReaderCallbackMap_;
    int64_t parentElementId_ = INVALID_PARENT_ID;
    uint32_t parentWindowId_ = 0;
    int32_t parentTreeId_ = 0;
    uint32_t parentWebWindowId_ = 0;
    std::function<void(int32_t&, int32_t&)> getParentRectHandler_;
    std::function<void(AccessibilityParentRectInfo&)> getParentRectHandlerNew_;
    const int32_t shiftBits = 16;
    bool isUseJson_ = false;
    bool reentrantLimit_ = false;
    std::optional<std::string> pageMode_ = std::nullopt;
    std::queue<AccessibilityEvent> eventQueue_;
    std::vector<AccessibilityEvent> cacheEventVec_;
    std::mutex detachFocusFallbackCacheMutex_;
    std::unordered_map<int32_t, DetachFocusFallbackData> detachFocusFallbackCache_;
    std::list<WeakPtr<NG::FrameNode>> defaultFocusList_;
    std::vector<std::pair<WeakPtr<NG::FrameNode>, bool>> extensionComponentStatusVec_;
    std::unordered_map<int32_t, std::optional<AccessibilityEvent>> pageIdEventMap_;
    NG::NextFocusRelationController nextFocusRelationController_;
    std::vector<uint32_t> eventWhiteList_;

    AccessibilityParentRectInfo uecRectInfo_;
    NG::PageEventController pageController_;
    NG::HoverTransparentCallbackController hoverTransparentCallbackController_;

    bool isIgnoreAllAction_ = false;

    NG::AccessibilityEventBlockerInAction blockerInAction_;

    struct CmpRegistration {
        explicit CmpRegistration(uint32_t windowId) : windowId(windowId) {}
        void SetTreeId(int32_t id);
        void Complete(bool accepted);
        void Cancel();
        int32_t GetAcceptedTreeId();
        bool IsCanceled();
        void CleanupIfCanceled();

        std::mutex mutex;
        const uint32_t windowId;
        int32_t treeId = -1;
        bool finished = false;
        bool canceled = false;
        bool cleaned = false;
    };

    // CMP mixed-rendering: binding state for exported subtree binding.
    struct CmpBindingState {
        WeakPtr<NG::FrameNode> owner;
        WeakPtr<NG::FrameNode> exportedRoot;
        RefPtr<JsAccessibilityManager> subAM; // strong ref: subAM lifecycle = binding lifecycle
        std::shared_ptr<CmpRegistration> registration;
        int32_t cmpTreeId = -1; // CMP (parent) tree treeId — set at convergence
        uint32_t windowId = 0; // host window id — set at registration, used by detach
        // WAIT_TREE_ID: waiting for the CMP treeId.
        enum class State { UNBOUND, WAIT_TREE_ID, REGISTERING, REGISTERED } state = State::UNBOUND;
    };
    // (owner nodeId, bindId) -> binding state.
    std::map<std::pair<int32_t, int64_t>, CmpBindingState> cmpBindingMap_;
    std::mutex cmpBindingMapMutex_;
    // CMP mixed-rendering: set by BindExportTree on every subAM it creates (see MarkCmpSubAM).
    bool isCmpSubAM_ = false;
    // CMP mixed-rendering: escalation anchors set on the subAM at RegisterCmpSubtreeToAams (used by.
    WeakPtr<NG::FrameNode> cmpExportedRoot_;
    std::weak_ptr<JsThirdProviderInteractionOperation> cmpProviderOperation_;

    // CMP mixed-rendering: dedicated operator for the exported subtree.
    class CmpInteractionOperation : public Accessibility::AccessibilityElementOperator {
    public:
        CmpInteractionOperation(int32_t parentTreeId, int64_t hostElementId,
            const WeakPtr<NG::FrameNode>& exportedRoot, int32_t ownerNodeId,
            const WeakPtr<JsAccessibilityManager>& bindingOwner,
            const std::shared_ptr<CmpRegistration>& registration = nullptr)
            : js_(), bindingOwner_(bindingOwner), exportedRoot_(exportedRoot),
              parentTreeId_(parentTreeId), hostElementId_(hostElementId), ownerNodeId_(ownerNodeId),
              registration_(registration) {}
        ~CmpInteractionOperation() override = default;

        void SetHandler(const WeakPtr<JsAccessibilityManager>& js)
        {
            js_ = js;
        }
        const WeakPtr<JsAccessibilityManager>& GetHandler() const
        {
            return js_;
        }
        const WeakPtr<NG::FrameNode>& GetExportedRoot() const
        {
            return exportedRoot_;
        }
        int32_t GetParentTreeId() const
        {
            return parentTreeId_;
        }
        int64_t GetHostElementId() const
        {
            return hostElementId_;
        }

        // AccessibilityElementOperator overrides.
        RetError SearchElementInfoByAccessibilityId(const int64_t elementId, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t mode) override;
        void SearchElementInfoBySpecificProperty(const int64_t elementId, const SpecificPropertyParam& param,
            const int32_t requestId, Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void SearchElementInfosByText(const int64_t elementId, const std::string& text, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void SearchDefaultFocusByWindowId(const int32_t windowId, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t pageId) override;
        void FindFocusedElementInfo(const int64_t elementId, const int32_t focusType, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void FocusMoveSearch(const int64_t elementId, const int32_t direction, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void ExecuteAction(const int64_t elementId, const int32_t action,
            const std::map<std::string, std::string>& actionArguments, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void ClearFocus() override;
        void OutsideTouch() override;
        void GetCursorPosition(const int64_t elementId, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void SetChildTreeIdAndWinId(const int64_t nodeId, const int32_t treeId,
            const int32_t childWindowId) override;
        void SetBelongTreeId(const int32_t treeId) override;
        void FocusMoveSearchWithCondition(const AccessibilityElementInfo& info,
            const AccessibilityFocusMoveParam param, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void UpdateCustomAccessibilityProperty(const int64_t elementId,
            const AccessibilityVirtualNode& accessibilityVirtualNode, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void AddAccessibilityVirtualNode(const int64_t elementId,
            const std::vector<AccessibilityVirtualNode>& nodes, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;
        void RemoveAccessibilityVirtualNode(const int64_t elementId, const int32_t requestId,
            Accessibility::AccessibilityElementOperatorCallback& callback) override;

    private:
        // Focus fallback: when no next focus is found in the exported subtree, return ElementInfo with.
        void ReturnFocusFallback(Accessibility::AccessibilityElementOperatorCallback& callback,
            const int32_t requestId);

        // Same F-05 host anchor through the with-condition channel: FocusMoveSearchWithCondition requests are answered.
        static void ReturnFocusFallbackWithCondition(int32_t parentTreeId, int64_t hostElementId,
            Accessibility::AccessibilityElementOperatorCallback& callback, const int32_t requestId);

        WeakPtr<JsAccessibilityManager> js_; // SetHandler → subAM (ADR-1 operator wiring)
        // Main AM - STATE-1 cmpBindingMap_ owner.
        WeakPtr<JsAccessibilityManager> bindingOwner_;
        WeakPtr<NG::FrameNode> exportedRoot_; // search root (Bind 3rd param, Owner rule)
        int32_t parentTreeId_ = -1; // CMP tree treeId (host tree)
        int64_t hostElementId_ = -1; // bindId (mount node accessibility ID in CMP tree)
        int32_t ownerNodeId_ = -1; // binding owner node id (MarkCmpBindingRegistered key part)
        std::shared_ptr<CmpRegistration> registration_;
    };
};

} // namespace OHOS::Ace::Framework

#endif // FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_ACCESSIBILITY_JS_ACCESSIBILITY_MANAGER_H
