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
#include "core/components_ng/pattern/option/option_pattern.h"
#include "core/event/touch_event.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
void MenuPattern::OnModifyDone()
{
    if (!onClick_) {
        RegisterOnClick();
    }

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);

    // get theme from SelectThemeManager
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();

    // set background color
    auto bgColor = theme->GetBackgroundColor();
    renderContext->UpdateBackgroundColor(bgColor);

    // make menu round rect
    BorderRadiusProperty borderRadius;
    borderRadius.SetRadius(theme->GetPopupRRectSize());
    renderContext->UpdateBorderRadius(borderRadius);
    renderContext->UpdateBackShadow(ShadowConfig::DefaultShadowM);
}

// close menu on touch up
void MenuPattern::RegisterOnClick()
{
    auto event = [targetId = targetId_, isContextMenu = IsContextMenu()](const TouchEventInfo& info) {
        auto touches = info.GetTouches();
        if (touches.empty() || touches.front().GetTouchType() != TouchType::UP) {
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
        LOGI("closing menu %{public}d", targetId);
    };
    onClick_ = MakeRefPtr<TouchEventImpl>(std::move(event));

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gestureHub = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->AddTouchEvent(onClick_);
}

} // namespace OHOS::Ace::NG
