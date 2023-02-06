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

#include "core/components_ng/pattern/menu/menu_pattern.h"

#include "base/utils/utils.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/select/select_theme.h"
#include "core/components_ng/event/click_event.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/pattern/option/option_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/event/touch_event.h"
#include "core/pipeline/pipeline_base.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
void MenuPattern::OnModifyDone()
{
    if (!onClick_) {
        RegisterOnClick();
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    RegisterOnKeyEvent(focusHub);

    if (IsMultiMenu()) {
        return;
    }

    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);

    // get theme from SelectThemeManager
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();

    // set background color
    auto bgColor = theme->GetBackgroundColor();
    renderContext->UpdateBackgroundColor(bgColor);

    // make menu round rect
    BorderRadiusProperty borderRadius;
    borderRadius.SetRadius(theme->GetMenuBorderRadius());
    renderContext->UpdateBorderRadius(borderRadius);
    renderContext->UpdateBackShadow(ShadowConfig::DefaultShadowM);
    renderContext->SetClipToBounds(true);
}

// close menu on touch up
void MenuPattern::RegisterOnClick()
{
    auto event = [targetId = targetId_, isContextMenu = IsContextMenu(), weak = WeakClaim(this)](
                     const TouchEventInfo& info) {
        auto menuPattern = weak.Upgrade();
        CHECK_NULL_VOID(menuPattern);
        auto menuNode = menuPattern->GetHost();
        CHECK_NULL_VOID(menuNode);
        bool isCustomOption = false;
        auto firstChild = menuNode->GetChildAtIndex(0);
        if (firstChild && firstChild->GetTag() == V2::SCROLL_ETS_TAG) {
            isCustomOption = true;
            auto scrollChild = firstChild->GetChildAtIndex(0);
            if ((scrollChild && scrollChild->GetTag() == V2::MENU_ETS_TAG)) {
                isCustomOption = false;
            }
        }
        if (!menuPattern->IsSubMenu() && !isCustomOption) {
            // menu with options or menu items is closed by option or menuItem click event.
            return;
        }

        auto touches = info.GetTouches();
        if (touches.empty() || touches.front().GetTouchType() != TouchType::UP) {
            return;
        }

        if (menuPattern->IsSubMenu()) {
            auto menuItemParent = menuPattern->GetParentMenuItem();
            auto menuItemPattern = menuItemParent->GetPattern<MenuItemPattern>();
            menuItemPattern->CloseMenu();
            return;
        }

        if (isContextMenu) {
            SubwindowManager::GetInstance()->HideMenuNG(targetId);
            return;
        }
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto overlayManager = pipeline->GetOverlayManager();
        CHECK_NULL_VOID(overlayManager);
        overlayManager->HideMenu(targetId);
        LOGI("MenuPattern closing menu %{public}d", targetId);
    };
    onClick_ = MakeRefPtr<TouchEventImpl>(std::move(event));

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gestureHub = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->AddTouchEvent(onClick_);
}

void MenuPattern::RegisterOnKeyEvent(const RefPtr<FocusHub>& focusHub)
{
    auto onKeyEvent = [wp = WeakClaim(this)](const KeyEvent& event) -> bool {
        auto pattern = wp.Upgrade();
        CHECK_NULL_RETURN_NOLOG(pattern, false);
        return pattern->OnKeyEvent(event);
    };
    focusHub->SetOnKeyEventInternal(std::move(onKeyEvent));
}

bool MenuPattern::OnKeyEvent(const KeyEvent& event) const
{
    if (event.action != KeyAction::DOWN || IsMultiMenu()) {
        return false;
    }
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto overlayManager = pipeline->GetOverlayManager();
    CHECK_NULL_RETURN(overlayManager, false);
    if (event.code == KeyCode::KEY_ESCAPE) {
        if (IsContextMenu()) {
            SubwindowManager::GetInstance()->HideMenuNG(targetId_);
            return true;
        }
        overlayManager->HideMenu(targetId_);
        return true;
    }
    if (event.code == KeyCode::KEY_DPAD_LEFT && IsSubMenu()) {
        overlayManager->HideMenu(targetId_);
        return true;
    }
    return false;
}

void MenuPattern::RemoveParentHoverStyle()
{
    if (!IsSubMenu()) {
        return;
    }
    auto menuItemParent = GetParentMenuItem();
    CHECK_NULL_VOID(menuItemParent);
    auto menuItemPattern = menuItemParent->GetPattern<MenuItemPattern>();
    CHECK_NULL_VOID(menuItemPattern);
    menuItemPattern->SetIsSubMenuShowed(false);

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    menuItemPattern->UpdateBackgroundColor(theme->GetBackgroundColor());
}
} // namespace OHOS::Ace::NG