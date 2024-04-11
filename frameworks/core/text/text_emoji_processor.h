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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_TEXT_ACE_TEXT_EMOJI_PROCESSOR_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_TEXT_ACE_TEXT_EMOJI_PROCESSOR_H

#include "base/utils/string_utils.h"

namespace OHOS::Ace {
class TextEmojiProcessor {
public:
    // static functions
    /*
     * Delete characters from the content string according to the startIndex and length.
     * startIndex: The UTF-16 start index.
     * For backward delete, it's the end index + 1, for forward delete, it's the start index.
     * Example: content = "abcde";
     * For backward at end, the startIndex is 5. For forward at start, the startIndex is 0.
     * length: The number of characters to delete.
     * Note: An emoji may have multiple bytes, but it's considered as ONE character.
     * content: The content string to delete.
     * isBackward: Whether the delete action is backward.
     * return deleted UTF-16 bytes length.
     * Why UTF-16? TextSelectController->GetCaretIndex() returns UTF-16 index.
     */
    static int32_t Delete(int32_t startIndex, int32_t length, std::string& content, bool isBackward);
    static bool IsEmoji(uint32_t codePoint);
    static bool IsEmojiModifierBase(uint32_t codePoint);
    static bool IsVariationSelector(uint32_t codePoint);
    static bool IsRegionalIndicatorSymbol(uint32_t codePoint);
    static bool IsEmojiModifier(uint32_t codePoint);
    static bool IsTagSpec(uint32_t codePoint);
    static bool IsKeycapBase(uint32_t codePoint);

private:
    static void OnBeginState(uint32_t codePoint, int& state, int& deleteCount, bool isBackward);
    static void OnRISState(uint32_t codePoint, int& state, int& deleteCount, bool isBackward);
    static void OnCRLFState(uint32_t codePoint, int& state, int& deleteCount, bool isBackward);
    static void OnZWJState(uint32_t codePoint, int& state, int& deleteCount, int& lastVSCount, bool isBackward);
    static void OnVSState(uint32_t codePoint, int& state, int& deleteCount, bool isBackward);
    static void OnKeyCapState(uint32_t codePoint, int& state, int& deleteCount, int& lastVSCount, bool isBackward);
    static void OnEMState(uint32_t codePoint, int& state, int& deleteCount, int& lastVSCount, bool isBackward);
    static void OnEmojiState(uint32_t codePoint, int& state, int& deleteCount, bool isBackward);
    static void OnForwardSecondState(uint32_t codePoint, int& state, int& deleteCount);
    static void OnTagQueueState(uint32_t codePoint, int& state, int& deleteCount, bool isBackward);
    static bool BackwardDelete(std::u32string& u32Content);
    static bool ForwardDelete(std::u32string& u32Content);
    static bool HandleDeleteAction(std::u32string& u32Content, int32_t deleteCount, bool isBackward);
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_TEXT_ACE_TEXT_EMOJI_PROCESSOR_H
