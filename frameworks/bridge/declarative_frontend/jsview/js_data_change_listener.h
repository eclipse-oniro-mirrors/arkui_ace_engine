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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_DATA_CHANGE_LISTENER_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_DATA_CHANGE_LISTENER_H

#include <functional>
#include <set>
#include <string>

#include "base/memory/ace_type.h"
#include "bridge/declarative_frontend/engine/bindings.h"
#include "bridge/declarative_frontend/engine/js_ref_ptr.h"
#include "bridge/declarative_frontend/jsview/js_view.h"

namespace OHOS::Ace::Framework {

class JSDataChangeListener : public Referenced {
public:
    static void JSBind(BindingTarget globalObj);

    void AddListener(const WeakPtr<V2::DataChangeListener>& listener)
    {
        listeners_.emplace(listener);
    }

    void RemoveListener(const WeakPtr<V2::DataChangeListener>& listener)
    {
        listeners_.erase(listener);
    }

private:
    static void Constructor(const JSCallbackInfo& args)
    {
        auto listener = Referenced::MakeRefPtr<JSDataChangeListener>();
        listener->instanceId_ = ContainerScope::CurrentId();
        listener->IncRefCount();
        args.SetReturnValue(Referenced::RawPtr(listener));
    }

    static void Destructor(JSDataChangeListener* listener)
    {
        if (listener != nullptr) {
            listener->DecRefCount();
        }
    }

    bool ConvertFromJSCallbackInfo(const JSCallbackInfo& args, size_t index, size_t& result)
    {
        int32_t value = 0;
        if (!args.GetInt32Arg(index, value)) {
            return false;
        }
        if (value < 0) {
            TAG_LOGW(AceLogTag::ACE_LAZY_FOREACH, "Invalid index: %{public}d, change negative value to 0",
                value);
            value = 0;
        }
        result = static_cast<size_t>(value);
        return true;
    }

    void OnDataReloaded(const JSCallbackInfo& args)
    {
        useOldInterface = true;
        if (useAnotherInterface(useNewInterface)) {
            return;
        }
        NotifyAll(&V2::DataChangeListener::OnDataReloaded);
    }

    void OnDataAdded(const JSCallbackInfo& args)
    {
        useOldInterface = true;
        if (useAnotherInterface(useNewInterface)) {
            return;
        }
        NotifyAll(&V2::DataChangeListener::OnDataAdded, args);
    }

    void OnDataBulkAdded(const JSCallbackInfo& args)
    {
        useOldInterface = true;
        if (useAnotherInterface(useNewInterface)) {
            return;
        }
        ContainerScope scope(instanceId_);
        size_t index = 0;
        size_t count = 0;
        if (args.Length() < 2 || !ConvertFromJSCallbackInfo(args, 0, index) ||
            !ConvertFromJSCallbackInfo(args, 1, count)) {
            return;
        }
        NotifyAll(&V2::DataChangeListener::OnDataBulkAdded, index, count);
    }

    void OnDataDeleted(const JSCallbackInfo& args)
    {
        useOldInterface = true;
        if (useAnotherInterface(useNewInterface)) {
            return;
        }
        NotifyAll(&V2::DataChangeListener::OnDataDeleted, args);
    }

    void OnDataBulkDeleted(const JSCallbackInfo& args)
    {
        useOldInterface = true;
        if (useAnotherInterface(useNewInterface)) {
            return;
        }
        ContainerScope scope(instanceId_);
        size_t index = 0;
        size_t count = 0;
        if (args.Length() < 2 || !ConvertFromJSCallbackInfo(args, 0, index) ||
            !ConvertFromJSCallbackInfo(args, 1, count)) {
            return;
        }
        NotifyAll(&V2::DataChangeListener::OnDataBulkDeleted, index, count);
    }

    void OnDataChanged(const JSCallbackInfo& args)
    {
        useOldInterface = true;
        if (useAnotherInterface(useNewInterface)) {
            return;
        }
        NotifyAll(&V2::DataChangeListener::OnDataChanged, args);
    }

    void OnDataMoved(const JSCallbackInfo& args)
    {
        useOldInterface = true;
        if (useAnotherInterface(useNewInterface)) {
            return;
        }
        ContainerScope scope(instanceId_);
        size_t from = 0;
        size_t to = 0;
        if (args.Length() < 2 || !ConvertFromJSCallbackInfo(args, 0, from) ||
            !ConvertFromJSCallbackInfo(args, 1, to)) {
            return;
        }
        NotifyAll(&V2::DataChangeListener::OnDataMoved, from, to);
    }

    void OnDatasetChange(const JSCallbackInfo& args)
    {
        useNewInterface = true;
        if (useAnotherInterface(useOldInterface)) {
            return;
        }
        ContainerScope scope(instanceId_);
        if (args.Length() != 1 || !args[0]->IsArray()) {
            return;
        }
        JSRef<JSArray> jsArr = JSRef<JSArray>::Cast(args[0]);
        std::list<V2::Operation> DataOperations;
        for (size_t i = 0; i < jsArr->Length(); i++) {
            if (jsArr->GetValueAt(i)->IsObject()) {
                JSRef<JSObject> value = JSRef<JSObject>::Cast(jsArr->GetValueAt(i));
                TransferJSInfoType(DataOperations, value);
            }
        }
        if (allocateMoreKeys) {
            JSException::Throw(ERROR_CODE_PARAM_INVALID, "%s",
                "The number of key is more than count for ADD operation");
        }
        NotifyAll(&V2::DataChangeListener::OnDatasetChange, DataOperations);
    }

    void TransferJSInfoType(std::list<V2::Operation>& DataOperations, JSRef<JSObject> value)
    {
        auto jsType = value->GetProperty("type");
        if (!jsType->IsString()) {
            return;
        }
        V2::Operation dataOperation;
        std::string operationType = jsType->ToString();
        dataOperation.type = operationType;
        dataOperation.count = 1;
        const int ADDOP = 1;
        const int DELETEOP = 2;
        const int CHANGEOP = 3;
        const int MOVEOP = 4;
        const int EXCHANGEOP = 5;
        switch (operationTypeMap[operationType]) {
            case ADDOP:
                transferIndex(value, dataOperation);
                transferCount(value, dataOperation);
                transferKey(value, dataOperation);
                break;
            case DELETEOP:
                transferIndex(value, dataOperation);
                transferCount(value, dataOperation);
                break;
            case CHANGEOP:
            case MOVEOP:
            case EXCHANGEOP:
                transferIndex(value, dataOperation);
                transferKey(value, dataOperation);
                break;
        }
        if (dataOperation.count < 0) {
            return;
        }
        DataOperations.push_back(dataOperation);
    }

    void transferIndex(JSRef<JSObject> value, V2::Operation& dataOperation)
    {
        auto jsIndex = value->GetProperty("index");
        if (jsIndex->IsNumber()) {
            dataOperation.index = jsIndex->ToNumber<int32_t>();
        } else if (jsIndex->IsObject()) {
            JSRef<JSObject> coupleIndex = JSRef<JSObject>::Cast(jsIndex);
            auto jsFrom = coupleIndex->GetProperty("from");
            if (jsFrom->IsNumber()) {
                dataOperation.coupleIndex.first = jsFrom->ToNumber<int32_t>();
            }
            auto jsTo = coupleIndex->GetProperty("to");
            if (jsTo->IsNumber()) {
                dataOperation.coupleIndex.second = jsTo->ToNumber<int32_t>();
            }
            auto jsStart = coupleIndex->GetProperty("start");
            if (jsStart->IsNumber()) {
                dataOperation.coupleIndex.first = jsStart->ToNumber<int32_t>();
            }
            auto jsEnd = coupleIndex->GetProperty("end");
            if (jsEnd->IsNumber()) {
                dataOperation.coupleIndex.second = jsEnd->ToNumber<int32_t>();
            }
        }
    }

    void transferCount(JSRef<JSObject> value, V2::Operation& dataOperation)
    {
        auto jsCount = value->GetProperty("count");
        if (jsCount->IsNumber()) {
            dataOperation.count = jsCount->ToNumber<int32_t>();
        }
    }

    void transferKey(JSRef<JSObject> value, V2::Operation& dataOperation)
    {
        auto jsKey = value->GetProperty("key");
        if (jsKey->IsString()) {
            dataOperation.key = jsKey->ToString();
        } else if (jsKey->IsArray()) {
            JSRef<JSArray> keys = JSRef<JSArray>::Cast(jsKey);
            if (dataOperation.type == "add" && static_cast<size_t>(dataOperation.count) < keys->Length()) {
                allocateMoreKeys = true;
                return;
            }
            std::list<std::string> keyList;
            for (size_t i = 0; i<keys->Length(); i++) {
                if (keys->GetValueAt(i)->IsString()) {
                    keyList.push_back(keys->GetValueAt(i)->ToString());
                }
            }
            dataOperation.keyList = keyList;
        } else if (jsKey->IsObject()) {
            JSRef<JSObject> coupleKey = JSRef<JSObject>::Cast(jsKey);
            auto jsFrom = coupleKey->GetProperty("from");
            if (jsFrom->IsString()) {
                dataOperation.coupleKey.first = jsFrom->ToString();
            }
            auto jsTo = coupleKey->GetProperty("to");
            if (jsTo->IsString()) {
                dataOperation.coupleKey.second = jsTo->ToString();
            }
            auto jsStart = coupleKey->GetProperty("start");
            if (jsStart->IsString()) {
                dataOperation.coupleKey.first = jsStart->ToString();
            }
            auto jsEnd = coupleKey->GetProperty("end");
            if (jsEnd->IsString()) {
                dataOperation.coupleKey.second = jsEnd->ToString();
            }
        }
    }

    bool useAnotherInterface(bool forbid)
    {
        if (forbid) {
            JSException::Throw(ERROR_CODE_PARAM_INVALID, "%s",
                "onDatasetChange cannot be used with other interface");
            return true;
        }
        return false;
    }

    template<class... Args>
    void NotifyAll(void (V2::DataChangeListener::*method)(Args...), const JSCallbackInfo& args)
    {
        ContainerScope scope(instanceId_);
        size_t index = 0;
        if (args.Length() > 0 && ConvertFromJSCallbackInfo(args, 0, index)) {
            NotifyAll(method, index);
        }
    }

    template<class... Args>
    void NotifyAll(void (V2::DataChangeListener::*method)(Args...), const std::list<V2::Operation>& args)
    {
        ContainerScope scope(instanceId_);
        for (auto it = listeners_.begin(); it != listeners_.end();) {
            auto listener = it->Upgrade();
            if (!listener) {
                it = listeners_.erase(it);
                continue;
            }
            ++it;
            ((*listener).*method)(args);
        }
    }

    template<class... Args>
    void NotifyAll(void (V2::DataChangeListener::*method)(Args...), Args... args)
    {
        ContainerScope scope(instanceId_);
        for (auto it = listeners_.begin(); it != listeners_.end();) {
            auto listener = it->Upgrade();
            if (!listener) {
                it = listeners_.erase(it);
                continue;
            }
            ++it;
            ((*listener).*method)(args...);
        }
    }

    std::set<WeakPtr<V2::DataChangeListener>> listeners_;
    int32_t instanceId_ = -1;
    std::map<std::string, int32_t> operationTypeMap = {
        {"add", 1},
        {"delete", 2},
        {"change", 3},
        {"move", 4},
        {"exchange", 5},
        {"reload", 6}
    };
    bool useOldInterface = false;
    bool useNewInterface = false;
    bool allocateMoreKeys = false;
};

} // namespace OHOS::Ace::Framework

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_DATA_CHANGE_LISTENER_H
