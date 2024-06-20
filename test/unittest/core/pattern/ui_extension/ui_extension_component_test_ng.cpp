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

#include "gtest/gtest.h"
#include "base/memory/ace_type.h"
#define private public
#define protected public
#include "core/components_ng/pattern/ui_extension/ui_extension_pattern.h"


using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
    const std::string UI_EXTENSION_COMPONENT_ETS_TAG = "UIExtensionComponent";
    const int32_t CODE = 10001;
    const std::string NAME = "Test name";
    const std::string MSG = "Test msg";
} // namespace

class UIExtensionComponentTextNg : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
};

void UIExtensionComponentTextNg::SetUp() {}

void UIExtensionComponentTextNg::TearDown() {}

/**
 * @tc.name: UIExtensionComponentTest001
 * @tc.desc: Test UIExtensionPattern SetPlaceholderNode/MountPlaceholderNode/RemovePlaceholderNode
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionComponentTextNg, UIExtensionComponentTest001, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    /**
     * @tc.steps: step1. construct a UIExtensionComponent Node
     */
    auto uiExtensionNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto uiExtensionNode = FrameNode::GetOrCreateFrameNode(
        UI_EXTENSION_COMPONENT_ETS_TAG, uiExtensionNodeId, []() { return AceType::MakeRefPtr<UIExtensionPattern>(); });
    ASSERT_NE(uiExtensionNode, nullptr);
    EXPECT_EQ(uiExtensionNode->GetTag(), V2::UI_EXTENSION_COMPONENT_ETS_TAG);

    /**
     * @tc.steps: step2. get UIExtensionPattern
     */
    auto uiExtensionPattern = uiExtensionNode->GetPattern<UIExtensionPattern>();
    ASSERT_NE(uiExtensionPattern, nullptr);
    EXPECT_EQ(uiExtensionPattern->placeholderNode_, nullptr);
    EXPECT_EQ(uiExtensionPattern->IsShowPlaceholder(), false);
    uiExtensionPattern->AttachToFrameNode(uiExtensionNode);
    uiExtensionPattern->OnModifyDone();

    /**
     * @tc.steps: step3. SetPlaceholderNode
     */
    auto placeholderNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto placeholderNode = FrameNode::GetOrCreateFrameNode(
        "placeholderNode", placeholderNodeId, []() { return AceType::MakeRefPtr<Pattern>(); });
    uiExtensionPattern->SetPlaceholderNode(placeholderNode);
    ASSERT_NE(uiExtensionPattern->placeholderNode_, nullptr);

    /**
     * @tc.steps: step4. MountPlaceholderNode
     */
    uiExtensionPattern->MountPlaceholderNode();
    EXPECT_EQ(uiExtensionPattern->IsShowPlaceholder(), true);

    /**
     * @tc.steps: step5. RemovePlaceholderNode
     */
    uiExtensionPattern->RemovePlaceholderNode();
    EXPECT_EQ(uiExtensionPattern->IsShowPlaceholder(), false);
#endif
}

/**
 * @tc.name: UIExtensionComponentTest002
 * @tc.desc: Test UIExtensionPattern SetDensityDpi function
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionComponentTextNg, UIExtensionComponentTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct a UIExtensionComponent Node
     */
    auto uiExtensionNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto uiExtensionNode = FrameNode::GetOrCreateFrameNode(
        UI_EXTENSION_COMPONENT_ETS_TAG, uiExtensionNodeId, []() { return AceType::MakeRefPtr<UIExtensionPattern>(); });
    ASSERT_NE(uiExtensionNode, nullptr);
    EXPECT_EQ(uiExtensionNode->GetTag(), V2::UI_EXTENSION_COMPONENT_ETS_TAG);

    /**
     * @tc.steps: step2. get UIExtensionPattern
     */
    auto uiExtensionPattern = uiExtensionNode->GetPattern<UIExtensionPattern>();
    ASSERT_NE(uiExtensionPattern, nullptr);
    bool densityDpi = uiExtensionPattern->GetDensityDpi();
    EXPECT_FALSE(densityDpi);

    /**
     * @tc.steps: step3. SetDensityDpi false
     */
    uiExtensionPattern->SetDensityDpi(false);
    densityDpi = uiExtensionPattern->GetDensityDpi();
    EXPECT_FALSE(densityDpi);

    /**
     * @tc.steps: step4. SetDensityDpi true
     */
    uiExtensionPattern->SetDensityDpi(true);
    densityDpi = uiExtensionPattern->GetDensityDpi();
    EXPECT_TRUE(densityDpi);
}

/**
 * @tc.name: UIExtensionComponentTest003
 * @tc.desc: Test UIExtensionPattern OnTerminated
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionComponentTextNg, UIExtensionComponentTest003, TestSize.Level1)
{
#ifdef OHOS_STANDARD_SYSTEM
    /**
     * @tc.steps: step1. construct a UIExtensionComponent Node
     */
    auto uiExtensionNodeId = ElementRegister::GetInstance()->MakeUniqueId();
    auto uiExtensionNode = FrameNode::GetOrCreateFrameNode(
        UI_EXTENSION_COMPONENT_ETS_TAG, uiExtensionNodeId, []() { return AceType::MakeRefPtr<UIExtensionPattern>(); });
    ASSERT_NE(uiExtensionNode, nullptr);
    EXPECT_EQ(uiExtensionNode->GetTag(), V2::UI_EXTENSION_COMPONENT_ETS_TAG);
    /**
     * @tc.steps: step2. Get uiExtensionPattern
     */
    auto uiExtensionPattern = uiExtensionNode->GetPattern<UIExtensionPattern>();
    ASSERT_NE(uiExtensionPattern, nullptr);
    /**
     * @tc.steps: step3. Test not compatible oldVersion and OnTerminated
     */
    auto onTerminatedFuntion =
        [](int32_t code, const RefPtr<WantWrap>& wantWrap) { EXPECT_EQ(code, CODE); };
    uiExtensionPattern->SetOnTerminatedCallback(onTerminatedFuntion);
    uiExtensionPattern->FireOnTerminatedCallback(CODE, nullptr);
    EXPECT_FALSE(uiExtensionPattern->IsCompatibleOldVersion());
    /**
     * @tc.steps: step3. Test compatible oldVersion
     */
    uiExtensionPattern->SetOnTerminatedCallback(nullptr);
    uiExtensionPattern->FireOnTerminatedCallback(CODE, nullptr);
    EXPECT_TRUE(uiExtensionPattern->IsCompatibleOldVersion());
    /**
     * @tc.steps: step4. Test onError
     */
    auto onError =
        [](int32_t code, const std::string& name, const std::string& message) {
            EXPECT_EQ(code, CODE);
            EXPECT_EQ(name, NAME);
            EXPECT_EQ(message, MSG);
        };
    uiExtensionPattern->SetOnErrorCallback(std::move(onError));
    uiExtensionPattern->FireOnErrorCallback(CODE, NAME, MSG);
#endif
}
} // namespace OHOS::Ace::NG
