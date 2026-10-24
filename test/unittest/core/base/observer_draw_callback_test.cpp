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

#include <thread>
#include <vector>

#include "gtest/gtest.h"

#include "core/common/container_scope.h"
#include "core/components_ng/base/observer_handler.h"

using namespace testing::ext;

namespace OHOS::Ace::NG {
class ObserverDrawCallbackTest : public testing::Test {};

/**
 * @tc.name: DrawAndLayoutCompleteBeforeReturning
 * @tc.desc: Pipeline notifications run on the caller thread before subsequent frame work.
 * @tc.type: FUNC
 */
HWTEST_F(ObserverDrawCallbackTest, DrawAndLayoutCompleteBeforeReturning, TestSize.Level1)
{
    UIObserverHandler handler;
    std::vector<int32_t> order;
    const auto callerThread = std::this_thread::get_id();
    handler.SetLayoutDoneHandleFunc([&order]() { order.emplace_back(1); });
    handler.SetDrawCommandSendHandleFunc([&order, callerThread]() {
        EXPECT_EQ(std::this_thread::get_id(), callerThread);
        order.emplace_back(3);
    });

    handler.HandleLayoutDoneCallBack();
    order.emplace_back(2);
    handler.HandleDrawCommandSendCallBack();
    order.emplace_back(4);

    EXPECT_EQ(order, (std::vector<int32_t> { 1, 2, 3, 4 }));
}

/**
 * @tc.name: DrawPreservesSourceInstanceAndHonorsUnregister
 * @tc.desc: Each synchronous notification sees its source scope; an empty handler does not notify.
 * @tc.type: FUNC
 */
HWTEST_F(ObserverDrawCallbackTest, DrawPreservesSourceInstanceAndHonorsUnregister, TestSize.Level1)
{
    UIObserverHandler handler;
    std::vector<int32_t> instances;
    constexpr int32_t firstInstance = 101;
    constexpr int32_t secondInstance = 202;
    ContainerScope firstScope(firstInstance);
    handler.SetDrawCommandSendHandleFunc([&instances]() {
        instances.emplace_back(ContainerScope::CurrentId());
    });

    handler.HandleDrawCommandSendCallBack();
    {
        ContainerScope secondScope(secondInstance);
        handler.HandleDrawCommandSendCallBack();
    }
    EXPECT_EQ(ContainerScope::CurrentId(), firstInstance);
    handler.SetDrawCommandSendHandleFunc(nullptr);
    handler.HandleDrawCommandSendCallBack();

    EXPECT_EQ(instances, (std::vector<int32_t> { firstInstance, secondInstance }));
}
} // namespace OHOS::Ace::NG
