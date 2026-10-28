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

// Host-test stand-in for core/pipeline_ng/pipeline_context.h. It shadows the
// real header through include-path order (see
// interaction_observer_manager_test.cpp) and provides exactly the surface
// UIInfoCollectionManager consumes: the instance id of the pipeline.
#ifndef FOUNDATION_ACE_TEST_CAPI_HOST_MANAGER_STUBS_PIPELINE_CONTEXT_H
#define FOUNDATION_ACE_TEST_CAPI_HOST_MANAGER_STUBS_PIPELINE_CONTEXT_H

#include <cstdint>

#include "ui/base/ace_type.h"

namespace OHOS::Ace::NG {
class PipelineContext : public AceType {
public:
    explicit PipelineContext(int32_t instanceId) : instanceId_(instanceId) {}

    int32_t GetInstanceId() const { return instanceId_; }

private:
    int32_t instanceId_ = -1;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_TEST_CAPI_HOST_MANAGER_STUBS_PIPELINE_CONTEXT_H
