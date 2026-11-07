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
#include "core/components_ng/manager/handedness/handedness_types.h"

namespace OHOS::Ace::NG {
class PipelineContext;

/**
 * @brief Manages operating hand state, anchor nodes, and holding hand queries
 *        for the handedness-follow feature.
 */
class ACE_FORCE_EXPORT HandednessManager : public virtual AceType {
    DECLARE_ACE_TYPE(HandednessManager, AceType);

public:
    explicit HandednessManager(const WeakPtr<PipelineContext>& context);
    ~HandednessManager() override = default;

    void SetOperatingHand(int32_t operatingHand);

    void SetLastFocusNodeId(int32_t nodeId);

    void SetReferenceNodeId(int32_t nodeId);

    int32_t GetOperatingHand() const { return operatingHand_; }
    int32_t GetLastFocusNodeId() const { return lastFocusNodeId_; }
    int32_t GetReferenceNodeId() const { return referenceNodeId_; }

    int32_t QueryHoldHandPosture();

    bool IsBreakpointMet();

    /**
     * @brief Query which side (LEFT/CENTER/RIGHT) the popup should be placed on for
     *        FOLLOW_GRIP_HAND mode, based on holding hand posture and operating hand.
     */
    PopupSide QueryPopupSideByHoldingHand();

    static PopupSide CalcPopupSide(int32_t posture, int32_t operatingHand);

    std::pair<int32_t, OffsetF> QueryAnchorPosition();

    std::pair<int32_t, OffsetF> QueryAnchorPosition(int32_t nodeId);

private:
    int32_t QueryHoldHandPostureFromMsdp();

    WeakPtr<PipelineContext> context_;
    int32_t operatingHand_ = 0;
    int32_t lastFocusNodeId_ = -1;
    int32_t referenceNodeId_ = -1;
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_HANDEDNESS_HANDEDNESS_MANAGER_H
