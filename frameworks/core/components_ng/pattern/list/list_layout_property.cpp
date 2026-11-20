/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/list/list_layout_property.h"
#include "core/components_ng/pattern/scrollable/scroll_snap_strategy.h"
#include "interfaces/inner_api/ui_session/param_config.h"

namespace OHOS::Ace::NG {
namespace {
V2::ItemDivider ItemDividerFromJson(const std::unique_ptr<JsonValue>& json)
{
    V2::ItemDivider divider;
    divider.strokeWidth = Dimension::FromString(json->GetString("strokeWidth"));
    divider.startMargin = Dimension::FromString(json->GetString("startMargin"));
    divider.endMargin = Dimension::FromString(json->GetString("endMargin"));
    divider.color = Color::ColorFromString(json->GetString("color"));
    return divider;
}

const char* ScrollSnapStrategyAlignToString(ScrollSnapAlign align)
{
    switch (align) {
        case ScrollSnapAlign::START:
            return "ScrollSnapAlign.START";
        case ScrollSnapAlign::CENTER:
            return "ScrollSnapAlign.CENTER";
        case ScrollSnapAlign::END:
            return "ScrollSnapAlign.END";
        case ScrollSnapAlign::NONE:
            return "ScrollSnapAlign.NONE";
        default:
            return "ScrollSnapAlign.NONE";
    }
}
} // namespace

std::optional<ScrollSnapStrategy> ListLayoutProperty::GetScrollSnapStrategy() const
{
    if (!snapStrategyAlign_) {
        return std::nullopt;
    }
    return ScrollSnapStrategy { snapStrategyAlign_.value(), snapHasProvider_, snapApproachOffset_, snapOffset_ };
}

void ListLayoutProperty::UpdateScrollSnapStrategy(const ScrollSnapStrategy& strategy)
{
    if (snapStrategyAlign_ && !snapHasProvider_ && !strategy.hasProvider &&
        snapStrategyAlign_.value() == strategy.align) {
        return;
    }
    snapStrategyAlign_ = strategy.align;
    snapHasProvider_ = strategy.hasProvider;
    snapApproachOffset_ = strategy.calculateApproachOffset;
    snapOffset_ = strategy.calculateSnapOffset;
    UpdatePropertyChangeFlag(PROPERTY_UPDATE_MEASURE);
}

void ListLayoutProperty::ResetScrollSnapStrategy()
{
    snapStrategyAlign_.reset();
    snapHasProvider_ = false;
    snapApproachOffset_ = nullptr;
    snapOffset_ = nullptr;
}

void ListLayoutProperty::SetListItemFillPolicy(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const
{
    if (propItemFillPolicy_.has_value()) {
        if (propItemFillPolicy_.value() == PresetFillType::BREAKPOINT_SM1MD2LG3) {
            json->PutExtAttr("itemFillPolicy", "PresetFillType::BREAKPOINT_SM1MD2LG3", filter);
        } else if (propItemFillPolicy_.value() == PresetFillType::BREAKPOINT_SM2MD3LG5) {
            json->PutExtAttr("itemFillPolicy", "PresetFillType::BREAKPOINT_SM2MD3LG5", filter);
        } else {
            json->PutExtAttr("itemFillPolicy", "PresetFillType::BREAKPOINT_DEFAULT", filter);
        }
    }
}

void ListLayoutProperty::DumpInfo()
{
    auto listItemAlign = propListItemAlign_.value_or(V2::ListItemAlign::START);
    DumpLog::GetInstance().AddDesc("alignListItem:" + std::to_string(static_cast<int32_t>(listItemAlign)));
    if (propDivider_.has_value()) {
        auto& div = propDivider_.value();
        DumpLog::GetInstance().AddDesc("divider.strokeWidth:" + div.strokeWidth.ToString());
        DumpLog::GetInstance().AddDesc("divider.startMargin:" + div.startMargin.ToString());
        DumpLog::GetInstance().AddDesc("divider.endMargin:" + div.endMargin.ToString());
        DumpLog::GetInstance().AddDesc("divider.color:" + div.color.ColorToString());
    } else {
        DumpLog::GetInstance().AddDesc("divider: None");
    }
    auto stickyStyle = propStickyStyle_.value_or(V2::StickyStyle::NONE);
    DumpLog::GetInstance().AddDesc(
        "sticky:" + std::to_string(static_cast<uint32_t>(stickyStyle)));
    auto scrollSnapAlign = propScrollSnapAlign_.value_or(ScrollSnapAlign::NONE);
    DumpLog::GetInstance().AddDesc(
        "scrollSnapAlign:" + std::to_string(static_cast<int32_t>(scrollSnapAlign)));
    if (snapStrategyAlign_) {
        DumpLog::GetInstance().AddDesc(std::string("scrollSnapStrategy:") +
            (snapHasProvider_ ? "provider" : ScrollSnapStrategyAlignToString(snapStrategyAlign_.value())));
    } else {
        DumpLog::GetInstance().AddDesc("scrollSnapStrategy:unset");
    }
    propSyncLoad_.value_or(false) ? DumpLog::GetInstance().AddDesc("syncLoad:true")
                                 : DumpLog::GetInstance().AddDesc("syncLoad:false");
}

void ListLayoutProperty::DumpInfo(std::unique_ptr<JsonValue>& json)
{
    DumpInfo(json, ParamConfig());
}

void ListLayoutProperty::DumpInfo(std::unique_ptr<JsonValue>& json, const ParamConfig& config)
{
    if (!config.IsPublicTree() || config.visualProperties) {
        auto listItemAlign = propListItemAlign_.value_or(V2::ListItemAlign::START);
        if (config.IsPublicTree()) {
            json->Put("alignListItem", static_cast<int32_t>(listItemAlign));
        } else {
            json->Put("alignListItem", std::to_string(static_cast<int32_t>(listItemAlign)).c_str());
        }
        if (propDivider_.has_value()) {
            auto& div = propDivider_.value();
            std::unique_ptr<JsonValue> dividerJson = JsonUtil::Create(true);
            dividerJson->Put("strokeWidth", div.strokeWidth.ToString().c_str());
            dividerJson->Put("startMargin", div.startMargin.ToString().c_str());
            dividerJson->Put("endMargin", div.endMargin.ToString().c_str());
            dividerJson->Put("color", div.color.ColorToString().c_str());
            json->Put("divider", dividerJson);
        } else if (!config.IsPublicTree()) {
            json->Put("divider", "None");
        }
    }
    if (!config.IsPublicTree() || config.functionalProperties) {
        auto stickyStyle = propStickyStyle_.value_or(V2::StickyStyle::NONE);
        if (config.IsPublicTree()) {
            json->Put("sticky", static_cast<int64_t>(static_cast<uint32_t>(stickyStyle)));
        } else {
            json->Put("sticky", std::to_string(static_cast<uint32_t>(stickyStyle)).c_str());
        }
        auto scrollSnapAlign = propScrollSnapAlign_.value_or(ScrollSnapAlign::NONE);
        if (config.IsPublicTree()) {
            json->Put("scrollSnapAlign", static_cast<int32_t>(scrollSnapAlign));
        } else {
            json->Put("scrollSnapAlign", std::to_string(static_cast<int32_t>(scrollSnapAlign)).c_str());
        }
        json->Put("syncLoad", propSyncLoad_.value_or(false));
    }
}

void ListLayoutProperty::ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const
{
    ScrollableLayoutProperty::ToJsonValue(json, filter);
    /* no fixed attr below, just return */
    if (filter.IsFastFilter()) {
        ScrollSnapPropToJsonValue(json, filter);
        return;
    }
    json->PutExtAttr("space", propSpace_.value_or(Dimension(0, DimensionUnit::VP)).ToString().c_str(), filter);
    json->PutExtAttr("initialIndex", std::to_string(propInitialIndex_.value_or(0)).c_str(), filter);
    json->PutExtAttr("listDirection", propListDirection_.value_or(Axis::VERTICAL) == Axis::VERTICAL
                                   ? "Axis.Vertical" : "Axis.Horizontal", filter);
    json->PutExtAttr("editMode", propEditMode_.value_or(false), filter);
    json->PutExtAttr("chainAnimation", propChainAnimation_.value_or(false), filter);
    auto divider = JsonUtil::Create(true);
    if (propDivider_.has_value()) {
        divider->Put("strokeWidth", propDivider_.value().strokeWidth.ToString().c_str());
        divider->Put("startMargin", propDivider_.value().startMargin.ToString().c_str());
        divider->Put("endMargin", propDivider_.value().endMargin.ToString().c_str());
        divider->Put("color", propDivider_.value().color.ColorToString().c_str());
    }
    json->PutExtAttr("divider", divider, filter);
    json->PutExtAttr("lanes", std::to_string(propLanes_.value_or(0)).c_str(), filter);
    json->PutExtAttr("laneMinLength",
        propLaneMinLength_.value_or(Dimension(0, DimensionUnit::VP)).ToString().c_str(), filter);
    json->PutExtAttr("laneMaxLength",
        propLaneMaxLength_.value_or(Dimension(0, DimensionUnit::VP)).ToString().c_str(), filter);
    SetListItemFillPolicy(json, filter);
    json->PutExtAttr("laneGutter",
        propLaneGutter_.value_or(Dimension(0, DimensionUnit::VP)).ToString().c_str(), filter);
    if (propListItemAlign_.value_or(V2::ListItemAlign::START) == V2::ListItemAlign::START) {
        json->PutExtAttr("alignListItem", "ListItemAlign.Start", filter);
    } else if (propListItemAlign_.value_or(V2::ListItemAlign::START) == V2::ListItemAlign::CENTER) {
        json->PutExtAttr("alignListItem", "ListItemAlign.Center", filter);
    } else {
        json->PutExtAttr("alignListItem", "ListItemAlign.End", filter);
    }
    json->PutExtAttr("cachedCount", std::to_string(propCachedCount_.value_or(0)).c_str(), filter);
    auto sticky = propStickyStyle_.value_or(V2::StickyStyle::NONE);
    if (sticky == V2::StickyStyle::HEADER) {
        json->PutExtAttr("sticky", "StickyStyle.Header", filter);
    } else if (sticky == V2::StickyStyle::FOOTER) {
        json->PutExtAttr("sticky", "StickyStyle.Footer", filter);
    } else if (sticky == V2::StickyStyle::BOTH) {
        json->PutExtAttr("sticky", "StickyStyle.Header | StickyStyle.Footer", filter);
    } else {
        json->PutExtAttr("sticky", "StickyStyle.None", filter);
    }
    ScrollSnapPropToJsonValue(json, filter);
    json->PutExtAttr("syncLoad", propSyncLoad_.value_or(true), filter);
}

void ListLayoutProperty::ScrollSnapPropToJsonValue(
    std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const
{
    /* no fixed attr below, just return */
    if (filter.IsFastFilter()) {
        return;
    }
    auto scrollSnapAlign = propScrollSnapAlign_.value_or(ScrollSnapAlign::NONE);
    if (scrollSnapAlign == ScrollSnapAlign::START) {
        json->PutExtAttr("scrollSnapAlign", "ScrollSnapAlign.START", filter);
    } else if (scrollSnapAlign == ScrollSnapAlign::CENTER) {
        json->PutExtAttr("scrollSnapAlign", "ScrollSnapAlign.CENTER", filter);
    } else if (scrollSnapAlign == ScrollSnapAlign::END) {
        json->PutExtAttr("scrollSnapAlign", "ScrollSnapAlign.END", filter);
    } else {
        json->PutExtAttr("scrollSnapAlign", "ScrollSnapAlign.NONE", filter);
    }
    // Inspector-visible final value of scrollSnapStrategy — built-in align
    // enum, cleared state, or a mounted provider (callbacks themselves are not serialized).
    // Family-unified convention (matches the sibling scrollSnapAlign above): an unset
    // strategy serializes as the effective default ScrollSnapAlign.NONE.
    if (snapHasProvider_) {
        json->PutExtAttr("scrollSnapStrategy", "ScrollSnapOffsetProvider", filter);
    } else {
        json->PutExtAttr("scrollSnapStrategy",
            ScrollSnapStrategyAlignToString(snapStrategyAlign_.value_or(ScrollSnapAlign::NONE)), filter);
    }
    json->PutExtAttr("enableScrollInteraction", propScrollEnabled_.value_or(true), filter);
    json->PutExtAttr(
        "spaceWidth", propSpaceWidth_.value_or(Dimension(0, DimensionUnit::VP)).ToString().c_str(), filter);
}

void ListLayoutProperty::FromJson(const std::unique_ptr<JsonValue>& json)
{
    UpdateSpace(Dimension::FromString(json->GetString("space")));
    UpdateInitialIndex(StringUtils::StringToInt(json->GetString("initialIndex")));
    auto dividerJson = json->GetObject("divider");
    if (dividerJson->Contains("strokeWidth")) {
        UpdateDivider(ItemDividerFromJson(dividerJson));
    }
    LayoutProperty::FromJson(json);
}

void ListLayoutProperty::Clone(RefPtr<LayoutProperty> layoutProperty) const
{
    auto value = DynamicCast<ListLayoutProperty>(layoutProperty);
    ScrollableLayoutProperty::Clone(value);
    value->LayoutProperty::UpdateLayoutProperty(this);
    value->propSpace_ = CloneSpace();
    value->propSpaceWidth_ = CloneSpaceWidth();
    value->propInitialIndex_ = CloneInitialIndex();
    value->propListDirection_ = CloneListDirection();
    value->propDivider_ = CloneDivider();
    value->propLanes_ = CloneLanes();
    value->propLaneMinLength_ = CloneLaneMinLength();
    value->propLaneMaxLength_ = CloneLaneMaxLength();
    value->propLaneGutter_ = CloneLaneGutter();
    value->propListItemAlign_ = CloneListItemAlign();
    value->propCachedCount_ = CloneCachedCount();
    value->propShowCachedItems_ = CloneShowCachedItems();
    value->propStickyStyle_ = CloneStickyStyle();
    value->propScrollSnapAlign_ = CloneScrollSnapAlign();
    value->propEditMode_ = CloneEditMode();
    value->propScrollEnabled_ = CloneScrollEnabled();
    value->propStackFromEnd_ = CloneStackFromEnd();
    value->propSyncLoad_ = CloneSyncLoad();
    value->propCacheRange_ = CloneCacheRange();
    value->propItemFillPolicy_ = CloneItemFillPolicy();
    value->snapStrategyAlign_ = snapStrategyAlign_;
    value->snapHasProvider_ = snapHasProvider_;
    value->snapApproachOffset_ = snapApproachOffset_;
    value->snapOffset_ = snapOffset_;
}

void ListLayoutProperty::Reset()
{
    ScrollableLayoutProperty::Reset();
    ResetSpace();
    ResetSpaceWidth();
    ResetInitialIndex();
    ResetListDirection();
    ResetDivider();
    ResetLanes();
    ResetLaneMinLength();
    ResetLaneMaxLength();
    ResetLaneGutter();
    ResetListItemAlign();
    ResetCachedCount();
    ResetShowCachedItems();
    ResetStickyStyle();
    ResetScrollSnapAlign();
    ResetEditMode();
    ResetScrollEnabled();
    ResetStackFromEnd();
    ResetSyncLoad();
    ResetCacheRange();
    ResetItemFillPolicy();
    ResetScrollSnapStrategy();
}
} // namespace OHOS::Ace::NG
