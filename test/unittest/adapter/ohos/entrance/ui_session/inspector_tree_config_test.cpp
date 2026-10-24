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
#ifndef ACE_INSPECTOR_TREE_REAL_PARCEL_TEST
#include "accesstoken_kit.h"
#include "interfaces/inner_api/ui_session/ui_content_stub_impl.h"
#endif

namespace OHOS::Ace {
class InspectorTreeTask01 : public testing::Test {};

TEST_F(InspectorTreeTask01, DefaultAndLegacyConstructorDisableNewCapabilities)
{
    GetInspectorTreeConfigImpl defaults;
    EXPECT_FALSE(defaults.GetConfig().withResponseRegion);
    EXPECT_FALSE(defaults.GetConfig().withCustomContent);
    EXPECT_TRUE(defaults.GetConfig().cacheNodes);
    GetInspectorTreeConfigImpl legacy(true, true, false, true, true, true, 0.5f);
    EXPECT_FALSE(legacy.GetConfig().withResponseRegion);
    EXPECT_FALSE(legacy.GetConfig().withCustomContent);
    EXPECT_FLOAT_EQ(legacy.GetConfig().minOpacity, 0.5f);
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

#ifndef ACE_INSPECTOR_TREE_REAL_PARCEL_TEST
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
#endif

// The legacy IPC fixture's Parcel mock discards values. These wire-format
// tests are also compiled separately against the real c_utils Parcel; running
// only the mock fixture cannot establish serialization or truncation coverage.
#ifdef ACE_INSPECTOR_TREE_REAL_PARCEL_TEST
TEST_F(InspectorTreeTask01, RealParcelRoundTripAllCombinations)
{
    for (bool responseRegion : { false, true }) {
        for (bool xComponent : { false, true }) {
            ParamConfig config { true, true, false, true, true, true, 0.75f };
            config.withResponseRegion = responseRegion;
            config.withCustomContent = xComponent;
            GetInspectorTreeConfigImpl original(config);
            Parcel parcel;
            ASSERT_TRUE(original.Marshalling(parcel));
            std::unique_ptr<GetInspectorTreeConfigImpl> decoded(GetInspectorTreeConfigImpl::Unmarshalling(parcel));
            ASSERT_NE(decoded, nullptr);
            const auto actual = decoded->GetConfig();
            EXPECT_TRUE(actual.interactionInfo);
            EXPECT_TRUE(actual.accessibilityInfo);
            EXPECT_FALSE(actual.cacheNodes);
            EXPECT_TRUE(actual.withWeb);
            EXPECT_TRUE(actual.withUIExtension);
            EXPECT_TRUE(actual.rectCulling);
            EXPECT_FLOAT_EQ(actual.minOpacity, 0.75f);
            EXPECT_EQ(actual.withResponseRegion, responseRegion);
            EXPECT_EQ(actual.withCustomContent, xComponent);
        }
    }
}

TEST_F(InspectorTreeTask01, RealParcelRejectsEveryTruncatedField)
{
    for (size_t fields = 0; fields < 9; ++fields) {
        Parcel parcel;
        for (size_t i = 0; i < fields; ++i) {
            ASSERT_TRUE(i == 6 ? parcel.WriteFloat(0.5f) : parcel.WriteBool(true));
        }
        std::unique_ptr<GetInspectorTreeConfigImpl> decoded(GetInspectorTreeConfigImpl::Unmarshalling(parcel));
        EXPECT_EQ(decoded, nullptr) << "fields=" << fields;
    }
}

TEST_F(InspectorTreeTask01, RealParcelKeepsOriginalFieldOrder)
{
    GetInspectorTreeConfigImpl original(true, false, true, false, true, false, 0.25f, true, false);
    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));
    for (bool expected : { true, false, true, false, true, false }) {
        bool actual = false;
        ASSERT_TRUE(parcel.ReadBool(actual));
        EXPECT_EQ(actual, expected);
    }
    float opacity = 0;
    ASSERT_TRUE(parcel.ReadFloat(opacity));
    EXPECT_FLOAT_EQ(opacity, 0.25f);
    bool region = false;
    bool xComponent = true;
    ASSERT_TRUE(parcel.ReadBool(region));
    ASSERT_TRUE(parcel.ReadBool(xComponent));
    EXPECT_TRUE(region);
    EXPECT_FALSE(xComponent);
}
#endif
} // namespace OHOS::Ace
