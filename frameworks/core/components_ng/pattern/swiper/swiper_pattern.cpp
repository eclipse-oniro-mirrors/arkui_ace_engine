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

#include "core/components_ng/pattern/swiper/swiper_pattern.h"

#include "base/geometry/axis.h"
#include "base/utils/utils.h"
#include "core/components/scroll/scrollable.h"
#include "core/components_ng/pattern/swiper/swiper_layout_algorithm.h"
#include "core/components_ng/pattern/swiper/swiper_layout_property.h"
#include "core/components_ng/property/property.h"

namespace OHOS::Ace::NG {

void SwiperPattern::OnAttachToFrameNode()
{
    auto host = frameNode_.Upgrade();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToFrame(true);
}

void SwiperPattern::OnModifyDone()
{
    // auto host = frameNode_.Upgrade();
    // CHECK_NULL_VOID(host);
    // auto listLayoutProperty = host->GetLayoutProperty<ListLayoutProperty>();
    // CHECK_NULL_VOID(listLayoutProperty);
    // auto task = [weak = WeakClaim(this)](double offset, int32_t source) {
    //     if (source != SCROLL_FROM_START) {
    //         auto pattern = weak.Upgrade();
    //         if (pattern) {
    //             pattern->UpdateCurrentOffset(static_cast<float>(offset));
    //         }
    //     }
    //     return true;
    // };

    // auto hub = host->GetEventHub<EventHub>();
    // CHECK_NULL_VOID(hub);
    // auto gestureHub = hub->GetOrCreateGestureEventHub();
    // CHECK_NULL_VOID(gestureHub);
    // if (scrollableEvent_) {
    //     gestureHub->RemoveScrollableEvent(scrollableEvent_);
    // }
    // scrollableEvent_ = MakeRefPtr<ScrollableEvent>(listLayoutProperty->GetListDirection().value_or(Axis::VERTICAL));
    // scrollableEvent_->SetScrollPositionCallback(std::move(task));
    // gestureHub->AddScrollableEvent(scrollableEvent_);
}

bool SwiperPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, bool skipMeasure, bool skipLayout)
{
    if (skipMeasure && skipLayout) {
        return false;
    }
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto swiperLayoutAlgorithm = DynamicCast<SwiperLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(swiperLayoutAlgorithm, false);
    startIndex_ = swiperLayoutAlgorithm->GetStartIndex();
    endIndex_ = swiperLayoutAlgorithm->GetEndIndex();
    return false;
}

void SwiperPattern::UpdateCurrentOffset(float offset)
{
    // currentOffset_ = currentOffset_ - offset;
    // auto host = frameNode_.Upgrade();
    // CHECK_NULL_VOID(host);
    // host->MarkDirtyNode(PROPERTY_REQUEST_NEW_CHILD_NODE);
}

} // namespace OHOS::Ace::NG
