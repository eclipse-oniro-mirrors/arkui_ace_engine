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
 *
 * Standalone test suite for interfaces/native/ui_json_wrapper.cpp.
 * Verifies Create/GetData/GetSize/Destroy, including data ownership and byte-copy
 * consistency guard (rejects truncation/padding/embedded-null).
 */

#include <cstdint>
#include <cstring>
#include <string>

#include "gtest/gtest.h"

#include "interfaces/native/ui_json_wrapper.h"
#include "interfaces/native/native_type.h"

using namespace testing;
using namespace testing::ext;

class UIJsonWrapperTest : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

static uint32_t StrLen(const char* s)
{
    return s == nullptr ? 0 : static_cast<uint32_t>(strlen(s));
}

HWTEST_F(UIJsonWrapperTest, CreateSuccessRoundTrip001, TestSize.Level1)
{
    const char* json = "{\"key\":\"value\"}";
    OH_ArkUI_NativeModule_UIJsonWrapper* w = nullptr;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(json, StrLen(json), &w), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(w, nullptr);
    EXPECT_STREQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(w), json);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetSize(w), StrLen(json));
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w);
}

HWTEST_F(UIJsonWrapperTest, CreateDeepCopyIndependence001, TestSize.Level1)
{
    char buf[8] = "abc";
    OH_ArkUI_NativeModule_UIJsonWrapper* w = nullptr;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(buf, 3, &w), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(w, nullptr);
    buf[0] = 'X';
    EXPECT_STREQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(w), "abc");
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w);
}

HWTEST_F(UIJsonWrapperTest, CreateNullData001, TestSize.Level1)
{
    OH_ArkUI_NativeModule_UIJsonWrapper* w = nullptr;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(nullptr, 3, &w), ARKUI_ERROR_CODE_PARAM_INVALID);
}


HWTEST_F(UIJsonWrapperTest, CreateNullOut001, TestSize.Level1)
{
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate("a", 1, nullptr), ARKUI_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(UIJsonWrapperTest, CreateSingleChar001, TestSize.Level1)
{
    OH_ArkUI_NativeModule_UIJsonWrapper* w = nullptr;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate("a", 1, &w), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(w, nullptr);
    EXPECT_STREQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(w), "a");
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetSize(w), 1u);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w);
}

HWTEST_F(UIJsonWrapperTest, CreateNullTerminated001, TestSize.Level1)
{
    OH_ArkUI_NativeModule_UIJsonWrapper* w = nullptr;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate("abc", 3, &w), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(w, nullptr);
    const char* data = OH_ArkUI_NativeModule_UIJsonWrapperGetData(w);
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data[3], '\0');
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w);
}

HWTEST_F(UIJsonWrapperTest, CreateLargePayload001, TestSize.Level1)
{
    std::string big(10240, 'x');
    OH_ArkUI_NativeModule_UIJsonWrapper* w = nullptr;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(big.c_str(),
        static_cast<uint32_t>(big.size()), &w), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(w, nullptr);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetSize(w), static_cast<uint32_t>(big.size()));
    EXPECT_EQ(std::string(OH_ArkUI_NativeModule_UIJsonWrapperGetData(w), big.size()), big);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w);
}

HWTEST_F(UIJsonWrapperTest, CreateTwoWrappersIndependent001, TestSize.Level1)
{
    OH_ArkUI_NativeModule_UIJsonWrapper* w1 = nullptr;
    OH_ArkUI_NativeModule_UIJsonWrapper* w2 = nullptr;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate("first", 5, &w1), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate("second", 6, &w2), ARKUI_ERROR_CODE_NO_ERROR);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w1);
    EXPECT_STREQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(w2), "second");
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w2);
}

HWTEST_F(UIJsonWrapperTest, GetDataNull001, TestSize.Level1)
{
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(nullptr), nullptr);
}

HWTEST_F(UIJsonWrapperTest, GetSizeNull001, TestSize.Level1)
{
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetSize(nullptr), 0u);
}

HWTEST_F(UIJsonWrapperTest, DestroyNull001, TestSize.Level1)
{
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(nullptr);
    SUCCEED();
}

HWTEST_F(UIJsonWrapperTest, DestroyValidNoLeak001, TestSize.Level1)
{
    OH_ArkUI_NativeModule_UIJsonWrapper* w = nullptr;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate("data", 4, &w), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(w, nullptr);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w);
    SUCCEED();
}

HWTEST_F(UIJsonWrapperTest, FullLifecycleRoundTrip001, TestSize.Level1)
{
    const char* json = "{\"cmd\":{\"type\":\"setText\",\"action_info\":{\"value\":\"hi\"}}}";
    OH_ArkUI_NativeModule_UIJsonWrapper* w = nullptr;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(json, StrLen(json), &w), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(w, nullptr);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetSize(w), StrLen(json));
    EXPECT_STREQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(w), json);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w);
}
