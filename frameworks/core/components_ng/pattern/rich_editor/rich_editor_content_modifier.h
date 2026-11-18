/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RICH_EDITOR_TEXT_CONTENT_MODIFIER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RICH_EDITOR_TEXT_CONTENT_MODIFIER_H

#include "core/components_ng/pattern/rich_editor/rich_editor_paragraph_manager.h"
#include "core/components_ng/pattern/text/text_content_modifier.h"
#include "core/components_ng/pattern/text_field/auto_fill_host.h"

namespace OHOS::Ace::NG {
class RichEditorContentModifier : public TextContentModifier, public IAutoFillContentModifier {
    DECLARE_ACE_TYPE(RichEditorContentModifier, TextContentModifier);

public:
    explicit RichEditorContentModifier(const std::optional<TextStyle>& textStyle,
        RichEditorParagraphManager* const pManager, const WeakPtr<OHOS::Ace::NG::Pattern>& pattern);

    ~RichEditorContentModifier() override = default;

    void onDraw(DrawingContext& drawingContext) override;
    void SetRichTextRectX(float value);
    void SetRichTextRectY(float value);
    float GetRichTextRectX() const;
    float GetRichTextRectY() const;
    void SetClipOffset(OffsetF offset);
    void SetClipSize(SizeF size);

    // IAutoFillContentModifier implementation
    void SetAutoFillTextScrollOffset(float value) override;
    void SetAutoFillTranslationOffset(float value) override;
    void SetAutoFillOriginTextColor(const Color& value) override;
    void SetAutoFillEmphasizeCharIndex(float value) override;
    void SetAutoFillDefaultCharIndex(float value) override;
    void ResetAutoFillProperties() override;
    void SetDefaultAutoFillProperties();

private:
    void PaintCustomSpan(DrawingContext& drawingContext);
    float AdjustParagraphX(const ParagraphManager::ParagraphInfo& info, const RectF& contentRect);
    void DoAutoFillDraw(DrawingContext& drawingContext);

    RichEditorParagraphManager* const pManager_;
    WeakPtr<OHOS::Ace::NG::Pattern> pattern_;
    RefPtr<PropertyFloat> richTextRectX_;
    RefPtr<PropertyFloat> richTextRectY_;
    RefPtr<AnimatablePropertyOffsetF> clipOffset_;
    RefPtr<AnimatablePropertySizeF> clipSize_;

    // AutoFill animation properties
    RefPtr<AnimatablePropertyFloat> autoFillTranslationOffset_;
    RefPtr<AnimatablePropertyFloat> autoFillTextScrollOffset_;
    RefPtr<AnimatablePropertyFloat> autoFillEmphasizeCharIndex_;
    RefPtr<AnimatablePropertyFloat> autoFillDefaultCharIndex_;
    std::optional<Color> autoFillOriginTextColor_;
    Color autoFillEmphasizeCharTextColor_;
    Color autoFillDefaultCharInitTextColor_;

    ACE_DISALLOW_COPY_AND_MOVE(RichEditorContentModifier);
};
} // namespace OHOS::Ace::NG

#endif