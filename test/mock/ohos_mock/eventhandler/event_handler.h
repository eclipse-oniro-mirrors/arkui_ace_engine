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

#ifndef FOUNDATION_ACE_MOCK_EVENTHANDLER_EVENT_HANDLER_H
#define FOUNDATION_ACE_MOCK_EVENTHANDLER_EVENT_HANDLER_H

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <utility>
#include <vector>

namespace OHOS {
namespace AppExecFwk {

class EventHandler {
public:
    EventHandler() = default;
    virtual ~EventHandler() = default;

    static std::shared_ptr<EventHandler> Current()
    {
        return nullptr;
    }

    template<typename Task>
    bool PostTask(Task&& task, const std::string& name, int32_t delayTime)
    {
        if (failPostTask_) { return false; }
        tasks_[name] = { now_ + delayTime, std::forward<Task>(task) };
        return true;
    }

    template<typename Task>
    bool PostTask(Task&& task, const char* name, int32_t delayTime)
    {
        return PostTask(std::forward<Task>(task), std::string(name), delayTime);
    }

    void RemoveTask(const std::string& name) { tasks_.erase(name); }
    void RemoveTask(const char* name) { tasks_.erase(name); }

    void SetPostTaskFailure(bool fail) { failPostTask_ = fail; }
    void AdvanceTime(int32_t milliseconds)
    {
        now_ += milliseconds;
        std::vector<std::function<void()>> ready;
        for (auto iter = tasks_.begin(); iter != tasks_.end();) {
            if (iter->second.first <= now_) {
                ready.emplace_back(std::move(iter->second.second));
                iter = tasks_.erase(iter);
            } else {
                ++iter;
            }
        }
        for (auto& task : ready) { task(); }
    }
private:
    int64_t now_ = 0;
    bool failPostTask_ = false;
    std::map<std::string, std::pair<int64_t, std::function<void()>>> tasks_;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // FOUNDATION_ACE_MOCK_EVENTHANDLER_EVENT_HANDLER_H
