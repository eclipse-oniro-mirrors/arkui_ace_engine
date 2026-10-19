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
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector_tree_request_context.h"
#include "core/components_ng/pattern/pattern.h"

namespace OHOS::Ace::NG {
class InspectorIdFrameNode : public FrameNode {
    DECLARE_ACE_TYPE(InspectorIdFrameNode, FrameNode);

public:
    InspectorIdFrameNode() : FrameNode("test", 7, AceType::MakeRefPtr<Pattern>()) {}
    void SetInspectorAccessibilityId(int64_t id)
    {
        accessibilityId_ = id;
    }
};

class InspectorTreeTask03 : public testing::Test {
public:
    void SetUp() override
    {
        MockPipelineContext::SetUp();
    }
    void TearDown() override
    {
        MockPipelineContext::TearDown();
    }
};

TEST_F(InspectorTreeTask03, NativeIdsAreLosslessAndGated)
{
    auto frame = AceType::MakeRefPtr<InspectorIdFrameNode>();
    frame->InitializePatternAndContext();
    ParamConfig config;
    config.accessibilityInfo = true;
    InspectorTreeRequestContext context(config, {}, {});
    config.inspectorContext = &context;
    for (int64_t id : { 0LL, 9007199254740991LL, 9007199254740992LL, 9007199254740993LL, 9223372036854775807LL }) {
        frame->SetInspectorAccessibilityId(id);
        auto json = JsonUtil::CreateSharedPtrJson();
        frame->DumpSimplifyTreeNode(json, config);
        EXPECT_TRUE(json->GetValue("accessibilityId")->IsString());
        EXPECT_EQ(json->GetString("accessibilityId"), std::to_string(id));
        EXPECT_EQ(json->GetInt("$ID"), 7);
        EXPECT_TRUE(json->GetValue("accessibilityGroup")->IsBool());
    }
    config.accessibilityInfo = false;
    auto disabled = JsonUtil::CreateSharedPtrJson();
    frame->DumpSimplifyTreeNode(disabled, config);
    EXPECT_FALSE(disabled->Contains("accessibilityId"));
    config.accessibilityInfo = true;
    frame->SetInspectorAccessibilityId(-1);
    auto invalid = JsonUtil::CreateSharedPtrJson();
    frame->DumpSimplifyTreeNode(invalid, config);
    EXPECT_FALSE(invalid->Contains("accessibilityId"));
}

TEST_F(InspectorTreeTask03, FalseInteractionAndAbsentRegions)
{
    auto frame = FrameNode::CreateFrameNode("test", 8, AceType::MakeRefPtr<Pattern>());
    frame->GetOrCreateGestureEventHub()->SetTouchable(false);
    ParamConfig config;
    config.interactionInfo = true;
    config.withResponseRegion = true;
    InspectorTreeRequestContext context(config, {}, {});
    config.inspectorContext = &context;
    auto json = JsonUtil::CreateSharedPtrJson();
    frame->DumpSimplifyTreeNode(json, config);
    EXPECT_TRUE(json->GetValue("touchable")->IsBool());
    EXPECT_FALSE(json->GetBool("touchable"));
    EXPECT_TRUE(json->GetValue("checkable")->IsBool());
    EXPECT_FALSE(json->Contains("checked"));
    EXPECT_FALSE(json->Contains("responseRegion"));
    EXPECT_FALSE(json->Contains("responseRegionMap"));
    EXPECT_FALSE(json->Contains("mouseResponseRegion"));
}
} // namespace OHOS::Ace::NG
