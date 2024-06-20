/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "bridge/declarative_frontend/jsview/js_menu_item.h"

#include "want.h"

#include "base/log/ace_scoring_log.h"
#include "base/log/log_wrapper.h"
#include "bridge/declarative_frontend/engine/js_types.h"
#include "bridge/declarative_frontend/jsview/js_form_menu_item.h"
#include "bridge/declarative_frontend/jsview/models/form_model_impl.h"
#include "bridge/declarative_frontend/jsview/models/menu_item_model_impl.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_abstract_model.h"
#include "core/components_ng/base/view_stack_model.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/form/form_model_ng.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_model.h"
#include "core/components_ng/pattern/menu/menu_item/menu_item_model_ng.h"
#include "frameworks/bridge/declarative_frontend/engine/functions/js_click_function.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_utils.h"
#include "frameworks/bridge/declarative_frontend/view_stack_processor.h"

namespace OHOS::Ace::Framework {
constexpr int NUM_WANT_1 = 0;
constexpr int NUM_ID_2 = 1;
constexpr int NUM_DATA_3 = 2;
constexpr int NUM_FUN_4 = 3;
constexpr int NUM_CALLBACKNUM = 2;

void JSFormMenuItem::JSBind(BindingTarget globalObj)
{
    JSClass<JSFormMenuItem>::Declare("FormMenuItem");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSFormMenuItem>::StaticMethod("create", &JSMenuItem::Create, opt);
    JSClass<JSFormMenuItem>::StaticMethod("onRegClick", &JSFormMenuItem::JsOnRegClick);
    JSClass<JSFormMenuItem>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSFormMenuItem>::InheritAndBind<JSViewAbstract>(globalObj);
}

void JSFormMenuItem::RequestPublishFormWithSnapshot(JSRef<JSVal> wantValue,
    const std::string& formBindingDataStr, RefPtr<JsFunction> jsCBFunc)
{
    RefPtr<WantWrap> wantWrap = CreateWantWrapFromNapiValue(wantValue);
    if (!wantWrap) {
        TAG_LOGE(AceLogTag::ACE_FORM, "onTap wantWrap is NULL.");
        return;
    }
    int64_t formId = 0;
    AAFwk::Want& want = const_cast<AAFwk::Want&>(wantWrap->GetWant());
    if (!want.HasParameter("ohos.extra.param.key.add_form_to_host_snapshot") ||
        !want.HasParameter("ohos.extra.param.key.add_form_to_host_width") ||
        !want.HasParameter("ohos.extra.param.key.add_form_to_host_height") ||
        !want.HasParameter("ohos.extra.param.key.add_form_to_host_screenx") ||
        !want.HasParameter("ohos.extra.param.key.add_form_to_host_screeny")) {
        TAG_LOGE(AceLogTag::ACE_FORM, "want has no component snapshot info");
        return;
    }

    FormModel::GetInstance()->RequestPublishFormWithSnapshot(want, formBindingDataStr, formId);
    if (!jsCBFunc) {
        TAG_LOGE(AceLogTag::ACE_FORM, "jsCBFunc is null");
        return;
    }

    JSRef<JSVal> params[NUM_CALLBACKNUM];
    params[0] = JSRef<JSVal>::Make(ToJSValue((formId > 0) ? 0 : -1));
    params[1] = JSRef<JSVal>::Make(ToJSValue(formId));
    jsCBFunc->ExecuteJS(NUM_CALLBACKNUM, params);
}

void JSFormMenuItem::JsOnRegClick(const JSCallbackInfo& info)
{
    bool retFlag;
    OnClickParameterCheck(info, retFlag);
    if (retFlag) {
        return;
    }

    std::string compId;
    JSViewAbstract::ParseJsString(info[NUM_ID_2], compId);
    if (compId.empty()) {
        TAG_LOGE(AceLogTag::ACE_FORM, "JsOnClick compId is empty.Input parameter componentId check failed.");
        return;
    }
    
    std::string formBindingDataStr;
    JSViewAbstract::ParseJsString(info[NUM_DATA_3], formBindingDataStr);
    if (formBindingDataStr.empty()) {
        TAG_LOGW(AceLogTag::ACE_FORM, "JsOnClick formBindingDataStr is empty");
    }

    RefPtr<JsFunction> jsCallBackFunc = nullptr;
    if (!info[NUM_FUN_4]->IsUndefined() && info[NUM_FUN_4]->IsFunction()) {
        jsCallBackFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[NUM_FUN_4]));
    }

    WeakPtr<NG::FrameNode> targetNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());

    auto onTap = [execCtx = info.GetExecutionContext(),  node = targetNode,
        jsCBFunc = std::move(jsCallBackFunc), want = info[NUM_WANT_1], formBindingDataStr] (GestureEvent& event) {
        JSRef<JSVal> wantValue = JSRef<JSVal>::Cast(want);
        if (wantValue->IsNull()) {
            TAG_LOGE(AceLogTag::ACE_FORM, "JsOnClick wantValue is null");
            return;
        }
        TAG_LOGI(AceLogTag::ACE_FORM, "JsOnClick ontap");
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onTap");
        RequestPublishFormWithSnapshot(wantValue, formBindingDataStr, jsCBFunc);
    };

    auto onClick = [execCtx = info.GetExecutionContext(), node = targetNode,
        jsCBFunc = std::move(jsCallBackFunc), want = info[NUM_WANT_1], formBindingDataStr]
        (const ClickInfo* event) {
        JSRef<JSVal> wantValue = JSRef<JSVal>::Cast(want);
        if (wantValue->IsNull()) {
            TAG_LOGE(AceLogTag::ACE_FORM, "JsOnClick wantValue is null");
            return;
        }
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onClick");
        PipelineContext::SetCallBackNode(node);
        RequestPublishFormWithSnapshot(wantValue, formBindingDataStr, jsCBFunc);
    };
    ViewAbstractModel::GetInstance()->SetOnClick(std::move(onTap), std::move(onClick));
}

void JSFormMenuItem::OnClickParameterCheck(const JSCallbackInfo& info, bool& retFlag)
{
    retFlag = true;

    if (info[NUM_WANT_1]->IsUndefined() || !info[NUM_WANT_1]->IsObject() || info[NUM_ID_2]->IsUndefined() ||
        !info[NUM_ID_2]->IsString()) {
        TAG_LOGE(AceLogTag::ACE_FORM, "OnClickParameterCheck bad parameter info[1] and info[2]");
        return;
    }
    retFlag = false;
}
} // namespace OHOS::Ace::Framework