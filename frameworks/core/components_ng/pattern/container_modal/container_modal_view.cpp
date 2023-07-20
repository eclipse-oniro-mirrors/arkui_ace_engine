/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/container_modal/container_modal_view.h"

#include "base/geometry/ng/offset_t.h"
#include "base/image/pixel_map.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/gestures/tap_gesture.h"
#include "core/components_ng/gestures/pan_gesture.h"
#include "core/components_ng/pattern/button/button_layout_property.h"
#include "core/components_ng/pattern/button/button_pattern.h"
#include "core/components_ng/pattern/container_modal/container_modal_pattern.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/list/list_pattern.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/event/mouse_event.h"
#include "frameworks/bridge/common/utils/utils.h"

namespace OHOS::Ace::NG {

/**
 * The structure of container_modal is designed as follows :
 * |--container_modal(stack)
 *   |--column
 *      |--container_modal_title(row)
 *          |--icon(image), label(text), [leftSplit, maxRecover, minimize, close](button)
 *      |--stack
 *          |--container_modal_content(stage)
 *              |--page
 *          |--dialog(when show)
 *   |--container_modal_floating_title(row)
 *          |--icon(image), label(text), [leftSplit, maxRecover, minimize, close](button)
 */

RefPtr<FrameNode> ContainerModalView::Create(RefPtr<FrameNode>& content)
{
    auto containerModalNode = FrameNode::CreateFrameNode(
        "ContainerModal", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ContainerModalPattern>());
    containerModalNode->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_PARENT);
    auto stack = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<StackPattern>());
    auto column = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(true));

    column->AddChild(BuildTitle(containerModalNode));
    stack->AddChild(content);
    column->AddChild(stack);
    content->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_CONTENT);
    content->GetLayoutProperty()->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(1.0, DimensionUnit::PERCENT), CalcLength(1.0, DimensionUnit::PERCENT)));
    containerModalNode->AddChild(column);
    containerModalNode->AddChild(BuildTitle(containerModalNode, true));

    CHECK_NULL_RETURN(stack->GetLayoutProperty(), nullptr);
    stack->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_PARENT);
    CHECK_NULL_RETURN(column->GetLayoutProperty(), nullptr);
    column->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_PARENT);

    auto containerPattern = containerModalNode->GetPattern<ContainerModalPattern>();
    CHECK_NULL_RETURN(containerPattern, nullptr);
    containerModalNode->MarkModifyDone();
    containerPattern->InitContainerEvent();

    return containerModalNode;
}

RefPtr<FrameNode> ContainerModalView::BuildTitle(RefPtr<FrameNode>& containerNode, bool isFloatingTitle)
{
    LOGI("ContainerModalView BuildTitle called");
    auto titleContainer = BuildTitleContainer(containerNode, isFloatingTitle);
    CHECK_NULL_RETURN(titleContainer, nullptr);
    return AddControlButtons(titleContainer);
}
 
RefPtr<FrameNode> ContainerModalView::BuildTitleContainer(RefPtr<FrameNode>& containerNode, bool isFloatingTitle)
{
    auto containerTitleRow = BuildTitleRow(isFloatingTitle);
    auto pipeline = PipelineContext::GetCurrentContext();
    auto themeManager = pipeline->GetThemeManager();
    CHECK_NULL_RETURN(themeManager, nullptr);
    auto themeConstants = themeManager->GetThemeConstants();
    CHECK_NULL_RETURN(themeConstants, nullptr);
    // create title icon
    ImageSourceInfo imageSourceInfo;
    auto titleIcon = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    auto titleFocus = titleIcon->GetFocusHub();
    if (titleFocus) {
        titleFocus->SetFocusable(false);
    }
    auto id = pipeline->GetWindowManager()->GetAppIconId();
    auto pixelMap = themeConstants->GetPixelMap(id);
    if (pixelMap) {
        imageSourceInfo = ImageSourceInfo(PixelMap::CreatePixelMap(&pixelMap));
    } else {
        imageSourceInfo.SetSrc(themeConstants->GetMediaPath(id));
    }
    auto imageLayoutProperty = titleIcon->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_RETURN(imageLayoutProperty, nullptr);
    imageLayoutProperty->UpdateImageSourceInfo(imageSourceInfo);
    imageLayoutProperty->UpdateUserDefinedIdealSize(CalcSize(CalcLength(TITLE_ICON_SIZE), CalcLength(TITLE_ICON_SIZE)));
    MarginProperty margin = { CalcLength(TITLE_PADDING_START), CalcLength(TITLE_ELEMENT_MARGIN_HORIZONTAL) };
    imageLayoutProperty->UpdateMargin(margin);
    titleIcon->MarkModifyDone();

    // create title label
    auto titleLabel = FrameNode::CreateFrameNode(
        V2::TEXT_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<TextPattern>());
    auto textLayoutProperty = titleLabel->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(textLayoutProperty, nullptr);
    textLayoutProperty->UpdateContent(themeConstants->GetString(pipeline->GetWindowManager()->GetAppLabelId()));
    textLayoutProperty->UpdateMaxLines(1);
    textLayoutProperty->UpdateFontSize(TITLE_TEXT_FONT_SIZE);
    textLayoutProperty->UpdateTextColor(TITLE_TEXT_COLOR);
    textLayoutProperty->UpdateTextOverflow(TextOverflow::ELLIPSIS);
    textLayoutProperty->UpdateFontWeight(FontWeight::W500);
    textLayoutProperty->UpdateAlignment(Alignment::CENTER_LEFT);
    textLayoutProperty->UpdateLayoutWeight(1.0f);

    // add icon and label
    containerTitleRow->AddChild(titleIcon);
    containerTitleRow->AddChild(titleLabel);
    return containerTitleRow;
}

RefPtr<FrameNode> ContainerModalView::BuildTitleRow(bool isFloatingTitle)
{
    auto containerTitleRow = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<LinearLayoutPattern>(false));
    containerTitleRow->UpdateInspectorId("ContainerModalTitleRow");
    auto layoutProperty = containerTitleRow->GetLayoutProperty<LinearLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, nullptr);
    layoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(1.0, DimensionUnit::PERCENT), CalcLength(CONTAINER_TITLE_HEIGHT)));
    layoutProperty->UpdateMainAxisAlign(FlexAlign::FLEX_START);
    layoutProperty->UpdateCrossAxisAlign(FlexAlign::CENTER);
    if (isFloatingTitle) {
        auto renderContext = containerTitleRow->GetRenderContext();
        CHECK_NULL_RETURN(renderContext, nullptr);
        renderContext->UpdateBackgroundColor(CONTAINER_BACKGROUND_COLOR);
    }

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto windowManager = pipeline->GetWindowManager();
    CHECK_NULL_RETURN(windowManager, nullptr);
    if (!isFloatingTitle) {
        // touch the title to move the floating window
        auto eventHub = containerTitleRow->GetOrCreateGestureEventHub();
        CHECK_NULL_RETURN(eventHub, nullptr);
        PanDirection panDirection;
        panDirection.type = PanDirection::ALL;
        auto panActionStart = [wk = AceType::WeakClaim(AceType::RawPtr(windowManager))](const GestureEvent&) {
            auto windowManager = wk.Upgrade();
            CHECK_NULL_VOID_NOLOG(windowManager);
            if (windowManager->GetCurrentWindowMaximizeMode() != MaximizeMode::MODE_AVOID_SYSTEM_BAR) {
                LOGI("container window start move.");
                windowManager->WindowStartMove();
            }
        };
        auto panEvent = AceType::MakeRefPtr<PanEvent>(std::move(panActionStart), nullptr, nullptr, nullptr);
        eventHub->AddPanEvent(panEvent, panDirection, DEFAULT_PAN_FINGER, DEFAULT_PAN_DISTANCE);
    }
    return containerTitleRow;
}

RefPtr<FrameNode> ContainerModalView::AddControlButtons(RefPtr<FrameNode>& containerTitleRow)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto windowManager = pipeline->GetWindowManager();
    CHECK_NULL_RETURN(windowManager, nullptr);
 
    // add leftSplit / maxRecover / minimize / close button
    containerTitleRow->AddChild(BuildControlButton(InternalResource::ResourceId::CONTAINER_MODAL_WINDOW_SPLIT_LEFT,
        [wk = AceType::WeakClaim(AceType::RawPtr(windowManager))](GestureEvent& info) {
            auto windowManager = wk.Upgrade();
            CHECK_NULL_VOID(windowManager);
            LOGI("left split button clicked");
            windowManager->FireWindowSplitCallBack();
        }));
    containerTitleRow->AddChild(BuildControlButton(InternalResource::ResourceId::CONTAINER_MODAL_WINDOW_MAXIMIZE,
        [wk = AceType::WeakClaim(AceType::RawPtr(windowManager))](GestureEvent& info) {
            auto windowManager = wk.Upgrade();
            CHECK_NULL_VOID_NOLOG(windowManager);
            auto mode = windowManager->GetWindowMode();
            if (mode == WindowMode::WINDOW_MODE_FULLSCREEN) {
                LOGI("recover button clicked");
                windowManager->WindowRecover();
            } else {
                LOGI("maximize button clicked");
                windowManager->WindowMaximize();
            }
        }));
    containerTitleRow->AddChild(BuildControlButton(InternalResource::ResourceId::CONTAINER_MODAL_WINDOW_MINIMIZE,
        [wk = AceType::WeakClaim(AceType::RawPtr(windowManager))] (GestureEvent& info) {
            auto windowManager = wk.Upgrade();
            CHECK_NULL_VOID(windowManager);
            LOGI("minimize button clicked");
            windowManager->WindowMinimize();
        }));
    containerTitleRow->AddChild(BuildControlButton(
        InternalResource::ResourceId::CONTAINER_MODAL_WINDOW_CLOSE,
        [wk = AceType::WeakClaim(AceType::RawPtr(windowManager))](GestureEvent& info) {
            auto windowManager = wk.Upgrade();
            CHECK_NULL_VOID(windowManager);
            LOGI("close button clicked");
            windowManager->WindowClose();
        }, true));

    return containerTitleRow;
}

RefPtr<FrameNode> ContainerModalView::BuildControlButton(InternalResource::ResourceId icon,
    GestureEventFunc&& clickCallback, bool isCloseButton, bool canDrag)
{
    // button image icon
    ImageSourceInfo imageSourceInfo;
    auto imageIcon = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    auto imageFocus = imageIcon->GetFocusHub();
    if (imageFocus) {
        imageFocus->SetFocusable(false);
    }
    imageSourceInfo.SetResourceId(icon);
    auto imageLayoutProperty = imageIcon->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_RETURN(imageLayoutProperty, nullptr);
    imageLayoutProperty->UpdateUserDefinedIdealSize(CalcSize(CalcLength(TITLE_ICON_SIZE), CalcLength(TITLE_ICON_SIZE)));
    imageLayoutProperty->UpdateImageSourceInfo(imageSourceInfo);
    auto imageRenderProperty = imageIcon->GetPaintProperty<ImageRenderProperty>();
    CHECK_NULL_RETURN(imageRenderProperty, nullptr);
    imageRenderProperty->UpdateImageInterpolation(ImageInterpolation::HIGH);
    imageIcon->MarkModifyDone();

    auto buttonNode = FrameNode::CreateFrameNode(
        V2::BUTTON_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ButtonPattern>());
    auto buttonFocus = buttonNode->GetFocusHub();
    if (buttonFocus) {
        buttonFocus->SetFocusable(false);
    }

    AddButtonHover(buttonNode, imageIcon);
    AddButtonMouse(buttonNode, imageIcon);

    auto renderContext = buttonNode->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, nullptr);
    renderContext->UpdateBackgroundColor(TITLE_BUTTON_BACKGROUND_COLOR);

    auto buttonPattern = AceType::DynamicCast<ButtonPattern>(buttonNode->GetPattern());
    CHECK_NULL_RETURN(buttonPattern, nullptr);
    buttonPattern->SetClickedColor(TITLE_BUTTON_CLICKED_COLOR);

    auto buttonEventHub = buttonNode->GetOrCreateGestureEventHub();
    CHECK_NULL_RETURN(buttonEventHub, nullptr);
    auto clickEvent = AceType::MakeRefPtr<ClickEvent>(std::move(clickCallback));
    buttonEventHub->AddClickEvent(clickEvent);
    // if can not be drag, cover father panEvent
    if (!canDrag) {
        auto panEvent = AceType::MakeRefPtr<PanEvent>(nullptr, nullptr, nullptr, nullptr);
        PanDirection panDirection;
        buttonEventHub->AddPanEvent(panEvent, panDirection, DEFAULT_PAN_FINGER, DEFAULT_PAN_DISTANCE);
    }

    auto buttonLayoutProperty = buttonNode->GetLayoutProperty<ButtonLayoutProperty>();
    CHECK_NULL_RETURN(buttonLayoutProperty, nullptr);
    buttonLayoutProperty->UpdateType(ButtonType::CIRCLE);
    buttonLayoutProperty->UpdateUserDefinedIdealSize(
        CalcSize(CalcLength(TITLE_BUTTON_SIZE), CalcLength(TITLE_BUTTON_SIZE)));

    MarginProperty margin;
    margin.right = CalcLength(isCloseButton ? TITLE_PADDING_END : TITLE_ELEMENT_MARGIN_HORIZONTAL);
    buttonLayoutProperty->UpdateMargin(margin);
    buttonNode->MarkModifyDone();

    buttonNode->AddChild(imageIcon);
    return buttonNode;
}

void ContainerModalView::AddButtonHover(RefPtr<FrameNode>& buttonNode, RefPtr<FrameNode>& imageNode)
{
    auto inputHub = buttonNode->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(inputHub);
    auto hoverTask = [buttonNode, imageNode](bool isHover) {
        auto buttonPattern = AceType::DynamicCast<ButtonPattern>(buttonNode->GetPattern());
        CHECK_NULL_VOID(buttonPattern);
        buttonPattern->SetInHover(isHover);
        float halfSize = TITLE_ICON_SIZE.Value() / 2.0f;
        float translateX = (buttonPattern->GetLocalLocation().GetX() - halfSize) / halfSize * 2;
        float translateY = (buttonPattern->GetLocalLocation().GetY() - halfSize) / halfSize * 2;
        auto buttonNodeRenderContext = buttonNode->GetRenderContext();
        auto imageIconRenderContext = imageNode->GetRenderContext();
        CHECK_NULL_VOID(buttonNodeRenderContext);
        CHECK_NULL_VOID(imageIconRenderContext);
        double imageScale = isHover ? 1.10 : 1.0;
        AnimationOption option = AnimationOption();
        option.SetDuration(100);
        auto icurve = AceType::MakeRefPtr<CubicCurve>(0.5f, 0.0f, 0.5f, 1.0f);
        option.SetCurve(icurve);
        TranslateOptions translate;
        translate.x = isHover ? translateX : 0.0f;
        translate.y = isHover ? translateY : 0.0f;
        if (isHover) {
            AnimationUtils::Animate(option,
                [buttonNodeRenderContext, imageIconRenderContext, imageScale, translate]() {
                    buttonNodeRenderContext->UpdateTransformScale(VectorF(imageScale, imageScale));
                    imageIconRenderContext->UpdateTransformScale(VectorF(1 / imageScale, 1 / imageScale));
                    imageIconRenderContext->UpdateTransformTranslate(translate);
                });
        } else {
            AnimationUtils::Animate(option, [buttonNodeRenderContext, imageIconRenderContext, imageScale, translate]() {
                buttonNodeRenderContext->UpdateTransformScale(VectorF(imageScale, imageScale));
                imageIconRenderContext->UpdateTransformScale(VectorF(imageScale, imageScale));
                imageIconRenderContext->UpdateTransformTranslate(translate);
            });
        }
    };
    auto hoverEvent =  AceType::MakeRefPtr<InputEvent>(std::move(hoverTask));
    inputHub->AddOnHoverEvent(hoverEvent);
}

void ContainerModalView::AddButtonMouse(RefPtr<FrameNode>& buttonNode, RefPtr<FrameNode>& imageNode)
{
    auto inputHub = buttonNode->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(inputHub);
    auto mouseTask = [buttonNode, imageNode](MouseInfo& info) {
        auto buttonPattern = AceType::DynamicCast<ButtonPattern>(buttonNode->GetPattern());
        CHECK_NULL_VOID(buttonPattern);
        if (info.GetAction() != MouseAction::MOVE || !buttonPattern->GetIsInHover()) {
            buttonPattern->SetLocalLocation(info.GetLocalLocation());
            return;
        }
        auto imageIconRenderContext = imageNode->GetRenderContext();
        CHECK_NULL_VOID(imageIconRenderContext);
        float halfSize = TITLE_ICON_SIZE.Value() / 2.0f;
        float translateX = (info.GetLocalLocation().GetX() - halfSize) / halfSize * 2;
        float translateY = (info.GetLocalLocation().GetY() - halfSize) / halfSize * 2;
        float response = ResponsiveSpringMotion::DEFAULT_RESPONSIVE_SPRING_MOTION_RESPONSE;
        float dampingRatio = ResponsiveSpringMotion::DEFAULT_RESPONSIVE_SPRING_MOTION_DAMPING_RATIO;
        float blendDuration = ResponsiveSpringMotion::DEFAULT_RESPONSIVE_SPRING_MOTION_BLEND_DURATION;
        auto motion = AceType::MakeRefPtr<ResponsiveSpringMotion>(response, dampingRatio, blendDuration);
        AnimationOption option = AnimationOption();
        option.SetCurve(motion);
        TranslateOptions translate;
        translate.x = translateX;
        translate.y = translateY;
        AnimationUtils::Animate(option, [imageIconRenderContext, translate]() {
            imageIconRenderContext->UpdateTransformTranslate(translate);
        });
    };
    auto mouseEvent =  AceType::MakeRefPtr<InputEvent>(std::move(mouseTask));
    inputHub->AddOnMouseEvent(mouseEvent);
}

} // namespace OHOS::Ace::NG
