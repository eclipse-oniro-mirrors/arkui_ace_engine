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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_SAFE_AREA_INSET_VIEW_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_SAFE_AREA_INSET_VIEW_H

#include <array>

#include "base/memory/referenced.h"
#include "core/components_ng/property/safe_area_inset_property.h"

namespace OHOS::Ace::NG {

class UINode;
class FrameNode;

// Element count is the SafeAreaInset edge count (TOP, END, BOTTOM, START).
using SafeAreaInsetContentArray = std::array<RefPtr<UINode>, SAFE_AREA_INSET_EDGE_COUNT>;

namespace SafeAreaInsetView {
// View-layer entry of the safeAreaInset attribute, defined in view_abstract.cpp and
// declared here instead of view_abstract.h so the widely included shared header carries
// no safeAreaInset surface. Callers: view_abstract_model_ng, the ani modifier, tests.
void SetSafeAreaInset(
    FrameNode* frameNode, const SafeAreaInsetOptions& options, const SafeAreaInsetContentArray& contents);
void ResetSafeAreaInset(FrameNode* frameNode);
} // namespace SafeAreaInsetView

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_SAFE_AREA_INSET_VIEW_H
