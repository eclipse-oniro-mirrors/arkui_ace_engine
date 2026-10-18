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

#include "core/components_ng/pattern/scrollable/axis/axis_animator.h"

#include "core/animation/animator.h"
#include "base/utils/time_util.h"
#include "core/common/container.h"
#include "core/components_ng/pattern/scrollable/axis/axis_scroll_motion.h"
#include "core/components_ng/pattern/scrollable/axis/axis_scroll_motion_2d.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {

AxisAnimator::AxisAnimator() = default;

AxisAnimator::AxisAnimator(AxisAnimationCallback&& axisAnimationCallback,
    AxisAnimationStartCallback&& axisAnimationStartCallback, AxisAnimationFinishCallback&& axisAnimationFinishCallback)
    : axisAnimationCallback_(std::move(axisAnimationCallback)),
      axisAnimationStartCallback_(std::move(axisAnimationStartCallback)),
      axisAnimationFinishCallback_(std::move(axisAnimationFinishCallback))
{
    CreateAnimator();
}

AxisAnimator::AxisAnimator(AxisAnimationCallback2D&& axisAnimationCallback2D,
    AxisAnimationStartCallback2D&& axisAnimationStartCallback2D,
    AxisAnimationFinishCallback&& axisAnimationFinishCallback)
    : axisAnimationCallback2D_(std::move(axisAnimationCallback2D)),
      axisAnimationStartCallback2D_(std::move(axisAnimationStartCallback2D)),
      axisAnimationFinishCallback_(std::move(axisAnimationFinishCallback))
{
    CreateAnimator2D();
}

AxisAnimator::~AxisAnimator() = default;

void AxisAnimator::Initialize(const WeakPtr<PipelineBase>& context)
{
    context_ = context;
}

void AxisAnimator::OnAxis(float mainDelta, float scrollablePos)
{
    CHECK_NULL_VOID(axisAnimationCallback_ && !NearZero(mainDelta));
    if (!axisScrollMotion_ || !axisScrollAnimator_) {
        CreateAnimator();
    }
    if (IsRunning()) {
        auto finalPos = axisScrollMotion_->GetFinalPosition();
        auto currentPos = axisScrollMotion_->GetCurrentPosition();
        if ((GreatNotEqual(currentPos, finalPos) && Positive(mainDelta)) ||
            (LessNotEqual(currentPos, finalPos) && !Positive(mainDelta))) {
            axisScrollAnimator_->Stop();
            return;
        }
        auto distance = finalPos + mainDelta - currentPos;
        axisScrollMotion_->Reset(currentPos, distance);
    } else {
        if (!axisScrollMotion_) {
            return;
        }
        axisScrollMotion_->Reset(scrollablePos, mainDelta);
        axisScrollAnimator_->PlayMotion(axisScrollMotion_);
        if (axisAnimationStartCallback_) {
            axisAnimationStartCallback_(scrollablePos);
        }
    }
}


void AxisAnimator::OnAxis2D(OffsetF delta, OffsetF scrollablePos)
{
    CHECK_NULL_VOID(axisAnimationCallback2D_);
    if (NearZero(delta.GetX()) && NearZero(delta.GetY())) {
        return;
    }
    if (!axisScrollMotion2D_ || !axisScrollAnimator_) {
        CreateAnimator2D();
    }
    if (IsRunning()) {
        auto finalPos = axisScrollMotion2D_->GetFinalPosition();
        auto currentPos = axisScrollMotion2D_->GetCurrentPosition2D();
        bool xReverse = false;
        bool yReverse = false;
        if (!NearZero(delta.GetX())) {
            xReverse = (GreatNotEqual(currentPos.GetX(), finalPos.GetX()) && Positive(delta.GetX())) ||
                      (LessNotEqual(currentPos.GetX(), finalPos.GetX()) && !Positive(delta.GetX()));
            if (!xReverse) {
                float newXDistance = finalPos.GetX() + delta.GetX() - currentPos.GetX();
                axisScrollMotion2D_->ResetXAxis(currentPos.GetX(), newXDistance);
            }
        }
        if (!NearZero(delta.GetY())) {
            yReverse = (GreatNotEqual(currentPos.GetY(), finalPos.GetY()) && Positive(delta.GetY())) ||
                      (LessNotEqual(currentPos.GetY(), finalPos.GetY()) && !Positive(delta.GetY()));
            if (!yReverse) {
                float newYDistance = finalPos.GetY() + delta.GetY() - currentPos.GetY();
                axisScrollMotion2D_->ResetYAxis(currentPos.GetY(), newYDistance);
            }
        }
        if (xReverse && yReverse) {
            axisScrollAnimator_->Stop();
            return;
        }
        if (xReverse) {
            axisScrollMotion2D_->ResetXAxis(currentPos.GetX(), 0.0f);
        }
        if (yReverse) {
            axisScrollMotion2D_->ResetYAxis(currentPos.GetY(), 0.0f);
        }
    } else {
        if (!axisScrollMotion2D_) {
            return;
        }
        axisScrollMotion2D_->ResetFreshStart(scrollablePos, delta);
        axisScrollAnimator_->PlayMotion(axisScrollMotion2D_);
        if (axisAnimationStartCallback2D_) {
            axisAnimationStartCallback2D_(scrollablePos);
        }
    }
}

void AxisAnimator::CreateAnimator2D()
{
    if (!axisScrollMotion2D_) {
        axisScrollMotion2D_ = AceType::MakeRefPtr<AxisScrollMotion2D>(OffsetF(0.f, 0.f), OffsetF(0.f, 0.f));
        axisScrollMotion2D_->AddListener2D([weak = AceType::WeakClaim(this)](OffsetF offset) {
            auto axisAnimator = weak.Upgrade();
            CHECK_NULL_VOID(axisAnimator && axisAnimator->axisAnimationCallback2D_);
            axisAnimator->axisAnimationCallback2D_(offset);
        });
    }
    if (!axisScrollAnimator_) {
        auto context = context_.Upgrade();
        CHECK_NULL_VOID(context);
        axisScrollAnimator_ = CREATE_ANIMATOR(context);
        axisScrollAnimator_->AddStopListener([weak = AceType::WeakClaim(this)]() {
            auto axisAnimator = weak.Upgrade();
            CHECK_NULL_VOID(axisAnimator);
            CHECK_NULL_VOID(axisAnimator->axisAnimationFinishCallback_);
            axisAnimator->axisAnimationFinishCallback_();
            auto axisScrollMotion2D = axisAnimator->GetAxisScrollMotion2D();
            CHECK_NULL_VOID(axisScrollMotion2D);
            axisScrollMotion2D->ResetCurrentTime();
        });
    }
}

void AxisAnimator::CreateAnimator()
{
    if (!axisScrollMotion_) {
        axisScrollMotion_ = AceType::MakeRefPtr<AxisScrollMotion>(0.f, 0.f);
        axisScrollMotion_->AddListener([weak = AceType::WeakClaim(this)](double offset) {
            auto axisAnimator = weak.Upgrade();
            CHECK_NULL_VOID(axisAnimator && axisAnimator->axisAnimationCallback_);
            axisAnimator->axisAnimationCallback_(offset);
        });
    }
    if (!axisScrollAnimator_) {
        auto context = context_.Upgrade();
        CHECK_NULL_VOID(context);
        axisScrollAnimator_ = CREATE_ANIMATOR(context);
        axisScrollAnimator_->AddStopListener([weak = AceType::WeakClaim(this)]() {
            auto axisAnimator = weak.Upgrade();
            CHECK_NULL_VOID(axisAnimator);
            CHECK_NULL_VOID(axisAnimator->axisAnimationFinishCallback_);
            axisAnimator->axisAnimationFinishCallback_();
            auto axisScrollMotion = axisAnimator->GetAxisScrollMotion();
            CHECK_NULL_VOID(axisScrollMotion);
            axisScrollMotion->ResetCurrentTime();
        });
    }
}

bool AxisAnimator::IsRunning()
{
    return axisScrollAnimator_ && axisScrollAnimator_->IsRunning();
}

RefPtr<AxisScrollMotion> AxisAnimator::GetAxisScrollMotion() const
{
    return axisScrollMotion_;
}

RefPtr<AxisScrollMotion2D> AxisAnimator::GetAxisScrollMotion2D() const
{
    return axisScrollMotion2D_;
}

void AxisAnimator::StopAxisAnimation()
{
    if (IsRunning()) {
        axisScrollAnimator_->Stop();
    }
}

} // namespace OHOS::Ace::NG