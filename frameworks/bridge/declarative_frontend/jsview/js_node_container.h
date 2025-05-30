/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_NODE_CONTAINER_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_NODE_CONTAINER_H

#include "core/components_ng/pattern/node_container/node_container_pattern.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_interactable_view.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_abstract.h"

namespace OHOS::Ace::Framework {
class JSNodeContainer : public JSViewAbstract, public JSInteractableView {
public:
    static void Create(const JSCallbackInfo& info);
    static void SetNodeController(const JSRef<JSObject>& object, JsiExecutionContext execCtx);
    static void ResetNodeController();
    static JSRef<JSVal> GetCurrentContext();
    static RefPtr<NG::NodeContainerPattern> GetNodeContainerPattern(int32_t containerId);
    static void SetOnAppearFunc(const JSRef<JSObject>& object, JsiExecutionContext execCtx);
    static void SetOnDisappearFunc(const JSRef<JSObject>& object, JsiExecutionContext execCtx);
    static void SetOnResizeFunc(const JSRef<JSObject>& object, JsiExecutionContext execCtx);
    static void SetOnTouchEventFunc(const JSRef<JSObject>& object, JsiExecutionContext execCtx);
    static void ResetNodeContainerId(const JSWeak<JSObject>& firstArg, int32_t nodeContainerId, EcmaVM* vm);
    static void SetOnAttachFunc(const JSRef<JSObject>& object, JsiExecutionContext execCtx);
    static void SetOnDetachFunc(const JSRef<JSObject>& object, JsiExecutionContext execCtx);
    static void SetOnWillBindFunc(const JSRef<JSObject>& object, JsiExecutionContext execCtx);
    static void SetOnWillUnbindFunc(const JSRef<JSObject>& object, JsiExecutionContext execCtx);
    static void SetOnBindFunc(const JSRef<JSObject>& object, JsiExecutionContext execCtx);
    static void SetOnUnbindFunc(const JSRef<JSObject>& object, JsiExecutionContext execCtx);
    static void FireOnWillBind(int32_t containerId);
    static void FireOnWillUnbind(int32_t containerId);
    static void FireOnBind(int32_t containerId);
    static void FireOnUnbind(int32_t containerId);
    static void JSBind(BindingTarget globalObj);
};

} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_NODE_CONTAINER_H
