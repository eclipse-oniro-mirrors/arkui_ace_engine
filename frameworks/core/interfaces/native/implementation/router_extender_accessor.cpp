/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "arkoala_api_generated.h"
#include "base/memory/ace_type.h"
#include "core/common/container.h"
#include "core/common/frontend.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/stage/page_node.h"
#include "core/interfaces/native/utility/callback_helper.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/promise_helper.h"
#include "core/interfaces/native/utility/reverse_converter.h"

#include <cstring>

namespace OHOS::Ace::NG::GeneratedModifier {
namespace RouterExtenderAccessor {
void Push0Impl(Ark_VMContext vmContext,
               Ark_AsyncWorkerPtr asyncWorker,
               Ark_NativePointer jsView,
               const Ark_PageRouterOptions* options,
               const Opt_RouterFinishCallback* finishCallback,
               const Callback_Opt_Array_String_Void* outputArgumentForReturningPromise)
{
    CHECK_NULL_VOID(vmContext);
    CHECK_NULL_VOID(asyncWorker);
    CHECK_NULL_VOID(jsView);
    CHECK_NULL_VOID(options);
    CHECK_NULL_VOID(finishCallback);
    CHECK_NULL_VOID(outputArgumentForReturningPromise);
    auto promise = std::make_shared<PromiseHelper<Callback_Opt_Array_String_Void>>(outputArgumentForReturningPromise);
    auto finishFunc = [promise](const std::string& errStr, int32_t errCode) {
        if (errCode == ERROR_CODE_NO_ERROR) {
            promise->Resolve();
        } else {
            promise->Reject({std::to_string(errCode), errStr});
        }
    };
    PageRouterOptions routerOptions;
    routerOptions.errorCallback = finishFunc;
    routerOptions.url = Converter::Convert<std::string>(options->options.url);
    if (options->options.recoverable.tag != InteropTag::INTEROP_TAG_UNDEFINED) {
        routerOptions.recoverable = Converter::Convert<bool>(options->options.recoverable.value);
    }
    if (options->mode.tag != InteropTag::INTEROP_TAG_UNDEFINED) {
        routerOptions.routerMode = static_cast<uint32_t>(options->mode.value);
    }
    auto execFunc = [routerOptions, finishCallback, jsView]() {
        std::function<void()> callback;
        if (finishCallback->tag != InteropTag::INTEROP_TAG_UNDEFINED) {
            callback = [finish = CallbackHelper(finishCallback->value), jsNode = jsView]() {
                finish.InvokeSync(jsNode);
            };
        }
        auto container = Container::Current();
        CHECK_NULL_VOID(container);
        auto delegate = container->GetFrontend();
        CHECK_NULL_VOID(delegate);
        delegate->PushExtender(routerOptions, std::move(callback), jsView);
    };
    promise->StartAsync(vmContext, *asyncWorker, execFunc);
}
void Push1Impl(Ark_NativePointer jsView,
               const Ark_PageRouterOptions* options,
               const Opt_RouterFinishCallback* finishCallback,
               const Opt_AsyncCallback_Void* callback_)
{
    CHECK_NULL_VOID(jsView);
    CHECK_NULL_VOID(options);
    CHECK_NULL_VOID(finishCallback);
    CHECK_NULL_VOID(callback_);
}
void Replace0Impl(Ark_VMContext vmContext,
                  Ark_AsyncWorkerPtr asyncWorker,
                  Ark_NativePointer jsView,
                  const Ark_PageRouterOptions* options,
                  const Opt_RouterFinishCallback* enterFinishCallback,
                  const Callback_Opt_Array_String_Void* outputArgumentForReturningPromise)
{
    CHECK_NULL_VOID(vmContext);
    CHECK_NULL_VOID(asyncWorker);
    CHECK_NULL_VOID(jsView);
    CHECK_NULL_VOID(options);
    CHECK_NULL_VOID(enterFinishCallback);
    CHECK_NULL_VOID(outputArgumentForReturningPromise);
    auto promise = std::make_shared<PromiseHelper<Callback_Opt_Array_String_Void>>(outputArgumentForReturningPromise);
    auto finishFunc = [promise](const std::string& errStr, int32_t errCode) {
        if (errCode == ERROR_CODE_NO_ERROR) {
            promise->Resolve();
        } else {
            promise->Reject({std::to_string(errCode), errStr});
        }
    };
    PageRouterOptions routerOptions;
    routerOptions.errorCallback = finishFunc;
    routerOptions.url = Converter::Convert<std::string>(options->options.url);
    if (options->options.recoverable.tag != InteropTag::INTEROP_TAG_UNDEFINED) {
        routerOptions.recoverable = Converter::Convert<bool>(options->options.recoverable.value);
    }
    if (options->mode.tag != InteropTag::INTEROP_TAG_UNDEFINED) {
        routerOptions.routerMode = static_cast<uint32_t>(options->mode.value);
    }
    auto execFunc = [routerOptions, enterFinishCallback, jsView]() {
        std::function<void()> callback;
        if (enterFinishCallback->tag != InteropTag::INTEROP_TAG_UNDEFINED) {
            callback = [finish = CallbackHelper(enterFinishCallback->value), jsNode = jsView]() {
                finish.InvokeSync(jsNode);
            };
        }
        auto container = Container::Current();
        CHECK_NULL_VOID(container);
        auto delegate = container->GetFrontend();
        CHECK_NULL_VOID(delegate);
        delegate->ReplaceExtender(routerOptions, std::move(callback), jsView);
    };
    promise->StartAsync(vmContext, *asyncWorker, execFunc);
}
void Replace1Impl(Ark_NativePointer jsView,
                  const Ark_PageRouterOptions* options,
                  const Opt_RouterFinishCallback* finishCallback,
                  const Opt_AsyncCallback_Void* callback_)
{
    CHECK_NULL_VOID(jsView);
    CHECK_NULL_VOID(options);
    CHECK_NULL_VOID(finishCallback);
    CHECK_NULL_VOID(callback_);
}
void Back0Impl(const Opt_router_RouterOptions* options)
{
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto delegate = container->GetFrontend();
    CHECK_NULL_VOID(delegate);
    delegate->BackExtender("", "");
}
void Back1Impl(Ark_Int32 index,
               const Opt_Object* params)
{
}
void RunPageImpl(Ark_NativePointer jsView,
                              const Ark_PageRouterOptions* options,
                              const Opt_RouterFinishCallback* finishCallback)
{
    CHECK_NULL_VOID(jsView);
    CHECK_NULL_VOID(options);
    CHECK_NULL_VOID(finishCallback);
    PageRouterOptions routerOptions;
    routerOptions.url = Converter::Convert<std::string>(options->options.url);
    if (options->options.recoverable.tag != InteropTag::INTEROP_TAG_UNDEFINED) {
        routerOptions.recoverable = Converter::Convert<bool>(options->options.recoverable.value);
    }
    if (options->mode.tag != InteropTag::INTEROP_TAG_UNDEFINED) {
        routerOptions.routerMode = static_cast<uint32_t>(options->mode.value);
    }
    std::function<void()> callback;
    if (finishCallback->tag != InteropTag::INTEROP_TAG_UNDEFINED) {
        callback = [finish = CallbackHelper(finishCallback->value), jsNode = jsView]() {
            finish.InvokeSync(jsNode);
        };
    }
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto delegate = container->GetFrontend();
    CHECK_NULL_VOID(delegate);
    delegate->RunPageExtender(routerOptions, std::move(callback), jsView);
}
void ClearImpl()
{
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto delegate = container->GetFrontend();
    CHECK_NULL_VOID(delegate);
    delegate->ClearExtender();
}
void ShowAlertBeforeBackPageImpl(const Ark_String* message)
{
    CHECK_NULL_VOID(message);
    std::string info = Converter::Convert<std::string>(*message);
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto delegate = container->GetFrontend();
    CHECK_NULL_VOID(delegate);
    delegate->ShowAlertBeforeBackPageExtender(info);
}
void HideAlertBeforeBackPageImpl()
{
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto delegate = container->GetFrontend();
    CHECK_NULL_VOID(delegate);
    delegate->HideAlertBeforeBackPageExtender();
}
Ark_String GetLengthImpl()
{
    return {};
}
Ark_router_RouterState GetStateImpl()
{
    return {};
}
Opt_router_RouterState GetStateByIndexImpl(Ark_Int32 index)
{
    return {};
}
Array_router_RouterState GetStateByUrlImpl(const Ark_String* url)
{
    return {};
}
Ark_Object GetParamsImpl()
{
    return {};
}
void PushNamedRoute0Impl(Ark_VMContext vmContext,
                         Ark_AsyncWorkerPtr asyncWorker,
                         Ark_NativePointer jsView,
                         const Ark_PageRouterOptions* options,
                         const Opt_RouterFinishCallback* finishCallback,
                         const Callback_Opt_Array_String_Void* outputArgumentForReturningPromise)
{
    CHECK_NULL_VOID(vmContext);
    CHECK_NULL_VOID(asyncWorker);
    CHECK_NULL_VOID(jsView);
    CHECK_NULL_VOID(options);
    CHECK_NULL_VOID(finishCallback);
    CHECK_NULL_VOID(outputArgumentForReturningPromise);
    auto promise = std::make_shared<PromiseHelper<Callback_Opt_Array_String_Void>>(outputArgumentForReturningPromise);
    auto finishFunc = [promise](const std::string& errStr, int32_t errCode) {
        if (errCode == ERROR_CODE_NO_ERROR) {
            promise->Resolve();
        } else {
            promise->Reject({std::to_string(errCode), errStr});
        }
    };
    PageRouterOptions routerOptions;
    routerOptions.errorCallback = finishFunc;
    routerOptions.url = Converter::Convert<std::string>(options->options.url);
    routerOptions.isNamedRouterMode = true;
    if (options->options.recoverable.tag != InteropTag::INTEROP_TAG_UNDEFINED) {
        routerOptions.recoverable = Converter::Convert<bool>(options->options.recoverable.value);
    }
    if (options->mode.tag != InteropTag::INTEROP_TAG_UNDEFINED) {
        routerOptions.routerMode = static_cast<uint32_t>(options->mode.value);
    }
    auto execFunc = [routerOptions, finishCallback, jsView]() {
        std::function<void()> callback;
        if (finishCallback->tag != InteropTag::INTEROP_TAG_UNDEFINED) {
            callback = [finish = CallbackHelper(finishCallback->value), jsNode = jsView]() {
                finish.InvokeSync(jsNode);
            };
        }
        auto container = Container::Current();
        CHECK_NULL_VOID(container);
        auto delegate = container->GetFrontend();
        CHECK_NULL_VOID(delegate);
        delegate->PushNamedRouteExtender(routerOptions, std::move(callback), jsView);
    };
    promise->StartAsync(vmContext, *asyncWorker, execFunc);
}
void PushNamedRoute1Impl(Ark_NativePointer jsView,
                         const Ark_PageRouterOptions* options,
                         const Opt_RouterFinishCallback* finishCallback,
                         const Opt_AsyncCallback_Void* callback_)
{
}
void ReplaceNamedRoute0Impl(Ark_VMContext vmContext,
                            Ark_AsyncWorkerPtr asyncWorker,
                            Ark_NativePointer jsView,
                            const Ark_PageRouterOptions* options,
                            const Opt_RouterFinishCallback* finishCallback,
                            const Callback_Opt_Array_String_Void* outputArgumentForReturningPromise)
{
    CHECK_NULL_VOID(vmContext);
    CHECK_NULL_VOID(asyncWorker);
    CHECK_NULL_VOID(jsView);
    CHECK_NULL_VOID(options);
    CHECK_NULL_VOID(finishCallback);
    CHECK_NULL_VOID(outputArgumentForReturningPromise);
    auto promise = std::make_shared<PromiseHelper<Callback_Opt_Array_String_Void>>(outputArgumentForReturningPromise);
    auto finishFunc = [promise](const std::string& errStr, int32_t errCode) {
        if (errCode == ERROR_CODE_NO_ERROR) {
            promise->Resolve();
        } else {
            promise->Reject({std::to_string(errCode), errStr});
        }
    };
    PageRouterOptions routerOptions;
    routerOptions.errorCallback = finishFunc;
    routerOptions.url = Converter::Convert<std::string>(options->options.url);
    routerOptions.isNamedRouterMode = true;
    if (options->options.recoverable.tag != InteropTag::INTEROP_TAG_UNDEFINED) {
        routerOptions.recoverable = Converter::Convert<bool>(options->options.recoverable.value);
    }
    if (options->mode.tag != InteropTag::INTEROP_TAG_UNDEFINED) {
        routerOptions.routerMode = static_cast<uint32_t>(options->mode.value);
    }
    auto execFunc = [routerOptions, finishCallback, jsView]() {
        std::function<void()> callback;
        if (finishCallback->tag != InteropTag::INTEROP_TAG_UNDEFINED) {
            callback = [finish = CallbackHelper(finishCallback->value), jsNode = jsView]() {
                finish.InvokeSync(jsNode);
            };
        }
        auto container = Container::Current();
        CHECK_NULL_VOID(container);
        auto delegate = container->GetFrontend();
        CHECK_NULL_VOID(delegate);
        delegate->ReplaceNamedRouteExtender(routerOptions, std::move(callback), jsView);
    };
    promise->StartAsync(vmContext, *asyncWorker, execFunc);
}
void ReplaceNamedRoute1Impl(Ark_NativePointer jsView,
                            const Ark_PageRouterOptions* options,
                            const Opt_RouterFinishCallback* finishCallback,
                            const Opt_AsyncCallback_Void* callback_)
{
}
} // RouterExtenderAccessor
const GENERATED_ArkUIRouterExtenderAccessor* GetRouterExtenderAccessor()
{
    static const GENERATED_ArkUIRouterExtenderAccessor RouterExtenderAccessorImpl {
        RouterExtenderAccessor::Push0Impl,
        RouterExtenderAccessor::Push1Impl,
        RouterExtenderAccessor::Replace0Impl,
        RouterExtenderAccessor::Replace1Impl,
        RouterExtenderAccessor::Back0Impl,
        RouterExtenderAccessor::Back1Impl,
        RouterExtenderAccessor::RunPageImpl,
        RouterExtenderAccessor::ClearImpl,
        RouterExtenderAccessor::ShowAlertBeforeBackPageImpl,
        RouterExtenderAccessor::HideAlertBeforeBackPageImpl,
        RouterExtenderAccessor::GetLengthImpl,
        RouterExtenderAccessor::GetStateImpl,
        RouterExtenderAccessor::GetStateByIndexImpl,
        RouterExtenderAccessor::GetStateByUrlImpl,
        RouterExtenderAccessor::GetParamsImpl,
        RouterExtenderAccessor::PushNamedRoute0Impl,
        RouterExtenderAccessor::PushNamedRoute1Impl,
        RouterExtenderAccessor::ReplaceNamedRoute0Impl,
        RouterExtenderAccessor::ReplaceNamedRoute1Impl,
    };
    return &RouterExtenderAccessorImpl;
}

}