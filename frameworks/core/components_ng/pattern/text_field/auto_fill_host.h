/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_AUTO_FILL_HOST_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_AUTO_FILL_HOST_H

#include <string>

#include "base/geometry/ng/rect_t.h"
#include "base/memory/ace_type.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/render/paragraph.h"

namespace OHOS::Ace {
class TextFieldTheme;
enum class AceAutoFillType;
} // namespace OHOS::Ace

namespace OHOS::Ace::NG {
class FrameNode;
class PipelineContext;
class AutoFillController; // forward declaration — full definition in auto_fill_controller.h
class IAutoFillHost; // forward declaration — defined below

// Free function that constructs an AutoFillController bound to the given host.
// Declared here (forward) so AutoFillHostBase can call it without including
// auto_fill_controller.h (which would create a circular include).
// Defined in auto_fill_controller.cpp where AutoFillController is fully visible.
RefPtr<AutoFillController> CreateAutoFillControllerForHost(IAutoFillHost* host);

// Free function implementing the shared BeforeAutoFillAnimation flow.
// Uses only IAutoFillHost interface methods + AutoFillController.
// Defined in auto_fill_controller.cpp where all dependencies are available.
void ExecuteBeforeAutoFillAnimation(IAutoFillHost* host, const std::u16string& content, const AceAutoFillType& type);

// Interface for content modifiers that support AutoFill animation properties.
// Inherits virtual AceType so DynamicCast<IAutoFillContentModifier> works from RefPtr<ContentModifier>.
class IAutoFillContentModifier : public virtual AceType {
    DECLARE_ACE_TYPE(IAutoFillContentModifier, AceType);

public:
    virtual ~IAutoFillContentModifier() = default;
    virtual void SetAutoFillTranslationOffset(float value) = 0;
    virtual void SetAutoFillTextScrollOffset(float value) = 0;
    virtual void SetAutoFillEmphasizeCharIndex(float value) = 0;
    virtual void SetAutoFillDefaultCharIndex(float value) = 0;
    virtual void SetAutoFillOriginTextColor(const Color& value) = 0;
    virtual void ResetAutoFillProperties() = 0;
};

// Interface for patterns that host AutoFill animation.
// Inherits virtual AceType so DynamicCast<IAutoFillHost> works from RefPtr<Pattern>.
class IAutoFillHost : public virtual AceType {
    DECLARE_ACE_TYPE(IAutoFillHost, AceType);

public:
    // --- Methods with common implementations (provided by AutoFillHostBase) ---
    virtual RefPtr<FrameNode> GetAutoFillHostNode() = 0;
    virtual RectF GetAutoFillTextContentRect() = 0;
    virtual RectF GetAutoFillTextRect() = 0;
    virtual TextDirection GetAutoFillTextDirection() = 0;
    virtual void StopAutoFillTwinkling() = 0;
    virtual void StartAutoFillTwinkling() = 0;
    virtual RefPtr<PipelineContext> GetAutoFillContext() = 0;

    // Hook: paragraph direction for TextDirection::AUTO fallback.
    // Called by AutoFillHostBase::GetAutoFillTextDirection when layout direction is AUTO.
    virtual TextDirection GetAutoFillParagraphDirection(TextDirection fallback) { return fallback; }

    // --- Component-specific methods (implemented by each pattern) ---
    // Default implementations return nullptr/fallback for the case when
    // ENABLE_AUTO_FILL_CONTROLLER is not defined. Subclasses override these
    // under #ifdef ENABLE_AUTO_FILL_CONTROLLER to provide real implementations.
    virtual RefPtr<Paragraph> GetAutoFillParagraph() { return nullptr; }
    virtual IAutoFillContentModifier* GetAutoFillContentModifier() { return nullptr; }
    virtual RefPtr<TextFieldTheme> GetAutoFillTheme() { return nullptr; }

    // --- Hooks for BeforeAutoFillAnimation (called by AutoFillHostBase) ---
    // Prepare/filter the text value before setting it on the controller.
    // Default: return content as-is. TextField overrides to filter.
    virtual std::u16string PrepareAutoFillText(const std::u16string& content) { return content; }
    // Whether the animation feature is enabled. Default: true.
    // TextField overrides to check GetEnableAutoFillAnimationValue from layout property.
    virtual bool IsAutoFillAnimationEnabled() const { return true; }
    // Insert the final text value after animation completes (or immediately if no animation).
    virtual void ExecuteAutoFillInsert(const std::u16string& content) {}

#ifdef ENABLE_AUTO_FILL_CONTROLLER
    // Get or create the AutoFillController for this host.
    // Implemented by AutoFillHostBase; called by ExecuteBeforeAutoFillAnimation.
    virtual const RefPtr<AutoFillController>& GetOrCreateAutoFillController() = 0;
#endif
};

// CRTP base providing default implementations for the common IAutoFillHost methods.
// Derived must support GetHost(), GetTextContentRect(), GetTextRect(), StopTwinkling(),
// StartTwinkling(), GetContext(), and GetLayoutProperty<LayoutPropertyT>() (e.g., inherit from Pattern).
// LayoutPropertyT must provide GetNonAutoLayoutDirection() and GetTextDirectionValue().
//
// Also owns the autoFillController_ member and provides GetOrCreateAutoFillController().
//
// Usage:
//   class TextFieldPattern : public AutoFillHostBase<TextFieldPattern, TextFieldLayoutProperty> {};
//   class RichEditorPattern : public AutoFillHostBase<RichEditorPattern, RichEditorLayoutProperty> {};
template<typename Derived, typename LayoutPropertyT>
class AutoFillHostBase : public IAutoFillHost {
public:
    RefPtr<FrameNode> GetAutoFillHostNode() override
    {
        return static_cast<Derived*>(this)->GetHost();
    }

    RectF GetAutoFillTextContentRect() override
    {
        return static_cast<Derived*>(this)->GetTextContentRect();
    }

    RectF GetAutoFillTextRect() override
    {
        return static_cast<Derived*>(this)->GetTextRect();
    }

    void StopAutoFillTwinkling() override
    {
        static_cast<Derived*>(this)->StopTwinkling();
    }

    void StartAutoFillTwinkling() override
    {
        static_cast<Derived*>(this)->StartTwinkling();
    }

    RefPtr<PipelineContext> GetAutoFillContext() override
    {
        return AceType::Claim(static_cast<Derived*>(this)->GetContext());
    }

    TextDirection GetAutoFillTextDirection() override
    {
        auto layoutProperty = static_cast<Derived*>(this)->template GetLayoutProperty<LayoutPropertyT>();
        CHECK_NULL_RETURN(layoutProperty, TextDirection::LTR);
        auto direction = layoutProperty->GetNonAutoLayoutDirection();
        auto textDirection = layoutProperty->GetTextDirectionValue(TextDirection::INHERIT);
        if (textDirection == TextDirection::INHERIT) {
            return direction;
        } else if (textDirection == TextDirection::AUTO) {
            return static_cast<Derived*>(this)->GetAutoFillParagraphDirection(direction);
        } else {
            return textDirection;
        }
    }

#ifdef ENABLE_AUTO_FILL_CONTROLLER
    const RefPtr<AutoFillController>& GetOrCreateAutoFillController() override
    {
        if (!autoFillController_) {
            auto hostNode = GetAutoFillHostNode();
            if (hostNode) {
                ACE_UINODE_TRACE(hostNode);
            }
            // Use the free function to avoid needing AutoFillController's full definition here.
            autoFillController_ = CreateAutoFillControllerForHost(this);
        }
        return autoFillController_;
    }

    // Shared BeforeAutoFillAnimation — delegates to free function.
    void BeforeAutoFillAnimation(const std::u16string& content, const AceAutoFillType& type)
    {
        ExecuteBeforeAutoFillAnimation(this, content, type);
    }

protected:
    RefPtr<AutoFillController> autoFillController_;
#endif // ENABLE_AUTO_FILL_CONTROLLER
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_AUTO_FILL_HOST_H
