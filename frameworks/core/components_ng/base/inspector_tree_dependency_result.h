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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_TREE_DEPENDENCY_RESULT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_TREE_DEPENDENCY_RESULT_H

#include <string_view>

namespace OHOS::Ace::NG {
enum class InspectorTreeError {
    NONE,
    COLLECTION_FAILED,
    PROVIDER_QUERY_FAILED,
    INVALID_PROVIDER_DATA,
    NODE_COUNT_EXCEEDS_LIMIT,
    TREE_DATA_SIZE_EXCEEDS_LIMIT,
};

// Consumes dependency outcomes; structural and size validation belong to the dependency.
class InspectorTreeDependencyResult {
public:
    void ReportError(InspectorTreeError error)
    {
        if (error_ == InspectorTreeError::NONE) {
            error_ = error;
        }
    }

    bool HasError() const
    {
        return error_ != InspectorTreeError::NONE;
    }

    const char* GetErrorMessage() const
    {
        switch (error_) {
            case InspectorTreeError::NONE:
                return "no error";
            case InspectorTreeError::PROVIDER_QUERY_FAILED:
                return "provider query failed";
            case InspectorTreeError::INVALID_PROVIDER_DATA:
                return "invalid provider data";
            case InspectorTreeError::NODE_COUNT_EXCEEDS_LIMIT:
                return "node count exceeds limit";
            case InspectorTreeError::TREE_DATA_SIZE_EXCEEDS_LIMIT:
                return "tree data size exceeds limit";
            default:
                return "collection failed";
        }
    }

    void ReportErrorMessage(std::string_view message)
    {
        if (message == "no error") {
            return;
        }
        if (message == "provider query failed") {
            ReportError(InspectorTreeError::PROVIDER_QUERY_FAILED);
        } else if (message == "invalid provider data") {
            ReportError(InspectorTreeError::INVALID_PROVIDER_DATA);
        } else if (message == "node count exceeds limit") {
            ReportError(InspectorTreeError::NODE_COUNT_EXCEEDS_LIMIT);
        } else if (message == "tree data size exceeds limit") {
            ReportError(InspectorTreeError::TREE_DATA_SIZE_EXCEEDS_LIMIT);
        } else {
            ReportError(InspectorTreeError::COLLECTION_FAILED);
        }
    }

private:
    InspectorTreeError error_ = InspectorTreeError::NONE;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_BASE_INSPECTOR_TREE_DEPENDENCY_RESULT_H
