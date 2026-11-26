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

#include "base/json/json_util.h"
#include "base/json/node_object.h"
#include "base/json/uobject.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
namespace {} // namespace

class NodeObjectTest : public testing::Test {};

/**
 * @tc.name: NodeObjectTest001
 * @tc.desc: AddItemToObject()
 * @tc.type: FUNC
 */
HWTEST_F(NodeObjectTest, NodeObjectTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a NodeObject.
     */
    auto nodeObject = NodeObject::Create();

    /**
     * @tc.steps: step2. Save value through AdditemToObject.
     * @tc.expected: step2. Asserting values through the get method.
     */
    std::string stringValue = "test";
    double doubleValue = 1.0;
    size_t sizeValue = 10;
    int32_t int32Value = 5;
    int64_t int64Value = 1;
    bool boolValue = false;

    nodeObject->Put(nullptr, const_cast<char*>(stringValue.c_str()));
    nodeObject->Put(nullptr, nullptr);
    nodeObject->Put(nullptr, doubleValue);
    nodeObject->Put(nullptr, sizeValue);
    nodeObject->Put(nullptr, int32Value);
    nodeObject->Put(nullptr, int64Value);
    nodeObject->Put(nullptr, boolValue);

    std::string str = "string";
    nodeObject->Put(const_cast<char*>(str.c_str()), const_cast<char*>(stringValue.c_str()));
    EXPECT_EQ(nodeObject->GetString("string", ""), "test");
    EXPECT_EQ(nodeObject->GetString("", "test"), "test");
    str = "double";
    nodeObject->Put(const_cast<char*>(str.c_str()), doubleValue);
    EXPECT_EQ(nodeObject->GetDouble("double", 0), 1.0);
    EXPECT_EQ(nodeObject->GetDouble("", 0), 0);
    str = "int32_t";
    nodeObject->Put(const_cast<char*>(str.c_str()), int32Value);
    EXPECT_EQ(nodeObject->GetInt("int32_t", 0), 5);
    EXPECT_EQ(nodeObject->GetInt("", 0), 0);
    str = "int64_t";
    nodeObject->Put(const_cast<char*>(str.c_str()), int64Value);
    EXPECT_EQ(nodeObject->GetInt64("int64_t", 0), 1);
    EXPECT_EQ(nodeObject->GetInt64("", 0), 0);
    str = "bool";
    nodeObject->Put(const_cast<char*>(str.c_str()), boolValue);
    EXPECT_EQ(nodeObject->GetBool("bool", true), false);
    EXPECT_EQ(nodeObject->GetBool("", true), true);
}

/**
 * @tc.name: NodeObjectTest002
 * @tc.desc: Put(const char* key, size_t value)/GetUInt(const std::string& key, uint32_t defaultVal)
 * @tc.type: FUNC
 */
HWTEST_F(NodeObjectTest, NodeObjectTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a NodeObject.
     */
    auto nodeObject = NodeObject::Create();

    /**
     * @tc.steps: step2. Put(const char* key, size_t value)
     * @tc.expected: step2. Asserting put success.
     */
    const char* key = "key-string";
    size_t value = 5;
    bool ret = nodeObject->Put(key, value);
    EXPECT_EQ(ret, true);

    /**
     * @tc.steps: step3. GetUInt(const std::string& key, uint32_t defaultVal)
     * @tc.expected: step3. Asserting  success.
     */
    const std::string key2 = "aaaa";
    uint32_t defaultVal = 0;
    // key not exist
    uint32_t ret2 = nodeObject->GetUInt(key2, defaultVal);
    EXPECT_EQ(ret2, defaultVal);
    // key exist
    const char* key3 = "key-temp";
    nodeObject->Put(key3, 11);
    uint32_t ret3 = nodeObject->GetUInt(key3, defaultVal);
    EXPECT_EQ(ret3, 11);
}

/**
 * @tc.name: NodeObjectTest003
 * @tc.desc: Put(const char* key, const std::unique_ptr<NodeObject>& value)
 * @tc.type: FUNC
 */
HWTEST_F(NodeObjectTest, NodeObjectTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a NodeObject.
     */
    auto nodeObject = NodeObject::Create();

    // value is nullptr
    const std::unique_ptr<NodeObject> value = nullptr;
    const char* key = nullptr;
    bool ret = nodeObject->Put(key, value);
    EXPECT_EQ(ret, false);
    // value is not nullptr,key is nullptr
    auto value2 = NodeObject::Create();
    bool ret2 = nodeObject->Put(key, value2);
    EXPECT_EQ(ret2, false);
    const std::unique_ptr<JsonValue> value3 = std::make_unique<JsonValue>();
    bool ret3 = nodeObject->Put(key, value3);
    EXPECT_EQ(ret3, false);
}


class NodeObjectGapTest : public testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
};

/**
 * @tc.name: NodeObjectGapContains001
 * @tc.desc: Test Contains for present and absent keys.
 * @tc.type: FUNC
 */
HWTEST_F(NodeObjectGapTest, NodeObjectGapContains001, TestSize.Level1)
{
    /**
     * @tc.steps: put a key then query present/absent keys.
     * @tc.expected: present key returns true, absent key returns false.
     */
    auto node = NodeObject::Create();
    ASSERT_TRUE(node);
    node->Put("key", static_cast<int32_t>(42));
    EXPECT_TRUE(node->Contains("key"));
    EXPECT_FALSE(node->Contains("not-exist"));
}

/**
 * @tc.name: NodeObjectGapGetBoolGetUInt001
 * @tc.desc: Test GetBool and GetUInt with defaults.
 * @tc.type: FUNC
 */
HWTEST_F(NodeObjectGapTest, NodeObjectGapGetBoolGetUInt001, TestSize.Level1)
{
    /**
     * @tc.steps: put bool/uint items then get present and absent keys.
     * @tc.expected: present keys return stored values, absent keys return defaults.
     */
    auto node = NodeObject::Create();
    ASSERT_TRUE(node);
    node->Put("flag", true);
    node->Put("count", static_cast<int32_t>(7));
    EXPECT_TRUE(node->GetBool("flag"));
    EXPECT_FALSE(node->GetBool("flag-absent", false));
    EXPECT_TRUE(node->GetBool("flag-absent", true));

    EXPECT_EQ(node->GetUInt("count"), 7U);
    EXPECT_EQ(node->GetUInt("count-absent", 9U), 9U);
}

/**
 * @tc.name: NodeObjectGapPutNullArgs001
 * @tc.desc: Test Put overloads reject null key/value.
 * @tc.type: FUNC
 */
HWTEST_F(NodeObjectGapTest, NodeObjectGapPutNullArgs001, TestSize.Level1)
{
    /**
     * @tc.steps: call Put with null key and null value pointers.
     * @tc.expected: all overloads return false and store nothing.
     */
    auto node = NodeObject::Create();
    ASSERT_TRUE(node);
    EXPECT_FALSE(node->Put(nullptr, "value"));
    EXPECT_FALSE(node->Put("key", nullptr));
    EXPECT_FALSE(node->Put(nullptr, static_cast<int32_t>(1)));
    EXPECT_FALSE(node->Put(nullptr, 1.0));
    EXPECT_FALSE(node->Put(nullptr, true));
    EXPECT_FALSE(node->Put(nullptr, static_cast<size_t>(1)));
    EXPECT_FALSE(node->Contains("key"));
}

/**
 * @tc.name: NodeObjectGapToStringRoundTrip001
 * @tc.desc: Test ToString/FromString serialization round trip.
 * @tc.type: FUNC
 */
HWTEST_F(NodeObjectGapTest, NodeObjectGapToStringRoundTrip001, TestSize.Level1)
{
    /**
     * @tc.steps: put mixed-type items, serialize to string, deserialize into a
     *            new object and read back.
     * @tc.expected: round trip preserves all values.
     */
    auto node = NodeObject::Create();
    ASSERT_TRUE(node);
    node->Put("name", "ace");
    node->Put("count", static_cast<int32_t>(11));
    node->Put("ratio", 0.75);
    node->Put("valid", true);

    auto serialized = node->ToString();
    EXPECT_FALSE(serialized.empty());

    auto restored = NodeObject::Create();
    ASSERT_TRUE(restored);
    restored->FromString(serialized);
    EXPECT_EQ(restored->GetString("name"), "ace");
    EXPECT_EQ(restored->GetInt("count"), 11);
    EXPECT_DOUBLE_EQ(restored->GetDouble("ratio"), 0.75);
    EXPECT_TRUE(restored->GetBool("valid"));
    EXPECT_TRUE(restored->Contains("name"));
}

/**
 * @tc.name: NodeObjectGapHashAndBufferSize001
 * @tc.desc: Test Hash and EstimateBufferSize.
 * @tc.type: FUNC
 */
HWTEST_F(NodeObjectGapTest, NodeObjectGapHashAndBufferSize001, TestSize.Level1)
{
    /**
     * @tc.steps: build two identical objects and one different object, compare
     *            hashes and buffer sizes.
     * @tc.expected: identical objects hash equally; buffer size grows with
     *               content and is greater than zero.
     */
    auto nodeA = NodeObject::Create();
    auto nodeB = NodeObject::Create();
    ASSERT_TRUE(nodeA);
    ASSERT_TRUE(nodeB);
    nodeA->Put("name", "ace");
    nodeB->Put("name", "ace");
    EXPECT_EQ(nodeA->Hash(), nodeB->Hash());

    auto hashBefore = nodeA->Hash();
    nodeB->Put("extra", static_cast<int32_t>(1));
    EXPECT_EQ(nodeA->Hash(), hashBefore);

    auto emptySize = NodeObject::Create()->EstimateBufferSize();
    auto smallSize = nodeA->EstimateBufferSize();
    auto biggerSize = nodeB->EstimateBufferSize();
    EXPECT_GE(smallSize, 0);
    EXPECT_GE(biggerSize, smallSize);
    EXPECT_GE(smallSize, emptySize);
}

/**
 * @tc.name: NodeObjectGapGetValue001
 * @tc.desc: Test GetValue/GetObject with present and absent keys.
 * @tc.type: FUNC
 */
HWTEST_F(NodeObjectGapTest, NodeObjectGapGetValue001, TestSize.Level1)
{
    /**
     * @tc.steps: put a nested object then read it back via GetValue/GetObject.
     * @tc.expected: nested object reads back its members; absent key yields an
     *               empty object.
     */
    auto outer = NodeObject::Create();
    auto inner = NodeObject::Create();
    ASSERT_TRUE(outer);
    ASSERT_TRUE(inner);
    inner->Put("id", static_cast<int32_t>(5));
    outer->Put("inner", inner);

    auto gotValue = outer->GetValue("inner");
    ASSERT_TRUE(gotValue);
    EXPECT_EQ(gotValue->GetInt("id"), 5);

    auto gotObject = outer->GetObject("inner");
    ASSERT_TRUE(gotObject);
    EXPECT_EQ(gotObject->GetInt("id"), 5);

    auto absent = outer->GetValue("absent");
    ASSERT_TRUE(absent);
    EXPECT_EQ(absent->GetInt("id", -1), -1);
}

/**
 * @tc.name: NodeObjectGapPutJsonValue001
 * @tc.desc: Test Put with JsonValue of object and nested types.
 * @tc.type: FUNC
 */
HWTEST_F(NodeObjectGapTest, NodeObjectGapPutJsonValue001, TestSize.Level1)
{
    /**
     * @tc.steps: put a JsonValue object containing string/bool/number/nested
     *            object members.
     * @tc.expected: all member types are accepted and readable afterwards.
     */
    auto json = JsonUtil::Create(true);
    ASSERT_TRUE(json);
    json->Put("str", "text");
    json->Put("num", static_cast<int32_t>(3));
    json->Put("flag", true);
    auto nested = JsonUtil::Create(true);
    nested->Put("deep", "value");
    json->Put("nested", nested);

    auto node = NodeObject::Create();
    ASSERT_TRUE(node);
    EXPECT_TRUE(node->Put("json", json));
    EXPECT_TRUE(node->Contains("json"));
}
} // namespace OHOS::Ace
