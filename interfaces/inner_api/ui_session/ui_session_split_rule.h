/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_INTERFACES_INNER_API_UI_SESSION_UI_SESSION_SPLIT_RULE_H
#define FOUNDATION_ACE_INTERFACES_INNER_API_UI_SESSION_UI_SESSION_SPLIT_RULE_H

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "cJSON.h"

namespace OHOS::Ace {
namespace PageSceneSplit {
constexpr int32_t DEFAULT_MIN_REPORT_INTERVAL_MS = 500;
} // namespace PageSceneSplit

// Value-only configuration shared by the IPC boundary and UI-side matcher.
// Only the SPLIT branch uses these strict checks; legacy scenes keep their parser.
struct PageSceneSplitRule {
    std::string ruleId;
    std::vector<std::string> compNames;
    bool enabled = true;
    bool allowNavigation = true;
    double heightRatioGt = 0.0f;
    double widthRatioLte = 0.0f;
    double totalWidthRatioGt = 0.0f;
    bool nonOverlapping = true;
    bool reportOnRegister = true;
    int32_t minReportIntervalMs = PageSceneSplit::DEFAULT_MIN_REPORT_INTERVAL_MS;
};

struct PageSceneSplitRuleSet {
    std::string ruleSetId;
    std::vector<PageSceneSplitRule> rules;
};

namespace PageSceneSplit {
constexpr size_t MAX_ID_LENGTH = 128;
constexpr int32_t MAX_RULES = 10;
constexpr int32_t MAX_SELECTORS = 5;
constexpr size_t MAX_RULE_SETS = 2;
constexpr double SPLIT_RULE_VERSION = 1.0;
constexpr unsigned char JSON_WHITESPACE_MAX = ' ';
using JsonPtr = std::unique_ptr<cJSON, decltype(&cJSON_Delete)>;

inline JsonPtr Parse(const std::string& json)
{
    return JsonPtr(cJSON_ParseWithOpts(json.c_str(), nullptr, true), cJSON_Delete);
}

inline const cJSON* Field(const cJSON* object, const char* name)
{
    return cJSON_GetObjectItemCaseSensitive(object, name);
}

inline bool ReadString(const cJSON* object, const char* name, std::string& value)
{
    auto item = Field(object, name);
    if (!cJSON_IsString(item) || !item->valuestring || item->valuestring[0] == '\0') {
        return false;
    }
    value = item->valuestring;
    return true;
}

inline bool IsValidId(const std::string& id)
{
    return !id.empty() && id.size() <= MAX_ID_LENGTH && std::all_of(id.begin(), id.end(), [](unsigned char ch) {
        return std::isalnum(ch) || ch == '_' || ch == '-';
    });
}

inline bool ReadOptionalBool(const cJSON* object, const char* name, bool& value)
{
    auto item = Field(object, name);
    if (!item || !cJSON_IsBool(item)) {
        return item == nullptr;
    }
    value = cJSON_IsTrue(item);
    return true;
}

inline bool ReadRatio(const cJSON* object, const char* name, double& value, double minValue)
{
    auto item = Field(object, name);
    if (!cJSON_IsNumber(item) || !std::isfinite(item->valuedouble) ||
        item->valuedouble < minValue || item->valuedouble > 1.0) {
        return false;
    }
    value = item->valuedouble;
    return true;
}

constexpr double HEIGHT_RATIO_MIN = 0.8;
constexpr double WIDTH_RATIO_MIN = 0.2;
constexpr double TOTAL_WIDTH_RATIO_MIN = 0.8;

// Exact built-in FrameNode tags from inspector_constants.cpp and NG node/model factories.
// Keep this catalog in sync when adding built-in FrameNode tags. Proxy validation must work
// without a live UI tree or a link dependency on inspector/component implementations.
// Syntax-only UINode tags (ForEach, Repeat, Span, etc.) and arbitrary custom tags are not selectors.
constexpr const char* SPLIT_FRAME_NODE_TAGS[] = { "AbilityComponent", "Accessibility_focus_paint_node", "ActionSheet",
    "AlertDialog", "AlphabetIndexer", "AppBar", "ArcAlphabetIndexer", "ArcList", "ArcListItem", "ArcScrollBar",
    "ArcSwiper", "AtomicService", "BackButton", "BackButtonImage", "BackgroundMask", "Badge", "BarItem", "Blank",
    "Button", "Calendar", "CalendarPicker", "Canvas", "Checkbox", "CheckboxGroup", "Circle", "CloseIcon", "Column",
    "ColumnSplit", "Component3D", "ContainerModal", "ControlParts", "Counter", "CustomFrameNode", "CustomSpanNode",
    "DataPanel", "DatePicker", "DepthComponent", "Dialog", "DistortionComponent", "DistortionContent", "Divider",
    "DragBar", "DynamicComponent", "EffectComponent", "Ellipse", "EmbeddedComponent", "Flex", "FlowItem", "Folder",
    "FormButton", "FormComponent", "FormLink", "Gauge", "GaugeDescription", "Grid", "GridCol", "GridContainer",
    "GridItem", "GridRow", "Hover", "Hyperlink", "Image", "ImageAnimator", "IndicatorComponent", "IsolatedComponent",
    "JsView", "Keyboard", "LazyVGridLayout", "LeftArrow", "Line", "LinearIndicator", "List", "ListItem",
    "ListItemGroup", "LoadingProgress", "Magnifier", "Marquee", "Menu", "MenuDivider", "MenuItem", "MenuItemGroup",
    "MenuPreview", "MenuWrapper", "ModalPage", "NavBar", "NavBarContent", "NavDestination", "NavDestinationContent",
    "NavRouter", "Navigation", "NavigationContent", "NavigationFullScreenOverlay", "NavigationMenu", "Navigator",
    "NodeContainer", "Option", "OrderOverlay", "Panel", "Particle", "PasteButton", "Path", "PatternLock",
    "PlaceholderSpan", "PluginComponent", "Polygon", "Polyline", "Popup", "PrimaryContent", "Progress", "QRCode",
    "Radio", "Rating", "Rect", "Refresh", "RelativeContainer", "RemoteWindow", "RenderNode", "RichEditor",
    "RichEditorContent", "RichEditorDRAG", "RightArrow", "RootScene", "Row", "RowSplit", "SaveButton", "Screen",
    "Scroll", "ScrollBar", "ScrollPlaceholderShell", "Search", "SearchField", "Select", "SelectOverlay",
    "SelectionContainer", "Shape", "SheetPage", "SheetWrapper", "SideBarContainer", "Slider",
    "SplitPlaceholderContentNode", "Stack", "Stepper", "StepperItem", "Swiper", "SwiperIndicator", "SwiperLeftCapture",
    "SwiperRightCapture", "SymbolGlyph", "TabBar", "TabContent", "Tabs", "TabsSideBar", "TabsSideBarTabList", "Text",
    "TextArea", "TextClock", "TextDrag", "TextInput", "TextPicker", "TextTimer", "TimePicker", "TitleBar", "Toast",
    "Toggle", "ToolBar", "ToolBarItem", "Tool_Bar", "UIExtensionComponent", "UIExtensionSurface", "UIPickerComponent",
    "UnionEffectContainer", "V2::ListComponent", "Video", "WaterFlow", "Web", "WindowScene", "XComponent", "__Common__",
    "overlay", "page", "root", "stage" };

inline bool IsValidFrameNodeTag(const std::string& name)
{
    for (const auto* tag : SPLIT_FRAME_NODE_TAGS) {
        if (name == tag) {
            return true;
        }
    }
    return false;
}

inline bool AppendSelector(const cJSON* selector, std::set<std::string>& names, PageSceneSplitRule& rule)
{
    std::string name;
    if (!cJSON_IsObject(selector) || !ReadString(selector, "compName", name) || !IsValidFrameNodeTag(name) ||
        !names.insert(name).second) {
        return false;
    }
    rule.compNames.emplace_back(std::move(name));
    return true;
}

inline bool AppendSelectors(const cJSON* selectors, PageSceneSplitRule& rule)
{
    std::set<std::string> names;
    for (int32_t i = 0; i < cJSON_GetArraySize(selectors); ++i) {
        if (!AppendSelector(cJSON_GetArrayItem(selectors, i), names, rule)) {
            return false;
        }
    }
    return true;
}

inline bool ReadSelectors(const cJSON* custom, PageSceneSplitRule& rule)
{
    auto selectors = Field(custom, "selectors");
    return cJSON_IsArray(selectors) && cJSON_GetArraySize(selectors) > 0 &&
           cJSON_GetArraySize(selectors) <= MAX_SELECTORS && AppendSelectors(selectors, rule);
}

inline bool ReadGeometryRatios(const cJSON* geometry, PageSceneSplitRule& rule)
{
    return cJSON_IsObject(geometry) &&
           ReadRatio(geometry, "heightRatioGt", rule.heightRatioGt, HEIGHT_RATIO_MIN) &&
           ReadRatio(geometry, "widthRatioLte", rule.widthRatioLte, WIDTH_RATIO_MIN) &&
           ReadRatio(geometry, "totalWidthRatioGt", rule.totalWidthRatioGt, TOTAL_WIDTH_RATIO_MIN);
}

inline bool ReadGeometry(const cJSON* custom, PageSceneSplitRule& rule)
{
    auto geometry = Field(custom, "geometry");
    auto overlapping = Field(geometry, "nonOverlapping");
    if (!ReadGeometryRatios(geometry, rule) || !cJSON_IsBool(overlapping)) {
        return false;
    }
    rule.nonOverlapping = cJSON_IsTrue(overlapping);
    return true;
}

inline bool ParseCustom(const cJSON* custom, PageSceneSplitRule& rule)
{
    return cJSON_IsObject(custom) && ReadSelectors(custom, rule) && ReadGeometry(custom, rule);
}

inline bool IsValidInterval(const cJSON* interval)
{
    return cJSON_IsNumber(interval) && std::isfinite(interval->valuedouble) && interval->valuedouble >= 0.0 &&
           interval->valuedouble <= std::numeric_limits<int32_t>::max() &&
           std::floor(interval->valuedouble) == interval->valuedouble;
}

inline bool ReadReportInterval(const cJSON* policy, PageSceneSplitRule& rule)
{
    auto interval = Field(policy, "minReportIntervalMs");
    if (interval && !IsValidInterval(interval)) {
        return false;
    }
    if (interval) {
        rule.minReportIntervalMs = static_cast<int32_t>(interval->valuedouble);
    }
    return true;
}

inline bool ReadPolicy(const cJSON* object, PageSceneSplitRule& rule)
{
    auto policy = Field(object, "policy");
    return !policy || (cJSON_IsObject(policy) && ReadOptionalBool(policy, "reportOnRegister", rule.reportOnRegister) &&
                          ReadReportInterval(policy, rule));
}

inline bool ReadBasicRule(const cJSON* object, PageSceneSplitRule& rule)
{
    std::string sceneType;
    return cJSON_IsObject(object) && ReadString(object, "ruleId", rule.ruleId) && IsValidId(rule.ruleId) &&
           ReadString(object, "sceneType", sceneType) && sceneType == "SPLIT" &&
           ReadOptionalBool(object, "enabled", rule.enabled) &&
           ReadOptionalBool(object, "navigation", rule.allowNavigation) && ReadPolicy(object, rule);
}

inline std::optional<PageSceneSplitRule> ParseRule(const cJSON* object)
{
    PageSceneSplitRule rule;
    auto custom = Field(object, "custom");
    if (!ReadBasicRule(object, rule) || (custom ? !ParseCustom(custom, rule) : !rule.allowNavigation)) {
        return std::nullopt;
    }
    return rule;
}

inline bool IsSplitRuleItem(const cJSON* item)
{
    std::string type;
    return ReadString(item, "sceneType", type) && type == "SPLIT";
}

inline bool ContainsSplitRule(const cJSON* root)
{
    auto rules = Field(root, "rules");
    if (!cJSON_IsArray(rules)) {
        return false;
    }
    for (int32_t i = 0; i < cJSON_GetArraySize(rules); ++i) {
        if (IsSplitRuleItem(cJSON_GetArrayItem(rules, i))) {
            return true;
        }
    }
    return false;
}

inline bool ContainsSplitRule(const std::string& json)
{
    auto root = Parse(json);
    return root && ContainsSplitRule(root.get());
}

inline bool ReadRuleSetItemType(const cJSON* item, std::set<std::string>& ids, std::string& sceneType)
{
    std::string id;
    return cJSON_IsObject(item) && ReadString(item, "ruleId", id) && IsValidId(id) && ids.insert(id).second &&
           ReadString(item, "sceneType", sceneType);
}

inline bool AppendSplitRule(const cJSON* item, PageSceneSplitRuleSet& result)
{
    auto rule = ParseRule(item);
    if (!rule) {
        return false;
    }
    result.rules.emplace_back(std::move(*rule));
    return true;
}

inline bool ReadRuleSetItem(const cJSON* item, std::set<std::string>& ids, PageSceneSplitRuleSet& result)
{
    std::string sceneType;
    if (!ReadRuleSetItemType(item, ids, sceneType)) {
        return false;
    }
    if (sceneType != "SPLIT") {
        // Other scene types retain their own validation and are not collected as SPLIT rules.
        return true;
    }
    return AppendSplitRule(item, result);
}

inline bool ReadRuleSetHeader(const cJSON* root, PageSceneSplitRuleSet& result, const cJSON*& rules)
{
    auto version = Field(root, "version");
    rules = Field(root, "rules");
    return cJSON_IsObject(root) && cJSON_IsNumber(version) && version->valuedouble == SPLIT_RULE_VERSION &&
           ReadString(root, "ruleSetId", result.ruleSetId) && IsValidId(result.ruleSetId) && cJSON_IsArray(rules) &&
           cJSON_GetArraySize(rules) > 0 && cJSON_GetArraySize(rules) <= MAX_RULES;
}

inline bool ReadRuleSetItems(const cJSON* rules, PageSceneSplitRuleSet& result)
{
    std::set<std::string> ids;
    for (int32_t i = 0; i < cJSON_GetArraySize(rules); ++i) {
        if (!ReadRuleSetItem(cJSON_GetArrayItem(rules, i), ids, result)) {
            return false;
        }
    }
    return !result.rules.empty();
}

// Borrows the tree and tail for this call; parseEnd must point to a valid C string.
inline std::optional<PageSceneSplitRuleSet> ParseRuleSet(const cJSON* root, const char* parseEnd)
{
    // cJSON accepts bytes up to ASCII space as trailing whitespace.
    const auto* tailEnd = parseEnd + std::char_traits<char>::length(parseEnd);
    const bool hasTrailingData =
        std::any_of(parseEnd, tailEnd, [](unsigned char ch) { return ch > JSON_WHITESPACE_MAX; });
    PageSceneSplitRuleSet result;
    const cJSON* rules = nullptr;
    if (hasTrailingData || !ReadRuleSetHeader(root, result, rules) || !ReadRuleSetItems(rules, result)) {
        return std::nullopt;
    }
    return result;
}

inline std::optional<PageSceneSplitRuleSet> ParseRuleSet(const std::string& json)
{
    auto root = Parse(json);
    return ParseRuleSet(root.get(), "");
}
} // namespace PageSceneSplit
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_INTERFACES_INNER_API_UI_SESSION_UI_SESSION_SPLIT_RULE_H
