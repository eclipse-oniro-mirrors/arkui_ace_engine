/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WATERFLOW_WATER_FLOW_SECTIONS_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WATERFLOW_WATER_FLOW_SECTIONS_H

#include <cstdint>
#include <functional>
#include <optional>

#include "base/geometry/dimension.h"
#include "core/components_ng/property/measure_property.h"

namespace OHOS::Ace::NG {
using GetItemMainSizeByIndex = std::function<float(int32_t)>;

class WaterFlowSections : public virtual AceType {
    DECLARE_ACE_TYPE(WaterFlowSections, AceType)
public:
    struct Section {
        bool operator==(const Section& other) const
        {
            return itemsCount == other.itemsCount && crossCount == other.crossCount && columnsGap == other.columnsGap &&
                   rowsGap == other.rowsGap && margin == other.margin;
        }
        bool operator!=(const Section& other) const
        {
            return !(*this == other);
        }

        bool OnlyCountChange(const Section& other) const
        {
            return crossCount == other.crossCount && columnsGap == other.columnsGap && rowsGap == other.rowsGap &&
                   margin == other.margin;
        }

        int32_t itemsCount = 0;
        std::optional<int32_t> crossCount;
        GetItemMainSizeByIndex onGetItemMainSizeByIndex;
        std::optional<Dimension> columnsGap;
        std::optional<Dimension> rowsGap;
        std::optional<MarginProperty> margin;
    };

    WaterFlowSections() = default;
    ~WaterFlowSections() override = default;
    void SetOnDataChange(std::function<void(int32_t start)>&& func)
    {
        onSectionDataChange_ = func;
    }
    void SetOnDataChangeNow(std::function<void(int32_t start)>&& func)
    {
        onSectionDataChangeNow_ = func;
    }
    void ChangeData(int32_t start, int32_t deleteCount, const std::vector<Section>& newSections);
    void ChangeDataNow(int32_t start, int32_t deleteCount, const std::vector<Section>& newSections);
    const std::vector<Section>& GetSectionInfo() const
    {
        return sections_;
    }

    /**
     * @brief check if last update was a special case where only itemCount in the last section is modified.
     *
     * @return true only if itemCount in the modified section has changed and everything else remains the same.
     */
    bool IsSpecialUpdate() const;
    bool IsSpecialUpdateCAPI(int32_t updateIndex) const;
private:
    // {first changed section, index of that section}
    // for comparing and handling special update case
    std::optional<std::pair<Section, int32_t>> prevSection_;
    std::vector<Section> sections_;
    // for comparing and handling special case
    std::vector<Section> prevSections_;
    std::function<void(int32_t start)> onSectionDataChange_;
    std::function<void(int32_t start)> onSectionDataChangeNow_;
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WATERFLOW_WATER_FLOW_SECTIONS_H
