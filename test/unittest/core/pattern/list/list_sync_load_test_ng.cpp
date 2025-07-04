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

#include "list_test_ng.h"
#include "test/mock/core/animation/mock_animation_manager.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"

namespace OHOS::Ace::NG {
class ListSyncLoadTestNg : public ListTestNg {
public:
    void TearDown() override;
    ListItemModelNG CreateListItem();
    void CreateListItems(int32_t itemNumber);
};

void ListSyncLoadTestNg::TearDown()
{
    ListTestNg::TearDown();
    MockPipelineContext::GetCurrent()->SetResponseTime(INT32_MAX);
}

ListItemModelNG ListSyncLoadTestNg::CreateListItem()
{
    ViewStackProcessor::GetInstance()->StartGetAccessRecordingFor(GetElmtId());
    ListItemModelNG itemModel;
    itemModel.Create([](int32_t) {}, V2::ListItemStyle::NONE);
    Axis axis = layoutProperty_->GetListDirection().value_or(Axis::VERTICAL);
    SetSize(axis, CalcLength(FILL_LENGTH), CalcLength(ITEM_MAIN_SIZE));
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->GetMainElementNode();
    auto frameNode = AceType::DynamicCast<FrameNode>(element);
    frameNode->measureCallback_ = [](RefPtr<Kit::FrameNode>& node) {
        NG::MockPipelineContext::GetCurrent()->DecResponseTime();
    };
    return itemModel;
}

void ListSyncLoadTestNg::CreateListItems(int32_t itemNumber)
{
    for (int32_t index = 0; index < itemNumber; index++) {
        CreateListItem();
        ViewStackProcessor::GetInstance()->Pop();
        ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    }
}

/**
 * @tc.name: SyncLoad001
 * @tc.desc: Test List sync load
 * @tc.type: FUNC
 */
HWTEST_F(ListSyncLoadTestNg, SyncLoad001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List
     * @tc.expected: List load 2 item in first frame
     */
    ListModelNG model = CreateList();
    CreateListItems(10);
    model.SetSyncLoad(false);
    MockPipelineContext::GetCurrent()->SetResponseTime(2);
    CreateDone();
    EXPECT_EQ(pattern_->itemPosition_.size(), 2);
    EXPECT_TRUE(pattern_->prevMeasureBreak_);

    /**
     * @tc.steps: step1. Flush next frame
     * @tc.expected: List load 2 item in 2th frame
     */
    MockPipelineContext::GetCurrent()->SetResponseTime(2);
    FlushUITasks(frameNode_);
    EXPECT_EQ(pattern_->itemPosition_.size(), 4);
    EXPECT_FALSE(pattern_->prevMeasureBreak_);
}

/**
 * @tc.name: SyncLoad002
 * @tc.desc: Test List sync load, jump to index with end align
 * @tc.type: FUNC
 */
HWTEST_F(ListSyncLoadTestNg, SyncLoad002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List, jump to index with end align
     * @tc.expected: List load 2 item in first frame
     */
    ListModelNG model = CreateList();
    CreateListItems(10);
    model.SetSyncLoad(false);
    MockPipelineContext::GetCurrent()->SetResponseTime(2);
    pattern_->jumpIndex_ = 0;
    pattern_->scrollAlign_ = ScrollAlign::END;
    CreateDone();
    EXPECT_EQ(pattern_->itemPosition_.size(), 2);
    EXPECT_TRUE(pattern_->prevMeasureBreak_);

    /**
     * @tc.steps: step1. Flush next frame
     * @tc.expected: List load 2 item in 2th frame
     */
    MockPipelineContext::GetCurrent()->SetResponseTime(2);
    FlushUITasks(frameNode_);
    EXPECT_EQ(pattern_->itemPosition_.size(), 4);
    EXPECT_FALSE(pattern_->prevMeasureBreak_);
}

/**
 * @tc.name: SyncLoad003
 * @tc.desc: Create List without height, List not sync load
 * @tc.type: FUNC
 */
HWTEST_F(ListSyncLoadTestNg, SyncLoad003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List without height
     * @tc.expected: List not sync load
     */
    ListModelNG model = CreateList();
    CreateListItems(6);
    model.SetSyncLoad(false);
    layoutProperty_->ClearUserDefinedIdealSize(false, true);
    MockPipelineContext::GetCurrent()->SetResponseTime(2);
    CreateDone();
    EXPECT_EQ(pattern_->itemPosition_.size(), 6);
    EXPECT_FALSE(pattern_->prevMeasureBreak_);
}

/**
 * @tc.name: SyncLoad004
 * @tc.desc: Test List with out sync load
 * @tc.type: FUNC
 */
HWTEST_F(ListSyncLoadTestNg, SyncLoad004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List
     * @tc.expected: List not sync load
     */
    ListModelNG model = CreateList();
    model.SetSyncLoad(true);
    CreateListItems(10);
    MockPipelineContext::GetCurrent()->SetResponseTime(2);
    CreateDone();
    EXPECT_EQ(pattern_->itemPosition_.size(), 4);
    EXPECT_FALSE(pattern_->prevMeasureBreak_);
}

/**
 * @tc.name: SyncLoad005
 * @tc.desc: Test List async load with inactive
 * @tc.type: FUNC
 */
HWTEST_F(ListSyncLoadTestNg, SyncLoad005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List
     * @tc.expected: List async load
     */
    ListModelNG model = CreateList();
    model.SetSyncLoad(false);
    CreateListItems(10);
    RefPtr<UINode> element = ViewStackProcessor::GetInstance()->Finish();
    ViewStackProcessor::GetInstance()->StopGetAccessRecording();
    EXPECT_FALSE(frameNode_->IsActive());

    /**
     * @tc.steps: step2. Layout List
     * @tc.expected: List async load, item2 inactive
     */
    MockPipelineContext::GetCurrent()->SetResponseTime(2);
    frameNode_->SetLayoutDirtyMarked(true);
    frameNode_->CreateLayoutTask();
    IsExistAndActive(frameNode_, 1);
    IsExistAndInActive(frameNode_, 2);
    EXPECT_FALSE(frameNode_->IsActive());
    frameNode_->SetLayoutDirtyMarked(true);

    /**
     * @tc.steps: step3. Layout List 2th frame
     * @tc.expected: List async load, item3 is active
     */
    MockPipelineContext::GetCurrent()->SetResponseTime(2);
    frameNode_->CreateLayoutTask();
    IsExistAndActive(frameNode_, 3);
    EXPECT_FALSE(frameNode_->IsActive());
}

/**
 * @tc.name: GroupSyncLoad001
 * @tc.desc: Test ListItemGroup sync load
 * @tc.type: FUNC
 */
HWTEST_F(ListSyncLoadTestNg, GroupSyncLoad001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List
     * @tc.expected: ListItemGroup load 2 item in first frame
     */
    ListModelNG model = CreateList();
    model.SetSyncLoad(false);
    CreateListItemGroup(V2::ListItemGroupStyle::NONE);
    CreateListItems(10);
    MockPipelineContext::GetCurrent()->SetResponseTime(2);
    CreateDone();
    EXPECT_EQ(itemGroupPatters_[0]->itemPosition_.size(), 2);
    EXPECT_TRUE(pattern_->prevMeasureBreak_);

    /**
     * @tc.steps: step2. Flush next frame
     * @tc.expected: List load 3 item in 2th frame
     */
    MockPipelineContext::GetCurrent()->SetResponseTime(3);
    FlushUITasks(frameNode_);
    EXPECT_EQ(itemGroupPatters_[0]->itemPosition_.size(), 5);
    EXPECT_FALSE(pattern_->prevMeasureBreak_);
}

/**
 * @tc.name: GroupSyncLoad002
 * @tc.desc: Test List sync load
 * @tc.type: FUNC
 */
HWTEST_F(ListSyncLoadTestNg, GroupSyncLoad002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create List, and jump to item9 in group
     * @tc.expected: List load 2 item in first frame
     */
    ListModelNG model = CreateList();
    model.SetSyncLoad(false);
    CreateListItemGroup(V2::ListItemGroupStyle::NONE);
    CreateListItems(10);
    MockPipelineContext::GetCurrent()->SetResponseTime(3); // item9 will measure 2 times
    pattern_->jumpIndex_ = 0;
    pattern_->jumpIndexInGroup_ = 9;
    CreateDone();
    EXPECT_EQ(itemGroupPatters_[0]->itemPosition_.size(), 2);
    EXPECT_EQ(itemGroupPatters_[0]->itemPosition_.count(9), 1);
    EXPECT_EQ(itemGroupPatters_[0]->itemPosition_.count(8), 1);
    EXPECT_TRUE(pattern_->prevMeasureBreak_);

    /**
     * @tc.steps: step2. Flush next frame
     * @tc.expected: List load 3 item in 2th frame
     */
    MockPipelineContext::GetCurrent()->SetResponseTime(3);
    FlushUITasks(frameNode_);
    EXPECT_EQ(itemGroupPatters_[0]->itemPosition_.size(), 5);
    EXPECT_FALSE(pattern_->prevMeasureBreak_);
}
}
