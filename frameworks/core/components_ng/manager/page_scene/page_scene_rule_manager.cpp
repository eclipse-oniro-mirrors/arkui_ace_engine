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

#include "core/components_ng/manager/page_scene/page_scene_rule_manager.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>

#include "base/geometry/ng/rect_t.h"
#include "base/json/json_util.h"
#include "base/log/log_wrapper.h"
#include "base/utils/time_util.h"
#include "base/utils/utf_helper.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/event/focus_hub.h"
#include "core/components_ng/pattern/rich_editor/bridge/rich_editor_custom_modifier.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_layout_property.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_layout_property.h"
#include "core/components_ng/pattern/text_field/text_field_type_utils.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/interfaces/native/node/node_text_input_modifier.h"
#include "core/interfaces/native/node/rich_editor_modifier.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t DEFAULT_MIN_REPORT_INTERVAL_MS = 500;
constexpr int32_t MAX_ID_LENGTH = 128;
constexpr int32_t PAGE_SCENE_RULE_VERSION = 1;
constexpr int32_t PAGE_SCENE_NO_ERROR = 0;
constexpr int32_t PAGE_SCENE_PARAM_INVALID = 4;
constexpr int32_t PAGE_SCENE_LAST_UNFINISH = 5;
const char TEXT_EDITOR_SCENE[] = "TEXT_EDITOR";
const char EXIT_EVENT_SUFFIX[] = "_EXIT";
const char COUNT_GTE_OPERATOR[] = "COUNT_GTE";
const char SOURCE_ARKUI[] = "ARKUI";
const char SEARCH_FIELD_TAG[] = "SearchField";

constexpr int32_t MAX_SEARCH_RULE_SETS_PER_PROCESS = 2;
constexpr int32_t MAX_SEARCH_RULES_PER_RULE_SET = 10;
constexpr int32_t MAX_SEARCH_REGIONS_PER_RULE = 5;
constexpr int32_t MAX_SEARCH_KEYWORDS_PER_RULE = 5;
constexpr int32_t MAX_SEARCH_KEYWORD_UTF8_BYTES = 20;
constexpr int32_t SEARCH_COMPONENT_TYPES = 5;
// Bound retained input when the next stable detection point is delayed or never arrives.
constexpr size_t MAX_PENDING_SEARCH_CLICKS = 256;
constexpr int64_t SEARCH_CLICK_EXPIRATION_MS = 5000;
constexpr int64_t NANOSECONDS_PER_MILLISECOND = 1000000;
const char PAGE_TYPE_NAVIGATION[] = "Navigation";
const char PAGE_TYPE_ROUTER[] = "Router";
const char SEARCH_COMPONENT_IMAGE[] = "Image";
const char SEARCH_COMPONENT_SWIPER[] = "Swiper";
const char SEARCH_COMPONENT_SEARCH[] = "Search";
const char SEARCH_COMPONENT_BUTTON[] = "Button";
const char SEARCH_COMPONENT_TEXT_INPUT[] = "TextInput";

bool IsValidSearchComponentType(const std::string& type)
{
    return type == SEARCH_COMPONENT_IMAGE || type == SEARCH_COMPONENT_SWIPER ||
           type == SEARCH_COMPONENT_SEARCH || type == SEARCH_COMPONENT_BUTTON ||
           type == SEARCH_COMPONENT_TEXT_INPUT;
}

bool IsPositiveFiniteNumber(double value)
{
    return value > 0.0 && std::isfinite(value);
}

std::unique_ptr<JsonValue> GetObjectValue(const std::unique_ptr<JsonValue>& json, const std::string& key);

bool HasPositiveAreaOverlap(const PageSceneRectInfo& left, const PageSceneRectInfo& right)
{
    float overlapWidth = std::min(left.x + left.width, right.x + right.width) - std::max(left.x, right.x);
    float overlapHeight = std::min(left.y + left.height, right.y + right.height) - std::max(left.y, right.y);
    return overlapWidth > 0.0f && overlapHeight > 0.0f;
}

std::optional<SearchComponentFeature> ParseSearchComponent(const std::unique_ptr<JsonValue>& component)
{
    if (!component || !component->IsObject()) {
        return std::nullopt;
    }
    SearchComponentFeature feature;
    feature.type = component->GetString("type");
    if (!IsValidSearchComponentType(feature.type)) {
        return std::nullopt;
    }
    auto maxSize = GetObjectValue(component, "maxSize");
    if (maxSize) {
        auto width = maxSize->GetValue("width");
        auto height = maxSize->GetValue("height");
        if (!width || !width->IsNumber() || !height || !height->IsNumber() ||
            !IsPositiveFiniteNumber(width->GetDouble()) || !IsPositiveFiniteNumber(height->GetDouble())) {
            return std::nullopt;
        }
        feature.maxWidth = static_cast<float>(width->GetDouble());
        feature.maxHeight = static_cast<float>(height->GetDouble());
    }
    auto maxRatio = component->GetValue("maxRatio");
    if (maxRatio && !maxRatio->IsNull()) {
        if (!maxRatio->IsNumber() || !IsPositiveFiniteNumber(maxRatio->GetDouble())) {
            return std::nullopt;
        }
        feature.maxRatio = static_cast<float>(maxRatio->GetDouble());
    }
    auto autoPlay = component->GetValue("autoPlay");
    if (autoPlay && !autoPlay->IsNull()) {
        if (!autoPlay->IsBool()) {
            return std::nullopt;
        }
        feature.autoPlay = autoPlay->GetBool();
    }
    return feature;
}

bool ParseSearchRegions(const std::unique_ptr<JsonValue>& ruleJson, SearchSceneRule& rule)
{
    auto regions = ruleJson->GetValue("regions");
    if (!regions || !regions->IsArray() || regions->GetArraySize() <= 0 ||
        regions->GetArraySize() > MAX_SEARCH_REGIONS_PER_RULE) {
        return false;
    }
    for (int32_t index = 0; index < regions->GetArraySize(); ++index) {
        auto region = regions->GetArrayItem(index);
        if (!region || !region->IsObject()) {
            return false;
        }
        auto x = region->GetValue("x");
        auto y = region->GetValue("y");
        auto width = region->GetValue("width");
        auto height = region->GetValue("height");
        if (!x || !x->IsNumber() || !y || !y->IsNumber() ||
            !width || !width->IsNumber() || !height || !height->IsNumber()) {
            return false;
        }
        PageSceneRectInfo rect;
        rect.x = static_cast<float>(x->GetDouble());
        rect.y = static_cast<float>(y->GetDouble());
        rect.width = static_cast<float>(width->GetDouble());
        rect.height = static_cast<float>(height->GetDouble());
        if (!IsPositiveFiniteNumber(rect.width) || !IsPositiveFiniteNumber(rect.height)) {
            return false;
        }
        for (const auto& existing : rule.regions) {
            if (HasPositiveAreaOverlap(existing, rect)) {
                return false;
            }
        }
        rule.regions.emplace_back(rect);
    }
    return true;
}

bool ParseSearchKeywords(const std::unique_ptr<JsonValue>& ruleJson, SearchSceneRule& rule)
{
    auto keywords = ruleJson->GetValue("keywords");
    if (keywords && !keywords->IsNull()) {
        if (!keywords->IsArray() || keywords->GetArraySize() > MAX_SEARCH_KEYWORDS_PER_RULE) {
            return false;
        }
        for (int32_t index = 0; index < keywords->GetArraySize(); ++index) {
            auto item = keywords->GetArrayItem(index);
            if (!item || !item->IsString()) {
                return false;
            }
            auto keyword = item->GetString();
            if (keyword.empty() || keyword.size() > MAX_SEARCH_KEYWORD_UTF8_BYTES) {
                return false;
            }
            rule.keywords.emplace_back(keyword);
        }
    }
    return true;
}

bool ParseSearchComponents(const std::unique_ptr<JsonValue>& ruleJson, SearchSceneRule& rule)
{
    auto components = ruleJson->GetValue("components");
    if (components && !components->IsNull()) {
        if (!components->IsArray() || components->GetArraySize() > SEARCH_COMPONENT_TYPES) {
            return false;
        }
        std::set<std::string> componentTypes;
        for (int32_t index = 0; index < components->GetArraySize(); ++index) {
            auto feature = ParseSearchComponent(components->GetArrayItem(index));
            if (!feature.has_value()) {
                return false;
            }
            if (componentTypes.find(feature->type) != componentTypes.end()) {
                return false;
            }
            componentTypes.insert(feature->type);
            rule.components.emplace_back(feature.value());
        }
    }
    return true;
}

bool ParseSearchInspectorIds(const std::unique_ptr<JsonValue>& ruleJson, SearchSceneRule& rule)
{
    auto inspectorIds = ruleJson->GetValue("inspectorIds");
    if (inspectorIds && !inspectorIds->IsNull()) {
        if (!inspectorIds->IsArray()) {
            return false;
        }
        std::set<std::string> idSet;
        for (int32_t index = 0; index < inspectorIds->GetArraySize(); ++index) {
            auto item = inspectorIds->GetArrayItem(index);
            if (!item || !item->IsString()) {
                return false;
            }
            auto id = item->GetString();
            if (id.empty()) {
                return false;
            }
            if (idSet.find(id) == idSet.end()) {
                idSet.insert(id);
                rule.inspectorIds.emplace_back(id);
            }
        }
    }
    return true;
}

std::string ExtractTextFieldText(const RefPtr<FrameNode>& node)
{
    CHECK_NULL_RETURN(node, "");
    auto customModifier = NodeModifier::GetTextInputCustomModifier();
    CHECK_NULL_RETURN(customModifier, "");
    CHECK_NULL_RETURN(customModifier->getTextByPattern, "");
    std::string text;
    if (customModifier->getTextByPattern(node, text)) {
        return text;
    }
    return "";
}

std::string ExtractRichEditorText(const RefPtr<FrameNode>& node)
{
    CHECK_NULL_RETURN(node, "");
    auto customModifier = NodeModifier::GetRichEditorCustomModifier();
    CHECK_NULL_RETURN(customModifier, "");
    CHECK_NULL_RETURN(customModifier->getContentBySpans, "");
    std::u16string text;
    if (!customModifier->getContentBySpans(node, text)) {
        return "";
    }
    return UtfUtils::Str16DebugToStr8(text);
}

bool IsValidId(const std::string& id)
{
    if (id.empty() || id.size() > MAX_ID_LENGTH) {
        return false;
    }
    return std::all_of(id.begin(), id.end(), [](char ch) {
        return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == '-';
    });
}

std::unique_ptr<JsonValue> GetObjectValue(const std::unique_ptr<JsonValue>& json, const std::string& key)
{
    if (!json || !json->IsObject()) {
        return nullptr;
    }
    auto value = json->GetValue(key);
    if (!value || !value->IsObject()) {
        return nullptr;
    }
    return value;
}

std::string NormalizeNodeType(const std::string& tag)
{
    if (tag == V2::TEXTINPUT_ETS_TAG) {
        return "TextInput";
    }
    if (tag == V2::TEXTAREA_ETS_TAG) {
        return "TextArea";
    }
    if (tag == V2::SEARCH_ETS_TAG) {
        return "Search";
    }
    if (tag == V2::RICH_EDITOR_ETS_TAG) {
        return "RichEditor";
    }
    return tag;
}

bool IsPageSceneInputControlTag(const std::string& tag)
{
    return tag == V2::TEXTINPUT_ETS_TAG || tag == V2::TEXTAREA_ETS_TAG ||
           tag == V2::SEARCH_ETS_TAG || tag == V2::RICH_EDITOR_ETS_TAG;
}

std::unique_ptr<JsonValue> BuildRectJson(const PageSceneRectInfo& rect)
{
    auto rectJson = JsonUtil::Create(true);
    rectJson->Put("x", static_cast<double>(rect.x));
    rectJson->Put("y", static_cast<double>(rect.y));
    rectJson->Put("width", static_cast<double>(rect.width));
    rectJson->Put("height", static_cast<double>(rect.height));
    return rectJson;
}

RefPtr<FrameNode> FindSearchFieldNode(const RefPtr<FrameNode>& node)
{
    CHECK_NULL_RETURN(node, nullptr);
    for (const auto& child : node->GetChildrenForInspector(true)) {
        auto childFrameNode = AceType::DynamicCast<FrameNode>(child);
        if (childFrameNode && childFrameNode->GetTag() == SEARCH_FIELD_TAG) {
            return childFrameNode;
        }
    }
    return nullptr;
}

void FillTextFieldInfo(const RefPtr<FrameNode>& node, PageSceneNodeInfo& info)
{
    CHECK_NULL_VOID(node);
    info.text = ExtractTextFieldText(node);
    auto layoutProperty = node->GetLayoutProperty<TextFieldLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    if (layoutProperty->HasPlaceholder()) {
        info.placeholder = UtfUtils::Str16DebugToStr8(layoutProperty->GetPlaceholderValue());
    }
    const bool isTextArea = node->GetTag() == V2::TEXTAREA_ETS_TAG;
    auto contentType = layoutProperty->GetTextContentTypeValue(TextContentType::UNSPECIFIED);
    auto inputType = layoutProperty->GetTextInputTypeValue(TextInputType::UNSPECIFIED);
    info.contentType = TextFieldTypeUtils::ToContentTypeName(contentType);
    info.inputType = TextFieldTypeUtils::ToInputTypeName(inputType, isTextArea, !isTextArea);
}

void FillRichEditorInfo(const RefPtr<FrameNode>& node, PageSceneNodeInfo& info)
{
    CHECK_NULL_VOID(node);
    info.text = ExtractRichEditorText(node);
    auto layoutProperty = node->GetLayoutProperty<RichEditorLayoutProperty>();
    if (layoutProperty && layoutProperty->HasPlaceholder()) {
        info.placeholder = UtfUtils::Str16DebugToStr8(layoutProperty->GetPlaceholderValue());
    }
}

void FillInputInfo(const RefPtr<FrameNode>& node, PageSceneNodeInfo& info)
{
    CHECK_NULL_VOID(node);
    auto tag = node->GetTag();
    if (tag == V2::TEXTINPUT_ETS_TAG || tag == V2::TEXTAREA_ETS_TAG || tag == SEARCH_FIELD_TAG) {
        FillTextFieldInfo(node, info);
        return;
    }
    if (tag == V2::SEARCH_ETS_TAG) {
        FillTextFieldInfo(FindSearchFieldNode(node), info);
        return;
    }
    if (tag == V2::RICH_EDITOR_ETS_TAG) {
        FillRichEditorInfo(node, info);
    }
}

// enableAutoFill can only be set on TextInput/TextArea nodes: Search has no such API and its
// internal SearchField node is not publicly configurable, RichEditor has no EnableAutoFill property.
void FillEnableAutoFillInfo(const RefPtr<FrameNode>& node, PageSceneNodeInfo& info)
{
    CHECK_NULL_VOID(node);
    auto tag = node->GetTag();
    if (tag != V2::TEXTINPUT_ETS_TAG && tag != V2::TEXTAREA_ETS_TAG) {
        return;
    }
    auto layoutProperty = node->GetLayoutProperty<TextFieldLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    if (layoutProperty->HasEnableAutoFill()) {
        info.enableAutoFill = layoutProperty->GetEnableAutoFillValue(true);
    }
}

std::string BuildPageSceneEventName(const std::string& sceneType, bool matched, bool forceReportUnmatched)
{
    if (!matched && !forceReportUnmatched) {
        return sceneType + EXIT_EVENT_SUFFIX;
    }
    return sceneType;
}
} // namespace

void PageSceneInputCountTracker::Initialize(
    const PageSceneRuleSet& ruleSet, const PageSceneRule& rule, const DumpStartNodeSet& startNodes)
{
    Reset();
    if (rule.rectCulling && startNodes.dumpBeginNode) {
        pageViewportRect_ = startNodes.dumpBeginNode->GetTransformRectRelativeToWindow(true);
    }
    for (const auto& node : startNodes.pageStartNodes) {
        CollectInputNodes(ruleSet, rule, node);
    }
    for (const auto& node : startNodes.overlayNodes) {
        CollectInputNodes(ruleSet, rule, node);
    }
    std::sort(visibleInputNodes_.begin(), visibleInputNodes_.end(), [](const auto& left, const auto& right) {
        return left.nodeId < right.nodeId;
    });
}

void PageSceneInputCountTracker::Reset()
{
    visibleInputNodes_.clear();
    pageViewportRect_ = RectF();
}

const std::vector<PageSceneNodeInfo>& PageSceneInputCountTracker::GetVisibleInputNodes() const
{
    return visibleInputNodes_;
}

bool PageSceneInputCountTracker::IsOpacityVisible(const RefPtr<FrameNode>& node) const
{
    double finalOpacity = 1.0;
    auto current = node;
    while (current) {
        auto renderContext = current->GetRenderContext();
        if (renderContext) {
            finalOpacity *= renderContext->GetOpacityValue(1.0);
            if (NearZero(finalOpacity)) {
                return false;
            }
        }
        current = current->GetAncestorNodeOfFrame(true);
    }
    return !NearZero(finalOpacity);
}

void PageSceneInputCountTracker::CollectInputNodes(
    const PageSceneRuleSet& ruleSet, const PageSceneRule& rule, const RefPtr<UINode>& node)
{
    CHECK_NULL_VOID(node);
    auto frameNode = AceType::DynamicCast<FrameNode>(node);
    if (frameNode) {
        auto nodeInfo = BuildNodeInfo(ruleSet, rule, frameNode);
        if (nodeInfo.has_value()) {
            visibleInputNodes_.emplace_back(nodeInfo.value());
            return;
        }
        if (IsPageSceneInputControlTag(frameNode->GetTag())) {
            return;
        }
    }
    for (const auto& child : node->GetChildrenForInspector(true)) {
        CollectInputNodes(ruleSet, rule, child);
    }
}

std::optional<PageSceneNodeInfo> PageSceneInputCountTracker::BuildNodeInfo(
    const PageSceneRuleSet& ruleSet, const PageSceneRule& rule, const RefPtr<FrameNode>& node) const
{
    CHECK_NULL_RETURN(node, std::nullopt);
    if (!IsPageSceneInputControlTag(node->GetTag())) {
        return std::nullopt;
    }
    auto nodeType = NormalizeNodeType(node->GetTag());
    if (rule.nodeTypes.find(nodeType) == rule.nodeTypes.end()) {
        return std::nullopt;
    }

    const bool needRect = rule.onlyVisible || rule.rectCulling || rule.includeRect;
    RectF rect;
    if (needRect) {
        rect = rule.onlyVisible ? node->GetTransformRectRelativeToWindowOnlyVisible(true)
                                : node->GetTransformRectRelativeToWindow(true);
    }
    if (rule.onlyVisible) {
        const bool visible = node->IsVisibleAndActive() && IsOpacityVisible(node) &&
            rect.Width() > 0.0f && rect.Height() > 0.0f;
        if (!visible) {
            return std::nullopt;
        }
    }
    if (rule.rectCulling && !pageViewportRect_.IsInnerIntersectWith(rect)) {
        return std::nullopt;
    }

    auto focusHub = node->GetFocusHub();
    bool focusable = focusHub != nullptr && focusHub->IsFocusable();
    if (!ruleSet.includeUnfocusableTextInput && !focusable) {
        return std::nullopt;
    }

    PageSceneNodeInfo info;
    info.nodeId = node->GetId();
    info.nodeType = nodeType;
    if (rule.includeText) {
        FillInputInfo(node, info);
    }
    FillEnableAutoFillInfo(node, info);
    info.focusable = focusable;
    info.rect.x = rect.GetX();
    info.rect.y = rect.GetY();
    info.rect.width = rect.Width();
    info.rect.height = rect.Height();
    return info;
}

PageSceneRuleManager::~PageSceneRuleManager()
{
    UnregisterSearchClickObserver(searchClickObserverId_);
}

int32_t PageSceneRuleManager::RegisterRuleSet(int32_t processId, const std::string& ruleJson)
{
    // Search scene ruleJson (frozen schema) carries no "version" field; the legacy
    // TEXT_EDITOR ruleJson requires version == 1. Route by presence of "version".
    auto root = JsonUtil::ParseJsonString(ruleJson);
    auto version = (root && root->IsObject()) ? root->GetValue("version") : nullptr;
    if (!version || version->IsNull()) {
        return RegisterSearchRuleSet(processId, ruleJson);
    }
    auto ruleSet = ParseRuleSet(ruleJson);
    if (!ruleSet.has_value()) {
        return PAGE_SCENE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (registeredRuleSets_.find(processId) != registeredRuleSets_.end()) {
        return PAGE_SCENE_LAST_UNFINISH;
    }
    registeredRuleSets_[processId] = ruleSet.value();
    return PAGE_SCENE_NO_ERROR;
}

int32_t PageSceneRuleManager::UnregisterRuleSet(int32_t processId, const std::string& ruleSetId)
{
    if (ruleSetId.empty()) {
        return PAGE_SCENE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = registeredRuleSets_.find(processId);
    if (iter == registeredRuleSets_.end() || iter->second.ruleSetId != ruleSetId) {
        return PAGE_SCENE_PARAM_INVALID;
    }
    registeredRuleSets_.erase(iter);
    pendingGetProcesses_.erase(processId);
    for (auto stateIter = reportStates_.begin(); stateIter != reportStates_.end();) {
        if (stateIter->first.find(std::to_string(processId) + ":") == 0) {
            stateIter = reportStates_.erase(stateIter);
        } else {
            ++stateIter;
        }
    }
    return PAGE_SCENE_NO_ERROR;
}

int32_t PageSceneRuleManager::RegisterSearchRuleSet(int32_t processId, const std::string& ruleJson)
{
    auto ruleSet = ParseSearchRuleSet(ruleJson);
    if (!ruleSet.has_value()) {
        return PAGE_SCENE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto processIter = searchRuleSets_.find(processId);
    if (processIter == searchRuleSets_.end()) {
        searchRuleSets_[processId][ruleSet->ruleSetId] = ruleSet.value();
        return PAGE_SCENE_NO_ERROR;
    }
    auto existing = processIter->second.find(ruleSet->ruleSetId);
    if (existing != processIter->second.end()) {
        const auto& currentGeneration = existing->second.registrationGeneration;
        const auto& newGeneration = ruleSet->registrationGeneration;
        // The adapter uses canonical decimal generations. Permit a newer
        // registration to replace a cache entry whose cleanup is still queued.
        bool isNewer = !newGeneration.empty() && (newGeneration.size() > currentGeneration.size() ||
            (newGeneration.size() == currentGeneration.size() && newGeneration > currentGeneration));
        if (!isNewer) {
            return PAGE_SCENE_PARAM_INVALID;
        }
        existing->second = ruleSet.value();
        std::lock_guard<std::mutex> presenceLock(presenceStateMutex_);
        presenceStateMachine_.ClearRuleSet(std::to_string(processId), ruleSet->ruleSetId);
        return PAGE_SCENE_NO_ERROR;
    }
    if (static_cast<int32_t>(processIter->second.size()) >= MAX_SEARCH_RULE_SETS_PER_PROCESS) {
        // A third ruleSet for the same SA is rejected (AC-1.15).
        return PAGE_SCENE_PARAM_INVALID;
    }
    processIter->second[ruleSet->ruleSetId] = ruleSet.value();
    return PAGE_SCENE_NO_ERROR;
}

int32_t PageSceneRuleManager::UnregisterSearchRuleSet(
    int32_t processId, const std::string& ruleSetId, const std::string& generation)
{
    if (ruleSetId.empty()) {
        return PAGE_SCENE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto processIter = searchRuleSets_.find(processId);
    if (processIter == searchRuleSets_.end()) {
        return PAGE_SCENE_PARAM_INVALID;
    }
    auto setIter = processIter->second.find(ruleSetId);
    if (setIter == processIter->second.end()) {
        return PAGE_SCENE_PARAM_INVALID;
    }
    if (!generation.empty() && setIter->second.registrationGeneration != generation) {
        return PAGE_SCENE_PARAM_INVALID;
    }
    {
        std::lock_guard<std::mutex> presenceLock(presenceStateMutex_);
        presenceStateMachine_.ClearRuleSet(std::to_string(processId), ruleSetId);
    }
    processIter->second.erase(setIter);
    if (processIter->second.empty()) {
        searchRuleSets_.erase(processIter);
        std::lock_guard<std::mutex> presenceLock(presenceStateMutex_);
        presenceStateMachine_.ClearPrefix(std::to_string(processId) + ":");
    }
    StopSearchClickObservationIfIdleLocked();
    return PAGE_SCENE_NO_ERROR;
}

std::optional<SearchRuleSet> PageSceneRuleManager::GetSearchRuleSet(
    int32_t processId, const std::string& ruleSetId) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto processIter = searchRuleSets_.find(processId);
    if (processIter == searchRuleSets_.end()) {
        return std::nullopt;
    }
    auto setIter = processIter->second.find(ruleSetId);
    if (setIter == processIter->second.end()) {
        return std::nullopt;
    }
    return setIter->second;
}

bool PageSceneRuleManager::HasRegisteredSearchRule(int32_t processId) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = searchRuleSets_.find(processId);
    return iter != searchRuleSets_.end() && !iter->second.empty();
}

std::vector<SearchMatchEvidence> PageSceneRuleManager::MatchSearchScene(
    int32_t processId, const SearchSceneSnapshot& snapshot) const
{
    SearchSnapshotMatcher matcher;
    std::vector<SearchMatchEvidence> hits;
    std::lock_guard<std::mutex> lock(mutex_);
    auto processIter = searchRuleSets_.find(processId);
    if (processIter == searchRuleSets_.end()) {
        return hits;
    }
    for (const auto& [ruleSetId, ruleSet] : processIter->second) {
        auto ruleHits = matcher.Match(snapshot, ruleSet);
        hits.insert(hits.end(), ruleHits.begin(), ruleHits.end());
    }
    return hits;
}

std::vector<SearchPresenceStateMachine::PresenceEvent> PageSceneRuleManager::UpdateSearchPresence(
    int32_t processId, const SearchSceneSnapshot& snapshot, const std::vector<SearchMatchEvidence>& hits)
{
    std::lock_guard<std::mutex> presenceLock(presenceStateMutex_);
    std::vector<SearchPresenceStateMachine::PresenceEvent> events;
    // Derive pageName by pageType instead of the concatenated snapshot.pageName.
    // Navigation → navDestinationName; Router → pageUrl; fallback → pageName.
    std::string pageName = !snapshot.navDestinationName.empty()
        ? snapshot.navDestinationName : (!snapshot.pageUrl.empty() ? snapshot.pageUrl : snapshot.pageName);
    if (pageName.empty()) {
        return events;
    }
    std::map<std::string, std::vector<SearchMatchEvidence>> hitsBySuffix {
        { "", {} }, { SUBWINDOW_DIALOG_SUFFIX, {} }, { SUBWINDOW_SHEET_SUFFIX, {} }
    };
    for (const auto& hit : hits) {
        const auto suffix = hit.isSubwindow
            ? (hit.subwindowSuffix.empty() ? std::string(SUBWINDOW_DIALOG_SUFFIX) : hit.subwindowSuffix) : "";
        hitsBySuffix[suffix].emplace_back(hit);
    }
    // Update every supported state even on a miss or after a window closes.
    // Dialog and Sheet must have independent evidence and exit transitions.
    std::map<std::string, std::vector<SearchMatchEvidence>> hitsByPage;
    for (const auto& [suffix, pageHits] : hitsBySuffix) {
        const auto name = suffix.empty() ? pageName : BuildSearchSubwindowPageName(pageName, suffix);
        auto& combined = hitsByPage[name];
        combined.insert(combined.end(), pageHits.begin(), pageHits.end());
    }
    for (const auto& [eventPageName, pageHits] : hitsByPage) {
        const auto key = std::to_string(processId) + ":" + eventPageName;
        auto event = presenceStateMachine_.Update(key, eventPageName, !pageHits.empty(), pageHits);
        if (event) {
            events.emplace_back(std::move(*event));
        }
    }
    return events;
}

std::optional<SearchPresenceStateMachine::PresenceEvent> PageSceneRuleManager::ExitSearchPresence(
    int32_t processId, const std::string& pageName)
{
    std::lock_guard<std::mutex> presenceLock(presenceStateMutex_);
    if (pageName.empty()) {
        return std::nullopt;
    }
    return presenceStateMachine_.Exit(std::to_string(processId) + ":" + pageName, pageName);
}

std::vector<SearchPresenceStateMachine::PresenceEvent> PageSceneRuleManager::ExitOtherSearchPresence(
    int32_t processId, const std::string& currentPageName)
{
    std::lock_guard<std::mutex> presenceLock(presenceStateMutex_);
    // Do not exit any PRESENT state when there is no current page identity
    // (subwindow UIContent / transition), to avoid subwindow focus detection
    // clearing the main-window presence (AC-5.1).
    if (currentPageName.empty()) {
        return {};
    }
    std::string prefix = std::to_string(processId) + ":";
    return presenceStateMachine_.ExitOtherPages(prefix, currentPageName);
}

std::vector<std::pair<int32_t, SearchPresenceStateMachine::PresenceEvent>> PageSceneRuleManager::ExitAllSearchPresence()
{
    std::vector<std::pair<int32_t, SearchPresenceStateMachine::PresenceEvent>> events;
    std::lock_guard<std::mutex> lock(mutex_);
    std::lock_guard<std::mutex> presenceLock(presenceStateMutex_);
    for (const auto& [pid, ruleSets] : searchRuleSets_) {
        auto pidEvents = presenceStateMachine_.ExitAllExcept(std::to_string(pid) + ":", "");
        for (auto& event : pidEvents) {
            events.emplace_back(pid, std::move(event));
        }
    }
    return events;
}

uint32_t PageSceneRuleManager::RegisterSearchClickObserver(
    const std::weak_ptr<PageSceneRuleManager>& weakManager)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (searchRuleSets_.empty() || weakManager.expired()) {
        return 0;
    }
    if (searchClickObserverId_ != 0) {
        return searchClickObserverId_;
    }
    searchClickObserverId_ = SearchClickObserver::GetInstance().AddHandler(
        [weakManager](const SearchClickCandidate& candidate) {
            auto manager = weakManager.lock();
            if (manager) {
                manager->AddPendingClickCandidate(candidate);
            }
        });
    return searchClickObserverId_;
}

void PageSceneRuleManager::UnregisterSearchClickObserver(uint32_t handlerId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (handlerId != 0) {
        SearchClickObserver::GetInstance().RemoveHandler(handlerId);
    }
    if (handlerId == searchClickObserverId_) {
        searchClickObserverId_ = 0;
    }
}

void PageSceneRuleManager::StopSearchClickObservationIfIdleLocked()
{
    if (!searchRuleSets_.empty()) {
        return;
    }
    pendingClickCandidates_.clear();
    recentClickOperations_.clear();
    if (searchClickObserverId_ != 0) {
        SearchClickObserver::GetInstance().RemoveHandler(searchClickObserverId_);
        searchClickObserverId_ = 0;
    }
}

void PageSceneRuleManager::PrunePendingClickCandidatesLocked(int64_t nowMs)
{
    for (auto iter = pendingClickCandidates_.begin(); iter != pendingClickCandidates_.end();) {
        auto timestamp = iter->second.operationTimestampMs;
        if (timestamp > nowMs || nowMs - timestamp > SEARCH_CLICK_EXPIRATION_MS) {
            iter = pendingClickCandidates_.erase(iter);
        } else {
            ++iter;
        }
    }
    for (auto iter = recentClickOperations_.begin(); iter != recentClickOperations_.end();) {
        if (iter->second > nowMs || nowMs - iter->second > SEARCH_CLICK_EXPIRATION_MS) {
            iter = recentClickOperations_.erase(iter);
        } else {
            ++iter;
        }
    }
}

void PageSceneRuleManager::AddPendingClickCandidate(const SearchClickCandidate& candidate)
{
    if (candidate.operationId.empty() || candidate.nodeId < 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (searchRuleSets_.empty()) {
        return;
    }
    auto nowMs = GetSysTimestamp() / NANOSECONDS_PER_MILLISECOND;
    if (candidate.operationTimestampMs < 0 || candidate.operationTimestampMs > nowMs ||
        nowMs - candidate.operationTimestampMs > SEARCH_CLICK_EXPIRATION_MS) {
        return;
    }
    PrunePendingClickCandidatesLocked(nowMs);
    if (recentClickOperations_.find(candidate.operationId) != recentClickOperations_.end() ||
        pendingClickCandidates_.find(candidate.operationId) != pendingClickCandidates_.end()) {
        return;
    }
    if (pendingClickCandidates_.size() >= MAX_PENDING_SEARCH_CLICKS) {
        auto oldest = std::min_element(pendingClickCandidates_.begin(), pendingClickCandidates_.end(),
            [](const auto& left, const auto& right) {
                return left.second.operationTimestampMs < right.second.operationTimestampMs;
            });
        pendingClickCandidates_.erase(oldest);
    }
    if (recentClickOperations_.size() >= MAX_PENDING_SEARCH_CLICKS) {
        auto oldest = std::min_element(recentClickOperations_.begin(), recentClickOperations_.end(),
            [](const auto& left, const auto& right) { return left.second < right.second; });
        recentClickOperations_.erase(oldest);
    }
    recentClickOperations_[candidate.operationId] = nowMs;
    pendingClickCandidates_[candidate.operationId] = candidate;
}

std::vector<SearchClickMatchResult> PageSceneRuleManager::ConsumeSearchClickCandidates(
    const SearchClickPageContext& page)
{
    std::vector<SearchClickCandidate> candidates;
    std::vector<std::pair<int32_t, SearchRuleSet>> ruleSets;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        PrunePendingClickCandidatesLocked(GetSysTimestamp() / NANOSECONDS_PER_MILLISECOND);
        for (const auto& [operationId, candidate] : pendingClickCandidates_) {
            candidates.emplace_back(candidate);
        }
        pendingClickCandidates_.clear();
        for (const auto& [processId, processRuleSets] : searchRuleSets_) {
            for (const auto& [ruleSetId, ruleSet] : processRuleSets) {
                ruleSets.emplace_back(processId, ruleSet);
            }
        }
    }
    SearchClickMatcher matcher;
    std::vector<SearchClickMatchResult> results;
    for (const auto& [processId, ruleSet] : ruleSets) {
        for (const auto& candidate : candidates) {
            auto responseNode = candidate.responseNode.Upgrade();
            auto ruleResults = matcher.Match(candidate, page, ruleSet, responseNode);
            for (auto& result : ruleResults) {
                result.processId = processId;
                result.operationId = candidate.operationId;
            }
            results.insert(results.end(), ruleResults.begin(), ruleResults.end());
        }
    }
    return results;
}

void PageSceneRuleManager::SetIsContentStableFunction(IsContentStableFunction&& func)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isContentStableFunc_ = std::move(func);
}

std::vector<SearchClickMatchResult> PageSceneRuleManager::FlushPendingSearchClicksIfStable(
    const SearchClickPageContext& page)
{
    // Check the stability gate first (reusing ContentChangeManager's
    // IsScrolling/IsTransitioning/IsSwiperScrolling); if not stable, skip
    // consumption and leave candidates for the ContentChange flush path (AC-7.11).
    IsContentStableFunction isStable;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        isStable = isContentStableFunc_;
    }
    if (isStable && !isStable()) {
        return {};
    }
    auto clickResults = ConsumeSearchClickCandidates(page);
    return clickResults;
}

int32_t PageSceneRuleManager::BeginGetPageScene(
    int32_t processId, const std::string& ruleJsonOrRuleSetId, std::string& ruleJson)
{
    if (ruleJsonOrRuleSetId.empty()) {
        return PAGE_SCENE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (pendingGetProcesses_.find(processId) != pendingGetProcesses_.end()) {
        return PAGE_SCENE_LAST_UNFINISH;
    }
    auto registeredIter = registeredRuleSets_.find(processId);
    if (registeredIter != registeredRuleSets_.end() && registeredIter->second.ruleSetId == ruleJsonOrRuleSetId) {
        ruleJson = registeredIter->second.rawJson;
    } else {
        auto ruleSet = ParseRuleSet(ruleJsonOrRuleSetId);
        if (!ruleSet.has_value()) {
            return PAGE_SCENE_PARAM_INVALID;
        }
        ruleJson = ruleJsonOrRuleSetId;
    }
    pendingGetProcesses_.insert(processId);
    return PAGE_SCENE_NO_ERROR;
}

void PageSceneRuleManager::CompleteGetPageScene(int32_t processId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    pendingGetProcesses_.erase(processId);
}

void PageSceneRuleManager::ClearProcess(int32_t processId, bool clearSearchRules)
{
    std::lock_guard<std::mutex> lock(mutex_);
    registeredRuleSets_.erase(processId);
    if (clearSearchRules) {
        searchRuleSets_.erase(processId);
        std::lock_guard<std::mutex> presenceLock(presenceStateMutex_);
        presenceStateMachine_.ClearPrefix(std::to_string(processId) + ":");
    }
    StopSearchClickObservationIfIdleLocked();
    pendingGetProcesses_.erase(processId);
    for (auto stateIter = reportStates_.begin(); stateIter != reportStates_.end();) {
        if (stateIter->first.find(std::to_string(processId) + ":") == 0) {
            stateIter = reportStates_.erase(stateIter);
        } else {
            ++stateIter;
        }
    }
}

std::vector<std::pair<int32_t, std::string>> PageSceneRuleManager::GetActiveRuleJsons() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::pair<int32_t, std::string>> ruleJsons;
    for (const auto& [pid, ruleSet] : registeredRuleSets_) {
        ruleJsons.emplace_back(pid, ruleSet.rawJson);
    }
    return ruleJsons;
}

std::optional<PageSceneMatchResult> PageSceneRuleManager::MatchPageScene(
    int32_t processId, const std::string& ruleJson, const DumpStartNodeSet& startNodes,
    const std::string& pageName, bool forceReportUnmatched)
{
    auto ruleSet = ParseRuleSet(ruleJson);
    // dumpBeginNode is only a viewport source. AtomicService roots are branch
    // markers used to discover overlays, not PageScene traversal roots.
    const bool hasPageRoot = std::any_of(startNodes.pageStartNodes.begin(), startNodes.pageStartNodes.end(),
        [](const auto& node) { return node != nullptr; });
    const bool hasOverlayRoot = std::any_of(startNodes.overlayNodes.begin(), startNodes.overlayNodes.end(),
        [](const auto& node) { return node != nullptr; });
    const bool hasTraversalRoot = hasPageRoot || hasOverlayRoot;
    if (!ruleSet.has_value() || !ruleSet->arkuiEnabled || !hasTraversalRoot) {
        return std::nullopt;
    }
    std::optional<PageSceneMatchResult> unmatchedResult;
    for (const auto& rule : ruleSet->rules) {
        if (!rule.enabled || rule.sceneType != TEXT_EDITOR_SCENE) {
            continue;
        }
        PageSceneInputCountTracker tracker;
        tracker.Initialize(ruleSet.value(), rule, startNodes);
        const auto& nodes = tracker.GetVisibleInputNodes();
        bool matched = static_cast<int32_t>(nodes.size()) >= rule.threshold;
        PageSceneMatchResult result;
        result.ruleSetId = ruleSet->ruleSetId;
        result.ruleId = rule.ruleId;
        result.sceneType = rule.sceneType;
        result.eventName = BuildPageSceneEventName(rule.sceneType, matched, forceReportUnmatched);
        result.pageName = pageName;
        result.matched = matched;
        result.matchedCount = static_cast<int32_t>(nodes.size());
        result.minReportIntervalMs = rule.minReportIntervalMs;
        result.deduplicate = rule.deduplicate;
        result.nodeIds.reserve(nodes.size());
        for (const auto& node : nodes) {
            result.nodeIds.emplace_back(node.nodeId);
        }
        result.sceneJson = BuildSceneJson(ruleSet.value(), rule, pageName, nodes, matched, result.eventName);
        if (matched || forceReportUnmatched) {
            return result;
        }
        if (!unmatchedResult.has_value()) {
            unmatchedResult = result;
        }
    }
    if (unmatchedResult.has_value()) {
        return unmatchedResult;
    }
    return std::nullopt;
}

bool PageSceneRuleManager::ShouldReport(int32_t processId, const PageSceneMatchResult& result)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto key = std::to_string(processId) + ":" + result.ruleSetId + ":" + result.ruleId + ":" + SOURCE_ARKUI;
    auto iter = reportStates_.find(key);
    if (!result.matched) {
        if (iter == reportStates_.end()) {
            return false;
        }
        reportStates_.erase(iter);
        return true;
    }
    auto now = std::chrono::steady_clock::now();
    if (iter != reportStates_.end()) {
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - iter->second.lastReportTime).count();
        if (result.deduplicate && iter->second.lastReportedNodeIds == result.nodeIds) {
            return false;
        }
        if (elapsed < result.minReportIntervalMs) {
            return false;
        }
    }
    reportStates_[key] = { result.nodeIds, now };
    return true;
}

bool PageSceneRuleManager::IsTextInputNodeType(const std::string& nodeType) const
{
    auto normalizedType = NormalizeNodeType(nodeType);
    return normalizedType == "TextInput" || normalizedType == "TextArea" ||
           normalizedType == "Search" || normalizedType == "RichEditor";
}

std::optional<PageSceneRuleSet> PageSceneRuleManager::ParseRuleSet(const std::string& ruleJson) const
{
    auto root = JsonUtil::ParseJsonString(ruleJson);
    if (!root || !root->IsObject()) {
        LOGW("Parse page scene rule failed, invalid json");
        return std::nullopt;
    }
    PageSceneRuleSet ruleSet;
    ruleSet.rawJson = ruleJson;
    ruleSet.version = root->GetInt("version", 0);
    if (ruleSet.version != PAGE_SCENE_RULE_VERSION) {
        return std::nullopt;
    }
    ruleSet.ruleSetId = root->GetString("ruleSetId");
    if (!IsValidId(ruleSet.ruleSetId)) {
        return std::nullopt;
    }

    auto globalConfig = GetObjectValue(root, "globalConfig");
    if (globalConfig) {
        ruleSet.includeUnfocusableTextInput = globalConfig->GetBool("includeUnfocusableTextInput", false);
    }

    auto sourceConfig = GetObjectValue(root, "sourceConfig");
    if (sourceConfig) {
        ruleSet.arkuiEnabled = sourceConfig->GetBool("arkui", true);
        ruleSet.webEnabled = sourceConfig->GetBool("web", false);
        ruleSet.uiExtensionEnabled = sourceConfig->GetBool("uiExtension", false);
    }

    auto webRules = root->GetValue("webRules");
    if (webRules && !webRules->IsNull()) {
        ruleSet.webRulesRaw = webRules->ToString();
    }

    auto rules = root->GetValue("rules");
    if (!rules || !rules->IsArray() || rules->GetArraySize() <= 0) {
        return std::nullopt;
    }
    for (int32_t index = 0; index < rules->GetArraySize(); ++index) {
        auto ruleJsonValue = rules->GetArrayItem(index);
        auto rule = ParseRule(ruleJsonValue);
        if (rule.has_value()) {
            ruleSet.rules.emplace_back(rule.value());
        }
    }
    if (ruleSet.rules.empty()) {
        return std::nullopt;
    }
    return ruleSet;
}

std::optional<PageSceneRule> PageSceneRuleManager::ParseRule(const std::unique_ptr<JsonValue>& ruleJson) const
{
    if (!ruleJson || !ruleJson->IsObject()) {
        return std::nullopt;
    }
    PageSceneRule rule;
    rule.ruleId = ruleJson->GetString("ruleId");
    rule.sceneType = ruleJson->GetString("sceneType");
    rule.enabled = ruleJson->GetBool("enabled", true);
    if (!IsValidId(rule.ruleId) || rule.sceneType != TEXT_EDITOR_SCENE) {
        return std::nullopt;
    }

    auto scope = GetObjectValue(ruleJson, "scope");
    if (scope) {
        rule.onlyVisible = scope->GetBool("onlyVisible", true);
        rule.rectCulling = scope->GetBool("rectCulling", false);
        rule.includeWeb = scope->GetBool("includeWeb", false);
        rule.includeUIExtension = scope->GetBool("includeUIExtension", false);
    }

    auto selector = GetObjectValue(ruleJson, "selector");
    auto nodeTypes = selector ? selector->GetValue("nodeTypes") : nullptr;
    if (!nodeTypes || !nodeTypes->IsArray() || nodeTypes->GetArraySize() <= 0) {
        return std::nullopt;
    }
    for (int32_t index = 0; index < nodeTypes->GetArraySize(); ++index) {
        auto item = nodeTypes->GetArrayItem(index);
        auto nodeType = item ? NormalizeNodeType(item->GetString()) : "";
        if (IsTextInputNodeType(nodeType)) {
            rule.nodeTypes.insert(nodeType);
        }
    }
    if (rule.nodeTypes.empty()) {
        return std::nullopt;
    }

    auto condition = GetObjectValue(ruleJson, "condition");
    if (!condition || condition->GetString("operator") != COUNT_GTE_OPERATOR) {
        return std::nullopt;
    }
    rule.threshold = condition->GetInt("threshold", 0);
    if (rule.threshold <= 0) {
        return std::nullopt;
    }

    auto report = GetObjectValue(ruleJson, "report");
    if (report) {
        rule.includeNodeIds = report->GetBool("includeNodeIds", true);
        rule.includeNodeTypes = report->GetBool("includeNodeTypes", true);
        rule.includeRect = report->GetBool("includeRect", true);
        rule.includeFocusable = report->GetBool("includeFocusable", true);
        rule.includeText = report->GetBool("includeText", false);
    }

    auto policy = GetObjectValue(ruleJson, "policy");
    if (policy) {
        rule.reportOnRegister = policy->GetBool("reportOnRegister", true);
        rule.deduplicate = policy->GetBool("deduplicate", true);
        rule.minReportIntervalMs = policy->GetInt("minReportIntervalMs", DEFAULT_MIN_REPORT_INTERVAL_MS);
        if (rule.minReportIntervalMs < 0) {
            rule.minReportIntervalMs = DEFAULT_MIN_REPORT_INTERVAL_MS;
        }
    }
    return rule;
}

std::optional<SearchRuleSet> PageSceneRuleManager::ParseSearchRuleSet(const std::string& ruleJson) const
{
    auto root = JsonUtil::ParseJsonString(ruleJson);
    if (!root || !root->IsObject()) {
        LOGW("Parse search rule failed, invalid json");
        return std::nullopt;
    }
    SearchRuleSet ruleSet;
    ruleSet.rawJson = ruleJson;
    ruleSet.registrationGeneration = root->GetString("_pageSceneGeneration");
    ruleSet.ruleSetId = root->GetString("ruleSetId");
    if (!IsValidId(ruleSet.ruleSetId)) {
        return std::nullopt;
    }
    auto rules = root->GetValue("rules");
    if (!rules || !rules->IsArray() || rules->GetArraySize() <= 0 ||
        rules->GetArraySize() > MAX_SEARCH_RULES_PER_RULE_SET) {
        return std::nullopt;
    }
    std::set<std::string> ruleIds;
    for (int32_t index = 0; index < rules->GetArraySize(); ++index) {
        auto rule = ParseSearchRule(rules->GetArrayItem(index), ruleIds);
        if (!rule.has_value()) {
            return std::nullopt;
        }
        ruleSet.rules.emplace_back(rule.value());
    }
    return ruleSet;
}

std::optional<SearchSceneRule> PageSceneRuleManager::ParseSearchRule(
    const std::unique_ptr<JsonValue>& ruleJson, std::set<std::string>& ruleIds) const
{
    if (!ruleJson || !ruleJson->IsObject()) {
        return std::nullopt;
    }
    SearchSceneRule rule;
    rule.ruleId = ruleJson->GetString("ruleId");
    if (rule.ruleId.empty() || !IsValidId(rule.ruleId) || ruleIds.find(rule.ruleId) != ruleIds.end()) {
        return std::nullopt;
    }
    ruleIds.insert(rule.ruleId);

    rule.pageType = ruleJson->GetString("pageType");
    if (rule.pageType != PAGE_TYPE_NAVIGATION && rule.pageType != PAGE_TYPE_ROUTER) {
        return std::nullopt;
    }
    rule.pageName = ruleJson->GetString("pageName");
    if (rule.pageName.empty()) {
        return std::nullopt;
    }

    if (!ParseSearchRegions(ruleJson, rule) || !ParseSearchKeywords(ruleJson, rule) ||
        !ParseSearchComponents(ruleJson, rule) || !ParseSearchInspectorIds(ruleJson, rule)) {
        return std::nullopt;
    }
    return rule;
}

std::string PageSceneRuleManager::BuildSceneJson(const PageSceneRuleSet& ruleSet, const PageSceneRule& rule,
    const std::string& pageName, const std::vector<PageSceneNodeInfo>& nodes, bool matched,
    const std::string& eventName) const
{
    auto root = JsonUtil::Create(true);
    root->Put("ruleSetId", ruleSet.ruleSetId.c_str());
    root->Put("ruleId", rule.ruleId.c_str());
    root->Put("sceneType", rule.sceneType.c_str());
    root->Put("eventName", eventName.c_str());
    root->Put("currentPageName", pageName.c_str());
    auto source = JsonUtil::Create(true);
    source->Put("type", SOURCE_ARKUI);
    source->Put("hostNodeId", -1);
    root->Put("source", source);
    root->Put("matched", matched);
    root->Put("matchedCount", static_cast<int32_t>(nodes.size()));
    auto nodesJson = JsonUtil::CreateArray(true);
    for (const auto& node : nodes) {
        auto nodeJson = JsonUtil::Create(true);
        if (rule.includeNodeIds) {
            nodeJson->Put("nodeId", node.nodeId);
        }
        if (rule.includeNodeTypes) {
            nodeJson->Put("nodeType", node.nodeType.c_str());
        }
        if (rule.includeFocusable) {
            nodeJson->Put("focusable", node.focusable);
        }
        if (rule.includeRect) {
            nodeJson->Put("rect", BuildRectJson(node.rect));
        }
        if (rule.includeText) {
            nodeJson->Put("text", node.text.c_str());
            nodeJson->Put("placeholder", node.placeholder.c_str());
            nodeJson->Put("contentType", node.contentType.c_str());
            nodeJson->Put("inputType", node.inputType.c_str());
        }
        if (node.enableAutoFill.has_value()) {
            nodeJson->Put("enableAutoFill", node.enableAutoFill.value());
        }
        nodesJson->Put(nodeJson);
    }
    root->Put("nodes", nodesJson);
    return root->ToString();
}

} // namespace OHOS::Ace::NG
