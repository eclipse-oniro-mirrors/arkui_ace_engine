/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef ARKOALA_ARKTS_ARKUI_OHOS_SRC_ANI_NATIVE_MODAL_UI_EXTENSION_MODAL_UI_EXTENSION_MODULE_H
#define ARKOALA_ARKTS_ARKUI_OHOS_SRC_ANI_NATIVE_MODAL_UI_EXTENSION_MODAL_UI_EXTENSION_MODULE_H

#include "ani.h"

namespace OHOS::Ace::Ani {
ani_int CreateModalUIExtension(ani_env* env, [[maybe_unused]] ani_object obj, ani_object want, ani_object options, ani_object callback);
void CloseModalUIExtension(ani_env* env, [[maybe_unused]] ani_object obj, ani_int sessionId);
void ModalUIExtensionProxySend(ani_env* env, [[maybe_unused]] ani_object obj, ani_long proxyPtr, ani_object data);
} // namespace OHOS::Ace::Ani

#endif // ARKOALA_ARKTS_ARKUI_OHOS_SRC_ANI_NATIVE_MODAL_UI_EXTENSION_MODAL_UI_EXTENSION_MODULE_H
