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

#include "core/components_ng/pattern/menu/wrapper/menu_wrapper_pattern.h"

#include "base/utils/utils.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/select/select_theme.h"
#include "core/components_ng/event/click_event.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
void MenuWrapperPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gestureHub = host->GetOrCreateGestureEventHub();

    // close menu when clicked outside the menu region
    auto callback = [targetId = targetId_, weak = WeakClaim(RawPtr(host))](const TouchEventInfo& info) {
        if (info.GetTouches().empty()) {
            return;
        }
        auto host = weak.Upgrade();
        CHECK_NULL_VOID(host);

        auto menuNode = host->GetChildAtIndex(0);
        CHECK_NULL_VOID(menuNode);
        if (!InstanceOf<FrameNode>(menuNode)) {
            LOGW("MenuWrapper's child is not a Menu! type = %{public}s", menuNode->GetTag().c_str());
            return;
        }
        auto menuFrameNode = DynamicCast<FrameNode>(menuNode);
        CHECK_NULL_VOID(menuFrameNode);
        auto menuPattern = menuFrameNode->GetPattern<MenuPattern>();
        CHECK_NULL_VOID(menuPattern);
        // ContextMenu: close subwindow whenever touch event calls
        if (menuPattern->IsContextMenu()) {
            SubwindowManager::GetInstance()->HideMenuNG(targetId);
            return;
        }

        // get menuNode's touch region
        auto menuGeometryNode = menuFrameNode->GetGeometryNode();
        CHECK_NULL_VOID(menuGeometryNode);
        auto menuZone = menuGeometryNode->GetFrameRect();
        const auto& position = info.GetTouches().front().GetGlobalLocation();
        // if DOWN-touched outside the menu region, then hide menu
        if (!menuZone.IsInRegion(PointF(position.GetX(), position.GetY()))) {
            auto pipeline = host->GetContext();
            CHECK_NULL_VOID(pipeline);
            auto overlayManager = pipeline->GetOverlayManager();

            overlayManager->HideMenu(targetId);
            LOGD("touch is outside the menu region (%{public}f, %{public}f), menu %{public}d removed", position.GetX(),
                position.GetY(), targetId);
        }
    };
    auto touchEvent = MakeRefPtr<TouchEventImpl>(std::move(callback));
    gestureHub->AddTouchEvent(touchEvent);
}

bool MenuWrapperPattern::OnDirtyLayoutWrapperSwap(
    const RefPtr<LayoutWrapper>& /*dirty*/, bool /*skipMeasure*/, bool /*skipLayout*/)
{
    // setup contextMenu's subWindow hotZone
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto gestureHub = host->GetOrCreateGestureEventHub();

    auto menuNode = host->GetChildAtIndex(0);
    CHECK_NULL_RETURN(menuNode, false);
    if (!InstanceOf<FrameNode>(menuNode)) {
        LOGW("MenuWrapper's child is not a Menu! type = %{public}s", menuNode->GetTag().c_str());
        return false;
    }
    auto menuFrameNode = DynamicCast<FrameNode>(menuNode);
    CHECK_NULL_RETURN(menuFrameNode, false);
    auto menuGeometryNode = menuFrameNode->GetGeometryNode();
    CHECK_NULL_RETURN(menuGeometryNode, false);
    auto menuZone = menuGeometryNode->GetFrameRect();
    auto menuPattern = menuFrameNode->GetPattern<MenuPattern>();
    CHECK_NULL_RETURN(menuPattern, false);

    if (menuPattern->IsContextMenu()) {
        std::vector<Rect> rects;
        rects.emplace_back(Rect(Offset(menuFrameNode->GetOffsetRelativeToWindow().GetX(),
                                    menuFrameNode->GetOffsetRelativeToWindow().GetY()),
            Size(menuZone.Width(), menuZone.Height())));
        SubwindowManager::GetInstance()->SetHotAreas(rects);
    }
    return true;
}

} // namespace OHOS::Ace::NG
