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

#include "core/components_ng/base/frame_node.h"
#include "core/interfaces/native/utility/converter.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace PathModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    return nullptr;
}
} // PathModifier
namespace PathInterfaceModifier {
void SetPathOptionsImpl(Ark_NativePointer node,
                        const Opt_PathOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = options ? Converter::OptConvert<type>(*options) : std::nullopt;
    //PathModelNG::SetSetPathOptions(frameNode, convValue);
}
} // PathInterfaceModifier
namespace PathAttributeModifier {
void CommandsImpl(Ark_NativePointer node,
                  const Ark_String* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto convValue = Converter::Convert<std::string>(*value);
    //PathModelNG::SetCommands(frameNode, convValue);
}
} // PathAttributeModifier
const GENERATED_ArkUIPathModifier* GetPathModifier()
{
    static const GENERATED_ArkUIPathModifier ArkUIPathModifierImpl {
        PathModifier::ConstructImpl,
        PathInterfaceModifier::SetPathOptionsImpl,
        PathAttributeModifier::CommandsImpl,
    };
    return &ArkUIPathModifierImpl;
}

}
