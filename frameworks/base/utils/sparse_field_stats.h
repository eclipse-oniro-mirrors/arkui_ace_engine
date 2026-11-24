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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_SPARSE_FIELD_STATS_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_SPARSE_FIELD_STATS_H

// Access counters of StableSparseFields — a diagnostic build only (ACE_SPARSE_FIELDS_STATS).
//
// Every Has/Get/Emplace/Reset of every field of every class calls Count(), which bumps its own counter. Once
// per frame FlushFrame() hands the non-zero counters to a sink (e.g. ACE_COUNT_TRACE) and zeroes them. Without
// ACE_SPARSE_FIELDS_STATS only the operation numbers and an empty Count() are left: the storage has no counters
// at all.

#include <cstdint>

#ifdef ACE_SPARSE_FIELDS_STATS
#include <atomic>
#include <cstring>
#include <string>
#include <type_traits>
#endif

namespace OHOS::Ace::SparseFieldStats {
enum Op : uint32_t {
    OP_HAS,
    OP_GET,
    OP_EMPLACE,
    OP_RESET,
};

#ifdef ACE_SPARSE_FIELDS_STATS

inline const char* OpName(uint32_t op)
{
    static const char* const names[] = { "has", "get", "emplace", "reset" };
    return op < sizeof(names) / sizeof(names[0]) ? names[op] : "?";
}

struct Counter;
inline std::atomic<Counter*>& Head()
{
    static std::atomic<Counter*> head { nullptr };
    return head;
}

struct Counter {
    Counter(const char* pretty, const char* field, uint32_t id, uint32_t op)
        : pretty(pretty), field(field), id(id), op(op)
    {
        next = Head().load(std::memory_order_relaxed);
        while (!Head().compare_exchange_weak(next, this, std::memory_order_release, std::memory_order_relaxed)) {
        }
    }
    void Hit()
    {
        hits.fetch_add(1, std::memory_order_relaxed);
    }

    // "Class::field:op", built once. The class is cut out of the __PRETTY_FUNCTION__ of Get<Schema, ...>.
    // Called only from the single-threaded frame-flush sink; if it ever has to be called from several
    // threads, replace the key.empty() check with the standard one-time initialization pattern — a mutable
    // std::once_flag plus std::call_once — and re-check the performance of that variant first.
    const std::string& Key() const
    {
        if (key.empty()) {
            key = OwnerName() + "::" + (field ? std::string(field) : "#" + std::to_string(id)) + ":" + OpName(op);
        }
        return key;
    }

    const char* pretty;
    const char* field;
    uint32_t id;
    uint32_t op;
    std::atomic<uint32_t> hits { 0 };
    Counter* next = nullptr;

private:
    // The __PRETTY_FUNCTION__ layout checked on the OHOS clang 15 toolchain; the fallback returns it as is.
    std::string OwnerName() const
    {
        const char* marker = "Schema = ";
        const char* begin = std::strstr(pretty, marker);
        if (begin == nullptr) {
            return pretty;
        }
        begin += std::strlen(marker);
        std::string name(begin, std::strcspn(begin, ",;]"));
        const std::string suffix = "::SparseFieldSchema";
        if (name.size() > suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0) {
            name.resize(name.size() - suffix.size());
        }
        return name;
    }

    mutable std::string key;
};

template<class Schema, class = void>
struct HasNames : std::false_type {};
template<class Schema>
struct HasNames<Schema, std::void_t<decltype(Schema::kNames)>> : std::true_type {};

template<class Schema>
const char* FieldName(uint32_t id)
{
    if constexpr (HasNames<Schema>::value) {
        // id comes from Get<Schema, Id, Op>, so it is always a valid field index below kFields.
        return Schema::kNames[id];
    } else {
        return nullptr;
    }
}

template<class Schema, uint32_t Id, uint32_t Op>
Counter& Get()
{
    static Counter counter(__PRETTY_FUNCTION__, FieldName<Schema>(Id), Id, Op);
    return counter;
}

// One access of field Id of Schema.
template<class Schema, uint32_t Id, uint32_t Op>
void Count()
{
    Get<Schema, Id, Op>().Hit();
}

// Once per frame: sink(const Counter&, uint32_t hits) for every counter hit since the previous flush.
template<class Sink>
void FlushFrame(Sink&& sink)
{
    for (Counter* c = Head().load(std::memory_order_acquire); c != nullptr; c = c->next) {
        const uint32_t n = c->hits.exchange(0, std::memory_order_relaxed);
        if (n != 0) {
            sink(*c, n);
        }
    }
}

#else

// Not a diagnostic build: an access is not counted, the call compiles to nothing.
template<class Schema, uint32_t Id, uint32_t Op>
void Count()
{}

#endif // ACE_SPARSE_FIELDS_STATS
} // namespace OHOS::Ace::SparseFieldStats

#ifdef ACE_SPARSE_FIELDS_STATS
#define SSF_SCHEMA_NAMES(...) static constexpr const char* kNames[] = { __VA_ARGS__ nullptr }
#else
#define SSF_SCHEMA_NAMES(...) static_assert(true, "")
#endif

// Field name for the schema's kNames (used only with ACE_SPARSE_FIELDS_STATS).
#define SSF_FIELD_NAME(Name, ...) #Name,

#endif // FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_SPARSE_FIELD_STATS_H
