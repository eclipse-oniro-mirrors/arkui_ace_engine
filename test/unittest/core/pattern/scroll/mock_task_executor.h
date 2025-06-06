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

#ifndef FOUNDATION_ACE_TEST_UNITTEST_CORE_PATTERN_SCROLL_MOCK_TASK_EXECUTOR_H
#define FOUNDATION_ACE_TEST_UNITTEST_CORE_PATTERN_SCROLL_MOCK_TASK_EXECUTOR_H

#include "gmock/gmock.h"

#include "base/thread/task_executor.h"
#include "base/utils/utils.h"

namespace OHOS::Ace {
class MockScrollTaskExecutor : public TaskExecutor {
public:
    MockScrollTaskExecutor() = default;
    explicit MockScrollTaskExecutor(bool delayRun) : delayRun_(delayRun) {}

    bool OnPostTask(Task&& task, TaskType type, uint32_t delayTime, const std::string& name,
        PriorityType priorityType = PriorityType::LOW) const override
    {
        CHECK_NULL_RETURN(task, false);
        if (delayRun_) {
            return true;
        }
        delayTask_ = task;
        return true;
    }

    void RunDelayTask()
    {
        if (delayTask_) {
            delayTask_();
        }
    }

    bool WillRunOnCurrentThread(TaskType type) const override
    {
        return true;
    }

    Task WrapTaskWithTraceId(Task&& /*task*/, int32_t /*id*/) const override
    {
        return nullptr;
    }

    void AddTaskObserver(Task&& callback) override {}

    void RemoveTaskObserver() override {}

    void RemoveTask(TaskType type, const std::string& name) override {}

    bool OnPostTaskWithoutTraceId(Task&& task, TaskType type, uint32_t delayTime, const std::string& name,
        PriorityType priorityType = PriorityType::LOW) const override
    {
        CHECK_NULL_RETURN(task, false);
        if (delayRun_) {
            return true;
        }
        task();
        return true;
    }

private:
    bool delayRun_ = false;
    mutable Task delayTask_;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_TEST_UNITTEST_CORE_PATTERN_SCROLL_MOCK_TASK_EXECUTOR_H
