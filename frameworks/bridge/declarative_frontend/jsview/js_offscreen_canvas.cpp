/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "bridge/declarative_frontend/jsview/js_offscreen_canvas.h"

#include "base/utils/utils.h"
#include "bridge/declarative_frontend/engine/bindings.h"
#include "bridge/declarative_frontend/jsview/js_canvas_gradient.h"
#include "bridge/declarative_frontend/jsview/js_canvas_pattern.h"
#include "bridge/declarative_frontend/jsview/js_matrix2d.h"
#include "bridge/declarative_frontend/jsview/js_render_image.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS::Ace::Framework {
constexpr int32_t ARGS_COUNT_ONE = 1;
constexpr int32_t ARGS_COUNT_TWO = 2;

void* DetachOffscreenCanvas(napi_env env, void* value, void* hint)
{
    if (value == nullptr) {
        LOGW("Invalid parameter.");
        return nullptr;
    }
    JSOffscreenCanvas* workCanvas = (JSOffscreenCanvas*)value;
    if (workCanvas->IsGetContext()) {
        JSException::Throw(ERROR_CODE_PARAM_INVALID, "%s",
            "An OffscreenCanvas could not be transferred because it had a rendering context.");
        return nullptr;
    }
    if (workCanvas->IsDetached()) {
        JSException::Throw(ERROR_CODE_PARAM_INVALID, "%s",
            "An OffscreenCanvas could not be transferred because the object is detached.");
        return nullptr;
    }
    workCanvas->SetDetachStatus(true);

    auto result = new (std::nothrow) JSOffscreenCanvas();
    result->SetWidth(workCanvas->GetWidth());
    result->SetHeight(workCanvas->GetHeight());
    return result;
}

napi_value AttachOffscreenCanvas(napi_env env, void* value, void*)
{
    if (value == nullptr) {
        LOGW("Invalid parameter.");
        return nullptr;
    }
    JSOffscreenCanvas* workCanvas = (JSOffscreenCanvas*)value;
    if (workCanvas == nullptr) {
        LOGW("Invalid context.");
        return nullptr;
    }

    napi_value offscreenCanvas = nullptr;
    napi_create_object(env, &offscreenCanvas);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_GETTER_SETTER("width", JSOffscreenCanvas::JsGetWidth, JSOffscreenCanvas::JsSetWidth),
        DECLARE_NAPI_GETTER_SETTER("height", JSOffscreenCanvas::JsGetHeight, JSOffscreenCanvas::JsSetHeight),
        DECLARE_NAPI_FUNCTION("transferToImageBitmap", JSOffscreenCanvas::JsTransferToImageBitmap),
        DECLARE_NAPI_FUNCTION("getContext", JSOffscreenCanvas::JsGetContext),
    };
    napi_define_properties(env, offscreenCanvas, sizeof(desc) / sizeof(*desc), desc);
    napi_coerce_to_native_binding_object(
        env, offscreenCanvas, DetachOffscreenCanvas, AttachOffscreenCanvas, value, nullptr);
    napi_wrap(
        env, offscreenCanvas, value,
        [](napi_env env, void* data, void* hint) {
            LOGD("Finalizer for offscreen canvas is called");
            auto wrapper = reinterpret_cast<JSOffscreenCanvas*>(data);
            delete wrapper;
            wrapper = nullptr;
        },
        nullptr, nullptr);
    return offscreenCanvas;
}

napi_value JSOffscreenCanvas::InitOffscreenCanvas(napi_env env)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_GETTER_SETTER("width", JsGetWidth, JsSetWidth),
        DECLARE_NAPI_GETTER_SETTER("height", JsGetHeight, JsSetHeight),
        DECLARE_NAPI_FUNCTION("transferToImageBitmap", JsTransferToImageBitmap),
        DECLARE_NAPI_FUNCTION("getContext", JsGetContext),
    };
    napi_status status = napi_define_class(
        env, "OffscreenCanvas", NAPI_AUTO_LENGTH, Constructor, nullptr, sizeof(desc) / sizeof(*desc), desc, &object);
    if (status != napi_ok) {
        LOGW("Initialize offscreen canvas failed");
        return nullptr;
    }
    return object;
}

void JSOffscreenCanvas::JSBind(BindingTarget globalObj, void* nativeEngine)
{
    if (!nativeEngine) {
        return;
    }
    napi_env env = reinterpret_cast<napi_env>(nativeEngine);

    napi_value jsGlobalObj = nullptr;
    napi_get_global(env, &jsGlobalObj);

    napi_value result = InitOffscreenCanvas(env);
    napi_set_named_property(env, jsGlobalObj, "OffscreenCanvas", result);
}

napi_value JSOffscreenCanvas::Constructor(napi_env env, napi_callback_info info)
{
    ContainerScope scope(Container::CurrentIdSafely());
    size_t argc = 2;
    napi_value thisVar = nullptr;
    napi_value argv[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    if (argc < ARGS_COUNT_TWO || argv[0] == nullptr || argv[1] == nullptr) {
        LOGW("Invalid args.");
        return nullptr;
    }
    double fWidth = 0.0;
    double fHeight = 0.0;
    auto workCanvas = new (std::nothrow) JSOffscreenCanvas();
    auto context = PipelineBase::GetCurrentContext();
    if (context != nullptr) {
        workCanvas->instanceId_ = context->GetInstanceId();
    }
    if (napi_get_value_double(env, argv[0], &fWidth) == napi_ok) {
        fWidth = PipelineBase::Vp2PxWithCurrentDensity(fWidth);
        workCanvas->SetWidth(fWidth);
    }
    if (napi_get_value_double(env, argv[1], &fHeight) == napi_ok) {
        fHeight = PipelineBase::Vp2PxWithCurrentDensity(fHeight);
        workCanvas->SetHeight(fHeight);
    }

    napi_coerce_to_native_binding_object(
        env, thisVar, DetachOffscreenCanvas, AttachOffscreenCanvas, workCanvas, nullptr);
    napi_wrap(
        env, thisVar, workCanvas,
        [](napi_env env, void* data, void* hint) {
            LOGD("Finalizer for offscreen canvas is called");
            auto workCanvas = reinterpret_cast<JSOffscreenCanvas*>(data);
            delete workCanvas;
            workCanvas = nullptr;
        },
        nullptr, nullptr);
    return thisVar;
}

napi_value JSOffscreenCanvas::JsGetWidth(napi_env env, napi_callback_info info)
{
    JSOffscreenCanvas* me = static_cast<JSOffscreenCanvas*>(GetNapiCallbackInfoAndThis(env, info));
    napi_value defaultWidth = nullptr;
    napi_create_double(env, 0.0, &defaultWidth);
    return (me != nullptr && !me->isDetached_) ? me->OnGetWidth(env) : defaultWidth;
}

napi_value JSOffscreenCanvas::JsGetHeight(napi_env env, napi_callback_info info)
{
    JSOffscreenCanvas* me = static_cast<JSOffscreenCanvas*>(GetNapiCallbackInfoAndThis(env, info));
    napi_value defaultHeight = nullptr;
    napi_create_double(env, 0.0, &defaultHeight);
    return (me != nullptr && !me->isDetached_) ? me->OnGetHeight(env) : defaultHeight;
}

napi_value JSOffscreenCanvas::JsSetWidth(napi_env env, napi_callback_info info)
{
    ContainerScope scope(Container::CurrentIdSafely());
    JSOffscreenCanvas* me = static_cast<JSOffscreenCanvas*>(GetNapiCallbackInfoAndThis(env, info));
    return (me != nullptr && !me->isDetached_) ? me->OnSetWidth(env, info) : nullptr;
}

napi_value JSOffscreenCanvas::JsSetHeight(napi_env env, napi_callback_info info)
{
    ContainerScope scope(Container::CurrentIdSafely());
    JSOffscreenCanvas* me = static_cast<JSOffscreenCanvas*>(GetNapiCallbackInfoAndThis(env, info));
    return (me != nullptr && !me->isDetached_) ? me->OnSetHeight(env, info) : nullptr;
}
napi_value JSOffscreenCanvas::JsTransferToImageBitmap(napi_env env, napi_callback_info info)
{
    ContainerScope scope(Container::CurrentIdSafely());
    JSOffscreenCanvas* me = static_cast<JSOffscreenCanvas*>(GetNapiCallbackInfoAndThis(env, info));
    if (me->isDetached_) {
        JSException::Throw("%s", "Failed to execute 'transferToImageBitmap' on 'OffscreenCanvas': Cannot transfer an "
                                 "ImageBitmap from a detached OffscreenCanvas");
        return nullptr;
    }
    napi_value defaultImage = nullptr;
    napi_create_object(env, &defaultImage);
    return (me != nullptr) ? me->onTransferToImageBitmap(env) : defaultImage;
}

napi_value JSOffscreenCanvas::JsGetContext(napi_env env, napi_callback_info info)
{
    ContainerScope scope(Container::CurrentIdSafely());
    JSOffscreenCanvas* me = static_cast<JSOffscreenCanvas*>(GetNapiCallbackInfoAndThis(env, info));
    if (me->isDetached_) {
        JSException::Throw(
            "%s", "Failed to execute 'getContext' on 'OffscreenCanvas': OffscreenCanvas object is detached");
        return nullptr;
    }
    napi_value defaultContext = nullptr;
    napi_create_object(env, &defaultContext);
    return (me != nullptr) ? me->onGetContext(env, info) : defaultContext;
}

napi_value JSOffscreenCanvas::OnGetWidth(napi_env env)
{
    double fWidth = PipelineBase::Px2VpWithCurrentDensity(GetWidth());
    napi_value width = nullptr;
    napi_create_double(env, fWidth, &width);
    return width;
}

napi_value JSOffscreenCanvas::OnGetHeight(napi_env env)
{
    double fHeight = PipelineBase::Px2VpWithCurrentDensity(GetHeight());
    napi_value height = nullptr;
    napi_create_double(env, fHeight, &height);
    return height;
}

napi_value JSOffscreenCanvas::OnSetWidth(napi_env env, napi_callback_info info)
{
    CHECK_NULL_RETURN(offscreenCanvasPattern_, nullptr);
    size_t argc = 0;
    napi_value argv = nullptr;
    napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
    if (argc != ARGS_COUNT_ONE) {
        LOGD("Invalid args.");
        return nullptr;
    }
    napi_get_cb_info(env, info, &argc, &argv, nullptr, nullptr);
    if (argv == nullptr) {
        return nullptr;
    }
    double width = 0.0;
    if (napi_get_value_double(env, argv, &width) == napi_ok) {
        width = PipelineBase::Vp2PxWithCurrentDensity(width);
    } else {
        return nullptr;
    }

    if (width_ != width) {
        width_ = width;
        offscreenCanvasPattern_->UpdateSize(width_, height_);
    }
    return nullptr;
}

napi_value JSOffscreenCanvas::OnSetHeight(napi_env env, napi_callback_info info)
{
    CHECK_NULL_RETURN(offscreenCanvasPattern_, nullptr);
    size_t argc = 0;
    napi_value argv = nullptr;
    napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
    if (argc != ARGS_COUNT_ONE) {
        LOGD("Invalid args.");
        return nullptr;
    }
    napi_get_cb_info(env, info, &argc, &argv, nullptr, nullptr);
    if (argv == nullptr) {
        return nullptr;
    }
    double height = 0.0;
    if (napi_get_value_double(env, argv, &height) == napi_ok) {
        height = PipelineBase::Vp2PxWithCurrentDensity(height);
    } else {
        return nullptr;
    }

    if (height_ != height) {
        height_ = height;
        offscreenCanvasPattern_->UpdateSize(width_, height_);
    }
    return nullptr;
}

napi_value JSOffscreenCanvas::onTransferToImageBitmap(napi_env env)
{
    std::string type = "ImageBitmap";
    if (offscreenCanvasContext_ == nullptr) {
        return nullptr;
    }
    uint32_t id = offscreenCanvasContext_->GetId();
    auto final_height = static_cast<uint32_t>(GetHeight());
    auto final_width = static_cast<uint32_t>(GetWidth());
    napi_value renderImage = nullptr;
    napi_value jsType = nullptr;
    napi_value jsId = nullptr;
    napi_value jsHeight = nullptr;
    napi_value jsWidth = nullptr;
    napi_create_object(env, &renderImage);
    napi_create_string_utf8(env, type.c_str(), type.length(), &jsType);
    napi_create_uint32(env, id, &jsId);
    napi_create_double(env, final_height, &jsHeight);
    napi_create_double(env, final_width, &jsWidth);
    napi_set_named_property(env, renderImage, "__type", jsType);
    napi_set_named_property(env, renderImage, "__id", jsId);
    napi_set_named_property(env, renderImage, "height", jsHeight);
    napi_set_named_property(env, renderImage, "width", jsWidth);
    return renderImage;
}

napi_value JSOffscreenCanvas::onGetContext(napi_env env, napi_callback_info info)
{
    isGetContext_ = true;
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    napi_value offscreenCanvas = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &offscreenCanvas, nullptr));
    if (argc < ARGS_COUNT_ONE || argc > ARGS_COUNT_TWO) {
        LOGD("Invalid args.");
        return nullptr;
    }
    if (argv[0] == nullptr) {
        return nullptr;
    }
    if (!Container::IsCurrentUseNewPipeline()) {
        return nullptr;
    }

    size_t textLen = 0;
    std::string contextType = "";
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &textLen);
    std::unique_ptr<char[]> text = std::make_unique<char[]>(textLen + 1);
    napi_get_value_string_utf8(env, argv[0], text.get(), textLen + 1, &textLen);
    contextType = text.get();
    if (contextType == "2d") {
        contextType_ = ContextType::CONTEXT_2D;
        napi_value contextObj = CreateContext2d(env, GetWidth(), GetHeight());
        if (contextObj == nullptr) {
            return nullptr;
        }
        napi_value isSucceed = nullptr;
        if (napi_get_named_property(env, contextObj, "__isSucceed", &isSucceed) == napi_ok) {
            bool value = true;
            napi_get_value_bool(env, isSucceed, &value);
            if (!value) {
                return nullptr;
            }
        } else {
            return nullptr;
        }
        if (argv[1] != nullptr) {
            panda::Local<panda::ObjectRef> localValue = NapiValueToLocalValue(argv[1]);
            JSObject jsObject(localValue);
            offscreenCanvasSettings_ = jsObject.Unwrap<JSRenderingContextSettings>();
            if (offscreenCanvasSettings_ != nullptr && offscreenCanvasContext_ != nullptr) {
                bool anti = offscreenCanvasSettings_->GetAntialias();
                offscreenCanvasContext_->SetAnti(anti);
                offscreenCanvasContext_->SetAntiAlias();
            }
        }
        return contextObj;
    }
    return nullptr;
}

napi_value JSOffscreenCanvas::CreateContext2d(napi_env env, double width, double height)
{
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_value constructor = nullptr;
    status = napi_get_named_property(env, global, "OffscreenCanvasRenderingContext2D", &constructor);
    if (status != napi_ok) {
        return nullptr;
    }

    napi_value thisVal = nullptr;
    napi_create_object(env, &thisVal);
    status = napi_new_instance(env, constructor, 0, nullptr, &thisVal);
    if (status != napi_ok) {
        return nullptr;
    }
    if (instanceId_ != -1) {
        offscreenCanvasPattern_ = AceType::MakeRefPtr<NG::OffscreenCanvasPattern>(
            GetContext(), static_cast<int32_t>(width), static_cast<int32_t>(height));
    } else {
        offscreenCanvasPattern_ = AceType::MakeRefPtr<NG::OffscreenCanvasPattern>(
            static_cast<int32_t>(width), static_cast<int32_t>(height));
    }
    if (offscreenCanvasPattern_ == nullptr) {
        return thisVal;
    }
    bool isSucceed = true;
    napi_value value = nullptr;
    if (!offscreenCanvasPattern_->IsSucceed()) {
        isSucceed = false;
        napi_create_int32(env, isSucceed, &value);
        napi_set_named_property(env, thisVal, "__isSucceed", value);
        return thisVal;
    }
    napi_create_int32(env, isSucceed, &value);
    napi_set_named_property(env, thisVal, "__isSucceed", value);

    panda::Local<panda::ObjectRef> localValue = NapiValueToLocalValue(thisVal);
    JSObject jsObject(localValue);
    offscreenCanvasContext_ = Referenced::Claim(jsObject.Unwrap<JSOffscreenRenderingContext>());
    offscreenCanvasContext_->SetInstanceId(Container::CurrentId());
    offscreenCanvasContext_->SetOffscreenPattern(offscreenCanvasPattern_);
    offscreenCanvasContext_->AddOffscreenCanvasPattern(offscreenCanvasPattern_);
    return thisVal;
}
} // namespace OHOS::Ace::Framework
