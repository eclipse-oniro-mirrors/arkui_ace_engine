/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/radio/radio_accessibility_property.h"

#include "core/components_ng/pattern/radio/radio_pattern.h"

namespace OHOS::Ace::NG {
bool RadioAccessibilityProperty::IsCheckable() const
{
    return true;
}

bool RadioAccessibilityProperty::IsChecked() const
{
    auto frameNode = host_.Upgrade();
    CHECK_NULL_RETURN(frameNode, false);
    auto radioPaintProperty = frameNode->GetPaintProperty<RadioPaintProperty>();
    CHECK_NULL_RETURN(radioPaintProperty, false);
    return radioPaintProperty->GetRadioCheck().value_or(false);
}

std::string RadioAccessibilityProperty::GetText() const
{
    auto frameNode = host_.Upgrade();
    CHECK_NULL_RETURN(frameNode, "");
    auto radioEventHub = frameNode->GetOrCreateEventHub<NG::RadioEventHub>();
    auto value = radioEventHub ? radioEventHub->GetValue() : "";
    return value;
}
} // namespace OHOS::Ace::NG