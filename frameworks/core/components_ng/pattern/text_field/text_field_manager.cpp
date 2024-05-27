/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/text_field/text_field_manager.h"

#include "base/geometry/dimension.h"
#include "base/memory/ace_type.h"
#include "base/utils/utils.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_ng/pattern/text/text_base.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"

namespace OHOS::Ace::NG {
namespace {
constexpr Dimension RESERVE_BOTTOM_HEIGHT = 24.0_vp;
} // namespace

void TextFieldManagerNG::ClearOnFocusTextField()
{
    onFocusTextField_ = nullptr;
}

void TextFieldManagerNG::ClearOnFocusTextField(int32_t id)
{
    if (onFocusTextFieldId == id) {
        onFocusTextField_ = nullptr;
    }
}

void TextFieldManagerNG::SetClickPosition(const Offset& position)
{
    auto pipeline =  PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto rootHeight = pipeline->GetRootHeight();
    if (GreatOrEqual(position.GetY(), rootHeight) || LessOrEqual(position.GetY(), 0.0f)) {
        return;
    }
    auto rootWidth = pipeline->GetRootWidth();
    if (GreatOrEqual(position.GetX(), rootWidth) || LessOrEqual(position.GetX(), 0.0f)) {
        return;
    }
    position_ = position;
}

bool TextFieldManagerNG::OnBackPressed()
{
    auto pattern = onFocusTextField_.Upgrade();
    CHECK_NULL_RETURN(pattern, false);
    auto textBasePattern = AceType::DynamicCast<TextBase>(pattern);
    CHECK_NULL_RETURN(textBasePattern, false);
    return textBasePattern->OnBackPressed();
}

RefPtr<FrameNode> TextFieldManagerNG::FindScrollableOfFocusedTextField(const RefPtr<FrameNode>& textField)
{
    CHECK_NULL_RETURN(textField, {});
    auto parent = textField->GetAncestorNodeOfFrame();
    while (parent) {
        auto pattern = parent->GetPattern<ScrollablePattern>();
        if (pattern) {
            return parent;
        }
        parent = parent->GetAncestorNodeOfFrame();
    }
    return {};
}

void TextFieldManagerNG::ScrollToSafeAreaHelper(
    const SafeAreaInsets::Inset& bottomInset, bool isShowKeyboard)
{
    auto node = onFocusTextField_.Upgrade();
    CHECK_NULL_VOID(node);
    auto frameNode = node->GetHost();
    CHECK_NULL_VOID(frameNode);
    auto textBase = DynamicCast<TextBase>(node);
    CHECK_NULL_VOID(textBase);
    textBase->OnVirtualKeyboardAreaChanged();

    auto scrollableNode = FindScrollableOfFocusedTextField(frameNode);
    CHECK_NULL_VOID(scrollableNode);
    auto scrollPattern = scrollableNode->GetPattern<ScrollablePattern>();
    CHECK_NULL_VOID(scrollPattern && scrollPattern->IsScrollToSafeAreaHelper());
    if (scrollPattern->GetAxis() == Axis::HORIZONTAL) {
        return;
    }

    auto scrollableRect = scrollableNode->GetTransformRectRelativeToWindow();
    if (isShowKeyboard) {
        CHECK_NULL_VOID(scrollableRect.Top() < bottomInset.start);
    }

    auto caretRect = textBase->GetCaretRect() + frameNode->GetOffsetRelativeToWindow();
    auto diffTop = caretRect.Top() - scrollableRect.Top();
    // caret height larger scroll's content region
    if (isShowKeyboard) {
        if (diffTop <= 0 &&
            LessNotEqual(bottomInset.start, (caretRect.Bottom() + RESERVE_BOTTOM_HEIGHT.ConvertToPx()))) {
            return;
        }
    }

    // caret above scroll's content region
    if (diffTop < 0) {
        scrollPattern->ScrollTo(scrollPattern->GetTotalOffset() + diffTop);
        return;
    }

    // caret inner scroll's content region
    if (isShowKeyboard) {
        if (LessNotEqual((caretRect.Bottom() + RESERVE_BOTTOM_HEIGHT.ConvertToPx()), bottomInset.start)) {
            return;
        }
    }

    // caret below safeArea
    float diffBot = 0.0f;
    if (isShowKeyboard) {
        if (LessNotEqual(scrollableRect.Bottom(), bottomInset.start)) {
            diffBot = scrollableRect.Bottom() - caretRect.Bottom() - RESERVE_BOTTOM_HEIGHT.ConvertToPx();
        } else {
            diffBot = bottomInset.start - caretRect.Bottom() - RESERVE_BOTTOM_HEIGHT.ConvertToPx();
        }
    } else {
        diffBot = scrollableRect.Bottom() - caretRect.Bottom() - RESERVE_BOTTOM_HEIGHT.ConvertToPx();
    }
    CHECK_NULL_VOID(diffBot < 0);
    scrollPattern->ScrollTo(scrollPattern->GetTotalOffset() - diffBot);
}

void TextFieldManagerNG::ScrollTextFieldToSafeArea()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto keyboardInset = pipeline->GetSafeAreaManager()->GetKeyboardInset();
    bool isShowKeyboard = keyboardInset.IsValid();
    if (isShowKeyboard) {
        auto bottomInset = pipeline->GetSafeArea().bottom_.Combine(keyboardInset);
        CHECK_NULL_VOID(bottomInset.IsValid());
        ScrollToSafeAreaHelper(bottomInset, isShowKeyboard);
    } else if (pipeline->GetSafeAreaManager()->KeyboardSafeAreaEnabled()) {
        // hide keyboard only scroll when keyboard avoid mode is resize
        ScrollToSafeAreaHelper({0, 0}, isShowKeyboard);
    }
}

void TextFieldManagerNG::SetHeight(float height)
{
    height_ = height + RESERVE_BOTTOM_HEIGHT.ConvertToPx();
}

void TextFieldManagerNG::UpdateScrollableParentViewPort(const RefPtr<FrameNode>& node)
{
    CHECK_NULL_VOID(node);
    auto scrollableNode = FindScrollableOfFocusedTextField(node);
    CHECK_NULL_VOID(scrollableNode);
    auto scrollPattern = scrollableNode->GetPattern<ScrollablePattern>();
    CHECK_NULL_VOID(scrollPattern);
    if (scrollPattern->GetAxis() == Axis::HORIZONTAL) {
        return;
    }
    auto scrollableRect = scrollableNode->GetTransformRectRelativeToWindow();
    scrollableNode->SetViewPort(scrollableRect);
}
void TextFieldManagerNG::ProcessNavKeyboard()
{
    if (imeShow_ || uiExtensionImeShow_) {
        TAG_LOGI(AceLogTag::ACE_KEYBOARD, "Nav notNeedSoftKeyboard.");
        FocusHub::NavCloseKeyboard();
    }
}
} // namespace OHOS::Ace::NG
