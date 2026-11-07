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

#include "gtest/gtest.h"
#include "test/unittest/core/pattern/test_ng.h"

#include "core/components_ng/layout/layout_wrapper_node.h"

#include "core/components_ng/pattern/list/list_item_group_pattern.h"
#include "core/components_ng/pattern/list/list_layout_algorithm.h"
#include "core/components_ng/pattern/list/list_pattern.h"
#include "core/components_ng/pattern/list/list_position_map.h"
#include "core/components_ng/syntax/lazy_for_each_model.h"
#include "core/components_ng/syntax/repeat_virtual_scroll_2_caches.h"
#include "core/components_ng/syntax/repeat_virtual_scroll_2_node.h"
#include "core/components_ng/syntax/repeat_virtual_scroll_node.h"
#include "core/components_v2/inspector/inspector_constants.h"

namespace OHOS::Ace::NG {
using namespace testing;
using namespace testing::ext;

class ListPositionMapTestNg : public TestNG {
public:
};

/**
 * @tc.name: FastScrollLanesRowRejectsInvalidInputs001
 * @tc.desc: Invalid counts, indices, lanes and segments never reach an unsafe iterator or division.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollLanesRowRejectsInvalidInputs001, TestSize.Level1)
{
    constexpr int32_t itemCount = 6;
    constexpr int32_t lanes = 2;
    ListPositionMap map;
    map.UpdateTotalCount(itemCount);
    const std::map<int32_t, int32_t> validRanges = { { 0, itemCount - 1 } };
    for (const bool stackFromEnd : { false, true }) {
        SCOPED_TRACE(stackFromEnd);
        EXPECT_FALSE(map.GetLanesRow(0, 0, validRanges, stackFromEnd));
        EXPECT_FALSE(map.GetLanesRow(0, -1, validRanges, stackFromEnd));
        EXPECT_FALSE(map.GetLanesRow(-1, lanes, validRanges, stackFromEnd));
        EXPECT_FALSE(map.GetLanesRow(itemCount, lanes, validRanges, stackFromEnd));
        for (const auto& ranges : { std::map<int32_t, int32_t> {},
            std::map<int32_t, int32_t> { { 1, itemCount - 1 } },
            std::map<int32_t, int32_t> { { 0, -1 } },
            std::map<int32_t, int32_t> { { -1, itemCount - 1 } },
            std::map<int32_t, int32_t> { { 0, itemCount } } }) {
            const int32_t index = stackFromEnd ? itemCount - 1 : 0;
            EXPECT_FALSE(map.GetLanesRow(index, lanes, ranges, stackFromEnd));
        }
    }
}

/**
 * @tc.name: FastScrollLanesRangesShareRowPhase001
 * @tc.desc: Long segments derive complete rows, partial Group boundaries, and mirrored rows without another cache.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollLanesRangesShareRowPhase001, TestSize.Level1)
{
    ListPositionMap map;
    map.UpdateTotalCount(20);
    const std::map<int32_t, int32_t> itemRanges = { { 3, 5 }, { 6, 6 }, { 7, 14 } };
    const std::vector<ListPositionMap::EstimateRange> expected = {
        { 3, 5 }, { 6, 6 }, { 7, 9 }, { 10, 12 }, { 13, 14 }
    };
    for (const auto& row : expected) {
        for (int32_t index = row.startIndex; index <= row.endIndex; ++index) {
            const auto normal = map.GetLanesRow(index, 3, itemRanges);
            ASSERT_TRUE(normal);
            EXPECT_EQ(normal->startIndex, row.startIndex);
            EXPECT_EQ(normal->endIndex, row.endIndex);
            const auto mirrored = map.GetLanesRow(19 - index, 3, itemRanges, true);
            ASSERT_TRUE(mirrored);
            EXPECT_EQ(mirrored->startIndex, 19 - row.endIndex);
            EXPECT_EQ(mirrored->endIndex, 19 - row.startIndex);
        }
    }
    EXPECT_FALSE(map.GetLanesRow(2, 3, itemRanges));
    EXPECT_FALSE(map.GetLanesRow(15, 3, itemRanges));
    EXPECT_TRUE(map.posMap_.empty());
}

/**
 * @tc.name: FastScrollLanesRangesZeroHeight001
 * @tc.desc: Zero-height rows with zero spacing remain different rows even when all positions are equal.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollLanesRangesZeroHeight001, TestSize.Level1)
{
    ListPositionMap map;
    map.UpdateTotalCount(7);
    const std::map<int32_t, int32_t> itemRanges = { { 0, 2 }, { 3, 3 }, { 4, 6 } };
    map.UpdatePos(3, { -1.0f, 0.0f, true });
    ASSERT_TRUE(map.EstimatePositions({ 0, 6 }, 0.0f, 3, 0.0f, itemRanges, false));
    EXPECT_EQ(map.GetLanesRow(1, 3, itemRanges)->startIndex, 0);
    EXPECT_EQ(map.GetLanesRow(3, 3, itemRanges)->startIndex, 3);
    EXPECT_EQ(map.GetLanesRow(5, 3, itemRanges)->startIndex, 4);
    EXPECT_TRUE(map.GetPositionInfo(3).isGroup);
    for (int32_t index = 0; index < 7; ++index) {
        EXPECT_FLOAT_EQ(map.GetPositionInfo(index).mainPos, 0.0f);
        EXPECT_FLOAT_EQ(map.GetPositionInfo(index).mainSize, 0.0f);
    }
}

/**
 * @tc.name: FastScrollLanesRangesBackwardGroupPhase001
 * @tc.desc: Rows use their ordinary segment origin and stop before a Group, including an incomplete row.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollLanesRangesBackwardGroupPhase001, TestSize.Level1)
{
    ListPositionMap map;
    map.UpdateTotalCount(7);
    const std::map<int32_t, int32_t> itemRanges = { { 0, 2 }, { 3, 3 }, { 4, 6 } };
    map.UpdatePos(3, { -1.0f, 400.0f, true });
    map.UpdatePos(4, { 630.0f, 100.0f });
    ASSERT_TRUE(map.EstimatePositions({ 0, 5 }, 100.0f, 2, 10.0f, itemRanges, false));
    EXPECT_FLOAT_EQ(map.GetPositionInfo(0).mainPos, 0.0f);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(1).mainPos, 0.0f);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(2).mainPos, 110.0f);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(3).mainPos, 220.0f);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(4).mainPos, 630.0f);
}

/**
 * @tc.name: FastScrollGroupFlagsSurvivePositionReestimate001
 * @tc.desc: Confirmed types survive re-estimation without restoring obsolete positions or sizes.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollGroupFlagsSurvivePositionReestimate001, TestSize.Level1)
{
    ListPositionMap map;
    map.UpdateTotalCount(12);
    map.UpdatePos(2, { 100.0f, 200.0f, true });
    map.UpdatePos(3, { 310.0f, 80.0f, false });
    map.UpdatePos(7, { -1.0f, -1.0f, true });
    const auto flags = map.GetItemGroupFlags();
    map.ClearPosMap();
    map.UpdatePos(2, { 150.0f, 240.0f, false });
    map.UpdatePos(3, { 400.0f, 90.0f, true, true });
    map.UpdateItemGroupFlags(flags);
    EXPECT_TRUE(map.GetPositionInfo(2).isGroup);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(2).mainPos, 150.0f);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(2).mainSize, 240.0f);
    EXPECT_FALSE(map.GetPositionInfo(3).isGroup);
    EXPECT_TRUE(map.GetPositionInfo(3).isLazyChild);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(3).mainPos, 400.0f);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(3).mainSize, 90.0f);
    EXPECT_TRUE(map.GetPositionInfo(7).isGroup);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(7).mainPos, -1.0f);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(7).mainSize, -1.0f);
    EXPECT_EQ(map.posMap_.size(), 3);
}

/**
 * @tc.name: FastScrollGroupFlagsRespectCurrentCount001
 * @tc.desc: Restoring confirmed flags cannot reinsert indices removed by a smaller total count.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollGroupFlagsRespectCurrentCount001, TestSize.Level1)
{
    ListPositionMap map;
    map.UpdateTotalCount(8);
    map.UpdatePos(1, { 100.0f, 200.0f, true });
    map.UpdatePos(7, { 1000.0f, 200.0f, true });
    const auto flags = map.GetItemGroupFlags();
    map.ClearPosMap();
    map.UpdateTotalCount(4);
    map.UpdateItemGroupFlags(flags);
    EXPECT_TRUE(map.GetPositionInfo(1).isGroup);
    EXPECT_EQ(map.posMap_.count(7), 0);
    EXPECT_EQ(map.posMap_.size(), 1);
}

/**
 * @tc.name: FastScrollConfirmedRowsConnectBackward001
 * @tc.desc: Nonzero type segments use row maxima and connect to the visible anchor without creating a prefix.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollConfirmedRowsConnectBackward001, TestSize.Level1)
{
    ListPositionMap map;
    map.UpdateTotalCount(20);
    const std::map<int32_t, int32_t> itemRanges = { { 3, 5 }, { 6, 6 }, { 7, 14 } };
    map.UpdatePos(3, { -1.0f, 120.0f });
    map.UpdatePos(4, { -1.0f, 200.0f });
    map.UpdatePos(5, { -1.0f, 80.0f });
    map.UpdatePos(6, { -1.0f, 700.0f, true });
    map.UpdatePos(7, { -1.0f, 150.0f });
    map.UpdatePos(12, { 1000.0f, 100.0f });
    ASSERT_TRUE(map.EstimatePositions({ 5, 12 }, 100.0f, 3, 10.0f, itemRanges, false));
    EXPECT_EQ(map.posMap_.size(), 10);
    EXPECT_EQ(map.posMap_.begin()->first, 3);
    EXPECT_EQ(map.posMap_.rbegin()->first, 12);
    const std::vector<std::pair<int32_t, float>> expected = {
        { 3, 0.0f }, { 6, 210.0f }, { 7, 920.0f }, { 10, 1080.0f }
    };
    for (const auto& [start, position] : expected) {
        const auto row = map.GetLanesRow(start, 3, itemRanges).value();
        for (int32_t index = start; index <= row.endIndex; ++index) {
            EXPECT_FLOAT_EQ(map.GetPositionInfo(index).mainPos, position);
            EXPECT_FLOAT_EQ(map.GetPositionInfo(index).mainSize, map.GetPositionInfo(start).mainSize);
        }
    }
    EXPECT_TRUE(map.GetPositionInfo(6).isGroup);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(12).mainPos - map.GetPositionInfo(3).mainPos, 1080.0f);
}

/**
 * @tc.name: FastScrollConfirmedRowsRejectGap001
 * @tc.desc: An unconfirmed middle row cannot silently produce a disconnected estimated map.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollConfirmedRowsRejectGap001, TestSize.Level1)
{
    ListPositionMap map;
    map.UpdateTotalCount(20);
    const std::map<int32_t, int32_t> itemRanges = { { 3, 5 }, { 9, 11 } };
    map.UpdatePos(9, { 1000.0f, 100.0f });
    EXPECT_FALSE(map.EstimatePositions({ 3, 10 }, 100.0f, 3, 10.0f, itemRanges, false));
    ASSERT_EQ(map.posMap_.size(), 1);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(9).mainPos, 1000.0f);
    EXPECT_FALSE(map.GetLanesRow(6, 3, itemRanges));
    EXPECT_FLOAT_EQ(map.GetPositionInfo(6).mainSize, -1.0f);
}

/**
 * @tc.name: FastScrollConfirmedMirroredRowsPreserveTypes001
 * @tc.desc: Mirrored partial rows reuse confirmed boundaries and preserve Group and Lazy flags during hinted insertion.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollConfirmedMirroredRowsPreserveTypes001, TestSize.Level1)
{
    ListPositionMap map;
    map.UpdateTotalCount(7);
    const std::map<int32_t, int32_t> itemRanges = { { 0, 1 }, { 2, 2 }, { 3, 6 } };
    map.UpdatePos(0, { -1.0f, 80.0f });
    map.UpdatePos(1, { -1.0f, 90.0f, false, true });
    map.UpdatePos(2, { -1.0f, 120.0f });
    map.UpdatePos(4, { -1.0f, 300.0f, true });
    map.UpdatePos(6, { 530.0f, 100.0f });
    ASSERT_TRUE(map.EstimatePositions({ 0, 6 }, 100.0f, 3, 10.0f, itemRanges, true));
    const std::vector<float> positions = { 0.0f, 90.0f, 90.0f, 90.0f, 220.0f, 530.0f, 530.0f };
    const std::vector<float> sizes = { 80.0f, 120.0f, 120.0f, 120.0f, 300.0f, 100.0f, 100.0f };
    ASSERT_EQ(map.posMap_.size(), positions.size());
    for (int32_t index = 0; index < 7; ++index) {
        EXPECT_FLOAT_EQ(map.GetPositionInfo(index).mainPos, positions[index]);
        EXPECT_FLOAT_EQ(map.GetPositionInfo(index).mainSize, sizes[index]);
    }
    EXPECT_TRUE(map.GetPositionInfo(1).isLazyChild);
    EXPECT_TRUE(map.GetPositionInfo(4).isGroup);
}

/**
 * @tc.name: FastScrollConfirmedRowsKeepPrefix001
 * @tc.desc: Backward estimation preserves the prefix and connects after its end, including positive overlap.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollConfirmedRowsKeepPrefix001, TestSize.Level1)
{
    for (const float anchorPosition : { 100.0f, 400.0f }) {
        SCOPED_TRACE(anchorPosition);
        ListPositionMap map;
        map.UpdateTotalCount(13);
        map.UpdatePos(0, { 0.0f, 100.0f });
        map.UpdatePos(1, { 0.0f, 100.0f });
        map.UpdatePos(2, { 110.0f, 200.0f });
        map.UpdatePos(3, { 110.0f, 200.0f });
        const auto prefix = map.posMap_;
        map.UpdatePos(8, { anchorPosition, 100.0f });
        map.UpdatePos(10, { 250.0f, 80.0f });
        map.UpdatePos(11, { 250.0f, 80.0f });
        map.UpdateItemGroupFlags({ { 12, true } });
        const std::map<int32_t, int32_t> itemRanges = { { 4, 9 } };
        ASSERT_TRUE(map.EstimatePositions({ 4, 8 }, 100.0f, 2, 10.0f, itemRanges));
        for (const auto& [index, info] : prefix) {
            EXPECT_FLOAT_EQ(map.GetPositionInfo(index).mainPos, info.mainPos);
            EXPECT_FLOAT_EQ(map.GetPositionInfo(index).mainSize, info.mainSize);
        }
        EXPECT_FLOAT_EQ(map.GetPositionInfo(4).mainPos, 320.0f);
        EXPECT_FLOAT_EQ(map.GetPositionInfo(6).mainPos, 430.0f);
        EXPECT_FLOAT_EQ(map.GetPositionInfo(8).mainPos, 540.0f);
        EXPECT_FLOAT_EQ(map.GetPositionInfo(10).mainPos, 650.0f);
        EXPECT_FLOAT_EQ(map.GetPositionInfo(11).mainPos, 650.0f);
        EXPECT_FLOAT_EQ(map.GetPositionInfo(12).mainPos, -1.0f);
        EXPECT_FLOAT_EQ(map.GetPositionInfo(12).mainSize, -1.0f);
        EXPECT_TRUE(map.GetPositionInfo(12).isGroup);
    }
}

/**
 * @tc.name: FastScrollSuffixSkipsUnmeasuredEntries001
 * @tc.desc: Type-only and size-only suffix records neither choose the connection offset nor receive it.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollSuffixSkipsUnmeasuredEntries001, TestSize.Level1)
{
    ListPositionMap map;
    map.UpdateTotalCount(10);
    map.UpdatePos(3, { 100.0f, 100.0f });
    map.UpdateItemGroupFlags({ { 4, true } });
    map.UpdatePos(5, { -1.0f, 50.0f, false, true });
    map.UpdatePos(6, { 150.0f, 80.0f, false, true });
    map.UpdatePos(7, { 150.0f, 80.0f });
    map.UpdatePos(8, { 200.0f, -1.0f, true });
    map.UpdatePos(9, { 300.0f, 10.0f });
    const auto original = map.posMap_;
    map.UpdateFastScrollPosMapEnd(3, 10.0f, true);
    for (const int32_t index : { 4, 5, 8 }) {
        EXPECT_FLOAT_EQ(map.GetPositionInfo(index).mainPos, original.at(index).mainPos);
        EXPECT_FLOAT_EQ(map.GetPositionInfo(index).mainSize, original.at(index).mainSize);
        EXPECT_EQ(map.GetPositionInfo(index).isGroup, original.at(index).isGroup);
        EXPECT_EQ(map.GetPositionInfo(index).isLazyChild, original.at(index).isLazyChild);
    }
    EXPECT_FLOAT_EQ(map.GetPositionInfo(6).mainPos, 210.0f);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(7).mainPos, 210.0f);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(9).mainPos, 360.0f);
    EXPECT_TRUE(map.GetPositionInfo(6).isLazyChild);
}

/**
 * @tc.name: FastScrollGroupSuffixShrinksWithoutVisibleTail001
 * @tc.desc: Refining a partial Group's full extent reconnects its adjacent suffix in the fast path only.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollGroupSuffixShrinksWithoutVisibleTail001, TestSize.Level1)
{
    ListPositionMap fast;
    ListPositionMap legacy;
    for (auto* map : { &fast, &legacy }) {
        map->UpdatePos(3, { 100.0f, 210.0f, true });
        map->UpdatePos(4, { 310.0f, 252.0f, true });
        map->UpdatePos(5, { 562.0f, 80.0f });
        map->UpdatePos(3, { 100.0f, 204.0f, true });
    }
    fast.UpdateFastScrollPosMapEnd(3, 0.0f, false);
    EXPECT_FLOAT_EQ(fast.GetPositionInfo(4).mainPos, 304.0f);
    EXPECT_FLOAT_EQ(fast.GetPositionInfo(5).mainPos, 556.0f);
    legacy.UpdatePosMapEnd(3, 0.0f, false);
    EXPECT_FLOAT_EQ(legacy.GetPositionInfo(4).mainPos, 310.0f);
    EXPECT_FLOAT_EQ(legacy.GetPositionInfo(5).mainPos, 562.0f);
}

/**
 * @tc.name: FastScrollPrefixSkipsUnmeasuredEntries001
 * @tc.desc: An incomplete record cannot replace the last geometric prefix when correcting current offset.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollPrefixSkipsUnmeasuredEntries001, TestSize.Level1)
{
    ListPositionMap map;
    map.UpdateTotalCount(5);
    map.UpdatePos(0, { 0.0f, 100.0f });
    map.UpdateItemGroupFlags({ { 1, true } });
    map.UpdatePos(2, { 200.0f, -1.0f, true });
    map.UpdatePos(3, { -1.0f, 50.0f });
    map.UpdatePos(4, { 400.0f, 100.0f });
    const auto original = map.posMap_;
    double currentPosition = 100.0;
    map.UpdateFastScrollPosMapStart(0.0f, currentPosition, 10.0f, 4, 0.0f, false);
    EXPECT_DOUBLE_EQ(currentPosition, 110.0);
    for (const auto& [index, info] : original) {
        EXPECT_FLOAT_EQ(map.GetPositionInfo(index).mainPos, info.mainPos);
        EXPECT_FLOAT_EQ(map.GetPositionInfo(index).mainSize, info.mainSize);
    }
}

/**
 * @tc.name: FastScrollPositionUpdatesMatchLegacyGeometry001
 * @tc.desc: Valid contiguous and gapped records retain the legacy prefix and suffix correction rules.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollPositionUpdatesMatchLegacyGeometry001, TestSize.Level1)
{
    for (const bool atBoundary : { false, true }) {
        for (const int32_t index : { 1, 2 }) {
            for (const float gap : { -50.0f, 0.0f, 50.0f }) {
                SCOPED_TRACE(testing::Message() << atBoundary << ':' << index << ':' << gap);
                ListPositionMap legacy;
                ListPositionMap fast;
                for (auto* map : { &legacy, &fast }) {
                    map->UpdatePos(0, { 0.0f, 100.0f });
                    map->UpdatePos(index, { 110.0f + gap, 80.0f, atBoundary });
                    map->UpdatePos(index + 2, { 150.0f, 50.0f, false, true });
                }
                double legacyOffset = 125.0;
                double fastOffset = legacyOffset;
                legacy.UpdatePosMapStart(10.0f, legacyOffset, 10.0f, index, -15.0f + gap, atBoundary);
                fast.UpdateFastScrollPosMapStart(10.0f, fastOffset, 10.0f, index, -15.0f + gap, atBoundary);
                EXPECT_DOUBLE_EQ(fastOffset, legacyOffset);
                legacy.UpdatePosMapEnd(index, 10.0f, atBoundary);
                fast.UpdateFastScrollPosMapEnd(index, 10.0f, atBoundary);
                for (const auto& [key, info] : legacy.posMap_) {
                    const auto actual = fast.GetPositionInfo(key);
                    EXPECT_FLOAT_EQ(actual.mainPos, info.mainPos);
                    EXPECT_FLOAT_EQ(actual.mainSize, info.mainSize);
                    EXPECT_EQ(actual.isGroup, info.isGroup);
                    EXPECT_EQ(actual.isLazyChild, info.isLazyChild);
                }
            }
        }
    }
}

/**
 * @tc.name: FastScrollRemoveUnpositionedEntries001
 * @tc.desc: Finishing a request discards type-only and size-only records but preserves all positioned geometry.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollRemoveUnpositionedEntries001, TestSize.Level1)
{
    ListPositionMap map;
    map.UpdatePos(0, { 0.0f, 0.0f });
    map.UpdatePos(1, { -1.0f, -1.0f, true });
    map.UpdatePos(2, { -1.0f, 50.0f, false, true });
    map.UpdatePos(3, { 200.0f, -1.0f, true });
    map.UpdatePos(4, { 100.0f, 80.0f, true, true });
    map.RemoveUnpositionedEntries();
    ASSERT_EQ(map.posMap_.size(), 2);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(0).mainPos, 0.0f);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(0).mainSize, 0.0f);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(4).mainPos, 100.0f);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(4).mainSize, 80.0f);
    EXPECT_TRUE(map.GetPositionInfo(4).isGroup);
    EXPECT_TRUE(map.GetPositionInfo(4).isLazyChild);
    map.RemoveUnpositionedEntries();
    EXPECT_EQ(map.posMap_.size(), 2);
}

/**
 * @tc.name: FastScrollLookupSkipsUnmeasuredEntries001
 * @tc.desc: Metadata without a size or position is never returned as a geometric row, including the last record.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollLookupSkipsUnmeasuredEntries001, TestSize.Level1)
{
    ListPositionMap map;
    map.UpdateTotalCount(9);
    map.UpdatePos(0, { 0.0f, 100.0f });
    map.UpdateItemGroupFlags({ { 1, true }, { 8, true } });
    map.UpdatePos(2, { 200.0f, -1.0f, true });
    map.UpdatePos(3, { -1.0f, 90.0f });
    map.UpdatePos(4, { 200.0f, 100.0f });
    EXPECT_EQ(map.FindRowAtPosition(25.0f, 2), std::make_pair(0, 25.0f));
    EXPECT_EQ(map.FindRowAtPosition(250.0f, 2), std::make_pair(4, 50.0f));
    EXPECT_EQ(map.FindRowAtPosition(1000.0f, 2), std::make_pair(4, 800.0f));
    map.ClearPosMap();
    map.UpdateItemGroupFlags({ { 1, true }, { 8, false } });
    EXPECT_EQ(map.FindRowAtPosition(100.0f, 2), std::make_pair(-1, 0.0f));
}

/**
 * @tc.name: FastScrollEstimateSkipsUnmeasuredAnchor001
 * @tc.desc: A position attached to a type-only record does not override the later valid measurement anchor.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateSkipsUnmeasuredAnchor001, TestSize.Level1)
{
    ListPositionMap map;
    map.UpdateTotalCount(8);
    map.UpdatePos(4, { 50.0f, -1.0f, true });
    map.UpdatePos(6, { 300.0f, 100.0f });
    ASSERT_TRUE(map.EstimatePositions({ 4, 6 }, 100.0f, 1, 0.0f));
    EXPECT_FLOAT_EQ(map.GetPositionInfo(4).mainPos, 100.0f);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(5).mainPos, 200.0f);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(6).mainPos, 300.0f);
    EXPECT_FLOAT_EQ(map.GetPositionInfo(4).mainSize, 100.0f);
    EXPECT_TRUE(map.GetPositionInfo(4).isGroup);
}

/**
 * @tc.name: FastScrollAverageSingleLaneSizes001
 * @tc.desc: Historical and size-only entries contribute; invalid sizes do not alter the map.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollAverageSingleLaneSizes001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(6);
    positionMap->UpdatePos(0, { 0.0f, 100.0f });
    positionMap->UpdatePos(1, { -1.0f, 200.0f, false, true });
    positionMap->UpdatePos(2, { 300.0f, 0.0f, true });
    positionMap->UpdatePos(3, { 300.0f, -1.0f });
    positionMap->UpdatePos(4, { 300.0f, std::numeric_limits<float>::quiet_NaN() });
    positionMap->UpdatePos(5, { 300.0f, std::numeric_limits<float>::infinity() });
    const auto average = positionMap->GetAverageRowHeight(1);
    ASSERT_TRUE(average.has_value());
    EXPECT_FLOAT_EQ(average.value(), 100.0f);
    EXPECT_EQ(positionMap->posMap_.size(), 6);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(1).mainPos, -1.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(1).mainSize, 200.0f);
    EXPECT_TRUE(positionMap->GetPositionInfo(1).isLazyChild);
    EXPECT_TRUE(positionMap->GetPositionInfo(2).isGroup);
    EXPECT_TRUE(std::isnan(positionMap->GetPositionInfo(4).mainSize));
}

/**
 * @tc.name: FastScrollAverageMultiLaneRows001
 * @tc.desc: Ordinary rows use their maximum, Groups stand alone, and size-only entries are not merged.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollAverageMultiLaneRows001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(8);
    positionMap->UpdatePos(0, { 0.0f, 10.0f });
    positionMap->UpdatePos(1, { 0.0f, 30.0f });
    positionMap->UpdatePos(2, { 40.0f, 90.0f, true });
    positionMap->UpdatePos(3, { 140.0f, 20.0f });
    positionMap->UpdatePos(4, { 140.0f, 40.0f });
    positionMap->UpdatePos(5, { 140.0f, 10.0f });
    positionMap->UpdatePos(6, { -1.0f, 60.0f });
    positionMap->UpdatePos(7, { -1.0f, 80.0f });
    const auto average = positionMap->GetAverageRowHeight(3);
    ASSERT_TRUE(average.has_value());
    EXPECT_FLOAT_EQ(average.value(), 60.0f);
    EXPECT_EQ(positionMap->posMap_.size(), 8);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(7).mainPos, -1.0f);
}

/**
 * @tc.name: FastScrollAverageSparseRows001
 * @tc.desc: Equal positions across an index gap cannot merge two samples or insert the missing entry.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollAverageSparseRows001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(4);
    positionMap->UpdatePos(0, { 0.0f, 10.0f });
    positionMap->UpdatePos(2, { 0.0f, 50.0f });
    positionMap->UpdatePos(3, { 0.0f, 30.0f });
    const auto average = positionMap->GetAverageRowHeight(3);
    ASSERT_TRUE(average.has_value());
    EXPECT_FLOAT_EQ(average.value(), 30.0f);
    EXPECT_EQ(positionMap->posMap_.size(), 3);
    EXPECT_EQ(positionMap->posMap_.count(1), 0);
}

/**
 * @tc.name: FastScrollAverageZeroAndMissingSamples001
 * @tc.desc: Missing samples differ from legal zero-height rows; lane limits apply even at equal positions.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollAverageZeroAndMissingSamples001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(4);
    EXPECT_FALSE(positionMap->GetAverageRowHeight(1).has_value());
    positionMap->UpdatePos(0, { 0.0f, -1.0f });
    EXPECT_FALSE(positionMap->GetAverageRowHeight(1).has_value());
    positionMap->UpdatePos(0, { 0.0f, 0.0f });
    positionMap->UpdatePos(1, { 0.0f, -0.0005f });
    ASSERT_TRUE(positionMap->GetAverageRowHeight(1).has_value());
    EXPECT_FLOAT_EQ(positionMap->GetAverageRowHeight(1).value(), 0.0f);
    EXPECT_FALSE(positionMap->GetAverageRowHeight(0).has_value());
    EXPECT_FALSE(positionMap->GetAverageRowHeight(-1).has_value());
    positionMap->UpdatePos(0, { 0.0f, 10.0f });
    positionMap->UpdatePos(1, { 0.0f, 20.0f });
    positionMap->UpdatePos(2, { 0.0f, 30.0f });
    positionMap->UpdatePos(3, { 0.0f, 40.0f });
    ASSERT_TRUE(positionMap->GetAverageRowHeight(3).has_value());
    EXPECT_FLOAT_EQ(positionMap->GetAverageRowHeight(3).value(), 35.0f);
}

/**
 * @tc.name: FastScrollAverageKeepsEstimateWeight001
 * @tc.desc: Existing estimates participate, and a normal size update is reflected in the next average.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollAverageKeepsEstimateWeight001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(104);
    positionMap->UpdatePosRange(0, 100, { 0.0f, 100.0f }, 0.0f, 1);
    positionMap->UpdatePosRange(100, 104, { 10000.0f, 200.0f }, 0.0f, 1);
    ASSERT_TRUE(positionMap->GetAverageRowHeight(1).has_value());
    EXPECT_NEAR(positionMap->GetAverageRowHeight(1).value(), 10800.0f / 104.0f, 0.001f);
    positionMap->UpdatePos(0, { 0.0f, 300.0f });
    EXPECT_NEAR(positionMap->GetAverageRowHeight(1).value(), 11000.0f / 104.0f, 0.001f);
}

/**
 * @tc.name: FastScrollAverageLargeFiniteSizes001
 * @tc.desc: Accumulation does not overflow when the finite average fits in float.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollAverageLargeFiniteSizes001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(2);
    const float size = std::numeric_limits<float>::max();
    positionMap->UpdatePos(0, { -1.0f, size });
    positionMap->UpdatePos(1, { -1.0f, size });
    const auto average = positionMap->GetAverageRowHeight(1);
    ASSERT_TRUE(average.has_value());
    EXPECT_FLOAT_EQ(average.value(), size);
}

/**
 * @tc.name: CalculateUINode001
 * @tc.desc: Test ListPositionMap CalculateUINode
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, CalculateUINode001, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemGroupPattern> listItemGroupPattern = AceType::MakeRefPtr<ListItemGroupPattern>(shallowBuilder,
        V2::ListItemGroupOptions{V2::ListItemGroupStyle::CARD});
    listItemGroupPattern->itemStartIndex_ = 2;
    auto listNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 1, listItemGroupPattern);
    ASSERT_NE(listNode, nullptr);
    auto node = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 2, listItemGroupPattern);
    ASSERT_NE(node, nullptr);
    auto lazyForEachNode = LazyForEachNode::CreateLazyForEachNode(2, nullptr);
    ASSERT_NE(lazyForEachNode, nullptr);
    lazyForEachNode->children_.clear();
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    listNode->children_.clear();
    listNode->children_ = { lazyForEachNode };
    listPositionMap->curIndex_ = 2;
    listPositionMap->CalculateUINode(listNode);
    EXPECT_EQ(listPositionMap->curRowHeight_, 0.0f);
    EXPECT_EQ(listPositionMap->curIndex_, 2);
}

/**
 * @tc.name: CalculateUINode002
 * @tc.desc: Test ListPositionMap CalculateUINode
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, CalculateUINode002, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemGroupPattern> listItemGroupPattern = AceType::MakeRefPtr<ListItemGroupPattern>(shallowBuilder,
        V2::ListItemGroupOptions{V2::ListItemGroupStyle::CARD});
    listItemGroupPattern->itemStartIndex_ = 2;
    auto listNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 1, listItemGroupPattern);
    ASSERT_NE(listNode, nullptr);
    auto node = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 2, listItemGroupPattern);
    ASSERT_NE(node, nullptr);
    const std::map<std::string, std::pair<bool, uint32_t>> templateCacheCountMap = {
        { "template1", { true, 0 } },
        { "template2", { false, 0 } },
    };
    const std::function<void(uint32_t)> onCreateNode = [](uint32_t index) {};
    const std::function<void(const std::string&, uint32_t)> onUpdateNode = [](const std::string& key, uint32_t index) {
    };
    const std::function<std::list<std::string>(uint32_t, uint32_t)> onGetKeys4Range = [](uint32_t start, uint32_t end) {
        return std::list<std::string> { "template1", "template2" };
    };
    const std::function<std::list<std::string>(uint32_t, uint32_t)> onGetTypes4Range = [](uint32_t start,
                                                                                           uint32_t end) {
        return std::list<std::string> { "template1", "template2" };
    };
    const std::function<void(int32_t, int32_t)> onSetActiveRange = [](int32_t start, int32_t end) {};
    RefPtr<RepeatVirtualScrollNode> repeatVirtualScrollNode = AceType::MakeRefPtr<RepeatVirtualScrollNode>(2, 5,
        templateCacheCountMap, onCreateNode, onUpdateNode, onGetKeys4Range, onGetTypes4Range, onSetActiveRange, true);
    ASSERT_NE(repeatVirtualScrollNode, nullptr);
    repeatVirtualScrollNode->children_ = { node };
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    listNode->children_.clear();
    listNode->children_ = { repeatVirtualScrollNode };
    node->children_ = { listNode };
    std::vector<float> mainSize = { 2.0f, 2.0f, 2.0f };
    RefPtr<ListChildrenMainSize> listChildrenMainSize = AceType::MakeRefPtr<ListChildrenMainSize>(mainSize, 2.0f);
    listPositionMap->childrenSize_ = listChildrenMainSize;
    listPositionMap->curIndex_ = 2;
    listPositionMap->curRowHeight_ = 0.0f;
    listPositionMap->CalculateUINode(listNode);
    EXPECT_EQ(listPositionMap->curRowHeight_, 2.0f);
    EXPECT_EQ(listPositionMap->curIndex_, 7);
}

/**
 * @tc.name: CalculateUINode003
 * @tc.desc: Test ListPositionMap CalculateUINode
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, CalculateUINode003, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemGroupPattern> listItemGroupPattern = AceType::MakeRefPtr<ListItemGroupPattern>(shallowBuilder,
        V2::ListItemGroupOptions{V2::ListItemGroupStyle::CARD});
    listItemGroupPattern->itemStartIndex_ = 2;
    auto listNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 1, listItemGroupPattern);
    ASSERT_NE(listNode, nullptr);
    auto node = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 2, listItemGroupPattern);
    ASSERT_NE(node, nullptr);
    std::function<std::pair<RIDType, uint32_t>(IndexType, bool, bool)> onGetRid4Index = [](int32_t index,
        bool inAnimation, bool forceCreateNewChild) {
        return std::make_pair(2, 0);
    };
    std::function<void(IndexType, IndexType)> onRecycleItems = [](int32_t start, int32_t end) {};
    std::function<void(int32_t, int32_t, int32_t, int32_t, bool, bool)> onActiveRange =
        [](int32_t start, int32_t end, int32_t vStart, int32_t vEnd, bool isCache, bool forceUpdate) {};
    std::function<void(IndexType, IndexType)> onMoveFromTo = [](int32_t start, int32_t end) {};
    std::function<void()> onPurge = []() {};
    std::function<void()> onPurgeAll = []() {};
    std::function<void()> onUpdateDirty = []() {};
    RefPtr<RepeatVirtualScroll2Node> repeatVirtualScroll2Node = AceType::MakeRefPtr<RepeatVirtualScroll2Node>(
        2, 2, 5, 0, onGetRid4Index, onRecycleItems, onActiveRange, onMoveFromTo, onPurge, onPurgeAll, onUpdateDirty);
    ASSERT_NE(repeatVirtualScroll2Node, nullptr);
    repeatVirtualScroll2Node->children_ = { node };
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    listNode->children_.clear();
    listNode->children_ = { repeatVirtualScroll2Node };
    node->children_ = { listNode };
    std::vector<float> mainSize = { 2.0f, 2.0f, 2.0f };
    RefPtr<ListChildrenMainSize> listChildrenMainSize = AceType::MakeRefPtr<ListChildrenMainSize>(mainSize, 2.0f);
    listPositionMap->childrenSize_ = listChildrenMainSize;
    listPositionMap->curIndex_ = 2;
    listPositionMap->curRowHeight_ = 0.0f;
    listPositionMap->CalculateUINode(listNode);
    EXPECT_EQ(listPositionMap->curRowHeight_, 2.0f);
    EXPECT_EQ(listPositionMap->curIndex_, 4);
}

/**
 * @tc.name: CalculateUINode004
 * @tc.desc: Test ListPositionMap CalculateUINode
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, CalculateUINode004, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemGroupPattern> listItemGroupPattern = AceType::MakeRefPtr<ListItemGroupPattern>(shallowBuilder,
        V2::ListItemGroupOptions{V2::ListItemGroupStyle::CARD});
    listItemGroupPattern->itemStartIndex_ = 2;
    auto listNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 1, listItemGroupPattern);
    ASSERT_NE(listNode, nullptr);
    auto node = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 2, listItemGroupPattern);
    ASSERT_NE(node, nullptr);
    auto customNode = CustomNode::CreateCustomNode(2, "test");
    ASSERT_NE(customNode, nullptr);
    customNode->children_ = { node };
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    listNode->children_.clear();
    listNode->children_ = { customNode };
    std::vector<float> mainSize = { 2.0f, 2.0f, 2.0f };
    RefPtr<ListChildrenMainSize> listChildrenMainSize = AceType::MakeRefPtr<ListChildrenMainSize>(mainSize, 2.0f);
    listPositionMap->childrenSize_ = listChildrenMainSize;
    node->children_ = { listNode };
    listPositionMap->curIndex_ = 2;
    listPositionMap->curRowHeight_ = 0.0f;
    listPositionMap->CalculateUINode(listNode);
    EXPECT_EQ(listPositionMap->curRowHeight_, 0.0f);
    EXPECT_EQ(listPositionMap->curIndex_, 3);
}

/**
 * @tc.name: GetStartIndexAndPos001
 * @tc.desc: Test ListPositionMap GetStartIndexAndPos
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, GetStartIndexAndPos001, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    listPositionMap->posMap_.clear();
    auto result = listPositionMap->GetStartIndexAndPos();
    EXPECT_EQ(result.first, -1);
    EXPECT_EQ(result.second, 0.0f);
}

/**
 * @tc.name: GetLazyForEachChildIsGroup001
 * @tc.desc: Test ListPositionMap GetLazyForEachChildIsGroup
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, GetLazyForEachChildIsGroup001, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemGroupPattern> listItemGroupPattern = AceType::MakeRefPtr<ListItemGroupPattern>(shallowBuilder,
        V2::ListItemGroupOptions{V2::ListItemGroupStyle::CARD});
    listItemGroupPattern->itemStartIndex_ = 2;
    auto listNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 1, listItemGroupPattern);
    ASSERT_NE(listNode, nullptr);
    auto node = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 2, listItemGroupPattern);
    ASSERT_NE(node, nullptr);
    auto lazyForEachNode = LazyForEachNode::CreateLazyForEachNode(2, nullptr);
    ASSERT_NE(lazyForEachNode, nullptr);
    lazyForEachNode->children_.clear();
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    listNode->children_.clear();
    listNode->children_ = { node, node };
    listNode->parent_ = node;
    node->children_ = { lazyForEachNode };
    listPositionMap->curIndex_ = 2;
    auto result = listPositionMap->GetLazyForEachChildIsGroup(listNode);
    EXPECT_EQ(listPositionMap->curIndex_, 2);
    EXPECT_FALSE(result.value());
}

/**
 * @tc.name: GetLazyForEachChildIsGroup002
 * @tc.desc: Test ListPositionMap GetLazyForEachChildIsGroup
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, GetLazyForEachChildIsGroup002, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemGroupPattern> listItemGroupPattern = AceType::MakeRefPtr<ListItemGroupPattern>(shallowBuilder,
        V2::ListItemGroupOptions{V2::ListItemGroupStyle::CARD});
    listItemGroupPattern->itemStartIndex_ = 2;
    auto listNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 1, listItemGroupPattern);
    ASSERT_NE(listNode, nullptr);
    auto node = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 2, listItemGroupPattern);
    ASSERT_NE(node, nullptr);
    auto customNode = CustomNode::CreateCustomNode(2, "test");
    ASSERT_NE(customNode, nullptr);
    customNode->children_.clear();
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    listNode->children_.clear();
    listNode->children_ = { customNode, node };
    listNode->parent_ = node;
    listPositionMap->curIndex_ = 2;
    auto result = listPositionMap->GetLazyForEachChildIsGroup(listNode);
    EXPECT_EQ(listPositionMap->curRowHeight_, 0.0f);
    EXPECT_EQ(listPositionMap->curIndex_, 2);
    EXPECT_FALSE(result.value());
}

/**
 * @tc.name: GetLazyForEachChildIsGroup003
 * @tc.desc: Test ListPositionMap GetLazyForEachChildIsGroup
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, GetLazyForEachChildIsGroup003, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemGroupPattern> listItemGroupPattern = AceType::MakeRefPtr<ListItemGroupPattern>(shallowBuilder,
        V2::ListItemGroupOptions{V2::ListItemGroupStyle::CARD});
    listItemGroupPattern->itemStartIndex_ = 2;
    auto listNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 1, listItemGroupPattern);
    ASSERT_NE(listNode, nullptr);
    auto node = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 2, listItemGroupPattern);
    ASSERT_NE(node, nullptr);
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    listNode->children_.clear();
    listNode->parent_ = node;
    listPositionMap->curIndex_ = 2;
    listPositionMap->GetLazyForEachChildIsGroup(listNode);
    EXPECT_EQ(listPositionMap->curRowHeight_, 0.0f);
    EXPECT_EQ(listPositionMap->curIndex_, 2);
}

/**
 * @tc.name: UpdateGroupPosMap001
 * @tc.desc: Test ListPositionMap UpdateGroupPosMap
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, UpdateGroupPosMap001, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    std::vector<float> mainSize = { 2.0f, 2.0f, 2.0f };
    RefPtr<ListChildrenMainSize> listChildrenMainSize = AceType::MakeRefPtr<ListChildrenMainSize>(mainSize, 2.0f);
    listPositionMap->childrenSize_ = listChildrenMainSize;
    listPositionMap->curIndex_ = 2;
    listPositionMap->curIndex_ = 2;
    listPositionMap->totalItemCount_ = 2;
    listPositionMap->curRowHeight_ = 4.0f;
    listPositionMap->space_ = 1.0f;
    listPositionMap->curLine_ = 1;
    listPositionMap->lanes_ = 1;
    listPositionMap->dirty_ = LIST_UPDATE_SPACE;
    listPositionMap->headerSize_ = 2.0f;
    listPositionMap->footerSize_ = 4.0f;
    listPositionMap->UpdateGroupPosMap(2, 1, 1.0f, listChildrenMainSize, 2.0f, 4.0f);
    EXPECT_EQ(listPositionMap->curRowHeight_, 0.0f);
    EXPECT_EQ(listPositionMap->curIndex_, 0);
}

/**
 * @tc.name: UpdateGroupPosMap002
 * @tc.desc: Test ListPositionMap UpdateGroupPosMap
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, UpdateGroupPosMap002, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    std::vector<float> mainSize = { 2.0f, 2.0f, 2.0f };
    RefPtr<ListChildrenMainSize> listChildrenMainSize = AceType::MakeRefPtr<ListChildrenMainSize>(mainSize, 2.0f);
    listPositionMap->childrenSize_ = listChildrenMainSize;
    listPositionMap->curIndex_ = 2;
    listPositionMap->curIndex_ = 2;
    listPositionMap->totalItemCount_ = 0;
    listPositionMap->curRowHeight_ = 4.0f;
    listPositionMap->space_ = 1.0f;
    listPositionMap->curLine_ = 1;
    listPositionMap->lanes_ = 1;
    listPositionMap->dirty_ = LIST_UPDATE_SPACE;
    listPositionMap->headerSize_ = 2.0f;
    listPositionMap->footerSize_ = 4.0f;
    listPositionMap->totalHeight_ = 4.0f;
    listPositionMap->UpdateGroupPosMap(0, 1, 1.0f, listChildrenMainSize, 2.0f, 4.0f);
    EXPECT_EQ(listPositionMap->totalHeight_, 0.0f);
}

/**
 * @tc.name: ReversePosMap001
 * @tc.desc: Test ListPositionMap ReversePosMap
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ReversePosMap001, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    listPositionMap->posMap_[0] = { 2.0f, 4.0f, true };
    listPositionMap->posMap_[1] = { 2.0f, 4.0f, true };
    listPositionMap->posMap_[2] = { 3.0f, 6.0f, true };
    listPositionMap->totalItemCount_ = 3;
    listPositionMap->space_ = 1.0f;
    listPositionMap->ReversePosMap();
    EXPECT_EQ(listPositionMap->totalHeight_, 11.0f);
    EXPECT_EQ(listPositionMap->posMap_[0].mainPos, 0.0f);
    EXPECT_EQ(listPositionMap->posMap_[1].mainPos, 5.0f);
}

/**
 * @tc.name: CalculateGroupNode001
 * @tc.desc: Test ListPositionMap CalculateGroupNode
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, CalculateGroupNode001, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    std::vector<float> mainSize = { 2.0f, 2.0f, 2.0f };
    RefPtr<ListChildrenMainSize> listChildrenMainSize = AceType::MakeRefPtr<ListChildrenMainSize>(mainSize, 2.0f);
    listPositionMap->childrenSize_ = listChildrenMainSize;
    listPositionMap->curIndex_ = 2;
    listPositionMap->totalHeight_ = 2.0f;
    listPositionMap->curRowHeight_ = 4.0f;
    listPositionMap->space_ = 1.0f;
    listPositionMap->curLine_ = 1;
    listPositionMap->CalculateGroupNode();
    EXPECT_EQ(listPositionMap->posMap_[0].mainPos, 0.0f);
    EXPECT_EQ(listPositionMap->posMap_[2].mainPos, 7.0f);
}

/**
 * @tc.name: UpdatePosMap001
 * @tc.desc: Test ListPositionMap UpdatePosMap
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, UpdatePosMap001, TestSize.Level1)
{
    RefPtr<ShallowBuilder> shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(nullptr);
    RefPtr<ListItemGroupPattern> listItemGroupPattern = AceType::MakeRefPtr<ListItemGroupPattern>(shallowBuilder,
        V2::ListItemGroupOptions{V2::ListItemGroupStyle::CARD});
    RefPtr<ListLayoutAlgorithm> listLayoutAlgorithm = AceType::MakeRefPtr<ListLayoutAlgorithm>(2);
    RefPtr<ListLayoutProperty> listLayoutProperty = AceType::MakeRefPtr<ListLayoutProperty>();
    listItemGroupPattern->itemStartIndex_ = 2;
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    std::vector<float> mainSize = { 2.0f, 2.0f, 2.0f };
    RefPtr<ListChildrenMainSize> listChildrenMainSize = AceType::MakeRefPtr<ListChildrenMainSize>(mainSize, 2.0f);
    listPositionMap->childrenSize_ = listChildrenMainSize;
    auto frameNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, 2, listItemGroupPattern);
    ASSERT_NE(frameNode, nullptr);
    RefPtr<GeometryNode> geometryNode = AceType::MakeRefPtr<GeometryNode>();
    EXPECT_NE(geometryNode, nullptr);
    LayoutWrapperNode layoutWrapper(frameNode, geometryNode, listLayoutProperty);
    layoutWrapper.currentChildCount_ = 2;
    listPositionMap->curIndex_ = 2;
    listPositionMap->curIndex_ = 2;
    listPositionMap->totalItemCount_ = 2;
    listPositionMap->curRowHeight_ = 4.0f;
    listPositionMap->space_ = 1.0f;
    listPositionMap->curLine_ = 1;
    listPositionMap->lanes_ = 1;
    listPositionMap->dirty_ = LIST_UPDATE_DEFAULT_SIZE;
    listPositionMap->UpdatePosMap(&layoutWrapper, 1, 1.0f, listChildrenMainSize);
    EXPECT_EQ(listPositionMap->curRowHeight_, 2.0f);
    EXPECT_EQ(listPositionMap->curIndex_, 0);
}

/**
 * @tc.name: ListPositionMapUpdatePosWithCheck001
 * @tc.desc: Test ListPositionMap UpdatePosWithCheck with smaller mainSize
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapUpdatePosWithCheck001, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();

    // Initialize with a larger size
    ListPositionInfo posInfo = { 0.0f, 200.0f, false };
    listPositionMap->UpdatePos(0, posInfo);

    // Update with a smaller size - should not update
    ListPositionInfo newPosInfo = { 0.0f, 100.0f, false };
    listPositionMap->UpdatePosWithCheck(0, newPosInfo);

    auto result = listPositionMap->GetPositionInfo(0);
    EXPECT_EQ(result.mainSize, 200.0f); // Should keep the larger size
}

/**
 * @tc.name: ListPositionMapUpdatePosWithCheck002
 * @tc.desc: Test ListPositionMap UpdatePosWithCheck with larger mainSize
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapUpdatePosWithCheck002, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();

    // Initialize with a smaller size
    ListPositionInfo posInfo = { 0.0f, 100.0f, false };
    listPositionMap->UpdatePos(0, posInfo);

    // Update with a larger size - should update
    ListPositionInfo newPosInfo = { 0.0f, 200.0f, false };
    listPositionMap->UpdatePosWithCheck(0, newPosInfo);

    auto result = listPositionMap->GetPositionInfo(0);
    EXPECT_EQ(result.mainSize, 200.0f); // Should update to larger size
}

/**
 * @tc.name: ListPositionMapUpdatePosWithCheck003
 * @tc.desc: Test ListPositionMap UpdatePosWithCheck with new index
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapUpdatePosWithCheck003, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();

    // Update a non-existent index
    ListPositionInfo posInfo = { 0.0f, 100.0f, false };
    listPositionMap->UpdatePosWithCheck(0, posInfo);

    auto result = listPositionMap->GetPositionInfo(0);
    EXPECT_EQ(result.mainSize, 100.0f);
    EXPECT_FALSE(result.isGroup);
}

/**
 * @tc.name: ListPositionMapUpdatePosWithCheck004
 * @tc.desc: Test ListPositionMap UpdatePosWithCheck with isGroup update
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapUpdatePosWithCheck004, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();

    // Initialize with isGroup = false
    ListPositionInfo posInfo = { 0.0f, 100.0f, false };
    listPositionMap->UpdatePos(0, posInfo);

    // Update isGroup to true
    ListPositionInfo newPosInfo = { 0.0f, 100.0f, true };
    listPositionMap->UpdatePosWithCheck(0, newPosInfo);

    auto result = listPositionMap->GetPositionInfo(0);
    EXPECT_TRUE(result.isGroup);
}

/**
 * @tc.name: ListPositionMapUpdatePosRangeKeepsIsLazyChild001
 * @tc.desc: Test ListPositionMap UpdatePosRange keeps existing isLazyChild
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapUpdatePosRangeKeepsIsLazyChild001, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    listPositionMap->UpdatePos(0, { 0.0f, 100.0f, false, true });
    listPositionMap->UpdatePos(1, { 110.0f, 100.0f, false, false });

    listPositionMap->UpdatePosRange(0, 2, { 20.0f, 200.0f, true }, 10.0f, 1);

    auto lazyResult = listPositionMap->GetPositionInfo(0);
    EXPECT_EQ(lazyResult.mainPos, 20.0f);
    EXPECT_EQ(lazyResult.mainSize, 200.0f);
    EXPECT_TRUE(lazyResult.isGroup);
    EXPECT_TRUE(lazyResult.isLazyChild);

    auto normalResult = listPositionMap->GetPositionInfo(1);
    EXPECT_EQ(normalResult.mainPos, 230.0f);
    EXPECT_EQ(normalResult.mainSize, 200.0f);
    EXPECT_TRUE(normalResult.isGroup);
    EXPECT_FALSE(normalResult.isLazyChild);
}

/**
 * @tc.name: ListPositionMapUpdatePosWithCheckUpdatesIsLazyChild001
 * @tc.desc: Test ListPositionMap UpdatePosWithCheck updates isLazyChild
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapUpdatePosWithCheckUpdatesIsLazyChild001, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    listPositionMap->UpdatePos(0, { 0.0f, 200.0f, false, false });

    listPositionMap->UpdatePosWithCheck(0, { 0.0f, 100.0f, false, true });

    auto result = listPositionMap->GetPositionInfo(0);
    EXPECT_EQ(result.mainSize, 200.0f);
    EXPECT_TRUE(result.isLazyChild);
}

/**
 * @tc.name: ListPositionMapOptimizeBeforeMeasure001
 * @tc.desc: Test ListPositionMap OptimizeBeforeMeasure with zero offset
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapOptimizeBeforeMeasure001, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();

    ListPositionInfo posInfo = { 0.0f, 100.0f, false };
    listPositionMap->UpdatePos(0, posInfo);
    listPositionMap->totalItemCount_ = 1;
    listPositionMap->totalHeight_ = 100.0f;

    int32_t beginIndex = 0;
    float beginPos = 0.0f;
    float offset = 0.0f;
    float contentSize = 100.0f;

    listPositionMap->OptimizeBeforeMeasure(beginIndex, beginPos, offset, contentSize);

    // Should not change
    EXPECT_EQ(beginIndex, 0);
    EXPECT_EQ(beginPos, 0.0f);
}

/**
 * @tc.name: ListPositionMapOptimizeBeforeMeasure002
 * @tc.desc: Test ListPositionMap OptimizeBeforeMeasure with contentSize >= totalHeight
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapOptimizeBeforeMeasure002, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();

    ListPositionInfo posInfo = { 0.0f, 100.0f, false };
    listPositionMap->UpdatePos(0, posInfo);
    listPositionMap->totalItemCount_ = 1;
    listPositionMap->totalHeight_ = 100.0f;

    int32_t beginIndex = 0;
    float beginPos = 0.0f;
    float offset = 50.0f;
    float contentSize = 200.0f; // Larger than totalHeight

    listPositionMap->OptimizeBeforeMeasure(beginIndex, beginPos, offset, contentSize);

    // Should not change
    EXPECT_EQ(beginIndex, 0);
    EXPECT_EQ(beginPos, 0.0f);
}

/**
 * @tc.name: ListPositionMapUpdatePosMapStart001
 * @tc.desc: Test ListPositionMap UpdatePosMapStart with groupAtStart false
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapUpdatePosMapStart001, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();

    ListPositionInfo posInfo1 = { 100.0f, 100.0f, false };
    ListPositionInfo posInfo2 = { 210.0f, 100.0f, false };
    listPositionMap->UpdatePos(0, posInfo1);
    listPositionMap->UpdatePos(1, posInfo2);

    double listCurrentPos = 0.0f;
    float space = 10.0f;
    int32_t startIndex = 1;
    float startPos = 220.0f;
    bool groupAtStart = false;

    listPositionMap->UpdatePosMapStart(10.0f, listCurrentPos, space, startIndex, startPos, groupAtStart);

    // Should not update listCurrentPos because prevPos < startPos
    EXPECT_EQ(listCurrentPos, 0.0f);
}

/**
 * @tc.name: ListPositionMapUpdatePosMapEnd001
 * @tc.desc: Test ListPositionMap UpdatePosMapEnd with non-existent index
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapUpdatePosMapEnd001, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();

    ListPositionInfo posInfo = { 0.0f, 100.0f, false };
    listPositionMap->UpdatePos(0, posInfo);

    float space = 10.0f;
    int32_t prevEndIndex = 5; // Non-existent
    bool groupAtEnd = true;

    // Should not crash
    listPositionMap->UpdatePosMapEnd(prevEndIndex, space, groupAtEnd);

    auto result = listPositionMap->GetPositionInfo(0);
    EXPECT_EQ(result.mainPos, 0.0f);
}

/**
 * @tc.name: ListChildrenMainSizeGetChildSize001
 * @tc.desc: Test ListChildrenMainSize GetChildSize with rounding mode
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListChildrenMainSizeGetChildSize001, TestSize.Level1)
{
    std::vector<float> mainSize = { 100.5f, 200.3f, 150.7f };
    RefPtr<ListChildrenMainSize> listChildrenMainSize =
        AceType::MakeRefPtr<ListChildrenMainSize>(mainSize, 100.0f);

    listChildrenMainSize->SetIsRoundingMode();

    auto size = listChildrenMainSize->GetChildSize(0);
    EXPECT_NEAR(size, Round(100.5f), 0.001f);
}

/**
 * @tc.name: ListChildrenMainSizeGetChildSize002
 * @tc.desc: Test ListChildrenMainSize GetChildSize with invalid index
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListChildrenMainSizeGetChildSize002, TestSize.Level1)
{
    std::vector<float> mainSize = { 100.0f, 200.0f, 150.0f };
    RefPtr<ListChildrenMainSize> listChildrenMainSize =
        AceType::MakeRefPtr<ListChildrenMainSize>(mainSize, 50.0f);

    // Test with out-of-bounds index
    auto size = listChildrenMainSize->GetChildSize(10);
    EXPECT_EQ(size, 50.0f); // Should return default size
}

/**
 * @tc.name: ListChildrenMainSizeGetChildSize003
 * @tc.desc: Test ListChildrenMainSize GetChildSize with DEFAULT_SIZE value
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListChildrenMainSizeGetChildSize003, TestSize.Level1)
{
    std::vector<float> mainSize = { -1.0f, 200.0f, 150.0f };
    float defaultSize = 80.0f;
    RefPtr<ListChildrenMainSize> listChildrenMainSize =
        AceType::MakeRefPtr<ListChildrenMainSize>(mainSize, defaultSize);

    // Test with DEFAULT_SIZE (-1.0f)
    auto size = listChildrenMainSize->GetChildSize(0);
    EXPECT_EQ(size, 80.0f); // Should return default size
}

/**
 * @tc.name: ListChildrenMainSizeChangeData001
 * @tc.desc: Test ListChildrenMainSize ChangeData with deleteCount -1
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListChildrenMainSizeChangeData001, TestSize.Level1)
{
    RefPtr<ListChildrenMainSize> listChildrenMainSize =
        AceType::MakeRefPtr<ListChildrenMainSize>();
    listChildrenMainSize->ResizeChildrenSize(5);

    // Test resize with deleteCount -1
    std::vector<float> newChildrenSize;
    listChildrenMainSize->ChangeData(3, -1, newChildrenSize);

    auto size = listChildrenMainSize->GetChildSize(0);
    EXPECT_EQ(size, 0.0f); // Default size
}

/**
 * @tc.name: ListChildrenMainSizeChangeData002
 * @tc.desc: Test ListChildrenMainSize ChangeData with insert
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListChildrenMainSizeChangeData002, TestSize.Level1)
{
    std::vector<float> mainSize = { 100.0f, 200.0f, 150.0f };
    RefPtr<ListChildrenMainSize> listChildrenMainSize =
        AceType::MakeRefPtr<ListChildrenMainSize>(mainSize, 50.0f);

    // Insert new items
    std::vector<float> newChildrenSize = { 120.0f, 130.0f };
    listChildrenMainSize->ChangeData(1, 0, newChildrenSize);

    auto size = listChildrenMainSize->GetChildSize(1);
    EXPECT_EQ(size, 120.0f);
}

/**
 * @tc.name: ListChildrenMainSizeChangeData003
 * @tc.desc: Test ListChildrenMainSize ChangeData with delete more than insert
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListChildrenMainSizeChangeData003, TestSize.Level1)
{
    std::vector<float> mainSize = { 100.0f, 200.0f, 150.0f, 180.0f };
    RefPtr<ListChildrenMainSize> listChildrenMainSize =
        AceType::MakeRefPtr<ListChildrenMainSize>(mainSize, 50.0f);

    // Delete 3 items, insert 1
    std::vector<float> newChildrenSize = { 120.0f };
    listChildrenMainSize->ChangeData(1, 3, newChildrenSize);

    // Should have 2 items now
    auto size0 = listChildrenMainSize->GetChildSize(0);
    auto size1 = listChildrenMainSize->GetChildSize(1);
    EXPECT_EQ(size0, 100.0f);
    EXPECT_EQ(size1, 120.0f);
}

/**
 * @tc.name: ListPatternHandleTargetIndex001
 * @tc.desc: Test ListPattern HandleTargetIndex with isJump true
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPatternHandleTargetIndex001, TestSize.Level1)
{
    auto listPattern = AceType::MakeRefPtr<ListPattern>();
    listPattern->targetIndex_ = 5;

    bool result = listPattern->HandleTargetIndex(true);

    EXPECT_TRUE(result);
    EXPECT_TRUE(listPattern->targetIndex_.has_value());
}

/**
 * @tc.name: ListPatternHandleTargetIndex002
 * @tc.desc: Test ListPattern HandleTargetIndex with non-existent target
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPatternHandleTargetIndex002, TestSize.Level1)
{
    auto listPattern = AceType::MakeRefPtr<ListPattern>();
    listPattern->itemPosition_[0] = { 0, 0.0f, 100.0f, false };
    listPattern->itemPosition_[1] = { 1, 110.0f, 100.0f, false };
    listPattern->targetIndex_ = 5; // Non-existent

    bool result = listPattern->HandleTargetIndex(false);

    EXPECT_FALSE(result);
    EXPECT_FALSE(listPattern->targetIndex_.has_value());
}

/**
 * @tc.name: ListLayoutAlgorithmGetStartIndex001
 * @tc.desc: Test ListLayoutAlgorithm GetStartIndex with empty position
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListLayoutAlgorithmGetStartIndex001, TestSize.Level1)
{
    ListLayoutAlgorithm listLayoutAlgorithm(0);

    auto startIndex = listLayoutAlgorithm.GetStartIndex();

    EXPECT_EQ(startIndex, -1);
}

/**
 * @tc.name: ListLayoutAlgorithmGetEndIndex001
 * @tc.desc: Test ListLayoutAlgorithm GetEndIndex with empty position
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListLayoutAlgorithmGetEndIndex001, TestSize.Level1)
{
    ListLayoutAlgorithm listLayoutAlgorithm(0);

    auto endIndex = listLayoutAlgorithm.GetEndIndex();

    EXPECT_EQ(endIndex, -1);
}

/**
 * @tc.name: ListLayoutAlgorithmGetStartPosition001
 * @tc.desc: Test ListLayoutAlgorithm GetStartPosition with empty position
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListLayoutAlgorithmGetStartPosition001, TestSize.Level1)
{
    ListLayoutAlgorithm listLayoutAlgorithm(0);

    auto startPos = listLayoutAlgorithm.GetStartPosition();

    EXPECT_EQ(startPos, 0.0f);
}

/**
 * @tc.name: ListLayoutAlgorithmGetEndPosition001
 * @tc.desc: Test ListLayoutAlgorithm GetEndPosition with empty position
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListLayoutAlgorithmGetEndPosition001, TestSize.Level1)
{
    ListLayoutAlgorithm listLayoutAlgorithm(0);

    auto endPos = listLayoutAlgorithm.GetEndPosition();

    EXPECT_EQ(endPos, 0.0f);
}

/**
 * @tc.name: ListPositionMapGetRowStartIndex001
 * @tc.desc: Test ListPositionMap GetRowStartIndex with single lane
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapGetRowStartIndex001, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();

    ListPositionInfo posInfo1 = { 0.0f, 100.0f, false };
    ListPositionInfo posInfo2 = { 110.0f, 100.0f, false };
    ListPositionInfo posInfo3 = { 220.0f, 100.0f, false };
    listPositionMap->UpdatePos(0, posInfo1);
    listPositionMap->UpdatePos(1, posInfo2);
    listPositionMap->UpdatePos(2, posInfo3);
    listPositionMap->totalItemCount_ = 3;

    auto startIndex = listPositionMap->GetRowStartIndex(1);

    EXPECT_EQ(startIndex, 1);
}

/**
 * @tc.name: ListPositionMapGetRowStartIndex002
 * @tc.desc: Test ListPositionMap GetRowStartIndex with multi-lane
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapGetRowStartIndex002, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();

    // Two items on same row (same position)
    ListPositionInfo posInfo = { 0.0f, 100.0f, false };
    listPositionMap->UpdatePos(0, posInfo);
    listPositionMap->UpdatePos(1, posInfo);
    listPositionMap->totalItemCount_ = 2;

    auto startIndex = listPositionMap->GetRowStartIndex(1);

    EXPECT_EQ(startIndex, 0);
}

/**
 * @tc.name: ListPositionMapGetRowEndIndexAndHeight001
 * @tc.desc: Test ListPositionMap GetRowEndIndexAndHeight for last item
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapGetRowEndIndexAndHeight001, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();

    ListPositionInfo posInfo1 = { 0.0f, 100.0f, false };
    ListPositionInfo posInfo2 = { 110.0f, 100.0f, false };
    listPositionMap->UpdatePos(0, posInfo1);
    listPositionMap->UpdatePos(1, posInfo2);
    listPositionMap->totalItemCount_ = 2;
    listPositionMap->totalHeight_ = 210.0f;
    listPositionMap->space_ = 10.0f;
    listPositionMap->footerSize_ = 0.0f;

    auto [endIndex, height] = listPositionMap->GetRowEndIndexAndHeight(1);

    EXPECT_EQ(endIndex, 1);
    EXPECT_GT(height, 0.0f);
}

/**
 * @tc.name: ListPositionMapGetRowEndIndexAndHeight002
 * @tc.desc: Test ListPositionMap GetRowEndIndexAndHeight with footer
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapGetRowEndIndexAndHeight002, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();

    ListPositionInfo posInfo1 = { 0.0f, 100.0f, false };
    listPositionMap->UpdatePos(0, posInfo1);
    listPositionMap->totalItemCount_ = 1;
    listPositionMap->totalHeight_ = 150.0f;
    listPositionMap->footerSize_ = 50.0f;

    auto [endIndex, height] = listPositionMap->GetRowEndIndexAndHeight(0);

    EXPECT_EQ(endIndex, 0);
    EXPECT_EQ(height, 100.0f); // totalHeight - pos - footer
}

/**
 * @tc.name: ListPositionMapCheckPosMapUpdateRule001
 * @tc.desc: Test ListPositionMap CheckPosMapUpdateRule with NO_CHANGE
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapCheckPosMapUpdateRule001, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    listPositionMap->ClearDirty();

    auto result = listPositionMap->CheckPosMapUpdateRule();

    EXPECT_EQ(result, ListPosMapUpdate::NO_CHANGE);
}

/**
 * @tc.name: ListPositionMapCheckPosMapUpdateRule002
 * @tc.desc: Test ListPositionMap CheckPosMapUpdateRule with UPDATE_ALL_SIZE
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapCheckPosMapUpdateRule002, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    listPositionMap->MarkDirty(LIST_UPDATE_DEFAULT_SIZE);

    auto result = listPositionMap->CheckPosMapUpdateRule();

    EXPECT_EQ(result, ListPosMapUpdate::UPDATE_ALL_SIZE);
}

/**
 * @tc.name: ListPositionMapCheckPosMapUpdateRule003
 * @tc.desc: Test ListPositionMap CheckPosMapUpdateRule with RE_CALCULATE
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapCheckPosMapUpdateRule003, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();
    listPositionMap->MarkDirty(LIST_UPDATE_CHILD_SIZE);

    auto result = listPositionMap->CheckPosMapUpdateRule();

    EXPECT_EQ(result, ListPosMapUpdate::RE_CALCULATE);
}

/**
 * @tc.name: ListPositionMapUpdateTotalCount001
 * @tc.desc: Test ListPositionMap UpdateTotalCount
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapUpdateTotalCount001, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();

    // Add some positions
    ListPositionInfo posInfo = { 0.0f, 100.0f, false };
    listPositionMap->UpdatePos(0, posInfo);
    listPositionMap->UpdatePos(1, posInfo);
    listPositionMap->UpdatePos(2, posInfo);
    listPositionMap->totalItemCount_ = 3;

    // Reduce total count
    listPositionMap->UpdateTotalCount(2);

    EXPECT_EQ(listPositionMap->totalItemCount_, 2);
}

/**
 * @tc.name: ListPositionMapGetPositionInfo001
 * @tc.desc: Test ListPositionMap GetPositionInfo with non-existent index
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, ListPositionMapGetPositionInfo001, TestSize.Level1)
{
    RefPtr<ListPositionMap> listPositionMap = AceType::MakeRefPtr<ListPositionMap>();

    auto result = listPositionMap->GetPositionInfo(5);

    EXPECT_EQ(result.mainPos, -1.0f);
    EXPECT_EQ(result.mainSize, -1.0f);
}

namespace {
void ExpectFastScrollPositions(const ListPositionMap& positionMap, const std::vector<float>& positions)
{
    for (size_t index = 0; index < positions.size(); ++index) {
        SCOPED_TRACE(index);
        EXPECT_FLOAT_EQ(positionMap.GetPositionInfo(static_cast<int32_t>(index)).mainPos, positions[index]);
    }
}
} // namespace

/**
 * @tc.name: FastScrollEstimateConnectLeft001
 * @tc.desc: Extend a continuous range to the left using its existing position, including the intervening gap.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateConnectLeft001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(12);
    positionMap->UpdatePos(8, { 800.0f, 100.0f });
    positionMap->UpdatePos(9, { 900.0f, 100.0f });
    ASSERT_TRUE(positionMap->EstimatePositions({ 2, 3 }, 100.0f, 1, 0.0f));
    EXPECT_EQ(positionMap->posMap_.size(), 8);
    for (int32_t index = 2; index <= 9; ++index) {
        EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(index).mainPos, index * 100.0f);
    }
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(1).mainSize, -1.0f);
}

/**
 * @tc.name: FastScrollEstimateConnectRight001
 * @tc.desc: Extend an existing range to the right without constructing a prefix from index zero.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateConnectRight001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(12);
    positionMap->UpdatePos(2, { 200.0f, 100.0f });
    positionMap->UpdatePos(3, { 300.0f, 100.0f });
    ASSERT_TRUE(positionMap->EstimatePositions({ 8, 9 }, 100.0f, 1, 0.0f));
    EXPECT_EQ(positionMap->posMap_.size(), 8);
    for (int32_t index = 2; index <= 9; ++index) {
        EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(index).mainPos, index * 100.0f);
    }
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(1).mainSize, -1.0f);
}

/**
 * @tc.name: FastScrollEstimateConnectPreparedSize001
 * @tc.desc: A prepared size without a position is connected to the valid range, not used as its origin.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateConnectPreparedSize001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(8);
    positionMap->UpdatePos(2, { 220.0f, 100.0f });
    positionMap->UpdatePos(6, { -1.0f, 250.0f, true, true });
    ASSERT_TRUE(positionMap->EstimatePositions({ 6, 7 }, 100.0f, 1, 10.0f));
    EXPECT_EQ(positionMap->posMap_.size(), 6);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(6).mainPos, 660.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(7).mainPos, 920.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(6).mainSize, 250.0f);
    EXPECT_TRUE(positionMap->GetPositionInfo(6).isGroup);
    EXPECT_TRUE(positionMap->GetPositionInfo(6).isLazyChild);
    for (int32_t index = 2; index < 6; ++index) {
        EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(index).mainPos, index * 110.0f);
    }
}

/**
 * @tc.name: FastScrollEstimateAroundIterator001
 * @tc.desc: Fill both sides of an interior reference, preserving unequal sizes and joining the suffix.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateAroundIterator001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(10);
    positionMap->UpdatePos(2, { -1.0f, 50.0f, false, true });
    positionMap->UpdatePos(4, { 500.0f, 200.0f, true, true });
    positionMap->UpdatePos(6, { -1.0f, 80.0f });
    positionMap->UpdatePos(8, { 1100.0f, 90.0f });
    ASSERT_TRUE(positionMap->EstimatePositions({ 2, 7 }, 100.0f, 1, 10.0f));
    const std::vector<float> positions { 330.0f, 390.0f, 500.0f, 710.0f, 820.0f, 910.0f, 1020.0f };
    for (size_t index = 0; index < positions.size(); ++index) {
        SCOPED_TRACE(index);
        EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(index + 2).mainPos, positions[index]);
    }
    EXPECT_EQ(positionMap->posMap_.size(), positions.size());
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(1).mainSize, -1.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(2).mainSize, 50.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(4).mainSize, 200.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(6).mainSize, 80.0f);
    EXPECT_TRUE(positionMap->GetPositionInfo(2).isLazyChild);
    EXPECT_TRUE(positionMap->GetPositionInfo(4).isGroup);
    EXPECT_TRUE(positionMap->GetPositionInfo(4).isLazyChild);
}

/**
 * @tc.name: FastScrollEstimateExplicitOrigin001
 * @tc.desc: A range beginning at zero uses the list origin rather than the stored reference position.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateExplicitOrigin001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(5);
    positionMap->UpdatePos(0, { 40.0f, 50.0f });
    positionMap->UpdatePos(2, { -1.0f, 200.0f, true });
    ASSERT_TRUE(positionMap->EstimatePositions({ 0, 3 }, 100.0f, 1, 10.0f));
    ExpectFastScrollPositions(*positionMap, { 0.0f, 60.0f, 170.0f, 380.0f });
    EXPECT_EQ(positionMap->posMap_.size(), 4);
    EXPECT_TRUE(positionMap->GetPositionInfo(2).isGroup);
}

class ListPositionMapEstimateValidationTestNg : public ListPositionMapTestNg,
    public testing::WithParamInterface<std::pair<int32_t, float>> {};

/**
 * @tc.name: FastScrollEstimateRejectsBeforeWrite001
 * @tc.desc: Invalid sizes on either side of a reference fail without inserting or modifying entries.
 * @tc.type: FUNC
 */
HWTEST_P(ListPositionMapEstimateValidationTestNg, FastScrollEstimateRejectsBeforeWrite001, TestSize.Level1)
{
    const auto [badIndex, badSize] = GetParam();
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(10);
    positionMap->UpdatePos(4, { 400.0f, 100.0f, true, true });
    positionMap->UpdatePos(badIndex, { -1.0f, badSize, false, true });
    const float sampleSize = std::isfinite(badSize) ? badSize : 100.0f;
    EXPECT_FALSE(positionMap->EstimatePositions({ 2, 7 }, sampleSize, 1, 10.0f));
    EXPECT_EQ(positionMap->posMap_.size(), 2);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(3).mainSize, -1.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(6).mainSize, -1.0f);
    const auto reference = positionMap->GetPositionInfo(4);
    EXPECT_FLOAT_EQ(reference.mainPos, 400.0f);
    EXPECT_FLOAT_EQ(reference.mainSize, 100.0f);
    EXPECT_TRUE(reference.isGroup);
    EXPECT_TRUE(reference.isLazyChild);
    const auto invalid = positionMap->GetPositionInfo(badIndex);
    EXPECT_FLOAT_EQ(invalid.mainPos, -1.0f);
    EXPECT_TRUE(invalid.isLazyChild);
    if (std::isnan(badSize)) {
        EXPECT_TRUE(std::isnan(invalid.mainSize));
    } else {
        EXPECT_EQ(invalid.mainSize, badSize);
    }
}

INSTANTIATE_TEST_SUITE_P(FastScrollEstimateValidation, ListPositionMapEstimateValidationTestNg,
    testing::Values(std::make_pair(2, std::numeric_limits<float>::quiet_NaN()),
        std::make_pair(5, std::numeric_limits<float>::quiet_NaN()),
        std::make_pair(2, std::numeric_limits<float>::infinity()),
        std::make_pair(5, std::numeric_limits<float>::infinity()),
        std::make_pair(2, std::numeric_limits<float>::max()),
        std::make_pair(5, std::numeric_limits<float>::max())));

/**
 * @tc.name: FastScrollEstimateOriginRejectsBeforeWrite001
 * @tc.desc: Rechecking an expanded origin prefix must reject invalid sizes without changing the map.
 * @tc.type: FUNC
 */
HWTEST_P(ListPositionMapEstimateValidationTestNg, FastScrollEstimateOriginRejectsBeforeWrite001, TestSize.Level1)
{
    const auto [badIndex, badSize] = GetParam();
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(9);
    positionMap->UpdatePos(0, { 0.0f, 100.0f });
    positionMap->UpdatePos(7, { 50.0f, 100.0f, true, true });
    positionMap->UpdatePos(badIndex, { -1.0f, badSize, false, true });
    const float sampleSize = std::isfinite(badSize) ? badSize : 100.0f;
    EXPECT_FALSE(positionMap->EstimatePositions({ 6, 7 }, sampleSize, 1, 0.0f));
    EXPECT_EQ(positionMap->posMap_.size(), 3);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(6).mainSize, -1.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(0).mainPos, 0.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(0).mainSize, 100.0f);
    const auto reference = positionMap->GetPositionInfo(7);
    EXPECT_FLOAT_EQ(reference.mainPos, 50.0f);
    EXPECT_FLOAT_EQ(reference.mainSize, 100.0f);
    EXPECT_TRUE(reference.isGroup);
    EXPECT_TRUE(reference.isLazyChild);
    const auto invalid = positionMap->GetPositionInfo(badIndex);
    EXPECT_FLOAT_EQ(invalid.mainPos, -1.0f);
    EXPECT_TRUE(invalid.isLazyChild);
    if (std::isnan(badSize)) {
        EXPECT_TRUE(std::isnan(invalid.mainSize));
    } else {
        EXPECT_EQ(invalid.mainSize, badSize);
    }
}

/**
 * @tc.name: FastScrollEstimateLargeSuffixKeepsReference001
 * @tc.desc: An extremely large suffix must not lose small offsets before an interior reference.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateLargeSuffixKeepsReference001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(6);
    positionMap->UpdatePos(2, { 250.0f, 25.0f, true, true });
    positionMap->UpdatePos(4, { -1.0f, 1.0e30f });
    ASSERT_TRUE(positionMap->EstimatePositions({ 1, 5 }, 50.0f, 1, 5.0f));
    EXPECT_EQ(positionMap->posMap_.size(), 5);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(1).mainPos, 195.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(2).mainPos, 250.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(3).mainPos, 280.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(4).mainPos, 335.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(5).mainPos, 1.0e30f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(2).mainSize, 25.0f);
    EXPECT_TRUE(positionMap->GetPositionInfo(2).isGroup);
    EXPECT_TRUE(positionMap->GetPositionInfo(2).isLazyChild);
}

/**
 * @tc.name: FastScrollEstimateShiftRejectsBeforeWrite001
 * @tc.desc: Overflow while rebasing an unrelated valid position leaves the entire map unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateShiftRejectsBeforeWrite001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(10);
    positionMap->UpdatePos(4, { 1.0e38f, 1.0e38f });
    positionMap->UpdatePos(8, { std::numeric_limits<float>::max(), 100.0f, true, true });
    EXPECT_FALSE(positionMap->EstimatePositions({ 1, 4 }, 1.0e38f, 1, 0.0f));
    EXPECT_EQ(positionMap->posMap_.size(), 2);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(1).mainSize, -1.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(4).mainPos, 1.0e38f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(8).mainPos, std::numeric_limits<float>::max());
    EXPECT_TRUE(positionMap->GetPositionInfo(8).isGroup);
    EXPECT_TRUE(positionMap->GetPositionInfo(8).isLazyChild);
}

/**
 * @tc.name: FastScrollEstimateEmptyNonzeroRange001
 * @tc.desc: Without a valid position, fill the prefix from the list origin rather than assigning zero to the target.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateEmptyNonzeroRange001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(8);
    ASSERT_TRUE(positionMap->EstimatePositions({ 4, 6 }, 100.0f, 1, 10.0f));
    EXPECT_EQ(positionMap->posMap_.size(), 7);
    ExpectFastScrollPositions(*positionMap, { 0.0f, 110.0f, 220.0f, 330.0f, 440.0f, 550.0f, 660.0f });
}

/**
 * @tc.name: FastScrollEstimateClosestValidRange001
 * @tc.desc: Prefer a valid position within the requested range over a distant prefix across an existing hole.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateClosestValidRange001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(12);
    positionMap->UpdatePos(0, { 0.0f, 100.0f });
    positionMap->UpdatePos(9, { 900.0f, 100.0f });
    ASSERT_TRUE(positionMap->EstimatePositions({ 8, 10 }, 100.0f, 1, 0.0f));
    EXPECT_EQ(positionMap->posMap_.size(), 4);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(8).mainPos, 800.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(9).mainPos, 900.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(10).mainPos, 1000.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(7).mainSize, -1.0f);
}

/**
 * @tc.name: FastScrollEstimateBackwardKeepsOrigin001
 * @tc.desc: When reverse estimation crosses zero, rebuild from the origin rather than rebasing index zero.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateBackwardKeepsOrigin001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(6);
    positionMap->UpdatePos(0, { 0.0f, 100.0f });
    positionMap->UpdatePos(4, { 50.0f, 100.0f });
    ASSERT_TRUE(positionMap->EstimatePositions({ 3, 4 }, 100.0f, 1, 0.0f));
    ExpectFastScrollPositions(*positionMap, { 0.0f, 100.0f, 200.0f, 300.0f, 400.0f });
    EXPECT_EQ(positionMap->posMap_.size(), 5);
}

/**
 * @tc.name: FastScrollEstimateInvalidPositionOrigin001
 * @tc.desc: Size-only records cannot provide an anchor, but their measured sizes survive origin estimation.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateInvalidPositionOrigin001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(6);
    positionMap->UpdatePos(2, { -1.0f, 250.0f });
    ASSERT_TRUE(positionMap->EstimatePositions({ 2, 4 }, 100.0f, 1, 0.0f));
    ExpectFastScrollPositions(*positionMap, { 0.0f, 100.0f, 200.0f, 450.0f, 550.0f });
    EXPECT_EQ(positionMap->posMap_.size(), 5);
}

/**
 * @tc.name: FastScrollConfirmedRowsPreserveRowMaximum001
 * @tc.desc: Confirmed rows store the measured row maximum and retain the partial last row.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollConfirmedRowsPreserveRowMaximum001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(7);
    positionMap->UpdatePos(0, { 0.0f, 80.0f });
    positionMap->UpdatePos(1, { 0.0f, 120.0f });
    const std::map<int32_t, int32_t> itemRanges = { { 0, 6 } };
    ASSERT_TRUE(positionMap->EstimatePositions({ 0, 6 }, 100.0f, 2, 10.0f, itemRanges));

    ExpectFastScrollPositions(*positionMap, { 0.0f, 0.0f, 130.0f, 130.0f, 240.0f, 240.0f, 350.0f });
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(0).mainSize, 120.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(1).mainSize, 120.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(6).mainSize, 100.0f);
    EXPECT_EQ(positionMap->posMap_.size(), 7);
}

/**
 * @tc.name: FastScrollEstimateGroup001
 * @tc.desc: A group ends a partial row and restarts lane counting without changing its size or flags.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateGroup001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(8);
    positionMap->UpdatePos(0, { 0.0f, 80.0f });
    positionMap->UpdatePos(1, { 90.0f, 350.0f, true });
    positionMap->UpdatePos(4, { 560.0f, 500.0f, true });
    const std::map<int32_t, int32_t> itemRanges = { { 0, 0 }, { 1, 1 }, { 2, 3 }, { 4, 4 }, { 5, 7 } };
    ASSERT_TRUE(positionMap->EstimatePositions({ 0, 7 }, 100.0f, 3, 10.0f, itemRanges));

    ExpectFastScrollPositions(*positionMap, { 0.0f, 90.0f, 450.0f, 450.0f, 560.0f, 1070.0f, 1070.0f, 1070.0f });
    EXPECT_TRUE(positionMap->GetPositionInfo(1).isGroup);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(1).mainSize, 350.0f);
    EXPECT_TRUE(positionMap->GetPositionInfo(4).isGroup);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(4).mainSize, 500.0f);
}

/**
 * @tc.name: FastScrollEstimateSparse001
 * @tc.desc: Connect the requested range to a known lazy child while preserving its measured size.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateSparse001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(8);
    positionMap->UpdatePos(5, { 900.0f, 300.0f, false, true });
    ASSERT_TRUE(positionMap->EstimatePositions({ 0, 2 }, 100.0f, 1, 10.0f));

    ExpectFastScrollPositions(*positionMap, { 0.0f, 110.0f, 220.0f, 330.0f, 440.0f, 550.0f });
    EXPECT_EQ(positionMap->posMap_.size(), 6);
    EXPECT_TRUE(positionMap->GetPositionInfo(5).isLazyChild);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(5).mainPos, 550.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(5).mainSize, 300.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(3).mainSize, 100.0f);
}

/**
 * @tc.name: FastScrollEstimateBackwardAnchor001
 * @tc.desc: Backward estimation preserves the visible anchor and measured target size without creating a prefix.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateBackwardAnchor001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(8);
    positionMap->UpdatePos(0, { 0.0f, 100.0f });
    positionMap->UpdatePos(2, { -1.0f, 250.0f });
    positionMap->UpdatePos(6, { 700.0f, 70.0f });
    ASSERT_TRUE(positionMap->EstimatePositions({ 2, 6 }, 100.0f, 1, 10.0f));

    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(2).mainPos, 110.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(2).mainSize, 250.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(3).mainPos, 370.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(6).mainPos, 700.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(0).mainPos, 0.0f);
    EXPECT_EQ(positionMap->GetPositionInfo(1).mainSize, -1.0f);
    EXPECT_EQ(positionMap->posMap_.size(), 6);
}

/**
 * @tc.name: FastScrollEstimateRebaseBackward001
 * @tc.desc: A backward range may rebase sparse coordinates but never writes negative valid positions.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateRebaseBackward001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(5);
    positionMap->UpdatePos(4, { 100.0f, 100.0f });
    ASSERT_TRUE(positionMap->EstimatePositions({ 1, 4 }, 100.0f, 1, 0.0f));
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(1).mainPos, 0.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(4).mainPos, 300.0f);
    EXPECT_EQ(positionMap->GetPositionInfo(0).mainSize, -1.0f);
}

/**
 * @tc.name: FastScrollEstimateKeepsOrigin001
 * @tc.desc: A shorter measured prefix keeps index zero at the origin and rebases the visible anchor.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateKeepsOrigin001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(7);
    positionMap->UpdatePos(0, { 0.0f, 100.0f });
    positionMap->UpdatePos(2, { -1.0f, 50.0f });
    positionMap->UpdatePos(6, { 600.0f, 100.0f });
    ASSERT_TRUE(positionMap->EstimatePositions({ 0, 6 }, 100.0f, 1, 0.0f));

    ExpectFastScrollPositions(*positionMap, { 0.0f, 100.0f, 200.0f, 250.0f, 350.0f, 450.0f, 550.0f });
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(2).mainSize, 50.0f);
}

/**
 * @tc.name: FastScrollEstimateJoinsAdjacentRanges001
 * @tc.desc: A measured range joins its existing predecessor and shifts the existing successor consistently.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateJoinsAdjacentRanges001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(8);
    for (int32_t index = 0; index < 8; ++index) {
        positionMap->UpdatePos(index, { index * 110.0f, 100.0f });
    }
    positionMap->UpdatePos(2, { 220.0f, 50.0f });
    ASSERT_TRUE(positionMap->EstimatePositions({ 2, 6 }, 100.0f, 1, 10.0f));

    ExpectFastScrollPositions(*positionMap, { 0.0f, 110.0f, 220.0f, 280.0f, 390.0f, 500.0f, 610.0f, 720.0f });
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(2).mainSize, 50.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(7).mainSize, 100.0f);
}

/**
 * @tc.name: FastScrollEstimateInvalid001
 * @tc.desc: Invalid estimation inputs do not change existing metadata.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateInvalid001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(2);
    positionMap->UpdatePos(1, { 123.0f, 80.0f, false, true });
    const auto infinity = std::numeric_limits<float>::infinity();
    const auto nan = std::numeric_limits<float>::quiet_NaN();
    const ListPositionMap::EstimateRange range { 0, 1 };
    EXPECT_FALSE(positionMap->EstimatePositions({ 0, -1 }, 100.0f, 1, 0.0f));
    EXPECT_FALSE(positionMap->EstimatePositions({ 0, 2 }, 100.0f, 1, 0.0f));
    EXPECT_FALSE(positionMap->EstimatePositions({ 1, 0 }, 100.0f, 1, 0.0f));
    EXPECT_FALSE(positionMap->EstimatePositions(range, -1.0f, 1, 0.0f));
    EXPECT_FALSE(positionMap->EstimatePositions(range, 100.0f, 0, 0.0f));
    EXPECT_FALSE(positionMap->EstimatePositions(range, 100.0f, 2, 0.0f));
    EXPECT_FALSE(positionMap->EstimatePositions(range, 100.0f, 1, -1.0f));
    EXPECT_FALSE(positionMap->EstimatePositions(range, infinity, 1, 0.0f));
    EXPECT_FALSE(positionMap->EstimatePositions(range, nan, 1, 0.0f));
    EXPECT_FALSE(positionMap->EstimatePositions(range, 100.0f, 1, infinity));
    EXPECT_FALSE(positionMap->EstimatePositions(range, 100.0f, 1, nan));
    EXPECT_EQ(positionMap->posMap_.size(), 1);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(1).mainPos, 123.0f);
    EXPECT_TRUE(positionMap->GetPositionInfo(1).isLazyChild);
}

/**
 * @tc.name: FastScrollEstimateKnownSize001
 * @tc.desc: Estimation must not overwrite ChildrenMainSize metadata.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateKnownSize001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(2);
    positionMap->UpdatePos(1, { 123.0f, 80.0f });
    positionMap->childrenSize_ = AceType::MakeRefPtr<ListChildrenMainSize>(std::vector<float> { 123.0f, 80.0f },
        100.0f);
    EXPECT_FALSE(positionMap->EstimatePositions({ 0, 1 }, 100.0f, 1, 0.0f));
    EXPECT_EQ(positionMap->posMap_.size(), 1);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(1).mainPos, 123.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(1).mainSize, 80.0f);
}

/**
 * @tc.name: FastScrollFindRow001
 * @tc.desc: Query uses row height, returns the first lane, and assigns a gap to the next row.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollFindRow001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdatePos(0, { 0.0f, 80.0f });
    positionMap->UpdatePos(1, { 0.0f, 120.0f });
    positionMap->UpdatePos(2, { 130.0f, 60.0f });
    positionMap->UpdatePos(3, { 130.0f, 100.0f });
    positionMap->UpdatePos(4, { 240.0f, 90.0f });
    const std::vector<std::pair<float, std::pair<int32_t, float>>> cases = {
        { -10.0f, { 0, -10.0f } }, { 0.0f, { 0, 0.0f } }, { 100.0f, { 0, 100.0f } },
        { 120.0f, { 2, -10.0f } }, { 125.0f, { 2, -5.0f } }, { 130.0f, { 2, 0.0f } },
        { 200.0f, { 2, 70.0f } }, { 230.0f, { 4, -10.0f } }, { 400.0f, { 4, 160.0f } }
    };
    for (const auto& [position, expected] : cases) {
        SCOPED_TRACE(position);
        auto [index, delta] = positionMap->FindRowAtPosition(position, 2);
        EXPECT_EQ(index, expected.first);
        EXPECT_FLOAT_EQ(delta, expected.second);
    }
    EXPECT_EQ(positionMap->posMap_.size(), 5);
}

/**
 * @tc.name: FastScrollFindRowFloatBoundary001
 * @tc.desc: Row-end lookup uses the framework tolerance and assigns an approximately equal end to the next row.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollFindRowFloatBoundary001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdatePos(0, { 0.0f, 100.0f });
    positionMap->UpdatePos(1, { 110.0f, 50.0f });
    const std::vector<std::pair<float, int32_t>> cases = {
        { 99.998f, 0 }, { 99.9995f, 1 }, { 100.0f, 1 }, { 100.0005f, 1 }, { 160.0f, 1 }
    };
    for (const auto& [position, expectedIndex] : cases) {
        SCOPED_TRACE(position);
        const auto [index, delta] = positionMap->FindRowAtPosition(position, 1);
        EXPECT_EQ(index, expectedIndex);
        EXPECT_FLOAT_EQ(delta, position - positionMap->GetPositionInfo(expectedIndex).mainPos);
    }
    EXPECT_EQ(positionMap->posMap_.size(), 2);
}

/**
 * @tc.name: FastScrollEstimateZeroPositionTolerance001
 * @tc.desc: A near-zero position remains a valid reference instead of rebuilding the prefix unnecessarily.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateZeroPositionTolerance001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(4);
    positionMap->UpdatePos(2, { -0.0005f, 100.0f, false, true });
    ASSERT_TRUE(positionMap->EstimatePositions({ 2, 3 }, 100.0f, 1, 0.0f));
    EXPECT_EQ(positionMap->posMap_.size(), 2);
    EXPECT_EQ(positionMap->posMap_.count(0), 0);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(2).mainPos, 0.0f);
    EXPECT_FLOAT_EQ(positionMap->GetPositionInfo(3).mainPos, 100.0f);
    EXPECT_TRUE(positionMap->GetPositionInfo(2).isLazyChild);
}

/**
 * @tc.name: FastScrollFindRowEmpty001
 * @tc.desc: Empty and non-finite queries do not insert records.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollFindRowEmpty001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    EXPECT_EQ(positionMap->FindRowAtPosition(0.0f, 1).first, -1);
    EXPECT_TRUE(positionMap->posMap_.empty());
    positionMap->UpdatePos(5, { 500.0f, 100.0f });
    EXPECT_EQ(positionMap->FindRowAtPosition(std::numeric_limits<float>::infinity(), 1).first, -1);
    EXPECT_EQ(positionMap->FindRowAtPosition(std::numeric_limits<float>::quiet_NaN(), 1).first, -1);
    EXPECT_EQ(positionMap->FindRowAtPosition(0.0f, 0).first, -1);
    EXPECT_EQ(positionMap->posMap_.size(), 1);
}

/**
 * @tc.name: FastScrollEstimateZeroSize001
 * @tc.desc: Zero-height rows and groups terminate estimation and lookup without dividing by zero.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollEstimateZeroSize001, TestSize.Level1)
{
    auto positionMap = AceType::MakeRefPtr<ListPositionMap>();
    positionMap->UpdateTotalCount(4);
    positionMap->UpdatePos(1, { 0.0f, 0.0f, true });
    const std::map<int32_t, int32_t> itemRanges = { { 0, 0 }, { 1, 1 }, { 2, 3 } };
    ASSERT_TRUE(positionMap->EstimatePositions({ 0, 3 }, 0.0f, 2, 0.0f, itemRanges));
    ExpectFastScrollPositions(*positionMap, { 0.0f, 0.0f, 0.0f, 0.0f });
    EXPECT_EQ(positionMap->FindRowAtPosition(0.0f, 2).first, 2);
    positionMap->UpdatePos(1, { 0.0f, 0.0f });
    EXPECT_EQ(positionMap->FindRowAtPosition(0.0f, 2).first, 2);
    EXPECT_EQ(positionMap->FindRowAtPosition(0.0f, 1).first, 3);
    EXPECT_EQ(positionMap->posMap_.size(), 4);
}
/**
 * @tc.name: FastScrollMirroredPosMapKeepsMixedRows001
 * @tc.desc: A mirrored view preserves sizes and types without changing the shared table, including a partial row.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollMirroredPosMapKeepsMixedRows001, TestSize.Level1)
{
    ListPositionMap positions;
    positions.totalItemCount_ = 6;
    positions.totalHeight_ = 315.0f;
    positions.posMap_ = {
        { 0, { 0.0f, 10.0f } }, { 1, { 0.0f, 10.0f } }, { 2, { 0.0f, 10.0f } },
        { 3, { 15.0f, 200.0f, true } }, { 4, { 220.0f, 95.0f, false, true } }, { 5, { 220.0f, 95.0f } }
    };
    const auto original = positions.posMap_;
    const auto mirrored = positions.CreateFastScrollMirroredPosMap();
    EXPECT_FLOAT_EQ(mirrored->GetPositionInfo(0).mainPos, 0.0f);
    EXPECT_FLOAT_EQ(mirrored->GetPositionInfo(1).mainSize, 95.0f);
    EXPECT_TRUE(mirrored->GetPositionInfo(1).isLazyChild);
    EXPECT_FLOAT_EQ(mirrored->GetPositionInfo(2).mainPos, 100.0f);
    EXPECT_FLOAT_EQ(mirrored->GetPositionInfo(2).mainSize, 200.0f);
    EXPECT_TRUE(mirrored->GetPositionInfo(2).isGroup);
    EXPECT_FLOAT_EQ(mirrored->GetPositionInfo(3).mainPos, 305.0f);
    EXPECT_FLOAT_EQ(mirrored->GetTotalHeight(), 315.0f);
    const auto restored = mirrored->CreateFastScrollMirroredPosMap();
    for (const auto& [index, info] : original) {
        EXPECT_FLOAT_EQ(positions.GetPositionInfo(index).mainPos, info.mainPos);
        EXPECT_FLOAT_EQ(positions.GetPositionInfo(index).mainSize, info.mainSize);
        EXPECT_EQ(positions.GetPositionInfo(index).isGroup, info.isGroup);
        EXPECT_EQ(positions.GetPositionInfo(index).isLazyChild, info.isLazyChild);
        EXPECT_FLOAT_EQ(restored->GetPositionInfo(index).mainPos, info.mainPos);
        EXPECT_FLOAT_EQ(restored->GetPositionInfo(index).mainSize, info.mainSize);
        EXPECT_EQ(restored->GetPositionInfo(index).isGroup, info.isGroup);
        EXPECT_EQ(restored->GetPositionInfo(index).isLazyChild, info.isLazyChild);
    }
}

/**
 * @tc.name: FastScrollConfirmedRowsRejectPositionOverflow001
 * @tc.desc: A finite row length plus a large anchor must not insert infinite coordinates or mutate the map.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollConfirmedRowsRejectPositionOverflow001, TestSize.Level1)
{
    ListPositionMap positions;
    positions.UpdateTotalCount(4);
    const std::map<int32_t, int32_t> itemRanges = { { 0, 3 } };
    const float maximum = std::numeric_limits<float>::max();
    positions.UpdatePos(0, { maximum * 0.75f, maximum * 0.5f });
    EXPECT_FALSE(positions.EstimatePositions({ 0, 3 }, maximum * 0.5f, 2, 0.0f, itemRanges, false));
    EXPECT_EQ(positions.posMap_.size(), 1u);
    EXPECT_FLOAT_EQ(positions.GetPositionInfo(0).mainPos, maximum * 0.75f);
    EXPECT_FLOAT_EQ(positions.GetPositionInfo(0).mainSize, maximum * 0.5f);
}

/**
 * @tc.name: FastScrollMirroredViewLeavesLegacyReverseUnchanged001
 * @tc.desc: Creating a FastScroll view cannot change either of the subsequent ordinary reversal passes.
 * @tc.type: FUNC
 */
HWTEST_F(ListPositionMapTestNg, FastScrollMirroredViewLeavesLegacyReverseUnchanged001, TestSize.Level1)
{
    ListPositionMap positions;
    ListPositionMap control;
    positions.UpdateTotalCount(6);
    positions.totalHeight_ = 360.0f;
    positions.space_ = 5.0f;
    for (int32_t index = 0; index < 6; ++index) {
        const float start = index < 2 ? 0.0f : (index < 4 ? 55.0f : 160.0f);
        const float size = index < 2 ? 50.0f : (index < 4 ? 100.0f : 200.0f);
        positions.UpdatePos(index, { start, size });
    }
    control.posMap_ = positions.posMap_;
    control.totalItemCount_ = 6;
    control.totalHeight_ = positions.totalHeight_;
    control.space_ = positions.space_;
    for (int32_t pass = 0; pass < 2; ++pass) {
        const auto mirrored = positions.CreateFastScrollMirroredPosMap();
        EXPECT_FLOAT_EQ(mirrored->GetPositionInfo(0).mainSize, 200.0f);
        positions.ReversePosMap();
        control.ReversePosMap();
        EXPECT_FLOAT_EQ(positions.GetTotalHeight(), control.GetTotalHeight());
        for (const auto& [index, info] : control.posMap_) {
            EXPECT_FLOAT_EQ(positions.GetPositionInfo(index).mainPos, info.mainPos);
            EXPECT_FLOAT_EQ(positions.GetPositionInfo(index).mainSize, info.mainSize);
        }
    }
    positions.ClearPosMap();
    EXPECT_TRUE(positions.posMap_.empty());
}

} // namespace OHOS::Ace::NG
