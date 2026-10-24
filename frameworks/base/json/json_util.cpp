/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "base/json/json_util.h"
#include "base/json/json_util_internal.h"

#include "cJSON.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>

namespace OHOS::Ace {

namespace {
thread_local bool* jsonWriteFailure = nullptr;

bool FailJsonWrite()
{
    JsonWriteErrorScope::RecordFailure();
    return false;
}

bool AddOwnedJsonItem(cJSON* parent, const char* key, cJSON* child, bool object)
{
    // Preserve the legacy successful no-op for an empty destination or released value.
    if (!parent || !child) {
        cJSON_Delete(child);
        return true;
    }
    if (object ? cJSON_AddItemToObject(parent, key, child) : cJSON_AddItemToArray(parent, child)) {
        return true;
    }
    cJSON_Delete(child);
    // Keep the legacy return value; collection scopes detect attachment failures separately.
    JsonWriteErrorScope::RecordFailure();
    return true;
}

bool ReplaceOwnedJsonItem(cJSON* parent, const char* key, cJSON* child)
{
    if (!child) {
        return FailJsonWrite();
    }
    if (cJSON_ReplaceItemInObject(parent, key, child)) {
        return true;
    }
    // cJSON assigns the duplicated key before looking up the target. A missing key on
    // the replacement indicates allocation failure; a missing target leaves it intact.
    const bool allocationFailed = child->string == nullptr;
    cJSON_Delete(child);
    return allocationFailed ? FailJsonWrite() : false;
}

} // namespace

JsonWriteErrorScope::JsonWriteErrorScope() : previous_(jsonWriteFailure)
{
    jsonWriteFailure = &failed_;
}

JsonWriteErrorScope::~JsonWriteErrorScope()
{
    jsonWriteFailure = previous_;
    if (failed_ && previous_) {
        *previous_ = true;
    }
}

void JsonWriteErrorScope::RecordFailure()
{
    if (jsonWriteFailure) {
        *jsonWriteFailure = true;
    }
}

JsonValue::JsonValue(JsonObject* object) : object_(object) {}

JsonValue::JsonValue(JsonObject* object, bool isRoot) : object_(object), isRoot_(isRoot) {}

JsonValue::~JsonValue()
{
    if (object_ != nullptr && isRoot_) {
        cJSON_Delete(object_);
    }
    object_ = nullptr;
}

bool JsonValue::IsBool() const
{
    return cJSON_IsBool(object_);
}

bool JsonValue::IsNumber() const
{
    return cJSON_IsNumber(object_);
}

bool JsonValue::IsString() const
{
    return cJSON_IsString(object_);
}

bool JsonValue::IsArray() const
{
    return cJSON_IsArray(object_);
}

bool JsonValue::IsObject() const
{
    return cJSON_IsObject(object_);
}

bool JsonValue::IsValid() const
{
    return (object_ != nullptr) && !cJSON_IsInvalid(object_);
}

bool JsonValue::IsNull() const
{
    return (object_ == nullptr) || cJSON_IsNull(object_);
}

bool JsonValue::Contains(const std::string& key) const
{
    return cJSON_HasObjectItem(object_, key.c_str());
}

bool JsonValue::GetBool() const
{
    return cJSON_IsTrue(object_) != 0;
}

bool JsonValue::GetBool(const std::string& key, bool defaultValue) const
{
    if (Contains(key) && GetValue(key)->IsBool()) {
        return GetValue(key)->GetBool();
    }
    return defaultValue;
}

int32_t JsonValue::GetInt() const
{
    return static_cast<int32_t>((object_ == nullptr) ? 0 : object_->valuedouble);
}

uint32_t JsonValue::GetUInt() const
{
    return static_cast<uint32_t>((object_ == nullptr) ? 0 : object_->valuedouble);
}

int64_t JsonValue::GetInt64() const
{
    return static_cast<int64_t>((object_ == nullptr) ? 0 : object_->valuedouble);
}

double JsonValue::GetDouble() const
{
    return (object_ == nullptr) ? 0.0 : object_->valuedouble;
}

double JsonValue::GetDouble(const std::string& key, double defaultVal) const
{
    auto value = GetValue(key);
    if (value && value->IsNumber()) {
        return value->GetDouble();
    }
    return defaultVal;
}

std::string JsonValue::GetString() const
{
    return ((object_ == nullptr) || (object_->valuestring == nullptr)) ? "" : std::string(object_->valuestring);
}

std::unique_ptr<JsonValue> JsonValue::GetNext() const
{
    if (object_ == nullptr) {
        return std::make_unique<JsonValue>(nullptr);
    }
    return std::make_unique<JsonValue>(object_->next);
}

std::unique_ptr<JsonValue> JsonValue::GetChild() const
{
    if (object_ == nullptr) {
        return std::make_unique<JsonValue>(nullptr);
    }
    return std::make_unique<JsonValue>(object_->child);
}

std::string JsonValue::GetKey() const
{
    return ((object_ == nullptr) || (object_->string == nullptr)) ? "" : std::string(object_->string);
}
std::unique_ptr<JsonValue> JsonValue::GetValue(const std::string& key) const
{
    return std::make_unique<JsonValue>(cJSON_GetObjectItem(object_, key.c_str()));
}

std::unique_ptr<JsonValue> JsonValue::GetObject(const std::string& key) const
{
    if (Contains(key) && GetValue(key)->IsObject()) {
        return GetValue(key);
    }
    return std::make_unique<JsonValue>();
}

int32_t JsonValue::GetArraySize() const
{
    return cJSON_GetArraySize(object_);
}

std::unique_ptr<JsonValue> JsonValue::GetArrayItem(int32_t index) const
{
    return std::make_unique<JsonValue>(cJSON_GetArrayItem(object_, index));
}

bool JsonValue::Put(const char* key, const char* value)
{
    if (!value || !key) {
        return false;
    }

    cJSON* child = cJSON_CreateString(value);
    if (child == nullptr) {
        return FailJsonWrite();
    }
    return AddOwnedJsonItem(object_, key, child, true);
}

bool JsonValue::Put(double value)
{
    cJSON* child = cJSON_CreateNumber(value);
    if (child == nullptr) {
        return FailJsonWrite();
    }
    return AddOwnedJsonItem(object_, nullptr, child, false);
}

bool JsonValue::PutFixedAttr(const char* key, const char* value,
    const NG::InspectorFilter& filter, NG::FixedAttrBit attr)
{
    if (filter.CheckFixedAttr(attr)) {
        return Put(key, value);
    }
    return false;
}

bool JsonValue::PutExtAttr(const char* key, const char* value, const NG::InspectorFilter& filter)
{
    if (filter.CheckExtAttr(key)) {
        return Put(key, value);
    }
    return false;
}

const JsonObject* JsonValue::GetJsonObject() const
{
    return object_;
}

bool JsonValue::Put(const char* key, const std::unique_ptr<JsonValue>& value)
{
    if (!value || !key) {
        return false;
    }
    cJSON* jsonObject = cJSON_Duplicate(value->GetJsonObject(), true);
    if (jsonObject == nullptr) {
        return FailJsonWrite();
    }

    return AddOwnedJsonItem(object_, key, jsonObject, true);
}

bool JsonValue::Put(const char* key, const std::shared_ptr<JsonValue>& value)
{
    if (!value || !key) {
        return false;
    }
    cJSON* jsonObject = cJSON_Duplicate(value->GetJsonObject(), true);
    if (jsonObject == nullptr) {
        return FailJsonWrite();
    }

    return AddOwnedJsonItem(object_, key, jsonObject, true);
}

bool JsonValue::PutFixedAttr(const char* key, const std::unique_ptr<JsonValue>& value,
    const NG::InspectorFilter& filter, NG::FixedAttrBit attr)
{
    if (filter.CheckFixedAttr(attr)) {
        return Put(key, value);
    }
    return false;
}

bool JsonValue::PutExtAttr(const char* key, const std::unique_ptr<JsonValue>& value,
    const NG::InspectorFilter& filter)
{
    if (filter.CheckExtAttr(key)) {
        return Put(key, value);
    }
    return false;
}

// add item to array
bool JsonValue::Put(const std::unique_ptr<JsonValue>& value)
{
    if (!value) {
        return false;
    }
    cJSON* jsonObject = cJSON_Duplicate(value->GetJsonObject(), true);
    if (jsonObject == nullptr) {
        return FailJsonWrite();
    }

    return AddOwnedJsonItem(object_, nullptr, jsonObject, false);
}

bool JsonValue::Put(const std::shared_ptr<JsonValue>& value)
{
    if (!value) {
        return false;
    }
    cJSON* jsonObject = cJSON_Duplicate(value->GetJsonObject(), true);
    if (jsonObject == nullptr) {
        return FailJsonWrite();
    }

    return AddOwnedJsonItem(object_, nullptr, jsonObject, false);
}

bool JsonValue::Put(const char* key, size_t value)
{
    if (key == nullptr) {
        return false;
    }

    cJSON* child = cJSON_CreateNumber(static_cast<double>(value));
    if (child == nullptr) {
        return FailJsonWrite();
    }
    return AddOwnedJsonItem(object_, key, child, true);
}

bool JsonValue::PutFixedAttr(const char* key, size_t value,
    const NG::InspectorFilter& filter, NG::FixedAttrBit attr)
{
    if (filter.CheckFixedAttr(attr)) {
        return Put(key, value);
    }
    return false;
}

bool JsonValue::PutExtAttr(const char* key, size_t value, const NG::InspectorFilter& filter)
{
    if (filter.CheckExtAttr(key)) {
        return Put(key, value);
    }
    return false;
}

bool JsonValue::Put(const char* key, int32_t value)
{
    if (key == nullptr) {
        return false;
    }

    cJSON* child = cJSON_CreateNumber(static_cast<double>(value));
    if (child == nullptr) {
        return FailJsonWrite();
    }
    return AddOwnedJsonItem(object_, key, child, true);
}

bool JsonValue::PutFixedAttr(const char* key, int32_t value,
    const NG::InspectorFilter& filter, NG::FixedAttrBit attr)
{
    if (filter.CheckFixedAttr(attr)) {
        return Put(key, value);
    }
    return false;
}

bool JsonValue::PutExtAttr(const char* key, int32_t value, const NG::InspectorFilter& filter)
{
    if (filter.CheckExtAttr(key)) {
        return Put(key, value);
    }
    return false;
}

bool JsonValue::Put(const char* key, int64_t value)
{
    return Put(key, static_cast<double>(value));
}

bool JsonValue::PutFixedAttr(const char* key, int64_t value,
    const NG::InspectorFilter& filter, NG::FixedAttrBit attr)
{
    if (filter.CheckFixedAttr(attr)) {
        return Put(key, value);
    }
    return false;
}

bool JsonValue::PutExtAttr(const char* key, int64_t value, const NG::InspectorFilter& filter)
{
    if (filter.CheckExtAttr(key)) {
        return Put(key, value);
    }
    return false;
}

bool JsonValue::Put(const char* key, double value)
{
    if (key == nullptr) {
        return false;
    }

    cJSON* child = cJSON_CreateNumber(value);
    if (child == nullptr) {
        return FailJsonWrite();
    }
    return AddOwnedJsonItem(object_, key, child, true);
}

bool JsonValue::PutFixedAttr(const char* key, double value,
    const NG::InspectorFilter& filter, NG::FixedAttrBit attr)
{
    if (filter.CheckFixedAttr(attr)) {
        return Put(key, value);
    }
    return false;
}

bool JsonValue::PutExtAttr(const char* key, double value, const NG::InspectorFilter& filter)
{
    if (filter.CheckExtAttr(key)) {
        return Put(key, value);
    }
    return false;
}

JsonObject* JsonValue::ReleaseJsonObject()
{
    if (!isRoot_) {
        return nullptr;
    }
    JsonObject* object = object_;
    object_ = nullptr;
    return object;
}

bool JsonValue::PutRef(const char* key, std::unique_ptr<JsonValue>&& value)
{
    if (key == nullptr || value == nullptr) {
        return false;
    }
    /*
    * If value is root, it controls the lifecycle of JsonObject, we can just move it into current object
    * Else we need to copy the JsonObject and put the new object in current object
    */
    if (value->isRoot_) {
        return AddOwnedJsonItem(object_, key, value->ReleaseJsonObject(), true);
    } else {
        std::unique_ptr<JsonValue> lValue = std::move(value);
        return Put(key, lValue);
    }
}

bool JsonValue::PutRef(std::unique_ptr<JsonValue>&& value)
{
    if (value == nullptr) {
        return false;
    }
    /*
    * If value is root, it controls the lifecycle of JsonObject, we can just move it into current object
    * Else we need to copy the JsonObject and put the new object in current object
    */
    if (value->isRoot_) {
        return AddOwnedJsonItem(object_, nullptr, value->ReleaseJsonObject(), false);
    } else {
        std::unique_ptr<JsonValue> lValue = std::move(value);
        return Put(lValue);
    }
}

bool JsonValue::PutRef(std::shared_ptr<JsonValue>&& value)
{
    if (value == nullptr) {
        return false;
    }
    /*
    * If value is root, it controls the lifecycle of JsonObject, we can just move it into current object
    * Else we need to copy the JsonObject and put the new object in current object
    */
    if (value->isRoot_) {
        return AddOwnedJsonItem(object_, nullptr, value->ReleaseJsonObject(), false);
    } else {
        std::shared_ptr<JsonValue> lValue = std::move(value);
        return Put(lValue);
    }
}

bool JsonValue::Replace(const char* key, double value)
{
    if (key == nullptr) {
        return false;
    }

    return ReplaceOwnedJsonItem(object_, key, cJSON_CreateNumber(value));
}

bool JsonValue::Put(const char* key, bool value)
{
    if (key == nullptr) {
        return false;
    }

    cJSON* child = cJSON_CreateBool(value);
    if (child == nullptr) {
        return FailJsonWrite();
    }
    return AddOwnedJsonItem(object_, key, child, true);
}

bool JsonValue::PutFixedAttr(const char* key, bool value,
    const NG::InspectorFilter& filter, NG::FixedAttrBit attr)
{
    if (filter.CheckFixedAttr(attr)) {
        return Put(key, value);
    }
    return false;
}

bool JsonValue::PutExtAttr(const char* key, bool value, const NG::InspectorFilter& filter)
{
    if (filter.CheckExtAttr(key)) {
        return Put(key, value);
    }
    return false;
}

bool JsonValue::Replace(const char* key, bool value)
{
    if (key == nullptr) {
        return false;
    }

    return ReplaceOwnedJsonItem(object_, key, cJSON_CreateBool(value));
}

bool JsonValue::Replace(const char* key, const char* value)
{
    if ((value == nullptr) || (key == nullptr)) {
        return false;
    }

    return ReplaceOwnedJsonItem(object_, key, cJSON_CreateString(value));
}

bool JsonValue::Replace(const char* key, int32_t value)
{
    if (key == nullptr) {
        return false;
    }

    return ReplaceOwnedJsonItem(object_, key, cJSON_CreateNumber(static_cast<double>(value)));
}

bool JsonValue::Replace(const char* key, const std::unique_ptr<JsonValue>& value)
{
    if ((value == nullptr) || (key == nullptr) || (value->GetJsonObject() == nullptr)) {
        return false;
    }
    return ReplaceOwnedJsonItem(object_, key, cJSON_Duplicate(value->GetJsonObject(), true));
}

bool JsonValue::Delete(const char* key)
{
    if (key == nullptr) {
        return false;
    }
    cJSON_DeleteItemFromObject(object_, key);
    return true;
}

std::string JsonValue::ToString()
{
    std::string result;
    if (!object_) {
        return result;
    }

    // It is null-terminated.
    char* unformatted = cJSON_PrintUnformatted(object_);
    if (unformatted != nullptr) {
        result = unformatted;
        cJSON_free(unformatted);
    }
    return result;
}

std::string JsonValue::GetString(const std::string& key, const std::string& defaultVal) const
{
    auto value = GetValue(key);
    if (value && value->IsString()) {
        return value->GetString();
    }
    return defaultVal;
}

int32_t JsonValue::GetInt(const std::string& key, int32_t defaultVal) const
{
    auto value = GetValue(key);
    if (value && value->IsNumber()) {
        return value->GetInt();
    }
    return defaultVal;
}

uint32_t JsonValue::GetUInt(const std::string& key, uint32_t defaultVal) const
{
    auto value = GetValue(key);
    if (value && value->IsNumber()) {
        return value->GetUInt();
    }
    return defaultVal;
}

int64_t JsonValue::GetInt64(const std::string& key, int64_t defaultVal) const
{
    auto value = GetValue(key);
    if (value && value->IsNumber()) {
        return value->GetInt64();
    }
    return defaultVal;
}

std::unique_ptr<JsonValue> JsonValue::Duplicate()
{
    if (object_ == nullptr) {
        return std::make_unique<JsonValue>(nullptr);
    }
    cJSON* jsonObject = cJSON_Duplicate(object_, true);
    if (jsonObject == nullptr) {
        return std::make_unique<JsonValue>(nullptr);
    }
    return std::make_unique<JsonValue>(jsonObject, true);
}

std::unique_ptr<JsonValue> JsonUtil::ParseJsonData(const char* data, const char** parseEnd)
{
    return std::make_unique<JsonValue>(cJSON_ParseWithOpts(data, parseEnd, true), true);
}

#if defined(ACE_STATIC)
std::unique_ptr<JsonValue> JsonUtil::ParseJsonDataWithLength(const char* data, size_t len)
{
    return std::make_unique<JsonValue>(cJSON_ParseWithLength(data, len), true);
}
#endif

std::unique_ptr<JsonValue> JsonUtil::ParseJsonString(const std::string& content, const char** parseEnd)
{
    return ParseJsonData(content.c_str(), parseEnd);
}

std::unique_ptr<JsonValue> JsonUtil::Create(bool isRoot)
{
    auto* object = cJSON_CreateObject();
    if (!object) {
        JsonWriteErrorScope::RecordFailure();
    }
    return std::make_unique<JsonValue>(object, isRoot);
}

std::shared_ptr<JsonValue> JsonUtil::CreateSharedPtrJson(bool isRoot)
{
    auto* object = cJSON_CreateObject();
    if (!object) {
        JsonWriteErrorScope::RecordFailure();
    }
    return std::make_shared<JsonValue>(object, isRoot);
}

std::unique_ptr<JsonValue> JsonUtil::CreateArray(bool isRoot)
{
    auto* object = cJSON_CreateArray();
    if (!object) {
        JsonWriteErrorScope::RecordFailure();
    }
    return std::make_unique<JsonValue>(object, isRoot);
}

bool PutJsonObjectReference(JsonValue& destination, const char* key, const JsonValue& source)
{
    if (!key || !source.IsObject()) {
        return false;
    }
    auto* reference = cJSON_CreateObjectReference(source.GetJsonObject()->child);
    if (reference && cJSON_AddItemToObject(const_cast<cJSON*>(destination.GetJsonObject()), key, reference)) {
        return true;
    }
    cJSON_Delete(reference);
    JsonWriteErrorScope::RecordFailure();
    return false;
}

namespace {
// cJSON_PrintPreallocated documents five extra bytes for conservative size estimates.
constexpr size_t CJSON_PRINT_SAFETY_MARGIN = 5;
constexpr size_t JSON_TERMINATOR_SIZE = sizeof(char);

// Native cJSON uses 26 bytes for number formatting, including its terminator.
constexpr size_t CJSON_NUMBER_BUFFER_SIZE = 26;
constexpr unsigned char JSON_CONTROL_CHARACTER_END = 0x20;
constexpr size_t JSON_QUOTES_SIZE = sizeof("\"\"") - 1;
constexpr size_t JSON_SHORT_ESCAPE_SIZE = sizeof("\\n") - 1;
constexpr size_t JSON_UNICODE_ESCAPE_SIZE = sizeof("\\u0000") - 1;

class CompactJsonCounter final {
public:
    explicit CompactJsonCounter(size_t limit) : limit_(limit)
    {}

    JsonWriteResult Count(const cJSON* value, const char* shallowArrayKey = nullptr)
    {
        if (!value) {
            return JsonWriteResult::INVALID_VALUE;
        }
        if (cJSON_IsObject(value)) {
            return CountObject(value, shallowArrayKey);
        }
        if (cJSON_IsArray(value)) {
            return CountArray(value, false);
        }
        if (cJSON_IsString(value)) {
            return value->valuestring ? CountString(value->valuestring) : JsonWriteResult::INVALID_VALUE;
        }
        return CountScalar(value);
    }

    size_t Size() const
    {
        return size_;
    }

private:
    JsonWriteResult Add(size_t bytes)
    {
        if (bytes > limit_ - size_) {
            return JsonWriteResult::TOO_LARGE;
        }
        size_ += bytes;
        return JsonWriteResult::SUCCESS;
    }

    JsonWriteResult CountString(const char* value)
    {
        auto result = Add(JSON_QUOTES_SIZE);
        // cJSON prints an absent object-member key as an empty string.
        for (const auto* cursor = reinterpret_cast<const unsigned char*>(value);
             result == JsonWriteResult::SUCCESS && cursor && *cursor; ++cursor) {
            size_t bytes = sizeof(char);
            if (*cursor == '\"' || *cursor == '\\' || *cursor == '\b' || *cursor == '\f' ||
                *cursor == '\n' || *cursor == '\r' || *cursor == '\t') {
                bytes = JSON_SHORT_ESCAPE_SIZE;
            } else if (*cursor < JSON_CONTROL_CHARACTER_END) {
                bytes = JSON_UNICODE_ESCAPE_SIZE;
            }
            result = Add(bytes);
        }
        return result;
    }

    JsonWriteResult CountScalar(const cJSON* value)
    {
        if (cJSON_IsBool(value)) {
            return Add(cJSON_IsTrue(value) ? sizeof("true") - 1 : sizeof("false") - 1);
        }
        if (cJSON_IsNull(value)) {
            return Add(sizeof("null") - 1);
        }
        if (!cJSON_IsNumber(value) || !std::isfinite(value->valuedouble)) {
            return JsonWriteResult::INVALID_VALUE;
        }
        char buffer[CJSON_NUMBER_BUFFER_SIZE + CJSON_PRINT_SAFETY_MARGIN] {};
        if (!cJSON_PrintPreallocated(const_cast<cJSON*>(value), buffer, static_cast<int>(sizeof(buffer)), false)) {
            return JsonWriteResult::INVALID_VALUE;
        }
        return Add(std::strlen(buffer));
    }

    JsonWriteResult CountArray(const cJSON* value, bool shallow)
    {
        auto result = Add(sizeof("[]") - 1);
        for (auto* child = value->child; child && result == JsonWriteResult::SUCCESS; child = child->next) {
            if (child != value->child) {
                result = Add(sizeof(",") - 1);
            }
            if (!shallow && result == JsonWriteResult::SUCCESS) {
                result = Count(child);
            }
        }
        return result;
    }

    JsonWriteResult CountObject(const cJSON* value, const char* shallowArrayKey)
    {
        auto result = Add(sizeof("{}") - 1);
        for (auto* child = value->child; child && result == JsonWriteResult::SUCCESS; child = child->next) {
            if (child != value->child) {
                result = Add(sizeof(",") - 1);
            }
            if (result == JsonWriteResult::SUCCESS) {
                result = CountString(child->string);
            }
            if (result == JsonWriteResult::SUCCESS) {
                result = Add(sizeof(":") - 1);
            }
            if (result == JsonWriteResult::SUCCESS) {
                bool shallow = shallowArrayKey && child->string && cJSON_IsArray(child) &&
                    std::strcmp(child->string, shallowArrayKey) == 0;
                result = shallow ? CountArray(child, true) : Count(child);
            }
        }
        return result;
    }

    const size_t limit_;
    size_t size_ = 0;
};

bool IsPrintableJson(const cJSON* value)
{
    if (!value) {
        return false;
    }
    if (cJSON_IsObject(value) || cJSON_IsArray(value)) {
        for (auto* child = value->child; child; child = child->next) {
            if (!IsPrintableJson(child)) {
                return false;
            }
        }
        return true;
    }
    if (cJSON_IsString(value)) {
        return value->valuestring != nullptr;
    }
    if (cJSON_IsNumber(value)) {
        return std::isfinite(value->valuedouble);
    }
    return cJSON_IsBool(value) || cJSON_IsNull(value);
}
} // namespace

JsonWriteResult MeasureCompactJson(const JsonValue& value, size_t limit, size_t& size, const char* shallowArrayKey)
{
    CompactJsonCounter counter(limit);
    auto result = counter.Count(value.GetJsonObject(), shallowArrayKey);
    size = result == JsonWriteResult::SUCCESS ? counter.Size() : 0;
    return result;
}

JsonWriteResult WriteJsonBounded(const JsonValue& value, bool pretty, size_t limit, char** data, size_t* size)
{
    if (data) {
        *data = nullptr;
    }
    if (size) {
        *size = 0;
    }
    constexpr size_t BUFFER_OVERHEAD = JSON_TERMINATOR_SIZE + CJSON_PRINT_SAFETY_MARGIN;
    if (!data || !size || limit > static_cast<size_t>(std::numeric_limits<int>::max()) - BUFFER_OVERHEAD ||
        !IsPrintableJson(value.GetJsonObject())) {
        return JsonWriteResult::INVALID_VALUE;
    }
    const auto capacity = limit + BUFFER_OVERHEAD;
    auto* buffer = static_cast<char*>(std::malloc(capacity));
    if (!buffer) {
        return JsonWriteResult::NO_MEMORY;
    }
    // Validate values first: preallocated printing does not allocate or report a detailed failure reason.
    if (!cJSON_PrintPreallocated(
        const_cast<cJSON*>(value.GetJsonObject()), buffer, static_cast<int>(capacity), pretty)) {
        std::free(buffer);
        return JsonWriteResult::TOO_LARGE;
    }
    const auto length = std::strlen(buffer);
    if (length > limit) {
        std::free(buffer);
        return JsonWriteResult::TOO_LARGE;
    }
    *data = buffer;
    *size = length;
    return JsonWriteResult::SUCCESS;
}

} // namespace OHOS::Ace
