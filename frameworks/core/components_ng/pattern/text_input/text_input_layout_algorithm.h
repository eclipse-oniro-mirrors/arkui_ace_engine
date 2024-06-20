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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_INPUT_TEXT_INPUT_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_INPUT_TEXT_INPUT_PATTERN_H

#include "core/components_ng/pattern/text_field/text_field_layout_algorithm.h"
#include "core/components_ng/pattern/text_field/text_input_response_area.h"

namespace OHOS::Ace::NG {
class TextFieldPattern;
class TextInputLayoutAlgorithm : public TextFieldLayoutAlgorithm {
    DECLARE_ACE_TYPE(TextInputLayoutAlgorithm, TextFieldLayoutAlgorithm);

public:
    TextInputLayoutAlgorithm() = default;
    ~TextInputLayoutAlgorithm() override = default;

    void Measure(LayoutWrapper* layoutWrapper) override;
    void ResponseAreaMeasure(LayoutWrapper* layoutWrapper, OptionalSizeF& frameSize,
        LayoutConstraintF& textFieldContentConstraint, float contentWidth, float contentHeight);
    std::optional<SizeF> MeasureContent(
        const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper) override;
    void Layout(LayoutWrapper* layoutWrapper) override;
    float GetDefaultHeightByType(LayoutWrapper* layoutWrapper);
protected:
    bool CreateParagraphEx(const TextStyle& textStyle, const std::string& content,
        const LayoutConstraintF& contentConstraint, LayoutWrapper* layoutWrapper) override;
    LayoutConstraintF BuildLayoutConstraintWithoutResponseArea(const LayoutConstraintF& contentConstraint,
        LayoutWrapper* layoutWrapper);

private:
    struct UpdateContentPositionParams {
        bool isRTL;
        OffsetF offsetBase;
        SizeT<float> size;
        SizeT<float> contentSize;
        Alignment align;
        RefPtr<TextInputResponseArea> responseArea;
    };
    struct UpdateTextRectParams {
        RefPtr<TextFieldLayoutProperty> layoutProperty;
        RefPtr<TextFieldPattern> pattern;
        SizeT<float> contentSize;
        bool isRTL;
        RefPtr<TextInputResponseArea> responseArea;
        OffsetF offsetBase;
    };
    void UpdateContentPosition(const UpdateContentPositionParams& params,
        const std::unique_ptr<GeometryProperty>& content);
    void UpdateTextRect(const UpdateTextRectParams& params);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_INPUT_TEXT_INPUT_PATTERN_H