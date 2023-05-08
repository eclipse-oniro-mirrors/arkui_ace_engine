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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_TEXT_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_TEXT_LAYOUT_ALGORITHM_H

#include <optional>
#include <string>
#include <utility>

#include "core/components_ng/layout/box_layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/text/span_node.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_styles.h"

namespace OHOS::Ace::NG {
class PipelineContext;
class TextContentModifier;

// TextLayoutAlgorithm acts as the underlying text layout.
class ACE_EXPORT TextLayoutAlgorithm : public BoxLayoutAlgorithm {
    DECLARE_ACE_TYPE(TextLayoutAlgorithm, BoxLayoutAlgorithm);

public:
    TextLayoutAlgorithm();

    TextLayoutAlgorithm(std::list<RefPtr<SpanItem>> spanItemChildren, const RefPtr<Paragraph>& paragraph)
        : spanItemChildren_(std::move(spanItemChildren)), paragraph_(paragraph)
    {}

    ~TextLayoutAlgorithm() override = default;

    void OnReset() override;

    void Measure(FrameNode* frameNode) override;

    std::optional<SizeF> MeasureContent(
        const LayoutConstraintF& contentConstraint, FrameNode* frameNode) override;

    void Layout(FrameNode* frameNode) override;

    const RefPtr<Paragraph>& GetParagraph();

    std::list<RefPtr<SpanItem>>&& GetSpanItemChildren();

    float GetBaselineOffset() const;

    std::optional<TextStyle> GetTextStyle() const;
    void ApplyIndents(const TextStyle& textStyle, double width);

private:
    bool CreateParagraph(const TextStyle& textStyle, std::string content, FrameNode* frameNode);
    bool CreateParagraphAndLayout(const TextStyle& textStyle, const std::string& content,
        const LayoutConstraintF& contentConstraint, FrameNode* frameNode);
    bool AdaptMinTextSize(TextStyle& textStyle, const std::string& content, const LayoutConstraintF& contentConstraint,
        const RefPtr<PipelineContext>& pipeline, FrameNode* frameNode);
    bool DidExceedMaxLines(const SizeF& maxSize);
    static TextDirection GetTextDirection(const std::string& content);
    float GetTextWidth() const;
    SizeF GetMaxMeasureSize(const LayoutConstraintF& contentConstraint) const;
    bool BuildParagraph(TextStyle& textStyle, const RefPtr<TextLayoutProperty>& layoutProperty,
        const LayoutConstraintF& contentConstraint, const RefPtr<PipelineContext>& pipeline,
        FrameNode* frameNode);
    bool BuildParagraphAdaptUseMinFontSize(TextStyle& textStyle, const RefPtr<TextLayoutProperty>& layoutProperty,
        const LayoutConstraintF& contentConstraint, const RefPtr<PipelineContext>& pipeline,
        FrameNode* frameNode);
    bool BuildParagraphAdaptUseLayoutConstraint(TextStyle& textStyle, const RefPtr<TextLayoutProperty>& layoutProperty,
        const LayoutConstraintF& contentConstraint, const RefPtr<PipelineContext>& pipeline,
        FrameNode* frameNode);
    std::optional<SizeF> BuildTextRaceParagraph(TextStyle& textStyle, const RefPtr<TextLayoutProperty>& layoutProperty,
        const LayoutConstraintF& contentConstraint, const RefPtr<PipelineContext>& pipeline,
        FrameNode* frameNode);
    void SetPropertyToModifier(const RefPtr<TextLayoutProperty>& layoutProperty, RefPtr<TextContentModifier> modifier);
    bool AdaptMaxTextSize(TextStyle& textStyle, const std::string& content, const LayoutConstraintF& contentConstraint,
        const RefPtr<PipelineContext>& pipeline, FrameNode* frameNode);
    void UpdateTextColorIfForeground(const RefPtr<FrameNode>& frameNode, TextStyle& textStyle);
    void UpdateParagraph(FrameNode* frameNode);
    OffsetF GetContentOffset(FrameNode* frameNode) const;

    std::list<RefPtr<SpanItem>> spanItemChildren_;
    RefPtr<Paragraph> paragraph_;
    float baselineOffset_ = 0.0f;
    std::optional<TextStyle> textStyle_;

    ACE_DISALLOW_COPY_AND_MOVE(TextLayoutAlgorithm);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_TEXT_LAYOUT_ALGORITHM_H
