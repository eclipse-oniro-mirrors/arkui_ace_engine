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

#include "placeholder_register/placeholder_register_module.h"

#include "base/utils/utils.h"
#include "load.h"
#include "utils/ani_utils.h"

namespace OHOS::Ace::Ani {
namespace {

// Northbound result code when the engine modifiers table is unavailable or
// older than the placeholder entry: same contract as the engine's generic
// registration failure, so the module side raises the documented BusinessError.
constexpr ani_int REGISTER_MODIFIER_UNAVAILABLE = -1;

const ArkUIAniPlaceholderRegisterModifier* GetPlaceholderRegisterModifier()
{
    const auto* modifiers = GetNodeAniModifier();
    CHECK_NULL_RETURN(modifiers, nullptr);
    // The placeholder getter sits at the tail of ArkUIAniModifiers: an older
    // engine without the entry must read as unavailable, not as a stale
    // pointer read past the end of its struct.
    if (modifiers->version < ARKUI_ANI_API_VERSION) {
        return nullptr;
    }
    CHECK_NULL_RETURN(modifiers->getPlaceholderRegisterAniModifier, nullptr);
    return modifiers->getPlaceholderRegisterAniModifier();
}

} // namespace

ani_int PlaceholderRegisterRegister(ani_env* env, [[maybe_unused]] ani_object aniClass, ani_string id,
    ani_object builder, ani_object execution)
{
    const auto* placeholderModifier = GetPlaceholderRegisterModifier();
    CHECK_NULL_RETURN(placeholderModifier, REGISTER_MODIFIER_UNAVAILABLE);
    CHECK_NULL_RETURN(placeholderModifier->registerTemplate, REGISTER_MODIFIER_UNAVAILABLE);
    return placeholderModifier->registerTemplate(env, AniUtils::ANIStringToStdString(env, id), builder, execution);
}

void PlaceholderRegisterUnregister(ani_env* env, [[maybe_unused]] ani_object aniClass, ani_string id)
{
    const auto* placeholderModifier = GetPlaceholderRegisterModifier();
    CHECK_NULL_VOID(placeholderModifier);
    CHECK_NULL_VOID(placeholderModifier->unregisterTemplate);
    placeholderModifier->unregisterTemplate(AniUtils::ANIStringToStdString(env, id));
}

ani_boolean PlaceholderRegisterIsRegistered(ani_env* env, [[maybe_unused]] ani_object aniClass, ani_string id)
{
    const auto* placeholderModifier = GetPlaceholderRegisterModifier();
    CHECK_NULL_RETURN(placeholderModifier, ANI_FALSE);
    CHECK_NULL_RETURN(placeholderModifier->isTemplateRegistered, ANI_FALSE);
    return placeholderModifier->isTemplateRegistered(AniUtils::ANIStringToStdString(env, id));
}

void PlaceholderRegisterPublishProduct(
    [[maybe_unused]] ani_env* env, [[maybe_unused]] ani_object aniClass, ani_long productPtr)
{
    const auto* placeholderModifier = GetPlaceholderRegisterModifier();
    CHECK_NULL_VOID(placeholderModifier);
    CHECK_NULL_VOID(placeholderModifier->publishProduct);
    placeholderModifier->publishProduct(productPtr);
}

void PlaceholderBeginScrollPlaceholder([[maybe_unused]] ani_env* env, [[maybe_unused]] ani_object aniClass)
{
    const auto* placeholderModifier = GetPlaceholderRegisterModifier();
    CHECK_NULL_VOID(placeholderModifier);
    CHECK_NULL_VOID(placeholderModifier->beginScrollPlaceholder);
    placeholderModifier->beginScrollPlaceholder();
}

void PlaceholderEndScrollPlaceholder([[maybe_unused]] ani_env* env, [[maybe_unused]] ani_object aniClass)
{
    const auto* placeholderModifier = GetPlaceholderRegisterModifier();
    CHECK_NULL_VOID(placeholderModifier);
    CHECK_NULL_VOID(placeholderModifier->endScrollPlaceholder);
    placeholderModifier->endScrollPlaceholder();
}

} // namespace OHOS::Ace::Ani
