/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_SCROLLABLE_BASE_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_SCROLLABLE_BASE_H

#include "bridge/declarative_frontend/jsview/js_container_base.h"

namespace OHOS::Ace::Framework {
class JSScrollableBase : public JSContainerBase {
public:
    static void JSFlingSpeedLimit(const JSCallbackInfo& info);
    static void JsOnWillScroll(const JSCallbackInfo& args);
    static void JsOnDidScroll(const JSCallbackInfo& args);
    static void JSBind(BindingTarget globalObj);
    static void SetFadingEdge(const JSCallbackInfo& info);
    static void JSClipContent(const JSCallbackInfo& info);
    static void SetDigitalCrownSensitivity(const JSCallbackInfo& info);
    static void SetScrollBarMargin(const JSCallbackInfo& info);
    static void JSBackToTop(const JSCallbackInfo& info);
    static void JSOnWillStopDragging(const JSCallbackInfo& args);
};
} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_SCROLLABLE_BASE_H
