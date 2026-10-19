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

#ifndef FOUNDATION_ACE_TEST_UNITTEST_BASE_SPARSE_REF_PTR_LATE_TYPE_H
#define FOUNDATION_ACE_TEST_UNITTEST_BASE_SPARSE_REF_PTR_LATE_TYPE_H

#include <cstdint>

#include "base/utils/sparse_fields.h"
#include "base/utils/sparse_ref_ptr.h"

// A class with a SparseRefPtr field of a type it only declares, as in a header. LateType is complete only in
// sparse_ref_ptr_late_type.cpp, which instantiates the holder as the .cpp of such a class does; sparse_fields_test.cpp
// uses the field where LateType stays incomplete.
namespace OHOS::Ace::SparseRefPtrTest {
class LateType;
} // namespace OHOS::Ace::SparseRefPtrTest

extern template struct OHOS::Ace::SparseRefPtr<OHOS::Ace::SparseRefPtrTest::LateType>;

namespace OHOS::Ace::SparseRefPtrTest {
#define LATE_HOLDER_SPARSE_FIELDS(F) F(Late, SparseRefPtr<LateType>)
class LateHolder {
public:
    SPARSE_FIELDS(LATE_HOLDER_SPARSE_FIELDS);
};

// What the destructors of LateType objects have done: the live count, and what the last one with an owner found in
// the field of its owner.
struct LateEvents {
    int32_t alive = 0;
    bool seenSet = false;
    const void* seenValue = nullptr;
};

// Defined where LateType is complete.
void SetNewLate(SparseRefPtr<LateType>& field, int32_t value, LateHolder* owner);
int32_t GetLateValue(const SparseRefPtr<LateType>& field);
const void* GetLateAddress(const SparseRefPtr<LateType>& field);
void KeepLate(const SparseRefPtr<LateType>& field);
void DropKeptLate();
const LateEvents& GetLateEvents();
void ResetLateEvents();
} // namespace OHOS::Ace::SparseRefPtrTest

#endif // FOUNDATION_ACE_TEST_UNITTEST_BASE_SPARSE_REF_PTR_LATE_TYPE_H
