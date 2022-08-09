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

#include "core/components_ng/pattern/swiper/swiper_view.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {

void SwiperView::Create()
{
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 0, AceType::MakeRefPtr<SwiperPattern>());
    ViewStackProcessor::GetInstance()->Push(frameNode);
}

void SwiperView::SetDirection(Axis axis)
{
    ACE_UPDATE_LAYOUT_PROPERTY(SwiperLayoutProperty, Direction, axis);
}

} // namespace OHOS::Ace::NG
