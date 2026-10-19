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

#include "test/unittest/base/sparse_ref_ptr_late_type.h"

#include "base/memory/ace_type.h"

namespace OHOS::Ace::SparseRefPtrTest {
class LateType : public AceType {
    DECLARE_ACE_TYPE(LateType, AceType);

public:
    LateType(int32_t value, LateHolder* owner);
    ~LateType() override;

    int32_t GetValue() const
    {
        return value_;
    }

private:
    int32_t value_ = 0;
    LateHolder* owner_ = nullptr;
};

namespace {
LateEvents g_lateEvents;
RefPtr<LateType> g_keptLate;
} // namespace

LateType::LateType(int32_t value, LateHolder* owner) : value_(value), owner_(owner)
{
    ++g_lateEvents.alive;
}

LateType::~LateType()
{
    --g_lateEvents.alive;
    if (owner_ != nullptr) {
        const auto* field = owner_->GetLate();
        g_lateEvents.seenSet = field != nullptr;
        g_lateEvents.seenValue = field != nullptr ? AceType::RawPtr(field->ptr) : nullptr;
    }
}

void SetNewLate(SparseRefPtr<LateType>& field, int32_t value, LateHolder* owner)
{
    field.ptr = AceType::MakeRefPtr<LateType>(value, owner);
}

int32_t GetLateValue(const SparseRefPtr<LateType>& field)
{
    return field.ptr ? field.ptr->GetValue() : 0;
}

const void* GetLateAddress(const SparseRefPtr<LateType>& field)
{
    return AceType::RawPtr(field.ptr);
}

void KeepLate(const SparseRefPtr<LateType>& field)
{
    g_keptLate = field.ptr;
}

void DropKeptLate()
{
    g_keptLate = nullptr;
}

const LateEvents& GetLateEvents()
{
    return g_lateEvents;
}

void ResetLateEvents()
{
    g_lateEvents = LateEvents();
}
} // namespace OHOS::Ace::SparseRefPtrTest

// The holder of the field, built where LateType is complete, as in the .cpp of the class.
template struct OHOS::Ace::SparseRefPtr<OHOS::Ace::SparseRefPtrTest::LateType>;
