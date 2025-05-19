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

#include "core/components/counter/counter_theme.h"
#include "core/components_ng/pattern/counter/counter_pattern.h"

#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/utility/callback_helper.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::NG::Converter;

namespace OHOS::Ace::NG {
namespace  {
    const auto ATTRIBUTE_ENABLE_DEC_NAME = "enableDec";
    const auto ATTRIBUTE_ENABLE_DEC_DEFAULT_VALUE = "true";
    const auto ATTRIBUTE_ENABLE_INC_NAME = "enableInc";
    const auto ATTRIBUTE_ENABLE_INC_DEFAULT_VALUE = "true";
    const auto ATTRIBUTE_WIDTH_NAME = "width";
    const auto ATTRIBUTE_WIDTH_DEFAULT_VALUE = "100.00vp";
    const auto ATTRIBUTE_HEIGHT_NAME = "height";
    const auto ATTRIBUTE_HEIGHT_DEFAULT_VALUE = "32.00vp";

    const Ark_Int32 FAKE_RES_ID(1234);
    const Ark_Length RES_ARK_LENGTH = Converter::ArkValue<Ark_Length>(FAKE_RES_ID);
} // namespace

class CounterModifierTest : public ModifierTestBase<GENERATED_ArkUICounterModifier,
    &GENERATED_ArkUINodeModifiers::getCounterModifier, GENERATED_ARKUI_COUNTER> {
public:

    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        SetupTheme<CounterTheme>();
    }
};

/*
 * @tc.name: setEnableDecTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, setEnableDecTestDefaultValues, TestSize.Level1)
{
    std::string strResult;

    strResult = GetStringAttribute(node_, ATTRIBUTE_ENABLE_DEC_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_ENABLE_DEC_DEFAULT_VALUE);
}

/*
 * @tc.name: setEnableDecTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, setEnableDecTestValidValues, TestSize.Level1)
{
    std::string strResult;

    auto inputValue = Converter::ArkValue<Opt_Boolean>(false);
    modifier_->setEnableDec(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_ENABLE_DEC_NAME);
    EXPECT_EQ(strResult, "false");

    inputValue = Converter::ArkValue<Opt_Boolean>(true);
    modifier_->setEnableDec(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_ENABLE_DEC_NAME);
    EXPECT_EQ(strResult, "true");
}

/*
 * @tc.name: setEnableIncTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, setEnableIncTestDefaultValues, TestSize.Level1)
{
    std::string strResult;

    strResult = GetStringAttribute(node_, ATTRIBUTE_ENABLE_INC_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_ENABLE_INC_DEFAULT_VALUE);
}

/*
 * @tc.name: setEnableIncTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, setEnableIncTestValidValues, TestSize.Level1)
{
    std::string strResult;

    auto inputValue = Converter::ArkValue<Opt_Boolean>(false);
    modifier_->setEnableInc(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_ENABLE_INC_NAME);
    EXPECT_EQ(strResult, "false");

    inputValue = Converter::ArkValue<Opt_Boolean>(true);
    modifier_->setEnableInc(node_, &inputValue);
    strResult = GetStringAttribute(node_, ATTRIBUTE_ENABLE_INC_NAME);
    EXPECT_EQ(strResult, "true");
}

/*
 * @tc.name: setOnIncTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, setOnIncTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto pattern = frameNode->GetPattern<CounterPattern>();
    auto addId = pattern->GetAddId();
    auto addNode = AceType::DynamicCast<FrameNode>(frameNode->GetChildAtIndex(frameNode->GetChildIndexById(addId)));
    auto subGestureHub = addNode->GetOrCreateGestureEventHub();

    struct CheckEvent {
        int32_t nodeId;
        int32_t value;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    static constexpr int32_t contextId = 123;

    auto checkCallback = [](const Ark_Int32 resourceId) {
        checkEvent = {
            .nodeId = resourceId,
            .value = 1
        };
    };

    VoidCallback arkCallback = Converter::ArkValue<VoidCallback>(checkCallback, contextId);
    auto optCallback = Converter::ArkValue<Opt_VoidCallback>(arkCallback);

    modifier_->setOnInc(node_, &optCallback);

    EXPECT_EQ(checkEvent.has_value(), false);
    subGestureHub->ActClick();
    EXPECT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, contextId);
    EXPECT_EQ(checkEvent->value, 1);
}

/*
 * @tc.name: setOnDecTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, setOnDecTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto pattern = frameNode->GetPattern<CounterPattern>();
    auto subId = pattern->GetSubId();
    auto addNode = AceType::DynamicCast<FrameNode>(frameNode->GetChildAtIndex(frameNode->GetChildIndexById(subId)));
    auto subGestureHub = addNode->GetOrCreateGestureEventHub();

    struct CheckEvent {
        int32_t nodeId;
        int32_t value;
    };
    static std::optional<CheckEvent> checkEvent = std::nullopt;
    static constexpr int32_t contextId = 123;

    auto checkCallback = [](const Ark_Int32 resourceId) {
        checkEvent = {
            .nodeId = resourceId,
            .value = -1
        };
    };

    VoidCallback arkCallback = Converter::ArkValue<VoidCallback>(checkCallback, contextId);
    auto optCallback = Converter::ArkValue<Opt_VoidCallback>(arkCallback);

    modifier_->setOnDec(node_, &optCallback);

    EXPECT_EQ(checkEvent.has_value(), false);
    subGestureHub->ActClick();
    EXPECT_EQ(checkEvent.has_value(), true);
    EXPECT_EQ(checkEvent->nodeId, contextId);
    EXPECT_EQ(checkEvent->value, -1);
}

/*
 * @tc.name: setBackgroundColorTest
 * @tc.desc: Check the functionality of CounterModifierTest.setBackgroundColor
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, setBackgroundColorTest, TestSize.Level1)
{
    using OneTestStep = std::pair<Opt_ResourceColor, std::string>;
    static const std::string PROP_NAME("backgroundColor");
    static auto resName = NamedResourceId("aa.bb.cc", Converter::ResourceType::COLOR);
    static auto resId = IntResourceId(1234, Converter::ResourceType::COLOR);
    static const std::string EXPECTED_RESOURCE_COLOR =
        Color::RED.ToString(); // Color::RED is result of ThemeConstants::GetColorXxxx stubs
    static const std::vector<OneTestStep> testPlan = {
        { Converter::ArkValue<Opt_ResourceColor>(ArkUnion<Ark_ResourceColor, Ark_Color>(ARK_COLOR_WHITE)),
             "#FFFFFFFF" },
        { Converter::ArkValue<Opt_ResourceColor>(ArkUnion<Ark_ResourceColor, Ark_Number>(0x123456)), "#FF123456" },
        { Converter::ArkValue<Opt_ResourceColor>(ArkUnion<Ark_ResourceColor, Ark_Number>(0.5f)), "#00000000" },
        { Converter::ArkValue<Opt_ResourceColor>(ArkUnion<Ark_ResourceColor, Ark_String>("#11223344")), "#11223344" },
        { Converter::ArkValue<Opt_ResourceColor>(ArkUnion<Ark_ResourceColor, Ark_String>("65535")), "#FF00FFFF" },
        { Converter::ArkValue<Opt_ResourceColor>(CreateResourceUnion<Ark_ResourceColor>(resName)),
             EXPECTED_RESOURCE_COLOR },
        { Converter::ArkValue<Opt_ResourceColor>(CreateResourceUnion<Ark_ResourceColor>(resId)),
             EXPECTED_RESOURCE_COLOR },
    };

    ASSERT_NE(commonModifier_->setBackgroundColor0, nullptr);

    auto checkInitial = GetAttrValue<std::string>(node_, PROP_NAME);
    EXPECT_EQ(checkInitial, Color::TRANSPARENT.ToString());

    for (const auto &[arkResColor, expected]: testPlan) {
        commonModifier_->setBackgroundColor0(node_, &arkResColor);
        auto checkColor = GetAttrValue<std::string>(node_, PROP_NAME);
        EXPECT_EQ(checkColor, expected);
    }
}

/*
 * @tc.name: setWidthTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, setWidthTestDefaultValues, TestSize.Level1)
{
    std::string strResult = GetStringAttribute(node_, ATTRIBUTE_WIDTH_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_WIDTH_DEFAULT_VALUE);
}

/*
 * @tc.name: setWidthTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, setWidth0TestValidValues, TestSize.Level1)
{
    typedef std::pair<Opt_Length, std::string> OneTestStep;
    static const std::vector<OneTestStep> testPlan = {
        { Converter::ArkValue<Opt_Length>(2.45f), "2.45vp" },
        { Converter::ArkValue<Opt_Length>(5.0_px), "5.00px" },
        { Converter::ArkValue<Opt_Length>(22.35_px), "22.35px" },
        { Converter::ArkValue<Opt_Length>(7.0_vp), "7.00vp" },
        { Converter::ArkValue<Opt_Length>(1.65_vp), "1.65vp" },
        { Converter::ArkValue<Opt_Length>(65.0_fp), "65.00fp" },
        { Converter::ArkValue<Opt_Length>(4.3_fp), "4.30fp" },
        { Converter::ArkValue<Opt_Length>("12.00%"), "12.00%" },
        { Converter::ArkValue<Opt_Length>(RES_ARK_LENGTH), "10.00px" },
    };

    for (const auto &[arkLength, expected]: testPlan) {
        commonModifier_->setWidth0(node_, &arkLength);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_WIDTH_NAME);
        EXPECT_EQ(strResult, expected);
    }
}

/*
 * @tc.name: setWidthTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, DISABLED_setWidth0TestInvalidValues, TestSize.Level1)
{
    static const std::vector<Opt_Length> testPlan = {
        Converter::ArkValue<Opt_Length>(-2.45f),
        Converter::ArkValue<Opt_Length>(-5.0_px),
        Converter::ArkValue<Opt_Length>(-22.35_px),
        Converter::ArkValue<Opt_Length>(-7.0_vp),
        Converter::ArkValue<Opt_Length>(-1.65_vp),
        Converter::ArkValue<Opt_Length>(-65.0_fp),
        Converter::ArkValue<Opt_Length>(-4.3_fp),
    };

    for (const auto &arkLength : testPlan) {
        commonModifier_->setWidth0(node_, &arkLength);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_WIDTH_NAME);
        EXPECT_EQ(strResult, ATTRIBUTE_WIDTH_DEFAULT_VALUE);
    }

    commonModifier_->setWidth0(node_, nullptr);
    auto strResult = GetStringAttribute(node_, ATTRIBUTE_WIDTH_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_WIDTH_DEFAULT_VALUE);
}

/*
 * @tc.name: setHeightTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, setHeightTestDefaultValues, TestSize.Level1)
{
    std::string strResult = GetStringAttribute(node_, ATTRIBUTE_HEIGHT_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_HEIGHT_DEFAULT_VALUE);
}

/*
 * @tc.name: setHeightTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, setHeight0TestValidValues, TestSize.Level1)
{
    typedef std::pair<Opt_Length, std::string> OneTestStep;
    static const std::vector<OneTestStep> testPlan = {
        { Converter::ArkValue<Opt_Length>(2.45f), "2.45vp" },
        { Converter::ArkValue<Opt_Length>(5.0_px), "5.00px" },
        { Converter::ArkValue<Opt_Length>(22.35_px), "22.35px" },
        { Converter::ArkValue<Opt_Length>(7.0_vp), "7.00vp" },
        { Converter::ArkValue<Opt_Length>(1.65_vp), "1.65vp" },
        { Converter::ArkValue<Opt_Length>(65.0_fp), "65.00fp" },
        { Converter::ArkValue<Opt_Length>(4.3_fp), "4.30fp" },
        { Converter::ArkValue<Opt_Length>("12.00%"), "12.00%" },
        { Converter::ArkValue<Opt_Length>(RES_ARK_LENGTH), "10.00px" },
    };

    for (const auto &[arkLength, expected]: testPlan) {
        commonModifier_->setHeight0(node_, &arkLength);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_HEIGHT_NAME);
        EXPECT_EQ(strResult, expected);
    }
}

/*
 * @tc.name: setHeightTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, DISABLED_setHeight0TestInvalidValues, TestSize.Level1)
{
    static const std::vector<Opt_Length> testPlan = {
        Converter::ArkValue<Opt_Length>(-2.45f),
        Converter::ArkValue<Opt_Length>(-5.0_px),
        Converter::ArkValue<Opt_Length>(-22.35_px),
        Converter::ArkValue<Opt_Length>(-7.0_vp),
        Converter::ArkValue<Opt_Length>(-1.65_vp),
        Converter::ArkValue<Opt_Length>(-65.0_fp),
        Converter::ArkValue<Opt_Length>(-4.3_fp),
    };

    for (const auto &arkLength : testPlan) {
        //auto inputValue = Converter::ArkValue<Opt_Length>(arkLength);
        commonModifier_->setHeight0(node_, &arkLength);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_HEIGHT_NAME);
        EXPECT_EQ(strResult, ATTRIBUTE_HEIGHT_DEFAULT_VALUE);
    }

    commonModifier_->setHeight0(node_, nullptr);
    auto strResult = GetStringAttribute(node_, ATTRIBUTE_HEIGHT_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_HEIGHT_DEFAULT_VALUE);
}

/*
 * @tc.name: setSizeTestValidHeightValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, setSizeTestValidHeightValues, TestSize.Level1)
{
    typedef std::pair<Ark_Length, std::string> OneTestStep;
    static const std::vector<OneTestStep> testPlan = {
        { Converter::ArkValue<Ark_Length>(2.45f), "2.45vp" },
        { Converter::ArkValue<Ark_Length>(5.0_px), "5.00px" },
        { Converter::ArkValue<Ark_Length>(22.35_px), "22.35px" },
        { Converter::ArkValue<Ark_Length>(7.0_vp), "7.00vp" },
        { Converter::ArkValue<Ark_Length>(1.65_vp), "1.65vp" },
        { Converter::ArkValue<Ark_Length>(65.0_fp), "65.00fp" },
        { Converter::ArkValue<Ark_Length>(4.3_fp), "4.30fp" },
        { Converter::ArkValue<Ark_Length>("12.00%"), "12.00%" },
        { RES_ARK_LENGTH, "10.00px" },
    };

    for (const auto &[arkLength, expected]: testPlan) {
        Ark_SizeOptions inputValue;
        inputValue.height = Converter::ArkValue<Opt_Length>(arkLength);
        inputValue.width = Converter::ArkValue<Opt_Length>(Ark_Empty());
        auto optValue = Converter::ArkValue<Opt_SizeOptions>(inputValue);
        commonModifier_->setSize(node_, &optValue);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_HEIGHT_NAME);
        EXPECT_EQ(strResult, expected);
        strResult = GetStringAttribute(node_, ATTRIBUTE_WIDTH_NAME);
        EXPECT_EQ(strResult, ATTRIBUTE_WIDTH_DEFAULT_VALUE);
    }
}

/*
 * @tc.name: setSizeTestValidWidthValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, setSizeTestValidWidthValues, TestSize.Level1)
{
    typedef std::pair<Ark_Length, std::string> OneTestStep;
    static const std::vector<OneTestStep> testPlan = {
        { Converter::ArkValue<Ark_Length>(2.45f), "2.45vp" },
        { Converter::ArkValue<Ark_Length>(5.0_px), "5.00px" },
        { Converter::ArkValue<Ark_Length>(22.35_px), "22.35px" },
        { Converter::ArkValue<Ark_Length>(7.0_vp), "7.00vp" },
        { Converter::ArkValue<Ark_Length>(1.65_vp), "1.65vp" },
        { Converter::ArkValue<Ark_Length>(65.0_fp), "65.00fp" },
        { Converter::ArkValue<Ark_Length>(4.3_fp), "4.30fp" },
        { Converter::ArkValue<Ark_Length>("12.00%"), "12.00%" },
        { RES_ARK_LENGTH, "10.00px" },
    };

    for (const auto &[arkLength, expected]: testPlan) {
        Ark_SizeOptions inputValue;
        inputValue.width = Converter::ArkValue<Opt_Length>(arkLength);
        inputValue.height = Converter::ArkValue<Opt_Length>(Ark_Empty());
        auto optValue = Converter::ArkValue<Opt_SizeOptions>(inputValue);
        commonModifier_->setSize(node_, &optValue);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_WIDTH_NAME);
        EXPECT_EQ(strResult, expected);
        strResult = GetStringAttribute(node_, ATTRIBUTE_HEIGHT_NAME);
        EXPECT_EQ(strResult, ATTRIBUTE_HEIGHT_DEFAULT_VALUE);
    }
}

/*
 * @tc.name: setSizeTestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CounterModifierTest, setSizeTestInvalidValues, TestSize.Level1)
{
    Ark_SizeOptions inputValue;

    static const std::vector<Ark_Length> testPlan = {
        Converter::ArkValue<Ark_Length>(-2.45f),
        Converter::ArkValue<Ark_Length>(-5.0_px),
        Converter::ArkValue<Ark_Length>(-22.35_px),
        Converter::ArkValue<Ark_Length>(-7.0_vp),
        Converter::ArkValue<Ark_Length>(-1.65_vp),
        Converter::ArkValue<Ark_Length>(-65.0_fp),
        Converter::ArkValue<Ark_Length>(-4.3_fp),
    };

    for (const auto &arkLength : testPlan) {
        inputValue.width = Converter::ArkValue<Opt_Length>(arkLength);
        inputValue.height = Converter::ArkValue<Opt_Length>(Ark_Empty());
        auto optValue = Converter::ArkValue<Opt_SizeOptions>(inputValue);
        commonModifier_->setSize(node_, &optValue);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_WIDTH_NAME);
        EXPECT_EQ(strResult, ATTRIBUTE_WIDTH_DEFAULT_VALUE);
        strResult = GetStringAttribute(node_, ATTRIBUTE_HEIGHT_NAME);
        EXPECT_EQ(strResult, ATTRIBUTE_HEIGHT_DEFAULT_VALUE);
    }

    for (const auto &arkLength : testPlan) {
        inputValue.height = Converter::ArkValue<Opt_Length>(arkLength);
        inputValue.width = Converter::ArkValue<Opt_Length>(Ark_Empty());
        auto optValue = Converter::ArkValue<Opt_SizeOptions>(inputValue);
        commonModifier_->setSize(node_, &optValue);
        auto strResult = GetStringAttribute(node_, ATTRIBUTE_WIDTH_NAME);
        EXPECT_EQ(strResult, ATTRIBUTE_WIDTH_DEFAULT_VALUE);
        strResult = GetStringAttribute(node_, ATTRIBUTE_HEIGHT_NAME);
        EXPECT_EQ(strResult, ATTRIBUTE_HEIGHT_DEFAULT_VALUE);
    }

    commonModifier_->setSize(node_, nullptr);
    auto strResult = GetStringAttribute(node_, ATTRIBUTE_WIDTH_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_WIDTH_DEFAULT_VALUE);
    strResult = GetStringAttribute(node_, ATTRIBUTE_HEIGHT_NAME);
    EXPECT_EQ(strResult, ATTRIBUTE_HEIGHT_DEFAULT_VALUE);
}
} // namespace OHOS::Ace::NG
