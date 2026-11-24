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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_SPARSE_FIELDS_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_SPARSE_FIELDS_H

// Macros that move rarely set fields of a class into sparse storage with a single line.
//
// Usage: a class lists its rarely set fields in a macro and puts SPARSE_FIELDS(LIST) in its body. The
// list holds entries F(Name, Type) — for example Shadow of ShadowStyle, DragPreview of DragPreviewInfo,
// and Callbacks of a std::map<int32_t, std::function<void()>> (a type with commas is taken as a whole).
// The macro expands to the accessors below and leaves the class in private:, as at a class start.
//
// Each list entry F(Name, Type) gives the class the methods:
//
//   bool HasName() const                        — whether the field is set
//   const Type* GetName() const / Type* GetName() — pointer to the value or nullptr; edit in place
//   Type& EmplaceName(args...)                  — create (or recreate in place) from arguments
//   Type& GetOrEmplaceName(args...)             — return the existing value or create it from arguments
//   void ResetName()                            — remove the field
//
// Guarantee: a non-null pointer from GetName() stays valid until ResetName() or until the object dies,
// whatever happens to other fields. While no field is set the object holds 24 bytes (16 with more than
// 64 fields) and no heap allocations. Storage layout — stable_sparse_fields.h.
//
// Converting an existing field: a std::optional<ShadowStyle> shadow_ becomes an entry F(Shadow,
// ShadowStyle) — a set field is read through GetShadow() (a pointer, nullptr when unset) instead of the
// optional's operator bool; assigning shadow_ = style becomes EmplaceShadow(style); editing in place
// shadow_->radius = 4 becomes GetShadow()->radius = 4 when the field is surely set; clearing
// shadow_.reset() becomes ResetShadow(). A std::unique_ptr<DragPreviewInfo> drag_ becomes an entry
// F(DragPreview, DragPreviewInfo) with EmplaceDragPreview(a, b), GetOrEmplaceDragPreview() and
// GetDragPreview() instead of drag_.get().
//
// Rules:
//   * a type with commas is written as is: F(Callbacks, std::map<int32_t, Fn>) — the whole type is taken;
//   * the field type must be complete where the class is declared (as with std::optional);
//   * up to 64 fields — 24 bytes in the object (mask + slot numbers + pointer); from 65 to 254 the mask
//     is turned off automatically, the object is 16 bytes and lookups of unset fields are slower;
//     field numbers follow the list order;
//   * 8 slots per block by default; for classes where almost every object has at most four rare fields
//     set — SPARSE_FIELDS_WITH_SLOTS(LIST, 4): blocks of 4 slots, the second one is allocated on demand;
//   * a derived class may declare its own list — base and derived storages are separate.

#include <cstddef>
#include <cstdint>
#include <utility>

#include "base/utils/stable_sparse_fields.h"

namespace OHOS::Ace::SparseFieldsDetail {
// List of field types and type selection by number. The last element is a void placeholder after the
// trailing comma left by the macro; no field number ever reaches it.
template<class... Ts>
struct TypeList {};

template<size_t I, class List>
struct TypeAtImpl;
template<class Head, class... Tail>
struct TypeAtImpl<0, TypeList<Head, Tail...>> {
    using type = Head;
};
template<size_t I, class Head, class... Tail>
struct TypeAtImpl<I, TypeList<Head, Tail...>> {
    using type = typename TypeAtImpl<I - 1, TypeList<Tail...>>::type;
};
template<size_t I, class List>
using TypeAt = typename TypeAtImpl<I, List>::type;
} // namespace OHOS::Ace::SparseFieldsDetail

// Field number; the type is taken from __VA_ARGS__ so that it may contain commas.
#define SSF_FIELD_ID(Name, ...) Name,
#define SSF_FIELD_TYPE(Name, ...) __VA_ARGS__,

#define SSF_FIELD_ACCESSORS(Name, ...)                                                               \
    bool Has##Name() const                                                                           \
    {                                                                                                \
        return sparseFields_.template Has<SparseFieldIds::Name>();                                   \
    }                                                                                                \
    const __VA_ARGS__* Get##Name() const                                                             \
    {                                                                                                \
        return sparseFields_.template Get<SparseFieldIds::Name>();                                   \
    }                                                                                                \
    __VA_ARGS__* Get##Name()                                                                         \
    {                                                                                                \
        return sparseFields_.template Get<SparseFieldIds::Name>();                                   \
    }                                                                                                \
    template<class... SsfArgs>                                                                       \
    __VA_ARGS__& Emplace##Name(SsfArgs&&... args)                                                    \
    {                                                                                                \
        return sparseFields_.template Emplace<SparseFieldIds::Name>(std::forward<SsfArgs>(args)...); \
    }                                                                                                \
    template<class... SsfArgs>                                                                       \
    __VA_ARGS__& GetOrEmplace##Name(SsfArgs&&... args)                                               \
    {                                                                                                \
        if (__VA_ARGS__* existing = Get##Name()) {                                                   \
            return *existing;                                                                        \
        }                                                                                            \
        return Emplace##Name(std::forward<SsfArgs>(args)...);                                        \
    }                                                                                                \
    void Reset##Name()                                                                               \
    {                                                                                                \
        sparseFields_.template Reset<SparseFieldIds::Name>();                                        \
    }

// Sparse fields of a class from list LIST; SLOTS — slots per block (8 or 4).
#define SPARSE_FIELDS_WITH_SLOTS(LIST, SLOTS)                                               \
public:                                                                                     \
    struct SparseFieldIds {                                                                 \
        enum : uint32_t { LIST(SSF_FIELD_ID) kCount };                                      \
    };                                                                                      \
    struct SparseFieldSchema {                                                              \
        using Types = ::OHOS::Ace::SparseFieldsDetail::TypeList<LIST(SSF_FIELD_TYPE) void>; \
        static constexpr uint32_t kFields = SparseFieldIds::kCount;                         \
        template<uint32_t Id>                                                               \
        using Type = ::OHOS::Ace::SparseFieldsDetail::TypeAt<Id, Types>;                    \
        SSF_SCHEMA_NAMES(LIST(SSF_FIELD_NAME));                                             \
    };                                                                                      \
    LIST(SSF_FIELD_ACCESSORS)                                                               \
                                                                                            \
private:                                                                                    \
    ::OHOS::Ace::StableSparseFields<SparseFieldSchema, SLOTS, (SparseFieldIds::kCount <= 64)> sparseFields_

#define SPARSE_FIELDS(LIST) SPARSE_FIELDS_WITH_SLOTS(LIST, 8)

#endif // FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_SPARSE_FIELDS_H
