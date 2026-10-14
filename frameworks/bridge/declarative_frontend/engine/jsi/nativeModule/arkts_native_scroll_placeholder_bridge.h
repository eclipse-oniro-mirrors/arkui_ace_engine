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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_JSI_NATIVEMODULE_ARKTS_NATIVE_SCROLL_PLACEHOLDER_BRIDGE_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_JSI_NATIVEMODULE_ARKTS_NATIVE_SCROLL_PLACEHOLDER_BRIDGE_H

#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_api_bridge.h"

namespace OHOS::Ace::NG {

// Dynamic ArkTS registration bridge for the scroll placeholder template registry
// (js_scroll_placeholder_register). Exposed on the internal ArkUI native module
// surface as `scrollPlaceholderRegister`; the northbound handle is the
// PlaceholderRegister instance returned by UIContext.getPlaceholderRegister(),
// whose methods call into this surface inside the UI instance scope.
//
// Register() performs the single-layer-instrumentation admission (ADR-11): the
// template builder closure runs inside an isolated creation context (independent
// view stack, product never mounted), and whether the compiler-injected
// begin/end instrumentation fired during that execution is the sole legality
// signal — a plain uninstrumented builder is rejected with BusinessError 100031
// and never reaches the registry. The ADR-14 registration probe (restricted
// copy-protocol validation of the template subtree) shares the same execution's
// source instance; an uncopyable component rejects the whole template.
class ScrollPlaceholderRegisterBridge {
public:
    static ArkUINativeModuleValue Register(ArkUIRuntimeCallInfo* runtimeCallInfo);
    static ArkUINativeModuleValue Unregister(ArkUIRuntimeCallInfo* runtimeCallInfo);
    static ArkUINativeModuleValue IsRegistered(ArkUIRuntimeCallInfo* runtimeCallInfo);

    // Runtime entries of the restricted-builder instrumentation contract: the
    // compiler emits paired calls at the first/last line of every
    // @Builder({ isPlaceholder: true }) conversion product, injected as
    // module-level bindings (never globalThis properties, so dynamic JS cannot
    // reference or forge them). They drive the restricted-scope state machine.
    static ArkUINativeModuleValue BeginPlaceholderScope(ArkUIRuntimeCallInfo* runtimeCallInfo);
    static ArkUINativeModuleValue EndPlaceholderScope(ArkUIRuntimeCallInfo* runtimeCallInfo);
};

} // namespace OHOS::Ace::NG

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_JSI_NATIVEMODULE_ARKTS_NATIVE_SCROLL_PLACEHOLDER_BRIDGE_H
