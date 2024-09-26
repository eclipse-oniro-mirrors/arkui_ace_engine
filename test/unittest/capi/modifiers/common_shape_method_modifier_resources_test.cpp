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

#include "modifier_test_base.h"
#include "modifiers_test_utils.h"

#include "core/interfaces/arkoala/utility/converter.h"
#include "core/interfaces/arkoala/utility/reverse_converter.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::NG::Converter;

namespace OHOS::Ace::NG {
namespace  {
Ark_Resource ArkResource(Ark_String *name, int id = -1,
    NodeModifier::ResourceType type = NodeModifier::ResourceType::COLOR,
    const char *module = "", const char *bundle = "")
{
    return {
        .id = ArkValue<Ark_Number>(id),
        .type = ArkValue<Ark_Number>(static_cast<Ark_Int32>(type)),
        .moduleName = ArkValue<Ark_String>(module),
        .bundleName = ArkValue<Ark_String>(bundle),
        .params = { .tag = ARK_TAG_OBJECT, .value = {.array = name, .length = name ? 1 : 0} }
    };
}

// attributes name
const auto ATTRIBUTE_STROKE_WIDTH_NAME = "strokeWidth";
const auto ATTRIBUTE_STROKE_OPACITY_NAME = "strokeOpacity";
const auto ATTRIBUTE_FILL_OPACITY_NAME = "fillOpacity";

// global variables
const Ark_Int32 FAKE_RES_ID(1234);

// resource names and id
const auto RES_NAME = "aa.bb.cc";
const auto RES_ARK_NAME = ArkValue<Ark_String>(RES_NAME);
const auto RES_ID = 11111;

// resource values
const auto RESOURCE_OPACITY_BY_STRING = 0.4f;
const auto RESOURCE_OPACITY_BY_NUMBER = 0.5f;

// check
const auto CHECK_RESOURCE_OPACITY_BY_STRING = "0.400000";
const auto CHECK_RESOURCE_OPACITY_BY_NUMBER = "0.500000";

using OneUnionNumStrResStep = std::pair<Union_Number_String_Resource, std::string>;

static const std::vector<OneUnionNumStrResStep> UNION_NUM_STR_RES_RESOURECES_TEST_PLAN = {
    { ArkUnion<Union_Number_String_Resource, Ark_Resource>(ArkResource(const_cast<Ark_String*>(&RES_ARK_NAME),
        -1, NodeModifier::ResourceType::FLOAT)),
        CHECK_RESOURCE_OPACITY_BY_STRING
    },
    { ArkUnion<Union_Number_String_Resource, Ark_Resource>(ArkResource(nullptr, RES_ID,
        NodeModifier::ResourceType::FLOAT)),
        CHECK_RESOURCE_OPACITY_BY_NUMBER
    },
    { ArkUnion<Union_Number_String_Resource, Ark_Resource>(ArkResource(nullptr, -1,
        NodeModifier::ResourceType::FLOAT)),
        "0.000000"
    }
};
} // namespace;
class CommonShapeMethodModifierResourcesTest : public ModifierTestBase<GENERATED_ArkUICommonShapeMethodModifier,
    &GENERATED_ArkUINodeModifiers::getCommonShapeMethodModifier,
    GENERATED_ARKUI_CIRCLE // test common shape methods on frameNode for Circle component
    > {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        // set test values to Theme Pattern as data for the Theme building
        AddResource(RES_NAME, RESOURCE_OPACITY_BY_STRING);
        AddResource(RES_ID, RESOURCE_OPACITY_BY_NUMBER);
    }
};

/*
 * @tc.name: setStrokeWidthTestValidValues
 * @tc.desc: check setStrokeWidth from resource
 * @tc.type: FUNC
 */
HWTEST_F(CommonShapeMethodModifierResourcesTest, DISABLED_setStrokeWidthTestResources, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    double result;

    typedef std::pair<Ark_Length, double> OneTestStep;
    const std::vector<OneTestStep> testPlan = {
    { 
        { .type = ARK_TAG_RESOURCE, .resource = FAKE_RES_ID }, 10 }
    };

    for (const auto &[arkLength, expected]: testPlan) {
        modifier_->setStrokeWidth(node_, &arkLength);
        jsonValue = GetJsonValue(node_);
        result = GetAttrValue<double>(jsonValue, ATTRIBUTE_STROKE_WIDTH_NAME);
        EXPECT_NEAR(result, expected, FLT_EPSILON);
    }
}

/**
 * @tc.name: setFillOpacityTest
 * @tc.desc: Check the functionality of setFillOpacity
 * @tc.type: FUNC
 */
HWTEST_F(CommonShapeMethodModifierResourcesTest, setFillOpacityTestResources, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    for (const auto &[value, expectVal]: UNION_NUM_STR_RES_RESOURECES_TEST_PLAN) {
        modifier_->setFillOpacity(node_, &value);
        jsonValue = GetJsonValue(node_);
        auto checkVal = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_FILL_OPACITY_NAME);
        EXPECT_EQ(checkVal, expectVal);
    }
}

/**
 * @tc.name: setStrokeOpacityTest
 * @tc.desc: Check the functionality of setStrokeOpacity
 * @tc.type: FUNC
 */
HWTEST_F(CommonShapeMethodModifierResourcesTest, setStrokeOpacityTestResources, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    for (const auto &[value, expectVal]: UNION_NUM_STR_RES_RESOURECES_TEST_PLAN) {
        modifier_->setStrokeOpacity(node_, &value);
        jsonValue = GetJsonValue(node_);
        auto checkVal = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_STROKE_OPACITY_NAME);
        EXPECT_EQ(checkVal, expectVal);
    }
}
} // namespace OHOS::Ace::NG
