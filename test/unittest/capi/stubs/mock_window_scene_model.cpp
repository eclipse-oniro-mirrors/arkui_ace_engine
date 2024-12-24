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

#include "mock_window_scene_model.h"

#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

RefPtr<FrameNode> WindowSceneModel::Create(int32_t persistentId)
{
    auto stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    ACE_SCOPED_TRACE("Create[%s][self:%d][TransformScene]", V2::WINDOW_SCENE_ETS_TAG, nodeId);
    auto node = FrameNode::GetOrCreateFrameNode(
        V2::WINDOW_SCENE_ETS_TAG, nodeId, []() { return nullptr; });
    stack->Push(node);
    return node;
}

void WindowSceneModel::SetAttractionEffect(const AttractionEffect& effect)
{ // A stub for mocking
}

RefPtr<FrameNode> WindowSceneModel::CreateNode(int32_t nodeId)
{
    return WindowSceneModel::Create(0);
}
} // namespace OHOS::Ace::NG
