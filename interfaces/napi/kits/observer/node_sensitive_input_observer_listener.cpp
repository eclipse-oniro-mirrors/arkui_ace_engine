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

#include "ui_observer_listener.h"

#include "base/utils/napi_scope_raii.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_raw_input_event_wrapper.h"
#include "core/common/container.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/node_sensitive_input.h"
#include "napi/native_common.h"
#include "native_engine/impl/ark/ark_native_engine.h"

namespace OHOS::Ace::Napi {
namespace {
constexpr int32_t PARAM_SIZE_ONE = 1;
constexpr int32_t PARAM_SIZE_TWO = 2;
} // namespace

void UIObserverListener::OnNodeSensitiveInput(NG::FrameNode* frameNode, const NG::NodeSensitiveInputInfo& info)
{
    if (!env_ || !callback_.IsValid()) {
        return;
    }
    ScopeRAII scope(env_);
    if (!scope) {
        return;
    }
    napi_value result = nullptr;
    napi_create_object(env_, &result);
    napi_value isAgent = nullptr;
    napi_get_boolean(env_, info.isAgent, &isAgent);
    napi_set_named_property(env_, result, "isAgent", isAgent);
    napi_value type = nullptr;
    napi_create_int32(env_, static_cast<int32_t>(info.type), &type);
    napi_set_named_property(env_, result, "type", type);

    auto* engine = reinterpret_cast<ArkNativeEngine*>(env_);
    auto* vm = const_cast<panda::EcmaVM*>(engine->GetEcmaVm());
    CHECK_NULL_VOID(vm);
    panda::LocalScope eventScope(vm);
    auto node = frameNode ? AceType::Claim(frameNode) : RefPtr<NG::FrameNode>();
    auto setEvent = [this, vm, node, result](const char* name, const RawInputEventWrapper& wrapper) {
        auto event = Framework::ArkTSNativeRawInputEventWrapper::CreateSensitiveInputEvent(wrapper, vm, node);
        napi_set_named_property(env_, result, name, LocalValueToLocalNapiValue(event));
    };
    if (info.type == NG::NodeSensitiveInputEventType::TOUCH && info.touchEvent) {
        setEvent("touchEvent", RawInputEventWrapper(*info.touchEvent));
    } else if (info.type == NG::NodeSensitiveInputEventType::MOUSE && info.mouseEvent) {
        setEvent("mouseEvent", RawInputEventWrapper(*info.mouseEvent));
    } else if (info.type == NG::NodeSensitiveInputEventType::KEY && info.keyEvent) {
        setEvent("keyEvent", RawInputEventWrapper(*info.keyEvent));
    } else {
        return;
    }

    napi_value nodeValue = nullptr;
    auto container = Container::Current();
    auto frontEnd = container ? container->GetFrontend() : nullptr;
    if (frontEnd && frameNode) {
        nodeValue = frontEnd->GetFrameNodeValueByNodeId(frameNode->GetId());
    }
    napi_value callback = callback_.GetValue();
    if (nodeValue) {
        napi_value argv[] = { result, nodeValue };
        napi_call_function(env_, nullptr, callback, PARAM_SIZE_TWO, argv, nullptr);
    } else {
        napi_value argv[] = { result };
        napi_call_function(env_, nullptr, callback, PARAM_SIZE_ONE, argv, nullptr);
    }
}
} // namespace OHOS::Ace::Napi
