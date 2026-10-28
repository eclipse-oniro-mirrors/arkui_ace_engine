/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include <algorithm>
#include <cstdlib>
#include <limits>
#include <cstdint>
#ifdef ACE_HOST_PRODUCT
#include "cJSON.h"
#include "core/components_ng/base/ui_tree_collector.h"
#include "interfaces/native/native_type.h"
#endif
#include "base/json/json_util_internal.h"
#include <sys/types.h>

#include "gtest/gtest.h"

#include "base/json/json_util.h"
#include "base/utils/utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
namespace {
const std::string TEST_STRING = "Ace Unittest";
const std::string TEST_KEY = "JsonObjectTypeTest";
const std::string TEST_FALSE_KEY = "FalseKey";
} // namespace

class JsonUtilTest : public testing::Test {};

/**
 * @tc.name: JsonUtilTest001
 * @tc.desc: Check json util function for bool type value
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the test string with bool value.
     */
    std::string testJson = "true";

    /**
     * @tc.steps: step2. get JsonValue and check results of IsValid, IsNull, IsBool and GetBool.
     * @tc.expected: step2. get the JsonValue successfully and the results are correct.
     */
    std::unique_ptr<JsonValue> boolValue = JsonUtil::ParseJsonString(testJson);
    ASSERT_TRUE(boolValue);
    EXPECT_TRUE(boolValue->IsValid());
    EXPECT_FALSE(boolValue->IsNull());
    EXPECT_TRUE(boolValue->IsBool());
    EXPECT_TRUE(boolValue->GetBool());
}

/**
 * @tc.name: JsonUtilTest002
 * @tc.desc: Check json util function for bool type value
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the test string with bool value.
     */
    std::string testJson = "false";

    /**
     * @tc.steps: step2. get JsonValue and check results of IsValid, IsNull, IsBool and GetBool.
     * @tc.expected: step2. get the JsonValue successfully and the results are correct.
     */
    std::unique_ptr<JsonValue> boolValue = JsonUtil::ParseJsonString(testJson);
    ASSERT_TRUE(boolValue);
    EXPECT_TRUE(boolValue->IsValid());
    EXPECT_FALSE(boolValue->IsNull());
    EXPECT_TRUE(boolValue->IsBool());
    EXPECT_FALSE(boolValue->GetBool());
}

/**
 * @tc.name: JsonUtilTest003
 * @tc.desc: Check json util function for signed integer
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the test string with signed integer.
     */
    std::string testJson = "-1";
    int32_t intNum = -1;
    double doubleNum = -1;

    /**
     * @tc.steps: step2. get JsonValue and check results of IsValid, IsNull, IsNumber, GetInt, GetUInt and GetDouble.
     * @@tc.expected: step2. get the JsonValue unsuccessfully and the results are 0
     */
    std::unique_ptr<JsonValue> intValue = JsonUtil::ParseJsonString(testJson);
    ASSERT_TRUE(intValue);
    EXPECT_TRUE(intValue->IsValid());
    EXPECT_FALSE(intValue->IsNull());
    EXPECT_TRUE(intValue->IsNumber());
    EXPECT_TRUE(intValue->GetInt() == intNum);
    EXPECT_TRUE(NearEqual(intValue->GetDouble(), doubleNum));
}

/**
 * @tc.name: JsonUtilTest004
 * @tc.desc: Check json util function for unsigned integer
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the test string with unsigned integer.
     */
    std::string testJson = "1";
    int32_t intNum = 1;
    uint32_t uintNum = 1;
    double doubleNum = 1;

    /**
     * @tc.steps: step2. get JsonValue and check results of IsValid, IsNull, IsNumber, GetInt, GetUInt and GetDouble.
     * @tc.expected: step2. get the JsonValue successfully and the results are correct.
     */
    std::unique_ptr<JsonValue> uintValue = JsonUtil::ParseJsonString(testJson);
    ASSERT_TRUE(uintValue);
    EXPECT_TRUE(uintValue->IsValid());
    EXPECT_FALSE(uintValue->IsNull());
    EXPECT_TRUE(uintValue->IsNumber());
    EXPECT_TRUE(uintValue->GetInt() == intNum);
    EXPECT_TRUE(uintValue->GetUInt() == uintNum);
    EXPECT_TRUE(NearEqual(uintValue->GetDouble(), doubleNum));
}

/**
 * @tc.name: JsonUtilTest005
 * @tc.desc: Check json util function for decimal number
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the test string with decimal number.
     */
    std::string testJson = "6.66";
    int32_t intNum = 6;
    uint32_t uintNum = 6;
    double doubleNum = 6.66;

    /**
     * @tc.steps: step2. get JsonValue and check results of IsValid, IsNull, IsNumber, GetInt, GetUInt and GetDouble.
     * @tc.expected: step2. get the JsonValue successfully and the results are correct.
     */
    std::unique_ptr<JsonValue> doubleValue = JsonUtil::ParseJsonString(testJson);
    ASSERT_TRUE(doubleValue);
    EXPECT_TRUE(doubleValue->IsValid());
    EXPECT_FALSE(doubleValue->IsNull());
    EXPECT_TRUE(doubleValue->IsNumber());
    EXPECT_TRUE(doubleValue->GetInt() == intNum);
    EXPECT_TRUE(doubleValue->GetUInt() == uintNum);
    EXPECT_TRUE(NearEqual(doubleValue->GetDouble(), doubleNum));
}

/**
 * @tc.name: JsonUtilTest006
 * @tc.desc: Check json util function for string
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the test string with string.
     */
    std::string testJson = "\"Ace Unittest\"";

    /**
     * @tc.steps: step2. get JsonValue and check results of IsValid, IsNull, IsString and GetString.
     * @tc.expected: step2. get the JsonValue successfully and the results are correct.
     */
    std::unique_ptr<JsonValue> stringValue = JsonUtil::ParseJsonString(testJson);
    ASSERT_TRUE(stringValue);
    EXPECT_TRUE(stringValue->IsValid());
    EXPECT_FALSE(stringValue->IsNull());
    EXPECT_TRUE(stringValue->IsString());
    EXPECT_TRUE(stringValue->GetString() == TEST_STRING);
}

/**
 * @tc.name: JsonUtilTest007
 * @tc.desc: Check json util function for empty string
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the test string with empty string.
     */
    std::string testJson = "\"\"";

    /**
     * @tc.steps: step2. get JsonValue and check results of IsValid, IsNull, IsString and GetString.
     * @tc.expected: step2. get the JsonValue successfully and the results are correct.
     */
    std::unique_ptr<JsonValue> emptyStringValue = JsonUtil::ParseJsonString(testJson);
    ASSERT_TRUE(emptyStringValue);
    EXPECT_TRUE(emptyStringValue->IsValid());
    EXPECT_FALSE(emptyStringValue->IsNull());
    EXPECT_TRUE(emptyStringValue->IsString());
    EXPECT_TRUE(emptyStringValue->GetString().empty());
}

/**
 * @tc.name: JsonUtilTest008
 * @tc.desc: Check json util function for JsonObject
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the test string with JsonObject.
     */
    std::string testJson = R"({"JsonObjectTypeTest": "Ace Unittest"})";

    /**
     * @tc.steps: step2. get JsonValue and check results of IsValid, IsNull, IsObject, Contains and GetValue.
     * @tc.expected: step2. get the JsonValue successfully and the results are correct.
     */
    std::unique_ptr<JsonValue> objectValue = JsonUtil::ParseJsonString(testJson);
    ASSERT_TRUE(objectValue);
    EXPECT_TRUE(objectValue->IsValid());
    EXPECT_FALSE(objectValue->IsNull());
    EXPECT_TRUE(objectValue->IsObject());
    EXPECT_TRUE(objectValue->Contains(TEST_KEY));
    EXPECT_FALSE(objectValue->Contains(TEST_FALSE_KEY));
    EXPECT_TRUE(objectValue->GetValue(TEST_KEY)->GetString() == TEST_STRING);
    EXPECT_TRUE(objectValue->GetValue(TEST_FALSE_KEY)->GetString().empty());
}

/**
 * @tc.name: JsonUtilTest009
 * @tc.desc: Check json util function for incorrect JsonObject
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the test string with incorrect JsonObject.
     */
    std::string testJson = R"({"JsonObjectTypeTest": ""})";

    /**
     * @tc.steps: step2. get JsonValue and check results of IsValid, IsNull, IsObject, Contains and GetValue.
     * @tc.expected: step2. get the JsonValue successfully and the results are correct.
     */
    std::unique_ptr<JsonValue> objectValue = JsonUtil::ParseJsonString(testJson);
    ASSERT_TRUE(objectValue);
    EXPECT_TRUE(objectValue->IsValid());
    EXPECT_FALSE(objectValue->IsNull());
    EXPECT_TRUE(objectValue->IsObject());
    EXPECT_TRUE(objectValue->Contains(TEST_KEY));
    EXPECT_TRUE(objectValue->GetValue(TEST_KEY)->GetString().empty());
}

/**
 * @tc.name: JsonUtilTest010
 * @tc.desc: Check json util function for array
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the test string with array.
     */
    std::string testJson = "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9]";
    int32_t testArraySize = 10;
    int32_t testArrayIndex = 5;
    int32_t testArrayValue = 5;

    /**
     * @tc.steps: step2. get JsonValue and check results of IsValid, IsNull, IsArray, GetArraySize and GetArrayItem.
     * @tc.expected: step2. get the JsonValue successfully and the results are correct.
     */
    std::unique_ptr<JsonValue> arrayValue = JsonUtil::ParseJsonString(testJson);
    ASSERT_TRUE(arrayValue);
    EXPECT_TRUE(arrayValue->IsValid());
    EXPECT_FALSE(arrayValue->IsNull());
    EXPECT_TRUE(arrayValue->IsArray());
    EXPECT_TRUE(arrayValue->GetArraySize() == testArraySize);
    EXPECT_TRUE(arrayValue->GetArrayItem(testArrayIndex)->GetInt() == testArrayValue);
}

/**
 * @tc.name: JsonUtilTest011
 * @tc.desc: Check json util function for empty array
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the test string with empty array.
     */
    std::string testJson = "[]";

    /**
     * @tc.steps: step2. get JsonValue and check results of IsValid, IsNull, IsArray, GetArraySize and GetArrayItem.
     * @tc.expected: step2. get the JsonValue successfully and the results are correct.
     */
    std::unique_ptr<JsonValue> arrayValue = JsonUtil::ParseJsonString(testJson);
    ASSERT_TRUE(arrayValue);
    EXPECT_TRUE(arrayValue->IsValid());
    EXPECT_FALSE(arrayValue->IsNull());
    EXPECT_TRUE(arrayValue->IsArray());
    EXPECT_TRUE(arrayValue->GetArraySize() == 0);
}

/**
 * @tc.name: JsonUtilTest012
 * @tc.desc: Check json util function for empty test string
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the empty test string.
     */
    std::string testJson;

    /**
     * @tc.steps: step2. get JsonValue and check results of IsValid, IsNull.
     * @tc.expected: step2. get the JsonValue successfully and the results are correct.
     */
    std::unique_ptr<JsonValue> emptyValue = JsonUtil::ParseJsonString(testJson);
    ASSERT_TRUE(emptyValue);
    EXPECT_FALSE(emptyValue->IsValid());
    EXPECT_TRUE(emptyValue->IsNull());
}

/**
 * @tc.name: JsonUtilTest013
 * @tc.desc: Check json util function for illegal type value
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest013, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the test string with illegal type value.
     */
    std::string testJson = "{Ace Unittest}";

    /**
     * @tc.steps: step2. get JsonValue and check results of IsValid, IsNull.
     * @tc.expected: step2. get the JsonValue successfully and the results are correct.
     */
    std::unique_ptr<JsonValue> illegalValue = JsonUtil::ParseJsonString(testJson);
    ASSERT_TRUE(illegalValue);
    EXPECT_FALSE(illegalValue->IsValid());
    EXPECT_TRUE(illegalValue->IsNull());
}

/**
 * @tc.name: JsonUtilTest014
 * @tc.desc: Check json util function Put(const char* key, const char* value) for nullptr key or value
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest014, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key.
     */
    const char* key = nullptr;
    const char* value = nullptr;
    JsonValue jsonValue;
    /**
     * @tc.steps: step2. get results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.Put(key, value);
    EXPECT_FALSE(ret);

    /**
     * @tc.steps: step3. construct the nullptr value.
     */
    const char* key2 = "5";
    const char* value2 = nullptr;
    /**
     * @tc.steps: step4. get results
     * @tc.expected: step4.  the results are correct.
     */
    bool ret2 = jsonValue.Put(key2, value2);
    EXPECT_FALSE(ret2);

    /**
     * @tc.steps: step5. construct value not null
     */
    const char* value3 = "v";
    bool ret3 = jsonValue.Put(key2, value3);
    EXPECT_TRUE(ret3);
    bool ret4 = jsonValue.Put(key, value3);
    EXPECT_FALSE(ret4);
}

/**
 * @tc.name: JsonUtilTest015
 * @tc.desc: Check json util function Put(const char* key, bool value) for nullptr key
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest015, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key.
     */
    const char* key = nullptr;
    bool value = true;
    JsonValue jsonValue;
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.Put(key, value);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: JsonUtilTest016
 * @tc.desc: Check json util function Put(const char* key, const std::unique_ptr<JsonValue>& value)
 * for nullptr key or nullptr value
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest016, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key.
     */
    const char* key = nullptr;
    const std::unique_ptr<JsonValue>& value = nullptr;
    JsonValue jsonValue;
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.Put(key, value);
    EXPECT_FALSE(ret);

    /**
     * @tc.steps: step1. construct the nullptr value.
     */
    const char* key2 = "5";
    const std::unique_ptr<JsonValue>& value2 = nullptr;
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret2 = jsonValue.Put(key2, value2);
    EXPECT_FALSE(ret2);

    /**
     * @tc.steps: step3. construct the value not null
     */
    const char* valueTemp = "5";
    bool ret3 = jsonValue.Put(key2, valueTemp);
    EXPECT_TRUE(ret3);
    bool ret4 = jsonValue.Put(key, valueTemp);
    EXPECT_FALSE(ret4);
}

/**
 * @tc.name: JsonUtilTest017
 * @tc.desc: Check json util function Put(const char* key, size_t value) for nullptr key
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest017, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key.
     */
    const char* key = nullptr;
    size_t value = 8;
    JsonValue jsonValue;
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.Put(key, value);
    EXPECT_FALSE(ret);

    /**
     * @tc.steps: step1. construct the nullptr value.
     */
    const char* key2 = "5";
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret2 = jsonValue.Put(key2, value);
    EXPECT_TRUE(ret2);
}

/**
 * @tc.name: JsonUtilTest018
 * @tc.desc: Check json util function Put(const char* key, int32_t value) for nullptr key
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest018, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key.
     */
    const char* key = nullptr;
    int32_t value = 8;
    JsonValue jsonValue;
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.Put(key, value);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: JsonUtilTest019
 * @tc.desc: Check json util function Put(const char* key, double value) for nullptr key
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest019, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key.
     */
    const char* key = nullptr;
    double value = 8;
    JsonValue jsonValue;
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.Put(key, value);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: JsonUtilTest020
 * @tc.desc: Check json util function Replace(const char* key, double value) for nullptr key
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest020, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key.
     */
    const char* key = nullptr;
    double value = 8.0;
    std::unique_ptr<JsonValue> jsonValue = JsonUtil::Create(false);

    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue->Replace(key, value);
    EXPECT_FALSE(ret);

    /**
     * @tc.steps: step3. construct key(not exist) and value
     * @tc.expected: step3. true
     */
    key = "aaa";
    bool ret3 = jsonValue->Replace(key, value);
    EXPECT_FALSE(ret3);

    /**
     * @tc.steps: step4. construct key(exist) and value
     * @tc.expected: step3. true
     */
    double valueTmp = 8.0;
    key = "aaa";
    bool putRet = jsonValue->Put(key, valueTmp);
    EXPECT_TRUE(putRet);
    bool ret4 = jsonValue->Replace(key, value);
    EXPECT_TRUE(ret4);
}

/**
 * @tc.name: JsonUtilTest021
 * @tc.desc: Check json util function Replace(const char* key, bool value) for nullptr key
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest021, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key.
     */
    const char* key = nullptr;
    bool value = true;
    std::unique_ptr<JsonValue> jsonValue = JsonUtil::Create(false);
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue->Replace(key, value);
    EXPECT_FALSE(ret);

    /**
     * @tc.steps: step3. construct key(not exist) and value
     * @tc.expected: step3. true
     */
    key = "aaa";
    bool ret3 = jsonValue->Replace(key, value);
    EXPECT_FALSE(ret3);

    /**
     * @tc.steps: step4. construct key(exist) and value
     * @tc.expected: step3. true
     */
    jsonValue->Put(key, false);
    bool ret4 = jsonValue->Replace(key, value);
    EXPECT_TRUE(ret4);
}

/**
 * @tc.name: JsonUtilTest022
 * @tc.desc: Check json util function Replace(const char* key, const char* value) for nullptr key
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest022, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key.
     */
    const char* key = nullptr;
    const char* value = nullptr;
    std::unique_ptr<JsonValue> jsonValue = JsonUtil::Create(false);
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue->Replace(key, value);
    EXPECT_FALSE(ret);

    /**
     * @tc.steps: step3. construct the nullptr value.
     */
    char c = '5';
    const char* value2 = &c;
    const char* key2 = nullptr;
    /**
     * @tc.steps: step4. get  results
     * @tc.expected: step4.  the results are correct.
     */
    bool ret2 = jsonValue->Replace(key2, value2);
    EXPECT_FALSE(ret2);

    /**
     * @tc.steps: step5. repalce key(not exist) and value
     * @tc.expected: step5. repalce fail
     */
    const char* key3 = "aaa";
    bool ret3 = jsonValue->Replace(key3, value2);
    EXPECT_FALSE(ret3);
    bool putRet = jsonValue->Put(key3, value2);
    EXPECT_TRUE(putRet);
    bool ret4 = jsonValue->Replace(key3, value2);
    EXPECT_TRUE(ret4);
}

/**
 * @tc.name: JsonUtilTest023
 * @tc.desc: Check json util function Replace(const char* key, int32_t value) for nullptr key
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest023, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key.
     */
    const char* key = nullptr;
    int32_t value = 5;
    std::unique_ptr<JsonValue> jsonValue = JsonUtil::Create(false);
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue->Replace(key, value);
    EXPECT_FALSE(ret);

    /**
     * @tc.steps: step3. repalce key(not exist) and value
     * @tc.expected: step3. repalce fail
     */
    const char* key3 = "aaa";
    bool ret3 = jsonValue->Replace(key3, value);
    EXPECT_FALSE(ret3);
    bool putRet = jsonValue->Put(key3, value);
    EXPECT_TRUE(putRet);
    std::string objStr = jsonValue->ToString();
    EXPECT_TRUE(objStr.find("\"aaa\":5") != std::string::npos);
    bool ret4 = jsonValue->Replace(key3, value);
    EXPECT_TRUE(ret4);
}

/**
 * @tc.name: JsonUtilTest024
 * @tc.desc: Check json util Replace(const char* key, const std::unique_ptr<JsonValue>& value) for nullptr key
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest024, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key.
     */
    const char* key = nullptr;
    const std::unique_ptr<JsonValue>& value = nullptr;
    std::unique_ptr<JsonValue> jsonValue = JsonUtil::Create(false);
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue->Replace(key, value);
    EXPECT_FALSE(ret);
    std::unique_ptr<JsonValue> valueTmp = JsonUtil::Create(false);
    bool ret2 = jsonValue->Replace(key, valueTmp);
    EXPECT_FALSE(ret2);

    /**
     * @tc.steps: step3. construct key(not exist) and value
     * @tc.expected: step3. true
     */
    key = "aaa";
    bool ret3 = jsonValue->Replace(key, valueTmp);
    EXPECT_FALSE(ret3);

    /**
     * @tc.steps: step4. construct key(exist) and value
     * @tc.expected: step3. true
     */
    jsonValue->Put(key, valueTmp);
    bool ret4 = jsonValue->Replace(key, valueTmp);
    EXPECT_TRUE(ret4);
}

/**
 * @tc.name: JsonUtilTest025
 * @tc.desc: Check json util bool Delete(const char* key) for nullptr key
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest025, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key.
     */
    const char* key = nullptr;
    JsonValue jsonValue;
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.Delete(key);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: JsonUtilTest026
 * @tc.desc: Check json util int64_t GetInt64()
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest026, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr object_.
     */
    std::unique_ptr<JsonValue> jsonValue = JsonUtil::Create(false);
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    int64_t ret = jsonValue->GetInt64();
    ASSERT_EQ(ret, 0);

    /**
     * @tc.steps: step3. get key(not exist)
     * @tc.expected: step3. reture defalut value 0
     */
    const std::string key = "key-aaa";
    int64_t ret3 = jsonValue->GetInt64(key, 0);
    ASSERT_EQ(ret3, 0);

    /**
     * @tc.steps: step4. get key(exist) but value not a number
     * @tc.expected: step4. reture defalut value 0
     */
    const char* keyPut = "key-aaa";
    const char* value = "value-bbb";
    jsonValue->Put(keyPut, value);
    int64_t ret4 = jsonValue->GetInt64(key, 0);
    ASSERT_EQ(ret4, 0);

    /**
     * @tc.steps: step5. get key(exist) and value is a number
     * @tc.expected: step5. reture value
     */
    keyPut = "key-number";
    jsonValue->Put(keyPut, 100);
    int64_t ret5 = jsonValue->GetInt64(keyPut, 0);
    ASSERT_EQ(ret5, 100);
}

/**
 * @tc.name: JsonUtilTest027
 * @tc.desc: Check json util std::unique_ptr<JsonValue> GetNext() for nullptr object_
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest027, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr object_.
     */
    JsonValue jsonValue(nullptr);
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    std::unique_ptr<JsonValue> ret = jsonValue.GetNext();
    EXPECT_TRUE(ret->IsNull());
}

/**
 * @tc.name: JsonUtilTest028
 * @tc.desc: Check json util std::unique_ptr<JsonValue> GetChild() for nullptr object_
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest028, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr object_.
     */
    JsonValue jsonValue(nullptr);
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    std::unique_ptr<JsonValue> ret = jsonValue.GetChild();
    EXPECT_TRUE(ret->IsNull());
}

/**
 * @tc.name: JsonUtilTest029
 * @tc.desc: Check json util bool Replace(const char* key, double value) for nullptr key
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest029, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key.
     */
    const char* key = nullptr;
    double value = 5;
    JsonValue jsonValue;
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.Replace(key, value);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: JsonUtilTest030
 * @tc.desc: Check json util bool Replace(const char* key, bool value) for nullptr key
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest030, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key.
     */
    const char* key = nullptr;
    bool value = true;
    JsonValue jsonValue;
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.Replace(key, value);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: JsonUtilTest031
 * @tc.desc: Check json util bool Replace(const char* key, const char* value) for nullptr key or nullptr value
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest031, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr value.
     */
    const char* key = nullptr;
    const char* value = nullptr;
    JsonValue jsonValue;
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.Replace(key, value);
    EXPECT_FALSE(ret);

    /**
     * @tc.steps: step3. construct the nullptr key.
     */
    const char* key2 = nullptr;
    char c = '5';
    const char* value2 = &c;
    /**
     * @tc.steps: step4. get  results
     * @tc.expected: step4.  the results are correct.
     */
    bool ret2 = jsonValue.Replace(key2, value2);
    EXPECT_FALSE(ret2);
}

/**
 * @tc.name: JsonUtilTest032
 * @tc.desc: Check json util bool Replace(const char* key, int32_t value) for nullptr key
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest032, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key.
     */
    const char* key = nullptr;
    int32_t value = 5;
    JsonValue jsonValue;
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.Replace(key, value);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: JsonUtilTest033
 * @tc.desc: Check json util func bool PutFixedAttr  with const char* value
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest033, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the filter object, add filter attr, construct the jsonValue object.
     */
    NG::InspectorFilter filter;
    JsonValue jsonValue;
    const std::string attr = "color";
    filter.AddFilterAttr(attr);
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.PutFixedAttr("editable", "EditMode.None", filter, NG::FIXED_ATTR_EDITABLE);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: JsonUtilTest034
 * @tc.desc: Check json util bool PutExtAttr
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest034, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the filter object, add filter attr, construct the jsonValue object.
     */
    NG::InspectorFilter filter;
    JsonValue jsonValue;
    const std::string attr = "color";
    filter.AddFilterAttr(attr);
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.PutExtAttr("editable", "EditMode.None", filter);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: JsonUtilTest035
 * @tc.desc: Check json util func bool PutFixedAttr with const std::unique_ptr<JsonValue>& value
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest035, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the filter object, add filter attr, construct the empty jsonValue object
     */
    NG::InspectorFilter filter;
    JsonValue jsonValue;
    JsonValue jsonValueTemp;
    const std::unique_ptr<JsonValue> value = std::make_unique<JsonValue>(jsonValueTemp);
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.PutFixedAttr("editable", value, filter, NG::FIXED_ATTR_EDITABLE);
    EXPECT_FALSE(ret);

    /**
     * @tc.steps: step3. set the value to jsonValueTemp, construct the jsonValue object.
     * @tc.expected: step3.  the results are correct.
     */
    std::string testJson = "true";
    std::unique_ptr<JsonValue> boolValue = JsonUtil::ParseJsonString(testJson);
    bool ret2 = jsonValue.PutFixedAttr("editable", boolValue, filter, NG::FIXED_ATTR_EDITABLE);
    EXPECT_TRUE(ret2);

    /**
     * @tc.steps: step4. construct the filter object, add filter attr, construct the jsonValue object.
     */
    const std::string attr = "color";
    filter.AddFilterAttr(attr);
    /**
     * @tc.steps: step5. get  results
     * @tc.expected: step5.  the results are correct.
     */
    bool ret3 = jsonValue.PutFixedAttr("editable", value, filter, NG::FIXED_ATTR_EDITABLE);
    EXPECT_FALSE(ret3);
}

/**
 * @tc.name: JsonUtilTest036
 * @tc.desc: Check json util func bool PutExtAttr with size_t value/int32_t value/double value/bool value/int64_t
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest036, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the filter object, add filter attr, construct the jsonValue object.
     */
    NG::InspectorFilter filter;
    JsonValue jsonValue;
    size_t value = 5;
    int32_t value2 = 5;
    double value3 = 5.0;
    int64_t value4 = 5;
    bool value5 = true;
    bool ret = jsonValue.PutExtAttr("editable", value, filter);
    bool ret2 = jsonValue.PutExtAttr("editable", value2, filter);
    bool ret3 = jsonValue.PutExtAttr("editable", value3, filter);
    bool ret4 = jsonValue.PutExtAttr("editable", value4, filter);
    bool ret5 = jsonValue.PutExtAttr("editable", value5, filter);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(ret2);
    EXPECT_TRUE(ret3);
    EXPECT_TRUE(ret4);
    EXPECT_TRUE(ret5);
    /**
     * @tc.steps: step2. make filterExt not empty
     */
    const std::string attr = "color";
    filter.AddFilterAttr(attr);
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret6 = jsonValue.PutExtAttr("editable", value, filter);
    bool ret7 = jsonValue.PutExtAttr("editable", value2, filter);
    bool ret8 = jsonValue.PutExtAttr("editable", value3, filter);
    bool ret9 = jsonValue.PutExtAttr("editable", value4, filter);
    bool ret10 = jsonValue.PutExtAttr("editable", value5, filter);
    EXPECT_FALSE(ret6);
    EXPECT_FALSE(ret7);
    EXPECT_FALSE(ret8);
    EXPECT_FALSE(ret9);
    EXPECT_FALSE(ret10);
}

/**
 * @tc.name: JsonUtilTest037
 * @tc.desc: Check json util func bool PutFixedAttr with size_t value/int32_t value/double value/bool value/int64_t
 * value
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest037, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the filter object, add filter attr, construct the empty jsonValue object
     */
    NG::InspectorFilter filter;
    JsonValue jsonValue;
    size_t value = 5;
    int32_t value2 = 5;
    double value3 = 5.0;
    int64_t value4 = 5;
    bool value5 = true;
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.PutFixedAttr("editable", value, filter, NG::FIXED_ATTR_EDITABLE);
    bool ret2 = jsonValue.PutFixedAttr("editable", value2, filter, NG::FIXED_ATTR_EDITABLE);
    bool ret3 = jsonValue.PutFixedAttr("editable", value3, filter, NG::FIXED_ATTR_EDITABLE);
    bool ret4 = jsonValue.PutFixedAttr("editable", value4, filter, NG::FIXED_ATTR_EDITABLE);
    bool ret5 = jsonValue.PutFixedAttr("editable", value5, filter, NG::FIXED_ATTR_EDITABLE);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(ret2);
    EXPECT_TRUE(ret3);
    EXPECT_TRUE(ret4);
    EXPECT_TRUE(ret5);

    /**
     * @tc.steps: step3. construct the filter object, add filter attr
     */
    const std::string attr = "color";
    filter.AddFilterAttr(attr);
    /**
     * @tc.steps: step4. get  results
     * @tc.expected: step4.  the results are correct.
     */
    bool ret6 = jsonValue.PutFixedAttr("editable", value, filter, NG::FIXED_ATTR_EDITABLE);
    bool ret7 = jsonValue.PutFixedAttr("editable", value2, filter, NG::FIXED_ATTR_EDITABLE);
    bool ret8 = jsonValue.PutFixedAttr("editable", value3, filter, NG::FIXED_ATTR_EDITABLE);
    bool ret9 = jsonValue.PutFixedAttr("editable", value4, filter, NG::FIXED_ATTR_EDITABLE);
    bool ret10 = jsonValue.PutFixedAttr("editable", value5, filter, NG::FIXED_ATTR_EDITABLE);
    EXPECT_FALSE(ret6);
    EXPECT_FALSE(ret7);
    EXPECT_FALSE(ret8);
    EXPECT_FALSE(ret9);
    EXPECT_FALSE(ret10);
}

/**
 * @tc.name: JsonUtilTest038
 * @tc.desc: Check json util func bool PutExtAttr with const std::unique_ptr<JsonValue>& value
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest038, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the filter object, add filter attr, construct the empty jsonValue object
     */
    NG::InspectorFilter filter;
    JsonValue jsonValue;
    JsonValue jsonValueTemp;
    const std::unique_ptr<JsonValue> value = std::make_unique<JsonValue>(jsonValueTemp);
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.PutExtAttr("editable", value, filter);
    EXPECT_FALSE(ret);

    /**
     * @tc.steps: step3. set the value to jsonValueTemp, construct the jsonValue object.
     * @tc.expected: step3.  the results are correct.
     */
    std::string testJson = "true";
    std::unique_ptr<JsonValue> boolValue = JsonUtil::ParseJsonString(testJson);
    bool ret2 = jsonValue.PutExtAttr("editable", boolValue, filter);
    EXPECT_TRUE(ret2);

    /**
     * @tc.steps: step4. construct the filter object, add filter attr, construct the jsonValue object.
     */
    const std::string attr = "color";
    filter.AddFilterAttr(attr);
    /**
     * @tc.steps: step5. get  results
     * @tc.expected: step5.  the results are correct.
     */
    bool ret3 = jsonValue.PutExtAttr("editable", value, filter);
    EXPECT_FALSE(ret3);
}

/**
 * @tc.name: JsonUtilTest039
 * @tc.desc: Check json util bool bool PutRef(const char* key, std::unique_ptr<JsonValue>&& value)
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest039, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct the nullptr key
     */
    JsonValue jsonValue;
    const char* key = nullptr;
    std::unique_ptr<JsonValue> value = std::make_unique<JsonValue>();
    /**
     * @tc.steps: step2. get  results
     * @tc.expected: step2.  the results are correct.
     */
    bool ret = jsonValue.PutRef(key, std::move(value));
    EXPECT_FALSE(ret);

    /**
     * @tc.steps: step3. construct the nullptr value
     */
    char a = 'a';
    const char* key2 = &a;
    std::unique_ptr<JsonValue> value2;
    /**
     * @tc.steps: step4. get  results
     * @tc.expected: step4.  the results are correct.
     */
    bool ret2 = jsonValue.PutRef(key2, std::move(value2));
    bool ret3 = jsonValue.PutRef(std::move(value2));
    EXPECT_FALSE(ret2);
    EXPECT_FALSE(ret3);
}

/**
 * @tc.name: JsonUtilTest040
 * @tc.desc: Check json util int64_t GetUInt/GetInt/GetString
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest040, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct jsonValue
     */
    std::unique_ptr<JsonValue> jsonValue = JsonUtil::Create(false);

    /**
     * @tc.steps: step2. get key(not exist)
     * @tc.expected: step2. reture defalut value 0
     */
    const std::string key = "key-aaa";
    int64_t ret3 = jsonValue->GetUInt(key, 0);
    ASSERT_EQ(ret3, 0);

    /**
     * @tc.steps: step3. get key(exist) but value not a number
     * @tc.expected: step3. reture defalut value 0
     */
    const char* keyPut = "key-aaa";
    const char* value = "value-bbb";
    jsonValue->Put(keyPut, value);
    int64_t ret4 = jsonValue->GetUInt(key, 0);
    int64_t ret42 = jsonValue->GetInt(key, 0);
    ASSERT_EQ(ret4, 0);
    ASSERT_EQ(ret42, 0);

    /**
     * @tc.steps: step5. get key(exist) and value is a number
     * @tc.expected: step5. reture value
     */
    keyPut = "key-number";
    jsonValue->Put(keyPut, 100);
    int64_t ret5 = jsonValue->GetUInt(keyPut, 0);
    ASSERT_EQ(ret5, 100);
    std::string ret52 = jsonValue->GetString(keyPut, "default");
    ASSERT_EQ(ret52, "default");
}

/**
 * @tc.name: JsonUtilTest041
 * @tc.desc: Check json util Put(const std::unique_ptr<JsonValue>& value)/ReleaseJsonObject
 * @tc.type: FUNC
 */
HWTEST_F(JsonUtilTest, JsonUtilTest041, TestSize.Level1)
{
    std::unique_ptr<JsonValue> value = nullptr;
    JsonValue jsonValue;
    ASSERT_FALSE(jsonValue.Put(value));

    // ReleaseJsonObject() isRoot_ false
    JsonObject* ret = jsonValue.ReleaseJsonObject();
    ASSERT_TRUE(ret == nullptr);
    // PutRef() isRoot_ true
    const char* keyPtr = "k";
    std::unique_ptr<JsonValue> value2 = JsonUtil::Create(true);
    bool ret2 = jsonValue.PutRef(keyPtr, std::move(value2));
    ASSERT_TRUE(ret2);
    bool ret3 = jsonValue.PutRef(std::move(value2));
    ASSERT_TRUE(ret3);
}
} // namespace OHOS::Ace

namespace OHOS::Ace {
namespace {
constexpr uintptr_t OUTPUT_POINTER_SENTINEL = 1;
constexpr uint32_t OUTPUT_SIZE_SENTINEL = 99;
constexpr size_t TEST_JSON_BYTE_LIMIT = 4096;
constexpr size_t MAX_TREE_BYTE_LIMIT = 5 * 1024 * 1024;
constexpr int MAX_TEST_TREE_DEPTH = 5000;
constexpr int TREE_SCHEMA_VERSION = 1;
#ifdef ACE_HOST_PRODUCT
constexpr int ALLOCATION_FAILURE_DISABLED = -1;
constexpr int SUBTREE_ALLOCATION_COUNT = 5;
constexpr int32_t REPLACEMENT_NUMBER = 42;
constexpr char REPLACEMENT_STRING[] = "replacement";
// cJSON hooks are process-global: these fault tests run sequentially and restore the allocator.
int jsonAllocationCountdown = ALLOCATION_FAILURE_DISABLED;
size_t jsonTrackedAllocations = 0;
size_t jsonTrackedFrees = 0;
void* FailJsonAllocation(size_t size)
{
    if (jsonAllocationCountdown == 0) {
        jsonAllocationCountdown = ALLOCATION_FAILURE_DISABLED; // fail once, so cleanup and the next request can proceed
        return nullptr;
    }
    if (jsonAllocationCountdown > 0) {
        --jsonAllocationCountdown;
    }
    auto* allocation = std::malloc(size);
    if (allocation) {
        ++jsonTrackedAllocations;
    }
    return allocation;
}
void FreeJsonAllocation(void* value)
{
    if (value) {
        ++jsonTrackedFrees;
    }
    std::free(value);
}
class JsonAllocationFailure final {
public:
    explicit JsonAllocationFailure(int countdown)
    {
        jsonAllocationCountdown = countdown;
        jsonTrackedAllocations = 0;
        jsonTrackedFrees = 0;
        cJSON_Hooks hooks { FailJsonAllocation, FreeJsonAllocation };
        cJSON_InitHooks(&hooks);
    }
    ~JsonAllocationFailure()
    {
        cJSON_InitHooks(nullptr);
    }
};

template<typename Replace>
void CheckReplaceAllocationFailures(Replace replace)
{
    size_t allocationCount = 0;
    {
        auto object = JsonUtil::Create();
        ASSERT_TRUE(object->Put(TEST_KEY.c_str(), TEST_STRING.c_str()));
        JsonAllocationFailure allocations(ALLOCATION_FAILURE_DISABLED);
        ASSERT_TRUE(replace(*object));
        allocationCount = jsonTrackedAllocations;
    }
    ASSERT_GT(allocationCount, 0u);
    auto object = JsonUtil::Create();
    ASSERT_TRUE(object->Put(TEST_KEY.c_str(), TEST_STRING.c_str()));
    for (size_t allocation = 0; allocation < allocationCount; ++allocation) {
        SCOPED_TRACE(allocation);
        JsonWriteErrorScope scope;
        JsonAllocationFailure fault(static_cast<int>(allocation));
        EXPECT_FALSE(replace(*object));
        EXPECT_TRUE(scope.HasError());
        EXPECT_EQ(object->GetString(TEST_KEY), TEST_STRING);
        EXPECT_EQ(jsonAllocationCountdown, ALLOCATION_FAILURE_DISABLED);
        EXPECT_EQ(jsonTrackedAllocations, jsonTrackedFrees);
    }
    JsonWriteErrorScope recovered;
    EXPECT_TRUE(replace(*object));
    EXPECT_FALSE(recovered.HasError());
}

template<typename Put>
void CheckPutAllocationFailures(Put put)
{
    size_t allocationCount = 0;
    {
        auto object = JsonUtil::Create();
        JsonAllocationFailure allocations(ALLOCATION_FAILURE_DISABLED);
        ASSERT_TRUE(put(*object));
        allocationCount = jsonTrackedAllocations;
    }
    ASSERT_GT(allocationCount, 0u);
    auto object = JsonUtil::Create();
    for (size_t allocation = 0; allocation < allocationCount; ++allocation) {
        SCOPED_TRACE(allocation);
        JsonWriteErrorScope scope;
        JsonAllocationFailure fault(static_cast<int>(allocation));
        // The final allocation duplicates the key: attachment failure historically returns true.
        EXPECT_EQ(put(*object), allocation + 1 == allocationCount);
        EXPECT_TRUE(scope.HasError());
        EXPECT_FALSE(object->Contains(TEST_KEY));
        EXPECT_EQ(jsonAllocationCountdown, ALLOCATION_FAILURE_DISABLED);
        EXPECT_EQ(jsonTrackedAllocations, jsonTrackedFrees);
    }
    JsonWriteErrorScope recovered;
    EXPECT_TRUE(put(*object));
    EXPECT_TRUE(object->Contains(TEST_KEY));
    EXPECT_FALSE(recovered.HasError());
}
#endif

std::string BoundedOutput(const JsonValue& value, bool pretty, size_t limit, JsonWriteResult expected)
{
    char* data = reinterpret_cast<char*>(OUTPUT_POINTER_SENTINEL);
    size_t size = OUTPUT_SIZE_SENTINEL;
    auto result = WriteJsonBounded(value, pretty, limit, &data, &size);
    EXPECT_EQ(result, expected);
    if (result != JsonWriteResult::SUCCESS) {
        EXPECT_EQ(data, nullptr);
        EXPECT_EQ(size, 0u);
        return "";
    }
    EXPECT_NE(data, nullptr);
    EXPECT_EQ(data[size], '\0');
    std::string output(data, size);
    std::free(data);
    return output;
}
} // namespace

#ifdef ACE_HOST_PRODUCT
HWTEST_F(JsonUtilTest, EmptyTreeSerializationRejectsEveryJsonAllocationFailure, TestSize.Level1)
{
    auto empty = JsonUtil::CreateSharedPtrJson();
    for (bool pretty : { false, true }) {
        size_t allocationCount = 0;
        {
            JsonAllocationFailure allocations(ALLOCATION_FAILURE_DISABLED);
            char* data = nullptr;
            uint32_t size = 0;
            EXPECT_EQ(NG::SerializeUITree(empty, pretty, &data, &size), ARKUI_ERROR_CODE_NO_ERROR);
            EXPECT_NE(data, nullptr);
            std::free(data);
            allocationCount = jsonTrackedAllocations;
            EXPECT_EQ(jsonTrackedAllocations, jsonTrackedFrees);
        }
        ASSERT_GT(allocationCount, 0u);
        // Sweep actual allocations, including creation of the treeInfo null value.
        for (size_t allocation = 0; allocation < allocationCount; ++allocation) {
            SCOPED_TRACE(allocation);
            JsonAllocationFailure fault(static_cast<int>(allocation));
            char* data = nullptr;
            uint32_t size = OUTPUT_SIZE_SENTINEL;
            EXPECT_EQ(NG::SerializeUITree(empty, pretty, &data, &size), ARKUI_ERROR_CODE_RESOURCE_EXHAUSTED);
            EXPECT_EQ(data, nullptr);
            EXPECT_EQ(size, 0u);
            EXPECT_EQ(jsonAllocationCountdown, ALLOCATION_FAILURE_DISABLED);
            EXPECT_EQ(jsonTrackedAllocations, jsonTrackedFrees);
            std::free(data);
        }
        char* data = nullptr;
        uint32_t size = 0;
        ASSERT_EQ(NG::SerializeUITree(empty, pretty, &data, &size), ARKUI_ERROR_CODE_NO_ERROR);
        ASSERT_NE(data, nullptr);
        auto result = JsonUtil::ParseJsonString(data);
        std::free(data);
        EXPECT_EQ(result->GetInt("schemaVersion"), TREE_SCHEMA_VERSION);
        EXPECT_TRUE(result->Contains("treeInfo"));
        EXPECT_TRUE(result->GetValue("treeInfo")->IsNull());
    }
}

HWTEST_F(JsonUtilTest, WriteFailureScopeTracksCreateCopyAndKeyAllocation, TestSize.Level1)
{
    auto parent = JsonUtil::Create();
    auto source = JsonUtil::Create();
    source->Put("text", "value");
    // Number allocation succeeds, then allocation of its object key fails.
    {
        JsonWriteErrorScope scope;
        JsonAllocationFailure fault(1);
        EXPECT_TRUE(parent->Put("number", 1));
        EXPECT_TRUE(scope.HasError());
        EXPECT_FALSE(parent->Contains("number"));
        EXPECT_EQ(jsonTrackedAllocations, jsonTrackedFrees);
    }
    {
        JsonWriteErrorScope scope;
        JsonAllocationFailure fault(0);
        auto value = JsonUtil::CreateArray();
        EXPECT_FALSE(value->IsValid());
        EXPECT_TRUE(scope.HasError());
    }
    // Exercise every allocation in a small subtree duplication and its final object key.
    for (int allocation = 0; allocation < SUBTREE_ALLOCATION_COUNT; ++allocation) {
        JsonWriteErrorScope scope;
        JsonAllocationFailure fault(allocation);
        EXPECT_EQ(parent->Put("subtree", source), allocation + 1 == SUBTREE_ALLOCATION_COUNT);
        EXPECT_TRUE(scope.HasError());
        EXPECT_FALSE(parent->Contains("subtree"));
        EXPECT_EQ(jsonTrackedAllocations, jsonTrackedFrees);
    }
    JsonWriteErrorScope recovered;
    EXPECT_TRUE(parent->Put("subtree", source));
    EXPECT_FALSE(recovered.HasError());
}

HWTEST_F(JsonUtilTest, PutPreservesLegacyReturnValuesOnAllocationFailure, TestSize.Level1)
{
    CheckPutAllocationFailures([](JsonValue& object) {
        return object.Put(TEST_KEY.c_str(), TEST_STRING.c_str());
    });
    CheckPutAllocationFailures([](JsonValue& object) {
        return object.Put(TEST_KEY.c_str(), true);
    });
    CheckPutAllocationFailures([](JsonValue& object) {
        return object.Put(TEST_KEY.c_str(), REPLACEMENT_NUMBER);
    });
    CheckPutAllocationFailures([](JsonValue& object) {
        return object.Put(TEST_KEY.c_str(), static_cast<size_t>(REPLACEMENT_NUMBER));
    });
    CheckPutAllocationFailures([](JsonValue& object) {
        return object.Put(TEST_KEY.c_str(), static_cast<double>(REPLACEMENT_NUMBER));
    });
    auto value = JsonUtil::Create();
    ASSERT_TRUE(value->Put(TEST_KEY.c_str(), TEST_STRING.c_str()));
    CheckPutAllocationFailures([&value](JsonValue& object) {
        return object.Put(TEST_KEY.c_str(), value);
    });
    auto sharedValue = JsonUtil::CreateSharedPtrJson();
    ASSERT_TRUE(sharedValue->Put(TEST_KEY.c_str(), TEST_STRING.c_str()));
    CheckPutAllocationFailures([&sharedValue](JsonValue& object) {
        return object.Put(TEST_KEY.c_str(), sharedValue);
    });
}

HWTEST_F(JsonUtilTest, WriteFailureScopeTracksReplaceAllocations, TestSize.Level1)
{
    CheckReplaceAllocationFailures([](JsonValue& object) {
        return object.Replace(TEST_KEY.c_str(), true);
    });
    CheckReplaceAllocationFailures([](JsonValue& object) {
        return object.Replace(TEST_KEY.c_str(), REPLACEMENT_NUMBER);
    });
    CheckReplaceAllocationFailures([](JsonValue& object) {
        return object.Replace(TEST_KEY.c_str(), static_cast<double>(REPLACEMENT_NUMBER));
    });
    CheckReplaceAllocationFailures([](JsonValue& object) {
        return object.Replace(TEST_KEY.c_str(), REPLACEMENT_STRING);
    });
    auto subtree = JsonUtil::Create();
    ASSERT_TRUE(subtree->Put(TEST_KEY.c_str(), TEST_STRING.c_str()));
    CheckReplaceAllocationFailures([&subtree](JsonValue& object) {
        return object.Replace(TEST_KEY.c_str(), subtree);
    });
}

HWTEST_F(JsonUtilTest, WriteFailureScopeIgnoresMissingReplaceTargets, TestSize.Level1)
{
    auto object = JsonUtil::Create();
    auto subtree = JsonUtil::Create();
    ASSERT_TRUE(object->Put(TEST_KEY.c_str(), TEST_STRING.c_str()));
    ASSERT_TRUE(subtree->Put(TEST_KEY.c_str(), TEST_STRING.c_str()));
    JsonWriteErrorScope scope;
    JsonAllocationFailure allocations(ALLOCATION_FAILURE_DISABLED);
    EXPECT_FALSE(object->Replace(TEST_FALSE_KEY.c_str(), true));
    EXPECT_FALSE(object->Replace(TEST_FALSE_KEY.c_str(), REPLACEMENT_NUMBER));
    EXPECT_FALSE(object->Replace(TEST_FALSE_KEY.c_str(), static_cast<double>(REPLACEMENT_NUMBER)));
    EXPECT_FALSE(object->Replace(TEST_FALSE_KEY.c_str(), REPLACEMENT_STRING));
    EXPECT_FALSE(object->Replace(TEST_FALSE_KEY.c_str(), subtree));
    EXPECT_FALSE(scope.HasError());
    EXPECT_EQ(object->GetString(TEST_KEY), TEST_STRING);
    EXPECT_EQ(jsonTrackedAllocations, jsonTrackedFrees);
}

HWTEST_F(JsonUtilTest, WriteFailureScopeIgnoresInvalidReplaceArguments, TestSize.Level1)
{
    auto object = JsonUtil::Create();
    auto subtree = JsonUtil::Create();
    auto invalidValue = std::make_unique<JsonValue>();
    std::unique_ptr<JsonValue> emptyValue;
    JsonWriteErrorScope scope;
    JsonAllocationFailure fault(0);
    EXPECT_FALSE(object->Replace(nullptr, true));
    EXPECT_FALSE(object->Replace(nullptr, REPLACEMENT_NUMBER));
    EXPECT_FALSE(object->Replace(nullptr, static_cast<double>(REPLACEMENT_NUMBER)));
    EXPECT_FALSE(object->Replace(nullptr, REPLACEMENT_STRING));
    EXPECT_FALSE(object->Replace(nullptr, subtree));
    EXPECT_FALSE(object->Replace(TEST_KEY.c_str(), static_cast<const char*>(nullptr)));
    EXPECT_FALSE(object->Replace(TEST_KEY.c_str(), emptyValue));
    EXPECT_FALSE(object->Replace(TEST_KEY.c_str(), invalidValue));
    EXPECT_FALSE(scope.HasError());
    EXPECT_EQ(jsonAllocationCountdown, 0);
    EXPECT_EQ(jsonTrackedAllocations, 0u);
}

HWTEST_F(JsonUtilTest, WriteFailureScopeIgnoresInvalidArguments, TestSize.Level1)
{
    auto object = JsonUtil::Create();
    auto value = JsonUtil::Create();
    auto sharedValue = JsonUtil::CreateSharedPtrJson();
    auto nullValue = JsonUtil::ParseJsonString("null");
    std::unique_ptr<JsonValue> emptyValue;
    std::shared_ptr<JsonValue> emptySharedValue;
    JsonWriteErrorScope scope;
    JsonAllocationFailure fault(0);
    EXPECT_FALSE(object->Put(nullptr, "value"));
    EXPECT_FALSE(object->Put("key", static_cast<const char*>(nullptr)));
    EXPECT_FALSE(object->Put(nullptr, value));
    EXPECT_FALSE(object->Put("key", emptyValue));
    EXPECT_FALSE(object->Put(nullptr, sharedValue));
    EXPECT_FALSE(object->Put("key", emptySharedValue));
    EXPECT_FALSE(object->Put(emptyValue));
    EXPECT_FALSE(object->Put(emptySharedValue));
    EXPECT_FALSE(object->Put(nullptr, static_cast<size_t>(1)));
    EXPECT_FALSE(object->Put(nullptr, static_cast<int32_t>(1)));
    EXPECT_FALSE(object->Put(nullptr, 1.0));
    EXPECT_FALSE(object->Put(nullptr, true));
    EXPECT_FALSE(object->PutRef(nullptr, std::move(value)));
    EXPECT_NE(value, nullptr);
    EXPECT_FALSE(object->PutRef("key", std::move(emptyValue)));
    EXPECT_FALSE(object->PutRef(std::move(emptyValue)));
    EXPECT_FALSE(object->PutRef(std::move(emptySharedValue)));
    EXPECT_FALSE(PutJsonObjectReference(*object, nullptr, *sharedValue));
    EXPECT_FALSE(PutJsonObjectReference(*object, "key", *nullValue));
    EXPECT_FALSE(scope.HasError());
    EXPECT_EQ(jsonAllocationCountdown, 0); // Invalid arguments must not attempt cJSON allocation.

    EXPECT_FALSE(object->Put("key", true)); // A real allocation failure is still recorded.
    EXPECT_TRUE(scope.HasError());
    EXPECT_EQ(jsonAllocationCountdown, ALLOCATION_FAILURE_DISABLED);
}

HWTEST_F(JsonUtilTest, WriteFailureScopeTracksOwnershipTransfer, TestSize.Level1)
{
    auto parent = JsonUtil::Create();
    JsonWriteErrorScope scope;
    JsonAllocationFailure fault(ALLOCATION_FAILURE_DISABLED);
    auto value = JsonUtil::Create();
    value->Put("leaf", true);
    jsonAllocationCountdown = 0; // only the destination key allocation should fail
    EXPECT_TRUE(parent->PutRef("child", std::move(value)));
    EXPECT_TRUE(scope.HasError());
    EXPECT_FALSE(parent->Contains("child"));
    EXPECT_EQ(jsonTrackedAllocations, jsonTrackedFrees);
}

HWTEST_F(JsonUtilTest, WriteFailureScopePreservesEmptyDestinationCompatibility, TestSize.Level1)
{
    JsonAllocationFailure allocations(ALLOCATION_FAILURE_DISABLED);
    JsonWriteErrorScope scope;
    {
        JsonValue empty;
        EXPECT_TRUE(empty.Put("number", 1));
        EXPECT_TRUE(empty.Put(1.0));
        EXPECT_TRUE(empty.Put("text", "value"));
        auto value = JsonUtil::Create();
        EXPECT_TRUE(value->Put("leaf", true));
        EXPECT_TRUE(empty.Put("copy", value));
        EXPECT_TRUE(empty.PutRef("moved", std::move(value)));
        EXPECT_TRUE(value->IsNull());
        EXPECT_TRUE(empty.PutRef(std::move(value)));
        auto parent = JsonUtil::Create();
        EXPECT_TRUE(parent->PutRef("released", std::move(value)));
        EXPECT_FALSE(parent->Contains("released"));
        EXPECT_TRUE(empty.IsNull());
        EXPECT_FALSE(scope.HasError());
    }
    EXPECT_EQ(jsonTrackedAllocations, jsonTrackedFrees);
}

HWTEST_F(JsonUtilTest, WriteFailureScopeNestingAndOptionalReads, TestSize.Level1)
{
    auto object = JsonUtil::Create();
    JsonWriteErrorScope outer;
    EXPECT_FALSE(object->GetValue("absent")->IsValid());
    EXPECT_FALSE(outer.HasError());
    {
        JsonWriteErrorScope inner;
        JsonAllocationFailure fault(0);
        object->Put("lost", true); // deliberately ignore the return as existing property readers do
        EXPECT_TRUE(inner.HasError());
    }
    EXPECT_TRUE(outer.HasError());
    JsonWriteErrorScope independent;
    EXPECT_TRUE(object->Put("present", true));
    EXPECT_FALSE(independent.HasError());
}

#endif

HWTEST_F(JsonUtilTest, BoundedJsonFormatsEscapesAndNativeTypes, TestSize.Level1)
{
    constexpr char JSON_TYPES_SAMPLE[] = R"({"s":"中\"\\\n\t\b\f\r\u0001",)"
                                         R"("n":1.2345678901234567,"b":true,"a":[null,false,{},[]]})";
    auto object = JsonUtil::ParseJsonString(JSON_TYPES_SAMPLE);
    auto compact = BoundedOutput(*object, false, TEST_JSON_BYTE_LIMIT, JsonWriteResult::SUCCESS);
    auto pretty = BoundedOutput(*object, true, TEST_JSON_BYTE_LIMIT, JsonWriteResult::SUCCESS);
    EXPECT_EQ(JsonUtil::ParseJsonString(compact)->ToString(), object->ToString());
    EXPECT_EQ(JsonUtil::ParseJsonString(pretty)->ToString(), object->ToString());
    EXPECT_EQ(compact, object->ToString());
#ifdef ACE_HOST_PRODUCT
    auto* nativePretty = cJSON_Print(object->GetJsonObject());
    ASSERT_NE(nativePretty, nullptr);
    EXPECT_EQ(pretty, nativePretty);
    cJSON_free(nativePretty);
#endif
}

HWTEST_F(JsonUtilTest, BoundedJsonPreservesUnkeyedObjectHistory, TestSize.Level1)
{
    constexpr const char* HISTORY_RECORDS[] = { "first", "second", "third" };
    auto object = JsonUtil::Create();
    for (const auto* record : HISTORY_RECORDS) {
        auto entry = JsonUtil::Create();
        ASSERT_TRUE(entry->Put("record", record));
        ASSERT_TRUE(object->PutRef(std::move(entry)));
    }
    const auto legacy = object->ToString();
    for (bool pretty : { false, true }) {
        auto output = BoundedOutput(*object, pretty, TEST_JSON_BYTE_LIMIT, JsonWriteResult::SUCCESS);
        auto parsed = JsonUtil::ParseJsonString(output);
        ASSERT_TRUE(parsed->IsObject());
        EXPECT_EQ(parsed->ToString(), legacy);
        auto child = parsed->GetChild();
        for (const auto* record : HISTORY_RECORDS) {
            ASSERT_TRUE(child->IsValid());
            EXPECT_EQ(child->GetString("record"), record);
            child = child->GetNext();
        }
        EXPECT_FALSE(child->IsValid());
        EXPECT_EQ(object->ToString(), legacy);
    }
}

HWTEST_F(JsonUtilTest, BoundedJsonPreservesMixedObjectKeys, TestSize.Level1)
{
    auto object = JsonUtil::Create();
    ASSERT_TRUE(object->Put("named", "normal"));
    ASSERT_TRUE(object->Put("", "explicit empty key"));
    auto entry = JsonUtil::Create();
    ASSERT_TRUE(entry->Put("record", "unkeyed"));
    ASSERT_TRUE(object->PutRef(std::move(entry)));
    constexpr char EXPECTED_JSON[] = R"({"named":"normal","":"explicit empty key","":{"record":"unkeyed"}})";
    ASSERT_EQ(object->ToString(), EXPECTED_JSON);
    for (bool pretty : { false, true }) {
        auto output = BoundedOutput(*object, pretty, TEST_JSON_BYTE_LIMIT, JsonWriteResult::SUCCESS);
        auto parsed = JsonUtil::ParseJsonString(output);
        ASSERT_TRUE(parsed->IsObject());
        EXPECT_EQ(parsed->ToString(), EXPECTED_JSON);
        EXPECT_EQ(object->ToString(), EXPECTED_JSON);
    }
}

HWTEST_F(JsonUtilTest, BoundedJsonUnkeyedObjectRespectsOutputLimit, TestSize.Level1)
{
    auto object = JsonUtil::Create();
    auto entry = JsonUtil::Create();
    ASSERT_TRUE(entry->Put("record", "history"));
    ASSERT_TRUE(object->PutRef(std::move(entry)));
    for (bool pretty : { false, true }) {
        auto output = BoundedOutput(*object, pretty, TEST_JSON_BYTE_LIMIT, JsonWriteResult::SUCCESS);
        ASSERT_FALSE(output.empty());
        EXPECT_EQ(BoundedOutput(*object, pretty, output.size(), JsonWriteResult::SUCCESS), output);
        BoundedOutput(*object, pretty, output.size() - 1, JsonWriteResult::TOO_LARGE);
    }
}

HWTEST_F(JsonUtilTest, BoundedJsonExactLimitsAndPrettyExpansion, TestSize.Level1)
{
    for (bool pretty : { false, true }) {
        auto object = JsonUtil::Create();
        object->Put("s", "");
        auto overhead = BoundedOutput(*object, pretty, MAX_TREE_BYTE_LIMIT, JsonWriteResult::SUCCESS).size();
        std::string text(MAX_TREE_BYTE_LIMIT - overhead, 'x');
        object->Replace("s", text.c_str());
        EXPECT_EQ(
            BoundedOutput(*object, pretty, MAX_TREE_BYTE_LIMIT, JsonWriteResult::SUCCESS).size(), MAX_TREE_BYTE_LIMIT);
        BoundedOutput(*object, pretty, MAX_TREE_BYTE_LIMIT - 1, JsonWriteResult::TOO_LARGE);
    }
    auto object = JsonUtil::ParseJsonString(R"({"a":[1,2,3]})");
    auto compact = BoundedOutput(*object, false, MAX_TREE_BYTE_LIMIT, JsonWriteResult::SUCCESS);
    BoundedOutput(*object, true, compact.size(), JsonWriteResult::TOO_LARGE);
    BoundedOutput(*object, false, 0, JsonWriteResult::TOO_LARGE);
}

HWTEST_F(JsonUtilTest, BoundedJsonDeepTreeAndBorrowedEnvelope, TestSize.Level1)
{
    auto root = JsonUtil::Create();
    root->Put("$type", "Text");
    for (int depth = 1; depth < MAX_TEST_TREE_DEPTH; ++depth) {
        auto parent = JsonUtil::Create();
        parent->Put("$type", "Text");
        auto children = JsonUtil::CreateArray();
        children->PutRef(std::move(root));
        parent->PutRef("$children", std::move(children));
        root = std::move(parent);
    }
    {
        auto envelope = JsonUtil::Create();
        envelope->Put("schemaVersion", TREE_SCHEMA_VERSION);
        ASSERT_TRUE(PutJsonObjectReference(*envelope, "treeInfo", *root));
        auto compact = BoundedOutput(*envelope, false, MAX_TREE_BYTE_LIMIT, JsonWriteResult::SUCCESS);
        EXPECT_EQ(std::count(compact.begin(), compact.end(), '['), MAX_TEST_TREE_DEPTH - 1);
        BoundedOutput(*envelope, true, MAX_TREE_BYTE_LIMIT, JsonWriteResult::TOO_LARGE);
    }
    EXPECT_EQ(root->GetString("$type"), "Text"); // destroying the envelope did not consume the tree
}

HWTEST_F(JsonUtilTest, CompactMeasurementMatchesNativeOutput, TestSize.Level1)
{
    constexpr const char* SAMPLES[] = {
        R"({"s":"中\"\\\n\t\b\f\r\u0001","a":[null,false,true,{},[]]})",
        R"({"\"\u0002":"value","n":1.2345678901234567,"small":1e-300,"large":1e300})",
        R"([0,-0.0,-2147483648,2147483647,0.1])", R"("")", "{}", "[]", "true", "false", "null"
    };
    for (const auto* sample : SAMPLES) {
        auto value = JsonUtil::ParseJsonString(sample);
        auto output = BoundedOutput(*value, false, TEST_JSON_BYTE_LIMIT, JsonWriteResult::SUCCESS);
        size_t size = 0;
        EXPECT_EQ(MeasureCompactJson(*value, output.size(), size), JsonWriteResult::SUCCESS);
        EXPECT_EQ(size, output.size());
        EXPECT_EQ(MeasureCompactJson(*value, output.size() - 1, size), JsonWriteResult::TOO_LARGE);
        EXPECT_EQ(size, 0u);
    }
}

HWTEST_F(JsonUtilTest, CompactMeasurementCountsOnlyImmediateChildStructure, TestSize.Level1)
{
    auto root = JsonUtil::ParseJsonString(
        R"({"$type":"Root","attrs":{"$children":["included"]},"$children":[{"v":1},{"v":2}]})");
    auto children = root->GetValue("$children");
    size_t total = 0;
    ASSERT_EQ(MeasureCompactJson(*root, TEST_JSON_BYTE_LIMIT, total, "$children"), JsonWriteResult::SUCCESS);
    for (auto child = children->GetChild(); child->IsValid(); child = child->GetNext()) {
        size_t size = 0;
        ASSERT_EQ(MeasureCompactJson(*child, TEST_JSON_BYTE_LIMIT, size), JsonWriteResult::SUCCESS);
        total += size;
    }
    EXPECT_EQ(total, BoundedOutput(*root, false, TEST_JSON_BYTE_LIMIT, JsonWriteResult::SUCCESS).size());
}

HWTEST_F(JsonUtilTest, CompactMeasurementHandlesUnkeyedMembersAndInvalidValues, TestSize.Level1)
{
    auto root = JsonUtil::Create();
    auto child = JsonUtil::Create();
    ASSERT_TRUE(child->Put("value", "unkeyed"));
    ASSERT_TRUE(root->PutRef(std::move(child)));
    size_t size = 0;
    EXPECT_EQ(MeasureCompactJson(*root, TEST_JSON_BYTE_LIMIT, size), JsonWriteResult::SUCCESS);
    EXPECT_EQ(size, BoundedOutput(*root, false, TEST_JSON_BYTE_LIMIT, JsonWriteResult::SUCCESS).size());
    JsonValue invalid;
    EXPECT_EQ(MeasureCompactJson(invalid, TEST_JSON_BYTE_LIMIT, size), JsonWriteResult::INVALID_VALUE);
    EXPECT_EQ(size, 0u);
    root->Put("invalid", std::numeric_limits<double>::infinity());
    EXPECT_EQ(MeasureCompactJson(*root, TEST_JSON_BYTE_LIMIT, size), JsonWriteResult::INVALID_VALUE);
    EXPECT_EQ(size, 0u);
}

#ifdef ACE_HOST_PRODUCT
HWTEST_F(JsonUtilTest, CompactMeasurementDoesNotAllocate, TestSize.Level1)
{
    auto root = JsonUtil::ParseJsonString(R"({"s":"中\n","n":1.2345678901234567,"a":[true,null]})");
    const auto expected = root->ToString().size();
    JsonAllocationFailure allocations(ALLOCATION_FAILURE_DISABLED);
    size_t size = 0;
    EXPECT_EQ(MeasureCompactJson(*root, TEST_JSON_BYTE_LIMIT, size), JsonWriteResult::SUCCESS);
    EXPECT_EQ(size, expected);
    EXPECT_EQ(jsonTrackedAllocations, 0u);
}
#endif

HWTEST_F(JsonUtilTest, BoundedJsonInvalidValueClearsOutput, TestSize.Level1)
{
    JsonValue invalid;
    BoundedOutput(invalid, false, TEST_JSON_BYTE_LIMIT, JsonWriteResult::INVALID_VALUE);
    auto object = JsonUtil::Create();
    object->Put("number", std::numeric_limits<double>::infinity());
    BoundedOutput(*object, false, TEST_JSON_BYTE_LIMIT, JsonWriteResult::INVALID_VALUE);
}
} // namespace OHOS::Ace
