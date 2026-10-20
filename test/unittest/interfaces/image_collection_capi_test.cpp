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
#include <map>
#include <memory>

#include "gtest/gtest.h"

#include "interfaces/native/ui_info_collection.h"
#include "interfaces/native/node/node_model.h"
#include "interfaces/native/native_type.h"
#include "pixelmap_native_impl.h"

namespace {
// Named test values keep the fixtures readable and satisfy the gate's
// G.CNS.02 no-magic-numbers check.
constexpr int32_t CONTEXT_ID = 17;
constexpr int32_t NODE_ID_A = 5;
constexpr int32_t NODE_ID_B = 7;
constexpr int32_t UNKNOWN_NODE_ID = 99;
constexpr size_t NODE_ID_COUNT = 2;
constexpr size_t MAX_NODE_ID_COUNT = 21;
// Sentinel distinct from every result the mock can produce, proving TakeItem
// writes its outputs. ArkUI_ErrorCode is an unscoped C enum, so a nonzero int
// cannot be assigned to it implicitly.
constexpr ArkUI_ErrorCode UNWRITTEN_ERROR = ARKUI_ERROR_CODE_INTERNAL_ERROR;

// Mock knobs shared with the stubbed ArkUISnapshotAPI. The NodeModel stubs
// defined below replace the real node_model.cpp symbols in this executable.
thread_local bool support = true;            // GetFullImpl() returns nullptr when false.
thread_local bool snapshotIncomplete = false; // Snapshot API with null collection callbacks.
thread_local int32_t postCode = 0;            // postImageCollectionTask return code; 0 accepts.
thread_local bool deferPost = false;          // Record the task instead of invoking it.
thread_local void (*pendingTask)(void*, int32_t) = nullptr;
thread_local void* pendingData = nullptr;
thread_local int32_t lastPostedContextId = -1;
thread_local std::map<int32_t, int32_t> nodeResults; // nodeId -> getImageByNodeId result code.

struct Collected {
    bool called = false;
    ArkUI_ErrorCode error = UNWRITTEN_ERROR;
    OH_ArkUI_NativeModule_ImageCollection* collection = nullptr;
    void* userData = nullptr;
};
thread_local Collected g_collected;

void OnCollection(ArkUI_ErrorCode error, OH_ArkUI_NativeModule_ImageCollection* collection, void* userData)
{
    g_collected.called = true;
    g_collected.error = error;
    g_collected.collection = collection;
    g_collected.userData = userData;
}

int32_t MockPostImageCollectionTask(int32_t contextId, void* data, void (*task)(void*, int32_t))
{
    lastPostedContextId = contextId;
    if (postCode) {
        // A rejected post never invokes the task.
        return postCode;
    }
    if (deferPost) {
        pendingTask = task;
        pendingData = data;
        return 0;
    }
    // Simulate the UI thread running the task immediately.
    task(data, 0);
    return 0;
}

int32_t MockGetImageByNodeId(int32_t nodeId, void* mediaPixel)
{
    auto& result = *static_cast<std::shared_ptr<OHOS::Media::PixelMap>*>(mediaPixel);
    result.reset();
    auto it = nodeResults.find(nodeId);
    if (it == nodeResults.end()) {
        return ARKUI_ERROR_CODE_NODE_NOT_FOUND;
    }
    if (it->second == ARKUI_ERROR_CODE_NO_ERROR) {
        result = std::make_shared<OHOS::Media::PixelMap>();
    }
    return it->second;
}

const ArkUISnapshotAPI* MockSnapshotAPI()
{
    static ArkUISnapshotAPI api {};
    api.postImageCollectionTask = snapshotIncomplete ? nullptr : MockPostImageCollectionTask;
    api.getImageByNodeId = snapshotIncomplete ? nullptr : MockGetImageByNodeId;
    return &api;
}
} // namespace

namespace OHOS::Ace::NodeModel {
ArkUIFullNodeAPI* GetFullImpl()
{
    static ArkUIFullNodeAPI impl {};
    impl.getSnapshotAPI = []() -> const ArkUISnapshotAPI* { return MockSnapshotAPI(); };
    return support ? &impl : nullptr;
}
} // namespace OHOS::Ace::NodeModel

class ImageCollectionCapiTest : public testing::Test {
public:
    void SetUp() override
    {
        support = true;
        snapshotIncomplete = false;
        postCode = 0;
        deferPost = false;
        pendingTask = nullptr;
        pendingData = nullptr;
        lastPostedContextId = -1;
        nodeResults.clear();
        g_collected = Collected {};
    }
};

TEST_F(ImageCollectionCapiTest, validationOrder)
{
    int32_t ids[] = { NODE_ID_A };
    ArkUI_Context context { CONTEXT_ID };
    // 190001 is ARKUI_ERROR_CODE_UI_CONTEXT_INVALID for the null UI context.
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(nullptr, ids, 1, OnCollection, nullptr),
        ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    // 190002 is ARKUI_ERROR_CODE_CALLBACK_INVALID for the null callback.
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, ids, 1, nullptr, nullptr),
        ARKUI_ERROR_CODE_CALLBACK_INVALID);
    // 106405 is ARKUI_ERROR_CODE_PARAM_OUT_OF_RANGE for an out-of-range count.
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, ids, 0, OnCollection, nullptr),
        ARKUI_ERROR_CODE_PARAM_OUT_OF_RANGE);
    int32_t manyIds[MAX_NODE_ID_COUNT] = {};
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, manyIds, MAX_NODE_ID_COUNT, OnCollection, nullptr),
        ARKUI_ERROR_CODE_PARAM_OUT_OF_RANGE);
    // 401 is ARKUI_ERROR_CODE_PARAM_INVALID for the null node IDs.
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, nullptr, 1, OnCollection, nullptr),
        ARKUI_ERROR_CODE_PARAM_INVALID);
    // 106413 is ARKUI_ERROR_CODE_DUPLICATE_NODE_ID for repeated node IDs.
    int32_t dup[] = { NODE_ID_A, NODE_ID_A };
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, dup, NODE_ID_COUNT, OnCollection, nullptr),
        ARKUI_ERROR_CODE_DUPLICATE_NODE_ID);
    // 190001 while the native implementation is unavailable.
    support = false;
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, ids, 1, OnCollection, nullptr),
        ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    // 190001 while the snapshot API lacks the collection callbacks.
    support = true;
    snapshotIncomplete = true;
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, ids, 1, OnCollection, nullptr),
        ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    EXPECT_FALSE(g_collected.called);
}

TEST_F(ImageCollectionCapiTest, successCollectionTakeAndDestroy)
{
    nodeResults[NODE_ID_A] = ARKUI_ERROR_CODE_NO_ERROR;
    nodeResults[NODE_ID_B] = ARKUI_ERROR_CODE_NO_ERROR;
    int32_t ids[] = { NODE_ID_A, NODE_ID_B };
    int32_t marker = 42;
    ArkUI_Context context { CONTEXT_ID };
    // The instance ID must be forwarded to the collection task layer.
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, ids, NODE_ID_COUNT, OnCollection, &marker), 0);
    EXPECT_EQ(lastPostedContextId, CONTEXT_ID);
    ASSERT_TRUE(g_collected.called);
    EXPECT_EQ(g_collected.error, ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(g_collected.collection, nullptr);
    // userData is returned unchanged.
    EXPECT_EQ(g_collected.userData, &marker);
    auto* collection = g_collected.collection;
    OH_PixelmapNative* pixelmap = nullptr;
    ArkUI_ErrorCode itemError = UNWRITTEN_ERROR;
    // 401 for a null collection or null output pointers.
    EXPECT_EQ(OH_ArkUI_NativeModule_ImageCollectionTakeItemByNodeId(nullptr, NODE_ID_A, &pixelmap, &itemError),
        ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_ArkUI_NativeModule_ImageCollectionTakeItemByNodeId(collection, NODE_ID_A, nullptr, &itemError),
        ARKUI_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_ArkUI_NativeModule_ImageCollectionTakeItemByNodeId(collection, NODE_ID_A, &pixelmap, nullptr),
        ARKUI_ERROR_CODE_PARAM_INVALID);
    // 106415 for a node ID that is not part of the collection.
    EXPECT_EQ(OH_ArkUI_NativeModule_ImageCollectionTakeItemByNodeId(collection, UNKNOWN_NODE_ID, &pixelmap, &itemError),
        ARKUI_ERROR_CODE_COLLECTION_ITEM_NOT_FOUND);
    EXPECT_EQ(OH_ArkUI_NativeModule_ImageCollectionTakeItemByNodeId(collection, NODE_ID_A, &pixelmap, &itemError), 0);
    EXPECT_NE(pixelmap, nullptr);
    EXPECT_EQ(itemError, ARKUI_ERROR_CODE_NO_ERROR);
    delete pixelmap;
    pixelmap = nullptr;
    // 106416 because the first node has already been consumed.
    EXPECT_EQ(OH_ArkUI_NativeModule_ImageCollectionTakeItemByNodeId(collection, NODE_ID_A, &pixelmap, &itemError),
        ARKUI_ERROR_CODE_COLLECTION_ITEM_ALREADY_TAKEN);
    EXPECT_EQ(OH_ArkUI_NativeModule_ImageCollectionTakeItemByNodeId(collection, NODE_ID_B, &pixelmap, &itemError), 0);
    EXPECT_NE(pixelmap, nullptr);
    EXPECT_EQ(itemError, ARKUI_ERROR_CODE_NO_ERROR);
    delete pixelmap;
    // Destroy releases the remaining items and tolerates a null collection.
    OH_ArkUI_NativeModule_ImageCollectionDestroy(collection);
    OH_ArkUI_NativeModule_ImageCollectionDestroy(nullptr);
}

TEST_F(ImageCollectionCapiTest, partialFailureStillDeliversCollection)
{
    nodeResults[NODE_ID_A] = ARKUI_ERROR_CODE_NO_ERROR;
    nodeResults[NODE_ID_B] = ARKUI_ERROR_CODE_NODE_NOT_FOUND;
    int32_t ids[] = { NODE_ID_A, NODE_ID_B };
    ArkUI_Context context { 1 };
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, ids, NODE_ID_COUNT, OnCollection, nullptr), 0);
    ASSERT_TRUE(g_collected.called);
    // 106414 is ARKUI_ERROR_CODE_COLLECTION_PARTIAL_FAILED; the collection is still delivered.
    EXPECT_EQ(g_collected.error, ARKUI_ERROR_CODE_COLLECTION_PARTIAL_FAILED);
    ASSERT_NE(g_collected.collection, nullptr);
    auto* collection = g_collected.collection;
    OH_PixelmapNative* pixelmap = reinterpret_cast<OH_PixelmapNative*>(0x1);
    ArkUI_ErrorCode itemError = UNWRITTEN_ERROR;
    EXPECT_EQ(OH_ArkUI_NativeModule_ImageCollectionTakeItemByNodeId(collection, NODE_ID_B, &pixelmap, &itemError), 0);
    // A failed item consumes without transferring a PixelMap.
    EXPECT_EQ(pixelmap, nullptr);
    EXPECT_EQ(itemError, ARKUI_ERROR_CODE_NODE_NOT_FOUND);
    EXPECT_EQ(OH_ArkUI_NativeModule_ImageCollectionTakeItemByNodeId(collection, NODE_ID_A, &pixelmap, &itemError), 0);
    EXPECT_NE(pixelmap, nullptr);
    EXPECT_EQ(itemError, ARKUI_ERROR_CODE_NO_ERROR);
    delete pixelmap;
    OH_ArkUI_NativeModule_ImageCollectionDestroy(collection);
}

TEST_F(ImageCollectionCapiTest, resourceExhaustedItem)
{
    nodeResults[NODE_ID_A] = ARKUI_ERROR_CODE_RESOURCE_EXHAUSTED;
    int32_t ids[] = { NODE_ID_A };
    ArkUI_Context context { 1 };
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, ids, 1, OnCollection, nullptr), 0);
    ASSERT_TRUE(g_collected.called);
    EXPECT_EQ(g_collected.error, ARKUI_ERROR_CODE_COLLECTION_PARTIAL_FAILED);
    ASSERT_NE(g_collected.collection, nullptr);
    auto* collection = g_collected.collection;
    OH_PixelmapNative* pixelmap = reinterpret_cast<OH_PixelmapNative*>(0x1);
    ArkUI_ErrorCode itemError = UNWRITTEN_ERROR;
    EXPECT_EQ(OH_ArkUI_NativeModule_ImageCollectionTakeItemByNodeId(collection, NODE_ID_A, &pixelmap, &itemError), 0);
    EXPECT_EQ(pixelmap, nullptr);
    EXPECT_EQ(itemError, ARKUI_ERROR_CODE_RESOURCE_EXHAUSTED);
    OH_ArkUI_NativeModule_ImageCollectionDestroy(collection);
}

TEST_F(ImageCollectionCapiTest, oneActiveRequestPerContext)
{
    deferPost = true;
    nodeResults[NODE_ID_A] = ARKUI_ERROR_CODE_NO_ERROR;
    int32_t ids[] = { NODE_ID_A };
    ArkUI_Context context { CONTEXT_ID };
    // The first request is accepted but its task is deferred, keeping the slot active.
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, ids, 1, OnCollection, nullptr), 0);
    EXPECT_FALSE(g_collected.called);
    // 106409 is ARKUI_ERROR_CODE_COMMAND_UNFINISHED for the active slot.
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, ids, 1, OnCollection, nullptr),
        ARKUI_ERROR_CODE_COMMAND_UNFINISHED);
    // Completing the deferred task runs the callback exactly once and frees the slot.
    ASSERT_NE(pendingTask, nullptr);
    pendingTask(pendingData, 0);
    ASSERT_TRUE(g_collected.called);
    EXPECT_EQ(g_collected.error, ARKUI_ERROR_CODE_NO_ERROR);
    ASSERT_NE(g_collected.collection, nullptr);
    OH_ArkUI_NativeModule_ImageCollectionDestroy(g_collected.collection);
    // The same context accepts a new request after the first callback returns.
    g_collected = Collected {};
    deferPost = false;
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, ids, 1, OnCollection, nullptr), 0);
    ASSERT_TRUE(g_collected.called);
    ASSERT_NE(g_collected.collection, nullptr);
    OH_ArkUI_NativeModule_ImageCollectionDestroy(g_collected.collection);
}

TEST_F(ImageCollectionCapiTest, postFailureCleansUpAndPropagatesCode)
{
    postCode = ARKUI_ERROR_CODE_UI_CONTEXT_INVALID;
    int32_t ids[] = { NODE_ID_A };
    ArkUI_Context context { CONTEXT_ID };
    // A rejected post propagates its code and never invokes the callback.
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, ids, 1, OnCollection, nullptr),
        ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    EXPECT_FALSE(g_collected.called);
    // The rejected request must not remain active.
    postCode = 0;
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, ids, 1, OnCollection, nullptr), 0);
    ASSERT_TRUE(g_collected.called);
    ASSERT_NE(g_collected.collection, nullptr);
    OH_ArkUI_NativeModule_ImageCollectionDestroy(g_collected.collection);
}

TEST_F(ImageCollectionCapiTest, contextInvalidationPassedToCallback)
{
    deferPost = true;
    nodeResults[NODE_ID_A] = ARKUI_ERROR_CODE_NO_ERROR;
    int32_t ids[] = { NODE_ID_A };
    ArkUI_Context context { CONTEXT_ID };
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, ids, 1, OnCollection, nullptr), 0);
    ASSERT_NE(pendingTask, nullptr);
    // The container becomes invalid before the task runs; no collection is produced.
    pendingTask(pendingData, ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    ASSERT_TRUE(g_collected.called);
    EXPECT_EQ(g_collected.error, ARKUI_ERROR_CODE_UI_CONTEXT_INVALID);
    EXPECT_EQ(g_collected.collection, nullptr);
    // The slot is freed after the invalidation callback.
    deferPost = false;
    EXPECT_EQ(OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(&context, ids, 1, OnCollection, nullptr), 0);
    ASSERT_TRUE(g_collected.called);
    ASSERT_NE(g_collected.collection, nullptr);
    OH_ArkUI_NativeModule_ImageCollectionDestroy(g_collected.collection);
}
