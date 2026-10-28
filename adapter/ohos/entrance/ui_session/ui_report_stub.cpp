/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "interfaces/inner_api/ui_session/ui_report_stub.h"

#include <algorithm>
#include <atomic>
#include <cinttypes>

#include "cJSON.h"

#include "pixel_map.h"

#include "adapter/ohos/entrance/ui_session/include/large_string_ashmem.h"
#include "adapter/ohos/entrance/ui_session/include/ui_session_trace.h"
#include "adapter/ohos/entrance/ui_session/include/ui_session_log.h"
#include "interfaces/inner_api/ui_session/ui_content_errors.h"
#include "interfaces/inner_api/ui_session/ui_session_ipc_util.h"

namespace {
constexpr char GET_INSPECTOR_TREE_CALLBACK_TIMEOUT[] = "GetInspectorTreeCallbackTimeout";
constexpr char NAVIGATION_QUERY_CALLBACK_TIMEOUT[] = "NavigationQueryCallbackTimeout";
constexpr char LAZY_FOREACH_QUERY_CALLBACK_TIMEOUT[] = "LazyForEachQueryCallbackTimeout";
constexpr char NODES_IN_RECT_QUERY_CALLBACK_TIMEOUT[] = "NodesInRectQueryCallbackTimeout";
constexpr char PAGE_TRANSLATE_CALLBACK_TIMEOUT[] = "PageTranslateCallbackTimeout";
constexpr char PAGE_TRANSLATE_RESULT_WATCHDOG_PREFIX[] = "PageTranslateResultWatchdog";
std::atomic<uint64_t> g_nextCommandRequestId { 0 };
constexpr char SEND_COMMAND_CALLBACK_TIMEOUT_PREFIX[] = "SendCommandCallbackTimeout";
constexpr int32_t SPECIFIED_CONTENT_OFFSETS_LOOP_UPPERBOUND = 1024000;
constexpr char PAGE_SCENE_RULE_SET_ID[] = "ruleSetId";
constexpr char PAGE_SCENE_HITS[] = "hits";
constexpr char PAGE_SCENE_EXITS[] = "exits";

// Legacy events carry a top-level ruleSetId; search events use hits/exits.
// Dispatch once per referenced ruleSet, filtering search evidence before delivery.
std::vector<std::string> ExtractPageSceneRuleSetIds(const std::string& sceneJson)
{
    std::vector<std::string> ruleSetIds;
    auto root = cJSON_ParseWithOpts(sceneJson.c_str(), nullptr, true);
    if (!cJSON_IsObject(root)) {
        cJSON_Delete(root);
        return ruleSetIds;
    }
    auto addRuleSetId = [&ruleSetIds](const cJSON* value) {
        if (!cJSON_IsString(value) || value->valuestring == nullptr || value->valuestring[0] == '\0') {
            return;
        }
        std::string ruleSetId(value->valuestring);
        if (std::find(ruleSetIds.begin(), ruleSetIds.end(), ruleSetId) == ruleSetIds.end()) {
            ruleSetIds.emplace_back(std::move(ruleSetId));
        }
    };
    addRuleSetId(cJSON_GetObjectItem(root, PAGE_SCENE_RULE_SET_ID));
    for (const auto* key : { PAGE_SCENE_HITS, PAGE_SCENE_EXITS }) {
        auto array = cJSON_GetObjectItem(root, key);
        if (!cJSON_IsArray(array)) {
            continue;
        }
        for (int32_t index = 0; index < cJSON_GetArraySize(array); ++index) {
            auto item = cJSON_GetArrayItem(array, index);
            if (!cJSON_IsObject(item)) {
                continue;
            }
            addRuleSetId(cJSON_GetObjectItem(item, PAGE_SCENE_RULE_SET_ID));
        }
    }
    cJSON_Delete(root);
    return ruleSetIds;
}

std::string FilterPageSceneEvent(const std::string& sceneJson, const std::string& ruleSetId)
{
    auto root = cJSON_ParseWithOpts(sceneJson.c_str(), nullptr, true);
    if (!cJSON_IsObject(root)) {
        cJSON_Delete(root);
        return "";
    }
    auto legacyRuleSetId = cJSON_GetObjectItem(root, PAGE_SCENE_RULE_SET_ID);
    if (cJSON_IsString(legacyRuleSetId) && legacyRuleSetId->valuestring != nullptr &&
        ruleSetId != legacyRuleSetId->valuestring) {
        cJSON_Delete(root);
        return "";
    }
    bool filtered = false;
    for (const auto* key : { PAGE_SCENE_HITS, PAGE_SCENE_EXITS }) {
        auto array = cJSON_GetObjectItem(root, key);
        if (array == nullptr) {
            continue;
        }
        if (!cJSON_IsArray(array)) {
            cJSON_Delete(root);
            return "";
        }
        for (int32_t index = cJSON_GetArraySize(array) - 1; index >= 0; --index) {
            auto item = cJSON_GetArrayItem(array, index);
            if (item == nullptr) {
                cJSON_DeleteItemFromArray(array, index);
                filtered = true;
                continue;
            }
            auto id = cJSON_GetObjectItem(item, PAGE_SCENE_RULE_SET_ID);
            if (!cJSON_IsString(id) || id->valuestring == nullptr || ruleSetId != id->valuestring) {
                cJSON_DeleteItemFromArray(array, index);
                filtered = true;
            }
        }
    }
    if (!filtered) {
        cJSON_Delete(root);
        return sceneJson;
    }
    char* serialized = cJSON_PrintUnformatted(root);
    std::string result = serialized == nullptr ? "" : serialized;
    if (serialized != nullptr) {
        cJSON_free(serialized);
    }
    cJSON_Delete(root);
    return result;
}


inline int32_t NormalizeTimeout(int32_t timeout, int32_t defaultTimeout)
{
    return timeout > 0 ? timeout : defaultTimeout;
}

std::string MakePageTranslateResultWatchdogTaskName(int32_t nodeId, int64_t version)
{
    return std::string(PAGE_TRANSLATE_RESULT_WATCHDOG_PREFIX) + "_" + std::to_string(nodeId) + "_" +
           std::to_string(version);
}

std::string MakeSendCommandCallbackTimeoutTaskName(uint64_t requestId)
{
    return std::string(SEND_COMMAND_CALLBACK_TIMEOUT_PREFIX) + "_" + std::to_string(requestId);
}

void AddArkUIImagesByIds(OHOS::MessageParcel& data,
    std::unordered_map<int32_t, std::shared_ptr<OHOS::Media::PixelMap>>& componentImages)
{
    uint64_t componentImagesSize = data.ReadUint64();
    constexpr int32_t GET_IMAGES_BY_ID_LOOP_UPPERBOUND = 20;
    if (componentImagesSize > GET_IMAGES_BY_ID_LOOP_UPPERBOUND) {
        return;
    }
    for (uint64_t i = 0; i < componentImagesSize; ++i) {
        std::shared_ptr<OHOS::Media::PixelMap> pixelMap = nullptr;
        int32_t componentImageId = data.ReadInt32();
        bool nextPixelMapIsAvailable = data.ReadBool();
        if (nextPixelMapIsAvailable) {
            pixelMap.reset(OHOS::Media::PixelMap::Unmarshalling(data));
        }
        componentImages.emplace(componentImageId, pixelMap);
    }
}

void AddArkWebImagesByIds(OHOS::MessageParcel& data,
    std::map<int32_t, std::map<int32_t, std::shared_ptr<OHOS::Media::PixelMap>>>& webImages)
{
    uint64_t webImagesAllMapSize = data.ReadUint64();
    constexpr int32_t GET_IMAGES_BY_ID_LOOP_UPPERBOUND = 20;
    if (webImagesAllMapSize > GET_IMAGES_BY_ID_LOOP_UPPERBOUND) {
        return;
    }
    for (uint64_t i = 0; i < webImagesAllMapSize; ++i) {
        int32_t webId = data.ReadInt32();
        uint64_t imagesInOneWebSize = data.ReadUint64();
        if (imagesInOneWebSize > GET_IMAGES_BY_ID_LOOP_UPPERBOUND) {
            return;
        }
        std::map<int32_t, std::shared_ptr<OHOS::Media::PixelMap>> imagesInOneWeb;
        for (uint64_t j = 0; j < imagesInOneWebSize; ++j) {
            std::shared_ptr<OHOS::Media::PixelMap> pixelMap = nullptr;
            int32_t imageId = data.ReadInt32();
            bool nextPixelMapIsAvailable = data.ReadBool();
            if (nextPixelMapIsAvailable) {
                pixelMap.reset(OHOS::Media::PixelMap::Unmarshalling(data));
            }
            imagesInOneWeb.emplace(imageId, pixelMap);
        }
        webImages.emplace(webId, std::move(imagesInOneWeb));
    }
}

} // namespace

namespace OHOS::Ace {

int32_t UiReportStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LOGW("ui_session InterfaceToken check failed");
        return -1;
    }
    switch (code) {
        case REPORT_CLICK_EVENT: {
            std::string result = data.ReadString();
            ReportClickEvent(result);
            break;
        }
        case REPORT_SWITCH_EVENT: {
            std::string result = data.ReadString();
            ReportRouterChangeEvent(result);
            break;
        }
        case REPORT_COMPONENT_EVENT: {
            std::string result = data.ReadString();
            ReportComponentChangeEvent(result);
            break;
        }
        case REPORT_SEARCH_EVENT: {
            std::string result = data.ReadString();
            ReportSearchEvent(result);
            break;
        }
        case REPORT_TEXT_CHANGE_EVENT: {
            std::string result = data.ReadString();
            ReportTextChangeEvent(result);
            break;
        }
        case REPORT_INSPECTOR_VALUE: {
            sptr<LargeStringAshmem> largeStringAshmem = data.ReadParcelable<LargeStringAshmem>();
            if (!largeStringAshmem) {
                LOGW("ReportInspectorTreeValue read LargeStringAshmem failed");
                break;
            }
            std::string result = "";
            if (!largeStringAshmem->ReadFromAshmem(result)) {
                LOGW("ReportInspectorTreeValue read data failed");
                break;
            }
            int32_t partNum = data.ReadInt32();
            bool isLastPart = data.ReadBool();
            ReportInspectorTreeValue(result, partNum, isLastPart);
            break;
        }
        case REPORT_WEB_UNFOCUS_EVENT: {
            std::string result = data.ReadString();
            int64_t accessibilityId = data.ReadInt64();
            ReportWebUnfocusEvent(accessibilityId, result);
            break;
        }
        case REPORT_SCROLL_EVENT: {
            std::string result = data.ReadString();
            ReportScrollEvent(result);
            break;
        }
        case REPORT_LIFE_CYCLE_EVENT: {
            std::string result = data.ReadString();
            ReportLifeCycleEvent(result);
            break;
        }
        case REPORT_SELECT_TEXT_EVENT: {
            std::string result = data.ReadString();
            ReportSelectTextEvent(result);
            break;
        }
        case SEND_BASE_INFO: {
            std::string result = data.ReadString();
            SendBaseInfo(result);
            break;
        }
        case SEND_CURRENT_LANGUAGE: {
            std::string result = data.ReadString();

            SendCurrentLanguage(result);
            break;
        }
        case SEND_TEXT: {
            std::string res = data.ReadString();
            int32_t nodeId = data.ReadInt32();
            SendWebText(nodeId, res);
            break;
        }
        case SEND_IMAGES: {
            std::vector<std::pair<int32_t, std::shared_ptr<Media::PixelMap>>> result;
            int32_t size = 0;
            if (!data.ReadInt32(size)) {
                LOGW("SendShowingImage size read failed");
                break;
            }
            constexpr int32_t SEND_IMAGES_SIZE_UPPERBOUND = 1000;
            if (size < 0 || size > SEND_IMAGES_SIZE_UPPERBOUND) {
                LOGW("SendShowingImage size is negative or greater than upper bound: %{public}d", size);
                break;
            }
            for (int32_t i = 0; i < size; i++) {
                int32_t nodeId = data.ReadInt32();
                auto pixelMap = std::shared_ptr<Media::PixelMap>(OHOS::Media::PixelMap::Unmarshalling(data));
                std::pair<int32_t, std::shared_ptr<Media::PixelMap>> value = { nodeId, pixelMap };
                result.push_back(value);
            }
            SendShowingImage(result);
            break;
        }
        case SEND_ARKUI_IMAGES_BY_ID: {
            std::unordered_map<int32_t, std::shared_ptr<OHOS::Media::PixelMap>> componentImages;
            int32_t windowId = data.ReadInt32();
            AddArkUIImagesByIds(data, componentImages);
            int32_t arkUIErrorCode = data.ReadInt32();
            SendArkUIImagesById(windowId, componentImages, static_cast<MultiImageQueryErrorCode>(arkUIErrorCode));
            break;
        }
        case SEND_ARKWEB_IMAGES_BY_ID: {
            std::map<int32_t, std::map<int32_t, std::shared_ptr<OHOS::Media::PixelMap>>> webImages;
            int32_t windowId = data.ReadInt32();
            AddArkWebImagesByIds(data, webImages);
            int32_t arkWebErrorCode = data.ReadInt32();
            SendArkWebImagesById(windowId, webImages, static_cast<MultiImageQueryErrorCode>(arkWebErrorCode));
            break;
        }
        case SEND_CURRENT_PAGE_NAME: {
            std::string result = data.ReadString();
            SendCurrentPageName(result);
            break;
        }
        case SEND_EXE_APP_AI_FUNCTION_RESULT: {
            uint32_t result = data.ReadUint32();
            std::string dataString = data.ReadString();
            SendExeAppAIFunctionResult(result, dataString);
            break;
        }
        case SEND_SPECIFIED_CONTENT_OFFSETS: {
            std::vector<std::pair<float, float>> result;
            int32_t size = 0;
            if (!data.ReadInt32(size)) {
                LOGW("SendSpecifiedContentOffsets size read failed");
                break;
            }
            const int32_t loopSize = std::clamp(size, 0, SPECIFIED_CONTENT_OFFSETS_LOOP_UPPERBOUND);
            if (size < 0 || size > loopSize) {
                LOGW("SendSpecifiedContentOffsets size %{public}d exceeds boundary, clamp to %{public}d",
                    size, loopSize);
            }
            for (int32_t i = 0; i < loopSize; i++) {
                float offsetX = data.ReadFloat();
                float offsetY = data.ReadFloat();
                std::pair<float, float> value = { offsetX, offsetY };
                result.push_back(value);
            }
            SendSpecifiedContentOffsets(result);
            break;
        }
        case SEND_CONTENT_CHANGE: {
            ChangeType type = static_cast<ChangeType>(data.ReadInt32());
            std::string simpleTree = data.ReadString();
            SendContentChange(type, simpleTree);
            break;
        }
        case REPORT_HIT_TEST_NODE_INFOS: {
            std::string result = data.ReadString();
            int32_t partNum = data.ReadInt32();
            bool isLastPart = data.ReadBool();
            ReportHitTestNodeInfos(result, partNum, isLastPart);
            break;
        }
        case REPORT_STATE_MGMT_INFO: {
            std::vector<std::string> results;
            data.ReadStringVector(&results);
            ReportGetStateMgmtInfo(results);
            break;
        }
        case SEND_WEB_INFO_BY_REQUEST: {
            OnGetWebInfoByRequestInner(data);
            break;
        }
        case SEND_PAGE_TEXT: {
            OnSendPageTextInner(data);
            break;
        }
        case REPORT_PAGE_SCENE_EVENT: {
            std::string result = data.ReadString();
            bool isGetResult = false;
            if (data.GetReadableBytes() >= sizeof(bool)) {
                isGetResult = data.ReadBool();
            }
            ReportPageSceneEvent(result, isGetResult);
            break;
        }
        case REPORT_NAVIGATION_QUERY_RESULT: {
            sptr<LargeStringAshmem> largeStringAshmem = data.ReadParcelable<LargeStringAshmem>();
            if (!largeStringAshmem) {
                LOGW("ReportNavigationQueryResult read LargeStringAshmem failed");
                break;
            }
            std::string result;
            if (!largeStringAshmem->ReadFromAshmem(result)) {
                LOGW("ReportNavigationQueryResult read data failed");
                break;
            }
            ReportNavigationQueryResult(result);
            break;
        }
        case REPORT_LAZY_FOREACH_QUERY_RESULT: {
            sptr<LargeStringAshmem> largeStringAshmem = data.ReadParcelable<LargeStringAshmem>();
            if (!largeStringAshmem) {
                LOGW("ReportLazyForEachQueryResult read LargeStringAshmem failed");
                break;
            }
            std::string result;
            if (!largeStringAshmem->ReadFromAshmem(result)) {
                LOGW("ReportLazyForEachQueryResult read data failed");
                break;
            }
            ReportLazyForEachQueryResult(result);
            break;
        }
        case REPORT_NODES_IN_RECT_QUERY_RESULT: {
            sptr<LargeStringAshmem> largeStringAshmem = data.ReadParcelable<LargeStringAshmem>();
            if (!largeStringAshmem) {
                LOGW("ReportNodesInRectQueryResult read LargeStringAshmem failed");
                break;
            }
            std::string result;
            if (!largeStringAshmem->ReadFromAshmem(result)) {
                LOGW("ReportNodesInRectQueryResult read data failed");
                break;
            }
            ReportNodesInRectQueryResult(result);
            break;
        }
        case REPORT_COMMAND_RESULT: {
            uint64_t requestId = 0;
            if (!data.ReadUint64(requestId)) {
                LOGW("ReportCommandResult read parcel failed");
                return FAILED;
            }
            std::string resultJson;
            if (requestId == 0 || !data.ReadString(resultJson)) {
                return FAILED;
            }
            ReportCommandResult(requestId, resultJson);
            break;
        }

        default: {
            LOGI("ui_session unknown transaction code %{public}d", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
    return 0;
}

void UiReportStub::RegisterGetWebInfoByRequestCallback(const GetWebInfoByRequestCallback& finishCallback)
{
    getWebInfoByRequestCallback_ = std::move(finishCallback);
}

void UiReportStub::OnGetWebInfoByRequestInner(MessageParcel& data)
{
    uint32_t windowId = data.ReadUint32();
    int32_t webId = data.ReadInt32();
    std::string request = data.ReadString();
    sptr<LargeStringAshmem> largeStringAshmem = data.ReadParcelable<LargeStringAshmem>();
    if (!largeStringAshmem) {
        LOGW("OnGetWebInfoByRequestInner read LargeStringAshmem failed");
        return;
    }
    std::string result = "";
    if (!largeStringAshmem->ReadFromAshmem(result)) {
        LOGW("OnGetWebInfoByRequestInner read result failed");
        return;
    }
    WebRequestErrorCode errorCode = static_cast<WebRequestErrorCode>(data.ReadInt32());
    SendWebInfoRequestResult(windowId, webId, request, result, errorCode);
}

void UiReportStub::OnSendPageTextInner(MessageParcel& data)
{
    int32_t nodeId = data.ReadInt32();
    int64_t version = data.ReadInt64();
    std::string text;
    if (!UiSessionIpcUtil::ReadStringWithAshmemFlag(data, text, "OnSendPageTextInner")) {
        return;
    }
    SendPageText(nodeId, text, version);
}

void UiReportStub::SendWebInfoRequestResult(
    uint32_t windowId,
    int32_t webId,
    const std::string& request,
    const std::string& result, WebRequestErrorCode errorCode)
{
    if (!getWebInfoByRequestCallback_) {
        LOGW("getWebInfoByRequestCallback null");
        return;
    }
    getWebInfoByRequestCallback_(windowId, webId, request, result, errorCode);
}

void UiReportStub::ReportClickEvent(const std::string& data)
{
    if (clickEventCallback_ != nullptr) {
        clickEventCallback_(data);
    }
}

void UiReportStub::ReportRouterChangeEvent(const std::string& data)
{
    if (RouterChangeEventCallback_ != nullptr) {
        RouterChangeEventCallback_(data);
    }
}

void UiReportStub::ReportComponentChangeEvent(const std::string& data)
{
    if (ComponentChangeEventCallback_ != nullptr) {
        ComponentChangeEventCallback_(data);
    } else {
        LOGW("ReportComponentChangeEvent ComponentChangeEventCallback_ is null");
    }
}

void UiReportStub::ReportTextChangeEvent(const std::string& data)
{
    if (textChangeEventCallback_ != nullptr) {
        textChangeEventCallback_(data);
    }
}

void UiReportStub::ReportSearchEvent(const std::string& data)
{
    if (searchEventCallback_ != nullptr) {
        searchEventCallback_(data);
    }
}

void UiReportStub::ReportInspectorTreeValue(const std::string& data, int32_t partNum, bool isLastPart)
{
    std::function<void(std::string, int32_t, bool)> inspectorTreeCallback;
    {
        std::lock_guard<std::mutex> lock(inspectorTreeCallbackMutex_);
        if (inspectorTreeCallback_ == nullptr) {
            return;
        }
        inspectorTreeCallback = inspectorTreeCallback_;
        if (isLastPart) {
            CancelGetInspectorTreeCallbackTimeoutTaskLocked();
            inspectorTreeCallback_ = nullptr;
        }
    }
    inspectorTreeCallback(data, partNum, isLastPart);
}

void UiReportStub::ReportHitTestNodeInfos(const std::string& data, int32_t partNum, bool isLastPart)
{
    if (getHitTestNodeInfoCallback_ != nullptr) {
        getHitTestNodeInfoCallback_(data, partNum, isLastPart);
    }
}

void UiReportStub::RegisterGetHitTestNodeInfoCallback(
    const std::function<void(std::string, int32_t, bool)>& eventCallback)
{
    getHitTestNodeInfoCallback_ = std::move(eventCallback);
}

void UiReportStub::ReportNavigationQueryResult(const std::string& data)
{
    std::function<void(const std::string&)> callback;
    {
        std::lock_guard<std::mutex> lock(navigationQueryCallbackMutex_);
        if (!navigationQueryCallback_) {
            return;
        }
        callback = navigationQueryCallback_;
        CancelNavigationQueryCallbackTimeoutTaskLocked();
        navigationQueryCallback_ = nullptr;
    }
    callback(data);
}

bool UiReportStub::RegisterNavigationQueryCallback(
    const std::function<void(const std::string&)>& eventCallback)
{
    std::lock_guard<std::mutex> lock(navigationQueryCallbackMutex_);
    if (navigationQueryCallback_) {
        return false;
    }
    navigationQueryCallback_ = eventCallback;
    return true;
}

void UiReportStub::UnregisterNavigationQueryCallback()
{
    std::lock_guard<std::mutex> lock(navigationQueryCallbackMutex_);
    navigationQueryCallback_ = nullptr;
}

bool UiReportStub::PostNavigationQueryCallbackRemoveTask(int32_t timeout)
{
    auto eventHandler = eventHandler_.lock();
    if (eventHandler == nullptr) {
        LOGE("PostNavigationQueryCallbackRemoveTask eventHandler is null");
        std::function<void(const std::string&)> callback;
        {
            std::lock_guard<std::mutex> lock(navigationQueryCallbackMutex_);
            callback = navigationQueryCallback_;
            navigationQueryCallback_ = nullptr;
        }
        if (callback) {
            callback("{\"errorCode\":8}");
        }
        return false;
    }
    wptr<UiReportStub> weakThis = this;
    bool postTaskSuccess = eventHandler->PostTask([weakThis]() {
        auto uiReportStub = weakThis.promote();
        if (uiReportStub == nullptr) {
            LOGE("PostNavigationQueryCallbackRemoveTask uiReportStub is null");
            return;
        }
        uiReportStub->HandleNavigationQueryCallbackTimeout();
    }, NAVIGATION_QUERY_CALLBACK_TIMEOUT,
        NormalizeTimeout(timeout, DEFAULT_INSPECTOR_TREE_CALLBACK_TIMEOUT_MS));
    if (!postTaskSuccess) {
        LOGW("Post NavigationQuery timeout task failed");
        std::function<void(const std::string&)> callback;
        {
            std::lock_guard<std::mutex> lock(navigationQueryCallbackMutex_);
            callback = navigationQueryCallback_;
            navigationQueryCallback_ = nullptr;
        }
        if (callback) {
            callback("{\"errorCode\":8}");
        }
        return false;
    }
    return true;
}

void UiReportStub::HandleNavigationQueryCallbackTimeout()
{
    std::function<void(const std::string&)> callback;
    {
        std::lock_guard<std::mutex> lock(navigationQueryCallbackMutex_);
        if (!navigationQueryCallback_) {
            return;
        }
        callback = navigationQueryCallback_;
        navigationQueryCallback_ = nullptr;
    }
    callback("{\"errorCode\":8}");
}

void UiReportStub::CancelNavigationQueryCallbackTimeoutTaskLocked()
{
    auto eventHandler = eventHandler_.lock();
    if (eventHandler == nullptr) {
        LOGE("CancelNavigationQueryCallbackTimeoutTaskLocked eventHandler is null");
        return;
    }
    eventHandler->RemoveTask(NAVIGATION_QUERY_CALLBACK_TIMEOUT);
}

void UiReportStub::ReportLazyForEachQueryResult(const std::string& data)
{
    std::function<void(const std::string&)> callback;
    {
        std::lock_guard<std::mutex> lock(lazyForEachQueryCallbackMutex_);
        if (!lazyForEachQueryCallback_) {
            return;
        }
        callback = lazyForEachQueryCallback_;
        CancelLazyForEachQueryCallbackTimeoutTaskLocked();
        lazyForEachQueryCallback_ = nullptr;
    }
    callback(data);
}

bool UiReportStub::RegisterLazyForEachQueryCallback(
    const std::function<void(const std::string&)>& eventCallback)
{
    std::lock_guard<std::mutex> lock(lazyForEachQueryCallbackMutex_);
    if (lazyForEachQueryCallback_) {
        return false;
    }
    lazyForEachQueryCallback_ = eventCallback;
    return true;
}

void UiReportStub::UnregisterLazyForEachQueryCallback()
{
    std::lock_guard<std::mutex> lock(lazyForEachQueryCallbackMutex_);
    lazyForEachQueryCallback_ = nullptr;
}

bool UiReportStub::PostLazyForEachQueryCallbackRemoveTask(int32_t timeout)
{
    auto eventHandler = eventHandler_.lock();
    if (eventHandler == nullptr) {
        LOGE("PostLazyForEachQueryCallbackRemoveTask eventHandler is null");
        std::function<void(const std::string&)> callback;
        {
            std::lock_guard<std::mutex> lock(lazyForEachQueryCallbackMutex_);
            callback = lazyForEachQueryCallback_;
            lazyForEachQueryCallback_ = nullptr;
        }
        if (callback) {
            callback("{\"errorCode\":8}");
        }
        return false;
    }
    wptr<UiReportStub> weakThis = this;
    bool postTaskSuccess = eventHandler->PostTask([weakThis]() {
        auto uiReportStub = weakThis.promote();
        if (uiReportStub == nullptr) {
            LOGE("PostLazyForEachQueryCallbackRemoveTask uiReportStub is null");
            return;
        }
        uiReportStub->HandleLazyForEachQueryCallbackTimeout();
    }, LAZY_FOREACH_QUERY_CALLBACK_TIMEOUT,
        NormalizeTimeout(timeout, DEFAULT_INSPECTOR_TREE_CALLBACK_TIMEOUT_MS));
    if (!postTaskSuccess) {
        LOGW("Post LazyForEachQuery timeout task failed");
        std::function<void(const std::string&)> callback;
        {
            std::lock_guard<std::mutex> lock(lazyForEachQueryCallbackMutex_);
            callback = lazyForEachQueryCallback_;
            lazyForEachQueryCallback_ = nullptr;
        }
        if (callback) {
            callback("{\"errorCode\":8}");
        }
        return false;
    }
    return true;
}

void UiReportStub::HandleLazyForEachQueryCallbackTimeout()
{
    std::function<void(const std::string&)> callback;
    {
        std::lock_guard<std::mutex> lock(lazyForEachQueryCallbackMutex_);
        if (!lazyForEachQueryCallback_) {
            return;
        }
        callback = lazyForEachQueryCallback_;
        lazyForEachQueryCallback_ = nullptr;
    }
    callback("{\"errorCode\":8}");
}

void UiReportStub::CancelLazyForEachQueryCallbackTimeoutTaskLocked()
{
    auto eventHandler = eventHandler_.lock();
    if (eventHandler == nullptr) {
        LOGE("CancelLazyForEachQueryCallbackTimeoutTaskLocked eventHandler is null");
        return;
    }
    eventHandler->RemoveTask(LAZY_FOREACH_QUERY_CALLBACK_TIMEOUT);
}

void UiReportStub::ReportNodesInRectQueryResult(const std::string& data)
{
    std::function<void(const std::string&)> callback;
    {
        std::lock_guard<std::mutex> lock(nodesInRectQueryCallbackMutex_);
        if (!nodesInRectQueryCallback_) {
            return;
        }
        callback = nodesInRectQueryCallback_;
        CancelNodesInRectQueryCallbackTimeoutTaskLocked();
        nodesInRectQueryCallback_ = nullptr;
    }
    callback(data);
}

bool UiReportStub::RegisterNodesInRectQueryCallback(
    const std::function<void(const std::string&)>& eventCallback)
{
    std::lock_guard<std::mutex> lock(nodesInRectQueryCallbackMutex_);
    if (nodesInRectQueryCallback_) {
        return false;
    }
    nodesInRectQueryCallback_ = eventCallback;
    return true;
}

void UiReportStub::UnregisterNodesInRectQueryCallback()
{
    std::lock_guard<std::mutex> lock(nodesInRectQueryCallbackMutex_);
    nodesInRectQueryCallback_ = nullptr;
}

bool UiReportStub::PostNodesInRectQueryCallbackRemoveTask(int32_t timeout)
{
    auto eventHandler = eventHandler_.lock();
    if (eventHandler == nullptr) {
        LOGE("PostNodesInRectQueryCallbackRemoveTask eventHandler is null");
        std::function<void(const std::string&)> callback;
        {
            std::lock_guard<std::mutex> lock(nodesInRectQueryCallbackMutex_);
            callback = nodesInRectQueryCallback_;
            nodesInRectQueryCallback_ = nullptr;
        }
        if (callback) {
            callback("{\"errorCode\":8}");
        }
        return false;
    }
    wptr<UiReportStub> weakThis = this;
    bool postTaskSuccess = eventHandler->PostTask([weakThis]() {
        auto uiReportStub = weakThis.promote();
        if (uiReportStub == nullptr) {
            LOGE("PostNodesInRectQueryCallbackRemoveTask uiReportStub is null");
            return;
        }
        uiReportStub->HandleNodesInRectQueryCallbackTimeout();
    }, NODES_IN_RECT_QUERY_CALLBACK_TIMEOUT,
        NormalizeTimeout(timeout, DEFAULT_INSPECTOR_TREE_CALLBACK_TIMEOUT_MS));
    if (!postTaskSuccess) {
        LOGW("Post NodesInRectQuery timeout task failed");
        std::function<void(const std::string&)> callback;
        {
            std::lock_guard<std::mutex> lock(nodesInRectQueryCallbackMutex_);
            callback = nodesInRectQueryCallback_;
            nodesInRectQueryCallback_ = nullptr;
        }
        if (callback) {
            callback("{\"errorCode\":8}");
        }
        return false;
    }
    return true;
}

void UiReportStub::HandleNodesInRectQueryCallbackTimeout()
{
    std::function<void(const std::string&)> callback;
    {
        std::lock_guard<std::mutex> lock(nodesInRectQueryCallbackMutex_);
        if (!nodesInRectQueryCallback_) {
            return;
        }
        callback = nodesInRectQueryCallback_;
        nodesInRectQueryCallback_ = nullptr;
    }
    callback("{\"errorCode\":8}");
}

void UiReportStub::CancelNodesInRectQueryCallbackTimeoutTaskLocked()
{
    auto eventHandler = eventHandler_.lock();
    if (eventHandler == nullptr) {
        LOGE("CancelNodesInRectQueryCallbackTimeoutTaskLocked eventHandler is null");
        return;
    }
    eventHandler->RemoveTask(NODES_IN_RECT_QUERY_CALLBACK_TIMEOUT);
}

void UiReportStub::ReportWebUnfocusEvent(int64_t accessibilityId, const std::string& data)
{
    if (unfocusEvent_ != nullptr) {
        unfocusEvent_(accessibilityId, data);
    }
}

void UiReportStub::ReportScrollEvent(const std::string& data)
{
    if (scrollEventCallback_ != nullptr) {
        scrollEventCallback_(data);
    }
}

void UiReportStub::ReportLifeCycleEvent(const std::string& data)
{
    if (lifeCycleEventCallback_ != nullptr) {
        lifeCycleEventCallback_(data);
    }
}

void UiReportStub::ReportSelectTextEvent(const std::string& data)
{
    if (selectTextEventCallback_ != nullptr) {
        selectTextEventCallback_(data);
    }
}

void UiReportStub::SendSpecifiedContentOffsets(const std::vector<std::pair<float, float>>& offsets)
{
    if (getSpecifiedContentOffsets_ != nullptr) {
        getSpecifiedContentOffsets_(offsets);
    }
}

void UiReportStub::SendBaseInfo(const std::string& data)
{
    if (sendBaseInfoCallback_ != nullptr) {
        sendBaseInfoCallback_(data);
    }
}

void UiReportStub::RegisterGetBaseInfoCallback(const EventCallback& eventCallback)
{
    sendBaseInfoCallback_ = std::move(eventCallback);
}

void UiReportStub::RegisterClickEventCallback(const EventCallback& eventCallback)
{
    clickEventCallback_ = std::move(eventCallback);
}

void UiReportStub::UnregisterGetInspectorTreeCallback()
{
    std::lock_guard<std::mutex> lock(inspectorTreeCallbackMutex_);
    inspectorTreeCallback_ = nullptr;
}

void UiReportStub::PostGetInspectorTreeCallbackRemoveTask(int32_t timeout)
{
    auto eventHandler = eventHandler_.lock();
    if (eventHandler == nullptr) {
        LOGE("PostGetInspectorTreeCallbackRemoveTask eventHandler is null");
        return;
    }
    wptr<UiReportStub> weakThis = this;
    bool postTaskSuccess = eventHandler->PostTask([weakThis]() {
        auto uiReportStub = weakThis.promote();
        if (uiReportStub == nullptr) {
            LOGE("PostGetInspectorTreeCallbackRemoveTask uiReportStub is null");
            return;
        }
        uiReportStub->HandleInspectorTreeCallbackTimeout();
    }, GET_INSPECTOR_TREE_CALLBACK_TIMEOUT, NormalizeTimeout(timeout, DEFAULT_INSPECTOR_TREE_CALLBACK_TIMEOUT_MS));
    if (!postTaskSuccess) {
        LOGW("Post GetInspectorTree timeout task failed");
        std::lock_guard<std::mutex> lock(inspectorTreeCallbackMutex_);
        inspectorTreeCallback_ = nullptr;
    }
}

void UiReportStub::PostPageTranslateCallbackRemoveTask(int32_t timeout)
{
    auto eventHandler = eventHandler_.lock();
    if (eventHandler == nullptr) {
        LOGE("PostPageTranslateCallbackRemoveTask eventHandler is null");
        return;
    }
    wptr<UiReportStub> weakThis = this;
    int64_t requestId = 0;
    {
        std::lock_guard<std::mutex> lock(pageTranslateCallbackMutex_);
        requestId = pageTranslateRequestId_;
    }
    bool postTaskSuccess = eventHandler->PostTask([weakThis, requestId]() {
        auto uiReportStub = weakThis.promote();
        if (uiReportStub == nullptr) {
            LOGE("PostPageTranslateCallbackRemoveTask uiReportStub is null");
            return;
        }
        uiReportStub->HandlePageTranslateCallbackTimeout(requestId);
    }, PAGE_TRANSLATE_CALLBACK_TIMEOUT, NormalizeTimeout(timeout, DEFAULT_PAGE_TRANSLATE_CALLBACK_TIMEOUT_MS));
    if (!postTaskSuccess) {
        LOGW("Post PageTranslate timeout task failed");
        std::lock_guard<std::mutex> lock(pageTranslateCallbackMutex_);
        ClearPageTranslateCallbackLocked();
    }
}

void UiReportStub::PostPageTranslateResultWatchdogTask(int32_t nodeId, int64_t version, int32_t timeout)
{
    auto eventHandler = eventHandler_.lock();
    if (eventHandler == nullptr) {
        LOGE("PostPageTranslateResultWatchdogTask eventHandler is null");
        return;
    }
    wptr<UiReportStub> weakThis = this;
    auto taskName = MakePageTranslateResultWatchdogTaskName(nodeId, version);
    bool postTaskSuccess = eventHandler->PostTask([weakThis, nodeId, version]() {
        auto uiReportStub = weakThis.promote();
        if (uiReportStub == nullptr) {
            LOGE("PostPageTranslateResultWatchdogTask uiReportStub is null");
            return;
        }
        uiReportStub->HandlePageTranslateResultWatchdogTimeout(nodeId, version);
    }, taskName, NormalizeTimeout(timeout, DEFAULT_PAGE_TRANSLATE_CALLBACK_TIMEOUT_MS));
    if (!postTaskSuccess) {
        LOGW("Post PageTranslate result watchdog failed, nodeId=%{public}d, version=%{public}" PRId64, nodeId, version);
    }
}

void UiReportStub::CancelPageTranslateResultWatchdogTask(int32_t nodeId, int64_t version)
{
    auto eventHandler = eventHandler_.lock();
    if (eventHandler == nullptr) {
        LOGE("CancelPageTranslateResultWatchdogTask eventHandler is null");
        return;
    }
    eventHandler->RemoveTask(MakePageTranslateResultWatchdogTaskName(nodeId, version));
}

bool UiReportStub::RegisterGetInspectorTreeCallback(
    const std::function<void(std::string, int32_t, bool)>& eventCallback)
{
    std::lock_guard<std::mutex> lock(inspectorTreeCallbackMutex_);
    if (inspectorTreeCallback_ != nullptr) {
        return false;
    }
    inspectorTreeCallback_ = eventCallback;
    return true;
}

bool UiReportStub::RegisterPageTranslateTextCallback(const PageTranslateTextCallback& eventCallback, bool isContinuous)
{
    std::lock_guard<std::mutex> lock(pageTranslateCallbackMutex_);
    if (pageTranslateTextCallback_ != nullptr) {
        return false;
    }
    ClearPageTranslateCallbackLocked();
    pageTranslateTextCallback_ = eventCallback;
    pageTranslateContinuous_ = isContinuous;
    ++pageTranslateRequestId_;
    return true;
}

void UiReportStub::RegisterPageTranslateTimeoutCallback(std::function<void()>&& timeoutCallback)
{
    std::lock_guard<std::mutex> lock(pageTranslateCallbackMutex_);
    pageTranslateTimeoutCallback_ = std::move(timeoutCallback);
}

void UiReportStub::UnregisterPageTranslateTextCallback()
{
    std::lock_guard<std::mutex> lock(pageTranslateCallbackMutex_);
    CancelPageTranslateCallbackTimeoutTaskLocked();
    ClearPageTranslateCallbackLocked();
}

void UiReportStub::FinishPageTranslateTextRequest()
{
    std::lock_guard<std::mutex> lock(pageTranslateCallbackMutex_);
    if (pageTranslateContinuous_) {
        return;
    }
    CancelPageTranslateCallbackTimeoutTaskLocked();
    ClearPageTranslateCallbackLocked();
}

void UiReportStub::HandleInspectorTreeCallbackTimeout()
{
    std::lock_guard<std::mutex> lock(inspectorTreeCallbackMutex_);
    LOGW("GetInspectorTree callback timeout, clear pending callback");
    inspectorTreeCallback_ = nullptr;
}

void UiReportStub::HandlePageTranslateCallbackTimeout(int64_t requestId)
{
    std::function<void()> timeoutCallback;
    {
        std::lock_guard<std::mutex> lock(pageTranslateCallbackMutex_);
        if (requestId != pageTranslateRequestId_) {
            LOGW("PageTranslate callback timeout ignored, requestId expired");
            return;
        }
        LOGW("PageTranslate callback timeout, clear pending callback");
        timeoutCallback = pageTranslateTimeoutCallback_;
        ClearPageTranslateCallbackLocked();
    }
    if (timeoutCallback) {
        timeoutCallback();
    }
}

bool UiReportStub::RegisterSendCommandFinishCallback(
    const SendCommandFinishCallback& finishCallback, uint64_t& requestId)
{
    requestId = 0;
    if (!finishCallback) {
        return false;
    }
    std::lock_guard<std::mutex> lock(sendCommandCallbackMutex_);
    if (sendCommandFinishCallback_ != nullptr) {
        return false;
    }
    do {
        sendCommandRequestId_ = ++g_nextCommandRequestId;
    } while (sendCommandRequestId_ == 0);
    sendCommandFinishCallback_ = finishCallback;
    requestId = sendCommandRequestId_;
    return true;
}

bool UiReportStub::PostSendCommandCallbackRemoveTask(uint64_t requestId, int32_t timeout)
{
    auto eventHandler = eventHandler_.lock();
    if (eventHandler == nullptr) {
        LOGW("PostSendCommandCallbackRemoveTask eventHandler is null");
        return false;
    }
    wptr<UiReportStub> weakThis = this;
    return eventHandler->PostTask(
        [weakThis, requestId]() {
            auto uiReportStub = weakThis.promote();
            if (uiReportStub == nullptr) {
                return;
            }
            uiReportStub->HandleSendCommandCallbackTimeout(requestId);
        },
        MakeSendCommandCallbackTimeoutTaskName(requestId),
        NormalizeTimeout(timeout, DEFAULT_SEND_COMMAND_CALLBACK_TIMEOUT_MS));
}

void UiReportStub::UnregisterSendCommandFinishCallback(uint64_t requestId)
{
    std::lock_guard<std::mutex> lock(sendCommandCallbackMutex_);
    if (requestId != sendCommandRequestId_) {
        return;
    }
    CancelSendCommandCallbackTimeoutTaskLocked(requestId);
    ClearSendCommandFinishCallbackLocked();
}

void UiReportStub::HandleSendCommandCallbackTimeout(uint64_t requestId)
{
    std::lock_guard<std::mutex> lock(sendCommandCallbackMutex_);
    if (requestId != sendCommandRequestId_) {
        return;
    }
    ClearSendCommandFinishCallbackLocked();
}

void UiReportStub::CancelSendCommandCallbackTimeoutTaskLocked(uint64_t requestId)
{
    auto eventHandler = eventHandler_.lock();
    if (eventHandler == nullptr) {
        return;
    }
    eventHandler->RemoveTask(MakeSendCommandCallbackTimeoutTaskName(requestId));
}

void UiReportStub::ClearSendCommandFinishCallbackLocked()
{
    sendCommandFinishCallback_ = nullptr;
    sendCommandRequestId_ = 0;
}

void UiReportStub::ReportCommandResult(uint64_t requestId, const std::string& resultJson)
{
    SendCommandFinishCallback finishCallback;
    {
        std::lock_guard<std::mutex> lock(sendCommandCallbackMutex_);
        if (requestId != sendCommandRequestId_ || sendCommandFinishCallback_ == nullptr) {
            return;
        }
        CancelSendCommandCallbackTimeoutTaskLocked(requestId);
        finishCallback.swap(sendCommandFinishCallback_);
        sendCommandRequestId_ = 0;
    }
    UI_SESSION_SCOPED_TRACE("SendCommand.Callback.%llu", static_cast<unsigned long long>(requestId));
    finishCallback(resultJson);
}

void UiReportStub::HandlePageTranslateResultWatchdogTimeout(int32_t nodeId, int64_t version)
{
    LOGW("PageTranslate result timeout, nodeId=%{public}d, version=%{public}" PRId64, nodeId, version);
    std::function<void()> timeoutCallback;
    {
        std::lock_guard<std::mutex> lock(pageTranslateCallbackMutex_);
        timeoutCallback = pageTranslateTimeoutCallback_;
        if (timeoutCallback) {
            ClearPageTranslateCallbackLocked();
        }
    }
    if (timeoutCallback) {
        timeoutCallback();
    }
}

void UiReportStub::SetEventHandler(std::shared_ptr<AppExecFwk::EventHandler> eventHandler)
{
    eventHandler_ = eventHandler;
}

void UiReportStub::CancelGetInspectorTreeCallbackTimeoutTaskLocked()
{
    auto eventHandler = eventHandler_.lock();
    if (eventHandler == nullptr) {
        LOGE("CancelGetInspectorTreeCallbackTimeoutTaskLocked eventHandler is null");
        return;
    }
    eventHandler->RemoveTask(GET_INSPECTOR_TREE_CALLBACK_TIMEOUT);
}

void UiReportStub::ClearPageTranslateCallbackLocked()
{
    pageTranslateTextCallback_ = nullptr;
    pageTranslateTimeoutCallback_ = nullptr;
    pageTranslateContinuous_ = false;
}

void UiReportStub::CancelPageTranslateCallbackTimeoutTaskLocked()
{
    auto eventHandler = eventHandler_.lock();
    if (eventHandler == nullptr) {
        LOGE("CancelPageTranslateCallbackTimeoutTaskLocked eventHandler is null");
        return;
    }
    eventHandler->RemoveTask(PAGE_TRANSLATE_CALLBACK_TIMEOUT);
}

void UiReportStub::RegisterRouterChangeEventCallback(const EventCallback& eventCallback)
{
    RouterChangeEventCallback_ = std::move(eventCallback);
}

void UiReportStub::RegisterSearchEventCallback(const EventCallback& eventCallback)
{
    searchEventCallback_ = std::move(eventCallback);
}

void UiReportStub::RegisterTextChangeEventCallback(const EventCallback& eventCallback)
{
    textChangeEventCallback_ = std::move(eventCallback);
}

void UiReportStub::RegisterComponentChangeEventCallback(const EventCallback& eventCallback)
{
    ComponentChangeEventCallback_ = std::move(eventCallback);
}

bool UiReportStub::IsComponentChangeEventCallbackRegistered() const
{
    return ComponentChangeEventCallback_ != nullptr;
}

void UiReportStub::RegisterWebUnfocusEventCallback(
    const std::function<void(int64_t accessibilityId, const std::string& data)>& eventCallback)
{
    unfocusEvent_ = std::move(eventCallback);
}

void UiReportStub::RegisterScrollEventCallback(const EventCallback& eventCallback)
{
    scrollEventCallback_ = std::move(eventCallback);
}

void UiReportStub::RegisterLifeCycleEventCallback(const EventCallback& eventCallback)
{
    lifeCycleEventCallback_ = std::move(eventCallback);
}

void UiReportStub::RegisterSelectTextEventCallback(const EventCallback& eventCallback)
{
    selectTextEventCallback_ = std::move(eventCallback);
}

void UiReportStub::RegisterGetSpecifiedContentOffsets(
    const std::function<void(std::vector<std::pair<float, float>>)>& eventCallback)
{
    getSpecifiedContentOffsets_ = std::move(eventCallback);
}

void UiReportStub::RegisterGetWebViewCurrentLanguage(const EventCallback& eventCallback)
{
    getWebViewCurrentLanguageCallback_ = std::move(eventCallback);
}

void UiReportStub::RegisterGetCurrentPageName(const EventCallback& eventCallback)
{
    getCurrentPageNameCallback_ = std::move(eventCallback);
}

void UiReportStub::RegisterGetTranslateTextCallback(const std::function<void(int32_t, std::string)>& eventCallback)
{
    getTranslateTextCallback_ = std::move(eventCallback);
}

void UiReportStub::UnregisterClickEventCallback()
{
    clickEventCallback_ = nullptr;
}

void UiReportStub::UnregisterSearchEventCallback()
{
    searchEventCallback_ = nullptr;
}

void UiReportStub::UnregisterTextChangeEventCallback()
{
    textChangeEventCallback_ = nullptr;
}

void UiReportStub::UnregisterRouterChangeEventCallback()
{
    RouterChangeEventCallback_ = nullptr;
}

void UiReportStub::UnregisterComponentChangeEventCallback()
{
    ComponentChangeEventCallback_ = nullptr;
}

void UiReportStub::UnregisterWebUnfocusEventCallback()
{
    unfocusEvent_ = nullptr;
}

void UiReportStub::UnregisterScrollEventCallback()
{
    scrollEventCallback_ = nullptr;
}

void UiReportStub::UnregisterLifeCycleEventCallback()
{
    lifeCycleEventCallback_ = nullptr;
}

void UiReportStub::UnregisterSelectTextEventCallback()
{
    selectTextEventCallback_ = nullptr;
}

void UiReportStub::SendCurrentLanguage(const std::string& data)
{
    if (getWebViewCurrentLanguageCallback_) {
        getWebViewCurrentLanguageCallback_(data);
    }
}

void UiReportStub::SendCurrentPageName(const std::string& result)
{
    if (getCurrentPageNameCallback_) {
        getCurrentPageNameCallback_(result);
    }
}

void UiReportStub::SendWebText(int32_t nodeId, std::string res)
{
    if (getTranslateTextCallback_) {
        getTranslateTextCallback_(nodeId, res);
    }
}

void UiReportStub::SendPageText(int32_t nodeId, const std::string& text, int64_t version)
{
    PageTranslateTextCallback pageTranslateTextCallback;
    {
        std::lock_guard<std::mutex> lock(pageTranslateCallbackMutex_);
        if (pageTranslateTextCallback_ == nullptr) {
            return;
        }
        pageTranslateTextCallback = pageTranslateTextCallback_;
        if (nodeId >= 0) {
            CancelPageTranslateCallbackTimeoutTaskLocked();
        }
    }
    pageTranslateTextCallback(nodeId, text, version);
    if (nodeId >= 0) {
        PostPageTranslateResultWatchdogTask(nodeId, version);
    }
}

void UiReportStub::RegisterGetShowingImageCallback(
    const std::function<void(std::vector<std::pair<int32_t, std::shared_ptr<Media::PixelMap>>>)>& eventCallback)
{
    getShowingImageCallback_ = std::move(eventCallback);
}

void UiReportStub::RegisterGetImagesByIdCallback(
    const std::function<void(int32_t, const std::unordered_map<int32_t, std::shared_ptr<Media::PixelMap>>&,
        MultiImageQueryErrorCode)>& arkUIfinishCallback,
    const std::function<void(int32_t,
        const std::map<int32_t, std::map<int32_t, std::shared_ptr<Media::PixelMap>>>&,
        MultiImageQueryErrorCode)>& arkWebfinishCallback)
{
    getImagesByIdArkUIFinishCallback_ = arkUIfinishCallback;
    getImagesByIdArkWebFinishCallback_ = arkWebfinishCallback;
}

void UiReportStub::SendShowingImage(std::vector<std::pair<int32_t, std::shared_ptr<Media::PixelMap>>> maps)
{
    if (getShowingImageCallback_) {
        getShowingImageCallback_(maps);
    }
}

void UiReportStub::SendArkUIImagesById(int32_t windowId,
    const std::unordered_map<int32_t, std::shared_ptr<Media::PixelMap>>& componentImages,
    MultiImageQueryErrorCode arkUIErrorCode)
{
    if (getImagesByIdArkUIFinishCallback_) {
        getImagesByIdArkUIFinishCallback_(windowId, componentImages, arkUIErrorCode);
    }
}

void UiReportStub::SendArkWebImagesById(int32_t windowId, const std::map<int32_t, std::map<int32_t,
    std::shared_ptr<Media::PixelMap>>>& webImages, MultiImageQueryErrorCode arkWebErrorCode)
{
    if (getImagesByIdArkWebFinishCallback_) {
        getImagesByIdArkWebFinishCallback_(windowId, webImages, arkWebErrorCode);
    }
}

void UiReportStub::RegisterExeAppAIFunction(const std::function<void(uint32_t, std::string)>& finishCallback)
{
    exeAppAIFunctionCallback_ = std::move(finishCallback);
}

void UiReportStub::SendExeAppAIFunctionResult(uint32_t result, const std::string& data)
{
    if (exeAppAIFunctionCallback_) {
        exeAppAIFunctionCallback_(result, data);
    }
}

void UiReportStub::RegisterContentChangeCallback(
    const std::function<void(ChangeType type, const std::string& simpleTree)> callback)
{
    contentChangeCallback_ = callback;
}

bool UiReportStub::IsContentChangeCallbackRegistered() const
{
    return contentChangeCallback_ != nullptr;
}

void UiReportStub::UnregisterContentChangeCallback()
{
    contentChangeCallback_ = nullptr;
}

void UiReportStub::SendContentChange(ChangeType type, const std::string& simpleTree)
{
    if (contentChangeCallback_) {
        contentChangeCallback_(type, simpleTree);
    } else {
        LOGW("SendContentChange contentChangeCallback_ is null");
    }
}

void UiReportStub::RegisterGetStateMgmtInfoCallback(const std::function<void(std::vector<std::string>)>& eventCallback)
{
    getStateMgmtInfoCallback_ = std::move(eventCallback);
}

void UiReportStub::ReportGetStateMgmtInfo(std::vector<std::string> results)
{
    if (getStateMgmtInfoCallback_) {
        getStateMgmtInfoCallback_(results);
    }
}

bool UiReportStub::RegisterPageSceneEventCallback(
    const std::string& ruleSetId, const PageSceneEventCallback& eventCallback)
{
    return TryRegisterPageSceneEventCallback(ruleSetId, eventCallback);
}

bool UiReportStub::TryRegisterPageSceneEventCallback(
    const std::string& ruleSetId, const PageSceneEventCallback& eventCallback)
{
    if (ruleSetId.empty() || eventCallback == nullptr) {
        LOGW("TryRegisterPageSceneEventCallback invalid ruleSetId or callback");
        return false;
    }
    std::lock_guard<std::mutex> lock(pageSceneCallbackMutex_);
    return pageSceneEventCallbacks_.emplace(ruleSetId, eventCallback).second;
}

void UiReportStub::UnregisterPageSceneEventCallback(const std::string& ruleSetId)
{
    if (ruleSetId.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lock(pageSceneCallbackMutex_);
    pageSceneEventCallbacks_.erase(ruleSetId);
}

bool UiReportStub::RegisterGetPageSceneCallback(const PageSceneEventCallback& eventCallback)
{
    std::lock_guard<std::mutex> lock(pageSceneCallbackMutex_);
    getPageSceneCallback_ = eventCallback;
    return true;
}

void UiReportStub::UnregisterGetPageSceneCallback()
{
    std::lock_guard<std::mutex> lock(pageSceneCallbackMutex_);
    getPageSceneCallback_ = nullptr;
}

void UiReportStub::ReportPageSceneEvent(const std::string& sceneJson, bool isGetResult)
{
    PageSceneEventCallback getPageSceneCallback;
    {
        std::lock_guard<std::mutex> lock(pageSceneCallbackMutex_);
        getPageSceneCallback = getPageSceneCallback_;
    }
    if (isGetResult) {
        if (getPageSceneCallback) {
            getPageSceneCallback(sceneJson);
        }
        return;
    }
    // Snapshot callbacks, then filter and invoke outside the lock. Each callback
    // receives only its own ruleSet's hits/exits; shared event metadata is preserved.
    auto ruleSetIds = ExtractPageSceneRuleSetIds(sceneJson);
    std::vector<std::pair<std::string, PageSceneEventCallback>> callbackSnapshot;
    {
        std::lock_guard<std::mutex> lock(pageSceneCallbackMutex_);
        for (const auto& ruleSetId : ruleSetIds) {
            auto iter = pageSceneEventCallbacks_.find(ruleSetId);
            if (iter != pageSceneEventCallbacks_.end()) {
                callbackSnapshot.emplace_back(ruleSetId, iter->second);
            }
        }
    }
    for (const auto& [ruleSetId, callback] : callbackSnapshot) {
        if (callback) {
            auto filteredJson = FilterPageSceneEvent(sceneJson, ruleSetId);
            if (!filteredJson.empty()) {
                callback(filteredJson);
            }
        }
    }
}
} // namespace OHOS::Ace
