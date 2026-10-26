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
#include "core/components_ng/syntax/lazy_for_each_builder.h"
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
 *           target-not-found maps to OK and a non-empty error payload is produced.
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
        static_cast<int32_t>(ComponentTreeQueryError::OK));
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
} // namespace OHOS::Ace::NG
