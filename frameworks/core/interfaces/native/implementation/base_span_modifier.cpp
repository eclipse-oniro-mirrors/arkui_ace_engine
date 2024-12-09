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
namespace BaseSpanModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    return nullptr;
}
void TextBackgroundStyleImpl(Ark_NativePointer node,
                             const Ark_TextBackgroundStyle* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //BaseSpanModelNG::SetTextBackgroundStyle(frameNode, convValue);
}
void BaselineOffsetImpl(Ark_NativePointer node,
                        const Ark_LengthMetrics* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //BaseSpanModelNG::SetBaselineOffset(frameNode, convValue);
}
} // BaseSpanModifier
const GENERATED_ArkUIBaseSpanModifier* GetBaseSpanModifier()
{
    static const GENERATED_ArkUIBaseSpanModifier ArkUIBaseSpanModifierImpl {
        BaseSpanModifier::ConstructImpl,
        BaseSpanModifier::TextBackgroundStyleImpl,
        BaseSpanModifier::BaselineOffsetImpl,
    };
    return &ArkUIBaseSpanModifierImpl;
}

}
