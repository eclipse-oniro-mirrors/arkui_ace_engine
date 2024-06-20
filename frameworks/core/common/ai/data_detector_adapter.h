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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_AI_PROPERTIES_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_AI_PROPERTIES_H

#include <set>

#include "interfaces/inner_api/ace/ai/data_detector_interface.h"

#include "base/memory/ace_type.h"
#include "base/thread/cancelable_callback.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/property/property.h"
#include "core/components_v2/inspector/utils.h"

namespace OHOS::AAFwk {
class Want;
class WantParams;
} // namespace OHOS::AAFwk

namespace OHOS::Ace {

namespace NG {
class TextPattern;
class RichEditorPattern;
}

struct AISpan {
    int32_t start = 0;
    int32_t end = 0;
    std::string content = "";
    TextDataDetectType type = TextDataDetectType::PHONE_NUMBER;
};
class DataDetectorAdapter : public AceType {
    DECLARE_ACE_TYPE(DataDetectorAdapter, AceType);

public:
    DataDetectorAdapter() = default;
    ~DataDetectorAdapter() override = default;

    RefPtr<NG::FrameNode> GetHost() const
    {
        return frameNode_.Upgrade();
    }
    void SetTextDetectResult(const TextDataDetectResult result)
    {
        textDetectResult_ = result;
    }
    void FireOnResult(const TextDataDetectResult& result)
    {
        if (onResult_) {
            auto resultJson = JsonUtil::Create(true);
            resultJson->Put("code", result.code);
            resultJson->Put("entity", result.entity.c_str());
            onResult_(resultJson->ToString());
        }
    }
    void InitTextDetect(int32_t startPos, std::string detectText);
    void SetTextDetectTypes(const std::string& types);
    void ParseAIResult(const TextDataDetectResult& result, int32_t startPos);
    void ParseAIJson(const std::unique_ptr<JsonValue>& jsonValue, TextDataDetectType type, int32_t startPos);
    void StartAITask();
    void CancelAITask()
    {
        if (aiDetectDelayTask_) {
            aiDetectDelayTask_.Cancel();
        }
        aiDetectInitialized_ = false;
    }
    bool ShowUIExtensionMenu(const AISpan& aiSpan, NG::RectF aiRect, const RefPtr<NG::FrameNode>& targetNode);
    void ResponseBestMatchItem(const AISpan& aiSpan);
    void StartAbilityByType(const std::string& type, AAFwk::WantParams& wantParams);

private:
    friend class NG::TextPattern;
    friend class NG::RichEditorPattern;

    std::function<void(const AAFwk::WantParams&)> GetOnReceive(
        NG::RectF aiRect, const RefPtr<NG::FrameNode>& targetNode);
    std::function<void()> GetDetectDelayTask(const std::map<int32_t, AISpan>& aiSpanMap);
    void SetWantParamaters(const AISpan& aiSpan, AAFwk::Want& want);

    RefPtr<NG::FrameNode> uiExtNode_;
    WeakPtr<NG::FrameNode> frameNode_;
    bool aiDetectInitialized_ = false;
    bool hasClickedAISpan_ = false;
    bool pressedByLeftMouse_ = false;
    bool typeChanged_ = false;
    AISpan clickedAISpan_;
    std::string textDetectTypes_;
    std::string textForAI_;
    std::string lastTextForAI_;
    std::set<std::string> textDetectTypesSet_;
    std::optional<TextDataDetectResult> textDetectResult_;
    std::function<void(const std::string&)> onResult_;
    std::function<void(const std::string&)> onClickMenu_;
    std::map<int32_t, AISpan> aiSpanMap_;
    CancelableCallback<void()> aiDetectDelayTask_;
    std::string uiExtensionBundleName_;
    std::string uiExtensionAbilityName_;
    std::unordered_map<int32_t, std::string> entityJson_;
    TimeStamp startDetectorTimeStamp_;
    std::vector<std::string> detectTexts;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMMON_AI_PROPERTIES_H
