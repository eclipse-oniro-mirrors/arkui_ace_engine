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

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"

#define PRIVATE public
#define PROTECTED public
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/syntax/arkoala_parallelize_ui_adapter_node.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"
#undef PRIVATE
#undef PROTECTED

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {

class ParallelizeUIAdapterNodeTestNg : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        MockPipelineContext::SetUp();
    }

    static void TearDownTestSuite()
    {
        MockPipelineContext::TearDown();
    }
};

/**
 * @tc.name: ParallelizeUIAdapterNode_FrameCount
 * @tc.desc: Test FrameCount before and after RegisterCallback with getCount
 * @tc.type: FUNC
 */
HWTEST_F(ParallelizeUIAdapterNodeTestNg, FrameCountTest001, TestSize.Level1)
{
    auto node = AceType::MakeRefPtr<ParallelizeUIAdapterNode>(100);
    // default no getCount_ set -> should be 0
    EXPECT_EQ(node->FrameCount(), 0);

    // register getCount only (registerCallback returns nullptr)
    node->RegisterCallback(
        std::function<int32_t()>([]() { return 3; }),
        std::function<ArkUINodeHandle(int32_t, int32_t, int32_t)>(
            [](int32_t, int32_t, int32_t) -> ArkUINodeHandle { return nullptr; })
    );

    EXPECT_EQ(node->FrameCount(), 3);
}

/**
 * @tc.name: ParallelizeUIAdapterNode_GetFrameChild
 * @tc.desc: Register callbacks to return a FrameNode and verify GetFrameChildByIndex and GetChildren
 * @tc.type: FUNC
 */
HWTEST_F(ParallelizeUIAdapterNodeTestNg, GetFrameChildByIndexTest002, TestSize.Level1)
{
    // create a child FrameNode that will be returned by the registerCallback
    auto pattern = AceType::MakeRefPtr<Pattern>();
    auto childFrameNode = AceType::MakeRefPtr<FrameNode>("child", -1, pattern);
    pattern->AttachToFrameNode(childFrameNode);

    auto node = AceType::MakeRefPtr<ParallelizeUIAdapterNode>(101);

    // register callbacks: getCount -> 1, registerCallback -> return the raw pointer of childFrameNode
    node->RegisterCallback(std::function<int32_t()>([]() { return 1; }),
        std::function<ArkUINodeHandle(int32_t, int32_t, int32_t)>(
            [child = childFrameNode](int32_t /*index*/, int32_t /*needBuild*/, int32_t /*isCache*/) -> ArkUINodeHandle {
                return reinterpret_cast<ArkUINodeHandle>(Referenced::RawPtr(child));
            }));

    // request frame child and add to render tree
    auto frameChild = AceType::DynamicCast<FrameNode>(node->GetFrameChildByIndex(0, true, false, true));
    ASSERT_NE(frameChild, nullptr);

    // after adding, children list should contain one item
    const auto& children = node->GetChildren();
    EXPECT_EQ(children.size(), 1u);
    node->Reset();
    auto frameChild2 = AceType::DynamicCast<FrameNode>(node->GetFrameChildByIndex(0, true, false, true));
    ASSERT_NE(frameChild2, nullptr);
    EXPECT_EQ(node->GetChildren().size(), 1u);
}

/**
 * @tc.name: ParallelizeUIAdapterNode_ExceptionBranch
 * @tc.desc: registerCallback_ nullptr and return nullptr branch test
 * @tc.type: FUNC
 */
HWTEST_F(ParallelizeUIAdapterNodeTestNg, ExceptionBranchTest003, TestSize.Level1)
{
    auto node = AceType::MakeRefPtr<ParallelizeUIAdapterNode>(102);
    auto result = node->GetFrameChildByIndex(0, true, false, true);
    EXPECT_EQ(result, nullptr);
    node->RegisterCallback(std::function<int32_t()>([]() { return 1; }),
        std::function<ArkUINodeHandle(int32_t, int32_t, int32_t)>([](int32_t, int32_t, int32_t) -> ArkUINodeHandle {
            return nullptr;
        }));
    auto result2 = node->GetFrameChildByIndex(0, true, false, true);
    EXPECT_EQ(result2, nullptr);
    EXPECT_EQ(node->GetChildren().size(), 0u);
}
} // namespace OHOS::Ace::NG
