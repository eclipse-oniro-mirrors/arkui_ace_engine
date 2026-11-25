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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WEB_WEB_HANDEDNESS_ANCHOR_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WEB_WEB_HANDEDNESS_ANCHOR_H

#include <optional>
#include <utility>

#include "base/geometry/ng/offset_t.h"
#include "base/memory/ace_type.h"

namespace OHOS::Ace::NG {

class Pattern;

/**
 * @brief Web 链路：查询 Web 组件上报的页面内聚焦元素锚点。
 *
 * @param pattern 待查询的组件 Pattern
 * @param nodeId Web 组件节点 id
 * @return Web 组件时返回锚点（区域无效时 nodeId 为 -1）；非 Web 组件返回 nullopt
 */
std::optional<std::pair<int32_t, OffsetF>> QueryWebHandednessAnchor(
    const RefPtr<Pattern>& pattern, int32_t nodeId);

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WEB_WEB_HANDEDNESS_ANCHOR_H
