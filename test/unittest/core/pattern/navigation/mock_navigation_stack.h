/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_TEST_MOCK_NAVIGATION_STACK_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_TEST_MOCK_NAVIGATION_STACK_H

#include "gmock/gmock.h"

#define protected public
#define private public
#include "core/components_ng/pattern/navigation/navigation_stack.h"
struct MockReplace {
    int32_t isReplace_ = 0;
};
using NavigationInterceptionEvent = std::function<void(const OHOS::Ace::RefPtr<OHOS::Ace::NG::NavDestinationContext>,
    const OHOS::Ace::RefPtr<OHOS::Ace::NG::NavDestinationContext>, OHOS::Ace::NG::NavigationOperation, bool)>;
class MockNavigationStack : public OHOS::Ace::NG::NavigationStack {
    DECLARE_ACE_TYPE(MockNavigationStack, NavigationStack);
public:
    void UpdateReplaceValue(int32_t isReplace) const override
    {
        mockReplace_->isReplace_ = isReplace;
    }

    int32_t GetReplaceValue() const override
    {
        return mockReplace_->isReplace_;
    }

    bool GetDisableAnimation() const override
    {
        return false;
    }

    void SetOnStateChangedCallback(std::function<void()> callback) override
    {
        onStateChangedCallback_ = callback;
    }

    std::function<void()> GetOnStateChangedCallback() const
    {
        return onStateChangedCallback_;
    }

        void SetInterceptionBeforeCallback(NavigationInterceptionEvent callback)
    {
        beforeCallback_ = callback;
    }

    void SetInterceptionAfterCallback(NavigationInterceptionEvent afterCallback)
    {
        afterCallback_ = afterCallback;
    }

    void SetInterceptionModeCallback(std::function<void(OHOS::Ace::NG::NavigationMode)> modeCallback)
    {
        modeCallback_ = modeCallback;
    }

    void FireNavigationModeChange(OHOS::Ace::NG::NavigationMode mode) override
    {
        if (modeCallback_) {
            modeCallback_(mode);
        }
    }

    void FireNavigationInterception(bool isBefore, const OHOS::Ace::RefPtr<OHOS::Ace::NG::NavDestinationContext>& from,
        const OHOS::Ace::RefPtr<OHOS::Ace::NG::NavDestinationContext>& to, OHOS::Ace::NG::NavigationOperation operation,
        bool isAnimated) override
    {
        if (isBefore) {
            if (beforeCallback_) {
                beforeCallback_(from, to, operation, isAnimated);
            }
        } else {
            if (afterCallback_) {
                afterCallback_(from, to, operation, isAnimated);
            }
        }
    }

    MOCK_METHOD1(OnAttachToParent, void(OHOS::Ace::RefPtr<OHOS::Ace::NG::NavigationStack>));
    MOCK_METHOD0(OnDetachFromParent, void());

private:
    std::function<void()> onStateChangedCallback_;
    NavigationInterceptionEvent beforeCallback_;
    NavigationInterceptionEvent afterCallback_;
    std::function<void(OHOS::Ace::NG::NavigationMode)> modeCallback_;
    MockReplace *mockReplace_ = new MockReplace();
};
#endif