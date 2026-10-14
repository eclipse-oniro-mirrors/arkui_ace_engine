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

#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_scroll_placeholder_bridge.h"

#include <cinttypes>
#include <memory>
#include <string>

#include "base/error/error_code.h"
#include "base/log/log.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_manager.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_restricted_scope.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {

// Northbound-facing message text of the registration rejection (SDK documents
// error 100031 for exactly this condition).
constexpr char REGISTER_REJECTED_MESSAGE[] =
    "Registration rejected: the builder is not marked with @Builder({ isPlaceholder: true }).";
constexpr char UNSUPPORTED_COMPONENT_MESSAGE_PREFIX[] =
    "Registration rejected: the placeholder template contains an unsupported component";

void ThrowRejected(EcmaVM* vm)
{
    ArkTSUtils::ThrowBusinessError(vm, REGISTER_REJECTED_MESSAGE,
        ERROR_CODE_SCROLL_PLACEHOLDER_REGISTER_REJECTED);
}

// Validates the (id, WrappedBuilder) argument pair and unwraps the inner builder
// function. Throws the parameter error (401) itself and returns false on any
// invalid form; only the wrapBuilder-wrapped form is accepted - a bare builder
// function or an arbitrary object is a parameter error.
bool ParseRegisterArguments(EcmaVM* vm, ArkUIRuntimeCallInfo* runtimeCallInfo,
    std::string& templateId, panda::Local<panda::FunctionRef>& builderFunc)
{
    if (runtimeCallInfo->GetArgsNumber() < 2) { // 2: id + builder
        ArkTSUtils::ThrowError(vm, "Parameter error.", ERROR_CODE_PARAM_INVALID);
        return false;
    }
    Local<JSValueRef> idArg = runtimeCallInfo->GetCallArgRef(0);
    Local<JSValueRef> builderArg = runtimeCallInfo->GetCallArgRef(1);
    if (!idArg->IsString(vm)) {
        ArkTSUtils::ThrowError(vm, "Parameter error.", ERROR_CODE_PARAM_INVALID);
        return false;
    }
    templateId = idArg->ToString(vm)->ToString(vm);
    if (templateId.empty() || !builderArg->IsObject(vm)) {
        ArkTSUtils::ThrowError(vm, "Parameter error.", ERROR_CODE_PARAM_INVALID);
        return false;
    }
    auto wrappedBuilder = builderArg->ToObject(vm);
    Local<JSValueRef> builderProp = wrappedBuilder->Get(vm, panda::StringRef::NewFromUtf8(vm, "builder"));
    if (!builderProp->IsFunction(vm)) {
        ArkTSUtils::ThrowError(vm, "Parameter error.", ERROR_CODE_PARAM_INVALID);
        return false;
    }
    builderFunc = builderProp;
    return true;
}

// Wraps the unwrapped builder into the template builder closure: every execution
// runs inside an isolated creation context (independent view stack, product never
// mounted, no LRU) and records its instrumentation-signal snapshot for admission.
ScrollPlaceholderBuilder MakeIsolatedTemplateBuilder(EcmaVM* vm,
    const panda::Local<panda::FunctionRef>& builderFunc,
    const std::shared_ptr<ScrollPlaceholderRestrictedScope::Probe>& admission)
{
    auto globalFunc = panda::CopyableGlobal<panda::FunctionRef>(vm, builderFunc);
    return ScrollPlaceholderBuilder(
        [vm, builderFunc = std::move(globalFunc), admission]() -> RefPtr<UINode> {
            ScopedViewStackProcessor isolatedStack;
            ScrollPlaceholderRestrictedScope::Probe probe;
            {
                panda::LocalScope localScope(vm);
                panda::TryCatch tryCatch(vm);
                builderFunc->Call(vm, builderFunc.ToLocal(), nullptr, 0);
                if (tryCatch.HasCaught()) {
                    // A throwing builder is a half-failed execution: record it as
                    // a violation so the admission probe rejects the template
                    // instead of misreading a fired begin as admission success.
                    ScrollPlaceholderRestrictedScope::NotifyViolation();
                }
            }
            probe.Close();
            *admission = probe;
            auto product = ViewStackProcessor::GetInstance()->Finish();
            if (probe.Admitted() && product) {
                // Runtime defense: products created under a properly closed
                // restricted scope carry the mark bit validated before mounting.
                ScrollPlaceholderRestrictedScope::MarkProduct(product);
            }
            return product;
        });
}

// Outcome of the post-registration admission probe: the template registry entry
// is rolled back on either rejection before the mapped BusinessError is thrown.
enum class RegisterProbeStatus : int32_t {
    ADMITTED,
    NOT_INSTRUMENTED,
    UNSUPPORTED_COMPONENT,
};

// Post-registration admission checks: an uninstrumented (or mis-paired) builder
// (ADR-11) and an uncopyable template subtree (ADR-14 probe clone) both roll the
// registration back so the registry stays clean; diagnostics distinguish the two.
RegisterProbeStatus RunRegistrationAdmissionProbe(const RefPtr<ScrollPlaceholderManager>& manager,
    const std::string& templateId, const ScrollPlaceholderRestrictedScope::Probe& admission,
    std::string& subtreePath)
{
    if (!admission.Admitted()) {
        manager->UnregisterTemplate(templateId);
        TAG_LOGW(AceLogTag::ACE_SCROLL,
            "PlaceholderRegister.register: builder without isPlaceholder instrumentation, "
            "id=%{public}s entered=%{public}d closed=%{public}d violation=%{public}d",
            templateId.c_str(), static_cast<int32_t>(admission.RestrictedScopeEntered()),
            static_cast<int32_t>(admission.ScopeClosed()),
            static_cast<int32_t>(admission.OutOfScopeTriggered()));
        return RegisterProbeStatus::NOT_INSTRUMENTED;
    }
    if (!manager->ProbeTemplateClone(templateId, subtreePath)) {
        manager->UnregisterTemplate(templateId);
        TAG_LOGW(AceLogTag::ACE_SCROLL,
            "PlaceholderRegister.register: unsupported component at %{public}s, id=%{public}s",
            subtreePath.c_str(), templateId.c_str());
        return RegisterProbeStatus::UNSUPPORTED_COMPONENT;
    }
    return RegisterProbeStatus::ADMITTED;
}

} // namespace

// Registration flow (single-layer instrumentation admission, ADR-11 + ADR-14):
//  1. parameter validation — id must be a non-empty string and the builder a
//     WrappedBuilder wrapping a function; anything else is a parameter error (401);
//  2. the builder closure runs every execution inside an isolated creation context
//     (independent view stack, product never mounted, no LRU) and observes the
//     instrumentation signals of that execution; RegisterTemplate's synchronous
//     source build IS the admission try-execute — one execution, no extra runs;
//  3. begin never fired → the builder was not compiled under
//     @Builder({ isPlaceholder: true }) → rollback + BusinessError 100031;
//  4. admission passed → the ADR-14 probe clone validates the source subtree
//     against the restricted copy protocol on the same execution's product;
//     an uncopyable node rejects the whole template (rollback + 100031).
ArkUINativeModuleValue ScrollPlaceholderRegisterBridge::Register(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    std::string templateId;
    panda::Local<panda::FunctionRef> builderFunc;
    if (!ParseRegisterArguments(vm, runtimeCallInfo, templateId, builderFunc)) {
        return panda::JSValueRef::Undefined(vm);
    }
    auto context = PipelineContext::GetCurrentContext();
    if (!context) {
        TAG_LOGW(AceLogTag::ACE_SCROLL,
            "PlaceholderRegister.register: no pipeline context, id=%{public}s", templateId.c_str());
        ThrowRejected(vm);
        return panda::JSValueRef::Undefined(vm);
    }
    const auto& manager = context->GetOrCreateScrollPlaceholderManager();
    CHECK_NULL_RETURN(manager, panda::JSValueRef::Undefined(vm));

    // Latest-execution admission result, shared between the closure (which runs
    // inside RegisterTemplate's synchronous source build) and this call site.
    auto admission = std::make_shared<ScrollPlaceholderRestrictedScope::Probe>();
    auto builderClosure = MakeIsolatedTemplateBuilder(vm, builderFunc, admission);
    uint64_t generation = manager->RegisterTemplate(templateId, std::move(builderClosure));
    if (generation == 0) {
        TAG_LOGW(AceLogTag::ACE_SCROLL,
            "PlaceholderRegister.register: template registration failed, id=%{public}s", templateId.c_str());
        ThrowRejected(vm);
        return panda::JSValueRef::Undefined(vm);
    }
    std::string subtreePath;
    RegisterProbeStatus probeStatus = RunRegistrationAdmissionProbe(manager, templateId, *admission, subtreePath);
    if (probeStatus == RegisterProbeStatus::NOT_INSTRUMENTED) {
        ThrowRejected(vm);
        return panda::JSValueRef::Undefined(vm);
    }
    if (probeStatus == RegisterProbeStatus::UNSUPPORTED_COMPONENT) {
        ArkTSUtils::ThrowBusinessError(vm,
            std::string(UNSUPPORTED_COMPONENT_MESSAGE_PREFIX).append(" (").append(subtreePath).append(")."),
            ERROR_CODE_SCROLL_PLACEHOLDER_REGISTER_REJECTED);
        return panda::JSValueRef::Undefined(vm);
    }
    TAG_LOGI(AceLogTag::ACE_SCROLL,
        "PlaceholderRegister.register: id=%{public}s generation=%{public}" PRIu64,
        templateId.c_str(), generation);
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollPlaceholderRegisterBridge::Unregister(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    if (runtimeCallInfo->GetArgsNumber() < 1) {
        ArkTSUtils::ThrowError(vm, "Parameter error.", ERROR_CODE_PARAM_INVALID);
        return panda::JSValueRef::Undefined(vm);
    }
    Local<JSValueRef> idArg = runtimeCallInfo->GetCallArgRef(0);
    if (!idArg->IsString(vm)) {
        ArkTSUtils::ThrowError(vm, "Parameter error.", ERROR_CODE_PARAM_INVALID);
        return panda::JSValueRef::Undefined(vm);
    }
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, panda::JSValueRef::Undefined(vm));
    const auto& manager = context->GetOrCreateScrollPlaceholderManager();
    CHECK_NULL_RETURN(manager, panda::JSValueRef::Undefined(vm));
    // Unregistering an unknown id stays an idempotent silent no-op (not a failure);
    // known ids leave the matchable set and their in-flight tasks turn stale.
    manager->UnregisterTemplate(idArg->ToString(vm)->ToString(vm));
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollPlaceholderRegisterBridge::IsRegistered(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    if (runtimeCallInfo->GetArgsNumber() < 1) {
        ArkTSUtils::ThrowError(vm, "Parameter error.", ERROR_CODE_PARAM_INVALID);
        return panda::BooleanRef::New(vm, false);
    }
    Local<JSValueRef> idArg = runtimeCallInfo->GetCallArgRef(0);
    if (!idArg->IsString(vm)) {
        ArkTSUtils::ThrowError(vm, "Parameter error.", ERROR_CODE_PARAM_INVALID);
        return panda::BooleanRef::New(vm, false);
    }
    // Read-only query: a template stays registered regardless of whether prebuilt
    // instances are currently retained; no manager simply means "not registered".
    auto context = PipelineContext::GetCurrentContext();
    if (!context) {
        return panda::BooleanRef::New(vm, false);
    }
    const auto& manager = context->GetOrCreateScrollPlaceholderManager();
    bool registered = manager && manager->IsTemplateRegistered(idArg->ToString(vm)->ToString(vm));
    return panda::BooleanRef::New(vm, registered);
}

ArkUINativeModuleValue ScrollPlaceholderRegisterBridge::BeginPlaceholderScope(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    ScrollPlaceholderRestrictedScope::Begin();
    return panda::JSValueRef::Undefined(vm);
}

ArkUINativeModuleValue ScrollPlaceholderRegisterBridge::EndPlaceholderScope(ArkUIRuntimeCallInfo* runtimeCallInfo)
{
    EcmaVM* vm = runtimeCallInfo->GetVM();
    CHECK_NULL_RETURN(vm, panda::JSValueRef::Undefined(vm));
    ScrollPlaceholderRestrictedScope::End();
    return panda::JSValueRef::Undefined(vm);
}

} // namespace OHOS::Ace::NG
