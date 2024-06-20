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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_MENU_MENU_ITEM_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_MENU_MENU_ITEM_PATTERN_H

#include "base/memory/referenced.h"
#include "base/utils/noncopyable.h"
#include "core/components/slider/render_slider.h"
#include "core/components_ng/event/long_press_event.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_accessibility_property.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_event_hub.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_layout_algorithm.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_layout_property.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_paint_method.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_paint_property.h"
#include "core/components_ng/pattern/menu/menu_pattern.h"
#include "core/components_ng/pattern/pattern.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT MenuItemPattern : public Pattern {
    DECLARE_ACE_TYPE(MenuItemPattern, Pattern);

public:
    MenuItemPattern() = default;
    ~MenuItemPattern() override = default;

    bool IsAtomicNode() const override
    {
        return false;
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, true, FocusStyleType::INNER_BORDER };
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<MenuItemEventHub>();
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<MenuItemLayoutProperty>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<MenuItemAccessibilityProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<MenuItemLayoutAlgorithm>();
    }

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        return MakeRefPtr<MenuItemPaintMethod>();
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<MenuItemPaintProperty>();
    }

    void MarkIsSelected(bool isSelected);
    void SetSelected(bool isSelected)
    {
        isSelected_ = isSelected;
        GetHost()->MarkModifyDone();
    }

    bool IsSelected() const
    {
        return isSelected_;
    }

    void SetSubBuilder(const std::function<void()>& subBuilderFunc)
    {
        subBuilderFunc_ = subBuilderFunc;
    }

    std::function<void()>& GetSubBuilder()
    {
        return subBuilderFunc_;
    }

    bool IsSubMenuShowed() const
    {
        return isSubMenuShowed_;
    }

    void SetIsSubMenuShowed(bool isSubMenuShowed)
    {
        isSubMenuShowed_ = isSubMenuShowed;
    }

    bool IsSubMenuHovered() const
    {
        return isSubMenuHovered_;
    }

    void SetIsSubMenuHovered(bool isSubMenuHovered)
    {
        isSubMenuHovered_ = isSubMenuHovered;
    }

    void AddHoverRegions(const OffsetF& topLeftPoint, const OffsetF& bottomRightPoint);

    bool IsInHoverRegions(double x, double y);

    void ClearHoverRegions()
    {
        hoverRegions_.clear();
    }

    void ResetWrapperMouseEvent()
    {
        wrapperMouseEvent_.Reset();
    }

    void SetChange()
    {
        isSelected_ = !isSelected_;
    }

    bool IsChange() const
    {
        return isChanged_;
    }

    void CloseMenu();

    RefPtr<FrameNode> GetContentNode()
    {
        return content_;
    }

    RefPtr<FrameNode> GetLabelNode()
    {
        return label_;
    }

    void PlayBgColorAnimation(bool isHoverChange = true);
    void SetBgBlendColor(const Color& color)
    {
        bgBlendColor_ = color;
    }
    Color GetBgBlendColor() const
    {
        return bgBlendColor_;
    }
    bool IsDisabled();

    RefPtr<FrameNode> GetMenu(bool needTopMenu = false);
    RefPtr<MenuPattern> GetMenuPattern(bool needTopMenu = false);
    void UpdateTextNodes();

    void OnAttachToFrameNode() override;
    void OnModifyDone() override;
    void OnMountToParentDone() override;

    bool HasSelectIcon() const
    {
        return selectIcon_ != nullptr;
    }
    bool HasStartIcon() const
    {
        return startIcon_ != nullptr;
    }

    void SetClickMenuItemId(int32_t id)
    {
        clickMenuItemId_ = id;
    }

    int32_t GetClickMenuItemId() const
    {
        return clickMenuItemId_;
    }

    void OnVisibleChange(bool isVisible) override;
    void InitLongPressEvent();
    void UpdateNeedDivider(bool need);
    void SetIndex(int32_t index)
    {
        index_ = index;
    }
    float GetDividerStroke();

    SubMenuExpandingMode GetExpandingMode()
    {
        return expandingMode_;
    }
    bool IsSubMenu();
    void SetIsStackSubmenuHeader()
    {
        isStackSubmenuHeader_ = true;
    }
    bool IsStackSubmenuHeader() {
        return isStackSubmenuHeader_;
    }

protected:
    void RegisterOnKeyEvent();
    void RegisterOnTouch();
    void OnAfterModifyDone() override;
    RefPtr<FrameNode> GetMenuWrapper();

private:
    // register menu item's callback
    void RegisterOnClick();
    void RegisterOnHover();
    virtual void OnTouch(const TouchEventInfo& info);
    void OnHover(bool isHover);
    virtual bool OnKeyEvent(const KeyEvent& event);

    void RegisterWrapperMouseEvent();

    void AddSelectIcon(RefPtr<FrameNode>& row);
    void UpdateIcon(RefPtr<FrameNode>& row, bool isStart);
    void AddExpandIcon(RefPtr<FrameNode>& row);
    void UpdateText(RefPtr<FrameNode>& row, RefPtr<MenuLayoutProperty>& menuProperty, bool isLabel);
    void UpdateTexOverflow(RefPtr<TextLayoutProperty>& textProperty);
    bool IsTextFadeOut();
    void UpdateFont(RefPtr<MenuLayoutProperty>& menuProperty, RefPtr<SelectTheme>& theme, bool isLabel);
    void UpdateExpandableArea();
    void BuildEmbeddedMenuItems(RefPtr<UINode>& node, bool needNextLevel = true);
    void AddStackSubMenuHeader(RefPtr<FrameNode>& menuNode);
    RefPtr<FrameNode> GetClickableArea();
    void ShowEmbeddedSubMenu(bool hasFurtherExpand);

    void UpdateDisabledStyle();

    void ShowSubMenu();
    void UpdateStackSubmenuNode(RefPtr<UINode>& customNode);
    void ShowSubMenuHelper(const RefPtr<FrameNode>& subMenu);
    void HideSubMenu();
    void HideEmbeddedExpandMenu(const RefPtr<FrameNode>& expandableNode);
    void ShowEmbeddedExpandMenu(const RefPtr<FrameNode>& expandableNode);

    OffsetF GetSubMenuPosition(const RefPtr<FrameNode>& targetNode);

    void AddSelfHoverRegion(const RefPtr<FrameNode>& targetNode);
    void SetAccessibilityAction();
    bool IsSelectOverlayMenu();
    void RecordChangeEvent() const;
    void ParseMenuRadius(MenuParam& param);
    void ModifyDivider();

    void InitFocusEvent();
    void HandleFocusEvent();
    void HandleBlurEvent();

    void UpdateSymbolNode(RefPtr<FrameNode>& row, RefPtr<FrameNode>& selectIcon_);
    void UpdateImageNode(RefPtr<FrameNode>& row, RefPtr<FrameNode>& selectIcon_);
    void UpdateSymbolIcon(RefPtr<FrameNode>& row, RefPtr<FrameNode>& iconNode, ImageSourceInfo& iconSrc,
        std::function<void(WeakPtr<NG::FrameNode>)>& symbol, bool isStart);
    void UpdateImageIcon(RefPtr<FrameNode>& row, RefPtr<FrameNode>& iconNode, ImageSourceInfo& iconSrc,
        std::function<void(WeakPtr<NG::FrameNode>)>& symbol, bool isStart);

    std::list<TouchRegion> hoverRegions_;

    RefPtr<InputEvent> wrapperMouseEvent_;

    bool isTextFadeOut_ = false;
    bool isSelected_ = false;
    bool isSubMenuShowed_ = false;
    bool isSubMenuHovered_ = false;

    bool isChanged_ = false;
    bool isHovered_ = false;
    bool isFocused_ = false;
    bool isFocusShadowSet_ = false;
    bool isFocusBGColorSet_ = false;
    bool isExpanded_ = false;
    int32_t clickMenuItemId_ = -1;
    int32_t index_ = 0;

    std::function<void()> subBuilderFunc_ = nullptr;

    int32_t subMenuId_ = -1;
    RefPtr<FrameNode> subMenu_;
    RefPtr<FrameNode> content_ = nullptr;
    RefPtr<FrameNode> label_ = nullptr;
    RefPtr<FrameNode> startIcon_ = nullptr;
    RefPtr<FrameNode> endIcon_ = nullptr;
    RefPtr<FrameNode> selectIcon_ = nullptr;
    RefPtr<FrameNode> expandIcon_ = nullptr;
    RefPtr<LongPressEvent> longPressEvent_;
    std::vector<RefPtr<FrameNode>> expandableItems_;
    bool onTouchEventSet_ = false;
    bool onHoverEventSet_ = false;
    bool onKeyEventSet_ = false;
    bool onClickEventSet_ = false;
    SubMenuExpandingMode expandingMode_ = SubMenuExpandingMode::SIDE;
    bool isStackSubmenuHeader_ = false;

    Color bgBlendColor_ = Color::TRANSPARENT;

    ACE_DISALLOW_COPY_AND_MOVE(MenuItemPattern);
};

class CustomMenuItemPattern : public MenuItemPattern {
    DECLARE_ACE_TYPE(CustomMenuItemPattern, MenuItemPattern);

public:
    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<BoxLayoutAlgorithm>();
    }
    void OnAttachToFrameNode() override;

private:
    void OnTouch(const TouchEventInfo& info) override;
    bool OnKeyEvent(const KeyEvent& event) override;
    std::unique_ptr<Offset> lastTouchOffset_;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_MENU_MENU_ITEM_PATTERN_H
