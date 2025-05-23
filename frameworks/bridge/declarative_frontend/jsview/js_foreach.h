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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_FOREACH_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_FOREACH_H

#include "bridge/declarative_frontend/engine/bindings.h"
#include "bridge/declarative_frontend/engine/js_ref_ptr.h"

namespace OHOS::Ace::Framework {

class JSForEach {
public:
    static void JSBind(BindingTarget globalObj);
    static void Create(const JSCallbackInfo& info);
    static void CreateForPartialUpdate();
    static void CreateForPartialUpdateNG();
    static void Pop();
    static void GetIdArray(const JSCallbackInfo& info);
    static void SetIdArray(const JSCallbackInfo& info);

    // function to call before exec item generator function
    // signature is
    // id: string | number
    // parentView : JSView
    static void CreateNewChildStart(const JSCallbackInfo& info);

    // function to call after exec item generator function
    // signature is
    // id: string | number
    // parentView : JSView
    static void CreateNewChildFinish(const JSCallbackInfo& info);

    static void OnMove(const JSCallbackInfo& info);
    static void JsParseItemDragEventHandler(const JsiExecutionContext& context, const JSRef<JSObject>& itemDragEventObj);
};

} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_FOREACH_H
