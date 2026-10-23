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

#include "ecmascript/napi/include/jsnapi.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::Framework {
/**
 * @tc.name: ArkJSRuntimeExecuteModuleBufferTest001
 * @tc.desc: Verify JSNApi::ExecuteModuleBufferSecure accepts non-const uint8_t* and returns false for invalid buffer.
 *           This validates the signature contract that ArkJSRuntime::ExecuteModuleBuffer relies on after switching
 *           from ExecuteModuleBuffer(const uint8_t*) to ExecuteModuleBufferSecure(uint8_t*).
 * @tc.type: FUNC
 */
HWTEST(ArkJSRuntimeTest, ArkJSRuntimeExecuteModuleBufferTest001, TestSize.Level1)
{
    panda::RuntimeOption option;
    option.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
    const int64_t poolSize = 0x1000000; // 16M
    option.SetGcPoolSize(poolSize);
    option.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::ERROR);
    option.SetDebuggerLibraryPath("");
    panda::EcmaVM* vm = panda::JSNApi::CreateJSVM(option);
    ASSERT_NE(vm, nullptr);

    // ExecuteModuleBufferSecure requires non-const uint8_t* — verify a writable buffer is accepted.
    uint8_t invalidBuffer[] = { 0x00, 0x01, 0x02, 0x03 };
    int32_t size = static_cast<int32_t>(sizeof(invalidBuffer));
    std::string filename = "test_invalid.abc";

    bool ret = panda::JSNApi::ExecuteModuleBufferSecure(vm, invalidBuffer, size, filename, false, nullptr);
    // Invalid abc buffer should return false.
    EXPECT_FALSE(ret);

    panda::JSNApi::DestroyJSVM(vm);
}

/**
 * @tc.name: ArkJSRuntimeExecuteModuleBufferTest002
 * @tc.desc: Verify ExecuteModuleBufferSecure accepts zero-size buffer without crash.
 * @tc.type: FUNC
 */
HWTEST(ArkJSRuntimeTest, ArkJSRuntimeExecuteModuleBufferTest002, TestSize.Level1)
{
    panda::RuntimeOption option;
    option.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
    const int64_t poolSize = 0x1000000; // 16M
    option.SetGcPoolSize(poolSize);
    option.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::ERROR);
    option.SetDebuggerLibraryPath("");
    panda::EcmaVM* vm = panda::JSNApi::CreateJSVM(option);
    ASSERT_NE(vm, nullptr);

    uint8_t emptyBuffer[1] = { 0 };
    bool ret = panda::JSNApi::ExecuteModuleBufferSecure(vm, emptyBuffer, 0, "empty.abc", false, nullptr);
    EXPECT_FALSE(ret);

    panda::JSNApi::DestroyJSVM(vm);
}

/**
 * @tc.name: ArkJSRuntimeExecuteModuleBufferTest003
 * @tc.desc: Verify that a std::vector<uint8_t>::data() (non-const) is compatible with
 *           ExecuteModuleBufferSecure's uint8_t* parameter — this is the exact calling
 *           pattern used by ArkJSRuntime::ExecuteModuleBuffer callers in jsi_declarative_engine.
 * @tc.type: FUNC
 */
HWTEST(ArkJSRuntimeTest, ArkJSRuntimeExecuteModuleBufferTest003, TestSize.Level1)
{
    panda::RuntimeOption option;
    option.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
    const int64_t poolSize = 0x1000000; // 16M
    option.SetGcPoolSize(poolSize);
    option.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::ERROR);
    option.SetDebuggerLibraryPath("");
    panda::EcmaVM* vm = panda::JSNApi::CreateJSVM(option);
    ASSERT_NE(vm, nullptr);

    // Simulate the real calling pattern: std::vector<uint8_t> content; content.data()
    std::vector<uint8_t> content = { 0xDE, 0xAD, 0xBE, 0xEF };
    bool ret = panda::JSNApi::ExecuteModuleBufferSecure(
        vm, content.data(), static_cast<int32_t>(content.size()), "vector_test.abc", false, nullptr);
    EXPECT_FALSE(ret);

    panda::JSNApi::DestroyJSVM(vm);
}
} // namespace OHOS::Ace::Framework
