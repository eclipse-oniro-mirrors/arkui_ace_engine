/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 /*
  * Description: Grid组件高频接口CAPI补齐
  * Writer: renxiaowen
  */

#ifndef ARKUI_CAPI_DEMO_TOGGLE_COMPONENT_H
#define ARKUI_CAPI_DEMO_TOGGLE_COMPONENT_H

#include <string>

#include "components/component.h"

namespace ArkUICApiDemo {
class ToggleComponent : public Component {
public:
    ToggleComponent() : Component(ARKUI_NODE_TOGGLE) {}
    explicit ToggleComponent(ArkUI_NodeHandle handle) : Component(handle) {}
    void SetUnselectedColor(uint32_t color);
    void SetToggleValue(int32_t type);
};
} // namespace ArkUICApiDemo
#endif // ARKUI_CAPI_DEMO_TOGGLE_COMPONENT_H
