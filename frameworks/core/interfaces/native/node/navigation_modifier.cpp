/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "core/interfaces/native/node/navigation_modifier.h"

#include "base/geometry/calc_dimension.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/pattern/navigation/navigation_declaration.h"
#include "core/components_ng/pattern/navigation/navigation_model_ng.h"
#include "core/pipeline/base/element_register.h"

namespace OHOS::Ace::NG {
constexpr int32_t NAV_BAR_POSITION_RANGE_MODIFIER = 1;
constexpr int32_t NAVIGATION_MODE_RANGE_MODIFIER = 2;
constexpr int32_t DEFAULT_NAV_BAR_WIDTH_FOR_MODIFIER = 240;
constexpr int32_t DEFAULT_SAFE_AREA_TYPE = 0b1;
constexpr int32_t DEFAULT_SAFE_AREA_EDGE = 0b1111;
void SetHideToolBar(ArkUINodeHandle node, ArkUI_Bool hide)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavigationModelNG::SetHideToolBar(frameNode, hide);
}

void ResetHideToolBar(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavigationModelNG::SetHideToolBar(frameNode, false);
}

void SetMinContentWidth(ArkUINodeHandle node, ArkUI_Float32 value, int unit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension width = Dimension(value, static_cast<OHOS::Ace::DimensionUnit>(unit));
    NavigationModelNG::SetMinContentWidth(frameNode, width);
}

void ResetMinContentWidth(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavigationModelNG::SetMinContentWidth(frameNode, DEFAULT_MIN_CONTENT_WIDTH);
}

void SetMinNavBarWidth(ArkUINodeHandle node, ArkUI_Float32 minValue, int minUnit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension minNavBarWidth = Dimension(minValue, static_cast<OHOS::Ace::DimensionUnit>(minUnit));
    if (LessNotEqual(minNavBarWidth.Value(), 0.0)) {
        minNavBarWidth = DEFAULT_MIN_NAV_BAR_WIDTH;
    }

    NavigationModelNG::SetMinNavBarWidth(frameNode, minNavBarWidth);
}

void ResetMinNavBarWidth(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension minNavBarWidth = DEFAULT_MIN_NAV_BAR_WIDTH;
    NavigationModelNG::SetMinNavBarWidth(frameNode, minNavBarWidth);
}

void SetMaxNavBarWidth(ArkUINodeHandle node, ArkUI_Float32 maxValue, int maxUnit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension maxNavBarWidth = Dimension(maxValue, static_cast<OHOS::Ace::DimensionUnit>(maxUnit));
    if (LessNotEqual(maxNavBarWidth.Value(), 0.0)) {
        maxNavBarWidth = DEFAULT_MAX_NAV_BAR_WIDTH;
    }
    
    NavigationModelNG::SetMaxNavBarWidth(frameNode, maxNavBarWidth);
}

void ResetMaxNavBarWidth(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension maxNavBarWidth = DEFAULT_MAX_NAV_BAR_WIDTH;
    NavigationModelNG::SetMaxNavBarWidth(frameNode, maxNavBarWidth);
}

void SetNavBarWidth(ArkUINodeHandle node, ArkUI_Float32 value, int unit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension width = Dimension(value, static_cast<OHOS::Ace::DimensionUnit>(unit));
    NavigationModelNG::SetNavBarWidth(frameNode, width);
}

void ResetNavBarWidth(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension navBarWidth;
    navBarWidth.SetValue(DEFAULT_NAV_BAR_WIDTH_FOR_MODIFIER);
    NavigationModelNG::SetNavBarWidth(frameNode, navBarWidth);
}

void SetNavBarPosition(ArkUINodeHandle node, int32_t value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    if (value >= 0 && value <= NAV_BAR_POSITION_RANGE_MODIFIER) {
        NavigationModelNG::SetNavBarPosition(frameNode, static_cast<NG::NavBarPosition>(value));
    } else {
        NavigationModelNG::SetNavBarPosition(frameNode, NG::NavBarPosition::START);
    }
}

void ResetNavBarPosition(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavigationModelNG::SetNavBarPosition(frameNode, NG::NavBarPosition::START);
}

void SetHideNavBar(ArkUINodeHandle node, ArkUI_Bool hideNavBar)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavigationModelNG::SetHideNavBar(frameNode, hideNavBar);
}

void ResetHideNavBar(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavigationModelNG::SetHideNavBar(frameNode, false);
}

void SetSubtitle(ArkUINodeHandle node, const char* subtitle)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    std::string value = subtitle;
    NavigationModelNG::SetSubtitle(frameNode, value);
}

void ResetSubtitle(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavigationModelNG::SetSubtitle(frameNode, "");
}

void SetHideBackButton(ArkUINodeHandle node, ArkUI_Bool hideBackButton)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavigationModelNG::SetHideBackButton(frameNode, hideBackButton);
}

void ResetHideBackButton(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavigationModelNG::SetHideBackButton(frameNode, false);
}

void SetTitleMode(ArkUINodeHandle node, int32_t value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavigationModelNG::SetTitleMode(frameNode, static_cast<NG::NavigationTitleMode>(value));
}

void ResetTitleMode(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavigationModelNG::SetTitleMode(frameNode, NG::NavigationTitleMode::FREE);
}

void SetUsrNavigationMode(ArkUINodeHandle node, int32_t value)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    if (value >= 0 && value <= NAVIGATION_MODE_RANGE_MODIFIER) {
        NavigationModelNG::SetUsrNavigationMode(frameNode, static_cast<NG::NavigationMode>(value));
    } else {
        NavigationModelNG::SetUsrNavigationMode(frameNode, NG::NavigationMode::AUTO);
    }
}
void ResetUsrNavigationMode(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavigationModelNG::SetUsrNavigationMode(frameNode, NG::NavigationMode::AUTO);
}

void SetNavHideTitleBar(ArkUINodeHandle node, ArkUI_Bool hideBackButton)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavigationModelNG::SetHideTitleBar(frameNode, hideBackButton);
}

void ResetNavHideTitleBar(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NavigationModelNG::SetHideTitleBar(frameNode, false);
}

void SetNavIgnoreLayoutSafeArea(ArkUINodeHandle node, const char* typeStr, const char* edgesStr)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NG::SafeAreaExpandOpts opts { .type = NG::SAFE_AREA_TYPE_SYSTEM, .edges = NG::SAFE_AREA_EDGE_ALL };
    uint32_t safeAreaType = NG::SAFE_AREA_TYPE_NONE;
    uint32_t safeAreaEdge = NG::SAFE_AREA_EDGE_NONE;
    std::string safeAreaTypeStr = std::string(typeStr);
    std::string safeAreaEdgeStr = std::string(edgesStr);
    if (safeAreaTypeStr == "" || safeAreaEdgeStr == "") {
        NG::SafeAreaExpandOpts opts { .type = NG::SAFE_AREA_TYPE_NONE, .edges = NG::SAFE_AREA_EDGE_NONE};
        NavigationModelNG::SetIgnoreLayoutSafeArea(frameNode, opts);
        return;
    }
    std::string delimiter = "|";
    size_t pos = 0;
    std::string type;
    std::string edges;
    while ((pos = safeAreaTypeStr.find(delimiter)) != std::string::npos) {
        type = safeAreaTypeStr.substr(0, pos);
        safeAreaType |= (1 << StringUtils::StringToUint(type));
        safeAreaTypeStr.erase(0, pos + delimiter.length());
    }
    safeAreaType |= (1 << StringUtils::StringToUint(safeAreaTypeStr));
    pos = 0;
    while ((pos = safeAreaEdgeStr.find(delimiter)) != std::string::npos) {
        edges = safeAreaEdgeStr.substr(0, pos);
        safeAreaEdge |= (1 << StringUtils::StringToUint(edges));
        safeAreaEdgeStr.erase(0, pos + delimiter.length());
    }
    safeAreaEdge |= (1 << StringUtils::StringToUint(safeAreaEdgeStr));
    opts.type = safeAreaType;
    opts.edges = safeAreaEdge;
    NavigationModelNG::SetIgnoreLayoutSafeArea(frameNode, opts);
}

void ResetNavIgnoreLayoutSafeArea(ArkUINodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NG::SafeAreaExpandOpts opts;
    opts.type = DEFAULT_SAFE_AREA_TYPE;
    opts.edges = DEFAULT_SAFE_AREA_EDGE;
    NavigationModelNG::SetIgnoreLayoutSafeArea(frameNode, opts);
}

namespace NodeModifier {
const ArkUINavigationModifier* GetNavigationModifier()
{
    static const ArkUINavigationModifier modifier = {
        SetHideToolBar,
        ResetHideToolBar,
        SetHideNavBar,
        ResetHideNavBar,
        SetTitleMode,
        ResetTitleMode,
        SetHideBackButton,
        ResetHideBackButton,
        SetSubtitle,
        ResetSubtitle,
        ResetUsrNavigationMode,
        SetUsrNavigationMode,
        SetNavHideTitleBar,
        ResetNavHideTitleBar,
        SetNavBarPosition,
        ResetNavBarPosition,
        SetMinContentWidth,
        ResetMinContentWidth,
        SetMaxNavBarWidth,
        ResetMaxNavBarWidth,
        SetMinNavBarWidth,
        ResetMinNavBarWidth,
        SetNavBarWidth,
        ResetNavBarWidth,
        SetNavIgnoreLayoutSafeArea,
        ResetNavIgnoreLayoutSafeArea
    };

    return &modifier;
}
}
} // namespace OHOS::Ace::NG