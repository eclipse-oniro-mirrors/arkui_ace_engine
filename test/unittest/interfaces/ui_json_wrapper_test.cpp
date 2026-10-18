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

// Merged: CreateNullData001 + CreateNullOut001 + GetDataNull001 + GetSizeNull001 + DestroyNull001
// Covers null-param rejections for Create and null-safe accessors for GetData/GetSize/Destroy.
HWTEST_F(UIJsonWrapperTest, NullParamBoundary001, TestSize.Level1)
{
    OH_ArkUI_NativeModule_UIJsonWrapper* w = nullptr;
    // Create null data → 401
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(nullptr, 3, &w), ARKUI_ERROR_CODE_PARAM_INVALID);
    // Create null out → 401
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate("a", 1, nullptr), ARKUI_ERROR_CODE_PARAM_INVALID);
    // GetData(nullptr) → nullptr
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(nullptr), nullptr);
    // GetSize(nullptr) → 0
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetSize(nullptr), 0u);
    // Destroy(nullptr) → no-op
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(nullptr);
    SUCCEED();
}

// Covers single-char payload round-trip and null-terminator guarantee.
HWTEST_F(UIJsonWrapperTest, CreateSingleCharAndNullTerminator001, TestSize.Level1)
{
    OH_ArkUI_NativeModule_UIJsonWrapper* w = nullptr;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate("a", 1, &w), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(w, nullptr);
    EXPECT_STREQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(w), "a");
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetSize(w), 1u);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w);

    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate("abc", 3, &w), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(w, nullptr);
    const char* data = OH_ArkUI_NativeModule_UIJsonWrapperGetData(w);
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data[3], '\0');
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w);
}

// Merged: CreateSuccessRoundTrip001 + CreateDeepCopyIndependence001 + CreateLargePayload001
// Covers Create round-trip correctness, deep-copy independence, and large payload handling.
HWTEST_F(UIJsonWrapperTest, CreateAndDeepCopy001, TestSize.Level1)
{
    // Standard round-trip
    const char* json = "{\"key\":\"value\"}";
    OH_ArkUI_NativeModule_UIJsonWrapper* w = nullptr;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(json, StrLen(json), &w), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(w, nullptr);
    EXPECT_STREQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(w), json);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetSize(w), StrLen(json));
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w);

    // Deep-copy independence: modify source after Create
    char buf[8] = "abc";
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(buf, 3, &w), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(w, nullptr);
    buf[0] = 'X';
    EXPECT_STREQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(w), "abc");
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w);

    // Large payload (10KB)
    std::string big(10240, 'x');
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate(big.c_str(),
        static_cast<uint32_t>(big.size()), &w), ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(w, nullptr);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetSize(w), static_cast<uint32_t>(big.size()));
    EXPECT_EQ(std::string(OH_ArkUI_NativeModule_UIJsonWrapperGetData(w), big.size()), big);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w);
}

// Merged: CreateTwoWrappersIndependent001 + DestroyValidNoLeak001
// Covers two wrappers independent after destroying one, and valid destroy no leak.
HWTEST_F(UIJsonWrapperTest, TwoWrappersAndDestroy001, TestSize.Level1)
{
    OH_ArkUI_NativeModule_UIJsonWrapper* w1 = nullptr;
    OH_ArkUI_NativeModule_UIJsonWrapper* w2 = nullptr;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate("first", 5, &w1), ARKUI_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperCreate("second", 6, &w2), ARKUI_ERROR_CODE_NO_ERROR);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w1);
    EXPECT_STREQ(OH_ArkUI_NativeModule_UIJsonWrapperGetData(w2), "second");
    EXPECT_EQ(OH_ArkUI_NativeModule_UIJsonWrapperGetSize(w2), 6u);
    OH_ArkUI_NativeModule_UIJsonWrapperDestroy(w2);

    // Valid destroy — no crash, no leak
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
