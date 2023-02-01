/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/option/option_layout_algorithm.h"

#include <string>

#include "base/utils/utils.h"
#include "core/components/select/select_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/option/option_paint_property.h"
#include "core/components_ng/pattern/option/option_theme.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {

void OptionLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    verInterval_ = VERTICAL_INTERVAL_PHONE.ConvertToPx();
    horInterval_ = HORIZONTAL_INTERVAL_PHONE.ConvertToPx();
    auto props = layoutWrapper->GetLayoutProperty();
    CHECK_NULL_VOID(props);
    auto layoutConstraint = props->GetLayoutConstraint();
    CHECK_NULL_VOID(layoutConstraint);

    float maxTextWidth = layoutConstraint->maxSize.Width() - horInterval_ * 2.0;

    // measure text
    auto childConstraint = props->CreateChildConstraint();
    childConstraint.maxSize.SetWidth(maxTextWidth);
    childConstraint.percentReference.SetWidth(maxTextWidth);
    auto text = layoutWrapper->GetOrCreateChildByIndex(0);
    CHECK_NULL_VOID(text);
    text->Measure(childConstraint);

    // set self size based on TextNode size;
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<SelectTheme>();
    CHECK_NULL_VOID(theme);
    auto minOptionHeight = theme->GetOptionMinHeight().ConvertToPx();

    auto textSize = text->GetGeometryNode()->GetFrameSize();
    SizeF size(
        textSize.Width() + horInterval_ * 2.0, std::max((textSize.Height() + verInterval_ * 2.0), minOptionHeight));
    LOGD("option frame size set to %{public}f x %{public}f", size.Width(), size.Height());
    layoutWrapper->GetGeometryNode()->SetFrameSize(size);
}

void OptionLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto text = layoutWrapper->GetOrCreateChildByIndex(0);
    text->GetGeometryNode()->SetMarginFrameOffset(OffsetF(horInterval_, verInterval_));
    text->Layout();
}

} // namespace OHOS::Ace::NG