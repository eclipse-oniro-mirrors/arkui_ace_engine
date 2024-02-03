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

#include "frameworks/bridge/declarative_frontend/jsview/js_navdestination.h"

#include "base/log/ace_scoring_log.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "bridge/declarative_frontend/engine/functions/js_function.h"
#include "bridge/declarative_frontend/engine/js_execution_scope_defines.h"
#include "bridge/declarative_frontend/engine/js_ref_ptr.h"
#include "bridge/declarative_frontend/engine/js_types.h"
#include "bridge/declarative_frontend/jsview/js_navdestination_context.h"
#include "bridge/declarative_frontend/jsview/js_navigation.h"
#include "bridge/declarative_frontend/jsview/js_utils.h"
#include "core/components_ng/base/view_stack_model.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/navrouter/navdestination_model_ng.h"

namespace OHOS::Ace {
std::unique_ptr<NavDestinationModel> NavDestinationModel::instance_ = nullptr;
std::mutex NavDestinationModel::mutex_;

NavDestinationModel* NavDestinationModel::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        instance_.reset(new NG::NavDestinationModelNG());
    }
    return instance_.get();
}

} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {

namespace {

bool ParseCommonTitle(const JSRef<JSObject>& jsObj)
{
    JSRef<JSVal> subtitle = jsObj->GetProperty("sub");
    JSRef<JSVal> title = jsObj->GetProperty("main");
    bool hasSub = subtitle->IsString();
    bool hasMain = title->IsString();
    if (hasSub || hasMain) {
        return NG::NavDestinationModelNG::GetInstance()->ParseCommonTitle(
            hasSub, hasMain, subtitle->ToString(), title->ToString());
    }
    return false;
}

} // namespace

void JSNavDestination::Create()
{
    NavDestinationModel::GetInstance()->Create();
}

void JSNavDestination::Create(const JSCallbackInfo& info)
{
    if (info.Length() <= 0 && !info[0]->IsFunction()) {
        NavDestinationModel::GetInstance()->Create();
        return;
    }

    auto builderFunctionJS = info[0];
    auto builderFunc = [context = info.GetExecutionContext(), builder = std::move(builderFunctionJS)]() {
        JAVASCRIPT_EXECUTION_SCOPE(context)
        JSRef<JSFunc>::Cast(builder)->Call(JSRef<JSObject>());
    };
    auto ctx = AceType::MakeRefPtr<JSNavDestinationContext>();
    auto navPathInfo = AceType::MakeRefPtr<JSNavPathInfo>();
    ctx->SetNavPathInfo(navPathInfo);
    NavDestinationModel::GetInstance()->Create(std::move(builderFunc), std::move(ctx));
}

void JSNavDestination::SetHideTitleBar(bool hide)
{
    NavDestinationModel::GetInstance()->SetHideTitleBar(hide);
}

void JSNavDestination::SetTitle(const JSCallbackInfo& info)
{
    // Resource and string type.
    std::string title;
    if (ParseJsString(info[0], title)) {
        NavDestinationModel::GetInstance()->ParseCommonTitle(false, true, "", title);
    } else if (info[0]->IsObject()) {
        JSRef<JSObject> jsObj = JSRef<JSObject>::Cast(info[0]);
        do {
            // NavigationCommonTitle
            if (ParseCommonTitle(jsObj)) {
                break;
            }
            // CustomBuilder | NavigationCustomTitle
            CalcDimension titleHeight;
            if (!jsObj->HasProperty("height")) {
                NavDestinationModel::GetInstance()->SetTitleHeight(titleHeight, false);
                break;
            }
            JSRef<JSVal> height = jsObj->GetProperty("height");
            bool isValid = JSContainerBase::ParseJsDimensionVpNG(height, titleHeight);
            if (height->IsString()) {
                std::string heightValue;
                ParseJsString(height, heightValue);
                if (heightValue == NG::TITLE_MAIN_WITH_SUB) {
                    NavDestinationModel::GetInstance()->SetTitleHeight(NG::DOUBLE_LINE_TITLEBAR_HEIGHT);
                    break;
                }
                if (heightValue == NG::TITLE_MAIN) {
                    NavDestinationModel::GetInstance()->SetTitleHeight(NG::SINGLE_LINE_TITLEBAR_HEIGHT);
                    break;
                }
            }
            if (!isValid || titleHeight.Value() < 0) {
                NavDestinationModel::GetInstance()->SetTitleHeight(Dimension(), true);
                break;
            }
            NavDestinationModel::GetInstance()->SetTitleHeight(titleHeight);
        } while (0);
        JSRef<JSVal> builderObject = jsObj->GetProperty("builder");
        if (builderObject->IsFunction()) {
            ViewStackModel::GetInstance()->NewScope();
            JsFunction jsBuilderFunc(info.This(), JSRef<JSObject>::Cast(builderObject));
            ACE_SCORING_EVENT("Navdestination.title.builder");
            jsBuilderFunc.Execute();
            auto customNode = ViewStackModel::GetInstance()->Finish();
            NavDestinationModel::GetInstance()->SetCustomTitle(customNode);
        }
    } else {
        TAG_LOGI(AceLogTag::ACE_NAVIGATION, "SetTitle is undefined");
        NavDestinationModel::GetInstance()->ParseCommonTitle(false, false, "", "");
    }
}

void JSNavDestination::SetBackButtonIcon(const JSCallbackInfo& info)
{
    // srcType、pixmap、string
    if (info.Length() < 1) {
        return;
    }
    std::string src;
    auto noPixMap = ParseJsMedia(info[0], src);

    RefPtr<PixelMap> pixMap = nullptr;
#if defined(PIXEL_MAP_SUPPORTED)
    if (!noPixMap) {
        pixMap = CreatePixelMapFromNapiValue(info[0]);
    }
#endif
    std::string bundleName;
    std::string moduleName;
    GetJsMediaBundleInfo(info[0], bundleName, moduleName);
    NavDestinationModel::GetInstance()->SetBackButtonIcon(src, noPixMap, pixMap, bundleName, moduleName);
}

void JSNavDestination::SetOnShown(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }

    auto onShownCallback = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    WeakPtr<NG::FrameNode> targetNode = NG::ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto onShown = [execCtx = info.GetExecutionContext(), func = std::move(onShownCallback), node = targetNode]() {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("NavDestination.onShown");
        PipelineContext::SetCallBackNode(node);
        JSRef<JSVal> params[1];
        params[0] = JSRef<JSVal>::Make(ToJSValue("undefined"));
        func->ExecuteJS(1, params);
    };
    NavDestinationModel::GetInstance()->SetOnShown(std::move(onShown));
    info.ReturnSelf();
}

void JSNavDestination::SetOnHidden(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }
    auto onHiddenCallback = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    WeakPtr<NG::FrameNode> targetNode = NG::ViewStackProcessor::GetInstance()->GetMainFrameNode();
    auto onHidden = [execCtx = info.GetExecutionContext(), func = std::move(onHiddenCallback), node = targetNode]() {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("NavDestination.onHidden");
        PipelineContext::SetCallBackNode(node);
        func->ExecuteJS();
    };
    NavDestinationModel::GetInstance()->SetOnHidden(std::move(onHidden));
    info.ReturnSelf();
}

void JSNavDestination::SetOnBackPressed(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }
    auto onBackPressedCallback = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto onBackPressed = [execCtx = info.GetExecutionContext(), func = std::move(onBackPressedCallback)]() -> bool {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx, false);
        ACE_SCORING_EVENT("NavDestination.onBackPressed");
        return (func->ExecuteJS())->ToBoolean();
    };
    NavDestinationModel::GetInstance()->SetOnBackPressed(std::move(onBackPressed));
    info.ReturnSelf();
}

void JSNavDestination::SetOnReady(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }
    auto onReadyCallback = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto onReady = [execCtx = info.GetExecutionContext(), func = std::move(onReadyCallback)](
                       RefPtr<NG::NavDestinationContext> context) {
        auto jsContext = AceType::DynamicCast<JSNavDestinationContext>(context);
        CHECK_NULL_VOID(jsContext);
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("NavDestination.onReady");
        JSRef<JSVal> params[1];
        params[0] = jsContext->CreateJSObject();
        func->ExecuteJS(1, params);
    };
    NavDestinationModel::GetInstance()->SetOnReady(std::move(onReady));
    info.ReturnSelf();
}

void JSNavDestination::SetMode(const JSCallbackInfo& info)
{
    if (!info[0]->IsNumber()) {
        // set default back ground color
        NavDestinationModel::GetInstance()->SetNavDestinationMode(NG::NavDestinationMode::STANDARD);
        return;
    }
    auto mode = info[0]->ToNumber<int32_t>();
    NavDestinationModel::GetInstance()->SetNavDestinationMode(static_cast<NG::NavDestinationMode>(mode));
}

void JSNavDestination::JSBind(BindingTarget globalObj)
{
    JSClass<JSNavDestination>::Declare("NavDestination");
    JSClass<JSNavDestination>::StaticMethod("create", &JSNavDestination::Create);
    JSClass<JSNavDestination>::StaticMethod("title", &JSNavDestination::SetTitle);
    JSClass<JSNavDestination>::StaticMethod("hideTitleBar", &JSNavDestination::SetHideTitleBar);
    JSClass<JSNavDestination>::StaticMethod("backButtonIcon", &JSNavDestination::SetBackButtonIcon);
    JSClass<JSNavDestination>::StaticMethod("onShown", &JSNavDestination::SetOnShown);
    JSClass<JSNavDestination>::StaticMethod("onHidden", &JSNavDestination::SetOnHidden);
    JSClass<JSNavDestination>::StaticMethod("onBackPressed", &JSNavDestination::SetOnBackPressed);
    JSClass<JSNavDestination>::StaticMethod("onReady", &JSNavDestination::SetOnReady);
    JSClass<JSNavDestination>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSNavDestination>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSNavDestination>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSNavDestination>::StaticMethod("id", &JSViewAbstract::JsId);
    JSClass<JSNavDestination>::StaticMethod("mode", &JSNavDestination::SetMode);
    JSClass<JSNavDestination>::InheritAndBind<JSContainerBase>(globalObj);
}

} // namespace OHOS::Ace::Framework
