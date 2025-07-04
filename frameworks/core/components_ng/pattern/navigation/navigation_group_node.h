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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVIGATION_GROUP_NODE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVIGATION_GROUP_NODE_H

#include <atomic>
#include <cstdint>
#include <list>

#include "base/memory/referenced.h"
#include "core/animation/page_transition_common.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/group_node.h"
#include "core/components_ng/pattern/navigation/bar_item_node.h"
#include "core/components_ng/pattern/navigation/navigation_declaration.h"
#include "core/components_ng/pattern/navigation/navigation_stack.h"
#include "core/components_ng/pattern/navigation/title_bar_node.h"
#include "core/components_ng/pattern/navrouter/navdestination_group_node.h"
#include "core/components_ng/pattern/navrouter/navrouter_pattern.h"
#include "core/components_ng/property/property.h"

namespace OHOS::Ace::NG {
class InspectorFilter;

struct TransitionUnitInfo {
    const RefPtr<FrameNode>& transitionNode;
    bool isUseCustomTransition = false;
    int32_t animationId = -1;

    TransitionUnitInfo(const RefPtr<FrameNode>& node, bool isUseCustomTransition, int32_t animationId)
        : transitionNode(node), isUseCustomTransition(isUseCustomTransition), animationId(animationId)
    {}
};

class ACE_FORCE_EXPORT NavigationGroupNode : public GroupNode {
    DECLARE_ACE_TYPE(NavigationGroupNode, GroupNode)
public:
    NavigationGroupNode(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern)
        : GroupNode(tag, nodeId, pattern)
    {}

    ~NavigationGroupNode() override;

    using AnimationFinishCallback = std::function<void()>;

    void AddChildToGroup(const RefPtr<UINode>& child, int32_t slot = DEFAULT_NODE_SLOT) override;

    // remain child needs to keep to use pop animation
    void UpdateNavDestinationNodeWithoutMarkDirty(const RefPtr<UINode>& remainChild, bool modeChange = false);
    static RefPtr<NavigationGroupNode> GetOrCreateGroupNode(
        const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator);

    bool IsAtomicNode() const override
    {
        return false;
    }

    void SetPrimaryContentNode(const RefPtr<UINode>& content)
    {
        primaryContentNode_ = content;
    }
    const RefPtr<UINode>& GetPrimaryContentNode() const
    {
        return primaryContentNode_;
    }
    void SetForceSplitPlaceHolderNode(const RefPtr<UINode>& node)
    {
        forceSplitPlaceHolderNode_ = node;
    }
    const RefPtr<UINode>& GetForceSplitPlaceHolderNode() const
    {
        return forceSplitPlaceHolderNode_;
    }

    void SetNavBarNode(const RefPtr<UINode>& navBarNode)
    {
        navBarNode_ = navBarNode;
    }

    const RefPtr<UINode>& GetNavBarNode() const
    {
        return navBarNode_;
    }
    const RefPtr<UINode>& GetHomeDestinationNode() const
    {
        return customHomeDestination_;
    }
    const RefPtr<UINode>& GetNavBarOrHomeDestinationNode() const;
    bool IsNavBarOrHomeDestination(const RefPtr<UINode>& node) const;

    const std::optional<bool> GetUseHomeDestination() const
    {
        return useHomeDestination_;
    }
    void SetUseHomeDestinatoin(bool use)
    {
        useHomeDestination_ = use;
    }

    void CreateHomeDestinationIfNeeded();

    void SetSplitPlaceholder(const RefPtr<NG::UINode>& splitPlaceholder);

    void SetPlaceholderContentNode(const RefPtr<NG::UINode>& placeholderContentNode)
    {
        placeholderContentNode_ = placeholderContentNode;
    }

    RefPtr<UINode> GetPlaceholderContentNode() const
    {
        return placeholderContentNode_;
    }

    void ResetSplitPlaceholder()
    {
        if (placeholderContentNode_) {
            RemoveChild(placeholderContentNode_);
        }
        placeholderContentNode_ = nullptr;
        splitPlaceholder_ = nullptr;
    }

    void SetContentNode(const RefPtr<UINode>& contentNode)
    {
        contentNode_ = contentNode;
    }

    const RefPtr<UINode>& GetContentNode() const
    {
        return contentNode_;
    }

    void SetDividerNode(const RefPtr<UINode>& dividerNode)
    {
        dividerNode_ = dividerNode;
    }

    const RefPtr<UINode>& GetDividerNode() const
    {
        return dividerNode_;
    }

    const std::string& GetCurId() const
    {
        return curId_;
    }

    bool GetIsModeChange() const
    {
        return isModeChange_;
    }

    void SetIsModeChange(bool isModeChange)
    {
        isModeChange_ = isModeChange;
    }

    bool GetNeedSetInvisible() const
    {
        return needSetInvisible_;
    }

    void SetNeedSetInvisible(bool needSetInvisible)
    {
        needSetInvisible_ = needSetInvisible;
    }

    bool IsOnModeSwitchAnimation()
    {
        return isOnModeSwitchAnimation_;
    }

    void SetDoingModeSwitchAnimationFlag(bool isOnAnimation)
    {
        isOnModeSwitchAnimation_ = isOnAnimation;
    }

    std::list<std::shared_ptr<AnimationUtils::Animation>>& GetPushAnimations()
    {
        return pushAnimations_;
    }

    std::list<std::shared_ptr<AnimationUtils::Animation>>& GetPopAnimations()
    {
        return popAnimations_;
    }

    void CleanPushAnimations()
    {
        pushAnimations_.clear();
    }

    void CleanPopAnimations()
    {
        popAnimations_.clear();
    }

    bool CheckCanHandleBack(bool& isEntry);
    
    void CheckIsNeedForceExitWindow(bool result);

    void OnInspectorIdUpdate(const std::string& id) override;

    bool HandleBack(const RefPtr<FrameNode>& node, bool isLastChild, bool isOverride);

    void ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const override;
    static RefPtr<UINode> GetNavDestinationNode(RefPtr<UINode> uiNode);
    void SetBackButtonEvent(const RefPtr<NavDestinationGroupNode>& navDestination);

    void ConfigureNavigationWithAnimation(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode);
    void ResetTransitionAnimationNodeState(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode);
    RefPtr<NavigationManager> FetchNavigationManager();
    void TransitionWithPop(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode,
        bool isNavBarOrHomeDestination = false);
    void TransitionWithPush(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode,
        bool isNavBarOrHomeDestination = false);
    virtual void CreateAnimationWithPop(const TransitionUnitInfo& preInfo, const TransitionUnitInfo& curInfo,
        const AnimationFinishCallback finishCallback, bool isNavBarOrHomeDestination = false);
    virtual void CreateAnimationWithPush(const TransitionUnitInfo& preInfo, const TransitionUnitInfo& curInfo,
        const AnimationFinishCallback finishCallback, bool isNavBarOrHomeDestination = false);
    void CreateSoftAnimationWithPush(const TransitionUnitInfo& preInfo, const TransitionUnitInfo& curInfo,
        const AnimationFinishCallback finishCallback, bool isNavBar = false);
    void CreateSoftAnimationWithPop(const TransitionUnitInfo& preInfo, const TransitionUnitInfo& curInfo,
        const AnimationFinishCallback finishCallback, bool isNavBar = false);
    virtual void ResetSystemAnimationProperties(const RefPtr<FrameNode>& navDestinationNode);

    std::shared_ptr<AnimationUtils::Animation> BackButtonAnimation(
        const RefPtr<FrameNode>& backButtonNode, bool isTransitionIn);
    std::shared_ptr<AnimationUtils::Animation> MaskAnimation(const RefPtr<FrameNode>& curNode, bool isTransitionIn);
    std::shared_ptr<AnimationUtils::Animation> TitleOpacityAnimation(
        const RefPtr<FrameNode>& node, bool isTransitionOut);
    void TransitionWithReplace(
        const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode, bool isNavBarOrHomeDestination);
    void DealNavigationExit(const RefPtr<FrameNode>& preNode, bool isNavBarOrHomeDestination, bool isAnimated = true);
    void NotifyPageHide();
    void UpdateLastStandardIndex();

    int32_t GetPreLastStandardIndex() const
    {
        return preLastStandardIndex_;
    }

    void PreNodeFinishCallback(const RefPtr<FrameNode>& preNode);
    void CreateAnimationWithDialogPop(const AnimationFinishCallback callback,
    const std::vector<WeakPtr<FrameNode>> prevNavList, const std::vector<WeakPtr<FrameNode>> curNavList);
    void CreateAnimationWithDialogPush(const AnimationFinishCallback callback,
    const std::vector<WeakPtr<FrameNode>> prevNavList, const std::vector<WeakPtr<FrameNode>> curNavList);
    void TransitionWithDialogPush(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode,
        bool isNavBar = false);
    void TransitionWithDialogPop(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode,
        bool isNavBar = false);
    void StartDialogtransition(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode,
        bool isTransitionIn);

    void InitPopPreList(const RefPtr<FrameNode>& preNode, std::vector<WeakPtr<FrameNode>>& preNavList,
        const std::vector<WeakPtr<FrameNode>>& curNavList);
    void InitPopCurList(const RefPtr<FrameNode>& curNode, std::vector<WeakPtr<FrameNode>>& curNavList,
        bool isNavbarNeedAnimation);
    void InitPushPreList(const RefPtr<FrameNode>& preNode, std::vector<WeakPtr<FrameNode>>& prevNavList,
        const std::vector<WeakPtr<FrameNode>>& curNavList, bool isNavbarNeedAnimation);
    void InitPushCurList(const RefPtr<FrameNode>& curNode, std::vector<WeakPtr<FrameNode>>& curNavList);

    std::vector<WeakPtr<NavDestinationGroupNode>> FindNodesPoped(const RefPtr<FrameNode>& preNode,
        const RefPtr<FrameNode>& curNode);
    void DialogTransitionPopAnimation(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode,
        AnimationOption option);
    void DialogTransitionPushAnimation(const RefPtr<FrameNode>& preNode, const RefPtr<FrameNode>& curNode,
        AnimationOption option);

    int32_t GetLastStandardIndex() const
    {
        return lastStandardIndex_;
    }

    AnimationOption CreateAnimationOption(const RefPtr<Curve>& curve, FillMode mode,
        int32_t duration, const NavigationGroupNode::AnimationFinishCallback& callback);

    NavigationMode GetNavigationMode();

    void SetIsOnAnimation(bool isOnAnimation)
    {
        isOnAnimation_ = isOnAnimation;
    }
    RefPtr<FrameNode> GetTopDestination();
    void OnDetachFromMainTree(bool recursive, PipelineContext* context = nullptr) override;
    void OnAttachToMainTree(bool recursive) override;

    void FireHideNodeChange(NavDestinationLifecycle lifecycle);

    void ReduceModeSwitchAnimationCnt()
    {
        --modeSwitchAnimationCnt_;
    }

    void IncreaseModeSwitchAnimationCnt()
    {
        ++modeSwitchAnimationCnt_;
    }

    int32_t GetModeSwitchAnimationCnt()
    {
        return modeSwitchAnimationCnt_;
    }

    float CheckLanguageDirection();

    void RemoveDialogDestination(bool isReplace = false, bool isTriggerByInteractiveCancel = false);
    void AddDestinationNode(const RefPtr<UINode>& parent);
    WeakPtr<NavDestinationGroupNode> GetParentDestinationNode() const
    {
        return parentDestinationNode_;
    }
    void SetNavigationPathInfo(const std::string& moduleName, const std::string& pagePath)
    {
        navigationPathInfo_ = pagePath;
        navigationModuleName_ = moduleName;
    }

    const std::string& GetNavigationPathInfo() const
    {
        return navigationPathInfo_;
    }

    void CleanHideNodes()
    {
        hideNodes_.clear();
    }

    std::vector<std::pair<RefPtr<NavDestinationGroupNode>, bool>> GetHideNodes() const
    {
        return hideNodes_;
    }

    void SetRecoverable(bool recoverable)
    {
        recoverable_ = recoverable;
    }

    bool CanRecovery() const
    {
        return recoverable_ && !curId_.empty();
    }

    void SetDragBarNode(const RefPtr<UINode>& dragNode)
    {
        dragBarNode_ = dragNode;
    }

    const RefPtr<UINode>& GetDragBarNode() const
    {
        return dragBarNode_;
    }

    int32_t MakeUniqueAnimationId()
    {
        return ++animationId_;
    }

    int32_t GetAnimationId() const
    {
        return animationId_;
    }

    bool CheckAnimationIdValid(const RefPtr<FrameNode>& curNode, const int32_t animationId);

    std::string ToDumpString();

protected:
    std::list<std::shared_ptr<AnimationUtils::Animation>> pushAnimations_;
    std::list<std::shared_ptr<AnimationUtils::Animation>> popAnimations_;
private:
    bool UpdateNavDestinationVisibility(const RefPtr<NavDestinationGroupNode>& navDestination,
        const RefPtr<UINode>& remainChild, int32_t index, size_t destinationSize,
        const RefPtr<UINode>& preLastStandardNode);
    bool ReorderNavDestination(
        const std::vector<std::pair<std::string, RefPtr<UINode>>>& navDestinationNodes,
        RefPtr<FrameNode>& navigationContentNode, int32_t& slot, bool& hasChanged);
    void RemoveRedundantNavDestination(RefPtr<FrameNode>& navigationContentNode,
        const RefPtr<UINode>& remainChild, int32_t slot, bool& hasChanged,
        const RefPtr<NavDestinationGroupNode>& preLastStandardNode);
    void ReorderAnimatingDestination(RefPtr<FrameNode>& navigationContentNode, RefPtr<UINode>& maxAnimatingDestination,
        RefPtr<UINode>& remainDestination, RefPtr<UINode>& curTopDestination);
    bool FindNavigationParent(const std::string& parentName);
    void DealRemoveDestination(const RefPtr<NavDestinationGroupNode>& destination);
    RefPtr<FrameNode> TransitionAnimationIsValid(
        const RefPtr<FrameNode>& node, bool isNavBarOrHomeDestination, bool isUseNavDestCustomTransition);
    bool CheckNeedUpdateParentNode(const RefPtr<UINode>& node);
    void RemoveJsChildImmediately(const RefPtr<FrameNode>& preNode, bool preUseCustomTransition,
        int32_t preAnimationId);

    void StartSoftOpacityAnimationPush(const RefPtr<FrameNode>& curNode);
    void StartSoftOpacityAnimationPop(const RefPtr<FrameNode>& preNode);
    void SoftTransitionAnimationPush(const RefPtr<FrameNode>& preNode,
        const RefPtr<FrameNode>& curNode, bool isNavBar, bool preUseCustomTransition, bool curUseCustomTransition,
        const NavigationGroupNode::AnimationFinishCallback& callback);
    void SoftTransitionAnimationPop(const RefPtr<FrameNode>& preNode,
        const RefPtr<FrameNode>& curNode, bool isNavBar, bool preUseCustomTransition, bool curUseCustomTransition,
        const NavigationGroupNode::AnimationFinishCallback& callback);
    bool HandleBackForHomeDestination();

    std::optional<bool> useHomeDestination_;
    RefPtr<UINode> customHomeNode_;
    RefPtr<UINode> customHomeDestination_;
    RefPtr<UINode> navBarNode_;
    RefPtr<UINode> contentNode_;
    RefPtr<UINode> dividerNode_;
    RefPtr<UINode> dragBarNode_;
    RefPtr<UINode> splitPlaceholder_;
    RefPtr<UINode> placeholderContentNode_;
    WeakPtr<NavDestinationGroupNode> parentDestinationNode_;
    // dialog hideNodes, if is true, nodes need remove
    std::vector<std::pair<RefPtr<NavDestinationGroupNode>, bool>> hideNodes_;
    std::vector<RefPtr<NavDestinationGroupNode>> showNodes_;
    int32_t lastStandardIndex_ = -1;
    std::atomic_int32_t animationId_ = 0;
    std::atomic_int32_t modeSwitchAnimationCnt_ = 0;
    bool isOnAnimation_ { false };
    bool isModeChange_ { false };
    bool needSetInvisible_ { false };
    bool isOnModeSwitchAnimation_ { false };
    bool recoverable_ { false };
    std::string curId_;
    std::string navigationPathInfo_;
    std::string navigationModuleName_;
    int32_t preLastStandardIndex_ = -1;

    //-------for force split------- begin------
    std::vector<RefPtr<NavDestinationGroupNode>> primaryNodesToBeRemoved_;
    RefPtr<UINode> primaryContentNode_;
    RefPtr<UINode> forceSplitPlaceHolderNode_;
    //-------for force split------- end  ------
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVIGATION_GROUP_NODE_H
