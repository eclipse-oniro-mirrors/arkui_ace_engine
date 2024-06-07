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

#include "core/components_ng/render/adapter/component_snapshot.h"

#include <memory>

#include "transaction/rs_interfaces.h"

#include "base/log/log_wrapper.h"
#include "bridge/common/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/stack/stack_pattern.h"
#include "core/components_ng/render/adapter/rosen_render_context.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

constexpr int32_t DELAY_TIME_FIFTY = 50;
constexpr int32_t DELAY_TIME_DEFAULT = 10;
namespace {
class CustomizedCallback : public Rosen::SurfaceCaptureCallback {
public:
    CustomizedCallback(ComponentSnapshot::JsCallback&& jsCallback, WeakPtr<FrameNode> node)
        : callback_(std::move(jsCallback)), node_(std::move(node))
    {}
    ~CustomizedCallback() override = default;
    void OnSurfaceCapture(std::shared_ptr<Media::PixelMap> pixelMap) override
    {
        if (callback_ == nullptr) {
            auto node = node_.Upgrade();
            CHECK_NULL_VOID(node);
            Inspector::RemoveOffscreenNode(node);
            return;
        }
        if (!pixelMap) {
            TAG_LOGW(AceLogTag::ACE_COMPONENT_SNAPSHOT, "Internal error! The pixelmap returned by the system is null");
            callback_(nullptr, ERROR_CODE_INTERNAL_ERROR, [node = node_]() {
                auto frameNode = node.Upgrade();
                CHECK_NULL_VOID(frameNode);
                Inspector::RemoveOffscreenNode(frameNode);
            });
        } else {
            callback_(pixelMap, ERROR_CODE_NO_ERROR, [node = node_]() {
                auto frameNode = node.Upgrade();
                CHECK_NULL_VOID(frameNode);
                Inspector::RemoveOffscreenNode(frameNode);
            });
        }
    }

private:
    ComponentSnapshot::JsCallback callback_;
    WeakPtr<FrameNode> node_;
};

class NormalCaptureCallback : public Rosen::SurfaceCaptureCallback {
public:
    explicit NormalCaptureCallback(ComponentSnapshot::NormalCallback&& callback) : callback_(std::move(callback)) {}
    ~NormalCaptureCallback() override = default;
    void OnSurfaceCapture(std::shared_ptr<Media::PixelMap> pixelMap) override
    {
        CHECK_NULL_VOID(callback_);
        callback_(pixelMap);
    }

private:
    ComponentSnapshot::NormalCallback callback_;
};
} // namespace

std::shared_ptr<Rosen::RSNode> ComponentSnapshot::GetRsNode(const RefPtr<FrameNode>& node)
{
    CHECK_NULL_RETURN(node, nullptr);
    auto context = AceType::DynamicCast<RosenRenderContext>(node->GetRenderContext());
    CHECK_NULL_RETURN(context, nullptr);
    auto rsNode = context->GetRSNode();
    return rsNode;
}

void ComponentSnapshot::Get(const std::string& componentId, JsCallback&& callback)
{
    auto node = Inspector::GetFrameNodeByKey(componentId);
    if (!node) {
        callback(nullptr, ERROR_CODE_INTERNAL_ERROR, nullptr);
        TAG_LOGW(AceLogTag::ACE_COMPONENT_SNAPSHOT,
            "Can't find a component that id or key are %{public}s, Please check your parameters are correct",
            componentId.c_str());
        return;
    }
    auto rsNode = GetRsNode(node);
    if (!rsNode) {
        callback(nullptr, ERROR_CODE_INTERNAL_ERROR, nullptr);
        TAG_LOGW(AceLogTag::ACE_COMPONENT_SNAPSHOT,
            "RsNode is null from FrameNode(id=%{public}s)",
            componentId.c_str());
        return;
    }
    auto& rsInterface = Rosen::RSInterfaces::GetInstance();
    rsInterface.TakeSurfaceCaptureForUI(rsNode, std::make_shared<CustomizedCallback>(std::move(callback), nullptr));
}

void ComponentSnapshot::Create(
    const RefPtr<AceType>& customNode, JsCallback&& callback, bool enableInspector, const int32_t delayTime, bool flag)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto stackNode = FrameNode::CreateFrameNode(V2::STACK_ETS_TAG, nodeId, AceType::MakeRefPtr<StackPattern>());
    auto uiNode = AceType::DynamicCast<UINode>(customNode);
    if (!uiNode) {
        callback(nullptr, ERROR_CODE_INTERNAL_ERROR, nullptr);
        return;
    }
    auto node = AceType::DynamicCast<FrameNode>(customNode);
    if (!node) {
        stackNode->AddChild(uiNode);
        node = stackNode;
    }
    FrameNode::ProcessOffscreenNode(node);
    TAG_LOGI(AceLogTag::ACE_COMPONENT_SNAPSHOT, "Process off screen Node finished, root size = %{public}s",
        node->GetGeometryNode()->GetFrameSize().ToString().c_str());

    if (enableInspector) {
        Inspector::AddOffscreenNode(node);
    }

    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto executor = pipeline->GetTaskExecutor();
    CHECK_NULL_VOID(executor);

    auto time = delayTime - DELAY_TIME_FIFTY <= 0 ? DELAY_TIME_DEFAULT : delayTime - DELAY_TIME_FIFTY;
    executor->PostDelayedTask(
        [callback, node, enableInspector, pipeline, flag]() mutable {
            auto executor = pipeline->GetTaskExecutor();
            CHECK_NULL_VOID(executor);
            if (flag) {
                pipeline->FlushUITasks();
                pipeline->FlushMessages();
            }
            executor->PostDelayedTask(
                [callback, node, enableInspector]() mutable {
                    auto rsNode = GetRsNode(node);
                    TAG_LOGI(AceLogTag::ACE_COMPONENT_SNAPSHOT,
                        "Begin to take surfaceCapture for ui, rootNode = %{public}s", node->GetTag().c_str());
                    auto& rsInterface = Rosen::RSInterfaces::GetInstance();
                    rsInterface.TakeSurfaceCaptureForUI(
                        rsNode,
                        std::make_shared<CustomizedCallback>(std::move(callback), enableInspector ? node : nullptr));
                },
                TaskExecutor::TaskType::UI, DELAY_TIME_FIFTY, "ArkUIComponentSnapshotCreateCapture_1");
        },
        TaskExecutor::TaskType::UI, time, "ArkUIComponentSnapshotCreateCapture_0");
}

void ComponentSnapshot::GetNormalCapture(const RefPtr<FrameNode>& frameNode, NormalCallback&& callback)
{
    auto rsNode = GetRsNode(frameNode);
    auto& rsInterface = Rosen::RSInterfaces::GetInstance();
    rsInterface.TakeSurfaceCaptureForUI(rsNode, std::make_shared<NormalCaptureCallback>(std::move(callback)));
}
} // namespace OHOS::Ace::NG
