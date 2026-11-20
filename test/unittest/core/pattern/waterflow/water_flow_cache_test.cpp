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

#define protected public
#define private public
#include "core/components_ng/pattern/scrollable/scrollable_utils.h"
#include "core/components_ng/pattern/waterflow/layout/water_flow_layout_algorithm_base.h"
#undef protected
#undef private
#include "test/mock/adapter/ohos/osal/mock_system_properties.h"
#include "test/mock/frameworks/core/common/mock_resource_adapter_v2.h"
#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/frameworks/core/components_ng/render/mock_render_context.h"
#include "water_flow_test_ng.h"

#include "core/common/container.h"
#include "core/components_ng/pattern/waterflow/layout/top_down/water_flow_layout_algorithm.h"
#ifdef TEST_WATER_FLOW_SW
#include "core/components_ng/pattern/waterflow/layout/sliding_window/water_flow_layout_info_sw.h"
#endif

namespace OHOS::Ace::NG {
/**
 * @tc.name: Cache001
 * @tc.desc: Test cache item preload
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, Cache001, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return i % 2 ? 100.0f : 200.0f; });

    model.SetCachedCount(3);
    model.SetColumnsTemplate("1fr 1fr");
    model.SetRowsGap(Dimension(10));
    model.SetColumnsGap(Dimension(10));
    CreateDone();
    auto info = pattern_->layoutInfo_;
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 10);

    const std::list<int32_t> preloadList = { 11, 12, 13 };
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 11));
    EXPECT_EQ(pattern_->preloadItems_, preloadList);
    EXPECT_TRUE(pattern_->cacheLayout_);
    PipelineContext::GetCurrentContext()->OnIdle(INT64_MAX);
    EXPECT_TRUE(pattern_->preloadItems_.empty());
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 11));
    EXPECT_EQ(GetChildHeight(frameNode_, 12), 200.0f);
    EXPECT_EQ(GetChildWidth(frameNode_, 13), (WATER_FLOW_WIDTH - 10.0f) / 2.0f);
    EXPECT_EQ(layoutProperty_->GetPropertyChangeFlag(), PROPERTY_UPDATE_LAYOUT);

    UpdateCurrentOffset(-500.0f);
    EXPECT_EQ(info->startIndex_, 4);
    EXPECT_EQ(info->endIndex_, 17);
    EXPECT_EQ(GetChildY(frameNode_, 3), -290.0f);
    EXPECT_EQ(GetChildY(frameNode_, 2), -390.0f);
    EXPECT_EQ(GetChildY(frameNode_, 1), -500.0f);
    const std::list<int32_t> preloadList2 = { 18, 19, 20 };
    EXPECT_EQ(pattern_->preloadItems_, preloadList2);
    PipelineContext::GetCurrentContext()->OnIdle(GetSysTimestamp());
    EXPECT_EQ(pattern_->preloadItems_, preloadList2);
}

/**
 * @tc.name: Cache004
 * @tc.desc: Test cache item reaching deadline
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, Cache004, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return i % 2 ? 100.0f : 200.0f; });

    model.SetCachedCount(3);
    model.SetColumnsTemplate("1fr 1fr");
    model.SetRowsGap(Dimension(10));
    model.SetColumnsGap(Dimension(10));
    CreateDone();
    auto info = pattern_->layoutInfo_;
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 10);

    const std::list<int32_t> preloadList = { 11, 12, 13 };
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 11));
    EXPECT_EQ(pattern_->preloadItems_, preloadList);
    EXPECT_TRUE(pattern_->cacheLayout_);
    // later expand to fuzz test
    PipelineContext::GetCurrentContext()->OnIdle(100000);
    // items still in preload list should not be created
    for (auto&& item : pattern_->preloadItems_) {
        EXPECT_FALSE(GetChildFrameNode(frameNode_, item));
    }
    for (auto&& itemIdx : preloadList) {
        // check preloaded items
        if (!pattern_->preloadItems_.empty() && itemIdx == *pattern_->preloadItems_.begin()) {
            break;
        }
        EXPECT_TRUE(GetChildFrameNode(frameNode_, itemIdx));
        EXPECT_EQ(GetChildHeight(frameNode_, itemIdx), itemIdx % 2 ? 100.0f : 200.0f);
        EXPECT_EQ(GetChildWidth(frameNode_, itemIdx), (WATER_FLOW_WIDTH - 10.0f) / 2.0f);
    }
    if (pattern_->preloadItems_.size() != preloadList.size()) {
        EXPECT_EQ(layoutProperty_->GetPropertyChangeFlag(), PROPERTY_UPDATE_LAYOUT);
    }
}

/**
 * @tc.name: Cache005
 * @tc.desc: Test items in preloadList when Waterflow's height is changed to 0.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, Cache005, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return 100.0f; });
    model.SetCachedCount(3);
    model.SetColumnsTemplate("1fr 1fr");
    CreateDone();

    pattern_->ScrollToIndex(10);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 10);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 25);
    std::list<int32_t> preloadList = { 26, 27, 28 };
    EXPECT_EQ(pattern_->preloadItems_, preloadList);

    // change height to 0.0f.
    layoutProperty_->UpdateUserDefinedIdealSize(CalcSize(CalcLength(500.0f), CalcLength(Dimension(0.0f))));
    FlushUITasks();
    EXPECT_TRUE(IsEqual(frameNode_->GetGeometryNode()->GetFrameRect(), RectF(0, 0, 500.0f, 0)));
    EXPECT_TRUE(pattern_->PreloadListEmpty());
}

/**
 * @tc.name: Cache006
 * @tc.desc: Test items in preloadList when Waterflow is empty.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, Cache006, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    model.SetCachedCount(3);
    model.SetColumnsTemplate("1fr 1fr");
    CreateDone();
    EXPECT_TRUE(pattern_->PreloadListEmpty());
}

/**
 * @tc.name: Cache007
 * @tc.desc: Test layout cache items without scrolling.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, Cache007, TestSize.Level1)
{
    WaterFlowModelNG model = CreateWaterFlow();
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(2);
    CreateItemsInLazyForEach(50, [](uint32_t idx) { return 50.0f; });
    CreateDone();

    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 31);
    EXPECT_FALSE(GetItem(32, true));
    PipelineContext::GetCurrentContext()->OnIdle(INT64_MAX);
    auto item = GetItem(32, true);
    ASSERT_TRUE(item);
    auto ctx = AceType::DynamicCast<MockRenderContext>(item->GetRenderContext());
    FlushUITasks();
    EXPECT_EQ(item->GetGeometryNode()->GetFrameRect().ToString(), "RectT (0.00, 800.00) - [240.00 x 50.00]");
    EXPECT_EQ(ctx->paintRect_.ToString(), "RectT (0.00, 800.00) - [240.00 x 50.00]"); // should be synced with layout

    layoutProperty_->UpdateUserDefinedIdealSize(CalcSize(std::nullopt, CalcLength(1000.0f)));
    FlushUITasks();
    EXPECT_EQ(item->GetGeometryNode()->GetFrameRect().ToString(), "RectT (0.00, 800.00) - [240.00 x 50.00]");
    EXPECT_EQ(ctx->paintRect_, item->GetGeometryNode()->GetFrameRect()); // should be synced with layout
    EXPECT_EQ(GetChildFrameNode(frameNode_, 32), item);
    EXPECT_EQ(ctx, GetItem(32)->GetRenderContext());
}

/**
 * @tc.name: SetScrollBarColor001
 * @tc.desc: Test SetScrollBarColor function.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, SetScrollBarColor001, TestSize.Level1)
{
    WaterFlowModelNG model = CreateWaterFlow();
    ASSERT_NE(frameNode_, nullptr);
    model.SetScrollBarColor(std::nullopt);
    auto paintProperty = frameNode_->GetPaintProperty<ScrollablePaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto scrollBarColor = paintProperty->GetScrollBarColor();
    EXPECT_EQ(scrollBarColor, std::nullopt);
    model.SetScrollBarColor(Color::BLUE);
    EXPECT_EQ(WaterFlowModelNG::GetScrollBarColor(AceType::RawPtr(frameNode_)), Color::BLUE.GetValue());
    CreateDone();
}

/*
 * @tc.name: ShowCache004
 * @tc.desc: Test cache items active state
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, ShowCache004, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInLazyForEach(50, [](uint32_t idx) { return 100.0f; });
    model.SetCachedCount(1, true);
    model.SetLayoutDirection(FlexDirection::COLUMN_REVERSE);
    model.SetColumnsTemplate("1fr 1fr");
    CreateDone();

    UpdateCurrentOffset(-300.0f);
    UpdateCurrentOffset(110.0f);
    UpdateCurrentOffset(-200.0f);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 6);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 23);
        for (int i = 0; i < 50; ++i) {
        if (i < 5 || i > 24) {
            if (GetItem(i, true)) {
                EXPECT_FALSE(GetItem(i, true)->IsActive());
            }
        } else {
            EXPECT_TRUE(GetItem(i, true));
            EXPECT_TRUE(GetItem(i, true)->IsActive());
        }
    }
}

/**
 * @tc.name: CacheScroll001
 * @tc.desc: Layout WaterFlow cache items
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, CacheScroll001, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    model.SetCachedCount(10);
    model.SetRowsGap(Dimension(10));
    model.SetColumnsGap(Dimension(10));
    CreateItemsInLazyForEach(100, [](int32_t) { return 100.0f; });
    CreateDone();

    pattern_->isAnimationStop_ = false;
    UpdateCurrentOffset(-2000.0f);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 18);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 25);
    EXPECT_EQ(GetChildY(frameNode_, 18), -20.0f);
    PipelineContext::GetCurrentContext()->OnIdle(INT64_MAX);
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 8));

    UpdateCurrentOffset(200.0f);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 16);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 23);
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 7));
    EXPECT_EQ(GetChildY(frameNode_, 18), 180.0f);
    EXPECT_TRUE(GetItem(18)->IsOnMainTree());

    PipelineContext::GetCurrentContext()->OnIdle(INT64_MAX);
    ASSERT_TRUE(GetItem(7, true));
    EXPECT_FALSE(GetItem(7, true)->IsOnMainTree());
    EXPECT_EQ(GetItem(7, true)->GetLayoutProperty()->GetPropertyChangeFlag() & PROPERTY_UPDATE_MEASURE,
        PROPERTY_UPDATE_MEASURE);
    UpdateCurrentOffset(5.0f);
    EXPECT_EQ(GetChildLayoutProperty<LayoutProperty>(frameNode_, 7)->GetPropertyChangeFlag() & PROPERTY_UPDATE_MEASURE,
        PROPERTY_UPDATE_MEASURE);
}

/**
 * @tc.name: LazyForEachJump001
 * @tc.desc: Test jump function after changing dataSource in lazyforeach.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, LazyForEachJump001, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    model.SetCachedCount(10);
    CreateItemsInLazyForEach(100, [](int32_t) { return 100.0f; });
    CreateDone();

    AddItemInLazyForEach(1);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 7);
    EXPECT_EQ(GetChildY(frameNode_, 0), 0.0f);

    AddItemInLazyForEach(6);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 7);
    EXPECT_EQ(GetChildY(frameNode_, 6), 600.0f);
    ScrollToIndex(6, false, ScrollAlign::START);
    PipelineContext::GetCurrentContext()->OnIdle(INT64_MAX);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 6);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 13);
    EXPECT_FALSE(GetItem(5, true)->IsActive());

    AddItemInLazyForEach(17);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 6);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 13);
    ScrollToIndex(17, false, ScrollAlign::START);
    PipelineContext::GetCurrentContext()->OnIdle(INT64_MAX);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 17);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 24);
    EXPECT_EQ(GetChildY(frameNode_, 17), 0.0f);
    EXPECT_FALSE(GetItem(5, true));

    AddItemInLazyForEach(99);
    FlushUITasks();
    ScrollToIndex(99, false, ScrollAlign::START);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 92);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 99);
    EXPECT_EQ(frameNode_->GetTotalChildCount(), 100);
}

/**
 * @tc.name: LazyForEachJump002
 * @tc.desc: Test jump function after changing dataSource in lazyforeach.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, LazyForEachJump002, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    model.SetCachedCount(10);
    RefPtr<WaterFlowMockLazy> mockLazy = CreateItemsInLazyForEach(100, [](int32_t) { return 100.0f; });
    CreateDone();

    pattern_->ScrollToIndex(2, false, ScrollAlign::START);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 2);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 9);

    DeleteItemInLazyForEach(58);
    mockLazy->SetTotalCount(99);
    FlushUITasks();
    pattern_->ScrollToIndex(58, false, ScrollAlign::START);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 58);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 65);
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 58);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 65);
    EXPECT_EQ(GetChildY(frameNode_, 58), 0.0f);
    pattern_->ScrollToIndex(60, false, ScrollAlign::START);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 60);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 67);
    EXPECT_TRUE(GetItem(58, true)->IsOnMainTree());

    DeleteItemInLazyForEach(52);
    mockLazy->SetTotalCount(97);
    FlushUITasks();
    pattern_->ScrollToIndex(49, false, ScrollAlign::START);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 49);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 56);

    DeleteItemInLazyForEach(0);
    mockLazy->SetTotalCount(96);
    FlushUITasks();
    pattern_->ScrollToIndex(0, false, ScrollAlign::START);
    FlushUITasks();
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 0);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 7);
    EXPECT_EQ(frameNode_->GetTotalChildCount(), 96);
}

/**
 * @tc.name: LargeDataPerformance001
 * @tc.desc: Test WaterFlow performance with large dataset and memory management
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, LargeDataPerformance001, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    ViewAbstract::SetWidth(CalcLength(400.0f));
    ViewAbstract::SetHeight(CalcLength(600.0f));
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);

    // Create large number of items (1000) to test virtualization
    CreateItemsInLazyForEach(1000, [](uint32_t idx) {
        return 50.0f + (idx % 5) * 20.0f; // Different heights
    });
    CreateDone();

    // Verify that only visible items are created initially
    EXPECT_LE(pattern_->layoutInfo_->endIndex_, 20);
    int32_t initialChildCount = frameNode_->GetTotalChildCount();

    // Quick scroll to middle position
    pattern_->ScrollToIndex(500, false, ScrollAlign::START);
    FlushUITasks();

    // Verify memory management: total child count should remain within reasonable range
    EXPECT_LE(frameNode_->GetTotalChildCount(), initialChildCount + 50);
    EXPECT_GE(pattern_->layoutInfo_->startIndex_, 480);
    EXPECT_LE(pattern_->layoutInfo_->endIndex_, 520);

    // Quick scroll to the end
    UpdateCurrentOffset(-50000.0f);
    FlushUITasks();

    // Verify state when reaching the end
    EXPECT_GE(pattern_->layoutInfo_->startIndex_, 980);
    EXPECT_EQ(pattern_->layoutInfo_->endIndex_, 999);

    // Verify cached items count control
    EXPECT_LE(pattern_->preloadItems_.size(), 5);
}

/**
 * @tc.name: OnColorModeChange
 * @tc.desc: Test WaterFlow's OnColorModeChange
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, OnColorModeChange001, TestSize.Level1)
{
    g_isConfigChangePerform = true;

    auto model = CreateWaterFlow();
    ViewAbstract::SetWidth(CalcLength(400.0f));
    ViewAbstract::SetHeight(CalcLength(600.0f));
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);

    CreateDone();

    /**
     * @tc.steps: Set startIndex_ of pattern_ to 2
     */
    pattern_->OnColorModeChange(static_cast<int32_t>(ColorMode::DARK));
    auto paintProperty = pattern_->GetPaintProperty<ScrollablePaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    EXPECT_NE(paintProperty->GetScrollBarProperty(), nullptr);

    paintProperty = pattern_->GetPaintProperty<ScrollablePaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    paintProperty->ResetScrollBarProperty();
    pattern_->OnColorModeChange(static_cast<int32_t>(ColorMode::DARK));
    ASSERT_NE(paintProperty, nullptr);
    EXPECT_EQ(paintProperty->GetScrollBarProperty(), nullptr);

    g_isConfigChangePerform = false;
}

/**
 * @tc.name: ParseResObjScrollBarColor
 * @tc.desc: Test WaterFlow's ParseResObjScrollBarColor
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, ParseResObjScrollBarColor001, TestSize.Level1)
{
    ResetMockResourceData();
    g_isConfigChangePerform = true;

    const int32_t resId = 0;
    const int32_t resType = static_cast<int32_t>(ResourceType::COLOR);
    const Color resData = Color::RED;
    AddMockResourceData(0, resData);

    /**
     * @tc.steps: step1. Construct the objects for test
     */
    auto model = CreateWaterFlow();
    ViewAbstract::SetWidth(CalcLength(400.0f));
    ViewAbstract::SetHeight(CalcLength(600.0f));
    model.SetColumnsTemplate("1fr 1fr");
    model.SetCachedCount(5);
    ASSERT_NE(frameNode_, nullptr);
    ASSERT_NE(pattern_, nullptr);

    /**
     * @tc.steps: step2. Test
     */
    model.ParseResObjScrollBarColor(nullptr);

    std::vector<ResourceObjectParams> params;
    auto resObj = AceType::MakeRefPtr<ResourceObject>(resId, resType, params, "", "", Container::CurrentIdSafely());
    model.ParseResObjScrollBarColor(resObj);
    pattern_->OnColorModeChange(static_cast<int32_t>(ColorMode::DARK));

    auto currentColor = WaterFlowModelNG::GetScrollBarColor(AceType::RawPtr(frameNode_));
    EXPECT_EQ(resData.GetValue(), currentColor);

    CreateDone();

    ResetMockResourceData();
    g_isConfigChangePerform = false;
}

/**
 * @tc.name: SyncCacheLayoutChildCount001
 * @tc.desc: Test WaterFlowPattern sync-cache-in-layout reference counting
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, SyncCacheLayoutChildCount001, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(10, [](int32_t i) { return 100.0f; });
    model.SetCachedCount(1);
    model.SetColumnsTemplate("1fr 1fr");
    CreateDone();

    // no reference held, strategy disabled by default
    EXPECT_FALSE(pattern_->IsSyncCacheLayoutEnabled());
    // each attached Repeat child holds one reference
    pattern_->AddSyncCacheLayoutChild();
    EXPECT_TRUE(pattern_->IsSyncCacheLayoutEnabled());
    pattern_->AddSyncCacheLayoutChild();
    EXPECT_TRUE(pattern_->IsSyncCacheLayoutEnabled());
    // releasing one reference keeps the other one active
    pattern_->RemoveSyncCacheLayoutChild();
    EXPECT_TRUE(pattern_->IsSyncCacheLayoutEnabled());
    // last reference released: strategy disabled, counter clamped at zero
    pattern_->RemoveSyncCacheLayoutChild();
    EXPECT_FALSE(pattern_->IsSyncCacheLayoutEnabled());
    pattern_->RemoveSyncCacheLayoutChild();
    EXPECT_FALSE(pattern_->IsSyncCacheLayoutEnabled());
}

/**
 * @tc.name: IsSyncCacheLayoutEnabledWrapper001
 * @tc.desc: Test WaterFlowLayoutBase::IsSyncCacheLayoutEnabled wrapper resolution chain
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, IsSyncCacheLayoutEnabledWrapper001, TestSize.Level1)
{
    WaterFlowLayoutAlgorithm algorithm(AceType::MakeRefPtr<WaterFlowLayoutInfo>());
    // null wrapper
    EXPECT_FALSE(algorithm.IsSyncCacheLayoutEnabled(nullptr));

    // host without a WaterFlowPattern
    auto columnNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG, 902, AceType::MakeRefPtr<Pattern>());
    auto columnWrapper = columnNode->CreateLayoutWrapper();
    ASSERT_NE(columnWrapper, nullptr);
    EXPECT_FALSE(algorithm.IsSyncCacheLayoutEnabled(columnWrapper.GetRawPtr()));

    // waterflow host: disabled by default, enabled while the pattern holds a reference
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(10, [](int32_t i) { return 100.0f; });
    model.SetCachedCount(1);
    model.SetColumnsTemplate("1fr 1fr");
    CreateDone();
    auto waterFlowWrapper = pattern_->GetHost()->CreateLayoutWrapper();
    ASSERT_NE(waterFlowWrapper, nullptr);
    EXPECT_FALSE(algorithm.IsSyncCacheLayoutEnabled(waterFlowWrapper.GetRawPtr()));
    pattern_->AddSyncCacheLayoutChild();
    EXPECT_TRUE(algorithm.IsSyncCacheLayoutEnabled(waterFlowWrapper.GetRawPtr()));
    pattern_->RemoveSyncCacheLayoutChild();
    EXPECT_FALSE(algorithm.IsSyncCacheLayoutEnabled(waterFlowWrapper.GetRawPtr()));
}

// The sync-cache-in-layout strategy is implemented by the TOP_DOWN and SLIDING_WINDOW
// layout algorithms only; WaterFlowSegmentedLayout still fills its cache through the
// asynchronous preload task, so these tests are excluded from the segmented build.
#ifndef TEST_SEGMENTED_WATER_FLOW
/**
 * @tc.name: SyncCacheLayout001
 * @tc.desc: Test sync-cache-in-layout: cache items are built in the same frame instead of
 *           being preloaded asynchronously on idle
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, SyncCacheLayout001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Same setup as Cache001, but hold the sync-cache-in-layout reference
     *             on the pattern (as an attached Repeat with allowReuseInAnimation=false
     *             does via ToggleParentSyncCacheLayout)
     * @tc.expected: the cache items below the view are built, measured and deactivated
     *               within the same frame and no idle preload task is pending
     */
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return i % 2 ? 100.0f : 200.0f; });

    model.SetCachedCount(3);
    model.SetColumnsTemplate("1fr 1fr");
    model.SetRowsGap(Dimension(10));
    model.SetColumnsGap(Dimension(10));
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();
    auto info = pattern_->layoutInfo_;
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 10);

    // async strategy would leave preloadItems_ = { 11, 12, 13 } and child 11 unbuilt
    EXPECT_TRUE(pattern_->preloadItems_.empty());
    auto item = GetChildFrameNode(frameNode_, 11);
    ASSERT_NE(item, nullptr);
    EXPECT_FALSE(item->IsActive());
    EXPECT_EQ(GetChildHeight(frameNode_, 12), 200.0f);
    EXPECT_EQ(GetChildWidth(frameNode_, 13), (WATER_FLOW_WIDTH - 10.0f) / 2.0f);
    // the anchor at endIndex_ stays active: the tail fill deactivates every item it
    // re-measures (MeasureToTarget cache branch), the anchor re-activation restores it
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 10)->IsActive());

    /**
     * @tc.steps: step2. Scroll down with the sync strategy still held
     * @tc.expected: the new cache items below the view are again built in the same frame
     */
    UpdateCurrentOffset(-500.0f);
    EXPECT_EQ(info->startIndex_, 4);
    EXPECT_EQ(info->endIndex_, 17);
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 18));
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 18)->IsActive());
    EXPECT_TRUE(pattern_->preloadItems_.empty());
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 17)->IsActive());

    /**
     * @tc.steps: step3. Release the reference and scroll again
     * @tc.expected: cache filling falls back to the asynchronous preload strategy
     */
    pattern_->RemoveSyncCacheLayoutChild();
    UpdateCurrentOffset(-200.0f);
    EXPECT_FALSE(pattern_->preloadItems_.empty());
}

/**
 * @tc.name: SyncCacheLayout002
 * @tc.desc: Test the tail-scan skip path of SyncLayoutCacheItems: after a small scroll the
 *           new cache window overlaps the previously cached items, which are skipped
 *           (both record and node exist) and the fill chains MeasureToTarget from the last
 *           recorded anchor, building only the first unknown item in the same frame
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, SyncCacheLayout002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Same setup as SyncCacheLayout001
     * @tc.expected: cache items 11-13 are built in the first frame
     */
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return i % 2 ? 100.0f : 200.0f; });

    model.SetCachedCount(3);
    model.SetColumnsTemplate("1fr 1fr");
    model.SetRowsGap(Dimension(10));
    model.SetColumnsGap(Dimension(10));
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();
    auto info = pattern_->layoutInfo_;
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 10);
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 11));
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 12));
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 13));

    /**
     * @tc.steps: step2. Scroll slightly (only item 11 enters the viewport), keeping the
     *             sync strategy held
     * @tc.expected: items 12 and 13 are skipped by the tail scan (record + node both
     *               exist), the fill anchors at item 13 and builds item 14 in-frame;
     *               skipped items keep their nodes
     */
    UpdateCurrentOffset(-100.0f);
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 11);
    EXPECT_TRUE(pattern_->preloadItems_.empty());
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 12));
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 13));
    auto item = GetChildFrameNode(frameNode_, 14);
    ASSERT_NE(item, nullptr);
    EXPECT_FALSE(item->IsActive());
    // the skipped items (record and node both exist) are laid out and deactivated by
    // the tail scan in the same frame
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 12)->IsActive());
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 13)->IsActive());
}

/**
 * @tc.name: SyncCacheLayout003
 * @tc.desc: Test the early-return branch of SyncLayoutCacheItems: with cachedCount == 0
 *           nothing is built beyond the viewport, and the sync strategy still suppresses
 *           the asynchronous preload task
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, SyncCacheLayout003, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return i % 2 ? 100.0f : 200.0f; });

    model.SetCachedCount(0);
    model.SetColumnsTemplate("1fr 1fr");
    model.SetRowsGap(Dimension(10));
    model.SetColumnsGap(Dimension(10));
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();
    auto info = pattern_->layoutInfo_;
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 10);

    // no cache count: no item is built beyond the viewport and no preload task is queued
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 11));
    EXPECT_TRUE(pattern_->preloadItems_.empty());
}

#ifndef TEST_WATER_FLOW_SW
/**
 * @tc.name: SyncCacheLayout004
 * @tc.desc: When only the size of the last visible item changes (same viewport range,
 *           no rerender, so the Repeat node skips its range update and nothing outside
 *           the layout algorithm re-activates nodes), the tail refill anchors at
 *           endIndex_ and the anchor keeps its active state
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, SyncCacheLayout004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Single column of 100px items, cachedCount 2, sync strategy held:
     *            viewport items 0-7, tail cache 8-9
     * @tc.expected: the last visible item is active, the tail cache items are not
     */
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(20, [](int32_t i) { return 100.0f; });
    model.SetCachedCount(2);
    model.SetColumnsTemplate("1fr");
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();
    auto info = pattern_->layoutInfo_;
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 7);
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 7)->IsActive());
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 8)->IsActive());

    /**
     * @tc.steps: step2. Grow the last visible item's height via a direct layout
     *             property change: the visible range stays 0-7, so the syntax node
     *             skips its range update (no reactivation outside the layout)
     * @tc.expected: the tail records are cleared by the size change, the tail refill
     *               re-measures from the anchor at endIndex_ (deactivating it on the
     *               cache path), and the anchor re-activation keeps item 7 active
     */
    auto item7 = GetChildFrameNode(frameNode_, 7);
    ASSERT_NE(item7, nullptr);
    item7->GetLayoutProperty()->UpdateUserDefinedIdealSize(CalcSize(std::nullopt, CalcLength(110.0f)));
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_BY_CHILD_REQUEST);
    FlushUITasks();

    EXPECT_EQ(info->endIndex_, 7);
    item7 = GetChildFrameNode(frameNode_, 7);
    ASSERT_NE(item7, nullptr);
    EXPECT_TRUE(item7->IsActive()) << "anchor at endIndex_ must survive the tail refill";
    EXPECT_EQ(GetChildHeight(frameNode_, 7), 110.0f);
    // tail cache rebuilt at the shifted positions stays inactive
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 8));
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 8)->IsActive());
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 9));
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 9)->IsActive());
}

/**
 * @tc.name: SyncCacheLayout005
 * @tc.desc: After scrolling down and back up, the head cache items above the viewport
 *           (position recorded, node still existing) are laid out and deactivated by
 *           the head loop of SyncLayoutCacheItems in the same frame
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, SyncCacheLayout005, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return i % 2 ? 100.0f : 200.0f; });
    model.SetCachedCount(3);
    model.SetColumnsTemplate("1fr 1fr");
    model.SetRowsGap(Dimension(10));
    model.SetColumnsGap(Dimension(10));
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();
    auto info = pattern_->layoutInfo_;
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 10);

    // scroll down twice so the head cache range holds items that were dropped from the
    // Repeat L1 cache (recycled) while their position records survive
    UpdateCurrentOffset(-500.0f);
    UpdateCurrentOffset(-500.0f);
    EXPECT_GT(info->startIndex_, 0);
    // scroll back up: the head loop rebuilds the farthest recycled head items
    // (record kept, node missing: measured on first build) and re-lays out the nearer
    // ones (record and node kept), deactivating both in the same frame
    UpdateCurrentOffset(250.0f);
    EXPECT_GT(info->startIndex_, 0);
    const int32_t headNear = info->startIndex_ - 1;
    ASSERT_GE(headNear, 0);
    auto headItem = GetChildFrameNode(frameNode_, headNear);
    ASSERT_NE(headItem, nullptr) << "head cache node kept in-frame";
    EXPECT_FALSE(headItem->IsActive());
    const int32_t headFar = std::max(0, info->startIndex_ - 3);
    auto rebuiltHead = GetChildFrameNode(frameNode_, headFar);
    ASSERT_NE(rebuiltHead, nullptr) << "recycled head cache node rebuilt in-frame";
    EXPECT_FALSE(rebuiltHead->IsActive());
    EXPECT_TRUE(pattern_->preloadItems_.empty());
}
#endif // !TEST_WATER_FLOW_SW

#ifdef TEST_WATER_FLOW_SW
/**
 * @tc.name: SyncCacheLayoutSW001
 * @tc.desc: [SLIDING_WINDOW mode] sync-cache-in-layout: cache items are built, measured
 *           and deactivated in the same frame instead of being preloaded asynchronously
 *           on idle
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, SyncCacheLayoutSW001, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return i % 2 ? 100.0f : 200.0f; });

    model.SetCachedCount(3);
    model.SetColumnsTemplate("1fr 1fr");
    model.SetRowsGap(Dimension(10));
    model.SetColumnsGap(Dimension(10));
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();
    auto info = pattern_->layoutInfo_;
    EXPECT_EQ(info->startIndex_, 0);
    EXPECT_EQ(info->endIndex_, 10);

    // async strategy would leave preloadItems_ = { 11, 12, 13 } and child 11 unbuilt
    EXPECT_TRUE(pattern_->preloadItems_.empty());
    auto item = GetChildFrameNode(frameNode_, 11);
    ASSERT_NE(item, nullptr);
    EXPECT_FALSE(item->IsActive());
    EXPECT_EQ(GetChildHeight(frameNode_, 12), 200.0f);
    EXPECT_EQ(GetChildWidth(frameNode_, 13), (WATER_FLOW_WIDTH - 10.0f) / 2.0f);

    // scrolling extends the cache in the same frame as well
    UpdateCurrentOffset(-500.0f);
    EXPECT_EQ(info->startIndex_, 4);
    EXPECT_EQ(info->endIndex_, 17);
    EXPECT_TRUE(GetChildFrameNode(frameNode_, 18));
    EXPECT_FALSE(GetChildFrameNode(frameNode_, 18)->IsActive());
    EXPECT_TRUE(pattern_->preloadItems_.empty());

    // releasing the reference falls back to the asynchronous preload strategy
    pattern_->RemoveSyncCacheLayoutChild();
    UpdateCurrentOffset(-200.0f);
    EXPECT_FALSE(pattern_->preloadItems_.empty());
}

/**
 * @tc.name: SyncCacheLayoutSW002
 * @tc.desc: [SLIDING_WINDOW mode] a jump sets needFixOffsetCache_ to prime the
 *           clip-extended cache side; while the sync-cache strategy is held the
 *           fix-offset preload is skipped (the sync pass fills the same range), so the
 *           flag survives and is honored by the first frame without the strategy
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, SyncCacheLayoutSW002, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return 100.0f; });

    model.SetCachedCount(2);
    model.SetColumnsTemplate("1fr 1fr");
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();
    auto swInfo = AceType::DynamicCast<WaterFlowLayoutInfoSW>(pattern_->layoutInfo_);
    ASSERT_NE(swInfo, nullptr);
    EXPECT_FALSE(swInfo->needFixOffsetCache_);

    // jump far outside the view: the jump path sets needFixOffsetCache_, but the sync
    // strategy fills the cache range in this very frame, so the fix-offset preload is
    // skipped and the flag stays set
    pattern_->ScrollToIndex(40, false, ScrollAlign::START);
    FlushUITasks();
    // the SW window shifts up from item 40 so the viewport plus its cache stays filled
    EXPECT_EQ(pattern_->layoutInfo_->startIndex_, 34);
    EXPECT_TRUE(swInfo->needFixOffsetCache_) << "flag kept while the sync strategy covers the range";
    EXPECT_TRUE(pattern_->preloadItems_.empty());
    // the item just above the window head is built by the sync pass and deactivated
    auto cacheItem = GetChildFrameNode(frameNode_, 33);
    ASSERT_NE(cacheItem, nullptr);
    EXPECT_FALSE(cacheItem->IsActive());

    // releasing the reference: the next frame consumes the surviving flag through the
    // fix-offset preload instead of losing it
    pattern_->RemoveSyncCacheLayoutChild();
    UpdateCurrentOffset(-100.0f);
    EXPECT_FALSE(swInfo->needFixOffsetCache_) << "flag consumed by the fix-offset preload";
}
#endif // TEST_WATER_FLOW_SW
#endif // !TEST_SEGMENTED_WATER_FLOW

#ifdef TEST_SEGMENTED_WATER_FLOW
/**
 * @tc.name: SyncCacheLayoutSegmented001
 * @tc.desc: [SEGMENTED layout] sync-cache-in-layout: cache items on both sides of the
 *           viewport are built, laid out and deactivated in the same frame instead of
 *           being preloaded asynchronously on idle
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, SyncCacheLayoutSegmented001, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return 100.0f; });

    model.SetCachedCount(2);
    model.SetColumnsTemplate("1fr 1fr");
    pattern_->AddSyncCacheLayoutChild();
    CreateDone();
    auto info = pattern_->layoutInfo_;
    EXPECT_EQ(info->startIndex_, 0);

    // tail cache built, laid out and deactivated in-frame; the async preload list of the
    // segmented layout stays empty
    EXPECT_TRUE(pattern_->preloadItems_.empty());
    const int32_t tailIdx = info->endIndex_ + 1;
    ASSERT_LT(tailIdx, 50);
    auto tail = GetChildFrameNode(frameNode_, tailIdx);
    ASSERT_NE(tail, nullptr) << "tail cache built synchronously during Layout";
    EXPECT_FALSE(tail->IsActive());
    EXPECT_EQ(GetChildHeight(frameNode_, tailIdx), 100.0f);

    // scrolling down: the new tail cache is built in-frame as well
    UpdateCurrentOffset(-300.0f);
    const int32_t newTailIdx = info->endIndex_ + 1;
    ASSERT_LT(newTailIdx, 50);
    EXPECT_TRUE(GetChildFrameNode(frameNode_, newTailIdx));
    EXPECT_FALSE(GetChildFrameNode(frameNode_, newTailIdx)->IsActive());
    EXPECT_TRUE(pattern_->preloadItems_.empty());

    // scrolling back up (partially): the head cache items above the viewport (nodes
    // kept from the earlier frames) are laid out and deactivated in-frame
    UpdateCurrentOffset(100.0f);
    const int32_t headIdx = info->startIndex_ - 1;
    ASSERT_GE(headIdx, 0);
    auto headItem = GetChildFrameNode(frameNode_, headIdx);
    ASSERT_NE(headItem, nullptr) << "head cache kept in-frame";
    EXPECT_FALSE(headItem->IsActive());

    // releasing the reference falls back to the asynchronous preload strategy
    pattern_->RemoveSyncCacheLayoutChild();
    UpdateCurrentOffset(-300.0f);
    EXPECT_FALSE(pattern_->preloadItems_.empty());
}
#endif // TEST_SEGMENTED_WATER_FLOW

/**
 * @tc.name: UpdateWaterFlowDecodeCacheWindow001
 * @tc.desc: Test UpdateWaterFlowDecodeCacheWindow guard conditions do not crash.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, UpdateWaterFlowDecodeCacheWindow001, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return ITEM_MAIN_SIZE; });
    model.SetCachedCount(5);
    model.SetColumnsTemplate("1fr 1fr");
    CreateDone();
    auto layoutAlgorithm = AceType::DynamicCast<WaterFlowLayoutBase>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateWaterFlowDecodeCacheWindow(AceType::RawPtr(wrapper), 5); // 5 : cacheCount
}

/**
 * @tc.name: UpdateWaterFlowDecodeCacheWindow002
 * @tc.desc: Test UpdateWaterFlowDecodeCacheWindow with cacheCount=0 skips gracefully.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, UpdateWaterFlowDecodeCacheWindow002, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return ITEM_MAIN_SIZE; });
    model.SetCachedCount(0);
    model.SetColumnsTemplate("1fr 1fr");
    CreateDone();
    auto layoutAlgorithm = AceType::DynamicCast<WaterFlowLayoutBase>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateWaterFlowDecodeCacheWindow(AceType::RawPtr(wrapper), 0);
}

/**
 * @tc.name: UpdateWaterFlowDecodeCacheWindow003
 * @tc.desc: Test UpdateWaterFlowDecodeCacheWindow does not crash after scrolling.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, UpdateWaterFlowDecodeCacheWindow003, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return ITEM_MAIN_SIZE; });
    model.SetCachedCount(5);
    model.SetColumnsTemplate("1fr 1fr");
    CreateDone();
    UpdateCurrentOffset(-500.0f); // 500 : scroll offset
    auto layoutAlgorithm = AceType::DynamicCast<WaterFlowLayoutBase>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateWaterFlowDecodeCacheWindow(AceType::RawPtr(wrapper), 5); // 5 : cacheCount
}

/**
 * @tc.name: UpdateWaterFlowDecodeCacheWindow004
 * @tc.desc: Test B1 — layoutWrapper == nullptr does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, UpdateWaterFlowDecodeCacheWindow004, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return ITEM_MAIN_SIZE; });
    model.SetCachedCount(5);
    model.SetColumnsTemplate("1fr 1fr");
    CreateDone();
    auto layoutAlgorithm = AceType::DynamicCast<WaterFlowLayoutBase>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    layoutAlgorithm->UpdateWaterFlowDecodeCacheWindow(nullptr, 5); // 5 : cacheCount
}

/**
 * @tc.name: UpdateDecodeCacheWindowInternal001
 * @tc.desc: Test B6 — showCachedItems == true early return via internal method.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, UpdateDecodeCacheWindowInternal001, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return ITEM_MAIN_SIZE; });
    model.SetCachedCount(5);
    model.SetColumnsTemplate("1fr 1fr");
    CreateDone();
    WaterFlowModelNG::SetShowCached(AceType::RawPtr(frameNode_), true);
    auto layoutAlgorithm = AceType::DynamicCast<WaterFlowLayoutBase>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateDecodeCacheWindowInternal(AceType::RawPtr(wrapper), 5); // 5 : cacheCount
}

/**
 * @tc.name: UpdateDecodeCacheWindowInternal002
 * @tc.desc: Test B7 — cacheCount <= 0 early return via internal method.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, UpdateDecodeCacheWindowInternal002, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return ITEM_MAIN_SIZE; });
    model.SetCachedCount(5);
    model.SetColumnsTemplate("1fr 1fr");
    CreateDone();
    auto layoutAlgorithm = AceType::DynamicCast<WaterFlowLayoutBase>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateDecodeCacheWindowInternal(AceType::RawPtr(wrapper), 0);
}

/**
 * @tc.name: UpdateDecodeCacheWindowInternal003
 * @tc.desc: Test B8 — startIndex < 0 early return via internal method.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, UpdateDecodeCacheWindowInternal003, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return ITEM_MAIN_SIZE; });
    model.SetCachedCount(5);
    model.SetColumnsTemplate("1fr 1fr");
    CreateDone();
    auto layoutAlgorithm = AceType::DynamicCast<WaterFlowLayoutBase>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto info = layoutAlgorithm->LayoutInfo();
    ASSERT_NE(info, nullptr);
    info->startIndex_ = -1;
    info->endIndex_ = -1;
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateDecodeCacheWindowInternal(AceType::RawPtr(wrapper), 5); // 5 : cacheCount
}

/**
 * @tc.name: UpdateDecodeCacheWindowInternal004
 * @tc.desc: Test B10/B11/B13/B14/B16 — normal path, non-segmented (footerIndex_ <= 0).
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, UpdateDecodeCacheWindowInternal004, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return ITEM_MAIN_SIZE; });
    model.SetCachedCount(5);
    model.SetColumnsTemplate("1fr 1fr");
    CreateDone();
    auto layoutAlgorithm = AceType::DynamicCast<WaterFlowLayoutBase>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto info = layoutAlgorithm->LayoutInfo();
    ASSERT_NE(info, nullptr);
    ASSERT_GE(info->startIndex_, 0);
    ASSERT_GE(info->endIndex_, 0);
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateDecodeCacheWindowInternal(AceType::RawPtr(wrapper), 5); // 5 : cacheCount
}

/**
 * @tc.name: UpdateDecodeCacheWindowInternal005
 * @tc.desc: Test B12 — footerIndex_ > 0 (segmented path: childIdx = idx).
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, UpdateDecodeCacheWindowInternal005, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return ITEM_MAIN_SIZE; });
    model.SetCachedCount(5);
    model.SetColumnsTemplate("1fr 1fr");
    CreateDone();
    auto layoutAlgorithm = AceType::DynamicCast<WaterFlowLayoutBase>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto info = layoutAlgorithm->LayoutInfo();
    ASSERT_NE(info, nullptr);
    ASSERT_GE(info->startIndex_, 0);
    ASSERT_GE(info->endIndex_, 0);
    info->footerIndex_ = 49; // 49 : simulate segmented footer at last child
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateDecodeCacheWindowInternal(AceType::RawPtr(wrapper), 5); // 5 : cacheCount
}

/**
 * @tc.name: UpdateDecodeCacheWindowInternal006
 * @tc.desc: Test I10 — unbuilt items (wrapper null) are skipped gracefully.
 * @tc.type: FUNC
 */
HWTEST_F(WaterFlowTestNg, UpdateDecodeCacheWindowInternal006, TestSize.Level1)
{
    auto model = CreateWaterFlow();
    CreateItemsInRepeat(50, [](int32_t i) { return ITEM_MAIN_SIZE; });
    model.SetCachedCount(5);
    model.SetColumnsTemplate("1fr 1fr");
    CreateDone();
    auto layoutAlgorithm = AceType::DynamicCast<WaterFlowLayoutBase>(pattern_->CreateLayoutAlgorithm());
    ASSERT_TRUE(layoutAlgorithm);
    auto info = layoutAlgorithm->LayoutInfo();
    ASSERT_NE(info, nullptr);
    ASSERT_GE(info->startIndex_, 0);
    ASSERT_GE(info->endIndex_, 0);
    const int32_t originStart = info->startIndex_;
    const int32_t originEnd = info->endIndex_;
    // Increase totalItemCount beyond built items to simulate unbuilt items
    info->firstRepeatCount_ = 0;
    info->childrenCount_ = 200; // 200 : simulate 200 items, only 50 built
    auto wrapper = frameNode_->CreateLayoutWrapper();
    ASSERT_NE(wrapper, nullptr);
    layoutAlgorithm->UpdateDecodeCacheWindowInternal(AceType::RawPtr(wrapper), 5); // 5 : cacheCount
    // Unbuilt items (idx 50..199) silently skipped, info state unchanged
    EXPECT_EQ(info->startIndex_, originStart);
    EXPECT_EQ(info->endIndex_, originEnd);
}

} // namespace OHOS::Ace::NG
