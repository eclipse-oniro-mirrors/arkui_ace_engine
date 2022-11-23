/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/common/layout_inspector.h"

#include <string>

#include "third_party/skia/include/core/SkString.h"
#include "third_party/skia/include/utils/SkBase64.h"
#include "wm/window.h"

#include "base/thread/background_task_executor.h"
#include "core/common/connect_server_manager.h"
#include "core/common/container.h"
#include "core/components_v2/inspector/inspector.h"
#include "foundation/ability/ability_runtime/frameworks/native/runtime/connect_server_manager.h"

namespace OHOS::Ace {

bool LayoutInspector::layoutInspectorStatus_ = false;

void LayoutInspector::SupportInspector()
{
    auto container = Container::Current();
    if (!container) {
        LOGE("container null");
        return;
    }
    if (!layoutInspectorStatus_) {
        SetlayoutInspectorStatus();
        if (!layoutInspectorStatus_) {
            return;
        }
    }
    auto pipelineContext = AceType::DynamicCast<PipelineContext>(container->GetPipelineContext());
    if (!pipelineContext) {
        LOGE("pipelineContext null");
        return;
    }
    std::string jsonTreeStr = V2::Inspector::GetInspectorTree(pipelineContext);
    OHOS::sptr<OHOS::Rosen::Window> window = OHOS::Rosen::Window::GetTopWindowWithId(container->GetWindowId());
    if (!window) {
        LOGE("GetWindow is null!");
        return;
    }
    auto pixelMap = window->Snapshot();
    if (pixelMap == nullptr) {
        LOGE("Pixel is null");
        return;
    }
    std::string pixelStr = "";
    auto data = (*pixelMap).GetPixels();
    auto height = (*pixelMap).GetHeight();
    auto stride = (*pixelMap).GetRowBytes();
    for (int32_t i = 0; i < height * stride; i++) {
        pixelStr += std::to_string(*(data + i));
    }
    auto message = JsonUtil::Create(true);
    message->Put("type", "snapShot");
    message->Put("width", (*pixelMap).GetWidth());
    message->Put("height", height);
    message->Put("posX", container->GetViewPosX());
    message->Put("posY", container->GetViewPosY());
    int32_t encodeLength = SkBase64::Encode(data, height * stride, nullptr);
    SkString info(encodeLength);
    SkBase64::Encode(data, height * stride, info.writable_str());
    message->Put("pixelMapBase64", info.c_str());
    auto sendTask = [jsonTreeStr, jsonSnapshotStr = message->ToString(), container]() {
        if (container->IsUseStageModel()) {
            OHOS::AbilityRuntime::ConnectServerManager::Get().SendInspector(jsonTreeStr, jsonSnapshotStr);
        } else {
            OHOS::Ace::ConnectServerManager::Get().SendInspector(jsonTreeStr, jsonSnapshotStr);
        }
    };
    BackgroundTaskExecutor::GetInstance().PostTask(std::move(sendTask));
}

void LayoutInspector::SetlayoutInspectorStatus()
{
    auto container = Container::Current();
    if (!container) {
        LOGE("container is null");
        return;
    }
    if (container->IsUseStageModel()) {
        layoutInspectorStatus_ = OHOS::AbilityRuntime::ConnectServerManager::Get().GetlayoutInspectorStatus();
        return;
    }
    layoutInspectorStatus_ = OHOS::Ace::ConnectServerManager::Get().GetlayoutInspectorStatus();
}

} // namespace OHOS::Ace