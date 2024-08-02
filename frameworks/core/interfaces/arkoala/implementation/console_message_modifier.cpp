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

#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier {
    namespace ConsoleMessageModifier {
        Ark_NativePointer CtorImpl(const Ark_String* message, const Ark_String* sourceId, const Ark_Number* lineNumber, Ark_Int32 messageLevel) {
            return 0;
        }
        Ark_NativePointer GetFinalizerImpl() {
            return 0;
        }
        void GetMessageImpl(ConsoleMessagePeer* peer) {
        }
        void GetSourceIdImpl(ConsoleMessagePeer* peer) {
        }
        Ark_Int32 GetLineNumberImpl(ConsoleMessagePeer* peer) {
            return 0;
        }
        Ark_NativePointer GetMessageLevelImpl(ConsoleMessagePeer* peer) {
            return 0;
        }
    } // ConsoleMessageModifier
    const GENERATED_ArkUIConsoleMessageAccessor* GetConsoleMessageAccessor() {
        static const GENERATED_ArkUIConsoleMessageAccessor ConsoleMessageAccessorImpl {
            ConsoleMessageModifier::CtorImpl,
            ConsoleMessageModifier::GetFinalizerImpl,
            ConsoleMessageModifier::GetMessageImpl,
            ConsoleMessageModifier::GetSourceIdImpl,
            ConsoleMessageModifier::GetLineNumberImpl,
            ConsoleMessageModifier::GetMessageLevelImpl,
        };
        return &ConsoleMessageAccessorImpl;
    }

}
