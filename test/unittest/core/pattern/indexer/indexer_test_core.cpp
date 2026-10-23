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

#include "indexer_test_min.h"

#define private public
#define protected public
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"
#include "core/components_ng/manager/content_change_manager/content_change_manager.h"
#include "core/components_ng/pattern/indexer/indexer_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#undef private
#undef protected
#include "interfaces/native/ui_info_collection.h"

using namespace testing;
using namespace testing::ext;


namespace OHOS::Ace::NG {

// Test classes for TDD development (quick compile mode)
// These classes use different names from full test suite to avoid conflicts
// When ace_enable_full_test_suite=true, full test files will be compiled instead

class IndexerLayoutAlgorithmTestCore : public IndexerTestBase {};
class IndexerModelNgTestCore : public IndexerTestBase {};
class IndexerPaintPropertyTestCore : public IndexerTestBase {};
class IndexerPatternTestCore : public IndexerTestBase {};

// Placeholder test to ensure compilation
HWTEST_F(IndexerLayoutAlgorithmTestCore, IndexerTestPlaceholder, TestSize.Level1)
{
    EXPECT_TRUE(true);
}

/**
 * @tc.name: CapiBubbleRemoveAndRecreate
 * @tc.desc: Removing a visible bubble closes its own target and permits a newly created bubble to show.
 * @tc.type: FUNC
 */
HWTEST_F(IndexerPatternTestCore, CapiBubbleRemoveAndRecreate, TestSize.Level1)
{
    MockPipelineContext::SetUp();
    auto pipeline = MockPipelineContext::GetCurrent();
    auto manager = pipeline->GetContentChangeManager();
    auto pattern = AceType::MakeRefPtr<IndexerPattern>();
    auto host = FrameNode::CreateFrameNode(V2::INDEXER_ETS_TAG, 8601, pattern);
    pipeline->overlayManager_ = AceType::MakeRefPtr<OverlayManager>(host);
    std::vector<uint32_t> events;
    auto sink = [](uint32_t type, uint64_t, const char*, uint32_t, void* data) {
        static_cast<std::vector<uint32_t>*>(data)->push_back(type);
    };
    auto id = manager->RegisterCapiContentChangeEvent(false, 4, 0, &events, sink, nullptr);
    for (int i = 0; i < 2; ++i) {
        pattern->popupNode_ = FrameNode::CreateFrameNode(
            V2::COLUMN_ETS_TAG, 8610 + i, AceType::MakeRefPtr<LinearLayoutPattern>(true));
        pattern->popupNode_->MountToParent(host);
        pattern->popupNode_->AttachToMainTree(false);
        pattern->StartBubbleAppearAnimation();
        pattern->StartBubbleAppearAnimation();
        EXPECT_EQ(events.size(), static_cast<size_t>(2 * i + 1));
        pattern->RemoveBubble();
        EXPECT_FALSE(pattern->capiBubbleVisible_);
        EXPECT_EQ(events.size(), static_cast<size_t>(2 * i + 2));
    }
    EXPECT_EQ(events, (std::vector<uint32_t> {
        OH_ARKUI_NATIVEMODULE_EVENT_OVERLAY_SHOW, OH_ARKUI_NATIVEMODULE_EVENT_OVERLAY_HIDE,
        OH_ARKUI_NATIVEMODULE_EVENT_OVERLAY_SHOW, OH_ARKUI_NATIVEMODULE_EVENT_OVERLAY_HIDE }));
    manager->UnregisterCapiContentChangeEvent(id);
    host.Reset();
    pattern.Reset();
    MockPipelineContext::TearDown();
}

} // namespace OHOS::Ace::NG
