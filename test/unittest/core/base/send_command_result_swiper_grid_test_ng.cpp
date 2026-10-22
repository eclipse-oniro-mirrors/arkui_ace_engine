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


#include "test/unittest/core/base/frame_node_test_ng.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/grid/grid_pattern.h"
#include "core/components_ng/pattern/calendar_picker/calendar_dialog_pattern.h"

namespace OHOS::Ace::NG {
RefPtr<Pattern> CreateSendCommandTabsTestPattern()
{
    return AceType::MakeRefPtr<TabsPattern>();
}

RefPtr<Pattern> CreateSendCommandSwiperTestPattern()
{
    return AceType::MakeRefPtr<SwiperPattern>();
}

RefPtr<Pattern> CreateSendCommandGridTestPattern()
{
    return AceType::MakeRefPtr<GridPattern>();
}

RefPtr<Pattern> CreateSendCommandCalendarTestPattern()
{
    return AceType::MakeRefPtr<CalendarDialogPattern>();
}
} // namespace OHOS::Ace::NG
