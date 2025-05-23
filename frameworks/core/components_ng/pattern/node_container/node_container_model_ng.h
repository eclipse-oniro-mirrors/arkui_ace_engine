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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NODE_CONTAINER_NODE_CONTAINER_MODEL_NG_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NODE_CONTAINER_NODE_CONTAINER_MODEL_NG_H

#include "core/components_ng/pattern/node_container/node_container_model.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT NodeContainerModelNG : public NodeContainerModel {
public:
    void Create() override;
    void SetMakeFunction(std::function<RefPtr<UINode>()>&& makeFunc) override;
    void SetOnResize(std::function<void(const SizeF& size)>&& resizeFunc) override;
    void SetOnAppear(std::function<void()>&& onAppearCallback) override;
    void SetOnDisAppear(std::function<void()>&& onDisAppearCallback) override;
    void BindController(std::function<void()>&& resetFunc) override;
    void SetOnTouchEvent(TouchEventFunc &&touchEventFunc) override;
    void SetOnAttach(std::function<void()>&& onAttachCallback) override;
    void SetOnDetach(std::function<void()>&& onDetachCallback) override;
    void SetOnWillBind(std::function<void(int32_t)>&& onWillBindCallback) override;
    void SetOnWillUnbind(std::function<void(int32_t)>&& onWillUnbindCallback) override;
    void SetOnBind(std::function<void(int32_t)>&& onBindCallback) override;
    void SetOnUnbind(std::function<void(int32_t)>&& onUnbindCallback) override;
    void ResetController() override;
    void FireMakeNode() override;
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_NODE_CONTAINER_NODE_CONTAINER_MODEL_NG_H
