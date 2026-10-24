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

#include "node_model_safely.h"

#include <initializer_list>
#include <vector>
#include "node/config_manager.h"

#include "node_extened.h"
#include "node_model.h"

#include "interfaces/native/native_error_message_macros.h"
#include "base/error/error_code.h"
#include "base/log/log_wrapper.h"
#include "base/utils/utils.h"

#ifdef __cplusplus
extern "C" {
#endif
int32_t OH_ArkUI_PostAsyncUITask(ArkUI_ContextHandle context,
    void* asyncUITaskData, void (*asyncUITask)(void* asyncUITaskData), void(*onFinish)(void* asyncUITaskData))
{
    if (!context) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID,
            __FUNCTION__, "Context parameter is null");
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    if (!asyncUITask) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID,
            __FUNCTION__, "Async UI task callback is null");
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    auto* contextPtr = reinterpret_cast<ArkUI_Context*>(context);
    const auto* impl = OHOS::Ace::NodeModel::GetFullImpl();
    CHECK_NULL_RETURN_WITH_MESSAGE(impl, ARKUI_ERROR_CODE_PARAM_INVALID,
        __FUNCTION__, "Native module not initialized");
    auto result =
        impl->getMultiThreadManagerAPI()->postAsyncUITask(contextPtr->id, asyncUITaskData, asyncUITask, onFinish);
    if (result != ARKUI_ERROR_CODE_NO_ERROR) {
        SET_ERROR_MESSAGE(result, __FUNCTION__, "Failed to post async UI task");
    }
    return result;
}

int32_t OH_ArkUI_PostUITask(ArkUI_ContextHandle context, void* taskData, void(*task)(void* taskData))
{
    if (!context) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID,
            __FUNCTION__, "Context parameter is null");
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    if (!task) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID,
            __FUNCTION__, "UI task callback is null");
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    auto* contextPtr = reinterpret_cast<ArkUI_Context*>(context);
    const auto* impl = OHOS::Ace::NodeModel::GetFullImpl();
    CHECK_NULL_RETURN_WITH_MESSAGE(impl, ARKUI_ERROR_CODE_PARAM_INVALID,
        __FUNCTION__, "Native module not initialized");
    auto result = impl->getMultiThreadManagerAPI()->postUITask(contextPtr->id, taskData, task);
    if (result != ARKUI_ERROR_CODE_NO_ERROR) {
        SET_ERROR_MESSAGE(result, __FUNCTION__, "Failed to post UI task");
    }
    return result;
}

int32_t OH_ArkUI_PostUITaskAndWait(ArkUI_ContextHandle context, void* taskData, void(*task)(void* taskData))
{
    if (!context) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID,
            __FUNCTION__, "Context parameter is null");
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    if (!task) {
        SET_ERROR_MESSAGE(ARKUI_ERROR_CODE_PARAM_INVALID,
            __FUNCTION__, "UI task callback is null");
        return ARKUI_ERROR_CODE_PARAM_INVALID;
    }
    auto* contextPtr = reinterpret_cast<ArkUI_Context*>(context);
    const auto* impl = OHOS::Ace::NodeModel::GetFullImpl();
    CHECK_NULL_RETURN_WITH_MESSAGE(impl, ARKUI_ERROR_CODE_PARAM_INVALID,
        __FUNCTION__, "Native module not initialized");
    auto result = impl->getMultiThreadManagerAPI()->postUITaskAndWait(contextPtr->id, taskData, task);
    if (result != ARKUI_ERROR_CODE_NO_ERROR) {
        SET_ERROR_MESSAGE(result, __FUNCTION__, "Failed to post UI task and wait");
    }
    return result;
}
#ifdef __cplusplus
};
#endif

namespace OHOS::Ace::NodeModel {
std::mutex g_nodeSetMutex_;
std::set<ArkUI_NodeHandle> g_nodeSetSafely;

namespace {
thread_local uint32_t g_checkedScopeDepth = 0;

class NativeNodeOperationScope final {
public:
    NativeNodeOperationScope(std::initializer_list<ArkUI_NodeHandle> nodes, const char* apiName)
    {
        const auto* api = GetFullImpl()->getMultiThreadManagerAPI();
        if (!api->beginNodeTreeOperation || !api->needsNodeTreeOperationCheck ||
            !api->needsNodeTreeOperationCheck()) {
            return;
        }
        ArkUINodeHandle handles[3] = { nullptr, nullptr, nullptr };
        int32_t count = 0;
        for (auto node : nodes) {
            if (node) {
                handles[count++] = node->uiNodeHandle;
            }
        }
        void* snapshot = nullptr;
        {
            std::lock_guard<std::mutex> lock(g_nodeSetMutex_);
            std::vector<ArkUINodeHandle> published;
            published.reserve(g_nodeSetSafely.size());
            for (auto node : g_nodeSetSafely) {
                published.push_back(node->uiNodeHandle);
            }
            snapshot = api->captureNodeTreeOperationNodes(published.data(), published.size());
        }
        // The snapshot pins only published handles. Never traverse topology or
        // emit diagnostics while holding the Native handle-set mutex.
        operation_ = api->beginNodeTreeOperation(handles, count, apiName, snapshot);
        if (operation_) {
            ++g_checkedScopeDepth;
        }
    }

    ~NativeNodeOperationScope()
    {
        if (operation_) {
            --g_checkedScopeDepth;
            GetFullImpl()->getMultiThreadManagerAPI()->endNodeTreeOperation(operation_);
        }
    }

    void IncludeNode(ArkUI_NodeHandle node)
    {
        if (operation_ && node) {
            GetFullImpl()->getMultiThreadManagerAPI()->includeNodeTreeOperationResult(operation_, node->uiNodeHandle);
        }
    }

    bool IsChecking() const { return operation_ != nullptr; }

private:
    void* operation_ = nullptr;
};
} // namespace

class ThreadSafeNodeScope {
public:
    ThreadSafeNodeScope(std::initializer_list<ArkUI_NodeHandle> nodes, const char* apiName)
        : operation_(nodes, apiName)
    {
        const auto* api = GetFullImpl()->getMultiThreadManagerAPI();
        if (operation_.IsChecking() && api->isThreadSafeNodeScope) {
            previous_ = api->isThreadSafeNodeScope();
        }
        api->setIsThreadSafeNodeScope(true);
    }

    ~ThreadSafeNodeScope()
    {
        GetFullImpl()->getMultiThreadManagerAPI()->setIsThreadSafeNodeScope(previous_);
    }

    void IncludeNode(ArkUI_NodeHandle node) { operation_.IncludeNode(node); }

    bool IsChecking() const { return operation_.IsChecking(); }

private:
    NativeNodeOperationScope operation_;
    bool previous_ = false;
};

class MarkNodeTreeFreeScope {
public:
    MarkNodeTreeFreeScope()
    {
        const auto* api = GetFullImpl()->getMultiThreadManagerAPI();
        if (g_checkedScopeDepth && api->isNeedMarkNodeTreeFree) {
            previous_ = api->isNeedMarkNodeTreeFree();
        }
        api->setNeedMarkNodeTreeFree(true);
    }

    ~MarkNodeTreeFreeScope()
    {
        GetFullImpl()->getMultiThreadManagerAPI()->setNeedMarkNodeTreeFree(previous_);
    }

private:
    bool previous_ = false;
};


ArkUI_NodeHandle CreateNodeSafely(ArkUI_NodeType type)
{
    static const ArkUINodeType nodes[] = { ARKUI_CUSTOM, ARKUI_TEXT, ARKUI_SPAN, ARKUI_IMAGE_SPAN, ARKUI_IMAGE,
        ARKUI_TOGGLE, ARKUI_LOADING_PROGRESS, ARKUI_TEXT_INPUT, ARKUI_TEXTAREA, ARKUI_BUTTON, ARKUI_PROGRESS,
        ARKUI_CHECKBOX, ARKUI_XCOMPONENT, ARKUI_DATE_PICKER, ARKUI_TIME_PICKER, ARKUI_TEXT_PICKER,
        ARKUI_CALENDAR_PICKER, ARKUI_SLIDER, ARKUI_RADIO, ARKUI_IMAGE_ANIMATOR, ARKUI_XCOMPONENT_TEXTURE,
        ARKUI_CHECK_BOX_GROUP, ARKUI_RICH_EDITOR, ARKUI_ARC_ALPHABET_INDEXER, ARKUI_STACK, ARKUI_SWIPER,
        ARKUI_SCROLL, ARKUI_LIST, ARKUI_LIST_ITEM, ARKUI_LIST_ITEM_GROUP, ARKUI_COLUMN, ARKUI_ROW, ARKUI_FLEX,
        ARKUI_REFRESH, ARKUI_WATER_FLOW, ARKUI_FLOW_ITEM, ARKUI_RELATIVE_CONTAINER, ARKUI_GRID, ARKUI_GRID_ITEM,
        ARKUI_CUSTOM_SPAN, ARKUI_EMBEDDED_COMPONENT, ARKUI_UNDEFINED, ARKUI_PICKER, ARKUI_ARC_LIST,
        ARKUI_ARC_LIST_ITEM, ARKUI_ARC_SCROLL_BAR, ARKUI_ARC_SWIPER };
    // already check in entry point.
    uint32_t nodeType = type < MAX_NODE_SCOPE_NUM ? type : (type - MAX_NODE_SCOPE_NUM + BASIC_COMPONENT_NUM);
    const auto* impl = GetFullImpl();
    if (!impl) {
        TAG_LOGE(AceLogTag::ACE_NATIVE_NODE, "Native node implementation is not initialized");
        return nullptr;
    }
    if (nodeType >= sizeof(nodes) / sizeof(ArkUINodeType) || nodes[nodeType] == ARKUI_UNDEFINED) {
        TAG_LOGE(AceLogTag::ACE_NATIVE_NODE, "node type: %{public}d NOT IMPLEMENT", type);
        return nullptr;
    }

    ArkUI_Int32 id = ARKUI_AUTO_GENERATE_NODE_ID;
    ThreadSafeNodeScope threadSafeNodeScope({}, __FUNCTION__);
    auto* uiNode = impl->getBasicAPI()->createNode(nodes[nodeType], id, ARKUI_NODE_FLAG_C);
    if (!uiNode) {
        TAG_LOGE(AceLogTag::ACE_NATIVE_NODE, "node type: %{public}d can not find in full impl", type);
        return nullptr;
    }
    impl->getBasicAPI()->markDirty(uiNode, ARKUI_DIRTY_FLAG_ATTRIBUTE_DIFF);
    ArkUI_Node* arkUINode = new ArkUI_Node({ .type = type, .uiNodeHandle = uiNode, .cNode = true,
        .threadSafeNode = true, .magic = ARKUI_NODE_MAGIC_VALID });
    impl->getExtendedAPI()->setAttachNodePtr(uiNode, reinterpret_cast<void*>(arkUINode));
    {
        std::lock_guard<std::mutex> lock(g_nodeSetMutex_);
        g_nodeSetSafely.emplace(arkUINode);
    }
    threadSafeNodeScope.IncludeNode(arkUINode);
    return arkUINode;
}

void DisposeNodeSafely(ArkUI_NodeHandle nativePtr)
{
    CHECK_NULL_VOID(nativePtr);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(nativePtr, "Node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nativePtr->uiNodeHandle)) {
        return;
    }
    if (!(nativePtr->threadSafeNode)) {
        DisposeNode(nativePtr);
        return;
    }
    if (!CheckIsCNode(nativePtr)) {
        return;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ nativePtr }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nativePtr->uiNodeHandle)) {
        return;
    }
    {
        // Unpublish before the last engine reference can be released. A mode
        // enabled during disposal must not snapshot a stale uiNodeHandle.
        std::lock_guard<std::mutex> lock(g_nodeSetMutex_);
        g_nodeSetSafely.erase(nativePtr);
    }
    impl->getBasicAPI()->disposeNode(nativePtr->uiNodeHandle);
    DisposeNativeSource(nativePtr);
    // Prevent the invalidation store from being optimized away before deallocation.
    volatile uint32_t* magic = &nativePtr->magic;
    *magic = ARKUI_NODE_MAGIC_INVALID;
    delete nativePtr;
    nativePtr = nullptr;
}

int32_t AddChildSafely(ArkUI_NodeHandle parentNode, ArkUI_NodeHandle childNode)
{
    CHECK_NULL_RETURN(parentNode, ERROR_CODE_PARAM_INVALID);
    CHECK_NULL_RETURN(childNode, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(parentNode, "Parent node has been disposed");
    CHECK_NODE_DISPOSED(childNode, "Child node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(parentNode->uiNodeHandle) ||
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(childNode->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ parentNode, childNode }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(parentNode->uiNodeHandle) ||
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(childNode->uiNodeHandle))) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    return AddChild(parentNode, childNode);
}

int32_t RemoveChildSafely(ArkUI_NodeHandle parentNode, ArkUI_NodeHandle childNode)
{
    CHECK_NULL_RETURN(parentNode, ERROR_CODE_PARAM_INVALID);
    CHECK_NULL_RETURN(childNode, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(parentNode, "Parent node has been disposed");
    CHECK_NODE_DISPOSED(childNode, "Child node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(parentNode->uiNodeHandle) ||
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(childNode->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ parentNode, childNode }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(parentNode->uiNodeHandle) ||
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(childNode->uiNodeHandle))) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    MarkNodeTreeFreeScope markNodeTreeFreeScope;
    return RemoveChild(parentNode, childNode);
}

int32_t InsertChildAfterSafely(ArkUI_NodeHandle parentNode, ArkUI_NodeHandle childNode, ArkUI_NodeHandle siblingNode)
{
    CHECK_NULL_RETURN(parentNode, ERROR_CODE_PARAM_INVALID);
    CHECK_NULL_RETURN(childNode, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(parentNode, "Parent node has been disposed");
    CHECK_NODE_DISPOSED(childNode, "Child node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(parentNode->uiNodeHandle) ||
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(childNode->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    CHECK_NODE_DISPOSED(siblingNode, "Sibling node has been disposed");
    ThreadSafeNodeScope threadSafeNodeScope({ parentNode, childNode, siblingNode }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(parentNode->uiNodeHandle) ||
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(childNode->uiNodeHandle))) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    return InsertChildAfter(parentNode, childNode, siblingNode);
}

int32_t InsertChildBeforeSafely(ArkUI_NodeHandle parentNode, ArkUI_NodeHandle childNode, ArkUI_NodeHandle siblingNode)
{
    CHECK_NULL_RETURN(parentNode, ERROR_CODE_PARAM_INVALID);
    CHECK_NULL_RETURN(childNode, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(parentNode, "Parent node has been disposed");
    CHECK_NODE_DISPOSED(childNode, "Child node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(parentNode->uiNodeHandle) ||
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(childNode->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    CHECK_NODE_DISPOSED(siblingNode, "Sibling node has been disposed");
    ThreadSafeNodeScope threadSafeNodeScope({ parentNode, childNode, siblingNode }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(parentNode->uiNodeHandle) ||
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(childNode->uiNodeHandle))) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    return InsertChildBefore(parentNode, childNode, siblingNode);
}

int32_t InsertChildAtSafely(ArkUI_NodeHandle parentNode, ArkUI_NodeHandle childNode, int32_t position)
{
    CHECK_NULL_RETURN(parentNode, ERROR_CODE_PARAM_INVALID);
    CHECK_NULL_RETURN(childNode, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(parentNode, "Parent node has been disposed");
    CHECK_NODE_DISPOSED(childNode, "Child node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(parentNode->uiNodeHandle) ||
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(childNode->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ parentNode, childNode }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(parentNode->uiNodeHandle) ||
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(childNode->uiNodeHandle))) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    return InsertChildAt(parentNode, childNode, position);
}

int32_t SetAttributeSafely(ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute, const ArkUI_AttributeItem* value)
{
    CHECK_NULL_RETURN(node, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(node, "Node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ node }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    return SetAttribute(node, attribute, value);
}

int32_t ResetAttributeSafely(ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute)
{
    CHECK_NULL_RETURN(node, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(node, "Node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ node }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    return ResetAttribute(node, attribute);
}

const ArkUI_AttributeItem* GetAttributeSafely(ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute)
{
    CHECK_NULL_RETURN(node, nullptr);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(node, "Node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return nullptr;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ node }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return nullptr;
    }
    return GetAttribute(node, attribute);
}

int32_t RegisterNodeEventSafely(ArkUI_NodeHandle nodePtr,
    ArkUI_NodeEventType eventType, int32_t targetId, void* userData)
{
    CHECK_NULL_RETURN(nodePtr, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nodePtr->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ nodePtr }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nodePtr->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    return RegisterNodeEvent(nodePtr, eventType, targetId, userData);
}

void UnregisterNodeEventSafely(ArkUI_NodeHandle nodePtr, ArkUI_NodeEventType eventType)
{
    CHECK_NULL_VOID(nodePtr);
    auto* impl = GetFullImpl();
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nodePtr->uiNodeHandle)) {
        return;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ nodePtr }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nodePtr->uiNodeHandle)) {
        return;
    }
    UnregisterNodeEvent(nodePtr, eventType);
}

void RegisterOnEventSafely(void (*eventReceiver)(ArkUI_NodeEvent* event))
{
    auto* impl = GetFullImpl();
    if (impl->getMultiThreadManagerAPI()->checkOnUIThread()) {
        RegisterOnEvent(eventReceiver);
    }
}

void UnregisterOnEventSafely()
{
    auto* impl = GetFullImpl();
    if (impl->getMultiThreadManagerAPI()->checkOnUIThread()) {
        UnregisterOnEvent();
    }
}

int32_t SetUserDataSafely(ArkUI_NodeHandle node, void* userData)
{
    CHECK_NULL_RETURN(node, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(node, "Node has been disposed");
    if (impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        NativeNodeOperationScope treeOperation({ node }, __FUNCTION__);
        if (treeOperation.IsChecking() &&
            !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
            return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
        }
        return SetUserData(node, userData);
    }
    return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
}

void* GetUserDataSafely(ArkUI_NodeHandle node)
{
    CHECK_NULL_RETURN(node, nullptr);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(node, "Node has been disposed");
    if (impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        NativeNodeOperationScope treeOperation({ node }, __FUNCTION__);
        if (treeOperation.IsChecking() &&
            !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
            return nullptr;
        }
        return GetUserData(node);
    }
    return nullptr;
}

int32_t SetLengthMetricUnitSafely(ArkUI_NodeHandle nodePtr, ArkUI_LengthMetricUnit unit)
{
    CHECK_NULL_RETURN(nodePtr, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(nodePtr, "Node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nodePtr->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ nodePtr }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nodePtr->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    return SetLengthMetricUnit(nodePtr, unit);
}

void MarkDirtySafely(ArkUI_NodeHandle nodePtr, ArkUI_NodeDirtyFlag dirtyFlag)
{
    auto* impl = GetFullImpl();
    if (impl->getMultiThreadManagerAPI()->checkOnUIThread()) {
        CHECK_NODE_DISPOSED(nodePtr, "Node has been disposed");
        NativeNodeOperationScope treeOperation({ nodePtr }, __FUNCTION__);
        if (nodePtr) {
            impl->getMultiThreadManagerAPI()->markNodeTreeNotFree(nodePtr->uiNodeHandle);
        }
        MarkDirty(nodePtr, dirtyFlag);
    }
}

int32_t AddNodeEventReceiverSafely(ArkUI_NodeHandle nodePtr, void (*eventReceiver)(ArkUI_NodeEvent* event))
{
    CHECK_NULL_RETURN(nodePtr, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nodePtr->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ nodePtr }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nodePtr->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    return AddNodeEventReceiver(nodePtr, eventReceiver);
}

int32_t RemoveNodeEventReceiverSafely(ArkUI_NodeHandle nodePtr, void (*eventReceiver)(ArkUI_NodeEvent* event))
{
    CHECK_NULL_RETURN(nodePtr, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nodePtr->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ nodePtr }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nodePtr->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    return RemoveNodeEventReceiver(nodePtr, eventReceiver);
}

int32_t RegisterNodeCustomEventSafely(ArkUI_NodeHandle node,
    ArkUI_NodeCustomEventType eventType, int32_t targetId, void* userData)
{
    CHECK_NULL_RETURN(node, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ node }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    return RegisterNodeCustomEvent(node, eventType, targetId, userData);
}

void UnregisterNodeCustomEventSafely(ArkUI_NodeHandle node, ArkUI_NodeCustomEventType eventType)
{
    CHECK_NULL_VOID(node);
    auto* impl = GetFullImpl();
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ node }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return;
    }
    UnregisterNodeCustomEvent(node, eventType);
}

void RegisterNodeCustomReceiverSafely(void (*eventReceiver)(ArkUI_NodeCustomEvent* event))
{
    auto* impl = GetFullImpl();
    if (impl->getMultiThreadManagerAPI()->checkOnUIThread()) {
        RegisterNodeCustomReceiver(eventReceiver);
    }
}

void UnregisterNodeCustomEventReceiverSafely()
{
    auto* impl = GetFullImpl();
    if (impl->getMultiThreadManagerAPI()->checkOnUIThread()) {
        UnregisterNodeCustomEventReceiver();
    }
}

int32_t AddNodeCustomEventReceiverSafely(ArkUI_NodeHandle nodePtr, void (*eventReceiver)(ArkUI_NodeCustomEvent* event))
{
    CHECK_NULL_RETURN(nodePtr, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nodePtr->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ nodePtr }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nodePtr->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    return AddNodeCustomEventReceiver(nodePtr, eventReceiver);
}

int32_t RemoveNodeCustomEventReceiverSafely(ArkUI_NodeHandle nodePtr,
    void (*eventReceiver)(ArkUI_NodeCustomEvent* event))
{
    CHECK_NULL_RETURN(nodePtr, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nodePtr->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ nodePtr }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(nodePtr->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    return RemoveNodeCustomEventReceiver(nodePtr, eventReceiver);
}

int32_t SetMeasuredSizeSafely(ArkUI_NodeHandle node, int32_t width, int32_t height)
{
    CHECK_NULL_RETURN(node, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    if (impl->getMultiThreadManagerAPI()->checkOnUIThread()) {
        NativeNodeOperationScope treeOperation({ node }, __FUNCTION__);
        return SetMeasuredSize(node, width, height);
    }
    return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
}

int32_t SetLayoutPositionSafely(ArkUI_NodeHandle node, int32_t positionX, int32_t positionY)
{
    CHECK_NULL_RETURN(node, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    if (impl->getMultiThreadManagerAPI()->checkOnUIThread()) {
        NativeNodeOperationScope treeOperation({ node }, __FUNCTION__);
        return SetLayoutPosition(node, positionX, positionY);
    }
    return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
}

ArkUI_IntSize GetMeasuredSizeSafely(ArkUI_NodeHandle node)
{
    ArkUI_IntSize size = {0, 0};
    CHECK_NULL_RETURN(node, size);
    auto* impl = GetFullImpl();
    if (impl->getMultiThreadManagerAPI()->checkOnUIThread()) {
        NativeNodeOperationScope treeOperation({ node }, __FUNCTION__);
        return GetMeasuredSize(node);
    }
    return size;
}

ArkUI_IntOffset GetLayoutPositionSafely(ArkUI_NodeHandle node)
{
    ArkUI_IntOffset offset;
    CHECK_NULL_RETURN(node, offset);
    auto* impl = GetFullImpl();
    if (impl->getMultiThreadManagerAPI()->checkOnUIThread()) {
        NativeNodeOperationScope treeOperation({ node }, __FUNCTION__);
        return GetLayoutPosition(node);
    }
    return offset;
}

int32_t MeasureNodeSafely(ArkUI_NodeHandle node, ArkUI_LayoutConstraint* constraint)
{
    CHECK_NULL_RETURN(node, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    if (impl->getMultiThreadManagerAPI()->checkOnUIThread()) {
        CHECK_NODE_DISPOSED(node, "Node has been disposed");
        NativeNodeOperationScope treeOperation({ node }, __FUNCTION__);
        impl->getMultiThreadManagerAPI()->markNodeTreeNotFree(node->uiNodeHandle);
        return MeasureNode(node, constraint);
    }
    return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
}

int32_t LayoutNodeSafely(ArkUI_NodeHandle node, int32_t positionX, int32_t positionY)
{
    CHECK_NULL_RETURN(node, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    if (impl->getMultiThreadManagerAPI()->checkOnUIThread()) {
        CHECK_NODE_DISPOSED(node, "Node has been disposed");
        NativeNodeOperationScope treeOperation({ node }, __FUNCTION__);
        impl->getMultiThreadManagerAPI()->markNodeTreeNotFree(node->uiNodeHandle);
        return LayoutNode(node, positionX, positionY);
    }
    return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
}

uint32_t GetTotalChildCountSafely(ArkUI_NodeHandle node)
{
    CHECK_NULL_RETURN(node, 0);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(node, "Node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return 0;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ node }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return 0;
    }
    return GetTotalChildCount(node);
}

ArkUI_NodeHandle GetChildAtSafely(ArkUI_NodeHandle node, int32_t position)
{
    CHECK_NULL_RETURN(node, nullptr);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(node, "Node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return nullptr;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ node }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return nullptr;
    }
    auto result = GetChildAt(node, position);
    threadSafeNodeScope.IncludeNode(result);
    return result;
}

ArkUI_NodeHandle GetFirstChildSafely(ArkUI_NodeHandle node)
{
    CHECK_NULL_RETURN(node, nullptr);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(node, "Node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return nullptr;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ node }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return nullptr;
    }
    auto result = GetFirstChild(node);
    threadSafeNodeScope.IncludeNode(result);
    return result;
}

ArkUI_NodeHandle GetLastChildSafely(ArkUI_NodeHandle node)
{
    CHECK_NULL_RETURN(node, nullptr);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(node, "Node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return nullptr;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ node }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return nullptr;
    }
    auto result = GetLastChild(node);
    threadSafeNodeScope.IncludeNode(result);
    return result;
}

ArkUI_NodeHandle GetPreviousSiblingSafely(ArkUI_NodeHandle node)
{
    CHECK_NULL_RETURN(node, nullptr);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(node, "Node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return nullptr;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ node }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return nullptr;
    }
    auto result = GetPreviousSibling(node);
    threadSafeNodeScope.IncludeNode(result);
    return result;
}

ArkUI_NodeHandle GetNextSiblingSafely(ArkUI_NodeHandle node)
{
    CHECK_NULL_RETURN(node, nullptr);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(node, "Node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return nullptr;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ node }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return nullptr;
    }
    auto result = GetNextSibling(node);
    threadSafeNodeScope.IncludeNode(result);
    return result;
}

ArkUI_NodeHandle GetParentSafely(ArkUI_NodeHandle node)
{
    CHECK_NULL_RETURN(node, nullptr);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(node, "Node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return nullptr;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ node }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(node->uiNodeHandle)) {
        return nullptr;
    }
    auto result = GetParent(node);
    threadSafeNodeScope.IncludeNode(result);
    return result;
}

int32_t RemoveAllChildrenSafely(ArkUI_NodeHandle parentNode)
{
    CHECK_NULL_RETURN(parentNode, ERROR_CODE_PARAM_INVALID);
    auto* impl = GetFullImpl();
    CHECK_NODE_DISPOSED(parentNode, "Node has been disposed");
    if (!impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(parentNode->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    ThreadSafeNodeScope threadSafeNodeScope({ parentNode }, __FUNCTION__);
    if (threadSafeNodeScope.IsChecking() &&
        !impl->getMultiThreadManagerAPI()->checkNodeOnValidThread(parentNode->uiNodeHandle)) {
        return ERROR_CODE_NATIVE_IMPL_NODE_ON_INVALID_THREAD;
    }
    MarkNodeTreeFreeScope markNodeTreeFreeScope;
    return RemoveAllChildren(parentNode);
}

bool IsValidArkUINodeMultiThread(ArkUI_NodeHandle nodePtr)
{
    std::lock_guard<std::mutex> lock(g_nodeSetMutex_);
    return g_nodeSetSafely.count(nodePtr) > 0;
}
} // namespace OHOS::Ace::NodeModel
