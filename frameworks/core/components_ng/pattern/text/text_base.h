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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_TEXT_BASE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_TEXT_BASE_H

#include "base/memory/ace_type.h"
#include "core/components_ng/manager/select_overlay/selection_host.h"
#include "core/components_ng/pattern/text_field/text_selector.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/paragraph.h"

namespace OHOS::Ace::NG {
using ParagraphT = std::variant<std::shared_ptr<RSParagraph>, RefPtr<Paragraph>>;

enum class MouseStatus { PRESSED, RELEASED, MOVE, NONE };

class TextBase : public virtual SelectionHost {
    DECLARE_ACE_TYPE(TextBase, SelectionHost);

public:
    TextBase() = default;
    ~TextBase() override = default;

    virtual OffsetF GetContentOffset()
    {
        return OffsetF(0, 0);
    }

    virtual bool IsSelected() const
    {
        return textSelector_.IsValid() && !textSelector_.StartEqualToDest();
    }

    MouseStatus GetMouseStatus() const
    {
        return mouseStatus_;
    }

protected:
    TextSelector textSelector_;
    MouseStatus mouseStatus_ = MouseStatus::NONE;
    ACE_DISALLOW_COPY_AND_MOVE(TextBase);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_TEXT_BASE_H
