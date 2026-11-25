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

#include "gtest/gtest.h"

#include "frameworks/base/error/error_code.h"
#include "frameworks/core/components/xcomponent/native_interface_xcomponent_impl.h"
#include "interfaces/native/native_interface_xcomponent.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
namespace {
constexpr int32_t SUCCESS_RESULT = OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
} // namespace

class NativeInterfaceXComponentSupplementTestNg : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override
    {
        impl_ = new NativeXComponentImpl();
        component_ = new OH_NativeXComponent(impl_);
    }

    void TearDown() override
    {
        delete component_;
        component_ = nullptr;
        delete impl_;
        impl_ = nullptr;
    }

    NativeXComponentImpl* impl_ = nullptr;
    OH_NativeXComponent* component_ = nullptr;
};