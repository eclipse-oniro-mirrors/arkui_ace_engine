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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TABS_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TABS_PATTERN_H

#include <optional>

#include "base/memory/referenced.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/blur_style_option.h"
#include "core/components_ng/manager/recoverable/recoverable_view.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/swiper/swiper_event_hub.h"
#include "core/components_ng/pattern/swiper/swiper_model.h"
#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"
#include "core/components_ng/pattern/tabs/tab_content_model.h"
#include "core/components_ng/pattern/tabs/tabs_layout_algorithm.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/event/input_event_hub.h"

namespace OHOS::Ace::NG {
namespace {
enum class FloatingBarPosition {
    CENTER = 0,
    LEFT,
    RIGHT,
};

constexpr float FLOATING_BAR_SCALE = 1.0f;
constexpr float FLOATING_BAR_SCALE_ENLARGED = 1.15f;
}

class TabsNode;
class TabContentNode;

class TabsPattern : public Pattern, public virtual RecoverableView {
    DECLARE_ACE_TYPE(TabsPattern, Pattern, RecoverableView);

public:
    TabsPattern() = default;
    ~TabsPattern() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }

    bool IsNeedPercent() const override
    {
        return true;
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        ACE_UINODE_TRACE(GetHost());
        return MakeRefPtr<TabsLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override;
    BarOverlapMode GetComputedBarOverlapMode() const { return computedBarOverlapMode_; }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::SCOPE, true };
    }

    ScopeFocusAlgorithm GetScopeFocusAlgorithm() override;

    void PerformanceCheckTabChange(
        RefPtr<OHOS::Ace::NG::TabsPattern> pattern, RefPtr<OHOS::Ace::NG::TabsNode> tabsNode, bool currentIndex);
    
    void SetOnChangeEvent(std::function<void(const BaseEventInfo*)>&& event);

    ChangeEventWithPreIndexPtr GetChangeEvent()
    {
        return onChangeEvent_;
    }

    void SetOnTabBarClickEvent(std::function<void(const BaseEventInfo*)>&& event);

    void SetAnimationStartEvent(AnimationStartEvent&& event);

    void SetAnimationEndEvent(AnimationEndEvent&& event);

    void SetOnSelectedEvent(std::function<void(const BaseEventInfo*)>&& event);

    void SetOnUnselectedEvent(std::function<void(const BaseEventInfo*)>&& event);

    void SetOnBarDisplayModeChangeEvent(std::function<void(TabBarDisplayMode)>&& event);

    void SetOnContentDidScroll(ContentDidScrollEvent&& onContentDidScroll);

    ChangeEventPtr GetTabBarClickEvent()
    {
        return onTabBarClickEvent_;
    }

    void OnModifyDone() override;

    std::string ProvideRestoreInfo() override;

    void OnRestoreInfo(const std::string& restoreInfo) override;

    void AddInnerOnGestureRecognizerJudgeBegin(GestureRecognizerJudgeFunc&& gestureRecognizerJudgeFunc) override;

    void RecoverInnerOnGestureRecognizerJudgeBegin() override;

    void SetOnIndexChangeEvent(std::function<void(const BaseEventInfo*)>&& event);

    ChangeEventPtr GetIndexChangeEvent()
    {
        return onIndexChangeEvent_;
    }

    bool GetIsCustomAnimation() const
    {
        return isCustomAnimation_;
    }

    void SetIsCustomAnimation(bool isCustomAnimation)
    {
        isCustomAnimation_ = isCustomAnimation;
    }

    void SetIsDisableSwipe(bool isDisableSwipe)
    {
        isDisableSwipe_ = isDisableSwipe;
    }
    bool GetIsRealDisableSwipe() const;

    void SetOnContentWillChange(std::function<bool(int32_t, int32_t)>&& callback)
    {
        callback_ = callback;
    }

    std::optional<bool> OnContentWillChange(int32_t preIndex, int32_t index) const
    {
        std::optional<bool> ret;
        if (callback_) {
            ret = callback_(preIndex, index);
        }
        return ret;
    }

    void SetInterceptStatus(bool status)
    {
        interceptStatus_ = status;
    }

    bool GetInterceptStatus() const
    {
        return interceptStatus_;
    }

    void SetAnimateMode(TabAnimateMode mode);

    TabAnimateMode GetAnimateMode() const
    {
        return animateMode_;
    }

    std::optional<TabBarDisplayMode> GetCurrentBarDisplayMode() const
    {
        return currentBarDisplayMode_;
    }
    TabBarDisplayMode GetActiveBarDisplayMode() const;
    void SetCurrentBarDisplayMode(TabBarDisplayMode mode);

    void MarkUserSetSafeAreaOpts()
    {
        isUserSetSafeAreaOpts_ = true;
    }

    void FireBarDisplayModeChangeEvent(TabBarDisplayMode mode);

    void StartDisplayModeSwitchAnimation(TabBarDisplayMode fromMode, TabBarDisplayMode toMode);

    void HandleChildrenUpdated(const RefPtr<FrameNode>& swiperNode, const RefPtr<FrameNode>& tabBarNode);

    void UpdateSelectedState(const RefPtr<FrameNode>& swiperNode, const RefPtr<TabBarPattern>& tabBarPattern,
        const RefPtr<TabsLayoutProperty>& tabsLayoutProperty, int index);

    void UpdateDividerStrokeWidth()
    {
        OnUpdateShowDivider();
    }
    void UpdateDividerColor()
    {
        OnUpdateShowDivider();
    }
    void UpdateSidebarDividerStrokeWidth()
    {
        UpdateSideBarDivider();
    }
    int32_t OnInjectionEvent(const std::string& command) override;
    CommandResult OnInjectionEventWithResult(const std::string& command) override;
    void OnColorConfigurationUpdate() override;
    void OnColorModeChange(uint32_t colorMode) override;
    bool OnThemeScopeUpdate(int32_t themeScopeId) override;
    void DumpInfo() override;

    void OnWindowSizeChanged(int32_t  /*width*/, int32_t  /*height*/, WindowSizeChangeReason type) override;
    void OnAttachToMainTree() override;
    bool OnSaveData(std::string& data) override;
    void OnDetachFromMainTree() override;
    void ResetTabBarFollowHandPosition();
    void SetFloatingBarMargin(std::optional<float> floatingBarMargin)
    {
        floatingBarMargin_ = floatingBarMargin;
    }
    void SetFloatingTopBarMargin(std::optional<float> floatingTopBarMargin)
    {
        floatingTopBarMargin_ = floatingTopBarMargin;
    }
    bool IsFloatingBar() const
    {
        return isFloatingBar_;
    }
    bool LastFloatingBar() const
    {
        return lastFloatingBar_;
    }
    bool IsFloatingTopBar() const
    {
        return isFloatingTopBar_;
    }
    bool LastFloatingTopBar() const
    {
        return lastFloatingTopBar_;
    }
    void SetFloatingScaleEnabled(bool isFloatingScaleEnabled);

    float GetBaseFloatingScale() const
    {
        return baseFloatingScale_;
    }

    // Whether the bottom tab bar (floating tab bar or normal bottom tab bar) should
    // be shown after the sidebar is collapsed.
    bool IsShowBottomBarOnSidebarCollapseEnabled() const;
    // Whether the tab bar is a bottom-style bar able to join the sidebar
    // collapse/expand switch: a floating bar, or a normal bar actually placed
    // at the bottom (not vertical, barPosition == END). Other placements
    // (vertical bar / top bar) never animate in the switch.
    bool IsTabBarBottomStyle() const;
    // Whether the bottom tab bar actually joins (this phase of) the sidebar
    // collapse/expand switch — the switch flips the display mode and fires the
    // developer change event ONLY when it does. See the .cpp for the two
    // direction-dependent conditions.
    bool WillBottomBarJoinCollapseSwitch(bool toCollapsed) const;
    // Whether the tab bar node is currently laid out visible (not GONE). The
    // expand-direction hide is an invariant keyed on this live state, not on
    // the shown-after-collapse bookkeeping.
    bool IsTabBarNodeVisible() const;
    // Whether the effective divider is explicitly null (hidden via divider-node
    // opacity 0 by the model layer). Display-mode switch animations must not
    // restore opacity to 1.0 for a null divider.
    bool IsTabBarDividerNull() const;
    // The three frame nodes that move together whenever the tab bar enters or
    // exits a bar display arrangement: the bar itself, its divider and the
    // floating background mask (when present). Nodes that do not exist (yet)
    // are left null — write sites gate on the fields they need.
    struct TabBarTransitionNodes {
        RefPtr<FrameNode> tabBar;
        RefPtr<FrameNode> tabBarDivider;
        RefPtr<FrameNode> bgMaskNode;
    };
    TabBarTransitionNodes GetTabBarTransitionNodes() const;
    // Whether keepTabBarInSideBar keeps the tab bar visible in the sidebar
    // arrangement (the bar then never joins mode-switch hides).
    bool IsKeepTabBarInSideBarMode() const;
    // Set one opacity on the transition nodes: tab bar, divider, and — only
    // for the floating bar families, which animate the mask — the background
    // mask. Restoring visibility (non-zero opacity) must skip a divider
    // explicitly set to null (the model layer hides it via opacity 0);
    // hiding (zero opacity) always applies.
    void UpdateTabBarTransitionOpacity(
        const TabBarTransitionNodes& nodes, float opacity, bool withBackgroundMask);
    // Two-pass sidebar collapse/expand switch: the click only registers the
    // request and marks the tabs dirty. The next layout pass (pass 1) keeps
    // the "from" arrangement but additionally measures/layouts the entering
    // components so they have valid geometry; an after-layout task then
    // applies the initial transforms and starts the animation whose closure
    // re-enters layout (pass 2) in the "to" arrangement (re-entrant
    // FlushUITasks) and sets the sidebar translate target — plus the bottom
    // bar transform/opacity targets only when showBottomBarOnSidebarCollapse
    // lets the bar join the switch. No-op while another switch is
    // pending/running.
    void RequestSidebarCollapseSwitchAnimation(bool toCollapsed);
    // Phase of the two-pass sidebar collapse/expand switch state machine.
    enum class SidebarCollapseSwitchState {
        NONE = 0,
        COLLAPSE_REQUESTED,
        COLLAPSE_ANIMATING,
        EXPAND_REQUESTED,
        EXPAND_ANIMATING,
    };
    bool IsSidebarCollapseSwitchBusy() const
    {
        return sidebarCollapseSwitchState_ != SidebarCollapseSwitchState::NONE;
    }
    bool IsSidebarCollapseSwitchRequested() const
    {
        return sidebarCollapseSwitchState_ == SidebarCollapseSwitchState::COLLAPSE_REQUESTED ||
               sidebarCollapseSwitchState_ == SidebarCollapseSwitchState::EXPAND_REQUESTED;
    }
    bool IsSidebarCollapseSwitchToCollapsed() const
    {
        return sidebarCollapseSwitchState_ == SidebarCollapseSwitchState::COLLAPSE_REQUESTED ||
               sidebarCollapseSwitchState_ == SidebarCollapseSwitchState::COLLAPSE_ANIMATING;
    }
    bool IsOnSidebarCollapseSwitchAnimation() const
    {
        return sidebarCollapseSwitchState_ == SidebarCollapseSwitchState::COLLAPSE_ANIMATING ||
               sidebarCollapseSwitchState_ == SidebarCollapseSwitchState::EXPAND_ANIMATING;
    }
    // Whether the steady layout is in the "sidebar collapsed with bottom bar
    // shown" arrangement (content shrinks by the bar height, sidebar GONE).
    bool IsBottomBarShownAfterCollapse() const
    {
        return bottomBarShownAfterCollapse_;
    }
    // Whether the in-flight collapse/expand switch actually has the bottom bar
    // participating. Frozen at switch start (StartSidebarCollapseSwitchAnimation):
    // a mid-animation flip of showBottomBarOnSidebarCollapse must not change the
    // running switch — it takes effect on the next switch. The animation-phase
    // masking in the layout algorithm reads this instead of re-evaluating the
    // live property, so pass-1/pass-2/finish stay consistent with the started
    // animation.
    bool IsSidebarCollapseSwitchWithBottomBar() const
    {
        return sidebarCollapseSwitchWithBottomBar_;
    }
    // Invoked from the after-layout task of the pass-1 layout.
    void StartSidebarCollapseSwitchAnimation();

    bool IsTabBarFollowingHand() const
    {
        return floatingBarPosition_ != FloatingBarPosition::CENTER || floatTabBarFollowHandAnimationCount_ > 0;
    }

    void SetSidebarHeaderNode(const RefPtr<NG::UINode>& header)
    {
        sidebarHeaderNode_ = header;
    }
    RefPtr<NG::UINode> GetSidebarHeaderNode() const
    {
        return sidebarHeaderNode_;
    }

    void SetSidebarFooterNode(const RefPtr<NG::UINode>& footer)
    {
        sidebarFooterNode_ = footer;
    }
    const RefPtr<NG::UINode>& GetSidebarFooterNode() const
    {
        return sidebarFooterNode_;
    }

    void SetSidebarBottomBarNode(const RefPtr<NG::UINode>& bottomBar)
    {
        sidebarBottomBarNode_ = bottomBar;
    }
    const RefPtr<NG::UINode>& GetSidebarBottomBarNode() const
    {
        return sidebarBottomBarNode_;
    }

    TabBarDisplayMode CalculateTabBarDisplayMode(float width);

    RefPtr<FrameNode> GetSideBarNode() const
    {
        return sideBarNode_;
    }

    RefPtr<FrameNode> GetSideBarDividerNode() const
    {
        return sideBarDividerNode_;
    }

    RefPtr<FrameNode> GetCollapseButtonNode() const
    {
        return collapseButtonNode_;
    }

    void SetTabsSidebarSearchableOptions(const TabsSidebarSearchableOptions& options)
    {
        searchableOptions_ = options;
    }

    void SetShowSidebarControlButton(bool showControlButton)
    {
        if (showSidebarControlButton_ == showControlButton) {
            return;
        }
        showSidebarControlButton_ = showControlButton;
        if (sideBarNode_) {
            SyncPropertiesToSideBar();
            UpdateCollapseButtonIfNeeded();
        }
    }

    bool GetShowSidebarControlButton() const
    {
        return showSidebarControlButton_;
    }

    void UpdateCollapseButtonIfNeeded();

    void UpdateCollapseButtonForDisplayMode(TabBarDisplayMode mode);

    void SetSidebarTitleBar(const TabsSidebarTitleBarOptions& options)
    {
        sidebarTitleBarOptions_ = options;
    }

    bool HasSidebarTitleBar() const
    {
        return !sidebarTitleBarOptions_.isNull;
    }

    const TabsSidebarTitleBarOptions& GetSidebarTitleBar() const
    {
        return sidebarTitleBarOptions_;
    }

    void UpdateSidebarTitleBarIcon(const std::string& icon)
    {
        sidebarTitleBarOptions_.icon = icon;
    }
    void UpdateSidebarTitleBarText(const std::string& text)
    {
        sidebarTitleBarOptions_.text = text;
    }

    const TabsSidebarSearchableOptions& GetTabsSidebarSearchableOptions() const
    {
        return searchableOptions_;
    }

    void SetBarModifierApply(std::function<void(WeakPtr<NG::FrameNode>)>&& onApply)
    {
        barModifierApply_ = std::move(onApply);
    }

    std::function<void(WeakPtr<NG::FrameNode>)> GetBarModifierApply() const
    {
        return barModifierApply_;
    }

    void SetBarBlurStyleOption(const BlurStyleOption& option)
    {
        hasBarBlurStyle_ = true;
        barBlurStyleOption_ = option;
    }
    const BlurStyleOption& GetBarBlurStyleOption() const
    {
        return barBlurStyleOption_;
    }
    void SetBarBlurStyleOptionForSideBar(const BlurStyleOption& option)
    {
        hasBarBlurStyleForSideBar_ = true;
        barBlurStyleOptionForSideBar_ = option;
    }
    const BlurStyleOption& GetBarBlurStyleOptionForSideBar() const
    {
        return barBlurStyleOptionForSideBar_;
    }
    void SetSidebarBlurStyleOptions(const BlurStyleOption& option)
    {
        hasSidebarBlurStyle_ = true;
        sidebarBlurStyleOptions_ = option;
    }
    const BlurStyleOption& GetSidebarBlurStyleOption() const
    {
        return sidebarBlurStyleOptions_;
    }
    const BlurStyleOption& GetEffectiveSidebarBlurStyleOptions() const
    {
        if (hasSidebarBlurStyle_) {
            return sidebarBlurStyleOptions_;
        }
        if (hasBarBlurStyleForSideBar_) {
            return barBlurStyleOptionForSideBar_;
        }
        return barBlurStyleOption_;
    }

    void AddTabContentNode(const RefPtr<TabContentNode>& tabContentNode);
    void RemoveTabContentNode(const RefPtr<TabContentNode>& tabContentNode);
    bool IsColorInvertEnabled();
    ColorMode GetColorInvertColorMode();
    ColorMode GetBarMaskColorMode();

    const std::optional<float>& GetRealSideBarWidthPx() const
    {
        return realSideBarWidthPx_;
    }

    void SetRealSideBarWidthPx(float width)
    {
        realSideBarWidthPx_ = width;
    }

    void ClearRealSideBarWidthPx()
    {
        realSideBarWidthPx_.reset();
    }

    float GetPreSideBarWidthPx() const
    {
        return preSideBarWidthPx_;
    }

    void SetPreSideBarWidthPx(float width)
    {
        preSideBarWidthPx_ = width;
    }

    std::optional<Dimension> GetEffectiveSidebarWidth() const;

    void SetSidebarWidthCalled()
    {
        hasSidebarWidth_ = true;
    }

    // Sync bar* attributes to the sidebar (no-op when sidebar does not exist).
    void UpdateSideBarDivider();
    void UpdateSidebarDividerColor();
    void UpdateSideBarBackgroundColor();
    void UpdateSideBarBackgroundBlurStyle();

    struct SideBarDragRange {
        float minPx = 0.0f;
        float maxPx = 0.0f;
        bool draggable = false;
        float initialWidthPx = 0.0f;
    };
    SideBarDragRange CalcSideBarDragRange(float tabsWidth);
    void ClampSideBarWidthToRange();

    bool IsOnDisplayModeSwitchAnimation() const
    {
        return isOnDisplayModeSwitchAnimation_;
    }

    void SetTabBarClickIntercept(bool active)
    {
        tabBarClickIntercept_ = active;
    }

    bool GetTabBarClickIntercept() const
    {
        return tabBarClickIntercept_;
    }

    // Centralized sidebar tab view customization.
    // SetTabViewCustomization stores the customization from ModelNG without
    // applying it. The actual diff check against appliedTabViewCustomization_
    // and application happen in OnModifyDone, where all properties are resolved.
    void SetTabViewCustomization(const TabViewCustomization& customization)
    {
        tabViewCustomization_ = customization;
    }

    void ResetTabViewCustomization()
    {
        tabViewCustomization_.reset();
    }

    const std::optional<TabViewCustomization>& GetAppliedTabViewCustomization() const
    {
        return appliedTabViewCustomization_;
    }

    // Refresh the sidebar tab list with the current customization.
    // Called from TabContentPattern::OnModifyDone when a single tab's
    // visibility/section property changes and the sidebar is active.
    void RefreshSideBarTabView();

    // Diff tabViewCustomization_ (incoming from ModelNG) against
    // appliedTabViewCustomization_ (currently applied). If changed (or forceApply
    // is true), sync appliedTabViewCustomization_, re-apply to the sidebar tab
    // list, and mark the host dirty for layout.
    // forceApply=true: always apply regardless of diff (e.g. sidebar becoming
    // visible, entering sidebar mode, property-change full rebuild).
    // forceApply=false: only apply if customization actually changed.
    void ApplyTabViewCustomizationIfNeeded(bool forceApply = false);

    // Register/Remove a ResObj callback for customization sectionMap
    // resource-backed section titles. Called from OnModifyDone after the
    // customization has been applied.
    //   - any section has valid resourceTitleObject → register a single
    //     ResObj callback (with an empty ResourceObject as notification trigger)
    //     that re-parses all section titles and refreshes the sidebar tab list.
    //   - no resourceTitleObject → remove the callback to avoid stale triggers.
    void RegisterCustomizationSectionResObj();

private:
    void OnAttachToFrameNode() override;
    void OnAfterModifyDone() override;
    void OnUpdateShowDivider();
    WeakPtr<FocusHub> GetNextFocusNode(FocusStep step, const WeakPtr<FocusHub>& currentFocusNode);
    // Sidebar display mode: the TabBar is hidden (GONE) and the sidebar tab list takes its place
    // in the Tabs focus chain. `handled` is set to true when the sidebar tab list focus hub exists,
    // meaning the caller must NOT fall back to the legacy TabBar logic.
    WeakPtr<FocusHub> GetNextFocusNodeInSideBarMode(FocusStep step, const RefPtr<FocusHub>& curFocusNode,
        const RefPtr<FocusHub>& swiperFocusNode, const RefPtr<TabsLayoutProperty>& property, bool& handled);
    void BeforeCreateLayoutWrapper() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    std::string GetTabBarTextByIndex(int32_t index) const;
    void UpdateSwiperDisableSwipe(bool disableSwipe);
    void SetSwiperPaddingAndBorder();
    void RecordChangeEvent(int32_t index);
    void FireTabContentStateCallback(int32_t oldIndex, int32_t nextIndex) const;
    void FireTabChangeCallback(int32_t preIndex, int32_t nextIndex);
    void UpdateBackBlurStyle(const RefPtr<FrameNode>& tabBarNode);
    bool GetTargetIndex(const std::string& command, int32_t& targetIndex);
    // Information on TabChange event
    struct TabChangeInfo {
        int32_t index = 0;
        bool  isShow = false;
        std::optional<int32_t> lastFocusIndex;
    };
    static bool IsValidFireTabChange(const std::optional<TabChangeInfo>& lastTabChangeInfo,
        int32_t index, bool isShow);
    static bool IsNeedFireTabChange(bool isInit,
        int32_t targetIndex, int32_t currentIndex, int32_t preIndex);
    void HandleTabChangeWhenChildrenUpdated(bool isInit, int32_t tabContentNum, int32_t targetIndex);
    void UpdateIndex(const RefPtr<FrameNode>& tabsNode, const RefPtr<FrameNode>& tabBarNode,
        const RefPtr<FrameNode>& swiperNode, const RefPtr<TabsLayoutProperty>& tabsLayoutProperty);
    void InitFocusEvent();
    RefPtr<FocusHub> GetCurrentFocusNode(FocusIntension intension);
    // Sidebar display mode: entry focus hub selection between the sidebar tab list and the Swiper.
    RefPtr<FocusHub> GetCurrentFocusNodeInSideBarMode(FocusIntension intension,
        const RefPtr<FocusHub>& sideBarFocusHub, const RefPtr<FocusHub>& swiperFocusHub,
        const RefPtr<TabsLayoutProperty>& property);
    void InitAccessibilityZIndex();

    void InitFloatingBar();
    void ApplyStrategyBarOverlapBlurStyle();

    void UpdateBgMaskNode();
    void UpdateTopBgMaskNode();
    GradientColor CreatePercentGradientColor(float percent, Color color);
    void InitTouchEvent();
    void OnTouchEvent(const TouchEventInfo& info);
    void FollowHandAnimation();
    void InitTabBarTransformAttributeIfNeeded();
    void HandleOnTouchScaleAnimation();
    void HandleOnTouchDelayScaleAnimation();
    void OnFollowHandAnimationFinish();
    void ApplySystemMaterial();
    void ResetSystemMaterial();
    void InitColorPickerIfNeeded();
    void UnregisterColorPicker();
    void OnLuminanceUpdate(uint32_t luminance);
    void StartColorInvertAnimation();
    void HandleColorInvert();
    void SetTabBarIsFloating(bool isFloating)
    {
        auto tabsNode = AceType::DynamicCast<TabsNode>(GetHost());
        CHECK_NULL_VOID(tabsNode);
        auto tabBar = AceType::DynamicCast<FrameNode>(tabsNode->GetTabBar());
        if (tabBar) {
            auto tabBarPattern = tabBar->GetPattern<TabBarPattern>();
            if (tabBarPattern) {
                tabBarPattern->SetIsFloatingBar(isFloating);
            }
        }
    }
    void SetTabBarIsFloatingTopBar(bool isFloating)
    {
        auto tabsNode = AceType::DynamicCast<TabsNode>(GetHost());
        CHECK_NULL_VOID(tabsNode);
        auto tabBar = AceType::DynamicCast<FrameNode>(tabsNode->GetTabBar());
        if (tabBar) {
            auto tabBarPattern = tabBar->GetPattern<TabBarPattern>();
            if (tabBarPattern) {
                tabBarPattern->SetIsFloatingTopBar(isFloating);
            }
        }
    }

    void UpdateSideBarIfNeeded();
    void UpdateSideBarNode();
    void UpdateSideBarAttributes();
    void SyncPropertiesToSideBar();
    void RegisterSideBarTabItems(bool update);
    void SyncSideBarTabListIndicator(int32_t currentIndex);
    RefPtr<FrameNode> CreateSideBarNode();
    RefPtr<FrameNode> CreateSideBarDividerNode();

    // Divider drag
    void InitDividerDragEvent();
    void ClearDividerDragEvent();
    void InitDividerPanEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleDividerDragStart();
    void HandleDividerDragUpdate(float xOffset);
    void HandleDividerDragEnd();
    void HandleDividerDragCancel();
    void InitDividerMouseEvent(const RefPtr<InputEventHub>& inputHub);
    void OnDividerHover(bool isHover);
    void OnDividerMouseEvent(MouseInfo& info);
    void SetMouseStyle(MouseFormat format);
    void AddDividerHotZoneRect();
    bool IsDividerDraggable() const;

    TabsItemDivider GetEffectiveSidebarDividerConfig() const;
    float GetEffectiveSidebarDividerWidthPx() const;
    std::optional<Color> GetEffectiveSidebarBackgroundColor() const;

    void OnDisplayModeSwitchAnimationsFinished();
    void SetElementVisibilityForMode(TabBarDisplayMode targetMode);
    // Compute the horizontal translate offset that places sidebar+divider just outside the
    // tabs container (used as the sidebar enter/exit visual start position).
    float CalcSidebarOutsideTranslateX() const;
    // Apply initial transform state (translate for sidebar/divider, scale/opacity for tabbar)
    // to entering elements without animation. Used on first-time animation start so that the
    // entering element renders outside the container before the animation closure runs.
    void ApplyEnteringElementInitialTransform(TabBarDisplayMode toMode);
    void ApplyEnteringSidebarInitialTransform(float outsideX);
    void ApplyEnteringBottomTabBarInitialTransform(const RefPtr<FrameNode>& tabBar,
        const RefPtr<FrameNode>& tabBarDivider);
    // Reset all transform/opacity previously applied by the mode-switch animation.
    void ResetDisplayModeSwitchTransforms();
    void StartNonFloatingTabBarAnimation(bool toSidebar, float outsideX, float tabHeight,
        const std::function<void()>& finishCb);
    void ApplyNonFloatingTabBarAnimClosure(bool toSidebar, float outsideX, float tabHeight);
    void StartFloatingTabBarAnimation(bool toSidebar, float outsideX, float baseScale,
        const std::function<void()>& finishCb);
    void StartFloatingSidebarTranslateAnim(
        bool toSidebar, float outsideX, const std::function<void()>& finishCb);
    void StartFloatingTabBarExitAnim(float baseScale, const std::function<void()>& finishCb);
    void StartFloatingTabBarEnterAnim(float baseScale, const std::function<void()>& finishCb);
    // Bottom tab bar show/hide on sidebar collapse/expand (two-pass switch):
    // per-variant transform targets. Floating tab bar scales+fades; normal
    // bottom tab bar slides (translate = ±tabBarHeight+dividerHeight)+fades.
    void ApplyFloatingTabBarShownTargets();
    void ApplyFloatingTabBarHiddenTargets();
    void ApplyNonFloatingTabBarShownTargets();
    void ApplyNonFloatingTabBarHiddenTargets(float tabHeight);
    // Dispatch to the variant targets above; keepMode (keepTabBarInSideBar)
    // keeps the tab bar managed by the sidebar mode and is skipped.
    void ApplyBottomBarShownTargets();
    void ApplyBottomBarHiddenTargets(float tabHeight);
    // Instantly (non-animated) hide a still-visible tab bar at sidebar-expand
    // animation start when the bar does not join the switch animation
    // (non-bottom placements never animate): restores the SIDEBAR
    // arrangement's hidden invariant. keepMode keeps the bar visible and is
    // skipped.
    void HideTabBarForSidebarRestore();
    // Set translate on sidebar + sidebar divider (shared by both directions).
    void ApplySideBarTranslate(float translateX);
    // Animation frames of the two-pass collapse/expand switch, invoked from the
    // StartAnimation closures (lambdas stay thin).
    // Main (228/30) frame: pass-2 re-entrant layout in the "to" arrangement +
    // sidebar translate (+ normal bottom bar translate/opacity targets).
    // tabHeight: bar height captured from the pass-1 geometry before the
    // animation starts (expand-direction exit target); withBottomBar: whether
    // the bottom tab bar actually joined this switch (captured at start).
    void RunSidebarCollapseSwitchMainFrame(bool toCollapsed, float tabHeight, bool withBottomBar);
    // Floating bar (228/20) frame: bottom tab bar scale+opacity targets only.
    void RunSidebarCollapseSwitchFloatingBarFrame(bool toCollapsed, float tabHeight);
    // Finish bookkeeping of the two-pass collapse/expand switch animation:
    // flip the shown-after-collapse arrangement flag, set the leaving element
    // GONE, reset transient transforms and clear the animation flags.
    // withBottomBar: whether the bottom tab bar actually participated in this
    // switch (collapse: showBottomBarOnSidebarCollapse read at animation start;
    // expand: it was shown by the collapse switch).
    void OnSidebarCollapseSwitchAnimationFinished(bool toCollapsed, bool withBottomBar);

    bool isCustomAnimation_ = false;
    bool isDisableSwipe_ = false;
    bool isInit_ = true;

    TabAnimateMode animateMode_ = TabAnimateMode::CONTENT_FIRST;
    ChangeEventWithPreIndexPtr onChangeEvent_;
    std::shared_ptr<ContentDidScrollEvent> onContentDidScroll_;
    ChangeEventPtr selectedEvent_;
    ChangeEventPtr unselectedEvent_;
    ChangeEventPtr onTabBarClickEvent_;
    ChangeEventPtr onIndexChangeEvent_;
    AnimationStartEventPtr animationStartEvent_;
    AnimationEndEventPtr animationEndEvent_;
    std::function<void(TabBarDisplayMode)> onBarDisplayModeChangeEvent_;
    std::function<bool(int32_t, int32_t)> callback_;
    bool interceptStatus_ = false;
    BarPosition barPosition_ = BarPosition::END; // default accessibilityZIndex is consistent with BarPosition::END
    std::optional<TabChangeInfo> lastTabChangeInfo_;

    bool isFloatingBar_ = false;
    bool lastFloatingBar_ = false;
    FloatingBarPosition floatingBarPosition_ = FloatingBarPosition::CENTER;
    RefPtr<TouchEventImpl> touchListener_ = nullptr;
    int32_t floatTabBarFollowHandAnimationCount_ = 0;
    std::list<std::shared_ptr<AnimationUtils::Animation>> floatTabBarFollowHandAnimations_;
    std::optional<float> floatingBarMargin_ = 0.0f;
    float baseFloatingScale_ = FLOATING_BAR_SCALE;

    bool isFloatingTopBar_ = false;
    bool lastFloatingTopBar_ = false;
    std::optional<float> floatingTopBarMargin_ = 0.0f;

    std::optional<TabBarDisplayMode> currentBarDisplayMode_;
    std::optional<TabBarLayoutStyle> layoutStyle_;
    RefPtr<FrameNode> sideBarNode_ = nullptr;
    RefPtr<FrameNode> sideBarDividerNode_ = nullptr;
    RefPtr<FrameNode> collapseButtonNode_ = nullptr;
    RefPtr<NG::UINode> sidebarHeaderNode_;
    RefPtr<NG::UINode> sidebarFooterNode_;
    RefPtr<NG::UINode> sidebarBottomBarNode_;
    TabsSidebarSearchableOptions searchableOptions_;
    TabsSidebarTitleBarOptions sidebarTitleBarOptions_;
    bool showSidebarControlButton_ = false;
    std::function<void(WeakPtr<NG::FrameNode>)> barModifierApply_;
    bool hasBarBlurStyle_ = false;
    BlurStyleOption barBlurStyleOption_;
    bool hasBarBlurStyleForSideBar_ = false;
    BlurStyleOption barBlurStyleOptionForSideBar_;
    bool hasSidebarBlurStyle_ = false;
    BlurStyleOption sidebarBlurStyleOptions_;
    bool hasSidebarWidth_ = false;
    std::vector<WeakPtr<TabContentNode>> tabContentNodes_;
    // Color invert state for auto-inversion
    std::optional<bool> isColorPickerDark_;
    bool hasRegisterColorPicker_ = false;

    // Divider drag state
    RefPtr<PanEvent> dividerPanEvent_;
    RefPtr<InputEvent> hoverEvent_;
    RefPtr<InputEvent> dividerMouseEvent_;
    bool isInDividerDrag_ = false;
    std::optional<float> realSideBarWidthPx_;
    float preSideBarWidthPx_ = 0.0f;
    float minSideBarWidth_ = -1.0f;
    float maxSideBarWidth_ = -1.0f;
    bool strategyEnabled_ = false;
    BarOverlapMode computedBarOverlapMode_ = BarOverlapMode::STANDARD;

    // Display mode switch animation state.
    // sidebar translate + tabbar scale/opacity run as independent animations; a counter
    // tracks in-flight animations so the final state is applied only after all of them
    // finish. Interrupting switches do not stop in-flight animations — they start new ones
    // and let old ones finish naturally; the last finish callback sets the steady state.
    bool isOnDisplayModeSwitchAnimation_ = false;
    int32_t displayModeAnimCount_ = 0;
    std::optional<TabBarDisplayMode> displayModeAnimTarget_;
    // Two-pass collapse/expand switch (showBottomBarOnSidebarCollapse):
    // sidebarCollapseSwitchState_ = single state machine covering the
    // requested (waiting for the pass-1 layout) and animating (pass-2
    // re-entrant) phases, in both collapse and expand directions;
    // bottomBarShownAfterCollapse_ = steady arrangement flag.
    SidebarCollapseSwitchState sidebarCollapseSwitchState_ = SidebarCollapseSwitchState::NONE;
    bool bottomBarShownAfterCollapse_ = false;
    // Frozen "bar participates" decision of the in-flight switch (see
    // IsSidebarCollapseSwitchWithBottomBar); reset together with the state.
    bool sidebarCollapseSwitchWithBottomBar_ = false;
    std::shared_ptr<AnimationUtils::Animation> sidebarTranslateAnim_;
    std::shared_ptr<AnimationUtils::Animation> tabBarEnterExitAnim_;
    bool tabBarClickIntercept_ = false;

    bool isUserSetSafeAreaOpts_ = false;

    // Centralized customization for the sidebar tab bar.
    // tabViewCustomization_: the latest customization set from ModelNG, not yet applied.
    // appliedTabViewCustomization_: the currently-applied customization.
    std::optional<TabViewCustomization> tabViewCustomization_;
    std::optional<TabViewCustomization> appliedTabViewCustomization_;
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_TABS_TABS_PATTERN_H
