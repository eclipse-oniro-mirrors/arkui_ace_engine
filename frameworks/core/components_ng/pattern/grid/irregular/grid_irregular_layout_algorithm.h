/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_GRID_IRREGULAR_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_GRID_IRREGULAR_LAYOUT_ALGORITHM_H

#include "base/utils/noncopyable.h"
#include "core/components/common/properties/alignment.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/grid/grid_layout_base_algorithm.h"
#include "core/components_ng/pattern/grid/grid_layout_info.h"

/**
 * @brief GridIrregularLayout class supports irregular grid items that take multiple rows and multiple columns.
 *
 * INVARIANT: gridMatrix_ is always filled from the first row up to endIndex_ at the beginning of each layout.
 * INVARIANT: startMainLineIndex_ always corresponds to where Item [startIndex_] is placed.
 * But endMainLineIndex_ corresponds to the last line in viewport.
 */
namespace OHOS::Ace::NG {
class GridLayoutProperty;

class GridIrregularLayoutAlgorithm : public GridLayoutBaseAlgorithm {
    DECLARE_ACE_TYPE(GridIrregularLayoutAlgorithm, GridLayoutBaseAlgorithm);

public:
    explicit GridIrregularLayoutAlgorithm(
        GridLayoutInfo info, bool canOverScrollStart = false, bool canOverScrollEnd = false)
        : GridLayoutBaseAlgorithm(std::move(info)), canOverScrollStart_(canOverScrollStart),
          canOverScrollEnd_(canOverScrollEnd) {};

    ~GridIrregularLayoutAlgorithm() override = default;

    void Measure(LayoutWrapper* layoutWrapper) override;

    void Layout(LayoutWrapper* layoutWrapper) override;

    // Apply the prepared target during the relocation layout batch.
    void SetFastScrollTargetForLayout(const GridFastScrollTarget& target);

    void SetEnableSkip(bool value)
    {
        enableSkip_ = value;
    }

private:
    void PrepareFastScroll(float mainSize);
    std::optional<RectF> GetFastScrollTargetRect(int32_t targetIndex);

    /**
     * @brief Measures the size of Grid based on the given GridLayoutProperty.
     * @param props The GridLayoutProperty object containing the layout properties.
     * @return The main-axis length of Grid contentRect.
     */
    float MeasureSelf(const RefPtr<GridLayoutProperty>& props);

    /**
     * @brief Initializes member variables based on the given GridLayoutProperty.
     * @param props The GridLayoutProperty object containing the layout properties.
     */
    void Init(const RefPtr<GridLayoutProperty>& props);

    void MeasureOnOffset(float mainSize);
    void MeasureForward(float mainSize);
    void MeasureBackward(float mainSize, bool toAdjust = false);

    /**
     * @brief Measure lines above the start line that are visible in the start contentClip
     * extension area (top padding / safe area / custom clip region).
     *
     * Used after jump/skip paths that don't fill backward: those paths leave the start
     * extension area without measured lineHeightMap_ entries, so items scrolled into it
     * would not participate in layout every frame. startFixOffset_ == 0 (no extension)
     * is a no-op, preserving the original behavior.
     */
    void MeasureBackwardForStartExtension();

    /**
     * @brief Check if offset is larger than the entire viewport. If so, skip measuring intermediate items and jump
     * directly to the estimated destination.
     *
     * @param mainSize main-axis length of the viewport.
     * @return true if a skip is performed.
     */
    bool TrySkipping(float mainSize);

    /**
     * @brief Measure all items until targetIndex_ is reached. For performing scrollTo with animation.
     *
     */
    void MeasureToTarget();

    /**
     * @brief Check if layout states (matrix, height map) need to be reset during Init.
     */
    void CheckForReset();

    /**
     * @brief Per-tile geometry shared by one LayoutChildren pass: tile alignment,
     *        cross positions, padding-free frame size for the RTL flip, left padding.
     */
    struct TileLayoutContext {
        Alignment align;
        std::vector<float> crossPos;
        SizeF frameSize;
        bool isRtl = false;
        float paddingOffset = 0.0f;
    };

    /**
     * @brief Inclusive item-index window; items outside it are skipped during layout.
     */
    struct ItemRange {
        int32_t head = 0;
        int32_t tail = -1;
    };

    /**
     * @brief Performs the layout of the children based on the main offset.
     * @param mainOffset The main offset of the layout.
     * @param cacheLine number of lines of cache items to layout
     * @param syncCacheLayout whether the sync-cache-in-layout strategy is enabled.
     * @param syncCacheLayoutChild whether a sync-cache-layout child is attached
     *        (GridPattern::IsSyncCacheLayoutEnabled); gates the index-window adjustments
     *        so they stay inert for grids without the feature.
     * @return number of cached items laid out in front and back
     */
    std::pair<int32_t, int32_t> LayoutChildren(
        float mainOffset, int32_t cacheLine, bool syncCacheLayout, bool syncCacheLayoutChild);

    /**
     * @brief Resolves the item-index window of the layout walk.
     * @param cacheLine number of lines of cache items to layout.
     * @param syncCacheLayoutChild whether a sync-cache-layout child is attached.
     * @return [head, tail] bounds; [0, childrenCount - 1] without the feature.
     */
    ItemRange GetItemRange(int32_t cacheLine, bool syncCacheLayoutChild) const;

    /**
     * @brief Lays out every item of matrix line [lineIdx] at main-axis position [mainPos].
     */
    void LayoutMatrixLine(int32_t lineIdx, const std::map<int32_t, int32_t>& row, float lineHeight, bool isCache,
        bool syncCacheLayout, bool syncCacheLayoutChild, const ItemRange& range, const TileLayoutContext& ctx,
        float mainPos, std::pair<int32_t, int32_t>& cacheCnt);

    /**
     * @brief Counts [itemIdx] into the head or tail cache count of the layout pass.
     */
    void CountCacheItem(int32_t itemIdx, int32_t lineIdx, bool syncCacheLayoutChild,
        std::pair<int32_t, int32_t>& cacheCnt) const;

    /**
     * @brief Update variables in GridLayoutInfo at the end of Layout.
     */
    void UpdateLayoutInfo();

    /**
     * @brief Calculates the cross positions based on the padding.
     * @param padding The padding property of the layout.
     * @return A vector containing the cross positions.
     */
    std::vector<float> CalculateCrossPositions(const PaddingPropertyF& padding);

    // ========================================== MeasureOnJump functions =====================================

    std::optional<float> GetFastScrollAlignmentCorrection(float mainSize, int32_t target, ScrollAlign requestedAlign);
    void MeasureOnJump(float mainSize);
    void Jump(float mainSize, bool considerContentOffset = false);

    /**
     * @brief Find the line the jumpIdx item resides in. If not in matrix, fill the matrix up to [jumpIdx].
     *
     * @param jumpIdx The GridItem index to jump to.
     * @return The line index of the item in GridMatrix.
     */
    int32_t FindJumpLineIdx(int32_t jumpIdx);

    /**
     * @brief Prepares GridLayoutInfo::lineHeightMap_ using GridIrregularFiller.
     *
     * If the algorithm identifies that mainSize can't be filled with the current scrollAlign_ and jumpLineIdx, these
     * params will be adjusted. For instance, jumping to the last line with ScrollAlign::START isn't possible.
     *
     * @param mainSize The main-axis length of the grid.
     * @param jumpLineIdx The line index to jump to, can be adjusted during the function call.
     */
    void PrepareLineHeight(float mainSize, int32_t& jumpLineIdx);
    // ========================================== MeasureOnJump ends ===========================================

    /**
     * @brief Skip forward by currentOffset_ and fill the matrix along the way.
     *
     * @return item index to jump to after skipping.
     */
    int32_t SkipLinesForward();

    /**
     * @brief Skip backward by currentOffset_. Can assume that the matrix is already filled up to startIdx_
     *
     * @return item index to jump to after skipping.
     */
    int32_t SkipLinesBackward() const;

    bool IsIrregularLine(int32_t lineIndex) const override;

    /**
     * @brief post delayed task to preload GridItems in cache range.
     */
    void PreloadItems(int32_t cacheCnt);
    /**
     * @brief immediately create & measure GridItems in cache range.
     */
    void SyncPreloadItems(int32_t cacheCnt);
    /**
     * @brief Sync-cache-in-layout strategy: build, measure and deactivate cache-range
     *        GridItems synchronously inside the current-frame Layout when showCache is
     *        false, instead of posting the delayed preload task. Gated by
     *        GridLayoutBaseAlgorithm::IsSyncCacheLayoutEnabled.
     */
    void SyncLayoutCacheItems(int32_t cacheCnt);
    /**
     * @brief Sync-cache-in-layout: deactivates a cache-range item in the same frame.
     *        [syncCacheLayout] is the strategy resolved once by the Layout caller.
     */
    void DeactivateSyncCacheItem(const RefPtr<LayoutWrapper>& child, bool isCache, bool syncCacheLayout);

    /**
     * @brief Returns the child of the tile's top-left item, or nullptr when [itemIdx] is
     *        not a top-left tile of the matrix or the child is not built.
     */
    RefPtr<LayoutWrapper> GetTileChild(int32_t itemIdx, int32_t mainLine, int32_t col, bool isCache);

    /**
     * @brief Positions one child tile inside its block (cross offset, main position,
     *        alignment, RTL flip, padding) and lays it out, or only syncs its geometry
     *        for clean cache items, then checks the new-op-inc node.
     */
    void LayoutChildAtTile(const RefPtr<LayoutWrapper>& child, float crossOffset, float mainPos,
        const OffsetF& alignPos, const SizeF& childSize, const SizeF& frameSize, bool isRtl, float paddingOffset,
        bool isCache, bool syncCacheLayout);

    void AdaptToChildMainSize(RefPtr<GridLayoutProperty>& gridLayoutProperty, float mainSize, SizeF idealSize);

    LayoutWrapper* wrapper_ = nullptr;

    std::vector<float> crossLens_; /**< The column widths of the GridItems. */
    float crossGap_ = 0.0f;        /**< The cross-axis gap between GridItems. */
    float mainGap_ = 0.0f;         /**< The main-axis gap between GridItems. */

    float postJumpOffset_ = 0.0f; /**< The offset to be applied after performing a jump. */
    float overscrollOffsetBeforeJump_ = 0.0f;

    bool fastScrollLayout_ = false;
    bool fastScrollAutoAlign_ = false;
    bool fastScrollPreparationOnly_ = false;

    bool enableSkip_ = true;
    bool canOverScrollStart_ = false;
    bool canOverScrollEnd_ = false;

    SizeF frameSize_;
    double originalWidth_ = 0.0;
    ACE_DISALLOW_COPY_AND_MOVE(GridIrregularLayoutAlgorithm);
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_GRID_GRID_IRREGULAR_LAYOUT_ALGORITHM_H
