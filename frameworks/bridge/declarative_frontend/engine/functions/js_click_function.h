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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_FUNCTION_JS_CLICK_FUNCTION_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_FUNCTION_JS_CLICK_FUNCTION_H

#include "core/event/mouse_event.h"
#include "core/gestures/click_recognizer.h"
#include "frameworks/bridge/declarative_frontend/engine/bindings.h"
#include "frameworks/bridge/declarative_frontend/engine/functions/js_function.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_abstract.h"

namespace OHOS::Ace::Framework {

#define HAND_NONE 0
#define HAND_LEFT 1
#define HAND_RIGHT 2
class JsClickFunction : public JsFunction {
    DECLARE_ACE_TYPE(JsClickFunction, JsFunction)

public:
    explicit JsClickFunction(const JSRef<JSFunc>& jsFunction) : JsFunction(JSRef<JSObject>(), jsFunction) {}

    ~JsClickFunction() override {};
    void Execute();
    void Execute(const ClickInfo& info);
    void Execute(GestureEvent& info);
    void Execute(MouseInfo& info);
};

class JsWeakClickFunction : public JsWeakFunction {
    DECLARE_ACE_TYPE(JsWeakClickFunction, JsWeakFunction)

public:
    explicit JsWeakClickFunction(const JSRef<JSFunc>& jsFunction) : JsWeakFunction(JSRef<JSObject>(), jsFunction) {}

    ~JsWeakClickFunction() override {};
    void Execute();
    void Execute(const ClickInfo& info);
    void Execute(GestureEvent& info);
    void Execute(MouseInfo& info);
};

} // namespace OHOS::Ace::Framework

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_FUNCTION_JS_CLICK_FUNCTION_H
