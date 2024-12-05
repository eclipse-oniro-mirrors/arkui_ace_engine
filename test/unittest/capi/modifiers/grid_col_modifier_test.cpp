/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "modifier_test_base.h"
#include "modifiers_test_utils.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "arkoala_api_generated.h"
#include <tuple>

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace Converter {
    void AssignOptValue(Opt_Number& dst, const int32_t& src)
    {
        auto arkValue = Converter::ArkValue<Ark_Number>(src);
        dst.tag = ARK_TAG_INT32;
        dst.value = arkValue;
    }
    void AssignOptValue(Opt_Union_Number_GridColColumnOption& dst, const Ark_Union_Number_GridColColumnOption& src)
    {
        dst.tag = ARK_TAG_OBJECT;
        dst.value = src;
    }
    void AssignOptValue(Opt_GridColOptions& dst, const Ark_GridColOptions& src)
    {
        dst.tag = ARK_TAG_OBJECT;
        dst.value = src;
    }
    template<typename To, typename From>
    To OptValue(const From& src)
    {
        To result;
        AssignOptValue(result, src);
        return result;
    }
    template<>
    Ark_GridColColumnOption ArkValue(const int32_t& value)
    {
        Opt_Number optNumValue = OptValue<Opt_Number>(value);
        return {
            .lg = optNumValue,
            .md = optNumValue,
            .sm = optNumValue,
            .xl = optNumValue,
            .xs = optNumValue,
            .xxl = optNumValue
        };
    }
} // namespace Converter

struct GridColOptions {
    int32_t span;
    int32_t offset;
    int32_t order;
};
struct GridColOptionsTestRow {
    GridColOptions result;
    GridColOptions input;
    GridColOptions expected;
};
GridColOptionsTestRow GridColOptionsTestRowCreate(
    GridColOptions result,
    GridColOptions input,
    GridColOptions expected)
{
    return {
        .result = result,
        .input = input,
        .expected = expected
    };
}
using TupleOptionsTest = std::tuple<GridColOptions, Opt_GridColOptions, GridColOptions>;
using VectorOptionsTest = std::vector<TupleOptionsTest>;

TupleOptionsTest getTestTuple(const GridColOptionsTestRow& src)
{
    auto arkSpanInputValue = Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_Number>(src.input.span);
    auto arkOffsetInputValue = Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_Number>(src.input.offset);
    auto arkOrderInputValue = Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_Number>(src.input.order);
    Ark_GridColOptions arkInputValue = {
        .span = Converter::OptValue<Opt_Union_Number_GridColColumnOption>(arkSpanInputValue),
        .offset = Converter::OptValue<Opt_Union_Number_GridColColumnOption>(arkOffsetInputValue),
        .order = Converter::OptValue<Opt_Union_Number_GridColColumnOption>(arkOrderInputValue),
    };
    auto optInputValue = Converter::OptValue<Opt_GridColOptions>(arkInputValue);
    return {src.result, optInputValue, src.expected};
}

namespace  {
    const auto ATTRIBUTE_SPAN_NAME = "span";
    const auto ATTRIBUTE_GRID_COL_OFFSET_NAME = "gridColOffset";
    const auto ATTRIBUTE_ORDER_NAME = "order";
    const auto ATTRIBUTE_SPAN_DEFAULT_VALUE = 1;
    const auto ATTRIBUTE_GRID_COL_OFFSET_DEFAULT_VALUE = 0;
    const auto ATTRIBUTE_ORDER_DEFAULT_VALUE = 0;
} // namespace

class GridColModifierTest : public ModifierTestBase<GENERATED_ArkUIGridColModifier,
                     &GENERATED_ArkUINodeModifiers::getGridColModifier, GENERATED_ARKUI_GRID_COL> {
};

/*
 * @tc.name: setGridColOptionsTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridColModifierTest, setGridColOptionsTestDefaultValues, TestSize.Level1)
{
    CHECK_NULL_VOID(node_);
    Opt_GridColOptions initValue = {.tag = ARK_TAG_UNDEFINED, .value = {}};
    modifier_->setGridColOptions(node_, &initValue);
    std::unique_ptr<JsonValue> jsonLayout = GetLayoutJsonValue(node_);
    CHECK_NULL_VOID(jsonLayout);
    int32_t span = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_SPAN_NAME));
    int32_t offset = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_GRID_COL_OFFSET_NAME));
    int32_t order = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_ORDER_NAME));
    EXPECT_EQ(span, ATTRIBUTE_SPAN_DEFAULT_VALUE);
    EXPECT_EQ(offset, ATTRIBUTE_GRID_COL_OFFSET_DEFAULT_VALUE);
    EXPECT_EQ(order, ATTRIBUTE_ORDER_DEFAULT_VALUE);
}

static VectorOptionsTest setGridColOptionsValidValues = {
    getTestTuple(GridColOptionsTestRowCreate({0, 0, 0}, {0, 0, 0}, {0, 0, 0})),
    getTestTuple(GridColOptionsTestRowCreate({1, 2, 3}, {1, 2, 3}, {1, 2, 3})),
    getTestTuple(GridColOptionsTestRowCreate({3, 1, 2}, {3, 1, 2}, {3, 1, 2})),
};

/*
 * @tc.name: setGridColOptionsTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridColModifierTest, setGridColOptionsTestValidValues, TestSize.Level1)
{
    CHECK_NULL_VOID(node_);
    GridColOptions expectedValue;
    Opt_GridColOptions optInputValue;
    std::unique_ptr<JsonValue> jsonLayout;
    // Verifying attribute's  values
    for (auto validValue: setGridColOptionsValidValues) {
        optInputValue = std::get<1>(validValue);
        expectedValue = std::get<2>(validValue);
        modifier_->setGridColOptions(node_, &optInputValue);
        jsonLayout = GetLayoutJsonValue(node_);
        CHECK_NULL_VOID(jsonLayout);
        int32_t resultSpan = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_SPAN_NAME));
        EXPECT_EQ(resultSpan, expectedValue.span);
        int32_t resultOffset = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_GRID_COL_OFFSET_NAME));
        EXPECT_EQ(resultOffset, expectedValue.offset);
        int32_t resultOrder = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_ORDER_NAME));
        EXPECT_EQ(resultOrder, expectedValue.order);
    }
}

// Invalid values for attribute 'options' of method 'setGridColOptions'
using TupleInvalidOptionsTest = std::tuple<double, bool, Opt_GridColOptions>;
using VectorInvalidOptionsTest = std::vector<TupleInvalidOptionsTest>;

static VectorOptionsTest setGridColOptionsInvalidValues = {
    getTestTuple(GridColOptionsTestRowCreate({1, 0, 0}, {-1, -1, -1}, {1, 0, 0})),
    getTestTuple(GridColOptionsTestRowCreate({1, 0, 0}, {-10, -10, -10}, {1, 0, 0})),
};

/*
 * @tc.name: setGridColOptionsTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridColModifierTest, setGridColOptionsTestInvalidValues, TestSize.Level1)
{
    CHECK_NULL_VOID(node_);
    GridColOptions expectedValue;
    Opt_GridColOptions optInputValue;
    std::unique_ptr<JsonValue> jsonLayout;
    // Verifying attribute's  values
    for (auto validValue: setGridColOptionsInvalidValues) {
        optInputValue = std::get<1>(validValue);
        expectedValue = std::get<2>(validValue);
        modifier_->setGridColOptions(node_, &optInputValue);
        jsonLayout = GetLayoutJsonValue(node_);
        CHECK_NULL_VOID(jsonLayout);
        int32_t resultSpan = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_SPAN_NAME));
        EXPECT_EQ(resultSpan, expectedValue.span);
        int32_t resultOffset = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_GRID_COL_OFFSET_NAME));
        EXPECT_EQ(resultOffset, expectedValue.offset);
        int32_t resultOrder = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_ORDER_NAME));
        EXPECT_EQ(resultOrder, expectedValue.order);
    }
}

/*
 * @tc.name: setSpanTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridColModifierTest, setSpanTestDefaultValues, TestSize.Level1)
{
    CHECK_NULL_VOID(node_);
    auto initValue = Converter::ArkUnion<Ark_Union_Number_GridColColumnOption,
        Ark_GridColColumnOption>(ATTRIBUTE_SPAN_DEFAULT_VALUE);
    modifier_->setSpan(node_, &initValue);
    std::unique_ptr<JsonValue> jsonLayout = GetLayoutJsonValue(node_);
    CHECK_NULL_VOID(jsonLayout);
    int32_t span = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_SPAN_NAME));
    EXPECT_EQ(span, ATTRIBUTE_SPAN_DEFAULT_VALUE);
}

// Valid values for attribute 'Span' of method 'Span'
static std::vector<std::tuple<int32_t, Ark_Union_Number_GridColColumnOption, int32_t>> spanValidValues = {
    {0, Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_GridColColumnOption>(0), 0},
    {1, Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_GridColColumnOption>(1), 1},
    {5, Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_GridColColumnOption>(5), 5}
};

/*
 * @tc.name: setSpanTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridColModifierTest, setSpanTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonLayout;
    int32_t expectedValue {0};
    int32_t resultValue {0};
    Ark_Union_Number_GridColColumnOption inputValueSpan;
    // Initial setup
    Ark_Union_Number_GridColColumnOption initValueSpan = std::get<1>(spanValidValues[0]);
    // Verifying attribute's  values
    inputValueSpan = initValueSpan;
    for (auto&& value: spanValidValues) {
        inputValueSpan = std::get<1>(value);
        modifier_->setSpan(node_, &inputValueSpan);
        jsonLayout = GetLayoutJsonValue(node_);
        CHECK_NULL_VOID(jsonLayout);
        resultValue = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_SPAN_NAME));
        expectedValue = std::get<2>(value);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << std::get<0>(value);
    }
}

// Invalid values for attribute 'span' of method 'span'
static std::vector<std::tuple<int32_t, Ark_Union_Number_GridColColumnOption, int32_t>> spanInvalidValues = {
    {1, Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_GridColColumnOption>(-1), 1},
    {1, Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_GridColColumnOption>(-5), 1},
};

/*
 * @tc.name: setSpanTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridColModifierTest, setSpanTestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonLayout;
    int32_t expectedValue {0};
    int32_t resultValue {0};
    Ark_Union_Number_GridColColumnOption inputValueSpan;
    // Initial setup
    Ark_Union_Number_GridColColumnOption initValueSpan = std::get<1>(spanValidValues[0]);
    // Verifying attribute's  values
    inputValueSpan = initValueSpan;
    for (auto&& value: spanInvalidValues) {
        inputValueSpan = std::get<1>(value);
        modifier_->setSpan(node_, &inputValueSpan);
        jsonLayout = GetLayoutJsonValue(node_);
        CHECK_NULL_VOID(jsonLayout);
        resultValue = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_SPAN_NAME));
        expectedValue = std::get<2>(value);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << std::get<0>(value);
    }
}

/*
 * @tc.name: setOffsetTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridColModifierTest, setOffsetTestDefaultValues, TestSize.Level1)
{
    CHECK_NULL_VOID(node_);
    auto initValue = Converter::ArkUnion<Ark_Union_Number_GridColColumnOption,
        Ark_GridColColumnOption>(ATTRIBUTE_GRID_COL_OFFSET_DEFAULT_VALUE);
    modifier_->setGridColOffset(node_, &initValue);
    std::unique_ptr<JsonValue> jsonLayout = GetLayoutJsonValue(node_);
    CHECK_NULL_VOID(jsonLayout);
    int32_t offset = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_GRID_COL_OFFSET_NAME));
    EXPECT_EQ(offset, ATTRIBUTE_GRID_COL_OFFSET_DEFAULT_VALUE);
}

// Valid values for attribute 'Offset' of method 'Offset'
static std::vector<std::tuple<int32_t, Ark_Union_Number_GridColColumnOption, int32_t>> offsetValidValues = {
    {0, Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_GridColColumnOption>(0), 0},
    {1, Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_GridColColumnOption>(1), 1},
    {5, Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_GridColColumnOption>(5), 5}
};

/*
 * @tc.name: setOffsetTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridColModifierTest, setOffsetTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonLayout;
    int32_t expectedValue {0};
    int32_t resultValue {0};
    Ark_Union_Number_GridColColumnOption inputValueOffset;
    // Initial setup
    Ark_Union_Number_GridColColumnOption initValueOffset = std::get<1>(offsetValidValues[0]);
    // Verifying attribute's  values
    inputValueOffset = initValueOffset;
    for (auto&& value: offsetValidValues) {
        inputValueOffset = std::get<1>(value);
        modifier_->setGridColOffset(node_, &inputValueOffset);
        jsonLayout = GetLayoutJsonValue(node_);
        CHECK_NULL_VOID(jsonLayout);
        resultValue = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_GRID_COL_OFFSET_NAME));
        expectedValue = std::get<2>(value);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << std::get<0>(value);
    }
}

// Invalid values for attribute 'Offset' of method 'Offset'
static std::vector<std::tuple<int32_t, Ark_Union_Number_GridColColumnOption, int32_t>> offsetInvalidValues = {
    {0, Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_GridColColumnOption>(-1), 0},
    {0, Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_GridColColumnOption>(-5), 0},
};

/*
 * @tc.name: setOffsetTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridColModifierTest, setOffsetTestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonLayout;
    int32_t expectedValue {0};
    int32_t resultValue {0};
    Ark_Union_Number_GridColColumnOption inputValueOffset;
    // Initial setup
    Ark_Union_Number_GridColColumnOption initValueOffset = std::get<1>(offsetValidValues[0]);
    // Verifying attribute's  values
    inputValueOffset = initValueOffset;
    for (auto&& value: offsetInvalidValues) {
        inputValueOffset = std::get<1>(value);
        modifier_->setGridColOffset(node_, &inputValueOffset);
        jsonLayout = GetLayoutJsonValue(node_);
        CHECK_NULL_VOID(jsonLayout);
        resultValue = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_GRID_COL_OFFSET_NAME));
        expectedValue = std::get<2>(value);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << std::get<0>(value);
    }
}

/*
 * @tc.name: setOrderTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridColModifierTest, setOrderTestDefaultValues, TestSize.Level1)
{
    CHECK_NULL_VOID(node_);
    auto initValue = Converter::ArkUnion<Ark_Union_Number_GridColColumnOption,
        Ark_GridColColumnOption>(ATTRIBUTE_ORDER_DEFAULT_VALUE);
    modifier_->setOrder(node_, &initValue);
    std::unique_ptr<JsonValue> jsonLayout = GetLayoutJsonValue(node_);
    CHECK_NULL_VOID(jsonLayout);
    int32_t order = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_ORDER_NAME));
    EXPECT_EQ(order, ATTRIBUTE_ORDER_DEFAULT_VALUE);
}

// Valid values for attribute 'Order' of method 'Order'
static std::vector<std::tuple<int32_t, Ark_Union_Number_GridColColumnOption, int32_t>> orderValidValues = {
    {0, Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_GridColColumnOption>(0), 0},
    {1, Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_GridColColumnOption>(1), 1},
    {5, Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_GridColColumnOption>(5), 5}
};

/*
 * @tc.name: setOrderTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridColModifierTest, setOrderTestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonLayout;
    int32_t expectedValue {0};
    int32_t resultValue {0};
    Ark_Union_Number_GridColColumnOption inputValueOrder;
    // Initial setup
    Ark_Union_Number_GridColColumnOption initValueOrder = std::get<1>(orderValidValues[0]);
    // Verifying attribute's  values
    inputValueOrder = initValueOrder;
    for (auto&& value: orderValidValues) {
        inputValueOrder = std::get<1>(value);
        modifier_->setOrder(node_, &inputValueOrder);
        jsonLayout = GetLayoutJsonValue(node_);
        CHECK_NULL_VOID(jsonLayout);
        resultValue = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_ORDER_NAME));
        expectedValue = std::get<2>(value);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << std::get<0>(value);
    }
}

// Invalid values for attribute 'Order' of method 'Order'
static std::vector<std::tuple<int32_t, Ark_Union_Number_GridColColumnOption, int32_t>> orderInvalidValues = {
    {0, Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_GridColColumnOption>(-1), 0},
    {0, Converter::ArkUnion<Ark_Union_Number_GridColColumnOption, Ark_GridColColumnOption>(-5), 0},
};

/*
 * @tc.name: setOrderTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(GridColModifierTest, setOrderTestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonLayout;
    int32_t expectedValue {0};
    int32_t resultValue {0};
    Ark_Union_Number_GridColColumnOption inputValueOrder;
    // Initial setup
    Ark_Union_Number_GridColColumnOption initValueOrder = std::get<1>(orderValidValues[0]);
    // Verifying attribute's  values
    inputValueOrder = initValueOrder;
    for (auto&& value: orderInvalidValues) {
        inputValueOrder = std::get<1>(value);
        modifier_->setOrder(node_, &inputValueOrder);
        jsonLayout = GetLayoutJsonValue(node_);
        CHECK_NULL_VOID(jsonLayout);
        resultValue = std::stoi(GetAttrValue<std::string>(jsonLayout, ATTRIBUTE_ORDER_NAME));
        expectedValue = std::get<2>(value);
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << std::get<0>(value);
    }
}
} // namespace OHOS::Ace::NG
