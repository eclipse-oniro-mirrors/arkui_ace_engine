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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RADIO_RADIO_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RADIO_RADIO_PATTERN_H

#include "base/memory/referenced.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/radio/radio_event_hub.h"
#include "core/components_ng/pattern/radio/radio_layout_algorithm.h"
#include "core/components_ng/pattern/radio/radio_paint_method.h"
#include "core/components_ng/pattern/radio/radio_paint_property.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

class RadioPattern : public Pattern {
    DECLARE_ACE_TYPE(RadioPattern, Pattern);

public:
    RadioPattern() = default;
    ~RadioPattern() override = default;

    bool IsAtomicNode() const override
    {
        return true;
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<RadioPaintProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<RadioLayoutAlgorithm>();
    }

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, nullptr);
        auto eventHub = host->GetEventHub<EventHub>();
        CHECK_NULL_RETURN(eventHub, nullptr);
        auto enabled = eventHub->IsEnabled();
        auto paintMethod =
            MakeRefPtr<RadioPaintMethod>(enabled, isTouch_, isHover_, totalScale_, pointScale_, uiStatus_);
        return paintMethod;
    }

    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& /*dirty*/, const DirtySwapConfig& /*config*/) override
    {
        return true;
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<RadioEventHub>();
    }

    const std::optional<std::string>& GetPreValue()
    {
        return preValue_;
    }

    const std::optional<std::string>& GetPreGroup()
    {
        return preGroup_;
    }

    void SetPreValue(const std::string& value)
    {
        preValue_ = value;
    }

    void SetPreGroup(const std::string& group)
    {
        preGroup_ = group;
    }

    void UpdateUncheckStatus(const RefPtr<FrameNode>& frameNode);

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, true, FocusStyleType::OUTER_BORDER };
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override
    {
        Pattern::ToJsonValue(json);
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto radioEventHub = host->GetEventHub<NG::RadioEventHub>();
        auto value = radioEventHub ? radioEventHub->GetValue() : "";
        auto group = radioEventHub ? radioEventHub->GetGroup() : "";
        auto resultJson = JsonUtil::Create(true);
        resultJson->Put("value", value.c_str());
        resultJson->Put("group", group.c_str());
        json->Put("value", resultJson->ToString().c_str());
    }

private:
    void OnAttachToFrameNode() override;
    void OnDetachFromFrameNode(FrameNode* frameNode) override;
    void OnModifyDone() override;
    void InitClickEvent();
    void InitTouchEvent();
    void InitMouseEvent();
    void OnClick();
    void UpdateState();
    void UpdateGroupCheckStatus(const RefPtr<FrameNode>& frameNode, bool check);
    void OnTouchDown();
    void OnTouchUp();
    void HandleMouseEvent(bool isHover);
    void PlayAnimation(bool isOn);
    void StopTranslateAnimation();
    void StopAnimation();
    void UpdateTotalScale(float scale);
    void UpdatePointScale(float scale);
    void UpdateUIStatus(bool check);
    RectF GetHotZoneRect(bool isOriginal) const;

    RefPtr<ClickEvent> clickListener_;
    RefPtr<TouchEventImpl> touchListener_;
    RefPtr<InputEvent> mouseEvent_;
    RefPtr<Animator> onController_;
    RefPtr<Animator> offController_;

    bool isFirstCreated_ = true;
    bool preCheck_ = false;
    std::optional<std::string> preValue_;
    std::optional<std::string> preGroup_;
    bool isTouch_ = false;
    bool isHover_ = false;
    float totalScale_ = 1.0f;
    float pointScale_ = 0.5f;
    UIStatus uiStatus_ = UIStatus::UNSELECTED;
    Dimension hotZoneHorizontalPadding_ = 11.0_vp;

    ACE_DISALLOW_COPY_AND_MOVE(RadioPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RADIO_RADIO_PATTERN_H
