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

#include "core/pipeline_ng/test/mock/mock_pipeline_base.h"

#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace {
class Window : public AceType {
    DECLARE_ACE_TYPE(Window, AceType);
};
class FontManager : public AceType {
    DECLARE_ACE_TYPE(FontManager, AceType);
};
class ManagerInterface : public AceType {
    DECLARE_ACE_TYPE(ManagerInterface, AceType);
};
class Frontend : public AceType {
    DECLARE_ACE_TYPE(Frontend, AceType);
};
class OffscreenCanvas : public AceType {
    DECLARE_ACE_TYPE(Frontend, AceType);
};
enum class FrontendType {};

RefPtr<PipelineBase> PipelineBase::GetCurrentContext()
{
    return AceType::MakeRefPtr<MockPipelineBase>();
}

double PipelineBase::NormalizeToPx(const Dimension& /* dimension */) const
{
    return 1.0f;
}

PipelineBase::~PipelineBase() = default;

uint64_t PipelineBase::GetTimeFromExternalTimer()
{
    return 1;
}

void PipelineBase::OnVsyncEvent(uint64_t nanoTimestamp, uint32_t frameCount) {}

void PipelineBase::SendEventToAccessibility(const AccessibilityEvent& accessibilityEvent) {}

void PipelineBase::OnActionEvent(const std::string& action) {}

RefPtr<AccessibilityManager> PipelineBase::GetAccessibilityManager() const
{
    return nullptr;
}
} // namespace OHOS::Ace