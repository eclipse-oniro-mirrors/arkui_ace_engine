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
#ifndef FOUNDATION_ARKUI_ACE_ENGINE_FRAMEWORKS_CORE_INTERFACES_ARKOALA_IMPL_SEARCH_CONTROLLER_PEER_IMPL_H
#define FOUNDATION_ARKUI_ACE_ENGINE_FRAMEWORKS_CORE_INTERFACES_ARKOALA_IMPL_SEARCH_CONTROLLER_PEER_IMPL_H

#include <optional>

#include "core/interfaces/native/utility/converter.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "core/components_ng/pattern/text_field/text_field_model.h"

namespace OHOS::Ace::NG::GeneratedModifier {
class SearchControllerPeerImpl : public Referenced {
public:
    SearchControllerPeerImpl() = default;

    const WeakPtr<TextFieldControllerBase>& GetController() const;
    void SetController(const RefPtr<TextFieldControllerBase>& controller);
    void TriggerCaretPosition(int32_t caretPosition);
    void TriggerStopEditing();
    void TriggerSetTextSelection(int32_t selectionStart, int32_t selectionEnd,
        const std::optional<SelectionOptions>& options = std::nullopt);

private:
    WeakPtr<TextFieldControllerBase> controllerWeak_;
};
} // namespace OHOS::Ace::NG::GeneratedModifier
#endif // FOUNDATION_ARKUI_ACE_ENGINE_FRAMEWORKS_CORE_INTERFACES_ARKOALA_IMPL_SEARCH_CONTROLLER_PEER_IMPL_H