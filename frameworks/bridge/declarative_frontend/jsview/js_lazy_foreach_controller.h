/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_LAZY_FOREACH_CONTROLLER_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_LAZY_FOREACH_CONTROLLER_H

#include "base/memory/referenced.h"
#include "bridge/declarative_frontend/engine/bindings.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"

namespace OHOS::Ace::Framework {

// JS global class "LazyForEachController": controller for actively releasing the preloaded
// cache of the bound LazyForEach component (LazyForEachOptions.controller API, @since 26.1.0).
// Binding happens when the LazyForEach is created and is immutable on the component side,
// while the controller side is strictly one-to-one: binding the controller to another
// LazyForEach replaces the previous binding (last-bind-wins, the previously bound
// component is no longer controlled). The controller only holds a weak ref to the node,
// so a node destroyed in the meantime results in a safe no-op clearCache() (spec R-6).
class JSLazyForEachController : public Referenced {
public:
    JSLazyForEachController() = default;
    ~JSLazyForEachController() override = default;

    // Records the bound node weakly, replacing any previous binding (one controller, one
    // LazyForEach component; the last binding wins). A destroyed node expires the weak
    // reference automatically, no notify-back is needed.
    void BindToNode(const RefPtr<NG::LazyForEachNode>& node);

    static void JSBind(BindingTarget globalObj);
    static void Constructor(const JSCallbackInfo& args);
    static void Destructor(JSLazyForEachController* controller);
    void ClearCache(const JSCallbackInfo& args);

private:
    WeakPtr<NG::LazyForEachNode> weakNode_;

    ACE_DISALLOW_COPY_AND_MOVE(JSLazyForEachController);
};

} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_LAZY_FOREACH_CONTROLLER_H
