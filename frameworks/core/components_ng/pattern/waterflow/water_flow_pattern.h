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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WATERFLOW_WATER_FLOW_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WATERFLOW_WATER_FLOW_PATTERN_H

#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_ng/pattern/waterflow/water_flow_accessibility_property.h"
#include "core/components_ng/pattern/waterflow/water_flow_content_modifier.h"
#include "core/components_ng/pattern/waterflow/water_flow_event_hub.h"
#include "core/components_ng/pattern/waterflow/water_flow_layout_info.h"
#include "core/components_ng/pattern/waterflow/water_flow_layout_property.h"
#include "core/components_ng/pattern/waterflow/water_flow_sections.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT WaterFlowPattern : public ScrollablePattern {
    DECLARE_ACE_TYPE(WaterFlowPattern, ScrollablePattern);

public:
    bool UpdateCurrentOffset(float delta, int32_t source) override;
    bool IsScrollable() const override;
    bool IsAtTop() const override;
    bool IsAtBottom() const override;
    bool IsReverse() const override;
    bool hasFooter()
    {
        return footer_.Upgrade() != nullptr;
    };
    OverScrollOffset GetOverScrollOffset(double delta) const override;
    void UpdateScrollBarOffset() override;

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override;

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<WaterFlowLayoutProperty>();
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<WaterFlowEventHub>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<WaterFlowAccessibilityProperty>();
    }

    RefPtr<ScrollableController> GetPositionController() const
    {
        return positionController_;
    }

    void TriggerModifyDone();
    
    RefPtr<NodePaintMethod> CreateNodePaintMethod() override;

    bool UpdateStartIndex(int32_t index);

    void AddFooter(const RefPtr<NG::UINode>& footer);

    void ResetLayoutInfo();

    int32_t GetBeginIndex() const
    {
        return layoutInfo_.startIndex_;
    }

    int32_t GetEndIndex() const
    {
        return layoutInfo_.endIndex_;
    }

    int32_t GetChildrenCount() const
    {
        return layoutInfo_.childrenCount_;
    }

    float GetTotalOffset() const override
    {
        return -layoutInfo_.currentOffset_;
    }

    int32_t GetRows() const;

    int32_t GetColumns() const;

    void SetAccessibilityAction();

    void OnAnimateStop() override;

    void ScrollPage(bool reverse, bool smooth = false) override;

    void ScrollToIndex(int32_t index, bool smooth = false, ScrollAlign align = ScrollAlign::START,
        std::optional<float> extraOffset = std::nullopt) override;

    double GetStoredOffset() const
    {
        return layoutInfo_.storedOffset_;
    }

    void SetRestoreOffset(double restoreOffset)
    {
        layoutInfo_.restoreOffset_ = restoreOffset;
    }

    void SetScrollAlign(ScrollAlign align)
    {
        layoutInfo_.align_ = align;
    }

    std::string ProvideRestoreInfo() override;
    void OnRestoreInfo(const std::string& restoreInfo) override;
    Rect GetItemRect(int32_t index) const override;

    RefPtr<WaterFlowSections> GetSections() const
    {
        return sections_;
    }
    RefPtr<WaterFlowSections> GetOrCreateWaterFlowSections();
    void ResetSections();

    /**
     * @brief Callback function when Sections data has changed.
     *
     * @param start the index of the first modified section.
     */
    void OnSectionChanged(int32_t start);

    void OnSectionChangedNow(int32_t start);

private:
    DisplayMode GetDefaultScrollBarDisplayMode() const override
    {
        return DisplayMode::OFF;
    }
    void OnModifyDone() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    void CheckScrollable();
    bool IsOutOfBoundary(bool useCurrentDelta = true) override;
    void SetEdgeEffectCallback(const RefPtr<ScrollEdgeEffect>& scrollEffect) override;
    SizeF GetContentSize() const;
    void MarkDirtyNodeSelf();
    void OnScrollEndCallback() override;
    bool ScrollToTargetIndex(int32_t index);
    bool NeedRender();
    std::optional<int32_t> targetIndex_;
    WaterFlowLayoutInfo layoutInfo_;
    RefPtr<WaterFlowSections> sections_;

    float prevOffset_ = 0.0f;
    SizeF lastSize_;
    WeakPtr<UINode> footer_;

    // clip padding of WaterFlow
    RefPtr<WaterFlowContentModifier> contentModifier_;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WATERFLOW_WATER_FLOW_PATTERN_H
