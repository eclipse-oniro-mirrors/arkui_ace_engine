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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_PICKER_BRIDGE_TEXTPICKER_OPTIONS_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_PICKER_BRIDGE_TEXTPICKER_OPTIONS_H

#include <cstdint>
#include <string>
#include <vector>

#include "base/geometry/dimension.h"
#include "core/components_ng/pattern/text_picker/textpicker_types.h"

namespace OHOS::Ace::NG {
struct TextPickerOptions {
    std::vector<RangeContent> range;
    uint32_t kind = 0;
    uint32_t selected = 0;
    std::string value = "";
    std::vector<TextCascadePickerOptions> options;
    std::vector<uint32_t> selecteds;
    std::vector<std::string> values;
    std::vector<Dimension> columnWidths;
    bool hasValue = false;
    bool hasSelected = false;
    bool isCascade = false;
    uint32_t maxCount = 0;
};

void ValidateSingleTextPickerOptions(TextPickerOptions& options);
void ValidateMultiTextPickerOptions(TextPickerOptions& options);
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_PICKER_BRIDGE_TEXTPICKER_OPTIONS_H
