/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "stateMgmt_ani_modifier.h"
#include "base/log/log.h"
#include "base/memory/referenced.h"
#include "base/i18n/localization.h"
#include "core/common/ace_engine.h"
#include "core/common/ace_application_info.h"
#include "core/common/container.h"
#include "core/common/storage/storage_proxy.h"
#include "core/common/environment/environment_proxy.h"
#include "core/pipeline/pipeline_base.h"

#include <vector>
#include <memory>

namespace OHOS::Ace::NG {

std::string PersistentStorageGet(std::string key)
{
#if defined(PREVIEW)
    LOGW("[Engine Log] Unable to use the PersistentStorage in the Previewer. Perform this operation on the "
        "emulator or a real device instead.");
    return "";
#endif
    auto storage = StorageProxy::GetInstance()->GetStorage();
    if (!storage) {
        LOGW("no storage available");
        return "";
    }
    std::string value = storage->GetString(key);
    LOGI("PersistentStorageGet:%s", key.c_str());
    if (value.empty() || value == "undefined") {
        LOGE("value is empty or undefined");
        return "";
    }
    return value;
}

void PersistentStorageSet(std::string key, std::string value)
{
#if defined(PREVIEW)
    LOGW("[Engine Log] Unable to use the PersistentStorage in the Previewer. Perform this operation on the "
        "emulator or a real device instead.");
    return;
#endif
    if (!StorageProxy::GetInstance()->GetStorage()) {
        LOGW("no storage available");
        return;
    }
    LOGI("PersistentStorageSet:%s value:%s", key.c_str(), value.c_str());
    StorageProxy::GetInstance()->GetStorage()->SetString(key, value);
}

bool PersistentStorageHas(std::string key)
{
#if defined(PREVIEW)
    LOGW("[Engine Log] Unable to use the PersistentStorage in the Previewer. Perform this operation on the "
        "emulator or a real device instead.");
    return false;
#endif
    if (!StorageProxy::GetInstance()->GetStorage()) {
        LOGW("no storage available");
        return false;
    }
    std::string value = StorageProxy::GetInstance()->GetStorage()->GetString(key);
    LOGI("PersistentStorageHas:key:%s, value:%s", key.c_str(), value.c_str());
    if (!value.empty()) {
        return true;
    }
    return false;
}

void PersistentStorageDelete(std::string key)
{
#if defined(PREVIEW)
    LOGW("[Engine Log] Unable to use the PersistentStorage in the Previewer. Perform this operation on the "
        "emulator or a real device instead.");
    return;
#endif
    if (!StorageProxy::GetInstance()->GetStorage()) {
        LOGW("no storage available");
        return;
    }
    StorageProxy::GetInstance()->GetStorage()->Delete(key);
    LOGI("PersistentStorageDelete:%s", key.c_str());
}

void PersistentStorageClear()
{
#if defined(PREVIEW)
    LOGW("[Engine Log] Unable to use the PersistentStorage in the Previewer. Perform this operation on the "
        "emulator or a real device instead.");
    return;
#endif
    if (!StorageProxy::GetInstance()->GetStorage()) {
        LOGW("no storage available");
        return;
    }
    StorageProxy::GetInstance()->GetStorage()->Clear();
    LOGI("PersistentStorageClear");
}

int32_t GetColorMode()
{
    auto colorMode = Container::CurrentColorMode();
    LOGI("GetColorMode colorMode:%d", colorMode);
    return static_cast<int32_t>(colorMode);
}

float GetFontScale()
{
    auto container = Container::Current();
    if (!container) {
        LOGW("container is null");
        return -1;
    }
    auto context = container->GetPipelineContext();
    if (!context) {
        LOGW("context is null");
        return -1;
    }
    auto returnValue = context->GetFontScale();
    LOGI("GetFontScale returnValue:%.2f", returnValue);
    return returnValue;
}

float GetFontWeightScale()
{
    auto weightScale = SystemProperties::GetFontWeightScale();
    LOGI("GetFontWeightScale weightScale:%.2f", weightScale);
    return weightScale;
}

bool GetAccessibilityEnabled()
{
    std::string value;
#if defined(PREVIEW)
    value = "false";
#else
    auto container = Container::Current();
    if (!container) {
        LOGW("container is null");
        return false;
    }
    auto executor = container->GetTaskExecutor();
    value = EnvironmentProxy::GetInstance()->GetEnvironment(executor)->GetAccessibilityEnabled();
#endif
    LOGI("GetAccessibilityEnabled value:%s", value.c_str());
    if (value == "true") {
        return true;
    }
    return  false;
}

std::string GetLayoutDirection()
{
    auto isRTL = AceApplicationInfo::GetInstance().IsRightToLeft();
    auto value = isRTL ? 0 : 1;
    auto strValue = std::to_string(value);
    LOGI("GetLayoutDirection strValue:%s", strValue.c_str());
    return strValue;
}

std::string GetLanguageCode()
{
    auto container = Container::Current();
    if (!container) {
        LOGW("Initialize is not complete, cannot get the language");
        return "";
    }
    auto location = Localization::GetInstance();
    auto language = location->GetLanguage();
    LOGI("GetLanguageCode language:%s", language.c_str());
    return language;
}

const ArkUIAniStateMgmtModifier* GetStateMgmtAniModifier()
{
    static const ArkUIAniStateMgmtModifier impl = {
        .persistentStorageGet = OHOS::Ace::NG::PersistentStorageGet,
        .persistentStorageSet = OHOS::Ace::NG::PersistentStorageSet,
        .persistentStorageHas = OHOS::Ace::NG::PersistentStorageHas,
        .persistentStorageDelete = OHOS::Ace::NG::PersistentStorageDelete,
        .persistentStorageClear = OHOS::Ace::NG::PersistentStorageClear,
        .getColorMode = OHOS::Ace::NG::GetColorMode,
        .getFontWeightScale = OHOS::Ace::NG::GetFontWeightScale,
        .getFontScale = OHOS::Ace::NG::GetFontScale,
        .getAccessibilityEnabled = OHOS::Ace::NG::GetAccessibilityEnabled,
        .getLayoutDirection = OHOS::Ace::NG::GetLayoutDirection,
        .getLanguageCode = OHOS::Ace::NG::GetLanguageCode
    };
    return &impl;
}

} // namespace OHOS::Ace::NG