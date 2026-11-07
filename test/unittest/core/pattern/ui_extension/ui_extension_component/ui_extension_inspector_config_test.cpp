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
#include "test/mock/frameworks/base/thread/mock_task_executor.h"

#include "base/memory/ace_type.h"
#define private public
#define protected public
#include "test/mock/frameworks/core/common/mock_container.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

#include "core/components_ng/pattern/ui_extension/ui_extension_component/session_wrapper_impl.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_component/ui_extension_node.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_component/ui_extension_pattern.h"
#include "core/components_v2/inspector/inspector_constants.h"
#undef private
#undef protected

namespace OHOS::Ace::NG {
namespace {
class RecordingInspectorSession : public SessionWrapperImpl {
    DECLARE_ACE_TYPE(RecordingInspectorSession, SessionWrapperImpl);

public:
    explicit RecordingInspectorSession(const WeakPtr<UIExtensionPattern>& pattern)
        : SessionWrapperImpl(pattern, 0, false, SessionType::UI_EXTENSION_ABILITY)
    {}

    void NotifyUieDump(const std::vector<std::string>& params, std::vector<std::string>& info) override
    {
        capturedParams = params;
        ++calls;
    }

    size_t calls = 0;
    std::vector<std::string> capturedParams;
};
} // namespace

class InspectorTreeTask01UIExtension : public testing::Test {
protected:
    void SetUp() override
    {
        MockPipelineContext::SetUp();
        MockContainer::SetUp();
        MockContainer::Current()->taskExecutor_ = AceType::MakeRefPtr<MockTaskExecutor>();
        MockContainer::Current()->pipelineContext_ = MockPipelineContext::GetCurrentContext();
        MockPipelineContext::GetCurrentContext()->taskExecutor_ = MockContainer::Current()->taskExecutor_;
    }

    void TearDown() override
    {
        MockPipelineContext::TearDown();
        MockContainer::TearDown();
    }
};

TEST_F(InspectorTreeTask01UIExtension, ForwardIndependentCapabilitiesInWireOrder)
{
    constexpr size_t INTERACTION_INFO_INDEX = 2;
    constexpr size_t ACCESSIBILITY_INFO_INDEX = 3;
    constexpr size_t CACHE_NODES_INDEX = 4;
    constexpr size_t WITH_WEB_INDEX = 5;
    constexpr size_t WITH_UI_EXTENSION_INDEX = 6;
    constexpr size_t RECT_CULLING_INDEX = 7;
    constexpr size_t MIN_OPACITY_INDEX = 8;
    constexpr size_t WITH_RESPONSE_REGION_INDEX = 9;
    constexpr size_t WITH_CUSTOM_CONTENT_INDEX = 10;
    constexpr size_t PARAM_COUNT = 11;
    auto node = UIExtensionNode::GetOrCreateUIExtensionNode(
        V2::UI_EXTENSION_COMPONENT_ETS_TAG, 100, []() { return AceType::MakeRefPtr<UIExtensionPattern>(); });
    auto pattern = node->GetPattern<UIExtensionPattern>();
    auto session = AceType::MakeRefPtr<RecordingInspectorSession>(pattern);
    pattern->sessionWrapper_ = session;
    pattern->instanceId_ = 0;
    for (bool responseRegion : { false, true }) {
        for (bool xComponent : { false, true }) {
            ParamConfig config { true, false, true, false, true, true, 0.25f };
            config.withResponseRegion = responseRegion;
            config.withCustomContent = xComponent;
            auto json = JsonUtil::CreateSharedPtrJson();
            const auto previousCalls = session->calls;
            pattern->AddExtraInfoWithParamConfig(json, config);
            ASSERT_EQ(session->calls, previousCalls + 1);
            ASSERT_GE(session->capturedParams.size(), PARAM_COUNT);
            EXPECT_EQ(session->capturedParams[0], "-allInfoWithParamConfigTotal");
            EXPECT_EQ(session->capturedParams[INTERACTION_INFO_INDEX], "1");
            EXPECT_EQ(session->capturedParams[ACCESSIBILITY_INFO_INDEX], "0");
            EXPECT_EQ(session->capturedParams[CACHE_NODES_INDEX], "1");
            EXPECT_EQ(session->capturedParams[WITH_WEB_INDEX], "0");
            EXPECT_EQ(session->capturedParams[WITH_UI_EXTENSION_INDEX], "1");
            EXPECT_EQ(session->capturedParams[RECT_CULLING_INDEX], "1");
            EXPECT_EQ(session->capturedParams[MIN_OPACITY_INDEX], std::to_string(0.25f));
            EXPECT_EQ(session->capturedParams[WITH_RESPONSE_REGION_INDEX], responseRegion ? "1" : "0");
            EXPECT_EQ(session->capturedParams[WITH_CUSTOM_CONTENT_INDEX], xComponent ? "1" : "0");
        }
    }
}

TEST_F(InspectorTreeTask01UIExtension, DisabledUIExtensionDoesNotQuerySession)
{
    auto node = UIExtensionNode::GetOrCreateUIExtensionNode(
        V2::UI_EXTENSION_COMPONENT_ETS_TAG, 101, []() { return AceType::MakeRefPtr<UIExtensionPattern>(); });
    auto pattern = node->GetPattern<UIExtensionPattern>();
    auto session = AceType::MakeRefPtr<RecordingInspectorSession>(pattern);
    pattern->sessionWrapper_ = session;
    ParamConfig config;
    config.withResponseRegion = true;
    config.withCustomContent = true;
    auto json = JsonUtil::CreateSharedPtrJson();
    pattern->AddExtraInfoWithParamConfig(json, config);
    EXPECT_EQ(session->calls, 0u);
}
} // namespace OHOS::Ace::NG
