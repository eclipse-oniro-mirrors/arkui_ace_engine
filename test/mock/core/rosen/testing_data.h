/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MOCK_ROSEN_TEST_TESTING_DATA_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MOCK_ROSEN_TEST_TESTING_DATA_H

#include <cstddef>
#include <memory>
namespace OHOS::Ace::Testing {
class TestingData {
public:
    TestingData() = default;
    virtual ~TestingData() = default;

    virtual bool BuildWithCopy(const void* data, size_t length)
    {
        return length > 0 ? true : false;
    }

    virtual size_t GetSize() const
    {
        return 0;
    }

    virtual void* GetData() const
    {
        return nullptr;
    }

    static std::shared_ptr<TestingData> MakeFromFileName(const char path[])
    {
        return std::make_shared<TestingData>();
    }

    virtual bool BuildFromMalloc(const void* data, size_t length)
    {
        return true;
    }

    virtual bool BuildUninitialized(size_t length)
    {
        return true;
    }

    virtual void* WritableData()
    {
        return nullptr;
    }
};
} // namespace OHOS::Ace::Testing
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MOCK_ROSEN_TEST_TESTING_DATA_H
