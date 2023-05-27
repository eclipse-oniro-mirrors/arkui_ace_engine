/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "frameworks/bridge/declarative_frontend/jsview/js_blank.h"

#include "core/components_ng/pattern/blank/blank_model_ng.h"
#include "frameworks/bridge/declarative_frontend/jsview/models/blank_model_impl.h"
#include "frameworks/bridge/declarative_frontend/view_stack_processor.h"

namespace OHOS::Ace {
std::unique_ptr<BlankModel> BlankModel::instance_ = nullptr;
std::mutex BlankModel::mutex_;

BlankModel* BlankModel::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
#ifdef NG_BUILD
            instance_.reset(new NG::BlankModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                instance_.reset(new NG::BlankModelNG());
            } else {
                instance_.reset(new Framework::BlankModelImpl());
            }
#endif
        }
    }
    return instance_.get();
}
} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {
void JSBlank::Create(const JSCallbackInfo& info)
{
    CalcDimension blankMin;
    BlankModel::GetInstance()->Create();
    if (info.Length() < 1) {
        LOGD("Blank min needs at least 1 param");
        return;
    }
    if (info[0]->IsUndefined()) {
        BlankModel::GetInstance()->SetBlankMin(blankMin);
        return;
    }
    if (ParseJsDimensionVp(info[0], blankMin)) {
        if (blankMin.IsNegative()) {
            blankMin.SetValue(0.0);
        }
        BlankModel::GetInstance()->SetBlankMin(blankMin);
    }
}

void JSBlank::Height(const JSCallbackInfo& info)
{
    JSViewAbstract::JsHeight(info);
    CalcDimension value;
    if (!ParseJsDimensionVp(info[0], value)) {
        return;
    }

    BlankModel::GetInstance()->SetHeight(value);
}

void JSBlank::JSBind(BindingTarget globalObj)
{
    JSClass<JSBlank>::Declare("Blank");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSBlank>::StaticMethod("create", &JSBlank::Create, opt);
    JSClass<JSBlank>::StaticMethod("height", &JSBlank::Height, opt);
    JSClass<JSBlank>::StaticMethod("color", &JSViewAbstract::JsBackgroundColor, opt);
    JSClass<JSBlank>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSBlank>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSBlank>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSBlank>::StaticMethod("onHover", &JSInteractableView::JsOnHover);
    JSClass<JSBlank>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSBlank>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSBlank>::StaticMethod("onClick", &JSInteractableView::JsOnClick);
    JSClass<JSBlank>::StaticMethod("remoteMessage", &JSInteractableView::JsCommonRemoteMessage);

    JSClass<JSBlank>::Inherit<JSViewAbstract>();
    JSClass<JSBlank>::Bind<>(globalObj);
}
} // namespace OHOS::Ace::Framework
