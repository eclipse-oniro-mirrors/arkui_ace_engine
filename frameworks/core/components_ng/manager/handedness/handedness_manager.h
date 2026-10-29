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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_HANDEDNESS_HANDEDNESS_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_HANDEDNESS_HANDEDNESS_MANAGER_H

#include <utility>

#include "base/geometry/ng/offset_t.h"
#include "base/memory/ace_type.h"

namespace OHOS::Ace::NG {
class PipelineContext;
enum class PopupSide;

/**
 * @brief 智感握姿公共服务 Manager。
 *
 * 职责：
 * - 管理操作手状态（来自 MMI 3601，TouchEvent.operatingHand）
 * - 管理最后焦点节点 id（FOLLOW_ANCHOR 锚点源）
 * - 同步查询握持手（MSDP 3605），通过 GetMotionData 获取 posture 值
 * - 对外提供两个查询接口：
 *   1. QueryPopupSideByHoldingHand() → PopupSide（侧别）
 *   2. QueryAnchorPosition() → std::pair<int32_t, OffsetF>（焦点节点 id + 组件中心坐标）
 *
 * 不职责：
 * - 不做布局坐标计算（由调用方 Dialog/Sheet 各自处理）
 * - 不感知 DialogAlignment / sheetOffsetX_ 等调用方私有概念
 */
class ACE_FORCE_EXPORT HandednessManager : public virtual AceType {
    DECLARE_ACE_TYPE(HandednessManager, AceType);

public:
    explicit HandednessManager(const WeakPtr<PipelineContext>& context);
    ~HandednessManager() override = default;

    // ========== 状态写入（由 EventManager / 触摸事件链调用） ==========

    /// 记录操作手（来自 MMI 3601，TouchEvent.operatingHand）
    void SetOperatingHand(int32_t operatingHand);

    /// 记录最后焦点节点 id（触摸 DOWN 时刻 hit-test leaf node）
    void SetLastFocusNodeId(int32_t nodeId);

    /// 设置参考组件节点 id（调用方传入，供 QueryAnchorPosition(id) 使用）
    void SetReferenceNodeId(int32_t nodeId);

    // ========== 状态读取（调试/查询用） ==========

    int32_t GetOperatingHand() const { return operatingHand_; }
    int32_t GetLastFocusNodeId() const { return lastFocusNodeId_; }
    int32_t GetReferenceNodeId() const { return referenceNodeId_; }

    /// 同步查询握持手姿态（MSDP 3605）：0/1/2/3/16；MSDP 不可用返回 0
    int32_t QueryHoldHandPosture();

    /**
     * @brief 判断当前窗口断点是否满足生效条件。
     *
     * 宽度断点 >= MD (WidthBreakpoint::WIDTH_MD) 且
     * 高度断点 >= MD (HeightBreakpoint::HEIGHT_MD) 时返回 true。
     *
     * @return 满足条件返回 true；窗口不可用或断点不足时返回 false
     */
    bool IsBreakpointMet();

    // ========== 查询接口（调用方使用） ==========

    /**
     * @brief FOLLOW_HOLDING_HAND 模式：查询弹窗应放置的侧别。
     *
     * 实现：弹窗弹出时同步调 Msdp::MotionClient::GetMotionData(3605)，
     *      读 MotionEvent.status 获取握持手，结合 operatingHand_ 判定侧别。
     *
     * 判定规则：
     * - 握持手=1 (左手握持)：操作手=1 (左手) → LEFT；否则 → CENTER
     * - 握持手=2 (右手握持)：操作手=2 (右手) → RIGHT；否则 → CENTER
     * - 握持手=3 (双手握持) → 跟随操作手侧
     *     - 操作手=1 → LEFT
     *     - 操作手=2 → RIGHT
     *     - 操作手=0 → CENTER
     * - 握持手=0/16 (其他) → CENTER
     *
     * @return PopupSide；MSDP 不可用或查询失败时返回 CENTER（降级）
     */
    PopupSide QueryPopupSideByHoldingHand();

    /**
     * @brief FOLLOW_ANCHOR 模式：查询锚点节点 id 和坐标（组件中心）。
     *
     * 锚点 = 最后一次触摸 DOWN 时刻记录的焦点组件（lastFocusNodeId_）的
     *        frameRect 中心在 window 坐标系下的位置。
     *
     * @return { nodeId, offset }；无有效焦点节点时 nodeId 为 -1、offset 为 (0, 0)
     */
    std::pair<int32_t, OffsetF> QueryAnchorPosition();

    /**
     * @brief FOLLOW_ANCHOR 模式：查询指定节点的锚点坐标（组件中心）。
     *
     * @param nodeId 参考组件节点 id；传 -1 时走 QueryAnchorPosition() 原有逻辑
     *               （使用 lastFocusNodeId_）。
     * @return { nodeId, offset }；无有效节点时 nodeId 为 -1、offset 为 (0, 0)
     */
    std::pair<int32_t, OffsetF> QueryAnchorPosition(int32_t nodeId);

private:
    /// 同步查询 MSDP 3605 握持手姿态，返回 status 值（0/1/2/3/16）；查询失败返回 0
    int32_t QueryHoldHandPostureFromMsdp();

    WeakPtr<PipelineContext> context_;
    int32_t operatingHand_ = 0;
    int32_t lastFocusNodeId_ = -1;
    int32_t referenceNodeId_ = -1;
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_HANDEDNESS_HANDEDNESS_MANAGER_H
