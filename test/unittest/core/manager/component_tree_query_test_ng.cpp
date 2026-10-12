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
#include "interfaces/inner_api/ui_session/component_tree_query_type.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {

class ComponentTreeQueryTest : public testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: ComponentTreeQueryPointHit001
 * @tc.desc: point-in-rect with boundary inclusive + finite check.
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryPointHit001, TestSize.Level1)
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
 * @tc.name: ComponentTreeQueryNavigationInvalidParams002
 * @tc.desc: invalid Navigation query parameters return error codes
 * with errorCode field in resultJson.
 */
HWTEST_F(ComponentTreeQueryTest, ComponentTreeQueryNavigationInvalidParams002, TestSize.Level1)
{
    ComponentTreeQueryRequest request;
    std::string resultJson;

    // Empty pattern -> INVALID_PARAMETER.
    request.type = ComponentTreeQueryType::NAVIGATION_CONTENT_BY_POINT;
    request.x = 10.0f;
    request.y = 10.0f;
    request.pattern = "";
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::INVALID_PARAMETER));
    EXPECT_FALSE(resultJson.empty());

    // Non-finite coordinates -> INVALID_PARAMETER.
    request.pattern = "[a-z]+";
    request.x = std::nanf("");
    request.y = 100.0f;
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::INVALID_PARAMETER));
    EXPECT_FALSE(resultJson.empty());

    // Oversize regex -> RESOURCE_LIMIT.
    request.x = 10.0f;
    request.y = 10.0f;
    request.pattern = std::string(COMPONENT_TREE_QUERY_MAX_REGEX_BYTES + 1, 'a');
    EXPECT_EQ(ComponentTreeQuery::Execute(request, resultJson),
        static_cast<int32_t>(ComponentTreeQueryError::RESOURCE_LIMIT));
    EXPECT_FALSE(resultJson.empty());
}
} // namespace OHOS::Ace::NG
