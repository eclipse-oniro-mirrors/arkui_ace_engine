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

#include "core/components_ng/pattern/particle/particle_pattern.h"

namespace OHOS::Ace::NG {

void ParticlePattern::OnVisibleChange(bool isVisible)
{
    if (HaveUnVisibleParent() == !isVisible) {
        return;
    }
    SetHaveUnVisibleParent(!isVisible);
    if (isVisible) {
        auto host = GetHost();
        auto context = host->GetRenderContext();
        context->OnParticleOptionArrayUpdate(context->GetParticleOptionArray().value());
    }
}

void ParticlePattern::OnAttachToMainTree()
{
    auto host = GetHost();
    auto parent = host->GetParent();
    while (parent) {
        if (InstanceOf<FrameNode>(parent)) {
            auto frameNode = DynamicCast<FrameNode>(parent);
            if (!frameNode->IsVisible()) {
                SetHaveUnVisibleParent(true);
                return;
            }
        }
        parent = parent->GetParent();
    }
}
} // namespace OHOS::Ace::NG