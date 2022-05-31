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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_ADAPTER_ROSEN_RENDER_CONTEXT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_ADAPTER_ROSEN_RENDER_CONTEXT_H

#include "render_service_client/core/ui/rs_node.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkRefCnt.h"

#include "core/components_ng/render/render_context.h"

class SkImage;
class SkPicture;
class SkPictureRecorder;

namespace OHOS::Ace::NG {

class RosenRenderContext : public RenderContext {
public:
    RosenRenderContext() = default;
    ~RosenRenderContext() override;

    void InitContext(bool isRoot = false) override;

    void SyncGeometryProperties(GeometryNode* geometryNode) override;

    void RebuildFrame(FrameNode* self) override;

    void UpdateBgColor(const Color& value) override;

    void* GetCanvas() override;
    void Restore() override;

    const std::shared_ptr<Rosen::RSNode>& GetRSNode();

    void StartRecording(float x, float y, float width, float height) override;

    void StopRecordingIfNeeded() override;

    bool IsRecording()
    {
        return !!recordingCanvas_;
    }

    void StartPictureRecording(float x, float y, float width, float height);
    sk_sp<SkPicture> FinishRecordingAsPicture();

    void SetDrawContentAtLast(bool usedrawContentLastOrder) override
    {
        if (rsNode_) {
            rsNode_->SetPaintOrder(usedrawContentLastOrder);
        }
    }

private:
    void ReCreateRsNodeTree(FrameNode* node);

    std::shared_ptr<Rosen::RSNode> rsNode_ = nullptr;
    SkPictureRecorder* recorder_ = nullptr;
    SkCanvas* recordingCanvas_ = nullptr;
    SkCanvas* rosenCanvas_ = nullptr;
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_ADAPTER_ROSEN_RENDER_CONTEXT_H
