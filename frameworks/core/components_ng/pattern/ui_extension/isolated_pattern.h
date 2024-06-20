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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_ISOLATED_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_ISOLATED_PATTERN_H

#include "core/common/dynamic_component_renderer.h"
#include "core/components_ng/pattern/ui_extension/platform_pattern.h"

namespace OHOS::Ace::NG {
struct IsolatedInfo {
    std::string abcPath;
    std::string reourcePath;
    std::string entryPoint;
};

struct IsolatedDumpInfo {
    int64_t createLimitedWorkerTime;
};

class IsolatedPattern : public PlatformPattern {
    DECLARE_ACE_TYPE(IsolatedPattern, PlatformPattern);

public:
    IsolatedPattern();
    ~IsolatedPattern() override;

    void InitializeDynamicComponent(const std::string& hapPath,
        const std::string& abcPath, const std::string& entryPoint, void* runtime);
    void InitializeIsolatedComponent(
        const RefPtr<OHOS::Ace::WantWrap>& wantWrap, void* runtime);
    bool OnDirtyLayoutWrapperSwap(
        const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    void OnDetachFromFrameNode(FrameNode* frameNode) override;

    void OnSizeChanged(int32_t width, int32_t height)
    {
        if (onSizeChanged_) {
            onSizeChanged_(width, height);
        }
    }

    void SetOnSizeChangedCallback(std::function<void(int32_t, int32_t)>&& callback)
    {
        onSizeChanged_ = std::move(callback);
    }

    virtual void SearchExtensionElementInfoByAccessibilityId(int64_t elementId, int32_t mode, int64_t baseParent,
        std::list<Accessibility::AccessibilityElementInfo>& output) override;
    virtual void SearchElementInfosByText(int64_t elementId, const std::string& text, int64_t baseParent,
        std::list<Accessibility::AccessibilityElementInfo>& output) override;
    virtual void FindFocusedElementInfo(int64_t elementId, int32_t focusType, int64_t baseParent,
        Accessibility::AccessibilityElementInfo& output) override;
    virtual void FocusMoveSearch(int64_t elementId, int32_t direction, int64_t baseParent,
        Accessibility::AccessibilityElementInfo& output) override;
    virtual bool TransferExecuteAction(int64_t elementId, const std::map<std::string, std::string>& actionArguments,
        int32_t action, int64_t offset) override;

    void DumpInfo() override;
    void FireOnErrorCallbackOnUI(
        int32_t code, const std::string& name, const std::string& msg);

private:
    void InitializeRender(void* runtime);

    void DispatchPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) override;
    void DispatchFocusActiveEvent(bool isFocusActive) override;
    bool HandleKeyEvent(const KeyEvent& event) override;
    void HandleFocusEvent() override;
    void HandleBlurEvent() override;

    void OnAttachToFrameNode() override;

    int32_t ApplyIsolatedId();

    RefPtr<DynamicComponentRenderer> dynamicComponentRenderer_;
    std::function<void(int32_t, int32_t)> onSizeChanged_;
    IsolatedInfo curIsolatedInfo_;
    IsolatedDumpInfo isolatedDumpInfo_;

    static int32_t isolatedIdGenerator_; // only run on JS thread, and do not require mutex
    ACE_DISALLOW_COPY_AND_MOVE(IsolatedPattern);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_ISOLATED_PATTERN_H
