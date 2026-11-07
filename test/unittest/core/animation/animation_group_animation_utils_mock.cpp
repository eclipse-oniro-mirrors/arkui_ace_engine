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

#include <memory>
#include <unordered_map>

#include "core/components_ng/render/animation_utils.h"

namespace OHOS::Ace {
namespace {
std::unordered_map<AnimationUtils::InteractiveAnimation*, InteractiveAnimationStatus>& GetStatusMap()
{
    static std::unordered_map<AnimationUtils::InteractiveAnimation*, InteractiveAnimationStatus> statusMap;
    return statusMap;
}
} // namespace

namespace AnimationGroupTestHelper {

void SetInteractiveAnimationStatus(
    const std::shared_ptr<AnimationUtils::InteractiveAnimation>& animation, InteractiveAnimationStatus status)
{
    if (!animation) {
        return;
    }
    GetStatusMap()[animation.get()] = status;
}

void ClearInteractiveAnimationStatus(
    const std::shared_ptr<AnimationUtils::InteractiveAnimation>& animation)
{
    if (!animation) {
        return;
    }
    GetStatusMap().erase(animation.get());
}

void ClearAllInteractiveAnimationStatus()
{
    GetStatusMap().clear();
}

} // namespace AnimationGroupTestHelper

void AnimationUtils::FinishInteractiveAnimation(
    const std::shared_ptr<AnimationUtils::InteractiveAnimation>& interactiveAnimation,
    InteractiveAnimationFinishPosition position)
{
    (void)position;
    if (interactiveAnimation) {
        GetStatusMap()[interactiveAnimation.get()] = InteractiveAnimationStatus::INACTIVE;
    }
}

void AnimationUtils::PauseInteractiveAnimation(
    const std::shared_ptr<AnimationUtils::InteractiveAnimation>& interactiveAnimation)
{
    if (interactiveAnimation) {
        GetStatusMap()[interactiveAnimation.get()] = InteractiveAnimationStatus::PAUSED;
    }
}

InteractiveAnimationStatus AnimationUtils::GetInteractiveAnimationStatus(
    const std::shared_ptr<AnimationUtils::InteractiveAnimation>& interactiveAnimation)
{
    if (!interactiveAnimation) {
        return InteractiveAnimationStatus::INACTIVE;
    }
    auto it = GetStatusMap().find(interactiveAnimation.get());
    if (it == GetStatusMap().end()) {
        return InteractiveAnimationStatus::RUNNING;
    }
    return it->second;
}
} // namespace OHOS::Ace
