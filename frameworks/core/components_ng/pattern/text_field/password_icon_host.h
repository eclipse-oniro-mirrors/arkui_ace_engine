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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_PASSWORD_ICON_HOST_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_PASSWORD_ICON_HOST_H

#include <optional>
#include <vector>

#include "base/log/log_wrapper.h"
#include "base/memory/ace_type.h"
#include "core/common/ace_application_info.h"
#include "core/common/ime/password_obscure_helper.h"
#include "core/common/ime/text_input_action.h"
#include "core/common/ime/text_input_filter.h"
#include "core/common/ime/text_input_keyboard_utils.h"
#include "core/common/ime/text_input_type.h"
#include "core/common/text_capability_model.h"
#include "core/common/text_field_manager_ng.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/property/accessibility_property.h"
#include "core/event/mouse_event.h"
#include "core/image/image_source_info.h"

namespace OHOS::Ace {
class ImageSourceInfo;
class TextFieldTheme;
enum class TextDirection;

namespace NG {
class RoundRect;
class EventHub;
class PasswordResponseArea;

/**
 * @brief Interface for text input mode hosting, decoupling PasswordResponseArea
 *        from concrete Pattern types (TextFieldPattern, RichEditorPattern).
 *
 * Input Mode is determined by TextInputType. Different input modes correspond to
 * different keyboard types, capability sets, and obscure behaviors.
 * Password input mode is a subset of input mode, not the whole.
 */
class IPasswordIconHost : public virtual AceType {
    DECLARE_ACE_TYPE(IPasswordIconHost, AceType);
public:
    ~IPasswordIconHost() override = default;

    virtual bool IsInPasswordMode() const = 0;
    virtual bool IsShowPasswordIcon() const = 0;
    virtual bool IsShowPasswordSymbol() const = 0;
    virtual void OnObscuredChanged(bool isObscured) = 0;
    virtual RefPtr<FrameNode> GetHost() const = 0;
    virtual TextDirection GetPasswordIconDirection() const = 0;
    virtual bool CheckLayoutProperty() const = 0;
    virtual ImageSourceInfo GetShowPasswordSourceInfo(const ImageSourceInfo& defaultInfo) = 0;
    virtual ImageSourceInfo GetHidePasswordSourceInfo(const ImageSourceInfo& defaultInfo) = 0;
    virtual bool IsDisabled() const = 0;
    virtual bool IsTV() const = 0;
    virtual void RestoreDefaultMouseState() = 0;
    virtual bool SetPasswordIconHoverColor(const std::vector<RoundRect>& rects, uint32_t color) = 0;
    virtual bool ClearPasswordIconHoverColor() = 0;
    virtual bool GetPasswordIconHoverColor(uint32_t& color) = 0;
    virtual bool GetPasswordIconPressColor(uint32_t& color) = 0;
    virtual void SetResponseButtonTouched(bool isTouched) {}
    virtual void OnHover(bool isHover, const HoverInfo& info) {}
};

template<typename Derived, typename LayoutPropertyT>
class PasswordIconHostBase : public IPasswordIconHost {
public:
    RefPtr<TextFieldTheme> GetTextFieldTheme() const
    {
        return static_cast<const Derived*>(this)->GetTextFieldThemeImpl();
    }

    TextDirection GetPasswordIconDirection() const override
    {
        auto prop = static_cast<const Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>();
        CHECK_NULL_RETURN(prop, TextDirection::LTR);
        return prop->GetNonAutoLayoutDirection();
    }

    bool CheckLayoutProperty() const override
    {
        return static_cast<const Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>() != nullptr;
    }

    ImageSourceInfo GetShowPasswordSourceInfo(const ImageSourceInfo& defaultInfo) override
    {
        auto prop = static_cast<const Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>();
        CHECK_NULL_RETURN(prop, defaultInfo);
        return prop->GetShowPasswordSourceInfoValue(defaultInfo);
    }

    ImageSourceInfo GetHidePasswordSourceInfo(const ImageSourceInfo& defaultInfo) override
    {
        auto prop = static_cast<const Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>();
        CHECK_NULL_RETURN(prop, defaultInfo);
        return prop->GetHidePasswordSourceInfoValue(defaultInfo);
    }

    bool IsShowPasswordIcon() const override
    {
        auto prop = static_cast<const Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>();
        CHECK_NULL_RETURN(prop, false);
        auto theme = GetTextFieldTheme();
        CHECK_NULL_RETURN(theme, false);
        return prop->GetShowPasswordIconValue(theme->IsShowPasswordIcon())
            && static_cast<const Derived*>(this)->IsInPasswordMode();
    }

    bool IsTV() const override
    {
        auto theme = GetTextFieldTheme();
        CHECK_NULL_RETURN(theme, false);
        return theme->GetHoverAndPressBgColorEnabled();
    }

    bool IsDisabled() const override
    {
        auto eventHub = static_cast<const Derived*>(this)->template GetEventHub<EventHub>();
        CHECK_NULL_RETURN(eventHub, true);
        auto prop = static_cast<const Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>();
        CHECK_NULL_RETURN(prop, true);
        return !eventHub->IsEnabled();
    }

    bool GetPasswordIconHoverColor(uint32_t& color) override
    {
        auto theme = GetTextFieldTheme();
        CHECK_NULL_RETURN(theme, false);
        color = theme->GetHoverColor().GetValue();
        return true;
    }

    bool GetPasswordIconPressColor(uint32_t& color) override
    {
        auto theme = GetTextFieldTheme();
        CHECK_NULL_RETURN(theme, false);
        color = theme->GetPressColor().GetValue();
        return true;
    }

    bool SetPasswordIconHoverColor(const std::vector<RoundRect>& rects, uint32_t color) override
    {
        auto host = static_cast<Derived*>(this)->GetHost();
        CHECK_NULL_RETURN(host, false);
        CHECK_NULL_RETURN(SetHoverColorAndRectsOnModifier(rects, color), false);
        host->MarkDirtyNode(GetHoverDirtyFlag());
        return true;
    }

    bool ClearPasswordIconHoverColor() override
    {
        auto host = static_cast<Derived*>(this)->GetHost();
        CHECK_NULL_RETURN(host, false);
        CHECK_NULL_RETURN(ClearHoverColorAndRectsOnModifier(), false);
        host->MarkDirtyNode(GetHoverDirtyFlag());
        return true;
    }

    void SetIsPasswordSymbol(bool isPasswordSymbol)
    {
        isPasswordSymbol_ = isPasswordSymbol;
    }

    bool IsShowPasswordSymbol() const override
    {
        return isPasswordSymbol_ &&
            AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_THIRTEEN);
    }

    TextInputType GetTextInputType() const
    {
        auto prop = static_cast<const Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>();
        return prop ? prop->GetTextInputTypeValue(TextInputType::UNSPECIFIED) : TextInputType::UNSPECIFIED;
    }

    bool IsOneTimeCodeType() const
    {
        return IsOneTimeCodeInputType(GetTextInputType());
    }

    bool IsInPasswordMode() const override
    {
        return IsPasswordInputType(GetTextInputType());
    }

    virtual bool IsSingleLineForPassword() const = 0;

    void UpdateObscure(const std::u16string& insertValue, bool hasInsert)
    {
        CHECK_NULL_VOID(IsSingleLineForPassword() && IsInPasswordMode() && GetTextObscured());
        auto inputType = GetTextInputType();
        bool isSingleVisibleChar = hasInsert && insertValue.length() == 1 &&
            (inputType != TextInputType::NUMBER_PASSWORD || std::isdigit(insertValue[0]));
        if (isSingleVisibleChar) {
            obscureHelper_.UpdateObscure(
                inputType, insertValue, hasInsert, GetObscureContent(), GetObscureCaretPosition());
        } else {
            obscureHelper_.ClearState();
        }
    }

    int32_t GetNakedCharPosition() const
    {
        if (!TextInputObscureUtils::ShouldRevealNakedChar(IsSingleLineForPassword(), IsInPasswordMode(),
                GetTextObscured(), obscureHelper_.GetTickCountDown())) {
            return -1;
        }
        CHECK_NULL_RETURN(CheckLayoutProperty(), -1);
        return obscureHelper_.GetNakedCharPosition(HasObscureContent());
    }

    bool IsPasswordObscured() const
    {
        return IsSingleLineForPassword() && IsInPasswordMode() && GetTextObscured();
    }

    void CheckAndUpdateInputTypeForOtp()
    {
        auto host = static_cast<Derived*>(this)->GetHost();
        CHECK_NULL_VOID(host);
        auto layoutProperty = static_cast<Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>();
        CHECK_NULL_VOID(layoutProperty);
        CHECK_NULL_VOID(TryUpgradeToOtpType(layoutProperty.GetRawPtr(), keyboard_, isFilterChanged_));
        TAG_LOGI(GetLogTag(), "%{public}d detected verify code, update type to OTC", host->GetId());
        if (static_cast<Derived*>(this)->HasFocus()) {
            RequestKeyboardForOTP();
        }
    }

    bool DoRequestKeyboardNotByFocusSwitch(SourceType sourceType)
    {
        CHECK_NULL_RETURN(DoCallRequestKeyboard(sourceType), false);
        auto textFieldManager = GetTextFieldManager();
        CHECK_NULL_RETURN(textFieldManager, true);
        textFieldManager->SetNeedToRequestKeyboard(false);
        return true;
    }

    RefPtr<TextFieldManagerNG> GetTextFieldManager()
    {
        auto pipeline = static_cast<Derived*>(this)->GetContext();
        CHECK_NULL_RETURN(pipeline, nullptr);
        return DynamicCast<TextFieldManagerNG>(pipeline->GetTextFieldManager());
    }

    void CheckPasswordAreaState()
    {
        auto obscuredState = GetObscuredState();
        CHECK_NULL_VOID(obscuredState.has_value());
        auto passwordArea = GetPasswordResponseArea();
        CHECK_NULL_VOID(passwordArea);
        bool obscured = obscuredState.value();
        bool showPasswordText = !obscured;
        if (!showPasswordState_.has_value() || showPasswordState_.value() != showPasswordText) {
            showPasswordState_ = showPasswordText;
            passwordArea->SetObscured(obscured);
        }
    }

    bool ResetObscureTickCountDown(PropertyChangeFlag flag = PROPERTY_UPDATE_MEASURE_SELF)
    {
        CHECK_NULL_RETURN(IsPasswordObscured(), false);
        bool changed = obscureHelper_.ResetTickCountDown();
        if (changed) {
            OnObscureDirty(flag);
        }
        return changed;
    }

    void TickDownPasswordObscure()
    {
        if (!IsPasswordObscured() || obscureHelper_.GetTickCountDown() <= 0) {
            return;
        }
        obscureHelper_.TickDown();
        if (obscureHelper_.GetTickCountDown() == 0) {
            OnObscureDirty(PROPERTY_UPDATE_MEASURE);
        }
    }

    void CheckIfNeedToResetKeyboard()
    {
        auto host = static_cast<Derived*>(this)->GetHost();
        CHECK_NULL_VOID(host);
        auto currentType = GetTextInputType();
        if (ShouldDoDynamicSwitch(currentType)) {
            DoDynamicSwitch(currentType);
            return;
        }
        bool needToResetKeyboard = false;
        if (IsKeyboardTypeChanged()) {
            TAG_LOGI(GetLogTag(), "%{public}d KBType %{public}d -> %{public}d",
                host->GetId(), static_cast<int32_t>(keyboard_),
                static_cast<int32_t>(currentType));
            keyboard_ = currentType;
            OnKeyboardTypeChanged();
            needToResetKeyboard = ShouldResetKeyboardOnTypeChange();
        }
        DoResetKeyboardCommon(host, needToResetKeyboard, false);
    }

    void DoResetKeyboardCommon(const RefPtr<FrameNode>& host,
        bool needToResetKeyboard, bool isDynamic)
    {
        if (ShouldCheckOtpOnReset(isDynamic)) {
            CheckAndUpdateInputTypeForOtp();
        }
        auto currentAction = static_cast<Derived*>(this)->GetTextInputActionValue(
            static_cast<Derived*>(this)->GetDefaultTextInputAction());
        if (!needToResetKeyboard && action_ != TextInputAction::UNSPECIFIED) {
            needToResetKeyboard = action_ != currentAction;
        }
        action_ = currentAction;
        DoKeyboardResetPlatform(host, needToResetKeyboard, isDynamic);
    }

    TextCapability GetDisabledCapabilities() const
    {
        return IsInPasswordMode() ? GetPasswordDisabledCapabilities() : TextCapability::NONE;
    }

    bool IsCapabilityAllowed(TextCapability cap) const
    {
        return OHOS::Ace::IsCapabilityAllowed(GetDisabledCapabilities(), cap);
    }

    bool IsCapabilityDisabled(TextCapability cap) const
    {
        return OHOS::Ace::IsCapabilityDisabled(GetDisabledCapabilities(), cap);
    }

    std::string GetPasswordIconPromptInformation(bool show)
    {
        auto theme = GetTextFieldTheme();
        CHECK_NULL_RETURN(theme, "");
        return show ? theme->GetShowPasswordPromptInformation()
                    : theme->GetHiddenPasswordPromptInformation();
    }

    void SetAccessibilityPasswordIconAction()
    {
        CHECK_NULL_VOID(IsShowPasswordIcon());
        auto passwordArea = GetPasswordResponseArea();
        CHECK_NULL_VOID(passwordArea);
        auto node = passwordArea->GetFrameNode();
        CHECK_NULL_VOID(node);
        auto textAccessibilityProperty = node->template GetAccessibilityProperty<AccessibilityProperty>();
        CHECK_NULL_VOID(textAccessibilityProperty);
        textAccessibilityProperty->SetAccessibilityLevel("yes");
        textAccessibilityProperty->SetAccessibilityText(GetPasswordIconPromptInformation(passwordArea->IsObscured()));
        textAccessibilityProperty->SetAccessibilityCustomRole("button");
    }

    bool ProcessPasswordArea()
    {
        auto host = static_cast<Derived*>(this)->GetHost();
        CHECK_NULL_RETURN(host, false);
        // Not in password mode: callers own the teardown of any leftover
        // password area, so the shared base stays free of host-specific cleanup.
        if (!IsInPasswordMode()) {
            return false;
        }
        auto area = GetPasswordResponseArea();
        if (area) {
            // A password area already exists.
            if (IsShowPasswordIcon()) {
                area->Refresh();
            } else {
                area->ClearArea();
            }
            CheckPasswordAreaState();
            return true;
        }
        // No password area yet; the slot may hold a non-password area left over
        // from a previous mode, tear it down before installing the new one.
        ClearNonPasswordResponseArea();
        auto newArea = AceType::MakeRefPtr<PasswordResponseArea>(
            WeakClaim(static_cast<Derived*>(this)), textObscured_);
        SetPasswordResponseArea(newArea);
        if (IsShowPasswordIcon()) {
            newArea->InitResponseArea();
        } else {
            // Legacy behavior: allocate then immediately clear. ClearArea is a
            // no-op because InitResponseArea was not called; the slot is left
            // non-null, matching the old inline logic.
            newArea->ClearArea();
        }
        CheckPasswordAreaState();
        return true;
    }

protected:
    PasswordObscureHelper obscureHelper_;
    bool textObscured_ = true;
    // Input mode core attributes: keyboard_ records the keyboard type for the
    // current input mode; action_ records the enter key action.
    TextInputType keyboard_ = TextInputType::UNSPECIFIED;
    TextInputAction action_ = TextInputAction::UNSPECIFIED;
    std::optional<bool> showPasswordState_;
    bool isFilterChanged_ = false; // re-filter existing content on next layout

public:
    bool GetTextObscured() const { return textObscured_; }
    TextInputType GetKeyboard() const { return keyboard_; }
    TextInputAction GetAction() const { return action_; }

    bool IsFilterChanged() const { return isFilterChanged_; }
    void SetFilterChanged(bool changed) { isFilterChanged_ = changed; }
    bool ConsumeFilterChanged()
    {
        bool v = isFilterChanged_;
        isFilterChanged_ = false;
        return v;
    }
    void RequestKeyboardForOTP()
    {
        auto host = static_cast<Derived*>(this)->GetHost();
        CHECK_NULL_VOID(host);
        TAG_LOGI(GetLogTag(), "%{public}d requestKB, reason: RESET_KEYBOARD (OTP)", host->GetId());
        DoRequestKeyboardNotByFocusSwitch(SourceType::NONE);
    }
    virtual AceLogTag GetLogTag() const = 0;

    bool DoCallRequestKeyboard(SourceType sourceType)
    {
        return static_cast<Derived*>(this)->RequestKeyboard(false, true, true, sourceType);
    }

    virtual std::optional<bool> GetObscuredState() { return textObscured_; }
    virtual RefPtr<PasswordResponseArea> GetPasswordResponseArea() = 0;

    virtual void OnObscureDirty(PropertyChangeFlag flag) {}

    virtual bool ShouldDoDynamicSwitch(TextInputType currentType) { return false; }
    virtual void DoDynamicSwitch(TextInputType currentType) {}
    virtual bool IsKeyboardTypeChanged() { return GetTextInputType() != keyboard_; }
    virtual void OnKeyboardTypeChanged() { ResetPreviewTextState(); }
    virtual void ResetPreviewTextState() = 0;
    virtual bool ShouldResetKeyboardOnTypeChange() const { return true; }
    virtual bool ShouldCheckOtpOnReset(bool isDynamic) const { return true; }
    virtual void DoKeyboardResetPlatform(const RefPtr<FrameNode>& host, bool needToResetKeyboard, bool isDynamic)
    {
        DoKeyboardResetPlatformCommon(needToResetKeyboard);
    }

    void DoKeyboardResetPlatformCommon(bool needToResetKeyboard)
    {
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
        if (needToResetKeyboard && static_cast<Derived*>(this)->HasFocus()) {
            if (IsCustomKeyboardAttached() || IsOneTimeCodeType()) {
                auto kbHost = static_cast<Derived*>(this)->GetHost();
                CHECK_NULL_VOID(kbHost);
                TAG_LOGI(GetLogTag(), "%{public}d requestKB, reason: RESET_KEYBOARD, sourceType:%{public}d",
                    kbHost->GetId(), static_cast<int32_t>(SourceType::NONE));
                DoRequestKeyboardNotByFocusSwitch(SourceType::NONE);
                return;
            }
#if defined(ENABLE_STANDARD_INPUT)
            auto kbHost = static_cast<Derived*>(this)->GetHost();
            CHECK_NULL_VOID(kbHost);
            TAG_LOGI(GetLogTag(), "%{public}d KB action:%{public}d",
                kbHost->GetId(), static_cast<int32_t>(action_));
#endif
            NotifyInputMethodConfigChange(action_, keyboard_);
        }
#else
        if (needToResetKeyboard && static_cast<Derived*>(this)->HasConnection()) {
            static_cast<Derived*>(this)->CloseKeyboard(true);
            static_cast<Derived*>(this)->RequestKeyboard(false, true, true);
        }
#endif
    }

    virtual bool IsCustomKeyboardAttached() const { return false; }

    // Bridges hover color/rects to the host's overlay modifier. Default impl delegates to
    // Derived::SetOverlayHoverColorAndRects so password-icon and clean-button hover share
    // one hook (see CleanNodeHostBase). Override only to customize modifier access.
    virtual bool SetHoverColorAndRectsOnModifier(const std::vector<RoundRect>& rects, uint32_t color)
    {
        return static_cast<Derived*>(this)->SetOverlayHoverColorAndRects(rects, color);
    }
    virtual bool ClearHoverColorAndRectsOnModifier()
    {
        return static_cast<Derived*>(this)->ClearOverlayHoverColorAndRects();
    }
    virtual PropertyChangeFlag GetHoverDirtyFlag() const
    {
        return PROPERTY_UPDATE_MEASURE_SELF;
    }
    void FireSecurityStateChanged(bool isSecure)
    {
        auto hub = static_cast<Derived*>(this)->GetSecurityEventHubImpl();
        CHECK_NULL_VOID(hub);
        hub->FireOnSecurityStateChanged(isSecure);
    }
    virtual void SetPasswordResponseArea(const RefPtr<PasswordResponseArea>& area) = 0;
    // Tears down a pre-existing response area that is not a PasswordResponseArea
    // (e.g. a UnitResponseArea left over from a previous non-password mode).
    // Returns true when an area was actually cleared. Default is a no-op for hosts
    // that keep the password area in a dedicated member (e.g. RichEditorPattern).
    virtual bool ClearNonPasswordResponseArea() { return false; }

private:
    virtual std::u16string GetObscureContent() const = 0;
    virtual int32_t GetObscureCaretPosition() const = 0;
    virtual bool HasObscureContent() const = 0;

    bool isPasswordSymbol_ = true;
};

} // namespace OHOS::Ace::NG
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_PASSWORD_ICON_HOST_H
