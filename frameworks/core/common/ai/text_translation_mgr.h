/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_TEXT_TRANSLATION_MGR_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_TEXT_TRANSLATION_MGR_H

#include "text_translation_loader.h"

#include "base/utils/macros.h"

namespace OHOS::Ace {
class ACE_EXPORT TextTranslationMgr : public TextTranslationInterface {
public:
    static TextTranslationMgr& GetInstance();

    int32_t PullUpTranslationPanel(std::string translateText, std::string rectStr) override;

protected:
    ~TextTranslationMgr() = default;

private:
    TextTranslationMgr();
    TextTranslationInstance engine_ = nullptr;
    bool textTranslationIsSupported = false;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_TEXT_TRANSLATION_MGR_H