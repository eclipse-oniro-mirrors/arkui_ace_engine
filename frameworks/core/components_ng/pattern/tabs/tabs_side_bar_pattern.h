/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TABS_SIDE_BAR_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TABS_SIDE_BAR_PATTERN_H

#include <optional>

#include "base/memory/referenced.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/manager/recoverable/recoverable_view.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/swiper/swiper_model.h"
#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_layout_algorithm.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_side_bar_layout_algorithm.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/render/animation_utils.h"
#include "core/components_ng/event/pan_event.h"
#include "core/components/common/properties/gradient.h"
#include "core/animation/curve.h"

namespace OHOS::Ace::NG {

class TabsSideBarPattern : public Pattern {
    DECLARE_ACE_TYPE(TabsSideBarPattern, Pattern);

public:
    TabsSideBarPattern() = default;
    ~TabsSideBarPattern() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<TabsSideBarLayoutAlgorithm>();
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::SCOPE, true };
    }

    void SetHeaderNode(const RefPtr<UINode>& header)
    {
        headerNode_ = header;
    }

    void SetFooterNode(const RefPtr<UINode>& footer)
    {
        footerNode_ = footer;
    }

    void SetBottomBarNode(const RefPtr<UINode>& bottomBar)
    {
        bottomBarNode_ = bottomBar;
    }

    RefPtr<UINode> GetHeaderNode() const
    {
        return curHeaderNode_;
    }

    RefPtr<FrameNode> GetBottomBarContainerNode() const
    {
        return bottomBarContainerNode_;
    }

    RefPtr<FrameNode> GetHeaderContainerNode() const
    {
        return headerContainerNode_;
    }

    RefPtr<FrameNode> GetTabListNode() const
    {
        return tabListNode_;
    }

    RefPtr<FrameNode> GetHeaderContainerMaskBlurNode() const
    {
        return headerContainerMaskBlurNode_;
    }

    RefPtr<FrameNode> GetHeaderContainerMaskNode() const
    {
        return headerContainerMaskNode_;
    }

    RefPtr<FrameNode> GetBottomBarMaskBlurNode() const
    {
        return bottomBarMaskBlurNode_;
    }

    RefPtr<FrameNode> GetBottomBarMaskNode() const
    {
        return bottomBarMaskNode_;
    }

    RefPtr<FrameNode> GetFooterContainerNode() const
    {
        return footerContainerNode_;
    }

    RefPtr<FrameNode> GetTitleBarRowNode() const
    {
        return titleBarRowNode_;
    }

    void SetSwiperController(const RefPtr<SwiperController>& controller)
    {
        swiperController_ = controller;
    }

    void SetSideBarSearchableOptions(const TabsSidebarSearchableOptions& options)
    {
        incommingOptions_ = options;
    }

    void SetShowSidebarControlButton(bool show)
    {
        showControlButton_ = show;
    }

    void SetSidebarTitleBarOptions(const TabsSidebarTitleBarOptions& options)
    {
        titleBarOptions_ = options;
    }

    const TabsSidebarTitleBarOptions& GetSidebarTitleBarOptions() const
    {
        return titleBarOptions_;
    }

    void CreateChildNodeIfNeeded(const RefPtr<FrameNode>& tabsNode);

    bool IsScrollEffectEnabled() const
    {
        return isScrollEffectEnabled_;
    }

    RefPtr<FrameNode> GetCollapseButtonNode() const
    {
        return collapseButtonNode_;
    }

    RefPtr<FrameNode> CreateCollapseButton(const RefPtr<FrameNode>& tabsNode);

    void ClearCollapseButtonState();

    void UpdateCollapseButtonIcon();
    // Re-apply the glyph fill color (hamburger bars + triangle) from the theme;
    // used on color-configuration updates (dark/light mode switch) — the colors
    // are baked into the Shape paint properties at creation time.
    void UpdateCollapseButtonIconColor();
    // Re-apply the (possibly mirrored) left/top offsets of the hamburger bars
    // inside the collapse button icon (Stack children tagged CollapseIconButtonBar).
    void UpdateCollapseButtonBarPositions();
    void AnimateCollapseButtonIconRotation(bool isCollapse);
    void UpdateCollapseTrianglePoints(const RefPtr<FrameNode>& triangleNode, float progress);
    void UpdateTitleBarContent();
    Dimension ComputeTitleBarRowHeight() const;

    bool IsCollapsed() const
    {
        return isCollapsed_;
    }

    bool IsBottomBarScrollEffectEnabled() const
    {
        return isBottomBarScrollEffectEnabled_;
    }

    bool ShouldAlignWithControlButtons() const;

private:
    void OnModifyDone() override;
    void CreateTabListIfNeeded(const RefPtr<FrameNode>& tabsNode);
    void UpdateTabListIfNeeded();
    void UpdateHeaderNodeIfNeeded();
    void UpdateFooterNodeIfNeeded();
    void UpdateBottomBarNodeIfNeeded();
    void UpdateSearchNodeIfNeeded();
    void OnSearchChange(const std::u16string& newText);
    void CreateHeaderContainerIfNeeded();
    void CreateBottomBarContainerIfNeeded();
    RefPtr<FrameNode> CreateSearchContainer();
    void CreateMaskNodeIfNeeded();
    RefPtr<FrameNode> CreateCollapseButtonNode();
    RefPtr<FrameNode> CreateCollapseButtonIcon();
    RefPtr<FrameNode> CreateCollapseIconButtonBar(
        const Color& color, float left, float top, float width, float height);
    bool IsSidebarOnEndSide() const;
    // Whether the collapse-button icon is mirrored horizontally (RTL layouts only;
    // independent of sidebarPosition — the triangle side is decided by the reading
    // direction, its pointing direction by the sidebar's on-screen side).
    bool IsIconMirroredByRtl() const;
    void UpdateCollapseTrianglePosition(const RefPtr<FrameNode>& triangleNode) const;
    void CreateTitleBarRowIfNeeded();
    RefPtr<FrameNode> CreateTitleBarIconNode();
    RefPtr<FrameNode> CreateTitleBarTextNode();
    void InitCollapseButtonClickEvent(const RefPtr<GestureEventHub>& gestureHub);
    void OnCollapseButtonClick();
    void CreateBottomBarMaskNodeIfNeeded();
    void OnTabListScroll(float totalOffset, float scrollableDistance);

    // Gradual blur effect
    void InitHeaderContainerScrollEffect(bool isScrollEffectEnabled);
    void UpdateHeaderContainerBlurStyle(float scrollScale);
    void InitBottomBarScrollEffect(bool isScrollEffectEnabled);
    void UpdateBottomBarBlurStyle(float scrollScale);
    void InitScrollEffectImpl(bool isScrollEffectEnabled, bool& isEnabledFlag, float& cachedScale,
        const RefPtr<FrameNode>& maskBlurNode, const RefPtr<FrameNode>& maskNode,
        GradientDirection direction);
    void UpdateBlurStyleImpl(float scrollScale, float& cachedScale, bool isEnabled,
        const RefPtr<FrameNode>& maskBlurNode, const RefPtr<FrameNode>& maskNode,
        GradientDirection direction);
    void UpdateBlurStyleImplInner(
        float scrollScale, const RefPtr<FrameNode>& maskBlurNode, const RefPtr<FrameNode>& maskNode,
        GradientDirection direction);
    RefPtr<FrameNode> CreateEffectNode(const std::string& tag);
    Color GetGradientMaskBlendColor();
    void OnColorConfigurationUpdate() override;

    WeakPtr<FrameNode> tabsNode_ = nullptr;
    RefPtr<UINode> headerNode_ = nullptr;
    RefPtr<UINode> footerNode_ = nullptr;
    RefPtr<UINode> bottomBarNode_ = nullptr;
    std::optional<TabsSidebarSearchableOptions> incommingOptions_;
    TabsSidebarSearchableOptions searchableOptions_;
    RefPtr<UINode> curHeaderNode_ = nullptr;
    RefPtr<UINode> curFooterNode_ = nullptr;
    RefPtr<UINode> curBottomBarNode_ = nullptr;
    RefPtr<FrameNode> searchContainerNode_ = nullptr;
    RefPtr<FrameNode> headerContainerNode_ = nullptr;
    RefPtr<FrameNode> bottomBarContainerNode_ = nullptr;
    RefPtr<FrameNode> footerContainerNode_ = nullptr;
    RefPtr<FrameNode> tabListNode_ = nullptr;
    RefPtr<SwiperController> swiperController_ = nullptr;

    // Gradual blur effect nodes & state
    RefPtr<FrameNode> headerContainerMaskBlurNode_ = nullptr;
    RefPtr<FrameNode> headerContainerMaskNode_ = nullptr;
    bool isScrollEffectEnabled_ = false;
    float scrollScale_ = -1.0f;

    RefPtr<FrameNode> collapseButtonNode_ = nullptr;
    RefPtr<FrameNode> collapseButtonIconNode_ = nullptr;
    RefPtr<FrameNode> collapseButtonTriangleNode_ = nullptr;
    RefPtr<FrameNode> titleBarRowNode_ = nullptr;
    RefPtr<FrameNode> titleBarIconNode_ = nullptr;
    RefPtr<FrameNode> titleBarTextNode_ = nullptr;
    TabsSidebarTitleBarOptions titleBarOptions_;
    bool isCollapsed_ = false;
    bool showControlButton_ = false;

    // BottomBar gradual blur effect nodes & state
    RefPtr<FrameNode> bottomBarMaskBlurNode_ = nullptr;
    RefPtr<FrameNode> bottomBarMaskNode_ = nullptr;
    bool isBottomBarScrollEffectEnabled_ = false;
    float bottomBarScrollScale_ = -1.0f;
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TABS_SIDE_BAR_PATTERN_H
