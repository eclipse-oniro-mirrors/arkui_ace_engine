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

#include "test/mock/frameworks/core/components_ng/render/mock_paragraph.h"
#include "text_input_base.h"
#include "core/components_ng/pattern/search/search_pattern.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/common/ace_application_info.h"

namespace OHOS::Ace::NG {

namespace {} // namespace

class TextFieldPatternTestSeven : public TextInputBases {
public:
};

/**
 * @tc.name: ResetPreviewTextState001
 * @tc.desc: test testInput text ResetPreviewTextState
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, ResetPreviewTextState001, TestSize.Level0)
{
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->hasPreviewText_ = true;
    pattern->ResetPreviewTextState();
    EXPECT_TRUE(textFieldNode->isLayoutDirtyMarked_);
}

/**
 * @tc.name: UpdateSelectionByLongPress001
 * @tc.desc: test testInput text UpdateSelectionByLongPress
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, UpdateSelectionByLongPress001, TestSize.Level0)
{
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    pattern->contentController_->Reset();

    Offset offset(1.0, 1.0);
    pattern->magnifierController_->magnifierNodeExist_ = false;
    pattern->UpdateSelectionByLongPress(1, 1, offset);
    EXPECT_FALSE(pattern->magnifierController_->magnifierNodeExist_);
}

/**
 * @tc.name: UpdateSelectionByLongPress002
 * @tc.desc: test testInput text UpdateSelectionByLongPress
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, UpdateSelectionByLongPress002, TestSize.Level0)
{
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    Offset offset(1.0, 1.0);
    pattern->longPressFingerNum_ = 0;
    pattern->magnifierController_->magnifierNodeExist_ = false;
    pattern->UpdateSelectionByLongPress(1, 1, offset);
    EXPECT_FALSE(pattern->magnifierController_->magnifierNodeExist_);
}

/**
 * @tc.name: SetAccessibilityUnitAction001
 * @tc.desc: test testInput text SetAccessibilityUnitAction
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, SetAccessibilityUnitAction001, TestSize.Level0)
{
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto accessibilityProperty = textFieldNode->GetAccessibilityProperty<AccessibilityProperty>();

    pattern->unitNode_ = nullptr;
    pattern->responseArea_ = nullptr;
    pattern->SetAccessibilityUnitAction();
    EXPECT_FALSE(accessibilityProperty->accessibilityLevel_.has_value());
}

/**
 * @tc.name: SetAccessibilityUnitAction002
 * @tc.desc: test testInput text SetAccessibilityUnitAction
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, SetAccessibilityUnitAction002, TestSize.Level0)
{
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto accessibilityProperty = textFieldNode->GetAccessibilityProperty<AccessibilityProperty>();

    pattern->unitNode_ = nullptr;
    auto unitResponseArea =
        AceType::MakeRefPtr<UnitResponseArea>(AceType::WeakClaim(AceType::RawPtr(pattern)), textFieldNode);
    ASSERT_NE(unitResponseArea, nullptr);
    pattern->responseArea_ = std::move(unitResponseArea);
    pattern->SetAccessibilityUnitAction();
    EXPECT_FALSE(accessibilityProperty->accessibilityLevel_.has_value());
}

/**
 * @tc.name: AdjustSelectedBlankLineWidth001
 * @tc.desc: Test TextFieldPattern AdjustSelectedBlankLineWidth
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, AdjustSelectedBlankLineWidth001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create frameNode
     */
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();

    ASSERT_NE(pattern, nullptr);
    auto accessibilityProperty = textFieldNode->GetAccessibilityProperty<AccessibilityProperty>();

    RectF rect;
    pattern->AdjustSelectedBlankLineWidth(rect);
    ASSERT_EQ(NearZero(rect.GetX()), true);
    ASSERT_EQ(NearZero(rect.GetY()), true);
}

/**
 * @tc.name: GetGlyphPositionAtCoordinate001
 * @tc.desc: Test TextFieldPattern GetGlyphPositionAtCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, GetGlyphPositionAtCoordinate001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create frameNode
     */
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);

    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    auto positionWithAffinity = pattern->GetGlyphPositionAtCoordinate(1, 1);
    EXPECT_EQ(positionWithAffinity.position_, 0);
}

/**
 * @tc.name: UpdateSelectionAndHandleVisibility001
 * @tc.desc: Test TextFieldPattern UpdateSelectionAndHandleVisibility
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, UpdateSelectionAndHandleVisibility001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create frameNode
     */
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);

    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->UpdateSelectionAndHandleVisibility();
    ASSERT_NE(pattern, nullptr);
}

/**
 * @tc.name: CreateTextDragInfo001
 * @tc.desc: Test TextFieldPattern CreateTextDragInfo
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, CreateTextDragInfo001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create frameNode
     */
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);

    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    auto dragInfo = pattern->CreateTextDragInfo();
    ASSERT_EQ(NearZero(dragInfo.maxSelectedWidth), true);
}

/**
 * @tc.name: UpdateTextFieldInfo001
 * @tc.desc: Test TextFieldPattern UpdateTextFieldInfo
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, UpdateTextFieldInfo001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create frameNode
     */
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);

    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    pattern->UpdateTextFieldInfo();
    ASSERT_EQ(pattern->IsNeedProcessAutoFill(), true);
}

/**
 * @tc.name: ConvertTouchOffsetToTextOffset001
 * @tc.desc: Test TextFieldPattern ConvertTouchOffsetToTextOffset
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, ConvertTouchOffsetToTextOffset001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create frameNode
     */
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);

    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    Offset temp;
    auto ret = pattern->ConvertTouchOffsetToTextOffset(temp);
    ASSERT_EQ(NearZero(ret.GetX()), true);
    ASSERT_EQ(NearZero(ret.GetY()), true);
}

/**
 * @tc.name: ExpandDefaultResponseRegion001
 * @tc.desc: Test TextFieldPattern ExpandDefaultResponseRegion
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, ExpandDefaultResponseRegion001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create frameNode
     */
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);

    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    RectF rect;
    auto ret = pattern->ExpandDefaultResponseRegion(rect);
    ASSERT_EQ(NearZero(ret.GetX()), true);
    ASSERT_EQ(NearZero(ret.GetY()), false);
}

/**
 * @tc.name: CheckPreviewTextValidate001
 * @tc.desc: Test TextFieldPattern CheckPreviewTextValidate
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, CheckPreviewTextValidate001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create frameNode
     */
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    const std::string previewValue = "hhh";
    PreviewRange range;
    range.Set(0, 1);
    auto ret = pattern->CheckPreviewTextValidate(previewValue, range);
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: SetPreviewText001
 * @tc.desc: Test TextFieldPattern SetPreviewText
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, SetPreviewText001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create frameNode
     */
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    const std::string previewValue = "hhh";
    PreviewRange range;
    range.Set(0, 1);
    auto ret = pattern->SetPreviewText(previewValue, range);
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: ProcessThemePadding001
 * @tc.desc: test testInput text ProcessThemePadding
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, ProcessThemePadding001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create target node.
     */
    auto theme = GetTheme();
    CHECK_NULL_VOID(theme);
    CreateTextField();
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    RefPtr<TextFieldPattern> pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto textFieldTheme = GetTheme();
    CHECK_NULL_VOID(textFieldTheme);
    auto themePadding = pattern->IsUnderlineMode()
        ? pattern->GetUnderlinePadding(textFieldTheme, true, false) : textFieldTheme->GetPadding();
    ASSERT_EQ(themePadding.Left(), theme->GetPadding().Left());
}

/**
 * @tc.name: ProvidePlaceHolderText001
 * @tc.desc: test provide placeholder information to inputmethod function
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, ProvidePlaceHolderText, TestSize.Level1)
{
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);

    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
#if defined(ENABLE_STANDARD_INPUT)
    auto miscTextConfig = pattern->GetMiscTextConfig();
    auto textconfig = miscTextConfig.value();
    auto placeholder = UtfUtils::Str16ToStr8(textconfig.inputAttribute.placeholder).c_str();
    size_t count = 0;
    size_t i = 0;
    while (i < placeholder.size()) {
        count++;
        i += (placeholder[i] >= 0xD800 && placeholder[i] <= 0xDBFF) ? 2 : 1;
    }
    EXPECT_NE(count, 0);
#endif
}

/**
 * @tc.name: ProvideabilityNameText001
 * @tc.desc: test provide placeholder information to inputmethod function
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, ProvideabilityNameText, TestSize.Level1)
{
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);

    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
#if defined(ENABLE_STANDARD_INPUT)
    auto miscTextConfig = pattern->GetMiscTextConfig();
    auto textconfig = miscTextConfig.value();
    auto abilityName = UtfUtils::Str16ToStr8(textconfig.inputAttribute.abilityName).c_str();
    size_t count = 0;
    size_t i = 0;
    while (i < abilityName.size()) {
        count++;
        i += (abilityName[i] >= 0xD800 && abilityName[i] <= 0xDBFF) ? 2 : 1;
    }
    EXPECT_NE(count, 0);
#endif
}

/**
 * @tc.name: FireOnWillAttachIME001
 * @tc.desc: Test TextFieldPattern FireOnWillAttachIME
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, FireOnWillAttachIME001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize textInput and focusHub
     */
    CreateTextField();
    GetFocus();

    bool fireOnWillAttachIME = false;
    auto onWillAttachIME = [&fireOnWillAttachIME](const IMEClient& info) { fireOnWillAttachIME = true; };

    eventHub_->SetOnWillAttachIME(std::move(onWillAttachIME));
    pattern_->RequestKeyboard(false, true, true);

#if defined(ENABLE_STANDARD_INPUT)
    EXPECT_EQ(fireOnWillAttachIME, true);
#else
    EXPECT_EQ(fireOnWillAttachIME, false);
#endif
}

/**
 * @tc.name: FireOnWillAttachIME002
 * @tc.desc: Test TextFieldPattern FireOnWillAttachIME
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, FireOnWillAttachIME002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize textInput and focusHub
     */
    CreateTextField();
    GetFocus();

    bool fireOnWillAttachIME = false;
    auto onWillAttachIME = [&fireOnWillAttachIME](const IMEClient& info) { fireOnWillAttachIME = true; };

    eventHub_->SetOnWillAttachIME(std::move(onWillAttachIME));
    IMEClient clientInfo;
    pattern_->FireOnWillAttachIME(clientInfo);
    EXPECT_EQ(fireOnWillAttachIME, true);
}

/**
 * @tc.name: GetIMEClientInfo001
 * @tc.desc: Test TextFieldPattern GetIMEClientInfo
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, GetIMEClientInfo001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize textInput and focusHub
     */
    CreateTextField();
    auto host = pattern_->GetHost();
    EXPECT_NE(host, nullptr);

    IMEClient iMEClientInfo = pattern_->GetIMEClientInfo();
    EXPECT_EQ(iMEClientInfo.nodeId, host->GetId());
}

/**
 * @tc.name: SetAccessibilityErrorText001
 * @tc.desc: Test function SetAccessibilityErrorText.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, SetAccessibilityErrorText001, TestSize.Level0)
{
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetShowCounter(true);
        model.SetMaxLength(10);
        model.SetShowError(u"error", true);
    });
    GetFocus();

    ASSERT_NE(pattern_, nullptr);
    pattern_->SetAccessibilityErrorText();
    ASSERT_NE(accessibilityProperty_, nullptr);
    EXPECT_EQ(accessibilityProperty_->GetErrorText(), "error");
}

/**
 * @tc.name: SetAccessibilityErrorText002
 * @tc.desc: Test function SetAccessibilityErrorText.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, SetAccessibilityErrorText002, TestSize.Level0)
{
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetShowCounter(true);
        model.SetMaxLength(10);
        model.SetShowError(u"error", false);
    });
    GetFocus();

    ASSERT_NE(pattern_, nullptr);
    pattern_->SetAccessibilityErrorText();
    ASSERT_NE(accessibilityProperty_, nullptr);
    EXPECT_EQ(accessibilityProperty_->GetErrorText(), "");
}

/**
 * @tc.name: SetAccessibilityErrorText003
 * @tc.desc: Test function SetAccessibilityErrorText.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, SetAccessibilityErrorText003, TestSize.Level0)
{
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetShowCounter(true);
        model.SetMaxLength(10);
        model.SetShowError(u"error", true);
        model.SetInputStyle(InputStyle::INLINE);
    });
    GetFocus();

    ASSERT_NE(pattern_, nullptr);
    pattern_->SetAccessibilityErrorText();
    ASSERT_NE(accessibilityProperty_, nullptr);
    EXPECT_EQ(accessibilityProperty_->GetErrorText(), "");
}

/**
 * @tc.name: SetAccessibilityErrorText004
 * @tc.desc: Test function SetAccessibilityErrorText.
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, SetAccessibilityErrorText004, TestSize.Level0)
{
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetShowCounter(true);
        model.SetMaxLength(10);
        model.SetShowError(u"error", true);
    });
    GetFocus();
    ASSERT_NE(eventHub_, nullptr);
    eventHub_->SetEnabled(false);
    ASSERT_NE(pattern_, nullptr);
    pattern_->SetAccessibilityErrorText();
    ASSERT_NE(accessibilityProperty_, nullptr);
    EXPECT_EQ(accessibilityProperty_->GetErrorText(), "");
}

/**
 * @tc.name: CreateTextDragInfo002
 * @tc.desc: Test TextFieldPattern CreateTextDragInfo
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, CreateTextDragInfo002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create frameNode
     */
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);

    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);

    auto dragInfo = pattern->CreateTextDragInfo();
    EXPECT_EQ(dragInfo.dragBackgroundColor.value_or(Color::WHITE), Color::WHITE);
}

/**
 * @tc.name: ProcessMenuAndSelectionInBlurEvent001
 * @tc.desc: Test ProcessMenuAndSelectionInBlurEvent with shouldKeepSelection false
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, ProcessMenuAndSelectionInBlurEvent001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextField node and pattern
     */
    auto textFieldNode = FrameNode::GetOrCreateFrameNode(V2::TEXTINPUT_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<TextFieldPattern>(); });
    ASSERT_NE(textFieldNode, nullptr);
    auto pattern = textFieldNode->GetPattern<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    
    /**
     * @tc.steps: step2. Set initial state for testing
     */
    pattern->isKeyboardClosedByUser_ = false;
    pattern->blurReason_ = BlurReason::FOCUS_SWITCH;
    
    /**
     * @tc.steps: step3. Call ProcessMenuAndSelectionInBlurEvent with shouldKeepSelection false
     * @tc.expected: CloseSelectOverlay should be called
     */
    bool shouldKeepSelection = false;
    pattern->ProcessMenuAndSelectionInBlurEvent(shouldKeepSelection);
    EXPECT_FALSE(pattern->isKeyboardClosedByUser_);
}

/**
 * @tc.name: ProcessMenuAndSelectionInBlurEvent002
 * @tc.desc: Test ProcessMenuAndSelectionInBlurEvent with shouldKeepSelection true and IsSelected true
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, ProcessMenuAndSelectionInBlurEvent002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextField node and pattern
     */
    CreateTextField(DEFAULT_TEXT);
    ASSERT_NE(pattern_, nullptr);
    GetFocus();
    
    /**
     * @tc.steps: step2. Set selection state to simulate selected text
     */
    pattern_->selectController_->UpdateCaretIndex(5);
    pattern_->HandleOnSelectAll(true);
    
    /**
     * @tc.steps: step3. Call ProcessMenuAndSelectionInBlurEvent with shouldKeepSelection true
     * @tc.expected: HideMenu should be called when IsSelected and selectOverlay exist
     */
    bool shouldKeepSelection = true;
    pattern_->ProcessMenuAndSelectionInBlurEvent(shouldKeepSelection);
    EXPECT_TRUE(pattern_->IsSelected());
}

/**
 * @tc.name: ProcessMenuAndSelectionInBlurEvent003
 * @tc.desc: Test ProcessMenuAndSelectionInBlurEvent with shouldKeepSelection true but not selected
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, ProcessMenuAndSelectionInBlurEvent003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextField node and pattern
     */
    CreateTextField(DEFAULT_TEXT);
    ASSERT_NE(pattern_, nullptr);
    GetFocus();
    
    /**
     * @tc.steps: step2. Ensure text is not selected
     */
    pattern_->selectController_->UpdateCaretIndex(0);
    
    /**
     * @tc.steps: step3. Call ProcessMenuAndSelectionInBlurEvent with shouldKeepSelection true
     * @tc.expected: No action taken when not selected
     */
    bool shouldKeepSelection = true;
    pattern_->ProcessMenuAndSelectionInBlurEvent(shouldKeepSelection);
    EXPECT_FALSE(pattern_->IsSelected());
}

/**
 * @tc.name: HandleBetweenSelectedPosition001
 * @tc.desc: Test HandleBetweenSelectedPosition when not using mouse and overlay is on
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, HandleBetweenSelectedPosition001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextField and set up selection
     */
    CreateTextField(DEFAULT_TEXT);
    ASSERT_NE(pattern_, nullptr);
    GetFocus();
    
    /**
     * @tc.steps: step2. Setup gesture event and selection state
     */
    GestureEvent info;
    Offset globalOffset(100.0, 100.0);
    info.SetGlobalLocation(globalOffset);
    
    pattern_->HandleOnSelectAll(true);
    ASSERT_NE(pattern_->selectOverlay_, nullptr);
    
    /**
     * @tc.steps: step3. Call HandleBetweenSelectedPosition
     * @tc.expected: Should handle click within selected area
     */
    auto result = pattern_->HandleBetweenSelectedPosition(info);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: HandleBetweenSelectedPosition002
 * @tc.desc: Test HandleBetweenSelectedPosition when using mouse
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, HandleBetweenSelectedPosition002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextField and pattern
     */
    CreateTextField(DEFAULT_TEXT);
    ASSERT_NE(pattern_, nullptr);
    GetFocus();
    
    /**
     * @tc.steps: step2. Setup gesture event with mouse mode
     */
    GestureEvent info;
    Offset globalOffset(100.0, 100.0);
    info.SetGlobalLocation(globalOffset);
    
    ASSERT_NE(pattern_->selectOverlay_, nullptr);
    pattern_->selectOverlay_->SetUsingMouse(true);
    
    /**
     * @tc.steps: step3. Call HandleBetweenSelectedPosition with mouse mode
     * @tc.expected: Should return false when using mouse
     */
    auto result = pattern_->HandleBetweenSelectedPosition(info);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: HandleBetweenSelectedPosition003
 * @tc.desc: Test HandleBetweenSelectedPosition when UnFocusOnHandleClick is true
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, HandleBetweenSelectedPosition003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextField and set up selection
     */
    CreateTextField(DEFAULT_TEXT);
    ASSERT_NE(pattern_, nullptr);
    GetFocus();
    
    /**
     * @tc.steps: step2. Setup gesture event and special state
     */
    GestureEvent info;
    Offset globalOffset(100.0, 100.0);
    info.SetGlobalLocation(globalOffset);
    
    pattern_->HandleOnSelectAll(true);
    ASSERT_NE(pattern_->selectOverlay_, nullptr);
    pattern_->UnFocusOnHandleClick_ = true;
    
    /**
     * @tc.steps: step3. Call HandleBetweenSelectedPosition with UnFocusOnHandleClick true
     * @tc.expected: Should return true when UnFocusOnHandleClick and smart edge state
     */
    auto result = pattern_->HandleBetweenSelectedPosition(info);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetInspectorId001
 * @tc.desc: Test GetInspectorId with normal TextField node
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, GetInspectorId001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create parent node and TextField node
     */
    auto parentNode = FrameNode::CreateFrameNode("parent", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(parentNode, nullptr);
    
    CreateTextField(DEFAULT_TEXT);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(frameNode_, nullptr);
    frameNode_->SetParent(parentNode);
    ViewAbstract::SetInspectorId(AceType::RawPtr(frameNode_), "testTextField");
    
    /**
     * @tc.steps: step2. Call GetInspectorId
     * @tc.expected: Should return the set inspector id
     */
    auto inspectorId = pattern_->GetInspectorId();
    EXPECT_EQ(inspectorId, "testTextField");
}

/**
 * @tc.name: GetInspectorId002
 * @tc.desc: Test GetInspectorId with Search parent and special prefix
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, GetInspectorId002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create Search parent node
     */
    auto searchNode = FrameNode::GetOrCreateFrameNode(V2::SEARCH_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<SearchPattern>(); });
    ASSERT_NE(searchNode, nullptr);
    ViewAbstract::SetInspectorId(AceType::RawPtr(searchNode), "searchParent");
    
    /**
     * @tc.steps: step2. Create TextField with special inspector id
     */
    CreateTextField(DEFAULT_TEXT);
    ASSERT_NE(frameNode_, nullptr);
    ViewAbstract::SetInspectorId(AceType::RawPtr(frameNode_), "__SearchField__123");
    
    /**
     * @tc.steps: step3. Set parent to Search node
     */
    frameNode_->SetParent(searchNode);
    
    /**
     * @tc.steps: step4. Call GetInspectorId
     * @tc.expected: Should return parent's inspector id when has SearchField prefix and Search parent
     */
    auto inspectorId = pattern_->GetInspectorId();
    EXPECT_NE(inspectorId, "");
}

/**
 * @tc.name: GetInspectorId003
 * @tc.desc: Test GetInspectorId with null host
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, GetInspectorId003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextField pattern without host
     */
    auto pattern = AceType::MakeRefPtr<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    
    /**
     * @tc.steps: step2. Call GetInspectorId
     * @tc.expected: Should return empty string when host is null
     */
    auto inspectorId = pattern->GetInspectorId();
    EXPECT_EQ(inspectorId, "");
}

/**
 * @tc.name: GetInspectorId004
 * @tc.desc: Test GetInspectorId with a parent and an unset inspector id
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, GetInspectorId004, TestSize.Level0)
{
    auto parentNode = FrameNode::CreateFrameNode("parent", ElementRegister::GetInstance()->MakeUniqueId(),
        AceType::MakeRefPtr<Pattern>());
    ASSERT_NE(parentNode, nullptr);

    CreateTextField(DEFAULT_TEXT);
    ASSERT_NE(pattern_, nullptr);
    ASSERT_NE(frameNode_, nullptr);
    frameNode_->SetParent(parentNode);
    frameNode_->ResetInspectorId();
    ASSERT_NE(frameNode_->GetParent(), nullptr);
    ASSERT_FALSE(frameNode_->GetInspectorId().has_value());

    EXPECT_EQ(pattern_->GetInspectorId(), "");
}

/**
 * @tc.name: GetUnderlinePadding001
 * @tc.desc: Test GetUnderlinePadding in underline mode with RTL direction
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, GetUnderlinePadding001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextField in underline and password mode
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetInputStyle(InputStyle::DEFAULT);
        model.SetShowUnderline(true);
        model.SetType(TextInputType::VISIBLE_PASSWORD);
    });
    ASSERT_NE(pattern_, nullptr);
    auto theme = GetTheme();
    ASSERT_NE(theme, nullptr);
    
    /**
     * @tc.steps: step2. Set RTL direction and force layout
     */
    layoutProperty_->UpdateLayoutDirection(TextDirection::RTL);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushLayoutTask(frameNode_);
    
    /**
     * @tc.steps: step3. Call GetUnderlinePadding
     * @tc.expected: Should adjust left padding for RTL in underline+button mode
     */
    auto padding = pattern_->GetUnderlinePadding(theme, false, false);
    EXPECT_EQ(padding.Left(), theme->GetPadding().Left());
}

/**
 * @tc.name: GetUnderlinePadding002
 * @tc.desc: Test GetUnderlinePadding in underline mode with LTR direction
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, GetUnderlinePadding002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextField in underline and password mode
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetInputStyle(InputStyle::DEFAULT);
        model.SetShowUnderline(true);
        model.SetType(TextInputType::VISIBLE_PASSWORD);
    });
    ASSERT_NE(pattern_, nullptr);
    auto theme = GetTheme();
    ASSERT_NE(theme, nullptr);
    
    /**
     * @tc.steps: step2. Set LTR direction and force layout
     */
    layoutProperty_->UpdateLayoutDirection(TextDirection::LTR);
    frameNode_->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    FlushLayoutTask(frameNode_);
    
    /**
     * @tc.steps: step3. Call GetUnderlinePadding
     * @tc.expected: Should adjust right padding for LTR in underline+button mode
     */
    auto padding = pattern_->GetUnderlinePadding(theme, false, false);
    EXPECT_EQ(padding.Right(), theme->GetPadding().Right());
}

/**
 * @tc.name: GetUnderlinePadding003
 * @tc.desc: Test GetUnderlinePadding with processLeftPadding true
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, GetUnderlinePadding003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextField in underline and password mode
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetInputStyle(InputStyle::DEFAULT);
        model.SetShowUnderline(true);
        model.SetType(TextInputType::VISIBLE_PASSWORD);
    });
    ASSERT_NE(pattern_, nullptr);
    auto theme = GetTheme();
    ASSERT_NE(theme, nullptr);
    
    /**
     * @tc.steps: step2. Call GetUnderlinePadding with processLeftPadding true
     * @tc.expected: Should set left padding from theme's padding
     */
    auto padding = pattern_->GetUnderlinePadding(theme, true, false);
    EXPECT_EQ(padding.Left(), theme->GetPadding().Left());
}

/**
 * @tc.name: GetUnderlinePadding004
 * @tc.desc: Test GetUnderlinePadding with processRightPadding true
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, GetUnderlinePadding004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextField in underline and password mode
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetInputStyle(InputStyle::DEFAULT);
        model.SetShowUnderline(true);
        model.SetType(TextInputType::VISIBLE_PASSWORD);
    });
    ASSERT_NE(pattern_, nullptr);
    auto theme = GetTheme();
    ASSERT_NE(theme, nullptr);
    
    /**
     * @tc.steps: step2. Call GetUnderlinePadding with processRightPadding true
     * @tc.expected: Should set right padding from theme's padding
     */
    auto padding = pattern_->GetUnderlinePadding(theme, false, true);
    EXPECT_EQ(padding.Right(), theme->GetPadding().Right());
}

/**
 * @tc.name: GetUnderlinePadding005
 * @tc.desc: Test GetUnderlinePadding in inline mode
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, GetUnderlinePadding005, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextField in inline mode
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetInputStyle(InputStyle::INLINE);
    });
    ASSERT_NE(pattern_, nullptr);
    auto theme = GetTheme();
    ASSERT_NE(theme, nullptr);
    
    /**
     * @tc.steps: step2. Call GetUnderlinePadding in inline mode
     * @tc.expected: Should return theme's underline padding directly
     */
    auto padding = pattern_->GetUnderlinePadding(theme, false, false);
    EXPECT_EQ(padding.Left(), theme->GetUnderlinePadding().Left());
    EXPECT_EQ(padding.Right(), theme->GetUnderlinePadding().Right());
}

/**
 * @tc.name: GetUnderlinePadding006
 * @tc.desc: Test GetUnderlinePadding with default theme and underline mode disabled
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, GetUnderlinePadding006, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextField pattern without underline mode
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetInputStyle(InputStyle::DEFAULT);
    });
    ASSERT_NE(pattern_, nullptr);
    auto theme = GetTheme();
    ASSERT_NE(theme, nullptr);
    
    /**
     * @tc.steps: step2. Call GetUnderlinePadding in non-underline mode
     * @tc.expected: Should return theme's default underline padding
     */
    auto padding = pattern_->GetUnderlinePadding(theme, false, false);
    auto expectedPadding = theme->GetUnderlinePadding();
    EXPECT_EQ(padding.Left(), expectedPadding.Left());
    EXPECT_EQ(padding.Right(), expectedPadding.Right());
}

/**
 * @tc.name: GetUnderlinePadding007
 * @tc.desc: Test GetUnderlinePadding with all padding processing flags true
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, GetUnderlinePadding007, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextField in underline and password mode
     */
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetInputStyle(InputStyle::DEFAULT);
        model.SetShowUnderline(true);
        model.SetType(TextInputType::VISIBLE_PASSWORD);
    });
    ASSERT_NE(pattern_, nullptr);
    auto theme = GetTheme();
    ASSERT_NE(theme, nullptr);
    
    /**
     * @tc.steps: step2. Call GetUnderlinePadding with all flags true
     * @tc.expected: Should process both left and right padding
     */
    auto padding = pattern_->GetUnderlinePadding(theme, true, true);
    EXPECT_EQ(padding.Left(), theme->GetPadding().Left());
    EXPECT_EQ(padding.Right(), theme->GetPadding().Right());
}

/**
 * @tc.name: GetUnderlinePadding008
 * @tc.desc: Test GetUnderlinePadding when api target version is less than VERSION_EIGHTEEN
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, GetUnderlinePadding008, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create TextField in underline and password mode with a low api version
     */
    auto backupApiVersion = AceApplicationInfo::GetInstance().GetApiTargetVersion();
    AceApplicationInfo::GetInstance().SetApiTargetVersion(static_cast<int32_t>(PlatformVersion::VERSION_TWELVE));
    CreateTextField(DEFAULT_TEXT, "", [](TextFieldModelNG model) {
        model.SetInputStyle(InputStyle::DEFAULT);
        model.SetShowUnderline(true);
        model.SetType(TextInputType::VISIBLE_PASSWORD);
    });
    ASSERT_NE(pattern_, nullptr);
    auto theme = GetTheme();
    ASSERT_NE(theme, nullptr);
    AceApplicationInfo::GetInstance().SetApiTargetVersion(backupApiVersion);

    /**
     * @tc.steps: step2. Set RTL direction and call GetUnderlinePadding with both flags true
     * @tc.expected: Should return theme's underline padding directly when api version is low
     */
    layoutProperty_->UpdateLayoutDirection(TextDirection::RTL);
    auto padding = pattern_->GetUnderlinePadding(theme, true, true);
    EXPECT_EQ(padding.Left(), theme->GetUnderlinePadding().Left());
    EXPECT_EQ(padding.Right(), theme->GetUnderlinePadding().Right());
}

/**
 * @tc.name: GetUnderlinePadding009
 * @tc.desc: Test GetUnderlinePadding when pattern has no host node
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, GetUnderlinePadding009, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create a standalone pattern without host node
     */
    auto pattern = AceType::MakeRefPtr<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto theme = GetTheme();
    ASSERT_NE(theme, nullptr);

    /**
     * @tc.steps: step2. Call GetUnderlinePadding
     * @tc.expected: Should return theme's underline padding when host is null
     */
    auto padding = pattern->GetUnderlinePadding(theme, true, true);
    EXPECT_EQ(padding.Left(), theme->GetUnderlinePadding().Left());
    EXPECT_EQ(padding.Right(), theme->GetUnderlinePadding().Right());
}

/**
 * @tc.name: OnThemeScopeUpdateColor001
 * @tc.desc: Test OnThemeScopeUpdateColor when pattern has no host node
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, OnThemeScopeUpdateColor001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a standalone pattern without host node
     */
    auto pattern = AceType::MakeRefPtr<TextFieldPattern>();
    ASSERT_NE(pattern, nullptr);
    auto theme = AceType::MakeRefPtr<TextFieldTheme>();
    ASSERT_NE(theme, nullptr);

    /**
     * @tc.steps: step2. Call OnThemeScopeUpdateColor
     * @tc.expected: Should return false when host is null
     */
    EXPECT_FALSE(pattern->OnThemeScopeUpdateColor(nullptr, nullptr, theme));
}

/**
 * @tc.name: OnThemeScopeUpdateColor002
 * @tc.desc: Test OnThemeScopeUpdateColor when paint property is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, OnThemeScopeUpdateColor002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField and a theme
     */
    CreateTextField();
    ASSERT_NE(pattern_, nullptr);
    auto theme = AceType::MakeRefPtr<TextFieldTheme>();
    ASSERT_NE(theme, nullptr);

    /**
     * @tc.steps: step2. Call OnThemeScopeUpdateColor with null paint property
     * @tc.expected: Should return false
     */
    EXPECT_FALSE(pattern_->OnThemeScopeUpdateColor(layoutProperty_, nullptr, theme));
}

/**
 * @tc.name: OnThemeScopeUpdateColor003
 * @tc.desc: Test OnThemeScopeUpdateColor when text field theme is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, OnThemeScopeUpdateColor003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField and get paint property
     */
    CreateTextField();
    ASSERT_NE(pattern_, nullptr);
    auto paintProperty = frameNode_->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);

    /**
     * @tc.steps: step2. Call OnThemeScopeUpdateColor with null theme
     * @tc.expected: Should return false
     */
    EXPECT_FALSE(pattern_->OnThemeScopeUpdateColor(layoutProperty_, paintProperty, nullptr));
}

/**
 * @tc.name: OnThemeScopeUpdateColor004
 * @tc.desc: Test selected background color update when theme selected color alpha equals DEFAULT_ALPHA
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, OnThemeScopeUpdateColor004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField and a theme whose selected color alpha is DEFAULT_ALPHA
     */
    CreateTextField();
    ASSERT_NE(pattern_, nullptr);
    auto paintProperty = frameNode_->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto theme = AceType::MakeRefPtr<TextFieldTheme>();
    ASSERT_NE(theme, nullptr);
    theme->selectedColor_ = Color(0xFF112233);
    EXPECT_FALSE(paintProperty->HasSelectedBackgroundColor());

    /**
     * @tc.steps: step2. Call OnThemeScopeUpdateColor
     * @tc.expected: Selected color should be changed with default opacity
     */
    EXPECT_TRUE(pattern_->OnThemeScopeUpdateColor(layoutProperty_, paintProperty, theme));
    EXPECT_TRUE(paintProperty->HasSelectedBackgroundColor());
    EXPECT_EQ(paintProperty->GetSelectedBackgroundColorValue(), Color(0xFF112233).ChangeOpacity(0.2));
}

/**
 * @tc.name: OnThemeScopeUpdateColor005
 * @tc.desc: Test selected background color update when theme selected color alpha is not DEFAULT_ALPHA
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, OnThemeScopeUpdateColor005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField and a theme whose selected color alpha is not DEFAULT_ALPHA
     */
    CreateTextField();
    ASSERT_NE(pattern_, nullptr);
    auto paintProperty = frameNode_->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto theme = AceType::MakeRefPtr<TextFieldTheme>();
    ASSERT_NE(theme, nullptr);
    theme->selectedColor_ = Color(0x80112233);

    /**
     * @tc.steps: step2. Call OnThemeScopeUpdateColor
     * @tc.expected: Selected color should be used directly without opacity change
     */
    EXPECT_TRUE(pattern_->OnThemeScopeUpdateColor(layoutProperty_, paintProperty, theme));
    EXPECT_TRUE(paintProperty->HasSelectedBackgroundColor());
    EXPECT_EQ(paintProperty->GetSelectedBackgroundColorValue(), Color(0x80112233));
}

/**
 * @tc.name: OnThemeScopeUpdateColor006
 * @tc.desc: Test OnThemeScopeUpdateColor when selected background color is set by user
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, OnThemeScopeUpdateColor006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField and set selected background color flag by user
     */
    CreateTextField();
    ASSERT_NE(pattern_, nullptr);
    auto paintProperty = frameNode_->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto theme = AceType::MakeRefPtr<TextFieldTheme>();
    ASSERT_NE(theme, nullptr);
    theme->selectedColor_ = Color(0xFF112233);
    paintProperty->UpdateSelectedBackgroundColorFlagByUser(true);

    /**
     * @tc.steps: step2. Call OnThemeScopeUpdateColor
     * @tc.expected: Selected background color should not be updated and result is false
     */
    EXPECT_FALSE(pattern_->OnThemeScopeUpdateColor(layoutProperty_, paintProperty, theme));
    EXPECT_FALSE(paintProperty->HasSelectedBackgroundColor());
}

/**
 * @tc.name: OnThemeScopeUpdateColor007
 * @tc.desc: Test counter border with theme over counter color when border width is not set by user
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, OnThemeScopeUpdateColor007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField, get focus and enable counter border style
     */
    CreateTextField();
    ASSERT_NE(pattern_, nullptr);
    GetFocus();
    auto paintProperty = frameNode_->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto theme = AceType::MakeRefPtr<TextFieldTheme>();
    ASSERT_NE(theme, nullptr);
    theme->overCounterColor_ = Color(0xFF0000FF);
    pattern_->showCountBorderStyle_ = true;

    /**
     * @tc.steps: step2. Call OnThemeScopeUpdateColor
     * @tc.expected: Inner border should use theme's over counter color
     */
    EXPECT_TRUE(pattern_->OnThemeScopeUpdateColor(layoutProperty_, paintProperty, theme));
    EXPECT_TRUE(paintProperty->HasInnerBorderWidth());
    EXPECT_TRUE(paintProperty->HasInnerBorderColor());
    EXPECT_EQ(paintProperty->GetInnerBorderColorValue(), Color(0xFF0000FF));
    EXPECT_TRUE(paintProperty->HasSelectedBackgroundColor());
}

/**
 * @tc.name: OnThemeScopeUpdateColor008
 * @tc.desc: Test counter border with counter text overflow color when border width is not set by user
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, OnThemeScopeUpdateColor008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField, get focus, enable counter style and set counter overflow color
     */
    CreateTextField();
    ASSERT_NE(pattern_, nullptr);
    GetFocus();
    auto paintProperty = frameNode_->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto theme = AceType::MakeRefPtr<TextFieldTheme>();
    ASSERT_NE(theme, nullptr);
    theme->overCounterColor_ = Color(0xFF0000FF);
    layoutProperty_->UpdateCounterTextOverflowColor(Color(0xFF123456));
    pattern_->showCountBorderStyle_ = true;

    /**
     * @tc.steps: step2. Call OnThemeScopeUpdateColor
     * @tc.expected: Inner border color should use user's counter text overflow color
     */
    EXPECT_TRUE(pattern_->OnThemeScopeUpdateColor(layoutProperty_, paintProperty, theme));
    EXPECT_TRUE(paintProperty->HasInnerBorderWidth());
    EXPECT_EQ(paintProperty->GetInnerBorderColorValue(), Color(0xFF123456));
}

/**
 * @tc.name: OnThemeScopeUpdateColor009
 * @tc.desc: Test counter border with theme over counter color when border width is set by user
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, OnThemeScopeUpdateColor009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField, get focus, enable counter style and set border width by user
     */
    CreateTextField();
    ASSERT_NE(pattern_, nullptr);
    GetFocus();
    auto paintProperty = frameNode_->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto theme = AceType::MakeRefPtr<TextFieldTheme>();
    ASSERT_NE(theme, nullptr);
    theme->overCounterColor_ = Color(0xFF00FF00);
    pattern_->showCountBorderStyle_ = true;
    paintProperty->UpdateBorderWidthFlagByUser(BorderWidthProperty {});

    /**
     * @tc.steps: step2. Call OnThemeScopeUpdateColor
     * @tc.expected: Border color should be updated through render context and inner border width not touched
     */
    EXPECT_TRUE(pattern_->OnThemeScopeUpdateColor(layoutProperty_, paintProperty, theme));
    EXPECT_FALSE(paintProperty->HasInnerBorderWidth());
}

/**
 * @tc.name: OnThemeScopeUpdateColor010
 * @tc.desc: Test counter border with counter overflow color when border width is set by user
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, OnThemeScopeUpdateColor010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField, get focus, enable counter style, set border width and overflow color by user
     */
    CreateTextField();
    ASSERT_NE(pattern_, nullptr);
    GetFocus();
    auto paintProperty = frameNode_->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto theme = AceType::MakeRefPtr<TextFieldTheme>();
    ASSERT_NE(theme, nullptr);
    theme->overCounterColor_ = Color(0xFF00FF00);
    layoutProperty_->UpdateCounterTextOverflowColor(Color(0xFF123456));
    pattern_->showCountBorderStyle_ = true;
    paintProperty->UpdateBorderWidthFlagByUser(BorderWidthProperty {});

    /**
     * @tc.steps: step2. Call OnThemeScopeUpdateColor
     * @tc.expected: Border color should use user's counter overflow color through render context
     */
    EXPECT_TRUE(pattern_->OnThemeScopeUpdateColor(layoutProperty_, paintProperty, theme));
    EXPECT_FALSE(paintProperty->HasInnerBorderWidth());
}

/**
 * @tc.name: OnThemeScopeUpdateColor011
 * @tc.desc: Test counter border not updated when counter style is enabled but has no focus
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, OnThemeScopeUpdateColor011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField and enable counter style without focus
     */
    CreateTextField();
    ASSERT_NE(pattern_, nullptr);
    auto paintProperty = frameNode_->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto theme = AceType::MakeRefPtr<TextFieldTheme>();
    ASSERT_NE(theme, nullptr);
    pattern_->showCountBorderStyle_ = true;

    /**
     * @tc.steps: step2. Call OnThemeScopeUpdateColor
     * @tc.expected: Counter border should not be updated without focus
     */
    EXPECT_TRUE(pattern_->OnThemeScopeUpdateColor(layoutProperty_, paintProperty, theme));
    EXPECT_FALSE(paintProperty->HasInnerBorderWidth());
    EXPECT_TRUE(paintProperty->HasSelectedBackgroundColor());
}

/**
 * @tc.name: OnThemeScopeUpdateColor012
 * @tc.desc: Test counter border not updated when focus is got but counter style is disabled
 * @tc.type: FUNC
 */
HWTEST_F(TextFieldPatternTestSeven, OnThemeScopeUpdateColor012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create TextField and get focus without counter style
     */
    CreateTextField();
    ASSERT_NE(pattern_, nullptr);
    GetFocus();
    auto paintProperty = frameNode_->GetPaintProperty<TextFieldPaintProperty>();
    ASSERT_NE(paintProperty, nullptr);
    auto theme = AceType::MakeRefPtr<TextFieldTheme>();
    ASSERT_NE(theme, nullptr);

    /**
     * @tc.steps: step2. Call OnThemeScopeUpdateColor
     * @tc.expected: Counter border should not be updated without counter style
     */
    EXPECT_TRUE(pattern_->OnThemeScopeUpdateColor(layoutProperty_, paintProperty, theme));
    EXPECT_FALSE(paintProperty->HasInnerBorderWidth());
    EXPECT_TRUE(paintProperty->HasSelectedBackgroundColor());
}
}