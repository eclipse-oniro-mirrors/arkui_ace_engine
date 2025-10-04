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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_MAGNIFIER_CONTROLLER_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_MAGNIFIER_CONTROLLER_H

#include "base/memory/referenced.h"
#include "bridge/declarative_frontend/engine/bindings.h"

namespace OHOS::Ace::Framework {

class JSMagnifierController : public Referenced {
public:
    JSMagnifierController() = default;
    ~JSMagnifierController() override = default;

    static void JSBind(BindingTarget globalObj);
    static void Constructor(const JSCallbackInfo& args);
    static void Destructor(JSMagnifierController* magnifierController);
    static void Bind(const JSCallbackInfo& args);
    static void Show(const JSCallbackInfo& args);
    static void Unbind();
};

} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_MAGNIFIER_CONTROLLER_H
