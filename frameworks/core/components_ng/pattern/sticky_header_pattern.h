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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_STICKY_HEADER_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_STICKY_HEADER_PATTERN_H

#include <optional>

#include "base/memory/ace_type.h"

namespace OHOS::Ace::NG {

// Geometry-only section handoff; implementing this interface does not grant lazy layout eligibility.
class StickyHeaderPattern : public virtual AceType {
    DECLARE_ACE_TYPE(StickyHeaderPattern, AceType);

public:
    virtual bool HasStickyHeader() const = 0;
    virtual void PublishNextStickyHeaderGap(const std::optional<float>& gap) = 0;
    virtual const std::optional<float>& GetNextStickyHeaderGap() const = 0;
    // Existing lazy sections publish during measurement; ordinary sections may also need layout-only refreshes.
    virtual bool RequiresStickyHandoffInLayout() const
    {
        return false;
    }
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_STICKY_HEADER_PATTERN_H
