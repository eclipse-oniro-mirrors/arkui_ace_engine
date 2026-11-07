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

#include "core/common/dynamic_module_helper.h"
#include "test/mock/frameworks/core/common/mock_canvas_dynamic_module.h"
#include "test/mock/frameworks/core/common/mock_swiper_dynamic_module.h"
#include "test/mock/frameworks/core/common/mock_tabs_dynamic_module.h"

namespace OHOS::Ace {

DynamicModuleHelper& DynamicModuleHelper::GetInstance()
{
    static DynamicModuleHelper instance;
    return instance;
}

std::unique_ptr<ComponentLoader> DynamicModuleHelper::GetLoaderByName(const char* name)
{
    return nullptr;
}

DynamicModule* DynamicModuleHelper::GetDynamicModule(const std::string& name)
{
#ifdef MOCK_DYNAMIC_MODULE_COMPONENTS
    static MockSwiperDynamicModule swiperModule;
    static MockTabsDynamicModule tabsModule;
    static MockCanvasDynamicModule canvasModule;
    if (name == "Swiper") {
        return &swiperModule;
    }
    if (name == "TabContent") {
        return &tabsModule;
    }
    if (name == "Canvas") {
        return &canvasModule;
    }
#endif
    return nullptr;
}

bool DynamicModuleHelper::IsDynamicModuleLoaded(const std::string& name)
{
    return true;
}
} // namespace OHOS::Ace
