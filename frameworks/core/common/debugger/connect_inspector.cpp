/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "frameworks/core/common/debugger/connect_inspector.h"
#include "base/log/log.h"

namespace OHOS::Ace {
std::unique_ptr<ConnectInspector> g_inspector = nullptr;

void* HandleDebugManager(void* const server)
{
    if (server == nullptr) {
        LOGE("HandleDebugManager server nullptr");
        return nullptr;
    }
    static_cast<ConnectServer*>(server)->RunServer();
    return nullptr;
}

void OnMessage(const std::string& message)
{
    if (message.empty()) {
        LOGE("message is empty");
        return;
    }

    std::string checkMessage = "connected";
    if (message.find(checkMessage, 0) != std::string::npos) {
        LOGI("Find the target string: %{private}s", message.c_str());
        if (g_inspector != nullptr && g_inspector->connectServer_ != nullptr) {
            g_inspector->waitingForDebugger_ = false;
            for (auto &info : g_inspector->infoBuffer_) {
                g_inspector->connectServer_->SendMessage(info.second);
            }
        }
    }
}

void ResetService()
{
    if (g_inspector != nullptr && g_inspector->connectServer_ != nullptr) {
        g_inspector->connectServer_->StopServer();
        g_inspector->connectServer_.reset();
    }
}

void StartServer(const std::string& componentName)
{
    g_inspector = std::make_unique<ConnectInspector>();
    g_inspector->connectServer_ = std::make_unique<ConnectServer>(componentName,
        std::bind(&OnMessage, std::placeholders::_1));

    pthread_t tid;
    if (pthread_create(&tid, nullptr, &HandleDebugManager,
        static_cast<void*>(g_inspector->connectServer_.get())) != 0) {
        LOGE("pthread_create fail!");
        ResetService();
        return;
    }
}

void StopServer(const std::string& componentName)
{
    ResetService();
}

void StoreMessage(int32_t instanceId, const std::string& message)
{
    if (g_inspector->infoBuffer_.count(instanceId) == 1) {
        LOGE("The message with the current instance id has been existed.");
        return;
    }
    g_inspector->infoBuffer_[instanceId] = message;
}

void RemoveMessage(int32_t instanceId)
{
    if (g_inspector->infoBuffer_.count(instanceId) != 1) {
        LOGE("The message with the current instance id does not exist.");
        return;
    }
    g_inspector->infoBuffer_.erase(instanceId);
}

void SendMessage(const std::string& message)
{
    if (g_inspector != nullptr && g_inspector->connectServer_ != nullptr && !g_inspector->waitingForDebugger_) {
        g_inspector->connectServer_->SendMessage(message);
    }
}

bool WaitForDebugger()
{
    if (g_inspector == nullptr) {
        return true;
    }
    return g_inspector->waitingForDebugger_;
}

} // namespace OHOS::Ace
