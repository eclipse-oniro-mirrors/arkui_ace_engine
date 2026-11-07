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

#include "frameworks/bridge/declarative_frontend/jsview/js_lazy_foreach_controller.h"

#include "base/log/log_wrapper.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::Framework {

void JSLazyForEachController::BindToNode(const RefPtr<NG::LazyForEachNode>& node)
{
    CHECK_NULL_VOID(node);
    // One-to-one binding, last-bind-wins: binding this controller to another
    // LazyForEach replaces the previous binding (spec R-8). A destroyed node expires
    // the weak reference automatically, no notify-back is needed.
    weakNode_ = node;
}

void JSLazyForEachController::JSBind(BindingTarget globalObj)
{
    JSClass<JSLazyForEachController>::Declare("LazyForEachController");
    JSClass<JSLazyForEachController>::CustomMethod("clearCache", &JSLazyForEachController::ClearCache);
    JSClass<JSLazyForEachController>::Bind(
        globalObj, JSLazyForEachController::Constructor, JSLazyForEachController::Destructor);
}

void JSLazyForEachController::Constructor(const JSCallbackInfo& args)
{
    auto controller = Referenced::MakeRefPtr<JSLazyForEachController>();
    controller->IncRefCount();
    args.SetReturnValue(Referenced::RawPtr(controller));
}

void JSLazyForEachController::Destructor(JSLazyForEachController* controller)
{
    if (controller != nullptr) {
        controller->DecRefCount();
    }
}

void JSLazyForEachController::ClearCache(const JSCallbackInfo& args)
{
    auto node = weakNode_.Upgrade();
    if (!node) {
        return; // no live binding (or the bound node was destroyed): safe no-op (spec R-6/AC-1.4)
    }
    TAG_LOGI(AceLogTag::ACE_LAZY_FOREACH,
        "LazyForEachController.clearCache id[%{public}d]", node->GetId());
    // Directly trigger the existing progressive cache clean (ADR-3): CleanCache(false, true)
    // releases the prebuilt cache frame-by-frame through idle/predict tasks without keeping
    // any idle cache count (ReduceCacheCount returns 0 in this round), and a restore task
    // still cancels the remaining release via the existing mechanism.
    node->CleanCache(false, true);
}

} // namespace OHOS::Ace::Framework
