/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WATERFLOW_WATER_FLOW_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WATERFLOW_WATER_FLOW_LAYOUT_ALGORITHM_H

#include "core/components_ng/pattern/waterflow/layout/top_down/water_flow_layout_info.h"
#include "core/components_ng/pattern/waterflow/layout/water_flow_layout_algorithm_base.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT WaterFlowLayoutAlgorithm : public WaterFlowLayoutBase {
    DECLARE_ACE_TYPE(WaterFlowLayoutAlgorithm, WaterFlowLayoutBase);

public:
    explicit WaterFlowLayoutAlgorithm(const RefPtr<WaterFlowLayoutInfo>& layoutInfo) : layoutInfo_(layoutInfo) {}
    ~WaterFlowLayoutAlgorithm() override = default;

    void Measure(LayoutWrapper* layoutWrapper) override;

    void Layout(LayoutWrapper* layoutWrapper) override;

    bool PreloadItem(LayoutWrapper* host, int32_t itemIdx, int64_t deadline) override;

    bool MeasureInNextFrame() const override
    {
        return layoutInfo_->measureInNextFrame_;
    }
private:
    FlowItemPosition GetItemPosition(int32_t index);
    void MeasureItems(LayoutWrapper* layoutWrapper);
    bool MeasureToTarget(LayoutWrapper* layoutWrapper, int32_t startFrom, std::optional<int64_t> cacheDeadline);
    void FillViewport(float mainSize, LayoutWrapper* layoutWrapper);
    void MeasureLazyLayoutItem(const RefPtr<LayoutWrapper>& item, float crossSize, float startMainPos, float mainSize,
        const RefPtr<WaterFlowLayoutProperty>& layoutProperty) const;
    // Measures one item with the constraint chosen by the column count: multiple columns
    // use the plain viewport constraint, a single column uses the lazy layout constraint.
    void MeasureItemByColumnCount(const RefPtr<LayoutWrapper>& item, float crossSize, float startMainPos,
        float mainSize, const RefPtr<WaterFlowLayoutProperty>& layoutProperty) const;
    // Sync-cache-in-layout: runs SyncLayoutCacheItems before the positioning loop when the
    // cached items are not shown and the layout is not deferred to the next frame.
    // [syncCacheLayout] is the strategy resolved once by the Layout caller.
    void SyncLayoutCacheItemsIfNeeded(LayoutWrapper* layoutWrapper, bool syncCacheLayout, int32_t cachedCount);
    // Sync-cache-in-layout: deactivates a cache-range item in the same frame.
    // [syncCacheLayout] is the strategy resolved once by the Layout caller.
    void DeactivateSyncCacheItem(const RefPtr<LayoutWrapper>& wrapper, bool isCache, bool syncCacheLayout);
    void ModifyCurrentOffsetWhenReachEnd(float mainSize, LayoutWrapper* layoutWrapper);
    float ComputeCrossPosition(int32_t crossIndex) const;
    bool GetCrossOffset(int32_t crossIndex, float crossSize, bool isRtl, float& crossOffset) const;
    void InitialItemsCrossSize(const RefPtr<WaterFlowLayoutProperty>& layoutProperty, const SizeF& frameSize,
        int32_t childrenCount, double originalWidth);
    int32_t GetChildIndexWithFooter(int32_t index) const
    {
        return layoutInfo_->NodeIdx(index);
    }
    void LayoutFooter(LayoutWrapper* layoutWrapper, const OffsetF& childFrameOffset, bool reverse);

    void SyncPreloadItem(LayoutWrapper* host, int32_t itemIdx) override;

    /**
     * @brief Sync-cache-in-layout strategy: build, measure and deactivate cache-range items
     *        synchronously inside the current-frame Layout when showCache is false.
     *
     *        Tail items without a recorded position are measured forward up to the cache
     *        bound via MeasureToTarget (cache build path: not attached to the render tree);
     *        head items with a recorded position are built and measured on demand. The main
     *        positioning loop then layouts them like any other item in the cache range.
     */
    void SyncLayoutCacheItems(LayoutWrapper* layoutWrapper, int32_t cachedCount);

    // Sync-cache-in-layout, tail half of SyncLayoutCacheItems: lay out recorded tail items
    // (LayoutRecordedTailItems), then measure the unknown tail from the last verified
    // anchor up to the cache bound (MeasureTailToCacheBound).
    void SyncTailCacheItems(LayoutWrapper* layoutWrapper, int32_t cachedCount);
    // Sync-cache-in-layout: lays out tail items up to [endBound] whose position record and
    // node both exist; returns the first tail index missing either of them.
    int32_t LayoutRecordedTailItems(LayoutWrapper* layoutWrapper, int32_t endBound);
    // Sync-cache-in-layout: measures the unknown tail from the anchor up to [endBound] via
    // MeasureToTarget on the cache build path (not attached to the render tree), then
    // re-activates the anchor when it is a visible item.
    void MeasureTailToCacheBound(LayoutWrapper* layoutWrapper, int32_t anchorIdx, int32_t endBound);
    // Sync-cache-in-layout, head half of SyncLayoutCacheItems: build, measure and lay out
    // head items whose position is already recorded, down to the cache bound.
    void SyncHeadCacheItems(LayoutWrapper* layoutWrapper, int32_t cachedCount);

    /**
     * @brief Handle end-of-content detection and adjust endIndex for zero-height trailing items
     */
    void HandleItemEnd(int32_t currentIndex);
    void HandleItemEndAndSyncReportRange(float mainSize, int32_t currentIndex);
    void KeepTrailingZeroHeightItemsInReportRange();

    std::map<int32_t, float> itemsCrossSize_;
    std::map<int32_t, float> itemsCrossPosition_;
    Axis axis_ = Axis::VERTICAL;

    RefPtr<WaterFlowLayoutInfo> layoutInfo_;

    RefPtr<WaterFlowLayoutInfoBase> LayoutInfo() const override {
        return layoutInfo_;
    }

    void ReMeasureItems(LayoutWrapper* layoutWrapper);

    float mainGap_ = 0.0f;
    float crossGap_ = 0.0f;
    float mainSize_ = 0.0f;
    float footerMainStartPos_ = 0.0f;
    bool skipMeasure_ = false;

    // Record the index range after measurement completion
    int32_t measuredStartIndex_ = -1;
    int32_t measuredEndIndex_ = -1;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_WATERFLOW_WATER_FLOW_LAYOUT_ALGORITHM_H
