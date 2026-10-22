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

#ifndef KOALA_PROJECTS_ARKOALA_ARKTS_ARKUI_OHOS_ANI_NATIVE_PLACEHOLDER_REGISTER_MODULE_H
#define KOALA_PROJECTS_ARKOALA_ARKTS_ARKUI_OHOS_ANI_NATIVE_PLACEHOLDER_REGISTER_MODULE_H

#include "ani.h"

namespace OHOS::Ace::Ani {
ani_int PlaceholderRegisterRegister(
    ani_env* env, ani_object aniClass, ani_string id, ani_object builder, ani_object execution);
void PlaceholderRegisterUnregister(ani_env* env, ani_object aniClass, ani_string id);
ani_boolean PlaceholderRegisterIsRegistered(ani_env* env, ani_object aniClass, ani_string id);
void PlaceholderRegisterPublishProduct(ani_env* env, ani_object aniClass, ani_long productPtr);
void PlaceholderBeginScrollPlaceholder(ani_env* env, ani_object aniClass);
void PlaceholderEndScrollPlaceholder(ani_env* env, ani_object aniClass);
} // namespace OHOS::Ace::Ani

#endif // KOALA_PROJECTS_ARKOALA_ARKTS_ARKUI_OHOS_ANI_NATIVE_PLACEHOLDER_REGISTER_MODULE_H
