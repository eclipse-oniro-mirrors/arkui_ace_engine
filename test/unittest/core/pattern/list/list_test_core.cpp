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

#include "gtest/gtest.h"

#include <memory>
#include <vector>

#define protected public
#define private public

#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/animation/animator.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/list/list_item_event_hub.h"
#include "core/components_ng/pattern/list/list_item_layout_property.h"
#include "core/components_ng/pattern/list/list_item_pattern.h"
#include "core/components_ng/pattern/list/list_layout_property.h"
#include "core/components_ng/pattern/list/list_model_ng.h"
#include "core/components_ng/pattern/list/list_pattern.h"
#include "core/components_ng/pattern/root/root_pattern.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {

class ListTestCore : public testing::Test {
public:
    static void SetUpTestCase()
    {
        MockPipelineContext::SetUp();
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    }

    static void TearDownTestCase()
    {
        MockPipelineContext::TearDown();
    }

    void TearDown() override
    {
        if (itemPattern_ && itemPattern_->springController_) {
            itemPattern_->springController_->ClearStopListeners();
            itemPattern_->springController_->Stop();
        }
        itemPattern_.Reset();
        item_.Reset();
        listPattern_.Reset();
        list_.Reset();
    }

    void CreateSwipeItem(bool hasStart = false, bool hasEnd = true)
    {
        auto stack = ViewStackProcessor::GetInstance();
        listPattern_ = AceType::MakeRefPtr<ListPattern>();
        list_ = FrameNode::CreateFrameNode(V2::LIST_ETS_TAG, stack->ClaimNodeId(), listPattern_);
        itemPattern_ = AceType::MakeRefPtr<ListItemPattern>(nullptr);
        item_ = FrameNode::CreateFrameNode(V2::LIST_ITEM_ETS_TAG, stack->ClaimNodeId(), itemPattern_);
        item_->MountToParent(list_);
        auto child = FrameNode::CreateFrameNode(V2::ROW_ETS_TAG, stack->ClaimNodeId(), AceType::MakeRefPtr<Pattern>());
        child->MountToParent(item_);
        if (hasStart) {
            auto start = FrameNode::CreateFrameNode(
                V2::ROW_ETS_TAG, stack->ClaimNodeId(), AceType::MakeRefPtr<Pattern>());
            itemPattern_->SetStartNode(start);
        }
        if (hasEnd) {
            auto end = FrameNode::CreateFrameNode(
                V2::ROW_ETS_TAG, stack->ClaimNodeId(), AceType::MakeRefPtr<Pattern>());
            itemPattern_->SetEndNode(end);
        }
        item_->GetGeometryNode()->SetFrameSize(SizeF(240.0f, 64.0f));
        item_->GetLayoutProperty()->UpdateLayoutDirection(TextDirection::LTR);
        itemPattern_->axis_ = Axis::VERTICAL;
        itemPattern_->InitSwiperAction(false);
        CleanItemDirty();
    }

    void StartDrag()
    {
        GestureEvent info;
        itemPattern_->panEvent_->GetActionStartEventFunc()(info);
    }

    void UpdateDrag(float delta)
    {
        GestureEvent info;
        info.SetMainDelta(delta);
        itemPattern_->panEvent_->GetActionUpdateEventFunc()(info);
    }

    void CleanItemDirty()
    {
        item_->SetLayoutDirtyMarked(false);
        item_->GetLayoutProperty()->CleanDirty();
    }

    RefPtr<FrameNode> list_;
    RefPtr<ListPattern> listPattern_;
    RefPtr<FrameNode> item_;
    RefPtr<ListItemPattern> itemPattern_;
};

/**
 * @tc.name: RecycleSwipeChildOffsetRight
 * @tc.desc: The empty start side can overdrag while ITEM_CHILD and must be reset on recycle.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeChildOffsetRight, TestSize.Level1)
{
    CreateSwipeItem();
    auto offsets = std::make_shared<std::vector<int32_t>>();
    itemPattern_->SetOffsetChangeCallBack([offsets](int32_t offset) { offsets->emplace_back(offset); });
    StartDrag();
    UpdateDrag(40.0f);
    ASSERT_EQ(itemPattern_->GetSwiperIndex(), ListItemSwipeIndex::ITEM_CHILD);
    ASSERT_GT(itemPattern_->curOffset_, 0.0f);
    ASSERT_TRUE(itemPattern_->isDragging_);
    offsets->clear();
    CleanItemDirty();

    itemPattern_->OnRecycle();

    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_EQ(itemPattern_->GetSwipeActionState(), SwipeActionState::COLLAPSED);
    EXPECT_EQ(*offsets, std::vector<int32_t>({ 0 }));
    EXPECT_TRUE(item_->IsLayoutDirtyMarked());
    EXPECT_NE(item_->GetLayoutProperty()->GetPropertyChangeFlag(), PROPERTY_UPDATE_NORMAL);
    CleanItemDirty();
    offsets->clear();
    itemPattern_->OnRecycle();
    EXPECT_TRUE(offsets->empty());
    EXPECT_FALSE(item_->IsLayoutDirtyMarked());
    EXPECT_EQ(item_->GetLayoutProperty()->GetPropertyChangeFlag(), PROPERTY_UPDATE_NORMAL);
}

/**
 * @tc.name: RecycleSwipeChildOffsetLeft
 * @tc.desc: The symmetric empty end side also clears its ITEM_CHILD offset and drag flag.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeChildOffsetLeft, TestSize.Level1)
{
    CreateSwipeItem(true, false);
    StartDrag();
    UpdateDrag(-40.0f);
    ASSERT_EQ(itemPattern_->GetSwiperIndex(), ListItemSwipeIndex::ITEM_CHILD);
    ASSERT_LT(itemPattern_->curOffset_, 0.0f);
    ASSERT_TRUE(itemPattern_->isDragging_);
    CleanItemDirty();

    itemPattern_->OnRecycle();

    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_TRUE(item_->IsLayoutDirtyMarked());
    EXPECT_NE(item_->GetLayoutProperty()->GetPropertyChangeFlag(), PROPERTY_UPDATE_NORMAL);
}

/**
 * @tc.name: RecycleSwipeOffsetWithoutDragging
 * @tc.desc: A nonzero residual requires dirty layout even when the drag flag has already been cleared.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeOffsetWithoutDragging, TestSize.Level1)
{
    CreateSwipeItem();
    StartDrag();
    UpdateDrag(40.0f);
    ASSERT_GT(itemPattern_->curOffset_, 0.0f);
    itemPattern_->isDragging_ = false;
    CleanItemDirty();

    itemPattern_->OnRecycle();

    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_TRUE(item_->IsLayoutDirtyMarked());
    EXPECT_NE(item_->GetLayoutProperty()->GetPropertyChangeFlag(), PROPERTY_UPDATE_NORMAL);
}

/**
 * @tc.name: RecycleSwipeDetachedEnd
 * @tc.desc: End after losing the List parent resets a stopped drag before any later recycle.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeDetachedEnd, TestSize.Level1)
{
    CreateSwipeItem();
    StartDrag();
    UpdateDrag(40.0f);
    auto end = itemPattern_->panEvent_->GetActionEndEventFunc();
    list_->RemoveChild(item_);
    ASSERT_EQ(itemPattern_->GetListFrameNode(), nullptr);
    auto offsets = std::make_shared<std::vector<int32_t>>();
    itemPattern_->SetOffsetChangeCallBack([offsets](int32_t offset) { offsets->emplace_back(offset); });
    CleanItemDirty();
    GestureEvent info;
    end(info);
    ASSERT_EQ(itemPattern_->curOffset_, 0.0f);
    ASSERT_FALSE(itemPattern_->isDragging_);
    EXPECT_EQ(*offsets, std::vector<int32_t>({ 0 }));
    EXPECT_TRUE(item_->IsLayoutDirtyMarked());
    CleanItemDirty();
    offsets->clear();

    itemPattern_->OnRecycle();

    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_EQ(itemPattern_->GetSwiperIndex(), ListItemSwipeIndex::ITEM_CHILD);
    EXPECT_TRUE(offsets->empty());
    EXPECT_FALSE(item_->IsLayoutDirtyMarked());
}

/**
 * @tc.name: RecycleSwipeDirectEndWithoutList
 * @tc.desc: The direct HandleDragEnd entry safely returns when its List parent is absent.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeDirectEndWithoutList, TestSize.Level1)
{
    CreateSwipeItem();
    StartDrag();
    UpdateDrag(40.0f);
    const float offset = itemPattern_->curOffset_;
    const auto controllerStatus = itemPattern_->springController_->GetStatus();
    ASSERT_EQ(itemPattern_->springMotion_, nullptr);
    list_->RemoveChild(item_);
    ASSERT_EQ(itemPattern_->GetListFrameNode(), nullptr);
    GestureEvent info;

    itemPattern_->HandleDragEnd(info);

    EXPECT_EQ(itemPattern_->curOffset_, offset);
    EXPECT_EQ(itemPattern_->springController_->GetStatus(), controllerStatus);
    EXPECT_EQ(itemPattern_->springMotion_, nullptr);
}

/**
 * @tc.name: RecycleSwipeOldCallbacks
 * @tc.desc: Saved Update, End and Cancel callbacks cannot restart a recycled drag before a new Start.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeOldCallbacks, TestSize.Level1)
{
    CreateSwipeItem();
    StartDrag();
    UpdateDrag(40.0f);
    ASSERT_EQ(itemPattern_->GetListFrameNode(), list_);
    ASSERT_TRUE(listPattern_->IsCurrentSwiperItem(AceType::WeakClaim(AceType::RawPtr(itemPattern_))));
    auto update = itemPattern_->panEvent_->GetActionUpdateEventFunc();
    auto end = itemPattern_->panEvent_->GetActionEndEventFunc();
    auto cancel = itemPattern_->panEvent_->GetActionCancelEventFunc();
    itemPattern_->OnRecycle();
    ASSERT_EQ(itemPattern_->curOffset_, 0.0f);
    ASSERT_FALSE(itemPattern_->isDragging_);
    auto offsets = std::make_shared<std::vector<int32_t>>();
    itemPattern_->SetOffsetChangeCallBack([offsets](int32_t offset) { offsets->emplace_back(offset); });
    GestureEvent info;
    info.SetMainDelta(20.0f);

    update(info);
    end(info);
    cancel();

    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_NE(itemPattern_->springController_->GetStatus(), Animator::Status::RUNNING);
    EXPECT_TRUE(offsets->empty());
    StartDrag();
    ASSERT_TRUE(itemPattern_->isDragging_);
    update(info);
    EXPECT_GT(itemPattern_->curOffset_, 0.0f);
}

/**
 * @tc.name: RecycleSwipeZeroOffsetDragging
 * @tc.desc: Recycle ends a drag before its first Update and releases this item's List lock.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeZeroOffsetDragging, TestSize.Level1)
{
    CreateSwipeItem();
    StartDrag();
    ASSERT_TRUE(itemPattern_->isDragging_);
    ASSERT_EQ(itemPattern_->curOffset_, 0.0f);
    ASSERT_FALSE(listPattern_->canReplaceSwiperItem_);

    itemPattern_->OnRecycle();

    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_TRUE(listPattern_->canReplaceSwiperItem_);
    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
}

/**
 * @tc.name: RecycleSwipeIdleNoDirty
 * @tc.desc: Repeated recycle of an untouched IDLE item neither dirties layout nor emits offset changes.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeIdleNoDirty, TestSize.Level1)
{
    CreateSwipeItem();
    ASSERT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::IDLE);
    auto offsets = std::make_shared<std::vector<int32_t>>();
    itemPattern_->SetOffsetChangeCallBack([offsets](int32_t offset) { offsets->emplace_back(offset); });

    itemPattern_->OnRecycle();
    itemPattern_->OnRecycle();

    EXPECT_FALSE(item_->IsLayoutDirtyMarked());
    EXPECT_EQ(item_->GetLayoutProperty()->GetPropertyChangeFlag(), PROPERTY_UPDATE_NORMAL);
    EXPECT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::IDLE);
    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
    EXPECT_TRUE(offsets->empty());
}

/**
 * @tc.name: RecycleSwipeStoppedNoDirty
 * @tc.desc: A collapsed stopped controller does not cause layout work on repeated recycle.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeStoppedNoDirty, TestSize.Level1)
{
    CreateSwipeItem();
    itemPattern_->springController_->Stop();
    ASSERT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::STOPPED);
    CleanItemDirty();

    itemPattern_->OnRecycle();
    itemPattern_->OnRecycle();

    EXPECT_FALSE(item_->IsLayoutDirtyMarked());
    EXPECT_EQ(item_->GetLayoutProperty()->GetPropertyChangeFlag(), PROPERTY_UPDATE_NORMAL);
    EXPECT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::STOPPED);
    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
}

/**
 * @tc.name: RecycleSwipeRunningController
 * @tc.desc: Recycle preserves a running CHILD close and its completion listener.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeRunningController, TestSize.Level1)
{
    CreateSwipeItem();
    itemPattern_->UpdatePostion(40.0f);
    auto finished = std::make_shared<int32_t>(0);
    itemPattern_->onFinishEvent_ = [finished]() { ++*finished; };
    itemPattern_->StartSpringMotion(40.0f, 0.0f, 0.0f, true);
    // The product mock immediately samples the final motion position but keeps RUNNING status.
    ASSERT_EQ(itemPattern_->curOffset_, 0.0f);
    ASSERT_EQ(itemPattern_->GetSwiperIndex(), ListItemSwipeIndex::ITEM_CHILD);
    ASSERT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::RUNNING);
    ASSERT_EQ(*finished, 0);

    itemPattern_->OnRecycle();

    EXPECT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::RUNNING);
    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
    EXPECT_EQ(*finished, 0);

    // Dispatch the preserved stop listener explicitly; the mock has no real frame scheduler.
    itemPattern_->springController_->Stop();

    EXPECT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::STOPPED);
    EXPECT_FALSE(itemPattern_->isSpringMotionRunning_);
    EXPECT_EQ(*finished, 1);
}

/**
 * @tc.name: RecycleSwipePausedController
 * @tc.desc: Recycle preserves a paused CHILD close instead of cancelling its completion listener.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipePausedController, TestSize.Level1)
{
    CreateSwipeItem();
    itemPattern_->UpdatePostion(40.0f);
    auto finished = std::make_shared<int32_t>(0);
    itemPattern_->onFinishEvent_ = [finished]() { ++*finished; };
    itemPattern_->StartSpringMotion(40.0f, 0.0f, 0.0f, true);
    ASSERT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::RUNNING);
    // Pause() is empty in the product mock; inject the real Animator's reachable PAUSED state.
    itemPattern_->springController_->status_ = Animator::Status::PAUSED;

    itemPattern_->OnRecycle();

    EXPECT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::PAUSED);
    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
    EXPECT_EQ(*finished, 0);

    itemPattern_->springController_->Stop();

    EXPECT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::STOPPED);
    EXPECT_FALSE(itemPattern_->isSpringMotionRunning_);
    EXPECT_EQ(*finished, 1);
}

/**
 * @tc.name: RecycleSwipeSpringBookkeeping
 * @tc.desc: A CHILD animation keeps its spring bookkeeping until its preserved stop listener runs.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeSpringBookkeeping, TestSize.Level1)
{
    CreateSwipeItem();
    itemPattern_->StartSpringMotion(0.0f, 0.0f, 0.0f);
    ASSERT_TRUE(itemPattern_->isSpringMotionRunning_);
    ASSERT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::RUNNING);

    itemPattern_->OnRecycle();

    EXPECT_TRUE(itemPattern_->isSpringMotionRunning_);
    EXPECT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::RUNNING);
    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);

    itemPattern_->springController_->Stop();

    EXPECT_FALSE(itemPattern_->isSpringMotionRunning_);
    EXPECT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::STOPPED);
}

/**
 * @tc.name: RecycleSwipeExpandedStart
 * @tc.desc: An expanded start action retains its collapse callback direction when recycled.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeExpandedStart, TestSize.Level1)
{
    CreateSwipeItem(true, true);
    itemPattern_->UpdatePostion(80.0f);
    itemPattern_->startNodeSize_ = 80.0f;
    itemPattern_->endNodeSize_ = 100.0f;
    itemPattern_->FireSwipeActionStateChange(ListItemSwipeIndex::SWIPER_START);
    auto states = std::make_shared<std::vector<SwipeActionState>>();
    auto oppositeStates = std::make_shared<std::vector<SwipeActionState>>();
    auto hub = item_->GetEventHub<ListItemEventHub>();
    hub->SetStartOnStateChange([states](SwipeActionState state) { states->emplace_back(state); });
    hub->SetEndOnStateChange([oppositeStates](SwipeActionState state) { oppositeStates->emplace_back(state); });

    itemPattern_->OnRecycle();

    EXPECT_EQ(itemPattern_->GetSwiperIndex(), ListItemSwipeIndex::ITEM_CHILD);
    EXPECT_EQ(itemPattern_->GetSwipeActionState(), SwipeActionState::COLLAPSED);
    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
    EXPECT_EQ(*states, std::vector<SwipeActionState>({ SwipeActionState::COLLAPSED }));
    EXPECT_TRUE(oppositeStates->empty());
    EXPECT_EQ(itemPattern_->startNodeSize_, 0.0f);
    EXPECT_EQ(itemPattern_->endNodeSize_, 0.0f);
}

/**
 * @tc.name: RecycleSwipeExpandedEnd
 * @tc.desc: An expanded end action retains its collapse callback direction when recycled.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeExpandedEnd, TestSize.Level1)
{
    CreateSwipeItem(true, true);
    itemPattern_->UpdatePostion(-100.0f);
    itemPattern_->startNodeSize_ = 80.0f;
    itemPattern_->endNodeSize_ = 100.0f;
    itemPattern_->FireSwipeActionStateChange(ListItemSwipeIndex::SWIPER_END);
    auto states = std::make_shared<std::vector<SwipeActionState>>();
    auto oppositeStates = std::make_shared<std::vector<SwipeActionState>>();
    auto hub = item_->GetEventHub<ListItemEventHub>();
    hub->SetEndOnStateChange([states](SwipeActionState state) { states->emplace_back(state); });
    hub->SetStartOnStateChange([oppositeStates](SwipeActionState state) { oppositeStates->emplace_back(state); });

    itemPattern_->OnRecycle();

    EXPECT_EQ(itemPattern_->GetSwiperIndex(), ListItemSwipeIndex::ITEM_CHILD);
    EXPECT_EQ(itemPattern_->GetSwipeActionState(), SwipeActionState::COLLAPSED);
    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
    EXPECT_EQ(*states, std::vector<SwipeActionState>({ SwipeActionState::COLLAPSED }));
    EXPECT_TRUE(oppositeStates->empty());
    EXPECT_EQ(itemPattern_->startNodeSize_, 0.0f);
    EXPECT_EQ(itemPattern_->endNodeSize_, 0.0f);
}

/**
 * @tc.name: RecycleSwipeDetachedReattach_001
 * @tc.desc: An ended detached node can reattach without recycle and accept another swipe.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeDetachedReattach_001, TestSize.Level1)
{
    CreateSwipeItem();
    auto context = MockPipelineContext::GetCurrent();
    item_->AttachToMainTree(false, AceType::RawPtr(context));
    ASSERT_TRUE(item_->IsOnMainTree());
    StartDrag();
    UpdateDrag(40.0f);
    ASSERT_GT(itemPattern_->curOffset_, 0.0f);
    ASSERT_FALSE(listPattern_->canReplaceSwiperItem_);
    auto end = itemPattern_->panEvent_->GetActionEndEventFunc();

    list_->RemoveChild(item_);
    ASSERT_FALSE(item_->IsOnMainTree());
    ASSERT_EQ(itemPattern_->GetListFrameNode(), nullptr);
    EXPECT_TRUE(listPattern_->canReplaceSwiperItem_);
    GestureEvent info;
    end(info);
    ASSERT_EQ(itemPattern_->curOffset_, 0.0f);
    ASSERT_FALSE(itemPattern_->isDragging_);
    ASSERT_EQ(itemPattern_->springMotion_, nullptr);

    // Reattach the same object directly; neither OnRecycle nor a new Pattern is involved.
    item_->MountToParent(list_);
    item_->AttachToMainTree(false, AceType::RawPtr(context));
    ASSERT_TRUE(item_->IsOnMainTree());
    ASSERT_EQ(itemPattern_->GetListFrameNode(), list_);
    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
    StartDrag();
    ASSERT_TRUE(itemPattern_->isDragging_);
    UpdateDrag(20.0f);
    ASSERT_GT(itemPattern_->curOffset_, 0.0f);

    end(info);

    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_TRUE(listPattern_->canReplaceSwiperItem_);
    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
    EXPECT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::RUNNING);
    item_->DetachFromMainTree();
}

/**
 * @tc.name: RecycleSwipeDetachedOffsetCallback_001
 * @tc.desc: A synchronous zero-offset callback can change the offset without being overwritten by End cleanup.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeDetachedOffsetCallback_001, TestSize.Level1)
{
    CreateSwipeItem();
    StartDrag();
    UpdateDrag(40.0f);
    auto offsets = std::make_shared<std::vector<int32_t>>();
    auto weak = AceType::WeakClaim(AceType::RawPtr(itemPattern_));
    itemPattern_->SetOffsetChangeCallBack([weak, offsets](int32_t offset) {
        offsets->emplace_back(offset);
        auto pattern = weak.Upgrade();
        ASSERT_NE(pattern, nullptr);
        if (offset == 0) {
            EXPECT_FALSE(pattern->isDragging_);
            EXPECT_TRUE(pattern->GetHost()->IsLayoutDirtyMarked());
            pattern->UpdatePostion(12.0f);
        }
    });
    list_->RemoveChild(item_);
    CleanItemDirty();
    GestureEvent info;

    itemPattern_->panEvent_->GetActionEndEventFunc()(info);

    EXPECT_EQ(itemPattern_->curOffset_, 12.0f);
    EXPECT_FALSE(itemPattern_->isDragging_);
    ASSERT_EQ(offsets->size(), 2u);
    EXPECT_EQ(offsets->front(), 0);
    EXPECT_GT(offsets->back(), 0);
    EXPECT_TRUE(item_->IsLayoutDirtyMarked());
}

/**
 * @tc.name: RecycleSwipeDetachedProgramClose_001
 * @tc.desc: A detached End ends dragging while preserving an active programmatic close and its completion.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeDetachedProgramClose_001, TestSize.Level1)
{
    CreateSwipeItem();
    StartDrag();
    UpdateDrag(40.0f);
    auto finished = std::make_shared<int32_t>(0);
    itemPattern_->onFinishEvent_ = [finished]() { ++*finished; };
    itemPattern_->StartSpringMotion(40.0f, 0.0f, 0.0f, true);
    ASSERT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::RUNNING);
    // The mock jumps to the target; model a nonzero in-flight sample for this guard check.
    itemPattern_->UpdatePostion(24.0f);
    auto motion = itemPattern_->springMotion_;
    auto update = itemPattern_->panEvent_->GetActionUpdateEventFunc();
    list_->RemoveChild(item_);
    ASSERT_TRUE(itemPattern_->isDragging_);
    ASSERT_EQ(itemPattern_->GetSwiperIndex(), ListItemSwipeIndex::ITEM_CHILD);
    CleanItemDirty();
    auto offsets = std::make_shared<std::vector<int32_t>>();
    itemPattern_->SetOffsetChangeCallBack([offsets](int32_t offset) { offsets->emplace_back(offset); });
    GestureEvent info;

    itemPattern_->panEvent_->GetActionEndEventFunc()(info);

    EXPECT_EQ(itemPattern_->curOffset_, 24.0f);
    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::RUNNING);
    EXPECT_EQ(itemPattern_->springMotion_, motion);
    EXPECT_EQ(*finished, 0);
    EXPECT_TRUE(offsets->empty());
    EXPECT_FALSE(item_->IsLayoutDirtyMarked());
    // Returning to the same List does not let an old Update resume the ended drag.
    item_->MountToParent(list_);
    ASSERT_TRUE(listPattern_->IsCurrentSwiperItem(AceType::WeakClaim(AceType::RawPtr(itemPattern_))));
    CleanItemDirty();
    info.SetMainDelta(20.0f);
    update(info);
    EXPECT_EQ(itemPattern_->curOffset_, 24.0f);
    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::RUNNING);
    EXPECT_EQ(itemPattern_->springMotion_, motion);
    EXPECT_EQ(*finished, 0);
    EXPECT_TRUE(offsets->empty());
    EXPECT_FALSE(item_->IsLayoutDirtyMarked());
    itemPattern_->springController_->Stop();
    EXPECT_EQ(*finished, 1);
}

/**
 * @tc.name: RecycleSwipeDetachedOnMainTree_001
 * @tc.desc: Losing the List parent ends a drag even when the node remains marked on the main tree.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeDetachedOnMainTree_001, TestSize.Level1)
{
    CreateSwipeItem();
    StartDrag();
    UpdateDrag(40.0f);
    list_->RemoveChild(item_);
    auto context = MockPipelineContext::GetCurrent();
    item_->AttachToMainTree(false, AceType::RawPtr(context));
    ASSERT_TRUE(item_->IsOnMainTree());
    ASSERT_EQ(itemPattern_->GetListFrameNode(), nullptr);
    ASSERT_TRUE(itemPattern_->isDragging_);
    auto offsets = std::make_shared<std::vector<int32_t>>();
    itemPattern_->SetOffsetChangeCallBack([offsets](int32_t offset) { offsets->emplace_back(offset); });
    CleanItemDirty();
    GestureEvent info;

    itemPattern_->panEvent_->GetActionEndEventFunc()(info);

    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_EQ(itemPattern_->springMotion_, nullptr);
    EXPECT_EQ(*offsets, std::vector<int32_t>({ 0 }));
    EXPECT_TRUE(item_->IsLayoutDirtyMarked());
    EXPECT_TRUE(item_->IsOnMainTree());
    item_->DetachFromMainTree();
}

/**
 * @tc.name: RecycleSwipeDetachedZeroOffset_001
 * @tc.desc: A detached End before the first Update ends dragging without offset notification or layout dirt.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeDetachedZeroOffset_001, TestSize.Level1)
{
    CreateSwipeItem();
    StartDrag();
    ASSERT_TRUE(itemPattern_->isDragging_);
    ASSERT_EQ(itemPattern_->curOffset_, 0.0f);
    list_->RemoveChild(item_);
    auto offsets = std::make_shared<std::vector<int32_t>>();
    itemPattern_->SetOffsetChangeCallBack([offsets](int32_t offset) { offsets->emplace_back(offset); });
    CleanItemDirty();
    GestureEvent info;

    itemPattern_->panEvent_->GetActionEndEventFunc()(info);

    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
    EXPECT_EQ(itemPattern_->springMotion_, nullptr);
    EXPECT_TRUE(offsets->empty());
    EXPECT_FALSE(item_->IsLayoutDirtyMarked());
}

/**
 * @tc.name: RecycleSwipeDetachedPendingAction_001
 * @tc.desc: A detached End preserves an actual pending SwipeForward command while ending dragging.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeDetachedPendingAction_001, TestSize.Level1)
{
    CreateSwipeItem();
    StartDrag();
    UpdateDrag(40.0f);
    itemPattern_->SwipeForward();
    ASSERT_TRUE(itemPattern_->pendingSwipeFunc_);
    ASSERT_TRUE(itemPattern_->isDragging_);
    ASSERT_EQ(itemPattern_->GetSwiperIndex(), ListItemSwipeIndex::ITEM_CHILD);
    ASSERT_EQ(itemPattern_->GetSwipeActionState(), SwipeActionState::COLLAPSED);
    const float offset = itemPattern_->curOffset_;
    ASSERT_LT(offset, 0.0f);
    list_->RemoveChild(item_);
    auto offsets = std::make_shared<std::vector<int32_t>>();
    itemPattern_->SetOffsetChangeCallBack([offsets](int32_t value) { offsets->emplace_back(value); });
    CleanItemDirty();
    GestureEvent info;

    itemPattern_->panEvent_->GetActionEndEventFunc()(info);

    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_EQ(itemPattern_->curOffset_, offset);
    EXPECT_TRUE(itemPattern_->pendingSwipeFunc_);
    EXPECT_EQ(itemPattern_->GetSwiperIndex(), ListItemSwipeIndex::ITEM_CHILD);
    EXPECT_EQ(itemPattern_->springMotion_, nullptr);
    EXPECT_TRUE(offsets->empty());
    EXPECT_FALSE(item_->IsLayoutDirtyMarked());
}

/**
 * @tc.name: RecycleSwipeEndNotDragging_001
 * @tc.desc: End does not reset an offset after another action has cleared the drag flag.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeEndNotDragging_001, TestSize.Level1)
{
    CreateSwipeItem();
    StartDrag();
    UpdateDrag(40.0f);
    itemPattern_->isDragging_ = false;
    const float offset = itemPattern_->curOffset_;
    auto end = itemPattern_->panEvent_->GetActionEndEventFunc();
    auto offsets = std::make_shared<std::vector<int32_t>>();
    itemPattern_->SetOffsetChangeCallBack([offsets](int32_t value) { offsets->emplace_back(value); });
    CleanItemDirty();
    GestureEvent info;

    end(info);
    EXPECT_EQ(itemPattern_->curOffset_, offset);
    EXPECT_FALSE(item_->IsLayoutDirtyMarked());
    list_->RemoveChild(item_);
    CleanItemDirty();
    end(info);

    EXPECT_EQ(itemPattern_->curOffset_, offset);
    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_FALSE(item_->IsLayoutDirtyMarked());
    EXPECT_TRUE(offsets->empty());
    EXPECT_EQ(itemPattern_->springMotion_, nullptr);
}

/**
 * @tc.name: RecycleSwipeDetachedCancel_001
 * @tc.desc: Cancel after losing the List parent also finishes a stopped drag without starting animation.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeDetachedCancel_001, TestSize.Level1)
{
    CreateSwipeItem();
    StartDrag();
    UpdateDrag(40.0f);
    auto offsets = std::make_shared<std::vector<int32_t>>();
    itemPattern_->SetOffsetChangeCallBack([offsets](int32_t value) { offsets->emplace_back(value); });
    list_->RemoveChild(item_);
    CleanItemDirty();

    itemPattern_->panEvent_->GetActionCancelEventFunc()();

    EXPECT_EQ(itemPattern_->curOffset_, 0.0f);
    EXPECT_FALSE(itemPattern_->isDragging_);
    EXPECT_EQ(itemPattern_->springController_->GetStatus(), Animator::Status::STOPPED);
    EXPECT_EQ(itemPattern_->springMotion_, nullptr);
    EXPECT_EQ(*offsets, std::vector<int32_t>({ 0 }));
    EXPECT_TRUE(item_->IsLayoutDirtyMarked());
}

/**
 * @tc.name: RecycleSwipeDetachedMouseAxis_001
 * @tc.desc: An ignored mouse-axis End cannot reset a detached touch drag.
 * @tc.type: FUNC
 */
HWTEST_F(ListTestCore, RecycleSwipeDetachedMouseAxis_001, TestSize.Level1)
{
    CreateSwipeItem();
    StartDrag();
    UpdateDrag(40.0f);
    const float offset = itemPattern_->curOffset_;
    list_->RemoveChild(item_);
    CleanItemDirty();
    GestureEvent info;
    info.SetInputEventType(InputEventType::AXIS);
    info.SetSourceTool(SourceTool::MOUSE);

    itemPattern_->panEvent_->GetActionEndEventFunc()(info);

    EXPECT_EQ(itemPattern_->curOffset_, offset);
    EXPECT_TRUE(itemPattern_->isDragging_);
    EXPECT_EQ(itemPattern_->springMotion_, nullptr);
    EXPECT_FALSE(item_->IsLayoutDirtyMarked());
}

} // namespace
} // namespace OHOS::Ace::NG
