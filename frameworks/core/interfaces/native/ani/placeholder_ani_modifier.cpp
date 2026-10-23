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

#include <cinttypes>
#include <memory>
#include <string>

#include "ani.h"

#include "placeholder_ani_modifier.h"

#include "base/log/ace_trace.h"
#include "base/utils/utils.h"
#include "base/log/log_wrapper.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_manager.h"
#include "core/components_ng/manager/scroll_placeholder/scroll_placeholder_restricted_scope.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {

// Northbound result codes of register: 0 admitted, negative values carry the
// rejection cause so the module side can raise the documented BusinessError.
constexpr int32_t REGISTER_OK = 0;
constexpr int32_t REGISTER_FAILED = -1;
constexpr int32_t REGISTER_PARAM_INVALID = -2;
constexpr int32_t REGISTER_NOT_INSTRUMENTED = -3;
constexpr int32_t REGISTER_UNSUPPORTED_COMPONENT = -4;

bool IsFunction(ani_env* env, ani_object obj)
{
    CHECK_NULL_RETURN(env, false);
    ani_class funcClass = nullptr;
    if (env->FindClass("std.core.Function0", &funcClass) != ANI_OK) {
        env->ResetError();
        return false;
    }
    ani_boolean result = ANI_FALSE;
    if (env->Object_InstanceOf(obj, funcClass, &result) != ANI_OK) {
        return false;
    }
    return static_cast<bool>(result);
}

// Product hand-off slot of one isolated execution: the ArkTS execution closure
// swaps its own InteropViewStackProcessor in, runs the compiled builder in the
// canonical static creation context, pops the built subtree root, and publishes
// the pointer through _PlaceholderRegisterPublishProduct. UI-thread only.
intptr_t g_pendingProduct = 0;

// Persistent handle of the ArkTS execution closure: the manager may execute it
// at any later frame (source rebuilds, clone probes), so the function reference
// must outlive the register call. Released only when the whole module goes
// away; unregistering keeps the handle alive harmlessly.
class AniTemplateBuilderHandle {
public:
    AniTemplateBuilderHandle(ani_vm* vm, ani_ref globalRef) : vm_(vm), globalRef_(globalRef) {}

    ani_fn_object Fn() const
    {
        return reinterpret_cast<ani_fn_object>(globalRef_);
    }

    ani_env* Env() const
    {
        ani_env* env = nullptr;
        if (vm_ == nullptr || vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
            return nullptr;
        }
        return env;
    }

private:
    ani_vm* vm_ = nullptr;
    ani_ref globalRef_ = nullptr;
};

using AniTemplateBuilderHandlePtr = std::shared_ptr<AniTemplateBuilderHandle>;

// Same admission protocol as the declarative registration bridge, but the
// isolated creation context is owned by the ArkTS execution closure: it swaps
// an InteropViewStackProcessor in, runs the compiled builder in the canonical
// static creation context (instance scoping, local state manager, V1 rendering
// mode, need-create), and publishes the popped product root through
// _PlaceholderRegisterPublishProduct. Calling the compiled builder bare from
// here corrupts the live rendering state (white screen), which is exactly why
// the declarative bridge also receives an execution closure instead of the raw
// builder. The probe still observes the __aceBegin/EndScrollPlaceholder__
// instrumentation of that execution natively.
ScrollPlaceholderBuilder MakeTemplateBuilder(const AniTemplateBuilderHandlePtr& handle,
    const std::shared_ptr<ScrollPlaceholderRestrictedScope::Probe>& admission)
{
    return [handle, admission]() -> RefPtr<UINode> {
        ACE_SCOPED_TRACE("ScrollPlaceholder.IsolatedExecution");
        TAG_LOGI(AceLogTag::ACE_SCROLL, "ScrollPlaceholder isolated execution: begin");
        ani_env* env = handle->Env();
        if (env == nullptr) {
            TAG_LOGW(AceLogTag::ACE_SCROLL, "ScrollPlaceholder builder lost the ani env");
            ScrollPlaceholderRestrictedScope::NotifyViolation();
            return nullptr;
        }
        ScrollPlaceholderRestrictedScope::Probe probe;
        g_pendingProduct = 0;
        {
            ani_ref result = nullptr;
            ani_status status = env->FunctionalObject_Call(handle->Fn(), 0, nullptr, &result);
            if (status != ANI_OK) {
                // A throwing builder is a half-failed execution: record it as a
                // violation so the admission probe rejects the template instead
                // of misreading a fired begin as admission success.
                TAG_LOGW(AceLogTag::ACE_SCROLL,
                    "ScrollPlaceholder execution closure threw (ani status=%{public}d), "
                    "see the module-side 'execution: builder THREW' line for the reason", status);
                ScrollPlaceholderRestrictedScope::NotifyViolation();
                // The rejection is reported through the register result code.
                // Leaving the exception pending would propagate it through
                // Register() into the caller's build pass and abort the whole
                // page, turning a placeholder failure into a white screen.
                env->ResetError();
            } else {
                TAG_LOGI(AceLogTag::ACE_SCROLL, "ScrollPlaceholder execution closure returned normally");
            }
        }
        probe.Close();
        *admission = probe;
        TAG_LOGI(AceLogTag::ACE_SCROLL,
            "ScrollPlaceholder probe closed: admitted=%{public}d beginFired=%{public}d "
            "endFired=%{public}d violation=%{public}d",
            static_cast<int32_t>(probe.Admitted()),
            static_cast<int32_t>(probe.RestrictedScopeEntered()),
            static_cast<int32_t>(probe.ScopeClosed()),
            static_cast<int32_t>(probe.OutOfScopeTriggered()));
        // _PopViewStackProcessor hands the root over with an acquired reference;
        // Claim adopts that reference into the registry's product.
        RefPtr<UINode> product;
        if (g_pendingProduct != 0) {
            product = AceType::Claim(reinterpret_cast<UINode*>(g_pendingProduct));
        }
        // createUiDetachedRoot builds the template under an engine root wrapper
        // (tag __Common__) that has no CloneForPlaceholder support. The wrapper
        // is not part of the template: publish the single builder root beneath
        // it so the registry probes and clones the actual content.
        while (product != nullptr && product->GetTag() == "__Common__") {
            const auto& children = product->GetChildren();
            if (children.size() != 1) {
                break;
            }
            auto inner = children.front();
            TAG_LOGI(AceLogTag::ACE_SCROLL,
                "ScrollPlaceholder product unwrapped: wrapper=__Common__, root=%{public}s, id=%{public}d",
                inner->GetTag().c_str(), inner->GetId());
            product = inner;
        }
        if (!product) {
            TAG_LOGW(AceLogTag::ACE_SCROLL,
                "ScrollPlaceholder isolated execution produced no product "
                "(pending product ptr is 0 or claim failed)");
        } else {
            TAG_LOGI(AceLogTag::ACE_SCROLL,
                "ScrollPlaceholder product claimed: ptr=%{public}p, tag=%{public}s, id=%{public}d",
                AceType::RawPtr(product), product->GetTag().c_str(), product->GetId());
        }
        g_pendingProduct = 0;
        if (probe.Admitted() && product) {
            ScrollPlaceholderRestrictedScope::MarkProduct(product);
            TAG_LOGI(AceLogTag::ACE_SCROLL, "ScrollPlaceholder product marked as verified");
        } else {
            TAG_LOGW(AceLogTag::ACE_SCROLL,
                "ScrollPlaceholder product NOT marked: admitted=%{public}d hasProduct=%{public}d "
                "(unmarked products are never mounted)",
                static_cast<int32_t>(probe.Admitted()), product != nullptr);
        }
        TAG_LOGI(AceLogTag::ACE_SCROLL, "ScrollPlaceholder isolated execution: end");
        return product;
    };
}

ani_int RegisterTemplate(ani_env* env, const std::string& templateId, ani_object builderArg, ani_object executionArg)
{
    TAG_LOGI(AceLogTag::ACE_SCROLL,
        "PlaceholderRegister.register: enter, id=%{public}s, builderObj=%{public}p, executionObj=%{public}p",
        templateId.c_str(), builderArg, executionArg);
    ani_ref builderProp = nullptr;
    ani_object builderObj = static_cast<ani_object>(builderArg);
    if (templateId.empty()) {
        TAG_LOGW(AceLogTag::ACE_SCROLL, "PlaceholderRegister.register: rejected, id is empty");
        return REGISTER_PARAM_INVALID;
    }
    if (builderObj == nullptr) {
        TAG_LOGW(AceLogTag::ACE_SCROLL, "PlaceholderRegister.register: rejected, builder object is null");
        return REGISTER_PARAM_INVALID;
    }
    // Static-track WrappedBuilder stores the builder in a generic field and the
    // @Builder function does not implement std.core.Function0 (the builder
    // transform adds the parent/context parameter), so the old WrappedBuilder
    // shape check rejected valid static builders. The argument is never invoked
    // natively - every execution goes through executionArg - so the shape is
    // recorded for diagnostics only.
    ani_status builderPropStatus = env->Object_GetPropertyByName_Ref(builderObj, "builder", &builderProp);
    if (builderPropStatus != ANI_OK || builderProp == nullptr) {
        TAG_LOGW(AceLogTag::ACE_SCROLL,
            "PlaceholderRegister.register: builder has no readable '.builder' (status=%{public}d); "
            "continuing with the execution closure", static_cast<int32_t>(builderPropStatus));
    }
    if (!IsFunction(env, static_cast<ani_object>(executionArg))) {
        TAG_LOGW(AceLogTag::ACE_SCROLL,
            "PlaceholderRegister.register: execution closure is not std.core.Function0; "
            "continuing and letting FunctionalObject_Call validate it");
    }
    auto context = PipelineContext::GetCurrentContext();
    if (!context) {
        TAG_LOGW(AceLogTag::ACE_SCROLL,
            "PlaceholderRegister.register: no pipeline context, id=%{public}s", templateId.c_str());
        return REGISTER_FAILED;
    }
    TAG_LOGI(AceLogTag::ACE_SCROLL, "PlaceholderRegister.register: pipeline context instanceId=%{public}d",
        context->GetInstanceId());
    const auto& manager = context->GetOrCreateScrollPlaceholderManager();
    if (!manager) {
        TAG_LOGW(AceLogTag::ACE_SCROLL,
            "PlaceholderRegister.register: scroll placeholder manager unavailable, id=%{public}s",
            templateId.c_str());
        return REGISTER_FAILED;
    }

    ani_ref globalRef = nullptr;
    if (env->GlobalReference_Create(executionArg, &globalRef) != ANI_OK || globalRef == nullptr) {
        TAG_LOGW(AceLogTag::ACE_SCROLL,
            "PlaceholderRegister.register: global reference creation failed, id=%{public}s", templateId.c_str());
        return REGISTER_FAILED;
    }
    ani_vm* vm = nullptr;
    env->GetVM(&vm);
    auto handle = std::make_shared<AniTemplateBuilderHandle>(vm, globalRef);
    auto admission = std::make_shared<ScrollPlaceholderRestrictedScope::Probe>();
    auto builderClosure = MakeTemplateBuilder(handle, admission);
    TAG_LOGI(AceLogTag::ACE_SCROLL,
        "PlaceholderRegister.register: registering template, id=%{public}s "
        "(runs the synchronous admission execution)", templateId.c_str());
    uint64_t generation = manager->RegisterTemplate(templateId, std::move(builderClosure));
    if (generation == 0) {
        TAG_LOGW(AceLogTag::ACE_SCROLL,
            "PlaceholderRegister.register: template registration failed, id=%{public}s "
            "(registry rejected: empty id or destroyed)", templateId.c_str());
        return REGISTER_FAILED;
    }
    TAG_LOGI(AceLogTag::ACE_SCROLL,
        "PlaceholderRegister.register: template registered, id=%{public}s, generation=%{public}" PRIu64,
        templateId.c_str(), generation);
    if (!admission->Admitted()) {
        manager->UnregisterTemplate(templateId);
        TAG_LOGW(AceLogTag::ACE_SCROLL,
            "PlaceholderRegister.register: builder without isPlaceholder instrumentation, "
            "id=%{public}s entered=%{public}d closed=%{public}d violation=%{public}d",
            templateId.c_str(), static_cast<int32_t>(admission->RestrictedScopeEntered()),
            static_cast<int32_t>(admission->ScopeClosed()),
            static_cast<int32_t>(admission->OutOfScopeTriggered()));
        return REGISTER_NOT_INSTRUMENTED;
    }
    std::string subtreePath;
    if (!manager->ProbeTemplateClone(templateId, subtreePath)) {
        manager->UnregisterTemplate(templateId);
        TAG_LOGW(AceLogTag::ACE_SCROLL,
            "PlaceholderRegister.register: unsupported component at %{public}s, id=%{public}s",
            subtreePath.c_str(), templateId.c_str());
        return REGISTER_UNSUPPORTED_COMPONENT;
    }
    TAG_LOGI(AceLogTag::ACE_SCROLL,
        "PlaceholderRegister.register: clone probe passed (source subtree copyable), "
        "id=%{public}s, subtree=%{public}s, generation=%{public}" PRIu64,
        templateId.c_str(), subtreePath.c_str(), generation);
    return REGISTER_OK;
}

void UnregisterTemplate(const std::string& templateId)
{
    auto context = PipelineContext::GetCurrentContext();
    if (!context) {
        TAG_LOGW(AceLogTag::ACE_SCROLL,
            "PlaceholderRegister.unregister: no pipeline context (treated as no-op), id=%{public}s",
            templateId.c_str());
        return;
    }
    const auto& manager = context->GetOrCreateScrollPlaceholderManager();
    if (!manager) {
        TAG_LOGW(AceLogTag::ACE_SCROLL,
            "PlaceholderRegister.unregister: manager unavailable (treated as no-op), id=%{public}s",
            templateId.c_str());
        return;
    }
    // Unregistering an unknown id stays an idempotent silent no-op, matching
    // the declarative bridge.
    bool removed = manager->UnregisterTemplate(templateId);
    TAG_LOGI(AceLogTag::ACE_SCROLL,
        "PlaceholderRegister.unregister: id=%{public}s, removed=%{public}d "
        "(false = was not registered)", templateId.c_str(), static_cast<int32_t>(removed));
}

ani_boolean IsTemplateRegistered(const std::string& templateId)
{
    auto context = PipelineContext::GetCurrentContext();
    if (!context) {
        TAG_LOGW(AceLogTag::ACE_SCROLL, "PlaceholderRegister.isRegistered: no pipeline context, id=%{public}s",
            templateId.c_str());
        return ANI_FALSE;
    }
    const auto& manager = context->GetOrCreateScrollPlaceholderManager();
    // Read-only query: no manager simply means "not registered".
    ani_boolean registered = ANI_FALSE;
    if (manager && manager->IsTemplateRegistered(templateId)) {
        registered = ANI_TRUE;
    }
    TAG_LOGI(AceLogTag::ACE_SCROLL, "PlaceholderRegister.isRegistered: id=%{public}s, result=%{public}d",
        templateId.c_str(), static_cast<int32_t>(registered));
    return registered;
}

// Product hand-off of one isolated execution; see g_pendingProduct.
void PublishProduct(ani_long productPtr)
{
    TAG_LOGI(AceLogTag::ACE_SCROLL, "PlaceholderRegister publish product: ptr=%{public}p",
        reinterpret_cast<void*>(static_cast<intptr_t>(productPtr)));
    g_pendingProduct = static_cast<intptr_t>(productPtr);
}

void BeginScrollPlaceholder()
{
    ScrollPlaceholderRestrictedScope::Begin();
}

void EndScrollPlaceholder()
{
    ScrollPlaceholderRestrictedScope::End();
}

} // namespace

const ArkUIAniPlaceholderRegisterModifier* GetPlaceholderRegisterAniModifier()
{
    static const ArkUIAniPlaceholderRegisterModifier impl = {
        .registerTemplate = RegisterTemplate,
        .unregisterTemplate = UnregisterTemplate,
        .isTemplateRegistered = IsTemplateRegistered,
        .publishProduct = PublishProduct,
        .beginScrollPlaceholder = BeginScrollPlaceholder,
        .endScrollPlaceholder = EndScrollPlaceholder,
    };
    return &impl;
}

} // namespace OHOS::Ace::NG
