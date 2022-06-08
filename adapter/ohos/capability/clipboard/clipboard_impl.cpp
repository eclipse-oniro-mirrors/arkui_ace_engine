/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "adapter/ohos/capability/clipboard/clipboard_impl.h"

#ifdef SYSTEM_CLIPBOARD_SUPPORTED
#include "pasteboard_client.h"
#endif

namespace OHOS::Ace {
#ifndef SYSTEM_CLIPBOARD_SUPPORTED
namespace {
std::string g_clipboard;
}
#endif
void ClipboardImpl::SetData(const std::string& data)
{
#ifdef SYSTEM_CLIPBOARD_SUPPORTED
    if (taskExecutor_) {
        taskExecutor_->PostTask(
            [data]() {
                auto pasteData = OHOS::MiscServices::PasteboardClient::GetInstance()->CreatePlainTextData(data);
                if (!pasteData) {
                    LOGE("create SystemKeyboardData fail from MiscServices");
                    return;
                }
                OHOS::MiscServices::PasteboardClient::GetInstance()->SetPasteData(*pasteData);
            },
            TaskExecutor::TaskType::PLATFORM);
    }
#else
    LOGI("Current device doesn't support system clipboard");
    taskExecutor_->PostTask([data]() { g_clipboard = data; }, TaskExecutor::TaskType::UI);
#endif
}

void ClipboardImpl::GetData(const std::function<void(const std::string&)>& callback, bool syncMode)
{
#ifdef SYSTEM_CLIPBOARD_SUPPORTED
    if (!taskExecutor_ || !callback) {
        return;
    }

    if (syncMode) {
        GetDataSync(callback);
    } else {
        GetDataAsync(callback);
    }
#else
    LOGI("Current device doesn't support system clipboard");
    if (syncMode) {
        callback(g_clipboard);
    } else {
        taskExecutor_->PostTask(
            [callback, taskExecutor = WeakClaim(RawPtr(taskExecutor_)), textData = g_clipboard]() {
                callback(textData);
            },
            TaskExecutor::TaskType::UI);
    }
#endif
}

#ifdef SYSTEM_CLIPBOARD_SUPPORTED
void ClipboardImpl::GetDataSync(const std::function<void(const std::string&)>& callback)
{
    std::string result;
    taskExecutor_->PostSyncTask(
        [&result]() {
            auto has = OHOS::MiscServices::PasteboardClient::GetInstance()->HasPasteData();
            if (!has) {
                LOGE("SystemKeyboardData is not exist from MiscServices");
                return;
            }
            OHOS::MiscServices::PasteData pasteData;
            auto ok = OHOS::MiscServices::PasteboardClient::GetInstance()->GetPasteData(pasteData);
            if (!ok) {
                LOGE("Get SystemKeyboardData fail from MiscServices");
                return;
            }
            auto textData = pasteData.GetPrimaryText();
            if (!textData) {
                LOGE("Get SystemKeyboardTextData fail from MiscServices");
                return;
            }
            result = *textData;
        },
        TaskExecutor::TaskType::PLATFORM);
    callback(result);
}

void ClipboardImpl::GetDataAsync(const std::function<void(const std::string&)>& callback)
{
    taskExecutor_->PostTask(
        [callback, weakExecutor = WeakClaim(RawPtr(taskExecutor_))]() {
            auto taskExecutor = weakExecutor.Upgrade();
            if (!taskExecutor) {
                LOGE("TaskExecutor is not exist");
                return;
            }
            auto has = OHOS::MiscServices::PasteboardClient::GetInstance()->HasPasteData();
            if (!has) {
                LOGE("SystemKeyboardData is not exist from MiscServices");
                taskExecutor->PostTask([callback]() { callback(""); }, TaskExecutor::TaskType::UI);
                return;
            }
            OHOS::MiscServices::PasteData pasteData;
            auto ok = OHOS::MiscServices::PasteboardClient::GetInstance()->GetPasteData(pasteData);
            if (!ok) {
                LOGE("Get SystemKeyboardData fail from MiscServices");
                taskExecutor->PostTask([callback]() { callback(""); }, TaskExecutor::TaskType::UI);
                return;
            }
            auto textData = pasteData.GetPrimaryText();
            if (!textData) {
                LOGE("Get SystemKeyboardTextData fail from MiscServices");
                taskExecutor->PostTask([callback]() { callback(""); }, TaskExecutor::TaskType::UI);
                return;
            }
            auto result = *textData;
            taskExecutor->PostTask([callback, result]() { callback(result); }, TaskExecutor::TaskType::UI);
        },
        TaskExecutor::TaskType::PLATFORM);
}
#endif

void ClipboardImpl::Clear() {}

} // namespace OHOS::Ace