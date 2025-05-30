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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_VIDEO_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_VIDEO_H

#include "frameworks/bridge/declarative_frontend/jsview/js_interactable_view.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_abstract.h"

namespace OHOS::Ace::Framework {

class JSVideo : public JSViewAbstract, public JSInteractableView {
public:
    using EventCallback = std::function<void(const std::string&)>;
    static void Create(const JSCallbackInfo& info);
    static bool ParseJsPosterOptions(const JSRef<JSVal>& jsValue, bool& result);
    static void JsMuted(const JSCallbackInfo& info);
    static void JsAutoPlay(const JSCallbackInfo& info);
    static void JsControls(const JSCallbackInfo& info);
    static void JsLoop(const JSCallbackInfo& info);
    static void JsObjectFit(const JSCallbackInfo& info);
    static void JsSurfaceBackgroundColor(const JSCallbackInfo& info);
    static void JsSetShortcutKeyEnabled(const JSCallbackInfo& info);

    static void JsOnStart(const JSCallbackInfo& info);
    static void JsOnPause(const JSCallbackInfo& info);
    static void JsOnFinish(const JSCallbackInfo& info);
    static void JsOnFullscreenChange(const JSCallbackInfo& info);
    static void JsOnPrepared(const JSCallbackInfo& info);
    static void JsOnSeeking(const JSCallbackInfo& info);
    static void JsOnSeeked(const JSCallbackInfo& info);
    static void JsOnUpdate(const JSCallbackInfo& info);
    static void JsOnError(const JSCallbackInfo& info);
    static void JsOnStop(const JSCallbackInfo& info);
    static void EnableAnalyzer(bool enable);
    static void AnalyzerConfig(const JSCallbackInfo& info);

    static void JSBind(BindingTarget globalObj);

private:
    static EventMarker GetEventMarker(const JSCallbackInfo& info, const std::vector<std::string>& keys);
    static void ParseImageAIOptions(const JSRef<JSVal>& jsValue);
};

} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_VIDEO_H
