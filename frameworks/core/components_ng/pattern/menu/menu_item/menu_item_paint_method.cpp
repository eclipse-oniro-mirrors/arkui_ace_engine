/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/menu/menu_item/menu_item_paint_method.h"

#include "core/components_ng/pattern/menu/menu_item/menu_item_pattern.h"
#include "core/components_ng/render/drawing.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace::NG {
CanvasDrawFunction MenuItemPaintMethod::GetOverlayDrawFunction(PaintWrapper* paintWrapper)
{
    return [weak = WeakClaim(this), paintWrapper, isOption = isOption_](RSCanvas& canvas) {
        auto menuItem = weak.Upgrade();
        CHECK_NULL_VOID(menuItem);
        CHECK_NULL_VOID(paintWrapper);
    
        auto props = DynamicCast<MenuItemPaintProperty>(paintWrapper->GetPaintProperty());
        CHECK_NULL_VOID(props);
        if (!isOption && !props->GetDividerColor().has_value()) {
            return;
        }
    
        bool needDivider = props->GetNeedDivider().value_or(true);
        bool press = props->GetPress().value_or(false);
        bool hover = props->GetHover().value_or(false);
        if (!needDivider) {
            return;
        }
        auto renderContext = paintWrapper->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        auto host = renderContext->GetHost();
        CHECK_NULL_VOID(host);
        auto pattern = host->GetPattern<MenuItemPattern>();
        CHECK_NULL_VOID(pattern);
        auto topDivider = pattern->GetTopDivider();
        if (topDivider && topDivider->GetParent()) {
            return;
        }
        auto pipeline = host->GetContext();
        CHECK_NULL_VOID(pipeline);
        auto selectTheme = host->GetTheme<SelectTheme>(true);
        CHECK_NULL_VOID(selectTheme);
        if (!selectTheme->GetDefaultShowDivider() && (press || hover)) {
            return;
        }
        RSPath path;
        if (isOption) {
            menuItem->HandleOption(paintWrapper, props, selectTheme, path, canvas);
        } else {
            menuItem->HandleMenuItem(paintWrapper, props, selectTheme, path, canvas);
        }
    };
}

void MenuItemPaintMethod::DrawPathWithBrush(RSCanvas& canvas, const RSPath& path, const Color& color)
{
    RSBrush brush;
    brush.SetColor(static_cast<int>(color.GetValue()));
    brush.SetAntiAlias(true);
    canvas.AttachBrush(brush);
    canvas.DrawPath(path);
    canvas.DetachBrush();
}

float MenuItemPaintMethod::GetLeftRowStartEdge(PaintWrapper* paintWrapper, bool isRtl)
{
    constexpr float INVALID_EDGE = -1.0f;
    auto renderContext = paintWrapper->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, INVALID_EDGE);
    auto host = renderContext->GetHost();
    CHECK_NULL_RETURN(host, INVALID_EDGE);
    // Custom builder (CustomMenuItemPattern) has unpredictable child structure;
    // index 0/1 assumption breaks, return INVALID_EDGE to fall back to theme default.
    auto pattern = host->GetPattern<MenuItemPattern>();
    CHECK_NULL_RETURN(pattern, INVALID_EDGE);
    if (pattern->IsCustomMenuItem()) {
        return INVALID_EDGE;
    }
    // LTR: index 0 = leftRow; RTL: index 1 = rightRow (visually on left)
    auto row = host->GetChildAtIndex(isRtl ? 1 : 0);
    CHECK_NULL_RETURN(row, INVALID_EDGE);
    auto frameNode = AceType::DynamicCast<FrameNode>(row);
    CHECK_NULL_RETURN(frameNode, INVALID_EDGE);
    auto geometry = frameNode->GetGeometryNode();
    CHECK_NULL_RETURN(geometry, INVALID_EDGE);
    return geometry->GetFrameRect().GetX();
}

float MenuItemPaintMethod::GetRightRowEndEdge(PaintWrapper* paintWrapper, bool isRtl)
{
    constexpr float INVALID_EDGE = -1.0f;
    auto renderContext = paintWrapper->GetRenderContext();
    CHECK_NULL_RETURN(renderContext, INVALID_EDGE);
    auto host = renderContext->GetHost();
    CHECK_NULL_RETURN(host, INVALID_EDGE);
    // Custom builder (CustomMenuItemPattern) has unpredictable child structure;
    // index 0/1 assumption breaks, return INVALID_EDGE to fall back to theme default.
    auto pattern = host->GetPattern<MenuItemPattern>();
    CHECK_NULL_RETURN(pattern, INVALID_EDGE);
    if (pattern->IsCustomMenuItem()) {
        return INVALID_EDGE;
    }
    // LTR: index 1 = rightRow; RTL: index 0 = leftRow (visually on right)
    auto row = host->GetChildAtIndex(isRtl ? 0 : 1);
    CHECK_NULL_RETURN(row, INVALID_EDGE);
    auto frameNode = AceType::DynamicCast<FrameNode>(row);
    CHECK_NULL_RETURN(frameNode, INVALID_EDGE);
    auto geometry = frameNode->GetGeometryNode();
    CHECK_NULL_RETURN(geometry, INVALID_EDGE);
    auto rect = geometry->GetFrameRect();
    return rect.GetX() + rect.Width();
}

void MenuItemPaintMethod::HandleMenuItem(PaintWrapper* paintWrapper, const RefPtr<MenuItemPaintProperty>& props,
    const RefPtr<SelectTheme>& selectTheme, RSPath& path, RSCanvas& canvas)
{
    auto itemSize = paintWrapper->GetGeometryNode()->GetFrameSize();
    auto strokeWidth = Dimension(1.0f, DimensionUnit::PX);
    Color dividerColor = Color::TRANSPARENT;
    if (selectTheme) {
        strokeWidth = selectTheme->GetDefaultDividerWidth();
        dividerColor =  selectTheme->GetLineColor();
    }

    ItemDividerInfo info;
    info.width = itemSize.Width();
    info.strokeWidth = props->GetStrokeWidth().value_or(strokeWidth).ConvertToPxWithSize(itemSize.Height());
    info.color = props->GetDividerColor().value_or(dividerColor);
    // Bridge sets Unit to INVALID on parse failure/unset; property always has a value,
    // so check Unit != INVALID to determine if user set it explicitly, not has_value().
    auto startMarginOpt = props->GetStartMargin();
    auto endMarginOpt = props->GetEndMargin();
    bool isStartMarginSet = startMarginOpt.has_value() && startMarginOpt->Unit() != DimensionUnit::INVALID;
    bool isEndMarginSet = endMarginOpt.has_value() && endMarginOpt->Unit() != DimensionUnit::INVALID;
    auto renderContext = paintWrapper->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto host = renderContext->GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    bool isRtl = layoutProperty->GetNonAutoLayoutDirection() == TextDirection::RTL;
    if (isRtl ? isEndMarginSet : isStartMarginSet) {
        info.startMargin = (isRtl ? endMarginOpt : startMarginOpt)->ConvertToPxWithSize(itemSize.Width());
    } else {
        info.startMargin = GetLeftRowStartEdge(paintWrapper, isRtl);
        if (info.startMargin < 0.0f || info.startMargin > itemSize.Width()) {
            auto defaultStart =
                selectTheme ? selectTheme->GetDefaultDividerStartMargin() : Dimension(0.0f, DimensionUnit::PX);
            info.startMargin = defaultStart.ConvertToPxWithSize(itemSize.Width());
        }
    }
    if (isRtl ? isStartMarginSet : isEndMarginSet) {
        info.endMargin = (isRtl ? startMarginOpt : endMarginOpt)->ConvertToPxWithSize(itemSize.Width());
    } else {
        auto rightEdge = GetRightRowEndEdge(paintWrapper, isRtl);
        if (rightEdge >= 0.0f && rightEdge <= itemSize.Width()) {
            info.endMargin = itemSize.Width() - rightEdge;
        } else {
            auto defaultEnd =
                selectTheme ? selectTheme->GetDefaultDividerEndMargin() : Dimension(0.0f, DimensionUnit::PX);
            info.endMargin = defaultEnd.ConvertToPxWithSize(itemSize.Width());
        }
    }
    path.AddRect(info.startMargin, info.topMargin, info.width - info.endMargin, info.topMargin + info.strokeWidth);
    DrawPathWithBrush(canvas, path, info.color);
}

void MenuItemPaintMethod::HandleOption(PaintWrapper* paintWrapper, const RefPtr<MenuItemPaintProperty>& props,
    const RefPtr<SelectTheme>& selectTheme, RSPath& path, RSCanvas& canvas)
{
    CHECK_NULL_VOID(selectTheme);
    auto renderContext = paintWrapper->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto optionNode = renderContext->GetHost();
    CHECK_NULL_VOID(optionNode);
    auto layoutProperty = optionNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);

    auto optionSize = paintWrapper->GetGeometryNode()->GetFrameSize();
    auto horInterval = static_cast<float>(selectTheme->GetMenuItemHorIntervalPadding().ConvertToPx());
    auto hasIcon = props->GetHasIcon().value_or(false);
    float iconHorInterval = 0.0f;
    auto textDirection = layoutProperty->GetNonAutoLayoutDirection();
    auto isRtl = (textDirection == TextDirection::RTL) ? true : false;
    if (hasIcon) {
        iconHorInterval = static_cast<float>(selectTheme->GetIconSideLength().ConvertToPx()) +
                          static_cast<float>(selectTheme->GetIconContentPadding().ConvertToPx());
    }

    // draw divider above content, length = content width
    if (props->HasDivider()) {
        PaintCustomDivider(optionSize, horInterval, iconHorInterval, paintWrapper, path);
    } else {
        auto startX = isRtl ? optionSize.Width() - (horInterval + iconHorInterval) : horInterval + iconHorInterval;
        auto rectWidth = isRtl ? horInterval : optionSize.Width() - horInterval;
        path.AddRect(startX, 0, rectWidth, static_cast<float>(selectTheme->GetDefaultDividerWidth().ConvertToPx()));
    }
    auto dividerColor = props->HasDivider() ? props->GetDividerValue().color : selectTheme->GetLineColor();
    DrawPathWithBrush(canvas, path, dividerColor);
}

void MenuItemPaintMethod::PaintCustomDivider(SizeF optionSize, float horInterval, float iconHorInterval,
    PaintWrapper* paintWrapper, RSPath& path)
{
    CHECK_NULL_VOID(paintWrapper);
    auto props = AceType::DynamicCast<MenuItemPaintProperty>(paintWrapper->GetPaintProperty());
    CHECK_NULL_VOID(props);
    auto dividerWidth = static_cast<float>(props->GetDividerValue().strokeWidth.ConvertToPx());
    auto startMargin = static_cast<float>(props->GetDividerValue().startMargin.ConvertToPx());
    auto endMargin = static_cast<float>(props->GetDividerValue().endMargin.ConvertToPx());
    if (startMargin < 0.0f || startMargin > optionSize.Width()) {
        startMargin = horInterval + iconHorInterval;
    }
    if (endMargin < 0.0f || endMargin > optionSize.Width()) {
        endMargin = horInterval;
    }
    if ((startMargin + endMargin) > optionSize.Width()) {
        startMargin = horInterval + iconHorInterval;
        endMargin = horInterval;
    }
    auto renderContext = paintWrapper->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto optionNode = renderContext->GetHost();
    CHECK_NULL_VOID(optionNode);
    auto layoutProperty = optionNode->GetLayoutProperty();
    CHECK_NULL_VOID(layoutProperty);
    auto textDirection = layoutProperty->GetNonAutoLayoutDirection();
    auto dividerRtl = static_cast<float>(props->GetDividerValue().isRtl);
    auto paintWidth = props->GetDividerValue().isDividerStyle ? dividerWidth / 2 : dividerWidth;
    dividerRtl = (textDirection == TextDirection::RTL) ? true : false;
    if (dividerRtl) {
        auto rtlStartMargin = startMargin;
        startMargin = endMargin;
        endMargin = rtlStartMargin;
    }
    path.AddRect(startMargin, -paintWidth, optionSize.Width() - endMargin, paintWidth);
}
} // namespace OHOS::Ace::NG
