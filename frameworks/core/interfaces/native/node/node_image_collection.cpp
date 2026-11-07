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

#include "core/interfaces/native/node/node_image_collection.h"

#include <memory>
#include <utility>

#include "interfaces/native/native_type.h"

#include "core/common/container.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/render/adapter/component_snapshot.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {
int32_t PostImageCollectionTask(int32_t contextId, void* data, void (*task)(void*, int32_t))
{
    auto container = Container::GetContainer(contextId);
    if (!container || !task) {
        return ARKUI_ERROR_CODE_UI_CONTEXT_INVALID;
    }
    auto executor = container->GetTaskExecutor();
    if (!executor || !container->GetPipelineContext()) {
        return ARKUI_ERROR_CODE_UI_CONTEXT_INVALID;
    }
    // The executor owns its UI task runner. Retaining it until execution lets an
    // accepted request report invalidation even after the Container is removed.
    // Do not retain the caller's Context handle or keep the Container alive.
    auto posted = executor->PostTask(
        [contextId, weak = WeakPtr<Container>(container), executor, data, task]() {
            ContainerScope scope(contextId);
            auto original = weak.Upgrade();
            auto current = Container::GetContainer(contextId);
            auto pipeline = current ? current->GetPipelineContext() : nullptr;
            auto valid = original && current == original && pipeline && !pipeline->IsDestroyed();
            task(data, valid ? ARKUI_ERROR_CODE_NO_ERROR : ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
        }, TaskExecutor::TaskType::UI, "ArkUIImageCollection");
    return posted ? ARKUI_ERROR_CODE_NO_ERROR : ARKUI_ERROR_CODE_UI_CONTEXT_INVALID;
}

int32_t GetImageByNodeId(int32_t nodeId, void* mediaPixel)
{
    auto& result = *static_cast<std::shared_ptr<Media::PixelMap>*>(mediaPixel);
    result.reset();
    auto node = AceType::DynamicCast<FrameNode>(ElementRegister::GetInstance()->GetNodeById(nodeId));
    if (!node) {
        return ARKUI_ERROR_CODE_NODE_NOT_FOUND;
    }
    // Only ArkUI FrameNodes are resolved. A Web host uses the normal snapshot path;
    // no Web-internal IDs, enumeration or image collection protocol is involved.
    if (node->GetTag() == V2::IMAGE_ETS_TAG) {
        auto pattern = node->GetPattern<ImagePattern>();
        auto image = pattern ? pattern->GetCanvasImage() : nullptr;
        auto pixelmap = image ? image->GetPixelMap() : nullptr;
        if (pixelmap) {
            result = pixelmap->GetPixelMapSharedPtr();
        }
        if (result) {
            return ARKUI_ERROR_CODE_NO_ERROR;
        }
    }
    SnapshotOptions options;
    options.waitUntilRenderFinished = true;
    auto captured = ComponentSnapshot::GetSyncByUniqueId(nodeId, options);
    result = std::move(captured.second);
    return captured.first;
}
} // namespace OHOS::Ace::NG
