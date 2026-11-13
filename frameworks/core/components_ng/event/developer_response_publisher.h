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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_DEVELOPER_RESPONSE_PUBLISHER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_DEVELOPER_RESPONSE_PUBLISHER_H

#include <atomic>
#include <string>

#include "base/memory/ace_type.h"
#include "base/utils/noncopyable.h"
#include "ui/event/event_constants.h"

namespace OHOS::Ace::NG {

class FrameNode;

enum class DeveloperResponseCallbackType : uint8_t {
    CLICK,
    TOUCH,
};

// Response candidate produced after a developer onClick/onTouch callback actually executes.
struct DeveloperResponseCandidate {
    WeakPtr<FrameNode> node;
    std::string operationToken;
    DeveloperResponseCallbackType callbackType = DeveloperResponseCallbackType::CLICK;
    TouchType touchType = TouchType::UNKNOWN;
};

class DeveloperResponseObserver : public AceType {
    DECLARE_ACE_TYPE(DeveloperResponseObserver, AceType);
public:
    DeveloperResponseObserver() = default;
    ~DeveloperResponseObserver() override = default;

    virtual void OnDeveloperResponse(const DeveloperResponseCandidate& candidate) = 0;

    ACE_DISALLOW_COPY_AND_MOVE(DeveloperResponseObserver);
};

class DeveloperResponsePublisher : public AceType {
    DECLARE_ACE_TYPE(DeveloperResponsePublisher, AceType);
public:
    static DeveloperResponsePublisher* GetInstance();

    void SetObserver(DeveloperResponseObserver* observer);

    void PublishClick(const DeveloperResponseCandidate& candidate);
    void PublishTouch(const DeveloperResponseCandidate& candidate);

private:
    DeveloperResponsePublisher() = default;
    ~DeveloperResponsePublisher() override = default;

    std::atomic<DeveloperResponseObserver*> observer_ { nullptr };

    ACE_DISALLOW_COPY_AND_MOVE(DeveloperResponsePublisher);
};

} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_EVENT_DEVELOPER_RESPONSE_PUBLISHER_H
