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

#include "frameworks/core/components_ng/manager/uisession_query/component_tree_query.h"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <regex>
#include <unordered_set>
#include <vector>

#include "base/log/log.h"
#include "base/utils/utils.h"
#include "base/log/ace_trace.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/inspector.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/navigation/navigation_group_node.h"
#include "core/components_ng/syntax/arkoala_lazy_node.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"
#include "core/common/container_scope.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/base/json/json_util.h"

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t ERR_OK = static_cast<int32_t>(ComponentTreeQueryError::OK);
constexpr int32_t ERR_INVALID_PARAMETER = static_cast<int32_t>(ComponentTreeQueryError::INVALID_PARAMETER);
constexpr int32_t ERR_TARGET_NOT_FOUND = static_cast<int32_t>(ComponentTreeQueryError::TARGET_NOT_FOUND);
constexpr int32_t ERR_UNSUPPORTED_FRONTEND = static_cast<int32_t>(ComponentTreeQueryError::UNSUPPORTED_FRONTEND);
constexpr int32_t ERR_DATA_SOURCE_ERROR = static_cast<int32_t>(ComponentTreeQueryError::DATA_SOURCE_ERROR);
constexpr int32_t ERR_RESOURCE_LIMIT = static_cast<int32_t>(ComponentTreeQueryError::RESOURCE_LIMIT);
constexpr int32_t ERR_INTERNAL_ERROR = static_cast<int32_t>(ComponentTreeQueryError::INTERNAL_ERROR);
constexpr int32_t ERR_TIMEOUT = static_cast<int32_t>(ComponentTreeQueryError::TIMEOUT);

const char QUERY_KEY_CONTENT[] = "content";
const char QUERY_KEY_NAV_BAR_MATCHES[] = "navBarMatches";
const char QUERY_KEY_RESULTS[] = "results";

// Traversal state shared by all preorder walks of this query.
struct ScanState {
    int32_t scanned = 0; // nodes visited so far (resource limit)
    bool limitExceeded = false;
    bool timedOut = false;
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
};

// Accumulated match results during NavBar walk. Bundles the output list,
// running serialized size, and scroll-item dedup set into one struct so
// ProcessMatchedNode and WalkNavBarAndMatch don't pass them individually.
struct MatchResult {
    std::vector<std::string> matchJsonStrs;
    size_t serializedSize = 0;
    std::unordered_set<int32_t> visitedItemIds;
};

// Best point-hit candidate: deepest node wins; equal depth keeps the node
// visited later in preorder (siblings drawn later are on top).
struct HitCandidate {
    RefPtr<FrameNode> node;
    int32_t depth = -1;
};

// Bundles the point-hit query params (x, y — constant) and the best-candidate
// output into one struct so VisitForPointHit doesn't pass them individually.
struct PointHitQuery {
    float x = 0.0f;
    float y = 0.0f;
    HitCandidate best;
};

RectF GetVisibleRectRelativeToWindow(const RefPtr<FrameNode>& node)
{
    if (!node) {
        return RectF();
    }
    // ADR-2: returns invalid rect for inactive nodes and nodes under invisible ancestors.
    return node->GetTransformRectRelativeToWindowOnlyVisible();
}

// Collects direct children plus the overlay branch (same order the Inspector uses).
std::vector<RefPtr<UINode>> GetTraversalChildren(const RefPtr<UINode>& node)
{
    std::vector<RefPtr<UINode>> children;
    if (!node) {
        return children;
    }
    for (const auto& child : node->GetChildren()) {
        children.emplace_back(child);
    }
    auto frameNode = AceType::DynamicCast<FrameNode>(node);
    if (frameNode) {
        auto overlay = frameNode->GetOverlayNode();
        if (overlay) {
            children.emplace_back(overlay);
        }
    }
    return children;
}

bool ConsumeScanQuota(ScanState& state)
{
    ++state.scanned;
    if (state.scanned > COMPONENT_TREE_QUERY_MAX_SCAN_NODES) {
        state.limitExceeded = true;
        return false;
    }
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - state.startTime).count();
    if (elapsedMs >= COMPONENT_TREE_QUERY_MAX_EXECUTION_MS) {
        state.timedOut = true;
        state.limitExceeded = true;
        return false;
    }
    return true;
}

void VisitForPointHit(const RefPtr<UINode>& node, int32_t depth,
    PointHitQuery& query, ScanState& state)
{
    if (!node || !ConsumeScanQuota(state)) {
        return;
    }
    // Skip invisible/inactive nodes entirely.
    if (!node->IsVisibleAndActive()) {
        return;
    }
    auto frameNode = AceType::DynamicCast<FrameNode>(node);
    if (frameNode) {
        auto rect = GetVisibleRectRelativeToWindow(frameNode);
        if (ComponentTreeQuery::IsPointInRectBoundaryInclusive(query.x, query.y, rect) &&
            GreatNotEqual(rect.Width(), 0.0f) && GreatNotEqual(rect.Height(), 0.0f)) {
            // Preorder guarantees later visits at the same depth are drawn on top.
            if (depth >= query.best.depth) {
                query.best.node = frameNode;
                query.best.depth = depth;
            }
        }
    }
    for (const auto& child : GetTraversalChildren(node)) {
        VisitForPointHit(child, depth + 1, query, state);
        if (state.limitExceeded) {
            return;
        }
    }
}

// Walks up from the hit node; returns the first ancestor matching pred together
// with the child on the path directly below it.
RefPtr<UINode> FindAncestorWithDirectChild(const RefPtr<UINode>& hitNode,
    const std::function<bool(const RefPtr<UINode>&)>& pred, RefPtr<UINode>& directChild)
{
    auto current = hitNode;
    while (current) {
        auto parent = current->GetParent();
        if (!parent) {
            break;
        }
        if (pred(parent)) {
            directChild = current;
            return parent;
        }
        current = parent;
    }
    directChild = nullptr;
    return nullptr;
}

std::unique_ptr<JsonValue> ParseJsonOrEmpty(const std::string& jsonStr)
{
    auto parsed = JsonUtil::ParseJsonString(jsonStr);
    if (!parsed || !parsed->IsValid()) {
        return JsonUtil::Create(true);
    }
    return parsed;
}

bool CheckSerializedSize(size_t current)
{
    return current <= COMPONENT_TREE_QUERY_MAX_SERIALIZED_BYTES;
}

bool IsScrollItem(const RefPtr<UINode>& n)
{
    const auto& tag = n->GetTag();
    return tag == "ListItem" || tag == "GridItem" || tag == "FlowItem";
}

RefPtr<NavigationGroupNode> FindNavigationByPoint(float x, float y, ScanState& state, int32_t& errorCode)
{
    auto context = PipelineContext::GetCurrentContextSafely();
    auto root = context ? context->GetRootElement() : nullptr;
    if (!root) {
        errorCode = ERR_INTERNAL_ERROR;
        return nullptr;
    }
    PointHitQuery query;
    query.x = x;
    query.y = y;
    VisitForPointHit(root, 0, query, state);
    if (state.timedOut) {
        errorCode = ERR_TIMEOUT;
        return nullptr;
    }
    if (state.limitExceeded) {
        errorCode = ERR_RESOURCE_LIMIT;
        return nullptr;
    }
    if (!query.best.node) {
        errorCode = ERR_TARGET_NOT_FOUND;
        return nullptr;
    }
    RefPtr<UINode> ignored;
    auto navigationGroup = FindAncestorWithDirectChild(query.best.node,
        [](const RefPtr<UINode>& n) { return AceType::InstanceOf<NavigationGroupNode>(n); }, ignored);
    auto navGroupNode = AceType::DynamicCast<NavigationGroupNode>(navigationGroup);
    if (!navGroupNode) {
        errorCode = ERR_TARGET_NOT_FOUND;
        return nullptr;
    }
    errorCode = ERR_OK;
    return navGroupNode;
}

RefPtr<LazyForEachNode> FindLazyForEachByPoint(float x, float y, ScanState& state, int32_t& errorCode)
{
    auto context = PipelineContext::GetCurrentContextSafely();
    auto root = context ? context->GetRootElement() : nullptr;
    if (!root) {
        errorCode = ERR_INTERNAL_ERROR;
        return nullptr;
    }
    PointHitQuery query;
    query.x = x;
    query.y = y;
    VisitForPointHit(root, 0, query, state);
    if (state.limitExceeded) {
        errorCode = ERR_RESOURCE_LIMIT;
        return nullptr;
    }
    if (!query.best.node) {
        errorCode = ERR_TARGET_NOT_FOUND;
        return nullptr;
    }
    RefPtr<UINode> ignored;
    auto staticLazy = FindAncestorWithDirectChild(query.best.node,
        [](const RefPtr<UINode>& n) { return AceType::InstanceOf<ArkoalaLazyNode>(n); }, ignored);
    if (staticLazy) {
        errorCode = ERR_UNSUPPORTED_FRONTEND;
        return nullptr;
    }
    auto lazyForEach = FindAncestorWithDirectChild(query.best.node,
        [](const RefPtr<UINode>& n) { return AceType::InstanceOf<LazyForEachNode>(n); }, ignored);
    if (!lazyForEach) {
        errorCode = ERR_TARGET_NOT_FOUND;
        return nullptr;
    }
    auto node = AceType::DynamicCast<LazyForEachNode>(lazyForEach);
    if (!node) {
        errorCode = ERR_INTERNAL_ERROR;
        return nullptr;
    }
    errorCode = ERR_OK;
    return node;
}

void ProcessMatchedNode(const RefPtr<UINode>& node, const std::string& nodeJsonStr,
    ScanState& state, MatchResult& result)
{
    RefPtr<UINode> scrollItem = IsScrollItem(node) ? node : nullptr;
    if (!scrollItem) {
        RefPtr<UINode> ignored;
        scrollItem = FindAncestorWithDirectChild(node, IsScrollItem, ignored);
    }
    if (scrollItem && result.visitedItemIds.find(scrollItem->GetId()) == result.visitedItemIds.end()) {
        result.visitedItemIds.emplace(scrollItem->GetId());
        auto itemJsonStr = Inspector::GetInspectorOfNodeSubtree(scrollItem);
        if (!itemJsonStr.empty() && CheckSerializedSize(result.serializedSize + itemJsonStr.size())) {
            result.matchJsonStrs.emplace_back(itemJsonStr);
            result.serializedSize += itemJsonStr.size();
        } else {
            state.limitExceeded = true;
        }
    } else if (!scrollItem) {
        result.matchJsonStrs.emplace_back(nodeJsonStr);
        result.serializedSize += nodeJsonStr.size();
    }
}

void WalkNavBarAndMatch(const RefPtr<UINode>& navBarNode, const std::regex& regex,
    ScanState& state, MatchResult& result)
{
    std::unordered_set<int32_t> visitedIds;
    std::function<void(const RefPtr<UINode>&)> walk = [&](const RefPtr<UINode>& node) {
        if (!node || !ConsumeScanQuota(state)) {
            return;
        }
        if (visitedIds.find(node->GetId()) == visitedIds.end()) {
            visitedIds.emplace(node->GetId());
            auto jsonValue = JsonUtil::CreateSharedPtrJson(true);
            node->DumpSimplifyTreeNode(jsonValue, ParamConfig());
            auto nodeJsonStr = jsonValue->ToString();
            if (nodeJsonStr.empty() || !CheckSerializedSize(result.serializedSize + nodeJsonStr.size())) {
                state.limitExceeded = true;
                return;
            }
            if (std::regex_search(nodeJsonStr, regex)) {
                ProcessMatchedNode(node, nodeJsonStr, state, result);
            }
        }
        if (!node->IsVisibleAndActive()) {
            return;
        }
        for (const auto& child : GetTraversalChildren(node)) {
            walk(child);
            if (state.limitExceeded) {
                return;
            }
        }
    };
    walk(navBarNode);
}

std::string BuildQueryErrorJson(int32_t code)
{
    auto json = JsonUtil::CreateSharedPtrJson(true);
    json->Put("errorCode", code);
    auto ctx = PipelineContext::GetCurrentContextSafely();
    if (ctx) {
        ctx->GetAppInfo(json);
    }
    return json->ToString();
}

int32_t ExtractAndBuildLazyForEachData(const RefPtr<LazyForEachBuilder>& builder,
    int32_t instanceId, const ScanState& state, std::string& resultJson)
{
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - state.startTime).count();
    if (elapsedMs >= COMPONENT_TREE_QUERY_MAX_EXECUTION_MS) {
        resultJson = BuildQueryErrorJson(ERR_TIMEOUT);
        return ERR_TIMEOUT;
    }
    ContainerScope containerScope(instanceId);
    auto results = JsonUtil::CreateArray(true);
    int32_t extractedCount = 0;
    int32_t totalCount = builder->GetTotalCount();
    auto allChildren = builder->GetAllChildren();
    for (const auto& item : allChildren) {
        if (item.first < 0 || item.first >= totalCount) {
            continue;
        }
        int32_t dataError = ERR_OK;
        auto dataJson = builder->GetDataByIndexAsJson(item.first, dataError);
        if (dataError != ERR_OK || dataJson.empty()) {
            continue;
        }
        auto entry = JsonUtil::CreateSharedPtrJson(true);
        entry->Put("index", item.first);
        entry->PutRef("data", ParseJsonOrEmpty(dataJson));
        results->PutRef(std::move(entry));
        extractedCount++;
    }
    if (extractedCount == 0) {
        resultJson = BuildQueryErrorJson(ERR_DATA_SOURCE_ERROR);
        return ERR_DATA_SOURCE_ERROR;
    }
    auto json = JsonUtil::CreateSharedPtrJson(true);
    json->Put("errorCode", ERR_OK);
    auto context = PipelineContext::GetCurrentContextSafely();
    if (context) {
        context->GetAppInfo(json);
    }
    json->PutRef(QUERY_KEY_RESULTS, std::move(results));
    resultJson = json->ToString();
    if (!CheckSerializedSize(resultJson.size())) {
        resultJson = BuildQueryErrorJson(ERR_RESOURCE_LIMIT);
        return ERR_RESOURCE_LIMIT;
    }
    return ERR_OK;
}
} // namespace

// static
bool ComponentTreeQuery::IsFinitePoint(float x, float y)
{
    return std::isfinite(x) && std::isfinite(y);
}

// static
bool ComponentTreeQuery::IsPointInRectBoundaryInclusive(float x, float y, const RectF& rect)
{
    return GreatOrEqual(x, rect.Left()) && LessOrEqual(x, rect.Right()) && GreatOrEqual(y, rect.Top()) &&
           LessOrEqual(y, rect.Bottom());
}

// static
int32_t ComponentTreeQuery::CheckRegex(const std::string& pattern)
{
    if (pattern.empty()) {
        return ERR_INVALID_PARAMETER;
    }
    if (pattern.size() > COMPONENT_TREE_QUERY_MAX_REGEX_BYTES) {
        return ERR_RESOURCE_LIMIT;
    }
    // Caller is responsible for pattern validity (std::regex aborts under -fno-exceptions).
    return ERR_OK;
}

// static
int32_t ComponentTreeQuery::QueryNavigationContentByPoint(
    float x, float y, const std::string& pattern, std::string& resultJson)
{
    ACE_SCOPED_TRACE("QueryNavigationContentByPoint[x:%f][y:%f][patternLen:%zu]", x, y, pattern.size());

    auto buildResult = [](int32_t code) {
        auto json = JsonUtil::CreateSharedPtrJson(true);
        json->Put("errorCode", code);
        auto ctx = PipelineContext::GetCurrentContextSafely();
        if (ctx) {
            ctx->GetAppInfo(json);
        }
        return json->ToString();
    };

    // Convert physical pixel coordinates (from SA) to vp used by component tree rects.
    auto pipeline = PipelineContext::GetCurrentContextSafely();
    if (!pipeline) {
        resultJson = buildResult(ERR_INTERNAL_ERROR);
        return ERR_INTERNAL_ERROR;
    }
    float viewScale = pipeline->GetViewScale();
    if (viewScale > 0.0f) {
        x /= viewScale;
        y /= viewScale;
    }

    auto regexCheck = CheckRegex(pattern);
    if (regexCheck != ERR_OK) {
        resultJson = buildResult(regexCheck);
        return regexCheck;
    }
    if (!IsFinitePoint(x, y)) {
        resultJson = buildResult(ERR_INVALID_PARAMETER);
        return ERR_INVALID_PARAMETER;
    }

    ScanState state;
    int32_t errorCode = ERR_OK;
    auto navGroupNode = FindNavigationByPoint(x, y, state, errorCode);
    if (!navGroupNode) {
        if (errorCode != ERR_TARGET_NOT_FOUND) {
            resultJson = buildResult(errorCode);
            return errorCode;
        }
        // Navigation not found.
        resultJson = buildResult(ERR_OK);
        return ERR_OK;
    }

    auto contentNode = navGroupNode->GetContentNode();
    if (!contentNode) {
        resultJson = buildResult(ERR_OK);
        return ERR_OK;
    }
    // Check remaining time budget before subtree serialization.
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - state.startTime).count();
    if (elapsedMs >= COMPONENT_TREE_QUERY_MAX_EXECUTION_MS) {
        resultJson = buildResult(ERR_TIMEOUT);
        return ERR_TIMEOUT;
    }
    auto contentJsonStr = Inspector::GetInspectorOfNodeSubtree(contentNode);
    if (!CheckSerializedSize(contentJsonStr.size())) {
        resultJson = buildResult(ERR_RESOURCE_LIMIT);
        return ERR_RESOURCE_LIMIT;
    }

    std::regex regex(pattern);
    MatchResult matchResult;
    matchResult.serializedSize = contentJsonStr.size();
    auto navBarNode = navGroupNode->GetNavBarNode();
    if (navBarNode) {
        WalkNavBarAndMatch(navBarNode, regex, state, matchResult);
    }

    if (state.timedOut) {
        resultJson = buildResult(ERR_TIMEOUT);
        return ERR_TIMEOUT;
    }
    if (state.limitExceeded) {
        resultJson = buildResult(ERR_RESOURCE_LIMIT);
        return ERR_RESOURCE_LIMIT;
    }

    auto json = JsonUtil::CreateSharedPtrJson(true);
    json->Put("errorCode", ERR_OK);
    auto context = PipelineContext::GetCurrentContextSafely();
    if (context) {
        context->GetAppInfo(json);
    }
    json->PutRef(QUERY_KEY_CONTENT, ParseJsonOrEmpty(contentJsonStr));
    auto matches = JsonUtil::CreateArray(true);
    for (const auto& matchStr : matchResult.matchJsonStrs) {
        matches->PutRef(ParseJsonOrEmpty(matchStr));
    }
    json->PutRef(QUERY_KEY_NAV_BAR_MATCHES, std::move(matches));
    resultJson = json->ToString();
    if (!CheckSerializedSize(resultJson.size())) {
        resultJson = buildResult(ERR_RESOURCE_LIMIT);
        return ERR_RESOURCE_LIMIT;
    }
    return ERR_OK;
}

// static
int32_t ComponentTreeQuery::QueryLazyForEachDataByPoint(float x, float y, std::string& resultJson)
{
    ACE_SCOPED_TRACE("QueryLazyForEachDataByPoint[x:%f][y:%f]", x, y);

    auto pipeline = PipelineContext::GetCurrentContextSafely();
    if (!pipeline) {
        resultJson = BuildQueryErrorJson(ERR_INTERNAL_ERROR);
        return ERR_INTERNAL_ERROR;
    }
    float viewScale = pipeline->GetViewScale();
    if (viewScale > 0.0f) {
        x /= viewScale;
        y /= viewScale;
    }
    if (!IsFinitePoint(x, y)) {
        resultJson = BuildQueryErrorJson(ERR_INVALID_PARAMETER);
        return ERR_INVALID_PARAMETER;
    }

    ScanState state;
    int32_t errorCode = ERR_OK;
    auto lazyForEachNode = FindLazyForEachByPoint(x, y, state, errorCode);
    if (!lazyForEachNode) {
        resultJson = BuildQueryErrorJson(errorCode);
        return errorCode;
    }

    auto builder = lazyForEachNode->GetBuilder();
    if (!builder) {
        resultJson = BuildQueryErrorJson(ERR_OK);
        return ERR_OK;
    }
    if (!builder->IsDataExtractionSupported()) {
        resultJson = BuildQueryErrorJson(ERR_UNSUPPORTED_FRONTEND);
        return ERR_UNSUPPORTED_FRONTEND;
    }
    if (builder->GetAllChildren().empty()) {
        resultJson = BuildQueryErrorJson(ERR_OK);
        return ERR_OK;
    }
    return ExtractAndBuildLazyForEachData(builder, lazyForEachNode->GetInstanceId(), state, resultJson);
}

// static
int32_t ComponentTreeQuery::Execute(const ComponentTreeQueryRequest& request, std::string& resultJson)
{
    switch (request.type) {
        case ComponentTreeQueryType::NAVIGATION_CONTENT_BY_POINT:
            return QueryNavigationContentByPoint(request.x, request.y, request.pattern, resultJson);
        case ComponentTreeQueryType::LAZY_FOREACH_DATA_BY_POINT:
            return QueryLazyForEachDataByPoint(request.x, request.y, resultJson);
        default:
            return ERR_INVALID_PARAMETER;
    }
}
} // namespace OHOS::Ace::NG
