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

#include <cmath>
#include <string>

#include "gtest/gtest.h"

#include "base/geometry/ng/rect_t.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/manager/uisession_query/component_tree_query.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/render/render_context.h"
#include "core/components_ng/syntax/lazy_for_each_builder.h"
#include "core/components_ng/syntax/lazy_for_each_node.h"
#include "interfaces/inner_api/ui_session/component_tree_query_type.h"
#include "test/mock/frameworks/core/pipeline/mock_pipeline_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
class MockUnsupportedFrontendBuilder : public LazyForEachBuilder {
    DECLARE_ACE_TYPE(MockUnsupportedFrontendBuilder, LazyForEachBuilder);

public:
    void ReleaseChildGroupById(const std::string& id) override {}
    void RegisterDataChangeListener(const RefPtr<V2::DataChangeListener>& listener) override {}
    void UnregisterDataChangeListener(V2::DataChangeListener* listener) override {}

protected:
    int32_t OnGetTotalCount() override
    {
        return 0;
    }
    LazyForEachChild OnGetChildByIndex(int32_t index,
        std::unordered_map<std::string, LazyForEachCacheChild>& cachedItems) override
    {
        return {};
    }
    LazyForEachChild OnGetChildByIndexNew(int32_t index,
        std::map<int32_t, LazyForEachChild>& cachedItems,
        std::unordered_map<std::string, LazyForEachCacheChild>& expiringItems) override
    {
        return {};
    }
    void OnExpandChildrenOnInitialInNG() override {}
    void NotifyDataChanged(size_t index, const RefPtr<UINode>& lazyForEachNode, bool isRebuild = true) override {}
    void NotifyDataDeleted(const RefPtr<UINode>& lazyForEachNode, size_t index, bool removeIds) override {}
    void NotifyDataAdded(size_t index) override {}
    void KeepRemovedItemInCache(LazyForEachChild node,
        std::unordered_map<std::string, LazyForEachCacheChild>& cachedItems) override {}
};

class MockSupportedFrontendBuilder : public MockUnsupportedFrontendBuilder {
    DECLARE_ACE_TYPE(MockSupportedFrontendBuilder, MockUnsupportedFrontendBuilder);

public:
    bool IsDataExtractionSupported() const override
    {
        return true;
    }
    std::string GetDataByIndexAsJson(int32_t index, int32_t& errorCode) override
    {
        errorCode = static_cast<int32_t>(ComponentTreeQueryError::OK);
        return "{\"name\":\"item-" + std::to_string(index) + "\"}";
    }
};

// Builder whose GetDataByIndexAsJson returns a caller-configured error code + data string.
// Supports per-index results via SetItemResult; indexes not configured fall back to SetResult.
class MockDataErrorBuilder : public MockUnsupportedFrontendBuilder {
    DECLARE_ACE_TYPE(MockDataErrorBuilder, MockUnsupportedFrontendBuilder);

public:
    bool IsDataExtractionSupported() const override
    {
        return true;
    }
    std::string GetDataByIndexAsJson(int32_t index, int32_t& errorCode) override
    {
        auto it = itemResults_.find(index);
        if (it != itemResults_.end()) {
            errorCode = it->second.first;
            return it->second.second;
        }
        errorCode = errorCode_;
        return dataJson_;
    }
    void SetResult(int32_t code, const std::string& data)
    {
        errorCode_ = code;
        dataJson_ = data;
    }
    void SetItemResult(int32_t index, int32_t code, const std::string& data)
    {
        itemResults_[index] = { code, data };
    }
    void SetTotalCount(int32_t count)
    {
        totalCount_ = count;
    }

protected:
    int32_t OnGetTotalCount() override
    {
        return totalCount_;
    }

private:
    int32_t errorCode_ = static_cast<int32_t>(ComponentTreeQueryError::OK);
    std::string dataJson_;
    int32_t totalCount_ = 1;
    std::map<int32_t, std::pair<int32_t, std::string>> itemResults_;
};

// FrameNode subclass that reports visible+active (the shared FrameNode mock returns false,
// which would short-circuit VisitForPointHit at the root and never accumulate the scan quota)
// and exposes a caller-controlled children list so the traversal actually recurses.
// rootNode_ on PipelineContext is RefPtr<FrameNode>, so the mock root must be a FrameNode.
class MockVisibleFrameNode : public FrameNode {
    DECLARE_ACE_TYPE(MockVisibleFrameNode, FrameNode);

public:
    explicit MockVisibleFrameNode(int32_t id)
        : FrameNode("mockVisible", id, AceType::MakeRefPtr<Pattern>(), false) {}
    bool IsVisibleAndActive() const override
    {
        return true;
    }
    const std::list<RefPtr<UINode>>& GetChildren(bool = false) const override
    {
        return children_;
    }
    void SetChildren(std::list<RefPtr<UINode>> children)
    {
        children_ = std::move(children);
    }

private:
    std::list<RefPtr<UINode>> children_;
};

// RenderContext that returns a caller-controlled paint rect so that
// GetTransformRectRelativeToWindowOnlyVisible yields a non-empty rect and
// VisitForPointHit's point-in-rect check can succeed.
class MockRenderContext : public RenderContext {
    DECLARE_ACE_TYPE(MockRenderContext, RenderContext);

public:
    RectF GetPaintRectWithTransform() override
    {
        return paintRect_;
    }
    void SetPaintRect(const RectF& rect)
    {
        paintRect_ = rect;
    }

private:
    RectF paintRect_;
};

// LazyForEachNode subclass that overrides GetChildren to bypass the complex
// LoadChildren / TryTriggleAdditionalLayout machinery in the base class.
class MockLazyForEachNode : public LazyForEachNode {
    DECLARE_ACE_TYPE(MockLazyForEachNode, LazyForEachNode);

public:
    MockLazyForEachNode(int32_t nodeId, const RefPtr<LazyForEachBuilder>& builder)
        : LazyForEachNode(nodeId, builder) {}
    const std::list<RefPtr<UINode>>& GetChildren(bool = false) const override
    {
        return children_;
    }
    void SetChildren(std::list<RefPtr<UINode>> children)
    {
        children_ = std::move(children);
    }

private:
    std::list<RefPtr<UINode>> children_;
};

// Helper: builds a mock tree where a point-hit on hitChild traverses up to a
// LazyForEachNode with the given builder, then calls Execute.
struct LazyForEachMockTree {
    RefPtr<MockVisibleFrameNode> root;
    RefPtr<MockLazyForEachNode> lazyForEach;
    RefPtr<MockVisibleFrameNode> hitChild;
    RefPtr<MockRenderContext> hitChildRenderContext;

    void Install(int32_t rootId, int32_t lazyId, int32_t hitId,
        const RefPtr<LazyForEachBuilder>& builder, float hitX, float hitY)
    {
        root = AceType::MakeRefPtr<MockVisibleFrameNode>(rootId);
        lazyForEach = AceType::MakeRefPtr<MockLazyForEachNode>(lazyId, builder);
        hitChild = AceType::MakeRefPtr<MockVisibleFrameNode>(hitId);
        hitChildRenderContext = AceType::MakeRefPtr<MockRenderContext>();
        // 200×200 rect centred on (100,100); hit at (hitX, hitY) inside it.
        hitChildRenderContext->SetPaintRect(RectF(0.0f, 0.0f, 200.0f, 200.0f));
        hitChild->renderContext_ = hitChildRenderContext;
        hitChild->SetActive(true);
        hitChild->SetParent(AceType::WeakClaim(AceType::RawPtr(lazyForEach)));

        lazyForEach->SetChildren({ hitChild });
        lazyForEach->SetParent(AceType::WeakClaim(AceType::RawPtr(root)));

        root->SetChildren({ lazyForEach });

        auto ctx = PipelineContext::GetCurrentContextSafely();
        ASSERT_NE(ctx, nullptr);
        originalRoot = ctx->rootNode_;
        ctx->rootNode_ = root;
    }

    void Restore()
    {
        auto ctx = PipelineContext::GetCurrentContextSafely();
        if (ctx) {
            ctx->rootNode_ = originalRoot;
        }
    }

private:
    RefPtr<FrameNode> originalRoot;
};
} // namespace

class ComponentTreeQueryTest : public testing::Test {
public:
    static void SetUpTestSuite() { MockPipelineContext::SetUp(); }
    static void TearDownTestSuite() { MockPipelineContext::TearDown(); }
};

/**
 * @tc.name: ComponentTreeQueryPointHit
 * @tc.desc: point-in-rect with boundary inclusive + finite check.
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryPointHit, TestSize.Level1)
{
    RectF rect(0.0f, 0.0f, 10.0f, 10.0f);
    EXPECT_TRUE(ComponentTreeQuery::IsPointInRectBoundaryInclusive(5.0f, 5.0f, rect));
    // Boundary points count as hit.
    EXPECT_TRUE(ComponentTreeQuery::IsPointInRectBoundaryInclusive(0.0f, 0.0f, rect));
    EXPECT_TRUE(ComponentTreeQuery::IsPointInRectBoundaryInclusive(10.0f, 10.0f, rect));
    EXPECT_FALSE(ComponentTreeQuery::IsPointInRectBoundaryInclusive(10.1f, 5.0f, rect));
    EXPECT_FALSE(ComponentTreeQuery::IsPointInRectBoundaryInclusive(-0.1f, 5.0f, rect));

    EXPECT_TRUE(ComponentTreeQuery::IsFinitePoint(1.0f, 2.0f));
    EXPECT_FALSE(ComponentTreeQuery::IsFinitePoint(std::nanf(""), 2.0f));
    EXPECT_FALSE(ComponentTreeQuery::IsFinitePoint(1.0f, HUGE_VALF));
}

/**
 * @tc.name: ComponentTreeQueryInvalidParams001
 * @tc.desc: invalid Navigation query parameters return correct error codes.
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryInvalidParams001, TestSize.Level1)
{
    ComponentTreeQueryRequest request;
    std::string resultJson;

    request.type = ComponentTreeQueryType::NAVIGATION_CONTENT_BY_POINT;
    request.x = 10.0f;
    request.y = 10.0f;
    request.pattern = "";
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::INVALID_PARAMETER));
    EXPECT_FALSE(resultJson.empty());

    request.pattern = "[a-z]+";
    request.x = std::nanf("");
    request.y = 100.0f;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::INVALID_PARAMETER));
    EXPECT_FALSE(resultJson.empty());

    request.x = 10.0f;
    request.y = 10.0f;
    request.pattern = std::string(COMPONENT_TREE_QUERY_MAX_REGEX_BYTES + 1, 'a');
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::RESOURCE_LIMIT));
    EXPECT_FALSE(resultJson.empty());
}

/**
 * @tc.name: ComponentTreeQueryInvalidParams002
 * @tc.desc: invalid LazyForEach query parameters return correct error codes.
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryInvalidParams002, TestSize.Level1)
{
    ComponentTreeQueryRequest request;
    std::string resultJson;

    request.type = ComponentTreeQueryType::LAZY_FOREACH_DATA_BY_POINT;
    request.x = std::nanf("");
    request.y = 100.0f;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::INVALID_PARAMETER));
    EXPECT_FALSE(resultJson.empty());

    request.x = 50.0f;
    request.y = HUGE_VALF;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::INVALID_PARAMETER));
    EXPECT_FALSE(resultJson.empty());
}

/**
 * @tc.name: ComponentTreeQueryRectIntersection
 * @tc.desc: rect/rect positive-area intersection + normalization.
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryRectIntersection, TestSize.Level1)
{
    RectF query(0.0f, 0.0f, 10.0f, 10.0f);
    EXPECT_TRUE(ComponentTreeQuery::IsPositiveAreaRectIntersection(query, RectF(5.0f, 5.0f, 10.0f, 10.0f)));
    EXPECT_TRUE(ComponentTreeQuery::IsPositiveAreaRectIntersection(query, RectF(1.0f, 1.0f, 2.0f, 2.0f)));
    // Touching edges only: zero-area intersection, not a hit.
    EXPECT_FALSE(ComponentTreeQuery::IsPositiveAreaRectIntersection(query, RectF(10.0f, 0.0f, 5.0f, 5.0f)));
    EXPECT_FALSE(ComponentTreeQuery::IsPositiveAreaRectIntersection(query, RectF(20.0f, 20.0f, 5.0f, 5.0f)));
    EXPECT_FALSE(ComponentTreeQuery::IsPositiveAreaRectIntersection(query, RectF(10.0f, 10.0f, 0.0f, 0.0f)));

    // Diagonal corner normalization: any two opposite corners yield the same rect.
    auto normalized = ComponentTreeQuery::NormalizeRect(10.0f, 10.0f, 0.0f, 4.0f);
    EXPECT_EQ(normalized.Left(), 0.0f);
    EXPECT_EQ(normalized.Top(), 4.0f);
    EXPECT_EQ(normalized.Right(), 10.0f);
    EXPECT_EQ(normalized.Bottom(), 10.0f);
    auto normalizedSame = ComponentTreeQuery::NormalizeRect(0.0f, 4.0f, 10.0f, 10.0f);
    EXPECT_EQ(normalized.Left(), normalizedSame.Left());
    EXPECT_EQ(normalized.Top(), normalizedSame.Top());
    EXPECT_EQ(normalized.Right(), normalizedSame.Right());
    EXPECT_EQ(normalized.Bottom(), normalizedSame.Bottom());
}

/**
 * @tc.name: ComponentTreeQueryInvalidParams003
 * @tc.desc: invalid NodesInRect query parameters return correct error codes.
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryInvalidParams003, TestSize.Level1)
{
    ComponentTreeQueryRequest request;
    std::string resultJson;

    request.type = ComponentTreeQueryType::NODES_IN_RECT;
    request.x = std::nanf("");
    request.y = 0.0f;
    request.extraX = 10.0f;
    request.extraY = 10.0f;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::INVALID_PARAMETER));
    EXPECT_FALSE(resultJson.empty());

    request.x = 0.0f;
    request.y = 0.0f;
    request.extraX = std::nanf("");
    request.extraY = 10.0f;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::INVALID_PARAMETER));
    EXPECT_FALSE(resultJson.empty());

    // Zero-area rect after normalization: x1==x2 yields zero width.
    request.x = 0.0f;
    request.y = 0.0f;
    request.extraX = 0.0f;
    request.extraY = 10.0f;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::INVALID_PARAMETER));
    EXPECT_FALSE(resultJson.empty());

    // Zero-area rect after normalization: y1==y2 yields zero height.
    request.x = 0.0f;
    request.y = 5.0f;
    request.extraX = 10.0f;
    request.extraY = 5.0f;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::INVALID_PARAMETER));
    EXPECT_FALSE(resultJson.empty());

    // Infinity coords after normalization (not finite).
    request.x = 0.0f;
    request.y = 0.0f;
    request.extraX = HUGE_VALF;
    request.extraY = 10.0f;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::INVALID_PARAMETER));
    EXPECT_FALSE(resultJson.empty());
}

/**
 * @tc.name: ComponentTreeQueryFrontend
 * @tc.desc: builder frontend support contract. Base (static
 * ArkTS/CJ/C-API) builders report unsupported, dynamic ArkTS reports supported.
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryFrontend, TestSize.Level1)
{
    auto unsupported = AceType::MakeRefPtr<MockUnsupportedFrontendBuilder>();
    ASSERT_TRUE(unsupported);
    EXPECT_FALSE(unsupported->IsDataExtractionSupported());
    int32_t errorCode = static_cast<int32_t>(ComponentTreeQueryError::OK);
    auto data = unsupported->GetDataByIndexAsJson(0, errorCode);
    EXPECT_TRUE(data.empty());
    EXPECT_EQ(errorCode, static_cast<int32_t>(ComponentTreeQueryError::OK));

    auto supported = AceType::MakeRefPtr<MockSupportedFrontendBuilder>();
    ASSERT_TRUE(supported);
    EXPECT_TRUE(supported->IsDataExtractionSupported());
    data = supported->GetDataByIndexAsJson(3, errorCode);
    EXPECT_EQ(errorCode, static_cast<int32_t>(ComponentTreeQueryError::OK));
    EXPECT_EQ(data, "{\"name\":\"item-3\"}");
}

/**
 * @tc.name: ComponentTreeQueryExecuteDefault
 * @tc.desc: Execute with an unknown query type falls through to default and returns INVALID_PARAMETER.
 * @tc.type: FUNC
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryExecuteDefault, TestSize.Level1)
{
    ComponentTreeQueryRequest request;
    request.type = static_cast<ComponentTreeQueryType>(999);
    request.x = 10.0f;
    request.y = 10.0f;
    std::string resultJson;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::INVALID_PARAMETER));
    EXPECT_TRUE(resultJson.empty());
}

/**
 * @tc.name: ComponentTreeQueryNavigationNoTarget
 * @tc.desc: Navigation query traverses the mock root tree; with no NavigationGroupNode present,
 *           returns TARGET_NOT_FOUND with errorCode in resultJson.
 * @tc.type: FUNC
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryNavigationNoTarget, TestSize.Level1)
{
    ComponentTreeQueryRequest request;
    request.type = ComponentTreeQueryType::NAVIGATION_CONTENT_BY_POINT;
    request.x = 10.0f;
    request.y = 10.0f;
    request.pattern = "[a-z]+";
    std::string resultJson;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::TARGET_NOT_FOUND));
    EXPECT_FALSE(resultJson.empty());
    EXPECT_NE(resultJson.find("errorCode"), std::string::npos);
}

/**
 * @tc.name: ComponentTreeQueryLazyForEachNoTarget
 * @tc.desc: LazyForEach query traverses the mock root tree; with no LazyForEachNode present,
 *           the traversal layer surfaces TARGET_NOT_FOUND via BuildQueryErrorJson.
 * @tc.type: FUNC
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryLazyForEachNoTarget, TestSize.Level1)
{
    ComponentTreeQueryRequest request;
    request.type = ComponentTreeQueryType::LAZY_FOREACH_DATA_BY_POINT;
    request.x = 10.0f;
    request.y = 10.0f;
    std::string resultJson;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::TARGET_NOT_FOUND));
    EXPECT_FALSE(resultJson.empty());
    EXPECT_NE(resultJson.find("errorCode"), std::string::npos);
}

/**
 * @tc.name: ComponentTreeQueryNavigationScanLimit
 * @tc.desc: Navigation query whose traversal exceeds MAX_SCAN_NODES returns RESOURCE_LIMIT.
 *           Covers ConsumeScanQuota's scan-budget branch and FindNavigationByPoint's
 *           limitExceeded path (unreachable with the default FrameNode mock because its
 *           IsVisibleAndActive is false and VisitForPointHit never recurses).
 * @tc.type: FUNC
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryNavigationScanLimit, TestSize.Level1)
{
    auto ctx = PipelineContext::GetCurrentContextSafely();
    ASSERT_NE(ctx, nullptr);
    auto originalRoot = ctx->rootNode_;

    auto mockRoot = AceType::MakeRefPtr<MockVisibleFrameNode>(900001);
    auto dummy = AceType::MakeRefPtr<MockVisibleFrameNode>(900002);
    std::list<RefPtr<UINode>> children;
    constexpr int32_t overScan = COMPONENT_TREE_QUERY_MAX_SCAN_NODES + 2;
    for (int32_t i = 0; i < overScan; ++i) {
        children.emplace_back(dummy);
    }
    mockRoot->SetChildren(std::move(children));
    ctx->rootNode_ = mockRoot;

    ComponentTreeQueryRequest request;
    request.type = ComponentTreeQueryType::NAVIGATION_CONTENT_BY_POINT;
    request.x = 0.0f;
    request.y = 0.0f;
    request.pattern = "[a-z]+";
    std::string resultJson;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::RESOURCE_LIMIT));
    EXPECT_FALSE(resultJson.empty());
    EXPECT_NE(resultJson.find("errorCode"), std::string::npos);

    ctx->rootNode_ = originalRoot;
}

/**
 * @tc.name: ComponentTreeQueryLazyForEachScanLimit
 * @tc.desc: LazyForEach query whose traversal exceeds MAX_SCAN_NODES returns RESOURCE_LIMIT
 *           via FindLazyForEachByPoint's limitExceeded path and BuildQueryErrorJson.
 * @tc.type: FUNC
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryLazyForEachScanLimit, TestSize.Level1)
{
    auto ctx = PipelineContext::GetCurrentContextSafely();
    ASSERT_NE(ctx, nullptr);
    auto originalRoot = ctx->rootNode_;

    auto mockRoot = AceType::MakeRefPtr<MockVisibleFrameNode>(900011);
    auto dummy = AceType::MakeRefPtr<MockVisibleFrameNode>(900012);
    std::list<RefPtr<UINode>> children;
    constexpr int32_t overScan = COMPONENT_TREE_QUERY_MAX_SCAN_NODES + 2;
    for (int32_t i = 0; i < overScan; ++i) {
        children.emplace_back(dummy);
    }
    mockRoot->SetChildren(std::move(children));
    ctx->rootNode_ = mockRoot;

    ComponentTreeQueryRequest request;
    request.type = ComponentTreeQueryType::LAZY_FOREACH_DATA_BY_POINT;
    request.x = 0.0f;
    request.y = 0.0f;
    std::string resultJson;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::RESOURCE_LIMIT));
    EXPECT_FALSE(resultJson.empty());
    EXPECT_NE(resultJson.find("errorCode"), std::string::npos);

    ctx->rootNode_ = originalRoot;
}

/**
 * @tc.name: ComponentTreeQueryLazyForEachDataSuccess
 * @tc.desc: getData returns OK + valid JSON → OK + results array contains data.
 * @tc.type: FUNC
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryLazyForEachDataSuccess, TestSize.Level1)
{
    auto builder = AceType::MakeRefPtr<MockDataErrorBuilder>();
    builder->SetResult(static_cast<int32_t>(ComponentTreeQueryError::OK), "{\"name\":\"item-0\"}");
    // Populate cachedItems_ so GetAllChildren() returns non-empty.
    builder->cachedItems_[0] = { "key_0", nullptr };

    LazyForEachMockTree tree;
    tree.Install(900101, 900102, 900103, builder, 50.0f, 50.0f);

    ComponentTreeQueryRequest request;
    request.type = ComponentTreeQueryType::LAZY_FOREACH_DATA_BY_POINT;
    request.x = 50.0f;
    request.y = 50.0f;
    std::string resultJson;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::OK));
    EXPECT_FALSE(resultJson.empty());
    EXPECT_NE(resultJson.find("item-0"), std::string::npos);

    tree.Restore();
}

/**
 * @tc.name: ComponentTreeQueryLazyForEachDataSourceError
 * @tc.desc: getData throws → DATA_SOURCE_ERROR (4) returned immediately (fail-fast).
 * @tc.type: FUNC
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryLazyForEachDataSourceError, TestSize.Level1)
{
    auto builder = AceType::MakeRefPtr<MockDataErrorBuilder>();
    builder->SetResult(static_cast<int32_t>(ComponentTreeQueryError::DATA_SOURCE_ERROR), "");
    builder->cachedItems_[0] = { "key_0", nullptr };

    LazyForEachMockTree tree;
    tree.Install(900111, 900112, 900113, builder, 50.0f, 50.0f);

    ComponentTreeQueryRequest request;
    request.type = ComponentTreeQueryType::LAZY_FOREACH_DATA_BY_POINT;
    request.x = 50.0f;
    request.y = 50.0f;
    std::string resultJson;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::DATA_SOURCE_ERROR));
    EXPECT_FALSE(resultJson.empty());
    EXPECT_NE(resultJson.find("errorCode"), std::string::npos);

    tree.Restore();
}

/**
 * @tc.name: ComponentTreeQueryLazyForEachSerializationError
 * @tc.desc: getData returns non-serializable data → SERIALIZATION_ERROR (5) returned immediately (fail-fast).
 * @tc.type: FUNC
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryLazyForEachSerializationError, TestSize.Level1)
{
    auto builder = AceType::MakeRefPtr<MockDataErrorBuilder>();
    builder->SetResult(static_cast<int32_t>(ComponentTreeQueryError::SERIALIZATION_ERROR), "");
    builder->cachedItems_[0] = { "key_0", nullptr };

    LazyForEachMockTree tree;
    tree.Install(900121, 900122, 900123, builder, 50.0f, 50.0f);

    ComponentTreeQueryRequest request;
    request.type = ComponentTreeQueryType::LAZY_FOREACH_DATA_BY_POINT;
    request.x = 50.0f;
    request.y = 50.0f;
    std::string resultJson;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::SERIALIZATION_ERROR));
    EXPECT_FALSE(resultJson.empty());
    EXPECT_NE(resultJson.find("errorCode"), std::string::npos);

    tree.Restore();
}

/**
 * @tc.name: ComponentTreeQueryLazyForEachEmptyData
 * @tc.desc: getData returns OK but empty string → DATA_SOURCE_ERROR (4).
 * @tc.type: FUNC
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryLazyForEachEmptyData, TestSize.Level1)
{
    auto builder = AceType::MakeRefPtr<MockDataErrorBuilder>();
    builder->SetResult(static_cast<int32_t>(ComponentTreeQueryError::OK), "");
    builder->cachedItems_[0] = { "key_0", nullptr };

    LazyForEachMockTree tree;
    tree.Install(900131, 900132, 900133, builder, 50.0f, 50.0f);

    ComponentTreeQueryRequest request;
    request.type = ComponentTreeQueryType::LAZY_FOREACH_DATA_BY_POINT;
    request.x = 50.0f;
    request.y = 50.0f;
    std::string resultJson;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::DATA_SOURCE_ERROR));
    EXPECT_FALSE(resultJson.empty());
    EXPECT_NE(resultJson.find("errorCode"), std::string::npos);

    tree.Restore();
}

/**
 * @tc.name: ComponentTreeQueryLazyForEachFailFastDiscardsPriorData
 * @tc.desc: Multi-item fail-fast: item 0 succeeds, item 1 returns DATA_SOURCE_ERROR.
 *           Execute must return 4 immediately and NOT include item 0's data in results.
 *           Verifies the BestEffort→FailFast behavior change: previously extracted data
 *           is discarded on the first error.
 * @tc.type: FUNC
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryLazyForEachFailFastDiscardsPriorData, TestSize.Level1)
{
    auto builder = AceType::MakeRefPtr<MockDataErrorBuilder>();
    builder->SetTotalCount(2);
    builder->SetItemResult(0, static_cast<int32_t>(ComponentTreeQueryError::OK), "{\"name\":\"item-0\"}");
    builder->SetItemResult(1, static_cast<int32_t>(ComponentTreeQueryError::DATA_SOURCE_ERROR), "");
    builder->cachedItems_[0] = { "key_0", nullptr };
    builder->cachedItems_[1] = { "key_1", nullptr };

    LazyForEachMockTree tree;
    tree.Install(900141, 900142, 900143, builder, 50.0f, 50.0f);

    ComponentTreeQueryRequest request;
    request.type = ComponentTreeQueryType::LAZY_FOREACH_DATA_BY_POINT;
    request.x = 50.0f;
    request.y = 50.0f;
    std::string resultJson;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::DATA_SOURCE_ERROR));
    EXPECT_FALSE(resultJson.empty());
    // Error response must not contain a results array — item 0's data is discarded.
    EXPECT_EQ(resultJson.find("results"), std::string::npos);
    // item-0 data must not leak into the error response.
    EXPECT_EQ(resultJson.find("item-0"), std::string::npos);

    tree.Restore();
}

/**
 * @tc.name: ComponentTreeQueryLazyForEachFailFastSerializationMidStream
 * @tc.desc: Multi-item fail-fast: item 0 succeeds, item 1 returns SERIALIZATION_ERROR.
 *           Execute must return 5 immediately and discard item 0's data.
 * @tc.type: FUNC
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryLazyForEachFailFastSerializationMidStream, TestSize.Level1)
{
    auto builder = AceType::MakeRefPtr<MockDataErrorBuilder>();
    builder->SetTotalCount(2);
    builder->SetItemResult(0, static_cast<int32_t>(ComponentTreeQueryError::OK), "{\"name\":\"item-0\"}");
    builder->SetItemResult(1, static_cast<int32_t>(ComponentTreeQueryError::SERIALIZATION_ERROR), "");
    builder->cachedItems_[0] = { "key_0", nullptr };
    builder->cachedItems_[1] = { "key_1", nullptr };

    LazyForEachMockTree tree;
    tree.Install(900151, 900152, 900153, builder, 50.0f, 50.0f);

    ComponentTreeQueryRequest request;
    request.type = ComponentTreeQueryType::LAZY_FOREACH_DATA_BY_POINT;
    request.x = 50.0f;
    request.y = 50.0f;
    std::string resultJson;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::SERIALIZATION_ERROR));
    EXPECT_FALSE(resultJson.empty());
    EXPECT_EQ(resultJson.find("results"), std::string::npos);
    EXPECT_EQ(resultJson.find("item-0"), std::string::npos);

    tree.Restore();
}
} // namespace OHOS::Ace::NG
