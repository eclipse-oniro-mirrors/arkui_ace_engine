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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_UISESSION_QUERY_COMPONENT_TREE_QUERY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_UISESSION_QUERY_COMPONENT_TREE_QUERY_H

#include <string>

#include "base/geometry/ng/rect_t.h"
#include "base/utils/macros.h"
#include "interfaces/inner_api/ui_session/component_tree_query_type.h"

namespace OHOS::Ace::NG {

class ACE_FORCE_EXPORT ComponentTreeQuery {
public:
    /**
     * @brief Dispatch one query request. Always fills resultJson (error payload
     * included) unless the request itself can not be executed at all.
     * @return ComponentTreeQueryError of the executed query.
     */
    static int32_t Execute(const ComponentTreeQueryRequest& request, std::string& resultJson);

    static bool IsPointInRectBoundaryInclusive(float x, float y, const RectF& rect);
    static bool IsFinitePoint(float x, float y);
    static int32_t CheckRegex(const std::string& pattern);

    // Geometry helpers for region queries, exposed for unit tests.
    // Strict positive-area intersection: tangent / edge-touching does not count.
    static bool IsPositiveAreaRectIntersection(const RectF& lhs, const RectF& rhs);
    // Normalizes any two diagonal corners (x1,y1)-(x2,y2) into a left-top origin rect.
    static RectF NormalizeRect(float x1, float y1, float x2, float y2);

private:
    static int32_t QueryNavigationContentByPoint(float x, float y, const std::string& pattern,
        std::string& resultJson);
    static int32_t QueryLazyForEachDataByPoint(float x, float y, std::string& resultJson);
    static int32_t QueryNodesInRect(float x1, float y1, float x2, float y2, std::string& resultJson);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_UISESSION_QUERY_COMPONENT_TREE_QUERY_H
