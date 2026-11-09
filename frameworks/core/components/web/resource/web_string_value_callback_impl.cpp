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

#include "core/components/web/resource/web_string_value_callback_impl.h"

#include "base/thread/task_executor.h"
#include "core/components/web/resource/web_delegate.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace {

void WebStringValueCallbackImpl::OnReceiveValue(const std::string& value)
{
    auto callback = std::move(callback_);
    if (!callback) {
        return;
    }
    auto delegate = webDelegate_.Upgrade();
    if (!delegate) {
        callback(value);
        return;
    }
    auto context = delegate->context_.Upgrade();
    if (!context) {
        callback(value);
        return;
    }
    context->GetTaskExecutor()->PostTask(
        [callback = std::move(callback), value]() {
            if (callback) {
                callback(value);
            }
        },
        TaskExecutor::TaskType::PLATFORM, "ArkUIWebStringValueCallback");
}

} // namespace OHOS::Ace