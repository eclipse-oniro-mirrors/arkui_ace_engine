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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CHECKBOX_CHECKBOX_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CHECKBOX_CHECKBOX_PATTERN_H

#include "base/geometry/axis.h"
#include "base/memory/referenced.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/pattern/checkbox/checkbox_event_hub.h"
#include "core/components_ng/pattern/checkbox/checkbox_layout_algorithm.h"
#include "core/components_ng/pattern/checkbox/checkbox_paint_method.h"
#include "core/components_ng/pattern/checkbox/checkbox_paint_property.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

class CheckBoxPattern : public Pattern {
    DECLARE_ACE_TYPE(CheckBoxPattern, Pattern);

public:
    CheckBoxPattern() = default;
    ~CheckBoxPattern() override = default;

    bool IsAtomicNode() const override
    {
        return true;
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<CheckBoxPaintProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<CheckBoxLayoutAlgorithm>();
    }

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        auto host = GetHost();
        CHECK_NULL_RETURN(host, nullptr);
        auto eventHub = host->GetEventHub<EventHub>();
        CHECK_NULL_RETURN(eventHub, nullptr);
        auto enabled = eventHub->IsEnabled();
        auto paintMethod = MakeRefPtr<CheckBoxPaintMethod>(enabled, isTouch_, isHover_, shapeScale_, uiStatus_);
        return paintMethod;
    }

    bool OnDirtyLayoutWrapperSwap(
        const RefPtr<LayoutWrapper>& /*dirty*/, bool /*skipMeasure*/, bool /*skipLayout*/) override
    {
        return true;
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<CheckBoxEventHub>();
    }

    const std::optional<std::string>& GetPreName()
    {
        return preName_;
    }

    const std::optional<std::string>& GetPreGroup()
    {
        return preGroup_;
    }

    void SetPreName(const std::string& name)
    {
        preName_ = name;
    }

    void SetPreGroup(const std::string& group)
    {
        preGroup_ = group;
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, true, FocusStyleType::OUTER_BORDER };
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override
    {
        Pattern::ToJsonValue(json);
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto checkBoxEventHub = host->GetEventHub<NG::CheckBoxEventHub>();
        auto name = checkBoxEventHub ? checkBoxEventHub->GetName() : "";
        auto group = checkBoxEventHub ? checkBoxEventHub->GetGroupName() : "";
        json->Put("name", name.c_str());
        json->Put("group", group.c_str());
        json->Put("type", "ToggleType.Checkbox");
    }

    void UpdateAnimation(bool check);
    void UpdateUIStatus(bool check);

private:
    void OnAttachToFrameNode() override;
    void OnDetachFromFrameNode(FrameNode* frameNode) override;
    void OnModifyDone() override;
    void InitClickEvent();
    void InitTouchEvent();
    void InitMouseEvent();
    void OnClick();
    void OnTouchDown();
    void OnTouchUp();
    void HandleMouseEvent(bool isHover);
    void UpdateCheckBoxShape(float value);
    void UpdateState();
    void UpdateUnSelect();
    void UpdateCheckBoxGroupStatus(const RefPtr<FrameNode>& frameNode,
        std::unordered_map<std::string, std::list<WeakPtr<FrameNode>>>& checkBoxGroupMap, bool isSelected);
    void CheckBoxGroupIsTrue();

    std::optional<std::string> preName_;
    std::optional<std::string> preGroup_;

    RefPtr<ClickEvent> clickListener_;
    RefPtr<TouchEventImpl> touchListener_;
    RefPtr<InputEvent> mouseEvent_;
    bool isTouch_ = false;
    bool isHover_ = false;
    bool isFirstCreated_ = true;
    // animation control
    RefPtr<Animator> controller_;
    RefPtr<CurveAnimation<float>> translate_;
    float shapeScale_ = 1.0f;
    UIStatus uiStatus_ = UIStatus::UNSELECTED;

    ACE_DISALLOW_COPY_AND_MOVE(CheckBoxPattern);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_CHECKBOX_CHECKBOX_PATTERN_H
