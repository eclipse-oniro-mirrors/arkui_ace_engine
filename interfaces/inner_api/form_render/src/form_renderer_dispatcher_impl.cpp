/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "form_renderer_dispatcher_impl.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace Ace {
FormRendererDispatcherImpl::FormRendererDispatcherImpl(const std::shared_ptr<UIContent> uiContent)
    : uiContent_(uiContent) {}

void FormRendererDispatcherImpl::DispatchPointerEvent(
    const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent)
{
    auto uiContent = uiContent_.lock();
    if (!uiContent) {
        HILOG_ERROR("uiContent is nullptr");
        return;
    }

    uiContent->ProcessPointerEvent(pointerEvent);
}
} // namespace Ace
} // namespace OHOS
