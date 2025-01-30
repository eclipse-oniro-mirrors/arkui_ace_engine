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
#include "generated/test_fixtures.h"
#include "generated/type_helpers.h"

#include "core/interfaces/native/implementation/text_controller_peer_impl.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/converter2.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/utility/callback_helper.h"
#include "generated/type_helpers.h"

#include "core/components/text/text_theme.h"
#include "core/components_ng/pattern/text/text_event_hub.h"
#include "core/components_ng/pattern/text/text_model_ng.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "test/unittest/capi/utils/custom_node_builder_test_helper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
    const auto ATTRIBUTE_BIND_SELECTION_MENU_NAME = "bindSelectionMenu";
    const auto ATTRIBUTE_BIND_SELECTION_MENU_DEFAULT_VALUE = "[]";
} // namespace

struct SelectionMenuOptions {
    std::optional<MenuOnAppearCallback> onAppear;
    std::optional<Callback_Void> onDisappear;
    std::optional<Ark_MenuType> menuType;
};

namespace Converter {
    void AssignArkValue(Ark_SelectionMenuOptions& dst, const SelectionMenuOptions& src, ConvContext *ctx = nullptr)
    {
        dst.onAppear = Converter::ArkValue<Opt_MenuOnAppearCallback>(src.onAppear);
        dst.onDisappear = Converter::ArkValue<Opt_Callback_Void>(src.onDisappear);
        dst.menuType = Converter::ArkValue<Opt_MenuType>(src.menuType);
    }
}

class TextModifierTest2 : public ModifierTestBase<GENERATED_ArkUITextModifier,
    &GENERATED_ArkUINodeModifiers::getTextModifier, GENERATED_ARKUI_TEXT> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();
        SetupTheme<TextTheme>();
    }
};

/*
 * @tc.name: bindSelectionMenuTestDefaultValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest2, bindSelectionMenuTestDefaultValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue = GetJsonValue(node_);
    std::string resultStr;
    resultStr = GetAttrValue<std::string>(jsonValue, ATTRIBUTE_BIND_SELECTION_MENU_NAME);
    EXPECT_EQ(resultStr, ATTRIBUTE_BIND_SELECTION_MENU_DEFAULT_VALUE) <<
        "Default value for attribute 'BindSelectionMenu'";
}

/*
 * @tc.name: bindSelectionMenuTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
using OneTestTextSpanTypeStep = std::tuple<Ark_TextSpanType, Ark_TextResponseType>;
static const std::vector<OneTestTextSpanTypeStep> testPlan = {
    {ARK_TEXT_SPAN_TYPE_TEXT, ARK_TEXT_RESPONSE_TYPE_SELECT},
    {ARK_TEXT_SPAN_TYPE_MIXED, ARK_TEXT_RESPONSE_TYPE_SELECT},
    {ARK_TEXT_SPAN_TYPE_IMAGE, ARK_TEXT_RESPONSE_TYPE_SELECT},
    {ARK_TEXT_SPAN_TYPE_TEXT, ARK_TEXT_RESPONSE_TYPE_LONG_PRESS},
    {ARK_TEXT_SPAN_TYPE_MIXED, ARK_TEXT_RESPONSE_TYPE_LONG_PRESS},
    {ARK_TEXT_SPAN_TYPE_IMAGE, ARK_TEXT_RESPONSE_TYPE_LONG_PRESS},
    {ARK_TEXT_SPAN_TYPE_TEXT, ARK_TEXT_RESPONSE_TYPE_RIGHT_CLICK},
    {ARK_TEXT_SPAN_TYPE_MIXED, ARK_TEXT_RESPONSE_TYPE_RIGHT_CLICK},
    {ARK_TEXT_SPAN_TYPE_IMAGE, ARK_TEXT_RESPONSE_TYPE_RIGHT_CLICK},
};
std::optional<SelectOverlayInfo> GetSelectionMenuParams_Patched(FrameNode* frameNode, Ark_TextSpanType arkSpanType,
                                                                Ark_TextResponseType arkResponseType)
{
    auto pattern = frameNode->GetPattern<TextPattern>();
    CHECK_NULL_RETURN(pattern, std::nullopt);
    pattern->SetSelectedType(Converter::OptConvert<TextSpanType>(arkSpanType).value_or(TextSpanType::NONE));
    pattern->SetTextResponseType(
        Converter::OptConvert<TextResponseType>(arkResponseType).value_or(TextResponseType::NONE));
    SelectOverlayInfo selectInfo;
    pattern->CopySelectionMenuParams(selectInfo);
    return selectInfo;
}
HWTEST_F(TextModifierTest2, bindSelectionMenuTestValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setBindSelectionMenu, nullptr);
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);

    int callsCount = 0;
    CustomNodeBuilderTestHelper<TextModifierTest2> builderHelper(this, frameNode);
    const CustomNodeBuilder builder = builderHelper.GetBuilder();

    std::unique_ptr<JsonValue> fullJson;
    std::string resultValue;
    SelectionMenuOptions selectionMenuOptions1 = {.onAppear = std::nullopt, .onDisappear = std::nullopt,
        .menuType = Ark_MenuType::ARK_MENU_TYPE_SELECTION_MENU};
    auto options1 = Converter::ArkValue<Opt_SelectionMenuOptions>(selectionMenuOptions1);

    SelectionMenuOptions selectionMenuOptions2 = {.onAppear = std::nullopt, .onDisappear = std::nullopt,
        .menuType = Ark_MenuType::ARK_MENU_TYPE_PREVIEW_MENU};
    auto options2 = Converter::ArkValue<Opt_SelectionMenuOptions>(selectionMenuOptions2);

    std::optional<SelectOverlayInfo> selectInfo;

    for (auto [spanType, responseType]: testPlan) {
        modifier_->setBindSelectionMenu(node_, spanType, &builder, responseType, &options1);
        selectInfo = GetSelectionMenuParams_Patched(frameNode, spanType, responseType);
        ASSERT_TRUE(selectInfo.has_value());
        ASSERT_NE(selectInfo->menuInfo.menuBuilder, nullptr);
        selectInfo->menuInfo.menuBuilder();
        EXPECT_EQ(builderHelper.GetCallsCount(), ++callsCount);

        modifier_->setBindSelectionMenu(node_, spanType, &builder, responseType, &options2);
        selectInfo = GetSelectionMenuParams_Patched(frameNode, spanType, responseType);
        ASSERT_TRUE(selectInfo.has_value());
        ASSERT_NE(selectInfo->menuInfo.menuBuilder, nullptr);
        selectInfo->menuInfo.menuBuilder();
        EXPECT_EQ(builderHelper.GetCallsCount(), ++callsCount);

        fullJson = GetJsonValue(node_);
        resultValue = GetAttrValue<std::string>(fullJson, ATTRIBUTE_BIND_SELECTION_MENU_NAME);
    }

    std::string expectedValue =
        "[{\"spanType\":0,\"responseType\":0,\"menuType\":0},"
        "{\"spanType\":0,\"responseType\":1,\"menuType\":0},"
        "{\"spanType\":0,\"responseType\":2,\"menuType\":0},"
        "{\"spanType\":1,\"responseType\":0,\"menuType\":0},"
        "{\"spanType\":1,\"responseType\":1,\"menuType\":0},"
        "{\"spanType\":1,\"responseType\":2,\"menuType\":0},"
        "{\"spanType\":2,\"responseType\":0,\"menuType\":0},"
        "{\"spanType\":2,\"responseType\":1,\"menuType\":0},"
        "{\"spanType\":2,\"responseType\":2,\"menuType\":0}]";
    EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
}

/*
 * @tc.name: DISABLED_bindSelectionMenu_MenuTypeTestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(TextModifierTest2, DISABLED_bindSelectionMenu_MenuTypeTestValidValues, TestSize.Level1)
{
    ASSERT_NE(modifier_->setBindSelectionMenu, nullptr);
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);

    using OneTestMenuTypeStep = std::tuple<Ark_MenuType, int32_t>;
    static const std::vector<OneTestMenuTypeStep> testPlan = {
        {Ark_MenuType::ARK_MENU_TYPE_PREVIEW_MENU, 0},
        {Ark_MenuType::ARK_MENU_TYPE_SELECTION_MENU, 1},
    };

    CustomNodeBuilderTestHelper<TextModifierTest2> builderHelper(this, frameNode);
    const CustomNodeBuilder builder = builderHelper.GetBuilder();
    std::unique_ptr<JsonValue> fullJson;
    std::string resultValue;
    auto spanType = Ark_TextSpanType::ARK_TEXT_SPAN_TYPE_TEXT;
    auto responseType = Ark_TextResponseType::ARK_TEXT_RESPONSE_TYPE_SELECT;
    SelectionMenuOptions selectionMenuOptions = {};
    Opt_SelectionMenuOptions options;

    for (auto [arkMenuType, aceMenuType]: testPlan) {
        selectionMenuOptions.menuType = arkMenuType;
        options = Converter::ArkValue<Opt_SelectionMenuOptions>(selectionMenuOptions);
        modifier_->setBindSelectionMenu(node_, spanType, &builder, responseType, &options);
        fullJson = GetJsonValue(node_);
        resultValue = GetAttrValue<std::string>(fullJson, ATTRIBUTE_BIND_SELECTION_MENU_NAME);
    }

    std::string expectedValue =
        "[{\"spanType\":0,\"responseType\":2,\"menuType\":0},"
        "{\"spanType\":0,\"responseType\":2,\"menuType\":1}]";
    EXPECT_EQ(resultValue, expectedValue) << "Passed value is: " << expectedValue;
}
}