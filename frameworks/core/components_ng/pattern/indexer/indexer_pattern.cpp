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

#include "core/components_ng/pattern/indexer/indexer_pattern.h"

#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/indexer/indexer_theme.h"
#include "core/components_ng/pattern/list/list_item_pattern.h"
#include "core/components_ng/pattern/text/text_layout_property.h"
#include "core/components_ng/pattern/text/text_model.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

void IndexerPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);

    auto layoutProperty = host->GetLayoutProperty<IndexerLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    if (layoutProperty->GetArrayValue().has_value()) {
        arrayValue_ = layoutProperty->GetArrayValue().value();
        itemCount_ = static_cast<int32_t>(arrayValue_.size());
    }

    if (layoutProperty->GetSelected().has_value() && storeSelected_ != layoutProperty->GetSelected().value()) {
        storeSelected_ = layoutProperty->GetSelected().value();
        selected_ = storeSelected_;
        if (storeSelected_ >= itemCount_) {
            storeSelected_ = 0;
            selected_ = 0;
        }
        ApplyIndexChanged();
    }

    auto gesture = host->GetOrCreateGestureEventHub();
    if (gesture) {
        InitPanEvent(gesture);
    }

    if (!touchListener_) {
        auto touchCallback = [weak = WeakClaim(this)](const TouchEventInfo& info) {
            auto indexerPattern = weak.Upgrade();
            CHECK_NULL_VOID(indexerPattern);
            if (info.GetTouches().front().GetTouchType() == TouchType::DOWN) {
                indexerPattern->OnTouchDown(info);
            }
            indexerPattern->SetIsTouch(true);
        };
        touchListener_ = MakeRefPtr<TouchEventImpl>(std::move(touchCallback));
        gesture->AddTouchEvent(touchListener_);
    }

    auto focusHub = host->GetFocusHub();
    if (focusHub) {
        InitOnKeyEvent(focusHub);
    }
}

bool IndexerPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure && config.skipLayout) {
        return false;
    }
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto indexerLayoutAlgorithm = DynamicCast<IndexerLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(indexerLayoutAlgorithm, false);
    isInitialized_ = indexerLayoutAlgorithm->GetIsInitialized();
    selected_ = indexerLayoutAlgorithm->GetSelected();
    itemSizeRender_ = indexerLayoutAlgorithm->GetItemSizeRender();
    return false;
}

void IndexerPattern::InitPanEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (panEvent_) {
        return;
    }
    auto onActionStart = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            if (info.GetInputEventType() == InputEventType::AXIS) {
                return;
            }
            pattern->MoveIndexByOffset(info.GetLocalLocation(), false);
        }
    };

    auto onActionUpdate = [weak = WeakClaim(this)](const GestureEvent& info) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            if (info.GetInputEventType() == InputEventType::AXIS) {
                if (GreatNotEqual(info.GetMainDelta(), 0.0)) {
                    pattern->MoveIndexByStep(-1);
                } else if (LessNotEqual(info.GetMainDelta(), 0.0)) {
                    pattern->MoveIndexByStep(1);
                }
            } else {
                pattern->MoveIndexByOffset(info.GetLocalLocation(), false);
            }
        }
    };

    auto onActionEnd = [weak = WeakClaim(this)](const GestureEvent& info) {};

    auto onActionCancel = [weak = WeakClaim(this)]() {};

    PanDirection panDirection;
    panDirection.type = PanDirection::VERTICAL;
    panEvent_ = MakeRefPtr<PanEvent>(
        std::move(onActionStart), std::move(onActionUpdate), std::move(onActionEnd), std::move(onActionCancel));
    gestureHub->AddPanEvent(panEvent_, panDirection, 1, 0.0);
}

void IndexerPattern::OnTouchDown(const TouchEventInfo& info)
{
    MoveIndexByOffset(info.GetTouches().front().GetLocalLocation());
}

void IndexerPattern::MoveIndexByOffset(const Offset& offset, bool isRepeatCalled)
{
    LOGD("Move index by offset: (%{public}f,%{public}f). isRepeatCalled is %{public}d", offset.GetX(), offset.GetY(),
        isRepeatCalled);
    if (itemSizeRender_ <= 0) {
        return;
    }
    if (itemCount_ <= 0) {
        LOGE("AlphabetIndexer arrayValue size is less than 0");
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<IndexerLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);

    auto size = SizeF(itemSizeRender_, itemSizeRender_ * static_cast<float>(itemCount_));
    auto padding = layoutProperty->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    auto top = padding.top.value_or(0.0f);

    auto nextSelectIndex = static_cast<int32_t>((offset.GetY() - top) / itemSizeRender_);
    nextSelectIndex = std::clamp(nextSelectIndex, 0, itemCount_ - 1);
    if (!isRepeatCalled && nextSelectIndex == selected_) {
        return;
    }
    selected_ = nextSelectIndex;
    LOGD("Move to index: %{public}d", selected_);
    ApplyIndexChanged();
}

bool IndexerPattern::MoveIndexByStep(int32_t step)
{
    auto nextSelected = selected_ + step;
    LOGD("Move index by step: %{public}d, Move index from %{public}d to %{public}d", step, selected_, nextSelected);
    if (nextSelected < 0 || nextSelected >= itemCount_) {
        LOGW("Next select index: %{public}d is out of bounds[0, %{public}d))", nextSelected, itemCount_);
        return false;
    }
    selected_ = nextSelected;
    LOGD("Move to index: %{public}d", selected_);
    ApplyIndexChanged();
    return true;
}

bool IndexerPattern::MoveIndexBySearch(const std::string& searchStr)
{
    LOGD("Move index by search string: %{public}s", searchStr.c_str());
    int32_t nextSelectIndex = -1;
    for (auto i = selected_ + 1; i < itemCount_; ++i) {
        const auto& indexValue = arrayValue_.at(i);
        if (searchStr.length() > indexValue.length()) {
            continue;
        }
        if (strcasecmp(indexValue.substr(0, searchStr.length()).c_str(), searchStr.c_str()) == 0) {
            nextSelectIndex = i;
            break;
        }
    }
    if (nextSelectIndex >= 0 && nextSelectIndex < itemCount_) {
        selected_ = nextSelectIndex;
        LOGD("Find string: %{public}s at index: %{public}d. Move to it.", searchStr.c_str(), selected_);
        ApplyIndexChanged();
        return true;
    }
    for (auto i = 0; i < selected_; ++i) {
        const auto& indexValue = arrayValue_.at(i);
        if (searchStr.length() > indexValue.length()) {
            continue;
        }
        if (strcasecmp(indexValue.substr(0, searchStr.length()).c_str(), searchStr.c_str()) == 0) {
            nextSelectIndex = i;
            break;
        }
    }
    if (nextSelectIndex >= 0 && nextSelectIndex < itemCount_) {
        selected_ = nextSelectIndex;
        LOGD("Find string: %{public}s at index: %{public}d. Move to it.", searchStr.c_str(), selected_);
        ApplyIndexChanged();
        return true;
    }
    LOGD("Search string: %{public}s does not exist.", searchStr.c_str());
    return false;
}

void IndexerPattern::ApplyIndexChanged()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<IndexerLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);

    auto indexerEventHub = host->GetEventHub<IndexerEventHub>();
    CHECK_NULL_VOID(indexerEventHub);
        
    auto onSelected = indexerEventHub->GetOnSelected();
    if (onSelected && (selected_ >= 0) && (selected_ < itemCount_)) {
        onSelected(selected_);
    }

    auto onRequestPopupData = indexerEventHub->GetOnRequestPopupData();
    std::optional<std::vector<std::string>> popupData;
    if (onRequestPopupData && (selected_ >= 0) && (selected_ < itemCount_)) {
        popupData = onRequestPopupData(selected_);
    }

    auto onPopupSelected = indexerEventHub->GetOnPopupSelected();
    if (onPopupSelected && (selected_ >= 0) && (selected_ < itemCount_)) {
        onPopupSelected(selected_);
    }
        
    auto color = layoutProperty->GetColor().value_or(Color(INDEXER_LIST_COLOR));
    auto selectedColor = layoutProperty->GetSelectedColor().value_or(Color(INDEXER_LIST_ACTIVE_COLOR));
    auto popupColor = layoutProperty->GetPopupColor().value_or(Color(BUBBLE_FONT_COLOR));
    auto selectedBackgroundColor = layoutProperty->GetSelectedBackgroundColor()
        .value_or(Color(INDEXER_ACTIVE_BG_COLOR));
    auto popupBackground = layoutProperty->GetPopupBackground().value_or(Color(BUBBLE_BG_COLOR));
    auto usingPopup = layoutProperty->GetUsingPopup().value_or(false);
    TextStyle textStyle;
    auto selectedFont = layoutProperty->GetSelectedFont().value_or(textStyle);
    auto popupFont = layoutProperty->GetPopupFont().value_or(textStyle);
    auto font = layoutProperty->GetFont().value_or(textStyle);

    int32_t index = 0;
    auto childrenNode = host->GetChildren();
    for (auto& iter : childrenNode) {
        auto childNode = AceType::DynamicCast<FrameNode>(iter);
        auto nodeLayoutProperty = childNode->GetLayoutProperty<TextLayoutProperty>();
        if (index == selected_) {
            nodeLayoutProperty->UpdateTextColor(selectedColor);
            auto fontSize = selectedFont.GetFontSize();
            nodeLayoutProperty->UpdateFontSize(fontSize);
            auto fontWeight = selectedFont.GetFontWeight();
            nodeLayoutProperty->UpdateFontWeight(fontWeight);
            auto childRenderContext = childNode->GetRenderContext();
            childRenderContext->BlendBgColor(selectedBackgroundColor);
        } else if (index == itemCount_) {
            if (usingPopup) {
                std::vector<std::string> arrayValueSelected = {};
                auto popupDataValue = popupData.value_or(arrayValueSelected);
                popupDataValue.insert(std::begin(popupDataValue), arrayValue_[selected_]);

                auto listNode = AceType::DynamicCast<FrameNode>(iter);
                listNode->Clean();
                int32_t popupDataIndex = 0;
                for (const auto& data: popupDataValue) {
                    auto textNode = FrameNode::CreateFrameNode(V2::TEXT_ETS_TAG, -1,
                        AceType::MakeRefPtr<TextPattern>());
                    auto textNodeLayoutProperty = textNode->GetLayoutProperty<TextLayoutProperty>();
                    textNodeLayoutProperty->UpdateContent(data);
                    textNodeLayoutProperty->UpdateTextColor(popupColor);
                    auto fontSize = popupFont.GetFontSize();
                    textNodeLayoutProperty->UpdateFontSize(fontSize);
                    auto fontWeight = popupFont.GetFontWeight();
                    textNodeLayoutProperty->UpdateFontWeight(fontWeight);

                    CalcLength width = CalcLength(Dimension(BUBBLE_BOX_SIZE));
                    CalcLength height = CalcLength(Dimension(BUBBLE_BOX_SIZE));
                    textNodeLayoutProperty->UpdateUserDefinedIdealSize(CalcSize(width, height));

                    auto textNodeRenderContext = textNode->GetRenderContext();
                    textNodeRenderContext->BlendBgColor(popupBackground);

                    Dimension radius = Dimension(NG::BUBBLE_BOX_RADIUS);
                    if (popupDataValue.size() <= 1) {
                        BorderRadiusProperty borderRadius { radius, radius, radius, radius };
                        textNodeRenderContext->UpdateBorderRadius(borderRadius);
                    } else {
                        if (popupDataIndex == 0) {
                            BorderRadiusProperty borderRadius { radius, radius, Dimension(0), Dimension(0) };
                            textNodeRenderContext->UpdateBorderRadius(borderRadius);
                        } else if (popupDataIndex == INDEXER_BUBBLE_MAXSIZE - 1) {
                            BorderRadiusProperty borderRadius { Dimension(0), Dimension(0), radius, radius };
                            textNodeRenderContext->UpdateBorderRadius(borderRadius);
                        }
                    }
                    textNode->MarkModifyDone();
                    auto listItemNode = FrameNode::GetOrCreateFrameNode(
                        V2::LIST_ITEM_ETS_TAG, -1, []() { return AceType::MakeRefPtr<ListItemPattern>(nullptr); });
                    listItemNode->AddChild(textNode);
                    listItemNode->MarkModifyDone();
                    
                    listNode->AddChild(listItemNode);
                    popupDataIndex++;
                }
                listNode->MarkModifyDone();
                listNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
                BeginBubbleAnimation(listNode);
            }
        } else {
            nodeLayoutProperty->UpdateTextColor(color);
            auto fontSize = font.GetFontSize();
            nodeLayoutProperty->UpdateFontSize(fontSize);
            auto fontWeight = font.GetFontWeight();
            nodeLayoutProperty->UpdateFontWeight(fontWeight);

            auto childRenderContext = childNode->GetRenderContext();
            childRenderContext->ResetBlendBgColor();
        }
        index++;
    }

    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

void IndexerPattern::InitOnKeyEvent(const RefPtr<FocusHub>& focusHub)
{
    auto onKeyEvent = [wp = WeakClaim(this)](const KeyEvent& event) -> bool {
        auto pattern = wp.Upgrade();
        if (pattern) {
            return pattern->OnKeyEvent(event);
        }
        return false;
    };
    focusHub->SetOnKeyEventInternal(std::move(onKeyEvent));
}

bool IndexerPattern::OnKeyEvent(const KeyEvent& event)
{
    if (event.action != KeyAction::DOWN) {
        return false;
    }
    if (event.code == KeyCode::KEY_DPAD_UP) {
        return MoveIndexByStep(-1);
    }
    if (event.code == KeyCode::KEY_DPAD_DOWN) {
        return MoveIndexByStep(1);
    }
    if (!event.IsCombinationKey() && (event.IsLetterKey() || event.IsNumberKey())) {
        return MoveIndexBySearch(event.ConvertCodeToString());
    }
    return false;
}

void IndexerPattern::BeginBubbleAnimation(RefPtr<FrameNode> animationNode)
{
    auto renderContext = animationNode->GetRenderContext();
    AnimationOption animationOption;
    animationOption.SetDuration(INDEXER_BUBBLE_ANIMATION_DURATION);
    animationOption.SetCurve(Curves::DECELE);
    renderContext->OpacityAnimation(animationOption, 1.0f, 0.0f);
}
} // namespace OHOS::Ace::NG
