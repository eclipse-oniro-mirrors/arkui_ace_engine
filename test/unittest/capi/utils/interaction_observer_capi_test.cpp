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
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "securec.h"

#include "base/json/json_util.h"
#include "core/components_ng/manager/interaction_observer/ui_interaction_json_builder.h"
#include "core/interfaces/native/node/interaction_observer_api.h"
#include "core/interfaces/native/utility/error_message_manager.h"
#include "interfaces/native/native_error_message_wrapper.h"
#include "interfaces/native/native_interface.h"
#include "interfaces/native/node/node_model.h"
#include "interfaces/native/ui_info_collection.h"

namespace {
// Bridge availability and container validity are controlled by the test.
bool support = true;
bool instanceValid = true;
int32_t lastInstance = -1;
uint32_t lastMask = 0;

struct FakeObserver {
    uint32_t mask = 0;
    void (*callback)(const OH_ArkUI_NativeModule_UIJsonWrapper* json, void* userData) = nullptr;
    void* userData = nullptr;
    OH_ArkUI_NativeModule_UIJsonWrapper* (*createWrapper)(const char* data, uint32_t size) = nullptr;
    void (*destroyWrapper)(const OH_ArkUI_NativeModule_UIJsonWrapper* wrapper) = nullptr;
};
std::map<uint32_t, FakeObserver> fakeObservers;
std::map<int32_t, std::vector<uint32_t>> fakeByInstance;
uint32_t fakeNextId = 1;

constexpr uint32_t SUPPORTED_MASK = (1u << 9) - 1;

int32_t FakeRegister(int32_t instanceId, uint32_t eventMask,
    void (*callback)(const OH_ArkUI_NativeModule_UIJsonWrapper* json, void* userData), void* userData,
    OH_ArkUI_NativeModule_UIJsonWrapper* (*createWrapper)(const char* data, uint32_t size),
    void (*destroyWrapper)(const OH_ArkUI_NativeModule_UIJsonWrapper* wrapper), uint32_t* registerID)
{
    if (registerID) {
        *registerID = 0;
    }
    if (!registerID || !callback || !createWrapper || !destroyWrapper) {
        return 401;
    }
    if ((eventMask & SUPPORTED_MASK) == 0) {
        return 401;
    }
    if (!instanceValid) {
        return 190001;
    }
    FakeObserver observer;
    observer.mask = eventMask & SUPPORTED_MASK;
    observer.callback = callback;
    observer.userData = userData;
    observer.createWrapper = createWrapper;
    observer.destroyWrapper = destroyWrapper;
    const uint32_t id = fakeNextId++;
    fakeObservers[id] = observer;
    fakeByInstance[instanceId].push_back(id);
    lastInstance = instanceId;
    lastMask = eventMask;
    *registerID = id;
    return 0;
}

int32_t FakeUnregister(uint32_t registerID)
{
    if (registerID == 0) {
        return 401;
    }
    if (fakeObservers.find(registerID) == fakeObservers.end()) {
        return 401;
    }
    fakeObservers.erase(registerID);
    return 0;
}

// Simulates the framework-side dispatch: creates the wrapper through the
// trampolines supplied at registration, invokes the matching callbacks and
// destroys the wrapper right after the last callback returns.
void FakeFireEvent(int32_t instanceId, uint32_t eventBit, const std::string& json)
{
    auto it = fakeByInstance.find(instanceId);
    if (it == fakeByInstance.end()) {
        return;
    }
    for (uint32_t id : it->second) {
        auto observerIt = fakeObservers.find(id);
        if (observerIt == fakeObservers.end()) {
            continue;
        }
        const FakeObserver& observer = observerIt->second;
        if (!(observer.mask & eventBit) || !observer.callback) {
            continue;
        }
        OH_ArkUI_NativeModule_UIJsonWrapper* wrapper =
            observer.createWrapper(json.c_str(), static_cast<uint32_t>(json.size()));
        ASSERT_NE(wrapper, nullptr);
        observer.callback(wrapper, observer.userData);
        observer.destroyWrapper(wrapper);
    }
}

void SetCode(int32_t code, const char* reason)
{
    OHOS::Ace::ErrorMessageManager::GetInstance().SetErrorCodeAndMessage(code, reason);
}
void SetName(const char* name)
{
    OHOS::Ace::ErrorMessageManager::GetInstance().SetFunctionName(name);
}
const char* GetMessage()
{
    return OHOS::Ace::ErrorMessageManager::GetInstance().GetErrorMessage();
}
const ArkUIBasicAPI* Basic()
{
    static ArkUIBasicAPI api {};
    api.setErrorCodeAndMessage = SetCode;
    api.setErrorFunctionName = SetName;
    api.getErrorMessage = GetMessage;
    return &api;
}
} // namespace

namespace OHOS::Ace::NodeModel {
const ArkUIInteractionObserverAPI* GetInteractionObserverAPI()
{
    static const ArkUIInteractionObserverAPI api { FakeRegister, FakeUnregister };
    return support ? &api : nullptr;
}
ArkUIFullNodeAPI* GetFullImplForErrorMessage()
{
    static ArkUIFullNodeAPI api {};
    api.getBasicAPI = Basic;
    return &api;
}
} // namespace OHOS::Ace::NodeModel

class InteractionObserverCapiTest : public testing::Test {
public:
    void SetUp() override
    {
        support = true;
        instanceValid = true;
        lastInstance = -1;
        lastMask = 0;
        fakeObservers.clear();
        fakeByInstance.clear();
        fakeNextId = 1;
    }
};

TEST_F(InteractionObserverCapiTest, registerValidationOrderAndDiagnostics)
{
    ArkUI_Context context { 17 };
    uint32_t registerID = 12345;
    // 401 is ARKUI_ERROR_CODE_PARAM_INVALID for the null output slot.
    EXPECT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
        &context, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TAP, nullptr, nullptr, nullptr), 401);
    EXPECT_NE(std::string(OH_ArkUI_NativeModule_GetErrorMessage()).find("Registration ID"), std::string::npos);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
        &context, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TAP, &registerID, nullptr, nullptr), 401);
    // The registration ID slot is reset to 0 on failure.
    EXPECT_EQ(registerID, 0u);
    EXPECT_NE(std::string(OH_ArkUI_NativeModule_GetErrorMessage()).find("callback is null"), std::string::npos);
    // A mask without any supported bit is rejected.
    EXPECT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
        &context, 0, &registerID,
        [](const OH_ArkUI_NativeModule_UIJsonWrapper*, void*) {}, nullptr), 401);
    EXPECT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
        &context, 1u << 20, &registerID,
        [](const OH_ArkUI_NativeModule_UIJsonWrapper*, void*) {}, nullptr), 401);
    EXPECT_EQ(registerID, 0u);
    EXPECT_NE(std::string(OH_ArkUI_NativeModule_GetErrorMessage()).find("no supported event bit"),
        std::string::npos);
    // 190001 is ARKUI_ERROR_CODE_UI_CONTEXT_INVALID for the null UI context.
    EXPECT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
        nullptr, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TAP, &registerID,
        [](const OH_ArkUI_NativeModule_UIJsonWrapper*, void*) {}, nullptr), 190001);
    EXPECT_EQ(registerID, 0u);
    // 500 is ARKUI_ERROR_CODE_CAPI_INIT_ERROR while the bridge is unavailable.
    support = false;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
        &context, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TAP, &registerID,
        [](const OH_ArkUI_NativeModule_UIJsonWrapper*, void*) {}, nullptr), 500);
    EXPECT_EQ(registerID, 0u);
    auto message = std::string(OH_ArkUI_NativeModule_GetErrorMessage());
    EXPECT_NE(message.find("OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver"),
        std::string::npos);
    EXPECT_NE(message.find("unavailable"), std::string::npos);
}

TEST_F(InteractionObserverCapiTest, registerSuccessIndependentIdsAndMaskFiltering)
{
    ArkUI_Context context { 17 };
    auto callback = [](const OH_ArkUI_NativeModule_UIJsonWrapper*, void*) {};
    uint32_t first = 0;
    uint32_t second = 0;
    ASSERT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
        &context, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TAP, &first, callback, nullptr), 0);
    ASSERT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
        &context, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TAP, &second, callback, nullptr), 0);
    // The collector must receive the exact mock instance ID 17 supplied by the caller.
    EXPECT_EQ(lastInstance, 17);
    // Re-registering the same callback and userData creates an independent observer.
    EXPECT_NE(first, 0u);
    EXPECT_NE(second, 0u);
    EXPECT_NE(first, second);
    // Bits outside the supported range are ignored, not rejected.
    uint32_t third = 0;
    ASSERT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
        &context, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_CLICK | (1u << 20), &third, callback,
        nullptr), 0);
    EXPECT_EQ(fakeObservers[third].mask, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_CLICK);
    // A stale instance without a live container reports an invalid UI context.
    instanceValid = false;
    uint32_t stale = 99;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
        &context, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TAP, &stale, callback, nullptr), 190001);
    EXPECT_EQ(stale, 0u);
}

TEST_F(InteractionObserverCapiTest, unregisterSemantics)
{
    ArkUI_Context context { 3 };
    auto callback = [](const OH_ArkUI_NativeModule_UIJsonWrapper*, void*) {};
    // 0 is never a valid registration ID.
    EXPECT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionUnregisterInteractionObserver(0), 401);
    // An unknown ID is rejected.
    EXPECT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionUnregisterInteractionObserver(777), 401);
    uint32_t id = 0;
    ASSERT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
        &context, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TOUCH, &id, callback, nullptr), 0);
    ASSERT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionUnregisterInteractionObserver(id), 0);
    // Unregistering the same ID again reports an invalid ID and stays crash-free.
    EXPECT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionUnregisterInteractionObserver(id), 401);
    // The bridge is resolved again on each unregister call.
    support = false;
    EXPECT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionUnregisterInteractionObserver(id), 500);
    EXPECT_NE(std::string(OH_ArkUI_NativeModule_GetErrorMessage()).find(
        "OH_ArkUI_NativeModule_UIInfoCollectionUnregisterInteractionObserver"), std::string::npos);
}

TEST_F(InteractionObserverCapiTest, dispatchDeliversBorrowedWrapperAndStopsAfterUnregister)
{
    ArkUI_Context context { 21 };
    struct Captured {
        std::string payload;
        int calls = 0;
        void* seenUserData = nullptr;
    } captured;
    auto callback = [](const OH_ArkUI_NativeModule_UIJsonWrapper* json, void* userData) {
        auto* sink = static_cast<Captured*>(userData);
        sink->payload = OH_ArkUI_NativeModule_UIJsonWrapperGetData(json);
        sink->calls++;
        sink->seenUserData = userData;
    };
    uint32_t tapObserver = 0;
    uint32_t clickObserver = 0;
    ASSERT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
        &context, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TAP, &tapObserver, callback,
        &captured), 0);
    ASSERT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
        &context, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_CLICK, &clickObserver, callback,
        nullptr), 0);
    const std::string tapJson = "{\"schemaVersion\":1,\"type\":\"Tap\",\"id\":5}";
    FakeFireEvent(21, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TAP, tapJson);
    // Only the observer subscribed to the event bit receives the payload.
    EXPECT_EQ(captured.calls, 1);
    EXPECT_EQ(captured.payload, tapJson);
    EXPECT_EQ(captured.seenUserData, &captured);
    // The copy taken inside the callback survives the wrapper destruction.
    const std::string copied = captured.payload;
    // Unregister stops further deliveries.
    ASSERT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionUnregisterInteractionObserver(tapObserver), 0);
    FakeFireEvent(21, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TAP, tapJson);
    EXPECT_EQ(captured.calls, 1);
    EXPECT_EQ(copied, tapJson);
    // Events of an unrelated instance are not delivered.
    FakeFireEvent(99, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_CLICK,
        "{\"schemaVersion\":1,\"type\":\"Click\"}");
    ASSERT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionUnregisterInteractionObserver(clickObserver), 0);
}

TEST_F(InteractionObserverCapiTest, dispatchSharesImmutableWrapperAcrossObservers)
{
    ArkUI_Context context { 4 };
    int firstCalls = 0;
    int secondCalls = 0;
    std::string firstPayload;
    std::string secondPayload;
    struct Sink {
        int* calls;
        std::string* payload;
    } first { &firstCalls, &firstPayload }, second { &secondCalls, &secondPayload };
    auto callback = [](const OH_ArkUI_NativeModule_UIJsonWrapper* json, void* userData) {
        auto* sink = static_cast<Sink*>(userData);
        (*sink->calls)++;
        *sink->payload = OH_ArkUI_NativeModule_UIJsonWrapperGetData(json);
    };
    uint32_t firstId = 0;
    uint32_t secondId = 0;
    ASSERT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
        &context, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_DRAG, &firstId, callback, &first), 0);
    ASSERT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
        &context, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_DRAG |
            OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TOUCH, &secondId, callback, &second), 0);
    const std::string dragJson = "{\"schemaVersion\":1,\"type\":\"Drag\",\"action\":\"start\"}";
    FakeFireEvent(4, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_DRAG, dragJson);
    EXPECT_EQ(firstCalls, 1);
    EXPECT_EQ(secondCalls, 1);
    EXPECT_EQ(firstPayload, dragJson);
    EXPECT_EQ(secondPayload, dragJson);
    // The touch bit only reaches the second observer.
    FakeFireEvent(4, OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TOUCH,
        "{\"schemaVersion\":1,\"type\":\"Touch\",\"action\":\"down\"}");
    EXPECT_EQ(firstCalls, 1);
    EXPECT_EQ(secondCalls, 2);
    ASSERT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionUnregisterInteractionObserver(firstId), 0);
    ASSERT_EQ(OH_ArkUI_NativeModule_UIInfoCollectionUnregisterInteractionObserver(secondId), 0);
}

TEST(InteractionJsonBuilderTest, tapClickAndTouchSchemas)
{
    auto tap = OHOS::Ace::NG::ArkUIInteractionJsonBuilder::BuildTapOrClickJson(
        false, 5, { 100.0, 200.0 }, 1, 1);
    auto json = OHOS::Ace::JsonUtil::ParseJsonString(tap);
    ASSERT_NE(json, nullptr);
    auto point = json->GetValue("point");
    ASSERT_NE(point, nullptr);
    EXPECT_EQ(json->GetInt("schemaVersion"), 1);
    EXPECT_EQ(json->GetString("type"), "Tap");
    EXPECT_EQ(json->GetInt("id"), 5);
    EXPECT_TRUE(point->IsArray());
    ASSERT_EQ(point->GetArraySize(), 2);
    EXPECT_DOUBLE_EQ(point->GetArrayItem(0)->GetDouble(), 100.0);
    EXPECT_DOUBLE_EQ(point->GetArrayItem(1)->GetDouble(), 200.0);
    EXPECT_EQ(json->GetInt("count"), 1);
    EXPECT_EQ(json->GetInt("fingers"), 1);

    auto click = OHOS::Ace::NG::ArkUIInteractionJsonBuilder::BuildTapOrClickJson(
        true, 6, { 10.5, 20.5 }, 2, 1);
    json = OHOS::Ace::JsonUtil::ParseJsonString(click);
    ASSERT_NE(json, nullptr);
    point = json->GetValue("point");
    ASSERT_NE(point, nullptr);
    EXPECT_EQ(json->GetString("type"), "Click");
    EXPECT_EQ(json->GetInt("count"), 2);
    EXPECT_DOUBLE_EQ(point->GetArrayItem(0)->GetDouble(), 10.5);

    auto down = OHOS::Ace::NG::ArkUIInteractionJsonBuilder::BuildTouchJson(true, 3, { 11.0, 22.0 }, true, 42);
    json = OHOS::Ace::JsonUtil::ParseJsonString(down);
    ASSERT_NE(json, nullptr);
    EXPECT_EQ(json->GetString("type"), "Touch");
    EXPECT_EQ(json->GetString("action"), "down");
    EXPECT_EQ(json->GetInt("fingerId"), 3);
    EXPECT_EQ(json->GetInt("id"), 42);

    auto up = OHOS::Ace::NG::ArkUIInteractionJsonBuilder::BuildTouchJson(false, 3, { 11.0, 22.0 }, true, 42);
    json = OHOS::Ace::JsonUtil::ParseJsonString(up);
    ASSERT_NE(json, nullptr);
    EXPECT_EQ(json->GetString("action"), "up");
    // Up events never carry a hit node id.
    EXPECT_FALSE(json->Contains("id"));
}

TEST(InteractionJsonBuilderTest, longPressPanPinchRotationSchemas)
{
    auto longPress = OHOS::Ace::NG::ArkUIInteractionJsonBuilder::BuildLongPressJson(7, { 1.0, 2.0 }, 500);
    auto json = OHOS::Ace::JsonUtil::ParseJsonString(longPress);
    ASSERT_NE(json, nullptr);
    EXPECT_EQ(json->GetString("type"), "LongPress");
    EXPECT_EQ(json->GetString("action"), "end");
    EXPECT_EQ(json->GetInt64("actualDuration"), 500);

    // PanDirection::HORIZONTAL == 3.
    auto panCancel = OHOS::Ace::NG::ArkUIInteractionJsonBuilder::BuildPanJson(8, { 3.0, 4.0 }, 3, "cancel");
    json = OHOS::Ace::JsonUtil::ParseJsonString(panCancel);
    ASSERT_NE(json, nullptr);
    EXPECT_EQ(json->GetString("type"), "Pan");
    EXPECT_EQ(json->GetString("action"), "cancel");
    // Cancel keeps id/point/direction per the interaction observer contract.
    EXPECT_EQ(json->GetInt("id"), 8);
    EXPECT_EQ(json->GetInt("direction"), 3);

    auto pinchEnd = OHOS::Ace::NG::ArkUIInteractionJsonBuilder::BuildPinchJson(
        { 9, { { 1.0, 2.0 }, { 3.0, 4.0 } }, 2, "end", 1.5 });
    json = OHOS::Ace::JsonUtil::ParseJsonString(pinchEnd);
    ASSERT_NE(json, nullptr);
    auto point = json->GetValue("point");
    ASSERT_NE(point, nullptr);
    EXPECT_EQ(json->GetString("type"), "Pinch");
    EXPECT_EQ(json->GetString("action"), "end");
    EXPECT_EQ(json->GetInt("fingers"), 2);
    ASSERT_EQ(point->GetArraySize(), 2);
    EXPECT_DOUBLE_EQ(point->GetArrayItem(1)->GetArrayItem(0)->GetDouble(), 3.0);
    EXPECT_DOUBLE_EQ(json->GetDouble("scale"), 1.5);

    auto pinchStart = OHOS::Ace::NG::ArkUIInteractionJsonBuilder::BuildPinchJson(
        { 9, { { 1.0, 2.0 } }, 1, "start", std::nullopt });
    json = OHOS::Ace::JsonUtil::ParseJsonString(pinchStart);
    ASSERT_NE(json, nullptr);
    // Scale is only present on end.
    EXPECT_FALSE(json->Contains("scale"));

    auto rotationEnd = OHOS::Ace::NG::ArkUIInteractionJsonBuilder::BuildRotationJson(
        { 10, { { 5.0, 6.0 }, { 7.0, 8.0 } }, 2, "end", -45.0 });
    json = OHOS::Ace::JsonUtil::ParseJsonString(rotationEnd);
    ASSERT_NE(json, nullptr);
    EXPECT_EQ(json->GetString("type"), "Rotation");
    EXPECT_DOUBLE_EQ(json->GetDouble("angle"), -45.0);
    auto rotationCancel = OHOS::Ace::NG::ArkUIInteractionJsonBuilder::BuildRotationJson(
        { 10, { { 5.0, 6.0 } }, 1, "cancel", std::nullopt });
    json = OHOS::Ace::JsonUtil::ParseJsonString(rotationCancel);
    ASSERT_NE(json, nullptr);
    EXPECT_FALSE(json->Contains("angle"));
}

TEST(InteractionJsonBuilderTest, swipeAndDragSchemas)
{
    auto swipe = OHOS::Ace::NG::ArkUIInteractionJsonBuilder::BuildSwipeJson(
        { 11, { { 1.0, 2.0 } }, { { 3.0, 4.0 } }, 12, 100.0, 250.5 });
    auto json = OHOS::Ace::JsonUtil::ParseJsonString(swipe);
    ASSERT_NE(json, nullptr);
    auto downPoint = json->GetValue("downPoint");
    auto upPoint = json->GetValue("upPoint");
    ASSERT_NE(downPoint, nullptr);
    ASSERT_NE(upPoint, nullptr);
    EXPECT_EQ(json->GetString("type"), "Swipe");
    ASSERT_EQ(downPoint->GetArraySize(), 1);
    ASSERT_EQ(upPoint->GetArraySize(), 1);
    EXPECT_DOUBLE_EQ(downPoint->GetArrayItem(0)->GetArrayItem(1)->GetDouble(), 2.0);
    EXPECT_EQ(json->GetInt("direction"), 12);
    EXPECT_DOUBLE_EQ(json->GetDouble("speed"), 100.0);
    EXPECT_DOUBLE_EQ(json->GetDouble("actualSpeed"), 250.5);
    // Without recorded down events the up positions act as the down positions.
    auto swipeFallback = OHOS::Ace::NG::ArkUIInteractionJsonBuilder::BuildSwipeJson(
        { 11, {}, { { 3.0, 4.0 } }, 12, 100.0, 250.5 });
    json = OHOS::Ace::JsonUtil::ParseJsonString(swipeFallback);
    ASSERT_NE(json, nullptr);
    downPoint = json->GetValue("downPoint");
    ASSERT_NE(downPoint, nullptr);
    ASSERT_EQ(downPoint->GetArraySize(), 1);
    EXPECT_DOUBLE_EQ(downPoint->GetArrayItem(0)->GetArrayItem(0)->GetDouble(), 3.0);

    auto dragStart = OHOS::Ace::NG::ArkUIInteractionJsonBuilder::BuildDragStartJson(
        12, { 30.0, 40.0 }, "com.example.app", 800);
    json = OHOS::Ace::JsonUtil::ParseJsonString(dragStart);
    ASSERT_NE(json, nullptr);
    EXPECT_EQ(json->GetString("type"), "Drag");
    EXPECT_EQ(json->GetString("action"), "start");
    EXPECT_EQ(json->GetInt("id"), 12);
    EXPECT_EQ(json->GetString("hostName"), "com.example.app");
    EXPECT_EQ(json->GetInt64("actualDuration"), 800);

    auto dragEndSuccess = OHOS::Ace::NG::ArkUIInteractionJsonBuilder::BuildDragEndJson(
        { { 50.0, 60.0 }, "com.example.app", true, 77 });
    json = OHOS::Ace::JsonUtil::ParseJsonString(dragEndSuccess);
    ASSERT_NE(json, nullptr);
    EXPECT_EQ(json->GetString("action"), "end");
    EXPECT_EQ(json->GetString("dropResult"), "success");
    EXPECT_EQ(json->GetInt("id"), 77);

    auto dragEndFail = OHOS::Ace::NG::ArkUIInteractionJsonBuilder::BuildDragEndJson(
        { { 50.0, 60.0 }, "com.example.app", false, std::nullopt });
    json = OHOS::Ace::JsonUtil::ParseJsonString(dragEndFail);
    ASSERT_NE(json, nullptr);
    EXPECT_EQ(json->GetString("dropResult"), "fail");
    // The target id is only reported on success.
    EXPECT_FALSE(json->Contains("id"));
}

// The page-text and UI-agent tree entry points live in the same translation
// unit; stub their bridges so this executable links without the collectors.
// The CHECK_UI_THREAD hooks reference ConfigManager::CheckUIThread, defined
// in config_manager.cpp together with the GetFullImpl() bridge (node_model
// graph) that this minimal harness deliberately avoids. The hook is
// diagnosis-only and never gates the API, so a no-op definition keeps the
// real ui_info_collection.cpp linkable with unchanged behavior; the real
// implementation is covered by test/unittest/capi/runtime_check/.
#include "core/interfaces/native/node/page_text_api.h"
#include "core/interfaces/native/node/ui_tree_api.h"
#include "interfaces/native/node/config_manager.h"
namespace OHOS::Ace::NodeModel {
const ArkUIPageTextAPI* GetPageTextAPI()
{
    return nullptr;
}
const ArkUITreeAPI* GetTreeAPI()
{
    return nullptr;
}
} // namespace OHOS::Ace::NodeModel
namespace OHOS::Ace::NodeModel::ConfigManager {
void CheckUIThread(const char*) {}
} // namespace OHOS::Ace::NodeModel::ConfigManager
