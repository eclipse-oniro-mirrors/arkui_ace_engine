/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_PLUGIN_COMPONENT_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_PLUGIN_COMPONENT_MANAGER_H

#include <map>
#include <mutex>
#include <unordered_set>

#include "bundlemgr/bundle_mgr_interface.h"
#ifdef UI_SERVICE_WITH_IDL
#include "iu_i_service_new.h"
#include "ui_service_new_stub.h"
#else
#include "ui_service_interface.h"
#include "ui_service_stub.h"
#endif
#include "want.h"

#include "base/json/json_util.h"
#include "base/memory/ace_type.h"
#include "core/components/common/layout/grid_column_info.h"
#include "core/components/plugin/plugin_component_callback.h"
#include "core/components/plugin/plugin_component_template.h"

namespace OHOS::Ace {
class ACE_FORCE_EXPORT PluginComponentManager final {
public:
    PluginComponentManager()
    {
        if (!listener_) {
            listener_ = new (std::nothrow) UIServiceListener();
        }
    }
    ~PluginComponentManager() = default;

    ACE_EXPORT static std::shared_ptr<PluginComponentManager> GetInstance();

    int Push(const AAFwk::Want& want, const std::string& name, const std::string& jsonPath, const std::string& data,
        const std::string& extraData);
    int Request(const AAFwk::Want& want, const std::string& name, const std::string& jsonPath, const std::string& data);
    int ReturnRequest(
        const AAFwk::Want& want, const std::string& pluginName, const std::string& data, const std::string& extraData);
    void RegisterCallBack(
        const AAFwk::Want& want, const std::shared_ptr<PluginComponentCallBack>& callback, CallBackType callBackType);
    void UnregisterCallBack(const std::shared_ptr<PluginComponentCallBack>& callback);
    void UnregisterCallBack(const AAFwk::Want& want);

    bool GetTemplatePathFromJsonFile(const std::string& packagePathStr, const std::string& srcPath,
        const std::string& jsonPath, std::string& jsonStr);
    std::string GetPackagePath(const AAFwk::Want& want) const;

    sptr<AppExecFwk::IBundleMgr> GetBundleManager();

#ifdef UI_SERVICE_WITH_IDL
    class UIServiceListener final : public Ace::UIServiceNewStub {
#else
    class UIServiceListener final : public Ace::UIServiceStub {
#endif
    public:
        UIServiceListener() = default;
        ~UIServiceListener()
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            callbackVec_.clear();
        };

        void ResgisterListener(const std::shared_ptr<PluginComponentCallBack>& callback, CallBackType callBackType);
        void UnresgisterListener(const std::shared_ptr<PluginComponentCallBack>& callback);
#ifdef UI_SERVICE_WITH_IDL
        ErrCode OnPushCallBack(const AAFwk::Want& want, const std::string& name, const std::string& jsonPath,
            const std::string& data, const std::string& extraData) override;
        ErrCode OnRequestCallBack(const AAFwk::Want& want, const std::string& name, const std::string& data) override;
        ErrCode OnReturnRequest(const AAFwk::Want& want, const std::string& source, const std::string& data,
            const std::string& extraData) override;
#else
        void OnPushCallBack(const AAFwk::Want& want, const std::string& name, const std::string& jsonPath,
            const std::string& data, const std::string& extraData) override;
        void OnRequestCallBack(const AAFwk::Want& want, const std::string& name, const std::string& data) override;
        void OnReturnRequest(const AAFwk::Want& want, const std::string& source, const std::string& data,
            const std::string& extraData) override;
#endif
        void RequestByJsonPath(const PluginComponentTemplate& pluginTemplate, const std::string& data);

    private:
        std::recursive_mutex mutex_;
        std::map<std::shared_ptr<PluginComponentCallBack>, CallBackType> callbackVec_;
        std::unordered_set<std::shared_ptr<PluginComponentCallBack>> callbacks_;

#ifdef UI_SERVICE_WITH_IDL
        DISALLOW_COPY_AND_MOVE(UIServiceListener);
#endif
    };

private:
    static std::mutex mutex_;
    static std::shared_ptr<PluginComponentManager> instance_;
    sptr<UIServiceListener> listener_ = nullptr;

    ACE_DISALLOW_COPY_AND_MOVE(PluginComponentManager);
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_PLUGIN_COMPONENT_MANAGER_H
