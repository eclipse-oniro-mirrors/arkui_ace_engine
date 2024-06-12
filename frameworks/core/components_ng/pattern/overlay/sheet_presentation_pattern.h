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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_SHEET_PRESENTATION_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_SHEET_PRESENTATION_PATTERN_H

#include <functional>
#include <utility>

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components/common/properties/alignment.h"
#include "core/components_ng/manager/focus/focus_view.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_algorithm.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_property.h"
#include "core/components_ng/pattern/overlay/popup_base_pattern.h"
#include "core/components_ng/pattern/overlay/sheet_presentation_layout_algorithm.h"
#include "core/components_ng/pattern/overlay/sheet_presentation_property.h"
#include "core/components_ng/pattern/overlay/sheet_style.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

enum class BindSheetDismissReason {
    BACK_PRESSED = 0,
    TOUCH_OUTSIDE,
    CLOSE_BUTTON,
    SLIDE_DOWN,
};
class ACE_EXPORT SheetPresentationPattern : public LinearLayoutPattern, public PopupBasePattern, public FocusView {
    DECLARE_ACE_TYPE(SheetPresentationPattern, LinearLayoutPattern, PopupBasePattern, FocusView);

public:
    SheetPresentationPattern(
        int32_t targetId, const std::string& targetTag, std::function<void(const std::string&)>&& callback)
        : LinearLayoutPattern(true)
    {
        targetId_ = targetId;
        targetTag_ = targetTag;
        callback_ = std::move(callback);
    }

    ~SheetPresentationPattern() override = default;

    bool IsMeasureBoundary() const override
    {
        return true;
    }

    bool IsAtomicNode() const override
    {
        return false;
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<SheetPresentationLayoutAlgorithm>(targetId_, targetTag_, GetSheetType());
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<SheetPresentationProperty>();
    }

    int32_t GetTargetId() const override
    {
        return targetId_;
    }

    void FireCallback(const std::string& value)
    {
        if (callback_) {
            callback_(value);
        }
    }

    bool HasCallback() const
    {
        return static_cast<bool>(callback_);
    }

    void UpdateShouldDismiss(std::function<void()>&& shouldDismiss)
    {
        shouldDismiss_ = std::move(shouldDismiss);
    }

    bool HasShouldDismiss()
    {
        if (shouldDismiss_) {
            return true;
        }
        return false;
    }

    void CallShouldDismiss()
    {
        if (shouldDismiss_) {
            shouldDismiss_();
        }
    }

    void UpdateOnDisappear(std::function<void()>&& onDisappear)
    {
        onDisappear_ = std::move(onDisappear);
        isExecuteOnDisappear_ = false;
    }

    void OnDisappear()
    {
        if (onDisappear_) {
            TAG_LOGI(AceLogTag::ACE_SHEET, "bindsheet lifecycle change to onDisappear state.");
            isExecuteOnDisappear_ = true;
            onDisappear_();
        }
    }

    void UpdateOnWillDisappear(std::function<void()>&& onWillDisappear)
    {
        onWillDisappear_ = std::move(onWillDisappear);
    }

    void OnWillDisappear()
    {
        if (onWillDisappear_) {
            TAG_LOGI(AceLogTag::ACE_SHEET, "bindsheet lifecycle change to onWillDisappear state.");
            onWillDisappear_();
        }
    }

    void UpdateOnAppear(std::function<void()>&& onAppear)
    {
        onAppear_ = std::move(onAppear);
    }

    void OnAppear()
    {
        if (onAppear_) {
            TAG_LOGI(AceLogTag::ACE_SHEET, "bindsheet lifecycle change to onAppear state.");
            onAppear_();
        }
    }

    void UpdateOnHeightDidChange(std::function<void(const float)>&& onHeightDidChange)
    {
        onHeightDidChange_ = std::move(onHeightDidChange);
    }

    void OnHeightDidChange(float currentHeight) const
    {
        if (onHeightDidChange_) {
            onHeightDidChange_(currentHeight);
        }
    }

    void FireOnHeightDidChange(float height);

    bool HasOnHeightDidChange()
    {
        if (onHeightDidChange_) {
            return true;
        }
        return false;
    }

    void UpdateOnDetentsDidChange(std::function<void(const float)>&& onDetentsDidChange)
    {
        onDetentsDidChange_ = std::move(onDetentsDidChange);
    }

    void OnDetentsDidChange(float currentHeight) const
    {
        if (onDetentsDidChange_) {
            onDetentsDidChange_(currentHeight);
        }
    }

    void FireOnDetentsDidChange(float height);

    void UpdateOnWidthDidChange(std::function<void(const float)>&& onWidthDidChange)
    {
        onWidthDidChange_ = std::move(onWidthDidChange);
    }

    void onWidthDidChange(float currentWidth) const
    {
        if (onWidthDidChange_) {
            onWidthDidChange_(currentWidth);
        }
    }

    void FireOnWidthDidChange(RefPtr<FrameNode> sheetNode);

    void UpdateOnTypeDidChange(std::function<void(const float)>&& onTypeDidChange)
    {
        onTypeDidChange_ = std::move(onTypeDidChange);
    }

    void onTypeDidChange(float currentType) const
    {
        if (onTypeDidChange_) {
            onTypeDidChange_(currentType);
        }
    }

    void FireOnTypeDidChange();

    void UpdateOnWillDismiss(std::function<void(const int32_t)>&& onWillDismiss)
    {
        onWillDismiss_ = std::move(onWillDismiss);
    }

    bool HasOnWillDismiss() const
    {
        if (onWillDismiss_) {
            return true;
        }
        return false;
    }

    void CallOnWillDismiss(const int32_t reason)
    {
        if (onWillDismiss_) {
            onWillDismiss_(reason);
        }
    }

    void UpdateSheetSpringBack(std::function<void()>&& sheetSpringBack)
    {
        sheetSpringBack_ = std::move(sheetSpringBack);
    }

    bool HasSheetSpringBack() const
    {
        if (sheetSpringBack_) {
            return true;
        }
        return false;
    }

    void CallSheetSpringBack()
    {
        if (sheetSpringBack_) {
            sheetSpringBack_();
        }
    }

    void DismissSheet()
    {
        DismissTransition(false);
    }

    void SheetSpringBack()
    {
        SheetTransition(true);
    }

    void InitialLayoutProps();

    bool IsScrollable() const;
    void AvoidAiBar();

    void AvoidSafeArea();
    void CheckBuilderChange();
    float GetSheetHeightChange();
    void ScrollTo(float height);
    bool AdditionalScrollTo(const RefPtr<FrameNode>& scroll, float height);
    float InitialSingleGearHeight(NG::SheetStyle& sheetStyle);

    // initial drag gesture event
    void InitPanEvent();

    void HandleDragStart();

    void HandleDragUpdate(const GestureEvent& info);

    void HandleDragEnd(float dragVelocity);

    void OnCoordScrollStart();

    bool OnCoordScrollUpdate(float scrollOffset);

    void OnCoordScrollEnd(float dragVelocity);

    void SheetTransition(bool isTransitionIn, float dragVelocity = 0.0f);

    void ModifyFireSheetTransition(float dragVelocity = 0.0f);

    void SheetInteractiveDismiss(BindSheetDismissReason dismissReason, float dragVelocity = 0.0f);

    void SetCurrentOffset(float currentOffset)
    {
        currentOffset_ = currentOffset;
    }

    void SetIsDirectionUp(bool isDirectionUp)
    {
        isDirectionUp_ = isDirectionUp;
    }

    void SetCurrentHeight(float currentHeight)
    {
        if (height_ != currentHeight) {
            height_ = currentHeight;
            ChangeScrollHeight(height_);
        }
        ProcessColumnRect(height_);
    }

    void SetCurrentHeightToOverlay(float height)
    {
        auto overlayManager = GetOverlayManager();
        CHECK_NULL_VOID(overlayManager);
        overlayManager->SetSheetHeight(height);
    }

    void ChangeScrollHeight(float height);

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::SCOPE, true };
    }

    std::list<int32_t> GetRouteOfFirstScope() override
    {
        return { 1, 0 };
    }

    bool IsExecuteOnDisappear() const
    {
        return isExecuteOnDisappear_;
    }

    bool AvoidKeyboard() const override
    {
        return false;
    }

    void OnWindowSizeChanged(int32_t width, int32_t height, WindowSizeChangeReason type) override;

    bool HasTitleNode() const
    {
        return titleId_.has_value();
    }

    bool SetTitleId(const int32_t id)
    {
        if (HasTitleNode()) {
            return false;
        }
        titleId_ = id;
        return true;
    }

    int32_t GetTitleId()
    {
        if (!titleId_.has_value()) {
            titleId_ = ElementRegister::GetInstance()->MakeUniqueId();
        }
        return titleId_.value();
    }

    bool HasSubtitleNode() const
    {
        return titleId_.has_value();
    }

    bool SetSubtitleId(const int32_t id)
    {
        if (HasSubtitleNode()) {
            return false;
        }
        subtitleId_ = id;
        return true;
    }

    int32_t GetSubtitleId()
    {
        if (!subtitleId_.has_value()) {
            subtitleId_ = ElementRegister::GetInstance()->MakeUniqueId();
        }
        return subtitleId_.value();
    }

    static float CalculateFriction(float gamma)
    {
        constexpr float RATIO = 1.848f;
        if (GreatOrEqual(gamma, 1.0)) {
            gamma = 1.0f;
        }
        return exp(-RATIO * gamma);
    }

    SheetType GetSheetType();
    ScrollSizeMode GetScrollSizeMode();
    void GetSheetTypeWithAuto(SheetType& sheetType);
    void GetSheetTypeWithPopup(SheetType& sheetType);

    void BubbleStyleSheetTransition(bool isTransitionIn);

    void StartOffsetEnteringAnimation();

    void StartAlphaEnteringAnimation(std::function<void()> finish);

    void StartOffsetExitingAnimation();

    void StartAlphaExitingAnimation(std::function<void()> finish);

    void ResetToInvisible();

    bool IsFold();

    bool GetAnimationBreak() const
    {
        return isAnimationBreak_;
    }

    void SetAnimationProcess(bool isProcess)
    {
        isAnimationProcess_ = isProcess;
    }

    bool GetAnimationProcess()
    {
        return isAnimationProcess_;
    }

    float GetPageHeightWithoutOffset() const
    {
        return pageHeight_;
    }

    float GetPageHeight()
    {
        auto parentOffsetY = GetRootOffsetYToWindow();
        return pageHeight_ - parentOffsetY;
    }

    float GetSheetMaxHeight()
    {
        // pageHeight - statusBarHeight
        return sheetMaxHeight_;
    }

    float GetSheetMaxWidth()
    {
        return sheetMaxWidth_;
    }

    float GetSheetOffset()
    {
        return sheetOffsetY_;
    }

    float GetFitContentHeight();

    void ProcessColumnRect(float height = 0.0f);

    void SetShowState(bool show)
    {
        show_ = show;
    }

    bool GetShowState() const
    {
        return show_;
    }

    void SetIsDragging(bool isDrag)
    {
        isDrag_ = isDrag;
    }

    bool IsDragging() const
    {
        return isDrag_;
    }

    // Get ScrollHeight before avoid keyboard
    float GetScrollHeight() const
    {
        if (sheetType_ == SheetType::SHEET_CENTER) {
            return centerHeight_;
        }
        return height_;
    }

    RefPtr<OverlayManager> GetOverlayManager();
    RefPtr<FrameNode> GetOverlayRoot();
    float GetRootOffsetYToWindow();

    bool IsAvoidingKeyboard() const
    {
        return Positive(keyboardHeight_);
    }

    bool IsTypeNeedAvoidAiBar() const
    {
        return sheetType_ == SheetType::SHEET_BOTTOM || sheetType_ == SheetType::SHEET_BOTTOMLANDSPACE;
    }

    void GetBuilderInitHeight();
    void ChangeSheetPage(float height);
    void DumpAdvanceInfo() override;
    float GetTitleHeight();
protected:
    void OnDetachFromFrameNode(FrameNode* frameNode) override;

private:
    void OnModifyDone() override;
    void OnAttachToFrameNode() override;
    void OnColorConfigurationUpdate() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    void InitPageHeight();
    void TranslateTo(float height);
    void SetColumnMinSize(bool reset = false);
    void UpdateDragBarStatus();
    void UpdateCloseIconStatus();
    void UpdateSheetTitle();
    void UpdateInteractive();
    void UpdateFontScaleStatus();
    RefPtr<RenderContext> GetRenderContext();
    bool PostTask(const TaskExecutor::Task& task, const std::string& name);
    void CheckSheetHeightChange();
    void InitSheetDetents();
    void HandleFitContontChange(float height);
    void ChangeSheetHeight(float height);
    void StartSheetTransitionAnimation(const AnimationOption& option, bool isTransitionIn, float offset);
    void DismissSheetShadow(const RefPtr<RenderContext>& context);
    void ClipSheetNode();
    void CreatePropertyCallback();
    std::string GetPopupStyleSheetClipPath(SizeF sheetSize, Dimension sheetRadius);
    std::string GetCenterStyleSheetClipPath(SizeF sheetSize, Dimension sheetRadius);
    std::string GetBottomStyleSheetClipPath(SizeF sheetSize, Dimension sheetRadius);
    std::string MoveTo(double x, double y);
    std::string LineTo(double x, double y);
    std::string ArcTo(double rx, double ry, double rotation, int32_t arc_flag, double x, double y);
    void DismissTransition(bool isTransitionIn, float dragVelocity = 0.0f);
    uint32_t keyboardHeight_ = 0;
    int32_t targetId_ = -1;
    std::optional<int32_t> titleId_;
    std::optional<int32_t> subtitleId_;
    std::string targetTag_;
    std::function<void(const std::string&)> callback_;
    std::function<void()> onDisappear_;
    std::function<void()> onWillDisappear_;
    std::function<void()> shouldDismiss_;
    std::function<void(const int32_t info)> onWillDismiss_;
    std::function<void()> sheetSpringBack_;
    std::function<void(const float)> onHeightDidChange_;
    std::function<void(const float)> onDetentsDidChange_;
    std::function<void(const float)> onWidthDidChange_;
    std::function<void(const float)> onTypeDidChange_;
    std::function<void()> onAppear_;
    RefPtr<PanEvent> panEvent_;
    OffsetF arrowOffset_;
    float currentOffset_ = 0.0f;

    float preDidHeight_ = 0.0f;
    float sheetHeightUp_ = 0.0f; // sheet offset to move up when avoiding keyboard
    float height_ = 0.0f; // sheet height, start from the bottom, before avoiding keyboard
    float sheetHeight_ = 0.0f; // sheet frameSize Height
    float pageHeight_ = 0.0f; // root Height, = maxSize.Height()
    float scrollHeight_ = 0.0f;
    float preWidth_ = 0.0f;
    int32_t preType_ = -1;
    float statusBarHeight_ = .0f;
    bool isExecuteOnDisappear_ = false;
    bool windowRotate_ = false;
    bool firstMeasure_ = true;
    bool isScrolling_ = false;
    float builderHeight_ = 0.0f;
    float sheetMaxHeight_ = 0.0f; // start from the bottom, pageHeight - statusBarHeight
    float sheetMaxWidth_ = 0.0f;
    float centerHeight_ = 0.0f; // node height, not translate height
    float sheetFitContentHeight_ = 0.0f;
    float sheetOffsetX_ = 0.0f;
    float sheetOffsetY_ = 0.0f;
    bool isFirstInit_ = true;
    bool isAnimationBreak_ = false;
    bool isAnimationProcess_ = false;
    SheetType sheetType_ = SheetType::SHEET_BOTTOM;
    bool windowChanged_ = false;
    bool isDirectionUp_ = true;
    ScrollSizeMode scrollSizeMode_ = ScrollSizeMode::FOLLOW_DETENT;

    std::string sheetThemeType_ = "auto";

    std::vector<float> sheetDetentHeight_;

    std::shared_ptr<AnimationUtils::Animation> animation_;

    bool show_ = true;
    bool isDrag_ = false;
    bool isNeedProcessHeight_ = false;

    double start_ = 0.0; // start position of detents changed
    RefPtr<NodeAnimatablePropertyFloat> property_;

    ACE_DISALLOW_COPY_AND_MOVE(SheetPresentationPattern);

    float preDetentsHeight_ = 0.0f;
    float scale_ = 1.0;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_SHEET_PRESENTATION_PATTERN_H
