/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_TEXT_SELECT_OVERLAY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_TEXT_SELECT_OVERLAY_H
#include <optional>

#include "base/geometry/ng/rect_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/pattern/text/base_text_select_overlay.h"
#include "core/components_ng/pattern/text/text_base.h"

namespace OHOS::Ace::NG {
class TextSelectOverlay : public BaseTextSelectOverlay {
    DECLARE_ACE_TYPE(TextSelectOverlay, BaseTextSelectOverlay);

public:
    explicit TextSelectOverlay(const WeakPtr<TextBase>& textBase) : BaseTextSelectOverlay(textBase) {}
    ~TextSelectOverlay() = default;

    // override TextBaseSelectOverlay
    bool PreProcessOverlay(const OverlayRequest& request) override;
    bool CheckHandleVisible(const RectF& paintRect) override;
    bool CheckAndAdjustHandle(RectF& paintRect);
    void OnResetTextSelection() override;

    // override SelectOverlayHolder
    std::optional<SelectHandleInfo> GetFirstHandleInfo() override;
    std::optional<SelectHandleInfo> GetSecondHandleInfo() override;
    void OnUpdateMenuInfo(SelectMenuInfo& menuInfo, SelectOverlayDirtyFlag dirtyFlag) override;
    void OnUpdateSelectOverlayInfo(SelectOverlayInfo& overlayInfo, int32_t requestCode) override;
    RectF GetSelectArea() override;
    void GetSelectAreaFromHandle(RectF& rect);
    std::string GetSelectedText() override;

    // override SelectOverlayCallback
    void OnMenuItemAction(OptionMenuActionId id, OptionMenuType type) override;
    void OnHandleMove(const RectF& rect, bool isFirst) override;
    void OnHandleMoveDone(const RectF& rect, bool isFirst) override;
    void OnCloseOverlay(OptionMenuType menuType, CloseReason reason, RefPtr<OverlayInfo> info = nullptr) override;
    void OnHandleGlobalTouchEvent(SourceType sourceType, TouchType touchType) override;

protected:
    virtual void UpdateSelectorOnHandleMove(const OffsetF& handleOffset, bool isFirstHandle);
    bool selectTextUseTopHandle = false;

private:
    ACE_DISALLOW_COPY_AND_MOVE(TextSelectOverlay);
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_TEXT_SELECT_OVERLAY_H
