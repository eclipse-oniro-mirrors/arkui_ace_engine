/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORK_JAVASCRIPT_BRIDGE_JS_VIEW_JS_CANVAS_H
#define FOUNDATION_ACE_FRAMEWORK_JAVASCRIPT_BRIDGE_JS_VIEW_JS_CANVAS_H

#include "bridge/declarative_frontend/jsview/canvas/js_canvas_renderer.h"
#include "bridge/declarative_frontend/jsview/canvas/js_drawing_rendering_context.h"
#include "bridge/declarative_frontend/jsview/js_container_base.h"

namespace OHOS::Ace::Framework {

class JSCanvas : public JSViewAbstract, public JSInteractableView {
public:
    static void JSBind(BindingTarget globalObj);
    static void Create(const JSCallbackInfo& info);
    static void OnReady(const JSCallbackInfo& info);
    static void EnableAnalyzer(bool enable);
};

} // namespace OHOS::Ace::Framework

#endif // FOUNDATION_ACE_FRAMEWORK_JAVASCRIPT_BRIDGE_JS_VIEW_JS_CANVAS_H
