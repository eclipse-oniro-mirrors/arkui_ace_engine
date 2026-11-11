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

#include "form_mgr.h"

#include "core/pipeline/pipeline_base.h"

#define private public
#define protected public
#include "core/components/form/resource/form_manager_delegate.h"
#include "core/components/form/resource/form_callback_client.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
namespace {
constexpr int64_t CARD_ID = 10000;
} // namespace

class FormManagerDelegateTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: OnDeathReceivedWithoutPipelineContext001
 * @tc.desc: OnDeathReceived stops the recovery chain cleanly when the pipeline context is gone.
 * @tc.type: FUNC
 */
HWTEST_F(FormManagerDelegateTest, OnDeathReceivedWithoutPipelineContext001, TestSize.Level1)
{
    auto delegate = AceType::MakeRefPtr<FormManagerDelegate>(WeakPtr<PipelineBase>());
    delegate->runningCardId_ = CARD_ID;

    delegate->OnDeathReceived();

    EXPECT_FALSE(delegate->deathRecoveryRunning_);
}

/**
 * @tc.name: OnDeathReceivedAlreadyRunning002
 * @tc.desc: OnDeathReceived skips when a recovery chain is already running for this delegate.
 * @tc.type: FUNC
 */
HWTEST_F(FormManagerDelegateTest, OnDeathReceivedAlreadyRunning002, TestSize.Level1)
{
    auto delegate = AceType::MakeRefPtr<FormManagerDelegate>(WeakPtr<PipelineBase>());
    delegate->runningCardId_ = CARD_ID;
    delegate->deathRecoveryRunning_ = true;

    delegate->OnDeathReceived();

    EXPECT_TRUE(delegate->deathRecoveryRunning_);
}

/**
 * @tc.name: ExecuteDeathRecoveryFormReleased003
 * @tc.desc: ExecuteDeathRecovery resets the running flag when the form is released during recovery.
 * @tc.type: FUNC
 */
HWTEST_F(FormManagerDelegateTest, ExecuteDeathRecoveryFormReleased003, TestSize.Level1)
{
    auto delegate = AceType::MakeRefPtr<FormManagerDelegate>(WeakPtr<PipelineBase>());
    delegate->runningCardId_ = -1;
    delegate->deathRecoveryRunning_ = true;

    delegate->ExecuteDeathRecovery(0);

    EXPECT_FALSE(delegate->deathRecoveryRunning_);
}

/**
 * @tc.name: TryReAddFormWithoutRenderDelegate004
 * @tc.desc: TryReAddForm skips the surface param instead of crashing when renderDelegate_ is null.
 * @tc.type: FUNC
 */
HWTEST_F(FormManagerDelegateTest, TryReAddFormWithoutRenderDelegate004, TestSize.Level1)
{
    auto delegate = AceType::MakeRefPtr<FormManagerDelegate>(WeakPtr<PipelineBase>());
    delegate->runningCardId_ = CARD_ID;

    auto ret = delegate->TryReAddForm();

    EXPECT_NE(ret, 0);
    EXPECT_FALSE(delegate->wantCache_.HasParameter("ohos.extra.param.key.process_on_add_surface"));
}

/**
 * @tc.name: DeathCallbackRegistrationLifecycle005
 * @tc.desc: ReleaseForm unregisters the death callback registered for FMS restart recovery.
 * @tc.type: FUNC
 */
HWTEST_F(FormManagerDelegateTest, DeathCallbackRegistrationLifecycle005, TestSize.Level1)
{
    auto& formMgr = AppExecFwk::FormMgr::GetInstance();
    auto client = std::make_shared<FormCallbackClient>();
    EXPECT_FALSE(formMgr.CheckIsDeathCallbackRegistered(client));

    auto delegate = AceType::MakeRefPtr<FormManagerDelegate>(WeakPtr<PipelineBase>());
    delegate->runningCardId_ = CARD_ID;
    delegate->formCallbackClient_ = client;

    formMgr.RegisterDeathCallback(client);
    EXPECT_TRUE(formMgr.CheckIsDeathCallbackRegistered(client));

    // runningCompId_ is empty so ReleaseForm skips the StopRenderingForm IPC call.
    delegate->ReleaseForm();
    EXPECT_FALSE(formMgr.CheckIsDeathCallbackRegistered(client));
}
} // namespace OHOS::Ace
