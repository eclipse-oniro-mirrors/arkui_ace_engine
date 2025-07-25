/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_TOGGLE_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_TOGGLE_H

#include "frameworks/bridge/declarative_frontend/jsview/js_container_base.h"

namespace OHOS::Ace::Framework {

enum class ToggleType {
    CHECKBOX = 0,
    SWITCH,
    BUTTON,
};

class JSToggle : public JSViewAbstract, public JSInteractableView {
public:
    static void JSBind(BindingTarget globalObj);
    static void Create(const JSCallbackInfo& info);
    static void JsWidth(const JSCallbackInfo& info);
    static void JsHeight(const JSCallbackInfo& info);
    static void JsPadding(const JSCallbackInfo& info);
    static void JsMargin(const JSCallbackInfo& info);
    static void JsResponseRegion(const JSCallbackInfo& info);
    static void JsWidth(const JSRef<JSVal>& jsValue);
    static void JsHeight(const JSRef<JSVal>& jsValue);
    static void JsSize(const JSCallbackInfo& info);
    static void OnChange(const JSCallbackInfo& info);
    static void SelectedColor(const JSCallbackInfo& info);
    static void SwitchPointColor(const JSCallbackInfo& info);
    static void SetBackgroundColor(const JSCallbackInfo& info);
    static void JsHoverEffect(const JSCallbackInfo& info);
    static void Pop();
    static void SwitchStyle(const JSCallbackInfo& info);
    static NG::PaddingPropertyF GetOldPadding(const JSCallbackInfo& info);
    static NG::PaddingProperty GetNewPadding(const JSCallbackInfo& info);
    static NG::PaddingProperty GetPadding(const std::optional<CalcDimension>& top,
        const std::optional<CalcDimension>& bottom, const std::optional<CalcDimension>& left,
        const std::optional<CalcDimension>& right);
    static void JsRadius(const JSCallbackInfo& info);
    static void JsBorder(const JSCallbackInfo& info);

private:
    static int32_t toggleType_;
    static void CreateWithDimensionResourceObj(RefPtr<ResourceObject>& resObj, const int32_t resType);
    static void CreateWithColorResourceObj(RefPtr<ResourceObject>& resObj, const int32_t resType);
    static void SetPointRadius(const JSRef<JSObject>& jsObj);
    static void SetUnselectedColor(const JSRef<JSObject>& jsObj);
    static void SetPointColor(const JSRef<JSObject>& jsObj);
    static void SetTrackBorderRadius(const JSRef<JSObject>& jsObj);
};

} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_TOGGLE_H
