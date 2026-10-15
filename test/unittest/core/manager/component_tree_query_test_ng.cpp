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
} // namespace OHOS::Ace::NG
