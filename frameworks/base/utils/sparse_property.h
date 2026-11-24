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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_SPARSE_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_SPARSE_PROPERTY_H

// Properties (ACE_DEFINE_PROPERTY_*) in the class's sparse storage, with the same public API.
//
// A list mixes two kinds of entries: F(Name, Type) is a sparse field — the accessors HasX/GetX/EmplaceX/
// ... of SPARSE_FIELDS; P(Name, Type) is a property — a number and a type only, its API is defined below.
//
// A class puts SPARSE_FIELDS_WITH_PROPS(LIST) FIRST in its body (it leaves private:, as at a class
// start): the property macros below name the storage type in their return types, so it must be declared
// above them. A property group struct puts SPARSE_FIELDS_GROUP(LIST) first in its own body (it leaves
// public:, as at a struct start) and gets its own copyable storage.
//
// Differences from the original ACE_DEFINE_PROPERTY_* macros:
//   * Get##name() returns a live view (SparsePropView) with the interface of const std::optional<T>&:
//     every access asks the storage again. `auto x = GetX()` is a view, not a snapshot — take
//     GetX().ToOptional() for a snapshot; template deduction and `?:` need .ToOptional() too;
//   * group getters return a live pointer-like view (SparseGroupView) instead of const unique_ptr&;
//   * no prop##name##_ / prop##name members: reads — Get##name(), raw writes — Assign##name(), which
//     like the former direct assignment skips NearEqual, change flags and callbacks;
//   * Update semantics, access specifiers after the macro and all other signatures are unchanged.

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "base/utils/sparse_fields.h"

namespace OHOS::Ace {
// Live read-only view of one property: the interface of const std::optional<T>&, no copy of T.
template<class Storage, uint32_t Id>
class SparsePropView {
public:
    using value_type = typename Storage::template T<Id>;

    explicit SparsePropView(const Storage* storage) : storage_(storage) {}

    bool has_value() const
    {
        return storage_->template Has<Id>();
    }
    explicit operator bool() const
    {
        return has_value();
    }
    const value_type& value() const
    {
        const value_type* p = Ptr();
        if (p == nullptr) {
            std::abort(); // as std::optional::value() without exceptions
        }
        return *p;
    }
    const value_type& operator*() const
    {
        return *Ptr();
    }
    const value_type* operator->() const
    {
        return Ptr();
    }
    template<class U>
    value_type value_or(U&& defaultValue) const
    {
        const value_type* p = Ptr();
        return p != nullptr ? *p : static_cast<value_type>(std::forward<U>(defaultValue));
    }
    std::optional<value_type> ToOptional() const
    {
        const value_type* p = Ptr();
        return p != nullptr ? std::optional<value_type>(*p) : std::nullopt;
    }
    operator std::optional<value_type>() const
    {
        return ToOptional();
    }

    friend bool operator==(const SparsePropView& a, const SparsePropView& b)
    {
        const value_type* pa = a.Ptr();
        const value_type* pb = b.Ptr();
        return (pa == nullptr || pb == nullptr) ? pa == pb : *pa == *pb;
    }
    friend bool operator==(const SparsePropView& a, const std::optional<value_type>& b)
    {
        const value_type* pa = a.Ptr();
        return (pa == nullptr || !b) ? (pa == nullptr) == !b : *pa == *b;
    }
    friend bool operator==(const std::optional<value_type>& b, const SparsePropView& a)
    {
        return a == b;
    }
    friend bool operator==(const SparsePropView& a, std::nullopt_t)
    {
        return !a.has_value();
    }
    friend bool operator==(std::nullopt_t, const SparsePropView& a)
    {
        return !a.has_value();
    }
    friend bool operator==(const SparsePropView& a, const value_type& b)
    {
        const value_type* pa = a.Ptr();
        return pa != nullptr && *pa == b;
    }
    friend bool operator==(const value_type& b, const SparsePropView& a)
    {
        return a == b;
    }
    friend bool operator!=(const SparsePropView& a, const SparsePropView& b)
    {
        return !(a == b);
    }
    friend bool operator!=(const SparsePropView& a, const std::optional<value_type>& b)
    {
        return !(a == b);
    }
    friend bool operator!=(const std::optional<value_type>& b, const SparsePropView& a)
    {
        return !(a == b);
    }
    friend bool operator!=(const SparsePropView& a, std::nullopt_t)
    {
        return a.has_value();
    }
    friend bool operator!=(std::nullopt_t, const SparsePropView& a)
    {
        return a.has_value();
    }
    friend bool operator!=(const SparsePropView& a, const value_type& b)
    {
        return !(a == b);
    }
    friend bool operator!=(const value_type& b, const SparsePropView& a)
    {
        return !(a == b);
    }

private:
    const value_type* Ptr() const
    {
        return storage_->template Get<Id>();
    }

    const Storage* storage_;
};

// Live view of a property group: the interface of const std::unique_ptr<G>&. As before, the group is
// mutable through the view even when it was taken from a const owner (unique_ptr gave a non-const G*).
template<class Storage, uint32_t Id>
class SparseGroupView {
public:
    using element_type = typename Storage::template T<Id>;

    explicit SparseGroupView(const Storage* storage) : storage_(storage) {}

    element_type* get() const
    {
        return const_cast<Storage*>(storage_)->template Get<Id>();
    }
    element_type* operator->() const
    {
        return get();
    }
    element_type& operator*() const
    {
        return *get();
    }
    explicit operator bool() const
    {
        return storage_->template Has<Id>();
    }

    friend bool operator==(const SparseGroupView& a, std::nullptr_t)
    {
        return !a;
    }
    friend bool operator==(std::nullptr_t, const SparseGroupView& a)
    {
        return !a;
    }
    friend bool operator!=(const SparseGroupView& a, std::nullptr_t)
    {
        return static_cast<bool>(a);
    }
    friend bool operator!=(std::nullptr_t, const SparseGroupView& a)
    {
        return static_cast<bool>(a);
    }

private:
    const Storage* storage_;
};

namespace SparseFieldsDetail {
// Raw write, as the former `prop##name##_ = value`: in place if set, otherwise created.
template<uint32_t Id, class Storage, class V>
void AssignProp(Storage& storage, V&& value)
{
    if (auto* current = storage.template Get<Id>()) {
        *current = std::forward<V>(value);
    } else {
        storage.template Emplace<Id>(std::forward<V>(value));
    }
}
} // namespace SparseFieldsDetail
} // namespace OHOS::Ace

// ---------------------------------------------------------------------------------------------------------
// Storage: a list LIST(F, P). F entries get accessors as in SPARSE_FIELDS, P entries only a number and a type.
// ---------------------------------------------------------------------------------------------------------

#define SSF_NO_ACCESSORS(Name, ...)

#define SSF_SCHEMA_TYPES(LIST)                                                                              \
    struct SparseFieldIds {                                                                                 \
        enum : uint32_t { LIST(SSF_FIELD_ID, SSF_FIELD_ID) kCount };                                        \
    };                                                                                                      \
    struct SparseFieldSchema {                                                                              \
        using Types = ::OHOS::Ace::SparseFieldsDetail::TypeList<LIST(SSF_FIELD_TYPE, SSF_FIELD_TYPE) void>; \
        static constexpr uint32_t kFields = SparseFieldIds::kCount;                                         \
        template<uint32_t Id>                                                                               \
        using Type = ::OHOS::Ace::SparseFieldsDetail::TypeAt<Id, Types>;                                    \
        SSF_SCHEMA_NAMES(LIST(SSF_FIELD_NAME, SSF_FIELD_NAME));                                             \
    };                                                                                                      \
    static_assert(true, "")

// Class with properties: first in the class body; leaves private: (the default at a class start).
#define SPARSE_FIELDS_WITH_PROPS_AND_SLOTS(LIST, SLOTS)                                            \
public:                                                                                            \
    SSF_SCHEMA_TYPES(LIST);                                                                        \
    using SparseFieldStorage =                                                                     \
        ::OHOS::Ace::StableSparseFields<SparseFieldSchema, SLOTS, (SparseFieldIds::kCount <= 64)>; \
    LIST(SSF_FIELD_ACCESSORS, SSF_NO_ACCESSORS)                                                    \
                                                                                                   \
private:                                                                                           \
    SparseFieldStorage sparseFields_

#define SPARSE_FIELDS_WITH_PROPS(LIST) SPARSE_FIELDS_WITH_PROPS_AND_SLOTS(LIST, 8)

// Property group struct: first in the struct body; the storage is copied with the struct; leaves public:.
#define SPARSE_FIELDS_GROUP(LIST)                                                                      \
public:                                                                                                \
    SSF_SCHEMA_TYPES(LIST);                                                                            \
    using SparseFieldStorage =                                                                         \
        ::OHOS::Ace::CopyableStableSparseFields<SparseFieldSchema, 8, (SparseFieldIds::kCount <= 64)>; \
    LIST(SSF_FIELD_ACCESSORS, SSF_NO_ACCESSORS)                                                        \
                                                                                                       \
private:                                                                                               \
    SparseFieldStorage sparseFields_;                                                                  \
                                                                                                       \
public:                                                                                                \
    static_assert(true, "")

// ---------------------------------------------------------------------------------------------------------
// Properties of a class (were ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP*, property.h)
// ---------------------------------------------------------------------------------------------------------

// The field name is an enumerator of SparseFieldIds used right after the scope operator, where it cannot
// be enclosed in parentheses; the SSF_ID step keeps a bare macro argument away from that position.
#define SSF_ID(name) name
#define SSF_PROP_ID(name) SparseFieldIds::SSF_ID(name)
#define SSF_PROP_CHECK_TYPE(name, type)                                             \
    static_assert(std::is_same_v<SparseFieldSchema::Type<SSF_PROP_ID(name)>, type>, \
        "P(" #name ", ...) in the sparse list has a different type than the property macro")

// Compare-and-write shared by all Update variants: returns `ret` if the value is NearEqual to the stored
// one, otherwise writes it (in place, or creates the field). Same order and NearEqual lookup as before.
#define SSF_PROP_WRITE_OR_RETURN(name, value, ret)                                   \
    if (auto* ssfCurrent = sparseFields_.template Get<SSF_PROP_ID(name)>()) {        \
        if (NearEqual(*ssfCurrent, value)) {                                         \
            return ret;                                                              \
        }                                                                            \
        *ssfCurrent = value;                                                         \
    } else {                                                                         \
        sparseFields_.template Emplace<SSF_PROP_ID(name)>(value);                    \
    }

// Getters, Clone, Reset and raw Assign — was ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP_GET; leaves protected:.
// The views of properties and groups are returned as const: `auto& x = GetX();` binds to them, as it did to
// the former const optional& and const unique_ptr&.
#define ACE_DEFINE_SPARSE_PROPERTY_ITEM_GET(name, type)                                                       \
public:                                                                                                       \
    const ::OHOS::Ace::SparsePropView<SparseFieldStorage, SSF_PROP_ID(name)> Get##name() const                \
    {                                                                                                         \
        return ::OHOS::Ace::SparsePropView<SparseFieldStorage, SSF_PROP_ID(name)>(&sparseFields_);            \
    }                                                                                                         \
    bool Has##name() const                                                                                    \
    {                                                                                                         \
        SSF_PROP_CHECK_TYPE(name, type);                                                                      \
        return sparseFields_.template Has<SSF_PROP_ID(name)>();                                               \
    }                                                                                                         \
    const type& Get##name##Value() const                                                                      \
    {                                                                                                         \
        return Get##name().value();                                                                           \
    }                                                                                                         \
    const type& Get##name##Value(const type& defaultValue) const                                              \
    {                                                                                                         \
        const type* ssfValue = sparseFields_.template Get<SSF_PROP_ID(name)>();                               \
        return ssfValue != nullptr ? *ssfValue : defaultValue;                                                \
    }                                                                                                         \
    std::optional<type> Clone##name() const                                                                   \
    {                                                                                                         \
        return Get##name().ToOptional();                                                                      \
    }                                                                                                         \
    void Reset##name()                                                                                        \
    {                                                                                                         \
        sparseFields_.template Reset<SSF_PROP_ID(name)>();                                                    \
    }                                                                                                         \
    void Assign##name(const type& value)                                                                      \
    {                                                                                                         \
        ::OHOS::Ace::SparseFieldsDetail::AssignProp<SSF_PROP_ID(name)>(sparseFields_, value);                 \
    }                                                                                                         \
    void Assign##name(const std::optional<type>& value)                                                       \
    {                                                                                                         \
        if (value) {                                                                                          \
            Assign##name(*value);                                                                             \
        } else {                                                                                              \
            Reset##name();                                                                                    \
        }                                                                                                     \
    }                                                                                                         \
    void Assign##name(std::optional<type>&& value)                                                            \
    {                                                                                                         \
        if (value) {                                                                                          \
            ::OHOS::Ace::SparseFieldsDetail::AssignProp<SSF_PROP_ID(name)>(sparseFields_, std::move(*value)); \
        } else {                                                                                              \
            Reset##name();                                                                                    \
        }                                                                                                     \
    }                                                                                                         \
                                                                                                              \
protected:                                                                                                    \
    static_assert(true, "")

// Was ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP: NearEqual -> write -> change flag.
#define ACE_DEFINE_SPARSE_PROPERTY_ITEM(name, type, changeFlag) \
    ACE_DEFINE_SPARSE_PROPERTY_ITEM_GET(name, type);            \
public:                                                         \
    void Update##name(const type& value)                        \
    {                                                           \
        SSF_PROP_WRITE_OR_RETURN(name, value, )                 \
        UpdatePropertyChangeFlag(changeFlag);                   \
    }

// Was ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP_AND_USING_CALLBACK: ... -> change flag -> On##name##Update.
#define ACE_DEFINE_SPARSE_PROPERTY_ITEM_AND_USING_CALLBACK(name, type, changeFlag) \
    ACE_DEFINE_SPARSE_PROPERTY_ITEM_GET(name, type);                               \
public:                                                                            \
    void Update##name(const type& value)                                           \
    {                                                                              \
        SSF_PROP_WRITE_OR_RETURN(name, value, )                                    \
        UpdatePropertyChangeFlag(changeFlag);                                      \
        On##name##Update(value);                                                   \
    }

// Was ACE_DEFINE_PROPERTY_ITEM_FUNC_WITHOUT_GROUP: NearEqual -> write -> On##name##Update, no flag.
#define ACE_DEFINE_SPARSE_PROPERTY_ITEM_FUNC(name, type) \
    ACE_DEFINE_SPARSE_PROPERTY_ITEM_GET(name, type);     \
public:                                                  \
    void Update##name(const type& value)                 \
    {                                                    \
        SSF_PROP_WRITE_OR_RETURN(name, value, )          \
        On##name##Update(value);                         \
    }

// Was ACE_DEFINE_PROPERTY_ITEM_FUNC_WITHOUT_GROUP_FOR_VIRTUAL_NODE.
#define ACE_DEFINE_SPARSE_PROPERTY_ITEM_FUNC_FOR_VIRTUAL_NODE(name, type)                        \
    ACE_DEFINE_SPARSE_PROPERTY_ITEM_GET(name, type);                                             \
public:                                                                                          \
    void Update##name(const type& value, const int64_t accessibilityIdForVirtualNode = -2100000) \
    {                                                                                            \
        SSF_PROP_WRITE_OR_RETURN(name, value, )                                                  \
        On##name##Update(value, accessibilityIdForVirtualNode);                                  \
    }

// Was ACE_DEFINE_PROPERTY_GROUP_ITEM_WITH_CALLBACK (despite the name — a class property, not a group item):
// NearEqual -> On##name##Update BEFORE the write -> write. The callback sees the old value; the field is
// looked up again after it (the callback may have reset or set it).
#define ACE_DEFINE_SPARSE_PROPERTY_GROUP_ITEM_WITH_CALLBACK(name, type)                              \
    ACE_DEFINE_SPARSE_PROPERTY_ITEM_GET(name, type);                                                 \
public:                                                                                              \
    bool Update##name(const type& value)                                                             \
    {                                                                                                \
        if (const type* ssfCurrent = sparseFields_.template Get<SSF_PROP_ID(name)>()) {              \
            if (NearEqual(*ssfCurrent, value)) {                                                     \
                return false;                                                                        \
            }                                                                                        \
        }                                                                                            \
        On##name##Update(value);                                                                     \
        ::OHOS::Ace::SparseFieldsDetail::AssignProp<SSF_PROP_ID(name)>(sparseFields_, value);        \
        return true;                                                                                 \
    }

// ---------------------------------------------------------------------------------------------------------
// Property groups
// ---------------------------------------------------------------------------------------------------------

// In the owner: the group is a P entry of the owner's list, the struct lives in the owner's storage.
// Was ACE_DEFINE_PROPERTY_GROUP; leaves protected:. The ..._WITH_GROUP* item macros of property.h work
// with it unchanged (`auto& groupProperty = Get##group();` binds to the const view).
#define ACE_DEFINE_SPARSE_PROPERTY_GROUP(group, type)                                                          \
public:                                                                                                        \
    const ::OHOS::Ace::SparseGroupView<SparseFieldStorage, SSF_PROP_ID(group)> GetOrCreate##group()            \
    {                                                                                                          \
        if (!sparseFields_.template Has<SSF_PROP_ID(group)>()) {                                               \
            sparseFields_.template Emplace<SSF_PROP_ID(group)>();                                              \
        }                                                                                                      \
        return ::OHOS::Ace::SparseGroupView<SparseFieldStorage, SSF_PROP_ID(group)>(&sparseFields_);           \
    }                                                                                                          \
    const ::OHOS::Ace::SparseGroupView<SparseFieldStorage, SSF_PROP_ID(group)> Get##group() const              \
    {                                                                                                          \
        SSF_PROP_CHECK_TYPE(group, type);                                                                      \
        return ::OHOS::Ace::SparseGroupView<SparseFieldStorage, SSF_PROP_ID(group)>(&sparseFields_);           \
    }                                                                                                          \
    std::unique_ptr<type> Clone##group() const                                                                 \
    {                                                                                                          \
        if (const type* ssfGroup = sparseFields_.template Get<SSF_PROP_ID(group)>()) {                         \
            return std::make_unique<type>(*ssfGroup);                                                          \
        }                                                                                                      \
        return nullptr;                                                                                        \
    }                                                                                                          \
    void Reset##group()                                                                                        \
    {                                                                                                          \
        sparseFields_.template Reset<SSF_PROP_ID(group)>();                                                    \
    }                                                                                                          \
    /* was `value->prop##group##_ = Clone##group();` */                                                        \
    void Assign##group(std::unique_ptr<type> value)                                                            \
    {                                                                                                          \
        if (value) {                                                                                           \
            ::OHOS::Ace::SparseFieldsDetail::AssignProp<SSF_PROP_ID(group)>(sparseFields_, std::move(*value)); \
        } else {                                                                                               \
            Reset##group();                                                                                    \
        }                                                                                                      \
    }                                                                                                          \
                                                                                                               \
protected:                                                                                                     \
    static_assert(true, "")

// In the group struct (after SPARSE_FIELDS_GROUP): was ACE_DEFINE_PROPERTY_GROUP_ITEM, no public prop##name.
#define ACE_DEFINE_SPARSE_PROPERTY_GROUP_ITEM(name, type)                                          \
    const ::OHOS::Ace::SparsePropView<SparseFieldStorage, SSF_PROP_ID(name)> Get##name() const     \
    {                                                                                              \
        return ::OHOS::Ace::SparsePropView<SparseFieldStorage, SSF_PROP_ID(name)>(&sparseFields_); \
    }                                                                                              \
    bool Has##name() const                                                                         \
    {                                                                                              \
        SSF_PROP_CHECK_TYPE(name, type);                                                           \
        return sparseFields_.template Has<SSF_PROP_ID(name)>();                                    \
    }                                                                                              \
    type Get##name##Value() const                                                                  \
    {                                                                                              \
        return Get##name().value();                                                                \
    }                                                                                              \
    bool Update##name(const type& value)                                                           \
    {                                                                                              \
        SSF_PROP_WRITE_OR_RETURN(name, value, false)                                               \
        return true;                                                                               \
    }                                                                                              \
    bool Update##name(const std::optional<type>& value)                                            \
    {                                                                                              \
        if (value.has_value()) {                                                                   \
            return Update##name(value.value());                                                    \
        }                                                                                          \
        return false;                                                                              \
    }                                                                                              \
    bool Check##name(const type& value) const                                                      \
    {                                                                                              \
        const type* ssfCurrent = sparseFields_.template Get<SSF_PROP_ID(name)>();                  \
        return ssfCurrent != nullptr && NearEqual(*ssfCurrent, value);                             \
    }                                                                                              \
    void Reset##name()                                                                             \
    {                                                                                              \
        sparseFields_.template Reset<SSF_PROP_ID(name)>();                                         \
    }                                                                                              \
    void Assign##name(const type& value)                                                           \
    {                                                                                              \
        ::OHOS::Ace::SparseFieldsDetail::AssignProp<SSF_PROP_ID(name)>(sparseFields_, value);      \
    }                                                                                              \
    void Assign##name(const std::optional<type>& value)                                            \
    {                                                                                              \
        if (value) {                                                                               \
            Assign##name(*value);                                                                  \
        } else {                                                                                   \
            Reset##name();                                                                         \
        }                                                                                          \
    }

#endif // FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_SPARSE_PROPERTY_H
