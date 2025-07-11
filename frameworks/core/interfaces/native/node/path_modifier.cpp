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
#include "core/interfaces/native/node/path_modifier.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/shape/path_model_ng.h"

namespace OHOS::Ace::NG {

void SetPathCommands(ArkUINodeHandle node, ArkUI_CharPtr commands, void* resObjPtr)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    auto commandsVal = std::string(commands);
    PathModelNG::SetCommands(frameNode, commandsVal);
    auto pattern = frameNode->GetPattern();
    CHECK_NULL_VOID(pattern);
    pattern->UnRegisterResource("PathCommands");
    if (SystemProperties::ConfigChangePerform() && resObjPtr) {
        auto resObj = AceType::Claim(reinterpret_cast<ResourceObject*>(resObjPtr));
        pattern->RegisterResource<std::string>("PathCommands", resObj, commandsVal);
    }
}

void ResetPathCommands(ArkUINodeHandle node)
{
    std::string outCommands = "";
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    PathModelNG::SetCommands(frameNode, outCommands);
    auto pattern = frameNode->GetPattern();
    CHECK_NULL_VOID(pattern);
    pattern->UnRegisterResource("PathCommands");
}

namespace NodeModifier {
const ArkUIPathModifier* GetPathModifier()
{
    CHECK_INITIALIZED_FIELDS_BEGIN(); // don't move this line
    static const ArkUIPathModifier modifier = {
        .setPathCommands = SetPathCommands,
        .resetPathCommands = ResetPathCommands,
    };
    CHECK_INITIALIZED_FIELDS_END(modifier, 0, 0, 0); // don't move this line

    return &modifier;
}

const CJUIPathModifier* GetCJUIPathModifier()
{
    CHECK_INITIALIZED_FIELDS_BEGIN(); // don't move this line
    static const CJUIPathModifier modifier = {
        .setPathCommands = SetPathCommands,
        .resetPathCommands = ResetPathCommands,
    };
    CHECK_INITIALIZED_FIELDS_END(modifier, 0, 0, 0); // don't move this line

    return &modifier;
}
}
}
