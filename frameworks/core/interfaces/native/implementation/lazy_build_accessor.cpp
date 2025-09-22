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

#include "arkoala_api_generated.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/tabs/tab_content_model_static.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/callback_helper.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace LazyBuildAccessor {
void ApplyLazyBuilderImpl()
{
}
void SetListItemLazyBuilderImpl(Ark_NativePointer node,
                                const CustomNodeBuilder* builder)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(node);
    //auto convValue = Converter::OptConvert<type>(node); // for enums
    // undefinedModelNG::SetSetListItemLazyBuilder(frameNode, convValue);
}
void SetTabContentLazyBuilderImpl(Ark_NativePointer node,
                                  const CustomNodeBuilder* builder)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(builder);
    auto deepRender = [arkBuilder = CallbackHelper(*builder), node,
        weak = AceType::WeakClaim(frameNode)]() -> RefPtr<FrameNode> {
        auto refPtr = weak.Upgrade();
        CHECK_NULL_RETURN(refPtr, nullptr);
        arkBuilder.BuildAsync(
            [refPtr](const RefPtr<UINode>& uiNode) mutable {
                uiNode->MountToParent(refPtr);
                refPtr->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
            },
            node);
        return nullptr;
    };
    auto shallowBuilder = AceType::MakeRefPtr<ShallowBuilder>(std::move(deepRender));
    TabContentModelStatic::SetShallowBuilder(frameNode, shallowBuilder);
}
} // LazyBuildAccessor
const GENERATED_ArkUILazyBuildAccessor* GetLazyBuildAccessor()
{
    static const GENERATED_ArkUILazyBuildAccessor LazyBuildAccessorImpl {
        LazyBuildAccessor::ApplyLazyBuilderImpl,
        LazyBuildAccessor::SetListItemLazyBuilderImpl,
        LazyBuildAccessor::SetTabContentLazyBuilderImpl,
    };
    return &LazyBuildAccessorImpl;
}

}
