/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

namespace OHOS::Ace::NG::NodeModifier {
const ArkUICommonModifier* GetCommonModifier();

void SetOnAppear(ArkUINodeHandle node,  void* extraParam);
void SetOnDisappear(ArkUINodeHandle node,  void* extraParam);
void SetOnAttach(ArkUINodeHandle node, void* extraParam);
void SetOnDetach(ArkUINodeHandle node, void* extraParam);
void SetOnFocus(ArkUINodeHandle node, void* extraParam);
void SetOnBlur(ArkUINodeHandle node, void* extraParam);
void SetOnAreaChange(ArkUINodeHandle node, void* extraParam);
void SetOnClick(ArkUINodeHandle node, void* extraParam);
void SetOnTouch(ArkUINodeHandle node, void* extraParam);
void SetOnTouchIntercept(ArkUINodeHandle node, void* extraParam);
void SetOnHover(ArkUINodeHandle node, void* extraParam);
void SetOnMouse(ArkUINodeHandle node, void* extraParam);
} // namespace OHOS::Ace::NG::NodeModifier
