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
#include "interfaces/native/ui_info_collection.h"

#define protected public
#define private public

#include "test/mock/frameworks/core/common/mock_theme_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/interfaces/inner_api/ui_session/mock_ui_session_manager.h"

#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/manager/content_change_manager/content_change_manager.h"
#include "core/components_ng/pattern/swiper/swiper_layout_algorithm.h"
#include "core/components_ng/pattern/swiper/swiper_layout_property.h"
#include "core/components_ng/pattern/swiper/swiper_model_ng.h"
#include "core/components_ng/pattern/swiper/swiper_paint_property.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/root/root_pattern.h"
#include "core/components_ng/pattern/tabs/tab_bar_pattern.h"
#include "core/components_ng/pattern/tabs/tabs_layout_property.h"
#include "core/components_ng/pattern/tabs/tabs_pattern.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {

class SwiperTestCore : public testing::Test {
public:
    static void SetUpTestCase()
    {
        MockPipelineContext::SetUp();
        auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
        EXPECT_CALL(*themeManager, GetTheme(_, _)).Times(AnyNumber());
        MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    }

    static void TearDownTestCase()
    {
        MockPipelineContext::TearDown();
    }

    static void FinishNoAnimationLayout(const RefPtr<FrameNode>& swiperNode, int32_t index)
    {
        auto layoutProperty = swiperNode->GetLayoutProperty<SwiperLayoutProperty>();
        auto dirty = AceType::MakeRefPtr<LayoutWrapperNode>(
            swiperNode, swiperNode->GetGeometryNode(), layoutProperty);
        auto algorithm = AceType::MakeRefPtr<SwiperLayoutAlgorithm>();
        algorithm->SetCurrentIndex(index);
        dirty->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(algorithm));
        swiperNode->GetPattern<SwiperPattern>()->OnDirtyLayoutWrapperSwap(dirty, DirtySwapConfig());
        auto pipeline = MockPipelineContext::GetCurrent();
        pipeline->taskScheduler_->FlushAfterRenderTask();
        pipeline->GetContentChangeManager()->OnVsyncEnd(RectF());
    }
};

} // namespace

/**
 * @tc.name: ContentChangeSwiperStartTest001
 * @tc.desc: Test Swiper phase end keeps active state and explicit termination reports a normal END.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperTestCore, ContentChangeSwiperStartTest001, TestSize.Level1)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    auto manager = pipeline->GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);

    auto pattern = AceType::MakeRefPtr<SwiperPattern>();
    auto swiperNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 431, pattern, true);
    ASSERT_NE(swiperNode, nullptr);
    auto mockUiSessionManager = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(mockUiSessionManager, nullptr);

    auto paintProperty = swiperNode->GetPaintProperty<SwiperPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    paintProperty->UpdateAutoPlay(true);
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SWIPER_START, _)).Times(0);
    manager->OnSwiperScrollStart(swiperNode);
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    manager->OnSwiperScrollEnd(swiperNode);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    paintProperty->UpdateAutoPlay(false);
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SWIPER_START, _)).Times(1);
    manager->OnSwiperScrollStart(swiperNode);
    manager->OnSwiperScrollStart(swiperNode);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SWIPER, _)).Times(0);
    pattern->ContentChangeOnTransitionEnd(swiperNode);
    EXPECT_FALSE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    AnimationCallbackInfo info;
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SWIPER, Truly([](const auto& json) {
        auto value = JsonUtil::ParseJsonString(json);
        return value && value->GetString("endReason").empty();
    }))).Times(1);
    pattern->FireAnimationEndEvent(0, info, true);
    manager->OnVsyncEnd(RectF());
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeTabsStartTest001
 * @tc.desc: Test a Tabs-contained Swiper reports the outer Tabs identity.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperTestCore, ContentChangeTabsStartTest001, TestSize.Level1)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    auto manager = pipeline->GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);

    auto tabsNode = FrameNode::CreateFrameNode(V2::TABS_ETS_TAG, 441, AceType::MakeRefPtr<Pattern>(), true);
    auto pattern = AceType::MakeRefPtr<SwiperPattern>();
    auto swiperNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 442, pattern, true);
    ASSERT_NE(tabsNode, nullptr);
    ASSERT_NE(swiperNode, nullptr);
    tabsNode->AddChild(swiperNode);
    pattern->hasTabsAncestor_ = true;
    auto mockUiSessionManager = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(mockUiSessionManager, nullptr);

    EXPECT_CALL(*mockUiSessionManager,
        ReportContentChangeEvent(ChangeType::TABS_START, HasSubstr("\"$ID\":441"))).Times(1);
    manager->OnSwiperScrollStart(swiperNode, true);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    AnimationCallbackInfo info;
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::TABS, Truly([](const auto& json) {
        auto value = JsonUtil::ParseJsonString(json);
        return value && value->GetString("endReason").empty();
    }))).Times(1);
    pattern->FireAnimationEndEvent(0, info, true);
    manager->OnVsyncEnd(RectF());
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeTabsPropertyIndexTest001
 * @tc.desc: Tabs index changes report START before updating the index and preserve END with START disabled.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperTestCore, ContentChangeTabsPropertyIndexTest001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    auto mock = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(mock, nullptr);
    for (bool reportStartEvent : { false, true }) {
        ContentChangeConfig config;
        config.reportStartEvent = reportStartEvent;
        manager->StartContentChangeReport(config);
        auto tabsPattern = AceType::MakeRefPtr<TabsPattern>();
        auto tabs = FrameNode::CreateFrameNode(V2::TABS_ETS_TAG, 445, tabsPattern, true);
        auto tabBar = FrameNode::CreateFrameNode(
            V2::TAB_BAR_ETS_TAG, 446, AceType::MakeRefPtr<TabBarPattern>(), true);
        auto pattern = AceType::MakeRefPtr<SwiperPattern>();
        auto swiper = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 447, pattern, true);
        swiper->MountToParent(tabs);
        for (int32_t index = 0; index < 2; ++index) {
            swiper->AddChild(FrameNode::CreateFrameNode(V2::TAB_CONTENT_ITEM_ETS_TAG, 448 + index,
                AceType::MakeRefPtr<Pattern>(), true));
        }
        auto tabsProperty = tabs->GetLayoutProperty<TabsLayoutProperty>();
        auto swiperProperty = swiper->GetLayoutProperty<SwiperLayoutProperty>();
        tabsProperty->UpdateIndex(0);
        tabsProperty->UpdateIndexSetByUser(1);
        swiperProperty->UpdateIndex(0);
        pattern->isInit_ = false;
        pattern->SetHasTabsAncestor(true);
        if (reportStartEvent) {
            EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::TABS_START, HasSubstr("\"$ID\":445")))
                .WillOnce(Invoke([&](ChangeType, const std::string&) {
                    EXPECT_EQ(tabsProperty->GetIndexValue(0), 0);
                    EXPECT_EQ(swiperProperty->GetIndexValue(0), 0);
                }));
        } else {
            EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::TABS_START, _)).Times(0);
        }
        tabsPattern->UpdateIndex(tabs, tabBar, swiper, tabsProperty);
        EXPECT_EQ(tabsProperty->GetIndexValue(0), 1);
        EXPECT_EQ(swiperProperty->GetIndexValue(0), 1);
        EXPECT_EQ(manager->activeContentChanges_.count(445), reportStartEvent ? 1u : 0u);
        Mock::VerifyAndClearExpectations(mock);

        EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::TABS_START, _)).Times(0);
        tabsProperty->UpdateIndexSetByUser(1);
        tabsPattern->UpdateIndex(tabs, tabBar, swiper, tabsProperty);
        pattern->BeforeCreateLayoutWrapper();
        pattern->BeforeCreateLayoutWrapper();
        EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::TABS, _)).Times(1);
        FinishNoAnimationLayout(swiper, 1);
        EXPECT_TRUE(manager->activeContentChanges_.empty());
        Mock::VerifyAndClearExpectations(mock);
        manager->StopContentChangeReport();
    }
}

/**
 * @tc.name: ContentChangeTabsPropertyIndexTest002
 * @tc.desc: Unchanged, invalid and intercepted Tabs property indices do not report START or change the index.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperTestCore, ContentChangeTabsPropertyIndexTest002, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    auto tabsPattern = AceType::MakeRefPtr<TabsPattern>();
    auto tabs = FrameNode::CreateFrameNode(V2::TABS_ETS_TAG, 452, tabsPattern, true);
    auto tabBar = FrameNode::CreateFrameNode(
        V2::TAB_BAR_ETS_TAG, 453, AceType::MakeRefPtr<TabBarPattern>(), true);
    auto swiper = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 454, AceType::MakeRefPtr<SwiperPattern>(), true);
    swiper->MountToParent(tabs);
    for (int32_t index = 0; index < 2; ++index) {
        swiper->AddChild(FrameNode::CreateFrameNode(V2::TAB_CONTENT_ITEM_ETS_TAG, 455 + index,
            AceType::MakeRefPtr<Pattern>(), true));
    }
    auto tabsProperty = tabs->GetLayoutProperty<TabsLayoutProperty>();
    auto swiperProperty = swiper->GetLayoutProperty<SwiperLayoutProperty>();
    tabsProperty->UpdateIndex(0);
    swiperProperty->UpdateIndex(0);
    auto mock = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(mock, nullptr);
    EXPECT_CALL(*mock, ReportContentChangeEvent(_, _)).Times(0);
    for (int32_t index : { 0, -1, 2 }) {
        tabsProperty->UpdateIndexSetByUser(index);
        tabsPattern->UpdateIndex(tabs, tabBar, swiper, tabsProperty);
        EXPECT_EQ(tabsProperty->GetIndexValue(0), 0);
        EXPECT_EQ(swiperProperty->GetIndexValue(0), 0);
        EXPECT_TRUE(manager->activeContentChanges_.empty());
    }
    bool intercepted = false;
    tabsPattern->SetInterceptStatus(true);
    tabsPattern->SetOnContentWillChange([&](int32_t from, int32_t to) {
        EXPECT_EQ(from, 0);
        EXPECT_EQ(to, 1);
        intercepted = true;
        return false;
    });
    tabsProperty->UpdateIndexSetByUser(1);
    tabsPattern->UpdateIndex(tabs, tabBar, swiper, tabsProperty);
    EXPECT_TRUE(intercepted);
    EXPECT_EQ(tabsProperty->GetIndexValue(0), 0);
    EXPECT_EQ(swiperProperty->GetIndexValue(0), 0);
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mock);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeSwiperStartTest002
 * @tc.desc: Test a Swiper returning to its current index closes the START cycle with CANCEL.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperTestCore, ContentChangeSwiperStartTest002, TestSize.Level1)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    auto manager = pipeline->GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);

    auto pattern = AceType::MakeRefPtr<SwiperPattern>();
    auto swiperNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 451, pattern, true);
    ASSERT_NE(swiperNode, nullptr);
    auto mockUiSessionManager = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(mockUiSessionManager, nullptr);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SWIPER_START, _)).Times(1);
    manager->OnSwiperScrollStart(swiperNode);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    pattern->targetIndex_ = pattern->currentIndex_;
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SWIPER_START_CANCEL,
        HasSubstr("\"$ID\":451"))).Times(1);
    pattern->ContentChangeReport(swiperNode, true);
    manager->OnVsyncEnd(RectF());
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeTabsStartCancelTest001
 * @tc.desc: Test a Tabs transition returning to its current index reports TABS_START_CANCEL.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperTestCore, ContentChangeTabsStartCancelTest001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);

    auto tabsNode = FrameNode::CreateFrameNode(V2::TABS_ETS_TAG, 455, AceType::MakeRefPtr<Pattern>(), true);
    auto pattern = AceType::MakeRefPtr<SwiperPattern>();
    auto swiperNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 456, pattern, true);
    ASSERT_NE(tabsNode, nullptr);
    ASSERT_NE(swiperNode, nullptr);
    swiperNode->MountToParent(tabsNode);
    pattern->SetHasTabsAncestor(true);
    auto mockUiSessionManager = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::TABS_START,
        HasSubstr("\"$ID\":455"))).Times(1);
    manager->OnSwiperScrollStart(swiperNode, true);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    pattern->targetIndex_ = pattern->currentIndex_;
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::TABS_START_CANCEL,
        HasSubstr("\"$ID\":455"))).Times(1);
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::TABS, _)).Times(0);
    pattern->ContentChangeReport(swiperNode, true);
    manager->OnVsyncEnd(RectF());
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::TABS_START,
        HasSubstr("\"$ID\":455"))).Times(1);
    manager->OnSwiperScrollStart(swiperNode, true);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeNestedSwiperTest001
 * @tc.desc: A Swiper inside TabContent has its own START/END identity and does not suppress Tabs START.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperTestCore, ContentChangeNestedSwiperTest001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    auto tabsNode = FrameNode::CreateFrameNode(V2::TABS_ETS_TAG, 461, AceType::MakeRefPtr<Pattern>(), true);
    auto tabsPattern = AceType::MakeRefPtr<SwiperPattern>();
    auto tabsSwiper = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 462, tabsPattern, true);
    tabsPattern->SetHasTabsAncestor(true);
    tabsSwiper->MountToParent(tabsNode);
    auto tabContent =
        FrameNode::CreateFrameNode(V2::TAB_CONTENT_ITEM_ETS_TAG, 463, AceType::MakeRefPtr<Pattern>(), true);
    tabContent->MountToParent(tabsSwiper);
    auto innerPattern = AceType::MakeRefPtr<SwiperPattern>();
    auto innerSwiper = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 464, innerPattern, true);
    innerSwiper->MountToParent(tabContent);
    innerSwiper->AddChild(FrameNode::CreateFrameNode("Text", 465, AceType::MakeRefPtr<Pattern>()));
    auto mockUiSessionManager = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::TABS_START, _)).Times(0);
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SWIPER_START, HasSubstr("\"$ID\":464")))
        .Times(1);
    innerPattern->FireScrollStateEvent(ScrollState::SCROLL);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SWIPER, _)).Times(1);
    innerPattern->FireScrollStateEvent(ScrollState::IDLE);
    innerPattern->ContentChangeReport(innerSwiper, true);
    manager->OnVsyncEnd(RectF());
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::TABS_START, HasSubstr("\"$ID\":461")))
        .Times(1);
    tabsPattern->FireScrollStateEvent(ScrollState::FLING);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeTabsCustomTransitionTest001
 * @tc.desc: Custom Tabs transitions report START before the callback and close through the normal END.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperTestCore, ContentChangeTabsCustomTransitionTest001, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    auto mockUiSessionManager = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    for (bool reportStartEvent : { false, true }) {
        ContentChangeConfig config;
        config.reportStartEvent = reportStartEvent;
        manager->StartContentChangeReport(config);
        auto tabsNode = FrameNode::CreateFrameNode(V2::TABS_ETS_TAG, 471, AceType::MakeRefPtr<Pattern>(), true);
        auto pattern = AceType::MakeRefPtr<SwiperPattern>();
        auto swiperNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 472, pattern, true);
        swiperNode->MountToParent(tabsNode);
        pattern->SetHasTabsAncestor(true);
        RefPtr<TabContentTransitionProxy> proxy;
        pattern->SetTabsCustomContentTransition([&](int32_t, int32_t) {
            EXPECT_EQ(manager->activeContentChanges_.empty(), !reportStartEvent);
            return TabContentAnimatedTransition { .timeout = 1000,
                .transition = [&](const RefPtr<TabContentTransitionProxy>& value) { proxy = value; } };
        });

        EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::TABS_START, HasSubstr("\"$ID\":471")))
            .Times(reportStartEvent ? 1 : 0);
        pattern->OnCustomContentTransition(1);
        ASSERT_NE(proxy, nullptr);
        EXPECT_FALSE(manager->IsSwiperScrolling());
        Mock::VerifyAndClearExpectations(mockUiSessionManager);

        EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::TABS, _)).Times(1);
        proxy->FinishTransition();
        manager->OnVsyncEnd(RectF());
        EXPECT_TRUE(manager->activeContentChanges_.empty());
        EXPECT_FALSE(manager->IsContentChanging());
        Mock::VerifyAndClearExpectations(mockUiSessionManager);
        manager->StopContentChangeReport();
    }
}


/**
 * @tc.name: CapiFirstSwiperLayoutAndSameIndexCompletion
 * @tc.desc: An initial layout with a normal END starts once; an unfinished same-index command closes once.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperTestCore, CapiFirstSwiperLayoutAndSameIndexCompletion, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    std::vector<uint32_t> a;
    std::vector<uint32_t> b;
    auto sink = [](uint32_t type, uint64_t, const char*, uint32_t, void* data) {
        static_cast<std::vector<uint32_t>*>(data)->push_back(type);
    };
    auto aid = manager->RegisterCapiContentChangeEvent(true, 1, 0, &a, sink, nullptr);
    auto bid = manager->RegisterCapiContentChangeEvent(false, 1, 0, &b, sink, nullptr);
    auto pattern = AceType::MakeRefPtr<SwiperPattern>();
    auto node = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 498, pattern, true);
    for (int i = 0; i < 2; ++i) {
        node->AddChild(FrameNode::CreateFrameNode("Text", 499 + i, AceType::MakeRefPtr<Pattern>(), true));
    }
    node->GetLayoutProperty<SwiperLayoutProperty>()->UpdateShowIndicator(false);
    pattern->isInit_ = true;
    pattern->jumpIndex_ = 0;
    FinishNoAnimationLayout(node, 0);
    EXPECT_EQ(a, (std::vector<uint32_t> {
        OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_START, OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_END }));
    EXPECT_EQ(b, (std::vector<uint32_t> { OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_END }));
    a.clear();
    b.clear();
    manager->OnSwiperChangeStart(node, false);
    pattern->ContentChangeReport(node, false);
    manager->OnVsyncEnd(RectF());
    EXPECT_EQ(a, (std::vector<uint32_t> {
        OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_START, OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_CANCEL }));
    EXPECT_TRUE(b.empty());
    pattern->ContentChangeReport(node, false);
    manager->OnVsyncEnd(RectF());
    EXPECT_EQ(a.size(), 2U);
    manager->UnregisterCapiContentChangeEvent(aid);
    manager->UnregisterCapiContentChangeEvent(bid);
}

/**
 * @tc.name: CapiAutoPlayPropertyChangeDropsStartBeforeLayout
 * @tc.desc: Enabling autoPlay clears manual START before layout or destruction can report a terminal event.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperTestCore, CapiAutoPlayPropertyChangeDropsStartBeforeLayout, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    std::vector<uint32_t> events;
    auto sink = [](uint32_t type, uint64_t, const char*, uint32_t, void* data) {
        static_cast<std::vector<uint32_t>*>(data)->push_back(type);
    };
    auto id = manager->RegisterCapiContentChangeEvent(true, 1, 0, &events, sink, nullptr);
    auto pattern = AceType::MakeRefPtr<SwiperPattern>();
    auto node = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 501, pattern, true);
    manager->OnSwiperChangeStart(node, false);
    node->GetPaintProperty<SwiperPaintProperty>()->UpdateAutoPlay(true);
    pattern->OnModifyDone();
    manager->OnContentChangeNodeDestroyed(node->GetId());
    manager->OnVsyncEnd(RectF());
    EXPECT_EQ(events, (std::vector<uint32_t> { OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_START }));
    manager->UnregisterCapiContentChangeEvent(id);
}

/**
 * @tc.name: CapiNormalizedIndexReportsOnlyActualContentChange
 * @tc.desc: An out-of-range property corrected to another item starts; correction to the same item stays silent.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperTestCore, CapiNormalizedIndexReportsOnlyActualContentChange, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    std::vector<uint32_t> events;
    auto sink = [](uint32_t type, uint64_t, const char*, uint32_t, void* data) {
        static_cast<std::vector<uint32_t>*>(data)->push_back(type);
    };
    auto id = manager->RegisterCapiContentChangeEvent(true, 1, 0, &events, sink, nullptr);
    auto pattern = AceType::MakeRefPtr<SwiperPattern>();
    auto node = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 502, pattern, true);
    for (int i = 0; i < 3; ++i) {
        node->AddChild(FrameNode::CreateFrameNode("Text", 503 + i, AceType::MakeRefPtr<Pattern>(), true));
    }
    auto layout = node->GetLayoutProperty<SwiperLayoutProperty>();
    layout->UpdateShowIndicator(false);
    layout->UpdateLoop(false);
    pattern->isInit_ = false;
    pattern->UpdateCurrentIndex(1);
    pattern->oldIndex_ = 1;
    layout->UpdateIndex(99);
    pattern->BeforeCreateLayoutWrapper();
    EXPECT_EQ(pattern->currentIndex_, 0);
    EXPECT_EQ(events, (std::vector<uint32_t> { OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_START }));
    FinishNoAnimationLayout(node, 0);
    EXPECT_EQ(events, (std::vector<uint32_t> {
        OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_START, OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_END }));
    events.clear();
    layout->UpdateIndex(99);
    pattern->BeforeCreateLayoutWrapper();
    FinishNoAnimationLayout(node, 0);
    EXPECT_TRUE(events.empty());
    manager->UnregisterCapiContentChangeEvent(id);
}

/**
 * @tc.name: EdgeAnimationCompletesWithCancel
 * @tc.desc: Spring/fade completion at the same index cancels the active cycle once for Swiper and Tabs.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperTestCore, EdgeAnimationCompletesWithCancel, TestSize.Level1)
{
    auto manager = MockPipelineContext::GetCurrent()->GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    std::vector<uint32_t> a;
    std::vector<uint32_t> b;
    auto sink = [](uint32_t type, uint64_t, const char*, uint32_t, void* data) {
        static_cast<std::vector<uint32_t>*>(data)->push_back(type);
    };
    auto aid = manager->RegisterCapiContentChangeEvent(true, 1, 0, &a, sink, nullptr);
    auto bid = manager->RegisterCapiContentChangeEvent(false, 1, 0, &b, sink, nullptr);
    for (bool tabs : { false, true }) {
        auto pattern = AceType::MakeRefPtr<SwiperPattern>();
        auto node = FrameNode::CreateFrameNode(
            V2::SWIPER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), pattern);
        auto parent = FrameNode::CreateFrameNode(
            V2::TABS_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        if (tabs) {
            node->MountToParent(parent);
        }
        pattern->hasTabsAncestor_ = tabs;
        node->GetLayoutProperty<SwiperLayoutProperty>()->UpdateLoop(false);
        for (int i = 0; i < 3; ++i) {
            node->AddChild(FrameNode::CreateFrameNode(
                "Text", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>()));
        }
        for (int index : { 0, 2 }) {
            a.clear();
            b.clear();
            pattern->currentIndex_ = index;
            pattern->nextIndex_ = index;
            manager->OnSwiperChangeStart(node, tabs);
            pattern->OnSpringAndFadeAnimationFinish();
            pattern->OnSpringAndFadeAnimationFinish();
            manager->OnVsyncEnd(RectF());
            EXPECT_EQ(a, (std::vector<uint32_t> { tabs ? 9U : 6U, tabs ? 11U : 8U }));
            EXPECT_TRUE(b.empty());
        }
        a.clear();
        b.clear();
        pattern->currentIndex_ = 0;
        pattern->nextIndex_ = 1;
        pattern->targetIndex_ = 1;
        manager->OnSwiperChangeStart(node, tabs);
        pattern->OnSpringAndFadeAnimationFinish();
        manager->OnVsyncEnd(RectF());
        EXPECT_EQ(a, (std::vector<uint32_t> { tabs ? 9U : 6U, tabs ? 10U : 7U }));
        EXPECT_EQ(b, (std::vector<uint32_t> { tabs ? 10U : 7U }));
    }
    manager->UnregisterCapiContentChangeEvent(aid);
    manager->UnregisterCapiContentChangeEvent(bid);
}

class SwiperDeviceRegressionTest : public SwiperTestCore {
public:
    void SetUp() override
    {
        manager_ = MockPipelineContext::GetCurrent()->GetContentChangeManager();
        manager_->StopContentChangeReport();
        manager_->InvalidateCapiContentChangeSubscriptions();
        auto sink = [](uint32_t type, uint64_t, const char*, uint32_t, void* data) {
            static_cast<std::vector<uint32_t>*>(data)->push_back(type);
        };
        aid_ = manager_->RegisterCapiContentChangeEvent(true, 1, 0, &a_, sink, nullptr);
        bid_ = manager_->RegisterCapiContentChangeEvent(false, 1, 0, &b_, sink, nullptr);
    }

    void TearDown() override
    {
        manager_->UnregisterCapiContentChangeEvent(aid_);
        manager_->UnregisterCapiContentChangeEvent(bid_);
    }

    RefPtr<FrameNode> CreateSwiper(bool tabs, int count)
    {
        auto node = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SwiperPattern>());
        parent_ = FrameNode::CreateFrameNode(V2::TABS_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        if (tabs) {
            node->MountToParent(parent_);
        }
        node->GetPattern<SwiperPattern>()->hasTabsAncestor_ = tabs;
        auto layout = node->GetLayoutProperty<SwiperLayoutProperty>();
        layout->UpdateShowIndicator(false);
        layout->UpdateLoop(false);
        for (int i = 0; i < count; ++i) {
            node->AddChild(FrameNode::CreateFrameNode("Text",
                ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>()));
        }
        return node;
    }

    RefPtr<ContentChangeManager> manager_;
    RefPtr<FrameNode> parent_;
    std::vector<uint32_t> a_;
    std::vector<uint32_t> b_;
    uint64_t aid_ = 0;
    uint64_t bid_ = 0;
};

/**
 * @tc.name: EmptyLayoutAndDragStaySilent
 * @tc.desc: Empty Swiper and Tabs neither report initial END nor accept a gesture cycle.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperDeviceRegressionTest, EmptyLayoutAndDragStaySilent, TestSize.Level1)
{
    for (bool tabs : { false, true }) {
        auto node = CreateSwiper(tabs, 0);
        auto pattern = node->GetPattern<SwiperPattern>();
        pattern->jumpIndex_ = 0;
        FinishNoAnimationLayout(node, 0);
        pattern->FireScrollStateEvent(ScrollState::SCROLL);
        pattern->OnSpringAndFadeAnimationFinish();
        manager_->OnVsyncEnd(RectF());
        EXPECT_TRUE(a_.empty());
        EXPECT_TRUE(b_.empty());
    }
}

/**
 * @tc.name: FadeBoundaryReportsCancel
 * @tc.desc: Visible Fade at either edge starts a cycle and restoring that edge cancels it once.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperDeviceRegressionTest, FadeBoundaryReportsCancel, TestSize.Level1)
{
    for (bool tabs : { false, true }) {
        for (int index : { 0, 2 }) {
            auto node = CreateSwiper(tabs, 3);
            auto pattern = node->GetPattern<SwiperPattern>();
            pattern->contentMainSize_ = 480.0f;
            pattern->currentIndex_ = index;
            pattern->nextIndex_ = index;
            pattern->itemPosition_[index] = { 0.0f, 480.0f };
            node->GetPaintProperty<SwiperPaintProperty>()->UpdateEdgeEffect(EdgeEffect::FADE);
            int businessStateChanges = 0;
            node->GetEventHub<SwiperEventHub>()->AddOnScrollStateChangedEvent(
                std::make_shared<ChangeEvent>([&businessStateChanges](int32_t) { ++businessStateChanges; }));
            const auto previousState = pattern->scrollState_;
            EXPECT_TRUE(pattern->FadeOverScroll(index == 0 ? 100.0f : -100.0f));
            EXPECT_EQ(pattern->scrollState_, previousState);
            EXPECT_EQ(businessStateChanges, 0);
            EXPECT_FALSE(NearZero(pattern->fadeOffset_));
            EXPECT_EQ(a_, (std::vector<uint32_t> { tabs ? 9U : 6U }));
            pattern->OnSpringAndFadeAnimationFinish();
            pattern->OnSpringAndFadeAnimationFinish();
            manager_->OnVsyncEnd(RectF());
            EXPECT_EQ(a_, (std::vector<uint32_t> { tabs ? 9U : 6U, tabs ? 11U : 8U }));
            EXPECT_TRUE(b_.empty());
            a_.clear();
        }
    }
}

/**
 * @tc.name: ZeroDistanceCompletionClosesFakeDrag
 * @tc.desc: Returning to the original index completes an accepted drag with CANCEL, never END.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperDeviceRegressionTest, ZeroDistanceCompletionClosesFakeDrag, TestSize.Level1)
{
    auto node = CreateSwiper(false, 3);
    auto pattern = node->GetPattern<SwiperPattern>();
    pattern->contentMainSize_ = 480.0f;
    pattern->itemPosition_[0] = { 0.0f, 480.0f };
    pattern->itemPosition_[1] = { 480.0f, 960.0f };
    pattern->isFakeDragging_ = true;
    EXPECT_TRUE(pattern->FakeDragBy(-180.0f));
    pattern->lastDragByOffset_.reset();
    pattern->itemPosition_[0] = { -180.0f, 300.0f };
    EXPECT_TRUE(pattern->FakeDragBy(180.0f));
    pattern->PlayTranslateAnimation(0.0f, 0.0f, 0, false, 0.0f);
    manager_->OnVsyncEnd(RectF());
    EXPECT_EQ(a_, (std::vector<uint32_t> { 6U, 8U }));
    EXPECT_TRUE(b_.empty());
    a_.clear();
    pattern->PlayTranslateAnimation(0.0f, 0.0f, 0, false, 0.0f);
    manager_->OnVsyncEnd(RectF());
    EXPECT_TRUE(a_.empty());
}

/**
 * @tc.name: RetainedDetachedSwiperIgnoresLateFinish
 * @tc.desc: Removing a live Swiper or its Tabs ancestor ends once even when the retained animation finishes later.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperDeviceRegressionTest, RetainedDetachedSwiperIgnoresLateFinish, TestSize.Level1)
{
    for (bool tabs : { false, true }) {
        auto node = CreateSwiper(tabs, 3);
        auto pattern = node->GetPattern<SwiperPattern>();
        auto root = tabs ? parent_ : node;
        auto pipeline = MockPipelineContext::GetCurrent();
        root->AttachToMainTree(true, AceType::RawPtr(pipeline));
        pattern->FireScrollStateEvent(ScrollState::SCROLL);
        root->DetachFromMainTree();
        manager_->OnVsyncEnd(RectF());
        pattern->ContentChangeReport(node, true);
        manager_->OnVsyncEnd(RectF());
        EXPECT_EQ(a_, (std::vector<uint32_t> { tabs ? 9U : 6U, tabs ? 10U : 7U }));
        EXPECT_EQ(b_, (std::vector<uint32_t> { tabs ? 10U : 7U }));
        a_.clear();
        b_.clear();
    }
}

/**
 * @tc.name: SelectedTabIdentityChangesAtSameIndex
 * @tc.desc: Replacing the visible child reports once; deleting another child is silent in normal/custom modes.
 * @tc.type: FUNC
 */
HWTEST_F(SwiperDeviceRegressionTest, SelectedTabIdentityChangesAtSameIndex, TestSize.Level1)
{
    for (bool custom : { false, true }) {
        auto node = CreateSwiper(true, 3);
        auto pattern = node->GetPattern<SwiperPattern>();
        node->GetLayoutProperty<SwiperLayoutProperty>()->UpdateIsCustomAnimation(custom);
        pattern->jumpIndex_ = 0;
        FinishNoAnimationLayout(node, 0);
        a_.clear();
        b_.clear();
        node->RemoveChild(node->GetChildAtIndex(2));
        FinishNoAnimationLayout(node, 0);
        EXPECT_TRUE(a_.empty());
        EXPECT_TRUE(b_.empty());
        node->RemoveChild(node->GetChildAtIndex(0));
        FinishNoAnimationLayout(node, 0);
        EXPECT_EQ(a_, (std::vector<uint32_t> { 9U, 10U }));
        EXPECT_EQ(b_, (std::vector<uint32_t> { 10U }));
        FinishNoAnimationLayout(node, 0);
        EXPECT_EQ(a_.size(), 2U);
        EXPECT_EQ(b_.size(), 1U);
        a_.clear();
        b_.clear();
    }
}

} // namespace OHOS::Ace::NG
