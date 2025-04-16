/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "test_fixtures_enums.h"

#include <climits>

namespace OHOS::Ace::NG::Fixtures {
std::vector<std::tuple<std::string, Ark_AccessibilityRoleType, std::string>>
    testFixtureEnumAccessibilityRoleTypeValidValues = {
        { "ARK_ACCESSIBILITY_ROLE_TYPE_ACTION_SHEET", ARK_ACCESSIBILITY_ROLE_TYPE_ACTION_SHEET,
            "AccessibilityRoleType.ACTION_SHEET" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_ALERT_DIALOG", ARK_ACCESSIBILITY_ROLE_TYPE_ALERT_DIALOG,
            "AccessibilityRoleType.ALERT_DIALOG" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_INDEXER_COMPONENT", ARK_ACCESSIBILITY_ROLE_TYPE_INDEXER_COMPONENT,
            "AccessibilityRoleType.INDEXER_COMPONENT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_BADGE_COMPONENT", ARK_ACCESSIBILITY_ROLE_TYPE_BADGE_COMPONENT,
            "AccessibilityRoleType.BADGE_COMPONENT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_BLANK", ARK_ACCESSIBILITY_ROLE_TYPE_BLANK, "AccessibilityRoleType.BLANK" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_BUTTON", ARK_ACCESSIBILITY_ROLE_TYPE_BUTTON, "AccessibilityRoleType.BUTTON" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_BACK_BUTTON", ARK_ACCESSIBILITY_ROLE_TYPE_BACK_BUTTON,
            "AccessibilityRoleType.BACK_BUTTON" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_SHEET_DRAG_BAR", ARK_ACCESSIBILITY_ROLE_TYPE_SHEET_DRAG_BAR,
            "AccessibilityRoleType.SHEET_DRAG_BAR" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_CALENDAR_PICKER", ARK_ACCESSIBILITY_ROLE_TYPE_CALENDAR_PICKER,
            "AccessibilityRoleType.CALENDAR_PICKER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_CALENDAR", ARK_ACCESSIBILITY_ROLE_TYPE_CALENDAR,
            "AccessibilityRoleType.CALENDAR" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_CANVAS", ARK_ACCESSIBILITY_ROLE_TYPE_CANVAS, "AccessibilityRoleType.CANVAS" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_CANVAS_GRADIENT", ARK_ACCESSIBILITY_ROLE_TYPE_CANVAS_GRADIENT,
            "AccessibilityRoleType.CANVAS_GRADIENT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_CANVAS_PATTERN", ARK_ACCESSIBILITY_ROLE_TYPE_CANVAS_PATTERN,
            "AccessibilityRoleType.CANVAS_PATTERN" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_CHECKBOX", ARK_ACCESSIBILITY_ROLE_TYPE_CHECKBOX,
            "AccessibilityRoleType.CHECKBOX" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_CHECKBOX_GROUP", ARK_ACCESSIBILITY_ROLE_TYPE_CHECKBOX_GROUP,
            "AccessibilityRoleType.CHECKBOX_GROUP" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_CIRCLE", ARK_ACCESSIBILITY_ROLE_TYPE_CIRCLE, "AccessibilityRoleType.CIRCLE" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_COLUMN_SPLIT", ARK_ACCESSIBILITY_ROLE_TYPE_COLUMN_SPLIT,
            "AccessibilityRoleType.COLUMN_SPLIT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_COLUMN", ARK_ACCESSIBILITY_ROLE_TYPE_COLUMN, "AccessibilityRoleType.COLUMN" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_CANVAS_RENDERING_CONTEXT_2D",
            ARK_ACCESSIBILITY_ROLE_TYPE_CANVAS_RENDERING_CONTEXT_2D,
            "AccessibilityRoleType.CANVAS_RENDERING_CONTEXT_2D" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_CHART", ARK_ACCESSIBILITY_ROLE_TYPE_CHART, "AccessibilityRoleType.CHART" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_COUNTER", ARK_ACCESSIBILITY_ROLE_TYPE_COUNTER, "AccessibilityRoleType.COUNTER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_CONTAINER_MODAL", ARK_ACCESSIBILITY_ROLE_TYPE_CONTAINER_MODAL,
            "AccessibilityRoleType.CONTAINER_MODAL" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_DATA_PANEL", ARK_ACCESSIBILITY_ROLE_TYPE_DATA_PANEL,
            "AccessibilityRoleType.DATA_PANEL" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_DATE_PICKER", ARK_ACCESSIBILITY_ROLE_TYPE_DATE_PICKER,
            "AccessibilityRoleType.DATE_PICKER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_DIALOG", ARK_ACCESSIBILITY_ROLE_TYPE_DIALOG, "AccessibilityRoleType.DIALOG" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_DIVIDER", ARK_ACCESSIBILITY_ROLE_TYPE_DIVIDER, "AccessibilityRoleType.DIVIDER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_DRAG_BAR", ARK_ACCESSIBILITY_ROLE_TYPE_DRAG_BAR,
            "AccessibilityRoleType.DRAG_BAR" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_EFFECT_COMPONENT", ARK_ACCESSIBILITY_ROLE_TYPE_EFFECT_COMPONENT,
            "AccessibilityRoleType.EFFECT_COMPONENT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_ELLIPSE", ARK_ACCESSIBILITY_ROLE_TYPE_ELLIPSE, "AccessibilityRoleType.ELLIPSE" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_FLEX", ARK_ACCESSIBILITY_ROLE_TYPE_FLEX, "AccessibilityRoleType.FLEX" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_FLOW_ITEM", ARK_ACCESSIBILITY_ROLE_TYPE_FLOW_ITEM,
            "AccessibilityRoleType.FLOW_ITEM" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_FORM_COMPONENT", ARK_ACCESSIBILITY_ROLE_TYPE_FORM_COMPONENT,
            "AccessibilityRoleType.FORM_COMPONENT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_FORM_LINK", ARK_ACCESSIBILITY_ROLE_TYPE_FORM_LINK,
            "AccessibilityRoleType.FORM_LINK" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_GAUGE", ARK_ACCESSIBILITY_ROLE_TYPE_GAUGE, "AccessibilityRoleType.GAUGE" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_GRID", ARK_ACCESSIBILITY_ROLE_TYPE_GRID, "AccessibilityRoleType.GRID" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_GRID_COL", ARK_ACCESSIBILITY_ROLE_TYPE_GRID_COL,
            "AccessibilityRoleType.GRID_COL" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_GRID_CONTAINER", ARK_ACCESSIBILITY_ROLE_TYPE_GRID_CONTAINER,
            "AccessibilityRoleType.GRID_CONTAINER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_GRID_ITEM", ARK_ACCESSIBILITY_ROLE_TYPE_GRID_ITEM,
            "AccessibilityRoleType.GRID_ITEM" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_GRID_ROW", ARK_ACCESSIBILITY_ROLE_TYPE_GRID_ROW,
            "AccessibilityRoleType.GRID_ROW" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_HYPERLINK", ARK_ACCESSIBILITY_ROLE_TYPE_HYPERLINK,
            "AccessibilityRoleType.HYPERLINK" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_IMAGE", ARK_ACCESSIBILITY_ROLE_TYPE_IMAGE, "AccessibilityRoleType.IMAGE" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_IMAGE_ANIMATOR", ARK_ACCESSIBILITY_ROLE_TYPE_IMAGE_ANIMATOR,
            "AccessibilityRoleType.IMAGE_ANIMATOR" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_IMAGE_BITMAP", ARK_ACCESSIBILITY_ROLE_TYPE_IMAGE_BITMAP,
            "AccessibilityRoleType.IMAGE_BITMAP" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_IMAGE_DATA", ARK_ACCESSIBILITY_ROLE_TYPE_IMAGE_DATA,
            "AccessibilityRoleType.IMAGE_DATA" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_IMAGE_SPAN", ARK_ACCESSIBILITY_ROLE_TYPE_IMAGE_SPAN,
            "AccessibilityRoleType.IMAGE_SPAN" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_LABEL", ARK_ACCESSIBILITY_ROLE_TYPE_LABEL, "AccessibilityRoleType.LABEL" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_LINE", ARK_ACCESSIBILITY_ROLE_TYPE_LINE, "AccessibilityRoleType.LINE" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_LIST", ARK_ACCESSIBILITY_ROLE_TYPE_LIST, "AccessibilityRoleType.LIST" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_LIST_ITEM", ARK_ACCESSIBILITY_ROLE_TYPE_LIST_ITEM,
            "AccessibilityRoleType.LIST_ITEM" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_LIST_ITEM_GROUP", ARK_ACCESSIBILITY_ROLE_TYPE_LIST_ITEM_GROUP,
            "AccessibilityRoleType.LIST_ITEM_GROUP" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_LOADING_PROGRESS", ARK_ACCESSIBILITY_ROLE_TYPE_LOADING_PROGRESS,
            "AccessibilityRoleType.LOADING_PROGRESS" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_MARQUEE", ARK_ACCESSIBILITY_ROLE_TYPE_MARQUEE, "AccessibilityRoleType.MARQUEE" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_MATRIX2D", ARK_ACCESSIBILITY_ROLE_TYPE_MATRIX2D,
            "AccessibilityRoleType.MATRIX2D" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_MENU", ARK_ACCESSIBILITY_ROLE_TYPE_MENU, "AccessibilityRoleType.MENU" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_MENU_ITEM", ARK_ACCESSIBILITY_ROLE_TYPE_MENU_ITEM,
            "AccessibilityRoleType.MENU_ITEM" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_MENU_ITEM_GROUP", ARK_ACCESSIBILITY_ROLE_TYPE_MENU_ITEM_GROUP,
            "AccessibilityRoleType.MENU_ITEM_GROUP" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_NAV_DESTINATION", ARK_ACCESSIBILITY_ROLE_TYPE_NAV_DESTINATION,
            "AccessibilityRoleType.NAV_DESTINATION" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_NAV_ROUTER", ARK_ACCESSIBILITY_ROLE_TYPE_NAV_ROUTER,
            "AccessibilityRoleType.NAV_ROUTER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_NAVIGATION", ARK_ACCESSIBILITY_ROLE_TYPE_NAVIGATION,
            "AccessibilityRoleType.NAVIGATION" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_NAVIGATION_BAR", ARK_ACCESSIBILITY_ROLE_TYPE_NAVIGATION_BAR,
            "AccessibilityRoleType.NAVIGATION_BAR" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_NAVIGATION_MENU", ARK_ACCESSIBILITY_ROLE_TYPE_NAVIGATION_MENU,
            "AccessibilityRoleType.NAVIGATION_MENU" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_NAVIGATOR", ARK_ACCESSIBILITY_ROLE_TYPE_NAVIGATOR,
            "AccessibilityRoleType.NAVIGATOR" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_OFFSCREEN_CANVAS", ARK_ACCESSIBILITY_ROLE_TYPE_OFFSCREEN_CANVAS,
            "AccessibilityRoleType.OFFSCREEN_CANVAS" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_OFFSCREEN_CANVAS_RENDERING_CONTEXT2D",
            ARK_ACCESSIBILITY_ROLE_TYPE_OFFSCREEN_CANVAS_RENDERING_CONTEXT2D,
            "AccessibilityRoleType.OFFSCREEN_CANVAS_RENDERING_CONTEXT2D" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_OPTION", ARK_ACCESSIBILITY_ROLE_TYPE_OPTION, "AccessibilityRoleType.OPTION" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_PANEL", ARK_ACCESSIBILITY_ROLE_TYPE_PANEL, "AccessibilityRoleType.PANEL" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_PAPER_PAGE", ARK_ACCESSIBILITY_ROLE_TYPE_PAPER_PAGE,
            "AccessibilityRoleType.PAPER_PAGE" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_PATH", ARK_ACCESSIBILITY_ROLE_TYPE_PATH, "AccessibilityRoleType.PATH" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_PATH2D", ARK_ACCESSIBILITY_ROLE_TYPE_PATH2D, "AccessibilityRoleType.PATH2D" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_PATTERN_LOCK", ARK_ACCESSIBILITY_ROLE_TYPE_PATTERN_LOCK,
            "AccessibilityRoleType.PATTERN_LOCK" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_PICKER", ARK_ACCESSIBILITY_ROLE_TYPE_PICKER, "AccessibilityRoleType.PICKER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_PICKER_VIEW", ARK_ACCESSIBILITY_ROLE_TYPE_PICKER_VIEW,
            "AccessibilityRoleType.PICKER_VIEW" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_PLUGIN_COMPONENT", ARK_ACCESSIBILITY_ROLE_TYPE_PLUGIN_COMPONENT,
            "AccessibilityRoleType.PLUGIN_COMPONENT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_POLYGON", ARK_ACCESSIBILITY_ROLE_TYPE_POLYGON, "AccessibilityRoleType.POLYGON" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_POLYLINE", ARK_ACCESSIBILITY_ROLE_TYPE_POLYLINE,
            "AccessibilityRoleType.POLYLINE" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_POPUP", ARK_ACCESSIBILITY_ROLE_TYPE_POPUP, "AccessibilityRoleType.POPUP" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_PROGRESS", ARK_ACCESSIBILITY_ROLE_TYPE_PROGRESS,
            "AccessibilityRoleType.PROGRESS" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_QRCODE", ARK_ACCESSIBILITY_ROLE_TYPE_QRCODE, "AccessibilityRoleType.QRCODE" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_RADIO", ARK_ACCESSIBILITY_ROLE_TYPE_RADIO, "AccessibilityRoleType.RADIO" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_RATING", ARK_ACCESSIBILITY_ROLE_TYPE_RATING, "AccessibilityRoleType.RATING" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_RECT", ARK_ACCESSIBILITY_ROLE_TYPE_RECT, "AccessibilityRoleType.RECT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_REFRESH", ARK_ACCESSIBILITY_ROLE_TYPE_REFRESH, "AccessibilityRoleType.REFRESH" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_RELATIVE_CONTAINER", ARK_ACCESSIBILITY_ROLE_TYPE_RELATIVE_CONTAINER,
            "AccessibilityRoleType.RELATIVE_CONTAINER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_REMOTE_WINDOW", ARK_ACCESSIBILITY_ROLE_TYPE_REMOTE_WINDOW,
            "AccessibilityRoleType.REMOTE_WINDOW" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_RICH_EDITOR", ARK_ACCESSIBILITY_ROLE_TYPE_RICH_EDITOR,
            "AccessibilityRoleType.RICH_EDITOR" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_RICH_TEXT", ARK_ACCESSIBILITY_ROLE_TYPE_RICH_TEXT,
            "AccessibilityRoleType.RICH_TEXT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_ROLE_PAGER", ARK_ACCESSIBILITY_ROLE_TYPE_ROLE_PAGER,
            "AccessibilityRoleType.ROLE_PAGER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_ROW", ARK_ACCESSIBILITY_ROLE_TYPE_ROW, "AccessibilityRoleType.ROW" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_ROW_SPLIT", ARK_ACCESSIBILITY_ROLE_TYPE_ROW_SPLIT,
            "AccessibilityRoleType.ROW_SPLIT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_SCROLL", ARK_ACCESSIBILITY_ROLE_TYPE_SCROLL, "AccessibilityRoleType.SCROLL" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_SCROLL_BAR", ARK_ACCESSIBILITY_ROLE_TYPE_SCROLL_BAR,
            "AccessibilityRoleType.SCROLL_BAR" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_SEARCH", ARK_ACCESSIBILITY_ROLE_TYPE_SEARCH, "AccessibilityRoleType.SEARCH" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_SEARCH_FIELD", ARK_ACCESSIBILITY_ROLE_TYPE_SEARCH_FIELD,
            "AccessibilityRoleType.SEARCH_FIELD" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_SELECT", ARK_ACCESSIBILITY_ROLE_TYPE_SELECT, "AccessibilityRoleType.SELECT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_SHAPE", ARK_ACCESSIBILITY_ROLE_TYPE_SHAPE, "AccessibilityRoleType.SHAPE" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_SIDEBAR_CONTAINER", ARK_ACCESSIBILITY_ROLE_TYPE_SIDEBAR_CONTAINER,
            "AccessibilityRoleType.SIDEBAR_CONTAINER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_SLIDER", ARK_ACCESSIBILITY_ROLE_TYPE_SLIDER, "AccessibilityRoleType.SLIDER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_SPAN", ARK_ACCESSIBILITY_ROLE_TYPE_SPAN, "AccessibilityRoleType.SPAN" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_STACK", ARK_ACCESSIBILITY_ROLE_TYPE_STACK, "AccessibilityRoleType.STACK" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_STEPPER", ARK_ACCESSIBILITY_ROLE_TYPE_STEPPER, "AccessibilityRoleType.STEPPER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_STEPPER_ITEM", ARK_ACCESSIBILITY_ROLE_TYPE_STEPPER_ITEM,
            "AccessibilityRoleType.STEPPER_ITEM" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_SWIPER", ARK_ACCESSIBILITY_ROLE_TYPE_SWIPER, "AccessibilityRoleType.SWIPER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_SWIPER_INDICATOR", ARK_ACCESSIBILITY_ROLE_TYPE_SWIPER_INDICATOR,
            "AccessibilityRoleType.SWIPER_INDICATOR" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_SWITCH", ARK_ACCESSIBILITY_ROLE_TYPE_SWITCH, "AccessibilityRoleType.SWITCH" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_SYMBOL_GLYPH", ARK_ACCESSIBILITY_ROLE_TYPE_SYMBOL_GLYPH,
            "AccessibilityRoleType.SYMBOL_GLYPH" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_TAB_CONTENT", ARK_ACCESSIBILITY_ROLE_TYPE_TAB_CONTENT,
            "AccessibilityRoleType.TAB_CONTENT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_TAB_BAR", ARK_ACCESSIBILITY_ROLE_TYPE_TAB_BAR, "AccessibilityRoleType.TAB_BAR" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_TABS", ARK_ACCESSIBILITY_ROLE_TYPE_TABS, "AccessibilityRoleType.TABS" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_TEXT", ARK_ACCESSIBILITY_ROLE_TYPE_TEXT, "AccessibilityRoleType.TEXT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_TEXT_CLOCK", ARK_ACCESSIBILITY_ROLE_TYPE_TEXT_CLOCK,
            "AccessibilityRoleType.TEXT_CLOCK" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_TEXT_ENTRY", ARK_ACCESSIBILITY_ROLE_TYPE_TEXT_ENTRY,
            "AccessibilityRoleType.TEXT_ENTRY" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_TEXT_INPUT", ARK_ACCESSIBILITY_ROLE_TYPE_TEXT_INPUT,
            "AccessibilityRoleType.TEXT_INPUT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_TEXT_PICKER", ARK_ACCESSIBILITY_ROLE_TYPE_TEXT_PICKER,
            "AccessibilityRoleType.TEXT_PICKER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_TEXT_TIMER", ARK_ACCESSIBILITY_ROLE_TYPE_TEXT_TIMER,
            "AccessibilityRoleType.TEXT_TIMER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_TEXT_AREA", ARK_ACCESSIBILITY_ROLE_TYPE_TEXT_AREA,
            "AccessibilityRoleType.TEXT_AREA" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_TEXT_FIELD", ARK_ACCESSIBILITY_ROLE_TYPE_TEXT_FIELD,
            "AccessibilityRoleType.TEXT_FIELD" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_TIME_PICKER", ARK_ACCESSIBILITY_ROLE_TYPE_TIME_PICKER,
            "AccessibilityRoleType.TIME_PICKER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_TITLE_BAR", ARK_ACCESSIBILITY_ROLE_TYPE_TITLE_BAR,
            "AccessibilityRoleType.TITLE_BAR" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_TOGGLER", ARK_ACCESSIBILITY_ROLE_TYPE_TOGGLER, "AccessibilityRoleType.TOGGLER" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_UI_EXTENSION_COMPONENT", ARK_ACCESSIBILITY_ROLE_TYPE_UI_EXTENSION_COMPONENT,
            "AccessibilityRoleType.UI_EXTENSION_COMPONENT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_VIDEO", ARK_ACCESSIBILITY_ROLE_TYPE_VIDEO, "AccessibilityRoleType.VIDEO" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_WATER_FLOW", ARK_ACCESSIBILITY_ROLE_TYPE_WATER_FLOW,
            "AccessibilityRoleType.WATER_FLOW" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_WEB", ARK_ACCESSIBILITY_ROLE_TYPE_WEB, "AccessibilityRoleType.WEB" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_XCOMPONENT", ARK_ACCESSIBILITY_ROLE_TYPE_XCOMPONENT,
            "AccessibilityRoleType.XCOMPONENT" },
        { "ARK_ACCESSIBILITY_ROLE_TYPE_ROLE_NONE", ARK_ACCESSIBILITY_ROLE_TYPE_ROLE_NONE,
            "AccessibilityRoleType.ROLE_NONE" },
    };

std::vector<std::tuple<std::string, Ark_AccessibilityRoleType>> testFixtureEnumAccessibilityRoleTypeInvalidValues = {
    { "-1", static_cast<Ark_AccessibilityRoleType>(-1) },
    { "INT_MAX", static_cast<Ark_AccessibilityRoleType>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_AccessibilitySamePageMode, std::string>>
    testFixtureEnumAccessibilitySamePageModeValidValues = {
        { "ARK_ACCESSIBILITY_SAME_PAGE_MODE_SEMI_SILENT", ARK_ACCESSIBILITY_SAME_PAGE_MODE_SEMI_SILENT,
            "AccessibilitySamePageMode.SEMI_SILENT" },
        { "ARK_ACCESSIBILITY_SAME_PAGE_MODE_FULL_SILENT", ARK_ACCESSIBILITY_SAME_PAGE_MODE_FULL_SILENT,
            "AccessibilitySamePageMode.FULL_SILENT" },
    };

std::vector<std::tuple<std::string, Ark_AccessibilitySamePageMode>>
    testFixtureEnumAccessibilitySamePageModeInvalidValues = {
        { "-1", static_cast<Ark_AccessibilitySamePageMode>(-1) },
        { "INT_MAX", static_cast<Ark_AccessibilitySamePageMode>(INT_MAX) },
    };

std::vector<std::tuple<std::string, Ark_AdaptiveColor, std::string>> testFixtureEnumAdaptiveColorValidValues = {
    { "ARK_ADAPTIVE_COLOR_DEFAULT", ARK_ADAPTIVE_COLOR_DEFAULT, "AdaptiveColor.DEFAULT" },
    { "ARK_ADAPTIVE_COLOR_AVERAGE", ARK_ADAPTIVE_COLOR_AVERAGE, "AdaptiveColor.AVERAGE" },
};

std::vector<std::tuple<std::string, Ark_AdaptiveColor>> testFixtureEnumAdaptiveColorInvalidValues = {
    { "-1", static_cast<Ark_AdaptiveColor>(-1) },
    { "INT_MAX", static_cast<Ark_AdaptiveColor>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_Alignment, std::string>> testFixtureEnumAlignmentValidValues = {
    { "ARK_ALIGNMENT_TOP_START", ARK_ALIGNMENT_TOP_START, "Alignment.TopStart" },
    { "ARK_ALIGNMENT_TOP", ARK_ALIGNMENT_TOP, "Alignment.Top" },
    { "ARK_ALIGNMENT_TOP_END", ARK_ALIGNMENT_TOP_END, "Alignment.TopEnd" },
    { "ARK_ALIGNMENT_START", ARK_ALIGNMENT_START, "Alignment.Start" },
    { "ARK_ALIGNMENT_CENTER", ARK_ALIGNMENT_CENTER, "Alignment.Center" },
    { "ARK_ALIGNMENT_END", ARK_ALIGNMENT_END, "Alignment.End" },
    { "ARK_ALIGNMENT_BOTTOM_START", ARK_ALIGNMENT_BOTTOM_START, "Alignment.BottomStart" },
    { "ARK_ALIGNMENT_BOTTOM", ARK_ALIGNMENT_BOTTOM, "Alignment.Bottom" },
    { "ARK_ALIGNMENT_BOTTOM_END", ARK_ALIGNMENT_BOTTOM_END, "Alignment.BottomEnd" },
};

std::vector<std::tuple<std::string, Ark_Alignment>> testFixtureEnumAlignmentInvalidValues = {
    { "-1", static_cast<Ark_Alignment>(-1) },
    { "INT_MAX", static_cast<Ark_Alignment>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_AnimationStatus, std::string>> testFixtureEnumAnimationStatusValidValues = {
    { "ARK_ANIMATION_STATUS_INITIAL", ARK_ANIMATION_STATUS_INITIAL, "AnimationStatus.Initial" },
    { "ARK_ANIMATION_STATUS_RUNNING", ARK_ANIMATION_STATUS_RUNNING, "AnimationStatus.Running" },
    { "ARK_ANIMATION_STATUS_PAUSED", ARK_ANIMATION_STATUS_PAUSED, "AnimationStatus.Paused" },
    { "ARK_ANIMATION_STATUS_STOPPED", ARK_ANIMATION_STATUS_STOPPED, "AnimationStatus.Stopped" },
};

std::vector<std::tuple<std::string, Ark_AnimationStatus>> testFixtureEnumAnimationStatusInvalidValues = {
    { "-1", static_cast<Ark_AnimationStatus>(-1) },
    { "INT_MAX", static_cast<Ark_AnimationStatus>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ArrowPointPosition, std::string>>
    testFixtureEnumArrowPointPositionValidValues = {
        { "ARK_ARROW_POINT_POSITION_START", ARK_ARROW_POINT_POSITION_START, "ArrowPointPosition.START" },
        { "ARK_ARROW_POINT_POSITION_CENTER", ARK_ARROW_POINT_POSITION_CENTER, "ArrowPointPosition.CENTER" },
        { "ARK_ARROW_POINT_POSITION_END", ARK_ARROW_POINT_POSITION_END, "ArrowPointPosition.END" },
    };

std::vector<std::tuple<std::string, Ark_ArrowPointPosition>> testFixtureEnumArrowPointPositionInvalidValues = {
    { "-1", static_cast<Ark_ArrowPointPosition>(-1) },
    { "INT_MAX", static_cast<Ark_ArrowPointPosition>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_Axis, std::string>> testFixtureEnumAxisValidValues = {
    { "ARK_AXIS_VERTICAL", ARK_AXIS_VERTICAL, "Axis.Vertical" },
    { "ARK_AXIS_HORIZONTAL", ARK_AXIS_HORIZONTAL, "Axis.Horizontal" },
};

std::vector<std::tuple<std::string, Ark_Axis>> testFixtureEnumAxisInvalidValues = {
    { "-1", static_cast<Ark_Axis>(-1) },
    { "INT_MAX", static_cast<Ark_Axis>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_BarState, std::string>> testFixtureEnumBarStateValidValues = {
    { "ARK_BAR_STATE_OFF", ARK_BAR_STATE_OFF, "BarState.Off" },
    { "ARK_BAR_STATE_AUTO", ARK_BAR_STATE_AUTO, "BarState.Auto" },
    { "ARK_BAR_STATE_ON", ARK_BAR_STATE_ON, "BarState.On" },
};

std::vector<std::tuple<std::string, Ark_BarState>> testFixtureEnumBarStateInvalidValues = {
    { "-1", static_cast<Ark_BarState>(-1) },
    { "INT_MAX", static_cast<Ark_BarState>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_BlendApplyType, std::string>> testFixtureEnumBlendApplyTypeValidValues = {
    { "ARK_BLEND_APPLY_TYPE_FAST", ARK_BLEND_APPLY_TYPE_FAST, "BlendApplyType.FAST" },
    { "ARK_BLEND_APPLY_TYPE_OFFSCREEN", ARK_BLEND_APPLY_TYPE_OFFSCREEN, "BlendApplyType.OFFSCREEN" },
};

std::vector<std::tuple<std::string, Ark_BlendApplyType>> testFixtureEnumBlendApplyTypeInvalidValues = {
    { "-1", static_cast<Ark_BlendApplyType>(-1) },
    { "INT_MAX", static_cast<Ark_BlendApplyType>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_BlendMode, std::string>> testFixtureEnumBlendModeValidValues = {
    { "ARK_BLEND_MODE_NONE", ARK_BLEND_MODE_NONE, "BlendMode.NONE" },
    { "ARK_BLEND_MODE_CLEAR", ARK_BLEND_MODE_CLEAR, "BlendMode.CLEAR" },
    { "ARK_BLEND_MODE_SRC", ARK_BLEND_MODE_SRC, "BlendMode.SRC" },
    { "ARK_BLEND_MODE_DST", ARK_BLEND_MODE_DST, "BlendMode.DST" },
    { "ARK_BLEND_MODE_SRC_OVER", ARK_BLEND_MODE_SRC_OVER, "BlendMode.SRC_OVER" },
    { "ARK_BLEND_MODE_DST_OVER", ARK_BLEND_MODE_DST_OVER, "BlendMode.DST_OVER" },
    { "ARK_BLEND_MODE_SRC_IN", ARK_BLEND_MODE_SRC_IN, "BlendMode.SRC_IN" },
    { "ARK_BLEND_MODE_DST_IN", ARK_BLEND_MODE_DST_IN, "BlendMode.DST_IN" },
    { "ARK_BLEND_MODE_SRC_OUT", ARK_BLEND_MODE_SRC_OUT, "BlendMode.SRC_OUT" },
    { "ARK_BLEND_MODE_DST_OUT", ARK_BLEND_MODE_DST_OUT, "BlendMode.DST_OUT" },
    { "ARK_BLEND_MODE_SRC_ATOP", ARK_BLEND_MODE_SRC_ATOP, "BlendMode.SRC_ATOP" },
    { "ARK_BLEND_MODE_DST_ATOP", ARK_BLEND_MODE_DST_ATOP, "BlendMode.DST_ATOP" },
    { "ARK_BLEND_MODE_XOR", ARK_BLEND_MODE_XOR, "BlendMode.XOR" },
    { "ARK_BLEND_MODE_PLUS", ARK_BLEND_MODE_PLUS, "BlendMode.PLUS" },
    { "ARK_BLEND_MODE_MODULATE", ARK_BLEND_MODE_MODULATE, "BlendMode.MODULATE" },
    { "ARK_BLEND_MODE_SCREEN", ARK_BLEND_MODE_SCREEN, "BlendMode.SCREEN" },
    { "ARK_BLEND_MODE_OVERLAY", ARK_BLEND_MODE_OVERLAY, "BlendMode.OVERLAY" },
    { "ARK_BLEND_MODE_DARKEN", ARK_BLEND_MODE_DARKEN, "BlendMode.DARKEN" },
    { "ARK_BLEND_MODE_LIGHTEN", ARK_BLEND_MODE_LIGHTEN, "BlendMode.LIGHTEN" },
    { "ARK_BLEND_MODE_COLOR_DODGE", ARK_BLEND_MODE_COLOR_DODGE, "BlendMode.COLOR_DODGE" },
    { "ARK_BLEND_MODE_COLOR_BURN", ARK_BLEND_MODE_COLOR_BURN, "BlendMode.COLOR_BURN" },
    { "ARK_BLEND_MODE_HARD_LIGHT", ARK_BLEND_MODE_HARD_LIGHT, "BlendMode.HARD_LIGHT" },
    { "ARK_BLEND_MODE_SOFT_LIGHT", ARK_BLEND_MODE_SOFT_LIGHT, "BlendMode.SOFT_LIGHT" },
    { "ARK_BLEND_MODE_DIFFERENCE", ARK_BLEND_MODE_DIFFERENCE, "BlendMode.DIFFERENCE" },
    { "ARK_BLEND_MODE_EXCLUSION", ARK_BLEND_MODE_EXCLUSION, "BlendMode.EXCLUSION" },
    { "ARK_BLEND_MODE_MULTIPLY", ARK_BLEND_MODE_MULTIPLY, "BlendMode.MULTIPLY" },
    { "ARK_BLEND_MODE_HUE", ARK_BLEND_MODE_HUE, "BlendMode.HUE" },
    { "ARK_BLEND_MODE_SATURATION", ARK_BLEND_MODE_SATURATION, "BlendMode.SATURATION" },
    { "ARK_BLEND_MODE_COLOR", ARK_BLEND_MODE_COLOR, "BlendMode.COLOR" },
    { "ARK_BLEND_MODE_LUMINOSITY", ARK_BLEND_MODE_LUMINOSITY, "BlendMode.LUMINOSITY" },
};

std::vector<std::tuple<std::string, Ark_BlendMode>> testFixtureEnumBlendModeInvalidValues = {
    { "-1", static_cast<Ark_BlendMode>(-1) },
    { "INT_MAX", static_cast<Ark_BlendMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_BlurOnKeyboardHideMode, std::string>>
    testFixtureEnumBlurOnKeyboardHideModeValidValues = {
        { "ARK_BLUR_ON_KEYBOARD_HIDE_MODE_SILENT", ARK_BLUR_ON_KEYBOARD_HIDE_MODE_SILENT,
            "BlurOnKeyboardHideMode.SILENT" },
        { "ARK_BLUR_ON_KEYBOARD_HIDE_MODE_BLUR", ARK_BLUR_ON_KEYBOARD_HIDE_MODE_BLUR, "BlurOnKeyboardHideMode.BLUR" },
    };

std::vector<std::tuple<std::string, Ark_BlurOnKeyboardHideMode>> testFixtureEnumBlurOnKeyboardHideModeInvalidValues = {
    { "-1", static_cast<Ark_BlurOnKeyboardHideMode>(-1) },
    { "INT_MAX", static_cast<Ark_BlurOnKeyboardHideMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_BlurStyle, std::string>> testFixtureEnumBlurStyleValidValues = {
    { "ARK_BLUR_STYLE_THIN", ARK_BLUR_STYLE_THIN, "BlurStyle.Thin" },
    { "ARK_BLUR_STYLE_REGULAR", ARK_BLUR_STYLE_REGULAR, "BlurStyle.Regular" },
    { "ARK_BLUR_STYLE_THICK", ARK_BLUR_STYLE_THICK, "BlurStyle.Thick" },
    { "ARK_BLUR_STYLE_BACKGROUND_THIN", ARK_BLUR_STYLE_BACKGROUND_THIN, "BlurStyle.BACKGROUND_THIN" },
    { "ARK_BLUR_STYLE_BACKGROUND_REGULAR", ARK_BLUR_STYLE_BACKGROUND_REGULAR, "BlurStyle.BACKGROUND_REGULAR" },
    { "ARK_BLUR_STYLE_BACKGROUND_THICK", ARK_BLUR_STYLE_BACKGROUND_THICK, "BlurStyle.BACKGROUND_THICK" },
    { "ARK_BLUR_STYLE_BACKGROUND_ULTRA_THICK", ARK_BLUR_STYLE_BACKGROUND_ULTRA_THICK,
        "BlurStyle.BACKGROUND_ULTRA_THICK" },
    { "ARK_BLUR_STYLE_NONE", ARK_BLUR_STYLE_NONE, "BlurStyle.NONE" },
    { "ARK_BLUR_STYLE_COMPONENT_ULTRA_THIN", ARK_BLUR_STYLE_COMPONENT_ULTRA_THIN, "BlurStyle.COMPONENT_ULTRA_THIN" },
    { "ARK_BLUR_STYLE_COMPONENT_THIN", ARK_BLUR_STYLE_COMPONENT_THIN, "BlurStyle.COMPONENT_THIN" },
    { "ARK_BLUR_STYLE_COMPONENT_REGULAR", ARK_BLUR_STYLE_COMPONENT_REGULAR, "BlurStyle.COMPONENT_REGULAR" },
    { "ARK_BLUR_STYLE_COMPONENT_THICK", ARK_BLUR_STYLE_COMPONENT_THICK, "BlurStyle.COMPONENT_THICK" },
    { "ARK_BLUR_STYLE_COMPONENT_ULTRA_THICK", ARK_BLUR_STYLE_COMPONENT_ULTRA_THICK, "BlurStyle.COMPONENT_ULTRA_THICK" },
};

std::vector<std::tuple<std::string, Ark_BlurStyle>> testFixtureEnumBlurStyleInvalidValues = {
    { "-1", static_cast<Ark_BlurStyle>(-1) },
    { "INT_MAX", static_cast<Ark_BlurStyle>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_BlurStyleActivePolicy, std::string>>
    testFixtureEnumBlurStyleActivePolicyValidValues = {
        { "ARK_BLUR_STYLE_ACTIVE_POLICY_FOLLOWS_WINDOW_ACTIVE_STATE",
            ARK_BLUR_STYLE_ACTIVE_POLICY_FOLLOWS_WINDOW_ACTIVE_STATE,
            "BlurStyleActivePolicy.FOLLOWS_WINDOW_ACTIVE_STATE" },
        { "ARK_BLUR_STYLE_ACTIVE_POLICY_ALWAYS_ACTIVE", ARK_BLUR_STYLE_ACTIVE_POLICY_ALWAYS_ACTIVE,
            "BlurStyleActivePolicy.ALWAYS_ACTIVE" },
        { "ARK_BLUR_STYLE_ACTIVE_POLICY_ALWAYS_INACTIVE", ARK_BLUR_STYLE_ACTIVE_POLICY_ALWAYS_INACTIVE,
            "BlurStyleActivePolicy.ALWAYS_INACTIVE" },
    };

std::vector<std::tuple<std::string, Ark_BlurStyleActivePolicy>> testFixtureEnumBlurStyleActivePolicyInvalidValues = {
    { "-1", static_cast<Ark_BlurStyleActivePolicy>(-1) },
    { "INT_MAX", static_cast<Ark_BlurStyleActivePolicy>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_BorderStyle, std::string>> testFixtureEnumBorderStyleValidValues = {
    { "ARK_BORDER_STYLE_DOTTED", ARK_BORDER_STYLE_DOTTED, "BorderStyle.Dotted" },
    { "ARK_BORDER_STYLE_DASHED", ARK_BORDER_STYLE_DASHED, "BorderStyle.Dashed" },
    { "ARK_BORDER_STYLE_SOLID", ARK_BORDER_STYLE_SOLID, "BorderStyle.Solid" },
};

std::vector<std::tuple<std::string, Ark_BorderStyle>> testFixtureEnumBorderStyleInvalidValues = {
    { "-1", static_cast<Ark_BorderStyle>(-1) },
    { "INT_MAX", static_cast<Ark_BorderStyle>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_BreakpointsReference, std::string>>
    testFixtureEnumBreakpointsReferenceValidValues = {
        { "ARK_BREAKPOINTS_REFERENCE_WINDOW_SIZE", ARK_BREAKPOINTS_REFERENCE_WINDOW_SIZE,
            "BreakpointsReference.WindowSize" },
        { "ARK_BREAKPOINTS_REFERENCE_COMPONENT_SIZE", ARK_BREAKPOINTS_REFERENCE_COMPONENT_SIZE,
            "BreakpointsReference.ComponentSize" },
    };

std::vector<std::tuple<std::string, Ark_BreakpointsReference>> testFixtureEnumBreakpointsReferenceInvalidValues = {
    { "-1", static_cast<Ark_BreakpointsReference>(-1) },
    { "INT_MAX", static_cast<Ark_BreakpointsReference>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ButtonRole, std::string>> testFixtureEnumButtonRoleValidValues = {
    { "ARK_BUTTON_ROLE_NORMAL", ARK_BUTTON_ROLE_NORMAL, "ButtonRole.NORMAL" },
    { "ARK_BUTTON_ROLE_ERROR", ARK_BUTTON_ROLE_ERROR, "ButtonRole.ERROR" },
};

std::vector<std::tuple<std::string, Ark_ButtonRole>> testFixtureEnumButtonRoleInvalidValues = {
    { "-1", static_cast<Ark_ButtonRole>(-1) },
    { "INT_MAX", static_cast<Ark_ButtonRole>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ButtonStyleMode, std::string>> testFixtureEnumButtonStyleModeValidValues = {
    { "ARK_BUTTON_STYLE_MODE_NORMAL", ARK_BUTTON_STYLE_MODE_NORMAL, "ButtonStyleMode.NORMAL" },
    { "ARK_BUTTON_STYLE_MODE_EMPHASIZED", ARK_BUTTON_STYLE_MODE_EMPHASIZED, "ButtonStyleMode.EMPHASIZED" },
    { "ARK_BUTTON_STYLE_MODE_TEXTUAL", ARK_BUTTON_STYLE_MODE_TEXTUAL, "ButtonStyleMode.TEXTUAL" },
};

std::vector<std::tuple<std::string, Ark_ButtonStyleMode>> testFixtureEnumButtonStyleModeInvalidValues = {
    { "-1", static_cast<Ark_ButtonStyleMode>(-1) },
    { "INT_MAX", static_cast<Ark_ButtonStyleMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ButtonType, std::string>> testFixtureEnumButtonTypeValidValues = {
    { "ARK_BUTTON_TYPE_CAPSULE", ARK_BUTTON_TYPE_CAPSULE, "ButtonType.Capsule" },
    { "ARK_BUTTON_TYPE_CIRCLE", ARK_BUTTON_TYPE_CIRCLE, "ButtonType.Circle" },
    { "ARK_BUTTON_TYPE_NORMAL", ARK_BUTTON_TYPE_NORMAL, "ButtonType.Normal" },
    { "ARK_BUTTON_TYPE_ROUNDED_RECTANGLE", ARK_BUTTON_TYPE_ROUNDED_RECTANGLE, "ButtonType.ROUNDED_RECTANGLE" },
};

std::vector<std::tuple<std::string, Ark_ButtonType>> testFixtureEnumButtonTypeInvalidValues = {
    { "-1", static_cast<Ark_ButtonType>(-1) },
    { "INT_MAX", static_cast<Ark_ButtonType>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_CacheMode, std::string>> testFixtureEnumCacheModeValidValues = {
    { "ARK_CACHE_MODE_LEGACY_DEFAULT", ARK_CACHE_MODE_LEGACY_DEFAULT, "CacheMode.Default" },
    { "ARK_CACHE_MODE_DEFAULT", ARK_CACHE_MODE_DEFAULT, "CacheMode.DEFAULT" },
    { "ARK_CACHE_MODE_NONE", ARK_CACHE_MODE_NONE, "CacheMode.None" },
    { "ARK_CACHE_MODE_CACHE_ELSE_NETWORK", ARK_CACHE_MODE_CACHE_ELSE_NETWORK, "CacheMode.CACHE_ELSE_NETWORK" },
    { "ARK_CACHE_MODE_ONLINE", ARK_CACHE_MODE_ONLINE, "CacheMode.Online" },
    { "ARK_CACHE_MODE_NO_CACHE", ARK_CACHE_MODE_NO_CACHE, "CacheMode.NO_CACHE" },
    { "ARK_CACHE_MODE_ONLY", ARK_CACHE_MODE_ONLY, "CacheMode.Only" },
    { "ARK_CACHE_MODE_CACHE_ONLY", ARK_CACHE_MODE_CACHE_ONLY, "CacheMode.CACHE_ONLY" },
};

std::vector<std::tuple<std::string, Ark_CacheMode>> testFixtureEnumCacheModeInvalidValues = {
    { "-1", static_cast<Ark_CacheMode>(-1) },
    { "INT_MAX", static_cast<Ark_CacheMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ChainStyle, std::string>> testFixtureEnumChainStyleValidValues = {
    { "ARK_CHAIN_STYLE_SPREAD", ARK_CHAIN_STYLE_SPREAD, "ChainStyle.SPREAD" },
    { "ARK_CHAIN_STYLE_SPREAD_INSIDE", ARK_CHAIN_STYLE_SPREAD_INSIDE, "ChainStyle.SPREAD_INSIDE" },
    { "ARK_CHAIN_STYLE_PACKED", ARK_CHAIN_STYLE_PACKED, "ChainStyle.PACKED" },
};

std::vector<std::tuple<std::string, Ark_ChainStyle>> testFixtureEnumChainStyleInvalidValues = {
    { "-1", static_cast<Ark_ChainStyle>(-1) },
    { "INT_MAX", static_cast<Ark_ChainStyle>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ClickEffectLevel, std::string>> testFixtureEnumClickEffectLevelValidValues = {
    { "ARK_CLICK_EFFECT_LEVEL_LIGHT", ARK_CLICK_EFFECT_LEVEL_LIGHT, "ClickEffectLevel.LIGHT" },
    { "ARK_CLICK_EFFECT_LEVEL_MIDDLE", ARK_CLICK_EFFECT_LEVEL_MIDDLE, "ClickEffectLevel.MIDDLE" },
    { "ARK_CLICK_EFFECT_LEVEL_HEAVY", ARK_CLICK_EFFECT_LEVEL_HEAVY, "ClickEffectLevel.HEAVY" },
};

std::vector<std::tuple<std::string, Ark_ClickEffectLevel>> testFixtureEnumClickEffectLevelInvalidValues = {
    { "-1", static_cast<Ark_ClickEffectLevel>(-1) },
    { "INT_MAX", static_cast<Ark_ClickEffectLevel>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_Color, std::string>> testFixtureEnumColorValidValues = {
    { "ARK_COLOR_WHITE", ARK_COLOR_WHITE, "Color.White" },
    { "ARK_COLOR_BLACK", ARK_COLOR_BLACK, "Color.Black" },
    { "ARK_COLOR_BLUE", ARK_COLOR_BLUE, "Color.Blue" },
    { "ARK_COLOR_BROWN", ARK_COLOR_BROWN, "Color.Brown" },
    { "ARK_COLOR_GRAY", ARK_COLOR_GRAY, "Color.Gray" },
    { "ARK_COLOR_GREEN", ARK_COLOR_GREEN, "Color.Green" },
    { "ARK_COLOR_GREY", ARK_COLOR_GREY, "Color.Grey" },
    { "ARK_COLOR_ORANGE", ARK_COLOR_ORANGE, "Color.Orange" },
    { "ARK_COLOR_PINK", ARK_COLOR_PINK, "Color.Pink" },
    { "ARK_COLOR_RED", ARK_COLOR_RED, "Color.Red" },
    { "ARK_COLOR_YELLOW", ARK_COLOR_YELLOW, "Color.Yellow" },
    { "ARK_COLOR_TRANSPARENT", ARK_COLOR_TRANSPARENT, "Color.Transparent" },
};

std::vector<std::tuple<std::string, Ark_Color>> testFixtureEnumColorInvalidValues = {
    { "-1", static_cast<Ark_Color>(-1) },
    { "INT_MAX", static_cast<Ark_Color>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ColoringStrategy, std::string>> testFixtureEnumColoringStrategyValidValues = {
    { "ARK_COLORING_STRATEGY_INVERT", ARK_COLORING_STRATEGY_INVERT, "ColoringStrategy.INVERT" },
    { "ARK_COLORING_STRATEGY_AVERAGE", ARK_COLORING_STRATEGY_AVERAGE, "ColoringStrategy.AVERAGE" },
    { "ARK_COLORING_STRATEGY_PRIMARY", ARK_COLORING_STRATEGY_PRIMARY, "ColoringStrategy.PRIMARY" },
};

std::vector<std::tuple<std::string, Ark_ColoringStrategy>> testFixtureEnumColoringStrategyInvalidValues = {
    { "-1", static_cast<Ark_ColoringStrategy>(-1) },
    { "INT_MAX", static_cast<Ark_ColoringStrategy>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ContentClipMode, std::string>> testFixtureEnumContentClipModeValidValues = {
    { "ARK_CONTENT_CLIP_MODE_CONTENT_ONLY", ARK_CONTENT_CLIP_MODE_CONTENT_ONLY, "ContentClipMode.CONTENT_ONLY" },
    { "ARK_CONTENT_CLIP_MODE_BOUNDARY", ARK_CONTENT_CLIP_MODE_BOUNDARY, "ContentClipMode.BOUNDARY" },
    { "ARK_CONTENT_CLIP_MODE_SAFE_AREA", ARK_CONTENT_CLIP_MODE_SAFE_AREA, "ContentClipMode.SAFE_AREA" },
};

std::vector<std::tuple<std::string, Ark_ContentClipMode>> testFixtureEnumContentClipModeInvalidValues = {
    { "-1", static_cast<Ark_ContentClipMode>(-1) },
    { "INT_MAX", static_cast<Ark_ContentClipMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ControlSize, std::string>> testFixtureEnumControlSizeValidValues = {
    { "ARK_CONTROL_SIZE_SMALL", ARK_CONTROL_SIZE_SMALL, "ControlSize.SMALL" },
    { "ARK_CONTROL_SIZE_NORMAL", ARK_CONTROL_SIZE_NORMAL, "ControlSize.NORMAL" },
};

std::vector<std::tuple<std::string, Ark_ControlSize>> testFixtureEnumControlSizeInvalidValues = {
    { "-1", static_cast<Ark_ControlSize>(-1) },
    { "INT_MAX", static_cast<Ark_ControlSize>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_CopyOptions, std::string>> testFixtureEnumCopyOptionsValidValues = {
    { "ARK_COPY_OPTIONS_NONE", ARK_COPY_OPTIONS_NONE, "CopyOptions.None" },
    { "ARK_COPY_OPTIONS_IN_APP", ARK_COPY_OPTIONS_IN_APP, "CopyOptions.InApp" },
    { "ARK_COPY_OPTIONS_LOCAL_DEVICE", ARK_COPY_OPTIONS_LOCAL_DEVICE, "CopyOptions.LocalDevice" },
    { "ARK_COPY_OPTIONS_CROSS_DEVICE", ARK_COPY_OPTIONS_CROSS_DEVICE, "CopyOptions.CROSS_DEVICE" },
};

std::vector<std::tuple<std::string, Ark_CopyOptions>> testFixtureEnumCopyOptionsInvalidValues = {
    { "-1", static_cast<Ark_CopyOptions>(-1) },
    { "INT_MAX", static_cast<Ark_CopyOptions>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_CrownSensitivity, std::string>> testFixtureEnumCrownSensitivityValidValues = {
    { "ARK_CROWN_SENSITIVITY_LOW", ARK_CROWN_SENSITIVITY_LOW, "CrownSensitivity.LOW" },
    { "ARK_CROWN_SENSITIVITY_MEDIUM", ARK_CROWN_SENSITIVITY_MEDIUM, "CrownSensitivity.MEDIUM" },
    { "ARK_CROWN_SENSITIVITY_HIGH", ARK_CROWN_SENSITIVITY_HIGH, "CrownSensitivity.HIGH" },
};

std::vector<std::tuple<std::string, Ark_CrownSensitivity>> testFixtureEnumCrownSensitivityInvalidValues = {
    { "-1", static_cast<Ark_CrownSensitivity>(-1) },
    { "INT_MAX", static_cast<Ark_CrownSensitivity>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_Direction, std::string>> testFixtureEnumDirectionValidValues = {
    { "ARK_DIRECTION_LTR", ARK_DIRECTION_LTR, "Direction.Ltr" },
    { "ARK_DIRECTION_RTL", ARK_DIRECTION_RTL, "Direction.Rtl" },
    { "ARK_DIRECTION_AUTO", ARK_DIRECTION_AUTO, "Direction.Auto" },
};

std::vector<std::tuple<std::string, Ark_Direction>> testFixtureEnumDirectionInvalidValues = {
    { "-1", static_cast<Ark_Direction>(-1) },
    { "INT_MAX", static_cast<Ark_Direction>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_DragPreviewMode, std::string>> testFixtureEnumDragPreviewModeValidValues = {
    { "ARK_DRAG_PREVIEW_MODE_AUTO", ARK_DRAG_PREVIEW_MODE_AUTO, "DragPreviewMode.AUTO" },
    { "ARK_DRAG_PREVIEW_MODE_DISABLE_SCALE", ARK_DRAG_PREVIEW_MODE_DISABLE_SCALE, "DragPreviewMode.DISABLE_SCALE" },
    { "ARK_DRAG_PREVIEW_MODE_ENABLE_DEFAULT_SHADOW", ARK_DRAG_PREVIEW_MODE_ENABLE_DEFAULT_SHADOW,
        "DragPreviewMode.ENABLE_DEFAULT_SHADOW" },
    { "ARK_DRAG_PREVIEW_MODE_ENABLE_DEFAULT_RADIUS", ARK_DRAG_PREVIEW_MODE_ENABLE_DEFAULT_RADIUS,
        "DragPreviewMode.ENABLE_DEFAULT_RADIUS" },
    { "ARK_DRAG_PREVIEW_MODE_ENABLE_DRAG_ITEM_GRAY_EFFECT", ARK_DRAG_PREVIEW_MODE_ENABLE_DRAG_ITEM_GRAY_EFFECT,
        "DragPreviewMode.ENABLE_DRAG_ITEM_GRAY_EFFECT" },
    { "ARK_DRAG_PREVIEW_MODE_ENABLE_MULTI_TILE_EFFECT", ARK_DRAG_PREVIEW_MODE_ENABLE_MULTI_TILE_EFFECT,
        "DragPreviewMode.ENABLE_MULTI_TILE_EFFECT" },
};

std::vector<std::tuple<std::string, Ark_DragPreviewMode>> testFixtureEnumDragPreviewModeInvalidValues = {
    { "-1", static_cast<Ark_DragPreviewMode>(-1) },
    { "INT_MAX", static_cast<Ark_DragPreviewMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_DynamicRangeMode, std::string>> testFixtureEnumDynamicRangeModeValidValues = {
    { "ARK_DYNAMIC_RANGE_MODE_HIGH", ARK_DYNAMIC_RANGE_MODE_HIGH, "DynamicRangeMode.HIGH" },
    { "ARK_DYNAMIC_RANGE_MODE_CONSTRAINT", ARK_DYNAMIC_RANGE_MODE_CONSTRAINT, "DynamicRangeMode.CONSTRAINT" },
    { "ARK_DYNAMIC_RANGE_MODE_STANDARD", ARK_DYNAMIC_RANGE_MODE_STANDARD, "DynamicRangeMode.STANDARD" },
};

std::vector<std::tuple<std::string, Ark_DynamicRangeMode>> testFixtureEnumDynamicRangeModeInvalidValues = {
    { "-1", static_cast<Ark_DynamicRangeMode>(-1) },
    { "INT_MAX", static_cast<Ark_DynamicRangeMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_EdgeEffect, std::string>> testFixtureEnumEdgeEffectValidValues = {
    { "ARK_EDGE_EFFECT_SPRING", ARK_EDGE_EFFECT_SPRING, "EdgeEffect.Spring" },
    { "ARK_EDGE_EFFECT_FADE", ARK_EDGE_EFFECT_FADE, "EdgeEffect.Fade" },
    { "ARK_EDGE_EFFECT_NONE", ARK_EDGE_EFFECT_NONE, "EdgeEffect.None" },
};

std::vector<std::tuple<std::string, Ark_EdgeEffect>> testFixtureEnumEdgeEffectInvalidValues = {
    { "-1", static_cast<Ark_EdgeEffect>(-1) },
    { "INT_MAX", static_cast<Ark_EdgeEffect>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_EffectType, std::string>> testFixtureEnumEffectTypeValidValues = {
    { "ARK_EFFECT_TYPE_DEFAULT", ARK_EFFECT_TYPE_DEFAULT, "EffectType.DEFAULT" },
    { "ARK_EFFECT_TYPE_WINDOW_EFFECT", ARK_EFFECT_TYPE_WINDOW_EFFECT, "EffectType.WINDOW_EFFECT" },
};

std::vector<std::tuple<std::string, Ark_EffectType>> testFixtureEnumEffectTypeInvalidValues = {
    { "-1", static_cast<Ark_EffectType>(-1) },
    { "INT_MAX", static_cast<Ark_EffectType>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_EllipsisMode, std::string>> testFixtureEnumEllipsisModeValidValues = {
    { "ARK_ELLIPSIS_MODE_START", ARK_ELLIPSIS_MODE_START, "EllipsisMode.START" },
    { "ARK_ELLIPSIS_MODE_CENTER", ARK_ELLIPSIS_MODE_CENTER, "EllipsisMode.CENTER" },
    { "ARK_ELLIPSIS_MODE_END", ARK_ELLIPSIS_MODE_END, "EllipsisMode.END" },
};

std::vector<std::tuple<std::string, Ark_EllipsisMode>> testFixtureEnumEllipsisModeInvalidValues = {
    { "-1", static_cast<Ark_EllipsisMode>(-1) },
    { "INT_MAX", static_cast<Ark_EllipsisMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_EnterKeyType, std::string>> testFixtureEnumEnterKeyTypeValidValues = {
    { "ARK_ENTER_KEY_TYPE_GO", ARK_ENTER_KEY_TYPE_GO, "EnterKeyType.Go" },
    { "ARK_ENTER_KEY_TYPE_SEARCH", ARK_ENTER_KEY_TYPE_SEARCH, "EnterKeyType.Search" },
    { "ARK_ENTER_KEY_TYPE_SEND", ARK_ENTER_KEY_TYPE_SEND, "EnterKeyType.Send" },
    { "ARK_ENTER_KEY_TYPE_NEXT", ARK_ENTER_KEY_TYPE_NEXT, "EnterKeyType.Next" },
    { "ARK_ENTER_KEY_TYPE_DONE", ARK_ENTER_KEY_TYPE_DONE, "EnterKeyType.Done" },
    { "ARK_ENTER_KEY_TYPE_PREVIOUS", ARK_ENTER_KEY_TYPE_PREVIOUS, "EnterKeyType.PREVIOUS" },
    { "ARK_ENTER_KEY_TYPE_NEW_LINE", ARK_ENTER_KEY_TYPE_NEW_LINE, "EnterKeyType.NEW_LINE" },
};

std::vector<std::tuple<std::string, Ark_EnterKeyType>> testFixtureEnumEnterKeyTypeInvalidValues = {
    { "-1", static_cast<Ark_EnterKeyType>(-1) },
    { "INT_MAX", static_cast<Ark_EnterKeyType>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_FillMode, std::string>> testFixtureEnumFillModeValidValues = {
    { "ARK_FILL_MODE_NONE", ARK_FILL_MODE_NONE, "FillMode.None" },
    { "ARK_FILL_MODE_FORWARDS", ARK_FILL_MODE_FORWARDS, "FillMode.Forwards" },
    { "ARK_FILL_MODE_BACKWARDS", ARK_FILL_MODE_BACKWARDS, "FillMode.Backwards" },
    { "ARK_FILL_MODE_BOTH", ARK_FILL_MODE_BOTH, "FillMode.Both" },
};

std::vector<std::tuple<std::string, Ark_FillMode>> testFixtureEnumFillModeInvalidValues = {
    { "-1", static_cast<Ark_FillMode>(-1) },
    { "INT_MAX", static_cast<Ark_FillMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_FinishCallbackType, std::string>>
    testFixtureEnumFinishCallbackTypeValidValues = {
        { "ARK_FINISH_CALLBACK_TYPE_REMOVED", ARK_FINISH_CALLBACK_TYPE_REMOVED, "FinishCallbackType.REMOVED" },
        { "ARK_FINISH_CALLBACK_TYPE_LOGICALLY", ARK_FINISH_CALLBACK_TYPE_LOGICALLY, "FinishCallbackType.LOGICALLY" },
    };

std::vector<std::tuple<std::string, Ark_FinishCallbackType>> testFixtureEnumFinishCallbackTypeInvalidValues = {
    { "-1", static_cast<Ark_FinishCallbackType>(-1) },
    { "INT_MAX", static_cast<Ark_FinishCallbackType>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_FlexAlign, std::string>> testFixtureEnumFlexAlignValidValues = {
    { "ARK_FLEX_ALIGN_START", ARK_FLEX_ALIGN_START, "FlexAlign.Start" },
    { "ARK_FLEX_ALIGN_CENTER", ARK_FLEX_ALIGN_CENTER, "FlexAlign.Center" },
    { "ARK_FLEX_ALIGN_END", ARK_FLEX_ALIGN_END, "FlexAlign.End" },
    { "ARK_FLEX_ALIGN_SPACE_BETWEEN", ARK_FLEX_ALIGN_SPACE_BETWEEN, "FlexAlign.SpaceBetween" },
    { "ARK_FLEX_ALIGN_SPACE_AROUND", ARK_FLEX_ALIGN_SPACE_AROUND, "FlexAlign.SpaceAround" },
    { "ARK_FLEX_ALIGN_SPACE_EVENLY", ARK_FLEX_ALIGN_SPACE_EVENLY, "FlexAlign.SpaceEvenly" },
};

std::vector<std::tuple<std::string, Ark_FlexAlign>> testFixtureEnumFlexAlignInvalidValues = {
    { "-1", static_cast<Ark_FlexAlign>(-1) },
    { "INT_MAX", static_cast<Ark_FlexAlign>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_FlexDirection, std::string>> testFixtureEnumFlexDirectionValidValues = {
    { "ARK_FLEX_DIRECTION_ROW", ARK_FLEX_DIRECTION_ROW, "FlexDirection.Row" },
    { "ARK_FLEX_DIRECTION_COLUMN", ARK_FLEX_DIRECTION_COLUMN, "FlexDirection.Column" },
    { "ARK_FLEX_DIRECTION_ROW_REVERSE", ARK_FLEX_DIRECTION_ROW_REVERSE, "FlexDirection.RowReverse" },
    { "ARK_FLEX_DIRECTION_COLUMN_REVERSE", ARK_FLEX_DIRECTION_COLUMN_REVERSE, "FlexDirection.ColumnReverse" },
};

std::vector<std::tuple<std::string, Ark_FlexDirection>> testFixtureEnumFlexDirectionInvalidValues = {
    { "-1", static_cast<Ark_FlexDirection>(-1) },
    { "INT_MAX", static_cast<Ark_FlexDirection>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_FocusDrawLevel, std::string>> testFixtureEnumFocusDrawLevelValidValues = {
    { "ARK_FOCUS_DRAW_LEVEL_SELF", ARK_FOCUS_DRAW_LEVEL_SELF, "FocusDrawLevel.SELF" },
    { "ARK_FOCUS_DRAW_LEVEL_TOP", ARK_FOCUS_DRAW_LEVEL_TOP, "FocusDrawLevel.TOP" },
};

std::vector<std::tuple<std::string, Ark_FocusDrawLevel>> testFixtureEnumFocusDrawLevelInvalidValues = {
    { "-1", static_cast<Ark_FocusDrawLevel>(-1) },
    { "INT_MAX", static_cast<Ark_FocusDrawLevel>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_FocusPriority, std::string>> testFixtureEnumFocusPriorityValidValues = {
    { "ARK_FOCUS_PRIORITY_AUTO", ARK_FOCUS_PRIORITY_AUTO, "FocusPriority.AUTO" },
    { "ARK_FOCUS_PRIORITY_PRIOR", ARK_FOCUS_PRIORITY_PRIOR, "FocusPriority.PRIOR" },
    { "ARK_FOCUS_PRIORITY_PREVIOUS", ARK_FOCUS_PRIORITY_PREVIOUS, "FocusPriority.PREVIOUS" },
};

std::vector<std::tuple<std::string, Ark_FocusPriority>> testFixtureEnumFocusPriorityInvalidValues = {
    { "-1", static_cast<Ark_FocusPriority>(-1) },
    { "INT_MAX", static_cast<Ark_FocusPriority>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_FontStyle, std::string>> testFixtureEnumFontStyleValidValues = {
    { "ARK_FONT_STYLE_NORMAL", ARK_FONT_STYLE_NORMAL, "FontStyle.Normal" },
    { "ARK_FONT_STYLE_ITALIC", ARK_FONT_STYLE_ITALIC, "FontStyle.Italic" },
};

std::vector<std::tuple<std::string, Ark_FontStyle>> testFixtureEnumFontStyleInvalidValues = {
    { "-1", static_cast<Ark_FontStyle>(-1) },
    { "INT_MAX", static_cast<Ark_FontStyle>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_FontWeight, std::string>> testFixtureEnumFontWeightValidValues = {
    { "ARK_FONT_WEIGHT_LIGHTER", ARK_FONT_WEIGHT_LIGHTER, "FontWeight.Lighter" },
    { "ARK_FONT_WEIGHT_NORMAL", ARK_FONT_WEIGHT_NORMAL, "FontWeight.Normal" },
    { "ARK_FONT_WEIGHT_REGULAR", ARK_FONT_WEIGHT_REGULAR, "FontWeight.Regular" },
    { "ARK_FONT_WEIGHT_MEDIUM", ARK_FONT_WEIGHT_MEDIUM, "FontWeight.Medium" },
    { "ARK_FONT_WEIGHT_BOLD", ARK_FONT_WEIGHT_BOLD, "FontWeight.Bold" },
    { "ARK_FONT_WEIGHT_BOLDER", ARK_FONT_WEIGHT_BOLDER, "FontWeight.Bolder" },
};

std::vector<std::tuple<std::string, Ark_FontWeight>> testFixtureEnumFontWeightInvalidValues = {
    { "-1", static_cast<Ark_FontWeight>(-1) },
    { "INT_MAX", static_cast<Ark_FontWeight>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_GestureControl_GestureType, std::string>>
    testFixtureEnumGestureControl_GestureTypeValidValues = {
        { "ARK_GESTURE_CONTROL_GESTURE_TYPE_TAP_GESTURE", ARK_GESTURE_CONTROL_GESTURE_TYPE_TAP_GESTURE,
            "GestureControl_GestureType.TAP_GESTURE" },
        { "ARK_GESTURE_CONTROL_GESTURE_TYPE_LONG_PRESS_GESTURE", ARK_GESTURE_CONTROL_GESTURE_TYPE_LONG_PRESS_GESTURE,
            "GestureControl_GestureType.LONG_PRESS_GESTURE" },
        { "ARK_GESTURE_CONTROL_GESTURE_TYPE_PAN_GESTURE", ARK_GESTURE_CONTROL_GESTURE_TYPE_PAN_GESTURE,
            "GestureControl_GestureType.PAN_GESTURE" },
        { "ARK_GESTURE_CONTROL_GESTURE_TYPE_PINCH_GESTURE", ARK_GESTURE_CONTROL_GESTURE_TYPE_PINCH_GESTURE,
            "GestureControl_GestureType.PINCH_GESTURE" },
        { "ARK_GESTURE_CONTROL_GESTURE_TYPE_SWIPE_GESTURE", ARK_GESTURE_CONTROL_GESTURE_TYPE_SWIPE_GESTURE,
            "GestureControl_GestureType.SWIPE_GESTURE" },
        { "ARK_GESTURE_CONTROL_GESTURE_TYPE_ROTATION_GESTURE", ARK_GESTURE_CONTROL_GESTURE_TYPE_ROTATION_GESTURE,
            "GestureControl_GestureType.ROTATION_GESTURE" },
        { "ARK_GESTURE_CONTROL_GESTURE_TYPE_DRAG", ARK_GESTURE_CONTROL_GESTURE_TYPE_DRAG,
            "GestureControl_GestureType.DRAG" },
        { "ARK_GESTURE_CONTROL_GESTURE_TYPE_CLICK", ARK_GESTURE_CONTROL_GESTURE_TYPE_CLICK,
            "GestureControl_GestureType.CLICK" },
    };

std::vector<std::tuple<std::string, Ark_GestureControl_GestureType>>
    testFixtureEnumGestureControl_GestureTypeInvalidValues = {
        { "-1", static_cast<Ark_GestureControl_GestureType>(-1) },
        { "INT_MAX", static_cast<Ark_GestureControl_GestureType>(INT_MAX) },
    };

std::vector<std::tuple<std::string, Ark_GestureMask, std::string>> testFixtureEnumGestureMaskValidValues = {
    { "ARK_GESTURE_MASK_NORMAL", ARK_GESTURE_MASK_NORMAL, "GestureMask.Normal" },
    { "ARK_GESTURE_MASK_IGNORE_INTERNAL", ARK_GESTURE_MASK_IGNORE_INTERNAL, "GestureMask.IgnoreInternal" },
};

std::vector<std::tuple<std::string, Ark_GestureMask>> testFixtureEnumGestureMaskInvalidValues = {
    { "-1", static_cast<Ark_GestureMask>(-1) },
    { "INT_MAX", static_cast<Ark_GestureMask>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_GradientDirection, std::string>> testFixtureEnumGradientDirectionValidValues = {
    { "ARK_GRADIENT_DIRECTION_LEFT", ARK_GRADIENT_DIRECTION_LEFT, "GradientDirection.Left" },
    { "ARK_GRADIENT_DIRECTION_TOP", ARK_GRADIENT_DIRECTION_TOP, "GradientDirection.Top" },
    { "ARK_GRADIENT_DIRECTION_RIGHT", ARK_GRADIENT_DIRECTION_RIGHT, "GradientDirection.Right" },
    { "ARK_GRADIENT_DIRECTION_BOTTOM", ARK_GRADIENT_DIRECTION_BOTTOM, "GradientDirection.Bottom" },
    { "ARK_GRADIENT_DIRECTION_LEFT_TOP", ARK_GRADIENT_DIRECTION_LEFT_TOP, "GradientDirection.LeftTop" },
    { "ARK_GRADIENT_DIRECTION_LEFT_BOTTOM", ARK_GRADIENT_DIRECTION_LEFT_BOTTOM, "GradientDirection.LeftBottom" },
    { "ARK_GRADIENT_DIRECTION_RIGHT_TOP", ARK_GRADIENT_DIRECTION_RIGHT_TOP, "GradientDirection.RightTop" },
    { "ARK_GRADIENT_DIRECTION_RIGHT_BOTTOM", ARK_GRADIENT_DIRECTION_RIGHT_BOTTOM, "GradientDirection.RightBottom" },
    { "ARK_GRADIENT_DIRECTION_NONE", ARK_GRADIENT_DIRECTION_NONE, "GradientDirection.None" },
};

std::vector<std::tuple<std::string, Ark_GradientDirection>> testFixtureEnumGradientDirectionInvalidValues = {
    { "-1", static_cast<Ark_GradientDirection>(-1) },
    { "INT_MAX", static_cast<Ark_GradientDirection>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_GridRowDirection, std::string>> testFixtureEnumGridRowDirectionValidValues = {
    { "ARK_GRID_ROW_DIRECTION_ROW", ARK_GRID_ROW_DIRECTION_ROW, "GridRowDirection.Row" },
    { "ARK_GRID_ROW_DIRECTION_ROW_REVERSE", ARK_GRID_ROW_DIRECTION_ROW_REVERSE, "GridRowDirection.RowReverse" },
};

std::vector<std::tuple<std::string, Ark_GridRowDirection>> testFixtureEnumGridRowDirectionInvalidValues = {
    { "-1", static_cast<Ark_GridRowDirection>(-1) },
    { "INT_MAX", static_cast<Ark_GridRowDirection>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_HitTestMode, std::string>> testFixtureEnumHitTestModeValidValues = {
    { "ARK_HIT_TEST_MODE_DEFAULT", ARK_HIT_TEST_MODE_DEFAULT, "HitTestMode.Default" },
    { "ARK_HIT_TEST_MODE_BLOCK", ARK_HIT_TEST_MODE_BLOCK, "HitTestMode.Block" },
    { "ARK_HIT_TEST_MODE_TRANSPARENT", ARK_HIT_TEST_MODE_TRANSPARENT, "HitTestMode.Transparent" },
    { "ARK_HIT_TEST_MODE_NONE", ARK_HIT_TEST_MODE_NONE, "HitTestMode.None" },
};

std::vector<std::tuple<std::string, Ark_HitTestMode>> testFixtureEnumHitTestModeInvalidValues = {
    { "-1", static_cast<Ark_HitTestMode>(-1) },
    { "INT_MAX", static_cast<Ark_HitTestMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_HorizontalAlign, std::string>> testFixtureEnumHorizontalAlignValidValues = {
    { "ARK_HORIZONTAL_ALIGN_START", ARK_HORIZONTAL_ALIGN_START, "HorizontalAlign.Start" },
    { "ARK_HORIZONTAL_ALIGN_CENTER", ARK_HORIZONTAL_ALIGN_CENTER, "HorizontalAlign.Center" },
    { "ARK_HORIZONTAL_ALIGN_END", ARK_HORIZONTAL_ALIGN_END, "HorizontalAlign.End" },
};

std::vector<std::tuple<std::string, Ark_HorizontalAlign>> testFixtureEnumHorizontalAlignInvalidValues = {
    { "-1", static_cast<Ark_HorizontalAlign>(-1) },
    { "INT_MAX", static_cast<Ark_HorizontalAlign>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_HoverEffect, std::string>> testFixtureEnumHoverEffectValidValues = {
    { "ARK_HOVER_EFFECT_AUTO", ARK_HOVER_EFFECT_AUTO, "HoverEffect.Auto" },
    { "ARK_HOVER_EFFECT_SCALE", ARK_HOVER_EFFECT_SCALE, "HoverEffect.Scale" },
    { "ARK_HOVER_EFFECT_HIGHLIGHT", ARK_HOVER_EFFECT_HIGHLIGHT, "HoverEffect.Highlight" },
    { "ARK_HOVER_EFFECT_NONE", ARK_HOVER_EFFECT_NONE, "HoverEffect.None" },
};

std::vector<std::tuple<std::string, Ark_HoverEffect>> testFixtureEnumHoverEffectInvalidValues = {
    { "-1", static_cast<Ark_HoverEffect>(-1) },
    { "INT_MAX", static_cast<Ark_HoverEffect>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_IlluminatedType, std::string>> testFixtureEnumIlluminatedTypeValidValues = {
    { "ARK_ILLUMINATED_TYPE_NONE", ARK_ILLUMINATED_TYPE_NONE, "IlluminatedType.NONE" },
    { "ARK_ILLUMINATED_TYPE_BORDER", ARK_ILLUMINATED_TYPE_BORDER, "IlluminatedType.BORDER" },
    { "ARK_ILLUMINATED_TYPE_CONTENT", ARK_ILLUMINATED_TYPE_CONTENT, "IlluminatedType.CONTENT" },
    { "ARK_ILLUMINATED_TYPE_BORDER_CONTENT", ARK_ILLUMINATED_TYPE_BORDER_CONTENT, "IlluminatedType.BORDER_CONTENT" },
    { "ARK_ILLUMINATED_TYPE_BLOOM_BORDER", ARK_ILLUMINATED_TYPE_BLOOM_BORDER, "IlluminatedType.BLOOM_BORDER" },
    { "ARK_ILLUMINATED_TYPE_BLOOM_BORDER_CONTENT", ARK_ILLUMINATED_TYPE_BLOOM_BORDER_CONTENT,
        "IlluminatedType.BLOOM_BORDER_CONTENT" },
};

std::vector<std::tuple<std::string, Ark_IlluminatedType>> testFixtureEnumIlluminatedTypeInvalidValues = {
    { "-1", static_cast<Ark_IlluminatedType>(-1) },
    { "INT_MAX", static_cast<Ark_IlluminatedType>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ImageContent, std::string>> testFixtureEnumImageContentValidValues = {
    { "ARK_IMAGE_CONTENT_EMPTY", ARK_IMAGE_CONTENT_EMPTY, "ImageContent.EMPTY" },
};

std::vector<std::tuple<std::string, Ark_ImageContent>> testFixtureEnumImageContentInvalidValues = {
    { "-1", static_cast<Ark_ImageContent>(-1) },
    { "INT_MAX", static_cast<Ark_ImageContent>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ImageFit, std::string>> testFixtureEnumImageFitValidValues = {
    { "ARK_IMAGE_FIT_CONTAIN", ARK_IMAGE_FIT_CONTAIN, "ImageFit.Contain" },
    { "ARK_IMAGE_FIT_COVER", ARK_IMAGE_FIT_COVER, "ImageFit.Cover" },
    { "ARK_IMAGE_FIT_AUTO", ARK_IMAGE_FIT_AUTO, "ImageFit.Auto" },
    { "ARK_IMAGE_FIT_FILL", ARK_IMAGE_FIT_FILL, "ImageFit.Fill" },
    { "ARK_IMAGE_FIT_SCALE_DOWN", ARK_IMAGE_FIT_SCALE_DOWN, "ImageFit.ScaleDown" },
    { "ARK_IMAGE_FIT_NONE", ARK_IMAGE_FIT_NONE, "ImageFit.None" },
    { "ARK_IMAGE_FIT_TOP_START", ARK_IMAGE_FIT_TOP_START, "ImageFit.TOP_START" },
    { "ARK_IMAGE_FIT_TOP", ARK_IMAGE_FIT_TOP, "ImageFit.TOP" },
    { "ARK_IMAGE_FIT_TOP_END", ARK_IMAGE_FIT_TOP_END, "ImageFit.TOP_END" },
    { "ARK_IMAGE_FIT_START", ARK_IMAGE_FIT_START, "ImageFit.START" },
    { "ARK_IMAGE_FIT_CENTER", ARK_IMAGE_FIT_CENTER, "ImageFit.CENTER" },
    { "ARK_IMAGE_FIT_END", ARK_IMAGE_FIT_END, "ImageFit.END" },
    { "ARK_IMAGE_FIT_BOTTOM_START", ARK_IMAGE_FIT_BOTTOM_START, "ImageFit.BOTTOM_START" },
    { "ARK_IMAGE_FIT_BOTTOM", ARK_IMAGE_FIT_BOTTOM, "ImageFit.BOTTOM" },
    { "ARK_IMAGE_FIT_BOTTOM_END", ARK_IMAGE_FIT_BOTTOM_END, "ImageFit.BOTTOM_END" },
    { "ARK_IMAGE_FIT_MATRIX", ARK_IMAGE_FIT_MATRIX, "ImageFit.MATRIX" },
};

std::vector<std::tuple<std::string, Ark_ImageFit>> testFixtureEnumImageFitInvalidValues = {
    { "-1", static_cast<Ark_ImageFit>(-1) },
    { "INT_MAX", static_cast<Ark_ImageFit>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ImageInterpolation, std::string>>
    testFixtureEnumImageInterpolationValidValues = {
        { "ARK_IMAGE_INTERPOLATION_NONE", ARK_IMAGE_INTERPOLATION_NONE, "ImageInterpolation.None" },
        { "ARK_IMAGE_INTERPOLATION_LOW", ARK_IMAGE_INTERPOLATION_LOW, "ImageInterpolation.Low" },
        { "ARK_IMAGE_INTERPOLATION_MEDIUM", ARK_IMAGE_INTERPOLATION_MEDIUM, "ImageInterpolation.Medium" },
        { "ARK_IMAGE_INTERPOLATION_HIGH", ARK_IMAGE_INTERPOLATION_HIGH, "ImageInterpolation.High" },
    };

std::vector<std::tuple<std::string, Ark_ImageInterpolation>> testFixtureEnumImageInterpolationInvalidValues = {
    { "-1", static_cast<Ark_ImageInterpolation>(-1) },
    { "INT_MAX", static_cast<Ark_ImageInterpolation>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ImageRenderMode, std::string>> testFixtureEnumImageRenderModeValidValues = {
    { "ARK_IMAGE_RENDER_MODE_ORIGINAL", ARK_IMAGE_RENDER_MODE_ORIGINAL, "ImageRenderMode.Original" },
    { "ARK_IMAGE_RENDER_MODE_TEMPLATE", ARK_IMAGE_RENDER_MODE_TEMPLATE, "ImageRenderMode.Template" },
};

std::vector<std::tuple<std::string, Ark_ImageRenderMode>> testFixtureEnumImageRenderModeInvalidValues = {
    { "-1", static_cast<Ark_ImageRenderMode>(-1) },
    { "INT_MAX", static_cast<Ark_ImageRenderMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ImageRepeat, std::string>> testFixtureEnumImageRepeatValidValues = {
    { "ARK_IMAGE_REPEAT_NO_REPEAT", ARK_IMAGE_REPEAT_NO_REPEAT, "ImageRepeat.NoRepeat" },
    { "ARK_IMAGE_REPEAT_X", ARK_IMAGE_REPEAT_X, "ImageRepeat.X" },
    { "ARK_IMAGE_REPEAT_Y", ARK_IMAGE_REPEAT_Y, "ImageRepeat.Y" },
    { "ARK_IMAGE_REPEAT_XY", ARK_IMAGE_REPEAT_XY, "ImageRepeat.XY" },
};

std::vector<std::tuple<std::string, Ark_ImageRepeat>> testFixtureEnumImageRepeatInvalidValues = {
    { "-1", static_cast<Ark_ImageRepeat>(-1) },
    { "INT_MAX", static_cast<Ark_ImageRepeat>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_InputType, std::string>> testFixtureEnumInputTypeValidValues = {
    { "ARK_INPUT_TYPE_NORMAL", ARK_INPUT_TYPE_NORMAL, "InputType.Normal" },
    { "ARK_INPUT_TYPE_NUMBER", ARK_INPUT_TYPE_NUMBER, "InputType.Number" },
    { "ARK_INPUT_TYPE_PHONE_NUMBER", ARK_INPUT_TYPE_PHONE_NUMBER, "InputType.PhoneNumber" },
    { "ARK_INPUT_TYPE_EMAIL", ARK_INPUT_TYPE_EMAIL, "InputType.Email" },
    { "ARK_INPUT_TYPE_PASSWORD", ARK_INPUT_TYPE_PASSWORD, "InputType.Password" },
    { "ARK_INPUT_TYPE_NUMBER_PASSWORD", ARK_INPUT_TYPE_NUMBER_PASSWORD, "InputType.NUMBER_PASSWORD" },
    { "ARK_INPUT_TYPE_SCREEN_LOCK_PASSWORD", ARK_INPUT_TYPE_SCREEN_LOCK_PASSWORD, "InputType.SCREEN_LOCK_PASSWORD" },
    { "ARK_INPUT_TYPE_USER_NAME", ARK_INPUT_TYPE_USER_NAME, "InputType.USER_NAME" },
    { "ARK_INPUT_TYPE_NEW_PASSWORD", ARK_INPUT_TYPE_NEW_PASSWORD, "InputType.NEW_PASSWORD" },
    { "ARK_INPUT_TYPE_NUMBER_DECIMAL", ARK_INPUT_TYPE_NUMBER_DECIMAL, "InputType.NUMBER_DECIMAL" },
    { "ARK_INPUT_TYPE_URL", ARK_INPUT_TYPE_URL, "InputType.URL" },
};

std::vector<std::tuple<std::string, Ark_InputType>> testFixtureEnumInputTypeInvalidValues = {
    { "-1", static_cast<Ark_InputType>(-1) },
    { "INT_MAX", static_cast<Ark_InputType>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ItemAlign, std::string>> testFixtureEnumItemAlignValidValues = {
    { "ARK_ITEM_ALIGN_AUTO", ARK_ITEM_ALIGN_AUTO, "ItemAlign.Auto" },
    { "ARK_ITEM_ALIGN_START", ARK_ITEM_ALIGN_START, "ItemAlign.Start" },
    { "ARK_ITEM_ALIGN_CENTER", ARK_ITEM_ALIGN_CENTER, "ItemAlign.Center" },
    { "ARK_ITEM_ALIGN_END", ARK_ITEM_ALIGN_END, "ItemAlign.End" },
    { "ARK_ITEM_ALIGN_BASELINE", ARK_ITEM_ALIGN_BASELINE, "ItemAlign.Baseline" },
    { "ARK_ITEM_ALIGN_STRETCH", ARK_ITEM_ALIGN_STRETCH, "ItemAlign.Stretch" },
};

std::vector<std::tuple<std::string, Ark_ItemAlign>> testFixtureEnumItemAlignInvalidValues = {
    { "-1", static_cast<Ark_ItemAlign>(-1) },
    { "INT_MAX", static_cast<Ark_ItemAlign>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_KeyboardAppearance, std::string>>
    testFixtureEnumKeyboardAppearanceValidValues = {
        { "ARK_KEYBOARD_APPEARANCE_NONE_IMMERSIVE", ARK_KEYBOARD_APPEARANCE_NONE_IMMERSIVE,
            "KeyboardAppearance.NONE_IMMERSIVE" },
        { "ARK_KEYBOARD_APPEARANCE_IMMERSIVE", ARK_KEYBOARD_APPEARANCE_IMMERSIVE, "KeyboardAppearance.IMMERSIVE" },
        { "ARK_KEYBOARD_APPEARANCE_LIGHT_IMMERSIVE", ARK_KEYBOARD_APPEARANCE_LIGHT_IMMERSIVE,
            "KeyboardAppearance.LIGHT_IMMERSIVE" },
        { "ARK_KEYBOARD_APPEARANCE_DARK_IMMERSIVE", ARK_KEYBOARD_APPEARANCE_DARK_IMMERSIVE,
            "KeyboardAppearance.DARK_IMMERSIVE" },
    };

std::vector<std::tuple<std::string, Ark_KeyboardAppearance>> testFixtureEnumKeyboardAppearanceInvalidValues = {
    { "-1", static_cast<Ark_KeyboardAppearance>(-1) },
    { "INT_MAX", static_cast<Ark_KeyboardAppearance>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_KeyboardAvoidMode, std::string>> testFixtureEnumKeyboardAvoidModeValidValues = {
    { "ARK_KEYBOARD_AVOID_MODE_DEFAULT", ARK_KEYBOARD_AVOID_MODE_DEFAULT, "KeyboardAvoidMode.DEFAULT" },
    { "ARK_KEYBOARD_AVOID_MODE_NONE", ARK_KEYBOARD_AVOID_MODE_NONE, "KeyboardAvoidMode.NONE" },
};

std::vector<std::tuple<std::string, Ark_KeyboardAvoidMode>> testFixtureEnumKeyboardAvoidModeInvalidValues = {
    { "-1", static_cast<Ark_KeyboardAvoidMode>(-1) },
    { "INT_MAX", static_cast<Ark_KeyboardAvoidMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_MarqueeStartPolicy, std::string>>
    testFixtureEnumMarqueeStartPolicyValidValues = {
        { "ARK_MARQUEE_START_POLICY_DEFAULT", ARK_MARQUEE_START_POLICY_DEFAULT, "MarqueeStartPolicy.DEFAULT" },
        { "ARK_MARQUEE_START_POLICY_ON_FOCUS", ARK_MARQUEE_START_POLICY_ON_FOCUS, "MarqueeStartPolicy.ON_FOCUS" },
    };

std::vector<std::tuple<std::string, Ark_MarqueeStartPolicy>> testFixtureEnumMarqueeStartPolicyInvalidValues = {
    { "-1", static_cast<Ark_MarqueeStartPolicy>(-1) },
    { "INT_MAX", static_cast<Ark_MarqueeStartPolicy>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_MixedMode, std::string>> testFixtureEnumMixedModeValidValues = {
    { "ARK_MIXED_MODE_ALL", ARK_MIXED_MODE_ALL, "MixedMode.All" },
    { "ARK_MIXED_MODE_COMPATIBLE", ARK_MIXED_MODE_COMPATIBLE, "MixedMode.Compatible" },
    { "ARK_MIXED_MODE_NONE", ARK_MIXED_MODE_NONE, "MixedMode.None" },
};

std::vector<std::tuple<std::string, Ark_MixedMode>> testFixtureEnumMixedModeInvalidValues = {
    { "-1", static_cast<Ark_MixedMode>(-1) },
    { "INT_MAX", static_cast<Ark_MixedMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ModelType, std::string>> testFixtureEnumModelTypeValidValues = {
    { "ARK_MODEL_TYPE_TEXTURE", ARK_MODEL_TYPE_TEXTURE, "ModelType.TEXTURE" },
    { "ARK_MODEL_TYPE_SURFACE", ARK_MODEL_TYPE_SURFACE, "ModelType.SURFACE" },
};

std::vector<std::tuple<std::string, Ark_ModelType>> testFixtureEnumModelTypeInvalidValues = {
    { "-1", static_cast<Ark_ModelType>(-1) },
    { "INT_MAX", static_cast<Ark_ModelType>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_NestedScrollMode, std::string>> testFixtureEnumNestedScrollModeValidValues = {
    { "ARK_NESTED_SCROLL_MODE_SELF_ONLY", ARK_NESTED_SCROLL_MODE_SELF_ONLY, "NestedScrollMode.SELF_ONLY" },
    { "ARK_NESTED_SCROLL_MODE_SELF_FIRST", ARK_NESTED_SCROLL_MODE_SELF_FIRST, "NestedScrollMode.SELF_FIRST" },
    { "ARK_NESTED_SCROLL_MODE_PARENT_FIRST", ARK_NESTED_SCROLL_MODE_PARENT_FIRST, "NestedScrollMode.PARENT_FIRST" },
    { "ARK_NESTED_SCROLL_MODE_PARALLEL", ARK_NESTED_SCROLL_MODE_PARALLEL, "NestedScrollMode.PARALLEL" },
};

std::vector<std::tuple<std::string, Ark_NestedScrollMode>> testFixtureEnumNestedScrollModeInvalidValues = {
    { "-1", static_cast<Ark_NestedScrollMode>(-1) },
    { "INT_MAX", static_cast<Ark_NestedScrollMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_OutlineStyle, std::string>> testFixtureEnumOutlineStyleValidValues = {
    { "ARK_OUTLINE_STYLE_SOLID", ARK_OUTLINE_STYLE_SOLID, "OutlineStyle.SOLID" },
    { "ARK_OUTLINE_STYLE_DASHED", ARK_OUTLINE_STYLE_DASHED, "OutlineStyle.DASHED" },
    { "ARK_OUTLINE_STYLE_DOTTED", ARK_OUTLINE_STYLE_DOTTED, "OutlineStyle.DOTTED" },
};

std::vector<std::tuple<std::string, Ark_OutlineStyle>> testFixtureEnumOutlineStyleInvalidValues = {
    { "-1", static_cast<Ark_OutlineStyle>(-1) },
    { "INT_MAX", static_cast<Ark_OutlineStyle>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_OverScrollMode, std::string>> testFixtureEnumOverScrollModeValidValues = {
    { "ARK_OVER_SCROLL_MODE_NEVER", ARK_OVER_SCROLL_MODE_NEVER, "OverScrollMode.NEVER" },
    { "ARK_OVER_SCROLL_MODE_ALWAYS", ARK_OVER_SCROLL_MODE_ALWAYS, "OverScrollMode.ALWAYS" },
};

std::vector<std::tuple<std::string, Ark_OverScrollMode>> testFixtureEnumOverScrollModeInvalidValues = {
    { "-1", static_cast<Ark_OverScrollMode>(-1) },
    { "INT_MAX", static_cast<Ark_OverScrollMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_PixelRoundCalcPolicy, std::string>>
    testFixtureEnumPixelRoundCalcPolicyValidValues = {
        { "ARK_PIXEL_ROUND_CALC_POLICY_NO_FORCE_ROUND", ARK_PIXEL_ROUND_CALC_POLICY_NO_FORCE_ROUND,
            "PixelRoundCalcPolicy.NO_FORCE_ROUND" },
        { "ARK_PIXEL_ROUND_CALC_POLICY_FORCE_CEIL", ARK_PIXEL_ROUND_CALC_POLICY_FORCE_CEIL,
            "PixelRoundCalcPolicy.FORCE_CEIL" },
        { "ARK_PIXEL_ROUND_CALC_POLICY_FORCE_FLOOR", ARK_PIXEL_ROUND_CALC_POLICY_FORCE_FLOOR,
            "PixelRoundCalcPolicy.FORCE_FLOOR" },
    };

std::vector<std::tuple<std::string, Ark_PixelRoundCalcPolicy>> testFixtureEnumPixelRoundCalcPolicyInvalidValues = {
    { "-1", static_cast<Ark_PixelRoundCalcPolicy>(-1) },
    { "INT_MAX", static_cast<Ark_PixelRoundCalcPolicy>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_Placement, std::string>> testFixtureEnumPlacementValidValues = {
    { "ARK_PLACEMENT_LEFT", ARK_PLACEMENT_LEFT, "Placement.Left" },
    { "ARK_PLACEMENT_RIGHT", ARK_PLACEMENT_RIGHT, "Placement.Right" },
    { "ARK_PLACEMENT_TOP", ARK_PLACEMENT_TOP, "Placement.Top" },
    { "ARK_PLACEMENT_BOTTOM", ARK_PLACEMENT_BOTTOM, "Placement.Bottom" },
    { "ARK_PLACEMENT_TOP_LEFT", ARK_PLACEMENT_TOP_LEFT, "Placement.TopLeft" },
    { "ARK_PLACEMENT_TOP_RIGHT", ARK_PLACEMENT_TOP_RIGHT, "Placement.TopRight" },
    { "ARK_PLACEMENT_BOTTOM_LEFT", ARK_PLACEMENT_BOTTOM_LEFT, "Placement.BottomLeft" },
    { "ARK_PLACEMENT_BOTTOM_RIGHT", ARK_PLACEMENT_BOTTOM_RIGHT, "Placement.BottomRight" },
    { "ARK_PLACEMENT_LEFT_TOP", ARK_PLACEMENT_LEFT_TOP, "Placement.LeftTop" },
    { "ARK_PLACEMENT_LEFT_BOTTOM", ARK_PLACEMENT_LEFT_BOTTOM, "Placement.LeftBottom" },
    { "ARK_PLACEMENT_RIGHT_TOP", ARK_PLACEMENT_RIGHT_TOP, "Placement.RightTop" },
    { "ARK_PLACEMENT_RIGHT_BOTTOM", ARK_PLACEMENT_RIGHT_BOTTOM, "Placement.RightBottom" },
};

std::vector<std::tuple<std::string, Ark_Placement>> testFixtureEnumPlacementInvalidValues = {
    { "-1", static_cast<Ark_Placement>(-1) },
    { "INT_MAX", static_cast<Ark_Placement>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_PlayMode, std::string>> testFixtureEnumPlayModeValidValues = {
    { "ARK_PLAY_MODE_NORMAL", ARK_PLAY_MODE_NORMAL, "PlayMode.Normal" },
    { "ARK_PLAY_MODE_REVERSE", ARK_PLAY_MODE_REVERSE, "PlayMode.Reverse" },
    { "ARK_PLAY_MODE_ALTERNATE", ARK_PLAY_MODE_ALTERNATE, "PlayMode.Alternate" },
    { "ARK_PLAY_MODE_ALTERNATE_REVERSE", ARK_PLAY_MODE_ALTERNATE_REVERSE, "PlayMode.AlternateReverse" },
};

std::vector<std::tuple<std::string, Ark_PlayMode>> testFixtureEnumPlayModeInvalidValues = {
    { "-1", static_cast<Ark_PlayMode>(-1) },
    { "INT_MAX", static_cast<Ark_PlayMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ProgressStatus, std::string>> testFixtureEnumProgressStatusValidValues = {
    { "ARK_PROGRESS_STATUS_LOADING", ARK_PROGRESS_STATUS_LOADING, "ProgressStatus.LOADING" },
    { "ARK_PROGRESS_STATUS_PROGRESSING", ARK_PROGRESS_STATUS_PROGRESSING, "ProgressStatus.PROGRESSING" },
};

std::vector<std::tuple<std::string, Ark_ProgressStatus>> testFixtureEnumProgressStatusInvalidValues = {
    { "-1", static_cast<Ark_ProgressStatus>(-1) },
    { "INT_MAX", static_cast<Ark_ProgressStatus>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_ProgressStyle, std::string>> testFixtureEnumProgressStyleValidValues = {
    { "ARK_PROGRESS_STYLE_LINEAR", ARK_PROGRESS_STYLE_LINEAR, "ProgressStyle.Linear" },
    { "ARK_PROGRESS_STYLE_RING", ARK_PROGRESS_STYLE_RING, "ProgressStyle.Ring" },
    { "ARK_PROGRESS_STYLE_ECLIPSE", ARK_PROGRESS_STYLE_ECLIPSE, "ProgressStyle.Eclipse" },
    { "ARK_PROGRESS_STYLE_SCALE_RING", ARK_PROGRESS_STYLE_SCALE_RING, "ProgressStyle.ScaleRing" },
    { "ARK_PROGRESS_STYLE_CAPSULE", ARK_PROGRESS_STYLE_CAPSULE, "ProgressStyle.Capsule" },
};

std::vector<std::tuple<std::string, Ark_ProgressStyle>> testFixtureEnumProgressStyleInvalidValues = {
    { "-1", static_cast<Ark_ProgressStyle>(-1) },
    { "INT_MAX", static_cast<Ark_ProgressStyle>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_RadioIndicatorType, std::string>>
    testFixtureEnumRadioIndicatorTypeValidValues = {
        { "ARK_RADIO_INDICATOR_TYPE_TICK", ARK_RADIO_INDICATOR_TYPE_TICK, "RadioIndicatorType.TICK" },
        { "ARK_RADIO_INDICATOR_TYPE_DOT", ARK_RADIO_INDICATOR_TYPE_DOT, "RadioIndicatorType.DOT" },
        { "ARK_RADIO_INDICATOR_TYPE_CUSTOM", ARK_RADIO_INDICATOR_TYPE_CUSTOM, "RadioIndicatorType.CUSTOM" },
    };

std::vector<std::tuple<std::string, Ark_RadioIndicatorType>> testFixtureEnumRadioIndicatorTypeInvalidValues = {
    { "-1", static_cast<Ark_RadioIndicatorType>(-1) },
    { "INT_MAX", static_cast<Ark_RadioIndicatorType>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_RenderFit, std::string>> testFixtureEnumRenderFitValidValues = {
    { "ARK_RENDER_FIT_CENTER", ARK_RENDER_FIT_CENTER, "RenderFit.CENTER" },
    { "ARK_RENDER_FIT_TOP", ARK_RENDER_FIT_TOP, "RenderFit.TOP" },
    { "ARK_RENDER_FIT_BOTTOM", ARK_RENDER_FIT_BOTTOM, "RenderFit.BOTTOM" },
    { "ARK_RENDER_FIT_LEFT", ARK_RENDER_FIT_LEFT, "RenderFit.LEFT" },
    { "ARK_RENDER_FIT_RIGHT", ARK_RENDER_FIT_RIGHT, "RenderFit.RIGHT" },
    { "ARK_RENDER_FIT_TOP_LEFT", ARK_RENDER_FIT_TOP_LEFT, "RenderFit.TOP_LEFT" },
    { "ARK_RENDER_FIT_TOP_RIGHT", ARK_RENDER_FIT_TOP_RIGHT, "RenderFit.TOP_RIGHT" },
    { "ARK_RENDER_FIT_BOTTOM_LEFT", ARK_RENDER_FIT_BOTTOM_LEFT, "RenderFit.BOTTOM_LEFT" },
    { "ARK_RENDER_FIT_BOTTOM_RIGHT", ARK_RENDER_FIT_BOTTOM_RIGHT, "RenderFit.BOTTOM_RIGHT" },
    { "ARK_RENDER_FIT_RESIZE_FILL", ARK_RENDER_FIT_RESIZE_FILL, "RenderFit.RESIZE_FILL" },
    { "ARK_RENDER_FIT_RESIZE_CONTAIN", ARK_RENDER_FIT_RESIZE_CONTAIN, "RenderFit.RESIZE_CONTAIN" },
    { "ARK_RENDER_FIT_RESIZE_CONTAIN_TOP_LEFT", ARK_RENDER_FIT_RESIZE_CONTAIN_TOP_LEFT,
        "RenderFit.RESIZE_CONTAIN_TOP_LEFT" },
    { "ARK_RENDER_FIT_RESIZE_CONTAIN_BOTTOM_RIGHT", ARK_RENDER_FIT_RESIZE_CONTAIN_BOTTOM_RIGHT,
        "RenderFit.RESIZE_CONTAIN_BOTTOM_RIGHT" },
    { "ARK_RENDER_FIT_RESIZE_COVER", ARK_RENDER_FIT_RESIZE_COVER, "RenderFit.RESIZE_COVER" },
    { "ARK_RENDER_FIT_RESIZE_COVER_TOP_LEFT", ARK_RENDER_FIT_RESIZE_COVER_TOP_LEFT, "RenderFit.RESIZE_COVER_TOP_LEFT" },
    { "ARK_RENDER_FIT_RESIZE_COVER_BOTTOM_RIGHT", ARK_RENDER_FIT_RESIZE_COVER_BOTTOM_RIGHT,
        "RenderFit.RESIZE_COVER_BOTTOM_RIGHT" },
};

std::vector<std::tuple<std::string, Ark_RenderFit>> testFixtureEnumRenderFitInvalidValues = {
    { "-1", static_cast<Ark_RenderFit>(-1) },
    { "INT_MAX", static_cast<Ark_RenderFit>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_RenderMode, std::string>> testFixtureEnumRenderModeValidValues = {
    { "ARK_RENDER_MODE_ASYNC_RENDER", ARK_RENDER_MODE_ASYNC_RENDER, "RenderMode.ASYNC_RENDER" },
    { "ARK_RENDER_MODE_SYNC_RENDER", ARK_RENDER_MODE_SYNC_RENDER, "RenderMode.SYNC_RENDER" },
};

std::vector<std::tuple<std::string, Ark_RenderMode>> testFixtureEnumRenderModeInvalidValues = {
    { "-1", static_cast<Ark_RenderMode>(-1) },
    { "INT_MAX", static_cast<Ark_RenderMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_RepeatMode, std::string>> testFixtureEnumRepeatModeValidValues = {
    { "ARK_REPEAT_MODE_REPEAT", ARK_REPEAT_MODE_REPEAT, "RepeatMode.Repeat" },
    { "ARK_REPEAT_MODE_STRETCH", ARK_REPEAT_MODE_STRETCH, "RepeatMode.Stretch" },
    { "ARK_REPEAT_MODE_ROUND", ARK_REPEAT_MODE_ROUND, "RepeatMode.Round" },
    { "ARK_REPEAT_MODE_SPACE", ARK_REPEAT_MODE_SPACE, "RepeatMode.Space" },
};

std::vector<std::tuple<std::string, Ark_RepeatMode>> testFixtureEnumRepeatModeInvalidValues = {
    { "-1", static_cast<Ark_RepeatMode>(-1) },
    { "INT_MAX", static_cast<Ark_RepeatMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_SecurityComponentLayoutDirection, std::string>>
    testFixtureEnumSecurityComponentLayoutDirectionValidValues = {
        { "ARK_SECURITY_COMPONENT_LAYOUT_DIRECTION_HORIZONTAL", ARK_SECURITY_COMPONENT_LAYOUT_DIRECTION_HORIZONTAL,
            "SecurityComponentLayoutDirection.HORIZONTAL" },
        { "ARK_SECURITY_COMPONENT_LAYOUT_DIRECTION_VERTICAL", ARK_SECURITY_COMPONENT_LAYOUT_DIRECTION_VERTICAL,
            "SecurityComponentLayoutDirection.VERTICAL" },
    };

std::vector<std::tuple<std::string, Ark_SecurityComponentLayoutDirection>>
    testFixtureEnumSecurityComponentLayoutDirectionInvalidValues = {
        { "-1", static_cast<Ark_SecurityComponentLayoutDirection>(-1) },
        { "INT_MAX", static_cast<Ark_SecurityComponentLayoutDirection>(INT_MAX) },
    };

std::vector<std::tuple<std::string, Ark_ShadowType, std::string>> testFixtureEnumShadowTypeValidValues = {
    { "ARK_SHADOW_TYPE_COLOR", ARK_SHADOW_TYPE_COLOR, "ShadowType.COLOR" },
    { "ARK_SHADOW_TYPE_BLUR", ARK_SHADOW_TYPE_BLUR, "ShadowType.BLUR" },
};

std::vector<std::tuple<std::string, Ark_ShadowType>> testFixtureEnumShadowTypeInvalidValues = {
    { "-1", static_cast<Ark_ShadowType>(-1) },
    { "INT_MAX", static_cast<Ark_ShadowType>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_SharedTransitionEffectType, std::string>>
    testFixtureEnumSharedTransitionEffectTypeValidValues = {
        { "ARK_SHARED_TRANSITION_EFFECT_TYPE_STATIC", ARK_SHARED_TRANSITION_EFFECT_TYPE_STATIC,
            "SharedTransitionEffectType.Static" },
        { "ARK_SHARED_TRANSITION_EFFECT_TYPE_EXCHANGE", ARK_SHARED_TRANSITION_EFFECT_TYPE_EXCHANGE,
            "SharedTransitionEffectType.Exchange" },
    };

std::vector<std::tuple<std::string, Ark_SharedTransitionEffectType>>
    testFixtureEnumSharedTransitionEffectTypeInvalidValues = {
        { "-1", static_cast<Ark_SharedTransitionEffectType>(-1) },
        { "INT_MAX", static_cast<Ark_SharedTransitionEffectType>(INT_MAX) },
    };

std::vector<std::tuple<std::string, Ark_SliderBlockType, std::string>> testFixtureEnumSliderBlockTypeValidValues = {
    { "ARK_SLIDER_BLOCK_TYPE_DEFAULT", ARK_SLIDER_BLOCK_TYPE_DEFAULT, "SliderBlockType.DEFAULT" },
    { "ARK_SLIDER_BLOCK_TYPE_IMAGE", ARK_SLIDER_BLOCK_TYPE_IMAGE, "SliderBlockType.IMAGE" },
    { "ARK_SLIDER_BLOCK_TYPE_SHAPE", ARK_SLIDER_BLOCK_TYPE_SHAPE, "SliderBlockType.SHAPE" },
};

std::vector<std::tuple<std::string, Ark_SliderBlockType>> testFixtureEnumSliderBlockTypeInvalidValues = {
    { "-1", static_cast<Ark_SliderBlockType>(-1) },
    { "INT_MAX", static_cast<Ark_SliderBlockType>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_SliderInteraction, std::string>> testFixtureEnumSliderInteractionValidValues = {
    { "ARK_SLIDER_INTERACTION_SLIDE_AND_CLICK", ARK_SLIDER_INTERACTION_SLIDE_AND_CLICK,
        "SliderInteraction.SLIDE_AND_CLICK" },
    { "ARK_SLIDER_INTERACTION_SLIDE_ONLY", ARK_SLIDER_INTERACTION_SLIDE_ONLY, "SliderInteraction.SLIDE_ONLY" },
    { "ARK_SLIDER_INTERACTION_SLIDE_AND_CLICK_UP", ARK_SLIDER_INTERACTION_SLIDE_AND_CLICK_UP,
        "SliderInteraction.SLIDE_AND_CLICK_UP" },
};

std::vector<std::tuple<std::string, Ark_SliderInteraction>> testFixtureEnumSliderInteractionInvalidValues = {
    { "-1", static_cast<Ark_SliderInteraction>(-1) },
    { "INT_MAX", static_cast<Ark_SliderInteraction>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_SliderStyle, std::string>> testFixtureEnumSliderStyleValidValues = {
    { "ARK_SLIDER_STYLE_OUT_SET", ARK_SLIDER_STYLE_OUT_SET, "SliderStyle.OutSet" },
    { "ARK_SLIDER_STYLE_IN_SET", ARK_SLIDER_STYLE_IN_SET, "SliderStyle.InSet" },
    { "ARK_SLIDER_STYLE_NONE", ARK_SLIDER_STYLE_NONE, "SliderStyle.NONE" },
};

std::vector<std::tuple<std::string, Ark_SliderStyle>> testFixtureEnumSliderStyleInvalidValues = {
    { "-1", static_cast<Ark_SliderStyle>(-1) },
    { "INT_MAX", static_cast<Ark_SliderStyle>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_SymbolEffectStrategy, std::string>>
    testFixtureEnumSymbolEffectStrategyValidValues = {
        { "ARK_SYMBOL_EFFECT_STRATEGY_NONE", ARK_SYMBOL_EFFECT_STRATEGY_NONE, "SymbolEffectStrategy.NONE" },
        { "ARK_SYMBOL_EFFECT_STRATEGY_SCALE", ARK_SYMBOL_EFFECT_STRATEGY_SCALE, "SymbolEffectStrategy.SCALE" },
        { "ARK_SYMBOL_EFFECT_STRATEGY_HIERARCHICAL", ARK_SYMBOL_EFFECT_STRATEGY_HIERARCHICAL,
            "SymbolEffectStrategy.HIERARCHICAL" },
    };

std::vector<std::tuple<std::string, Ark_SymbolEffectStrategy>> testFixtureEnumSymbolEffectStrategyInvalidValues = {
    { "-1", static_cast<Ark_SymbolEffectStrategy>(-1) },
    { "INT_MAX", static_cast<Ark_SymbolEffectStrategy>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_SymbolRenderingStrategy, std::string>>
    testFixtureEnumSymbolRenderingStrategyValidValues = {
        { "ARK_SYMBOL_RENDERING_STRATEGY_SINGLE", ARK_SYMBOL_RENDERING_STRATEGY_SINGLE,
            "SymbolRenderingStrategy.SINGLE" },
        { "ARK_SYMBOL_RENDERING_STRATEGY_MULTIPLE_COLOR", ARK_SYMBOL_RENDERING_STRATEGY_MULTIPLE_COLOR,
            "SymbolRenderingStrategy.MULTIPLE_COLOR" },
        { "ARK_SYMBOL_RENDERING_STRATEGY_MULTIPLE_OPACITY", ARK_SYMBOL_RENDERING_STRATEGY_MULTIPLE_OPACITY,
            "SymbolRenderingStrategy.MULTIPLE_OPACITY" },
    };

std::vector<std::tuple<std::string, Ark_SymbolRenderingStrategy>>
    testFixtureEnumSymbolRenderingStrategyInvalidValues = {
        { "-1", static_cast<Ark_SymbolRenderingStrategy>(-1) },
        { "INT_MAX", static_cast<Ark_SymbolRenderingStrategy>(INT_MAX) },
    };

std::vector<std::tuple<std::string, Ark_TextAlign, std::string>> testFixtureEnumTextAlignValidValues = {
    { "ARK_TEXT_ALIGN_CENTER", ARK_TEXT_ALIGN_CENTER, "TextAlign.Center" },
    { "ARK_TEXT_ALIGN_START", ARK_TEXT_ALIGN_START, "TextAlign.Start" },
    { "ARK_TEXT_ALIGN_END", ARK_TEXT_ALIGN_END, "TextAlign.End" },
    { "ARK_TEXT_ALIGN_JUSTIFY", ARK_TEXT_ALIGN_JUSTIFY, "TextAlign.JUSTIFY" },
};

std::vector<std::tuple<std::string, Ark_TextAlign>> testFixtureEnumTextAlignInvalidValues = {
    { "-1", static_cast<Ark_TextAlign>(-1) },
    { "INT_MAX", static_cast<Ark_TextAlign>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_TextAreaType, std::string>> testFixtureEnumTextAreaTypeValidValues = {
    { "ARK_TEXT_AREA_TYPE_NORMAL", ARK_TEXT_AREA_TYPE_NORMAL, "TextAreaType.NORMAL" },
    { "ARK_TEXT_AREA_TYPE_NUMBER", ARK_TEXT_AREA_TYPE_NUMBER, "TextAreaType.NUMBER" },
    { "ARK_TEXT_AREA_TYPE_PHONE_NUMBER", ARK_TEXT_AREA_TYPE_PHONE_NUMBER, "TextAreaType.PHONE_NUMBER" },
    { "ARK_TEXT_AREA_TYPE_EMAIL", ARK_TEXT_AREA_TYPE_EMAIL, "TextAreaType.EMAIL" },
    { "ARK_TEXT_AREA_TYPE_NUMBER_DECIMAL", ARK_TEXT_AREA_TYPE_NUMBER_DECIMAL, "TextAreaType.NUMBER_DECIMAL" },
    { "ARK_TEXT_AREA_TYPE_URL", ARK_TEXT_AREA_TYPE_URL, "TextAreaType.URL" },
};

std::vector<std::tuple<std::string, Ark_TextAreaType>> testFixtureEnumTextAreaTypeInvalidValues = {
    { "-1", static_cast<Ark_TextAreaType>(-1) },
    { "INT_MAX", static_cast<Ark_TextAreaType>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_TextCase, std::string>> testFixtureEnumTextCaseValidValues = {
    { "ARK_TEXT_CASE_NORMAL", ARK_TEXT_CASE_NORMAL, "TextCase.Normal" },
    { "ARK_TEXT_CASE_LOWER_CASE", ARK_TEXT_CASE_LOWER_CASE, "TextCase.LowerCase" },
    { "ARK_TEXT_CASE_UPPER_CASE", ARK_TEXT_CASE_UPPER_CASE, "TextCase.UpperCase" },
};

std::vector<std::tuple<std::string, Ark_TextCase>> testFixtureEnumTextCaseInvalidValues = {
    { "-1", static_cast<Ark_TextCase>(-1) },
    { "INT_MAX", static_cast<Ark_TextCase>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_TextContentStyle, std::string>> testFixtureEnumTextContentStyleValidValues = {
    { "ARK_TEXT_CONTENT_STYLE_DEFAULT", ARK_TEXT_CONTENT_STYLE_DEFAULT, "TextContentStyle.DEFAULT" },
    { "ARK_TEXT_CONTENT_STYLE_INLINE", ARK_TEXT_CONTENT_STYLE_INLINE, "TextContentStyle.INLINE" },
};

std::vector<std::tuple<std::string, Ark_TextContentStyle>> testFixtureEnumTextContentStyleInvalidValues = {
    { "-1", static_cast<Ark_TextContentStyle>(-1) },
    { "INT_MAX", static_cast<Ark_TextContentStyle>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_TextDecorationStyle, std::string>>
    testFixtureEnumTextDecorationStyleValidValues = {
        { "ARK_TEXT_DECORATION_STYLE_SOLID", ARK_TEXT_DECORATION_STYLE_SOLID, "TextDecorationStyle.SOLID" },
        { "ARK_TEXT_DECORATION_STYLE_DOUBLE", ARK_TEXT_DECORATION_STYLE_DOUBLE, "TextDecorationStyle.DOUBLE" },
        { "ARK_TEXT_DECORATION_STYLE_DOTTED", ARK_TEXT_DECORATION_STYLE_DOTTED, "TextDecorationStyle.DOTTED" },
        { "ARK_TEXT_DECORATION_STYLE_DASHED", ARK_TEXT_DECORATION_STYLE_DASHED, "TextDecorationStyle.DASHED" },
        { "ARK_TEXT_DECORATION_STYLE_WAVY", ARK_TEXT_DECORATION_STYLE_WAVY, "TextDecorationStyle.WAVY" },
    };

std::vector<std::tuple<std::string, Ark_TextDecorationStyle>> testFixtureEnumTextDecorationStyleInvalidValues = {
    { "-1", static_cast<Ark_TextDecorationStyle>(-1) },
    { "INT_MAX", static_cast<Ark_TextDecorationStyle>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_TextDecorationType, std::string>>
    testFixtureEnumTextDecorationTypeValidValues = {
        { "ARK_TEXT_DECORATION_TYPE_NONE", ARK_TEXT_DECORATION_TYPE_NONE, "TextDecorationType.None" },
        { "ARK_TEXT_DECORATION_TYPE_UNDERLINE", ARK_TEXT_DECORATION_TYPE_UNDERLINE, "TextDecorationType.Underline" },
        { "ARK_TEXT_DECORATION_TYPE_OVERLINE", ARK_TEXT_DECORATION_TYPE_OVERLINE, "TextDecorationType.Overline" },
        { "ARK_TEXT_DECORATION_TYPE_LINE_THROUGH", ARK_TEXT_DECORATION_TYPE_LINE_THROUGH,
            "TextDecorationType.LineThrough" },
    };

std::vector<std::tuple<std::string, Ark_TextDecorationType>> testFixtureEnumTextDecorationTypeInvalidValues = {
    { "-1", static_cast<Ark_TextDecorationType>(-1) },
    { "INT_MAX", static_cast<Ark_TextDecorationType>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_TextHeightAdaptivePolicy, std::string>>
    testFixtureEnumTextHeightAdaptivePolicyValidValues = {
        { "ARK_TEXT_HEIGHT_ADAPTIVE_POLICY_MAX_LINES_FIRST", ARK_TEXT_HEIGHT_ADAPTIVE_POLICY_MAX_LINES_FIRST,
            "TextHeightAdaptivePolicy.MAX_LINES_FIRST" },
        { "ARK_TEXT_HEIGHT_ADAPTIVE_POLICY_MIN_FONT_SIZE_FIRST", ARK_TEXT_HEIGHT_ADAPTIVE_POLICY_MIN_FONT_SIZE_FIRST,
            "TextHeightAdaptivePolicy.MIN_FONT_SIZE_FIRST" },
        { "ARK_TEXT_HEIGHT_ADAPTIVE_POLICY_LAYOUT_CONSTRAINT_FIRST",
            ARK_TEXT_HEIGHT_ADAPTIVE_POLICY_LAYOUT_CONSTRAINT_FIRST,
            "TextHeightAdaptivePolicy.LAYOUT_CONSTRAINT_FIRST" },
    };

std::vector<std::tuple<std::string, Ark_TextHeightAdaptivePolicy>>
    testFixtureEnumTextHeightAdaptivePolicyInvalidValues = {
        { "-1", static_cast<Ark_TextHeightAdaptivePolicy>(-1) },
        { "INT_MAX", static_cast<Ark_TextHeightAdaptivePolicy>(INT_MAX) },
    };

std::vector<std::tuple<std::string, Ark_TextInputStyle, std::string>> testFixtureEnumTextInputStyleValidValues = {
    { "ARK_TEXT_INPUT_STYLE_DEFAULT", ARK_TEXT_INPUT_STYLE_DEFAULT, "TextInputStyle.Default" },
    { "ARK_TEXT_INPUT_STYLE_INLINE", ARK_TEXT_INPUT_STYLE_INLINE, "TextInputStyle.Inline" },
};

std::vector<std::tuple<std::string, Ark_TextInputStyle>> testFixtureEnumTextInputStyleInvalidValues = {
    { "-1", static_cast<Ark_TextInputStyle>(-1) },
    { "INT_MAX", static_cast<Ark_TextInputStyle>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_TextOverflow, std::string>> testFixtureEnumTextOverflowValidValues = {
    { "ARK_TEXT_OVERFLOW_NONE", ARK_TEXT_OVERFLOW_NONE, "TextOverflow.None" },
    { "ARK_TEXT_OVERFLOW_CLIP", ARK_TEXT_OVERFLOW_CLIP, "TextOverflow.Clip" },
    { "ARK_TEXT_OVERFLOW_ELLIPSIS", ARK_TEXT_OVERFLOW_ELLIPSIS, "TextOverflow.Ellipsis" },
    { "ARK_TEXT_OVERFLOW_MARQUEE", ARK_TEXT_OVERFLOW_MARQUEE, "TextOverflow.MARQUEE" },
};

std::vector<std::tuple<std::string, Ark_TextOverflow>> testFixtureEnumTextOverflowInvalidValues = {
    { "-1", static_cast<Ark_TextOverflow>(-1) },
    { "INT_MAX", static_cast<Ark_TextOverflow>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_TransitionHierarchyStrategy, std::string>>
    testFixtureEnumTransitionHierarchyStrategyValidValues = {
        { "ARK_TRANSITION_HIERARCHY_STRATEGY_NONE", ARK_TRANSITION_HIERARCHY_STRATEGY_NONE,
            "TransitionHierarchyStrategy.NONE" },
        { "ARK_TRANSITION_HIERARCHY_STRATEGY_ADAPTIVE", ARK_TRANSITION_HIERARCHY_STRATEGY_ADAPTIVE,
            "TransitionHierarchyStrategy.ADAPTIVE" },
    };

std::vector<std::tuple<std::string, Ark_TransitionHierarchyStrategy>>
    testFixtureEnumTransitionHierarchyStrategyInvalidValues = {
        { "-1", static_cast<Ark_TransitionHierarchyStrategy>(-1) },
        { "INT_MAX", static_cast<Ark_TransitionHierarchyStrategy>(INT_MAX) },
    };

std::vector<std::tuple<std::string, Ark_TransitionType, std::string>> testFixtureEnumTransitionTypeValidValues = {
    { "ARK_TRANSITION_TYPE_ALL", ARK_TRANSITION_TYPE_ALL, "TransitionType.All" },
    { "ARK_TRANSITION_TYPE_INSERT", ARK_TRANSITION_TYPE_INSERT, "TransitionType.Insert" },
    { "ARK_TRANSITION_TYPE_DELETE", ARK_TRANSITION_TYPE_DELETE, "TransitionType.Delete" },
};

std::vector<std::tuple<std::string, Ark_TransitionType>> testFixtureEnumTransitionTypeInvalidValues = {
    { "-1", static_cast<Ark_TransitionType>(-1) },
    { "INT_MAX", static_cast<Ark_TransitionType>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_VerticalAlign, std::string>> testFixtureEnumVerticalAlignValidValues = {
    { "ARK_VERTICAL_ALIGN_TOP", ARK_VERTICAL_ALIGN_TOP, "VerticalAlign.Top" },
    { "ARK_VERTICAL_ALIGN_CENTER", ARK_VERTICAL_ALIGN_CENTER, "VerticalAlign.Center" },
    { "ARK_VERTICAL_ALIGN_BOTTOM", ARK_VERTICAL_ALIGN_BOTTOM, "VerticalAlign.Bottom" },
};

std::vector<std::tuple<std::string, Ark_VerticalAlign>> testFixtureEnumVerticalAlignInvalidValues = {
    { "-1", static_cast<Ark_VerticalAlign>(-1) },
    { "INT_MAX", static_cast<Ark_VerticalAlign>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_Visibility, std::string>> testFixtureEnumVisibilityValidValues = {
    { "ARK_VISIBILITY_VISIBLE", ARK_VISIBILITY_VISIBLE, "Visibility.Visible" },
    { "ARK_VISIBILITY_HIDDEN", ARK_VISIBILITY_HIDDEN, "Visibility.Hidden" },
    { "ARK_VISIBILITY_NONE", ARK_VISIBILITY_NONE, "Visibility.None" },
};

std::vector<std::tuple<std::string, Ark_Visibility>> testFixtureEnumVisibilityInvalidValues = {
    { "-1", static_cast<Ark_Visibility>(-1) },
    { "INT_MAX", static_cast<Ark_Visibility>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_WaterFlowLayoutMode, std::string>>
    testFixtureEnumWaterFlowLayoutModeValidValues = {
        { "ARK_WATER_FLOW_LAYOUT_MODE_ALWAYS_TOP_DOWN", ARK_WATER_FLOW_LAYOUT_MODE_ALWAYS_TOP_DOWN,
            "WaterFlowLayoutMode.ALWAYS_TOP_DOWN" },
        { "ARK_WATER_FLOW_LAYOUT_MODE_SLIDING_WINDOW", ARK_WATER_FLOW_LAYOUT_MODE_SLIDING_WINDOW,
            "WaterFlowLayoutMode.SLIDING_WINDOW" },
    };

std::vector<std::tuple<std::string, Ark_WaterFlowLayoutMode>> testFixtureEnumWaterFlowLayoutModeInvalidValues = {
    { "-1", static_cast<Ark_WaterFlowLayoutMode>(-1) },
    { "INT_MAX", static_cast<Ark_WaterFlowLayoutMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_WebDarkMode, std::string>> testFixtureEnumWebDarkModeValidValues = {
    { "ARK_WEB_DARK_MODE_OFF", ARK_WEB_DARK_MODE_OFF, "WebDarkMode.Off" },
    { "ARK_WEB_DARK_MODE_ON", ARK_WEB_DARK_MODE_ON, "WebDarkMode.On" },
    { "ARK_WEB_DARK_MODE_AUTO", ARK_WEB_DARK_MODE_AUTO, "WebDarkMode.Auto" },
};

std::vector<std::tuple<std::string, Ark_WebDarkMode>> testFixtureEnumWebDarkModeInvalidValues = {
    { "-1", static_cast<Ark_WebDarkMode>(-1) },
    { "INT_MAX", static_cast<Ark_WebDarkMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_WebKeyboardAvoidMode, std::string>>
    testFixtureEnumWebKeyboardAvoidModeValidValues = {
        { "ARK_WEB_KEYBOARD_AVOID_MODE_RESIZE_VISUAL", ARK_WEB_KEYBOARD_AVOID_MODE_RESIZE_VISUAL,
            "WebKeyboardAvoidMode.RESIZE_VISUAL" },
        { "ARK_WEB_KEYBOARD_AVOID_MODE_RESIZE_CONTENT", ARK_WEB_KEYBOARD_AVOID_MODE_RESIZE_CONTENT,
            "WebKeyboardAvoidMode.RESIZE_CONTENT" },
        { "ARK_WEB_KEYBOARD_AVOID_MODE_OVERLAYS_CONTENT", ARK_WEB_KEYBOARD_AVOID_MODE_OVERLAYS_CONTENT,
            "WebKeyboardAvoidMode.OVERLAYS_CONTENT" },
    };

std::vector<std::tuple<std::string, Ark_WebKeyboardAvoidMode>> testFixtureEnumWebKeyboardAvoidModeInvalidValues = {
    { "-1", static_cast<Ark_WebKeyboardAvoidMode>(-1) },
    { "INT_MAX", static_cast<Ark_WebKeyboardAvoidMode>(INT_MAX) },
};

std::vector<std::tuple<std::string, Ark_WebLayoutMode, std::string>> testFixtureEnumWebLayoutModeValidValues = {
    { "ARK_WEB_LAYOUT_MODE_NONE", ARK_WEB_LAYOUT_MODE_NONE, "WebLayoutMode.NONE" },
    { "ARK_WEB_LAYOUT_MODE_FIT_CONTENT", ARK_WEB_LAYOUT_MODE_FIT_CONTENT, "WebLayoutMode.FIT_CONTENT" },
};

std::vector<std::tuple<std::string, Ark_WebLayoutMode>> testFixtureEnumWebLayoutModeInvalidValues = {
    { "-1", static_cast<Ark_WebLayoutMode>(-1) },
    { "INT_MAX", static_cast<Ark_WebLayoutMode>(INT_MAX) },
};

} // namespace OHOS::Ace::NG::Fixtures
