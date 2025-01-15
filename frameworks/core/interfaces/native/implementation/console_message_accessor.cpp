/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
namespace ConsoleMessageAccessor {
void DestroyPeerImpl(ConsoleMessagePeer* peer)
{
}
Ark_NativePointer CtorImpl(const Ark_String* message,
                           const Ark_String* sourceId,
                           const Ark_Number* lineNumber,
                           Ark_MessageLevel messageLevel)
{
    return new ConsoleMessagePeer(message, sourceId, lineNumber, messageLevel);
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
void GetMessageImpl(ConsoleMessagePeer* peer)
{
}
void GetSourceIdImpl(ConsoleMessagePeer* peer)
{
}
Ark_Int32 GetLineNumberImpl(ConsoleMessagePeer* peer)
{
    return 0;
}
Ark_NativePointer GetMessageLevelImpl(ConsoleMessagePeer* peer)
{
    return nullptr;
}
} // ConsoleMessageAccessor
const GENERATED_ArkUIConsoleMessageAccessor* GetConsoleMessageAccessor()
{
    static const GENERATED_ArkUIConsoleMessageAccessor ConsoleMessageAccessorImpl {
        ConsoleMessageAccessor::DestroyPeerImpl,
        ConsoleMessageAccessor::CtorImpl,
        ConsoleMessageAccessor::GetFinalizerImpl,
        ConsoleMessageAccessor::GetMessageImpl,
        ConsoleMessageAccessor::GetSourceIdImpl,
        ConsoleMessageAccessor::GetLineNumberImpl,
        ConsoleMessageAccessor::GetMessageLevelImpl,
    };
    return &ConsoleMessageAccessorImpl;
}

}
