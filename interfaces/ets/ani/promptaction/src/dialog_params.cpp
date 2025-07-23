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

#include "dialog_params.h"
#include "prompt_action_controller.h"
#include "prompt_action_utils.h"

#include "frameworks/base/error/error_code.h"
#include "frameworks/base/i18n/localization.h"
#include "frameworks/core/common/ace_engine.h"
#include "frameworks/core/components_ng/base/ui_node.h"
#include "frameworks/core/components_ng/base/view_stack_processor.h"
#include "frameworks/core/interfaces/native/implementation/frame_node_peer_impl.h"
#include "frameworks/core/interfaces/native/implementation/transition_effect_peer_impl.h"

constexpr int32_t CALLBACK_PARAM_LENGTH = 2;
constexpr int32_t PRIMARY_BUTTON_COUNT_MAX = 1;

bool GetButtonInfo(ani_env* env, ani_ref resultRef, OHOS::Ace::ButtonInfo& result)
{
    if (IsUndefinedObject(env, resultRef)) {
        return false;
    }
    ani_object object = static_cast<ani_object>(resultRef);
    return GetButtonInfo(env, object, result);
}

bool GetButtonInfo(ani_env* env, ani_object object, OHOS::Ace::ButtonInfo& result)
{
    if (!IsClassObject(env, object, "L@ohos/promptAction/promptAction/Button;")) {
        return false;
    }
    GetResourceStrParam(env, object, "text", result.text);
    GetResourceStrParam(env, object, "color", result.textColor);
    GetBoolParam(env, object, "primary", result.isPrimary);
    return true;
}

bool GetButtonArray(ani_env *env, ani_object object, const char *name, std::vector<OHOS::Ace::ButtonInfo>& result)
{
    ani_ref resultRef;
    ani_status status = env->Object_GetPropertyByName_Ref(object, name, &resultRef);
    if (status != ANI_OK) {
        return false;
    }

    if (IsUndefinedObject(env, resultRef) || !IsArrayObject(env, resultRef)) {
        return false;
    }

    ani_size length;
    ani_array resultObj = static_cast<ani_array>(resultRef);
    status = env->Array_GetLength(resultObj, &length);
    if (status != ANI_OK) {
        return false;
    }

    std::vector<OHOS::Ace::ButtonInfo> buttonArray;
    for (int i = 0; i < int(length); i++) {
        ani_ref itemRef;
        status = env->Array_Get(resultObj, (ani_size)i, &itemRef);
        if (status != ANI_OK) {
            continue;
        }

        OHOS::Ace::ButtonInfo button;
        if (GetButtonInfo(env, itemRef, button)) {
            buttonArray.emplace_back(button);
        }
    }

    std::function<bool(OHOS::Ace::ButtonInfo)> isPrimary = [](OHOS::Ace::ButtonInfo button) {
        return button.isPrimary;
    };
    int32_t primaryButtonCount = std::count_if(buttonArray.begin(), buttonArray.end(), isPrimary);
    if (primaryButtonCount > PRIMARY_BUTTON_COUNT_MAX) {
        for (auto& button : buttonArray) {
            button.isPrimary = false;
        }
    }
    result = buttonArray;
    return true;
}

bool GetMaskRect(ani_env *env, ani_object object, std::optional<OHOS::Ace::DimensionRect>& result)
{
    OHOS::Ace::DimensionRect maskRect;
    maskRect.SetOffset(OHOS::Ace::DimensionOffset(OHOS::Ace::CalcDimension(0, OHOS::Ace::DimensionUnit::VP),
        OHOS::Ace::CalcDimension(0, OHOS::Ace::DimensionUnit::VP)));
    maskRect.SetSize(OHOS::Ace::DimensionSize(OHOS::Ace::CalcDimension(1, OHOS::Ace::DimensionUnit::PERCENT),
        OHOS::Ace::CalcDimension(1, OHOS::Ace::DimensionUnit::PERCENT)));
    result = std::make_optional<OHOS::Ace::DimensionRect>(maskRect);

    ani_ref resultRef;
    ani_status status = env->Object_GetPropertyByName_Ref(object, "maskRect", &resultRef);
    if (status != ANI_OK) {
        return false;
    }

    if (IsUndefinedObject(env, resultRef)) {
        return false;
    }

    ani_object resultObj = static_cast<ani_object>(resultRef);
    ani_ref dxRef;
    status = env->Object_GetPropertyByName_Ref(resultObj, "x", &dxRef);
    if (status != ANI_OK) {
        return false;
    }
    OHOS::Ace::CalcDimension dx;
    if (!GetLengthParam(env, dxRef, dx)) {
        return false;
    }

    ani_ref dyRef;
    status = env->Object_GetPropertyByName_Ref(resultObj, "y", &dyRef);
    if (status != ANI_OK) {
        return false;
    }
    OHOS::Ace::CalcDimension dy;
    if (!GetLengthParam(env, dyRef, dy)) {
        return false;
    }

    ani_ref widthRef;
    status = env->Object_GetPropertyByName_Ref(resultObj, "width", &widthRef);
    if (status != ANI_OK) {
        return false;
    }
    OHOS::Ace::CalcDimension width;
    if (!GetLengthParam(env, widthRef, width)) {
        return false;
    }

    ani_ref heightRef;
    status = env->Object_GetPropertyByName_Ref(resultObj, "height", &heightRef);
    if (status != ANI_OK) {
        return false;
    }
    OHOS::Ace::CalcDimension height;
    if (!GetLengthParam(env, heightRef, height)) {
        return false;
    }

    maskRect.SetOffset(OHOS::Ace::DimensionOffset(dx, dy));
    maskRect.SetSize(OHOS::Ace::DimensionSize(width, height));
    result = std::make_optional<OHOS::Ace::DimensionRect>(maskRect);
    return true;
}

bool GetDialogAlignment(ani_env* env, ani_object object, OHOS::Ace::DialogAlignment& result)
{
    int32_t resultInt;
    if (!GetEnumInt(env, object, "alignment", "Larkui/component/alertDialog/DialogAlignment;", resultInt)) {
        return false;
    }
    result = static_cast<OHOS::Ace::DialogAlignment>(resultInt);
    return true;
}

bool GetThemeColorMode(ani_env* env, ani_object object, OHOS::Ace::ThemeColorMode& result)
{
    int32_t resultInt;
    if (!GetEnumInt(env, object, "colorMode", "Larkui/component/common/ThemeColorMode;", resultInt)) {
        return false;
    }
    result = static_cast<OHOS::Ace::ThemeColorMode>(resultInt);
    return true;
}

bool GetAdaptiveColor(ani_env* env, ani_object object, OHOS::Ace::AdaptiveColor& result)
{
    int32_t resultInt;
    if (!GetEnumInt(env, object, "adaptiveColor", "Larkui/component/common/AdaptiveColor;", resultInt)) {
        return false;
    }
    result = static_cast<OHOS::Ace::AdaptiveColor>(resultInt);
    return true;
}

bool GetGrayscale(ani_env* env, ani_object object, std::vector<float>& result)
{
    ani_ref resultRef;
    ani_status status = env->Object_GetPropertyByName_Ref(object, "grayscale", &resultRef);
    if (status != ANI_OK) {
        return false;
    }

    if (IsUndefinedObject(env, resultRef)) {
        return false;
    }

    ani_size length;
    ani_tuple_value resultObj = static_cast<ani_tuple_value>(resultRef);
    status = env->TupleValue_GetNumberOfItems(resultObj, &length);
    if (status != ANI_OK) {
        return false;
    }

    std::vector<float> floatArray;
    for (int i = 0; i < int(length); i++) {
        ani_ref itemRef;
        status = env->Object_GetFieldByName_Ref(resultObj, ("$" + std::to_string(i)).c_str(), &itemRef);
        if (status != ANI_OK) {
            continue;
        }

        ani_object itemObj = static_cast<ani_object>(itemRef);
        double itemValue;
        if (GetDoubleParam(env, itemObj, itemValue)) {
            uint32_t itemInt32 = static_cast<int32_t>(itemValue);
            floatArray.emplace_back(static_cast<float>(itemInt32));
        }
    }
    result = floatArray;
    return true;
}

bool GetBlurOptions(ani_env* env, ani_object object, OHOS::Ace::BlurOption& result)
{
    ani_ref resultRef;
    ani_status status = env->Object_GetPropertyByName_Ref(object, "blurOptions", &resultRef);
    if (status != ANI_OK) {
        return false;
    }

    if (IsUndefinedObject(env, resultRef)) {
        return false;
    }

    ani_object resultObj = static_cast<ani_object>(resultRef);
    if (!IsClassObject(env, resultObj, "Larkui/component/common/BlurOptions;")) {
        return false;
    }

    std::vector<float> grayscale;
    if (!GetGrayscale(env, resultObj, grayscale)) {
        return false;
    }
    result.grayscale = grayscale;
    return true;
}

bool GetBlurStyleActivePolicy(ani_env* env, ani_object object, OHOS::Ace::BlurStyleActivePolicy& result)
{
    ani_int resultInt;
    if (!GetEnumInt(env, object, "policy", "Larkui/component/common/BlurStyleActivePolicy;", resultInt)) {
        return false;
    }
    result = static_cast<OHOS::Ace::BlurStyleActivePolicy>(resultInt);
    return true;
}

bool GetBackgroundBlurStyleOptions(ani_env* env, ani_object object, std::optional<OHOS::Ace::BlurStyleOption>& result)
{
    ani_ref resultRef;
    ani_status status = env->Object_GetPropertyByName_Ref(object, "backgroundBlurStyleOptions", &resultRef);
    if (status != ANI_OK) {
        return false;
    }

    if (IsUndefinedObject(env, resultRef)) {
        return false;
    }

    ani_object resultObj = static_cast<ani_object>(resultRef);
    if (!IsClassObject(env, resultObj, "Larkui/component/common/BackgroundBlurStyleOptions;")) {
        return false;
    }

    OHOS::Ace::BlurStyleOption blurStyleOption;
    GetThemeColorMode(env, resultObj, blurStyleOption.colorMode);
    GetAdaptiveColor(env, resultObj, blurStyleOption.adaptiveColor);
    GetDoubleParam(env, resultObj, "scale", blurStyleOption.scale);
    GetBlurOptions(env, resultObj, blurStyleOption.blurOption);
    GetBlurStyleActivePolicy(env, resultObj, blurStyleOption.policy);
    if (GetResourceColorParam(env, resultObj, "inactiveColor", blurStyleOption.inactiveColor)) {
        blurStyleOption.isValidColor = true;
    }
    result = std::make_optional<OHOS::Ace::BlurStyleOption>(blurStyleOption);
    return true;
}

bool GetBackgroundEffectOptions(ani_env* env, ani_object object, std::optional<OHOS::Ace::EffectOption>& result)
{
    ani_ref resultRef;
    ani_status status = env->Object_GetPropertyByName_Ref(object, "backgroundEffect", &resultRef);
    if (status != ANI_OK) {
        return false;
    }

    if (IsUndefinedObject(env, resultRef)) {
        return false;
    }

    ani_object resultObj = static_cast<ani_object>(resultRef);
    if (!IsClassObject(env, resultObj, "Larkui/component/common/BackgroundEffectOptions;")) {
        return false;
    }

    OHOS::Ace::EffectOption effectOption;
    double radius = 0.0f;
    ani_double aniRadius;
    status = env->Object_GetPropertyByName_Double(resultObj, "radius", &aniRadius);
    if (status == ANI_OK) {
        radius = static_cast<double>(aniRadius);
    }
    radius = OHOS::Ace::LessNotEqual(radius, 0.0f) ? 0.0f : radius;
    effectOption.radius = OHOS::Ace::CalcDimension(radius, OHOS::Ace::DimensionUnit::VP);

    double saturation = 1.0f;
    GetDoubleParam(env, resultObj, "saturation", saturation);
    effectOption.saturation = (OHOS::Ace::GreatNotEqual(saturation, 0.0f) || OHOS::Ace::NearZero(saturation)) ?
        saturation : 1.0f;

    double brightness = 1.0f;
    GetDoubleParam(env, resultObj, "brightness", brightness);
    effectOption.brightness = (OHOS::Ace::GreatNotEqual(brightness, 0.0f) || OHOS::Ace::NearZero(brightness)) ?
        brightness : 1.0f;

    GetResourceColorParam(env, resultObj, "color", effectOption.color);
    GetAdaptiveColor(env, resultObj, effectOption.adaptiveColor);
    GetBlurOptions(env, resultObj, effectOption.blurOption);
    GetBlurStyleActivePolicy(env, resultObj, effectOption.policy);
    if (GetResourceColorParam(env, resultObj, "inactiveColor", effectOption.inactiveColor)) {
        effectOption.isValidColor = true;
    }
    result = std::make_optional<OHOS::Ace::EffectOption>(effectOption);
    return true;
}

bool GetLevelMode(ani_env* env, ani_object object, OHOS::Ace::LevelMode& result)
{
    ani_int resultInt;
    if (!GetEnumInt(env, object, "levelMode", "L@ohos/promptAction/LevelMode;", resultInt)) {
        return false;
    }
    result = static_cast<OHOS::Ace::LevelMode>(resultInt);
    return true;
}

bool GetImmersiveMode(ani_env* env, ani_object object, OHOS::Ace::ImmersiveMode& result)
{
    ani_int resultInt;
    if (!GetEnumInt(env, object, "immersiveMode", "L@ohos/promptAction/ImmersiveMode;", resultInt)) {
        return false;
    }
    result = static_cast<OHOS::Ace::ImmersiveMode>(resultInt);
    return true;
}

void UpdateDialogAlignment(OHOS::Ace::DialogAlignment& alignment)
{
    bool isRtl = OHOS::Ace::AceApplicationInfo::GetInstance().IsRightToLeft();
    if (alignment == OHOS::Ace::DialogAlignment::TOP_START) {
        if (isRtl) {
            alignment = OHOS::Ace::DialogAlignment::TOP_END;
        }
    } else if (alignment == OHOS::Ace::DialogAlignment::TOP_END) {
        if (isRtl) {
            alignment = OHOS::Ace::DialogAlignment::TOP_START;
        }
    } else if (alignment == OHOS::Ace::DialogAlignment::CENTER_START) {
        if (isRtl) {
            alignment = OHOS::Ace::DialogAlignment::CENTER_END;
        }
    } else if (alignment == OHOS::Ace::DialogAlignment::CENTER_END) {
        if (isRtl) {
            alignment = OHOS::Ace::DialogAlignment::CENTER_START;
        }
    } else if (alignment == OHOS::Ace::DialogAlignment::BOTTOM_START) {
        if (isRtl) {
            alignment = OHOS::Ace::DialogAlignment::BOTTOM_END;
        }
    } else if (alignment == OHOS::Ace::DialogAlignment::BOTTOM_END) {
        if (isRtl) {
            alignment = OHOS::Ace::DialogAlignment::BOTTOM_START;
        }
    }
}

bool GetOnLanguageChange(OHOS::Ace::DialogProperties& dialogProps)
{
    auto onLanguageChange = [shadow = dialogProps.shadow, alignment = dialogProps.alignment,
        offset = dialogProps.offset, maskRect = dialogProps.maskRect,
        updateAlignment = UpdateDialogAlignment](OHOS::Ace::DialogProperties& dialogProps) {
        bool isRtl = OHOS::Ace::AceApplicationInfo::GetInstance().IsRightToLeft();
        if (shadow.has_value()) {
            OHOS::Ace::Shadow newShadow = shadow.value();
            double offsetX = isRtl ? newShadow.GetOffset().GetX() * (-1) : newShadow.GetOffset().GetX();
            newShadow.SetOffsetX(offsetX);
            dialogProps.shadow = newShadow;
        }

        OHOS::Ace::DialogAlignment newAlignment = alignment;
        updateAlignment(newAlignment);
        dialogProps.alignment = newAlignment;

        OHOS::Ace::DimensionOffset newOffset = offset;
        OHOS::Ace::Dimension offsetX = isRtl ? newOffset.GetX() * (-1) : newOffset.GetX();
        newOffset.SetX(offsetX);
        dialogProps.offset = newOffset;

        if (maskRect.has_value()) {
            OHOS::Ace::DimensionRect newMaskRect = maskRect.value();
            auto rectOffset = newMaskRect.GetOffset();
            OHOS::Ace::Dimension offsetX = isRtl ? rectOffset.GetX() * (-1) : rectOffset.GetX();
            rectOffset.SetX(offsetX);
            newMaskRect.SetOffset(rectOffset);
            dialogProps.maskRect = newMaskRect;
        }
    };
    dialogProps.onLanguageChange = onLanguageChange;
    return true;
}

bool GetShowDialogOptions(ani_env* env, ani_object object, OHOS::Ace::DialogProperties& dialogProps)
{
    if (IsUndefinedObject(env, object)) {
        return false;
    }

    if (!IsClassObject(env, object, "L@ohos/promptAction/promptAction/ShowDialogOptions;")) {
        return false;
    }

    GetResourceStrParam(env, object, "title", dialogProps.title);
    GetResourceStrParam(env, object, "message", dialogProps.content);
    GetButtonArray(env, object, "buttons", dialogProps.buttons);
    GetMaskRect(env, object, dialogProps.maskRect);
    GetDialogAlignment(env, object, dialogProps.alignment);
    GetOffsetParam(env, object, dialogProps.offset);
    GetBoolParam(env, object, "showInSubWindow", dialogProps.isShowInSubWindow);
    GetBoolParam(env, object, "isModal", dialogProps.isModal);
    GetResourceColorParamOpt(env, object, "backgroundColor", dialogProps.backgroundColor);
    GetBackgroundBlurStyleParamOpt(env, object, dialogProps.backgroundBlurStyle);
    GetBackgroundBlurStyleOptions(env, object, dialogProps.blurStyleOption);
    GetBackgroundEffectOptions(env, object, dialogProps.effectOption);
    GetShadowParamOpt(env, object, dialogProps.shadow);
    bool enableHoverMode = dialogProps.enableHoverMode.value_or(false);
    GetBoolParam(env, object, "enableHoverMode", enableHoverMode);
    GetHoverModeAreaParamOpt(env, object, dialogProps.hoverModeArea);
    GetFunctionParam(env, object, "onDidAppear", dialogProps.onDidAppear);
    GetFunctionParam(env, object, "onDidDisappear", dialogProps.onDidDisappear);
    GetFunctionParam(env, object, "onWillAppear", dialogProps.onWillAppear);
    GetFunctionParam(env, object, "onWillDisappear", dialogProps.onWillDisappear);
    if (!dialogProps.isShowInSubWindow) {
        GetLevelMode(env, object, dialogProps.dialogLevelMode);
    }
    double levelUniqueId = -1;
    GetDoubleParam(env, object, "levelUniqueId", levelUniqueId);
    dialogProps.dialogLevelUniqueId = static_cast<int32_t>(levelUniqueId);
    GetImmersiveMode(env, object, dialogProps.dialogImmersiveMode);
    GetOnLanguageChange(dialogProps);
    return true;
}

bool GetShowDialogOptionsInternal(ani_env* env, ani_object object, OHOS::Ace::DialogProperties& dialogProps)
{
    if (IsUndefinedObject(env, object)) {
        return false;
    }

    if (!IsClassObject(env, object, "L@ohos/promptAction/promptAction/ShowDialogOptionsInternal;")) {
        return false;
    }

    if (!dialogProps.isShowInSubWindow) {
        GetDoubleParamOpt(env, object, "levelOrder", dialogProps.levelOrder);
    }
    return true;
}

ani_ref CreateShowDialogSuccessResponse(ani_env* env, int32_t index)
{
    ani_class responseCls;
    ani_status status = env->FindClass(
        "L@ohos/promptAction/promptAction/ShowDialogSuccessResponseInner;", &responseCls);
    if (status != ANI_OK) {
        TAG_LOGE(OHOS::Ace::AceLogTag::ACE_OVERLAY, "FindClass failed %{public}d", status);
        return nullptr;
    }

    ani_method ctorMethod;
    status = env->Class_FindMethod(responseCls, "<ctor>", nullptr, &ctorMethod);
    if (status != ANI_OK) {
        return nullptr;
    }

    ani_object responseObj;
    status = env->Object_New(responseCls, ctorMethod, &responseObj);
    if (status != ANI_OK) {
        return nullptr;
    }

    ani_double aniIndex = static_cast<ani_double>(index);
    status = env->Object_SetPropertyByName_Double(responseObj, "index", aniIndex);
    if (status != ANI_OK) {
        return nullptr;
    }
    return reinterpret_cast<ani_ref>(responseObj);
}

std::function<void(int32_t, int32_t)> GetShowDialogCallback(std::shared_ptr<PromptActionAsyncContext>& asyncContext)
{
    auto callback = [asyncContext](int32_t errorCode, int32_t successIndex) mutable {
        if (!asyncContext) {
            return;
        }

        auto container = OHOS::Ace::AceEngine::Get().GetContainer(asyncContext->instanceId);
        if (!container) {
            return;
        }

        auto taskExecutor = container->GetTaskExecutor();
        if (!taskExecutor) {
            return;
        }

        auto task = [asyncContext, errorCode, successIndex]() {
            if (asyncContext == nullptr) {
                return;
            }

            ani_size nrRefs = 16;
            ani_status status = asyncContext->env->CreateLocalScope(nrRefs);
            if (status != ANI_OK && !nrRefs) {
                return;
            }

            std::vector<ani_ref> args(CALLBACK_PARAM_LENGTH);
            if (errorCode == OHOS::Ace::ERROR_CODE_NO_ERROR) {
                asyncContext->env->GetNull(&args[0]);
            } else {
                args[0] = CreateBusinessError(asyncContext->env, errorCode, "cancel");
            }
            args[1] = CreateShowDialogSuccessResponse(asyncContext->env, successIndex);
            ani_ref fnReturnVal {};
            status = asyncContext->env->FunctionalObject_Call(
                asyncContext->callback, args.size(), args.data(), &fnReturnVal);
            status = asyncContext->env->DestroyLocalScope();
        };
        taskExecutor->PostTask(
            std::move(task), OHOS::Ace::TaskExecutor::TaskType::JS, "ArkUIDialogParseDialogCallback");
        asyncContext = nullptr;
    };
    return callback;
}

std::function<void(int32_t, int32_t)> GetShowDialogPromise(std::shared_ptr<PromptActionAsyncContext>& asyncContext)
{
    auto callback = [asyncContext](int32_t callbackType, int32_t successIndex) mutable {
        if (!asyncContext) {
            return;
        }

        auto container = OHOS::Ace::AceEngine::Get().GetContainer(asyncContext->instanceId);
        if (!container) {
            return;
        }

        auto taskExecutor = container->GetTaskExecutor();
        if (!taskExecutor) {
            return;
        }

        auto task = [asyncContext, callbackType, successIndex]() {
            if (asyncContext == nullptr) {
                return;
            }

            if (!asyncContext->deferred) {
                return;
            }

            ani_size nrRefs = 16;
            ani_status status = asyncContext->env->CreateLocalScope(nrRefs);
            if (status != ANI_OK && !nrRefs) {
                return;
            }

            if (callbackType == 0) {
                ani_ref responseRef = CreateShowDialogSuccessResponse(asyncContext->env, successIndex);
                status = asyncContext->env->PromiseResolver_Resolve(asyncContext->deferred, responseRef);
                if (status != ANI_OK) {
                    TAG_LOGW(OHOS::Ace::AceLogTag::ACE_DIALOG,
                        "[ANI] PromiseResolver_Resolve fail. status: %{public}d", status);
                }
            } else {
                ani_ref errorRef = CreateBusinessError(asyncContext->env, 0, "cancel");
                ani_error error = static_cast<ani_error>(errorRef);
                status = asyncContext->env->PromiseResolver_Reject(asyncContext->deferred, error);
                if (status != ANI_OK) {
                    TAG_LOGW(OHOS::Ace::AceLogTag::ACE_OVERLAY,
                        "[ANI] PromiseResolver_Reject fail. status: %{public}d", status);
                }
            }
            status = asyncContext->env->DestroyLocalScope();
        };
        taskExecutor->PostTask(
            std::move(task), OHOS::Ace::TaskExecutor::TaskType::JS, "ArkUIDialogParseCustomDialogIdCallback");
        asyncContext = nullptr;
    };
    return callback;
}

bool GetActionMenuButtons(ani_env *env, ani_object object, std::vector<OHOS::Ace::ButtonInfo>& result)
{
    ani_ref resultRef;
    ani_status status = env->Object_GetPropertyByName_Ref(object, "buttons", &resultRef);
    if (status != ANI_OK) {
        return false;
    }

    if (IsUndefinedObject(env, resultRef)) {
        return false;
    }

    ani_size length;
    ani_tuple_value resultObj = static_cast<ani_tuple_value>(resultRef);
    status = env->TupleValue_GetNumberOfItems(resultObj, &length);
    if (status != ANI_OK) {
        return false;
    }

    std::vector<OHOS::Ace::ButtonInfo> buttonArray;
    for (int i = 0; i < int(length); i++) {
        ani_ref itemRef;
        status = env->Object_GetFieldByName_Ref(resultObj, ("$" + std::to_string(i)).c_str(), &itemRef);
        if (status != ANI_OK) {
            continue;
        }

        OHOS::Ace::ButtonInfo button;
        if (GetButtonInfo(env, itemRef, button)) {
            buttonArray.emplace_back(button);
        }
    }

    std::function<bool(OHOS::Ace::ButtonInfo)> isPrimary = [](OHOS::Ace::ButtonInfo button) {
        return button.isPrimary;
    };
    int32_t primaryButtonCount = std::count_if(buttonArray.begin(), buttonArray.end(), isPrimary);
    if (primaryButtonCount > PRIMARY_BUTTON_COUNT_MAX) {
        for (auto& button : buttonArray) {
            button.isPrimary = false;
        }
    }

    OHOS::Ace::ButtonInfo cancelButton = {
        .text = OHOS::Ace::Localization::GetInstance()->GetEntryLetters("common.cancel"),
        .textColor = "",
        .isPrimary = primaryButtonCount == 0,
    };
    buttonArray.emplace_back(cancelButton);
    result = buttonArray;
    return true;
}

bool GetActionMenuOptions(ani_env* env, ani_object object, OHOS::Ace::DialogProperties& dialogProps)
{
    if (IsUndefinedObject(env, object)) {
        return false;
    }

    if (!IsClassObject(env, object, "L@ohos/promptAction/promptAction/ActionMenuOptions;")) {
        return false;
    }

    GetResourceStrParam(env, object, "title", dialogProps.title);
    GetActionMenuButtons(env, object, dialogProps.buttons);
    GetBoolParam(env, object, "showInSubWindow", dialogProps.isShowInSubWindow);
    GetBoolParam(env, object, "isModal", dialogProps.isModal);
    if (!dialogProps.isShowInSubWindow) {
        GetLevelMode(env, object, dialogProps.dialogLevelMode);
    }
    double levelUniqueId = -1;
    GetDoubleParam(env, object, "levelUniqueId", levelUniqueId);
    dialogProps.dialogLevelUniqueId = static_cast<int32_t>(levelUniqueId);
    GetImmersiveMode(env, object, dialogProps.dialogImmersiveMode);
    return true;
}

ani_ref CreateActionMenuSuccessResponse(ani_env* env, int32_t index)
{
    ani_class responseCls;
    ani_status status = env->FindClass(
        "L@ohos/promptAction/promptAction/ActionMenuSuccessResponseInner;", &responseCls);
    if (status != ANI_OK) {
        TAG_LOGE(OHOS::Ace::AceLogTag::ACE_OVERLAY, "FindClass failed %{public}d", status);
        return nullptr;
    }

    ani_method ctorMethod;
    status = env->Class_FindMethod(responseCls, "<ctor>", nullptr, &ctorMethod);
    if (status != ANI_OK) {
        return nullptr;
    }

    ani_object responseObj;
    status = env->Object_New(responseCls, ctorMethod, &responseObj);
    if (status != ANI_OK) {
        return nullptr;
    }

    ani_double aniIndex = static_cast<ani_double>(index);
    status = env->Object_SetPropertyByName_Double(responseObj, "index", aniIndex);
    if (status != ANI_OK) {
        return nullptr;
    }
    return reinterpret_cast<ani_ref>(responseObj);
}

std::function<void(int32_t, int32_t)> GetShowActionMenuCallback(
    std::shared_ptr<PromptActionAsyncContext>& asyncContext)
{
    auto callback = [asyncContext](int32_t errorCode, int32_t successIndex) mutable {
        if (!asyncContext) {
            return;
        }

        auto container = OHOS::Ace::AceEngine::Get().GetContainer(asyncContext->instanceId);
        if (!container) {
            return;
        }

        auto taskExecutor = container->GetTaskExecutor();
        if (!taskExecutor) {
            return;
        }

        auto task = [asyncContext, errorCode, successIndex]() {
            if (asyncContext == nullptr) {
                return;
            }

            ani_size nrRefs = 16;
            ani_status status = asyncContext->env->CreateLocalScope(nrRefs);
            if (status != ANI_OK && !nrRefs) {
                return;
            }

            std::vector<ani_ref> args(CALLBACK_PARAM_LENGTH);
            if (errorCode == OHOS::Ace::ERROR_CODE_NO_ERROR) {
                asyncContext->env->GetNull(&args[0]);
            } else {
                args[0] = CreateBusinessError(asyncContext->env, errorCode, "cancel");
            }
            args[1] = CreateActionMenuSuccessResponse(asyncContext->env, successIndex);
            ani_ref fnReturnVal {};
            status = asyncContext->env->FunctionalObject_Call(
                asyncContext->callback, args.size(), args.data(), &fnReturnVal);
            status = asyncContext->env->DestroyLocalScope();
        };
        taskExecutor->PostTask(
            std::move(task), OHOS::Ace::TaskExecutor::TaskType::JS, "ArkUIDialogParseActionMenuCallback");
        asyncContext = nullptr;
    };
    return callback;
}

std::function<void(int32_t, int32_t)> GetShowActionMenuPromise(std::shared_ptr<PromptActionAsyncContext>& asyncContext)
{
    auto callback = [asyncContext](int32_t callbackType, int32_t successIndex) mutable {
        if (!asyncContext) {
            return;
        }

        auto container = OHOS::Ace::AceEngine::Get().GetContainer(asyncContext->instanceId);
        if (!container) {
            return;
        }

        auto taskExecutor = container->GetTaskExecutor();
        if (!taskExecutor) {
            return;
        }

        auto task = [asyncContext, callbackType, successIndex]() {
            if (asyncContext == nullptr) {
                return;
            }

            if (!asyncContext->deferred) {
                return;
            }

            ani_size nrRefs = 16;
            ani_status status = asyncContext->env->CreateLocalScope(nrRefs);
            if (status != ANI_OK && !nrRefs) {
                return;
            }

            if (callbackType == 0) {
                ani_ref responseRef = CreateActionMenuSuccessResponse(asyncContext->env, successIndex);
                status = asyncContext->env->PromiseResolver_Resolve(asyncContext->deferred, responseRef);
                if (status != ANI_OK) {
                    TAG_LOGW(OHOS::Ace::AceLogTag::ACE_DIALOG,
                        "[ANI] PromiseResolver_Resolve fail. status: %{public}d", status);
                }
            } else {
                ani_ref errorRef = CreateBusinessError(asyncContext->env, 0, "cancel");
                ani_error error = static_cast<ani_error>(errorRef);
                status = asyncContext->env->PromiseResolver_Reject(asyncContext->deferred, error);
                if (status != ANI_OK) {
                    TAG_LOGE(OHOS::Ace::AceLogTag::ACE_OVERLAY,
                        "[ANI] PromiseResolver_Reject fail. status: %{public}d", status);
                }
            }
            status = asyncContext->env->DestroyLocalScope();
        };
        taskExecutor->PostTask(
            std::move(task), OHOS::Ace::TaskExecutor::TaskType::JS, "ArkUIDialogParseActionMenuCallback");
        asyncContext = nullptr;
    };
    return callback;
}

bool GetOnWillDismiss(ani_env* env, ani_object object,
    std::function<void(const int32_t& reason, const int32_t& instanceId)>& result)
{
    ani_ref resultRef;
    ani_status status = env->Object_GetPropertyByName_Ref(object, "onWillDismiss", &resultRef);
    if (status != ANI_OK) {
        return false;
    }

    if (IsUndefinedObject(env, resultRef)) {
        return false;
    }

    ani_ref globalRef;
    env->GlobalReference_Create(resultRef, &globalRef);
    result = [env, globalRef](const int32_t reason, const int32_t instanceId) {
        TAG_LOGI(OHOS::Ace::AceLogTag::ACE_DIALOG,
            "Dissmiss dialog enter. reason: %{public}d, instanceId: %{public}d", reason, instanceId);
        if (globalRef) {
            ani_object dismissDialogAction = OHOS::Ace::Ani::ANICreateDismissDialogAction(env, reason, instanceId);
            ani_ref actionRef = static_cast<ani_ref>(dismissDialogAction);
            ani_fn_object func = static_cast<ani_fn_object>(globalRef);
            ani_ref fnReturnVal {};
            ani_status status = env->FunctionalObject_Call(func, 1, &actionRef, &fnReturnVal);
            if (status != ANI_OK) {
                TAG_LOGE(OHOS::Ace::AceLogTag::ACE_DIALOG, "Dissmiss dialog fail. status: %{public}d", status);
            }
        }
    };
    return true;
}

bool GetKeyboardAvoidMode(ani_env* env, ani_object object, OHOS::Ace::KeyboardAvoidMode& result)
{
    ani_int resultInt;
    if (!GetEnumInt(env, object, "keyboardAvoidMode", "Larkui/component/common/KeyboardAvoidMode;", resultInt)) {
        return false;
    }
    result = static_cast<OHOS::Ace::KeyboardAvoidMode>(resultInt);
    return true;
}

bool GetKeyboardAvoidDistance(ani_env *env, ani_object object, std::optional<OHOS::Ace::Dimension>& result)
{
    ani_ref resultRef;
    ani_status status = env->Object_GetPropertyByName_Ref(object, "keyboardAvoidDistance", &resultRef);
    if (status != ANI_OK) {
        return false;
    }

    if (IsUndefinedObject(env, resultRef)) {
        return false;
    }

    ani_double aniValue;
    ani_object resultObj = static_cast<ani_object>(resultRef);
    status = env->Object_GetPropertyByName_Double(resultObj, "value", &aniValue);
    if (status != ANI_OK) {
        return false;
    }

    int32_t unit;
    if (!GetEnumInt(env, resultObj, "unit", "Larkui/Graphics/LengthUnit;", unit)) {
        return false;
    }

    double value = static_cast<double>(aniValue);
    auto dimensionUnit = static_cast<OHOS::Ace::DimensionUnit>(unit);
    if (OHOS::Ace::GreatOrEqual(value, 0.0) && dimensionUnit >= OHOS::Ace::DimensionUnit::PX
        && dimensionUnit <= OHOS::Ace::DimensionUnit::CALC && dimensionUnit != OHOS::Ace::DimensionUnit::PERCENT) {
        OHOS::Ace::Dimension dimension(value, dimensionUnit);
        result = dimension;
    } else {
        OHOS::Ace::Dimension dimension(DEFAULT_AVOID_DISTANCE, OHOS::Ace::DimensionUnit::VP);
        result = dimension;
    }
    return true;
}

bool GetBaseDialogOptions(ani_env* env, ani_object object, OHOS::Ace::DialogProperties& dialogProps)
{
    if (IsUndefinedObject(env, object)) {
        return false;
    }

    if (!IsClassObject(env, object, "L@ohos/promptAction/promptAction/BaseDialogOptions;")) {
        return false;
    }

    GetMaskRect(env, object, dialogProps.maskRect);
    GetDialogAlignment(env, object, dialogProps.alignment);
    GetOffsetParam(env, object, dialogProps.offset);
    GetBoolParam(env, object, "showInSubWindow", dialogProps.isShowInSubWindow);
    GetBoolParam(env, object, "isModal", dialogProps.isModal);
    GetBoolParam(env, object, "autoCancel", dialogProps.autoCancel);
    GetResourceColorParamOpt(env, object, "maskColor", dialogProps.maskColor);
    GetOnWillDismiss(env, object, dialogProps.onWillDismiss);
    GetFunctionParam(env, object, "onDidAppear", dialogProps.onDidAppear);
    GetFunctionParam(env, object, "onDidDisappear", dialogProps.onDidDisappear);
    GetFunctionParam(env, object, "onWillAppear", dialogProps.onWillAppear);
    GetFunctionParam(env, object, "onWillDisappear", dialogProps.onWillDisappear);
    GetKeyboardAvoidMode(env, object, dialogProps.keyboardAvoidMode);
    bool enableHoverMode = dialogProps.enableHoverMode.value_or(false);
    GetBoolParam(env, object, "enableHoverMode", enableHoverMode);
    GetHoverModeAreaParamOpt(env, object, dialogProps.hoverModeArea);
    GetBackgroundBlurStyleOptions(env, object, dialogProps.blurStyleOption);
    GetBackgroundEffectOptions(env, object, dialogProps.effectOption);
    GetKeyboardAvoidDistance(env, object, dialogProps.keyboardAvoidDistance);
    if (!dialogProps.isShowInSubWindow) {
        GetLevelMode(env, object, dialogProps.dialogLevelMode);
    }
    double levelUniqueId = -1;
    GetDoubleParam(env, object, "levelUniqueId", levelUniqueId);
    dialogProps.dialogLevelUniqueId = static_cast<int32_t>(levelUniqueId);
    GetImmersiveMode(env, object, dialogProps.dialogImmersiveMode);
    GetBoolParam(env, object, "focusable", dialogProps.focusable);
    return true;
}

bool GetTransitionEffectParam(ani_env* env, ani_object object, const char *name,
    OHOS::Ace::RefPtr<OHOS::Ace::NG::ChainedTransitionEffect>& result)
{
    int64_t transitionEffectPtr;
    if (!GetInt64Param(env, object, name, transitionEffectPtr)) {
        return false;
    }

    Ark_TransitionEffect transitionEffect = (Ark_TransitionEffect)transitionEffectPtr;
    CHECK_NULL_RETURN(transitionEffect, false);
    CHECK_NULL_RETURN(transitionEffect->handler, false);
    result = transitionEffect->handler;
    return true;
}

bool GetDialogOptionsInternal(ani_env* env, ani_object object, OHOS::Ace::DialogProperties& dialogProps)
{
    if (IsUndefinedObject(env, object)) {
        return false;
    }

    if (!IsClassObject(env, object, "L@ohos/promptAction/promptAction/DialogOptionsInternal;")) {
        return false;
    }

    GetTransitionEffectParam(env, object, "transition", dialogProps.transitionEffect);
    GetTransitionEffectParam(env, object, "dialogTransition", dialogProps.dialogTransitionEffect);
    GetTransitionEffectParam(env, object, "maskTransition", dialogProps.maskTransitionEffect);
    if (!dialogProps.isShowInSubWindow) {
        GetDoubleParamOpt(env, object, "levelOrder", dialogProps.levelOrder);
    }
    return true;
}

std::function<void()> GetCustomBuilder(ani_env *env, ani_long builder)
{
    auto result = [env, builder]() {
        if (builder) {
            auto* builderNode = reinterpret_cast<ArkUINodeHandle>(builder);
            CHECK_NULL_VOID(builderNode);
            auto uiNode = OHOS::Ace::AceType::Claim(reinterpret_cast<OHOS::Ace::NG::UINode *>(builderNode));
            CHECK_NULL_VOID(uiNode);
            OHOS::Ace::NG::ViewStackProcessor::GetInstance()->Push(uiNode);
        }
    };
    return result;
}

bool GetCornerRadius(ani_env *env, ani_object object, std::optional<OHOS::Ace::NG::BorderRadiusProperty>& result)
{
    ani_ref resultRef;
    ani_status status = env->Object_GetPropertyByName_Ref(object, "cornerRadius", &resultRef);
    if (status != ANI_OK) {
        return false;
    }

    if (IsUndefinedObject(env, resultRef)) {
        return false;
    }

    ani_object resultObj = static_cast<ani_object>(resultRef);
    OHOS::Ace::CalcDimension dimension;
    if (GetDimensionParam(env, resultObj, dimension)) {
        CheckDimension(dimension);
        result = OHOS::Ace::NG::BorderRadiusProperty(dimension);
        return true;
    }

    OHOS::Ace::NG::BorderRadiusProperty borderRadius;
    OHOS::Ace::CalcDimension topLeft;
    if (GetDimensionParam(env, resultObj, "topLeft", topLeft)) {
        CheckDimension(topLeft);
        borderRadius.radiusTopLeft = topLeft;
    }

    OHOS::Ace::CalcDimension topRight;
    if (GetDimensionParam(env, resultObj, "topRight", topRight)) {
        CheckDimension(topRight);
        borderRadius.radiusTopRight = topRight;
    }

    OHOS::Ace::CalcDimension bottomLeft;
    if (GetDimensionParam(env, resultObj, "bottomLeft", bottomLeft)) {
        CheckDimension(bottomLeft);
        borderRadius.radiusBottomLeft = bottomLeft;
    }

    OHOS::Ace::CalcDimension bottomRight;
    if (GetDimensionParam(env, resultObj, "bottomRight", bottomRight)) {
        CheckDimension(bottomRight);
        borderRadius.radiusBottomRight = bottomRight;
    }
    borderRadius.multiValued = true;
    result = std::make_optional<OHOS::Ace::NG::BorderRadiusProperty>(borderRadius);
    return true;
}

bool GetBorderWidth(ani_env *env, ani_object object, std::optional<OHOS::Ace::NG::BorderWidthProperty>& result)
{
    ani_ref resultRef;
    ani_status status = env->Object_GetPropertyByName_Ref(object, "borderWidth", &resultRef);
    if (status != ANI_OK) {
        return false;
    }

    if (IsUndefinedObject(env, resultRef)) {
        return false;
    }

    ani_object resultObj = static_cast<ani_object>(resultRef);
    OHOS::Ace::CalcDimension dimension;
    if (GetDimensionParam(env, resultObj, dimension)) {
        CheckDimension(dimension);
        result = OHOS::Ace::NG::BorderWidthProperty({ dimension, dimension, dimension, dimension });
        return true;
    }

    OHOS::Ace::NG::BorderWidthProperty borderWidth;
    OHOS::Ace::CalcDimension left;
    if (GetDimensionParam(env, resultObj, "left", left)) {
        CheckDimension(left);
        borderWidth.leftDimen = left;
    }

    OHOS::Ace::CalcDimension right;
    if (GetDimensionParam(env, resultObj, "right", right)) {
        CheckDimension(left);
        borderWidth.rightDimen = right;
    }

    OHOS::Ace::CalcDimension top;
    if (GetDimensionParam(env, resultObj, "top", top)) {
        CheckDimension(top);
        borderWidth.topDimen = top;
    }

    OHOS::Ace::CalcDimension bottom;
    if (GetDimensionParam(env, resultObj, "bottom", bottom)) {
        CheckDimension(bottom);
        borderWidth.bottomDimen = bottom;
    }
    borderWidth.multiValued = true;
    result = std::make_optional<OHOS::Ace::NG::BorderWidthProperty>(borderWidth);
    return true;
}

bool GetBorderColor(ani_env *env, ani_object object, std::optional<OHOS::Ace::NG::BorderColorProperty>& result)
{
    ani_ref resultRef;
    ani_status status = env->Object_GetPropertyByName_Ref(object, "borderColor", &resultRef);
    if (status != ANI_OK) {
        return false;
    }

    if (IsUndefinedObject(env, resultRef)) {
        return false;
    }

    OHOS::Ace::NG::BorderColorProperty borderColor;
    ani_object resultObj = static_cast<ani_object>(resultRef);
    OHOS::Ace::Color color;
    if (GetResourceColorParam(env, resultObj, color)) {
        borderColor.SetColor(color);
        result = std::make_optional<OHOS::Ace::NG::BorderColorProperty>(borderColor);
        return true;
    }

    OHOS::Ace::Color left;
    if (GetResourceColorParam(env, resultObj, "left", left)) {
        borderColor.leftColor = left;
    }

    OHOS::Ace::Color right;
    if (GetResourceColorParam(env, resultObj, "right", right)) {
        borderColor.rightColor = right;
    }

    OHOS::Ace::Color top;
    if (GetResourceColorParam(env, resultObj, "top", top)) {
        borderColor.topColor = top;
    }

    OHOS::Ace::Color bottom;
    if (GetResourceColorParam(env, resultObj, "bottom", bottom)) {
        borderColor.bottomColor = bottom;
    }
    borderColor.multiValued = true;
    result = std::make_optional<OHOS::Ace::NG::BorderColorProperty>(borderColor);
    return true;
}

bool GetBorderStyle(ani_env* env, ani_object object, OHOS::Ace::BorderStyle& result)
{
    ani_int resultInt;
    if (!GetEnumInt(env, object, "Larkui/component/enums/BorderStyle;", resultInt)) {
        return false;
    }
    result = static_cast<OHOS::Ace::BorderStyle>(resultInt);
    return true;
}

bool GetBorderStyle(ani_env* env, ani_object object, const char *name, OHOS::Ace::BorderStyle& result)
{
    ani_int resultInt;
    if (!GetEnumInt(env, object, name, "Larkui/component/enums/BorderStyle;", resultInt)) {
        return false;
    }
    result = static_cast<OHOS::Ace::BorderStyle>(resultInt);
    return true;
}

bool GetBorderStyle(ani_env *env, ani_object object, std::optional<OHOS::Ace::NG::BorderStyleProperty>& result)
{
    ani_ref resultRef;
    ani_status status = env->Object_GetPropertyByName_Ref(object, "borderStyle", &resultRef);
    if (status != ANI_OK) {
        return false;
    }

    if (IsUndefinedObject(env, resultRef)) {
        return false;
    }

    ani_object resultObj = static_cast<ani_object>(resultRef);
    OHOS::Ace::BorderStyle style;
    if (GetBorderStyle(env, resultObj, style)) {
        result = OHOS::Ace::NG::BorderStyleProperty({ style, style, style, style });
        return true;
    }

    OHOS::Ace::NG::BorderStyleProperty borderStyle;
    OHOS::Ace::BorderStyle left;
    if (GetBorderStyle(env, resultObj, "left", left)) {
        borderStyle.styleLeft = left;
    }

    OHOS::Ace::BorderStyle right;
    if (GetBorderStyle(env, resultObj, "right", right)) {
        borderStyle.styleRight = right;
    }

    OHOS::Ace::BorderStyle top;
    if (GetBorderStyle(env, resultObj, "top", top)) {
        borderStyle.styleTop = top;
    }

    OHOS::Ace::BorderStyle bottom;
    if (GetBorderStyle(env, resultObj, "bottom", bottom)) {
        borderStyle.styleBottom = bottom;
    }
    borderStyle.multiValued = true;
    result = std::make_optional<OHOS::Ace::NG::BorderStyleProperty>(borderStyle);
    return true;
}

bool GetCustomDialogOptions(ani_env* env, ani_object object, OHOS::Ace::DialogProperties& dialogProps)
{
    if (IsUndefinedObject(env, object)) {
        return false;
    }

    if (!IsClassObject(env, object, "L@ohos/promptAction/promptAction/CustomDialogOptions;")) {
        return false;
    }

    GetBaseDialogOptions(env, object, dialogProps);
    GetResourceColorParamOpt(env, object, "backgroundColor", dialogProps.backgroundColor);
    GetCornerRadius(env, object, dialogProps.borderRadius);
    GetDimensionParamOpt(env, object, "width", dialogProps.width);
    GetDimensionParamOpt(env, object, "height", dialogProps.height);
    GetBorderWidth(env, object, dialogProps.borderWidth);
    GetBorderColor(env, object, dialogProps.borderColor);
    GetBorderStyle(env, object, dialogProps.borderStyle);
    GetBackgroundBlurStyleParamOpt(env, object, dialogProps.backgroundBlurStyle);
    GetShadowParamOpt(env, object, dialogProps.shadow);
    return true;
}

std::function<void(int32_t)> GetOpenCustomDialogPromise(std::shared_ptr<PromptActionAsyncContext>& asyncContext)
{
    auto callback = [asyncContext](int32_t dialogId) mutable {
        if (!asyncContext) {
            return;
        }

        auto container = OHOS::Ace::AceEngine::Get().GetContainer(asyncContext->instanceId);
        if (!container) {
            return;
        }

        auto taskExecutor = container->GetTaskExecutor();
        if (!taskExecutor) {
            return;
        }

        auto task = [asyncContext, dialogId]() {
            if (asyncContext == nullptr) {
                return;
            }

            if (!asyncContext->deferred) {
                return;
            }

            ani_size nrRefs = 16;
            asyncContext->env->CreateLocalScope(nrRefs);
            if (!nrRefs) {
                return;
            }

            if (dialogId > 0) {
                double returnDialogId = static_cast<double>(dialogId);
                ani_object dialogIdObj = CreateANIDoubleObject(asyncContext->env, returnDialogId);
                ani_ref dialogRef = static_cast<ani_ref>(dialogIdObj);
                ani_status status = asyncContext->env->PromiseResolver_Resolve(asyncContext->deferred, dialogRef);
                if (status != ANI_OK) {
                    TAG_LOGW(OHOS::Ace::AceLogTag::ACE_DIALOG, "[ANI] PromiseResolver_Resolve fail.");
                }
            } else {
                int32_t errorCode = OHOS::Ace::ERROR_CODE_INTERNAL_ERROR;
                std::string errorMsg = OHOS::Ace::Ani::GetErrorMsg(errorCode);
                ani_ref errorRef = CreateBusinessError(asyncContext->env, errorCode, errorMsg);
                ani_error error = static_cast<ani_error>(errorRef);
                ani_status status = asyncContext->env->PromiseResolver_Reject(asyncContext->deferred, error);
                if (status != ANI_OK) {
                    TAG_LOGW(OHOS::Ace::AceLogTag::ACE_DIALOG, "[ANI] PromiseResolver_Reject fail.");
                }
            }
            asyncContext->env->DestroyLocalScope();
        };
        taskExecutor->PostTask(
            std::move(task), OHOS::Ace::TaskExecutor::TaskType::JS, "ArkUIDialogParseCustomDialogIdCallback");
        asyncContext = nullptr;
    };
    return callback;
}

std::function<void(int32_t)> GetCustomDialogContentPromise(std::shared_ptr<PromptActionAsyncContext>& asyncContext)
{
    auto callback = [asyncContext](int32_t errorCode) mutable {
        if (!asyncContext) {
            return;
        }

        auto container = OHOS::Ace::AceEngine::Get().GetContainer(asyncContext->instanceId);
        if (!container) {
            return;
        }

        auto taskExecutor = container->GetTaskExecutor();
        if (!taskExecutor) {
            return;
        }

        auto task = [asyncContext, errorCode]() {
            if (asyncContext == nullptr) {
                return;
            }

            if (!asyncContext->deferred) {
                return;
            }

            ani_size nrRefs = 16;
            asyncContext->env->CreateLocalScope(nrRefs);
            if (!nrRefs) {
                return;
            }

            if (errorCode == OHOS::Ace::ERROR_CODE_NO_ERROR) {
                ani_ref successRef;
                asyncContext->env->GetUndefined(&successRef);
                ani_status status = asyncContext->env->PromiseResolver_Resolve(asyncContext->deferred, successRef);
                if (status != ANI_OK) {
                    TAG_LOGW(OHOS::Ace::AceLogTag::ACE_DIALOG, "[ANI] PromiseResolver_Resolve fail.");
                }
            } else {
                std::string strMsg = OHOS::Ace::Ani::GetErrorMsg(errorCode);
                ani_ref errorRef = CreateBusinessError(asyncContext->env, errorCode, strMsg);
                ani_error error = static_cast<ani_error>(errorRef);
                ani_status status = asyncContext->env->PromiseResolver_Reject(asyncContext->deferred, error);
                if (status != ANI_OK) {
                    TAG_LOGW(OHOS::Ace::AceLogTag::ACE_DIALOG, "[ANI] PromiseResolver_Reject fail.");
                }
            }
            asyncContext->env->DestroyLocalScope();
        };
        taskExecutor->PostTask(
            std::move(task), OHOS::Ace::TaskExecutor::TaskType::JS, "ArkUIDialogParseCustomDialogContentCallback");
        asyncContext = nullptr;
    };
    return callback;
}

bool GetDialogOptions(ani_env* env, ani_object object, OHOS::Ace::DialogProperties& dialogProps)
{
    if (IsUndefinedObject(env, object)) {
        return false;
    }

    if (!IsClassObject(env, object, "L@ohos/promptAction/promptAction/DialogOptions;")) {
        return false;
    }

    GetBaseDialogOptions(env, object, dialogProps);
    GetResourceColorParamOpt(env, object, "backgroundColor", dialogProps.backgroundColor);
    GetCornerRadius(env, object, dialogProps.borderRadius);
    GetDimensionParamOpt(env, object, "width", dialogProps.width);
    GetDimensionParamOpt(env, object, "height", dialogProps.height);
    GetBorderWidth(env, object, dialogProps.borderWidth);
    GetBorderColor(env, object, dialogProps.borderColor);
    GetBorderStyle(env, object, dialogProps.borderStyle);
    GetBackgroundBlurStyleParamOpt(env, object, dialogProps.backgroundBlurStyle);
    GetShadowParamOpt(env, object, dialogProps.shadow);
    return true;
}
