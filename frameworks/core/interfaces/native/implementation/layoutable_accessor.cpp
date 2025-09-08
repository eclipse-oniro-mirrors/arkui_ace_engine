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
#include "core/interfaces/native/utility/reverse_converter.h"
#include "core/interfaces/native/implementation/layoutable_peer.h"
#include "core/interfaces/native/utility/peer_utils.h"
#include "arkoala_api_generated.h"

namespace OHOS::Ace::NG::GeneratedModifier {
namespace {
Ark_DirectionalEdgesT GenEdgesGlobalized(const NG::PaddingPropertyT<float>& edgeNative, TextDirection direction)
{
    Ark_DirectionalEdgesT edges;
    auto pipeline = PipelineBase::GetCurrentContext();
    double px2vpScale = (pipeline && !NearZero(pipeline->GetDipScale())) ? 1.0 / pipeline->GetDipScale() : 1.0;
    edges.top = Converter::ArkValue<Ark_Number>(edgeNative.top.value_or(0) * px2vpScale);
    edges.bottom = Converter::ArkValue<Ark_Number>(edgeNative.bottom.value_or(0) * px2vpScale);
    if (direction != TextDirection::RTL) {
        edges.start = Converter::ArkValue<Ark_Number>(edgeNative.left.value_or(0) * px2vpScale);
        edges.end = Converter::ArkValue<Ark_Number>(edgeNative.right.value_or(0) * px2vpScale);
    } else {
        edges.start = Converter::ArkValue<Ark_Number>(edgeNative.right.value_or(0) * px2vpScale);
        edges.end = Converter::ArkValue<Ark_Number>(edgeNative.left.value_or(0) * px2vpScale);
    }
    return edges;
}
Ark_DirectionalEdgesT GenBorderWidthGlobalized(const NG::BorderWidthPropertyT<float>& edgeNative,
    TextDirection direction)
{
    Ark_DirectionalEdgesT edges;
    auto pipeline = PipelineBase::GetCurrentContext();
    double px2vpScale = (pipeline && !NearZero(pipeline->GetDipScale())) ? 1.0 / pipeline->GetDipScale() : 1.0;
    edges.top = Converter::ArkValue<Ark_Number>(edgeNative.topDimen.value_or(0) * px2vpScale);
    edges.bottom = Converter::ArkValue<Ark_Number>(edgeNative.bottomDimen.value_or(0) * px2vpScale);
    if (direction != TextDirection::RTL) {
        edges.start = Converter::ArkValue<Ark_Number>(edgeNative.leftDimen.value_or(0) * px2vpScale);
        edges.end = Converter::ArkValue<Ark_Number>(edgeNative.rightDimen.value_or(0) * px2vpScale);
    } else {
        edges.start = Converter::ArkValue<Ark_Number>(edgeNative.rightDimen.value_or(0) * px2vpScale);
        edges.end = Converter::ArkValue<Ark_Number>(edgeNative.leftDimen.value_or(0) * px2vpScale);
    }
    return edges;
}
const Ark_DirectionalEdgesT DEFAULT_EDGES = {
    .top = Converter::ArkValue<Ark_Number>(0.0f),
    .bottom = Converter::ArkValue<Ark_Number>(0.0f),
    .start = Converter::ArkValue<Ark_Number>(0.0f),
    .end = Converter::ArkValue<Ark_Number>(0.0f),
};
const Ark_MeasureResult DEFAULT_MEASURE_RESULT = {
    .width = Converter::ArkValue<Ark_Number>(0.0f),
    .height = Converter::ArkValue<Ark_Number>(0.0f),
};
} // namespace
namespace LayoutableAccessor {
void DestroyPeerImpl(Ark_Layoutable peer)
{
    PeerUtils::DestroyPeer(peer);
}
Ark_Layoutable ConstructImpl()
{
    return PeerUtils::CreatePeer<LayoutablePeer>();
}
Ark_NativePointer GetFinalizerImpl()
{
    return reinterpret_cast<void *>(&DestroyPeerImpl);
}
void LayoutImpl(Ark_Layoutable peer,
                const Ark_Position* position)
{
    CHECK_NULL_VOID(peer && peer->measureLayoutParam && position);
    auto child = peer->measureLayoutParam->GetOrCreateChildByIndex(peer->index);
    CHECK_NULL_VOID(child);
    auto x = Converter::OptConvert<Dimension>(position->x);
    auto y = Converter::OptConvert<Dimension>(position->y);
    if (!(x.has_value() || y.has_value())) {
        LOGE("LayoutableAccessor::LayoutImpl the position prop is illegal");
    } else {
        double xVal = x.value_or(Dimension()).ConvertToPx();
        double yVal = y.value_or(Dimension()).ConvertToPx();
        OffsetF offset(xVal, yVal);
        child->GetGeometryNode()->SetMarginFrameOffset(offset);
    }
    child->Layout();
}
Ark_DirectionalEdgesT GetMarginImpl(Ark_Layoutable peer)
{
    CHECK_NULL_RETURN(peer && peer->measureLayoutParam, DEFAULT_EDGES);
    auto child = peer->measureLayoutParam->GetOrCreateChildByIndex(peer->index);
    CHECK_NULL_RETURN(child, DEFAULT_EDGES);
    auto layoutProperty = child->GetLayoutProperty();
    CHECK_NULL_RETURN(child->GetLayoutProperty(), DEFAULT_EDGES);
    auto direction = layoutProperty->GetNonAutoLayoutDirection();
    return GenEdgesGlobalized(layoutProperty->CreateMarginWithoutCache(), direction);
}
Ark_DirectionalEdgesT GetPaddingImpl(Ark_Layoutable peer)
{
    CHECK_NULL_RETURN(peer && peer->measureLayoutParam, DEFAULT_EDGES);
    auto child = peer->measureLayoutParam->GetOrCreateChildByIndex(peer->index);
    CHECK_NULL_RETURN(child, DEFAULT_EDGES);
    auto layoutProperty = child->GetLayoutProperty();
    CHECK_NULL_RETURN(child->GetLayoutProperty(), DEFAULT_EDGES);
    auto direction = layoutProperty->GetNonAutoLayoutDirection();
    return GenEdgesGlobalized(layoutProperty->CreatePaddingWithoutBorder(false, false), direction);
}
Ark_DirectionalEdgesT GetBorderWidthImpl(Ark_Layoutable peer)
{
    CHECK_NULL_RETURN(peer && peer->measureLayoutParam, DEFAULT_EDGES);
    auto child = peer->measureLayoutParam->GetOrCreateChildByIndex(peer->index);
    CHECK_NULL_RETURN(child, DEFAULT_EDGES);
    auto layoutProperty = child->GetLayoutProperty();
    CHECK_NULL_RETURN(child->GetLayoutProperty(), DEFAULT_EDGES);
    auto direction = layoutProperty->GetNonAutoLayoutDirection();
    return GenBorderWidthGlobalized(layoutProperty->CreateBorder(), direction);
}
Ark_MeasureResult GetMeasureResultImpl(Ark_Layoutable peer)
{
    CHECK_NULL_RETURN(peer && peer->measureLayoutParam, DEFAULT_MEASURE_RESULT);
    auto child = peer->measureLayoutParam->GetOrCreateChildByIndex(peer->index);
    CHECK_NULL_RETURN(child, DEFAULT_MEASURE_RESULT);
    auto size = child->GetGeometryNode()->GetFrameSize();
    Dimension measureWidth(size.Width(), DimensionUnit::PX);
    Dimension measureHeight(size.Height(), DimensionUnit::PX);
    Ark_MeasureResult measureResult = {
        .width = Converter::ArkValue<Ark_Number>(measureWidth.ConvertToVp()),
        .height = Converter::ArkValue<Ark_Number>(measureHeight.ConvertToVp()),
    };
    return measureResult;
}
void SetMeasureResultImpl(Ark_Layoutable peer,
                          const Ark_MeasureResult* measureResult)
{
    LOGE("LayoutableAccessor::SetMeasureResultImpl is not implemented, only getter is supported");
}
Opt_Number GetUniqueIdImpl(Ark_Layoutable peer)
{
    Opt_Number invalid = Converter::ArkValue<Opt_Number>(Ark_Empty());
    CHECK_NULL_RETURN(peer && peer->measureLayoutParam, invalid);
    auto child = peer->measureLayoutParam->GetOrCreateChildByIndex(peer->index);
    CHECK_NULL_RETURN(child, invalid);
    auto uniqueId = child->GetHostNode()->GetId();
    return Converter::ArkValue<Opt_Number>(uniqueId);
}
void SetUniqueIdImpl(Ark_Layoutable peer,
                     const Opt_Number* uniqueId)
{
    LOGE("LayoutableAccessor::SetUniqueIdImpl is not implemented, only getter is supported");
}
} // LayoutableAccessor
const GENERATED_ArkUILayoutableAccessor* GetLayoutableAccessor()
{
    static const GENERATED_ArkUILayoutableAccessor LayoutableAccessorImpl {
        LayoutableAccessor::DestroyPeerImpl,
        LayoutableAccessor::ConstructImpl,
        LayoutableAccessor::GetFinalizerImpl,
        LayoutableAccessor::LayoutImpl,
        LayoutableAccessor::GetMarginImpl,
        LayoutableAccessor::GetPaddingImpl,
        LayoutableAccessor::GetBorderWidthImpl,
        LayoutableAccessor::GetMeasureResultImpl,
        LayoutableAccessor::SetMeasureResultImpl,
        LayoutableAccessor::GetUniqueIdImpl,
        LayoutableAccessor::SetUniqueIdImpl,
    };
    return &LayoutableAccessorImpl;
}

}
