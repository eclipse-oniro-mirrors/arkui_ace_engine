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
#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_JSON_JSON_UTIL_INTERNAL_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_JSON_JSON_UTIL_INTERNAL_H

#include "base/json/json_util.h"

namespace OHOS::Ace {
// Tracks failed writes, including those made by existing readers that ignore Put's return value.
// Invalid input arguments return false without marking the scope as failed.
// Each scope belongs to its calling thread; nested failures propagate to the enclosing scope.
class ACE_EXPORT JsonWriteErrorScope final {
public:
    JsonWriteErrorScope();
    ~JsonWriteErrorScope();
    JsonWriteErrorScope(const JsonWriteErrorScope&) = delete;
    JsonWriteErrorScope& operator=(const JsonWriteErrorScope&) = delete;
    bool HasError() const
    {
        return failed_;
    }
    static void RecordFailure();

private:
    bool failed_ = false;
    bool* previous_ = nullptr;
};

enum class JsonWriteResult { SUCCESS, TOO_LARGE, NO_MEMORY, INVALID_VALUE };

// The source must outlive the destination. Only the reference wrapper is owned by destination.
ACE_EXPORT bool PutJsonObjectReference(JsonValue& destination, const char* key, const JsonValue& source);

// Counts native COMPACT bytes without allocating an output buffer; size excludes NUL.
// If supplied, shallowArrayKey counts a root object's array field structure but omits its elements' values.
// This lets tree collection count each node once, including all child separators.
ACE_EXPORT JsonWriteResult MeasureCompactJson(
    const JsonValue& value, size_t limit, size_t& size, const char* shallowArrayKey = nullptr);

// On success transfers a malloc-owned, NUL-terminated buffer; size excludes NUL.
// On failure clears both outputs. Uses cJSON native formatting and recursive printing.
// The output limit excludes NUL; the buffer includes cJSON print safety padding.
ACE_EXPORT JsonWriteResult WriteJsonBounded(
    const JsonValue& value, bool pretty, size_t limit, char** data, size_t* size);
} // namespace OHOS::Ace

#endif
