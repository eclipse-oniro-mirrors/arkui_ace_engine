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

#include "core/components/search/search_theme.h"
#include "core/components/text_field/textfield_theme.h"
#include "core/components/theme/icon_theme.h"
#include "core/components_ng/pattern/stage/page_event_hub.h"
#include "core/components_ng/pattern/search/search_pattern.h"

#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/utility/callback_helper.h"
#include "core/interfaces/native/implementation/search_controller_accessor_peer.h"

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;
using namespace Converter;

namespace {
// attrs
#ifdef WRONG_CALLBACK
const auto INPUT_FILTER_ATTR("inputFilter");
#endif
// check text
const auto CHECK_TEXT(u"test_text");
#ifdef WRONG_CALLBACK
const auto ERROR_TEXT("test_error_text");
#endif
PreviewText PREVIEW_TEXT = { .offset = 1234, .value = u"test_offset" };
const auto EMPTY_TEXT(u"");

const std::vector<TextDeleteDirection> DELETE_DIRECTION_TEST_PLAN = {
    TextDeleteDirection::FORWARD,
    TextDeleteDirection::BACKWARD,
};

// events
bool g_isEditChangeTest(true);
std::u16string g_EventTestString(u"");
std::string g_EventErrorTestString("");
int32_t g_EventTestOffset(0);
int32_t g_startValue(0);
int32_t g_endValue(0);
float g_scrollX(0);
float g_scrollY(0);
TextDeleteDirection g_deleteDirection(TextDeleteDirection::FORWARD);

// length values
const Ark_Int32 AINT32_POS(1234);
const Ark_Int32 AINT32_NEG(INT_MIN);
const Ark_Float32 AFLT32_POS(1.234f);
const Ark_Float32 AFLT32_NEG(-5.6789f);

using ArkNumberIntTest = std::pair<Ark_Int32, Ark_Int32>;
using ArkNumberFloatTest = std::pair<float, float>;

const std::vector<ArkNumberIntTest> INT_NUMBER_TEST_PLAN = {
    { AINT32_POS, AINT32_POS },
    { AINT32_NEG, AINT32_NEG },
};

const std::vector<ArkNumberFloatTest> FLOAT_NUMBER_TEST_PLAN = {
    { AFLT32_POS, AFLT32_POS },
    { AFLT32_NEG, AFLT32_NEG },
};

struct EventsTracker {
    static inline GENERATED_ArkUISearchEventsReceiver eventsReceiver {};

    static inline const GENERATED_ArkUIEventsAPI eventsApiImpl = {
        .getSearchEventsReceiver = [] () -> const GENERATED_ArkUISearchEventsReceiver* {
            return &eventsReceiver;
        }
    };
}; // EventsTracker
} // namespace

class SearchModifierCallbackTest : public ModifierTestBase<GENERATED_ArkUISearchModifier,
                               &GENERATED_ArkUINodeModifiers::getSearchModifier, GENERATED_ARKUI_SEARCH> {
public:
    static void SetUpTestCase()
    {
        ModifierTestBase::SetUpTestCase();

        SetupTheme<SearchTheme>();
        SetupTheme<TextFieldTheme>();
        SetupTheme<IconTheme>();

        fullAPI_->setArkUIEventsAPI(&EventsTracker::eventsApiImpl);
    }
};

/**
 * @tc.name: setSearchOptionsTestSearchController
 * @tc.desc: Check the functionality of setSearchOptions
 * @tc.type: FUNC
 */
HWTEST_F(SearchModifierCallbackTest, setSearchOptionsTestSearchController, TestSize.Level1)
{
    Ark_SearchOptions options = {};
    options.value = ArkValue<Opt_String>(Ark_Empty());
    options.placeholder = ArkUnion<Opt_ResourceStr, Ark_Empty>(nullptr);
    options.icon = ArkValue<Opt_String>(Ark_Empty());

    auto controllerPtr =
        fullAPI_->getAccessors()->getSearchControllerAccessor()->ctor();
    auto peerImplPtr = reinterpret_cast<SearchControllerPeer*>(controllerPtr);
    EXPECT_NE(peerImplPtr, nullptr);

    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    EXPECT_NE(frameNode, nullptr);

    Ark_SearchController arkController;
    arkController.ptr = controllerPtr;
    options.controller = Converter::ArkValue<Opt_SearchController>(arkController);
    auto optOptions = ArkValue<Opt_SearchOptions>(options);

    modifier_->setSearchOptions(node_, &optOptions);

    auto pattern = frameNode->GetPattern<SearchPattern>();
    EXPECT_NE(pattern, nullptr);
    auto controller = pattern->GetSearchController();
    EXPECT_NE(controller, nullptr);

    EXPECT_EQ(peerImplPtr->GetController(), controller);

    Ark_NativePointer finalizerPtr =
        fullAPI_->getAccessors()->getSearchControllerAccessor()->getFinalizer();
    auto finalyzer = reinterpret_cast<void (*)(SearchControllerPeer *)>(finalizerPtr);
    EXPECT_NE(finalyzer, nullptr);
    finalyzer(reinterpret_cast<SearchControllerPeer *>(controllerPtr));
}

/**
 * @tc.name: setInputFilterTest
 * @tc.desc: Check the functionality of setInputFilter
 * @tc.type: FUNC
 */
#ifdef WRONG_CALLBACK
HWTEST_F(SearchModifierCallbackTest, setInputFilterTest, TestSize.Level1)
{
    g_EventTestString = u"";
    g_EventErrorTestString = "";
    ASSERT_NE(modifier_->setInputFilter, nullptr);
    EventsTracker::eventsReceiver.inputFilter = [](Ark_Int32 nodeId, const Ark_String data) {
        g_EventErrorTestString = Convert<std::string>(data);
        g_EventTestString = g_EventErrorTestString;
    };
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    Opt_Callback_String_Void func{};
    auto textFieldChild = AceType::DynamicCast<FrameNode>(frameNode->GetChildren().front());
    auto textFieldEventHub = textFieldChild->GetEventHub<TextFieldEventHub>();

    auto sendString = ArkValue<Ark_String>(ERROR_TEXT);
    auto sendResource = ArkUnion<Ark_ResourceStr, Ark_String>(sendString);
    modifier_->setInputFilter(node_, &sendResource, &func);
    textFieldEventHub->FireOnInputFilterError(ERROR_TEXT);
    auto filterValue = GetStringAttribute(node_, INPUT_FILTER_ATTR);
    EXPECT_EQ(filterValue, ERROR_TEXT);
    EXPECT_EQ(g_EventTestString, ERROR_TEXT);
    EXPECT_EQ(g_EventErrorTestString, ERROR_TEXT);
}
#endif

/**
 * @tc.name: setOnTextSelectionChangeTest
 * @tc.desc: Check the functionality of setOnTextSelectionChange
 * @tc.type: FUNC
 */
HWTEST_F(SearchModifierCallbackTest, setOnTextSelectionChangeTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setOnTextSelectionChange, nullptr);
    EventsTracker::eventsReceiver.onTextSelectionChange = [](Ark_Int32 nodeId,
        Ark_Number selectionStart,
        Ark_Number selectionEnd) {
        g_startValue = Convert<int32_t>(selectionStart);
        g_endValue = Convert<int32_t>(selectionEnd);
    };
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    OnTextSelectionChangeCallback func{};
    auto textFieldChild = AceType::DynamicCast<FrameNode>(frameNode->GetChildren().front());
    auto textFieldEventHub = textFieldChild->GetEventHub<TextFieldEventHub>();

    for (const auto& [value, expectVal] : INT_NUMBER_TEST_PLAN) {
        modifier_->setOnTextSelectionChange(node_, &func);
        textFieldEventHub->FireOnSelectionChange(value, value);
        EXPECT_EQ(g_startValue, expectVal);
        EXPECT_EQ(g_endValue, expectVal);
    }
}

/**
 * @tc.name: setOnCopyTest
 * @tc.desc: Test Seacrh setOnCopy event.
 * @tc.type: FUNC
 */
HWTEST_F(SearchModifierCallbackTest, setOnCopyTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setOnCopy, nullptr);
    EventsTracker::eventsReceiver.onCopy = [](Ark_Int32 nodeId, Ark_String value) {
        auto textString = Convert<std::u16string>(value);
        g_EventTestString = textString;
    };
    g_EventTestString = EMPTY_TEXT;
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto textFieldChild = AceType::DynamicCast<FrameNode>(frameNode->GetChildren().front());
    auto textFieldEventHub = textFieldChild->GetEventHub<TextFieldEventHub>();
    ASSERT_NE(textFieldEventHub, nullptr);
    textFieldEventHub->FireOnCopy(CHECK_TEXT);
    EXPECT_EQ(g_EventTestString, EMPTY_TEXT);
    Callback_String_Void func{};
    modifier_->setOnCopy(node_, &func);
    textFieldEventHub->FireOnCopy(CHECK_TEXT);
    EXPECT_EQ(g_EventTestString, CHECK_TEXT);
}

/**
 * @tc.name: setOnCutTest
 * @tc.desc: Test Seacrh setOnCut event.
 * @tc.type: FUNC
 */
HWTEST_F(SearchModifierCallbackTest, setOnCutTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setOnCut, nullptr);
    EventsTracker::eventsReceiver.onCut = [](Ark_Int32 nodeId, Ark_String value) {
        g_EventTestString = Convert<std::u16string>(value);
    };
    g_EventTestString = EMPTY_TEXT;
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto textFieldChild = AceType::DynamicCast<FrameNode>(frameNode->GetChildren().front());
    auto textFieldEventHub = textFieldChild->GetEventHub<TextFieldEventHub>();
    ASSERT_NE(textFieldEventHub, nullptr);
    textFieldEventHub->FireOnCut(CHECK_TEXT);
    EXPECT_EQ(g_EventTestString, EMPTY_TEXT);
    Callback_String_Void func{};
    modifier_->setOnCut(node_, &func);
    textFieldEventHub->FireOnCut(CHECK_TEXT);
    EXPECT_EQ(g_EventTestString, CHECK_TEXT);
}

/**
 * @tc.name: setOnContentScrollTest
 * @tc.desc: Check the functionality of setOnContentScroll
 * @tc.type: FUNC
 */
HWTEST_F(SearchModifierCallbackTest, setOnContentScrollTest, TestSize.Level1)
{
    ASSERT_NE(modifier_->setOnContentScroll, nullptr);
    EventsTracker::eventsReceiver.onContentScroll = [](Ark_Int32 nodeId,
        Ark_Number totalOffsetX,
        Ark_Number totalOffsetY) {
        g_scrollX = Convert<float>(totalOffsetX);
        g_scrollY = Convert<float>(totalOffsetY);
    };
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    OnContentScrollCallback func{};
    auto textFieldChild = AceType::DynamicCast<FrameNode>(frameNode->GetChildren().front());
    auto textFieldEventHub = textFieldChild->GetEventHub<TextFieldEventHub>();

    for (const auto& [value, expectVal] : FLOAT_NUMBER_TEST_PLAN) {
        modifier_->setOnContentScroll(node_, &func);
        textFieldEventHub->FireOnScrollChangeEvent(value, value);
        EXPECT_EQ(g_scrollX, expectVal);
        EXPECT_EQ(g_scrollY, expectVal);
    }
}

/**
 * @tc.name: setOnEditChangeTest
 * @tc.desc: Test Seacrh setOnEditChange event.
 * @tc.type: FUNC
 */
HWTEST_F(SearchModifierCallbackTest, setOnEditChangeTest, TestSize.Level1)
{
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    EventsTracker::eventsReceiver.onEditChange = [](Ark_Int32 nodeId, const Ark_Boolean isEditChange) {
        g_isEditChangeTest = isEditChange;
    };
    Callback_Boolean_Void func{};
    modifier_->setOnEditChange(node_, &func);
    auto textFieldChild = AceType::DynamicCast<FrameNode>(frameNode->GetChildren().front());
    auto textFieldEventHub = textFieldChild->GetEventHub<TextFieldEventHub>();
    EXPECT_EQ(g_isEditChangeTest, true);
    ASSERT_NE(textFieldEventHub, nullptr);
    textFieldEventHub->FireOnEditChanged(false);
    EXPECT_EQ(g_isEditChangeTest, false);
    textFieldEventHub->FireOnEditChanged(true);
    EXPECT_EQ(g_isEditChangeTest, true);
}
/**
 * @tc.name: setOnSubmit0Test
 * @tc.desc: Test Seacrh setOnSubmit0 event.
 * @tc.type: FUNC
 */
HWTEST_F(SearchModifierCallbackTest, setOnSubmit0Test, TestSize.Level1)
{
    ASSERT_NE(modifier_->setOnSubmit0, nullptr);
    EventsTracker::eventsReceiver.onSubmit0 = [](Ark_Int32 nodeId, Ark_String value) {
        g_EventTestString = Convert<std::u16string>(value);
    };
    g_EventTestString = EMPTY_TEXT;
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    Callback_String_Void func{};
    modifier_->setOnSubmit0(node_, &func);
    auto searchEventHub = frameNode->GetEventHub<SearchEventHub>();
    EXPECT_EQ(g_EventTestString, EMPTY_TEXT);
    ASSERT_NE(searchEventHub, nullptr);
    NG::TextFieldCommonEvent event;
    searchEventHub->FireOnSubmit(CHECK_TEXT, event);
    EXPECT_EQ(g_EventTestString, CHECK_TEXT);
    searchEventHub->FireOnSubmit(EMPTY_TEXT, event);
    EXPECT_EQ(g_EventTestString, EMPTY_TEXT);
}

/**
 * @tc.name: setOnChangeTest
 * @tc.desc: Test Seacrh setOnChange event.
 * @tc.type: FUNC
 */
HWTEST_F(SearchModifierCallbackTest, setOnChangeTest, TestSize.Level1)
{
    g_EventTestString = u"";
    g_EventTestOffset = 0;
    EventsTracker::eventsReceiver.onChange = [](Ark_Int32 nodeId,
        const Ark_String value,
        const Opt_PreviewText previewText) {
        g_EventTestOffset = PREVIEW_TEXT.offset;
        g_EventTestString.append(CHECK_TEXT).append(PREVIEW_TEXT.value);
    };
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto textFieldChild = AceType::DynamicCast<FrameNode>(frameNode->GetChildren().front());
    auto textFieldEventHub = textFieldChild->GetEventHub<TextFieldEventHub>();
    ASSERT_NE(textFieldEventHub, nullptr);
    EXPECT_EQ(g_EventTestString, EMPTY_TEXT);
    EXPECT_EQ(g_EventTestOffset, 0);
    EditableTextOnChangeCallback func{};
    modifier_->setOnChange(node_, &func);
    textFieldEventHub->FireOnChange(CHECK_TEXT, PREVIEW_TEXT);
    std::u16string checkString = CHECK_TEXT;
    checkString.append(PREVIEW_TEXT.value);
    EXPECT_EQ(g_EventTestString, checkString);
    EXPECT_EQ(g_EventTestOffset, PREVIEW_TEXT.offset);
}

/**
 * @tc.name: setOnWillInsertTest
 * @tc.desc: Test Seacrh setOnWillInsert event.
 * @tc.type: FUNC
 */
HWTEST_F(SearchModifierCallbackTest, setOnWillInsertTest, TestSize.Level1)
{
    static const Ark_Int32 expectedResId = 123;
    auto onWillInsertHandler = [](Ark_VMContext context, const Ark_Int32 resourceId, const Ark_InsertValue data,
        const Callback_Boolean_Void cbReturn) {
        EXPECT_EQ(resourceId, expectedResId);
        EXPECT_EQ(Convert<std::u16string>(data.insertValue), CHECK_TEXT);
        auto result = Convert<int32_t>(data.insertOffset) > 0;
        CallbackHelper(cbReturn).Invoke(Converter::ArkValue<Ark_Boolean>(result));
    };
    auto arkFunc = Converter::ArkValue<Callback_InsertValue_Boolean>(nullptr, onWillInsertHandler, expectedResId);
    modifier_->setOnWillInsert(node_, &arkFunc);

    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto textFieldChild = AceType::DynamicCast<FrameNode>(frameNode->GetChildren().front());
    auto textFieldEventHub = textFieldChild->GetEventHub<TextFieldEventHub>();
    ASSERT_NE(textFieldEventHub, nullptr);

    {
        InsertValueInfo checkValue = { .insertOffset = AINT32_POS, .insertValue = CHECK_TEXT };
        auto returnVal = textFieldEventHub->FireOnWillInsertValueEvent(checkValue);
        EXPECT_TRUE(returnVal);
    }

    {
        InsertValueInfo checkValue = { .insertOffset = AINT32_NEG, .insertValue = CHECK_TEXT };
        auto returnVal = textFieldEventHub->FireOnWillInsertValueEvent(checkValue);
        EXPECT_FALSE(returnVal);
    }
}

/**
 * @tc.name: setOnDidInsertTest
 * @tc.desc: Test Seacrh setOnDidInsert event.
 * @tc.type: FUNC
 */
HWTEST_F(SearchModifierCallbackTest, setOnDidInsertTest, TestSize.Level1)
{
    g_EventTestString = u"";
    g_EventTestOffset = 0;
    EventsTracker::eventsReceiver.onDidInsert = [](Ark_Int32 nodeId, const Ark_InsertValue data) {
        g_EventTestString = Convert<std::u16string>(data.insertValue);
        g_EventTestOffset = Convert<int32_t>(data.insertOffset);
    };
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto textFieldChild = AceType::DynamicCast<FrameNode>(frameNode->GetChildren().front());
    auto textFieldEventHub = textFieldChild->GetEventHub<TextFieldEventHub>();
    ASSERT_NE(textFieldEventHub, nullptr);
    InsertValueInfo checkValueDefault;
    textFieldEventHub->FireOnDidInsertValueEvent(checkValueDefault);
    EXPECT_EQ(g_EventTestString, EMPTY_TEXT);
    EXPECT_EQ(g_EventTestOffset, 0);
    Callback_InsertValue_Void func{};
    modifier_->setOnDidInsert(node_, &func);
    for (const auto& [value, expectVal] : INT_NUMBER_TEST_PLAN) {
        InsertValueInfo checkValue = { .insertOffset = value, .insertValue = CHECK_TEXT };
        textFieldEventHub->FireOnDidInsertValueEvent(checkValue);
        EXPECT_EQ(g_EventTestString, CHECK_TEXT);
        EXPECT_EQ(g_EventTestOffset, expectVal);
    }
}

/**
 * @tc.name: setOnWillDeleteTest
 * @tc.desc: Test Seacrh setOnWillDelete event.
 * @tc.type: FUNC
 */
HWTEST_F(SearchModifierCallbackTest, setOnWillDeleteTest, TestSize.Level1)
{
    static const Ark_Int32 expectedResId = 123;
    static const Ark_Int32 expectedOffset = AINT32_POS;

    ASSERT_NE(modifier_->setOnWillDelete, nullptr);
    auto onWillDeleteHandler = [](Ark_VMContext context, const Ark_Int32 resourceId,
        const Ark_DeleteValue data, const Callback_Boolean_Void cbReturn) {
        EXPECT_EQ(resourceId, expectedResId);
        EXPECT_EQ(Convert<std::u16string>(data.deleteValue), CHECK_TEXT);
        EXPECT_EQ(Convert<int32_t>(data.deleteOffset), expectedOffset);
        auto willDeleteDirection = OptConvert<TextDeleteDirection>(data.direction);
        auto result = willDeleteDirection == TextDeleteDirection::FORWARD;
        CallbackHelper(cbReturn).Invoke(Converter::ArkValue<Ark_Boolean>(result));
    };
    auto arkFunc = Converter::ArkValue<Callback_DeleteValue_Boolean>(nullptr, onWillDeleteHandler, expectedResId);
    modifier_->setOnWillDelete(node_, &arkFunc);

    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto textFieldChild = AceType::DynamicCast<FrameNode>(frameNode->GetChildren().front());
    auto textFieldEventHub = textFieldChild->GetEventHub<TextFieldEventHub>();
    ASSERT_NE(textFieldEventHub, nullptr);
    DeleteValueInfo checkValueDefault;

    {
        DeleteValueInfo checkValue = {
            .deleteOffset = expectedOffset, .deleteValue = CHECK_TEXT, .direction = TextDeleteDirection::FORWARD
        };
        auto checkVal = textFieldEventHub->FireOnWillDeleteEvent(checkValue);
        EXPECT_TRUE(checkVal);
    }

    {
        DeleteValueInfo checkValue = {
            .deleteOffset = expectedOffset, .deleteValue = CHECK_TEXT, .direction = TextDeleteDirection::BACKWARD
        };
        auto checkVal = textFieldEventHub->FireOnWillDeleteEvent(checkValue);
        EXPECT_FALSE(checkVal);
    }
}

/**
 * @tc.name: setOnDidDeleteTest
 * @tc.desc: Test Seacrh setOnDidlDelete event.
 * @tc.type: FUNC
 */
HWTEST_F(SearchModifierCallbackTest, setOnDidDeleteTest, TestSize.Level1)
{
    g_EventTestString = u"";
    g_EventTestOffset = 0;
    EventsTracker::eventsReceiver.onDidDelete = [](Ark_Int32 nodeId, const Ark_DeleteValue data) {
        g_EventTestString = Convert<std::u16string>(data.deleteValue);
        g_EventTestOffset = Convert<int32_t>(data.deleteOffset);
        auto didDeleteDirection = OptConvert<TextDeleteDirection>(data.direction);
        if (didDeleteDirection) {
            g_deleteDirection = didDeleteDirection.value();
        }
    };
    g_deleteDirection = TextDeleteDirection::FORWARD;
    auto frameNode = reinterpret_cast<FrameNode*>(node_);
    auto textFieldChild = AceType::DynamicCast<FrameNode>(frameNode->GetChildren().front());
    auto textFieldEventHub = textFieldChild->GetEventHub<TextFieldEventHub>();
    ASSERT_NE(textFieldEventHub, nullptr);
    DeleteValueInfo checkValueDefault;
    textFieldEventHub->FireOnDidDeleteValueEvent(checkValueDefault);
    EXPECT_EQ(g_EventTestString, EMPTY_TEXT);
    EXPECT_EQ(g_EventTestOffset, 0);
    EXPECT_EQ(g_deleteDirection, TextDeleteDirection::FORWARD);
    Callback_DeleteValue_Void func{};
    modifier_->setOnDidDelete(node_, &func);
    for (const auto& [value, expectVal] : INT_NUMBER_TEST_PLAN) {
        for (const auto& deleteDirection : DELETE_DIRECTION_TEST_PLAN) {
            DeleteValueInfo checkValue = {
                .deleteOffset = value, .deleteValue = CHECK_TEXT, .direction = deleteDirection
            };
            textFieldEventHub->FireOnDidDeleteValueEvent(checkValue);
            EXPECT_EQ(g_EventTestString, CHECK_TEXT);
            EXPECT_EQ(g_EventTestOffset, expectVal);
            EXPECT_EQ(g_deleteDirection, deleteDirection);
        }
    }
}
} // namespace OHOS::Ace::NG
