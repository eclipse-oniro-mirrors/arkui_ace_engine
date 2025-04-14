/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

// #include "test/mock/core/pipeline/mock_pipeline_context.cpp"

namespace OHOS::Ace::NG {

using namespace testing;
using namespace testing::ext;

class FocusControllerAccessorTest : public StaticAccessorTest<GENERATED_ArkUIFocusControllerAccessor,
    &GENERATED_ArkUIAccessors::getFocusControllerAccessor> { };

/**
 * @tc.name: RequestFocusTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(FocusControllerAccessorTest, RequestFocusTest, TestSize.Level1)
{
    ASSERT_NE(accessor_->requestFocus, nullptr);
    const std::string expectedKey = "123";
    auto arkKey = Converter::ArkValue<Ark_String>(expectedKey);

    auto pipeLine = MockPipelineContext::GetCurrent();
    ASSERT_NE(pipeLine, nullptr);

    EXPECT_CALL(*pipeLine, RequestFocus(expectedKey, false)).Times(1);
    
    accessor_->requestFocus(&arkKey);
}
} // namespace OHOS::Ace::NG
