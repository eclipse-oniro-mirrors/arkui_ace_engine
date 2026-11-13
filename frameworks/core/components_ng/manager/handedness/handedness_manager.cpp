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

#include "core/components_ng/manager/handedness/handedness_manager.h"

#ifdef MSDP_MOTION_EXISTS
#include "motion_client.h"
#include "motion_agent_type.h"
#include "motion_callback_stub.h"
#endif

#include "base/log/log.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/pipeline/base/element_register.h"
#include "core/pipeline_ng/pipeline_context.h"

#ifdef MSDP_MOTION_EXISTS
namespace OHOS::Ace::NG {
class HandednessMotionCallback : public Msdp::MotionCallbackStub {
public:
    HandednessMotionCallback() = default;
    ~HandednessMotionCallback() override = default;
    void OnMotionChanged(const Msdp::MotionEvent& event) override {}
};
} // namespace OHOS::Ace::NG
#endif

namespace OHOS::Ace::NG {

HandednessManager::HandednessManager(const WeakPtr<PipelineContext>& context) : context_(context) {}

void HandednessManager::SetOperatingHand(int32_t operatingHand)
{
    operatingHand_ = operatingHand;
}

void HandednessManager::SetLastFocusNodeId(int32_t nodeId)
{
    lastFocusNodeId_ = nodeId;
}

void HandednessManager::SetReferenceNodeId(int32_t nodeId)
{
    referenceNodeId_ = nodeId;
}

bool HandednessManager::IsBreakpointMet()
{
    // WidthBreakpoint: WIDTH_XS=0, WIDTH_SM=1, WIDTH_MD=2, WIDTH_LG=3, WIDTH_XL=4
    // HeightBreakpoint: HEIGHT_SM=0, HEIGHT_MD=1, HEIGHT_LG=2
    constexpr int32_t WIDTH_BREAKPOINT_MD = 2;
    constexpr int32_t WIDTH_BREAKPOINT_LG = 3;
    constexpr int32_t HEIGHT_BREAKPOINT_MD = 1;
    int32_t widthBp = ViewAbstract::GetWindowWidthBreakpoint();
    int32_t heightBp = ViewAbstract::GetWindowHeightBreakpoint();
    if (widthBp < 0 || heightBp < 0) {
        LOGW("IsBreakpointMet failed to get breakpoint, width=%{public}d height=%{public}d", widthBp, heightBp);
        return false;
    }
    bool met = (widthBp >= WIDTH_BREAKPOINT_LG) ||
        (widthBp >= WIDTH_BREAKPOINT_MD && heightBp >= HEIGHT_BREAKPOINT_MD);
    return met;
}

int32_t HandednessManager::QueryHoldHandPosture()
{
    return QueryHoldHandPostureFromMsdp();
}

int32_t HandednessManager::QueryHoldHandPostureFromMsdp()
{
#ifdef MSDP_MOTION_EXISTS
    auto callback = sptr<HandednessMotionCallback>(new HandednessMotionCallback());
    auto& motionClient = Msdp::MotionClient::GetInstance();
    int32_t ret = motionClient.SubscribeCallback(Msdp::MOTION_TYPE_HOLDING_HAND, callback);
    if (ret != 0) {
        LOGW("QueryHoldHandPostureFromMsdp SubscribeCallback failed ret=%{public}d", ret);
        return 0;
    }
    auto event = motionClient.GetMotionData(Msdp::MOTION_TYPE_HOLDING_HAND);
    motionClient.UnsubscribeCallback(Msdp::MOTION_TYPE_HOLDING_HAND, callback);
    return event.status >= 0 ? event.status : 0;
#else
    return 0; // MSDP 不可用，降级为未知
#endif
}

PopupSide HandednessManager::QueryPopupSideByHoldingHand()
{
    if (!IsBreakpointMet()) {
        return PopupSide::CENTER;
    }
    int32_t posture = QueryHoldHandPostureFromMsdp();
    if (posture == 1) {
        // 左手握持：操作手也是左手→LEFT，否则→CENTER
        return operatingHand_ == 1 ? PopupSide::LEFT : PopupSide::CENTER;
    }
    if (posture == 2) {
        // 右手握持：操作手也是右手→RIGHT，否则→CENTER
        return operatingHand_ == 2 ? PopupSide::RIGHT : PopupSide::CENTER;
    }
    if (posture == 3) {
        // 双手握持：跟随操作手
        if (operatingHand_ == 1) {
            return PopupSide::LEFT;
        }
        if (operatingHand_ == 2) {
            return PopupSide::RIGHT;
        }
        return PopupSide::CENTER;
    }
    // posture == 0/16 或查询失败：未知姿态，居中
    return PopupSide::CENTER;
}

std::pair<int32_t, OffsetF> HandednessManager::QueryAnchorPosition()
{
    return QueryAnchorPosition(-1);
}

std::pair<int32_t, OffsetF> HandednessManager::QueryAnchorPosition(int32_t nodeId)
{
    constexpr int32_t INVALID_NODE_ID = -1;
    if (!IsBreakpointMet()) {
        return { INVALID_NODE_ID, OffsetF(0, 0) };
    }
    // 传 -1 时走原有逻辑，使用 lastFocusNodeId_
    int32_t targetId = (nodeId < 0) ? lastFocusNodeId_ : nodeId;
    if (targetId < 0) {
        return { INVALID_NODE_ID, OffsetF(0, 0) };
    }
    auto node = ElementRegister::GetInstance()->GetNodeById(targetId);
    if (!node) {
        return { INVALID_NODE_ID, OffsetF(0, 0) };
    }
    auto frameNode = DynamicCast<FrameNode>(node);
    if (!frameNode) {
        return { INVALID_NODE_ID, OffsetF(0, 0) };
    }
    auto geoNode = frameNode->GetGeometryNode();
    CHECK_NULL_RETURN(geoNode, std::make_pair(INVALID_NODE_ID, OffsetF(0, 0)));
    auto frameRect = geoNode->GetFrameRect();
    auto nodeOffset = frameNode->GetOffsetRelativeToWindow();
    float centerX = nodeOffset.GetX() + frameRect.Width() / 2.0f;
    float centerY = nodeOffset.GetY() + frameRect.Height() / 2.0f;
    return { targetId, OffsetF(centerX, centerY) };
}

} // namespace OHOS::Ace::NG
