/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_FORM_LINK_MODEL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_FORM_LINK_MODEL_H

#include <memory>
#include <mutex>

#include "core/components_ng/base/frame_node.h"

namespace OHOS::Ace {
class FormLinkModel {
public:
    static FormLinkModel* GetInstance();
    static FormLinkModel* GetStsInstance();
    virtual ~FormLinkModel() = default;

    virtual void Create(const std::string& action) = 0;
    virtual RefPtr<OHOS::Ace::NG::FrameNode> StsCreateFrameNode() = 0;
    virtual void StsSetAction(OHOS::Ace::NG::FrameNode* frameNode, const std::string& action) = 0;

private:
    static std::unique_ptr<FormLinkModel> instance_;
    static std::mutex mutex_;

    static std::unique_ptr<FormLinkModel> stsInstance_;
    static std::mutex stsMutex_;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_FORM_LINK_MODEL_H
