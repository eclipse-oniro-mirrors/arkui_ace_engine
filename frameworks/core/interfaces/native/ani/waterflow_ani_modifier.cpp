/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "waterflow_ani_modifier.h"

#include "base/log/log.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/waterflow/water_flow_model_static.h"
#include "core/interfaces/native/implementation/frame_node_peer_impl.h"

namespace OHOS::Ace::NG {
Dimension ConvertLengthToDimension(const ArkUIWaterFlowSectionGap &src)
{
    auto unit = static_cast<OHOS::Ace::DimensionUnit>(src.unit);
    auto value = src.value;
    if (unit == OHOS::Ace::DimensionUnit::PERCENT) {
        value /= 100.0f;
    }
    return Dimension(value, unit);
}

MarginProperty ConvertToMargin(const ArkUIWaterFlowSectionPadding& src)
{
    MarginProperty padding;
    padding.left = CalcLength(ConvertLengthToDimension(src.left));
    padding.top = CalcLength(ConvertLengthToDimension(src.top));
    padding.right = CalcLength(ConvertLengthToDimension(src.right));
    padding.bottom = CalcLength(ConvertLengthToDimension(src.bottom));
    return padding;
}

std::vector<NG::WaterFlowSections::Section> ConvertToSection(const std::vector<ArkUIWaterFlowSection> &sections)
{
    std::vector<NG::WaterFlowSections::Section> res;
    for (size_t i = 0; i < sections.size(); i++) {
        NG::WaterFlowSections::Section curSection;
        curSection.itemsCount = sections[i].itemsCount;
        curSection.crossCount = sections[i].crossCount;
        curSection.columnsGap = ConvertLengthToDimension(sections[i].columnsGap);
        curSection.rowsGap = ConvertLengthToDimension(sections[i].rowsGap);
        curSection.margin = ConvertToMargin(sections[i].margin);
        curSection.onGetItemMainSizeByIndex = sections[i].onGetItemMainSizeByIndex;
        res.emplace_back(curSection);
    }
    return res;
}

void SetWaterFlowFooterContent(ArkUINodeHandle node, ArkUINodeHandle footerPtr)
{
    auto* frameNode = reinterpret_cast<NG::FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto* footerContentPtr = reinterpret_cast<FrameNodePeer*>(footerPtr);
    CHECK_NULL_VOID(footerContentPtr);

    auto footerContentNodePtr = FrameNodePeer::GetFrameNodeByPeer(footerContentPtr);
    CHECK_NULL_VOID(footerContentNodePtr);
    NG::WaterFlowModelStatic::SetFooter(frameNode, footerContentNodePtr);
}

void SetWaterFlowSection(ArkUINodeHandle node, double start, double deleteCount, void* section, ArkUI_Int32 size)
{
    auto* frameNode = reinterpret_cast<NG::FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(section);

    std::vector<ArkUIWaterFlowSection> sections(static_cast<ArkUIWaterFlowSection*>(section),
        static_cast<ArkUIWaterFlowSection*>(section) + size);
    std::vector<NG::WaterFlowSections::Section> newSections = ConvertToSection(sections);
    auto waterFlowSections = NG::WaterFlowModelStatic::GetOrCreateWaterFlowSections(frameNode);
    CHECK_NULL_VOID(waterFlowSections);
    waterFlowSections->ChangeData(start, deleteCount, newSections);
}

const ArkUIAniWaterFlowModifier* GetArkUIAniWaterFlowModifier()
{
    static const ArkUIAniWaterFlowModifier impl = {
        .setWaterFlowSection = OHOS::Ace::NG::SetWaterFlowSection,
        .setWaterFlowFooterContent = OHOS::Ace::NG::SetWaterFlowFooterContent,
    };
    return &impl;
}
} // namespace OHOS::Ace::NG
