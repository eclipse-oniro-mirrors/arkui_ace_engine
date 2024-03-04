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

#pragma once

#include <cstdint>

#include "native_node.h"
#include "native_type.h"

#include "frameworks/core/interfaces/arkoala/arkoala_api.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ArkUI_Node {
    int32_t type;
    ArkUINodeHandle uiNodeHandle = nullptr;
};

constexpr int BASIC_COMPONENT_NUM = 18;

#ifdef __cplusplus
};
#endif

struct ArkUIFullNodeAPI;

namespace OHOS::Ace::NodeModel {

ArkUIFullNodeAPI* GetFullImpl();
ArkUI_NodeHandle CreateNode(ArkUI_NodeType type);
void DisposeNode(ArkUI_NodeHandle nativePtr);

int32_t AddChild(ArkUI_NodeHandle parentNode, ArkUI_NodeHandle childNode);
int32_t RemoveChild(ArkUI_NodeHandle parentNode, ArkUI_NodeHandle childNode);
int32_t InsertChildAfter(ArkUI_NodeHandle parentNode, ArkUI_NodeHandle childNode, ArkUI_NodeHandle siblingNode);

// deprecated.
[[deprecated]] void SetAttribute(ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute, const char* value);

int32_t SetAttribute(ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute, const ArkUI_AttributeItem* value);
const ArkUI_AttributeItem* GetAttribute(ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute);
int32_t ResetAttribute(ArkUI_NodeHandle node, ArkUI_NodeAttributeType attribute);

int32_t RegisterNodeEvent(ArkUI_NodeHandle nodePtr, ArkUI_NodeEventType eventType, int32_t eventId);
void UnregisterNodeEvent(ArkUI_NodeHandle nodePtr, ArkUI_NodeEventType eventType);
void RegisterOnEvent(void (*eventReceiver)(ArkUI_NodeEvent* event));
void UnregisterOnEvent();
int32_t CheckEvent(ArkUI_NodeEvent* event);

void ApplyModifierFinish(ArkUI_NodeHandle nodePtr);
void MarkDirty(ArkUI_NodeHandle nodePtr, ArkUI_NodeDirtyFlag dirtyFlag);

}; // namespace OHOS::Ace::NodeModel
