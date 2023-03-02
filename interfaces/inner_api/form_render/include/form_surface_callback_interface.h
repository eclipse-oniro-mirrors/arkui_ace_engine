/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef FOUNDATION_ACE_INTERFACE_INNERKITS_FORM_SURFACE_CALLBACK_INTERFACE_H
#define FOUNDATION_ACE_INTERFACE_INNERKITS_FORM_SURFACE_CALLBACK_INTERFACE_H

#include "base/utils/macros.h"
#include "form_js_info.h"
#include "ui/rs_surface_node.h"
#include "want.h"

namespace OHOS {
namespace Ace {
class ACE_EXPORT FormSurfaceCallbackInterface {
public:
    /**
     * @brief Update form.
     *
     * @param formJsInfo Indicates the obtained {@code FormJsInfo} instance.
     */
    virtual void ProcessAddFormSurface(
        const AppExecFwk::FormJsInfo& formInfo,
        const std::shared_ptr<Rosen::RSSurfaceNode>& rsSurfaceNode,
        const AAFwk::Want& want) = 0;
};
} // namespace Ace
} // namespace OHOS
#endif // FOUNDATION_ACE_INTERFACE_INNERKITS_FORM_SURFACE_CALLBACK_INTERFACE_H
