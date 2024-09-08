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

#include "core/components_ng/pattern/grid/irregular/grid_irregular_layout_algorithm.h"

#include "base/utils/utils.h"
#include "core/components/scroll/scroll_controller_base.h"
#include "core/components_ng/pattern/grid/grid_layout_info.h"
#include "core/components_ng/pattern/grid/grid_layout_property.h"
#include "core/components_ng/pattern/grid/grid_utils.h"
#include "core/components_ng/pattern/grid/irregular/grid_irregular_filler.h"
#include "core/components_ng/pattern/grid/irregular/grid_layout_range_solver.h"
#include "core/components_ng/pattern/grid/irregular/grid_layout_utils.h"
#include "core/components_ng/pattern/scrollable/scrollable_utils.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/property/templates_parser.h"

namespace OHOS::Ace::NG {
void GridIrregularLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    if (gridLayoutInfo_.childrenCount_ <= 0) {
        return;
    }
    wrapper_ = layoutWrapper;
    auto props = DynamicCast<GridLayoutProperty>(wrapper_->GetLayoutProperty());

    float mainSize = MeasureSelf(props);
    Init(props);

    if (gridLayoutInfo_.targetIndex_) {
        MeasureToTarget();
        gridLayoutInfo_.targetIndex_.reset();
    }

    if (gridLayoutInfo_.jumpIndex_ != EMPTY_JUMP_INDEX) {
        MeasureOnJump(mainSize);
        if (!NearZero(postJumpOffset_)) {
            gridLayoutInfo_.currentOffset_ = postJumpOffset_;
            enableSkip_ = false;
            MeasureOnOffset(mainSize);
        }
    } else {
        MeasureOnOffset(mainSize);
    }

    if (props->GetAlignItems().value_or(GridItemAlignment::DEFAULT) == GridItemAlignment::STRETCH) {
        GridLayoutBaseAlgorithm::AdjustChildrenHeight(layoutWrapper);
    }

    UpdateLayoutInfo();
    wrapper_->SetCacheCount(static_cast<int32_t>(props->GetCachedCountValue(1) * gridLayoutInfo_.crossCount_));
}

void GridIrregularLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    const auto& info = gridLayoutInfo_;
    if (info.childrenCount_ <= 0) {
        return;
    }
    wrapper_ = layoutWrapper;

    LayoutChildren(info.currentOffset_);

    auto props = DynamicCast<GridLayoutProperty>(wrapper_->GetLayoutProperty());
    CHECK_NULL_VOID(props);
    const int32_t cacheCnt = props->GetCachedCountValue(1) * info.crossCount_;
    wrapper_->SetActiveChildRange(info.startIndex_, info.endIndex_, cacheCnt, cacheCnt);

    std::list<int32_t> itemsToPreload;
    for (int32_t i = 1; i <= cacheCnt; ++i) {
        const int32_t l = info.startIndex_ - i;
        if (l >= 0 && !wrapper_->GetChildByIndex(l)) {
            itemsToPreload.push_back(l);
        }
        const int32_t r = info.endIndex_ + i;
        if (r < info.childrenCount_ && !wrapper_->GetChildByIndex(r)) {
            itemsToPreload.push_back(r);
        }
    }
    GridLayoutUtils::PreloadGridItems(wrapper_->GetHostNode()->GetPattern<GridPattern>(), std::move(itemsToPreload),
        [](const RefPtr<FrameNode>& host, int32_t itemIdx) {
            if (host) {
                host->GetOrCreateChildByIndex(itemIdx);
            };
            return false;
        });
}

float GridIrregularLayoutAlgorithm::MeasureSelf(const RefPtr<GridLayoutProperty>& props)
{
    // set self size
    auto size =
        CreateIdealSize(props->GetLayoutConstraint().value(), gridLayoutInfo_.axis_, props->GetMeasureType(), true);
    wrapper_->GetGeometryNode()->SetFrameSize(size);

    // set content size
    const auto& padding = props->CreatePaddingAndBorder();
    wrapper_->GetGeometryNode()->UpdatePaddingWithBorder(padding);
    MinusPaddingToSize(padding, size);
    gridLayoutInfo_.contentEndPadding_ = ScrollableUtils::CheckHeightExpansion(props, gridLayoutInfo_.axis_);
    size.AddHeight(gridLayoutInfo_.contentEndPadding_);
    wrapper_->GetGeometryNode()->SetContentSize(size);

    return size.MainSize(gridLayoutInfo_.axis_);
}

void GridIrregularLayoutAlgorithm::Init(const RefPtr<GridLayoutProperty>& props)
{
    auto& info = gridLayoutInfo_;
    const auto& contentSize = wrapper_->GetGeometryNode()->GetContentSize();
    crossGap_ = GridUtils::GetCrossGap(props, contentSize, info.axis_);
    mainGap_ = GridUtils::GetMainGap(props, contentSize, info.axis_);

    std::string args;
    if (props->GetRowsTemplate()) {
        info.axis_ = Axis::HORIZONTAL;
        args = props->GetRowsTemplate().value_or("");
    } else {
        info.axis_ = Axis::VERTICAL;
        args = props->GetColumnsTemplate().value_or("");
    }

    const float crossSize = contentSize.CrossSize(info.axis_);
    auto res = ParseTemplateArgs(GridUtils::ParseArgs(args), crossSize, crossGap_, info.childrenCount_);

    crossLens_ = std::vector<float>(res.first.begin(), res.first.end());
    if (crossLens_.empty()) {
        crossLens_.push_back(crossSize);
    }

    crossGap_ = res.second;

    info.crossCount_ = static_cast<int32_t>(crossLens_.size());
    CheckForReset();

    if (info.extraOffset_) {
        postJumpOffset_ += *info.extraOffset_;
    }
}

namespace {
inline void PrepareJumpOnReset(GridLayoutInfo& info)
{
    info.jumpIndex_ = std::min(info.startIndex_, info.childrenCount_ - 1);
    info.scrollAlign_ = ScrollAlign::START;
}
inline void ResetMaps(GridLayoutInfo& info)
{
    info.gridMatrix_.clear();
    info.lineHeightMap_.clear();
}
inline void ResetLayoutRange(GridLayoutInfo& info)
{
    info.startIndex_ = 0;
    info.endIndex_ = -1;
    info.startMainLineIndex_ = 0;
    info.endMainLineIndex_ = -1;
    info.currentOffset_ = 0.0f;
    info.prevOffset_ = 0.0f;
}
} // namespace

void GridIrregularLayoutAlgorithm::CheckForReset()
{
    auto& info = gridLayoutInfo_;

    if (info.IsResetted()) {
        // reset layout info and perform jump to current startIndex
        postJumpOffset_ = info.currentOffset_;
        PrepareJumpOnReset(info);
        ResetMaps(info);
        ResetLayoutRange(info);
        return;
    }

    int32_t updateIdx = wrapper_->GetHostNode()->GetChildrenUpdated();
    if (updateIdx != -1) {
        auto it = info.FindInMatrix(updateIdx);
        if (it != info.gridMatrix_.end()) {
            info.ClearHeightsToEnd(it->first);
            info.ClearMatrixToEnd(updateIdx, it->first);
        }
        if (updateIdx <= info.endIndex_) {
            postJumpOffset_ = info.currentOffset_;
            PrepareJumpOnReset(info);
            ResetLayoutRange(info);
        }
        wrapper_->GetHostNode()->ChildrenUpdatedFrom(-1);
        return;
    }

    if (wrapper_->GetLayoutProperty()->GetPropertyChangeFlag() & PROPERTY_UPDATE_BY_CHILD_REQUEST) {
        postJumpOffset_ = info.currentOffset_;
        info.lineHeightMap_.clear();
        PrepareJumpOnReset(info);
        ResetLayoutRange(info);
        return;
    }

    if (!wrapper_->IsConstraintNoChanged()) {
        // need to remeasure all items in current view
        postJumpOffset_ = info.currentOffset_;
        PrepareJumpOnReset(info);
    }
}

void GridIrregularLayoutAlgorithm::MeasureOnOffset(float mainSize)
{
    if (TrySkipping(mainSize)) {
        return;
    }

    if (GreatNotEqual(gridLayoutInfo_.currentOffset_, gridLayoutInfo_.prevOffset_)) {
        MeasureBackward(mainSize);
    } else {
        MeasureForward(mainSize);
    }
}

namespace {
inline void UpdateStartInfo(GridLayoutInfo& info, const GridLayoutRangeSolver::StartingRowInfo& res)
{
    info.startMainLineIndex_ = res.row;
    info.currentOffset_ = res.pos;
    info.startIndex_ = res.idx;
}

inline float GetPrevHeight(const GridLayoutInfo& info, float mainGap)
{
    return info.GetHeightInRange(info.startMainLineIndex_, info.endMainLineIndex_, mainGap);
}
} // namespace

void GridIrregularLayoutAlgorithm::MeasureForward(float mainSize)
{
    auto& info = gridLayoutInfo_;

    float heightToFill = mainSize - info.currentOffset_ - GetPrevHeight(info, mainGap_);
    if (Positive(heightToFill)) {
        GridIrregularFiller filler(&gridLayoutInfo_, wrapper_);
        filler.Fill({ crossLens_, crossGap_, mainGap_ }, heightToFill, info.endMainLineIndex_);
    }

    GridLayoutRangeSolver solver(&info, wrapper_);
    auto res = solver.FindStartingRow(mainGap_);
    UpdateStartInfo(info, res);
    auto [endMainLineIdx, endIdx] = solver.SolveForwardForEndIdx(mainGap_, mainSize - res.pos, res.row);
    info.endMainLineIndex_ = endMainLineIdx;
    info.endIndex_ = endIdx;

    // adjust offset
    if (!overScroll_ && info.endIndex_ == info.childrenCount_ - 1) {
        float overDis =
            -info.GetDistanceToBottom(mainSize, info.GetTotalHeightOfItemsInView(mainGap_, false), mainGap_);
        if (Negative(overDis)) {
            return;
        }
        info.currentOffset_ += overDis;
        if (Positive(info.currentOffset_)) {
            MeasureBackward(mainSize);
        }
    }
}

void GridIrregularLayoutAlgorithm::MeasureBackward(float mainSize)
{
    auto& info = gridLayoutInfo_;
    // skip adding starting lines that are outside viewport in LayoutIrregular
    auto [it, offset] = info.SkipLinesAboveView(mainGap_);
    GridIrregularFiller filler(&gridLayoutInfo_, wrapper_);
    filler.MeasureBackward({ crossLens_, crossGap_, mainGap_ }, offset + it->second + mainGap_, it->first);

    GridLayoutRangeSolver solver(&info, wrapper_);
    auto res = solver.FindStartingRow(mainGap_);
    if (!overScroll_ && res.row == 0) {
        res.pos = std::min(res.pos, 0.0f);
    }
    UpdateStartInfo(info, res);

    auto [endLine, endIdx] = solver.SolveForwardForEndIdx(mainGap_, mainSize - res.pos, res.row);
    info.endMainLineIndex_ = endLine;
    info.endIndex_ = endIdx;
}

namespace {
constexpr float SKIP_THRESHOLD = 2.0f;
}

bool GridIrregularLayoutAlgorithm::TrySkipping(float mainSize)
{
    auto& info = gridLayoutInfo_;
    float delta = std::abs(info.currentOffset_ - info.prevOffset_);
    if (enableSkip_ && GreatNotEqual(delta, mainSize)) {
        // a more costly check, therefore perform after comparing to [mainSize]
        if (LessOrEqual(delta, SKIP_THRESHOLD * info.GetTotalHeightOfItemsInView(mainGap_))) {
            return false;
        }
        info.jumpIndex_ = Negative(info.currentOffset_) ? SkipLinesForward() : SkipLinesBackward();
        info.scrollAlign_ = ScrollAlign::START;
        info.currentOffset_ = 0.0f;
        MeasureOnJump(mainSize);
        return true;
    }
    return false;
}

void GridIrregularLayoutAlgorithm::MeasureOnJump(float mainSize)
{
    auto& info = gridLayoutInfo_;

    if (info.jumpIndex_ == JUMP_TO_BOTTOM_EDGE) {
        GridIrregularFiller filler(&info, wrapper_);
        filler.FillMatrixOnly(info.childrenCount_ - 1);
        info.PrepareJumpToBottom();
    }

    if (info.jumpIndex_ == LAST_ITEM) {
        info.jumpIndex_ = info.childrenCount_ - 1;
    }

    if (info.scrollAlign_ == ScrollAlign::AUTO) {
        int32_t height = GridLayoutUtils::GetItemSize(&info, wrapper_, info.jumpIndex_).rows;
        info.scrollAlign_ = info.TransformAutoScrollAlign(info.jumpIndex_, height, mainSize, mainGap_);
    }
    if (info.scrollAlign_ == ScrollAlign::NONE) {
        info.jumpIndex_ = EMPTY_JUMP_INDEX;
        return;
    }

    int32_t jumpLineIdx = FindJumpLineIdx(info.jumpIndex_);

    PrepareLineHeight(mainSize, jumpLineIdx);

    GridLayoutRangeSolver solver(&info, wrapper_);
    const auto res = solver.FindRangeOnJump(info.jumpIndex_, jumpLineIdx, mainGap_);

    info.currentOffset_ = res.pos;
    info.startMainLineIndex_ = res.startRow;
    info.startIndex_ = res.startIdx;
    info.endMainLineIndex_ = res.endRow;
    info.endIndex_ = res.endIdx;
    info.jumpIndex_ = EMPTY_JUMP_INDEX;
}

void GridIrregularLayoutAlgorithm::UpdateLayoutInfo()
{
    auto& info = gridLayoutInfo_;

    info.reachStart_ = info.startIndex_ == 0 && NonNegative(info.currentOffset_);
    // GridLayoutInfo::reachEnd_ has a different meaning
    info.reachEnd_ = info.endIndex_ == info.childrenCount_ - 1;

    float mainSize = wrapper_->GetGeometryNode()->GetContentSize().MainSize(info.axis_);

    info.lastMainSize_ = mainSize;
    info.totalHeightOfItemsInView_ = info.GetTotalHeightOfItemsInView(mainGap_, false);
    info.avgLineHeight_ = info.GetTotalLineHeight(0.0f) / static_cast<float>(info.lineHeightMap_.size());

    if (info.reachEnd_) {
        info.offsetEnd_ = NonPositive(info.GetDistanceToBottom(mainSize, info.totalHeightOfItemsInView_, mainGap_));
    } else {
        info.offsetEnd_ = false;
    }
    info.prevOffset_ = info.currentOffset_;

    // validity check
    for (int i = info.startMainLineIndex_; i <= info.endMainLineIndex_; ++i) {
        if (info.lineHeightMap_.find(i) == info.lineHeightMap_.end()) {
            TAG_LOGW(AceLogTag::ACE_GRID,
                "lineHeight at line %d not ready. Data is corrupted. StartLine = %d, EndLine = %d", i,
                info.startMainLineIndex_, info.endMainLineIndex_);
            info.endMainLineIndex_ = i - 1;
            info.endIndex_ = info.startIndex_ - 1;
            return;
        }
    }
}

void GridIrregularLayoutAlgorithm::LayoutChildren(float mainOffset)
{
    const auto& info = gridLayoutInfo_;
    Alignment align = info.axis_ == Axis::VERTICAL ? Alignment::TOP_CENTER : Alignment::CENTER_LEFT;
    const auto& props = DynamicCast<GridLayoutProperty>(wrapper_->GetLayoutProperty());
    const auto& positionProp = props->GetPositionProperty();
    if (positionProp) {
        align = positionProp->GetAlignment().value_or(align);
    }

    const auto& padding = *wrapper_->GetGeometryNode()->GetPadding();
    mainOffset += info.axis_ == Axis::HORIZONTAL ? 0.0f : padding.top.value_or(0.0f);
    auto crossPos = CalculateCrossPositions(padding);

    auto frameSize = wrapper_->GetGeometryNode()->GetFrameSize();
    MinusPaddingToSize(padding, frameSize);
    const bool isRtl = props->GetNonAutoLayoutDirection() == TextDirection::RTL;

    auto endIt = info.gridMatrix_.upper_bound(std::max(info.endMainLineIndex_, info.startMainLineIndex_));
    for (auto it = info.gridMatrix_.lower_bound(info.startMainLineIndex_); it != endIt; ++it) {
        auto lineHeightIt = info.lineHeightMap_.find(it->first);
        if (lineHeightIt == info.lineHeightMap_.end()) {
            continue;
        }
        const auto& row = it->second;
        for (const auto& [c, itemIdx] : row) {
            if (itemIdx < 0) {
                // not top-left tile
                continue;
            }
            if (itemIdx == 0 && (it->first > 0 || c > 0)) {
                continue;
            }
            auto child = wrapper_->GetOrCreateChildByIndex(itemIdx);
            if (!child) {
                continue;
            }

            SizeF blockSize = SizeF(crossLens_.at(c), lineHeightIt->second, info.axis_);
            auto childSize = child->GetGeometryNode()->GetMarginFrameSize();
            auto alignPos = Alignment::GetAlignPosition(blockSize, childSize, align);

            OffsetF offset = OffsetF(crossPos[c], mainOffset, info.axis_);

            if (isRtl) {
                offset.SetX(frameSize.Width() - offset.GetX() - childSize.Width());
            }
            offset += OffsetF { padding.left.value_or(0.0f), 0.0f };
            child->GetGeometryNode()->SetMarginFrameOffset(offset + alignPos);
            if (child->CheckNeedForceMeasureAndLayout()) {
                child->Layout();
            } else {
                child->GetHostNode()->ForceSyncGeometryNode();
            }
        }
        // add mainGap below the item
        mainOffset += lineHeightIt->second + mainGap_;
    }
}

std::vector<float> GridIrregularLayoutAlgorithm::CalculateCrossPositions(const PaddingPropertyF& padding)
{
    std::vector<float> res(gridLayoutInfo_.crossCount_, 0.0f);
    res[0] = gridLayoutInfo_.axis_ == Axis::HORIZONTAL ? padding.top.value_or(0.0f) : 0.0f;
    for (int32_t i = 1; i < gridLayoutInfo_.crossCount_; ++i) {
        res[i] = res[i - 1] + crossLens_[i - 1] + crossGap_;
    }
    return res;
}

int32_t GridIrregularLayoutAlgorithm::FindJumpLineIdx(int32_t jumpIdx)
{
    auto& info = gridLayoutInfo_;
    int32_t jumpLine = -1;
    auto it = info.FindInMatrix(jumpIdx);
    if (it == info.gridMatrix_.end()) {
        // fill matrix up to jumpIndex_
        GridIrregularFiller filler(&info, wrapper_);
        jumpLine = filler.FillMatrixOnly(jumpIdx);
    } else {
        jumpLine = it->first;
    }

    if (info.scrollAlign_ == ScrollAlign::END) {
        // jump to the last line the item occupies
        auto lastLine = jumpLine + GridLayoutUtils::GetItemSize(&info, wrapper_, jumpIdx).rows - 1;
        GridIrregularFiller filler(&info, wrapper_);
        filler.FillMatrixByLine(jumpLine, lastLine + 1);
        jumpLine = lastLine;
    }
    return jumpLine;
}

void GridIrregularLayoutAlgorithm::PrepareLineHeight(float mainSize, int32_t& jumpLineIdx)
{
    /* When mainSize can't be filled, adjust parameters and call function again. The maximum length of
     * the recursion is 3 iterations ([Start && len not filled] -> [End && len not filled] -> [Start with jumpIdx 0]).
     */
    auto& info = gridLayoutInfo_;
    GridIrregularFiller filler(&info, wrapper_);
    switch (info.scrollAlign_) {
        case ScrollAlign::START: {
            const GridIrregularFiller::FillParameters params { crossLens_, crossGap_, mainGap_ };
            // call this to ensure irregular items on the first line are measured, not skipped
            filler.MeasureLineWithIrregulars(params, jumpLineIdx);

            float len = filler.Fill(params, mainSize, jumpLineIdx).length;
            // condition [jumpLineIdx > 0] guarantees a finite call stack
            if (LessNotEqual(len, mainSize) && jumpLineIdx > 0) {
                jumpLineIdx = info.lineHeightMap_.rbegin()->first;
                info.scrollAlign_ = ScrollAlign::END;
                PrepareLineHeight(mainSize, jumpLineIdx);
            }
            break;
        }
        case ScrollAlign::CENTER: {
            // because the current line's height is unknown, we can't determine the exact target length to fill.
            // Using the full [mainSize]
            float targetLen = mainSize / 2.0f;
            float backwardLen = filler.MeasureBackward({ crossLens_, crossGap_, mainGap_ }, mainSize, jumpLineIdx);
            backwardLen -= info.lineHeightMap_.at(jumpLineIdx) / 2.0f;
            if (LessNotEqual(backwardLen, targetLen)) {
                jumpLineIdx = 0;
                info.scrollAlign_ = ScrollAlign::START;
                PrepareLineHeight(mainSize, jumpLineIdx);
                return;
            }
            float forwardLen = filler.Fill({ crossLens_, crossGap_, mainGap_ }, mainSize, jumpLineIdx).length;
            forwardLen -= info.lineHeightMap_.at(jumpLineIdx) / 2.0f;
            if (LessNotEqual(forwardLen, targetLen)) {
                jumpLineIdx = info.lineHeightMap_.rbegin()->first;
                info.scrollAlign_ = ScrollAlign::END;
                PrepareLineHeight(mainSize, jumpLineIdx);
            }
            break;
        }
        case ScrollAlign::END: {
            float len = filler.MeasureBackward({ crossLens_, crossGap_, mainGap_ }, mainSize, jumpLineIdx);
            if (LessNotEqual(len, mainSize)) {
                jumpLineIdx = 0;
                info.scrollAlign_ = ScrollAlign::START;
                PrepareLineHeight(mainSize, jumpLineIdx);
            }
            break;
        }
        default:
            break;
    }
}

namespace {
void AddLineHeight(float& height, int32_t curLine, int32_t startLine, const std::map<int32_t, float>& lineHeights)
{
    auto iter = lineHeights.find(curLine);
    if (iter != lineHeights.end()) {
        height += iter->second;
    } else {
        // estimation
        height += height / std::abs(curLine - startLine);
    }
}
} // namespace

int32_t GridIrregularLayoutAlgorithm::SkipLinesForward()
{
    auto& info = gridLayoutInfo_;
    int32_t line = info.startMainLineIndex_;
    float height = 0.0f;
    while (LessNotEqual(height, -info.currentOffset_)) {
        AddLineHeight(height, line++, info.startMainLineIndex_, info.lineHeightMap_);
    }
    GridIrregularFiller filler(&info, wrapper_);
    return filler.FillMatrixByLine(info.startMainLineIndex_, line);
}

int32_t GridIrregularLayoutAlgorithm::SkipLinesBackward() const
{
    const auto& info = gridLayoutInfo_;
    float height = info.GetHeightInRange(info.startMainLineIndex_, info.endMainLineIndex_ + 1, 0.0f);

    float target = info.currentOffset_ + height;
    int32_t line = info.startMainLineIndex_;
    while (LessNotEqual(height, target) && line > 0) {
        AddLineHeight(height, --line, info.endMainLineIndex_, info.lineHeightMap_);
    }
    return std::max(0, info.FindEndIdx(line).itemIdx);
}

void GridIrregularLayoutAlgorithm::MeasureToTarget()
{
    auto& info = gridLayoutInfo_;
    GridIrregularFiller filler(&gridLayoutInfo_, wrapper_);
    GridIrregularFiller::FillParameters param { crossLens_, crossGap_, mainGap_ };
    if (info.targetIndex_ < info.startIndex_) {
        auto it = info.FindInMatrix(*info.targetIndex_);
        filler.MeasureBackwardToTarget(param, it->first, info.startMainLineIndex_);
    } else {
        filler.FillToTarget(param, *info.targetIndex_, info.startMainLineIndex_);
    }
}

bool GridIrregularLayoutAlgorithm::IsIrregularLine(int32_t lineIndex) const
{
    const auto& line = gridLayoutInfo_.gridMatrix_.find(lineIndex);
    if (line == gridLayoutInfo_.gridMatrix_.end() || line->second.empty()) {
        return true;
    }
    auto props = AceType::DynamicCast<GridLayoutProperty>(wrapper_->GetLayoutProperty());
    auto opts = &props->GetLayoutOptions().value();
    for (const auto& item : line->second) {
        if (!item.second || opts->irregularIndexes.find(std::abs(item.second)) != opts->irregularIndexes.end()) {
            return true;
        }
    }
    return false;
}
} // namespace OHOS::Ace::NG
