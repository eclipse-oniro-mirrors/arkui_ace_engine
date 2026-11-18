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
#include "core/components_ng/pattern/text_field/auto_fill_controller.h"

#include <algorithm>
#include <cstdint>
#include <string>

#include "base/utils/string_utils.h"
#include "base/utils/utf_helper.h"
#include "base/utils/utils.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/render/drawing.h"
#include "core/components/text_field/textfield_theme.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "base/view_data/ace_auto_fill_type.h"

namespace OHOS::Ace::NG {

// Free function bridging AutoFillHostBase (header-only template) and AutoFillController.
RefPtr<AutoFillController> CreateAutoFillControllerForHost(IAutoFillHost* host)
{
    return AceType::MakeRefPtr<AutoFillController>(AceType::WeakClaim(host));
}

// Shared BeforeAutoFillAnimation flow using only IAutoFillHost interface + AutoFillController.
void ExecuteBeforeAutoFillAnimation(IAutoFillHost* host, const std::u16string& content, const AceAutoFillType& type)
{
    CHECK_NULL_VOID(host);
    auto hostNode = host->GetAutoFillHostNode();
    CHECK_NULL_VOID(hostNode);
    auto textValue = host->PrepareAutoFillText(content);
    auto& controller = host->GetOrCreateAutoFillController();
    CHECK_NULL_VOID(controller);
    controller->SetAutoFillTextUtf16Value(textValue);
    auto onFinishCallback = [weak = AceType::WeakClaim(host), unFilteredValue = content]() {
        auto host = weak.Upgrade();
        CHECK_NULL_VOID(host);
        auto& controller = host->GetOrCreateAutoFillController();
        CHECK_NULL_VOID(controller);
        controller->ResetAutoFillAnimationStatus();
        auto node = host->GetAutoFillHostNode();
        CHECK_NULL_VOID(node);
        host->ExecuteAutoFillInsert(unFilteredValue);
        node->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    };
    auto pipeline = PipelineContext::GetCurrentContextSafelyWithCheck();
    auto needsAnimation = pipeline && host->IsAutoFillAnimationEnabled() &&
        (type == AceAutoFillType::ACE_NEW_PASSWORD || type == AceAutoFillType::ACE_PASSWORD) &&
        textValue.length() > 0;
    if (needsAnimation) {
        controller->SetAutoFillAnimationStatus(AutoFillAnimationStatus::SHOW_ICON);
        hostNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        pipeline->AddAfterLayoutTask(
            [weak = AceType::WeakClaim(host), onFinish = std::move(onFinishCallback), textValue]() {
                auto host = weak.Upgrade();
                CHECK_NULL_VOID(host);
                auto& controller = host->GetOrCreateAutoFillController();
                CHECK_NULL_VOID(controller);
                controller->StartAutoFillAnimation(onFinish, textValue);
            });
    } else {
        onFinishCallback();
    }
}

namespace {
constexpr int32_t AUTO_FILL_ICON_ANIMATION_DURATION = 250;
constexpr float AUTO_FILL_SPRING_RESPONSE = 0.313f;
constexpr float AUTO_FILL_SPRING_DAMPING_FRACTION = 0.94f;
constexpr float AUTO_FILL_SPRING_RESPONSE_EXTRA_LONG = 0.413f;
constexpr float AUTO_FILL_SPRING_DAMPING_FRACTION_EXTRA_LONG = 0.93f;
constexpr uint32_t AUTO_FILL_SYMBOL_RENDERING_STRATEGY = 1;
constexpr float AUTO_FILL_ICON_INIT_SCALE = 0.2f;
constexpr int32_t AUTO_FILL_TEXT_SCROLL_DELAY_DURATION = 500;
constexpr int32_t AUTO_FILL_DEFAULT_CHAR_DELAY_DURATION = 150;
constexpr int32_t AUTO_FILL_ICON_HIDE_DELAY_DURATION_SHORT = 500;
constexpr int32_t AUTO_FILL_ICON_HIDE_DELAY_DURATION_MEDIUM = 600;
constexpr int32_t AUTO_FILL_ICON_HIDE_DELAY_DURATION_LONG = 700;
constexpr int32_t AUTO_FILL_ICON_HIDE_DELAY_DURATION_EXTRA_LONG = 1100;
} // namespace

void AutoFillController::StartAutoFillAnimation(
    const std::function<void()>& onFinishCallback, const std::u16string& content)
{
    auto host = host_.Upgrade();
    CHECK_NULL_VOID(host);
    auto hostNode = host->GetAutoFillHostNode();
    if (hostNode) {
        ACE_UINODE_TRACE(hostNode);
    }
    auto initParagraphSuc = InitAutoFillParagraph(content);
    auto createAutoFillIconSuc = CreateAutoFillIcon();
    if (!initParagraphSuc || !createAutoFillIconSuc) {
        if (onFinishCallback) {
            onFinishCallback();
        }
        ResetAutoFillAnimationStatus();
        return;
    }
    AutoFillContentLengthMode mode = GetAutoFillContentLengthMode();
    if (mode == AutoFillContentLengthMode::INVALID) {
        if (onFinishCallback) {
            onFinishCallback();
        }
        ResetAutoFillAnimationStatus();
        return;
    }
    host->StopAutoFillTwinkling();
    UpdateAnimationTextRect();
    if (mode == AutoFillContentLengthMode::EXTRA_LONG) {
        PlayAutoFillIconShowAnimation(mode);
        PlayAutoFillTextScrollAnimation();
        PlayAutoFillIconHideAnimation(onFinishCallback, mode);
    } else {
        PlayAutoFillIconShowAnimation(mode);
        PlayAutoFillIconHideAnimation(onFinishCallback, mode);
    }
}

AutoFillContentLengthMode AutoFillController::GetAutoFillContentLengthMode()
{
    auto host = host_.Upgrade();
    CHECK_NULL_RETURN(host, AutoFillContentLengthMode::INVALID);
    CHECK_NULL_RETURN(autoFillParagraph_, AutoFillContentLengthMode::INVALID);

    float autoFillParagraphWidth = std::max(autoFillParagraph_->GetLongestLine(), 0.0f);
    auto contentRect = host->GetAutoFillTextContentRect();
    float textFieldContentWidth = std::max(contentRect.Width(), 0.0f);
    if (LessOrEqual(autoFillParagraphWidth, 0.0f) || LessOrEqual(textFieldContentWidth, 0.0f)) {
        return AutoFillContentLengthMode::INVALID;
    }

    if (GreatNotEqual(autoFillParagraphWidth, textFieldContentWidth)) {
        return AutoFillContentLengthMode::EXTRA_LONG;
    }

    auto shortContentLength = textFieldContentWidth * 1.0f / 3.0f;
    auto middleContentLength = textFieldContentWidth * 2.0f / 3.0f;
    if (LessOrEqual(autoFillParagraphWidth, shortContentLength)) {
        return AutoFillContentLengthMode::SHORT;
    }

    if (LessOrEqual(autoFillParagraphWidth, middleContentLength)) {
        return AutoFillContentLengthMode::MEDIUM;
    }

    return AutoFillContentLengthMode::LONG;
}

bool AutoFillController::InitAutoFillParagraph(const std::u16string& content)
{
    auto host = host_.Upgrade();
    CHECK_NULL_RETURN(host, false);
    autoFillParagraph_ = host->GetAutoFillParagraph();
    if (!autoFillParagraph_) {
        return false;
    }
    auto* contentModifier = host->GetAutoFillContentModifier();
    if (!contentModifier) {
        return false;
    }
    contentModifier->SetAutoFillOriginTextColor(autoFillOriginTextColor_);
    return true;
}

void AutoFillController::PlayAutoFillIconShowAnimation(const AutoFillContentLengthMode& mode)
{
    auto symbolNode = autoFillIconNode_.Upgrade();
    CHECK_NULL_VOID(symbolNode);
    auto symbolRenderContext = symbolNode->GetRenderContext();
    CHECK_NULL_VOID(symbolRenderContext);
    AnimationOption option = AnimationOption();
    option.SetDuration(AUTO_FILL_ICON_ANIMATION_DURATION);
    option.SetCurve(Curves::FRICTION);
    AnimationUtils::Animate(
        option,
        [symbolRenderContext]() {
            symbolRenderContext->UpdateOpacity(1.0);
            symbolRenderContext->UpdateTransformScale({ 1.0, 1.0 });
        },
        [weak = AceType::WeakClaim(this), mode]() {
            auto autofillController = weak.Upgrade();
            CHECK_NULL_VOID(autofillController);
            autofillController->PlayAutoFillTranslationAnimation(mode);
            autofillController->PlayAutoFillDefaultCharAnimation(mode);
        }, nullptr, symbolNode->GetContextRefPtr());
}

void AutoFillController::PlayAutoFillDefaultCharAnimation(const AutoFillContentLengthMode& mode)
{
    if (!autoFillParagraph_) {
        ResetAutoFillAnimationStatus();
        return;
    }
    auto host = host_.Upgrade();
    CHECK_NULL_VOID(host);
    auto hostNode = host->GetAutoFillHostNode();
    if (hostNode) {
        ACE_UINODE_TRACE(hostNode);
    }
    auto contentLength = autoFillParagraph_->GetParagraphText().length();
    auto response = GetSpringAnimationResponse(mode);
    auto damping = GetSpringAnimationDamping(mode);
    auto motion = AceType::MakeRefPtr<ResponsiveSpringMotion>(response, damping);
    AnimationOption option;
    option.SetCurve(motion);
    option.SetDelay(AUTO_FILL_DEFAULT_CHAR_DELAY_DURATION);
    auto pipelineContext = host->GetAutoFillContext();
    AnimationUtils::Animate(option, [weak = AceType::WeakClaim(this), contentLength]() {
        auto autofillController = weak.Upgrade();
        CHECK_NULL_VOID(autofillController);
        auto host = autofillController->host_.Upgrade();
        CHECK_NULL_VOID(host);
        auto* contentModifier = host->GetAutoFillContentModifier();
        CHECK_NULL_VOID(contentModifier);
        autofillController->autoFillAnimationStatus_ = AutoFillAnimationStatus::TRANSLATION;
        contentModifier->SetAutoFillDefaultCharIndex(std::max(contentLength - 1.0f, 0.0f));
    }, nullptr, nullptr, Claim(AceType::RawPtr(pipelineContext)));
}

void AutoFillController::PlayAutoFillTranslationAnimation(const AutoFillContentLengthMode& mode)
{
    if (!autoFillParagraph_) {
        ResetAutoFillAnimationStatus();
        return;
    }
    auto host = host_.Upgrade();
    CHECK_NULL_VOID(host);
    auto hostNode = host->GetAutoFillHostNode();
    if (hostNode) {
        ACE_UINODE_TRACE(hostNode);
    }
    auto symbolNode = autoFillIconNode_.Upgrade();
    CHECK_NULL_VOID(symbolNode);
    auto symbolRenderContext = symbolNode->GetRenderContext();
    CHECK_NULL_VOID(symbolRenderContext);
    float autoFillParagraphWidth = std::max(autoFillParagraph_->GetLongestLine(), 0.0f);
    auto contentRect = host->GetAutoFillTextContentRect();
    float textFieldContentWidth = std::max(contentRect.Width(), 0.0f);
    auto theme = host->GetAutoFillTheme();
    CHECK_NULL_VOID(theme);
    auto iconWidth = static_cast<float>(theme->GetAutoFillIconSize().ConvertToPx());
    auto isRTL = host->GetAutoFillTextDirection() == TextDirection::RTL;
    // In LTR, autoFillFirstCharOffset_ includes contentRect.GetX() (the content
    // area's left offset within the host node). The upper bound for the icon's
    // travel must also include this offset so the icon stays within the content
    // rect's right edge: contentRect.GetX() + contentRect.Width() - iconWidth.
    // In RTL the offset is relative to the right edge and does not include
    // contentRect.GetX(), so the bound is just contentRect.Width() - iconWidth.
    float maxOffset = isRTL ? (textFieldContentWidth - iconWidth)
                            : (textFieldContentWidth + contentRect.GetX() - iconWidth);
    auto translationOffset = std::min(autoFillParagraphWidth + autoFillFirstCharOffset_, maxOffset);
    translationOffset = isRTL ? -translationOffset : translationOffset;
    auto contentLength = autoFillParagraph_->GetParagraphText().length();
    auto response = GetSpringAnimationResponse(mode);
    auto damping = GetSpringAnimationDamping(mode);
    auto motion = AceType::MakeRefPtr<ResponsiveSpringMotion>(response, damping);
    AnimationOption option;
    option.SetCurve(motion);
    auto pipelineContext = host->GetAutoFillContext();
    AnimationUtils::Animate(option, [weak = AceType::WeakClaim(this), translationOffset,
                                        symbolRenderContext, contentLength]() {
        auto autofillController = weak.Upgrade();
        CHECK_NULL_VOID(autofillController);
        auto host = autofillController->host_.Upgrade();
        CHECK_NULL_VOID(host);
        auto* contentModifier = host->GetAutoFillContentModifier();
        CHECK_NULL_VOID(contentModifier);
        autofillController->autoFillAnimationStatus_ = AutoFillAnimationStatus::TRANSLATION;
        contentModifier->SetAutoFillEmphasizeCharIndex(std::max(contentLength - 1.0f, 0.0f));
        contentModifier->SetAutoFillTranslationOffset(translationOffset);
        symbolRenderContext->UpdateTransformTranslate({ translationOffset,
            autofillController->autoFillIconYOffset_, 0.0f });
    }, nullptr, nullptr, Claim(AceType::RawPtr(pipelineContext)));
}

void AutoFillController::PlayAutoFillTextScrollAnimation()
{
    if (!autoFillParagraph_) {
        ResetAutoFillAnimationStatus();
        return;
    }
    auto host = host_.Upgrade();
    CHECK_NULL_VOID(host);
    auto isRTL = host->GetAutoFillTextDirection() == TextDirection::RTL;
    float autoFillParagraphWidth = std::max(autoFillParagraph_->GetLongestLine(), 0.0f);
    auto contentRect = host->GetAutoFillTextContentRect();
    float textFieldContentWidth = std::max(contentRect.Width(), 0.0f);
    auto startScrollOffset = isRTL ? autoFillParagraphWidth - textFieldContentWidth : 0;
    auto endScrollOffset = isRTL ? 0 : autoFillParagraphWidth - textFieldContentWidth;
    auto* contentModifier = host->GetAutoFillContentModifier();
    CHECK_NULL_VOID(contentModifier);
    contentModifier->SetAutoFillTextScrollOffset(startScrollOffset);
    auto motion = AceType::MakeRefPtr<ResponsiveSpringMotion>(
        AUTO_FILL_SPRING_RESPONSE_EXTRA_LONG, AUTO_FILL_SPRING_DAMPING_FRACTION_EXTRA_LONG);
    AnimationOption option;
    option.SetCurve(motion);
    option.SetDelay(AUTO_FILL_TEXT_SCROLL_DELAY_DURATION);
    auto pipelineContext = host->GetAutoFillContext();
    AnimationUtils::Animate(option, [weak = AceType::WeakClaim(this), endScrollOffset]() {
        auto autofillController = weak.Upgrade();
        CHECK_NULL_VOID(autofillController);
        auto host = autofillController->host_.Upgrade();
        CHECK_NULL_VOID(host);
        auto* contentModifier = host->GetAutoFillContentModifier();
        CHECK_NULL_VOID(contentModifier);
        autofillController->autoFillAnimationStatus_ = AutoFillAnimationStatus::TRANSLATION;
        contentModifier->SetAutoFillTextScrollOffset(endScrollOffset);
    }, nullptr, nullptr, Claim(AceType::RawPtr(pipelineContext)));
}

void AutoFillController::PlayAutoFillIconHideAnimation(
    const std::function<void()>& onFinishCallback, const AutoFillContentLengthMode& mode)
{
    auto symbolNode = autoFillIconNode_.Upgrade();
    if (!symbolNode) {
        ResetAutoFillAnimationStatus();
        return;
    }
    auto symbolRenderContext = symbolNode->GetRenderContext();
    if (!symbolRenderContext) {
        ResetAutoFillAnimationStatus();
        return;
    }
    auto delayDuration = AUTO_FILL_ICON_HIDE_DELAY_DURATION_SHORT;
    if (mode == AutoFillContentLengthMode::MEDIUM) {
        delayDuration = AUTO_FILL_ICON_HIDE_DELAY_DURATION_MEDIUM;
    } else if (mode == AutoFillContentLengthMode::LONG) {
        delayDuration = AUTO_FILL_ICON_HIDE_DELAY_DURATION_LONG;
    } else if (mode == AutoFillContentLengthMode::EXTRA_LONG) {
        delayDuration = AUTO_FILL_ICON_HIDE_DELAY_DURATION_EXTRA_LONG;
    }

    AnimationOption option = AnimationOption();
    option.SetDuration(AUTO_FILL_ICON_ANIMATION_DURATION);
    option.SetCurve(Curves::FRICTION);
    option.SetDelay(delayDuration);
    AnimationUtils::Animate(
        option,
        [weak = AceType::WeakClaim(this), symbolRenderContext]() {
            auto autofillController = weak.Upgrade();
            CHECK_NULL_VOID(autofillController);
            autofillController->autoFillAnimationStatus_ = AutoFillAnimationStatus::HIDE_ICON;
            symbolRenderContext->UpdateOpacity(0.0);
            symbolRenderContext->UpdateTransformScale({ AUTO_FILL_ICON_INIT_SCALE, AUTO_FILL_ICON_INIT_SCALE });
        },
        [weak = AceType::WeakClaim(this), onFinish = std::move(onFinishCallback)]() {
            auto autofillController = weak.Upgrade();
            CHECK_NULL_VOID(autofillController);
            auto host = autofillController->host_.Upgrade();
            CHECK_NULL_VOID(host);
            autofillController->ResetAutoFillAnimationStatus();
            autofillController->ResetAutoFillIcon();
            host->StartAutoFillTwinkling();
            if (onFinish) {
                onFinish();
            }
        }, nullptr, symbolNode->GetContextRefPtr());
}

bool AutoFillController::CreateAutoFillIcon()
{
    auto host = host_.Upgrade();
    CHECK_NULL_RETURN(host, false);
    auto hostNode = host->GetAutoFillHostNode();
    CHECK_NULL_RETURN(hostNode, false);
    auto symbolNode = FrameNode::GetOrCreateFrameNode(V2::SYMBOL_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextPattern>(); });
    CHECK_NULL_RETURN(symbolNode, false);
    autoFillIconNode_ = WeakClaim(RawPtr(symbolNode));
    auto symbolProperty = symbolNode->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(symbolProperty, false);
    auto theme = host->GetAutoFillTheme();
    CHECK_NULL_RETURN(theme, false);
    symbolProperty->UpdateSymbolSourceInfo(SymbolSourceInfo(theme->GetAutoFillSymbolId()));
    symbolProperty->UpdateFontSize(theme->GetAutoFillIconSize());
    symbolProperty->UpdateSymbolColorList(
        { theme->GetAutoFillIconPrimaryColor(), theme->GetAutoFillIconEmphasizeColor() });
    symbolProperty->UpdateSymbolRenderingStrategy(AUTO_FILL_SYMBOL_RENDERING_STRATEGY);
    // Set a fixed ideal size so the layout system does not assign MATCH_PARENT
    // (which would center the icon in the content-host frame). In TextField this
    // is unnecessary because LayoutAlgorithm::Layout is a no-op; in RichEditor
    // the content-host's layout algorithm measures children with MATCH_PARENT.
    auto iconSize = theme->GetAutoFillIconSize();
    auto iconPx = static_cast<float>(iconSize.ConvertToPx());
    symbolProperty->UpdateUserDefinedIdealSize(CalcSize(CalcLength(iconPx), CalcLength(iconPx)));
    auto symbolRenderContext = symbolNode->GetRenderContext();
    CHECK_NULL_RETURN(symbolRenderContext, false);
    symbolRenderContext->UpdateOpacity(0.0);
    symbolRenderContext->UpdateTransformScale({ AUTO_FILL_ICON_INIT_SCALE, AUTO_FILL_ICON_INIT_SCALE });
    CHECK_NULL_RETURN(autoFillParagraph_, false);
    auto isRTL = host->GetAutoFillTextDirection() == TextDirection::RTL;
    auto lineMetrics = autoFillParagraph_->GetLineMetrics(0);
    auto contentRect = host->GetAutoFillTextContentRect();
    auto startOffset = static_cast<float>(lineMetrics.x);
    if (isRTL) {
        // animation from right to left
        startOffset = contentRect.Width() - (startOffset + lineMetrics.width);
        startOffset = std::max(startOffset, 0.0f);
        autoFillFirstCharOffset_ = startOffset;
        startOffset = -startOffset;
    } else {
        // In LTR, the icon must align with the first character, which sits at
        // contentRect.GetX() + lineMetrics.x in the host node's local space.
        // In single-level hosts (TextField) contentRect.GetX() is typically 0,
        // so this is equivalent to the previous lineMetrics.x. In two-level
        // hosts (RichEditor) the content rect has a non-zero X offset (padding),
        // so we must add it to position the icon correctly.
        autoFillFirstCharOffset_ = contentRect.GetX() + startOffset;
        startOffset = autoFillFirstCharOffset_;
    }
    // Align icon Y with the text content rect's Y offset so the icon vertically
    // aligns with the text being drawn. In single-level hosts (TextField) the
    // content rect Y is typically 0; in two-level hosts (RichEditor) it includes
    // the content offset within the content-host node.
    autoFillIconYOffset_ = contentRect.GetY();
    symbolRenderContext->UpdateTransformTranslate({ startOffset, autoFillIconYOffset_, 0.0f });
    symbolNode->MountToParent(hostNode);
    symbolNode->MarkModifyDone();
    symbolNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    return true;
}

void AutoFillController::ResetAutoFillIcon()
{
    auto host = host_.Upgrade();
    CHECK_NULL_VOID(host);
    auto hostNode = host->GetAutoFillHostNode();
    CHECK_NULL_VOID(hostNode);
    auto symbolNode = autoFillIconNode_.Upgrade();
    CHECK_NULL_VOID(symbolNode);
    hostNode->RemoveChild(symbolNode);
    hostNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void AutoFillController::ResetAutoFillAnimationStatus()
{
    autoFillAnimationStatus_ = AutoFillAnimationStatus::INIT;
    if (autoFillParagraph_) {
        autoFillParagraph_ = nullptr;
    }
    auto host = host_.Upgrade();
    CHECK_NULL_VOID(host);
    auto* contentModifier = host->GetAutoFillContentModifier();
    CHECK_NULL_VOID(contentModifier);
    contentModifier->ResetAutoFillProperties();
}

float AutoFillController::GetSpringAnimationResponse(const AutoFillContentLengthMode& mode)
{
    return mode == AutoFillContentLengthMode::EXTRA_LONG ? AUTO_FILL_SPRING_RESPONSE_EXTRA_LONG
                                                         : AUTO_FILL_SPRING_RESPONSE;
}

float AutoFillController::GetSpringAnimationDamping(const AutoFillContentLengthMode& mode)
{
    return mode == AutoFillContentLengthMode::EXTRA_LONG ? AUTO_FILL_SPRING_DAMPING_FRACTION_EXTRA_LONG
                                                         : AUTO_FILL_SPRING_DAMPING_FRACTION;
}

void AutoFillController::UpdateAnimationTextRect()
{
    auto host = host_.Upgrade();
    CHECK_NULL_VOID(host);
    auto textRect = host->GetAutoFillTextRect();
    auto contentRect = host->GetAutoFillTextContentRect();
    animationTextRect_ = textRect;
    animationTextRect_.SetLeft(contentRect.GetX());
}

void AutoFillController::GetFrameRectClip(RSRect& clipRect, std::vector<RSPoint>& clipRadius)
{
    auto host = host_.Upgrade();
    CHECK_NULL_VOID(host);
    auto hostNode = host->GetAutoFillHostNode();
    CHECK_NULL_VOID(hostNode);
    auto renderContext = hostNode->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto geometryNode = hostNode->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto frameSize = geometryNode->GetFrameSize();
    clipRect = RSRect(0.0f, 0.0f, frameSize.Width(), frameSize.Height());
    auto radius = renderContext->GetBorderRadius().value_or(BorderRadiusProperty());
    auto radiusTopLeft = RSPoint(static_cast<float>(radius.radiusTopLeft.value_or(0.0_vp).ConvertToPx()),
        static_cast<float>(radius.radiusTopLeft.value_or(0.0_vp).ConvertToPx()));
    clipRadius.emplace_back(radiusTopLeft);
    auto radiusTopRight = RSPoint(static_cast<float>(radius.radiusTopRight.value_or(0.0_vp).ConvertToPx()),
        static_cast<float>(radius.radiusTopRight.value_or(0.0_vp).ConvertToPx()));
    clipRadius.emplace_back(radiusTopRight);
    auto radiusBottomRight = RSPoint(static_cast<float>(radius.radiusBottomRight.value_or(0.0_vp).ConvertToPx()),
        static_cast<float>(radius.radiusBottomRight.value_or(0.0_vp).ConvertToPx()));
    clipRadius.emplace_back(radiusBottomRight);
    auto radiusBottomLeft = RSPoint(static_cast<float>(radius.radiusBottomLeft.value_or(0.0_vp).ConvertToPx()),
        static_cast<float>(radius.radiusBottomLeft.value_or(0.0_vp).ConvertToPx()));
    clipRadius.emplace_back(radiusBottomLeft);
}

void AutoFillController::DrawAutoFill(RSCanvas& canvas, const OffsetF& contentOffset, float translationOffset,
    float textScrollOffset, float defaultCharIndex, const Color& emphasizeCharTextColor,
    const Color& defaultCharInitTextColor, const std::optional<Color>& originTextColor)
{
    auto host = host_.Upgrade();
    CHECK_NULL_VOID(host);
    CHECK_NULL_VOID(autoFillParagraph_);
    auto contentRect = host->GetAutoFillTextContentRect();
    auto clipRectY1 = contentRect.GetY() + contentRect.Height();

    // 1. Color partition: [0..defaultCharIndex] = origin color, [defaultCharIndex..end] = emphasize color
    auto ceilDefaultCharIndex = std::ceil(defaultCharIndex);
    auto length = autoFillParagraph_->GetParagraphText().length();
    auto resolvedOriginColor = originTextColor.value_or(defaultCharInitTextColor);
    autoFillParagraph_->UpdateColor(0, ceilDefaultCharIndex, resolvedOriginColor);
    autoFillParagraph_->UpdateColor(ceilDefaultCharIndex, length, emphasizeCharTextColor);

    // 2. ClipRect: only show the area the icon has "passed through"
    auto isRTL = host->GetAutoFillTextDirection() == TextDirection::RTL;
    canvas.Save();
    // In LTR, translationOffset includes contentRect.GetX() (the content area's
    // left offset). The visible text width is from contentRect.GetX() to the
    // icon position, so subtract contentRect.GetX(). In RTL, translationOffset
    // is relative to the right edge and does not include contentRect.GetX().
    auto textShowWidth = isRTL ? std::abs(translationOffset)
                               : std::max(std::abs(translationOffset) - contentRect.GetX(), 0.0f);
    auto clipRectX0 = contentRect.GetX();
    auto clipRectX1 = contentRect.GetX() + textShowWidth;
    if (isRTL) {
        clipRectX0 = contentRect.GetX() + contentRect.Width() + translationOffset;
        clipRectX1 = contentRect.GetX() + contentRect.Width();
    }
    RSRect clipInnerRect = RSRect(clipRectX0, contentRect.GetY(), clipRectX1, clipRectY1);
    canvas.ClipRect(clipInnerRect, RSClipOp::INTERSECT);

    // 3. ClipRoundRect: clip within the frame's border radius
    auto textRectX = animationTextRect_.GetX();
    RSRect clipRect;
    std::vector<RSPoint> clipRadius;
    GetFrameRectClip(clipRect, clipRadius);
    clipRect = RSRect(0.0f, 0.0f, clipRectX1, clipRect.GetHeight());
    if (isRTL) {
        clipRect = RSRect(clipRectX0, 0.0f, clipRectX1, clipRect.GetHeight());
    }
    canvas.ClipRoundRect(clipRect, clipRadius, true);

    // 4. Paint the paragraph on the clipped canvas
    autoFillParagraph_->Paint(canvas, textRectX - textScrollOffset, contentOffset.GetY());
    canvas.Restore();
}
} // namespace OHOS::Ace::NG
