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

#include <memory>

#include "gtest/gtest.h"

#include "adapter/ohos/entrance/ui_session/get_inspector_tree_config_impl.h"
#include "accesstoken_kit.h"
#include "interfaces/inner_api/ui_session/ui_content_stub_impl.h"

namespace OHOS::Ace {
class InspectorTreeTask01 : public testing::Test {};

TEST_F(InspectorTreeTask01, DefaultAndLegacyConstructorDisableNewCapabilities)
{
    GetInspectorTreeConfigImpl defaults;
    EXPECT_FALSE(defaults.GetConfig().withResponseRegion);
    EXPECT_FALSE(defaults.GetConfig().withCustomContent);
    EXPECT_TRUE(defaults.GetConfig().cacheNodes);
    EXPECT_EQ(defaults.GetConfig().inputCheckInterval, UINT32_MAX);
    GetInspectorTreeConfigImpl legacy(true, true, false, true, true, true, 0.5f);
    EXPECT_FALSE(legacy.GetConfig().withResponseRegion);
    EXPECT_FALSE(legacy.GetConfig().withCustomContent);
    EXPECT_FLOAT_EQ(legacy.GetConfig().minOpacity, 0.5f);
    EXPECT_EQ(legacy.GetConfig().inputCheckInterval, UINT32_MAX);
}

TEST_F(InspectorTreeTask01, CopyPreservesIndependentCapabilitiesAndOriginalFields)
{
    for (bool responseRegion : { false, true }) {
        for (bool xComponent : { false, true }) {
            ParamConfig config { true, true, false, true, true, true, 0.5f };
            config.withResponseRegion = responseRegion;
            config.withCustomContent = xComponent;
            GetInspectorTreeConfigImpl value(config);
            const auto actual = value.GetConfig();
            EXPECT_TRUE(actual.interactionInfo);
            EXPECT_TRUE(actual.accessibilityInfo);
            EXPECT_FALSE(actual.cacheNodes);
            EXPECT_TRUE(actual.withWeb);
            EXPECT_TRUE(actual.withUIExtension);
            EXPECT_TRUE(actual.rectCulling);
            EXPECT_FLOAT_EQ(actual.minOpacity, 0.5f);
            EXPECT_EQ(actual.withResponseRegion, responseRegion);
            EXPECT_EQ(actual.withCustomContent, xComponent);
        }
    }
}

TEST_F(InspectorTreeTask01, BothEntrypointsRejectInvalidAccessAndInterfaceTokens)
{
    using namespace Security::AccessToken;
    const auto previous = AccessTokenKit::mockTokenTypeFlag_;
    auto service = std::make_shared<UIContentServiceStubImpl>();
    for (auto code : { IUiContentService::UI_CONTENT_SERVICE_GET_TREE, IUiContentService::GET_VISIBLE_TREE }) {
        for (auto token : { TOKEN_HAP, TOKEN_SHELL, TOKEN_INVALID }) {
            AccessTokenKit::mockTokenTypeFlag_ = token;
            MessageParcel data;
            MessageParcel reply;
            MessageOption option;
            data.WriteInterfaceToken(u"ohos.ace.UIContentService");
            EXPECT_EQ(service->OnRemoteRequest(code, data, reply, option), -1);
        }
        AccessTokenKit::mockTokenTypeFlag_ = TOKEN_NATIVE;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        data.WriteInterfaceToken(u"invalid.interface");
        EXPECT_EQ(service->OnRemoteRequest(code, data, reply, option), -1);
    }
    AccessTokenKit::mockTokenTypeFlag_ = previous;
}

} // namespace OHOS::Ace
