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

#include "ui_info_collection.h"

#include <algorithm>
#include <array>
#include <memory>
#include <mutex>
#include <new>

#include "node/node_model.h"
#include "pixelmap_native_impl.h"

#include "base/error/error_code.h"

namespace {
constexpr size_t MAX_IMAGE_COLLECTION_COUNT = 20;

struct ImageCollectionItem {
    int32_t nodeId = 0;
    std::unique_ptr<OH_PixelmapNative> pixelmap;
    ArkUI_ErrorCode error = ARKUI_ERROR_CODE_NO_ERROR;
    bool taken = false;
};
} // namespace

struct OH_ArkUI_NativeModule_ImageCollection {
    std::array<ImageCollectionItem, MAX_IMAGE_COLLECTION_COUNT> items;
    size_t count = 0;
};

namespace {
struct ImageCollectionRequest {
    int32_t contextId = 0;
    std::array<int32_t, MAX_IMAGE_COLLECTION_COUNT> nodeIds {};
    size_t count = 0;
    OH_ArkUI_NativeModule_ImageCollectionCallback callback = nullptr;
    void* userData = nullptr;
    const ArkUISnapshotAPI* snapshot = nullptr;
    ImageCollectionRequest* next = nullptr;
};

// Intrusive registration avoids a second allocation after accepting a request.
std::mutex g_activeRequestMutex;
ImageCollectionRequest* g_activeRequests = nullptr;

bool AddActiveRequest(ImageCollectionRequest* request)
{
    std::lock_guard<std::mutex> lock(g_activeRequestMutex);
    for (auto* active = g_activeRequests; active; active = active->next) {
        if (active->contextId == request->contextId) {
            return false;
        }
    }
    request->next = g_activeRequests;
    g_activeRequests = request;
    return true;
}

void RemoveActiveRequest(ImageCollectionRequest* request)
{
    std::lock_guard<std::mutex> lock(g_activeRequestMutex);
    auto** current = &g_activeRequests;
    while (*current && *current != request) {
        current = &(*current)->next;
    }
    if (*current) {
        *current = request->next;
    }
}

ArkUI_ErrorCode CollectImage(const ArkUISnapshotAPI& snapshot, ImageCollectionItem& item)
{
    std::shared_ptr<OHOS::Media::PixelMap> pixelmap;
    auto result = snapshot.getImageByNodeId(item.nodeId, &pixelmap);
    switch (result) {
        case OHOS::Ace::ERROR_CODE_NO_ERROR:
            if (!pixelmap) {
                return ARKUI_ERROR_CODE_INTERNAL_ERROR;
            }
            item.pixelmap.reset(new (std::nothrow) OH_PixelmapNative(std::move(pixelmap)));
            return item.pixelmap ? ARKUI_ERROR_CODE_NO_ERROR : ARKUI_ERROR_CODE_RESOURCE_EXHAUSTED;
        case ARKUI_ERROR_CODE_NODE_NOT_FOUND:
            return ARKUI_ERROR_CODE_NODE_NOT_FOUND;
        case ARKUI_ERROR_CODE_RESOURCE_EXHAUSTED:
            return ARKUI_ERROR_CODE_RESOURCE_EXHAUSTED;
        case OHOS::Ace::ERROR_CODE_COMPONENT_SNAPSHOT_TIMEOUT:
            return ARKUI_ERROR_CODE_COMPONENT_SNAPSHOT_TIMEOUT;
        case OHOS::Ace::ERROR_CODE_COMPONENT_SNAPSHOT_MODE_NOT_SUPPORTED:
            return ARKUI_ERROR_CODE_COMPONENT_SNAPSHOT_MODE_NOT_SUPPORTED;
        case OHOS::Ace::ERROR_CODE_COMPONENT_SNAPSHOT_AUTO_NOT_SUPPORTED:
            return ARKUI_ERROR_CODE_COMPONENT_SNAPSHOT_AUTO_NOT_SUPPORTED;
        default:
            return ARKUI_ERROR_CODE_INTERNAL_ERROR;
    }
}

void CollectImageItems(const ImageCollectionRequest& request,
    OH_ArkUI_NativeModule_ImageCollection& collection, ArkUI_ErrorCode& overallError)
{
    collection.count = request.count;
    for (size_t index = 0; index < request.count; ++index) {
        auto& item = collection.items[index];
        item.nodeId = request.nodeIds[index];
        item.error = CollectImage(*request.snapshot, item);
        if (item.error != ARKUI_ERROR_CODE_NO_ERROR) {
            overallError = ARKUI_ERROR_CODE_COLLECTION_PARTIAL_FAILED;
        }
    }
}

void CollectImages(void* data, int32_t contextError)
{
    std::unique_ptr<ImageCollectionRequest> request(static_cast<ImageCollectionRequest*>(data));
    ArkUI_ErrorCode overallError = static_cast<ArkUI_ErrorCode>(contextError);
    std::unique_ptr<OH_ArkUI_NativeModule_ImageCollection> collection;
    if (overallError == ARKUI_ERROR_CODE_NO_ERROR) {
        collection.reset(new (std::nothrow) OH_ArkUI_NativeModule_ImageCollection());
        if (!collection) {
            overallError = ARKUI_ERROR_CODE_RESOURCE_EXHAUSTED;
        } else {
            CollectImageItems(*request, *collection, overallError);
        }
    }
    // Ownership passes to the caller even for partial failure. Do not touch the
    // collection after invoking user code: it may be consumed/destroyed there.
    request->callback(overallError, collection.release(), request->userData);
    // Keep the Context occupied during user code, including callback reentry.
    RemoveActiveRequest(request.get());
}
} // namespace

extern "C" {
ArkUI_ErrorCode OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(ArkUI_ContextHandle context, const int32_t* nodeIds,
    size_t count, OH_ArkUI_NativeModule_ImageCollectionCallback callback, void* userData)
{
    if (!context) {
        return ARKUI_ERROR_CODE_UI_CONTEXT_INVALID;
    }
    if (!callback) {
        return ARKUI_ERROR_CODE_CALLBACK_INVALID;
    }
    if (count < 1 || count > MAX_IMAGE_COLLECTION_COUNT) {
        return ARKUI_ERROR_CODE_PARAM_OUT_OF_RANGE;
    }
    if (!nodeIds) {
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    for (size_t index = 0; index < count; ++index) {
        if (std::find(nodeIds, nodeIds + index, nodeIds[index]) != nodeIds + index) {
            return ARKUI_ERROR_CODE_DUPLICATE_NODE_ID;
        }
    }
    const auto* impl = OHOS::Ace::NodeModel::GetFullImpl();
    const auto* snapshot = impl && impl->getSnapshotAPI ? impl->getSnapshotAPI() : nullptr;
    if (!snapshot || !snapshot->postImageCollectionTask || !snapshot->getImageByNodeId) {
        return ARKUI_ERROR_CODE_UI_CONTEXT_INVALID;
    }
    auto request = std::make_unique<ImageCollectionRequest>();
    request->contextId = context->id;
    std::copy_n(nodeIds, count, request->nodeIds.begin());
    request->count = count;
    request->callback = callback;
    request->userData = userData;
    request->snapshot = snapshot;
    if (!AddActiveRequest(request.get())) {
        return ARKUI_ERROR_CODE_COMMAND_UNFINISHED;
    }
    // A different UI thread can execute the task before PostTask returns.
    // Transfer the request first; a rejected post never invokes the task.
    auto* accepted = request.release();
    auto result = snapshot->postImageCollectionTask(accepted->contextId, accepted, CollectImages);
    if (result != ARKUI_ERROR_CODE_NO_ERROR) {
        RemoveActiveRequest(accepted);
        delete accepted;
        return static_cast<ArkUI_ErrorCode>(result);
    }
    return ARKUI_ERROR_CODE_NO_ERROR;
}

ArkUI_ErrorCode OH_ArkUI_NativeModule_ImageCollectionTakeItemByNodeId(
    OH_ArkUI_NativeModule_ImageCollection* collection, int32_t nodeId,
    OH_PixelmapNative** outPixelmap, ArkUI_ErrorCode* outItemError)
{
    if (!collection || !outPixelmap || !outItemError) {
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    auto end = collection->items.begin() + collection->count;
    auto item = std::find_if(collection->items.begin(), end,
        [nodeId](const ImageCollectionItem& value) { return value.nodeId == nodeId; });
    if (item == end) {
        return ARKUI_ERROR_CODE_COLLECTION_ITEM_NOT_FOUND;
    }
    if (item->taken) {
        return ARKUI_ERROR_CODE_COLLECTION_ITEM_ALREADY_TAKEN;
    }
    item->taken = true;
    *outPixelmap = item->pixelmap.release();
    *outItemError = item->error;
    return ARKUI_ERROR_CODE_NO_ERROR;
}

void OH_ArkUI_NativeModule_ImageCollectionDestroy(OH_ArkUI_NativeModule_ImageCollection* collection)
{
    // Caller serializes collection operations. unique_ptr destroys only items
    // whose ownership has not already been transferred by TakeItem.
    delete collection;
}
} // extern "C"
