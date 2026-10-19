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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_SPARSE_REF_PTR_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_SPARSE_REF_PTR_H

#include <utility>

#include "base/memory/referenced.h"

// A RefPtr value of a sparse field whose type is only declared (class Foo;) where the class with the field is
// declared.
//
// The sparse storage builds the destructors of its values together with the class (sparse_fields.h), so an entry
// F(Name, RefPtr<Foo>) needs Foo complete in every translation unit that sees the class. The special members of
// SparseRefPtr<Foo> are not inline: an explicit instantiation declaration next to the class keeps the other
// translation units from building them, and one .cpp where Foo is complete builds them:
//
//   header of the class:   extern template struct OHOS::Ace::SparseRefPtr<OHOS::Ace::NG::Foo>;   (after class Foo;)
//                          F(Name, SparseRefPtr<Foo>)                    (the pointer is GetName()->ptr)
//   its .cpp (Foo complete):
//                          template struct OHOS::Ace::SparseRefPtr<OHOS::Ace::NG::Foo>;
//
// Each type is instantiated in exactly one translation unit of a binary. The instantiation has the visibility of
// that .cpp (hidden in ace), so code in another shared library reaches such a field only through out-of-line
// functions of the class. The holder is movable, so the storage keeps the order of a RefPtr field: Reset and a new
// value release the old object after the field reads absent or holds the new one.
namespace OHOS::Ace {
template<typename T>
struct SparseRefPtr final {
    SparseRefPtr();
    ~SparseRefPtr();
    SparseRefPtr(const SparseRefPtr&) = delete;
    SparseRefPtr& operator=(const SparseRefPtr&) = delete;
    SparseRefPtr(SparseRefPtr&& other) noexcept;
    SparseRefPtr& operator=(SparseRefPtr&& other) noexcept;

    RefPtr<T> ptr;
};

template<typename T>
SparseRefPtr<T>::SparseRefPtr() = default;

template<typename T>
SparseRefPtr<T>::~SparseRefPtr() = default;

template<typename T>
SparseRefPtr<T>::SparseRefPtr(SparseRefPtr&& other) noexcept : ptr(std::move(other.ptr))
{}

template<typename T>
SparseRefPtr<T>& SparseRefPtr<T>::operator=(SparseRefPtr&& other) noexcept
{
    ptr = std::move(other.ptr);
    return *this;
}
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_BASE_UTILS_SPARSE_REF_PTR_H
