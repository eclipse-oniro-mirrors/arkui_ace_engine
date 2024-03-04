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

#include "core/interfaces/native/node/node_api.h"
#include "core/interfaces/native/node/extension_companion_node.h"

namespace OHOS::Ace::NG::ViewModel {

void* CreateNode(ArkUINodeType tag, ArkUI_Int32 nodeId);

void DisposeNode(void* nativePtr);

void AddChild(void* parentNode, void* childNode);

void RemoveChild(void* parentNode, void* childNode);

void InsertChildAfter(void* parentNode, void* childNode, void* siblingNode);

void RegisterCompanion(void* node, int peerId, ArkUI_Int32 flags);

ExtensionCompanionNode* GetCompanion(void* nodePtr);

void SetCustomCallback(void* nodePtr, ArkUI_Int32 callback);

void SetCallbackMethod(ArkUIAPICallbackMethod* method);

ArkUIAPICallbackMethod* GetCallbackMethod();

ArkUI_Int32 MeasureNode(ArkUIVMContext context, ArkUINodeHandle nodePtr, ArkUI_Float32* data);

ArkUI_Int32 LayoutNode(ArkUIVMContext context, ArkUINodeHandle nodePtr, ArkUI_Float32* data);

ArkUI_Int32 DrawNode(ArkUIVMContext context, ArkUINodeHandle nodePtr, ArkUI_Float32* data);

} // namespace OHOS::Ace::NG::ViewModel
