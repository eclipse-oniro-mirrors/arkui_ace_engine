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

#include "password_icon_host.h"

#include "base/log/log_wrapper.h"
#include "core/common/ace_application_info.h"
#include "core/common/ime/text_input_filter.h"
#include "core/common/ime/text_input_keyboard_utils.h"
#include "core/common/manager_interface.h"
#include "core/common/text_capability_model.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/property/accessibility_property.h"
#include "core/image/image_source_info.h"
#include "ui/base/event_source.h"

#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"

namespace OHOS::Ace::NG {

template<typename Derived, typename LayoutPropertyT>
RefPtr<TextFieldTheme> PasswordIconHostBase<Derived, LayoutPropertyT>::GetTextFieldTheme() const
{
    return static_cast<const Derived*>(this)->GetTextFieldThemeImpl();
}

template<typename Derived, typename LayoutPropertyT>
TextDirection PasswordIconHostBase<Derived, LayoutPropertyT>::GetPasswordIconDirection() const
{
    auto prop = static_cast<const Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>();
    CHECK_NULL_RETURN(prop, TextDirection::LTR);
    return prop->GetNonAutoLayoutDirection();
}

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::CheckLayoutProperty() const
{
    return static_cast<const Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>() != nullptr;
}

template<typename Derived, typename LayoutPropertyT>
ImageSourceInfo PasswordIconHostBase<Derived, LayoutPropertyT>::GetShowPasswordSourceInfo(
    const ImageSourceInfo& defaultInfo)
{
    auto prop = static_cast<const Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>();
    CHECK_NULL_RETURN(prop, defaultInfo);
    return prop->GetShowPasswordSourceInfoValue(defaultInfo);
}

template<typename Derived, typename LayoutPropertyT>
ImageSourceInfo PasswordIconHostBase<Derived, LayoutPropertyT>::GetHidePasswordSourceInfo(
    const ImageSourceInfo& defaultInfo)
{
    auto prop = static_cast<const Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>();
    CHECK_NULL_RETURN(prop, defaultInfo);
    return prop->GetHidePasswordSourceInfoValue(defaultInfo);
}

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::IsShowPasswordIcon() const
{
    auto prop = static_cast<const Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>();
    CHECK_NULL_RETURN(prop, false);
    auto theme = GetTextFieldTheme();
    CHECK_NULL_RETURN(theme, false);
    return prop->GetShowPasswordIconValue(theme->IsShowPasswordIcon())
        && static_cast<const Derived*>(this)->IsInPasswordMode();
}

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::IsTV() const
{
    auto theme = GetTextFieldTheme();
    CHECK_NULL_RETURN(theme, false);
    return theme->GetHoverAndPressBgColorEnabled();
}

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::IsDisabled() const
{
    auto eventHub = static_cast<const Derived*>(this)->template GetEventHub<EventHub>();
    CHECK_NULL_RETURN(eventHub, true);
    auto prop = static_cast<const Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>();
    CHECK_NULL_RETURN(prop, true);
    return !eventHub->IsEnabled();
}

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::GetPasswordIconHoverColor(uint32_t& color)
{
    auto theme = GetTextFieldTheme();
    CHECK_NULL_RETURN(theme, false);
    color = theme->GetHoverColor().GetValue();
    return true;
}

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::GetPasswordIconPressColor(uint32_t& color)
{
    auto theme = GetTextFieldTheme();
    CHECK_NULL_RETURN(theme, false);
    color = theme->GetPressColor().GetValue();
    return true;
}

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::SetPasswordIconHoverColor(
    const std::vector<RoundRect>& rects, uint32_t color)
{
    auto host = static_cast<Derived*>(this)->GetHost();
    CHECK_NULL_RETURN(host, false);
    CHECK_NULL_RETURN(SetHoverColorAndRectsOnModifier(rects, color), false);
    host->MarkDirtyNode(GetHoverDirtyFlag());
    return true;
}

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::ClearPasswordIconHoverColor()
{
    auto host = static_cast<Derived*>(this)->GetHost();
    CHECK_NULL_RETURN(host, false);
    CHECK_NULL_RETURN(ClearHoverColorAndRectsOnModifier(), false);
    host->MarkDirtyNode(GetHoverDirtyFlag());
    return true;
}

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::IsShowPasswordSymbol() const
{
    return isPasswordSymbol_ &&
        AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_THIRTEEN);
}

template<typename Derived, typename LayoutPropertyT>
TextInputType PasswordIconHostBase<Derived, LayoutPropertyT>::GetTextInputType() const
{
    auto prop = static_cast<const Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>();
    return prop ? prop->GetTextInputTypeValue(TextInputType::UNSPECIFIED) : TextInputType::UNSPECIFIED;
}

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::IsOneTimeCodeType() const
{
    return IsOneTimeCodeInputType(GetTextInputType());
}

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::IsInPasswordMode() const
{
    return IsPasswordInputType(GetTextInputType());
}

template<typename Derived, typename LayoutPropertyT>
void PasswordIconHostBase<Derived, LayoutPropertyT>::UpdateObscure(const std::u16string& insertValue, bool hasInsert)
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

template<typename Derived, typename LayoutPropertyT>
int32_t PasswordIconHostBase<Derived, LayoutPropertyT>::GetNakedCharPosition() const
{
    if (!TextInputObscureUtils::ShouldRevealNakedChar(IsSingleLineForPassword(), IsInPasswordMode(),
            GetTextObscured(), obscureHelper_.GetTickCountDown())) {
        return -1;
    }
    CHECK_NULL_RETURN(CheckLayoutProperty(), -1);
    return obscureHelper_.GetNakedCharPosition(HasObscureContent());
}

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::IsPasswordObscured() const
{
    return IsSingleLineForPassword() && IsInPasswordMode() && GetTextObscured();
}

template<typename Derived, typename LayoutPropertyT>
void PasswordIconHostBase<Derived, LayoutPropertyT>::CheckAndUpdateInputTypeForOtp()
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

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::DoRequestKeyboardNotByFocusSwitch(SourceType sourceType)
{
    CHECK_NULL_RETURN(DoCallRequestKeyboard(sourceType), false);
    auto pipeline = static_cast<Derived*>(this)->GetContext();
    if (pipeline) {
        const auto& textFieldManager = pipeline->GetTextFieldManager();
        if (textFieldManager) {
            textFieldManager->SetNeedToRequestKeyboard(false);
        }
    }
    return true;
}

template<typename Derived, typename LayoutPropertyT>
void PasswordIconHostBase<Derived, LayoutPropertyT>::CheckPasswordAreaState()
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

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::ResetObscureTickCountDown(PropertyChangeFlag flag)
{
    CHECK_NULL_RETURN(IsPasswordObscured(), false);
    bool changed = obscureHelper_.ResetTickCountDown();
    if (changed) {
        OnObscureDirty(flag);
    }
    return changed;
}

template<typename Derived, typename LayoutPropertyT>
void PasswordIconHostBase<Derived, LayoutPropertyT>::TickDownPasswordObscure()
{
    if (!IsPasswordObscured() || obscureHelper_.GetTickCountDown() <= 0) {
        return;
    }
    obscureHelper_.TickDown();
    if (obscureHelper_.GetTickCountDown() == 0) {
        OnObscureDirty(PROPERTY_UPDATE_MEASURE);
    }
}

template<typename Derived, typename LayoutPropertyT>
void PasswordIconHostBase<Derived, LayoutPropertyT>::CheckIfNeedToResetKeyboard()
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

template<typename Derived, typename LayoutPropertyT>
void PasswordIconHostBase<Derived, LayoutPropertyT>::DoResetKeyboardCommon(const RefPtr<FrameNode>& host,
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

template<typename Derived, typename LayoutPropertyT>
TextCapability PasswordIconHostBase<Derived, LayoutPropertyT>::GetDisabledCapabilities() const
{
    return IsInPasswordMode() ? GetPasswordDisabledCapabilities() : TextCapability::NONE;
}

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::IsCapabilityAllowed(TextCapability cap) const
{
    return OHOS::Ace::IsCapabilityAllowed(GetDisabledCapabilities(), cap);
}

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::IsCapabilityDisabled(TextCapability cap) const
{
    return OHOS::Ace::IsCapabilityDisabled(GetDisabledCapabilities(), cap);
}

template<typename Derived, typename LayoutPropertyT>
std::string PasswordIconHostBase<Derived, LayoutPropertyT>::GetPasswordIconPromptInformation(bool show)
{
    auto theme = GetTextFieldTheme();
    CHECK_NULL_RETURN(theme, "");
    return show ? theme->GetShowPasswordPromptInformation()
                : theme->GetHiddenPasswordPromptInformation();
}

template<typename Derived, typename LayoutPropertyT>
void PasswordIconHostBase<Derived, LayoutPropertyT>::SetAccessibilityPasswordIconAction()
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

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::ProcessPasswordArea()
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

template<typename Derived, typename LayoutPropertyT>
void PasswordIconHostBase<Derived, LayoutPropertyT>::RequestKeyboardForOTP()
{
    auto host = static_cast<Derived*>(this)->GetHost();
    CHECK_NULL_VOID(host);
    TAG_LOGI(GetLogTag(), "%{public}d requestKB, reason: RESET_KEYBOARD (OTP)", host->GetId());
    DoRequestKeyboardNotByFocusSwitch(SourceType::NONE);
}

template<typename Derived, typename LayoutPropertyT>
bool PasswordIconHostBase<Derived, LayoutPropertyT>::DoCallRequestKeyboard(SourceType sourceType)
{
    return static_cast<Derived*>(this)->RequestKeyboard(false, true, true, sourceType);
}

template<typename Derived, typename LayoutPropertyT>
void PasswordIconHostBase<Derived, LayoutPropertyT>::DoKeyboardResetPlatformCommon(bool needToResetKeyboard)
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

template<typename Derived, typename LayoutPropertyT>
void PasswordIconHostBase<Derived, LayoutPropertyT>::FireSecurityStateChanged(bool isSecure)
{
    auto hub = static_cast<Derived*>(this)->GetSecurityEventHubImpl();
    CHECK_NULL_VOID(hub);
    hub->FireOnSecurityStateChanged(isSecure);
}

// Explicit instantiation — skipped when this file is included from
// another translation unit (e.g. rich_editor) to avoid duplicate symbols.
#ifndef PASSWORD_ICON_HOST_INCLUDED
template class PasswordIconHostBase<TextFieldPattern, TextFieldLayoutProperty>;
#endif

} // namespace OHOS::Ace::NG
