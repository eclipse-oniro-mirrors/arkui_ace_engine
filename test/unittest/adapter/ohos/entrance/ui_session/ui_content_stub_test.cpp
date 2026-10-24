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

#include <atomic>

#define private public
#define protected public

#include "interfaces/inner_api/ui_session/ui_content_stub_impl.h"
#include "interfaces/inner_api/ui_session/ui_content_proxy.h"
#include "interfaces/inner_api/ui_session/ui_report_stub.h"
#include "interfaces/inner_api/ui_session/ui_session_json_util.h"
#include "interfaces/inner_api/ui_session/ui_session_manager.h"
#include "interfaces/inner_api/ui_session/component_tree_query_type.h"
#include "interfaces/inner_api/ui_session/ui_session_request_guard.h"
#include "adapter/ohos/entrance/ui_session/ui_translate_request_util.h"
#include "test/mock/interfaces/inner_api/ui_session/mock_ui_session_manager.h"

#include "accesstoken_kit.h"

#include <functional>
#include <memory>
#include <utility>
#include <vector>

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t INVALID_EMPTY_SCOPE = 0;
constexpr int32_t INVALID_UNKNOWN_SCOPE = 1 << 4;
constexpr int32_t ARKWEB_SCOPE = static_cast<int32_t>(TranslateContentScope::ARKWEB_ONLY);
constexpr int32_t ARKUI_SCOPE = static_cast<int32_t>(TranslateContentScope::ARKUI_ONLY);
constexpr int32_t ARKUI_ARKWEB_SCOPE = static_cast<int32_t>(TranslateContentScope::ARKUI_ARKWEB);
constexpr int32_t RESERVED_XCOMPONENT_SCOPE = static_cast<int32_t>(TranslateContentScope::XCOMPONENT);
constexpr int32_t RESERVED_CANVAS_SCOPE = static_cast<int32_t>(TranslateContentScope::CANVAS_NODE);
constexpr int32_t PAGE_ALL_SCOPE = static_cast<int32_t>(TranslateContentScope::PAGE_ALL);
constexpr char EMPTY_TRANSLATE_REQUEST[] = "";
constexpr char LEGACY_TRANSLATE_EXTRA_DATA[] = "legacy extra data";
constexpr char VALID_NON_OBJECT_TRANSLATE_REQUEST[] = R"([{"scope":3}])";
constexpr char VALID_NO_SCOPE_TRANSLATE_REQUEST[] = R"({"extraData":"test"})";
constexpr char VALID_NULL_SCOPE_TRANSLATE_REQUEST[] = R"({"scope":null})";
constexpr char VALID_TRANSLATE_SCOPE_REQUEST[] = R"({"scope":3})";
constexpr char VALID_TRANSLATE_EXTRA_ONLY_REQUEST[] = R"({"extraData":"test"})";
constexpr char INVALID_ZERO_TRANSLATE_SCOPE_REQUEST[] = R"({"scope":0})";
constexpr char INVALID_NEGATIVE_TRANSLATE_SCOPE_REQUEST[] = R"({"scope":-1})";
constexpr char INVALID_UNKNOWN_TRANSLATE_SCOPE_REQUEST[] = R"({"scope":16})";
constexpr char INVALID_STRING_TRANSLATE_SCOPE_REQUEST[] = R"({"scope":"3"})";
constexpr int32_t TRANSLATE_TITLE_NODE_ID = 6;
constexpr int32_t TRANSLATE_PLACEHOLDER_NODE_ID = 13;
constexpr int32_t TRANSLATE_BUTTON_NODE_ID = 16;
constexpr int64_t TRANSLATE_SNAPSHOT_VERSION = 0;
constexpr int64_t TRANSLATE_FIRST_VERSION = 1;
constexpr int64_t TRANSLATE_SECOND_VERSION = 2;
constexpr int64_t TRANSLATE_THIRD_VERSION = 3;
constexpr int64_t TRANSLATE_LARGE_VERSION = 3000000000;

struct TranslateScopeTestData {
    int32_t scope;
    bool expected;
};

struct TranslateRequestTestData {
    const char* request;
    bool expected;
};

struct TranslateTextDecodeTestData {
    const char* text;
    const char* expected;
};

struct TranslateResultParseTestData {
    const char* result;
    bool expected;
    size_t expectedSize;
};

struct TranslateResultIdentityTestData {
    const char* result;
    std::vector<std::pair<int32_t, int64_t>> expected;
};

void PrepareValidUiSessionParcel(MessageParcel& data)
{
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
}
} // namespace

class UIContentStubTest : public testing::Test {};

/**
 * @tc.name: UIContentStubTest001
 * @tc.desc: Test UI_CONTENT_SERVICE_GET_TREE
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::UI_CONTENT_SERVICE_GET_TREE, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest002
 * @tc.desc: Test UI_CONTENT_CONNECT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::UI_CONTENT_CONNECT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest003
 * @tc.desc: Test REGISTER_CLICK_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::REGISTER_CLICK_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest004
 * @tc.desc: Test REGISTER_SEARCH_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::REGISTER_SEARCH_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest005
 * @tc.desc: Test REGISTER_TEXT_CHANGE_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::REGISTER_TEXT_CHANGE_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest006
 * @tc.desc: Test REGISTER_ROUTER_CHANGE_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::REGISTER_ROUTER_CHANGE_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest007
 * @tc.desc: Test REGISTER_COMPONENT_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::REGISTER_COMPONENT_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest008
 * @tc.desc: Test REGISTER_WEB_UNFOCUS_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::REGISTER_WEB_UNFOCUS_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest009
 * @tc.desc: Test REGISTER_SCROLL_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::REGISTER_SCROLL_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest010
 * @tc.desc: Test REGISTER_LIFE_CYCLE_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::REGISTER_LIFE_CYCLE_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest011
 * @tc.desc: Test REGISTER_SELECT_TEXT_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::REGISTER_SELECT_TEXT_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest012
 * @tc.desc: Test SENDCOMMAND_SYNC_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::SENDCOMMAND_SYNC_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest013
 * @tc.desc: Test SENDCOMMAND_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest013, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::SENDCOMMAND_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest014
 * @tc.desc: Test UNREGISTER_CLICK_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest014, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::UNREGISTER_CLICK_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest015
 * @tc.desc: Test UNREGISTER_SEARCH_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest015, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::UNREGISTER_SEARCH_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest016
 * @tc.desc: Test UNREGISTER_TEXT_CHANGE_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest016, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::UNREGISTER_TEXT_CHANGE_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest017
 * @tc.desc: Test UNREGISTER_ROUTER_CHANGE_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest017, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::UNREGISTER_ROUTER_CHANGE_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest018
 * @tc.desc: Test UNREGISTER_COMPONENT_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest018, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::UNREGISTER_COMPONENT_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest019
 * @tc.desc: Test UNREGISTER_WEB_UNFOCUS_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest019, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::UNREGISTER_WEB_UNFOCUS_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest020
 * @tc.desc: Test UNREGISTER_SCROLL_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest020, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::UNREGISTER_SCROLL_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest021
 * @tc.desc: Test UNREGISTER_LIFE_CYCLE_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest021, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::UNREGISTER_LIFE_CYCLE_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest022
 * @tc.desc: Test UNREGISTER_SELECT_TEXT_EVENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest022, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::UNREGISTER_SELECT_TEXT_EVENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest023
 * @tc.desc: Test RESET_ALL_TEXT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest023, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::RESET_ALL_TEXT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest024
 * @tc.desc: Test RESET_TEXT_BY_ID
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest024, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::RESET_TEXT_BY_ID, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest025
 * @tc.desc: Test GET_WEB_VIEW_LANGUAGE
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest025, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::GET_WEB_VIEW_LANGUAGE, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest026
 * @tc.desc: Test GET_WEB_TRANSLATE_TEXT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest026, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::GET_WEB_TRANSLATE_TEXT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest027
 * @tc.desc: Test CONTINUE_GET_WEB_TEXT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest027, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::CONTINUE_GET_WEB_TEXT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest028
 * @tc.desc: Test SEND_TRANSLATE_RESULT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest028, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::SEND_TRANSLATE_RESULT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest029
 * @tc.desc: Test END_WEB_TRANSLATE
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest029, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::END_WEB_TRANSLATE, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest030
 * @tc.desc: Test SEND_TRANSLATE_RESULT_STR
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest030, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::SEND_TRANSLATE_RESULT_STR, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest031
 * @tc.desc: Test GET_CURRENT_PAGE_NAME
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest031, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::GET_CURRENT_PAGE_NAME, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest032
 * @tc.desc: Test GET_CURRENT_SHOWING_IMAGE
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest032, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::GET_CURRENT_SHOWING_IMAGE, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest033
 * @tc.desc: Test GET_VISIBLE_TREE
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest033, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::GET_VISIBLE_TREE, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest034
 * @tc.desc: Test SEND_COMMAND
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest034, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::SEND_COMMAND, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest035
 * @tc.desc: Test EXE_APP_AI_FUNCTION
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest035, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::EXE_APP_AI_FUNCTION, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest036
 * @tc.desc: Test GET_SPECIFIED_CONTENT_OFFSETS
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest036, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::GET_SPECIFIED_CONTENT_OFFSETS, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest037
 * @tc.desc: Test HIGHLIGHT_SPECIFIED_CONTENT
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest037, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::HIGHLIGHT_SPECIFIED_CONTENT, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest038
 * @tc.desc: Test GET_MULTI_IMAGES_BY_ID
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest038, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::GET_MULTI_IMAGES_BY_ID, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest039
 * @tc.desc: Test REGISTER_CONTENT_CHANGE
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest039, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::REGISTER_CONTENT_CHANGE, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest040
 * @tc.desc: Test UNREGISTER_CONTENT_CHANGE
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest040, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::UNREGISTER_CONTENT_CHANGE, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest041
 * @tc.desc: Test GET_HIT_TEST_NODE_INFO_FOR_TOUCH
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest041, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::GET_HIT_TEST_NODE_INFO_FOR_TOUCH, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest042
 * @tc.desc: Test REQUEST_STATE_MGMT_INFO
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest042, TestSize.Level1)
{
    /**
     * @tc.steps: step1. creat UIContentServiceStubImpl and creat MessageParcel add MessageOption
     * @tc.expected: step1. call UIContentServiceStubImpl OnRemoteRequest return -1 because of AccessToken check failed.
     */
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    /**
     * @tc.steps: step2. simulate AccessToken check success.
     * @tc.expected: step2. call UIContentServiceStubImpl OnRemoteRequest return success.
     */
    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    res = ptr->OnRemoteRequest(IUiContentService::REQUEST_STATE_MGMT_INFO, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: UIContentStubTest043
 * @tc.desc: Test translate content scope bitmask validation.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest043, TestSize.Level1)
{
    const TranslateScopeTestData testData[] = {
        { ARKWEB_SCOPE, true },
        { ARKUI_SCOPE, true },
        { ARKUI_ARKWEB_SCOPE, true },
        { RESERVED_XCOMPONENT_SCOPE, true },
        { RESERVED_CANVAS_SCOPE, true },
        { PAGE_ALL_SCOPE, true },
        { INVALID_EMPTY_SCOPE, false },
        { INVALID_UNKNOWN_SCOPE, false },
    };

    for (const auto& item : testData) {
        EXPECT_EQ(PageTranslateRequestUtil::IsTranslateScopeValid(item.scope), item.expected);
    }
}

/**
 * @tc.name: UIContentStubTest044
 * @tc.desc: Test page translate request validation.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest044, TestSize.Level1)
{
    const TranslateRequestTestData testData[] = {
        { EMPTY_TRANSLATE_REQUEST, true },
        { LEGACY_TRANSLATE_EXTRA_DATA, true },
        { VALID_NON_OBJECT_TRANSLATE_REQUEST, true },
        { VALID_NO_SCOPE_TRANSLATE_REQUEST, true },
        { VALID_NULL_SCOPE_TRANSLATE_REQUEST, true },
        { VALID_TRANSLATE_SCOPE_REQUEST, true },
        { VALID_TRANSLATE_EXTRA_ONLY_REQUEST, true },
        { INVALID_ZERO_TRANSLATE_SCOPE_REQUEST, false },
        { INVALID_NEGATIVE_TRANSLATE_SCOPE_REQUEST, false },
        { INVALID_UNKNOWN_TRANSLATE_SCOPE_REQUEST, false },
        { INVALID_STRING_TRANSLATE_SCOPE_REQUEST, false },
    };

    for (const auto& item : testData) {
        EXPECT_EQ(PageTranslateRequestUtil::IsPageTranslateRequestValid(item.request), item.expected);
    }
}

/**
 * @tc.name: UIContentStubTest045
 * @tc.desc: Test page translate result text decodes unicode escape literals only.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest045, TestSize.Level1)
{
    const TranslateTextDecodeTestData testData[] = {
        { u8"详情中文", u8"详情中文" },
        { R"(\u8be6\u60c5\u4e2d\u6587)", u8"详情中文" },
        { R"(prefix-\u8be6\u60c5-suffix)", u8"prefix-详情-suffix" },
        { R"(\ud83d\ude00)", u8"😀" },
        { R"(\u8bg6\x60c5)", R"(\u8bg6\x60c5)" },
        { R"(\ud83d)", R"(\ud83d)" },
        { R"(C:\temp\file)", R"(C:\temp\file)" },
    };

    for (const auto& item : testData) {
        EXPECT_EQ(PageTranslateRequestUtil::DecodeJsonUnicodeEscapes(item.text), item.expected);
    }
}

/**
 * @tc.name: UIContentStubTest046
 * @tc.desc: Test page translate result parser supports single and batch results.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest046, TestSize.Level1)
{
    const TranslateResultParseTestData testData[] = {
        { R"({"nodeId":6, "version":0, "translatedText":"\u6807\u9898"})", true, 1 },
        { R"({"nodeId":7, "version":2, "text":"详情中文"})", true, 1 },
        { R"({"results":[{"nodeId":6, "version":1, "translatedText":"\u6807\u9898"},)"
          R"({"nodeId":13, "version":1, "text":"输入框提示"}]})",
            true, 2 },
        { R"([{"nodeId":6, "version":3, "translatedText":"标题"}, {"nodeId":7, "version":3, "text":"详情"}])", true, 2 },
        { R"({"results":[]})", false, 0 },
        { R"({"results":[1, false]})", false, 0 },
        { R"({"version":1, "translatedText":"标题"})", false, 0 },
        { R"({"nodeId":6, "translatedText":"标题"})", false, 0 },
        { R"({"nodeId":6, "version":1, "translatedText":""})", false, 0 },
        { R"("invalid")", false, 0 },
    };

    for (const auto& item : testData) {
        std::vector<TranslateResult> results;
        EXPECT_EQ(PageTranslateRequestUtil::ParseTranslateResults(item.result, results), item.expected);
        EXPECT_EQ(results.size(), item.expectedSize);
    }

    std::vector<TranslateResult> results;
    ASSERT_TRUE(PageTranslateRequestUtil::ParseTranslateResults(
        R"({"results":[{"nodeId":6, "version":3000000000, "translatedText":"\u6807\u9898"},)"
        R"({"nodeId":13, "version":2, "text":"输入框提示"}]})",
        results));
    ASSERT_EQ(results.size(), 2U);
    EXPECT_EQ(results[0].nodeId, 6);
    EXPECT_EQ(results[0].version, TRANSLATE_LARGE_VERSION);
    EXPECT_EQ(results[0].translatedText, u8"标题");
    EXPECT_EQ(results[1].nodeId, 13);
    EXPECT_EQ(results[1].version, 2);
    EXPECT_EQ(results[1].translatedText, u8"输入框提示");
}

/**
 * @tc.name: UIContentStubTest047
 * @tc.desc: Test page translate result identity parser supports single, array, and batch results.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest047, TestSize.Level1)
{
    const TranslateResultIdentityTestData testData[] = {
        { R"({"nodeId":6, "version":0, "translatedText":"\u6807\u9898"})",
            { { TRANSLATE_TITLE_NODE_ID, TRANSLATE_SNAPSHOT_VERSION } } },
        { R"({"results":[{"nodeId":6, "version":1, "translatedText":"\u6807\u9898"},)"
          R"({"nodeId":13, "version":2, "text":"输入框提示"}, {"nodeId":16, "version":0, "text":"按钮"}]})",
            { { TRANSLATE_TITLE_NODE_ID, TRANSLATE_FIRST_VERSION },
                { TRANSLATE_PLACEHOLDER_NODE_ID, TRANSLATE_SECOND_VERSION },
                { TRANSLATE_BUTTON_NODE_ID, TRANSLATE_SNAPSHOT_VERSION } } },
        { R"([{"nodeId":6, "version":3, "translatedText":"标题"}, {"nodeId":13, "version":2, "text":"详情"}])",
            { { TRANSLATE_TITLE_NODE_ID, TRANSLATE_THIRD_VERSION },
                { TRANSLATE_PLACEHOLDER_NODE_ID, TRANSLATE_SECOND_VERSION } } },
        { R"({"nodeId":6, "version":3000000000, "translatedText":"标题"})",
            { { TRANSLATE_TITLE_NODE_ID, TRANSLATE_LARGE_VERSION } } },
        { R"({"results":[{"nodeId":-1, "version":1}, {"nodeId":16, "version":0}, {"nodeId":)"
          R"(7}]})",
            { { TRANSLATE_BUTTON_NODE_ID, TRANSLATE_SNAPSHOT_VERSION } } },
        { R"({"nodeId":6, "translatedText":"missing version"})", {} },
        { R"({"results":[]})", {} },
        { R"("invalid")", {} },
        { R"(invalid json)", {} },
    };

    for (const auto& item : testData) {
        EXPECT_EQ(PageTranslateRequestUtil::ParseTranslateResultIdentities(item.result), item.expected);
    }
}

/**
 * @tc.name: UIContentStubTest048
 * @tc.desc: Test page translate request transactions are dispatched to UiSessionManager.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest048, TestSize.Level1)
{
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    auto manager = static_cast<OHOS::Ace::MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(manager, nullptr);

    struct RequestTransactionCase {
        uint32_t code;
        int32_t result;
    };
    const RequestTransactionCase testData[] = {
        { IUiContentService::GET_PAGE_TRANSLATE_TEXT, PARAM_INVALID },
        { IUiContentService::START_PAGE_TRANSLATE, LAST_UNFINISH },
    };

    for (const auto& item : testData) {
        Mock::VerifyAndClearExpectations(manager);
        if (item.code == IUiContentService::GET_PAGE_TRANSLATE_TEXT) {
            EXPECT_CALL(*manager, GetPageTranslateText(StrEq("mockString"))).Times(1).WillOnce(Return(item.result));
        } else {
            EXPECT_CALL(*manager, StartPageTranslate(StrEq("mockString"))).Times(1).WillOnce(Return(item.result));
        }
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        PrepareValidUiSessionParcel(data);
        data.WriteString(VALID_TRANSLATE_SCOPE_REQUEST);

        EXPECT_EQ(ptr->OnRemoteRequest(item.code, data, reply, option), NO_ERROR);
    }
    Mock::VerifyAndClearExpectations(manager);
}

/**
 * @tc.name: UIContentStubTest049
 * @tc.desc: Test page translate control transactions are dispatched to UiSessionManager.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest049, TestSize.Level1)
{
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    auto manager = static_cast<OHOS::Ace::MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(manager, nullptr);

    struct ControlTransactionCase {
        uint32_t code;
        std::function<void(MessageParcel&)> writePayload;
        std::function<void()> expectCall;
    };
    const std::string translateResult = R"({"nodeId":6, "version":1, "translatedText":"标题"})";
    const ControlTransactionCase testData[] = {
        {
            IUiContentService::END_PAGE_TRANSLATE,
            [](MessageParcel&) {},
            [manager]() { EXPECT_CALL(*manager, EndPageTranslate()).Times(1); },
        },
        {
            IUiContentService::RESET_PAGE_TRANSLATE,
            [](MessageParcel& data) { data.WriteInt32(TRANSLATE_TITLE_NODE_ID); },
            [manager]() { EXPECT_CALL(*manager, ResetPageTranslate(0)).Times(1); },
        },
        {
            IUiContentService::SEND_PAGE_TRANSLATE_RESULT,
            [&translateResult](MessageParcel& data) { data.WriteString(translateResult); },
            [manager]() { EXPECT_CALL(*manager, SendPageTranslateResult(_)).Times(0); },
        },
    };

    for (const auto& item : testData) {
        Mock::VerifyAndClearExpectations(manager);
        item.expectCall();
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        PrepareValidUiSessionParcel(data);
        item.writePayload(data);

        EXPECT_EQ(ptr->OnRemoteRequest(item.code, data, reply, option), NO_ERROR);
    }
    Mock::VerifyAndClearExpectations(manager);
}

/**
 * @tc.name: UIContentStubTest050
 * @tc.desc: Test page translate callback rejects repeated single get and releases after result finish.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest050, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    int32_t reportCount = 0;
    PageTranslateTextCallback callback = [&reportCount](int32_t, const std::string&, int64_t) {
        ++reportCount;
    };

    EXPECT_TRUE(report->RegisterPageTranslateTextCallback(callback, false));
    EXPECT_FALSE(report->RegisterPageTranslateTextCallback(callback, false));
    report->SendPageText(TRANSLATE_TITLE_NODE_ID, "title", TRANSLATE_FIRST_VERSION);
    EXPECT_EQ(reportCount, 1);
    EXPECT_FALSE(report->RegisterPageTranslateTextCallback(callback, false));
    report->FinishPageTranslateTextRequest();
    EXPECT_TRUE(report->RegisterPageTranslateTextCallback(callback, false));
    report->UnregisterPageTranslateTextCallback();
}

/**
 * @tc.name: UIContentStubTest051
 * @tc.desc: Test continuous page translate callback is released only by unregister.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest051, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    PageTranslateTextCallback callback = [](int32_t, const std::string&, int64_t) {};
    int32_t timeoutCount = 0;

    report->pageTranslateTimeoutCallback_ = [&timeoutCount]() {
        ++timeoutCount;
    };
    EXPECT_TRUE(report->RegisterPageTranslateTextCallback(callback, true));
    EXPECT_FALSE(report->pageTranslateTimeoutCallback_);
    report->HandlePageTranslateResultWatchdogTimeout(TRANSLATE_TITLE_NODE_ID, TRANSLATE_FIRST_VERSION);
    EXPECT_EQ(timeoutCount, 0);
    report->FinishPageTranslateTextRequest();
    EXPECT_FALSE(report->RegisterPageTranslateTextCallback(callback, false));
    report->UnregisterPageTranslateTextCallback();
    EXPECT_TRUE(report->RegisterPageTranslateTextCallback(callback, false));
    report->UnregisterPageTranslateTextCallback();
}

/**
 * @tc.name: UIContentStubTest052
 * @tc.desc: Test current ability language transaction uses synchronous manager query.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest052, TestSize.Level1)
{
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    auto manager = static_cast<OHOS::Ace::MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(manager, nullptr);
    EXPECT_CALL(*manager, GetCurrentAbilityLanguageInfo(_, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>("zh"), SetArgReferee<1>("CN"), Return(NO_ERROR)));
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    PrepareValidUiSessionParcel(data);

    EXPECT_EQ(ptr->OnRemoteRequest(IUiContentService::GET_CURRENT_ABILITY_LANGUAGE_INFO, data, reply, option),
        NO_ERROR);
    Mock::VerifyAndClearExpectations(manager);
}

/**
 * @tc.name: UIContentStubTest053
 * @tc.desc: Test synchronous request guard rejects repeated pending requests.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest053, TestSize.Level1)
{
    std::atomic_bool pending = false;
    {
        SyncRequestGuard firstGuard(pending);
        EXPECT_TRUE(firstGuard.IsAcquired());
        SyncRequestGuard secondGuard(pending);
        EXPECT_FALSE(secondGuard.IsAcquired());
        EXPECT_TRUE(pending.load());
    }
    EXPECT_FALSE(pending.load());

    SyncRequestGuard guardAfterRelease(pending);
    EXPECT_TRUE(guardAfterRelease.IsAcquired());
}

/**
 * @tc.name: UIContentStubTest054
 * @tc.desc: Test SEND_IMAGES accepts valid size=0 and dispatches to SendShowingImage.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTest054, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    int32_t callCount = 0;
    size_t receivedSize = 0;
    auto callback = [&callCount, &receivedSize](
        std::vector<std::pair<int32_t, std::shared_ptr<Media::PixelMap>>> maps) {
        ++callCount;
        receivedSize = maps.size();
    };
    report->RegisterGetShowingImageCallback(callback);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(u"ohos.ace.ReportService");
    data.WriteInt32(0);

    EXPECT_EQ(report->OnRemoteRequest(ReportService::SEND_IMAGES, data, reply, option), 0);
    EXPECT_EQ(callCount, 1);
    EXPECT_EQ(receivedSize, 0u);
}

/**
 * @tc.name: UIContentStubTestGetNavigationContentByPoint001
 * @tc.desc: Test GET_NAVIGATION_CONTENT_BY_POINT IPC dispatch with parameters.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestGetNavigationContentByPoint001, TestSize.Level1)
{
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    data.WriteFloat(100.0f);
    data.WriteFloat(200.0f);
    data.WriteString("nav.*");

    res = ptr->OnRemoteRequest(IUiContentService::GET_NAVIGATION_CONTENT_BY_POINT, data, reply, option);
    EXPECT_EQ(res, 0);
    EXPECT_EQ(reply.ReadInt32(), 0);
}

/**
 * @tc.name: UIContentStubTestGetLazyForEachDataByPoint001
 * @tc.desc: Test GET_LAZY_FOREACH_DATA_BY_POINT IPC dispatch with parameters.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestGetLazyForEachDataByPoint001, TestSize.Level1)
{
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    MessageParcel data = MessageParcel();
    MessageParcel reply = MessageParcel();
    MessageOption option = MessageOption();

    auto res = ptr->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, -1);

    Security::AccessToken::AccessTokenKit::mockTokenTypeFlag_ = Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    data.WriteInterfaceToken(u"ohos.ace.UIContentService");
    data.WriteFloat(50.0f);
    data.WriteFloat(75.0f);

    res = ptr->OnRemoteRequest(IUiContentService::GET_LAZY_FOREACH_DATA_BY_POINT, data, reply, option);
    EXPECT_EQ(res, 0);
    EXPECT_EQ(reply.ReadInt32(), 0);
}

/**
 * @tc.name: UIContentStubTestPageSceneDualRuleSet001
 * @tc.desc: A merged presence event dispatches only each callback's registered ruleSet evidence.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestPageSceneDualRuleSet001, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    int32_t callbackACount = 0;
    int32_t callbackBCount = 0;
    std::string lastA;
    std::string lastB;
    report->RegisterPageSceneEventCallback("search-set-a",
        [&callbackACount, &lastA](const std::string& data) {
            ++callbackACount;
            lastA = data;
        });
    report->RegisterPageSceneEventCallback("search-set-b",
        [&callbackBCount, &lastB](const std::string& data) {
            ++callbackBCount;
            lastB = data;
        });

    const std::string mergedJson = R"({"eventType":"presence", "hasSearch":true, "pageName":"Home",
        "hits":[{"ruleSetId":"search-set-a", "ruleId":"r1"},
                {"ruleSetId":"search-set-b", "ruleId":"r1"}]})";
    report->ReportPageSceneEvent(mergedJson, false);
    EXPECT_EQ(callbackACount, 1);
    EXPECT_EQ(callbackBCount, 1);
    for (const auto& [payload, ruleSetId] :
        { std::make_pair(lastA, "search-set-a"), std::make_pair(lastB, "search-set-b") }) {
        auto root = InspectorJsonUtil::ParseJsonString(payload);
        ASSERT_TRUE(root->IsObject());
        EXPECT_EQ(root->GetString("eventType"), "presence");
        EXPECT_EQ(root->GetString("pageName"), "Home");
        EXPECT_EQ(root->GetValue("hasSearch")->ToString(), "true");
        auto hits = root->GetValue("hits");
        ASSERT_EQ(hits->GetArraySize(), 1);
        EXPECT_EQ(hits->GetArrayItem(0)->GetString("ruleSetId"), ruleSetId);
        EXPECT_EQ(hits->GetArrayItem(0)->GetString("ruleId"), "r1");
    }
}

/**
 * @tc.name: UIContentStubTestPageSceneLegacyCallback001
 * @tc.desc: Legacy entry/exit payloads route by top-level ruleSetId without changing the JSON.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestPageSceneLegacyCallback001, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    std::vector<std::string> received;
    int32_t otherCount = 0;
    report->RegisterPageSceneEventCallback("legacy-set",
        [&received](const std::string& data) { received.emplace_back(data); });
    report->RegisterPageSceneEventCallback("search-set",
        [&otherCount](const std::string&) { ++otherCount; });
    const std::string entered = R"({"ruleSetId":"legacy-set", "ruleId":"editor",
        "sceneType":"TEXT_EDITOR", "eventName":"TEXT_EDITOR", "matched":true, "nodes":[]})";
    const std::string exited = R"({"ruleSetId":"legacy-set", "ruleId":"editor",
        "sceneType":"TEXT_EDITOR", "eventName":"TEXT_EDITOR_EXIT", "matched":false, "nodes":[]})";
    report->ReportPageSceneEvent(entered, false);
    report->ReportPageSceneEvent(exited, false);
    EXPECT_EQ(received, (std::vector<std::string> { entered, exited }));
    EXPECT_EQ(otherCount, 0);
    report->UnregisterPageSceneEventCallback("legacy-set");
    report->ReportPageSceneEvent(entered, false);
    EXPECT_EQ(received.size(), 2U);
}

/**
 * @tc.name: UIContentStubTestPageSceneLegacyCallback002
 * @tc.desc: Deduplicate ids across legacy/search fields and retain independent GetPageScene delivery.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestPageSceneLegacyCallback002, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    int32_t eventCount = 0;
    int32_t getCount = 0;
    report->RegisterPageSceneEventCallback("legacy-set",
        [&eventCount](const std::string&) { ++eventCount; });
    report->RegisterGetPageSceneCallback([&getCount](const std::string&) { ++getCount; });
    const std::string payload = R"({"ruleSetId":"legacy-set",
        "hits":[{"ruleSetId":"legacy-set"}], "exits":[{"ruleSetId":"legacy-set"}]})";
    report->ReportPageSceneEvent(payload, false);
    EXPECT_EQ(eventCount, 1);
    report->ReportPageSceneEvent(payload, true);
    EXPECT_EQ(eventCount, 1);
    EXPECT_EQ(getCount, 1);
}

/**
 * @tc.name: UIContentStubTestPageSceneDualRuleSet002
 * @tc.desc: Unregister one ruleSet keeps the other; a missing callback does not
 *           block the present one. Exit events route by ruleSetId.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestPageSceneDualRuleSet002, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    int32_t callbackACount = 0;
    int32_t callbackBCount = 0;
    report->RegisterPageSceneEventCallback("search-set-a",
        [&callbackACount](const std::string&) { ++callbackACount; });
    report->RegisterPageSceneEventCallback("search-set-b",
        [&callbackBCount](const std::string&) { ++callbackBCount; });

    const std::string mergedJson = R"({"eventType":"presence", "hasSearch":true, "pageName":"Home",
        "hits":[{"ruleSetId":"search-set-a", "ruleId":"r1"},
                {"ruleSetId":"search-set-b", "ruleId":"r1"}]})";
    report->ReportPageSceneEvent(mergedJson, false);
    EXPECT_EQ(callbackACount, 1);
    EXPECT_EQ(callbackBCount, 1);

    report->UnregisterPageSceneEventCallback("search-set-a");
    report->ReportPageSceneEvent(mergedJson, false);
    EXPECT_EQ(callbackACount, 1);
    EXPECT_EQ(callbackBCount, 2);

    // Exit events route by ruleSetId.
    const std::string falseJson = R"({"eventType":"presence", "hasSearch":false, "pageName":"Home",
        "exits":[{"ruleSetId":"search-set-b", "ruleId":"r1"}]})";
    report->ReportPageSceneEvent(falseJson, false);
    EXPECT_EQ(callbackACount, 1);
    EXPECT_EQ(callbackBCount, 3);
}

/**
 * @tc.name: UIContentStubTestPageSceneFilteredEvent001
 * @tc.desc: Filter both hits and exits, preserving all matching rules and excluding unowned entries.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestPageSceneFilteredEvent001, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    std::string lastA;
    std::string lastB;
    report->RegisterPageSceneEventCallback("a", [&lastA](const std::string& data) { lastA = data; });
    report->RegisterPageSceneEventCallback("b", [&lastB](const std::string& data) { lastB = data; });
    report->ReportPageSceneEvent(R"({"eventType":"presence", "hasSearch":false, "pageName":"Home",
        "hits":[{"ruleSetId":"a", "ruleId":"r1"}, {"ruleSetId":"b", "ruleId":"r2"}],
        "exits":[{"ruleSetId":"b", "ruleId":"r2"}, {"ruleSetId":"a", "ruleId":"r1"},
                 {"ruleSetId":"a", "ruleId":"r3"}, {"ruleId":"unowned"}, null]})", false);

    for (const auto& [payload, ruleSetId] : { std::make_pair(lastA, "a"), std::make_pair(lastB, "b") }) {
        auto root = InspectorJsonUtil::ParseJsonString(payload);
        ASSERT_TRUE(root->IsObject());
        EXPECT_EQ(root->GetValue("hasSearch")->ToString(), "false");
        EXPECT_EQ(root->GetString("pageName"), "Home");
        auto hits = root->GetValue("hits");
        ASSERT_EQ(hits->GetArraySize(), 1);
        EXPECT_EQ(hits->GetArrayItem(0)->GetString("ruleSetId"), ruleSetId);
        auto exits = root->GetValue("exits");
        ASSERT_EQ(exits->GetArraySize(), std::string(ruleSetId) == "a" ? 2 : 1);
        for (int32_t index = 0; index < exits->GetArraySize(); ++index) {
            EXPECT_EQ(exits->GetArrayItem(index)->GetString("ruleSetId"), ruleSetId);
        }
    }
    auto rootA = InspectorJsonUtil::ParseJsonString(lastA);
    auto exitsA = rootA->GetValue("exits");
    EXPECT_EQ(exitsA->GetArrayItem(0)->GetString("ruleId"), "r1");
    EXPECT_EQ(exitsA->GetArrayItem(1)->GetString("ruleId"), "r3");
}

/**
 * @tc.name: UIContentStubTestPageSceneFilteredEvent002
 * @tc.desc: Click evidence remains complete after filtering, and callbacks can unregister themselves.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestPageSceneFilteredEvent002, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    std::string lastA;
    std::string lastB;
    report->RegisterPageSceneEventCallback("a", [&lastA, &report](const std::string& data) {
        lastA = data;
        report->UnregisterPageSceneEventCallback("a");
    });
    report->RegisterPageSceneEventCallback("b", [&lastB](const std::string& data) { lastB = data; });
    const std::string payload = R"({"eventType":"click", "pageName":"Home+Dialog", "hits":[
        {"ruleSetId":"a", "ruleId":"r1", "matchType":"inspectorId", "matchedKeyword":"search",
         "responseNode":{"inspectorId":"search-button", "nodeId":7, "type":"Button",
                         "rect":{"x":1, "y":2, "width":30, "height":40}}},
        {"ruleSetId":"b", "ruleId":"r2", "responseNode":{"nodeId":8}}]})";
    report->ReportPageSceneEvent(payload, false);

    auto rootA = InspectorJsonUtil::ParseJsonString(lastA);
    ASSERT_TRUE(rootA->IsObject());
    EXPECT_EQ(rootA->GetString("eventType"), "click");
    EXPECT_EQ(rootA->GetString("pageName"), "Home+Dialog");
    auto hitsA = rootA->GetValue("hits");
    ASSERT_EQ(hitsA->GetArraySize(), 1);
    auto hitA = hitsA->GetArrayItem(0);
    EXPECT_EQ(hitA->GetString("ruleSetId"), "a");
    EXPECT_EQ(hitA->GetString("matchedKeyword"), "search");
    auto original = InspectorJsonUtil::ParseJsonString(payload);
    EXPECT_EQ(hitA->ToString(), original->GetValue("hits")->GetArrayItem(0)->ToString());
    auto rootB = InspectorJsonUtil::ParseJsonString(lastB);
    ASSERT_TRUE(rootB->IsObject());
    ASSERT_EQ(rootB->GetValue("hits")->GetArraySize(), 1);
    EXPECT_EQ(rootB->GetValue("hits")->GetArrayItem(0)->GetString("ruleSetId"), "b");
    EXPECT_EQ(report->pageSceneEventCallbacks_.count("a"), 0U);
}

/**
 * @tc.name: UIContentStubTestPageSceneFilteredEvent003
 * @tc.desc: Legacy top-level data cannot be delivered to a different ruleSet named in an array.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestPageSceneFilteredEvent003, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    std::string received;
    int32_t otherCount = 0;
    report->RegisterPageSceneEventCallback("legacy", [&received](const std::string& data) { received = data; });
    report->RegisterPageSceneEventCallback("other", [&otherCount](const std::string&) { ++otherCount; });
    report->ReportPageSceneEvent(R"({"ruleSetId":"legacy", "nodes":[{"nodeId":1}],
        "hits":[{"ruleSetId":"other", "ruleId":"r1"}]})", false);
    auto root = InspectorJsonUtil::ParseJsonString(received);
    ASSERT_TRUE(root->IsObject());
    EXPECT_EQ(root->GetString("ruleSetId"), "legacy");
    EXPECT_EQ(root->GetValue("nodes")->GetArraySize(), 1);
    EXPECT_EQ(root->GetValue("hits")->GetArraySize(), 0);
    EXPECT_EQ(otherCount, 0);
}

/**
 * @tc.name: UIContentStubTestPageSceneCallbackRegistration001
 * @tc.desc: Duplicate callback reservation preserves the original callback; unregister permits a retry.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestPageSceneCallbackRegistration001, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    int32_t originalCount = 0;
    int32_t replacementCount = 0;
    PageSceneEventCallback original = [&originalCount](const std::string&) { ++originalCount; };
    PageSceneEventCallback replacement = [&replacementCount](const std::string&) { ++replacementCount; };
    EXPECT_FALSE(report->TryRegisterPageSceneEventCallback("", original));
    EXPECT_FALSE(report->TryRegisterPageSceneEventCallback("a", nullptr));
    ASSERT_TRUE(report->TryRegisterPageSceneEventCallback("a", original));
    EXPECT_FALSE(report->TryRegisterPageSceneEventCallback("a", replacement));
    const std::string payload = R"({"eventType":"presence", "hits":[{"ruleSetId":"a", "ruleId":"r1"}]})";
    report->ReportPageSceneEvent(payload, false);
    EXPECT_EQ(originalCount, 1);
    EXPECT_EQ(replacementCount, 0);
    report->UnregisterPageSceneEventCallback("a");
    ASSERT_TRUE(report->TryRegisterPageSceneEventCallback("a", replacement));
    report->ReportPageSceneEvent(payload, false);
    EXPECT_EQ(originalCount, 1);
    EXPECT_EQ(replacementCount, 1);
}

/**
 * @tc.name: UIContentStubTestQueryRegister001
 * @tc.desc: Register succeeds when empty; duplicate registration rejected.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryRegister001, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    bool called = false;
    auto callback = [&called](const std::string&) { called = true; };

    EXPECT_TRUE(report->RegisterNavigationQueryCallback(callback));
    EXPECT_FALSE(report->RegisterNavigationQueryCallback(callback));
    EXPECT_FALSE(called);
    report->UnregisterNavigationQueryCallback();
}

/**
 * @tc.name: UIContentStubTestQueryRegister002
 * @tc.desc: After unregister, re-registration succeeds.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryRegister002, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    auto callback = [](const std::string&) {};

    EXPECT_TRUE(report->RegisterNavigationQueryCallback(callback));
    report->UnregisterNavigationQueryCallback();
    EXPECT_TRUE(report->RegisterNavigationQueryCallback(callback));
    report->UnregisterNavigationQueryCallback();
}

/**
 * @tc.name: UIContentStubTestQueryRegister003
 * @tc.desc: Navigation and LazyForEach register independently.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryRegister003, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    auto navCb = [](const std::string&) {};
    auto lazyCb = [](const std::string&) {};

    EXPECT_TRUE(report->RegisterNavigationQueryCallback(navCb));
    EXPECT_TRUE(report->RegisterLazyForEachQueryCallback(lazyCb));
    EXPECT_FALSE(report->RegisterNavigationQueryCallback(navCb));
    EXPECT_FALSE(report->RegisterLazyForEachQueryCallback(lazyCb));
    report->UnregisterNavigationQueryCallback();
    report->UnregisterLazyForEachQueryCallback();
}

/**
 * @tc.name: UIContentStubTestQueryUnregister001
 * @tc.desc: Unregister clears callback; subsequent Report silently drops data.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryUnregister001, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    int32_t callCount = 0;
    auto callback = [&callCount](const std::string&) { ++callCount; };

    EXPECT_TRUE(report->RegisterNavigationQueryCallback(callback));
    report->UnregisterNavigationQueryCallback();
    report->ReportNavigationQueryResult("test-data");
    EXPECT_EQ(callCount, 0);
}

/**
 * @tc.name: UIContentStubTestQueryPostTimeout001
 * @tc.desc: PostTimeout without event handler returns false and clears callback.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryPostTimeout001, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    auto callback = [](const std::string&) {};

    EXPECT_TRUE(report->RegisterNavigationQueryCallback(callback));
    EXPECT_FALSE(report->PostNavigationQueryCallbackRemoveTask(1000));
    EXPECT_FALSE(report->navigationQueryCallback_);
    EXPECT_TRUE(report->RegisterNavigationQueryCallback(callback));
    report->UnregisterNavigationQueryCallback();
}

/**
 * @tc.name: UIContentStubTestQueryPostTimeout002
 * @tc.desc: LazyForEach PostTimeout without event handler returns false and clears callback.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryPostTimeout002, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    auto callback = [](const std::string&) {};

    EXPECT_TRUE(report->RegisterLazyForEachQueryCallback(callback));
    EXPECT_FALSE(report->PostLazyForEachQueryCallbackRemoveTask(1000));
    EXPECT_FALSE(report->lazyForEachQueryCallback_);
    EXPECT_TRUE(report->RegisterLazyForEachQueryCallback(callback));
    report->UnregisterLazyForEachQueryCallback();
}

/**
 * @tc.name: UIContentStubTestQueryReport001
 * @tc.desc: Registered callback receives data; callback cleared after report.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryReport001, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    std::string received;
    auto callback = [&received](const std::string& data) { received = data; };

    EXPECT_TRUE(report->RegisterNavigationQueryCallback(callback));
    report->ReportNavigationQueryResult("hello-world");
    EXPECT_EQ(received, "hello-world");
    EXPECT_FALSE(report->navigationQueryCallback_);
}

/**
 * @tc.name: UIContentStubTestQueryReport002
 * @tc.desc: Without registered callback, Report silently drops data (no crash).
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryReport002, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    report->ReportNavigationQueryResult("orphan-data");
    EXPECT_FALSE(report->navigationQueryCallback_);
}

/**
 * @tc.name: UIContentStubTestQueryReport003
 * @tc.desc: Navigation report does not affect LazyForEach callback and vice versa.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryReport003, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    std::string navReceived;
    std::string lazyReceived;
    auto navCb = [&navReceived](const std::string& data) { navReceived = data; };
    auto lazyCb = [&lazyReceived](const std::string& data) { lazyReceived = data; };

    EXPECT_TRUE(report->RegisterNavigationQueryCallback(navCb));
    EXPECT_TRUE(report->RegisterLazyForEachQueryCallback(lazyCb));

    report->ReportNavigationQueryResult("nav-result");
    EXPECT_EQ(navReceived, "nav-result");
    EXPECT_TRUE(lazyReceived.empty());
    EXPECT_TRUE(report->lazyForEachQueryCallback_);

    report->ReportLazyForEachQueryResult("lazy-result");
    EXPECT_EQ(lazyReceived, "lazy-result");
}

/**
 * @tc.name: UIContentStubTestQueryReport004
 * @tc.desc: LazyForEach registered callback receives data; callback cleared after report.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryReport004, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    std::string received;
    auto callback = [&received](const std::string& data) { received = data; };

    EXPECT_TRUE(report->RegisterLazyForEachQueryCallback(callback));
    report->ReportLazyForEachQueryResult("lazy-data");
    EXPECT_EQ(received, "lazy-data");
    EXPECT_FALSE(report->lazyForEachQueryCallback_);
}

HWTEST_F(UIContentStubTest, SearchReview_ClickFlushDoesNotDetectPages, TestSize.Level1)
{
    class SearchClickSession : public UiSessionManager {
    public:
        void NotifyPageSceneContentChanged() override { ++detectCount; }
        void FlushPageSceneNodeChanged() override { ++detectCount; }
        int detectCount = 0;
    };
    SearchClickSession manager;
    int flushCount = 0;
    // No callback is a safe no-op, including during UIContent teardown.
    manager.FlushPendingSearchClicksIfStable();
    manager.SaveSearchClickFlushFunction([&manager, &flushCount] {
        ++flushCount;
        // Replacement must be safe: the dispatcher cannot hold its mutex here.
        manager.SaveSearchClickFlushFunction({});
    });
    manager.FlushPendingSearchClicksIfStable();
    manager.FlushPendingSearchClicksIfStable();
    EXPECT_EQ(flushCount, 1);
    EXPECT_EQ(manager.detectCount, 0);
}

/**
 * @tc.name: UIContentStubTestGetNavigationContentByPoint002
 * @tc.desc: Navigation query dispatches SaveProcessId (key is a literal in the Inner) and
 *           ComponentTreeQuery (request type is an enum literal). x/y/pattern passthrough is
 *           NOT asserted: the host MessageParcel mock is stateless (WriteFloat/WriteString
 *           store nothing; ReadFloat returns 0.0f, ReadString returns "mockString"), so the
 *           Inner always reads mock constants regardless of what the test writes.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestGetNavigationContentByPoint002, TestSize.Level1)
{
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    auto manager = static_cast<OHOS::Ace::MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(manager, nullptr);

    ComponentTreeQueryRequest captured;
    EXPECT_CALL(*manager, SaveProcessId(StrEq("componentTreeQuery"), _)).Times(1);
    EXPECT_CALL(*manager, ComponentTreeQuery(_)).Times(1).WillOnce(SaveArg<0>(&captured));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    PrepareValidUiSessionParcel(data);
    data.WriteFloat(10.5f);
    data.WriteFloat(20.5f);
    data.WriteString("bar.*");

    EXPECT_EQ(ptr->OnRemoteRequest(IUiContentService::GET_NAVIGATION_CONTENT_BY_POINT, data, reply, option), NO_ERROR);
    EXPECT_EQ(reply.ReadInt32(), NO_ERROR);
    EXPECT_EQ(captured.type, ComponentTreeQueryType::NAVIGATION_CONTENT_BY_POINT);
    Mock::VerifyAndClearExpectations(manager);
}

/**
 * @tc.name: UIContentStubTestGetNavigationContentByPoint003
 * @tc.desc: Navigation impl field-mapping contract. Calling GetNavigationContentByPoint
 *           directly (bypassing the stateless MessageParcel mock) verifies x/y/pattern reach
 *           the ComponentTreeQueryRequest. The impl is linear with no boundary-specific logic,
 *           so the non-positive coordinates and empty pattern here are representative inputs,
 *           not a boundary-behavior probe. SaveProcessId is NOT expected (only the Inner calls
 *           it; the impl does not). Guards against regressions like swapped x/y or an omitted
 *           pattern field, and pins the impl contract independently of the Inner/parcel path.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestGetNavigationContentByPoint003, TestSize.Level1)
{
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    auto manager = static_cast<OHOS::Ace::MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(manager, nullptr);

    ComponentTreeQueryRequest captured;
    EXPECT_CALL(*manager, ComponentTreeQuery(_)).Times(1).WillOnce(SaveArg<0>(&captured));

    EXPECT_EQ(ptr->GetNavigationContentByPoint(-1.0f, 0.0f, "", nullptr), NO_ERROR);
    EXPECT_EQ(captured.type, ComponentTreeQueryType::NAVIGATION_CONTENT_BY_POINT);
    EXPECT_FLOAT_EQ(captured.x, -1.0f);
    EXPECT_FLOAT_EQ(captured.y, 0.0f);
    EXPECT_TRUE(captured.pattern.empty());
    Mock::VerifyAndClearExpectations(manager);
}

/**
 * @tc.name: UIContentStubTestGetLazyForEachDataByPoint002
 * @tc.desc: LazyForEach query dispatches SaveProcessId (key literal) and ComponentTreeQuery
 *           (type literal). x/y passthrough is NOT asserted (stateless MessageParcel mock;
 *           see UIContentStubTestGetNavigationContentByPoint002 for rationale).
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestGetLazyForEachDataByPoint002, TestSize.Level1)
{
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    auto manager = static_cast<OHOS::Ace::MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(manager, nullptr);

    ComponentTreeQueryRequest captured;
    EXPECT_CALL(*manager, SaveProcessId(StrEq("lazyForEachQuery"), _)).Times(1);
    EXPECT_CALL(*manager, ComponentTreeQuery(_)).Times(1).WillOnce(SaveArg<0>(&captured));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    PrepareValidUiSessionParcel(data);
    data.WriteFloat(50.0f);
    data.WriteFloat(75.0f);

    EXPECT_EQ(ptr->OnRemoteRequest(IUiContentService::GET_LAZY_FOREACH_DATA_BY_POINT, data, reply, option), NO_ERROR);
    EXPECT_EQ(reply.ReadInt32(), NO_ERROR);
    EXPECT_EQ(captured.type, ComponentTreeQueryType::LAZY_FOREACH_DATA_BY_POINT);
    Mock::VerifyAndClearExpectations(manager);
}

/**
 * @tc.name: UIContentStubTestGetLazyForEachDataByPoint003
 * @tc.desc: LazyForEach impl field-mapping contract. Calling GetLazyForEachDataByPoint directly
 *           verifies x/y reach the ComponentTreeQueryRequest. The impl is linear with no
 *           boundary-specific logic; the zero/negative coordinates are representative inputs.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestGetLazyForEachDataByPoint003, TestSize.Level1)
{
    sptr<UIContentServiceStubImpl> ptr = std::make_shared<UIContentServiceStubImpl>();
    auto manager = static_cast<OHOS::Ace::MockUiSessionManager*>(UiSessionManager::GetInstance());
    ASSERT_NE(manager, nullptr);

    ComponentTreeQueryRequest captured;
    EXPECT_CALL(*manager, ComponentTreeQuery(_)).Times(1).WillOnce(SaveArg<0>(&captured));

    EXPECT_EQ(ptr->GetLazyForEachDataByPoint(0.0f, -100.0f, nullptr), NO_ERROR);
    EXPECT_EQ(captured.type, ComponentTreeQueryType::LAZY_FOREACH_DATA_BY_POINT);
    EXPECT_FLOAT_EQ(captured.x, 0.0f);
    EXPECT_FLOAT_EQ(captured.y, -100.0f);
    EXPECT_TRUE(captured.pattern.empty());
    Mock::VerifyAndClearExpectations(manager);
}

/**
 * @tc.name: UIContentStubTestQueryTimeout001
 * @tc.desc: HandleNavigationQueryCallbackTimeout invokes callback with errorCode and clears it.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryTimeout001, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    std::string received;
    auto callback = [&received](const std::string& data) { received = data; };

    EXPECT_TRUE(report->RegisterNavigationQueryCallback(callback));
    report->HandleNavigationQueryCallbackTimeout();
    EXPECT_EQ(received, R"({"errorCode":8})");
    EXPECT_FALSE(report->navigationQueryCallback_);
}

/**
 * @tc.name: UIContentStubTestQueryTimeout002
 * @tc.desc: HandleNavigationQueryCallbackTimeout without registered callback returns silently.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryTimeout002, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    report->HandleNavigationQueryCallbackTimeout();
    EXPECT_FALSE(report->navigationQueryCallback_);
}

/**
 * @tc.name: UIContentStubTestQueryTimeout003
 * @tc.desc: HandleLazyForEachQueryCallbackTimeout invokes callback with errorCode and clears it.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryTimeout003, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    std::string received;
    auto callback = [&received](const std::string& data) { received = data; };

    EXPECT_TRUE(report->RegisterLazyForEachQueryCallback(callback));
    report->HandleLazyForEachQueryCallbackTimeout();
    EXPECT_EQ(received, R"({"errorCode":8})");
    EXPECT_FALSE(report->lazyForEachQueryCallback_);
}

/**
 * @tc.name: UIContentStubTestQueryTimeout004
 * @tc.desc: HandleLazyForEachQueryCallbackTimeout without registered callback returns silently.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryTimeout004, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    report->HandleLazyForEachQueryCallbackTimeout();
    EXPECT_FALSE(report->lazyForEachQueryCallback_);
}

/**
 * @tc.name: UIContentStubTestQueryPostTimeout003
 * @tc.desc: PostNavigationQueryCallbackRemoveTask without handler invokes callback with errorCode.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryPostTimeout003, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    std::string received;
    auto callback = [&received](const std::string& data) { received = data; };

    EXPECT_TRUE(report->RegisterNavigationQueryCallback(callback));
    EXPECT_FALSE(report->PostNavigationQueryCallbackRemoveTask(1000));
    EXPECT_EQ(received, R"({"errorCode":8})");
    EXPECT_FALSE(report->navigationQueryCallback_);
}

/**
 * @tc.name: UIContentStubTestQueryPostTimeout004
 * @tc.desc: PostLazyForEachQueryCallbackRemoveTask without handler invokes callback with errorCode.
 * @tc.type: FUNC
 */
HWTEST_F(UIContentStubTest, UIContentStubTestQueryPostTimeout004, TestSize.Level1)
{
    sptr<UiReportStub> report = std::make_shared<UiReportStub>();
    std::string received;
    auto callback = [&received](const std::string& data) { received = data; };

    EXPECT_TRUE(report->RegisterLazyForEachQueryCallback(callback));
    EXPECT_FALSE(report->PostLazyForEachQueryCallbackRemoveTask(1000));
    EXPECT_EQ(received, R"({"errorCode":8})");
    EXPECT_FALSE(report->lazyForEachQueryCallback_);
}

} // namespace OHOS::Ace::NG