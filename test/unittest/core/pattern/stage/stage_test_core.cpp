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

#include "stage_test_min.h"

#define private public
#define protected public

#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "test/mock/interfaces/inner_api/ui_session/mock_ui_session_manager.h"

#include "core/components_ng/manager/content_change_manager/content_change_manager.h"
#include "core/components_ng/pattern/stage/page_pattern.h"
#include "core/components_ng/pattern/stage/stage_manager.h"
#include "core/components_ng/pattern/stage/stage_pattern.h"
#include "interfaces/native/ui_info_collection.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;


namespace OHOS::Ace::NG {

// Test classes for TDD development (quick compile mode)
// These classes use different names from full test suite to avoid conflicts
// When ace_enable_full_test_suite=true, full test files will be compiled instead

class StagePatternTestCore : public StageTestBase {
public:
    static void SetUpTestCase()
    {
        MockPipelineContext::SetUp();
    }

    static void TearDownTestCase()
    {
        MockPipelineContext::TearDown();
    }
};

class ContentChangePagePattern : public PagePattern {
    DECLARE_ACE_TYPE(ContentChangePagePattern, PagePattern);

public:
    ContentChangePagePattern() : PagePattern(AceType::MakeRefPtr<PageInfo>()) {}

    void TriggerDefaultTransition(
        const std::function<void()>& onFinish, PageTransitionType /* type */) override
    {
        transitionFinish_ = onFinish;
    }

    std::function<void()> transitionFinish_;
};

// Placeholder test to ensure compilation
HWTEST_F(StagePatternTestCore, StageTestPlaceholder, TestSize.Level1)
{
    EXPECT_TRUE(true);
}

/**
 * @tc.name: ContentChangePageStartTest001
 * @tc.desc: Test only entering page transitions report START.
 * @tc.type: FUNC
 */
HWTEST_F(StagePatternTestCore, ContentChangePageStartTest001, TestSize.Level1)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    auto manager = pipeline->GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);

    auto pattern = AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>());
    auto pageNode = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, 401, pattern, true);
    ASSERT_NE(pageNode, nullptr);
    auto mockUiSessionManager = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(mockUiSessionManager, nullptr);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::PAGE_START, _)).Times(0);
    pattern->FirePageTransitionStart(PageTransitionType::EXIT_PUSH);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::PAGE_START, _)).Times(1);
    pattern->FirePageTransitionStart(PageTransitionType::ENTER_PUSH);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangePageStartTest002
 * @tc.desc: Test an interrupted page transition keeps the existing animation completion END.
 * @tc.type: FUNC
 */
HWTEST_F(StagePatternTestCore, ContentChangePageStartTest002, TestSize.Level1)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    auto manager = pipeline->GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);

    auto pattern = AceType::MakeRefPtr<ContentChangePagePattern>();
    auto pageNode = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, 402, pattern, true);
    ASSERT_NE(pageNode, nullptr);
    auto mockUiSessionManager = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(mockUiSessionManager, nullptr);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::PAGE_START, _)).Times(1);
    pattern->TriggerPageTransition({}, PageTransitionType::ENTER_PUSH);
    ASSERT_TRUE(pattern->transitionFinish_);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::PAGE, _)).Times(0);
    pattern->StopPageTransition();
    EXPECT_FALSE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::PAGE, Truly([](const auto& json) {
        auto value = JsonUtil::ParseJsonString(json);
        return value && value->GetString("endReason").empty();
    }))).Times(1);
    pattern->transitionFinish_();
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangePageStartTest003
 * @tc.desc: Test stale transition completion keeps the legacy normal END behavior when START reporting is disabled.
 * @tc.type: FUNC
 */
HWTEST_F(StagePatternTestCore, ContentChangePageStartTest003, TestSize.Level1)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    auto manager = pipeline->GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    manager->StartContentChangeReport({});

    auto pattern = AceType::MakeRefPtr<ContentChangePagePattern>();
    auto pageNode = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, 403, pattern, true);
    ASSERT_NE(pageNode, nullptr);
    auto mockUiSessionManager = reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(mockUiSessionManager, nullptr);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::PAGE_START, _)).Times(0);
    pattern->TriggerPageTransition({}, PageTransitionType::ENTER_PUSH);
    ASSERT_TRUE(pattern->transitionFinish_);
    pattern->StopPageTransition();
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::PAGE, _)).Times(1);
    pattern->transitionFinish_();
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: CapiPopWithoutAnimationReportsOnlyVisibleChange
 * @tc.desc: A visible no-animation back reports START/END; background cleanup creates no page event.
 * @tc.type: FUNC
 */
HWTEST_F(StagePatternTestCore, CapiPopWithoutAnimationReportsOnlyVisibleChange, TestSize.Level1)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    pipeline->SetUseFlushUITasks(true);
    auto manager = pipeline->GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    std::vector<uint32_t> events;
    auto sink = [](uint32_t type, uint64_t, const char*, uint32_t, void* data) {
        static_cast<std::vector<uint32_t>*>(data)->push_back(type);
    };
    auto id = manager->RegisterCapiContentChangeEvent(true, 1, 0, &events, sink, nullptr);
    for (bool showNext : { false, true }) {
        auto stage = FrameNode::CreateFrameNode(V2::STAGE_ETS_TAG, 8700, AceType::MakeRefPtr<StagePattern>());
        auto in = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, 8701,
            AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
        auto out = FrameNode::CreateFrameNode(V2::PAGE_ETS_TAG, 8702,
            AceType::MakeRefPtr<PagePattern>(AceType::MakeRefPtr<PageInfo>()));
        in->MountToParent(stage);
        out->MountToParent(stage);
        auto stageManager = AceType::MakeRefPtr<StageManager>(stage);
        stageManager->srcPageNode_ = out;
        ASSERT_TRUE(stageManager->PopPage(in, showNext, false));
        EXPECT_EQ(events.size(), showNext ? 1U : 0U);
        pipeline->taskScheduler_->FlushAfterLayoutTask();
        if (showNext) {
            EXPECT_EQ(events, (std::vector<uint32_t> {
                OH_ARKUI_NATIVEMODULE_EVENT_PAGE_CHANGE_START, OH_ARKUI_NATIVEMODULE_EVENT_PAGE_CHANGE_END }));
        } else {
            EXPECT_TRUE(events.empty());
        }
    }
    manager->UnregisterCapiContentChangeEvent(id);
    pipeline->SetUseFlushUITasks(false);
}

} // namespace OHOS::Ace::NG
