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

#include "frameworks/bridge/declarative_frontend/jsview/js_repeat_controller.h"

#include "base/log/log_wrapper.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::Framework {

void JSRepeatController::BindToNode(const RefPtr<NG::RepeatVirtualScroll2Node>& node)
{
    CHECK_NULL_VOID(node);
    // One-to-one binding, last-bind-wins: binding this controller to another
    // Repeat replaces the previous binding (spec R-8). A destroyed node expires
    // the weak reference automatically, no notify-back is needed.
    weakNode_ = node;
}

void JSRepeatController::JSBind(BindingTarget globalObj)
{
    JSClass<JSRepeatController>::Declare("RepeatController");
    JSClass<JSRepeatController>::CustomMethod("clearCache", &JSRepeatController::ClearCache);
    JSClass<JSRepeatController>::Bind(
        globalObj, JSRepeatController::Constructor, JSRepeatController::Destructor);
}

void JSRepeatController::Constructor(const JSCallbackInfo& args)
{
    auto controller = Referenced::MakeRefPtr<JSRepeatController>();
    controller->IncRefCount();
    args.SetReturnValue(Referenced::RawPtr(controller));
}

void JSRepeatController::Destructor(JSRepeatController* controller)
{
    if (controller != nullptr) {
        controller->DecRefCount();
    }
}

void JSRepeatController::ClearCache(const JSCallbackInfo& args)
{
    auto node = weakNode_.Upgrade();
    if (!node) {
        return; // no live binding (or the bound node was destroyed): safe no-op (spec R-6/AC-2.4)
    }
    TAG_LOGI(AceLogTag::ACE_REPEAT, "RepeatController.clearCache id[%{public}d]", node->GetId());
    // Directly trigger the existing L2-only progressive clean (ADR-3/ADR-4):
    // CleanCache(false) -> PurgeAll -> onPurgeAll (TS side purges spareRid_ items only,
    // L1 is preserved), released frame-by-frame through idle tasks.
    node->CleanCache(false);
}

} // namespace OHOS::Ace::Framework
