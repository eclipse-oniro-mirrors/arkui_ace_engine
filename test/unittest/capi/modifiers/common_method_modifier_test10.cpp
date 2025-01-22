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

#include "common_method_modifier_test.h"

#include "modifier_test_base.h"
#include "modifiers_test_utils.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/utility/callback_helper.h"
#include "test/unittest/capi/utils/custom_node_builder_test_helper.h"
#include "generated/type_helpers.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
    const auto ATTRIBUTE_BACKGROUND_NAME = "background";
    const auto ATTRIBUTE_BACKGROUND_DEFAULT_VALUE = "";
}

struct DoublePair {
    double first;
    double second;
    bool operator<(const DoublePair& other) const
    {
        if (first != other.first) {
            return first < other.first;
        }
        return second < other.second;
    }
};
namespace Converter {
    std::string Convert(const Alignment& value)
    {
        double horizontal = value.GetHorizontal();
        double vertical = value.GetVertical();
        std::map<DoublePair, std::string> alignmentMap = {
            {{-1.0, -1.0}, "TOP_LEFT"},
            {{0.0, -1.0}, "TOP_CENTER"},
            {{1.0, -1.0}, "TOP_RIGHT"},
            {{-1.0, 0.0}, "CENTER_LEFT"},
            {{0.0, 0.0}, "CENTER"},
            {{1.0, 0.0}, "CENTER_RIGHT"},
            {{-1.0, 1.0}, "BOTTOM_LEFT"},
            {{0.0, 1.0}, "BOTTOM_CENTER"},
            {{1.0, 1.0}, "BOTTOM_RIGHT"}
        };
        auto it = alignmentMap.find({horizontal, vertical});
        if (it != alignmentMap.end()) {
            return it->second;
        }
        return "";
    }
    template<>
    void AssignArkValue(Ark_Literal_Alignment_align& dst, const Ark_Alignment& src, ConvContext *ctx)
    {
        dst.align = Converter::ArkValue<Opt_Alignment>(src);
    }
}

class CommonMethodModifierTest10 : public ModifierTestBase<GENERATED_ArkUICommonMethodModifier,
    &GENERATED_ArkUINodeModifiers::getCommonMethodModifier,
    GENERATED_ARKUI_BLANK // test common methods on frameNode for Blank component
    > {
public:
    RefPtr<RenderContext> render_;

    void *CreateNodeImpl() override
    {
        return nodeModifiers_->getBlankModifier()->construct(GetId(), 0);
    }
};

//////// Background
/*
 * @tc.name: setBackgroundTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest10, setBackgroundTestDefaultValues, TestSize.Level1)
{
    std::string strResult = GetStringAttribute(node_, ATTRIBUTE_BACKGROUND_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_BACKGROUND_DEFAULT_VALUE);
}

/*
 * @tc.name: setBackgroundCustomNodeBuilderTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest10, setBackgroundCustomNodeBuilderTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setBackground, nullptr);
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);

    // Test of using three helpers
    int callsCount(0);
    CustomNodeBuilderTestHelper<CommonMethodModifierTest10> builderHelper1(this, frameNode);
    CustomNodeBuilderTestHelper<CommonMethodModifierTest10> builderHelper2(this, frameNode);
    CustomNodeBuilderTestHelper<CommonMethodModifierTest10> builderHelper3(this, frameNode);

    const CustomNodeBuilder builder1 = builderHelper1.GetBuilder();
    const CustomNodeBuilder builder2 = builderHelper2.GetBuilder();
    const CustomNodeBuilder builder3 = builderHelper3.GetBuilder();

    // Testing builderHelper3
    modifier_->setBackground(node_, &builder3, nullptr);
    EXPECT_EQ(builderHelper3.GetCallsCount(), ++callsCount);

    // Testing builderHelper2
    modifier_->setBackground(node_, &builder2, nullptr);
    EXPECT_EQ(builderHelper2.GetCallsCount(), callsCount);

    // Testing builderHelper1
    modifier_->setBackground(node_, &builder1, nullptr);
    EXPECT_EQ(builderHelper1.GetCallsCount(), callsCount);
    modifier_->setBackground(node_, &builder1, nullptr);
    EXPECT_EQ(builderHelper1.GetCallsCount(), ++callsCount);
}

/*
 * @tc.name: setBackgroundTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CommonMethodModifierTest10, setBackgroundTestValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setBackground, nullptr);
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);

    int callsCount(0);
    CustomNodeBuilderTestHelper<CommonMethodModifierTest10> builderHelper(this, frameNode);
    const CustomNodeBuilder builder = builderHelper.GetBuilder();

    using OneTestStep = std::tuple<Ark_Alignment, std::string>;
    static const std::vector<OneTestStep> testPlan = {
        {Ark_Alignment::ARK_ALIGNMENT_TOP_START, "TOP_LEFT"},
        {Ark_Alignment::ARK_ALIGNMENT_TOP, "TOP_CENTER"},
        {Ark_Alignment::ARK_ALIGNMENT_TOP_END, "TOP_RIGHT"},
        {Ark_Alignment::ARK_ALIGNMENT_START, "CENTER_LEFT"},
        {Ark_Alignment::ARK_ALIGNMENT_CENTER, "CENTER"},
        {Ark_Alignment::ARK_ALIGNMENT_END, "CENTER_RIGHT"},
        {Ark_Alignment::ARK_ALIGNMENT_BOTTOM_START, "BOTTOM_LEFT"},
        {Ark_Alignment::ARK_ALIGNMENT_BOTTOM, "BOTTOM_CENTER"},
        {Ark_Alignment::ARK_ALIGNMENT_BOTTOM_END, "BOTTOM_RIGHT"},
    };
    std::string resultValue = "";
    const auto& renderContext = builderHelper.GetCustomNode()->GetRenderContext();

    for (auto [inputValue, expectedValue]: testPlan) {
        auto optInputValue = Converter::ArkValue<Opt_Literal_Alignment_align>(inputValue);
        modifier_->setBackground(node_, &builder, &optInputValue);
        EXPECT_EQ(builderHelper.GetCallsCount(), ++callsCount);
        if (renderContext) {
            auto background = renderContext->GetBackgroundAlign();
            resultValue = background.has_value() ? Converter::Convert(background.value()) : "";
        }
        EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
    }
}
}
