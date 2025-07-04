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
#ifndef FOUNDATION_ARKUI_ACE_ENGINE_FRAMEWORKS_CORE_INTERFACES_ARKOALA_IMPL_SWIPER_CONTROLLER_PEER_IMPL_H
#define FOUNDATION_ARKUI_ACE_ENGINE_FRAMEWORKS_CORE_INTERFACES_ARKOALA_IMPL_SWIPER_CONTROLLER_PEER_IMPL_H

#include <optional>
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components/swiper/swiper_controller.h"

namespace OHOS::Ace::NG::GeneratedModifier {
class SwiperControllerPeerImpl : public Referenced {
public:
    SwiperControllerPeerImpl() = default;

    void AddTargetController(const WeakPtr<SwiperController> &handler)
    {
        CHECK_NULL_VOID(!handler.Invalid());
        handlers_.push_back(handler);
    }

    void TriggerShowNext()
    {
        for (auto &handler: handlers_) {
            if (auto controller = handler.Upgrade(); controller) {
                controller->ShowNext();
            }
        }
    }

    void TriggerShowPrevious()
    {
        for (auto &handler: handlers_) {
            if (auto controller = handler.Upgrade(); controller) {
                controller->ShowPrevious();
            }
        }
    }

    void TriggerChangeIndex(int32_t index, SwiperAnimationMode mode)
    {
        index = index < 0 ? 0 : index;
        for (auto &handler: handlers_) {
            if (auto controller = handler.Upgrade(); controller) {
                controller->ChangeIndex(index, mode);
            }
        }
    }

    void SetFinishCallback(const CommonFunc &callbackFunc)
    {
        for (auto &handler: handlers_) {
            if (auto controller = handler.Upgrade(); controller) {
                controller->SetFinishCallback(callbackFunc);
            }
        }
    }

    void TriggerFinishAnimation()
    {
        for (auto &handler: handlers_) {
            if (auto controller = handler.Upgrade(); controller) {
                controller->FinishAnimation();
            }
        }
    }

    void TriggerChangeIndex(int32_t index, const SwiperAnimationMode& animationMode)
    {
        index = index < 0 ? 0 : index;
        for (auto &handler: handlers_) {
            if (auto controller = handler.Upgrade(); controller) {
                controller->ChangeIndex(index, animationMode);
            }
        }
    }

    void TriggerPreloadItems(const std::set<int32_t>& indexSet) const
    {
        for (auto &handler: handlers_) {
            if (auto controller = handler.Upgrade(); controller) {
                controller->PreloadItems(indexSet);
            }
        }
    }

    void TriggerSetPreloadFinishCallback(PreloadItemsFinishFunc&& preloadFinishCallback) const
    {
        for (auto &handler: handlers_) {
            if (auto controller = handler.Upgrade(); controller) {
                controller->SetPreloadFinishCallback(preloadFinishCallback);
            }
        }
    }
private:
    std::vector<Ace::WeakPtr<SwiperController>> handlers_;
};
} // namespace OHOS::Ace::NG::GeneratedModifier

struct SwiperControllerPeer : public OHOS::Ace::NG::GeneratedModifier::SwiperControllerPeerImpl {};
#endif //FOUNDATION_ARKUI_ACE_ENGINE_FRAMEWORKS_CORE_INTERFACES_ARKOALA_IMPL_SWIPER_CONTROLLER_PEER_IMPL_H
