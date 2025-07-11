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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_UIEXTENSION_SURFACE_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_UIEXTENSION_SURFACE_PATTERN_H

#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_layout_algorithm.h"


namespace OHOS::Ace::NG {
class UIExtensionSurfacePattern : public Pattern {
    DECLARE_ACE_TYPE(UIExtensionSurfacePattern, Pattern);

public:
    UIExtensionSurfacePattern() = default;
    ~UIExtensionSurfacePattern() override = default;
    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<UIExtensionLayoutAlgorithm>();
    }

    void OnDetachFromFrameNode(FrameNode* framenode) override
    {
        LOGI("UIExtensionSurface: OnDetachFromFrameNode");
        CHECK_NULL_VOID(framenode);
        auto renderContext = framenode->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        renderContext->RemoveFromTree();
    }

    void BeforeSyncGeometryProperties(const DirtySwapConfig& config) override
    {
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto renderContext = host->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        bool isDisappearing = renderContext->IsDisappearing();
        auto paintRect = renderContext->GetPaintRectWithoutTransform();
        TAG_LOGI(AceLogTag::ACE_UIEXTENSIONCOMPONENT,
            "UIExtensionSurface: isDisappearing = %{public}d, paintRect = %{public}s.",
                isDisappearing, paintRect.ToString().c_str());
    }
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_UIEXTENSION_SURFACE_PATTERN_H
