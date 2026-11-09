/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_LAYOUT_SAFE_AREA_INSET_WRAPPER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_LAYOUT_SAFE_AREA_INSET_WRAPPER_H

namespace OHOS::Ace::NG {

class LayoutWrapper;

// safeAreaInset container measure/layout pass, defined in layout_wrapper.cpp and called
// only from FrameNode::MeasureSafeAreaInset/LayoutSafeAreaInset. Declared here instead
// of layout_wrapper.h so the widely included wrapper header carries no safeAreaInset API.
void MeasureSafeAreaInsetContainer(LayoutWrapper* self, LayoutWrapper* insetContainer);
void LayoutSafeAreaInsetContainer(LayoutWrapper* self, LayoutWrapper* insetContainer);

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_LAYOUT_SAFE_AREA_INSET_WRAPPER_H
