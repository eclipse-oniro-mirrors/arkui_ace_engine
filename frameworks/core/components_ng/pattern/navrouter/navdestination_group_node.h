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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVROUTER_NAVDESTINATION_GROUP_NODE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVROUTER_NAVDESTINATION_GROUP_NODE_H

#include <cstdint>
#include <list>

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/group_node.h"
#include "core/components_ng/pattern/navigation/navigation_declaration.h"
#include "core/components_ng/property/property.h"
#include "core/pipeline/base/element_register.h"

namespace OHOS::Ace::NG {

class CustomNodeBase;

using NavDestinationBackButtonEvent = std::function<bool(GestureEvent&)>;

class ACE_EXPORT NavDestinationGroupNode : public GroupNode {
    DECLARE_ACE_TYPE(NavDestinationGroupNode, GroupNode)
public:
    NavDestinationGroupNode(const std::string& tag, int32_t nodeId, const RefPtr<Pattern>& pattern)
        : GroupNode(tag, nodeId, pattern)
    {}
    ~NavDestinationGroupNode() override;
    void AddChildToGroup(const RefPtr<UINode>& child, int32_t slot = DEFAULT_NODE_SLOT) override;
    void DeleteChildFromGroup(int32_t slot = DEFAULT_NODE_SLOT) override;
    static RefPtr<NavDestinationGroupNode> GetOrCreateGroupNode(
        const std::string& tag, int32_t nodeId, const std::function<RefPtr<Pattern>(void)>& patternCreator);

    void SetTitleBarNode(const RefPtr<UINode>& title)
    {
        titleBarNode_ = title;
    }

    const RefPtr<UINode>& GetTitleBarNode() const
    {
        return titleBarNode_;
    }

    void SetContentNode(const RefPtr<UINode>& contentNode)
    {
        contentNode_ = contentNode;
    }

    const RefPtr<UINode>& GetContentNode() const
    {
        return contentNode_;
    }

    void SetNavDestinationBackButtonEvent(const NavDestinationBackButtonEvent& backButtonEvent)
    {
        backButtonEvent_ = backButtonEvent;
    }

    NavDestinationBackButtonEvent GetNavDestinationBackButtonEvent() const
    {
        return backButtonEvent_;
    }

    // custom node checking
    ACE_DEFINE_PROPERTY_ITEM_FUNC_WITHOUT_GROUP(PrevTitleIsCustom, bool);
    void OnPrevTitleIsCustomUpdate(bool value) {}

    void OnAttachToMainTree(bool recursive) override;

    void OnOffscreenProcess(bool recursive) override;

    void ProcessShallowBuilder();

    void SetTransitionType(PageTransitionType type)
    {
        transitionType_ = type;
    }

    void SetIsOnAnimation(bool isOnAnimation)
    {
        isOnAnimation_ = isOnAnimation;
    }

    bool IsOnAnimation() const
    {
        return isOnAnimation_;
    }

    PageTransitionType GetTransitionType() const
    {
        return transitionType_;
    }

    RefPtr<CustomNodeBase> GetNavDestinationCustomNode();

    void SetNavDestinationMode(NavDestinationMode mode)
    {
        mode_ = mode;
    }

    NavDestinationMode GetNavDestinationMode() const
    {
        return mode_;
    }

    void SetIndex(int32_t index)
    {
        index_ = index;
    }

    int32_t GetIndex() const
    {
        return index_;
    }

    void SetIsCacheNode(bool cache)
    {
        isCacheNode_ = cache;
    }

    bool IsCacheNode() const
    {
        return isCacheNode_;
    }

    void SetIsAnimated(const bool isAnimated)
    {
        isAnimated_ = isAnimated;
    }

    bool GetIsAnimated() const
    {
        return isAnimated_;
    }

    void SetNavDestinationPathInfo(const std::string& moduleName, const std::string& pagePath)
    {
        navDestinationPathInfo_ = pagePath;
        navDestinationModuleName_ = moduleName;
    }

    const std::string& GetNavDestinationPathInfo() const
    {
        return navDestinationPathInfo_;
    }

private:
    RefPtr<UINode> titleBarNode_;
    RefPtr<UINode> contentNode_;
    NavDestinationBackButtonEvent backButtonEvent_;
    bool isOnAnimation_ = false;
    int32_t index_ = -1;
    PageTransitionType transitionType_ = PageTransitionType::NONE;
    NavDestinationMode mode_ = NavDestinationMode::STANDARD;
    bool isCacheNode_ = false;
    bool isAnimated_ = false;
    std::string navDestinationPathInfo_;
    std::string navDestinationModuleName_;
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NAVROUTER_NAVDESTINATION_GROUP_NODE_H
