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

#include <functional>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>

#include "animate_impl.h"
#include "core/interfaces/arkoala/arkoala_api.h"
#include "dialog_model.h"
#include "error_code.h"
#include "event_converter.h"
#include "gtest/gtest.h"
#include "native_animate.h"
#include "native_interface.h"
#include "native_node.h"
#include "node_extened.h"
#include "node_model.h"
#include "native_dialog.h"
#include "interfaces/native/node/config_manager.h"
#include "test/mock/frameworks/base/log/mock_log_wrapper.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Ace::DialogModel;

class DialogModelTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
};

bool OnWillDismissEvent(int32_t reason)
{
    return true;
}

/**
 * @tc.name: DialogModelTest001
 * @tc.desc: Test Create function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest001, TestSize.Level1)
{
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_EQ(nativeDialogHandle, nullptr);

    Dispose(nativeDialogHandle);
}

/**
 * @tc.name: DialogModelTest002
 * @tc.desc: Test Create function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest002, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);

    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest003
 * @tc.desc: Test SetContent function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest003, TestSize.Level1)
{
    int32_t ret = SetContent(nullptr, nullptr);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest004
 * @tc.desc: Test SetContent function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest004, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    ArkUI_NodeHandle nodeHandle = new ArkUI_Node({ .magic = ARKUI_NODE_MAGIC_VALID });
    int32_t ret = SetContent(nativeDialogHandle, nodeHandle);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
    delete nodeHandle;
}

/**
 * @tc.name: DialogModelTest005
 * @tc.desc: Test RemoveContent function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest005, TestSize.Level1)
{
    int32_t ret = RemoveContent(nullptr);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest006
 * @tc.desc: Test RemoveContent function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest006, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = RemoveContent(nativeDialogHandle);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest007
 * @tc.desc: Test SetContentAlignment function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest007, TestSize.Level1)
{
    int32_t ret = SetContentAlignment(nullptr, 0, 0.0f, 0.0f);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest008
 * @tc.desc: Test SetContentAlignment function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest008, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetContentAlignment(nativeDialogHandle, 0, 0.0f, 0.0f);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest009
 * @tc.desc: Test ResetContentAlignment function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest009, TestSize.Level1)
{
    int32_t ret = ResetContentAlignment(nullptr);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest010
 * @tc.desc: Test SetContentAlignment function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest010, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = ResetContentAlignment(nativeDialogHandle);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest011
 * @tc.desc: Test SetModalMode function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest011, TestSize.Level1)
{
    int32_t ret = SetModalMode(nullptr, true);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest012
 * @tc.desc: Test SetModalMode function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest012, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetModalMode(nativeDialogHandle, true);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest013
 * @tc.desc: Test SetAutoCancel function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest013, TestSize.Level1)
{
    int32_t ret = SetAutoCancel(nullptr, true);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest014
 * @tc.desc: Test SetAutoCancel function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest014, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetAutoCancel(nativeDialogHandle, true);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest015
 * @tc.desc: Test SetMask function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest015, TestSize.Level1)
{
    int32_t ret = SetMask(nullptr, 0, nullptr);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest016
 * @tc.desc: Test SetMask function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest016, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    ArkUI_Rect rect = {0.0f, 0.0f, 0.0f, 0.0f};
    int32_t ret = SetMask(nativeDialogHandle, 0, &rect);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest017
 * @tc.desc: Test SetMask function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest017, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetMask(nativeDialogHandle, 0, nullptr);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest018
 * @tc.desc: Test SetBackgroundColor function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest018, TestSize.Level1)
{
    int32_t ret = SetBackgroundColor(nullptr, 0);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest019
 * @tc.desc: Test SetBackgroundColor function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest019, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetBackgroundColor(nativeDialogHandle, 0);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest020
 * @tc.desc: Test SetCornerRadius function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest020, TestSize.Level1)
{
    int32_t ret = SetCornerRadius(nullptr, 0.0f, 0.0f, 0.0f, 0.0f);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest021
 * @tc.desc: Test SetCornerRadius function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest021, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetCornerRadius(nativeDialogHandle, 0.0f, 0.0f, 0.0f, 0.0f);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest022
 * @tc.desc: Test SetGridColumnCount function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest022, TestSize.Level1)
{
    int32_t ret = SetGridColumnCount(nullptr, 0);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest023
 * @tc.desc: Test SetGridColumnCount function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest023, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetGridColumnCount(nativeDialogHandle, 0);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest024
 * @tc.desc: Test EnableCustomStyle function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest024, TestSize.Level1)
{
    int32_t ret = EnableCustomStyle(nullptr, true);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest025
 * @tc.desc: Test EnableCustomStyle function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest025, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = EnableCustomStyle(nativeDialogHandle, true);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest026
 * @tc.desc: Test EnableCustomAnimation function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest026, TestSize.Level1)
{
    int32_t ret = EnableCustomAnimation(nullptr, true);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest027
 * @tc.desc: Test EnableCustomAnimation function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest027, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = EnableCustomAnimation(nativeDialogHandle, true);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest028
 * @tc.desc: Test Show function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest028, TestSize.Level1)
{
    int32_t ret = Show(nullptr, true);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest029
 * @tc.desc: Test Show function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest029, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = Show(nativeDialogHandle, false);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest030
 * @tc.desc: Test Close function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest030, TestSize.Level1)
{
    int32_t ret = Close(nullptr);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest031
 * @tc.desc: Test Close function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest031, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = Close(nativeDialogHandle);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest032
 * @tc.desc: Test RegisterOnWillDismiss function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest032, TestSize.Level1)
{
    int32_t ret = RegisterOnWillDismiss(nullptr, nullptr);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest033
 * @tc.desc: Test RegisterOnWillDismiss function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest033, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    ArkUI_OnWillDismissEvent eventHandler = OnWillDismissEvent;
    int32_t ret = RegisterOnWillDismiss(nativeDialogHandle, eventHandler);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest034
 * @tc.desc: Test RegisterOnWillDismissWithUserData function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest034, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    ArkUI_DialogDismissEvent nEvent;
    int32_t ret =
        RegisterOnWillDismissWithUserData(nativeDialogHandle, nEvent.userData, [](ArkUI_DialogDismissEvent* event) {});
    ASSERT_EQ(ret, 0);
    ret = RegisterOnWillDismissWithUserData(nullptr, nullptr, nullptr);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest035
 * @tc.desc: Test ArkUI_DialogDismissEvent_SetShouldBlockDismiss function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest035, TestSize.Level1)
{
    ArkUI_DialogDismissEvent* nEvent = new ArkUI_DialogDismissEvent({ nullptr, 0, false });
    OH_ArkUI_DialogDismissEvent_SetShouldBlockDismiss(nEvent, true);
    ASSERT_TRUE(nEvent->BlockDismiss);
    nEvent = nullptr;
    OH_ArkUI_DialogDismissEvent_SetShouldBlockDismiss(nEvent, true);
    ASSERT_TRUE(nEvent == nullptr);
}

/**
 * @tc.name: DialogModelTest036
 * @tc.desc: Test ArkUI_DialogDismissEvent_GetUserData function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest036, TestSize.Level1)
{
    struct ArkUICustomData {
        float id;
        int index;
    };
    auto customData = new ArkUICustomData { 12, 1 };
    ArkUI_DialogDismissEvent* nEvent = new ArkUI_DialogDismissEvent({ nullptr, 0, false });
    nEvent->userData = customData;
    void* ret = OH_ArkUI_DialogDismissEvent_GetUserData(nEvent);
    ASSERT_EQ(ret, nEvent->userData);
    nEvent = nullptr;
    ret = OH_ArkUI_DialogDismissEvent_GetUserData(nEvent);
    ASSERT_TRUE(ret == nullptr);
}

/**
 * @tc.name: DialogModelTest037
 * @tc.desc: Test ArkUI_DialogDismissEvent_GetUserData function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest037, TestSize.Level1)
{
    ArkUI_DialogDismissEvent* nEvent = new ArkUI_DialogDismissEvent({ nullptr, 0, false });
    int32_t ret = OH_ArkUI_DialogDismissEvent_GetDismissReason(nEvent);
    ASSERT_EQ(ret, nEvent->reason);
    nEvent = nullptr;
    ret = OH_ArkUI_DialogDismissEvent_GetDismissReason(nEvent);
    ASSERT_EQ(ret, -1);
}

/**
 * @tc.name: DialogModelTest038
 * @tc.desc: Test SetLevelMode function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest038, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetLevelMode(nativeDialogHandle, ARKUI_LEVEL_MODE_OVERLAY);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    ret = SetLevelMode(nativeDialogHandle, static_cast<ArkUI_LevelMode>(-1));
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest039
 * @tc.desc: Test SetLevelUniqueId function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest039, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetLevelUniqueId(nativeDialogHandle, 0);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    ret = SetLevelUniqueId(nativeDialogHandle, -1);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest040
 * @tc.desc: Test SetImmersiveMode function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest040, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetImmersiveMode(nativeDialogHandle, ARKUI_IMMERSIVE_MODE_DEFAULT);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    ret = SetImmersiveMode(nativeDialogHandle, static_cast<ArkUI_ImmersiveMode>(-1));
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest041
 * @tc.desc: Test SetLevelOrder function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest041, TestSize.Level1)
{
    int32_t ret = SetLevelOrder(nullptr, 0.0f);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest042
 * @tc.desc: Test SetLevelOrder function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest042, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetLevelOrder(nativeDialogHandle, 0.0f);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest043
 * @tc.desc: Test SetFocusable function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest043, TestSize.Level1)
{
    int32_t ret = SetFocusable(nullptr, false);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: DialogModelTest044
 * @tc.desc: Test SetFocusable function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest044, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetFocusable(nativeDialogHandle, false);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest045
 * @tc.desc: Test SetBackgroundBlurStyleOptions function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest045, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    ArkUI_NumberValue value[] = { { .i32 = ARKUI_COLOR_MODE_LIGHT }, { .i32 = ARKUI_ADAPTIVE_COLOR_AVERAGE },
        { .f32 = 1.0f }, { .u32 = 20 }, { .u32 = 20 }, { .i32 = ARKUI_BLUR_STYLE_ACTIVE_POLICY_ALWAYS_ACTIVE },
        { .u32 = 0xFFFF0000 } };
    ArkUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
    int32_t ret = SetBackgroundBlurStyleOptions(nativeDialogHandle, &item);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest046
 * @tc.desc: Test SetBackgroundEffect function.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest046, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    ArkUI_NumberValue value[] = { { .f32 = 20.0f }, { .f32 = 1.0f }, { .f32 = 1.0f }, { .u32 = 0x66FFFFFF },
        { .i32 = ARKUI_ADAPTIVE_COLOR_AVERAGE }, { .u32 = 20 }, { .u32 = 20 },
        { .i32 = ARKUI_BLUR_STYLE_ACTIVE_POLICY_ALWAYS_ACTIVE }, { .u32 = 0xFFFF0000 } };
    ArkUI_AttributeItem item = { value, sizeof(value) / sizeof(ArkUI_NumberValue) };
    int32_t ret = SetBackgroundEffect(nativeDialogHandle, &item);
    ASSERT_EQ(ret, OHOS::Ace::ERROR_CODE_NO_ERROR);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest047
 * @tc.desc: Test SetContent error message when handle is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest047, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NodeHandle nodeHandle = new ArkUI_Node({ .magic = ARKUI_NODE_MAGIC_VALID });
    int32_t ret = SetContent(nullptr, nodeHandle);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("handle is null"), std::string::npos);
    delete nodeHandle;
}

/**
 * @tc.name: DialogModelTest048
 * @tc.desc: Test SetContent error message when content is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest048, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetContent(nativeDialogHandle, nullptr);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("content is null"), std::string::npos);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest049
 * @tc.desc: Test SetCustomShadow error message when customShadow is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest049, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetCustomShadow(nativeDialogHandle, nullptr);
    EXPECT_EQ(ret, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("customShadow is null"), std::string::npos);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest050
 * @tc.desc: Test SetCustomShadow error message when customShadow size is 0.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest050, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    ArkUI_AttributeItem item = { nullptr, 0 };
    int32_t ret = SetCustomShadow(nativeDialogHandle, &item);
    EXPECT_EQ(ret, ARKUI_ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("customShadow size is 0"), std::string::npos);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest051
 * @tc.desc: Test SetBackgroundBlurStyleOptions error message when parameter is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest051, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetBackgroundBlurStyleOptions(nativeDialogHandle, nullptr);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("backgroundBlurStyleOptions is null"), std::string::npos);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest052
 * @tc.desc: Test SetBackgroundEffect error message when parameter is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest052, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetBackgroundEffect(nativeDialogHandle, nullptr);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("backgroundEffect is null"), std::string::npos);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest053
 * @tc.desc: Test OH_ArkUI_DialogDismissEvent_SetShouldBlockDismiss error message when event is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest053, TestSize.Level1)
{
    OH_ArkUI_DialogDismissEvent_SetShouldBlockDismiss(nullptr, true);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("event is null"), std::string::npos);
}

/**
 * @tc.name: DialogModelTest054
 * @tc.desc: Test OH_ArkUI_DialogDismissEvent_GetUserData error message when event is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest054, TestSize.Level1)
{
    void* ret = OH_ArkUI_DialogDismissEvent_GetUserData(nullptr);
    EXPECT_EQ(ret, nullptr);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("event is null"), std::string::npos);
}

/**
 * @tc.name: DialogModelTest055
 * @tc.desc: Test OH_ArkUI_DialogDismissEvent_GetDismissReason error message when event is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest055, TestSize.Level1)
{
    int32_t ret = OH_ArkUI_DialogDismissEvent_GetDismissReason(nullptr);
    EXPECT_EQ(ret, -1);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("event is null"), std::string::npos);
}

/**
 * @tc.name: DialogModelTest056
 * @tc.desc: Test SetLevelMode error message when levelMode is invalid.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest056, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetLevelMode(nativeDialogHandle, static_cast<ArkUI_LevelMode>(-1));
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("levelMode is invalid"), std::string::npos);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest057
 * @tc.desc: Test SetImmersiveMode error message when immersiveMode is invalid.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest057, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetImmersiveMode(nativeDialogHandle, static_cast<ArkUI_ImmersiveMode>(-1));
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("immersiveMode is invalid"), std::string::npos);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest058
 * @tc.desc: Test SetLevelOrder error message when levelOrder is out of range.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest058, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetLevelOrder(nativeDialogHandle, -200000.0);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("levelOrder is out of range"), std::string::npos);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest059
 * @tc.desc: Test SetContent SET_ERROR_FUNCTION_NAME when backend returns error.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest059, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    ArkUI_NodeHandle nodeHandle = new ArkUI_Node({ .magic = ARKUI_NODE_MAGIC_VALID });
    int32_t ret = SetContent(nativeDialogHandle, nodeHandle);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("SetContent"), std::string::npos);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
    delete nodeHandle;
}

/**
 * @tc.name: DialogModelTest060
 * @tc.desc: Test Show SET_ERROR_FUNCTION_NAME when backend returns error.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest060, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = Show(nativeDialogHandle, false);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("Show"), std::string::npos);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest061
 * @tc.desc: Test Close SET_ERROR_FUNCTION_NAME when backend returns error.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest061, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = Close(nativeDialogHandle);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("Close"), std::string::npos);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest062
 * @tc.desc: Test SetShadow SET_ERROR_FUNCTION_NAME when backend returns error for invalid shadow.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest062, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetShadow(nativeDialogHandle, static_cast<ArkUI_ShadowStyle>(-1));
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("SetShadow"), std::string::npos);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest063
 * @tc.desc: Test SetContent SET_ERROR_FUNCTION_NAME when controller is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest063, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = new ArkUI_NativeDialog({ nullptr });
    ASSERT_NE(nativeDialogHandle, nullptr);
    ArkUI_NodeHandle nodeHandle = new ArkUI_Node({ .magic = ARKUI_NODE_MAGIC_VALID });
    int32_t ret = SetContent(nativeDialogHandle, nodeHandle);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("SetContent"), std::string::npos);
    delete nativeDialogHandle;
    nativeDialogHandle = nullptr;
    delete nodeHandle;
}

/**
 * @tc.name: DialogModelTest064
 * @tc.desc: Test RegisterOnWillDismissWithUserData SET_ERROR_FUNCTION_NAME when controller is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest064, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = new ArkUI_NativeDialog({ nullptr });
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = RegisterOnWillDismissWithUserData(nativeDialogHandle, nullptr, nullptr);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("RegisterOnWillDismissWithUserData"), std::string::npos);
    delete nativeDialogHandle;
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest065
 * @tc.desc: Test RemoveContent SET_ERROR_FUNCTION_NAME when controller is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest065, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = new ArkUI_NativeDialog({ nullptr });
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = RemoveContent(nativeDialogHandle);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("RemoveContent"), std::string::npos);
    delete nativeDialogHandle;
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest066
 * @tc.desc: Test SetModalMode SET_ERROR_FUNCTION_NAME when controller is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest066, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = new ArkUI_NativeDialog({ nullptr });
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetModalMode(nativeDialogHandle, true);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("SetModalMode"), std::string::npos);
    delete nativeDialogHandle;
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest067
 * @tc.desc: Test SetAutoCancel SET_ERROR_FUNCTION_NAME when controller is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest067, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = new ArkUI_NativeDialog({ nullptr });
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetAutoCancel(nativeDialogHandle, true);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("SetAutoCancel"), std::string::npos);
    delete nativeDialogHandle;
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest068
 * @tc.desc: Test SetBackgroundColor SET_ERROR_FUNCTION_NAME when controller is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest068, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = new ArkUI_NativeDialog({ nullptr });
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetBackgroundColor(nativeDialogHandle, 0);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("SetBackgroundColor"), std::string::npos);
    delete nativeDialogHandle;
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest069
 * @tc.desc: Test SetCornerRadius SET_ERROR_FUNCTION_NAME when controller is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest069, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = new ArkUI_NativeDialog({ nullptr });
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetCornerRadius(nativeDialogHandle, 1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("SetCornerRadius"), std::string::npos);
    delete nativeDialogHandle;
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest070
 * @tc.desc: Test SetGridColumnCount SET_ERROR_FUNCTION_NAME when controller is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest070, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = new ArkUI_NativeDialog({ nullptr });
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetGridColumnCount(nativeDialogHandle, 4);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("SetGridColumnCount"), std::string::npos);
    delete nativeDialogHandle;
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest071
 * @tc.desc: Test EnableCustomStyle SET_ERROR_FUNCTION_NAME when controller is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest071, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = new ArkUI_NativeDialog({ nullptr });
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = EnableCustomStyle(nativeDialogHandle, true);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("EnableCustomStyle"), std::string::npos);
    delete nativeDialogHandle;
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest072
 * @tc.desc: Test EnableCustomAnimation SET_ERROR_FUNCTION_NAME when controller is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest072, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = new ArkUI_NativeDialog({ nullptr });
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = EnableCustomAnimation(nativeDialogHandle, true);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("EnableCustomAnimation"), std::string::npos);
    delete nativeDialogHandle;
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest073
 * @tc.desc: Test RegisterOnWillDismiss SET_ERROR_FUNCTION_NAME when controller is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest073, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = new ArkUI_NativeDialog({ nullptr });
    ASSERT_NE(nativeDialogHandle, nullptr);
    ArkUI_OnWillDismissEvent eventHandler = OnWillDismissEvent;
    int32_t ret = RegisterOnWillDismiss(nativeDialogHandle, eventHandler);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("RegisterOnWillDismiss"), std::string::npos);
    delete nativeDialogHandle;
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest074
 * @tc.desc: Test SetFocusable SET_ERROR_FUNCTION_NAME when controller is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest074, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = new ArkUI_NativeDialog({ nullptr });
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetFocusable(nativeDialogHandle, true);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("SetFocusable"), std::string::npos);
    delete nativeDialogHandle;
    nativeDialogHandle = nullptr;
}

/**
 * @tc.name: DialogModelTest075
 * @tc.desc: Test SetBackgroundBlurStyle SET_ERROR_FUNCTION_NAME when controller is null.
 * @tc.type: FUNC
 */
HWTEST_F(DialogModelTest, DialogModelTest075, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    ArkUI_NativeDialogHandle nativeDialogHandle = new ArkUI_NativeDialog({ nullptr });
    ASSERT_NE(nativeDialogHandle, nullptr);
    int32_t ret = SetBackgroundBlurStyle(nativeDialogHandle, ARKUI_BLUR_STYLE_COMPONENT_THIN);
    EXPECT_EQ(ret, OHOS::Ace::ERROR_CODE_PARAM_INVALID);

    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(errorMessage, nullptr);
    std::string errorMessageStr(errorMessage);
    EXPECT_NE(errorMessageStr.find("SetBackgroundBlurStyle"), std::string::npos);
    delete nativeDialogHandle;
    nativeDialogHandle = nullptr;
}

namespace {
// Value of OH_ARKUI_NATIVEMODULE_CHECK_TYPE_NODE_DISPOSED, kept in sync with
// native_interface.h; the raw value mirrors the NativeXComponentUafTest setup.
constexpr int32_t DIALOG_UAF_CHECK_TYPE_NODE_DISPOSED = 1;

// Restore the process-wide disposed-node check to DISABLED so later tests in
// this binary are unaffected by the modes configured below.
void RestoreDialogUafCheckMode()
{
    EXPECT_TRUE(OHOS::Ace::NodeModel::ConfigManager::SetRuntimeCheckMode(DIALOG_UAF_CHECK_TYPE_NODE_DISPOSED,
        static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED)));
}

// Switches the process-wide disposed-node check to the given mode for the assertions
// that follow; the fixture TearDown restores DISABLED.
void EnableDialogUafCheckMode(int32_t mode)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::ConfigManager::SetRuntimeCheckMode(
        DIALOG_UAF_CHECK_TYPE_NODE_DISPOSED, mode));
}

// Asserts one captured diagnostic log carries the three stable fields: the check name
// NODE_DISPOSED, the given API name and the fixed reason text.
void AssertDiagnosticLog(const DiagnosticLog* diag, const char* apiName)
{
    ASSERT_NE(diag, nullptr);
    ASSERT_NE(diag->checkName, nullptr);
    ASSERT_NE(diag->apiName, nullptr);
    ASSERT_NE(diag->reason, nullptr);
    EXPECT_NE(std::string(diag->checkName).find("NODE_DISPOSED"), std::string::npos);
    EXPECT_NE(std::string(diag->apiName).find(apiName), std::string::npos);
    EXPECT_NE(std::string(diag->reason).find("Node has been disposed"), std::string::npos);
}

// The disposed-node check state is configured through ConfigManager::SetRuntimeCheckMode,
// whose thread guard calls isCurrentThreadSafe from the basic API table provided by the
// runtime image (libace_compatible.z.so of the device system; the test run does not ship
// its own). That callback only exists once the runtime ships the runtime-check mechanism;
// runtime images of the same struct revision but built before that merge fill the slot
// with unrelated small data and the call crashes (observed as a branch to a garbage
// address on an older image). Reading the slot value is safe (no call); a populated
// slot holds a text address.
bool RuntimeHasNodeDisposedCheck()
{
    const auto* full = OHOS::Ace::NodeModel::GetFullImpl();
    const auto* basic = (full != nullptr) ? full->getBasicAPI() : nullptr;
    if (basic == nullptr) {
        return false;
    }
    const uintptr_t callback = reinterpret_cast<uintptr_t>(basic->isCurrentThreadSafe);
    return callback > 0x1000;
}
} // namespace

class NativeDialogUafTest : public testing::Test {
public:
    NativeDialogUafTest() = default;
    ~NativeDialogUafTest() override = default;
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override
    {
        RestoreDialogUafCheckMode();
    }
};

/**
 * @tc.name: NativeDialogUafTest001
 * @tc.desc: A disposed-style content node hits the NODE_DISPOSED guard in LOG mode on both
 *           listed dialog APIs: the diagnostic log carries NODE_DISPOSED, the API name and
 *           the reason; the return values stay identical to DISABLED mode; the error
 *           message query channel stays free of disposed content.
 * @tc.type: FUNC
 */
HWTEST_F(NativeDialogUafTest, NativeDialogUafTest001, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    if (!RuntimeHasNodeDisposedCheck()) {
        GTEST_SKIP() << "runtime libace_compatible predates the runtime-check mechanism "
                        "(no OH_ArkUI_NativeModule_SetRuntimeCheckMode); run on a version-matched runtime";
    }
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    // Stack-controlled sample: the magic field drives the disposed-state check while
    // uiNodeHandle stays null, so the inner dialog implementation only sees its
    // existing null-content contract (same shape as DialogModelTest004).
    ArkUI_Node disposedNode;
    disposedNode.uiNodeHandle = nullptr;
    disposedNode.magic = ARKUI_NODE_MAGIC_INVALID;

    EnableDialogUafCheckMode(static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED));
    ResetDiagnosticLog();
    const int32_t setContentDisabled = SetContent(nativeDialogHandle, &disposedNode);
    ArkUI_CustomDialogOptions* createOptionsDisabled = OH_ArkUI_CustomDialog_CreateOptions(&disposedNode);
    EXPECT_EQ(GetDiagnosticLogCount(), 0);

    EnableDialogUafCheckMode(static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG));
    ResetDiagnosticLog();
    const int32_t setContentLog = SetContent(nativeDialogHandle, &disposedNode);
    ASSERT_EQ(GetDiagnosticLogCount(), 1);
    AssertDiagnosticLog(GetLastDiagnosticLog(), "SetContent");

    ArkUI_CustomDialogOptions* createOptionsLog = OH_ArkUI_CustomDialog_CreateOptions(&disposedNode);
    ASSERT_EQ(GetDiagnosticLogCount(), 2);
    AssertDiagnosticLog(GetLastDiagnosticLog(), "OH_ArkUI_CustomDialog_CreateOptions");

    // LOG only diagnoses and never short-circuits: both APIs keep the DISABLED
    // mode return values.
    EXPECT_EQ(setContentLog, setContentDisabled);
    EXPECT_EQ(createOptionsLog != nullptr, createOptionsDisabled != nullptr);
    // The error message query channel does not carry disposed content from this check.
    const char* errorMessage = OH_ArkUI_NativeModule_GetErrorMessage();
    if (errorMessage != nullptr) {
        EXPECT_EQ(std::string(errorMessage).find("has been disposed"), std::string::npos);
    }
    OH_ArkUI_CustomDialog_DisposeOptions(createOptionsDisabled);
    OH_ArkUI_CustomDialog_DisposeOptions(createOptionsLog);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
    RestoreDialogUafCheckMode();
}

/**
 * @tc.name: NativeDialogUafTest002
 * @tc.desc: Valid magic never triggers the guard: LOG/DISABLED/CRASH modes keep identical
 *           return values on both listed APIs for a valid handle and CRASH does not
 *           terminate the process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeDialogUafTest, NativeDialogUafTest002, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    if (!RuntimeHasNodeDisposedCheck()) {
        GTEST_SKIP() << "runtime libace_compatible predates the runtime-check mechanism "
                        "(no OH_ArkUI_NativeModule_SetRuntimeCheckMode); run on a version-matched runtime";
    }
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    ArkUI_Node validNode;
    validNode.uiNodeHandle = nullptr;
    validNode.magic = ARKUI_NODE_MAGIC_VALID;

    EnableDialogUafCheckMode(static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG));
    ResetDiagnosticLog();
    const int32_t setContentLog = SetContent(nativeDialogHandle, &validNode);
    ArkUI_CustomDialogOptions* createOptionsLog = OH_ArkUI_CustomDialog_CreateOptions(&validNode);
    EXPECT_EQ(GetDiagnosticLogCount(), 0);

    EnableDialogUafCheckMode(static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_DISABLED));
    const int32_t setContentDisabled = SetContent(nativeDialogHandle, &validNode);
    ArkUI_CustomDialogOptions* createOptionsDisabled = OH_ArkUI_CustomDialog_CreateOptions(&validNode);

    // A valid handle does not trigger the check in CRASH mode either (no termination).
    EnableDialogUafCheckMode(static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_CRASH));
    const int32_t setContentCrash = SetContent(nativeDialogHandle, &validNode);
    ArkUI_CustomDialogOptions* createOptionsCrash = OH_ArkUI_CustomDialog_CreateOptions(&validNode);
    EXPECT_EQ(GetDiagnosticLogCount(), 0);

    EXPECT_EQ(setContentDisabled, setContentLog);
    EXPECT_EQ(setContentCrash, setContentLog);
    EXPECT_EQ(createOptionsDisabled != nullptr, createOptionsLog != nullptr);
    EXPECT_EQ(createOptionsCrash != nullptr, createOptionsLog != nullptr);
    OH_ArkUI_CustomDialog_DisposeOptions(createOptionsLog);
    OH_ArkUI_CustomDialog_DisposeOptions(createOptionsDisabled);
    OH_ArkUI_CustomDialog_DisposeOptions(createOptionsCrash);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
    RestoreDialogUafCheckMode();
}

/**
 * @tc.name: NativeDialogUafTest003
 * @tc.desc: Null inputs keep the existing parameter-error contracts on both listed APIs;
 *           the entry guard is null-safe and stays silent.
 * @tc.type: FUNC
 */
HWTEST_F(NativeDialogUafTest, NativeDialogUafTest003, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    if (!RuntimeHasNodeDisposedCheck()) {
        GTEST_SKIP() << "runtime libace_compatible predates the runtime-check mechanism "
                        "(no OH_ArkUI_NativeModule_SetRuntimeCheckMode); run on a version-matched runtime";
    }
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    EnableDialogUafCheckMode(static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_LOG));
    ResetDiagnosticLog();

    EXPECT_EQ(SetContent(nativeDialogHandle, nullptr), OHOS::Ace::ERROR_CODE_PARAM_INVALID);
    const char* setContentError = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(setContentError, nullptr);
    EXPECT_NE(std::string(setContentError).find("content is null"), std::string::npos);

    EXPECT_EQ(OH_ArkUI_CustomDialog_CreateOptions(nullptr), nullptr);
    const char* createOptionsError = OH_ArkUI_NativeModule_GetErrorMessage();
    ASSERT_NE(createOptionsError, nullptr);
    EXPECT_NE(std::string(createOptionsError).find("content is null"), std::string::npos);

    EXPECT_EQ(GetDiagnosticLogCount(), 0);
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
    RestoreDialogUafCheckMode();
}

/**
 * @tc.name: NativeDialogUafTest004
 * @tc.desc: CRASH mode diagnoses and aborts on a disposed-style content node before the
 *           content pointer is read, on both listed APIs. Uses manual fork instead of
 *           EXPECT_DEATH to avoid GoogleTest's CapturedStream which requires /tmp.
 * @tc.type: FUNC
 */
HWTEST_F(NativeDialogUafTest, NativeDialogUafTest004, TestSize.Level1)
{
    ASSERT_TRUE(OHOS::Ace::NodeModel::InitialFullImpl());
    if (!RuntimeHasNodeDisposedCheck()) {
        GTEST_SKIP() << "runtime libace_compatible predates the runtime-check mechanism "
                        "(no OH_ArkUI_NativeModule_SetRuntimeCheckMode); run on a version-matched runtime";
    }
    ArkUI_NativeDialogHandle nativeDialogHandle = Create();
    ASSERT_NE(nativeDialogHandle, nullptr);
    ArkUI_Node disposedNode;
    disposedNode.uiNodeHandle = nullptr;
    disposedNode.magic = ARKUI_NODE_MAGIC_INVALID;

    EnableDialogUafCheckMode(static_cast<int32_t>(OH_ARKUI_NATIVEMODULE_CHECK_MODE_CRASH));

    // Manually fork to verify CRASH mode aborts; EXPECT_DEATH relies on GoogleTest's
    // CapturedStream which creates temp files in /tmp, a directory that may not exist
    // on device environments (e.g. rk3568), causing the test process itself to abort.
    auto expectAbort = [](const std::function<void()>& fn) {
        fflush(stdout);
        fflush(stderr);
        pid_t pid = fork();
        if (pid < 0) {
            ADD_FAILURE() << "fork() failed";
            return;
        }
        if (pid == 0) {
            fn();
            _exit(0); // Should never reach here if CRASH mode works correctly
        }
        int status = 0;
        if (waitpid(pid, &status, 0) != pid) {
            ADD_FAILURE() << "waitpid() failed";
            return;
        }
        EXPECT_TRUE(WIFSIGNALED(status)) << "expected signal termination, got exit code: " << WEXITSTATUS(status);
        EXPECT_EQ(WTERMSIG(status), SIGABRT) << "expected SIGABRT, got signal: " << WTERMSIG(status);
    };

    expectAbort([&]() { SetContent(nativeDialogHandle, &disposedNode); });
    expectAbort([&]() { OH_ArkUI_CustomDialog_CreateOptions(&disposedNode); });

    RestoreDialogUafCheckMode();
    Dispose(nativeDialogHandle);
    nativeDialogHandle = nullptr;
}
