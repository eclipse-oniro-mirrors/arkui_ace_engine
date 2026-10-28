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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_INTERFACES_NATIVE_ANI_PLACEHOLDER_ANI_MODIFIER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_INTERFACES_NATIVE_ANI_PLACEHOLDER_ANI_MODIFIER_H

#include "core/interfaces/ani/ani_api.h"

namespace OHOS::Ace::NG {

// Engine face of the ArkTS placeholder registry: template admission
// (isolated execution, restricted-scope probe, clone check), the product
// hand-off slot, and the restricted-builder scope counter. The ANI library
// binds the ArkTS native methods itself and reaches these entries through
// GetArkUIAniModifiers(), like every other static ANI API.
const ArkUIAniPlaceholderRegisterModifier* GetPlaceholderRegisterAniModifier();

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_INTERFACES_NATIVE_ANI_PLACEHOLDER_ANI_MODIFIER_H
