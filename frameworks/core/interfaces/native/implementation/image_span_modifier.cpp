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
namespace ImageSpanModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    return {};
}
} // ImageSpanModifier
namespace ImageSpanInterfaceModifier {
void SetImageSpanOptionsImpl(Ark_NativePointer node,
                             const Ark_Union_ResourceStr_PixelMap* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    //auto convValue = Converter::OptConvert<type_name>(*value);
    //ImageSpanModelNG::SetSetImageSpanOptions(frameNode, convValue);
}
} // ImageSpanInterfaceModifier
namespace ImageSpanAttributeModifier {
void SetVerticalAlignImpl(Ark_NativePointer node,
                          const Opt_ImageSpanAlignment* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //ImageSpanModelNG::SetSetVerticalAlign(frameNode, convValue);
}
void SetObjectFitImpl(Ark_NativePointer node,
                      const Opt_ImageFit* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //ImageSpanModelNG::SetSetObjectFit(frameNode, convValue);
}
void SetOnCompleteImpl(Ark_NativePointer node,
                       const Opt_ImageCompleteCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //ImageSpanModelNG::SetSetOnComplete(frameNode, convValue);
}
void SetOnErrorImpl(Ark_NativePointer node,
                    const Opt_ImageErrorCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //ImageSpanModelNG::SetSetOnError(frameNode, convValue);
}
void SetAltImpl(Ark_NativePointer node,
                const Opt_image_PixelMap* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //ImageSpanModelNG::SetSetAlt(frameNode, convValue);
}
} // ImageSpanAttributeModifier
const GENERATED_ArkUIImageSpanModifier* GetImageSpanModifier()
{
    static const GENERATED_ArkUIImageSpanModifier ArkUIImageSpanModifierImpl {
        ImageSpanModifier::ConstructImpl,
        ImageSpanInterfaceModifier::SetImageSpanOptionsImpl,
        ImageSpanAttributeModifier::SetVerticalAlignImpl,
        ImageSpanAttributeModifier::SetObjectFitImpl,
        ImageSpanAttributeModifier::SetOnCompleteImpl,
        ImageSpanAttributeModifier::SetOnErrorImpl,
        ImageSpanAttributeModifier::SetAltImpl,
    };
    return &ArkUIImageSpanModifierImpl;
}

}
