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

#include "accessor_test_base.h"

#include "core/interfaces/native/implementation/image_analyzer_controller_peer_impl.h"
#include "gmock/gmock.h"

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;

namespace {
const size_t EMPTY_SIZE = 0;
} // namespace
class ImageAnalyzerControllerAccessorTest
    : public AccessorTestBase<GENERATED_ArkUIImageAnalyzerControllerAccessor,
    &GENERATED_ArkUIAccessors::getImageAnalyzerControllerAccessor, ImageAnalyzerControllerPeer> {
public:
    void SetUp(void) override
    {
        AccessorTestBase::SetUp();
    }

    void TearDown() override
    {
        AccessorTestBase::TearDown();
    }
};

/**
 * @tc.name: getImageAnalyzerSupportTypesTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImageAnalyzerControllerAccessorTest, getImageAnalyzerSupportTypesTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->getImageAnalyzerSupportTypes, nullptr);
    auto result = accessor_->getImageAnalyzerSupportTypes(peer_);
    auto types = Converter::Convert<std::vector<ImageAnalyzerType>>(result);
    EXPECT_EQ(types.data(), nullptr);
    EXPECT_EQ(types.size(), EMPTY_SIZE);
}

} // namespace OHOS::Ace::NG
