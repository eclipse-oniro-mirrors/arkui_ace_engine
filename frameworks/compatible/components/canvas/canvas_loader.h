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

#ifndef FOUNDATION_ACE_FRAMEWORKS_COMPATILBE_COMPONENTS_CANVAS_LOADER
#define FOUNDATION_ACE_FRAMEWORKS_COMPATILBE_COMPONENTS_CANVAS_LOADER

#include "compatible/components/component_loader.h"

namespace OHOS::Ace {

class CanvasLoader : public ComponentLoader {
public:
    CanvasLoader() = default;
    ~CanvasLoader() = default;

    RefPtr<Framework::DOMNode> CreateDomNode(int32_t nodeId, const std::string& nodeName) override;
    void* CreateModel() override;
    RefPtr<Declaration> CreateDeclaration() override;
    RefPtr<V2::InspectorComposedElement> CreateInspectorElement(const std::string& id) override;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_COMPATILBE_COMPONENTS_CANVAS_LOADER