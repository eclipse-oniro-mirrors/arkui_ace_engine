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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PROPERTIES_FLEX_PROPERTIES_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PROPERTIES_FLEX_PROPERTIES_H

#include <memory>

#include "core/components/common/layout/constants.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_ng/property/property.h"

namespace OHOS::Ace::NG {

struct FlexItemProperty {
    ACE_DEFINE_STRUCT_PROPERTY(FlexGrow, int32_t);
    ACE_DEFINE_STRUCT_PROPERTY(FlexShrink, int32_t);
    ACE_DEFINE_STRUCT_PROPERTY(AlignSelf, FlexAlign);
    ACE_DEFINE_STRUCT_PROPERTY(FlexBasis, CalcLength);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PROPERTIES_FLEX_PROPERTIES_H