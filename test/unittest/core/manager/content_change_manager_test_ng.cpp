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

#include <optional>
#include <vector>

#include "interfaces/native/ui_info_collection.h"

#include "gtest/gtest.h"
#define private public
#define protected public

#include "test/mock/interfaces/inner_api/ui_session/mock_ui_session_manager.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "core/components_ng/manager/content_change_manager/content_change_manager_impl.h"
#include "core/pipeline/base/element_register.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t INITIAL_NODE_SIZE = 5;
constexpr int32_t INITIAL_NODE_SIZE_PLUS_ONE = INITIAL_NODE_SIZE + 1;
constexpr int32_t NORMAL_NODE_ID = 0;
constexpr int32_t TEST_SCROLLING_NODE_ID = 100;
constexpr int32_t TEST_TRANSITIONING_NODE_ID = 200;
constexpr int32_t NEVER_ONCE = 0;
constexpr int32_t AT_LEAST_ONCE = 1;

class CapiTreeReadProbe : public UINode {
    DECLARE_ACE_TYPE(CapiTreeReadProbe, UINode);

public:
    explicit CapiTreeReadProbe(int32_t id) : UINode("CapiTreeReadProbe", id) {}

    bool IsAtomicNode() const override
    {
        return false;
    }

    const std::list<RefPtr<UINode>>& GetChildren(bool notDetach = false) const override
    {
        ++reads;
        return UINode::GetChildren(notDetach);
    }

    mutable size_t reads = 0;
};

RefPtr<ContentChangeManager> GetContentChangeManager()
{
    auto pipeline = MockPipelineContext::GetCurrent();
    return pipeline ? pipeline->GetContentChangeManager() : nullptr;
}

MockUiSessionManager* GetMockUiSessionManager()
{
    return reinterpret_cast<MockUiSessionManager*>(UiSessionManager::GetInstance());
}

std::string GetSimpleTree(const RefPtr<FrameNode>& node)
{
    auto simpleTree = JsonUtil::CreateSharedPtrJson(true);
    node->DumpSimplifyTreeWithParamConfig(0, simpleTree, false, {false, false, false});
    return simpleTree->ToString();
}

std::string GetDialogSimpleTree(const RefPtr<FrameNode>& node, bool isShow)
{
    auto simpleTree = JsonUtil::CreateSharedPtrJson(true);
    if (isShow) {
        node->DumpSimplifyTreeWithParamConfig(0, simpleTree, false, {false, false, false});
    } else {
        simpleTree->Put("$type", node->GetTag().c_str());
    }
    simpleTree->Put("show", isShow);
    return simpleTree->ToString();
}

void SetOnContentChangeNodes()
{
    auto contentChangeMgr = GetContentChangeManager();
    if (contentChangeMgr) {
        for (int32_t i = 0; i < INITIAL_NODE_SIZE; i++) {
            auto node = FrameNode::CreateFrameNode("frameNode", i, AceType::MakeRefPtr<Pattern>(), true);
            auto weak = AceType::WeakClaim(AceType::RawPtr(node));
            contentChangeMgr->onContentChangeNodes_.emplace(weak);
        }
    }
}

void ResetOnContentChangeNodes()
{
    auto contentChangeMgr = GetContentChangeManager();
    if (contentChangeMgr) {
        contentChangeMgr->onContentChangeNodes_.clear();
    }
}

void SetScrollingNodes()
{
    auto contentChangeMgr = GetContentChangeManager();
    if (contentChangeMgr) {
        contentChangeMgr->scrollingNodes_.emplace(TEST_SCROLLING_NODE_ID);
    }
}

void ResetScrollingNodes()
{
    auto contentChangeMgr = GetContentChangeManager();
    if (contentChangeMgr) {
        contentChangeMgr->scrollingNodes_.clear();
    }
}

void SetChangedSwiperNodes()
{
    auto contentChangeMgr = GetContentChangeManager();
    if (contentChangeMgr) {
        for (int32_t i = 0; i < INITIAL_NODE_SIZE; i++) {
            auto node = FrameNode::CreateFrameNode("frameNode", i, AceType::MakeRefPtr<Pattern>(), true);
            auto weak = AceType::WeakClaim(AceType::RawPtr(node));
            bool flag = (i % 2 == 0);
            contentChangeMgr->changedSwiperNodes_.emplace(std::make_pair(weak, flag));
        }
    }
}

void ResetChangedSwiperNodes()
{
    auto contentChangeMgr = GetContentChangeManager();
    if (contentChangeMgr) {
        contentChangeMgr->changedSwiperNodes_.clear();
    }
}

void SetTransitioningNodes()
{
    auto contentChangeMgr = GetContentChangeManager();
    if (contentChangeMgr) {
        contentChangeMgr->transitioningNodes_.emplace(TEST_TRANSITIONING_NODE_ID);
    }
}

void ResetTransitioningNodes()
{
    auto contentChangeMgr = GetContentChangeManager();
    if (contentChangeMgr) {
        contentChangeMgr->transitioningNodes_.clear();
    }
}
} // namespace

class ContentChangeManagerTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void ContentChangeManagerTestNg::SetUpTestCase()
{
    MockPipelineContext::SetUp();
}

void ContentChangeManagerTestNg::TearDownTestCase()
{
    MockPipelineContext::TearDown();
}

/**
 * @tc.name: ContentChangeManagerTest001
 * @tc.desc: Test config.textContentRatio validity
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    const float DEFAULT_TEXT_CONTENT_RATIO = contentChangeMgr->DEFAULT_TEXT_CONTENT_RATIO;

    /**
     * @tc.steps: step2. call StartContentChangeReport when textContentRatio is less than 0.0.
     * @tc.expected: textContentRatio is set to default value.
     */
    ContentChangeConfig config;
    config.textContentRatio = -0.5f;
    contentChangeMgr->StartContentChangeReport(config);
    std::optional<ContentChangeConfig> currentConfig = contentChangeMgr->currentContentChangeConfig_;
    ASSERT_TRUE(currentConfig.has_value());
    EXPECT_FLOAT_EQ(currentConfig->textContentRatio, DEFAULT_TEXT_CONTENT_RATIO);
    EXPECT_FLOAT_EQ(contentChangeMgr->textContentRatio_, DEFAULT_TEXT_CONTENT_RATIO);

    /**
     * @tc.steps: step3. call StartContentChangeReport when textContentRatio is greater than 1.0.
     * @tc.expected: textContentRatio is set to default value.
     */
    config.textContentRatio = 1.5f;
    contentChangeMgr->StartContentChangeReport(config);
    currentConfig = contentChangeMgr->currentContentChangeConfig_;
    ASSERT_TRUE(currentConfig.has_value());
    EXPECT_FLOAT_EQ(currentConfig->textContentRatio, DEFAULT_TEXT_CONTENT_RATIO);
    EXPECT_FLOAT_EQ(contentChangeMgr->textContentRatio_, DEFAULT_TEXT_CONTENT_RATIO);

    /**
     * @tc.steps: step4. call StartContentChangeReport when textContentRatio is valid.
     * @tc.expected: textContentRatio is set accordingly.
     */
    config.textContentRatio = 0.0f;
    contentChangeMgr->StartContentChangeReport(config);
    currentConfig = contentChangeMgr->currentContentChangeConfig_;
    ASSERT_TRUE(currentConfig.has_value());
    EXPECT_FLOAT_EQ(currentConfig->textContentRatio, 0.0f);
    EXPECT_FLOAT_EQ(contentChangeMgr->textContentRatio_, 0.0f);

    config.textContentRatio = 0.1f;
    contentChangeMgr->StartContentChangeReport(config);
    currentConfig = contentChangeMgr->currentContentChangeConfig_;
    ASSERT_TRUE(currentConfig.has_value());
    EXPECT_FLOAT_EQ(currentConfig->textContentRatio, 0.1f);
    EXPECT_FLOAT_EQ(contentChangeMgr->textContentRatio_, 0.1f);

    config.textContentRatio = 1.0f;
    contentChangeMgr->StartContentChangeReport(config);
    currentConfig = contentChangeMgr->currentContentChangeConfig_;
    ASSERT_TRUE(currentConfig.has_value());
    EXPECT_FLOAT_EQ(currentConfig->textContentRatio, 1.0f);
    EXPECT_FLOAT_EQ(contentChangeMgr->textContentRatio_, 1.0f);

    /**
     * @tc.steps: step5. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
}

/**
 * @tc.name: ContentChangeManagerTest002
 * @tc.desc: Test config.minReportTime validity
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    const float DEFAULT_COMPONENT_MIN_REPORT_TIME = contentChangeMgr->DEFAULT_COMPONENT_MIN_REPORT_TIME;
    const uint64_t NS_PER_MS = contentChangeMgr->NS_PER_MS;

    /**
     * @tc.steps: step2. call StartContentChangeReport when minReportTime is less than 0.
     * @tc.expected: minReportTime is set to default value.
     */
    ContentChangeConfig config;
    config.minReportTime = -1;
    contentChangeMgr->StartContentChangeReport(config);
    std::optional<ContentChangeConfig> currentConfig = contentChangeMgr->currentContentChangeConfig_;
    ASSERT_TRUE(currentConfig.has_value());
    EXPECT_EQ(currentConfig->minReportTime, DEFAULT_COMPONENT_MIN_REPORT_TIME);
    EXPECT_EQ(contentChangeMgr->componentReportInterval_, DEFAULT_COMPONENT_MIN_REPORT_TIME * NS_PER_MS);

    /**
     * @tc.steps: step3. call StartContentChangeReport when minReportTime is valid.
     * @tc.expected: minReportTime is set accordingly.
     */
    config.minReportTime = 200;
    contentChangeMgr->StartContentChangeReport(config);
    currentConfig = contentChangeMgr->currentContentChangeConfig_;
    ASSERT_TRUE(currentConfig.has_value());
    EXPECT_EQ(currentConfig->minReportTime, 200);
    EXPECT_EQ(contentChangeMgr->componentReportInterval_, 200 * NS_PER_MS);

    /**
     * @tc.steps: step4. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
}

/**
 * @tc.name: ContentChangeManagerTest003
 * @tc.desc: Test StartContentChangeReport, StopContentChangeReport
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. test add one node.
     * @tc.expected: size is correct.
     */
    SetOnContentChangeNodes();
    auto node = FrameNode::CreateFrameNode("frameNode", INITIAL_NODE_SIZE, AceType::MakeRefPtr<Pattern>(), true);
    auto weak = AceType::WeakClaim(AceType::RawPtr(node));
    contentChangeMgr->onContentChangeNodes_.emplace(weak);
    weak.Reset();
    auto size = static_cast<int32_t>(contentChangeMgr->onContentChangeNodes_.size());
    EXPECT_EQ(size, INITIAL_NODE_SIZE_PLUS_ONE);

    /**
     * @tc.steps: step3. test StartContentChangeReport.
     * @tc.expected: IsContentChangeDetectEnable returns true.
     */
    ContentChangeConfig config;
    contentChangeMgr->StartContentChangeReport(config);
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());

    /**
     * @tc.steps: step4. test StopContentChangeReport.
     * @tc.expected: IsContentChangeDetectEnable returns false.
     */
    contentChangeMgr->StopContentChangeReport();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());

    /**
     * @tc.steps: step5. reset.
     */
    ResetOnContentChangeNodes();
    size = static_cast<int32_t>(contentChangeMgr->onContentChangeNodes_.size());
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: ContentChangeManagerTest004
 * @tc.desc: Test AddOnContentChangeNode, RemoveOnContentChangeNode
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. test AddOnContentChangeNode.
     * @tc.expected: size is correct.
     */
    SetOnContentChangeNodes();
    auto node = FrameNode::CreateFrameNode("frameNode", INITIAL_NODE_SIZE, AceType::MakeRefPtr<Pattern>(), true);
    auto weak = AceType::WeakClaim(AceType::RawPtr(node));
    contentChangeMgr->AddOnContentChangeNode(weak);
    auto size = static_cast<int32_t>(contentChangeMgr->onContentChangeNodes_.size());
    EXPECT_EQ(size, INITIAL_NODE_SIZE_PLUS_ONE);

    /**
     * @tc.steps: step3. test RemoveOnContentChangeNode.
     * @tc.expected: size is correct.
     */
    contentChangeMgr->RemoveOnContentChangeNode(weak);
    size = static_cast<int32_t>(contentChangeMgr->onContentChangeNodes_.size());
    EXPECT_EQ(size, INITIAL_NODE_SIZE);

    /**
     * @tc.steps: step4. when IsContentChangeDetectEnable, test AddOnContentChangeNode.
     * @tc.expected: size is correct.
     */
    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->AddOnContentChangeNode(weak);
    size = static_cast<int32_t>(contentChangeMgr->onContentChangeNodes_.size());
    EXPECT_EQ(size, INITIAL_NODE_SIZE_PLUS_ONE);

    /**
     * @tc.steps: step5. reset.
     */
    ResetOnContentChangeNodes();
    size = static_cast<int32_t>(contentChangeMgr->onContentChangeNodes_.size());
    EXPECT_EQ(size, 0);
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
}

/**
 * @tc.name: ContentChangeManagerTest005
 * @tc.desc: Test OnPageTransitionEnd
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto node = FrameNode::CreateFrameNode("frameNode", NORMAL_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(node, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);
    auto simpleTree = GetSimpleTree(node);

    /**
     * @tc.steps: step2. call OnPageTransitionEnd under abnormal conditions.
     * @tc.expected: ReportContentChangeEvent never called.
     */
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _))
        .Times(NEVER_ONCE);
    contentChangeMgr->OnPageTransitionEnd(nullptr);

    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _))
        .Times(NEVER_ONCE);
    contentChangeMgr->OnPageTransitionEnd(nullptr);

    /**
     * @tc.steps: step3. call OnPageTransitionEnd under normal conditions.
     * @tc.expected: ReportContentChangeEvent called once.
     */
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::PAGE, simpleTree))
        .Times(AtLeast(AT_LEAST_ONCE));
    contentChangeMgr->OnPageTransitionEnd(node);

    /**
     * @tc.steps: step4. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
}

/**
 * @tc.name: ContentChangeManagerTest006
 * @tc.desc: Test OnScrollChangeEnd with abnormal conditions
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto node = FrameNode::CreateFrameNode("frameNode", NORMAL_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(node, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);

    /**
     * @tc.steps: step2. call OnScrollChangeEnd with nullptr node.
     * @tc.expected: ReportContentChangeEvent never called.
     */
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _))
        .Times(NEVER_ONCE);
    contentChangeMgr->OnScrollChangeEnd(nullptr);

    /**
     * @tc.steps: step3. call OnScrollChangeEnd when detect is disabled.
     * @tc.expected: ReportContentChangeEvent never called, scrollingNodes_ cleaned up.
     */
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _))
        .Times(NEVER_ONCE);
    contentChangeMgr->OnScrollChangeEnd(node);
    EXPECT_EQ(static_cast<int32_t>(contentChangeMgr->scrollingNodes_.count(node->GetId())), 0);

    /**
     * @tc.steps: step4. call OnScrollChangeEnd with node in scrollingNodes but detect disabled.
     * @tc.expected: ReportContentChangeEvent never called, node removed from scrollingNodes_.
     */
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->scrollingNodes_.emplace(node->GetId());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _)).Times(NEVER_ONCE);
    contentChangeMgr->OnScrollChangeEnd(node);
    EXPECT_EQ(static_cast<int32_t>(contentChangeMgr->scrollingNodes_.count(node->GetId())), 0);

    /**
     * @tc.steps: step5. call OnScrollChangeEnd when other nodes are still scrolling.
     * @tc.expected: ReportContentChangeEvent never called.
     */
    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    SetScrollingNodes();
    contentChangeMgr->scrollingNodes_.emplace(node->GetId());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _))
        .Times(NEVER_ONCE);
    contentChangeMgr->OnScrollChangeEnd(node);
    ResetScrollingNodes();

    /**
     * @tc.steps: step6. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
}

/**
 * @tc.name: ContentChangeManagerTest006_1
 * @tc.desc: Test OnScrollChangeEnd with different scroll reasons
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest006_1, TestSize.Level1)
{
    /**
     * @tc.steps: step1. initialize content change manager and enable detect.
     * @tc.expected: contentChangeMgr is not nullptr and detect is enabled.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto node = FrameNode::CreateFrameNode("frameNode", NORMAL_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(node, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);

    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());

    /**
     * @tc.steps: step2. call OnScrollChangeEnd with default reason (scrollEnd).
     * @tc.expected: ReportContentChangeEvent called with scrollEnd reason.
     */
    contentChangeMgr->scrollingNodes_.emplace(node->GetId());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SCROLL,
        "{\"scrollReason\":\"scrollEnd\"}"))
        .Times(AtLeast(AT_LEAST_ONCE));
    contentChangeMgr->OnScrollChangeEnd(node);

    /**
     * @tc.steps: step3. call OnScrollChangeEnd with SCROLL_TO reason.
     * @tc.expected: ReportContentChangeEvent called with scrollTo reason.
     */
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    contentChangeMgr->scrollingNodes_.emplace(node->GetId());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SCROLL,
        "{\"scrollReason\":\"scrollTo\"}"))
        .Times(AtLeast(AT_LEAST_ONCE));
    contentChangeMgr->OnScrollChangeEnd(node, ContentChangeManager::SCROLL_TO);

    /**
     * @tc.steps: step4. call OnScrollChangeEnd with SCROLL_TO_INDEX reason.
     * @tc.expected: ReportContentChangeEvent called with scrollToIndex reason.
     */
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    contentChangeMgr->scrollingNodes_.emplace(node->GetId());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SCROLL,
        "{\"scrollReason\":\"scrollToIndex\"}"))
        .Times(AtLeast(AT_LEAST_ONCE));
    contentChangeMgr->OnScrollChangeEnd(node, ContentChangeManager::SCROLL_TO_INDEX);

    /**
     * @tc.steps: step5. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
}

/**
 * @tc.name: ContentChangeManagerTest006_2
 * @tc.desc: Test OnScrollChangeEnd with ignoreEventMask
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest006_2, TestSize.Level1)
{
    /**
     * @tc.steps: step1. initialize content change manager and enable detect.
     * @tc.expected: contentChangeMgr is not nullptr and detect is enabled.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto node = FrameNode::CreateFrameNode("frameNode", NORMAL_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(node, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);

    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());

    /**
     * @tc.steps: step2. set ignoreEventMask to ignore SCROLL_TO and SCROLL_TO_INDEX.
     * @tc.expected: ignoreEventMask is set correctly.
     */
    contentChangeMgr->ignoreEventMask_ = ContentChangeManager::SCROLL_TO |
        ContentChangeManager::SCROLL_TO_INDEX;

    /**
     * @tc.steps: step3. call OnScrollChangeEnd with ignored SCROLL_TO reason.
     * @tc.expected: ReportContentChangeEvent never called, scrollingNodes_ is cleaned up.
     */
    contentChangeMgr->scrollingNodes_.emplace(node->GetId());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _)).Times(NEVER_ONCE);
    contentChangeMgr->OnScrollChangeEnd(node, ContentChangeManager::SCROLL_TO);
    EXPECT_TRUE(contentChangeMgr->scrollingNodes_.empty());

    /**
     * @tc.steps: step4. call OnScrollChangeEnd with default reason (not ignored).
     * @tc.expected: ReportContentChangeEvent called with scrollEnd reason.
     */
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    contentChangeMgr->scrollingNodes_.emplace(node->GetId());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SCROLL,
        "{\"scrollReason\":\"scrollEnd\"}"))
        .Times(AtLeast(AT_LEAST_ONCE));
    contentChangeMgr->OnScrollChangeEnd(node);

    /**
     * @tc.steps: step5. reset ignoreEventMask and config.
     */
    contentChangeMgr->ignoreEventMask_ = ContentChangeManager::NONE;
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
}

/**
 * @tc.name: ContentChangeManagerTest007
 * @tc.desc: Test OnSwiperChangeEnd
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto node = FrameNode::CreateFrameNode("frameNode", NORMAL_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(node, nullptr);

    /**
     * @tc.steps: step2. call OnSwiperChangeEnd under abnormal conditions.
     * @tc.expected: changedSwiperNodes_ is empty.
     */
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->OnSwiperChangeEnd(nullptr, false);
    EXPECT_TRUE(contentChangeMgr->changedSwiperNodes_.empty());

    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->OnSwiperChangeEnd(nullptr, false);
    EXPECT_TRUE(contentChangeMgr->changedSwiperNodes_.empty());

    /**
     * @tc.steps: step3. call OnSwiperChangeEnd under normal conditions.
     * @tc.expected: changedSwiperNodes_ is not empty.
     */
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->OnSwiperChangeEnd(node, false);
    EXPECT_FALSE(contentChangeMgr->changedSwiperNodes_.empty());

    /**
     * @tc.steps: step4. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->changedSwiperNodes_.clear();
    EXPECT_TRUE(contentChangeMgr->changedSwiperNodes_.empty());
}

/**
 * @tc.name: ContentChangeManagerTest008
 * @tc.desc: Test OnDialogChangeEnd
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto node = FrameNode::CreateFrameNode("frameNode", NORMAL_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(node, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);
    auto showSimpleTree = GetDialogSimpleTree(node, true);
    auto notShowSimpleTree = GetDialogSimpleTree(node, false);

    /**
     * @tc.steps: step2. call OnDialogChangeEnd under abnormal conditions.
     * @tc.expected: ReportContentChangeEvent never called.
     */
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _))
        .Times(NEVER_ONCE);
    contentChangeMgr->OnDialogChangeEnd(nullptr, true);

    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _))
        .Times(NEVER_ONCE);
    contentChangeMgr->OnDialogChangeEnd(nullptr, true);

    /**
     * @tc.steps: step3. call OnDialogChangeEnd under normal conditions.
     * @tc.expected: ReportContentChangeEvent called once.
     */
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::DIALOG, showSimpleTree))
        .Times(AtLeast(AT_LEAST_ONCE));
    contentChangeMgr->OnDialogChangeEnd(node, true);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::DIALOG, notShowSimpleTree))
        .Times(AtLeast(AT_LEAST_ONCE));
    contentChangeMgr->OnDialogChangeEnd(node, false);

    /**
     * @tc.steps: step4. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
}

/**
 * @tc.name: ContentChangeManagerTest009
 * @tc.desc: Test OnTextChangeEnd
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    RectF rootRect(0.0, 0.0, 100.0, 100.0);
    RectF rect1(0.0, 0.0, 10.0, 10.0);
    RectF rect2(0.0, 0.0, 5.0, 20.0);
    RectF emptyRect(0.0, 0.0, 0.0, 0.0);

    /**
     * @tc.steps: step2. call OnTextChangeEnd under abnormal conditions.
     * @tc.expected: textAABB_ is empty.
     */
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->OnTextChangeEnd(emptyRect, rootRect);
    EXPECT_TRUE(contentChangeMgr->textAABB_.IsEmpty());

    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_FALSE(contentChangeMgr->textCollecting_);
    contentChangeMgr->OnTextChangeEnd(emptyRect, rootRect);
    EXPECT_TRUE(contentChangeMgr->textAABB_.IsEmpty());

    contentChangeMgr->textCollecting_ = true;
    contentChangeMgr->OnTextChangeEnd(emptyRect, rootRect);
    EXPECT_TRUE(contentChangeMgr->textAABB_.IsEmpty());

    SetScrollingNodes();
    EXPECT_TRUE(contentChangeMgr->IsScrolling());
    contentChangeMgr->OnTextChangeEnd(rect1, rootRect);
    EXPECT_TRUE(contentChangeMgr->textAABB_.IsEmpty());
    ResetScrollingNodes();

    contentChangeMgr->scrollingSwiperNodes_.emplace(TEST_SCROLLING_NODE_ID);
    EXPECT_TRUE(contentChangeMgr->IsSwiperScrolling());
    contentChangeMgr->OnTextChangeEnd(rect1, rootRect);
    EXPECT_TRUE(contentChangeMgr->textAABB_.IsEmpty());
    contentChangeMgr->scrollingSwiperNodes_.clear();

    /**
     * @tc.steps: step3. call OnTextChangeEnd under normal conditions.
     * @tc.expected: textAABB_ is not empty and combined correctly.
     */
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_TRUE(contentChangeMgr->textCollecting_);
    EXPECT_FALSE(contentChangeMgr->IsScrolling());
    contentChangeMgr->OnTextChangeEnd(rect1, rootRect);
    EXPECT_FALSE(contentChangeMgr->textAABB_.IsEmpty());
    EXPECT_FLOAT_EQ(contentChangeMgr->textAABB_.GetX(), 0.0);
    EXPECT_FLOAT_EQ(contentChangeMgr->textAABB_.GetY(), 0.0);
    EXPECT_FLOAT_EQ(contentChangeMgr->textAABB_.Width(), 10.0);
    EXPECT_FLOAT_EQ(contentChangeMgr->textAABB_.Height(), 10.0);

    contentChangeMgr->OnTextChangeEnd(rect2, rootRect);
    EXPECT_FALSE(contentChangeMgr->textAABB_.IsEmpty());
    EXPECT_FLOAT_EQ(contentChangeMgr->textAABB_.GetX(), 0.0);
    EXPECT_FLOAT_EQ(contentChangeMgr->textAABB_.GetY(), 0.0);
    EXPECT_FLOAT_EQ(contentChangeMgr->textAABB_.Width(), 10.0);
    EXPECT_FLOAT_EQ(contentChangeMgr->textAABB_.Height(), 20.0);

    /**
     * @tc.steps: step4. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->textCollecting_ = false;
    contentChangeMgr->textAABB_.Reset();
    EXPECT_TRUE(contentChangeMgr->textAABB_.IsEmpty());
}

/**
 * @tc.name: ContentChangeManagerTest010
 * @tc.desc: Test OnVsyncStart
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. test OnVsyncStart.
     * @tc.expected: StartTextAABBCollecting called accordingly.
     */
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->OnVsyncStart();
    EXPECT_FALSE(contentChangeMgr->textCollecting_);

    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_FALSE(contentChangeMgr->textCollecting_);
    EXPECT_FALSE(contentChangeMgr->IsScrolling());
    contentChangeMgr->OnVsyncStart();
    EXPECT_TRUE(contentChangeMgr->textCollecting_);

    /**
     * @tc.steps: step3. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->textCollecting_ = false;
}

/**
 * @tc.name: ContentChangeManagerTest011
 * @tc.desc: Test OnVsyncEnd
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    RectF rootRect(0.0, 0.0, 100.0, 100.0);
    RectF notIntersectRect(150.0, 150.0, 10.0, 10.0);

    /**
     * @tc.steps: step2. test OnVsyncEnd when StopTextAABBCollecting called.
     * @tc.expected: StopTextAABBCollecting called accordingly.
     */
    contentChangeMgr->textAABB_ = notIntersectRect;
    contentChangeMgr->textCollecting_ = true;
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->OnVsyncEnd(rootRect);
    EXPECT_FALSE(contentChangeMgr->textAABB_.IsEmpty());
    EXPECT_TRUE(contentChangeMgr->textCollecting_);

    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_FALSE(contentChangeMgr->textAABB_.IsEmpty());
    EXPECT_FALSE(contentChangeMgr->IsScrolling());
    contentChangeMgr->OnVsyncEnd(rootRect);
    EXPECT_TRUE(contentChangeMgr->textAABB_.IsEmpty());
    EXPECT_FALSE(contentChangeMgr->textCollecting_);

    /**
     * @tc.steps: step3. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
}

/**
 * @tc.name: ContentChangeManagerTest012
 * @tc.desc: Test OnVsyncEnd
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    RectF rootRect(0.0, 0.0, 100.0, 100.0);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);

    /**
     * @tc.steps: step2. test add one node.
     * @tc.expected: size is correct.
     */
    SetChangedSwiperNodes();
    auto node = FrameNode::CreateFrameNode("frameNode", INITIAL_NODE_SIZE, AceType::MakeRefPtr<Pattern>(), true);
    auto weak = AceType::WeakClaim(AceType::RawPtr(node));
    contentChangeMgr->changedSwiperNodes_.emplace(std::make_pair(weak, false));
    weak.Reset();
    auto size = static_cast<int32_t>(contentChangeMgr->changedSwiperNodes_.size());
    EXPECT_EQ(size, INITIAL_NODE_SIZE_PLUS_ONE);

    /**
     * @tc.steps: step3. test OnVsyncEnd when changedSwiperNodes_ is not empty.
     * @tc.expected: ReportContentChangeEvent will be called.
     */
    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _))
        .Times(AtLeast(AT_LEAST_ONCE));
    contentChangeMgr->OnVsyncEnd(rootRect);

    /**
     * @tc.steps: step4. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    ResetChangedSwiperNodes();
    size = static_cast<int32_t>(contentChangeMgr->changedSwiperNodes_.size());
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: ContentChangeManagerTest013
 * @tc.desc: Test ProcessSwiperNodes
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest013, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. test add one node.
     * @tc.expected: size is correct.
     */
    SetChangedSwiperNodes();
    auto node = FrameNode::CreateFrameNode("frameNode", INITIAL_NODE_SIZE, AceType::MakeRefPtr<Pattern>(), true);
    auto weak = AceType::WeakClaim(AceType::RawPtr(node));
    contentChangeMgr->changedSwiperNodes_.emplace(std::make_pair(weak, false));
    weak.Reset();
    auto size = static_cast<int32_t>(contentChangeMgr->changedSwiperNodes_.size());
    EXPECT_EQ(size, INITIAL_NODE_SIZE_PLUS_ONE);

    /**
     * @tc.steps: step3. test ProcessSwiperNodes when changedSwiperNodes_ is not empty.
     * @tc.expected: changedSwiperNodes_ is reset.
     */
    contentChangeMgr->ProcessSwiperNodes();
    size = static_cast<int32_t>(contentChangeMgr->changedSwiperNodes_.size());
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: ContentChangeManagerTest014
 * @tc.desc: Test ReportSwiperEvent
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest014, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);

    /**
     * @tc.steps: steps2. create nodes and pattern
     * @tc.expected: each node and pattern is not nullptr.
     */
    auto swiperPattern = AceType::MakeRefPtr<SwiperPattern>();
    ASSERT_NE(swiperPattern, nullptr);
    auto tabsNode = FrameNode::CreateFrameNode(V2::TABS_ETS_TAG, 0, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(tabsNode, nullptr);
    auto swiperNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 1, swiperPattern, true);
    ASSERT_NE(swiperNode, nullptr);
    auto itemNode = FrameNode::CreateFrameNode("frameNode", 2, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(itemNode, nullptr);
    SwiperItemInfo swiperItemInfo;
    swiperItemInfo.node = itemNode;
    swiperPattern->itemPosition_.emplace(itemNode->GetId(), swiperItemInfo);
    swiperNode->AddChild(itemNode);

    /**
     * @tc.steps: steps3. when not hasTabsAncestor, call ReportSwiperEvent
     * @tc.expected: ReportContentChangeEvent called on type swiper.
     */
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SWIPER, _))
        .Times(AtLeast(AT_LEAST_ONCE));
    contentChangeMgr->ReportSwiperEvent(swiperNode, false);

    /**
     * @tc.steps: steps4. when hasTabsAncestor, call ReportSwiperEvent
     * @tc.expected: ReportContentChangeEvent called on type tabs.
     */
    tabsNode->AddChild(swiperNode);
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::TABS, _))
        .Times(AtLeast(AT_LEAST_ONCE));
    contentChangeMgr->ReportSwiperEvent(swiperNode, true);
}

/**
 * @tc.name: ContentChangeManagerTest015
 * @tc.desc: Test StartTextAABBCollecting
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest015, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call StartTextAABBCollecting under abnormal conditions.
     * @tc.expected: textCollecting_ is not set true.
     */
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->StartTextAABBCollecting();
    EXPECT_FALSE(contentChangeMgr->textCollecting_);

    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->textCollecting_ = true;
    contentChangeMgr->StartTextAABBCollecting();
    contentChangeMgr->textCollecting_ = false;

    SetScrollingNodes();
    EXPECT_TRUE(contentChangeMgr->IsScrolling());
    contentChangeMgr->StartTextAABBCollecting();
    EXPECT_FALSE(contentChangeMgr->textCollecting_);
    ResetScrollingNodes();

    contentChangeMgr->scrollingSwiperNodes_.emplace(TEST_SCROLLING_NODE_ID);
    EXPECT_TRUE(contentChangeMgr->IsSwiperScrolling());
    contentChangeMgr->StartTextAABBCollecting();
    EXPECT_FALSE(contentChangeMgr->textCollecting_);
    contentChangeMgr->scrollingSwiperNodes_.clear();

    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_FALSE(contentChangeMgr->textCollecting_);
    EXPECT_FALSE(contentChangeMgr->IsScrolling());
    contentChangeMgr->lastComponentReportTime_ = GetSysTimestamp();
    EXPECT_TRUE(GetSysTimestamp() - contentChangeMgr->lastComponentReportTime_ <
        contentChangeMgr->componentReportInterval_);
    contentChangeMgr->StartTextAABBCollecting();
    EXPECT_FALSE(contentChangeMgr->textCollecting_);
    contentChangeMgr->lastComponentReportTime_ = 0;

    /**
     * @tc.steps: step3. call StartTextAABBCollecting under normal conditions.
     * @tc.expected: textCollecting_ is set true.
     */
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_FALSE(contentChangeMgr->textCollecting_);
    EXPECT_FALSE(contentChangeMgr->IsScrolling());
    EXPECT_FALSE(GetSysTimestamp() - contentChangeMgr->lastComponentReportTime_ <
        contentChangeMgr->componentReportInterval_);
    contentChangeMgr->StartTextAABBCollecting();
    EXPECT_TRUE(contentChangeMgr->textCollecting_);

    /**
     * @tc.steps: step4. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->textCollecting_ = false;
}

/**
 * @tc.name: ContentChangeManagerTest016
 * @tc.desc: Test StopTextAABBCollecting
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest016, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    RectF rootRect(0.0, 0.0, 100.0, 100.0);
    RectF notIntersectRect(150.0, 150.0, 10.0, 10.0);

    /**
     * @tc.steps: step2. call StopTextAABBCollecting under abnormal conditions.
     * @tc.expected: textAABB_ and textCollecting_ is not reset.
     */
    contentChangeMgr->textCollecting_ = true;
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->StopTextAABBCollecting(rootRect);
    EXPECT_TRUE(contentChangeMgr->textCollecting_);

    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_TRUE(contentChangeMgr->textAABB_.IsEmpty());
    contentChangeMgr->StopTextAABBCollecting(rootRect);
    EXPECT_TRUE(contentChangeMgr->textCollecting_);

    contentChangeMgr->textAABB_ = notIntersectRect;
    EXPECT_FALSE(contentChangeMgr->textAABB_.IsEmpty());
    SetScrollingNodes();
    EXPECT_TRUE(contentChangeMgr->IsScrolling());
    contentChangeMgr->StopTextAABBCollecting(rootRect);
    EXPECT_FALSE(contentChangeMgr->textAABB_.IsEmpty());
    EXPECT_TRUE(contentChangeMgr->textCollecting_);
    ResetScrollingNodes();

    contentChangeMgr->scrollingSwiperNodes_.emplace(TEST_SCROLLING_NODE_ID);
    EXPECT_TRUE(contentChangeMgr->IsSwiperScrolling());
    contentChangeMgr->StopTextAABBCollecting(rootRect);
    EXPECT_FALSE(contentChangeMgr->textAABB_.IsEmpty());
    EXPECT_TRUE(contentChangeMgr->textCollecting_);
    contentChangeMgr->scrollingSwiperNodes_.clear();

    /**
     * @tc.steps: step3. call StopTextAABBCollecting under normal conditions.
     * @tc.expected: textAABB_ and textCollecting_ is reset.
     */
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_FALSE(contentChangeMgr->textAABB_.IsEmpty());
    EXPECT_FALSE(contentChangeMgr->IsScrolling());
    contentChangeMgr->StopTextAABBCollecting(rootRect);
    EXPECT_TRUE(contentChangeMgr->textAABB_.IsEmpty());
    EXPECT_FALSE(contentChangeMgr->textCollecting_);

    /**
     * @tc.steps: step4. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
}

/**
 * @tc.name: ContentChangeManagerTest017
 * @tc.desc: Test StopTextAABBCollecting
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest017, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    RectF rootRect(0.0, 0.0, 100.0, 100.0);
    RectF intersectRect(0.0, 0.0, 10.0, 10.0);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);
    auto originRatio = contentChangeMgr->textContentRatio_;

    /**
     * @tc.steps: step2. rootRect should intersect with textAABB_.
     */
    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->textAABB_ = intersectRect;
    EXPECT_FALSE(contentChangeMgr->textAABB_.IsEmpty());
    EXPECT_FALSE(contentChangeMgr->IsScrolling());
    EXPECT_TRUE(rootRect.IsIntersectWith(contentChangeMgr->textAABB_));

    /**
     * @tc.steps: step3. call StopTextAABBCollecting when below the ratio.
     * @tc.expected: textAABB_ and textCollecting_ is reset.
     */
    contentChangeMgr->textContentRatio_ = 1.0f;
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _))
        .Times(NEVER_ONCE);
    contentChangeMgr->StopTextAABBCollecting(rootRect);
    EXPECT_EQ(contentChangeMgr->lastComponentReportTime_, 0);
    EXPECT_TRUE(contentChangeMgr->textAABB_.IsEmpty());
    EXPECT_FALSE(contentChangeMgr->textCollecting_);

    /**
     * @tc.steps: step4. call StopTextAABBCollecting when below the ratio.
     * @tc.expected: textAABB_ and textCollecting_ is reset.
     */
    contentChangeMgr->textContentRatio_ = 0.0f;
    contentChangeMgr->textAABB_ = intersectRect;
    contentChangeMgr->componentReportDelayTime_ = 0;
    EXPECT_FALSE(contentChangeMgr->textAABB_.IsEmpty());
    EXPECT_FALSE(contentChangeMgr->IsInTransitionDelayWindow());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::TEXT, ""))
        .Times(AtLeast(AT_LEAST_ONCE));
    contentChangeMgr->StopTextAABBCollecting(rootRect);
    EXPECT_NE(contentChangeMgr->lastComponentReportTime_, 0);
    EXPECT_TRUE(contentChangeMgr->textAABB_.IsEmpty());
    EXPECT_FALSE(contentChangeMgr->textCollecting_);

    /**
     * @tc.steps: step5. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->textContentRatio_ = originRatio;
}

/**
 * @tc.name: ContentChangeManagerTest018
 * @tc.desc: Test OnScrollChangeStart
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest018, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto node = FrameNode::CreateFrameNode("frameNode", NORMAL_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(node, nullptr);

    /**
     * @tc.steps: step2. call OnScrollChangeStart under abnormal conditions.
     * @tc.expected: scrollingNodes_ is empty.
     */
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->OnScrollChangeStart(nullptr);
    EXPECT_TRUE(contentChangeMgr->scrollingNodes_.empty());

    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->OnScrollChangeStart(nullptr);
    EXPECT_TRUE(contentChangeMgr->scrollingNodes_.empty());

    /**
     * @tc.steps: step3. call OnScrollChangeStart under normal conditions.
     * @tc.expected: scrollingNodes_ is not empty.
     */
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    EXPECT_FALSE(contentChangeMgr->IsScrolling());
    contentChangeMgr->OnScrollChangeStart(node);
    EXPECT_FALSE(contentChangeMgr->scrollingNodes_.empty());

    /**
     * @tc.steps: step4. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    ResetScrollingNodes();
    EXPECT_TRUE(contentChangeMgr->scrollingNodes_.empty());
}

/**
 * @tc.name: ContentChangeManagerTest019
 * @tc.desc: Test OnScrollRemoved
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest019, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. test OnScrollRemoved.
     * @tc.expected: scrollingNodes_ is empty.
     */
    SetScrollingNodes();
    EXPECT_FALSE(contentChangeMgr->scrollingNodes_.empty());

    contentChangeMgr->OnScrollRemoved(NORMAL_NODE_ID);
    contentChangeMgr->OnScrollRemoved(TEST_SCROLLING_NODE_ID);
    EXPECT_TRUE(contentChangeMgr->scrollingNodes_.empty());
}

/**
 * @tc.name: ContentChangeManagerDumpTest001
 * @tc.desc: Test Dump of ContentChangeManager.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerDumpTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call StartContentChangeReport.
     * @tc.expected: the contentContentChangeManager is in reporting state
     */
    ContentChangeConfig config;
    contentChangeMgr->StartContentChangeReport(config);
    ASSERT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());

    /**
     * @tc.steps: step3. call StopContentChangeReport.
     * @tc.expected: the contentContentChangeManager is not in reporting state
     */
    contentChangeMgr->StopContentChangeReport();
    ASSERT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());

    /**
     * @tc.steps: step4. call dump.
     * @tc.expected: the dump string contains REGISTER and UNREGISTER event.
     */
    std::string str = contentChangeMgr->DumpInfo();
    EXPECT_TRUE(str.find("UNREGISTERED") != std::string::npos);
    EXPECT_TRUE(str.find("REGISTER") != std::string::npos);
}

/**
 * @tc.name: ContentChangeManagerGetIgnoreEventMaskTest001
 * @tc.desc: Test GetIgnoreEventMask with empty string
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerGetIgnoreEventMaskTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call GetIgnoreEventMask with empty string.
     * @tc.expected: returns 0 (NONE).
     */
    uint32_t mask = contentChangeMgr->GetIgnoreEventMask("");
    EXPECT_EQ(mask, ContentChangeManager::NONE);
}

/**
 * @tc.name: ContentChangeManagerGetIgnoreEventMaskTest002
 * @tc.desc: Test GetIgnoreEventMask with invalid JSON string
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerGetIgnoreEventMaskTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call GetIgnoreEventMask with invalid JSON.
     * @tc.expected: returns 0 (NONE).
     */
    uint32_t mask = contentChangeMgr->GetIgnoreEventMask("{invalid json");
    EXPECT_EQ(mask, ContentChangeManager::NONE);

    mask = contentChangeMgr->GetIgnoreEventMask("not a json");
    EXPECT_EQ(mask, ContentChangeManager::NONE);

    mask = contentChangeMgr->GetIgnoreEventMask("{\"unclosed\": ");
    EXPECT_EQ(mask, ContentChangeManager::NONE);
}

/**
 * @tc.name: ContentChangeManagerGetIgnoreEventMaskTest003
 * @tc.desc: Test GetIgnoreEventMask with JSON that is not an object
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerGetIgnoreEventMaskTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call GetIgnoreEventMask with array JSON.
     * @tc.expected: returns 0 (NONE).
     */
    uint32_t mask = contentChangeMgr->GetIgnoreEventMask("[\"scrollTo\"]");
    EXPECT_EQ(mask, ContentChangeManager::NONE);

    /**
     * @tc.steps: step3. call GetIgnoreEventMask with string JSON.
     * @tc.expected: returns 0 (NONE).
     */
    mask = contentChangeMgr->GetIgnoreEventMask("\"scrollTo\"");
    EXPECT_EQ(mask, ContentChangeManager::NONE);

    /**
     * @tc.steps: step4. call GetIgnoreEventMask with number JSON.
     * @tc.expected: returns 0 (NONE).
     */
    mask = contentChangeMgr->GetIgnoreEventMask("123");
    EXPECT_EQ(mask, ContentChangeManager::NONE);
}

/**
 * @tc.name: ContentChangeManagerGetIgnoreEventMaskTest004
 * @tc.desc: Test GetIgnoreEventMask with JSON object without SCROLL field
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerGetIgnoreEventMaskTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call GetIgnoreEventMask with empty object.
     * @tc.expected: returns 0 (NONE).
     */
    uint32_t mask = contentChangeMgr->GetIgnoreEventMask("{}");
    EXPECT_EQ(mask, ContentChangeManager::NONE);

    /**
     * @tc.steps: step3. call GetIgnoreEventMask with other field.
     * @tc.expected: returns 0 (NONE).
     */
    mask = contentChangeMgr->GetIgnoreEventMask("{\"OTHER\": [\"scrollTo\"]}");
    EXPECT_EQ(mask, ContentChangeManager::NONE);
}

/**
 * @tc.name: ContentChangeManagerGetIgnoreEventMaskTest005
 * @tc.desc: Test GetIgnoreEventMask with SCROLL field that is not an array
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerGetIgnoreEventMaskTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call GetIgnoreEventMask with SCROLL as string.
     * @tc.expected: returns 0 (NONE).
     */
    uint32_t mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": \"scrollTo\"}");
    EXPECT_EQ(mask, ContentChangeManager::NONE);

    /**
     * @tc.steps: step3. call GetIgnoreEventMask with SCROLL as object.
     * @tc.expected: returns 0 (NONE).
     */
    mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": {\"event\": \"scrollTo\"}}");
    EXPECT_EQ(mask, ContentChangeManager::NONE);

    /**
     * @tc.steps: step4. call GetIgnoreEventMask with SCROLL as number.
     * @tc.expected: returns 0 (NONE).
     */
    mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": 123}");
    EXPECT_EQ(mask, ContentChangeManager::NONE);
}

/**
 * @tc.name: ContentChangeManagerGetIgnoreEventMaskTest006
 * @tc.desc: Test GetIgnoreEventMask with empty SCROLL array
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerGetIgnoreEventMaskTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call GetIgnoreEventMask with empty SCROLL array.
     * @tc.expected: returns 0 (NONE).
     */
    uint32_t mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": []}");
    EXPECT_EQ(mask, ContentChangeManager::NONE);
}

/**
 * @tc.name: ContentChangeManagerGetIgnoreEventMaskTest007
 * @tc.desc: Test GetIgnoreEventMask with valid single event in SCROLL array
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerGetIgnoreEventMaskTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call GetIgnoreEventMask with "scrollTo" event.
     * @tc.expected: returns SCROLL_TO mask.
     */
    uint32_t mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [\"scrollTo\"]}");
    EXPECT_EQ(mask, ContentChangeManager::SCROLL_TO);

    /**
     * @tc.steps: step3. call GetIgnoreEventMask with "scrollToIndex" event.
     * @tc.expected: returns SCROLL_TO_INDEX mask.
     */
    mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [\"scrollToIndex\"]}");
    EXPECT_EQ(mask, ContentChangeManager::SCROLL_TO_INDEX);
}

/**
 * @tc.name: ContentChangeManagerGetIgnoreEventMaskTest008
 * @tc.desc: Test GetIgnoreEventMask with multiple valid events in SCROLL array
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerGetIgnoreEventMaskTest008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call GetIgnoreEventMask with duplicate "scrollTo" events.
     * @tc.expected: returns SCROLL_TO mask (OR operation with same value).
     */
    uint32_t mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [\"scrollTo\", \"scrollTo\"]}");
    EXPECT_EQ(mask, ContentChangeManager::SCROLL_TO);

    /**
     * @tc.steps: step3. call GetIgnoreEventMask with duplicate "scrollToIndex" events.
     * @tc.expected: returns SCROLL_TO_INDEX mask (OR operation with same value).
     */
    mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [\"scrollToIndex\", \"scrollToIndex\"]}");
    EXPECT_EQ(mask, ContentChangeManager::SCROLL_TO_INDEX);
}

/**
 * @tc.name: ContentChangeManagerGetIgnoreEventMaskTest009
 * @tc.desc: Test GetIgnoreEventMask with invalid event strings in SCROLL array
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerGetIgnoreEventMaskTest009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call GetIgnoreEventMask with unknown event.
     * @tc.expected: returns 0 (NONE) - unknown events are ignored.
     */
    uint32_t mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [\"unknownEvent\"]}");
    EXPECT_EQ(mask, ContentChangeManager::NONE);

    /**
     * @tc.steps: step3. call GetIgnoreEventMask with invalid case.
     * @tc.expected: returns 0 (NONE) - case sensitive.
     */
    mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [\"ScrollTo\"]}");
    EXPECT_EQ(mask, ContentChangeManager::NONE);

    mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [\"SCROLLTO\"]}");
    EXPECT_EQ(mask, ContentChangeManager::NONE);

    /**
     * @tc.steps: step4. call GetIgnoreEventMask with invalid case for scrollToIndex.
     * @tc.expected: returns 0 (NONE) - case sensitive.
     */
    mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [\"ScrollToIndex\"]}");
    EXPECT_EQ(mask, ContentChangeManager::NONE);

    mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [\"SCROLLTOINDEX\"]}");
    EXPECT_EQ(mask, ContentChangeManager::NONE);
}

/**
 * @tc.name: ContentChangeManagerGetIgnoreEventMaskTest010
 * @tc.desc: Test GetIgnoreEventMask with mixed valid and invalid events
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerGetIgnoreEventMaskTest010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call GetIgnoreEventMask with valid and invalid events.
     * @tc.expected: returns SCROLL_TO mask - only valid events are processed.
     */
    uint32_t mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [\"unknown1\", \"scrollTo\", \"unknown2\"]}");
    EXPECT_EQ(mask, ContentChangeManager::SCROLL_TO);

    /**
     * @tc.steps: step3. call GetIgnoreEventMask with more complex mix.
     * @tc.expected: returns SCROLL_TO mask.
     */
    mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [\"\", \"scrollTo\", null, \"unknown\"]}");
    EXPECT_EQ(mask, ContentChangeManager::SCROLL_TO);

    /**
     * @tc.steps: step4. call GetIgnoreEventMask with scrollToIndex in mixed events.
     * @tc.expected: returns SCROLL_TO_INDEX mask.
     */
    mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [\"unknown1\", \"scrollToIndex\", \"unknown2\"]}");
    EXPECT_EQ(mask, ContentChangeManager::SCROLL_TO_INDEX);

    /**
     * @tc.steps: step5. call GetIgnoreEventMask with both scrollTo and scrollToIndex.
     * @tc.expected: returns SCROLL_TO | SCROLL_TO_INDEX mask.
     */
    mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [\"scrollTo\", \"scrollToIndex\"]}");
    EXPECT_EQ(mask, ContentChangeManager::SCROLL_TO | ContentChangeManager::SCROLL_TO_INDEX);
}

/**
 * @tc.name: ContentChangeManagerGetIgnoreEventMaskTest011
 * @tc.desc: Test GetIgnoreEventMask with non-string elements in SCROLL array
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerGetIgnoreEventMaskTest011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call GetIgnoreEventMask with number in array.
     * @tc.expected: returns 0 (NONE) - non-string elements are ignored.
     */
    uint32_t mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [123]}");
    EXPECT_EQ(mask, ContentChangeManager::NONE);

    /**
     * @tc.steps: step3. call GetIgnoreEventMask with null in array.
     * @tc.expected: returns 0 (NONE).
     */
    mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [null]}");
    EXPECT_EQ(mask, ContentChangeManager::NONE);

    /**
     * @tc.steps: step4. call GetIgnoreEventMask with boolean in array.
     * @tc.expected: returns 0 (NONE).
     */
    mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [true]}");
    EXPECT_EQ(mask, ContentChangeManager::NONE);

    /**
     * @tc.steps: step5. call GetIgnoreEventMask with object in array.
     * @tc.expected: returns 0 (NONE).
     */
    mask = contentChangeMgr->GetIgnoreEventMask("{\"SCROLL\": [{\"event\": \"scrollTo\"}]}");
    EXPECT_EQ(mask, ContentChangeManager::NONE);
}

/**
 * @tc.name: ContentChangeManagerGetIgnoreEventMaskTest012
 * @tc.desc: Test ConvertEventStringToEnum function
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerGetIgnoreEventMaskTest012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. test ConvertEventStringToEnum with "scrollTo" and "scrollToIndex".
     * @tc.expected: returns SCROLL_TO and SCROLL_TO_INDEX respectively.
     */
    uint32_t eventType = contentChangeMgr->ConvertEventStringToEnum("scrollTo");
    EXPECT_EQ(eventType, ContentChangeManager::SCROLL_TO);

    eventType = contentChangeMgr->ConvertEventStringToEnum("scrollToIndex");
    EXPECT_EQ(eventType, ContentChangeManager::SCROLL_TO_INDEX);

    /**
     * @tc.steps: step3. test ConvertEventStringToEnum with unknown string.
     * @tc.expected: returns NONE (0).
     */
    eventType = contentChangeMgr->ConvertEventStringToEnum("unknownEvent");
    EXPECT_EQ(eventType, ContentChangeManager::NONE);

    /**
     * @tc.steps: step4. test ConvertEventStringToEnum with empty string.
     * @tc.expected: returns NONE (0).
     */
    eventType = contentChangeMgr->ConvertEventStringToEnum("");
    EXPECT_EQ(eventType, ContentChangeManager::NONE);

    /**
     * @tc.steps: step5. test ConvertEventStringToEnum case sensitivity.
     * @tc.expected: returns NONE (0) - case sensitive.
     */
    eventType = contentChangeMgr->ConvertEventStringToEnum("ScrollTo");
    EXPECT_EQ(eventType, ContentChangeManager::NONE);

    eventType = contentChangeMgr->ConvertEventStringToEnum("SCROLLTO");
    EXPECT_EQ(eventType, ContentChangeManager::NONE);
}

/**
 * @tc.name: ContentChangeManagerGetIgnoreEventMaskTest013
 * @tc.desc: Test GetIgnoreEventMask enum values
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerGetIgnoreEventMaskTest013, TestSize.Level1)
{
    /**
     * @tc.steps: step1. verify enum values.
     * @tc.expected: enum values are correctly defined.
     */
    EXPECT_EQ(ContentChangeManager::NONE, 0);
    EXPECT_EQ(ContentChangeManager::SCROLL_TO, 1 << 0);
    EXPECT_EQ(ContentChangeManager::SCROLL_TO_INDEX, 1 << 1);

    /**
     * @tc.steps: step2. verify bit operations.
     * @tc.expected: bit operations work correctly.
     */
    uint32_t mask = 0;
    mask |= ContentChangeManager::SCROLL_TO;
    EXPECT_EQ(mask, 1);

    mask |= ContentChangeManager::SCROLL_TO;
    EXPECT_EQ(mask, 1); // OR with same value should not change

    mask |= ContentChangeManager::SCROLL_TO_INDEX;
    EXPECT_EQ(mask, 3); // 1 | 2 = 3
}

/**
 * @tc.name: ContentChangeManagerGetIgnoreEventMaskTest014
 * @tc.desc: Test GetIgnoreEventMask with complex valid JSON
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerGetIgnoreEventMaskTest014, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call GetIgnoreEventMask with complex JSON.
     * @tc.expected: correctly parses SCROLL array.
     */
    std::string jsonStr = R"({"otherField": "value", "SCROLL": ["scrollTo"], "anotherField": 123})";
    uint32_t mask = contentChangeMgr->GetIgnoreEventMask(jsonStr);
    EXPECT_EQ(mask, ContentChangeManager::SCROLL_TO);

    /**
     * @tc.steps: step3. call GetIgnoreEventMask with formatted JSON.
     * @tc.expected: correctly parses SCROLL array.
     */
    jsonStr = R"({
        "SCROLL": [
            "scrollTo"
        ]
    })";
    mask = contentChangeMgr->GetIgnoreEventMask(jsonStr);
    EXPECT_EQ(mask, ContentChangeManager::SCROLL_TO);
}

/**
 * @tc.name: ContentChangeManagerGetIgnoreEventMaskTest015
 * @tc.desc: Test GetIgnoreEventMask with ARKWEB ignore events.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerGetIgnoreEventMaskTest015, TestSize.Level1)
{
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    auto mask = contentChangeMgr->GetIgnoreEventMask(R"({"ARKWEB": ["all"]})");
    EXPECT_EQ(mask, ContentChangeManager::ARKWEB_ALL);

    mask = contentChangeMgr->GetIgnoreEventMask(R"({"SCROLL": ["scrollTo"], "ARKWEB": ["all"]})");
    EXPECT_EQ(mask, ContentChangeManager::SCROLL_TO | ContentChangeManager::ARKWEB_ALL);

    mask = contentChangeMgr->GetIgnoreEventMask(R"({"ARKWEB": ["ALL"]})");
    EXPECT_EQ(mask, ContentChangeManager::NONE);
}

/**
 * @tc.name: ContentChangeManagerTest020
 * @tc.desc: Test OnTransitionAdded
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest020, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call OnTransitionAdded when content change detect is disabled.
     * @tc.expected: transitioningNodes_ is empty.
     */
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->OnTransitionAdded(NORMAL_NODE_ID);
    EXPECT_TRUE(contentChangeMgr->transitioningNodes_.empty());

    /**
     * @tc.steps: step3. call OnTransitionAdded under normal conditions.
     * @tc.expected: transitioningNodes_ is not empty and contains the node id.
     */
    ContentChangeConfig config;
    contentChangeMgr->currentContentChangeConfig_ = config;
    EXPECT_TRUE(contentChangeMgr->IsContentChangeDetectEnable());
    contentChangeMgr->OnTransitionAdded(NORMAL_NODE_ID);
    EXPECT_FALSE(contentChangeMgr->transitioningNodes_.empty());
    EXPECT_EQ(static_cast<int32_t>(contentChangeMgr->transitioningNodes_.count(NORMAL_NODE_ID)), 1);
    auto size = static_cast<int32_t>(contentChangeMgr->transitioningNodes_.size());
    EXPECT_EQ(size, 1);

    /**
     * @tc.steps: step4. call OnTransitionAdded when transitioningNodes_ already has nodes.
     * @tc.expected: transitioningNodes_ size increases and contains both node ids.
     */
    SetTransitioningNodes();
    size = static_cast<int32_t>(contentChangeMgr->transitioningNodes_.size());
    EXPECT_EQ(size, 2);
    EXPECT_EQ(static_cast<int32_t>(contentChangeMgr->transitioningNodes_.count(NORMAL_NODE_ID)), 1);
    EXPECT_EQ(static_cast<int32_t>(contentChangeMgr->transitioningNodes_.count(TEST_TRANSITIONING_NODE_ID)), 1);

    /**
     * @tc.steps: step5. call OnTransitionAdded with existing node id (duplicate test).
     * @tc.expected: transitioningNodes_ size remains unchanged (set deduplication).
     */
    contentChangeMgr->OnTransitionAdded(NORMAL_NODE_ID);
    size = static_cast<int32_t>(contentChangeMgr->transitioningNodes_.size());
    EXPECT_EQ(size, 2);

    /**
     * @tc.steps: step6. reset.
     */
    contentChangeMgr->currentContentChangeConfig_.reset();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());
    ResetTransitioningNodes();
    EXPECT_TRUE(contentChangeMgr->transitioningNodes_.empty());
}

/**
 * @tc.name: ContentChangeManagerTest021
 * @tc.desc: Test OnTransitionRemoved
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest021, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    constexpr int32_t TEST_NODE_ID_NOT_EXIST = 999;

    /**
     * @tc.steps: step2. call OnTransitionRemoved when node does not exist.
     * @tc.expected: transitioningNodes_ remains empty.
     */
    EXPECT_TRUE(contentChangeMgr->transitioningNodes_.empty());
    contentChangeMgr->OnTransitionRemoved(TEST_NODE_ID_NOT_EXIST);
    EXPECT_TRUE(contentChangeMgr->transitioningNodes_.empty());

    /**
     * @tc.steps: step3. add some nodes and then remove a non-existent node.
     * @tc.expected: transitioningNodes_ size remains unchanged.
     */
    contentChangeMgr->transitioningNodes_.emplace(NORMAL_NODE_ID);
    contentChangeMgr->transitioningNodes_.emplace(TEST_TRANSITIONING_NODE_ID);
    auto size = static_cast<int32_t>(contentChangeMgr->transitioningNodes_.size());
    EXPECT_EQ(size, 2);
    contentChangeMgr->OnTransitionRemoved(TEST_NODE_ID_NOT_EXIST);
    size = static_cast<int32_t>(contentChangeMgr->transitioningNodes_.size());
    EXPECT_EQ(size, 2);

    /**
     * @tc.steps: step4. remove an existing node.
     * @tc.expected: transitioningNodes_ size decreases by 1.
     */
    contentChangeMgr->OnTransitionRemoved(NORMAL_NODE_ID);
    size = static_cast<int32_t>(contentChangeMgr->transitioningNodes_.size());
    EXPECT_EQ(size, 1);
    EXPECT_EQ(static_cast<int32_t>(contentChangeMgr->transitioningNodes_.count(NORMAL_NODE_ID)), 0);
    EXPECT_EQ(static_cast<int32_t>(contentChangeMgr->transitioningNodes_.count(TEST_TRANSITIONING_NODE_ID)), 1);

    /**
     * @tc.steps: step5. remove the last existing node.
     * @tc.expected: transitioningNodes_ is empty.
     */
    contentChangeMgr->OnTransitionRemoved(TEST_TRANSITIONING_NODE_ID);
    EXPECT_TRUE(contentChangeMgr->transitioningNodes_.empty());

    /**
     * @tc.steps: step6. reset.
     */
    ResetTransitioningNodes();
    EXPECT_TRUE(contentChangeMgr->transitioningNodes_.empty());
}

/**
 * @tc.name: ContentChangeManagerTest022
 * @tc.desc: Test OnSwiperScrollStart
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest022, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call OnSwiperScrollStart with nullptr.
     * @tc.expected: scrollingSwiperNodes_ remains empty.
     */
    contentChangeMgr->scrollingSwiperNodes_.clear();
    contentChangeMgr->OnSwiperScrollStart(nullptr);
    EXPECT_TRUE(contentChangeMgr->scrollingSwiperNodes_.empty());

    /**
     * @tc.steps: step3. call OnSwiperScrollStart with valid frameNode.
     * @tc.expected: scrollingSwiperNodes_ contains the node id.
     */
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 100, AceType::MakeRefPtr<SwiperPattern>());
    ASSERT_NE(frameNode, nullptr);
    contentChangeMgr->OnSwiperScrollStart(frameNode);
    EXPECT_EQ(static_cast<int32_t>(contentChangeMgr->scrollingSwiperNodes_.count(100)), 1);

    /**
     * @tc.steps: step4. call OnSwiperScrollStart with another node.
     * @tc.expected: scrollingSwiperNodes_ size increases.
     */
    auto frameNode2 = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 101, AceType::MakeRefPtr<SwiperPattern>());
    ASSERT_NE(frameNode2, nullptr);
    contentChangeMgr->OnSwiperScrollStart(frameNode2);
    EXPECT_EQ(static_cast<int32_t>(contentChangeMgr->scrollingSwiperNodes_.size()), 2);

    /**
     * @tc.steps: step5. reset.
     */
    contentChangeMgr->scrollingSwiperNodes_.clear();
    EXPECT_TRUE(contentChangeMgr->scrollingSwiperNodes_.empty());
}

/**
 * @tc.name: ContentChangeManagerTest023
 * @tc.desc: Test OnSwiperScrollEnd
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest023, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. call OnSwiperScrollEnd with nullptr.
     * @tc.expected: scrollingSwiperNodes_ remains unchanged.
     */
    contentChangeMgr->scrollingSwiperNodes_.emplace(100);
    contentChangeMgr->OnSwiperScrollEnd(nullptr);
    EXPECT_EQ(static_cast<int32_t>(contentChangeMgr->scrollingSwiperNodes_.size()), 1);

    /**
     * @tc.steps: step3. call OnSwiperScrollEnd with valid frameNode that exists in set.
     * @tc.expected: scrollingSwiperNodes_ size decreases.
     */
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 100, AceType::MakeRefPtr<SwiperPattern>());
    ASSERT_NE(frameNode, nullptr);
    contentChangeMgr->OnSwiperScrollEnd(frameNode);
    EXPECT_TRUE(contentChangeMgr->scrollingSwiperNodes_.empty());

    /**
     * @tc.steps: step4. call OnSwiperScrollEnd with node id not in set.
     * @tc.expected: scrollingSwiperNodes_ remains unchanged.
     */
    contentChangeMgr->scrollingSwiperNodes_.emplace(200);
    auto frameNode2 = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 999, AceType::MakeRefPtr<SwiperPattern>());
    ASSERT_NE(frameNode2, nullptr);
    contentChangeMgr->OnSwiperScrollEnd(frameNode2);
    EXPECT_EQ(static_cast<int32_t>(contentChangeMgr->scrollingSwiperNodes_.size()), 1);

    /**
     * @tc.steps: step5. reset.
     */
    contentChangeMgr->scrollingSwiperNodes_.clear();
    EXPECT_TRUE(contentChangeMgr->scrollingSwiperNodes_.empty());
}

/**
 * @tc.name: ContentChangeManagerTest024
 * @tc.desc: Test IsSwiperScrolling
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest024, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. test IsSwiperScrolling when scrollingSwiperNodes_ is empty.
     * @tc.expected: IsSwiperScrolling returns false.
     */
    contentChangeMgr->scrollingSwiperNodes_.clear();
    EXPECT_FALSE(contentChangeMgr->IsSwiperScrolling());

    /**
     * @tc.steps: step3. test IsSwiperScrolling when scrollingSwiperNodes_ has elements.
     * @tc.expected: IsSwiperScrolling returns true.
     */
    contentChangeMgr->scrollingSwiperNodes_.emplace(100);
    EXPECT_TRUE(contentChangeMgr->IsSwiperScrolling());

    /**
     * @tc.steps: step4. test IsSwiperScrolling after clearing.
     * @tc.expected: IsSwiperScrolling returns false.
     */
    contentChangeMgr->scrollingSwiperNodes_.clear();
    EXPECT_FALSE(contentChangeMgr->IsSwiperScrolling());

    /**
     * @tc.steps: step5. test with multiple elements.
     * @tc.expected: IsSwiperScrolling returns true.
     */
    contentChangeMgr->scrollingSwiperNodes_.emplace(100);
    contentChangeMgr->scrollingSwiperNodes_.emplace(200);
    contentChangeMgr->scrollingSwiperNodes_.emplace(300);
    EXPECT_TRUE(contentChangeMgr->IsSwiperScrolling());

    /**
     * @tc.steps: step6. reset.
     */
    contentChangeMgr->scrollingSwiperNodes_.clear();
    EXPECT_FALSE(contentChangeMgr->IsSwiperScrolling());
}

/**
 * @tc.name: ContentChangeManagerTest025
 * @tc.desc: Test OnSwiperScrollStart and OnSwiperScrollEnd combined
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest025, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. simulate swiper scroll start and end sequence.
     * @tc.expected: scrollingSwiperNodes_ state changes correctly.
     */
    auto frameNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 100, AceType::MakeRefPtr<SwiperPattern>());
    ASSERT_NE(frameNode, nullptr);

    EXPECT_FALSE(contentChangeMgr->IsSwiperScrolling());
    contentChangeMgr->OnSwiperScrollStart(frameNode);
    EXPECT_TRUE(contentChangeMgr->IsSwiperScrolling());
    contentChangeMgr->OnSwiperScrollEnd(frameNode);
    EXPECT_FALSE(contentChangeMgr->IsSwiperScrolling());

    /**
     * @tc.steps: step3. test duplicate start calls.
     * @tc.expected: set deduplication works correctly.
     */
    contentChangeMgr->OnSwiperScrollStart(frameNode);
    contentChangeMgr->OnSwiperScrollStart(frameNode);
    EXPECT_EQ(static_cast<int32_t>(contentChangeMgr->scrollingSwiperNodes_.size()), 1);
    EXPECT_TRUE(contentChangeMgr->IsSwiperScrolling());

    /**
     * @tc.steps: step4. reset.
     */
    contentChangeMgr->scrollingSwiperNodes_.clear();
    EXPECT_FALSE(contentChangeMgr->IsSwiperScrolling());
}

/**
 * @tc.name: ContentChangeManagerTest026
 * @tc.desc: Test StartContentChangeReport clears scrollingSwiperNodes_
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest026, TestSize.Level1)
{
    /**
     * @tc.steps: step1. test whether can get content change manager.
     * @tc.expected: contentChangeMgr is not nullptr.
     */
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    /**
     * @tc.steps: step2. add elements to scrollingSwiperNodes_.
     * @tc.expected: scrollingSwiperNodes_ contains elements.
     */
    contentChangeMgr->scrollingSwiperNodes_.emplace(100);
    contentChangeMgr->scrollingSwiperNodes_.emplace(200);
    EXPECT_TRUE(contentChangeMgr->IsSwiperScrolling());

    /**
     * @tc.steps: step3. call StartContentChangeReport.
     * @tc.expected: scrollingSwiperNodes_ is cleared.
     */
    ContentChangeConfig config;
    contentChangeMgr->StartContentChangeReport(config);
    EXPECT_TRUE(contentChangeMgr->scrollingSwiperNodes_.empty());
    EXPECT_FALSE(contentChangeMgr->IsSwiperScrolling());
}

/**
 * @tc.name: ContentChangeManagerTest027
 * @tc.desc: Test OnVsyncEnd flushes PageScene when only PageScene is registered
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest027, TestSize.Level1)
{
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);

    contentChangeMgr->currentContentChangeConfig_.reset();
    ResetScrollingNodes();
    ResetTransitioningNodes();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());

    EXPECT_CALL(*mockUiSessionManager, GetPageSceneRulesRegistered()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _)).Times(NEVER_ONCE);
    EXPECT_CALL(*mockUiSessionManager, NotifyPageSceneContentChanged()).Times(NEVER_ONCE);
    EXPECT_CALL(*mockUiSessionManager, FlushPageSceneNodeChanged()).Times(1);

    contentChangeMgr->OnVsyncEnd(RectF(0.0f, 0.0f, 100.0f, 100.0f));

    Mock::VerifyAndClearExpectations(mockUiSessionManager);
}

/**
 * @tc.name: ContentChangeManagerTest028
 * @tc.desc: Test stable ContentChange report points trigger PageScene without ContentChange report
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest028, TestSize.Level1)
{
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto node = FrameNode::CreateFrameNode("frameNode", NORMAL_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(node, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);

    contentChangeMgr->currentContentChangeConfig_.reset();
    ResetScrollingNodes();
    ResetTransitioningNodes();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());

    EXPECT_CALL(*mockUiSessionManager, GetPageSceneRulesRegistered()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _)).Times(NEVER_ONCE);
    EXPECT_CALL(*mockUiSessionManager, NotifyPageSceneContentChanged()).Times(3);
    EXPECT_CALL(*mockUiSessionManager, FlushPageSceneNodeChanged()).Times(3);

    contentChangeMgr->OnPageTransitionEnd(node);
    contentChangeMgr->OnScrollChangeStart(node);
    EXPECT_TRUE(contentChangeMgr->IsScrolling());
    contentChangeMgr->OnScrollChangeEnd(node);
    EXPECT_FALSE(contentChangeMgr->IsScrolling());
    contentChangeMgr->OnDialogChangeEnd(node, true);

    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    ResetScrollingNodes();
    ResetTransitioningNodes();
}

/**
 * @tc.name: ContentChangeManagerTest029
 * @tc.desc: Test swiper stable point defers PageScene flush to OnVsyncEnd
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest029, TestSize.Level1)
{
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto node = FrameNode::CreateFrameNode("frameNode", NORMAL_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(node, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);

    contentChangeMgr->currentContentChangeConfig_.reset();
    contentChangeMgr->changedSwiperNodes_.clear();
    ResetTransitioningNodes();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());

    EXPECT_CALL(*mockUiSessionManager, GetPageSceneRulesRegistered()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _)).Times(NEVER_ONCE);
    EXPECT_CALL(*mockUiSessionManager, NotifyPageSceneContentChanged()).Times(1);
    EXPECT_CALL(*mockUiSessionManager, FlushPageSceneNodeChanged()).Times(1);

    contentChangeMgr->OnSwiperChangeEnd(node, false);
    EXPECT_TRUE(contentChangeMgr->changedSwiperNodes_.empty());
    contentChangeMgr->OnVsyncEnd(RectF(0.0f, 0.0f, 100.0f, 100.0f));

    Mock::VerifyAndClearExpectations(mockUiSessionManager);
}

/**
 * @tc.name: ContentChangeManagerTest030
 * @tc.desc: Test PageScene flush waits until swiper scrolling becomes stable
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest030, TestSize.Level1)
{
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto node = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, NORMAL_NODE_ID, AceType::MakeRefPtr<SwiperPattern>());
    ASSERT_NE(node, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);

    contentChangeMgr->currentContentChangeConfig_.reset();
    contentChangeMgr->scrollingSwiperNodes_.clear();
    ResetScrollingNodes();
    ResetTransitioningNodes();
    EXPECT_FALSE(contentChangeMgr->IsContentChangeDetectEnable());

    EXPECT_CALL(*mockUiSessionManager, GetPageSceneRulesRegistered()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _)).Times(NEVER_ONCE);
    EXPECT_CALL(*mockUiSessionManager, NotifyPageSceneContentChanged()).Times(NEVER_ONCE);
    EXPECT_CALL(*mockUiSessionManager, FlushPageSceneNodeChanged()).Times(1);

    contentChangeMgr->OnSwiperScrollStart(node);
    EXPECT_TRUE(contentChangeMgr->IsSwiperScrolling());
    contentChangeMgr->OnVsyncEnd(RectF(0.0f, 0.0f, 100.0f, 100.0f));

    contentChangeMgr->OnSwiperScrollEnd(node);
    EXPECT_FALSE(contentChangeMgr->IsSwiperScrolling());
    contentChangeMgr->OnVsyncEnd(RectF(0.0f, 0.0f, 100.0f, 100.0f));

    Mock::VerifyAndClearExpectations(mockUiSessionManager);
}

/**
 * @tc.name: ContentChangeManagerStartEventTest001
 * @tc.desc: Test START reporting is disabled by default and deduplicated until normal END.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerStartEventTest001, TestSize.Level1)
{
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);
    auto node = FrameNode::CreateFrameNode("Page", 301, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(node, nullptr);

    ContentChangeConfig config;
    contentChangeMgr->StartContentChangeReport(config);
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::PAGE_START, _)).Times(NEVER_ONCE);
    contentChangeMgr->OnContentChangeStart(node, ChangeType::PAGE);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    config.reportStartEvent = true;
    contentChangeMgr->StartContentChangeReport(config);
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::PAGE_START, Truly([&](const auto& json) {
        auto value = JsonUtil::ParseJsonString(json);
        return value && value->IsValid() && value->GetString("$type") == node->GetTag() &&
               value->GetInt("$ID") == node->GetId() && value->GetInt64("startTimestamp", -1) > 0 &&
               value->GetString("endReason").empty();
    }))).Times(1);
    contentChangeMgr->OnContentChangeStart(node, ChangeType::PAGE);
    contentChangeMgr->OnContentChangeStart(node, ChangeType::PAGE);
    EXPECT_EQ(contentChangeMgr->activeContentChanges_.size(), 1);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::PAGE, _)).Times(1);
    contentChangeMgr->OnPageTransitionEnd(node);
    EXPECT_TRUE(contentChangeMgr->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    contentChangeMgr->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeManagerStartEventTest002
 * @tc.desc: Test PAGE, SCROLL, SWIPER and TABS START event types and payload identities.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerStartEventTest002, TestSize.Level1)
{
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);
    ContentChangeConfig config;
    config.reportStartEvent = true;
    contentChangeMgr->StartContentChangeReport(config);

    const std::vector<std::tuple<ChangeType, ChangeType, RefPtr<FrameNode>>> cases = {
        { ChangeType::PAGE, ChangeType::PAGE_START,
            FrameNode::CreateFrameNode("Page", 311, AceType::MakeRefPtr<Pattern>(), true) },
        { ChangeType::SCROLL, ChangeType::SCROLL_START,
            FrameNode::CreateFrameNode("List", 312, AceType::MakeRefPtr<Pattern>(), true) },
        { ChangeType::SWIPER, ChangeType::SWIPER_START,
            FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 313, AceType::MakeRefPtr<SwiperPattern>(), true) },
        { ChangeType::TABS, ChangeType::TABS_START,
            FrameNode::CreateFrameNode(V2::TABS_ETS_TAG, 314, AceType::MakeRefPtr<Pattern>(), true) },
    };
    for (const auto& [endType, startType, node] : cases) {
        ASSERT_NE(node, nullptr);
        auto expectedNode = node;
        EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(startType, Truly([&](const auto& json) {
            auto value = JsonUtil::ParseJsonString(json);
            return value && value->IsValid() && value->GetString("$type") == expectedNode->GetTag() &&
                   value->GetInt("$ID") == expectedNode->GetId() && value->GetInt64("startTimestamp", -1) > 0;
        }))).Times(1);
        contentChangeMgr->OnContentChangeStart(node, endType);
        Mock::VerifyAndClearExpectations(mockUiSessionManager);
    }
    EXPECT_EQ(contentChangeMgr->activeContentChanges_.size(), cases.size());

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _)).Times(NEVER_ONCE);
    contentChangeMgr->StopContentChangeReport();
    EXPECT_TRUE(contentChangeMgr->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
}

/**
 * @tc.name: ContentChangeManagerStartEventTest003
 * @tc.desc: Test terminal normal END, destroyed END and unregister lifecycle closure semantics.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerStartEventTest003, TestSize.Level1)
{
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);
    auto node = FrameNode::CreateFrameNode("List", 321, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(node, nullptr);
    ContentChangeConfig config;
    config.reportStartEvent = true;
    contentChangeMgr->StartContentChangeReport(config);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::PAGE_START, _)).Times(1);
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SCROLL_START, _)).Times(1);
    contentChangeMgr->OnContentChangeStart(node, ChangeType::PAGE);
    contentChangeMgr->OnContentChangeStart(node, ChangeType::SCROLL);
    contentChangeMgr->OnTransitionAdded(node->GetId());
    contentChangeMgr->OnScrollChangeStart(node);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::PAGE, Truly([&](const auto& json) {
        auto value = JsonUtil::ParseJsonString(json);
        return value && value->GetString("$type") == node->GetTag() && value->GetInt("$ID") == node->GetId() &&
               value->GetString("endReason").empty();
    }))).Times(1);
    contentChangeMgr->OnContentChangeInterrupted(node, ChangeType::PAGE);
    contentChangeMgr->OnContentChangeInterrupted(node, ChangeType::PAGE);
    EXPECT_TRUE(contentChangeMgr->transitioningNodes_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SCROLL, Truly([&](const auto& json) {
        auto value = JsonUtil::ParseJsonString(json);
        return value && value->GetString("$type") == node->GetTag() && value->GetInt("$ID") == node->GetId() &&
               value->GetString("endReason") == "destroyed";
    }))).Times(1);
    contentChangeMgr->OnContentChangeNodeDestroyed(node->GetId());
    contentChangeMgr->OnContentChangeNodeDestroyed(node->GetId());
    EXPECT_TRUE(contentChangeMgr->activeContentChanges_.empty());
    EXPECT_TRUE(contentChangeMgr->scrollingNodes_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SWIPER_START, _)).Times(1);
    contentChangeMgr->OnContentChangeStart(node, ChangeType::SWIPER);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    contentChangeMgr->scrollingNodes_.emplace(node->GetId());
    contentChangeMgr->transitioningNodes_.emplace(node->GetId());
    contentChangeMgr->scrollingSwiperNodes_.emplace(node->GetId());
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _)).Times(NEVER_ONCE);
    contentChangeMgr->StopContentChangeReport();
    EXPECT_TRUE(contentChangeMgr->activeContentChanges_.empty());
    EXPECT_TRUE(contentChangeMgr->scrollingNodes_.empty());
    EXPECT_TRUE(contentChangeMgr->transitioningNodes_.empty());
    EXPECT_TRUE(contentChangeMgr->scrollingSwiperNodes_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
}

/**
 * @tc.name: ContentChangeManagerStartEventTest004
 * @tc.desc: Test Swiper normal END stays pending until the existing Vsync report point.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerStartEventTest004, TestSize.Level1)
{
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);
    auto swiperPattern = AceType::MakeRefPtr<SwiperPattern>();
    auto swiperNode = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 331, swiperPattern, true);
    auto itemNode = FrameNode::CreateFrameNode("Item", 332, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(swiperNode, nullptr);
    ASSERT_NE(itemNode, nullptr);
    SwiperItemInfo swiperItemInfo;
    swiperItemInfo.node = itemNode;
    swiperPattern->itemPosition_.emplace(itemNode->GetId(), swiperItemInfo);
    swiperNode->AddChild(itemNode);

    ContentChangeConfig config;
    config.reportStartEvent = true;
    contentChangeMgr->StartContentChangeReport(config);
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SWIPER_START, _)).Times(1);
    contentChangeMgr->OnContentChangeStart(swiperNode, ChangeType::SWIPER);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SWIPER, _)).Times(NEVER_ONCE);
    contentChangeMgr->OnSwiperChangeEnd(swiperNode, false);
    EXPECT_TRUE(contentChangeMgr->activeContentChanges_.at(swiperNode->GetId())
        .at(ChangeType::SWIPER).normalEndPending);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SWIPER, _)).Times(1);
    contentChangeMgr->OnVsyncEnd(RectF(0.0f, 0.0f, 100.0f, 100.0f));
    EXPECT_TRUE(contentChangeMgr->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    contentChangeMgr->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeManagerStartEventTest005
 * @tc.desc: Test FrameNode detach reports destroyed END once and clears active state.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerStartEventTest005, TestSize.Level1)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeline, nullptr);
    auto contentChangeMgr = pipeline->GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);
    auto node = FrameNode::CreateFrameNode("List", 341, AceType::MakeRefPtr<Pattern>(), true);
    ASSERT_NE(node, nullptr);
    ContentChangeConfig config;
    config.reportStartEvent = true;
    contentChangeMgr->StartContentChangeReport(config);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SCROLL_START, _)).Times(1);
    contentChangeMgr->OnContentChangeStart(node, ChangeType::SCROLL);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SCROLL, Truly([&](const auto& json) {
        auto value = JsonUtil::ParseJsonString(json);
        return value && value->GetInt("$ID") == node->GetId() && value->GetString("endReason") == "destroyed";
    }))).Times(1);
    node->OnDetachFromMainTree(false, AceType::RawPtr(pipeline));
    EXPECT_TRUE(contentChangeMgr->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    contentChangeMgr->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeManagerStartEventTest006
 * @tc.desc: Test destroying a pending Swiper report node clears active and pending state.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerStartEventTest006, TestSize.Level1)
{
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(mockUiSessionManager, nullptr);
    auto swiperNode = FrameNode::CreateFrameNode(
        V2::SWIPER_ETS_TAG, 351, AceType::MakeRefPtr<SwiperPattern>(), true);
    ASSERT_NE(swiperNode, nullptr);
    ContentChangeConfig config;
    config.reportStartEvent = true;
    contentChangeMgr->StartContentChangeReport(config);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SWIPER_START, _)).Times(1);
    contentChangeMgr->OnContentChangeStart(swiperNode, ChangeType::SWIPER);
    contentChangeMgr->OnSwiperChangeEnd(swiperNode, false);
    ASSERT_FALSE(contentChangeMgr->pendingSwiperChanges_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager,
        ReportContentChangeEvent(ChangeType::SWIPER, HasSubstr("\"endReason\":\"destroyed\""))).Times(1);
    contentChangeMgr->OnContentChangeNodeDestroyed(swiperNode->GetId());
    EXPECT_TRUE(contentChangeMgr->activeContentChanges_.empty());
    EXPECT_TRUE(contentChangeMgr->pendingSwiperChanges_.empty());
    EXPECT_TRUE(contentChangeMgr->changedSwiperNodes_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SWIPER, _)).Times(NEVER_ONCE);
    contentChangeMgr->OnVsyncEnd(RectF(0.0f, 0.0f, 100.0f, 100.0f));
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    auto tabsNode = FrameNode::CreateFrameNode(V2::TABS_ETS_TAG, 352, AceType::MakeRefPtr<Pattern>(), true);
    auto tabsSwiperNode = FrameNode::CreateFrameNode(
        V2::SWIPER_ETS_TAG, 353, AceType::MakeRefPtr<SwiperPattern>(), true);
    ASSERT_NE(tabsNode, nullptr);
    ASSERT_NE(tabsSwiperNode, nullptr);
    tabsSwiperNode->MountToParent(tabsNode);
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::TABS_START, _)).Times(1);
    contentChangeMgr->OnContentChangeStart(tabsNode, ChangeType::TABS);
    contentChangeMgr->OnSwiperChangeEnd(tabsSwiperNode, true);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::TABS, Truly([&](const auto& json) {
        auto value = JsonUtil::ParseJsonString(json);
        return value && value->GetInt("$ID") == tabsNode->GetId() &&
               value->GetString("endReason") == "destroyed";
    }))).Times(1);
    contentChangeMgr->OnContentChangeNodeDestroyed(tabsSwiperNode->GetId());
    EXPECT_TRUE(contentChangeMgr->activeContentChanges_.empty());
    EXPECT_TRUE(contentChangeMgr->pendingSwiperChanges_.empty());
    EXPECT_TRUE(contentChangeMgr->changedSwiperNodes_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    contentChangeMgr->StopContentChangeReport();
}

/**
 * @tc.name: ConcurrentScrollInterruptionWaitsForLastNode
 * @tc.desc: A scroll abort clears its active state without ending another node's ongoing scroll.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ConcurrentScrollInterruptionWaitsForLastNode, TestSize.Level1)
{
    auto contentChangeMgr = GetContentChangeManager();
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    ASSERT_NE(mockUiSessionManager, nullptr);
    auto first = FrameNode::CreateFrameNode("List", 361, AceType::MakeRefPtr<Pattern>(), true);
    auto second = FrameNode::CreateFrameNode("Scroll", 362, AceType::MakeRefPtr<Pattern>(), true);
    ContentChangeConfig config;
    config.reportStartEvent = true;
    contentChangeMgr->StartContentChangeReport(config);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SCROLL_START, _)).Times(2);
    for (const auto& node : { first, second }) {
        contentChangeMgr->OnScrollChangeStart(node);
        contentChangeMgr->OnContentChangeStart(node, ChangeType::SCROLL);
    }
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _)).Times(NEVER_ONCE);
    contentChangeMgr->OnContentChangeInterrupted(first, ChangeType::SCROLL);
    contentChangeMgr->OnContentChangeInterrupted(first, ChangeType::SCROLL);
    EXPECT_EQ(contentChangeMgr->scrollingNodes_.size(), 1u);
    EXPECT_EQ(contentChangeMgr->scrollingNodes_.count(second->GetId()), 1u);
    EXPECT_EQ(contentChangeMgr->activeContentChanges_.count(first->GetId()), 0u);
    EXPECT_EQ(contentChangeMgr->activeContentChanges_.count(second->GetId()), 1u);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SCROLL, _)).Times(1);
    contentChangeMgr->OnScrollChangeEnd(second);
    EXPECT_TRUE(contentChangeMgr->activeContentChanges_.empty());
    EXPECT_TRUE(contentChangeMgr->scrollingNodes_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SCROLL_START, _)).Times(1);
    contentChangeMgr->OnScrollChangeStart(first);
    contentChangeMgr->OnContentChangeStart(first, ChangeType::SCROLL);
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::SCROLL, Truly([&](const auto& json) {
        auto value = JsonUtil::ParseJsonString(json);
        return value && value->GetInt("$ID") == first->GetId() && value->GetString("endReason").empty();
    }))).Times(1);
    contentChangeMgr->OnContentChangeInterrupted(first, ChangeType::SCROLL);
    contentChangeMgr->OnContentChangeInterrupted(first, ChangeType::SCROLL);
    EXPECT_TRUE(contentChangeMgr->activeContentChanges_.empty());
    EXPECT_TRUE(contentChangeMgr->scrollingNodes_.empty());
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    contentChangeMgr->StopContentChangeReport();
}

/**
 * @tc.name: DetachedSwiperWithoutPairClearsPending
 * @tc.desc: With IPC START enabled, detach clears pending END without fabricating a missing START pair.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, DetachedSwiperWithoutPairClearsPending, TestSize.Level1)
{
    auto pipeline = MockPipelineContext::GetCurrent();
    auto contentChangeMgr = GetContentChangeManager();
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(pipeline, nullptr);
    ASSERT_NE(contentChangeMgr, nullptr);
    ASSERT_NE(mockUiSessionManager, nullptr);
    auto swiper = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 371, AceType::MakeRefPtr<SwiperPattern>(), true);
    ContentChangeConfig config;
    config.reportStartEvent = true;
    contentChangeMgr->StartContentChangeReport(config);

    EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _)).Times(NEVER_ONCE);
    contentChangeMgr->OnSwiperChangeEnd(swiper, false);
    ASSERT_EQ(contentChangeMgr->changedSwiperNodes_.size(), 1u);
    ASSERT_EQ(contentChangeMgr->pendingSwiperChanges_.size(), 1U);
    swiper->OnDetachFromMainTree(false, AceType::RawPtr(pipeline));
    EXPECT_TRUE(contentChangeMgr->changedSwiperNodes_.empty());
    contentChangeMgr->OnVsyncEnd(RectF(0.0f, 0.0f, 100.0f, 100.0f));
    Mock::VerifyAndClearExpectations(mockUiSessionManager);
    contentChangeMgr->StopContentChangeReport();
}

/**
 * @tc.name: DestroyedTabsClearsDeferredEnd
 * @tc.desc: Destroying the logical Tabs node clears deferred and pending state with or without START.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, DestroyedTabsClearsDeferredEnd, TestSize.Level1)
{
    auto contentChangeMgr = GetContentChangeManager();
    auto mockUiSessionManager = GetMockUiSessionManager();
    ASSERT_NE(contentChangeMgr, nullptr);
    ASSERT_NE(mockUiSessionManager, nullptr);
    auto tabs = FrameNode::CreateFrameNode(V2::TABS_ETS_TAG, 381, AceType::MakeRefPtr<Pattern>(), true);
    auto swiper = FrameNode::CreateFrameNode(V2::SWIPER_ETS_TAG, 382, AceType::MakeRefPtr<SwiperPattern>(), true);
    swiper->MountToParent(tabs);

    for (bool reportStartEvent : { false, true }) {
        ContentChangeConfig config;
        config.reportStartEvent = reportStartEvent;
        contentChangeMgr->StartContentChangeReport(config);
        EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(ChangeType::TABS_START, _))
            .Times(reportStartEvent ? 1 : 0);
        contentChangeMgr->OnSwiperChangeStart(swiper, true);
        contentChangeMgr->OnSwiperChangeEnd(swiper, true);
        ASSERT_EQ(contentChangeMgr->changedSwiperNodes_.size(), 1u);
        Mock::VerifyAndClearExpectations(mockUiSessionManager);

        EXPECT_CALL(*mockUiSessionManager,
            ReportContentChangeEvent(ChangeType::TABS, HasSubstr("\"endReason\":\"destroyed\"")))
            .Times(reportStartEvent ? 1 : 0);
        contentChangeMgr->OnContentChangeNodeDestroyed(tabs->GetId());
        EXPECT_TRUE(contentChangeMgr->activeContentChanges_.empty());
        EXPECT_TRUE(contentChangeMgr->pendingSwiperChanges_.empty());
        EXPECT_TRUE(contentChangeMgr->changedSwiperNodes_.empty());
        Mock::VerifyAndClearExpectations(mockUiSessionManager);

        EXPECT_CALL(*mockUiSessionManager, ReportContentChangeEvent(_, _)).Times(NEVER_ONCE);
        contentChangeMgr->OnVsyncEnd(RectF(0.0f, 0.0f, 100.0f, 100.0f));
        Mock::VerifyAndClearExpectations(mockUiSessionManager);
        contentChangeMgr->StopContentChangeReport();
    }
}

/**
 * @tc.name: ContentChangeManagerTest031
 * @tc.desc: Test IsContentChanging
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerTest031, TestSize.Level1)
{
    auto contentChangeMgr = GetContentChangeManager();
    ASSERT_NE(contentChangeMgr, nullptr);

    ResetScrollingNodes();
    ResetTransitioningNodes();
    contentChangeMgr->scrollingSwiperNodes_.clear();
    EXPECT_FALSE(contentChangeMgr->IsContentChanging());

    SetScrollingNodes();
    EXPECT_TRUE(contentChangeMgr->IsContentChanging());
    ResetScrollingNodes();

    SetTransitioningNodes();
    EXPECT_TRUE(contentChangeMgr->IsContentChanging());
    ResetTransitioningNodes();

    contentChangeMgr->scrollingSwiperNodes_.emplace(TEST_SCROLLING_NODE_ID);
    EXPECT_TRUE(contentChangeMgr->IsContentChanging());
    contentChangeMgr->scrollingSwiperNodes_.clear();

    EXPECT_FALSE(contentChangeMgr->IsContentChanging());
}

namespace {
constexpr uint32_t PAGE_CATEGORY = OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_PAGE;
constexpr uint32_t SCROLL_CATEGORY = OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_SCROLL;
constexpr uint32_t OVERLAY_CATEGORY = OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_OVERLAY;
constexpr uint32_t IGNORE_SCROLL_BY = OH_ARKUI_NATIVEMODULE_CONTENTCHANGE_IGNORE_SCROLLBY;
constexpr uint32_t IGNORE_SCROLL_TO = OH_ARKUI_NATIVEMODULE_CONTENTCHANGE_IGNORE_SCROLLTO;
constexpr uint32_t IGNORE_TOAST = OH_ARKUI_NATIVEMODULE_CONTENTCHANGE_IGNORE_TOAST;
constexpr uint32_t PAGE_START = OH_ARKUI_NATIVEMODULE_EVENT_PAGE_CHANGE_START;
constexpr uint32_t PAGE_END = OH_ARKUI_NATIVEMODULE_EVENT_PAGE_CHANGE_END;
constexpr uint32_t SCROLL_START = OH_ARKUI_NATIVEMODULE_EVENT_SCROLL_START;
constexpr uint32_t SCROLL_END = OH_ARKUI_NATIVEMODULE_EVENT_SCROLL_END;
constexpr uint32_t OVERLAY_SHOW = OH_ARKUI_NATIVEMODULE_EVENT_OVERLAY_SHOW;
constexpr uint32_t OVERLAY_HIDE = OH_ARKUI_NATIVEMODULE_EVENT_OVERLAY_HIDE;
constexpr uint32_t SWIPER_START = OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_START;
constexpr uint32_t SWIPER_END = OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_END;
constexpr uint32_t SWIPER_CANCEL = OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_CANCEL;
constexpr uint32_t TABS_START = OH_ARKUI_NATIVEMODULE_EVENT_TABS_START;
constexpr uint32_t TABS_CANCEL = OH_ARKUI_NATIVEMODULE_EVENT_TABS_CANCEL;
constexpr uint32_t TABS_END = OH_ARKUI_NATIVEMODULE_EVENT_TABS_END;
constexpr uint32_t NO_IGNORED_EVENTS = 0;
constexpr uint32_t NO_SCROLL_EVENT_SOURCE = 0;
constexpr int32_t CONTENT_CHANGE_SCHEMA_VERSION = 1;

struct CapiTestState {
    RefPtr<ContentChangeManager> manager;
    uint64_t removeId = 0;
    int32_t calls = 0;
    int32_t releases = 0;
    std::string json;
    std::vector<uint32_t> types;
    std::function<void(uint32_t)> onEvent;
};

struct CapiTestCookie {
    CapiTestState* state;
};

void CapiContentChangeTestSink(uint32_t type, uint64_t timestamp, const char* json, uint32_t size, void* cookie)
{
    auto* state = static_cast<CapiTestCookie*>(cookie)->state;
    EXPECT_GT(timestamp, 0U);
    ++state->calls;
    state->types.emplace_back(type);
    state->json.assign(json, size);
    auto onEvent = state->onEvent;
    if (onEvent) {
        onEvent(type);
    }
    if (state->removeId) {
        EXPECT_TRUE(state->manager->UnregisterCapiContentChangeEvent(state->removeId));
        state->removeId = 0;
    }
}

void ReleaseCapiContentChangeTestCookie(void* cookie)
{
    auto* record = static_cast<CapiTestCookie*>(cookie);
    ++record->state->releases;
    delete record;
}
} // namespace

/**
 * @tc.name: CapiUnregisterSkipsUnenteredCallback
 * @tc.desc: Unregister during a callback skips unentered callbacks in this batch and subsequent events.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiUnregisterSkipsUnenteredCallback, TestSize.Level1)
{
    constexpr int32_t EXPECTED_CALLBACK_COUNT = 2;
    constexpr int32_t PAGE_NODE_ID = 410;
    auto manager = GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    CapiTestState first { manager };
    CapiTestState second { manager };
    auto firstId = manager->RegisterCapiContentChangeEvent(
        false, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &first },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto secondId = manager->RegisterCapiContentChangeEvent(
        false, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &second },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    ASSERT_NE(firstId, 0U);
    ASSERT_NE(secondId, 0U);
    first.removeId = secondId;
    auto page = FrameNode::CreateFrameNode("page", PAGE_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);

    manager->capiImpl_->DispatchCapiContentChangeEvent(PAGE_END, page);
    EXPECT_EQ(first.calls, 1);
    EXPECT_EQ(second.calls, 0);
    EXPECT_EQ(second.releases, 1);
    manager->capiImpl_->DispatchCapiContentChangeEvent(PAGE_END, page);
    EXPECT_EQ(first.calls, EXPECTED_CALLBACK_COUNT);
    EXPECT_EQ(second.calls, 0);
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(firstId));
    EXPECT_EQ(first.releases, 1);
}

/**
 * @tc.name: CapiStateSurvivesInnerStop
 * @tc.desc: Stopping the IPC channel preserves independent CAPI subscriptions and state.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiStateSurvivesInnerStop, TestSize.Level1)
{
    constexpr int32_t EXPECTED_CHILD_COUNT = 2;
    constexpr int32_t EXPECTED_CALLBACK_COUNT = 2;
    constexpr int32_t PAGE_NODE_ID = 411;
    constexpr int32_t FIRST_CHILD_NODE_ID = 417;
    constexpr int32_t SECOND_CHILD_NODE_ID = 418;
    auto manager = GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    ASSERT_NE(id, 0U);
    manager->StopContentChangeReport();
    auto page = FrameNode::CreateFrameNode("page", PAGE_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    auto firstChild = FrameNode::CreateFrameNode("Text", FIRST_CHILD_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    auto secondChild = FrameNode::CreateFrameNode("Button", SECOND_CHILD_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    page->AddChild(firstChild);
    page->AddChild(secondChild);
    manager->OnContentChangeStart(page, ChangeType::PAGE);
    manager->OnPageTransitionEnd(page);
    EXPECT_EQ(state.calls, EXPECTED_CALLBACK_COUNT);
    auto json = JsonUtil::ParseJsonString(state.json);
    ASSERT_TRUE(json);
    EXPECT_EQ(json->GetInt("schemaVersion"), CONTENT_CHANGE_SCHEMA_VERSION);
    EXPECT_EQ(json->GetInt("eventType"), PAGE_END);
    EXPECT_EQ(json->GetString("category"), "page");
    auto target = json->GetValue("targetNode");
    ASSERT_TRUE(target);
    EXPECT_EQ(target->GetString("id"), std::to_string(PAGE_NODE_ID));
    EXPECT_EQ(target->GetString("type"), "page");
    auto trigger = json->GetValue("triggerNode");
    ASSERT_TRUE(trigger);
    EXPECT_TRUE(trigger->IsNull());
    auto overlayType = json->GetValue("overlayType");
    ASSERT_TRUE(overlayType);
    EXPECT_TRUE(overlayType->IsNull());
    auto subtree = json->GetValue("subtree");
    ASSERT_TRUE(subtree);
    EXPECT_EQ(subtree->GetString("id"), std::to_string(PAGE_NODE_ID));
    auto children = subtree->GetValue("children");
    ASSERT_TRUE(children);
    ASSERT_EQ(children->GetArraySize(), EXPECTED_CHILD_COUNT);
    EXPECT_EQ(children->GetArrayItem(0)->GetString("id"), std::to_string(FIRST_CHILD_NODE_ID));
    EXPECT_EQ(children->GetArrayItem(1)->GetString("id"), std::to_string(SECOND_CHILD_NODE_ID));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
    EXPECT_EQ(state.releases, 1);
}

/**
 * @tc.name: CapiInterruptedEventsRequireActiveCycle
 * @tc.desc: Abnormal END notifications require an actual matching START.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiInterruptedEventsRequireActiveCycle, TestSize.Level1)
{
    constexpr int32_t PAGE_NODE_ID = 421;
    constexpr int32_t SCROLL_NODE_ID = 422;
    auto manager = GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    manager->InvalidateCapiContentChangeSubscriptions();
    manager->StopContentChangeReport();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY | SCROLL_CATEGORY, NO_IGNORED_EVENTS,
        new CapiTestCookie { &state }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    ASSERT_NE(id, 0U);
    auto page = FrameNode::CreateFrameNode("page", PAGE_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    auto scroll = FrameNode::CreateFrameNode("scroll", SCROLL_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);

    manager->OnContentChangeInterrupted(page, ChangeType::PAGE);
    manager->OnContentChangeInterrupted(scroll, ChangeType::SCROLL);
    EXPECT_TRUE(state.types.empty());

    manager->OnContentChangeStart(page, ChangeType::PAGE);
    manager->OnScrollChangeStart(scroll);
    manager->OnContentChangeStart(scroll, ChangeType::SCROLL);
    manager->OnContentChangeInterrupted(page, ChangeType::PAGE);
    manager->OnContentChangeInterrupted(scroll, ChangeType::SCROLL);
    manager->OnContentChangeInterrupted(page, ChangeType::PAGE);
    manager->OnContentChangeInterrupted(scroll, ChangeType::SCROLL);
    EXPECT_EQ(state.types, (std::vector<uint32_t> { PAGE_START, SCROLL_START, PAGE_END, SCROLL_END }));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
    EXPECT_EQ(state.releases, 1);
}

/**
 * @tc.name: CapiFiltersAndEmptyTarget
 * @tc.desc: Event masks and ignore masks filter notifications without inventing target nodes.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiFiltersAndEmptyTarget, TestSize.Level1)
{
    constexpr int32_t NODE_NODE_ID = 412;
    auto manager = GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        false, PAGE_CATEGORY | SCROLL_CATEGORY | OVERLAY_CATEGORY, IGNORE_SCROLL_BY | IGNORE_TOAST,
        new CapiTestCookie { &state }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    ASSERT_NE(id, 0U);
    auto node = FrameNode::CreateFrameNode("page", NODE_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    manager->capiImpl_->DispatchCapiContentChangeEvent(PAGE_START, node);
    manager->capiImpl_->DispatchCapiContentChangeEvent(SCROLL_START, node);
    manager->capiImpl_->DispatchCapiContentChangeEvent(SCROLL_END, node, IGNORE_SCROLL_BY);
    manager->capiImpl_->DispatchCapiContentChangeEvent(OVERLAY_SHOW, node, NO_SCROLL_EVENT_SOURCE, "toast");
    EXPECT_EQ(state.calls, 0);
    manager->capiImpl_->DispatchCapiContentChangeEvent(PAGE_END, nullptr);
    EXPECT_EQ(state.calls, 1);
    EXPECT_EQ(state.json, "");
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
    EXPECT_EQ(state.releases, 1);
}

/**
 * @tc.name: CapiScrollEndUsesLastNodeSource
 * @tc.desc: The last scrolling node determines the normal END source and ignore filtering.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiScrollEndUsesLastNodeSource, TestSize.Level1)
{
    constexpr int32_t FIRST_NODE_ID = 413;
    constexpr int32_t LAST_NODE_ID = 414;
    auto manager = GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(false, SCROLL_CATEGORY, IGNORE_SCROLL_TO,
        new CapiTestCookie { &state }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    ASSERT_NE(id, 0U);
    auto first = FrameNode::CreateFrameNode("scroll", FIRST_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    auto last = FrameNode::CreateFrameNode("scroll", LAST_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    manager->OnScrollChangeStart(first);
    manager->OnScrollChangeStart(last);
    manager->MarkCapiScrollToEventSource(first->GetId());
    manager->OnScrollChangeEnd(first, ContentChangeManager::SCROLL_TO);
    EXPECT_EQ(state.calls, 0);
    manager->MarkCapiScrollByEventSource(last->GetId());
    manager->OnScrollChangeEnd(last, 0);
    ASSERT_EQ(state.calls, 1);
    EXPECT_EQ(state.types.front(), SCROLL_END);
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiOverlayReportsEveryCallAcrossRegistration
 * @tc.desc: Every Overlay notification is delivered independently of prior SHOW and registration changes.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiOverlayReportsEveryCallAcrossRegistration, TestSize.Level1)
{
    constexpr int32_t POPUP_NODE_ID = 415;
    auto manager = GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    manager->InvalidateCapiContentChangeSubscriptions();
    auto popup = FrameNode::CreateFrameNode("popup", POPUP_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    manager->ReportCapiOverlayEvent(popup, true, "popup");
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(false, OVERLAY_CATEGORY, NO_IGNORED_EVENTS,
        new CapiTestCookie { &state }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    ASSERT_NE(id, 0U);
    manager->ReportCapiOverlayEvent(popup, false, "popup");
    manager->ReportCapiOverlayEvent(popup, true, "popup");
    manager->OnContentChangeNodeDestroyed(popup->GetId());
    manager->ReportCapiOverlayEvent(popup, false, "popup");
    manager->ReportCapiOverlayEvent(popup, false, "popup");
    EXPECT_EQ(state.types, (std::vector<uint32_t> { OVERLAY_HIDE, OVERLAY_SHOW, OVERLAY_HIDE, OVERLAY_HIDE }));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiSwiperEndAndCancelKeepDistinctEdges
 * @tc.desc: Normal completion and cancellation keep distinct Swiper event semantics.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiSwiperEndAndCancelKeepDistinctEdges, TestSize.Level1)
{
    constexpr int32_t SWIPER_NODE_ID = 416;
    auto manager = GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY, NO_IGNORED_EVENTS,
        new CapiTestCookie { &state }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    ASSERT_NE(id, 0U);
    auto swiper = FrameNode::CreateFrameNode("Swiper", SWIPER_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    manager->OnSwiperChangeStart(swiper, false);
    manager->OnSwiperChangeEnd(swiper, false);
    manager->capiImpl_->FlushCapiSwiperEndEvents();
    manager->OnSwiperChangeStart(swiper, false);
    manager->OnSwiperChangeCancel(swiper, false);
    manager->capiImpl_->FlushCapiSwiperEndEvents();
    EXPECT_EQ(state.types, (std::vector<uint32_t> { SWIPER_START, SWIPER_END, SWIPER_START, SWIPER_CANCEL }));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiDestroyedActiveNodesReportEndOnce
 * @tc.desc: Destroyed active nodes complete cached START pairs only once.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiDestroyedActiveNodesReportEndOnce, TestSize.Level1)
{
    constexpr int32_t PAGE_NODE_ID = 419;
    constexpr int32_t SCROLL_NODE_ID = 420;
    auto manager = GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY | SCROLL_CATEGORY, NO_IGNORED_EVENTS,
        new CapiTestCookie { &state }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    ASSERT_NE(id, 0U);
    auto page = FrameNode::CreateFrameNode("page", PAGE_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    auto scroll = FrameNode::CreateFrameNode("scroll", SCROLL_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    manager->OnContentChangeStart(page, ChangeType::PAGE);
    manager->OnScrollChangeStart(scroll);
    manager->OnContentChangeStart(scroll, ChangeType::SCROLL);
    manager->OnContentChangeNodeDestroyed(page->GetId());
    manager->OnContentChangeNodeDestroyed(scroll->GetId());
    manager->OnContentChangeNodeDestroyed(page->GetId());
    manager->OnContentChangeNodeDestroyed(scroll->GetId());
    EXPECT_EQ(state.types, (std::vector<uint32_t> { PAGE_START, SCROLL_START, PAGE_END, SCROLL_END }));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiDetachWithoutStartPreservesNormalPending
 * @tc.desc: Detach preserves a live node normal pending END for listeners without START.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiDetachWithoutStartPreservesNormalPending, TestSize.Level1)
{
    constexpr int32_t SCROLL_NODE_ID = 423;
    constexpr int32_t SWIPER_NODE_ID = 424;
    auto manager = GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(false, PAGE_CATEGORY | SCROLL_CATEGORY, NO_IGNORED_EVENTS,
        new CapiTestCookie { &state }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    ASSERT_NE(id, 0U);
    auto scroll = FrameNode::CreateFrameNode("scroll", SCROLL_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    auto swiper = FrameNode::CreateFrameNode("Swiper", SWIPER_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    manager->OnScrollChangeStart(scroll);
    manager->OnSwiperChangeEnd(swiper, false);
    manager->OnContentChangeNodeDestroyed(scroll->GetId());
    manager->OnContentChangeNodeDestroyed(swiper->GetId());
    manager->capiImpl_->FlushCapiSwiperEndEvents();
    EXPECT_EQ(state.types, (std::vector<uint32_t> { SWIPER_END }));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiSwiperFlushCanRevokeLaterEvent
 * @tc.desc: A callback can cancel a later pending Swiper END in the same frame.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiSwiperFlushCanRevokeLaterEvent, TestSize.Level1)
{
    constexpr int32_t FIRST_NODE_ID = 430;
    constexpr int32_t SECOND_NODE_ID = 431;
    auto manager = GetContentChangeManager();
    ASSERT_NE(manager, nullptr);
    manager->StopContentChangeReport();
    for (bool destroy : { false, true }) {
        manager->InvalidateCapiContentChangeSubscriptions();
        CapiTestState state { manager };
        auto id = manager->RegisterCapiContentChangeEvent(
            true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
            CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
        auto first = FrameNode::CreateFrameNode("Swiper", FIRST_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
        auto second = FrameNode::CreateFrameNode("Swiper", SECOND_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
        manager->OnSwiperChangeStart(first, false);
        manager->OnSwiperChangeStart(second, false);
        manager->OnSwiperChangeEnd(first, false);
        manager->OnSwiperChangeEnd(second, false);
        state.onEvent = [&](uint32_t type) {
            if (type != SWIPER_END) {
                return;
            }
            state.onEvent = nullptr;
            if (destroy) {
                manager->OnContentChangeNodeDestroyed(second->GetId());
            } else {
                manager->OnSwiperChangeCancel(second, false);
            }
        };
        manager->capiImpl_->FlushCapiSwiperEndEvents();
        EXPECT_EQ(state.types,
            (std::vector<uint32_t> { SWIPER_START, SWIPER_START, SWIPER_END, destroy ? SWIPER_END : SWIPER_CANCEL }));
        EXPECT_TRUE(manager->capiImpl_->capiPendingSwipers_.empty());
        EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
    }
}

/**
 * @tc.name: CapiSwiperFlushKeepsNewTransactionForNextFrame
 * @tc.desc: Reentrant completion keeps a new pending transaction for the next frame.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiSwiperFlushKeepsNewTransactionForNextFrame, TestSize.Level1)
{
    constexpr int32_t FIRST_NODE_ID = 432;
    constexpr int32_t SECOND_NODE_ID = 433;
    auto manager = GetContentChangeManager();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto first = FrameNode::CreateFrameNode("Swiper", FIRST_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    auto second = FrameNode::CreateFrameNode("Swiper", SECOND_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    manager->OnSwiperChangeStart(first, false);
    manager->OnSwiperChangeStart(second, false);
    manager->OnSwiperChangeEnd(first, false);
    manager->OnSwiperChangeEnd(second, false);
    state.onEvent = [&](uint32_t type) {
        if (type != SWIPER_END) {
            return;
        }
        state.onEvent = nullptr;
        manager->OnSwiperChangeCancel(second, false);
        manager->OnSwiperChangeStart(second, false);
        manager->OnSwiperChangeEnd(second, false);
    };
    manager->capiImpl_->FlushCapiSwiperEndEvents();
    EXPECT_EQ(state.types,
        (std::vector<uint32_t> { SWIPER_START, SWIPER_START, SWIPER_END, SWIPER_CANCEL, SWIPER_START }));
    EXPECT_EQ(manager->capiImpl_->capiPendingSwipers_.size(), 1U);
    manager->capiImpl_->FlushCapiSwiperEndEvents();
    EXPECT_EQ(state.types,
        (std::vector<uint32_t> { SWIPER_START, SWIPER_START, SWIPER_END, SWIPER_CANCEL, SWIPER_START, SWIPER_END }));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiSwiperFlushInvalidationDropsOldBatch
 * @tc.desc: Context invalidation discards remaining transactions from an old frame snapshot.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiSwiperFlushInvalidationDropsOldBatch, TestSize.Level1)
{
    constexpr int32_t FIRST_NODE_ID = 434;
    constexpr int32_t SECOND_NODE_ID = 435;
    auto manager = GetContentChangeManager();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state }, CapiContentChangeTestSink,
        ReleaseCapiContentChangeTestCookie);
    auto first = FrameNode::CreateFrameNode("Swiper", FIRST_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    auto second = FrameNode::CreateFrameNode("Swiper", SECOND_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    manager->OnSwiperChangeStart(first, false);
    manager->OnSwiperChangeStart(second, false);
    manager->OnSwiperChangeEnd(first, false);
    manager->OnSwiperChangeEnd(second, false);
    uint64_t replacement = 0;
    state.onEvent = [&](uint32_t type) {
        if (type != SWIPER_END) {
            return;
        }
        state.onEvent = nullptr;
        manager->InvalidateCapiContentChangeSubscriptions();
        replacement = manager->RegisterCapiContentChangeEvent(
            true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
            CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    };
    manager->capiImpl_->FlushCapiSwiperEndEvents();
    EXPECT_EQ(state.types, (std::vector<uint32_t> { SWIPER_START, SWIPER_START, SWIPER_END }));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(replacement));
}


/**
 * @tc.name: CapiExplicitScrollToSourceOverridesPreviousScrollBy
 * @tc.desc: Explicit ScrollTo END uses its own source instead of an earlier ScrollBy marker.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiExplicitScrollToSourceOverridesPreviousScrollBy, TestSize.Level1)
{
    constexpr int32_t NODE_NODE_ID = 443;
    auto manager = GetContentChangeManager();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        false, SCROLL_CATEGORY, IGNORE_SCROLL_TO, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto node = FrameNode::CreateFrameNode("Scroll", NODE_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    manager->MarkCapiScrollByEventSource(node->GetId());
    manager->OnScrollChangeEnd(node, ContentChangeManager::SCROLL_TO);
    EXPECT_TRUE(state.types.empty());
    EXPECT_EQ(manager->capiImpl_->capiScrollSources_.count(node->GetId()), 0U);
    manager->OnScrollChangeEnd(node);
    EXPECT_EQ(state.types, (std::vector<uint32_t> { SCROLL_END }));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}


/**
 * @tc.name: CapiScrollSourceSurvivesSubscriptionGap
 * @tc.desc: Operation provenance survives late registration and is cleared on completion without listeners.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiScrollSourceSurvivesSubscriptionGap, TestSize.Level1)
{
    constexpr int32_t NODE_NODE_ID = 444;
    auto manager = GetContentChangeManager();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto node = FrameNode::CreateFrameNode("Scroll", NODE_NODE_ID, AceType::MakeRefPtr<Pattern>(), true);
    // An operation can start before the first subscription.
    manager->MarkCapiScrollToEventSource(node->GetId());
    auto id = manager->RegisterCapiContentChangeEvent(
        false, SCROLL_CATEGORY, IGNORE_SCROLL_TO, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    manager->OnScrollChangeEnd(node);
    EXPECT_TRUE(state.types.empty());
    manager->MarkCapiScrollToEventSource(node->GetId());
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
    id = manager->RegisterCapiContentChangeEvent(
        false, SCROLL_CATEGORY, IGNORE_SCROLL_TO, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    manager->OnScrollChangeEnd(node);
    EXPECT_TRUE(state.types.empty());
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
    // With no subscribers, completion and detach must still release provenance.
    manager->MarkCapiScrollByEventSource(node->GetId());
    manager->OnScrollChangeEnd(node);
    EXPECT_TRUE(manager->capiImpl_->capiScrollSources_.empty());
    manager->MarkCapiScrollToEventSource(node->GetId());
    manager->OnContentChangeNodeDestroyed(node->GetId());
    EXPECT_TRUE(manager->capiImpl_->capiScrollSources_.empty());
}

/**
 * @tc.name: DetachedSwiperWithoutStartClearsPending
 * @tc.desc: IPC detach clears pending END whether the reporting node remains alive or is released.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, DetachedSwiperWithoutStartClearsPending, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    auto ipc = GetMockUiSessionManager();
    ContentChangeConfig config;
    config.reportStartEvent = false;
    manager->StartContentChangeReport(config);
    auto released = FrameNode::CreateFrameNode(
        V2::SWIPER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SwiperPattern>());
    auto retained = FrameNode::CreateFrameNode(
        V2::SWIPER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SwiperPattern>());
    manager->OnSwiperChangeEnd(released, false);
    manager->OnSwiperChangeEnd(retained, false);
    manager->OnContentChangeNodeDestroyed(released->GetId());
    manager->OnContentChangeNodeDestroyed(retained->GetId());
    WeakPtr<FrameNode> weak = released;
    released.Reset();
    ASSERT_TRUE(weak.Invalid());
    EXPECT_CALL(*ipc, ReportContentChangeEvent(_, _)).Times(NEVER_ONCE);
    manager->OnVsyncEnd(RectF());
    EXPECT_TRUE(manager->pendingSwiperChanges_.empty());
    EXPECT_TRUE(manager->changedSwiperNodes_.empty());
    Mock::VerifyAndClearExpectations(ipc);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: CapiCancelTerminatesAllSubscriptions
 * @tc.desc: Cancellation revokes the same pending END for both START and END-only listeners.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiCancelTerminatesAllSubscriptions, TestSize.Level1)
{
    constexpr int32_t SWIPER_NODE_ID = 502;
    auto manager = GetContentChangeManager();
    auto ipc = GetMockUiSessionManager();
    for (bool start : { false, true }) {
        manager->InvalidateCapiContentChangeSubscriptions();
        ContentChangeConfig config;
        config.reportStartEvent = start;
        manager->StartContentChangeReport(config);
        CapiTestState a { manager };
        CapiTestState b { manager };
        auto aid = manager->RegisterCapiContentChangeEvent(
            start, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &a },
            CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
        auto bid = manager->RegisterCapiContentChangeEvent(
            false, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &b },
            CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
        auto swiper = FrameNode::CreateFrameNode(
            V2::SWIPER_ETS_TAG, SWIPER_NODE_ID, AceType::MakeRefPtr<SwiperPattern>());
        EXPECT_CALL(*ipc, ReportContentChangeEvent(ChangeType::SWIPER_START, _)).Times(start ? 1 : 0);
        EXPECT_CALL(*ipc, ReportContentChangeEvent(ChangeType::SWIPER_START_CANCEL, _)).Times(start ? 1 : 0);
        EXPECT_CALL(*ipc, ReportContentChangeEvent(ChangeType::SWIPER, _)).Times(0);
        manager->OnSwiperChangeStart(swiper, false);
        manager->OnSwiperChangeEnd(swiper, false);
        manager->OnSwiperChangeCancel(swiper, false);
        manager->OnContentChangeNodeDestroyed(swiper->GetId());
        manager->OnVsyncEnd(RectF());
        EXPECT_EQ(a.types, start ? (std::vector<uint32_t> { SWIPER_START, SWIPER_CANCEL }) : std::vector<uint32_t>());
        EXPECT_TRUE(b.types.empty());
        Mock::VerifyAndClearExpectations(ipc);
        EXPECT_CALL(*ipc, ReportContentChangeEvent(ChangeType::SWIPER_START, _)).Times(start ? 1 : 0);
        EXPECT_CALL(*ipc, ReportContentChangeEvent(ChangeType::SWIPER, _)).Times(1);
        manager->OnSwiperChangeStart(swiper, false);
        manager->OnSwiperChangeEnd(swiper, false);
        manager->OnVsyncEnd(RectF());
        EXPECT_EQ(b.types, (std::vector<uint32_t> { SWIPER_END }));
        Mock::VerifyAndClearExpectations(ipc);
        EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(aid));
        EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(bid));
        manager->StopContentChangeReport();
    }
}

/**
 * @tc.name: CapiScrollDetachKeepsSubscriptionPairing
 * @tc.desc: Detach supplements require START; a later normal END also reaches late subscribers.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiScrollDetachKeepsSubscriptionPairing, TestSize.Level1)
{
    constexpr int32_t FIRST_NODE_ID = 503;
    constexpr int32_t SECOND_NODE_ID = 504;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState a { manager };
    CapiTestState b { manager };
    CapiTestState late { manager };
    auto aid = manager->RegisterCapiContentChangeEvent(
        true, SCROLL_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &a },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto bid = manager->RegisterCapiContentChangeEvent(
        false, SCROLL_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &b },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto first = FrameNode::CreateFrameNode("Scroll", FIRST_NODE_ID, AceType::MakeRefPtr<Pattern>());
    auto second = FrameNode::CreateFrameNode("Scroll", SECOND_NODE_ID, AceType::MakeRefPtr<Pattern>());
    for (const auto& node : { first, second }) {
        manager->OnScrollChangeStart(node);
        manager->OnContentChangeStart(node, ChangeType::SCROLL);
    }
    auto lateId = manager->RegisterCapiContentChangeEvent(
        true, SCROLL_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &late },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    manager->OnContentChangeNodeDestroyed(first->GetId());
    manager->OnContentChangeNodeDestroyed(first->GetId());
    EXPECT_EQ(a.types, (std::vector<uint32_t> { SCROLL_START, SCROLL_START, SCROLL_END }));
    EXPECT_TRUE(b.types.empty());
    EXPECT_TRUE(late.types.empty());
    manager->OnScrollChangeEnd(second);
    EXPECT_EQ(b.types, (std::vector<uint32_t> { SCROLL_END }));
    EXPECT_EQ(late.types, (std::vector<uint32_t> { SCROLL_END }));
    EXPECT_TRUE(manager->capiImpl_->capiScrollingNodes_.empty());
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(aid));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(bid));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(lateId));
}

/**
 * @tc.name: CapiTabsDetachPreservesNormalPendingForFalse
 * @tc.desc: Tabs detach preserves normal pending END for END-only listeners without double completion.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiTabsDetachPreservesNormalPendingForFalse, TestSize.Level1)
{
    constexpr size_t PAIRED_EVENT_COUNT = 2U;
    constexpr int32_t TABS_NODE_ID = 505;
    constexpr int32_t SWIPER_NODE_ID = 506;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState a { manager };
    CapiTestState b { manager };
    auto aid = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &a },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto bid = manager->RegisterCapiContentChangeEvent(
        false, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &b },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto tabs = FrameNode::CreateFrameNode("Tabs", TABS_NODE_ID, AceType::MakeRefPtr<Pattern>());
    auto swiper = FrameNode::CreateFrameNode("Swiper", SWIPER_NODE_ID, AceType::MakeRefPtr<Pattern>());
    swiper->MountToParent(tabs);
    manager->OnSwiperChangeStart(swiper, true);
    manager->OnSwiperChangeEnd(swiper, true);
    manager->OnContentChangeNodeDestroyed(tabs->GetId());
    manager->OnContentChangeNodeDestroyed(swiper->GetId());
    EXPECT_EQ(a.types, (std::vector<uint32_t> { TABS_START, TABS_END }));
    EXPECT_TRUE(b.types.empty());
    manager->capiImpl_->FlushCapiSwiperEndEvents();
    EXPECT_EQ(a.types.size(), PAIRED_EVENT_COUNT);
    EXPECT_EQ(b.types, (std::vector<uint32_t> { TABS_END }));
    EXPECT_NE(b.json.find(std::to_string(TABS_NODE_ID)), std::string::npos);
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(aid));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(bid));
}

/**
 * @tc.name: CapiTabsMissingAncestorFallsBackToSwiper
 * @tc.desc: Tabs notifications fall back to the Swiper target when its Tabs ancestor is unavailable.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiTabsMissingAncestorFallsBackToSwiper, TestSize.Level1)
{
    constexpr int32_t SWIPER_NODE_ID = 507;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto swiper = FrameNode::CreateFrameNode("Swiper", SWIPER_NODE_ID, AceType::MakeRefPtr<Pattern>());
    manager->OnSwiperChangeStart(swiper, true);
    manager->OnSwiperChangeEnd(swiper, true);
    manager->capiImpl_->FlushCapiSwiperEndEvents();
    EXPECT_EQ(state.types, (std::vector<uint32_t> { TABS_START, TABS_END }));
    EXPECT_NE(state.json.find(std::to_string(SWIPER_NODE_ID)), std::string::npos);
    EXPECT_NE(state.json.find("Swiper"), std::string::npos);
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiPendingEndMatchesSubscribersAtDispatch
 * @tc.desc: Deferred normal END matches listeners at dispatch, including late registrations.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiPendingEndMatchesSubscribersAtDispatch, TestSize.Level1)
{
    constexpr int32_t SWIPER_NODE_ID = 508;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState a { manager };
    CapiTestState b { manager };
    auto aid = manager->RegisterCapiContentChangeEvent(
        false, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &a },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto swiper = FrameNode::CreateFrameNode("Swiper", SWIPER_NODE_ID, AceType::MakeRefPtr<Pattern>());
    manager->OnSwiperChangeEnd(swiper, false);
    auto bid = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &b },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(aid));
    manager->capiImpl_->FlushCapiSwiperEndEvents();
    EXPECT_TRUE(a.types.empty());
    EXPECT_EQ(b.types, (std::vector<uint32_t> { SWIPER_END }));
    manager->OnSwiperChangeEnd(swiper, false);
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(bid));
    auto next = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &b },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    manager->capiImpl_->FlushCapiSwiperEndEvents();
    EXPECT_EQ(b.types.size(), 1U);
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(next));
}

/**
 * @tc.name: CapiFailedRegistrationRetainsCookieOwnership
 * @tc.desc: Failed registration leaves callback-cookie ownership with the caller.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiFailedRegistrationRetainsCookieOwnership, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto* cookie = new CapiTestCookie { &state };
    auto previous = ContentChangeManager::CapiImpl::nextCapiSubscriptionId_.exchange(0);
    auto id = manager->RegisterCapiContentChangeEvent(
        false, PAGE_CATEGORY, NO_IGNORED_EVENTS, cookie, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    ContentChangeManager::CapiImpl::nextCapiSubscriptionId_.store(previous);
    EXPECT_EQ(id, 0U);
    EXPECT_EQ(state.releases, 0);
    EXPECT_FALSE(manager->HasCapiContentChangeSubscriptions());
    ReleaseCapiContentChangeTestCookie(cookie);
    EXPECT_EQ(state.releases, 1);
}

/**
 * @tc.name: CapiSubtreeIncludesDisappearingChildren
 * @tc.desc: Normal event snapshots include disappearing children in the existing tree traversal.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiSubtreeIncludesDisappearingChildren, TestSize.Level1)
{
    constexpr int32_t PAGE_NODE_ID = 509;
    constexpr int32_t CHILD_NODE_ID = 510;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        false, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto page = FrameNode::CreateFrameNode("page", PAGE_NODE_ID, AceType::MakeRefPtr<Pattern>());
    auto child = FrameNode::CreateFrameNode("Text", CHILD_NODE_ID, AceType::MakeRefPtr<Pattern>());
    page->AddDisappearingChild(child, 0);
    manager->OnPageTransitionEnd(page);
    EXPECT_EQ(state.types, (std::vector<uint32_t> { PAGE_END }));
    EXPECT_NE(state.json.find(std::to_string(CHILD_NODE_ID)), std::string::npos);
    EXPECT_TRUE(page->RemoveDisappearingChild(child));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiTabsPendingEndFallsBackAfterAncestorReleased
 * @tc.desc: Pending Tabs END resolves a fallback target after its ancestor is released.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiTabsPendingEndFallsBackAfterAncestorReleased, TestSize.Level1)
{
    constexpr int32_t TABS_NODE_ID = 515;
    constexpr int32_t SWIPER_NODE_ID = 516;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        false, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto tabs = FrameNode::CreateFrameNode("Tabs", TABS_NODE_ID, AceType::MakeRefPtr<Pattern>());
    auto swiper = FrameNode::CreateFrameNode("Swiper", SWIPER_NODE_ID, AceType::MakeRefPtr<Pattern>());
    swiper->MountToParent(tabs);
    manager->OnSwiperChangeEnd(swiper, true);
    tabs->RemoveChild(swiper);
    tabs.Reset();
    manager->capiImpl_->FlushCapiSwiperEndEvents();
    EXPECT_EQ(state.types, (std::vector<uint32_t> { TABS_END }));
    EXPECT_NE(state.json.find(std::to_string(SWIPER_NODE_ID)), std::string::npos);
    EXPECT_NE(state.json.find("Swiper"), std::string::npos);
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiDestroyedPageUsesCachedMetadataWithoutRetainingNodes
 * @tc.desc: Destroyed page END uses cached metadata without retaining the node.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiDestroyedPageUsesCachedMetadataWithoutRetainingNodes, TestSize.Level1)
{
    constexpr size_t PAIRED_EVENT_COUNT = 2U;
    constexpr int32_t PAGE_NODE_ID = 520;
    constexpr int32_t CHILD_NODE_ID = 521;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto page = FrameNode::CreateFrameNode("page", PAGE_NODE_ID, AceType::MakeRefPtr<Pattern>());
    auto child = FrameNode::CreateFrameNode("Text", CHILD_NODE_ID, AceType::MakeRefPtr<Pattern>());
    page->AddChild(child);
    WeakPtr<FrameNode> weakPage = page;
    WeakPtr<FrameNode> weakChild = child;
    const auto refs = page->RefCount();
    manager->OnContentChangeStart(page, ChangeType::PAGE);
    EXPECT_EQ(page->RefCount(), refs);
    // Release both nodes before delivering the ID-only notification.
    child.Reset();
    page.Reset();
    EXPECT_EQ(weakPage.Upgrade(), nullptr);
    EXPECT_EQ(weakChild.Upgrade(), nullptr);
    manager->OnContentChangeNodeDestroyed(PAGE_NODE_ID);
    EXPECT_EQ(state.types, (std::vector<uint32_t> { PAGE_START, PAGE_END }));
    auto json = JsonUtil::ParseJsonString(state.json);
    EXPECT_EQ(json->GetInt("eventType"), PAGE_END);
    EXPECT_EQ(json->GetValue("targetNode")->GetString("id"), std::to_string(PAGE_NODE_ID));
    EXPECT_TRUE(json->GetValue("subtree")->IsNull());
    manager->OnContentChangeNodeDestroyed(PAGE_NODE_ID);
    EXPECT_EQ(state.types.size(), PAIRED_EVENT_COUNT);
    EXPECT_TRUE(manager->capiImpl_->capiSubscriptions_.at(id)->starts.empty());
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiStartAndDestroyedEndDoNotTraverseChildren
 * @tc.desc: START and destroyed-node paired END do not traverse the component subtree.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiStartAndDestroyedEndDoNotTraverseChildren, TestSize.Level1)
{
    constexpr int32_t CHILD_NODE_ID = 532;
    constexpr int32_t PAGE_NODE_ID = 530;
    constexpr int32_t PROBE_NODE_ID = 531;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto page = FrameNode::CreateFrameNode("page", PAGE_NODE_ID, AceType::MakeRefPtr<Pattern>());
    auto probe = AceType::MakeRefPtr<CapiTreeReadProbe>(PROBE_NODE_ID);
    probe->AddChild(FrameNode::CreateFrameNode("Text", CHILD_NODE_ID, AceType::MakeRefPtr<Pattern>()));
    page->AddChild(probe);
    probe->reads = 0;
    manager->OnContentChangeStart(page, ChangeType::PAGE);
    EXPECT_EQ(probe->reads, 0U);
    EXPECT_TRUE(JsonUtil::ParseJsonString(state.json)->GetValue("subtree")->IsNull());
    manager->OnContentChangeNodeDestroyed(page->GetId());
    EXPECT_EQ(probe->reads, 0U);
    EXPECT_TRUE(JsonUtil::ParseJsonString(state.json)->GetValue("subtree")->IsNull());
    manager->OnContentChangeStart(page, ChangeType::PAGE);
    manager->OnPageTransitionEnd(page);
    EXPECT_GT(probe->reads, 0U);
    auto json = JsonUtil::ParseJsonString(state.json);
    auto children = json->GetValue("subtree")->GetValue("children");
    ASSERT_EQ(children->GetArraySize(), 1);
    EXPECT_EQ(children->GetArrayItem(0)->GetString("id"), std::to_string(CHILD_NODE_ID));
    EXPECT_EQ(state.types, (std::vector<uint32_t> { PAGE_START, PAGE_END, PAGE_START, PAGE_END }));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiPageInterruptUsesMetadataLikeIpc
 * @tc.desc: Page interruption reports paired metadata without a normal-completion subtree.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiPageInterruptUsesMetadataLikeIpc, TestSize.Level1)
{
    constexpr int32_t PAGE_NODE_ID = 542;
    constexpr int32_t PROBE_NODE_ID = 543;
    auto manager = GetContentChangeManager();
    manager->InvalidateCapiContentChangeSubscriptions();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto page = FrameNode::CreateFrameNode("page", PAGE_NODE_ID, AceType::MakeRefPtr<Pattern>());
    auto probe = AceType::MakeRefPtr<CapiTreeReadProbe>(PROBE_NODE_ID);
    page->AddChild(probe);
    EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(ChangeType::PAGE_START, _)).Times(1);
    EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(ChangeType::PAGE, _))
        .WillOnce([=](ChangeType, const std::string& data) {
            auto json = JsonUtil::ParseJsonString(data);
            EXPECT_EQ(json->GetInt("$ID"), PAGE_NODE_ID);
            EXPECT_FALSE(json->Contains("$children"));
        });
    probe->reads = 0;
    manager->OnContentChangeStart(page, ChangeType::PAGE);
    manager->OnContentChangeInterrupted(page, ChangeType::PAGE);
    EXPECT_EQ(probe->reads, 0U);
    EXPECT_EQ(state.types, (std::vector<uint32_t> { PAGE_START, PAGE_END }));
    auto json = JsonUtil::ParseJsonString(state.json);
    EXPECT_EQ(json->GetValue("targetNode")->GetString("id"), std::to_string(PAGE_NODE_ID));
    EXPECT_TRUE(json->GetValue("subtree")->IsNull());
    Mock::VerifyAndClearExpectations(GetMockUiSessionManager());
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
    manager->StopContentChangeReport();
}

/**
 * @tc.name: CapiScrollInterruptCompletesEachStartedNode
 * @tc.desc: Interrupted scroll closes each accepted START; END-only subscriptions do not receive interrupt events.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiScrollInterruptCompletesEachStartedNode, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    for (bool withStart : { false, true }) {
        manager->InvalidateCapiContentChangeSubscriptions();
        CapiTestState state { manager };
        auto id = manager->RegisterCapiContentChangeEvent(
            withStart, SCROLL_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
            CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
        auto first = FrameNode::CreateFrameNode(
            V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
        auto second = FrameNode::CreateFrameNode(
            V2::SCROLL_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ScrollPattern>());
        EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(_, _)).Times(0);
        manager->OnScrollChangeStart(first);
        manager->OnContentChangeStart(first, ChangeType::SCROLL);
        manager->OnScrollChangeStart(second);
        manager->OnContentChangeStart(second, ChangeType::SCROLL);
        manager->MarkCapiScrollToEventSource(first->GetId());
        manager->OnScrollInterrupted(first, false);
        EXPECT_EQ(manager->capiImpl_->capiScrollingNodes_.count(second->GetId()), 1U);
        EXPECT_EQ(manager->capiImpl_->capiScrollingNodes_.count(first->GetId()), 0U);
        EXPECT_TRUE(manager->capiImpl_->capiScrollSources_.empty());
        EXPECT_FALSE(manager->capiImpl_->HasCapiContentChangeStart(first->GetId(), SCROLL_START));
        EXPECT_EQ(state.types,
            withStart ? (std::vector<uint32_t> { SCROLL_START, SCROLL_START, SCROLL_END }) : std::vector<uint32_t>());
        manager->OnScrollInterrupted(second, false);
        manager->OnScrollInterrupted(second, false);
        EXPECT_TRUE(manager->capiImpl_->capiScrollingNodes_.empty());
        EXPECT_FALSE(manager->capiImpl_->HasCapiContentChangeStart(second->GetId(), SCROLL_START));
        EXPECT_EQ(state.types,
            withStart ? (std::vector<uint32_t> { SCROLL_START, SCROLL_START, SCROLL_END, SCROLL_END }) :
                std::vector<uint32_t>());
        Mock::VerifyAndClearExpectations(GetMockUiSessionManager());
        EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
    }
}

/**
 * @tc.name: CapiTabsInterruptDropsManualStartAfterAutoPlay
 * @tc.desc: Enabling autoPlay drops an unfinished Tabs cycle without END or CANCEL.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiTabsInterruptDropsManualStartAfterAutoPlay, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    for (bool autoPlay : { false, true }) {
        manager->InvalidateCapiContentChangeSubscriptions();
        CapiTestState withStart { manager };
        CapiTestState withoutStart { manager };
        auto first = manager->RegisterCapiContentChangeEvent(
            true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &withStart },
            CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
        auto second = manager->RegisterCapiContentChangeEvent(
            false, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &withoutStart },
            CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
        auto tabs = FrameNode::CreateFrameNode(
            V2::TABS_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        auto pattern = AceType::MakeRefPtr<SwiperPattern>();
        auto swiper = FrameNode::CreateFrameNode(
            V2::SWIPER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), pattern);
        swiper->MountToParent(tabs);
        EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(_, _)).Times(0);
        manager->OnSwiperChangeStart(swiper, true);
        swiper->GetPaintProperty<SwiperPaintProperty>()->UpdateAutoPlay(autoPlay);
        manager->OnSwiperChangeInterrupted(swiper, true);
        EXPECT_EQ(withStart.types, (std::vector<uint32_t> { TABS_START }));
        manager->OnVsyncEnd(RectF());
        manager->OnSwiperChangeInterrupted(swiper, true);
        manager->OnVsyncEnd(RectF());
        EXPECT_EQ(withStart.types, autoPlay ? (std::vector<uint32_t> { TABS_START }) :
            (std::vector<uint32_t> { TABS_START, TABS_END }));
        EXPECT_TRUE(withoutStart.types.empty());
        EXPECT_FALSE(manager->capiImpl_->HasCapiContentChangeStart(tabs->GetId(), TABS_START));
        EXPECT_TRUE(manager->capiImpl_->capiActiveSwipers_.empty());
        EXPECT_TRUE(manager->capiImpl_->capiPendingSwipers_.empty());
        Mock::VerifyAndClearExpectations(GetMockUiSessionManager());
        EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(first));
        EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(second));
    }
}

/**
 * @tc.name: CapiFrameNodeDestructorReportsCachedEndAtZeroReferences
 * @tc.desc: Final node destruction reports cached END without claiming the dying node.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiFrameNodeDestructorReportsCachedEndAtZeroReferences, TestSize.Level1)
{
    constexpr int32_t NODE_NODE_ID = 522;
    auto pipeline = MockPipelineContext::GetCurrent();
    auto manager = pipeline->GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY | SCROLL_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    for (auto type : { ChangeType::PAGE, ChangeType::SCROLL }) {
        auto node = FrameNode::CreateFrameNode("node", NODE_NODE_ID, AceType::MakeRefPtr<Pattern>());
        node->RegisterReleaseFunc(false);
        node->AttachToMainTree(false, AceType::RawPtr(pipeline));
        manager->OnContentChangeStart(node, type);
        auto* rawNode = AceType::RawPtr(node);
        state.onEvent = [rawNode](uint32_t eventType) {
            EXPECT_TRUE(eventType == PAGE_END || eventType == SCROLL_END);
            EXPECT_EQ(rawNode->RefCount(), 0);
        };
        WeakPtr<FrameNode> weak = node;
        node.Reset();
        EXPECT_EQ(weak.Upgrade(), nullptr);
        state.onEvent = nullptr;
    }
    EXPECT_EQ(state.types, (std::vector<uint32_t> { PAGE_START, PAGE_END, SCROLL_START, SCROLL_END }));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiNormalPageEndUsesFreshTreeAndReleasesCache
 * @tc.desc: Normal page END uses a fresh subtree and releases its START metadata cache.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiNormalPageEndUsesFreshTreeAndReleasesCache, TestSize.Level1)
{
    constexpr int32_t PAGE_NODE_ID = 523;
    constexpr int32_t CHILD_NODE_ID = 524;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto page = FrameNode::CreateFrameNode("page", PAGE_NODE_ID, AceType::MakeRefPtr<Pattern>());
    manager->OnContentChangeStart(page, ChangeType::PAGE);
    std::weak_ptr<const std::string> cache =
        manager->capiImpl_->capiSubscriptions_.at(id)->starts.at({ PAGE_NODE_ID, PAGE_START });
    auto child = FrameNode::CreateFrameNode("Button", CHILD_NODE_ID, AceType::MakeRefPtr<Pattern>());
    page->AddChild(child);
    manager->OnPageTransitionEnd(page);
    auto json = JsonUtil::ParseJsonString(state.json);
    EXPECT_EQ(json->GetValue("subtree")->GetValue("children")->GetArraySize(), 1);
    EXPECT_TRUE(cache.expired());
    manager->OnContentChangeNodeDestroyed(PAGE_NODE_ID);
    EXPECT_EQ(state.types, (std::vector<uint32_t> { PAGE_START, PAGE_END }));
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiDestroyedTabsUsesCachedLogicalTarget
 * @tc.desc: Destroyed Tabs END retains the cached logical target identity.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiDestroyedTabsUsesCachedLogicalTarget, TestSize.Level1)
{
    constexpr size_t PAIRED_EVENT_COUNT = 2U;
    constexpr int32_t TABS_NODE_ID = 525;
    constexpr int32_t SWIPER_NODE_ID = 526;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto tabs = FrameNode::CreateFrameNode("Tabs", TABS_NODE_ID, AceType::MakeRefPtr<Pattern>());
    auto swiper = FrameNode::CreateFrameNode("Swiper", SWIPER_NODE_ID, AceType::MakeRefPtr<Pattern>());
    swiper->MountToParent(tabs);
    manager->OnSwiperChangeStart(swiper, true);
    manager->OnSwiperChangeEnd(swiper, true);
    WeakPtr<FrameNode> weakTabs = tabs;
    tabs.Reset();
    EXPECT_EQ(weakTabs.Upgrade(), nullptr);
    manager->OnContentChangeNodeDestroyed(TABS_NODE_ID);
    EXPECT_EQ(state.types, (std::vector<uint32_t> { TABS_START, TABS_END }));
    auto json = JsonUtil::ParseJsonString(state.json);
    EXPECT_EQ(json->GetValue("targetNode")->GetString("id"), std::to_string(TABS_NODE_ID));
    EXPECT_EQ(json->GetValue("targetNode")->GetString("type"), "Tabs");
    manager->OnContentChangeNodeDestroyed(SWIPER_NODE_ID);
    manager->capiImpl_->FlushCapiSwiperEndEvents();
    EXPECT_EQ(state.types.size(), PAIRED_EVENT_COUNT);
    EXPECT_TRUE(manager->capiImpl_->capiSubscriptions_.at(id)->starts.empty());
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiCachedDestroyEndPreservesReentrantStart
 * @tc.desc: A new START created by a destroyed-node callback survives old transaction cleanup.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiCachedDestroyEndPreservesReentrantStart, TestSize.Level1)
{
    constexpr size_t TOTAL_EVENT_COUNT = 6U;
    constexpr int32_t NODE_NODE_ID = 527;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY | SCROLL_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto node = FrameNode::CreateFrameNode("node", NODE_NODE_ID, AceType::MakeRefPtr<Pattern>());
    manager->OnContentChangeStart(node, ChangeType::PAGE);
    manager->OnContentChangeStart(node, ChangeType::SCROLL);
    state.onEvent = [&](uint32_t type) {
        if (type == PAGE_END) {
            state.onEvent = nullptr;
            manager->OnContentChangeNodeDestroyed(NODE_NODE_ID);
            manager->OnContentChangeStart(node, ChangeType::SCROLL);
        }
    };
    manager->OnContentChangeNodeDestroyed(NODE_NODE_ID);
    EXPECT_EQ(state.types, (std::vector<uint32_t> { PAGE_START, SCROLL_START, PAGE_END, SCROLL_END, SCROLL_START }));
    EXPECT_TRUE(manager->capiImpl_->HasCapiContentChangeStart(NODE_NODE_ID, SCROLL_START));
    manager->OnContentChangeNodeDestroyed(NODE_NODE_ID);
    EXPECT_EQ(state.types.back(), SCROLL_END);
    EXPECT_EQ(state.types.size(), TOTAL_EVENT_COUNT);
    EXPECT_TRUE(manager->capiImpl_->capiSubscriptions_.at(id)->starts.empty());
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
}

/**
 * @tc.name: CapiCachedDestroyEndSkipsUnregisteredReceiver
 * @tc.desc: Unregister skips both the current cached END and later queued END callbacks.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiCachedDestroyEndSkipsUnregisteredReceiver, TestSize.Level1)
{
    constexpr int32_t NODE_NODE_ID = 528;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState first { manager };
    CapiTestState second { manager };
    auto firstId = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY | SCROLL_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &first },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto secondId = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY | SCROLL_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &second },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto node = FrameNode::CreateFrameNode("node", NODE_NODE_ID, AceType::MakeRefPtr<Pattern>());
    manager->OnContentChangeStart(node, ChangeType::PAGE);
    manager->OnContentChangeStart(node, ChangeType::SCROLL);
    first.removeId = secondId;
    manager->OnContentChangeNodeDestroyed(NODE_NODE_ID);
    EXPECT_EQ(first.types, (std::vector<uint32_t> { PAGE_START, SCROLL_START, PAGE_END, SCROLL_END }));
    EXPECT_EQ(second.types, (std::vector<uint32_t> { PAGE_START, SCROLL_START }));
    EXPECT_EQ(second.releases, 1);
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(firstId));
}

/**
 * @tc.name: CapiCachedEndReleasedOnCancelUnregisterAndInvalidation
 * @tc.desc: Cancel, unregister and context invalidation release cached paired-event metadata.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiCachedEndReleasedOnCancelUnregisterAndInvalidation, TestSize.Level1)
{
    constexpr int32_t RELEASED_SUBSCRIPTION_COUNT = 3;
    constexpr int32_t SWIPER_NODE_ID = 529;
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState state { manager };
    auto first = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto second = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto swiper = FrameNode::CreateFrameNode("Swiper", SWIPER_NODE_ID, AceType::MakeRefPtr<Pattern>());
    manager->OnSwiperChangeStart(swiper, false);
    std::weak_ptr<const std::string> cache =
        manager->capiImpl_->capiSubscriptions_.at(first)->starts.at({ SWIPER_NODE_ID, SWIPER_START });
    EXPECT_EQ(cache.lock(),
        manager->capiImpl_->capiSubscriptions_.at(second)->starts.at({ SWIPER_NODE_ID, SWIPER_START }));
    manager->OnSwiperChangeCancel(swiper, false);
    EXPECT_TRUE(cache.expired());
    manager->OnSwiperChangeStart(swiper, false);
    cache = manager->capiImpl_->capiSubscriptions_.at(first)->starts.at({ SWIPER_NODE_ID, SWIPER_START });
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(first));
    EXPECT_FALSE(cache.expired());
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(second));
    EXPECT_TRUE(cache.expired());
    auto third = manager->RegisterCapiContentChangeEvent(
        true, PAGE_CATEGORY, NO_IGNORED_EVENTS, new CapiTestCookie { &state },
        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    manager->OnSwiperChangeStart(swiper, false);
    cache = manager->capiImpl_->capiSubscriptions_.at(third)->starts.at({ SWIPER_NODE_ID, SWIPER_START });
    manager->InvalidateCapiContentChangeSubscriptions();
    EXPECT_TRUE(cache.expired());
    manager->OnContentChangeNodeDestroyed(SWIPER_NODE_ID);
    EXPECT_EQ(state.types,
        (std::vector<uint32_t> { SWIPER_START, SWIPER_START, SWIPER_CANCEL, SWIPER_CANCEL, SWIPER_START,
        SWIPER_START, SWIPER_START }));
    EXPECT_EQ(state.releases, RELEASED_SUBSCRIPTION_COUNT);
}


/**
 * @tc.name: CapiScrollNormalEndIsIndependentPerNode
 * @tc.desc: Both subscription modes receive each node normal END; IPC retains its last-node behavior.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiScrollNormalEndIsIndependentPerNode, TestSize.Level1)
{
    constexpr int32_t SCROLLING_NODE_COUNT = 2;
    constexpr size_t COMPLETED_EVENT_COUNT = 4;
    auto manager = GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    CapiTestState paired { manager };
    CapiTestState unpaired { manager };
    auto firstId = manager->RegisterCapiContentChangeEvent(true, SCROLL_CATEGORY, NO_IGNORED_EVENTS,
        new CapiTestCookie { &paired }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto secondId = manager->RegisterCapiContentChangeEvent(false, SCROLL_CATEGORY, NO_IGNORED_EVENTS,
        new CapiTestCookie { &unpaired }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto first = FrameNode::CreateFrameNode(
        "Scroll", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto second = FrameNode::CreateFrameNode(
        "List", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(ChangeType::SCROLL_START, _))
        .Times(SCROLLING_NODE_COUNT);
    for (const auto& node : { first, second }) {
        manager->OnScrollChangeStart(node);
        manager->OnContentChangeStart(node, ChangeType::SCROLL);
    }
    Mock::VerifyAndClearExpectations(GetMockUiSessionManager());
    EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(ChangeType::SCROLL, _)).Times(0);
    manager->OnScrollChangeEnd(first);
    EXPECT_EQ(paired.types, (std::vector<uint32_t> { SCROLL_START, SCROLL_START, SCROLL_END }));
    EXPECT_EQ(unpaired.types, (std::vector<uint32_t> { SCROLL_END }));
    Mock::VerifyAndClearExpectations(GetMockUiSessionManager());
    EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(ChangeType::SCROLL, _)).Times(1);
    manager->OnScrollChangeEnd(second);
    EXPECT_EQ(paired.types.size(), COMPLETED_EVENT_COUNT);
    EXPECT_EQ(unpaired.types, (std::vector<uint32_t> { SCROLL_END, SCROLL_END }));
    Mock::VerifyAndClearExpectations(GetMockUiSessionManager());
    EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(_, _)).Times(0);
    manager->OnScrollInterrupted(first, true);
    EXPECT_EQ(paired.types.size(), COMPLETED_EVENT_COUNT);
    manager->UnregisterCapiContentChangeEvent(firstId);
    manager->UnregisterCapiContentChangeEvent(secondId);
    manager->StopContentChangeReport();
    Mock::VerifyAndClearExpectations(GetMockUiSessionManager());
}

/**
 * @tc.name: CapiNormalEndDoesNotRequireStart
 * @tc.desc: Both START-enabled and END-only receivers accept normal END without a previous START.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiNormalEndDoesNotRequireStart, TestSize.Level1)
{
    constexpr int32_t IPC_NORMAL_END_COUNT = 3;
    auto manager = GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    CapiTestState paired { manager };
    CapiTestState unpaired { manager };
    auto aid = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY | SCROLL_CATEGORY, NO_IGNORED_EVENTS,
        new CapiTestCookie { &paired }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto bid = manager->RegisterCapiContentChangeEvent(false, PAGE_CATEGORY | SCROLL_CATEGORY, NO_IGNORED_EVENTS,
        new CapiTestCookie { &unpaired }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto node = FrameNode::CreateFrameNode(
        "Swiper", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SwiperPattern>());
    EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(_, _)).Times(IPC_NORMAL_END_COUNT);
    manager->OnPageTransitionEnd(node);
    manager->OnScrollChangeEnd(node);
    manager->OnSwiperChangeEnd(node, false);
    manager->capiImpl_->FlushCapiSwiperEndEvents();
    manager->ProcessSwiperNodes();
    EXPECT_EQ(paired.types, (std::vector<uint32_t> { PAGE_END, SCROLL_END, SWIPER_END }));
    EXPECT_EQ(unpaired.types, (std::vector<uint32_t> { PAGE_END, SCROLL_END, SWIPER_END }));
    manager->UnregisterCapiContentChangeEvent(aid);
    manager->UnregisterCapiContentChangeEvent(bid);
    manager->StopContentChangeReport();
    Mock::VerifyAndClearExpectations(GetMockUiSessionManager());
}

/**
 * @tc.name: StrictPairingKeepsAcceptedStartAcrossFilters
 * @tc.desc: Changing the scroll source cannot suppress the terminal event of an already accepted START.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, StrictPairingKeepsAcceptedStartAcrossFilters, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    config.ignoreEventType = R"({"SCROLL":["scrollTo"]})";
    manager->StartContentChangeReport(config);
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(true, SCROLL_CATEGORY, IGNORE_SCROLL_TO,
        new CapiTestCookie { &state }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto node = FrameNode::CreateFrameNode(
        "Scroll", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(ChangeType::SCROLL_START, _)).Times(1);
    manager->OnScrollChangeStart(node);
    manager->OnContentChangeStart(node, ChangeType::SCROLL);
    EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(ChangeType::SCROLL, _)).Times(0);
    manager->MarkCapiScrollToEventSource(node->GetId());
    manager->OnScrollChangeEnd(node, ContentChangeManager::SCROLL_TO);
    EXPECT_EQ(state.types, (std::vector<uint32_t> { SCROLL_START, SCROLL_END }));
    manager->UnregisterCapiContentChangeEvent(id);
    manager->StopContentChangeReport();
    Mock::VerifyAndClearExpectations(GetMockUiSessionManager());
}

/**
 * @tc.name: CapiDropsManualStartAfterAutoPlayEnabled
 * @tc.desc: AutoPlay suppresses terminal events and discards the unfinished manual START.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiDropsManualStartAfterAutoPlayEnabled, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY, NO_IGNORED_EVENTS,
        new CapiTestCookie { &state }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto node = FrameNode::CreateFrameNode(
        "Swiper", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SwiperPattern>());
    EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(ChangeType::SWIPER_START, _)).Times(1);
    manager->OnSwiperChangeStart(node, false);
    node->GetPaintProperty<SwiperPaintProperty>()->UpdateAutoPlay(true);
    EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(ChangeType::SWIPER, _)).Times(0);
    manager->OnSwiperChangeInterrupted(node, false);
    manager->OnVsyncEnd(RectF());
    EXPECT_EQ(state.types, (std::vector<uint32_t> { SWIPER_START }));
    EXPECT_FALSE(manager->capiImpl_->HasCapiContentChangeStart(node->GetId(), SWIPER_START));
    manager->UnregisterCapiContentChangeEvent(id);
    manager->StopContentChangeReport();
    Mock::VerifyAndClearExpectations(GetMockUiSessionManager());
}

/**
 * @tc.name: StrictPairingOrdersReentrantStartAndEndForEveryReceiver
 * @tc.desc: A nested completion from the first START callback must not overtake START for the second receiver.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, StrictPairingOrdersReentrantStartAndEndForEveryReceiver, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    CapiTestState first { manager };
    CapiTestState second { manager };
    auto node = FrameNode::CreateFrameNode(
        "Page", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    first.onEvent = [manager, node](uint32_t type) {
        if (type == PAGE_START) {
            manager->OnPageTransitionEnd(node);
        }
    };
    auto aid = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY, NO_IGNORED_EVENTS,
        new CapiTestCookie { &first }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto bid = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY, NO_IGNORED_EVENTS,
        new CapiTestCookie { &second }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    manager->OnContentChangeStart(node, ChangeType::PAGE);
    EXPECT_EQ(first.types, (std::vector<uint32_t> { PAGE_START, PAGE_END }));
    EXPECT_EQ(second.types, first.types);
    manager->UnregisterCapiContentChangeEvent(aid);
    manager->UnregisterCapiContentChangeEvent(bid);
}

/**
 * @tc.name: StrictPairingPendingDestroyCannotConsumeNewCycle
 * @tc.desc: A queued old END skips completed receivers without erasing a later START on the same node.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, StrictPairingPendingDestroyCannotConsumeNewCycle, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY, NO_IGNORED_EVENTS,
        new CapiTestCookie { &state }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto swiper = FrameNode::CreateFrameNode(
        V2::SWIPER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SwiperPattern>());
    manager->OnSwiperChangeStart(swiper, false);
    manager->OnSwiperChangeEnd(swiper, false);
    manager->OnContentChangeNodeDestroyed(swiper->GetId());
    manager->OnSwiperChangeStart(swiper, false);
    manager->capiImpl_->FlushCapiSwiperEndEvents();
    EXPECT_EQ(state.types, (std::vector<uint32_t> { SWIPER_START, SWIPER_END, SWIPER_START }));
    EXPECT_TRUE(manager->capiImpl_->HasCapiContentChangeStart(swiper->GetId(), SWIPER_START));
    EXPECT_FALSE(manager->capiImpl_->capiActiveSwipers_.empty());
    manager->OnSwiperChangeEnd(swiper, false);
    manager->capiImpl_->FlushCapiSwiperEndEvents();
    EXPECT_EQ(state.types, (std::vector<uint32_t> { SWIPER_START, SWIPER_END, SWIPER_START, SWIPER_END }));
    EXPECT_FALSE(manager->capiImpl_->HasCapiContentChangeStart(swiper->GetId(), SWIPER_START));
    manager->UnregisterCapiContentChangeEvent(id);
}

/**
 * @tc.name: ContentChangeManagerPageLogicalIdTest001
 * @tc.desc: An explicit logical ID closes only its START while END dumps a different node.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerPageLogicalIdTest001, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    auto mock = GetMockUiSessionManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    auto navigation = FrameNode::CreateFrameNode("Navigation", 450, AceType::MakeRefPtr<Pattern>(), true);
    auto target = FrameNode::CreateFrameNode("NavDestination", 451, AceType::MakeRefPtr<Pattern>(), true);
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE_START, _)).Times(2);
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE, GetSimpleTree(target))).Times(2);
    manager->OnTransitionAdded(navigation->GetId());
    manager->OnContentChangeStart(navigation, ChangeType::PAGE);
    manager->OnTransitionAdded(target->GetId());
    manager->OnContentChangeStart(target, ChangeType::PAGE);
    manager->OnPageTransitionEnd(target, navigation->GetId());
    EXPECT_EQ(manager->transitioningNodes_.count(navigation->GetId()), 0);
    EXPECT_EQ(manager->activeContentChanges_.count(navigation->GetId()), 0);
    EXPECT_EQ(manager->transitioningNodes_.count(target->GetId()), 1);
    EXPECT_EQ(manager->activeContentChanges_.count(target->GetId()), 1);
    manager->OnPageTransitionEnd(target);
    EXPECT_FALSE(manager->IsTransitioning());
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mock);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeManagerPageLogicalIdTest002
 * @tc.desc: Interruption clears Navigation identity and allows a subsequent START without PAGE CANCEL.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerPageLogicalIdTest002, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    auto mock = GetMockUiSessionManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    auto navigation = FrameNode::CreateFrameNode("Navigation", 452, AceType::MakeRefPtr<Pattern>(), true);
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE_START, HasSubstr("\"$ID\":452"))).Times(2);
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE, HasSubstr("\"$ID\":452"))).Times(2);
    for (int32_t i = 0; i < 2; ++i) {
        manager->OnTransitionAdded(navigation->GetId());
        manager->OnContentChangeStart(navigation, ChangeType::PAGE);
        manager->OnContentChangeInterrupted(navigation, ChangeType::PAGE);
        EXPECT_FALSE(manager->IsTransitioning());
        EXPECT_TRUE(manager->activeContentChanges_.empty());
    }
    Mock::VerifyAndClearExpectations(mock);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: ContentChangeManagerPageLogicalIdTest003
 * @tc.desc: A missing dump node does not prevent cleanup of the explicit logical ID.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, ContentChangeManagerPageLogicalIdTest003, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    auto mock = GetMockUiSessionManager();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    auto navigation = FrameNode::CreateFrameNode("Navigation", 453, AceType::MakeRefPtr<Pattern>(), true);
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE_START, _)).Times(1);
    EXPECT_CALL(*mock, ReportContentChangeEvent(ChangeType::PAGE, _)).Times(0);
    manager->OnTransitionAdded(navigation->GetId());
    manager->OnContentChangeStart(navigation, ChangeType::PAGE);
    manager->OnPageTransitionEnd(nullptr, navigation->GetId());
    EXPECT_FALSE(manager->IsTransitioning());
    EXPECT_TRUE(manager->activeContentChanges_.empty());
    Mock::VerifyAndClearExpectations(mock);
    manager->StopContentChangeReport();
}

/**
 * @tc.name: CapiPageLogicalIdPreservesBothChannels
 * @tc.desc: Logical START identity and END snapshot target remain distinct with each IPC START configuration.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiPageLogicalIdPreservesBothChannels, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    auto ipc = GetMockUiSessionManager();
    for (int32_t ipcMode = -1; ipcMode <= 1; ++ipcMode) {
        SCOPED_TRACE(ipcMode);
        manager->StopContentChangeReport();
        if (ipcMode >= 0) {
            ContentChangeConfig config;
            config.reportStartEvent = ipcMode == 1;
            manager->StartContentChangeReport(config);
        }
        CapiTestState state { manager };
        auto id = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY, NO_IGNORED_EVENTS,
            new CapiTestCookie { &state }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
        auto navigation = FrameNode::CreateFrameNode(V2::NAVIGATION_VIEW_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        auto target = FrameNode::CreateFrameNode(V2::NAVDESTINATION_VIEW_ETS_TAG,
            ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        EXPECT_CALL(*ipc, ReportContentChangeEvent(ChangeType::PAGE_START, _)).Times(ipcMode == 1 ? 2 : 0);
        EXPECT_CALL(*ipc, ReportContentChangeEvent(ChangeType::PAGE, _)).Times(ipcMode >= 0 ? 2 : 0);
        manager->OnContentChangeStart(navigation, ChangeType::PAGE);
        manager->OnContentChangeStart(target, ChangeType::PAGE);
        manager->OnPageTransitionEnd(target, navigation->GetId());
        EXPECT_EQ(state.types, (std::vector<uint32_t> { PAGE_START, PAGE_START, PAGE_END }));
        EXPECT_FALSE(manager->capiImpl_->HasCapiContentChangeStart(navigation->GetId(), PAGE_START));
        EXPECT_TRUE(manager->capiImpl_->HasCapiContentChangeStart(target->GetId(), PAGE_START));
        auto json = JsonUtil::ParseJsonString(state.json);
        ASSERT_TRUE(json && json->IsValid());
        EXPECT_EQ(json->GetObject("targetNode")->GetString("id"), std::to_string(target->GetId()));
        manager->OnPageTransitionEnd(target);
        EXPECT_EQ(state.types, (std::vector<uint32_t> { PAGE_START, PAGE_START, PAGE_END, PAGE_END }));
        EXPECT_FALSE(manager->capiImpl_->HasCapiContentChangeStart(target->GetId(), PAGE_START));
        EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
        Mock::VerifyAndClearExpectations(ipc);
        manager->StopContentChangeReport();
    }
}

/**
 * @tc.name: CapiRegistrationInsideStartReceivesNormalEndOnly
 * @tc.desc: A listener added in START receives normal END but no replayed START or abnormal completion.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiRegistrationInsideStartReceivesNormalEndOnly, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    for (auto start : { PAGE_START, SCROLL_START, SWIPER_START, TABS_START }) {
        for (bool normal : { true, false }) {
            manager->InvalidateCapiContentChangeSubscriptions();
            CapiTestState first { manager };
            CapiTestState late { manager };
            uint64_t lateId = 0;
            auto node = FrameNode::CreateFrameNode(
                "node", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
            first.onEvent = [&](uint32_t type) {
                if (type == start) {
                    lateId = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY | SCROLL_CATEGORY,
                        NO_IGNORED_EVENTS, new CapiTestCookie { &late },
                        CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
                }
            };
            auto firstId = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY | SCROLL_CATEGORY,
                NO_IGNORED_EVENTS, new CapiTestCookie { &first },
                CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
            manager->capiImpl_->DispatchCapiContentChangeEvent(start, node);
            ASSERT_NE(lateId, 0U);
            EXPECT_TRUE(late.types.empty());
            const auto end = start + (PAGE_END - PAGE_START);
            manager->capiImpl_->DispatchCapiContentChangeEvent(
                end, node, NO_SCROLL_EVENT_SOURCE, nullptr, nullptr, !normal);
            EXPECT_EQ(first.types, (std::vector<uint32_t> { start, end }));
            EXPECT_EQ(late.types, normal ? (std::vector<uint32_t> { end }) : std::vector<uint32_t>());
            EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(firstId));
            EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(lateId));
        }
    }
}

/**
 * @tc.name: CapiOverlayUnregisterSkipsCurrentReceiver
 * @tc.desc: Unregistering B in A's SHOW skips B immediately and preserves A's current event.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiOverlayUnregisterSkipsCurrentReceiver, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState first { manager };
    CapiTestState second { manager };
    auto aid = manager->RegisterCapiContentChangeEvent(false, OVERLAY_CATEGORY, NO_IGNORED_EVENTS,
        new CapiTestCookie { &first }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    first.removeId = manager->RegisterCapiContentChangeEvent(false, OVERLAY_CATEGORY, NO_IGNORED_EVENTS,
        new CapiTestCookie { &second }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto node = FrameNode::CreateFrameNode(
        "Popup", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    manager->capiImpl_->DispatchCapiContentChangeEvent(OVERLAY_SHOW, node, NO_SCROLL_EVENT_SOURCE, "popup");
    EXPECT_EQ(first.types, (std::vector<uint32_t> { OVERLAY_SHOW }));
    EXPECT_TRUE(second.types.empty());
    EXPECT_EQ(second.releases, 1);
    EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(aid));
}

/**
 * @tc.name: CapiRouterStageCleanupAllowsNextStart
 * @tc.desc: Router null-target cleanup sends no END and allows the next Stage START in every IPC mode.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiRouterStageCleanupAllowsNextStart, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    auto ipc = GetMockUiSessionManager();
    for (const auto& enabled : { std::optional<bool>(), std::optional<bool>(false), std::optional<bool>(true) }) {
        manager->InvalidateCapiContentChangeSubscriptions();
        manager->StopContentChangeReport();
        if (enabled.has_value()) {
            ContentChangeConfig config;
            config.reportStartEvent = enabled.value();
            manager->StartContentChangeReport(config);
        }
        CapiTestState state { manager };
        auto id = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY, NO_IGNORED_EVENTS,
            new CapiTestCookie { &state }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
        auto stage = FrameNode::CreateFrameNode(
            "Stage", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        auto page = FrameNode::CreateFrameNode(
            "Page", ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        EXPECT_CALL(*ipc, ReportContentChangeEvent(ChangeType::PAGE_START, _)).Times(enabled.value_or(false) ? 2 : 0);
        EXPECT_CALL(*ipc, ReportContentChangeEvent(ChangeType::PAGE, _)).Times(enabled.has_value() ? 1 : 0);
        manager->OnContentChangeStart(stage, ChangeType::PAGE);
        manager->OnPageTransitionEnd(nullptr, stage->GetId());
        EXPECT_FALSE(manager->capiImpl_->HasCapiContentChangeStart(stage->GetId(), PAGE_START));
        EXPECT_EQ(state.types, (std::vector<uint32_t> { PAGE_START }));
        manager->OnContentChangeStart(stage, ChangeType::PAGE);
        manager->OnPageTransitionEnd(nullptr, stage->GetId());
        manager->OnPageTransitionEnd(page);
        EXPECT_EQ(state.types, (std::vector<uint32_t> { PAGE_START, PAGE_START, PAGE_END }));
        EXPECT_TRUE(manager->UnregisterCapiContentChangeEvent(id));
        Mock::VerifyAndClearExpectations(ipc);
        manager->StopContentChangeReport();
    }
}


/**
 * @tc.name: CapiSameIndexCancelsOnlyStartedListeners
 * @tc.desc: No visible change cancels a started Swiper/Tabs cycle, without END for late or END-only listeners.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiSameIndexCancelsOnlyStartedListeners, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    for (bool isTabs : { false, true }) {
        manager->InvalidateCapiContentChangeSubscriptions();
        CapiTestState a { manager };
        CapiTestState b { manager };
        CapiTestState late { manager };
        auto aid = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY, 0,
            new CapiTestCookie { &a }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
        auto bid = manager->RegisterCapiContentChangeEvent(false, PAGE_CATEGORY, 0,
            new CapiTestCookie { &b }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
        auto tabs = FrameNode::CreateFrameNode(
            V2::TABS_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
        auto node = FrameNode::CreateFrameNode(
            V2::SWIPER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SwiperPattern>());
        if (isTabs) {
            node->MountToParent(tabs);
        }
        manager->CompleteCapiSwiperChangeWithoutMove(node, isTabs);
        manager->OnVsyncEnd(RectF());
        EXPECT_TRUE(a.types.empty());
        EXPECT_TRUE(b.types.empty());
        manager->ReportCapiSwiperStartEvent(node, isTabs);
        auto lid = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY, 0,
            new CapiTestCookie { &late }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
        manager->CompleteCapiSwiperChangeWithoutMove(node, isTabs);
        manager->CompleteCapiSwiperChangeWithoutMove(node, isTabs);
        manager->OnVsyncEnd(RectF());
        EXPECT_EQ(a.types, (std::vector<uint32_t> {
            isTabs ? TABS_START : SWIPER_START, isTabs ? TABS_CANCEL : SWIPER_CANCEL }));
        EXPECT_TRUE(b.types.empty());
        EXPECT_TRUE(late.types.empty());
        EXPECT_TRUE(manager->capiImpl_->capiPendingSwipers_.empty());
        manager->UnregisterCapiContentChangeEvent(aid);
        manager->UnregisterCapiContentChangeEvent(bid);
        manager->UnregisterCapiContentChangeEvent(lid);
    }
}

/**
 * @tc.name: CapiAutoPlayDropsPendingEndAndRecoversWhenDisabled
 * @tc.desc: AutoPlay filters queued completion, manual starts and cancellation; disabling restores normal reporting.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiAutoPlayDropsPendingEndAndRecoversWhenDisabled, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    manager->StopContentChangeReport();
    manager->InvalidateCapiContentChangeSubscriptions();
    CapiTestState a { manager };
    CapiTestState b { manager };
    auto aid = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY, 0,
        new CapiTestCookie { &a }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto bid = manager->RegisterCapiContentChangeEvent(false, PAGE_CATEGORY, 0,
        new CapiTestCookie { &b }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto node = FrameNode::CreateFrameNode(
        V2::SWIPER_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<SwiperPattern>());
    auto paint = node->GetPaintProperty<SwiperPaintProperty>();
    manager->ReportCapiSwiperStartEvent(node, false);
    manager->OnSwiperChangeEnd(node, false);
    ASSERT_FALSE(manager->capiImpl_->capiPendingSwipers_.empty());
    paint->UpdateAutoPlay(true);
    manager->OnVsyncEnd(RectF());
    manager->ReportCapiSwiperStartEvent(node, false);
    manager->OnSwiperChangeCancel(node, false);
    manager->CompleteCapiSwiperChangeWithoutMove(node, false);
    manager->OnSwiperChangeInterrupted(node, false);
    manager->OnContentChangeNodeDestroyed(node->GetId());
    manager->OnVsyncEnd(RectF());
    EXPECT_EQ(a.types, (std::vector<uint32_t> { SWIPER_START }));
    EXPECT_TRUE(b.types.empty());
    EXPECT_TRUE(manager->capiImpl_->capiActiveSwipers_.empty());
    EXPECT_TRUE(manager->capiImpl_->capiPendingSwipers_.empty());
    paint->UpdateAutoPlay(false);
    manager->ReportCapiSwiperStartEvent(node, false);
    manager->OnSwiperChangeEnd(node, false);
    manager->OnVsyncEnd(RectF());
    EXPECT_EQ(a.types, (std::vector<uint32_t> { SWIPER_START, SWIPER_START, SWIPER_END }));
    EXPECT_EQ(b.types, (std::vector<uint32_t> { SWIPER_END }));
    manager->UnregisterCapiContentChangeEvent(aid);
    manager->UnregisterCapiContentChangeEvent(bid);
}

/**
 * @tc.name: CapiSpeculativePageStartDoesNotEscapeToCapi
 * @tc.desc: Failed speculative loads preserve IPC reporting without publishing a CAPI page change.
 * @tc.type: FUNC
 */
HWTEST_F(ContentChangeManagerTestNg, CapiSpeculativePageStartDoesNotEscapeToCapi, TestSize.Level1)
{
    auto manager = GetContentChangeManager();
    manager->InvalidateCapiContentChangeSubscriptions();
    ContentChangeConfig config;
    config.reportStartEvent = true;
    manager->StartContentChangeReport(config);
    CapiTestState state { manager };
    auto id = manager->RegisterCapiContentChangeEvent(true, PAGE_CATEGORY, 0,
        new CapiTestCookie { &state }, CapiContentChangeTestSink, ReleaseCapiContentChangeTestCookie);
    auto node = FrameNode::CreateFrameNode("Page", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    EXPECT_CALL(*GetMockUiSessionManager(), ReportContentChangeEvent(ChangeType::PAGE_START, _)).Times(1);
    manager->OnContentChangeStart(node, ChangeType::PAGE, false);
    EXPECT_TRUE(state.types.empty());
    manager->OnPageTransitionEnd(nullptr, node->GetId());
    EXPECT_TRUE(state.types.empty());
    manager->ReportCapiPageStartEvent(node);
    manager->ReportCapiPageEndEvent(node, node->GetId());
    EXPECT_EQ(state.types, (std::vector<uint32_t> { PAGE_START, PAGE_END }));
    manager->UnregisterCapiContentChangeEvent(id);
    manager->StopContentChangeReport();
    Mock::VerifyAndClearExpectations(GetMockUiSessionManager());
}

} // namespace OHOS::Ace::NG
