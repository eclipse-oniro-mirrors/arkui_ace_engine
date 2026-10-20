/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LIST_LIST_POSITION_MAP_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LIST_LIST_POSITION_MAP_H

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <utility>

#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components_ng/pattern/list/list_children_main_size.h"


namespace OHOS::Ace::NG {
class FrameNode;
class LayoutWrapper;
class UINode;

struct ListPositionInfo {
    float mainPos;
    float mainSize;
    bool isGroup = false;
    bool isLazyChild = false;
};

enum class ListPosMapUpdate {
    NO_CHANGE = 0,
    UPDATE_ALL_SIZE,
    RE_CALCULATE,
};

class ACE_FORCE_EXPORT ListPositionMap : public AceType {
    DECLARE_ACE_TYPE(ListPositionMap, AceType);
public:
    struct EstimateRange {
        int32_t startIndex;
        int32_t endIndex;
    };
    using ItemGroupFlags = std::vector<std::pair<int32_t, bool>>;

    ListPositionMap() = default;
    ~ListPositionMap() override = default;

    void UpdatePos(int32_t index, ListPositionInfo posInfo)
    {
        posMap_[index] = posInfo;
    }

    void UpdatePosRange(int32_t startIndex, int32_t endIndex, ListPositionInfo posInfo, float space, int32_t lanes);

    void UpdatePosWithCheck(int32_t index, ListPositionInfo posInfo);

    void ClearPosMap()
    {
        posMap_.clear();
    }

    void MarkDirty(ListChangeFlag flag)
    {
        dirty_ = dirty_ | flag;
    }

    void ClearDirty()
    {
        dirty_ = LIST_NO_CHANGE;
    }

    float GetTotalHeight() const
    {
        return totalHeight_;
    }

    float GetPrevTotalHeight() const
    {
        return prevTotalHeight_;
    }

    ListPosMapUpdate CheckPosMapUpdateRule();

    void UpdatePosMapStart(float delta, double& listCurrentPos, float space,
        int32_t startIndex, float startPos, bool groupAtStart);

    void UpdatePosMapEnd(int32_t prevEndIndex, float space, bool groupAtEnd);

    void UpdateFastScrollPosMapStart(float delta, double& listCurrentPos, float space,
        int32_t startIndex, float startPos, bool groupAtStart);
    void UpdateFastScrollPosMapEnd(int32_t prevEndIndex, float space, bool groupAtEnd);
    // Discard FastScroll metadata without geometry before resuming legacy position updates.
    void RemoveUnpositionedEntries();

    void CalculateUINode(RefPtr<UINode> node);

    std::optional<bool> GetLazyForEachChildIsGroup(RefPtr<UINode> node);

    void CalculateLazyForEachNode(RefPtr<UINode> node);

    void CalculateFrameNode(RefPtr<FrameNode> frameNode);

    void CalculateListItemNode();

    void CalculateGroupNode();

    void PosMapRecalculate(LayoutWrapper* layoutWrapper);

    void GroupPosMapRecalculate();

    virtual void UpdatePosMap(LayoutWrapper* layoutWrapper, int32_t lanes, float space,
        RefPtr<ListChildrenMainSize>& childrenSize);

    void UpdateGroupPosMap(int32_t totalCount, int32_t lanes, float space,
        RefPtr<ListChildrenMainSize>& childrenSize, float headerSize, float footerSize);

    void UpdateTotalCount(int32_t totalCount);

    float GetPos(int32_t index, float offset = 0.0f)
    {
        return posMap_[index].mainPos - offset;
    }

    float GetGroupLayoutOffset(int32_t startIndex, float startPos)
    {
        return posMap_[startIndex].mainPos - startPos;
    }

    ListPositionInfo GetPositionInfo(int32_t index) const;

    ItemGroupFlags GetItemGroupFlags() const;
    void UpdateItemGroupFlags(const ItemGroupFlags& flags);

    // Size-only entries are standalone samples; adjacent positioned items share a row up to lanes.
    std::optional<float> GetAverageRowHeight(int32_t lanes) const;

    // Connect the requested range to a valid position; the caller compensates coordinate changes.
    bool EstimatePositions(const EstimateRange& range, float estimatedSize, int32_t lanes, float space,
        const std::map<int32_t, int32_t>& itemRanges = {}, bool stackFromEnd = false);

    // itemRanges uses layout indices; the returned row uses physical indices when stackFromEnd is true.
    std::optional<EstimateRange> GetLanesRow(int32_t index, int32_t lanes,
        const std::map<int32_t, int32_t>& itemRanges, bool stackFromEnd = false) const;

    // Returns the first item in the row and the offset from its start; gaps belong to the next row.
    std::pair<int32_t, float> FindRowAtPosition(float position, int32_t lanes) const;

    std::pair<int32_t, float> GetStartIndexAndPos() const;

    std::pair<int32_t, float> GetEndIndexAndPos() const;

    int32_t GetEntryAtOrBeforeIndex(int32_t index) const;

    int32_t GetEntryAtOrAfterIndex(int32_t index) const;

    void OptimizeBeforeMeasure(int32_t& beginIndex, float& beginPos, const float offset, const float contentSize);

    void SetChainOffsetCallback(std::function<float(int32_t)> func)
    {
        chainOffsetFunc_ = std::move(func);
    }

    int32_t GetRowStartIndex(const int32_t input);

    int32_t GetRowEndIndex(const int32_t input);

    float GetRowHeight(int32_t input);

    std::pair<int32_t, float> GetRowEndIndexAndHeight(const int32_t input);

    void ReversePosMap();
    // Create a layout-local mirrored view; the shared physical position table is not modified.
    RefPtr<ListPositionMap> CreateFastScrollMirroredPosMap() const;
protected:
    RefPtr<ListChildrenMainSize> childrenSize_;
    ListChangeFlag dirty_ = LIST_NO_CHANGE;
    int32_t totalItemCount_ = 0;
    float space_ = 0.0f;
    int32_t lanes_ = -1;

private:
    using PositionMap = std::map<int32_t, ListPositionInfo>;

    PositionMap::iterator FindEstimatedAnchor(const EstimateRange& range);
    std::optional<double> CheckEstimatedPositions(
        const EstimateRange& range, PositionMap::const_iterator anchor, float estimatedSize, float space) const;
    void FillEstimatedPositions(const EstimateRange& range, PositionMap::iterator anchor,
        double anchorPos, float estimatedSize, float space);
    bool EstimateSingleLanePositions(EstimateRange range, float estimatedSize, float space);
    struct LanesEstimate {
        int32_t lanes;
        bool stackFromEnd;
        const std::map<int32_t, int32_t>& itemRanges;
    };

    bool EstimateConfirmedRows(EstimateRange range, float estimatedSize, float space, const LanesEstimate& layout);
    std::optional<double> CollectEstimatedRows(const EstimateRange& range, int32_t anchorIndex,
        float estimatedSize, float space, const LanesEstimate& layout,
        std::vector<std::pair<int32_t, float>>& heights) const;
    void FillConfirmedRows(int32_t startIndex,
        const std::vector<std::pair<int32_t, float>>& heights, double position, float space);

    PositionMap posMap_;
    std::function<float(int32_t)> chainOffsetFunc_;
    int32_t curLine_ = 0;
    int32_t curIndex_ = 0;
    float totalHeight_ = 0.0f;
    float prevTotalHeight_ = 0.0f;
    float curRowHeight_ = 0.0f;
    float headerSize_ = 0.0f;
    float footerSize_ = 0.0f;
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_LIST_LIST_POSITION_MAP_H
