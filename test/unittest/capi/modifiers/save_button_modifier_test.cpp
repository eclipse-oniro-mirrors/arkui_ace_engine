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
#include "core/components_ng/pattern/security_component/save_button/save_button_common.h"
#include "core/components_ng/pattern/security_component/save_button/save_button_model_ng.h"
#include "core/interfaces/arkoala/utility/reverse_converter.h"
#include "arkoala_api_generated.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace  {
    const auto ATTRIBUTE_ICON_NAME = "icon";
    const auto ATTRIBUTE_ICON_DEFAULT_VALUE = SaveButtonIconStyle::ICON_NULL;
    const auto ATTRIBUTE_TEXT_NAME = "text";
    const auto ATTRIBUTE_TEXT_DEFAULT_VALUE = SaveButtonSaveDescription::TEXT_NULL;
    const auto ATTRIBUTE_BUTTON_TYPE_NAME = "buttonType";
    const auto ATTRIBUTE_BUTTON_TYPE_DEFAULT_VALUE = ButtonType::CAPSULE;

    const auto DEFAULT_JSON_INT = -1;

    struct EventsTracker {
        static inline GENERATED_ArkUISaveButtonEventsReceiver eventReceiver {};

        static inline const GENERATED_ArkUIEventsAPI eventsApiImpl {
            .getSaveButtonEventsReceiver = [] () -> const GENERATED_ArkUISaveButtonEventsReceiver* {
                return &eventReceiver;
            }
        };
    }; // EventsTracker
} // namespace

class SaveButtonModifierTest : public ModifierTestBase<GENERATED_ArkUISaveButtonModifier,
    &GENERATED_ArkUINodeModifiers::getSaveButtonModifier, GENERATED_ARKUI_SAVE_BUTTON> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        SetupTheme<SecurityComponentTheme>();

        fullAPI_->setArkUIEventsAPI(&EventsTracker::eventsApiImpl);
    }
};

/*
 * @tc.name: setSaveButtonOptions0
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(SaveButtonModifierTest, setSaveButtonOptions0, TestSize.Level1)
{
    auto node = CreateNode();
    modifier_->setSaveButtonOptions0(node);
    auto jsonValue = GetJsonValue(node);
    DisposeNode(node);
    int32_t resultText = jsonValue->GetInt(ATTRIBUTE_TEXT_NAME, DEFAULT_JSON_INT);
    int32_t resultIcon = jsonValue->GetInt(ATTRIBUTE_ICON_NAME, DEFAULT_JSON_INT);
    int32_t resultButtonType = jsonValue->GetInt(ATTRIBUTE_BUTTON_TYPE_NAME, DEFAULT_JSON_INT);
    EXPECT_EQ(resultText, static_cast<int32_t>(SaveButtonStyle::DEFAULT_TEXT));
    EXPECT_EQ(resultIcon, static_cast<int32_t>(SaveButtonStyle::DEFAULT_ICON));
    EXPECT_EQ(resultButtonType, static_cast<int32_t>(SaveButtonStyle::DEFAULT_BACKGROUND_TYPE));
}

// Valid values for attribute 'icon' of method 'setSaveButtonOptions'
using IconValidValuesStep = std::tuple<std::string, Opt_SaveIconStyle, SaveButtonIconStyle>;
static std::vector<IconValidValuesStep> setSaveButtonOptionsIconValidValues = {
    {
        "ARK_SAVE_ICON_STYLE_FULL_FILLED",
        Converter::ArkValue<Opt_SaveIconStyle>(ARK_SAVE_ICON_STYLE_FULL_FILLED),
        SaveButtonIconStyle::ICON_FULL_FILLED
    },
    {
        "ARK_SAVE_ICON_STYLE_LINES",
        Converter::ArkValue<Opt_SaveIconStyle>(ARK_SAVE_ICON_STYLE_LINES),
        SaveButtonIconStyle::ICON_LINE
    },
    {
        "ARK_SAVE_ICON_STYLE_PICTURE",
        Converter::ArkValue<Opt_SaveIconStyle>(ARK_SAVE_ICON_STYLE_PICTURE),
        SaveButtonIconStyle::ICON_PICTURE
    },
};

// Valid values for attribute 'text' of method 'setSaveButtonOptions'
using TextValidValuesStep = std::tuple<std::string, Opt_SaveDescription, SaveButtonSaveDescription>;
static std::vector<TextValidValuesStep> setSaveButtonOptionsTextValidValues = {
    {
        "ARK_SAVE_DESCRIPTION_DOWNLOAD",
        Converter::ArkValue<Opt_SaveDescription>(ARK_SAVE_DESCRIPTION_DOWNLOAD),
        SaveButtonSaveDescription::DOWNLOAD
    },
    {
        "ARK_SAVE_DESCRIPTION_DOWNLOAD_FILE",
        Converter::ArkValue<Opt_SaveDescription>(ARK_SAVE_DESCRIPTION_DOWNLOAD_FILE),
        SaveButtonSaveDescription::DOWNLOAD_FILE
    },
    {
        "ARK_SAVE_DESCRIPTION_SAVE",
        Converter::ArkValue<Opt_SaveDescription>(ARK_SAVE_DESCRIPTION_SAVE),
        SaveButtonSaveDescription::SAVE
    },
    {
        "ARK_SAVE_DESCRIPTION_SAVE_IMAGE",
        Converter::ArkValue<Opt_SaveDescription>(ARK_SAVE_DESCRIPTION_SAVE_IMAGE),
        SaveButtonSaveDescription::SAVE_IMAGE
    },
    {
        "ARK_SAVE_DESCRIPTION_SAVE_FILE",
        Converter::ArkValue<Opt_SaveDescription>(ARK_SAVE_DESCRIPTION_SAVE_FILE),
        SaveButtonSaveDescription::SAVE_FILE
    },
    {
        "ARK_SAVE_DESCRIPTION_DOWNLOAD_AND_SHARE",
        Converter::ArkValue<Opt_SaveDescription>(ARK_SAVE_DESCRIPTION_DOWNLOAD_AND_SHARE),
        SaveButtonSaveDescription::DOWNLOAD_AND_SHARE
    },
    {
        "ARK_SAVE_DESCRIPTION_RECEIVE",
        Converter::ArkValue<Opt_SaveDescription>(ARK_SAVE_DESCRIPTION_RECEIVE),
        SaveButtonSaveDescription::RECEIVE
    },
    {
        "ARK_SAVE_DESCRIPTION_CONTINUE_TO_RECEIVE",
        Converter::ArkValue<Opt_SaveDescription>(ARK_SAVE_DESCRIPTION_CONTINUE_TO_RECEIVE),
        SaveButtonSaveDescription::CONTINUE_TO_RECEIVE
    },
    {
        "ARK_SAVE_DESCRIPTION_SAVE_TO_GALLERY",
        Converter::ArkValue<Opt_SaveDescription>(ARK_SAVE_DESCRIPTION_SAVE_TO_GALLERY),
        SaveButtonSaveDescription::SAVE_TO_GALLERY
    },
    {
        "ARK_SAVE_DESCRIPTION_EXPORT_TO_GALLERY",
        Converter::ArkValue<Opt_SaveDescription>(ARK_SAVE_DESCRIPTION_EXPORT_TO_GALLERY),
        SaveButtonSaveDescription::EXPORT_TO_GALLERY
    },
    {
        "ARK_SAVE_DESCRIPTION_QUICK_SAVE_TO_GALLERY",
        Converter::ArkValue<Opt_SaveDescription>(ARK_SAVE_DESCRIPTION_QUICK_SAVE_TO_GALLERY),
        SaveButtonSaveDescription::QUICK_SAVE_TO_GALLERY
    },
    {
        "ARK_SAVE_DESCRIPTION_RESAVE_TO_GALLERY",
        Converter::ArkValue<Opt_SaveDescription>(ARK_SAVE_DESCRIPTION_RESAVE_TO_GALLERY),
        SaveButtonSaveDescription::RESAVE_TO_GALLERY
    },
};

// Valid values for attribute 'buttonType' of method 'setSaveButtonOptions'
static std::vector<std::tuple<std::string, Opt_ButtonType, ButtonType>> setSaveButtonOptionsButtonTypeValidValues = {
    {
        "ARK_BUTTON_TYPE_CAPSULE",
        Converter::ArkValue<Opt_ButtonType>(ARK_BUTTON_TYPE_CAPSULE),
        ButtonType::CAPSULE
    },
    {
        "ARK_BUTTON_TYPE_CIRCLE",
        Converter::ArkValue<Opt_ButtonType>(ARK_BUTTON_TYPE_CIRCLE),
        ButtonType::CIRCLE
    },
    {
        "ARK_BUTTON_TYPE_NORMAL",
        Converter::ArkValue<Opt_ButtonType>(ARK_BUTTON_TYPE_NORMAL),
        ButtonType::NORMAL
    },
    {
        "ARK_BUTTON_TYPE_ROUNDED_RECTANGLE",
        Converter::ArkValue<Opt_ButtonType>(ARK_BUTTON_TYPE_ROUNDED_RECTANGLE),
        ButtonType::ROUNDED_RECTANGLE
    },
};

/*
 * @tc.name: setSaveButtonOptions1TestValidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(SaveButtonModifierTest, setSaveButtonOptions1TestValidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    int32_t result;
    int32_t expected;
    Ark_SaveButtonOptions inputValueOptions;
    Ark_SaveButtonOptions initValueOptions;

    // Initial setup
    initValueOptions.icon = std::get<1>(setSaveButtonOptionsIconValidValues[0]);
    initValueOptions.text = std::get<1>(setSaveButtonOptionsTextValidValues[0]);
    initValueOptions.buttonType = std::get<1>(setSaveButtonOptionsButtonTypeValidValues[0]);

    // Verifying attribute's 'icon'  values
    inputValueOptions = initValueOptions;
    for (auto&& value: setSaveButtonOptionsIconValidValues) {
        inputValueOptions.icon = std::get<1>(value);
        auto node = CreateNode();
        modifier_->setSaveButtonOptions1(node, &inputValueOptions);
        jsonValue = GetJsonValue(node);
        DisposeNode(node);
        result = jsonValue->GetInt(ATTRIBUTE_ICON_NAME, DEFAULT_JSON_INT);
        expected = static_cast<int32_t>(std::get<2>(value));
        EXPECT_EQ(result, expected) << "Passed value is: " << std::get<0>(value);
    }

    // Verifying attribute's 'text'  values
    inputValueOptions = initValueOptions;
    for (auto&& value: setSaveButtonOptionsTextValidValues) {
        inputValueOptions.text = std::get<1>(value);
        auto node = CreateNode();
        modifier_->setSaveButtonOptions1(node, &inputValueOptions);
        jsonValue = GetJsonValue(node);
        DisposeNode(node);
        result = jsonValue->GetInt(ATTRIBUTE_TEXT_NAME, DEFAULT_JSON_INT);
        expected = static_cast<int32_t>(std::get<2>(value));
        EXPECT_EQ(result, expected) << "Passed value is: " << std::get<0>(value);
    }

    // Verifying attribute's 'buttonType'  values
    inputValueOptions = initValueOptions;
    for (auto&& value: setSaveButtonOptionsButtonTypeValidValues) {
        inputValueOptions.buttonType = std::get<1>(value);
        auto node = CreateNode();
        modifier_->setSaveButtonOptions1(node, &inputValueOptions);
        jsonValue = GetJsonValue(node);
        DisposeNode(node);
        result = jsonValue->GetInt(ATTRIBUTE_BUTTON_TYPE_NAME, DEFAULT_JSON_INT);
        expected = static_cast<int32_t>(std::get<2>(value));
        EXPECT_EQ(result, expected) << "Passed value is: " << std::get<0>(value);
    }
}

// Invalid values for attribute 'icon' of method 'setSaveButtonOptions'
static std::vector<std::tuple<std::string, Opt_SaveIconStyle>> setSaveButtonOptionsIconInvalidValues = {
    {"Ark_Empty()", Converter::ArkValue<Opt_SaveIconStyle>(Ark_Empty())},
    {"static_cast<Ark_SaveIconStyle>(-1)", Converter::ArkValue<Opt_SaveIconStyle>(static_cast<Ark_SaveIconStyle>(-1))},
};

// Invalid values for attribute 'text' of method 'setSaveButtonOptions'
static std::vector<std::tuple<std::string, Opt_SaveDescription>> setSaveButtonOptionsTextInvalidValues = {
    {
        "Ark_Empty()",
        Converter::ArkValue<Opt_SaveDescription>(Ark_Empty())
    },
    {
        "static_cast<Ark_SaveDescription>(-1)",
        Converter::ArkValue<Opt_SaveDescription>(static_cast<Ark_SaveDescription>(-1))
    },
};

// Invalid values for attribute 'buttonType' of method 'setSaveButtonOptions'
static std::vector<std::tuple<std::string, Opt_ButtonType>> setSaveButtonOptionsButtonTypeInvalidValues = {
    {"Ark_Empty()", Converter::ArkValue<Opt_ButtonType>(Ark_Empty())},
    {"static_cast<Ark_ButtonType>(-1)", Converter::ArkValue<Opt_ButtonType>(static_cast<Ark_ButtonType>(-1))},
};

/*
 * @tc.name: setSaveButtonOptions1TestInvalidValues
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(SaveButtonModifierTest, setSaveButtonOptions1TestInvalidValues, TestSize.Level1)
{
    std::unique_ptr<JsonValue> jsonValue;
    int32_t result;
    int32_t expected;
    Ark_SaveButtonOptions inputValueOptions;
    Ark_SaveButtonOptions initValueOptions;

    // Initial setup
    initValueOptions.icon = std::get<1>(setSaveButtonOptionsIconValidValues[0]);
    initValueOptions.text = std::get<1>(setSaveButtonOptionsTextValidValues[0]);
    initValueOptions.buttonType = std::get<1>(setSaveButtonOptionsButtonTypeValidValues[0]);

    // Verifying attribute's 'icon'  values
    for (auto&& value: setSaveButtonOptionsIconInvalidValues) {
        inputValueOptions = initValueOptions;
        inputValueOptions.icon = std::get<1>(value);
        auto node = CreateNode();
        modifier_->setSaveButtonOptions1(node, &inputValueOptions);
        jsonValue = GetJsonValue(node);
        DisposeNode(node);
        result = jsonValue->GetInt(ATTRIBUTE_ICON_NAME, DEFAULT_JSON_INT);
        expected = static_cast<int32_t>(ATTRIBUTE_ICON_DEFAULT_VALUE);
        EXPECT_EQ(result, expected) << "Passed value is: " << std::get<0>(value);
    }

    // Verifying attribute's 'text'  values
    for (auto&& value: setSaveButtonOptionsTextInvalidValues) {
        inputValueOptions = initValueOptions;
        inputValueOptions.text = std::get<1>(value);
        auto node = CreateNode();
        modifier_->setSaveButtonOptions1(node, &inputValueOptions);
        jsonValue = GetJsonValue(node);
        DisposeNode(node);
        result = jsonValue->GetInt(ATTRIBUTE_TEXT_NAME, DEFAULT_JSON_INT);
        expected = static_cast<int32_t>(ATTRIBUTE_TEXT_DEFAULT_VALUE);
        EXPECT_EQ(result, expected) << "Passed value is: " << std::get<0>(value);
    }

    // Verifying attribute's 'buttonType'  values
    for (auto&& value: setSaveButtonOptionsButtonTypeInvalidValues) {
        inputValueOptions = initValueOptions;
        inputValueOptions.buttonType = std::get<1>(value);
        auto node = CreateNode();
        modifier_->setSaveButtonOptions1(node, &inputValueOptions);
        jsonValue = GetJsonValue(node);
        DisposeNode(node);
        result = jsonValue->GetInt(ATTRIBUTE_BUTTON_TYPE_NAME, DEFAULT_JSON_INT);
        expected = static_cast<int32_t>(ATTRIBUTE_BUTTON_TYPE_DEFAULT_VALUE);
        EXPECT_EQ(result, expected) << "Passed value is: " << std::get<0>(value);
    }
}

/*
 * @tc.name: setSaveButtonOptions1TestTextAndIconEmpty
 * @tc.desc: Verify that all attributes are set to default values in case neither text nor icon is set.
 * @tc.type: FUNC
 */
HWTEST_F(SaveButtonModifierTest, setSaveButtonOptions1TestTextAndIconEmpty, TestSize.Level1)
{
    Ark_SaveButtonOptions inputValueOptions = {
        .text = Converter::ArkValue<Opt_SaveDescription>(Ark_Empty()),
        .icon = Converter::ArkValue<Opt_SaveIconStyle>(Ark_Empty()),
        .buttonType = Converter::ArkValue<Opt_ButtonType>(ARK_BUTTON_TYPE_NORMAL)
    };
    auto node = CreateNode();
    modifier_->setSaveButtonOptions1(node, &inputValueOptions);
    auto jsonValue = GetJsonValue(node);
    DisposeNode(node);
    int32_t resultText = jsonValue->GetInt(ATTRIBUTE_TEXT_NAME, DEFAULT_JSON_INT);
    int32_t resultIcon = jsonValue->GetInt(ATTRIBUTE_ICON_NAME, DEFAULT_JSON_INT);
    int32_t resultButtonType = jsonValue->GetInt(ATTRIBUTE_BUTTON_TYPE_NAME, DEFAULT_JSON_INT);
    EXPECT_EQ(resultText, static_cast<int32_t>(SaveButtonStyle::DEFAULT_TEXT));
    EXPECT_EQ(resultIcon, static_cast<int32_t>(SaveButtonStyle::DEFAULT_ICON));
    EXPECT_EQ(resultButtonType, static_cast<int32_t>(SaveButtonStyle::DEFAULT_BACKGROUND_TYPE));
}

/*
 * @tc.name: setOnClickTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(SaveButtonModifierTest, setOnClickTest, TestSize.Level1)
{
    static std::optional<Ark_SaveButtonOnClickResult> checkEvent;

    EventsTracker::eventReceiver.onClick = [](Ark_Int32 nodeId, const Ark_ClickEvent event,
        const Ark_SaveButtonOnClickResult result)
    {
        checkEvent = result;
    };

    modifier_->setOnClick(node_, {});

    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    ASSERT_NE(frameNode, nullptr);
    auto gestureEventHub = frameNode->GetOrCreateGestureEventHub();
    ASSERT_NE(gestureEventHub, nullptr);

    auto createJson = [](SecurityComponentHandleResult value)
    {
        int32_t res = static_cast<int32_t>(value);
        auto jsonNode = JsonUtil::Create(true);
        jsonNode->Put("handleRes", res);
        std::shared_ptr<JsonValue> jsonShrd(jsonNode.release());
        return jsonShrd;
    };

#ifdef SECURITY_COMPONENT_ENABLE
    checkEvent.reset();
    gestureEventHub->ActClick(createJson(SecurityComponentHandleResult::CLICK_SUCCESS));
    ASSERT_TRUE(checkEvent.has_value());
    EXPECT_EQ(checkEvent.value(), ARK_SAVE_BUTTON_ON_CLICK_RESULT_SUCCESS);

    checkEvent.reset();
    gestureEventHub->ActClick(createJson(SecurityComponentHandleResult::CLICK_GRANT_FAILED));
    ASSERT_TRUE(checkEvent.has_value());
    EXPECT_EQ(checkEvent.value(), ARK_SAVE_BUTTON_ON_CLICK_RESULT_TEMPORARY_AUTHORIZATION_FAILED);

    checkEvent.reset();
    gestureEventHub->ActClick(createJson(SecurityComponentHandleResult::DROP_CLICK));
    ASSERT_FALSE(checkEvent.has_value());
#else
    checkEvent.reset();
    gestureEventHub->ActClick(createJson(SecurityComponentHandleResult::CLICK_SUCCESS));
    ASSERT_TRUE(checkEvent.has_value());
    EXPECT_EQ(checkEvent.value(), ARK_SAVE_BUTTON_ON_CLICK_RESULT_TEMPORARY_AUTHORIZATION_FAILED);

    checkEvent.reset();
    gestureEventHub->ActClick(createJson(SecurityComponentHandleResult::CLICK_GRANT_FAILED));
    ASSERT_TRUE(checkEvent.has_value());
    EXPECT_EQ(checkEvent.value(), ARK_SAVE_BUTTON_ON_CLICK_RESULT_TEMPORARY_AUTHORIZATION_FAILED);

    checkEvent.reset();
    gestureEventHub->ActClick(createJson(SecurityComponentHandleResult::DROP_CLICK));
    ASSERT_TRUE(checkEvent.has_value());
    EXPECT_EQ(checkEvent.value(), ARK_SAVE_BUTTON_ON_CLICK_RESULT_TEMPORARY_AUTHORIZATION_FAILED);
#endif
}

} // namespace OHOS::Ace::NG