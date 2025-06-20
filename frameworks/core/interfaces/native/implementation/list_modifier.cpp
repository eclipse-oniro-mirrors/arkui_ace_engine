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

#include "base/geometry/axis.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/list/list_model_ng.h"
#include "core/interfaces/native/utility/callback_helper.h"
#include "core/interfaces/native/utility/converter.h"
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/utility/callback_helper.h"
#include "core/interfaces/native/generated/interface/node_api.h"
#include "frameworks/core/components_ng/pattern/scrollable/scrollable_properties.h"
#include "frameworks/core/components_ng/pattern/scrollable/scrollable_model_static.h"
#include "frameworks/core/components_v2/list/list_properties.h"
#include "core/common/container.h"
#include "core/components_ng/pattern/scroll_bar/proxy/scroll_bar_proxy.h"
#include "list_scroller_peer_impl.h"
#include "children_main_size_peer.h"

namespace OHOS::Ace::NG {
using ListLanesType = std::variant<int, std::pair<Dimension, Dimension>>;
}

namespace OHOS::Ace::NG::Converter {
    struct ListOptions {
        std::optional<int> initialIndex;
        std::optional<Dimension> space;
        std::optional<Ark_Scroller> scroller;
    };

    struct NestedScrollModeOptions {
        std::optional<NestedScrollMode> forward;
        std::optional<NestedScrollMode> backward;
    };

    template<>
    inline NestedScrollModeOptions Convert(const Ark_NestedScrollOptions& src)
    {
        NestedScrollModeOptions options;
        options.forward = Converter::OptConvert<NestedScrollMode>(src.scrollForward);
        options.backward = Converter::OptConvert<NestedScrollMode>(src.scrollBackward);
        return options;
    }

    template<>
    V2::ItemDivider Convert(const Ark_ListDividerOptions& src)
    {
        auto dst = V2::ItemDivider{}; // this struct is initialized by default
        dst.strokeWidth = OptConvert<Dimension>(src.strokeWidth).value_or(dst.strokeWidth);
        auto colorOpt = OptConvert<Color>(src.color);
        if (colorOpt.has_value()) {
            dst.color = colorOpt.value();
        }
        auto startMarginOpt = OptConvert<Dimension>(src.startMargin);
        if (startMarginOpt.has_value()) {
            dst.startMargin = startMarginOpt.value();
        }
        auto endMarginOpt = OptConvert<Dimension>(src.endMargin);
        if (endMarginOpt.has_value()) {
            dst.endMargin = endMarginOpt.value();
        }
        return dst;
    }

    template<>
    ChainAnimationOptions Convert(const Ark_ChainAnimationOptions& src)
    {
        auto options = ChainAnimationOptions{}; // this struct is initialized by default
        options.minSpace = OptConvert<Dimension>(src.minSpace).value_or(options.minSpace);
        options.maxSpace = OptConvert<Dimension>(src.maxSpace).value_or(options.maxSpace);

        auto intensity = Converter::OptConvert<float>(src.intensity);
        if (intensity.has_value()) {
            options.intensity = intensity.value();
        }

        auto conductivity = Converter::OptConvert<float>(src.conductivity);
        if (conductivity.has_value()) {
            options.conductivity = conductivity.value();
        }

        auto stiffness = Converter::OptConvert<float>(src.stiffness);
        if (stiffness.has_value()) {
            options.stiffness = stiffness.value();
        }

        auto damping = Converter::OptConvert<float>(src.damping);
        if (damping.has_value()) {
            options.damping = damping.value();
        }

        auto edgeEffect = Converter::OptConvert<ChainEdgeEffect>(src.edgeEffect);
        if (edgeEffect.has_value()) {
            options.edgeEffect = static_cast<int32_t>(edgeEffect.value());
        }
        return options;
    }

    template<>
    ListLanesType Convert(const Ark_Number& src)
    {
        return Converter::Convert<int>(src);
    }

    template<>
    ListLanesType Convert(const Ark_LengthConstrain& src)
    {
        return Converter::Convert<std::pair<Dimension, Dimension>>(src);
    }

    template<>
    ListOptions Convert(const Ark_ListOptions& src)
    {
        return {
            .initialIndex = OptConvert<int>(src.initialIndex),
            .space = OptConvert<Dimension>(src.space),
            .scroller = OptConvert<Ark_Scroller>(src.scroller)
        };
    }
}
namespace OHOS::Ace::NG::GeneratedModifier {
namespace ListModifier {
Ark_NativePointer ConstructImpl(Ark_Int32 id,
                                Ark_Int32 flags)
{
    auto frameNode = ListModelNG::CreateFrameNode(id);
    CHECK_NULL_RETURN(frameNode, nullptr);
    frameNode->IncRefCount();
    return AceType::RawPtr(frameNode);
}
} // ListModifier
namespace ListInterfaceModifier {
void SetListOptionsImpl(Ark_NativePointer node,
                        const Opt_ListOptions* options)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(options);

    auto optionsOpt = Converter::OptConvert<Converter::ListOptions>(*options);
    if (!optionsOpt.has_value()) {
        return;
    }

    auto initialIndex = optionsOpt.value().initialIndex;
    ListModelNG::SetInitialIndex(frameNode, initialIndex);
    auto space = optionsOpt.value().space;
    ListModelNG::SetListSpace(frameNode, space);

    RefPtr<ScrollControllerBase> positionController = ListModelNG::GetOrCreateController(frameNode);
    RefPtr<ScrollProxy> scrollBarProxy = ListModelNG::GetOrCreateScrollBarProxy(frameNode);
    auto abstPeerPtrOpt = optionsOpt.value().scroller;
    CHECK_NULL_VOID(abstPeerPtrOpt);
    auto peerImplPtr = *abstPeerPtrOpt;
    CHECK_NULL_VOID(peerImplPtr);
    peerImplPtr->SetController(positionController);
    peerImplPtr->SetScrollBarProxy(scrollBarProxy);
}
} // ListInterfaceModifier
namespace ListAttributeModifier {
void AlignListItemImpl(Ark_NativePointer node,
                       const Opt_ListItemAlign* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ListModelNG::SetListItemAlign(frameNode, Converter::OptConvert<V2::ListItemAlign>(*value));
}
void ListDirectionImpl(Ark_NativePointer node,
                       const Opt_Axis* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    std::optional<Axis> direction = Converter::OptConvert<Axis>(*value);
    ListModelNG::SetListDirection(frameNode, EnumToInt(direction));
}
void ContentStartOffsetImpl(Ark_NativePointer node,
                            const Opt_Number* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<float>(*value);
    if (!convValue) {
        // TODO: Reset value
        return;
    }
    ListModelNG::SetContentStartOffset(frameNode, *convValue);
}
void ContentEndOffsetImpl(Ark_NativePointer node,
                          const Opt_Number* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<float>(*value);
    if (!convValue) {
        // TODO: Reset value
        return;
    }
    ListModelNG::SetContentEndOffset(frameNode, *convValue);
}
void DividerImpl(Ark_NativePointer node,
                 const Opt_ListDividerOptions* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(value);
    auto divider = Converter::OptConvert<V2::ItemDivider>(*value);
    ListModelNG::SetDivider(frameNode, divider);
}
void MultiSelectableImpl(Ark_NativePointer node,
                         const Opt_Boolean* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<bool>(*value);
    if (!convValue) {
        // TODO: Reset value
        return;
    }
    ListModelNG::SetMultiSelectable(frameNode, *convValue);
}
void CachedCount0Impl(Ark_NativePointer node,
                      const Opt_Number* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<int>(*value);
    if (!convValue) {
        // TODO: Reset value
        return;
    }
    ListModelNG::SetCachedCount(frameNode, *convValue);
}
void CachedCount1Impl(Ark_NativePointer node,
                      const Opt_Number* count,
                      const Opt_Boolean* show)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = Converter::Convert<type>(count);
    //auto convValue = Converter::OptConvert<type>(count); // for enums
    //ListModelNG::SetCachedCount1(frameNode, convValue);
}
void ChainAnimationImpl(Ark_NativePointer node,
                        const Opt_Boolean* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<bool>(*value);
    if (!convValue) {
        // TODO: Reset value
        return;
    }
    ListModelNG::SetChainAnimation(frameNode, *convValue);
}
void ChainAnimationOptionsImpl(Ark_NativePointer node,
                               const Opt_ChainAnimationOptions* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<ChainAnimationOptions>(*value);
    if (!convValue) {
        // TODO: Reset value
        return;
    }
    ListModelNG::SetChainAnimationOptions(frameNode, *convValue);
}
void StickyImpl(Ark_NativePointer node,
                const Opt_StickyStyle* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    std::optional<V2::StickyStyle> style = Converter::OptConvert<V2::StickyStyle>(*value);
    ListModelNG::SetSticky(frameNode, EnumToInt(style));
}
void ScrollSnapAlignImpl(Ark_NativePointer node,
                         const Opt_ScrollSnapAlign* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    ListModelNG::SetScrollSnapAlign(frameNode, Converter::OptConvert<ScrollSnapAlign>(*value));
}
void ChildrenMainSizeImpl(Ark_NativePointer node,
                          const Opt_ChildrenMainSize* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optValue = Converter::GetOptPtr(value);
    if (!optValue) {
        // TODO: Reset value
        return;
    }
    auto peer = *optValue;
    CHECK_NULL_VOID(peer);
    RefPtr<ListChildrenMainSize> handler = ListModelNG::GetOrCreateListChildrenMainSize(frameNode);
    peer->SetHandler(handler);
}
void MaintainVisibleContentPositionImpl(Ark_NativePointer node,
                                        const Opt_Boolean* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto convValue = Converter::OptConvert<bool>(*value);
    if (!convValue) {
        // TODO: Reset value
        return;
    }
    ListModelNG::SetListMaintainVisibleContentPosition(frameNode, *convValue);
}
void StackFromEndImpl(Ark_NativePointer node,
                      const Opt_Boolean* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //ListModelNG::SetStackFromEnd(frameNode, convValue);
}
void OnScrollIndexImpl(Ark_NativePointer node,
                       const Opt_Callback_Number_Number_Number_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optValue = Converter::GetOptPtr(value);
    if (!optValue) {
        // TODO: Reset value
        return;
    }
    auto onScrollIndex = [arkCallback = CallbackHelper(*optValue)]
            (const int32_t start, const int32_t end, const int32_t center) {
        auto arkStart = Converter::ArkValue<Ark_Number>(start);
        auto arkEnd = Converter::ArkValue<Ark_Number>(end);
        auto arkCenter = Converter::ArkValue<Ark_Number>(center);
        arkCallback.Invoke(arkStart, arkEnd, arkCenter);
    };
    ListModelNG::SetOnScrollIndex(frameNode, std::move(onScrollIndex));
}
void OnScrollVisibleContentChangeImpl(Ark_NativePointer node,
                                      const Opt_OnScrollVisibleContentChangeCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optValue = Converter::GetOptPtr(value);
    if (!optValue) {
        // TODO: Reset value
        return;
    }
    auto onScrollVisibleContentChange = [arkCallback = CallbackHelper(*optValue)]
            (const ListItemIndex& start, const ListItemIndex& end) {
        auto startItemInfo = Converter::ArkValue<Ark_VisibleListContentInfo>(start);
        auto endItemInfo = Converter::ArkValue<Ark_VisibleListContentInfo>(end);
        arkCallback.Invoke(startItemInfo, endItemInfo);
    };
    ListModelNG::SetOnScrollVisibleContentChange(frameNode, std::move(onScrollVisibleContentChange));
}
void OnItemMoveImpl(Ark_NativePointer node,
                    const Opt_Callback_Number_Number_Boolean* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optValue = Converter::GetOptPtr(value);
    if (!optValue) {
        // TODO: Reset value
        return;
    }
    auto onItemMove = [callback = CallbackHelper(*optValue)](int32_t from, int32_t to) {
        auto arkFrom = Converter::ArkValue<Ark_Number>(from);
        auto arkTo = Converter::ArkValue<Ark_Number>(to);
        auto arkResult = callback.InvokeWithObtainResult<Ark_Boolean, Callback_Boolean_Void>(arkFrom, arkTo);
        return Converter::Convert<bool>(arkResult);
    };
    ListModelNG::SetOnItemMove(frameNode, std::move(onItemMove));
}
void OnItemDragStartImpl(Ark_NativePointer node,
                         const Opt_OnItemDragStartCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optValue = Converter::GetOptPtr(value);
    if (!optValue) {
        // TODO: Reset value
        return;
    }
    auto onItemDragStart = [callback = CallbackHelper(*optValue), frameNode, node](
        const ItemDragInfo& dragInfo, int32_t itemIndex
    ) -> RefPtr<AceType> {
        auto arkDragInfo = Converter::ArkValue<Ark_ItemDragInfo>(dragInfo);
        auto arkItemIndex = Converter::ArkValue<Ark_Number>(itemIndex);
        auto builder =
            callback.InvokeWithObtainCallback<CustomNodeBuilder, Callback_Opt_CustomBuilder_Void>(
                arkDragInfo, arkItemIndex);
        return builder->BuildSync(node);
    };
    ListModelNG::SetOnItemDragStart(frameNode, std::move(onItemDragStart));
}
void OnItemDragEnterImpl(Ark_NativePointer node,
                         const Opt_Callback_ItemDragInfo_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optValue = Converter::GetOptPtr(value);
    if (!optValue) {
        // TODO: Reset value
        return;
    }
    auto onItemDragEnter = [arkCallback = CallbackHelper(*optValue)](const ItemDragInfo& dragInfo) {
        auto arkDragInfo = Converter::ArkValue<Ark_ItemDragInfo>(dragInfo);
        arkCallback.Invoke(arkDragInfo);
    };
    ListModelNG::SetOnItemDragEnter(frameNode, std::move(onItemDragEnter));
}
void OnItemDragMoveImpl(Ark_NativePointer node,
                        const Opt_Callback_ItemDragInfo_Number_Number_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optValue = Converter::GetOptPtr(value);
    if (!optValue) {
        // TODO: Reset value
        return;
    }
    auto onItemDragMove = [arkCallback = CallbackHelper(*optValue)](const ItemDragInfo& dragInfo,
            int32_t itemIndex, int32_t insertIndex) {
        auto arkDragInfo = Converter::ArkValue<Ark_ItemDragInfo>(dragInfo);
        auto arkItemIndex = Converter::ArkValue<Ark_Number>(itemIndex);
        auto arkInsertIndex = Converter::ArkValue<Ark_Number>(insertIndex);
        arkCallback.Invoke(arkDragInfo, arkItemIndex, arkInsertIndex);
    };
    ListModelNG::SetOnItemDragMove(frameNode, std::move(onItemDragMove));
}
void OnItemDragLeaveImpl(Ark_NativePointer node,
                         const Opt_Callback_ItemDragInfo_Number_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optValue = Converter::GetOptPtr(value);
    if (!optValue) {
        // TODO: Reset value
        return;
    }
    auto onItemDragLeave = [arkCallback = CallbackHelper(*optValue)](const ItemDragInfo& dragInfo, int32_t itemIndex) {
        auto arkDragInfo = Converter::ArkValue<Ark_ItemDragInfo>(dragInfo);
        auto arkItemIndex = Converter::ArkValue<Ark_Number>(itemIndex);
        arkCallback.Invoke(arkDragInfo, arkItemIndex);
    };
    ListModelNG::SetOnItemDragLeave(frameNode, std::move(onItemDragLeave));
}
void OnItemDropImpl(Ark_NativePointer node,
                    const Opt_Callback_ItemDragInfo_Number_Number_Boolean_Void* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optValue = Converter::GetOptPtr(value);
    if (!optValue) {
        // TODO: Reset value
        return;
    }
    auto onItemDrop = [arkCallback = CallbackHelper(*optValue)](const ItemDragInfo& dragInfo,
            int32_t itemIndex, int32_t insertIndex, bool isSuccess) {
        auto arkDragInfo = Converter::ArkValue<Ark_ItemDragInfo>(dragInfo);
        auto arkItemIndex = Converter::ArkValue<Ark_Number>(itemIndex);
        auto arkInsertIndex = Converter::ArkValue<Ark_Number>(insertIndex);
        auto arkIsSuccess = Converter::ArkValue<Ark_Boolean>(isSuccess);
        arkCallback.Invoke(arkDragInfo, arkItemIndex, arkInsertIndex, arkIsSuccess);
    };
    ListModelNG::SetOnItemDrop(frameNode, std::move(onItemDrop));
}
void OnScrollFrameBeginImpl(Ark_NativePointer node,
                            const Opt_OnScrollFrameBeginCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto optValue = Converter::GetOptPtr(value);
    if (!optValue) {
        // TODO: Reset value
        return;
    }
    auto onScrollFrameBegin = [callback = CallbackHelper(*optValue)](
            const Dimension& offset, const ScrollState& state
        ) -> ScrollFrameResult {
        auto arkOffset = Converter::ArkValue<Ark_Number>(offset);
        auto arkState = Converter::ArkValue<Ark_ScrollState>(state);
        auto arkResult = callback.InvokeWithObtainResult<Ark_OnScrollFrameBeginHandlerResult,
            Callback_OnScrollFrameBeginHandlerResult_Void>(arkOffset, arkState);
        return {
            .offset = Converter::Convert<Dimension>(arkResult.offsetRemain)
        };
    };
    ListModelNG::SetOnScrollFrameBegin(frameNode, std::move(onScrollFrameBegin));
}
void OnWillScrollImpl(Ark_NativePointer node,
                      const Opt_OnWillScrollCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //ListModelNG::SetOnWillScroll(frameNode, convValue);
}
void OnDidScrollImpl(Ark_NativePointer node,
                     const Opt_OnScrollCallback* value)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    //auto convValue = value ? Converter::OptConvert<type>(*value) : std::nullopt;
    //ListModelNG::SetOnDidScroll(frameNode, convValue);
}
void LanesImpl(Ark_NativePointer node,
               const Opt_Union_Number_LengthConstrain* value,
               const Opt_Dimension* gutter)
{
    auto frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);

    if (value != nullptr) {
        auto lanes = Converter::OptConvert<ListLanesType>(*value);
        if (lanes) {
            if (lanes.value().index() == 0) {
                int lane = std::get<0>(lanes.value());
                ListModelNG::SetLanes(frameNode, lane);
                ListModelNG::SetLaneConstrain(frameNode, Dimension(), Dimension());
            } else {
                auto dimensions = std::get<1>(lanes.value());
                ListModelNG::SetLanes(frameNode, 1);
                ListModelNG::SetLaneConstrain(frameNode, std::get<0>(dimensions), std::get<1>(dimensions));
            }
        }
    }

    if (gutter != nullptr) {
        std::optional<Dimension> gutterOpt;
        Converter::AssignOptionalTo(gutterOpt, *gutter);
        if (gutterOpt.has_value()) {
            ListModelNG::SetLaneGutter(frameNode, gutterOpt);
        }
    }
}
} // ListAttributeModifier
const GENERATED_ArkUIListModifier* GetListModifier()
{
    static const GENERATED_ArkUIListModifier ArkUIListModifierImpl {
        ListModifier::ConstructImpl,
        ListInterfaceModifier::SetListOptionsImpl,
        ListAttributeModifier::AlignListItemImpl,
        ListAttributeModifier::ListDirectionImpl,
        ListAttributeModifier::ContentStartOffsetImpl,
        ListAttributeModifier::ContentEndOffsetImpl,
        ListAttributeModifier::DividerImpl,
        ListAttributeModifier::MultiSelectableImpl,
        ListAttributeModifier::CachedCount0Impl,
        ListAttributeModifier::CachedCount1Impl,
        ListAttributeModifier::ChainAnimationImpl,
        ListAttributeModifier::ChainAnimationOptionsImpl,
        ListAttributeModifier::StickyImpl,
        ListAttributeModifier::ScrollSnapAlignImpl,
        ListAttributeModifier::ChildrenMainSizeImpl,
        ListAttributeModifier::MaintainVisibleContentPositionImpl,
        ListAttributeModifier::StackFromEndImpl,
        ListAttributeModifier::OnScrollIndexImpl,
        ListAttributeModifier::OnScrollVisibleContentChangeImpl,
        ListAttributeModifier::OnItemMoveImpl,
        ListAttributeModifier::OnItemDragStartImpl,
        ListAttributeModifier::OnItemDragEnterImpl,
        ListAttributeModifier::OnItemDragMoveImpl,
        ListAttributeModifier::OnItemDragLeaveImpl,
        ListAttributeModifier::OnItemDropImpl,
        ListAttributeModifier::OnScrollFrameBeginImpl,
        ListAttributeModifier::OnWillScrollImpl,
        ListAttributeModifier::OnDidScrollImpl,
        ListAttributeModifier::LanesImpl,
    };
    return &ArkUIListModifierImpl;
}
}
