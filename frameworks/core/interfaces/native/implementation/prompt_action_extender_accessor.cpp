/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/progress/progress_model_ng.h"
#include "core/components_ng/pattern/progress/progress_model_static.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/validators.h"
#include "arkoala_api_generated.h"
#include "core/interfaces/native/implementation/frame_node_peer_impl.h"
#include "core/components/popup/popup_theme.h"
#include "core/components/theme/shadow_theme.h"
#include "core/components_ng/base/view_abstract_model_static.h"
#include "core/interfaces/native/generated/interface/ui_node_api.h"
#include "core/interfaces/native/utility/callback_helper.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "frameworks/base/utils/utils.h"
#include "frameworks/base/utils/system_properties.h"
#include "frameworks/bridge/common/utils/engine_helper.h"
#include "core/interfaces/native/utility/promise_helper.h"

using namespace OHOS::Ace::NG::Converter;
namespace OHOS::Ace::NG {
constexpr int32_t INVALID_ID = -1;
std::unordered_map<int32_t, std::string> UICONTEXT_ERROR_MAP = {
    { ERROR_CODE_TARGET_ID_NOT_EXIST, "The targetId does not exist." },
    { ERROR_CODE_TARGET_NOT_ON_MAIN_TREE, "The node of targetId is not on the component tree." },
    { ERROR_CODE_TARGET_NOT_PAGE_CHILD,
        "The node of targetId is not a child of the page node or NavDestination node." },
    { ERROR_CODE_INTERNAL_ERROR, "Internal error." },
    { ERROR_CODE_PARAM_INVALID, "Parameter error. Possible causes: 1. Mandatory parameters are left unspecified;"
        "2. Incorrect parameter types; 3. Parameter verification failed." },
    { ERROR_CODE_DIALOG_CONTENT_ERROR, "The ComponentContent is incorrect." },
    { ERROR_CODE_DIALOG_CONTENT_ALREADY_EXIST, "The ComponentContent already exists." },
    { ERROR_CODE_DIALOG_CONTENT_NOT_FOUND, "The ComponentContent cannot be found." },
    { ERROR_CODE_TARGET_INFO_NOT_EXIST, "The targetId does not exist." },
    { ERROR_CODE_TARGET_NOT_ON_COMPONENT_TREE, "The node of targetId is not on the component tree." }
};

void ReturnPromise(const Callback_Opt_Array_String_Void* promiseValue, int32_t errorCode)
{
    auto promise = std::make_shared<PromiseHelper<Callback_Opt_Array_String_Void>>(promiseValue);
    if (errorCode == ERROR_CODE_NO_ERROR) {
        promise->Resolve();
    } else {
        auto codeValue = std::to_string(errorCode);
        auto codeInfo = UICONTEXT_ERROR_MAP[errorCode];
        StringArray ErrorList = { codeValue, codeInfo };
        promise->Reject(ErrorList);
    }
}

int32_t ParseTargetInfo(const Ark_TargetInfo* targetInfo, int32_t& targetId)
{
    auto targetInfoID = targetInfo->id;
    auto targetInfoComponentId = targetInfo->componentId;
    int result = ERROR_CODE_NO_ERROR;
    Converter::VisitUnion(targetInfoID,
        [&targetId](const Ark_Number& value) {
            targetId = Converter::Convert<int32_t>(value);
        },
        [&targetId, targetInfoComponentId, &result](const Ark_String& value) {
            auto targetIdString = Converter::Convert<std::string>(value);
            auto targetComponentId = Converter::OptConvert<int32_t>(targetInfoComponentId);
            if (targetComponentId.has_value()) {
                auto componentId = targetComponentId.value();
                auto targetComponentIdNode =
                    ElementRegister::GetInstance()->GetSpecificItemById<NG::FrameNode>(componentId);
                if (!targetComponentIdNode) {
                    result = ERROR_CODE_TARGET_INFO_NOT_EXIST;
                    return;
                }
                if (targetComponentIdNode->GetInspectorId().value_or("") == targetIdString) {
                    targetId = targetComponentIdNode->GetId();
                    return;
                }
                auto targetNode = NG::FrameNode::FindChildByName(targetComponentIdNode, targetIdString);
                if (!targetNode) {
                    result = ERROR_CODE_TARGET_INFO_NOT_EXIST;
                    return;
                }
                targetId = targetNode->GetId();
            } else {
                auto targetNode = ElementRegister::GetInstance()->GetAttachedFrameNodeById(targetIdString);
                if (!targetNode) {
                    result = ERROR_CODE_TARGET_INFO_NOT_EXIST;
                    return;
                }
                targetId = targetNode->GetId();
            }
        }, []() {});
    if (targetId < 0) {
        return ERROR_CODE_PARAM_INVALID;
    }
    return result;
}

auto g_bindMenuOptionsParamCallbacks = [](
    const auto& menuOptions, MenuParam& menuParam, WeakPtr<FrameNode> weakNode) {
    auto onAppearValue = OptConvert<Callback_Void>(menuOptions.onAppear);
    if (onAppearValue) {
        auto onAppear = [arkCallback = CallbackHelper(onAppearValue.value()), weakNode]() {
            PipelineContext::SetCallBackNode(weakNode);
            arkCallback.Invoke();
        };
        menuParam.onAppear = std::move(onAppear);
    }
    auto onDisappearValue = OptConvert<Callback_Void>(menuOptions.onDisappear);
    if (onDisappearValue) {
        auto onDisappear = [arkCallback = CallbackHelper(onDisappearValue.value()), weakNode]() {
            PipelineContext::SetCallBackNode(weakNode);
            arkCallback.Invoke();
        };
        menuParam.onDisappear = std::move(onDisappear);
    }
    auto aboutToAppearValue = OptConvert<Callback_Void>(menuOptions.aboutToAppear);
    if (aboutToAppearValue) {
        auto aboutToAppear = [arkCallback = CallbackHelper(aboutToAppearValue.value()), weakNode]() {
            PipelineContext::SetCallBackNode(weakNode);
            arkCallback.Invoke();
        };
        menuParam.aboutToAppear = std::move(aboutToAppear);
    }
    auto aboutToDisAppearValue = OptConvert<Callback_Void>(menuOptions.aboutToDisappear);
    if (aboutToDisAppearValue) {
        auto aboutToDisappear = [arkCallback = CallbackHelper(aboutToDisAppearValue.value()), weakNode]() {
            PipelineContext::SetCallBackNode(weakNode);
            arkCallback.Invoke();
        };
        menuParam.aboutToDisappear = std::move(aboutToDisappear);
    }
};

auto g_bindMenuOptionsParam = [](const auto& menuOptions, MenuParam& menuParam) {
    auto offsetVal =
        OptConvert<std::pair<std::optional<Dimension>, std::optional<Dimension>>>(menuOptions.offset);
    if (offsetVal) {
        menuParam.positionOffset.SetX(offsetVal.value().first->ConvertToPx());
        menuParam.positionOffset.SetY(offsetVal.value().second->ConvertToPx());
    }
    menuParam.enableHoverMode = OptConvert<bool>(menuOptions.enableHoverMode);
    menuParam.backgroundColor = OptConvert<Color>(menuOptions.backgroundColor);
    auto backgroundBlurStyle = OptConvert<BlurStyle>(menuOptions.backgroundBlurStyle);
    menuParam.backgroundBlurStyle =
        backgroundBlurStyle ? std::optional<int32_t>(static_cast<int32_t>(backgroundBlurStyle.value())) : std::nullopt;
    auto transitionOpt = OptConvert<RefPtr<NG::ChainedTransitionEffect>>(menuOptions.transition);
    menuParam.transition = transitionOpt.value_or(menuParam.transition);
    menuParam.hasTransitionEffect = transitionOpt.has_value();
    menuParam.enableArrow = OptConvert<bool>(menuOptions.enableArrow);
    menuParam.arrowOffset = OptConvert<CalcDimension>(menuOptions.arrowOffset);
    menuParam.placement = OptConvert<Placement>(menuOptions.placement);
    // if enableArrow is true and placement not set, set placement default value to top.
    if (menuParam.enableArrow.has_value() && !menuParam.placement.has_value() && menuParam.enableArrow.value()) {
        menuParam.placement = Placement::TOP;
    }
    if (!menuParam.placement.has_value()) {
        menuParam.placement = Placement::BOTTOM_LEFT;
    }
    auto borderRadius = OptConvert<BorderRadiusProperty>(menuOptions.borderRadius);
    if (borderRadius.has_value() && (borderRadius.value().radiusTopLeft.has_value()
        || borderRadius.value().radiusTopRight.has_value()
        || borderRadius.value().radiusBottomLeft.has_value()
        || borderRadius.value().radiusBottomRight.has_value())) {
        menuParam.borderRadius = borderRadius;
    }
    Converter::VisitUnion(
        menuOptions.preview,
        [&menuParam](const Ark_MenuPreviewMode& value) {
            auto mode = Converter::OptConvert<MenuPreviewMode>(value);
            if (mode && mode.value() == MenuPreviewMode::IMAGE) {
                menuParam.previewMode = MenuPreviewMode::IMAGE;
            }
        },
        [](const CustomNodeBuilder& value) {}, []() {});
    menuParam.previewBorderRadius = OptConvert<BorderRadiusProperty>(menuOptions.previewBorderRadius);
    menuParam.layoutRegionMargin = OptConvert<PaddingProperty>(menuOptions.layoutRegionMargin);
    menuParam.layoutRegionMargin->start = menuParam.layoutRegionMargin->left;
    menuParam.layoutRegionMargin->end = menuParam.layoutRegionMargin->right;
    menuParam.hapticFeedbackMode =
        OptConvert<HapticFeedbackMode>(menuOptions.hapticFeedbackMode).value_or(menuParam.hapticFeedbackMode);
    menuParam.outlineColor = OptConvert<BorderColorProperty>(menuOptions.outlineColor);
    menuParam.outlineWidth = OptConvert<BorderWidthProperty>(menuOptions.outlineWidth);
    menuParam.effectOption = OptConvert<EffectOption>(menuOptions.backgroundEffect);
    menuParam.blurStyleOption = OptConvert<BlurStyleOption>(menuOptions.backgroundBlurStyleOptions);
};

auto g_onWillDismissPopup = [](
    const Opt_Union_Boolean_Callback_DismissPopupAction_Void& param, RefPtr<PopupParam>& popupParam) {
    CHECK_NULL_VOID(popupParam);
    Converter::VisitUnion(param,
        [&popupParam](const Ark_Boolean& value) {
            popupParam->SetInteractiveDismiss(Converter::Convert<bool>(value));
            popupParam->SetOnWillDismiss(nullptr);
        },
        [&popupParam](const Callback_DismissPopupAction_Void& value) {
            auto callback = [arkCallback = CallbackHelper(value)](int32_t reason) {
            };
            popupParam->SetOnWillDismiss(std::move(callback));
            popupParam->SetInteractiveDismiss(true);
        },
        []() {});
};

auto g_setPopupDefaultBlurStyle = [](RefPtr<PopupParam>& popupParam) {
    auto container = Container::CurrentSafely();
    CHECK_NULL_VOID(container);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    auto popupTheme = pipelineContext->GetTheme<PopupTheme>();
    CHECK_NULL_VOID(popupTheme);
    auto blurStyle = static_cast<BlurStyle>(popupTheme->GetPopupBackgroundBlurStyle());
    popupParam->SetBlurStyle(blurStyle);
};

auto g_getShadowFromTheme = [](ShadowStyle shadowStyle, Shadow& shadow) -> bool {
    auto colorMode = Container::CurrentColorMode();
    if (shadowStyle == ShadowStyle::None) {
        return true;
    }
    auto container = Container::CurrentSafely();
    CHECK_NULL_RETURN(container, false);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_RETURN(pipelineContext, false);
    auto shadowTheme = pipelineContext->GetTheme<ShadowTheme>();
    if (!shadowTheme) {
        return false;
    }
    shadow = shadowTheme->GetShadow(shadowStyle, colorMode);
    return true;
};

auto g_getPopupDefaultShadow = []() -> ShadowStyle {
    auto shadowStyle = ShadowStyle::OuterDefaultMD;
    auto container = Container::CurrentSafely();
    CHECK_NULL_RETURN(container, shadowStyle);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_RETURN(pipelineContext, shadowStyle);
    auto popupTheme = pipelineContext->GetTheme<PopupTheme>();
    CHECK_NULL_RETURN(popupTheme, shadowStyle);
    return popupTheme->GetPopupShadowStyle();
};

void updatePopupCommonParamPart1(const Ark_PopupCommonOptions& src, RefPtr<PopupParam>& popupParam)
{
    popupParam->SetPlacement(Converter::OptConvert<Placement>(src.placement).value_or(Placement::BOTTOM));
    auto popupBackgroundColor = Converter::OptConvert<Color>(src.popupColor);
    if (popupBackgroundColor.has_value()) {
        popupParam->SetBackgroundColor(popupBackgroundColor.value());
    }
    popupParam->SetEnableArrow(Converter::OptConvert<bool>(src.enableArrow).value_or(popupParam->EnableArrow()));
    auto autoCancel = Converter::OptConvert<bool>(src.autoCancel);
    if (autoCancel.has_value()) {
        popupParam->SetHasAction(!autoCancel.value());
    }
    auto arkOnStateChange = Converter::OptConvert<PopupStateChangeCallback>(src.onStateChange);
    if (arkOnStateChange.has_value()) {
        auto onStateChangeCallback = [arkCallback = CallbackHelper(arkOnStateChange.value())](
            const std::string& param) {
            auto json = JsonUtil::ParseJsonString(param);
            json->Put("isVisible", param.c_str());
            Ark_PopupStateChangeParam event;
            event.isVisible = Converter::ArkValue<Ark_Boolean>(json->GetBool("isVisible", false));
            arkCallback.Invoke(event);
        };
        popupParam->SetOnStateChange(std::move(onStateChangeCallback));
    }
    bool showInSubBoolean = Converter::OptConvert<bool>(src.showInSubWindow).value_or(popupParam->IsShowInSubWindow());
#if defined(PREVIEW)
    showInSubBoolean = false;
#endif
    popupParam->SetShowInSubWindow(showInSubBoolean);
    Converter::VisitUnion(src.mask,
        [&popupParam](const Ark_Boolean& mask) {
            popupParam->SetBlockEvent(Converter::Convert<bool>(mask));
        },
        [&popupParam](const Ark_PopupMaskType& mask) {
            auto popupMaskColor = Converter::OptConvert<Color>(mask.color);
            if (popupMaskColor.has_value()) {
                popupParam->SetMaskColor(popupMaskColor.value());
            }
        },
        []() {});
    auto targetSpaceOpt = Converter::OptConvert<CalcDimension>(src.targetSpace);
    if (targetSpaceOpt.has_value()) {
        popupParam->SetTargetSpace(targetSpaceOpt.value());
    }
    auto offsetOpt = Converter::OptConvert<Dimension>(src.arrowOffset);
    auto pointPositionOpt = Converter::OptConvert<Dimension>(src.arrowPointPosition);
    if (pointPositionOpt.has_value()) {
        popupParam->SetArrowOffset(pointPositionOpt.value());
    } else if (offsetOpt.has_value()) {
        popupParam->SetArrowOffset(offsetOpt.value());
    }
}

void updatePopupCommonParamPart2(const Ark_PopupCommonOptions& src, RefPtr<PopupParam>& popupParam)
{
    auto offsetVal = src.offset.value;
    auto x = Converter::OptConvert<Dimension>(offsetVal.x);
    auto y = Converter::OptConvert<Dimension>(offsetVal.y);
    if (x.has_value() && y.has_value()) {
        Offset popupOffset;
        popupOffset.SetX(x.value().ConvertToPx());
        popupOffset.SetY(y.value().ConvertToPx());
        popupParam->SetTargetOffset(popupOffset);
    }
    auto widthOpt = Converter::OptConvert<CalcDimension>(src.width);
    Validator::ValidateNonNegative(widthOpt);
    if (widthOpt.has_value()) {
        popupParam->SetChildWidth(widthOpt.value());
    }
    auto arrowWidthOpt = Converter::OptConvert<CalcDimension>(src.arrowWidth);
    Validator::ValidateNonNegative(arrowWidthOpt);
    Validator::ValidateNonPercent(arrowWidthOpt);
    if (arrowWidthOpt.has_value()) {
        popupParam->SetArrowWidth(arrowWidthOpt.value());
    }
    auto arrowHeightOpt = Converter::OptConvert<CalcDimension>(src.arrowHeight);
    Validator::ValidateNonNegative(arrowHeightOpt);
    Validator::ValidateNonPercent(arrowHeightOpt);
    if (arrowHeightOpt.has_value()) {
        popupParam->SetArrowHeight(arrowHeightOpt.value());
    }
    auto radiusOpt = Converter::OptConvert<CalcDimension>(src.radius);
    Validator::ValidateNonNegative(radiusOpt);
    if (radiusOpt.has_value()) {
        popupParam->SetRadius(radiusOpt.value());
    }
    auto shadowOpt = Converter::OptConvert<Shadow>(src.shadow);
    if (shadowOpt.has_value()) {
        popupParam->SetShadow(shadowOpt.value());
    } else {
        auto defaultPopupShadowStyle = g_getPopupDefaultShadow();
        Shadow shadow;
        g_getShadowFromTheme(defaultPopupShadowStyle, shadow);
        popupParam->SetShadow(shadow);
    }
    auto popupBackgroundBlurStyleOpt = Converter::OptConvert<BlurStyle>(src.backgroundBlurStyle);
    if (popupBackgroundBlurStyleOpt.has_value()) {
        popupParam->SetBlurStyle(popupBackgroundBlurStyleOpt.value());
    } else {
        g_setPopupDefaultBlurStyle(popupParam);
    }
    popupParam->SetFocusable(Converter::OptConvert<bool>(src.focusable).value_or(popupParam->GetFocusable()));
    auto popupTransitionEffectsOpt = Converter::OptConvert<RefPtr<NG::ChainedTransitionEffect>>(src.transition);
    if (popupTransitionEffectsOpt.has_value()) {
        popupParam->SetTransitionEffects(popupTransitionEffectsOpt.value());
    }
    g_onWillDismissPopup(src.onWillDismiss, popupParam);
    popupParam->SetEnableHoverMode(Converter::OptConvert<bool>(src.enableHoverMode)
        .value_or(popupParam->EnableHoverMode()));
    popupParam->SetFollowTransformOfTarget(Converter::OptConvert<bool>(src.followTransformOfTarget)
        .value_or(popupParam->IsFollowTransformOfTarget()));
}

void updatePopupCommonParam(const Ark_PopupCommonOptions& src, RefPtr<PopupParam>& popupParam)
{
    CHECK_NULL_VOID(popupParam);
    updatePopupCommonParamPart1(src, popupParam);
    updatePopupCommonParamPart2(src, popupParam);
}

} // OHOS::Ace::NG

namespace OHOS::Ace::NG::Converter {

template<>
RefPtr<PopupParam> Convert(const Ark_PopupCommonOptions& src)
{
    auto popupParam = AceType::MakeRefPtr<PopupParam>();
    updatePopupCommonParam(src, popupParam);
    return popupParam;
}

template<>
MenuParam Convert(const Ark_MenuOptions& src)
{
    MenuParam menuParam;
    g_bindMenuOptionsParam(src, menuParam);
    return menuParam;
}
} // OHOS::Ace::NG::Converter

namespace OHOS::Ace::NG::GeneratedModifier {
namespace PromptActionExtenderAccessor {
void OpenPopupImpl(Ark_VMContext vmContext,
                   Ark_AsyncWorkerPtr asyncWorker,
                   Ark_NativePointer content,
                   const Ark_TargetInfo* target,
                   const Opt_PopupCommonOptions* options,
                   const Callback_Opt_Array_String_Void* outputArgumentForReturningPromise)
{
    Ark_FrameNode peerNode = (Ark_FrameNode)content;
    auto frameNode = FrameNodePeer::GetFrameNodeByPeer(peerNode);
    if (!frameNode) {
        ReturnPromise(outputArgumentForReturningPromise, ERROR_CODE_DIALOG_CONTENT_ERROR);
        return;
    }
    auto popupParam = AceType::MakeRefPtr<PopupParam>();
    CHECK_NULL_VOID(popupParam);
    popupParam = Converter::Convert<RefPtr<PopupParam>>(options->value);
    popupParam->SetIsShow(true);
    popupParam->SetUseCustomComponent(true);
    int targetId = INVALID_ID;
    auto result = ParseTargetInfo(target, targetId);
    if (result == ERROR_CODE_NO_ERROR) {
        popupParam->SetTargetId(std::to_string(targetId));
    } else {
        ReturnPromise(outputArgumentForReturningPromise, result);
        return;
    }
    result = ViewAbstractModelStatic::OpenPopup(popupParam, frameNode);
    if (result == ERROR_CODE_INTERNAL_ERROR) {
        result = ERROR_CODE_NO_ERROR;
    }
    ReturnPromise(outputArgumentForReturningPromise, result);
}
void UpdatePopupImpl(Ark_VMContext vmContext,
                    Ark_AsyncWorkerPtr asyncWorker,
                    Ark_NativePointer content,
                    const Ark_PopupCommonOptions* options,
                    const Opt_Boolean* partialUpdate,
                    const Callback_Opt_Array_String_Void* outputArgumentForReturningPromise)
{
    Ark_FrameNode peerNode = (Ark_FrameNode)content;
    auto frameNode = FrameNodePeer::GetFrameNodeByPeer(peerNode);
    if (!frameNode) {
        ReturnPromise(outputArgumentForReturningPromise, ERROR_CODE_DIALOG_CONTENT_ERROR);
        return;
    }
    auto popupParam = AceType::MakeRefPtr<PopupParam>();
    auto oldParam = AceType::MakeRefPtr<PopupParam>();
    auto result = ViewAbstractModelStatic::GetPopupParam(oldParam, frameNode);
    if (result == ERROR_CODE_NO_ERROR) {
        auto isPartialUpdate = Converter::OptConvert<bool>(*partialUpdate);
        if (isPartialUpdate.has_value() && isPartialUpdate.value()) {
            popupParam = oldParam;
            popupParam->SetIsPartialUpdate(isPartialUpdate.value());
        } else {
            popupParam->SetTargetId(oldParam->GetTargetId());
            popupParam->SetIsPartialUpdate(false);
        }
    } else {
        ReturnPromise(outputArgumentForReturningPromise, result);
        return;
    }
    auto isShowInSubWindow = oldParam->IsShowInSubWindow();
    auto focusable = oldParam->GetFocusable();
    popupParam->SetIsShow(true);
    popupParam->SetUseCustomComponent(true);
    updatePopupCommonParam(*options, popupParam);
    popupParam->SetShowInSubWindow(isShowInSubWindow);
    popupParam->SetFocusable(focusable);
    result = ViewAbstractModelStatic::UpdatePopup(popupParam, frameNode);
    if (result == ERROR_CODE_INTERNAL_ERROR) {
        result = ERROR_CODE_NO_ERROR;
    }
    ReturnPromise(outputArgumentForReturningPromise, result);
}
void ClosePopupImpl(Ark_VMContext vmContext,
                    Ark_AsyncWorkerPtr asyncWorker,
                    Ark_NativePointer content,
                    const Callback_Opt_Array_String_Void* outputArgumentForReturningPromise)
{
    Ark_FrameNode peerNode = (Ark_FrameNode)content;
    auto frameNode = FrameNodePeer::GetFrameNodeByPeer(peerNode);
    if (!frameNode) {
        ReturnPromise(outputArgumentForReturningPromise, ERROR_CODE_DIALOG_CONTENT_ERROR);
        return;
    }
    auto result = ViewAbstractModelStatic::ClosePopup(frameNode);
    if (result == ERROR_CODE_INTERNAL_ERROR) {
        result = ERROR_CODE_NO_ERROR;
    }
    ReturnPromise(outputArgumentForReturningPromise, result);
}
void OpenMenuImpl(Ark_VMContext vmContext,
                  Ark_AsyncWorkerPtr asyncWorker,
                  Ark_NativePointer content,
                  const Ark_TargetInfo* target,
                  const Opt_MenuOptions* options,
                  const Callback_Opt_Array_String_Void* outputArgumentForReturningPromise)
{
    Ark_FrameNode peerNode = (Ark_FrameNode)content;
    auto frameNode = FrameNodePeer::GetFrameNodeByPeer(peerNode);
    if (!frameNode) {
        ReturnPromise(outputArgumentForReturningPromise, ERROR_CODE_DIALOG_CONTENT_ERROR);
        return;
    }
    MenuParam menuParam = Converter::Convert<MenuParam>(options->value);
    g_bindMenuOptionsParamCallbacks(options->value, menuParam, AceType::WeakClaim(frameNode.GetRawPtr()));
    auto pipelineContext = frameNode->GetContext();
    if (!pipelineContext) {
        ReturnPromise(outputArgumentForReturningPromise, ERROR_CODE_DIALOG_CONTENT_ERROR);
        return;
    }
    auto theme = pipelineContext->GetTheme<SelectTheme>();
    if (!theme) {
        ReturnPromise(outputArgumentForReturningPromise, ERROR_CODE_DIALOG_CONTENT_ERROR);
        return;
    }
    menuParam.isShowInSubWindow =
        Converter::OptConvert<bool>(options->value.showInSubWindow).value_or(theme->GetExpandDisplay());
    int targetId = INVALID_ID;
    auto result = ParseTargetInfo(target, targetId);
    if (result == ERROR_CODE_NO_ERROR) {
        result = ViewAbstractModelStatic::OpenMenu(menuParam, frameNode, targetId);
    }
    if (result == ERROR_CODE_INTERNAL_ERROR) {
        result = ERROR_CODE_NO_ERROR;
    }
    ReturnPromise(outputArgumentForReturningPromise, result);
}
void UpdateMenuImpl(Ark_VMContext vmContext,
                    Ark_AsyncWorkerPtr asyncWorker,
                    Ark_NativePointer content,
                    const Ark_MenuOptions* options,
                    const Opt_Boolean* partialUpdate,
                    const Callback_Opt_Array_String_Void* outputArgumentForReturningPromise)
{
    Ark_FrameNode peerNode = (Ark_FrameNode)content;
    auto frameNode = FrameNodePeer::GetFrameNodeByPeer(peerNode);
    if (!frameNode) {
        ReturnPromise(outputArgumentForReturningPromise, ERROR_CODE_DIALOG_CONTENT_ERROR);
        return;
    }
    MenuParam menuParam;
    auto isPartialUpdate = Converter::OptConvert<bool>(*partialUpdate);
    if (!isPartialUpdate.has_value()) {
        ReturnPromise(outputArgumentForReturningPromise, ERROR_CODE_PARAM_INVALID);
        return;
    }
    if (isPartialUpdate.value()) {
        auto result = ViewAbstractModelStatic::GetMenuParam(menuParam, frameNode);
        if (result != ERROR_CODE_NO_ERROR && result != ERROR_CODE_INTERNAL_ERROR) {
            ReturnPromise(outputArgumentForReturningPromise, result);
            return;
        }
    }
    g_bindMenuOptionsParam(*options, menuParam);
    auto result = ViewAbstractModelStatic::UpdateMenu(menuParam, frameNode);
    if (result == ERROR_CODE_INTERNAL_ERROR) {
        result = ERROR_CODE_NO_ERROR;
    }
    ReturnPromise(outputArgumentForReturningPromise, result);
}
void CloseMenuImpl(Ark_VMContext vmContext,
                   Ark_AsyncWorkerPtr asyncWorker,
                   Ark_NativePointer content,
                   const Callback_Opt_Array_String_Void* outputArgumentForReturningPromise)
{
    Ark_FrameNode peerNode = (Ark_FrameNode)content;
    auto frameNode = FrameNodePeer::GetFrameNodeByPeer(peerNode);
    if (!frameNode) {
        ReturnPromise(outputArgumentForReturningPromise, ERROR_CODE_DIALOG_CONTENT_ERROR);
        return;
    }
    auto result = ViewAbstractModelStatic::CloseMenu(frameNode);
    if (result == ERROR_CODE_INTERNAL_ERROR) {
        result = ERROR_CODE_NO_ERROR;
    }
    ReturnPromise(outputArgumentForReturningPromise, result);
}
} // PromptActionExtenderAccessor
const GENERATED_ArkUIPromptActionExtenderAccessor* GetPromptActionExtenderAccessor()
{
    static const GENERATED_ArkUIPromptActionExtenderAccessor PromptActionExtenderAccessorImpl {
        PromptActionExtenderAccessor::OpenPopupImpl,
        PromptActionExtenderAccessor::UpdatePopupImpl,
        PromptActionExtenderAccessor::ClosePopupImpl,
        PromptActionExtenderAccessor::OpenMenuImpl,
        PromptActionExtenderAccessor::UpdateMenuImpl,
        PromptActionExtenderAccessor::CloseMenuImpl,
    };
    return &PromptActionExtenderAccessorImpl;
}
}
