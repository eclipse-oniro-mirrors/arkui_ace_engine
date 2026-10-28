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
#include <string>
#include <vector>

#include "base/memory/ace_type.h"
#include "core/common/ime/password_obscure_helper.h"
#include "core/common/ime/text_input_action.h"
#include "core/common/ime/text_input_type.h"
#include "core/common/text_capability_model.h"
#include "ui/properties/dirty_flag.h"

namespace OHOS::Ace {
class ImageSourceInfo;
class TextFieldTheme;
class HoverInfo;
enum class TextDirection;
enum AceLogTag : uint8_t;
enum class SourceType : int32_t;

namespace NG {
class RoundRect;
class EventHub;
class PasswordResponseArea;
class AccessibilityProperty;
class FrameNode;
class TextFieldPattern;
class TextFieldLayoutProperty;
class RichEditorPattern;
class RichEditorLayoutProperty;

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
    RefPtr<TextFieldTheme> GetTextFieldTheme() const;

    TextDirection GetPasswordIconDirection() const override;

    bool CheckLayoutProperty() const override;

    ImageSourceInfo GetShowPasswordSourceInfo(const ImageSourceInfo& defaultInfo) override;
    ImageSourceInfo GetHidePasswordSourceInfo(const ImageSourceInfo& defaultInfo) override;

    bool IsShowPasswordIcon() const override;

    bool IsTV() const override;

    bool IsDisabled() const override;

    bool GetPasswordIconHoverColor(uint32_t& color) override;
    bool GetPasswordIconPressColor(uint32_t& color) override;

    bool SetPasswordIconHoverColor(const std::vector<RoundRect>& rects, uint32_t color) override;
    bool ClearPasswordIconHoverColor() override;

    void SetIsPasswordSymbol(bool isPasswordSymbol)
    {
        isPasswordSymbol_ = isPasswordSymbol;
    }

    bool IsShowPasswordSymbol() const override;

    TextInputType GetTextInputType() const;

    bool IsOneTimeCodeType() const;

    bool IsInPasswordMode() const override;

    virtual bool IsSingleLineForPassword() const = 0;

    void UpdateObscure(const std::u16string& insertValue, bool hasInsert);

    int32_t GetNakedCharPosition() const;

    bool IsPasswordObscured() const;

    void CheckAndUpdateInputTypeForOtp();

    bool DoRequestKeyboardNotByFocusSwitch(SourceType sourceType);

    void CheckPasswordAreaState();

    bool ResetObscureTickCountDown(PropertyChangeFlag flag = PROPERTY_UPDATE_MEASURE_SELF);

    void TickDownPasswordObscure();

    void CheckIfNeedToResetKeyboard();

    void DoResetKeyboardCommon(const RefPtr<FrameNode>& host,
        bool needToResetKeyboard, bool isDynamic);

    TextCapability GetDisabledCapabilities() const;

    bool IsCapabilityAllowed(TextCapability cap) const;
    bool IsCapabilityDisabled(TextCapability cap) const;

    std::string GetPasswordIconPromptInformation(bool show);

    void SetAccessibilityPasswordIconAction();

    bool ProcessPasswordArea();

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
    void RequestKeyboardForOTP();

    virtual AceLogTag GetLogTag() const = 0;

    bool DoCallRequestKeyboard(SourceType sourceType);

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

    void DoKeyboardResetPlatformCommon(bool needToResetKeyboard);

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
    void FireSecurityStateChanged(bool isSecure);

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

// Suppress implicit instantiation — method bodies are in password_icon_host_base.cpp
extern template class PasswordIconHostBase<TextFieldPattern, TextFieldLayoutProperty>;
extern template class PasswordIconHostBase<RichEditorPattern, RichEditorLayoutProperty>;

} // namespace OHOS::Ace::NG
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_PASSWORD_ICON_HOST_H
