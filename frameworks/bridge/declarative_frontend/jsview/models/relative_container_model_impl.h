/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_RELATIVE_CONTAINER_MODEL_IMPL_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_RELATIVE_CONTAINER_MODEL_IMPL_H

#include "core/components_ng/pattern/relative_container/relative_container_model.h"
#include "frameworks/core/components/relative_container/relative_container_component.h"

namespace OHOS::Ace::Framework {

class ACE_EXPORT RelativeContainerModelImpl : public OHOS::Ace::RelativeContainerModel {
public:
    void Create() override;
    void SetBarrier(const std::vector<BarrierInfo>& barrierInfo) override {}
    void SetGuideline(const std::vector<GuidelineInfo>& guidelineInfo) override {}
};

} // namespace OHOS::Ace::Framework

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_RELATIVE_CONTAINER_MODEL_IMPL_H