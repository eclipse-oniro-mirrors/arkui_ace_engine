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

#include "core/components_ng/pattern/container_picker/container_picker_model.h"

#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/container_picker/container_picker_event_hub.h"
#include "core/components_ng/pattern/container_picker/container_picker_pattern.h"
#include "core/common/resource/resource_object.h"

namespace OHOS::Ace::NG {

void ContainerPickerModel::Create()
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", V2::CONTAINER_PICKER_ETS_TAG, nodeId);
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(V2::CONTAINER_PICKER_ETS_TAG, nodeId, []() {
            return AceType::MakeRefPtr<ContainerPickerPattern>();
        });
    stack->Push(frameNode);
}

void ContainerPickerModel::SetCanLoop(bool isLoop)
{
    ACE_UPDATE_LAYOUT_PROPERTY(ContainerPickerLayoutProperty, CanLoop, isLoop);
}

void ContainerPickerModel::SetEnableHapticFeedback(bool isEnableHapticFeedback)
{
    ACE_UPDATE_LAYOUT_PROPERTY(ContainerPickerLayoutProperty, EnableHapticFeedback, isEnableHapticFeedback);
}

void ContainerPickerModel::SetSelectedIndex(int32_t index)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pickerPattern = frameNode->GetPattern<ContainerPickerPattern>();
    CHECK_NULL_VOID(pickerPattern);
    auto layoutProperty = frameNode->GetLayoutProperty<ContainerPickerLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    if (layoutProperty->HasSelectedIndex()) {
        pickerPattern->SetTargetIndex(index);
    } else {
        pickerPattern->SetSelectedIndex(index);
    }
    
    ACE_UPDATE_LAYOUT_PROPERTY(ContainerPickerLayoutProperty, SelectedIndex, index);
}

void ContainerPickerModel::SetOnChange(ContainerPickerChangeEvent&& onChange)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<ContainerPickerEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnChange(std::move(onChange));
}

void ContainerPickerModel::SetOnScrollStop(ContainerPickerChangeEvent&& onScrollStop)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<ContainerPickerEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnScrollStop(std::move(onScrollStop));
}

void ContainerPickerModel::SetIndicatorStyle(const PickerIndicatorStyle& style)
{
    ACE_UPDATE_LAYOUT_PROPERTY(ContainerPickerLayoutProperty, IndicatorType, style.type);
    if (style.type == static_cast<int32_t>(IndicatorType::DIVIDER)) {
        if (style.dividerWidth.has_value()) {
            ACE_UPDATE_LAYOUT_PROPERTY(ContainerPickerLayoutProperty, IndicatorDividerWidth,
                style.dividerWidth.value());
        }
        if (style.dividerColor.has_value()) {
            ACE_UPDATE_LAYOUT_PROPERTY(ContainerPickerLayoutProperty, IndicatorDividerColor,
                style.dividerColor.value());
        }
        if (style.startMargin.has_value()) {
            ACE_UPDATE_LAYOUT_PROPERTY(ContainerPickerLayoutProperty, IndicatorStartMargin,
                style.startMargin.value());
        }
        if (style.endMargin.has_value()) {
            ACE_UPDATE_LAYOUT_PROPERTY(ContainerPickerLayoutProperty, IndicatorEndMargin,
                style.endMargin.value());
        }
    } else if (style.type == static_cast<int32_t>(IndicatorType::BACKGROUND)) {
        if (style.backgroundColor.has_value()) {
            ACE_UPDATE_LAYOUT_PROPERTY(ContainerPickerLayoutProperty, IndicatorBackgroundColor,
                style.backgroundColor.value());
        }
        if (style.borderRadius.has_value()) {
            ACE_UPDATE_LAYOUT_PROPERTY(ContainerPickerLayoutProperty, IndicatorBorderRadius,
                style.borderRadius.value());
        }
    } else {
        TAG_LOGE(AceLogTag::ACE_CONTAINER_PICKER, "invalid type of PickerIndicatorStyle.");
        return;
    }

    auto frameNode = ViewStackProcessor::GetInstace()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pickerPattern = frameNode->GetPattern<ContainerPickerPattern>();
    CHECK_NULL_VOID(pickerPattern);
    pickerPattern->SetIndicatorStyleVal(style);
}

void ContainerPickerModel::SetCanLoop(FrameNode* frameNode, bool isLoop)
{
    CHECK_NULL_VOID(frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(ContainerPickerLayoutProperty, CanLoop, isLoop, frameNode);
}

void ContainerPickerModel::SetEnableHapticFeedback(FrameNode* frameNode, bool isEnableHapticFeedback)
{
    CHECK_NULL_VOID(frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(
        ContainerPickerLayoutProperty, EnableHapticFeedback, isEnableHapticFeedback, frameNode);
}

void ContainerPickerModel::SetOnChange(FrameNode* frameNode, ContainerPickerChangeEvent&& onChange)
{
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<ContainerPickerEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnChange(std::move(onChange));
}

void ContainerPickerModel::SetOnScrollStop(FrameNode* frameNode, ContainerPickerChangeEvent&& onScrollStop)
{
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<ContainerPickerEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnScrollStop(std::move(onScrollStop));
}

void ContainerPickerModel::SetIndicatorStyle(FrameNode* frameNode, const PickerIndicatorStyle& style)
{
    CHECK_NULL_VOID(frameNode);
    ACE_UPDATE_NODE_LAYOUT_PROPERTY(ContainerPickerLayoutProperty, IndicatorType, style.type, frameNode);
    if (style.type == static_cast<int32_t>(IndicatorType::DIVIDER)) {
        if (style.dividerWidth.has_value()) {
            ACE_UPDATE_NODE_LAYOUT_PROPERTY(
                ContainerPickerLayoutProperty, IndicatorDividerWidth, style.dividerWidth.value(), frameNode);
        }
        if (style.dividerColor.has_value()) {
            ACE_UPDATE_NODE_LAYOUT_PROPERTY(
                ContainerPickerLayoutProperty, IndicatorDividerColor, style.dividerColor.value(), frameNode);
        }
        if (style.startMargin.has_value()) {
            ACE_UPDATE_NODE_LAYOUT_PROPERTY(
                ContainerPickerLayoutProperty, IndicatorStartMargin, style.startMargin.value(), frameNode);
        }
        if (style.endMargin.has_value()) {
            ACE_UPDATE_NODE_LAYOUT_PROPERTY(
                ContainerPickerLayoutProperty, IndicatorEndMargin, style.endMargin.value(), frameNode);
        }
    } else if (style.type == static_cast<int32_t>(IndicatorType::BACKGROUND)) {
        if (style.backgroundColor.has_value()) {
            ACE_UPDATE_NODE_LAYOUT_PROPERTY(
                ContainerPickerLayoutProperty, IndicatorBackgroundColor, style.backgroundColor.value(), frameNode);
        }
        if (style.borderRadius.has_value()) {
            ACE_UPDATE_NODE_LAYOUT_PROPERTY(
                ContainerPickerLayoutProperty, IndicatorBorderRadius, style.borderRadius.value(), frameNode);
        }
    } else {
        TAG_LOGE(AceLogTag::ACE_CONTAINER_PICKER, "invalid type of PickerIndicatorStyle.");
        return;
    }
    auto pickerPattern = frameNode->GetPattern<ContainerPickerPattern>();
    CHECK_NULL_VOID(pickerPattern);
    pickerPattern->SetIndicatorStyleVal(style);
}

void ContainerPickerModel::ProcessResourceObj(const std::string& key, const RefPtr<ResourceObject>& resObj)
{
    CHECK_NULL_VOID(SystemProperties::ConfigChangePerform());
    auto frameNode = ViewStackProcessor::GetInstace()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto pickerPattern = frameNode->GetPattern<ContainerPickerPattern>();
    CHECK_NULL_VOID(pickerPattern);
    CHECK_NULL_VOID(resObj);

    using handler = sd::function<void(const RefPtr<ResourceObject>&)>;
    const std::unordered_map<std::string, handler> handlers = {
        {"containerPicker.dividerWidth", [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& resObj) {
                auto pickerNode = weak.Upgrade();
                CHECK_NULL_VOID(pickerNode);
                auto pickerPattern = pickerNode->GetPattern<ContainerPickerPattern>();
                CHECK_NULL_VOID(pickerPattern);
                pickerPattern->UpdateDividerWidthWithResObj(resObj);
            }
        },
        {"containerPicker.dividerColor", [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& resObj) {
                auto pickerNode = weak.Upgrade();
                CHECK_NULL_VOID(pickerNode);
                auto pickerPattern = pickerNode->GetPattern<ContainerPickerPattern>();
                CHECK_NULL_VOID(pickerPattern);
                pickerPattern->UpdateDividerColorWithResObj(resObj);
            }
        },
        {"containerPicker.startMargin", [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& resObj) {
                auto pickerNode = weak.Upgrade();
                CHECK_NULL_VOID(pickerNode);
                auto pickerPattern = pickerNode->GetPattern<ContainerPickerPattern>();
                CHECK_NULL_VOID(pickerPattern);
                pickerPattern->UpdateStartMarginWithResObj(resObj);
            }
        },
        {"containerPicker.endMargin", [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& resObj) {
                auto pickerNode = weak.Upgrade();
                CHECK_NULL_VOID(pickerNode);
                auto pickerPattern = pickerNode->GetPattern<ContainerPickerPattern>();
                CHECK_NULL_VOID(pickerPattern);
                pickerPattern->UpdateEndMarginWithResObj(resObj);
            }
        },
        {"containerPicker.backgroundColor", [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& resObj) {
                auto pickerNode = weak.Upgrade();
                CHECK_NULL_VOID(pickerNode);
                auto pickerPattern = pickerNode->GetPattern<ContainerPickerPattern>();
                CHECK_NULL_VOID(pickerPattern);
                pickerPattern->UpdateBackgroundColorWithResObj(resObj);
            }
        },
        {"containerPicker.borderRadius", [weak = AceType::WeakClaim(frameNode)](const RefPtr<ResourceObject>& resObj) {
                auto pickerNode = weak.Upgrade();
                CHECK_NULL_VOID(pickerNode);
                auto pickerPattern = pickerNode->GetPattern<ContainerPickerPattern>();
                CHECK_NULL_VOID(pickerPattern);
                pickerPattern->UpdateBorderRadiusWithResObj(resObj);
            }
        }
    };

    auto it = handlers.find(key);
    if (it != handlers.end()) {
        auto updateFunc = handlers.at(key);
        pickerPattern->AddResObj(key, resObj, std::move(updateFunc));
    }
}

} // namespace OHOS::Ace::NG
