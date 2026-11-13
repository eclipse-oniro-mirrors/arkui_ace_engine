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

#include "core/interfaces/native/node/node_api_multi_thread.h"


#include "base/error/error_code.h"
#include "base/log/log_wrapper.h"
#include "base/utils/system_properties.h"
#include "core/common/multi_thread_build_manager.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/base/native_node_tree_scope.h"

namespace OHOS::Ace::NG {
void SetIsThreadSafeNodeScope(ArkUI_Bool isThreadSafeNodeScope)
{
    MultiThreadBuildManager::SetIsThreadSafeNodeScope(isThreadSafeNodeScope);
}

int32_t CheckNodeOnValidThread(ArkUINodeHandle node)
{
    UINode* currentNode = reinterpret_cast<UINode*>(node);
    return static_cast<int32_t>(MultiThreadBuildManager::CheckNodeOnValidThread(currentNode));
}

void SetNeedMarkNodeTreeFree(ArkUI_Bool needMarkNodeTreeFree)
{
    MultiThreadBuildManager::SetNeedMarkNodeTreeFree(needMarkNodeTreeFree);
}

void MarkNodeTreeNotFree(ArkUINodeHandle node)
{
    UINode* currentNode = reinterpret_cast<UINode*>(node);
    if (currentNode && currentNode->IsThreadSafeNode()) {
        currentNode->MarkNodeTreeNotFree();
    }
}

int32_t DebugThreadSafeNodeEnabled()
{
    return SystemProperties::GetDebugThreadSafeNodeEnabled();
}

int32_t CheckOnUIThread()
{
    return MultiThreadBuildManager::CheckOnUIThread();
}

int32_t PostAsyncUITask(ArkUI_Int32 contextId,
    void* asyncUITaskData, void (*asyncUITask)(void* asyncUITaskData), void(*onFinish)(void* asyncUITaskData))
{
    auto asyncUITaskFunc = [asyncUITaskData, asyncUITask]() {
        if (asyncUITask == nullptr) {
            return;
        }
        LOG_CALLBACK("OH_ArkUI_PostAsyncUITask:task", asyncUITask);
        asyncUITask(asyncUITaskData);
    };
    auto onFinishFunc = [asyncUITaskData, onFinish]() {
        if (onFinish == nullptr) {
            return;
        }
        LOG_CALLBACK("OH_ArkUI_PostAsyncUITask:finish", onFinish);
        onFinish(asyncUITaskData);
    };
    if (!MultiThreadBuildManager::GetInstance().PostAsyncUITask(
        contextId, std::move(asyncUITaskFunc), std::move(onFinishFunc))) {
        return ERROR_CODE_PARAM_INVALID;
    }
    return ERROR_CODE_NO_ERROR;
}

int32_t PostUITask(ArkUI_Int32 contextId, void* taskData, void(*task)(void* taskData))
{
    auto taskFunc = [taskData, task]() {
        if (task == nullptr) {
            return;
        }
        LOG_CALLBACK("OH_ArkUI_PostUITask", task);
        task(taskData);
    };
    if (!MultiThreadBuildManager::GetInstance().PostUITask(contextId, std::move(taskFunc))) {
        return ERROR_CODE_PARAM_INVALID;
    }
    return ERROR_CODE_NO_ERROR;
}

int32_t PostUITaskAndWait(ArkUI_Int32 contextId, void* taskData, void(*task)(void* taskData))
{
    auto taskFunc = [taskData, task]() {
        if (task == nullptr) {
            return;
        }
        LOG_CALLBACK("OH_ArkUI_PostUITaskAndWait", task);
        task(taskData);
    };
    if (!MultiThreadBuildManager::GetInstance().PostUITaskAndWait(contextId, std::move(taskFunc))) {
        return ERROR_CODE_PARAM_INVALID;
    }
    return ERROR_CODE_NO_ERROR;
}

const ArkUIMultiThreadManagerAPI* GetMultiThreadManagerAPI()
{
    static const ArkUIMultiThreadManagerAPI multiThreadImpl = {
        .setIsThreadSafeNodeScope = SetIsThreadSafeNodeScope,
        .checkNodeOnValidThread = CheckNodeOnValidThread,
        .checkOnUIThread = CheckOnUIThread,
        .postAsyncUITask = PostAsyncUITask,
        .postUITask = PostUITask,
        .postUITaskAndWait = PostUITaskAndWait,
        .markNodeTreeNotFree = MarkNodeTreeNotFree,
        .debugThreadSafeNodeEnabled = DebugThreadSafeNodeEnabled,
        .setNeedMarkNodeTreeFree = SetNeedMarkNodeTreeFree,
        .beginNodeTreeOperation = [](
            ArkUINodeHandle* nodes, ArkUI_Int32 count, const char* apiName) -> void* {
            const auto node = [nodes, count](int32_t index) -> UINode* {
                return index < count ? reinterpret_cast<UINode*>(nodes[index]) : nullptr;
            };
            return new NativeNodeTreeScope({ node(0), node(1), node(2) }, apiName);
        },
        .endNodeTreeOperation = [](void* operation) { delete static_cast<NativeNodeTreeScope*>(operation); },
        .setNodeTreeOperationCheckMode = NativeNodeTreeScope::SetCheckMode,
        .isThreadSafeNodeScope = []() -> ArkUI_Bool { return MultiThreadBuildManager::IsThreadSafeNodeScope(); },
        .isNeedMarkNodeTreeFree = []() -> ArkUI_Bool { return MultiThreadBuildManager::IsNeedMarkNodeTreeFree(); },
        .needsNodeTreeOperationCheck = []() -> ArkUI_Bool { return NativeNodeTreeScope::NeedsSynchronization(); },
        .includeNodeTreeOperationResult = [](void* operation, ArkUINodeHandle node) {
            if (operation) {
                static_cast<NativeNodeTreeScope*>(operation)->IncludeNode(reinterpret_cast<UINode*>(node));
            }
        },
        .prepareNodeTreeOperationChildren = [](void* operation, ArkUINodeHandle parent) {
            if (operation) {
                static_cast<NativeNodeTreeScope*>(operation)->PrepareChildren(reinterpret_cast<UINode*>(parent));
            }
        },
    };
    return &multiThreadImpl;
}
} // namespace OHOS::Ace::NG
