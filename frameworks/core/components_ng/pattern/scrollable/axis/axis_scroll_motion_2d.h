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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLLABLE_AXIS_AXIS_SCROLL_MOTION_2D_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLLABLE_AXIS_AXIS_SCROLL_MOTION_2D_H

#include <functional>

#include "base/memory/ace_type.h"
#include "core/animation/motion.h"
#include "core/components_ng/pattern/scrollable/axis/axis_scroll_motion.h"

namespace OHOS::Ace::NG {

class AxisScrollMotion2D : public Motion {
    DECLARE_ACE_TYPE(AxisScrollMotion2D, Motion);

public:
    AxisScrollMotion2D(OffsetF startPos, OffsetF distance)
    {
        xMotion_ = AceType::MakeRefPtr<AxisScrollMotion>(startPos.GetX(), distance.GetX());
        yMotion_ = AceType::MakeRefPtr<AxisScrollMotion>(startPos.GetY(), distance.GetY());
        Reset(startPos, distance);
    }

    double GetCurrentPosition() override
    {
        return currentPos_.GetMainOffset(Axis::VERTICAL);
    }

    OffsetF GetCurrentPosition2D() const
    {
        return currentPos_;
    }

    OffsetF GetFinalPosition() const
    {
        return OffsetF(xMotion_->GetFinalPosition(), yMotion_->GetFinalPosition());
    }

    double GetCurrentVelocity() override
    {
        return 0.0;
    }

    void Reset(OffsetF startPos, OffsetF distance)
    {
        xMotion_->Reset(startPos.GetX(), distance.GetX());
        yMotion_->Reset(startPos.GetY(), distance.GetY());
        currentPos_ = startPos;
    }

    void ResetXAxis(float currentX, float distance)
    {
        xMotion_->Reset(currentX, distance);
    }

    void ResetYAxis(float currentY, float distance)
    {
        yMotion_->Reset(currentY, distance);
    }

    void ResetFreshStart(OffsetF startPos, OffsetF distance)
    {
        xMotion_->ResetCurrentTime();
        yMotion_->ResetCurrentTime();
        xMotion_->Reset(startPos.GetX(), distance.GetX());
        yMotion_->Reset(startPos.GetY(), distance.GetY());
        currentPos_ = startPos;
    }

    bool IsCompleted() override
    {
        return xMotion_->IsCompleted() && yMotion_->IsCompleted();
    }

    std::string GetMotionType() const override
    {
        return "axis scroll motion 2d";
    }

    void Move(float offsetTime) override
    {
        xMotion_->Move(offsetTime);
        yMotion_->Move(offsetTime);
        currentPos_ = OffsetF(xMotion_->GetCurrentPosition(), yMotion_->GetCurrentPosition());
        if (listener2D_) {
            listener2D_(currentPos_);
        }
    }

    void ResetCurrentTime()
    {
        xMotion_->ResetCurrentTime();
        yMotion_->ResetCurrentTime();
    }

    void AddListener2D(std::function<void(OffsetF)>&& listener)
    {
        listener2D_ = std::move(listener);
    }

private:
    RefPtr<AxisScrollMotion> xMotion_;
    RefPtr<AxisScrollMotion> yMotion_;
    OffsetF currentPos_;
    std::function<void(OffsetF)> listener2D_;

    ACE_DISALLOW_COPY_AND_MOVE(AxisScrollMotion2D);
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_SCROLLABLE_AXIS_AXIS_SCROLL_MOTION_2D_H
