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

#include "bridge/declarative_frontend/jsview/models/view_abstract_model_impl.h"
#include "bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "core/components_ng/base/view_abstract_model_ng.h"
#include "frameworks/base/log/ace_scoring_log.h"
#include "bridge/declarative_frontend/jsview/js_accessibility.h"

namespace OHOS::Ace::Framework {
namespace {
}
void JSViewAbstract::JsAccessibilityGroup(const JSCallbackInfo& info)
{
    bool isGroup = false;
    if (info[0]->IsBoolean()) {
        isGroup = info[0]->ToBoolean();
    }
    ViewAbstractModel::GetInstance()->SetAccessibilityGroup(isGroup);

    if (info.Length() > 1 && info[1]->IsObject()) {
        auto obj = JSRef<JSObject>::Cast(info[1]);

        auto preferAccessibilityTextObj = obj->GetProperty("accessibilityPreferred");
        auto preferAccessibilityText =
            preferAccessibilityTextObj->IsBoolean() ? preferAccessibilityTextObj->ToBoolean() : false;
        ViewAbstractModel::GetInstance()->SetAccessibilityTextPreferred(preferAccessibilityText);
    }
}

void JSViewAbstract::JsAccessibilityText(const JSCallbackInfo& info)
{
    const JSRef<JSVal>& jsValue = info[0];
    std::string text;
    if (!ParseJsString(jsValue, text)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetAccessibilityText(text);
}

void JSViewAbstract::JsAccessibilityTextHint(const std::string& text)
{
    ViewAbstractModel::GetInstance()->SetAccessibilityTextHint(text);
}

void JSViewAbstract::JsAccessibilityNextFocusId(const JSCallbackInfo& info)
{
    const JSRef<JSVal>& jsValue = info[0];
    std::string nextFocusId;
    if (!ParseJsString(jsValue, nextFocusId)) {
        return;
    }
    ViewAbstractModel::GetInstance()->SetAccessibilityNextFocusId(nextFocusId);
}

void JSViewAbstract::JsAccessibilityDescription(const JSCallbackInfo& info)
{
    const JSRef<JSVal>& jsValue = info[0];
    std::string description;
    if (!ParseJsString(jsValue, description)) {
        return;
    }
    std::pair<bool, std::string> autoEventPair(false, "");
    std::pair<bool, std::string> descriptionPair(false, "");
    ParseAccessibilityDescriptionJson(description, autoEventPair, descriptionPair);
    if (descriptionPair.first) {
        ViewAbstractModel::GetInstance()->SetAccessibilityDescription(descriptionPair.second);
    } else {
        ViewAbstractModel::GetInstance()->SetAccessibilityDescription(description);
    }
    if (autoEventPair.first) {
        ViewAbstractModel::GetInstance()->SetAutoEventParam(autoEventPair.second);
    }
}

void JSViewAbstract::ParseAccessibilityDescriptionJson(const std::string& description,
    std::pair<bool, std::string>& autoEventPair, std::pair<bool, std::string>& descriptionPair)
{
    if (description.empty()) {
        return;
    }
    if (!StartWith(description, "{") || !EndWith(description, "}")) {
        return;
    }
    auto jsonObj = JsonUtil::ParseJsonString(description);
    if (!jsonObj || !jsonObj->IsValid() || !jsonObj->IsObject()) {
        return;
    }
    if (jsonObj->Contains("$autoEventParam")) {
        auto param = jsonObj->GetValue("$autoEventParam");
        if (param) {
            autoEventPair = std::make_pair(true, param->ToString());
        }
    }
    if (jsonObj->Contains("$accessibilityDescription")) {
        descriptionPair = std::make_pair(true, jsonObj->GetString("$accessibilityDescription"));
    } else if (jsonObj->Contains("$autoEventParam")) {
        descriptionPair = std::make_pair(true, "");
    }
}

void JSViewAbstract::JsAccessibilityImportance(const std::string& importance)
{
    ViewAbstractModel::GetInstance()->SetAccessibilityImportance(importance);
}

void JSViewAbstract::JsAccessibilityLevel(const std::string& level)
{
    ViewAbstractModel::GetInstance()->SetAccessibilityImportance(level);
}

void JSViewAbstract::JsAccessibilityVirtualNode(const JSCallbackInfo& info)
{
    // parse builder
    if (!info[0]->IsObject()) {
        return;
    }
    JSRef<JSObject> obj = JSRef<JSObject>::Cast(info[0]);
    auto builder = obj->GetProperty("builder");
    if (builder->IsFunction()) {
        auto builderFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSFunc>::Cast(builder));
        CHECK_NULL_VOID(builderFunc);
        auto buildFunc = [execCtx = info.GetExecutionContext(), func = std::move(builderFunc)]() {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("AccessibilityVirtualNode");
            func->Execute();
        };
        NG::ViewAbstractModelNG::GetInstance()->SetAccessibilityVirtualNode(std::move(buildFunc));
    }
}

void JSViewAbstract::JsAccessibilitySelected(const JSCallbackInfo& info)
{
    bool selected = false;
    bool resetValue = false;
    JSRef<JSVal> arg = info[0];
    if (arg->IsUndefined()) {
        resetValue = true;
    } else if (arg->IsBoolean()) {
        selected = arg->ToBoolean();
    } else {
        return;
    }

    ViewAbstractModel::GetInstance()->SetAccessibilitySelected(selected, resetValue);
}

void JSViewAbstract::JsAccessibilityChecked(const JSCallbackInfo& info)
{
    bool checked = false;
    bool resetValue = false;
    JSRef<JSVal> arg = info[0];
    if (arg->IsUndefined()) {
        resetValue = true;
    } else if (arg->IsBoolean()) {
        checked = arg->ToBoolean();
    } else {
        return;
    }

    ViewAbstractModel::GetInstance()->SetAccessibilityChecked(checked, resetValue);
}

void JSViewAbstract::JsAccessibilityScrollTriggerable(const JSCallbackInfo& info)
{
    bool scrollTriggerable = false;
    bool resetValue = false;
    JSRef<JSVal> arg = info[0];
    if (arg->IsUndefined()) {
        resetValue = true;
    } else if (arg->IsBoolean()) {
        scrollTriggerable = arg->ToBoolean();
    } else {
        return;
    }

    ViewAbstractModel::GetInstance()->SetAccessibilityScrollTriggerable(scrollTriggerable, resetValue);
}

void JSViewAbstract::JsAccessibilityRole(const JSCallbackInfo& info)
{
    bool resetValue = false;
    std::string role;
    if (info.Length() < 1 || !info[0]->IsNumber()) {
        ViewAbstractModel::GetInstance()->SetAccessibilityRole(role, true);
        return;
    }
    auto index = info[0]->ToNumber<int32_t>();
    AccessibilityRoleType text = static_cast<AccessibilityRoleType>(index);
    role = AccessibilityUtils::GetRoleByType(text);
    if (role.empty()) {
        resetValue = true;
    }
    ViewAbstractModel::GetInstance()->SetAccessibilityRole(role, resetValue);
}

void JSViewAbstract::JsOnAccessibilityFocus(const JSCallbackInfo& info)
{
    if (info[0]->IsUndefined() || !info[0]->IsFunction()) {
        ViewAbstractModel::GetInstance()->ResetOnAccessibilityFocus();
        return;
    }
    RefPtr<JsFunction> jsFoucusFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSFunc>::Cast(info[0]));
    WeakPtr<NG::FrameNode> frameNode = AceType::WeakClaim(NG::ViewStackProcessor::GetInstance()->GetMainFrameNode());
    auto onAccessibilityFoucus = [execCtx = info.GetExecutionContext(), func = std::move(jsFoucusFunc),
                                     node = frameNode](bool isFocus) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("onAccessibilityFoucus");
        PipelineContext::SetCallBackNode(node);
        JSRef<JSVal> newJSVal = JSRef<JSVal>::Make(ToJSValue(isFocus));
        func->ExecuteJS(1, &newJSVal);
    };
    ViewAbstractModel::GetInstance()->SetOnAccessibilityFocus(std::move(onAccessibilityFoucus));
}

void JSViewAbstract::JsAccessibilityDefaultFocus(const JSCallbackInfo& info)
{
    JSRef<JSVal> arg = info[0];
    if (arg->IsBoolean()) {
        auto isFocus = arg->ToBoolean();
        ViewAbstractModel::GetInstance()->SetAccessibilityDefaultFocus(isFocus);
        return;
    }
    ViewAbstractModel::GetInstance()->SetAccessibilityDefaultFocus(false);
}

void JSViewAbstract::JsAccessibilityUseSamePage(const JSCallbackInfo& info)
{
    JSRef<JSVal> arg = info[0];
    if (arg->IsNumber()) {
        auto pageModeType = arg->ToNumber<int32_t>();
        std::string pageMode = AccessibilityUtils::GetPageModeType(pageModeType);
        ViewAbstractModel::GetInstance()->SetAccessibilityUseSamePage(pageMode);
        return;
    }
    ViewAbstractModel::GetInstance()->SetAccessibilityUseSamePage("");
}

void JSViewAbstract::JsAccessibilityFocusDrawLevel(const JSCallbackInfo& info)
{
    int32_t drawLevel = 0;
    JSRef<JSVal> arg = info[0];
    do {
        if (!arg->IsNumber()) {
            break;
        }
        if (arg->ToNumber<int32_t>() > 1) {
            break;
        }
        drawLevel = arg->ToNumber<int32_t>();
    } while (false);
    ViewAbstractModel::GetInstance()->SetAccessibilityFocusDrawLevel(drawLevel);
}
}