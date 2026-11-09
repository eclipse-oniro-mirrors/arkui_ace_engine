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

#ifndef FOUNDATION_ACE_INTERFACE_INNERKITS_ACE_TEST_MOCK_MOCK_IREMOTE_STUB_H
#define FOUNDATION_ACE_INTERFACE_INNERKITS_ACE_TEST_MOCK_MOCK_IREMOTE_STUB_H

#include "message_parcel.h"
#include "message_option.h"
#include "iremote_object.h"
#include "iremote_broker.h"

namespace OHOS {
class IPCObjectStub : public IRemoteObject {
public:
    IPCObjectStub() = default;
    virtual ~IPCObjectStub() = default;

    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return 0;
    }
};

template <typename T>
class IRemoteStub : public IPCObjectStub, public T {
public:
    IRemoteStub() = default;
    virtual ~IRemoteStub() override = default;

    sptr<IRemoteBroker> QueryLocalInterface(const std::u16string& descriptor) override
    {
        if (descriptor != T::GetDescriptor()) {
            return {};
        }
        // Stack/uniquely owned mocks have no shared control block to retain.
        auto owner = this->weak_from_this().lock();
        if (!owner) {
            return {};
        }
        return sptr<IRemoteBroker>(std::shared_ptr<IRemoteBroker>(owner, static_cast<T*>(this)));
    }

    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override
    {
        return 0;
    }
};
}
#endif
