/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "gmock/gmock.h"

#include "accessor_test_base.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/utility/validators.h"

#include "core/interfaces/native/implementation/file_selector_result_peer_impl.h"

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;

namespace {
class MockFileSelectorResult : public FileSelectorResult {
public:
    MOCK_METHOD(void, HandleFileList, (std::vector<std::string>&));
};
} // namespace

class FileSelectorResultAccessorTest : public AccessorTestBase<GENERATED_ArkUIFileSelectorResultAccessor,
    &GENERATED_ArkUIAccessors::getFileSelectorResultAccessor, FileSelectorResultPeer> {
public:
    void SetUp() override
    {
        AccessorTestBase::SetUp();
        mockHandler_ = Referenced::MakeRefPtr<MockFileSelectorResult>();
        ASSERT_NE(mockHandler_, nullptr);
        peer_->handler = mockHandler_;
    }

    void TearDown() override
    {
        AccessorTestBase::TearDown();
        mockHandler_ = nullptr;
    }

    RefPtr<MockFileSelectorResult> mockHandler_ = nullptr;
};

/**
 * @tc.name: handleFileListTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(FileSelectorResultAccessorTest, handleFileListTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->handleFileList, nullptr);

    std::vector<std::string> vec{ "item1", "item2" };
    Converter::ArkArrayHolder<Array_String> vecHolder(vec);
    Array_String stringArrayResult = vecHolder.ArkValue();

    EXPECT_CALL(*mockHandler_, HandleFileList(vec)).Times(1);
    accessor_->handleFileList(peer_, &stringArrayResult);
}

} // namespace OHOS::Ace::NG
