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

// Branch-coverage tests for the LinearLayout component (Column / Row).
// Targets: column_model_ng.cpp, row_model_ng.cpp and linear_layout_utils.cpp
// (the FrameNode* model overloads, the resource-object / config-change paths,
//  and the LinearLayoutUtils::Measure/Layout paths that normal Column/Row
//  layout does not reach because it is driven by FlexLayoutAlgorithm).

#include <cstdint>
#include <list>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

// Mock resource adapter helpers (AddMockResourceData) are plain free functions, safe before test_ng.h opens
// the access specifiers below.
#include "test/mock/frameworks/core/common/mock_resource_adapter_v2.h"

#include "linear_layout_base_test_ng.h"

#include "core/common/ace_application_info.h"
#include "core/common/container.h"
#include "core/common/resource/resource_parse_utils.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper_node.h"
#include "core/components_ng/pattern/linear_layout/column_model_ng.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_pattern.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_property.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_utils.h"
#include "core/components_ng/pattern/linear_layout/row_model_ng.h"
#include "core/components_ng/pattern/text/text_model_ng.h"
#include "core/components_ng/property/calc_length.h"
#include "core/components_ng/property/layout_constraint.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/pipeline/base/element_register.h"
#include "test/mock/adapter/ohos/osal/mock_system_properties.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float PARENT_WIDTH = 720.0f;
constexpr float PARENT_HEIGHT = 1136.0f;

// Resource ids used with the mocked resource adapter. Each test uses its own ids so that the
// process-wide mock resource map cannot leak values between tests.
constexpr int32_t RES_COLUMN_CREATE_POSITIVE = 2101;
constexpr int32_t RES_COLUMN_CREATE_NEGATIVE = 2102;
constexpr int32_t RES_ROW_CREATE_POSITIVE = 2103;
constexpr int32_t RES_ROW_CREATE_NEGATIVE = 2104;
constexpr int32_t RES_COLUMN_SET_POSITIVE = 2105;
constexpr int32_t RES_COLUMN_SET_NEGATIVE = 2106;
constexpr int32_t RES_ROW_SET_POSITIVE = 2107;
constexpr int32_t RES_ROW_SET_NEGATIVE = 2108;
constexpr int32_t RES_COLUMN_RESET = 2109;
constexpr int32_t RES_ROW_RESET = 2110;

// Pops the node created by a model Create() call from the view stack (the normal TestNG convention).
// The stack is cleared afterwards: PopContainer() intentionally keeps the last entry as the implicit
// root, which would otherwise make the next Create() in the same test nest under this node.
RefPtr<FrameNode> PopFrameNodeFromStack()
{
    auto* stack = ViewStackProcessor::GetInstance();
    RefPtr<UINode> element = stack->GetMainElementNode();
    stack->PopContainer();
    stack->ClearStack();
    return AceType::DynamicCast<FrameNode>(element);
}

// Saves / restores the global RTL flag. Some other test file in this binary leaves it enabled,
// so every layout direction resolution done here must be made deterministic.
class ScopedRightToLeft {
public:
    explicit ScopedRightToLeft(bool rightToLeft)
    {
        previous_ = AceApplicationInfo::GetInstance().isRightToLeft_;
        AceApplicationInfo::GetInstance().isRightToLeft_ = rightToLeft;
    }
    ~ScopedRightToLeft()
    {
        AceApplicationInfo::GetInstance().isRightToLeft_ = previous_;
    }

private:
    bool previous_ = false;
};

// Saves / restores the sticky config-change switch.
class ScopedConfigChangePerform {
public:
    explicit ScopedConfigChangePerform(bool perform)
    {
        previous_ = g_isConfigChangePerform;
        g_isConfigChangePerform = perform;
    }
    ~ScopedConfigChangePerform()
    {
        g_isConfigChangePerform = previous_;
    }

private:
    bool previous_ = false;
};

// Builds a resolvable dimension resource on the mocked resource adapter.
RefPtr<ResourceObject> CreateDimensionResObj(int32_t resId, const Dimension& dimension)
{
    AddMockResourceData(static_cast<uint32_t>(resId), dimension);
    std::vector<ResourceObjectParams> params;
    return AceType::MakeRefPtr<ResourceObject>(
        resId, static_cast<int32_t>(ResourceType::FLOAT), params, "", "", Container::CurrentIdSafely());
}

// A FrameNode whose LayoutProperty was detached. GetLayoutPropertyPtr() then returns nullptr, which
// is the only safe way to drive the defensive "dynamic cast failed" arm of the property macros
// (the macros use a static_cast fast path, so a node with a *different* property type is undefined
// behaviour rather than a null result).
// test_ng.h opens the access specifiers (#define private public) before including frame_node.h.
RefPtr<FrameNode> CreateNodeWithoutLayoutProperty(bool isVertical)
{
    auto frameNode = FrameNode::CreateFrameNode(isVertical ? V2::COLUMN_ETS_TAG : V2::ROW_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(isVertical));
    frameNode->layoutProperty_.Reset();
    return frameNode;
}

struct WrapNodePair {
    RefPtr<FrameNode> frameNode;
    RefPtr<LayoutWrapperNode> wrapper;
};

// Container wrapper for LinearLayoutUtils::Measure / Layout tests. Both UpdateLayoutConstraint() and
// UpdateContentConstraint() are mandatory (see linear_layout_utils.cpp:110 / :201 / CreateChildConstraint).
WrapNodePair CreateWrapNodePair(bool isVertical, const std::optional<CalcSize>& idealSize, const SizeF& maxSize,
    const OptionalSizeF& parentIdealSize = OptionalSizeF())
{
    auto frameNode = FrameNode::CreateFrameNode(isVertical ? V2::COLUMN_ETS_TAG : V2::ROW_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(isVertical));
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto wrapper = AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, frameNode->GetLayoutProperty());
    auto layoutProperty = wrapper->GetLayoutProperty();
    if (idealSize.has_value()) {
        layoutProperty->UpdateUserDefinedIdealSize(idealSize.value());
    }
    LayoutConstraintF parentConstraint;
    parentConstraint.maxSize = maxSize;
    parentConstraint.percentReference = maxSize;
    parentConstraint.parentIdealSize = parentIdealSize;
    layoutProperty->UpdateLayoutConstraint(parentConstraint);
    layoutProperty->UpdateContentConstraint();
    return { frameNode, wrapper };
}

LayoutConstraintF CreateChildConstraintFor(const WrapNodePair& parent)
{
    auto childConstraint = parent.wrapper->GetLayoutProperty()->CreateChildConstraint();
    childConstraint.maxSize = SizeF(PARENT_WIDTH, PARENT_HEIGHT);
    childConstraint.minSize = SizeF(0.0f, 0.0f);
    return childConstraint;
}

// Child wrapper backed by a real Blank FrameNode. The child has to be registered on both the host
// FrameNode and the wrapper list, exactly like old_cases/linear_layout_test_ng.cpp does.
RefPtr<LayoutWrapperNode> AppendChildWrap(const WrapNodePair& parent, const LayoutConstraintF& childConstraint,
    float width, float height, const std::optional<float>& weight = std::nullopt)
{
    auto childNode = FrameNode::CreateFrameNode(
        V2::BLANK_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto childGeometry = AceType::MakeRefPtr<GeometryNode>();
    childGeometry->Reset();
    auto childWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(childNode, childGeometry, childNode->GetLayoutProperty());
    auto childProperty = childWrapper->GetLayoutProperty();
    childProperty->UpdateLayoutConstraint(childConstraint);
    childProperty->UpdateUserDefinedIdealSize(CalcSize(CalcLength(width), CalcLength(height)));
    if (weight.has_value()) {
        childProperty->UpdateLayoutWeight(weight.value());
    }
    auto boxLayoutAlgorithm = childNode->GetPattern<Pattern>()->CreateLayoutAlgorithm();
    childWrapper->SetLayoutAlgorithm(AceType::MakeRefPtr<LayoutAlgorithmWrapper>(boxLayoutAlgorithm));
    parent.frameNode->AddChild(childNode);
    parent.wrapper->AppendChild(childWrapper);
    return childWrapper;
}

// Child wrapper without a LayoutProperty: exercises the CHECK_NULL_CONTINUE arm of
// TravelChildrenFlexProps (linear_layout_utils.cpp:74).
RefPtr<LayoutWrapperNode> AppendChildWrapWithoutProperty(const WrapNodePair& parent)
{
    auto childNode = FrameNode::CreateFrameNode(
        V2::BLANK_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    auto childWrapper =
        AceType::MakeRefPtr<LayoutWrapperNode>(childNode, AceType::MakeRefPtr<GeometryNode>(), nullptr);
    parent.frameNode->AddChild(childNode);
    parent.wrapper->AppendChild(childWrapper);
    return childWrapper;
}

std::list<RefPtr<LayoutWrapper>> GetChildren(const WrapNodePair& parent)
{
    return parent.wrapper->GetAllChildrenWithBuild();
}
} // namespace

class LinearLayoutCoverageTestNG : public LinearLayoutBaseTestNG {};

// ---------------------------------------------------------------------------------------------
// ColumnModelNG: Create() overloads (column_model_ng.cpp:24-41)
// ---------------------------------------------------------------------------------------------

/**
 * @tc.name: ColumnCreateWithSpace
 * @tc.desc: Pin the space handling of ColumnModelNG::Create: valid space is written to the property,
 *           a negative space only hits the log arm and leaves the property untouched, and the
 *           std::nullopt overload skips the write entirely.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, ColumnCreateWithSpace, TestSize.Level0)
{
    // column_model_ng.cpp:33,35-T,36-T,37 - legal space is stored.
    {
        ColumnModelNG model;
        model.Create(Dimension(10.0, DimensionUnit::VP), nullptr, "");
        auto frameNode = PopFrameNodeFromStack();
        ASSERT_NE(frameNode, nullptr);
        auto layoutProperty = frameNode->GetLayoutProperty<LinearLayoutProperty>();
        ASSERT_NE(layoutProperty, nullptr);
        EXPECT_EQ(layoutProperty->GetFlexDirectionValue(FlexDirection::ROW), FlexDirection::COLUMN);
        EXPECT_TRUE(layoutProperty->HasSpace());
        EXPECT_EQ(ColumnModelNG::GetSpace(AceType::RawPtr(frameNode)), 10.0);
    }
    // column_model_ng.cpp:36-F,39 - negative space is rejected, the property keeps its default.
    {
        ColumnModelNG model;
        model.Create(Dimension(-5.0, DimensionUnit::VP), nullptr, "");
        auto frameNode = PopFrameNodeFromStack();
        ASSERT_NE(frameNode, nullptr);
        auto layoutProperty = frameNode->GetLayoutProperty<LinearLayoutProperty>();
        ASSERT_NE(layoutProperty, nullptr);
        EXPECT_EQ(layoutProperty->GetFlexDirectionValue(FlexDirection::ROW), FlexDirection::COLUMN);
        EXPECT_FALSE(layoutProperty->HasSpace());
        EXPECT_EQ(ColumnModelNG::GetSpace(AceType::RawPtr(frameNode)), 0.0);
    }
    // column_model_ng.cpp:35-F - the std::nullopt overload returns before touching the space.
    {
        auto frameNode = CreateColumn([](ColumnModelNG model) {});
        ASSERT_NE(frameNode, nullptr);
        auto layoutProperty = frameNode->GetLayoutProperty<LinearLayoutProperty>();
        ASSERT_NE(layoutProperty, nullptr);
        EXPECT_FALSE(layoutProperty->HasSpace());
    }
}

// ---------------------------------------------------------------------------------------------
// ColumnModelNG: FrameNode based setters / getters (column_model_ng.cpp:84-208)
// ---------------------------------------------------------------------------------------------

/**
 * @tc.name: ColumnFrameNodeSettersAndGetters
 * @tc.desc: Cover every FrameNode* static API of ColumnModelNG: CreateFrameNode() direction, the
 *           SetSpace(std::optional) / SetAlignItems / SetJustifyContent / SetIsReverse updates, the
 *           matching getters together with their default values, the CHECK_NULL_RETURN(nullptr) arms
 *           and the defensive null-LayoutProperty arms of the property macros.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, ColumnFrameNodeSettersAndGetters, TestSize.Level0)
{
    // column_model_ng.cpp:128-137 - CreateFrameNode() builds a COLUMN node and updates FlexDirection.
    auto frameNode = ColumnModelNG::CreateFrameNode(ElementRegister::GetInstance()->MakeUniqueId());
    ASSERT_NE(frameNode, nullptr);
    EXPECT_EQ(frameNode->GetTag(), V2::COLUMN_ETS_TAG);
    auto layoutProperty = frameNode->GetLayoutProperty<LinearLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetFlexDirectionValue(FlexDirection::ROW), FlexDirection::COLUMN);
    auto* raw = AceType::RawPtr(frameNode);

    // column_model_ng.cpp:182-192 - default values when the attributes are absent.
    EXPECT_EQ(ColumnModelNG::GetJustifyContent(raw), FlexAlign::AUTO);
    EXPECT_EQ(ColumnModelNG::GetAlignItems(raw), FlexAlign::CENTER);
    // column_model_ng.cpp:196-200 - unset space falls back to 0 VP.
    EXPECT_EQ(ColumnModelNG::GetSpace(raw), 0.0);
    // column_model_ng.cpp:204-207 - unset reverse falls back to false.
    EXPECT_FALSE(ColumnModelNG::GetIsReverse(raw));

    // column_model_ng.cpp:165-178 - static FrameNode setters.
    ColumnModelNG::SetJustifyContent(raw, FlexAlign::FLEX_END);
    ColumnModelNG::SetAlignItems(raw, FlexAlign::FLEX_START);
    ColumnModelNG::SetIsReverse(raw, true);
    ColumnModelNG::SetSpace(raw, Dimension(7.0, DimensionUnit::VP));
    EXPECT_EQ(ColumnModelNG::GetJustifyContent(raw), FlexAlign::FLEX_END);
    EXPECT_EQ(ColumnModelNG::GetAlignItems(raw), FlexAlign::FLEX_START);
    EXPECT_TRUE(ColumnModelNG::GetIsReverse(raw));
    EXPECT_EQ(ColumnModelNG::GetSpace(raw), 7.0);
    EXPECT_TRUE(layoutProperty->HasMainAxisAlign());
    EXPECT_TRUE(layoutProperty->HasCrossAxisAlign());
    EXPECT_TRUE(layoutProperty->HasIsReverse());
    EXPECT_TRUE(layoutProperty->HasSpace());

    // column_model_ng.cpp:197,205 - CHECK_NULL_RETURN(nullptr) arms.
    EXPECT_EQ(ColumnModelNG::GetSpace(nullptr), 0.0);
    EXPECT_FALSE(ColumnModelNG::GetIsReverse(nullptr));
    // column_model_ng.cpp:167,172,177,88 - the macro's CHECK_NULL_VOID(frameNode) arm.
    ColumnModelNG::SetIsReverse(nullptr, true);
    ColumnModelNG::SetJustifyContent(nullptr, FlexAlign::CENTER);
    ColumnModelNG::SetAlignItems(nullptr, FlexAlign::CENTER);
    ColumnModelNG::SetSpace(nullptr, Dimension(10.0, DimensionUnit::VP));
    ColumnModelNG::SetSpace(nullptr, std::nullopt);

    // column_model_ng.cpp:88,167,172,177,183,190,198,206 - "if (castLinearLayoutProperty)" false arm.
    auto propertyless = CreateNodeWithoutLayoutProperty(true);
    ASSERT_NE(propertyless, nullptr);
    auto* propertylessRaw = AceType::RawPtr(propertyless);
    ColumnModelNG::SetIsReverse(propertylessRaw, true);
    ColumnModelNG::SetJustifyContent(propertylessRaw, FlexAlign::CENTER);
    ColumnModelNG::SetAlignItems(propertylessRaw, FlexAlign::CENTER);
    ColumnModelNG::SetSpace(propertylessRaw, Dimension(10.0, DimensionUnit::VP));
    EXPECT_EQ(ColumnModelNG::GetJustifyContent(propertylessRaw), FlexAlign::AUTO);
    EXPECT_EQ(ColumnModelNG::GetAlignItems(propertylessRaw), FlexAlign::CENTER);
    EXPECT_EQ(ColumnModelNG::GetSpace(propertylessRaw), 0.0);
    EXPECT_FALSE(ColumnModelNG::GetIsReverse(propertylessRaw));
    EXPECT_FALSE(propertyless->GetLayoutProperty());
}

/**
 * @tc.name: ColumnSetSpaceFrameNodeOptional
 * @tc.desc: Pin the std::optional<Dimension> overload of ColumnModelNG::SetSpace: legal values are
 *           stored, negative values are rejected without touching the previous value, and an empty
 *           optional returns early.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, ColumnSetSpaceFrameNodeOptional, TestSize.Level0)
{
    auto frameNode = CreateColumn([](ColumnModelNG model) {});
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<LinearLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto* raw = AceType::RawPtr(frameNode);

    // column_model_ng.cpp:86-T,87-T,88-T
    ColumnModelNG::SetSpace(raw, Dimension(10.0, DimensionUnit::VP));
    EXPECT_TRUE(layoutProperty->HasSpace());
    EXPECT_EQ(ColumnModelNG::GetSpace(raw), 10.0);

    // column_model_ng.cpp:87-F,90 - negative space keeps the previous value.
    ColumnModelNG::SetSpace(raw, Dimension(-5.0, DimensionUnit::VP));
    EXPECT_EQ(ColumnModelNG::GetSpace(raw), 10.0);

    // column_model_ng.cpp:86-F - empty optional returns early.
    ColumnModelNG::SetSpace(raw, std::nullopt);
    EXPECT_EQ(ColumnModelNG::GetSpace(raw), 10.0);
}

/**
 * @tc.name: ColumnVisualStateSetters
 * @tc.desc: Cover the ViewStackProcessor based overloads of ColumnModelNG::SetAlignItems /
 *           SetJustifyContent / SetIsReverse for an empty stack (null main frame node) and for a
 *           main frame node whose layout property cannot be cast to LinearLayoutProperty.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, ColumnVisualStateSetters, TestSize.Level0)
{
    ColumnModelNG model;
    // column_model_ng.cpp:152,157,162 - GetMainFrameNode() == nullptr arm (empty stack).
    EXPECT_TRUE(ViewStackProcessor::GetInstance()->IsEmpty());
    model.SetAlignItems(FlexAlign::FLEX_START);
    model.SetJustifyContent(FlexAlign::CENTER);
    model.SetIsReverse(true);

    // column_model_ng.cpp:152,157,162 - cast false arm.
    auto propertyless = CreateNodeWithoutLayoutProperty(true);
    ViewStackProcessor::GetInstance()->Push(propertyless);
    EXPECT_EQ(ViewStackProcessor::GetInstance()->GetMainFrameNode(), AceType::RawPtr(propertyless));
    model.SetAlignItems(FlexAlign::FLEX_START);
    model.SetJustifyContent(FlexAlign::CENTER);
    model.SetIsReverse(true);
    // PopContainer() keeps the last stack entry as the implicit root, so clear explicitly here.
    ViewStackProcessor::GetInstance()->ClearStack();
    EXPECT_TRUE(ViewStackProcessor::GetInstance()->IsEmpty());

    // column_model_ng.cpp:152,157,162 - cast true arm on a real Column node.
    ColumnModelNG pushedModel;
    pushedModel.Create(std::nullopt, nullptr, "");
    pushedModel.SetAlignItems(FlexAlign::FLEX_END);
    pushedModel.SetJustifyContent(FlexAlign::FLEX_END);
    pushedModel.SetIsReverse(true);
    auto frameNode = PopFrameNodeFromStack();
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<LinearLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetCrossAxisAlignValue(FlexAlign::AUTO), FlexAlign::FLEX_END);
    EXPECT_EQ(layoutProperty->GetMainAxisAlignValue(FlexAlign::AUTO), FlexAlign::FLEX_END);
    EXPECT_TRUE(layoutProperty->GetIsReverseValue(false));
}

// ---------------------------------------------------------------------------------------------
// RowModelNG: Create() overloads (row_model_ng.cpp:24-41)
// ---------------------------------------------------------------------------------------------

/**
 * @tc.name: RowCreateWithSpace
 * @tc.desc: Pin the space handling of RowModelNG::Create for legal, negative and absent space.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, RowCreateWithSpace, TestSize.Level0)
{
    // row_model_ng.cpp:33,35-T,36-T,37
    {
        RowModelNG model;
        model.Create(Dimension(10.0, DimensionUnit::VP), nullptr, "");
        auto frameNode = PopFrameNodeFromStack();
        ASSERT_NE(frameNode, nullptr);
        auto layoutProperty = frameNode->GetLayoutProperty<LinearLayoutProperty>();
        ASSERT_NE(layoutProperty, nullptr);
        EXPECT_EQ(layoutProperty->GetFlexDirectionValue(FlexDirection::COLUMN), FlexDirection::ROW);
        EXPECT_TRUE(layoutProperty->HasSpace());
        EXPECT_EQ(RowModelNG::GetSpace(AceType::RawPtr(frameNode)), 10.0);
    }
    // row_model_ng.cpp:36-F,39
    {
        RowModelNG model;
        model.Create(Dimension(-5.0, DimensionUnit::VP), nullptr, "");
        auto frameNode = PopFrameNodeFromStack();
        ASSERT_NE(frameNode, nullptr);
        auto layoutProperty = frameNode->GetLayoutProperty<LinearLayoutProperty>();
        ASSERT_NE(layoutProperty, nullptr);
        EXPECT_EQ(layoutProperty->GetFlexDirectionValue(FlexDirection::COLUMN), FlexDirection::ROW);
        EXPECT_FALSE(layoutProperty->HasSpace());
        EXPECT_EQ(RowModelNG::GetSpace(AceType::RawPtr(frameNode)), 0.0);
    }
    // row_model_ng.cpp:35-F
    {
        auto frameNode = CreateRow([](RowModelNG model) {});
        ASSERT_NE(frameNode, nullptr);
        EXPECT_FALSE(frameNode->GetLayoutProperty<LinearLayoutProperty>()->HasSpace());
    }
}

// ---------------------------------------------------------------------------------------------
// RowModelNG: FrameNode based setters / getters (row_model_ng.cpp:85-212)
// ---------------------------------------------------------------------------------------------

/**
 * @tc.name: RowFrameNodeSettersAndGetters
 * @tc.desc: Cover every FrameNode* static API of RowModelNG, including the Row only
 *           CHECK_NULL_VOID(frameNode) guards of SetJustifyContent / SetAlignItems, the
 *           CHECK_NULL_RETURN(nullptr) getters and the null layout property macro arms.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, RowFrameNodeSettersAndGetters, TestSize.Level0)
{
    // row_model_ng.cpp:141-150 - CreateFrameNode() builds a ROW node and updates FlexDirection.
    auto frameNode = RowModelNG::CreateFrameNode(ElementRegister::GetInstance()->MakeUniqueId());
    ASSERT_NE(frameNode, nullptr);
    EXPECT_EQ(frameNode->GetTag(), V2::ROW_ETS_TAG);
    auto layoutProperty = frameNode->GetLayoutProperty<LinearLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetFlexDirectionValue(FlexDirection::COLUMN), FlexDirection::ROW);
    auto* raw = AceType::RawPtr(frameNode);

    // row_model_ng.cpp:184-196 - defaults when the attributes are absent.
    EXPECT_EQ(RowModelNG::GetJustifyContent(raw), FlexAlign::AUTO);
    EXPECT_EQ(RowModelNG::GetAlignItems(raw), FlexAlign::CENTER);
    // row_model_ng.cpp:198-204 - default space is 0.
    EXPECT_EQ(RowModelNG::GetSpace(raw), 0.0);
    // row_model_ng.cpp:206-211 - default reverse is false.
    EXPECT_FALSE(RowModelNG::GetIsReverse(raw));

    // row_model_ng.cpp:167-182 - static FrameNode setters.
    RowModelNG::SetJustifyContent(raw, FlexAlign::FLEX_START);
    RowModelNG::SetAlignItems(raw, FlexAlign::FLEX_END);
    RowModelNG::SetIsReverse(raw, true);
    RowModelNG::SetSpace(raw, Dimension(9.0, DimensionUnit::VP));
    EXPECT_EQ(RowModelNG::GetJustifyContent(raw), FlexAlign::FLEX_START);
    EXPECT_EQ(RowModelNG::GetAlignItems(raw), FlexAlign::FLEX_END);
    EXPECT_TRUE(RowModelNG::GetIsReverse(raw));
    EXPECT_EQ(RowModelNG::GetSpace(raw), 9.0);

    // row_model_ng.cpp:201,209 - CHECK_NULL_RETURN(nullptr) arms.
    EXPECT_EQ(RowModelNG::GetSpace(nullptr), 0.0);
    EXPECT_FALSE(RowModelNG::GetIsReverse(nullptr));
    // row_model_ng.cpp:174,180 - Row only CHECK_NULL_VOID(frameNode) guards.
    RowModelNG::SetJustifyContent(nullptr, FlexAlign::CENTER);
    RowModelNG::SetAlignItems(nullptr, FlexAlign::CENTER);
    // row_model_ng.cpp:169,88,89 - macro null arms.
    RowModelNG::SetIsReverse(nullptr, true);
    RowModelNG::SetSpace(nullptr, Dimension(10.0, DimensionUnit::VP));
    RowModelNG::SetSpace(nullptr, std::nullopt);

    // row_model_ng.cpp:169,175,181,187,194,202,210 - "if (castLayoutProperty)" false arms.
    auto propertyless = CreateNodeWithoutLayoutProperty(false);
    ASSERT_NE(propertyless, nullptr);
    auto* propertylessRaw = AceType::RawPtr(propertyless);
    RowModelNG::SetIsReverse(propertylessRaw, true);
    RowModelNG::SetJustifyContent(propertylessRaw, FlexAlign::CENTER);
    RowModelNG::SetAlignItems(propertylessRaw, FlexAlign::CENTER);
    RowModelNG::SetSpace(propertylessRaw, Dimension(10.0, DimensionUnit::VP));
    EXPECT_EQ(RowModelNG::GetJustifyContent(propertylessRaw), FlexAlign::AUTO);
    EXPECT_EQ(RowModelNG::GetAlignItems(propertylessRaw), FlexAlign::CENTER);
    EXPECT_EQ(RowModelNG::GetSpace(propertylessRaw), 0.0);
    EXPECT_FALSE(RowModelNG::GetIsReverse(propertylessRaw));
}

/**
 * @tc.name: RowSetSpaceFrameNodeOptional
 * @tc.desc: Pin the std::optional<Dimension> overload of RowModelNG::SetSpace.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, RowSetSpaceFrameNodeOptional, TestSize.Level0)
{
    auto frameNode = CreateRow([](RowModelNG model) {});
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<LinearLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    auto* raw = AceType::RawPtr(frameNode);

    // row_model_ng.cpp:87-T,88-T,89
    RowModelNG::SetSpace(raw, Dimension(10.0, DimensionUnit::VP));
    EXPECT_TRUE(layoutProperty->HasSpace());
    EXPECT_EQ(RowModelNG::GetSpace(raw), 10.0);
    // row_model_ng.cpp:88-F,91
    RowModelNG::SetSpace(raw, Dimension(-5.0, DimensionUnit::VP));
    EXPECT_EQ(RowModelNG::GetSpace(raw), 10.0);
    // row_model_ng.cpp:87-F
    RowModelNG::SetSpace(raw, std::nullopt);
    EXPECT_EQ(RowModelNG::GetSpace(raw), 10.0);
}

/**
 * @tc.name: RowVisualStateSetters
 * @tc.desc: Cover the ViewStackProcessor based overloads of RowModelNG::SetAlignItems /
 *           SetJustifyContent / SetIsReverse for the empty-stack, cast-false and cast-true cases.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, RowVisualStateSetters, TestSize.Level0)
{
    RowModelNG model;
    // row_model_ng.cpp:154,159,164 - null main frame node arm.
    EXPECT_TRUE(ViewStackProcessor::GetInstance()->IsEmpty());
    model.SetAlignItems(FlexAlign::FLEX_START);
    model.SetJustifyContent(FlexAlign::CENTER);
    model.SetIsReverse(true);

    // row_model_ng.cpp:154,159,164 - cast false arm.
    auto propertyless = CreateNodeWithoutLayoutProperty(false);
    ViewStackProcessor::GetInstance()->Push(propertyless);
    model.SetAlignItems(FlexAlign::FLEX_START);
    model.SetJustifyContent(FlexAlign::CENTER);
    model.SetIsReverse(true);
    // PopContainer() keeps the last stack entry as the implicit root, so clear explicitly here.
    ViewStackProcessor::GetInstance()->ClearStack();
    EXPECT_TRUE(ViewStackProcessor::GetInstance()->IsEmpty());

    // row_model_ng.cpp:154,159,164 - cast true arm on a real Row node.
    RowModelNG pushedModel;
    pushedModel.Create(std::nullopt, nullptr, "");
    pushedModel.SetAlignItems(FlexAlign::FLEX_END);
    pushedModel.SetJustifyContent(FlexAlign::FLEX_START);
    pushedModel.SetIsReverse(true);
    auto frameNode = PopFrameNodeFromStack();
    ASSERT_NE(frameNode, nullptr);
    auto layoutProperty = frameNode->GetLayoutProperty<LinearLayoutProperty>();
    ASSERT_NE(layoutProperty, nullptr);
    EXPECT_EQ(layoutProperty->GetCrossAxisAlignValue(FlexAlign::AUTO), FlexAlign::FLEX_END);
    EXPECT_EQ(layoutProperty->GetMainAxisAlignValue(FlexAlign::AUTO), FlexAlign::FLEX_START);
    EXPECT_TRUE(layoutProperty->GetIsReverseValue(false));
}

// ---------------------------------------------------------------------------------------------
// Resource-object overloads (column_model_ng.cpp:43-126, row_model_ng.cpp:43-128)
// ---------------------------------------------------------------------------------------------

/**
 * @tc.name: ColumnResourceCreateSpace
 * @tc.desc: Cover ColumnModelNG::Create(RefPtr<ResourceObject>): null resource early return, eager
 *           write of a resolved positive resource, the resource-update lambda cache-miss / cache-hit
 *           paths, positive cache replay and the "resolved value is negative" arms. The negative
 *           eager arm must not write the property at all.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, ColumnResourceCreateSpace, TestSize.Level0)
{
    // column_model_ng.cpp:54 - CHECK_NULL_VOID(spaceResObj) arm.
    {
        ColumnModelNG model;
        model.Create(RefPtr<ResourceObject>(nullptr), nullptr, "");
        auto frameNode = PopFrameNodeFromStack();
        ASSERT_NE(frameNode, nullptr);
        EXPECT_FALSE(frameNode->GetLayoutProperty<LinearLayoutProperty>()->HasSpace());
    }

    // column_model_ng.cpp:79-T,80 - a resolved positive resource is written eagerly.
    auto positiveRes = CreateDimensionResObj(RES_COLUMN_CREATE_POSITIVE, Dimension(8.0));
    CalcDimension probe;
    ASSERT_TRUE(ResourceParseUtils::ParseResDimensionVp(positiveRes, probe));
    ASSERT_EQ(probe.Value(), 8.0);
    ColumnModelNG positiveModel;
    positiveModel.Create(positiveRes, nullptr, "");
    auto positiveNode = PopFrameNodeFromStack();
    ASSERT_NE(positiveNode, nullptr);
    auto positiveProperty = positiveNode->GetLayoutProperty<LinearLayoutProperty>();
    auto positivePattern = positiveNode->GetPattern<LinearLayoutPattern>();
    ASSERT_NE(positiveProperty, nullptr);
    ASSERT_NE(positivePattern, nullptr);
    EXPECT_TRUE(positiveProperty->HasSpace());
    EXPECT_EQ(ColumnModelNG::GetSpace(AceType::RawPtr(positiveNode)), 8.0);
    // column_model_ng.cpp:65-T,66,67,71-T,72,73 - first replay fills the cache from the resource.
    positivePattern->OnColorModeChange(1);
    EXPECT_EQ(ColumnModelNG::GetSpace(AceType::RawPtr(positiveNode)), 8.0);
    // column_model_ng.cpp:65-F,69 - second replay reads the cached string instead of the resource.
    positivePattern->OnColorModeChange(1);
    EXPECT_EQ(ColumnModelNG::GetSpace(AceType::RawPtr(positiveNode)), 8.0);
    // column_model_ng.cpp:65-F,69,71-T - cache replay applies a new legal value.
    positivePattern->AddResCache("column.space", "13vp");
    positivePattern->OnColorModeChange(1);
    EXPECT_EQ(ColumnModelNG::GetSpace(AceType::RawPtr(positiveNode)), 13.0);
    // column_model_ng.cpp:71-F - cached negative value is ignored (no write, no reset).
    positivePattern->AddResCache("column.space", "-4vp");
    positivePattern->OnColorModeChange(1);
    EXPECT_EQ(ColumnModelNG::GetSpace(AceType::RawPtr(positiveNode)), 13.0);

    // column_model_ng.cpp:65-T,71-F,79-F - negative resolved resource neither writes nor resets.
    auto negativeRes = CreateDimensionResObj(RES_COLUMN_CREATE_NEGATIVE, Dimension(-6.0));
    ColumnModelNG negativeModel;
    negativeModel.Create(negativeRes, nullptr, "");
    auto negativeNode = PopFrameNodeFromStack();
    ASSERT_NE(negativeNode, nullptr);
    auto negativeProperty = negativeNode->GetLayoutProperty<LinearLayoutProperty>();
    auto negativePattern = negativeNode->GetPattern<LinearLayoutPattern>();
    ASSERT_NE(negativeProperty, nullptr);
    ASSERT_NE(negativePattern, nullptr);
    EXPECT_FALSE(negativeProperty->HasSpace());
    negativePattern->OnColorModeChange(1);
    EXPECT_FALSE(negativeProperty->HasSpace());
    EXPECT_EQ(ColumnModelNG::GetSpace(AceType::RawPtr(negativeNode)), 0.0);
}

/**
 * @tc.name: RowResourceCreateSpace
 * @tc.desc: Cover RowModelNG::Create(RefPtr<ResourceObject>). Unlike Column, Row invokes the update
 *           lambda immediately (row_model_ng.cpp:76) so cache-miss and cache-hit arms are both
 *           reachable without an extra replay; a resolved negative resource must be ignored.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, RowResourceCreateSpace, TestSize.Level0)
{
    // row_model_ng.cpp:54 - CHECK_NULL_VOID(spaceResObj) arm.
    {
        RowModelNG model;
        model.Create(RefPtr<ResourceObject>(nullptr), nullptr, "");
        auto frameNode = PopFrameNodeFromStack();
        ASSERT_NE(frameNode, nullptr);
        EXPECT_FALSE(frameNode->GetLayoutProperty<LinearLayoutProperty>()->HasSpace());
    }

    // row_model_ng.cpp:65-T,71-T,72,73,76,80-T - immediate lambda + eager write.
    auto positiveRes = CreateDimensionResObj(RES_ROW_CREATE_POSITIVE, Dimension(9.0));
    RowModelNG positiveModel;
    positiveModel.Create(positiveRes, nullptr, "");
    auto positiveNode = PopFrameNodeFromStack();
    ASSERT_NE(positiveNode, nullptr);
    auto positiveProperty = positiveNode->GetLayoutProperty<LinearLayoutProperty>();
    auto positivePattern = positiveNode->GetPattern<LinearLayoutPattern>();
    ASSERT_NE(positiveProperty, nullptr);
    ASSERT_NE(positivePattern, nullptr);
    EXPECT_TRUE(positiveProperty->HasSpace());
    EXPECT_EQ(RowModelNG::GetSpace(AceType::RawPtr(positiveNode)), 9.0);
    // row_model_ng.cpp:65-F - cache hit.
    positivePattern->OnColorModeChange(1);
    EXPECT_EQ(RowModelNG::GetSpace(AceType::RawPtr(positiveNode)), 9.0);
    // row_model_ng.cpp:65-F,71-T - cache replay applies a new legal value.
    positivePattern->AddResCache("row.space", "21vp");
    positivePattern->OnColorModeChange(1);
    EXPECT_EQ(RowModelNG::GetSpace(AceType::RawPtr(positiveNode)), 21.0);
    // row_model_ng.cpp:71-F - cached negative value is ignored.
    positivePattern->AddResCache("row.space", "-3vp");
    positivePattern->OnColorModeChange(1);
    EXPECT_EQ(RowModelNG::GetSpace(AceType::RawPtr(positiveNode)), 21.0);

    // row_model_ng.cpp:65-T,71-F,80-F - negative resource ignored by both lambda and eager write.
    auto negativeRes = CreateDimensionResObj(RES_ROW_CREATE_NEGATIVE, Dimension(-7.0));
    RowModelNG negativeModel;
    negativeModel.Create(negativeRes, nullptr, "");
    auto negativeNode = PopFrameNodeFromStack();
    ASSERT_NE(negativeNode, nullptr);
    auto negativeProperty = negativeNode->GetLayoutProperty<LinearLayoutProperty>();
    ASSERT_NE(negativeProperty, nullptr);
    EXPECT_FALSE(negativeProperty->HasSpace());
    EXPECT_EQ(RowModelNG::GetSpace(AceType::RawPtr(negativeNode)), 0.0);
}

/**
 * @tc.name: ColumnResourceSetSpace
 * @tc.desc: Cover ColumnModelNG::SetSpace(FrameNode*, RefPtr<ResourceObject>): null node early
 *           return, immediate lambda invocation, eager positive write, cache-hit replay and the
 *           reset branch taken when the replayed resource value becomes negative.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, ColumnResourceSetSpace, TestSize.Level0)
{
    auto frameNode = CreateColumn([](ColumnModelNG model) {});
    ASSERT_NE(frameNode, nullptr);
    auto* raw = AceType::RawPtr(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty<LinearLayoutProperty>();
    auto pattern = frameNode->GetPattern<LinearLayoutPattern>();
    ASSERT_NE(layoutProperty, nullptr);
    ASSERT_NE(pattern, nullptr);

    auto positiveRes = CreateDimensionResObj(RES_COLUMN_SET_POSITIVE, Dimension(6.0));
    // column_model_ng.cpp:96 - CHECK_NULL_VOID(frameNode) arm.
    ColumnModelNG::SetSpace(nullptr, positiveRes);

    // column_model_ng.cpp:107-T,108,109,113-T,114,115,119 + 123-T,124 - immediate lambda + eager write.
    ColumnModelNG::SetSpace(raw, positiveRes);
    EXPECT_TRUE(layoutProperty->HasSpace());
    EXPECT_EQ(ColumnModelNG::GetSpace(raw), 6.0);
    // column_model_ng.cpp:107-F,111,113-T - cache hit replay keeps the positive value.
    pattern->OnColorModeChange(1);
    EXPECT_EQ(ColumnModelNG::GetSpace(raw), 6.0);

    // column_model_ng.cpp:123-F - the eager write is skipped for a negative resource.
    auto negativeRes = CreateDimensionResObj(RES_COLUMN_SET_NEGATIVE, Dimension(-9.0));
    ColumnModelNG::SetSpace(raw, negativeRes);
    EXPECT_EQ(ColumnModelNG::GetSpace(raw), 6.0);
    // column_model_ng.cpp:107-T,113-F,117 - replaying the negative resource resets the space.
    pattern->OnColorModeChange(1);
    EXPECT_FALSE(layoutProperty->HasSpace());
    EXPECT_EQ(ColumnModelNG::GetSpace(raw), 0.0);
}

/**
 * @tc.name: RowResourceSetSpace
 * @tc.desc: Cover RowModelNG::SetSpace(FrameNode*, RefPtr<ResourceObject>): null node early return,
 *           immediate lambda invocation, eager positive write, cache-hit replay, the lambda
 *           "negative value does nothing" arm and the Row only LOGE eager arm (row_model_ng.cpp:126).
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, RowResourceSetSpace, TestSize.Level0)
{
    auto frameNode = CreateRow([](RowModelNG model) {});
    ASSERT_NE(frameNode, nullptr);
    auto* raw = AceType::RawPtr(frameNode);
    auto layoutProperty = frameNode->GetLayoutProperty<LinearLayoutProperty>();
    auto pattern = frameNode->GetPattern<LinearLayoutPattern>();
    ASSERT_NE(layoutProperty, nullptr);
    ASSERT_NE(pattern, nullptr);

    auto positiveRes = CreateDimensionResObj(RES_ROW_SET_POSITIVE, Dimension(11.0));
    // row_model_ng.cpp:97 - CHECK_NULL_VOID(frameNode) arm.
    RowModelNG::SetSpace(nullptr, positiveRes);

    // row_model_ng.cpp:108-T,109,110,114-T,115,116,119 + 123-T,124 - immediate lambda + eager write.
    RowModelNG::SetSpace(raw, positiveRes);
    EXPECT_TRUE(layoutProperty->HasSpace());
    EXPECT_EQ(RowModelNG::GetSpace(raw), 11.0);
    // row_model_ng.cpp:108-F,112,114-T - cache hit replay keeps the positive value.
    pattern->OnColorModeChange(1);
    EXPECT_EQ(RowModelNG::GetSpace(raw), 11.0);

    // row_model_ng.cpp:123-F,126 - eager LOGE arm when the resolved resource is negative.
    auto negativeRes = CreateDimensionResObj(RES_ROW_SET_NEGATIVE, Dimension(-5.0));
    RowModelNG::SetSpace(raw, negativeRes);
    EXPECT_EQ(RowModelNG::GetSpace(raw), 11.0);
    // row_model_ng.cpp:108-T,114-F - the Row lambda has no reset arm, the value is kept as-is.
    pattern->OnColorModeChange(1);
    EXPECT_EQ(RowModelNG::GetSpace(raw), 11.0);
    EXPECT_TRUE(layoutProperty->HasSpace());
}

/**
 * @tc.name: ResetResObjectOnConfigChange
 * @tc.desc: Cover the ResourceObject reset paths of ColumnModelNG / RowModelNG: the early return
 *           when config-change handling is disabled, the CHECK_NULL_VOID(frameNode) arm and the real
 *           removal (verified by replaying the resources after ResetResObj: the space must not change).
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, ResetResObjectOnConfigChange, TestSize.Level0)
{
    auto columnNode = CreateColumn([](ColumnModelNG model) {});
    ASSERT_NE(columnNode, nullptr);
    auto* columnRaw = AceType::RawPtr(columnNode);
    auto columnPattern = columnNode->GetPattern<LinearLayoutPattern>();
    ASSERT_NE(columnPattern, nullptr);

    // column_model_ng.cpp:141,144 + row_model_ng.cpp:132,135 - flag disabled early return.
    {
        ScopedConfigChangePerform configChange(false);
        ColumnModelNG::ResetResObj(columnRaw, "column.space");
        RowModelNG::ResetResObj(columnRaw, "row.space");
    }

    // column_model_ng.cpp:141-T,145,146,147 - flag enabled removes the registered updater.
    {
        ScopedConfigChangePerform configChange(true);
        auto columnRes = CreateDimensionResObj(RES_COLUMN_RESET, Dimension(12.0));
        ColumnModelNG::SetSpace(columnRaw, columnRes);
        EXPECT_EQ(ColumnModelNG::GetSpace(columnRaw), 12.0);
        ColumnModelNG::ResetResObj(columnRaw, "column.space");
        columnPattern->AddResCache("column.space", "99vp");
        columnPattern->OnColorModeChange(1);
        EXPECT_EQ(ColumnModelNG::GetSpace(columnRaw), 12.0);
        // column_model_ng.cpp:144 - CHECK_NULL_VOID(frameNode) arm.
        ColumnModelNG::ResetResObj(nullptr, "column.space");

        // row_model_ng.cpp:132-T,136,137,138 - Row mirror.
        auto rowNode = CreateRow([](RowModelNG model) {});
        ASSERT_NE(rowNode, nullptr);
        auto* rowRaw = AceType::RawPtr(rowNode);
        auto rowPattern = rowNode->GetPattern<LinearLayoutPattern>();
        ASSERT_NE(rowPattern, nullptr);
        auto rowRes = CreateDimensionResObj(RES_ROW_RESET, Dimension(7.0));
        RowModelNG::SetSpace(rowRaw, rowRes);
        EXPECT_EQ(RowModelNG::GetSpace(rowRaw), 7.0);
        RowModelNG::ResetResObj(rowRaw, "row.space");
        rowPattern->AddResCache("row.space", "88vp");
        rowPattern->OnColorModeChange(1);
        EXPECT_EQ(RowModelNG::GetSpace(rowRaw), 7.0);
        // row_model_ng.cpp:135 - CHECK_NULL_VOID(frameNode) arm.
        RowModelNG::ResetResObj(nullptr, "row.space");
    }
}

// ---------------------------------------------------------------------------------------------
// LinearLayoutUtils::Measure / Layout (linear_layout_utils.cpp)
// ---------------------------------------------------------------------------------------------

/**
 * @tc.name: LinearLayoutUtilsNullLayoutProperty
 * @tc.desc: Cover the CHECK_NULL_VOID(layoutProperty) guards of LinearLayoutUtils::Measure (:109)
 *           and LinearLayoutUtils::Layout (:200): a wrapper with a null layout property is ignored.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, LinearLayoutUtilsNullLayoutProperty, TestSize.Level0)
{
    auto frameNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto geometryNode = AceType::MakeRefPtr<GeometryNode>();
    auto wrapper = AceType::MakeRefPtr<LayoutWrapperNode>(frameNode, geometryNode, nullptr);
    ASSERT_NE(wrapper, nullptr);

    // linear_layout_utils.cpp:109 / :200
    LinearLayoutUtils::Measure(AceType::RawPtr(wrapper), true);
    EXPECT_EQ(wrapper->GetGeometryNode()->GetFrameSize(), SizeF(0.0f, 0.0f));
    LinearLayoutUtils::Layout(AceType::RawPtr(wrapper), false, FlexAlign::FLEX_START, FlexAlign::FLEX_START);
    EXPECT_EQ(wrapper->GetGeometryNode()->GetFrameOffset(), OffsetF(0.0f, 0.0f));
}

/**
 * @tc.name: LinearLayoutUtilsMeasureWithoutIdealSize
 * @tc.desc: Pin LinearLayoutUtils::Measure when the wrapper has no self ideal size: the measure type
 *           is MATCH_CONTENT so no parent ideal size is used (:121-F,:125-F). Because
 *           linearMeasureProperty.realSize is snapshotted at :127 *before* the max-size update at :131,
 *           it is still invalid at :184, so :185 fills the frame size from the measured children
 *           (wrap-content) instead of from maxSize. Also covers the relative-node measurement loop.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, LinearLayoutUtilsMeasureWithoutIdealSize, TestSize.Level0)
{
    auto container = CreateWrapNodePair(true, std::nullopt, SizeF(400.0f, 200.0f));
    auto childConstraint = CreateChildConstraintFor(container);
    auto child = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    ASSERT_NE(child, nullptr);

    LinearLayoutUtils::Measure(AceType::RawPtr(container.wrapper), true);
    // wrap-content: the container shrinks to the single child (100 x 40), it does NOT take maxSize.
    EXPECT_EQ(container.wrapper->GetGeometryNode()->GetFrameSize(), SizeF(100.0f, 40.0f));
    EXPECT_EQ(child->GetGeometryNode()->GetFrameSize(), SizeF(100.0f, 40.0f));

    // No children at all: both ":152 if (!relativeNodes.empty())" and ":158 if (!weightNodes.empty())"
    // take the false arm, so both main and cross size stay 0 and the frame collapses to 0 x 0.
    auto emptyContainer = CreateWrapNodePair(true, std::nullopt, SizeF(400.0f, 200.0f));
    LinearLayoutUtils::Measure(AceType::RawPtr(emptyContainer.wrapper), true);
    EXPECT_EQ(emptyContainer.wrapper->GetGeometryNode()->GetFrameSize(), SizeF(0.0f, 0.0f));
}

/**
 * @tc.name: LinearLayoutUtilsMeasureWithMatchParent
 * @tc.desc: Pin the MATCH_PARENT arm of LinearLayoutUtils::Measure: a partially defined self ideal
 *           size is invalid (:121-F) so the missing axis is filled from parentIdealSize (:125-T,:126);
 *           the already valid axis must be preserved.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, LinearLayoutUtilsMeasureWithMatchParent, TestSize.Level0)
{
    auto container = CreateWrapNodePair(true, CalcSize(CalcLength(300.0f), std::nullopt),
        SizeF(PARENT_WIDTH, PARENT_HEIGHT), OptionalSizeF(300.0f, 250.0f));
    auto layoutProperty = container.wrapper->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    // Measured with the default measure type first so the stored constraint keeps a partial ideal size.
    layoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);
    layoutProperty->UpdateContentConstraint();
    auto childConstraint = CreateChildConstraintFor(container);
    auto firstChild = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    auto secondChild = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    ASSERT_NE(firstChild, nullptr);
    ASSERT_NE(secondChild, nullptr);

    LinearLayoutUtils::Measure(AceType::RawPtr(container.wrapper), true);
    // Width came from the user defined ideal size, height was filled from parentIdealSize.
    EXPECT_EQ(container.wrapper->GetGeometryNode()->GetFrameSize(), SizeF(300.0f, 250.0f));
    EXPECT_EQ(firstChild->GetGeometryNode()->GetFrameSize(), SizeF(100.0f, 40.0f));
    EXPECT_EQ(secondChild->GetGeometryNode()->GetFrameSize(), SizeF(100.0f, 40.0f));
}

/**
 * @tc.name: LinearLayoutUtilsMeasureWeightsVertical
 * @tc.desc: Pin the weight measurement path of LinearLayoutUtils::Measure on a Column: a non-zero
 *           space is applied between relative nodes (:152-T,:153), the remain size of an overflowing
 *           container goes negative and is replaced by the container main size (:161-T,:162,:163),
 *           two weight nodes split the remain size by weight and the running cross size keeps the
 *           maximum (:176-T,:177-T).
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, LinearLayoutUtilsMeasureWeightsVertical, TestSize.Level0)
{
    auto container = CreateWrapNodePair(true, CalcSize(CalcLength(PARENT_WIDTH), CalcLength(200.0f)),
        SizeF(PARENT_WIDTH, PARENT_HEIGHT));
    auto flexProperty = AceType::DynamicCast<LinearLayoutProperty>(container.wrapper->GetLayoutProperty());
    ASSERT_NE(flexProperty, nullptr);
    flexProperty->UpdateSpace(Dimension(10.0));
    auto childConstraint = CreateChildConstraintFor(container);
    // Main axis overflows the 200 high container: 300 + 2 * 10 > 200.
    auto tallChild = AppendChildWrap(container, childConstraint, 100.0f, 300.0f);
    // Own main sizes (30) differ from the weight shares (50 / 150) so the assertions prove the split.
    auto firstWeight = AppendChildWrap(container, childConstraint, 150.0f, 30.0f, 1.0f);
    auto secondWeight = AppendChildWrap(container, childConstraint, 100.0f, 30.0f, 3.0f);
    ASSERT_NE(tallChild, nullptr);
    ASSERT_NE(firstWeight, nullptr);
    ASSERT_NE(secondWeight, nullptr);

    LinearLayoutUtils::Measure(AceType::RawPtr(container.wrapper), true);
    EXPECT_EQ(container.wrapper->GetGeometryNode()->GetFrameSize(), SizeF(PARENT_WIDTH, 200.0f));
    EXPECT_EQ(tallChild->GetGeometryNode()->GetFrameSize(), SizeF(100.0f, 300.0f));
    // remainSize is reset to the container main size (200): weights 1 and 3 split it 50 / 150.
    EXPECT_EQ(firstWeight->GetGeometryNode()->GetFrameSize(), SizeF(150.0f, 50.0f));
    EXPECT_EQ(secondWeight->GetGeometryNode()->GetFrameSize(), SizeF(100.0f, 150.0f));
}

/**
 * @tc.name: LinearLayoutUtilsMeasureWeightsHorizontal
 * @tc.desc: Pin the horizontal (isVertical == false) arms of LinearLayoutUtils::Measure: main axis
 *           helpers read Width() (:25-T,:46-T), SetIdealMainSize writes the width (:55-T) and
 *           CreateSize keeps the main/cross order (:64-T). A negative and a positive remain size are
 *           both exercised.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, LinearLayoutUtilsMeasureWeightsHorizontal, TestSize.Level0)
{
    // Positive remain size: 300 - 100 - 5 = 195 is handed to the single weight child.
    {
        auto container = CreateWrapNodePair(false, CalcSize(CalcLength(300.0f), CalcLength(100.0f)),
            SizeF(PARENT_WIDTH, PARENT_HEIGHT));
        auto flexProperty = AceType::DynamicCast<LinearLayoutProperty>(container.wrapper->GetLayoutProperty());
        ASSERT_NE(flexProperty, nullptr);
        flexProperty->UpdateSpace(Dimension(5.0));
        auto childConstraint = CreateChildConstraintFor(container);
        auto relative = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
        auto weight = AppendChildWrap(container, childConstraint, 100.0f, 50.0f, 1.0f);
        ASSERT_NE(relative, nullptr);
        ASSERT_NE(weight, nullptr);

        LinearLayoutUtils::Measure(AceType::RawPtr(container.wrapper), false);
        EXPECT_EQ(container.wrapper->GetGeometryNode()->GetFrameSize(), SizeF(300.0f, 100.0f));
        EXPECT_EQ(relative->GetGeometryNode()->GetFrameSize(), SizeF(100.0f, 40.0f));
        EXPECT_EQ(weight->GetGeometryNode()->GetFrameSize(), SizeF(195.0f, 50.0f));
    }
    // Negative remain size: 300 - 400 - 5 < 0, the container main size is used instead (300).
    {
        auto container = CreateWrapNodePair(false, CalcSize(CalcLength(300.0f), CalcLength(100.0f)),
            SizeF(PARENT_WIDTH, PARENT_HEIGHT));
        auto flexProperty = AceType::DynamicCast<LinearLayoutProperty>(container.wrapper->GetLayoutProperty());
        ASSERT_NE(flexProperty, nullptr);
        flexProperty->UpdateSpace(Dimension(5.0));
        auto childConstraint = CreateChildConstraintFor(container);
        auto oversized = AppendChildWrap(container, childConstraint, 400.0f, 40.0f);
        auto weight = AppendChildWrap(container, childConstraint, 100.0f, 50.0f, 1.0f);
        ASSERT_NE(oversized, nullptr);
        ASSERT_NE(weight, nullptr);

        LinearLayoutUtils::Measure(AceType::RawPtr(container.wrapper), false);
        EXPECT_EQ(oversized->GetGeometryNode()->GetFrameSize(), SizeF(400.0f, 40.0f));
        EXPECT_EQ(weight->GetGeometryNode()->GetFrameSize(), SizeF(300.0f, 50.0f));
    }
}

/**
 * @tc.name: LinearLayoutUtilsMeasureNullChildProperty
 * @tc.desc: Cover the CHECK_NULL_CONTINUE(childLayoutProperty) arm of TravelChildrenFlexProps
 *           (:74): a child wrapper without a layout property is skipped, the container size is not
 *           affected and the remaining children are still measured.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, LinearLayoutUtilsMeasureNullChildProperty, TestSize.Level0)
{
    auto container = CreateWrapNodePair(true, CalcSize(CalcLength(PARENT_WIDTH), CalcLength(200.0f)),
        SizeF(PARENT_WIDTH, PARENT_HEIGHT));
    auto childConstraint = CreateChildConstraintFor(container);
    auto normalChild = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    auto propertylessChild = AppendChildWrapWithoutProperty(container);
    ASSERT_NE(normalChild, nullptr);
    ASSERT_NE(propertylessChild, nullptr);

    LinearLayoutUtils::Measure(AceType::RawPtr(container.wrapper), true);
    EXPECT_EQ(container.wrapper->GetGeometryNode()->GetFrameSize(), SizeF(PARENT_WIDTH, 200.0f));
    EXPECT_EQ(normalChild->GetGeometryNode()->GetFrameSize(), SizeF(100.0f, 40.0f));
    // The propertyless child was skipped by TravelChildrenFlexProps and never measured.
    EXPECT_EQ(propertylessChild->GetGeometryNode()->GetFrameSize(), SizeF(0.0f, 0.0f));
}

/**
 * @tc.name: LinearLayoutUtilsMeasurePercentSpace
 * @tc.desc: Cover the ConvertToPx() value_or(nullopt) arm for the space (:138): a PERCENT space has
 *           no percent reference here, so it is silently dropped and the weight child receives the
 *           full remain size.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, LinearLayoutUtilsMeasurePercentSpace, TestSize.Level0)
{
    auto container = CreateWrapNodePair(true, CalcSize(CalcLength(PARENT_WIDTH), CalcLength(200.0f)),
        SizeF(PARENT_WIDTH, PARENT_HEIGHT));
    auto flexProperty = AceType::DynamicCast<LinearLayoutProperty>(container.wrapper->GetLayoutProperty());
    ASSERT_NE(flexProperty, nullptr);
    flexProperty->UpdateSpace(Dimension(50.0, DimensionUnit::PERCENT));
    auto childConstraint = CreateChildConstraintFor(container);
    auto relative = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    auto weight = AppendChildWrap(container, childConstraint, 100.0f, 50.0f, 1.0f);
    ASSERT_NE(relative, nullptr);
    ASSERT_NE(weight, nullptr);

    LinearLayoutUtils::Measure(AceType::RawPtr(container.wrapper), true);
    // 200 - 40 - 0 == 160 proves the percent space was converted to 0.
    EXPECT_EQ(weight->GetGeometryNode()->GetFrameSize(), SizeF(100.0f, 160.0f));
    EXPECT_EQ(container.wrapper->GetGeometryNode()->GetFrameSize(), SizeF(PARENT_WIDTH, 200.0f));
}

/**
 * @tc.name: LinearLayoutUtilsPlainLayoutProperty
 * @tc.desc: Cover the DynamicCast<LinearLayoutProperty> failure arms (:137-F,:203-F,:213-F): with a
 *           plain LayoutProperty the space is forced to 0, the direction resolves through the global
 *           RTL flag and Layout still produces the expected offsets.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, LinearLayoutUtilsPlainLayoutProperty, TestSize.Level0)
{
    ScopedRightToLeft rightToLeft(false);
    auto frameNode = FrameNode::CreateFrameNode(V2::COLUMN_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<LinearLayoutPattern>(true));
    auto plainProperty = AceType::MakeRefPtr<LayoutProperty>();
    auto wrapper = AceType::MakeRefPtr<LayoutWrapperNode>(
        frameNode, AceType::MakeRefPtr<GeometryNode>(), plainProperty);
    plainProperty->UpdateUserDefinedIdealSize(CalcSize(CalcLength(PARENT_WIDTH), CalcLength(200.0f)));
    LayoutConstraintF parentConstraint;
    parentConstraint.maxSize = SizeF(PARENT_WIDTH, PARENT_HEIGHT);
    parentConstraint.percentReference = SizeF(PARENT_WIDTH, PARENT_HEIGHT);
    plainProperty->UpdateLayoutConstraint(parentConstraint);
    plainProperty->UpdateContentConstraint();
    auto childConstraint = plainProperty->CreateChildConstraint();
    childConstraint.maxSize = SizeF(PARENT_WIDTH, PARENT_HEIGHT);
    childConstraint.minSize = SizeF(0.0f, 0.0f);
    auto container = WrapNodePair { frameNode, wrapper };
    auto firstChild = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    auto secondChild = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    ASSERT_NE(firstChild, nullptr);
    ASSERT_NE(secondChild, nullptr);

    LinearLayoutUtils::Measure(AceType::RawPtr(wrapper), true);
    EXPECT_EQ(wrapper->GetGeometryNode()->GetFrameSize(), SizeF(PARENT_WIDTH, 200.0f));
    LinearLayoutUtils::Layout(AceType::RawPtr(wrapper), true, FlexAlign::FLEX_START, FlexAlign::FLEX_START);
    EXPECT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0f, 0.0f));
    EXPECT_EQ(secondChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0f, 40.0f));
}

/**
 * @tc.name: LinearLayoutUtilsLayoutFlexStartVertical
 * @tc.desc: Pin the vertical FLEX_START early-return path of LinearLayoutUtils::Layout
 *           (:220-T,:221-:229) and all four CalculateCrossOffset arms by replaying the layout with
 *           FLEX_START / FLEX_END / CENTER / SPACE_BETWEEN cross alignment. Space is applied between
 *           the children.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, LinearLayoutUtilsLayoutFlexStartVertical, TestSize.Level0)
{
    ScopedRightToLeft rightToLeft(false);
    auto container = CreateWrapNodePair(true, CalcSize(CalcLength(PARENT_WIDTH), CalcLength(200.0f)),
        SizeF(PARENT_WIDTH, PARENT_HEIGHT));
    auto flexProperty = AceType::DynamicCast<LinearLayoutProperty>(container.wrapper->GetLayoutProperty());
    ASSERT_NE(flexProperty, nullptr);
    flexProperty->UpdateSpace(Dimension(10.0));
    auto childConstraint = CreateChildConstraintFor(container);
    auto firstChild = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    auto secondChild = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    ASSERT_NE(firstChild, nullptr);
    ASSERT_NE(secondChild, nullptr);
    LinearLayoutUtils::Measure(AceType::RawPtr(container.wrapper), true);

    // linear_layout_utils.cpp:213-T,214-T,215-F,:88 case FLEX_START,:218-T,:220-T
    LinearLayoutUtils::Layout(
        AceType::RawPtr(container.wrapper), true, FlexAlign::FLEX_START, FlexAlign::FLEX_START);
    EXPECT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0f, 0.0f));
    EXPECT_EQ(secondChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0f, 50.0f));

    // :92 case FLEX_END
    LinearLayoutUtils::Layout(AceType::RawPtr(container.wrapper), true, FlexAlign::FLEX_END, FlexAlign::FLEX_START);
    EXPECT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(620.0f, 0.0f));
    EXPECT_EQ(secondChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(620.0f, 50.0f));

    // :95 case CENTER
    LinearLayoutUtils::Layout(AceType::RawPtr(container.wrapper), true, FlexAlign::CENTER, FlexAlign::FLEX_START);
    EXPECT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(310.0f, 0.0f));
    EXPECT_EQ(secondChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(310.0f, 50.0f));

    // :98 default arm (SPACE_AROUND is neither FLEX_START, FLEX_END nor CENTER).
    LinearLayoutUtils::Layout(
        AceType::RawPtr(container.wrapper), true, FlexAlign::SPACE_AROUND, FlexAlign::FLEX_START);
    EXPECT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0f, 0.0f));
    EXPECT_EQ(secondChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0f, 50.0f));

    // :231-:234 a non FLEX_START main alignment goes through LayoutConditions.
    // CENTER: blankSpace = (200 - 80 - 10) / 2 = 55, space is added between the children.
    LinearLayoutUtils::Layout(AceType::RawPtr(container.wrapper), true, FlexAlign::FLEX_START, FlexAlign::CENTER);
    EXPECT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0f, 55.0f));
    EXPECT_EQ(secondChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0f, 105.0f));
}

/**
 * @tc.name: LinearLayoutUtilsLayoutFlexStartHorizontal
 * @tc.desc: Pin the horizontal FLEX_START early-return path of LinearLayoutUtils::Layout
 *           (:237-T,:238-:246) and the cross axis offsets computed from the frame height
 *           (:240,:241,:243,:244).
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, LinearLayoutUtilsLayoutFlexStartHorizontal, TestSize.Level0)
{
    ScopedRightToLeft rightToLeft(false);
    auto container = CreateWrapNodePair(false, CalcSize(CalcLength(PARENT_WIDTH), CalcLength(100.0f)),
        SizeF(PARENT_WIDTH, 100.0f));
    auto flexProperty = AceType::DynamicCast<LinearLayoutProperty>(container.wrapper->GetLayoutProperty());
    ASSERT_NE(flexProperty, nullptr);
    flexProperty->UpdateSpace(Dimension(10.0));
    auto childConstraint = CreateChildConstraintFor(container);
    auto firstChild = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    auto secondChild = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    ASSERT_NE(firstChild, nullptr);
    ASSERT_NE(secondChild, nullptr);
    LinearLayoutUtils::Measure(AceType::RawPtr(container.wrapper), false);

    // :237-T,:238-:246 FLEX_START cross alignment
    LinearLayoutUtils::Layout(
        AceType::RawPtr(container.wrapper), false, FlexAlign::FLEX_START, FlexAlign::FLEX_START);
    EXPECT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0f, 0.0f));
    EXPECT_EQ(secondChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(110.0f, 0.0f));

    // Cross FLEX_END: y = 100 - 40 = 60
    LinearLayoutUtils::Layout(
        AceType::RawPtr(container.wrapper), false, FlexAlign::FLEX_END, FlexAlign::FLEX_START);
    EXPECT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0f, 60.0f));
    EXPECT_EQ(secondChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(110.0f, 60.0f));

    // Cross CENTER: y = (100 - 40) / 2 = 30
    LinearLayoutUtils::Layout(AceType::RawPtr(container.wrapper), false, FlexAlign::CENTER, FlexAlign::FLEX_START);
    EXPECT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0f, 30.0f));

    // :248-:249 a non FLEX_START main alignment goes through LayoutConditions.
    // CENTER: blankSpace = (720 - 200 - 10) / 2 = 255, space is added between the children.
    LinearLayoutUtils::Layout(AceType::RawPtr(container.wrapper), false, FlexAlign::FLEX_START, FlexAlign::CENTER);
    EXPECT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(255.0f, 0.0f));
    EXPECT_EQ(secondChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(365.0f, 0.0f));
}

/**
 * @tc.name: LinearLayoutUtilsLayoutConditionVertical
 * @tc.desc: Pin every main axis alignment arm of the vertical LayoutCondition path
 *           (:255-:304, cases CENTER / FLEX_END / SPACE_BETWEEN / SPACE_AROUND / SPACE_EVENLY /
 *           default) including the three paths of the "CENTER || FLEX_END" decision (:297-:298) and
 *           the cross offset switch re-entry (:291,:292).
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, LinearLayoutUtilsLayoutConditionVertical, TestSize.Level0)
{
    auto container = CreateWrapNodePair(true, CalcSize(CalcLength(PARENT_WIDTH), CalcLength(200.0f)),
        SizeF(PARENT_WIDTH, PARENT_HEIGHT));
    auto childConstraint = CreateChildConstraintFor(container);
    auto firstChild = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    auto secondChild = AppendChildWrap(container, childConstraint, 100.0f, 60.0f);
    ASSERT_NE(firstChild, nullptr);
    ASSERT_NE(secondChild, nullptr);
    LinearLayoutUtils::Measure(AceType::RawPtr(container.wrapper), true);
    auto children = GetChildren(container);

    auto layoutWith = [&children, &firstChild, &secondChild](FlexAlign mainAlign, float expectedFirstY,
                            float expectedSecondY) {
        LayoutConditions conditions;
        conditions.isVertical = true;
        conditions.crossAlign = FlexAlign::FLEX_START;
        conditions.mainAlign = mainAlign;
        conditions.size = SizeF(PARENT_WIDTH, 200.0f);
        conditions.paddingOffset = OffsetF(0.0f, 0.0f);
        conditions.space = 10.0f;
        LinearLayoutUtils::LayoutCondition(children, TextDirection::LTR, conditions);
        EXPECT_FLOAT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset().GetY(), expectedFirstY);
        EXPECT_FLOAT_EQ(secondChild->GetGeometryNode()->GetMarginFrameOffset().GetY(), expectedSecondY);
        EXPECT_FLOAT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset().GetX(), 0.0f);
    };

    // :286 default arm - blankSpace is 0, yPos stays 0.
    layoutWith(FlexAlign::AUTO, 0.0f, 40.0f);
    // :266 case CENTER - blankSpace = (200 - 100 - 10) / 2 = 45. Space is added between children.
    layoutWith(FlexAlign::CENTER, 45.0f, 95.0f);
    // :271 case FLEX_END - blankSpace = 200 - 100 - 10 = 90.
    layoutWith(FlexAlign::FLEX_END, 90.0f, 140.0f);
    // :275 case SPACE_BETWEEN - blankSpace = (200 - 100) / 1 = 100, yPos is not pre-shifted.
    layoutWith(FlexAlign::SPACE_BETWEEN, 0.0f, 140.0f);
    // :278 case SPACE_AROUND - blankSpace = (200 - 100) / 2 = 50, yPos starts at 25.
    layoutWith(FlexAlign::SPACE_AROUND, 25.0f, 115.0f);
    // :282 case SPACE_EVENLY - blankSpace = (200 - 100) / 3.
    layoutWith(FlexAlign::SPACE_EVENLY, 100.0f / 3.0f, 100.0f / 3.0f + 40.0f + 100.0f / 3.0f);

    // :291 cross CENTER re-entry: x = (720 - 100) / 2
    LayoutConditions centeredCross;
    centeredCross.isVertical = true;
    centeredCross.crossAlign = FlexAlign::CENTER;
    centeredCross.mainAlign = FlexAlign::FLEX_START;
    centeredCross.size = SizeF(PARENT_WIDTH, 200.0f);
    centeredCross.space = 10.0f;
    LinearLayoutUtils::LayoutCondition(children, TextDirection::LTR, centeredCross);
    EXPECT_FLOAT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset().GetX(), 310.0f);
    EXPECT_FLOAT_EQ(secondChild->GetGeometryNode()->GetMarginFrameOffset().GetX(), 310.0f);
}

/**
 * @tc.name: LinearLayoutUtilsLayoutConditionHorizontal
 * @tc.desc: Pin every main axis alignment arm of the horizontal LayoutCondition path
 *           (:306-:350, cases CENTER / FLEX_END / SPACE_BETWEEN / SPACE_AROUND / SPACE_EVENLY /
 *           default) including the three paths of the "CENTER || FLEX_END" decision (:345-:346).
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, LinearLayoutUtilsLayoutConditionHorizontal, TestSize.Level0)
{
    auto container = CreateWrapNodePair(false, CalcSize(CalcLength(PARENT_WIDTH), CalcLength(100.0f)),
        SizeF(PARENT_WIDTH, 100.0f));
    auto childConstraint = CreateChildConstraintFor(container);
    auto firstChild = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    auto secondChild = AppendChildWrap(container, childConstraint, 100.0f, 60.0f);
    ASSERT_NE(firstChild, nullptr);
    ASSERT_NE(secondChild, nullptr);
    LinearLayoutUtils::Measure(AceType::RawPtr(container.wrapper), false);
    auto children = GetChildren(container);

    auto layoutWith = [&children, &firstChild, &secondChild](FlexAlign mainAlign, float expectedFirstX,
                            float expectedSecondX) {
        LayoutConditions conditions;
        conditions.isVertical = false;
        conditions.crossAlign = FlexAlign::FLEX_START;
        conditions.mainAlign = mainAlign;
        conditions.size = SizeF(PARENT_WIDTH, 100.0f);
        conditions.paddingOffset = OffsetF(0.0f, 0.0f);
        conditions.space = 10.0f;
        LinearLayoutUtils::LayoutCondition(children, TextDirection::LTR, conditions);
        EXPECT_FLOAT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset().GetX(), expectedFirstX);
        EXPECT_FLOAT_EQ(secondChild->GetGeometryNode()->GetMarginFrameOffset().GetX(), expectedSecondX);
        EXPECT_FLOAT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset().GetY(), 0.0f);
    };

    // :335 default arm
    layoutWith(FlexAlign::AUTO, 0.0f, 100.0f);
    // :316 case CENTER - blankSpace = (720 - 200 - 10) / 2 = 255
    layoutWith(FlexAlign::CENTER, 255.0f, 365.0f);
    // :320 case FLEX_END - blankSpace = 720 - 200 - 10 = 510
    layoutWith(FlexAlign::FLEX_END, 510.0f, 620.0f);
    // :324 case SPACE_BETWEEN - blankSpace = (720 - 200) / 1 = 520
    layoutWith(FlexAlign::SPACE_BETWEEN, 0.0f, 620.0f);
    // :327 case SPACE_AROUND - blankSpace = (720 - 200) / 2 = 260, xPos starts at 130
    layoutWith(FlexAlign::SPACE_AROUND, 130.0f, 490.0f);
    // :331 case SPACE_EVENLY - blankSpace = (720 - 200) / 3
    layoutWith(FlexAlign::SPACE_EVENLY, 520.0f / 3.0f, 520.0f / 3.0f + 100.0f + 520.0f / 3.0f);

    // :340 cross FLEX_END re-entry: the cross offset uses the child height.
    LayoutConditions crossEnd;
    crossEnd.isVertical = false;
    crossEnd.crossAlign = FlexAlign::FLEX_END;
    crossEnd.mainAlign = FlexAlign::FLEX_START;
    crossEnd.size = SizeF(PARENT_WIDTH, 100.0f);
    crossEnd.space = 10.0f;
    LinearLayoutUtils::LayoutCondition(children, TextDirection::LTR, crossEnd);
    EXPECT_FLOAT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset().GetY(), 60.0f);
    EXPECT_FLOAT_EQ(secondChild->GetGeometryNode()->GetMarginFrameOffset().GetY(), 40.0f);
}

/**
 * @tc.name: LinearLayoutUtilsLayoutRtlAndPadding
 * @tc.desc: Pin the RTL mirror of AdjustChildOnDirection (:193) together with padding handling in
 *           Layout (:209,:210,:211,:217): LTR keeps the raw offset, RTL mirrors it against the
 *           padding-subtracted width, and an explicit AUTO direction resolves from the global flag
 *           (:214-T,:215-T). Also covers the public AdjustChildOnDirection helper directly.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, LinearLayoutUtilsLayoutRtlAndPadding, TestSize.Level0)
{
    ScopedRightToLeft rightToLeft(false);
    auto container = CreateWrapNodePair(true, CalcSize(CalcLength(PARENT_WIDTH), CalcLength(200.0f)),
        SizeF(PARENT_WIDTH, PARENT_HEIGHT));
    auto layoutProperty = container.wrapper->GetLayoutProperty();
    ASSERT_NE(layoutProperty, nullptr);
    PaddingProperty padding;
    padding.left = CalcLength(10.0f);
    padding.right = CalcLength(10.0f);
    padding.top = CalcLength(5.0f);
    padding.bottom = CalcLength(5.0f);
    layoutProperty->UpdatePadding(padding);
    auto childConstraint = CreateChildConstraintFor(container);
    auto firstChild = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    auto secondChild = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    ASSERT_NE(firstChild, nullptr);
    ASSERT_NE(secondChild, nullptr);
    LinearLayoutUtils::Measure(AceType::RawPtr(container.wrapper), true);
    EXPECT_EQ(container.wrapper->GetGeometryNode()->GetFrameSize(), SizeF(PARENT_WIDTH, 200.0f));

    // :214-F explicit LTR - raw offset + padding offset.
    layoutProperty->UpdateLayoutDirection(TextDirection::LTR);
    LinearLayoutUtils::Layout(
        AceType::RawPtr(container.wrapper), true, FlexAlign::FLEX_START, FlexAlign::FLEX_START);
    EXPECT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(10.0f, 5.0f));
    EXPECT_EQ(secondChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(10.0f, 45.0f));

    // :193-T RTL mirror: x = 10 + (700 - 100 - 0) = 610, padding-subtracted width is 700.
    layoutProperty->UpdateLayoutDirection(TextDirection::RTL);
    LinearLayoutUtils::Layout(
        AceType::RawPtr(container.wrapper), true, FlexAlign::FLEX_START, FlexAlign::FLEX_START);
    EXPECT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(610.0f, 5.0f));
    EXPECT_EQ(secondChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(610.0f, 45.0f));

    // :214-T,:215-T AUTO direction with the global RTL flag enabled.
    {
        ScopedRightToLeft autoRightToLeft(true);
        layoutProperty->UpdateLayoutDirection(TextDirection::AUTO);
        LinearLayoutUtils::Layout(
            AceType::RawPtr(container.wrapper), true, FlexAlign::FLEX_START, FlexAlign::FLEX_START);
        EXPECT_EQ(firstChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(610.0f, 5.0f));
    }

    // Direct helper coverage: :193 both arms.
    auto ltrOffset = LinearLayoutUtils::AdjustChildOnDirection(
        firstChild, OffsetF(10.0f, 20.0f), TextDirection::LTR, 700.0f);
    EXPECT_EQ(ltrOffset, OffsetF(10.0f, 20.0f));
    auto rtlOffset = LinearLayoutUtils::AdjustChildOnDirection(
        firstChild, OffsetF(10.0f, 20.0f), TextDirection::RTL, 700.0f);
    EXPECT_EQ(rtlOffset, OffsetF(590.0f, 20.0f));
}

/**
 * @tc.name: LinearLayoutUtilsLayoutConditionSingleChild
 * @tc.desc: Regression guard for the SPACE_BETWEEN division by (childNum - 1) with a single child
 *           (:276 / :325): the computed blank space is only applied after the last child, so the
 *           single child must stay at offset 0 on both axes instead of leaking an infinity.
 * @tc.type: FUNC
 */
HWTEST_F(LinearLayoutCoverageTestNG, LinearLayoutUtilsLayoutConditionSingleChild, TestSize.Level0)
{
    auto container = CreateWrapNodePair(true, CalcSize(CalcLength(PARENT_WIDTH), CalcLength(200.0f)),
        SizeF(PARENT_WIDTH, PARENT_HEIGHT));
    auto childConstraint = CreateChildConstraintFor(container);
    auto onlyChild = AppendChildWrap(container, childConstraint, 100.0f, 40.0f);
    ASSERT_NE(onlyChild, nullptr);
    LinearLayoutUtils::Measure(AceType::RawPtr(container.wrapper), true);
    auto children = GetChildren(container);

    LayoutConditions verticalConditions;
    verticalConditions.isVertical = true;
    verticalConditions.crossAlign = FlexAlign::FLEX_START;
    verticalConditions.mainAlign = FlexAlign::SPACE_BETWEEN;
    verticalConditions.size = SizeF(PARENT_WIDTH, 200.0f);
    verticalConditions.space = 0.0f;
    LinearLayoutUtils::LayoutCondition(children, TextDirection::LTR, verticalConditions);
    EXPECT_EQ(onlyChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0f, 0.0f));

    auto rowContainer = CreateWrapNodePair(false, CalcSize(CalcLength(PARENT_WIDTH), CalcLength(100.0f)),
        SizeF(PARENT_WIDTH, 100.0f));
    auto rowChildConstraint = CreateChildConstraintFor(rowContainer);
    auto onlyRowChild = AppendChildWrap(rowContainer, rowChildConstraint, 100.0f, 40.0f);
    ASSERT_NE(onlyRowChild, nullptr);
    LinearLayoutUtils::Measure(AceType::RawPtr(rowContainer.wrapper), false);
    auto rowChildren = GetChildren(rowContainer);

    LayoutConditions horizontalConditions;
    horizontalConditions.isVertical = false;
    horizontalConditions.crossAlign = FlexAlign::FLEX_START;
    horizontalConditions.mainAlign = FlexAlign::SPACE_BETWEEN;
    horizontalConditions.size = SizeF(PARENT_WIDTH, 100.0f);
    horizontalConditions.space = 0.0f;
    LinearLayoutUtils::LayoutCondition(rowChildren, TextDirection::LTR, horizontalConditions);
    EXPECT_EQ(onlyRowChild->GetGeometryNode()->GetMarginFrameOffset(), OffsetF(0.0f, 0.0f));
}
} // namespace OHOS::Ace::NG
