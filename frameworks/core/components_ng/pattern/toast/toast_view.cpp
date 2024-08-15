/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "core/components_ng/pattern/toast/toast_view.h"

#include "base/geometry/dimension.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/toast/toast_theme.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/toast/toast_layout_property.h"
#include "core/components_ng/pattern/toast/toast_pattern.h"
#include "core/components_ng/property/property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {
RefPtr<FrameNode> ToastView::CreateToastNode(const ToastInfo& toastInfo)
{
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(context, nullptr);
    auto toastTheme = context->GetTheme<ToastTheme>();
    CHECK_NULL_RETURN(toastTheme, nullptr);

    auto textId = ElementRegister::GetInstance()->MakeUniqueId();
    auto toastId = ElementRegister::GetInstance()->MakeUniqueId();
    // make toast node
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::TOAST_ETS_TAG, toastId);
    auto toastNode = FrameNode::CreateFrameNode(V2::TOAST_ETS_TAG, toastId, AceType::MakeRefPtr<ToastPattern>());
    CHECK_NULL_RETURN(toastNode, nullptr);
    auto toastProperty = toastNode->GetLayoutProperty<ToastLayoutProperty>();
    CHECK_NULL_RETURN(toastProperty, nullptr);
    auto toastContext = toastNode->GetRenderContext();
    CHECK_NULL_RETURN(toastContext, nullptr);
    auto toastAccessibilityProperty = toastNode->GetAccessibilityProperty<AccessibilityProperty>();
    CHECK_NULL_RETURN(toastAccessibilityProperty, nullptr);
    toastAccessibilityProperty->SetText(toastInfo.message);
    // create text in toast
    auto textNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, textId, AceType::MakeRefPtr<TextPattern>());
    CHECK_NULL_RETURN(textNode, nullptr);
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProperty, nullptr);
    auto pattern = toastNode->GetPattern<ToastPattern>();
    CHECK_NULL_RETURN(pattern, nullptr);
    pattern->SetToastInfo(toastInfo);
    pattern->SetTextNode(textNode);
    UpdateTextLayoutProperty(textNode, toastInfo.message, toastInfo.isRightToLeft);
    UpdateTextContext(textNode);
    textNode->MountToParent(toastNode);
    auto align = Alignment::ParseAlignment(toastInfo.alignment);
    if (align.has_value()) {
        toastProperty->UpdateToastAlignment(align.value());
    } else {
        toastProperty->ResetToastAlignment();
    }
    if (toastInfo.offset.has_value()) {
        toastProperty->UpdateToastOffset(toastInfo.offset.value());
    } else {
        toastProperty->ResetToastOffset();
    }
    toastProperty->UpdateBottom(
        StringUtils::StringToDimensionWithThemeValue(toastInfo.bottom, true, toastTheme->GetBottom()));
    toastProperty->UpdateShowMode(toastInfo.showMode);
    toastNode->GetEventHub<EventHub>()->GetOrCreateGestureEventHub()->SetHitTestMode(HitTestMode::HTMTRANSPARENT);
    toastNode->MarkModifyDone();
    return toastNode;
}

void ToastView::UpdateTextLayoutProperty(
    const RefPtr<FrameNode>& textNode, const std::string& message, bool isRightToLeft)
{
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto toastTheme = context->GetTheme<ToastTheme>();
    CHECK_NULL_VOID(toastTheme);
    auto fontSize = toastTheme->GetTextStyle().GetFontSize();
    auto padding = toastTheme->GetPadding();
    auto fontWeight = toastTheme->GetTextStyle().GetFontWeight();
    auto textColor = toastTheme->GetTextStyle().GetTextColor();
    PaddingProperty paddings;
    paddings.top = NG::CalcLength(padding.Top());
    paddings.bottom = NG::CalcLength(padding.Bottom());
    paddings.left = NG::CalcLength(padding.Left());
    paddings.right = NG::CalcLength(padding.Right());

    textLayoutProperty->UpdateContent(message);
    textLayoutProperty->UpdateTextAlign(TextAlign::CENTER);
    textLayoutProperty->UpdateFontSize(fontSize);
    textLayoutProperty->UpdateLayoutDirection((isRightToLeft ? TextDirection::RTL : TextDirection::LTR));
    textLayoutProperty->UpdatePadding(paddings);
    textLayoutProperty->UpdateTextColor(textColor);
    textLayoutProperty->UpdateFontWeight(fontWeight);

    if (Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
        textLayoutProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
        textLayoutProperty->UpdateEllipsisMode(EllipsisMode::TAIL);
    }

    UpdateTextLayoutBorderShadowProperty(textNode);
}

void ToastView::UpdateTextLayoutBorderShadowProperty(
    const RefPtr<FrameNode>& textNode)
{
    auto textLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    auto context = PipelineBase::GetCurrentContextSafely();
    CHECK_NULL_VOID(context);
    auto toastTheme = context->GetTheme<ToastTheme>();
    CHECK_NULL_VOID(toastTheme);

    Color color = toastTheme->GetBorderColor();
    Dimension width = toastTheme->GetBorderWidth();
    BorderColorProperty borderColor;
    BorderWidthProperty borderWidth;

    borderColor.SetColor(color);
    borderWidth.SetBorderWidth(width);

    auto renderContext = textNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    if (!textLayoutProperty->GetBorderWidthProperty()) {
        if (!renderContext->HasBorderWidth()) {
            textLayoutProperty->UpdateBorderWidth(borderWidth);
            renderContext->UpdateBorderWidth(borderWidth);
        }
        if (!renderContext->HasBorderColor()) {
            renderContext->UpdateBorderColor(borderColor);
        }
    }

    auto && graphics = renderContext->GetOrCreateGraphics();
    CHECK_NULL_VOID(graphics);
    if (!graphics->HasBackShadow()) {
        ShadowStyle shadowStyle = static_cast<ShadowStyle>(toastTheme->GetShadowNormal());
        Shadow shadow = Shadow::CreateShadow(shadowStyle);
        renderContext->UpdateBackShadow(shadow);
    }
}

void ToastView::UpdateTextContext(const RefPtr<FrameNode>& textNode)
{
    auto textContext = textNode->GetRenderContext();
    CHECK_NULL_VOID(textContext);
    auto pipelineContext = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto toastTheme = pipelineContext->GetTheme<ToastTheme>();
    CHECK_NULL_VOID(toastTheme);
    auto radius = toastTheme->GetRadius();
    BorderRadiusProperty borderRadius;
    borderRadius.SetRadius(Dimension(radius.GetX().ConvertToPx()));
    textContext->UpdateBorderRadius(borderRadius);
    textContext->UpdateBackShadow(ShadowConfig::DefaultShadowL);
    textContext->UpdateClipEdge(false);

    if (Container::GreatOrEqualAPITargetVersion(PlatformVersion::VERSION_TWELVE)) {
        auto defaultBGcolor = toastTheme->GetDefaultBGColor();
        textContext->UpdateBackgroundColor(defaultBGcolor);

        BlurStyleOption styleOption;
        styleOption.blurStyle = BlurStyle::COMPONENT_ULTRA_THICK;
        styleOption.colorMode = static_cast<ThemeColorMode>(toastTheme->GetBgThemeColorMode());
        textContext->UpdateBackBlurStyle(styleOption);
    } else {
        auto toastBackgroundColor = toastTheme->GetBackgroundColor();
        textContext->UpdateBackgroundColor(toastBackgroundColor);
    }
}
} // namespace OHOS::Ace::NG
