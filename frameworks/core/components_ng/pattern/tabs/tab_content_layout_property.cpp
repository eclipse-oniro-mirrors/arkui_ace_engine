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

#include "core/components_ng/pattern/tabs/tab_content_layout_property.h"

#include "base/json/json_util.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/tabs/tab_content_pattern.h"

namespace OHOS::Ace::NG {

void TabContentLayoutProperty::BadgeToJsonValue(
    std::unique_ptr<JsonValue>& jsonValue, const InspectorFilter& filter) const
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pattern = host->GetPattern<TabContentPattern>();
    CHECK_NULL_VOID(pattern);
    const auto& badgeParam = pattern->GetTabBarBadge();
    if (!badgeParam.has_value()) {
        return;
    }
    auto jsonBadge = JsonUtil::Create(true);
    if (badgeParam->badgeValue.has_value()) {
        jsonBadge->PutExtAttr("badgeValue", badgeParam->badgeValue.value().c_str(), filter);
    }
    if (badgeParam->badgeCount.has_value()) {
        jsonBadge->PutExtAttr("badgeCount", badgeParam->badgeCount.value(), filter);
    }
    if (badgeParam->badgeMaxCount.has_value()) {
        jsonBadge->PutExtAttr("badgeMaxCount", badgeParam->badgeMaxCount.value(), filter);
    }
    jsonValue->PutExtAttr("badge", jsonBadge->ToString().c_str(), filter);
}

} // namespace OHOS::Ace::NG
