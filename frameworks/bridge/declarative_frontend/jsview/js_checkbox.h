/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_CHECKBOX_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_CHECKBOX_H

#include "core/components/checkable/checkable_component.h"
#include "bridge/declarative_frontend/jsview/js_view_abstract.h"

namespace OHOS::Ace::Framework {
class JSCheckbox : public JSViewAbstract {
public:
    static void JSBind(BindingTarget globalObj);
    static void Create(const JSCallbackInfo& info);
    static void SetSelect(const JSCallbackInfo& info);
    static void SetOnChange(const JSCallbackInfo& args);
    static void JsWidth(const JSCallbackInfo& info);
    static void JsHeight(const JSCallbackInfo& info);
    static void JsPadding(const JSCallbackInfo& info);
    static void JsMargin(const JSCallbackInfo& info);
    static void JsResponseRegion(const JSCallbackInfo& info);
    static void JsWidth(const JSRef<JSVal>& jsValue);
    static void JsHeight(const JSRef<JSVal>& jsValue);
    static void JsSize(const JSCallbackInfo& info);
    static void SelectedColor(const JSCallbackInfo& info);
    static void UnSelectedColor(const JSCallbackInfo& info);
    static void Mark(const JSCallbackInfo& info);
    static bool GetOldPadding(const JSCallbackInfo& info, NG::PaddingPropertyF& padding);
    static NG::PaddingProperty GetNewPadding(const JSCallbackInfo& info);
    static NG::PaddingProperty GetPadding(const std::optional<CalcDimension>& top,
        const std::optional<CalcDimension>& bottom, const std::optional<CalcDimension>& left,
        const std::optional<CalcDimension>& right);
    static void SetCheckboxStyle(int32_t checkBoxStyle);
};
} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_CHECKBOX_H
